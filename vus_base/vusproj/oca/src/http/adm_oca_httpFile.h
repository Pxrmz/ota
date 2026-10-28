#ifndef __ADM_OCA_HTTP_FILE_H__
#define __ADM_OCA_HTTP_FILE_H__

#ifdef __cplusplus
extern "C" {
#endif

#define EXPORT_FUN __attribute__((visibility("default")))

#include "adm_typedefs.h"
#include "adm_debug.h"
#include "curl/curl.h"
#include "adm_oca_connect.h"
#include "adm_oca_file.h"

/**
 * @brief 下载动作类型
 * 
 */
typedef enum ADM_OCA_DOWNLOAD_ACTION_TYPE_ {
    ADM_OCA_DOWNLOAD_ACTION_TYPE_NORMAL = 1,                    /** 正常下载       */
    ADM_OCA_DOWNLOAD_ACTION_TYPE_PAUSE,                         /** 暂停下载       */
    ADM_OCA_DOWNLOAD_ACTION_TYPE_CANCEL,                        /** 取消下载       */
    ADM_OCA_DOWNLOAD_ACTION_TYPE_NET_CHG_PAUSE,                 /** 由于网络切换导致的暂停 */
    ADM_OCA_DOWNLOAD_ACTION_TYPE_COND_ABNORMAL_PAUSE,           /** 由于下载条件检验不通过导致的暂停 */
    ADM_OCA_DOWNLOAD_ACTION_TYPE_PAUSE_IDEL_TRANSFER,           /** 下载过程中，暂停闲时传输 */
    ADM_OCA_DOWNLOAD_ACTION_TYPE_INVALID                        /** 类型无效       */
} ADM_OCA_DOWNLOAD_ACTION_TYPE_E;


// typedef struct ADM_OCA_FILE_DOWNLOAD_HANDLE_ {
//     struct curl_slist *headers;
//     CURL *curlHandle;
// } ADM_OCA_FILE_DOWNLOAD_HANDLE_T;


typedef struct ADM_OCA_FILE_UPLOAD_HANDLE_ {
    struct curl_httppost* formpost;
    struct curl_httppost* lastptr;
    struct curl_slist *headers;
    CURL *curlHandle;
} ADM_OCA_FILE_UPLOAD_HANDLE_T;


#ifdef __cplusplus  // close out "C" linkage in case of c++ compiling
}
#endif

#endif//__ADM_OCA_HTTP_FILE_H__
