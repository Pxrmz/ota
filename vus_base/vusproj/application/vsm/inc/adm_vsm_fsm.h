/*!
 * \file
 * \brief 状态模块
 * \author huhongwei
 * \date 2021-11-16
 * \copyright www.abupdate.com
 */

#ifndef __ADM_VSM_FSM_H_
#define __ADM_VSM_FSM_H_

#include "adm_application_common.h"
#include "adm_dim_host.h"
#include "adm_dim_vdi.h"

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * 状态机信息。
 */
typedef enum
{
    ADM_VSM_STATE_IDLE = 0, /*!< 空闲 */
    ADM_VSM_STATE_CHECK_DOING, /*!< 检测中 */
    ADM_VSM_STATE_DOWNLOAD_READY, /*!< 下载就绪 */
    ADM_VSM_STATE_DOWNLOAD_DOING, /*!< 下载中 */
    ADM_VSM_STATE_DOWNLOAD_PAUSE, /*!< 下载暂停 */
    ADM_VSM_STATE_UPGRADE_READY, /*!< 安装就绪 */
    ADM_VSM_STATE_UPGRADE_DOING, /*!< 安装中 */
    ADM_VSM_STATE_UPGRADE_PAUSE, /*!< 安装暂停 */
    ADM_VSM_STATE_UPGRADE_BROKEN, /*!< 安装致命失败 */
    ADM_VSM_STATE_MAX,
} ADM_VSM_STATE_E;

/*!
 * 状态机初始化。
 */
void adm_vsm_init(void);

/*!
 * 状态机释放。
 */
void adm_vsm_deinit(void);

/*!
 * 状态机设置HMI在线标志。DBOOL TRUE:online, FALSE:offline
 */
void adm_vsm_setHmiOnline(DBOOL onLine);

/*!
 * 获取状态机HMI在线标志。
 */
DBOOL adm_vsm_getHmiOnline(void);

/**
 * @brief 获取HMI状态是否已同步
 * 
 * @return DBOOL 
 */
DBOOL adm_vsm_getHmiSyncStat(void);

/**
 * @brief 设置HMI状态同步
 * 
 * @param[in/out] bIsSync     描述: \n
 * 范围值: \n
 * 单位: N/A \n
 * 
 */
void adm_vsm_setHmiSyncStat(DBOOL bIsSync);


/*!
 * 状态机获取。
 */
ADM_VSM_STATE_E adm_vsm_stateGet(void);

/*!
 * 状态机设置。
 */
void adm_vsm_stateSet(ADM_VSM_STATE_E eState);

/**
 * @brief 状态机清除，重归IDLE
 *
 */
void adm_vsm_stateDel(void);

ADM_APPLICATION_COMMON_CMDID_TYPE_E adm_vsm_getTspTriggerType();

/*!
 * 触发检测
 * @return DBOOL TRUE:successed, FALSE:failed
 */
DBOOL adm_vsm_triggerCheck(ADM_APPLICATION_COMMON_CMDID_TYPE_E eTriggerType);

/*!
 * 触发下载。
 * @return DBOOL TRUE:successed, FALSE:failed
 */
DBOOL adm_vsm_triggerDownload(ADM_APPLICATION_COMMON_CMDID_TYPE_E eTriggerType);

/*!
 * 触发安装 触发方式由检测和下载决定。
 * @return DBOOL TRUE:successed, FALSE:failed
 */
DBOOL adm_vsm_triggerInstall(void);

/*!
 * 状态恢复，上电时调用。
 */
void adm_vsm_resume(void);

D32S adm_vsm_dimSoaVdiSignalEventNotify(const ADM_DIM_SOA_VDI_VEHICLE_SIGNAL_TRIGGER_TYPE_E eSignalEventType);

D32S adm_vsm_dimSoaHostSignalEventNotify(const ADM_DIM_SOA_HOST_SIGNAL_EVENT_TYPE_E eSignalEventType, void* priv);
D32S adm_vsm_dimSoaHostTspRRMessageNotify(const ADM_DIM_SOA_TSP_OPERATION_TYPE_E eOperationType, D64S iAppointmentTimestamp, void* priv);
D32S adm_vsm_dimSoaHostTriggerEventNotify(const ADM_DIM_SOA_HOST_TRIGGER_EVENT_TYPE_E eTriggerEventType, void* priv);
void* adm_vsm_dimSoaHostPrivGet(void);
D32S adm_vsm_dimSoaHostTspTaskDeal(const ADM_DIM_SOA_HOST_TSP_TASK_T* pstTspTask);

/**
 * @brief 【修改】获取车辆是否为燃油类型
 * @note  防呆接口，如果动力类型未确认或无效，返回错误
 *
 * @param[out] pbIsFuel     [Out] DBOOL* 是否为燃油车
 *                              DTRUE: 是燃油车
 *                              DFALSE: 不是燃油车
 *
 * @return D32S     0:成功获取动力类型
 *                  non-0:获取失败(动力类型无效或未确认)
 */
extern D32S adm_vsm_getIsFuel(DBOOL *pbIsFuel);


#ifdef __cplusplus
}
#endif

#endif  //__ADM_VSM_FSM_H_
