#include "adm_cvm_uim.h"
#include "adm_cvm_log.h"

#include "adm_cvm_checkVersion.h"
#include "adm_com_modDefine.h"
#include "adm_com_errorCode.h"
#include "adm_cvs_versionPolicy.h"
#include "adm_cvs_common.h"
#include "adm_application_common.h"
#include "adm_vsm_fsm.h"
#include "adm_eim_getEcuInfo.h"
#include "adm_uim_request.h"

#include "adm_string.h"
#include "adm_memory.h"
#include <time.h>
#include <pthread.h>

static D32S adm_cvm_ucmToHmiCheckErrorCode(ADM_APPLICATION_COMMON_CMDID_TYPE_E cmdId, D32S upgradeMode, D32S errorCode)
{
    D32S code = ADM_ECODE_FAILED;

    if (ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_USER == cmdId || 
        ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_USB == cmdId) 
    {
        log_i("user click trigger check");
        if (ADM_ECODE_SUCCESSED == errorCode && upgradeMode == (D32S)ADM_CVS_UPGRADE_MODE_SLIENCE_DOWNLOAD) 
        {
            log_i("check success && silent download");
            code = ADM_ECODE_CHECK_LATEST_VERSION;
        } 
        else if (ADM_ECODE_SUCCESSED == errorCode) 
        {
            log_i("check success");
            code = ADM_ECODE_CHECK_LATEST_VERSION_IMMEDIATELY_UPGRADED;
        } 
        else if (ADM_OCS_AIR_STATUS_CODE_VEHICLE_NOT_EXIST == errorCode
            || ADM_OCS_AIR_STATUS_CODE_CONFIG_NEED_UPDATE == errorCode) 
        {
            log_i("check failed && cloud return exception");
            code = ADM_ECODE_CHECK_LATEST_VERSION;
        }
        else if (ADM_ECODE_CHECK_HTTP_POST_FAILED == errorCode) 
        {
            log_i("check failed && network exception");
            code = ADM_ECODE_CHECK_HTTP_POST_FAILED;
        } 
        else if (errorCode >= ADM_ECODE_CURL_INIT_FAILED && errorCode <= ADM_ECODE_PROTOCOL_TYPE_INVALID)
        {
            log_i("check failed && network error");
            code = errorCode;
        }
        else 
        {
            log_i("check failed && general error");
            code = ADM_ECODE_CHECK_LATEST_VERSION;
        }
    } 
    else 
    {
        if (ADM_ECODE_SUCCESSED == errorCode && upgradeMode != (D32S)ADM_CVS_UPGRADE_MODE_SLIENCE_DOWNLOAD) 
        {
            log_i("Non-user click trigger check && not-silent download");
            code = ADM_ECODE_CHECK_LATEST_VERSION_IMMEDIATELY_UPGRADED;
        } 
        else if (errorCode >= ADM_ECODE_CURL_INIT_FAILED && errorCode <= ADM_ECODE_PROTOCOL_TYPE_INVALID)
        {
            log_i("check failed && network error");
            code = errorCode;
        }
        else 
        {
            log_i("Non-user click trigger check && general case");
            code = ADM_ECODE_CHECK_LATEST_VERSION;
        }
    }
    return code;
}


/**
 * @brief 获取升级预计的总耗时
 * 
 * @param[in/out] pTotalTimeConsuming     描述: \n
 * 范围值: \n
 * 单位: \n
 * 
 * @return D32S 
 */
static D32S adm_cvm_ucmGetTotalTimeConsuming(D32S* pTotalTimeConsuming)
{
    D32S iRet = ADM_ECODE_FAILED;
    ADM_CVS_VERSION_POLICY_T* pstVersionPolicy = adm_cvs_newPolicyBody();
    struct json_object *root = DNULL;

    if (DNULL == pstVersionPolicy)
    {
        iRet = ADM_ECODE_APPLY_MEMORY_FAILED;
        log_e("adm_cvs_newPolicyBody failed, [err:%d]", iRet);
        return iRet;
    }

    do
    {
        iRet = adm_cvs_getVersionPolicy(pstVersionPolicy);
        if (ADM_ECODE_SUCCESSED != iRet)
        {
            log_e("adm_cvs_getVersionPolicy failed, [err:%d]", iRet);
            break;
        }

        if (DNULL == pstVersionPolicy->installMember.installPolicyObjectStr)
        {
            iRet = ADM_ECODE_INVALID_POINTER;
            log_e("versionPolicy installMember installPolicyObjectStr:nullptr, [err:%d]", iRet);
            break;
        }

        root = json_tokener_parse(pstVersionPolicy->installMember.installPolicyObjectStr);
        if (DNULL == root)
        {
            iRet = ADM_ECODE_INVALID_POINTER;
            log_w("path:%s to json failed, [err:%d]", pstVersionPolicy->installMember.installPolicyObjectStr, iRet);
            break;
        }


        *pTotalTimeConsuming = json_object_get_int(json_object_object_get(root, "totalTimeConsuming"));
        iRet = ADM_ECODE_SUCCESSED;

    } while (FALSE);
    
    if (DNULL != root)
    {
        json_object_put(root);
    }

    if (DNULL != pstVersionPolicy)
    {
        adm_cvs_deletePolicyBody(pstVersionPolicy);
    }

    return iRet;
}


static D32S __adm_cvm_newCheckHmiPoliyInfo(ADM_UIM_SOA_CHECK_HMI_POLICY_INFO_T** pppstPolicyInfo, const ADM_OCS_CHECK_RESP_ADAPTER_T* resp)
{
    ADM_UIM_SOA_CHECK_HMI_POLICY_INFO_T* ppstPolicyInfo = NULL;

    if((NULL == pppstPolicyInfo) || NULL == resp)
    {
        return ADM_ECODE_INVALID_PARAM;
    }

    ppstPolicyInfo = (ADM_UIM_SOA_CHECK_HMI_POLICY_INFO_T*)adm_malloc(sizeof(ADM_UIM_SOA_CHECK_HMI_POLICY_INFO_T));
    if(NULL == ppstPolicyInfo)
    {
        log_e("PpstPolicyInfo Failed to apply for memory");
        return ADM_ECODE_NO_MEMORY;
    }
    ppstPolicyInfo->hmiFile = (ADM_UIM_HMI_FILE_T*)adm_malloc(sizeof(ADM_UIM_HMI_FILE_T));
    if(NULL == ppstPolicyInfo->hmiFile)
    {
        log_e("ppstPolicyInfo->hmiFile Failed to apply for memory");
        return ADM_ECODE_NO_MEMORY;
    }
    ppstPolicyInfo->hmiFile->policyFileUrl = adm_strdup(resp->hmiMember->ucmHmiPolicyFileUrl);
    ppstPolicyInfo->hmiFile->policyFileHash = adm_strdup(resp->hmiMember->ucmHmiPolicyFileHash);
    ppstPolicyInfo->hmiFile->policyFileSize = (D64U)resp->hmiMember->ucmHmiPolicyFileSize;
    log_i("hmiFileUrl = %s", ppstPolicyInfo->hmiFile->policyFileUrl);
    log_i("hmiFileHash = %s", ppstPolicyInfo->hmiFile->policyFileHash);
    log_i("hmiFileSize = %llu", ppstPolicyInfo->hmiFile->policyFileSize);

    ppstPolicyInfo->upgradeMode = (ADM_UIM_UPGRADE_MODE_E)resp->upgradeMode;
    adm_cvm_ucmGetTotalTimeConsuming(&(ppstPolicyInfo->iTotalTimeS));
    log_i("upgradeMode = %d, iTotalTimeConsuming = %d [s]", ppstPolicyInfo->upgradeMode, ppstPolicyInfo->iTotalTimeS);

    ppstPolicyInfo->hmiInfo = (ADM_UIM_HMI_INFO*)adm_malloc(sizeof(ADM_UIM_HMI_INFO));
    if(NULL == ppstPolicyInfo->hmiInfo)
    {
        log_e("ppstPolicyInfo->hmiInfo Failed to apply for memory");
        return ADM_ECODE_NO_MEMORY;
    }
    ppstPolicyInfo->hmiInfo->ecuInfoList = (ADM_UIM_ECU_INFO_ARRAY_T*)adm_malloc(sizeof(ADM_UIM_ECU_INFO_ARRAY_T));
    if(NULL == ppstPolicyInfo->hmiInfo->ecuInfoList)
    {
        log_e("ppstPolicyInfo->hmiInfo->ecuInfoList Failed to apply for memory");
        return ADM_ECODE_NO_MEMORY;
    }
    ppstPolicyInfo->hmiInfo->ecuInfoList->hmiEcuInfoArrayLength = resp->hmiMember->hmiEcuInfoArrayLength;
    /*option*/
    if (NULL != resp->hmiMember->vehicleVersion) {
        ppstPolicyInfo->hmiInfo->vehicleVersion = adm_strdup(resp->hmiMember->vehicleVersion);
    }
    ppstPolicyInfo->hmiInfo->totalSize = (D64U)resp->hmiMember->totalSize;

    ppstPolicyInfo->hmiInfo->ecuInfoList->hmiEcuInfoArray = (ADM_UIM_ECU_INFO_T**)adm_malloc(
        sizeof(ADM_UIM_ECU_INFO_T*) * ppstPolicyInfo->hmiInfo->ecuInfoList->hmiEcuInfoArrayLength);
    if(NULL == ppstPolicyInfo->hmiInfo->ecuInfoList->hmiEcuInfoArray)
    {
        log_e("ppstPolicyInfo->hmiInfo->ecuInfoList->hmiEcuInfoArray Failed to apply for memory");
        return ADM_ECODE_NO_MEMORY;
    }
    log_i("vehicleVersion = %s", ppstPolicyInfo->hmiInfo->vehicleVersion);
    log_i("totalSize = %d", ppstPolicyInfo->hmiInfo->totalSize);
    log_i("hmiEcuInfoArrayLength = %d", ppstPolicyInfo->hmiInfo->ecuInfoList->hmiEcuInfoArrayLength);

    for (D16U i = 0; i < ppstPolicyInfo->hmiInfo->ecuInfoList->hmiEcuInfoArrayLength; ++i) {

        ppstPolicyInfo->hmiInfo->ecuInfoList->hmiEcuInfoArray[i]
            = (ADM_UIM_ECU_INFO_T*)adm_malloc(sizeof(ADM_UIM_ECU_INFO_T));
        if(NULL == ppstPolicyInfo->hmiInfo->ecuInfoList->hmiEcuInfoArray[i])
        {
            log_e("ppstPolicyInfo->hmiInfo->ecuInfoList->hmiEcuInfoArray[i] Failed to apply for memory");
            return ADM_ECODE_NO_MEMORY;
        }
        log_i("ecuSize[%d]  = %d", i, resp->hmiMember->hmiEcuInfoArray[i]->ecuSize);
        ppstPolicyInfo->hmiInfo->ecuInfoList->hmiEcuInfoArray[i]->ecuSize
            = (D64U)resp->hmiMember->hmiEcuInfoArray[i]->ecuSize;

        log_i("ecuVersion[%d]  = %s", i, resp->hmiMember->hmiEcuInfoArray[i]->ecuVersion);
        ppstPolicyInfo->hmiInfo->ecuInfoList->hmiEcuInfoArray[i]->ecuVersion
            = adm_strdup(resp->hmiMember->hmiEcuInfoArray[i]->ecuVersion);

        log_i("requestId[%d]  = %s", i, resp->hmiMember->hmiEcuInfoArray[i]->requestId);
        ppstPolicyInfo->hmiInfo->ecuInfoList->hmiEcuInfoArray[i]->requestId
            = adm_strdup(resp->hmiMember->hmiEcuInfoArray[i]->requestId);

        log_i("swDid[%d]  = %s", i, resp->hmiMember->hmiEcuInfoArray[i]->swDid);
        ppstPolicyInfo->hmiInfo->ecuInfoList->hmiEcuInfoArray[i]->swDid
            = adm_strdup(resp->hmiMember->hmiEcuInfoArray[i]->swDid);
    }
    *pppstPolicyInfo = ppstPolicyInfo;
    return ADM_ECODE_SUCCESSED;
}

static void __adm_cvm_delCheckHmiPoliyInfo(ADM_UIM_SOA_CHECK_HMI_POLICY_INFO_T** pppstPolicyInfo)
{
    ADM_UIM_SOA_CHECK_HMI_POLICY_INFO_T* ppstPolicyInfo = *pppstPolicyInfo;

    if (NULL != ppstPolicyInfo) {
        if(NULL != ppstPolicyInfo->hmiFile){
            adm_free(ppstPolicyInfo->hmiFile->policyFileUrl);
            adm_free(ppstPolicyInfo->hmiFile->policyFileHash);
            adm_free(ppstPolicyInfo->hmiFile);
        }
        if(NULL != ppstPolicyInfo->hmiInfo) {
            adm_free(ppstPolicyInfo->hmiInfo->vehicleVersion);
            if(NULL != ppstPolicyInfo->hmiInfo->ecuInfoList) {
                if(NULL != ppstPolicyInfo->hmiInfo->ecuInfoList->hmiEcuInfoArray) {
                    for (D16U i = 0; i < ppstPolicyInfo->hmiInfo->ecuInfoList->hmiEcuInfoArrayLength; ++i) {
                        adm_free(ppstPolicyInfo->hmiInfo->ecuInfoList->hmiEcuInfoArray[i]->ecuVersion);
                        adm_free(ppstPolicyInfo->hmiInfo->ecuInfoList->hmiEcuInfoArray[i]->requestId);
                        adm_free(ppstPolicyInfo->hmiInfo->ecuInfoList->hmiEcuInfoArray[i]->swDid);
                        adm_free(ppstPolicyInfo->hmiInfo->ecuInfoList->hmiEcuInfoArray[i]);
                    }
                    adm_free(ppstPolicyInfo->hmiInfo->ecuInfoList->hmiEcuInfoArray);
                }
                adm_free(ppstPolicyInfo->hmiInfo->ecuInfoList);
            }
            adm_free(ppstPolicyInfo->hmiInfo);   
        }
        adm_free(ppstPolicyInfo);
    }
}

static void cvmUimCheckProgressNotify(const ADM_CVS_COMMON_CMDID_TYPE_E eCmdidType, 
                                    const DCHAR* state, 
                                    const ADM_CVS_COMMON_CHECK_PROGRESS_TYPE_T eCheckProgressType)
{
    adm_uim_checkProgressNotify(eCmdidType, state, eCheckProgressType);
    return;
}


D32S adm_cvm_check(ADM_APPLICATION_COMMON_CMDID_TYPE_E cmdId)
{
    log_i("check start");

    ADM_OCS_CHECK_RESP_ADAPTER_T* resp = NULL;
    D32S retVal = ADM_ECODE_FAILED;
    D32S retValbak = ADM_ECODE_SUCCESSED;
    D32S respCode = 0;
    DCHAR* sessionId = NULL;
    DCHAR* state = NULL;
    ADM_UIM_SOA_CHECK_HMI_POLICY_INFO_T* ppstPolicyInfo = NULL;
    D32S errorCode;
    DBOOL berrDeal = FALSE; //比变量用于，当内存分配失败时，退出的标识。
    ADM_APPLICATION_COMMON_CMDID_TYPE_E eCmdId = ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_USER;

    do{
        retVal = adm_cvm_checkVersionAdaptorNotify(cmdId, cvmUimCheckProgressNotify, NULL, &resp);
        retValbak = retVal;
        if (ADM_ECODE_SUCCESSED == retVal || ADM_ECODE_CHECK_LATEST_VERSION == retVal) {
            log_i("check success");

            errorCode = adm_cvm_ucmToHmiCheckErrorCode(cmdId, resp->upgradeMode, retVal);
            retVal = __adm_cvm_newCheckHmiPoliyInfo(&ppstPolicyInfo, (const ADM_OCS_CHECK_RESP_ADAPTER_T*)resp);
            if(ADM_ECODE_SUCCESSED != retVal) {
                log_e("ppstPolicyInfo ADM_ECODE_NO_MEMORY");
                break;
            }

            sessionId = adm_strdup(resp->sessionId);

            log_i("UIM notify check result.eCmdId:%d",resp->upgradeMode);
            eCmdId = (ADM_APPLICATION_COMMON_CMDID_TYPE_E)resp->upgradeMode;

            state = adm_strdup("check_success");
        } else {
            errorCode = adm_cvm_ucmToHmiCheckErrorCode(cmdId, (D32S)ADM_CVS_UPGRADE_MODE_USER_CLICK, retVal);
            state = adm_strdup("check_failed");
        }

        adm_uim_hmi_checkResult_RR(eCmdId, sessionId, state, ppstPolicyInfo, errorCode, &respCode);
        log_i("UIM notify check result.recv respCode[%d]", respCode);
        adm_ocs_deleteCheckRespAdapterBody(resp);
    }while(FALSE);

    /** 上电检测失败 需要开启后台零件信息收集，保证零件完整 */
    if (ADM_IS_ERR(retValbak) && ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_IGN == cmdId)
    {
        pthread_t threadId = 0;
        if (0 == pthread_create(&threadId, NULL, adm_cvm_triggerGetEcuInfo, NULL)) 
        {
            log_i("pthread_create adm_cvm_triggerGetEcuInfo successed");
            (void)pthread_detach(threadId);
        } 
    }

    adm_free(sessionId);
    adm_free(state);
    __adm_cvm_delCheckHmiPoliyInfo(&ppstPolicyInfo);
    log_i("check end");
    return retVal;
}

/**
 * @brief usb升级模式触发检测(区别于普通的检测接口，新加此接口，避免编码杂糅的情况，不易理解和维护)
 * @details 与正常检测区别：
 *              1、此模式先校验车辆合法性
 *              2、此模式无需向云端请求，直接从U盘读取策略信息
 * 
 * @param[in/out] cmdId     描述: 触发方式 \n
 * 范围值: 枚举 \n
 * 单位: N/A \n
 * 
 * @return D32S 
 */
D32S adm_cvm_usbCheck(ADM_APPLICATION_COMMON_CMDID_TYPE_E cmdId)
{
    log_i("usb check start");

    ADM_OCS_CHECK_RESP_ADAPTER_T* resp = NULL;
    D32S retVal = ADM_ECODE_FAILED;
    D32S respCode = 0;
    DCHAR* sessionId = NULL;
    DCHAR* state = NULL;
    ADM_UIM_SOA_CHECK_HMI_POLICY_INFO_T* ppstPolicyInfo = NULL;
    D32S errorCode = ADM_ECODE_FAILED;

    do{
        
        retVal = adm_cvm_usbCheckVersion(cmdId, cvmUimCheckProgressNotify, NULL, &resp);
        if (ADM_ECODE_SUCCESSED == retVal || ADM_ECODE_CHECK_LATEST_VERSION == retVal) {
            log_i("check success");

            errorCode = adm_cvm_ucmToHmiCheckErrorCode(cmdId, resp->upgradeMode, retVal);
            retVal = __adm_cvm_newCheckHmiPoliyInfo(&ppstPolicyInfo, (const ADM_OCS_CHECK_RESP_ADAPTER_T*)resp);
            if(ADM_ECODE_SUCCESSED != retVal) {
                log_e("ppstPolicyInfo ADM_ECODE_NO_MEMORY");
                break;
            }

            sessionId = adm_strdup(resp->sessionId);
            state = adm_strdup("check_success");
        } else {
            errorCode = adm_cvm_ucmToHmiCheckErrorCode(cmdId, (D32S)ADM_CVS_UPGRADE_MODE_USER_CLICK, retVal);
            state = adm_strdup("check_failed");
        }

        log_i("UIM notify check result.");
        adm_uim_hmi_checkResult_RR(cmdId, sessionId, state, ppstPolicyInfo, errorCode, &respCode);
        log_i("UIM notify check result.recv respCode[%d]", respCode);
        adm_ocs_deleteCheckRespAdapterBody(resp);
    }while(FALSE);

    adm_free(sessionId);
    adm_free(state);
    __adm_cvm_delCheckHmiPoliyInfo(&ppstPolicyInfo);
    log_i("check end");
    return retVal;
}

void adm_cvm_checkTriggerCallback(ADM_UIM_SOA_ACTION_AUTH_TYPE_E* eActionType, void* priv)
{
    log_i("adm_cvm_checkTriggerCallback start");
    if(NULL == eActionType)
    {
        log_e("eActionType input param invalid");
        return ;
    }
    if (TRUE == adm_vsm_triggerCheck(ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_USER)) {
        *eActionType = ADM_UIM_SOA_ACTION_AUTH_TYPE_ALLOW;
    } else {
        *eActionType = ADM_UIM_SOA_ACTION_AUTH_TYPE_NOT_ALLOW;
    }
    log_i("adm_cvm_checkTriggerCallback end");
    return;
}

void adm_cvm_usbCheckTriggerCallback(const DCHAR* pUsbDir, ADM_UIM_SOA_ACTION_AUTH_TYPE_E* eActionType, void* priv)
{
    ADM_VSM_STATE_E state = ADM_VSM_STATE_IDLE;
    
    log_i("adm_cvm_usbCheckTriggerCallback start");
    if (NULL == eActionType)
    {
        log_e("eActionType input param invalid");
        return;
    }
    
    state = adm_vsm_stateGet();
    if (ADM_VSM_STATE_UPGRADE_DOING == state
       || ADM_VSM_STATE_CHECK_DOING == state
       || ADM_VSM_STATE_DOWNLOAD_DOING  == state)
    {
        log_e("vsm state is not idle, state = %d", state);
        *eActionType = ADM_UIM_SOA_ACTION_AUTH_TYPE_NOT_ALLOW;
        return;
    }
    else
    {
        log_d("current state = %d", state);
        adm_vsm_stateSet(ADM_VSM_STATE_IDLE);
    }
    
    (DVOID) adm_cvs_setUsbDir(pUsbDir);

    if (TRUE == adm_vsm_triggerCheck(ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_USB))
    {
        *eActionType = ADM_UIM_SOA_ACTION_AUTH_TYPE_ALLOW;
    }
    else
    {
        *eActionType = ADM_UIM_SOA_ACTION_AUTH_TYPE_NOT_ALLOW;
    }

    log_i("adm_cvm_usbCheckTriggerCallback end");

    return;
}

void adm_cvm_checkResultNotify(void)
{
    ADM_APPLICATION_COMMON_CMDID_TYPE_E eCmdidType = ADM_APPLICATION_COMMON_CMDID_TYPE_INVALID;
    DCHAR* sessionId = NULL;
    DCHAR* state = NULL;
    ADM_UIM_SOA_CHECK_HMI_POLICY_INFO_T* ppstPolicyInfo = NULL;
    D32S errorCode = ADM_ECODE_FAILED;
    void* priv = NULL;

    adm_cvm_queryResultCallback(&eCmdidType, &sessionId, &state, &ppstPolicyInfo, &errorCode, priv);
    if (ADM_ECODE_SUCCESSED == errorCode) {
        log_i("query check result.success");
        adm_uim_checkResultNotify(eCmdidType, sessionId, state, ppstPolicyInfo, errorCode);
        adm_free(sessionId);
        adm_free(state);
        if (NULL != ppstPolicyInfo) {
            adm_free(ppstPolicyInfo->hmiFile->policyFileUrl);
            adm_free(ppstPolicyInfo->hmiFile->policyFileHash);
            adm_free(ppstPolicyInfo->hmiFile);
            adm_free(ppstPolicyInfo->hmiInfo->vehicleVersion);
            for (D16U i = 0; i < ppstPolicyInfo->hmiInfo->ecuInfoList->hmiEcuInfoArrayLength; ++i) {
                adm_free(ppstPolicyInfo->hmiInfo->ecuInfoList->hmiEcuInfoArray[i]->ecuVersion);
                adm_free(ppstPolicyInfo->hmiInfo->ecuInfoList->hmiEcuInfoArray[i]->requestId);
                adm_free(ppstPolicyInfo->hmiInfo->ecuInfoList->hmiEcuInfoArray[i]->swDid);
                adm_free(ppstPolicyInfo->hmiInfo->ecuInfoList->hmiEcuInfoArray[i]);
            }
            adm_free(ppstPolicyInfo->hmiInfo->ecuInfoList->hmiEcuInfoArray);
            adm_free(ppstPolicyInfo->hmiInfo->ecuInfoList);
            adm_free(ppstPolicyInfo->hmiInfo);
            adm_free(ppstPolicyInfo);
        }
    } else {
        log_e("query check result.failed");
    }
}

void adm_cvm_queryResultCallback(ADM_APPLICATION_COMMON_CMDID_TYPE_E* eCmdidType,
    DCHAR** sessionId,
    DCHAR** state,
    ADM_UIM_SOA_CHECK_HMI_POLICY_INFO_T** ppstPolicyInfo,
    D32S* errorCode,
    void* priv)
{
    log_i("adm_cvm_queryResultCallback start");
    ADM_CVS_VERSION_POLICY_T* versionPolicy = NULL;
    *eCmdidType = ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_USER;
    priv = NULL;

    (*ppstPolicyInfo) = (ADM_UIM_SOA_CHECK_HMI_POLICY_INFO_T*)adm_malloc(sizeof(ADM_UIM_SOA_CHECK_HMI_POLICY_INFO_T));

    ADM_UIM_SOA_CHECK_HMI_POLICY_INFO_T* outPolicyInfo = *ppstPolicyInfo;
    outPolicyInfo->hmiFile = (ADM_UIM_HMI_FILE_T*)adm_malloc(sizeof(ADM_UIM_HMI_FILE_T));
    outPolicyInfo->hmiInfo = (ADM_UIM_HMI_INFO*)adm_malloc(sizeof(ADM_UIM_HMI_INFO));
    outPolicyInfo->hmiInfo->ecuInfoList = (ADM_UIM_ECU_INFO_ARRAY_T*)adm_malloc(sizeof(ADM_UIM_ECU_INFO_ARRAY_T));

    versionPolicy = adm_cvs_newPolicyBody();
    if (ADM_ECODE_SUCCESSED == adm_cvs_getVersionPolicy(versionPolicy)) {
        log_i("adm_cvs_getVersionPolicy success");
        *sessionId = adm_strdup(versionPolicy->sessionId);
        *state = adm_strdup("check_success");
        outPolicyInfo->hmiFile->policyFileUrl = adm_strdup(versionPolicy->hmiMember->ucmHmiPolicyFileUrl);
        outPolicyInfo->hmiFile->policyFileSize = (D64U)versionPolicy->hmiMember->ucmHmiPolicyFileSize;
        outPolicyInfo->hmiFile->policyFileHash = adm_strdup(versionPolicy->hmiMember->ucmHmiPolicyFileHash);
        /*option*/
        if (NULL != versionPolicy->hmiMember->vehicleVersion) {
            outPolicyInfo->hmiInfo->vehicleVersion = adm_strdup(versionPolicy->hmiMember->vehicleVersion);
        }
        outPolicyInfo->hmiInfo->totalSize = (D64U)versionPolicy->hmiMember->totalSize;

        outPolicyInfo->hmiInfo->ecuInfoList->hmiEcuInfoArray = (ADM_UIM_ECU_INFO_T**)adm_malloc(
            sizeof(ADM_UIM_ECU_INFO_T*) * versionPolicy->hmiMember->hmiEcuInfoArrayLength);

        log_i("hmiEcuInfoArrayLength = %d", versionPolicy->hmiMember->hmiEcuInfoArrayLength);

        outPolicyInfo->hmiInfo->ecuInfoList->hmiEcuInfoArrayLength = versionPolicy->hmiMember->hmiEcuInfoArrayLength;

        for (D16U i = 0; i < versionPolicy->hmiMember->hmiEcuInfoArrayLength; i++) {
            outPolicyInfo->hmiInfo->ecuInfoList->hmiEcuInfoArray[i]
                = (ADM_UIM_ECU_INFO_T*)adm_malloc(sizeof(ADM_UIM_ECU_INFO_T));

            log_i("requestId[%d]  = %s", i, outPolicyInfo->hmiInfo->ecuInfoList->hmiEcuInfoArray[i]->requestId);
            outPolicyInfo->hmiInfo->ecuInfoList->hmiEcuInfoArray[i]->requestId
                = adm_strdup(versionPolicy->hmiMember->hmiEcuInfoArray[i]->requestId);

            log_i("swDid[%d]  = %s", i, outPolicyInfo->hmiInfo->ecuInfoList->hmiEcuInfoArray[i]->swDid);
            outPolicyInfo->hmiInfo->ecuInfoList->hmiEcuInfoArray[i]->swDid
                = adm_strdup(versionPolicy->hmiMember->hmiEcuInfoArray[i]->swDid);

            log_i("ecuVersion[%d]  = %s", i, outPolicyInfo->hmiInfo->ecuInfoList->hmiEcuInfoArray[i]->ecuVersion);
            outPolicyInfo->hmiInfo->ecuInfoList->hmiEcuInfoArray[i]->ecuVersion
                = adm_strdup(versionPolicy->hmiMember->hmiEcuInfoArray[i]->ecuVersion);

            log_i("ecuSize[%d]  = %d", i, outPolicyInfo->hmiInfo->ecuInfoList->hmiEcuInfoArray[i]->ecuSize);
            outPolicyInfo->hmiInfo->ecuInfoList->hmiEcuInfoArray[i]->ecuSize
                = (D64U)versionPolicy->hmiMember->hmiEcuInfoArray[i]->ecuSize;
        }

        outPolicyInfo->upgradeMode = (ADM_UIM_UPGRADE_MODE_E)versionPolicy->upgradeMode;
        *eCmdidType = (ADM_APPLICATION_COMMON_CMDID_TYPE_E)versionPolicy->upgradeMode;
        
        adm_cvm_ucmGetTotalTimeConsuming(&(outPolicyInfo->iTotalTimeS));
        log_i("upgradeMode = %d, iTotalTimeConsuming = %d [s]", outPolicyInfo->upgradeMode, outPolicyInfo->iTotalTimeS);

        (*errorCode) = ADM_ECODE_SUCCESSED;
    } else {
        log_i("adm_cvs_getVersionPolicy failed");
        *state = adm_strdup("check_failed");
        (*errorCode) = ADM_ECODE_FAILED;
    }

    if (ADM_ECODE_SUCCESSED == *errorCode) {
        log_i("policyFileUrl %s", outPolicyInfo->hmiFile->policyFileUrl);
        log_i("policyFileSize %llu", outPolicyInfo->hmiFile->policyFileSize);
        log_i("policyFileHash %s", outPolicyInfo->hmiFile->policyFileHash);

        log_i("vehicleVersion %s", outPolicyInfo->hmiInfo->vehicleVersion);
        log_i("totalSize %llu", outPolicyInfo->hmiInfo->totalSize);

        log_i("hmiEcuInfoArrayLength %d ", outPolicyInfo->hmiInfo->ecuInfoList->hmiEcuInfoArrayLength);

        for (D16U i = 0; i < outPolicyInfo->hmiInfo->ecuInfoList->hmiEcuInfoArrayLength; i++) {
            log_i("requestId[%d]  %s", i, outPolicyInfo->hmiInfo->ecuInfoList->hmiEcuInfoArray[i]->requestId);
            log_i("swDid[%d]  %s", i, outPolicyInfo->hmiInfo->ecuInfoList->hmiEcuInfoArray[i]->swDid);
            log_i("ecuVersion[%d]  %s", i, outPolicyInfo->hmiInfo->ecuInfoList->hmiEcuInfoArray[i]->ecuVersion);
            log_i("ecuSize[%d]  %llu", i, outPolicyInfo->hmiInfo->ecuInfoList->hmiEcuInfoArray[i]->ecuSize);
        }
    }
    adm_cvs_deletePolicyBody(versionPolicy);
    log_i("adm_cvm_queryResultCallback end");
    return;
}

void adm_cvm_checkQueryEcuInfoCallback(DBOOL* pIsExist, void* priv)
{
    DCHAR* pAllEcuInfoStr = NULL;
    D32S iRet = ADM_ECODE_FAILED;

    if (ADM_IS_NULL(pIsExist))
    {
        log_e("Invalid param.");
        return;
    }

    iRet = adm_eim_getEcuInfoManagerKvGetStr(&pAllEcuInfoStr);
    if (ADM_ECODE_SUCCESSED == iRet && NULL != pAllEcuInfoStr)
    {
        log_i("Get all ecu info success.");
        *pIsExist = DTRUE;
    }
    else 
    {
        log_i("Get all ecu info failed.");
        *pIsExist = DFALSE;
    }

    return;
}