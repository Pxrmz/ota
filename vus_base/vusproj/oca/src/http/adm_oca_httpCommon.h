/**
 * @file adm_oca_httpCommon.h
 * @author mengwei@abupdate.com
 * @brief HTTP请求的公共接口，包括：错误码转换、发起HTTP请求
 * @version 0.1
 * @date 2021-02-27
 * 
 * @copyright Copyright © 2018 All rights 上海艾拉比智能科技有限公司
 * 
 */
#ifndef _ADM_OCA_HTTP_COMMON_H_
#define _ADM_OCA_HTTP_COMMON_H_

#include "adm_typedefs.h"
#include "adm_oca_log.h"
#include "curl/curl.h"
#include "adm_oca_connect.h"
#include "adm_oca_httpConnect.h"
// #include "adm_oca_httpRequest.h"

#ifdef __cplusplus
extern "C" {
#endif

#define EXPORT_FUN __attribute__((visibility("default")))

#ifdef SUPPORT_LION_PKI

#include "SC_PKI.h"
#include "SC_TLS.h"
// #define     USER_ID_OTA     "Ch_OTA_RSAPSS_4096"  ///< 信长城分配的 是否按照车型或者品牌区分，后续如果按照需要对接 和云端确认下
#define     USER_ID_OTA     "Ch_LIONOTA_RSAPSS_4K"  ///< 信长城分配的 是否按照车型或者品牌区分，后续如果按照需要对接 和云端确认下
#define     USER_ID_SSL     "userid_ota_ssl"     ///< abup按照品牌分区,自定义
extern SC_HANDLE handle_sc;

extern D32S adm_oca_checkSignCert();
extern D32S adm_oca_checkSslCert();
extern D32S adm_oca_scInit(DCHAR* pSnCode, DCHAR *vin, D32U uiServerType);
extern D32S adm_oca_scDeinit();
// extern D32S adm_oca_setPkiUrl(D32S iCountryCode,D32S iBrand);
#endif

/**
 * @brief 将HTTP平台响应的错误码转化为本地的错误码
 * 
 * @param curlCode  [IN]    const CURLcode      HTTP平台响应的状态码
 * @return D32S     0:successed， non-0:failed 
 */
extern D32S adm_oca_httpReqCurlCodeTransfer(const CURLcode curlCode);

// extern CURLcode sslctx_function(CURL *curl, void *sslctx, void *parm);

/**
 * @brief 最终使用libcurl发起HTTP请求
 * 
 * @param curlHandle    [IN]    CURL*     libcurl句柄
 * @param httpCode      [OUT]   DSIZE*           HTTP响应的状态码
 * @return D32S         0:successed， non-0:failed 
 */
extern D32S adm_oca_httpReqReallyPerform(CURL* curlHandle, DSIZE* httpCode);

/**
 * @brief HTTP请求设置超时限制，例如：配置60s内需要响应1024Bytes
 * 
 * @param curlHandle    [IN]    CURL*     curl句柄
 * @param speedTime     [IN]    const D32U      限制时间，单位：s
 * @param speedByte     [IN]    const D32U      限制字节数，单位：Byte
 * @return D32S         0:successed， non-0:failed
 */
extern D32S adm_oca_httpReqLimitTimeSpeedSet(CURL* curlHandle, const D32U speedTime, const D32U speedByte);


/**
 * @brief HTTP请求设置出口网卡或IP
 * 
 * @param curlHandle[IN]    CURL*   curl句柄
 * @param outputApn[IN]     const DCHAR*    网卡名或IP地址
 * @return D32S     0:Successed, None-0:failed
 */
extern D32S adm_oca_httpReqSetOutputApn(CURL* curlHandle, const DCHAR* outputApn);

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
extern D32S adm_oca_httpReqSetCurlOptDefault(CURL* curlHandle, const DCHAR* url, curl_write_callback writeCallback, void *data, DCHAR *messageBody);


/**
 * @brief 为HTTP POST请求设置headers
 * 
 * @param curlHandle    [IN]    CURL*               curl句柄
 * @param eContentType  [IN]    ADM_OCA_HTTP_POST_CONTENT_TYPE_E    post消息content类型
 * @param headers       [OUT]   struct curl_slist   设置后的headers结构
 * @return D32S 
 */
extern D32S adm_oca_httpReqPostContentTypeSet(CURL* curlHandle, ADM_OCA_HTTP_POST_CONTENT_TYPE_E eContentType, struct curl_slist **headers);


/**
 * @brief SSL认证的初始化
 * 
 * @param eAuthType     ADM_OCA_REQ_AUTH_TYPE_E       SSL认证类型
 * @param pstCertInfo   const ADM_OCA_CERT_INFO_T*          SSL证书信息
 * @param curlHandle    const CURL*                         CURL句柄
 * @return D32S         int_32  0:successed， non-0:failed
 */

/**
 * @brief SSL认证的初始化
 * 
 * @param eProtocolType         [IN]    ADM_OCA_CONNECT_PROTOCOL_TYPE_E     协议和认证类型
 * @param serverRootCert        [IN]    const DCHAR*    服务器根证书
 * @param clientCert            [IN]    const DCHAR*    客户端证书
 * @param clientPrivateCert     [IN]    const DCHAR*    客户端私钥
 * @param curlHandle            [IN]    const CURL*     curl句柄
 * @param bIsCloudDownload      [IN]    DBOOL           是否云端下载
 * @return D32S                 0:successed， non-0:failed
 */
extern D32S adm_oca_httpReqAuthInit(const ADM_OCA_CONNECT_PROTOCOL_TYPE_E eProtocolType, const DCHAR* serverRootCert, const DCHAR* clientCert, const DCHAR* clientPrivateCert, CURL* curlHandle, DBOOL bIsCloudDownload);

/**
 * @brief curl handler 初始化
 * 
 * @param curlHandler           [OUT]   CURL**  初始化的句柄
 * @return D32S                 0:successed, non-0:failed
 */
extern D32S adm_oca_httpCurlInit(CURL** curlHandler);


/**
 * @brief curl handler 释放
 * 
 * @param curlHander            [IN]    CURL*  初始化的句柄
 */
extern void adm_oca_httpCurlDeinit(CURL* curlHander);

#ifdef __cplusplus  // close out "C" linkage in case of c++ compiling
}
#endif

#endif//_ADM_OCA_HTTP_COMMON_H_
