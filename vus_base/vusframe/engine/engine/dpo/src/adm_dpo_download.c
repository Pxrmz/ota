/**
 * @file    adm_dpo_download.c
 * @brief   Download package once 
 * @details 下载服务
 * @author  wangyanpeng
 * @version v0.1.0
 * @date    2020-11-06
 * @copyright www.abupdate.com
 */
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <openssl/hmac.h>
#include <openssl/md5.h>
#include <openssl/sha.h>
#include <openssl/bio.h>
#include <openssl/evp.h>

#include "adm_file.h"
#include "adm_dpo_log.h"
#include "adm_dpo_download.h"
#include "adm_dps_downloadService.h"
#include "adm_com_errorCode.h"
#include "adm_ocs_airProtocolAdapter.h"
#include "adm_com_hash.h"
#include "adm_com_lib.h"
#include "adm_string.h"
#include "adm_memory.h"


//!< 失败下载重试间隔时间
#define ADM_DPO_DOWNLOAD_INTERNAL_RETRY_TIME (10)
#define ADM_DPO_MAX_DOWNLOAD_RETRY_TIME (120)
#define ADM_DPO_IDEL_TRANS_INTERNAL_RETRY_TIME (3)

//!< hash校验进度通知时间间隔
#define ADM_DPO_HASH_NOTIFY_TIME_WIDE (2000)

/*!
 * @brief 文件下载进度中转计算信息
 */
typedef struct stDPOProgressInfo_
{
    void           *priv;        //!< 私有参数
    ADM_DPO_PROGRESS_NOTIFY_CALLBACK_F  fnDownNotify;   //!< 下载进度及结果通知函数

    D32S            speed;      //!< 下载速率(byte/s)
    struct timespec startTime;  //!< 下载开始时间，用于计算下载速度
    D64S            totalTime;  //!< 下载耗时[us]
    D64S            beginBytes; //!< 初次下载前文件大小
    D64S            downBytes;  //!< 下载的字节数(用于计算ocs两次进度下载的delta)
    D64S            totalBytes; //!< 总文件大小[totalBytes = beginBytes + downBytes]
} ADM_DPO_ROGRESSINFO_T;
typedef struct 
{
    pthread_mutex_t mutex;
    volatile ADM_DPO_OPERATE_IDEL_TRANS_E eOperation;
} ADM_DPO_IDEL_TRANS_STATE_T;
static ADM_DPO_IDEL_TRANS_STATE_T g_stIdelTransState ={.mutex = PTHREAD_MUTEX_INITIALIZER, .eOperation = ADM_OPERATE_INVALID};

#if 0
#endif

/*!
 *******************************************************************************
 * 判断是否是网络原因导致的
 *
 * \note 
 * 
 * \param[in]   iError 
 * \retval      D32S
 *******************************************************************************
 */
static D32S __adm_dpm_isCausedByNetwork(D32S iError)
{
    D32S iRet = ADM_ECODE_SUCCESSED;

    switch (iError)
    {
        case ADM_ECODE_OPERATION_TIMEDOUT:
        case ADM_ECODE_SEND_ERROR:
        case ADM_ECODE_RECV_ERROR:
        case ADM_ECODE_REQUEST_TIMEOUT:
        case ADM_ECODE_COULDNT_RESOLVE_HOST:
            iRet = ADM_ECODE_NET_CAUSE_DOWN_FAILED;
            break;
        default:
            iRet = iError;
            break;
    }

    return iRet;
}

/*!
 *******************************************************************************
 * hash进度通知时间宽度判断
 *
 * \note 获取时间使用的是 CLOCK_MONOTONIC 不随系统时间改变而变化。
 *          入参传入上次的时间，可以保证该函数可重入
 * 
 * \param[in]   pstTs   上次通知的时间信息
 * \retval      DBOOL   TRUE: 超过通知时间阈值，FALSE：未超过通知时间阈值
 *******************************************************************************
 */
static DBOOL dpoHashIsOverNotifyTimeWide(struct timespec *pstTs)
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
              ADM_DPO_HASH_NOTIFY_TIME_WIDE)
    {
        bRet = TRUE;
        pstTs->tv_sec = tsNow.tv_sec;
        pstTs->tv_nsec = tsNow.tv_nsec;
    }

    return bRet;
}

static void dpoformatMem2Hex(const D8U *pMd, D32U iLen, DCHAR *pOut, D32U iOutLen) 
{
    if(iLen != (D32U)0)
    {
        if((iLen *(D32U)2 +(D32U)1) <= iOutLen)
        {
            D32U resLen = iLen *(D32U)2;
            for (D32U i = (D32U)0; i < iLen; i++) 
            {
                (void)sprintf(&pOut[i * (D32U)2], "%02x", pMd[i]);
            }
            pOut[resLen] = '\0';
        }
    }
    return ;
}

#define IO_BUFFER_SIZE 1 * 1024 * 1024
/*!
 *******************************************************************************
 * hash检验进度通知
 *
 * \param[in]   pFileId   文件id
 * \param[in]   pFilePath   文件路径
 * \param[in]   iFileSize   文件大小
 * \param[in]   pOutHash   输出hash
 * \param[in]   iOutHashLen   输出hash长度
 * \param[in]   fnVerifyHashCb   校验hash进度通知函数
 * \retval      void
 *******************************************************************************
 */
static void dpoMdFileProNotify
(
    const EVP_MD *md, 
    const DCHAR *pFileId,
    const DCHAR *pFilePath, 
    D64U iFileSize ,
    DCHAR *pOutHash, 
    D32U iOutHashLen,
    ADM_DPO_VERIFYHASHPROGRESS_NOTIFY_CALLBACK_F fnVerifyHashCb
)
{
    BIO *in = NULL;
    BIO *bmd = NULL;
    // D8U iBuf[FILE_RW_BUFF_SIZE] = {0};
    D8U *pBuf = NULL;
    static struct timespec stTs = {0};
    D32S iRlen = 0;     //读取的文件大小
    D32S iLen = 0;
    D64U iSize = 0;     //校验hash剩余大小
    D32S iNeedsize = 0; //计算一次hash的大小
    D16U iProcess = 0;  //计算hash的进度

    if(ADM_IS_NULL(pFileId) ||ADM_IS_NULL(pFilePath) || ADM_IS_NULL(pOutHash))
    {
        return;
    }
    
    iSize = iFileSize;
    pBuf = (D8U*)adm_malloc(IO_BUFFER_SIZE);
    do {
        in = BIO_new(BIO_s_file());
        if(NULL == in) 
        {
            break;
        }
        bmd = BIO_new(BIO_f_md());
        if(NULL == bmd) 
        {
            break;
        }
        EVP_MD_CTX *mctx = NULL;
        if (BIO_get_md_ctx(bmd, &mctx) <= 0) 
        {
            break;
        }
        if ( 0 == EVP_DigestInit_ex(mctx, md, NULL)) 
        {
            break;
        }
        BIO *inp = BIO_push(bmd, in);
        if (BIO_read_filename(in, pFilePath) <= 0) 
        {
            break;
        }

        while ((BIO_pending(inp) || !BIO_eof(inp)) && (iSize > (D64U)0)) 
        {
            if (iSize <= (D64U)IO_BUFFER_SIZE)
            {
                iNeedsize = (D32S)iSize;
            }
            else
            {
                iNeedsize = IO_BUFFER_SIZE;
            }
            
            iRlen = BIO_read(inp, (DCHAR *)pBuf, iNeedsize);
            if (iRlen < 0 || 0 ==iRlen) 
            {
                break;
            }
            iSize -= (D64U)iRlen;

            if (dpoHashIsOverNotifyTimeWide(&stTs) || 0== iSize)
            {
                iProcess = (D16U)(((iFileSize - iSize)*100)/iFileSize);
                fnVerifyHashCb(pFileId,iProcess);
            }
            
            adm_memset(pBuf,'\0', IO_BUFFER_SIZE, IO_BUFFER_SIZE);
        }
        if(iRlen < 0) 
        {
            break;
        }
        iLen = BIO_gets(inp, (DCHAR *)pBuf, IO_BUFFER_SIZE);
        if(iLen < 0) 
        {
            break;
        }
        dpoformatMem2Hex(pBuf, (D32U)iLen, pOutHash, iOutHashLen);
    } while(FALSE);

    adm_free(pBuf);

    if(NULL != in) 
    {
        (void)BIO_free(in);
    }
    if(NULL != bmd) 
    {
        (void)BIO_free(bmd);
    }

    return;
}

/*!
 *******************************************************************************
 * hash检验进度通知
 *
 * 
 * \param[in]   pFileId   文件id
 * \param[in]   pFilePath   文件路径
 * \param[in]   iFileSize   文件大小
 * \param[in]   pOutHash   输出hash
 * \param[in]   iOutHashLen   输出hash长度
 * \param[in]   fnVerifyHashCb   校验hash进度通知函数
 * \retval      void
 *******************************************************************************
 */
static void dpoCalcSha256AndProcessNotify
(
    DCHAR* pFileId,
    DCHAR* pFilePath, 
    D64U iFileSize, 
    DCHAR* pOutHash, 
    D32U iOutHashLen,
    ADM_DPO_VERIFYHASHPROGRESS_NOTIFY_CALLBACK_F fnVerifyHashCb
)
{
    if(ADM_IS_NULL(pFileId) ||ADM_IS_NULL(pFilePath) || ADM_IS_NULL(pOutHash))
    {
        return;
    }

    dpoMdFileProNotify(EVP_sha256(), pFileId, pFilePath, iFileSize, pOutHash, iOutHashLen, fnVerifyHashCb);

    return;
}

/*!
 *******************************************************************************
 * 根据时间宽度记录下载日志
 *
 * \note 获取时间使用的是 CLOCK_MONOTONIC 不随系统时间改变而变化。
 *           该函数不可重入
 * 
 * \param[in]   pstNewTs 当前时间
 * \param[in]   downSize 当前已下载大小
 * \retval      none
 *******************************************************************************
 */
static void __adm_dpo_recordDownloading
(
    struct timespec *pstNewTs,
    D64S            downSize,
    D64S             totalSize
)
{
    static struct timespec stOldTs;

    if (ADM_IS_NULL(pstNewTs))
    {
        return;
    }

    if (stOldTs.tv_sec != pstNewTs->tv_sec)
    {
        log_i("Downloading/uploading upload/downSize:[%lld] - [%lld]", downSize, totalSize);
    }

    stOldTs.tv_sec = pstNewTs->tv_sec;
    return;
}


/*!
 *******************************************************************************
 * 释放DPO进度信息.
 *
 * \note
 *
 * \param[in]   pstDPOProgress 进度信息
 * \retval      none
 *******************************************************************************
 */
static void __adm_dpo_freeDPOProgress(ADM_DPO_ROGRESSINFO_T **pstDPOProgress)
{
    if (ADM_IS_NULL(pstDPOProgress) || ADM_IS_NULL(*pstDPOProgress))
    {
        return;
    }

    adm_free(*pstDPOProgress);
    return;
}


/*!
 *******************************************************************************
 * 创建DPO进度信息.
 *
 * \note
 *
 * \param[in]   saveBytes          文件当前已存在的大小(单位byte)
 * \param[in]   pstDownReqInfo     下载包信息
 * \retval      ADM_DPO_ROGRESSINFO_T*
 *******************************************************************************
 */
static ADM_DPO_ROGRESSINFO_T* __adm_dpo_createDPOProgress
(
    D64S                         saveBytes,
    ADM_DPO_DOWNLOAD_REQPARAM_T *pstDownReqInfo
)
{
    ADM_DPO_ROGRESSINFO_T *pstDPOProgress = NULL;
    
    if (ADM_IS_NULL(pstDownReqInfo))
    {
        log_e("Invalid input param");
        return NULL;
    }
    
    pstDPOProgress = (ADM_DPO_ROGRESSINFO_T *)adm_malloc(sizeof(*pstDPOProgress));
    if (ADM_IS_NULL(pstDPOProgress))
    {
        log_e("Failed to malloc");
        return NULL;
    }
    (void)adm_memset(pstDPOProgress, 0x00, sizeof(*pstDPOProgress), sizeof(*pstDPOProgress));

    pstDPOProgress->priv = pstDownReqInfo->priv;
    pstDPOProgress->fnDownNotify = pstDownReqInfo->fnProgressNotify;

    pstDPOProgress->speed = 0;
    pstDPOProgress->totalTime  = 0;
    pstDPOProgress->downBytes  = 0;
    pstDPOProgress->beginBytes = saveBytes;
    pstDPOProgress->totalBytes = pstDownReqInfo->fileSize;
    (void)clock_gettime(CLOCK_MONOTONIC, &pstDPOProgress->startTime);

    return pstDPOProgress;
}

/*!
 *******************************************************************************
 * OCS下载进度回调句柄.
 *
 * \note 
 *
 * \param[in]   pstProgress 下载进度信息
 * \param[in]   priv        调用下载的私有数据信息
 * \retval      none
 *******************************************************************************
 */
static void __adm_dpo_OCSProgressHandler
(
    ADM_OCA_FILE_PROGRESS_T* pstProgress, 
    void* priv
)
{
    D64S detlaSize = 0;
    //!< ocs上报速度过快(ms 内)，所以使用us来累加
    D64S detlaTimeUs = 0;
    struct timespec curTime = {0};
    ADM_DPO_ROGRESSINFO_T *pstDPOProgress = NULL;
    ADM_DPO_PROGRESS_NOTIFY_T stNotifyInfo = {0};
    
    if (NULL != priv)
    {
        (void)clock_gettime(CLOCK_MONOTONIC, &curTime);
        pstDPOProgress = (ADM_DPO_ROGRESSINFO_T *)priv;

        //!< OCS一次就可以把文件下载完成时，计算下载速度
        detlaTimeUs = (D64S)(curTime.tv_sec  - pstDPOProgress->startTime.tv_sec) * 1000000 +
                (D64S)(curTime.tv_nsec - pstDPOProgress->startTime.tv_nsec) / 1000;
        detlaSize = pstProgress->savedSize - pstDPOProgress->downBytes;

        //!< 纯粹记录日志，保证下载过程不至于肉眼假死
        __adm_dpo_recordDownloading(&curTime, pstProgress->savedSize, pstDPOProgress->totalBytes);

        // TODO: ocs会多次上报 0 == detlaSize 的通知
        if (0 == detlaSize)
        {
            return;
        }
        pstDPOProgress->totalTime += detlaTimeUs;

        //!< ocs 上报速度太快导致计算出的速度有问题
        if (0 >= detlaTimeUs)
        {
            detlaTimeUs = 1;
        }
        pstDPOProgress->speed = (D32S)((detlaSize * 1000000) / detlaTimeUs);

        pstDPOProgress->startTime.tv_sec  = curTime.tv_sec;
        pstDPOProgress->startTime.tv_nsec = curTime.tv_nsec;
        pstDPOProgress->downBytes = pstProgress->savedSize;

        //log_d("download deltaTime[%lld]us, deltaSize[%lld], totalTime[%lld]", 
        //            detlaTimeUs, detlaSize, pstDPOProgress->totalTime);

        if (NULL != pstDPOProgress->fnDownNotify)
        {
            (void)adm_memset(&stNotifyInfo, 0x00, sizeof(stNotifyInfo), sizeof(stNotifyInfo));
            stNotifyInfo.downSpeed  = pstDPOProgress->speed;
            stNotifyInfo.totalTime  = (D32S)(pstDPOProgress->totalTime / 1000);
            stNotifyInfo.remainTime = (D32S)pstProgress->remainTime;
            stNotifyInfo.totalSize  = pstDPOProgress->totalBytes;
            stNotifyInfo.savedSize  = pstDPOProgress->beginBytes + pstDPOProgress->downBytes;

            pstDPOProgress->fnDownNotify(&stNotifyInfo, pstDPOProgress->priv);
        }
    }

    return;
}

static void dpoDoProgressNotify(ADM_DPO_DOWNLOAD_REQPARAM_T *pstDownReqInfo)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    struct stat fileStat = {0};
    ADM_DPO_PROGRESS_NOTIFY_T stDownNotifyPara = {0};

    if (ADM_IS_NULL(pstDownReqInfo))
    {
        log_e("Invalid input param");
        return ;
    }

    if (NULL != pstDownReqInfo->fnProgressNotify)
    {
        iRet = stat(pstDownReqInfo->filePath, &fileStat);
        if (ADM_IS_ERR(iRet))
        {
            return ;
        }

        (void)adm_memzero(&stDownNotifyPara, sizeof(stDownNotifyPara));
        stDownNotifyPara.savedSize = (D64S)fileStat.st_size;
        stDownNotifyPara.totalSize = pstDownReqInfo->fileSize;
        stDownNotifyPara.totalTime = 0;
        
        pstDownReqInfo->fnProgressNotify(&stDownNotifyPara, pstDownReqInfo->priv);
    }

    return;
}


/*!
 *******************************************************************************
 * 执行OCS下载升级包.
 *
 * \note 同步接口.
 *
 * \param[in]   pstDownReqInfo  待下载请求信息
 * \retval      D32S            0: 成功 non-0：失败
 *******************************************************************************
 */
static D32S __adm_dpo_doOCSDownload
(
    ADM_DPO_DOWNLOAD_REQPARAM_T *pstDownReqInfo
)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    struct stat fileStat = {0};
    ADM_DPO_ROGRESSINFO_T *pstDPOProgress = NULL;
    ADM_DPO_PROGRESS_NOTIFY_T stDownNotifyPara = {0};
    ADM_OCA_FILE_DOWNLOAD_INFO_T stDownloadInfo = {0};

    if (ADM_IS_NULL(pstDownReqInfo))
    {
        log_e("Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }

    if (ADM_IS_NULL(pstDownReqInfo->fnDownloadCb)) {
        log_e("Input param pstDownReqInfo fnDownloadCb is nil");
        return ADM_ECODE_INVALID_PARAM;
    }

	iRet = stat(pstDownReqInfo->filePath, &fileStat);
	if (ADM_IS_ERR(iRet))
	{
	    log_e("Failed to stat file info, filePath[%s], err[%d], strerror[%s]", 
            pstDownReqInfo->filePath, iRet, strerror(errno));
        return ADM_ECODE_GET_FILESIZE_FILED;
	}
    
    //!< 若实测的文件大小大于将要下载的文件大小，则将缓存的文件删除
    if (fileStat.st_size > pstDownReqInfo->fileSize)
    {
        log_e("File really size[%lld] greater assume size[%lld], filePath[%s]", 
                (D64S)fileStat.st_size, pstDownReqInfo->fileSize, pstDownReqInfo->filePath);
        (void)adm_fremove(pstDownReqInfo->filePath);
        fileStat.st_size = 0;
    }

    log_i("To down url[%s]", pstDownReqInfo->url);
    log_i("To down from[%lld - %lld]", 
            (D64S)fileStat.st_size, pstDownReqInfo->fileSize);

    if (fileStat.st_size < pstDownReqInfo->fileSize)
    {
        pstDPOProgress = __adm_dpo_createDPOProgress((D64S)fileStat.st_size, pstDownReqInfo);
        if (ADM_IS_NULL(pstDPOProgress))
        {
            //!< 一般情况下不会出错，故不必返回报错，回调中做好判断保护即可
            log_e("Failed to create DPO progress info");
        }

        (void)adm_memset(&stDownloadInfo, 0x00, sizeof(stDownloadInfo), sizeof(stDownloadInfo));
        stDownloadInfo.startPosition = (D64S)fileStat.st_size;
        stDownloadInfo.endPosition = 0;
        stDownloadInfo.savePath = pstDownReqInfo->filePath;
        stDownloadInfo.priv = (void *)pstDPOProgress;
        stDownloadInfo.cbDownloadProgress = __adm_dpo_OCSProgressHandler;

        //!< 将下载的句柄剥离到application层，文件下载只需要通过application传入的回调即可
        iRet = pstDownReqInfo->fnDownloadCb(pstDownReqInfo->url, &stDownloadInfo);
        // TODO: debug待调试为何返回 UNKNOW
        // iRet = ((ADM_ECODE_NET_UNKNOWN == iRet) ? (ADM_ECODE_SUCCESSED) : (iRet));
        if (ADM_IS_ERR(iRet))
        {
            //下载失败通知一次当前的进度
            dpoDoProgressNotify(pstDownReqInfo);

            //下载失败默认为网络问题导致下载失败 mod by wl 20220331
            iRet = __adm_dpm_isCausedByNetwork(iRet);
            __adm_dpo_freeDPOProgress(&pstDPOProgress);
            log_e("adm_ocs_startDownload failed, reason err[%d]", iRet);
            return iRet;
        }
    }
    else
    {
        //!< 结束后通知一次，防止上面if不满足时，未有进度上报
        dpoDoProgressNotify(pstDownReqInfo);
    }

    __adm_dpo_freeDPOProgress(&pstDPOProgress);
    log_i("OCS download[url(%s)-filePath(%s)] successed", 
            pstDownReqInfo->url, pstDownReqInfo->filePath);

    return ADM_ECODE_SUCCESSED;
}

static DVOID dpoCalcSha256AndSupportPause(ADM_DPO_DOWNLOAD_REQPARAM_T *pstRequest, DCHAR *out, D32U outLen)
{
    BIO *in = NULL;
    BIO *bmd = NULL;
    DCHAR *pBuf = NULL;
    const EVP_MD *md = NULL;

    if (ADM_IS_NULL(pstRequest) || ADM_IS_NULL(pstRequest->filePath))
    {
        log_e("Invalid param.");
        return;
    }

    do
    {
        md = EVP_sha256();

        in = BIO_new(BIO_s_file());
        if (NULL == in)
        {
            break;
        }
        bmd = BIO_new(BIO_f_md());
        if (NULL == bmd)
        {
            break;
        }
        EVP_MD_CTX *mctx = NULL;
        if (BIO_get_md_ctx(bmd, &mctx) <= 0)
        {
            break;
        }
        if (0 == EVP_DigestInit_ex(mctx, md, NULL))
        {
            break;
        }
        BIO *inp = BIO_push(bmd, in);
        if (BIO_read_filename(in, pstRequest->filePath) <= 0)
        {
            break;
        }

        pBuf = adm_malloc(IO_BUFFER_SIZE);
        D32S rlen = 0;
        while ((0 != BIO_pending(inp)) || (0 == BIO_eof(inp)))
        {
            while (ADM_PAUSE_TRANS == g_stIdelTransState.eOperation)
            {
                dpoDoProgressNotify(pstRequest);
                log_i("Cal hash256. Waiting for idle transfer condition satisfied.");
                (void)sleep(ADM_DPO_IDEL_TRANS_INTERNAL_RETRY_TIME);
                continue;
            }
            adm_memset(pBuf, '\0', IO_BUFFER_SIZE, IO_BUFFER_SIZE);
            rlen = BIO_read(inp, pBuf, IO_BUFFER_SIZE);
            if (rlen < 0 || rlen == 0)
            {
                break;
            }
        }
        if (rlen < 0)
        {
            break;
        }

        D32S len = BIO_gets(inp, pBuf, IO_BUFFER_SIZE);
        if (len < 0)
        {
            break;
        }

        dpoformatMem2Hex((const D8U *)pBuf, (D32U)len, out, outLen);
        adm_free(pBuf);
    } while (FALSE);

    if (NULL != in)
    {
        (void)BIO_free(in);
    }
    if (NULL != bmd)
    {
        (void)BIO_free(bmd);
    }

    return;
}

#define ADM_STR_SAFE_OUTPUT(S) (NULL != ((const DCHAR *)(S)) ? ((const DCHAR *)(S)) : "")
/*!
 *******************************************************************************
 * 检查下载文件信息.
 *
 * \note 文件大小、hash值
 *
 * \param[in]   pstDownReqInfo   待下载请求信息
 * \retval      D32S       0: 成功 non-0：失败
 *******************************************************************************
 */
D32S adm_dpo_checkDownloadFile
(
    ADM_DPO_DOWNLOAD_REQPARAM_T *pstDownReqInfo
)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    DCHAR fileHash[256] = {0};
    DCHAR szEventBuf[ADM_COM_LONGDATA_LEN] = {0};
    DCHAR *pFileId = NULL;
    struct stat fileStat = {0};
    struct timespec tsNowbegin;
    struct timespec tsNowend;

    if (ADM_IS_NULL(pstDownReqInfo))
    {
        log_e("Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }
    (void)clock_gettime(CLOCK_MONOTONIC, &tsNowbegin);

    //!< 下载文件的完整性校验
	iRet = stat(pstDownReqInfo->filePath, &fileStat);
	if (ADM_IS_ERR(iRet))
	{
	    log_e("Failed to stat file info, filePath[%s], err[%d], strerror[%s]", 
            pstDownReqInfo->filePath, iRet, strerror(errno));
        return iRet;
    }
    log_i("File DownSize[%lld] ReqSize[%lld]", fileStat.st_size, pstDownReqInfo->fileSize);

    if (fileStat.st_size != pstDownReqInfo->fileSize)
    {
        log_e("File really size[%lld] greater assume size[%lld], filePath[%s], err[%d]", 
                fileStat.st_size, pstDownReqInfo->fileSize, pstDownReqInfo->filePath, ADM_ECODE_DOWN_VERIFY_SIZE_FAIL);
        return ADM_ECODE_DOWN_VERIFY_SIZE_FAIL;
    }

    //!<计算文件hash 
    if(NULL != pstDownReqInfo->fnVerifyHashCb)
    {
        //计算文件hash并通知校验hash进度
        pFileId = adm_strdup((DCHAR *)pstDownReqInfo->priv);
        dpoCalcSha256AndProcessNotify(pFileId, pstDownReqInfo->filePath,pstDownReqInfo->fileSize,
                 fileHash, (D32U)(sizeof(fileHash)/sizeof(fileHash[0])), pstDownReqInfo->fnVerifyHashCb);
        adm_free(pFileId);
    }
    else if (ADM_DPO_TRANSFER == pstDownReqInfo->eType)
    {
        log_i("Cal transfer hash256.");
        dpoCalcSha256AndSupportPause(pstDownReqInfo, fileHash, (D32U)(sizeof(fileHash) / sizeof(fileHash[0])));
    }
    else
    {
        //计算文件hash
        adm_com_sha256File(pstDownReqInfo->filePath, fileHash, (D32U)(sizeof(fileHash)/sizeof(fileHash[0])));
    }

    if ('\0' == fileHash[0] || 0 != adm_strcmp(pstDownReqInfo->hash, fileHash))
    {
        log_e("Checkhash failed, expect[%s], calc[%s], err[%d]", 
                    pstDownReqInfo->hash, ADM_STR_SAFE_OUTPUT(fileHash), ADM_ECODE_DOWN_VERIFY_HASH_FAILED);
        return ADM_ECODE_DOWN_VERIFY_HASH_FAILED;
    }

    //TODO:目前只有升级包下载支持无限次重试，估先已这个条件作为升级包下载的条件进行过滤
    //后续hash校验会做成进度上报的形式之后再做优化.
    if(TRUE == pstDownReqInfo->IsUnlimitretry)
    {
        (void)clock_gettime(CLOCK_MONOTONIC, &tsNowend);
        (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), "STATSPACK hashTime:filePath[%s], hashTime[%d]%s, fileSize[%lld]",
        pstDownReqInfo->filePath,
        ((tsNowend.tv_sec - tsNowbegin.tv_sec) < 1 ? (tsNowend.tv_nsec / (1000 * 1000)) - (tsNowbegin.tv_nsec / (1000 * 1000)) : (tsNowend.tv_sec - tsNowbegin.tv_sec)),
        ((tsNowend.tv_sec - tsNowbegin.tv_sec) < 1 ? "ms" : "s"),
        pstDownReqInfo->fileSize);
        adm_dps_eventReport(ADM_COM_MOD_DPO, FALSE, ADM_EVENT_ID_DL_015, ADM_OCS_EVENT_LEVEL_DEBUG, szEventBuf, NULL);
    }

    return ADM_ECODE_SUCCESSED;
}


#if 0
#endif

/*!
 *******************************************************************************
 * 检查上传信息是否合法.
 *
 * \note 
 *
 * \param[in]   pstProgress 上传进度信息
 * \param[in]   priv        调用下载的私有数据信息
 * \retval      none
 *******************************************************************************
 */
static D32S __adm_dpo_checkUploadParam(ADM_DPO_UPLOAD_REQPARAM_T *pstUploadReqInfo)
{
    //!< 入参校验
    if (ADM_IS_NULL(pstUploadReqInfo)) 
    {
        log_e("Input param is nil, err[%d]", ADM_ECODE_INVALID_PARAM);
        return ADM_ECODE_INVALID_PARAM;
    }
    
    if (ADM_IS_NULL(pstUploadReqInfo->url)) 
    {
        log_e("Input param url is nil, err[%d]", ADM_ECODE_INVALID_PARAM);
        return ADM_ECODE_INVALID_PARAM;
    }
    if (ADM_IS_NULL(pstUploadReqInfo->fileName)) 
    {
        log_e("Input param fileName is nil, err[%d]", ADM_ECODE_INVALID_PARAM);
        return ADM_ECODE_INVALID_PARAM;
    }
    if (ADM_IS_NULL(pstUploadReqInfo->filePath)) 
    {
        log_e("Input param filePath is nil, err[%d]", ADM_ECODE_INVALID_PARAM);
        return ADM_ECODE_INVALID_PARAM;
    }
    if (TRUE != adm_com_isDirExist(pstUploadReqInfo->filePath))
    {
        log_e("Upload request file path is not exist, err[%d]", ADM_ECODE_UPLOAD_FILE_NOEXIST);
        return ADM_ECODE_UPLOAD_FILE_NOEXIST;
    }

    if (ADM_IS_NULL(pstUploadReqInfo->fileHash)) 
    {
        log_e("Input param fileHash is nil, err[%d]", ADM_ECODE_INVALID_PARAM);
        return ADM_ECODE_INVALID_PARAM;
    }
    if (0 >= pstUploadReqInfo->fileSize) 
    {
        log_e("Input param fileSize is invalied(%d), err[%d]", pstUploadReqInfo->fileSize, ADM_ECODE_INVALID_PARAM);
        return ADM_ECODE_INVALID_PARAM;
    }

    if (ADM_IS_NULL(pstUploadReqInfo->fnUploadCb)) 
    {
        log_e("Input param fnUploadCb is nil, err[%d]", ADM_ECODE_INVALID_PARAM);
        return ADM_ECODE_INVALID_PARAM;
    }

    return ADM_ECODE_SUCCESSED;
}


/*!
 *******************************************************************************
 * 创建DPO上传进度信息.
 *
 * \note
 *
 * \param[in]   pstUploadReqInfo     上传包信息
 * \retval      ADM_DPO_ROGRESSINFO_T*
 *******************************************************************************
 */
static ADM_DPO_ROGRESSINFO_T* __adm_dpo_createDPOUploadProgress
(
    ADM_DPO_UPLOAD_REQPARAM_T *pstUploadReqInfo
)
{

    ADM_DPO_ROGRESSINFO_T *pstDPOProgress = NULL;
    
    if (ADM_IS_NULL(pstUploadReqInfo))
    {
        log_e("Invalid input param");
        return NULL;
    }
    
    pstDPOProgress = (ADM_DPO_ROGRESSINFO_T *)adm_malloc(sizeof(*pstDPOProgress));
    if (ADM_IS_NULL(pstDPOProgress))
    {
        log_e("Failed to malloc");
        return NULL;
    }
    (void)adm_memset(pstDPOProgress, 0x00, sizeof(*pstDPOProgress), sizeof(*pstDPOProgress));

    pstDPOProgress->priv = pstUploadReqInfo->priv;
    pstDPOProgress->fnDownNotify = pstUploadReqInfo->fnProgressNotify;

    pstDPOProgress->speed = 0;
    pstDPOProgress->totalTime  = 0;
    pstDPOProgress->downBytes  = 0;
    pstDPOProgress->beginBytes = 0;
    pstDPOProgress->totalBytes = pstUploadReqInfo->fileSize;
    (void)clock_gettime(CLOCK_MONOTONIC, &pstDPOProgress->startTime);

    return pstDPOProgress;
}

/*!
 *******************************************************************************
 * 执行OCS上传文件.
 *
 * \note 同步接口.
 *
 * \param[in]   pstDownReqInfo   待下载请求信息
 * \retval      D32S       0: 成功 non-0：失败
 *******************************************************************************
 */
static D32S __adm_dpo_doOCSUploadFile
(
    ADM_DPO_UPLOAD_REQPARAM_T *pstUploadReqInfo
)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    ADM_DPO_ROGRESSINFO_T *pstDPOProgress = NULL;
    ADM_OCA_SIMPLE_FILEUPLOAD_INFO_T stUploadInfo = {0};
    DCHAR *pcRspOutData = NULL;

    if (ADM_IS_NULL(pstUploadReqInfo))
    {
        log_e("Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }

    log_i("To upload url[%s]", pstUploadReqInfo->url);
    log_i("To upload filePath[%s], fileSize[%lld]", pstUploadReqInfo->filePath, pstUploadReqInfo->fileSize);

    do 
    {
        pstDPOProgress = __adm_dpo_createDPOUploadProgress(pstUploadReqInfo);
        if (ADM_IS_NULL(pstDPOProgress))
        {
            //!< 一般情况下不会出错，故不必返回报错，回调中做好判断保护即可
            log_e("Failed to create DPO upload progress info.");
        }

        //!< 此处不必申请内存，内部调用接口后，接口内部会拷贝对应的数据
        (void)adm_memzero(&stUploadInfo,  sizeof(stUploadInfo));
        stUploadInfo.url      = pstUploadReqInfo->url;
        stUploadInfo.fileName = pstUploadReqInfo->fileName;
        stUploadInfo.filePath = pstUploadReqInfo->filePath;
        stUploadInfo.fileHash = pstUploadReqInfo->fileHash;
        stUploadInfo.fileSize = pstUploadReqInfo->fileSize;
        stUploadInfo.priv     = (void *)pstDPOProgress;
        stUploadInfo.cbUploadProgress = __adm_dpo_OCSProgressHandler;

        //!< 将下载的句柄剥离到application层，文件下载只需要通过application传入的回调即可
        iRet = pstUploadReqInfo->fnUploadCb(&stUploadInfo, &pcRspOutData);
        // TODO: debug待调试为何返回 UNKNOW
        iRet = ((ADM_ECODE_NET_UNKNOWN == iRet) ? (ADM_ECODE_SUCCESSED) : (iRet));
        if (ADM_IS_ERR(iRet))
        {
            iRet = __adm_dpm_isCausedByNetwork(iRet);
            __adm_dpo_freeDPOProgress(&pstDPOProgress);
            log_e("upload failed, reason err[%d]", iRet);
            return iRet;
        }

        //log_i("upload rsp message, rsp[%s]", pcRspOutData);
    } while(FALSE);

    adm_free(pcRspOutData);
    __adm_dpo_freeDPOProgress(&pstDPOProgress);

    return iRet;
}

#if 0
#endif

/*!
 *******************************************************************************
 * 生成DPO下载请求参数结构信息.
 *
 * \note
 *
 * \param  none
 * \retval ADM_DPO_DOWNLOAD_REQPARAM_T*
 *******************************************************************************
 */
ADM_DPO_DOWNLOAD_REQPARAM_T* adm_dpo_newDownReqParamInfo(void)
{
    ADM_DPO_DOWNLOAD_REQPARAM_T *pstDownReqInfo = NULL;

    pstDownReqInfo = (ADM_DPO_DOWNLOAD_REQPARAM_T *)adm_malloc(sizeof(*pstDownReqInfo));
    if (NULL != pstDownReqInfo)
    {
        (void)adm_memzero(pstDownReqInfo, sizeof(*pstDownReqInfo));
    }

    return pstDownReqInfo;
}

/*!
 *******************************************************************************
 * 清空DPO下载请求参数结构信息.
 *
 * \note 不释放最外层 pstDownReqInfo 的内存
 *
 * \param[in]   pstDownReqInfo 下载请求参数结构信息
 * \retval      none
 *******************************************************************************
 */
void adm_dpo_cleanDownReqParamInfo
(
    ADM_DPO_DOWNLOAD_REQPARAM_T *pstDownReqInfo
)
{
    if (ADM_IS_NULL(pstDownReqInfo))
    {
        return;
    }

    adm_free(pstDownReqInfo->url);
    adm_free(pstDownReqInfo->filePath);
    adm_free(pstDownReqInfo->hash);

    pstDownReqInfo->fileSize = 0;

    return;
}

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
void adm_dpo_delDownReqParamInfo
(
    ADM_DPO_DOWNLOAD_REQPARAM_T **pstDownReqInfo
)
{
    if (ADM_IS_NULL(pstDownReqInfo) || ADM_IS_NULL(*pstDownReqInfo))
    {
        return;
    }

    adm_dpo_cleanDownReqParamInfo(*pstDownReqInfo);
    adm_free(*pstDownReqInfo);

    return;
}




/*!
 *******************************************************************************
 * 生成DPO上传请求参数结构信息.
 *
 * \note
 *
 * \param  none
 * \retval ADM_DPO_UPLOAD_REQPARAM_T*
 *******************************************************************************
 */
ADM_DPO_UPLOAD_REQPARAM_T* adm_dpo_newUploadReqParamInfo(void)
{
    ADM_DPO_UPLOAD_REQPARAM_T *pstUploadReqInfo = NULL;

    pstUploadReqInfo = (ADM_DPO_UPLOAD_REQPARAM_T *)adm_malloc(sizeof(*pstUploadReqInfo));
    if (NULL != pstUploadReqInfo)
    {
        (void)adm_memzero(pstUploadReqInfo, sizeof(*pstUploadReqInfo));
    }

    return pstUploadReqInfo;
}

/*!
 *******************************************************************************
 * 清空DPO上传请求参数结构信息.
 *
 * \note 不释放最外层 pstDownReqInfo 的内存
 *
 * \param[in]   pstDownReqInfo 上传请求参数结构信息
 * \retval      none
 *******************************************************************************
 */
void adm_dpo_cleanUploadReqParamInfo
(
    ADM_DPO_UPLOAD_REQPARAM_T *pstUploadReqInfo
)
{
    if (ADM_IS_NULL(pstUploadReqInfo))
    {
        return;
    }

    adm_free(pstUploadReqInfo->url);
    adm_free(pstUploadReqInfo->fileName);
    adm_free(pstUploadReqInfo->filePath);
    adm_free(pstUploadReqInfo->fileHash);
    
    pstUploadReqInfo->fileSize = 0;

    return;
}

/*!
 *******************************************************************************
 * 释放DPO上传请求参数结构信息.
 *
 * \note
 *
 * \param[in]   pstDownReqInfo 上传请求参数结构信息
 * \retval      none
 *******************************************************************************
 */
void adm_dpo_delUploadReqParamInfo
(
    ADM_DPO_UPLOAD_REQPARAM_T **pstUploadReqInfo
)
{
    if (ADM_IS_NULL(pstUploadReqInfo) || ADM_IS_NULL(*pstUploadReqInfo))
    {
        return;
    }

    adm_dpo_cleanUploadReqParamInfo(*pstUploadReqInfo);
    adm_free(*pstUploadReqInfo);

    return;
}


#if 0
#endif


/*!
 *******************************************************************************
 * 执行下载文件.
 *
 * \note 同步接口
 *
 * \param[in]   pstDownReqInfo    待下载请求信息
 * \retval      D32S       0: 成功 non-0：失败
 *******************************************************************************
 */
D32S adm_dpo_doDownloadFile
(
    ADM_DPO_DOWNLOAD_REQPARAM_T *pstDownReqInfo
)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    D32S i = 0;
    D32S iSaveRetriesCnt = 0;
    D32S iNetworkRetriesCnt = 0;
    D32U uiWaitSenconds = 0;

    if (ADM_IS_NULL(pstDownReqInfo))
    {
        log_e("Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }

    if (ADM_IS_NULL(pstDownReqInfo->url))
    {
        log_e("Download request param url is null");
        return ADM_ECODE_INVALID_PARAM;
    }

    if (ADM_IS_NULL(pstDownReqInfo->filePath))
    {
        log_e("Download request param file path is null");
        return ADM_ECODE_INVALID_PARAM;
    }

    if (ADM_IS_NULL(pstDownReqInfo->hash))
    {
        log_e("Download request param hash info is null");
        return ADM_ECODE_INVALID_PARAM;
    }

    //保存下载重试次数
    iSaveRetriesCnt = pstDownReqInfo->downRetriesCnt;

    while(i < (pstDownReqInfo->downRetriesCnt + 1))
    {
        /** 非空闲传输控制短暂的休眠时间 */
        if (ADM_PAUSE_TRANS == g_stIdelTransState.eOperation)
        {
            /** 执行一次下载进度通知 */
            dpoDoProgressNotify(pstDownReqInfo);
            log_i("Waiting for idle transfer condition satisfied.");
            uiWaitSenconds = ADM_DPO_IDEL_TRANS_INTERNAL_RETRY_TIME;
            (void)sleep(uiWaitSenconds);
            continue;
        }
        else 
        {
            /** 非空闲传输最大休眠时间控制不超过主控超时等待时间 */
            if ((D32U)i * (D32U)ADM_DPO_DOWNLOAD_INTERNAL_RETRY_TIME > ADM_DPO_MAX_DOWNLOAD_RETRY_TIME)
            {
                uiWaitSenconds = ADM_DPO_MAX_DOWNLOAD_RETRY_TIME;
            }
            else 
            {
                uiWaitSenconds = (D32U)i * (D32U)ADM_DPO_DOWNLOAD_INTERNAL_RETRY_TIME;
            }
        }

        (void)sleep(uiWaitSenconds);

        iRet = adm_com_createFileDirAndFile(pstDownReqInfo->filePath, 
                    ((TRUE == pstDownReqInfo->IsResumeDown) ? FALSE : TRUE));
        if (ADM_IS_ERR(iRet))
        {
            i++;
            log_e("Failed to check/create file, filePath[%s], err[%d], download RetriesCnt[%d]", 
                  pstDownReqInfo->filePath, iRet, i);
            continue;
        }

        //!< 下载文件
        iRet = __adm_dpo_doOCSDownload(pstDownReqInfo);
        if (ADM_IS_ERR(iRet))
        {
            i++;
            log_e("Failed to download file, err[%d], download RetriesCnt[%d]", iRet, i);

            //返回下载暂停结果时,不需要进行下载重试
            if(iRet == ADM_ECODE_NET_CHG_PAUSE_DOWNLOAD || iRet == ADM_ECODE_DOWN_COND_ABNORMAL_PAUSE_DOWNLOAD)
            {
                break;
            }

            //报错信息由网络问题导致。
            if((iRet == ADM_ECODE_NET_CAUSE_DOWN_FAILED))
            {
                if (TRUE == pstDownReqInfo->IsUnlimitretry && 0 == pstDownReqInfo->networkRetriesCnt)
                {
                    /**无限次下载重试*/
                    pstDownReqInfo->downRetriesCnt = i;
                }
                else
                {
                    /**网络原因导致的失败，重试networkRetriesCnt次*/
                    if (iNetworkRetriesCnt < pstDownReqInfo->networkRetriesCnt)
                    {
                        pstDownReqInfo->downRetriesCnt = i;
                    }
                }
                iNetworkRetriesCnt++;
            }

            if (iRet == ADM_ECODE_PAUSE_IDEL_TRANSFER)
            {
                if (ADM_DPO_TRANSFER == pstDownReqInfo->eType)
                {
                    pstDownReqInfo->downRetriesCnt = i;
                }
                else 
                {
                    log_w("Do not find transfer mode");
                }
            }

            continue;
        }
        //下载结束恢复下载失败重试次数 mod by wl 20220331
        pstDownReqInfo->downRetriesCnt = iSaveRetriesCnt;

        //! 校验下载的文件信息
        iRet = adm_dpo_checkDownloadFile(pstDownReqInfo);
        if (ADM_IS_ERR(iRet))
        {
            i++;
            (void)adm_fremove(pstDownReqInfo->filePath);
            log_e("Failed to check download file[%s], err[%d], download RetriesCnt[%d]", pstDownReqInfo->filePath, iRet, i);
            continue;
        }
        
        //!< 成功下载则退出for循环
        break;
    }

    // TODO: 缺少暂停/取消下载错误码判断
    if (ADM_IS_ERR(iRet))
    {
        log_e("Failed to download[url(%s)-fileSize(%lld)]", pstDownReqInfo->url, pstDownReqInfo->fileSize);
    }
    
    return iRet;
}

D32S adm_dpo_switchIdelTransType(D32S iSwitchType)
{
    D32S iRet = ADM_ECODE_SUCCESSED;

    do
    {
        pthread_mutex_lock(&g_stIdelTransState.mutex);

        if (ADM_COTINUE_TRANS < iSwitchType || ADM_PAUSE_TRANS > iSwitchType || 
            iSwitchType == (D32S)g_stIdelTransState.eOperation)
        {
            log_i("Do not handle switch signal.[%d]", iSwitchType);
            break;
        }

        g_stIdelTransState.eOperation = (ADM_DPO_OPERATE_IDEL_TRANS_E)iSwitchType;
        pthread_mutex_unlock(&g_stIdelTransState.mutex);
    } while (FALSE);

    return iRet;
}


/*!
 *******************************************************************************
 * 执行上传文件.
 *
 * \note 同步接口
 *
 * \param[in]   pstUploadReqInfo  待上传请求信息
 * \retval      D32S       0: 成功 non-0：失败
 *******************************************************************************
 */
D32S adm_dpo_doUploadFile
(
    ADM_DPO_UPLOAD_REQPARAM_T *pstUploadReqInfo
)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    D32S i = 0;

    iRet = __adm_dpo_checkUploadParam(pstUploadReqInfo);
    if (ADM_IS_ERR(iRet))
    {
        return iRet;
    }

    for (i = 0; i < pstUploadReqInfo->uploadRetriesCnt + 1; i++)
    {
        (void)sleep((D32U)i * (D32U)ADM_DPO_DOWNLOAD_INTERNAL_RETRY_TIME);

        //!< 上传文件
        iRet = __adm_dpo_doOCSUploadFile(pstUploadReqInfo);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to upload file, err[%d], upload RetriesCnt[%d]", 
                    iRet, i + 1);
            continue;
        }

        //!< 成功下载则退出for循环
        break;
    }

    // TODO: 缺少暂停/取消上传错误码判断
    if (ADM_IS_ERR(iRet))
    {
        log_e("Failed to upload[url(%s)-filePath(%s)]", 
                pstUploadReqInfo->url, pstUploadReqInfo->filePath);
    }
    
    return iRet;
}


