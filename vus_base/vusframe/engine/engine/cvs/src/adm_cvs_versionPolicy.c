#include "adm_cvs_log.h"
#include "adm_cvs_versionPolicy.h"
#include "adm_com_errorCode.h"
#include "adm_memory.h"
#include "adm_string.h"
#include "adm_com_kvstorage.h"
#include "adm_engine_common.h"
#include "json-c/json.h"
#include "adm_cvs_common.h"

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

static DBOOL adm_cvm_getJsonUint64Val(const json_object* obj, const DCHAR* key, D64U* val)
{
    if (NULL == obj || NULL == key) {
        return FALSE;
    }

    json_object* jsonStr = NULL;
    if ((json_bool)0 == json_object_object_get_ex(obj, key, &jsonStr)) {
        return FALSE;
    }
    *val = json_object_get_uint64(jsonStr);
    return TRUE;
}

ADM_CVS_VERSION_POLICY_T* adm_cvs_newPolicyBody(void)
{
    ADM_CVS_VERSION_POLICY_T* versionPolicy = NULL;
    versionPolicy = (ADM_CVS_VERSION_POLICY_T*)adm_malloc(sizeof(ADM_CVS_VERSION_POLICY_T));
    if (NULL != versionPolicy) {
        (void)adm_memset(versionPolicy, 0x00, sizeof(ADM_CVS_VERSION_POLICY_T), sizeof(ADM_CVS_VERSION_POLICY_T));
    }

    return versionPolicy;
}

void adm_cvs_deletePolicyBody(ADM_CVS_VERSION_POLICY_T* versionPolicy)
{
    if (NULL == versionPolicy) {
        return;
    }
    adm_free(versionPolicy->sessionId);
    adm_free(versionPolicy->versionNumber);
    adm_free(versionPolicy->extsStr);
    adm_free(versionPolicy->downloadMember.downloadPolicyObjectStr);
    adm_free(versionPolicy->installMember.installPolicyObjectStr);
    adm_ocs_deleteCheckHmiBody(versionPolicy->hmiMember);
    adm_free(versionPolicy);

    log_i("free end");
}

static D32S adm_cvs_getUcmHmiPolicyFileInfo(const DCHAR* fileInfoJsonStr,
    DCHAR** fileUrl,
    DCHAR** fileHash,
    D64S* fileSize)
{
    if (NULL == fileInfoJsonStr) {
        return ADM_ECODE_INVALID_PARAM;
    }

    D32S retVal = ADM_ECODE_SUCCESSED;
    json_object* fileInfoObject = NULL;
    DCHAR* url = NULL;
    DCHAR* hash = NULL;
    D64S size;
    do {
        log_i("ucm hmi policy file info = %s", fileInfoJsonStr);
        fileInfoObject = json_tokener_parse(fileInfoJsonStr);
        if (NULL == fileInfoObject) {
            log_e("fileInfoJsonStr to json_object. failed");
            retVal = ADM_ECODE_STRING_TO_JSON_FAILED;
            break;
        }

        log_i("get ucm hmi policy file url");
        if (TRUE != adm_cvm_getJsonStrVal(fileInfoObject, ADM_CVS_UCM_HMI_POLICY_FILE_URL_FIELD, &url)) {
            log_e("get url value from json_object. failed");
            retVal = ADM_ECODE_JSON_GET_STRING_FAILED;
            break;
        }
        log_i("get ucm hmi policy file hash");
        if (TRUE != adm_cvm_getJsonStrVal(fileInfoObject, ADM_CVS_UCM_HMI_POLICY_FILE_HASH_FIELD, &hash)) {
            log_e("get hash value from json_object. failed");
            retVal = ADM_ECODE_JSON_GET_STRING_FAILED;
            break;
        }
        log_i("get ucm hmi policy file size");
        if (TRUE != adm_cvm_getJsonInt64Val(fileInfoObject, ADM_CVS_UCM_HMI_POLICY_FILE_SIZE_FIELD, &size)) {
            log_e("get size value from json_object. failed");
            retVal = ADM_ECODE_JSON_GET_INT_FAILED;
            break;
        }
        *fileUrl = url;
        url = NULL;
        *fileHash = hash;
        hash = NULL;
        *fileSize = size;
        retVal = ADM_ECODE_SUCCESSED;
    } while (FALSE);

    (void)json_object_put(fileInfoObject);
    if (ADM_ECODE_SUCCESSED != retVal) {
        adm_free(url);
        adm_free(hash);
    }
    return retVal;
}

D32S adm_cvs_getVersionPolicy(ADM_CVS_VERSION_POLICY_T* versionPolicy)
{
    if (NULL == versionPolicy) {
        return ADM_ECODE_INVALID_PARAM;
    }

    DCHAR* val = NULL;
    size_t length = 0;
    json_object* respObject = NULL;
    json_object* respDataObject = NULL;
    json_object* extsObject = NULL;
    json_object* downloadPoilcyObject = NULL;
    json_object* installPolicyObject = NULL;
    json_object* hmiPolicyObject = NULL;
	json_object* privacyPolicyObject = NULL;
    DCHAR* ucmHmiFileInfoJsonStr = NULL;

    D32S retVal = ADM_ECODE_KV_OPEN_FAILED;

    if (NULL == adm_core_service_getBusinessDatabaseDb()) {
        return ADM_ECODE_KV_OPEN_FAILED;
    }
    ADM_KV_STORAGE_VALUE_TYPE_E eValType;
    if (FALSE
        == adm_com_kvstorage_get_string(adm_core_service_getBusinessDatabaseDb(),
            (const DCHAR*)ADM_CVS_KV_RESPMESSAGBODY_KEY,
            &val,
            &length,
            &eValType)) {
        return ADM_ECODE_KV_GET_STRING_FAILED;
    }

    do {
        log_i("policy = %s", val);
        respObject = json_tokener_parse(val);
        if (NULL == respObject) {
            log_e("str to son_object. failed");
            retVal = ADM_ECODE_STRING_TO_JSON_FAILED;
            break;
        }

        /*get exts*/
        log_i("get exts");
        if ((json_bool)0 != json_object_object_get_ex(respObject, ADM_COM_AIR_PROTOCOL_FIELD_EXT, &extsObject)) {
            versionPolicy->extsStr = adm_strdup(json_object_to_json_string(extsObject));
            log_i("resp mesaage body contains exts");
        } else {
            log_e("resp mesaage body is not contains exts");
        }

        /*get data*/
        log_i("get data");
        if ((json_bool)0 == json_object_object_get_ex(respObject, ADM_COM_AIR_PROTOCOL_FIELD_DATA, &respDataObject)) {
            log_e("get data value from json_object. failed");
            retVal = ADM_ECODE_JSON_GET_OBJECT_FAILED;
            break;
        }

        if (NULL == respDataObject) {
            log_e("parse resp body failed");
            retVal = ADM_ECODE_STRING_TO_JSON_FAILED;
            break;
        }
        /*get sessionId*/
        log_i("get sessionId");
        if (FALSE
            == adm_cvm_getJsonStrVal(
                respDataObject, ADM_COM_AIR_PROTOCOL_FIELD_SESSIONID, &(versionPolicy->sessionId))) {
            log_e("get sessionId value from json_object. failed");
            retVal = ADM_ECODE_JSON_GET_STRING_FAILED;
            break;
        }
        /*get taskId*/
        log_i("get taskId");
        if (FALSE
            == adm_cvm_getJsonInt64Val(respDataObject, ADM_COM_AIR_PROTOCOL_FIELD_TASKID, &(versionPolicy->taskId))) {
            log_e("get taskId value from json_object. failed");
            retVal = ADM_ECODE_JSON_GET_INT64_FAILED;
            break;
        }
        /*get strategyId*/
        log_i("get strategyId");
        if (FALSE
            == adm_cvm_getJsonInt64Val(
                respDataObject, ADM_COM_AIR_PROTOCOL_FIELD_STRATEGYID, &(versionPolicy->strategyId))) {
            log_e("get strategyId value from json_object. failed");
            retVal = ADM_ECODE_JSON_GET_INT64_FAILED;
            break;
        }
        /*option get versionNumber*/
        if (TRUE
            == adm_cvm_getJsonStrVal(
                respDataObject, ADM_COM_AIR_PROTOCOL_FIELD_VERSIONNUMBER, &(versionPolicy->versionNumber))) {
            log_i("get versionNumber");
        } else {
            log_i("no versionNumber field");
        }

        /*get upgradeMode*/
        log_i("get upgradeMode");
        if (FALSE
            == adm_cvm_getJsonIntVal(
                respDataObject, ADM_COM_AIR_PROTOCOL_FIELD_UPGRADEMODE, &(versionPolicy->upgradeMode))) {
            log_e("get upgradeMode value from json_object. failed");
            retVal = ADM_ECODE_JSON_GET_INT_FAILED;
            break;
        }
        /*get download poilcy*/
        log_i("get download poilcy");
        if ((json_bool)0
            == json_object_object_get_ex(
                respDataObject, ADM_COM_AIR_PROTOCOL_FIELD_DOWNLOADPOLICY, &downloadPoilcyObject)) {
            log_e("get data object from json_object. failed");
            retVal = ADM_ECODE_JSON_GET_OBJECT_FAILED;
            break;
        }
        versionPolicy->downloadMember.downloadPolicyObjectStr
            = adm_strdup(json_object_to_json_string(downloadPoilcyObject));
        /*get install poilcy*/
        log_i("get install poilcy");
        if ((json_bool)0
            == json_object_object_get_ex(
                respDataObject, ADM_COM_AIR_PROTOCOL_FIELD_INSTALLPOLICY, &installPolicyObject)) {
            log_e("get data object from json_object. failed");
            retVal = ADM_ECODE_JSON_GET_OBJECT_FAILED;
            break;
        }
        versionPolicy->installMember.installPolicyObjectStr
            = adm_strdup(json_object_to_json_string(installPolicyObject));
		/** get privacy Policy 隐私协议 */
		log_i("get privacy Policy");
		if ((json_bool)0 != json_object_object_get_ex(respDataObject, ADM_COM_AIR_PROTOCOL_FIELD_PRIVACYPOLICY, &privacyPolicyObject)) 
		{
            log_i("resp mesaage body contains privacyPolicy");
			versionPolicy->privacyMember = (ADM_OCS_CHECK_PRIVACY_MEMBER_T*)adm_malloc(sizeof(ADM_OCS_CHECK_PRIVACY_MEMBER_T));
			retVal = adm_ocs_checkProtocolPrivacyMember(privacyPolicyObject, versionPolicy->privacyMember);
			if (ADM_ECODE_SUCCESSED != retVal) 
			{
				log_e("adm_ocs_checkProtocolHmiMember return failed");
				break; //隐私协议下发了但内部信息获取失败，也返回失败
			}
		}
		else
		{
            log_e("resp mesaage body is not contains exts");
		}		
        /*get hmi poilcy*/
        log_i("get hmi poilcy");
        if ((json_bool)0 == json_object_object_get_ex(respDataObject, ADM_COM_AIR_PROTOCOL_FIELD_HMIPOLICY, &hmiPolicyObject)) {
            log_e("get data object from json_object. failed");
            retVal = ADM_ECODE_JSON_GET_OBJECT_FAILED;
            break;
        }

        versionPolicy->hmiMember = (ADM_OCS_CHECK_HMI_MEMBER_T*)adm_malloc(sizeof(ADM_OCS_CHECK_HMI_MEMBER_T));
        retVal = adm_ocs_checkProtocolHmiMember(hmiPolicyObject, versionPolicy->hmiMember);
        if (ADM_ECODE_SUCCESSED != retVal) {
            log_e("adm_ocs_checkProtocolHmiMember return failed");
            break;
        }

        log_i("get ucm hmi file package url hash size");
        if (TRUE
            == adm_com_kvstorage_get_string(adm_core_service_getBusinessDatabaseDb(),
                (const DCHAR*)ADM_CVS_KV_UCMHMIPOLICYFILEINFO_KEY,
                &ucmHmiFileInfoJsonStr,
                &length,
                &eValType)) {
            if (ADM_ECODE_SUCCESSED
                != adm_cvs_getUcmHmiPolicyFileInfo(ucmHmiFileInfoJsonStr,
                    &(versionPolicy->hmiMember->ucmHmiPolicyFileUrl),
                    &(versionPolicy->hmiMember->ucmHmiPolicyFileHash),
                    &(versionPolicy->hmiMember->ucmHmiPolicyFileSize))) {
                log_w("get ucm hmi policy file info failed");
            }
        } else {
            log_w("ucm has not downloaded the hmi policy package");
        }

        retVal = ADM_ECODE_SUCCESSED;
    } while (FALSE);
    adm_free(ucmHmiFileInfoJsonStr);
    (void)json_object_put(respObject);
    adm_free(val);

    return retVal;
}

D32S adm_cvs_getTaskId(D64S* taskId)
{
    D32S nRet = ADM_ECODE_FAILED;
    ADM_CVS_VERSION_POLICY_T* versionPolicy = NULL;

    versionPolicy = adm_cvs_newPolicyBody();
    if (versionPolicy == NULL) {
        nRet = ADM_ECODE_NO_MEMORY;
        log_e("no memory [versionPolicy]");
        return nRet;
    }

    nRet = adm_cvs_getVersionPolicy(versionPolicy);
    if (ADM_ECODE_SUCCESSED == nRet) {
        *taskId = versionPolicy->taskId;
        log_i("versionPolicy taskId, taskId[ %lld ]", *taskId);
    }
    adm_cvs_deletePolicyBody(versionPolicy);
    return nRet;
}

D32S adm_cvs_getStrategyId(D64S* strategyId)
{
    D32S nRet = ADM_ECODE_FAILED;
    ADM_CVS_VERSION_POLICY_T* versionPolicy = NULL;

    versionPolicy = adm_cvs_newPolicyBody();
    if (versionPolicy == NULL) {
        nRet = ADM_ECODE_NO_MEMORY;
        log_e("no memory [versionPolicy]");
        return nRet;
    }

    nRet = adm_cvs_getVersionPolicy(versionPolicy);
    if (ADM_ECODE_SUCCESSED == nRet) {
        *strategyId = versionPolicy->strategyId;
        log_i("versionPolicy strategyId, strategyId[ %lld ]", *strategyId);
    }
    adm_cvs_deletePolicyBody(versionPolicy);
    return nRet;
}

D32S adm_cvs_getSessionId(DCHAR** sessionId)
{
    D32S nRet = ADM_ECODE_FAILED;
    ADM_CVS_VERSION_POLICY_T* versionPolicy = NULL;

    versionPolicy = adm_cvs_newPolicyBody();
    if (versionPolicy == NULL) {
        nRet = ADM_ECODE_NO_MEMORY;
        log_e("no memory [versionPolicy]");
        return nRet;
    }

    nRet = adm_cvs_getVersionPolicy(versionPolicy);
    if (ADM_ECODE_SUCCESSED == nRet) {
        *sessionId = adm_strdup(versionPolicy->sessionId);
        log_i("versionPolicy sessionId, sessionId[ %s ]", *sessionId);
    }
    adm_cvs_deletePolicyBody(versionPolicy);
    return nRet;
}

D32S adm_cvs_getUpgradeMode(D32S* upgradeMode)
{
    D32S nRet = ADM_ECODE_FAILED;
    ADM_CVS_VERSION_POLICY_T* versionPolicy = NULL;

    versionPolicy = adm_cvs_newPolicyBody();
    if (versionPolicy == NULL) {
        nRet = ADM_ECODE_NO_MEMORY;
        log_e("no memory [versionPolicy]");
        return nRet;
    }

    nRet = adm_cvs_getVersionPolicy(versionPolicy);
    if (ADM_ECODE_SUCCESSED == nRet) {
        *upgradeMode = versionPolicy->upgradeMode;
        log_i("versionPolicy upgradeMode[ %d ]", *upgradeMode);
    }
    adm_cvs_deletePolicyBody(versionPolicy);
    return nRet;
}

D32S adm_cvs_getSilenceUpgradeTime(D64U* pSecondTime)
{
    D32S nRet = ADM_ECODE_FAILED;

    ADM_CVS_VERSION_POLICY_T* versionPolicy = NULL;

    versionPolicy = adm_cvs_newPolicyBody();
    if (versionPolicy == NULL) {
        nRet = ADM_ECODE_NO_MEMORY;
        log_e("no memory [versionPolicy]");
        return nRet;
    }

    nRet = adm_cvs_getVersionPolicy(versionPolicy);
    if (ADM_ECODE_SUCCESSED == nRet) 
    {
        if (NULL != versionPolicy && NULL != versionPolicy->installMember.installPolicyObjectStr)
        {
            json_object* pJson = NULL;
            pJson = json_tokener_parse(versionPolicy->installMember.installPolicyObjectStr);
            if (ADM_IS_NULL(pJson)) 
            {
                log_e("str to son_object. failed");
                nRet = ADM_ECODE_STRING_TO_JSON_FAILED;
                return nRet;
            }

            log_i("get ucm hmi policy file url");
            if (TRUE != adm_cvm_getJsonUint64Val(pJson, "upgradeAutoStart", pSecondTime)) 
            {
                log_e("get url value upgradeAutoStart null.");
                *pSecondTime = 0;
            }
        }
        else 
        {
            *pSecondTime = 0;
        }
        log_i("versionPolicy upgradeAutoStart[ %llu ]", *pSecondTime);
    }

    return nRet;
}

DVOID adm_cvs_getVehicleBigVersion(DCHAR** ppVehicleBigVersion)
{
    D32S nRet = ADM_ECODE_FAILED;
    ADM_CVS_VERSION_POLICY_T* versionPolicy = DNULL;

    versionPolicy = adm_cvs_newPolicyBody();
    if (versionPolicy == NULL) 
    {
        log_e("no memory [versionPolicy vehicleVersion]");
        *ppVehicleBigVersion = NULL;
        return;
    }
    
    nRet = adm_cvs_getVersionPolicy(versionPolicy);
    if (ADM_ECODE_SUCCESSED == nRet && NULL != versionPolicy->versionNumber)
    {
        *ppVehicleBigVersion = adm_strdup(versionPolicy->versionNumber);
        log_i("versionPolicy vehicleBigVersion [%s]", *ppVehicleBigVersion);
    }
    else
    {
        log_w("find vehicleBigVersion NULL.");
        *ppVehicleBigVersion = DNULL;
    }

    adm_cvs_deletePolicyBody(versionPolicy);

    return;
}

DVOID adm_cvs_getHmiVehicleVersion(DCHAR** ppVehicleVersion)
{
    D32S nRet = ADM_ECODE_FAILED;
    ADM_CVS_VERSION_POLICY_T* versionPolicy = NULL;

    versionPolicy = adm_cvs_newPolicyBody();
    if (versionPolicy == NULL) 
    {
        log_e("no memory [versionPolicy vehicleVersion]");
        *ppVehicleVersion = NULL;
        return;
    }

    nRet = adm_cvs_getVersionPolicy(versionPolicy);
    if (ADM_ECODE_SUCCESSED == nRet && NULL !=  versionPolicy->hmiMember 
            && NULL != versionPolicy->hmiMember->vehicleVersion) 
    {
        *ppVehicleVersion = adm_strdup(versionPolicy->hmiMember->vehicleVersion);
        log_i("versionPolicy vehicleVersion, vehicleVersion[ %s ]", *ppVehicleVersion);
    }
    else 
    {
        log_w("find vehicleVersion NULL.");
        *ppVehicleVersion = NULL;
    }

    adm_cvs_deletePolicyBody(versionPolicy);
    return;
}

D32S adm_cvs_getModeSwitch(D32S* pModeSwitch)
{
    D32S nRet = ADM_ECODE_FAILED;

    ADM_CVS_VERSION_POLICY_T* versionPolicy = NULL;

    versionPolicy = adm_cvs_newPolicyBody();
    if (versionPolicy == NULL) {
        nRet = ADM_ECODE_NO_MEMORY;
        log_e("no memory [versionPolicy]");
        return nRet;
    }

    nRet = adm_cvs_getVersionPolicy(versionPolicy);
    if (ADM_ECODE_SUCCESSED == nRet) 
    {
        if (NULL != versionPolicy && NULL != versionPolicy->installMember.installPolicyObjectStr)
        {
            json_object* pJson = NULL;
            pJson = json_tokener_parse(versionPolicy->installMember.installPolicyObjectStr);
            if (ADM_IS_NULL(pJson)) 
            {
                log_e("str to son_object. failed");
                nRet = ADM_ECODE_STRING_TO_JSON_FAILED;
                return nRet;
            }

            log_i("get ucm hmi policy file url");
            if (TRUE != adm_cvm_getJsonIntVal(pJson, "usbModeSwitch", pModeSwitch)) 
            {
                log_e("get url value usbModeSwitch null.");
                nRet = ADM_ECODE_STRING_TO_JSON_FAILED;
                return nRet;
            }
        }
        else 
        {
            nRet = ADM_ECODE_FAILED;
        }
    }

    return nRet;
}

DBOOL adm_cvs_getIsVerifyVersion()
{
    DCHAR* pVal = NULL;
    size_t length = 0;
    json_object* respObject = NULL;
    json_object* respDataObject = NULL;
    ADM_KV_STORAGE_VALUE_TYPE_E eValType;
    DBOOL bIsVerifyVersion = DTRUE;         ///< 默认为校验版本号
    D32S iVersionVersion = ADM_ECODE_FAILED;

    do {
        if (NULL == adm_core_service_getBusinessDatabaseDb()) 
        {
            log_e("Db handler is null.");
            break;
        }
        
        if (FALSE
            == adm_com_kvstorage_get_string(adm_core_service_getBusinessDatabaseDb(),
                (const DCHAR*)ADM_CVS_KV_RESPMESSAGBODY_KEY,
                &pVal,
                &length,
                &eValType)) 
        {
            log_e("Get cvm policy failed.");
            break;
        }

        respObject = json_tokener_parse(pVal);
        if (NULL == respObject) 
        {
            log_e("str to son_object. failed");
            break;
        }

        /*get data*/
        log_i("get data");
        if ((json_bool)0 == json_object_object_get_ex(respObject, ADM_COM_AIR_PROTOCOL_FIELD_DATA, &respDataObject)) 
        {
            log_e("get data value from json_object. failed");
            break;
        }

        if (NULL == respDataObject) 
        {
            log_e("parse resp body failed");
            break;
        }
        
        /*get isTargetValRequired*/
        log_i("get isTargetValRequired");
        if (FALSE
            == adm_cvm_getJsonIntVal(
                respDataObject, ADM_COM_AIR_PROTOCOL_FIELD_VERIFY_TARGET_VERSION, &iVersionVersion)) 
        {
            log_e("get verify targer version value from json_object. failed");
            break;
        }

        log_i("Get Verify target value[%d]", iVersionVersion);
        /** 升级后不校验版本号 */
        if (0 == iVersionVersion)
        {
            bIsVerifyVersion = DFALSE;
        }
    } while (FALSE);

    (void)json_object_put(respObject);
    adm_free(pVal);

    return bIsVerifyVersion;
}