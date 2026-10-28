/**
 * @file adm_oca_file.h
 * @author mengwei@abupdate.com
 * @brief 用于定义文件上传和下载的通用接口
 * @version 0.1
 * @date 2021-05-15
 * 
 * @copyright Copyright © 2018 All rights 上海艾拉比智能科技有限公司
 * 
 */
#ifndef __ADM_OCA_FILE_H__
#define __ADM_OCA_FILE_H__

#ifdef __cplusplus
extern "C" {
#endif

#define EXPORT_FUN __attribute__((visibility("default")))

#include "adm_typedefs.h"
#include "adm_debug.h"
#include "adm_ocs_airProtocolAdapter.h"
#include "adm_oca_connect.h"

/***************************************************************************File download****************************************************************************/
/**
 * @brief 文件下载
 * @note 同步接口，此接口用于从平台下载文件
 * @warning 下载传入的文件路径为绝对路径，且路径目录存在，并需要执行可读写
 * 
 * @param connectHandler    [IN] const ADM_OCA_COMMON_INIT_HANDLER_T*  此次通信使用的句柄
 * @param pstDownloadInfo      [IN] ADM_OCA_FILE_DOWNLOAD_INFO_T1*      从平台下载文件的数据
 * @param bIsCloudDownload    [IN] DBOOL  是否从云端下载
 * @return D32S             0:successed, non-0:failed
 */
extern D32S adm_oca_startDownload(const ADM_OCA_COMMON_INIT_HANDLER_T* connectHandler, ADM_OCA_FILE_DOWNLOAD_INFO_T1* pstDownloadInfo,
DBOOL bIsCloudDownload);

/**
 * @brief 请求下载的信息空间申请
 * 
 * @return ADM_OCA_FILE_DOWNLOAD_INFO_T1*    NULL:failed, Non-NULL:successed
 */
extern ADM_OCA_FILE_DOWNLOAD_INFO_T1* adm_oca_newDownloadInfo(void);


/**
 * @brief 请求下载的信息空间释放
 * 
 * @param pstDownloadInfo   ADM_OCA_FILE_DOWNLOAD_INFO_T1*       使用adm_oca_newDownloadInfo申请的空间
 *   
 */
extern void adm_oca_delDownloadInfo(ADM_OCA_FILE_DOWNLOAD_INFO_T1* pstDownloadInfo);

/**
 * @brief 暂停下载
 * @note 同步接口
 * @param actionType   D32S      打断下载的类型枚举值 参考ADM_OCA_DOWNLOAD_ACTION_TYPE_E枚举
 * @return D32S     0:successed, non-0:failed
 */
extern D32S adm_oca_interuptDownload(const D32S actionType);
/***************************************************************************File download****************************************************************************/

/***************************************************************************File upload******************************************************************************/
/**
 * @brief HTTP POST section设置类型
 * 
 */
typedef enum ADM_OCA_HTTP_POST_SECTION_TYPE_ {
    ADM_OCA_HTTP_POST_SECTION_TYPE_FILE = 1,            /*!< 设置被上传文件的路径 */
    ADM_OCA_HTTP_POST_SECTION_TYPE_COPYCONTENTS,        /*!< 设置类型为内容 */
    ADM_OCA_HTTP_POST_SECTION_TYPE_INVALID              /*!< 设置类型无效 */
} ADM_OCA_HTTP_POST_SECTION_TYPE_E;

typedef struct ADM_OCA_FILE_UPLOAD_HANDLE_  ADM_OCA_FILE_UPLOAD_HANDLE_T;   //!< 前向声明上报使用的句柄

/**
 * @brief 初始化文件上传所需的句柄
 * @note 句柄使用完成后，请使用adm_oca_fileUploadDeinit释放句柄
 * 
 * @return ADM_OCA_FILE_UPLOAD_HANDLE_T*    Not-NULL:successed, NULL:failed 
 */
extern ADM_OCA_FILE_UPLOAD_HANDLE_T* adm_oca_fileUploadInit(void);

/**
 * @brief 释放文件上传所需句柄
 * @note 与adm_oca_fileUploadInit对应
 * 
 * @param pHandle   ADM_OCA_FILE_UPLOAD_HANDLE_T*   文件上传需要的句柄
 */
extern void adm_oca_fileUploadDeinit(ADM_OCA_FILE_UPLOAD_HANDLE_T* pHandle);


/**
 * @brief add a section to a multipart/formdata HTTP POST
 * 
 * @param pHandle       [IN] ADM_OCA_FILE_UPLOAD_HANDLE_T*    文件上传所需要的的句柄
 * @param sectionName   [IN] const DCHAR*   变量名称
 * @param section       [IN] const DCHAR*   变量值
 * @return D32S         0:successed, non-0:failed
 */
extern D32S adm_oca_addSectionToHttpPost(ADM_OCA_FILE_UPLOAD_HANDLE_T* pHandle, const DCHAR* sectionName, const DCHAR* section, ADM_OCA_HTTP_POST_SECTION_TYPE_E eType);

/**
 * @brief 请求上传的信息空间申请
 * 
 * @return ADM_OCA_FILE_UPLOAD_INFO_T*    NULL:failed, Non-NULL:successed
 */
extern ADM_OCA_FILE_UPLOAD_INFO_T* adm_oca_newUploadInfo(void);


/**
 * @brief 请求上传的信息空间释放
 * 
 * @param pstUploadInfo   ADM_OCA_FILE_UPLOAD_INFO_T*       使用adm_oca_newUploadInfo申请的空间
 *   
 */
extern void adm_oca_delUploadInfo(ADM_OCA_FILE_UPLOAD_INFO_T* pstUploadInfo);

/**
 * @brief 文件上传
 * @note 此接口用于向平台请求文件上传
 * @warning 上传的文件路径需要为全路径，且接口可读
 * 
 * @param connectHandler[IN]    const ADM_OCA_COMMON_INIT_HANDLER_T*   此次通信使用的句柄
 * @param pHandle       [IN]    const ADM_OCA_FILE_UPLOAD_HANDLE_T*    文件上传所需要的的句柄
 * @param uploadInfo    [IN]    ADM_OCA_FILE_UPLOAD_INFO_T*         向平台上传文件的数据
 * @param rspOutData    [OUT]   DCHAR**                     平台响应的数据信息
 * @warning rspOutData使用结束后需要free
 * 
 * @return D32S             0:successed, non-0:failed
 */
extern D32S adm_oca_startUpload(const ADM_OCA_COMMON_INIT_HANDLER_T* connectHandler, ADM_OCA_FILE_UPLOAD_HANDLE_T* pHandle, ADM_OCA_FILE_UPLOAD_INFO_T* uploadInfo, DCHAR** rspOutData);

/**
 * @brief 向平台上传文件
 * @param in ADM_OCS_UPLOAD_URL_INFO_T *pstUploadUrlInfo 上传文件url信息
 * @param in ADM_OCS_UPLOADFILE_T* pstUploadInfo 上传文件信息
 * @return D32S 0:successed, non-0:failed
 */
extern D32S adm_oca_startPutFile(DCHAR *pcCertPath, ADM_OCS_UPLOAD_URL_INFO_T *pstUploadUrlInfo, ADM_OCS_UPLOADFILE_T* pstUploadInfo);
/***************************************************************************File upload******************************************************************************/

#ifdef __cplusplus  // close out "C" linkage in case of c++ compiling
}
#endif

#endif//__ADM_OCA_FILE_H__
