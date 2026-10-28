#include "adm_vsm_log.h"
#include "adm_vsm_tspImpl.h"
#include "adm_vsm_impl.h"
#include "adm_application_common.h"
#include "adm_application_errorCode.h"
#include "adm_irm_logFile.h"
#include "adm_soa.h"
#include "adm_memory.h"
#include "adm_string.h"
#include "adm_dim_vdi.h"
#include "adm_irm_report.h"
#include "adm_cvs_versionPolicy.h"
#include "adm_vrs_registerInfo.h"
#include <unistd.h>

static ADM_VSM_TSP_IMPL_T g_tspImplInfo = {0};

DVOID adm_vsm_handleHmiStatSync()
{
    ADM_COM_FUNC_ENTRY();
    /** 等待2分钟状态同步 */
    for (D32U i = 0; i < 40; i++)
    {
        if (DFALSE == adm_vsm_getHmiSyncStat())
        {
            sleep(3);
        }
        else 
        {
            log_i("Hmi stat has already sync.");
            break;
        }
    }

    ADM_COM_FUNC_EXIT();

    return;
}

/**
 * @brief 任务状态上报
 * @note 
 * @param in D64S ulTaskId 任务ID
 * @param in pcState 任务状态
 * @param in iStateCode 任务状态码
 * @param in pcExts 扩展信息
 * @retval D32S 0 成功，其他失败
 */
D32S adm_vsm_reportState(D64S ulTaskId, DCHAR *pcState, D32S iStateCode, DCHAR *pcExts)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    DCHAR szReason[ADM_COM_LEN_256] = {0};
    D64S ulCurTaskId = 0;

    ADM_COM_FUNC_ENTRY();

    /** 入参检查 */
    if (ADM_IS_NULL(pcState))
    {
        log_e("pcState is NULL");
        return ADM_ECODE_INVALID_PARAM;
    }

    /** 升级状态上报 */
    log_d("pcState[%s], state code %d", pcState, iStateCode);

    if (0 == adm_strcasecmp(pcState, ADM_IRM_REPORT_SERVER_STATUS_TSP_Task_presence))
    {
        adm_cvs_getTaskId(&ulCurTaskId);
        adm_snprintf(szReason, ADM_COM_LEN_256, "%lld", ulCurTaskId);
    }
    else
    {
        adm_snprintf(szReason, ADM_COM_LEN_256, "%d", iStateCode);
    }
    
    iRet = adm_irm_reportVehicleState(time(NULL), ADM_SDK_VERSION, NULL, ulTaskId, 0, pcState, szReason, NULL, NULL, pcExts);
    if (ADM_IS_ERR(iRet))
    {
        log_e("adm_irm_reportVehicleState call error.iRet[%d]", iRet);
    }

    ADM_COM_FUNC_EXIT();

    return iRet;
}
DBOOL adm_vsm_isCurrentPowerOn(void)
{
    DBOOL bIsPowerOn = DFALSE;
    ADM_DIM_SOA_MSGID_DATA_ARRAY_T stReqMsgData = {};
    ADM_DIM_SOA_MSGID_DATA_ARRAY_T* pstRespMsgData=NULL;
    D32S iPowerType = -1;
    D32S iErrCode = -1;
    D32S iRet = ADM_ECODE_SUCCESSED;

    stReqMsgData.msgIdArrayLength = 1;
    stReqMsgData.msgIdArray = (ADM_DIM_SOA_VDI_MSGID_T**)adm_malloc(sizeof(ADM_DIM_SOA_VDI_MSGID_T*) * (DSIZE)stReqMsgData.msgIdArrayLength);
    
    (void)adm_memset(stReqMsgData.msgIdArray, 0, sizeof(ADM_DIM_SOA_VDI_MSGID_T*) * stReqMsgData.msgIdArrayLength,  
            sizeof(ADM_DIM_SOA_VDI_MSGID_T*) * stReqMsgData.msgIdArrayLength);
    
    stReqMsgData.msgIdArray[0] = (ADM_DIM_SOA_VDI_MSGID_T*)adm_malloc((DSIZE)sizeof(ADM_DIM_SOA_VDI_MSGID_T));

    log_i("begin request readVehicleStatus.");
    stReqMsgData.msgIdArray[0]->msgId  = VEHICLE_STATUS_POWER_SIGNAL;
    stReqMsgData.msgIdArray[0]->msgIdData = NULL;

    /** 判断是否非下电状态； 否则，退出升级 */
    iRet = adm_dim_host_readVehicleStatus(adm_soa_getHostInstanceId(), 15, &stReqMsgData, &pstRespMsgData, &iErrCode);
    if (ADM_IS_ERR(iRet) || ADM_IS_ERR(iErrCode))
    {
        /** 由触发源控制是否重试 */
        log_e("adm_dim_host_readVehicleStatus call error.iRet[%d], errCode[%d]", iRet, iErrCode);
        adm_free(stReqMsgData.msgIdArray[0]);
        adm_free(stReqMsgData.msgIdArray);
        return bIsPowerOn;
    }

    if (NULL != pstRespMsgData && NULL != pstRespMsgData->msgIdArray && 
        VEHICLE_STATUS_POWER_SIGNAL == pstRespMsgData->msgIdArray[0]->msgId && 
        NULL != pstRespMsgData->msgIdArray[0]->msgIdData)
    {
        do
        {
            if (VAR_S32 != pstRespMsgData->msgIdArray[0]->msgIdData->dataType)
            {
                /** 给触发源成功,不要进行重试 */
                log_e("Value type invalid.type[%u]", pstRespMsgData->msgIdArray[0]->msgIdData->dataType);
                iRet = ADM_ECODE_SUCCESSED;
                break;
            }
            iPowerType = pstRespMsgData->msgIdArray[0]->msgIdData->v.valueS32;

            /** 当前如果是PowerOn 状态则不需要执行升级 */
            if (POWER_COMFORTABLE == iPowerType || OPWER_ON == iPowerType)
            {
                log_w("iPowerType = %d is PowerOn state, do not need to trigger install", iPowerType);
                bIsPowerOn = DTRUE;
                break;
            }
            else
            {
                log_i("iPowerType = %d is PowerOff state, do exec trigger install now", iPowerType);
            }
        }while (FALSE);

        for (D16U i = 0; i < pstRespMsgData->msgIdArrayLength; i++)
        {
            if (NULL != pstRespMsgData->msgIdArray[i]->msgIdData)
            {
                if (VAR_STRING == pstRespMsgData->msgIdArray[i]->msgIdData->dataType)
                {
                    adm_free(pstRespMsgData->msgIdArray[i]->msgIdData->v.valueS);
                }
            }
            adm_free(pstRespMsgData->msgIdArray[i]->msgIdData);
            adm_free(pstRespMsgData->msgIdArray[i]);
        }
        adm_free(pstRespMsgData->msgIdArray);
        adm_free(pstRespMsgData);
    }

    adm_free(stReqMsgData.msgIdArray[0]);
    adm_free(stReqMsgData.msgIdArray);


    return bIsPowerOn;
}


D32S adm_vsm_tspActionUpgrade(D64S ulTaskId, ADM_DIM_SOA_TSP_OPERATION_TYPE_E eOperationType)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    D32S iErrCode = -1;
    ADM_VSM_STATE_E eState = ADM_VSM_STATE_MAX;
    ADM_DIM_SOA_MSGID_DATA_ARRAY_T stReqMsgData = {};
    ADM_DIM_SOA_MSGID_DATA_ARRAY_T* pstRespMsgData=NULL;
    DBOOL bIsPowerOn = DFALSE;
    DCHAR szReason[ADM_COM_LEN_256] = {0};
    DCHAR *pcVerifyResult = NULL;
    D32S piPowerType = 0;

    ADM_COM_FUNC_ENTRY();
    bIsPowerOn = adm_vsm_isCurrentPowerOn();
    eState = adm_vsm_stateGet();
    if (ADM_VSM_STATE_IDLE == eState)
    {
        log_i("vsmState is [%d], do check", eState);
        if (DTRUE == bIsPowerOn)
        {
            log_i("iPowerType = is PowerOn state, do exec check now");
            adm_vsm_triggerCheck(ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_TSP);
        }
        else
        {
            //!< 无论设置是否成功，都需要调用TBOX进入休眠的接口
            iRet = adm_dim_host_execOtaTask(adm_soa_getHostTboxInstanceId(), 5, ADM_DIM_SOA_HOST_OTA_TASK_TYPE_STOP_WAKE_UP, 0, &iErrCode);
            iRet = (iRet == ADM_ECODE_SUCCESSED) ? iErrCode : iRet;
        }
    }
    //!< 当手机APP触发立即升级 且 OTA的状态机为待安装状态
    else if ((ADM_VSM_STATE_UPGRADE_READY == eState || ADM_VSM_STATE_UPGRADE_PAUSE == eState) && 
             ADM_DIM_SOA_TSP_OPERATION_INSTALL_NOW == eOperationType)
    {
        if (DFALSE == bIsPowerOn && DNULL != g_tspImplInfo.tspSchduleNotifyHmi)
        {
            log_i("iPowerType = is PowerOff state, do exec trigger install now");
            adm_vsm_handleHmiStatSync();         ///< 等待HMI状态同步完成
            g_tspImplInfo.tspSchduleNotifyHmi(ADM_DIM_SOA_TSP_OPERATION_INSTALL_NOW, 0, DNULL);
        }
        else
        {
            adm_vsm_reportState(ulTaskId, 
                                ADM_IRM_REPORT_SERVER_STATUS_Install_Not_executed, 
                                ADM_ECODE_SUCCESSED, 
                                pcVerifyResult);

            //!< 无论设置是否成功，都需要调用TBOX进入休眠的接口
            iRet = adm_dim_host_execOtaTask(adm_soa_getHostTboxInstanceId(), 5, ADM_DIM_SOA_HOST_OTA_TASK_TYPE_STOP_WAKE_UP, 0, &iErrCode);
            iRet = (iRet == ADM_ECODE_SUCCESSED) ? iErrCode : iRet;
        }
    }
    else
    {
        log_i("vsmState is [%d], do resume", eState);
        adm_vsm_reportState(ulTaskId, ADM_IRM_REPORT_SERVER_STATUS_TSP_Task_presence, ADM_ECODE_SUCCESSED, NULL);

        // adm_vsm_resume();
    }

    ADM_COM_FUNC_EXIT();

    return iRet;
}

D32S adm_vsm_tspSlientDownload(ADM_DIM_SOA_TSP_OPERATION_TYPE_E eOperationType)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    D32S errorCode = ADM_ECODE_FAILED;
    ADM_VSM_STATE_E eState = ADM_VSM_STATE_MAX;

    log_i("adm_vsm_tspSlientDownload begin");

    if (DFALSE == adm_vsm_isCurrentPowerOn())
    {
        //!< 无论设置是否成功，都需要调用TBOX进入休眠的接口
        iRet = adm_dim_host_execOtaTask(adm_soa_getHostTboxInstanceId(), 5, ADM_DIM_SOA_HOST_OTA_TASK_TYPE_STOP_WAKE_UP, 0, &errorCode);
        iRet = (iRet == ADM_ECODE_SUCCESSED) ? errorCode : iRet;

        return iRet;
    }

    eState = adm_vsm_stateGet();
    if (ADM_VSM_STATE_IDLE == eState)
    {
        log_i("vsmState is [%d], do check", eState);
        adm_vsm_triggerCheck(ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_TSP);
    }
    //!< 当手机APP触发立即升级 且 OTA的状态机为待安装状态
    else if (ADM_VSM_STATE_UPGRADE_READY == eState && ADM_DIM_SOA_TSP_OPERATION_INSTALL_NOW == eOperationType)
    {
        adm_vsm_triggerInstall();
    }
    else
    {
        // log_i("vsmState is [%d], do resume", eState);
        log_i("vsmState is [%d], do not handle", eState);
        // adm_vsm_resume();
    }

    log_i("adm_vsm_tspSlientDownload end");

    return iRet;
}

D32S adm_vsm_tspSlientUpgrade(D64S ulTaskId)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    ADM_VSM_STATE_E eState = ADM_VSM_STATE_MAX;

    log_i("adm_vsm_tspSlientUpgrade begin");

    eState = adm_vsm_stateGet();
    if (ADM_VSM_STATE_IDLE == eState)
    {
        log_i("vsmState is [%d], do check", eState);
        adm_vsm_triggerCheck(ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_TSP);
    }
    else
    {
        log_i("vsmState is [%d], do resume", eState);
        adm_vsm_reportState(ulTaskId, ADM_IRM_REPORT_SERVER_STATUS_TSP_Task_presence, ADM_ECODE_SUCCESSED, NULL);
        // adm_vsm_resume();
    }

    log_i("adm_vsm_tspSlientUpgrade end");

    return iRet;
}

D32S adm_vsm_tspAppointUpgrade(ADM_DIM_SOA_TSP_OPERATION_TYPE_E eOperationType, D64S appointmentTimestamp)
{
    D32S errorCode = ADM_ECODE_FAILED;
    ADM_VSM_STATE_E eState = ADM_VSM_STATE_MAX;
    D32S ret = ADM_ECODE_FAILED;
    ADM_UIM_SOA_ACTION_AUTH_TYPE_E eAuthType = ADM_UIM_SOA_ACTION_AUTH_TYPE_ALLOW; 

    log_i("adm_vsm_tspAppointUpgrade begin. operationType[%d] time[%lld]", eOperationType, appointmentTimestamp);

    eState = adm_vsm_stateGet();
    if ((NULL != g_tspImplInfo.schduleInstallCallback) && 
        (eState == ADM_VSM_STATE_UPGRADE_READY) || (eState == ADM_VSM_STATE_UPGRADE_PAUSE))
    {
        adm_vsm_handleHmiStatSync(); ///< 等待HMI状态同步完成

        //!< tsp 推送设置预约时间 和 更新预约
        if (ADM_DIM_SOA_TSP_OPERATION_INSTALL_SCHDULE == eOperationType || ADM_DIM_SOA_TSP_OPERATION_SCHDULE_UPDATE == eOperationType)
        {
            D32S nRet = g_tspImplInfo.schduleInstallCallback(ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_TSP, ADM_APPLICATION_SCHEDULE_INSTALL_APP, ADM_UIM_SOA_INSTALL_SCHDULE_TYPE_SCHDULED, appointmentTimestamp, &eAuthType, DNULL);
            //!< 告知HMI
            if (DNULL != g_tspImplInfo.tspSchduleNotifyHmi && nRet == ADM_ECODE_SUCCESSED)
            {
                log_i("g_tspImplInfo.tspSchduleNotifyHmi appointmentTimestamp:%lld", appointmentTimestamp);
                ret = g_tspImplInfo.tspSchduleNotifyHmi(eOperationType, appointmentTimestamp, DNULL);
            }
        }
        //!< tsp 推送取消预约
        else if (ADM_DIM_SOA_TSP_OPERATION_SCHDULE_CANCEL == eOperationType)
        {
            D32S nRet = g_tspImplInfo.schduleInstallCallback(ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_TSP, 
            ADM_APPLICATION_SCHEDULE_INSTALL_APP, 
            ADM_UIM_SOA_INSTALL_SCHDULE_TYPE_CANCELED, appointmentTimestamp, &eAuthType, DNULL);
            //!< 告知HMI
            if (DNULL != g_tspImplInfo.tspSchduleNotifyHmi && nRet == ADM_ECODE_SUCCESSED)
            {
                log_i("g_tspImplInfo.tspSchduleNotifyHmi appointmentTimestamp:%lld", appointmentTimestamp);
                ret = g_tspImplInfo.tspSchduleNotifyHmi(eOperationType, appointmentTimestamp, DNULL);
            }
        }
        ret = ADM_ECODE_SUCCESSED;
        log_d("schduleTime:%lld, errorCode:%d, ret:%d", appointmentTimestamp, errorCode, ret);
        ret = (ret == ADM_ECODE_SUCCESSED) ? errorCode : ret;
    }
    else
    {
        log_w("vsmState = [%d], do not need to set appoint time", eState);
    }

    //!< 无论设置是否成功，都需要调用TBOX进入休眠的接口
    ret = adm_dim_host_execOtaTask(adm_soa_getHostTboxInstanceId(), 5, ADM_DIM_SOA_HOST_OTA_TASK_TYPE_STOP_WAKE_UP, 0, &errorCode);
    ret = (ret == ADM_ECODE_SUCCESSED) ? errorCode : ret;

    log_i("adm_vsm_tspAppointUpgrade end");

    return ret;
}

D32S adm_vsm_tspUsbUpgrade()
{
    D32S iRet = ADM_ECODE_FAILED;
    D32S errorCode = ADM_ECODE_FAILED;

    log_i("adm_vsm_tspUsbUpgrade do nothing begin");
    iRet = ADM_ECODE_SUCCESSED;
    //!< 无论设置是否成功，都需要调用TBOX进入休眠的接口
    iRet = adm_dim_host_execOtaTask(adm_soa_getHostTboxInstanceId(), 5, ADM_DIM_SOA_HOST_OTA_TASK_TYPE_STOP_WAKE_UP, 0, &errorCode);
    iRet = (iRet == ADM_ECODE_SUCCESSED) ? errorCode : iRet;

    log_i("adm_vsm_tspSlientUpgrade do nothing end");

    return iRet;
}

D32S adm_vsm_tspRescueUpgrade(D64S ulTaskId)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    D32S iErrCode = -1;
    D32S iPowerType = -1;
    ADM_VSM_STATE_E eState = ADM_VSM_STATE_MAX;
    ADM_DIM_SOA_MSGID_DATA_ARRAY_T stReqMsgData = {};
    ADM_DIM_SOA_MSGID_DATA_ARRAY_T* pstRespMsgData=NULL;

    log_i("adm_vsm_tspRescueUpgrade begin");

    stReqMsgData.msgIdArrayLength = 1;
    stReqMsgData.msgIdArray = (ADM_DIM_SOA_VDI_MSGID_T**)adm_malloc(sizeof(ADM_DIM_SOA_VDI_MSGID_T*) * (DSIZE)stReqMsgData.msgIdArrayLength);
    
    (void)adm_memset(stReqMsgData.msgIdArray, 0, sizeof(ADM_DIM_SOA_VDI_MSGID_T*) * stReqMsgData.msgIdArrayLength,  
               sizeof(ADM_DIM_SOA_VDI_MSGID_T*) * stReqMsgData.msgIdArrayLength);
    
    stReqMsgData.msgIdArray[0] = (ADM_DIM_SOA_VDI_MSGID_T*)adm_malloc((DSIZE)sizeof(ADM_DIM_SOA_VDI_MSGID_T));

    log_i("begin request readVehicleStatus.");
    stReqMsgData.msgIdArray[0]->msgId  = VEHICLE_STATUS_POWER_SIGNAL;
    stReqMsgData.msgIdArray[0]->msgIdData = NULL;

    /** 判断是否非下电状态； 否则，退出升级 */
    iRet = adm_dim_host_readVehicleStatus(adm_soa_getHostInstanceId(), 15, &stReqMsgData, &pstRespMsgData, &iErrCode);
    if (ADM_IS_ERR(iRet) || ADM_IS_ERR(iErrCode))
    {
        /** 由触发源控制是否重试 */
        log_e("adm_dim_host_readVehicleStatus call error.iRet[%d], errCode[%d]", iRet, iErrCode);
        adm_free(stReqMsgData.msgIdArray[0]);
        adm_free(stReqMsgData.msgIdArray);
        iRet = ADM_ECODE_FAILED;
        return iRet;
    }

    if (NULL != pstRespMsgData && NULL != pstRespMsgData->msgIdArray && 
        VEHICLE_STATUS_POWER_SIGNAL == pstRespMsgData->msgIdArray[0]->msgId && 
        NULL != pstRespMsgData->msgIdArray[0]->msgIdData)
    {
        do
        {
            if (VAR_S32 != pstRespMsgData->msgIdArray[0]->msgIdData->dataType)
            {
                /** 给触发源成功,不要进行重试 */
                log_e("Value type invalid.type[%u]", pstRespMsgData->msgIdArray[0]->msgIdData->dataType);
                iRet = ADM_ECODE_SUCCESSED;
                break;
            }
            iPowerType = pstRespMsgData->msgIdArray[0]->msgIdData->v.valueS32;

            /** 非下电和异常状态执行救援任务 */
            if (POWER_COMFORTABLE == iPowerType || OPWER_ON == iPowerType)
            {
                eState = adm_vsm_stateGet();
                if (ADM_VSM_STATE_IDLE == eState)
                {
                    log_i("vsmState is [%d], do check", eState);
                    adm_vsm_triggerCheck(ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_TSP);
                }
                else
                {
                    log_i("vsmState is [%d], do resume", eState);
                    adm_vsm_reportState(ulTaskId, ADM_IRM_REPORT_SERVER_STATUS_TSP_Task_presence, ADM_ECODE_SUCCESSED, NULL);
                    // adm_vsm_resume();
                }
            }
            else 
            {
                adm_vsm_reportState(ulTaskId, ADM_IRM_REPORT_SERVER_STATUS_TSP_Conditions_Not_Met, ADM_ECODE_SUCCESSED, NULL);

                /** 给触发源成功,不要进行重试,流程结束 */
                log_e("Power is not on. powerType[%d]", iPowerType);
                //!< 无论设置是否成功，都需要调用TBOX进入休眠的接口
                iRet = adm_dim_host_execOtaTask(adm_soa_getHostTboxInstanceId(), 5, ADM_DIM_SOA_HOST_OTA_TASK_TYPE_STOP_WAKE_UP, 0, &iErrCode);
                iRet = (iRet == ADM_ECODE_SUCCESSED) ? iErrCode : iRet;
                break;
            }
        } while (FALSE);

        for (D16U i = 0; i < pstRespMsgData->msgIdArrayLength; i++)
        {
            if (NULL != pstRespMsgData->msgIdArray[i]->msgIdData)
            {
                if (VAR_STRING == pstRespMsgData->msgIdArray[i]->msgIdData->dataType)
                {
                    adm_free(pstRespMsgData->msgIdArray[i]->msgIdData->v.valueS);
                }
            }
            adm_free(pstRespMsgData->msgIdArray[i]->msgIdData);
            adm_free(pstRespMsgData->msgIdArray[i]);
        }
        adm_free(pstRespMsgData->msgIdArray);
        adm_free(pstRespMsgData);
    }

    adm_free(stReqMsgData.msgIdArray[0]);
    adm_free(stReqMsgData.msgIdArray);

    log_i("adm_vsm_tspRescueUpgrade end");

    return iRet;
}

D32S adm_vsm_tspProductLineUpgrade(D64S ulTaskId)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    ADM_VSM_STATE_E eState = ADM_VSM_STATE_MAX;

    log_i("adm_vsm_tspProductLineUpgrade begin");

    eState = adm_vsm_stateGet();
    if (ADM_VSM_STATE_IDLE == eState)
    {
        log_i("vsmState is [%d], do check", eState);
        adm_vsm_triggerCheck(ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_TSP);
    }
    else
    {
        log_i("vsmState is [%d], do resume", eState);
        adm_vsm_reportState(ulTaskId, ADM_IRM_REPORT_SERVER_STATUS_TSP_Task_presence, ADM_ECODE_SUCCESSED, NULL);
        // adm_vsm_resume();
    }

    log_i("adm_vsm_tspProductLineUpgrade end");

    return iRet;
}

D32S adm_vsm_tspApkSelfUpgrade(D64S ulTaskId)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    ADM_VSM_STATE_E eState = ADM_VSM_STATE_MAX;

    log_i("adm_vsm_tspProductLineUpgrade begin");

    eState = adm_vsm_stateGet();
    if (ADM_VSM_STATE_IDLE == eState)
    {
        log_i("vsmState is [%d], do check", eState);
        adm_vsm_triggerCheck(ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_TSP);
    }
    else
    {
        log_i("vsmState is [%d], do resume", eState);
        adm_vsm_reportState(ulTaskId, ADM_IRM_REPORT_SERVER_STATUS_TSP_Task_presence, ADM_ECODE_SUCCESSED, NULL);
        // adm_vsm_resume();
    }

    log_i("adm_vsm_tspProductLineUpgrade end");

    return iRet;
}

D32S adm_vsm_tspForceUpgrade(D64S ulTaskId)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    ADM_VSM_STATE_E eState = ADM_VSM_STATE_MAX;

    log_i("adm_vsm_tspForceUpgrade begin");

    eState = adm_vsm_stateGet();
    if (ADM_VSM_STATE_IDLE == eState)
    {
        log_i("vsmState is [%d], do check", eState);
        adm_vsm_triggerCheck(ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_TSP);
    }
    else
    {
        log_i("vsmState is [%d], do resume", eState);
        adm_vsm_reportState(ulTaskId, ADM_IRM_REPORT_SERVER_STATUS_TSP_Task_presence, ADM_ECODE_SUCCESSED, NULL);
        // adm_vsm_resume();
    }

    log_i("adm_vsm_tspForceUpgrade end");

    return iRet;
}


static DBOOL vsmIsCurrentSpeedZero()
{   
    DBOOL bIsSpeedZero = DTRUE;
    ADM_DIM_SOA_MSGID_DATA_ARRAY_T stReqMsgData = {};
    ADM_DIM_SOA_MSGID_DATA_ARRAY_T* pstRespMsgData=NULL;
    D32S iErrCode = -1;
    D32S iSpeed = -1;
    D32S iRet = ADM_ECODE_SUCCESSED;

    stReqMsgData.msgIdArrayLength = 1;
    stReqMsgData.msgIdArray = (ADM_DIM_SOA_VDI_MSGID_T**)adm_malloc(sizeof(ADM_DIM_SOA_VDI_MSGID_T*) * (DSIZE)stReqMsgData.msgIdArrayLength);
    
    (void)adm_memset(stReqMsgData.msgIdArray, 0, sizeof(ADM_DIM_SOA_VDI_MSGID_T*) * stReqMsgData.msgIdArrayLength,  
            sizeof(ADM_DIM_SOA_VDI_MSGID_T*) * stReqMsgData.msgIdArrayLength);
    
    stReqMsgData.msgIdArray[0] = (ADM_DIM_SOA_VDI_MSGID_T*)adm_malloc((DSIZE)sizeof(ADM_DIM_SOA_VDI_MSGID_T));

    log_i("begin request readVehicleStatus.");
    stReqMsgData.msgIdArray[0]->msgId  = VEHICLE_STATUS_VSPD;
    stReqMsgData.msgIdArray[0]->msgIdData = NULL;

    /** 判断是否非下电状态； 否则，退出升级 */
    iRet = adm_dim_host_readVehicleStatus(adm_soa_getHostInstanceId(), 15, &stReqMsgData, &pstRespMsgData, &iErrCode);
    if (ADM_IS_ERR(iRet) || ADM_IS_ERR(iErrCode))
    {
        /** 由触发源控制是否重试 */
        log_e("adm_dim_host_readVehicleStatus call error.iRet[%d], errCode[%d]", iRet, iErrCode);
        adm_free(stReqMsgData.msgIdArray[0]);
        adm_free(stReqMsgData.msgIdArray);
        return bIsSpeedZero;
    }

    if (NULL != pstRespMsgData && NULL != pstRespMsgData->msgIdArray && 
        VEHICLE_STATUS_VSPD == pstRespMsgData->msgIdArray[0]->msgId && 
        NULL != pstRespMsgData->msgIdArray[0]->msgIdData)
    {
        do
        {
            if (VAR_S32 != pstRespMsgData->msgIdArray[0]->msgIdData->dataType)
            {
                /** 给触发源成功,不要进行重试 */
                log_e("Value type invalid.type[%u]", pstRespMsgData->msgIdArray[0]->msgIdData->dataType);
                iRet = ADM_ECODE_SUCCESSED;
                break;
            }
            iSpeed = pstRespMsgData->msgIdArray[0]->msgIdData->v.valueS32;
            log_i("iSpeed = %d", iSpeed);
            if (0 < iSpeed)
            {
                bIsSpeedZero = DFALSE;
            }
        }while (FALSE);

        for (D16U i = 0; i < pstRespMsgData->msgIdArrayLength; i++)
        {
            if (NULL != pstRespMsgData->msgIdArray[i]->msgIdData)
            {
                if (VAR_STRING == pstRespMsgData->msgIdArray[i]->msgIdData->dataType)
                {
                    adm_free(pstRespMsgData->msgIdArray[i]->msgIdData->v.valueS);
                }
            }
            adm_free(pstRespMsgData->msgIdArray[i]->msgIdData);
            adm_free(pstRespMsgData->msgIdArray[i]);
        }
        adm_free(pstRespMsgData->msgIdArray);
        adm_free(pstRespMsgData);
    }

    adm_free(stReqMsgData.msgIdArray[0]);
    adm_free(stReqMsgData.msgIdArray);


    return bIsSpeedZero;
}

void adm_vsm_tspOtaOneClickRescue()
{
    D32S iRet = ADM_ECODE_FAILED;
    D32S errorCode = ADM_ECODE_FAILED;
    DBOOL bIsFuel = DFALSE;  
    ADM_VRS_VEHICLE_POWER_TYPE_E ePowerType = VEHICLE_INVALID;
    log_i("adm_vsm_tspOtaOneClickRescue begin");

    if (DFALSE == vsmIsCurrentSpeedZero())
    {
        log_w("Current speed is not zero, do not reset now");
    }
    else
    {
        log_i("Current speed is zero, do reset now!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
        iRet = adm_vsm_getIsFuel(&bIsFuel);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Get vehicle power type failed, iRet:%d", iRet);
            //return;  // 获取失败直接不返回，仍执行reset
        }

        if (bIsFuel != DTRUE)
        {
            iRet = adm_dim_host_setVehicleStatus(adm_soa_getHostInstanceId(),30, VEHICLE_CTRL_EXIT_HV_MODE, &errorCode);
            log_i("Switch to %d. instanceId[%u], iRet[%d], errcode[%d]", VEHICLE_CTRL_EXIT_HV_MODE, adm_soa_getHostInstanceId(), iRet, errorCode);
            iRet = adm_dim_host_setVehicleStatus(adm_soa_getHostInstanceId(), 30, (D32U)VEHICLE_CTRL_NO_ACTIVE_HV_MODE, &errorCode);
            log_i("Not active. instanceId[%u], iRet[%d], errcode[%d]", adm_soa_getHostInstanceId(), iRet, errorCode);
        }

        iRet = adm_dim_host_setVehicleStatus(adm_soa_getHostInstanceId(), 30, VEHICLE_CTRL_EXIT_NORMAL_OTA_MODE ,&errorCode);
        log_d("instanceId:%u, eVechielStatusType:%u, ret:%d, errorCode:%d", adm_soa_getHostInstanceId(), VEHICLE_CTRL_EXIT_NORMAL_OTA_MODE, iRet, errorCode);
        iRet = (iRet == ADM_ECODE_SUCCESSED) ? errorCode : iRet;

#ifdef SUPPORT_REMOTE_AUTH
        iRet = adm_dim_host_setPower(adm_soa_getHostInstanceId(), ADM_POWER_CONTROL_TIMEOUT, (D32U)ADM_DIM_SOA_HOST_OTA_TASK_TYPE_REMOTE_POWER_OFF, &errorCode);
        log_i("power off ret[%d], errCode[%d]", iRet, errorCode);
        adm_ipm_remoutePowerOffEventReport(iRet);
#endif
        if (DNULL != g_tspImplInfo.tspOneClickRescueCb)
        {
            g_tspImplInfo.tspOneClickRescueCb();
        }

        /** tsp推送唤醒，此时退出唤醒 */
        iRet = adm_dim_host_execOtaTask(adm_soa_getHostTboxInstanceId(), 30, ADM_DIM_SOA_HOST_OTA_TASK_TYPE_STOP_WAKE_UP, 0, &errorCode);
        log_i("out of wake up ret[%d], errCode[%d]", iRet, errorCode);
    }

    log_i("adm_vsm_tspOtaOneClickRescue end");

    return;
}

D32S adm_vsm_tspOtaLogReport()
{
    log_i("adm_vsm_tspOtaLogReport begin");
    ADM_IRM_LOG_FILE_TG_REPORT_INFO_T streportInfo = {0};

    streportInfo.cmdId = ADM_OCS_CMDID_TSP;
    streportInfo.extsReqJsonStr = NULL;
    streportInfo.pushId = NULL;
    streportInfo.iKeyLogFlag = 0;

    adm_irm_triggerLogFileReport(&streportInfo);

    log_i("adm_vsm_tspOtaLogReport end");

    return ADM_ECODE_SUCCESSED;
}

D32S adm_vsm_execPowerOn()
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    D32S iErrorCode = ADM_ECODE_SUCCESSED;
    D32U uiRetryCnt = 0;

#ifdef SUPPORT_REMOTE_AUTH
    do
    {
        iRet = adm_dim_host_setPower(adm_soa_getHostInstanceId(), ADM_POWER_CONTROL_TIMEOUT, ADM_DIM_SOA_HOST_OTA_TASK_TYPE_REMOTE_POWER_ON, &iErrorCode);

        iRet = ADM_IS_ERR(iRet) ? iRet : iErrorCode;
        if (ADM_IS_SUCCESS(iRet))
        {
            log_i("Exec power on succeed. current try count[%u].", uiRetryCnt);
            break;
        }
        else 
        {
            log_e("Exec power on failed.ret[%d] errorCode[%d].current try count[%u].", iRet, iErrorCode, uiRetryCnt);
            uiRetryCnt++;
        }
    } while (uiRetryCnt < 4);

    adm_ipm_remoutePowerOnEventReport(iRet); ///< 上报上电事件
#endif

    return iRet;
}

D32S adm_vsm_execPowerOff()
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    D32S iErrorCode = ADM_ECODE_SUCCESSED;
    D32U uiRetryCnt = 0;

#ifdef SUPPORT_REMOTE_AUTH
    do
    {
        iRet = adm_dim_host_setPower(adm_soa_getHostInstanceId(), ADM_POWER_CONTROL_TIMEOUT, (D32U)ADM_DIM_SOA_HOST_OTA_TASK_TYPE_REMOTE_POWER_OFF, &iErrorCode);

        iRet = ADM_IS_ERR(iRet) ? iRet : iErrorCode;
        if (ADM_IS_SUCCESS(iRet))
        {
            log_i("Exec power off succeed. current try count[%u].", uiRetryCnt);
            break;
        }
        else 
        {
            log_e("Exec power off failed.ret[%d] errorCode[%d].current try count[%u].", iRet, iErrorCode, uiRetryCnt);
            uiRetryCnt++;
        }
    } while (uiRetryCnt < 4);

    if (ADM_IS_ERR(iRet))
    {
        /** 如果重试3次下电都失败,则调用紧急下电接口 */
        (DVOID)adm_dim_host_setIcuPowerOff(adm_soa_getHostInstanceId(), ADM_POWER_CONTROL_TIMEOUT, &iErrorCode);
    }

    adm_ipm_remoutePowerOffEventReport(iRet);
#endif

    return iRet;
}

D32S adm_vsm_entryHighHvMode()
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    D32S iErrorCode = ADM_ECODE_SUCCESSED;
    D32U uiRetryCnt = 0;

    DBOOL bIsFuel = DFALSE;  

    do
    {

        iRet = adm_vsm_getIsFuel(&bIsFuel);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Get vehicle power type failed, iRet:%d", iRet);
            return iRet;  // 直接返回失败
        }

        if (DTRUE == bIsFuel)
        {
            log_i("ICE Vechile.");
            break;
        }

        iRet = adm_dim_host_setVehicleStatus(adm_soa_getHostInstanceId(), ADM_HV_MODE_CONTROL_TIMEOUT, VEHICLE_CTRL_ENTER_HV_MODE, &iErrorCode);

        iRet = ADM_IS_ERR(iRet) ? iRet : iErrorCode;
        if (ADM_IS_SUCCESS(iRet))
        {
            log_i("Entry high hv mode succeed. current try count[%u].", uiRetryCnt);
            break;
        }
        else 
        {
            log_e("Entry high hv mode failed.ret[%d] errorCode[%d].current try count[%u].", iRet, iErrorCode, uiRetryCnt);
            uiRetryCnt++;
        }
    } while (uiRetryCnt < 4);

    return iRet;
}

D32S adm_vsm_exitHighHvMode()
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    D32S iErrorCode = ADM_ECODE_SUCCESSED;
    D32U uiRetryCnt = 0;
    DBOOL bIsFuel = DFALSE;

    do
    {
        
        iRet = adm_vsm_getIsFuel(&bIsFuel);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Get vehicle power type failed, iRet:%d", iRet);
            return iRet;  // 直接返回失败
        }

        if (DTRUE == bIsFuel)
        {
            log_i("ICE Vechile.");
            break;
        }

        iRet = adm_dim_host_setVehicleStatus(adm_soa_getHostInstanceId(), ADM_HV_MODE_CONTROL_TIMEOUT, VEHICLE_CTRL_EXIT_HV_MODE, &iErrorCode);

        iRet = ADM_IS_ERR(iRet) ? iRet : iErrorCode;
        if (ADM_IS_SUCCESS(iRet))
        {
            log_i("Exit high hv mode succeed. current try count[%u]", uiRetryCnt);
            break;
        }
        else 
        {
            log_e("Exit high hv mode failed.ret[%d] errorCode[%d].current try count[%u].", iRet, iErrorCode, uiRetryCnt);
            uiRetryCnt++;
        }
    } while (uiRetryCnt < 4);

    uiRetryCnt = 0;
    do
    {
        
        iRet = adm_vsm_getIsFuel(&bIsFuel);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Get vehicle power type failed, iRet:%d", iRet);
            return iRet;  // 直接返回失败
        }

        if (DTRUE == bIsFuel)
        {
            log_i("ICE Vechile.");
            break;
        }

        iRet = adm_dim_host_setVehicleStatus(adm_soa_getHostInstanceId(), ADM_HV_MODE_CONTROL_TIMEOUT, VEHICLE_CTRL_NO_ACTIVE_HV_MODE, &iErrorCode);
        iRet = ADM_IS_ERR(iRet) ? iRet : iErrorCode;

        if (ADM_IS_SUCCESS(iRet))
        {
            log_i("Exec not active succeed. current try count[%u]", uiRetryCnt);
            break;
        }
        else
        {
            log_e("Exec not active failed.ret[%d] errorCode[%d].current try count[%u].", iRet, iErrorCode, uiRetryCnt);
            uiRetryCnt++;
        }
    } while (uiRetryCnt < 4);

    return iRet;
}

D32S adm_vsm_entryOtaMode()
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    D32S iErrorCode = ADM_ECODE_SUCCESSED;
    D32U uiRetryCnt = 0;

    do
    {
        iRet = adm_dim_host_setVehicleStatus(adm_soa_getHostInstanceId(), ADM_OTA_MODE_CONTROL_TIMEOUT, VEHICLE_CTRL_ENTER_NORMAL_OTA_MODE, &iErrorCode);

        iRet = ADM_IS_ERR(iRet) ? iRet : iErrorCode;
        if (ADM_IS_SUCCESS(iRet))
        {
            log_i("Entry ota mode succeed. current try count[%u].", uiRetryCnt);
            break;
        }
        else 
        {
            log_e("Entry ota mode failed.ret[%d] errorCode[%d].current try count[%u].", iRet, iErrorCode, uiRetryCnt);
            uiRetryCnt++;
        }
    } while (uiRetryCnt < 4);

    return iRet;
}

D32S adm_vsm_exitOtaMode()
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    D32S iErrorCode = ADM_ECODE_SUCCESSED;
    D32U uiRetryCnt = 0;

    do
    {
        iRet = adm_dim_host_setVehicleStatus(adm_soa_getHostInstanceId(), ADM_OTA_MODE_CONTROL_TIMEOUT, VEHICLE_CTRL_EXIT_NORMAL_OTA_MODE, &iErrorCode);

        iRet = ADM_IS_ERR(iRet) ? iRet : iErrorCode;
        if (ADM_IS_SUCCESS(iRet))
        {
            log_i("Exit ota mode succeed. current try count[%u].", uiRetryCnt);
            break;
        }
        else 
        {
            log_e("Exit ota mode failed.ret[%d] errorCode[%d] current try count[%u].", iRet, iErrorCode, uiRetryCnt);
            uiRetryCnt++;
        }
    } while (uiRetryCnt < 4);

    return iRet;
}

D32S adm_vsm_stopWakeUp()
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    D32S iErrorCode = ADM_ECODE_SUCCESSED;

    iRet = adm_dim_host_execOtaTask(adm_soa_getHostTboxInstanceId(), ADM_WAKE_UP_CONTROL_TIMEOUT, ADM_DIM_SOA_HOST_OTA_TASK_TYPE_STOP_WAKE_UP, 0, &iErrorCode);

    log_i("Stop wake up ret[%d], errCode[%d]", iRet, iErrorCode);

    iRet = ADM_IS_ERR(iRet) ? iRet : iErrorCode;

    return iRet;
}

D32S adm_vsm_entryTspEnv()
{
    D32S iRet = ADM_ECODE_FAILED;
    D32S iErrorCode = ADM_ECODE_FAILED;

    do
    {
        log_i("Entry tsp env.");

        iRet = adm_vsm_entryOtaMode();
        if (ADM_IS_ERR(iRet))
        {
            (DVOID)adm_vsm_stopWakeUp();
            break;
        }

        iRet = adm_vsm_execPowerOn();    
        if (ADM_IS_ERR(iRet))
        {
            log_e("Power on failed.Start exec: 1.power off 2.exit ota mode.");

            (DVOID)adm_vsm_execPowerOff();
            (DVOID)adm_vsm_exitOtaMode();
            (DVOID)adm_vsm_stopWakeUp();
            break;
        }

        iRet = adm_vsm_entryHighHvMode();
        if (ADM_IS_ERR(iRet))
        {
            log_e("Entry high hv mode failed., Start exec: 1.exit high hv mode 2.power off 3.exit ota mode.");

            (DVOID)adm_vsm_exitHighHvMode();
            (DVOID)adm_vsm_execPowerOff();
            (DVOID)adm_vsm_exitOtaMode();
            (DVOID)adm_vsm_stopWakeUp();
            break;
        }
    } while (DFALSE);

    return iRet;
}

DVOID adm_vsm_exitTspEnv()
{
    (DVOID)adm_vsm_exitHighHvMode();
    (DVOID)adm_vsm_execPowerOff();
    (DVOID)adm_vsm_exitOtaMode();
    // (DVOID)adm_vsm_stopWakeUp();

    return;
}

D32S adm_vsm_tspImplInit(ADM_VSM_TSP_IMPL_T* pstTspImplInfo)
{
    if (DNULL == pstTspImplInfo)
    {
        log_e("Input param is nil, [iRet:%d]", ADM_ECODE_INVALID_PARAM);
        return ADM_ECODE_INVALID_PARAM;
    }

    g_tspImplInfo.tspSchduleNotifyHmi = pstTspImplInfo->tspSchduleNotifyHmi;
    g_tspImplInfo.schduleInstallCallback = pstTspImplInfo->schduleInstallCallback;
    g_tspImplInfo.tspOneClickRescueCb = pstTspImplInfo->tspOneClickRescueCb;

    return 0;
}

D32S adm_vsm_tspImplDeinit(void)
{
    g_tspImplInfo.tspOneClickRescueCb = DNULL;

    return 0;
}
