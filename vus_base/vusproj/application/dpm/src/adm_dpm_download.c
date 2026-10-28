/**
 * @file    adm_dpm_download.c
 * @brief   dpm download moudle 
 * @details dpm模块适配下载接口(master dpo and slave dpo download API)
 * @author  wangyanpeng
 * @version v0.1.0
 * @date    2020-11-06
 * @copyright www.abupdate.com
 */

#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include "adm_cvs_checkVersion.h"
#include "adm_cvs_versionPolicy.h"
#include "adm_dpm_log.h"
#include "adm_dpm_download.h"
#include "adm_dpm_report.h"
#include "adm_dpm_reportCloud.h"
#include "adm_dpm_uim.h"
#include "adm_dpo_download.h"
#include "adm_dim_dpo.h"
#include "adm_ocm_interface.h"
#include "adm_cfg_default.h"
#include "adm_cfg_api.h"
#include "adm_oca_file.h"
#include "adm_dim_host.h"
#include "adm_com_lib.h"
#include "adm_file.h"
#include "adm_irm_report.h"
#include "adm_application_errorCode.h"
#include <sys/stat.h>
#include "adm_soa.h"
#include "adm_pub_func.h"
#include "adm_sdk_version.h"
#include "adm_soa.h"

//!< 下载网络类型监控处理
static ADM_DOWN_ASYNCTAG_S gstDownAsyncTag = {0};

static D32S g_downloadIngInstId = 0;

/**下载网络类型异常标志*/
static DBOOL g_netWorkTypeAbnormalFlag = DFALSE;  


//<! 是否已通知 HMI 网络异常但尚未恢复
static DBOOL  g_hmiNetExceptionActive    = DTRUE; //!< 默认通知过，兼容重启前是网络异常情况

//<! 触发异常时的进度基线
static D64U   g_hmiNetExceptionRefBytes  = 0;

//<! 最近一次上报进度时的字节数
static D64U   g_lastReportedBytes        = 0;

//<! 速度监控：速度为0的连续计数
static D32U g_uiZeroSpeedCnt = 0;

//<! 网络恢复触发阈值
#define HMI_NET_RECOVER_MIN_BYTES   (1ULL * 1024ULL)  // 1KB
#define HMI_NET_RECOVER_MIN_PERCENT 1  // 1%

//<! 速度监控阈值：连续N次速度为0且字节没变化则判定网络异常
#define DPM_ZERO_SPEED_THRESHOLD    3



//!< dpm下载进度/结果消息通知时间间隔，单位ms, (进度通知时间间隔较大会导致，队列产生进度内容大于消费进度内容,导致结果通知之后，进度通知还未通知完，需要保证产生接近于消费)
#define ADM_DPM_MSG_NOTIFY_TIME_WIDE    2000

//!< dpm下载网络类型监控时间间隔，单位ms, 监控一次10s
#define ADM_DPM_NETWORKTYPE_MONITOR_TIME_WIDE 10000

#define ADM_DPM_DOWNLOAD_RETRY_INTERVAL 180

#if 0
#endif

D32S adm_dpm_getCurDownloadIngInstId()
{
    return g_downloadIngInstId;
}

// TODO: 同步转异步消息

/*!
 *******************************************************************************
 * 创建下载异步标签信息
 *
 * \note 对应销毁函数 __adm_dpm_destroyDownAsyncTagInfo
 * 
 * \param   none
 * \retval  ADM_DOWN_ASYNCTAG_S
 *******************************************************************************
 */
static ADM_DOWN_ASYNCTAG_S* __adm_dpm_createDownAsyncTagInfo(void)
{
    ADM_DOWN_ASYNCTAG_S *pstDownAsyncTag = NULL;

    pstDownAsyncTag = (ADM_DOWN_ASYNCTAG_S *)adm_malloc(sizeof(*pstDownAsyncTag));
    if (ADM_IS_NULL(pstDownAsyncTag))
    {
        log_e("Failed to malloc down async tag info");
        return NULL;
    }
    (void)adm_memzero(pstDownAsyncTag, sizeof(*pstDownAsyncTag));

    pthread_condattr_t cattr;
    pthread_condattr_init(&cattr);
    pthread_condattr_setclock(&cattr, CLOCK_MONOTONIC);

    pstDownAsyncTag->bHasResult = FALSE;
    pstDownAsyncTag->bIsFlush = FALSE;
    pstDownAsyncTag->stResult.result = ADM_ECODE_SUCCESSED;
    (void)pthread_mutex_init(&pstDownAsyncTag->stMutex, NULL);
    (void)pthread_cond_init(&pstDownAsyncTag->stCond, &cattr);

    return pstDownAsyncTag;
}

/*!
 *******************************************************************************
 * 销毁下载异步标签信息
 *
 * \note 对应创建函数 __adm_dpm_createDownAsyncTagInfo
 * 
 * \param[in]  pstDownAsyncTag   下载异步标签信息
 * \retval     none
 *******************************************************************************
 */
static void __adm_dpm_destroyDownAsyncTagInfo(ADM_DOWN_ASYNCTAG_S *pstDownAsyncTag)
{
    if (ADM_IS_NULL(pstDownAsyncTag))
    {
        return;
    }

    adm_free(pstDownAsyncTag->stResult.failReason);
    (void)pthread_mutex_destroy(&pstDownAsyncTag->stMutex);
    (void)pthread_cond_destroy(&pstDownAsyncTag->stCond);
    adm_free(pstDownAsyncTag);

    return;
}



/*!
 *******************************************************************************
 * dpm下载通知时间宽度判断
 *
 * \note 获取时间使用的是 CLOCK_MONOTONIC 不随系统时间改变而变化。
 *          入参传入上次的时间，可以保证该函数可重入
 * 
 * \param[in]   pstTs   上次通知的时间信息
 * \param[in]   pWide   和上次通知的时间间隔(s)
 * \retval      DBOOL   TRUE: 超过通知时间阈值，FALSE：未超过通知时间阈值
 *******************************************************************************
 */
static DBOOL __adm_dpm_isOverNotifyTimeWide(struct timespec *pstTs, D32U* pWide)
{
    DBOOL bRet = FALSE;
    struct timespec tsNow;
    D32U uiWide = 0;

    if (ADM_IS_NULL(pstTs))
    {
        log_e("Invalid input param");
        return bRet;
    }

    (void)clock_gettime(CLOCK_MONOTONIC, &tsNow);

    if (NULL != pWide)
    {
        *pWide = (tsNow.tv_sec - pstTs->tv_sec) + ((tsNow.tv_nsec - pstTs->tv_nsec) / (1000 * 1000 * 1000));
    }

    if(((tsNow.tv_sec - pstTs->tv_sec) * 1000 +
        (tsNow.tv_nsec - pstTs->tv_nsec) / (1000 * 1000)) > 
              ADM_DPM_MSG_NOTIFY_TIME_WIDE)
    {
        bRet = TRUE;
        pstTs->tv_sec = tsNow.tv_sec;
        pstTs->tv_nsec = tsNow.tv_nsec;
    }
    
    return bRet;
}

/*!
*******************************************************************************
* Slave下载通知性能统计
*
* \note 
* 
* \param[in]   pstDownloadProgressInfo   下载包参数
* \param[in]   pstDpmDownReqParam        现在请求参数
* \retval      none
*******************************************************************************
*/
static void __adm_dpm_SlaveDownStatisticsHandle(const ADM_DIM_SOA_DPO_DOWNLOAD_PROGRESS_INFO_T *pstDownloadProgressInfo, ADM_DPM_DOWNREQPARAM_T *pstDpmDownReqParam)
{
    DCHAR szEventBuf[ADM_COM_LONGDATA_LEN] = {0};

    //!< 无私有属性数据则可直接退出
    if (ADM_IS_NULL(pstDownloadProgressInfo) || ADM_IS_NULL(pstDpmDownReqParam))
    {
        log_e("Invalid input param");
        return;
    }

    //!< 性能统计,进度通知
    (void)adm_memzero(szEventBuf, sizeof(szEventBuf));
    (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), 
            "STATSPACK downSpend:ecuReqid[%s], ecuSwDid[%s], pkgType[%s], fileName[%s], speed[%d], progress[%d], remainTime[%d], DownSize[%lld - %lld], downType[2](1: master, 2: slave)",
            pstDpmDownReqParam->stPkgAttr.ecuReqid,
            pstDpmDownReqParam->stPkgAttr.ecuSwDid,
            (pstDpmDownReqParam->stPkgAttr.isRollback == FALSE ? "part-dstPkg" : "part-srcPkg"),
            pstDpmDownReqParam->stPkgAttr.fileName,
            pstDownloadProgressInfo->speed,
            (D32S)((pstDownloadProgressInfo->totalSize != 0) ? ((pstDownloadProgressInfo->savedSize * 100) / pstDownloadProgressInfo->totalSize) : 0),
            pstDownloadProgressInfo->remainTime,
            pstDownloadProgressInfo->savedSize,
            pstDownloadProgressInfo->totalSize);

    adm_dpm_addEventAndReport(ADM_COM_MOD_DPM, FALSE, ADM_EVENT_ID_DL_001, ADM_OCS_EVENT_LEVEL_INFO, szEventBuf, NULL);

    return;
}

/*!
*******************************************************************************
* master下载通知性能统计
*
* \note 
* 
* \param[in]   pstNotifyInfo   下载包参数
* \param[in]   priv            私有信息
* \retval      none
*******************************************************************************
*/
static void __adm_dpm_MasterDownStatisticsHandle(ADM_DPO_PROGRESS_NOTIFY_T *pstNotifyInfo, ADM_DPM_DOWNREQPARAM_T *pstDpmDownReqParam)
{
    DCHAR szEventBuf[ADM_COM_LONGDATA_LEN] = {0};

    //!< 无私有属性数据则可直接退出
    if (ADM_IS_NULL(pstNotifyInfo) || ADM_IS_NULL(pstDpmDownReqParam))
    {
        log_e("Invalid input param");
        return;
    }

    //!< 性能统计,进度通知并上报
    if(ADM_DPS_DOWNPACKAGE_COMBINE == pstDpmDownReqParam->stPkgAttr.enPkgFormat)
    {
        //集合包
        (void)adm_memzero(szEventBuf, sizeof(szEventBuf));
        (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), 
                "STATSPACK downSpend:pkgType[combine], fileName[%s], downTime[%d]%s, speed[%d], progress[%d], remainTime[%d], DownSize[%lld - %lld], downType[1](1: master, 2: slave)",
                pstDpmDownReqParam->stPkgAttr.fileName,
                ((pstNotifyInfo->totalTime < 1000) ? (pstNotifyInfo->totalTime) : (pstNotifyInfo->totalTime / 1000)),
                ((pstNotifyInfo->totalTime < 1000) ? "ms" : "s"),
                pstNotifyInfo->downSpeed,
                (D32S)((pstNotifyInfo->totalSize != 0) ? ((pstNotifyInfo->savedSize * 100) / pstNotifyInfo->totalSize) : 0),
                pstNotifyInfo->remainTime,
                pstNotifyInfo->savedSize,
                pstNotifyInfo->totalSize);
        adm_dpm_addEventAndReport(ADM_COM_MOD_DPM, FALSE, ADM_EVENT_ID_DL_003, ADM_OCS_EVENT_LEVEL_INFO, szEventBuf, NULL);

        //!< 下载结束
        if(pstNotifyInfo->savedSize == pstNotifyInfo->totalSize)
        {
            (void)adm_memzero(szEventBuf, sizeof(szEventBuf));
            (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), 
                "STATSPACK downOnceTime:pkgType[combine], fileName[%s], downTime[%d]%s, fileSize[%lld], downType[1](1: master, 2: slave)",
                pstDpmDownReqParam->stPkgAttr.fileName,
                ((pstNotifyInfo->totalTime < 1000) ? (pstNotifyInfo->totalTime) : (pstNotifyInfo->totalTime / 1000)),
                ((pstNotifyInfo->totalTime < 1000) ? "ms" : "s"),
                pstNotifyInfo->totalSize);
            adm_dpm_addEventAndReport(ADM_COM_MOD_DPM, FALSE, ADM_EVENT_ID_DL_004, ADM_OCS_EVENT_LEVEL_INFO, szEventBuf, NULL);
        }
    }
    else if(ADM_DPS_DOWNPACKAGE_PART == pstDpmDownReqParam->stPkgAttr.enPkgFormat)
    {
        //零件包
        (void)adm_memzero(szEventBuf, sizeof(szEventBuf));
        (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), 
                "STATSPACK downSpend:ecuReqid[%s], ecuSwDid[%s], pkgType[%s], fileName[%s], downTime[%d]%s, speed[%d], progress[%d], remainTime[%d], DownSize[%lld - %lld], downType[1](1: master, 2: slave)",
                pstDpmDownReqParam->stPkgAttr.ecuReqid,
                pstDpmDownReqParam->stPkgAttr.ecuSwDid,
                (pstDpmDownReqParam->stPkgAttr.isRollback == FALSE ? "part-dstPkg" : "part-srcPkg"),
                pstDpmDownReqParam->stPkgAttr.fileName,
                ((pstNotifyInfo->totalTime < 1000) ? (pstNotifyInfo->totalTime) : (pstNotifyInfo->totalTime / 1000)),
                ((pstNotifyInfo->totalTime < 1000) ? "ms" : "s"),
                pstNotifyInfo->downSpeed,
                (D32S)((pstNotifyInfo->totalSize != 0) ? ((pstNotifyInfo->savedSize * 100) / pstNotifyInfo->totalSize) : 0),
                pstNotifyInfo->remainTime,
                pstNotifyInfo->savedSize,
                pstNotifyInfo->totalSize);
        adm_dpm_addEventAndReport(ADM_COM_MOD_DPM, FALSE, ADM_EVENT_ID_DL_001, ADM_OCS_EVENT_LEVEL_INFO, szEventBuf, NULL);

        //!< 下载结束
        if(pstNotifyInfo->savedSize == pstNotifyInfo->totalSize)
        {
            (void)adm_memzero(szEventBuf, sizeof(szEventBuf));
            (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), 
                "STATSPACK downOnceTime:ecuReqid[%s], ecuSwDid[%s], pkgType[%s], fileName[%s], downTime[%d]%s, fileSize[%lld], downType[1](1: master, 2: slave)",
                pstDpmDownReqParam->stPkgAttr.ecuReqid,
                pstDpmDownReqParam->stPkgAttr.ecuSwDid,
                (pstDpmDownReqParam->stPkgAttr.isRollback == FALSE ? "part-dstPkg" : "part-srcPkg"),
                pstDpmDownReqParam->stPkgAttr.fileName,
                ((pstNotifyInfo->totalTime < 1000) ? (pstNotifyInfo->totalTime) : (pstNotifyInfo->totalTime / 1000)),
                ((pstNotifyInfo->totalTime < 1000) ? "ms" : "s"),
                pstNotifyInfo->totalSize);
            adm_dpm_addEventAndReport(ADM_COM_MOD_DPM, FALSE, ADM_EVENT_ID_DL_002, ADM_OCS_EVENT_LEVEL_INFO, szEventBuf, NULL);
        }
    }
    else
    {
        //do nothing;
    }

    return;
}


#if 0
#endif

/*!
 *******************************************************************************
 * 创建dpm下载网络类型监控参数
 *
 * \note
 *  
 * \param[in]   idownNetworkType  下载网络类型
 * \param[in]   iloopTime         循环监控时间 
 * \param[in]   fnInteruptDownload   中断下载回调函数
 * \retval      ADM_DOWN_NETWORKTYPE_MONITOR_S*
 *******************************************************************************
 */
ADM_DOWN_NETWORKTYPE_MONITOR_S* adm_dpm_createDPMDownNetMonitor
(
    D32S   idownNetworkType,
    D32S   iloopTime,
    ADM_DPM_INTERUPT_DOWNLOAD_CALLBACK_F fnInteruptDownload
)
{

    ADM_DOWN_NETWORKTYPE_MONITOR_S *pstDownNetworkTypeMonitor = NULL;

    pstDownNetworkTypeMonitor = (ADM_DOWN_NETWORKTYPE_MONITOR_S *)adm_malloc(sizeof(*pstDownNetworkTypeMonitor));
    if (ADM_IS_NULL(pstDownNetworkTypeMonitor))
    {
        log_e("Failed to malloc download param");
        return NULL;
    }
    (void)adm_memzero(pstDownNetworkTypeMonitor, sizeof(*pstDownNetworkTypeMonitor));

    pstDownNetworkTypeMonitor->downNetworkType = idownNetworkType;
    pstDownNetworkTypeMonitor->loopTime = iloopTime;
    pstDownNetworkTypeMonitor->fnInteruptDownload = fnInteruptDownload;

    return pstDownNetworkTypeMonitor;
}


/*!
 *******************************************************************************
 * 中断下载处理
 *
 * \note
 * \param[in]   actionType  下载处理动作类型
 * \retval      void
 *******************************************************************************
 */
void adm_dpm_InteruptDownload(D32S actionType)
{
    //校验参数是否合法
    if(actionType >= (D32S)ADM_DPM_DOWNLOAD_ACTION_TYPE_INVALID)
    {
        return ;
    }
    (void)adm_oca_interuptDownload(actionType);

    return ;
}

/*!
 *******************************************************************************
 * 网络类型监控到异常信息通知
 *
 * \note
 * \param[in]   actionType  下载处理动作类型
 * \retval      void
 *******************************************************************************
 */
static void dpmdownNetworkTypeAbnormalNotify(D32S actionType)
{
    D32S ierrorCode = 0;
    D32S iRet = ADM_ECODE_SUCCESSED;
    ADM_CVS_UPGRADE_MODE_E eUpgradeMode = ADM_CVS_UPGRADE_MODE_INVALID;

    log_e("====> Download task pause!");
    if (adm_dpm_getDownInfo()->s32UCMDownInstId == g_downloadIngInstId)
    {
        adm_dpm_InteruptDownload(actionType);
    }
    else
    {
        (void)adm_dim_dpo_interuptDownload((D16U)g_downloadIngInstId, 30, actionType, &ierrorCode);
    }

    iRet = adm_cvs_getUpgradeMode((D32S *)&eUpgradeMode);

    /** 常规任务和救援任务需要暂停下载，并通知用户网络类型不满足 */
    if (ADM_CVS_UPGRADE_MODE_USER_CLICK == eUpgradeMode || 
        ADM_CVS_UPGRADE_MODE_RESCUE == eUpgradeMode )
    {
        g_netWorkTypeAbnormalFlag = DTRUE;
    }
    
    return ;
}

/*!
 *******************************************************************************
 * 下载网络类型校验线程处理函数
 *
 * \note    
 * 
 * \param[in]  param  线程参数/通知回调函数
 * \retval     void *
 *******************************************************************************
 */
static void *dpmNetworkTypeMonitorThreadProc(void *param)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    struct timespec stTime;
    D32S downNetworkType = 0;
    D32S loopTime = 0;
    ADM_DOWN_NETWORKTYPE_MONITOR_S *pstDownNetworkTypeMonitor = NULL;

    log_i("Entry download network type monitor proc thread.");

    if (ADM_IS_NULL(param))
    {
        log_e("Invalid input param");
        return NULL;
    }

    pstDownNetworkTypeMonitor = (ADM_DOWN_NETWORKTYPE_MONITOR_S *)param;
    downNetworkType = pstDownNetworkTypeMonitor->downNetworkType;
    loopTime = pstDownNetworkTypeMonitor->loopTime;

    do
    {
        //!< 有结果，不再等待
        if (TRUE == gstDownAsyncTag.bHasResult)
        {   
            break ;
        }

        (void)pthread_mutex_lock(&gstDownAsyncTag.stMutex);
        //!< 获取下载的网络类型
        iRet = adm_dpm_networkTypeVerify(downNetworkType);
        if (ADM_IS_ERR(iRet))
        {
            //!< 网络类型异常暂停下载
            pstDownNetworkTypeMonitor->fnInteruptDownload(ADM_DPM_DOWNLOAD_ACTION_TYPE_NET_CHG_PAUSE);
        }

        //!< 等待超时时间 10s
        (void)adm_memzero(&stTime, sizeof(stTime));
        if (0 == clock_gettime(CLOCK_MONOTONIC, &stTime))
        {
            stTime.tv_nsec += ((D64S)loopTime % 1000) * 1000000;
            stTime.tv_sec  += (D64S)loopTime / 1000;
            if (stTime.tv_nsec > 1000000000)
            {
                stTime.tv_sec += 1;
                stTime.tv_nsec -= 1000000000;
            }
        }
        /** 无需改造,被系统信号打断重新校验网络无业务影响 */
        (void)pthread_cond_timedwait(&gstDownAsyncTag.stCond, &gstDownAsyncTag.stMutex, &stTime);
        (void)pthread_mutex_unlock(&gstDownAsyncTag.stMutex);

    }while (TRUE);

    adm_free(pstDownNetworkTypeMonitor);
    log_i("Return download network type monitor proc thread.");

    return NULL;
}

/*!
 *******************************************************************************
 * 开始下载中网络类型监控.
 *
 * \note 下载中网络类型监控
 * 
 * \retval      D32S
 *******************************************************************************
 */
static D32S dpmStartDownNetTypeMonitor(void)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    D32S idownNetworkType = 0;

    log_i("start download network type monitor!");
    //获取下载的网络类型
    iRet = adm_dps_getDownNetworkType(&idownNetworkType);
    if (ADM_IS_ERR(iRet))
    {
        log_e("Failed to get DownNetworkType, err[%d]", iRet);
        return iRet; 
    }

    //任意网络时不需要校验
    if((D32S)ADM_DPM_DOWNLOAD_NETWORK_TYPE_ALL != idownNetworkType)
    {
        iRet = adm_dpm_DownloadingNetworkTypeMonitor(idownNetworkType,
                                                     ADM_DPM_NETWORKTYPE_MONITOR_TIME_WIDE,
                                                     dpmdownNetworkTypeAbnormalNotify);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to create check down network type proc thread!");
            return iRet; 
        }
    }

    return iRet;
}

/*!
 *******************************************************************************
 * 下载网络类型校验
 *
 * \note   下载网络类型校验
 * 
 * \param   none
 * \retval  D32S
 *******************************************************************************
 */
D32S adm_dpm_networkTypeVerify(D32S idownNetworkType)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    D32S iucmNetworkType = 0;   //主控网络类型
    D32S s32hostInstId = 0;
    D32S errorCode = 0;
    ADM_DPM_DOWNLOAD_NETWORK_TYPE_E eNetworkType = ADM_DPM_DOWNLOAD_NETWORK_TYPE_INVALID;

    log_i("DownNetworkType is [%d]",idownNetworkType);

    //host实例id
    s32hostInstId = adm_cfg_iniParserGetInt(ADM_CFG_VUS_HOST_INSTANCEID,  ADM_CFG_VUS_HOST_INSTANCEID_DEFAULT_VAL);

    //主控host获取网络类型 
    iRet = adm_dim_host_getNetworkType((D16U)s32hostInstId, 30, &iucmNetworkType, &errorCode);
    if (ADM_IS_ERR(iRet) || ADM_IS_ERR(errorCode))
    {
        log_e("Failed to get ucmNetworkType,err[%d],errcode[%d]", iRet, errorCode);
        return ADM_ECODE_NET_TYPE_VERIFY_FAILED;
    }

    if (2 == iucmNetworkType)
    {
        eNetworkType = ADM_DPM_DOWNLOAD_NETWORK_TYPE_WIFI;
    }
    else if (1 == iucmNetworkType)
    {
        eNetworkType = ADM_DPM_DOWNLOAD_NETWORK_TYPE_OPERATOR;
    }
    else if (3 == iucmNetworkType)
    {
        log_i("get network all type.");
        eNetworkType = ADM_DPM_DOWNLOAD_NETWORK_TYPE_ALL;
    }
    else
    {
        log_w("get network invalid.[%d]", iucmNetworkType);
    }

    //校验网络类型
    if (ADM_DPM_DOWNLOAD_NETWORK_TYPE_ALL != eNetworkType && idownNetworkType != eNetworkType)
    {
        iRet = ADM_ECODE_NET_TYPE_VERIFY_FAILED;
        log_e("DownNetworkType[%d] and ucmNetworkType[%d] is different", idownNetworkType ,eNetworkType);
    }

    return iRet;
}

/*!
 *******************************************************************************
 * 下载中网络类型监控.
 *
 * \note 下载中网络类型监控
 * 
 * \param[in]   idownNetworkType  下载网络类型
 * \param[in]   iloopTime         循环监控时间 
 * \param[in]   fnInteruptDownload   中断下载回调函数
 * \retval      D32S
 *******************************************************************************
 */
D32S adm_dpm_DownloadingNetworkTypeMonitor
(
    D32S   idownNetworkType,
    D32S   iloopTime,
    ADM_DPM_INTERUPT_DOWNLOAD_CALLBACK_F fnInteruptDownload
)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    pthread_t stThreadHandle = {0};
    ADM_DOWN_NETWORKTYPE_MONITOR_S *pstDownNetworkTypeMonitor = NULL;

    log_i("download network type monitor !");

    do
    {
        //!< 下载过程网络类型监控(主控下载监控)
        pstDownNetworkTypeMonitor = adm_dpm_createDPMDownNetMonitor(idownNetworkType, iloopTime, fnInteruptDownload);
        if (ADM_IS_NULL(pstDownNetworkTypeMonitor))
        {
            iRet = ADM_ECODE_NO_MEMORY;
            log_e("Failed to new Down Network Type Monitor, err[%d]", iRet);
            break;
        }

        //校验下载的网络类型
        iRet = adm_dpm_networkTypeVerify(pstDownNetworkTypeMonitor->downNetworkType);
        if (ADM_IS_ERR(iRet))
        {
            ADM_CVS_UPGRADE_MODE_E eUpgradeMode = ADM_CVS_UPGRADE_MODE_INVALID;
            adm_cvs_getUpgradeMode((D32S *)&eUpgradeMode);
            /** 常规任务和救援任务需要暂停下载，并通知用户网络类型不满足 */
            if (ADM_CVS_UPGRADE_MODE_USER_CLICK == eUpgradeMode || 
                ADM_CVS_UPGRADE_MODE_RESCUE == eUpgradeMode )
            {
                g_netWorkTypeAbnormalFlag = DTRUE;
            }

            adm_free(pstDownNetworkTypeMonitor);
            log_e("Failed to Verify network, err[%d]", iRet);
            break;
        }

        //!< 初始化全局变量异步标签
        (void)adm_memzero(&gstDownAsyncTag, sizeof(gstDownAsyncTag));
        gstDownAsyncTag.bHasResult = FALSE;
        pthread_condattr_t cattr;
        pthread_condattr_init(&cattr);
        pthread_condattr_setclock(&cattr, CLOCK_MONOTONIC);

        (void)pthread_mutex_init(&gstDownAsyncTag.stMutex, NULL);
        (void)pthread_cond_init(&gstDownAsyncTag.stCond, &cattr);
        g_netWorkTypeAbnormalFlag = DFALSE;

         //!< 线程中进行下载网络类型校验
        iRet = adm_dpm_pthreadCreate(&stThreadHandle, dpmNetworkTypeMonitorThreadProc, pstDownNetworkTypeMonitor);
        if (ADM_IS_ERR(iRet))
        {
            adm_free(pstDownNetworkTypeMonitor);
            log_e("Failed to create Network Type Monitor proc thread!, err[%d]", iRet);
            break;
        }
    } while(FALSE);

    return iRet;
}

/*!
 *******************************************************************************
 * 创建下载结果信息
 *
 * \note 
 * 
 * \param[in]   iResult     下载结果 0: success, other: failed
 * \param[in]   pstPkgAttr  下载文件属性.
 * \retval      ADM_DPS_DOWNRESULT_T *
 *******************************************************************************
 */
ADM_DPS_DOWNRESULT_T *adm_dpm_createDownResultInfo
(
    D32S iResult, 
    ADM_DPS_PKGATTR_T *pstPkgAttr
)
{
    ADM_DPS_DOWNRESULT_T *pstResult = NULL;

    pstResult = (ADM_DPS_DOWNRESULT_T *)adm_malloc(sizeof(*pstResult));
    if (ADM_IS_NULL(pstResult))
    {   
        log_e("Failed to malloc dps down result info. err[%d]", ADM_ECODE_NO_MEMORY);
        return NULL;
    }
    (void)adm_memzero(pstResult, sizeof(*pstResult));
    pstResult->result = iResult;
    pstResult->failReason = adm_dpm_errorCode2String(iResult);
    // TODO: 这里需要根据 iResult 继续细化下载状态 
    pstResult->eDownState = (ADM_IS_ERR(iResult) ? 
                           (ADM_DPS_DOWNPKG_STATE_FAILED) : (ADM_DPS_DOWNPKG_STATE_FINISH));

    //!< 若未携带附加ecu属性 or 下载失败，则直接返回
    if (ADM_IS_NULL(pstPkgAttr) || ADM_IS_ERR(iResult))
    {
        return pstResult;
    }

    /*
     * 集合包，下载完成则置状态为 下载完成等待解析
     * if (ADM_DPS_DOWNPACKAGE_COMBINE == pstPkgAttr->enPkgFormat)
     * {
     *    pstResult->eDownState = ADM_DPS_DOWNPKG_STATE_WAITDECRYPT;
     * }
     * else if (ADM_DPS_DOWNPACKAGE_PART == pstPkgAttr->enPkgFormat) 
     * {
     *    若为dstPkg则任务该ecu的升级包下载完成(下载顺序一定要先 srcPkg->dstPkg)
     *    if (FALSE == pstPkgAttr->isRollback)
     *    {
     *        pstResult->eDownState = ADM_DPS_DOWNPKG_STATE_WAITDECRYPT;
     *    }
     *    else
     *    {
     *        pstResult->eDownState = ADM_DPS_DOWNPKG_STATE_SRCFINISH;
     *    }
     * }
     *
     */

    //!< 上面分析得知，零件包 && 源包时将结果置为源包下载完成，其他置为下载完成待解密
    if ((ADM_DPS_DOWNPACKAGE_PART == pstPkgAttr->enPkgFormat) &&
        ((TRUE == pstPkgAttr->isRollback)))
    {
        pstResult->eDownState = ADM_DPS_DOWNPKG_STATE_SRCFINISH;
    }
    else
    {
        pstResult->eDownState = ADM_DPS_DOWNPKG_STATE_WAITDECRYPT;
    }

    return pstResult;
}

/*!
 *******************************************************************************
 * 释放下载结果信息
 *
 * \note 
 * 
 * \param[in]   pstResult  下载结果信息 
 * \retval      none
 *******************************************************************************
 */
void adm_dpm_freeDownResultInfo(ADM_DPS_DOWNRESULT_T *pstResult)
{
    if (ADM_IS_NULL(pstResult))
    {
        return;
    }

    adm_free(pstResult->failReason);
    adm_free(pstResult);
    return;
}

/*!
 *******************************************************************************
 * 进程等待信号函数
 *
 * \note
 *
 * \param[in]  pstCond   信号量
 * \param[in]  pstMutex  信号量锁
 * \param[in]  pIsSignal 信号是否是主动打断
 * \param[in]  ulAbsTime 等待信号量超时时间(单位毫秒)
 * \retval     D32S
 *******************************************************************************
 */
D32S adm_dpm_ThrdCondWait
(
    pthread_cond_t  *pstCond,
    pthread_mutex_t *pstMutex,
    DBOOL *pIsSignal,
    DBOOL *pIsFlush,
    D32S  ulAbsTime
)
{
    if (NULL == pstCond || NULL == pstMutex || NULL == pIsSignal)
    {
        log_e("Invalid param.");
        return ADM_ECODE_INVALID_PARAM;
    }

    D32S iRet;
    // (void)pthread_mutex_lock(pstMutex);

    if(0 < ulAbsTime)
    {
        iRet = adm_pub_pthread_cond_timedwait(pstCond, pstMutex, pIsSignal, pIsFlush, ulAbsTime);
    }
    else
    {
        iRet = pthread_cond_wait(pstCond, pstMutex);
    }

    // (void)pthread_mutex_unlock(pstMutex);

    return iRet;
}


/*!
 *******************************************************************************
 * 异步转同步超时判断.
 *
 * \note 
 *
 * \param[in]   pstDownAsyncInfo 异步条件变量信息
 * \retval      none
 *******************************************************************************
 */
static D32S adm_dpm_waitAsyncResult(ADM_DOWN_ASYNCTAG_S *pstDownAsyncInfo)
{
    D32S iRet = ADM_ECODE_SUCCESSED;

    //!<  异步响应提前到达
    if (FALSE != pstDownAsyncInfo->bHasResult)
    {
        return iRet;
    }

#if 0
    do
    {
        //!< 1、异步请求转同步等待 --- 等待超时时间 10s
        log_i("Wait slave download async result, timeout[%d].", ADM_COM_ASYNC_WAIT_TIME);
        (void)adm_dpm_ThrdCondWait(&pstDownAsyncInfo->stCond, &pstDownAsyncInfo->stMutex,
                                   &pstDownAsyncInfo->bHasResult, ADM_COM_ASYNC_WAIT_TIME);

        //!< 2、等待10s后，有结果，不再等待
        if (TRUE == pstDownAsyncInfo->bHasResult)
        {
            break ;
        }
        //!< 3、等待10s后，没有结果，但是有进度，需要继续等待
        else if (TRUE == pstDownAsyncInfo->bIsFlush)
        {
            (void)pthread_mutex_lock(&pstDownAsyncInfo->stMutex);
            pstDownAsyncInfo->bIsFlush = FALSE;
            (void)pthread_mutex_unlock(&pstDownAsyncInfo->stMutex);
            continue; 
        }
        //!< 4、既没结果又没有进度刷新，返回异步响应超时
        else
        {
            iRet = ADM_ECODE_MSG_WAIT_TIMEOUT;
            break;
        }
    }while (TRUE);
#endif

    log_i("Wait slave download async result, timeout[%d].", ADM_COM_ASYNC_WAIT_TIME);
    iRet = adm_dpm_ThrdCondWait(&pstDownAsyncInfo->stCond, &pstDownAsyncInfo->stMutex,
                                &pstDownAsyncInfo->bHasResult, &pstDownAsyncInfo->bIsFlush, ADM_COM_ASYNC_WAIT_TIME);
    if (ADM_IS_ERR(iRet))
    {
        iRet = ADM_ECODE_MSG_WAIT_TIMEOUT;
    }

    return iRet;

}


#if 0
#endif


/*!
 *******************************************************************************
 * slave 下载结果通知回调函数
 *
 * \note
 * 
 * \param[in]   fileID    下载文件标识id
 * \param[in]   storePath 下载成功时slave侧文件的存储路径
 * \param[in]   pstDownloadResultInfo 下载结果信息
 * \param[in]   priv    私有信息
 * \retval      D32S
 *******************************************************************************
 */
D32S adm_dpm_dimSlaveDownResultHandle
(
    const DCHAR *fileID, 
    const DCHAR *storePath, 
    const ADM_DIM_SOA_DPO_DOWNLOAD_RESULT_INFO_T *pstDownloadResultInfo, 
    void *priv
)
{
    ADM_DPS_DOWNRESULT_T   *pstResult          = NULL;
    ADM_DOWN_ASYNCTAG_S    *pstDownAsyncTag    = NULL;
    ADM_DPM_DOWNREQPARAM_T *pstDpmDownReqParam = NULL;

    if (ADM_IS_NULL(fileID) || ADM_IS_NULL(pstDownloadResultInfo))
    {
        log_e("Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }

    if (ADM_IS_NULL(adm_dpm_getDownInfo()->pstDpmDownReqParam))
    {
        log_e("Invalid input param, down reqparam is null");
        return ADM_ECODE_INVALID_PARAM;
    }

    log_i("slave download result notify, filePath[%s],fileID[%s],result[%d]", storePath,fileID,pstDownloadResultInfo->result);

    pstDpmDownReqParam = (ADM_DPM_DOWNREQPARAM_T *)adm_dpm_getDownInfo()->pstDpmDownReqParam;

    if (ADM_IS_NULL(pstDpmDownReqParam->fileID))
    {
        log_i("Invalid input param, reqparam fileID is null");
        return ADM_ECODE_INVALID_POINTER;
    }

    if (0 != adm_strcmp(fileID, pstDpmDownReqParam->fileID))
    {
        log_e("Invalid download fileID");
        return ADM_ECODE_INVALID_PARAM;
    }

    //!< 异步下载成功，更新下载的文件存储路径及结果
    if (ADM_IS_SUCCESS(pstDownloadResultInfo->result))
    {
        adm_free(pstDpmDownReqParam->filePath);
        pstDpmDownReqParam->filePath = adm_strdup(storePath);
    }

    pstResult = adm_dpm_createDownResultInfo(pstDownloadResultInfo->result, &pstDpmDownReqParam->stPkgAttr);
    //!< 上报进度信息
    if (NULL != pstDpmDownReqParam->fnDownResultNotify)
    {
        pstDpmDownReqParam->fnDownResultNotify(pstResult, storePath, &pstDpmDownReqParam->stPkgAttr);
    }

    // 异步转同步 --- launch async cond
    if (NULL != pstDpmDownReqParam->pAsyncTag)
    {
        pstDownAsyncTag = (ADM_DOWN_ASYNCTAG_S *)pstDpmDownReqParam->pAsyncTag;

        (void)pthread_mutex_lock(&pstDownAsyncTag->stMutex);
        pstDownAsyncTag->bHasResult = TRUE;
        pstDownAsyncTag->stResult.result = pstResult->result;
        pstDownAsyncTag->stResult.failReason = ((NULL != pstResult->failReason) ? (adm_strdup(pstResult->failReason)) : (NULL));
        pstDownAsyncTag->stResult.eDownState = pstResult->eDownState;
        (void)pthread_cond_signal(&pstDownAsyncTag->stCond);
        (void)pthread_mutex_unlock(&pstDownAsyncTag->stMutex);
    }
    adm_dpm_freeDownResultInfo(pstResult);
    log_i("slave download result notify end");

    return ADM_ECODE_SUCCESSED;
}


static void __adm_dpm_notifyHmiNetException(D32S iResult)
{
    D32S iErrCode = ADM_ECODE_SUCCESSED;

    if (ADM_IS_SUCCESS(iResult))
    {
        log_i("notify hmi network ok");

        // 表示要通知 HMI：网络恢复
        if (DTRUE == g_hmiNetExceptionActive)
        {
            (void)adm_dim_host_downExceptionNotify(adm_soa_getHmiInstanceId(),
                                                   30,
                                                   ADM_ECODE_SUCCESSED,
                                                   &iErrCode);
            g_hmiNetExceptionActive   = DFALSE;
            g_hmiNetExceptionRefBytes = 0;

        }
    }
    else
    {
        log_i("notify hmi network abnormal");

        // 表示要通知 HMI：网络异常
        (void)adm_dim_host_downExceptionNotify(adm_soa_getHmiInstanceId(),
                                               30,
                                               iResult,
                                               &iErrCode);
        g_hmiNetExceptionActive   = DTRUE;
        g_hmiNetExceptionRefBytes = g_lastReportedBytes;   

    }
}


/*!
 *******************************************************************************
 * 速度监控：检测网络异常
 *
 * \note 通过检测下载速度和字节变化判断网络状态
 *       速度为0且下载字节没变化
 *
 * \param[in]   speed       下载速度
 * \param[in]   saveBytes   已下载字节数
 * \retval      void
 *******************************************************************************
 */
static void __adm_dpm_speedMonitor(D32U speed, D64U saveBytes)
{
    if (speed == 0 && saveBytes == g_lastReportedBytes)
    {
        g_uiZeroSpeedCnt++;
        log_w("Download speed is 0 and bytes not changed, zeroSpeedCnt[%u]", g_uiZeroSpeedCnt);
        if (g_uiZeroSpeedCnt >= DPM_ZERO_SPEED_THRESHOLD)
        {
            if (DTRUE != g_hmiNetExceptionActive)
            {
                log_e("Network exception detected by speed monitor, notify HMI");
                __adm_dpm_notifyHmiNetException(ADM_ECODE_NETWORK_SLOW);
            }
        }
    }
    else
    {
        if (g_uiZeroSpeedCnt > 0)
        {
            log_i("Download speed recovered or bytes increased, zeroSpeedCnt reset");
        }
        g_uiZeroSpeedCnt = 0;
    }
}



static void __adm_dpm_tryNotifyHmiNetRecover(D64U currentBytes,
                                             D64U totalBytes)
{
    if (DFALSE == g_hmiNetExceptionActive)
    {
        return;  
    }

    if (currentBytes <= g_hmiNetExceptionRefBytes)
    {
        return;  
    }

    D64U deltaBytes = currentBytes - g_hmiNetExceptionRefBytes;
    D32S percentGain = 0;

    if (totalBytes > 0)
    {
        D32S curPercent  = (D32S)((currentBytes * 100) / totalBytes);
        D32S refPercent  = (D32S)((g_hmiNetExceptionRefBytes * 100) / totalBytes);
        percentGain      = curPercent - refPercent;
    }

    

    if (deltaBytes >= HMI_NET_RECOVER_MIN_BYTES ||
        percentGain >= HMI_NET_RECOVER_MIN_PERCENT)
    {
        // 触发一次网络恢复通知
        __adm_dpm_notifyHmiNetException(ADM_ECODE_SUCCESSED);
        // 重置速度监控计数
        g_uiZeroSpeedCnt = 0;
    }
    else
    {
        log_i("wait network restore");
    }
}

/*!
 *******************************************************************************
 * slave 下载进度通知回调函数
 *
 * \note
 * 
 * \param[in]   fileID  下载文件标识id
 * \param[in]   pstDownloadProgressInfo 下载进度信息
 * \param[in]   priv    私有信息
 * \retval      D32S
 *******************************************************************************
 */
D32S adm_dpm_dimSlaveDownProgressHandle
(
    const DCHAR *fileID, 
    const ADM_DIM_SOA_DPO_DOWNLOAD_PROGRESS_INFO_T *pstDownloadProgressInfo, 
    void *priv
)
{

    static struct timespec stTs = {0};
    ADM_DPS_DOWNPROGRESS_T   stProgress = {0};
    ADM_DOWN_ASYNCTAG_S    *pstDownAsyncTag    = NULL;
    ADM_DPM_DOWNREQPARAM_T *pstDpmDownReqParam = NULL;

    if (ADM_IS_NULL(fileID) || ADM_IS_NULL(pstDownloadProgressInfo))
    {
        log_e("Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }

    if (ADM_IS_NULL(adm_dpm_getDownInfo()->pstDpmDownReqParam))
    {
        log_e("Invalid input param, down reqparam is null");
        return ADM_ECODE_INVALID_PARAM;
    }

    log_i("slave download progress notify, fileID[%s]", fileID);

    pstDpmDownReqParam = (ADM_DPM_DOWNREQPARAM_T *)adm_dpm_getDownInfo()->pstDpmDownReqParam;

    if (ADM_IS_NULL(pstDpmDownReqParam->fileID))
    {
        log_i("Invalid input param, reqparam fileID is null");
        return ADM_ECODE_INVALID_POINTER;
    }

    if (0 != adm_strcmp(fileID, pstDpmDownReqParam->fileID))
    {
        log_e("Invalid download fileID");
        return ADM_ECODE_INVALID_PARAM;
    }

    if ((NULL != pstDpmDownReqParam->fnDownProgressNotify) && 
        (__adm_dpm_isOverNotifyTimeWide(&stTs, NULL) || 
        (pstDownloadProgressInfo->savedSize == pstDownloadProgressInfo->totalSize)))
    {
        //!< 进度融合
        stProgress.speed      = pstDownloadProgressInfo->speed;
        stProgress.totalTime  = 0;
        stProgress.remainTime = pstDownloadProgressInfo->remainTime;
        stProgress.saveBytes  = pstDpmDownReqParam->stPkgAttr.baseSize + pstDownloadProgressInfo->savedSize;
        stProgress.totalBytes = pstDpmDownReqParam->stPkgAttr.wholePkgSize;

        //!< 速度监控：检测网络异常
        __adm_dpm_speedMonitor(stProgress.speed, stProgress.saveBytes);

        //!< 记录最新进度并尝试触发 HMI 网络恢复
        g_lastReportedBytes = stProgress.saveBytes;
        __adm_dpm_tryNotifyHmiNetRecover(stProgress.saveBytes,
                                     stProgress.totalBytes);

        //性能统计
        __adm_dpm_SlaveDownStatisticsHandle(pstDownloadProgressInfo, pstDpmDownReqParam);

        //!< 进度继续上报
        pstDpmDownReqParam->fnDownProgressNotify(&stProgress, &pstDpmDownReqParam->stPkgAttr);
    }

    // 异步转同步 --- launch async cond
    if (NULL != pstDpmDownReqParam->pAsyncTag)
    {
        pstDownAsyncTag = (ADM_DOWN_ASYNCTAG_S *)pstDpmDownReqParam->pAsyncTag;
    
        (void)pthread_mutex_lock(&pstDownAsyncTag->stMutex);
        pstDownAsyncTag->bIsFlush = TRUE;

        (void)pthread_cond_signal(&pstDownAsyncTag->stCond);
        (void)pthread_mutex_unlock(&pstDownAsyncTag->stMutex);
    }

    return ADM_ECODE_SUCCESSED;
}


/*!
 *******************************************************************************
 * hash校验进度通知回调函数
 *
 * \note
 * 
 * \param[in]   fileID  下载文件标识id
 * \param[in]   progress hash校验进度信息
 * \param[in]   priv 私有信息
 * \retval      D32S
 *******************************************************************************
 */
D32S adm_dpm_dimVerifyHashProgressNotify(const DCHAR *pFileID, const D16U iProgress, void *priv)
{
    ADM_DOWN_ASYNCTAG_S    *pstDownAsyncTag    = NULL;
    ADM_DPM_DOWNREQPARAM_T *pstDpmDownReqParam = NULL;

    if (ADM_IS_NULL(pFileID))
    {
        log_e("Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }

    if (ADM_IS_NULL(adm_dpm_getDownInfo()->pstDpmDownReqParam))
    {
        log_e("Invalid input param, down reqparam is null");
        return ADM_ECODE_INVALID_PARAM;
    }

    log_i(" verify hash progress notify, fileID[%s], progress[%d]", pFileID, iProgress);

    pstDpmDownReqParam = (ADM_DPM_DOWNREQPARAM_T *)adm_dpm_getDownInfo()->pstDpmDownReqParam;

    if (0 != adm_strcmp(pFileID, pstDpmDownReqParam->fileID))
    {
        log_e("Invalid download fileID");
        return ADM_ECODE_INVALID_PARAM;
    }

    // 异步转同步 --- launch async cond
    if (NULL != pstDpmDownReqParam->pAsyncTag)
    {
        pstDownAsyncTag = (ADM_DOWN_ASYNCTAG_S *)pstDpmDownReqParam->pAsyncTag;
    
        (void)pthread_mutex_lock(&pstDownAsyncTag->stMutex);
        pstDownAsyncTag->bIsFlush = TRUE;
        (void)pthread_cond_signal(&pstDownAsyncTag->stCond);
        (void)pthread_mutex_unlock(&pstDownAsyncTag->stMutex);
    }

    return ADM_ECODE_SUCCESSED;
}



#if 0
#endif


/*!
 *******************************************************************************
 * master下载通知回调函数
 *
 * \note 
 * 
 * \param[in]   pstNotifyInfo   下载包参数
 * \param[in]   priv            私有信息
 * \retval      none
 *******************************************************************************
 */
static void __adm_dpm_dpoMasterDownNotifyHandle(ADM_DPO_PROGRESS_NOTIFY_T *pstNotifyInfo, void *priv)
{
    ADM_DPS_DOWNPROGRESS_T   stProgress = {0};
    ADM_DPM_DOWNREQPARAM_T *pstDpmDownReqParam = NULL;
    static struct timespec stTs = {.tv_sec = 0, .tv_nsec = 0};

    //!< 无私有属性数据则可直接退出
    if (ADM_IS_NULL(pstNotifyInfo) || ADM_IS_NULL(priv))
    {
        log_e("Invalid input param");
        return;
    }
    
    pstDpmDownReqParam = (ADM_DPM_DOWNREQPARAM_T *)priv;

    if ((NULL != pstDpmDownReqParam->fnDownProgressNotify) && 
        (__adm_dpm_isOverNotifyTimeWide(&stTs, NULL) || 
        (pstNotifyInfo->savedSize == pstNotifyInfo->totalSize)))
    {  
        log_i("DownTime[%d]s, progress[%d].", pstNotifyInfo->totalTime / 1000, 
                      (D32S)((pstNotifyInfo->totalSize != 0) ? ((pstNotifyInfo->savedSize * 100) / pstNotifyInfo->totalSize) : 0));

        //!< 进度融合
        stProgress.speed      = pstNotifyInfo->downSpeed;
        stProgress.totalTime  = pstNotifyInfo->totalTime;
        stProgress.remainTime = pstNotifyInfo->remainTime;
        stProgress.saveBytes  = pstNotifyInfo->savedSize + pstDpmDownReqParam->stPkgAttr.baseSize;
        stProgress.totalBytes = pstDpmDownReqParam->stPkgAttr.wholePkgSize;
        
        //!< 记录最新进度并尝试触发 HMI 网络恢复
        g_lastReportedBytes = stProgress.saveBytes;
        __adm_dpm_tryNotifyHmiNetRecover(stProgress.saveBytes,
                                     stProgress.totalBytes);

        //!< 性能统计
        __adm_dpm_MasterDownStatisticsHandle(pstNotifyInfo, pstDpmDownReqParam);
        
        //!< 进度继续上报
        pstDpmDownReqParam->fnDownProgressNotify(&stProgress, &pstDpmDownReqParam->stPkgAttr);
    }

    return;
}


#if 0
#endif

/*!
 *******************************************************************************
 * 释放DPO下载请求参数结构信息.
 *
 * \note
 *
 * \param[in]   pstDownReqInfo 下载请求参数结构信息
 * \retval      none
 *******************************************************************************
 */
static void __adm_dpm_delDownReqParamInfo
(
    ADM_DPO_DOWNLOAD_REQPARAM_T **pstDownReqInfo
)
{
    if (ADM_IS_NULL(pstDownReqInfo) || ADM_IS_NULL(*pstDownReqInfo))
    {
        return;
    }

    adm_free((*pstDownReqInfo)->url);
    adm_free((*pstDownReqInfo)->filePath);
    adm_free((*pstDownReqInfo)->hash);
    adm_free(*pstDownReqInfo);

    return;
}


/*!
 *******************************************************************************
 * 创建dpo master下载请求参数
 *
 * \note
 *
 * \param[in]   pstDpmDownReqInfo  dpm下载请求参数
 * \param[in]   fnProgressNotify   下载进度通知函数
 * \retval      ADM_DPO_DOWNLOAD_REQPARAM_T*
 *******************************************************************************
 */
static ADM_DPO_DOWNLOAD_REQPARAM_T* __adm_dpm_createDpoMasterDownReqParam
(
    ADM_DPM_DOWNREQPARAM_T             *pstDpmDownReqInfo,
    ADM_DPO_PROGRESS_NOTIFY_CALLBACK_F  fnProgressNotify
)
{

    ADM_DPO_DOWNLOAD_REQPARAM_T *pstDpoDownReqInfo = NULL;

    if (ADM_IS_NULL(pstDpmDownReqInfo))
    {
        log_e("Invalid input param");
        return NULL;
    }
    
    pstDpoDownReqInfo = adm_dpo_newDownReqParamInfo();
    if (ADM_IS_NULL(pstDpoDownReqInfo))
    {
        log_e("Failed to new dpo download param info");
        return NULL;
    }

    pstDpoDownReqInfo->url      = adm_strdup(pstDpmDownReqInfo->url);
    pstDpoDownReqInfo->filePath = adm_strdup(pstDpmDownReqInfo->filePath);
    pstDpoDownReqInfo->hash     = adm_strdup(pstDpmDownReqInfo->hash);
    pstDpoDownReqInfo->fileSize = pstDpmDownReqInfo->size;

    //!< 填充dpo下载请求参数
    pstDpoDownReqInfo->downRetriesCnt = pstDpmDownReqInfo->downRetriesCnt;
    pstDpoDownReqInfo->IsResumeDown   = pstDpmDownReqInfo->isResumeDown;
    pstDpoDownReqInfo->IsUnlimitretry = pstDpmDownReqInfo->isUnlimitretry;
    pstDpoDownReqInfo->eType = ADM_DPO_DOWNLOAD;

    //!< dpm下载参数藏在dpo私有参数中使用
    pstDpoDownReqInfo->priv = pstDpmDownReqInfo;
    pstDpoDownReqInfo->fnProgressNotify = fnProgressNotify;
    pstDpoDownReqInfo->fnDownloadCb = adm_ocm_fileDownloadInterface;

    return pstDpoDownReqInfo;
}

#if 0
#endif

/*!
 *******************************************************************************
 * OCS下载进度回调句柄.
 *
 * \note 
 *
 * \param[in] pstResult   下载进度信息.
 * \param[in] storePath   下载的文件存储路径(可选字段，slave侧下载时必填)
 * \param[in] pstPkgAttr  下载文件属性.
 * \retval      none
 *******************************************************************************
 */
void adm_dpm_downResultHandler
(
    ADM_DPS_DOWNRESULT_T *pstResult, 
    const DCHAR          *storePath, 
    ADM_DPS_PKGATTR_T    *pstPkgAttr
)
{
    D32S iRet = ADM_ECODE_SUCCESSED;

    if (ADM_IS_NULL(pstResult) || ADM_IS_NULL(pstPkgAttr))
    {
        log_e("Invalid input param");
        return;
    }
    log_d("===> stResult.failReason[%s]", ADM_PRINT_STR(pstResult->failReason));

    iRet = adm_dps_updateDownPackageResult(pstPkgAttr, pstResult, storePath);
    if (ADM_IS_ERR(iRet))
    {
        log_e("Failed to update down package result, ecuReqid[%s], ecuSwDid[%s], fileName[%s], err[%d]", 
                ADM_PRINT_STR(pstPkgAttr->ecuReqid), 
                ADM_PRINT_STR(pstPkgAttr->ecuSwDid), 
                ADM_PRINT_STR(pstPkgAttr->fileName), 
                iRet);
    }

    //!< 停止下载过程网络类型监控
    if(FALSE == gstDownAsyncTag.bHasResult)
    {
        (void)pthread_mutex_lock(&gstDownAsyncTag.stMutex);
        gstDownAsyncTag.bHasResult = TRUE;
        (void)pthread_cond_signal(&gstDownAsyncTag.stCond);
        (void)pthread_mutex_unlock(&gstDownAsyncTag.stMutex);
    }

    //!< 上报下载结果
    adm_dpm_downloadResultReportProc(pstPkgAttr, pstResult);

    return;
}

/*!
 *******************************************************************************
 * OCS下载进度回调句柄.
 *
 * \note 
 *
 * \param[in] pstProgress   下载进度信息.
 * \param[in] pstPkgAttr    下载文件属性.
 * \retval      none
 *******************************************************************************
 */
void adm_dpm_downProgressHandler
(
    ADM_DPS_DOWNPROGRESS_T *pstProgress, 
    ADM_DPS_PKGATTR_T      *pstPkgAttr
)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    ADM_DPM_DOWNPROGRESSRPTNODE_T *pstDownProgressRptInfo = NULL;

    pstDownProgressRptInfo = adm_dpm_newDownProgressReportQueueNode();
    if (ADM_IS_NULL(pstDownProgressRptInfo))
    {
        log_e("Failed to new down progress report queue node, err[%s]", ADM_ECODE_NO_MEMORY);
        return;
    }

    pstDownProgressRptInfo->ecuReqid = ((NULL != pstPkgAttr->ecuReqid) ? (adm_strdup(pstPkgAttr->ecuReqid)) : (NULL));
    pstDownProgressRptInfo->ecuSwDid = ((NULL != pstPkgAttr->ecuSwDid) ? (adm_strdup(pstPkgAttr->ecuSwDid)) : (NULL));
    pstDownProgressRptInfo->fileName = ((NULL != pstPkgAttr->fileName) ? (adm_strdup(pstPkgAttr->fileName)) : (NULL));
    pstDownProgressRptInfo->enPkgFormat = pstPkgAttr->enPkgFormat;
    (void)adm_memcpy(&pstDownProgressRptInfo->stProgress, pstProgress, 
               sizeof(*pstProgress), sizeof(pstDownProgressRptInfo->stProgress));

    //!< 更新进度信息到engine层
    iRet = adm_dps_updateDownPackageProgress(pstPkgAttr, pstProgress);
    if (ADM_IS_ERR(iRet))
    {
        log_e("Failed to update down package progress, ecuReqid[%s], ecuSwDid[%s], fileName[%s], err[%d]", 
               ADM_PRINT_STR(pstPkgAttr->ecuReqid), 
               ADM_PRINT_STR(pstPkgAttr->ecuSwDid), 
               ADM_PRINT_STR(pstPkgAttr->fileName), 
               iRet);
    }

    //!< 添加到上报进度队列
    iRet = adm_dpm_addDownProgress2ReportQueue(pstDownProgressRptInfo);
    if (ADM_IS_ERR(iRet))
    {
        adm_dpm_delDownProgressReportQueueNode(&pstDownProgressRptInfo);
        log_e("Failed to add down package progress, ecuReqid[%s], ecuSwDid[%s], fileName[%s], err[%d]", 
               ADM_PRINT_STR(pstPkgAttr->ecuReqid), 
               ADM_PRINT_STR(pstPkgAttr->ecuSwDid), 
               ADM_PRINT_STR(pstPkgAttr->fileName), 
               iRet);
    }

    return;
}


#if 0
#endif


/*!
 *******************************************************************************
 * 执行slave下载升级包
 *
 * \note        转DIM通知slave下载
 * 
 * \param[in]   pcDownInstId        下载实例id
 * \param[in]   pstDpmDownReqParam  dpm下载请求参数信息
 * \retval      D32S
 *******************************************************************************
 */
static D32S __adm_dpm_doSlaveDownload
(
    const DCHAR                  *pcDownInstId,
    ADM_DPM_DOWNREQPARAM_T *pstDpmDownReqParam
)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    D32S iMsgArrive = ADM_ECODE_SUCCESSED;
    ADM_DPS_DOWNRESULT_T *pstResult        = NULL;
    ADM_DOWN_ASYNCTAG_S   *pstDownAsyncTag = NULL;
    DCHAR szEventBuf[ADM_COM_LEN_256] = {0};  //mod by wl 20220321
    struct timespec tsNowbegin;
    struct timespec tsNowend;
    DBOOL bIsNeedRetry = DFALSE;
    D32S iErrCode = ADM_ECODE_FAILED;
    D32U uiNetTypeCnt = 0;

    if (ADM_IS_NULL(pcDownInstId) || ADM_IS_NULL(pstDpmDownReqParam))
    {
        log_e("Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }

    //mod by wl 20220321
    (void)adm_memzero(szEventBuf, sizeof(szEventBuf));
    (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), 
                "download DownInstId[%s] fileID[%s], url[%s], hash[%s], fileSize[%lld], isResumeDown[%d], downRetriesCnt[%d].", 
                pcDownInstId,
                pstDpmDownReqParam->fileID, 
                pstDpmDownReqParam->url, 
                pstDpmDownReqParam->hash, 
                pstDpmDownReqParam->size,
                pstDpmDownReqParam->isResumeDown,
                pstDpmDownReqParam->downRetriesCnt);
    adm_dpm_addEventAndReport(ADM_COM_MOD_DPM, FALSE, ADM_EVENT_ID_DI_013, ADM_OCS_EVENT_LEVEL_TRACE, "vehicle use slave download package.", szEventBuf);

    pstDownAsyncTag = __adm_dpm_createDownAsyncTagInfo();
    if (ADM_IS_NULL(pstDownAsyncTag))
    {
        log_e("Failed to create down async tag info");
        return ADM_ECODE_NO_MEMORY;
    }
    
    pstDpmDownReqParam ->pAsyncTag = (void *)pstDownAsyncTag;

    (void)pthread_rwlock_rdlock(&adm_dpm_getDownInfo()->stRWlock);
    adm_dpm_getDownInfo()->pstDpmDownReqParam = (ADM_DPM_DOWNREQPARAM_T *)pstDpmDownReqParam;
    (void)pthread_rwlock_unlock(&adm_dpm_getDownInfo()->stRWlock);

    // 记录下载的开始时间
    (void)clock_gettime(CLOCK_MONOTONIC, &tsNowbegin);

    do
    {
        if (DTRUE == bIsNeedRetry)
        {
            pstDownAsyncTag->bHasResult = FALSE;
            pstDownAsyncTag->bIsFlush = FALSE;
            pstDownAsyncTag->stResult.result = ADM_ECODE_SUCCESSED;
            bIsNeedRetry = DFALSE;
        }

        do 
        {
            //!< 异步请求接口
            iRet = adm_dim_dpo_requestRemoteDownloadPackage((D16U)htoi(pcDownInstId), 30, pstDpmDownReqParam->fileID, 
                            pstDpmDownReqParam->url, 
                            pstDpmDownReqParam->size,
                            pstDpmDownReqParam->hash,
                            pstDpmDownReqParam->networkRetriesCnt,
                            &iMsgArrive);
            if (ADM_IS_ERR(iRet) || ADM_IS_ERR(iMsgArrive))
            {
                log_e("Failed to request remote download package, err[%d], msgArrive[%d]", iRet, iMsgArrive);

                /** 从控端下载包已经运行，则直接返回成功 */
                if (ADM_ECODE_PTHREAD_ALREADY_RUNNING == iMsgArrive)
                {
                    log_d("Slave download package already running, in fileID[%s]", pstDpmDownReqParam->fileID);
                }
                else
                {
                    iRet = (ADM_IS_ERR(iMsgArrive) ? (ADM_ECODE_DOWN_MSG_UNREACHABLE) : (iRet));

                    //mod by wl 20220321
                    (void)adm_memzero(szEventBuf, sizeof(szEventBuf));
                    (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), 
                            "Failed to request remote download package, in fileID[%s], err[%d]", pstDpmDownReqParam->fileID, iRet);
                    break;
                }
            }

            //!< 异步判断是否超时
            iRet = adm_dpm_waitAsyncResult(pstDownAsyncTag);
            if (ADM_IS_ERR(iRet))
            {
                log_e("Wait slave download result time out, in fileID[%s], err[%d]", pstDpmDownReqParam->fileID, iRet);
                break;
            }

        } while (FALSE);


        //!< 异步请求响应失败
        if (ADM_IS_ERR(iRet))
        {
            if (NULL != pstDpmDownReqParam->fnDownResultNotify)
            {            
                pstResult = adm_dpm_createDownResultInfo(iRet, &pstDpmDownReqParam->stPkgAttr);
                
                pstDpmDownReqParam->fnDownResultNotify(pstResult, NULL, &pstDpmDownReqParam->stPkgAttr);
                adm_dpm_freeDownResultInfo(pstResult);    
            }
        }
        else
        {
            //!< 异步请求响应成功，则将最终的结果从结果回调中同步过来
            iRet = pstDownAsyncTag->stResult.result;
            if (ADM_IS_ERR(iRet) && ADM_ECODE_NET_CHG_PAUSE_DOWNLOAD != iRet &&
                ADM_ECODE_DOWN_COND_ABNORMAL_PAUSE_DOWNLOAD != iRet)
            {
                log_i("Current download result[%d].", iRet);
                /** 下载失败进行无限重试, 用任务有效性进行打断*/
                iRet = adm_dpm_taskVerify();
                if (ADM_ECODE_UPGRADE_TASK_STOPED == iRet)
                {
                    log_i("Task is stopped.");
                    break;
                }
                else 
                {
                    log_i("Retry slave download.");
                    if (0 == uiNetTypeCnt)
                    {
                        __adm_dpm_notifyHmiNetException(ADM_ECODE_NETWORK_SLOW);
                    }
                    bIsNeedRetry = DTRUE;
                    uiNetTypeCnt++;
                    sleep(ADM_DPM_DOWNLOAD_RETRY_INTERVAL);
                }
            }
        }
    } while (DTRUE == bIsNeedRetry);

    if (uiNetTypeCnt > 0)
    {
        __adm_dpm_notifyHmiNetException(ADM_ECODE_SUCCESSED);
        sleep(1);
    }

    __adm_dpm_destroyDownAsyncTagInfo(pstDownAsyncTag);
    pstDpmDownReqParam->pAsyncTag = NULL;

    //mod by wl 20220321
    if (ADM_IS_ERR(iRet))
    {
        adm_dpm_addEventAndReport(ADM_COM_MOD_DPM, TRUE, ADM_EVENT_ID_DI_014, ADM_OCS_EVENT_LEVEL_ERROR, "vehicle use slave download package failed!", szEventBuf);
    }
    else
    {
        // 记录下载包的结束时间
        (void)clock_gettime(CLOCK_MONOTONIC, &tsNowend);
        (void)adm_memzero(szEventBuf, sizeof(szEventBuf));
        (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), "STATSPACK downOnceTime:ecuReqid[%s], ecuSwDid[%s], pkgType[%s], fileName[%s], downTime[%d]%s, fileSize[%lld], downType[2](1: master, 2: slave)",
        pstDpmDownReqParam->stPkgAttr.ecuReqid, pstDpmDownReqParam->stPkgAttr.ecuSwDid,
        (pstDpmDownReqParam->stPkgAttr.isRollback == FALSE ? "part-dstPkg" : "part-srcPkg"),
        pstDpmDownReqParam->stPkgAttr.fileName,
        ((tsNowend.tv_sec - tsNowbegin.tv_sec) < 1 ? (tsNowend.tv_nsec / (1000 * 1000)) - (tsNowbegin.tv_nsec / (1000 * 1000)) : (tsNowend.tv_sec - tsNowbegin.tv_sec)),
        ((tsNowend.tv_sec - tsNowbegin.tv_sec) < 1 ? "ms" : "s"), pstDpmDownReqParam->size);
        adm_dpm_addEventAndReport(ADM_COM_MOD_DPM, FALSE,ADM_EVENT_ID_DL_002, ADM_OCS_EVENT_LEVEL_INFO, szEventBuf, NULL);

        adm_dpm_addEventAndReport(ADM_COM_MOD_DPM, FALSE, ADM_EVENT_ID_DI_015, ADM_OCS_EVENT_LEVEL_DEBUG, "vehicle use slave download package successed.", szEventBuf);
    }

    return iRet;
}


/*!
 *******************************************************************************
 * 执行master下载升级包
 *
 * \note        master通过dpo下载
 * 
 * \param[in]   pstDpmDownReqParam  dpm下载请求参数信息
 * \retval      D32S
 *******************************************************************************
 */
static D32S __adm_dpm_doMasterDownload
(
    ADM_DPM_DOWNREQPARAM_T *pstDpmDownReqParam
)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    ADM_DPS_DOWNRESULT_T *pstResult = NULL;
    ADM_DPO_DOWNLOAD_REQPARAM_T *pstDPODownReqInfo = NULL;
    DCHAR szEventBuf[ADM_COM_LEN_256] = {0};  //mod by wl 20220321
    DBOOL bIsNeedRetry = DFALSE;
    D32U uiNetTypeCnt = 0;
    D32S iErrCode = ADM_ECODE_FAILED;

    if (ADM_IS_NULL(pstDpmDownReqParam))
    {
        log_e("Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }
    
    //mod by wl 20220321
    (void)adm_memzero(szEventBuf, sizeof(szEventBuf));
    (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), 
                "download fileID[%s], url[%s], hash[%s], fileSize[%lld], isResumeDown[%d], downRetriesCnt[%d].", 
                pstDpmDownReqParam->fileID, 
                pstDpmDownReqParam->url, 
                pstDpmDownReqParam->hash, 
                pstDpmDownReqParam->size,
                pstDpmDownReqParam->isResumeDown,
                pstDpmDownReqParam->downRetriesCnt);
    adm_dpm_addEventAndReport(ADM_COM_MOD_DPM, FALSE, ADM_EVENT_ID_DI_010, ADM_OCS_EVENT_LEVEL_TRACE, "vehicle use master download package.", szEventBuf);

    do
    {
        bIsNeedRetry = DFALSE;

        do
        {        
            if (ADM_IS_NULL(pstDPODownReqInfo))
            {
                pstDPODownReqInfo = __adm_dpm_createDpoMasterDownReqParam(pstDpmDownReqParam, 
                                        __adm_dpm_dpoMasterDownNotifyHandle);
                if (ADM_IS_NULL(pstDPODownReqInfo))
                {
                    iRet = ADM_ECODE_NO_MEMORY;
                    log_e("Failed to create dpo master down req param, err[%d]", iRet);

                    //mod by wl 20220321
                    (void)adm_memzero(szEventBuf, sizeof(szEventBuf));
                    (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), 
                            "Failed to create dpo master down req param, in fileID[%s], err[%d]", pstDpmDownReqParam->fileID, iRet);
                    break;
                }
            }

            iRet = adm_dpo_doDownloadFile(pstDPODownReqInfo);
            if (ADM_IS_ERR(iRet))
            {
                log_e("Failed to download ecu fileID[%s], err:[%d]", pstDpmDownReqParam->fileID, iRet);
                //mod by wl 20220321
                (void)adm_memzero(szEventBuf, sizeof(szEventBuf));
                (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), "Failed to download ecu fileID[%s], err[%d]", pstDpmDownReqParam->fileID, iRet);
                break;
            }
        } while (FALSE);

        if (ADM_IS_ERR(iRet) && ADM_ECODE_NET_CHG_PAUSE_DOWNLOAD != iRet &&
            ADM_ECODE_DOWN_COND_ABNORMAL_PAUSE_DOWNLOAD != iRet)
        {
            log_i("Current download result[%d].", iRet);
            /** 下载失败进行无限重试, 用任务有效性进行打断*/
            iRet = adm_dpm_taskVerify();
            if (ADM_ECODE_UPGRADE_TASK_STOPED == iRet)
            {
                log_i("Task is stopped.");
                break;
            }
            else
            {
                log_i("Retry master download.");
                if (0 == uiNetTypeCnt)
                {
                    /** 通知HMI网络差 */
                    __adm_dpm_notifyHmiNetException(ADM_ECODE_NETWORK_SLOW);
                }
                bIsNeedRetry = DTRUE;
                uiNetTypeCnt++;
                sleep(ADM_DPM_DOWNLOAD_RETRY_INTERVAL);
            }
        }
    } while (DTRUE == bIsNeedRetry);

    if (uiNetTypeCnt > 0)
    {
        __adm_dpm_notifyHmiNetException(ADM_ECODE_SUCCESSED);
        sleep(1);
    }

    __adm_dpm_delDownReqParamInfo(&pstDPODownReqInfo);

    //!< 更新下载结果
    if (NULL != pstDpmDownReqParam->fnDownResultNotify)
    {
        pstResult = adm_dpm_createDownResultInfo(iRet, &pstDpmDownReqParam->stPkgAttr);
        
        pstDpmDownReqParam->fnDownResultNotify(pstResult, NULL, &pstDpmDownReqParam->stPkgAttr);
        adm_dpm_freeDownResultInfo(pstResult);
    }

    //mod by wl 20220321
    if (ADM_IS_ERR(iRet))
    {
        adm_dpm_addEventAndReport(ADM_COM_MOD_DPM, TRUE, ADM_EVENT_ID_DI_011, ADM_OCS_EVENT_LEVEL_ERROR, "vehicle use master download package failed!", szEventBuf);
    }
    else
    {
        adm_dpm_addEventAndReport(ADM_COM_MOD_DPM, FALSE, ADM_EVENT_ID_DI_012, ADM_OCS_EVENT_LEVEL_DEBUG, "vehicle use master download package successed.", szEventBuf);
    }

    return iRet;
}

#if 0
#endif

/*!
 *******************************************************************************
 * 执行dpm下载升级包
 *
 * \note 网络类型监控处理：
 *  当常规下载 或者 救援升级任务时，需要监控网络类型，当网络类型变更时，需要无限次重试
 *  其他升级类型无限特殊处理
 * 
 * \param[in]   pcDownInstId        下载实例id
 * \param[in]   pstDpmDownReqParam  下载请求信息
 * \retval      D32S
 *******************************************************************************
 */
D32S adm_dpm_Download
(
    const DCHAR                  *pcDownInstId,
    ADM_DPM_DOWNREQPARAM_T *pstDpmDownReqParam
)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    D32S iErrcode = ADM_ECODE_SUCCESSED;
    D32S iNetErrcode = ADM_ECODE_SUCCESSED;
    D32S iCnt = 0;
    D32S NetTypeCnt = 0;
    D32S idownNetworkType = 0;

    if (ADM_IS_NULL(pcDownInstId) || ADM_IS_NULL(pstDpmDownReqParam))
    {
        log_e("Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }

    //获取下载的网络类型
    iRet = adm_dps_getDownNetworkType(&idownNetworkType);
    if (ADM_IS_ERR(iRet))
    {
        log_e("Failed to get DownNetworkType, err[%d]", iRet);
        return iRet; 
    }

    if (ADM_DPM_DOWNLOAD_NETWORK_TYPE_WIFI == idownNetworkType)
    {
        iNetErrcode = ADM_ECODE_WIFI_NET_VERIFY_FAILED;
    }
    else
    {
        iNetErrcode = ADM_ECODE_4G_NET_VERIFY_FAILED;
    }

    do
    {
        iRet = dpmStartDownNetTypeMonitor();
        if (ADM_IS_ERR(iRet))
        {
            if (0 == NetTypeCnt)
            {
                /*下载暂停停止进度上报*/
                adm_dpm_setUimDownloadStatus(3);

                /**通知HMI*/
                (void)adm_dim_host_downExceptionNotify(adm_soa_getHmiInstanceId(), 30, iNetErrcode, &iErrcode);

                /**通知云端*/
                (void)adm_dpm_vehicleStateReport2Cloud(ADM_IRM_REPORT_SERVER_STATUS_Task_Download_Paused, adm_dpm_errorCode2String(iRet));
            }
            log_e("start download network type monitor Abnormal, NetTypeCnt[%d], err:[%d]", NetTypeCnt,iRet);
            sleep(10);
            NetTypeCnt++;
            continue;
        }
        else
        {
            if (0 != NetTypeCnt) /**后续恢复下载后，需要通知云端下载中*/
            {
                /*网络恢复后，恢复上报*/
                adm_dpm_setUimDownloadStatus(0);

                (void)adm_dim_host_downExceptionNotify(adm_soa_getHmiInstanceId(), 30, iRet, &iErrcode);

                (void)adm_dpm_vehicleStateReport2Cloud(ADM_IRM_REPORT_SERVER_STATUS_Task_Download_Ing, adm_dpm_errorCode2String(iRet));
                NetTypeCnt = 0;
            }
        }
        g_downloadIngInstId = (D32S)htoi(pcDownInstId);

        log_i("[ABUP-Procedure] download package Start,downInstId[%s],fileID[%s]", pcDownInstId, pstDpmDownReqParam->fileID);
        if (adm_dpm_getDownInfo()->s32UCMDownInstId == (D32S)htoi(pcDownInstId))
        {
            iRet = __adm_dpm_doMasterDownload(pstDpmDownReqParam);
        }
        else
        {
            // do
            // {
                iRet = __adm_dpm_doSlaveDownload(pcDownInstId, pstDpmDownReqParam);
                // if (ADM_ECODE_MSG_WAIT_TIMEOUT == iRet)
                // {
                //     iCnt++;
                //     log_e("do slave download failed retry:%d,err[%d]",iCnt,iRet);
                // }
                // else
                // {
                //     break;
                // }
            // } while (iCnt < 3);
        }
        log_i("[ABUP-Procedure] download package End,ret[%d]",iRet);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to download, downInstId[%s], fileID[%s], err:[%d]", 
                                    pcDownInstId, pstDpmDownReqParam->fileID, iRet);
        }

        if (ADM_ECODE_NET_CHG_PAUSE_DOWNLOAD != iRet)
        {
            g_netWorkTypeAbnormalFlag = FALSE;
        }
        
        log_d("Download fileID[%s] ok, downInstId[%s]", ADM_PRINT_STR(pstDpmDownReqParam->fileID), pcDownInstId);

    } while (DTRUE == g_netWorkTypeAbnormalFlag);

    return iRet;
}

#define BUFFER_SIZE 1024
/**
 * @brief 从usb拷贝文件到本地（包括拷贝过程的进度和结果通知）
 * 
 * @param[in] pSrcPath     描述: 待拷贝文件绝对路径 \n
 * 范围值: 指针 \n
 * 单位: N/A \n
 * 
 * @param[in] pDstPath     描述: 本地文件绝对路径 \n
 * 范围值: 指针 \n
 * 单位: N/A \n
 * 
 * @param[in] pstDpmDownReqParam     描述: 下载请求信息 \n
 * 范围值: 指针 \n
 * 单位: N/A \n
 * 
 */
static D32S dpmCopyUsbFile(const DCHAR* pSrcPath, const DCHAR* pDstPath, ADM_DPM_DOWNREQPARAM_T* pstDpmDownReqParam)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    DCHAR aFileData[BUFFER_SIZE +1] = {0}; ///< 文件按照1K分段拷贝
    FILE* pFileIn = NULL;
    FILE* pFileOut = NULL;
    DSIZE uiBytesRead = 0;
    DSIZE uiBytesWritten = 0;
    D64U uiSavedSize = 0;
    D64U uiTotalSize = 0;
    D64U uiTmpSize = 0;
    D32U uiWide = 0;
    struct stat fileStats = {0};
    static struct timespec stTs = {.tv_sec = 0, .tv_nsec = 0};
    ADM_DPS_DOWNPROGRESS_T stProgress = {0};
    ADM_DPO_PROGRESS_NOTIFY_T stCurProgress = {0};
    ADM_DPS_DOWNRESULT_T* pstResult = NULL;

    if ((NULL == pSrcPath) || (NULL == pDstPath) || (NULL == pstDpmDownReqParam))
    {
        log_e( "Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }

    log_d("copy usb file [%s] to [%s]", pSrcPath, pDstPath);

    if (0 != access(pSrcPath, F_OK))
    {
        log_e( "src file [%s] not exist.", pSrcPath);
        return ADM_ECODE_FILE_NOT_EXIST;
    }

    iRet = stat(pSrcPath, &fileStats);
    if (iRet != 0) 
    {
        log_e("state file [%s] failed.", pSrcPath);
        return ADM_ECODE_OPEN_FILE_FAILED;
    }
    uiTotalSize = (D64U)fileStats.st_size;
    log_i("file:[%s] total size:[%llu]", pSrcPath, uiTotalSize);

    pFileIn = fopen(pSrcPath, "rb");
    if (NULL == pFileIn)
    {
        log_e( "open file %s, error %s", pSrcPath, strerror(errno));
        return ADM_ECODE_OPEN_FILE_FAILED;
    }

    pFileOut = fopen(pDstPath, "wb+");
    if (NULL == pFileOut) 
    {
        fclose(pFileIn);
        log_e( "open file %s, error %s", pDstPath, strerror(errno));
        return ADM_ECODE_OPEN_FILE_FAILED;
    }

    (void)adm_memset(aFileData, '\0', sizeof(aFileData), sizeof(aFileData));

    do
    {
        while ((uiBytesRead = fread(aFileData, 1, BUFFER_SIZE, pFileIn)) > 0)
        {
            uiBytesWritten = fwrite(aFileData, 1, uiBytesRead, pFileOut);
            if (uiBytesWritten < uiBytesRead)
            {
                log_e("write byte lower than read byte");
                iRet = ADM_ECODE_WRITE_FILE_FAILED;
                break;
            }

            uiSavedSize += (D64U)uiBytesWritten;
            uiTmpSize += (D64U)uiBytesWritten;

            if ((NULL != pstDpmDownReqParam->fnDownProgressNotify) && 
            (__adm_dpm_isOverNotifyTimeWide(&stTs, &uiWide) || 
            (uiSavedSize == uiTotalSize)))
            {
                //!< 进度融合
                if (uiWide > 0)
                {
                    stProgress.speed = (D32S)(uiTmpSize / uiWide);
                }
                if (stProgress.speed > 0)
                {
                    stProgress.totalTime  = uiTotalSize / stProgress.speed;
                    stProgress.remainTime = (uiTotalSize - uiSavedSize) / stProgress.speed;    
                }
                stProgress.saveBytes  = pstDpmDownReqParam->stPkgAttr.baseSize + uiSavedSize;
                stProgress.totalBytes = pstDpmDownReqParam->stPkgAttr.wholePkgSize;

                //性能统计
                stCurProgress.downSpeed = stProgress.speed;
                stCurProgress.remainTime = stProgress.remainTime;
                stCurProgress.totalTime = stProgress.totalTime;
                stCurProgress.savedSize = uiSavedSize;
                stCurProgress.totalSize = uiTotalSize;
                __adm_dpm_MasterDownStatisticsHandle(&stCurProgress, pstDpmDownReqParam);

                //!< 进度继续上报
                pstDpmDownReqParam->fnDownProgressNotify(&stProgress, &pstDpmDownReqParam->stPkgAttr);

                uiTmpSize = 0;
            }
        }
    } while (FALSE);

    pstResult = adm_dpm_createDownResultInfo(iRet, &pstDpmDownReqParam->stPkgAttr);
    if (NULL != pstResult)
    {
        pstResult->eDownState = (ADM_IS_SUCCESS(iRet) ? (ADM_DPS_DOWNPKG_STATE_FINISH) : (ADM_DPS_DOWNPKG_STATE_FAILED));
        pstDpmDownReqParam->fnDownResultNotify(pstResult, pDstPath, &pstDpmDownReqParam->stPkgAttr);
        adm_dpm_freeDownResultInfo(pstResult);
    }

    fclose(pFileIn);
    fclose(pFileOut);

    return iRet;
}

/*!
 *******************************************************************************
 * 执行dpm下载升级包(从usb获取升级包)
 *
 * \note usb下载不进行重试,失败就没必要重试了
 * 
 * \param[in]   pstDpmDownReqParam  下载请求信息
 * \retval      D32S
 *******************************************************************************
 */
static D32S downloadMasterFromUsb(ADM_DPM_DOWNREQPARAM_T *pstDpmDownReqParam, DCHAR *pcFileName)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    DCHAR szEventBuf[ADM_COM_LEN_256] = {0};
    ADM_DPO_DOWNLOAD_REQPARAM_T* pstDPODownReqInfo = NULL;
    ADM_DPS_DOWNRESULT_T *pstResult = NULL;
    DCHAR *pcUsbDownloadDir = NULL;
    DCHAR szFileUsbPath[ADM_COM_LEN_256] = {0};

    if (ADM_IS_NULL(pstDpmDownReqParam))
    {
        log_e("Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }

    do
    {
        log_i("Download package from usb.");

        //iRet = dpmStartDownNetTypeMonitor();
        //if (ADM_IS_ERR(iRet))
        //{
        //    log_e("Failed to start download network type monitor, err:[%d]", iRet);
        //    return iRet; 
        //}
        
        (void)adm_memzero(szEventBuf, sizeof(szEventBuf));
        (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), 
                    "download fileID[%s], url[%s], hash[%s], fileSize[%lld], isResumeDown[%d], downRetriesCnt[%d].", 
                    pstDpmDownReqParam->fileID, 
                    pstDpmDownReqParam->url, 
                    pstDpmDownReqParam->hash, 
                    pstDpmDownReqParam->size,
                    pstDpmDownReqParam->isResumeDown,
                    pstDpmDownReqParam->downRetriesCnt);
        adm_dpm_addEventAndReport(ADM_COM_MOD_DPM, FALSE, ADM_EVENT_ID_DI_010, ADM_OCS_EVENT_LEVEL_TRACE, "vehicle use master download usb package.", szEventBuf);

        pstDPODownReqInfo = __adm_dpm_createDpoMasterDownReqParam(pstDpmDownReqParam, 
                                __adm_dpm_dpoMasterDownNotifyHandle);
        if (ADM_IS_NULL(pstDPODownReqInfo))
        {
            iRet = ADM_ECODE_NO_MEMORY;
            log_e("Failed to create dpo master down req param, err[%d]", iRet);

            //mod by wl 20220321
            (void)adm_memzero(szEventBuf, sizeof(szEventBuf));
            (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), 
                    "Failed to create dpo master down req param, in fileID[%s], err[%d]", pstDpmDownReqParam->fileID, iRet);
            break;
        }

        /** 创建目录，清理缓存文件 */
        iRet = adm_com_createFileDirAndFile(pstDpmDownReqParam->filePath, TRUE);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to check/create file, filePath[%s], err[%d]", pstDpmDownReqParam->filePath, iRet);
            break;
        }

        /** 获取USB升级包下载目录 */
        pcUsbDownloadDir = adm_cvs_getUsbDownloadDir();
        if (ADM_IS_NULL(pcUsbDownloadDir))
        {
            log_e("adm_cvs_getUsbDownloadDir failed");
            break;
        }

        /** 拼接u盘升级包的绝对路径 */
        adm_snprintf(szFileUsbPath, sizeof(szFileUsbPath), "%s%s", pcUsbDownloadDir, pcFileName);

        iRet = dpmCopyUsbFile(szFileUsbPath, pstDpmDownReqParam->filePath, pstDpmDownReqParam);
        if (ADM_IS_ERR(iRet))
        {
            if (FALSE == adm_dpm_isUsbDirExist())
            {
                iRet = ADM_ECODE_USB_DIR_IS_NOT_EXIST;
            }
            else if (ADM_ECODE_FILE_NOT_EXIST == iRet || ADM_ECODE_OPEN_FILE_FAILED == iRet)
            {
                iRet = ADM_ECODE_DOWNLOAD_NO_PACKAGE_FAILED;
            }
            log_e("Copy usb file from [%s] to [%s] failed.", szFileUsbPath, pstDpmDownReqParam->filePath);
            break;
        }

        iRet = adm_dpo_checkDownloadFile(pstDPODownReqInfo);
        if (ADM_IS_ERR(iRet))
        {
            (void)adm_fremove(pstDPODownReqInfo->filePath);
            log_e("Failed to check download file[%s], err[%d]", pstDPODownReqInfo->filePath, iRet);
            break;
        }

        log_d("Download fileID[%s] ok,", ADM_PRINT_STR(pstDpmDownReqParam->fileID));
    } while (FALSE);

    ///< 更新下载结果,通知云和uim
    if (NULL != pstDpmDownReqParam->fnDownResultNotify)
    {
        pstResult = adm_dpm_createDownResultInfo(iRet, &pstDpmDownReqParam->stPkgAttr);
        
        pstDpmDownReqParam->fnDownResultNotify(pstResult, (const DCHAR*)pstDpmDownReqParam->filePath, &pstDpmDownReqParam->stPkgAttr);
        adm_dpm_freeDownResultInfo(pstResult);
    }

    if (ADM_IS_ERR(iRet))
    {
        adm_dpm_addEventAndReport(ADM_COM_MOD_DPM, TRUE, ADM_EVENT_ID_DI_011, ADM_OCS_EVENT_LEVEL_ERROR, "vehicle use master usb download package failed!", szEventBuf);
    }
    else
    {
        adm_dpm_addEventAndReport(ADM_COM_MOD_DPM, FALSE, ADM_EVENT_ID_DI_012, ADM_OCS_EVENT_LEVEL_DEBUG, "vehicle use master usb download package successed.", szEventBuf);
    }

    __adm_dpm_delDownReqParamInfo(&pstDPODownReqInfo);
    adm_free(pcUsbDownloadDir);

    return iRet;
}

/**
 * @brief 从usb下载升级包
 * @note 从控下载升级包
 * @param in const DCHAR *pcDownInstId   下载实例ID
 * @param in ADM_DPM_DOWNREQPARAM_T *pstDpmDownReqParam  下载请求信息
 * @param in DCHAR *pcFileName           下载文件名
 * @retval D32S 0:成功, 其他:失败
 */
static D32S downloadSlaveFromUsb(const DCHAR *pcDownInstId,
                                 ADM_DPM_DOWNREQPARAM_T *pstDpmDownReqParam,
                                 DCHAR *pcFileName)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    D32S iMsgArrive = ADM_ECODE_SUCCESSED;
    ADM_DPS_DOWNRESULT_T *pstResult        = NULL;
    ADM_DOWN_ASYNCTAG_S   *pstDownAsyncTag = NULL;
    DCHAR szEventBuf[ADM_COM_LEN_256] = {0};
    DCHAR szUsbUrl[ADM_COM_LEN_256] = {0};
    DCHAR *pcEwebAddress = NULL;
    struct timespec tsNowbegin;
    struct timespec tsNowend;

    if (ADM_IS_NULL(pcDownInstId) || ADM_IS_NULL(pstDpmDownReqParam) || ADM_IS_NULL(pcFileName))
    {
        log_e("Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }

    /** 获取eweb地址头 */
    pcEwebAddress = (DCHAR *)adm_cfg_iniParserGetString(ADM_CFG_EWEB_ADDRESS_HEADER, ADM_CFG_EWEB_ADDRESS_HEADER_DEFAULT_VAL);

    /** 拼接eweb地址头和文件名 */
    adm_snprintf(szUsbUrl, sizeof(szUsbUrl), "%s%s", pcEwebAddress, pcFileName);

    (void)adm_memzero(szEventBuf, sizeof(szEventBuf));
    (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), 
                "download DownInstId[%s] fileID[%s], szUsbUrl[%s], hash[%s], fileSize[%lld], isResumeDown[%d], downRetriesCnt[%d].", 
                pcDownInstId,
                pstDpmDownReqParam->fileID, 
                szUsbUrl, 
                pstDpmDownReqParam->hash, 
                pstDpmDownReqParam->size,
                pstDpmDownReqParam->isResumeDown,
                pstDpmDownReqParam->downRetriesCnt);
    adm_dpm_addEventAndReport(ADM_COM_MOD_DPM, FALSE, ADM_EVENT_ID_DI_013, ADM_OCS_EVENT_LEVEL_TRACE, "vehicle use slave download package from usb.", szEventBuf);

    pstDownAsyncTag = __adm_dpm_createDownAsyncTagInfo();
    if (ADM_IS_NULL(pstDownAsyncTag))
    {
        log_e("Failed to create down async tag info");
        return ADM_ECODE_NO_MEMORY;
    }
    pstDpmDownReqParam ->pAsyncTag = (void *)pstDownAsyncTag;

    (void)pthread_rwlock_rdlock(&adm_dpm_getDownInfo()->stRWlock);
    adm_dpm_getDownInfo()->pstDpmDownReqParam = (ADM_DPM_DOWNREQPARAM_T *)pstDpmDownReqParam;
    (void)pthread_rwlock_unlock(&adm_dpm_getDownInfo()->stRWlock);

    // 记录下载的开始时间
    (void)clock_gettime(CLOCK_MONOTONIC, &tsNowbegin);

    do 
    {
        //!< 异步请求接口
        iRet = adm_dim_dpo_requestRemoteDownloadPackage((D16U)htoi(pcDownInstId), 30, 
                                                        pstDpmDownReqParam->fileID,
                                                        szUsbUrl,
                                                        pstDpmDownReqParam->size,
                                                        pstDpmDownReqParam->hash,
                                                        pstDpmDownReqParam->networkRetriesCnt,
                                                        &iMsgArrive);
        if (ADM_IS_ERR(iRet) || ADM_IS_ERR(iMsgArrive))
        {
            log_e("Failed to request remote download package, err[%d], msgArrive[%d]", iRet, iMsgArrive);
            iRet = (ADM_IS_ERR(iMsgArrive) ? (ADM_ECODE_DOWN_MSG_UNREACHABLE) : (iRet));

            //mod by wl 20220321
            (void)adm_memzero(szEventBuf, sizeof(szEventBuf));
            (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), 
                    "Failed to request remote download package, in fileID[%s], err[%d]", pstDpmDownReqParam->fileID, iRet);
            break;
        }

        //!< 异步判断是否超时
        iRet = adm_dpm_waitAsyncResult(pstDownAsyncTag);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Wait slave download result time out, in fileID[%s], err[%d]", pstDpmDownReqParam->fileID, iRet);
            break;
        }

    } while (FALSE);


    //!< 异步请求响应失败
    if (ADM_IS_ERR(iRet))
    {
        if (NULL != pstDpmDownReqParam->fnDownResultNotify)
        {            
            pstResult = adm_dpm_createDownResultInfo(iRet, &pstDpmDownReqParam->stPkgAttr);
            
            pstDpmDownReqParam->fnDownResultNotify(pstResult, NULL, &pstDpmDownReqParam->stPkgAttr);
            adm_dpm_freeDownResultInfo(pstResult);    
        }
    }
    else
    {
        //!< 异步请求响应成功，则将最终的结果从结果回调中同步过来
        iRet = pstDownAsyncTag->stResult.result;
    }
    __adm_dpm_destroyDownAsyncTagInfo(pstDownAsyncTag);
    pstDpmDownReqParam->pAsyncTag = NULL;

    //mod by wl 20220321
    if (ADM_IS_ERR(iRet))
    {
        adm_dpm_addEventAndReport(ADM_COM_MOD_DPM, TRUE, ADM_EVENT_ID_DI_014, ADM_OCS_EVENT_LEVEL_ERROR, "vehicle use slave download package from usb failed!", szEventBuf);
    }
    else
    {
        // 记录下载包的结束时间
        (void)clock_gettime(CLOCK_MONOTONIC, &tsNowend);
        (void)adm_memzero(szEventBuf, sizeof(szEventBuf));
        (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), "STATSPACK downOnceTime:ecuReqid[%s], ecuSwDid[%s], pkgType[%s], fileName[%s], downTime[%d]%s, fileSize[%lld], downType[2](1: master, 2: slave)",
        pstDpmDownReqParam->stPkgAttr.ecuReqid, pstDpmDownReqParam->stPkgAttr.ecuSwDid,
        (pstDpmDownReqParam->stPkgAttr.isRollback == FALSE ? "part-dstPkg" : "part-srcPkg"),
        pstDpmDownReqParam->stPkgAttr.fileName,
        ((tsNowend.tv_sec - tsNowbegin.tv_sec) < 1 ? (tsNowend.tv_nsec / (1000 * 1000)) - (tsNowbegin.tv_nsec / (1000 * 1000)) : (tsNowend.tv_sec - tsNowbegin.tv_sec)),
        ((tsNowend.tv_sec - tsNowbegin.tv_sec) < 1 ? "ms" : "s"), pstDpmDownReqParam->size);
        adm_dpm_addEventAndReport(ADM_COM_MOD_DPM, FALSE,ADM_EVENT_ID_DL_002, ADM_OCS_EVENT_LEVEL_INFO, szEventBuf, NULL);

        adm_dpm_addEventAndReport(ADM_COM_MOD_DPM, FALSE, ADM_EVENT_ID_DI_015, ADM_OCS_EVENT_LEVEL_DEBUG, "vehicle use slave download package from usb successed.", szEventBuf);
    }

    return iRet;
}

/**
 * @brief 从USB下载文件
 * @note U盘升级模式的下载处理
 * @param in const DCHAR *pcDownInstId 下载实例ID
 * @param in ADM_DPM_DOWNREQPARAM_T *pstDpmDownReqParam 下载请求参数
 * @param in DCHAR *pcFileName 下载文件名
 * @retval D32S 0 成功，其他失败
 */
D32S adm_dpm_usbDownload(const DCHAR *pcDownInstId,
                         ADM_DPM_DOWNREQPARAM_T *pstDpmDownReqParam,
                         DCHAR *pcFileName)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    ADM_DPM_DOWNLOADINFO_T *pstDownInfo = NULL;
    D32S s32UclInstId = 0;

    if (ADM_IS_NULL(pcDownInstId) || ADM_IS_NULL(pstDpmDownReqParam) || ADM_IS_NULL(pcFileName))
    {
        log_e("Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }

    log_i("[ABUP-Procedure] usb download package Start,downInstId[%s],fileID[%s]", pcDownInstId, pstDpmDownReqParam->fileID);
    
    /**主控lite*/
    s32UclInstId = adm_cfg_iniParserGetInt(ADM_CFG_VUS_UCL_INSTANCEID,  ADM_CFG_VUS_UCL_INSTANCEID_DEFAULT_VAL);

    pstDownInfo = adm_dpm_getDownInfo();
    if (pstDownInfo->s32UCMDownInstId == (D32S)htoi(pcDownInstId)
        || s32UclInstId == (D32S)htoi(pcDownInstId))
    {
        iRet = downloadMasterFromUsb(pstDpmDownReqParam, pcFileName);
    }
    else
    {
        iRet = downloadSlaveFromUsb(pcDownInstId, pstDpmDownReqParam, pcFileName);
    }

    log_i("[ABUP-Procedure] usb download package End,ret[%d]", iRet);

    if (ADM_IS_ERR(iRet))
    {
        log_e("Failed to download, downInstId[%s], fileID[%s], err:[%d]", pcDownInstId, pstDpmDownReqParam->fileID, iRet);
    }
    else
    {
        log_d("Download fileID[%s] ok, downInstId[%s]", ADM_PRINT_STR(pstDpmDownReqParam->fileID), pcDownInstId);
    }
    
    return iRet;
}

DBOOL adm_dpm_isUsbDirExist(DVOID)
{
    DCHAR* pcUsbDir = DNULL;
    D32S iRet = ADM_ECODE_FAILED;

    pcUsbDir = adm_cvs_getUsbDir();
    if (ADM_IS_NULL(pcUsbDir))
    {
        iRet = ADM_ECODE_KV_GET_STRING_FAILED;
        log_e("Failed to get usb dir, [err:%d]", iRet);
        return DFALSE;
    }

    if (FALSE == adm_com_isDirExist(pcUsbDir))
    {
        iRet = ADM_ECODE_USB_DIR_IS_NOT_EXIST;
        log_e("Failed to check usb download, err[%d]", iRet);
        return DFALSE;
    }

    adm_free(pcUsbDir);

    return DTRUE;
}

D32S adm_dpm_taskVerify(void)
{
    D32S iRet = ADM_ECODE_FAILED;
    ADM_CVS_VERSION_POLICY_T *pstVersionPolicy = NULL;

    ADM_COM_FUNC_ENTRY();

    do
    {
        pstVersionPolicy = adm_cvs_newPolicyBody();
        if (ADM_IS_NULL(pstVersionPolicy))
        {
            iRet = ADM_ECODE_NO_MEMORY;
            log_e("no memory [versionPolicy]");
            break;
        }

        iRet = adm_cvs_getVersionPolicy(pstVersionPolicy);
        if (ADM_IS_ERR(iRet))
        {
            log_e("get version policy failed!");
            break;
        }

        iRet = adm_cvs_taskVerify(pstVersionPolicy->taskId,
                                  pstVersionPolicy->strategyId,
                                  time(NULL),
                                  adm_sdk_getSDKVersion(),
                                  NULL,
                                  adm_ocm_businessCommInterface);
    } while (FALSE);

    adm_cvs_deletePolicyBody(pstVersionPolicy);

    ADM_COM_FUNC_EXIT();

    return iRet;
}