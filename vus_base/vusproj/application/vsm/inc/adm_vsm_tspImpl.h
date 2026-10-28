#ifndef __ADM_VSM_TSP_IMPL_H_
#define __ADM_VSM_TSP_IMPL_H_

#include "adm_typedefs.h"
#include "adm_application_common.h"
#include "adm_uim_common.h"
#include "adm_dim_host.h"
#include "adm_ipm_install.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 确认hmi状态是否已同步
 * 
 * @return DVOID 
 */
extern DVOID adm_vsm_handleHmiStatSync();

/**
 * @brief 任务状态上报
 * @note 
 * @param in D64S ulTaskId 任务ID
 * @param in pcState 任务状态
 * @param in iStateCode 任务状态码
 * @retval D32S 0 成功，其他失败
 */
extern D32S adm_vsm_reportState(D64S ulTaskId, DCHAR *pcState, D32S iStateCode, DCHAR *pcExts);

extern DBOOL adm_vsm_isCurrentPowerOn(void);
extern D32S adm_vsm_tspActionUpgrade(D64S ulTaskId, ADM_DIM_SOA_TSP_OPERATION_TYPE_E eOperationType);
extern D32S adm_vsm_tspSlientDownload(ADM_DIM_SOA_TSP_OPERATION_TYPE_E eOperationType);
extern D32S adm_vsm_tspSlientUpgrade(D64S ulTaskId);
extern D32S adm_vsm_tspAppointUpgrade(ADM_DIM_SOA_TSP_OPERATION_TYPE_E eOperationType, D64S appointmentTimestamp);
extern D32S adm_vsm_tspUsbUpgrade();
extern D32S adm_vsm_tspRescueUpgrade(D64S ulTaskId);
extern D32S adm_vsm_tspProductLineUpgrade(D64S ulTaskId);
extern D32S adm_vsm_tspApkSelfUpgrade(D64S ulTaskId);
extern D32S adm_vsm_tspForceUpgrade(D64S ulTaskId);
extern void adm_vsm_tspOtaOneClickRescue();
extern D32S adm_vsm_tspOtaLogReport();
extern D32S adm_vsm_execPowerOn();
extern D32S adm_vsm_execPowerOff();
extern D32S adm_vsm_entryHighHvMode();
extern D32S adm_vsm_exitHighHvMode();
extern D32S adm_vsm_entryOtaMode();
extern D32S adm_vsm_exitOtaMode();
extern D32S adm_vsm_stopWakeUp();
extern D32S adm_vsm_entryTspEnv();
extern DVOID adm_vsm_exitTspEnv();


typedef void(*ADM_VSM_TSP_OTA_ONE_CLICK_RESCUE_CALLBACK_F)();
/**
 * @brief The user triggers the setting of the scheduled installation time.
 * @note The appointment time set by the user is given to the host device and is reduced by 15 minutes by default to remind the user
 * 
 * @param eCmdidType        [IN]    const ADM_APPLICATION_COMMON_CMDID_TYPE_E    Type of detection trigger.
 * @param eSource           [IN]    const ADM_APPLICATION_SCHEDULE_INSTALL_SOURCE_E    Source of schdule install.
 * @param eSchduleType      [IN]    const ADM_UIM_SOA_INSTALL_SCHDULE_TYPE_E    Type of schdule install.
 * @param schduleTime       [IN]    const D16U  Schdule install timestamp, unit: second
 * @param eActionType       [OUT]   ADM_SYSI4HMI_ACTION_AUTH_TYPE_E*    Whether the requested trigger action is allowed
 * @param priv                              [IN/OUT]    void*                               Reserved parameters
 */

typedef D32S (*ADM_VSM_TSP_INSTALL_SCHDULED_CALLBACK_F)(const ADM_APPLICATION_COMMON_CMDID_TYPE_E eCmdidType, 
    const ADM_APPLICATION_SCHEDULE_INSTALL_SOURCE_E eSource, const ADM_UIM_SOA_INSTALL_SCHDULE_TYPE_E eSchduleType, 
    const D64S schduleTime, ADM_UIM_SOA_ACTION_AUTH_TYPE_E* eActionType, void* priv);


typedef D32S (*ADM_VSM_DIM_SOA_HOST_TSP_RR_MESSAGE_NOTIFY_CALLBACK_F)(const ADM_DIM_SOA_TSP_OPERATION_TYPE_E eOperationType, D64S iAppointmentTimestamp, void* priv);

typedef struct 
{
    ADM_VSM_DIM_SOA_HOST_TSP_RR_MESSAGE_NOTIFY_CALLBACK_F tspSchduleNotifyHmi;  ///!< 通知HMI的手机APP预约升级消息
    ADM_VSM_TSP_INSTALL_SCHDULED_CALLBACK_F schduleInstallCallback;             ///!< 预约升级
    ADM_VSM_TSP_OTA_ONE_CLICK_RESCUE_CALLBACK_F tspOneClickRescueCb;            ///!< OTA软件一键救援
} ADM_VSM_TSP_IMPL_T;



extern D32S adm_vsm_tspImplInit(ADM_VSM_TSP_IMPL_T* pstTspImplInfo);
extern D32S adm_vsm_tspImplDeinit(void);

#ifdef __cplusplus
}
#endif

#endif  //__ADM_VSM_TSP_IMPL_H_
