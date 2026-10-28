/**
 * @file adm_oca_httpFileDownload.c
 * @author mengwei@abupdate.com
 * @brief 向平台请求文件下载
 * @version 0.1
 * @date 2021-05-15
 * 
 * @copyright Copyright © 2018 All rights 上海艾拉比智能科技有限公司
 * 
 */
#include "adm_oca_httpErrorCode.h"
#include "adm_oca_httpFile.h"
#include "adm_memory.h"
#include "adm_string.h"
#include "adm_error.h"
#include "adm_file.h"
#include "adm_cfg_api.h"
#include "adm_cfg_default.h"
#include "adm_oca_httpCommon.h"

static ADM_OCA_DOWNLOAD_ACTION_TYPE_E eActionType = ADM_OCA_DOWNLOAD_ACTION_TYPE_NORMAL;
static CURL* g_curlHandle = NULL;

#if 0
D32S adm_oca_pauseDownload(void)
{
    D32S iRet = ADM_OCA_HTTP_ECODE_FAILED;
    CURL* pCurlHandler = NULL;

    pCurlHandler = adm_oca_getCurlHandler();
    if (NULL == pCurlHandler) {
        iRet = ADM_OCA_HTTP_ECODE_CURL_INIT_FAILED;
        log_i("adm_oca_getCurlHandler failed, reason code:%d", iRet);
    } else {
        iRet = curl_easy_pause(pCurlHandler, CURLPAUSE_ALL);
        if (CURLE_OK != iRet) {
            log_i("curl_easy_pause failed, reason code:%d", iRet);
        } else {
            iRet = ADM_OCA_HTTP_ECODE_SUCCESSED;
            log_i("curl_easy_pause successed");
        }
    }

    return iRet;
}

D32S adm_oca_resumeDownload(void)
{
    D32S iRet = ADM_OCA_HTTP_ECODE_FAILED;
    CURL* pCurlHandler = NULL;

    pCurlHandler = adm_oca_getCurlHandler();
    if (NULL == pCurlHandler) {
        iRet = ADM_OCA_HTTP_ECODE_CURL_INIT_FAILED;
        log_i("adm_oca_getCurlHandler failed, reason code:%d", iRet);
    } else {
        iRet = curl_easy_pause(pCurlHandler, CURLPAUSE_CONT);
        if (CURLE_OK != iRet) {
            log_i("curl_easy_pause CURLPAUSE_CONT failed, reason code:%d", iRet);
        } else {
            iRet = ADM_OCA_HTTP_ECODE_SUCCESSED;
            log_i("curl_easy_pause CURLPAUSE_CONT successed");
        }
    }

    return iRet;
}
#endif

D32S adm_oca_interuptDownload(const D32S actionType)
{
    log_i("adm_oca_interuptDownload successed actionType[%d]",actionType);
    eActionType = (ADM_OCA_DOWNLOAD_ACTION_TYPE_E)actionType;
    return ADM_OCA_HTTP_ECODE_SUCCESSED;
}
/**
D32S adm_oca_pauseDownload(void)
{
    log_i("adm_oca_pauseDownload successed");
    eActionType = ADM_OCA_DOWNLOAD_ACTION_TYPE_PAUSE;
    return ADM_OCA_HTTP_ECODE_SUCCESSED;
}

D32S adm_oca_cancelDownload(void)
{
    log_i("adm_oca_cancelDownload successed");
    eActionType = ADM_OCA_DOWNLOAD_ACTION_TYPE_CANCEL;
    return ADM_OCA_HTTP_ECODE_SUCCESSED;
}
D32S adm_oca_network_chg_pause_download(void)
{
    log_i("adm_oca_network_chg_pause_download successed");
    eActionType = ADM_OCA_DOWNLOAD_ACTION_TYPE_NET_CHG_PAUSE;
    return ADM_OCA_HTTP_ECODE_SUCCESSED;
}
*/

//ADM_OCA_FILE_PROGRESS_CALLBACK_F progressCallback;

// static DCHAR* adm_oca_getDownloadCert(void)
// {
//     DCHAR* certFile = NULL;

//     // certFile = ADM_OCA_HTTPS_SERVER_ROOT_CERT;

//     return certFile;
// }

static size_t fileWriteCallback(char *buffer, size_t size, size_t nitems, void *outstream)  
{  
    return fwrite(buffer, size, nitems, outstream);  
//    size_t wrote_size = 0;
//    long wrote_bytes = 0;
//    
//    wrote_size = fwrite(buffer, size, nitems, outstream);
//    wrote_bytes = (wrote_size == nitems) ? (size * nitems) : 0;
//    
//    return wrote_bytes;
}

static D32S adm_oca_setDownloadRange(CURL* curlHandler, const D64S startPosition, const D64S endPosition)
{
    // log_i("Download continue startPosition:%lld", startPosition);
    // if (curl_easy_setopt(curl, CURLOPT_RESUME_FROM_LARGE, (startPosition>0)?startPosition:0)) {
    //     log_i("curl_easy_setopt CURLOPT_RANGE failed");
    //     return ADM_OCA_HTTP_ECODE_CURL_SET_ERR;
    // }
    DCHAR range[128] = {0};

   (void)adm_memset((void*)range, 0x00, sizeof(range), sizeof(range) * sizeof(DCHAR));
//    if (endPosition <= startPosition) {
//        (void)adm_snprintf(range, 128, "%lld-", startPosition);
//        log_w( "endPosition:%lld <= startPosition:%lld, need to download the entire file", endPosition, startPosition);
//    } else {
//        (void)adm_snprintf(range, 128, "%lld-%lld", startPosition, endPosition);
//    }
    
    (void)adm_snprintf(range, 128, "%lld-", startPosition);
    log_i("curl_easy_setopt set download range:%s", range);

    if (CURLE_OK != curl_easy_setopt(curlHandler, CURLOPT_RANGE, range)) {
        log_i("curl_easy_setopt CURLOPT_RANGE failed");
        return ADM_OCA_HTTP_ECODE_CURL_SET_ERR;
    }

    return ADM_OCA_HTTP_ECODE_SUCCESSED;
}

/**
 * @brief 请求下载的信息空间申请
 * 
 * @return ADM_OCA_FILE_DOWNLOAD_INFO_T1*    NULL:failed, Non-NULL:successed
 */
ADM_OCA_FILE_DOWNLOAD_INFO_T1* adm_oca_newDownloadInfo(void)
{
    ADM_OCA_FILE_DOWNLOAD_INFO_T1* pstDownloadInfo = NULL;

    pstDownloadInfo = adm_malloc(sizeof(ADM_OCA_FILE_DOWNLOAD_INFO_T1));
    if (NULL != pstDownloadInfo) {
        (void)adm_memset(pstDownloadInfo, 0x00, sizeof(ADM_OCA_FILE_DOWNLOAD_INFO_T1), sizeof(ADM_OCA_FILE_DOWNLOAD_INFO_T1) * sizeof(DCHAR*));
    } else {
        log_e( "adm_oca_newDownloadInfo failed");
    }

    return pstDownloadInfo;
}


/**
 * @brief 请求下载的信息空间释放
 * 
 * @param pstDownloadInfo   ADM_OCA_FILE_DOWNLOAD_INFO_T1*       使用adm_oca_newDownloadInfo申请的空间
 *   
 */
void adm_oca_delDownloadInfo(ADM_OCA_FILE_DOWNLOAD_INFO_T1* pstDownloadInfo)
{
    if (NULL != pstDownloadInfo) {

        adm_free(pstDownloadInfo);
    }
}

typedef struct {
    CURL* curl;
    ADM_OCA_FILE_PROGRESS_CALLBACK_F cbDownloadProgress;
    void *priv;
} MyProgress;

/**
 * @brief 升级进度处理接口
 * 
 * @param clientp       [IN]    void*       保留参数
 * @param dltotal       [IN]    curl_off_t  下载的总大小
 * @param dlnow         [IN]    curl_off_t  已下载的大小
 * @param ultotal       [IN]    curl_off_t  上传的总大小
 * @param ulnow         [IN]    curl_off_t  已上传的大小
 * @return curl_off_t   0：successed，1：异常
 */
static curl_off_t xferinfo(void *clientp,
                curl_off_t dltotal,
                curl_off_t dlnow,
                curl_off_t ultotal,
                curl_off_t ulnow)
{
    MyProgress* prog = (MyProgress*)clientp;
    ADM_OCA_FILE_PROGRESS_T progress;
    double speed = 0.00;
    double remainTime = 0.00;

    if (NULL != prog->curl) {
        /* curl_get_info必须在curl_easy_perform之后调用 */
        (void)curl_easy_getinfo(prog->curl, CURLINFO_SPEED_DOWNLOAD, &speed);
    }

    curl_off_t tmp = dltotal - dlnow;
    if (speed > 0.000001) {
        remainTime = (double)tmp / speed;
    }

    progress.remainTime = remainTime;
    progress.savedSize = (D64S)dlnow;
    progress.speed = speed;
    if (NULL != prog->cbDownloadProgress) {
        prog->cbDownloadProgress(&progress, prog->priv);
    }

    if (ADM_OCA_DOWNLOAD_ACTION_TYPE_CANCEL == eActionType || ADM_OCA_DOWNLOAD_ACTION_TYPE_PAUSE == eActionType || ADM_OCA_DOWNLOAD_ACTION_TYPE_NET_CHG_PAUSE == eActionType || ADM_OCA_DOWNLOAD_ACTION_TYPE_COND_ABNORMAL_PAUSE == eActionType || ADM_OCA_DOWNLOAD_ACTION_TYPE_PAUSE_IDEL_TRANSFER == eActionType)
    {
        return 1;
    }

    return 0;
}

/**
 * @brief 文件下载
 * @note 同步接口，此接口用于从平台下载文件
 * @warning 下载传入的文件路径为绝对路径，且路径目录存在，并需要执行可读写
 * 
 * @param connectHandler    [IN] const ADM_OCA_COMMON_INIT_HANDLER_T*  此次通信使用的句柄
 * @param downloadInfo      [IN] ADM_OCA_FILE_DOWNLOAD_INFO_T1*      从平台下载文件的数据
 * @param bIsCloudDownload    [IN] DBOOL  是否从云端下载
 * @return D32S             0:successed, non-0:failed
 */
D32S adm_oca_startDownload(const ADM_OCA_COMMON_INIT_HANDLER_T* connectHandler, ADM_OCA_FILE_DOWNLOAD_INFO_T1* pstDownloadInfo, 
DBOOL bIsCloudDownload)

{
    D32S iRet = ADM_OCA_HTTP_ECODE_FAILED;
    FILE* pDownloadFp = NULL;
    MyProgress prog = {0};
    //CURL* curlHandle = NULL;
    DSIZE httpCode = (DSIZE)0;

    eActionType = ADM_OCA_DOWNLOAD_ACTION_TYPE_NORMAL;

    if (NULL == pstDownloadInfo) {
        iRet = ADM_OCA_HTTP_ECODE_INVALID_POINTER;
        log_i("Parameter (downloadInfo) verification failed, pointer invalid, reason code:%d", iRet);
        return iRet;
    }

    if (0 > pstDownloadInfo->startPosition || 0 > pstDownloadInfo->endPosition) {
        iRet = ADM_OCA_HTTP_ECODE_RANGE_ERROR;
        log_i("Parameter downloadInfo (startPosition %lld | endPosition %lld) verification failed, reason code:%d", pstDownloadInfo->startPosition, pstDownloadInfo->endPosition, iRet);
        return iRet;
    } else {
        log_i("Download position start:%lld - end:%lld", pstDownloadInfo->startPosition, pstDownloadInfo->endPosition);
    }

    do {
        iRet = adm_fopen(pstDownloadInfo->savePath, "ab+", (void*)&pDownloadFp);
        if (ADM_OCA_HTTP_ECODE_SUCCESSED != iRet) {
            log_i("adm_fopen %s failed, reason code:%d", pstDownloadInfo->savePath, iRet);
            iRet = ADM_OCA_HTTP_ECODE_OPEN_FILE_FAILED;
            break;
        } else {
            if (NULL == pDownloadFp) {
                iRet = ADM_OCA_HTTP_ECODE_OPEN_FILE_FAILED;
                log_i("adm_fopen successed, but file handler is NULL, reason code:%d", iRet);
                break;
            } else {
                /** Note：把文件指针偏移到开始下载的位置 */
                log_i("File pointer offset to %lld", pstDownloadInfo->startPosition);
                if (0 != adm_fseek(pDownloadFp, (D32U)pstDownloadInfo->startPosition, (E_ADM_FILE_SEEK_T)SEEK_SET)) {
                    iRet = ADM_OCA_HTTP_ECODE_FP_SEEK_FAILED;
                    break;
                }
            }
        }

        // curlHandle = curl_easy_init();
        iRet = adm_oca_httpCurlInit(&g_curlHandle);
        if (NULL == g_curlHandle) {
            iRet = ADM_OCA_HTTP_ECODE_CURL_INIT_FAILED;
            log_e( "curl_easy_init failed, reason of error code is %d", iRet);
            break;
        }

        iRet = adm_oca_setDownloadRange(g_curlHandle, pstDownloadInfo->startPosition, pstDownloadInfo->endPosition);
        if (ADM_OCA_HTTP_ECODE_SUCCESSED != iRet) {
            log_e( "adm_oca_setDownloadRange failed, reason of error code is %d", iRet);
            break;
        }

        iRet = adm_oca_httpReqSetCurlOptDefault(g_curlHandle, connectHandler->address, fileWriteCallback, (void*)pDownloadFp, NULL);
        if (ADM_OCA_HTTP_ECODE_SUCCESSED != iRet) {
            log_e( "adm_oca_httpReqSetCurlOptDefault failed, reason of error code is %d", iRet);
            break;
        }

        /** Set the authentication method for the requesting platform */
        iRet = adm_oca_httpReqAuthInit(connectHandler->eConnectProtocolType, connectHandler->serverRootCert, connectHandler->clientCert, connectHandler->clientPrivateKey, g_curlHandle, bIsCloudDownload);
        if (ADM_OCA_HTTP_ECODE_SUCCESSED != iRet) {
            log_e("adm_oca_httpReqAuthInit error, reason code: %d", iRet);
            break;
        }

        iRet = adm_oca_httpReqLimitTimeSpeedSet(g_curlHandle, 60, 1024);
        if (ADM_OCA_HTTP_ECODE_SUCCESSED != iRet) {
            log_w("adm_oca_httpReqLimitTimeSpeedSet error, reason code: %d", iRet);
            break;
        }

        prog.curl = g_curlHandle;
        prog.priv = pstDownloadInfo->priv;
        prog.cbDownloadProgress = pstDownloadInfo->cbDownloadProgress;
        if (CURLE_OK != curl_easy_setopt(g_curlHandle, CURLOPT_XFERINFOFUNCTION, xferinfo)) {
            iRet = ADM_OCA_HTTP_ECODE_CURL_SET_ERR;
            log_w("curl_easy_setopt CURLOPT_XFERINFOFUNCTION error, [err:%d]", iRet);
            break;
        }

        if (CURLE_OK != curl_easy_setopt(g_curlHandle, CURLOPT_XFERINFODATA, &prog)) {
            iRet = ADM_OCA_HTTP_ECODE_CURL_SET_ERR;
            log_w("curl_easy_setopt CURL_LOW_SPEED_LIMIT error, [err:%d]", iRet);
            break;
        }

        if (CURLE_OK != curl_easy_setopt(g_curlHandle, CURLOPT_NOPROGRESS, FALSE)) {
            iRet = ADM_OCA_HTTP_ECODE_CURL_SET_ERR;
            log_w("curl_easy_setopt CURLOPT_NOPROGRESS error, [err:%d]", iRet);
            break;
        }

        /**设置网络代理服务*/
        DBOOL isHttpProxy = FALSE;
        isHttpProxy = adm_cfg_iniParserGetBoolean(ADM_CFG_VUS_IS_HTTP_PROXY, ADM_CFG_VUS_IS_HTTP_PROXY_DEFAULT_VAL);
        if (DTRUE == isHttpProxy && DTRUE == bIsCloudDownload)
        {
            const DCHAR *pHttpProxyIp = adm_cfg_iniParserGetString(ADM_CFG_VUS_IS_HTTP_PROXY_IP, ADM_CFG_VUS_IS_HTTP_PROXY_IP_DEFAULT_VAL);

            if (CURLE_OK != curl_easy_setopt(g_curlHandle, CURLOPT_PROXY, pHttpProxyIp)) {
                iRet = ADM_OCA_HTTP_ECODE_CURL_SET_ERR;
                log_w("curl_easy_setopt CURLOPT_NOPROGRESS error, [err:%d]", iRet);
                break;
            }

            if (CURLE_OK != curl_easy_setopt(g_curlHandle, CURLOPT_PROXYTYPE, CURLPROXY_HTTP)) {
                iRet = ADM_OCA_HTTP_ECODE_CURL_SET_ERR;
                log_w("curl_easy_setopt CURLOPT_NOPROGRESS error, [err:%d]", iRet);
                break;
            }
        }
        
        iRet = adm_oca_httpReqReallyPerform(g_curlHandle, &httpCode);
        if (ADM_OCA_HTTP_ECODE_SUCCESSED != iRet) {
            log_e("adm_oca_startDownload failed, reason code:%d", iRet);
        } else {
            log_i("adm_oca_startDownload successed, reason code:%d", iRet);
        }

        if (CURLE_ABORTED_BY_CALLBACK == (CURLcode)httpCode) 
        {
            if (ADM_OCA_DOWNLOAD_ACTION_TYPE_CANCEL == eActionType) 
            {
                iRet = ADM_OCA_HTTP_ECODE_CANCEL_DOWNLOAD;
            }
            else if (ADM_OCA_DOWNLOAD_ACTION_TYPE_PAUSE == eActionType) 
            {
                iRet = ADM_OCA_HTTP_ECODE_PAUSE_DOWNLOAD;
            }
            else if(ADM_OCA_DOWNLOAD_ACTION_TYPE_NET_CHG_PAUSE == eActionType)
            {
                iRet = ADM_OCA_HTTP_ECODE_NET_CHG_PAUSE_DOWNLOAD;
            }
            else if(ADM_OCA_DOWNLOAD_ACTION_TYPE_PAUSE_IDEL_TRANSFER == eActionType) 
            {
                iRet = ADM_OCA_HTTP_ECODE_PAUSE_IDEL_TRANSFER;
            }
            else if(ADM_OCA_DOWNLOAD_ACTION_TYPE_COND_ABNORMAL_PAUSE == eActionType)
            {
                iRet = ADM_OCA_HTTP_ECODE_COND_ABNORMAL_PAUSE_DOWNLOAD;
            }
            else
            {
                log_i("ADM_OCA_DOWNLOAD_ACTION_TYPE_NORMAL/ADM_OCA_DOWNLOAD_ACTION_TYPE_INVALID");
            }
        }

    } while(FALSE);

    if (NULL != pDownloadFp) {
        (void)adm_fclose((void*)&pDownloadFp);
    }

    adm_oca_httpCurlDeinit(g_curlHandle);
    
    return iRet;
}
