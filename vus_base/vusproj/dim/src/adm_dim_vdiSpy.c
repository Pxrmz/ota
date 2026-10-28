#include <pthread.h>

#include "adm_dim_vdiSpy.h"
#include "adm_com_errorCode.h"
#include "adm_dim_log.h"
#include "adm_typedefs.h"

#include "adm_memory.h"
#include "adm_string.h"
#include "adm_soa.h"

//监控任务数组结构体
static ADM_DIM_SPY_TASK_ARRAY_T s_stSpyTaskArr;

/*!
 *******************************************************************************
 * \brief 获取监控任务id
 *
 * \retval D64U 监控任务id
 *******************************************************************************
 */
static D64U __adm_dim_vdiSpyGetSpyTaskId()
{
    (void)pthread_mutex_lock(&s_stSpyTaskArr.stGetSpyTaskIdMutex);
    s_stSpyTaskArr.iSpyGetTaskId += 1;
    (void)pthread_mutex_unlock(&s_stSpyTaskArr.stGetSpyTaskIdMutex);
    return s_stSpyTaskArr.iSpyGetTaskId;
}

/*!
 *******************************************************************************
 * dim模块 向监控任务数组增加任务
 *
 * \note
 *
 * \param[in]      itaskId        监控任务id
 * \param[in]      cbSpyFunc      注册监控回调函数
 * \retval         D32S           D32S    0: 成功 non-0：失败
 *******************************************************************************
 */
static D32S __adm_dim_vdiSpyTaskAdd(D64U iTaskId, ADM_DIM_SPY_VEHICLE_STATUS_CALLBACK_F cbSpyFunc)
{
    if(0 == iTaskId || NULL == cbSpyFunc)
    {
        return ADM_ECODE_INVALID_PARAM;
    }
    //新建ADM_DIM_SPY_TASK_T
    ADM_DIM_SPY_TASK_T * pstspytask = (ADM_DIM_SPY_TASK_T *)adm_malloc(sizeof(ADM_DIM_SPY_TASK_T));
    if(NULL == pstspytask)
    {
        return ADM_ECODE_NO_MEMORY;
    }
    pstspytask->iSpyTaskId = iTaskId;
    pstspytask->cbSpyFunc = cbSpyFunc;
    (void)pthread_mutex_lock(&s_stSpyTaskArr.stGetSpyTaskIdMutex);
    if(s_stSpyTaskArr.iSpyTaskSize < SPY_VEHICLE_STATUS_MAX)
    {
        s_stSpyTaskArr.pstSpyTaskArr[s_stSpyTaskArr.iSpyTaskSize] = pstspytask;
        s_stSpyTaskArr.iSpyTaskSize += 1;
    }
    else
    {
        log_e("The number of monitors reached the maximum");
    }
    (void)pthread_mutex_unlock(&s_stSpyTaskArr.stGetSpyTaskIdMutex);
    return ADM_ECODE_SUCCESSED;
}

/*!
 *******************************************************************************
 * dim模块 从监控任务数组中删除任务
 *
 * \note
 *
 * \param[in]      itaskId        监控任务id
 * \retval         D32S           D32S    0: 成功 non-0：失败
 *******************************************************************************
 */
static D32S __adm_dim_vdiSpyTaskDel(D64U itaskId)
{
    if(0 == itaskId)
    {
        return ADM_ECODE_INVALID_PARAM;
    }
    (void)pthread_mutex_lock(&s_stSpyTaskArr.stGetSpyTaskIdMutex);
    for(int i = 0; i < s_stSpyTaskArr.iSpyTaskSize; i++)
    {
        if(itaskId == s_stSpyTaskArr.pstSpyTaskArr[i]->iSpyTaskId)
        {
            s_stSpyTaskArr.iSpyTaskSize -= 1;
            if(NULL != s_stSpyTaskArr.pstSpyTaskArr[i])
            {
                adm_free(s_stSpyTaskArr.pstSpyTaskArr[i]);
                s_stSpyTaskArr.pstSpyTaskArr[i] = NULL;
            }
            break;
        }
    }
    (void)pthread_mutex_unlock(&s_stSpyTaskArr.stGetSpyTaskIdMutex);
    return ADM_ECODE_SUCCESSED;
}

/*!
 *******************************************************************************
 * dim模块 从监控任务数组查询任务
 *
 * \note
 *
 * \param[in]      itaskId               监控任务id
 * \retval         ADM_DIM_SPY_TASK_T *  找到的监控任务
 *******************************************************************************
 */
static ADM_DIM_SPY_TASK_T *__adm_dim_vdiSpyFindTask(D64U itaskId)
{
    ADM_DIM_SPY_TASK_T * pstspyTask = NULL;
    if(0 == itaskId)
    {
        return NULL;
    }
    (void)pthread_mutex_lock(&s_stSpyTaskArr.stGetSpyTaskIdMutex);
    for(int i = 0; i < s_stSpyTaskArr.iSpyTaskSize; i++)
    {
        if(itaskId == s_stSpyTaskArr.pstSpyTaskArr[i]->iSpyTaskId)
        {
            if(NULL != s_stSpyTaskArr.pstSpyTaskArr[i])
            {
                pstspyTask = s_stSpyTaskArr.pstSpyTaskArr[i];
            }
            break;
        }
    }
    (void)pthread_mutex_unlock(&s_stSpyTaskArr.stGetSpyTaskIdMutex);
    return pstspyTask;
}

/*!
 *******************************************************************************
 * dim模块 监控模块init
 *
 * \note 
 * 
 *******************************************************************************
 */
void adm_dim_vdiSpyInit()
{
    s_stSpyTaskArr.iTimeout = VDI_FUNC_TIMEOUT;      //vdi回调等待超时时间，初始化为3秒
    s_stSpyTaskArr.iSpyGetTaskId = 0; //获取TaskId初始化
    s_stSpyTaskArr.iSpyTaskSize = 0;  //没有任务
    //任务指针数组全部赋NULL
    for(size_t j = 0U; j < SPY_VEHICLE_STATUS_MAX; ++j)
    {
        s_stSpyTaskArr.pstSpyTaskArr[j] = NULL;
    }
    //初始化锁
    (void)pthread_mutex_init(&s_stSpyTaskArr.stGetSpyTaskIdMutex, NULL);
}

/*!
 *******************************************************************************
 * dim模块 监控模块Deinit
 *
 * \note 
 * 
 *******************************************************************************
 */
void adm_dim_vdiSpyDeinit()
{
    for(size_t j = 0U; j < s_stSpyTaskArr.iSpyTaskSize; ++j)
    {
        if(NULL != s_stSpyTaskArr.pstSpyTaskArr[j])
        {
            adm_free(s_stSpyTaskArr.pstSpyTaskArr[j]);
            s_stSpyTaskArr.pstSpyTaskArr[j] = NULL;
        }
    }
    return;
}


/*!
 *******************************************************************************
 * dim模块 监控任务
 * 
 * \note
 *
 * \param[in]      pstSpyArr      车辆监控结构体
 * \param[in]      cbSpyFunc      注册监控回调函数
 * \param[out]     pu64TaskId      返回的监控任务id
 * \retval         D32S           D32S    0: 成功 non-0：失败
 *******************************************************************************
 */
D32S adm_dim_vdiSpyRegisterSpyTask(ADM_SYSI4VDI_ENGINE_SPY_ARRAY_T *pstSpyArr, ADM_DIM_SPY_VEHICLE_STATUS_CALLBACK_F cbSpyFunc, D64U* pu64TaskId)
{
    D32S iRet = ADM_ECODE_FAILED;
    D32S errorCode = ADM_ECODE_FAILED;

    //1.判断参数有效性
    if(NULL == pstSpyArr || NULL == cbSpyFunc || NULL == pu64TaskId)
    {
        return ADM_ECODE_INVALID_PARAM;
    }
    //2.获取任务id
    D64U itaskId = __adm_dim_vdiSpyGetSpyTaskId();

    //3.调用vdi接口，监控车辆上电事件
    iRet = adm_dim_vdi_setSpyVehicleStatus(adm_soa_getVdiInstanceId(), s_stSpyTaskArr.iTimeout, itaskId, pstSpyArr, &errorCode);
    if(ADM_ECODE_SUCCESSED != iRet)
    {
        log_e("adm_dim_vdi_setSpyVehicleStatus failed, iRet:%d", iRet);
        return iRet;
    } 
    else 
    {
        if(ADM_ECODE_SUCCESSED != errorCode)
        {
            log_e("adm_dim_vdi_setSpyVehicleStatus failed, errorCode:%d", errorCode);
            return errorCode;
        }
        log_i("adm_dim_vdi_setSpyVehicleStatus successed");   
    }

    //4.加入ADM_DIM_SPY_TASK_ARRAY_T结构体
    iRet = __adm_dim_vdiSpyTaskAdd(itaskId, cbSpyFunc);
    if(iRet != ADM_ECODE_SUCCESSED)
    {
        log_e("__adm_dim_vdiSpyTaskAdd failed, iRet = %d", iRet);
        return iRet;
    }

    //5.返回任务id，用于任务销毁
    *pu64TaskId = itaskId;
    return ADM_ECODE_SUCCESSED;
}

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
D32S adm_dim_vdiSpyCancelSpyTask(D64U u64TaskId)
{
    D32S iRet = ADM_ECODE_FAILED;
    D32S iErrorCode = ADM_ECODE_FAILED;
    if(0 == u64TaskId)
    {
        return ADM_ECODE_INVALID_PARAM;
    }

    iRet = adm_dim_vdi_clearSpyVehicleStatus(adm_soa_getVdiInstanceId(), s_stSpyTaskArr.iTimeout,
                                             u64TaskId, &iErrorCode);
    if(ADM_ECODE_SUCCESSED != iRet)
    {
        log_e("adm_dim_vdi_clearSpyVehicleStatus failed, iRet:%d", iRet);
        return iRet;
    } 
    else 
    {
        if(ADM_ECODE_SUCCESSED != iErrorCode)
        {
            log_e("adm_dim_vdi_clearSpyVehicleStatus failed, iErrorCode:%d", iErrorCode);
            return iErrorCode;
        }
        log_i("adm_dim_vdi_clearSpyVehicleStatus successed");   
    }
    iRet = __adm_dim_vdiSpyTaskDel(u64TaskId);
    if(iRet != ADM_ECODE_SUCCESSED)
    {
        log_e("__adm_dim_vdiSpyTaskAdd failed, iRet = %d", iRet);
        return iRet;
    }
    return ADM_ECODE_SUCCESSED;
}

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
D32S adm_dim_vdiSpyVehicleStatusCallback(D64U u64TaskId, void *priv)
{
    D32S iRet = ADM_ECODE_FAILED;

    log_i("adm_dim_vdiSpyVehicleStatusCallback enter!");   
    ADM_DIM_SPY_TASK_T * pstSpyTask = __adm_dim_vdiSpyFindTask(u64TaskId);
    if(NULL == pstSpyTask)
    {
       return iRet;
    }
    iRet = pstSpyTask->cbSpyFunc(u64TaskId, priv);
    log_i("adm_dim_vdiSpyVehicleStatusCallback exit!");   
    return iRet;
}
