#include "adm_vrs_log.h"
#include <time.h>
#include <string.h>

#include "json.h"
#include "adm_string.h"

#include "adm_com_errorCode.h"
#include "adm_vrs_registerManager.h"
#include "adm_vrs_registerInfoIn.h"
#include "adm_ocs_airProtocolAdapter.h"


#if 0
ADM_VRS_REGISTER_REQ_INFO_T* adm_vrs_newRegisterReqInfo(void)
{
    D32S iRet = ADM_ECODE_FAILED;
    ADM_VRS_REGISTER_REQ_INFO_T* pRegReqInfo = NULL;

    pRegReqInfo = adm_malloc(sizeof(ADM_VRS_REGISTER_REQ_INFO_T));
    if (NULL != pRegReqInfo) {
        (void)adm_memset(pRegReqInfo, 0x00, sizeof(ADM_VRS_REGISTER_REQ_INFO_T), sizeof(ADM_VRS_REGISTER_REQ_INFO_T));
    }

    return pRegReqInfo;
}

void adm_vrs_delRegisterReqInfo(const ADM_VRS_REGISTER_REQ_INFO_T* pRegReqInfo)
{
    if (NULL != pRegReqInfo) {
        if (NULL != pRegReqInfo->sdkVersion) {
            adm_free((pRegReqInfo->sdkVersion));
        }

        if (NULL != pRegReqInfo->vinCode) {
            adm_free((pRegReqInfo->vinCode));
        }

        if (NULL != pRegReqInfo->ext) {
            adm_free((pRegReqInfo->ext));
        }

        adm_free((pRegReqInfo));
    }
}


static D32S adm_vrs_createRegisterRequestInfoJsonVal(const ADM_VRS_REGISTER_REQ_INFO_T* registerReqInfo, json_object** ppJsonVal)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    D64S timestamp = 0;
    json_object* jsonVal = NULL;

    if (NULL == registerReqInfo) {
        log_e( "Parameter (registerReqInfo) verification failed, pointer invalid");
        return ADM_ECODE_INVALID_PARAM;
    }

    jsonVal = *ppJsonVal;
    if (NULL == jsonVal) {
        log_e( "Param (*ppJsonVal) verification failed, pointer invalied");
        return ADM_ECODE_INVALID_PARAM;
    }

    if (registerReqInfo->vinCode) {
        iRet = json_object_object_add(jsonVal, "vin", json_object_new_string(registerReqInfo->vinCode));
        if (ADM_ECODE_SUCCESSED != iRet) {
            log_w( "json_object_object_add string %s failed, reason of error code is %d", registerReqInfo->vinCode, iRet);
            iRet = ADM_ECODE_JSON_ADD_STRING_FAILED;
        }
    } else {
        log_e( "Parameter (registerReqInfo->vinCode) verification failed, pointer invalid");
        iRet = ADM_ECODE_INVALID_POINTER;
    }

    if (registerReqInfo->sdkVersion) {
        iRet = json_object_object_add(jsonVal, "sdkVersion", json_object_new_string(registerReqInfo->sdkVersion));
        if (ADM_ECODE_SUCCESSED != iRet) {
            log_w( "json_object_object_add string %s failed, reason of error code is %d", registerReqInfo->sdkVersion, iRet);
            iRet = ADM_ECODE_JSON_ADD_STRING_FAILED;
        }
    } else {
        log_e( "Parameter (registerReqInfo->sdkVersion) verification failed, pointer invalid");
        iRet = ADM_ECODE_INVALID_POINTER;
    }

    timestamp = time(NULL);
    json_object_object_add(jsonVal, "timestamp", json_object_new_uint64(timestamp));

    if (registerReqInfo->ext) {
        iRet = json_object_object_add(jsonVal, "ext", json_object_new_string(registerReqInfo->ext));
        if (ADM_ECODE_SUCCESSED != iRet) {
            log_e( "json_object_object_add string %s failed, reason of error code is %d", registerReqInfo->ext, iRet);
            iRet = ADM_ECODE_JSON_ADD_STRING_FAILED;
        }
    }

    return iRet;
}

static D32S adm_vrs_parseRegisterResponseInfoJsonVal(const DCHAR* pRegRspData, ADM_VRS_REGISTER_INFO_T** ppRegInfo, ADM_VRS_REGISTER_RSP_INFO_T** ppRegRspInfo)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    json_object* jsonObj = NULL;
    json_object* jsonStr = NULL;
    const DCHAR* jsonValue = NULL;
    ADM_VRS_REGISTER_INFO_T* pRegInfo = NULL;
    ADM_VRS_REGISTER_RSP_INFO_T* pRegRspInfo = NULL;

    if (NULL == pRegRspData) {
        log_e( "Parameter (pRegRspData) verification failed, pointer invalid");
        return ADM_ECODE_INVALID_PARAM;
    }

    if (NULL == *ppRegInfo) {
        log_e( "Parameter (ppRegInfo) verification failed, pointer invalid");
        return ADM_ECODE_INVALID_PARAM;
    }

    if (NULL == *ppRegRspInfo) {
        log_e( "Parameter (pRegRspOutData) verification failed, pointer invalid");
        return ADM_ECODE_INVALID_PARAM;
    }

    pRegInfo = *ppRegInfo;
    pRegRspInfo = *ppRegRspInfo;

    jsonObj = json_tokener_parse(pRegRspData);
    if (NULL == jsonObj) {
        iRet = ADM_ECODE_STRING_TO_JSON_FAILED;
    } else {
        jsonStr = json_object_object_get(jsonObj, "deviceId");
        if (NULL != jsonStr) {
            jsonValue = json_object_get_string(jsonStr);
            if (NULL != jsonValue) {
                pRegInfo->deviceId = strdup(jsonValue);
            } else {
                iRet = ADM_ECODE_JSON_GET_STRING_FAILED;
                log_w( "json_object_object_get secret failed");
            }
        } else {
            iRet = ADM_ECODE_JSON_GET_STRING_FAILED;
            log_w( "json_object_object_get secret failed");
        }

        jsonStr = json_object_object_get(jsonObj, "secret");
        if (NULL != jsonStr) {
            jsonValue = json_object_get_string(jsonStr);
            if (NULL != jsonValue) {
                pRegInfo->secret = strdup(jsonValue);
            } else {
                iRet = ADM_ECODE_JSON_GET_STRING_FAILED;
                log_w( "json_object_object_get secret failed");
            }
        } else {
            iRet = ADM_ECODE_JSON_GET_STRING_FAILED;
            log_e( "json_object_object_get secret failed");
        }

        jsonStr = json_object_object_get(jsonObj, "ext");
        if (NULL != jsonStr) {
            jsonValue = json_object_get_string(jsonStr);
            if (NULL != jsonValue) {
                pRegRspInfo->ext = strdup(jsonValue);
            } else {
                log_e( "json_object_object_get ext failed, ext is not required");
            }
        } else {
            log_i( "json_object_object_get ext failed, ext is not required");
        }

        json_object_put(jsonObj);
    }

    return iRet;
}

static D32S adm_vrs_dealRegisterResponseData(const DCHAR* pRegRspOutData, const ADM_VRS_REGISTER_REQ_INFO_T* pRegReqInfo, ADM_VRS_REGISTER_RSP_INFO_T** ppRegRspInfo)
{
    D32S iRet = ADM_ECODE_FAILED;
    ADM_VRS_REGISTER_INFO_T* pRegInfo = NULL;

    if (NULL == pRegRspOutData) {
        log_e( "Parameter (pRegRspOutData) verification failed, pointer invalid");
        return ADM_ECODE_INVALID_PARAM;
    }

    if (NULL == pRegReqInfo) {
        log_e( "Paramter (pRegReqInfo) verification failed, pointer invalid");
        return ADM_ECODE_INVALID_PARAM;
    }

    if (NULL == *ppRegRspInfo) {
        log_e( "Parameter (pRegRspOutData) verification failed, pointer invalid");
        return ADM_ECODE_INVALID_PARAM;
    }

    pRegInfo = adm_vrs_newRegisterInfo();
    if (NULL == pRegInfo) {
        iRet = ADM_ECODE_APPLY_MEMORY_FAILED;
        log_e( "adm_vrs_newRegisterInfo failed, reason of error code is %d", iRet);
    } else {
        iRet = adm_vrs_parseRegisterResponseInfoJsonVal(pRegRspOutData, &pRegInfo, ppRegRspInfo);
        if (ADM_ECODE_SUCCESSED != iRet) {
            log_w( "adm_vrs_parseRegisterResponseInfoJsonVal failed, reason of error code is %d", iRet);
        } else {
            if (NULL != pRegReqInfo->vinCode) {
                pRegInfo->vinCode = strdup(pRegReqInfo->vinCode);
            }
            iRet = adm_vrs_setRegisterInfo(pRegInfo);
            if (ADM_ECODE_SUCCESSED != iRet) {
                log_e( "adm_vrs_setRegisterInfo failed, reason of error code is %d", iRet);
            }
        }

        adm_vrs_delRegisterInfo(pRegInfo);
    }

    return iRet;
}

#endif

ADM_VRS_REGISTER_RSP_INFO_T* adm_vrs_newRegisterRspInfo(void)
{
    D32S iRet = ADM_ECODE_FAILED;
    ADM_VRS_REGISTER_RSP_INFO_T* pRegRspInfo = NULL;

    pRegRspInfo = adm_malloc(sizeof(ADM_VRS_REGISTER_RSP_INFO_T));
    if (NULL != pRegRspInfo) {
        (void)adm_memset(pRegRspInfo, 0x00, sizeof(ADM_VRS_REGISTER_RSP_INFO_T), sizeof(ADM_VRS_REGISTER_RSP_INFO_T));
    }

    return pRegRspInfo;
}

void adm_vrs_delRegisterRspInfo(ADM_VRS_REGISTER_RSP_INFO_T* pRegRspInfo)
{
    if (NULL != pRegRspInfo) {
        if (NULL != pRegRspInfo->ext) {
            adm_free((pRegRspInfo->ext));
        }

        adm_free(pRegRspInfo);
    }
}

D32S adm_vrs_isRegisteredOK(const DCHAR* vinCode)
{
    D32S iRet = ADM_ECODE_FAILED;
    ADM_VRS_REGISTER_INFO_T* registerInfo = NULL;

    if (NULL == vinCode) {
        log_e( "param vinCode is NULL");
        return ADM_ECODE_INVALID_PARAM;
    }

    registerInfo = adm_vrs_newRegisterInfo();
    if (NULL == registerInfo) {
        iRet = ADM_ECODE_APPLY_MEMORY_FAILED;
        log_w( "adm_vrs_newRegisterInfo failed, reason of error code is %d", iRet);
    } else {
        iRet = adm_vrs_getRegisterInfo(registerInfo);
        if (ADM_ECODE_SUCCESSED != iRet) {
            log_w( "adm_vrs_getRegisterInfo failed, reason of error code is %d", iRet);
        } else {
            if (NULL == registerInfo->vinCode) {
                log_w( "vin code is empty in the registration information");
                iRet = ADM_ECODE_REGISTER_INFO_VIN_EMPTY;
            } else {
                if (ADM_ECODE_SUCCESSED == adm_strcmp(registerInfo->vinCode, vinCode)) {
                    iRet = ADM_ECODE_SUCCESSED;
                } else {
                    log_w( "Vechicle code replacement, expect %s, but actual vin code is %s", registerInfo->vinCode, vinCode);
                    iRet = ADM_ECODE_REGISTER_VIN_REPLACEMENT;
                }
            }
        }
        adm_vrs_delRegisterInfo(registerInfo);
    }

    return iRet;
}

static D32S adm_vrs_dealRegisterResponseData(const ADM_OCS_REGISTER_REQUEST_INFO_T* pstRegisterRequestInfo, ADM_OCS_REGISTER_RESPONSE_INFO_T* pstRegisterResponseInfo)
{
    D32S iRet = ADM_ECODE_FAILED;
    ADM_VRS_REGISTER_INFO_T* pRegInfo = NULL;

    if (NULL == pstRegisterRequestInfo || NULL == pstRegisterResponseInfo) {
        log_e( "input params invalid");
        return ADM_ECODE_INVALID_PARAM;
    }

    pRegInfo = adm_vrs_newRegisterInfo();
    if (NULL == pRegInfo) {
        iRet = ADM_ECODE_APPLY_MEMORY_FAILED;
        log_e( "adm_vrs_newRegisterInfo failed, reason of error code is %d", iRet);
    } else {
        do {
            if (NULL != pstRegisterRequestInfo->vinCode) {
                pRegInfo->vinCode = strdup(pstRegisterRequestInfo->vinCode);
            } else {
                break;
            }

            if (NULL != pstRegisterResponseInfo->deviceId) {
                pRegInfo->deviceId = strdup(pstRegisterResponseInfo->deviceId);
            } else {
                break;
            }

            if (NULL != pstRegisterResponseInfo->secret) {
                pRegInfo->secret = strdup(pstRegisterResponseInfo->secret);
            } else {
                break;
            }

            if (NULL != pstRegisterResponseInfo->pcVehlType) {
                pRegInfo->pcVehlType = strdup(pstRegisterResponseInfo->pcVehlType);
            } else {
                break;
            }

            iRet = adm_vrs_setRegisterInfo(pRegInfo);
            if (ADM_ECODE_SUCCESSED != iRet) {
                log_e( "adm_vrs_setRegisterInfo failed, reason of error code is %d", iRet);
            }

        } while(FALSE);

        adm_vrs_delRegisterInfo(pRegInfo);
    }

    //设置平台响应时间戳到kv
    adm_vrs_setTimestampInfo(pstRegisterResponseInfo);

    return iRet;
}


D32S adm_vrs_doRegister(const ADM_OCS_REGISTER_REQUEST_INFO_T* pstRegisterRequestInfo, ADM_VRS_REGISTER_RSP_INFO_T* pstVrsResponseInfo)
{

    D32S iRet = ADM_ECODE_FAILED;
    DCHAR* pRespOutData = NULL;
    
    DCHAR* registerMessageBody = NULL;
    DSIZE statusCode = 0;
    ADM_OCS_REGISTER_RESPONSE_INFO_T* pstRegisterResponseInfo = NULL;
    const DCHAR* registerUrl = NULL;
    DCHAR eventContext[512] = {0};


    //!< 输入参数校验
    if (NULL == pstRegisterRequestInfo) {
        log_e( "pstRegisterRequestInfo is NULL");
        return ADM_ECODE_INVALID_POINTER;
    }

    do {
        //!< ZI_001
        if (NULL != pstRegisterRequestInfo->fnEventAddCb) {
            pstRegisterRequestInfo->fnEventAddCb(3, 1, ADM_OCS_EVENT_LEVEL_TRACE, 0, NULL);
        }
        //!< 输入参数校验
        if (NULL == pstRegisterRequestInfo->sdkVersion) {
            iRet = ADM_ECODE_INVALID_PARAM;
            log_e("Input param pstRegisterRequestInfo->sdkVersion is nil, iRet:%d", iRet);
            break;
        }

        if (NULL == pstRegisterRequestInfo->vinCode) {
            iRet = ADM_ECODE_INVALID_PARAM;
            log_e("Input param pstRegisterRequestInfo->vinCode is nil, iRet:%d", iRet);
            break;
        }

        if (NULL == pstRegisterRequestInfo->fnBusinessCommCb) {
            iRet = ADM_ECODE_INVALID_PARAM;
            log_e("Input param pstRegisterRequestInfo->reqConnectSerCb is nil, iRet:%d", iRet);
            break;
        }

        //!< ZI_002
        if (NULL != pstRegisterRequestInfo->fnEventAddCb) {
            pstRegisterRequestInfo->fnEventAddCb(3, 2, ADM_OCS_EVENT_LEVEL_TRACE, 0, NULL);
        }
        //!<  create register message body 
        registerMessageBody = adm_ocs_registerProtocolAdapter(pstRegisterRequestInfo, pstRegisterRequestInfo->vinCode);
        if (NULL == registerMessageBody) {
            iRet = ADM_ECODE_REGISTER_CREATE_MESSAGE_BODY_FAILED;
            log_e( "adm_ocs_registerProtocolAdapter failed, iRet:%d", iRet);
            break;
        }

        log_i("adm_requestConnectServer registerMessageBody:%s",
                registerMessageBody);

        //!< ZI_006
        if (NULL != pstRegisterRequestInfo->fnEventAddCb) {
            pstRegisterRequestInfo->fnEventAddCb(3, 6, ADM_OCS_EVENT_LEVEL_TRACE, 0, NULL);
        }
        //!< 使用句柄执行请求OTA平台注册
        iRet = pstRegisterRequestInfo->fnBusinessCommCb(ADM_OCS_BUSINESS_TYPE_REGISTER, registerMessageBody, &pRespOutData, &statusCode);
        if (ADM_ECODE_SUCCESSED != iRet) {
            log_e( "reqConnectSerCb failed, reason of code is %d, statusCode:%ld", iRet, statusCode);
            //!< ZI_007
            if (NULL != pstRegisterRequestInfo->fnEventAddCb) {
                pstRegisterRequestInfo->fnEventAddCb(3, 7, ADM_OCS_EVENT_LEVEL_ERROR, iRet, NULL);
            }
            break;
        }
        //!< ZI_008
        if (NULL != pstRegisterRequestInfo->fnEventAddCb) {
            (void)adm_memset(eventContext, 0x00, sizeof(eventContext), sizeof(eventContext));
            (void)adm_snprintf(eventContext, sizeof(eventContext), "register info is %s, statusCode:%ld", pRespOutData, statusCode);
            pstRegisterRequestInfo->fnEventAddCb(3, 8, ADM_OCS_EVENT_LEVEL_TRACE, iRet, eventContext);
        }
        log_i( "reqConnectSerCb successed, register info is %s, statusCode:%ld", pRespOutData, statusCode);
        
        //!< 注册响应信息句柄初始化
        pstRegisterResponseInfo = adm_ocs_newRegisterResponseInfo();
        if (NULL == pstRegisterResponseInfo) {
            iRet = ADM_ECODE_APPLY_MEMORY_FAILED;
            log_e( "adm_ocs_newRegisterResponseInfo failed");
            //!< ZI_009
            if (NULL != pstRegisterRequestInfo->fnEventAddCb) {
                pstRegisterRequestInfo->fnEventAddCb(3, 9, ADM_OCS_EVENT_LEVEL_ERROR, iRet, NULL);
            }
            break;
        }
        //!< ZI_010
        if (NULL != pstRegisterRequestInfo->fnEventAddCb) {
            pstRegisterRequestInfo->fnEventAddCb(3, 10, ADM_OCS_EVENT_LEVEL_TRACE, iRet, NULL);
        }

        //!< ZI_011
        if (NULL != pstRegisterRequestInfo->fnEventAddCb) {
            pstRegisterRequestInfo->fnEventAddCb(3, 11, ADM_OCS_EVENT_LEVEL_TRACE, iRet, NULL);
        }
        //!< 解析注册响应
        iRet = adm_ocs_registerProtocolParse(pRespOutData, (D32S)statusCode, pstRegisterRequestInfo->vinCode, pstRegisterResponseInfo);
        if (ADM_ECODE_SUCCESSED != iRet) 
        {
            log_e( "adm_ocs_registerProtocolParse failed, iRet:%d", iRet);

            if(ADM_ECODE_VEHICLE_NOT_EXIST == iRet)
            {
                log_e( "adm_vrs_rmoveRegisterInfo");
                adm_vrs_removeRegisterInfo();

            }
            //!< ZI_022
            if (NULL != pstRegisterRequestInfo->fnEventAddCb) {
                pstRegisterRequestInfo->fnEventAddCb(3, 22, ADM_OCS_EVENT_LEVEL_ERROR, iRet, NULL);
            }
            break;
        }
        //!< ZI_021
        if (NULL != pstRegisterRequestInfo->fnEventAddCb) {
            pstRegisterRequestInfo->fnEventAddCb(3, 21, ADM_OCS_EVENT_LEVEL_TRACE, iRet, NULL);
        }

        //!< ZI_023
        if (NULL != pstRegisterRequestInfo->fnEventAddCb) {
            pstRegisterRequestInfo->fnEventAddCb(3, 23, ADM_OCS_EVENT_LEVEL_TRACE, iRet, NULL);
        }
        //!< 处理注册响应
        iRet = adm_vrs_dealRegisterResponseData(pstRegisterRequestInfo, pstRegisterResponseInfo);
        if (ADM_ECODE_SUCCESSED != iRet) {
            log_e( "adm_vrs_dealRegisterResponseData failed, iRet:%d", iRet);
            //!< ZI_025
            if (NULL != pstRegisterRequestInfo->fnEventAddCb) {
                pstRegisterRequestInfo->fnEventAddCb(3, 25, ADM_OCS_EVENT_LEVEL_ERROR, iRet, NULL);
            }
            break;
        }
        //!< ZI_024
        if (NULL != pstRegisterRequestInfo->fnEventAddCb) {
            pstRegisterRequestInfo->fnEventAddCb(3, 24, ADM_OCS_EVENT_LEVEL_TRACE, iRet, NULL);
        }

        //!< 返回注册信息
        if (NULL != pstVrsResponseInfo) {
            if (NULL != pstRegisterResponseInfo->ext) {
                pstVrsResponseInfo->ext = strdup(pstRegisterResponseInfo->ext);
            }
        }

    } while(FALSE);
    
    //!< 释放申请的内存信息
    if (NULL != pRespOutData){
        adm_free(pRespOutData);
    }
    if (NULL != registerMessageBody) {
        adm_free(registerMessageBody);
    }
    adm_ocs_delRegisterResponseInfo(pstRegisterResponseInfo);
    return iRet;
}
