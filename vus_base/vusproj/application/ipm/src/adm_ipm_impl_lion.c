#include "adm_ipm_log.h"
#include "adm_ipm_impl_lion.h"
#include "adm_ips_install.h"
#include "adm_ips_debug.h"
#include "adm_cvs_versionPolicy.h"
#include "adm_typedefs.h"
#include "adm_com_errorCode.h"
#include "adm_com_hash.h"
#include "adm_application_errorCode.h"
#include "adm_string.h"
#include "adm_memory.h"
#include "adm_list.h"
#include "adm_uim_install.h"
#include "adm_uim_request.h"
#include "adm_dim_vdi.h"
#include "adm_dim_host.h"
#include "adm_dim_ucl.h"
#include "adm_vrs_registerInfo.h"
#include "adm_ocs_airProtocolSecurity.h"
#include "adm_soa.h"
#include "adm_vsm_fsm.h"
#include "adm_cvm_checkVersion.h"
#include "adm_cdm_verify_interface.h"
#include "adm_application_common.h"
#include "adm_pis_package.h"
#include "adm_com_kvstorage.h"
#include "adm_com_kvstorage.h"
#include "adm_irm_report.h"
#include "adm_irm_logFile.h"
#include "adm_eim_ecuInfoManager.h"
#include "adm_eim_parallel.h"
#include "adm_ecs_kvStorage.h"
#include "adm_dis_gac_priv.h"
#include "adm_uim_download.h"
#include "adm_ecs_parsePolicy.h"
#include "adm_dps_downloadService.h"
#include "adm_cfg_api.h"
#include "adm_file.h"
#include "adm_pub_func.h"
#include "adm_ocm_interface.h"
#include "adm_eim_getEcuInfo.h"
#include "adm_sdk_selfUpgrade.h"
#include "adm_sdk_version.h"
#include "adm_cvm_syncVehicleInfo.h"


#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <time.h>
#include <pthread.h>

#define ADM_IPM_SCHDULE_ARRIVE_COND_FILE_PATH "/data_fota/ota/etc/schduleInstallConditions.json"
#define ADM_IPM_ICE_SCHDULE_ARRIVE_COND_FILE_PATH "/data_fota/ota/etc/ICE-schduleInstallConditions.json"
#define ADM_IPM_QUERY_SMART_COND_FILE_PATH "/data_fota/ota/etc/smartScenConditions.json"

typedef enum 
{
    ADM_COND_SCHDULE_ARRIVED = 1,//预约时间到达条类型
    ADM_COND_QUERY_SMART = 2,//智能场景查询场景类型
} ADM_COND_COMPARE_TYPE_E;

static D32S ipmHtoi(const DCHAR *hex){
    D32S a = 0;
    if(NULL != hex){a = htoi(hex);}
    return a;
}

static D32S ipmGetRealCondResult(D32S iResult, ADM_UIM_SOA_CONDITION_DATA_ARRAY_T* pstCondResults)
{
    D32S iRet = iResult;

    do
    {
        if (ADM_IS_NULL(pstCondResults))
        {
            log_e("Invalid param.");
            break;
        }

        for (D16U i = 0; i < pstCondResults->condsArrayLength; i++)
        {
            if (NULL != pstCondResults->condsArray)
            {
                if (ADM_ECODE_SUCCESSED != pstCondResults->condsArray[i]->result)
                {
                    if (0x30D == pstCondResults->condsArray[i]->msgId)
                    {
                        iRet = ADM_ECODE_INSTALL_IN_CHILDPET_SCENARIO;
                        break;
                    }
                    else if (0x30C == pstCondResults->condsArray[i]->msgId)
                    {
                        iRet = ADM_ECODE_INSTALL_IN_SENTINEL_SCENARIO;
                        break;
                    }
                    else 
                    {
                        continue;
                    }
                }
            }
        }
    } while (DFALSE);

    return iRet;
}

static D32S ipmSchduleArrivedCondVerify(ADM_COND_COMPARE_TYPE_E eType, DCHAR **ppcResultBuf)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    ADM_COND_POLICY_T stConditions = {0};
    DCHAR *pcConditions = NULL;
    json_object *pJsonObj = NULL;
    ADM_UIM_SOA_CONDITION_DATA_ARRAY_T *pstCondResults = NULL;
    DBOOL bIsConform = FALSE;
    D32S iResult = ADM_ECODE_SUCCESSED;
    const DCHAR* pCondPath = NULL;
    DBOOL bIsFuel = DFALSE;  

    ADM_COM_FUNC_ENTRY();

    do
    {
        /** 入参校验 */
        if (ADM_IS_NULL(ppcResultBuf))
        {
            log_e("ppcResultBuf is NULL");
            iRet = ADM_ECODE_INVALID_PARAM;
            break;
        }

        if (ADM_COND_SCHDULE_ARRIVED == eType)
        {
            iRet = adm_vsm_getIsFuel(&bIsFuel);
            if (ADM_IS_ERR(iRet))
            {
                log_e("Get vehicle power type failed, iRet:%d", iRet);
                return ADM_ECODE_FAILED;  // 直接返回失败
            }

            if (DTRUE == bIsFuel)
            {
                log_i("ICE Vechile SCHDULE condition Verify start");
                pCondPath = ADM_IPM_ICE_SCHDULE_ARRIVE_COND_FILE_PATH;
            }
            else
            {
                pCondPath = ADM_IPM_SCHDULE_ARRIVE_COND_FILE_PATH;
            }
        }
        else if (ADM_COND_QUERY_SMART == eType)
        {
            pCondPath = ADM_IPM_QUERY_SMART_COND_FILE_PATH;
        }
        else 
        {
            log_e("Invalid cond type[%d].", eType);
            iRet = ADM_ECODE_INVALID_PARAM;
            break;
        }

        adm_condPolicyInit(&stConditions);

        /**读取条件配置文件 */
        iRet = adm_com_readFileData(pCondPath, &pcConditions);
        if (ADM_IS_ERR(iRet))
        {
            log_e("read file failed, fileName[%s]", pCondPath);
            iRet = ADM_ECODE_READ_FILE_FAILED;
            break;
        }

        /** 解析条件配置文件 */
        pJsonObj = json_tokener_parse(pcConditions);
        if (NULL == pJsonObj)
        {
            log_e("json_tokener_parse failed");
            iRet = ADM_ECODE_STRING_TO_JSON_FAILED;
            break;
        }

        /** 解析前置条件 */
        adm_condPolicyJsonParse(&stConditions, pJsonObj);

        /** 验证前置条件 */
        iResult = adm_cdm_verify_Compare(&stConditions, &pstCondResults, &bIsConform);
        if (ADM_IS_ERR(iResult))
        {
            /** 获取结果 */
            *ppcResultBuf = adm_ipm_reportConditionsNotMet(iResult, &stConditions, pstCondResults);
        }
    } while (FALSE);

    if (NULL != pJsonObj)
    {
        json_object_put(pJsonObj);
    }

    adm_condPolicyDeInit(&stConditions);
    adm_free(pcConditions);

    iRet = (iRet == ADM_ECODE_SUCCESSED) ? iResult : iRet;

    log_i("condition type[%d], retValue[%d]", eType, iRet);

    if (ADM_COND_QUERY_SMART == eType)
    {
        if (ADM_IS_ERR(iRet) && NULL != pstCondResults)
        {
            ipmGetRealCondResult(iRet, pstCondResults);
            log_i("Real retValue[%d]", iRet);
        }
    }

    /** 内存释放 */
    if (NULL != pstCondResults)
    {
        (void)adm_cdm_verify_free(pstCondResults);
    }

    ADM_COM_FUNC_EXIT();

    return iRet;
}

/*!
 *******************************************************************************
 * 部分智能场景/前置条件判断
 *
 * \note
 * 
 * \param[in]   pVehicleStatus 信号合集
 * \param[in]   uiNum 信号数量
 * \param[in]   bNeedSmart 是否需要满足智能场景
 * \param[out]   pCondSatisfy 部分前置条件是否满足（OFF挡、设防、非充电、非哨兵、非童锁/童宠）
 * \retval      none
 ******************************************************************************
*/
static D32S ipmGetSchduleNotifyPreCondition(DBOOL bNeedSmart, DBOOL* pCondSatisfy, DCHAR** ppVerifyResult)
{
    ADM_DIM_SOA_MSGID_DATA_ARRAY_T stReqMsgData = {};
    ADM_DIM_SOA_MSGID_DATA_ARRAY_T* pstRespMsgData=NULL;
    D32S iRet = ADM_ECODE_FAILED;
    D32S iErrCode = ADM_ECODE_FAILED;
    ADM_COND_COMPARE_TYPE_E eCondType;

    if (NULL == pCondSatisfy || NULL == ppVerifyResult)
    {
        log_w("Invalid param.");
        return ADM_ECODE_INVALID_PARAM;
    }

    if (DTRUE == bNeedSmart)
    {
        /** 要求是智能场景 */
        eCondType = ADM_COND_QUERY_SMART;
    }
    else 
    {
        /** 预约时间到达前置条件判断，要求非智能场景 */
        eCondType = ADM_COND_SCHDULE_ARRIVED;
    }

    iRet = ipmSchduleArrivedCondVerify(eCondType, ppVerifyResult);

    if (ADM_IS_SUCCESS(iRet))
    {
        *pCondSatisfy = DTRUE;
    }
    else
    {
        *pCondSatisfy = DFALSE;
    }

    return iRet;
}

/*!
 *******************************************************************************
 * 预约时间到达部分前置条件判断
 *
 * \note
 * 
 * \param[out]   pCondSatisfy 部分前置条件是否满足（OFF挡、设防、非充电、非哨兵、非童锁/童宠）
 * \param[out]   ppVerifyResult 条件校验结果列表
 * \retval      none
 *******************************************************************************
 */
void adm_ipm_getSchduleNotifyPreConditon(DBOOL* pCondSatisfy, DCHAR** ppVerifyResult)
{
    if (NULL == pCondSatisfy || NULL == ppVerifyResult)
    {
        log_e("Invalid param.");
        return;
    }

#ifdef ADM_IPM_SIGNAL
    log_i("debug module.");
    *pCondSatisfy = DTRUE;
#else
    // VEHICLE_STATUS_E aVehicleStatus[5] = {VEHICLE_STATUS_POWER_SIGNAL, VEHICLE_STATUS_ANTI_THEFT_STATUS, 
    //                 VEHICLE_STATUS_CHARGING, VEHICLE_STATUS_SENTINEL_MODE, VEHICLE_STATUS_CHILDPET_MODE};
    // D16U uiNum = 5;
    (DVOID)ipmGetSchduleNotifyPreCondition(DFALSE, pCondSatisfy, ppVerifyResult);
#endif
    return;
}
static DVOID int_timestamp_to_string(int timestamp, char *buffer, size_t buffer_size) {
    time_t time_stamp = (time_t)timestamp;
    struct tm *local_time = localtime(&time_stamp);
    if (local_time != NULL) {
        strftime(buffer, buffer_size, "%y/%m/%d %H:%M:%S", local_time);
    } else {
        snprintf(buffer, buffer_size, "");
        log_e("handle error timestamp %d ",timestamp);
    }
}

DVOID adm_ipm_getInstallSchedule(DCHAR ** pScheduleInfo)
{
    D32S nRet = ADM_ECODE_SUCCESSED;
    D32S iSource = -1;
    D64S iSchduleTime = -1;
    D64S iTimeStamp = -1;

    // ipmGetScheduleTimeStamp(&iSource,&iSchduleTime);
    nRet = ipmGetScheduleInfo(&iSource,&iSchduleTime,&iTimeStamp);
    if (ADM_IS_ERR(nRet))
    {
        log_e("get install schedule failed. %d ",nRet);
        *pScheduleInfo = adm_strdup("");
        log_e("get install schedule failed. %s ",*pScheduleInfo);
        return;
    }
    char time_string[1024] = {0};
    int_timestamp_to_string(iTimeStamp, time_string, sizeof(time_string));
    *pScheduleInfo = adm_strdup(time_string);
    log_i("get install schedule : %s time_string :%s ", *pScheduleInfo,time_string);
}
void adm_ipm_getIsSmartScenario(DBOOL* pCondSatisfy, D32S* pResult)
{
    DCHAR* pVerifyResult = NULL;
    D32S iRet = ADM_ECODE_FAILED;

    if (NULL == pCondSatisfy || NULL == pResult)
    {
        log_e("Invalid param.");
        return;
    }

#ifdef ADM_IPM_SIGNAL
    log_i("smart scenario debug module.");
    *pCondSatisfy = DFALSE;
#else
    // VEHICLE_STATUS_E aVehicleStatus[2] = {VEHICLE_STATUS_SENTINEL_MODE, VEHICLE_STATUS_CHILDPET_MODE};
    // D16U uiNum = 2;
    iRet = ipmGetSchduleNotifyPreCondition(DTRUE, pCondSatisfy, &pVerifyResult);
    if (DTRUE == (*pCondSatisfy))
    {
        if (iRet != ADM_ECODE_INSTALL_IN_SENTINEL_SCENARIO && iRet != ADM_ECODE_INSTALL_IN_CHILDPET_SCENARIO)
        {
            *pResult = -1;
        }
        else 
        {
            *pResult = iRet;
        }
    }
#endif
    return;
}

static D32S ipmReNotiPolicyResultGet(D32S* pSumResult)
{
    DBOOL bIsOk = DFALSE;
    size_t uiValLength = 0;
    DCHAR* pInfo = NULL;
    ADM_KV_STORAGE_VALUE_TYPE_E eValType = ADM_KV_STORAGE_VALUE_TYPE_INVALID;
    D32S iRet = ADM_ECODE_FAILED;
    json_object *jsonRoot = NULL;

    do
    {
        bIsOk = adm_com_kvstorage_get_string(adm_core_service_getBusinessDatabaseDb(), ADM_IPM_NEED_REPEAT_NOTIFY_POLICY_RESULT, 
                                &pInfo, &uiValLength, &eValType);
        if (!bIsOk)
        {
            log_i("Failed to get adm_com_kvstorage_get_string, key[%s]", ADM_IPM_NEED_REPEAT_NOTIFY_POLICY_RESULT);
            iRet =  ADM_ECODE_KV_GET_STRING_FAILED;
            break;
        }

        if (NULL == pInfo)
        {
            log_e("key[%s] - value is null.", ADM_IPM_NEED_REPEAT_NOTIFY_POLICY_RESULT);
            break;
        }

        jsonRoot = json_tokener_parse(pInfo);
        if (ADM_IS_NULL(jsonRoot)) 
        {
            iRet = ADM_ECODE_STRING_TO_JSON_FAILED;
            log_e("Failed string to json, string[%s]", pInfo);
            break;
        }

        iRet = adm_com_getJsonIntVal(jsonRoot, "result", pSumResult);
        if (ADM_IS_ERR(iRet))
        {
            log_e( "Failed to json value, key[result], err[%d]", iRet);
            break;
        }

        iRet = ADM_ECODE_SUCCESSED;
    } while (FALSE);

    (void)json_object_put(jsonRoot);

    return iRet;
}

DVOID adm_ipm_HandleRecover()
{
    adm_ipm_init();
    D32S iResult = ADM_ECODE_FAILED;
    D32S iRet = ADM_ECODE_FAILED;
    D32S respCode = ADM_ECODE_FAILED;
    ADM_UIM_SOA_INSTALL_RESULT_INFO_T stEcuInstallResult = {0};

    /** 1、确定是否重新通知policyInstallResult */
    if(TRUE == adm_com_kvstorage_has_key(adm_core_service_getBusinessDatabaseDb(), ADM_IPM_NEED_REPEAT_NOTIFY_POLICY_RESULT))
    {
        iRet = ipmReNotiPolicyResultGet(&iResult);
        if (ADM_IS_SUCCESS(iRet))
        {
            DCHAR* sessionId = uimSoaVehicleSessionId();
            uimSoaInstallResultInfoInitVehicle(&stEcuInstallResult, iResult);
            adm_uim_hmi_repeat_policy_installResult_RR(uimSoaCmdidTypeGet(), sessionId, ADM_STR_FROM_PARAM(ADM_IPS_PHASE_POST_INSTALL), &stEcuInstallResult, &respCode);
            log_i("adm_uim_hmi_repeat_policy_installResult_RR respose code[%d]", respCode);
        }
    }

    return;
}

static void ipmNeedReNotiPolicyResultClear()
{
    if(TRUE == adm_com_kvstorage_has_key(adm_core_service_getBusinessDatabaseDb(), ADM_IPM_NEED_REPEAT_NOTIFY_POLICY_RESULT))
	{
		adm_com_kvstorage_remove_key(adm_core_service_getBusinessDatabaseDb(), ADM_IPM_NEED_REPEAT_NOTIFY_POLICY_RESULT);	
	}	

    return;
}

DVOID adm_ipm_notifyConfirmHandle(D32S *pResult)
{
    if (NULL != pResult)
    {
        ipmNeedReNotiPolicyResultClear();
        *pResult = ADM_ECODE_SUCCESSED;
    }
    else 
    {
        log_w("Invalid param.");
    }

    return;
}

D32S adm_ipm_getAbInfo(D16U uiInstanceId, D16U uiTimeOut, DBOOL* pIsEnableAb, 
            ADM_DIM_SOA_VDI_ECU_AREA_INFO_E* pAbInfo, 
            ADM_DIM_SOA_VDI_REFLASH_INFO_ARRAY_T* pReflashInfoList)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    D32U uiErrorCode = ADM_ECODE_SUCCESSED;

    if (ADM_IS_NULL(pIsEnableAb) || ADM_IS_NULL(pAbInfo) || 
        ADM_IS_NULL(pReflashInfoList))
    {
        log_e("Invalid param.");
        iRet = ADM_ECODE_INVALID_PARAM;
        return iRet;
    }

    log_i("[ABUP-Procedure]: Install Start Vdi Flash get AB info.");
    iRet = adm_dim_vdi_readAbInfo(uiInstanceId, uiTimeOut,
        pReflashInfoList->reflashInfoArray[0]->pstEcuIdenInfo, 
        pReflashInfoList->reflashInfoArray[0]->eBusType,
        pReflashInfoList->reflashInfoArray[0]->eChannelType,
        pIsEnableAb, pAbInfo, &uiErrorCode);
    log_i("Get ab info ret[%d], errorCode[%u]", iRet, uiErrorCode);

    iRet = (iRet == ADM_ECODE_SUCCESSED) ? uiErrorCode : iRet;

    return iRet;
}

DVOID adm_ipm_getDestAbInfo(ADM_DIM_SOA_VDI_ECU_AREA_INFO_E pSrcAbInfo, ADM_DIM_SOA_VDI_ECU_AREA_INFO_E* pDstAbInfo)
{
    if (ADM_IS_NULL(pDstAbInfo))
    {
        log_e("Invalid param.");
        return;
    }

    switch (pSrcAbInfo)
    {
        case ADM_SYSI4VDI_ECU_AREA_INFO_A:
            *pDstAbInfo = ADM_SYSI4VDI_ECU_AREA_INFO_B;
            break;
        case ADM_SYSI4VDI_ECU_AREA_INFO_B:
            *pDstAbInfo = ADM_SYSI4VDI_ECU_AREA_INFO_A;
            break;
        default:
            log_w("Can not find match type[%d]", pSrcAbInfo);
            *pDstAbInfo = ADM_SYSI4VDI_ECU_AREA_INFO_UNKNOW;
            break;

    }

    return;
}

DVOID adm_ipm_getExitDestAbInfo(const DCHAR* pRequestId, const DCHAR* pSwDid, ADM_DIM_SOA_VDI_ECU_AREA_INFO_E* pExitDstAbInfo)
{
    if (ADM_IS_NULL(pRequestId) || ADM_IS_NULL(pSwDid) || 
        ADM_IS_NULL(pExitDstAbInfo))
    {
        log_e("Invalid param.");
        return;
    }
    
    D32S iRet = ADM_ECODE_FAILED;

    ADM_IPS_ECU_PROGRESS_T progress = {0};
    adm_ips_ecuProgressInit(&progress);
    iRet = adm_ips_ecuProgressGet(&progress, pRequestId, pSwDid);
    if (ADM_IS_SUCCESS(iRet))
    {
        *pExitDstAbInfo = progress.eDestAbInfo;
    }
    else 
    {
        log_w("Can not find progress.");
    }

    adm_ips_ecuProgressDeInit(&progress);
}

#define ADM_IPM_DEFAULT_MASK_LEVEL 0x07
DVOID adm_ipm_initSaMask(const DCHAR* pRequestId, const DCHAR* pSwDid, ADM_DIM_SOA_SA_MASK_INFO_T **ppSaMaskList)
{
    D32S iRet = ADM_ECODE_FAILED;
    ADM_DIM_SOA_SA_MASK_INFO_T* pSaMaskList = NULL;
    ADM_ECS_SA_MASK_LIST_T *pEcsSaMaskList = NULL;
    DCHAR* pMask = NULL;

    if (ADM_IS_NULL(ppSaMaskList) || ADM_IS_NULL(*ppSaMaskList))
    {
        log_e("Invalid param");
        return;
    }

    pSaMaskList = *ppSaMaskList;

    pEcsSaMaskList = (ADM_ECS_SA_MASK_LIST_T*)adm_malloc(sizeof(ADM_ECS_SA_MASK_LIST_T));

    iRet = adm_ecs_getEcuSaMaskList(pRequestId, pSwDid, &pEcsSaMaskList);
    if (ADM_IS_ERR(iRet) || ADM_IS_NULL(pEcsSaMaskList->saMaskInfo))
    {
        log_e("Get sa mask list from ecs config failed.[%d]", iRet);
        
        iRet = adm_ecs_getEcuSaMask(pRequestId, pSwDid, &pMask);
        if (ADM_IS_ERR(iRet) || ADM_IS_NULL(pMask))
        {
            pSaMaskList->num = 0;
            log_e("Get sa mask from ecs config failed.[%d]", iRet);
        }
        else 
        {
            pSaMaskList->num = 1;
            pSaMaskList->pSaData = (ADM_DIM_SOA_SA_DATA_T*)adm_malloc(pSaMaskList->num * sizeof(ADM_DIM_SOA_SA_DATA_T));
            pSaMaskList->pSaData[0].level = ADM_IPM_DEFAULT_MASK_LEVEL;
            pSaMaskList->pSaData[0].pMask = pMask;
        }
    }
    else 
    {
        pSaMaskList->num = pEcsSaMaskList->num;
        pSaMaskList->pSaData = (ADM_DIM_SOA_SA_DATA_T*)adm_malloc(pSaMaskList->num * sizeof(ADM_DIM_SOA_SA_DATA_T));

        for (D16U i = 0; i < pEcsSaMaskList->num; i++)
        {
            pSaMaskList->pSaData[i].level  = (D8U)pEcsSaMaskList->saMaskInfo[i].saMaskLevel;
            pSaMaskList->pSaData[i].pMask = adm_strdup(pEcsSaMaskList->saMaskInfo[i].saMaskValue);
        }
    }

    adm_ecs_delSaMaskList(&pEcsSaMaskList);

    return;
}

DVOID adm_ipm_deInitSaMask(ADM_DIM_SOA_SA_MASK_INFO_T **ppSaMaskList)
{
    ADM_DIM_SOA_SA_MASK_INFO_T* pSaMaskList = NULL;

    if (ADM_IS_NULL(ppSaMaskList) || ADM_IS_NULL(*ppSaMaskList))
    {
        log_e("Invalid param");
        return;
    }

    pSaMaskList = *ppSaMaskList;

    for (D8U i = 0 ; i < pSaMaskList->num; i++)
    {
        adm_free(pSaMaskList->pSaData[i].pMask);
    }

    adm_free(pSaMaskList->pSaData);

    return;
}

DVOID adm_ipm_updateFlashAbInfo(DBOOL bIsAb, const DCHAR* pRequestId, const DCHAR* pSwDid, DBOOL bIsBlockSwitch,
                        ADM_DIM_SOA_VDI_ECU_AREA_INFO_E eDestAbInfo)
{
    if (ADM_IS_NULL(pRequestId) || ADM_IS_NULL(pSwDid))
    {
        log_e("Invalid param.");
        return;
    }

    ADM_IPS_ECU_PROGRESS_T progress = {0};
    adm_ips_ecuProgressInit(&progress);
    (void)adm_ips_ecuProgressGet(&progress, pRequestId, pSwDid);

    progress.bIsAb = bIsAb;
    progress.bBlockSwitchAb = bIsBlockSwitch;
    progress.eDestAbInfo = eDestAbInfo;

    adm_ips_progressAbInfoSet(&progress);

    adm_ips_ecuProgressDeInit(&progress);

    return;
}

DVOID adm_ipm_updateFlashAbSwitch(const DCHAR* pRequestId, const DCHAR* pSwDid, DBOOL bIsHashSwitch)
{
    if (ADM_IS_NULL(pRequestId) || ADM_IS_NULL(pSwDid))
    {
        log_e("Invalid param.");
        return;
    }

    ADM_IPS_ECU_PROGRESS_T progress = {0};
    adm_ips_ecuProgressInit(&progress);
    (void)adm_ips_ecuProgressGet(&progress, pRequestId, pSwDid);

    progress.bIsFLashHasSwitchAb = bIsHashSwitch;

    adm_ips_progressAbInfoSet(&progress);

    adm_ips_ecuProgressDeInit(&progress);

    return;
}

DVOID adm_ipm_updateRollbackAbSwitch(const DCHAR* pRequestId, const DCHAR* pSwDid, DBOOL bIsHashSwitch)
{
    if (ADM_IS_NULL(pRequestId) || ADM_IS_NULL(pSwDid))
    {
        log_e("Invalid param.");
        return;
    }

    ADM_IPS_ECU_PROGRESS_T progress = {0};
    adm_ips_ecuProgressInit(&progress);
    (void)adm_ips_ecuProgressGet(&progress, pRequestId, pSwDid);

    progress.bIsRollbackHasSwitchAB = bIsHashSwitch;
    
    adm_ips_progressAbInfoSet(&progress);

    adm_ips_ecuProgressDeInit(&progress);

    return;
}

DVOID adm_ipm_updFLashAbSwitchFailed(const DCHAR* pRequestId, const DCHAR* pSwDid, D32S iResult)
{
    if (ADM_IS_NULL(pRequestId) || ADM_IS_NULL(pSwDid))
    {
        log_e("Invalid param.");
        return;
    }

    ADM_IPS_ECU_PROGRESS_T progress = {0};
    adm_ips_ecuProgressInit(&progress);
    (void)adm_ips_ecuProgressGet(&progress, pRequestId, pSwDid);

    progress.progress[ADM_IPS_ECU_PHASE_INDEX_UPGRADE].result = iResult;
    
    adm_ips_progressAbInfoSet(&progress);

    adm_ips_ecuProgressDeInit(&progress);

    return;
}

DVOID adm_ipm_updRollbackAbSwitchFailed(const DCHAR* pRequestId, const DCHAR* pSwDid, D32S iResult, D32S iUpgradeNumbers, D64S iEndTime)
{
    if (ADM_IS_NULL(pRequestId) || ADM_IS_NULL(pSwDid))
    {
        log_e("Invalid param.");
        return;
    }

    ADM_IPS_ECU_PROGRESS_T progress = {0};
    adm_ips_ecuProgressInit(&progress);
    (void)adm_ips_ecuProgressGet(&progress, pRequestId, pSwDid);

    progress.progress[ADM_IPS_ECU_PHASE_INDEX_UPGRADE].result = iResult;
    progress.progress[ADM_IPS_ECU_PHASE_INDEX_ROLLBACK].upgradeNumbers = iUpgradeNumbers;
    progress.progress[ADM_IPS_ECU_PHASE_INDEX_ROLLBACK].endTime = iEndTime;
    
    adm_ips_progressAbInfoSet(&progress);

    adm_ips_ecuProgressDeInit(&progress);

    return;
}

DVOID adm_ipm_getFlashAbInfo(const DCHAR* pRequestId, const DCHAR* pSwDid, 
            DBOOL* pIsAbInfo, DBOOL* pIsBlockSwitchAb,
            ADM_DIM_SOA_VDI_ECU_AREA_INFO_E* pDestAbInfo)
{
    if (ADM_IS_NULL(pIsAbInfo) || ADM_IS_NULL(pDestAbInfo))
    {
        log_e("Invalid param.");
        return;
    }
    D32S iRet = ADM_ECODE_FAILED;

    ADM_IPS_ECU_PROGRESS_T stProgress = {0};
    adm_ips_ecuProgressInit(&stProgress);
    iRet = adm_ips_ecuProgressGet(&stProgress, pRequestId, pSwDid);
    if (ADM_IS_SUCCESS(iRet))
    {
        *pIsAbInfo = stProgress.bIsAb;
        *pIsBlockSwitchAb = stProgress.bBlockSwitchAb;
        *pDestAbInfo = stProgress.eDestAbInfo;
    }

    adm_ips_ecuProgressDeInit(&stProgress);

    return;
}

DVOID adm_ipm_getIsRollback(const DCHAR *pEcuReqId, const DCHAR *pEcuSwDid, 
        DBOOL *pIsRollback)
{
    
    if (ADM_IS_NULL(pEcuReqId) || ADM_IS_NULL(pEcuSwDid) || 
        ADM_IS_NULL(pIsRollback))    
    {
        log_e("Invalid param.");
        return;
    }

    ADM_IPS_PROGRESS_T stProgressInfo = {0};
    ADM_IPS_ECU_PROGRESS_T stEcuprogress = {0};

    adm_ips_progressInit(&stProgressInfo);
    adm_ips_ecuProgressInit(&stEcuprogress);

    adm_ips_progressGet(&stProgressInfo);
    (void)adm_ips_ecuProgressGet(&stEcuprogress, pEcuReqId, pEcuSwDid);

    if(stProgressInfo.ecus.ecus != NULL)
    {
        for (D32S i = 0; i < stProgressInfo.ecus.ecusLen; ++i) 
        {
            ADM_IPS_ECU_PROGRESS_T* pstEcu = stProgressInfo.ecus.ecus + i;
            if (adm_strcasecmp(pEcuReqId, pstEcu->ecuReqId) == 0 && adm_strcasecmp(pEcuSwDid, pstEcu->ecuSwDid) == 0) 
            {
                if (ADM_IPS_ECU_PHASE_TIMELY_PRE_INSTALL_ROLLBACK == pstEcu->state || 
                ADM_IPS_ECU_PHASE_PRE_INSTALL_ROLLBACK ==  pstEcu->state || 
                ADM_IPS_ECU_PHASE_INSTALL_ROLLBACK == pstEcu->state || 
                ADM_IPS_ECU_PHASE_INSTALL_ROLLBACK_RETRY == pstEcu->state || 
                ADM_IPS_ECU_PHASE_POST_INSTALL_ROLLBACK == pstEcu->state)
                {
                    *pIsRollback = DTRUE;
                    log_i("Ecu RequestId[%s] swDid[%s] is doing rollback state[%d].", pEcuReqId, pEcuSwDid, pstEcu->state);
                }
                break;
            }
        }
    }

    adm_ips_progressDeInit(&stProgressInfo);
    adm_ips_ecuProgressDeInit(&stEcuprogress);

    return;
}

D32S adm_ipm_getBussinessScipt(ADM_IPM_BUSSINESS_SCRIPT_TYPE_E eBussType, DCHAR** pScriptPath)
{
    D32S iRet = ADM_ECODE_FAILED;
    ADM_ECS_RSCONFIGINFO_T *pstRsCfgData = NULL;
    ADM_ECS_GENERALCONFIG_T *pstGeneralCfg = NULL;

    do
    {
        if (ADM_IS_NULL(pScriptPath))
        {
            log_e("Invalid param.");
            iRet = ADM_ECODE_INVALID_PARAM;
            break;
        }

        log_i("Find bussiness script num[%d].", eBussType);
        pstRsCfgData = adm_ecs_newRSConfigInfo();

        if (ADM_IS_NULL(pstRsCfgData))
        {
            iRet = ADM_ECODE_NO_MEMORY;
            log_e("Failed to new ota config data info, err[%d]", iRet);
            break;
        }

        iRet = adm_ecs_getConfigInfo(pstRsCfgData);
        if (iRet != ADM_ECODE_SUCCESSED)
        {
            log_e("Failed get config info, nRet[%d]", iRet);
            adm_ecs_delRSConfigInfo(&pstRsCfgData);
            break;
        }

        pstGeneralCfg = &(pstRsCfgData->stGeneralCfg);
        if (0 < pstGeneralCfg->stBusinessScriptList.num && NULL != pstGeneralCfg->stBusinessScriptList.pstBusinessScriptInfo)
        {
            for (D32S i = 0; i < pstGeneralCfg->stBusinessScriptList.num; i++)
            {
                log_i("compare bussiness script num[%d].", pstGeneralCfg->stBusinessScriptList.pstBusinessScriptInfo[i].businessScriptNumber);
                if (eBussType == pstGeneralCfg->stBusinessScriptList.pstBusinessScriptInfo[i].businessScriptNumber)
                {
                    *pScriptPath = adm_strdup(pstGeneralCfg->stBusinessScriptList.pstBusinessScriptInfo[i].stBusinessScriptFile.filePath);
                }
            }
        }
        else 
        {
            log_e("Bussiness scriptList is null.");
            iRet = ADM_ECODE_ECS_DATA_NOT_FIND;
            break;
        }
    } while (FALSE);

    if (NULL != pstRsCfgData)
    {
        adm_ecs_delRSConfigInfo(&pstRsCfgData);
    }

    return iRet;
}

D32S adm_ipm_getFlashScipt(ADM_IPM_FLASH_SCRIPT_TYPE_E eFlashType, DCHAR** pScriptPath)
{
    D32S iRet = ADM_ECODE_FAILED;
    ADM_ECS_RSCONFIGINFO_T *pstRsCfgData = NULL;
    ADM_ECS_GENERALCONFIG_T *pstGeneralCfg = NULL;

    do
    {
        if (ADM_IS_NULL(pScriptPath))
        {
            log_e("Invalid param.");
            iRet = ADM_ECODE_INVALID_PARAM;
            break;
        }

        log_i("Find flash script num[%d].", eFlashType);
        pstRsCfgData = adm_ecs_newRSConfigInfo();

        if (ADM_IS_NULL(pstRsCfgData))
        {
            iRet = ADM_ECODE_NO_MEMORY;
            log_e("Failed to new ota config data info, err[%d]", iRet);
            break;
        }

        iRet = adm_ecs_getConfigInfo(pstRsCfgData);
        if (iRet != ADM_ECODE_SUCCESSED)
        {
            log_e("Failed get config info, nRet[%d]", iRet);
            adm_ecs_delRSConfigInfo(&pstRsCfgData);
            break;
        }

        pstGeneralCfg = &(pstRsCfgData->stGeneralCfg);
        if (0 < pstGeneralCfg->stBusinessScriptList.num && NULL != pstGeneralCfg->stBusinessScriptList.pstBusinessScriptInfo)
        {
            for (D32S i = 0; i < pstGeneralCfg->stflashScriptList.num; i++)
            {
                log_i("compare bussiness script num[%d].", pstGeneralCfg->stflashScriptList.pstFlashScriptInfo[i].flashScriptNumber);
                if (eFlashType == pstGeneralCfg->stflashScriptList.pstFlashScriptInfo[i].flashScriptNumber)
                {
                    *pScriptPath = adm_strdup(pstGeneralCfg->stflashScriptList.pstFlashScriptInfo[i].stFlashScriptFile.filePath);
                }
            }
        }
        else 
        {
            log_e("Flash scriptList is null.");
            iRet = ADM_ECODE_ECS_DATA_NOT_FIND;
            break;
        }
    } while (FALSE);

    if (NULL != pstRsCfgData)
    {
        adm_ecs_delRSConfigInfo(&pstRsCfgData);
    }

    return iRet;
}

D32S adm_ipm_switchAb(const DCHAR* pRequestId, const DCHAR* pSwDid,
        const D16U uiTimeOut,
        const ADM_DIM_SOA_VDI_ECU_AREA_INFO_E eDstAreaInfo)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    D32U uiErrorCode = ADM_ECODE_SUCCESSED;
    DCHAR* pSwitchScriptPath = NULL;
    DCHAR* pstInstallInstanceId = NULL;
    ADM_DIM_SOA_SA_MASK_INFO_T* pstSaMaskList = NULL;
    ADM_DIM_SOA_ECU_IDEN_INFO_T* pstEcuIdenInfo = NULL;
    DCHAR* pResponeseId = NULL;
    DCHAR* pFunctionId = NULL;
    D32S iBusType = -1;
    D32S iChannelType = -1;

    do
    {
        iRet = adm_ipm_getBussinessScipt(ADM_IPM_SCRIPT_SWITCH_AB, &pSwitchScriptPath);
        if (ADM_IS_ERR(iRet) || ADM_IS_NULL(pSwitchScriptPath))
        {
            log_e("Can not find switch Ab script.");
            iRet = ADM_ECODE_ECS_DATA_NOT_FIND;
            break;
        }

        iRet = adm_ecs_getEcuInfo(pRequestId, pSwDid, &pResponeseId, &pFunctionId);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Can not find ecu info.");
            iRet = ADM_ECODE_ECS_DATA_NOT_FIND;
            break;
        }

        pstEcuIdenInfo = (ADM_DIM_SOA_ECU_IDEN_INFO_T*)adm_malloc(sizeof(ADM_DIM_SOA_ECU_IDEN_INFO_T));
        pstEcuIdenInfo->requestId = ipmHtoi(pRequestId);
        pstEcuIdenInfo->swDid = ipmHtoi(pSwDid);
        pstEcuIdenInfo->responseId = ipmHtoi(pResponeseId);
        pstEcuIdenInfo->functionId = ipmHtoi(pFunctionId);

        pstInstallInstanceId = adm_ecs_getInstallInstanceId(pRequestId, pSwDid);
        if (ADM_IS_NULL(pstInstallInstanceId))
        {
            log_e("Can not find install instanceId.");
            iRet = ADM_ECODE_ECS_DATA_NOT_FIND;
            break;
        }

        iRet = adm_ecs_getBusType(pRequestId, pSwDid, &iBusType);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Can not find busType.");
            iRet = ADM_ECODE_ECS_DATA_NOT_FIND;
            break;
        }

        iRet = adm_ecs_getChannelType(pRequestId, pSwDid, &iChannelType);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Can not find busType.");
            iRet = ADM_ECODE_ECS_DATA_NOT_FIND;
            break;
        }

        pstSaMaskList = (ADM_DIM_SOA_SA_MASK_INFO_T*)adm_malloc(sizeof(ADM_DIM_SOA_SA_MASK_INFO_T));
        adm_ipm_initSaMask(pRequestId, pSwDid, &pstSaMaskList);

        log_i("[ABUP-Procedure]: Install Start Vdi switch AB info.");
        iRet = adm_dim_vdi_switchAb((D16U)ipmHtoi(pstInstallInstanceId), 
                                    uiTimeOut, 
                                    pstEcuIdenInfo, 
                                    pSwitchScriptPath, 
                                    (ADM_DIM_SOA_VDI_BUS_TYPE_E)iBusType,
                                    (ADM_DIM_SOA_PIN_CHANNEL_TYPE_E)iChannelType,
                                    eDstAreaInfo, 
                                    pstSaMaskList, 
                                    &uiErrorCode);
        log_i("Get ab info ret[%d], errorCode[%u]", iRet, uiErrorCode);

        iRet = (iRet == ADM_ECODE_SUCCESSED) ? uiErrorCode : iRet;
    } while (FALSE);

    adm_free(pSwitchScriptPath);
    adm_free(pstInstallInstanceId);
    adm_free(pResponeseId);
    adm_free(pFunctionId);
    adm_ipm_deInitSaMask(&pstSaMaskList);
    adm_free(pstSaMaskList);

    return iRet;
}

DVOID adm_ipm_notifyFlashState(ADM_DIM_SOA_VDI_FLASH_STATE_E eFlashState, D32U uiTimeOut)
{
    D32S iRet = ADM_ECODE_FAILED;
    D32S iErrorCode = ADM_ECODE_FAILED;

    for (D32S i = 0; i < 3; i++)
    {
        iRet = adm_dim_vdi_notifyFlashState(adm_soa_getVdiInstanceId(), uiTimeOut, eFlashState, &iErrorCode);
        if (ADM_IS_ERR(iRet) || ADM_IS_ERR(iErrorCode))
        {
            log_e("notify flash state error.iRet[%d] errorCode[%d].", iRet, iErrorCode);

        }
        else 
        {
            break;
        }
        sleep(5);
    }

    return;
}