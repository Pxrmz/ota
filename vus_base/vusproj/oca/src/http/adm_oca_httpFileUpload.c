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
#include "adm_oca_httpCommon.h"
#include "adm_oca_httpFile.h"
#include "adm_memory.h"
#include "adm_string.h"
#include "adm_error.h"
#include "adm_file.h"
#include "adm_cfg_api.h"
#include "adm_cfg_default.h"
#include <string.h>

// static struct curl_httppost* formpost = NULL;
// static struct curl_httppost* lastptr = NULL;

static FILE *g_uploadCurlVerboseFp = NULL;

static D32S adm_oca_openCurlVerboseUpload(CURL* curlHandle)
{
    D32S iRet = ADM_OCA_HTTP_ECODE_SUCCESSED;
    const DCHAR* curlDir = NULL;

    if (curlHandle == NULL)
    {
        log_w("curlHandle is null when open verbose");
        return ADM_OCA_HTTP_ECODE_INVALID_POINTER;
    }

    curlDir = adm_cfg_iniParserGetString(ADM_CFG_VUS_UPLOAD_CURL_VERBOSE_DIR, "");
    if (NULL == curlDir)
    {
        log_w("Curl verbose dir is null.");
        return ADM_OCA_HTTP_ECODE_INVALID_PARAM;
    }

    g_uploadCurlVerboseFp = fopen(curlDir, "a+");
    if (g_uploadCurlVerboseFp == NULL)
    {
        log_w("open curlDir[%s] error.", curlDir);
        return ADM_OCA_HTTP_ECODE_FAILED;
    }

    long size = ftell(g_uploadCurlVerboseFp);
    if (size > 5*1024*1024)
    {
        log_i("curl verbose file > 5M, rotate");
        fclose(g_uploadCurlVerboseFp);
        (DVOID)adm_fremove(curlDir);
        g_uploadCurlVerboseFp = fopen(curlDir, "a+");
        if (g_uploadCurlVerboseFp == NULL)
        {
            log_w("reopen curlDir[%s] error.", curlDir);
            return ADM_OCA_HTTP_ECODE_FAILED;
        }
    }

    if(curl_easy_setopt(curlHandle, CURLOPT_STDERR, g_uploadCurlVerboseFp))
    {
        log_w("curl_easy_setopt CURLOPT_STDERR error");
        fclose(g_uploadCurlVerboseFp);
        g_uploadCurlVerboseFp = NULL;
        return ADM_OCA_HTTP_ECODE_CURL_SET_ERR;
    }

    if(curl_easy_setopt(curlHandle, CURLOPT_VERBOSE, 1L))
    {
        log_w("curl_easy_setopt CURLOPT_VERBOSE error");
        fclose(g_uploadCurlVerboseFp);
        g_uploadCurlVerboseFp = NULL;
        return ADM_OCA_HTTP_ECODE_CURL_SET_ERR;
    }

    return iRet;
}

static void adm_oca_closeCurlVerboseUpload(CURL* curlHandle)
{
    if (curlHandle != NULL)
    {
        (void)curl_easy_setopt(curlHandle, CURLOPT_VERBOSE, 0L);
        (void)curl_easy_setopt(curlHandle, CURLOPT_STDERR, NULL);
    }
    if (g_uploadCurlVerboseFp != NULL)
    {
        fclose(g_uploadCurlVerboseFp);
        g_uploadCurlVerboseFp = NULL;
    }
}


/**
 * @brief 本地转换curl_formadd错误码为VUS错误码
 * 
 * @param curlformCode  [IN]    CURLFORMcode    curl_formadd错误码
 * @return D32S         0:successed, non-0:failed        
 */
static D32S adm_oca_curlformaddErrorCodeTransfer(CURLFORMcode curlformCode)
{
    D32S iRet = ADM_OCA_HTTP_ECODE_FAILED;
    switch (curlformCode)
    {
    case CURL_FORMADD_OK:
        iRet = ADM_OCA_HTTP_ECODE_SUCCESSED;
        break;
    case CURL_FORMADD_MEMORY:
        iRet = ADM_OCA_HTTP_ECODE_CURL_FORMADD_MEMORY;
        break;
    case CURL_FORMADD_OPTION_TWICE:
        iRet = ADM_OCA_HTTP_ECODE_CURL_FORMADD_OPTION_TWICE;
        break;
    case CURL_FORMADD_NULL:
        iRet = ADM_OCA_HTTP_ECODE_CURL_FORMADD_NULL;
        break;
    case CURL_FORMADD_UNKNOWN_OPTION:
        iRet = ADM_OCA_HTTP_ECODE_CURL_FORMADD_UNKNOWN_OPTION;
        break;
    case CURL_FORMADD_INCOMPLETE:
        iRet = ADM_OCA_HTTP_ECODE_CURL_FORMADD_INCOMPLETE;
        break;
    case CURL_FORMADD_ILLEGAL_ARRAY:
        iRet = ADM_OCA_HTTP_ECODE_CURL_FORMADD_ILLEGAL_ARRAY;
        break;
    case CURL_FORMADD_DISABLED:
        iRet = ADM_OCA_HTTP_ECODE_CURL_FORMADD_DISABLED;
        break;
    case CURL_FORMADD_LAST:
    default:
        break;
    }

    log_i( "curl_formadd code:%d, iRet:%d", curlformCode, iRet);

    return iRet;
}

ADM_OCA_FILE_UPLOAD_HANDLE_T* adm_oca_fileUploadInit(void)
{
    (void)curl_global_init(CURL_GLOBAL_ALL);

    ADM_OCA_FILE_UPLOAD_HANDLE_T* pHandle = NULL;
    pHandle = adm_malloc(sizeof(ADM_OCA_FILE_UPLOAD_HANDLE_T));
    if (NULL != pHandle) {
        (void)adm_memset(pHandle, 0x00, sizeof(ADM_OCA_FILE_UPLOAD_HANDLE_T), sizeof(ADM_OCA_FILE_UPLOAD_HANDLE_T) * sizeof(DCHAR));
    }

    return pHandle;
}

void adm_oca_fileUploadDeinit(ADM_OCA_FILE_UPLOAD_HANDLE_T* pHandle)
{
    if (NULL != pHandle) {
        if (NULL != pHandle->formpost) {
            curl_formfree(pHandle->formpost);
        }

        if (NULL != pHandle->lastptr) {
            // TODO: ????
        }

        if (NULL != pHandle->headers) {
            curl_slist_free_all(pHandle->headers);
        }

        adm_oca_httpCurlDeinit(pHandle->curlHandle);

        adm_free(pHandle);
    }
}

/**
 * @brief add a section to a multipart/formdata HTTP POST
 * 
 * @param pHandle       [IN] ADM_OCA_FILE_UPLOAD_HANDLE_T*    文件上传所需要的的句柄
 * @param sectionName   [IN] const DCHAR*   变量名称
 * @param section       [IN] const DCHAR*   变量值
 * @return D32S         0:successed, non-0:failed
 */
D32S adm_oca_addSectionToHttpPost(ADM_OCA_FILE_UPLOAD_HANDLE_T* pHandle, const DCHAR* sectionName, const DCHAR* section, ADM_OCA_HTTP_POST_SECTION_TYPE_E eType)
{
    D32S iRet = ADM_OCA_HTTP_ECODE_FAILED;
    CURLformoption eOptionType = CURLFORM_LASTENTRY;
    if (NULL == pHandle) {
        iRet = ADM_OCA_HTTP_ECODE_FILE_UPLOAD_HANDLE_INVALID;
        log_e("file upload handle invalid:%d", iRet);
        return iRet;
    }

    if (NULL == sectionName || NULL == section) {
        iRet = ADM_OCA_HTTP_ECODE_INVALID_PARAM;
        log_e( "Parameter (sectionName | section) verifaction failed, reason of error code is %d", iRet);
        return iRet;
    }

    switch (eType)
    {
    case ADM_OCA_HTTP_POST_SECTION_TYPE_FILE:
        eOptionType = CURLFORM_FILE;
        break;
    case ADM_OCA_HTTP_POST_SECTION_TYPE_COPYCONTENTS:
        eOptionType = CURLFORM_COPYCONTENTS;
        break;
    default:
        break;
    }

    iRet = (D32S)curl_formadd(&pHandle->formpost, &pHandle->lastptr, 
                        CURLFORM_COPYNAME, sectionName,
                        eOptionType, section,
                        CURLFORM_END);

    iRet = adm_oca_curlformaddErrorCodeTransfer((CURLFORMcode)iRet);
    if (ADM_OCA_HTTP_ECODE_SUCCESSED != iRet) {
        log_e( "curl_formadd sectionName:%s, section:%s failed, reason of error code is %d", iRet);
    }

    return iRet;
}

/**
 * @brief 请求上传的信息空间申请
 * 
 * @return ADM_OCA_FILE_UPLOAD_INFO_T*    NULL:failed, Non-NULL:successed
 */
ADM_OCA_FILE_UPLOAD_INFO_T* adm_oca_newUploadInfo(void)
{
    ADM_OCA_FILE_UPLOAD_INFO_T* pstUploadInfo = NULL;

    pstUploadInfo = adm_malloc(sizeof(ADM_OCA_FILE_UPLOAD_INFO_T) * sizeof(DCHAR));
    if (NULL != pstUploadInfo) {
        (void)adm_memset(pstUploadInfo, 0x00, sizeof(ADM_OCA_FILE_UPLOAD_INFO_T), sizeof(ADM_OCA_FILE_UPLOAD_INFO_T) * sizeof(DCHAR));
    }

    return pstUploadInfo;
}

/**
 * @brief 请求上传的信息空间释放
 * 
 * @param pstUploadInfo   ADM_OCA_FILE_UPLOAD_INFO_T*       使用adm_oca_newUploadInfo申请的空间
 *   
 */
void adm_oca_delUploadInfo(ADM_OCA_FILE_UPLOAD_INFO_T* pstUploadInfo)
{
    if (NULL != pstUploadInfo) {
        adm_free(pstUploadInfo);
    }
}



static size_t adm_oca_httpReqBufferWrite(DCHAR* rspData, const size_t rspDataSize, const size_t nItems, void* rspOutData)
{
    DCHAR **respond = (char**)rspOutData;
    DCHAR *buf = NULL;
    DSIZE mallocLen = 0;

    if (NULL == rspOutData) {
        log_e( "failed@@@@@@@@@@@@@@  ");
        return nItems;
    }

    if (*(char **)rspOutData == NULL)
    {
        mallocLen = rspDataSize * nItems + 1U;
        buf = (DCHAR *)adm_malloc(mallocLen);
        if(buf == NULL)
        {
            log_e("malloc size %d error", mallocLen);
            return ADM_OCA_HTTP_ECODE_SUCCESSED;
        }
        // extern void *(void)adm_memcpy(void *dest, void *src, DSIZE srcLen, DSIZE dstLen);
        (void)adm_memcpy((void*)buf, rspData, rspDataSize * nItems, mallocLen);
        buf[rspDataSize * nItems] = '\0';
    }
    else
    {
        DSIZE len = 0;

        len = adm_strlen((const char *)(*(DCHAR **)rspOutData));

        if (len > 0U) {
            mallocLen = rspDataSize * nItems + len + 1U;
            buf = (DCHAR *)adm_malloc(mallocLen);
            if(buf == NULL)
            {
                log_e("malloc size %d error", mallocLen);
                return ADM_OCA_HTTP_ECODE_SUCCESSED;
            }
            (void)adm_memset((void*)buf, 0, mallocLen, mallocLen);
            (void)adm_memcpy((void*)buf, rspOutData, len, mallocLen);
            
            // TODO: OS中需要增加adm_strncat函数
            (void)strncat(buf, rspData, rspDataSize * nItems);
            buf[mallocLen - 1U] = '\0';
            adm_free((*(DCHAR **)rspOutData));
            *(DCHAR **)rspOutData = NULL;
        } else {
            buf = NULL;
        }
    }

    *respond = buf;

    return nItems;
}


static D32S adm_oca_setUploadCurlOpt(CURL* curlHandle, struct curl_httppost* formpost)
{
    if (CURLE_OK != curl_easy_setopt(curlHandle, CURLOPT_POST, 1L)) {
        log_w( "curl_easy_setopt CURLOPT_POST failed");
        return ADM_OCA_HTTP_ECODE_CURL_SET_ERR;
    }

    if (CURLE_OK != curl_easy_setopt(curlHandle, CURLOPT_HTTPPOST, formpost)) {
        log_w( "curl_easy_setopt CURLOPT_HTTPPOST failed");
        return ADM_OCA_HTTP_ECODE_CURL_SET_ERR;
    }

    return ADM_OCA_HTTP_ECODE_SUCCESSED;
}

typedef struct {
    CURL* curl;
    ADM_OCA_FILE_PROGRESS_CALLBACK_F cbUploadProgress;
    void *priv;
} My_Progress;

/**
 * @brief 上传进度处理接口
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
    My_Progress* prog = (My_Progress*)clientp;
    ADM_OCA_FILE_PROGRESS_T progress;
    double speed = 0.00;
    double remainTime = 0.00;

    if (NULL != prog->curl) {
        /* curl_get_info必须在curl_easy_perform之后调用 */
        (void)curl_easy_getinfo(prog->curl, CURLINFO_SPEED_UPLOAD, &speed);
    }

    curl_off_t tmp = ultotal - ulnow;
    if (0.00 != speed) {
        remainTime =  (double)tmp/speed;
    }

    progress.remainTime = remainTime;
    progress.savedSize = (DSIZE)ulnow;
    progress.speed = speed;
    if (NULL != prog->cbUploadProgress) {
        prog->cbUploadProgress(&progress, prog->priv);
    }

    return 0;
}

/**
 * @brief 文件上传
 * @note 此接口用于向平台请求文件上传
 * @warning 上传的文件路径需要为全路径，且接口可读
 * 
 * @param pHandle       [IN]    const ADM_OCA_FILE_UPLOAD_HANDLE_T*    文件上传所需要的的句柄
 * @param uploadInfo    [IN]    ADM_OCA_FILE_UPLOAD_INFO_T*         向平台上传文件的数据
 * @param rspOutData    [OUT]   DCHAR**                     平台响应的数据信息
 * @warning rspOutData使用结束后需要free
 * 
 * @return D32S             0:successed, non-0:failed
 */
D32S adm_oca_startUpload(const ADM_OCA_COMMON_INIT_HANDLER_T* connectHandler, ADM_OCA_FILE_UPLOAD_HANDLE_T* pHandle, ADM_OCA_FILE_UPLOAD_INFO_T* uploadInfo, DCHAR** rspOutData)
{
    D32S iRet = ADM_OCA_HTTP_ECODE_FAILED;
    DSIZE httpCode = 0U;
    My_Progress prog = {0};
    DBOOL isFocusOnProg = FALSE;

    do {
        if (NULL == connectHandler || NULL == pHandle) {
            iRet = ADM_OCA_HTTP_ECODE_INVALID_POINTER;
            log_e("paramters connectHanlder or pHandle is nil");
            break;
        }

        //!< 参数校验
        if (NULL == uploadInfo) {
            iRet = ADM_OCA_HTTP_ECODE_INVALID_PARAM;
            log_w("Input param upload is nil, do not need to focus on progress, err:%d", iRet);
            // break;
        } else {
            isFocusOnProg = TRUE;
            log_i("File upload need to focus on progress");
        }

        // pHandle->curlHandle = curl_easy_init();
        iRet = adm_oca_httpCurlInit(&pHandle->curlHandle);
        if (NULL == pHandle->curlHandle) {
            iRet= ADM_OCA_HTTP_ECODE_CURL_INIT_FAILED;
            log_e("curl_easy_init init http post error reason code:%d", iRet);
            break;
        }

         /** Set up default of http request */
        iRet = adm_oca_httpReqSetCurlOptDefault(pHandle->curlHandle, connectHandler->address, adm_oca_httpReqBufferWrite, (void*)&rspOutData, NULL);
        if (ADM_OCA_HTTP_ECODE_SUCCESSED != iRet) {
            log_e( "adm_oca_httpReqSetCurlOptDefault failed, reason of error code is %d", iRet);
            break;
        }

        iRet = adm_oca_httpReqAuthInit(connectHandler->eConnectProtocolType, connectHandler->serverRootCert, connectHandler->clientCert, connectHandler->clientPrivateKey, pHandle->curlHandle, 
        DFALSE);
        if (ADM_OCA_HTTP_ECODE_SUCCESSED != iRet) {
            log_e("adm_oca_httpReqAuthInit error, reason code: %d", iRet);
            break;
        }

        /** Set http request time & speed limit*/
        iRet = adm_oca_httpReqLimitTimeSpeedSet(pHandle->curlHandle, 60, 1024);
        if (ADM_OCA_HTTP_ECODE_SUCCESSED != iRet) {
            log_w("adm_oca_httpReqLimitTimeSpeedSet error, reason code: %d", iRet);
            break;
        }

        if (TRUE == isFocusOnProg) {
            //!< 设置上传进度
            prog.curl = pHandle->curlHandle;
            prog.priv = uploadInfo->priv;
            prog.cbUploadProgress = uploadInfo->cbUploadProgress;
            if (CURLE_OK != curl_easy_setopt(pHandle->curlHandle, CURLOPT_XFERINFOFUNCTION, xferinfo)) {
                log_w("curl_easy_setopt CURLOPT_XFERINFOFUNCTION error");
                iRet = ADM_OCA_HTTP_ECODE_CURL_SET_ERR;
                break;
            }

            if (CURLE_OK != curl_easy_setopt(pHandle->curlHandle, CURLOPT_XFERINFODATA, &prog)) {
                log_w("curl_easy_setopt CURL_LOW_SPEED_LIMIT error");
                iRet = ADM_OCA_HTTP_ECODE_CURL_SET_ERR;
                break;
            }

            if (CURLE_OK != curl_easy_setopt(pHandle->curlHandle, CURLOPT_NOPROGRESS, 0L)) {
                log_w("curl_easy_setopt CURLOPT_NOPROGRESS error");
                iRet = ADM_OCA_HTTP_ECODE_CURL_SET_ERR;
                break;
            }
        } else {
            if (CURLE_OK != curl_easy_setopt(pHandle->curlHandle, CURLOPT_NOPROGRESS, 1L)) {
                log_w("curl_easy_setopt CURLOPT_NOPROGRESS error");
                iRet = ADM_OCA_HTTP_ECODE_CURL_SET_ERR;
                break;
            }
        }


        /** Warning: 
         *  1. 设置headers应该在设置CURLOPT_HTTPPOST之前；
         *  2. 此处有坑，如果两者顺序交换会出现 HTTP server响应错误码为507；
         */

        /** Set up http post content  */
        iRet = adm_oca_httpReqPostContentTypeSet(pHandle->curlHandle, ADM_OCA_HTTP_POST_CONTENT_TYPE_MULTIPART_FORM_DATA, &pHandle->headers);
        if (ADM_OCA_HTTP_ECODE_SUCCESSED != iRet) {
            log_e("adm_oca_httpReqPostContentTypeSet error, reason code: %d", iRet);
            break;
        }

        /** Set up the special configuration required for uploading */
        iRet = adm_oca_setUploadCurlOpt(pHandle->curlHandle, pHandle->formpost);
        if (ADM_OCA_HTTP_ECODE_SUCCESSED != iRet) {
            log_w( "adm_oca_setUploadCurlOpt failed, reason of error code is %d", iRet);
            break;
        }

        iRet = adm_oca_httpReqReallyPerform(pHandle->curlHandle, &httpCode);
        if (ADM_OCA_HTTP_ECODE_SUCCESSED != iRet) {
            log_e( "adm_oca_httpReqReallyPerform failed, reason code: %d", iRet);
        } else {
            log_i( "adm_oca_httpReqReallyPerform successed");
        }

    } while(FALSE);

    return iRet;
}

/**
 * @brief 向平台上传文件
 * @param in ADM_OCS_UPLOAD_URL_INFO_T *pstUploadUrlInfo 上传文件url信息
 * @param in ADM_OCS_UPLOADFILE_T* pstUploadInfo 上传文件信息
 * @return D32S 0:successed, non-0:failed
 */
D32S adm_oca_startPutFile(DCHAR *pcCertPath, ADM_OCS_UPLOAD_URL_INFO_T *pstUploadUrlInfo, ADM_OCS_UPLOADFILE_T* pstUploadInfo)
{
    CURL *curl;
    CURLcode res;
    FILE *file;
    D32S iRet = ADM_ECODE_SUCCESSED;
    long httpCode = 0;
    DCHAR *respBody = NULL;

    /** 打开文件 */
    file = fopen(pstUploadInfo->fullFileNamePath, "rb");
    if (!file) 
    {
        log_e("file open failed");
        return ADM_ECODE_OPEN_FILE_FAILED;
    }

    // 初始化 cURL
    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();

    if (curl) 
    {
        // 设置 URL
        curl_easy_setopt(curl, CURLOPT_URL, pstUploadUrlInfo->pcUploadUrl);

        // 使用 PUT 方法上传文件
        curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);

        // 绑定文件流
        curl_easy_setopt(curl, CURLOPT_READDATA, file);

        // 设置文件大小（可选）
        fseek(file, 0L, SEEK_END);
        long file_size = ftell(file);
        rewind(file);
        curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE, (curl_off_t)file_size);

        // 启用服务器证书验证（单向认证）
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L); // 验证服务器证书

        // 设置CA证书路径（用于验证服务器证书）
        curl_easy_setopt(curl, CURLOPT_CAINFO, pcCertPath);

        // 上传期间开启详细日志
        if (adm_oca_openCurlVerboseUpload(curl) != ADM_ECODE_SUCCESSED)
        {
            log_w("open curl verbose failed, continue without verbose");
        }

        // 捕获服务器响应体，增加调试信息
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, adm_oca_httpReqBufferWrite);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&respBody);

        // 执行上传
        res = curl_easy_perform(curl);

        //关闭详细日志
        adm_oca_closeCurlVerboseUpload(curl);

        // 获取 HTTP 响应码
        (void)curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);

        if (res == CURLE_OK) 
        {
            if (httpCode >= 200 && httpCode < 300)
            {
                log_d("put file successed, httpCode:%ld, resp:%s",
                      httpCode, (respBody != NULL ? respBody : "null"));
                iRet = ADM_ECODE_SUCCESSED;
            }
            else
            {
                log_e("put file http error, code:%ld, resp:%s",
                      httpCode, (respBody != NULL ? respBody : "null"));
                iRet = ADM_ECODE_FAILED;
            }
        }
        else 
        {
            if (httpCode == 0)
            {
                log_e("put file curl failed (no http response), curl:%s, resp:%s",
                      curl_easy_strerror(res), (respBody != NULL ? respBody : "null"));
            }
            else
            {
                log_e("put file curl failed, curl:%s, httpCode:%ld, resp:%s",
                      curl_easy_strerror(res), httpCode, (respBody != NULL ? respBody : "null"));
            }
            iRet = ADM_ECODE_FAILED;
        }

        // 清理
        curl_easy_cleanup(curl);
    }

    if (respBody != NULL)
    {
        adm_free(respBody);
    }

    fclose(file);
    curl_global_cleanup();

    return iRet;
}