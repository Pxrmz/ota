/**
 * @file adm_dim_soaHost.h
 * @brief 宿主设备上集成的模块，以服务化的形式为UC-Master提供的相关接口
 * @details 主要由宿主设备提供接口，用于OTA过程中需要宿主设备提供的支持接口。
 * @author Mengwei
 * @version v0.1.0
 * @date 2021-05-06
 * @copyright Copyright © 2018 All rights 上海艾拉比智能科技有限公司
 */
#ifndef __ADM_DIM_HOST_H__
#define __ADM_DIM_HOST_H__

#ifdef __cplusplus
extern "C" {
#endif

#define EXPORT_FUN __attribute__((visibility("default")))

#include "adm_typedefs.h"
#include "adm_application_common.h"
#include "adm_dim_common.h"

#define ADM_POWER_CONTROL_TIMEOUT 60        ///< 远程上下电超时时间
#define ADM_OTA_MODE_CONTROL_TIMEOUT 30     ///< OTA模式控制超时时间
#define ADM_HV_MODE_CONTROL_TIMEOUT 30     ///< 电压控制超时时间
#define ADM_WAKE_UP_CONTROL_TIMEOUT 30      ///< 唤醒控制超时时间

/*********************************** Get SDK Version *************************************/
/**
 * @brief 获取HOST的sdk版本号
 *
 * @param instanceId    [IN]    实例ID
 * @param timeout       [IN]    请求的超时时间
 * @param sdkVersion    [OUT]  SDK 版本号
 * @param errorCode     [OUT]  获取版本号的结果
 */
extern void adm_dim_host_getSdkVersion(const D16U instanceId, const D16U timeout, DCHAR** sdkVersion, D32S* errorCode);
typedef void (*adm_dim_host_getSdkVersion_func)(const D16U instanceId, const D16U timeout, DCHAR** sdkVersion, D32S* errorCode);
/*********************************** Get SDK Version *************************************/


/************************************ Get Vin*******************************************/
/**
 * @brief Request the host device to get vin.
 * @note sync interface
 *
 * @param instanceId            [IN]        const D16U  instance identifier
 * @param timeout               [IN]        const D16U  Timeout of sync interface, unit: second
 * @param vin                   [OUT]       DCHAR**     vin
 * @param errorCode             [OUT]       D32S*       The execution result of this request
 *
 * @return D32S                 0:successed, Non-0:failed
 */
extern D32S adm_dim_getVin(const D16U instanceId, const D16U timeout, DCHAR** vin, D32S* errorCode);
typedef D32S(*adm_dim_getVin_func)(const D16U instanceId, const D16U timeout, DCHAR** vin, D32S* errorCode);
/************************************ Get Vin*******************************************/

//获取icc车机显示版本号
extern D32S adm_dim_getVehicleVersion(const D16U instanceId, const D16U timeout, DCHAR** pVehicleVer, D32S* errorCode);
typedef D32S(*adm_dim_getVehicleVersion_func)(const D16U instanceId, const D16U timeout, DCHAR** pVehicleVer, D32S* errorCode);
extern D32S adm_dim_setVehicleVersion(const D16U instanceId, const D16U timeout, const DCHAR* pVehicleVer, D32S* errorCode);
typedef D32S(*adm_dim_setVehicleVersion_func)(const D16U instanceId, const D16U timeout, const DCHAR* pVehicleVer, D32S* errorCode);



extern D32S adm_dim_host_getNetworkType(const D16U instanceId, const D16U timeout, D32S* networkType, D32S* errorCode);
typedef D32S(*adm_dim_host_getNetworkType_func)(const D16U instanceId, const D16U timeout, D32S* networkType, D32S* errorCode);

/****************************** Set schdule time to wake up ***************************/
/**
 * @brief Set the scheduled wake-up time of the host device.
 *
 * @param instanceId        [IN]        const D16U  instance identifier
 * @param timeout               [IN]        const D16U  Timeout of sync interface, unit: second
 * @param eSchduleType      [IN]        const ADM_UIM_SOA_INSTALL_SCHDULE_TYPE_E eSchduleType
 * @param schduleTime       [IN]    const D64S  Schedule a wake-up time
 * @param errorCode         [OUT]   D32S*   The execution result of this request
 *
 * @return D32S             0:successed, Non-0:failed
 */

typedef enum
{
    ADM_DIM_HOST_SCHDULE_TYPE_SCHDULED = 1,      /**!< Schdule or Reschdule */
    ADM_DIM_HOST_SCHDULE_TYPE_CANCELED,          /**!< Cancel schdule */
    ADM_DIM_HOST_SCHDULE_TYPE_INVALID            /**!< Invalid */
}ADM_DIM_HOST_SCHDULE_TYPE_E;

extern D32S adm_dim_host_setSchduleWakeUpTime(const D16U instanceId, const D16U timeout,
        const ADM_DIM_HOST_SCHDULE_TYPE_E eSchduleType, const D64S schduleTime, D32S* errorCode);
typedef D32S(*adm_dim_host_setSchduleWakeUpTime_func)(const D16U instanceId, const D16U timeout,
        const ADM_DIM_HOST_SCHDULE_TYPE_E eSchduleType, const D64S schduleTime, D32S* errorCode);
/****************************** Set schdule time to wake up ***************************/


/****************************** Get schdule time to wake up ************************/
/**
 * @brief Get the scheduled wake-up time of the host device.
 *
 * @param instanceId        [IN]        const D16U  instance identifier
 * @param timeout               [IN]        const D16U  Timeout of sync interface, unit: second
 * @param schduleTime       [OUT]    D64S  *Schedule a wake-up time
 * @param errorCode         [OUT]   D32S*   The execution result of this request
 *
 * @return D32S             0:successed, Non-0:failed
 * author wj 20211208
 */
extern D32S adm_dim_host_getSchduleWakeUpTime(const D16U instanceId, const D16U timeout, D64S* schduleTime, D32S* errorCode);
typedef D32S(*adm_dim_host_getSchduleWakeUpTime_func)(const D16U instanceId, const D16U timeout, D64S* schduleTime,
        D32S* errorCode);
/****************************** Get schdule time to wake up ************************/


/***************************** Set & Get Vehicle big version ************************/
/**
 * @brief Set vehicle version information.
 *
 * @param instanceId        [IN]        const D16U  instance identifier
 * @param timeout               [IN]        const D16U  Timeout of sync interface, unit: second
 * @param vehicleBigVersion [IN]    const DCHAR*    Big version information of the vehicle
 * @param errorCode         [OUT]   D32S*   The execution result of this request
 *
 * @return D32S             0:successed, Non-0:failed
 */
extern D32S adm_dim_host_setVehicleBigVersion(const D16U instanceId, const D16U timeout, const DCHAR* vehicleBigVersion,
        D32S* errorCode);
typedef D32S(*adm_dim_host_setVehicleBigVersion_func)(const D16U instanceId, const D16U timeout, const DCHAR* vehicleBigVersion,
        D32S* errorCode);

/**
 * @brief Get vehicle version information.
 *
 * @param instanceId        [IN]        const D16U  instance identifier
 * @param timeout               [IN]        const D16U  Timeout of sync interface, unit: second
 * @param vehicleBigVersion [OUT]   DCHAR**     Big version information of the vehicle
 * @param errorCode         [OUT]   D32S*   The execution result of this request
 * @note vehicleBigVersion need to free
 * @return D32S             0:successed, Non-0:failed
 */
extern D32S adm_dim_host_getVehicleBigVersion(const D16U instanceId, const D16U timeout, DCHAR** vehicleBigVersion,
        D32S* errorCode);
typedef D32S(*adm_dim_host_getVehicleBigVersion_func)(const D16U instanceId, const D16U timeout, DCHAR** vehicleBigVersion,
        D32S* errorCode);
/***************************** Set & Get Vehicle big version ************************/


/*************************** Verify that the storage space is sufficient ******************/
/**
 * @brief Verify that the storage space is sufficient.
 *
 * @param instanceId        [IN]        const D16U  instance identifier
 * @param timeout               [IN]        const D16U  Timeout of sync interface, unit: second
 * @param needSize          [IN]    const D64S  The size of the required space, unit: byte
 * @param realSize          [OUT]   D64S*       The real space size, unit: byte
 * @param errorCode         [OUT]   D32S*   The execution result of this request
 *
 * @return D32S             0:successed, Non-0:failed
 */
extern D32S adm_dim_host_verifySpaceIsEnough(const D16U instanceId, const D16U timeout, const D64S needSize, D64S* realSize,
        D32S* errorCode);
typedef D32S(*adm_dim_host_verifySpaceIsEnough_func)(const D16U instanceId, const D16U timeout, const D64S needSize,
        D64S* realSize, D32S* errorCode);
/*************************** Verify that the storage space is sufficient ******************/



/**************************** Signature information verification *******************/
/**
 * @brief Signature verification.
 *
 * @param instanceId        [IN]        const D16U  instance identifier
 * @param timeout               [IN]        const D16U  Timeout of sync interface, unit: second
 * @param signedData        [IN]    const DCHAR*    Signature information
 * @param signedDataLength  [IN]    const D32S      Length of signature information
 * @param orignalData       [IN]    const DCHAR*    Signed original data
 * @param orignalDataLength [IN]    const D32S      Length of signed original data
 * @param errorCode         [OUT]   D32S*   The execution result of this request
 * @return D32S             0:successed, Non-0:failed
 */
extern D32S adm_dim_host_verifySignature(const D16U instanceId, const D16U timeout, const DCHAR* signedData,
        const D32S signedDataLength, const DCHAR* orignalData, const D32S orignalDataLength, D32S* errorCode);
typedef D32S(*adm_dim_host_verifySignature_func)(const D16U instanceId, const D16U timeout, const DCHAR* signedData,
        const D32S signedDataLength, const DCHAR* orignalData, const D32S orignalDataLength, D32S* errorCode);
/**************************** Signature information verification *******************/


/**************************** Get absolute path of cert **********************/
/**
 * @brief Certificate type definition.
 *
 */
typedef enum
{
    ADM_DIM_SOA_HOST_CERT_TYPE_SERVER_ROOT_CERT = 1,    /**!< Absolute path of server root cert */
    ADM_DIM_SOA_HOST_CERT_TYPE_CLIENT_CERT,             /**!< Absolute path of client cert */
    ADM_DIM_SOA_HOST_CERT_TYPE_PRIVATE_KEY,             /**!< Absolute path of client private key */
    ADM_DIM_SOA_HOST_CERT_TYPE_INVALID
} ADM_DIM_SOA_HOST_CERT_TYPE_E;

/**
 * @brief Obtain the absolute path of the certificate.
 *
 * @param instanceId        [IN]        const D16U  instance identifier
 * @param timeout               [IN]        const D16U  Timeout of sync interface, unit: second
 * @param eCertType     [IN]    const ADM_DIM_SOA_HOST_CERT_TYPE_E  Certificate type
 * @param certPath      [OUT]   DCHAR**     Absolute path of cert.
 * @param errorCode         [OUT]   D32S*   The execution result of this request
 * @return D32S         0:successed, Non-0:failed
 */
extern D32S adm_dim_host_getCertAbsolutePath(const D16U instanceId, const D16U timeout,
        const ADM_DIM_SOA_HOST_CERT_TYPE_E eCertType, DCHAR** certPath, D32S* errorCode);
typedef D32S(*adm_dim_host_getCertAbsolutePath_func)(const D16U instanceId, const D16U timeout,
        const ADM_DIM_SOA_HOST_CERT_TYPE_E eCertType, DCHAR** certPath, D32S* errorCode);
/**************************** Get absolute path of cert **********************/

/**
 * @brief Obtain the Privacy path.
 *
 * @param instanceId        [IN]        const D16U  instance identifier
 * @param timeout               [IN]        const D16U  Timeout of sync interface, unit: second
 * @param certPath      [OUT]   DCHAR**     Privacy path.
 * @param errorCode         [OUT]   D32S*   The execution result of this request
 * @return D32S         0:successed, Non-0:failed
 */
extern D32S adm_dim_host_getPrivacyPath(const D16U instanceId, const D16U timeout,
							 DCHAR** certPath, D32S* errorCode);
typedef D32S(*adm_dim_host_getPrivacy_func)(const D16U instanceId, const D16U timeout,
        					 DCHAR** certPath, D32S* errorCode);
/**************************** Get absolute path of cert **********************/

/**************************** Signal event notify ************************/
/**
 * @brief Definition of signal events type thrown by the host device.
 *
 */
typedef enum
{
    ADM_DIM_SIGNAL_EVENT_TYPE_SCHDULE_TIME_ARRIVED = 1,
    ADM_DIM_SIGNAL_EVENT_TYPE_SENSE_KEY_APPROACH = 2,
    ADM_DIM_SIGNAL_EVENT_TYPE_CLEAR_DTC = 3,            ///< 请求清除DTC
    ADM_DIM_SIGNAL_EVENT_TYPE_POWER_ON = 4,             ///< 车辆上电
    ADM_DIM_SIGNAL_EVENT_TYPE_POWER_OFF = 5,            ///< 车辆下电
    ADM_DIM_SIGNAL_EVENT_GEAR_P = 6,                    ///< 挡位切换P档
    ADM_DIM_SIGNAL_EVENT_GEAR_NONE_P = 7,               ///< 挡位切换非P档
    ADM_DIM_SIGNAL_EVENT_ANTI_THEFT_MODE_ON = 8,        ///< 设防（锁车）状态
    ADM_DIM_SIGNAL_EVENT_ANTI_THEFT_MODE_OFF = 9,       ///< 解防（非锁车）状态
    ADM_DIM_SIGNAL_EVENT_LFH_DOOR_OPEN = 10,            ///< 主驾门开
    ADM_DIM_SIGNAL_EVENT_LFH_DOOR_CLOSE = 11,           ///< 主驾门关
    ADM_DIM_SIGNAL_EVENTL_TYPE_INVALID = 0xFF           ///< Invalid
} ADM_DIM_SOA_HOST_SIGNAL_EVENT_TYPE_E;

/**
 * @brief The processing function of the signal event thrown by the host device.
 *
 * @param eSignalEventType      [IN]    const ADM_DIM_SOA_HOST_SIGNAL_EVENT_TYPE_E   signal events type thrown by the host device.
 * @param priv                  [IN/OUT]    void*                               Reserved parameters
 *
 * @return  D32S                0:successed, Non-0:failed
 *
 */
typedef D32S(*ADM_DIM_SOA_HOST_SIGNAL_EVENT_NOTIFY_CALLBACK_F)(const ADM_DIM_SOA_HOST_SIGNAL_EVENT_TYPE_E eSignalEventType,
        void* priv);
/**************************** Signal event notify ************************/


/**************************** Trigger event notify ************************/
/**
 * @brief Define the trigger event type of TSP push.
 *
 */
typedef enum
{
    ADM_DIM_SOA_HOST_TRIGGER_EVENT_TYPE_CHECK = 1,
    ADM_DIM_SOA_HOST_TRIGGER_EVENT_TYPE_DOWNLOAD,
    ADM_DIM_SOA_HOST_TRIGGER_EVENT_TYPE_INSTALL,
    ADM_DIM_SOA_HOST_TRIGGER_EVENT_TYPE_LOG_REPORT,
    ADM_DIM_SOA_HOST_TRIGGER_EVENT_TYPE_INVALID
} ADM_DIM_SOA_HOST_TRIGGER_EVENT_TYPE_E;

/**
 * @brief Functions used to handle trigger events of TSP push.
 *
 * @param eTriggerEventType     [IN]    const ADM_DIM_SOA_HOST_TRIGGER_EVENT_TYPE_E     trigger event type of TSP push
 * @param priv                  [IN/OUT]    void*                               Reserved parameters
 *
 * @return D32S                 0:successed, Non-0:failed
 */
typedef D32S(*ADM_DIM_SOA_HOST_TRIGGER_EVENT_NOTIFY_CALLBACK_F)(const ADM_DIM_SOA_HOST_TRIGGER_EVENT_TYPE_E eTriggerEventType,
        void* priv);
/**************************** Trigger event notify ************************/



/**************************** Exec OTA Task *****************************/
/**
 * @brief Type definition of the type of OTA task that the host device needs to provide.
 *
 */
typedef enum
{
    ADM_DIM_SOA_HOST_OTA_TASK_TYPE_START_OTA_ACTION = 0x01,    /**!< Request the execution of OTA actions, the user avoids conflicts with other tasks of the hosts */
    ADM_DIM_SOA_HOST_OTA_TASK_TYPE_STOP_OTA_ACTION,         /**!< Request to stop the execution of OTA action, the user avoids conflict with other tasks of the host */
    ADM_DIM_SOA_HOST_OTA_TASK_TYPE_START_WAKE_UP,           /**!< Request the host device to keep the device awake */
    ADM_DIM_SOA_HOST_OTA_TASK_TYPE_STOP_WAKE_UP,            /**!< Request that the host device can exit the awake state */
    ADM_DIM_SOA_HOST_OTA_TASK_TYPE_ENTER_VEHICLE_SILENT = 0x05,
    ADM_DIM_SOA_HOST_OTA_TASK_TYPE_EXIT_VEHICLE_SILENT = 0x06,
    ADM_DIM_SOA_HOST_OTA_TASK_TYPE_STOP_VEHICLE_SILENT = 0x07,
    ADM_DIM_SOA_HOST_OTA_TASK_TYPE_REMOTE_POWER_ON = 0x08,
    ADM_DIM_SOA_HOST_OTA_TASK_TYPE_REMOTE_POWER_OFF = 0x09,
    ADM_DIM_SOA_HOST_OTA_TASK_TYPE_ENTER_UPGRADE_MODE = 0x0A,
    ADM_DIM_SOA_HOST_OTA_TASK_TYPE_EXIT_UPGRADE_MODE = 0x0B,
    ADM_DIM_SOA_HOST_OTA_TASK_TYPE_INVALID                      /**!< Invalid */
} ADM_DIM_SOA_HOST_OTA_TASK_TYPE_E;

/**
 * @brief Request the host device to perform a specific OTA task.
 *
 * @param instanceId            [IN]        const D16U  instance identifier
 * @param timeout               [IN]        const D16U  Timeout of sync interface, unit: second
 * @param eOtaTaskType      [IN]    const ADM_DIM_SOA_HOST_OTA_TASK_TYPE_E  Type definition of the type of OTA task that the host device needs to provide.
 * @param keeptime          [IN]    const D32S specific ota task keep time, unit: second
 * @param errorCode         [OUT]   D32S*   The execution result of this request
 *
 * @return D32S             0:successed, Non-0:failed
 */
extern D32S adm_dim_host_execOtaTask(const D16U instanceId, const D16U timeout,
                                     const ADM_DIM_SOA_HOST_OTA_TASK_TYPE_E eOtaTaskType, const D32U keeptime, D32S* errorCode);
typedef D32S(*adm_dim_host_execOtaTask_func)(const D16U instanceId, const D16U timeout,
        const ADM_DIM_SOA_HOST_OTA_TASK_TYPE_E eOtaTaskType, const D32U keeptime, D32S* errorCode);
/**************************** Exec OTA Task *****************************/

/*************************** dim to host below *********************************/
//雄狮项目ICC特殊接口 below
//ICC零件标识 reauestID
typedef enum
{
    ADM_CUSTOM_ECU_TYPE_SOCMCU = 1,     ///< ICC SOC + MCU
    ADM_CUSTOM_ECU_TYPE_SOC = 2,               ///< SOC
    ADM_CUSTOM_ECU_TYPE_MCU = 3,               ///< MCU
    ADM_CUSTOM_ECU_TYPE_CARDASHBOARD = 4,      ///< 仪表屏
    ADM_CUSTOM_ECU_TYPE_CARPAD = 5,            ///< 中控屏
    ADM_CUSTOM_ECU_TYPE_AC = 6,                ///< 空调屏
    ADM_CUSTOM_ECU_TYPE_CEILING = 7,           ///< 吸顶屏
    ADM_CUSTOM_ECU_TYPE_INVALID                ///< 无效值
} ADM_CUSTOM_ECU_E;

extern D32S adm_dim_host_readVehicleStatus(const D16U instanceId, const D16U timeout,
        const ADM_DIM_SOA_MSGID_DATA_ARRAY_T* msgIdListRequest,
        ADM_DIM_SOA_MSGID_DATA_ARRAY_T** msgIdListResponse, D32S* errorCode);
typedef D32S(*adm_dim_host_readVehicleStatus_func)(const D16U instanceId, const D16U timeout,
        const ADM_DIM_SOA_MSGID_DATA_ARRAY_T* msgIdListRequest,
        ADM_DIM_SOA_MSGID_DATA_ARRAY_T** msgIdListResponse, D32S* errorCode);
/*************************** dim to host above *********************************/

//通知中心显示事件需要评审该接口
typedef enum
{
    ADM_CUSTOM_OTA_CHECK_NEW_VER,    ///< 检测到新版本
    ADM_CUSTOM_OTA_DOWNLOAD_ING,
    ADM_CUSTOM_OTA_DOWNLOAD_END,
    ADM_CUSTOM_OTA_INSTALL_ING,
    ADM_CUSTOM_OTA_INSTALL_END,
    ADM_CUSTOM_OTA_INVALID
} ADM_CUSTOM_OTA_TASKINFO_E;


extern D32S adm_dim_host_setVehicleStatus(const D16U instanceId, const D16U timeout, D32U msgType, D32S* errorCode);
typedef D32S(*adm_dim_host_setVehicleStatus_func)(const D16U instanceId, const D16U timeout, D32U msgType, D32S* errorCode);

//请求车机连接wifi
typedef struct
{
    DCHAR* pSsid;
    DCHAR* pPassword;
} ADM_HOST_WIFI_INFO_T;
typedef struct
{
    D16U uArrLen;
    ADM_HOST_WIFI_INFO_T pWifiList[1];
} ADM_HOST_WIFI_INFO_ARRAY_T;
extern D32S adm_dim_host_setWifiLink(const D16U instanceId, const D16U timeout,
                                     const ADM_HOST_WIFI_INFO_ARRAY_T* pWifiArray, D32S* errorCode);

extern D32S adm_dim_host_setPower(const D16U instanceId, const D16U timeout,
                              const D32S msgType, D32S* errorCode);
typedef D32S(*adm_dim_host_setWifiLink_func)(const D16U instanceId, const D16U timeout,
        const ADM_HOST_WIFI_INFO_ARRAY_T* pWifiArray, D32S* errorCode);

typedef D32S(*adm_dim_host_setPower_func)(const D16U instanceId, const D16U timeout,
        D32S msgType, D32S* errorCode);


extern D32S adm_dim_host_setUpgradeMode(const D16U instanceId, const D16U timeout, 
                                    D32S iUpgradeMode, D32S* errorCode);
typedef D32S (*adm_dim_host_setUpgradeMode_func)(const D16U instanceId, const D16U timeout, 
                                    D32S iUpgradeMode, D32S* errorCode);

/**
 * @brief 重启从控客户端
 * @note 
 * @param in const D16U instanceId  客户端实例Id
 * @param in const D16U timeout  超时时间，单位：秒
 * @param out D32S* errorCode  错误码
 * @return D32S  0:successed, Non-0:failed
 */
extern D32S adm_dim_host_reboot(const D16U instanceId, const D16U timeout, D32S* errorCode);
typedef D32S(*adm_dim_host_reboot_func)(const D16U instanceId, const D16U timeout, D32S* errorCode);


extern D32S adm_dim_host_downExceptionNotify(const D16U instanceId, const D16U timeout, 
                                    D32S iResult, D32S* errorCode);
typedef D32S (*adm_dim_host_downExceptionNotify_func)(const D16U instanceId, const D16U timeout, 
                                    D32S iResult, D32S* errorCode);


/**
 * @brief 获取OTA模式状态
 * @note
 * @param in const D16U instanceId  实例ID
 * @param in const D16U timeout  超时时间
 * @param out D32S *piOtaModeStatus  OTA模式状态 0:OTA模式OFF 1:OTA模式ON
 * @param out D32S *errorCode  错误码
 * @return D32S  返回值
 */
extern D32S adm_dim_host_getOtaModeStatus(const D16U instanceId, const D16U timeout, D32S *piOtaModeStatus, D32S* errorCode);
typedef D32S (*adm_dim_host_getOtaModeStatus_func)(const D16U instanceId, const D16U timeout, D32S *piOtaModeStatus, D32S* errorCode);

/**
 * @brief 获取TBOX远程诊断状态
 * @note
 * @param in const D16U uiInstanceId  实例ID
 * @param in const D16U uiTimeout  超时时间
 * @param out D32S *pTboxRemoteDiaStatus  TBOX远程诊断状态
 * @param out D32S *pErrorCode  错误码
 * @return D32S  返回值
 */
extern D32S adm_dim_host_getTboxRemoteDiaStatus(const D16U uiInstanceId, const D16U uiTimeout, D32S *pTboxRemoteDiaStatus, D32S* pErrorCode);
typedef D32S (*adm_dim_host_getTboxRemoteDiaStatus_func)(const D16U uiInstanceId, const D16U uiTimeout, D32S *pTboxRemoteDiaStatus, D32S *pErrorCode);

/**
 * @brief 紧急下电
 * @note
 * @param in const D16U instanceId  实例ID
 * @param in const D16U timeout  超时时间
 * @param out D32S *errorCode  错误码
 * @return D32S  返回值
 */
extern D32S adm_dim_host_setIcuPowerOff(const D16U instanceId, const D16U timeout, D32S* errorCode);
typedef D32S (*adm_dim_host_setIcuPowerOff_func)(const D16U instanceId, const D16U timeout, D32S* errorCode);

/**
 * @brief 让hmi通知车机OTA已准备就绪
 * @note
 * @param in const D16U instanceId  实例ID
 * @param in const D16U timeout  超时时间
 * @param out D32S *errorCode  错误码
 * @return D32S  返回值
 */
extern D32S adm_dim_host_notifyOtaHmiStart(const D16U instanceId, const D16U timeout, D32S *errorCode);
typedef D32S (*adm_dim_host_notifyOtaHmiStart_func)(const D16U instanceId, const D16U timeout, D32S *errorCode);

//获取鉴权算法安全密钥
//鉴权在ucm?
typedef struct
{
    DCHAR* pSc;
    DCHAR* pSk;
    DCHAR* reserved;
} ADM_HOST_SKSCINFO_T;
extern D32S adm_dim_host_getSKSCInfo(const D16U instanceId, const D16U timeout,
                                     ADM_HOST_SKSCINFO_T** pSkscinfo, D32S* errorCode);



/***************************************************************************Set & Clear diagnostic trouble code***********************************************************/
/**
 * @brief Set diagnostic trouble code.
 *
 * @param instanceId        [IN]        const D16U  instance identifier
 * @param timeout               [IN]        const D16U  Timeout of sync interface, unit: second
 * @param diagTroubleCode       [IN]    const D32S  diagnostic trouble code
 * @param dtcSnapShop           [IN]    const DCHAR*    Snapshot description of diagnostic trouble code.
 * @param errorCode         [OUT]   D32S*   The execution result of this request
 * @return D32S                 0:successed, Non-0:failed
 */
extern D32S adm_dim_host_setDiagTroubleCode(const D16U instanceId, const D16U timeout, const D32S diagTroubleCode,
        const DCHAR* dtcSnapShop, D32S* errorCode);
typedef D32S(*adm_dim_host_setDiagTroubleCode_func)(const D16U instanceId, const D16U timeout, const D32S diagTroubleCode,
        const DCHAR* dtcSnapShop, D32S* errorCode);


/**
 * @brief Clear diagnostic trouble code.
 *
 * @param instanceId        [IN]        const D16U  instance identifier
 * @param timeout               [IN]        const D16U  Timeout of sync interface, unit: second
 * @param diagTroubleCode       [IN]    const D32S  diagnostic trouble code
 * @param errorCode         [OUT]   D32S*   The execution result of this request
 * @return D32S                 0:successed, Non-0:failed
 */
extern D32S adm_dim_host_clearDiagTroubleCode(const D16U instanceId, const D16U timeout, const D32S diagTroubleCode,
        D32S* errorCode);
typedef D32S(*adm_dim_host_clearDiagTroubleCode_func)(const D16U instanceId, const D16U timeout, const D32S diagTroubleCode,
        D32S* errorCode);
/***************************************************************************Set & Clear diagnostic trouble code***********************************************************/

extern D32S adm_dim_host_set_log_level(const D16U instanceId, const D16U timeout,
                                       D32U logLevel, D32S* errorCode);
typedef D32S(*adm_dim_host_set_log_level_func)(const D16U instanceId, const D16U timeout,
        D32U logLevel, D32S* errorCode);

/******************************************* TSP 推送消息*****************************************/

/**
 * @brief TSP 推送任务的类型
 * 
 */
typedef enum
{
    ADM_DIM_SOA_TSP_ACTIVE_UPGRADE = 1,           ///!< 主动升级
    ADM_DIM_SOA_TSP_SLIENT_DOWNLOAD = 2,          ///!< 静默下载常规安装
    ADM_DIM_SOA_TSP_SLIENT_UPGRADE = 3,           ///!< 静默下载静默安装
    ADM_DIM_SOA_TSP_USB_UPGRADE = 11,             ///!< USB 升级
    ADM_DIM_SOA_TSP_RESCUE_UPGRADE = 12,          ///!< 救援升级
    ADM_DIM_SOA_TSP_PRODUCT_LINE_UPGRADE = 13,    ///!< 产线升级
    ADM_DIM_SOA_TSP_APK_SELF_UPGRADE = 14,        ///!< APK 自升级
    ADM_DIM_SOA_TSP_FORCE_UPGRADE = 15,           ///!< 强制升级
    ADM_DIM_SOA_TSP_OTA_ONE_CLICK_RESCUE = 16,    ///!< OTA 软件一键救援
    ADM_DIM_SOA_TSP_OTA_LOG_REPORT = 17,          ///!< OTA 日志上报
    ADM_DIM_SOA_TSP_APPOINT_UPGRADE = 18,         ///!< 预约升级，此时appointmentTimestamp必填
} ADM_DIM_SOA_TSP_TASK_TYPE_E;

/**
 * @brief TSP 推送的手机APP触发类型
 * 
 */
typedef enum
{
    ADM_DIM_SOA_TSP_OPERATION_INSTALL_NOW = 1,          ///!< 立即安装
    ADM_DIM_SOA_TSP_OPERATION_INSTALL_SCHDULE = 2,      ///!< 预约安装
    ADM_DIM_SOA_TSP_OPERATION_SCHDULE_UPDATE = 3,       ///!< 更新预约
    ADM_DIM_SOA_TSP_OPERATION_SCHDULE_CANCEL = 4,       ///!< 取消预约
} ADM_DIM_SOA_TSP_OPERATION_TYPE_E;

/**
 * @brief TSP 推送任务信息
 * 
 */
typedef struct 
{
    D32S iSource;                                     ///< 1-App   2-OTA云端;  注: source == 1时，operationType 才必填
    D64S ulTaskId;                                  ///< TSP 推送的任务ID, 必填
    ADM_DIM_SOA_TSP_TASK_TYPE_E eTaskType;         ///< TSP 推送的任务类型  必填
    ADM_DIM_SOA_TSP_OPERATION_TYPE_E eOperationType;                             ///< 1-立即安装  2-预约安装   3-更新预约   4-取消预约
    D64S appointmentTimestamp;                       ///< 预约升级的时间戳, operationType = 2 & operationType = 3 必填
} ADM_DIM_SOA_HOST_TSP_TASK_T;

// extern D32S adm_dim_host_tspTaskDeal(ADM_DIM_SOA_HOST_TSP_TASK_T* pstTspTask);
typedef D32S(*ADM_DIM_HOST_TSP_TASK_DEAL_CALLBACK_F)(const ADM_DIM_SOA_HOST_TSP_TASK_T* pstTspTask);

/******************************************* TSP 推送消息*****************************************/

typedef D32S (*ADM_DIM_HOST_HMI_EVENT_REPORT_CALLBACK_F)(DCHAR *pcEventInfo, DCHAR *pcEventExtInfo);

/***************************************************************************SOA-DIM-HOST init*****************************************************************************/
/**
 * @brief DIM-SOA-UCM initialization structure definition
 *
 */
typedef struct
{
    ADM_DIM_SOA_HOST_SIGNAL_EVENT_NOTIFY_CALLBACK_F
    signalEventNotifyCallback;          /**!< The processing function of the signal event thrown by the host device. */
    ADM_DIM_SOA_HOST_TRIGGER_EVENT_NOTIFY_CALLBACK_F
    triggerEventNotifyCallback;      /**!< Functions used to handle trigger events of TSP push. */
    ADM_DIM_HOST_TSP_TASK_DEAL_CALLBACK_F tspTaskDealCallback;  ///< TSP 推送请求消息的处理函数
    ADM_DIM_HOST_HMI_EVENT_REPORT_CALLBACK_F pfHmiEventReportCallback; /**< HMI事件上报处理函数 */
} ADM_DIM_SOA_HOST_INIT_INFO_T;

/**
 * @brief The DIM-SOA-HOST module initializes the interface
 * @note Use adm_dim_host_deinit to release the initialization information
 *
 * @param initInfo      [IN]        ADM_DIM_SOA_HOST_INIT_INFO_T*        The SOA-DIM-HOST module initializes information, passing in the callback function required by the module
 * @param priv          [IN/OUT]    void*                               Reserved parameters
 * @return D32S         0:successed, Non-0:failed
 */
extern D32S adm_dim_host_init(ADM_DIM_SOA_HOST_INIT_INFO_T* initInfo, void* priv);
typedef D32S(*adm_dim_host_init_func)(D16U instanceId, ADM_DIM_SOA_HOST_INIT_INFO_T* initInfo, void* priv);

/**
 * @brief Release initialization information
 * @note Used to release the information initialized by the adm_dim_host_init function
 */
extern void adm_dim_host_deinit(void);
typedef void (*adm_dim_host_deinit_func)(D16U instanceId);
/***************************************************************************SOA-DIM-HOST init*****************************************************************************/

#ifdef __cplusplus  // close out "C" linkage in case of c++ compiling
}
#endif
#endif//__ADM_DIM_HOST_H__