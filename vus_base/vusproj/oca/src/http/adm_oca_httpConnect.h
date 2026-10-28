/**
 * @file adm_oca_httpConnect.h
 * @author mengwei@abupdate.com
 * @brief   定义用HTTP协议与平台做报文交互的接口。
 * @version 0.1
 * @date 2021-05-14
 * 
 * @copyright Copyright © 2018 All rights 上海艾拉比智能科技有限公司
 * 
 */
#ifndef __ADM_OCA_HTTP_CONNECT_H__
#define __ADM_OCA_HTTP_CONNECT_H__

#ifdef __cplusplus
extern "C" {
#endif

#define EXPORT_FUN __attribute__((visibility("default")))

#include "adm_typedefs.h"
#include "adm_oca_common.h"

/**
 * @brief 使用HTTP协议请求平台的方式
 * 
*/
typedef enum ADM_OCA_HTTP_REQUEST_METHOD_ {
    ADM_OCA_HTTP_REQUEST_METHOD_GET = 1,        /* !< 使用GET方式请求平台   */
    ADM_OCA_HTTP_REQUEST_METHOD_POST,           /* !< 使用POST方式请求平台  */
    ADM_OCA_HTTP_REQUEST_METHOD_INVALID         /* !< 请求平台方式无效      */
} ADM_OCA_HTTP_REQUEST_METHOD_E;


/**
 * @brief 发送到服务器时的内容编码
 * 
*/
typedef enum ADM_OCA_HTTP_POST_CONTENT_TYPE_ {
    ADM_OCA_HTTP_POST_CONTENT_TYPE_AUTO = 1,                                /* !< auto      */
    ADM_OCA_HTTP_POST_CONTENT_TYPE_APPLICATION_X_WWW_FORM_URLENCODED,       /* !< application/x-www-form-urlencoded     */
    ADM_OCA_HTTP_POST_CONTENT_TYPE_MULTIPART_FORM_DATA,                     /* !< multipart/form-data       */
    ADM_OCA_HTTP_POST_CONTENT_TYPE_APPLICATION_JSON,                        /* !< application/json          */
    ADM_OCA_HTTP_POST_CONTENT_TYPE_TEXT_PLAIN,                              /* !< text/plain                */
    ADM_OCA_HTTP_POST_CONTENT_TYPE_INVALID                                  /* !< invalid                   */
} ADM_OCA_HTTP_POST_CONTENT_TYPE_E;


/**
 * @brief 使用HTTP/HTTPS协议请求与平台通信，并接收平台响应的数据。
 * 
 * @param connectHandler    [IN]    ADM_OCA_COMMON_INIT_HANDLER_T*   此次与平台通信的句柄
 * @param responseData      [OUT]   DCHAR**     平台响应的数据，使用后需要释放
 * @param statusCode        [OUT]   DSIZE*       平台响应的状态码，用于判断此次验签和解密的标识
 * @return D32S             0:successed, Non-0:failed
 */
extern D32S adm_oca_httpConnect(const ADM_OCA_COMMON_INIT_HANDLER_T* connectHandler, DCHAR** responseData, DSIZE* statusCode);

#ifdef __cplusplus  // close out "C" linkage in case of c++ compiling
}
#endif

#endif//__ADM_OCA_HTTP_CONNECT_H__