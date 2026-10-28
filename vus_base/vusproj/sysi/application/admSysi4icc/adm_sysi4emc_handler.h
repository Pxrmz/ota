#ifndef __ADM_SYSI4EMC_HANDLER_H__
#define __ADM_SYSI4EMC_HANDLER_H__

#ifdef __cplusplus
extern "C" {
#endif


#include "adm_typedefs.h"

/**
 * @brief 车辆状态控制类型定义
 * 
 */
typedef enum
{
    ADM_SYSI4HMI_EMC_VEHICLE_STATE_CTRL_HIGH_VOLTAGE_ON = 1,        ///< 上高压
    ADM_SYSI4HMI_EMC_VEHICLE_STATE_CTRL_HIGH_VOLTAGE_OFF,       ///< 下高压
    ADM_SYSI4HMI_EMC_VEHICLE_STATE_CTRL_OTA_MODE_ENTER,         ///< 进入OTA模式
    ADM_SYSI4HMI_EMC_VEHICLE_STATE_CTRL_OTA_MODE_EXIT,          ///< 退出OTA模式
    ADM_SYSI4HMI_EMC_VEHICLE_STATE_CTRL_REMOTE_POWER_ON,        ///< 远程上电
    ADM_SYSI4HMI_EMC_VEHICLE_STATE_CTRL_REMOTE_POWER_OFF,       ///< 远程下电
    ADM_SYSI4HMI_EMC_VEHICLE_STATE_CTRL_EXT1,                   ///< 扩展控制类型 1
    ADM_SYSI4HMI_EMC_VEHICLE_STATE_CTRL_EXT2,                   ///< 扩展控制类型 2
    ADM_SYSI4HMI_EMC_VEHICLE_STATE_CTRL_EXT3,                   ///< 扩展控制类型 3
    ADM_SYSI4HMI_EMC_VEHICLE_STATE_CTRL_EXT4,                   ///< 扩展控制类型 4
} ADM_SYSI4HMI_EMC_VEHICLE_STATE_CTRL_TYPE_E;

/**
 * @brief 车辆状态控制
 * 
 * @param[in] eCtrlType     描述:车辆状态控制类型定义 \n
 * 范围值: \n
 * 单位: \n
 * 
 * @param[out] errorCode     描述:车辆状态控制结果 0:Successed None-0:Failed \n
 * 范围值: \n
 * 单位: \n
 * 
 * @return D32S 
 */
extern D32S adm_sysi4hmi_emcVehicleStateCtrl(const ADM_SYSI4HMI_EMC_VEHICLE_STATE_CTRL_TYPE_E eCtrlType, D32S* errorCode);

/**
 * @brief 切换OTA车端环境
 * 
 * @param[out] pServerType     描述:车端环境类型标识 1: ini中配置的默认环境; 2: ini中配置的第二套环境 \n
 * 范围值: \n
 * 单位: \n
 * 
 * @param[out] errorCode     描述:车端环境切换的结果 0:Successed None-0:Failed \n
 * 范围值: \n
 * 单位: \n
 * 
 * @return D32S 
 */
extern D32S adm_sysi4hmi_emcOTAServerSwitch(D32U* pServerType, D32S* errorCode);

/**
 * @brief 校验 主控与OTA服务端之间连接是否OK
 * 
 * @param[out] errorCode     描述:结果 0:Successed None-0:Failed \n
 * 范围值: \n
 * 单位: \n
 * 
 * @return D32S 
 */
extern D32S adm_sysi4hmi_emcOTAServerConnectOk(D32S* errorCode);

/**
 * @brief OTA 车端软件控制类型
 * 
 */
typedef enum
{
    ADM_SYSI4HMI_EMC_OTA_APP_CTRL_CLEAN_DATA = 1,       ///< 清除OTA数据
    ADM_SYSI4HMI_EMC_OTA_APP_CTRL_STOP_UCM,         ///< 停止OTA主控
    ADM_SYSI4HMI_EMC_OTA_APP_CTRL_GET_LOG,          ///< 获取OTA程序日志
    ADM_SYSI4HMI_EMC_OTA_APP_CTRL_DEL_PACKAGE,      ///< 删除下载升级包
    ADM_SYSI4HMI_EMC_OTA_APP_CTRL_EXT1,             ///< 扩展控制类型 1
    ADM_SYSI4HMI_EMC_OTA_APP_CTRL_EXT2,             ///< 扩展控制类型 2
} ADM_SYSI4HMI_EMC_OTA_APP_CTRL_TYPE_E;

/**
 * @brief OTA 车辆软件控制接口
 * 
 * @param[in] eCtrlType     描述:OTA 车端软件控制类型 \n
 * 范围值: \n
 * 单位: \n
 * 
 * @param[out] errorCode     描述:OTA 车端软件控制结果， 0:Successed, None-0:Failed \n
 * 范围值: \n
 * 单位: \n
 * 注: ADM_SYSI4HMI_EMC_OTA_APP_CTRL_CLEAN_DATA / ADM_SYSI4HMI_EMC_OTA_APP_CTRL_STOP_UCM 两个类型无法返回结果哈  \n
 * 
 * 
 * @return D32S 
 */
extern D32S adm_sysi4hmi_emcOTAVehicleAppCtrl(const ADM_SYSI4HMI_EMC_OTA_APP_CTRL_TYPE_E eCtrlType, D32S* errorCode);


#ifdef __cplusplus
}
#endif

#endif//__ADM_SYSI4EMC_HANDLER_H__