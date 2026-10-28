#ifndef __ADM_VRM_REGISTER_H__
#define __ADM_VRM_REGISTER_H__

#include "adm_typedefs.h"
#include "adm_vrs_registerManager.h"
#include "adm_vrm_event.h"
#ifdef __cplusplus
extern "C" {
#endif


#define SDK_VERSION     "1.0.1.0.1"

//extern D32S adm_getVin(DCHAR** vin);

extern D32S adm_getSdkVersion(DCHAR** sdkVersion);

/**
 * @brief 执行注册，输入扩展信息，输出返回扩展信息
 * 
 * @param ext [IN]      与平台交互的扩展信息
 * @param rspInfo [IN]  平台返回的扩展信息
 * @return D32S     0:Successed(注册成功), None-0:Failed(注册失败)
 */
extern D32S adm_vrm_register(const DCHAR* ext, ADM_VRS_REGISTER_RSP_INFO_T* rspInfo);

/**
 * @brief 初始化事件上报接口
 * @param getSeqIdCb
 */
void adm_vrm_init(ADM_VRM_IRM_GET_EV_SEQID_CALLBACK_F getSeqIdCb, 
                                    ADM_VRM_IRM_REPORT_EVENT_CALLBACK_F reportEventCb);

/**
 * @brief 同步获取车辆动力类型（启动时使用）
 * @note 此接口会尝试从车机获取动力类型：
 *       - 数据库无数据：阻塞重试直到获取成功（30s间隔，无限循环）
 *       - 数据库有数据：重试3次（10s间隔），失败则使用数据库值
 * 
 * @return D32S     0:同步成功, non-0:同步失败
 */
extern D32S adm_vrm_syncVehiclePowerType(DVOID);

#ifdef __cplusplus
}
#endif

#endif//__ADM_VRM_REGISTER_H__
