/**
 * @file adm_uim_check.h
 * @brief 为检测流程定义的接口，包括：触发检测、版本信息反馈等。
 * @author Mengwei
 * @version v0.1.0
 * @date 2021-05-07
 * @copyright Copyright © 2018 All rights 上海艾拉比智能科技有限公司
 */
#ifndef __ADM_UIM_CHECK_H__
#define __ADM_UIM_CHECK_H__

#ifdef __cplusplus
extern "C" {
#endif

#define EXPORT_FUN __attribute__((visibility("default")))

#include "adm_typedefs.h"
#include "adm_uim_common.h"
#include "adm_application_common.h"

/***************************************************************************Trigger check*********************************************************************************/
/**
 * @brief User click to trigger version detection callback processing function.
 * 
 * @param eActionType                       [OUT]       ADM_UIM_SOA_ACTION_AUTH_TYPE_E*     Whether the requested trigger action is allowed
 * @param priv                              [IN/OUT]    void*                               Reserved parameters
 */
typedef void (*ADM_UIM_CHECK_TRIGGER_CALLBACK_F)(ADM_UIM_SOA_ACTION_AUTH_TYPE_E* eActionType, void* priv);
/***************************************************************************Trigger check*********************************************************************************/

/*************************************************************************Usb Trigger check*******************************************************************************/
/**
 * @brief User click to trigger version detection callback processing function.
 * 
 * @param pUsbDir       [IN]    USB 挂载路径
 * @param eActionType                       [OUT]       ADM_UIM_SOA_ACTION_AUTH_TYPE_E*     Whether the requested trigger action is allowed
 * @param priv                              [IN/OUT]    void*                               Reserved parameters
 */
typedef void (*ADM_UIM_USB_CHECK_TRIGGER_CALLBACK_F)(const DCHAR* pUsbDir, ADM_UIM_SOA_ACTION_AUTH_TYPE_E* eActionType, void* priv);
/*************************************************************************Usb Trigger check*******************************************************************************/

typedef void (*ADM_UIM_CHECK_QUERY_ECU_INFO_CALLBACK_F)(DBOOL* pIsExist, void* priv);

/***************************************************************************Notify check progress*************************************************************************/
// /**
//  * @brief Progress type value for version check.
//  * 
//  */
// typedef enum ADM_APPLICATION_COMMON_CHECK_PROGRESS_TYPE_ {
//     ADM_APPLICATION_COMMON_CHECK_PROGRESS_TYPE_START = 1,                  /*!< Start check */
//     ADM_APPLICATION_COMMON_CHECK_PROGRESS_TYPE_GET_REGISTER_INFO,          /*!< Get register info, deviceId, secret, etc */
//     ADM_UIM_CHECK_PROGRESS_TYPE_GET_CONFIG_INFO,            /*!< Get config info, ECU config & General config */
//     ADM_UIM_CHECK_PROGRESS_TYPE_GET_ECUINFO,                /*!< To get every ECU information, it needs to be thrown */
//     ADM_UIM_CHECK_PROGRESS_TYPE_GET_NET_INFO,               /*!< Get network info */
//     ADM_UIM_CHECK_PROGRESS_TYPE_SIGNED,                     /*!< Sign the message requesting check */
//     ADM_UIM_CHECK_PROGRESS_TYPE_ENCRYPT,                    /*!< Encrypt the message of check requesting */
//     ADM_UIM_CHECK_PROGRESS_TYPE_REQUEST_TO_SERVER,          /*!< Request server to check version */
//     ADM_UIM_CHECK_PROGRESS_TYPE_RESP_FROM_SERVER,           /*!< Response message from server */
//     ADM_UIM_CHECK_PROGRESS_TYPE_DECRYPT,                    /*!< Decrypt the message of check response */
//     ADM_UIM_CHECK_PROGRESS_TYPE_VERIFY_SIGN,                /*!< Verify signature of check response message */
//     ADM_UIM_CHECK_PROGRESS_TYPE_END,                        /*!< End check */
//     ADM_UIM_CHECK_PROGRESS_TYPE_INVALID                     /*!< Invalid */
// } ADM_APPLICATION_COMMON_CHECK_PROGRESS_TYPE_T;


/**
 * @brief Notify the progress of the test execution.
 * 
 * @param eCmdidTYpe            [IN]    const ADM_APPLICATION_COMMON_CMDID_TYPE_E      Type of detection trigger.
 * @param state                 [IN]    const DCHAR*    Task_State: Check_Ing
 * @param eCheckProgressType    [IN]    const ADM_APPLICATION_COMMON_CHECK_PROGRESS_TYPE_T     Progress type value for version check.
 */
typedef void (*adm_uim_checkProgressNotify_func)(const ADM_APPLICATION_COMMON_CMDID_TYPE_E eCmdidTYpe, const DCHAR* state, const ADM_APPLICATION_COMMON_CHECK_PROGRESS_TYPE_T eCheckProgressType);
extern void adm_uim_checkProgressNotify(const ADM_APPLICATION_COMMON_CMDID_TYPE_E eCmdidTYpe, const DCHAR* state, const ADM_APPLICATION_COMMON_CHECK_PROGRESS_TYPE_T eCheckProgressType);
/***************************************************************************Notify check progress*************************************************************************/

/***************************************************************************Notify check result***************************************************************************/
/**
 * @brief HMI display strategy file information structure definition.
 * 
 */

typedef struct {
    DCHAR*  policyFileUrl;              ///< Download address of the policy file */
    D64U    policyFileSize;             ///< Size of the policy file, unit:Byte */
    DCHAR*  policyFileHash;             ///< Hash of the policy file(SHA256 sum) */
} ADM_UIM_HMI_FILE_T;

typedef struct      
{
	//DCHAR* 	ecuPartNum;				///< Ecu part number
    DCHAR* 	requestId;         		///< Request ID
    DCHAR* 	swDid;             		///< The did of the software version, used to distinguish different upgrade parts of the same requestId ECU
    DCHAR*  ecuVersion; 			///< Ecu Version
    D64U 	ecuSize; 				///< Ecu upgrade package size
} ADM_UIM_ECU_INFO_T;

typedef struct {
    ADM_UIM_ECU_INFO_T**    hmiEcuInfoArray;   ///< Array of all ECU information to be flashed 
    D16U    hmiEcuInfoArrayLength;             ///< Array length of all ECU information to be flashed 
} ADM_UIM_ECU_INFO_ARRAY_T;

typedef struct {
	DCHAR*  	vehicleVersion; 	///< Vehicle version 	
	D64U 		totalSize;			///< all ecus's upgrade package size	
    ADM_UIM_ECU_INFO_ARRAY_T* ecuInfoList;
} ADM_UIM_HMI_INFO;

typedef enum ADM_UIM_UPGRADE_MODE_ {  
    ADM_UIM_UPGRADE_MODE_NORMAL_DOWN_NORMAL_INS = 1,    ///< normal downlaod, normal install
    ADM_UIM_UPGRADE_MODE_SILENT_DPWN_NORAML_INS,        ///< silent downlaod, normal install
    ADM_UIM_UPGRADE_MODE_SILENT_DPWN_SILENT_INS,        ///< silent downlaod, silent install
} ADM_UIM_UPGRADE_MODE_E;

typedef struct {
    ADM_UIM_HMI_FILE_T* 	hmiFile;
    ADM_UIM_HMI_INFO* 		hmiInfo;
    ADM_UIM_UPGRADE_MODE_E	upgradeMode;
    D32S iTotalTimeS;
} ADM_UIM_SOA_CHECK_HMI_POLICY_INFO_T;

typedef struct 
{
    D32S eCmdidType; 
    const DCHAR* sessionId; 
    const DCHAR* state;
    const ADM_UIM_SOA_CHECK_HMI_POLICY_INFO_T* policyInfo; 
    D32S errorCode; 
} ADM_UIM_CHECK_INFO_T;


/**
 * @brief Notification of version detection results.
 * 
 * @param eCmdidType        [IN]    const ADM_APPLICATION_COMMON_CMDID_TYPE_E    Type of detection trigger.
 * @param sessionId         [IN]    const DCHAR*    The session id of the platform response
 * @param state             [IN]    const DCHAR*    Task_State: Task_Check_Successed/Task_Check_Failed
 * @param pstPolicyInfo     [IN]    const ADM_UIM_SOA_CHECK_HMI_POLICY_INFO_T*  HMI display strategy file information structure.
 * @param errorCode         [IN]    const D32S  Error code of the reason for failure, 0 for success         
 */
extern void adm_uim_checkResultNotify(const ADM_APPLICATION_COMMON_CMDID_TYPE_E eCmdidType, const DCHAR* sessionId, const DCHAR* state, const ADM_UIM_SOA_CHECK_HMI_POLICY_INFO_T* pstPolicyInfo, const D32S errorCode);
typedef void (*adm_uim_checkResultNotify_func)(const ADM_APPLICATION_COMMON_CMDID_TYPE_E eCmdidType, const DCHAR* sessionId, const DCHAR* state, const ADM_UIM_SOA_CHECK_HMI_POLICY_INFO_T* pstPolicyInfo, const D32S errorCode);

/***************************************************************************Notify check result***************************************************************************/

/***************************************************************************Query check result****************************************************************************/
/**
 * @brief Used to request to query the currently stored version detection result.
 * @note Output type parameters need to release memory.
 * 
 * @param eCmdidType        [OUT]    ADM_APPLICATION_COMMON_CMDID_TYPE_E*    Type of detection trigger.
 * @param sessionId         [OUT]    DCHAR**    The session id of the platform response
 * @param state             [OUT]    DCHAR**    Task_State: Task_Check_Successed/Task_Check_Failed
 * @param pstPolicyInfo     [OUT]    ADM_UIM_SOA_CHECK_HMI_POLICY_INFO_T**  HMI display strategy file information structure.
 * @param errorCode         [OUT]    D32S*  Error code of the reason for failure, 0 for success
 * @param priv                              [IN/OUT]    void*                               Reserved parameters
 * @return  D32S                            0:successed, Non-0:failed
 */
typedef void (*ADM_UIM_CHECK_QUERY_RESULT_CALLBACK_F)(ADM_APPLICATION_COMMON_CMDID_TYPE_E* eCmdidType, DCHAR** sessionId, DCHAR** state, ADM_UIM_SOA_CHECK_HMI_POLICY_INFO_T** ppstPolicyInfo, D32S* errorCode, void* priv);
/***************************************************************************Query check result****************************************************************************/

/***************************************************************************SOA-UIM-CHECK init****************************************************************************/
/**
 * @brief The UIM-SOA-CHECK initialization structure definition
 * 
 */
typedef struct ADM_UIM_CHECK_INIT_INFO{
    ADM_UIM_CHECK_TRIGGER_CALLBACK_F   checkTriggerCallback;       /**!< User click to trigger version detection callback processing function. */
    ADM_UIM_USB_CHECK_TRIGGER_CALLBACK_F   usbCheckTriggerCallback;       /**!< Usb mode, user click to trigger version detection callback processing function. */
    ADM_UIM_CHECK_QUERY_RESULT_CALLBACK_F   queryResultCallback;   /**!< Used to request to query the currently stored version detection result. */
    ADM_UIM_CHECK_QUERY_ECU_INFO_CALLBACK_F checkQueryEcuInfoCallback;
} ADM_UIM_CHECK_INIT_INFO_T;


/**
 * @brief The SOA-UIM-CHECK module initializes the interface
 * @note Use adm_uim_checkDeinit to release the initialization information
 * 
 * @param pstInitInfo   [IN]        const ADM_DIM_SOA_CHECK_INIT_INFO_T*        The UIM-SOA-CHECK initialization structure
 * @param priv          [IN/OUT]    void*                               Reserved parameters
 * @return D32S         0:successed, Non-0:failed    
 */
extern D32S adm_uim_checkInit(const ADM_UIM_CHECK_INIT_INFO_T* initInfo, void* priv);
typedef D32S (*adm_uim_checkInit_func)(const ADM_UIM_CHECK_INIT_INFO_T* initInfo, void* priv);
/**
 * @brief Release initialization information
 * @note Used to release the information initialized by the adm_uim_checkInit function
 */
extern D32S adm_uim_checkDeinit(void);
typedef D32S (*adm_uim_checkDeinit_func)(void);
/***************************************************************************SOA-UIM-CHECK init****************************************************************************/


#ifdef __cplusplus  // close out "C" linkage in case of c++ compiling
}
#endif

#endif//__ADM_UIM_CHECK_H__
