#ifndef __ADM_VSM_SIGNAL_IMPL_H_
#define __ADM_VSM_SIGNAL_IMPL_H_

#include "adm_typedefs.h"
#include "adm_application_common.h"
#include "adm_uim_common.h"
#include "adm_dim_host.h"
#include "adm_ipm_install.h"

#ifdef __cplusplus
extern "C"
{
#endif

extern D32S adm_vsm_waitCheckSubedEventSuccessed(D32S usServiceId, D16U usEventId);

/**
 * @brief 预约时间到达信号处理
 * 
 * @param[in] priv     描述: 私有参数 \n
 * 范围值: 指针 \n
 * 单位: N/A \n
 * 
 * @return DVOID 
 */
extern DVOID adm_vsm_schduleTimeArrived(DVOID *priv);

/**
 * @brief 上电信号处理
 * 
 * @return DVOID 
 */
extern DVOID adm_vsm_powerOnArrived();

/**
 * @brief 车辆挂P档信号处理
 * 
 * @return DVOID 
 */
extern DVOID adm_vsm_gearPArrived();

/**
 * @brief 车辆切非P档信号处理
 * 
 * @return DVOID 
 */
extern DVOID adm_vsm_gearNonePArrived();

#ifdef __cplusplus
}
#endif

#endif //__ADM_VSM_SIGNAL_IMPL_H_
