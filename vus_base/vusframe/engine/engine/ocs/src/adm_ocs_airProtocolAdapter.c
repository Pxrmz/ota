#include "adm_ocs_log.h"
#include "adm_ocs_airProtocolAdapter.h"
#include <string.h>
#include "adm_com_modDefine.h"
#include "adm_com_errorCode.h"
#include "adm_debug.h"
#include "adm_string.h"
#include "adm_memory.h"
#include "adm_ocs_airProtocolSecurity.h"
#include "adm_iss_textAes.h"
#include "adm_iss_textRsa2048.h"
#include "openssl/rsa.h"
#include "adm_engine_common.h"
#include "adm_vrs_registerInfo.h"

typedef enum ADM_OCS_CHECHK_POLICY_TYPE_
{
    DOWNLOAD_POLICY = 1,
    INSTALL_POLICY,
    HMISHOW_POLICY
} ADM_OCS_CHECHK_POLICY_TYPE_T;

/*##################################################*/
static DBOOL adm_ocs_getJsonStrVal(const json_object* obj, const DCHAR* key, DCHAR** str)
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

static DBOOL adm_ocs_getJsonIntVal(const json_object* obj, const DCHAR* key, D32S* val)
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

static DBOOL adm_ocs_getJsonInt64Val(const json_object* obj, const DCHAR* key, D64S* val)
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

D32S adm_ocs_otaStatusCodeTransfer(const D32S statusCode)
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

/**
 * @brief 获取云端返回状态码对应的描述信息
 *
 * @param statusCode    [IN]    云端响应的状态码
 * @return const DCHAR*     NULL:Failed, None-NULL: 状态码对应的描述信息
 */
const DCHAR* adm_ocs_getErrMessage(const D32S statusCode)
{
    const DCHAR* errMessage = "undefined status code.";

    // TODO:
    log_i("Cloud response status code:%d", statusCode);
    switch (statusCode) {
    case ADM_OCS_AIR_STATUS_CODE_SUCCESSED:
        errMessage = "The cloud responded successfully.";
        break;
    case ADM_OCS_AIR_STATUS_CODE_NO_NEW_VERSION:
        errMessage = "No new version yet.";
        break;
    case ADM_OCS_AIR_STATUS_CODE_VEHICLE_NOT_EXIST:
        errMessage = "Vehicle does not exist.";
        break;
    case ADM_OCS_AIR_STATUS_CODE_OTA_DISABLED:
        errMessage = "OTA function was disabled.";
        break;
    case ADM_OCS_AIR_STATUS_CODE_CONFIG_NEED_UPDATE:
        errMessage = "Configuration information needs to be updated";
        break;
    case ADM_OCS_AIR_STATUS_CODE_CONFIG_NOT_EXIST:
        errMessage = "Configuration information does not exist.";
        break;
    case ADM_OCS_AIR_STATUS_CODE_PARAM_JSON_FORMAT_FAILED:
        errMessage = "Parameter json format exception.";
        break;
    case ADM_OCS_AIR_STATUS_CODE_PARAM_VERIFY_FAILED:
        errMessage = "Parameter validation failed.";
        break;
    case ADM_OCS_AIR_STATUS_CODE_SIGN_VERIFY_FAILED:
        errMessage = "Signature verification failed.";
        break;
    case ADM_OCS_AIR_STATUS_CODE_FILE_SAVED_FAILED:
        errMessage = "LOG file save abnormally.";
        break;
    case ADM_OCS_AIR_STATUS_CODE_DECRYPT_VERIFY_FAILED:
        errMessage = "Parameter decryption failed.";
        break;
    case ADM_OCS_AIR_STATUS_CODE_UPGRADE_TASK_STOPED:
        errMessage = "Task terminated.";
        break;
    case ADM_OCS_AIR_STATUS_CODE_SERVER_INTERNAL_EXCEPTION:
        errMessage = "Server internal exception.";
        break;
    case ADM_OCS_AIR_STATUS_CODE_NO_SHORT_TIME_ACCESS:
        errMessage = "Prohibit repeated access to the server in a short period of time.";
        break;
    default:
        break;
    }

    return errMessage;
}

/*##################################################*/

/*#######################init###########################*/
static adm_ocs_cloudRespStatus_F g_cloudRespStatusCb = NULL;

D32S adm_ocs_init(adm_ocs_cloudRespStatus_F cloudRespStatusCb)
{
    if (NULL == cloudRespStatusCb) {
        log_e("cloudRespStatusCb is NULL");
        return ADM_ECODE_INVALID_PARAM;
    }
    g_cloudRespStatusCb = cloudRespStatusCb;
    return ADM_ECODE_SUCCESSED;
}
/*#######################init###########################*/

static D32S adm_ocs_protocolSignEncrypt(json_object* protocolObject, DCHAR** protocolStrSignEncrypt)
{
    if (NULL == protocolObject) {
        log_e("protocolObject input param invalid");
        return ADM_ECODE_INVALID_PARAM;
    }

    D32S retVal = ADM_ECODE_FAILED;
    ADM_VRS_REGISTER_INFO_T* pRegisterInfo = NULL;
    json_object* stringObject = NULL;
    DCHAR* protocolSign = NULL;
    DCHAR* protocolStr = NULL;
    DCHAR* protocolStrSignEncryptTmp = NULL;

    // log_i("car-cloud protocol signature encryption");
    do {
        pRegisterInfo = adm_vrs_newRegisterInfo();
        if (NULL == pRegisterInfo) {
            log_e("adm_vrs_newRegisterInfo return failed");
            retVal = ADM_ECODE_APPLY_MEMORY_FAILED;
            break;
        }

        log_i("get deviceId secret from register info");
        retVal = adm_vrs_getRegisterInfo(pRegisterInfo);
        if (ADM_ECODE_SUCCESSED != retVal) {
            log_e("adm_vrs_getRegisterInfo return error[%d]", retVal);
            break;
        }

        // log_i("car-cloud protocol signature str");
        protocolSign = adm_ocs_ariProtocolSign(
            json_object_to_json_string(protocolObject), pRegisterInfo->secret, pRegisterInfo->deviceId);
        if (NULL == protocolSign) {
            retVal = ADM_ECODE_CAR_CLOUD_PROTOCOL_REQMESSAGE_SIGN_FAILED;
            log_e("protocolStr message sign failed");
            break;
        }

        // log_i("add sign to protocolObject");
        stringObject = json_object_new_string(protocolSign);
        if (NULL == stringObject) {
            log_e("protocolSign new json object failed");
            retVal = ADM_ECODE_NEW_JSON_OBJECT_FAILED;
            break;
        }
        if (0 != json_object_object_add(protocolObject, ADM_COM_AIR_PROTOCOL_FIELD_SIGN, stringObject)) {
            retVal = ADM_ECODE_JSON_ADD_STRING_FAILED;
            log_e("protocolObject add sign failed");
            break;
        }
        protocolStr = adm_strdup(json_object_to_json_string(protocolObject));
        if (NULL == protocolStr) {
            log_e("protocolObject to string add json object failed");
            retVal = ADM_ECODE_JSON_TO_STRING_FAILED;
            break;
        }
        log_i("car-cloud protocol str = %s", protocolStr);
#ifdef AIR_PROTOCOL_ENCRYPT
        log_i("car-cloud protocol encrypt str");
        protocolStrSignEncryptTmp = adm_iss_rsaPubEncrypt(pRegisterInfo->secret, protocolStr, RSA_PKCS1_PADDING);
        adm_free(protocolStr);
        if (NULL == protocolStrSignEncryptTmp) {
            retVal = ADM_ECODE_CAR_CLOUD_PROTOCOL_REQMESSAGE_ENCRYPT_FAILED;
            log_e("protocolStr message encrypt.failed");
            break;
        }
#else
        // log_i("car-cloud protocol no encrypt str");
        protocolStrSignEncryptTmp = protocolStr;
#endif
        /*end return success*/
        retVal = ADM_ECODE_SUCCESSED;
    } while(FALSE);
    adm_free(protocolSign);
    adm_vrs_delRegisterInfo(pRegisterInfo);
    if (ADM_ECODE_SUCCESSED == retVal) {
        *protocolStrSignEncrypt = protocolStrSignEncryptTmp;
        protocolStrSignEncryptTmp = NULL;
    } else {
        adm_free(protocolStrSignEncryptTmp);
    }
    return retVal;
}

static D32S adm_ocs_parseCloudRespStatus(const DCHAR* respBodyFromCloud, json_object** respObject, D32S* status)
{
    D32S retVal = ADM_ECODE_SUCCESSED;
    D32S respStatus;
    DCHAR* deCryptRespMessageBody = NULL;
    DCHAR* calSign = NULL;
    DCHAR* respSign = NULL;
    json_object* respObjectTmp = NULL;
    ADM_VRS_REGISTER_INFO_T* pRegisterInfo = NULL;

    do {
#if (defined AIR_PROTOCOL_ENCRYPT) || (defined AIR_PROTOCOL_SIGN)
        pRegisterInfo = adm_vrs_newRegisterInfo();
        if (NULL == pRegisterInfo) {
            log_e("call adm_vrs_newRegisterInfo failed");
            retVal = ADM_ECODE_APPLY_MEMORY_FAILED;
            break;
        }

        log_i("get deviceId secret from register info");
        if (ADM_ECODE_SUCCESSED != adm_vrs_getRegisterInfo(pRegisterInfo)) {
            log_e("call adm_vrs_getRegisterInfo failed");
            retVal = ADM_ECODE_FAILED;
            break;
        }
#endif
        /*decrypt*/
#ifdef AIR_PROTOCOL_ENCRYPT
        log_i("decrypt resp mesage");
        deCryptRespMessageBody = adm_iss_rsaPubDecrypt(pRegisterInfo->secret, respBodyFromCloud, RSA_PKCS1_PADDING);
        if (NULL == deCryptRespMessageBody) {
            retVal = ADM_ECODE_AES_ECB_DECRYPT_FAILED;
            log_e("decrypt failed");
            break;
        }
#else  // AIR_PROTOCOL_ENCRYPT
        deCryptRespMessageBody = adm_strdup(respBodyFromCloud);
#endif  // AIR_PROTOCOL_ENCRYPT

        log_i("respBodyFromCloud = %s", deCryptRespMessageBody);
        // log_i("deCryptRespMessageBody str to json");
        respObjectTmp = json_tokener_parse(deCryptRespMessageBody);
        if (NULL == respObjectTmp) {
            log_e("parse resp body failed");
            retVal = ADM_ECODE_STRING_TO_JSON_FAILED;
            break;
        }
#ifdef AIR_PROTOCOL_SIGN
        /*resp message get sign*/
        // log_i("resp message get sign");
        calSign = adm_ocs_ariProtocolSign(deCryptRespMessageBody, pRegisterInfo->secret, NULL);
        if (NULL == calSign) {
            retVal = ADM_ECODE_CHECK_MESSAGE_SIGN_FAILED;
            log_e("resp message get sign failed");
            break;
        }
        /*get sign*/
        // log_i("get sign from field");
        if (FALSE == adm_ocs_getJsonStrVal(respObjectTmp, ADM_COM_AIR_PROTOCOL_FIELD_SIGN, &respSign)) {
            log_e("get sign value from json_object. failed");
            retVal = ADM_ECODE_JSON_GET_STRING_FAILED;
            break;
        }
        log_i("resp sign = %s", respSign);

        /*compare sign*/
        // log_i("compare sign");
        if (0 != adm_memcmp(calSign, respSign, adm_strlen(respSign))) {
            retVal = ADM_ECODE_SIGN_VERIFY_FAILED;
            log_e("resp mesaage sign mismatch");
            break;
        }
        log_i("check sign passed");
#endif  // AIR_PROTOCOL_SIGN
        /*get status*/
        // log_i("get resp status");
        if (FALSE == adm_ocs_getJsonIntVal(respObjectTmp, ADM_COM_AIR_PROTOCOL_FIELD_STATUS, &respStatus)) {
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
    else 
    {
        (void)json_object_put(respObjectTmp);
        respObjectTmp = NULL;
    }
    adm_free(calSign);
    adm_free(respSign);
    adm_free(deCryptRespMessageBody);
#if (defined AIR_PROTOCOL_ENCRYPT) || (defined AIR_PROTOCOL_SIGN)
    adm_vrs_delRegisterInfo(pRegisterInfo);
#endif
    return retVal;
}

/**
 * @brief 注册的请求信息空间申请
 * @note 用于申请注册请求信息存储空间的接口，与adm_vrs_delRegisterRequestInfo函数匹配使用。
 *
 * @return ADM_OCS_REGISTER_REQUEST_INFO_T*     NULL:failed, Non-NULL:successed
 */
ADM_OCS_REGISTER_REQUEST_INFO_T* adm_ocs_newRegisterRequestInfo(void)
{
    ADM_OCS_REGISTER_REQUEST_INFO_T* pstRegisterRequestInfo = NULL;

    pstRegisterRequestInfo = adm_malloc(sizeof(ADM_OCS_REGISTER_REQUEST_INFO_T));
    if (NULL != pstRegisterRequestInfo) {
        (void)adm_memset(pstRegisterRequestInfo,
            0x00,
            sizeof(ADM_OCS_REGISTER_REQUEST_INFO_T),
            sizeof(ADM_OCS_REGISTER_REQUEST_INFO_T));
    }

    return pstRegisterRequestInfo;
}

/**
 * @brief 注册的请求信息空间释放.
 * @note
 * 用于释放注册请求信息存储空间的接口，与adm_ocs_newRegisterRequestInfo函数匹配使用。数据使用完成后，调用此接口释放空间
 *
 * @param pstRegisterRequestInfo   [In] ADM_OCS_REGISTER_REQUEST_INFO_T*     申请的空间地址
 *
 * @return void
 */
void adm_ocs_delRegisterRequestInfo(ADM_OCS_REGISTER_REQUEST_INFO_T* pstRegisterRequestInfo)
{
    if (NULL != pstRegisterRequestInfo) {
        if (NULL != pstRegisterRequestInfo->sdkVersion) {
            adm_free((pstRegisterRequestInfo->sdkVersion));
        }

        if (NULL != pstRegisterRequestInfo->vinCode) {
            adm_free((pstRegisterRequestInfo->vinCode));
        }

        if (NULL != pstRegisterRequestInfo->ext) {
            adm_free((pstRegisterRequestInfo->ext));
        }

        if (NULL != pstRegisterRequestInfo->fnBusinessCommCb) {
            pstRegisterRequestInfo->fnBusinessCommCb = NULL;
        }

        pstRegisterRequestInfo->timeStamp = 0;

        adm_free((pstRegisterRequestInfo));
    }
}

/**
 * @brief 注册的响应信息空间申请
 * @note 用于申请注册响应信息存储空间的接口，与adm_ocs_delRegisterResponseInfo函数匹配使用。
 *
 * @return ADM_OCS_REGISTER_RESPONSE_INFO_T*    NULL:failed, Non-NULL:successed
 */
ADM_OCS_REGISTER_RESPONSE_INFO_T* adm_ocs_newRegisterResponseInfo(void)
{
    ADM_OCS_REGISTER_RESPONSE_INFO_T* pstRegisterResponseInfo = NULL;

    pstRegisterResponseInfo = adm_malloc(sizeof(ADM_OCS_REGISTER_RESPONSE_INFO_T));
    if (NULL != pstRegisterResponseInfo) {
        (void)adm_memset(pstRegisterResponseInfo,
            0x00,
            sizeof(ADM_OCS_REGISTER_RESPONSE_INFO_T),
            sizeof(ADM_OCS_REGISTER_RESPONSE_INFO_T));
    }

    return pstRegisterResponseInfo;
}

/**
 * @brief 注册的响应信息空间释放
 * @note
 * 用于释放注册响应信息存储空间的接口，与adm_ocs_newRegisterResponseInfo函数匹配使用。数据使用完成后，调用此接口释放空间
 *
 * @param pstRegisterResponseInfo   [IN]    ADM_OCS_REGISTER_RESPONSE_INFO_T*   申请的空间地址
 */
void adm_ocs_delRegisterResponseInfo(ADM_OCS_REGISTER_RESPONSE_INFO_T* pstRegisterResponseInfo)
{
    if (NULL != pstRegisterResponseInfo)
    {
        if (NULL != pstRegisterResponseInfo->deviceId)
        {
            adm_free(pstRegisterResponseInfo->deviceId);
        }

        if (NULL != pstRegisterResponseInfo->secret)
        {
            adm_free(pstRegisterResponseInfo->secret);
        }

        if (NULL != pstRegisterResponseInfo->ext)
        {
            adm_free(pstRegisterResponseInfo->ext);
        }

        if (NULL != pstRegisterResponseInfo->pcVehlType)
        {
            adm_free(pstRegisterResponseInfo->pcVehlType);
        }

        adm_free(pstRegisterResponseInfo);
    }
}

static ADM_VRS_EVENT_ADD_CALLBACK_F g_fnEventAddCb = NULL;  //!< 添加事件的接口

/**
 * @brief 注册请求信息适配，包括：创建json数据，对json数据做签名，对请求数据做加密
 * @note 1. 注册接口数据加密使用的是RSA_2048非对称加密; 2.
非注册接口数据加密，使用的是注册响应的secret，AES256对称加密；
 *
 * @param pstRegisterRequestInfo    [IN]    const ADM_OCS_REGISTER_REQUEST_INFO_T* 注册请求数据结构体
 * @param secret                    [IN]    const DCHAR*    签名需要的secret，注册接口使用vin
//  * @param key                       [IN]    const DCHAR*    加密需要的key, 5.0平台使用
 * @return DCHAR*                   NULL:failed, Non-NULL:签名、加密后的注册请求信息
 */
DCHAR* adm_ocs_registerProtocolAdapter(const ADM_OCS_REGISTER_REQUEST_INFO_T* pstRegisterRequestInfo,
    const DCHAR* secret)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    json_object* jsonVal = NULL;
    DCHAR* registerProtocolStr = NULL;
    DCHAR* registerSignStr = NULL;
    json_object* jsonExt = NULL;
    DCHAR* registerProtocolEncStr = NULL;
    DCHAR eventContext[512] = {0};

    if (NULL == pstRegisterRequestInfo) {
        log_e("input param invalid");
        return NULL;
    }

    if (NULL != pstRegisterRequestInfo->fnEventAddCb) {
        g_fnEventAddCb = pstRegisterRequestInfo->fnEventAddCb;
    }

    jsonVal = json_object_new_object();
    if (NULL == jsonVal) {
        log_e("json_object_new_object failed");
        return NULL;
    }

    if (NULL != pstRegisterRequestInfo->vinCode) {
        iRet = json_object_object_add(
            jsonVal, ADM_COM_AIR_PROTOCOL_FIELD_VIN, json_object_new_string(pstRegisterRequestInfo->vinCode));
        if (ADM_ECODE_SUCCESSED != iRet) {
            log_w("json_object_object_add string %s failed, reason of error code is %d",
                pstRegisterRequestInfo->vinCode,
                iRet);
            // iRet = ADM_ECODE_JSON_ADD_STRING_FAILED;
        }
    } else {
        log_e("Parameter (pstRegisterRequestInfo->vinCode) verification failed, pointer invalid");
        // iRet = ADM_ECODE_INVALID_POINTER;
    }

    if (NULL != pstRegisterRequestInfo->sdkVersion) {
        iRet = json_object_object_add(
            jsonVal, ADM_COM_AIR_PROTOCOL_FIELD_SDKVERSION, json_object_new_string(pstRegisterRequestInfo->sdkVersion));
        if (ADM_ECODE_SUCCESSED != iRet) {
            log_w("json_object_object_add string %s failed, reason of error code is %d",
                pstRegisterRequestInfo->sdkVersion,
                iRet);
            // iRet = ADM_ECODE_JSON_ADD_STRING_FAILED;
        }
    } else {
        log_e("Parameter (pstRegisterRequestInfo->sdkVersion) verification failed, pointer invalid");
        // iRet = ADM_ECODE_INVALID_POINTER;
    }

    (void)json_object_object_add(
        jsonVal, ADM_COM_AIR_PROTOCOL_FIELD_TIMESTAMP, json_object_new_uint64((uint64_t)(pstRegisterRequestInfo->timeStamp)));

    if (NULL != pstRegisterRequestInfo->ext) {
        jsonExt = json_tokener_parse(pstRegisterRequestInfo->ext);
        if (NULL != jsonExt) {
            iRet = json_object_object_add(jsonVal, ADM_COM_AIR_PROTOCOL_FIELD_EXT, jsonExt);
            if (ADM_ECODE_SUCCESSED != iRet) {
                log_e("json_object_object_add string %s failed, reason of error code is %d",
                    pstRegisterRequestInfo->ext,
                    iRet);
                // iRet = ADM_ECODE_JSON_ADD_STRING_FAILED;
            }
        }
    }

    registerProtocolStr = adm_strdup(json_object_to_json_string(jsonVal));
    if (NULL != registerProtocolStr) {
        log_i("registerProtocolStr:%s, secret is vin:%s", registerProtocolStr,  secret);
        registerSignStr = adm_ocs_ariProtocolSign(registerProtocolStr, secret, NULL);

//        adm_free(registerProtocolStr);

        if (NULL == registerSignStr) {
            log_e("adm_ocs_ariProtocolSign failed");
            //!< ZI_004
            if (NULL != g_fnEventAddCb) {
                (void)adm_memset(eventContext, 0x00, sizeof(eventContext), sizeof(eventContext));
                (void)adm_snprintf(eventContext, sizeof(eventContext), "secret is vin:%s", secret);
                g_fnEventAddCb(3, 4, ADM_OCS_EVENT_LEVEL_ERROR, 0, eventContext);
            }
        } else {
            log_i("adm_ocs_airProtocolSign registerSignStr:%s", registerSignStr);
            (void)json_object_object_add(jsonVal, ADM_COM_AIR_PROTOCOL_FIELD_SIGN, json_object_new_string(registerSignStr));
            registerProtocolStr = adm_strdup(json_object_to_json_string(jsonVal));
            //!< ZI_003
            if (NULL != g_fnEventAddCb) {
                (void)adm_memset(eventContext, 0x00, sizeof(eventContext), sizeof(eventContext));
                (void)adm_snprintf(eventContext,
                    sizeof(eventContext),
                    "secret is vin:%s, signStr = %s, registerProtocolStr:%s",
                    secret,
                    registerSignStr,
                    registerProtocolStr);
                g_fnEventAddCb(3, 3, ADM_OCS_EVENT_LEVEL_TRACE, 0, eventContext);
            }
#ifdef AIR_PROTOCOL_ENCRYPT
            if (NULL != registerProtocolStr) {
                registerProtocolEncStr
                    = adm_iss_rsaPubEncrypt(PUBKEY_RSA2048, (D8U*)registerProtocolStr, RSA_PKCS1_PADDING);
                //!< 先释放注册请求数据，说明加密失败了
                adm_free(registerProtocolStr);
                if (NULL != registerProtocolEncStr) {
                    log_i("adm_iss_AES_ECB_PKCS7_Encrypt registerProtocolEncStr:%s", registerProtocolEncStr);
                    //!< 如果加密成功，将签名加密后的数据返回
                    registerProtocolStr = registerProtocolEncStr;
                    //!< ZI_005
                    if (NULL != g_fnEventAddCb) {
                        (void)adm_memset(eventContext, 0x00, sizeof(eventContext), sizeof(eventContext));
                        (void)adm_snprintf(eventContext, sizeof(eventContext), "Encrypt string:%s", registerProtocolStr);
                        g_fnEventAddCb(3, 5, ADM_OCS_EVENT_LEVEL_TRACE, 0, eventContext);
                    }
                } else {
                //!< 如果加密失败，返回为空
                TODO:
                    registerProtocolStr = NULL;
                }
            }
#endif  // AIR_PROTOCOL_ENCRYPT
        }
    }

    (void)json_object_put(jsonVal);

    adm_free(registerSignStr);
    return registerProtocolStr;
}

/**
 * @brief 获取签名使用的secret
 * @note 1. statusCode = 200，返回接口传入的公钥信息； 2. statusCode = 500，返回内置的公钥
 *
 * @param statusCode    [IN]    const D32S  OTA平台响应的消息code
 * @param secret        [IN]    DCHAR*      正常响应需要使用的签名secret
 * @return DCHAR*       返回验签使用的secret
 * @note 返回信息使用后需释放内存空间
 */
static DCHAR* adm_ocs_getResponseMessageSignSecret(const D32S statusCode, DCHAR* secret)
{
    DCHAR* signSecret = NULL;

    do {
        //!< Code = 500, 需要返回默认Secret
        if (ADM_OCA_AIR_STATUS_CODE_DEFAULT_SECRET == statusCode) {
            signSecret = adm_core_service_getOcsDefaultSecret();
        } else {
            if (NULL != secret) {
                signSecret = adm_strdup(secret);
            }
        }
    } while(FALSE);

    return signSecret;
}

/**
 * @brief 获取解密使用的key
 *
 * @param statusCode    [IN]    const D32S  OTA平台响应的消息code
 * @param key           [IN]    DCHAR*      正常响应需要使用解密需要的key
 * @return DCHAR*       返回解密使用的key
 */
static const DCHAR* adm_ocs_getResponseMessageDecryptKey(const D32S statusCode, DCHAR* key)
{
    if (500 == statusCode) {
        return PUBKEY_RSA2048;
    }

    return key;
}

/**
 * @brief 注册响应信息适配，包括：对响应数据的解密、验签
 * @note 1. 如注册接口,若服务端返回 Http 响应状态码为 200，则车端应使用约定的公钥解密报文，然后用 vin 验签；
 *       2. 如注册接口若服务端返回 Http 响应状态码为 500，则车端应使用约定的公钥解密报文，然后用约定的公钥验签；
 *
 * @param respBody                  [IN]    cosnt DCHAR*    注册接口响应的信息
 * @param statusCode                [IN]    const D32S      平台响应的状态码
 * @param secret                    [IN]    const DCHAR*
注册信息验签需要的secret，注册成功使用vin验签，否则使用约定的公钥验签
 * @param pstRegisterResponseInfo   [OUT]   ADM_OCS_REGISTER_RESPONSE_INFO_T*   注册响应信息
 * @return D32S                     0:successed, non-0: failed
 */
D32S adm_ocs_registerProtocolParse(const DCHAR* respBody,
    const D32S statusCode,
    const DCHAR* secret,
    ADM_OCS_REGISTER_RESPONSE_INFO_T* pstRegisterResponseInfo)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    DCHAR* pSecret = NULL;
    DCHAR* respDecStr = NULL;
    DCHAR* respMsgSignStr = NULL;
    json_object* jsonObj = NULL;
    json_object* dataObj = NULL;
    json_object* extObj = NULL;
    DCHAR* respSignField = NULL;
    D32S statusField = ADM_ECODE_UNKONW_STATUS_CODE;
    DCHAR eventContext[256] = {0};

    do {
        if (NULL == respBody || NULL == secret || NULL == pstRegisterResponseInfo) {
            iRet = ADM_ECODE_INVALID_PARAM;
            log_e("Input param respBody || secret || pstRegisterResponseInfo is nil");
            break;
        }

#ifdef AIR_PROTOCOL_ENCRYPT
        //!< ZI_012
        if (NULL != g_fnEventAddCb) {
            g_fnEventAddCb(3, 12, ADM_OCS_EVENT_LEVEL_TRACE, 0, NULL);
        }
        //!< 通过状态码获取secret，状态码 = 500 使用默认secret； 否则使用传入的secret
        pSecret = adm_ocs_getResponseMessageSignSecret(statusCode, (DCHAR*)secret);
        if (NULL == pSecret) {
            iRet = ADM_ECODE_SECRET_GET_FAILED;
            log_e("adm_ocs_getResponseMessageSignSecret failed, err:%d", iRet);
            break;
        }
        log_i("Response message secret is :%s", pSecret);
        //!< ZI_013
        if (NULL != g_fnEventAddCb) {
            (void)adm_memset(eventContext, 0x00, sizeof(eventContext), sizeof(eventContext));
            (void)adm_snprintf(eventContext, sizeof(eventContext), "statusCode:%d, secret:%s", pSecret);
            g_fnEventAddCb(3, 13, ADM_OCS_EVENT_LEVEL_TRACE, 0, eventContext);
        }

        //!< 注册接口非对称解密
        respDecStr = adm_iss_rsaPubDecrypt(PUBKEY_RSA2048, (D8U*)respBody, RSA_PKCS1_PADDING);
        if (NULL == respDecStr) {
            iRet = ADM_ECODE_REGISTER_RESPMESSAGE_DECRYPT_FAILED;
            log_e("adm_iss_rsaPubDecrypt failed, err:%d", iRet);
            break;
        }
        //!< ZI_014
        if (NULL != g_fnEventAddCb) {
            (void)adm_memset(eventContext, 0x00, sizeof(eventContext), sizeof(eventContext));
            (void)adm_snprintf(eventContext, sizeof(eventContext), "Dectypt string:%s", respDecStr);
            g_fnEventAddCb(3, 14, ADM_OCS_EVENT_LEVEL_TRACE, 0, eventContext);
        }
#else  // AIR_PROTOCOL_ENCRYPT
        respDecStr = adm_strdup(respBody);
#endif  // AIR_PROTOCOL_ENCRYPT
        log_i("Register response message body is:%s", respDecStr);

        //!< 将字符串转换为json对象
        jsonObj = json_tokener_parse(respDecStr);
        if (NULL == jsonObj) {
            iRet = ADM_ECODE_REGISTER_PARSE_MESSAGE_BODY_FAILED;
            log_e("json_tokener_parse failed, err:%d", iRet);
            break;
        }

        //!< 获取云端响应的状态码
        if (FALSE == adm_ocs_getJsonIntVal(jsonObj, ADM_COM_AIR_PROTOCOL_FIELD_STATUS, &statusField)) {
            log_e("get taskId value from json_object. failed");
            iRet = ADM_ECODE_JSON_GET_INT64_FAILED;
            //!< ZI_015
            if (NULL != g_fnEventAddCb) {
                (void)adm_memset(eventContext, 0x00, sizeof(eventContext), sizeof(eventContext));
                (void)adm_snprintf(eventContext, sizeof(eventContext), "statusField:%d, iRet:%d", statusField, iRet);
                g_fnEventAddCb(3, 15, ADM_OCS_EVENT_LEVEL_TRACE, iRet, eventContext);
            }
            break;
        }
        //!< ZI_015
        if (NULL != g_fnEventAddCb) {
            (void)adm_memset(eventContext, 0x00, sizeof(eventContext), sizeof(eventContext));
            (void)adm_snprintf(eventContext, sizeof(eventContext), "statusField:%d, iRet:%d", statusField, iRet);
            g_fnEventAddCb(3, 15, ADM_OCS_EVENT_LEVEL_TRACE, iRet, eventContext);
        }
        //!< 处理状态码
        if (ADM_OCS_AIR_STATUS_CODE_SUCCESSED != statusField) {
            if (NULL != g_cloudRespStatusCb) {
                //!< ZI_016
                if (NULL != g_fnEventAddCb) {
                    g_fnEventAddCb(3, 16, ADM_OCS_EVENT_LEVEL_TRACE, 0, NULL);
                }
                g_cloudRespStatusCb(statusField);
            } else {
                log_e("g_cloudRespStatusCb is NULL");
            }
        }

        //!< 转换云端响应状态码为车端本地状态码
        iRet = adm_ocs_otaStatusCodeTransfer(statusField);
        if (iRet != ADM_ECODE_SUCCESSED) {
            log_i("statusField:%d, iRet:%d", statusField, iRet);
            //!< ZI_017
            if (NULL != g_fnEventAddCb) {
                (void)adm_memset(eventContext, 0x00, sizeof(eventContext), sizeof(eventContext));
                (void)adm_snprintf(eventContext, sizeof(eventContext), "statusField:%d, iRet:%d", statusField, iRet);
                g_fnEventAddCb(3, 17, ADM_OCS_EVENT_LEVEL_TRACE, iRet, eventContext);
            }
            break;
        }
        //!< ZI_017
        if (NULL != g_fnEventAddCb) {
            (void)adm_memset(eventContext, 0x00, sizeof(eventContext), sizeof(eventContext));
            (void)adm_snprintf(eventContext, sizeof(eventContext), "statusField:%d, iRet:%d", statusField, iRet);
            g_fnEventAddCb(3, 17, ADM_OCS_EVENT_LEVEL_TRACE, 0, eventContext);
        }

        //!< 获取云端响应timestamp 获取不到也不报错
        if (FALSE == adm_ocs_getJsonInt64Val(jsonObj, ADM_COM_AIR_PROTOCOL_FIELD_TIMESTAMP, &(pstRegisterResponseInfo->itimestamp))) 
        {
            log_e("get timestamp value from json_object. failed");
        }
#ifdef AIR_PROTOCOL_ENCRYPT
        //!< 获取云端返回签名信息
        if (FALSE == adm_ocs_getJsonStrVal(jsonObj, ADM_COM_AIR_PROTOCOL_FIELD_SIGN, &respSignField)) {
            log_e("get sign value from json_object. failed");
            iRet = ADM_ECODE_JSON_GET_STRING_FAILED;
            break;
        }
        //!< ZI_018
        if (NULL != g_fnEventAddCb) {
            g_fnEventAddCb(3, 18, ADM_OCS_EVENT_LEVEL_TRACE, 0, eventContext);
        }
        //!< 云端响应的信息做本地签名
        respMsgSignStr = adm_ocs_ariProtocolSign(respDecStr, pSecret, NULL);
        if (NULL == respMsgSignStr) {
            log_e("adm_ocs_airProtocolSign failed");
            iRet = ADM_ECODE_REGISTER_RESPMESSAGE_SIGN_FAILED;
            break;
        }
        //!< 比较云端响应和本地生成签名信息
        if (0 != adm_strcmp(respSignField, respMsgSignStr)) {
            log_e("compare sign failed, Platform respSignFiled:%s, Vehicle generate respMsgSignStr:%s",
                respSignField,
                respMsgSignStr);
            //!< ZI_020
            if (NULL != g_fnEventAddCb) {
                (void)adm_memset(eventContext, 0x00, sizeof(eventContext), sizeof(eventContext));
                (void)adm_snprintf(eventContext, sizeof(eventContext), "expect:%s, real:%s", respSignField, respMsgSignStr);
                g_fnEventAddCb(3, 20, ADM_OCS_EVENT_LEVEL_ERROR, 0, eventContext);
            }
            break;
        }
        //!< ZI_019
        if (NULL != g_fnEventAddCb) {
            (void)adm_memset(eventContext, 0x00, sizeof(eventContext), sizeof(eventContext));
            (void)adm_snprintf(eventContext, sizeof(eventContext), "expect:%s, real:%s", respSignField, respMsgSignStr);
            g_fnEventAddCb(3, 19, ADM_OCS_EVENT_LEVEL_TRACE, 0, eventContext);
        }
#endif  // AIR_PROTOCOL_ENCRYPT
        //!< 获取data 对象，必填
        if ((json_bool)0 == json_object_object_get_ex(jsonObj, ADM_COM_AIR_PROTOCOL_FIELD_DATA, &dataObj)) {
            log_e("get data object from json_object. failed");
            iRet = ADM_ECODE_JSON_GET_OBJECT_FAILED;
            break;
        }

        //!< 获取云端响应deviceId，必填
        if (FALSE == adm_ocs_getJsonStrVal(
                dataObj, ADM_COM_AIR_PROTOCOL_FIELD_DEVICEID, &(pstRegisterResponseInfo->deviceId))) {
            log_e("get sign value from json_object. failed");
            iRet = ADM_ECODE_JSON_GET_STRING_FAILED;
            break;
        }

        //!< 获取云端响应secret，必填
        if (FALSE == adm_ocs_getJsonStrVal(dataObj, ADM_COM_AIR_PROTOCOL_FIELD_SECRET, &(pstRegisterResponseInfo->secret))) {
            log_e("get sign value from json_object. failed");
            iRet = ADM_ECODE_JSON_GET_STRING_FAILED;
            break;
        }

        //!< 获取云端响应vehicleType，必填
        if (FALSE == adm_ocs_getJsonStrVal(dataObj, "modelCode", &(pstRegisterResponseInfo->pcVehlType))) {
            log_e("get modelCode value from json_object. failed");
            iRet = ADM_ECODE_JSON_GET_STRING_FAILED;
            break;
        }

        //!< 获取云端响应扩展字段 object，非必填
        if ((json_bool)0 == json_object_object_get_ex(dataObj, ADM_COM_AIR_PROTOCOL_FIELD_EXT, &extObj)) {
            log_e("Ext field, get data object from json_object. failed");
        } else {
            //!< 将扩展字段object转换为字符串
            pstRegisterResponseInfo->ext = adm_strdup(json_object_to_json_string(extObj));
        }
    } while(FALSE);

    //!< 释放内存
    if (NULL != respMsgSignStr) {
        adm_free(respMsgSignStr);
    }
    if (NULL != respSignField) {
        adm_free(respSignField);
    }
    // TODO: Crash
    if (NULL != jsonObj) {
        (void)json_object_put(jsonObj);
    }
    if (NULL != respDecStr) {
        adm_free(respDecStr);
    }
    if (NULL != pSecret) {
        adm_free(pSecret);
    }

    return iRet;
}

/*##################################################*/

ADM_OCS_CHECK_REQ_ADAPTER_T* adm_ocs_newCheckReqAdapterBody(D32S cmdId,
    D64S timestamp,
    const DCHAR* netType,
    const DCHAR* versionNumber,
    const DCHAR* sdkVersion,
    const DCHAR* confVersion,
    const DCHAR* allEcuInfoObjectStr,
    const DCHAR* allModuleVerInfoStr,
    const DCHAR* vehicleVersion,
    const D64S ulTaskId,
    const DCHAR* extsObjectStr,
    ADM_OCS_BUSINESS_COMM_INTERFACE_CALLBACK_F fnBusinessCommCb)
{
    if (NULL == netType || NULL == sdkVersion || NULL == confVersion || NULL == allEcuInfoObjectStr || NULL == fnBusinessCommCb) {
        return NULL;
    }

    ADM_OCS_CHECK_REQ_ADAPTER_T* checkReq = NULL;
    checkReq = (ADM_OCS_CHECK_REQ_ADAPTER_T*)adm_malloc(sizeof(ADM_OCS_CHECK_REQ_ADAPTER_T));
    if (NULL != checkReq) {
        (void)adm_memset(checkReq, 0x00, sizeof(ADM_OCS_CHECK_REQ_ADAPTER_T), sizeof(ADM_OCS_CHECK_REQ_ADAPTER_T));

        checkReq->ulTaskId = ulTaskId;
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

        checkReq->vehicleVersion = adm_strdup(vehicleVersion);

        if (NULL != extsObjectStr) {
            checkReq->extsObjectStr = adm_strdup(extsObjectStr);
        } else {
            checkReq->extsObjectStr = NULL;
        }
        checkReq->fnBusinessCommCb = fnBusinessCommCb;
    }
    return checkReq;
}

void adm_ocs_deleteCheckReqAdapterBody(ADM_OCS_CHECK_REQ_ADAPTER_T* checkReq)
{
    if (NULL == checkReq)
    {
        return;
    }
    adm_free(checkReq->netType);
    adm_free(checkReq->versionNumber);
    adm_free(checkReq->sdkVersion);
    adm_free(checkReq->confVersion);
    adm_free(checkReq->allEcuInfoObjectStr);
    adm_free(checkReq->allModuleVerInfoStr);
    adm_free(checkReq->vehicleVersion);
    adm_free(checkReq->extsObjectStr);
    checkReq->fnBusinessCommCb = NULL;
    adm_free(checkReq);
    return;
}

void adm_ocs_deleteCheckRespEcuInfoBody(ADM_OCS_CHECK_ECU_INFO_MEMBER_T* checkRespEcuInfo)
{
    if (NULL == checkRespEcuInfo)
    {
        return;
    }
    /*option*/
    if (NULL != checkRespEcuInfo->ecuPartNum)
    {
        adm_free(checkRespEcuInfo->ecuPartNum);
    }
    adm_free(checkRespEcuInfo->requestId);
    adm_free(checkRespEcuInfo->swDid);
    adm_free(checkRespEcuInfo->ecuVersion);
    adm_free(checkRespEcuInfo);
    return;
}

void adm_ocs_deleteCheckHmiBody(ADM_OCS_CHECK_HMI_MEMBER_T* checkHmiMember)
{
    if (NULL == checkHmiMember)
    {
        return;
    }
    adm_free(checkHmiMember->ucmHmiPolicyFileUrl);
    adm_free(checkHmiMember->ucmHmiPolicyFileHash);
    adm_free(checkHmiMember->hmiShowFileUrl);
    adm_free(checkHmiMember->hmiShowFileHash);
    adm_free(checkHmiMember->hmiShowFileName);
    adm_free(checkHmiMember->vehicleVersion);
    for (D16U i = 0; i < checkHmiMember->hmiEcuInfoArrayLength; ++i) {
            adm_ocs_deleteCheckRespEcuInfoBody(checkHmiMember->hmiEcuInfoArray[i]);
    }
    adm_free(checkHmiMember->hmiEcuInfoArray);
    if (NULL != checkHmiMember->fileSecurityInfo) {
        adm_com_delFileSecurityInfo(&checkHmiMember->fileSecurityInfo);
    }
    adm_free(checkHmiMember);
    return;
}

ADM_OCS_CHECK_RESP_ADAPTER_T* adm_ocs_newCheckRespAdapterBody(void)
{
    ADM_OCS_CHECK_RESP_ADAPTER_T* checkResp = NULL;
    checkResp = (ADM_OCS_CHECK_RESP_ADAPTER_T*)adm_malloc(sizeof(ADM_OCS_CHECK_RESP_ADAPTER_T));
    if (NULL != checkResp) {
        (void)adm_memset(checkResp, 0x00, sizeof(ADM_OCS_CHECK_RESP_ADAPTER_T), sizeof(ADM_OCS_CHECK_RESP_ADAPTER_T));
    }
    return checkResp;
}

void adm_ocs_deleteCheckRespAdapterBody(ADM_OCS_CHECK_RESP_ADAPTER_T* checkResp)
{
    if (NULL == checkResp)
    {
        return;
    }
    adm_free(checkResp->respMessageBodyStr);
    adm_free(checkResp->sessionId);
    adm_free(checkResp->extsObjectStr);
    adm_ocs_deleteCheckHmiBody(checkResp->hmiMember);
    adm_free(checkResp);
    return;
}

static D32S adm_ocs_reqDataSetField(json_object* checkVersionObject, const ADM_OCS_CHECK_REQ_ADAPTER_T* checkReq)
{
    D32S retVal = ADM_ECODE_SUCCESSED;
    json_object* stringObject = NULL;
    json_object* d64Object = NULL;
    json_object* d32Object = NULL;
    json_object* networkInfoObject = NULL;
    json_object* ecuInfoObject = NULL;
    json_object* allModuleVerObject = NULL;
    json_object* extsObject = NULL;
    json_object* taskidObject = NULL;

    do {
        /*set cmdId*/
        log_i("set cmdId");
        d32Object = json_object_new_int(checkReq->cmdId);
        if (NULL == d32Object) {
            log_e("cmdId new json object failed");
            retVal = ADM_ECODE_NEW_JSON_OBJECT_FAILED;
            break;
        }
        if (0 != json_object_object_add(checkVersionObject, ADM_COM_AIR_PROTOCOL_FIELD_CMDID, d32Object)) {
            log_e("cmdId add json object failed");
            retVal = ADM_ECODE_JSON_ADD_STRING_FAILED;
            break;
        }
        /*set timestamp*/
        log_i("set timestamp");
        d64Object = json_object_new_int64(checkReq->timestamp);
        if (NULL == d64Object) {
            log_e("timestamp new json object failed");
            retVal = ADM_ECODE_NEW_JSON_OBJECT_FAILED;
            break;
        }
        if (0 != json_object_object_add(checkVersionObject, ADM_COM_AIR_PROTOCOL_FIELD_TIMESTAMP, d64Object)) {
            log_e("timestamp add json object failed");
            retVal = ADM_ECODE_JSON_ADD_INT64_FAILED;
            break;
        }
        /*set sdkVersion*/
        log_i("set sdkVersion");
        stringObject = NULL;
        if (NULL != checkReq->sdkVersion)
        {
            stringObject = json_object_new_string(checkReq->sdkVersion);
        }
        if (NULL == stringObject) {
            log_e("sdkVersion new json object failed");
            retVal = ADM_ECODE_NEW_JSON_OBJECT_FAILED;
            break;
        }
        if (0 != json_object_object_add(checkVersionObject, ADM_COM_AIR_PROTOCOL_FIELD_SDKVERSION, stringObject)) {
            log_e("sdkVersion add json object failed");
            retVal = ADM_ECODE_JSON_ADD_STRING_FAILED;
            break;
        }

        if (NULL != checkReq->versionNumber) {
            log_i("set versionNumber");
            stringObject = json_object_new_string(checkReq->versionNumber);
            if (NULL != stringObject) {
                if (0 != json_object_object_add(
                        checkVersionObject, ADM_COM_AIR_PROTOCOL_FIELD_VERSIONNUMBER, stringObject)) {
                    log_e("versionNumber add json object failed");
                }
            } else {
                log_e("versionNumber new json object failed");
            }
        }
        /*set confVersion*/
        log_i("set ecuConfVersion");
        stringObject = NULL;
        if (NULL != checkReq->confVersion)
        {
            stringObject = json_object_new_string(checkReq->confVersion);
        }
        if (NULL == stringObject) {
            log_e("ecuConfVersion new json object failed");
            retVal = ADM_ECODE_NEW_JSON_OBJECT_FAILED;
            break;
        }
        if (0 != json_object_object_add(checkVersionObject, ADM_COM_AIR_PROTOCOL_FIELD_ECUCONFVERSION, stringObject)) {
            log_e("ecuConfVersion add json object failed");
            retVal = ADM_ECODE_JSON_ADD_STRING_FAILED;
            break;
        }
        /*network info*/
        log_i("set network info");
        networkInfoObject = json_object_new_object();
        if (NULL == networkInfoObject) {
            log_e("networkInfoObject new json object failed");
            retVal = ADM_ECODE_NEW_JSON_OBJECT_FAILED;
            break;
        }
        /*netType*/
        stringObject = NULL;
        if (NULL != checkReq->netType)
        {
            stringObject = json_object_new_string(checkReq->netType);
        }
        if (NULL == stringObject) {
            log_e("netType new json object failed");
            retVal = ADM_ECODE_NEW_JSON_OBJECT_FAILED;
            break;
        }
        if (0 != json_object_object_add(networkInfoObject, ADM_COM_AIR_PROTOCOL_FIELD_NETTYPE, stringObject)) {
            log_e("netType add json object failed");
            retVal = ADM_ECODE_JSON_ADD_STRING_FAILED;
            break;
        }
        log_i("set networkInfoObject to checkVersionObject");
        if (0 != json_object_object_add(checkVersionObject, ADM_COM_AIR_PROTOCOL_FIELD_NETWORKINFO, networkInfoObject)) {
            log_e("networkInfoObject add json object failed");
            retVal = ADM_ECODE_JSON_ADD_STRING_FAILED;
            break;
        }
        /*exts*/
        if (NULL != checkReq->extsObjectStr) {
            log_i("set exts to checkVersionObject");
            extsObject = json_tokener_parse(checkReq->extsObjectStr);
            if (NULL != extsObject) {
                if (0 != json_object_object_add(checkVersionObject, ADM_COM_AIR_PROTOCOL_FIELD_EXT, extsObject)) {
                    log_w("exts add json object.failed");
                }
            } else {
                log_w("exts str to object.failed");
            }
        }
        /*set ecu info object*/
        ecuInfoObject = json_tokener_parse(checkReq->allEcuInfoObjectStr);
        if (NULL == ecuInfoObject) {
            log_e("ecu info str to object.failed");
            retVal = ADM_ECODE_STRING_TO_JSON_FAILED;
            break;
        }
        if (0 != json_object_object_add(checkVersionObject, ADM_COM_AIR_PROTOCOL_FIELD_ECUS, ecuInfoObject)) {
            log_e("ecu info  add json object.failed");
            retVal = ADM_ECODE_JSON_ADD_STRING_FAILED;
            break;
        }

        /*set vehicleVersion*/
        log_i("set vehicleVersion");
        stringObject = NULL;
        if (NULL != checkReq->vehicleVersion)
        {
            stringObject = json_object_new_string(checkReq->vehicleVersion);
        }
        if (NULL == stringObject) 
        {
            log_e("vehicleVersion new json object failed");
            retVal = ADM_ECODE_NEW_JSON_OBJECT_FAILED;
            break;
        }
        if (0 != json_object_object_add(checkVersionObject, ADM_COM_AIR_PROTOCOL_FIELD_VEHICLEVERSION, stringObject)) {
            log_e("vehicleVersion add json object failed");
            retVal = ADM_ECODE_JSON_ADD_STRING_FAILED;
            break;
        }

        /* set all module version info object 非必填字段 */
        if (NULL != checkReq->allModuleVerInfoStr) {
            allModuleVerObject = json_tokener_parse(checkReq->allModuleVerInfoStr);
            if (NULL == allModuleVerObject) {
                log_w("all module version info str to object. failed");
            } else {
                if (0 != json_object_object_add(
                        checkVersionObject, ADM_COM_AIR_PROTOCOL_FIELD_MODULE_VERS, allModuleVerObject)) {
                    log_e("all module version info add json object. failed");
                }
            }
        }

        /** TSP推送触发检测时，且任务ID不为0 */
        if (0 != checkReq->ulTaskId && 3 == checkReq->cmdId)
        {
            log_i("set taskid");
            taskidObject = json_object_new_int64(checkReq->ulTaskId);
            if (NULL == taskidObject)
            {
                log_e("taskid new json object failed");
                retVal = ADM_ECODE_NEW_JSON_OBJECT_FAILED;
                break;
            }
            if (0 != json_object_object_add(checkVersionObject, ADM_COM_AIR_PROTOCOL_FIELD_TASKID, taskidObject))
            {
                log_e("taskid add json object failed");
                retVal = ADM_ECODE_JSON_ADD_STRING_FAILED;
                break;
            }
        }

    } while(FALSE);
    return retVal;
}

D32S adm_ocs_checkProtocolAdapter(const ADM_OCS_CHECK_REQ_ADAPTER_T* checkReq, DCHAR** adapterCheckReq)
{
    if (NULL == checkReq) {
        log_e("input param invalid");
        return ADM_ECODE_INVALID_PARAM;
    }

    json_object* checkVersionObject = NULL;
    D32S retVal = ADM_ECODE_SUCCESSED;

    checkVersionObject = json_object_new_object();
    if (NULL == checkVersionObject) {
        log_e("checkVersionObject new object.failed");
        return ADM_ECODE_INVALID_PARAM;
    }
    log_i("creat req message body");

    do {
        retVal = adm_ocs_reqDataSetField(checkVersionObject, checkReq);
        if (ADM_ECODE_SUCCESSED != retVal) {
            log_e("call adm_ocs_reqDataSetField. failed");
            break;
        }
        retVal = adm_ocs_protocolSignEncrypt(checkVersionObject, adapterCheckReq);
        if (ADM_ECODE_SUCCESSED != retVal) {
            log_e("adm_ocs_protocolSignEncrypt return error[%d]", retVal);
            break;
        }
    } while(FALSE);
    (void)json_object_put(checkVersionObject);
    return retVal;
}
/*check protocol adapter########################################################################################*/

/*parse check resp##################################################*/
static D32S adm_ocs_respDataGetHmiPolicyFiled(ADM_OCS_CHECK_RESP_ADAPTER_T* checkResp,
    json_object* respObject,
    json_object* dataObject)
{
    D32S retVal = ADM_ECODE_SUCCESSED;
    json_object* hmiPolicyFileObject = NULL;
    json_object* hmiFileObject = NULL;
	json_object* privacyPolicyFileObject = NULL;
    json_object* privacyFileObject = NULL;
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
        if (FALSE == adm_ocs_getJsonStrVal(dataObject, ADM_COM_AIR_PROTOCOL_FIELD_SESSIONID, &(checkResp->sessionId))) {
            log_e("get sessionId value from json_object. failed");
            retVal = ADM_ECODE_JSON_GET_STRING_FAILED;
            break;
        }
        /*get upgradeMode*/
        log_i("get upgradeMode");
        if (FALSE == adm_ocs_getJsonIntVal(dataObject, ADM_COM_AIR_PROTOCOL_FIELD_UPGRADEMODE, &(checkResp->upgradeMode))) {
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
		log_i("get privacyPolicy object");
		{
            if ((json_bool)0 == json_object_object_get_ex(dataObject, ADM_COM_AIR_PROTOCOL_FIELD_PRIVACYPOLICY, &privacyPolicyFileObject)) {
                log_e("get privacyPolicy object. failed");
				retVal = ADM_ECODE_SUCCESSED;
                break;
            }

            checkResp->privacyMember = (ADM_OCS_CHECK_HMI_MEMBER_T*)adm_malloc(sizeof(ADM_OCS_CHECK_HMI_MEMBER_T));
            retVal = adm_ocs_checkProtocolPrivacyMember(privacyPolicyFileObject, checkResp->privacyMember);
            if (ADM_ECODE_SUCCESSED != retVal) {
                log_e("adm_ocs_checkProtocolPrivacyMember return failed");
                break;
            }
        }

    } while(FALSE);
    return retVal;
}

D32S adm_ocs_checkProtocolHmiMember(json_object* hmiPolicyFileObject, ADM_OCS_CHECK_HMI_MEMBER_T* hmiMember)
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
        if (FALSE == adm_ocs_getJsonStrVal(hmiFileObject, ADM_COM_AIR_PROTOCOL_FIELD_URL, &(hmiMember->hmiShowFileUrl))) {
            retVal = ADM_ECODE_JSON_GET_STRING_FAILED;
            log_e("get fileUrl value from json_object. failed");
            break;
        }
        log_i("get hmiPolicy fileHash");
        if (FALSE == adm_ocs_getJsonStrVal(hmiFileObject, ADM_COM_AIR_PROTOCOL_FIELD_HASH, &(hmiMember->hmiShowFileHash))) {
            retVal = ADM_ECODE_JSON_GET_STRING_FAILED;
            log_e("get fileHash value from json_object. failed");
            break;
        }
        log_i("get hmiPolicy fileSize");
        if (FALSE == adm_ocs_getJsonInt64Val(hmiFileObject, ADM_COM_AIR_PROTOCOL_FIELD_SIZE, &(hmiMember->hmiShowFileSize))) {
            retVal = ADM_ECODE_JSON_GET_INT_FAILED;
            log_e("get fileSize value from json_object. failed");
            break;
        }
        log_i("get hmiPolicy fileName");
        if (FALSE == adm_ocs_getJsonStrVal(hmiFileObject, ADM_COM_AIR_PROTOCOL_FIELD_FILENAME, &(hmiMember->hmiShowFileName))) {
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
        if (FALSE == adm_ocs_getJsonInt64Val(
                hmiVehicleVersionInfoObject, ADM_COM_AIR_PROTOCOL_FIELD_TOTALSIZE, &(hmiMember->totalSize))) {
            retVal = ADM_ECODE_JSON_GET_INT64_FAILED;
            log_e("get hmiInfo totalSize from json_object. failed");
            break;
        }

        /*option*/
        if (TRUE == adm_ocs_getJsonStrVal(
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
            = (ADM_OCS_CHECK_ECU_INFO_MEMBER_T**)adm_malloc(sizeof(ADM_OCS_CHECK_ECU_INFO_MEMBER_T*) * (DSIZE)ecuInfoLen);

        json_object* hmiEcuInfo = NULL;
        for (D32S i = 0; i < ecuInfoLen; ++i) {
            hmiEcuInfo = json_object_array_get_idx(hmiEcuInfosObject, (size_t)i);

            hmiMember->hmiEcuInfoArray[i]
                = (ADM_OCS_CHECK_ECU_INFO_MEMBER_T*)adm_malloc(sizeof(ADM_OCS_CHECK_ECU_INFO_MEMBER_T));

            log_i("get hmiPolicy hmiInfo ecuReqId");
            if (FALSE == adm_ocs_getJsonStrVal(
                    hmiEcuInfo, ADM_COM_AIR_PROTOCOL_FIELD_ECUREQID, &(hmiMember->hmiEcuInfoArray[i]->requestId))) {
                retVal = ADM_ECODE_JSON_GET_STRING_FAILED;
                log_e("get hmiInfo hmiInfo ecuReqId from json_object. failed");
                break;
            }

            /*option ecuPartNum*/
            if (TRUE == adm_ocs_getJsonStrVal(
                    hmiEcuInfo, ADM_COM_AIR_PROTOCOL_FIELD_ECUPARTNUM, &(hmiMember->hmiEcuInfoArray[i]->ecuPartNum))) {
                log_i("get hmiPolicy hmiInfo ecuPartNum");
            }

            log_i("get hmiPolicy hmiInfo ecuSwDid");
            if (FALSE == adm_ocs_getJsonStrVal(
                    hmiEcuInfo, ADM_COM_AIR_PROTOCOL_FIELD_ECUSWDID, &(hmiMember->hmiEcuInfoArray[i]->swDid))) {
                retVal = ADM_ECODE_JSON_GET_STRING_FAILED;
                log_e("get hmiInfo hmiInfo ecuSwDid from json_object. failed");
                break;
            }

            log_i("get hmiPolicy hmiInfo ecuVersion");
            if (FALSE == adm_ocs_getJsonStrVal(
                    hmiEcuInfo, ADM_COM_AIR_PROTOCOL_FIELD_ECUVERSION, &(hmiMember->hmiEcuInfoArray[i]->ecuVersion))) {
                retVal = ADM_ECODE_JSON_GET_STRING_FAILED;
                log_e("get hmiInfo hmiInfo ecuVersion from json_object. failed");
                break;
            }

            log_i("get hmiPolicy hmiInfo ecuSize");
            if (FALSE == adm_ocs_getJsonInt64Val(
                    hmiEcuInfo, ADM_COM_AIR_PROTOCOL_FIELD_ECUSIZE, &(hmiMember->hmiEcuInfoArray[i]->ecuSize))) {
                retVal = ADM_ECODE_JSON_GET_INT64_FAILED;
                log_e("get hmiInfo hmiInfo ecuSize from json_object. failed");
                break;
            }
        }
    } while(FALSE);
    return retVal;
}

D32S adm_ocs_checkProtocolPrivacyMember(json_object* privacyPolicyObject, ADM_OCS_CHECK_PRIVACY_MEMBER_T* privacyMember)
{
    if (NULL == privacyPolicyObject || NULL ==privacyMember) {
        log_e("input param invalid");
        return ADM_ECODE_INVALID_PARAM;
    }
    D32S retVal = ADM_ECODE_SUCCESSED;
    json_object* privacyFileObject = NULL;
    json_object* privacyInfoObject = NULL;
    json_object* privacyVehicleVersionInfoObject = NULL;
    json_object* privacyEcuInfosObject = NULL;

    do {
        if ((json_bool)0 == json_object_object_get_ex(privacyPolicyObject, ADM_COM_AIR_PROTOCOL_FIELD_PRIVACYFILE, &privacyFileObject)) {
            retVal = ADM_ECODE_JSON_GET_OBJECT_FAILED;
            log_e("get privacyFile object. failed");
            break;
        }
        log_i("get privacyPolicy fileUrl");
        if (FALSE == adm_ocs_getJsonStrVal(privacyFileObject, ADM_COM_AIR_PROTOCOL_FIELD_URL, &(privacyMember->privacyShowFileUrl))) {
            retVal = ADM_ECODE_JSON_GET_STRING_FAILED;
            log_e("get fileUrl value from json_object. failed");
            break;
        }
        log_i("get privacyPolicy fileHash");
        if (FALSE == adm_ocs_getJsonStrVal(privacyFileObject, ADM_COM_AIR_PROTOCOL_FIELD_HASH, &(privacyMember->privacyShowFileHash))) {
            retVal = ADM_ECODE_JSON_GET_STRING_FAILED;
            log_e("get fileHash value from json_object. failed");
            break;
        }
        log_i("get privacyPolicy fileSize");
        if (FALSE == adm_ocs_getJsonInt64Val(privacyFileObject, ADM_COM_AIR_PROTOCOL_FIELD_SIZE, &(privacyMember->privacyShowFileSize))) {
            retVal = ADM_ECODE_JSON_GET_INT_FAILED;
            log_e("get fileSize value from json_object. failed");
            break;
        }
        log_i("get privacyPolicy fileName");
        if (FALSE == adm_ocs_getJsonStrVal(privacyFileObject, ADM_COM_AIR_PROTOCOL_FIELD_FILENAME, &(privacyMember->privacyShowFileName))) {
            retVal = ADM_ECODE_JSON_GET_STRING_FAILED;
            log_e("get fileName value from json_object. failed");
            break;
        }
        log_i("get privacyPolicy file security info");
        retVal = adm_com_parseJsonFileSecurityInfo(privacyFileObject, &(privacyMember->fileSecurityInfo));
        if (ADM_ECODE_SUCCESSED != retVal) {
            log_e("adm_com_parseJsonFileSecurityInfo return error[%d]", retVal);
            break;
        }
		    
    } while(FALSE);
		
    return retVal;
}
D32S adm_ocs_checkProtocolParse(DCHAR* respBodyFromCloud, ADM_OCS_CHECK_RESP_ADAPTER_T* checkResp)
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

    do {
        log_i("parse cloud resp message");
        retVal = adm_ocs_parseCloudRespStatus(respBodyFromCloud, &respObject, &respStatus);
        if (ADM_ECODE_SUCCESSED != retVal) {
            log_e("adm_ocs_parseCloudRespStatus.error[%d]", retVal);
            break;
        }

        log_i("compare respStatus");
        if (ADM_OCS_AIR_STATUS_CODE_SUCCESSED != respStatus) {
            log_e("respStatus is not success, respStatus = %d", respStatus);
            //!< 处理状态码
            if (NULL != g_cloudRespStatusCb) {
                g_cloudRespStatusCb(respStatus);
            } else {
                log_e("g_cloudRespStatusCb is NULL");
            }
            retVal = adm_ocs_otaStatusCodeTransfer(respStatus);
            break;
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
            log_e("resp mesaage body is not contains exts");
        }
        /*get data filed*/
        log_i("get data filed");
        retVal = adm_ocs_respDataGetHmiPolicyFiled(checkResp, respObject, dataObject);
        if (ADM_ECODE_SUCCESSED != retVal) {
            log_e("call adm_ocs_respDataGetHmiPolicyFiled. failed");
            break;
        }
    } while(FALSE);
    if (NULL != respObject) {
        (void)json_object_put(respObject);
    }
    return retVal;
}

D32S adm_ocs_usbCheckProtocolParse(DCHAR* respBodyFromCloud, ADM_OCS_CHECK_RESP_ADAPTER_T* checkResp)
{
    D32S retVal = ADM_ECODE_SUCCESSED;
    json_object* dataObject = NULL;
    json_object *pJsonValue = NULL;

    if (NULL == respBodyFromCloud || NULL == checkResp)
    {
        log_e("input param invalid");
        return ADM_ECODE_INVALID_PARAM;
    }

    do
    {
        log_i("parse cloud resp message");
        pJsonValue = json_object_new_object();
        if (NULL == pJsonValue)
        {
            retVal = ADM_ECODE_NEW_JSON_OBJECT_FAILED;
            log_e("new json object. failed");
            break;
        }

        /*get dataObject*/
        dataObject = json_tokener_parse(respBodyFromCloud);
        if (NULL == dataObject)
        {
            log_e("parse resp body failed");
            retVal = ADM_ECODE_STRING_TO_JSON_FAILED;
            break;
        }

        if (0 != json_object_object_add(pJsonValue, ADM_COM_AIR_PROTOCOL_FIELD_DATA, dataObject)) 
        {
            log_e("cmdId add json object failed");
            retVal = ADM_ECODE_JSON_ADD_STRING_FAILED;
            (void)json_object_put(dataObject);
            break;
        }

        log_i("get data filed");
        retVal = adm_ocs_respDataGetHmiPolicyFiled(checkResp, pJsonValue, dataObject);
        if (ADM_ECODE_SUCCESSED != retVal)
        {
            log_e("call adm_ocs_respDataGetHmiPolicyFiled. failed");
            break;
        }
    } while (FALSE);

    if (NULL != pJsonValue)
    {
        (void)json_object_put(pJsonValue);
    }

    return retVal;
}

D32S adm_ocs_reportVehicleInfoProtocolParse(DCHAR* respBodyFromCloud, D32S* status, DCHAR** versionNumber)
{
    if (NULL == respBodyFromCloud) {
        log_e("input param invalid");
        return ADM_ECODE_INVALID_PARAM;
    }
    D32S retVal = ADM_ECODE_SUCCESSED;
    json_object* respObject = NULL;
    json_object* dataObject = NULL;

    retVal = adm_ocs_parseCloudRespStatus(respBodyFromCloud, &respObject, status);
    if (ADM_ECODE_SUCCESSED == retVal) {
        //!< 处理状态码
        if (ADM_OCS_AIR_STATUS_CODE_SUCCESSED == *status) {
            /*option get dataObject*/
            if ((json_bool)0 != json_object_object_get_ex(respObject, ADM_COM_AIR_PROTOCOL_FIELD_DATA, &dataObject)) {
                log_i("get data object");
                if (TRUE
                    == adm_ocs_getJsonStrVal(dataObject, ADM_COM_AIR_PROTOCOL_FIELD_VERSIONNUMBER, versionNumber)) {
                    log_i("get versionNumber success, versionNumber = %s", versionNumber);
                } else {
                    log_e("get versionNumber failed");
                    retVal = ADM_ECODE_JSON_GET_STRING_FAILED;
                }
            } else {
                log_w("there is no data object in respBodyFromCloud");
            }
        } else {
            if (NULL != g_cloudRespStatusCb) {
                g_cloudRespStatusCb(*status);
            } else {
                log_e("g_cloudRespStatusCb is NULL");
            }
            retVal = adm_ocs_otaStatusCodeTransfer(*status);
        }
    } else {
        log_e("adm_ocs_parseCloudRespStatus.error[%d]", retVal);
    }

    if (NULL != respObject) {
        (void)json_object_put(respObject);
    }
    return retVal;
}
/*parse check resp##################################################*/

/*base protocol adapter########################################################################################*/
ADM_OCS_BASE_REQ_ADAPTER_T* adm_ocs_newBaseReqAdapterBody(D64S timestamp,
    DCHAR* sdkVersion,
    DCHAR* extsObjectStr,
    ADM_OCS_BUSINESS_COMM_INTERFACE_CALLBACK_F fnBusinessCommCb)
{
    if (NULL == sdkVersion || NULL == fnBusinessCommCb) {
        return NULL;
    }

    ADM_OCS_BASE_REQ_ADAPTER_T* baseReq = NULL;
    baseReq = (ADM_OCS_BASE_REQ_ADAPTER_T*)adm_malloc(sizeof(ADM_OCS_BASE_REQ_ADAPTER_T));
    if (NULL != baseReq) {
        (void)adm_memset(baseReq, 0x00, sizeof(ADM_OCS_BASE_REQ_ADAPTER_T), sizeof(ADM_OCS_BASE_REQ_ADAPTER_T));
        baseReq->timestamp = timestamp;
        baseReq->sdkVersion = adm_strdup(sdkVersion);
        if (NULL != extsObjectStr) {
            baseReq->extsObjectStr = adm_strdup(extsObjectStr);
        } else {
            baseReq->extsObjectStr = NULL;
        }
        baseReq->fnBusinessCommCb = fnBusinessCommCb;
    }
    return baseReq;
}

void adm_ocs_deleteBaseReqAdapterBody(ADM_OCS_BASE_REQ_ADAPTER_T* baseReq)
{
    if (NULL == baseReq)
    {
        return;
    }
    adm_free(baseReq->sdkVersion);
    adm_free(baseReq->extsObjectStr);
    baseReq->fnBusinessCommCb = NULL;
    adm_free(baseReq);
    return;
}

ADM_OCS_BASE_RESP_ADAPTER_T* adm_ocs_newBaseRespAdapterBody(void)
{
    ADM_OCS_BASE_RESP_ADAPTER_T* baseResp = NULL;
    baseResp = (ADM_OCS_BASE_RESP_ADAPTER_T*)adm_malloc(sizeof(ADM_OCS_BASE_RESP_ADAPTER_T));
    if (NULL != baseResp) {
        (void)adm_memset(baseResp, 0x00, sizeof(ADM_OCS_BASE_RESP_ADAPTER_T), sizeof(ADM_OCS_BASE_RESP_ADAPTER_T));
    }
    return baseResp;
}

void adm_ocs_deleteBaseRespAdapterBody(ADM_OCS_BASE_RESP_ADAPTER_T* baseResp)
{
    if (NULL == baseResp)
    {
        return;
    }
    adm_free(baseResp->extsObjectStr);
    adm_free(baseResp->respDataStr);
    adm_free(baseResp);
    return;
}

D32S adm_ocs_baseProtocolAdapter(const ADM_OCS_BASE_REQ_ADAPTER_T* baseReq, DCHAR** adapterBaseReq)
{
    if (NULL == baseReq || NULL == baseReq->sdkVersion) {
        log_e("input param invalid");
        return ADM_ECODE_INVALID_PARAM;
    }

    json_object* rootObject = NULL;
    json_object* subObject = NULL;
    D32S retVal = ADM_ECODE_SUCCESSED;

    rootObject = json_object_new_object();
    if (NULL == rootObject) {
        log_e("rootObject new object.failed");
        return ADM_ECODE_INVALID_PARAM;
    }

    log_i("creat base req message body");
    do {
        /*set timestamp*/
        log_i("set timestamp");
        subObject = json_object_new_int64(baseReq->timestamp);
        if (NULL == subObject) {
            log_e("timestamp new json object failed");
            retVal = ADM_ECODE_NEW_JSON_OBJECT_FAILED;
            break;
        }
        if (0 != json_object_object_add(rootObject, ADM_COM_AIR_PROTOCOL_FIELD_TIMESTAMP, subObject)) {
            log_e("timestamp add json object failed");
            retVal = ADM_ECODE_JSON_ADD_INT64_FAILED;
            break;
        }
        /*set sdkVersion*/
        log_i("set sdkVersion");
        subObject = NULL;
        if (NULL != baseReq->sdkVersion)
        {
            subObject = json_object_new_string(baseReq->sdkVersion);
        }
        if (NULL == subObject) {
            log_e("sdkVersion new json object failed");
            retVal = ADM_ECODE_NEW_JSON_OBJECT_FAILED;
            break;
        }
        if (0 != json_object_object_add(rootObject, ADM_COM_AIR_PROTOCOL_FIELD_SDKVERSION, subObject)) {
            log_e("sdkVersion add json object failed");
            retVal = ADM_ECODE_JSON_ADD_STRING_FAILED;
            break;
        }
        /*option exts*/
        if (NULL != baseReq->extsObjectStr) {
            log_i("set exts to checkVersionObject");
            subObject = json_tokener_parse(baseReq->extsObjectStr);
            if (NULL != subObject) {
                if (0 != json_object_object_add(rootObject, ADM_COM_AIR_PROTOCOL_FIELD_EXT, subObject)) {
                    log_w("exts add json object.failed");
                }
            } else {
                log_w("exts str to object.failed");
            }
        }
        retVal = adm_ocs_protocolSignEncrypt(rootObject, adapterBaseReq);
        if (ADM_ECODE_SUCCESSED != retVal) {
            log_e("adm_ocs_protocolSignEncrypt return error[%d]", retVal);
            break;
        }

    } while(FALSE);
    (void)json_object_put(rootObject);
    return retVal;
}

D32S adm_ocs_baseProtocolParse(DCHAR* respBodyFromCloud, ADM_OCS_BASE_RESP_ADAPTER_T* baseResp)
{
    if (NULL == respBodyFromCloud || NULL == baseResp) {
        log_e("input param invalid");
        return ADM_ECODE_INVALID_PARAM;
    }

    D32S retVal = ADM_ECODE_SUCCESSED;
    D32S respStatus = -1;
    json_object* respObject = NULL;
    json_object* dataObject = NULL;
    json_object* extsObject = NULL;

    do {
        log_i("parse cloud resp message");
        retVal = adm_ocs_parseCloudRespStatus(respBodyFromCloud, &respObject, &respStatus);
        if (ADM_ECODE_SUCCESSED != retVal) {
            log_e("adm_ocs_parseCloudRespStatus.error[%d]", retVal);
            break;
        }
        baseResp->status = respStatus;
        log_i("compare respStatus");
        if (ADM_OCS_AIR_STATUS_CODE_SUCCESSED != respStatus && ADM_OCS_AIR_STATUS_CODE_HAVE_UPGRADE_TASK != respStatus
            && ADM_OCS_AIR_STATUS_CODE_NO_UPGRADE_TASK != respStatus) {
            log_e("respStatus is not success, respStatus = %d", respStatus);
            //!< 处理状态码
            if (NULL != g_cloudRespStatusCb) {
                g_cloudRespStatusCb(respStatus);
            } else {
                log_e("g_cloudRespStatusCb is NULL");
            }
            retVal = adm_ocs_otaStatusCodeTransfer(respStatus);
            break;
        }
        /*get dataObject*/
        log_i("get data object");
        if ((json_bool)0 != json_object_object_get_ex(respObject, ADM_COM_AIR_PROTOCOL_FIELD_DATA, &dataObject)) {
            baseResp->respDataStr = adm_strdup(json_object_to_json_string(dataObject));
        } else {
            log_e("get data object from json_object. failed");
            retVal = ADM_ECODE_JSON_GET_OBJECT_FAILED;
            break;
        }
        /*option:get exts*/
        log_i("get exts");
        if ((json_bool)0 != json_object_object_get_ex(respObject, ADM_COM_AIR_PROTOCOL_FIELD_EXT, &extsObject)) {
            baseResp->extsObjectStr = adm_strdup(json_object_to_json_string(extsObject));
            log_i("resp mesaage body contains exts");
        } else {
            log_e("resp mesaage body is not contains exts");
        }
    } while(FALSE);
    if (NULL != respObject) {
        (void)json_object_put(respObject);
    }
    return retVal;
}

/*base protocol adapter########################################################################################*/
/************************************license start*****************************************/
D32S adm_ocs_licenseCheckProtocolAdapter(const DCHAR* licenseHash,
    D64S timestamp,
    const DCHAR* sdkVersion,
    DCHAR** licenseCheckadapterReq)
{
    if (NULL == sdkVersion) {
        log_e("input param invalid");
        return ADM_ECODE_INVALID_PARAM;
    }
    json_object* rootObject = NULL;
    json_object* subObject = NULL;
    D32S retVal = ADM_ECODE_SUCCESSED;

    rootObject = json_object_new_object();
    if (NULL == rootObject) {
        log_e("rootObject new object.failed");
        return ADM_ECODE_INVALID_PARAM;
    }

    log_i("creat license check req message body");
    do {
        /*set timestamp*/
        log_i("set timestamp");
        subObject = json_object_new_int64(timestamp);
        if (NULL == subObject) {
            log_e("timestamp new json object failed");
            retVal = ADM_ECODE_NEW_JSON_OBJECT_FAILED;
            break;
        }
        if (0 != json_object_object_add(rootObject, ADM_COM_AIR_PROTOCOL_FIELD_TIMESTAMP, subObject)) {
            log_e("timestamp add json object failed");
            retVal = ADM_ECODE_JSON_ADD_INT64_FAILED;
            break;
        }
        /*set sdkVersion*/
        log_i("set sdkVersion");
        subObject = json_object_new_string(sdkVersion);
        if (NULL == subObject) {
            log_e("sdkVersion new json object failed");
            retVal = ADM_ECODE_NEW_JSON_OBJECT_FAILED;
            break;
        }
        if (0 != json_object_object_add(rootObject, ADM_COM_AIR_PROTOCOL_FIELD_SDKVERSION, subObject)) {
            log_e("sdkVersion add json object failed");
            retVal = ADM_ECODE_JSON_ADD_STRING_FAILED;
            break;
        }
        /*option set licenseHash*/
        if (NULL != licenseHash) {
            log_i("set licenseHash");
            subObject = json_object_new_string(licenseHash);
            if (NULL != subObject) {
                if (0 != json_object_object_add(rootObject, ADM_COM_AIR_PROTOCOL_FIELD_LICENSEHASH, subObject)) {
                    log_w("licenseHash add json object failed");
                }
            } else {
                log_w("licenseHash new json object failed");
            }
        }
        retVal = adm_ocs_protocolSignEncrypt(rootObject, licenseCheckadapterReq);
        if (ADM_ECODE_SUCCESSED != retVal) {
            log_e("adm_ocs_protocolSignEncrypt return error[%d]", retVal);
            break;
        }

    } while(FALSE);
    (void)json_object_put(rootObject);
    return retVal;
}

D32S adm_ocs_licenseCheckProtocolParse(const DCHAR* respBodyFromCloud, D32S* status, DCHAR** licenseCheckRespObjectStr)
{
    if (NULL == respBodyFromCloud) {
        log_e("input param invalid");
        return ADM_ECODE_INVALID_PARAM;
    }
    D32S retVal = ADM_ECODE_SUCCESSED;
    json_object* respObject = NULL;
    json_object* licenseObject = NULL;
    D32S respStatus = -1;

    do {
        retVal = adm_ocs_parseCloudRespStatus(respBodyFromCloud, &respObject, &respStatus);
        if (ADM_ECODE_SUCCESSED != retVal) {
            log_e("adm_ocs_parseCloudRespStatus.error[%d]", retVal);
            break;
        }
        *status = respStatus;
        log_i("compare respStatus");
        if (ADM_OCS_AIR_STATUS_CODE_SUCCESSED != respStatus && ADM_OCS_AIR_STATUS_CODE_HAVE_UPGRADE_TASK != respStatus
            && ADM_OCS_AIR_STATUS_CODE_NO_UPGRADE_TASK != respStatus) {
            log_e("respStatus is not success, respStatus = %d", respStatus);
            //!< 处理状态码
            if (NULL != g_cloudRespStatusCb) {
                g_cloudRespStatusCb(respStatus);
            } else {
                log_e("g_cloudRespStatusCb is NULL");
            }
            retVal = adm_ocs_otaStatusCodeTransfer(respStatus);
            break;
        }
        if ((json_bool)0 != json_object_object_get_ex(respObject, ADM_COM_AIR_PROTOCOL_FIELD_LICENSE, &licenseObject)) {
            *licenseCheckRespObjectStr = adm_strdup(json_object_to_json_string(licenseObject));
            log_i("licenseObject = %s", *licenseCheckRespObjectStr);
        } else {
            log_w("licenseObject is null");
            break;
        }

    } while(FALSE);
    (void)json_object_put(respObject);
    return retVal;
}

ADM_OCS_LICENSE_STATE_REPORT_REQ_ADAPTER_T* adm_ocs_newLicenseStateReportReqAdapterbody(const DCHAR* licenseHash,
    D64S timestamp,
    const DCHAR* sdkVersion,
    const DCHAR* state,
    const DCHAR* reason,
    const DCHAR* extsObjectStr)
{
    if (NULL == sdkVersion || NULL == state) {
        return NULL;
    }

    ADM_OCS_LICENSE_STATE_REPORT_REQ_ADAPTER_T* licenseStateReportReq = NULL;
    licenseStateReportReq
        = (ADM_OCS_LICENSE_STATE_REPORT_REQ_ADAPTER_T*)adm_malloc(sizeof(ADM_OCS_LICENSE_STATE_REPORT_REQ_ADAPTER_T));
    if (NULL != licenseStateReportReq) {
        if (NULL != licenseHash) {
            licenseStateReportReq->licenseHash = adm_strdup(licenseHash);
        } else {
            licenseStateReportReq->licenseHash = NULL;
        }
        licenseStateReportReq->timestamp = timestamp;
        licenseStateReportReq->sdkVersion = adm_strdup(sdkVersion);
        licenseStateReportReq->state = adm_strdup(state);
        if (NULL != reason) {
            licenseStateReportReq->reason = adm_strdup(reason);
        } else {
            licenseStateReportReq->reason = NULL;
        }
        if (NULL != extsObjectStr) {
            licenseStateReportReq->extsObjectStr = adm_strdup(extsObjectStr);
        } else {
            licenseStateReportReq->extsObjectStr = NULL;
        }
    }
    return licenseStateReportReq;
}

void adm_ocs_deleteLicenseStateReportReqAdapterbody(ADM_OCS_LICENSE_STATE_REPORT_REQ_ADAPTER_T* licenseStateReportReq)
{
    if (NULL == licenseStateReportReq) {
        return;
    }
    if (NULL != licenseStateReportReq->licenseHash)
    {
        adm_free(licenseStateReportReq->licenseHash);
    }
    if (NULL != licenseStateReportReq->reason)
    {
        adm_free(licenseStateReportReq->reason);
    }
    if (NULL != licenseStateReportReq->extsObjectStr)
    {
        adm_free(licenseStateReportReq->extsObjectStr);
    }
    adm_free(licenseStateReportReq->state);
    adm_free(licenseStateReportReq->sdkVersion);
    adm_free(licenseStateReportReq);
    return;
}

D32S adm_ocs_licenseReportProtocolAdapter(ADM_OCS_LICENSE_STATE_REPORT_REQ_ADAPTER_T* licenseStateReportReq,
    DCHAR** licenseCheckadapterReq)
{
    if (NULL == licenseStateReportReq) {
        log_e("input param invalid");
        return ADM_ECODE_INVALID_PARAM;
    }
    json_object* rootObject = NULL;
    json_object* subObject = NULL;
    D32S retVal = ADM_ECODE_SUCCESSED;

    rootObject = json_object_new_object();
    if (NULL == rootObject) {
        log_e("rootObject new object.failed");
        return ADM_ECODE_INVALID_PARAM;
    }

    log_i("creat license state report req message body");
    do {
        /*set timestamp*/
        log_i("set timestamp");
        subObject = json_object_new_int64(licenseStateReportReq->timestamp);
        if (NULL == subObject) {
            log_e("timestamp new json object failed");
            retVal = ADM_ECODE_NEW_JSON_OBJECT_FAILED;
            break;
        }
        if (0 != json_object_object_add(rootObject, ADM_COM_AIR_PROTOCOL_FIELD_TIMESTAMP, subObject)) {
            log_e("timestamp add json object failed");
            retVal = ADM_ECODE_JSON_ADD_INT64_FAILED;
            break;
        }
        /*set sdkVersion*/
        log_i("set sdkVersion");
        subObject = NULL;
        if (NULL != licenseStateReportReq->sdkVersion)
        {
            subObject = json_object_new_string(licenseStateReportReq->sdkVersion);
        }
        if (NULL == subObject) {
            log_e("sdkVersion new json object failed");
            retVal = ADM_ECODE_NEW_JSON_OBJECT_FAILED;
            break;
        }
        if (0 != json_object_object_add(rootObject, ADM_COM_AIR_PROTOCOL_FIELD_SDKVERSION, subObject)) {
            log_e("sdkVersion add json object failed");
            retVal = ADM_ECODE_JSON_ADD_STRING_FAILED;
            break;
        }
        /*option set licenseHash*/
        if (NULL != licenseStateReportReq->licenseHash) {
            log_i("set licenseHash");
            subObject = json_object_new_string(licenseStateReportReq->licenseHash);
            if (NULL != subObject) {
                if (0 != json_object_object_add(rootObject, ADM_COM_AIR_PROTOCOL_FIELD_LICENSEHASH, subObject)) {
                    log_w("licenseHash add json object failed");
                }
            } else {
                log_w("licenseHash new json object failed");
            }
        }
        /*set state*/
        log_i("set state");
        subObject = NULL;
        if (NULL != licenseStateReportReq->state)
        {
            subObject = json_object_new_string(licenseStateReportReq->state);
        }
        if (NULL == subObject) {
            log_e("state new json object failed");
            retVal = ADM_ECODE_NEW_JSON_OBJECT_FAILED;
            break;
        }
        if (0 != json_object_object_add(rootObject, ADM_COM_AIR_PROTOCOL_FIELD_STATE, subObject)) {
            log_e("state add json object failed");
            retVal = ADM_ECODE_JSON_ADD_STRING_FAILED;
            break;
        }
        /*option reason*/
        if (NULL != licenseStateReportReq->reason) {
            log_i("set reason");
            subObject = json_object_new_string(licenseStateReportReq->reason);
            if (NULL != subObject) {
                if (0 != json_object_object_add(rootObject, ADM_COM_AIR_PROTOCOL_FIELD_REASON, subObject)) {
                    log_w("reason add json object failed");
                }
            } else {
                log_w("reason new json object failed");
            }
        }
        /*option exts*/
        if (NULL != licenseStateReportReq->extsObjectStr) {
            log_i("set exts to checkVersionObject");
            subObject = json_tokener_parse(licenseStateReportReq->extsObjectStr);
            if (NULL != subObject) {
                if (0 != json_object_object_add(rootObject, ADM_COM_AIR_PROTOCOL_FIELD_EXT, subObject)) {
                    log_w("exts add json object.failed");
                }
            } else {
                log_w("exts str to object.failed");
            }
        }
        retVal = adm_ocs_protocolSignEncrypt(rootObject, licenseCheckadapterReq);
        if (ADM_ECODE_SUCCESSED != retVal) {
            log_e("adm_ocs_protocolSignEncrypt return error[%d]", retVal);
            break;
        }

    } while(FALSE);
    (void)json_object_put(rootObject);
    return retVal;
}

D32S adm_ocs_licenseStateReportProtocolParse(const DCHAR* respBodyFromCloud, D32S* status, DCHAR** licenseResp)
{
    if (NULL == respBodyFromCloud) {
        log_e("input param invalid");
        return ADM_ECODE_INVALID_PARAM;
    }

    D32S retVal = ADM_ECODE_SUCCESSED;
    D32S respStatus = -1;
    json_object* respObject = NULL;
    json_object* hashObject = NULL;

    do {
        log_i("parse cloud resp message");
        retVal = adm_ocs_parseCloudRespStatus(respBodyFromCloud, &respObject, &respStatus);
        if (ADM_ECODE_SUCCESSED != retVal) {
            log_e("adm_ocs_parseCloudRespStatus.error[%d]", retVal);
            break;
        }
        *status = respStatus;
        log_i("compare respStatus");
        if (ADM_OCS_AIR_STATUS_CODE_SUCCESSED != respStatus && ADM_OCS_AIR_STATUS_CODE_HAVE_UPGRADE_TASK != respStatus
            && ADM_OCS_AIR_STATUS_CODE_NO_UPGRADE_TASK != respStatus) {
            log_e("respStatus is not success, respStatus = %d", respStatus);
            //!< 处理状态码
            if (NULL != g_cloudRespStatusCb) {
                g_cloudRespStatusCb(respStatus);
            } else {
                log_e("g_cloudRespStatusCb is NULL");
            }
            retVal = adm_ocs_otaStatusCodeTransfer(respStatus);
            break;
        }
        if ((json_bool)0 != json_object_object_get_ex(respObject, ADM_COM_AIR_PROTOCOL_FIELD_LICENSE, &hashObject)) {
            log_i("get license object");
            *licenseResp = adm_strdup(json_object_to_json_string(hashObject));
        } else {
            log_w("resp mesaage body is not contains license");
        }
    } while(FALSE);
    if (NULL != respObject) {
        (void)json_object_put(respObject);
    }
    return retVal;
}

/************************************license end*****************************************/

/*parse upload resp##################################################*/

D32S adm_ocs_strSign(const DCHAR* str, DCHAR** sign)
{
    if (NULL == str) {
        log_e("input param invalid");
        return ADM_ECODE_INVALID_PARAM;
    }

    D32S retVal = ADM_ECODE_SUCCESSED;
    ADM_VRS_REGISTER_INFO_T* pRegisterInfo = NULL;
    DCHAR* tmpSign = NULL;
    do {
        pRegisterInfo = adm_vrs_newRegisterInfo();
        if (NULL == pRegisterInfo) {
            log_e("call adm_vrs_newRegisterInfo failed");
            retVal = ADM_ECODE_APPLY_MEMORY_FAILED;
            break;
        }

        log_i("get deviceId secret from register info");
        retVal = adm_vrs_getRegisterInfo(pRegisterInfo);
        if (ADM_ECODE_SUCCESSED != retVal) {
            log_e("call adm_vrs_getRegisterInfo error[%d]", retVal);
            break;
        }
        log_i("str = %s", str);
        tmpSign = adm_ocs_ariProtocolSign(str, pRegisterInfo->secret, pRegisterInfo->deviceId);
        if (NULL == tmpSign) {
            retVal = ADM_ECODE_CAR_CLOUD_PROTOCOL_REQMESSAGE_SIGN_FAILED;
            log_e("str sign failed");
            break;
        }
        *sign = tmpSign;
        tmpSign = NULL;
        log_i("str sign = %s", *sign);
    } while(FALSE);
    adm_vrs_delRegisterInfo(pRegisterInfo);
    return retVal;
}

D32S adm_ocs_uploadProtocolParse(DCHAR* respBodyFromCloud, D32S* status)
{
    if (NULL == respBodyFromCloud) {
        log_e("input param invalid");
        return ADM_ECODE_INVALID_PARAM;
    }
    D32S retVal = ADM_ECODE_SUCCESSED;
    json_object* respObject = NULL;
    retVal = adm_ocs_parseCloudRespStatus(respBodyFromCloud, &respObject, status);
    if (ADM_ECODE_SUCCESSED == retVal) {
        //!< 处理状态码
        if (ADM_OCS_AIR_STATUS_CODE_SUCCESSED != *status) {
            if (NULL != g_cloudRespStatusCb) {
                g_cloudRespStatusCb(*status);
            } else {
                log_e("g_cloudRespStatusCb is NULL");
            }
        }
    } else {
        log_e("adm_ocs_parseCloudRespStatus.error[%d]", retVal);
    }

    if (NULL != respObject) {
        (void)json_object_put(respObject);
    }
    return retVal;
}
/*parse upload resp##################################################*/
/*######################################### vehicle state######################################################*/
static D32S adm_ocs_basePackage(json_object* root,
    const DCHAR* sessionId,
    D64S taskId,
    D64S strategyId,
    D64S timestamp,
    const DCHAR* sdkVersion,
    D64S sequenceId,
    DBOOL isEventProtocol,
    const DCHAR* extsReqJsonStr)
{
    if (NULL == root) {
        log_e("input param invalid");
        return ADM_ECODE_INVALID_PARAM;
    }

    json_object* stringObject = NULL;
    json_object* d64Object = NULL;
    json_object* extsObject = NULL;
    D32S retVal = ADM_ECODE_SUCCESSED;

    do {
        /*option set sessionId*/
        if (NULL != sessionId) {
            // log_i("set sessionId");
            stringObject = json_object_new_string(sessionId);
            if (NULL == stringObject) {
                log_e("sessionId new json object failed");
                retVal = ADM_ECODE_NEW_JSON_OBJECT_FAILED;
                break;
            }
            if (0 != json_object_object_add(root, ADM_COM_AIR_PROTOCOL_FIELD_SESSIONID, stringObject)) {
                retVal = ADM_ECODE_JSON_ADD_STRING_FAILED;
                log_e("add sessionId failed");
                break;
            }
        }
        /*option set taskId*/
        if (0 != taskId) {
            // log_i("set taskId");
            d64Object = json_object_new_int64(taskId);
            if (NULL != d64Object) {
                if (0 != json_object_object_add(root, ADM_COM_AIR_PROTOCOL_FIELD_TASKID, d64Object)) {
                    log_w("taskId add json object failed");
                }
            } else {
                log_w("taskId new json object failed");
            }
        }
        /*option set strategyId*/
        if (0 != strategyId) {
            // log_i("set strategyId");
            d64Object = json_object_new_int64(strategyId);
            if (NULL != d64Object) {
                if (0 != json_object_object_add(root, ADM_COM_AIR_PROTOCOL_FIELD_STRATEGYID, d64Object)) {
                    log_w("strategyId add json object failed");
                }
            } else {
                log_w("strategyId new json object failed");
            }
        }
        /*set timestamp*/
        // log_i("set timestamp");
        d64Object = json_object_new_int64(timestamp);
        if (NULL == d64Object) {
            log_e("timestamp new json object failed");
            retVal = ADM_ECODE_NEW_JSON_OBJECT_FAILED;
            break;
        }
        if (0 != json_object_object_add(root, ADM_COM_AIR_PROTOCOL_FIELD_TIMESTAMP, d64Object)) {
            log_e("timestamp add json object failed");
            retVal = ADM_ECODE_JSON_ADD_INT64_FAILED;
            break;
        }
        /*set sdkVersion*/
        // log_i("set sdkVersion");
        stringObject = json_object_new_string(sdkVersion);
        if (NULL == stringObject) {
            log_e("sdkVersion new json object failed");
            retVal = ADM_ECODE_NEW_JSON_OBJECT_FAILED;
            break;
        }
        if (0 != json_object_object_add(root, ADM_COM_AIR_PROTOCOL_FIELD_SDKVERSION, stringObject)) {
            retVal = ADM_ECODE_JSON_ADD_STRING_FAILED;
            log_e("add sdkVersion failed");
            break;
        }

        /*set sequenceId,not event protocol*/
        if (FALSE == isEventProtocol) {
            // log_i("set sequenceId");
            d64Object = json_object_new_int64(sequenceId);
            if (NULL == d64Object) {
                log_e("sequenceId new json object failed");
                retVal = ADM_ECODE_NEW_JSON_OBJECT_FAILED;
                break;
            }
            if (0 != json_object_object_add(root, ADM_COM_AIR_PROTOCOL_FIELD_SEQUENCEID, d64Object)) {
                log_e("sequenceId add json object failed");
                retVal = ADM_ECODE_JSON_ADD_INT64_FAILED;
                break;
            }
        }

        if (NULL != extsReqJsonStr) {
            // log_i("set exts ");
            extsObject = json_tokener_parse(extsReqJsonStr);
            if (NULL != extsObject) {
                if (0 != json_object_object_add(root, ADM_COM_AIR_PROTOCOL_FIELD_EXT, extsObject)) {
                    log_w("exts add json object.failed");
                }
            } else {
                log_w("exts str to object.failed");
            }
        }
    } while(FALSE);
    return retVal;
}
typedef struct ADM_OCS_SUBOBJECT_
{
    json_object* subObject;
    DCHAR* field;
} ADM_OCS_SUBOBJECT_T;

static D32S adm_ocs_reportBaseProtocolAdapter(const DCHAR* sessionId,
    D64S taskId,
    D64S strategyId,
    D64S timestamp,
    const DCHAR* sdkVersion,
    D64S sequenceId,
    DBOOL isEventProtocol,
    ADM_OCS_SUBOBJECT_T* subObjectArray,
    D32S subObjectArrayLen,
    const DCHAR* extsReqJsonStr,
    DCHAR** generateMessageBody)
{
    if (NULL == sdkVersion || NULL == subObjectArray || 0 == subObjectArrayLen) {
        log_e("input param invalid");
        return ADM_ECODE_INVALID_PARAM;
    }

    D32S retVal = ADM_ECODE_SUCCESSED;
    json_object* root = NULL;
    json_object* stringObject = NULL;
    json_object* d64Object = NULL;
    DCHAR* rootStr = NULL;
    DCHAR* reqSign = NULL;
    DCHAR* enCryptProtocolStr = NULL;
    ADM_VRS_REGISTER_INFO_T* pRegisterInfo = NULL;
    do {

        pRegisterInfo = adm_vrs_newRegisterInfo();
        if (NULL == pRegisterInfo) {
            log_e("call adm_vrs_newRegisterInfo failed");
            retVal = ADM_ECODE_APPLY_MEMORY_FAILED;
            break;
        }

        // log_i("get deviceId secret from register info");
        if (ADM_ECODE_SUCCESSED != adm_vrs_getRegisterInfo(pRegisterInfo)) {
            log_e("call adm_vrs_getRegisterInfo failed");
            retVal = ADM_ECODE_FAILED;
            break;
        }

        root = json_object_new_object();
        if (NULL == root) {
            log_e("checkVersionObject new object.failed");
            retVal = ADM_ECODE_APPLY_MEMORY_FAILED;
            break;
        }

        retVal = adm_ocs_basePackage(
            root, sessionId, taskId, strategyId, timestamp, sdkVersion, sequenceId, isEventProtocol, extsReqJsonStr);
        if (ADM_ECODE_SUCCESSED != retVal) {
            log_e("adm_ocs_basePackage.failed");
            break;
        }
        /*add subObject*/
        // log_i("add subObject");
        for (D32S i = 0; i < subObjectArrayLen; ++i) {
            if (NULL != subObjectArray[i].field && NULL != subObjectArray[i].subObject) {
                // log_i("set %s", subObjectArray[i].field);
                (void)json_object_object_add(root, subObjectArray[i].field, subObjectArray[i].subObject);
            }
        }

        retVal = adm_ocs_protocolSignEncrypt(root, generateMessageBody);
        if (ADM_ECODE_SUCCESSED != retVal) {
            log_e("adm_ocs_protocolSignEncrypt return error[%d]", retVal);
            retVal = ADM_ECODE_CAR_CLOUD_PROTOCOL_REQMESSAGE_ENCRYPT_FAILED;
            break;
        }

    } while(FALSE);
    
    (void)json_object_put(root);
    adm_vrs_delRegisterInfo(pRegisterInfo);
    return retVal;
}

D32S adm_ocs_vehicleStateProtocolAdapter(D64S timestamp,
    const DCHAR* sdkVersion,
    const DCHAR* sessionId,
    D64S taskId,
    D64S strategyId,
    D64S sequenceId,
    const DCHAR* state,
    const DCHAR* reason,
    const DCHAR* versionNumber,
    const DCHAR* vehicleVersion,
    const DCHAR* extsReqJsonStr,
    DCHAR** generateMessageBody)
{
    if (NULL == sdkVersion || NULL == state || NULL == reason) {
        log_e("input param invalid");
        return ADM_ECODE_INVALID_PARAM;
    }

    D32S retVal = ADM_ECODE_SUCCESSED;
    ADM_OCS_SUBOBJECT_T* subObjectArray = NULL;
    D32S subObjectArrayLen = 0;
    const D32S subObjectArrayMaxLen = 3;
    json_object* stateObject = NULL;
    json_object* reasonObject = NULL;
    json_object* versionNumberObject = NULL;
    json_object* vehicleVersionObject = NULL;

    do {
        subObjectArray = (ADM_OCS_SUBOBJECT_T*)adm_malloc(sizeof(ADM_OCS_SUBOBJECT_T) * (DSIZE)subObjectArrayMaxLen);
        if (NULL != subObjectArray) {
            (void)adm_memzero(subObjectArray, sizeof(ADM_OCS_SUBOBJECT_T) * (D32U)subObjectArrayMaxLen);
        } else {
            log_e("adm_malloc failed");
            retVal = ADM_ECODE_NO_MEMORY;
            break;
        }

        stateObject = json_object_new_string(state);
        if (NULL != stateObject) {
            subObjectArray[subObjectArrayLen].subObject = stateObject;
            subObjectArray[subObjectArrayLen].field = adm_strdup(ADM_COM_AIR_PROTOCOL_FIELD_STATE);
            subObjectArrayLen += 1;
        } else {
            log_e("state new json object failed");
            retVal = ADM_ECODE_NEW_JSON_OBJECT_FAILED;
            break;
        }

        reasonObject = json_object_new_string(reason);
        if (NULL != reasonObject) {
            subObjectArray[subObjectArrayLen].subObject = reasonObject;
            subObjectArray[subObjectArrayLen].field = adm_strdup(ADM_COM_AIR_PROTOCOL_FIELD_REASON);
            subObjectArrayLen += 1;
        } else {
            log_e("reason new json object failed");
            retVal = ADM_ECODE_NEW_JSON_OBJECT_FAILED;
            break;
        }

        if (NULL != versionNumber) {
            /*set versionNumber*/
            log_i("set versionNumber");
            versionNumberObject = json_object_new_string(versionNumber);
            if (NULL != versionNumberObject) {
                subObjectArray[subObjectArrayLen].subObject = versionNumberObject;
                subObjectArray[subObjectArrayLen].field = adm_strdup(ADM_COM_AIR_PROTOCOL_FIELD_VERSIONNUMBER);
                subObjectArrayLen += 1;
            } else {
                log_e("versionNumber new json object failed");
                retVal = ADM_ECODE_NEW_JSON_OBJECT_FAILED;
            }
        }

        if (NULL != vehicleVersion) 
        {
            /*set vehicleVersion*/
            log_i("set vehicleVersion");
            vehicleVersionObject = json_object_new_string(vehicleVersion);
            if (NULL != vehicleVersionObject) 
            {
                subObjectArray[subObjectArrayLen].subObject = vehicleVersionObject;
                subObjectArray[subObjectArrayLen].field = adm_strdup(ADM_COM_AIR_PROTOCOL_FIELD_VEHICLEVERSION);
                subObjectArrayLen += 1;
            } else {
                log_e("vehicleVersion new json object failed");
                retVal = ADM_ECODE_NEW_JSON_OBJECT_FAILED;
            }
        }
        
        retVal = adm_ocs_reportBaseProtocolAdapter(sessionId,
            taskId,
            strategyId,
            timestamp,
            sdkVersion,
            sequenceId,
            FALSE,
            subObjectArray,
            subObjectArrayLen,
            extsReqJsonStr,
            generateMessageBody);
    } while(FALSE);

    for (D32S i = 0; i < subObjectArrayLen; ++i) {
            adm_free(subObjectArray[i].field);

            if((retVal != ADM_ECODE_SUCCESSED) && (retVal != ADM_ECODE_CAR_CLOUD_PROTOCOL_REQMESSAGE_ENCRYPT_FAILED))
            {
                json_object_put(subObjectArray[i].subObject);
            }
    }
    adm_free(subObjectArray);
    return retVal;
}

/*######################################### vehicle state######################################################*/
/*######################################### ecu state######################################################*/
static struct json_object* adm_ocs_ecuDidPackage(const ADM_OCS_ECU_DID_T* ecuDid)
{
    if (NULL == ecuDid || NULL == ecuDid->did || NULL == ecuDid->didValue) {
        log_e("ecuDid missing field");
        return NULL;
    }
    struct json_object* root = json_object_new_object();
    if (NULL != root) {
        (void)json_object_object_add(root, ADM_COM_AIR_PROTOCOL_FIELD_DID, json_object_new_string(ecuDid->did));
        (void)json_object_object_add(root, ADM_COM_AIR_PROTOCOL_FIELD_DIDVALUE, json_object_new_string(ecuDid->didValue));
        (void)json_object_object_add(root, ADM_COM_AIR_PROTOCOL_FIELD_START, json_object_new_int(ecuDid->start));
        (void)json_object_object_add(root, ADM_COM_AIR_PROTOCOL_FIELD_END, json_object_new_int(ecuDid->end));
    }
    return root;
}
static struct json_object* adm_ocs_ecuDidListPackage(const ADM_OCS_ECU_DID_LIST_T* ecuDids)
{
    if (NULL == ecuDids->ecuDids || 0 == ecuDids->ecuDidsLen) {
        log_e("ecuProgress ecuDidLists missing field");
        return NULL;
    }
    struct json_object* root = json_object_new_array();
    if (NULL != root) {
        for (D32S i = 0; i < ecuDids->ecuDidsLen; ++i) {
            struct json_object* ecuDidVal = adm_ocs_ecuDidPackage(ecuDids->ecuDids + i);
            if (NULL != ecuDidVal) {
                (void)json_object_array_add(root, ecuDidVal);
            }
        }
        if ((size_t)0 == json_object_array_length(root)) {
            log_e("ecuDids is empty");
            (void)json_object_put(root);
            return NULL;
        }
    }
    return root;
}

static struct json_object* adm_ocs_ecuInfoPackage(const ADM_OCS_ECU_STATE_INFO_T* reportEcu)
{
    if (NULL == reportEcu || NULL == reportEcu->ecuReqId || 
        NULL == reportEcu->ecuSwDid || NULL == reportEcu->ecuDidLists || 
        NULL == reportEcu->ecuState || NULL == reportEcu->ecuReason || 
        NULL == reportEcu->partNumber || NULL == reportEcu->partSupplier || 
        NULL == reportEcu->partHardwareNum) 
    {
        log_e("ecuState ecuInfoLists missing field");
        return NULL;
    }
    struct json_object* root = json_object_new_object();
    if (NULL != root) 
    {
        (void)json_object_object_add(root, ADM_COM_AIR_PROTOCOL_FIELD_ECUREQID, json_object_new_string(reportEcu->ecuReqId));

        (void)json_object_object_add(root, ADM_COM_AIR_PROTOCOL_FIELD_ECUSWDID, json_object_new_string(reportEcu->ecuSwDid));

        (void)json_object_object_add(root, ADM_COM_AIR_PROTOCOL_FIELD_ECUSTATE, json_object_new_string(reportEcu->ecuState));

        if (NULL != reportEcu->ecuResultDescrip)
        {
            (void)json_object_object_add(
                root, ADM_COM_AIR_PROTOCOL_FIELD_ECURESULTDESCRIP, json_object_new_string(reportEcu->ecuResultDescrip));
        }

        (void)json_object_object_add(
                root, ADM_COM_AIR_PROTOCOL_FIELD_ECUREASON, json_object_new_string(reportEcu->ecuReason));

        (void)json_object_object_add(
            root, ADM_COM_AIR_PROTOCOL_FIELD_PARTNUM, json_object_new_string(reportEcu->partNumber));

        (void)json_object_object_add(
            root, ADM_COM_AIR_PROTOCOL_FIELD_PARTSUPPLIER, json_object_new_string(reportEcu->partSupplier));

        if (NULL != reportEcu->softwarePartNumber)
        {
            (void)json_object_object_add(
                root, ADM_COM_AIR_PROTOCOL_FIELD_SOFTWAREPARTNUMBER, json_object_new_string(reportEcu->softwarePartNumber));
        }
        
        (void)json_object_object_add(
            root, ADM_COM_AIR_PROTOCOL_FIELD_PARTHARDWARENUM, json_object_new_string(reportEcu->partHardwareNum));

        (void)json_object_object_add(root, ADM_COM_AIR_PROTOCOL_FIELD_BEGINTIME, json_object_new_int64(reportEcu->beginTime));

        (void)json_object_object_add(root, ADM_COM_AIR_PROTOCOL_FIELD_ENDTIME, json_object_new_int64(reportEcu->endTime));

        struct json_object* ecuDidsVal = adm_ocs_ecuDidListPackage(reportEcu->ecuDidLists);
        if (NULL != ecuDidsVal) {
            (void)json_object_object_add(root, ADM_COM_AIR_PROTOCOL_FIELD_ECUDIDS, ecuDidsVal);
        } else {
            log_e("ecuDidLists is empty");
            (void)json_object_put(root);
            return NULL;
        }
    }
    return root;
}
static struct json_object* adm_ocs_ecuInfoListPackage(const ADM_OCS_ECU_STATE_INFO_LIST_T* reportEcus)
{
    if (NULL == reportEcus->reportEcus || 0 == reportEcus->reportEcusLen) {
        log_e("ecuState ecuLists missing field");
        return NULL;
    }
    struct json_object* root = json_object_new_array();
    if (NULL != root) {
        for (D32S i = 0; i < reportEcus->reportEcusLen; ++i) {
            struct json_object* reportEcuVal = adm_ocs_ecuInfoPackage(reportEcus->reportEcus + i);
            if (NULL != reportEcuVal) {
                (void)json_object_array_add(root, reportEcuVal);
            }
        }
        if ((size_t)0 == json_object_array_length(root)) {
            log_e("ecuInfoLists is empty");
            (void)json_object_put(root);
            return NULL;
        }
    }
    return root;
}

D32S adm_ocs_ecuStateProtocolAdapter(const DCHAR* sessionId,
    D64S taskId,
    D64S strategyId,
    D64S timestamp,
    const DCHAR* sdkVersion,
    D64S sequenceId,
    const DCHAR* extsReqJsonStr,
    ADM_OCS_ECU_STATE_INFO_LIST_T* ecusList,
    DCHAR** generateMessageBody)
{
    if (0 == taskId || 0 == strategyId || NULL == sdkVersion || NULL == ecusList) {
        log_e("input param invalid");
        return ADM_ECODE_INVALID_PARAM;
    }
    D32S retVal = ADM_ECODE_SUCCESSED;
    ADM_OCS_SUBOBJECT_T* subObjectArray = NULL;
    D32S subObjectArrayLen = 0;
    json_object* reportEcusJson = NULL;

    subObjectArray = (ADM_OCS_SUBOBJECT_T*)adm_malloc(sizeof(ADM_OCS_SUBOBJECT_T));
    if (NULL != subObjectArray) {
        (void)adm_memzero(subObjectArray, sizeof(ADM_OCS_SUBOBJECT_T));
        subObjectArrayLen = 1;
    } else {
        log_e("adm_malloc failed");
        return ADM_ECODE_NO_MEMORY;
    }

    reportEcusJson = adm_ocs_ecuInfoListPackage(ecusList);
    if (NULL != reportEcusJson) {
        subObjectArray->subObject = reportEcusJson;
        subObjectArray->field = adm_strdup(ADM_COM_AIR_PROTOCOL_FIELD_REPORTECUS);

        retVal = adm_ocs_reportBaseProtocolAdapter(sessionId,
            taskId,
            strategyId,
            timestamp,
            sdkVersion,
            sequenceId,
            FALSE,
            subObjectArray,
            subObjectArrayLen,
            extsReqJsonStr,
            generateMessageBody);

    } else {
        log_e("adm_ocs_ecuInfoListPackage.failed");
        retVal = ADM_ECODE_APPLY_MEMORY_FAILED;
    }
    adm_free(subObjectArray->field);
    if((retVal != ADM_ECODE_SUCCESSED) && (retVal != ADM_ECODE_CAR_CLOUD_PROTOCOL_REQMESSAGE_ENCRYPT_FAILED))
    {
        json_object_put(subObjectArray->subObject);
    }
    adm_free(subObjectArray);
    return retVal;
}
/*######################################### ecu state######################################################*/

/*######################################### vehicle progress######################################################*/

static struct json_object* adm_ocs_ecuProgressPackage(const ADM_OCS_ECU_PROGRESS_INFO_T* reportEcu)
{
    if (NULL == reportEcu || NULL == reportEcu->progressType || NULL == reportEcu->ecuReqId
        || NULL == reportEcu->ecuSwDid || NULL == reportEcu->partHardwareNum || 
        NULL == reportEcu->partNumber || NULL == reportEcu->partSupplier) 
    {
        log_e("ecuProgress ecuLists missing field");
        return NULL;
    }

    struct json_object* root = json_object_new_object();
    if (NULL != root) {
        (void)json_object_object_add(
            root, ADM_COM_AIR_PROTOCOL_FIELD_PROGRESSTYPE, json_object_new_string(reportEcu->progressType));
        (void)json_object_object_add(root, ADM_COM_AIR_PROTOCOL_FIELD_ECUREQID, json_object_new_string(reportEcu->ecuReqId));
        (void)json_object_object_add(root, ADM_COM_AIR_PROTOCOL_FIELD_ECUSWDID, json_object_new_string(reportEcu->ecuSwDid));
        
        (void)json_object_object_add(
            root, ADM_COM_AIR_PROTOCOL_FIELD_PARTNUM, json_object_new_string(reportEcu->partNumber));

        (void)json_object_object_add(
            root, ADM_COM_AIR_PROTOCOL_FIELD_PARTSUPPLIER, json_object_new_string(reportEcu->partSupplier));

        if (NULL != reportEcu->softwarePartNumber)
        {
            (void)json_object_object_add(
                root, ADM_COM_AIR_PROTOCOL_FIELD_SOFTWAREPARTNUMBER, json_object_new_string(reportEcu->softwarePartNumber));
        }
        
        (void)json_object_object_add(
            root, ADM_COM_AIR_PROTOCOL_FIELD_PARTHARDWARENUM, json_object_new_string(reportEcu->partHardwareNum));

        (void)json_object_object_add(root, ADM_COM_AIR_PROTOCOL_FIELD_PROGRESS, json_object_new_int(reportEcu->progress));
    }
    return root;
}
static struct json_object* adm_ocs_ecuProgressListPackage(const ADM_OCS_ECU_PROGRESS_INFO_LIST_T* reportEcus)
{
    if (NULL == reportEcus->reportEcus || 0 == reportEcus->reportEcusLen) {
        log_e("ecuProgress ecuLists missing field");
        return NULL;
    }
    struct json_object* root = json_object_new_array();
    if (NULL != root) {
        for (D32S i = 0; i < reportEcus->reportEcusLen; ++i) {
            struct json_object* reportEcuVal = adm_ocs_ecuProgressPackage(reportEcus->reportEcus + i);
            if (NULL != reportEcuVal) {
                (void)json_object_array_add(root, reportEcuVal);
            }
        }
        if ((size_t)0 == json_object_array_length(root)) {
            log_e("ecuProgress ecuLists is empty");
            (void)json_object_put(root);
            return NULL;
        }
    }
    return root;
}

D32S adm_ocs_vehicleProgressProtocolAdapter(const DCHAR* sessionId,
    D64S taskId,
    D64S strategyId,
    D64S timestamp,
    const DCHAR* sdkVersion,
    D64S sequenceId,
    const DCHAR* extsReqJsonStr,
    ADM_OCS_ECU_PROGRESS_INFO_LIST_T* ecusProgressList,
    DCHAR** generateMessageBody)
{
    if (0 == taskId || 0 == strategyId || NULL == sdkVersion || NULL == ecusProgressList) {
        log_e("input param invalid");
        return ADM_ECODE_INVALID_PARAM;
    }
    D32S retVal = ADM_ECODE_SUCCESSED;
    ADM_OCS_SUBOBJECT_T* subObjectArray = NULL;
    const D32S subObjectArrayMaxLen = 3;
    D32S subObjectArrayLen = 0;
    json_object* reportEcusJson = NULL;
    json_object *pTotalTypeObject = NULL;
    json_object *pTotalProgressObject = NULL;
    D32S i = 0;

    subObjectArray = (ADM_OCS_SUBOBJECT_T *)adm_malloc(sizeof(ADM_OCS_SUBOBJECT_T) * subObjectArrayMaxLen);
    if (subObjectArray)
    {
        adm_memzero(subObjectArray, sizeof(ADM_OCS_SUBOBJECT_T) * subObjectArrayMaxLen);
    }
    else
    {
        log_e("adm_malloc failed");
        return ADM_ECODE_NO_MEMORY;
    }

    do
    {
        if (NULL != ecusProgressList->progressType)
        {
            log_i("totalProgressType = %s", ecusProgressList->progressType);
            pTotalTypeObject = json_object_new_string(ecusProgressList->progressType);
            if (pTotalTypeObject)
            {
                subObjectArray[subObjectArrayLen].subObject = pTotalTypeObject;
                subObjectArray[subObjectArrayLen].field = adm_strdup(ADM_COM_AIR_PROTOCOL_FIELD_PROGRESSTYPE);
                subObjectArrayLen += 1;
            }
            else
            {
                log_e("progressType new json object failed");
                retVal = ADM_ECODE_NEW_JSON_OBJECT_FAILED;
                break;
            }

            pTotalProgressObject = json_object_new_int(ecusProgressList->iTotalProgress);
            if (pTotalProgressObject)
            {
                subObjectArray[subObjectArrayLen].subObject = pTotalProgressObject;
                subObjectArray[subObjectArrayLen].field = adm_strdup(ADM_COM_AIR_PROTOCOL_FIELD_PROGRESS);
                subObjectArrayLen += 1;
            }
            else
            {
                log_e("progress new json object failed");
                retVal = ADM_ECODE_NEW_JSON_OBJECT_FAILED;
                break;
            }
        }

        reportEcusJson = adm_ocs_ecuProgressListPackage(ecusProgressList);
        if (reportEcusJson)
        {
            subObjectArray[subObjectArrayLen].subObject = reportEcusJson;
            subObjectArray[subObjectArrayLen].field = adm_strdup(ADM_COM_AIR_PROTOCOL_FIELD_REPORTECUS);
            subObjectArrayLen += 1;
            retVal = adm_ocs_reportBaseProtocolAdapter(sessionId,
                                                       taskId,
                                                       strategyId,
                                                       timestamp,
                                                       sdkVersion,
                                                       sequenceId,
                                                       FALSE,
                                                       subObjectArray,
                                                       subObjectArrayLen,
                                                       extsReqJsonStr,
                                                       generateMessageBody);
        }
        else
        {
            log_e("adm_ocs_ecuProgressListPackage.failed");
            retVal = ADM_ECODE_APPLY_MEMORY_FAILED;
        }
    } while (0);

    for (i = 0; i < subObjectArrayLen; i++)
    {
        adm_free(subObjectArray[i].field);
    }

    adm_free(subObjectArray);
    
    return retVal;
}
/*######################################### vehicle progress######################################################*/

/*#########################################events######################################################*/
static struct json_object* adm_ocs_eventInfoNodePackage(const ADM_OCS_EVENTS_INFO_T* n)
{
    if (NULL == n || NULL == n->eventId || NULL == n->eventDesc) {
        log_e("eventInfo missing field");
        return NULL;
    }

    struct json_object* root = json_object_new_object();
    if (root != NULL) {
        struct json_object* eventSequenceIdVal = json_object_new_int64(n->eventSequenceId);
        if (eventSequenceIdVal != NULL) {
            (void)json_object_object_add(root, ADM_COM_AIR_PROTOCOL_FIELD_EVENTSEQUENCEID, eventSequenceIdVal);
        }

        struct json_object* eventIdVal = json_object_new_string(n->eventId);
        if (eventIdVal != NULL) {
            (void)json_object_object_add(root, ADM_COM_AIR_PROTOCOL_FIELD_EVENTID, eventIdVal);
        }

        struct json_object* eventLevelVal = json_object_new_int(n->eventLevel);
        if (eventLevelVal != NULL) {
            (void)json_object_object_add(root, ADM_COM_AIR_PROTOCOL_FIELD_EVENTLEVEL, eventLevelVal);
        }

        struct json_object* eventDescVal = json_object_new_string(n->eventDesc);
        if (eventDescVal != NULL) {
            (void)json_object_object_add(root, ADM_COM_AIR_PROTOCOL_FIELD_EVENTDESC, eventDescVal);
        }
        /*option*/
        if (NULL != n->eventContext) {
            struct json_object* eventContextVal = json_object_new_string(n->eventContext);
            if (eventContextVal != NULL) {
                (void)json_object_object_add(root, ADM_COM_AIR_PROTOCOL_FIELD_EVENTCONTEXT, eventContextVal);
            }
        }
        struct json_object* eventTimeVal = json_object_new_int64(n->eventTime);
        if (eventTimeVal != NULL) {
            (void)json_object_object_add(root, ADM_COM_AIR_PROTOCOL_FIELD_EVENTTIME, eventTimeVal);
        }
    }
    return root;
}
static struct json_object* adm_ocs_eventListsPackage(const ADM_OCS_EVENTS_INFO_LIST_T* data)
{
    if (NULL == data->reportEvents || 0 == data->reportEventsLen) {
        log_e("event missing field");
        return NULL;
    }
    struct json_object* root = json_object_new_array();
    if (NULL != root) {
        for (D32S i = 0; i < data->reportEventsLen; ++i) {
            struct json_object* reportEcuVal = adm_ocs_eventInfoNodePackage(data->reportEvents + i);
            if (NULL != reportEcuVal) {
                (void)json_object_array_add(root, reportEcuVal);
            }
        }
        if ((size_t)0 == json_object_array_length(root)) {
            (void)json_object_put(root);
            log_e("eventLists is empty");
            return NULL;
        }
    }
    return root;
}

D32S adm_ocs_eventProtocolAdapter(const DCHAR* sessionId,
    D64S taskId,
    D64S strategyId,
    D64S timestamp,
    const DCHAR* sdkVersion,
    const DCHAR* extsReqJsonStr,
    ADM_OCS_EVENTS_INFO_LIST_T* eventsList,
    ADM_OCS_EVENT_SOURCE_TYPE_E eEventSoureType,
    DCHAR** generateMessageBody)
{
    if (NULL == sdkVersion || NULL == eventsList)
    {
        log_e("input param invalid");
        return ADM_ECODE_INVALID_PARAM;
    }

    D32S retVal = ADM_ECODE_SUCCESSED;
    ADM_OCS_SUBOBJECT_T *subObjectArray = NULL;
    D32S subObjectArrayLen = 0;
    D32S subObjectArrayMaxLen = 2;
    D64S sequenceId = 0;
    json_object *eventsJson = NULL;
    json_object *pSourceType = NULL;
    D32S i = 0;

    subObjectArray = (ADM_OCS_SUBOBJECT_T *)adm_malloc(sizeof(ADM_OCS_SUBOBJECT_T) * subObjectArrayMaxLen);
    if (subObjectArray)
    {
        adm_memzero(subObjectArray, sizeof(sizeof(ADM_OCS_SUBOBJECT_T) * subObjectArrayMaxLen));
    }
    else
    {
        log_e("adm_malloc failed");
        return ADM_ECODE_NO_MEMORY;
    }

    do
    {
        eventsJson = adm_ocs_eventListsPackage(eventsList);
        if (ADM_IS_NULL(eventsJson))
        {
            log_e("eventsList new json object failed");
            retVal = ADM_ECODE_NEW_JSON_OBJECT_FAILED;
            break;
        }
        else
        {
            subObjectArray[subObjectArrayLen].subObject = eventsJson;
            subObjectArray[subObjectArrayLen].field = adm_strdup(ADM_COM_AIR_PROTOCOL_FIELD_EVENTS);
            subObjectArrayLen += 1;
        }

        pSourceType = json_object_new_int((D32S)eEventSoureType);
        if (pSourceType)
        {
            subObjectArray[subObjectArrayLen].subObject = pSourceType;
            subObjectArray[subObjectArrayLen].field = adm_strdup(ADM_COM_AIR_PROTOCOL_FIELD_EVENT_SOURCE);
            subObjectArrayLen += 1;
        }
        else
        {
            log_e("eEventSoureType new json object failed");
            retVal = ADM_ECODE_NEW_JSON_OBJECT_FAILED;
            break;
        }

        retVal = adm_ocs_reportBaseProtocolAdapter(sessionId,
                                                   taskId,
                                                   strategyId,
                                                   timestamp,
                                                   sdkVersion,
                                                   sequenceId,
                                                   TRUE,
                                                   subObjectArray,
                                                   subObjectArrayLen,
                                                   extsReqJsonStr,
                                                   generateMessageBody);
    } while (FALSE);

    for (i = 0; i < subObjectArrayLen; i++)
    {
        adm_free(subObjectArray[i].field);
    }

    adm_free(subObjectArray);

    return retVal;
}

D32S adm_ocs_scheduleProtocolAdapter(D64S timestamp,
    const DCHAR* sdkVersion,
    D64S taskId,
    D32S operaType,
    D32S source,
    D64S schduleTime,
    const DCHAR* extsReqJsonStr,
    DCHAR** generateMessageBody)
{
    if (NULL == sdkVersion) 
    {
        log_e("input param invalid");
        return ADM_ECODE_INVALID_PARAM;
    }

    D32S retVal = ADM_ECODE_SUCCESSED;
    json_object* stringObject = NULL;
    json_object* numberObject = NULL;
    struct json_object* root = NULL;

    do
    {
        root = json_object_new_object();
        if (root != NULL) 
        {
            log_i("set source[%d]", source);
            numberObject = json_object_new_int(source);
            if (numberObject != NULL) 
            {
                (void)json_object_object_add(root, ADM_COM_AIR_PROTOCOL_FIELD_SHCDULE_SOURCE, numberObject);
            }
            else 
            {
                log_e("source new json object failed");
                retVal = ADM_ECODE_JSON_ADD_INT_FAILED;
                break;
            }

            log_i("set operationType[%d]", operaType);
            numberObject = json_object_new_int(operaType);
            if (numberObject != NULL) 
            {
                (void)json_object_object_add(root, ADM_COM_AIR_PROTOCOL_FIELD_SHCDULE_OPERA_TYPE, numberObject);
            }
            else 
            {
                log_e("operationType new json object failed");
                retVal = ADM_ECODE_JSON_ADD_INT_FAILED;
                break;
            }
        
            if (0 != schduleTime) 
            {
                log_i("set upgradeTime[%lld]", schduleTime);
                numberObject = json_object_new_int64(schduleTime);
                if (numberObject != NULL) 
                {
                    (void)json_object_object_add(root, ADM_COM_AIR_PROTOCOL_FIELD_SHCDULE_TIME, numberObject);
                }
                else 
                {
                    log_e("upgradeTime new json object failed");
                    break;
                }
            }

            if (0 != taskId) 
            {
                log_i("set taskId");
                numberObject = json_object_new_int64(taskId);
                if (NULL != numberObject) 
                {
                    (void)json_object_object_add(root, ADM_COM_AIR_PROTOCOL_FIELD_TASKID, numberObject);
                } 
                else 
                {
                    log_e("taskId new json object failed");
                    retVal = ADM_ECODE_JSON_ADD_INT_FAILED;
                    break;
                }
            }

            log_i("set timestamp[%lld]", timestamp);
            numberObject = json_object_new_int64(timestamp);
            if (NULL != numberObject)
            {
                (void)json_object_object_add(root, ADM_COM_AIR_PROTOCOL_FIELD_TIMESTAMP, numberObject);
            }
            else
            {
                log_e("timestamp new json object failed");
                retVal = ADM_ECODE_NEW_JSON_OBJECT_FAILED;
                break;
            }
            
            log_i("set sdkVersion");
            stringObject = json_object_new_string(sdkVersion);
            if (NULL != stringObject) 
            {
                (void)json_object_object_add(root, ADM_COM_AIR_PROTOCOL_FIELD_SDKVERSION, stringObject);
            }
            else 
            {
                log_e("sdkVersion new json object failed");
                retVal = ADM_ECODE_NEW_JSON_OBJECT_FAILED;
                break;
            }

            if (NULL != extsReqJsonStr) 
            {
                log_i("set exts.");
                stringObject = json_tokener_parse(extsReqJsonStr);
                if (NULL != stringObject) 
                {
                    (void)json_object_object_add(root, ADM_COM_AIR_PROTOCOL_FIELD_EXT, stringObject);
                } 
                else 
                {
                    log_w("exts str to object.failed");
                }
            }

            retVal = adm_ocs_protocolSignEncrypt(root, generateMessageBody);
            if (ADM_ECODE_SUCCESSED != retVal) 
            {
                log_e("adm_ocs_protocolSignEncrypt return error[%d]", retVal);
                retVal = ADM_ECODE_CAR_CLOUD_PROTOCOL_REQMESSAGE_ENCRYPT_FAILED;
            }
        }
    } while (FALSE);

    (void)json_object_put(root);

    return retVal;
}

D32S adm_ocs_taskVerifyProtocolAdapter(const D64S ulTaskId,
                                       const D64S ulStrategyId,
                                       const D64S ulTimestamp,
                                       const DCHAR *pcSdkVersion,
                                       const DCHAR *extsObjectStr,
                                       DCHAR **ppcMessageBody)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    json_object *pJsonObj = NULL;
    json_object *pExtsObject = NULL;

    ADM_COM_FUNC_ENTRY();

    if (ADM_IS_NULL(pcSdkVersion) || ADM_IS_NULL(ppcMessageBody))
    {
        log_e("Input param is invalid");
        return ADM_ECODE_INVALID_PARAM;
    }
    
    do
    {
        pJsonObj = json_object_new_object();
        if (ADM_IS_NULL(pJsonObj)) 
        {
            log_e("new json object failed");
            iRet = ADM_ECODE_NEW_JSON_OBJECT_FAILED;
            break;
        }
        
        iRet = json_object_object_add(pJsonObj, ADM_COM_AIR_PROTOCOL_FIELD_TIMESTAMP, json_object_new_int64(ulTimestamp));
        if (ADM_ECODE_SUCCESSED != iRet) 
        {
            log_e("add timestamp failed");
            iRet = ADM_ECODE_JSON_ADD_INT64_FAILED;
            break;
        }
        
        iRet = json_object_object_add(pJsonObj, ADM_COM_AIR_PROTOCOL_FIELD_SDKVERSION, json_object_new_string(pcSdkVersion));
        if (ADM_ECODE_SUCCESSED != iRet) 
        {
            log_e("add sdkVersion failed");
            iRet = ADM_ECODE_JSON_ADD_STRING_FAILED;
            break;
        }

        iRet = json_object_object_add(pJsonObj, ADM_COM_AIR_PROTOCOL_FIELD_TASKID, json_object_new_int64(ulTaskId));
        if (ADM_ECODE_SUCCESSED != iRet) 
        {
            log_e("add taskId failed");
            iRet = ADM_ECODE_JSON_ADD_INT64_FAILED;
            break;
        }
        
        iRet = json_object_object_add(pJsonObj, ADM_COM_AIR_PROTOCOL_FIELD_STRATEGYID, json_object_new_int64(ulStrategyId));
        if (ADM_ECODE_SUCCESSED != iRet) 
        {
            log_e("add strategyId failed");
            iRet = ADM_ECODE_JSON_ADD_INT64_FAILED;
            break;
        }
        
        if (NULL != extsObjectStr) 
        {
            pExtsObject = json_tokener_parse(extsObjectStr);
            if (NULL != pExtsObject) 
            {
                iRet = json_object_object_add(pJsonObj, ADM_COM_AIR_PROTOCOL_FIELD_EXT, pExtsObject);
                if (ADM_ECODE_SUCCESSED != iRet) 
                {
                    log_e("add exts failed");
                    iRet = ADM_ECODE_JSON_ADD_STRING_FAILED;
                }
            }
        }
      
        iRet = adm_ocs_protocolSignEncrypt(pJsonObj, ppcMessageBody);
        if (ADM_ECODE_SUCCESSED != iRet) 
        {
            log_e("adm_ocs_protocolSignEncrypt return error[%d]", iRet);
            iRet = ADM_ECODE_CAR_CLOUD_PROTOCOL_REQMESSAGE_ENCRYPT_FAILED;
        }
    } while (FALSE);

    if (NULL != pJsonObj) 
    {
        (void)json_object_put(pJsonObj);
    }
    
    ADM_COM_FUNC_EXIT();

    return iRet;
}

D32S adm_ocs_taskVerifyProtocolAdapterResponse(DCHAR *pcRespBody)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    json_object *pJsonObj = NULL;
    D32S iRespStatus = 0;

    ADM_COM_FUNC_ENTRY();
    
    if (ADM_IS_NULL(pcRespBody)) 
    {
        log_e("Input param is invalid");
        return ADM_ECODE_INVALID_PARAM;
    }
    
    do
    {
        iRet = adm_ocs_parseCloudRespStatus(pcRespBody, &pJsonObj, &iRespStatus);
        if (ADM_ECODE_SUCCESSED != iRet) 
        {
            log_e("adm_ocs_parseCloudRespStatus.error[%d]", iRet);
            break;
        }

        if (ADM_OCS_AIR_STATUS_CODE_SUCCESSED != iRespStatus)
        {
            log_d("task verify failed, status = %d", iRespStatus);
            iRet = ADM_ECODE_UPGRADE_TASK_STOPED;
        }
    } while (FALSE);
    
    if (NULL != pJsonObj) 
    {
        (void)json_object_put(pJsonObj);
    }
    
    ADM_COM_FUNC_EXIT();

    return iRet;
}

/**
 * @brief  请求上传URL
 * @param in const D64S ulTimestamp       请求时间戳
 * @param in const DCHAR *pcSdkVersion    SDK版本号
 * @param in const DCHAR *extsObjectStr   扩展字段
 * @param out DCHAR **ppcMessageBody      返回报文
 * @return D32S 0 成功，其他失败
 */
D32S adm_ocs_requestUploadUrlProtocolAdapter(const D64S ulTimestamp,
                                             const DCHAR *pcSdkVersion,
                                             const DCHAR *extsObjectStr,
                                             DCHAR **ppcMessageBody)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    json_object *pJsonObj = NULL;
    json_object *pExtsObject = NULL;

    ADM_COM_FUNC_ENTRY();

    /** 参数校验 */
    if (ADM_IS_NULL(pcSdkVersion) || ADM_IS_NULL(ppcMessageBody))
    {
        log_e("Input param is invalid");
        return ADM_ECODE_INVALID_PARAM;
    }
    
    do
    {
        /** 创建json对象 */
        pJsonObj = json_object_new_object();
        if (ADM_IS_NULL(pJsonObj)) 
        {
            log_e("new json object failed");
            iRet = ADM_ECODE_NEW_JSON_OBJECT_FAILED;
            break;
        }
        
        /** 添加时间戳 */
        iRet = json_object_object_add(pJsonObj, ADM_COM_AIR_PROTOCOL_FIELD_TIMESTAMP, json_object_new_int64(ulTimestamp));
        if (ADM_ECODE_SUCCESSED != iRet) 
        {
            log_e("add timestamp failed");
            iRet = ADM_ECODE_JSON_ADD_INT64_FAILED;
            break;
        }
        
        /** 添加SDK版本号 */
        iRet = json_object_object_add(pJsonObj, ADM_COM_AIR_PROTOCOL_FIELD_SDKVERSION, json_object_new_string(pcSdkVersion));
        if (ADM_ECODE_SUCCESSED != iRet) 
        {
            log_e("add sdkVersion failed");
            iRet = ADM_ECODE_JSON_ADD_STRING_FAILED;
            break;
        }

        /** 添加扩展字段 */
        if (NULL != extsObjectStr) 
        {
            pExtsObject = json_tokener_parse(extsObjectStr);
            if (NULL != pExtsObject) 
            {
                iRet = json_object_object_add(pJsonObj, ADM_COM_AIR_PROTOCOL_FIELD_EXT, pExtsObject);
                if (ADM_ECODE_SUCCESSED != iRet) 
                {
                    log_e("add exts failed");
                    iRet = ADM_ECODE_JSON_ADD_STRING_FAILED;
                }
            }
        }
      
        /** 签名加密 */
        iRet = adm_ocs_protocolSignEncrypt(pJsonObj, ppcMessageBody);
        if (ADM_ECODE_SUCCESSED != iRet) 
        {
            log_e("adm_ocs_protocolSignEncrypt return error[%d]", iRet);
            iRet = ADM_ECODE_CAR_CLOUD_PROTOCOL_REQMESSAGE_ENCRYPT_FAILED;
        }
    } while (FALSE);

    /** 释放json对象 */
    if (NULL != pJsonObj) 
    {
        (void)json_object_put(pJsonObj);
    }
    
    ADM_COM_FUNC_EXIT();

    return iRet;
}

/**
 * @brief  解析上传URL
 * @param in DCHAR *respBodyFromCloud                       从云端返回的报文
 * @param out ADM_OCS_UPLOAD_URL_INFO_T *pstUploadUrlInfo   解析后的上传URL信息
 * @return D32S 0 成功，其他失败
 */
D32S adm_ocs_requestUploadUrlResponseParse(DCHAR* respBodyFromCloud, ADM_OCS_UPLOAD_URL_INFO_T *pstUploadUrlInfo)
{
    D32S retVal = ADM_ECODE_SUCCESSED;
    D32S respStatus = -1;
    json_object *respObject = NULL;
    json_object *dataObject = NULL;
    json_object *extsObject = NULL;

    ADM_COM_FUNC_ENTRY();

    /** 参数校验 */
    if (NULL == respBodyFromCloud || NULL == pstUploadUrlInfo)
    {
        log_e("input param invalid");
        return ADM_ECODE_INVALID_PARAM;
    }

    do
    {
        log_i("parse cloud resp message");
        /** 解析报文 */
        retVal = adm_ocs_parseCloudRespStatus(respBodyFromCloud, &respObject, &respStatus);
        if (ADM_ECODE_SUCCESSED != retVal)
        {
            log_e("adm_ocs_parseCloudRespStatus.error[%d]", retVal);
            break;
        }

        log_i("compare respStatus");
        /** 状态码校验 */
        if (ADM_OCS_AIR_STATUS_CODE_SUCCESSED != respStatus && ADM_OCS_AIR_STATUS_CODE_HAVE_UPGRADE_TASK != respStatus && ADM_OCS_AIR_STATUS_CODE_NO_UPGRADE_TASK != respStatus)
        {
            log_e("respStatus is not success, respStatus = %d", respStatus);

            /** 调用状态码回调函数 */
            if (NULL != g_cloudRespStatusCb)
            {
                g_cloudRespStatusCb(respStatus);
            }
            else
            {
                log_e("g_cloudRespStatusCb is NULL");
            }

            /** 转换状态码 */
            retVal = adm_ocs_otaStatusCodeTransfer(respStatus);
            break;
        }

        log_i("get data object");
        /** 获取data对象 */
        if ((json_bool)0 == json_object_object_get_ex(respObject, ADM_COM_AIR_PROTOCOL_FIELD_DATA, &dataObject))
        {
            log_e("get data object from json_object. failed");
            retVal = ADM_ECODE_JSON_GET_OBJECT_FAILED;
            break;
        }

        /** 获取uploadType */
        if (FALSE == adm_ocs_getJsonIntVal(dataObject, "uploadType", &pstUploadUrlInfo->iUploadType)) 
        {
            log_e("get uploadType value from json_object. failed");
            retVal = ADM_ECODE_JSON_GET_INT_FAILED;
            break;
        }

        /** 当uploadType为2时，获取filePath和tempUploadUrl */
        if (2 == pstUploadUrlInfo->iUploadType)
        {
            /** 获取filePath */
            if (FALSE == adm_ocs_getJsonStrVal(dataObject, "filePath", &pstUploadUrlInfo->pcFilePath)) 
            {
                log_e("get filePath value from data. failed");
                retVal = ADM_ECODE_JSON_GET_STRING_FAILED;
                break;
            }

            /** 获取tempUploadUrl */
            if (FALSE == adm_ocs_getJsonStrVal(dataObject, "tempUploadUrl", &pstUploadUrlInfo->pcUploadUrl)) 
            {
                log_e("get tempUploadUrl value from data. failed");
                retVal = ADM_ECODE_JSON_GET_STRING_FAILED;
                break;
            }
        }
    } while (FALSE);

    /** 释放json对象 */
    if (NULL != respObject)
    {
        (void)json_object_put(respObject);
    }

    ADM_COM_FUNC_EXIT();

    return retVal;
}
