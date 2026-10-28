/**
 * @file adm_dim_vdiSpy.h
 * @brief   封装VDI已服务化的形式为UC-Master提供的监控的接口
 * @details 主要是封装由VDI提供的接口，用于OTA过程中的监控功能
 * @author yaoqiangqiang
 * @version v5.2.2
 * @date 2022-11-22
 * @copyright Copyright © 2018 All rights 上海艾拉比智能科技有限公司
 */
#ifndef __ADM_DIM_VDISPY_H__
#define __ADM_DIM_VDISPY_H__

#ifdef __cplusplus
extern "C" {
#endif
#include "adm_dim_vdi.h"
#include <pthread.h>

//最大监控数
#define SPY_VEHICLE_STATUS_MAX 20
//调用vdi接口超时时间
#define VDI_FUNC_TIMEOUT       3

/**
* @struct ADM_DIM_SPY_TASK_INFO_T
* @brief Vehicle spy task of  Array structure.
*/
typedef struct
{
    D64U iSpyTaskId;                                   //Task ID 
    ADM_DIM_SPY_VEHICLE_STATUS_CALLBACK_F cbSpyFunc;   //spy Task Call back function
} ADM_DIM_SPY_TASK_T;

/**
* @struct ADM_DIM_SPY_TASK_ARRAY_T
* @brief Vehicle spy task arr of Array structure.
*/
typedef struct
{
    D16U iTimeout;                      // callback timeout
    D64U iSpyGetTaskId;                 // 用于获取监控任务id
    D16U iSpyTaskSize;                  // Task of size
    ADM_DIM_SPY_TASK_T *pstSpyTaskArr[SPY_VEHICLE_STATUS_MAX];  ///< flexible array of vehicle spy task @see ADM_DIM_SPY_TASK_T
    pthread_mutex_t  stGetSpyTaskIdMutex;
} ADM_DIM_SPY_TASK_ARRAY_T;

/*!
 *******************************************************************************
 * dim模块 监控模块init
 *
 * \note 
 * 
 *******************************************************************************
 */
extern void adm_dim_vdiSpyInit();

/*!
 *******************************************************************************
 * dim模块 监控模块Deinit
 *
 * \note 
 * 
 *******************************************************************************
 */
extern void adm_dim_vdiSpyDeinit();

/*!
 *******************************************************************************
 * dim模块 监控任务
 * 
 * \note
 *
 * \param[in]      pstspyArr      车辆监控结构体
 * \param[in]      cbspyfunc      注册监控回调函数
 * \param[out]      pitaskId      返回的监控任务id
 * \retval         D32S           D32S    0: 成功 non-0：失败
 *******************************************************************************
 */
extern D32S adm_dim_vdiSpyRegisterSpyTask(ADM_SYSI4VDI_ENGINE_SPY_ARRAY_T *pstspyArr, ADM_DIM_SPY_VEHICLE_STATUS_CALLBACK_F cbspyfunc, D64U* pitaskId);

/*!
 *******************************************************************************
 * dim模块 取消监控任务
 * 
 * \note
 *
 * \param[in]      u64TaskId      监控任务id
 * \retval         D32S           D32S    0: 成功 non-0：失败
 *******************************************************************************
 */
extern D32S adm_dim_vdiSpyCancelSpyTask(D64U pitaskId);

/*!
 *******************************************************************************
 * dim模块 监控到事件后的回调函数
 * 
 * \note
 *
 * \param[in]      u64TaskId      监控任务id
 * \param[inout]   priv           预留
 * \retval         D32S           D32S    0: 成功 non-0：失败
 *******************************************************************************
 */
extern D32S adm_dim_vdiSpyVehicleStatusCallback(D64U u64TaskId, void *priv);

#ifdef __cplusplus  // close out "C" linkage in case of c++ compiling
}
#endif

#endif//__ADM_DIM_VDI_SPY_H__