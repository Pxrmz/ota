/**
 * @file adm_oca_interface.h
 * @author mengwei@abupdate.com
 * @brief 定义车云通信、文件下载和上传的通用接口，并实现给到core service使用
 * @version 0.1
 * @date 2021-12-08
 * 
 * @copyright Copyright © 2018 All rights 上海艾拉比智能科技有限公司
 * 
 */
#ifndef __ADM_OCA_INTERFACE_H__
#define __ADM_OCA_INTERFACE_H__


#ifdef __cplusplus
extern "C" {
#endif

#define EXPORT_FUN __attribute__((visibility("default")))

#include "adm_typedefs.h"
#include "adm_oca_connect.h"
#include "adm_oca_file.h"
#include "adm_ocs_airProtocolAdapter.h"
#include "adm_ecs_rsConfig.h"

typedef D32S (*ADM_OCM_UPDATE_OTACONFIG_CACHE_INFO_CALLBACK_F)(ADM_ECS_RSCONFIGDATA_T **ppstOtaConfigData);   //!< 更新ota缓存的平台配置信息，从平台重新获取一次配置信息
typedef D32S (*ADM_OCM_APC_KVSTORAGE_INIT_CALLBACK_F)(void);            //!< kvstorage数据库初始化接口
typedef D32S (*ADM_OCM_APC_OTA_FIRST_BLOOD_CALLBACK_F)(void);             //!< 注册和获取配置的初始化接口
typedef void (*ADM_OCM_UPDATE_ECU_CACHE_INFO_CALLBACK_F)(void);         //!< 更新ECU缓存信息
typedef void (*ADM_OCM_VSM_STATE_DEL_CALLBACK_F)(void);                 //!< 状态机清零
typedef void (*ADM_OCM_DEAL_STATUSCODE_CALLBACK_F)(void);               //!< 处理云端响应的状态码
typedef D32S (*ADM_OCM_APC_LOG_INIT_CALLBACK_F)(void);                      //!< 初始化日志
typedef D32S (*ADM_OCM_APC_CFG_RESET_CALLBACK_F)(DBOOL bIsInstalling);                      //!< 配置文件重置

/**
 * @brief 定义用于处理状态码的结构信息
 * 
 */
typedef struct ADM_OCM_STATUS_CODE_INFO_ {
    D32S statusCode;                                //!< 云端响应的Code信息
    DBOOL isDoing;                                  //!< 状态码是否在处理中
    ADM_OCM_DEAL_STATUSCODE_CALLBACK_F dealCb;      //!< 处理状态码的回调
} ADM_OCM_STATUS_CODE_INFO_T;

/**
 * @brief 车云业务通信的初始化
 * 
 * @param updateCfgCb   [IN]        更新ota缓存的平台配置信息，从平台重新获取一次配置信息
 * @param kvInitCb      [IN]        kvstorage数据库初始化接口
 * @param otaFirstBlCb  [IN]        注册和获取配置的初始化接口
 * @param updateEcuCacheCb [IN]     更新ECU缓存信息的接口
 * @param stateDelCb [IN]           状态机清除的接口
 * @param logInitCb     [IN]        日志初始化的接口，用于配置更新时，需要更新日志的过滤级别
 * @param uiServerType[in]           服务器类型标识：1: ini文件中的默认服务器配置; 2: ini文件中的备用服务器配置   
 * @return D32S         0:Successed, None-0:Failed
 */
extern D32S adm_ocm_init(ADM_OCM_UPDATE_OTACONFIG_CACHE_INFO_CALLBACK_F updateCfgCb, 
                        ADM_OCM_APC_KVSTORAGE_INIT_CALLBACK_F kvInitCb,
                        ADM_OCM_APC_OTA_FIRST_BLOOD_CALLBACK_F otaFirstBlCb,
                        ADM_OCM_UPDATE_ECU_CACHE_INFO_CALLBACK_F updateEcuCacheCb,
                        ADM_OCM_VSM_STATE_DEL_CALLBACK_F stateDelCb,
                        ADM_OCM_APC_LOG_INIT_CALLBACK_F logInitCb,
                        D32U uiServerType);

/**
 * @brief 车云业务通信接口的释放
 * 
 * @param void     [IN]    void
 * @return D32S    0:Successed, None-0:Failed
 */
extern D32S adm_ocm_destroy(void);

/**
 * @brief 车云业务通信的接口，包含：注册、检测、获取配置、事件上报、状态上报、进度上报等
 * @note rspMessageBody 使用后需释放空间
 * 
 * @param eBusinessType     [IN]    车云通信协议中的接口ID标识
 * @param reqMessageBody    [IN]    请求平台的数据
 * @param rspMessageBody    [OUT]   平台响应的数据
 * @param rspStatusCode     [OUT]   平台响应的状态码 
 * @return D32S             0:Successed, None-0:Failed(根据不同的状态码区分消息)
 */
extern D32S adm_ocm_businessCommInterface(const ADM_OCS_BUSINESS_TYPE_E eBusinessType, const DCHAR* reqMessageBody, DCHAR** rspMessageBody, DSIZE* rspStatusCode);

/**
 * @brief 通过此接口注册给到core service执行文件下载
 * 
 * @param downloadUrl       [IN]    待下载文件的URL
 * @param pstDownloadInfo   [IN]    文件下载需要的信息
 * @return D32S             0:Successed, None-0:Failed(根据不同的状态码区分消息)
 */
extern D32S adm_ocm_fileDownloadInterface(const DCHAR* downloadUrl, ADM_OCA_FILE_DOWNLOAD_INFO_T1* pstDownloadInfo);

/**
 * @brief 通过此接口注册给到core service执行文件上传
 * 
 * @param uploadInfo        [IN]    文件上传需要的信息
 * @param rspMessageBody    [OUT]   平台响应的状态码
 * @param pstUploadInfo              [IN/OUT]    请求云端上传文件需要的参数
 * @return D32S             0:Successed, None-0:Failed
 */
extern D32S adm_ocm_fileUploadInterface(ADM_OCA_FILE_UPLOAD_INFO_T* uploadInfo, DCHAR** rspMessageBody, ADM_OCS_UPLOADFILE_T* pstUploadInfo);


/**
 * @brief 通过此接口注册给到core service 执行文件下载(用于UC-Lite从UC-Master下载文件)
 * 
 * @param downloadUrl       [IN]    待下载文件的URL
 * @param pstDownloadInfo   [IN]    文件下载需要的信息
 * @return D32S             0:Successed, None-0:Failed(根据不同的状态码区分消息)
 */
extern D32S adm_ocm_ewebDownloadFileInterface(const DCHAR* downloadUrl, ADM_OCA_FILE_DOWNLOAD_INFO_T1* pstDownloadInfo);


/**
 * @brief 通过此接口注册给到core service执行文件上传(用于UC-Lite向UC-Master上传文件)
 * 
 * @param enConnProtocolType [IN]        上传协议类型(当前仅支持http)
 * @param rspMessageBody     [OUT]       平台响应的状态码
 * @param priv               [IN/OUT]    请求云端上传文件需要的参数
 * @return D32S              0:Successed, None-0:Failed
 */
extern D32S adm_ocm_ewebUploadFileInterface(ADM_OCA_SIMPLE_FILEUPLOAD_INFO_T1 *pstUploadInfo, DCHAR** rspMessageBody);

/**
 * @brief 判断是否正在更新配置文件
 * 
 * @param void     [IN]    void
 * @return DBOOL   TRUE:正在更新配置文件，FALSE:没有在更新配置文件
 */
extern DBOOL adm_ocm_isUpdateConfig(void);

extern DCHAR* adm_ocm_getCommUrl(const ADM_OCS_BUSINESS_TYPE_E eBusinessType);

// extern D32S adm_ocm_setDomainName(D32S iCountryCode, D32S iBrand);


#ifdef __cplusplus  // close out "C" linkage in case of c++ compiling
}
#endif

#endif//__ADM_OCA_INTERFACE_H__
