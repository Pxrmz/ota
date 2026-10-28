/**
 * @file    adm_tpm_transferManagement.c
 * @brief   Transfer Package Management
 * @details 升级包预分包管理
 * @author  wangyanpeng
 * @version v0.1.0
 * @date    2020-11-06
 * @copyright www.abupdate.com
 */
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>

#include "adm_tpm_log.h"
#include "adm_tpm_transfer.h"
#include "adm_tpm_transferManagement.h"
#include "adm_tps_transferService.h"
#include "adm_ecs_rsConfig.h"
#include "adm_dpm_reportCloud.h"
#include "adm_dpm_uim.h"
#include "adm_com_lib.h"
#include "adm_com_errorCode.h"
#include "adm_uim_common.h"
#include "adm_cdm_verify_interface.h"
#include "adm_irm_report.h"
#include "adm_cvs_checkVersion.h"
#include "adm_soa.h"

#include "ab_ewebsrv_api.h"
#include "adm_dim_ucl.h"
#include "adm_cfg_api.h"
#include "adm_cds_parse.h"

#define ADM_TPM_DEFTRANS_GETTOBELIST_CNT (3)
#define ADM_TPM_DEFDOWN_CYCLE_CNT (5)

static ADM_TPS_TRNASFER_IDEL_CONTROL_T g_stIdelControl = 
                    {.eIdelControl = IDEL_TRANSFER_INVALID,
                    .bIsAllowIdelTrans = DFALSE,
                    .bIshandleIdelTrans = DFALSE,
                    .bIsHandleIdelTrans4Start = DFALSE,
                    .bIsHandleIdelTrans4Pause = DFALSE,
                    .bIsIdelTransMonitorStart = DFALSE};

static ADM_TPM_TRANSFERINFO_T gstTPMTransInfo = {0};

#if 0
#endif

ADM_TPM_TRANSFERINFO_T *adm_tpm_getTransferInfo(void)
{
    return &gstTPMTransInfo;
}

#define ADM_TPM_QUERY_IDEL_TRANS_COND_FILE_PATH "/data_fota/update/fota/abupota/etc/idelTransConditions.json"
static DBOOL tpmCheckIdelTransCondition(DBOOL bIsUnlimited)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    ADM_COND_POLICY_T stConditions = {0};
    DCHAR *pcConditions = NULL;
    json_object *pJsonObj = NULL;
    ADM_UIM_SOA_CONDITION_DATA_ARRAY_T *pstCondResults = NULL;
    DBOOL bIsConform = FALSE;
    D32S iResult = ADM_ECODE_SUCCESSED;
    const DCHAR *pCondPath = NULL;

    ADM_COM_FUNC_ENTRY();

    do
    {
        pCondPath = ADM_TPM_QUERY_IDEL_TRANS_COND_FILE_PATH;

        adm_condPolicyInit(&stConditions);

        /**读取条件配置文件 */
        iRet = adm_com_readFileData(pCondPath, &pcConditions);
        if (ADM_IS_ERR(iRet))
        {
            log_e("read file failed, fileName[%s]", pCondPath);
            iRet = ADM_ECODE_READ_FILE_FAILED;
            break;
        }

        /** 解析条件配置文件 */
        pJsonObj = json_tokener_parse(pcConditions);
        if (NULL == pJsonObj)
        {
            log_e("json_tokener_parse failed");
            iRet = ADM_ECODE_STRING_TO_JSON_FAILED;
            break;
        }

        /** 解析前置条件 */
        adm_condPolicyJsonParse(&stConditions, pJsonObj);

        /** 验证前置条件 无限等待*/
        iResult = adm_cdm_verify_Compare(&stConditions, &pstCondResults, &bIsConform);
        log_i("Trans in idle mode verify condition result[%d]", iResult);
        if (DFALSE == bIsUnlimited)
        {
            break;
        }

        if (ADM_IS_ERR(iResult))
        {
            json_object_put(pJsonObj);
            adm_condPolicyDeInit(&stConditions);
            adm_free(pcConditions);
            sleep(5);
        }
    } while (iResult != ADM_ECODE_SUCCESSED);

    /** 内存释放 */
    if (NULL != pstCondResults)
    {
        (void)adm_cdm_verify_free(pstCondResults);
    }

    if (NULL != pJsonObj)
    {
        json_object_put(pJsonObj);
    }

    adm_condPolicyDeInit(&stConditions);
    adm_free(pcConditions);

    iRet = (iRet == ADM_ECODE_SUCCESSED) ? iResult : iRet;

    ADM_COM_FUNC_EXIT();

    if (ADM_IS_SUCCESS(iRet))
    {
        return DTRUE;
    }
    else
    {
        return DFALSE;
    }
}

static DVOID *tpmTransCondMonitor(DVOID *pArgs)
{
    D32S iRet = ADM_ECODE_FAILED;

    while (DTRUE == g_stIdelControl.bIsIdelTransMonitorStart)
    {
        sleep(10);

        if (DTRUE == g_stIdelControl.bIsIdelTransMonitorStart)
        {
            if (DFALSE == g_stIdelControl.bIshandleIdelTrans && g_stIdelControl.bIsHandleIdelTrans4Pause == DFALSE &&
                g_stIdelControl.eIdelControl == IDEL_TRANSFER_START)
            {
                (void)pthread_mutex_lock(&g_stIdelControl.stLock);
                g_stIdelControl.bIsHandleIdelTrans4Pause = DTRUE;

                DBOOL bIsCondSatified = tpmCheckIdelTransCondition(DFALSE);
                if (DFALSE == bIsCondSatified)
                {
                    log_i("Pause idel trans.");
                    g_stIdelControl.eIdelControl = IDEL_TRANSFER_PAUSE;
                    adm_tps_setIdelControlInfo(&g_stIdelControl);
                    adm_tpm_operateIdelTrans(ADM_PAUSE_TRANS, adm_soa_getUclAdccInstanceId());
                    adm_tpm_switchHvMode(VEHICLE_CTRL_NO_ACTIVE_HV_MODE); ///< 下高压
                }
                g_stIdelControl.bIsHandleIdelTrans4Pause = DFALSE;
                (void)pthread_mutex_unlock(&g_stIdelControl.stLock);
            }

            if (DFALSE == g_stIdelControl.bIshandleIdelTrans && g_stIdelControl.bIsHandleIdelTrans4Start == DFALSE &&
                g_stIdelControl.eIdelControl == IDEL_TRANSFER_PAUSE)
            {
                (void)pthread_mutex_lock(&g_stIdelControl.stLock);
                g_stIdelControl.bIsHandleIdelTrans4Start = DTRUE;

                DBOOL bIsCondSatified = tpmCheckIdelTransCondition(DFALSE);
                if (DTRUE == bIsCondSatified)
                {
                    log_i("Start idel trans.");
                    iRet = adm_tpm_switchHvMode(VEHICLE_CTRL_ENTER_HV_MODE); ///< 上高压,上高压失败等待下一次条件满足传输文件
                    if (ADM_IS_SUCCESS(iRet))
                    {
                        g_stIdelControl.eIdelControl = IDEL_TRANSFER_START;
                        adm_tps_setIdelControlInfo(&g_stIdelControl);
                        adm_tpm_operateIdelTrans(ADM_COTINUE_TRANS, adm_soa_getUclAdccInstanceId());

                        pthread_cond_broadcast(&g_stIdelControl.stPauseCond);
                    }
                }
                g_stIdelControl.bIsHandleIdelTrans4Start = DFALSE;
                (void)pthread_mutex_unlock(&g_stIdelControl.stLock);
            }
        }
    }

    return NULL;
}

static DVOID tpmIdelTransMonitorStart()
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    D32S errNum = 0;
    pthread_attr_t stThreadAttr = {0};
    pthread_t pThreadHandle;

    /* 获取线程属性句柄 */
    iRet = pthread_attr_init(&stThreadAttr);
    if (ADM_IS_ERR(iRet))
    {
        errNum = errno;
        log_e("pthread_attr_init failed. err[%d], strerror[%s]", iRet, strerror(errNum));
        return;
    }

    /* 设置线程分离属性 */
    iRet = pthread_attr_setdetachstate(&stThreadAttr, PTHREAD_CREATE_DETACHED);
    if (ADM_IS_ERR(iRet))
    {
        errNum = errno;
        log_e("pthread_attr_setdetachstate failed. iRet[%u], strerror[%s]", iRet, strerror(errNum));
        (void)pthread_attr_destroy(&stThreadAttr);
        return;
    }

    //!< 启下载任务线程
    iRet = pthread_create(&pThreadHandle,
                          &stThreadAttr,
                          tpmTransCondMonitor,
                          NULL);
    if (ADM_IS_ERR(iRet))
    {
        errNum = errno;
        log_e("Failed to create thread, err[%d], strerror[%s]", iRet, strerror(errNum));
        (void)pthread_attr_destroy(&stThreadAttr);
        return;
    }

    /* 销毁线程属性句柄 */
    (void)pthread_attr_destroy(&stThreadAttr);

    g_stIdelControl.bIsIdelTransMonitorStart = DTRUE;

    return;
}

static DVOID tpmIdelTransMonitorStop()
{
    g_stIdelControl.bIsIdelTransMonitorStart = DFALSE;
    return;
}

static D32S tpmWaitCheckSubedEventSuccessed(D16U uiInstanceId)
{
    D32S iRet = ADM_ECODE_FAILED;
    D32S iErrCode = ADM_ECODE_FAILED;
    D32S i = 0;
    DCHAR *pSdkVersion = NULL;
    D32S iWaitCount = 0;

    do
    {
        iRet = adm_dim_ucl_getSdkVersion(uiInstanceId, 5, &pSdkVersion, &iErrCode);
        if (ADM_ECODE_SUCCESSED == iRet && NULL != pSdkVersion)
        {
            log_d("get ucl sdk version[%s]", pSdkVersion);
            adm_free(pSdkVersion);
            break;
        }

        /** 1分钟仍获取版本失败，等待超时退出 */
        if (iWaitCount >= 20)
        {
            log_e("get ucl sdk version failed");
            break;
        }

        iWaitCount++;

        sleep(3);
    } while (TRUE);

    return iRet;
}

static DVOID tpmBeginTrans4Idel(D32S* pIdelPackageNum)
{
    DBOOL bIsCondSatified = DFALSE;
    D32S iRet = ADM_ECODE_FAILED;
    D32U uiTobeTransCnt = 0;

    ADM_COM_FUNC_ENTRY();

    if (ADM_IS_NULL(pIdelPackageNum))
    {
        log_e("Invalid param.");
        return;
    }

    if (DFALSE == g_stIdelControl.bIsAllowIdelTrans || 0 == adm_soa_getIdelTransInstanceId())
    {
        log_i("Do not allow idel trans.");
        *pIdelPackageNum = 0;
        return;
    }

    g_stIdelControl.bIshandleIdelTrans = DTRUE; ///< 屏蔽条件监控通知的处理

    do
    {
        iRet = adm_tps_getIdelTransferEcuCnt(&uiTobeTransCnt);
        if (ADM_IS_SUCCESS(iRet) && uiTobeTransCnt == 0)
        {
            log_w("No ecu need trans in idel mode.ret[%d] cnt[%u].", iRet, uiTobeTransCnt);
            break;
        }

        (DVOID)tpmWaitCheckSubedEventSuccessed(adm_soa_getUclAdccInstanceId());

        /** 启动条件监控器 */
        tpmIdelTransMonitorStart();

        (DVOID)pthread_mutex_lock(&g_stIdelControl.stLock);

        /** 一直等待，直到条件满足 */
        bIsCondSatified = tpmCheckIdelTransCondition(DTRUE);
        if (DTRUE == bIsCondSatified)
        {
            /** 上高压操作 */
            iRet = adm_tpm_switchHvMode(VEHICLE_CTRL_ENTER_HV_MODE);
            if (ADM_IS_ERR(iRet))
            {
                log_e("Ctrl hvOn failed.Pause tranfer progress.Waiting for condition satisfied next time.");
                while (IDEL_TRANSFER_START != g_stIdelControl.eIdelControl)
                {
                    g_stIdelControl.eIdelControl = IDEL_TRANSFER_PAUSE;     ///< 不存入kv,让条件查询线程/条件监控线程有初始状态正常运行
                    g_stIdelControl.bIshandleIdelTrans = DFALSE;
                    /** 进入传输暂停状态，等待条件满足信号操作上高压 */
                    pthread_cond_wait(&g_stIdelControl.stPauseCond, &g_stIdelControl.stLock);
                }
            }
            else 
            {
                g_stIdelControl.eIdelControl = IDEL_TRANSFER_START;
                adm_tps_setIdelControlInfo(&g_stIdelControl);
                log_i("Start operatation for transfering in idel mode.");
                (DVOID)adm_tpm_operateIdelTrans(ADM_COTINUE_TRANS, adm_soa_getUclAdccInstanceId());
                log_i("End operatation for transfering in idel mode.");            
            }
        }
        else
        {
            log_i("Pause operatation for transfering in idel mode.");
            g_stIdelControl.eIdelControl = IDEL_TRANSFER_PAUSE;
            adm_tps_setIdelControlInfo(&g_stIdelControl);
            adm_tpm_operateIdelTrans(ADM_PAUSE_TRANS, adm_soa_getUclAdccInstanceId());
        }

        (DVOID)pthread_mutex_unlock(&g_stIdelControl.stLock);
    } while (FALSE);

    g_stIdelControl.bIshandleIdelTrans = DFALSE;

    *pIdelPackageNum = uiTobeTransCnt;

    ADM_COM_FUNC_EXIT();

    return;
}

static DVOID tpmEndTransIdel(D32S iIdelPackageNum)
{
    if (DFALSE == g_stIdelControl.bIsAllowIdelTrans || 0 == adm_soa_getIdelTransInstanceId())
    {
        return;
    }

    (DVOID)pthread_mutex_lock(&g_stIdelControl.stLock);
    g_stIdelControl.eIdelControl = IDEL_TRANSFER_END;
    adm_tps_setIdelControlInfo(&g_stIdelControl);
    (DVOID)pthread_mutex_unlock(&g_stIdelControl.stLock);

    if (iIdelPackageNum > 0)
    {
        /** 停止闲时传输条件监控 */
        tpmIdelTransMonitorStop();

        /** 下高压（no active）处理 */
        (DVOID)adm_tpm_switchHvMode(VEHICLE_CTRL_NO_ACTIVE_HV_MODE);
    }

    return;
}

/*!
 *******************************************************************************
 * 判断是否是传输暂停错误
 *
 * \note 
 * 
 * \param[in]   iError 
 * \retval      D32S
 *******************************************************************************
 */
static D32S tpmIstansferPause(D32S iError)
{
    D32S iRet = ADM_ECODE_SUCCESSED;

    switch (iError)
    {
        case ADM_ECODE_CONDITION_VERIFY_FAILED:
            iRet = ADM_ECODE_PAUSE_TRANSFER;
            break;
        default:
            iRet = iError;
            break;
    }

    return iRet;
}

/*!
*******************************************************************************
* 设置传输监控是否异常
*
* \note 
* 
* \param[in]   isMonitorAbnormal   是否监控异常
* \retval      none
*******************************************************************************
*/
static void tpmSetIsMonitorAbnormal(DBOOL bIsMonitorAbnormal)
{
    (void)pthread_rwlock_wrlock(&gstTPMTransInfo.stRWlock);
    gstTPMTransInfo.isMonitorAbnormal = bIsMonitorAbnormal;
    (void)pthread_rwlock_unlock(&gstTPMTransInfo.stRWlock);

    return;
}


/*!
*******************************************************************************
* 设置是否启动传输监控
*
* \note 
* 
* \param[in]   isMonitoring       是否启动监控
* \param[in]   monitorHandle      监控句柄
* \retval      none
*******************************************************************************
*/
static void tpmSetIsMonitoring(DBOOL bIsMonitoring, adm_handle_t monitorHandle)
{
    (void)pthread_rwlock_wrlock(&gstTPMTransInfo.stRWlock);
    gstTPMTransInfo.isMonitoring = bIsMonitoring;
    gstTPMTransInfo.monitorHandle = monitorHandle;
    (void)pthread_rwlock_unlock(&gstTPMTransInfo.stRWlock);

    return;
}


/*!
 *******************************************************************************
 * 传输过程监控到异常信息通知
 *
 * \note
 *          
 * \param[in]   pStCondResults  下载监控条件列表
 * \param[in]   pbIsConform     是否异常
 * \retval      D32S
 *******************************************************************************
 */
static void *__adm_tpm_transferMonitorAbnormalNotify
(
    ADM_UIM_SOA_CONDITION_DATA_ARRAY_T *pStCondResults,
    DBOOL *pbIsConform
)
{
    D32S iRet = ADM_ECODE_SUCCESSED;

    adm_dpm_addEventAndReport(ADM_COM_MOD_TPM, FALSE, ADM_EVENT_ID_DI_061, ADM_OCS_EVENT_LEVEL_DEBUG, "vehicle monitor transfer process abnormal notify.", NULL);
    tpmSetIsMonitorAbnormal(TRUE);

    //!< 通知 uim 传输过程监控异常信息
    iRet = adm_dpm_downCondVerifyResultNotify2Uim(gstTPMTransInfo.eTriggerType, 
                    "TransferMonitorAbnormal", 
                    pStCondResults,
                    (D32S)*pbIsConform);
    if (ADM_IS_ERR(iRet))
    {
        log_e("Failed to notify transfer monitor abnormal info! err[%d]", iRet);
        return NULL;
    }
    
    return NULL;
}


/*!
 *******************************************************************************
 * 传输监控异常是否异常
 *
 * \note    异常：返回ADM_ECODE_FAILED ； 无异常返回：ADM_ECODE_SUCCESSED.
 * 
 * \param[in]   
 * \retval      D32S
 *******************************************************************************
 */
static D32S adm_tpm_checkTransferIsAbnormal(void)
{
    D32S iRet = ADM_ECODE_SUCCESSED;   //默认监控无异常

    (void)pthread_rwlock_rdlock(&gstTPMTransInfo.stRWlock);
    if (TRUE == gstTPMTransInfo.isMonitorAbnormal)
    {
        iRet = ADM_ECODE_MONITOR_ABNORMAL_DOWN_BREAK;
    }

    (void)pthread_rwlock_unlock(&gstTPMTransInfo.stRWlock);
    
    return iRet;
}

#if 0
#endif

/*!
 *******************************************************************************
 * 检查预分包环境
 *
 * \note    主要判断：对端设备存储空间是否满足
 * 
 * \param   none
 * \retval  D32S
 *******************************************************************************
 */
static D32S adm_tpm_checkTransferEnviroment(void)
 {
    D32S iRet      = ADM_ECODE_SUCCESSED;
    D32S errorCode = ADM_ECODE_FAILED;
    D64S realSize = 0;
    D64S needSize = 0;
    DCHAR szEventBuf[ADM_COM_LEN_256] = {0};
    struct list_head *pstNode     = NULL;
    struct list_head *pstNextNode = NULL;
    ADM_TPS_TRANSFERLIST *pstTransferInstlist = NULL;
    ADM_TPS_TRANSFERINFO *pstTransferInstanceInfo = NULL;
    D32U uiTryCount = 0;
    D32U uiTimeoutCount = 0;

    adm_dpm_addEventAndReport(ADM_COM_MOD_TPM, FALSE, ADM_EVENT_ID_TP_016, ADM_OCS_EVENT_LEVEL_TRACE, "vehicle check transfer enviroment.", NULL);

    pstTransferInstlist = adm_tps_getEcutransferInstanceList();
    if (ADM_IS_NULL(pstTransferInstlist))
    {
        log_e("get ecu transfer instance list is null.");
        (void)adm_memzero(szEventBuf, sizeof(szEventBuf));
        (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), "get ecu transfer instance list is null.");
        adm_dpm_addEventAndReport(ADM_COM_MOD_TPM, FALSE, ADM_EVENT_ID_TP_018, ADM_OCS_EVENT_LEVEL_DEBUG, "vehicle check transfer enviroment successed.", szEventBuf);

        return iRet;
    }

    list_for_each_safe(pstNode, pstNextNode, &pstTransferInstlist->stHead)
    {
        //!< 遍历校验传输受控对象传输的升级包大小与内存的大小
        pstTransferInstanceInfo = list_entry(pstNode, ADM_TPS_TRANSFERINFO, stHead);

        realSize  = 0;
        errorCode = ADM_ECODE_FAILED;

        do
        {
            iRet = adm_dim_ucl_verifySpaceIsEnouth((D16U)htoi(pstTransferInstanceInfo->transferInstanceId), 
                                                    ADM_TPM_SYNC_REQ_TIMEOUT, pstTransferInstanceInfo->size, &realSize, &errorCode);
            if(ADM_IS_ERR(iRet))
            {
                /** 防止传输过程中重启，与对端未重连的场景 */
                if ((ADM_ECODE_RPC_INSTANCE_OFFLINE == iRet || ADM_ECODE_RPC_SERVICE_OFFLINE == iRet) &&
                    uiTryCount < 25)
                {
                    uiTryCount++;
                    sleep(5);
                    continue;
                }
                else if (ADM_ECODE_RPC_REQSERV_TIMEOUT == iRet && uiTimeoutCount < 1)
                {
                    uiTimeoutCount++;
                    sleep(2);
                    continue;
                }
                else 
                {
                    log_e("Failed to request host verify storage space, err[%d][%d]", iRet, ADM_ECODE_UCL_STORAGE_SPACE_NOT_ENOUGH);
                    (void)adm_memzero(szEventBuf, sizeof(szEventBuf));
                    (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), "Failed to request host verify storage space, err[%d][%d]", 
                    iRet, ADM_ECODE_UCL_STORAGE_SPACE_NOT_ENOUGH);
                    adm_dpm_addEventAndReport(ADM_COM_MOD_TPM, TRUE, ADM_EVENT_ID_TP_019, ADM_OCS_EVENT_LEVEL_ERROR, "vehicle verify UCL storage space is enouth failed!", szEventBuf);

                    iRet = ADM_ECODE_UCL_STORAGE_SPACE_NOT_ENOUGH;
                    break;
                }
            }
            else 
            {
                break;
            }
        } while (TRUE);

        if (ADM_IS_ERR(iRet))
        {
            break;
        }

        log_i("Check UCL storage space, rspCode[%d]", errorCode);
        (void)adm_memzero(szEventBuf, sizeof(szEventBuf));
        (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), "uclInstId[%s], needSize[%lld], realSize[%lld].", 
        pstTransferInstanceInfo->transferInstanceId, needSize, realSize);
        adm_dpm_addEventAndReport(ADM_COM_MOD_TPM, FALSE, ADM_EVENT_ID_TP_020, ADM_OCS_EVENT_LEVEL_DEBUG, "vehicle verify UCL storage space is enouth successed.", szEventBuf);

        if (ADM_IS_SUCCESS(errorCode))
        {
            continue;
        }
        else
        {
            iRet = ADM_ECODE_UCL_STORAGE_SPACE_NOT_ENOUGH;
            log_e("Check device storage space Failed, err[%d], rspCode[%d], needSize[%lld], realSize[%lld]", 
                  iRet, errorCode, needSize, realSize);

            (void)adm_memzero(szEventBuf, sizeof(szEventBuf));
            (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), "Check device storage space Failed, err[%d], rspCode[%d], needSize[%lld], realSize[%lld]", 
                         iRet, errorCode, needSize, realSize);
            break;
        }
   }

    adm_tps_delEcuTransferInstanceList(&pstTransferInstlist);

    if(ADM_IS_ERR(iRet))
    {
        adm_dpm_addEventAndReport(ADM_COM_MOD_TPM, TRUE, ADM_EVENT_ID_TP_017, ADM_OCS_EVENT_LEVEL_ERROR, "vehicle check transfer enviroment failed!", szEventBuf);
    }
    else
    {
        adm_dpm_addEventAndReport(ADM_COM_MOD_TPM, FALSE, ADM_EVENT_ID_TP_018, ADM_OCS_EVENT_LEVEL_DEBUG, "vehicle check transfer enviroment successed.", NULL);
    }

    return iRet;
 }


/*!
 *******************************************************************************
 * 检查预分包前置条件
 *
 * \note    主要判断：batteryPower/MemorySpace(包含MP5、TBox)
 * 
 * \param   none
 * \retval  D32S
 *******************************************************************************
 */
static D32S adm_tpm_checkTransferPreCondition(ADM_APPLICATION_COMMON_CMDID_TYPE_E eTriggerType)
{
    D32S i=0;
    D32S iRet = ADM_ECODE_SUCCESSED;
    DBOOL bIsConform = TRUE;
    ADM_DPS_TRANSFERCONDITION_T stTransCond = {0};
    ADM_UIM_SOA_CONDITION_DATA_ARRAY_T *pstCondResults = NULL;
    DCHAR szEventBuf[ADM_COM_LEN_256] = {0}; 
    ADM_DPM_UPGRADE_CONDITION_TYPE_E eConvertVal;

    do
    {
        //mod by wl 20220321
        (void)adm_memzero(szEventBuf, sizeof(szEventBuf));
        adm_dpm_addEventAndReport(ADM_COM_MOD_TPM, FALSE, ADM_EVENT_ID_TP_021, ADM_OCS_EVENT_LEVEL_DEBUG, "vehicle check transfer preCondition.", NULL);
    
        //!< 获取传输条件
        iRet = adm_tps_getTransferCondition(&stTransCond);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to get transfer condition, err[%d].", iRet);
            (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), "Failed to get transfer condition, err[%d].", iRet);
            adm_dpm_addEventAndReport(ADM_COM_MOD_TPM, FALSE, ADM_EVENT_ID_TP_024, ADM_OCS_EVENT_LEVEL_DEBUG, "Failed to get transfer condition.", szEventBuf);
            break;
        }

        //!< 平台未配置相关传输前置条件，直接返回
        if (0 == stTransCond.trigNum)
        {
            iRet = ADM_ECODE_SUCCESSED;
            log_i("OTA Service no set transfer condition.");
            (void)adm_memzero(szEventBuf, sizeof(szEventBuf));
            (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), "OTA Service no set transfer condition.");
            adm_dpm_addEventAndReport(ADM_COM_MOD_TPM, FALSE, ADM_EVENT_ID_TP_026, ADM_OCS_EVENT_LEVEL_DEBUG, "vehicle get transfer condition successed.", szEventBuf);
            break;
        }

        eConvertVal = adm_dpm_changeTriggerTypeVal(eTriggerType);

        //!< 找到车云协议对应模式的的前置条件
        for (i = 0; i < stTransCond.trigNum; i++)
        {
            if ((D32S)eConvertVal == stTransCond.pstTrigTransferCondList[i].iType)
            {
                break;
            }
        }

        //!< 未找到车云协议对应模式的的前置条件
        if (i == stTransCond.trigNum || 0 == stTransCond.pstTrigTransferCondList[i].iCondNum)
        {
            iRet = ADM_ECODE_SUCCESSED;
            log_i("OTA Service no find transfer condition.");
            (void)adm_memzero(szEventBuf, sizeof(szEventBuf));
            (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), "OTA Service no find transfer condition.");
            adm_dpm_addEventAndReport(ADM_COM_MOD_TPM, FALSE, ADM_EVENT_ID_TP_026, ADM_OCS_EVENT_LEVEL_DEBUG, "vehicle get transfer condition successed.", szEventBuf);
            break;
        }

        iRet = adm_cdm_verify_Compare(&stTransCond.pstTrigTransferCondList[i], &pstCondResults, &bIsConform);    
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to verify compare transfer condition, err[%d]", iRet);

            iRet = ADM_ECODE_CONDITION_VERIFY_FAILED;
            (void)adm_memzero(szEventBuf, sizeof(szEventBuf));
            (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), "Failed to verify compare transfer condition, err[%d].", iRet);
            adm_dpm_addEventAndReport(ADM_COM_MOD_TPM, TRUE, ADM_EVENT_ID_TP_025, ADM_OCS_EVENT_LEVEL_ERROR, "vehicle verify transfer condition failed!", szEventBuf);
            break;
        }

        //!< 通知 uim 传输前置条件校验结果
        iRet = adm_dpm_downCondVerifyResultNotify2Uim(eTriggerType, 
                        "TransferPreConditionVerify", 
                        pstCondResults,
                        (D32S)bIsConform);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to notify transfer condition verify result! err[%d]", iRet);
            //!< 通知失败不影响 
            iRet = ADM_ECODE_SUCCESSED;
            break;
        }
    } while(FALSE);


    //!< 最后释放比较结果信息内存
    if (NULL != pstCondResults)
    {
        (void)adm_cdm_verify_free(pstCondResults);
    }

    adm_dps_cleanTransferCondition(&stTransCond);
    if (ADM_IS_ERR(iRet))
    {
        //mod by wl 20220321
        adm_dpm_addEventAndReport(ADM_COM_MOD_TPM, TRUE, ADM_EVENT_ID_TP_022, ADM_OCS_EVENT_LEVEL_ERROR, "vehicle check transfer preCondition failed!", szEventBuf);
    }
    else
    {
        //mod by wl 20220321
        adm_dpm_addEventAndReport(ADM_COM_MOD_TPM, FALSE, ADM_EVENT_ID_TP_023, ADM_OCS_EVENT_LEVEL_DEBUG, "vehicle check transfer preCondition successed.", NULL);
    }

    return iRet;
}


/*!
 *******************************************************************************
 * 启动分包过程监控
 *
 * \note
 *          
 * \param[in]  fnMonitorNotify 分包过程条件异常通知回调函数
 * \retval     D32S
 *******************************************************************************
 */
static D32S adm_tpm_startTransferProcessMonitor
(
    ADM_CDM_NOTIFY_Fn fnMonitorNotify,
    ADM_APPLICATION_COMMON_CMDID_TYPE_E eTriggerType
)
{
    D32S i=0;
    D32S iRet = ADM_ECODE_SUCCESSED;
    adm_handle_t transMonitorHandle = NULL;
    ADM_DPS_TRANSFERCONDITION_T stTransCond = {0};
    DCHAR szEventBuf[ADM_COM_LEN_256] = {0}; 
    ADM_DPM_UPGRADE_CONDITION_TYPE_E eConvertVal;

    if (ADM_IS_NULL(fnMonitorNotify))
    {
        log_e("Invalid input param");
        return ADM_ECODE_INVALID_POINTER;
    }

    /** 传输条件为空也需要标记条件监控已启动,用于后续的闲时传输动作 */
    tpmSetIsMonitoring(TRUE, NULL);

    //mod by wl 20220321
    (void)adm_memzero(szEventBuf, sizeof(szEventBuf));
    adm_dpm_addEventAndReport(ADM_COM_MOD_TPM, FALSE, ADM_EVENT_ID_TP_027, ADM_OCS_EVENT_LEVEL_DEBUG, "vehicle start transfer process monitor.", NULL);

    (void)pthread_rwlock_rdlock(&gstTPMTransInfo.stRWlock);
    if (TRUE == gstTPMTransInfo.isMonitoring)
    {
        (void)pthread_rwlock_unlock(&gstTPMTransInfo.stRWlock);
        log_e("Transfer monitor thread alreadly running.");
        (void)adm_memzero(szEventBuf, sizeof(szEventBuf));
        (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), "transfer monitor thread alreadly running, err[%d]", ADM_ECODE_PTHREAD_ALREADY_RUNNING);
        adm_dpm_addEventAndReport(ADM_COM_MOD_TPM, FALSE, ADM_EVENT_ID_TP_029, ADM_OCS_EVENT_LEVEL_DEBUG, "vehicle start download process monitor successed.", szEventBuf);
        return ADM_ECODE_SUCCESSED;
    }
    (void)pthread_rwlock_unlock(&gstTPMTransInfo.stRWlock);

    //!< 获取预分包条件
    (void)adm_memzero(&stTransCond, sizeof(stTransCond));
    iRet = adm_tps_getTransferCondition(&stTransCond);
    if (ADM_IS_ERR(iRet))
    {
        log_e("Failed to get transfer condition, err[%d].", iRet);        
        //mod by wl 20220321
        (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), "Failed to get transfer condition, err[%d].", iRet);
        adm_dpm_addEventAndReport(ADM_COM_MOD_TPM, TRUE, ADM_EVENT_ID_TP_028, ADM_OCS_EVENT_LEVEL_ERROR, "vehicle start transfer process monitor failed!", szEventBuf);
        return iRet;
    }

    //!< 平台未配置相关传输前置条件，直接返回
    if (0 == stTransCond.trigNum)
    {
        iRet = ADM_ECODE_SUCCESSED;
        log_i("OTA Service not set transfer condition.");
        (void)adm_memzero(szEventBuf, sizeof(szEventBuf));
        (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), "OTA Service not set transfer condition.");
        adm_dpm_addEventAndReport(ADM_COM_MOD_TPM, FALSE, ADM_EVENT_ID_TP_029, ADM_OCS_EVENT_LEVEL_DEBUG, "vehicle start transfer process monitor successed.", szEventBuf);
        adm_dps_cleanTransferCondition(&stTransCond);
        return iRet;
    }

    eConvertVal = adm_dpm_changeTriggerTypeVal(eTriggerType);

    //!< 找到车云协议对应模式的的前置条件
    for (i = 0; i < stTransCond.trigNum; i++)
    {
        if ((D32S)eConvertVal == stTransCond.pstTrigTransferCondList[i].iType)
        {
            break;
        }
    }

    //!< 未找到车云协议对应模式的的前置条件
    if (i == stTransCond.trigNum || 0 == stTransCond.pstTrigTransferCondList[i].iCondNum)
    {
        iRet = ADM_ECODE_SUCCESSED;
        log_i("OTA Service not find transfer condition.");
        (void)adm_memzero(szEventBuf, sizeof(szEventBuf));
        (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), "OTA Service not find transfer condition.");
        adm_dpm_addEventAndReport(ADM_COM_MOD_TPM, FALSE, ADM_EVENT_ID_TP_029, ADM_OCS_EVENT_LEVEL_DEBUG, "vehicle start transfer process monitor successed.", szEventBuf);
        adm_dps_cleanTransferCondition(&stTransCond);

        return iRet;
    }

    //!< 启动传输过程监控
    transMonitorHandle = adm_cdm_LoopVerify(&stTransCond.pstTrigTransferCondList[i], ADM_TPM_DEFDOWN_CYCLE_CNT, fnMonitorNotify);
    if (ADM_IS_NULL(transMonitorHandle))
    {
        iRet = ADM_ECODE_INVALID_PARAM;
        log_e("Failed to start transfer condition by cdm, err[%d].", iRet);
        (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), "Failed to start transfer condition by cdm, err[%d].", iRet);
        adm_dpm_addEventAndReport(ADM_COM_MOD_TPM, TRUE, ADM_EVENT_ID_TP_028, ADM_OCS_EVENT_LEVEL_ERROR, "vehicle start transfer process monitor failed!", szEventBuf);
        adm_dps_cleanTransferCondition(&stTransCond);

        return iRet;
    }

    tpmSetIsMonitoring(TRUE, transMonitorHandle);
    adm_dps_cleanTransferCondition(&stTransCond);

    //mod by wl 20220321
    adm_dpm_addEventAndReport(ADM_COM_MOD_TPM, FALSE, ADM_EVENT_ID_TP_029, ADM_OCS_EVENT_LEVEL_DEBUG, "vehicle start transfer process monitor successed.", NULL);

    return iRet;
}

/*!
 *******************************************************************************
 * 停止分包过程监控
 *
 * \note    
 * 
 * \param   none
 * \retval  none
 *******************************************************************************
 */
static void adm_tpm_stopTransferProcessMonitor(void)
{
    (void)pthread_rwlock_rdlock(&gstTPMTransInfo.stRWlock);
    if (FALSE == gstTPMTransInfo.isMonitoring)
    {
        (void)pthread_rwlock_unlock(&gstTPMTransInfo.stRWlock);
        log_i("Transfer monitor thread not exist.");
        return;
    }
    (void)pthread_rwlock_unlock(&gstTPMTransInfo.stRWlock);

    adm_dpm_addEventAndReport(ADM_COM_MOD_TPM, TRUE, ADM_EVENT_ID_TE_001, ADM_OCS_EVENT_LEVEL_INFO, "vehicle stop transfer process monitor.", NULL);
    tpmSetIsMonitoring(FALSE,NULL);
    tpmSetIsMonitorAbnormal(FALSE);


    return;
}

#if 0
#endif

/*!
 *******************************************************************************
 * 强校验字符串匹配
 *
 * \note    pcSubStr 与 pcStr 前 adm_strlen(pcSubStr) 字符强一致
 *              且返回匹配的末端字符位置指针
 *          return pcStr + adm_strlen(pcSubStr)
 * 
 * \param[in]  pcStr     源字符串
 * \param[in]  pcSubStr  待匹配字符串
 * \retval     DCHAR     匹配后的字符串指针(若两个字符串相等，则返回 NULL)
 *******************************************************************************
 */
const DCHAR *adm_tpm_matchLastSubStrInStr(const DCHAR *pcStr, const DCHAR *pcSubStr)
{
    D32U iStrLen   = (D32U)0;
    D32U iSubLen   = (D32U)0;

    if (ADM_IS_NULL(pcStr) || ADM_IS_NULL(pcSubStr))
    {
        log_e("Invalid input param");
        return NULL;
    }

    iStrLen = adm_strlen(pcStr);
    iSubLen = adm_strlen(pcSubStr);
    // Check if the substring is longer than the string being searched.
    if (iSubLen >= iStrLen)
    {
        log_e("subString gt(et) strString size!");
        return NULL;
    }

    if (0 == adm_memcmp(pcStr, pcSubStr, iSubLen))
    {
        return pcStr + iSubLen;
    }

    return NULL;
}


/*!
 *******************************************************************************
 * 创建传输url
 *
 * \note       ewebServer 和 package 均在主控侧
 * 
 * \param[in]  pcMasterPkgPath  主控下载的升级包文件存储路径
 * \retval     DCHAR*           生成的 ewebServer 传输的 url
 *******************************************************************************
 */
static DCHAR *__adm_tpm_createTransUrlFromMaster(const DCHAR *pcMasterPkgPath)
{

    DCHAR szTransURL[ADM_COM_FILEPATH_LEN] = {0};
    const DCHAR *pcDownPkgSuffixUrl    = NULL;
    const DCHAR *pcEwebRootDir   = NULL;
    const DCHAR *pcEwebPrefixUrl = NULL;

    if (ADM_IS_NULL(pcMasterPkgPath))
    {
        log_e("Invalid input param");
        return NULL;
    }

    //!< 1. 获取eweb主目录 
    pcEwebRootDir = adm_cfg_iniParserGetString(ADM_CFG_EWEB_DOC_ROOT_DIR, ADM_CFG_EWEB_DOC_ROOT_DIR_DEFAULT_VAL);
    log_d("pcMasterPkgPath[%s], pcEwebRootDir[%s]", pcMasterPkgPath, pcEwebRootDir);

    //!< 2. 判断 pcMasterPkgPath 中是否包含 pcEwebRootDir 存储路径, 若不包含则无法使用eweb 
    pcDownPkgSuffixUrl = adm_tpm_matchLastSubStrInStr(pcMasterPkgPath, pcEwebRootDir);
    if (ADM_IS_NULL(pcDownPkgSuffixUrl))
    {
        log_e("Failed to check package path[%s] user eweb[%s].", pcMasterPkgPath, pcEwebRootDir);
        return NULL;
    }
    log_d("pcDownPkgSuffixUrl[%s]", pcDownPkgSuffixUrl);

    //!< 2. 序列化生成新的传输 url 
    pcEwebPrefixUrl = adm_cfg_iniParserGetString(ADM_CFG_EWEB_ADDRESS_HEADER, ADM_CFG_EWEB_ADDRESS_HEADER_DEFAULT_VAL);
    (void)adm_memzero(szTransURL, sizeof(szTransURL));
    (void)adm_snprintf(szTransURL, sizeof(szTransURL), "%s%s", pcEwebPrefixUrl, pcDownPkgSuffixUrl);
    log_d("pcMasterPkgPath[%s], szTransURL[%s]", pcMasterPkgPath, szTransURL);

    return adm_strdup(szTransURL);
}

/*!
 *******************************************************************************
 * 填充tpm分包请求参数ecu分包信息
 *
 * \note
 * 
 * \param[in]       enPkgType          包类型[src/dst]
 * \param[in]       pstTransPkg         待分包ecu升级包信息 
 * \param[inout]    pstTpmTransReqParam tpm分包请求参数结构信息
 * \retval          ADM_DPO_DOWNLOAD_REQPARAM_T*
 *******************************************************************************
 */
static D32S __adm_tpm_fillTPMTransReqParam
(
    ADM_DPS_PARTPKGTYPE_E        enPkgType,
    ADM_TPS_TRANSFERPKG_T       *pstTransPkg,
    ADM_TPM_TRANSFER_REQPARAM_T *pstTransReqParam
)
{
    if (ADM_IS_NULL(pstTransPkg) || ADM_IS_NULL(pstTransReqParam))
    {
        log_e("Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }
    
    //!< 目标包优先填充，增加if命中率
    if (ADM_PARTPKG_TYPE_DST == enPkgType)
    {
        pstTransReqParam->fileID = adm_tps_createTransFileIDByEcuInfo(
                                        pstTransPkg->ecuReqid, 
                                        pstTransPkg->ecuSwDid, 
                                        FALSE,
                                        pstTransPkg->stDstPkgInfo.fileName);
        if (ADM_IS_NULL(pstTransReqParam->fileID))
        {
            log_e("Failed to create trans file id, err[%d]", ADM_ECODE_NO_MEMORY);
            return ADM_ECODE_NO_MEMORY;
        }

        //!< 创建待传输的 url
        pstTransReqParam->url = __adm_tpm_createTransUrlFromMaster(pstTransPkg->stDstPkgInfo.filePath);
        if (ADM_IS_NULL(pstTransReqParam->url))
        {
            adm_tps_freeTransFileID(&pstTransReqParam->fileID);
            log_e("Failed to create dst pkg trans url, err[%d]", ADM_ECODE_TRANS_CREATEURL_FAILED);
            return ADM_ECODE_TRANS_CREATEURL_FAILED;
        }
        pstTransReqParam->hash = adm_strdup(pstTransPkg->stDstPkgInfo.hash); 
        pstTransReqParam->fileName   = adm_strdup(pstTransPkg->stDstPkgInfo.fileName);
        pstTransReqParam->fileSize   = pstTransPkg->stDstPkgInfo.size;
        pstTransReqParam->isRollback = FALSE;

        //!< 这个简单点约定先分 srcPkg, 再分 dstPkg 包
        //!< 叠加另一个包的大小，便于 srcPkg/dstPkg 进度融合 --- 分目标包则说明源包已经分完或者没有源包
        pstTransReqParam->baseSize = 
                ((ADM_IS_NULL(pstTransPkg->pstSrcPkgInfo)) ? (0) : (pstTransPkg->pstSrcPkgInfo->size));
    }
    else
    {
        if (ADM_IS_NULL(pstTransPkg->pstSrcPkgInfo))
        {
            log_e("Invalid package type, no find srcPakage info, err[%d]", ADM_ECODE_INVALID_PARAM);
            return ADM_ECODE_INVALID_PARAM;
        }

        pstTransReqParam->fileID = adm_tps_createTransFileIDByEcuInfo(
                                        pstTransPkg->ecuReqid, 
                                        pstTransPkg->ecuSwDid, 
                                        TRUE,
                                        pstTransPkg->pstSrcPkgInfo->fileName);
        if (ADM_IS_NULL(pstTransReqParam->fileID))
        {
            log_e("Failed to create trans file id, err[%d]", ADM_ECODE_NO_MEMORY);
            return ADM_ECODE_NO_MEMORY;
        }

        //!< 创建待传输的 url
        pstTransReqParam->url = __adm_tpm_createTransUrlFromMaster(pstTransPkg->pstSrcPkgInfo->filePath);
        if (ADM_IS_NULL(pstTransReqParam->url))
        {
            adm_tps_freeTransFileID(&pstTransReqParam->fileID);
            log_e("Failed to create src pkg trans url, err[%d]", ADM_ECODE_TRANS_CREATEURL_FAILED);
            return ADM_ECODE_TRANS_CREATEURL_FAILED;
        }
        pstTransReqParam->hash = adm_strdup(pstTransPkg->pstSrcPkgInfo->hash);
        pstTransReqParam->fileName = adm_strdup(pstTransPkg->pstSrcPkgInfo->fileName);
        pstTransReqParam->fileSize = pstTransPkg->pstSrcPkgInfo->size;
        pstTransReqParam->isRollback = TRUE;

        //!< 叠加另一个包的大小，便于 srcPkg/dstPkg 进度融合 --- 分源包则说明目标包没开始分
        pstTransReqParam->baseSize = 0;
    }

    pstTransReqParam->ecuReqid = adm_strdup(pstTransPkg->ecuReqid);
    pstTransReqParam->ecuSwDid = adm_strdup(pstTransPkg->ecuSwDid);
    log_d("Adapter fileID[%s]",  pstTransReqParam->fileID);
    log_d("Adapter fileurl[%s]", pstTransReqParam->url);

    return ADM_ECODE_SUCCESSED;
}

#if 0
#endif

/*!
 *******************************************************************************
 * 停止 ewebserver 进程.
 *
 * \note 
 *
 * \param   none   待启动的进程命令
 * \retval  D32S   0: 成功 non-0：失败
 *******************************************************************************
 */
D32S adm_tpm_stopEwebServer(void)
{

    //!< 修改进程启动ewebserver的方式为线程
#if 0
    DCHAR cmd[256] = {0};

    adm_memzero(cmd, sizeof(cmd));
    (void)adm_snprintf(cmd, sizeof(cmd), ADM_COM_TRANS_EWEBSERVER_STOP_CMD, 
            ADM_COM_TRANS_EWEBSERVER_NAME);
    log_i("===> stop cmd[%s]", cmd);

    //!< 停止 ewebserver 服务
    system(cmd);
#else
    //!< 使用API停止ewebserver
    abEWebSrv_Stop();
#endif
    return ADM_ECODE_SUCCESSED;
}

/*!
 *******************************************************************************
 * 启动 ewebserver 进程.
 *
 * \note 
 *
 * \param   none   待启动的进程命令
 * \retval  D32S   0: 成功 non-0：失败
 *******************************************************************************
 */
D32S adm_tpm_startEwebServer(void)
{
    //!< ewebserver的启动方式由进程修改为线程
#if 0
    D32S iRet = ADM_ECODE_SUCCESSED;
    DCHAR cmd[256] = {0};

    //!< 先停止后启动
    (void)adm_tpm_stopEwebServer();

    adm_memzero(cmd, sizeof(cmd));
    (void)adm_snprintf(cmd, sizeof(cmd), ADM_COM_TRANS_EWEBSERVER_START_CMD, 
        ADM_COM_TRANS_EWEBSERVER_STORE_PATH, ADM_COM_TRANS_EWEBSERVER_NAME);
    log_i("===> start cmd[%s]", cmd);

    //!< 启动 ewebserver 服务
    iRet = adm_tps_startProgress(cmd);
    if (ADM_IS_ERR(iRet)) 
    {
        log_e("Failed to start process[%s], err[%d]", cmd);
        return iRet;
    }
#else
    D32S iRet = ADM_ECODE_SUCCESSED;
    DBOOL isHttps = FALSE;
    const DCHAR* pIp = NULL;
    D32S iPort = -1;
    DCHAR* pIpPort = NULL;

    //!< 设置ewebserver的协议方式 http or https; 端口
    isHttps = adm_cfg_iniParserGetBoolean(ADM_CFG_EWEB_IS_HTTPS, (DBOOL)ADM_CFG_EWEB_IS_HTTPS_DEFAULT_VAL);
    pIp = adm_cfg_iniParserGetString(ADM_CFG_EWEB_LISTENING_IP, ADM_CFG_EWEB_LISTENING_IP_DEFAULT_VAL);
    iPort = adm_cfg_iniParserGetInt(ADM_CFG_EWEB_LISTENING_PORTS, ADM_CFG_EWEB_LISTENING_PORTS_DEFAULT_VAL);
    pIpPort = adm_com_createEwebIpPort(pIp, iPort);
    if (NULL == pIpPort)
    {
        log_e("Create eweb ip-port error.");
        iRet = ADM_ECODE_FAILED;
        return iRet;
    }

    //!< 设置Ewebserver存储日志的路径
    abEWebSrv_SetLog(AB_EWEBSRV_LOG_DEBUG, adm_cfg_iniParserGetString(ADM_CFG_EWEB_LOG_STORAGE_PATH, ADM_CFG_EWEB_LOG_STORAGE_PATH_DEFAULT_VAL),
                     true,
                     true,
                     false);

    //!< 设置下载的根目录和文件上传目录
    abEWebSrv_CfgSetDocumentDir(adm_cfg_iniParserGetString(ADM_CFG_EWEB_DOC_ROOT_DIR, ADM_CFG_EWEB_DOC_ROOT_DIR_DEFAULT_VAL),
                                adm_cfg_iniParserGetString(ADM_CFG_EWEB_DOC_UPLOAD_DIR, ADM_CFG_EWEB_DOC_UPLOAD_DIR_DEFAULT_VAL));
    
    abEWebSrv_CfgSetHttpIpPort(isHttps, pIpPort);
    adm_free(pIpPort);
    // abEWebSrv_CfgSetHttpPort(isHttps, adm_cfg_iniParserGetInt(ADM_CFG_EWEB_LISTENING_PORTS, ADM_CFG_EWEB_LISTENING_PORTS_DEFAULT_VAL));
    //!< 如果是HTTPS协议，需要设置证书路径
    if (TRUE == isHttps) {
        abEWebSrv_CfgSetCaCertificateFile(adm_cfg_iniParserGetString(ADM_CFG_EWEB_SSL_CA_PATH, ADM_CFG_EWEB_SSL_CERT_PATH_DEFAULT_VAL));
        abEWebSrv_CfgSetSSLCertificateFile(adm_cfg_iniParserGetString(ADM_CFG_EWEB_SSL_CERT_PATH, ADM_CFG_EWEB_SSL_CERT_PATH_DEFAULT_VAL));
        abEWebSrv_CfgSetSSLVerifyMode(adm_cfg_iniParserGetBoolean(ADM_CFG_EWEB_SSL_VERIFY_PEER, (DBOOL)ADM_CFG_EWEB_SSL_VERIFY_PEER_DEFAULT_VAL));
    }
    //!< 先关闭ewebserver
    abEWebSrv_Stop();

    /** 等待服务关闭OK */
    sleep(5);

    //!< 启动ewebserver
    iRet = abEWebSrv_Start();
    if (ADM_IS_ERR(iRet)) {
        log_e("Failed to start ewebserver in use thread method err[%d]", iRet);
        return iRet;
    }

#endif
    return ADM_ECODE_SUCCESSED;
}

/*!
 *******************************************************************************
 * ecu升级包分包处理
 *
 * \note 
 * 
 * \param[in]   pstTransPkg         待分包ecu升级包信息 
 * \param[in]   pstTpmTransReqParam tpm分包请求参数结构信息
 * \retval      D32S
 *******************************************************************************
 */
// TODO: 暂时默认 host(ewebserver) -> slave 
static D32S adm_tpm_transferEcuPackage
(
    ADM_TPS_TRANSFERPKG_T       *pstTransPkg,
    ADM_TPM_TRANSFER_REQPARAM_T *pstTransReqParam
)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    DCHAR szEventBuf[ADM_COM_LEN_256] = {0}; 

    if (ADM_IS_NULL(pstTransPkg) || ADM_IS_NULL(pstTransReqParam))
    {
        log_e("Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }

    //!< 下载源包
    if ((NULL != pstTransPkg->pstSrcPkgInfo) && 
        (ADM_TPS_TRANSPKG_STATE_SRCFINISH > pstTransPkg->stTransProcess.stResult.eTransState))
    {    
        log_i("Todo transfering src package");

        //mod by wl 20220321
        (void)adm_memzero(szEventBuf, sizeof(szEventBuf));
        (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), 
                        "src package info: ecuReqid[%s], ecuSwDid[%s], totalBytes[%lld], saveBytes[%lld], state[%d]", 
                        pstTransPkg->ecuReqid,
                        pstTransPkg->ecuSwDid,
                        pstTransPkg->pstSrcPkgInfo->size,
                        pstTransPkg->stTransProcess.stProgress.saveBytes,
                        pstTransPkg->stTransProcess.stResult.eTransState);
        adm_dpm_addEventAndReport(ADM_COM_MOD_TPM, FALSE, ADM_EVENT_ID_TI_004, ADM_OCS_EVENT_LEVEL_DEBUG, "vehicle transfer once ecu src package info.", szEventBuf);

        //!< 填充分包所需参数
        adm_tpm_cleanTransReqParamInfo(pstTransReqParam);
        iRet = __adm_tpm_fillTPMTransReqParam(ADM_PARTPKG_TYPE_SRC, pstTransPkg, pstTransReqParam);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to fill tpm transfer req param, err[%d]", iRet);
            
            //mod by wl 20220321
            (void)adm_memzero(szEventBuf, sizeof(szEventBuf));
            (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), 
                    "Failed to fill tpm transfer req param, ecuReqid[%s], ecuSwDid[%s], err[%d]", 
                    pstTransPkg->ecuReqid, pstTransPkg->ecuSwDid, iRet);
            adm_dpm_addEventAndReport(ADM_COM_MOD_TPM, TRUE, ADM_EVENT_ID_TI_005, ADM_OCS_EVENT_LEVEL_ERROR, "vehicle transfer once ecu src package info failed!", szEventBuf);
            return iRet;
        }

        iRet = adm_tpm_doTarnsfer(pstTransPkg->transferInstanceId, pstTransReqParam);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to trans package transInstId[%s], fileID[%s], err:[%d]", 
                                    pstTransPkg->transferInstanceId, pstTransReqParam->fileID, iRet);
            //mod by wl 20220321
            (void)adm_memzero(szEventBuf, sizeof(szEventBuf));
            (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), 
                    "Failed to trans package, ecuReqid[%s], ecuSwDid[%s], transInstId[%s], err[%d]", 
                    pstTransPkg->ecuReqid, pstTransPkg->ecuSwDid, pstTransPkg->transferInstanceId, iRet);
            adm_dpm_addEventAndReport(ADM_COM_MOD_TPM, TRUE, ADM_EVENT_ID_TI_005, ADM_OCS_EVENT_LEVEL_ERROR, "vehicle transfer once ecu src package info failed!", szEventBuf);
            return iRet; 
        }

        //mod by wl 20220321
        (void)adm_memzero(szEventBuf, sizeof(szEventBuf));
        (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), "once src package transfer successed, ecuReqid[%s], ecuSwDid[%s].", 
                pstTransPkg->ecuReqid, pstTransPkg->ecuSwDid);
        adm_dpm_addEventAndReport(ADM_COM_MOD_TPM, FALSE, ADM_EVENT_ID_TI_006, ADM_OCS_EVENT_LEVEL_DEBUG, "vehicle transfer once ecu src package info successed.", szEventBuf);
    }

    //!< 下载目的包
    log_i("Todo transfering dst package");

    //mod by wl 20220321
    (void)adm_memzero(szEventBuf, sizeof(szEventBuf));
    (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), 
                    "dst package info: ecuReqid[%s], ecuSwDid[%s], totalBytes[%lld], saveBytes[%lld], state[%d]", 
                    pstTransPkg->ecuReqid,
                    pstTransPkg->ecuSwDid,
                    pstTransPkg->stDstPkgInfo.size,
                    pstTransPkg->stTransProcess.stProgress.saveBytes - 
                        ((ADM_IS_NULL(pstTransPkg->pstSrcPkgInfo)) ? (0) : (pstTransPkg->pstSrcPkgInfo->size)),
                    pstTransPkg->stTransProcess.stResult.eTransState);
    adm_dpm_addEventAndReport(ADM_COM_MOD_TPM, FALSE, ADM_EVENT_ID_TI_007, ADM_OCS_EVENT_LEVEL_DEBUG, "vehicle transfer once ecu dst package info.", szEventBuf);
    
    //!< 填充分包所需参数
    adm_tpm_cleanTransReqParamInfo(pstTransReqParam);
    iRet = __adm_tpm_fillTPMTransReqParam(ADM_PARTPKG_TYPE_DST, pstTransPkg, pstTransReqParam);
    if (ADM_IS_ERR(iRet))
    {
        log_e("Failed to fill tpm transfer req param, err[%d]", iRet);

        //mod by wl 20220321
        (void)adm_memzero(szEventBuf, sizeof(szEventBuf));
        (void)adm_snprintf(szEventBuf, (D32U)sizeof(szEventBuf), 
                "Failed to fill tpm transfer req param, ecuReqid[%s], ecuSwDid[%s], err[%d]", 
                pstTransPkg->ecuReqid, pstTransPkg->ecuSwDid, iRet);
        adm_dpm_addEventAndReport(ADM_COM_MOD_TPM, TRUE, ADM_EVENT_ID_TI_008, ADM_OCS_EVENT_LEVEL_ERROR, "vehicle transfer once ecu dst package info failed!", szEventBuf);
        return iRet;
    }

    iRet = adm_tpm_doTarnsfer(pstTransPkg->transferInstanceId, pstTransReqParam);
    if (ADM_IS_ERR(iRet))
    {
        log_e("Failed to trans package transInstId[%s], fileID[%s], err:[%d]", 
                                pstTransPkg->transferInstanceId, pstTransReqParam->fileID, iRet);
        //mod by wl 20220321
        (void)adm_memzero(szEventBuf, sizeof(szEventBuf));
        (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), 
                "Failed to trans package, ecuReqid[%s], ecuSwDid[%s], transInstId[%s], err[%d]", 
                pstTransPkg->ecuReqid, pstTransPkg->ecuSwDid, pstTransPkg->transferInstanceId, iRet);
        adm_dpm_addEventAndReport(ADM_COM_MOD_TPM, TRUE, ADM_EVENT_ID_TI_008, ADM_OCS_EVENT_LEVEL_ERROR, "vehicle transfer once ecu dst package info failed!", szEventBuf);
        return iRet; 
    }

    //mod by wl 20220321
    (void)adm_memzero(szEventBuf, sizeof(szEventBuf));
    (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), "once dst package transfer successed, ecuReqid[%s], ecuSwDid[%s].", 
            pstTransPkg->ecuReqid, pstTransPkg->ecuSwDid);
    adm_dpm_addEventAndReport(ADM_COM_MOD_TPM, FALSE, ADM_EVENT_ID_TI_009, ADM_OCS_EVENT_LEVEL_DEBUG, "vehicle transfer once ecu dst package info successed.", szEventBuf);

    return iRet;
}

/**
 * @brief 分包处理升级包
 *
 * @param[in] bIsInIdel     描述: 是否闲时传输 \n
 * @param[in] pPackageNum   描述: 进行阐述的包的数量 \n
 * 范围值: 布尔类型 \n
 * 单位: N/A \n
 *
 * @return D32S
 */
static D32S adm_tpm_transferPackageProc(DBOOL bIsInIdel, D32S* pPackageNum)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    D32S i = 0;
    D32S tobeTransNum = 0;
    ADM_TPS_TRANSFERPKG_T       *pstTransPkgList = NULL;
    DCHAR szEventBuf[ADM_COM_LEN_256] = {0}; 
    D32S packageNum = 0;

    log_i("Entry transfer package proc.");

    //!< 避免后续循环中频繁malloc，减少内存碎片化
    pstTransPkgList = adm_tps_newTransferEcuPackageList(ADM_TPM_DEFTRANS_GETTOBELIST_CNT);
    if (ADM_IS_NULL(pstTransPkgList))
    {
        log_e("Failed to new ecu transfer package list, err[%d]", ADM_ECODE_NO_MEMORY);
        return ADM_ECODE_NO_MEMORY;
    }

    //!< 循环遍历待分包列表进行分包
    do
    {
        //mod by wl 20220321
        (void)adm_memzero(szEventBuf, sizeof(szEventBuf));
        adm_dpm_addEventAndReport(ADM_COM_MOD_TPM, FALSE, ADM_EVENT_ID_TI_001, ADM_OCS_EVENT_LEVEL_DEBUG, "vehicle get transfer ecu package info.", NULL);
    
        iRet = adm_tps_getTobeTransferEcuPackageInfo(ADM_TPM_DEFTRANS_GETTOBELIST_CNT, 
                    g_stIdelControl.bIsAllowIdelTrans, bIsInIdel, pstTransPkgList, &tobeTransNum);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to get tobe transfer ecu package, err[%d]", iRet);
            
            //mod by wl 20220321
            (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), "Failed to get tobe transfer ecu package, err[%d]", iRet);
            adm_dpm_addEventAndReport(ADM_COM_MOD_TPM, TRUE, ADM_EVENT_ID_TI_002, ADM_OCS_EVENT_LEVEL_ERROR, "vehicle get transfer ecu package info failed!", szEventBuf);
            break;
        }
        log_i("Tobe transfer package Num[%d]", tobeTransNum);

        //!< 记录传输包总个数
        packageNum += tobeTransNum;

        //mod by wl 20220321
        (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), "Tobe transfer package Num[%d]", tobeTransNum);
        adm_dpm_addEventAndReport(ADM_COM_MOD_TPM, FALSE, ADM_EVENT_ID_TI_003, ADM_OCS_EVENT_LEVEL_DEBUG, "vehicle get transfer ecu package info successed.", szEventBuf);

        for (i = 0; i < tobeTransNum; i++)
        {
            //传输过程监控到异常信息需要中断传输
            iRet = adm_tpm_checkTransferIsAbnormal();
            if (ADM_IS_ERR(iRet))
            {
                log_e("Notify transfer monitor abnormal failed to transfer part package, reqId[%s], swId[%s], err[%d]", 
                        pstTransPkgList[i].ecuReqid, pstTransPkgList[i].ecuSwDid, iRet);
                break;
            }

            //!< 分包单个ecu包信息
            log_i("[ABUP-Procedure] Transfer Ecu package Start, reqid[%s] swdid[%s]",pstTransPkgList[i].ecuReqid, pstTransPkgList[i].ecuSwDid);
            iRet = adm_tpm_transferEcuPackage(&pstTransPkgList[i], &gstTPMTransInfo.stTransReqParam);
            log_i("[ABUP-Procedure] Transfer Ecu package Start, ret[%d]",iRet);

            if (ADM_IS_ERR(iRet))
            {
                log_e("Failed to transfer package, reqId[%s], swId[%s], downloadInstId[%s], transInstId[%s], installInstId[%s], err[%d]", 
                    pstTransPkgList[i].ecuReqid, pstTransPkgList[i].ecuSwDid, pstTransPkgList[i].downloadInstanceId, 
                    pstTransPkgList[i].transferInstanceId, pstTransPkgList[i].installInstanceId, iRet);
                break;
            }
        }
        //!< 若for-break，则while-break
        if (ADM_IS_ERR(iRet))
        {
            break;
        }

        //!< 清空本次的内存脏数据
        adm_tps_cleanTransEcuPackageList(ADM_TPM_DEFTRANS_GETTOBELIST_CNT, pstTransPkgList);
    } while (0 < tobeTransNum);

    if (NULL != pPackageNum)
    {
        *pPackageNum = packageNum;
    }

    adm_tps_delTransferEcuPackageList(ADM_TPM_DEFTRANS_GETTOBELIST_CNT, &pstTransPkgList);

    return iRet;
}



#if 0
#endif

/*!
 *******************************************************************************
 * 比较是否需要传包函数
 *
 * \note pcDownloadInstanceId != pcinstallInstanceId 时 使用 transferInstanceId 分包
 * 
 * \param[in]  pcDownloadInstanceId  升级包的下载实例id
 * \param[in]  pctransferInstanceId  升级包的传输实例id
 * \retval     DBOOL                 true: 需要传输, false: 不需要传输
 *******************************************************************************
 */
DBOOL adm_tpm_cmpIsNeedTrans(DCHAR *pcDownloadInstanceId, DCHAR *pctransferInstanceId)
{

    if (ADM_IS_NULL(pcDownloadInstanceId) || ADM_IS_NULL(pctransferInstanceId))
    {
        log_e("Invalid input param");
        return FALSE;
    }

    //!< 下载实例id != 传输实例id
    if (0 != adm_strcasecmp(pcDownloadInstanceId, pctransferInstanceId))
    {
        return TRUE;
    }

    return FALSE;
}


#if 0
#endif

/*!
 *******************************************************************************
 * tpm模块初始化.
 *
 * \note
 *
 * \param   none
 * \retval  D32S    0: 成功 non-0：失败
 *******************************************************************************
 */
D32S adm_tpm_moduleInit(void)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    D32S errNum = 0;

    if (TRUE == gstTPMTransInfo.isInited)
    {
        log_i("dpm module already init.");
        return iRet;
    }

    do
    {
        g_stIdelControl.bIsAllowIdelTrans = adm_cfg_iniParserGetBoolean(ADM_CFG_VUS_IS_ALLOW_IDEL_TRANS, (DBOOL)ADM_CFG_VUS_IS_ALLOW_IDEL_TRANS_DEFAULT_VAL);

        iRet = pthread_mutex_init(&g_stIdelControl.stLock, NULL);
        if (ADM_IS_ERR(iRet))
        {
            iRet = ADM_ECODE_RWLOCK_INIT_FAILED;
            errNum = errno;
            log_e("Failed to init idel control rwlock, err[%d](%s)", iRet, strerror(errNum));
            break;
        }

        iRet = pthread_cond_init(&g_stIdelControl.stPauseCond, NULL);
        if (ADM_IS_ERR(iRet))
        {
            iRet = ADM_ECODE_RWLOCK_INIT_FAILED;
            errNum = errno;
            log_e("Failed to init idel control condLock, err[%d](%s)", iRet, strerror(errNum));
            break;
        }

        iRet = adm_tps_moduleInit(adm_tpm_cmpIsNeedTrans, adm_dpm_addEventAndReport, &g_stIdelControl);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to init tps module, err[%d]", iRet);
            break;
        }

        iRet = pthread_rwlock_init(&gstTPMTransInfo.stRWlock, NULL);
        if (ADM_IS_ERR(iRet))
        {
            iRet = ADM_ECODE_RWLOCK_INIT_FAILED;
            errNum = errno;
            log_e("Failed to init rwlock, err[%d](%s)", iRet, strerror(errNum));
            break;
        }

        gstTPMTransInfo.monitorHandle = NULL; 
        gstTPMTransInfo.isMonitoring  = FALSE;
        (void)adm_memzero(&gstTPMTransInfo.stTransReqParam, sizeof(gstTPMTransInfo.stTransReqParam));
        (void)adm_memzero(&gstTPMTransInfo.stUploadReqParam, sizeof(gstTPMTransInfo.stUploadReqParam));

        gstTPMTransInfo.isInited = TRUE;
    } while (FALSE);

    if (ADM_IS_ERR(iRet))
    {
        (void)adm_tpm_moduleDeInit();
    }

    return iRet;
}


/*!
 *******************************************************************************
 * tpm模块销毁.
 *
 * \note
 *
 * \param   none
 * \retval  D32S    0: 成功 non-0：失败
 *******************************************************************************
 */
D32S adm_tpm_moduleDeInit(void)
{

    if (TRUE == gstTPMTransInfo.isInited)
    {
        adm_tps_moduleDeInit();

        (void)pthread_mutex_destroy(&g_stIdelControl.stLock);
        (void)pthread_cond_destroy(&g_stIdelControl.stPauseCond);

        (void)pthread_rwlock_destroy(&gstTPMTransInfo.stRWlock);
        gstTPMTransInfo.monitorHandle = NULL; 
        gstTPMTransInfo.isMonitoring  = FALSE;
        adm_tpm_cleanTransReqParamInfo(&gstTPMTransInfo.stTransReqParam);
        adm_tpm_cleanUploadReqParamInfo(&gstTPMTransInfo.stUploadReqParam);

        gstTPMTransInfo.isInited = FALSE;
    }

    return ADM_ECODE_SUCCESSED;
}


/*!
 *******************************************************************************
 * 触发传包
 *
 * \note
 * 
 * \param[in] eTriggerType 触发预分包类型
 * \retval    D32S
 *******************************************************************************
 */
D32S adm_tpm_triggerTransferPkg
(
    ADM_APPLICATION_COMMON_CMDID_TYPE_E eTriggerType
)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    DCHAR szEventBuf[ADM_COM_LEN_256] = {0}; 
    D32S  i32WaitTransEcuNum = 0;
    DBOOL bIsForceUpdate = DFALSE;
    struct timespec tsNowbegin;
    struct timespec tsNowend;
    D32S iPackageNum = 0;
    D32S iNormalPackageNum = 0;
    D32S iIdelPackageNum = 0;

    (void)adm_memzero(szEventBuf, sizeof(szEventBuf));
    (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), "trigger vehicle transfer package, type[%d].", eTriggerType);
    adm_dpm_addEventAndReport(ADM_COM_MOD_TPM, FALSE, ADM_EVENT_ID_TT_001, ADM_OCS_EVENT_LEVEL_DEBUG, szEventBuf, ADM_TPM_MARKER_EV_SUB_BEGIN);
    
    log_i("Entry trigger transfer process.");
    log_i("[ABUP-Procedure] Entry trigger transfer process Start,eTriggerType[%d]",eTriggerType);

    //!< 进入预分包中 --- 其实应该是预分包准备 
    adm_dpm_setDownloadTransIng();

    do
    {
        gstTPMTransInfo.eTriggerType = eTriggerType;
        tpmSetIsMonitorAbnormal(FALSE);

        (void)adm_memzero(szEventBuf, sizeof(szEventBuf));

        if (ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_USB == eTriggerType)
        {
            bIsForceUpdate = DTRUE;
        }
        //!< 检查更新模块策略信息 
        log_d("Trigger check and update tps task.");
        log_i("[ABUP-Procedure] Check and update transfer info Start");
        iRet = adm_tps_checkAndUpdateTPSTransInfo(bIsForceUpdate,
                                                  adm_ecs_getConfigInfo, 
                                                  adm_dps_getDownloadTaskInfo, 
                                                  adm_dps_getTransCondition, 
                                                  adm_dps_getDownloadEcuPackageListInfo);
        if (ADM_IS_ERR(iRet))
        {   
            log_e("Failed to check and update tps info, err[%d].", iRet);
            (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), "Failed to check and update tps info, err[%d].", iRet);
            log_i("[ABUP-Procedure] Check and update transfer info End,ret[%d]",iRet);
            break;
        }
        log_i("[ABUP-Procedure] Check and update transfer info End,ret[%d]",iRet);

        (void)adm_dpm_vehicleStateReport2Cloud(ADM_IRM_REPORT_SERVER_STATUS_Task_Transfer_Prepare, 
                    adm_dpm_errorCode2String(ADM_ECODE_SUCCESSED));

        //!< 查看是否需要预分包
        log_d("Trigger try get wait trans Ecu num.");
        iRet = adm_tps_getWaitTransferEcuNum(&i32WaitTransEcuNum);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to get wait trans Ecu num, err[%d].", iRet);
            (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), "Failed to get wait trans Ecu num, err[%d].", iRet);
            break;
        }
        log_d("Trigger get wait trans Ecu num[%d].", i32WaitTransEcuNum);

        if (0 == i32WaitTransEcuNum)
        {
            iRet = ADM_ECODE_SUCCESSED;
            log_i("No ecu waiting for transmission.");
            (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), "No ecu waiting for transmission.");
            break;
        }

        //!< 检查预分包环境是否满足
        log_i("[ABUP-Procedure] Check transfer space Start");
        iRet = adm_tpm_checkTransferEnviroment();
        if (ADM_IS_ERR(iRet))
        {
            log_e( "Failed to check transfer enviroment, err[%d].", iRet);
            log_i("[ABUP-Procedure] Check transfer space End,ret[%d]",iRet);
            break;
        }
        log_i("[ABUP-Procedure] Check transfer space End,ret[%d]",iRet);

        //!< 检查预分包前置条件是否满足
        log_i("[ABUP-Procedure] Check transfer condition Start");
        iRet = adm_tpm_checkTransferPreCondition(eTriggerType);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to check transfer preCondition, err[%d].", iRet);
            (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), "Failed to check transfer preCondition, err[%d].", iRet);
            log_i("[ABUP-Procedure] Check transfer condition End,ret[%d]",iRet);
            break;
        }
        log_i("[ABUP-Procedure] Check transfer condition End,ret[%d]",iRet);

        //!< 启动分包过程监控
        log_i("[ABUP-Procedure] Start transfer process monitor Start");
        iRet = adm_tpm_startTransferProcessMonitor(__adm_tpm_transferMonitorAbnormalNotify, eTriggerType);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to start transfer monitor, err[%d].", iRet);
            (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), "Failed to start transfer monitor, err[%d].", iRet);
            log_i("[ABUP-Procedure] Start transfer process monitor End,ret[%d]",iRet);
            break;
        }
        log_i("[ABUP-Procedure] Start transfer process monitor End,ret[%d]",iRet);

        (void)adm_dpm_vehicleStateReport2Cloud(ADM_IRM_REPORT_SERVER_STATUS_Task_Transfer_Ing, 
                    adm_dpm_errorCode2String(ADM_ECODE_SUCCESSED));

        //!< 记录零件包下载的起始时间
        (void)clock_gettime(CLOCK_MONOTONIC, &tsNowbegin);

        //!< 分包处理 先处理普通包，后处理需要等待从控空闲进行传输的包,防止占用从控CPU
        iRet = adm_tpm_transferPackageProc(DFALSE, &iNormalPackageNum);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to transfer package proc in normal mode, err[%d]", iRet);
            (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), "Failed to transfer package proc, err[%d].", iRet);
            break;
        }

        /** 闲时传输启动处理 */
        tpmBeginTrans4Idel(&iIdelPackageNum);
        log_i("Normal transer ecu num[%d], Idel Transfer ecu num[%d]", iNormalPackageNum, iIdelPackageNum);

        if (iIdelPackageNum > 0)
        {
            iRet = adm_tpm_transferPackageProc(DTRUE, NULL);
            if (ADM_IS_ERR(iRet))
            {
                log_e("Failed to transfer package proc in idel mode, err[%d]", iRet);
                (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), "Failed to transfer package proc, err[%d].", iRet);
                break;
            }
        }

        /** 闲时传输终止处理 */
        tpmEndTransIdel(iIdelPackageNum);

        iPackageNum = iNormalPackageNum + iIdelPackageNum;

        if (ADM_IS_SUCCESS(iRet))
        {
            //!< 记录零件包下载的结束时间
            (void)clock_gettime(CLOCK_MONOTONIC, &tsNowend);
            (void)adm_memzero(szEventBuf, sizeof(szEventBuf));
            (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), "STATSPACK transAllTime:transTime[%d]%s, ecuNum[%d]", 
            ((tsNowend.tv_sec - tsNowbegin.tv_sec) < 1 ? (tsNowend.tv_nsec / (1000 * 1000)) - (tsNowbegin.tv_nsec / (1000 * 1000)) : (tsNowend.tv_sec - tsNowbegin.tv_sec)),
            ((tsNowend.tv_sec - tsNowbegin.tv_sec) < 1 ? "ms" : "s"),
            iPackageNum);
            adm_dpm_addEventAndReport(ADM_COM_MOD_DPM, FALSE,ADM_EVENT_ID_DL_014, ADM_OCS_EVENT_LEVEL_INFO, szEventBuf, NULL);
        }
    } while (FALSE);

    //!< 停止预分包过程监控
    log_i("[ABUP-Procedure] Stop transfer process monitor Start");
    adm_tpm_stopTransferProcessMonitor();
    log_i("[ABUP-Procedure] Stop transfer process monitor End");

    // TODO: 更新并通知下载结果    
    log_i("Return trigger transfer process, err[%d]", iRet);

    if (ADM_ECODE_SUCCESSED == iRet)
    {
        //!< 进入预分包成功  
        adm_dpm_setDownloadTransSuccessed();
        
        (void)adm_dpm_vehicleStateReport2Cloud(ADM_IRM_REPORT_SERVER_STATUS_Task_Transfer_Successed, 
                    adm_dpm_errorCode2String(iRet));

        adm_dpm_addEventAndReport(ADM_COM_MOD_TPM, TRUE, ADM_EVENT_ID_TE_003, ADM_OCS_EVENT_LEVEL_INFO, "vehicle transfer package successed.", ADM_TPM_MARKER_EV_SUB_END_PASS);
    }
    else if (ADM_ECODE_PAUSE_TRANSFER == tpmIstansferPause(iRet))
    {
        //!< 进入预分包暂停  
        adm_dpm_setDownloadTransPaused();

        (void)adm_dpm_vehicleStateReport2Cloud(ADM_IRM_REPORT_SERVER_STATUS_Task_Transfer_Paused, \
                    adm_dpm_errorCode2String(iRet));

        adm_dpm_addEventAndReport(ADM_COM_MOD_TPM, TRUE, ADM_EVENT_ID_TE_002, ADM_OCS_EVENT_LEVEL_INFO, "vehicle transfer package failed!", szEventBuf);
        adm_dpm_addEventAndReport(ADM_COM_MOD_TPM, TRUE, ADM_EVENT_ID_TE_002, ADM_OCS_EVENT_LEVEL_INFO, "vehicle transfer package failed!", ADM_TPM_MARKER_EV_SUB_END_FAIL);
    }
    else
    {
        //!< 进入预分包失败
        adm_dpm_setDownloadTransFailed();
            
        (void)adm_dpm_vehicleStateReport2Cloud(ADM_IRM_REPORT_SERVER_STATUS_Task_Transfer_Failed, \
                    adm_dpm_errorCode2String(iRet));
    
        adm_dpm_addEventAndReport(ADM_COM_MOD_TPM, TRUE, ADM_EVENT_ID_TE_002, ADM_OCS_EVENT_LEVEL_INFO, "vehicle transfer package failed!", szEventBuf);
        adm_dpm_addEventAndReport(ADM_COM_MOD_TPM, TRUE, ADM_EVENT_ID_TE_002, ADM_OCS_EVENT_LEVEL_INFO, "vehicle transfer package failed!", ADM_TPM_MARKER_EV_SUB_END_FAIL);
    }
    log_i("End transfer process, err[%d]", iRet);
    log_i("[ABUP-Procedure] Entry trigger transfer process End,ret[%d]",iRet);

    return iRet;
}

/**
 * @brief 启动ewebserver
 * @note U盘升级时，用于智能件的升级包下载
 * @param in 无
 * @retval D32S 0 成功，其他失败
 */
D32S adm_tpm_startUsbEwebServer(void)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    DBOOL isHttps = FALSE;
    DCHAR* pcUsbDownloadDir = DNULL;
    const DCHAR* pIp = NULL;
    D32S iPort = -1;
    DCHAR* pIpPort = NULL;

    //!< 设置ewebserver的协议方式 http or https; ip端口
    isHttps = adm_cfg_iniParserGetBoolean(ADM_CFG_EWEB_IS_HTTPS, (DBOOL)ADM_CFG_EWEB_IS_HTTPS_DEFAULT_VAL);
    pIp = adm_cfg_iniParserGetString(ADM_CFG_EWEB_LISTENING_IP, ADM_CFG_EWEB_LISTENING_IP_DEFAULT_VAL);
    iPort = adm_cfg_iniParserGetInt(ADM_CFG_EWEB_LISTENING_PORTS, ADM_CFG_EWEB_LISTENING_PORTS_DEFAULT_VAL);
    pIpPort = adm_com_createEwebIpPort(pIp, iPort);
    if (NULL == pIpPort)
    {
        log_e("Create eweb ip-port error.");
        iRet = ADM_ECODE_FAILED;
        return iRet;
    }
    
#ifdef CMAKE_CUSTOM_EWEBSERVER_SUPPORT
    /** 先关闭正常流程的ewebserver */
    abEWebSrv_Stop();
    log_e("stop ewebserver ok.");
    /** 等待服务关闭OK */
    sleep(5);
#endif

    //!< 设置下载的根目录和文件上传目录
    pcUsbDownloadDir = adm_cvs_getUsbDownloadDir();
    if (DNULL != pcUsbDownloadDir)
    {
        abEWebSrv_CfgSetDocumentDir(pcUsbDownloadDir,
                                    adm_cfg_iniParserGetString(ADM_CFG_EWEB_DOC_UPLOAD_DIR, ADM_CFG_EWEB_DOC_UPLOAD_DIR_DEFAULT_VAL));
        adm_free(pcUsbDownloadDir);
    }

    abEWebSrv_CfgSetHttpIpPort(isHttps, pIpPort);
    // abEWebSrv_CfgSetHttpPort(isHttps, adm_cfg_iniParserGetInt(ADM_CFG_EWEB_LISTENING_PORTS, ADM_CFG_EWEB_LISTENING_PORTS_DEFAULT_VAL));
    //!< 如果是HTTPS协议，需要设置证书路径
    if (TRUE == isHttps)
    {
        abEWebSrv_CfgSetCaCertificateFile(adm_cfg_iniParserGetString(ADM_CFG_EWEB_SSL_CA_PATH, ADM_CFG_EWEB_SSL_CERT_PATH_DEFAULT_VAL));
        abEWebSrv_CfgSetSSLCertificateFile(adm_cfg_iniParserGetString(ADM_CFG_EWEB_SSL_CERT_PATH, ADM_CFG_EWEB_SSL_CERT_PATH_DEFAULT_VAL));
        abEWebSrv_CfgSetSSLVerifyMode(adm_cfg_iniParserGetBoolean(ADM_CFG_EWEB_SSL_VERIFY_PEER, (DBOOL)ADM_CFG_EWEB_SSL_VERIFY_PEER_DEFAULT_VAL));
    }

    //!< 启动ewebserver
    iRet = abEWebSrv_Start();
    if (ADM_IS_ERR(iRet))
    {
        log_e("Failed to start ewebserver in use thread method err[%d]", iRet);
    }
    else
    {
        log_e("Start ewebserver in use thread method ok.");
    }

    return iRet;
}

/**
 * @brief 关闭ewebserver
 * @note U盘升级时，用于升级包下载结束后，升级包传输前
 * @param in 无
 * @retval D32S 0 成功，其他失败
 */
D32S adm_tpm_stopUsbEwebServer(void)
{
    D32S iRet = ADM_ECODE_SUCCESSED;

#ifdef CMAKE_CUSTOM_EWEBSERVER_SUPPORT
    /** 重新开启正常流程的ewebserver */
    iRet = adm_tpm_startEwebServer();
#else
    /** 使用API停止ewebserver */
    abEWebSrv_Stop();
#endif

    return iRet;
}

static DVOID* tpmHandleTransStartProc(DVOID* pArgv)
{
    D32S iRet = ADM_ECODE_SUCCESSED;

    (void)pthread_mutex_lock(&g_stIdelControl.stLock);

    if (g_stIdelControl.eIdelControl == IDEL_TRANSFER_PAUSE)
    {
        g_stIdelControl.bIsHandleIdelTrans4Start = DTRUE;

        sleep(2); ///< 防抖处理
        DBOOL bIsCondSatified = tpmCheckIdelTransCondition(DFALSE);
        if (DTRUE == bIsCondSatified)
        {
            log_i("Start idel trans.");
            iRet = adm_tpm_switchHvMode(VEHICLE_CTRL_ENTER_HV_MODE); ///< 上高压,上高压失败等待下一次条件满足传输文件
            if (ADM_IS_SUCCESS(iRet))
            {
                g_stIdelControl.eIdelControl = IDEL_TRANSFER_START;
                adm_tps_setIdelControlInfo(&g_stIdelControl);
                adm_tpm_operateIdelTrans(ADM_COTINUE_TRANS, adm_soa_getUclAdccInstanceId());

                pthread_cond_broadcast(&g_stIdelControl.stPauseCond);
            }
            else 
            {
                log_e("Entry hv mode failed.");
            } 
        }
        else   
        {
            log_i("Idel trans conditions do not satisfied. do not handle start.");
        }
        g_stIdelControl.bIsHandleIdelTrans4Start = DFALSE;
    }
    else
    {
        log_i("Do not handle.");
    }
    (void)pthread_mutex_unlock(&g_stIdelControl.stLock);

    return NULL;
}

static DVOID* tpmHandleTransPauseProc(DVOID* pArgv)
{
    (void)pthread_mutex_lock(&g_stIdelControl.stLock);
    if (g_stIdelControl.eIdelControl == IDEL_TRANSFER_START) ///< 防止其他线程占用锁资源修改
    {
        g_stIdelControl.bIsHandleIdelTrans4Pause = DTRUE;

        sleep(2); ///< 防抖处理
        DBOOL bIsCondSatified = tpmCheckIdelTransCondition(DFALSE);
        if (DFALSE == bIsCondSatified)
        {
            log_i("Pause idel trans.");
            g_stIdelControl.eIdelControl = IDEL_TRANSFER_PAUSE;
            adm_tps_setIdelControlInfo(&g_stIdelControl);
            adm_tpm_operateIdelTrans(ADM_PAUSE_TRANS, adm_soa_getUclAdccInstanceId());
            adm_tpm_switchHvMode(VEHICLE_CTRL_NO_ACTIVE_HV_MODE); ///< 下高压
        }
        else
        {
            log_i("Idel trans conditions satisfied. do not handle pause.");
        }
        g_stIdelControl.bIsHandleIdelTrans4Pause = DFALSE;
    }
    else
    {
        log_i("Do not handle.");
    }
    (void)pthread_mutex_unlock(&g_stIdelControl.stLock);

    return NULL;
}

typedef enum
{
    ADM_IDEL_TRASN_HANDLE_PAUSE,
    ADM_IDEL_TRASN_HANDLE_START
} ADM_IDEL_TRANS_HANDLE_E;

static DVOID tpmPreHandleIdelTrans(ADM_IDEL_TRANS_HANDLE_E eHandleType)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    D32S errNum = 0;
    pthread_attr_t stThreadAttr = {0};
    pthread_t pThreadHandle;

    /* 获取线程属性句柄 */
    iRet = pthread_attr_init(&stThreadAttr);
    if (ADM_IS_ERR(iRet))
    {
        errNum = errno;
        log_e("pthread_attr_init failed. err[%d], strerror[%s]", iRet, strerror(errNum));
        return;
    }

    /* 设置线程分离属性 */
    iRet = pthread_attr_setdetachstate(&stThreadAttr, PTHREAD_CREATE_DETACHED);
    if (ADM_IS_ERR(iRet))
    {
        errNum = errno;
        log_e("pthread_attr_setdetachstate failed. iRet[%u], strerror[%s]", iRet, strerror(errNum));
        (void)pthread_attr_destroy(&stThreadAttr);
        return;
    }

    if (ADM_IDEL_TRASN_HANDLE_PAUSE == eHandleType)
    {
        /** 处理待暂停的闲时传输 */ 
        iRet = pthread_create(&pThreadHandle,
                            &stThreadAttr,
                            tpmHandleTransPauseProc,
                            NULL);
    }
    else 
    {
        /** 处理待暂停的闲时传输 */ 
        iRet = pthread_create(&pThreadHandle,
                            &stThreadAttr,
                            tpmHandleTransStartProc,
                            NULL);
    }

    if (ADM_IS_ERR(iRet))
    {
        errNum = errno;
        log_e("Failed to create thread, err[%d], strerror[%s]", iRet, strerror(errNum));
        (void)pthread_attr_destroy(&stThreadAttr);
        return;
    }

    /* 销毁线程属性句柄 */
    (void)pthread_attr_destroy(&stThreadAttr);

    g_stIdelControl.bIsIdelTransMonitorStart = DTRUE;

    return;
}

DVOID adm_tpm_handleGearSignal(ADM_TPM_GEAR_E eGear)
{
    /** 不允许闲时传输 */
    if (DFALSE == g_stIdelControl.bIsAllowIdelTrans)
    {
        return;
    }

    log_i("Get gear signal[%d], current trans action[%d], function handle state start-pause[%d][%d], global handle status[%d].",
            eGear, g_stIdelControl.eIdelControl, g_stIdelControl.bIsHandleIdelTrans4Start, g_stIdelControl.bIsHandleIdelTrans4Pause, g_stIdelControl.bIshandleIdelTrans);

    if (DFALSE == g_stIdelControl.bIsIdelTransMonitorStart)
    {
        log_w("Transfer work in idel mode has not started.");
        return;
    }

    if (
            (GEAR_NONE_P == eGear && (DTRUE == g_stIdelControl.bIsHandleIdelTrans4Pause || DTRUE == g_stIdelControl.bIshandleIdelTrans)) ||
            (GEAR_P == eGear && (DTRUE == g_stIdelControl.bIsHandleIdelTrans4Start || DTRUE == g_stIdelControl.bIshandleIdelTrans))
        )
    {
        log_i("Do not handle.");
        return;
    }
    
    /** 创建新线程，不阻塞信号通知消息 */
    if (GEAR_NONE_P == eGear && g_stIdelControl.eIdelControl == IDEL_TRANSFER_START)
    {
        tpmPreHandleIdelTrans(ADM_IDEL_TRASN_HANDLE_PAUSE);
    }

    if (GEAR_P == eGear && g_stIdelControl.eIdelControl == IDEL_TRANSFER_PAUSE)
    {
        tpmPreHandleIdelTrans(ADM_IDEL_TRASN_HANDLE_START);
    }
        
    log_i("adm_tpm_handleGearSignal end.");

    return;
}