#include "adm_oca_httpErrorCode.h"
#include "adm_oca_httpCommon.h"
#include "adm_memory.h"
#include "adm_string.h"
#include "curl/curl.h"
#include "adm_cfg_api.h"
#include "adm_cfg_default.h"
#include "adm_ems_serverType.h"
#include "adm_pub_keylog.h"

static FILE *pCurlFileFp = NULL;
static const DCHAR* pCurlDir = NULL;

#ifdef SUPPORT_LION_PKI
static DCHAR g_clientID[128] = {};
SC_HANDLE handle_sc = NULL;
const char userPin[] = "abupForLion2024";        ///< 小于16位按照品牌不同自定义
DCHAR* g_pSnCode = NULL;
#endif
// static D32S g_iCountryCode = 0; 
// /**巴西 PKI*/
// #define BRAZIL_PKI_URL1 "https://pkipgw-sit-brbiz.cherystack.com:6443"
// #define BRAZIL_PKI_URL2 "https://pkipgw.omodajaecoo.com.br:6443"
// /**墨西哥 PKI*/
// #define MEXICO_PKI_URL1 "https://pkipgw-sit-mexbiz.cherystack.com:6443"
// #define MEXICO_PKI_URL2 "https://pkipgw.chirey.mx:6443"
// /**东盟 PKI*/
// #define ASEAN_PKI_URL1 "https://pkipgw-uat.cheryinternational.com:6443/th" 
// #define ASEAN_PKI_URL2 "https://pkipgw-uat.cheryinternational.com:6443/th"//待后续确认下来后更新

// /**巴西 租户id*/
// #define BR_TENANT_ID "X-Tenant-ID: brbiz"
// /**墨西哥 租户id*/
// #define MEX_TENANT_ID "X-Tenant-ID: mexbiz"
// /**东盟 新加坡 租户id*/
// #define SGP_TENANT_ID "X-Tenant-ID: sgpbiz"
// /**东盟 马来西亚 租户id*/
// #define MLS_TENANT_ID "X-Tenant-ID: mlsbiz"
// /**东盟 泰国 租户id*/
// #define THAI_TENANT_ID "X-Tenant-ID: thaibiz"
// /**东盟 印度尼西亚 租户id*/
// #define INDS_TENANT_ID "X-Tenant-ID: indsbiz"


// DCHAR *g_pPKIUrl1 = NULL; /**pki地址1*/
// DCHAR *g_pPKIUrl2 = NULL; /**pki地址2 用于环境切换*/
// DCHAR *g_pTenantInfo = NULL; /**租户信息*/

// typedef struct 
// {
//     ADM_COUNTRY_E eCountryCode[32];         ///< 国家码
//     ADM_BRAND_E  eBrandCode[10];            ///< 品牌码数组
//     const DCHAR* pTestPkiUrl;               ///< PKI测试环境URL
//     const DCHAR* pProductPkiUrl;            ///< PKI生产环境URL
//     const DCHAR* pTenantInfo;               ///< 租户信息
// } ADM_OCA_COUNTRY_PKI_INFO_T;

// static ADM_OCA_COUNTRY_PKI_INFO_T g_aPkiInfo[] = 
// {
//     /**巴西*/
//     {{COUNTRY_BR}, {OMODA_BRAND,JAECOO_BRAND}, BRAZIL_PKI_URL1, BRAZIL_PKI_URL2, BR_TENANT_ID},
//     /** 东盟新加坡 */
//     {{COUNTRY_SG}, {CHERY_BRAND,OMODA_BRAND,JAECOO_BRAND,EXLANTIX_BRAND},ASEAN_PKI_URL1, ASEAN_PKI_URL2, SGP_TENANT_ID},
// 	/** 东盟马来西亚 */
//     {{COUNTRY_MY}, {CHERY_BRAND,OMODA_BRAND,JAECOO_BRAND,EXLANTIX_BRAND},ASEAN_PKI_URL1, ASEAN_PKI_URL2, MLS_TENANT_ID},
//     /** 东盟泰国 */
//     {{COUNTRY_TH}, {CHERY_BRAND,OMODA_BRAND,JAECOO_BRAND,EXLANTIX_BRAND},ASEAN_PKI_URL1, ASEAN_PKI_URL2, THAI_TENANT_ID},
// 	/** 东盟印度尼西亚 */
//     {{COUNTRY_ID}, {CHERY_BRAND,OMODA_BRAND,JAECOO_BRAND,EXLANTIX_BRAND},ASEAN_PKI_URL1, ASEAN_PKI_URL2, INDS_TENANT_ID},	
//     /** 墨西哥*/
//     {{COUNTRY_MX}, {CHERY_BRAND,OMODA_BRAND,JAECOO_BRAND,EXEED_BRAND}, MEXICO_PKI_URL1, MEXICO_PKI_URL2, MEX_TENANT_ID}
// };

// static D32S ocaGetPkiUrl(ADM_COUNTRY_E eCountryCode, ADM_BRAND_E eBrandCode, DCHAR** ppTestPkiUrl, DCHAR** ppProductPkiUrl)
// {
//     D32S iRet = ADM_OCA_HTTP_ECODE_FAILED;
//     D32U i = 0;
//     D32U j = 0;
//     D32U k = 0;

//     log_i("Current coutry code:%d, Brand code:%d, to get PKI url.", (D32S)eCountryCode, (D32S)eBrandCode);

//     if (NULL == ppTestPkiUrl || NULL == ppProductPkiUrl)
//     {
//         log_e("Invalid param.");
//         return ADM_OCA_HTTP_ECODE_INVALID_PARAM;
//     }

//     if (eCountryCode < COUNTRY_CN || eCountryCode >= COUNTRY_DEFAULT ||  eBrandCode < CHERY_BRAND || eBrandCode >= DEFAULT_BRAND)
//     {
//         log_e("Invalid param.");
//         return ADM_OCA_HTTP_ECODE_INVALID_PARAM;
//     }

//     for (i = 0; i < sizeof(g_aPkiInfo)/sizeof(g_aPkiInfo[0]); i++)
//     {
//         for (j = 0; j < sizeof(g_aPkiInfo[i].eCountryCode)/sizeof(g_aPkiInfo[i].eCountryCode[0]); j++)
//         {
//             if (eCountryCode == g_aPkiInfo[i].eCountryCode[j])
//             {
//                 for (k = 0; k < sizeof(g_aPkiInfo[i].eBrandCode)/sizeof(g_aPkiInfo[i].eBrandCode[0]); k++)
//                 {
//                     if (eBrandCode == g_aPkiInfo[i].eBrandCode[k])
//                     {
//                         /**pki URL地址*/
//                         *ppTestPkiUrl = adm_strdup(g_aPkiInfo[i].pTestPkiUrl);
//                         *ppProductPkiUrl = adm_strdup(g_aPkiInfo[i].pProductPkiUrl);
//                         /**租户信息*/
//                         g_pTenantInfo = adm_strdup(g_aPkiInfo[i].pTenantInfo);  
//                         return ADM_OCA_HTTP_ECODE_SUCCESSED;
//                     }
//                 }
//             }
//         }
//     }

//     return iRet;
// }

/**
 * @brief 将HTTP平台响应的错误码转化为本地的错误码
 * 
 * @param curlCode  [IN]    const CURLcode      HTTP平台响应的状态码
 * @return D32S     0:successed， non-0:failed 
 */
D32S adm_oca_httpReqCurlCodeTransfer(const CURLcode curlCode)
{
    D32S ret = ADM_OCA_HTTP_ECODE_FAILED;

    switch(curlCode)
    {
    case CURLE_OK:
        ret = ADM_OCA_HTTP_ECODE_SUCCESSED;
        break;
    case CURLE_FAILED_INIT:
        ret = ADM_OCA_HTTP_ECODE_NO_MEMORY;
        break;
    case CURLE_URL_MALFORMAT:
        ret = ADM_OCA_HTTP_ECODE_URL_MALFORMAT;
        break;
    case CURLE_COULDNT_RESOLVE_HOST:
        ret = ADM_OCA_HTTP_ECODE_COULDNT_RESOLVE_HOST;
        break;
    case CURLE_COULDNT_CONNECT:
        ret = ADM_OCA_HTTP_ECODE_COULDNT_CONNECT;
        break;
    case CURLE_HTTP_RETURNED_ERROR:
        ret = ADM_OCA_HTTP_ECODE_HTTP_RETURNED_ERROR;
        break;
    case CURLE_WRITE_ERROR:
        ret = ADM_OCA_HTTP_ECODE_WRITE_FILE_FAIL;
        break;
    case CURLE_UPLOAD_FAILED:
        ret = ADM_OCA_HTTP_ECODE_UPLOAD_FAILED;
        break;
    case CURLE_READ_ERROR:
        ret = ADM_OCA_HTTP_ECODE_READ_FILE_FAIL;
        break;
    case CURLE_OUT_OF_MEMORY:
        ret = ADM_OCA_HTTP_ECODE_NO_MEMORY;
        break;
    case CURLE_OPERATION_TIMEDOUT:
        ret = ADM_OCA_HTTP_ECODE_OPERATION_TIMEDOUT;
        break;
    case CURLE_RANGE_ERROR:
        ret = ADM_OCA_HTTP_ECODE_RANGE_ERROR;
        break;
    case CURLE_HTTP_POST_ERROR:
        ret = ADM_OCA_HTTP_ECODE_HTTP_POST_ERROR;
        break;
    case CURLE_SSL_CONNECT_ERROR:
        ret = ADM_OCA_HTTP_ECODE_SSL_CONNECT_ERROR;
        break;
    case CURLE_BAD_DOWNLOAD_RESUME:
        ret = ADM_OCA_HTTP_ECODE_BAD_DOWNLOAD_RESUME;
        break;
    case CURLE_ABORTED_BY_CALLBACK:
        ret = ADM_OCA_HTTP_ECODE_ABORTED_BY_CALLBACK;
        break;
    case CURLE_TOO_MANY_REDIRECTS:
        ret = ADM_OCA_HTTP_ECODE_TOO_MANY_REDIRECTS;
        break;
    // case CURLE_PEER_FAILED_VERIFICATION:
    //     ret = ADM_OCA_HTTP_ECODE_PEER_FAILED_VERIFICATION;
    case CURLE_GOT_NOTHING:
        ret = ADM_OCA_HTTP_ECODE_GOT_NOTHING;
        break;
    case CURLE_SSL_ENGINE_NOTFOUND:
        ret = ADM_OCA_HTTP_ECODE_SSL_ENGINE_NOTFOUND;
        break;
    case CURLE_SSL_ENGINE_SETFAILED:
        ret = ADM_OCA_HTTP_ECODE_SSL_ENGINE_SETFAILED;
        break;
    case CURLE_SEND_ERROR:
        ret = ADM_OCA_HTTP_ECODE_SEND_ERROR;
        break;
    case CURLE_RECV_ERROR:
        ret = ADM_OCA_HTTP_ECODE_RECV_ERROR;
        break;
    case CURLE_SSL_CERTPROBLEM:
        ret = ADM_OCA_HTTP_ECODE_SSL_CERTPROBLEM;
        break;
    case CURLE_SSL_CIPHER:
        ret = ADM_OCA_HTTP_ECODE_SSL_CIPHER;
        break;
    case CURLE_SSL_CACERT:
        ret = ADM_OCA_HTTP_ECODE_SSL_CACERT;
        break;
    case CURLE_BAD_CONTENT_ENCODING:
        ret = ADM_OCA_HTTP_ECODE_BAD_CONTENT_ENCODING;
        break;
    case CURLE_FILESIZE_EXCEEDED:
        ret = ADM_OCA_HTTP_ECODE_FILESIZE_EXCEEDED;
        break;
    case CURLE_SSL_ENGINE_INITFAILED:
        ret = ADM_OCA_HTTP_ECODE_SSL_ENGINE_INITFAILED;
        break;
    case CURLE_LOGIN_DENIED:
        ret = ADM_OCA_HTTP_ECODE_LOGIN_DENIED;
        break;
    case CURLE_REMOTE_DISK_FULL:
        ret = ADM_OCA_HTTP_ECODE_REMOTE_DISK_FULL;
        break;
    case CURLE_SSL_CACERT_BADFILE:
        ret = ADM_OCA_HTTP_ECODE_SSL_CACERT_BADFILE;
        break;
    case CURLE_REMOTE_FILE_NOT_FOUND:
        ret = ADM_OCA_HTTP_ECODE_REMOTE_FILE_NOT_FOUND;
        break;
    default:
        // TODO：错误码定义范围越界
        ret = ADM_OCA_HTTP_ECODE_NET_UNKNOWN;
        break;
    }
    return ret;
}

/**
 * @brief 最终使用libcurl发起HTTP请求
 * 
 * @param curlHandle    [IN]    CURL*     libcurl句柄
 * @param httpCode      [OUT]   DSIZE*           HTTP响应的状态码
 * @return D32S         0:successed， non-0:failed 
 */
D32S adm_oca_httpReqReallyPerform(CURL* curlHandle, DSIZE* httpCode)
{
    D32S iRet = ADM_OCA_HTTP_ECODE_FAILED;
    CURLcode curlCode = ADM_OCA_HTTP_ECODE_FAILED;
    long lTmpHttpCode = 0;

    curlCode = curl_easy_perform(curlHandle);
    log_i( "curl_easy_perform curlCode is %d", curlCode);
    iRet = adm_oca_httpReqCurlCodeTransfer(curlCode);
    if (NULL != pCurlFileFp)
    {
        fclose(pCurlFileFp);
        pCurlFileFp = NULL;
    }
    if (ADM_OCA_HTTP_ECODE_SUCCESSED != iRet) {
        log_e("curl_easy_perform error reason code %d iRet %d", curlCode, iRet);
        *httpCode = (DSIZE)curlCode;
    } else {
        if (NULL != httpCode) {
            /** DSIZE在安卓系统解释为int型，但是CURLINFO_RESPONSE_CODE要求为long,大小不一致导致踩内存*/
            if(curl_easy_getinfo(curlHandle, CURLINFO_RESPONSE_CODE, &lTmpHttpCode))
            {
                log_w("curl_easy_getinfo get httpCode error");
            }
            else
            {
                *httpCode = (DSIZE)lTmpHttpCode;
                log_i("curl_easy_getinfo successed httpCode = %d", *httpCode);
                // iRet = *httpCode;
            }
        }
    }

    return iRet;
}


/**
 * @brief HTTP请求设置超时限制，例如：配置60s内需要响应1024Bytes
 * 
 * @param curlHandle        CURL*     curl句柄
 * @param speedTime         const D32U      限制时间，单位：s
 * @param speedByte         const D32U      限制字节数，单位：Byte
 * @return D32S             0:successed， non-0:failed
 */
D32S adm_oca_httpReqLimitTimeSpeedSet(CURL* curlHandle, const D32U speedTime, const D32U speedByte)
{
    if (NULL == curlHandle) {
        log_e("Parameter (curlhandle) verification failed, pointer invalid");
        return ADM_OCA_HTTP_ECODE_INVALID_POINTER;
    }

    //!< CURLOPT_CONNECTTIMEOUT：连接对方主机时的最长等待时间，此设置限制的是建立连接过程的时间，其它过程的时间不在控制范围
    if (curl_easy_setopt(curlHandle, CURLOPT_CONNECTTIMEOUT, speedTime)) {
        return ADM_OCA_HTTP_ECODE_CURL_SET_ERR;
    }

    if(curl_easy_setopt(curlHandle, CURLOPT_LOW_SPEED_TIME, speedTime)) {
        log_w("curl_easy_setopt CURLOPT_LOW_SPEED_TIME %u error", speedTime);
        return ADM_OCA_HTTP_ECODE_CURL_SET_ERR;
    }

    if(curl_easy_setopt(curlHandle, CURLOPT_LOW_SPEED_LIMIT, speedByte)) {
        log_w("curl_easy_setopt CURL_LOW_SPEED_LIMIT %u error", speedByte);
        return ADM_OCA_HTTP_ECODE_CURL_SET_ERR;
    }

    return ADM_OCA_HTTP_ECODE_SUCCESSED;
}

/**
 * @brief HTTP请求设置出口网卡或IP
 * 
 * @param curlHandle[IN]    CURL*   curl句柄
 * @param outputApn[IN]     const DCHAR*    网卡名或IP地址
 * @return D32S     0:Successed, None-0:failed
 */
D32S adm_oca_httpReqSetOutputApn(CURL* curlHandle, const DCHAR* outputApn)
{
    if (NULL == curlHandle) {
        log_e("Parameter (curlhandle) verification failed, pointer invalid");
        return ADM_OCA_HTTP_ECODE_INVALID_POINTER;
    }

    if (NULL == outputApn) {
        log_e("Parameter (outputAPn) is nil");
        return ADM_OCA_HTTP_ECODE_INVALID_POINTER;
    }

    if (curl_easy_setopt(curlHandle, CURLOPT_INTERFACE, outputApn)) {
        log_e("curl_easy_setopt CURLOPT_INTERFACE %s error", outputApn);
        return ADM_OCA_HTTP_ECODE_CURL_SET_ERR;
    }

    return ADM_OCA_HTTP_ECODE_SUCCESSED;
}

#ifdef CLOUD_ENV_TEST
static D32S ocaSetCurlVerboseFile(CURL* curlHandle)
{
    D32S iRet = ADM_OCA_HTTP_ECODE_SUCCESSED;
    if (NULL != pCurlDir)
    {
        pCurlFileFp = fopen(pCurlDir, "a+");
        if (NULL == pCurlFileFp)
        {
            log_w("open curlDir[%s] error.", pCurlDir);
        }
        else 
        {
            long size = ftell(pCurlFileFp);
            if (size > 5*1024*1024)
            {
                log_i("size > 5M");
                fclose(pCurlFileFp);
                pCurlFileFp = NULL;
                remove(pCurlDir);

                pCurlFileFp = fopen(pCurlDir, "a+");
                if (NULL == pCurlFileFp)
                {
                    log_w("open curlDir[%s] error.", pCurlDir);
                    /** 返回成功,不影响任务 */
                    return iRet;
                }
            }

            if(curl_easy_setopt(curlHandle, CURLOPT_STDERR, pCurlFileFp)) 
            {
                log_w("curl_easy_setopt CURLOPT_STDERR error");
                return ADM_OCA_HTTP_ECODE_CURL_SET_ERR;
            }
        }
    }
    else 
    {
        log_w("Curl verbose file is null.");
    }

    return iRet;
}
#endif

/**
 * @brief 设置HTTP请求的CURL句柄的默认配置
 * 
 * @param curlHandle    [IN]    CURL*             curl句柄
 * @param url           [IN]    const DCHAR*            请求地址
 * @param writeCallback [IN]    curl_write_callback     响应数据的写入回调函数
 * @param data          [IN]    void*                   保留处理参数
 * @param messageBody   [IN]    const DCHAR*            请求数据
 * @return D32S         0:successed， non-0:failed 
 */
D32S adm_oca_httpReqSetCurlOptDefault(CURL* curlHandle, const DCHAR* url, curl_write_callback writeCallback, void *data, DCHAR *messageBody)
{
    D32U msgBodyLen = 0;
    D32S curlPort = 0;

    if (NULL == curlHandle) {
        log_e("Parameter (curlhandle) verification failed, pointer invalid");
        return ADM_OCA_HTTP_ECODE_INVALID_POINTER;
    }

    if (NULL == url) {
        log_e("Parameter (url) verification failed, pointer invalid");
        return ADM_OCA_HTTP_ECODE_INVALID_POINTER;
    }

    /** Set callback */
    if (curl_easy_setopt(curlHandle, CURLOPT_WRITEFUNCTION, writeCallback)) {
        log_w("curl_easy_setopt CURLOPT_WRITEFUNCTION writeCallback error");
        return ADM_OCA_HTTP_ECODE_CURL_SET_ERR;
    }

    if (NULL != data) {
        /** Set response out data */
        if (curl_easy_setopt(curlHandle, CURLOPT_WRITEDATA, (void*)data)) {
            log_w("curl_easy_setopt CURLOPT_WRITEDATA data error");
            return ADM_OCA_HTTP_ECODE_CURL_SET_ERR;
        }
    }

    if (NULL != messageBody) {
        msgBodyLen = adm_strlen(messageBody);
        if (curl_easy_setopt(curlHandle, CURLOPT_POSTFIELDSIZE, msgBodyLen)) {
            log_w("curl_easy_setopt CURLOPT_URL messageBodyLen:%d error", msgBodyLen);
            return ADM_OCA_HTTP_ECODE_CURL_SET_ERR;
        }

        if (curl_easy_setopt(curlHandle, CURLOPT_COPYPOSTFIELDS, messageBody)) {
            log_w("curl_easy_setopt CURLOPT_URL messageBody:%s error", messageBody);
            return ADM_OCA_HTTP_ECODE_CURL_SET_ERR;
        }
    }

        /* Set http request enable redirect following */
    if (curl_easy_setopt(curlHandle, CURLOPT_FOLLOWLOCATION, 1L)) {
        log_w("curl_easy_setopt CURLOPT_FOLLOWLOCATION 1 error");
        return ADM_OCA_HTTP_ECODE_CURL_SET_ERR;
    }
    /* Set http request allow three redirects */
    if(curl_easy_setopt(curlHandle, CURLOPT_MAXREDIRS, 3L)) {
        log_w("curl_easy_setopt CURLOPT_MAXREDIRS 3 error");
        return ADM_OCA_HTTP_ECODE_CURL_SET_ERR;
    }

    /** Set http request & response buffer 8KB */
    if(curl_easy_setopt(curlHandle, CURLOPT_BUFFERSIZE, 8192)) {
        log_w("curl_easy_setopt CURLOPT_BUFFERSIZE 8192 error");
        return ADM_OCA_HTTP_ECODE_CURL_SET_ERR;
    }

#ifdef CLOUD_ENV_TEST
    /** Set http request & response VERBOSE */
    if(curl_easy_setopt(curlHandle, CURLOPT_VERBOSE, 1L)) {
        log_w("curl_easy_setopt CURLOPT_VERBOSE 1 error");
        return ADM_OCA_HTTP_ECODE_CURL_SET_ERR;
    }

    (void)ocaSetCurlVerboseFile(curlHandle);
    
#else
    if(curl_easy_setopt(curlHandle, CURLOPT_VERBOSE, 0L/*1L*/)) {
        log_w("curl_easy_setopt CURLOPT_VERBOSE 1 error");
        return ADM_OCA_HTTP_ECODE_CURL_SET_ERR;
    }
#endif

    if (curl_easy_setopt(curlHandle, CURLOPT_URL, url)) {
        log_w("curl_easy_setopt CURLOPT_URL reqAddr:%s error", url);
        return ADM_OCA_HTTP_ECODE_CURL_SET_ERR;
    }

    curlPort = adm_cfg_iniParserGetInt(ADM_CFG_VUS_CURL_PORT, ADM_CFG_VUS_CURL_PORT_DEFAULT_VAL);

    log_d("curlPort:%d", curlPort);
    if (ADM_CFG_VUS_CURL_PORT_DEFAULT_VAL != curlPort)
    {
        log_d("set curlPort");
        /** 设置客户端使用的本地端口号范围的起始端口 */
        if (curl_easy_setopt(curlHandle, CURLOPT_LOCALPORT, curlPort))
        {
            log_w("curl_easy_setopt CURLOPT_LOCALPORT port:%d error", curlPort);
            return ADM_OCA_HTTP_ECODE_CURL_SET_ERR;
        }
    }

    /** 此接口并没有使用到SSL/TLS，但会不会是用到了signals导致的crash呢？官方建议在多线程场景下应该设置CURLOPT_NOSIGNAL选项，
     * 因为在解析DNS出现超时的时候将会发生“糟糕”的情况。官方也给出了解决方法，可以使用c-ares[2]的libcurl版本实现异步域名解析来预防这种“糟糕”的情况，
     * 但是最后一句还是告诫我们：在多线程场景下，若不设置CURLOPT_NOSIGNAL选项，可能会有“意外”的情况发生。通过官方这段描述，可以大致猜测到是没有设置这个选项造成的crash 
     * */
    if (curl_easy_setopt(curlHandle, CURLOPT_NOSIGNAL, TRUE)) {
        log_w("curl_easy_setopt CURLOPT_NOSIGNAL error");
        return ADM_OCA_HTTP_ECODE_CURL_SET_ERR;
    }

    //!< 这是返回CURLOPT_FAILONERROR设置为TRUE和HTTP服务器返回的错误代码是> = 400。
    if (curl_easy_setopt(curlHandle, CURLOPT_FAILONERROR, TRUE)) {
        log_w("curl_easy_setopt CURLOPT_FAILONERROR error");
        return ADM_OCA_HTTP_ECODE_CURL_SET_ERR;
    }

    return ADM_OCA_HTTP_ECODE_SUCCESSED;
}

// D32S ocaPkiHttpReqPostContentTypeSet(struct curl_slist **headers)
// {
// #ifdef SUPPORT_LION_PKI
//     if (NULL != g_pTenantInfo)
//     {
//         *headers = curl_slist_append(*headers, g_pTenantInfo);
//         if (NULL == *headers)
//         {
//             log_e("curl_slist_append headers (X-Tenant-ID:) error");
//             return ADM_OCA_HTTP_ECODE_CURL_SET_ERR;
//         }
//     }

//     *headers = curl_slist_append(*headers, "X-Business-Type:ota");
//     if (NULL == *headers)
//     {
//         log_e("curl_slist_append headers (X-Business-Type:) error");
//         return ADM_OCA_HTTP_ECODE_CURL_SET_ERR;
//     }
// #endif
// 
    // return ADM_OCA_HTTP_ECODE_SUCCESSED;
// }

// void ocaPrint_curl_slist(struct curl_slist **headers)
// {
//     log_i("HTTP Headers:");
//     log_i("-------------");

//     struct curl_slist *current = *headers;
//     int count = 1;

//     while (current != NULL)
//     {
//         log_i("%s", current->data);
//         current = current->next;
//         count++;
//     }

//     if (count == 1)
//     {
//         log_i("No headers found.\n");
//     }

//     return;
// }

/**
 * @brief 为HTTP POST请求设置headers
 * 
 * @param curlHandle    [IN]    CURL*               curl句柄
 * @param eContentType  [IN]    ADM_OCA_HTTP_POST_CONTENT_TYPE_E    post消息content类型
 * @param headers       [OUT]   struct curl_slist   设置后的headers结构
 * @return D32S 
 */
D32S adm_oca_httpReqPostContentTypeSet(CURL* curlHandle, ADM_OCA_HTTP_POST_CONTENT_TYPE_E eContentType, struct curl_slist **headers)
{
    if (NULL == curlHandle) {
        log_i("Parameter (curlhandle) verification failed, pointer invalid");
        return ADM_OCA_HTTP_ECODE_INVALID_POINTER;
    }

    if (curl_easy_setopt(curlHandle, CURLOPT_POST, 1L)) {
        log_w("curl_easy_setopt CURLOPT_POST 1 failed");
        return ADM_OCA_HTTP_ECODE_CURL_SET_ERR;
    }

    switch (eContentType)
    {
        case ADM_OCA_HTTP_POST_CONTENT_TYPE_AUTO:
            /* code */
            break;
        
        case ADM_OCA_HTTP_POST_CONTENT_TYPE_APPLICATION_X_WWW_FORM_URLENCODED:
            break;

        case ADM_OCA_HTTP_POST_CONTENT_TYPE_MULTIPART_FORM_DATA:
            log_i( "eContentType is %d, use Content-Type: multipart/form-data request platform", eContentType);
            *headers = curl_slist_append(*headers, "Content-Type: multipart/form-data");
            if (NULL == *headers) {
                log_e( "curl_slist_append headers (Content-Type: multipart/form-data) error");
                return ADM_OCA_HTTP_ECODE_CURL_SET_ERR;
            }
            break;
        
        case ADM_OCA_HTTP_POST_CONTENT_TYPE_APPLICATION_JSON:
            log_i("eContentType is %d, use application/json request platform", eContentType);
            *headers = curl_slist_append(*headers, "Content-Type: application/json");
            if(NULL == *headers)
            {
                log_e("curl_slist_append headers (Content-Type: application/json) error");
                return ADM_OCA_HTTP_ECODE_CURL_SET_ERR;
            }
            break;
    
        case ADM_OCA_HTTP_POST_CONTENT_TYPE_TEXT_PLAIN:
            log_i("eContentType is %d, use text/plain request platform", eContentType);
            *headers = curl_slist_append(*headers, "Content-Type: text/plain");
            if(NULL == *headers)
            {
                log_e("curl_slist_append headers (Content-Type: text/plain) error");
                return ADM_OCA_HTTP_ECODE_CURL_SET_ERR;
            }
            break;

        case ADM_OCA_HTTP_POST_CONTENT_TYPE_INVALID:
        default:
            log_w("unknown contentType:%d", eContentType);
            break;
    }

    // (void)ocaPkiHttpReqPostContentTypeSet(headers);

    *headers = curl_slist_append(*headers, "Expect:");
    if (NULL == *headers) {
        log_e( "curl_slist_append headers (Expect:) error");
        return ADM_OCA_HTTP_ECODE_CURL_SET_ERR;
    }

    if (NULL != *headers) {
        if (curl_easy_setopt(curlHandle, CURLOPT_HTTPHEADER, *headers)) {
            log_e("curl_easy_setopt headers error");
            return ADM_OCA_HTTP_ECODE_CURL_SET_ERR;
        }
    }

    // ocaPrint_curl_slist(headers);

    return ADM_OCA_HTTP_ECODE_SUCCESSED;
}


#if 0
CURLcode sslctx_function(CURL *curl, void *sslctx, void *parm)
{
    CURLcode rv = CURLE_ABORTED_BY_CALLBACK;

    /** This example uses two (fake) certificates **/
    DCHAR *mypem = (DCHAR*)parm;

    BIO *cbio = BIO_new_mem_buf(mypem, adm_strlen(mypem));
    X509_STORE  *cts = SSL_CTX_get_cert_store((SSL_CTX *)sslctx);
    D32S i;
    STACK_OF(X509_INFO) *inf;
    (void)curl;
    //(void)parm;

    if(!cts || !cbio)
    {
        return rv;
    }

    inf = PEM_X509_INFO_read_bio(cbio, NULL, NULL, NULL);

    if(!inf)
    {
        BIO_free(cbio);
        return rv;
    }

    for(i = 0; i < sk_X509_INFO_num(inf); i++)
    {
        X509_INFO *itmp = sk_X509_INFO_value(inf, i);
        if(itmp->x509)
        {
            X509_STORE_add_cert(cts, itmp->x509);
        }
        if(itmp->crl)
        {
            X509_STORE_add_crl(cts, itmp->crl);
        }
    }

    sk_X509_INFO_pop_free(inf, X509_INFO_free);
    BIO_free(cbio);

    rv = CURLE_OK;
    return rv;
}
#endif

static D32S adm_oca_httpNoAuthInit(CURL* curlHandle)
{
    D32S iRet = ADM_OCA_HTTP_ECODE_FAILED;

    do {
        //!< 参数校验
        if (NULL == curlHandle) {
            iRet = ADM_OCA_HTTP_ECODE_INVALID_PARAM;
            log_e("Input param curlHandle is ni, [err:%d]", iRet);
            break;
        }

        //!< 设置 CURLOPT_PROTOCOLS 
        if(curl_easy_setopt(curlHandle, CURLOPT_PROTOCOLS, CURLPROTO_HTTP)) {
            iRet = ADM_OCA_HTTP_ECODE_CURL_SET_ERR;
            log_e("curl_easy_setopt CURLOPT_PROTOCOLS CURLPROTO_HTTP failed, [err:%d]", iRet);
            break;
        }

        //!< 设置CURLOPT_SSL_VERIFYHOST 为0L
        if (curl_easy_setopt(curlHandle, CURLOPT_SSL_VERIFYHOST, 0L)) {
            iRet = ADM_OCA_HTTP_ECODE_CURL_SET_ERR;
            log_e("curl_easy_setopt CURLOPT_SSL_VERIFYHOST failed, [err:%d]", iRet);
            break;
        }

        iRet = ADM_OCA_HTTP_ECODE_SUCCESSED;
    } while(FALSE);

    return iRet;
}

static D32S adm_oca_httpReqOneAuthInit(CURL* curlHandle, const DCHAR* serverRootCert, DBOOL bIsCloudDownload)
{
    if (NULL == curlHandle) {
        log_e("Parameter (curlhandle) verification failed, pointer invalid");
        return ADM_OCA_HTTP_ECODE_INVALID_POINTER;
    }

    if (NULL == serverRootCert) {
        log_e("serverRootCert is nil");
        return ADM_OCA_HTTP_ECODE_INVALID_POINTER;
    }

    if(curl_easy_setopt(curlHandle, CURLOPT_PROTOCOLS, CURLPROTO_HTTPS)) {
        log_w("curl_easy_setopt CURLOPT_PROTOCOLS CURLPROTO_HTTPS error");
        return ADM_OCA_HTTP_ECODE_CURL_SET_ERR;
    }

    if(curl_easy_setopt(curlHandle, CURLOPT_SSL_VERIFYPEER, 1L))
    // if(curl_easy_setopt(curlHandle, CURLOPT_SSL_VERIFYPEER, 0L))
    {
        log_w("curl_easy_setopt CURLOPT_SSL_VERIFYPEER 1 error");
        return ADM_OCA_HTTP_ECODE_CURL_SET_ERR;
    }

    if(curl_easy_setopt(curlHandle, CURLOPT_SSL_VERIFYHOST, 0L))
    {
        log_w("curl_easy_setopt CURLOPT_SSL_VERIFYHOST 0 error");
        return ADM_OCA_HTTP_ECODE_CURL_SET_ERR;
    }

    log_i("Cert info server root cert path is %s", serverRootCert);
    if(curl_easy_setopt(curlHandle, CURLOPT_CAINFO, serverRootCert))
    {
        log_w("curl_easy_setopt CURLOPT_CAINFO %s error", serverRootCert);
        return ADM_OCA_HTTP_ECODE_SSL_SERVER_ROOT_CERT;
    }

    return ADM_OCA_HTTP_ECODE_SUCCESSED;
}

static D32S adm_oca_httpReqMulAuthInit(CURL* curlHandle, const DCHAR* serverRootCert, const DCHAR* clientCert, const DCHAR* clientPrivateKey)
{
    if (NULL == curlHandle) {
        log_e("Parameter (curlhandle) verification failed, pointer invalid");
        return ADM_OCA_HTTP_ECODE_INVALID_POINTER;
    }

    if (NULL == serverRootCert || NULL == clientCert || NULL == clientPrivateKey) {
        log_e("serverRootCert or clientCert or clientPrivateKey is nil");
        return ADM_OCA_HTTP_ECODE_INVALID_POINTER;
    }

    if(curl_easy_setopt(curlHandle, CURLOPT_PROTOCOLS, CURLPROTO_HTTPS)) {
        log_w("curl_easy_setopt CURLOPT_PROTOCOLS CURLPROTO_HTTPS error");
        return ADM_OCA_HTTP_ECODE_CURL_SET_ERR;
    }

    if(curl_easy_setopt(curlHandle, CURLOPT_SSL_VERIFYPEER, 1L)) {
        log_w("curl_easy_setopt CURLOPT_SSL_VERIFYPEER 1 error");
        return ADM_OCA_HTTP_ECODE_CURL_SET_ERR;
    }

#ifdef SUPPORT_LION_PKI
    if(curl_easy_setopt(curlHandle, CURLOPT_SSL_VERIFYHOST, 1L))
    {
        log_w("curl_easy_setopt CURLOPT_SSL_VERIFYHOST 0 error");
        return ADM_OCA_HTTP_ECODE_CURL_SET_ERR;
    }
#else
    if(curl_easy_setopt(curlHandle, CURLOPT_SSL_VERIFYHOST, 0L))
    {
        log_w("curl_easy_setopt CURLOPT_SSL_VERIFYHOST 0 error");
        return ADM_OCA_HTTP_ECODE_CURL_SET_ERR;
    }
#endif

#ifdef SUPPORT_LION_PKI
    //设置双向认证证书
    if (curl_easy_setopt_cert_and_privatekey(handle_sc, curlHandle, USER_ID_SSL, userPin)) {
        log_w("curl_easy_setopt_se_tls error");
        return ADM_OCA_HTTP_ECODE_SSL_SERVER_ROOT_CERT;
    }

#else
    log_i("Cert info server root cert path is %s", serverRootCert);
    if(curl_easy_setopt(curlHandle, CURLOPT_CAINFO, serverRootCert))
    {
        log_w("curl_easy_setopt CURLOPT_CAINFO %s error", serverRootCert);
        return ADM_OCA_HTTP_ECODE_SSL_SERVER_ROOT_CERT;
    }

    log_i("Cert info client cert path is %s", clientCert);
    if (curl_easy_setopt(curlHandle, CURLOPT_SSLCERT, clientCert)) {
        log_w("curl_easy_setopt CURLOPT_SSLCERT %s error", clientCert);
        return ADM_OCA_HTTP_ECODE_SSL_CLIENT_CERT;
    }

    /** 设置客户端证书的密码和证书类型
    if (NULL != pstCertInfo->clientCertPasswd) {
        log_i("Cert info client cert passwd is %s", pstCertInfo->clientCertPasswd);
        if (curl_easy_setopt(curlHandle, CURLOPT_SSLCERTPASSWD, pstCertInfo->clientCertPasswd)) {
            log_w("curl_easy_setopt CURLOPT_SSLCERTPASSWD %s error", pstCertInfo->clientCertPasswd);
            return ADM_OCA_HTTP_ECODE_SSL_CLIENT_CERT_PASS;
        }
    } else {
        log_e("Cert info client cert passwd is NULL");
        return ADM_OCA_HTTP_ECODE_SSL_CLIENT_CERT_PASS;
    }

    if (NULL != pstCertInfo->clientCertType) {
        log_i("Cert info client cert type is %s", pstCertInfo->clientCertType);
        if (curl_easy_setopt(curlHandle, CURLOPT_SSLCERTTYPE, pstCertInfo->clientCertType)) {
            log_w("curl_easy_setopt CURLOPT_SSLCERTTYPE %s error", pstCertInfo->clientCertType);
            return ADM_OCA_HTTP_ECODE_SSL_CLIENT_CERT_TYPE;
        }
    } else {
        log_e("Cert info client cert type is NULL");
        return ADM_OCA_HTTP_ECODE_SSL_CLIENT_CERT_TYPE;
    }
    */

    log_i("Cert info client private key is %s", clientPrivateKey);
    if (curl_easy_setopt(curlHandle, CURLOPT_SSLKEY, clientPrivateKey)) {
        log_w("curl_easy_setopt CURLOPT_SSLKEY %s error", clientPrivateKey);
        return ADM_OCA_HTTP_ECODE_SSL_CLIENT_PRIVATE_KEY;
    }

    /** 设置客户端私钥的密码和私钥类型
    if (NULL != pstCertInfo->clientPrivateKeyPasswd) {
        log_i("Cert info client private key passwd is %s", pstCertInfo->clientPrivateKeyPasswd);
        if (curl_easy_setopt(curlHandle, CURLOPT_SSLKEYPASSWD, pstCertInfo->clientPrivateKeyPasswd)) {
            log_w("curl_easy_setopt CURLOPT_SSLKEYPASSWD %s error", pstCertInfo->clientPrivateKeyPasswd);
            return ADM_OCA_HTTP_ECODE_SSL_CLIENT_PRIVATE_KEY_PASS;
        }
    } else {
        log_e("Cert info client private key passwd is NULL");
        return ADM_OCA_HTTP_ECODE_SSL_CLIENT_PRIVATE_KEY_PASS;
    }

    if (NULL != pstCertInfo->clientPrivateKeyType) {
        log_i("Cert info client private key type is %s", pstCertInfo->clientPrivateKeyType);
        if (curl_easy_setopt(curlHandle, CURLOPT_SSLKEYTYPE, pstCertInfo->clientPrivateKeyType)) {
            log_w("curl_easy_setopt CURLOPT_SSLKEYTYPE %s error", pstCertInfo->clientPrivateKeyType);
            return ADM_OCA_HTTP_ECODE_SSL_CLIENT_PRIVATE_KEY_TYPE;
        }
    } else {
        log_e("Cert info client private key type is NULL");
        return ADM_OCA_HTTP_ECODE_SSL_CLIENT_PRIVATE_KEY_TYPE;
    }
    */
#endif

    return ADM_OCA_HTTP_ECODE_SUCCESSED;
}


// ssl初始化，不需要认证，单向认证，双向认证
D32S adm_oca_httpReqAuthInit(const ADM_OCA_CONNECT_PROTOCOL_TYPE_E eProtocolType, const DCHAR* serverRootCert, const DCHAR* clientCert, const DCHAR* clientPrivateCert, CURL* curlHandle, 
DBOOL bIsCloudDownload)
{
    D32S iRet = ADM_OCA_HTTP_ECODE_FAILED;

    if (NULL == curlHandle) {
        log_e("Parameter (curlhandle) verification failed, pointer invalid");
        return ADM_OCA_HTTP_ECODE_INVALID_POINTER;
    }

    switch (eProtocolType)
    {
        case ADM_OCA_CONNECT_PROTOCOL_TYPE_HTTP:
            log_i("eProtocolType is %d, use no auth to request platform", eProtocolType);
            iRet = adm_oca_httpNoAuthInit(curlHandle);
            if (ADM_OCA_HTTP_ECODE_SUCCESSED == iRet) {
                log_i("adm_oca_httpNoAuthInit successed, [err:%d]", iRet);
            }
            break;
        case ADM_OCA_CONNECT_PROTOCOL_TYPE_HTTPS_ONE_AUTH:
            log_i("eProtocolType is %d, use one auth to request platform", eProtocolType);
            iRet = adm_oca_httpReqOneAuthInit(curlHandle, serverRootCert, bIsCloudDownload);
            if (ADM_OCA_HTTP_ECODE_SUCCESSED != iRet) {
                log_e("adm_oca_httpReqOneAuthInit error, [err:%d]", iRet);
            }
            break;
        case ADM_OCA_CONNECT_PROTOCOL_TYPE_HTTPS_MUL_AUTH:
            log_i("eProtocolType is %d, use mul auth to request platform", eProtocolType);
            iRet = adm_oca_httpReqMulAuthInit(curlHandle, serverRootCert, clientCert, clientPrivateCert);
            if (ADM_OCA_HTTP_ECODE_SUCCESSED != iRet) {
                log_e("adm_oca_httpReqMulAuthInit error, [err:%d]", iRet);
            }
            break;
        default:
            log_w("eProtocolType %d is invalid", eProtocolType);
            iRet = ADM_OCA_HTTP_ECODE_INVALID_POINTER;
            break;
    }

    log_i("The result of http auth init is [err:%d]", iRet);
    return iRet;
}

static void adm_oca_tspRecvCallback(const DCHAR *data, D32U dataLen)
{
    // TODO:
    log_i("adm_oca_tspRecvCallback called !!!");
    return;
}

#ifdef SUPPORT_LION_PKI
/// @brief 检查验签证书
/// @return 
D32S adm_oca_checkSignCert()
{
    D32S iRet = ADM_OCA_HTTP_ECODE_SUCCESSED;

    // 检查验签证书（文件验签所需公钥）
    iRet = IW_CheckCertificateExist(handle_sc, USER_ID_OTA);
    if (iRet != 1) {
        // 验签证书通过 IW_GetCertificateByUserID 接口下载
        iRet = IW_GetCertificateByUserID(handle_sc, USER_ID_OTA);
        log_d("IW_GetCertificateByUserID Code: %02x.\n", iRet);
    } else {
        log_d("Certificate already exists.\n");
        iRet = ADM_OCA_HTTP_ECODE_SUCCESSED;
    }

    return iRet;
}

/// @brief 检查ssl证书
/// @return 
D32S adm_oca_checkSslCert()
{
    D32S iRet = ADM_OCA_HTTP_ECODE_SUCCESSED;

    SC_KeyAlgorithm algorithm = SC_NONE;

	//    FLAIRTESTVIND0001 >> hash:7ec6840be4d2c36fcb4e1f8df0d643453c1ef13415f9cf6701ee27eedfa605a6
    //char clientID[] = "7ec6840be4d2c36fcb4e1f8df0d643453c1ef13415f9cf6701ee27eedfa605a6";//VIN的sha256哈希

    if (NULL == g_pSnCode)
    {
        log_w("Oca has not set PKI info.");
        return ADM_OCA_HTTP_ECODE_FAILED;
    }

    // 检查SSL证书（双向认证）
    iRet = IW_CheckCertificateExist(handle_sc, USER_ID_SSL);
    if (iRet != 1) {
        SC_CSRInformation information = {0};
        
        strcpy(information.subjectInfo.C, "CN");        ///< 可选
        strcpy(information.subjectInfo.O, "Chery CA");  ///< 可选
        strcpy(information.subjectInfo.SN, g_clientID); ///< vin-sha256
        strcpy(information.subjectInfo.CN, g_pSnCode) ;  ///< ICC的sn号
        // strcpy(information.subjectInfo.DC1, "cdc_generic_tls_cvtsp");   ///< V23
        strcpy(information.subjectInfo.DC1, "cdc_generic_tls_ntsp");  /* E01 **/
        strcpy(information.subjectInfo.DC2, "cdc");
        strcpy(information.subjectInfo.DC3, "vehicle");
        
        algorithm = SC_ED25519;

        // SSL证书通过 IW_ApplyCertificate 接口下载
        iRet = IW_ApplyCertificate(handle_sc,
                                  USER_ID_SSL,
                                  userPin,
                                  algorithm,
                                  information);

        if (iRet == SC_SUCCESS) 
        {
            adm_pub_setKeylogString(ADM_PUB_KEYLOG_ID_007, iRet, "");
            log_d("IW_ApplyCertificate Success.\n");
        } 
        else 
        {
            adm_pub_setKeylogString(ADM_PUB_KEYLOG_ID_008, iRet, "");
            log_e("IW_ApplyCertificate Fail, Fail Code: %02x.\n", iRet);
            return iRet;
        }
        
    } else {
        log_d("Certificate already exists.\n");
        iRet = ADM_OCA_HTTP_ECODE_SUCCESSED;
    }

    return iRet;
}

/// @brief 计算hash-sha256
/// @param str 
/// @param digest 
static void _getStrHash(const DCHAR *str, D8U *result) {
    SHA256_CTX ctx;
    SHA256_Init(&ctx);
    SHA256_Update(&ctx, str, strlen(str));
    SHA256_Final(result, &ctx);
}

/// @brief 初始化服务（国内测试环境）
/// @return 
D32S adm_oca_scInit(DCHAR* pSnCode, DCHAR *vin, D32U uiServerType)
{
    D32S iRet = ADM_OCA_HTTP_ECODE_SUCCESSED;
    D8U clientID[SHA256_DIGEST_LENGTH];
    const DCHAR* pPkiDataDir = NULL;    ///< 持久化目录,可读写
    DCHAR* pPkiUrl = NULL;

    long certflag = 0;
    D32S i = 0;

    if (NULL == pSnCode || NULL == vin)
    {
        log_i("invalid param.");
        return ADM_OCA_HTTP_ECODE_INVALID_PARAM;
    }

    pCurlDir = adm_cfg_iniParserGetString(ADM_CFG_VUS_CURL_VERBOSE_DIR, NULL);
    
    /** 证书链 */
    certflag = 369098759;   // ssl+OTA V23 & D01 信长城提供

    g_pSnCode = adm_strdup(pSnCode);
    
    log_i("vin:%s", vin);
    log_i("snCode[%s]", pSnCode);

    /** V23车型 tsp不支持hash校验,直接用vin码 其他车型用hash*/ 
    DCHAR str_out[128];
    DCHAR *ptr = str_out;

    _getStrHash(vin, clientID);

    for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i) 
    {
        sprintf(ptr, "%02x", clientID[i]);
        ptr += 2;
    }
    log_e("str_out %s.\n", str_out);
    adm_strcpy(g_clientID, str_out, sizeof(g_clientID));

    // adm_strcpy(g_clientID, vin, sizeof(g_clientID));
    pPkiDataDir = adm_cfg_iniParserGetString(ADM_CFG_VUS_PKI_DIR, ADM_CFG_VUS_PKI_DIR_DEFAULT_VAL);

    if (ADM_EMS_SERVER_TYPE_DEFAULT_2 == uiServerType)
    {
        pPkiUrl =  adm_cfg_iniParserGetString(ADM_CFG_VUS_PKI_URL_2, ADM_CFG_VUS_PKI_URL_DEFAULT_VAL_2);
        // pPkiUrl = adm_strdup(g_pPKIUrl2);
    }
    else
    {
        pPkiUrl =  adm_cfg_iniParserGetString(ADM_CFG_VUS_PKI_URL, ADM_CFG_VUS_PKI_URL_DEFAULT_VAL);
        // pPkiUrl = adm_strdup(g_pPKIUrl1);
    }

    iRet = IW_InitService(&handle_sc,
                         pPkiDataDir,
                         pPkiUrl,
                         SC_Client_Vehicle,
                         g_clientID,
                         certflag);

    if (iRet == SC_SUCCESS) 
    {
        adm_pub_setKeylogString(ADM_PUB_KEYLOG_ID_005, iRet, "");
        log_e("InitService Success.\n");
    } 
    else 
    {
        adm_pub_setKeylogString(ADM_PUB_KEYLOG_ID_006, iRet, "");
        log_e("InitService Fail, Fail Code: %d.\n", iRet);
    }

    adm_free(pPkiUrl);
    return iRet;
}

/// @brief 反初始化服务
/// @return 
D32S adm_oca_scDeinit()
{
    IW_FreeService(handle_sc);//释放句柄
    adm_free(g_pSnCode);
}
#endif

// D32S adm_oca_setPkiUrl(D32S iCountryCode, D32S iBrand)
// {
//     D32S iRet = ADM_OCA_HTTP_ECODE_SUCCESSED;
//     adm_free(g_pPKIUrl1);
//     adm_free(g_pPKIUrl2);
//     adm_free(g_pTenantInfo);

// #ifdef CLOUD_ENV_TEST       /**测试环境*/
//     iRet = ocaGetPkiUrl((ADM_COUNTRY_E)iCountryCode,(ADM_BRAND_E)iBrand, &g_pPKIUrl1, &g_pPKIUrl2);
// #else /**生产环境*/
//     iRet = ocaGetPkiUrl((ADM_COUNTRY_E)iCountryCode,(ADM_BRAND_E)iBrand, &g_pPKIUrl2, &g_pPKIUrl1);
// #endif
//     if (iRet != ADM_OCA_HTTP_ECODE_SUCCESSED)
//     {
//         log_e("GetPkiUrl Failed, Fail Code: %d", iRet);
//     }
//     else
//     {
//         log_i("g_pPKIUrl1 : %s", g_pPKIUrl1);
//         log_i("g_pPKIUrl2 : %s", g_pPKIUrl2);
//         log_i("g_pTenantInfo : %s", g_pTenantInfo);
//     }

//     return iRet;
// }

/**
 * @brief curl handler 初始化
 * 
 * @param curlHandler           [OUT]   CURL**  初始化的句柄
 * @return D32S                 0:successed, non-0:failed
 */
D32S adm_oca_httpCurlInit(CURL** curlHandler)
{
    CURL* curl = NULL;
    D32S iRet = ADM_OCA_HTTP_ECODE_FAILED;
    static DBOOL isCurlGlobalInit = FALSE;

    do {
        //!< 因为此接口为非线程安全，所以全局只需要调用一次
        if (FALSE == isCurlGlobalInit) {
            if (CURLE_OK != curl_global_init(CURL_GLOBAL_ALL)) {
                log_e("curl_global_init error");
                iRet = ADM_OCA_HTTP_ECODE_CURL_INIT_FAILED;
                break;
            }
            isCurlGlobalInit = TRUE;
        }

        curl = curl_easy_init();
        if (NULL == curl) {
            log_e("curl_easy_init error, pointer invalid");
            iRet = ADM_OCA_HTTP_ECODE_CURL_INIT_FAILED;
            break;
        }

#ifdef SUPPORT_LION_PKI
        iRet = adm_oca_checkSslCert();
        if (iRet != CURLE_OK) {
			log_e("curl_easy_setopt_se_tls error, error code: %d\n", iRet);
            iRet = ADM_OCA_HTTP_ECODE_SSL_CERTPROBLEM;
            break;
		}    
#endif        

    } while(FALSE);

    *curlHandler = curl;
    
    return iRet;
}

/**
 * @brief curl handler 释放
 * 
 * @param curlHander            [IN]    CURL*  初始化的句柄
 */
void adm_oca_httpCurlDeinit(CURL* curlHander)
{
    if (curlHander != NULL) 
    {
    //TODO: 暂时注释掉此接口调用，再想解决办法
    // 1. 不调用此接口会出现内存泄漏问题；
    // 2. 调用此接口在树莓派Android环境会出现crash:
    //原因为其他程序内存泄露导致curlHander指针被设置为NULL，设置为全局变量问题暂时解决
    //TODO:maxiangyang 设置pki相关信息失败后会有崩溃,需要解决
        curl_easy_cleanup(curlHander);
    }
}
