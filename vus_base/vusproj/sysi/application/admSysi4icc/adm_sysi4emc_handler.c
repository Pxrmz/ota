#include "adm_sysi4emc_handler.h"
#include <stdio.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <unistd.h>
#include <pthread.h>

#include "adm_sysi4all_data.h"
#include "adm_sysi4hmi_common.h"
#include "adm_typedefs.h"
#include "adm_memory.h"
#include "adm_string.h"
#include "json_types.h"
#include "json_object.h"
#include "json_tokener.h"
#include "adm_debug.h"

#include "adm_com_errorCode.h"
#include "adm_arpc_service.h"
#include "adm_soa_cfg.h"
#include "adm_soa.h"

/**
 * @brief 车辆状态控制
 * 
 * @param[in] eCtrlType     描述:车辆状态控制类型定义 \n
 * 范围值: \n
 * 单位: \n
 * 
 * @param[out] errorCode     描述:车辆状态控制结果 0:Successed None-0:Failed \n
 * 范围值: \n
 * 单位: \n
 * 
 * @return D32S 
 */
D32S adm_sysi4hmi_emcVehicleStateCtrl(const ADM_SYSI4HMI_EMC_VEHICLE_STATE_CTRL_TYPE_E eCtrlType, D32S* errorCode)
{
    log_i("adm_sysi4hmi_emcVehicleStateCtrl start");

    DCHAR *resp = NULL;
    D32U respSize = 0;
    json_object* pObj = DNULL;
    DCHAR* pReqStr = DNULL;
    D32S  buildRet = ADM_ECODE_SUCCESSED, ret = ADM_ECODE_FAILED;

    do
    {
        json_new_object(pObj);
        json_add_int(pObj, "eCtrlType", (int32_t)eCtrlType);
        pReqStr = adm_strdup(json_object_get_string(pObj));
    }
    while (FALSE);
end_build_json:
    set_json_err_str(pReqStr);

    log_i("pReqStr:%s", pReqStr);
    (void)adm_arpc_request_service(UIM2TBOX_INSTANCE_ID, ADM_UIM_ENGINEER_MODE_SERVICE_ID,
                             ADM_UIM_HMI_ENGINEER_MODE_VEHICLE_STATE_CTRL_METHOD_ID, 30, pReqStr, adm_strlen(pReqStr) + (D32U)1,
                             &resp, &respSize);

    log_i("emc vehicle state ctrl :%d recv resp data:%s, size:%d", eCtrlType, resp, respSize);

    if (DNULL != pObj)
    {
        json_object_put(pObj);
    }
    adm_free(pReqStr);

    if (respSize == (D32U)0)
    {
        log_e("emc vehicle state ctrl :%d recv resp size: %d", eCtrlType, respSize);
        return ADM_ECODE_INVALID_PARAM;
    }

    struct json_object *root = NULL;
    struct json_object *pErrorCodeJson = NULL;
    root = json_tokener_parse(resp);
    adm_free(resp);
    if (NULL == root)
    {
        log_e("invalid json format resp\n");
        return ADM_ECODE_INVALID_POINTER;
    }

    pErrorCodeJson = json_object_object_get(root, "errorCode");
    if (DNULL != pErrorCodeJson)
    {
        (*errorCode) = json_object_get_int(pErrorCodeJson);
        ret = *errorCode;
    }
    
    (void)json_object_put(root);
    log_i("[SendRequest] :: emc vehicle state ctrl :%d,(*errorCode)=%d\r\n", eCtrlType, (*errorCode));

    return ret;
}


D32S adm_sysi4hmi_emcOTAServerSwitch(D32U* pServerType, D32S* errorCode)
{
    log_i("adm_sysi4hmi_emcOTAServerSwitch start");

    DCHAR *resp = NULL;
    D32U respSize = 0;
    json_object* pObj = DNULL;
    DCHAR* pReqStr = DNULL;
    D32S  buildRet = ADM_ECODE_SUCCESSED, ret = ADM_ECODE_FAILED;
    
    do
    {
        json_new_object(pObj);
        pReqStr = adm_strdup(json_object_get_string(pObj));
    }
    while (FALSE);
end_build_json:
    set_json_err_str(pReqStr);
    log_i("pReqStr:%s", pReqStr);

    (void)adm_arpc_request_service(UIM2TBOX_INSTANCE_ID, ADM_UIM_ENGINEER_MODE_SERVICE_ID,
                             ADM_UIM_HMI_ENGINEER_MODE_OTA_SERVER_SWITCH_METHOD_ID, 10, pReqStr, adm_strlen(pReqStr) + (D32U)1,
                             &resp, &respSize);

    log_i("emc ota server switch recv resp data:%s, size:%d", resp, respSize);

    if (DNULL != pObj)
    {
        json_object_put(pObj);
    }
    adm_free(pReqStr);

    if (respSize == (D32U)0)
    {
        log_e("emc ota server switch recv resp size: %d", respSize);
        return buildRet;
    }

    struct json_object *root = NULL;
    struct json_object *pErrorCodeJson = NULL;
    struct json_object *pServerTypeJson = DNULL;

    root = json_tokener_parse(resp);
    adm_free(resp);
    if (NULL == root)
    {
        log_e("invalid json format resp\n");
        return ADM_ECODE_INVALID_POINTER;
    }

    pErrorCodeJson = json_object_object_get(root, "errorCode");
    if (DNULL != pErrorCodeJson)
    {
        (*errorCode) = json_object_get_int(pErrorCodeJson);
        ret = *errorCode;
    }

    pServerTypeJson = json_object_object_get(root, "serverType");
    if (DNULL != pServerTypeJson)
    {
        (*pServerType) = json_object_get_int(pServerTypeJson);
    }

    (void)json_object_put(root);
    log_i("[SendRequest] :: emc ota server switch :%d, (*errorCode)=%d\r\n", (*pServerType), (*errorCode));

    return ret;
}

D32S adm_sysi4hmi_emcOTAServerConnectOk(D32S* errorCode)
{
    log_i("adm_sysi4hmi_emcOTAServerConnectOk start");

    DCHAR *resp = NULL;
    D32U respSize = 0;
    json_object* pObj = DNULL;
    DCHAR* pReqStr = DNULL;
    D32S  buildRet = ADM_ECODE_SUCCESSED, ret = ADM_ECODE_FAILED;

    do
    {
        json_new_object(pObj);
        pReqStr = adm_strdup(json_object_get_string(pObj));
    }
    while (FALSE);
end_build_json:
    set_json_err_str(pReqStr);

    log_i("pReqStr:%s", pReqStr);
    (void)adm_arpc_request_service(UIM2TBOX_INSTANCE_ID, ADM_UIM_ENGINEER_MODE_SERVICE_ID,
                             ADM_UIM_HMI_ENGINEER_MODE_OTA_SERVER_CONNECT_OK_METHOD_ID, 20, pReqStr, adm_strlen(pReqStr) + (D32U)1,
                             &resp, &respSize);

    log_i("emc ota server connect recv resp data:%s, size:%d", resp, respSize);

    if (DNULL != pObj)
    {
        json_object_put(pObj);
    }
    adm_free(pReqStr);

    if (respSize == (D32U)0)
    {
        log_e("emc ota server connect recv resp size: %d", respSize);
        return ADM_ECODE_INVALID_PARAM;
    }

    struct json_object *root = NULL;
    struct json_object *pErrorCodeJson = NULL;
    root = json_tokener_parse(resp);
    adm_free(resp);
    if (NULL == root)
    {
        log_e("invalid json format resp\n");
        return ADM_ECODE_INVALID_POINTER;
    }

    pErrorCodeJson = json_object_object_get(root, "errorCode");
    if (DNULL != pErrorCodeJson)
    {
        (*errorCode) = json_object_get_int(pErrorCodeJson);
        ret = *errorCode;
    }
    
    (void)json_object_put(root);
    log_i("[SendRequest] :: emc ota server connect,(*errorCode)=%d\r\n", (*errorCode));

    return ret;
}

D32S adm_sysi4hmi_emcOTAVehicleAppCtrl(const ADM_SYSI4HMI_EMC_OTA_APP_CTRL_TYPE_E eCtrlType, D32S* errorCode)
{
    log_i("adm_sysi4hmi_emcVehicleStateCtrl start");

    DCHAR *resp = NULL;
    D32U respSize = 0;
    json_object* pObj = DNULL;
    DCHAR* pReqStr = DNULL;
    D32S  buildRet = ADM_ECODE_SUCCESSED, ret = ADM_ECODE_FAILED;
    D32S iTimeOutS = 8;     //!< 8s 超时时间
    
    do
    {
        json_new_object(pObj);
        json_add_int(pObj, "eCtrlType", (int32_t)eCtrlType);
        pReqStr = adm_strdup(json_object_get_string(pObj));
    }
    while (FALSE);
end_build_json:
    set_json_err_str(pReqStr);

    if (ADM_SYSI4HMI_EMC_OTA_APP_CTRL_STOP_UCM == eCtrlType || ADM_SYSI4HMI_EMC_OTA_APP_CTRL_CLEAN_DATA == eCtrlType)
    {
        iTimeOutS = 2;
    }
    else if (ADM_SYSI4HMI_EMC_OTA_APP_CTRL_GET_LOG == eCtrlType)
    {
        iTimeOutS = 60 * 2;
    }
    log_i("pReqStr:%s", pReqStr);
    (void)adm_arpc_request_service(UIM2TBOX_INSTANCE_ID, ADM_UIM_ENGINEER_MODE_SERVICE_ID,
                             ADM_UIM_HMI_ENGINEER_MODE_VEHICLE_APP_CTRL_METHOD_ID, iTimeOutS, pReqStr, adm_strlen(pReqStr) + (D32U)1,
                             &resp, &respSize);

    log_i("emc vehicle app ctrl :%d recv resp data:%s, size:%d", eCtrlType, resp, respSize);

    if (DNULL != pObj)
    {
        json_object_put(pObj);
    }
    adm_free(pReqStr);

    if (respSize == (D32U)0)
    {
        log_e("emc vehicle app ctrl :%d recv resp size: %d", eCtrlType, respSize);
        if (ADM_SYSI4HMI_EMC_OTA_APP_CTRL_STOP_UCM == eCtrlType || ADM_SYSI4HMI_EMC_OTA_APP_CTRL_CLEAN_DATA == eCtrlType)
        {
            return buildRet;
        }
        else
        {
            return ADM_ECODE_INVALID_PARAM;
        }
    }

    struct json_object *root = NULL;
    struct json_object *pErrorCodeJson = NULL;
    root = json_tokener_parse(resp);
    adm_free(resp);
    if (NULL == root)
    {
        log_e("invalid json format resp\n");
        return ADM_ECODE_INVALID_POINTER;
    }

    pErrorCodeJson = json_object_object_get(root, "errorCode");
    if (DNULL != pErrorCodeJson)
    {
        (*errorCode) = json_object_get_int(pErrorCodeJson);
        ret = *errorCode;
    }
    
    (void)json_object_put(root);
    log_i("[SendRequest] :: emc vehicle app ctrl :%d,(*errorCode)=%d\r\n", eCtrlType, (*errorCode));

    return ret;
}
