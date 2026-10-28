#include "adm_cvm_gtest.h"
#include "adm_cvm_parseJson.h"
#include "adm_memory.h"
#include "adm_file.h"
#include "adm_string.h"
#include "adm_ocs_airProtocolAdapter.h"
#include "adm_dpo_download.h"
#include "adm_ocm_interface.h"
#include "adm_cvm_uim.h"
#include "adm_cfg_default.h"
#include "adm_com_kvstorage.h"
#include "adm_com_lib.h"
#include "adm_cfg_api.h"
#include "adm_file.h"

#define ADM_CVM_TEST_CONFIGJSON_FILEPATH 256

ADM_CVM_TEST_OTHER_INFO_CONF_T g_stOtherConfigData;

#ifdef __cplusplus
extern "C" {

/*-----------------------------step 1、代码重构-------------------------------*/
DCHAR *__wrap_adm_ecs_getRsConfigVersion(void)
{
    if(ADM_ECODE_SUCCESSED == g_stOtherConfigData.stConfVer.ret)
    {
        log_e("adm_ecs_getRsConfigVersion success, configVersion = %s", g_stOtherConfigData.stConfVer.configVersion);
        return adm_strdup(g_stOtherConfigData.stConfVer.configVersion);
    }
    return NULL;
}

D32S __wrap_adm_eim_getVehicleBigVersion(DCHAR** vehicleBigVersion)
{
    D32S iret = ADM_ECODE_FAILED;

    if(NULL == vehicleBigVersion)
    {
        return ADM_ECODE_INVALID_PARAM;
    }

    if(ADM_ECODE_SUCCESSED == g_stOtherConfigData.bigVersion.ret)
    {
        *vehicleBigVersion = adm_strdup(g_stOtherConfigData.bigVersion.vehicleBigVersion);
        log_i("adm_eim_getVehicleBigVersion success");
    }

    return g_stOtherConfigData.bigVersion.ret;
}

ADM_OCS_CHECK_RESP_ADAPTER_T* __wrap_adm_ocs_newCheckRespAdapterBody(void)
{
    if(ADM_ECODE_SUCCESSED == g_stOtherConfigData.newCheckRespAdapterBody_ret)
    {
        ADM_OCS_CHECK_RESP_ADAPTER_T* checkResp = NULL;
        checkResp = (ADM_OCS_CHECK_RESP_ADAPTER_T*)adm_malloc(sizeof(ADM_OCS_CHECK_RESP_ADAPTER_T));
        if (NULL != checkResp) {
            (void)adm_memset(checkResp, 0x00, sizeof(ADM_OCS_CHECK_RESP_ADAPTER_T), sizeof(ADM_OCS_CHECK_RESP_ADAPTER_T));
        }
        log_i("adm_ocs_newCheckRespAdapterBody success");
        return checkResp;
    }
    return NULL;
}

ADM_OCS_CHECK_REQ_ADAPTER_T* __wrap_adm_ocs_newCheckReqAdapterBody(D32S cmdId,
    D64S timestamp,
    const DCHAR* netType,
    const DCHAR* versionNumber,
    const DCHAR* sdkVersion,
    const DCHAR* confVersion,
    const DCHAR* allEcuInfoObjectStr,
    const DCHAR* allModuleVerInfoStr,
    const DCHAR* extsObjectStr,
    ADM_OCS_BUSINESS_COMM_INTERFACE_CALLBACK_F fnBusinessCommCb)
{
    if(ADM_ECODE_SUCCESSED == g_stOtherConfigData.newCheckReqAdapterBody_ret)
    {
        if (NULL == netType || NULL == sdkVersion || NULL == confVersion || NULL == allEcuInfoObjectStr || NULL == fnBusinessCommCb) {
            return NULL;
        }

        ADM_OCS_CHECK_REQ_ADAPTER_T* checkReq = NULL;
        checkReq = (ADM_OCS_CHECK_REQ_ADAPTER_T*)adm_malloc(sizeof(ADM_OCS_CHECK_REQ_ADAPTER_T));
        if (NULL != checkReq) {
            (void)adm_memset(checkReq, 0x00, sizeof(ADM_OCS_CHECK_REQ_ADAPTER_T), sizeof(ADM_OCS_CHECK_REQ_ADAPTER_T));

            checkReq->cmdId = cmdId;
            checkReq->timestamp = timestamp;
            checkReq->netType = adm_strdup(netType);
            if (NULL != versionNumber) {
                checkReq->versionNumber = adm_strdup(versionNumber);
            } else {
                checkReq->versionNumber = NULL;
            }
            checkReq->sdkVersion = adm_strdup(sdkVersion);
            checkReq->confVersion = adm_strdup(confVersion);
            checkReq->allEcuInfoObjectStr = adm_strdup(allEcuInfoObjectStr);
            if (NULL != allModuleVerInfoStr) {
                checkReq->allModuleVerInfoStr = adm_strdup(allModuleVerInfoStr);
            } else {
                checkReq->allModuleVerInfoStr = NULL;
            }
            if (NULL != extsObjectStr) {
                checkReq->extsObjectStr = adm_strdup(extsObjectStr);
            } else {
                checkReq->extsObjectStr = NULL;
            }
            checkReq->fnBusinessCommCb = fnBusinessCommCb;
        }
        log_i("adm_ocs_newCheckReqAdapterBody success");
        return checkReq;
    }
    return NULL;
}

D32S __wrap_adm_ocs_checkProtocolAdapter(const ADM_OCS_CHECK_REQ_ADAPTER_T* checkReq, DCHAR** adapterCheckReq)
{
    DCHAR *pcConfigFileDir = NULL;
    DCHAR acFilePath[ADM_CVM_GTEST_FILE_MAX_LEN] = { 0 };
    DCHAR acConfigFileFullPath[ADM_CVM_TEST_CONFIGJSON_FILEPATH] = {0};
    DCHAR* pcFileReadbuf = NULL;

    if(ADM_ECODE_SUCCESSED == g_stOtherConfigData.checkProtocolAdapter_ret)
    {
        if (NULL == checkReq || NULL == adapterCheckReq) {
            log_e("input param invalid");
            return ADM_ECODE_INVALID_PARAM;
        }

        DCHAR* pcConfigFileDir = adm_cvm_GtestGetOtherConfFileDir();
        if (NULL == pcConfigFileDir)
        {
            log_e(" Test Json Config File Not Found \n");
            return ADM_ECODE_FAILED;
        }

        (void)adm_snprintf(acFilePath, sizeof(acFilePath), "%s/%s", pcConfigFileDir, ADM_CVM_GTEST_CHECK_REQ_FILE);
        pcFileReadbuf = adm_cvm_GtestReadConfFile(acFilePath);
        if (NULL == pcFileReadbuf)
        {
            return ADM_ECODE_FAILED;
        }
        *adapterCheckReq = pcFileReadbuf;
        log_i("checkReq = %s", pcFileReadbuf);
        log_i("adm_ocs_checkProtocolAdapter success");
    }
    return g_stOtherConfigData.checkProtocolAdapter_ret;
}

static DBOOL adm_cvm_getJsonStrVal(const json_object* obj, const DCHAR* key, DCHAR** str)
{
    if (NULL == obj || NULL == key) {
        return FALSE;
    }

    json_object* jsonStr = NULL;

    if ((json_bool)0 == json_object_object_get_ex(obj, key, &jsonStr)) {
        return FALSE;
    }
    *str = adm_strdup(json_object_get_string(jsonStr));
    if (NULL != *str) {
        return TRUE;
    }
    return FALSE;
}

static DBOOL adm_cvm_getJsonIntVal(const json_object* obj, const DCHAR* key, D32S* val)
{
    if (NULL == obj || NULL == key) {
        return FALSE;
    }

    json_object* jsonStr = NULL;
    if ((json_bool)0 == json_object_object_get_ex(obj, key, &jsonStr)) {
        return FALSE;
    }
    *val = json_object_get_int(jsonStr);
    return TRUE;
}

static DBOOL adm_cvm_getJsonInt64Val(const json_object* obj, const DCHAR* key, D64S* val)
{
    if (NULL == obj || NULL == key) {
        return FALSE;
    }

    json_object* jsonStr = NULL;
    if ((json_bool)0 == json_object_object_get_ex(obj, key, &jsonStr)) {
        return FALSE;
    }
    *val = json_object_get_int64(jsonStr);
    return TRUE;
}

static D32S adm_cvm_parseCloudRespStatus(const DCHAR* respBodyFromCloud, json_object** respObject, D32S* status)
{
    D32S retVal = ADM_ECODE_SUCCESSED;
    D32S respStatus;
    DCHAR* deCryptRespMessageBody = NULL;
    json_object* respObjectTmp = NULL;

    do {
        deCryptRespMessageBody = adm_strdup(respBodyFromCloud);

        log_i("respBodyFromCloud = %s", deCryptRespMessageBody);
        log_i("deCryptRespMessageBody str to json");
        respObjectTmp = json_tokener_parse(deCryptRespMessageBody);
        if (NULL == respObjectTmp) {
            log_e("parse resp body failed");
            retVal = ADM_ECODE_STRING_TO_JSON_FAILED;
            break;
        }
        /*get status*/
        log_i("get resp status");
        if (FALSE == adm_cvm_getJsonIntVal(respObjectTmp, ADM_COM_AIR_PROTOCOL_FIELD_STATUS, &respStatus)) {
            log_e("get status value from json_object. failed");
            retVal = ADM_ECODE_JSON_GET_INT_FAILED;
            break;
        }
    } while(FALSE);

    if (ADM_ECODE_SUCCESSED == retVal) {
        *status = respStatus;
        *respObject = respObjectTmp;
        respObjectTmp = NULL;
    }
    adm_free(deCryptRespMessageBody);
    return retVal;
}

D32S __wrap_adm_ocs_checkProtocolHmiMember(json_object* hmiPolicyFileObject, ADM_OCS_CHECK_HMI_MEMBER_T* hmiMember)
{
    if (NULL == hmiPolicyFileObject || NULL ==hmiMember) {
        log_e("input param invalid");
        return ADM_ECODE_INVALID_PARAM;
    }
    D32S retVal = ADM_ECODE_SUCCESSED;
    json_object* hmiFileObject = NULL;
    json_object* hmiInfoObject = NULL;
    json_object* hmiVehicleVersionInfoObject = NULL;
    json_object* hmiEcuInfosObject = NULL;

    do {
        if ((json_bool)0 == json_object_object_get_ex(hmiPolicyFileObject, ADM_COM_AIR_PROTOCOL_FIELD_HMIFILE, &hmiFileObject)) {
            retVal = ADM_ECODE_JSON_GET_OBJECT_FAILED;
            log_e("get hmiFile object. failed");
            break;
        }
        log_i("get hmiPolicy fileUrl");
        if (FALSE == adm_cvm_getJsonStrVal(hmiFileObject, ADM_COM_AIR_PROTOCOL_FIELD_URL, &(hmiMember->hmiShowFileUrl))) {
            retVal = ADM_ECODE_JSON_GET_STRING_FAILED;
            log_e("get fileUrl value from json_object. failed");
            break;
        }
        log_i("get hmiPolicy fileHash");
        if (FALSE == adm_cvm_getJsonStrVal(hmiFileObject, ADM_COM_AIR_PROTOCOL_FIELD_HASH, &(hmiMember->hmiShowFileHash))) {
            retVal = ADM_ECODE_JSON_GET_STRING_FAILED;
            log_e("get fileHash value from json_object. failed");
            break;
        }
        log_i("get hmiPolicy fileSize");
        if (FALSE == adm_cvm_getJsonInt64Val(hmiFileObject, ADM_COM_AIR_PROTOCOL_FIELD_SIZE, &(hmiMember->hmiShowFileSize))) {
            retVal = ADM_ECODE_JSON_GET_INT_FAILED;
            log_e("get fileSize value from json_object. failed");
            break;
        }
        log_i("get hmiPolicy fileName");
        if (FALSE == adm_cvm_getJsonStrVal(hmiFileObject, ADM_COM_AIR_PROTOCOL_FIELD_FILENAME, &(hmiMember->hmiShowFileName))) {
            retVal = ADM_ECODE_JSON_GET_STRING_FAILED;
            log_e("get fileName value from json_object. failed");
            break;
        }
        log_i("get hmiPolicy file security info");
        retVal = adm_com_parseJsonFileSecurityInfo(hmiFileObject, &(hmiMember->fileSecurityInfo));
        if (ADM_ECODE_SUCCESSED != retVal) {
            log_e("adm_com_parseJsonFileSecurityInfo return error[%d]", retVal);
            break;
        }
        log_i("get hmiPolicy hmiInfo");
        if ((json_bool)0 == json_object_object_get_ex(hmiPolicyFileObject, ADM_COM_AIR_PROTOCOL_FIELD_HMIINFO, &hmiInfoObject)) {
            retVal = ADM_ECODE_JSON_GET_OBJECT_FAILED;
            log_e("get fileInfo value from json_object. failed");
            break;
        }

        log_i("get hmiPolicy hmiInfo vehicleVersionInfo");
        if ((json_bool)0 == json_object_object_get_ex(
                hmiInfoObject, ADM_COM_AIR_PROTOCOL_FIELD_VEHICLEVERSIONINFO, &hmiVehicleVersionInfoObject)) {
            retVal = ADM_ECODE_JSON_GET_OBJECT_FAILED;
            log_e("get hmiInfo vehicleVersionInfo from json_object. failed");
            break;
        }

        log_i("get hmiPolicy hmiInfo vehicleVersionInfo totalSize");
        if (FALSE == adm_cvm_getJsonInt64Val(
                hmiVehicleVersionInfoObject, ADM_COM_AIR_PROTOCOL_FIELD_TOTALSIZE, &(hmiMember->totalSize))) {
            retVal = ADM_ECODE_JSON_GET_INT64_FAILED;
            log_e("get hmiInfo totalSize from json_object. failed");
            break;
        }

        /*option*/
        if (TRUE == adm_cvm_getJsonStrVal(
                hmiVehicleVersionInfoObject, ADM_COM_AIR_PROTOCOL_FIELD_VEHICLEVERSION, &(hmiMember->vehicleVersion))) {
            log_i("get hmiPolicy hmiInfo vehicleVersionInfo vehicleVersion");
        }

        log_i("get hmiPolicy hmiInfo ecuInfos");
        if ((json_bool)0 == json_object_object_get_ex(hmiInfoObject, ADM_COM_AIR_PROTOCOL_FIELD_ECUINFOS, &hmiEcuInfosObject)) {
            retVal = ADM_ECODE_JSON_GET_OBJECT_FAILED;
            log_e("get hmiInfo ecuInfos from json_object. failed");
            break;
        }

        D32S ecuInfoLen = (D32S)json_object_array_length(hmiEcuInfosObject);

        hmiMember->hmiEcuInfoArrayLength = (D16U)ecuInfoLen;
        hmiMember->hmiEcuInfoArray
            = (ADM_OCS_CHECK_ECU_INFO_MEMBER_T**)adm_malloc(sizeof(ADM_OCS_CHECK_ECU_INFO_MEMBER_T) * (DSIZE)ecuInfoLen);

        json_object* hmiEcuInfo = NULL;
        for (D32S i = 0; i < ecuInfoLen; ++i) {
            hmiEcuInfo = json_object_array_get_idx(hmiEcuInfosObject, (size_t)i);

            hmiMember->hmiEcuInfoArray[i]
                = (ADM_OCS_CHECK_ECU_INFO_MEMBER_T*)adm_malloc(sizeof(ADM_OCS_CHECK_ECU_INFO_MEMBER_T));

            log_i("get hmiPolicy hmiInfo ecuReqId");
            if (FALSE == adm_cvm_getJsonStrVal(
                    hmiEcuInfo, ADM_COM_AIR_PROTOCOL_FIELD_ECUREQID, &(hmiMember->hmiEcuInfoArray[i]->requestId))) {
                retVal = ADM_ECODE_JSON_GET_STRING_FAILED;
                log_e("get hmiInfo hmiInfo ecuReqId from json_object. failed");
                break;
            }

            /*option ecuPartNum*/
            if (TRUE == adm_cvm_getJsonStrVal(
                    hmiEcuInfo, ADM_COM_AIR_PROTOCOL_FIELD_ECUPARTNUM, &(hmiMember->hmiEcuInfoArray[i]->ecuPartNum))) {
                log_i("get hmiPolicy hmiInfo ecuPartNum");
            }

            log_i("get hmiPolicy hmiInfo ecuSwDid");
            if (FALSE == adm_cvm_getJsonStrVal(
                    hmiEcuInfo, ADM_COM_AIR_PROTOCOL_FIELD_ECUSWDID, &(hmiMember->hmiEcuInfoArray[i]->swDid))) {
                retVal = ADM_ECODE_JSON_GET_STRING_FAILED;
                log_e("get hmiInfo hmiInfo ecuSwDid from json_object. failed");
                break;
            }

            log_i("get hmiPolicy hmiInfo ecuVersion");
            if (FALSE == adm_cvm_getJsonStrVal(
                    hmiEcuInfo, ADM_COM_AIR_PROTOCOL_FIELD_ECUVERSION, &(hmiMember->hmiEcuInfoArray[i]->ecuVersion))) {
                retVal = ADM_ECODE_JSON_GET_STRING_FAILED;
                log_e("get hmiInfo hmiInfo ecuVersion from json_object. failed");
                break;
            }

            log_i("get hmiPolicy hmiInfo ecuSize");
            if (FALSE == adm_cvm_getJsonInt64Val(
                    hmiEcuInfo, ADM_COM_AIR_PROTOCOL_FIELD_ECUSIZE, &(hmiMember->hmiEcuInfoArray[i]->ecuSize))) {
                retVal = ADM_ECODE_JSON_GET_INT64_FAILED;
                log_e("get hmiInfo hmiInfo ecuSize from json_object. failed");
                break;
            }
        }
    } while(FALSE);
    return retVal;
}

static D32S adm_cvm_respDataGetHmiPolicyFiled(ADM_OCS_CHECK_RESP_ADAPTER_T* checkResp,
    json_object* respObject,
    json_object* dataObject)
{
    D32S retVal = ADM_ECODE_SUCCESSED;
    json_object* hmiPolicyFileObject = NULL;
    json_object* hmiFileObject = NULL;
    json_object* hmiInfoObject = NULL;
    json_object* hmiVehicleVersionInfoObject = NULL;
    json_object* hmiEcuInfosObject = NULL;
    do {
        log_i("parse data object");
        checkResp->respMessageBodyStr = adm_strdup(json_object_to_json_string(respObject));
        if (NULL == checkResp->respMessageBodyStr) {
            retVal = ADM_ECODE_JSON_TO_STRING_FAILED;
            log_e("json data object to string. failed");
            break;
        }
        /*get sessionId*/
        log_i("get sessionId");
        if (FALSE == adm_cvm_getJsonStrVal(dataObject, ADM_COM_AIR_PROTOCOL_FIELD_SESSIONID, &(checkResp->sessionId))) {
            log_e("get sessionId value from json_object. failed");
            retVal = ADM_ECODE_JSON_GET_STRING_FAILED;
            break;
        }
        /*get upgradeMode*/
        log_i("get upgradeMode");
        if (FALSE == adm_cvm_getJsonIntVal(dataObject, ADM_COM_AIR_PROTOCOL_FIELD_UPGRADEMODE, &(checkResp->upgradeMode))) {
            log_e("get upgradeMode value from json_object. failed");
            retVal = ADM_ECODE_JSON_GET_STRING_FAILED;
            break;
        }
        log_i("get hmiPolicy object");
        {
            if ((json_bool)0 == json_object_object_get_ex(dataObject, ADM_COM_AIR_PROTOCOL_FIELD_HMIPOLICY, &hmiPolicyFileObject)) {
                retVal = ADM_ECODE_JSON_GET_OBJECT_FAILED;
                log_e("get hmiPolicy object. failed");
                break;
            }

            checkResp->hmiMember = (ADM_OCS_CHECK_HMI_MEMBER_T*)adm_malloc(sizeof(ADM_OCS_CHECK_HMI_MEMBER_T));
            retVal = adm_ocs_checkProtocolHmiMember(hmiPolicyFileObject, checkResp->hmiMember);
            if (ADM_ECODE_SUCCESSED != retVal) {
                log_e("adm_ocs_checkProtocolHmiMember return failed");
                break;
            }
        }

    } while(FALSE);
    return retVal;
}


D32S __wrap_adm_ocs_otaStatusCodeTransfer(const D32S statusCode)
{
    D32S iRet = ADM_ECODE_FAILED;
    switch (statusCode) {
    case ADM_OCS_AIR_STATUS_CODE_SUCCESSED:
        iRet = ADM_ECODE_SUCCESSED;
        break;
    case ADM_OCS_AIR_STATUS_CODE_NO_NEW_VERSION:
        iRet = ADM_ECODE_NO_NEW_VERSION;
        break;
    case ADM_OCS_AIR_STATUS_CODE_VEHICLE_NOT_EXIST:
        iRet = ADM_ECODE_VEHICLE_NOT_EXIST;
        break;
    case ADM_OCS_AIR_STATUS_CODE_CONFIG_NEED_UPDATE:
        iRet = ADM_ECODE_CONFIG_NEED_UPDATE;
        break;
    case ADM_OCS_AIR_STATUS_CODE_CONFIG_NOT_EXIST:
        iRet = ADM_ECODE_CONFIG_NOT_EXIST;
        break;
    case ADM_OCS_AIR_STATUS_CODE_PARAM_JSON_FORMAT_FAILED:
        iRet = ADM_ECODE_PARAM_JSON_FORMAT_FAILED;
        break;
    case ADM_OCS_AIR_STATUS_CODE_PARAM_VERIFY_FAILED:
        iRet = ADM_ECODE_PARAM_VERIFY_FAILED;
        break;
    case ADM_OCS_AIR_STATUS_CODE_SIGN_VERIFY_FAILED:
        iRet = ADM_ECODE_SIGN_VERIFY_FAILED;
        break;
    case ADM_OCS_AIR_STATUS_CODE_FILE_SAVED_FAILED:
        iRet = ADM_ECODE_FILE_SAVED_FAILED;
        break;
    case ADM_OCS_AIR_STATUS_CODE_DECRYPT_VERIFY_FAILED:
        iRet = ADM_ECODE_DECRYPT_VERIFY_FAILED;
        break;
    case ADM_OCS_AIR_STATUS_CODE_UPGRADE_TASK_STOPED:
        iRet = ADM_ECODE_UPGRADE_TASK_STOPED;
        break;
    case ADM_OCS_AIR_STATUS_CODE_SERVER_INTERNAL_EXCEPTION:
        iRet = ADM_ECODE_SERVER_INTERNAL_EXCEPTION;
        break;
    case ADM_OCS_AIR_STATUS_CODE_NO_SHORT_TIME_ACCESS:
        iRet = ADM_ECODE_NO_SHORT_TIME_ACCESS;
        break;
    default:
        iRet = ADM_ECODE_UNKONW_STATUS_CODE;
        break;
    }

    return iRet;
}

D32S __wrap_adm_ocs_checkProtocolParse(DCHAR* respBodyFromCloud, ADM_OCS_CHECK_RESP_ADAPTER_T* checkResp)
{
    if (NULL == respBodyFromCloud || NULL == checkResp) {
        log_e("input param invalid");
        return ADM_ECODE_INVALID_PARAM;
    }

    D32S retVal = ADM_ECODE_SUCCESSED;
    D32S respStatus = -1;
    json_object* respObject = NULL;
    json_object* dataObject = NULL;
    json_object* extsObject = NULL;

    if(ADM_ECODE_SUCCESSED == g_stOtherConfigData.checkProtocolParse_ret)
    {
        do {
            log_i("parse cloud resp message");
            retVal = adm_cvm_parseCloudRespStatus(respBodyFromCloud, &respObject, &respStatus);
            if (ADM_ECODE_SUCCESSED != retVal) {
                log_e("adm_ocs_parseCloudRespStatus.error[%d]", retVal);
                break;
            }
            if(ADM_OCS_AIR_STATUS_CODE_SUCCESSED == respStatus)
            {
                log_i("adm_ocs_checkProtocolParse CloudRespStatus = %d", respStatus);
            }
            else
            {
                log_e("adm_ocs_checkProtocolParse CloudRespStatus = %d", respStatus);
            }

            retVal = adm_ocs_otaStatusCodeTransfer(respStatus);
            if (retVal != ADM_ECODE_SUCCESSED) {
                log_i("statusField:%d, iRet:%d", respStatus, retVal);
                return retVal;
            }

            /*get dataObject*/
            log_i("get data object");
            if ((json_bool)0 == json_object_object_get_ex(respObject, ADM_COM_AIR_PROTOCOL_FIELD_DATA, &dataObject)) {
                log_e("get data object from json_object. failed");
                retVal = ADM_ECODE_JSON_GET_OBJECT_FAILED;
                break;
            }
            /*option:get exts*/
            log_i("get exts");
            if ((json_bool)0 != json_object_object_get_ex(respObject, ADM_COM_AIR_PROTOCOL_FIELD_EXT, &extsObject)) {
                checkResp->extsObjectStr = adm_strdup(json_object_to_json_string(extsObject));
                log_i("resp mesaage body contains exts");
            } else {
                log_i("resp mesaage body is not contains exts");
            }

            /*get data filed*/
            log_i("get data filed");
            retVal = adm_cvm_respDataGetHmiPolicyFiled(checkResp, respObject, dataObject);
            if (ADM_ECODE_SUCCESSED != retVal) {
                log_e("call adm_ocs_respDataGetHmiPolicyFiled. failed");
                break;
            }
            log_i("adm_ocs_checkProtocolParse success");
        } while(FALSE);
        if (NULL != respObject) {
            (void)json_object_put(respObject);
        }
        return retVal;
    }
    return g_stOtherConfigData.checkProtocolParse_ret;
}

D32S __wrap_adm_ecs_getEventReportLevel(D32S *ps32EventReportLevel)
{
    if(ADM_ECODE_SUCCESSED == g_stOtherConfigData.stEvent_Report.ret)
    {
        *ps32EventReportLevel = g_stOtherConfigData.stEvent_Report.reportLevel;
    }

    return g_stOtherConfigData.stEvent_Report.ret;
}

D32S __wrap_adm_ocm_businessCommInterface(const ADM_OCS_BUSINESS_TYPE_E eBusinessType, const DCHAR* reqMessageBody, DCHAR** rspMessageBody, DSIZE* rspStatusCode)
{
    DCHAR *pcConfigFileDir = NULL;
    DCHAR acFilePath[ADM_CVM_GTEST_FILE_MAX_LEN] = { 0 };
    DCHAR acConfigFileFullPath[ADM_CVM_TEST_CONFIGJSON_FILEPATH] = {0};
    DCHAR* pcFileReadbuf = NULL;

    if(ADM_ECODE_SUCCESSED == g_stOtherConfigData.Busi_Comm_inter_ret.ret)
    {
        DCHAR* pcConfigFileDir = adm_cvm_GtestGetOtherConfFileDir();
        if (NULL == pcConfigFileDir)
        {
            log_e(" Test Json Config File Not Found \n");
            return ADM_ECODE_FAILED;
        }

        if(ADM_OCS_AIR_STATUS_CODE_SUCCESSED == g_stOtherConfigData.Busi_Comm_inter_ret.rspStatusCode)
        {
            log_i("adm_ocm_businessCommInterface rspStatusCode = %llu", g_stOtherConfigData.Busi_Comm_inter_ret.rspStatusCode);
            (void)adm_snprintf(acFilePath, sizeof(acFilePath), "%s/%s", pcConfigFileDir, ADM_CVM_GTEST_CHECK_RES_SUCCESSED_FILE);
            pcFileReadbuf = adm_cvm_GtestReadConfFile(acFilePath);
        }
        else
        {
            log_e("adm_ocm_businessCommInterface rspStatusCode = %llu", g_stOtherConfigData.Busi_Comm_inter_ret.rspStatusCode);
            (void)adm_snprintf(acFilePath, sizeof(acFilePath), "%s/%s", pcConfigFileDir, ADM_CVM_GTEST_CHECK_RES_FAILED_FILE);
            pcFileReadbuf = adm_cvm_GtestReadConfFile(acFilePath);
        }

        if (NULL == pcFileReadbuf)
        {
            return ADM_ECODE_FAILED;
        }
        
        log_i("adm_ocm_businessCommInterface success");
    }

    *rspMessageBody = pcFileReadbuf;
    return g_stOtherConfigData.Busi_Comm_inter_ret.ret;
}

D32S __wrap_adm_dpo_doDownloadFile(ADM_DPO_DOWNLOAD_REQPARAM_T *pstDownReqInfo)
{
    D32S retVal = ADM_ECODE_SUCCESSED;
    void* file = NULL;
    if(ADM_ECODE_SUCCESSED == g_stOtherConfigData.doDownloadFile_ret)
    {
        log_i("adm_dpo_doDownloadFile success");
        return adm_fopen(pstDownReqInfo->filePath, "a+", &file);
    }
    return g_stOtherConfigData.doDownloadFile_ret;
}

D32S __wrap_adm_ecm_decryptAndVerificationSignature(ADM_COM_FILEOBJECT_T *pstFileObject)
{
    if(ADM_ECODE_SUCCESSED == g_stOtherConfigData.doDownloadFile_ret)
    {
        log_i("adm_ecm_decryptAndVerificationSignature success");
    }
    return g_stOtherConfigData.decryptAndVerificationSignature_ret;
}

void __wrap_adm_uim_checkResultNotify(const ADM_APPLICATION_COMMON_CMDID_TYPE_E eCmdidType, const DCHAR* sessionId, 
                               const DCHAR* state, const ADM_UIM_SOA_CHECK_HMI_POLICY_INFO_T* pstPolicyInfo, 
                               const D32S errorCode)
{
    log_i("eCmdidType[%d] sessionId[%s] state[%s] errorCode[%d]", eCmdidType, sessionId, state, errorCode);
    if(pstPolicyInfo)
    {
        log_i("upgradeMode[%d]",   pstPolicyInfo->upgradeMode);
        log_i("policyFileUrl[%s]",    pstPolicyInfo->hmiFile->policyFileUrl);
        log_i("policyFileHash[%s]",   pstPolicyInfo->hmiFile->policyFileHash);
        log_i("policyFileSize[%lld]", pstPolicyInfo->hmiFile->policyFileSize);
        log_i("vehicleVersion[%s]",   pstPolicyInfo->hmiInfo->vehicleVersion);
        log_i("totalSize[%lld]", pstPolicyInfo->hmiInfo->totalSize);
    }
}

void __wrap_adm_uim_hmi_checkResult_RR(const ADM_APPLICATION_COMMON_CMDID_TYPE_E eCmdidType,const DCHAR *sessionId,const DCHAR *state,
                const ADM_UIM_SOA_CHECK_HMI_POLICY_INFO_T *pstPolicyInfo,const D32S reqCode,D32S *respCode)
{
    log_i("eCmdidType[%d] sessionId[%s] state[%s] reqCode[%d]", eCmdidType, sessionId, state, reqCode);
    if(pstPolicyInfo)
    {
        log_i("upgradeMode[%d]",   pstPolicyInfo->upgradeMode);
        log_i("policyFileUrl[%s]",    pstPolicyInfo->hmiFile->policyFileUrl);
        log_i("policyFileHash[%s]",   pstPolicyInfo->hmiFile->policyFileHash);
        log_i("policyFileSize[%lld]", pstPolicyInfo->hmiFile->policyFileSize);
        log_i("vehicleVersion[%s]",   pstPolicyInfo->hmiInfo->vehicleVersion);
        log_i("totalSize[%lld]", pstPolicyInfo->hmiInfo->totalSize);
    }
}

void __wrap_adm_uim_checkProgressNotify(const ADM_APPLICATION_COMMON_CMDID_TYPE_E eCmdidTYpe, 
                                    const DCHAR* state, 
                                    const ADM_APPLICATION_COMMON_CHECK_PROGRESS_TYPE_T eCheckProgressType)
{
    log_i("eCmdidTYpe[%d] state[%s] eCheckProgressType[%d]", 
        eCmdidTYpe, state, eCheckProgressType);
}

DCHAR* __wrap_adm_sdk_getAllModulesVersionFormatJsonStr(void)
{
    DCHAR* allModuleVersionStr = NULL;
    allModuleVersionStr = adm_strdup("[ { \"moduleName\": \"engine\", \"moduleValue\": \"engine_android11_5.2.1_20221206_7f09ff8\" }, { \"moduleName\": \"foundation\", \"moduleValue\": \"foundation_android11_5.2.1_20221206_49a18324\" }, { \"moduleName\": \"ewebserver\", \"moduleValue\": \"ewebserver_android11_5.2.1_20221206_fcc3b55\" }, { \"moduleName\": \"tools\", \"moduleValue\": \"tools_default_version_1.1.0\" }, { \"moduleName\": \"arpc\", \"moduleValue\": \"arpc_android11_5.2.1_20221206_291b95f\" }, { \"moduleName\": \"admfotaapp\", \"moduleValue\": \"admfotaapp_android11_5.2.1_20221206_3c3145b\" }, { \"moduleName\": \"SYSI4HOST\", \"moduleValue\": \"v1.2.3\" }, { \"moduleName\": \"SYSI4UCL\", \"moduleValue\": \"v1.2.3.4\" } ]");
    return allModuleVersionStr;
}

D32S __wrap_adm_ocm_fileDownloadInterface(const DCHAR* downloadUrl, ADM_OCA_FILE_DOWNLOAD_INFO_T1* pstDownloadInfo)
{
    return ADM_ECODE_SUCCESSED;
}

D64S __wrap_adm_irm_getEventSequenceId()
{
    static D64S sequenceId = 0;
	++sequenceId;
	return sequenceId;
}

DBOOL __wrap_adm_vsm_triggerCheck(ADM_APPLICATION_COMMON_CMDID_TYPE_E eTriggerType)
{
    return DTRUE;
}

D32S __wrap_adm_eim_getEcuInfoManagerKvGetStr(DCHAR** ecuInfoListJsonStr)
{
    DCHAR acFilePath[ADM_CVM_GTEST_FILE_MAX_LEN] = { 0 };
    DCHAR* pcFileReadbuf = NULL;
    D32S iRet = ADM_ECODE_FAILED;

    if(ADM_ECODE_SUCCESSED == g_stOtherConfigData.EcuInfoManagerKv_ret)
    {
        DCHAR* pcConfigFileDir = adm_cvm_GtestGetOtherConfFileDir();
        if (NULL == pcConfigFileDir)
        {
            log_e(" Test Json Config File Not Found \n");
            return ADM_ECODE_FAILED;
        }

        (void)adm_snprintf(acFilePath, sizeof(acFilePath), "%s/%s", pcConfigFileDir, ADM_CVM_GTEST_CHECK_EIM_CONF_FILE);
        pcFileReadbuf = adm_cvm_GtestReadConfFile(acFilePath);
        if (NULL == pcFileReadbuf)
        {
            return ADM_ECODE_FAILED;
        }

        *ecuInfoListJsonStr = pcFileReadbuf;
        log_i("adm_eim_getEcuInfoManagerKvGetStr success");
    }
    return g_stOtherConfigData.EcuInfoManagerKv_ret;
}

D32S __wrap_adm_eim_getEcuInfoManagerDimGetStr(DCHAR** ecuInfoListJsonStr)
{
    DCHAR acFilePath[ADM_CVM_GTEST_FILE_MAX_LEN] = { 0 };
    DCHAR* pcFileReadbuf = NULL;
    D32S iRet = ADM_ECODE_FAILED;

    if(ADM_ECODE_SUCCESSED == g_stOtherConfigData.EcuInfoManagerDim_ret)
    {
        DCHAR* pcConfigFileDir = adm_cvm_GtestGetOtherConfFileDir();
        if (NULL == pcConfigFileDir)
        {
            log_e(" Test Json Config File Not Found \n");
            return ADM_ECODE_FAILED;
        }

        (void)adm_snprintf(acFilePath, sizeof(acFilePath), "%s/%s", pcConfigFileDir, ADM_CVM_GTEST_CHECK_EIM_CONF_FILE);
        pcFileReadbuf = adm_cvm_GtestReadConfFile(acFilePath);
        if (NULL == pcFileReadbuf)
        {
            return ADM_ECODE_FAILED;
        }

        *ecuInfoListJsonStr = pcFileReadbuf;
        log_i("adm_eim_getEcuInfoManagerDimGetStr success");
    }
    return g_stOtherConfigData.EcuInfoManagerDim_ret;
}

D32S __wrap_adm_irm_reportVehicleState(D64S timestamp,
    const DCHAR* sdkVersion,
    const DCHAR* sessionId,
    D64S taskId,
    D64S strategyId,
    const DCHAR* state,
    const DCHAR* reason,
    const DCHAR* versionNumber,
    const DCHAR* extsReqJsonStr) 
{
    log_i("timestamp[%ld] sdkVersion[%s] sessionId[%s] taskId[%d] strategyId[%d] state[%s] reason[%s] versionNumber[%s] ext[%s]", 
        timestamp, sdkVersion, sessionId, taskId, strategyId, state, reason, versionNumber, extsReqJsonStr);
    return ADM_ECODE_SUCCESSED;
}

D32S __wrap_adm_irm_reportEvent(const DCHAR* sessionId,
    D64S taskId,
    D64S strategyId,
    D64S timestamp,
    const DCHAR* sdkVersion,
    const DCHAR* extsReqJsonStr,
    ADM_OCS_EVENTS_INFO_LIST_T* eventsList)
{
    log_i("sessionId[%s] taskId[%d] strategyId[%d] timestamp[%ld] sdkVersion[%s] ext[%s]",
        sessionId, taskId, strategyId, timestamp, sdkVersion, extsReqJsonStr);
    for(int i = 0; i < eventsList->reportEventsLen; i++)
    {
        log_i("event[%d]: eventSequenceId[%lld] eventId[%s] eventLevel[%d] eventContext[%s] eventDesc[%s] eventTime[%lld]",
                i, eventsList->reportEvents[i].eventSequenceId, eventsList->reportEvents[i].eventId, eventsList->reportEvents[i].eventLevel,
                eventsList->reportEvents[i].eventContext, eventsList->reportEvents[i].eventDesc, eventsList->reportEvents[i].eventTime);
    }
    return ADM_ECODE_SUCCESSED;
}

}
#endif
/*-----------------------------step 2、测试环境初始化-------------------------------*/
/**
 * @brief kv storage初始化
 * 
 * @return D32S 	0:Successed, None-0:Failed
 */
static D32S __adm_cvm_kvStorageInit(void)
{
//	ADM_APC_CORE_INIT_ENTRY;
	D32S iRet = ADM_ECODE_FAILED;
	DBOOL isKvstorageEnc = FALSE;

	do {
		//!< 为kvstorage初始化加解密接口，通过配置文件中获取是否需要加密
		isKvstorageEnc = FALSE;//adm_cfg_iniParserGetBoolean(ADM_CFG_VUS_IS_KVSTORAGE_ENCRYPT, (DBOOL)ADM_CFG_VUS_IS_KVSTORAGE_ENCRYPT_DEFAULT_VAL);
		if (FALSE == isKvstorageEnc) {
			adm_com_kvstorageInit(NULL, NULL);
		} else {
//			adm_com_kvstorageInit(adm_iss_encryptString, adm_iss_decryptString);
		}

		//!< 初始化core中 kvstorage数据库
		iRet = adm_core_service_init();
		log_i("adm_core_service_init , iRet:%d", iRet);
		if (ADM_ECODE_SUCCESSED != iRet) 
		{
			log_e("adm_core_service_init failed, err:%d", iRet);
			break;
		}
	} while(FALSE);


//	ADM_APC_CORE_INIT_EXIT;
	return iRet;
}


static D32S __adm_cvm_TestInit(void)
{
    DCHAR acFilePath[ADM_CVM_GTEST_FILE_MAX_LEN] = { 0 };
	const DCHAR* configVal = NULL;

   //!< 初始化日志目录

    adm_com_createDir(ADM_CVM_TEST_LOGDIR);
    adm_debug_logInit(ADM_CVM_TEST_LOGDIR, ADM_CVM_TEST_LOGFILE_MAX_LENGTH, ADM_CVM_TEST_LOGFILE_MAX_NUM);

	DCHAR* pcConfigFileDir = adm_cvm_GtestGetOtherConfFileDir();
	if (NULL == pcConfigFileDir)
	{
	    log_e("get pcConfigFileDir failed");
        return ADM_ECODE_FAILED;
	}

    ADM_CFG_API_INFO_T pstApiData;
    pstApiData.isFileExistCb = adm_com_isFileExist;
    pstApiData.writeFileDataCb = adm_com_writeFileData;
	(void)adm_snprintf(acFilePath, sizeof(acFilePath), "%s/%s", pcConfigFileDir, ADM_CVM_TEST_UCMCONFIG_FILE);
    printf("acFilePath dir:%s\n", acFilePath);

	if (ADM_ECODE_SUCCESSED != adm_cfg_iniParserInit(acFilePath, &pstApiData))
    {
        log_e("read %s failed", acFilePath);
        return ADM_ECODE_FAILED;
    }


	//!< 初始化Kv storage
	//!< 初始化KV目录
	configVal = adm_cfg_iniParserGetString(ADM_CFG_VUS_KV_STORAGE_DIR, ADM_CFG_VUS_KV_STORAGE_DIR_DEFAULT_VAL);
	log_i("adm_core_service_initRuntimeDir eType:%d, dir:%s ", ADM_CORE_SERVICE_RUNTIME_DIR_TYPE_KV_STORAGE, configVal);
	if (FALSE == adm_core_service_initRuntimeDir(ADM_CORE_SERVICE_RUNTIME_DIR_TYPE_KV_STORAGE, configVal)) 
	{
	    log_e("adm_core_service_initRuntimeDir eType:%d, dir:%s failed", ADM_CORE_SERVICE_RUNTIME_DIR_TYPE_KV_STORAGE, configVal);
        return ADM_ECODE_FAILED;
	}
	else
	{
	    if (ADM_ECODE_SUCCESSED != __adm_cvm_kvStorageInit()) 
		{
	        log_e("__adm_cvm_kvStorageInit failed");
            return ADM_ECODE_FAILED;
	    }
	}
    return ADM_ECODE_SUCCESSED;
}

static D32S __adm_cvmgtest_EnvironmentInit(void)
{
    D32S iret = ADM_ECODE_SUCCESSED;
    DCHAR acConfigFileFullPath[ADM_CVM_TEST_CONFIGJSON_FILEPATH] = {0};
    DCHAR *pcConfigFileDir = NULL;
    
    // 目录初始化
    printf("init CVM gtest environment start\n");
    adm_memset(&g_stOtherConfigData,0,sizeof(ADM_CVM_TEST_OTHER_INFO_CONF_T), sizeof(ADM_CVM_TEST_OTHER_INFO_CONF_T));

    // 获取json配置
    printf("Get CVM test report config\n");
    (void)adm_snprintf(acConfigFileFullPath, sizeof(acConfigFileFullPath), "%s%s", getcwd(pcConfigFileDir, ADM_CVM_TEST_CONFIGJSON_FILEPATH), "/./filecfg");
    printf("acConfigFileFullPath= %s\n", acConfigFileFullPath);
    adm_cvm_GtestSetOtherConfFileDir(acConfigFileFullPath);
    
    iret = __adm_cvm_TestInit();
    if(ADM_ECODE_SUCCESSED != iret)
    {
        log_e("__adm_cvm_TestInit iret is [%d]", iret);
    }

    iret = adm_cvm_GtestGetOtherInfoConfig(&g_stOtherConfigData);
    if(ADM_ECODE_SUCCESSED != iret)
    {
        log_e("adm_cvm_GtestGetOtherInfoConfig iret is [%d]", iret);
        return ADM_ECODE_FAILED;
    }

    log_i("init IRM gtest environment finish");
    return iret;
}

CCVMTest::CCVMTest()
{
}

CCVMTest::~CCVMTest()
{   
}

void CCVMTest::adm_cvm_check_test()
{
    if(ADM_ECODE_SUCCESSED != __adm_cvmgtest_EnvironmentInit())
    {
        log_e("__adm_cvmgtest_EnvironmentInit failed");
    }

    if (ADM_ECODE_SUCCESSED == adm_cvm_check(ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_USER)) {
        log_i("adm_cvm_check successed");
    } else {
        log_e("adm_cvm_check failed");
    }
}

TEST_F(adm_cvm_check_gtest, Normal)
{
    CCVMTest aa;
    aa.adm_cvm_check_test();
}

#ifndef _WIN32
int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
#endif