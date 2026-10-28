#include "adm_dim_log.h"
#include "adm_dim_vdi.h"

#include "adm_com_errorCode.h"
#include "adm_typedefs.h"

#include "adm_memory.h"
#include "adm_string.h"

#include "adm_dim_api.h"
#include "adm_dim_apiImpl.h"
#include "adm_dim_vdiSpy.h"

#include "adm_cfg_api.h"
#include "adm_cfg_default.h"
#include "pub/inc/adm_pub_func.h"
#include "pub/inc/adm_pub_keylog.h"


D32S adm_dim_vdi_readDiagnosticInfo(const D16U instanceId, const D16U timeout,
                                    const ADM_DIM_SOA_ECU_IDEN_INFO_T* pstEcuIdenInfo,
                                    const DCHAR* diagScriptPath, const ADM_DIM_SOA_VDI_BUS_TYPE_E eBusType,
                                    const ADM_DIM_SOA_PIN_CHANNEL_TYPE_E eChannelType, const DBOOL isCGWRouted,
                                    const ADM_DIM_SOA_DID_DATA_ARRAY_T didListRequest,
                                    ADM_DIM_SOA_DID_DATA_ARRAY_T** didListResponse, D32S* errorCode)
{
    log_i("adm_dim_vdi_readDiagnosticInfo begin");

    D32S nRet = ADM_ECODE_FAILED;
    dim_func_list_t* func = adm_getFuncList();

    if(func == NULL || func->adm_dis_vdi_readDiagnosticInfo_handle == NULL)
    {
        nRet = ADM_ECODE_INVALID_POINTER;
        log_e("func or adm_dis_vdi_readDiagnosticInfo_handle pointer invalid, nRet[ %d ]",
              nRet);
        return nRet;
    }

    nRet = func->adm_dis_vdi_readDiagnosticInfo_handle(instanceId, timeout,
            pstEcuIdenInfo, diagScriptPath, eBusType, eChannelType,
            isCGWRouted, didListRequest, didListResponse, errorCode);
    log_i("adm_dim_vdi_readDiagnosticInfo end");

    return nRet;
}

D32S adm_dim_vdi_parallelCollectEcu(const D16U instanceId,
                                      const D16U timeout,
                                      const ADM_DIM_SOA_ECU_IDEN_INFO_T* pstEcuIdenInfo,
                                      const DCHAR* diagScriptPath, const ADM_DIM_SOA_VDI_BUS_TYPE_E eBusType,
                                      const ADM_DIM_SOA_PIN_CHANNEL_TYPE_E eChannelType,
                                      const DBOOL isCGWRouted, const ADM_DIM_SOA_DID_DATA_ARRAY_T didListRequest, D32S* errorCode)
{
    log_i("adm_dim_vdi_parallelCollectEcu begin");

    D32S nRet = ADM_ECODE_FAILED;
    dim_func_list_t* func = adm_getFuncList();

    if(func == NULL || func->adm_dis_vdi_parallelCollectEcu_handle == NULL)
    {
        nRet = ADM_ECODE_INVALID_POINTER;
        log_e("func or adm_dis_vdi_parallelCollectEcu_handle pointer invalid, nRet[ %d ]",
              nRet);
        return nRet;
    }

    nRet = func->adm_dis_vdi_parallelCollectEcu_handle(instanceId, timeout,
            pstEcuIdenInfo, diagScriptPath, eBusType, eChannelType,
            isCGWRouted, didListRequest, errorCode);
    log_i("adm_dim_vdi_parallelCollectEcu end");

    return nRet;
}


D32S adm_dim_vdi_reflashEcusTrigger(const D16U instanceId, const D16U timeout,
                                    const ADM_DIM_SOA_VDI_REFLASH_INFO_ARRAY_T* reflashInfoList, D32S* errorCode)
{
    log_i("adm_dim_vdi_reflashEcusTrigger begin");

    D32S nRet = ADM_ECODE_FAILED;
    DCHAR aDescription[128] = {0};
    dim_func_list_t* func = adm_getFuncList();

    if(func == NULL || func->adm_dis_vdi_reflashEcusTrigger_handle == NULL)
    {
        nRet = ADM_ECODE_INVALID_POINTER;
        log_e("func or adm_dim_vdi_reflashEcusTrigger_handle pointer invalid, nRet[ %d ]",
              nRet);
        return nRet;
    }

    nRet = func->adm_dis_vdi_reflashEcusTrigger_handle(instanceId, timeout,
            reflashInfoList, errorCode);

    if (ADM_IS_ERR(nRet))
    {
        adm_snprintf(aDescription, sizeof(aDescription), "errorCode=[%d] ", *errorCode);
        adm_pub_setKeylogString(ADM_PUB_KEYLOG_ID_030, nRet, aDescription);
    }
    else
    {
        adm_pub_setKeylogString(ADM_PUB_KEYLOG_ID_029, nRet, "");
    }

    
    log_i("adm_dim_vdi_reflashEcusTrigger end");

    return nRet;
}

D32S adm_dim_vdi_AbReflashEcusTrigger(const D16U instanceId, const D16U timeout, 
        DBOOL bIsEnableAb, ADM_DIM_SOA_VDI_ECU_AREA_INFO_E eAbInfo,
        const ADM_DIM_SOA_VDI_REFLASH_INFO_ARRAY_T* reflashInfoList, D32S* errorCode)
{
    log_i("adm_dim_vdi_AbReflashEcusTrigger begin");

    D32S nRet = ADM_ECODE_FAILED;
    dim_func_list_t* func = adm_getFuncList();

    if(func == NULL || func->adm_dis_vdi_AbReflashEcusTrigger_handle == NULL)
    {
        nRet = ADM_ECODE_INVALID_POINTER;
        log_e("func or adm_dim_vdi_AbReflashEcusTrigger_handle pointer invalid, nRet[ %d ]",
              nRet);
        return nRet;
    }

    nRet = func->adm_dis_vdi_AbReflashEcusTrigger_handle(instanceId, timeout,
            bIsEnableAb, eAbInfo,
            reflashInfoList, errorCode);
    log_i("adm_dim_vdi_AbReflashEcusTrigger end");

    return nRet;
}

D32S adm_dim_vdi_notifyFlashState(const D16U uiInstanceId, const D16U uiTimeout, 
        ADM_DIM_SOA_VDI_FLASH_STATE_E eFlashState, D32S* pErrorCode)
{
    log_i("adm_dim_vdi_notifyFlashState begin");

    D32S nRet = ADM_ECODE_FAILED;
    dim_func_list_t* func = adm_getFuncList();

    if(func == NULL || func->adm_dis_vdi_notifyFlashState_handle == NULL)
    {
        nRet = ADM_ECODE_INVALID_POINTER;
        log_e("func or adm_dim_vdi_notifyFlashState pointer invalid, nRet[ %d ]",nRet);
        return nRet;
    }

    nRet = func->adm_dis_vdi_notifyFlashState_handle(uiInstanceId, uiTimeout,
            (D32S)eFlashState, pErrorCode);
    log_i("adm_dim_vdi_notifyFlashState end");

    return nRet;
}

D32S adm_dim_vdi_clearDtc(const D16U uiInstanceId, const D16U uiTimeout,
                                D32S *pErrorCode)
{
    log_i("adm_dim_vdi_clearDtc begin");

    D32S nRet = ADM_ECODE_FAILED;
    dim_func_list_t *func = adm_getFuncList();

    if (func == NULL || func->adm_dis_vdi_clearDtc_handle == NULL)
    {
        nRet = ADM_ECODE_INVALID_POINTER;
        log_e("func or adm_dim_vdi_clearDtc pointer invalid, nRet[ %d ]", nRet);
        return nRet;
    }

    nRet = func->adm_dis_vdi_clearDtc_handle(uiInstanceId, uiTimeout, pErrorCode);
    log_i("adm_dim_vdi_clearDtc end");

    return nRet;
}

D32S adm_dim_vdi_queryReflashProgress(const D16U instanceId,
        const D16U timeout,
        const ADM_DIM_SOA_ECU_IDEN_INFO_T* pstEcuIdenInfo,
        D16U* reflashProgress, D32S* errorCode)
{
    log_i("adm_dim_vdi_queryReflashProgress begin");

    D32S nRet = ADM_ECODE_FAILED;
    dim_func_list_t* func = adm_getFuncList();

    if(func == NULL || func->adm_dis_vdi_queryReflashProgress_handle == NULL)
    {
        nRet = ADM_ECODE_INVALID_POINTER;
        log_e("func or adm_dim_vdi_queryReflashProgress_handle pointer invalid, nRet[ %d ]",
              nRet);
        return nRet;
    }

    nRet = func->adm_dis_vdi_queryReflashProgress_handle(instanceId, timeout,
            pstEcuIdenInfo, reflashProgress, errorCode);
    log_i("adm_dim_vdi_queryReflashProgress end");

    return nRet;

}

D32S adm_dim_vdi_queryReflashResult(const D16U instanceId, const D16U timeout,
                                    const ADM_DIM_SOA_ECU_IDEN_INFO_T* pstEcuIdenInfo,
                                    ADM_DIM_SOA_VDI_REFLASH_RESULT_INFO_T** pstReflashResultSendInfo,
                                    D32S* errorCode)
{
    log_i("adm_dim_vdi_queryReflashResult begin");

    D32S nRet = ADM_ECODE_FAILED;
    DCHAR aDescription[128] = {0};
    dim_func_list_t* func = adm_getFuncList();

    if(func == NULL || func->adm_dis_vdi_queryReflashResult_handle == NULL)
    {
        nRet = ADM_ECODE_INVALID_POINTER;
        log_e("func or adm_dim_vdi_queryReflashResult_handle pointer invalid, nRet[ %d ]",
              nRet);
        return nRet;
    }

    nRet = func->adm_dis_vdi_queryReflashResult_handle(instanceId, timeout,
            pstEcuIdenInfo, pstReflashResultSendInfo, errorCode);

    if (ADM_IS_ERR(nRet))
    {
        adm_snprintf(aDescription, sizeof(aDescription), "requestId[%0x],swDid[%0x],errorCode=[%d] ",pstEcuIdenInfo->requestId, pstEcuIdenInfo->swDid, *errorCode);
        adm_pub_setKeylogString(ADM_PUB_KEYLOG_ID_032, nRet, aDescription);
    }
    // else
    // {
    //     adm_pub_setKeylogString(ADM_PUB_KEYLOG_ID_031, nRet, "");
    // }
    
    log_i("adm_dim_vdi_queryReflashResult end");

    return nRet;

}

D32S adm_dim_vdi_performSpecDiagnosticTask(const D16U instanceId,
        const D16U timeout,
        const ADM_DIM_SOA_VDI_SPEC_DIAGNOSTIC_TASK_TYPE_E eSpecDiagTaskType,
        const DCHAR*    absolutePath, D32S* errorCode)
{
    log_i("adm_dim_vdi_performSpecDiagnosticTask begin");
    D32S nRet = ADM_ECODE_FAILED;
    dim_func_list_t* func = adm_getFuncList();

    if(func == NULL || func->adm_dis_vdi_performSpecDiagnosticTask_handle == NULL)
    {
        nRet = ADM_ECODE_INVALID_POINTER;
        log_e("func or adm_dim_vdi_performSpecDiagnosticTask_handle pointer invalid, nRet[ %d ]",
              nRet);
        return nRet;
    }

    nRet = func->adm_dis_vdi_performSpecDiagnosticTask_handle(instanceId, timeout,
            eSpecDiagTaskType, absolutePath, errorCode);
    log_i("adm_dim_vdi_performSpecDiagnosticTask end");

    return nRet;
}



/**
 * @brief Read vehicle status
 *
 * @param instanceId        [IN]        const D16U  instance identifier
 * @param timeout               [IN]        const D16U  Timeout of sync interface, unit: second
 * @param msgIdListRequest      [IN]    const ADM_DIM_SOA_VDI_MSGID_DATA_ARRAY_T*       List of vehicle statuses that need to be read
 * @param msgIdListResponse     [OUT]   ADM_DIM_SOA_VDI_MSGID_DATA_ARRAY_T**            Read the vehicle status list
 * @param errorCode             [OUT]   D32S*   The execution result of this request
 * @return D32S
 */
D32S adm_dim_vdi_readVehicleStatus(const D16U instanceId,
                                   const D16U timeout,
                                   const ADM_DIM_SOA_VDI_MSGID_DATA_ARRAY_T* msgIdListRequest,
                                   ADM_DIM_SOA_VDI_MSGID_DATA_ARRAY_T** msgIdListResponse, D32S* errorCode)
{
    log_i("adm_dim_vdi_readVehicleStatus begin");
    D32S nRet = ADM_ECODE_FAILED;
    dim_func_list_t* func = adm_getFuncList();

    if(func == NULL || func->adm_dis_vdi_readVehicleStatus_handle == NULL)
    {
        nRet = ADM_ECODE_INVALID_POINTER;
        log_e("func or adm_dim_vdi_readVehicleStatus_handle pointer invalid, nRet[ %d ]",
              nRet);
        return nRet;
    }

    nRet = func->adm_dis_vdi_readVehicleStatus_handle(instanceId, timeout,
            msgIdListRequest, msgIdListResponse, errorCode);
    log_i("adm_dim_vdi_readVehicleStatus end");

    return nRet;

}


/**
 * @brief Set the list of vehicle status that needs to be monitored
 *
 * @param instanceId        [IN]        const D16U  instance identifier
 * @param timeout               [IN]        const D16U  Timeout of sync interface, unit: second
 * @param msgIdList  [IN]   const ADM_DIM_SOA_VDI_MSGID_DATA_ARRAY_T*         List of vehicle statuses that need to be spy
 * @param errorCode         [OUT]   D32S*   The execution result of this request
 * @return D32S         0:successed, Non-0:failed
 */
D32S adm_dim_vdi_setSpyVehicleStatus(const D16U instanceId, const D16U timeout,
                                     const D64U u64TaskId,
                                     const ADM_SYSI4VDI_ENGINE_SPY_ARRAY_T* spyList, D32S* errorCode)
{
    log_i("adm_dim_vdi_setSpyVehicleStatus begin");

    D32S nRet = ADM_ECODE_FAILED;
    dim_func_list_t* func = adm_getFuncList();

    if(func == NULL || func->adm_dis_vdi_setSpyVehicleStatus_handle == NULL)
    {
        nRet = ADM_ECODE_INVALID_POINTER;
        log_e("func or adm_dim_vdi_setSpyVehicleStatus_handle pointer invalid, nRet[ %d ]",
              nRet);
        return nRet;
    }

    nRet = func->adm_dis_vdi_setSpyVehicleStatus_handle(instanceId, timeout,
            u64TaskId, spyList, errorCode);
    log_i("adm_dim_vdi_setSpyVehicleStatus end");

    return nRet;

}

D32S adm_dim_vdi_clearSpyVehicleStatus(const D16U instanceId,
                                       const D16U timeout,
                                       const D64U u64TaskId, D32S* errorCode)
{
    log_i("adm_dim_vdi_clearSpyVehicleStatus begin");
    D32S nRet = ADM_ECODE_FAILED;
    dim_func_list_t* func = adm_getFuncList();

    if(func == NULL || func->adm_dis_vdi_clearSpyVehicleStatus_handle == NULL)
    {
        nRet = ADM_ECODE_INVALID_POINTER;
        log_e("func or adm_dim_vdi_clearSpyVehicleStatus_handle pointer invalid, nRet[ %d ]",
              nRet);
        return nRet;
    }

    nRet = func->adm_dis_vdi_clearSpyVehicleStatus_handle(instanceId, timeout,
            u64TaskId, errorCode);
    log_i("adm_dim_vdi_clearSpyVehicleStatus end");

    return nRet;
}

D32S adm_dim_vdi_controlVehicleStatus(const D16U instanceId, const D16U timeout,
                                      ADM_DIM_SOA_VDI_CTRL_VEHICLE_STATUS_TYPE_E eVechielStatusType,
                                      ADM_DIM_SOA_DATA_T* pDiddate,
                                      const D32U keeptime, D32S* errorCode)
{
    log_i("adm_dim_vdi_controlVehicleStatus begin");
    D32S nRet = ADM_ECODE_FAILED;
    dim_func_list_t* func = adm_getFuncList();

    if(func == NULL || func->adm_dis_vdi_controlVehicleStatus_handle == NULL)
    {
        nRet = ADM_ECODE_INVALID_POINTER;
        log_e("func or adm_dim_vdi_controlVehicleStatus_handle pointer invalid, nRet[ %d ]",
              nRet);
        return nRet;
    }

    nRet = func->adm_dis_vdi_controlVehicleStatus_handle(instanceId, timeout,
            eVechielStatusType, pDiddate, keeptime, errorCode);
    log_i("adm_dim_vdi_controlVehicleStatus end");
    return nRet;
}

D32S adm_dim_vdi_softwareVersionCheckProcess(const D16U instanceId,
        const D16U timeout, const ADM_DIM_SOA_ECU_IDEN_INFO_T* pstEcuIdenInfo,
        const ADM_DIM_SOA_VDI_SOFT_VERSION_CHECK_TYPE_E eCheckType,
        const DBOOL isEnterBorkenMode, D32S* errorCode)
{
    log_i("adm_dim_vdi_softwareVersionCheckProcess begin");

    D32S nRet = ADM_ECODE_FAILED;
    dim_func_list_t* func = adm_getFuncList();

    if(func == NULL
            || func->adm_dis_vdi_softwareVersionCheckProcess_handle == NULL)
    {
        nRet = ADM_ECODE_INVALID_POINTER;
        log_e("func or adm_dim_vdi_softwareVersionCheckProcess_handle pointer invalid, nRet[ %d ]",
              nRet);
        return nRet;
    }

    nRet = func->adm_dis_vdi_softwareVersionCheckProcess_handle(instanceId, timeout,
            pstEcuIdenInfo, eCheckType, isEnterBorkenMode, errorCode);
    log_i("adm_dim_vdi_softwareVersionCheckProcess end");

    return nRet;

}

D32S adm_dim_vdi_set_log_level(const D16U instanceId, const D16U timeout,
                               D32U logLevel, D32S* errorCode)
{
    log_i("adm_dim_vdi_set_log_level begin");

    D32S nRet = ADM_ECODE_FAILED;
    dim_func_list_t* func = adm_getFuncList();

    if(func == NULL || func->adm_dis_vdi_set_log_level_handle == NULL)
    {
        nRet = ADM_ECODE_INVALID_POINTER;
        log_e("func or adm_dis_vdi_set_log_level_handle pointer invalid, nRet[ %d ]",
              nRet);
        return nRet;
    }

    nRet = func->adm_dis_vdi_set_log_level_handle(instanceId, timeout, logLevel,
            errorCode);
    log_i("adm_dim_vdi_set_log_level end");

    return nRet;
}

/**
 * @brief 触发编程
 * @note 预编程和后编程
 * @param in const D16U usInstanceId    服务实例ID
 * @param in const D16U usTimeout       超时时间
 * @param in const DCHAR *pcRequestBuf  请求数据
 * @param out DCHAR **ppcResponseBuf    响应数据
 * @retval D32S 错误码
 */
D32S adm_dim_vdi_programEcusTrigger(const D16U usInstanceId,
                                    const D16U usTimeout,
                                    const DCHAR *pcRequestBuf,
                                    DCHAR **ppcResponseBuf)
{
    log_i("adm_dim_vdi_programEcusTrigger begin");

    D32S iRet = ADM_ECODE_FAILED;
    dim_func_list_t* func = adm_getFuncList();

    if(func == NULL || func->adm_dis_vdi_programm_trigger_handle == NULL)
    {
        iRet = ADM_ECODE_INVALID_POINTER;
        log_e("func or adm_dis_vdi_programm_trigger_handle pointer invalid, nRet[ %d ]",
              iRet);
        return iRet;
    }

    iRet = func->adm_dis_vdi_programm_trigger_handle(usInstanceId, usTimeout, pcRequestBuf, ppcResponseBuf);
    if (ADM_IS_ERR(iRet))
    {
        adm_pub_setKeylogString(ADM_PUB_KEYLOG_ID_036, iRet, "");
    }
    else
    {
        adm_pub_setKeylogString(ADM_PUB_KEYLOG_ID_035, iRet, "");
    }

    log_i("adm_dim_vdi_programEcusTrigger end");

    return iRet;
}

void adm_dim_vdi_getSdkVersion(const D16U instanceId, const D16U timeout,
                               DCHAR** sdkVersion, D32S* errorCode)
{
    log_i("adm_dim_vdi_getSdkVersion begin");

    dim_func_list_t* func = adm_getFuncList();

    if(func == NULL || func->adm_dis_vdi_getSdkVersion_handle == NULL)
    {
        log_e("func or adm_dis_vdi_set_log_level_handle pointer invalid");
        return;
    }

    func->adm_dis_vdi_getSdkVersion_handle(instanceId, timeout, sdkVersion,
                                           errorCode);
    log_i("adm_dim_vdi_getSdkVersion end");

    return;
}

D32S adm_dim_vdi_readAbInfo(const D16U uiInstanceId,
        const D16U uiTimeout,
        const ADM_DIM_SOA_ECU_IDEN_INFO_T* pstEcuIdenInfo,
        const ADM_DIM_SOA_VDI_BUS_TYPE_E eBusType,
        const ADM_DIM_SOA_PIN_CHANNEL_TYPE_E eChannelType,
        DBOOL* pAreaABEnable,
        ADM_DIM_SOA_VDI_ECU_AREA_INFO_E *pAreaInfo,
        D32U* pErrorCode)
{
    log_i("adm_dim_vdi_readAbInfo begin");

    D32S iRet = ADM_ECODE_FAILED;
    dim_func_list_t* func = adm_getFuncList();

    if(func == NULL || func->adm_dis_vdi_readAbInfo_handle == NULL)
    {
        iRet = ADM_ECODE_INVALID_POINTER;
        log_e("func or adm_dis_vdi_readAbInfo_handle pointer invalid, nRet[ %d ]", iRet);
        return iRet;
    }

    iRet = func->adm_dis_vdi_readAbInfo_handle(uiInstanceId, uiTimeout, pstEcuIdenInfo, eBusType,
                                        eChannelType, pAreaABEnable, pAreaInfo, pErrorCode);

    log_i("adm_dim_vdi_readAbInfo end");

    return iRet;
}

D32S adm_dim_vdi_switchAb(const D16U uiInstanceId,
        const D16U uiTimeout,
        const ADM_DIM_SOA_ECU_IDEN_INFO_T* pstEcuIdenInfo,
        const DCHAR *pSwitchScriptPath,
        const ADM_DIM_SOA_VDI_BUS_TYPE_E eBusType,
        const ADM_DIM_SOA_PIN_CHANNEL_TYPE_E eChannelType,
        const ADM_DIM_SOA_VDI_ECU_AREA_INFO_E eDstAreaInfo,
        ADM_DIM_SOA_SA_MASK_INFO_T *pstSaMaskInfo,
        D32U* pErrorCode)
{
    log_i("adm_dim_vdi_switchAb begin");

    D32S iRet = ADM_ECODE_FAILED;
    dim_func_list_t* func = adm_getFuncList();

    if(func == NULL || func->adm_dis_vdi_switchAb_handle == NULL)
    {
        iRet = ADM_ECODE_INVALID_POINTER;
        log_e("func or adm_dis_vdi_switchAb_handle pointer invalid, nRet[ %d ]", iRet);
        return iRet;
    }

    iRet = func->adm_dis_vdi_switchAb_handle(uiInstanceId, uiTimeout, pstEcuIdenInfo, 
                                            pSwitchScriptPath, eBusType, eChannelType, 
                                            eDstAreaInfo, pstSaMaskInfo, pErrorCode);

    log_i("adm_dim_vdi_switchAb end");

    return iRet;
}

/**
 * @brief 触发重启
 * @param in const D16U usInstanceId    服务实例ID
 * @param in const D16U usTimeout       超时时间
 * @param out D32S *errorCode          错误码
 * @retval D32S 错误码
 */
D32S adm_dim_vdi_programReboot(const D16U usInstanceId,
                               const D16U usTimeout,
                               D32S *errorCode)
{
    ADM_COM_FUNC_ENTRY();
    
    D32S iRet = ADM_ECODE_FAILED;

    dim_func_list_t* func = adm_getFuncList();

    if(func == NULL || func->adm_dis_vdi_programm_reboot_handle == NULL)
    {
        log_e("func or adm_dis_vdi_programm_reboot_handle pointer invalid");
        iRet = ADM_ECODE_INVALID_POINTER;
        return iRet;
    }

    iRet = func->adm_dis_vdi_programm_reboot_handle(usInstanceId, usTimeout, errorCode);

    ADM_COM_FUNC_EXIT();

    return iRet;
}

/**
 * @brief 设置诊断通道
 * @param in const D16U usInstanceId    服务实例ID
 * @param in const D16U usTimeout       超时时间
 * @param out D32S *errorCode          错误码
 * @return D32S 错误码
 */
D32S adm_dim_vdi_setDiagTp(const D16U usInstanceId,
                           const D16U usTimeout,
                           D32S *errorCode)
{
    ADM_COM_FUNC_ENTRY();
    
    D32S iRet = ADM_ECODE_FAILED;

    dim_func_list_t* func = adm_getFuncList();

    if(func == NULL || func->adm_dis_vdi_setDiagTp_handle == NULL)
    {
        log_e("func or adm_dis_vdi_setDiagTp_handle pointer invalid");
        iRet = ADM_ECODE_INVALID_POINTER;
        return iRet;
    }

    iRet = func->adm_dis_vdi_setDiagTp_handle(usInstanceId, usTimeout, errorCode);

    ADM_COM_FUNC_EXIT();

    return iRet;
}

D32S adm_dim_vdi_init(ADM_DIM_SOA_VDI_INIT_INFO_T* initInfo, void* priv)
{
    log_i("adm_dim_vdi_init begin");

    log_i("appPriv: %p ", priv);

    D32S nRet = ADM_ECODE_FAILED;
    dim_func_list_t* func = adm_getFuncList();

    if(func == NULL || func->adm_dis_vdi_init_handle == NULL)
    {
        nRet = ADM_ECODE_INVALID_POINTER;
        log_e("func or adm_dim_vdi_init_handle pointer invalid, nRet[ %d ]", nRet);
        return nRet;
    }

    DCHAR* cfgInstances = adm_strdup(adm_cfg_iniParserGetString(ADM_CFG_VUS_VDI_INSTANCEID,  ""));

    DCHAR* subInstanceIdStr = NULL;
    subInstanceIdStr = strsep(&cfgInstances, ",");
    log_i("subInstanceIdStr:%s", subInstanceIdStr);

    while(NULL != subInstanceIdStr)
    {
        log_i("subInstanceIdStr:%s", subInstanceIdStr);
        nRet = func->adm_dis_vdi_init_handle(atoi(subInstanceIdStr), initInfo, priv);
        subInstanceIdStr = strsep(&cfgInstances, ",");
        adm_dim_vdiSpyInit();
    }

    adm_free(cfgInstances);

    log_i("adm_dim_vdi_init end");

    return nRet;
}

void adm_dim_vdi_deinit(void)
{
    log_i("adm_dim_vdi_deinit begin");

    dim_func_list_t* func = adm_getFuncList();

    if(func == NULL || func->adm_dis_vdi_deinit_handle == NULL)
    {
        log_e("func or adm_dim_vdi_deinit_handle pointer invalid");
        return ;
    }

    DCHAR* instance = NULL;
    DCHAR* cfgInstances = adm_strdup(adm_cfg_iniParserGetString(ADM_CFG_VUS_VDI_INSTANCEID,  ""));

    instance = strsep(&cfgInstances, ",");

    while(instance != NULL)
    {
        func->adm_dis_vdi_deinit_handle(atoi(instance));
        instance = strsep(&cfgInstances, ",");
    }

    adm_free(cfgInstances);
    log_i("adm_dim_vdi_deinit end");
}

