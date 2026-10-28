/**
 * @file    adm_dpm_report.c
 * @brief   dpm report msg to uim or cloud 
 * @details dpm 上报消息到 uim 或 cloud
 * @author  wangyanpeng
 * @version v0.1.0
 * @date    2020-11-06
 * @copyright www.abupdate.com
 */
#include "adm_dpm_log.h"
#include "adm_dpm_report.h"
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <pthread.h>

#include "adm_com_modDefine.h"
#include "adm_ocs_airProtocolAdapter.h"
#include "adm_dpm_uim.h"
#include "adm_dpm_reportCloud.h"
#include "adm_workqueue.h"

#include "adm_ecs_rsConfig.h"

//!< 上报间隔时间，单位 ms
#define ADM_DPM_REPORT_INTERNAL_TIME (1000)

/*!
 * @brief 上报信息
 */
typedef struct stDPMReportInfo
{
    DBOOL            isInited;      //!< 模块是否初始化

    ADM_LIB_Workqueue_T *pstDownReportQueue; //!< 下载上报队列    
} ADM_DPM_REPORTINFO_T;

static ADM_DPM_REPORTINFO_T gstDpmReportInfo = {0};


#if 0
#endif

/*!
 *******************************************************************************
 * 创建下载包结果信息通过ecu零件包结果信息
 *
 * \note  
 *
 * \param[in]   ecuReqid     ecu 诊断请求id
 * \param[in]   ecuSwDid     ecu 升级对象软件版本 DID
 * \param[in]   pstResult    单个ecu下载结果信息
 * \retval      ADM_DPS_DOWNPKGRESULT_T*
 *******************************************************************************
 */
static ADM_DPS_DOWNPKGRESULT_T *adm_dpm_createDownPkgResultFromPartPackage
(
    DCHAR                *ecuReqid,
    DCHAR                *ecuSwDid,
    ADM_DPS_DOWNRESULT_T *pstResult
)
{
    ADM_DPS_DOWNPKGRESULT_T *pstDownPkgResult = NULL;

    if (ADM_IS_NULL(ecuReqid) || ADM_IS_NULL(ecuSwDid) || ADM_IS_NULL(pstResult))
    {
        log_e("Invalid input param");
        return NULL;
    }
    
    //!< 填充相关结果信息
    pstDownPkgResult = adm_dps_newDownPackageResult();
    if (ADM_IS_NULL(pstDownPkgResult))
    {
        log_e("Failed to new down package progress info, err[%d]", ADM_ECODE_NO_MEMORY);
        return NULL;
    }

    //!< 单个ecu
    pstDownPkgResult->pstResultList = adm_dps_newDownPackageResultInfoList(1);
    if (ADM_IS_NULL(pstDownPkgResult->pstResultList))
    {
        adm_dps_delDownPackageResult(&pstDownPkgResult);
        log_e("Failed to new down package result info list, err[%d]", ADM_ECODE_NO_MEMORY);
        return NULL;
    }
    pstDownPkgResult->num = 1;

    pstDownPkgResult->pstResultList->ecuReqid = adm_strdup(ecuReqid);
    pstDownPkgResult->pstResultList->ecuSwDid = adm_strdup(ecuSwDid);
    pstDownPkgResult->pstResultList->stResult.result     = pstResult->result;
    pstDownPkgResult->pstResultList->stResult.eDownState = pstResult->eDownState;
    pstDownPkgResult->pstResultList->stResult.failReason = adm_strdup(pstResult->failReason);

    return pstDownPkgResult;
}


/*!
 *******************************************************************************
 * 创建下载包结果信息通过ecu集合包结果信息
 *
 * \note  
 *
 * \param[in]   fileName     集合包文件名
 * \param[in]   pstResult    单个ecu下载结果信息
 * \retval      ADM_DPS_DOWNRESULT_T*
 *******************************************************************************
 */
static ADM_DPS_DOWNPKGRESULT_T *adm_dpm_createDownPkgResultFromCombinePackage
(
    DCHAR                *fileName,
    ADM_DPS_DOWNRESULT_T *pstResult
)
{
    D32S i    = 0;
    D32S iRet = ADM_ECODE_SUCCESSED;
    ADM_DPS_ECUIDLIST_T stEcuIdList = {0};
    ADM_DPS_DOWNPKGRESULT_T *pstDownPkgResult = NULL;

    if (ADM_IS_NULL(fileName) || ADM_IS_NULL(pstResult))
    {
        log_e("Invalid input param");
        return NULL;
    }

    do
    {
        //!< 获取集合包中ecu信息
        (void)adm_memzero(&stEcuIdList, sizeof(stEcuIdList));
        iRet = adm_dps_getDownEcuCombinePackageEcuIdList(fileName, &stEcuIdList);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to get ecu combine package ecu id list info, err[%d]", iRet);
            break;
        }

        //!< 填充相关结果信息
        pstDownPkgResult = adm_dps_newDownPackageResult();
        if (ADM_IS_NULL(pstDownPkgResult))
        {
            log_e("Failed to new down package progress info, err[%d]", ADM_ECODE_NO_MEMORY);
            return NULL;
        }

        pstDownPkgResult->pstResultList = 
                    adm_dps_newDownPackageResultInfoList(stEcuIdList.num);
        if (ADM_IS_NULL(pstDownPkgResult->pstResultList))
        {
            iRet = ADM_ECODE_NO_MEMORY;
            log_e("Failed to new down package result info list, err[%d]", iRet);
            break;
        }
        pstDownPkgResult->num = stEcuIdList.num;

        for (i = 0; i < pstDownPkgResult->num; i++)
        {
            pstDownPkgResult->pstResultList[i].ecuReqid = 
                    adm_strdup(stEcuIdList.pstEcuIdInfo[i].ecuReqid);
            pstDownPkgResult->pstResultList[i].ecuSwDid = 
                    adm_strdup(stEcuIdList.pstEcuIdInfo[i].ecuSwDid);

            pstDownPkgResult->pstResultList[i].stResult.result     = pstResult->result;
            pstDownPkgResult->pstResultList[i].stResult.eDownState = pstResult->eDownState;
            pstDownPkgResult->pstResultList[i].stResult.failReason = adm_strdup(pstResult->failReason);
        }
    } while (FALSE);

    adm_dps_delEcuIdList((D32U)stEcuIdList.num, &stEcuIdList.pstEcuIdInfo);
    if (ADM_IS_ERR(iRet))
    {
        adm_dps_delDownPackageResult(&pstDownPkgResult);
        return NULL;
    }

    return pstDownPkgResult;
}


/*!
 *******************************************************************************
 * 创建下载包进度信息通过ecu零件包进度信息
 *
 * \note  
 *
 * \param[in]   ecuReqid     ecu 诊断请求id
 * \param[in]   ecuSwDid     ecu 升级对象软件版本 DID
 * \param[in]   pstProgress  单个ecu下载进度信息
 * \retval      ADM_DPS_DOWNPKGPROGRESS_T*
 *******************************************************************************
 */
static ADM_DPS_DOWNPKGPROGRESS_T *adm_dpm_createDownPkgProgressFromPartPackage
(
    DCHAR                  *ecuReqid,
    DCHAR                  *ecuSwDid,
    ADM_DPS_DOWNPROGRESS_T *pstProgress
)
{
    ADM_DPS_DOWNPKGPROGRESS_T *pstDownPkgProgress = NULL;

    if (ADM_IS_NULL(ecuReqid) || ADM_IS_NULL(ecuSwDid) || ADM_IS_NULL(pstProgress))
    {
        log_e("Invalid input param");
        return NULL;
    }
    
    //!< 填充相关进度信息
    pstDownPkgProgress = adm_dps_newDownPackageProgress();
    if (ADM_IS_NULL(pstDownPkgProgress))
    {
        log_e("Failed to new down package progress info, err[%d]", ADM_ECODE_NO_MEMORY);
        return NULL;
    }

    //!< 单个ecu
    pstDownPkgProgress->pstProgressList = adm_dps_newDownPackageProgressInfoList(1);
    if (ADM_IS_NULL(pstDownPkgProgress->pstProgressList))
    {
        adm_dps_delDownPackageProgress(&pstDownPkgProgress);
        log_e("Failed to new down package progress info list, err[%d]", ADM_ECODE_NO_MEMORY);
        return NULL;
    }
    pstDownPkgProgress->num = 1;

    pstDownPkgProgress->pstProgressList->ecuReqid = adm_strdup(ecuReqid);
    pstDownPkgProgress->pstProgressList->ecuSwDid = adm_strdup(ecuSwDid);
    (void)adm_memcpy(&pstDownPkgProgress->pstProgressList->stProgress, 
                pstProgress, sizeof(*pstProgress), 
                sizeof(pstDownPkgProgress->pstProgressList->stProgress));

    return pstDownPkgProgress;
}


/*!
 *******************************************************************************
 * 创建下载包进度信息通过集合包下载信息
 *
 * \note  
 *
 * \param[in]   fileName        集合包文件名
 * \param[in]   pstProgress     集合包下载进度信息
 * \retval      ADM_DPS_DOWNPKGPROGRESS_T*
 *******************************************************************************
 */
static ADM_DPS_DOWNPKGPROGRESS_T *adm_dpm_createDownPkgProgressFromCombinePackage
(
    DCHAR                  *fileName,
    ADM_DPS_DOWNPROGRESS_T *pstProgress
)
{
    D32S i    = 0;
    D32S iRet = ADM_ECODE_SUCCESSED;
    ADM_DPS_ECUIDLIST_T stEcuIdList = {0};
    ADM_DPS_DOWNPKGPROGRESS_T *pstDownPkgProgress = NULL;

    if (ADM_IS_NULL(fileName) || ADM_IS_NULL(pstProgress))
    {
        log_e("Invalid input param");
        return NULL;
    }

    do
    {
        //!< 获取集合包中ecu信息
        (void)adm_memzero(&stEcuIdList, sizeof(stEcuIdList));
        iRet = adm_dps_getDownEcuCombinePackageEcuIdList(fileName, &stEcuIdList);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to get ecu combine package ecu id list info, err[%d]", iRet);
            break;
        }

        //!< 填充相关进度信息
        pstDownPkgProgress = adm_dps_newDownPackageProgress();
        if (ADM_IS_NULL(pstDownPkgProgress))
        {
            iRet = ADM_ECODE_NO_MEMORY;
            log_e("Failed to new down package progress info, err[%d]", iRet);
            break;
        }

        pstDownPkgProgress->pstProgressList = 
                    adm_dps_newDownPackageProgressInfoList(stEcuIdList.num);
        if (ADM_IS_NULL(pstDownPkgProgress->pstProgressList))
        {
            iRet = ADM_ECODE_NO_MEMORY;
            log_e("Failed to new down package progress info list, err[%d]", iRet);
            break;
        }
        pstDownPkgProgress->num = stEcuIdList.num;

        for (i = 0; i < pstDownPkgProgress->num; i++)
        {
            pstDownPkgProgress->pstProgressList[i].ecuReqid = 
                    adm_strdup(stEcuIdList.pstEcuIdInfo[i].ecuReqid);
            pstDownPkgProgress->pstProgressList[i].ecuSwDid = 
                    adm_strdup(stEcuIdList.pstEcuIdInfo[i].ecuSwDid);

            (void)adm_memcpy(&pstDownPkgProgress->pstProgressList[i].stProgress, 
                       pstProgress, sizeof(*pstProgress), 
                       sizeof(pstDownPkgProgress->pstProgressList[i].stProgress));
        }
    } while (FALSE);

    adm_dps_delEcuIdList((D32U)stEcuIdList.num, &stEcuIdList.pstEcuIdInfo);
    if (ADM_IS_ERR(iRet))
    {
        adm_dps_delDownPackageProgress(&pstDownPkgProgress);
        return NULL;
    }

    return pstDownPkgProgress;
}


#if 0
#endif

/*!
 *******************************************************************************
 * 下载结果上报处理函数
 *
 * \note
 *
 * \param[in]   pstPkgAttr  下载文件属性
 * \param[in]   pstResult   下载结果信息
 * \retval      void
 *******************************************************************************
 */
void adm_dpm_downloadResultReportProc
(
    ADM_DPS_PKGATTR_T    *pstPkgAttr,
    ADM_DPS_DOWNRESULT_T *pstResult
)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    ADM_DPS_DOWNPKGRESULT_T *pstDownPkgResult = NULL;
    pthread_t stThreadHandle = {0};

    log_d("Download result report callback.");
    
    if (ADM_IS_NULL(pstPkgAttr) || ADM_IS_NULL(pstResult))
    {
        log_e("Invalid input param");
        return;
    }

    if (ADM_DPS_DOWNPACKAGE_COMBINE == pstPkgAttr->enPkgFormat)
    {
        pstDownPkgResult = adm_dpm_createDownPkgResultFromCombinePackage(
                                    pstPkgAttr->fileName,
                                    pstResult);
    }
    else if (ADM_DPS_DOWNPACKAGE_PART == pstPkgAttr->enPkgFormat)
    {
        pstDownPkgResult = adm_dpm_createDownPkgResultFromPartPackage(
                                    pstPkgAttr->ecuReqid,
                                    pstPkgAttr->ecuSwDid, 
                                    pstResult);
    }
    else
    {
        log_e("Invalid input param, input enPkgFormat[%d]", pstPkgAttr->enPkgFormat);
    }

    if (ADM_IS_NULL(pstDownPkgResult))
    {
        log_e("Failed to create down package result info, enPkgFormat[%d], err[%d]", 
                pstPkgAttr->enPkgFormat, iRet);
        return;
    }

    log_d("===> Down Result[%d]", pstDownPkgResult->pstResultList->stResult.result); 
    log_d("===> Down eDownState[%d]", pstDownPkgResult->pstResultList->stResult.eDownState);
    log_d("===> Down failReason[%s]", ADM_PRINT_STR(pstDownPkgResult->pstResultList->stResult.failReason));

    //!< 线程中进行 report 2 cloud(修改原因：树莓派结果上报UIM和云端函数耗时较大，导致结果通知响应sysi超时)
    iRet = adm_dpm_pthreadCreate(&stThreadHandle, adm_dpm_downResultReport2UimAndCloudProc, pstDownPkgResult);
    if (ADM_IS_ERR(iRet))
    {
        adm_dps_delDownPackageResult(&pstDownPkgResult);
        log_e("Failed to report down package result info 2 uim, err[%d]", iRet);
    }

    //iRet = adm_dpm_ecuDownResultNotify2Uim(pstDownPkgResult->pstResultList);
    //if (ADM_IS_ERR(iRet))
    //{
    //    log_e("Failed to report down package result info 2 uim, err[%d]", iRet);   
    //}

    //!< report 2 cloud
    //iRet = adm_dpm_downResultReport2Cloud(pstDownPkgResult);
    //if (ADM_IS_ERR(iRet))
    //{
    //    log_e("Failed to report down package result info 2 uim, err[%d]", iRet);
    //}
    //adm_dps_delDownPackageResult(&pstDownPkgResult);

    return;
}

/*!
 *******************************************************************************
 * 下载进度上报处理线程
 *
 * \note
 *
 * \param[in]   param   线程参数/通知回调函数
 * \retval      void 
 *******************************************************************************
 */
static void adm_dpm_downloadProgressReportThreadProc(void *param)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    ADM_DPS_DOWNPKGPROGRESS_T     *pstDownPkgProgress     = NULL;
    ADM_DPM_DOWNPROGRESSRPTNODE_T *pstDownProgressRptNode = NULL;

    log_d("Download progress report callback.");
    
    if (ADM_IS_NULL(param))
    {
        log_e("Invalid input param");
        return;
    }
    pstDownProgressRptNode = (ADM_DPM_DOWNPROGRESSRPTNODE_T *)param;

    if (0 != adm_dpm_getUimDownloadStatus())
    {
        log_e("download end donot deal download progress");
        adm_dpm_delDownProgressReportQueueNode(&pstDownProgressRptNode);
        return ;
    }
    
    if (ADM_DPS_DOWNPACKAGE_COMBINE == pstDownProgressRptNode->enPkgFormat)
    {
        pstDownPkgProgress = adm_dpm_createDownPkgProgressFromCombinePackage(
                                    pstDownProgressRptNode->fileName,
                                    &pstDownProgressRptNode->stProgress);
    }
    else if (ADM_DPS_DOWNPACKAGE_PART == pstDownProgressRptNode->enPkgFormat)
    {
        pstDownPkgProgress = adm_dpm_createDownPkgProgressFromPartPackage(
                                    pstDownProgressRptNode->ecuReqid,
                                    pstDownProgressRptNode->ecuSwDid, 
                                    &pstDownProgressRptNode->stProgress);
    }
    else
    {
        log_e("Invalid input param, input enPkgFormat[%d]", pstDownProgressRptNode->enPkgFormat);
    }

    if (ADM_IS_NULL(pstDownPkgProgress))
    {
        log_e("Failed to create down package progress info, enPkgFormat[%d], err[%d]", 
                pstDownProgressRptNode->enPkgFormat, iRet);
        adm_dpm_delDownProgressReportQueueNode(&pstDownProgressRptNode);
        return;
    }

    //!< report 2 uim
    //iRet = adm_dpm_allEcuDownProgressNotify2Uim(pstDownPkgProgress);
    if (ADM_IS_ERR(iRet))
    {
        log_e("Failed to report down package progress info 2 uim, err[%d]", iRet);   
    }

    iRet = adm_dpm_ecuDownProgressNotify2Uim(pstDownPkgProgress->pstProgressList);
    if (ADM_IS_ERR(iRet))
    {
        log_e("Failed to report down package progress info 2 uim, err[%d]", iRet);   
    }

    iRet = adm_dpm_policyDownProgressNotify2Uim();
    if (ADM_IS_ERR(iRet))
    {
        log_e("Failed to report down policy progress info 2 uim, err[%d]", iRet);   
    }

    //!< report 2 cloud
    iRet = adm_dpm_downProgressReport2Cloud(pstDownPkgProgress);
    if (ADM_IS_ERR(iRet))
    {
        log_e("Failed to report down package progress info 2 uim, err[%d]", iRet);   
    }

    adm_dps_delDownPackageProgress(&pstDownPkgProgress);
    adm_dpm_delDownProgressReportQueueNode(&pstDownProgressRptNode);

    return;
}


#if 0
#endif


/*!
 *******************************************************************************
 * 下载ecu进度上报
 *
 * \note   内部主要通知 uim/cloud 下载结果信息
 *             此函数不易频繁调用
 *
 * \param   none
 * \retval  D32
 *******************************************************************************
 */
D32S adm_dpm_downloadResultReport(void)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    ADM_DPS_DOWNPKGRESULT_T *pstDownPkgResult = NULL;

    pstDownPkgResult = adm_dps_getEcuDownloadResult();
    if (ADM_IS_NULL(pstDownPkgResult))
    {
        log_e("Failed to get cur download progress");
        return ADM_ECODE_FAILED;
    }

    // TODO: 通知 uim 
    iRet = adm_dpm_allEcuDownResultNotify2Uim(pstDownPkgResult);
    if (ADM_IS_ERR(iRet))
    {
        log_e("Failed to report uim, err[%d]", iRet);
    }

    // TODO: 通知 cloud 
    iRet = adm_dpm_downResultReport2Cloud(pstDownPkgResult);
    if (ADM_IS_ERR(iRet))
    {
        log_e("Failed to report cloud, err[%d]", iRet);
    }

    adm_dps_delDownPackageResult(&pstDownPkgResult);

    return iRet;
}


/*!
 *******************************************************************************
 * 下载ecu进度上报
 *
 * \note   内部主要通知 uim/cloud 当前下载进度信息
 *             此函数不易频繁调用
 *
 * \param   none
 * \retval  D32
 *******************************************************************************
 */
D32S adm_dpm_downloadProgressReport(void)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    ADM_DPS_DOWNPKGPROGRESS_T *pstDownPkgProgress = NULL;

    pstDownPkgProgress = adm_dps_getEcuDownloadProgress();
    if (ADM_IS_NULL(pstDownPkgProgress))
    {
        log_e("Failed to get cur download progress");
        return ADM_ECODE_FAILED;
    }

    // TODO: 通知 uim 
    iRet = adm_dpm_allEcuDownProgressNotify2Uim(pstDownPkgProgress);
    if (ADM_IS_ERR(iRet))
    {
        log_e("Failed to report uim, err[%d]", iRet);
    }

    // TODO: 通知 cloud 
    iRet = adm_dpm_downProgressReport2Cloud(pstDownPkgProgress);
    if (ADM_IS_ERR(iRet))
    {
        log_e("Failed to report cloud, err[%d]", iRet);
    }

    adm_dps_delDownPackageProgress(&pstDownPkgProgress);

    return iRet;
}

#if 0
#endif


/*!
 *******************************************************************************
 * 生成下载进度上报节点
 *
 * \note 
 *
 * \param   none
 * \retval  ADM_DPM_DOWNPROGRESSRPTNODE_T
 *******************************************************************************
 */
ADM_DPM_DOWNPROGRESSRPTNODE_T *adm_dpm_newDownProgressReportQueueNode(void)
{
    ADM_DPM_DOWNPROGRESSRPTNODE_T *pstDownProgressRptNode = NULL;

    pstDownProgressRptNode = (ADM_DPM_DOWNPROGRESSRPTNODE_T *)adm_malloc(sizeof(*pstDownProgressRptNode));
    if (NULL != pstDownProgressRptNode)
    {
        (void)adm_memzero(pstDownProgressRptNode, sizeof(*pstDownProgressRptNode));
    }

    return pstDownProgressRptNode;
}


/*!
 *******************************************************************************
 * 释放下载进度上报节点
 *
 * \note 
 *
 * \param[in]   pstDownProgressRptNode 下载进度上报节点
 * \retval      none
 *******************************************************************************
 */
void adm_dpm_delDownProgressReportQueueNode
(
    ADM_DPM_DOWNPROGRESSRPTNODE_T **pstDownProgressRptNode
)
{
    if (ADM_IS_NULL(pstDownProgressRptNode) || ADM_IS_NULL(*pstDownProgressRptNode))
    {
        return;
    }

    adm_free((*pstDownProgressRptNode)->ecuReqid);
    adm_free((*pstDownProgressRptNode)->ecuSwDid);
    adm_free((*pstDownProgressRptNode)->fileName);
    adm_free(*pstDownProgressRptNode);

    return;
}

/*!
 *******************************************************************************
 * 添加下载进度信息到上报队列
 *
 * \note 
 *
 * \param   none
 * \retval  D32
 *******************************************************************************
 */
D32S adm_dpm_addDownProgress2ReportQueue
(
    ADM_DPM_DOWNPROGRESSRPTNODE_T *pstDownProgressRptNode
)
{
    D32S iRet = ADM_ECODE_SUCCESSED;

    if (ADM_IS_NULL(pstDownProgressRptNode))
    {
        log_e("Invalid input param");
        return ADM_ECODE_FAILED;
    }

    if (FALSE == gstDpmReportInfo.isInited)
    {
        log_e( "DPM report module not init, err[%d]", ADM_ECODE_MODULE_UNINIT);
        return ADM_ECODE_MODULE_UNINIT;
    }

    iRet = adm_WorkqueueAdd(gstDpmReportInfo.pstDownReportQueue, (void *)pstDownProgressRptNode);
    if (ADM_IS_ERR(iRet))
    {
        log_e("Failed to add download progress to report queue, err[%d]", iRet);
        return iRet;
    }

    return iRet;
}


#if 0
#endif



/*!
 *******************************************************************************
 * dpm-uim-report 模块初始化.
 *
 * \note
 * 
 * \param  none
 * \retval D32S
 *******************************************************************************
 */
D32S adm_dpm_reportModuleInit(void)
{
    D32S iRet = ADM_ECODE_SUCCESSED;

    if (TRUE == gstDpmReportInfo.isInited)
    {
        log_i("dpm report module already init.");
        return iRet;
    }

    do
    {
        //!< 上报云信息 
        iRet = adm_dpm_reportCloudInit();
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to init download report module, err[%d]", iRet);
            break;
        }

        gstDpmReportInfo.pstDownReportQueue = 
                    adm_WorkqueueNew(1, adm_dpm_downloadProgressReportThreadProc); 
        if (ADM_IS_NULL(gstDpmReportInfo.pstDownReportQueue))
        {
            iRet = ADM_ECODE_NO_MEMORY;
            log_e("Failed to new download report queue, err[%d]", iRet);
            break;
        }

        gstDpmReportInfo.isInited = TRUE;
        adm_dpm_setUimDownloadStatus(0);
    } while (FALSE);

    if (ADM_IS_ERR(iRet))
    {
        (void)adm_dpm_reportModuleDeInit();
    }

    return iRet;
}


/*!
 *******************************************************************************
 * dpm-uim-report 模块销毁.
 *
 * \note
 * 
 * \param  none
 * \retval D32S
 *******************************************************************************
 */
D32S adm_dpm_reportModuleDeInit(void)
{
    D32S iRet = ADM_ECODE_SUCCESSED;

    adm_dpm_reportCloudDeInit();

    if (TRUE == gstDpmReportInfo.isInited)
    {
        gstDpmReportInfo.isInited = FALSE;
    
        if (NULL != gstDpmReportInfo.pstDownReportQueue)
        {
            (void)adm_WorkqueueDel(gstDpmReportInfo.pstDownReportQueue);
        }
    }

    return iRet;
}


