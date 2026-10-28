/**
 * @file adm_uim_install.h
 * @brief 为安装流程定义的接口，包括：触发安装、安装进度的反馈和查询、安装结果的反馈和查询等。
 * @author Mengwei
 * @version v0.1.0
 * @date 2021-05-07
 * @copyright Copyright © 2018 All rights 上海艾拉比智能科技有限公司
 */
#ifndef __ADM_UIM_INSTALL_H__
#define __ADM_UIM_INSTALL_H__

#ifdef __cplusplus
extern "C" {
#endif

#define EXPORT_FUN __attribute__((visibility("default")))

#include "adm_typedefs.h"
#include "adm_uim_common.h"
#include "adm_application_common.h"

/***************************************************************************Schdule install*********************************************************************************/


/**
 * @brief The user triggers the setting of the scheduled installation time.
 * @note The appointment time set by the user is given to the host device and is reduced by 15 minutes by default to remind the user
 * 
 * @param eCmdidType        [IN]    const ADM_APPLICATION_COMMON_CMDID_TYPE_E    Type of detection trigger.
 * @param eSource           [IN]    const ADM_APPLICATION_SCHEDULE_INSTALL_SOURCE_E    Source of detection trigger.
 * @param eSchduleType      [IN]    const ADM_UIM_SOA_INSTALL_SCHDULE_TYPE_E    Type of schdule install.
 * @param schduleTime       [IN]    const D16U  Schdule install timestamp, unit: second
 * @param eActionType       [OUT]   ADM_SYSI4HMI_ACTION_AUTH_TYPE_E*    Whether the requested trigger action is allowed
 * @param priv                              [IN/OUT]    D32S*                               Reserved parameters
 */
typedef D32S (*ADM_UIM_INSTALL_SCHDULED_CALLBACK_F)(const ADM_APPLICATION_COMMON_CMDID_TYPE_E eCmdidType, 
    const ADM_APPLICATION_SCHEDULE_INSTALL_SOURCE_E eSource, const ADM_UIM_SOA_INSTALL_SCHDULE_TYPE_E eSchduleType, 
    const D64S schduleTime, ADM_UIM_SOA_ACTION_AUTH_TYPE_E* eActionType, void* priv);

/**
 * @brief 15 minutes before the appointment time arrives, inform the HMI that the appointment time has arrived
 * 
 */
typedef void (*adm_uim_schduleTimeArrived_func)(DBOOL bCondSatisfy, DBOOL bIsTaskStop);
extern void adm_uim_schduleTimeArrived(DBOOL bCondSatisfy, DBOOL bIsTaskStop);



/**
 * @brief TSP 推送设置的预约升级时间
 * 
 */
typedef void (*adm_uim_tspSchduleTimestamp_func)(ADM_UIM_SOA_TSP_OPERATION_TYPE_E eOperationType, D64S appointmentTimestamp);
extern void adm_uim_tspSchduleTime(ADM_UIM_SOA_TSP_OPERATION_TYPE_E eOperationType, D64S appointmentTimestamp);
/***************************************************************************Schdule install*********************************************************************************/

/***************************************************************************Cancel install*********************************************************************************/
/**
 * @brief User triggers to cancel installation.
 * 
 * @param eCmdidType                        [IN]    const ADM_SYSI4HMI_CMDID_TYPE_E    Type of detection trigger.
 * @param eActionType       [OUT]   ADM_SYSI4HMI_ACTION_AUTH_TYPE_E*    Whether the requested trigger action is allowed
 * @param priv                              [IN/OUT]    void*                               Reserved parameters
 */
typedef void (*ADM_UIM_INSTALL_CANCELD_CALLBACK_F)(const ADM_APPLICATION_COMMON_CMDID_TYPE_E eCmdidType, ADM_UIM_SOA_ACTION_AUTH_TYPE_E* eActionType, void* priv);
/***************************************************************************Cancel install*********************************************************************************/


/***************************************************************************Refuse install*********************************************************************************/
/**
 * @brief User triggers to refuse install, when the user cancels the installation multiple times
 * 
 * @param eCmdidType        [IN]    const ADM_APPLICATION_COMMON_CMDID_TYPE_E     Type of detection trigger.
 * @param eActionType       [OUT]   ADM_UIM_SOA_ACTION_AUTH_TYPE_E*    Whether the requested trigger action is allowed
 */
typedef void (*ADM_UIM_INSTALL_REFUSED_CALLBACK_F)(const ADM_APPLICATION_COMMON_CMDID_TYPE_E eCmdidType, ADM_UIM_SOA_ACTION_AUTH_TYPE_E* eActionType);
/***************************************************************************Refuse install*********************************************************************************/


/***************************************************************************Trigger install*********************************************************************************/
/**
 * @brief User click to trigger install callback processing function.
 * 
 * @param eActionType                       [OUT]       ADM_UIM_SOA_ACTION_AUTH_TYPE_E*     Whether the requested trigger action is allowed
 * @param priv                              [IN/OUT]    void*                               Reserved parameters
 */
typedef void (*ADM_UIM_INSTALL_TRIGGER_CALLBACK_F)(ADM_UIM_SOA_ACTION_AUTH_TYPE_E* eActionType, void* priv);
/***************************************************************************Trigger install*********************************************************************************/

typedef void (*ADM_UIM_QUERY_SMART_SCENARIO_CALLBACK_F)(DBOOL* pIsSmartScen, D32S* pResult);

typedef void (*ADM_UIM_QUERY_NOTIFY_CONFIRM_CALLBACK_F)(D32S* pResult);

typedef void (*ADM_UIM_GET_INSTALL_SCHEDULE_CALLBACK_F)(DCHAR ** pSchedule);


/***************************************************************************Trigger verify conditions************************************************************************/
/**
 * @brief Trigger condition detection
 * 
 * @param eVerifiedType     [IN]    const ADM_UIM_SOA_CONDITIONS_VERIFIED_TYPE_E       Types of condition detection
 * @param cycleTime         [IN]    const D16U  Condition detection cycle, unit: Second, when everifiedType = ADM_UIM_SOA_INSTALL_CONDITIONS_VERIFIED_TYPE_CYCLE
 * @param timeout           [IN]    const D16U  Period detection condition timeout, unit:Second, when everifiedType = ADM_UIM_SOA_INSTALL_CONDITIONS_VERIFIED_TYPE_CYCLE
 * @param errorCode         [OUT]   D32S*   Error code of triggering verify conditions.
 * @param priv                              [IN/OUT]    void*                               Reserved parameters
 */
typedef void (*ADM_UIM_INSTALL_CONDITIONS_VERIFIED_CALLBACK_F)(const ADM_UIM_SOA_CONDITIONS_VERIFIED_TYPE_E eVerifiedType, const D16U cycleTime, const D16U timeout, D32S* errorCode, void* priv);

// TODO: error code to be defined
/**
 * @brief Notification of verification results of install conditions.
 * 
 * @param eCmdidType        [IN]    const ADM_APPLICATION_COMMON_CMDID_TYPE_E    Type of detection trigger.
 * @param sessionId         [IN]    const DCHAR*        The sessionId of the strategy
 * @param state             [IN]    const DCHAR*    Task_State: Task_Install_Prepare
 * @param condsArrarList    [IN]    const ADM_UIM_SOA_CONDITION_DATA_ARRAY_T*   Condition value data array 
 * @param verifyResult      [IN]    const D32S  Conditional check result, 0:successed, Non-0: error code
 * @return D32S             0:successed, Non-0:failed 
 */
typedef D32S (*adm_uim_installConditionsVerifyResultNotify_func)(const ADM_APPLICATION_COMMON_CMDID_TYPE_E eCmdidType, const DCHAR* sessionId, const DCHAR* state, const ADM_UIM_SOA_CONDITION_DATA_ARRAY_T* condsArrarList, const D32S verifyResult);
extern D32S adm_uim_installConditionsVerifyResultNotify(const ADM_APPLICATION_COMMON_CMDID_TYPE_E eCmdidType, const DCHAR* sessionId, const DCHAR* state, const ADM_UIM_SOA_CONDITION_DATA_ARRAY_T* condsArrarList, const D32S verifyResult);

/***************************************************************************Trigger verify conditions************************************************************************/


/***************************************************************************Notify policy install progress******************************************************************/
/**
 * @brief Definition of the progress information structure of the upgrade package install.
 * 
 */
typedef struct {
    D16U progress;          /**!< Upgrade progress value */
} ADM_UIM_SOA_INSTALL_PROGRESS_INFO_T;

typedef struct
{
    ADM_APPLICATION_COMMON_CMDID_TYPE_E eCmdidType;
    const DCHAR *state;
    const DCHAR *sessionId;
    const ADM_UIM_SOA_ECU_IDEN_INFO_T* pstEcuIdenInfo;
    const ADM_UIM_SOA_INSTALL_PROGRESS_INFO_T* pstEcuInstallProgress;
} ADM_UIM_INSTALL_PROGRESS_ECU_T;

typedef struct
{
    ADM_APPLICATION_COMMON_CMDID_TYPE_E eCmdidType;
    const DCHAR *state;
    const DCHAR *sessionId;
    const ADM_UIM_SOA_INSTALL_PROGRESS_INFO_T* pProgress;
} ADM_UIM_INSTALL_PROGRESS_ALL_T;

/**
 * @brief Policy install progress notification.
 * 
 * @param eCmdidType        [IN]    const ADM_APPLICATION_COMMON_CMDID_TYPE_E    Type of detection trigger.
 * @param sessionId         [IN]    const DCHAR*        The sessionId of the strategy
 * @param state             [IN]    const DCHAR*    Task_State: Task_Install_Ing
 * @param pstPolicyInstallProgress [IN]    const ADM_UIM_SOA_INSTALL_PROGRESS_INFO_T*     The progress information structure of the upgrade package install.
 * @return D32S             0:successed, Non-0:failed 
 */
typedef D32S (*adm_uim_policyInstallProgressNotify_func)(const ADM_APPLICATION_COMMON_CMDID_TYPE_E eCmdidType, const DCHAR* sessionId, const DCHAR* state, const ADM_UIM_SOA_INSTALL_PROGRESS_INFO_T* pstPolicyInstallProgress);
extern D32S adm_uim_policyInstallProgressNotify(const ADM_APPLICATION_COMMON_CMDID_TYPE_E eCmdidType, const DCHAR* sessionId, const DCHAR* state, const ADM_UIM_SOA_INSTALL_PROGRESS_INFO_T* pstPolicyInstallProgress);

/***************************************************************************Notify policy install progress******************************************************************/


/***************************************************************************Notify policy install result********************************************************************/
/**
 * @brief Definition of the result information structure of the upgrade package install.
 * 
 */
typedef struct {
    D32S result;                                                /**!< The result of the remote module installing the upgrade package */
    DCHAR*  expectVal;                                          /**!< Description of the reason for the failure, expected value */
    DCHAR*  realVal;                                            /**!< Description of the reason for the failure, real value */
} ADM_UIM_SOA_INSTALL_RESULT_INFO_T;

typedef struct
{
    ADM_APPLICATION_COMMON_CMDID_TYPE_E eCmdidType;
    const DCHAR *state;
    const DCHAR *sessionId;
    const ADM_UIM_SOA_ECU_IDEN_INFO_T* pstEcuIdenInfo;
    const ADM_UIM_SOA_INSTALL_RESULT_INFO_T* pstEcuInstallResult;
} ADM_UIM_INSTALL_RESULT_ECU_T;

typedef struct
{
    ADM_APPLICATION_COMMON_CMDID_TYPE_E eCmdidType;
    const DCHAR *state;
    const DCHAR *sessionId;
    const ADM_UIM_SOA_INSTALL_RESULT_INFO_T* pResult;
} ADM_UIM_INSTALL_RESULT_ALL_T;

/**
 * @brief Policy install result notification.
 * 
 * @param eCmdidType        [IN]    const ADM_APPLICATION_COMMON_CMDID_TYPE_E    Type of detection trigger.
 * @param sessionId         [IN]    const DCHAR*        The sessionId of the strategy
 * @param state             [IN]    const DCHAR*    Task_State: Task_Install_Successed/Task_Install_Failed/Task_Install_Paused/Task_Install_Completed
 * @param pstPolicyInstallResult   [IN]    const ADM_UIM_SOA_INSTALL_RESULT_INFO_T*   The result information structure of the upgrade package install.
 * @return D32S             0:successed, Non-0:failed  
 */
typedef D32S (*adm_uim_policyInstallResultNotify_func)(const ADM_APPLICATION_COMMON_CMDID_TYPE_E eCmdidType, const DCHAR* sessionId, const DCHAR* state, const ADM_UIM_SOA_INSTALL_RESULT_INFO_T* pstPolicyInstallResult);
extern D32S adm_uim_policyInstallResultNotify(const ADM_APPLICATION_COMMON_CMDID_TYPE_E eCmdidType, const DCHAR* sessionId, const DCHAR* state, const ADM_UIM_SOA_INSTALL_RESULT_INFO_T* pstPolicyInstallResult);

/***************************************************************************Notify policy install result********************************************************************/


/**
 * @brief Identifies the install progress information of the ECU
 * 
 */
typedef struct {
    ADM_UIM_SOA_ECU_IDEN_INFO_T* pstEcuIdenInfo;                    /**!< Identification information of ECU upgrade package install */
    DCHAR*  state;                                                  /**!< Ecu_state: Ecu_Install_Ing/Ecu_Install_Finished*/
    ADM_UIM_SOA_INSTALL_PROGRESS_INFO_T* pstEcuInstallProgress;   /**!< The progress information structure of the upgrade package install. */
} ADM_UIM_SOA_ECU_INSTALL_PROGRESS_INFO_T;

/**
 * @brief ECU progress information array structure definition
 * 
 */
typedef struct {
    ADM_UIM_SOA_ECU_INSTALL_PROGRESS_INFO_T** ecuInstallProgressArray;    /**!< ECU progress information array */
    D16U    ecuInstallProgressArrayLength;                                 /**!< Length of ECU progress information array */
} ADM_UIM_SOA_ECU_INSTALL_PROGRESS_ARRAY_INFO_T;


typedef struct
{
    ADM_APPLICATION_COMMON_CMDID_TYPE_E eCmdidType;
    const DCHAR *state;
    const DCHAR *sessionId;
    const ADM_UIM_SOA_ECU_INSTALL_PROGRESS_ARRAY_INFO_T* pProgressArrayInfo;
    const ADM_UIM_SOA_INSTALL_PROGRESS_INFO_T* pPolicyInstallProgress;
} ADM_UIM_INSTALL_PROGRESS_BOTH_T;

/**
 * @brief Identifies the install result information of the ECU
 * 
 */
typedef struct {
    ADM_UIM_SOA_ECU_IDEN_INFO_T* pstEcuIdenInfo;                    /**!< Identification information of ECU upgrade package install */
    ADM_UIM_SOA_INSTALL_RESULT_INFO_T* pstEcuInstallResult;       /**!< The result information structure of the upgrade package install. */
} ADM_UIM_SOA_ECU_INSTALL_RESULT_INFO_T;



typedef struct {
    ADM_UIM_SOA_ECU_INSTALL_RESULT_INFO_T** ecuInstallResultArray;    /**!< ECU result information array */
    D16U    ecuInstallResultArrayLength;                               /**!< Length of ECU result information array */
} ADM_UIM_SOA_ECU_INSTALL_RESULT_ARRAY_INFO_T;

typedef struct
{
    ADM_APPLICATION_COMMON_CMDID_TYPE_E eCmdidType;
    const DCHAR *state;
    const DCHAR *sessionId;
    const ADM_UIM_SOA_ECU_INSTALL_RESULT_ARRAY_INFO_T* pEcuResultArrayInfo;
    const ADM_UIM_SOA_INSTALL_RESULT_INFO_T* pPolicyInstallResult; 
} ADM_UIM_INSTALL_RESULT_BOTH_T;

/***************************************************************************Notify one ECU install progress & result ********************************************************************/
/**
 * @brief Ecu install progress notification.
 * 
 * @param sessionId         [IN]    const DCHAR*        The sessionId of the strategy
 * @param state             [IN]    const DCHAR*        The state of Task_State: Task_Download_Ing
 * @param pstEcuIdenInfo    [IN]    const ADM_UIM_SOA_ECU_IDEN_INFO_T*  Identification information of ECU upgrade package install.
 * @param pstEcuInstallProgressInfo    [IN]    const ADM_UIM_SOA_INSTALL_PROGRESS_INFO_T*      Identifies the install progress information of the ECU.
 * @return D32S             0:successed, Non-0:failed
 */
typedef D32S (*adm_uim_ecuInstallProgressNotify_func)(const ADM_APPLICATION_COMMON_CMDID_TYPE_E eCmdidType, const DCHAR* sessionId, const DCHAR* state, const ADM_UIM_SOA_ECU_IDEN_INFO_T* pstEcuIdenInfo, const ADM_UIM_SOA_INSTALL_PROGRESS_INFO_T* pstEcuInstallProgressInfo);
extern D32S adm_uim_ecuInstallProgressNotify(const ADM_APPLICATION_COMMON_CMDID_TYPE_E eCmdidType, const DCHAR* sessionId, const DCHAR* state, const ADM_UIM_SOA_ECU_IDEN_INFO_T* pstEcuIdenInfo, const ADM_UIM_SOA_INSTALL_PROGRESS_INFO_T* pstEcuInstallProgressInfo);

/**
 * @brief Ecu install result notification.
 * 
 * @param sessionId         [IN]    const DCHAR*        The sessionId of the strategy
 * @param state             [IN]    const DCHAR*        The state of Task_State: Task_Download_Ing
 * @param pstEcuIdenInfo    [IN]    const ADM_UIM_SOA_ECU_IDEN_INFO_T*  Identification information of ECU upgrade package install.
 * @param pstEcuInstallResultInfo  [IN]    const ADM_UIM_SOA_ECU_INSTALL_RESULT_INFO_T*   Identifies the install result information of the ECU.
 * @param pstPolicyInstallResult   [IN]    const ADM_UIM_SOA_INSTALL_RESULT_INFO_T*   The result information structure of the upgrade package install.
 * @return D32S             0:successed, Non-0:failed 
 */
typedef D32S (*adm_uim_ecuInstallResultNotify_func)(const ADM_APPLICATION_COMMON_CMDID_TYPE_E eCmdidType, const DCHAR* sessionId, const DCHAR* state, const ADM_UIM_SOA_ECU_IDEN_INFO_T* pstEcuIdenInfo, const ADM_UIM_SOA_INSTALL_RESULT_INFO_T* pstEcuInstallResultInfo);
extern D32S adm_uim_ecuInstallResultNotify(const ADM_APPLICATION_COMMON_CMDID_TYPE_E eCmdidType, const DCHAR* sessionId, const DCHAR* state, const ADM_UIM_SOA_ECU_IDEN_INFO_T* pstEcuIdenInfo, const ADM_UIM_SOA_INSTALL_RESULT_INFO_T* pstEcuInstallResultInfo);

/***************************************************************************Notify one ECU install progress & result ********************************************************************/

/***************************************************************************Notify all ECU install progress & result ********************************************************************/
/**
 * @brief All Ecu install progress notification.
 * 
 * @param eCmdidType        [IN]    const ADM_APPLICATION_COMMON_CMDID_TYPE_E    Type of detection trigger.
 * @param sessionId         [IN]    const DCHAR*          The sessionId of the strategy
 * @param state             [IN]    const DCHAR*        Task_State: Task_Install_Ing  
 * @param pstEcuInstallProgressArrayInfo   [IN] const ADM_UIM_SOA_ECU_INSTALL_PROGRESS_ARRAY_INFO_T*  ECU progress information array structure.
 * @param pstPolicyInstallProgress [IN]    const ADM_UIM_SOA_INSTALL_PROGRESS_INFO_T*     The progress information structure of the upgrade package install.
 * @return D32S             0:successed, Non-0:failed 
 */
typedef D32S (*adm_uim_allEcuInstallProgressNotify_func)(const ADM_APPLICATION_COMMON_CMDID_TYPE_E eCmdidType, const DCHAR* sessionId, const DCHAR* state, const ADM_UIM_SOA_ECU_INSTALL_PROGRESS_ARRAY_INFO_T* pstEcuInstallProgressArrayInfo, const ADM_UIM_SOA_INSTALL_PROGRESS_INFO_T* pstPolicyInstallProgress);
extern D32S adm_uim_allEcuInstallProgressNotify(const ADM_APPLICATION_COMMON_CMDID_TYPE_E eCmdidType, const DCHAR* sessionId, const DCHAR* state, const ADM_UIM_SOA_ECU_INSTALL_PROGRESS_ARRAY_INFO_T* pstEcuInstallProgressArrayInfo, const ADM_UIM_SOA_INSTALL_PROGRESS_INFO_T* pstPolicyInstallProgress);


/**
 * @brief All Ecu install result notification.
 * 
 * @param eCmdidType        [IN]    const ADM_APPLICATION_COMMON_CMDID_TYPE_E    Type of detection trigger.
 * @param sessionId         [IN]    const DCHAR*          The sessionId of the strategy
 * @param state             [IN]    const DCHAR*        Task_State: Task_Install_Paused/Task_Install_Completed/Task_Install_Successed/Task_Install_Failed/Task_Transfer_xxx/Task_Unpack_xxx
 * @param pstEcuInstallResultArrayInfo     [IN] const ADM_UIM_SOA_ECU_INSTALL_RESULT_ARRAY_INFO_T*    Ecu result information array structure.
 * @param pstPolicyInstallResult   [IN]    const ADM_UIM_SOA_INSTALL_RESULT_INFO_T*   The result information structure of the upgrade package install.
 * @return D32S             0:successed, Non-0:failed
 */
typedef D32S (*adm_uim_allEcuInstallResultNotify_func)(const ADM_APPLICATION_COMMON_CMDID_TYPE_E eCmdidType, const DCHAR* sessionId, const DCHAR* state, const ADM_UIM_SOA_ECU_INSTALL_RESULT_ARRAY_INFO_T* pstEcuInstallResultArrayInfo, const ADM_UIM_SOA_INSTALL_RESULT_INFO_T* pstPolicyInstallResult);
extern D32S adm_uim_allEcuInstallResultNotify(const ADM_APPLICATION_COMMON_CMDID_TYPE_E eCmdidType, const DCHAR* sessionId, const DCHAR* state, const ADM_UIM_SOA_ECU_INSTALL_RESULT_ARRAY_INFO_T* pstEcuInstallResultArrayInfo, const ADM_UIM_SOA_INSTALL_RESULT_INFO_T* pstPolicyInstallResult);

/***************************************************************************Notify all ECU install progress & result ********************************************************************/

/***************************************************************************Query all ECU install progress & result *********************************************************************/
/**
 * @brief Request to query the install progress of all current ECUs.
 * 
 * @param eCmdidType        [OUT]    ADM_APPLICATION_COMMON_CMDID_TYPE_E*    Type of detection trigger.
 * @param sessionId         [OUT]    DCHAR**          The sessionId of the strategy
 * @param state             [OUT]    DCHAR**        Task_State: Task_Install_Ing  
 * @param pstEcuInstallProgressArrayInfo   [OUT] ADM_UIM_SOA_ECU_INSTALL_PROGRESS_ARRAY_INFO_T**  ECU progress information array structure.
 * @param pstEcuInstallProgressInfo    [OUT]    ADM_UIM_SOA_INSTALL_PROGRESS_INFO_T**      Identifies the install progress information of the ECU.
 * @param errorCode         [OUT]   D32S*   Error code
 * @param priv                              [IN/OUT]    void*                               Reserved parameters
 * @return  D32S                            0:successed, Non-0:failed
 */
typedef D32S (*ADM_UIM_INSTALL_QUERY_ALL_ECU_PROGRESS_CALLBACK_F)(ADM_APPLICATION_COMMON_CMDID_TYPE_E* eCmdidType, DCHAR** sessionId, DCHAR** state, ADM_UIM_SOA_ECU_INSTALL_PROGRESS_ARRAY_INFO_T** pstEcuInstallProgressArrayInfo,  ADM_UIM_SOA_INSTALL_PROGRESS_INFO_T** pstPolicyInstallProgress, D32S* errorCode, void* priv);

/**
 * @brief Request to query the install result of all current ECUs.
 * 
 * @param eCmdidType        [OUT]    ADM_APPLICATION_COMMON_CMDID_TYPE_E*    Type of detection trigger.
 * @param sessionId         [OUT]    DCHAR**          The sessionId of the strategy
 * @param state             [OUT]    DCHAR**        Task_State: Task_Install_Paused/Task_Install_Completed/Task_Install_Successed/Task_Install_Failed/Task_Transfer_xxx/Task_Unpack_xxx
 * @param pstEcuInstallResultArrayInfo     [OUT] ADM_UIM_SOA_ECU_INSTALL_RESULT_ARRAY_INFO_T**    Ecu result information array structure.
 * @param pstPolicyInstallResult   [OUT]    ADM_UIM_SOA_INSTALL_RESULT_INFO_T**   The result information structure of the upgrade package install.
 * @param errorCode         [OUT]   D32S*   Error code
 * @param priv                              [IN/OUT]    void*                               Reserved parameters
 * @return  D32S                            0:successed, Non-0:failed
 */
typedef D32S (*ADM_UIM_INSTALL_QUERY_ALL_ECU_RESULT_CALLBACK_F)(ADM_APPLICATION_COMMON_CMDID_TYPE_E* eCmdidType, DCHAR** sessionId, DCHAR** state, ADM_UIM_SOA_ECU_INSTALL_RESULT_ARRAY_INFO_T** pstEcuInstallResultArrayInfo, ADM_UIM_SOA_INSTALL_RESULT_INFO_T** pstPolicyInstallResult, D32S* errorCode, void* priv);
/***************************************************************************Query all ECU install progress & result *********************************************************************/




/***************************************************************************SOA-UIM-INSTALL init*****************************************************************************************/
/**
 * @brief The UIM-SOA-INSTALL initialization structure definition
 * 
 */
typedef struct ADM_UIM_INSTALL_INIT_INFO{
    ADM_UIM_INSTALL_SCHDULED_CALLBACK_F schduleInstallCallback;                 /**!< The user triggers the setting of the scheduled installation time. */
    ADM_UIM_INSTALL_CANCELD_CALLBACK_F  cancelInstallCallback;                  /**!< User triggers to cancel installation. */
    ADM_UIM_INSTALL_REFUSED_CALLBACK_F  refuseInstallCallback;                  /**!< User triggers to refuse install, when user triggers multiple times to cancel install.  */
    ADM_UIM_INSTALL_TRIGGER_CALLBACK_F installTriggerCallback;                /**!< User click to trigger install callback processing function. */
    ADM_UIM_INSTALL_CONDITIONS_VERIFIED_CALLBACK_F conditionsVerifiedCallback; /**!< Trigger condition detection */
    ADM_UIM_INSTALL_QUERY_ALL_ECU_PROGRESS_CALLBACK_F allEcuInstallProgressCallback;  /**!< Request to query the install progress of all current ECUs. */
    ADM_UIM_INSTALL_QUERY_ALL_ECU_RESULT_CALLBACK_F allEcuInstallResultCallback;      /**!< Request to query the install result of all current ECUs. */
    ADM_UIM_QUERY_SMART_SCENARIO_CALLBACK_F querySmartScenarioCallback;
    ADM_UIM_QUERY_NOTIFY_CONFIRM_CALLBACK_F notifyConfirmCallback;
    ADM_UIM_GET_INSTALL_SCHEDULE_CALLBACK_F getInstallScheduleCallback;     //< OTA 获取预约时间
} ADM_UIM_INSTALL_INIT_INFO_T;

/**
 * @brief The SOA-UIM-INSTALL module initializes the interface
 * @note Use adm_uim_installDeinit to release the initialization information
 * 
 * @param pstInitInfo   [IN]        const ADM_UIM_SOA_INSTALL_INIT_INFO_T*        The UIM-SOA-INSTALL initialization structure
ADM_UIM_APK_READY_INIT_INFO_T * @param priv          [IN/OUT]    void*                               Reserved parameters
 * @return D32S         0:successed, Non-0:failed    
 */
typedef D32S (*adm_uim_installInit_func)(const ADM_UIM_INSTALL_INIT_INFO_T* initInfo, void* priv);
extern D32S adm_uim_installInit(const ADM_UIM_INSTALL_INIT_INFO_T* initInfo, void* priv);
/**
 * @brief Release initialization information
 * @note Used to release the information initialized by the adm_uim_installInit function
 */
typedef D32S (*adm_uim_installDeinit_func)(void);
extern D32S adm_uim_installDeinit(void);
/***************************************************************************SOA-UIM-INSTALL init*****************************************************************************************/

typedef void (*ADM_UIM_BEHAVIOR_ONLINE_CALLBACK_F)(void *priv);
typedef struct ADM_UIM_APK_READY_INIT_INFO{
    ADM_UIM_BEHAVIOR_ONLINE_CALLBACK_F behaviorOnlineCallback;
}ADM_UIM_APK_READY_INIT_INFO_T;

//apk ready��Ϣ��Ӧ�ò�Ļص�����
typedef D32S (*adm_uim_apkReadyNoticeInit_func)(const ADM_UIM_APK_READY_INIT_INFO_T* initInfo, void* priv);
extern D32S adm_uim_apkReadyNoticeInit(const ADM_UIM_APK_READY_INIT_INFO_T* initInfo, void* priv);
typedef D32S (*adm_uim_apkReadyNoticeDeinit_func)(void);
extern D32S adm_uim_apkReadyNoticeDeinit(void);


#ifdef __cplusplus  // close out "C" linkage in case of c++ compiling
}
#endif

#endif//__ADM_UIM_INSTALL_H__
