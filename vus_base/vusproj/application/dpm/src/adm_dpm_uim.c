/**
 * @file    adm_dpm_uim.c
 * @brief   dpm and uim msg
 * @details dpm 与 uim 交互消息
 * @author  wangyanpeng
 * @version v0.1.0
 * @date    2020-11-06
 * @copyright www.abupdate.com
 */
#include "adm_dpm_log.h"
#include "adm_dpm_uim.h"
#include "adm_com_modDefine.h"
#include "adm_cds_parse.h"
#include "adm_cvs_versionPolicy.h"
#include "adm_cvs_common.h"
#include "adm_cdm_verify_interface.h"
#include "adm_dpm_downloadManagement.h"
#include "adm_vsm_fsm.h"
#include "adm_vsm_uim.h"
#include "adm_uim_request.h"
#include "adm_dpm_reportCloud.h"
#include "adm_com_kvstorage.h"
#include <sys/stat.h>

//#include "adm_soa.h"
#include "adm_memory.h"


#if 0
#endif

/** 下载结束标志,1-下载成功 2-下载失败 3-下载暂停*/
static D32S g_DownloadEndStatus = 0;

/** 进度上报是否已结束*/
static DBOOL g_bIsReportEnd = DFALSE; 

D32S adm_dpm_getUimDownloadStatus()
{
    return g_DownloadEndStatus;
}

void adm_dpm_setUimDownloadStatus(D32S iDownloadStatus)
{
    g_DownloadEndStatus = iDownloadStatus;
    
    if (1 == iDownloadStatus && DFALSE == g_bIsReportEnd)
    {
        /**下载成功（结束）且未上报100进度时*/
        D32S i = 0;
        D32S iRet = ADM_ECODE_SUCCESSED;
        ADM_DPS_DOWNPKGPROGRESS_T            *pstDownPkgProgress   = NULL;
        ADM_UIM_SOA_DOWNLOAD_PROGRESS_INFO_T  stPolicyDownProgress = {0};

        do
        {
            pstDownPkgProgress = adm_dps_getEcuDownloadProgress();
            if (ADM_IS_NULL(pstDownPkgProgress))
            {
                iRet = ADM_ECODE_NO_MEMORY;
                log_e("Failed to get all ecu down progress info, err[%d]", iRet);
                break;
            }

            (void)adm_memzero(&stPolicyDownProgress, sizeof(stPolicyDownProgress));
            for (i = 0; i < pstDownPkgProgress->num; i++)
            {
                stPolicyDownProgress.speed      += pstDownPkgProgress->pstProgressList[i].stProgress.speed;
                stPolicyDownProgress.totalSize  += pstDownPkgProgress->pstProgressList[i].stProgress.totalBytes;
            }

            stPolicyDownProgress.savedSize = stPolicyDownProgress.totalSize;
            stPolicyDownProgress.remainTime = 0;
            stPolicyDownProgress.iTotalProgress = 100;
            
            // adm_uim_policyDownloadProgressNotify(adm_dpm_getDownInfo()->eTriggerType, 
            //                 "sessionId", 
            //                 "downloading", 
            //                 &stPolicyDownProgress);
        } while (FALSE);

        adm_dps_delDownPackageProgress(&pstDownPkgProgress);

        log_d("down policy progress notify uim, iRet[%d](0: ok, other: failed)", iRet);
    }
    else if (0 == iDownloadStatus)
    {
        g_bIsReportEnd = DFALSE;
    }
    else
    {
        ;/**do nothing*/
    }

    return;
}

/*!
 *******************************************************************************
 * uim触发下载动作.
 *
 * \note
 * 
 * \param[out]  pstRptParam 模块配置参数
 * \retval      D32S
 *******************************************************************************
 */
static void __adm_dpm_freeUimEcuDownProgressInfo
(
    ADM_UIM_SOA_ECU_DOWNLOAD_PROGRESS_INFO_T *pstUimEcuDownloadProgress
)
{
    if (ADM_IS_NULL(pstUimEcuDownloadProgress))
    {
        return;
    }
    
    if (NULL != pstUimEcuDownloadProgress->pstEcuIdenInfo)
    {
        adm_free(pstUimEcuDownloadProgress->pstEcuIdenInfo->name);
        adm_free(pstUimEcuDownloadProgress->pstEcuIdenInfo->partNum);
        adm_free(pstUimEcuDownloadProgress->pstEcuIdenInfo->softVer);
        adm_free(pstUimEcuDownloadProgress->pstEcuIdenInfo);
    }

    if (NULL != pstUimEcuDownloadProgress->pstEcuDownloadProgress)
    {
        adm_free(pstUimEcuDownloadProgress->pstEcuDownloadProgress);
    }

    adm_free(pstUimEcuDownloadProgress);

    return;
}


/*!
 *******************************************************************************
 * uim触发下载动作.
 *
 * \note
 * 
 * \param[out]  pstRptParam 模块配置参数
 * \retval      D32S
 *******************************************************************************
 */
static ADM_UIM_SOA_ECU_DOWNLOAD_PROGRESS_INFO_T *__adm_dpm_createEcuDownProgressByDpm
(
    ADM_DPS_DOWNPKGPROGRESSINFO_T *pstProgressInfo
)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    ADM_UIM_SOA_ECU_DOWNLOAD_PROGRESS_INFO_T *pstUimEcuDownloadProgress = NULL;

    if (ADM_IS_NULL(pstProgressInfo))
    {
        log_e("Invalid input param");
        return NULL;
    }

    do
    {
        pstUimEcuDownloadProgress = (ADM_UIM_SOA_ECU_DOWNLOAD_PROGRESS_INFO_T *)
                        adm_malloc(sizeof(*pstUimEcuDownloadProgress));
        if (ADM_IS_NULL(pstUimEcuDownloadProgress))
        {
            iRet = ADM_ECODE_NO_MEMORY;
            log_e("Failed to new uim ecu down progress info, err[%d]", iRet);
            break;
        }
        (void)adm_memzero(pstUimEcuDownloadProgress, sizeof(*pstUimEcuDownloadProgress));
        
        pstUimEcuDownloadProgress->pstEcuIdenInfo = (ADM_UIM_SOA_ECU_IDEN_INFO_T *)
                        adm_malloc(sizeof(*pstUimEcuDownloadProgress->pstEcuIdenInfo));
        if (ADM_IS_NULL(pstUimEcuDownloadProgress->pstEcuIdenInfo))
        {
            iRet = ADM_ECODE_NO_MEMORY;
            log_e("Failed to new uim ecu down progress ecu iden info, err[%d]", iRet);
            break;
        }

        pstUimEcuDownloadProgress->pstEcuDownloadProgress = (ADM_UIM_SOA_DOWNLOAD_PROGRESS_INFO_T *)
                        adm_malloc(sizeof(*pstUimEcuDownloadProgress->pstEcuDownloadProgress));
        if (ADM_IS_NULL(pstUimEcuDownloadProgress->pstEcuDownloadProgress))
        {
            iRet = ADM_ECODE_NO_MEMORY;
            log_e("Failed to new uim ecu down progress ecu progress info, err[%d]", iRet);
            break;
        }

        // TODO: debug data
        pstUimEcuDownloadProgress->pstEcuIdenInfo->partNum = NULL;
        pstUimEcuDownloadProgress->pstEcuIdenInfo->softVer = NULL;
        pstUimEcuDownloadProgress->pstEcuIdenInfo->name    = NULL;
        pstUimEcuDownloadProgress->pstEcuIdenInfo->swDid     = (D32S)htoi(pstProgressInfo->ecuSwDid);
        pstUimEcuDownloadProgress->pstEcuIdenInfo->requestId = (D32S)htoi(pstProgressInfo->ecuReqid);
        pstUimEcuDownloadProgress->pstEcuIdenInfo->responseId = 1;


        pstUimEcuDownloadProgress->pstEcuDownloadProgress->speed = 
            pstProgressInfo->stProgress.speed;
        pstUimEcuDownloadProgress->pstEcuDownloadProgress->savedSize = 
                    pstProgressInfo->stProgress.saveBytes;
        pstUimEcuDownloadProgress->pstEcuDownloadProgress->totalSize = 
                    pstProgressInfo->stProgress.totalBytes;
        pstUimEcuDownloadProgress->pstEcuDownloadProgress->remainTime = 
                    pstProgressInfo->stProgress.remainTime;
    } while (FALSE);
    
    if (ADM_IS_ERR(iRet))
    {
        __adm_dpm_freeUimEcuDownProgressInfo(pstUimEcuDownloadProgress);
    }

    return pstUimEcuDownloadProgress;
}

/*!
 *******************************************************************************
 * 清空uim ecu下载进度列表信息.
 *
 * \note 不释放最外层的节点内存
 * 
 * \param[inout]  pstEcuDownloadProgressArrayInfo uim ecu下载进度列表信息
 * \retval        D32S
 *******************************************************************************
 */
static void __adm_dpm_cleanUimEcuDownProgressList
(
    ADM_UIM_SOA_ECU_DOWNLOAD_PROGRESS_ARRAY_INFO_T *pstEcuDownloadProgressArrayInfo
)
{
    D32S i = 0;

    if (ADM_IS_NULL(pstEcuDownloadProgressArrayInfo))
    {
        return;
    }

    for (i = 0; (D16U)i < pstEcuDownloadProgressArrayInfo->ecuDownloadProgressArrayLength; i++)
    {
        __adm_dpm_freeUimEcuDownProgressInfo(pstEcuDownloadProgressArrayInfo->ecuDownloadProgressArray[i]);
    }
    adm_free(pstEcuDownloadProgressArrayInfo->ecuDownloadProgressArray);
    pstEcuDownloadProgressArrayInfo->ecuDownloadProgressArrayLength = (D16U)0;

    return;
}

/*!
 *******************************************************************************
 * 释放uim ecu下载进度列表信息.
 *
 * \note 
 * 
 * \param[inout]  pstEcuDownloadProgressArrayInfo uim ecu下载进度列表信息
 * \retval        D32S
 *******************************************************************************
 */
void adm_dpm_delUimEcuDownProgressList
(
    ADM_UIM_SOA_ECU_DOWNLOAD_PROGRESS_ARRAY_INFO_T **pstEcuDownloadProgressArrayInfo
)
{
    if (ADM_IS_NULL(pstEcuDownloadProgressArrayInfo) || ADM_IS_NULL(*pstEcuDownloadProgressArrayInfo))
    {
        return;
    }

    __adm_dpm_cleanUimEcuDownProgressList(*pstEcuDownloadProgressArrayInfo);
    adm_free(*pstEcuDownloadProgressArrayInfo);
    
    return;
}


/*!
 *******************************************************************************
 * 填充 uim 下载进度列表信息.
 *
 * \note
 * 
 * \param[in]   pstDownPkgProgress              dpm下载进度列表
 * \param[out]  pstEcuDownloadProgressArrayInfo soa下载进度列表
 * \retval      D32S
 *******************************************************************************
 */
static D32S __adm_dpm_fillUimEcuDownProgressList
(
    ADM_DPS_DOWNPKGPROGRESS_T *pstDownPkgProgress,
    ADM_UIM_SOA_ECU_DOWNLOAD_PROGRESS_ARRAY_INFO_T* pstEcuDownloadProgressArrayInfo
)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    D32S i = 0;

    if (ADM_IS_NULL(pstEcuDownloadProgressArrayInfo) || ADM_IS_NULL(pstDownPkgProgress))
    {
        log_e("Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }

    pstEcuDownloadProgressArrayInfo->ecuDownloadProgressArrayLength = 
            (D16U)pstDownPkgProgress->num;

    //!< 废物的设计, 很挫的数据结构, 该死的二级指针
    pstEcuDownloadProgressArrayInfo->ecuDownloadProgressArray = 
        (ADM_UIM_SOA_ECU_DOWNLOAD_PROGRESS_INFO_T **)adm_malloc((D32U)pstDownPkgProgress->num * sizeof(ADM_UIM_SOA_ECU_DOWNLOAD_PROGRESS_INFO_T *));
    if (ADM_IS_NULL(pstEcuDownloadProgressArrayInfo->ecuDownloadProgressArray))
    {
        log_e("Failed to malloc");
        pstEcuDownloadProgressArrayInfo->ecuDownloadProgressArrayLength = 0;
        return ADM_ECODE_NO_MEMORY;
    }

    for (i = 0; i < pstDownPkgProgress->num; i++)
    {
        pstEcuDownloadProgressArrayInfo->ecuDownloadProgressArray[i] = 
            __adm_dpm_createEcuDownProgressByDpm(&pstDownPkgProgress->pstProgressList[i]);
        if (ADM_IS_NULL(pstEcuDownloadProgressArrayInfo->ecuDownloadProgressArray[i]))
        {
            iRet = ADM_ECODE_NO_MEMORY;
            log_e("Failed to create ecut down progress by dpm progress, err[%d]", iRet);
            break;
        }
    }

    if (ADM_IS_ERR(iRet))
    {
        __adm_dpm_cleanUimEcuDownProgressList(pstEcuDownloadProgressArrayInfo);
    }

    return iRet;
}

#if 0
#endif

/*!
 *******************************************************************************
 * 释放uim下载结果结构信息.
 *
 * \note
 * 
 * \param[inout]  pstEcuDownloadResult uim下载结果信息
 * \retval        D32S
 *******************************************************************************
 */
static void __adm_dpm_freeUimEcuDownResultInfo
(
    ADM_UIM_SOA_ECU_DOWNLOAD_RESULT_INFO_T *pstEcuDownloadResult
)
{
    if (ADM_IS_NULL(pstEcuDownloadResult))
    {
        return;
    }
    
    if (NULL != pstEcuDownloadResult->pstEcuIdenInfo)
    {
        adm_free(pstEcuDownloadResult->pstEcuIdenInfo->name);
        adm_free(pstEcuDownloadResult->pstEcuIdenInfo->partNum);
        adm_free(pstEcuDownloadResult->pstEcuIdenInfo->softVer);
        adm_free(pstEcuDownloadResult->pstEcuIdenInfo);
    }

    adm_free(pstEcuDownloadResult->state);

    if (NULL != pstEcuDownloadResult->pstEcuDownloadResult)
    {
        adm_free(pstEcuDownloadResult->pstEcuDownloadResult->realVal);
        adm_free(pstEcuDownloadResult->pstEcuDownloadResult->expectVal);
        adm_free(pstEcuDownloadResult->pstEcuDownloadResult);
    }

    adm_free(pstEcuDownloadResult);

    return;
}


/*!
 *******************************************************************************
 * 创建uim ecu下载结果信息.
 *
 * \note
 * 
 * \param[in]  pstDownResultInfo dpm下载结果信息
 * \retval     D32S
 *******************************************************************************
 */
static ADM_UIM_SOA_ECU_DOWNLOAD_RESULT_INFO_T *__adm_dpm_createEcuDownResultByDpm
(
    ADM_DPS_DOWNPKGRESULTINFO_T *pstDownResultInfo
)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    ADM_UIM_SOA_ECU_DOWNLOAD_RESULT_INFO_T *pstUimEcuDownResultInfo = NULL;

    if (ADM_IS_NULL(pstDownResultInfo))
    {
        log_e("Invalid input param");
        return NULL;
    }

    do
    {
        pstUimEcuDownResultInfo = (ADM_UIM_SOA_ECU_DOWNLOAD_RESULT_INFO_T *)
                        adm_malloc(sizeof(*pstUimEcuDownResultInfo));
        if (ADM_IS_NULL(pstUimEcuDownResultInfo))
        {
            iRet = ADM_ECODE_NO_MEMORY;
            log_e("Failed to new uim ecu down result info, err[%d]", iRet);
            break;
        }

        pstUimEcuDownResultInfo->pstEcuIdenInfo = (ADM_UIM_SOA_ECU_IDEN_INFO_T *)
                        adm_malloc(sizeof(*pstUimEcuDownResultInfo->pstEcuIdenInfo));
        if (ADM_IS_NULL(pstUimEcuDownResultInfo->pstEcuIdenInfo))
        {
            iRet = ADM_ECODE_NO_MEMORY;
            log_e("Failed to new uim ecu down result ecu iden info, err[%d]", iRet);
            break;
        }

        pstUimEcuDownResultInfo->pstEcuDownloadResult = (ADM_UIM_SOA_DOWNLOAD_RESULT_INFO_T *)
                        adm_malloc(sizeof(*pstUimEcuDownResultInfo->pstEcuDownloadResult));
        if (ADM_IS_NULL(pstUimEcuDownResultInfo->pstEcuDownloadResult))
        {
            iRet = ADM_ECODE_NO_MEMORY;
            log_e("Failed to new uim ecu down result info, err[%d]", iRet);
            break;
        }

        // TODO: debug data
        pstUimEcuDownResultInfo->pstEcuIdenInfo->partNum = NULL;
        pstUimEcuDownResultInfo->pstEcuIdenInfo->softVer = NULL;
        pstUimEcuDownResultInfo->pstEcuIdenInfo->name    = NULL;
        pstUimEcuDownResultInfo->pstEcuIdenInfo->swDid     = (D32S)htoi(pstDownResultInfo->ecuSwDid);
        pstUimEcuDownResultInfo->pstEcuIdenInfo->requestId = (D32S)htoi(pstDownResultInfo->ecuReqid);
        pstUimEcuDownResultInfo->pstEcuIdenInfo->responseId = 1;

        pstUimEcuDownResultInfo->pstEcuDownloadResult->result = 
            pstDownResultInfo->stResult.result;
        pstUimEcuDownResultInfo->pstEcuDownloadResult->realVal = 
            adm_strdup(pstDownResultInfo->stResult.failReason);        
        pstUimEcuDownResultInfo->pstEcuDownloadResult->expectVal = 
            adm_strdup(pstDownResultInfo->stResult.failReason);

        pstUimEcuDownResultInfo->state = adm_strdup("ok"); 
            //adm_strdup(pstDownResultInfo->stResult.eDownState);   
    } while (FALSE);
    
    if (ADM_IS_ERR(iRet))
    {
        __adm_dpm_freeUimEcuDownResultInfo(pstUimEcuDownResultInfo);
    }

    return pstUimEcuDownResultInfo;
}

/*!
 *******************************************************************************
 * 清空uim ecu下载结果列表信息.
 *
 * \note 不释放最外层的节点内存
 * 
 * \param[inout]  pstUimEcuDownResultArrayInfo uim ecu下载结果列表信息
 * \retval        D32S
 *******************************************************************************
 */
static void __adm_dpm_cleanUimEcuDownResultList
(
    ADM_UIM_SOA_ECU_DOWNLOAD_RESULT_ARRAY_INFO_T *pstUimEcuDownResultArrayInfo
)
{
    D32S i = 0;

    if (ADM_IS_NULL(pstUimEcuDownResultArrayInfo))
    {
        return;
    }

    for (i = 0; (D16U)i < pstUimEcuDownResultArrayInfo->ecuDownloadResultArrayLength; i++)
    {
        __adm_dpm_freeUimEcuDownResultInfo(pstUimEcuDownResultArrayInfo->ecuDownloadResultArray[i]);
    }
    adm_free(pstUimEcuDownResultArrayInfo->ecuDownloadResultArray);
    pstUimEcuDownResultArrayInfo->ecuDownloadResultArrayLength = (D16U)0;

    return;
}

/*!
 *******************************************************************************
 * 释放uim ecu下载结果列表信息.
 *
 * \note 
 * 
 * \param[inout]  pstUimEcuDownResultArrayInfo uim ecu下载结果列表信息
 * \retval        D32S
 *******************************************************************************
 */
void adm_dpm_delUimEcuDownResultList
(
    ADM_UIM_SOA_ECU_DOWNLOAD_RESULT_ARRAY_INFO_T **pstUimEcuDownResultArrayInfo
)
{
    if (ADM_IS_NULL(pstUimEcuDownResultArrayInfo) || ADM_IS_NULL(*pstUimEcuDownResultArrayInfo))
    {
        return;
    }

    __adm_dpm_cleanUimEcuDownResultList(*pstUimEcuDownResultArrayInfo);
    adm_free(*pstUimEcuDownResultArrayInfo);
    
    return;
}

/*!
 *******************************************************************************
 * 填充 uim 下载结果列表信息.
 *
 * \note
 * 
 * \param[in]   pstDownPkgProgress              dpm下载进度列表
 * \param[out]  pstEcuDownloadProgressArrayInfo soa下载进度列表
 * \retval      D32S
 *******************************************************************************
 */
static D32S __adm_dpm_fillUimEcuDownResultList
(
    ADM_DPS_DOWNPKGRESULT_T *pstDownPkgResult,
    ADM_UIM_SOA_ECU_DOWNLOAD_RESULT_ARRAY_INFO_T *pstUimEcuDownResultArrayInfo
)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    D32S i = 0;

    if (ADM_IS_NULL(pstUimEcuDownResultArrayInfo) || ADM_IS_NULL(pstDownPkgResult))
    {
        log_e("Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }

    pstUimEcuDownResultArrayInfo->ecuDownloadResultArrayLength = 
            (D16U)pstDownPkgResult->num;

    //!< 废物的设计, 很挫的数据结构, 该死的二级指针
    pstUimEcuDownResultArrayInfo->ecuDownloadResultArray = 
        (ADM_UIM_SOA_ECU_DOWNLOAD_RESULT_INFO_T **)adm_malloc((D32U)pstDownPkgResult->num * sizeof(ADM_UIM_SOA_ECU_DOWNLOAD_RESULT_INFO_T *));
    if (ADM_IS_NULL(pstUimEcuDownResultArrayInfo->ecuDownloadResultArray))
    {
        log_e("Failed to malloc");
        pstUimEcuDownResultArrayInfo->ecuDownloadResultArrayLength = (D16U)0;
        return ADM_ECODE_NO_MEMORY;
    }

    for (i = 0; i < pstDownPkgResult->num; i++)
    {
        pstUimEcuDownResultArrayInfo->ecuDownloadResultArray[i] = 
            __adm_dpm_createEcuDownResultByDpm(&pstDownPkgResult->pstResultList[i]);
        if (ADM_IS_NULL(pstUimEcuDownResultArrayInfo->ecuDownloadResultArray[i]))
        {
            iRet = ADM_ECODE_NO_MEMORY;
            log_e("Failed to create ecut down result by dpm result, err[%d]", iRet);
            break;
        }
    }

    if (ADM_IS_ERR(iRet))
    {
        adm_dpm_delUimEcuDownResultList(&pstUimEcuDownResultArrayInfo);
    }

    return iRet;
}

#if 0
#endif

/*!
 *******************************************************************************
 * 打印uim ecu下载进度信息.
 *
 * \note
 * 
 * \param[out]  pstRptParam 模块配置参数
 * \retval      D32S
 *******************************************************************************
 */
static void __adm_dpm_debugUimAllEcuDownloadProgress
(
    ADM_UIM_SOA_ECU_DOWNLOAD_PROGRESS_ARRAY_INFO_T *pstEcuDownloadProgressArrayInfo
)
{

    D16U i = 0;
    ADM_UIM_SOA_ECU_DOWNLOAD_PROGRESS_INFO_T *pstEcuDownloadProgressArray = NULL;
    
    if (ADM_IS_NULL(pstEcuDownloadProgressArrayInfo))
    {
        log_e("Invalid input param");
        return;
    }

    log_e("===> all progress arrayNum[%d]", pstEcuDownloadProgressArrayInfo->ecuDownloadProgressArrayLength);
    for (i = 0; i < pstEcuDownloadProgressArrayInfo->ecuDownloadProgressArrayLength; i++)
    {
        pstEcuDownloadProgressArray = pstEcuDownloadProgressArrayInfo->ecuDownloadProgressArray[i];
        log_e("===> array[%d], partNum[%s]", i, ADM_PRINT_STR(pstEcuDownloadProgressArray->pstEcuIdenInfo->partNum));
        log_e("===> array[%d], name[%s]", i, ADM_PRINT_STR(pstEcuDownloadProgressArray->pstEcuIdenInfo->name));
        log_e("===> array[%d], softVer[%s]", i, ADM_PRINT_STR(pstEcuDownloadProgressArray->pstEcuIdenInfo->softVer));
        log_e("===> array[%d], requestId[%d]", i, pstEcuDownloadProgressArray->pstEcuIdenInfo->requestId);
        log_e("===> array[%d], responseId[%d]", i, pstEcuDownloadProgressArray->pstEcuIdenInfo->responseId);
        log_e("===> array[%d], swDid[%d]", i, pstEcuDownloadProgressArray->pstEcuIdenInfo->swDid);

        log_e("===> array[%d], speed[%d]", i, pstEcuDownloadProgressArray->pstEcuDownloadProgress->speed);
        log_e("===> array[%d], savedSize[%d]", i, pstEcuDownloadProgressArray->pstEcuDownloadProgress->savedSize);
        log_e("===> array[%d], totalSize[%d]", i, pstEcuDownloadProgressArray->pstEcuDownloadProgress->totalSize);
        log_e("===> array[%d], remainTime[%d]", i, pstEcuDownloadProgressArray->pstEcuDownloadProgress->remainTime);

        log_e("===> ");
    }

    return;
}


/*!
 *******************************************************************************
 * 打印uim ecu下载进度信息.
 *
 * \note
 * 
 * \param[out]  pstRptParam 模块配置参数
 * \retval      D32S
 *******************************************************************************
 */
static void __adm_dpm_debugUimAllEcuDownloadResult
(
    ADM_UIM_SOA_ECU_DOWNLOAD_RESULT_ARRAY_INFO_T *pstEcuDownloadResultArrayInfo
)
{

    D16U i = 0;
    ADM_UIM_SOA_ECU_DOWNLOAD_RESULT_INFO_T *pstEcuDownloadResultArray = NULL;
    
    if (ADM_IS_NULL(pstEcuDownloadResultArrayInfo))
    {
        log_e("Invalid input param");
        return;
    }

    log_e("===> all result arrayNum[%d]", pstEcuDownloadResultArrayInfo->ecuDownloadResultArrayLength);
    for (i = 0; i < pstEcuDownloadResultArrayInfo->ecuDownloadResultArrayLength; i++)
    {
        pstEcuDownloadResultArray = pstEcuDownloadResultArrayInfo->ecuDownloadResultArray[i];
        log_e("===> array[%d], partNum[%s]", i, ADM_PRINT_STR(pstEcuDownloadResultArray->pstEcuIdenInfo->partNum));
        log_e("===> array[%d], name[%s]", i, ADM_PRINT_STR(pstEcuDownloadResultArray->pstEcuIdenInfo->name));
        log_e("===> array[%d], softVer[%s]", i, ADM_PRINT_STR(pstEcuDownloadResultArray->pstEcuIdenInfo->softVer));
        log_e("===> array[%d], requestId[%d]", i, pstEcuDownloadResultArray->pstEcuIdenInfo->requestId);
        log_e("===> array[%d], responseId[%d]", i, pstEcuDownloadResultArray->pstEcuIdenInfo->responseId);
        log_e("===> array[%d], swDid[%d]", i, pstEcuDownloadResultArray->pstEcuIdenInfo->swDid);

        log_e("===> array[%d], result[%d]", i, pstEcuDownloadResultArray->pstEcuDownloadResult->result);
        log_e("===> array[%d], expectVal[%s]", i, ADM_PRINT_STR(pstEcuDownloadResultArray->pstEcuDownloadResult->expectVal));
        log_e("===> array[%d], realVal[%s]", i, ADM_PRINT_STR(pstEcuDownloadResultArray->pstEcuDownloadResult->realVal));

        log_e("===> ");
    }

    return;
}

#if 0
#endif

static DCHAR* adm_dpm_policyResultGetToKv(const DCHAR* policyResultKvKey)
{
    ADM_KV_STORAGE_VALUE_TYPE_E eType = ADM_KV_STORAGE_VALUE_TYPE_STRING;
	DCHAR* policyResultStr = NULL;
	size_t policyResultStrLen = 0;

   	if (TRUE == adm_com_kvstorage_get_string(adm_core_service_getBusinessDatabaseDb(),
                policyResultKvKey,
                &policyResultStr,
                &policyResultStrLen,
                &eType)) 
    {
		log_i("policyResultStr:%s", policyResultStr);
          
    }
	else 
	{
		log_w("policyResultStr not exist");
    }

	return policyResultStr;
}

static void adm_dpm_cleanPolicyResultFromKV()
{
    log_i("clean download PolicyResult info from kv");
    DBOOL iRet = adm_com_kvstorage_remove_key(adm_core_service_getBusinessDatabaseDb(), ADM_CVS_KV_PRIVACYPOLICY);
    if (iRet == FALSE)
    {
        log_i("Failed to remove key");
        return ;
    }
    return ;
}

/*!
 *******************************************************************************
 * 策略下载结果上报
 *
 * \note 注意时总升级包进度信息，傻x才会这样定义进度的上报格式
 *
 * \param[in] pstResult 下载结果信息
 * \retval    D32
 *******************************************************************************
 */
D32S adm_dpm_policyDownResultNotify2Uim
(
    ADM_DPS_DOWNRESULT_T *pstResult,
    const DCHAR* pState
)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    D32S iRespCode = 0;
    ADM_DPS_DOWNTSAKINFO_T stDownTaskInfo = {0};
    ADM_UIM_SOA_DOWNLOAD_RESULT_INFO_T stPolicyDownResult = {0};
    DCHAR szEventBuf[ADM_COM_LEN_256] = {0};
	DCHAR* policyResult = NULL;
    
    if (ADM_IS_NULL(pstResult))
    {
        log_e("Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }

    do
    {
        //!< 获取下载任务信息 
        (void)adm_memzero(&stDownTaskInfo, sizeof(stDownTaskInfo));
        iRet = adm_dps_getDownloadTaskInfo(&stDownTaskInfo);
        if (ADM_IS_ERR(iRet))
        {
            log_e( "Failed to get version policy, err[%d]", iRet);
            break;
        }
    
        stPolicyDownResult.result    = (pstResult->result);
        stPolicyDownResult.realVal   = adm_strdup(pstResult->failReason);
        stPolicyDownResult.expectVal = adm_strdup("OK");
		policyResult = adm_dpm_policyResultGetToKv(ADM_CVS_KV_PRIVACYPOLICY);
		if(ADM_ECODE_SUCCESSED == adm_strcmp(policyResult, "success"))
		{
			stPolicyDownResult.existPrivatePolicy = 0;
		}
		else
		{
			stPolicyDownResult.existPrivatePolicy = 1;
		}

        if(TRUE == adm_vsm_getHmiOnline())
        {
            adm_uim_hmi_policy_downloadResult_RR(adm_dpm_getDownInfo()->eTriggerType, 
                            stDownTaskInfo.sessionId, 
                            pState, 
                            &stPolicyDownResult,
                            &iRespCode);
            if (ADM_IS_ERR(iRespCode))
            {
                iRet = iRespCode;
                log_e("Failed to notify uim download policy result, err[%d]", iRespCode);
                (void)adm_memzero(szEventBuf, sizeof(szEventBuf));
                (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), "Failed to notify uim download policy result, RespCode[%d]",  iRespCode);
                adm_dpm_addEventAndReport(ADM_COM_MOD_DPM, TRUE, ADM_EVENT_ID_DI_065, ADM_OCS_EVENT_LEVEL_ERROR, " download policy result notify failed!", szEventBuf);
                break;
            }
			else
			{
				/**通知成功,删除记录的文件*/
				adm_dpm_cleanPolicyResultFromKV();
			}
        }
    } while (FALSE);

    adm_free(stPolicyDownResult.realVal);
    adm_free(stPolicyDownResult.expectVal);
    adm_dps_cleanDownTaskInfo(&stDownTaskInfo);

    log_d("down policy result notify uim, iRet[%d](0: ok, other: failed)", iRet);

    return iRet;
}


D32S g_SleepTime = 0;
D32S g_DownloadProgressSimulate = 0;
DBOOL g_StartSimulateThread = FALSE;
DBOOL g_isRuningSimulateProcess = FALSE;
pthread_mutex_t g_MutexStopSimulateThread=PTHREAD_MUTEX_INITIALIZER;
D32S adm_dpm_getDownloadProgressSimInfoFromKV(KvStorage_t *pstOtaKvHandle,ADM_DPM_PROGRESS_SIMULATE_INFO_T **pstProgressSimInfo)
{
    char *json_str = NULL;
    ADM_KV_STORAGE_VALUE_TYPE_E eValType;
    size_t valLength = 0;
    DBOOL ret = adm_com_kvstorage_get_string(pstOtaKvHandle, ADM_DPM_DOWNLOADPROGRESS_KV_KEY, &json_str,&valLength,&eValType);
    if (ret == FALSE || ADM_IS_NULL(json_str)) {
        log_e("Failed to get data from KV storage");
        return ADM_ECODE_FAILED;
    }

    log_i("get data from KV storage success json is [%s]",json_str);

    struct json_object *root = json_tokener_parse(json_str);
    if (ADM_IS_NULL(root)) {
        log_e("Failed to parse JSON data");
        adm_free(json_str);
        return ADM_ECODE_FAILED;
    }

    *pstProgressSimInfo = (ADM_DPM_PROGRESS_SIMULATE_INFO_T *)adm_malloc(sizeof(ADM_DPM_PROGRESS_SIMULATE_INFO_T));
    if (ADM_IS_NULL(*pstProgressSimInfo)) {
        log_e("Failed to allocate memory");
        json_object_put(root);
        adm_free(json_str);
        return ADM_ECODE_NO_MEMORY;
    }

    struct json_object *downloadProgressSimulate_obj;
    if (json_object_object_get_ex(root, "downloadProgressSimulate", &downloadProgressSimulate_obj)) {
        (*pstProgressSimInfo)->downloadProgressSimulate = json_object_get_int(downloadProgressSimulate_obj);
    }

    struct json_object *sleepTime_obj;
    if (json_object_object_get_ex(root, "sleepTime", &sleepTime_obj)) {
        (*pstProgressSimInfo)->sleepTime = json_object_get_int(sleepTime_obj);
    }

    log_i("get data is success  downloadProgressSimulate[%d]  sleepTime[%d]",(*pstProgressSimInfo)->downloadProgressSimulate,(*pstProgressSimInfo)->sleepTime);
    
    json_object_put(root);
    adm_free(json_str);
    return ADM_ECODE_SUCCESSED;
}
D32S adm_dpm_setDownloadProgressSimInfoToKV(KvStorage_t *pstOtaKvHandle,ADM_DPM_PROGRESS_SIMULATE_INFO_T *pstProgressSimInfo)
{
    log_i("set data to KV storage downloadProgressSimulate is [%d] sleepTime is [%d]",(pstProgressSimInfo)->downloadProgressSimulate,(pstProgressSimInfo)->sleepTime);
    if (ADM_IS_NULL(pstOtaKvHandle) || ADM_IS_NULL(pstProgressSimInfo)) {
        log_e("Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }

    struct json_object *root = json_object_new_object();
    if (ADM_IS_NULL(root)) {
        log_e("Failed to create JSON object");
        return ADM_ECODE_FAILED;
    }

    struct json_object *downloadProgressSimulate_obj = json_object_new_int((pstProgressSimInfo)->downloadProgressSimulate);
    json_object_object_add(root, "downloadProgressSimulate", downloadProgressSimulate_obj);

    struct json_object *sleepTime_obj = json_object_new_int((pstProgressSimInfo)->sleepTime);
    json_object_object_add(root, "sleepTime", sleepTime_obj);

    char *json_str = adm_strdup(json_object_to_json_string_ext(root, JSON_C_TO_STRING_SPACED));
    if (ADM_IS_NULL(json_str)) {
        log_e("Failed to convert JSON object to string");
        json_object_put(root);
        return ADM_ECODE_FAILED;
    }

    DBOOL ret = adm_com_kvstorage_set_string(pstOtaKvHandle, ADM_DPM_DOWNLOADPROGRESS_KV_KEY, json_str,ADM_KV_STORAGE_VALUE_TYPE_STRING);
    if (ret == FALSE) {
        log_e("Failed to set data to KV storage");
        return ADM_ECODE_FAILED;
    }

    log_i("set data to KV storage success json is [%s]",json_str);

    json_object_put(root);
    adm_free(json_str);
    return ADM_ECODE_SUCCESSED;
}
D32S adm_dpm_cleanDownloadProgressSimInfoFromKV()
{
    log_i("clean download progress simulate info from kv");
    DBOOL iRet = adm_com_kvstorage_remove_key(adm_core_service_getBusinessDatabaseDb(),ADM_DPM_DOWNLOADPROGRESS_KV_KEY);
    if (iRet == FALSE)
    {
        log_i("Failed to remove key");
        return ADM_ECODE_FAILED;
    }
    g_SleepTime = 0;
    g_DownloadProgressSimulate = 0;
    log_i("remove key success");
    return ADM_ECODE_SUCCESSED;
}
void* adm_dpm_downloadProgressSimulate(void * arg)
{
    log_i("start download progress simulate thread");
    KvStorage_t* pstOtakvHandle = adm_core_service_getBusinessDatabaseDb();
    g_SleepTime = 30;
    g_DownloadProgressSimulate = 91;
    ADM_DPM_PROGRESS_SIMULATE_INFO_T *pstProgressSimInfo = (ADM_DPM_PROGRESS_SIMULATE_INFO_T *)arg;
    if (pstProgressSimInfo != NULL)
    {
        g_DownloadProgressSimulate = pstProgressSimInfo->downloadProgressSimulate;
        g_SleepTime = pstProgressSimInfo->sleepTime;
    }

    ADM_UIM_SOA_DOWNLOAD_PROGRESS_INFO_T  stPolicyDownProgress = {0};
    stPolicyDownProgress.remainTime = 0;
    stPolicyDownProgress.savedSize = 0;
    stPolicyDownProgress.speed = 0;
    stPolicyDownProgress.totalSize = 0;
        
    
    ADM_APPLICATION_COMMON_CMDID_TYPE_E eTriggerType = adm_dpm_getDownInfo()->eTriggerType;

    do
    {
        log_i("into while loop g_DownloadProgressSimulate [%d] g_SleepTime [%d]",g_DownloadProgressSimulate,g_SleepTime);
        pthread_mutex_lock(&g_MutexStopSimulateThread);
        if (!g_StartSimulateThread)
        {
            pthread_mutex_unlock(&g_MutexStopSimulateThread);
            log_i("stop download progress simulate thread");
            break;
        }
        pthread_mutex_unlock(&g_MutexStopSimulateThread);
        
        stPolicyDownProgress.iTotalProgress = g_DownloadProgressSimulate;       

        ADM_DPM_PROGRESS_SIMULATE_INFO_T pstProgressSimInfoToKV = {0};
        pstProgressSimInfoToKV.downloadProgressSimulate = g_DownloadProgressSimulate;
        pstProgressSimInfoToKV.sleepTime = g_SleepTime;
        log_i("1 download progress simulate thread progressSimulate[%d] sleepTime[%d]",stPolicyDownProgress.iTotalProgress,g_SleepTime);
        adm_uim_policyDownloadProgressNotify(eTriggerType,"sessionId", "downloading", &stPolicyDownProgress);
        
        D32S ret =  adm_dpm_setDownloadProgressSimInfoToKV(pstOtakvHandle,&pstProgressSimInfoToKV);
        if (ret != ADM_ECODE_SUCCESSED)
        {
            log_e("Failed to set download progress simulate info to kv");
        }

        g_DownloadProgressSimulate++;
        sleep(g_SleepTime);

        log_i("2 download progress simulate thread progressSimulate[%d] sleepTime[%d]",stPolicyDownProgress.iTotalProgress,g_SleepTime);
    } while (g_DownloadProgressSimulate <= 100);
    adm_dpm_setGStartSimulateThread(FALSE);
    if (pstProgressSimInfo != NULL)
    {
        log_i("free pstProgressSimInfo");
        adm_free(pstProgressSimInfo);
    }
    
    log_i("end download progress simulate thread");

    return NULL;
}

void adm_dpm_setGStartSimulateThread(DBOOL flag)
{
    log_i("set g_StartSimulateThread to [%d]",flag);
    pthread_mutex_lock(&g_MutexStopSimulateThread);
    g_StartSimulateThread = flag;
    pthread_mutex_unlock(&g_MutexStopSimulateThread);
}

void adm_dpm_stopDownloadProgressSimulate()
{
    log_i("start... stop download progress simulate thread");
    if (g_isRuningSimulateProcess == FALSE && g_StartSimulateThread == FALSE)
    {
        log_i("download progress simulate process is not running");
        return;
    }
    
    KvStorage_t* pstOtakvHandle = adm_core_service_getBusinessDatabaseDb();
    //此处需要确认停止子线程，剩余进度由主流程线程上报。
    adm_dpm_setGStartSimulateThread(FALSE);
    g_SleepTime = 1;
    ADM_UIM_SOA_DOWNLOAD_PROGRESS_INFO_T  stPolicyDownProgress = {0};
    stPolicyDownProgress.remainTime = 0;
    stPolicyDownProgress.savedSize = 0;
    stPolicyDownProgress.speed = 0;
    stPolicyDownProgress.totalSize = 0;
    do
    {
        stPolicyDownProgress.iTotalProgress = g_DownloadProgressSimulate;
        adm_uim_policyDownloadProgressNotify(adm_dpm_getDownInfo()->eTriggerType,"sessionId", "downloading", &stPolicyDownProgress);
        // 保存数据库
        sleep(g_SleepTime);

        ADM_DPM_PROGRESS_SIMULATE_INFO_T pstProgressSimInfoToKV = {0};
        pstProgressSimInfoToKV.downloadProgressSimulate = g_DownloadProgressSimulate;
        pstProgressSimInfoToKV.sleepTime = g_SleepTime;
        D32S ret =  adm_dpm_setDownloadProgressSimInfoToKV(pstOtakvHandle,&pstProgressSimInfoToKV);
        if (ret != ADM_ECODE_SUCCESSED)
        {
            log_e("Failed to set download progress simulate info to kv");
        }

        g_DownloadProgressSimulate++;
    } while (g_DownloadProgressSimulate <= 100);
    
    adm_dpm_cleanDownloadProgressSimInfoFromKV();
    g_isRuningSimulateProcess = FALSE;
    log_i("end... stop download progress simulate thread");

    return ;
}
void adm_dpm_startDownloadProgressSimulate(DBOOL isResumeStart)
{
    log_i("start download progress simulate thread resumeStart [%d]",isResumeStart);
    
    pthread_mutex_lock(&g_MutexStopSimulateThread);
    DBOOL isStartSimulateThread = g_StartSimulateThread;
    pthread_mutex_unlock(&g_MutexStopSimulateThread);
    KvStorage_t* pstOtakvHandle = adm_core_service_getBusinessDatabaseDb();
    DBOOL keyExists = adm_com_kvstorage_has_key(pstOtakvHandle,ADM_DPM_DOWNLOADPROGRESS_KV_KEY);
    ADM_DPM_PROGRESS_SIMULATE_INFO_T *pstProgressSimInfo = NULL;
    D32S ret = ADM_ECODE_SUCCESSED;
    // 首先判断一下是哪种启动，如果是恢复启动，那么就看数据库是否有数据，有数据则启动线程，没有数据则不启动线程。进行下载进度模拟。
    if (isResumeStart) 
    {
        if (keyExists) 
        {
            // 从数据库读取数据
            ret = adm_dpm_getDownloadProgressSimInfoFromKV(pstOtakvHandle, &pstProgressSimInfo);
            if (ADM_ECODE_SUCCESSED == ret && pstProgressSimInfo && !isStartSimulateThread && !g_isRuningSimulateProcess) 
            {
                // 启动线程
                pthread_t threadHandle;
                ret = adm_dpm_pthreadCreate(&threadHandle, adm_dpm_downloadProgressSimulate, (void *)pstProgressSimInfo);
                if (ADM_ECODE_SUCCESSED != ret) 
                {
                    log_e("Failed to create download progress simulate thread");
                }
                adm_dpm_setGStartSimulateThread(TRUE);
                g_isRuningSimulateProcess = TRUE;
            } 
            else 
            {
                log_e("Failed to get download progress simulate info from KV");
            }
        } 
        else 
        {
            log_i("No download progress simulate info in KV, not starting thread");
        }
    } 
    else 
    {
        if (keyExists || isStartSimulateThread || g_isRuningSimulateProcess) 
        {
            log_i("Download progress simulate info exists in KV or starting thread");
        } 
        else 
        {
            // 启动线程
            pthread_t threadHandle;
            ret = adm_dpm_pthreadCreate(&threadHandle, adm_dpm_downloadProgressSimulate, NULL);
            if (ADM_ECODE_SUCCESSED != ret) 
            {
                log_e("Failed to create download progress simulate thread");
            }
            adm_dpm_setGStartSimulateThread(TRUE);
            g_isRuningSimulateProcess = TRUE;
        }
    }
}

/*!
 *******************************************************************************
 * 策略下载进度上报
 *
 * \note 注意是总升级包下载进度信息，傻x才会这样定义进度的上报数据
 *
 * \param   none 
 * \retval  D32
 *******************************************************************************
 */
D32S adm_dpm_policyDownProgressNotify2Uim(void)
{
    D32S i = 0;
    D32S iRet = ADM_ECODE_SUCCESSED;
    ADM_DPS_DOWNPKGPROGRESS_T            *pstDownPkgProgress   = NULL;
    ADM_UIM_SOA_DOWNLOAD_PROGRESS_INFO_T  stPolicyDownProgress = {0};

    do
    {
        pstDownPkgProgress = adm_dps_getEcuDownloadProgress();
        if (ADM_IS_NULL(pstDownPkgProgress))
        {
            iRet = ADM_ECODE_NO_MEMORY;
            log_e("Failed to get all ecu down progress info, err[%d]", iRet);
            break;
        }

        (void)adm_memzero(&stPolicyDownProgress, sizeof(stPolicyDownProgress));
        for (i = 0; i < pstDownPkgProgress->num; i++)
        {
            stPolicyDownProgress.speed      += pstDownPkgProgress->pstProgressList[i].stProgress.speed;
            stPolicyDownProgress.savedSize  += pstDownPkgProgress->pstProgressList[i].stProgress.saveBytes;
            stPolicyDownProgress.totalSize  += pstDownPkgProgress->pstProgressList[i].stProgress.totalBytes;
            stPolicyDownProgress.remainTime += pstDownPkgProgress->pstProgressList[i].stProgress.remainTime;
        }

        stPolicyDownProgress.iTotalProgress = (D32S)((stPolicyDownProgress.savedSize * 100) / stPolicyDownProgress.totalSize);
        
        if (100 == stPolicyDownProgress.iTotalProgress)
        {
            g_bIsReportEnd = DTRUE;
            // 升级成功了，就开始模拟下载进度
            adm_dpm_startDownloadProgressSimulate(FALSE);
        }
        
        // 转为90的进度
        log_i("old iTotalProgress[%d]", stPolicyDownProgress.iTotalProgress);
        stPolicyDownProgress.iTotalProgress = ((stPolicyDownProgress.iTotalProgress)*90)/100;
        g_DownloadProgressSimulate = stPolicyDownProgress.iTotalProgress;
        log_i("new iTotalProgress[%d]", stPolicyDownProgress.iTotalProgress);        

        // TODO: debug
        if (!adm_com_kvstorage_has_key(adm_core_service_getBusinessDatabaseDb(),ADM_DPM_DOWNLOADPROGRESS_KV_KEY))
        {
            //也就是说当前没有模拟进度才会由当前函数上报进度
            adm_uim_policyDownloadProgressNotify(adm_dpm_getDownInfo()->eTriggerType, 
                            "sessionId", 
                            "downloading", 
                            &stPolicyDownProgress);
        }
        //if (ADM_IS_ERR(iRet))
        //{
        //    log_e("Failed to notify down policy progress info, err[%d]", iRet);
        //    break;
        //}
    } while (FALSE);

    adm_dps_delDownPackageProgress(&pstDownPkgProgress);

    log_d("down policy progress notify uim, iRet[%d](0: ok, other: failed)", iRet);

    return iRet;
}

/*!
 *******************************************************************************
 * 下载条件检查结果通知到uim
 *
 * \note 
 *
 * \param[in] pstDownPkgProgress 下载升级包进度信息
 * \retval    D32
 *******************************************************************************
 */
D32S adm_dpm_downCondVerifyResultNotify2Uim
(
    ADM_APPLICATION_COMMON_CMDID_TYPE_E  eCmdidType,
    const DCHAR                               *state,
    ADM_UIM_SOA_CONDITION_DATA_ARRAY_T  *condsArrarList,
    D32S                                 verifyResult
)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    D32S iRespCode = 0;
    ADM_DPS_DOWNTSAKINFO_T stDownTaskInfo = {0};
    DCHAR szEventBuf[ADM_COM_LEN_256] = {0};

    if (ADM_IS_NULL(state) || ADM_IS_NULL(condsArrarList))
    {
        log_e("Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }

    do
    {
        //!< 获取下载任务信息 
        (void)adm_memzero(&stDownTaskInfo, sizeof(stDownTaskInfo));
        iRet = adm_dps_getDownloadTaskInfo(&stDownTaskInfo);
        if (ADM_IS_ERR(iRet))
        {
            log_e( "Failed to get version policy, err[%d]", iRet);
            break;
        }
        
        if(TRUE == adm_vsm_getHmiOnline())
        {
            adm_uim_hmi_conds_downloadResult_RR(eCmdidType, 
                            stDownTaskInfo.sessionId, 
                            state, 
                            condsArrarList, 
                            verifyResult,
                            &iRespCode);
            if (ADM_IS_ERR(iRespCode))
            {
                iRet = iRespCode;
                log_e("Failed to notify uim conditions verify, err[%d]", iRespCode);
                (void)adm_memzero(szEventBuf, sizeof(szEventBuf));
                (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), "Failed to notify uim conditions verify result, RespCode[%d]",  iRespCode);
                adm_dpm_addEventAndReport(ADM_COM_MOD_DPM, TRUE, ADM_EVENT_ID_DI_062, ADM_OCS_EVENT_LEVEL_ERROR, "conditions verify result notify failed!", szEventBuf);
                break;
            }
        }

    } while (FALSE);

    adm_dps_cleanDownTaskInfo(&stDownTaskInfo);
    log_d("down cond verify result notify uim, state[%d](0: ok, other: failed)", iRet);

    return iRet;
}

/*!
 *******************************************************************************
 * 单个ecu下载进度通知到uim
 *
 * \note 
 *
 * \param[in] pstDownPkgProgress ecu下载进度信息
 * \retval    D32
 *******************************************************************************
 */
D32S adm_dpm_ecuDownProgressNotify2Uim
(
    ADM_DPS_DOWNPKGPROGRESSINFO_T *pstDownProgressInfo
)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    ADM_UIM_SOA_ECU_DOWNLOAD_PROGRESS_INFO_T *pstUimEcuDownloadProgress = NULL;

    if (ADM_IS_NULL(pstDownProgressInfo))
    {
        log_e("Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }

    do
    {
        pstUimEcuDownloadProgress = __adm_dpm_createEcuDownProgressByDpm(pstDownProgressInfo);
        if (ADM_IS_NULL(pstUimEcuDownloadProgress))
        {
            iRet = ADM_ECODE_NO_MEMORY;
            log_e("Failed to create ecu down progress info, err[%d]", iRet);
            break;
        }

        // TODO: debug
        iRet = adm_uim_ecuDownloadProgressNotify(adm_dpm_getDownInfo()->eTriggerType, 
                        "sessionId", 
                        "downloading", 
                        pstUimEcuDownloadProgress->pstEcuIdenInfo, 
                        pstUimEcuDownloadProgress->pstEcuDownloadProgress);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to notify uim all ecu down progress, err[%d]", iRet);
            break;
        }
    } while (FALSE);

    __adm_dpm_freeUimEcuDownProgressInfo(pstUimEcuDownloadProgress);

    log_d("down one progress notify uim, state[%d](0: ok, other: failed)", iRet);

    return iRet;
}


/*!
 *******************************************************************************
 * 单个ecu下载结果通知到uim
 *
 * \note 
 *
 * \param[in] pstDownResultInfo ecu下载结果信息
 * \retval    D32
 *******************************************************************************
 */
D32S adm_dpm_ecuDownResultNotify2Uim
(
    ADM_DPS_DOWNPKGRESULTINFO_T *pstDownResultInfo
)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    D32S iRespCode = 0;
    ADM_DPS_DOWNTSAKINFO_T stDownTaskInfo = {0};
    ADM_UIM_SOA_ECU_DOWNLOAD_RESULT_INFO_T *pstUimEcuDownResultInfo = NULL;
    DCHAR szEventBuf[ADM_COM_LEN_256] = {0};

    if (ADM_IS_NULL(pstDownResultInfo))
    {
        log_e("Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }


    do
    {
        //!< 获取下载任务信息 
        (void)adm_memzero(&stDownTaskInfo, sizeof(stDownTaskInfo));
        iRet = adm_dps_getDownloadTaskInfo(&stDownTaskInfo);
        if (ADM_IS_ERR(iRet))
        {
            log_e( "Failed to get version policy, err[%d]", iRet);
            break;
        }
        
        pstUimEcuDownResultInfo = __adm_dpm_createEcuDownResultByDpm(pstDownResultInfo);
        if (ADM_IS_NULL(pstUimEcuDownResultInfo))
        {
            iRet = ADM_ECODE_NO_MEMORY;
            log_e("Failed to create ecu down result info, err[%d]", iRet);
            break;
        }
        
        if(TRUE == adm_vsm_getHmiOnline())
        {
            adm_uim_hmi_ecu_downloadResult_RR(adm_dpm_getDownInfo()->eTriggerType, 
                            stDownTaskInfo.sessionId, 
                            pstUimEcuDownResultInfo->state, 
                            pstUimEcuDownResultInfo->pstEcuIdenInfo, 
                            pstUimEcuDownResultInfo->pstEcuDownloadResult,
                            &iRespCode);
            if (ADM_IS_ERR(iRespCode))
            {
                iRet = iRespCode;
                log_e("Failed to notify uim all ecu down progress, err[%d]", iRespCode);
                (void)adm_memzero(szEventBuf, sizeof(szEventBuf));
                (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), "Failed to notify ecu download result, RespCode[%d]",  iRespCode);
                adm_dpm_addEventAndReport(ADM_COM_MOD_DPM, TRUE, ADM_EVENT_ID_DI_063, ADM_OCS_EVENT_LEVEL_ERROR, "ecu download result notify failed!", szEventBuf);
                break;
            }
        }
    } while (FALSE);

    __adm_dpm_freeUimEcuDownResultInfo(pstUimEcuDownResultInfo);
    adm_dps_cleanDownTaskInfo(&stDownTaskInfo);

    log_d("down progress notify uim, state[%d](0: ok, other: failed)", iRet);

    return iRet;
}


/*!
 *******************************************************************************
 * 所有ecu下载进度通知到uim
 *
 * \note 
 *
 * \param[in] pstDownPkgProgress 下载升级包进度信息
 * \retval    D32
 *******************************************************************************
 */
D32S adm_dpm_allEcuDownProgressNotify2Uim
(
    ADM_DPS_DOWNPKGPROGRESS_T *pstDownPkgProgress
)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    
    ADM_UIM_SOA_ECU_DOWNLOAD_PROGRESS_ARRAY_INFO_T stUimEcuDownProgressList = {0};
    ADM_UIM_SOA_DOWNLOAD_PROGRESS_INFO_T stUimDownloadProgress = {0};

    if (ADM_IS_NULL(pstDownPkgProgress))
    {
        log_e("Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }


    do
    {
        (void)adm_memzero(&stUimEcuDownProgressList, sizeof(stUimEcuDownProgressList));
        iRet = __adm_dpm_fillUimEcuDownProgressList(pstDownPkgProgress, &stUimEcuDownProgressList);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to fill ecu down progress to uim down progress, err[%d]", iRet);
            break;
        }

        // TODO: debug
        iRet = adm_uim_allEcuDownloadProgressNotify(adm_dpm_getDownInfo()->eTriggerType, 
                        "sessionId", 
                        "downloading", 
                        &stUimEcuDownProgressList, 
                        &stUimDownloadProgress);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to notify uim all ecu down progress, err[%d]", iRet);
            break;
        }
    } while (FALSE);

    __adm_dpm_cleanUimEcuDownProgressList(&stUimEcuDownProgressList);

    log_d("down progress notify uim, state[%d](0: ok, other: failed)", iRet);

    return iRet;
}


/*!
 *******************************************************************************
 * 所有ecu下载结果通知到uim
 *
 * \note 
 *
 * \param[in] pstDownPkgResult 下载升级包结果信息
 * \retval    D32
 *******************************************************************************
 */
D32S adm_dpm_allEcuDownResultNotify2Uim
(
    ADM_DPS_DOWNPKGRESULT_T *pstDownPkgResult
)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    D32S iRespCode = 0;
    ADM_DPS_DOWNTSAKINFO_T stDownTaskInfo = {0};
    ADM_UIM_SOA_DOWNLOAD_RESULT_INFO_T stEcuDownloadResult = {0};
    ADM_UIM_SOA_ECU_DOWNLOAD_RESULT_ARRAY_INFO_T stEcuDownloadResultArrayInfo = {0};
    DCHAR szEventBuf[ADM_COM_LEN_256] = {0};

    if (ADM_IS_NULL(pstDownPkgResult))
    {
        log_e("Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }

    do
    {
        //!< 获取下载任务信息 
        (void)adm_memzero(&stDownTaskInfo, sizeof(stDownTaskInfo));
        iRet = adm_dps_getDownloadTaskInfo(&stDownTaskInfo);
        if (ADM_IS_ERR(iRet))
        {
            log_e( "Failed to get version policy, err[%d]", iRet);
            break;
        }
        
        (void)adm_memzero(&stEcuDownloadResultArrayInfo, sizeof(stEcuDownloadResultArrayInfo));
        iRet = __adm_dpm_fillUimEcuDownResultList(pstDownPkgResult, &stEcuDownloadResultArrayInfo);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to fill ecu down reuslt to uim down result, err[%d]", iRet);
            break;
        }

        if(TRUE == adm_vsm_getHmiOnline())
        {
            adm_uim_hmi_allecu_downloadResult_RR(adm_dpm_getDownInfo()->eTriggerType, 
                            stDownTaskInfo.sessionId, 
                            "downloading", 
                            &stEcuDownloadResultArrayInfo, 
                            &stEcuDownloadResult,
                            &iRespCode);
            if (ADM_IS_ERR(iRespCode))
            {
                iRet = iRespCode;
                log_e("Failed to notify uim all ecu down result, err[%d]", iRespCode);
                (void)adm_memzero(szEventBuf, sizeof(szEventBuf));
                (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), "Failed to notify uim all ecu down result, RespCode[%d]",  iRespCode);
                adm_dpm_addEventAndReport(ADM_COM_MOD_DPM, TRUE, ADM_EVENT_ID_DI_064, ADM_OCS_EVENT_LEVEL_ERROR, "all ecu down result notify failed!", szEventBuf);
                break;
            }
        }
    } while (FALSE);

    __adm_dpm_cleanUimEcuDownResultList(&stEcuDownloadResultArrayInfo);
    adm_dps_cleanDownTaskInfo(&stDownTaskInfo);

    log_d("down result notify uim, state[%d](0: ok, other: failed)", iRet);

    return iRet;
}

D32S adm_dpm_downloadTaskStopNotify2Uim()
{
    D32S iRet = ADM_ECODE_FAILED;
    D32S iRespCode = 0;
    DBOOL bIsHandle = DFALSE;

    if (DTRUE == adm_vsm_getHmiOnline())
    {
        bIsHandle = DTRUE;
        adm_uim_hmi_downloadTaskStop_RR(adm_dpm_getDownInfo()->eTriggerType,
                                        "sessionId", &iRespCode);
        if (ADM_IS_ERR(iRespCode))
        {
            iRet = iRespCode;
            log_e("Failed to notify uim download task stop, err[%d]", iRespCode);
        }
        else 
        {
            iRet = ADM_ECODE_SUCCESSED;
        }
    }

    log_d("adm_dpm_downloadTaskStopNotify2Uim state[%d](0: ok, other: failed)", iRet);

    return iRet;
}

#if 0
#endif


/*!
 *******************************************************************************
 * hmi触发的下载前置条件校验线程处理函数
 *
 * \note    
 * 
 * \param[in]  param  线程参数/通知回调函数
 * \retval     void *
 *******************************************************************************
 */
static void *adm_dpm_triggerDownConditionThreadProc(void *param)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    DBOOL bIsConform = TRUE;
    D32S  verifyResult = 0;
    ADM_COND_POLICY_T                  *pstDownCondReq = NULL;
    ADM_UIM_SOA_CONDITION_DATA_ARRAY_T *pstCondResults = NULL;

    log_i("Entry hmi down condition check proc thread.");

    if (ADM_IS_NULL(param))
    {
        log_e("Invalid input param");
        return NULL;
    }

    do
    {
        pstDownCondReq = (ADM_COND_POLICY_T *)param;

        iRet = adm_cdm_verify_Compare(pstDownCondReq, &pstCondResults, &bIsConform);    
        if (ADM_IS_ERR(iRet))
        {
            log_e( "Failed to verify compare download condition, err[%d].", iRet);
        }

        if (0 != (D32S)bIsConform)
        {
            /**下载前前置条件校验失败，需要响应特殊错误码，回到download ready状态*/
            verifyResult = ADM_ECODE_BEFORE_DOWN_COND_VERIFY_FAILED;
        }

        //!< 通知 uim 下载前置条件校验结果
        iRet = adm_dpm_downCondVerifyResultNotify2Uim(ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_USER, 
                        "DownloadPreConditionVerify", 
                        pstCondResults,
                        verifyResult);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to notify down condition verify result! err[%d]", iRet);
        }

        (void)adm_cdm_verify_free(pstCondResults);
    }while (FALSE);

    adm_condPolicyDeInit(pstDownCondReq);
    adm_free(pstDownCondReq);
    
    log_i("Return hmi down condition check proc thread.");

    return NULL;
}



#if 0
#endif

/*!
 *******************************************************************************
 * uim触发下载动作.
 *
 * \note
 * 
 * \param[out]  pstRptParam 模块配置参数
 * \retval      D32S
 *******************************************************************************
 */
void adm_dpm_uimTriggerDownload
(
    ADM_UIM_SOA_ACTION_AUTH_TYPE_E* eActionType, 
    void* priv
)
{
    DBOOL bIsTrue = FALSE;
    D32S iRet = ADM_ECODE_SUCCESSED;
    ADM_CVS_UPGRADE_MODE_E eUpgradeMode = ADM_CVS_UPGRADE_MODE_INVALID;
    ADM_APPLICATION_COMMON_CMDID_TYPE_E eTriggerType = ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_USER;

    if (ADM_IS_NULL(eActionType))
    {
        log_e("Invalid input param");
        return;
    }

    log_i("===> dpm recv trigger download msg.");

    iRet = adm_cvs_getUpgradeMode((D32S *)&eUpgradeMode);
    if (ADM_IS_SUCCESS(iRet))
    {
        /** 救援和usb需要重新赋值，其他升级模式要么自动触发下载，要么与用户触发方式无异 */
        if (ADM_CVS_UPGRADE_MODE_USB_CHECK == eUpgradeMode || 
            ADM_CVS_UPGRADE_MODE_RESCUE == eUpgradeMode)
        {
            eTriggerType = (ADM_APPLICATION_COMMON_CMDID_TYPE_E)eUpgradeMode;
        }
    }

    //!< 设置状态机，状态机会触发下载
    log_i("download trigger type[%d]", eTriggerType);
    bIsTrue = adm_vsm_triggerDownload(eTriggerType);
    if (TRUE == bIsTrue)
    {
        *eActionType = ADM_UIM_SOA_ACTION_AUTH_TYPE_ALLOW;
    }
    else
    {
        *eActionType = ADM_UIM_SOA_ACTION_AUTH_TYPE_NOT_ALLOW;
    }

    return;
}

/*!
 *******************************************************************************
 * uim触发取消下载动作.
 *
 * \note
 * 
 * \param[out]  pstRptParam 模块配置参数
 * \retval      D32S
 *******************************************************************************
 */
void adm_dpm_uimCancelDownload
(
    const ADM_APPLICATION_COMMON_CMDID_TYPE_E  eCmdidType, 
    ADM_UIM_SOA_ACTION_AUTH_TYPE_E *eActionType, 
    D32S                           *errorCode, 
    void                           *priv
)
{

    if (ADM_IS_NULL(eActionType) || ADM_IS_NULL(errorCode))
    {
        log_e("Invalid input param");
        return;
    }

    log_i("===> dpm recv cancel download msg, but ignore.");

    /** U盘升级模式下，用户触发取消下载，设置状态为空闲 */
    adm_vsm_stateSet(ADM_VSM_STATE_IDLE);

    *eActionType = ADM_UIM_SOA_ACTION_AUTH_TYPE_ALLOW;
    *errorCode = ADM_ECODE_SUCCESSED;

    return;
}

/*!
 *******************************************************************************
 * uim触发暂停下载动作.
 *
 * \note
 * 
 * \param[out]  pstRptParam 模块配置参数
 * \retval      none
 *******************************************************************************
 */
void adm_dpm_uimPauseDownload
(
    const ADM_APPLICATION_COMMON_CMDID_TYPE_E  eCmdidType, 
    ADM_UIM_SOA_ACTION_AUTH_TYPE_E *eActionType, 
    D32S                           *errorCode, 
    void                           *priv
)
{

    if (ADM_IS_NULL(eActionType) || ADM_IS_NULL(errorCode))
    {
        log_e("Invalid input param");
        return;
    }

    log_i("===> dpm recv pause download msg, but ignore.");

    *eActionType = ADM_UIM_SOA_ACTION_AUTH_TYPE_ALLOW;
    *errorCode = ADM_ECODE_SUCCESSED;

    return;
}


/*!
 *******************************************************************************
 * uim触发下载条件检查.
 *
 * \note hmi触发下载前置条件校验
 * 
 * \param[out]  pstRptParam 模块配置参数
 * \retval      D32S
 *******************************************************************************
 */
void adm_dpm_uimDownloadConditionsVerified
(
    const ADM_UIM_SOA_CONDITIONS_VERIFIED_TYPE_E eVerifiedType, 
    const D16U  cycleTime, 
    const D16U  timeout, 
    D32S       *errorCode
)
{
    D32S i = 0;
    D32S iRet = ADM_ECODE_SUCCESSED;
    ADM_DPS_DOWNCONDITION_T stDownCond = {0};    
    ADM_COND_POLICY_T *pstDownCondReq = NULL;
    pthread_t stThreadHandle = {0};
    ADM_UIM_SOA_CONDITION_DATA_ARRAY_T stCondResults = {0};

    if (ADM_IS_NULL(errorCode))
    {
        log_e("Invalid input param");
        return;
    }
    log_i("===> dpm recv trigger down cond verified msg.");

    do
    {
        iRet = adm_dpm_checkAndUpdateDownInfo();
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to check and update DPS down info, err[%d]", iRet);
            break;            
        }
    
        //!< 获取下载条件
        iRet = adm_dps_getDownloadCondition(&stDownCond);
        if (ADM_IS_ERR(iRet))
        {
            log_e( "Failed to get downlaod condition, err[%d].", iRet);
            break;
        }

        //!< 平台未配置相关下载前置条件，直接返回
        if (0 == stDownCond.trigNum)
        {
            iRet = ADM_ECODE_DOWN_NOSET_DOWNCONDITION;
            log_i("OTA Service no set download condition.");
            break;
        }

        //!< 找到对应模式的的前置条件
        for (i = 0; i < stDownCond.trigNum; i++)
        {
            // TODO: 1: 车机触发的前置条件校验， 后续与其他模块统一改为宏
            if (ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_USER == (ADM_APPLICATION_COMMON_CMDID_TYPE_E)stDownCond.pstTrigDownCondList[i].iType)
            {
                break;
            }
        }

        //!< 未找到对应模式的的前置条件
        if (i == stDownCond.trigNum)
        {
            iRet = ADM_ECODE_DOWN_NOFIND_CONDITION;
            break;
        }

        //!< 拷贝对应的前置条件，添加到线程中处理
        pstDownCondReq = (ADM_COND_POLICY_T *)adm_malloc(sizeof(*pstDownCondReq));
        if (ADM_IS_NULL(pstDownCondReq))
        {
            log_e("Failed to malloc down condition info, err[%d]", ADM_ECODE_NO_MEMORY);
            break;
        }
        adm_condPolicyInit(pstDownCondReq);
        adm_condPolicyClone(&stDownCond.pstTrigDownCondList[i], pstDownCondReq);

        //!< 线程中进行前置条件校验
        iRet = adm_dpm_pthreadCreate(&stThreadHandle, adm_dpm_triggerDownConditionThreadProc, pstDownCondReq);
        if (ADM_IS_ERR(iRet))
        {
            adm_condPolicyDeInit(pstDownCondReq);
            adm_free(pstDownCondReq);
            log_e("Failed to create check down condition proc thread!, err[%d]", iRet);
            break;
        }
    } while(FALSE);
    adm_dps_cleanEcuDownCondition(&stDownCond);

    //!< 若平台未配置下载条件，则直接返回成功
    *errorCode = ((ADM_ECODE_DOWN_NOSET_DOWNCONDITION == iRet) ? (ADM_ECODE_SUCCESSED) : (iRet));

    //!< 若启动下载失败/未配置下载前置条件，则直接通知uim下载前置条件结果
    if (ADM_IS_ERR(iRet))
    {
        *errorCode = ADM_ECODE_BEFORE_DOWN_COND_VERIFY_FAILED;
        (void)adm_memzero(&stCondResults, sizeof(stCondResults));
        stCondResults.condsArrayLength = 0;
        stCondResults.condsArray = NULL;
        //!< 通知 uim 下载前置条件校验结果
        iRet = adm_dpm_downCondVerifyResultNotify2Uim(ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_USER, 
                        "DownloadPreConditionVerify", 
                        &stCondResults,
                        *errorCode);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to notify down condition verify result! err[%d]", iRet);
        }
    }

    return;
}



/*!
 *******************************************************************************
 * uim查询所有ecu下载进度信息.
 *
 * \note
 * 
 * \param[out]  pstRptParam 模块配置参数
 * \retval      D32S
 *******************************************************************************
 */
D32S adm_dpm_uimGetAllEcuDownloadProgress
(
    ADM_APPLICATION_COMMON_CMDID_TYPE_E *eCmdidType, 
    DCHAR **sessionId, 
    DCHAR **state, 
    ADM_UIM_SOA_ECU_DOWNLOAD_PROGRESS_ARRAY_INFO_T **pstEcuDownloadProgressArrayInfo, 
    ADM_UIM_SOA_DOWNLOAD_PROGRESS_INFO_T **pstPolicyDownloadProgress, 
    D32S *errorCode,
    void *priv
)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    ADM_CVS_VERSION_POLICY_T *pstCVSVersionPolicy = NULL;
    ADM_DPS_DOWNPKGPROGRESS_T *pstDownPkgProgress = NULL;
    ADM_UIM_SOA_ECU_DOWNLOAD_PROGRESS_ARRAY_INFO_T *pstUimEcuDownProgressArrayInfo = NULL;

    if (ADM_IS_NULL(eCmdidType) || ADM_IS_NULL(sessionId) || ADM_IS_NULL(state) || 
        ADM_IS_NULL(pstEcuDownloadProgressArrayInfo) || ADM_IS_NULL(pstPolicyDownloadProgress) || 
        ADM_IS_NULL(errorCode))
    {
        log_e("Invalid input param");
        if (NULL != errorCode)
        {
            *errorCode = ADM_ECODE_INVALID_PARAM;
        }
        return ADM_ECODE_INVALID_PARAM;
    }

    do 
    {
        //!< 获取检测信息
        pstCVSVersionPolicy = adm_cvs_newPolicyBody();
        if (ADM_IS_NULL(pstCVSVersionPolicy))
        {
            iRet = ADM_ECODE_NO_MEMORY;
            log_e( "Failed to new policy body");
            break;
        }

        log_e("=======> callBack getVersionPolicy");
        iRet = adm_cvs_getVersionPolicy(pstCVSVersionPolicy);
        if (ADM_IS_ERR(iRet))
        {
            log_e( "Failed to get version policy, err[%d]", iRet);
            break;
        }

        pstDownPkgProgress = adm_dps_getEcuDownloadProgress();
        if (ADM_IS_NULL(pstDownPkgProgress))
        {
            iRet = ADM_ECODE_FAILED;
            log_e("Failed to get ecu download progress list, err[%d]", iRet);
            break;
        }

        pstUimEcuDownProgressArrayInfo = (ADM_UIM_SOA_ECU_DOWNLOAD_PROGRESS_ARRAY_INFO_T *)adm_malloc(sizeof(*pstUimEcuDownProgressArrayInfo));
        if (ADM_IS_NULL(pstUimEcuDownProgressArrayInfo))
        {
            iRet = ADM_ECODE_NO_MEMORY;
            log_e("Failed to malloc soa ecu down progress array info, err[%dd]", iRet);
            break;
        }
        (void)adm_memzero(pstUimEcuDownProgressArrayInfo, sizeof(*pstUimEcuDownProgressArrayInfo));

        iRet = __adm_dpm_fillUimEcuDownProgressList(pstDownPkgProgress, pstUimEcuDownProgressArrayInfo);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to fill ecu down progress to uim down progress, err[%d]", iRet);
            break;
        }

        // TODO: 待补充
        *pstPolicyDownloadProgress = NULL;
        *pstEcuDownloadProgressArrayInfo = pstUimEcuDownProgressArrayInfo;
        *eCmdidType = ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_USER;
        *sessionId = adm_strdup(pstCVSVersionPolicy->sessionId);
        *state = adm_strdup("downloading");
        //*priv = NULL;


        // TODO: debug
        __adm_dpm_debugUimAllEcuDownloadProgress(pstUimEcuDownProgressArrayInfo);
        
    } while (FALSE);

    *errorCode = iRet;
    if (NULL != pstCVSVersionPolicy)
    {
        adm_cvs_deletePolicyBody(pstCVSVersionPolicy);
    }
    adm_dps_delDownPackageProgress(&pstDownPkgProgress);

    if (ADM_IS_ERR(iRet))
    {
        *pstEcuDownloadProgressArrayInfo = NULL;
        adm_dpm_delUimEcuDownProgressList(&pstUimEcuDownProgressArrayInfo);
        
        // TODO: 相关内存释放
    }

    return iRet;
}

/*!
 *******************************************************************************
 * uim查询所有ecu下载结果信息.
 *
 * \note
 * 
 * \param[out]  pstRptParam 模块配置参数
 * \retval      D32S
 *******************************************************************************
 */
D32S adm_dpm_uimGetAllEcuDownloadResult
(
    ADM_APPLICATION_COMMON_CMDID_TYPE_E *eCmdidType, 
    DCHAR **sessionId, 
    DCHAR **state, 
    ADM_UIM_SOA_ECU_DOWNLOAD_RESULT_ARRAY_INFO_T **pstEcuDownloadResultArrayInfo, 
    ADM_UIM_SOA_DOWNLOAD_RESULT_INFO_T **pstEcuDownloadResult, 
    D32S *errorCode, 
    void *priv
)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    ADM_CVS_VERSION_POLICY_T *pstCVSVersionPolicy = NULL;
    ADM_DPS_DOWNPKGRESULT_T     *pstDownPkgResult = NULL;
    ADM_UIM_SOA_ECU_DOWNLOAD_RESULT_ARRAY_INFO_T *pstUimEcuDownResultArrayInfo = NULL;

    if (ADM_IS_NULL(eCmdidType) || ADM_IS_NULL(sessionId) || ADM_IS_NULL(state) || 
        ADM_IS_NULL(pstEcuDownloadResultArrayInfo) || ADM_IS_NULL(pstEcuDownloadResult) ||
        ADM_IS_NULL(errorCode))
    {
        log_e("Invalid input param");
        if (NULL != errorCode)
        {
            *errorCode = ADM_ECODE_INVALID_PARAM;
        }
        return ADM_ECODE_INVALID_PARAM;
    }

    do
    {
        //!< 获取检测信息
        pstCVSVersionPolicy = adm_cvs_newPolicyBody();
        if (ADM_IS_NULL(pstCVSVersionPolicy))
        {
            iRet = ADM_ECODE_NO_MEMORY;
            log_e( "Failed to new policy body");
            break;
        }

        log_e("=======> callBack getVersionPolicy");
        iRet = adm_cvs_getVersionPolicy(pstCVSVersionPolicy);
        if (ADM_IS_ERR(iRet))
        {
            log_e( "Failed to get version policy, err[%d]", iRet);
            break;
        }

        pstDownPkgResult = adm_dps_getEcuDownloadResult();
        if (ADM_IS_NULL(pstDownPkgResult))
        {
            iRet = ADM_ECODE_FAILED;
            log_e("Failed to get ecu download result list, err[%d]", iRet);
            break;
        }

        pstUimEcuDownResultArrayInfo = (ADM_UIM_SOA_ECU_DOWNLOAD_RESULT_ARRAY_INFO_T *)adm_malloc(sizeof(*pstUimEcuDownResultArrayInfo));
        if (ADM_IS_NULL(pstUimEcuDownResultArrayInfo))
        {
            iRet = ADM_ECODE_NO_MEMORY;
            log_e("Failed to malloc soa ecu down progress array info, err[%dd]", iRet);
            break;
        }
        (void)adm_memzero(pstUimEcuDownResultArrayInfo, sizeof(*pstUimEcuDownResultArrayInfo));

        iRet = __adm_dpm_fillUimEcuDownResultList(pstDownPkgResult, pstUimEcuDownResultArrayInfo);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to fill ecu down reuslt to uim down result, err[%d]", iRet);
            break;
        }

        // TODO: 待补充
        *pstEcuDownloadResult = NULL;
        *pstEcuDownloadResultArrayInfo = pstUimEcuDownResultArrayInfo;
        *eCmdidType = ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_USER;
        *sessionId = adm_strdup(pstCVSVersionPolicy->sessionId);
        *state = adm_strdup("result");
        //*priv = NULL;

        // TODO: debug
        __adm_dpm_debugUimAllEcuDownloadResult(pstUimEcuDownResultArrayInfo);
    } while (FALSE);

    *errorCode = iRet;
    if (NULL != pstCVSVersionPolicy)
    {
        adm_cvs_deletePolicyBody(pstCVSVersionPolicy);
    }
    adm_dps_delDownPackageResult(&pstDownPkgResult);

    if (ADM_IS_ERR(iRet))
    {
        *pstEcuDownloadResultArrayInfo = NULL;
        adm_dpm_delUimEcuDownResultList(&pstUimEcuDownResultArrayInfo);
        
        // TODO: 相关内存释放
    }

    return iRet;
}


/*!
 *******************************************************************************
 * hmi 设置自动下载开关配置.
 *
 * \note
 * 
 * \param[in]   eSwitchType 自动下载开关
 * \param[out]  errorCode   设置自动下载响应结果
 * \retval      none
 *******************************************************************************
 */
void adm_dpm_uimSetAutoDownloadSwitch
(
    ADM_UIM_SOA_AUTO_DOWNLOAD_TYPE_E eSwitchType, 
    D32S *errorCode
)
{
    if ((ADM_UIM_SOA_AUTO_DOWNLOAD_INVALID <= eSwitchType) || ADM_IS_NULL(errorCode))
    {
        log_e("Invalid input param");
        if (NULL != errorCode)
        {
            *errorCode = ADM_ECODE_INVALID_PARAM;
        }
        return;
    }

    adm_vsm_hmiIsAutoDownload((DBOOL)eSwitchType, errorCode);

    return;
}


