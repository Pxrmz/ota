#include "adm_oca_httpConnect.h"
#include "adm_oca_httpCommon.h"
#include "curl/curl.h"
#include "adm_string.h"
#include "adm_memory.h"
#include "adm_oca_httpErrorCode.h"
#include <string.h>
#include <stdlib.h>

static D32S adm_oca_httpReqMethodSet(CURL *curlHandle, struct curl_slist **headers, ADM_OCA_HTTP_REQUEST_METHOD_E eMethodType, ADM_OCA_HTTP_POST_CONTENT_TYPE_E eContentType)
{
    D32S iRet = ADM_OCA_HTTP_ECODE_FAILED;

    if (NULL == curlHandle) {
        log_e("Parameter (curlhandle) verification failed, pointer invalid");
        return ADM_OCA_HTTP_ECODE_INVALID_POINTER;
    }

    switch (eMethodType)
    {
        case ADM_OCA_HTTP_REQUEST_METHOD_GET:
            /* code */

            break;

        case ADM_OCA_HTTP_REQUEST_METHOD_POST:
            log_i("eMethodType is %d, use POST request platform", eMethodType);
            iRet = adm_oca_httpReqPostContentTypeSet(curlHandle, eContentType, headers);
            if (ADM_OCA_HTTP_ECODE_SUCCESSED != iRet) {
                log_e("adm_oca_httpReqPostContentTypeSet error, reason code: %d", iRet);
            }
            break;
    
        case ADM_OCA_HTTP_REQUEST_METHOD_INVALID:
        default:
            break;
    }

    return ADM_OCA_HTTP_ECODE_SUCCESSED;
}

#if 0
//MemoryStruct是用来给curl的callback函数WriteMemoryCallback回传数据的，很有用
struct MemoryStruct {
	DCHAR *memory;
	size_t size;
};


//callback函数，用来在curl_eacy_perform之后,通过自己定义的函数
//获取网络上返回的数据，此示例是官方公布，将数据存入内容的方式，写的很巧妙
static size_t
adm_oca_httpReqBufferWrite(DCHAR *contents, size_t size, size_t nmemb, void *userp)
{
    size_t realsize = size * nmemb;
    struct MemoryStruct *mem = (struct MemoryStruct *)userp;

    log_i("*************%p, %p, %p", mem, mem->memory, &mem->size);
    log_i("realsize %lld, memorysize:%lld", realsize, mem->size);

    if (NULL == mem->memory) {
        log_e("input mem->memory is nil");
        return -1;
    }


    char *ptr = realloc(mem->memory, mem->size + realsize + 1);
    if(!ptr) {
        /* out of memory! */
        log_w("not enough memory (realloc returned NULL)\n");
        return 0;
    }

    mem->memory = ptr;
    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;

    return realsize;
}


static void adm_oca_chunkDeinit(struct MemoryStruct* chunk)
{
    if (NULL != chunk) {
        if (NULL != chunk->memory) {
            adm_free(chunk->memory);
        }

        adm_free(chunk);
    }
}

static struct MemoryStruct* adm_oca_chunkInit(void)
{
    struct MemoryStruct* chunk = NULL;

    chunk = (struct MemoryStruct*)adm_malloc(sizeof(struct MemoryStruct));
    if (NULL == chunk) {
        log_e("adm_oca_chunk malloc failed");
        return NULL;
    } else {
        (void)adm_memset(chunk, 0x00, sizeof(struct MemoryStruct), sizeof(struct MemoryStruct) * sizeof(DCHAR));
        chunk->memory = adm_malloc(sizeof(DCHAR));  /* will be grown as needed by realloc above */
        if (NULL == chunk->memory) {
            log_e("adm_oca_chunk memory malloc failed");
            adm_oca_chunkDeinit(chunk);
            return NULL;
        }
        chunk->size = 0;    /* no data at this point */
    }

    return chunk;
}
#endif

static size_t adm_oca_httpReqBufferWrite(DCHAR *buffer,
                               size_t size,
                               size_t nitems,
                               void *userdata)
{
    DCHAR **respond = (char**)userdata;
    DCHAR *buf = NULL;
    DSIZE mallocLen = 0;
    DSIZE len = 0;

    if (*(DCHAR **)userdata == NULL)
    {
        mallocLen = size * nitems + 1U;
        buf = (DCHAR *)adm_malloc(mallocLen);
        if(buf == NULL)
        {
            log_e("malloc size %d error", mallocLen);
            return 0;
        }
        (void)adm_memcpy(buf, buffer, size * nitems, mallocLen);
        buf[size * nitems] = '\0';
    }
    else
    {
        len = adm_strlen(*(DCHAR **)userdata);
        mallocLen = size * nitems + len + 1U;
        buf = (DCHAR *)adm_malloc(mallocLen);
        if(buf == NULL)
        {
            log_e("malloc size %d error", mallocLen);
            return 0;
        }
        (void)adm_memset(buf, 0, mallocLen, mallocLen);
        (void)adm_memcpy(buf, *(DCHAR **)userdata, len, mallocLen);
        (void)adm_strncat(buf, buffer, size * nitems, mallocLen);
        buf[mallocLen - 1U] = '\0';
        adm_free(*(DCHAR **)userdata);
        *(DCHAR **)userdata = NULL;
    }

    *respond = buf;

    return nitems;
}


static D32S adm_oca_httpReqCurlOptSet(const ADM_OCA_COMMON_INIT_HANDLER_T* connectHandler, CURL* curlHandle, struct curl_slist** headers, void* rspOutData)
{
    D32S iRet = ADM_OCA_HTTP_ECODE_FAILED;
 
    if (NULL == connectHandler || NULL == curlHandle) {
        log_e("connectHandler or curlHanle is nil");
        return ADM_OCA_HTTP_ECODE_INVALID_POINTER;
    }

    if (NULL == connectHandler->address || NULL == connectHandler->requestBody) {
        log_e("address or requestBody is nil");
        return ADM_OCA_HTTP_ECODE_INVALID_POINTER;
    }

   do {
        iRet = adm_oca_httpReqSetCurlOptDefault(curlHandle, connectHandler->address, adm_oca_httpReqBufferWrite, rspOutData, connectHandler->requestBody);
        if (ADM_OCA_HTTP_ECODE_SUCCESSED != iRet) {
            log_e( "adm_oca_httpReqSetCurlOptDefault failed, reason of error code is %d", iRet);
            break;
        }

        /** Set http request method & post content type*/
        iRet = adm_oca_httpReqMethodSet(curlHandle, headers, ADM_OCA_HTTP_REQUEST_METHOD_POST, ADM_OCA_HTTP_POST_CONTENT_TYPE_APPLICATION_JSON);
        if (ADM_OCA_HTTP_ECODE_SUCCESSED != iRet) {
            log_w("adm_oca_httpReqMethodSet ADM_OCS_HTTP_REQUEST_METHOD_POST ADM_OCS_HTTP_POST_CONTENT_TYPE_TEXT_PLAIN error");
            break;
        }

        /** Set the authentication method for the requesting platform */
        iRet = adm_oca_httpReqAuthInit(connectHandler->eConnectProtocolType, connectHandler->serverRootCert, connectHandler->clientCert, connectHandler->clientPrivateKey, curlHandle, 
        DFALSE);
        if (ADM_OCA_HTTP_ECODE_SUCCESSED != iRet) {
            log_e("adm_oca_httpReqAuthInit error, reason code: %d", iRet);
            break;
        }

            iRet = adm_oca_httpReqLimitTimeSpeedSet(curlHandle, 60, 1024);
        if (ADM_OCA_HTTP_ECODE_SUCCESSED != iRet) {
            log_w("adm_oca_httpReqLimitTimeSpeedSet error, reason code: %d", iRet);
            break;
        }

        if (NULL != connectHandler->outputApn) {
            iRet = adm_oca_httpReqSetOutputApn(curlHandle, connectHandler->outputApn);
            if (ADM_OCA_HTTP_ECODE_SUCCESSED != iRet) {
                log_w("adm_oca_httpReqSetOutputApn error, reason code:%d", iRet);
                break;
            }
        }

    } while(FALSE);

    return iRet;
}

/**
 * @brief 使用HTTP/HTTPS协议请求与平台通信，并接收平台响应的数据。
 * 
 * @param connectHandler    [IN]    ADM_OCA_COMMON_INIT_HANDLER_T*   此次与平台通信的句柄
 * @param responseData      [OUT]   DCHAR**     平台响应的数据，使用后需要释放
 * @param statusCode        [OUT]   DSIZE*       平台响应的状态码，用于判断此次验签和解密的标识
 * @return D32S             0:successed, Non-0:failed
 */
D32S adm_oca_httpConnect(const ADM_OCA_COMMON_INIT_HANDLER_T* connectHandler, DCHAR** responseData, DSIZE* statusCode)
{
    D32S iRet = ADM_OCA_HTTP_ECODE_FAILED;
    DSIZE httpCode = ADM_OCA_HTTP_ECODE_SUCCESSED;
    CURL *curlHandle = NULL; 
    struct curl_slist *headers = NULL;
    // struct MemoryStruct *chunk = NULL;

    if (NULL == connectHandler) {
        log_e("connectHandler is nil");
        return ADM_OCA_HTTP_ECODE_INVALID_POINTER;
    }

    do {
        iRet = adm_oca_httpCurlInit(&curlHandle);
        if (NULL == curlHandle) {
            log_e("curl_easy_init error, pointer invalid");
            iRet = ADM_OCA_HTTP_ECODE_CURL_INIT_FAILED;
            break;
        }
#if 0
        chunk = adm_oca_chunkInit();
        if (NULL == chunk) {
            iRet = ADM_OCA_HTTP_ECODE_APPLY_MEMORY_FAILED;
            log_e("adm_oca_chunkInit failed, iRet:%d", iRet);
            break;
        }
#endif
        iRet = adm_oca_httpReqCurlOptSet(connectHandler, curlHandle, &headers, (void*)responseData);
        if (ADM_OCA_HTTP_ECODE_SUCCESSED != iRet) {
            log_w("adm_oca_httpReqCurlOptSet error, reason code:%d", iRet);
        } else {
            iRet = adm_oca_httpReqReallyPerform(curlHandle, &httpCode);
            *statusCode = httpCode;
        }

        if (NULL != responseData) {
            log_i("OTA Server response data is %s", *responseData);
        } else {
            log_e("OTA Server response data is nil");
            *responseData = NULL;
        }

        // if (NULL != chunk->memory) {
        //     log_i("OTA Server response data is %s", chunk->memory);
        //     *responseData = adm_strdup(chunk->memory);
        // } else {
        //     log_e("OTA Server response data is nil");
        //     *responseData = NULL;
        // }

    } while(FALSE);


    if (NULL != headers) {
        curl_slist_free_all(headers);
    }
    adm_oca_httpCurlDeinit(curlHandle);
    // adm_oca_chunkDeinit(chunk);

    return iRet;
}
