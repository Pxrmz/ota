/**
 * @file    adm_tpm_transfer.c
 * @brief   Transfer Package 
 * @details 升级包分包
 * @author  wangyanpeng
 * @version v0.1.0
 * @date    2020-11-06
 * @copyright www.abupdate.com
 */
#include "stdio.h"
#include "unistd.h"
#include "stdlib.h"
#include <pthread.h>

#include "adm_dpm_reportCloud.h"
#include "adm_tpm_log.h"
#include "adm_tpm_transfer.h"
#include "adm_tps_transferService.h"
#include "adm_tpm_transferManagement.h"
#include "adm_cfg_api.h"
#include "adm_oca_file.h"
#include "adm_application_errorCode.h"
#include "adm_soa.h"
#include "adm_dim_common.h"
#include "adm_pub_func.h"

//!< tpm分包进度/结果消息通知时间间隔，单位ms
#define ADM_TPM_MSG_NOTIFY_TIME_WIDE    2000 

#define ADM_TPM_MSG_REQTRANSDOWN_TIMEOUT    30

//!< 收集日志超时时间
#define ADM_TPM_COLLEVT_LOG_ASYNC_WAIT_TIME 600000

/*!
 * @brief 异步条件变量信息
 */
typedef struct stTransAsyncInfo
{
    pthread_cond_t        stCond;   //!< 信号量
    pthread_mutex_t       stMutex;  //!< 信号量锁
    ADM_TPS_TRANSRESULT_T stResult; //!< 异步结果信息

    DBOOL                 bHasResult;  //!< 是否有结果
    DBOOL                 bIsFlush;    //!< 是否有进度刷新
    
} ADM_TPM_TRANS_ASYNCINFO_S;


#if 0
#endif


/*!
 *******************************************************************************
 * 更新分包状态信息
 *
 * \note 
 * 
 * \param[in]   iResult    分包结果 0: success, other: failed
 * \param[in]   isRollback 本次分的是否是回滚包(是否是源包)
 * \retval      none
 *******************************************************************************
 */
static ADM_TPS_TRANSPKGSTATE_E adm_tpm_updateTransStateInfo
(
    D32S    iResult,
    DBOOL   isRollback
)
{
    ADM_TPS_TRANSPKGSTATE_E eTransState = ADM_TPS_TRANSPKG_STATE_FAILED;

    if (ADM_IS_ERR(iResult))
    {
        //!< 源包下载失败等同于 ADM_TPS_TRANSPKG_STATE_FAILED 
        eTransState = 
            ((TRUE == isRollback) ? (ADM_TPS_TRANSPKG_STATE_FAILED) : (ADM_TPS_TRANSPKG_STATE_DSTFAILED));
    }
    else
    {
        eTransState = 
            ((TRUE == isRollback) ? (ADM_TPS_TRANSPKG_STATE_SRCFINISH) : (ADM_TPS_TRANSPKG_STATE_FINISH));
    }

    return eTransState;
}

/*!
*******************************************************************************
* 设置传输异步结构中的进度值
*
* \note 
* 
* \param[in]   pstTransAsyncInfo    传输异步结构信息
* \param[in]   bHasFlush            是否有进度
* \retval      none
*******************************************************************************
*/
static void tpmSetTransFlush(ADM_TPM_TRANS_ASYNCINFO_S *pstTransAsyncInfo, DBOOL bHasFlush)
{
    (void)pthread_mutex_lock(&pstTransAsyncInfo->stMutex);
    pstTransAsyncInfo->bIsFlush = bHasFlush;
    if (TRUE == bHasFlush)
    {
        (void)pthread_cond_signal(&pstTransAsyncInfo->stCond);
    }
    (void)pthread_mutex_unlock(&pstTransAsyncInfo->stMutex);

}

/*!
*******************************************************************************
* 设置传输异步结构中的结果值
*
* \note 
* 
* \param[in]   pstTransAsyncInfo    传输异步结构信息
* \param[in]   pstTransResultInfo   传输结果信息
* \param[in]   bIsRollback          是否是回滚包
* \retval      none
*******************************************************************************
*/
static void tpmSetTransResult(ADM_TPM_TRANS_ASYNCINFO_S *pstTransAsyncInfo,
                                      const ADM_DIM_SOA_TRANS_RESULT_INFO_T *pstTransResultInfo,
                                      DBOOL bIsRollback)
{
    (void)pthread_mutex_lock(&pstTransAsyncInfo->stMutex);
    pstTransAsyncInfo->bHasResult = TRUE;
    pstTransAsyncInfo->stResult.result = pstTransResultInfo->result;
    pstTransAsyncInfo->stResult.failReason  =  ((NULL != pstTransResultInfo->realVal) ? (adm_strdup(pstTransResultInfo->realVal)) : (NULL));
    pstTransAsyncInfo->stResult.eTransState = adm_tpm_updateTransStateInfo(pstTransResultInfo->result, bIsRollback);
    (void)pthread_cond_signal(&pstTransAsyncInfo->stCond);
    (void)pthread_mutex_unlock(&pstTransAsyncInfo->stMutex);

}

/*!
 *******************************************************************************
 * 创建分包异步信息
 *
 * \note 对应销毁函数 __adm_tpm_destroyTransAsyncInfo
 * 
 * \param   none
 * \retval  ADM_TPM_TRANS_ASYNCINFO_S
 *******************************************************************************
 */
static ADM_TPM_TRANS_ASYNCINFO_S* __adm_tpm_createTransAsyncInfo(void)
{
    ADM_TPM_TRANS_ASYNCINFO_S *pstTransAsyncInfo = NULL;

    pstTransAsyncInfo = (ADM_TPM_TRANS_ASYNCINFO_S *)adm_malloc(sizeof(*pstTransAsyncInfo));
    if (ADM_IS_NULL(pstTransAsyncInfo))
    {
        log_e("Failed to malloc transfer async info");
        return NULL;
    }
    (void)adm_memzero(pstTransAsyncInfo, sizeof(*pstTransAsyncInfo));

    pthread_condattr_t cattr;
    pthread_condattr_init(&cattr);
    pthread_condattr_setclock(&cattr, CLOCK_MONOTONIC);

    pstTransAsyncInfo->bHasResult = FALSE;
    pstTransAsyncInfo->bIsFlush = FALSE;
    pstTransAsyncInfo->stResult.result = ADM_ECODE_SUCCESSED;
    (void)pthread_mutex_init(&pstTransAsyncInfo->stMutex, NULL);
    (void)pthread_cond_init(&pstTransAsyncInfo->stCond, &cattr);

    return pstTransAsyncInfo;
}

/*!
 *******************************************************************************
 * 销毁分包异步信息
 *
 * \note 对应创建函数 __adm_tpm_createTransAsyncInfo
 * 
 * \param[in]  pstTransAsyncInfo   分包异步信息
 * \retval     none
 *******************************************************************************
 */
static void __adm_tpm_destroyTransAsyncInfo(ADM_TPM_TRANS_ASYNCINFO_S *pstTransAsyncInfo)
{
    if (ADM_IS_NULL(pstTransAsyncInfo))
    {
        return;
    }

    adm_free(pstTransAsyncInfo->stResult.failReason);
    (void)pthread_mutex_destroy(&pstTransAsyncInfo->stMutex);
    (void)pthread_cond_destroy(&pstTransAsyncInfo->stCond);
    adm_free(pstTransAsyncInfo);

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
 * \param[in]  pIsSignal 信号是否被主动打断
 * \param[in]  pIsFlush  是否刷新等待时间
 * \param[in]  ulAbsTime 等待信号量超时时间(单位毫秒)
 * \retval     D32S
 *******************************************************************************
 */
static D32S __adm_tpm_threadCondWait
(
    pthread_cond_t  *pstCond,
    pthread_mutex_t *pstMutex,
    DBOOL *pIsSignal,
    DBOOL *pIsFlush,
    D32S  ulAbsTime
)
{
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
 * tpm分包通知时间宽度判断
 *
 * \note 获取时间使用的是 CLOCK_MONOTONIC 不随系统时间改变而变化。
 *          入参传入上次的时间，可以保证该函数可重入
 * 
 * \param[in]   pstTs   上次通知的时间信息
 * \retval      DBOOL   TRUE: 超过通知时间阈值，FALSE：未超过通知时间阈值
 *******************************************************************************
 */
static DBOOL __adm_tpm_isOverNotifyTimeWide(struct timespec *pstTs)
{
    DBOOL bRet = FALSE;
    struct timespec tsNow;

    if (ADM_IS_NULL(pstTs))
    {
        log_e("Invalid input param");
        return bRet;
    }

    (void)clock_gettime(CLOCK_MONOTONIC, &tsNow);

    if(((tsNow.tv_sec - pstTs->tv_sec) * 1000 +
        (tsNow.tv_nsec - pstTs->tv_nsec) / (1000 * 1000)) > 
              ADM_TPM_MSG_NOTIFY_TIME_WIDE)
    {
        bRet = TRUE;
        pstTs->tv_sec = tsNow.tv_sec;
        pstTs->tv_nsec = tsNow.tv_nsec;
    }

    return bRet;
}


/*!
 *******************************************************************************
 * 更新分包结果信息
 *
 * \note 更新上报信息和engine层分包结果
 * 
 * \param[in]   fileID   分包文件标识id
 * \param[in]   iResult  分包结果 0: success, other: failed
 * \retval      none
 *******************************************************************************
 */
static void __adm_tpm_updateTransResultInfo
(
    DCHAR  *fileID,
    DBOOL   isRollback,
    D32S    iResult
)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    ADM_TPS_TRANSRESULT_T stResult = {0};

    if (ADM_IS_NULL(fileID))
    {
        log_e("Invalid input param");
        return;
    }

    (void)adm_memzero(&stResult, sizeof(stResult));
    stResult.result      = iResult;
    stResult.failReason  = adm_dpm_errorCode2String(iResult);
    stResult.eTransState = adm_tpm_updateTransStateInfo(iResult, isRollback);
    
    iRet = adm_tps_updateTransPackageResult(fileID, &stResult, NULL);
    adm_free(stResult.failReason);
    if (ADM_IS_ERR(iRet))
    {
        log_e("Failed to update transfer package result, fileID[%s], err[%s]", 
                        fileID, iRet);
        return;
    }

    return;
}

/*!
*******************************************************************************
* master传输进度通知性能统计
*
* \note 
* 
* \param[in]   pstTransProgressInfo   传输参数
* \param[in]   pstTpmTransReqParam    传输请求参数
* \retval      none
*******************************************************************************
*/
static void __adm_tpm_MastertransStatisticsHandle(const ADM_DIM_SOA_TRANS_PROGRESS_INFO_T *pstTransProgressInfo, ADM_TPM_TRANSFER_REQPARAM_T *pstTpmTransReqParam)
{
    DCHAR szEventBuf[ADM_COM_LEN_256] = {0};

    if (ADM_IS_NULL(pstTransProgressInfo) || ADM_IS_NULL(pstTpmTransReqParam))
    {
        log_e("Invalid input param");
        return;
    }

    //!< 性能统计,进度通知
    (void)adm_memzero(szEventBuf, sizeof(szEventBuf));
    (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), 
            "STATSPACK transSpend:ecuReqid[%s], ecuSwDid[%s], fileName[%s], speed[%d], progress[%d], remainTime[%lld], DownSize[%lld - %lld]",
            pstTpmTransReqParam->ecuReqid,
            pstTpmTransReqParam->ecuSwDid,
            pstTpmTransReqParam->fileName,
            pstTransProgressInfo->speed,
            (D32S)((pstTransProgressInfo->totalSize != 0) ? ((pstTransProgressInfo->savedSize * 100) / pstTransProgressInfo->totalSize) : 0),
            pstTransProgressInfo->remainTime,
            pstTransProgressInfo->savedSize,
            pstTransProgressInfo->totalSize);
    adm_dpm_addEventAndReport(ADM_COM_MOD_DPM, FALSE, ADM_EVENT_ID_DL_012, ADM_OCS_EVENT_LEVEL_INFO, szEventBuf, NULL);

    return;
}

#if 0
#endif


/*!
 *******************************************************************************
 * 异步转同步超时判断.
 *
 * \note 
 *
 * \param[in]   pstTransAsyncInfo 异步条件变量信息
 * \retval      none
 *******************************************************************************
 */
static D32S adm_tpm_waitAsyncResult(ADM_TPM_TRANS_ASYNCINFO_S *pstTransAsyncInfo)
{
    D32S iRet = ADM_ECODE_SUCCESSED;

    //!<  异步响应提前到达
    if (FALSE != pstTransAsyncInfo->bHasResult)
    {
        return iRet;
    }

#if 0
    do
    {
        //!< 1、异步请求转同步等待 --- 等待超时时间 10s

        (void)__adm_tpm_threadCondWait(&pstTransAsyncInfo->stCond, &pstTransAsyncInfo->stMutex,
                                       &pstTransAsyncInfo->bHasResult, ADM_COM_ASYNC_WAIT_TIME * 4);

        //!< 2、等待10s后，有结果，不再等待
        if (TRUE == pstTransAsyncInfo->bHasResult)
        {
            log_i("adm_tpm_waitAsyncResult has result.");
            break ;
        }
        //!< 3、等待10s后，没有结果，但是有进度，需要继续等待
        else if (TRUE == pstTransAsyncInfo->bIsFlush)
        {
            tpmSetTransFlush(pstTransAsyncInfo, FALSE);
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

    iRet = __adm_tpm_threadCondWait(&pstTransAsyncInfo->stCond, &pstTransAsyncInfo->stMutex,
                                    &pstTransAsyncInfo->bHasResult, &pstTransAsyncInfo->bIsFlush, ADM_COM_ASYNC_WAIT_TIME * 4);
    if (ADM_IS_ERR(iRet))
    {
        iRet = ADM_ECODE_MSG_WAIT_TIMEOUT;
    }

    return iRet;
}


/*!
 *******************************************************************************
 * 异步转同步超时判断.
 *
 * \note 
 *
 * \param[in]   pstTransAsyncInfo 异步条件变量信息
 * \retval      none
 *******************************************************************************
 */
static D32S adm_tpm_collectLogWaitAsyncResult(ADM_TPM_TRANS_ASYNCINFO_S *pstTransAsyncInfo)
{
    D32S iRet = ADM_ECODE_SUCCESSED;

    //!<  异步响应提前到达
    if (FALSE != pstTransAsyncInfo->bHasResult)
    {
        return iRet;
    }

#if 0
    do
    {
        //!< 1、异步请求转同步等待 --- 等待超时时间 10s
        (void)__adm_tpm_threadCondWait(&pstTransAsyncInfo->stCond, &pstTransAsyncInfo->stMutex,
                                       &pstTransAsyncInfo->bHasResult, ADM_COM_ASYNC_WAIT_TIME * 4);

        //!< 2、等待10s后，有结果，不再等待
        if (TRUE == pstTransAsyncInfo->bHasResult)
        {
            log_i("adm_tpm_waitAsyncResult has result.");
            break ;
        }
        //!< 3、等待10s后，没有结果，但是有进度，需要继续等待
        else if (TRUE == pstTransAsyncInfo->bIsFlush)
        {
            tpmSetTransFlush(pstTransAsyncInfo, FALSE);
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

    iRet = __adm_tpm_threadCondWait(&pstTransAsyncInfo->stCond, &pstTransAsyncInfo->stMutex,
                                    &pstTransAsyncInfo->bHasResult, &pstTransAsyncInfo->bIsFlush, ADM_TPM_COLLEVT_LOG_ASYNC_WAIT_TIME);
    if (ADM_IS_ERR(iRet))
    {
        iRet = ADM_ECODE_MSG_WAIT_TIMEOUT;
    }

    return iRet;
}


/*!
 *******************************************************************************
 * 清空tpm上传日志参数结构信息.
 *
 * \note 不释放最外层 pstTpmUploadReqParam 的内存
 *
 * \param[in]   pstTpmUploadReqParam tpm上传请求参数结构信息
 * \retval      none
 *******************************************************************************
 */
void adm_tpm_cleanUploadReqParamInfo
(
    ADM_TPM_UPLOAD_REQPARAM_T *pstTpmUploadReqParam
)
{
    if (ADM_IS_NULL(pstTpmUploadReqParam))
    {
        return;
    }

    adm_free(pstTpmUploadReqParam->url);
    adm_free(pstTpmUploadReqParam->hash);
    adm_free(pstTpmUploadReqParam->fileName);
    adm_tps_freeTransFileID(&pstTpmUploadReqParam->fileID);

    pstTpmUploadReqParam->fileSize   = 0;
    pstTpmUploadReqParam->isRollback = FALSE;
    pstTpmUploadReqParam->pAsyncInfo = NULL;

    return;
}

/*!
 *******************************************************************************
 * ucl日志上传到主控结果通知回调函数
 *
 * \note
 * 
 * \param[in]   fileID             上传文件标识id
 * \param[in]   pstTransResult     上传结果信息
 * \retval      D32S
 *******************************************************************************
 */
D32S adm_tpm_dimUploadToHostResultHandle
(
    const DCHAR *fileID, 
    const DCHAR *filename,
    const ADM_DIM_SOA_TRANS_RESULT_INFO_T *pstTransResult,
    void *priv
)
{
    ADM_TPM_TRANS_ASYNCINFO_S   *pstTransAsyncInfo = NULL;
    ADM_TPM_UPLOAD_REQPARAM_T   *pstTpmUploadReqParam = NULL;

    if (ADM_IS_NULL(fileID) || ADM_IS_NULL(pstTransResult) || ADM_IS_NULL(filename))
    {
        log_e("Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }
    log_i("Recv upload result notify, fileID[%s], result[%d].", fileID, pstTransResult->result);

    //!< 无私有属性数据则直接退出
    if (ADM_IS_NULL(adm_tpm_getTransferInfo()->stUploadReqParam.fileID))
    {
        log_e("Invalid input param, priv is null");
        return ADM_ECODE_INVALID_PARAM;
    }

    pstTpmUploadReqParam = (ADM_TPM_UPLOAD_REQPARAM_T *)&adm_tpm_getTransferInfo()->stUploadReqParam;
    if (0 != adm_strcmp(fileID, pstTpmUploadReqParam->fileID))
    {
        log_e("Invalid transfer fileID");
        return ADM_ECODE_INVALID_PARAM;
    }

    // 异步转同步 --- launch async cond
    if (NULL != pstTpmUploadReqParam->pAsyncInfo)
    {
        pstTransAsyncInfo = (ADM_TPM_TRANS_ASYNCINFO_S *)pstTpmUploadReqParam->pAsyncInfo;        
        tpmSetTransResult(pstTransAsyncInfo, pstTransResult, pstTpmUploadReqParam->isRollback);
    }
    
    return ADM_ECODE_SUCCESSED;
}

/*!
 *******************************************************************************
 * ucl日志上传到主控进度通知回调函数
 *
 * \note
 * 
 * \param[in]   fileID               上传文件标识id
 * \param[in]   pstTransProgress     上传进度信息
 * \retval      D32S
 *******************************************************************************
 */
D32S adm_tpm_dimUploadToHostProgressHandle
(
    const DCHAR *fileID,
    const ADM_DIM_SOA_TRANS_PROGRESS_INFO_T *pstTransProgress,
    void *priv
)
{
    ADM_TPM_UPLOAD_REQPARAM_T *pstTpmUploadReqParam = NULL;
    ADM_TPM_TRANS_ASYNCINFO_S   *pstTransAsyncInfo = NULL;

    if (ADM_IS_NULL(fileID) || ADM_IS_NULL(pstTransProgress))
    {
        log_e("Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }

    log_i("ucl log upload progress notify, fileID[%s], progress[%lld - %lld].", 
                    fileID, pstTransProgress->savedSize, pstTransProgress->totalSize);

    //!< 无私有属性数据则直接退出
    if (ADM_IS_NULL(adm_tpm_getTransferInfo()->stUploadReqParam.fileID))
    {
        log_e("Invalid input param, priv is null");
        return ADM_ECODE_INVALID_PARAM;
    }

    pstTpmUploadReqParam = (ADM_TPM_UPLOAD_REQPARAM_T *)&adm_tpm_getTransferInfo()->stUploadReqParam;
    if (0 != adm_strcmp(fileID, pstTpmUploadReqParam->fileID))
    {
        log_e("Invalid transfer fileID");
        return ADM_ECODE_INVALID_PARAM;
    }

    // 异步转同步 --- launch async cond
    if (NULL != pstTpmUploadReqParam->pAsyncInfo)
    {
        pstTransAsyncInfo = (ADM_TPM_TRANS_ASYNCINFO_S *)pstTpmUploadReqParam->pAsyncInfo;
        tpmSetTransFlush(pstTransAsyncInfo,TRUE);

    }

    return ADM_ECODE_SUCCESSED;
}

/*!
 *******************************************************************************
 * 创建ucl日志的保存路径
 *
 * \retval     DCHAR*           生成的 ewebServer 传输的 url
 *******************************************************************************
 */
static DCHAR *__adm_tpm_createUploadFilepathFromMaster(const DCHAR *pcfileName)
{
    DCHAR szUploadFilepath[ADM_COM_FILEPATH_LEN] = {0};
    const DCHAR *pcEwebRootDir   = NULL;

    pcEwebRootDir = adm_cfg_iniParserGetString(ADM_CFG_EWEB_DOC_UPLOAD_DIR, ADM_CFG_EWEB_DOC_UPLOAD_DIR_DEFAULT_VAL);

    (void)adm_memzero(szUploadFilepath, sizeof(szUploadFilepath));
    (void)adm_snprintf(szUploadFilepath, sizeof(szUploadFilepath), "%s%s", pcEwebRootDir, pcfileName);

    return adm_strdup(szUploadFilepath);
}

/*!
 *******************************************************************************
 * 执行ucl日志文件上传
 *
 * 
 * \param[in]    pcTransInstId        分包实例id
 * \param[out]   pcLogPath            日志路径
 * \retval       D32S
 *******************************************************************************
 */
D32S adm_tpm_CollectLogFromSysiToHost
( 
    DCHAR        *pcTransInstId,
    ADM_DIM_SOA_TRANS_FILE_TYPE_E eFileType,
    DCHAR        **ppcLogPath
)
{
    D32S     iRet = ADM_ECODE_SUCCESSED;
    D32S  errCode = ADM_ECODE_FAILED;
    DCHAR szEventBuf[ADM_COM_LEN_256] = {0}; 
    D64S lTimeStamp = 0;
    DCHAR FileIdName[128] = {0};
    ADM_TPM_TRANS_ASYNCINFO_S   *pstTransAsyncInfo = NULL;
    ADM_TPM_UPLOAD_REQPARAM_T   *pstTpmUploadReqParam = NULL;
    DCHAR acUploadFileId[128] = {0};

    pstTpmUploadReqParam = (ADM_TPM_UPLOAD_REQPARAM_T *)&adm_tpm_getTransferInfo()->stUploadReqParam;
    adm_tpm_cleanUploadReqParamInfo(pstTpmUploadReqParam);

    lTimeStamp = time(NULL);
    (void)adm_snprintf(FileIdName, sizeof(FileIdName), "%lld", lTimeStamp);

    pstTpmUploadReqParam->url = adm_strdup(adm_cfg_iniParserGetString(ADM_CFG_EWEB_ADDRESS_HEADER, ADM_CFG_EWEB_ADDRESS_HEADER_DEFAULT_VAL));
    (void)adm_memset(acUploadFileId, 0x00, sizeof(acUploadFileId), sizeof(acUploadFileId));
    (void)adm_snprintf(acUploadFileId, sizeof(acUploadFileId), "%s_%d", pcTransInstId, lTimeStamp);
    pstTpmUploadReqParam->fileID = adm_strdup(acUploadFileId);

    do
    {

        //!< 创建异步转同步数据结构信息 
        pstTransAsyncInfo = __adm_tpm_createTransAsyncInfo();
        if (ADM_IS_NULL(pstTransAsyncInfo))
        {
            iRet = ADM_ECODE_NO_MEMORY;
            log_e("Failed to create upload async info, err[%d]", iRet);

            //mod by wl 20220321
            (void)adm_memzero(szEventBuf, sizeof(szEventBuf));
            (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), "url[%s], failed to create upload async info, err[%d].", pstTpmUploadReqParam->url, iRet);
            break;
        }
        pstTpmUploadReqParam->pAsyncInfo = (void *)pstTransAsyncInfo;
    
        log_d("ReqInfo: fileID[%s], url[%s]", pstTpmUploadReqParam->fileID, pstTpmUploadReqParam->url);
        iRet = adm_dim_trans_requestTransferUploadToHost((D16U)htoi(pcTransInstId), ADM_TPM_MSG_REQTRANSDOWN_TIMEOUT, pstTpmUploadReqParam->fileID, 
                eFileType, pstTpmUploadReqParam->url, &pstTpmUploadReqParam->fileName, 
                &pstTpmUploadReqParam->hash, &pstTpmUploadReqParam->fileSize, &errCode);

        if (ADM_IS_ERR(iRet) || ADM_IS_ERR(errCode))
        {
            log_e("Failed to request upload from ucl, fileID[%s], iRet[%d], errCode[%d], err[%d]", 
                  pstTpmUploadReqParam->fileID, iRet, errCode, ADM_ECODE_TRANS_REQ_UPLOAD_FAILED);
            iRet = ADM_ECODE_TRANS_REQ_UPLOAD_FAILED;

            (void)adm_memzero(szEventBuf, sizeof(szEventBuf));
            (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), "transInstId[%s], url[%s], failed to request uploaded ucl log. err[%d]", 
                         pcTransInstId, pstTpmUploadReqParam->url, iRet);
            break;
        }

        /**兼容从控版本没有上keylog功能的问题*/
        if (0 == pstTpmUploadReqParam->fileSize)
        {
            iRet = ADM_ECODE_TRANS_REQ_UPLOAD_FAILED;
            break;
        }

        //!< 异步判断是否超时
        iRet = adm_tpm_collectLogWaitAsyncResult(pstTransAsyncInfo);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Wait upload result time out. err[%d]", iRet);
            break;
        }
        
        log_i("adm_dim_trans_requestTransferUploadToHost ok.");
        (void)adm_memzero(szEventBuf, sizeof(szEventBuf));
        (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), "req uploaded ucl log to url[%s] ok, async wait upload result, wait[%d]s", pstTpmUploadReqParam->url, ADM_COM_ASYNC_WAIT_TIME);

        adm_dpm_addEventAndReport(ADM_COM_MOD_TPM, FALSE, ADM_EVENT_ID_UP_001, ADM_OCS_EVENT_LEVEL_DEBUG, "Master collects slave logs successed.", szEventBuf);

    } while (FALSE);

    //!< 若上传失败，返回错误
    if (ADM_IS_ERR(iRet))
    {
        adm_dpm_addEventAndReport(ADM_COM_MOD_TPM, TRUE, ADM_EVENT_ID_UP_002, ADM_OCS_EVENT_LEVEL_ERROR, "Master collects slave logs failed.", szEventBuf);
    }
    else
    {
        //!< 若请求上传成功，异步上传结果为失败的情况下，则需要在这里向上层返回错误 
        iRet = pstTransAsyncInfo->stResult.result;
        if (ADM_IS_ERR(iRet))
        {
            /** 回调函数对端响应传包失败 */
            (void)adm_memzero(szEventBuf, sizeof(szEventBuf));
            (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), "async rsp upload err, url[%s], err[%d].", pstTpmUploadReqParam->url, iRet);
            adm_dpm_addEventAndReport(ADM_COM_MOD_TPM, TRUE, ADM_EVENT_ID_UP_002, ADM_OCS_EVENT_LEVEL_ERROR, "Master collects slave logs failed.", szEventBuf);
        }
        else
        {
            (void)adm_memzero(szEventBuf, sizeof(szEventBuf));
            (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), "url[%s], upload successed.", pstTpmUploadReqParam->url);
            adm_dpm_addEventAndReport(ADM_COM_MOD_TPM, FALSE, ADM_EVENT_ID_UP_001, ADM_OCS_EVENT_LEVEL_DEBUG, "Master collects slave logs successed.", szEventBuf);
            *ppcLogPath = __adm_tpm_createUploadFilepathFromMaster(pstTpmUploadReqParam->fileName);
        }
    }

    __adm_tpm_destroyTransAsyncInfo(pstTransAsyncInfo);
    pstTpmUploadReqParam->pAsyncInfo = NULL;

    log_e("Return upload request. iRet[%d]", iRet);

    return iRet;
}

#if 0
#endif


/*!
 *******************************************************************************
 * 更新Mater分包结果信息
 *
 * \note 
 * 
 * \param[in]   fileID              分包文件标识id
 * \param[in]   storePath           分包slave侧存储路径
 * \param[in]   pstTransResultInfo  分包结果信息
 * \retval      none
 *******************************************************************************
 */
static void __adm_tpm_dimMasterUpdateTransResultInfo
(
    const DCHAR                           *fileID,
    const DCHAR                           *storePath,
    const ADM_DIM_SOA_TRANS_RESULT_INFO_T *pstTransResultInfo
)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    ADM_TPS_TRANSRESULT_T stResult = {0};
    DCHAR szEventBuf[ADM_COM_LEN_256] = {0}; 

    if (ADM_IS_NULL(fileID) || ADM_IS_NULL(pstTransResultInfo))
    {
        log_e("Invalid input param");
        return;
    }

    (void)adm_memzero(&stResult, sizeof(stResult));
    stResult.result      = pstTransResultInfo->result;
    stResult.failReason  = ((NULL != pstTransResultInfo->realVal) ? (adm_strdup(pstTransResultInfo->realVal)) : (NULL));
    stResult.eTransState = adm_tpm_updateTransStateInfo(pstTransResultInfo->result, 
                                adm_tpm_getTransferInfo()->stTransReqParam.isRollback);
    log_d("Recv trans result, [%d][%s][%d]", stResult.result, ADM_PRINT_STR(stResult.failReason), stResult.eTransState);
    (void)adm_memzero(szEventBuf, sizeof(szEventBuf));
    (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), 
                    "Recv trans result, fileID[%s], storePath[%s], result[%d], failReason[%s], eTransState[%d].", 
                    fileID, 
                    ADM_PRINT_STR(storePath),
                    stResult.result,
                    ADM_PRINT_STR(stResult.failReason),
                    stResult.eTransState);
    adm_dpm_addEventAndReport(ADM_COM_MOD_TPM, FALSE, ADM_EVENT_ID_TI_013, ADM_OCS_EVENT_LEVEL_DEBUG, "vehicle recv do master transfer async result.", szEventBuf);

    iRet = adm_tps_updateTransPackageResult(fileID, &stResult, storePath);
    adm_free(stResult.failReason);
    if (ADM_IS_ERR(iRet))
    {
        log_e("Failed to update transfer package result, fileID[%s], err[%s]", 
                        fileID, iRet);
        return;
    }
    log_d("Recv trans result update to dps ok, fileID[%s], filePath[%s]", fileID, storePath);

    return;
}

/*!
 *******************************************************************************
 * 更新Master分包进度信息
 *
 * \note 
 * 
 * \param[in]   pstTransProgressInfo  分包进度信息
 * \param[in]   pstTpmTransReqParam   分包请求信息
 * \retval      none
 *******************************************************************************
 */
static void __adm_tpm_dimMasterUpdateTransProgressInfo
(
    const ADM_DIM_SOA_TRANS_PROGRESS_INFO_T *pstTransProgressInfo, 
    ADM_TPM_TRANSFER_REQPARAM_T             *pstTpmTransReqParam
)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    ADM_TPS_TRANSPROGRESS_T stProgress = {0};

    if (ADM_IS_NULL(pstTransProgressInfo) || ADM_IS_NULL(pstTpmTransReqParam))
    {
        log_e("Invalid input param");
        return;
    }

    (void)adm_memzero(&stProgress, sizeof(stProgress));
    stProgress.speed      = pstTransProgressInfo->speed;
    stProgress.totalTime  = 0;
    stProgress.remainTime = pstTransProgressInfo->remainTime;    
    stProgress.saveBytes  = pstTpmTransReqParam->baseSize + pstTransProgressInfo->savedSize;

    log_d("Recv trans progress, [%lld - %lld]", pstTransProgressInfo->savedSize, pstTransProgressInfo->totalSize);

    iRet = adm_tps_updateTransPackageProgress(pstTpmTransReqParam->fileID, &stProgress);
    if (ADM_IS_ERR(iRet))
    {
        log_e("Failed to update transfer package progress, fileID[%s], err[%s]", 
                        pstTpmTransReqParam->fileID, iRet);
        return;
    }

    return;
}



/*!
 *******************************************************************************
 * master 服务分包结果通知回调函数
 *
 * \note
 * 
 * \param[in]   fileID             分包文件标识id
 * \param[in]   storePath          分包成功时slave侧文件的存储路径
 * \param[in]   pstTransResultInfo 分包结果信息
 * \param[in]   priv               私有信息
 * \retval      D32S
 *******************************************************************************
 */
D32S adm_tpm_dimMasterTransResultHandle
(
    const DCHAR *fileID, 
    const DCHAR *storePath, 
    const ADM_DIM_SOA_TRANS_RESULT_INFO_T *pstTransResultInfo, 
    void *priv
)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    ADM_TPM_TRANS_ASYNCINFO_S   *pstTransAsyncInfo = NULL;
    ADM_TPM_TRANSFER_REQPARAM_T *pstTpmTransReqParam = NULL;

    if (ADM_IS_NULL(fileID) || ADM_IS_NULL(pstTransResultInfo))
    {
        log_e("Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }
    log_i("Recv transfer result notify, fileID[%s], result[%d].", fileID, pstTransResultInfo->result);

    //!< 无私有属性数据则直接退出
    if (ADM_IS_NULL(adm_tpm_getTransferInfo()->stTransReqParam.fileID))
    {
        log_e("Invalid input param, priv is null");
        return ADM_ECODE_INVALID_PARAM;
    }

    pstTpmTransReqParam = (ADM_TPM_TRANSFER_REQPARAM_T *)&adm_tpm_getTransferInfo()->stTransReqParam;
    if (0 != adm_strcmp(fileID, pstTpmTransReqParam->fileID))
    {
        log_e("Invalid transfer fileID");
        return ADM_ECODE_INVALID_PARAM;
    }

    __adm_tpm_dimMasterUpdateTransResultInfo(fileID, storePath, pstTransResultInfo);

    //!< 分包成功则需要将分包后的升级包存储路径更新到 pis  
    if (ADM_IS_SUCCESS(pstTransResultInfo->result))
    {
        log_d("Update trans package path to pis module. ecu[%s %s], path[%s]", 
                pstTpmTransReqParam->ecuReqid, pstTpmTransReqParam->ecuSwDid, storePath);
        iRet = adm_tps_updateEcuPkgSavePath2PIS(pstTpmTransReqParam->ecuReqid,
                    pstTpmTransReqParam->ecuSwDid,
                    pstTpmTransReqParam->isRollback);
        if (ADM_IS_ERR(iRet))
        {
            //!< 出错不用返回
            log_e("Failed to updateEcuPkgSavePath2PIS isRollback[%d] reqID[%s], swDID[%s], err:[%d]", 
                   pstTpmTransReqParam->isRollback, pstTpmTransReqParam->ecuReqid, pstTpmTransReqParam->ecuSwDid, iRet);
        }
    }

    // 异步转同步 --- launch async cond
    if (NULL != pstTpmTransReqParam->pAsyncInfo)
    {
        pstTransAsyncInfo = (ADM_TPM_TRANS_ASYNCINFO_S *)pstTpmTransReqParam->pAsyncInfo;
        tpmSetTransResult(pstTransAsyncInfo, pstTransResultInfo, pstTpmTransReqParam->isRollback);
    }
    
    return ADM_ECODE_SUCCESSED;
}


/*!
 *******************************************************************************
 * master 服务分包进度通知回调函数
 *
 * \note
 * 
 * \param[in]   fileID               分包文件标识id
 * \param[in]   pstTransProgressInfo 分包进度信息
 * \param[in]   priv                 私有信息
 * \retval      D32S
 *******************************************************************************
 */
D32S adm_tpm_dimMasterTransProgressHandle
(
    const DCHAR *fileID, 
    const ADM_DIM_SOA_TRANS_PROGRESS_INFO_T *pstTransProgressInfo, 
    void *priv
)
{
    static struct timespec stTs = {0};
    ADM_TPM_TRANSFER_REQPARAM_T *pstTpmTransReqParam = NULL;
    ADM_TPM_TRANS_ASYNCINFO_S   *pstTransAsyncInfo = NULL;

    if (ADM_IS_NULL(fileID) || ADM_IS_NULL(pstTransProgressInfo))
    {
        log_e("Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }

    log_i("Recv transfer progress notify, fileID[%s], progress[%lld - %lld].", 
                    fileID, pstTransProgressInfo->savedSize, pstTransProgressInfo->totalSize);

    //!< 无私有属性数据则直接退出
    if (ADM_IS_NULL(adm_tpm_getTransferInfo()->stTransReqParam.fileID))
    {
        log_e("Invalid input param, priv is null");
        return ADM_ECODE_INVALID_PARAM;
    }

    pstTpmTransReqParam = (ADM_TPM_TRANSFER_REQPARAM_T *)&adm_tpm_getTransferInfo()->stTransReqParam;
    if (0 != adm_strcmp(fileID, pstTpmTransReqParam->fileID))
    {
        log_e("Invalid transfer fileID");
        return ADM_ECODE_INVALID_PARAM;
    }

    if (__adm_tpm_isOverNotifyTimeWide(&stTs) || 
        (pstTransProgressInfo->savedSize == pstTransProgressInfo->totalSize))
    {
        __adm_tpm_MastertransStatisticsHandle(pstTransProgressInfo, pstTpmTransReqParam);
        __adm_tpm_dimMasterUpdateTransProgressInfo(pstTransProgressInfo, pstTpmTransReqParam);
    }

    // 异步转同步 --- launch async cond
    if (NULL != pstTpmTransReqParam->pAsyncInfo)
    {
        pstTransAsyncInfo = (ADM_TPM_TRANS_ASYNCINFO_S *)pstTpmTransReqParam->pAsyncInfo;
    
        tpmSetTransFlush(pstTransAsyncInfo,TRUE);

    }

    return ADM_ECODE_SUCCESSED;
}


/*!
 *******************************************************************************
 * 执行master下载升级包
 *
 * \note        master通过slave请求分包
 * 
 * \param[in]   pcTransInstId        分包实例id
 * \param[in]   pstTpmTransReqParam  分包请求信息
 * \retval      D32S
 *******************************************************************************
 */
static D32S __adm_tpm_doMasterTransfer
(
    DCHAR                       *pcTransInstId,
    ADM_TPM_TRANSFER_REQPARAM_T *pstTpmTransReqParam
)
{
    D32S     iRet = ADM_ECODE_SUCCESSED;
    D32S  errCode = ADM_ECODE_FAILED;
    DCHAR szEventBuf[ADM_COM_LEN_256] = {0}; 
    struct timespec tsNowtransbegin;
    struct timespec tsNowtransend;

    ADM_TPM_TRANS_ASYNCINFO_S          *pstTransAsyncInfo = NULL;

    if (ADM_IS_NULL(pstTpmTransReqParam))
    {
        log_e("Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }
    log_e("Entry transfer request.");

    do
    {
        //mod by wl 20220321
        (void)adm_memzero(szEventBuf, sizeof(szEventBuf));
        (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), 
                        "ecuReqid[%s], ecuSwDid[%s], transInstId[%s], url[%s], hash[%s], fileSize[%lld].", 
                        pstTpmTransReqParam->ecuReqid, 
                        pstTpmTransReqParam->ecuSwDid, 
                        pcTransInstId, 
                        pstTpmTransReqParam->url, 
                        pstTpmTransReqParam->hash, 
                        pstTpmTransReqParam->fileSize);
        adm_dpm_addEventAndReport(ADM_COM_MOD_TPM, FALSE, ADM_EVENT_ID_TI_001, ADM_OCS_EVENT_LEVEL_DEBUG, "vehicle use master transfer package.", szEventBuf);

        // 记录传输包的开始时间
        (void)clock_gettime(CLOCK_MONOTONIC, &tsNowtransbegin);

        //!< 创建异步转同步数据结构信息 
        pstTransAsyncInfo = __adm_tpm_createTransAsyncInfo();
        if (ADM_IS_NULL(pstTransAsyncInfo))
        {
            iRet = ADM_ECODE_NO_MEMORY;
            log_e("Failed to create trans async info, err[%d]", iRet);

            //mod by wl 20220321
            (void)adm_memzero(szEventBuf, sizeof(szEventBuf));
            (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), 
                            "ecuReqid[%s], ecuSwDid[%s], url[%s], failed to create trans async info, err[%d].", 
                            pstTpmTransReqParam->ecuReqid, 
                            pstTpmTransReqParam->ecuSwDid, 
                            pstTpmTransReqParam->url, 
                            iRet);
            break;
        }
        pstTpmTransReqParam->pAsyncInfo = (void *)pstTransAsyncInfo;
    
        //!< 请求 slave 从 master 下载升级包 
        log_d("ReqInfo: fileID[%s], url[%s], fileName[%s], fileSize[%lld]", 
                pstTpmTransReqParam->fileID, pstTpmTransReqParam->url, pstTpmTransReqParam->fileName, pstTpmTransReqParam->fileSize);
        iRet = adm_dim_trans_requestTransferDownloadFromHost((D16U)htoi(pcTransInstId), ADM_TPM_MSG_REQTRANSDOWN_TIMEOUT, pstTpmTransReqParam->fileID, 
                ADM_DIM_SOA_TRANS_FILE_TYPE_PKG, pstTpmTransReqParam->url, pstTpmTransReqParam->fileName, 
                pstTpmTransReqParam->fileSize, pstTpmTransReqParam->hash, &errCode);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to request transfer download from host, fileID[%s], iRet[%d], errCode[%d], err[%d]", 
                            pstTpmTransReqParam->fileID, iRet, errCode, ADM_ECODE_TRANS_REQ_DOWNLOAD_FAILED);
            iRet = ADM_ECODE_TRANS_REQ_DOWNLOAD_FAILED;

            (void)adm_memzero(szEventBuf, sizeof(szEventBuf));
            (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), 
                        "ecuReqid[%s], ecuSwDid[%s], transInstId[%s], url[%s], failed to request transfer download from host. err[%d]", 
                        pstTpmTransReqParam->ecuReqid, 
                        pstTpmTransReqParam->ecuSwDid, 
                        pcTransInstId, 
                        pstTpmTransReqParam->url, 
                        iRet);
            break;
        }
        log_e("adm_dim_soa_trans_requestTransferDownloadFromHost ok.");
        (void)adm_memzero(szEventBuf, sizeof(szEventBuf));
        (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), 
                        "req transfer[%s] ok, async wait trans result, wait[%d]s", 
                        pstTpmTransReqParam->url, 
                        ADM_COM_ASYNC_WAIT_TIME * 4);
        adm_dpm_addEventAndReport(ADM_COM_MOD_TPM, TRUE, ADM_EVENT_ID_TI_003, ADM_OCS_EVENT_LEVEL_DEBUG, "vehicle use master transfer package successed.", szEventBuf);

        //!< 异步判断是否超时
        iRet = adm_tpm_waitAsyncResult(pstTransAsyncInfo);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Wait transfer result time out. err[%d]", iRet);
            break;
        }

    } while (FALSE);

    //!< 若分包失败则更新分包结果，若请求分包成功则在上面的结果回调中更新分包结果
    if (ADM_IS_ERR(iRet))
    {
        __adm_tpm_updateTransResultInfo(pstTpmTransReqParam->fileID, pstTpmTransReqParam->isRollback, iRet);

        adm_dpm_addEventAndReport(ADM_COM_MOD_TPM, TRUE, ADM_EVENT_ID_TI_002, ADM_OCS_EVENT_LEVEL_ERROR, "vehicle use master transfer package failed!", szEventBuf);
    }
    else
    {
        //!< 若请求分包成功，异步分包结果为失败的情况下，则需要在这里向上层返回错误 
        iRet = pstTransAsyncInfo->stResult.result;

        if (ADM_IS_ERR(iRet))
        {
            /** 回调函数对端响应传包失败 */
            (void)adm_memzero(szEventBuf, sizeof(szEventBuf));
            (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), 
                            "ecuReqid[%s], ecuSwDid[%s], async rsp transfer err, url[%s], err[%d].", 
                            pstTpmTransReqParam->ecuReqid, 
                            pstTpmTransReqParam->ecuSwDid, 
                            pstTpmTransReqParam->url, 
                            iRet);
            adm_dpm_addEventAndReport(ADM_COM_MOD_TPM, TRUE, ADM_EVENT_ID_TI_002, ADM_OCS_EVENT_LEVEL_ERROR, "vehicle use master transfer package failed!", szEventBuf);
        }
        else
        {
            // 记录传输包的结束时间
            (void)clock_gettime(CLOCK_MONOTONIC, &tsNowtransend);
            (void)adm_memzero(szEventBuf, sizeof(szEventBuf));
            (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), "STATSPACK transOnceTime:ecuReqid[%s], ecuSwDid[%s], pkgType[%s], fileName[%s], transTime[%d]%s, fileSize[%lld]",
            pstTpmTransReqParam->ecuReqid, 
            pstTpmTransReqParam->ecuSwDid, 
            ((pstTpmTransReqParam->isRollback == FALSE) ? ("dstPkg") : ("srcPkg")),
            pstTpmTransReqParam->fileName,
            ((tsNowtransend.tv_sec - tsNowtransbegin.tv_sec) < 1 ? (tsNowtransend.tv_nsec / (1000 * 1000)) - (tsNowtransbegin.tv_nsec / (1000 * 1000)) : (tsNowtransend.tv_sec - tsNowtransbegin.tv_sec)),
            ((tsNowtransend.tv_sec - tsNowtransbegin.tv_sec) < 1 ? "ms" : "s"),
            pstTpmTransReqParam->fileSize);
            adm_dpm_addEventAndReport(ADM_COM_MOD_DPM, FALSE,ADM_EVENT_ID_DL_013, ADM_OCS_EVENT_LEVEL_INFO, szEventBuf, NULL);

            (void)adm_memzero(szEventBuf, sizeof(szEventBuf));
            (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), 
                            "ecuReqid[%s], ecuSwDid[%s], transfer successed.", 
                            pstTpmTransReqParam->ecuReqid, 
                            pstTpmTransReqParam->ecuSwDid);
            adm_dpm_addEventAndReport(ADM_COM_MOD_TPM, FALSE, ADM_EVENT_ID_TI_003, ADM_OCS_EVENT_LEVEL_DEBUG, "vehicle use master transfer package successed.", szEventBuf);
        }
    }

    __adm_tpm_destroyTransAsyncInfo(pstTransAsyncInfo);
    pstTpmTransReqParam->pAsyncInfo = NULL;


    log_e("Return transfer request. iRet[%d]", iRet);

    return iRet;
}



#if 0
#endif


/*!
 *******************************************************************************
 * 生成tpm分包请求参数结构信息.
 *
 * \note
 *
 * \param  none
 * \retval ADM_TPM_TRANSFER_REQPARAM_T*
 *******************************************************************************
 */
ADM_TPM_TRANSFER_REQPARAM_T* adm_tpm_newTransReqParamInfo(void)
{
    ADM_TPM_TRANSFER_REQPARAM_T *pstTransReqParam = NULL;

    pstTransReqParam = (ADM_TPM_TRANSFER_REQPARAM_T *)adm_malloc(sizeof(*pstTransReqParam));
    if (NULL != pstTransReqParam)
    {
        (void)adm_memzero(pstTransReqParam, sizeof(*pstTransReqParam));
    }

    return pstTransReqParam;
}

/*!
 *******************************************************************************
 * 清空tpm分包请求参数结构信息.
 *
 * \note 不释放最外层 pstTpmTransReqParam 的内存
 *
 * \param[in]   pstTpmTransReqParam tpm分包请求参数结构信息
 * \retval      none
 *******************************************************************************
 */
void adm_tpm_cleanTransReqParamInfo
(
    ADM_TPM_TRANSFER_REQPARAM_T *pstTpmTransReqParam
)
{
    if (ADM_IS_NULL(pstTpmTransReqParam))
    {
        return;
    }

    adm_free(pstTpmTransReqParam->url);
    adm_free(pstTpmTransReqParam->hash);
    adm_free(pstTpmTransReqParam->fileName);
    adm_free(pstTpmTransReqParam->ecuReqid);
    adm_free(pstTpmTransReqParam->ecuSwDid);
    adm_tps_freeTransFileID(&pstTpmTransReqParam->fileID);

    pstTpmTransReqParam->fileSize   = 0;
    pstTpmTransReqParam->baseSize   = 0;
    pstTpmTransReqParam->isRollback = FALSE;
    pstTpmTransReqParam->pAsyncInfo = NULL;

    return;
}

/*!
 *******************************************************************************
 * 释放tpm分包请求参数结构信息.
 *
 * \note
 *
 * \param[in]   pstTpmTransReqParam tpm分包请求参数结构信息
 * \retval      none
 *******************************************************************************
 */
void adm_tpm_delTransReqParamInfo
(
    ADM_TPM_TRANSFER_REQPARAM_T **pstTpmTransReqParam
)
{
    if (ADM_IS_NULL(pstTpmTransReqParam) || ADM_IS_NULL(*pstTpmTransReqParam))
    {
        return;
    }

    adm_tpm_cleanTransReqParamInfo(*pstTpmTransReqParam);
    adm_free(*pstTpmTransReqParam);

    return;
}


#if 0
#endif


/*!
 *******************************************************************************
 * 执行tpm分包
 *
 * \note
 * 
 * \param[in]   pcTransInstId        分包实例id
 * \param[in]   pstTpmTransReqParam  分包请求信息
 * \retval      D32S
 *******************************************************************************
 */
D32S adm_tpm_doTarnsfer
(
    DCHAR                       *pcTransInstId,
    ADM_TPM_TRANSFER_REQPARAM_T *pstTpmTransReqParam
)
{
    D32S iRet = ADM_ECODE_SUCCESSED;

    if (ADM_IS_NULL(pcTransInstId) || ADM_IS_NULL(pstTpmTransReqParam))
    {
        log_e("Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }
    log_i("[ABUP-Procedure] transfer package Start, transInstId[%s],fileID[%s]", pcTransInstId, pstTpmTransReqParam->fileID);
    iRet = __adm_tpm_doMasterTransfer( pcTransInstId, pstTpmTransReqParam);
    log_i("[ABUP-Procedure] transfer package End,ret[%d]",iRet);

//    if (0 == adm_strcmp(ADM_TPM_MASTER_TRANSINSTID, pcTransInstId))
//    {
//        iRet = __adm_tpm_doMasterTransfer(pcTransInstId, pstTpmTransReqParam);
//    }
//    else
//    {
//        iRet = __adm_tpm_doSlaveTransfer(pcTransInstId, pstTpmTransReqParam);
//    }

    if (ADM_IS_ERR(iRet))
    {
        log_e("Failed to transfer, transInstId[%s], fileID[%s], err:[%d]", 
                                pcTransInstId, pstTpmTransReqParam->fileID, iRet);
        return iRet; 
    }
    
    log_d("Transfer fileID[%s] ok, transInstId[%s]", pstTpmTransReqParam->fileID, pcTransInstId);

    return ADM_ECODE_SUCCESSED;
}

D32S adm_tpm_operateIdelTrans(ADM_TPM_OPERATE_IDEL_TRANS_E eOperation, D16U uiInstanceId)
{
    D32S iRet = ADM_ECODE_FAILED;
    D32S iErrCode = ADM_ECODE_FAILED;

    if (uiInstanceId == 0)
    {
        return ADM_ECODE_SUCCESSED;
    }

    do
    {
        iRet = adm_dim_trans_switchTransMode((D32S)eOperation, uiInstanceId, ADM_TPM_MSG_REQTRANSDOWN_TIMEOUT, &iErrCode);
        iRet = ADM_IS_SUCCESS(iRet) ? iErrCode : iRet;
        if (ADM_IS_ERR(iRet))
        {
            sleep(3);
        }
    } while (iRet != ADM_ECODE_SUCCESSED);

    return iRet;
}

static VEHICLE_VOLTAGE_STATE_E tpmGetCurrentHvMode(void)
{
    ADM_DIM_SOA_MSGID_DATA_ARRAY_T stReqMsgData = {};
    ADM_DIM_SOA_MSGID_DATA_ARRAY_T *pstRespMsgData = NULL;
    VEHICLE_VOLTAGE_STATE_E eType = HV_ABNORMAL;
    D32S iRet = ADM_ECODE_FAILED;
    D32S iErrCode = ADM_ECODE_FAILED;
    DBOOL bIsSwitch = DTRUE;
    D32S i = 0;

    ADM_COM_FUNC_ENTRY();

    stReqMsgData.msgIdArrayLength = 1;
    stReqMsgData.msgIdArray = (ADM_DIM_SOA_MSGID_T **)adm_malloc(sizeof(ADM_DIM_SOA_MSGID_T *) * (DSIZE)stReqMsgData.msgIdArrayLength);

    (void)adm_memzero(stReqMsgData.msgIdArray, sizeof(ADM_DIM_SOA_MSGID_T *) * stReqMsgData.msgIdArrayLength);

    stReqMsgData.msgIdArray[0] = (ADM_DIM_SOA_MSGID_T *)adm_malloc((DSIZE)sizeof(ADM_DIM_SOA_MSGID_T));

    log_i("begin request readVehicleStatus.");
    stReqMsgData.msgIdArray[0]->msgId = VEHICLE_HIGH_VOLTAGE_STATUS;
    stReqMsgData.msgIdArray[0]->msgIdData = NULL;

    do
    {
        /** 判断是否非下电状态； 否则，退出升级 */
        iRet = adm_dim_host_readVehicleStatus(adm_soa_getHostInstanceId(), 15, &stReqMsgData, &pstRespMsgData, &iErrCode);
        if (ADM_IS_ERR(iRet) || ADM_IS_ERR(iErrCode))
        {
            /** 默认获取不到则切换 */
            log_w("adm_dim_host_readVehicleStatus call error.iRet[%d], errCode[%d]", iRet, iErrCode);
            iRet = ADM_IS_ERR(iRet) ? iRet : iErrCode;
            break;
        }

        if (NULL == pstRespMsgData ||
            NULL == pstRespMsgData->msgIdArray ||
            VEHICLE_HIGH_VOLTAGE_STATUS != pstRespMsgData->msgIdArray[0]->msgId ||
            NULL == pstRespMsgData->msgIdArray[0]->msgIdData)
        {
            iRet = ADM_ECODE_INVALID_POINTER;
            log_e("Invalid response data.");
            break;
        }

        if (VAR_S32 != pstRespMsgData->msgIdArray[0]->msgIdData->dataType)
        {
            /** 给触发源成功,不要进行重试 */
            log_e("Value type invalid.type[%u]", pstRespMsgData->msgIdArray[0]->msgIdData->dataType);
            iRet = ADM_ECODE_INVALID_POINTER;
            break;
        }

        eType = (VEHICLE_VOLTAGE_STATE_E)pstRespMsgData->msgIdArray[0]->msgIdData->v.valueS32;

        for (i = 0; i < pstRespMsgData->msgIdArrayLength; i++)
        {
            if (NULL != pstRespMsgData->msgIdArray[i]->msgIdData)
            {
                if (VAR_STRING == pstRespMsgData->msgIdArray[i]->msgIdData->dataType)
                {
                    adm_free(pstRespMsgData->msgIdArray[i]->msgIdData->v.valueS);
                }
            }
            adm_free(pstRespMsgData->msgIdArray[i]->msgIdData);
            adm_free(pstRespMsgData->msgIdArray[i]);
        }
        adm_free(pstRespMsgData->msgIdArray);
        adm_free(pstRespMsgData);
    } while (FALSE);

    adm_free(stReqMsgData.msgIdArray[0]);
    adm_free(stReqMsgData.msgIdArray);

    ADM_COM_FUNC_EXIT();

    return eType;
}

D32S adm_tpm_switchHvMode(VEHICLE_CTRL_E eControlType)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    D32S errorCode = ADM_ECODE_FAILED;
    VEHICLE_VOLTAGE_STATE_E eCurrentState = HV_ABNORMAL;
    D32U uiRetryCount = 0;
    D32U uiSureRetryCnt = 0;
    VEHICLE_VOLTAGE_STATE_E eExpectState = HV_ABNORMAL;

    ADM_COM_FUNC_ENTRY();

    if (VEHICLE_CTRL_ENTER_HV_MODE == eControlType)
    {
        eExpectState = HV_HIGH;
    }

    if (VEHICLE_CTRL_EXIT_HV_MODE == eControlType)
    {
        eExpectState = HV_NO_HIGH;
    }

    do
    {
        /** 切换模式 */
        iRet = adm_dim_host_setVehicleStatus(adm_soa_getHostInstanceId(), 15, (D32U)eControlType, &errorCode);
        log_i("Switch to %d. instanceId[%u], iRet[%d], errcode[%d]", eControlType, adm_soa_getHostInstanceId(), iRet, errorCode);
        if (ADM_IS_ERR(iRet) || ADM_IS_ERR(errorCode))
        {
            uiRetryCount++;

            /** 3次重试 */
            if (3 == uiRetryCount)
            {
                if (VEHICLE_CTRL_ENTER_HV_MODE == eControlType)
                {
                    iRet = ADM_ECODE_INSTALL_ENTER_HIGH_VOLTAGE_FAILED;
                }
                else
                {
                    iRet = ADM_ECODE_INSTALL_EXIT_HIGH_VOLTAGE_FAILED;
                }

                break;
            }
        }

        iRet = ADM_IS_ERR(iRet) ? iRet : errorCode;

        if (ADM_IS_SUCCESS(iRet))
        {
            /** 上高压或者下高压成功后需要校验真实值是否正确 */
            if (VEHICLE_CTRL_ENTER_HV_MODE == eControlType || VEHICLE_CTRL_EXIT_HV_MODE == eControlType)
            {
                /** 切换成功之后，查询是否切换成功 */
                eCurrentState = tpmGetCurrentHvMode();

                /** 切换失败，进行重试 */
                if (eExpectState != eCurrentState)
                {
                    log_d("Switch failed. expect[%d], current[%d]", eExpectState, eCurrentState);
                    uiSureRetryCnt++;

                    if (3 == uiSureRetryCnt)
                    {
                        log_w("more than Retry count");
                        if (VEHICLE_CTRL_ENTER_HV_MODE == eControlType)
                        {
                            iRet = ADM_ECODE_INSTALL_ENTER_HIGH_VOLTAGE_FAILED;
                        }
                        else
                        {
                            iRet = ADM_ECODE_INSTALL_EXIT_HIGH_VOLTAGE_FAILED;
                        }
                        break;
                    }

                    log_d("Retry[%d]", uiSureRetryCnt);
                }
                else
                {
                    log_d("Switch success. expect[%d], current[%d]", eExpectState, eCurrentState);
                    break;
                }
            }
            else 
            {
                log_i("Not active sucess. exit.");
                break;
            }
        }

        sleep(2);
    } while (TRUE);

    ADM_COM_FUNC_EXIT();

    return iRet;
}