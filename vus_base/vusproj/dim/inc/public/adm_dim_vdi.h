/**
 * @file adm_dim_soaVdi.h
 * @brief VDI已服务化的形式为UC-Master提供的车身数据相关的接口
 * @details 主要由VDI提供接口，用于OTA过程中与车身相关的功能，包括：UDS诊断、车身状态、车身信号等
 * @author Mengwei
 * @version v0.1.0
 * @date 2021-04-29
 * @copyright Copyright © 2018 All rights 上海艾拉比智能科技有限公司
 */
#ifndef __ADM_DIM_SOA_VDI_H__
#define __ADM_DIM_SOA_VDI_H__

#ifdef __cplusplus
extern "C" {
#endif


#define EXPORT_FUN __attribute__((visibility("default")))

#include "adm_dim_common.h"


/************************* Get SDK Version ****************************/
/**
 * @brief 获取VDI的sdk版本号
 *
 * @param instanceId    [IN]    实例ID
 * @param timeout       [IN]    请求的超时时间
 * @param sdkVersion    [OUT]  SDK 版本号
 * @param errorCode     [OUT]  获取版本号的结果
 */
extern void adm_dim_vdi_getSdkVersion(const D16U instanceId, const D16U timeout,
                                      DCHAR** sdkVersion, D32S* errorCode);
typedef void (*adm_dim_vdi_getSdkVersion_func)(const D16U instanceId,
        const D16U timeout, DCHAR** sdkVersion, D32S* errorCode);

/************************** Get SDK Version *****************************/

/****************************************************************************************/
/***************************** vdi interface update below *******************************/
/****************************************************************************************/

/**
 * @brief ECU directly connected bus type
*/
typedef enum VEHICLE_BUS_TYPE_ENUM
{
    VEHICLE_BUS_TYPE_CAN = 1, ///< CAN
    VEHICLE_BUS_TYPE_CANFD,   ///< CANFD
    VEHICLE_BUS_TYPE_ETH,     ///< ETH
    VEHICLE_BUS_TYPE_LIN,     ///< LIN
    /** 自定义总线类型从这里开始*/

    /** 自定义总线类型这里结束*/
    VEHICLE_BUS_TYPE_INVALID  ///< INVALID
} VEHICLE_BUS_TYPE_E;
typedef enum VEHICLE_BUS_TYPE_ENUM ADM_DIM_SOA_VDI_BUS_TYPE_E;

/**
* @enum VEHICLE_BUS_PIN_TYPE_ENUM
* @brief CAN bus channel type
*
*/
typedef enum VEHICLE_BUS_PIN_TYPE_ENUM
{
    VEHICLE_BUS_PIN_TYPE_A_CAN = 1, ///< ACAN
    VEHICLE_BUS_PIN_TYPE_B_CAN = 2, ///< BCAN
    VEHICLE_BUS_PIN_TYPE_P_CAN = 3, ///< PCAN
    /** 自定义总线PIN类型从这里开始*/

    /** 自定义总线PIN类型这里结束*/
    VEHICLE_BUS_PIN_TYPE_INVALID    ///< Invalid
} VEHICLE_BUS_PIN_TYPE_E;
typedef enum VEHICLE_BUS_PIN_TYPE_ENUM ADM_DIM_SOA_PIN_CHANNEL_TYPE_E;

//车辆控制
typedef enum VEHICLE_CTRL_ENUM ADM_DIM_SOA_VDI_CTRL_VEHICLE_STATUS_TYPE_E;
//车辆事件
typedef enum VEHICLE_EVENT_ENUM ADM_DIM_SOA_VDI_VEHICLE_SIGNAL_TRIGGER_TYPE_E;
//车辆状态
typedef enum VEHICLE_STATUS_ENUM ADM_DIM_SOA_VDI_VEHICLE_STATUS_TYPE_E;

/**
 * @struct ADM_SYSI4VDI_ENGINE_SPY_T
 * @brief The structure definition of the vehicle spy.
 * @note 当compare_symbol为
 [COMPARE_SYMBOL_GREATE|COMPARE_SYMBOL_GREATEEQ|COMPARE_SYMBOL_EQ|COMPARE_SYMBOL_NOTEQ|COMPARE_SYMBOL_LESS|COMPARE_SYMBOL_LESSEQ]
  sysi4all_signal_variable_st rightSpyData; ///< 监控值@see sysi4all_signal_variable_st
 *时，leftSpyData有效；其他，leftSpyData和rightSpyData有效
 *
 */
typedef struct
{
    D32U vehicle_id;       ///< vehicle状态、触发事件枚举值 @see ADM_SYSI4VDI_ENGINE_CTRL_VEHICLE_STATUS_TYPE_E @see ADM_SYSI4VDI_ENGINE_VEHICLE_SIGNAL_TRIGGER_TYPE_E
    D32U compare_symbol;   ///< 监控逻辑 @see SYSI4ALL_COMPARE_SYMBOL
    D32U multi_relate;     ///< 当存在多个监控message时，多个message的逻辑关系 @see SYSI4ALL_COMPARE_CONNECT
    ADM_DIM_SOA_DATA_T firstSpyData;   ///< 监控值@see sysi4all_signal_variable_st
    ADM_DIM_SOA_DATA_T seondSpyData;  ///< 监控值@see sysi4all_signal_variable_st
} ADM_SYSI4VDI_ENGINE_SPY_T;

/**
* @struct ADM_SYSI4VDI_ENGINE_SPY_ARRAY_T
* @brief Vehicle spy of  Array structure.
*/
typedef struct
{
    D16U msgIdArrayLength;                   ///< Array length of vehicle spy
    ADM_SYSI4VDI_ENGINE_SPY_T spyArray[1];   ///< flexible array of vehicle spy @see ADM_SYSI4VDI_ENGINE_SPY_T
} ADM_SYSI4VDI_ENGINE_SPY_ARRAY_T;


/**
 * @brief the area info of ecu
 */
typedef enum 
{
    ADM_SYSI4VDI_ECU_AREA_INFO_UNKNOW = 0, ///< unknow
    ADM_SYSI4VDI_ECU_AREA_INFO_A = 1,      ///< "A"
    ADM_SYSI4VDI_ECU_AREA_INFO_B = 2       ///< "B"
} ADM_DIM_SOA_VDI_ECU_AREA_INFO_E;

/****************************************************************************************/
/***************************** vdi interface update above *******************************/
/****************************************************************************************/



/***************************************************************************Read diagnostic information**************************************************************************/
/**
 * @brief   Read diagnostic information
 *
 * @param instanceId        [IN]        const D16U  instance identifier
 * @param timeout               [IN]        const D16U  Timeout of sync interface, unit: second
 * @param pstEcuIdenInfo        [IN]    const ADM_DIM_SOA_ECU_IDEN_INFO_T*  Identification information of ECU
 * @param diagScriptPath        [IN]    const DCHAR*    Absolute path of script used to obtain diagnostic information
 * @param eBusType              [IN]    const ADM_DIM_SOA_VDI_BUS_TYPE_E    ECU directly connected bus type
 * @param isCGWRouted           [IN]    const DBOOL     True:CGW routed; False:Not CGW routed
 * @param didListRequest        [IN]    const ADM_DIM_SOA_DID_DATA_ARRAY_T*     Request structure for reading diagnostic information.
 * @param didListResponse       [OUT]   ADM_DIM_SOA_DID_DATA_ARRAY_T**          Response structure for reading diagnostic information.
 * @param errorCode             [OUT]   D32S*   The execution result of this request
 * @return D32S                 0:successed, Non-0:failed
 */
extern D32S adm_dim_vdi_readDiagnosticInfo(const D16U instanceId,
        const D16U timeout,
        const ADM_DIM_SOA_ECU_IDEN_INFO_T* pstEcuIdenInfo,
        const DCHAR* diagScriptPath, const ADM_DIM_SOA_VDI_BUS_TYPE_E eBusType,
        const ADM_DIM_SOA_PIN_CHANNEL_TYPE_E eChannelType,
        const DBOOL isCGWRouted, const ADM_DIM_SOA_DID_DATA_ARRAY_T didListRequest,
        ADM_DIM_SOA_DID_DATA_ARRAY_T** didListResponse, D32S* errorCode);
typedef D32S(*adm_dim_vdi_readDiagnosticInfo_func)(const D16U instanceId,
        const D16U timeout,
        const ADM_DIM_SOA_ECU_IDEN_INFO_T* pstEcuIdenInfo,
        const DCHAR* diagScriptPath, const ADM_DIM_SOA_VDI_BUS_TYPE_E eBusType,
        const ADM_DIM_SOA_PIN_CHANNEL_TYPE_E eChannelType,
        const DBOOL isCGWRouted, const ADM_DIM_SOA_DID_DATA_ARRAY_T didListRequest,
        ADM_DIM_SOA_DID_DATA_ARRAY_T** didListResponse, D32S* errorCode);


//并行采集零件
D32S adm_dim_vdi_parallelCollectEcu(const D16U instanceId,
                                    const D16U timeout,
                                    const ADM_DIM_SOA_ECU_IDEN_INFO_T* pstEcuIdenInfo,
                                    const DCHAR* diagScriptPath, const ADM_DIM_SOA_VDI_BUS_TYPE_E eBusType,
                                    const ADM_DIM_SOA_PIN_CHANNEL_TYPE_E eChannelType,
                                    const DBOOL isCGWRouted, const ADM_DIM_SOA_DID_DATA_ARRAY_T didListRequest, D32S* errorCode);
typedef D32S(*adm_dim_vdi_parallelCollectEcu_func)(const D16U instanceId,
        const D16U timeout,
        const ADM_DIM_SOA_ECU_IDEN_INFO_T* pstEcuIdenInfo,
        const DCHAR* diagScriptPath, const ADM_DIM_SOA_VDI_BUS_TYPE_E eBusType,
        const ADM_DIM_SOA_PIN_CHANNEL_TYPE_E eChannelType,
        const DBOOL isCGWRouted, const ADM_DIM_SOA_DID_DATA_ARRAY_T didListRequest,
        D32S* errorCode);


/**
 * @brief Read diagnostic information notification.
 *
 * @param pstEcuIdenInfo        [IN]    const ADM_DIM_SOA_ECU_IDEN_INFO_T*  Identification information of ECU
 * @param didListResponse       [OUT]   const ADM_DIM_SOA_DID_DATA_ARRAY_T*          Response structure for reading diagnostic information.
 *
 * @return non
 */
typedef void (*ADM_DIM_SOA_VDI_READ_DIAG_INFO_CALLBACK_F)(
    const ADM_DIM_SOA_ECU_IDEN_INFO_T* pstEcuIdenInfo,
    const ADM_DIM_SOA_DID_DATA_ARRAY_T* didListResponse);
/***************************************************************************Read diagnostic information**************************************************************************/

/***************************************************************************Do reflash ECUs**************************************************************************************/
/**
 * @brief  Absolute path of sa library used to UDS diagnostic flashing
 *
 */
typedef struct
{
    DCHAR* saLibPath;                               /**!< Absolute path of sa library used to UDS diagnostic flashing */
} ADM_SYSI4VDI_SA_LIB_INFO_T;

/**
 * @brief Array of salib information.
 * @note This information generally needs to be obtained from the decompressed upgrade package configuration file.
 *
 */
typedef struct
{
    ADM_SYSI4VDI_SA_LIB_INFO_T
    ** saLibArray;    /**!< Array of salib information. */
    D16U    saLibArrayNum;                   /**!< Array number of salib information. */
} ADM_SYSI4VDI_SA_LIB_INFO_ARRAY_T;


/**
 * @brief Image information to be flashed.
 *
 */
typedef struct
{
    DCHAR*  imagePath;                              /**!< Absolute path of image */
    DCHAR*  imageHash;                              /**!< Hash(SHA256 sum) of image */
    D64S    imageSize;                              /**!< Size of image, unit:Byte */
} ADM_DIM_SOA_VDI_IMAGE_INFO_T;

/**
 * @brief Array of image information to be flashed.
 * @note This information generally needs to be obtained from the decompressed upgrade package configuration file.
 *
 */
typedef struct
{
    ADM_DIM_SOA_VDI_IMAGE_INFO_T** imageInfoArray;     /**!< Array of image information to be flashed */
    D16U    imageInfoArrayLength;                   /**!< Array length of image information to be flashed */
} ADM_DIM_SOA_VDI_IMAGE_INFO_ARRAY_T;

/**
 * @struct sysi4all_crc_param_struct
 * @brief crc算法中使用的初始值
 *
 */
typedef struct sysi4all_crc_param_struct
{
    D32U polynomial;         ///< 多项式初始值
    D32U init_value;         ///< crc初始值
    D32U final_xor;          ///< crc结果异或值
    DBOOL is_input_reflect;  ///< crc输入值是否反转
    DBOOL is_output_reflect; ///< crc 输出值是否反转
} sysi4all_crc_param_st, *sysi4all_crc_param_pt;

typedef enum SYSI4ALL_CRC_TYPE_ENUM
{
    CRC_TYPE_CRC32 = 1, ///< crc32
    CRC_TYPE_CRC16,     ///< crc16
    CRC_TYPE_CRC8,      ///< crc8
    CRC_TYPE_MAX,
} SYSI4ALL_CRC_TYPE_E;


/**
 * @brief All information of the ECU to be flashed.
 *
 */
typedef struct
{
    ADM_DIM_SOA_ECU_IDEN_INFO_T* pstEcuIdenInfo;   /**!< Identification information of ECU */
    DCHAR* srcSoftVersion;                             /**!< The source version of the upgraded part */
    DCHAR* dstSoftVersion;                             /**!< The target version of the upgraded part */
    DCHAR* flashScriptPath;                         /**!< Absolute path of script used to UDS diagnostic flashing */
    ADM_DIM_SOA_VDI_BUS_TYPE_E eBusType;               /**!< ECU directly connected bus type */
    D16U    isCGWRouted;                            /**!< 1:CGW routed; 0:Not CGW routed */
    ADM_DIM_SOA_PIN_CHANNEL_TYPE_E eChannelType;    /**!<  */
    DCHAR*    saMask;                                 /**!< Algorithm mask required for UDS diagnostic flashing */
    D32S      flashProgressType;                     /**!< 文件刷写流程类型  0x0:传统部件 UDS service 34 36 37（默认值） 0x1:智能部件 UDS service 38 36 37 */
    ADM_SYSI4VDI_SA_LIB_INFO_ARRAY_T* saLibList;    /**!< Array of salib information. */
    D32U    crcMask;                                    /**!< CRC algorithm mask */
    sysi4all_crc_param_st crc;                           ///< crc param @see sysi4all_crc_param_st @see sysi4all_crc_param_st
    DCHAR*  crcLibPath;                                 /**!< Absolute path of crc library to UDS diagnostic flashing */
    ADM_DIM_SOA_VDI_IMAGE_INFO_ARRAY_T* imageInfoList; /**!< Array of image information to be flashed */
    ADM_DIM_SOA_SA_MASK_INFO_T* pstSaMaskList;          /** replace saMask */
    DBOOL isBlockPartion;                               /** if block switch ab partion */
} ADM_DIM_SOA_VDI_REFLASH_INFO_T;

typedef struct
{
    ADM_DIM_SOA_VDI_REFLASH_INFO_T** reflashInfoArray;   /**!< Array of all ECU information to be flashed */
    D16U    reflashInfoArrayLength;                     /**!< Array length of all ECU information to be flashed */
} ADM_DIM_SOA_VDI_REFLASH_INFO_ARRAY_T;

/**
 * @brief flash和read的doip逻辑地址互斥
 * 逻辑地址互斥 jira:QRV05-2646 below(安装前和安装后优先级有差异)
 */
typedef enum 
{
    ADM_DIM_SOA_VDI_FLASH_STATE_BEGIN = 0,
    ADM_DIM_SOA_VDI_FLASH_STATE_ENTER = 1,
    ADM_DIM_SOA_VDI_FLASH_STATE_EXIT = 2,
    ADM_DIM_SOA_VDI_FLASH_STATE_FINISH
}ADM_DIM_SOA_VDI_FLASH_STATE_E;

/**
 * @brief Trigger flashing of multiple or one ECU.
 * @warning This interface needs to support multi-threaded or parallel calls.
 *
 * @param instanceId        [IN]        const D16U  instance identifier
 * @param timeout           [IN]        const D16U  Timeout of sync interface, unit: second
 * @param bIsEnableAb       [IN]        If AB flash mode
 * @param eAbInfo           [IN]        flash AB info
 * @param reflashInfoList   [IN]    const ADM_DIM_SOA_VDI_REFLASH_INFO_ARRAY_T*      All information of the ECU to be flashed
 * @param errorCode         [OUT]   D32S*       The result of triggering flash
 * @return D32S             0:successed, Non-0:failed
 */
extern D32S adm_dim_vdi_reflashEcusTrigger(const D16U instanceId, const D16U timeout, 
        const ADM_DIM_SOA_VDI_REFLASH_INFO_ARRAY_T* reflashInfoList, D32S* errorCode);
typedef D32S(*adm_dim_vdi_reflashEcusTrigger_func)(const D16U instanceId, const D16U timeout, 
        const ADM_DIM_SOA_VDI_REFLASH_INFO_ARRAY_T* reflashInfoList, D32S* errorCode);

/** 增加AB分区刷写函数 */
extern D32S adm_dim_vdi_AbReflashEcusTrigger(const D16U instanceId, const D16U timeout, 
        DBOOL bIsEnableAb, ADM_DIM_SOA_VDI_ECU_AREA_INFO_E eAbInfo,
        const ADM_DIM_SOA_VDI_REFLASH_INFO_ARRAY_T* reflashInfoList, D32S* errorCode);
typedef D32S(*adm_dim_vdi_AbReflashEcusTrigger_func)(const D16U instanceId, const D16U timeout, 
        DBOOL bIsEnableAb, ADM_DIM_SOA_VDI_ECU_AREA_INFO_E eAbInfo,
        const ADM_DIM_SOA_VDI_REFLASH_INFO_ARRAY_T* reflashInfoList, D32S* errorCode);

extern D32S adm_dim_vdi_notifyFlashState(const D16U uiInstanceId, const D16U uiTimeout, 
        ADM_DIM_SOA_VDI_FLASH_STATE_E eFlashState, D32S* pErrorCode);

extern D32S adm_dim_vdi_clearDtc(const D16U uiInstanceId, const D16U uiTimeout,
                                D32S *pErrorCode);
typedef D32S(*adm_dim_vdi_notifyFlashState_func)(const D16U uiInstanceId, const D16U uiTimeout, 
        D32U uiFlashState, D32S* pErrorCode);

typedef D32S(*adm_dim_vdi_clearDtc_func)(const D16U uiInstanceId, const D16U uiTimeout, 
        D32S* pErrorCode);


/**
 * @brief The callback function is used to process the progress information thrown by the vdi relfash module.
 *
 * @param pstReflashProgressNotifyInfo      [IN]   const ADM_DIM_SOA_VDI_REFLASH_PROGRESS_INFO_T*  The progress relfash information of the ECU reflash thrown by the sysi4vdi module.
 * @param priv                              [IN/OUT]    void*                               Reserved parameters
 * @return  D32S                            0:successed, Non-0:failed
 */
typedef D32S(*ADM_DIM_REFLASH_PROGRESS_NOTIFY_CALLBACK_F)(
    const ADM_DIM_SOA_ECU_IDEN_INFO_T* pstIdenInfo, const D16U* reflashProgress,
    void* priv);

typedef D32S (*ADM_DIM_GET_VEHICLE_TYPE_CALLBACK_F)(ADM_DIM_SOA_VEHICLE_TYPE_E* pVehicleType);

/**
 * @brief Used to actively query the flashing progress of an ECU
 *
 * @param instanceId        [IN]        const D16U  instance identifier
 * @param timeout               [IN]        const D16U  Timeout of sync interface, unit: second
 * @param pstEcuIdenInfo                    [IN]    const ADM_DIM_SOA_ECU_IDEN_INFO_T*  Structure used to uniquely identify an ECU.
 * @param pstReflashProgressNotifyInfo      [OUT]   ADM_DIM_SOA_VDI_REFLASH_PROGRESS_INFO_T**  The progress relfash information of the ECU reflash thrown by the sysi4vdi module.
 * @param errorCode                         [OUT]   D32S*   Error code
 * @return D32S                             0:successed, Non-0:failed
 */
extern D32S adm_dim_vdi_queryReflashProgress(const D16U instanceId,
        const D16U timeout, const ADM_DIM_SOA_ECU_IDEN_INFO_T* pstEcuIdenInfo,
        D16U* reflashProgress, D32S* errorCode);
typedef D32S(*adm_dim_vdi_queryReflashProgress_func)(const D16U instanceId,
        const D16U timeout, const ADM_DIM_SOA_ECU_IDEN_INFO_T* pstEcuIdenInfo,
        D16U* reflashProgress, D32S* errorCode);

/**
 * @brief The result structure definition of the remote download upgrade package
 *
 */
typedef struct
{
    D32S result;                                                /**!< The result of the remote module downloading the upgrade package */
    DCHAR*  expectVal;                                          /**!< Description of the reason for the failure, expected value */
    DCHAR*  realVal;                                            /**!< Description of the reason for the failure, expected value */
} ADM_DIM_SOA_VDI_REFLASH_RESULT_INFO_T;

/**
 * @brief The callback function is used to process the result information thrown by the vdi relfash module.
 *
 * @param pstEcuIdenInfo                    [IN]    const ADM_DIM_SOA_ECU_IDEN_INFO_T*  Structure used to uniquely identify an ECU.
 * @param pstReflashResultSendInfo          [IN]   const ADM_DIM_SOA_VDI_REFLASH_RESULT_INFO_T*  The result relfash information of the ECU reflash thrown by the sysi4vdi module.
 * @param priv                              [IN/OUT]    void*                               Reserved parameters
 * @return  D32S                            0:successed, Non-0:failed
 */
typedef D32S(*ADM_DIM_REFLASH_RESULT_SEND_CALLBACK_F)(
    const ADM_DIM_SOA_ECU_IDEN_INFO_T* pstIdenInfo,
    const ADM_DIM_SOA_VDI_REFLASH_RESULT_INFO_T* pstReflashResultSendInfo,
    void* priv);

/**
 * @brief Used to actively query the flashing result of an ECU
 *
 * @param instanceId        [IN]        const D16U  instance identifier
 * @param timeout               [IN]        const D16U  Timeout of sync interface, unit: second
 * @param pstEcuIdenInfo                    [IN]    const ADM_DIM_SOA_ECU_IDEN_INFO_T*  Structure used to uniquely identify an ECU.
 * @param pstReflashResultSendInfo          [OUT]   ADM_DIM_SOA_VDI_REFLASH_RESULT_INFO_T** The result relfash information of the ECU reflash thrown by the sysi4vdi module.
 * @param errorCode                         [OUT]   D32S* error code
 * @return D32S                             0:successed, Non-0:failed
 */
extern D32S adm_dim_vdi_queryReflashResult(const D16U instanceId,
        const D16U timeout, const ADM_DIM_SOA_ECU_IDEN_INFO_T* pstEcuIdenInfo,
        ADM_DIM_SOA_VDI_REFLASH_RESULT_INFO_T** pstReflashResultSendInfo,
        D32S* errorCode);
typedef D32S(*adm_dim_vdi_queryReflashResult_func)(const D16U instanceId,
        const D16U timeout, const ADM_DIM_SOA_ECU_IDEN_INFO_T* pstEcuIdenInfo,
        ADM_DIM_SOA_VDI_REFLASH_RESULT_INFO_T** pstReflashResultSendInfo,
        D32S* errorCode);

/***************************************************************************Do reflash ECUs**************************************************************************************/

/***************************************************************************Perform special diagnostic task**********************************************************************/
/**
 * @brief Definition of special diagnostic task types
 * vdi has been deleted  deleted
 */
typedef enum
{
    ADM_DIM_SOA_VDI_SPEC_DIAGNOSTIC_TASK_TYPE_FUNC_SAFE_VEFIFY = 1,        /**!< Functional safety verification */
    ADM_DIM_SOA_VDI_SPEC_DIAGNOSTIC_TASK_TYPE_HOST_PRE_PROGRAMMING,        /**!< The host device enters the pre-programming process */
    ADM_DIM_SOA_VDI_SPEC_DIAGNOSTIC_TASK_TYPE_PACKAGE_STORE_PATH,          /**!< Specify the storage path of the upgrade package */
    ADM_DIM_SOA_VDI_SPEC_DIAGNOSTIC_TASK_TYPE_ENTER_DIAG_MODE,             /**!< The vehicle enters the diagnostic mode */
    ADM_DIM_SOA_VDI_SPEC_DIAGNOSTIC_TASK_TYPE_EXIT_DIAG_MODE,              /**!< The vehicle exits the diagnostic mode */
    ADM_DIM_SOA_VDI_SPEC_DIAGNOSTIC_TASK_TYPE_INVALID                      /**!< Invalid */
} ADM_DIM_SOA_VDI_SPEC_DIAGNOSTIC_TASK_TYPE_E;

/**
 * @brief Perform specific diagnostic tasks.
 *
 * @param instanceId        [IN]        const D16U  instance identifier
 * @param timeout               [IN]        const D16U  Timeout of sync interface, unit: second
 * @param eSpecDiagTaskType      [IN]    const ADM_DIM_SOA_VDI_SPEC_DIAGNOSTIC_TASK_TYPE_E   Special diagnostic task types
 * @param absolutePath          [IN]    const DCHAR*    File path parameters required for special diagnostic tasks
 * @param errorCode         [OUT]   D32S*   The execution result of this request
 * @return D32S             0:successed, Non-0:failed
 *
 */
extern D32S adm_dim_vdi_performSpecDiagnosticTask(const D16U instanceId,
        const D16U timeout, const ADM_DIM_SOA_VDI_SPEC_DIAGNOSTIC_TASK_TYPE_E
        eSpecDiagTaskType, const DCHAR*  absolutePath, D32S* errorCode);
typedef D32S(*adm_dim_vdi_performSpecDiagnosticTask_func)(const D16U instanceId,
        const D16U timeout, const ADM_DIM_SOA_VDI_SPEC_DIAGNOSTIC_TASK_TYPE_E
        eSpecDiagTaskType, const DCHAR*  absolutePath, D32S* errorCode);

/***************************************************************************Perform special diagnostic task**********************************************************************/


/***************************************************************************Read vehicle status*********************************************************************************/
typedef struct ADM_DIM_SOA_MSGID_ST ADM_DIM_SOA_VDI_MSGID_T;

/**
 * @brief MsgId Array structure.
*/
typedef struct
{
    ADM_DIM_SOA_VDI_MSGID_T** msgIdArray;              /**!< Array of masId and vehicle state */
    D16U msgIdArrayLength;                          /**!< Array length of masId and vehicle state */
} ADM_DIM_SOA_VDI_MSGID_DATA_ARRAY_T;

/**
 * @brief Read vehicle status
 *
 * @param instanceId        [IN]        const D16U  instance identifier
 * @param timeout               [IN]        const D16U  Timeout of sync interface, unit: second
 * @param msgIdListRequest      [IN]    const ADM_DIM_SOA_VDI_VEHICLE_STATUS_TYPE_E @enum ADM_DIM_SOA_VDI_VEHICLE_STATUS_TYPE_E
 * @param msgIdListResponse     [OUT]   ADM_DIM_SOA_DATA_T**            Read the vehicle status variable
 * @param errorCode             [OUT]   D32S*   The execution result of this request
 * @return D32S
 */

extern D32S adm_dim_vdi_readVehicleStatus(const D16U instanceId, const D16U timeout,
        const ADM_DIM_SOA_VDI_MSGID_DATA_ARRAY_T* msgIdListRequest,
        ADM_DIM_SOA_VDI_MSGID_DATA_ARRAY_T** msgIdListResponse, D32S* errorCode);
typedef D32S(*adm_dim_vdi_readVehicleStatus_func)(const D16U instanceId, const D16U timeout,
        const ADM_DIM_SOA_VDI_MSGID_DATA_ARRAY_T* msgIdListRequest,
        ADM_DIM_SOA_VDI_MSGID_DATA_ARRAY_T** msgIdListResponse, D32S* errorCode);

/***************************************************************************Read vehicle status*********************************************************************************/

/***************************************************************************Spy vehicle status**********************************************************************************/
/**
 * @brief Set the list of vehicle status that needs to be monitored
 *
 * @param instanceId        [IN]        const D16U  instance identifier
 * @param timeout               [IN]        const D16U  Timeout of sync interface, unit: second
 * @param msgIdList  [IN]   const ADM_DIM_SOA_VDI_MSGID_DATA_ARRAY_T*         List of vehicle statuses that need to be spy
 * @param errorCode         [OUT]   D32S*   The execution result of this request
 * @return D32S         0:successed, Non-0:failed
 */
extern D32S adm_dim_vdi_setSpyVehicleStatus(const D16U instanceId,
        const D16U timeout, const D64U u64TaskId,
        const ADM_SYSI4VDI_ENGINE_SPY_ARRAY_T* spyList, D32S* errorCode);
typedef D32S(*adm_dim_vdi_setSpyVehicleStatus_func)(const D16U instanceId,
        const D16U timeout, const D64U u64TaskId,
        const ADM_SYSI4VDI_ENGINE_SPY_ARRAY_T* spyList, D32S* errorCode);

/**
 * @brief Clear the monitored vehicle status information.
 *
 * @param instanceId        [IN]        const D16U  instance identifier
 * @param timeout               [IN]        const D16U  Timeout of sync interface, unit: second
 * @param msgIdList         [IN]    const ADM_DIM_SOA_VDI_MSGID_DATA_ARRAY_T*     Monitored vehicle status information
 * @param errorCode         [OUT]   D32S*   The execution result of this request
 * @return D32S             0:successed, Non-0:failed
 */
extern D32S adm_dim_vdi_clearSpyVehicleStatus(const D16U instanceId, const D16U timeout,
        const D64U u64TaskId, D32S* errorCode);
typedef D32S(*adm_dim_vdi_clearSpyVehicleStatus_func)(const D16U instanceId, const D16U timeout,
        const D64U u64TaskId, D32S* errorCode);

/**
 * @brief Throw out the state of the monitored vehicle.
 *
 * @param msgIdList  [IN]   TaskId  Monitored vehicle status information.
 * @param priv                              [IN/OUT]    void*                               Reserved parameters
 * @return D32S          0:successed, Non-0:failed
 */
typedef D32S(*ADM_DIM_SPY_VEHICLE_STATUS_CALLBACK_F)(D64U u64TaskId, void* priv);
/***************************************************************************Spy vehicle status**********************************************************************************/

/**
 * @brief Control the vehicle into a certain state
 *
 * @param instanceId        [IN]        const D16U  instance identifier
 * @param timeout               [IN]        const D16U  Timeout of sync interface, unit: second
 * @param eVehicleStatusType [IN]   ADM_DIM_SOA_VDI_CTRL_VEHICLE_STATUS_TYPE_E     Vehicle status type
 * @param keeptime               [IN]        const D32U control keep time, unit: second
 * @param errorCode         [OUT]   D32S*   The execution result of this request
 *
 * @return D32S         0:successed, Non-0:failed
 */
extern D32S adm_dim_vdi_controlVehicleStatus(const D16U instanceId, const D16U timeout,
        ADM_DIM_SOA_VDI_CTRL_VEHICLE_STATUS_TYPE_E eVechielStatusType, ADM_DIM_SOA_DATA_T* pDiddate,
        const D32U keeptime, D32S* errorCode);
typedef D32S(*adm_dim_vdi_controlVehicleStatus_func)(const D16U instanceId, const D16U timeout,
        ADM_DIM_SOA_VDI_CTRL_VEHICLE_STATUS_TYPE_E eVechielStatusType, ADM_DIM_SOA_DATA_T* pDiddate,
        const D32U keeptime, D32S* errorCode);

/***************************************************************************Control vehicle status*****************************************************************************/


/**
 * @brief Throws the vehicle signal trigger event
 *
 * @param eVehicleSignalTriggerType     [IN]    const ADM_DIM_SOA_VDI_VEHICLE_SIGNAL_TRIGGER_TYPE_E      Vehicle signal trigger event type
 * @return D32S         0:successed, Non-0:failed
 */
typedef D32S(*ADM_DIM_VEHICLE_SIGNAL_EVENT_CALLBACK_F)(
    const ADM_DIM_SOA_VDI_VEHICLE_SIGNAL_TRIGGER_TYPE_E eVehicleSignalTriggerType);

/***************************************************************************Motinor vehicle signal event***********************************************************************/

/***************************************************************************Software version check process*********************************************************************/
/**
 * @brief Software version type of checking process
 * vdi has been deleted  deleted
 */
typedef enum
{
    ADM_DIM_SOA_VDI_SOFT_VERSION_CHECK_TYPE_OLD = 1,        ///< check old version of target ecu.
    ADM_DIM_SOA_VDI_SOFT_VERSION_CHECK_TYPE_NEW,            ///< check new version of target ecu.
    ADM_DIM_SOA_VDI_SOFT_VERSION_CHECK_TYPE_INVALID
} ADM_DIM_SOA_VDI_SOFT_VERSION_CHECK_TYPE_E;

/**
 * @brief Request ECU to perform version check.
 *
 * @param pstEcuIdenInfo        []
 * @param eCheckType
 * @param isEnterBorkenMode
 */

/**
 * @brief Request ECU to perform version check.
 *
 * @param instanceId            [IN]        const D16U  instance identifier
 * @param timeout               [IN]        const D16U  Timeout of sync interface, unit: second
 * @param pstEcuIdenInfo        [IN]        const ADM_DIM_SOA_ECU_IDEN_INFO_T*  Identification information of ECU
 * @param eCheckType            [IN]        const ADM_DIM_SOA_VDI_SOFT_VERSION_CHECK_TYPE_E Software version type of checking process
 * @param isEnterBorkenMode     [IN]        const DBOOL     After the software version check fails, do I need to enter the broken mode?
 * @param errorCode             [OUT]   D32S*   The execution result of this request
 */
extern D32S adm_dim_vdi_softwareVersionCheckProcess(const D16U instanceId,
        const D16U timeout, const ADM_DIM_SOA_ECU_IDEN_INFO_T* pstEcuIdenInfo,
        const ADM_DIM_SOA_VDI_SOFT_VERSION_CHECK_TYPE_E eCheckType,
        const DBOOL isEnterBorkenMode, D32S* errorCode);
typedef D32S(*adm_dim_vdi_softwareVersionCheckProcess_func)(
    const D16U instanceId, const D16U timeout,
    const ADM_DIM_SOA_ECU_IDEN_INFO_T* pstEcuIdenInfo,
    const ADM_DIM_SOA_VDI_SOFT_VERSION_CHECK_TYPE_E eCheckType,
    const DBOOL isEnterBorkenMode, D32S* errorCode);

/***************************************************************************Software version check process*********************************************************************/

extern D32S adm_dim_vdi_set_log_level(const D16U instanceId, const D16U timeout,
                                      D32U logLevel, D32S* errorCode);
typedef D32S(*adm_dim_vdi_set_log_level_func)(const D16U instanceId, const D16U timeout,
        D32U logLevel, D32S* errorCode);

/**
 * @brief 触发编程
 * @note 预编程和后编程
 * @param in const D16U usInstanceId    服务实例ID
 * @param in const D16U usTimeout       超时时间
 * @param in const DCHAR *pcRequestBuf  请求数据
 * @param out DCHAR **ppcResponseBuf    响应数据
 * @retval D32S 错误码
 */
extern D32S adm_dim_vdi_programEcusTrigger(const D16U usInstanceId,
                                           const D16U usTimeout,
                                           const DCHAR *pcRequestBuf,
                                           DCHAR **ppcResponseBuf);

/**
 * @brief 触发编程
 * @note 预编程和后编程
 * @param in const D16U usInstanceId    服务实例ID
 * @param in const D16U usTimeout       超时时间
 * @param in const DCHAR *pcRequestBuf  请求数据
 * @param out DCHAR **ppcResponseBuf    响应数据
 * @retval D32S 错误码
 */
typedef D32S(*adm_dim_vdi_programm_trigger_func)(const D16U usInstanceId,
                                                const D16U usTimeout,
                                                const DCHAR *pcRequestBuf,
                                                DCHAR **ppcResponseBuf);


/**
 * @brief 获取零件当前处于A/B分区
 * 
 * @param[in] uiInstanceId     描述: 服务实例ID \n
 * 范围值: 无符号短整型 \n
 * 单位: N/A \n
 * 
 * @param[in] uiTimeout     描述: 超时时间 \n
 * 范围值: 无符号短整型 \n
 * 单位: N/A \n
 * 
 * @param[in] pstEcuIdenInfo     描述: 零件信息 \n
 * 范围值: 指针 \n
 * 单位: N/A \n
 * 
 * @param[in] eBusType     描述: 业务类型 \n
 * 范围值: 枚举 \n
 * 单位: N/A \n
 * 
 * @param[in] eChannelType     描述: 通道 \n
 * 范围值: 枚举 \n
 * 单位: N/A \n
 * 
 * @param[out] pAreaABEnable     描述: 是否AB件 \n
 * 范围值: 指针 \n
 * 单位: N/A \n
 * 
 * @param[out] pAreaInfo     描述: 当前处于A/B分区 \n
 * 范围值: 指针 \n
 * 单位: N/A \n
 * 
 * @param[out] pErrorCode     描述: 处理码值 \n
 * 范围值: 无符号整型 \n
 * 单位: N/A \n
 * 
 * @return D32S 
 */
extern D32S adm_dim_vdi_readAbInfo(const D16U uiInstanceId,
        const D16U uiTimeout,
        const ADM_DIM_SOA_ECU_IDEN_INFO_T* pstEcuIdenInfo,
        const ADM_DIM_SOA_VDI_BUS_TYPE_E eBusType,
        const ADM_DIM_SOA_PIN_CHANNEL_TYPE_E eChannelType,
        DBOOL* pAreaABEnable,
        ADM_DIM_SOA_VDI_ECU_AREA_INFO_E *pAreaInfo,
        D32U* pErrorCode);
typedef D32S(*adm_dim_vdi_readAbInfo_func)(const D16U uiInstanceId,
        const D16U uiTimeout,
        const ADM_DIM_SOA_ECU_IDEN_INFO_T* pstEcuIdenInfo,
        const ADM_DIM_SOA_VDI_BUS_TYPE_E eBusType,
        const ADM_DIM_SOA_PIN_CHANNEL_TYPE_E eChannelType,
        DBOOL* pAreaABEnable,
        ADM_DIM_SOA_VDI_ECU_AREA_INFO_E *pAreaInfo,
        D32U* pErrorCode);

/**
 * @brief 切换A/B分区
 * 
 * @param[in] uiInstanceId     描述: 服务实例ID \n
 * 范围值: 无符号短整型 \n
 * 单位: N/A \n
 * 
 * @param[in] uiTimeout     描述: 超时时间 \n
 * 范围值: 无符号短整型 \n
 * 单位: N/A \n
 * 
 * @param[in] pstEcuIdenInfo     描述: 零件信息 \n
 * 范围值: 指针 \n
 * 单位: N/A \n
 * 
 * @param[in] pSwitchScriptPath     描述: 切换分区脚本 \n
 * 范围值: 指针 \n
 * 单位: N/A \n
 * 
 * @param[in] eBusType     描述: 业务类型 \n
 * 范围值: 枚举 \n
 * 单位: N/A \n
 * 
 * @param[in] eChannelType     描述: 通道 \n
 * 范围值: 枚举 \n
 * 单位: N/A \n
 * 
 * @param[out] eDstAreaInfo     描述: 目的分区 \n
 * 范围值: 枚举 \n
 * 单位: N/A \n
 * 
 * @param[out] pstSaMaskInfo     描述: 安全算法列表 \n
 * 范围值: 指针 \n
 * 单位: N/A \n
 * 
 * @param[out] pErrorCode     描述: 处理码值 \n
 * 范围值: 无符号整型 \n
 * 单位: N/A \n
 * 
 * @return D32S 
 */
extern D32S adm_dim_vdi_switchAb(const D16U uiInstanceId,
        const D16U uiTimeout,
        const ADM_DIM_SOA_ECU_IDEN_INFO_T* pstEcuIdenInfo,
        const DCHAR *pSwitchScriptPath,
        const ADM_DIM_SOA_VDI_BUS_TYPE_E eBusType,
        const ADM_DIM_SOA_PIN_CHANNEL_TYPE_E eChannelType,
        const ADM_DIM_SOA_VDI_ECU_AREA_INFO_E eDstAreaInfo,
        ADM_DIM_SOA_SA_MASK_INFO_T *pstSaMaskInfo,
        D32U* pErrorCode);
typedef D32S(*adm_dim_vdi_switchAb_func)(const D16U uiInstanceId,
        const D16U uiTimeout,
        const ADM_DIM_SOA_ECU_IDEN_INFO_T* pstEcuIdenInfo,
        const DCHAR *pSwitchScriptPath,
        const ADM_DIM_SOA_VDI_BUS_TYPE_E eBusType,
        const ADM_DIM_SOA_PIN_CHANNEL_TYPE_E eChannelType,
        const ADM_DIM_SOA_VDI_ECU_AREA_INFO_E eDstAreaInfo,
        ADM_DIM_SOA_SA_MASK_INFO_T *pstSaMaskInfo,
        D32U* pErrorCode);

/**
 * @brief 触发重启
 * @param in const D16U usInstanceId    服务实例ID
 * @param in const D16U usTimeout       超时时间
 * @param out D32S *errorCode          错误码
 * @retval D32S 错误码
 */
extern D32S adm_dim_vdi_programReboot(const D16U usInstanceId,
                                      const D16U usTimeout,
                                      D32S *errorCode);
typedef D32S(*adm_dim_vdi_programm_reboot_func)(const D16U usInstanceId,
                                                const D16U usTimeout,
                                                D32S *errorCode);

/**
 * @brief 设置诊断通道
 * @param in const D16U usInstanceId    服务实例ID
 * @param in const D16U usTimeout       超时时间
 * @param out D32S *errorCode          错误码
 * @return D32S 错误码
 */
extern D32S adm_dim_vdi_setDiagTp(const D16U usInstanceId,
                                  const D16U usTimeout,
                                  D32S *errorCode);
typedef D32S (*adm_dim_vdi_setDiagTp_func)(const D16U usInstanceId,
                                           const D16U usTimeout,
                                           D32S *errorCode);
/***************************************************************************SOA-DIM-VDI initialization*************************************************************************/
/**
 * @brief DIM-SOA-VDI initialization structure definition
 *
 */
typedef struct
{
    ADM_DIM_SOA_VDI_READ_DIAG_INFO_CALLBACK_F
    readDiagInfoCallback;               /**!< Callback function to notify read diagnostic information */
    ADM_DIM_REFLASH_PROGRESS_NOTIFY_CALLBACK_F
    reflashProgressCallback;             /**!< Callback function to handle flashing progress */
    ADM_DIM_GET_VEHICLE_TYPE_CALLBACK_F getVehicleTypeCallback;
    ADM_DIM_REFLASH_RESULT_SEND_CALLBACK_F reflashResultCallback;               /**!< Callback function to handle flashing result */
    ADM_DIM_SPY_VEHICLE_STATUS_CALLBACK_F
    spyVehicleStatusCallback;           /**!< Callback function for handling vehicle status monitoring */
    ADM_DIM_VEHICLE_SIGNAL_EVENT_CALLBACK_F
    vehicleSignalEventCallback;         /**!< Callback function for handling vehicle signal event monitoring */
} ADM_DIM_SOA_VDI_INIT_INFO_T;

/**
 * @brief The DIM-SOA-VDI module initializes the interface
 * @note Use adm_dim_vdi_deinit to release the initialization information
 *
 * @param instanceId    [IN]        D16S                                multi sysi instance
 * @param initInfo      [IN]        ADM_DIM_SOA_VDI_INIT_INFO_T*        The SOA-DIM-VDI module initializes information, passing in the callback function required by the module
 * @param priv          [IN/OUT]    void*                               Reserved parameters
 * @return D32S         0:successed, Non-0:failed
 */
extern D32S adm_dim_vdi_init(ADM_DIM_SOA_VDI_INIT_INFO_T* initInfo, void* priv);
typedef D32S(*adm_dim_vdi_init_func)(D16U instanceId, ADM_DIM_SOA_VDI_INIT_INFO_T* initInfo, void* priv);

/**
 * @brief Release initialization information
 * @note Used to release the information initialized by the adm_dim_vdi_init function
 */
extern void adm_dim_vdi_deinit(void);
typedef void (*adm_dim_vdi_deinit_func)(D16U instanceId);

/***************************************************************************SOA-DIM-VDI initialization*************************************************************************/


#ifdef __cplusplus  // close out "C" linkage in case of c++ compiling
}
#endif

#endif//__ADM_DIM_SOA_VDI_H__
