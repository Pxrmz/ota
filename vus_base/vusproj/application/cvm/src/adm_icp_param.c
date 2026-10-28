/**
 * @file    adm_icp_param.c
 * @brief    
 * @details 组装icp请求参数
 * @author  wangqi
 * @version v0.1.0
 * @date    2025-1-12
 * @copyright chery
 */
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include "adm_file.h"
#include "adm_typedefs.h"
#include "adm_memory.h"
#include "adm_string.h"
#include "adm_com_errorCode.h"


#include "adm_com_lib.h"
#include "adm_icp_param.h"
#include "adm_ecs_parsePolicy.h"
#include "adm_iss_textRsa2048.h"
#include "adm_ocs_airProtocolAdapter.h"
#include "adm_ocs_airProtocolSecurity.h"


#if 0
#endif

/*!
 *******************************************************************************
 * 请求参数用json序列化.
 *
 * \note 此函数不添加sign信息
 * 
 * \param[out]  pstReqParam 模块基础配置参数
 * \retval      DCHAR*
 *******************************************************************************
 */
static DCHAR* __adm_icp_createJsonValConfigData(ADM_ICP_REQPARAM_T *pstReqParam)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    DCHAR *messageBody = NULL;
    json_object *getIcPObject = NULL;
    json_object* stringObject = NULL;
    json_object* d64Object = NULL;

    if (ADM_IS_NULL(pstReqParam))
    {
        log_e("Invalid input param");
        return NULL;
    }

    getIcPObject = json_object_new_object();
    if (ADM_IS_NULL(getIcPObject))
    {
        log_e("Faile to new getIcPObject json, err[%d]", ADM_ECODE_NEW_JSON_OBJECT_FAILED);
        return NULL;
    }

    do
    {
        //!< add timestamp
        d64Object = json_object_new_int64(pstReqParam->timestamp);
        if (ADM_IS_NULL(d64Object)) 
        {
            log_e("Failed to new timestamp json object");
            iRet = ADM_ECODE_NEW_JSON_OBJECT_FAILED;
            break;
        }
        iRet = json_object_object_add(getIcPObject, ADM_COM_AIR_PROTOCOL_FIELD_TIMESTAMP, d64Object);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to add timestamp json object");
            iRet = ADM_ECODE_JSON_ADD_INT64_FAILED;
            break;
        }

        //!< add sdkVersion
        stringObject = NULL;
        if (NULL != pstReqParam->sdkVersion)
        {
            stringObject = json_object_new_string(pstReqParam->sdkVersion);
        }
        if (ADM_IS_NULL(stringObject)) 
        {
            log_e("Failed to new sdkVersion json object");
            iRet = ADM_ECODE_NEW_JSON_OBJECT_FAILED;
            break;
        }
        iRet = json_object_object_add(getIcPObject, ADM_COM_AIR_PROTOCOL_FIELD_SDKVERSION, stringObject);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to add sdkVersion json object");
            iRet = ADM_ECODE_JSON_ADD_STRING_FAILED;
            break;
        }
        //!< json to string
        messageBody = adm_strdup(json_object_to_json_string(getIcPObject));
        if (ADM_IS_NULL(messageBody))
        {
            log_e("Failed to json to string");
            iRet = ADM_ECODE_JSON_TO_STRING_FAILED;
            break;
        }
        // TODO: exts 扩展参数有需要时使用

    } while (FALSE);
    if (ADM_IS_ERR(iRet))
    {
        return NULL;
    }
    return messageBody;
}

/*!
 *******************************************************************************
 * 请求参数用json序列化.
 *
 * \note 此函数不添加sign信息
 * 
 * \param[out]  pstReqParam 模块基础配置参数
 * \retval      DCHAR*
 *******************************************************************************
 */
static DCHAR* __adm_icp_configDataJsonAddSign(DCHAR *messageBody, DCHAR *msgSign)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    DCHAR *signMsg = NULL;
    json_object* jsonRoot = NULL;
    json_object* stringObject = NULL;

    if (ADM_IS_NULL(messageBody) || ADM_IS_NULL(msgSign))
    {
        log_e("Invalid input param");
        return NULL;
    }

    jsonRoot = json_tokener_parse(messageBody);
    if (ADM_IS_NULL(jsonRoot)) 
    {
        log_e("Failed string to json, string[%s]", messageBody);
        return NULL;
    }

    //!< add sign
    stringObject = json_object_new_string(msgSign);
    if (ADM_IS_NULL(stringObject)) 
    {
        log_e("Failed to new sign json object, err[%d]", 
            ADM_ECODE_NEW_JSON_OBJECT_FAILED);
        (void)json_object_put(jsonRoot);
        return NULL;
    }
    iRet = json_object_object_add(jsonRoot, ADM_COM_AIR_PROTOCOL_FIELD_SIGN, stringObject);
    if (ADM_IS_ERR(iRet))
    {
        log_e("Failed to add sign json object, err[%d]", 
            ADM_ECODE_JSON_ADD_STRING_FAILED);
        (void)json_object_put(stringObject);
        (void)json_object_put(jsonRoot);
        return NULL;
    }

    //!< json to string
    signMsg = adm_strdup(json_object_to_json_string(jsonRoot));
    if (ADM_IS_NULL(signMsg))
    {
        log_e("Failed to json to string, err[%d]", 
                ADM_ECODE_JSON_TO_STRING_FAILED);
        (void)json_object_put(jsonRoot);
        return NULL;
    }
    (void)json_object_put(jsonRoot);

    return signMsg;
}

/*!
 *******************************************************************************
 * 基础信息参数初始化.
 *
 * \note
 * 
 * \param[out]  pstReqParam 模块基础配置参数
 * \retval      D32S
 *******************************************************************************
 */
static D32S __adm_icp_signAndEnCrypt
(
    DCHAR *messageBody, 
    DCHAR *secret, 
    DCHAR *deviceId,
    DCHAR **enMessageBody
)
{
    DCHAR *msgSign = NULL;
    DCHAR *signMsg = NULL;
   
    if (ADM_IS_NULL(messageBody) || ADM_IS_NULL(secret) || ADM_IS_NULL(deviceId) || ADM_IS_NULL(enMessageBody))
    {
        log_e("Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }
    //!< 计算msg的sign
    msgSign = adm_ocs_ariProtocolSign(messageBody, secret, deviceId);
    if (ADM_IS_NULL(msgSign))
    {
        log_e("Failed to calculate message sign, err[%d]",
            ADM_ECODE_OTACFG_MESSAGE_SIGN_FAILED);
        return ADM_ECODE_OTACFG_MESSAGE_SIGN_FAILED;
    }
    //!< msg中添加sign
    signMsg = __adm_icp_configDataJsonAddSign(messageBody, msgSign);
    if (ADM_IS_NULL(signMsg))
    {
        adm_free(msgSign);
        log_e("Failed to add sign json to message");
        return ADM_ECODE_JSON_ADD_STRING_FAILED;
    }
    adm_free(msgSign);
    // TODO: 未加密
    *enMessageBody = signMsg;
    log_d("===> signMsg[%s]", signMsg);

#if 0
    //!< msg加密
    *enMessageBody = (DCHAR *)adm_iss_rsaPubEncrypt((D8U *)secret, (D8U *)signMsg, RSA_PKCS1_PADDING);
    if (ADM_IS_NULL(*enMessageBody))
    {
        adm_free(msgSign);
        adm_free(signMsg);
        return ADM_ECODE_CHECK_ENCRYPT_FAILED;
    }
    adm_free(msgSign);
    adm_free(signMsg);
#endif

    return ADM_ECODE_SUCCESSED;
}

#if 0
#endif

/*!
 *******************************************************************************
 * ICP创建请求平台消息体.
 *
 * \note
 * 
 * \param[in]   pstReqParam 请求参数
 * \param[out]  messageBody 创建好的消息体
 * \retval      D32S
 *******************************************************************************
 */
D32S adm_icp_createMessageBody(ADM_ICP_REQPARAM_T *pstReqParam, DCHAR **messageBody)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    DCHAR *msgJson = NULL;
    do
    {
        if (ADM_IS_NULL(pstReqParam) || ADM_IS_NULL(messageBody))
        {
            iRet = ADM_ECODE_INVALID_PARAM;
            log_e("Invalid input param");
            break;
        }
        if (ADM_IS_NULL(pstReqParam->sdkVersion))
        {
            iRet = ADM_ECODE_INVALID_PARAM;
            log_e("createMsg sdkVersion is null");
            break;
        }
        if (0 == pstReqParam->timestamp)
        {
            iRet = ADM_ECODE_INVALID_PARAM;
            log_e("createMsg timestamp is zero");
            break;
        }      
        msgJson = __adm_icp_createJsonValConfigData(pstReqParam);
        if (ADM_IS_NULL(msgJson))
        {
            iRet = ADM_ECODE_NO_MEMORY;
            break;
        }
        *messageBody = msgJson;       
        //!< 暂时取消http签名与验签
        iRet = __adm_icp_signAndEnCrypt(msgJson, pstReqParam->secret, 
                    pstReqParam->deviceId, messageBody);
        if (ADM_IS_ERR(iRet))
        {
            adm_free(msgJson);
            log_e("Failed to signAndEncrypt message, err[%d]", iRet);        
            break;
        }
        adm_free(msgJson);
    }while (FALSE);  
    return ADM_ECODE_SUCCESSED;
}

/*!
 *******************************************************************************
 * ICP解析平台响应的消息.
 *
 * \note
 * 
 * \param[in]   messageBody   平台响应的消息体
 * \param[in]   pstReqParam   请求参数(secret、deviceId)
 * \param[out]  pstRsIcpData  平台响应的Icp结构化数据
 * \retval      D32S
 *******************************************************************************
 */
D32S adm_icp_parseMessageBody
(
    DCHAR                  *messageBody, 
    ADM_ICP_REQPARAM_T     *pstReqParam,
    ADM_ICP_RSDATA_T *pstRsIcpData
)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    D32S status = 0;
    DCHAR *deMsgBody = NULL;
    json_object* jsonRoot = NULL;
    json_object *jsonData = NULL;

    json_type jsonType = json_type_null;

    if (ADM_IS_NULL(messageBody) || ADM_IS_NULL(pstReqParam) || ADM_IS_NULL(pstRsIcpData))
    {
        log_e("Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }
    log_d("===> messageBody[%s]", messageBody);
    jsonRoot = json_tokener_parse(messageBody);
    if (ADM_IS_NULL(jsonRoot)) 
    {
        adm_free(deMsgBody);
        log_e("Failed string to json, string[%s]", deMsgBody);
        return ADM_ECODE_STRING_TO_JSON_FAILED;
    }
    adm_free(deMsgBody);
    do 
    {
      
        //!< 先校验msg响应status
        iRet = adm_com_getJsonIntVal(jsonRoot, ADM_COM_AIR_PROTOCOL_FIELD_STATUS, &status);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to get json value, key[%s], err[%d]", 
                        ADM_COM_AIR_PROTOCOL_FIELD_STATUS, iRet);
            break;
        }
        
        //!< 解析云端响应的状态码
        iRet = adm_ocs_otaStatusCodeTransfer(status);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to check status[%d], iRet[%d]", status, iRet);
            break;
        }
        
        //!< 解析msg中data
        if (!json_object_object_get_ex(jsonRoot, ADM_COM_AIR_PROTOCOL_FIELD_DATA, &jsonData))
        {
            log_e("Failed to get json value, key[%s]", 
                        ADM_COM_AIR_PROTOCOL_FIELD_DATA);
            iRet = ADM_ECODE_JSON_GET_KEY_FAILED;
            break;
        }
        
        jsonType = json_object_get_type(jsonData);
        if (jsonType != json_type_object)
        {
            log_e("Json key[%s] type error", ADM_COM_AIR_PROTOCOL_FIELD_DATA);
            iRet = ADM_ECODE_JSON_KEY_TYPE_ERROR;
            break;
        }

        //!< 解析msg - data - icpVersion
        pstRsIcpData->icpVersion = adm_com_getJsonStrVal(jsonData, ADM_COM_AIR_PROTOCOL_FIELD_ICPVERSION);
        if (ADM_IS_NULL(pstRsIcpData->icpVersion))
        {
            log_e("Failed to get json value, key[%s]", ADM_COM_AIR_PROTOCOL_FIELD_ICPVERSION);
            iRet = ADM_ECODE_JSON_GET_STRING_FAILED;
            break;
        }else{
            log_i("icpVersion[%s]", pstRsIcpData->icpVersion);
        }
        //!< 解析msg - data - icpInfo
        pstRsIcpData->icpInfo = adm_strdup(json_object_get_string(jsonData));
        if (ADM_IS_NULL(pstRsIcpData->icpInfo))
        {
            log_e("Failed to get json value, key[%s]", ADM_COM_AIR_PROTOCOL_FIELD_ICPINFO);
            iRet = ADM_ECODE_JSON_GET_STRING_FAILED;
            break;
        }else{
            log_i("icpInfo[%s]", pstRsIcpData->icpInfo);
        }
    } while (FALSE);
    return iRet;
}




