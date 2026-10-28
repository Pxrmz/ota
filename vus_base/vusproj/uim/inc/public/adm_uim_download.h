/**
 * @file adm_uim_download.h
 * @brief 为下载流程定义的接口，包括：触发下载、下载进度反馈和查询、下载结果反馈和查询等。
 * @author Mengwei
 * @version v0.1.0
 * @date 2021-05-07
 * @copyright Copyright © 2018 All rights 上海艾拉比智能科技有限公司
 */
#ifndef __ADM_UIM_DOWNLOAD_H__
#define __ADM_UIM_DOWNLOAD_H__

#ifdef __cplusplus
extern "C" {
#endif

#define EXPORT_FUN __attribute__((visibility("default")))

#include "adm_typedefs.h"
#include "adm_uim_common.h"

/***************************************************************************Trigger download*********************************************************************************/
/**
 * @brief User click to trigger download callback processing function.
 * 
 * @param eActionType                       [OUT]       ADM_UIM_SOA_ACTION_AUTH_TYPE_E*     Whether the requested trigger action is allowed
 * @param priv                              [IN/OUT]    void*                               Reserved parameters
 */
typedef void (*ADM_UIM_DOWNLOAD_TRIGGER_CALLBACK_F)(ADM_UIM_SOA_ACTION_AUTH_TYPE_E* eActionType, void* priv);
/***************************************************************************Trigger download*********************************************************************************/

/**
 *@brief 自动下载的开关打开或者关闭
 *@param eSwitchType [IN] 		ADM_UIM_SOA_AUTO_DOWNLOAD_TYPE_E 自动下载的开关 0：关 1：开
 *@param errCode	 [OUT]		D32S*							 响应码 0：成功 non-0:失败
*/
typedef void (*ADM_UIM_DOWNLOAD_AUTO_CALLBACK_F)(ADM_UIM_SOA_AUTO_DOWNLOAD_TYPE_E eSwitchType, D32S *errorCode);

/***************************************************************************Cancel download*********************************************************************************/
/**
 * @brief User clicks to trigger to cancel download.
 * 
 * @param eCmdidType        [IN]    const ADM_APPLICATION_COMMON_CMDID_TYPE_E    Type of detection trigger.
 * @param eActionType       [OUT]   ADM_SYSI4HMI_ACTION_AUTH_TYPE_E*    Whether the requested trigger action is allowed
 * @param errorCode         [OUT]   D32S*   Result of cancel download
 * @param priv              [IN/OUT]    void*                               Reserved parameters
 */
typedef void (*ADM_UIM_DOWNLOAD_CALCEL_CALLBACK_F)(const ADM_APPLICATION_COMMON_CMDID_TYPE_E eCmdidType, ADM_UIM_SOA_ACTION_AUTH_TYPE_E* eActionType, D32S* errorCode, void* priv);
/***************************************************************************Cancel download*********************************************************************************/

/***************************************************************************Pause download*********************************************************************************/
/**
 * @brief User clicks to trigger to pause download.
 * 
 * @param eCmdidType        [IN]    const ADM_APPLICATION_COMMON_CMDID_TYPE_E    Type of detection trigger.
 * @param eActionType       [OUT]   ADM_SYSI4HMI_ACTION_AUTH_TYPE_E*    Whether the requested trigger action is allowed
 * @param errorCode         [OUT]   D32S*   Result of pause download
 * @param priv              [IN/OUT]    void*                               Reserved parameters
 */
typedef void (*ADM_UIM_DOWNLOAD_PAUSED_CALLBACK_F)(const ADM_APPLICATION_COMMON_CMDID_TYPE_E eCmdidType, ADM_UIM_SOA_ACTION_AUTH_TYPE_E* eActionType, D32S* errorCode, void* priv);
/***************************************************************************Pause download*********************************************************************************/

/***************************************************************************Trigger verify conditions************************************************************************/
/**
 * @brief Trigger condition detection
 * 
 * @param eVerifiedType     [IN]    const ADM_UIM_SOA_CONDITIONS_VERIFIED_TYPE_E       Types of condition detection
 * @param cycleTime         [IN]    const D16U  Condition detection cycle, unit: Second, when everifiedType = ADM_UIM_SOA_DOWNLOAD_CONDITIONS_VERIFIED_TYPE_CYCLE
 * @param timeout           [IN]    const D16U  Period detection condition timeout, unit:Second, when everifiedType = ADM_UIM_SOA_DOWNLOAD_CONDITIONS_VERIFIED_TYPE_CYCLE
 * @param errorCode         [OUT]   D32S*   Error code of triggering verify conditions.
 */
typedef void (*ADM_UIM_DOWNLOAD_CONDITIONS_VERIFIED_CALLBACK_F)(const ADM_UIM_SOA_CONDITIONS_VERIFIED_TYPE_E eVerifiedType, const D16U cycleTime, const D16U timeout, D32S* errorCode);

// TODO: error code to be defined
/**
 * @brief Notification of verification results of download conditions.
 * 
 * @param eCmdidType        [IN]    const ADM_APPLICATION_COMMON_CMDID_TYPE_E    Type of detection trigger.
 * @param sessionId         [IN]    const DCHAR*        The sessionId of the strategy
 * @param state             [IN]    const DCHAR*    Task_State: Task_Download_Prepare
 * @param condsArrarList    [IN]    const ADM_UIM_SOA_CONDITION_DATA_ARRAY_T*   Condition value data array 
 * @param verifyResult      [IN]    const D32S  Conditional check result, 0:successed, Non-0: error code
 */
extern void adm_uim_downloadConditionsVerifyResultNotify(const ADM_APPLICATION_COMMON_CMDID_TYPE_E eCmdidType, const DCHAR* sessionId, const DCHAR* state, const ADM_UIM_SOA_CONDITION_DATA_ARRAY_T* condsArrarList, const D32S verifyResult);
typedef void (*adm_uim_downloadConditionsVerifyResultNotify_func)(const ADM_APPLICATION_COMMON_CMDID_TYPE_E eCmdidType, const DCHAR* sessionId, const DCHAR* state, const ADM_UIM_SOA_CONDITION_DATA_ARRAY_T* condsArrarList, const D32S verifyResult);

/***************************************************************************Trigger verify conditions************************************************************************/

/***************************************************************************Notify policy download progress******************************************************************/
/**
 * @brief Definition of the progress information structure of the upgrade package download.
 * 
 */
typedef struct {
    D64S    savedSize;                                          /**!< The size of the downloaded upgrade package, unit:byte. */
    D64S    totalSize;                                          /**!< The total size of the upgrade package being downloaded, unit:byte */
    D32S    speed;                                              /**!< Download speed of upgrade package, unit:byte/s */
    D32S    remainTime;                                         /**!< Estimated remaining time to download, unit:second */
    D32S    iTotalProgress;                                     /** 下载总进度 */
} ADM_UIM_SOA_DOWNLOAD_PROGRESS_INFO_T;


typedef struct 
{
    D32S eCmdidType;
    const DCHAR* sessionId;
    const DCHAR* state;
    const ADM_UIM_SOA_ECU_IDEN_INFO_T* ecuIdenInfo;                   
    const ADM_UIM_SOA_DOWNLOAD_PROGRESS_INFO_T* ecuDownloadProgress;
} ADM_UIM_DOWNLOAD_PROGRESS_ECU_INFO_T;

typedef struct 
{
    D32S eCmdidType;
    const DCHAR* sessionId;
    const DCHAR* state;
    const ADM_UIM_SOA_DOWNLOAD_PROGRESS_INFO_T* policyDownloadProgress;
} ADM_UIM_DOWNLOAD_PROGRESS_ALL_INFO_T;

/**
 * @brief Policy download progress notification.
 * 
 * @param eCmdidType        [IN]    const ADM_APPLICATION_COMMON_CMDID_TYPE_E    Type of detection trigger.
 * @param sessionId         [IN]    const DCHAR*        The sessionId of the strategy
 * @param state             [IN]    const DCHAR*    Task_State: Task_Download_Ing
 * @param pstPolicyDownloadProgress [IN]    const ADM_UIM_SOA_DOWNLOAD_PROGRESS_INFO_T*     The progress information structure of the upgrade package download.
 */
extern void adm_uim_policyDownloadProgressNotify(const ADM_APPLICATION_COMMON_CMDID_TYPE_E eCmdidType, const DCHAR* sessionId, const DCHAR* state, const ADM_UIM_SOA_DOWNLOAD_PROGRESS_INFO_T* pstPolicyDownloadProgress);
typedef void (*adm_uim_policyDownloadProgressNotify_func)(const ADM_APPLICATION_COMMON_CMDID_TYPE_E eCmdidType, const DCHAR* sessionId, const DCHAR* state, const ADM_UIM_SOA_DOWNLOAD_PROGRESS_INFO_T* pstPolicyDownloadProgress);

/***************************************************************************Notify policy download progress******************************************************************/

/***************************************************************************Notify policy download result********************************************************************/
/**
 * @brief Definition of the result information structure of the upgrade package download.
 * 
 */
typedef struct {
    D32S result;                                                /**!< The result of the remote module downloading the upgrade package */
	D32S existPrivatePolicy;									/**!< The privatePolicy is or not exist*/
	DCHAR*  expectVal;                                          /**!< Description of the reason for the failure, expected value */
    DCHAR*  realVal;                                            /**!< Description of the reason for the failure, real value */
} ADM_UIM_SOA_DOWNLOAD_RESULT_INFO_T;

typedef struct 
{
    D32S eCmdidType;
    const DCHAR* sessionId;
    const DCHAR* state;
    const ADM_UIM_SOA_ECU_IDEN_INFO_T* ecuIdenInfo;                   
    const ADM_UIM_SOA_DOWNLOAD_RESULT_INFO_T* ecuDownloadResult;
} ADM_UIM_DOWNLOAD_RESULT_ECU_INFO_T;

typedef struct 
{
    D32S eCmdidType;
    const DCHAR* sessionId;
    const DCHAR* state;
    const ADM_UIM_SOA_DOWNLOAD_RESULT_INFO_T* policyDownloadResult;
} ADM_UIM_DOWNLOAD_RESULT_ALL_INFO_T;

/**
 * @brief Policy download result notification.
 * 
 * @param eCmdidType        [IN]    const ADM_APPLICATION_COMMON_CMDID_TYPE_E    Type of detection trigger.
 * @param sessionId         [IN]    const DCHAR*        The sessionId of the strategy
 * @param state             [IN]    const DCHAR*    Task_State: Task_Download_Successed/Task_Download_Failed/Task_Download_Paused/Task_Download_Completed
 * @param pstPolicyDownloadResult   [IN]    const ADM_UIM_SOA_DOWNLOAD_RESULT_INFO_T*   The result information structure of the upgrade package download.
 * @return D32S             0:successed, Non-0:failed  
 */
typedef D32S (*adm_uim_policyDownloadResultNotify_func)(const ADM_APPLICATION_COMMON_CMDID_TYPE_E eCmdidType, const DCHAR* sessionId, const DCHAR* state, const ADM_UIM_SOA_DOWNLOAD_RESULT_INFO_T* pstPolicyDownloadResult);
extern D32S adm_uim_policyDownloadResultNotify(const ADM_APPLICATION_COMMON_CMDID_TYPE_E eCmdidType, const DCHAR* sessionId, const DCHAR* state, const ADM_UIM_SOA_DOWNLOAD_RESULT_INFO_T* pstPolicyDownloadResult);
/***************************************************************************Notify policy download result********************************************************************/

/**
 * @brief Identifies the download progress information of the ECU
 * 
 */
typedef struct {
    ADM_UIM_SOA_ECU_IDEN_INFO_T* pstEcuIdenInfo;                    /**!< Identification information of ECU upgrade package download */
    ADM_UIM_SOA_DOWNLOAD_PROGRESS_INFO_T* pstEcuDownloadProgress;   /**!< The progress information structure of the upgrade package download. */
} ADM_UIM_SOA_ECU_DOWNLOAD_PROGRESS_INFO_T;

/**
 * @brief ECU progress information array structure definition
 * 
 */
typedef struct {
    ADM_UIM_SOA_ECU_DOWNLOAD_PROGRESS_INFO_T** ecuDownloadProgressArray;    /**!< ECU progress information array */
    D16U    ecuDownloadProgressArrayLength;                                 /**!< Length of ECU progress information array */
} ADM_UIM_SOA_ECU_DOWNLOAD_PROGRESS_ARRAY_INFO_T;

typedef struct
{
    const ADM_UIM_SOA_ECU_DOWNLOAD_PROGRESS_ARRAY_INFO_T *pstEcuDownloadProgressArrayInfo;
    const ADM_UIM_SOA_DOWNLOAD_PROGRESS_INFO_T *pstPolicyDownloadProgress;
    ADM_APPLICATION_COMMON_CMDID_TYPE_E eCmdidType;    
    const DCHAR *sessionId;
    const DCHAR *state;
} ADM_UIM_DOWNLOAD_PROGRESS_BOTH_INFO_T ;

/**
 * @brief Identifies the download result information of the ECU
 * 
 */
typedef struct {
    ADM_UIM_SOA_ECU_IDEN_INFO_T* pstEcuIdenInfo;                    /**!< Identification information of ECU upgrade package download */
    DCHAR*  state;                                                  /**!< Ecu_state: Ecu_Download_Finished*/
    ADM_UIM_SOA_DOWNLOAD_RESULT_INFO_T* pstEcuDownloadResult;       /**!< The result information structure of the upgrade package download. */
} ADM_UIM_SOA_ECU_DOWNLOAD_RESULT_INFO_T;



typedef struct {
    ADM_UIM_SOA_ECU_DOWNLOAD_RESULT_INFO_T** ecuDownloadResultArray;    /**!< ECU result information array */
    D16U    ecuDownloadResultArrayLength;                               /**!< Length of ECU result information array */
} ADM_UIM_SOA_ECU_DOWNLOAD_RESULT_ARRAY_INFO_T;

typedef struct
{
    const ADM_UIM_SOA_ECU_DOWNLOAD_RESULT_ARRAY_INFO_T *pstEcuDownloadResultArrayInfo;
    const ADM_UIM_SOA_DOWNLOAD_RESULT_INFO_T *pDownloadResult;
    ADM_APPLICATION_COMMON_CMDID_TYPE_E eCmdidType;    
    const DCHAR *sessionId;
    const DCHAR *state;
    D32S errorCode;
} ADM_UIM_DOWNLOAD_RESULT_BOTH_INFO_T ;

/***************************************************************************Notify one ECU download progress & result ********************************************************************/
/**
 * @brief Ecu download progress notification.
 * 
 * @param eCmdidType        [IN]    const ADM_APPLICATION_COMMON_CMDID_TYPE_E    Type of detection trigger.
 * @param sessionId         [IN]    const DCHAR*        The sessionId of the strategy
 * @param state             [IN]    const DCHAR*        The state of Task_State: Task_Download_Ing
 * @param pstEcuIdenInfo    [IN]    const ADM_UIM_SOA_ECU_IDEN_INFO_T*  Identification information of ECU upgrade package download
 * @param pstEcuDownloadProgress    [IN]    const ADM_UIM_SOA_DOWNLOAD_PROGRESS_INFO_T*  Download progress of ECU.
 * @return D32S             0:successed, Non-0:failed
 */
typedef D32S (*adm_uim_ecuDownloadProgressNotify_func)(const ADM_APPLICATION_COMMON_CMDID_TYPE_E eCmdidType, const DCHAR* sessionId, const DCHAR* state, const ADM_UIM_SOA_ECU_IDEN_INFO_T* pstEcuIdenInfo, const ADM_UIM_SOA_DOWNLOAD_PROGRESS_INFO_T* pstEcuDownloadProgress);
extern D32S adm_uim_ecuDownloadProgressNotify(const ADM_APPLICATION_COMMON_CMDID_TYPE_E eCmdidType, const DCHAR* sessionId, const DCHAR* state, const ADM_UIM_SOA_ECU_IDEN_INFO_T* pstEcuIdenInfo, const ADM_UIM_SOA_DOWNLOAD_PROGRESS_INFO_T* pstEcuDownloadProgress);

/**
 * @brief Ecu download result notification.
 * 
 * @param eCmdidType        [IN]    const ADM_APPLICATION_COMMON_CMDID_TYPE_E    Type of detection trigger.
 * @param sessionId         [IN]    const DCHAR*        The sessionId of the strategy
 * @param state             [IN]    const DCHAR*        The state of Task_State: Task_Download_Ing
 * @param pstEcuIdenInfo    [IN]    const ADM_UIM_SOA_ECU_IDEN_INFO_T*  Identification information of ECU upgrade package download
 * @param pstEcuDownloadResult  [IN]    const ADM_UIM_SOA_DOWNLOAD_RESULT_INFO_T*   Identifies the download result information of the ECU.
 * @return D32S             0:successed, Non-0:failed 
 */
typedef D32S (*adm_uim_ecuDownloadResultNotify_func)(const ADM_APPLICATION_COMMON_CMDID_TYPE_E eCmdidType, const DCHAR* sessionId, const DCHAR* state, const ADM_UIM_SOA_ECU_IDEN_INFO_T* pstEcuIdenInfo, const ADM_UIM_SOA_DOWNLOAD_RESULT_INFO_T* pstEcuDownloadResult);
extern D32S adm_uim_ecuDownloadResultNotify(const ADM_APPLICATION_COMMON_CMDID_TYPE_E eCmdidType, const DCHAR* sessionId, const DCHAR* state, const ADM_UIM_SOA_ECU_IDEN_INFO_T* pstEcuIdenInfo, const ADM_UIM_SOA_DOWNLOAD_RESULT_INFO_T* pstEcuDownloadResult);

/***************************************************************************Notify one ECU download progress & result ********************************************************************/

/***************************************************************************Notify all ECU download progress & result ********************************************************************/
/**
 * @brief All Ecu download progress notification.
 * 
 * @param eCmdidType        [IN]    const ADM_APPLICATION_COMMON_CMDID_TYPE_E    Type of detection trigger.
 * @param sessionId         [IN]    const DCHAR*          The sessionId of the strategy
 * @param state             [IN]    const DCHAR*        Task_State: Task_Download_Ing  
 * @param pstEcuDownloadProgressArrayInfo   [IN] const ADM_UIM_SOA_ECU_DOWNLOAD_PROGRESS_ARRAY_INFO_T*  ECU progress information array structure.
 * @param pstPolicyDownloadProgress [IN]    const ADM_UIM_SOA_DOWNLOAD_PROGRESS_INFO_T*     The progress information structure of the upgrade package download.
 * @return D32S             0:successed, Non-0:failed 
 */
typedef D32S (*adm_uim_allEcuDownloadProgressNotify_func)(const ADM_APPLICATION_COMMON_CMDID_TYPE_E eCmdidType, const DCHAR* sessionId, const DCHAR* state, const ADM_UIM_SOA_ECU_DOWNLOAD_PROGRESS_ARRAY_INFO_T* pstEcuDownloadProgressArrayInfo, const ADM_UIM_SOA_DOWNLOAD_PROGRESS_INFO_T* pstPolicyDownloadProgress);
extern D32S adm_uim_allEcuDownloadProgressNotify(const ADM_APPLICATION_COMMON_CMDID_TYPE_E eCmdidType, const DCHAR* sessionId, const DCHAR* state, const ADM_UIM_SOA_ECU_DOWNLOAD_PROGRESS_ARRAY_INFO_T* pstEcuDownloadProgressArrayInfo, const ADM_UIM_SOA_DOWNLOAD_PROGRESS_INFO_T* pstPolicyDownloadProgress);


/**
 * @brief All Ecu download result notification.
 * 
 * @param eCmdidType        [IN]    const ADM_APPLICATION_COMMON_CMDID_TYPE_E    Type of detection trigger.
 * @param sessionId         [IN]    const DCHAR*          The sessionId of the strategy
 * @param state             [IN]    const DCHAR*        Task_State: Task_Download_Paused/Task_Download_Completed/Task_Download_Successed/Task_Download_Failed/Task_Transfer_xxx/Task_Unpack_xxx
 * @param pstEcuDownloadResultArrayInfo     [IN] const ADM_UIM_SOA_ECU_DOWNLOAD_RESULT_ARRAY_INFO_T*    Ecu result information array structure.
 * @param pstPolicyDownloadResult   [IN]    const ADM_UIM_SOA_DOWNLOAD_RESULT_INFO_T*   The result information structure of the upgrade package download.
 * @return D32S             0:successed, Non-0:failed
 */
typedef D32S (*adm_uim_allEcuDownloadResultNotify_func)(const ADM_APPLICATION_COMMON_CMDID_TYPE_E eCmdidType, const DCHAR* sessionId, const DCHAR* state, const ADM_UIM_SOA_ECU_DOWNLOAD_RESULT_ARRAY_INFO_T* pstEcuDownloadResultArrayInfo, const ADM_UIM_SOA_DOWNLOAD_RESULT_INFO_T* pstPolicyDownloadResult);
extern D32S adm_uim_allEcuDownloadResultNotify(const ADM_APPLICATION_COMMON_CMDID_TYPE_E eCmdidType, const DCHAR* sessionId, const DCHAR* state, const ADM_UIM_SOA_ECU_DOWNLOAD_RESULT_ARRAY_INFO_T* pstEcuDownloadResultArrayInfo, const ADM_UIM_SOA_DOWNLOAD_RESULT_INFO_T* pstPolicyDownloadResult);

/***************************************************************************Notify all ECU download progress & result ********************************************************************/

/***************************************************************************Query all ECU download progress & result *********************************************************************/
/**
 * @brief Request to query the download progress of all current ECUs.
 * 
 * @param eCmdidType        [OUT]    ADM_APPLICATION_COMMON_CMDID_TYPE_E*    Type of detection trigger.
 * @param sessionId         [OUT]    DCHAR**          The sessionId of the strategy
 * @param state             [OUT]    DCHAR**        Task_State: Task_Download_Ing  
 * @param pstEcuDownloadProgressArrayInfo   [OUT] ADM_UIM_SOA_ECU_DOWNLOAD_PROGRESS_ARRAY_INFO_T**  ECU progress information array structure.
 * @param pstPolicyDownloadProgress [OUT]    ADM_UIM_SOA_DOWNLOAD_PROGRESS_INFO_T**     The progress information structure of the upgrade package download.
 * @param errorCode         [OUT]   D32S*   Error code
 * @param priv                              [IN/OUT]    void*                               Reserved parameters
 * @return  D32S                            0:successed, Non-0:failed
 */
typedef D32S (*ADM_UIM_DOWNLOAD_QUERY_ALL_ECU_PROGRESS_CALLBACK_F)(ADM_APPLICATION_COMMON_CMDID_TYPE_E* eCmdidType, DCHAR** sessionId, DCHAR** state, ADM_UIM_SOA_ECU_DOWNLOAD_PROGRESS_ARRAY_INFO_T** pstEcuDownloadProgressArrayInfo, ADM_UIM_SOA_DOWNLOAD_PROGRESS_INFO_T** pstPolicyDownloadProgress, D32S* errorCode, void* priv);

/**
 * @brief Request to query the download result of all current ECUs.
 * 
 * @param eCmdidType        [OUT]    ADM_APPLICATION_COMMON_CMDID_TYPE_E*    Type of detection trigger.
 * @param sessionId         [OUT]    DCHAR**          The sessionId of the strategy
 * @param state             [OUT]    DCHAR**        Task_State: Task_Download_Paused/Task_Download_Completed/Task_Download_Successed/Task_Download_Failed/Task_Transfer_xxx/Task_Unpack_xxx
 * @param pstEcuDownloadResultArrayInfo     [OUT] ADM_UIM_SOA_ECU_DOWNLOAD_RESULT_ARRAY_INFO_T**    Ecu result information array structure.
 * @param pstPolicyDownloadResult   [OUT]    ADM_UIM_SOA_DOWNLOAD_RESULT_INFO_T**   The result information structure of the upgrade package download.
 * @param errorCode         [OUT]   D32S*   Error code
 * @param priv                              [IN/OUT]    void*                               Reserved parameters
 * @return  D32S                            0:successed, Non-0:failed
 */
typedef D32S (*ADM_UIM_DOWNLOAD_QUERY_ALL_ECU_RESULT_CALLBACK_F)(ADM_APPLICATION_COMMON_CMDID_TYPE_E* eCmdidType, DCHAR** sessionId, DCHAR** state, ADM_UIM_SOA_ECU_DOWNLOAD_RESULT_ARRAY_INFO_T** pstEcuDownloadResultArrayInfo, ADM_UIM_SOA_DOWNLOAD_RESULT_INFO_T** pstPolicyDownloadResult, D32S* errorCode, void* priv);
/***************************************************************************Query all ECU download progress & result *********************************************************************/


/***************************************************************************SOA-UIM-DOWNLOAD init*****************************************************************************************/
/**
 * @brief The UIM-SOA-DOWNLOAD initialization structure definition
 * 
 */
typedef struct ADM_UIM_DOWNLOAD_INIT_INFO{
    ADM_UIM_DOWNLOAD_TRIGGER_CALLBACK_F downloadTriggerCallback;                /**!< User click to trigger download callback processing function. */
	ADM_UIM_DOWNLOAD_AUTO_CALLBACK_F autoDownloadSwitchSetCallback;             /**!< User click to auto download switch callback processing function. */
    ADM_UIM_DOWNLOAD_CALCEL_CALLBACK_F  downloadCancelCallback;                 /**!< User clicks to trigger to cancel download. */
    ADM_UIM_DOWNLOAD_PAUSED_CALLBACK_F  downloadPauseCallback;                  /**!< User clicks to trigger to pause download. */
    ADM_UIM_DOWNLOAD_CONDITIONS_VERIFIED_CALLBACK_F conditionsVerifiedCallback; /**!< Trigger condition detection */
    ADM_UIM_DOWNLOAD_QUERY_ALL_ECU_PROGRESS_CALLBACK_F allEcuDownloadProgressCallback;  /**!< Request to query the download progress of all current ECUs. */
    ADM_UIM_DOWNLOAD_QUERY_ALL_ECU_RESULT_CALLBACK_F allEcuDownloadResultCallback;      /**!< Request to query the download result of all current ECUs. */
} ADM_UIM_DOWNLOAD_INIT_INFO_T;

/**
 * @brief The SOA-UIM-DOWNLOAD module initializes the interface
 * @note Use adm_uim_downloadDeinit to release the initialization information
 * 
 * @param pstInitInfo   [IN]        const ADM_UIM_SOA_DOWNLOAD_INIT_INFO_T*        The UIM-SOA-DOWNLOAD initialization structure
 * @param priv          [IN/OUT]    void*                               Reserved parameters
 * @return D32S         0:successed, Non-0:failed    
 */
typedef D32S (*adm_uim_downloadInit_func)(const ADM_UIM_DOWNLOAD_INIT_INFO_T* initInfo, void* priv);
extern D32S adm_uim_downloadInit(const ADM_UIM_DOWNLOAD_INIT_INFO_T* initInfo, void* priv);

/**
 * @brief Release initialization information
 * @note Used to release the information initialized by the adm_uim_downloadInit function
 */
typedef D32S (*adm_uim_downloadDeinit_func)(void);
extern D32S adm_uim_downloadDeinit(void);

/***************************************************************************SOA-UIM-DOWNLOAD init*****************************************************************************************/

#ifdef __cplusplus  // close out "C" linkage in case of c++ compiling
}
#endif

#endif//__ADM_UIM_DOWNLOAD_H__
