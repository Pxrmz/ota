#include <unistd.h>
#include "adm_vsm_log.h"
#include "adm_vsm_impl.h"
#include "adm_vsm_tspImpl.h"
#include "adm_vsm_uim.h"
#include "adm_ipm_install.h"
#include "adm_com_errorCode.h"
#include "adm_memory.h"
#include "adm_cvm_uim.h"
#include "adm_cvm_checkVersion.h"
#include "adm_cvm_powerOncheck.h"
#include "adm_cvm_syncVehicleInfo.h"
#include "adm_dpm_uim.h"
#include "adm_dpm_downloadManagement.h"
#include "adm_com_kvstorage.h"
#include "adm_cvs_versionPolicy.h"
#include "adm_application_errorCode.h"
#include "adm_soa.h"
#include "adm_soa_cfg.h"
#include "adm_irm_report.h"
#include "adm_dpm_reportCloud.h"
#include "adm_vsm_signal_impl.h"
#include "adm_tpm_transferManagement.h"

D32S adm_vsm_waitCheckSubedEventSuccessed(D32S usServiceId, D16U usEventId)
{
    D32S iRet = ADM_ECODE_FAILED;
    D32S iErrCode = ADM_ECODE_FAILED;
    D32S i = 0;
    DCHAR *pSdkVersion = NULL;
    D32S iWaitCount = 0;

    for (i = 0; i < 60; i++)
    {
        log_d("wait hmi online");
        if (TRUE == adm_vsm_getHmiOnline())
        {
            log_d("hmi online");
            break;
        }

        sleep(3);
    }

    do
    {
        iRet = adm_dim_ucl_getSdkVersion(adm_soa_getHostInstanceId(), 5, &pSdkVersion, &iErrCode);
        if (ADM_ECODE_SUCCESSED == iRet && NULL != pSdkVersion)
        {
            log_d("get ucl sdk version[%s]", pSdkVersion);
            adm_free(pSdkVersion);
            break;
        }

        /** 1分钟仍获取版本失败，等待超时退出 */
        if (iWaitCount >= 20)
        {
            log_e("get ucl sdk version failed");
            break;
        }

        iWaitCount++;

        sleep(3);
    } while (TRUE);

    sleep(1);   ///< 避免函数注册，arpc方法未注册的情况

    return iRet;
}

DVOID adm_vsm_schduleTimeArrived(DVOID* priv)
{
    ADM_CVS_UPGRADE_MODE_E mode = ADM_CVS_UPGRADE_MODE_INVALID;
    ADM_UIM_SOA_ACTION_AUTH_TYPE_E eActionType = ADM_UIM_SOA_ACTION_AUTH_TYPE_INVALID;
    ADM_APPLICATION_COMMON_CMDID_TYPE_E eTriggerType = ADM_APPLICATION_COMMON_CMDID_TYPE_INVALID;
    ADM_APPLICATION_COMMON_CMDID_TYPE_E eCmdIdType = ADM_APPLICATION_COMMON_CMDID_TYPE_INVALID;
    DBOOL bCondSatisfy = DTRUE;
    DBOOL *pIsTaskStop = DFALSE;
    DBOOL bIsTaskStop = DFALSE;
    DCHAR *pcVerifyResult = NULL;
    D64S ulTaskId = 0;
    D32S iRet = ADM_ECODE_FAILED;

    log_i("[ABUP-Procedure] trigger schedule time arrived");

    if (NULL != priv)
    {
        log_i("Get priv param.");
        pIsTaskStop = (DBOOL *)priv;
        bIsTaskStop = *pIsTaskStop;
    }

    if (DFALSE == bIsTaskStop)
    {
        iRet = adm_dpm_taskVerify();
        if (ADM_ECODE_UPGRADE_TASK_STOPED == iRet)
        {
            log_i("Task is stoped.");
            bIsTaskStop = DTRUE;
        }
    }

    // if (DTRUE == bIsTaskStop)
    // {
    //     log_e("Task has been stopped.");
    //     adm_ipm_clearSchduleInfo(); ///< 任务失效就清除预约时间信息
    // }

    if (DFALSE == bIsTaskStop && DFALSE == adm_ipm_isScheduleInfoExist())
    {
        /** 恢复出厂设置会一同清理掉kv数据库 */
        log_e("Schedule info not exist");
        (DVOID)adm_vsm_stopWakeUp();
        return;
    }

    /** 预约时间到达就清除预约时间 */ 
    adm_ipm_removeSchduleInfo();

    D32S nResult = adm_cvs_getUpgradeMode((D32S *)&mode);
    if ((ADM_ECODE_SUCCESSED == nResult))
    {
        /** 静默/强制/救援升级直接触发安装,不通知车机 */
        switch (mode)
        {
        case ADM_CVS_UPGRADE_MODE_SLIENCE_INSTALL:
            eTriggerType = ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_SILENCE;
            bIsTaskStop = DFALSE;           /** 静默和强制走到安装流程,安装模块处理，本模块不处理  */
            bCondSatisfy = DTRUE;
            (void)adm_vsm_triggerInstall();
            break;
        case ADM_CVS_UPGRADE_MODE_FORCE:
            eTriggerType = ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_FORCE;
            bIsTaskStop = DFALSE;
            bCondSatisfy = DTRUE;
            (void)adm_vsm_triggerInstall();
            break;
        case ADM_CVS_UPGRADE_MODE_RESCUE:
            eTriggerType = ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_RESCUE;
            bIsTaskStop = DFALSE;
            bCondSatisfy = DTRUE;
            (void)adm_vsm_triggerInstall();
            break;
        default:
            if (adm_vsm_getHmiOnline())
            {
                log_i("Hmi is on line, notify Hmi schdule time arrived ");
                adm_vsm_waitCheckSubedEventSuccessed(ADM_UIM_INSTALL_SERVICE_ID, ADM_UIM_SCHDULE_TIME_ARRIVED_EVENT_ID);
                adm_ipm_getSchduleNotifyPreConditon(&bCondSatisfy, &pcVerifyResult);
                if (DFALSE == bCondSatisfy || DTRUE == bIsTaskStop)
                {
                    adm_cvs_getTaskId(&ulTaskId);
                    adm_vsm_reportState(ulTaskId,
                                        ADM_IRM_REPORT_SERVER_STATUS_Schduled_Not_executed,
                                        ADM_ECODE_SUCCESSED,
                                        pcVerifyResult);
                }

                adm_vsm_handleHmiStatSync(); ///< 等待HMI状态同步完成
                adm_uim_schduleTimeArrived(bCondSatisfy, bIsTaskStop);
            }
            else
            {
                if (DFALSE == bIsTaskStop)
                {
                    log_i("Hmi is not on line, trigger install ");
                    adm_vsm_setTriggerType(ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_SCHDULE);
                    (void)adm_vsm_triggerInstall();
                }
            }
            break;
        }
    }
    else
    {
        /** 找不到升级模式,尝试通知或升级 */
        if (adm_vsm_getHmiOnline())
        {
            adm_vsm_waitCheckSubedEventSuccessed(ADM_UIM_INSTALL_SERVICE_ID, ADM_UIM_SCHDULE_TIME_ARRIVED_EVENT_ID);
            log_i("Hmi is on line, notify Hmi schdule time arrived ");

            adm_ipm_getSchduleNotifyPreConditon(&bCondSatisfy, &pcVerifyResult);
            if (DFALSE == bCondSatisfy || DTRUE == bIsTaskStop)
            {
                adm_cvs_getTaskId(&ulTaskId);
                adm_vsm_reportState(ulTaskId,
                                    ADM_IRM_REPORT_SERVER_STATUS_Schduled_Not_executed,
                                    ADM_ECODE_SUCCESSED,
                                    pcVerifyResult);
            }

            adm_vsm_handleHmiStatSync(); ///< 等待HMI状态同步完成
            adm_uim_schduleTimeArrived(bCondSatisfy, bIsTaskStop);
        }
        else
        {
            if (DFALSE == bIsTaskStop)
            {
                log_i("Hmi is not on line, trigger install ");
                adm_vsm_setTriggerType(ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_SCHDULE);
                (void)adm_vsm_triggerInstall();
            }
        }
    }

    if (DTRUE == bIsTaskStop)
    {
        eCmdIdType = adm_vsm_getTspTriggerType();
        if (ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_TSP == eCmdIdType)
        {
            log_i("Tsp task. Exit tsp env.");
            adm_vsm_exitTspEnv();
        }

        adm_vsm_stateSet(ADM_VSM_STATE_IDLE);
    }

    if (DFALSE == bCondSatisfy || DTRUE == bIsTaskStop)
    {
        sleep(10);          ///< 等待hmi做相应处理然后再进入休眠
        (DVOID)adm_vsm_stopWakeUp();
    }

    return;
}

static DBOOL bIsNeedHandleTaskStop()
{
    DBOOL bIsNeedHandle = DFALSE;

    ADM_VSM_STATE_E eState = adm_vsm_stateGet();

    if (ADM_VSM_STATE_UPGRADE_READY == eState ||
        ADM_VSM_STATE_UPGRADE_PAUSE == eState)
    {
        log_i("Current state[%d] need verify task.", eState);
        bIsNeedHandle = DTRUE;
    }

    return bIsNeedHandle;
}

static DVOID* vsmNotifyUimTaskStop(DVOID* argv)
{
    static DBOOL bIsHandleTaskStop = DFALSE;
    ADM_APPLICATION_COMMON_CMDID_TYPE_E eCmdIdType = ADM_APPLICATION_COMMON_CMDID_TYPE_INVALID;
    D32S iRet = ADM_ECODE_SUCCESSED;
    D32U uiCount = 0;

    if (DTRUE == bIsHandleTaskStop)
    {
        return NULL;
    }

    bIsHandleTaskStop = DTRUE;

    while ((DTRUE == bIsNeedHandleTaskStop()) && uiCount < 20)
    {
        iRet = adm_dpm_downloadTaskStopNotify2Uim();
        if (ADM_IS_SUCCESS(iRet))
        {
            eCmdIdType = adm_vsm_getTspTriggerType();
            if (ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_TSP == eCmdIdType)
            {
                log_i("Tsp task. Exit tsp env.");
                adm_vsm_exitTspEnv();
            }

            adm_vsm_stateSet(ADM_VSM_STATE_IDLE);

            break;
        }
        uiCount++;
        sleep(5);
    }

    bIsHandleTaskStop = DFALSE;

    return NULL;
}
/**
 * @brief 下载过程中，任务失效处理
 *
 * @note 下载过程中或者下载完成，则等到下载完成并且重新上电(重启或者str模式退出都能触发)，判断任务是否仍然有效
 *       之所以没有等到安装前判断，是防止用户长久一直不点击安装，导致云端新任务无法下发的风险
 * @return DVOID
 */
static DVOID vsmHandleTaskStop()
{
    D32S iRet = ADM_ECODE_FAILED;
    D32S iErrorCode = ADM_ECODE_FAILED;
    D64S ulTaskId = 0;
    pthread_t uiId;
    ADM_APPLICATION_COMMON_CMDID_TYPE_E eCmdIdType = ADM_APPLICATION_COMMON_CMDID_TYPE_INVALID;

    if (DTRUE == bIsNeedHandleTaskStop())
    {
        iRet = adm_dpm_taskVerify();
        if (ADM_ECODE_UPGRADE_TASK_STOPED == iRet)
        {
            /** 防止校验任务有效性期间,进入安装中了 */
            if (DTRUE == bIsNeedHandleTaskStop())
            {
                log_i("Task is stop, clear upgrade info.");
                adm_vsm_forceResume(adm_vsm_stateGet(), DFALSE);

                if (DTRUE == adm_ipm_isScheduleInfoExist())
                {
                    /** 清除预约信息 */
                    adm_ipm_clearSchduleInfo();
                    adm_cvs_getTaskId(&ulTaskId);
                    adm_vsm_reportState(ulTaskId,
                                        ADM_IRM_REPORT_SERVER_STATUS_Schduled_Not_executed,
                                        ADM_ECODE_SUCCESSED,
                                        NULL);
                }

                /** 恢复下载的触发类型 */
                if (ADM_APPLICATION_COMMON_CMDID_TYPE_INVALID == adm_dpm_getDownInfo()->eTriggerType)
                {
                    adm_dpm_getDownInfo()->eTriggerType = adm_vsm_getTriggerType();
                }

                iRet = adm_dpm_downloadTaskStopNotify2Uim();
                if (ADM_IS_SUCCESS(iRet))
                {
                    eCmdIdType = adm_vsm_getTspTriggerType();
                    if (ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_TSP == eCmdIdType)
                    {
                        log_i("Tsp task. Exit tsp env.");
                        adm_vsm_exitTspEnv();
                    }

                    adm_vsm_stateSet(ADM_VSM_STATE_IDLE);
                }
                else 
                {
                    sleep(5);
                    if (0 == pthread_create(&uiId, NULL, vsmNotifyUimTaskStop, NULL))
                    {
                        (void)pthread_detach(uiId);
                    }
                    else
                    {
                        log_e("creat vsmNotifyUimTaskStop thread failed.");
                    }
                }
            }
        }
    }

    return;
}

DVOID adm_vsm_powerOnArrived()
{
    vsmHandleTaskStop();

    log_i("[ABUP-Procedure] trigger POWER_ON................");
    adm_cvm_PoweOnCheckVersion();

    return;
}

DVOID adm_vsm_gearPArrived()
{
    adm_tpm_handleGearSignal(GEAR_P);
    return;
}

DVOID adm_vsm_gearNonePArrived()
{
    adm_tpm_handleGearSignal(GEAR_NONE_P);
    return;
}