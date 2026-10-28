#ifndef __ADM_EMS_UIM_H__
#define __ADM_EMS_UIM_H__

#include "adm_typedefs.h"
#include "adm_uim_ems.h"
#include "adm_ems_serverType.h"

#ifdef __cplusplus
extern "C" {
#endif


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
extern D32S adm_ems_vehicleStateCtrlCallback(const ADM_UIM_EMS_VEHICLE_STATE_CTRL_TYPE_E eCtrlType, D32S* errorCode);

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
extern D32S adm_ems_otaServerSwitchCallback(D32U* pServerType, D32S* errorCode);

/**
 * @brief 校验 主控与OTA服务端之间连接是否OK
 * 
 * @param[out] errorCode     描述:结果 0:Successed None-0:Failed \n
 * 范围值: \n
 * 单位: \n
 * 
 * @return D32S 
 */
extern D32S adm_ems_otaServerConnectCallback(D32S* errorCode);

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
extern D32S adm_ems_vehicleAppCtrlCallback(const ADM_UIM_EMS_OTA_APP_CTRL_TYPE_E eCtrlType, D32S* errorCode);


typedef void (*ADM_EMS_RESET_OTA_CALLBACK_F)(void);
typedef D32S (*ADM_EMS_GET_OTA_LOG_CALLBACK_F)(void);
typedef D32S (*ADM_EMS_DEL_PACKAGE_CALLBACK_F)(void);
typedef D32S (*ADM_EMS_OTA_SERVER_CONN_OK_CALLBAFK_F)(void);
typedef D32S (*ADM_EMS_STOP_UCM_CALLBACK_F)(void);
typedef void (*ADM_EMS_OTA_APP_CTRL_EXT1_CALLBACK_F)(void* priv);
typedef void (*ADM_EMS_OTA_APP_CTRL_EXT2_CALLBACK_F)(void* priv);

typedef struct ADM_EMS_INIT_INFO_
{
    ADM_EMS_RESET_OTA_CALLBACK_F emsResetOtaCB;         ///!< Clean OTA data & Reset OTA
    ADM_EMS_GET_OTA_LOG_CALLBACK_F emsGetOtaLogCB;      ///!< Get OTA log
    ADM_EMS_DEL_PACKAGE_CALLBACK_F emsDelPackageCB;     ///!< Delete package of OTA
    ADM_EMS_OTA_SERVER_CONN_OK_CALLBAFK_F emsOtaServerConnOkCB; ///!< Connect OTA server OK
    ADM_EMS_STOP_UCM_CALLBACK_F emsCtrlStopUcmCB;       ///!< Stop Ucm
    ADM_EMS_OTA_APP_CTRL_EXT1_CALLBACK_F emsCtrlExt1CB; ///!< Ctrl OTA APP ext1
    ADM_EMS_OTA_APP_CTRL_EXT2_CALLBACK_F emsCtrlExt2CB; ///!< Ctrl OTA APP ext2
} ADM_EMS_INIT_INFO_T;

/**
 * @brief 初始化EMS模块
 * 
 * @param[in/out] pstInitInfo     描述: \n
 * 范围值: \n
 * 单位: \n
 * 
 * @param[in/out] priv     描述: \n
 * 范围值: \n
 * 单位: \n
 * 
 * @return D32S 
 */
extern D32S adm_ems_init(ADM_EMS_INIT_INFO_T* pstInitInfo, void* priv);

/**
 * @brief 反初始化EMS模块
 * 
 */
extern void adm_ems_deinit(void);

#ifdef __cplusplus
}
#endif

#endif//__ADM_EMS_UIM_H__