/**
 * @file adm_uim_ems.h
 * @author mengwei (mengwei@abupdate.com)
 * @brief 工程模式服务端
 * @version 0.1
 * @date 2024-08-14
 * 
 * @copyright Copyright (c) 2024 上海艾拉比智能科技有限公司
 * 
 * @par 修改日志:
 * <table>
 * <tr><th>Date       <th>Version <th>Author  <th>Description
 * <tr><td>2024-08-14 <td>1.0     <td>mengwei    <td>内容
 * </table>
 */
#ifndef __ADM_UIM_EMS_H__
#define __ADM_UIM_EMS_H__


#ifdef __cplusplus
extern "C" {
#endif

#define EXPORT_FUN __attribute__((visibility("default")))

#include "adm_typedefs.h"

/**
 * @brief 车辆状态控制类型定义
 * 
 */
typedef enum
{
    ADM_UIM_EMS_VEHICLE_STATE_CTRL_HIGH_VOLTAGE_ON = 1,        ///< 上高压
    ADM_UIM_EMS_VEHICLE_STATE_CTRL_HIGH_VOLTAGE_OFF,       ///< 下高压
    ADM_UIM_EMS_VEHICLE_STATE_CTRL_OTA_MODE_ENTER,         ///< 进入OTA模式
    ADM_UIM_EMS_VEHICLE_STATE_CTRL_OTA_MODE_EXIT,          ///< 退出OTA模式
    ADM_UIM_EMS_VEHICLE_STATE_CTRL_REMOTE_POWER_ON,        ///< 远程上电
    ADM_UIM_EMS_VEHICLE_STATE_CTRL_REMOTE_POWER_OFF,       ///< 远程下电
    ADM_UIM_EMS_VEHICLE_STATE_CTRL_EXT1,                   ///< 扩展控制类型 1
    ADM_UIM_EMS_VEHICLE_STATE_CTRL_EXT2,                   ///< 扩展控制类型 2
    ADM_UIM_EMS_VEHICLE_STATE_CTRL_EXT3,                   ///< 扩展控制类型 3
    ADM_UIM_EMS_VEHICLE_STATE_CTRL_EXT4,                   ///< 扩展控制类型 4
} ADM_UIM_EMS_VEHICLE_STATE_CTRL_TYPE_E;


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
typedef D32S (*ADM_UIM_EMS_VEHICLE_STATE_CTRL_CALLBACK_F)(const ADM_UIM_EMS_VEHICLE_STATE_CTRL_TYPE_E eCtrlType, D32S* errorCode);


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
typedef D32S (*ADM_UIM_EMS_OTA_SERVER_SWITCH_CALLBACK_F)(D32U* pServerType, D32S* errorCode);


/**
 * @brief 校验 主控与OTA服务端之间连接是否OK
 * 
 * @param[out] errorCode     描述:结果 0:Successed None-0:Failed \n
 * 范围值: \n
 * 单位: \n
 * 
 * @return D32S 
 */
typedef D32S (*ADM_UIM_EMS_OTA_SERVER_CONN_OK_CALLBACK_F)(D32S* errorCode);


/**
 * @brief OTA 车端软件控制类型
 * 
 */
typedef enum
{
    ADM_UIM_EMS_OTA_APP_CTRL_CLEAN_DATA = 1,       ///< 清除OTA数据
    ADM_UIM_EMS_OTA_APP_CTRL_STOP_UCM,         ///< 停止OTA主控
    ADM_UIM_EMS_OTA_APP_CTRL_GET_LOG,          ///< 获取OTA程序日志
    ADM_UIM_EMS_OTA_APP_CTRL_DEL_PACKAGE,      ///< 删除下载升级包
    ADM_UIM_EMS_OTA_APP_CTRL_EXT1,             ///< 扩展控制类型 1
    ADM_UIM_EMS_OTA_APP_CTRL_EXT2,             ///< 扩展控制类型 2
} ADM_UIM_EMS_OTA_APP_CTRL_TYPE_E;

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
 * 注: ADM_UIM_EMS_OTA_APP_CTRL_CLEAN_DATA / ADM_UIM_EMS_OTA_APP_CTRL_STOP_UCM 两个类型无法返回结果哈  \n
 * 
 * 
 * @return D32S 
 */
typedef D32S (*ADM_UIM_EMS_OTA_VEHICLE_APP_CTRL_CALLBACK_F)(const ADM_UIM_EMS_OTA_APP_CTRL_TYPE_E eCtrlType, D32S* errorCode);


/**
 * @brief Engineer mode 初始化结构体定义
 * 
 */
typedef struct ADM_UIM_EMS_INIT_INFO_
{
    ADM_UIM_EMS_VEHICLE_STATE_CTRL_CALLBACK_F   emsVehicleStateCtrlCallback;//< OTA 车辆状态控制
    ADM_UIM_EMS_OTA_SERVER_SWITCH_CALLBACK_F    emsOTAServerSwitchCallback; //< OTA 平台环境切换验证
    ADM_UIM_EMS_OTA_SERVER_CONN_OK_CALLBACK_F   emsOTAServerConnCallback;   //< OTA 平台连通性校验
    ADM_UIM_EMS_OTA_VEHICLE_APP_CTRL_CALLBACK_F emsOTAAppCtrlCallback;      //< OTA 车端软件控制
} ADM_UIM_EMS_INIT_INFO_T;


/**
 * @brief OTA engineer mode 服务端接口初始化
 * 
 * @param pstInitInfo   [IN]        const ADM_UIM_EMS_INIT_INFO_T*
 * @param priv          [IN/OUT]    void*                               Reserved parameters
 * @return D32S         0:successed, Non-0:failed    
 */
extern D32S adm_uim_emsInit(const ADM_UIM_EMS_INIT_INFO_T* initInfo, void* priv);
typedef D32S (*adm_uim_emsInit_func)(const ADM_UIM_EMS_INIT_INFO_T* initInfo, void* priv);


/**
 * @brief Release initialization information
 */
extern D32S adm_uim_emsDeinit(void);
typedef D32S (*adm_uim_emsDeinit_func)(void);


#ifdef __cplusplus  // close out "C" linkage in case of c++ compiling
}
#endif



#endif//__ADM_UIM_EMS_H__