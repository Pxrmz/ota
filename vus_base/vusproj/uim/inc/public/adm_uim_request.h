/**
 * @file adm_uim_request.h
 * @brief 为uim定义请求hmi的接口。
 * @copyright Copyright © 2018 All rights 上海艾拉比智能科技有限公司
 */
#ifndef __ADM_UIM_REQUEST_H__
#define __ADM_UIM_REQUEST_H__

#ifdef __cplusplus
extern "C" {
#endif

#define EXPORT_FUN __attribute__((visibility("default")))

#include "adm_typedefs.h"
#include "adm_uim_common.h"
#include "adm_uim_check.h"
#include "adm_uim_download.h"
#include "adm_uim_install.h"

//#include "adm_application_common.h"
extern D32S adm_uim_hmi_set_log_level(const D16U instanceId, const D16U timeout, 
        D32U logLevel, D32S* errorCodes);
typedef D32S (*adm_uim_hmi_set_log_level_func)(const D16U instanceId, const D16U timeout, 
        D32U logLevel, D32S* errorCodes);

extern void adm_uim_hmi_getSdkVersion(const D16U instanceId, const D16U timeout, 
        DCHAR **sdkVersion, DCHAR **apkVersion,D32S *errorCode);
typedef void (*adm_uim_hmi_getSdkVersion_func)(const D16U instanceId, const D16U timeout, 
        DCHAR **sdkVersion, DCHAR **apkVersion,D32S *errorCodes);

/**
 * @brief 检测结果通知-RR消息通知.
 * 
 * @param eCmdidType                    [IN]    const ADM_SYSI4HMI_CMDID_TYPE_E                 Type of detection trigger.
 * @param sessionId                     [IN]    const DCHAR*                                    The session id of the platform response
 * @param state                         [IN]    const DCHAR*                                    Task_State: Task_Check_Successed/Task_Check_Failed
 * @param pstPolicyInfo                 [IN]    const ADM_UIM_SOA_CHECK_HMI_POLICY_INFO_T*      HMI display strategy file information structure.
 * @param reqCode                       [IN]    const D32S reqCode                              hmi请求的errCode
 * @param respCode                      [OUT]   D32S *                                          安卓端接收到通知处理返回给hmi的响应码 0:success non-0:failed
 */
extern void adm_uim_hmi_checkResult_RR(const ADM_APPLICATION_COMMON_CMDID_TYPE_E eCmdidType,const DCHAR *sessionId,const DCHAR *state,
        const ADM_UIM_SOA_CHECK_HMI_POLICY_INFO_T *pstPolicyInfo,const D32S reqCode,D32S *respCode);
typedef void (*adm_uim_hmi_checkResult_RR_func)(const ADM_APPLICATION_COMMON_CMDID_TYPE_E eCmdidType,const DCHAR *sessionId,const DCHAR *state,
        const ADM_UIM_SOA_CHECK_HMI_POLICY_INFO_T *pstPolicyInfo,const D32S reqCode,D32S *respCode);

/**
 * @brief 条件下载结果通知-RR消息通知.
 * 
 * @param eCmdidType                    [IN]    const ADM_SYSI4HMI_CMDID_TYPE_E                 Type of detection trigger.
 * @param sessionId                     [IN]    const DCHAR*                                    The session id of the platform response
 * @param state                         [IN]    const DCHAR*                                    Task_State: Task_Check_Successed/Task_Check_Failed
 * @param condsArrarList                [IN]    const ADM_UIM_SOA_CONDITION_DATA_ARRAY_T*       条件数组.
 * @param verifyResult                  [IN]    const D32S                                      通知结果
 * @param respCode                      [OUT]   D32S *                                          安卓端接收到通知处理返回给hmi的响应码 0:success non-0:failed
 */
extern void adm_uim_hmi_conds_downloadResult_RR(const ADM_APPLICATION_COMMON_CMDID_TYPE_E eCmdidType, const DCHAR* sessionId, const DCHAR* state, 
        const ADM_UIM_SOA_CONDITION_DATA_ARRAY_T* condsArrarList, const D32S verifyResult,D32S *respCode);
typedef void (*adm_uim_hmi_conds_downloadResult_RR_func)(const ADM_APPLICATION_COMMON_CMDID_TYPE_E eCmdidType, const DCHAR* sessionId, const DCHAR* state, 
        const ADM_UIM_SOA_CONDITION_DATA_ARRAY_T* condsArrarList, const D32S verifyResult,D32S *respCode);

/**
 * @brief 下载总结果通知-RR消息通知.
 * 
 * @param eCmdidType                    [IN]    const ADM_SYSI4HMI_CMDID_TYPE_E                 Type of detection trigger.
 * @param sessionId                     [IN]    const DCHAR*                                    The session id of the platform response
 * @param state                         [IN]    const DCHAR*                                    Task_State: Task_Check_Successed/Task_Check_Failed
 * @param pstPolicyDownloadResult       [IN]    const ADM_UIM_SOA_DOWNLOAD_RESULT_INFO_T*       HMI display strategy file information structure.
 * @param respCode                      [OUT]   D32S *                                          安卓端接收到通知处理返回给hmi的响应码 0:success non-0:failed
 */
extern void adm_uim_hmi_policy_downloadResult_RR(const ADM_APPLICATION_COMMON_CMDID_TYPE_E eCmdidType,const DCHAR *sessionId,const DCHAR *state,
        const ADM_UIM_SOA_DOWNLOAD_RESULT_INFO_T *pstPolicyDownloadResult,D32S *respCode);
typedef void (*adm_uim_hmi_policy_downloadResult_RR_func)(const ADM_APPLICATION_COMMON_CMDID_TYPE_E eCmdidType,const DCHAR *sessionId,const DCHAR *state,
        const ADM_UIM_SOA_DOWNLOAD_RESULT_INFO_T *pstPolicyDownloadResult,D32S *respCode);

/**
 * @brief Single ECU下载结果通知-RR消息通知.
 * 
 * @param eCmdidType                    [IN]    const ADM_SYSI4HMI_CMDID_TYPE_E                 Type of detection trigger.
 * @param sessionId                     [IN]    const DCHAR*                                    The session id of the platform response
 * @param state                         [IN]    const DCHAR*                                    Task_State: Task_Check_Successed/Task_Check_Failed
 * @param pstEcuIdenInfo                [IN]    const ADM_UIM_SOA_ECU_IDEN_INFO_T *             Single ECU infomation
 * @param pstEcuDownloadResult          [IN]    const ADM_UIM_SOA_DOWNLOAD_RESULT_INFO_T*       HMI display strategy file information structure.
 * @param respCode                      [OUT]   D32S *                                          安卓端接收到通知处理返回给hmi的响应码 0:success non-0:failed
 */
extern void adm_uim_hmi_ecu_downloadResult_RR(const ADM_APPLICATION_COMMON_CMDID_TYPE_E eCmdidType,const DCHAR *sessionId,const DCHAR *state,
        const ADM_UIM_SOA_ECU_IDEN_INFO_T *pstEcuIdenInfo,const ADM_UIM_SOA_DOWNLOAD_RESULT_INFO_T *pstEcuDownloadResult,D32S *respCode);
typedef void (*adm_uim_hmi_ecu_downloadResult_RR_func)(const ADM_APPLICATION_COMMON_CMDID_TYPE_E eCmdidType,const DCHAR *sessionId,const DCHAR *state,
        const ADM_UIM_SOA_ECU_IDEN_INFO_T *pstEcuIdenInfo,const ADM_UIM_SOA_DOWNLOAD_RESULT_INFO_T *pstEcuDownloadResult,D32S *respCode);

/**
 * @brief ALL ECU 下载结果通知-RR消息通知.
 * 
 * @param eCmdidType                    [IN]    const ADM_SYSI4HMI_CMDID_TYPE_E                                 Type of detection trigger.
 * @param sessionId                     [IN]    const DCHAR*                                                    The session id of the platform response
 * @param state                         [IN]    const DCHAR*                                                    Task_State: Task_Check_Successed/Task_Check_Failed
 * @param pstEcuDownloadResultArrayInfo [IN]    const ADM_UIM_SOA_ECU_DOWNLOAD_RESULT_ARRAY_INFO_T *            ALL ECU infomation
 * @param pstPolicyDownloadResult       [IN]    const ADM_UIM_SOA_DOWNLOAD_RESULT_INFO_T*                       HMI display strategy file information structure.
 * @param respCode                      [OUT]   D32S *                                                          安卓端接收到通知处理返回给hmi的响应码 0:success non-0:failed
 */
extern void adm_uim_hmi_allecu_downloadResult_RR(const ADM_APPLICATION_COMMON_CMDID_TYPE_E eCmdidType,const DCHAR *sessionId,const DCHAR *state,
        const ADM_UIM_SOA_ECU_DOWNLOAD_RESULT_ARRAY_INFO_T *pstEcuDownloadResultArrayInfo,const ADM_UIM_SOA_DOWNLOAD_RESULT_INFO_T *pstPolicyDownloadResult,D32S *respCode);
typedef void (*adm_uim_hmi_allecu_downloadResult_RR_func)(const ADM_APPLICATION_COMMON_CMDID_TYPE_E eCmdidType,const DCHAR *sessionId,const DCHAR *state,
        const ADM_UIM_SOA_ECU_DOWNLOAD_RESULT_ARRAY_INFO_T *pstEcuDownloadResultArrayInfo,const ADM_UIM_SOA_DOWNLOAD_RESULT_INFO_T *pstPolicyDownloadResult,D32S *respCode);

/**
 * @brief 下载完成后，任务已失效通知
 * 
 * @param[in] eCmdidType     描述: 触发类型 \n
 * 范围值: 枚举值 \n
 * 单位: N/A \n
 * 
 * @param[in] sessionId     描述: 会话ID \n
 * 范围值: 指针 \n
 * 单位: N/A \n
 * 
 * @param[out] respCode     描述: 响应码 \n
 * 范围值: 整型指针 \n
 * 单位: N/A \n
 * 
 */
extern void adm_uim_hmi_downloadTaskStop_RR(const ADM_APPLICATION_COMMON_CMDID_TYPE_E eCmdidType,const DCHAR *sessionId,D32S *respCode);
typedef void (*adm_uim_hmi_downloadTaskStop_RR_func)(const ADM_APPLICATION_COMMON_CMDID_TYPE_E eCmdidType, const DCHAR *sessionId, D32S *respCode);

/**
 * @brief 待条件安装结果通知-RR消息通知.
 * 
 * @param eCmdidType                    [IN]    const ADM_SYSI4HMI_CMDID_TYPE_E                                 Type of detection trigger.
 * @param sessionId                     [IN]    const DCHAR*                                                    The session id of the platform response
 * @param state                         [IN]    const DCHAR*                                                    Task_State: Task_Check_Successed/Task_Check_Failed
 * @param condsArrarList                [IN]    const ADM_UIM_SOA_CONDITION_DATA_ARRAY_T*                       条件数组.
 * @param verifyResult                  [IN]    const D32S                                                      安装结果
 * @param respCode                      [OUT]   D32S *                                                          安卓端接收到通知处理返回给hmi的响应码 0:success non-0:failed
 */
typedef void (*adm_uim_hmi_conds_installResult_RR_func)(const ADM_APPLICATION_COMMON_CMDID_TYPE_E eCmdidType, const DCHAR* sessionId, const DCHAR* state, 
        const ADM_UIM_SOA_CONDITION_DATA_ARRAY_T *condsArrarList, const D32S verifyResult,D32S *respCode);
extern void adm_uim_hmi_conds_installResult_RR(const ADM_APPLICATION_COMMON_CMDID_TYPE_E eCmdidType, const DCHAR* sessionId, const DCHAR* state, 
        const ADM_UIM_SOA_CONDITION_DATA_ARRAY_T *condsArrarList, const D32S verifyResult,D32S *respCode);

/**
 * @brief 安装总结果通知-RR消息通知.
 * 
 * @param eCmdidType                    [IN]    const ADM_SYSI4HMI_CMDID_TYPE_E                                 Type of detection trigger.
 * @param sessionId                     [IN]    const DCHAR*                                                    The session id of the platform response
 * @param state                         [IN]    const DCHAR*                                                    Task_State: Task_Check_Successed/Task_Check_Failed
 * @param pstPolicyInstallResult        [IN]    const ADM_UIM_SOA_DOWNLOAD_RESULT_INFO_T*                       HMI display strategy file information structure.
 * @param respCode                      [OUT]   D32S *                                                          安卓端接收到通知处理返回给hmi的响应码 0:success non-0:failed
 */
extern void adm_uim_hmi_policy_installResult_RR(const ADM_APPLICATION_COMMON_CMDID_TYPE_E eCmdidType,const DCHAR *sessionId,const DCHAR *state,
        const ADM_UIM_SOA_INSTALL_RESULT_INFO_T *pstPolicyInstallResult,D32S *respCode);
typedef void (*adm_uim_hmi_policy_installResult_RR_func)(const ADM_APPLICATION_COMMON_CMDID_TYPE_E eCmdidType,const DCHAR *sessionId,const DCHAR *state,
        const ADM_UIM_SOA_INSTALL_RESULT_INFO_T *pstPolicyInstallResult,D32S *respCode);

extern void adm_uim_hmi_repeat_policy_installResult_RR(const ADM_APPLICATION_COMMON_CMDID_TYPE_E eCmdidType,const DCHAR *sessionId,const DCHAR *state,
        const ADM_UIM_SOA_INSTALL_RESULT_INFO_T *pstPolicyInstallResult,D32S *respCode);
/**
 * @brief 安装阶段性结果通知-RR消息通知.
 * 
 * @param eCmdidType                    [IN]    const ADM_SYSI4HMI_CMDID_TYPE_E                                 Type of detection trigger.
 * @param sessionId                     [IN]    const DCHAR*                                                    The session id of the platform response
 * @param state                         [IN]    const DCHAR*                                                    Task_State: Task_Check_Successed/Task_Check_Failed
 * @param pstPolicyInstallResult        [IN]    const ADM_UIM_SOA_DOWNLOAD_RESULT_INFO_T*                       HMI display strategy file information structure.
 * @param respCode                      [OUT]   D32S *                                                          安卓端接收到通知处理返回给hmi的响应码 0:success non-0:failed
 */
extern void adm_uim_hmi_stage_installResult_RR(const ADM_APPLICATION_COMMON_CMDID_TYPE_E eCmdidType,const DCHAR *sessionId,const DCHAR *state,
        const ADM_UIM_SOA_INSTALL_RESULT_INFO_T *pstPolicyInstallResult,D32S *respCode);
typedef void (*adm_uim_hmi_stage_installResult_RR_func)(const ADM_APPLICATION_COMMON_CMDID_TYPE_E eCmdidType,const DCHAR *sessionId,const DCHAR *state,
        const ADM_UIM_SOA_INSTALL_RESULT_INFO_T *pstPolicyInstallResult,D32S *respCode);

/**
 * @brief Single ECU 安装结果通知-RR消息通知.
 * 
 * @param eCmdidType                    [IN]    const ADM_SYSI4HMI_CMDID_TYPE_E                                 Type of detection trigger.
 * @param sessionId                     [IN]    const DCHAR*                                                    The session id of the platform response
 * @param state                         [IN]    const DCHAR*                                                    Task_State: Task_Check_Successed/Task_Check_Failed
 * @param pstEcuIdenInfo                [IN]    const ADM_UIM_SOA_ECU_IDEN_INFO_T *                             Single ECU infomation
 * @param pstEcuInstallResultInfo       [IN]    const ADM_UIM_SOA_INSTALL_RESULT_INFO_T*                        ecu对应安装结果信息.
 * @param respCode                      [OUT]   D32S *                                                          安卓端接收到通知处理返回给hmi的响应码 0:success non-0:failed
 */
extern void adm_uim_hmi_ecu_installResult_RR(const ADM_APPLICATION_COMMON_CMDID_TYPE_E eCmdidType,const DCHAR *sessionId,const DCHAR *state,
        const ADM_UIM_SOA_ECU_IDEN_INFO_T *pstEcuIdenInfo,const ADM_UIM_SOA_INSTALL_RESULT_INFO_T *pstEcuInstallResultInfo,D32S *respCode);
typedef void (*adm_uim_hmi_ecu_installResult_RR_func)(const ADM_APPLICATION_COMMON_CMDID_TYPE_E eCmdidType,const DCHAR *sessionId,const DCHAR *state,
        const ADM_UIM_SOA_ECU_IDEN_INFO_T *pstEcuIdenInfo,const ADM_UIM_SOA_INSTALL_RESULT_INFO_T *pstEcuInstallResultInfo,D32S *respCode);

/**
 * @brief ALL ECU 安装结果通知-RR消息通知.
 * 
 * @param eCmdidType                    [IN]    const ADM_SYSI4HMI_CMDID_TYPE_E                                 Type of detection trigger.
 * @param sessionId                     [IN]    const DCHAR*                                                    The session id of the platform response
 * @param state                         [IN]    const DCHAR*                                                    Task_State: Task_Check_Successed/Task_Check_Failed
 * @param pstEcuInstallResultArrayInfo  [IN]    const ADM_UIM_SOA_ECU_INSTALL_RESULT_ARRAY_INFO_T *             ALL ECU infomation
 * @param pstPolicyInstallResult        [IN]    const ADM_UIM_SOA_INSTALL_RESULT_INFO_T *                       各ecu对应安装结果.
 * @param respCode                      [OUT]   D32S *                                                          安卓端接收到通知处理返回给hmi的响应码 0:success non-0:failed
 */
extern void adm_uim_hmi_allecu_installResult_RR(const ADM_APPLICATION_COMMON_CMDID_TYPE_E eCmdidType,const DCHAR *sessionId,const DCHAR *state,
        const ADM_UIM_SOA_ECU_INSTALL_RESULT_ARRAY_INFO_T *pstEcuInstallResultArrayInfo,const ADM_UIM_SOA_INSTALL_RESULT_INFO_T *pstPolicyInstallResult,D32S *respCode);
typedef void (*adm_uim_hmi_allecu_installResult_RR_func)(const ADM_APPLICATION_COMMON_CMDID_TYPE_E eCmdidType,const DCHAR *sessionId,const DCHAR *state,
        const ADM_UIM_SOA_ECU_INSTALL_RESULT_ARRAY_INFO_T *pstEcuInstallResultArrayInfo,const ADM_UIM_SOA_INSTALL_RESULT_INFO_T *pstPolicyInstallResult,D32S *respCode);

#ifdef __cplusplus  // close out "C" linkage in case of c++ compiling
}
#endif

#endif//__ADM_UIM_CHECK_H__
