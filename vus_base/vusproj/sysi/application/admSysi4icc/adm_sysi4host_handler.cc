#include <string>
#include <list>
#ifdef __QNX__
#include <sys/statvfs.h>
#else
#include <sys/vfs.h>
#endif
#include "adm_sysi4host_common.h"
#include "adm_cfg_api.h"
#include "adm_sysi4all_loader.h"
#include "adm_sysi4icc.h"
#include "adm_sysi4host_jni.h"
#include "adm_sysi4all_data.h"
#include "json_types.h"
#include "json_object.h"
#include "json.h"
#include "json_tokener.h"
#include "adm_sysi4host_handler_lvm.h"
#include "adm_com_lib.h"
#include "adm_sysi4hmi_common.h"
#include "adm_arpc_service.h"
#include "adm_soa_cfg.h"
#include "adm_soa.h"
#include "adm_sysi_keylog.h"

ADM_SYSI4HOST_JNI_CALLBACK_T stSysi4hostJniCb = {};


#define ADM_VEH_VIN_ID                              "0x301"     // VIN码
#define ADM_VEH_SPEED_ID                            "0x302"     // 车速
#define ADM_PERCENT_OF_STORAGE_BAT_ID               "0x304"     // 蓄电池电量值（百分比）
#define ADM_PERCENT_OF_TRACTION_BAT_ID              "0x305"     // 动力电池电量值（百分比）
#define ADM_POWER_MODE_ID                           "0x31C"     // 整车电源模式状态
#define ADM_VEH_GEAR_ID                             "0x307"     // 行车档位状态
#define ADM_VEH_PARKING_STATE_ID                    "0x308"     // 手刹状态
#define ADM_VEH_CHARHE_OR_NOT_ID                    "0x309"     // 充电状态
#define ADM_VEH_BATTERY_VOLTAGE_ID                  "0x30A"     // 蓄电池电压值
#define ADM_VEH_ENGINE_STATE_ID                     "0x30B"     // 发动机状态
#define ADM_VEH_OBD_STATE_ID                        "0x30E"     // 主驾OBD占用状态
#define ADM_VEH_DISCHARGE_STATUS_ID                 "0x310"     // 放电状态
#define ADM_VEH_LOCK_STATUS_ID                      "0x314"     // 锁车状态---delete
#define ADM_VEH_READY_STATUS_ID                     "0x311"     // 动力就绪状态
#define ADM_VEH_POWER_BATTERY_SUPPLY_STATUS_ID      "0x312"     // 动力电池供电状态---delete
#define ADM_VEH_ECALL_OPEN_STATUS_ID                "0x313"     // E-CALL开关状态---delete
#define ADM_VEH_THEME_ID                            "0x330"     // 座舱主题
#define ADM_VEH_ANTI_THEFT_STATUS_ID                "0x331"     // 整车设防/解防状态
#define ADM_VEH_POWER_TYPE_ID                       "0x332"     // 动力类型
#define ADM_VEH_SOC_OF_TRACTION_BAT_ID              "0x333"     // 动力电池电量（续航公里数）
#define ADM_VEH_SOC_OF_STORAGE_BAT_ID               "0x334"     // 蓄电池电量（毫安时）
#define ADM_VEH_COUNT_OF_SCREEN_ID                  "0x335"     // 屏幕数量
#define ADM_VEH_SCREEN_OF_OTA_ID                    "0x336"     // OTA对应屏幕
#define ADM_VEH_NETWORK_TYPE_ID                     "0x337"     // 网络类型
#define ADM_VEH_BKLIGHT_MODE_ID                     "0x338"     // 背光模式
#define ADM_VEH_TIMEZONE_ID                         "0x339"     // 时区
#define ADM_VEH_ARCHITECTURE_MODE_ID                "0x33A"     // 整车架构类型
#define ADM_VEH_LFHDOORSTS_MODE_ID                  "0x33B"     // 主驾门开状态


// 定义结构体来表示键值对
typedef struct 
{
    DCHAR aVehicleStatus[10];   // 车辆状态
    D32S  iValue;               // 状态值
}ADM_SYSI4HOST_VEHICLE_STATUS_VALUE;

/** 车辆状态*/
static ADM_SYSI4HOST_VEHICLE_STATUS_VALUE g_stVehicleStatus[] = {
    {ADM_POWER_MODE_ID, -1},
    {ADM_VEH_GEAR_ID, -1},
    {ADM_VEH_LOCK_STATUS_ID, -1},
    {ADM_VEH_LFHDOORSTS_MODE_ID, -1}
};

// 更新指定车辆状态值
DBOOL updateVehicleStatusValue(const DCHAR *pVehicleStatus, D32S iNewValue) 
{
    D32S iSize = sizeof(g_stVehicleStatus) / sizeof(g_stVehicleStatus[0]);
    
    for (D32S i = 0; i < iSize; i++) 
    {
        if (0 == adm_strcmp(g_stVehicleStatus[i].aVehicleStatus, pVehicleStatus)) 
        {
            if (g_stVehicleStatus[i].iValue == iNewValue)
            {
                log_i("[%s] value [%d] is same, no need to update", g_stVehicleStatus[i].aVehicleStatus, iNewValue);
                return DFALSE;
            }
            else
            {
                g_stVehicleStatus[i].iValue = iNewValue;
                log_i("update VehicleStatus[%s],value[%d]",g_stVehicleStatus[i].aVehicleStatus, g_stVehicleStatus[i].iValue);
                return DTRUE;
            }
        }
    }

    return DFALSE;
}


static D32S sysi4host_ensureCheckSubedEventSuccess(D16U usServiceId, D16U usEventId, D16U usTimeOut)
{
    D32S iRet = ADM_ECODE_FAILED;
    D16U usTimeCount = 0;

    do
    {
        /* 查询事件是否订阅成功，1：订阅成功；0：未订阅成功 */
        iRet = adm_arpc_checkIfClientSubedEvent(adm_soa_getHostInstanceId(),
                                                usServiceId,
                                                usEventId,
                                                adm_soa_getUimInstanceId());
        log_w("adm_arpc_checkIfClientSubedEvent 0x%x, iRet = %d", usEventId,
              iRet);
        if (1 == iRet)
        {
            log_i("usEventId 0x%x Subed Success", usEventId);
            iRet = ADM_ECODE_SUCCESSED;
            break;
        }
        sleep(5);
        usTimeCount += 5; ///!< 每5s检查一次
    } while (usTimeCount < usTimeOut);

    return iRet;
}

//解析的零件响应的json报文
static D32S sysi4host_ParseEcuInfo(json_object *pstDidObj, ADM_SYSI4ALL_DID_DATA_ARRAY_T *pDidArrayRequest)
{
    json_object* pTmpObj = NULL;
    D32S iRet = ADM_ECODE_SUCCESSED;
    D16U i = 0;
    const DCHAR* pDidValue = NULL;
    D32U uiLength = 0;
    DCHAR aTemp[8] = {0};

    if(ADM_IS_NULL(pDidArrayRequest) || ADM_IS_NULL(pstDidObj))
    {
        log_e("input param is NULL");
        return ADM_ECODE_INVALID_PARAM;
    }

    for (; i < pDidArrayRequest->didArrayLength; i++)
    {
        if (NULL == pDidArrayRequest->didArray[i] || NULL == pDidArrayRequest->didArray[i]->variable)
        {
            log_e("resp ecu info is error.i=[%u]", i);
            iRet = ADM_ECODE_INVALID_PARAM;
            break;
        }

        sprintf(aTemp, "0x%X", pDidArrayRequest->didArray[i]->did);

        pTmpObj = json_object_object_get(pstDidObj, aTemp);
        if (NULL == pTmpObj)
        {
            continue;
        }

        pDidValue = json_object_get_string(pTmpObj);
        if (NULL != pDidValue)
        {
            uiLength = adm_strlen(pDidValue);
            (DVOID)adm_memcpy(pDidArrayRequest->didArray[i]->variable->v.value_s, pDidValue, 
                        uiLength, uiLength);
            pDidArrayRequest->didArray[i]->variable->v.value_s[uiLength] = '\0';
            pDidArrayRequest->didArray[i]->variable->data_length = uiLength;
            pDidArrayRequest->didArray[i]->variable->data_type = VAR_STRING;
        }
        else 
        {
            log_w("Key[%s] get empty value.", aTemp);
            pDidArrayRequest->didArray[i]->variable->data_length = 0;
        }
    }
    
    return iRet;
}

static D32S sysi4host_ParseSnInfo(DCHAR** ppSnInfo, const DCHAR* pStrEcuInfo)
{
    json_object* pInObj = NULL;
    json_object* child_obj = NULL;
    json_object* pValueObj = NULL;
    json_object* pResult = NULL;
    DBOOL bFind = DFALSE;
    D32S iRet = ADM_ECODE_SUCCESSED;


    if(ADM_IS_NULL(ppSnInfo) || ADM_IS_NULL(pStrEcuInfo))
    {
        log_e("input param is NULL");
        return ADM_ECODE_INVALID_PARAM;
    }
    
    pInObj = json_tokener_parse(pStrEcuInfo);
    if (NULL == pInObj)
    {
        log_e("collect ecu info error. EcuInfo[%s]", pStrEcuInfo);
        return ADM_ECODE_NEW_JSON_OBJECT_FAILED;
    }

    pResult = json_object_object_get(pInObj, "result");
    if (NULL == pResult)
    {
        log_e("collect result data error. EcuInfo[%s]", pStrEcuInfo);
        return ADM_ECODE_NEW_JSON_OBJECT_FAILED;
    }

    D32S iNum = json_object_array_length(pResult);
    for (D32S i = 0; i < iNum; i++)
    {
        child_obj = json_object_array_get_idx(pResult, i);

        /** 零件硬件序列号 */
        pValueObj = json_object_object_get(child_obj, "0xF18C");
        if (NULL == pValueObj)
        {
            continue;
        }

        *ppSnInfo = adm_strdup(json_object_get_string(pValueObj));
        bFind = DTRUE;
    }

    if (DFALSE == bFind)
    {
        log_e("0xF18C not exists.");
        iRet = ADM_ECODE_JSON_GET_STRING_FAILED;
    }
    

    (void)json_object_put(pInObj);

    return iRet;
}

//OTA模式控制
D32S sysi4host_OptOTAMode(D32S type)
{
    D32S iErrCode = ADM_ECODE_FAILED;
    DCHAR aDescription[128] = {0};

    CHECK_POINTER_RETURN_RET(stSysi4hostJniCb.controlOtaModeCb, ADM_ECODE_INVALID_POINTER);
    (DVOID)stSysi4hostJniCb.controlOtaModeCb(type, &iErrCode);

    adm_snprintf(aDescription, sizeof(aDescription), "type=[%d], type:1-进入 2-退出", type);
    if (ADM_IS_ERR(iErrCode))
    {
        adm_sysi_setKeylogString(ADM_KEYLOG_ID_020, iErrCode, aDescription);
    }
    else
    {
        adm_sysi_setKeylogString(ADM_KEYLOG_ID_019, iErrCode, aDescription);
    }

    log_i("type=%d, iErrCode=[%d]", type, iErrCode);
    return iErrCode;
}
//车辆高压控制
D32S sysi4host_OptHighVoltage(D32S type)
{
    D32S iErrCode = ADM_ECODE_FAILED;
    DCHAR aDescription[128] = {0};

    CHECK_POINTER_RETURN_RET(stSysi4hostJniCb.controlHighVoltageCb, ADM_ECODE_INVALID_POINTER);

    (DVOID)stSysi4hostJniCb.controlHighVoltageCb(type, &iErrCode);

    adm_snprintf(aDescription, sizeof(aDescription), "type=%d, type:1-上高压 2-下高压 0-高压未激活状态", type);
    if (ADM_IS_ERR(iErrCode))
    {
        adm_sysi_setKeylogString(ADM_KEYLOG_ID_022, iErrCode, aDescription);
    }
    else
    {
        adm_sysi_setKeylogString(ADM_KEYLOG_ID_021, iErrCode, aDescription);
    }

    log_i("type=%d, iErrCode=[%d]", type, iErrCode);
    return iErrCode;
}

/// @brief 宿主启动回调
/// @param reason 
void sysi4host_hostJustOnNotify(D32S reason)
{
    log_i("sysi4host_hostJustOnNotify start. reason %d!", reason);

    log_i("sysi4host_hostJustOnNotify end!");

    return;
}

/// @brief 宿主关闭回调
/// @param timeout_ms 
void sysi4host_hostWillOffNotify(D32S timeout_ms)
{
    log_i("sysi4host_hostWillOffNotify start. timeout_ms %d!", timeout_ms);

    log_i("sysi4host_hostWillOffNotify end!");

    return;
}

#define ADM_SMART_SCENARIO_SENTINEL_MODE_ID             "0x30C"
#define ADM_SMART_SCENARIO_CHILDPET_MODE_ID             "0x30D"
#define ADM_SMART_SCENARIO_TRAVEL_SCHEDULE_AVAIL_ID     "0x401"
#define ADM_SMART_SCENARIO_TRAVEL_SCHEDULE_START_ID     "0x402"
#define ADM_SMART_SCENARIO_TRAVEL_SCHEDULE_STOP_ID      "0x403"
#define ADM_SMART_SCENARIO_CHARGE_SCHEDULE_AVAIL_ID     "0x404"
#define ADM_SMART_SCENARIO_CHARGE_SCHEDULE_START_ID     "0x405"
#define ADM_SMART_SCENARIO_CHARGE_SCHEDULE_STOP_ID      "0x406"
#define ADM_SMART_SCENARIO_PRESENTATION_MODE_ID         "0x407"
#define ADM_SMART_SCENARIO_FACTORY_MODE_ID              "0x408"
#define ADM_SMART_SCENARIO_TRANSPORT_MODE_ID            "0x409"

/// @brief 智能场景变化回调
/// @param pMsg 
void sysi4host_smartScenarioChanged(DCHAR *pMsg)
{
    // TBD: 针对智能场景变化的处理逻辑
    if (NULL != pMsg)
    {
        log_i("Smart scenario changed.msg:%s", pMsg);
    }
    return;
}

/// @brief 获取智能场景
/// @param eVehicleState 
/// @param pValue 
/// @return 
D32S sysi4host_getSmartScenario(VEHICLE_STATUS_E eVehicleState, sysi4all_variable_pt pValue)
{
    D32S iErrCode = ADM_ECODE_FAILED;
    DCHAR *pResponse = NULL;
    DCHAR strTmp[8] = {0};
    json_object* pstJsonArrayStr = NULL;
    const DCHAR* pJsonArrayStr = NULL;
    //DCHAR aDescription[128] = {0};

    sprintf(strTmp, "0x%X", eVehicleState);

    pstJsonArrayStr = json_object_new_array();
    if(ADM_IS_NULL(pstJsonArrayStr))
    {
        log_e("json_object_new_object NULL");
        iErrCode = ADM_ECODE_NEW_JSON_OBJECT_FAILED;
        return iErrCode;
    }

    iErrCode = json_object_array_add(pstJsonArrayStr, json_object_new_string(strTmp));
    if(ADM_IS_ERR(iErrCode))
    {
        log_e("json_object_object_add %s error %d", strTmp, iErrCode);
        return iErrCode;
    }

    pJsonArrayStr = json_object_get_string(pstJsonArrayStr);

    log_i("getSmartScenario[%s]", pJsonArrayStr);

    CHECK_POINTER_RETURN_RET(stSysi4hostJniCb.getSmartScenarioCb, ADM_ECODE_INVALID_POINTER);
    iErrCode = stSysi4hostJniCb.getSmartScenarioCb((DCHAR*)pJsonArrayStr, &pResponse);
    if (ADM_IS_ERR(iErrCode) || ADM_IS_NULL(pResponse))
    {
        log_e("getSmartScenarioCb error!");
        //adm_snprintf(aDescription, sizeof(aDescription), "vehicle state=%s", strTmp);
        //adm_sysi_setKeylogString(ADM_KEYLOG_ID_038, iErrCode, aDescription);
        iErrCode = -1;
        return iErrCode;
    }

    json_object* resObj = NULL;
    resObj = json_tokener_parse(pResponse);
    if (ADM_IS_NULL(resObj) || json_object_get_type(resObj) != json_type_object) 
    {
        (void)json_object_put(resObj);
        log_i("this is an error object json!");
        return ADM_ECODE_STRING_TO_JSON_FAILED;
    }

    json_object* resultObj = NULL;
    resultObj = json_object_object_get(resObj, "result");
    if(NULL == resultObj){
        (void)json_object_put(resObj);
        log_i("result object not exist!");
        return ADM_ECODE_JSON_GET_OBJECT_FAILED;
    }

    D32S iNum = json_object_array_length(resultObj);
    for (D32S i = 0; i < iNum; i++)
    {
        json_object* child_obj = json_object_array_get_idx(resultObj, i);
        json_object* tmpObj = NULL;
        tmpObj = json_object_object_get(child_obj, strTmp);
        if(NULL == tmpObj){
            (void)json_object_put(resObj);
            log_i("%s object not exist!", strTmp);
            return ADM_ECODE_JSON_GET_OBJECT_FAILED;
        }

        switch(eVehicleState)
        {
            case VEHICLE_STATUS_SENTINEL_MODE:
            case VEHICLE_STATUS_CHILDPET_MODE:
            case VEHICLE_STATUS_TRAVEL_SCHEDULE_AVAIL:
            case VEHICLE_STATUS_CHARGE_SCHEDULE_AVAIL:
            case VEHICLE_STATUS_PRESENTATION_MODE:
            case VEHICLE_STATUS_FACTORY_MODE:
            case VEHICLE_STATUS_TRANSPORT_MODE:
            case VEHICLE_STATUS_NORMAL_MODE:
            {
                pValue->data_type = VAR_S32;
                pValue->data_length = sizeof(D32S);
                pValue->v.value_s32 = (D32S)json_object_get_int(tmpObj);
                //adm_snprintf(aDescription, sizeof(aDescription), "[%s]=[%d]", strTmp, pValue->v.value_s32);
                //adm_sysi_setKeylogString(ADM_KEYLOG_ID_039, iErrCode, aDescription);
                log_i("Get %s value=%d", strTmp, pValue->v.value_s32);

                break;
            }
            case VEHICLE_STATUS_TRAVEL_SCHEDULE_START:
            case VEHICLE_STATUS_TRAVEL_SCHEDULE_STOP:
            case VEHICLE_STATUS_CHARGE_SCHEDULE_START:
            case VEHICLE_STATUS_CHARGE_SCHEDULE_STOP:
            {
                pValue->data_type = VAR_STRING;

                const char* strTime = json_object_get_string(tmpObj);
                if(strTime != NULL)
                {
                    pValue->data_length = strlen(strTime);
                    sprintf(pValue->v.value_s, "%s", strTime);
                    //adm_snprintf(aDescription, sizeof(aDescription), "[%s]=[%s]", strTmp, pValue->v.value_s);
                    //adm_sysi_setKeylogString(ADM_KEYLOG_ID_039, iErrCode, aDescription);
                    log_i("Get %s value=%s", strTmp, pValue->v.value_s);
                }
                break;
            }
            default:
            {
                break;
            }
        }
    }

    (void)json_object_put(resObj);
    (void)json_object_put(pstJsonArrayStr);
    adm_free(pResponse);
    return iErrCode;
}

/// @brief 整车信号变化回调
/// @param pMsg 

DBOOL sysi4host_getSignal(json_object *pObj, ADM_SYSI4HOST_SIGNAL_EVENT_TYPE_E* pType)
{
    if (ADM_IS_NULL(pObj) || ADM_IS_NULL(pType))
    {
        log_e("Obj is null.");
        return DFALSE;
    }

    json_object *tmpObj = NULL;
    D32S iValue = -1;

    if (NULL != (tmpObj = json_object_object_get(pObj, ADM_POWER_MODE_ID)))
    {
        iValue = (D32S)json_object_get_int(tmpObj);
        log_i("Find Power mode changed. Handle signal[%s] value[%d].", ADM_POWER_MODE_ID, iValue);

        if (DFALSE == updateVehicleStatusValue(ADM_POWER_MODE_ID, iValue))
        {
            return DFALSE;
        }
        
        if (1 == iValue)
        {
            *pType = ADM_DIM_SIGNAL_EVENT_TYPE_POWER_ON;
        }
        else if (2 == iValue)
        {
            *pType = ADM_DIM_SIGNAL_EVENT_TYPE_POWER_OFF;
        }
        else
        {
            log_w("Handle power value abnormal.");
        }
    }
    else if (NULL != (tmpObj = json_object_object_get(pObj, ADM_VEH_GEAR_ID)))
    {
        iValue = (D32S)json_object_get_int(tmpObj);
        log_i("Find gear changed. Handle signal[%s] value[%d].", ADM_VEH_GEAR_ID, iValue);

        if (DFALSE == updateVehicleStatusValue(ADM_VEH_GEAR_ID, iValue))
        {
            return DFALSE;
        }

        if (0 == iValue)
        {
            *pType = ADM_DIM_SIGNAL_EVENT_GEAR_P; ///< P挡
        }
        else if (1 == iValue)
        {
            *pType = ADM_DIM_SIGNAL_EVENT_GEAR_NONE_P; ///< 非P档
        }
        else
        {
            log_w("Handle gear value abnormal.");
        }
    }
    else if (NULL != (tmpObj = json_object_object_get(pObj, ADM_VEH_LOCK_STATUS_ID)))
    {
        iValue = (D32S)json_object_get_int(tmpObj);
        log_i("Find lock status changed. Handle signal[%s] value[%d].", ADM_VEH_LOCK_STATUS_ID, iValue);

        if (DFALSE == updateVehicleStatusValue(ADM_VEH_LOCK_STATUS_ID, iValue))
        {
            return DFALSE;
        }

        if (1 == iValue)
        {
            *pType = ADM_DIM_SIGNAL_EVENT_ANTI_THEFT_MODE_OFF; ///< 解防（未锁车）
        }
        else if (2 == iValue)
        {
            *pType = ADM_DIM_SIGNAL_EVENT_ANTI_THEFT_MODE_ON; ///< 锁车
        }
        else
        {
            log_w("Handle lock status value abnormal.");
        }
    }
    else if (NULL != (tmpObj = json_object_object_get(pObj, ADM_VEH_LFHDOORSTS_MODE_ID)))
    {
        
        iValue = (D32S)json_object_get_int(tmpObj);
        log_i("Find door status changed. Handle signal[%s] value[%d].", ADM_VEH_LFHDOORSTS_MODE_ID, iValue);

        if (DFALSE == updateVehicleStatusValue(ADM_VEH_LFHDOORSTS_MODE_ID, iValue))
        {
            return DFALSE;
        }

        if (0 == iValue)
        {
            *pType = ADM_DIM_SIGNAL_EVENT_LFH_DOOR_OPEN;    ///< 主驾门开
        }
        else if (1 == iValue)
        {
            *pType = ADM_DIM_SIGNAL_EVENT_LFH_DOOR_CLOSE;   ///< 主驾门关
        }
        else
        {
            log_w("Handle door status value abnormal.");
        }
    }
    else
    {
        return DFALSE;
    }

    return DTRUE;
}

void sysi4host_vehicleSignalChanged(DCHAR *pMsg)
{
    json_object* resObj = NULL;
    json_object* resultObj = NULL;
    D32S iValue = -1;
    ADM_SYSI4HOST_SIGNAL_EVENT_TYPE_E eType = ADM_DIM_SIGNAL_EVENTL_TYPE_INVALID;
    SYSI4LOADER_FUNC_LIST_T *func = NULL;
    DBOOL bIsUpdate = DFALSE;

    if (NULL == pMsg)
    {
        log_i("Vehicle signal changed notify msg is null.");
        return;
    }

    do
    {
        func = adm_getSysi4LoaderFuncList();
        if (ADM_IS_NULL(func) || ADM_IS_NULL(func->adm_sysi4host_signalEventNotify_mem))
        {
            log_e("signalEventNotify cb is NULL");
            break;
        }

        log_i("Vehicle signal changed.msg:%s", pMsg);

        sysi4host_ensureCheckSubedEventSuccess(ADM_HOST_SERVICE_ID, ADM_HOST_SIGNALS_EVENT_NOTIFY_EVENT_ID, 180);

        resObj = json_tokener_parse(pMsg);
        if (ADM_IS_NULL(resObj) || json_object_get_type(resObj) != json_type_object) 
        {
            log_e("this is an error object json!");
            break;
        }
        
        resultObj = json_object_object_get(resObj, "result");
        if(NULL == resultObj)
        {
            log_e("result object not exist!");
            break;
        }

        D32S iNum = json_object_array_length(resultObj);
        for (D32S i = 0; i < iNum; i++)
        {
            json_object* child_obj = json_object_array_get_idx(resultObj, i);

            bIsUpdate = sysi4host_getSignal(child_obj, &eType);
            if ((eType != ADM_DIM_SIGNAL_EVENTL_TYPE_INVALID) && (DTRUE == bIsUpdate))
            {
                func->adm_sysi4host_signalEventNotify_mem(eType);
            }
        }
    } while (FALSE);

    if (NULL != resObj)
    {    
        (void)json_object_put(resObj);
    }
    return;
}

D32S adm_sysi4host_ipmGetInstallSchedule(DCHAR ** pMsg)
{
    log_i("adm_sysi4host_ipmGetInstallSchedule start.");
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
    (void)adm_arpc_request_service(UIM2TBOX_INSTANCE_ID, ADM_UIM_INSTALL_SERVICE_ID,
                             ADM_UIM_HMI_GET_INSTALL_SCHEDULE_METHOD_ID, 15, pReqStr, adm_strlen(pReqStr) + (D32U)1,
                             &resp, &respSize);
    log_i("install schedule resp:%s , size:%d", resp,respSize);
    if (DNULL != pObj)
    {
        json_object_put(pObj);
    }
    adm_free(pReqStr);
    if (respSize == (D32U)0)
    {
        log_e("respSize is %d",respSize);
        return ADM_ECODE_INVALID_PARAM;
    }
    struct json_object *root = NULL;
    struct json_object *pInstallSchedule = NULL;
    root = json_tokener_parse(resp);
    adm_free(resp);
    if (NULL == root)
    {
        log_e("json_tokener_parse failed");
        return ADM_ECODE_INVALID_POINTER;
    }

    pInstallSchedule = json_object_object_get(root, "installSchedule");
    if (DNULL != pInstallSchedule)
    {
        const DCHAR * pInstallSchedulePtr = json_object_get_string(pInstallSchedule);
        *pMsg = adm_strdup(pInstallSchedulePtr);
        log_i("get install schedule pInstallSchedulePtr: %s,pMsg : %s", pInstallSchedulePtr,*pMsg);
    }
    (void)json_object_put(root);
    return ADM_ECODE_SUCCESSED;
}

/** 车云和车机之间信号获取值拉齐 */
static  DVOID sysi4host_convertSignalValue(VEHICLE_STATUS_E eVehicleState, sysi4all_variable_pt pValue)
{
    if (NULL == pValue)
    {
        log_e("Invalid param.");
        return;
    }

    switch (eVehicleState)
    {
        case VEHICLE_STATUS_FORTIFY:
            if (1 == pValue->v.value_s32)
            {
                pValue->v.value_s32 = 0;    ///< 解防
            }
            else if (2 == pValue->v.value_s32)
            {
                pValue->v.value_s32 = 1;    ///< 设防
            }
        break;
        case VEHICLE_STATUS_GEAR:
            if (1 == pValue->v.value_s32)
            {
                pValue->v.value_s32 = 2;    ///< 非P档
            }
        break;
        case VEHICLE_STATUS_EPBLOCK:
        /** no break */
        case VEHICLE_STATUS_READY:
            if (1 == pValue->v.value_s32)
            {
                pValue->v.value_s32 = 0;
            }
            else if (0 == pValue->v.value_s32)
            {
                pValue->v.value_s32 = 1;
            }
        break;
        default:
            log_i("signal:%d do not need transfer.", eVehicleState);
            break;
    }

    return;
}

/// @brief 获取整车信号
/// @param eVehicleState 
/// @param pValue 
/// @return 
D32S sysi4host_getVehicleSignal(VEHICLE_STATUS_E eVehicleState, sysi4all_variable_pt pValue)
{
    D32S iErrCode = ADM_ECODE_FAILED;
    DCHAR *pResponse = NULL;
    DCHAR str_eVehicleState[8] = {0};
    json_object* pstJsonArrayStr = NULL;
    const DCHAR* pJsonArrayStr = NULL;
    DCHAR aDescription[128] = {0};

    /** 设防需要转换信号值  */
    if (VEHICLE_STATUS_ANTI_THEFT_STATUS == eVehicleState)
    {
        log_i("Convert %d to %d", VEHICLE_STATUS_ANTI_THEFT_STATUS, VEHICLE_STATUS_FORTIFY);
        eVehicleState = VEHICLE_STATUS_FORTIFY;
    }

    sprintf(str_eVehicleState, "0x%X", eVehicleState);
    
    pstJsonArrayStr = json_object_new_array();
    if(ADM_IS_NULL(pstJsonArrayStr))
    {
        log_e("json_object_new_object NULL");
        iErrCode = ADM_ECODE_NEW_JSON_OBJECT_FAILED;
        return iErrCode;
    }

    iErrCode = json_object_array_add(pstJsonArrayStr, json_object_new_string(str_eVehicleState));
    if(ADM_IS_ERR(iErrCode))
    {
        log_e("json_object_object_add %s error %d",str_eVehicleState,iErrCode);
        return iErrCode;
    }

    pJsonArrayStr = json_object_get_string(pstJsonArrayStr);

    log_i("Get VehicleSignal %s", pJsonArrayStr);

    

    CHECK_POINTER_RETURN_RET(stSysi4hostJniCb.getVehicleSignalCb, ADM_ECODE_INVALID_POINTER);
    iErrCode = stSysi4hostJniCb.getVehicleSignalCb((DCHAR*)pJsonArrayStr, &pResponse);
    if (ADM_IS_ERR(iErrCode) || ADM_IS_NULL(pResponse))
    {
        log_e("getVehicleSignalCb error!");
        adm_snprintf(aDescription, sizeof(aDescription), "vehicle state=%s", str_eVehicleState);
        adm_sysi_setKeylogString(ADM_KEYLOG_ID_036, iErrCode, aDescription);
        iErrCode = -1;
        return iErrCode;
    }

    json_object* resObj = NULL;
    resObj = json_tokener_parse(pResponse);
    if (ADM_IS_NULL(resObj) || json_object_get_type(resObj) != json_type_object) 
    {
        (void)json_object_put(resObj);
        log_i("this is an error object json!");
        return ADM_ECODE_STRING_TO_JSON_FAILED;
    }

    json_object* resultObj = NULL;
    resultObj = json_object_object_get(resObj, "result");
    if(NULL == resultObj){
        (void)json_object_put(resObj);
        log_i("result object not exist!");
        return ADM_ECODE_JSON_GET_OBJECT_FAILED;
    }

    D32S iNum = json_object_array_length(resultObj);
    for (D32S i = 0; i < iNum; i++)
    {
        json_object* child_obj = json_object_array_get_idx(resultObj, i);
    
        json_object* tmpObj = NULL;
        tmpObj = json_object_object_get(child_obj, str_eVehicleState);
        if(NULL == tmpObj)
        {
            (void)json_object_put(resObj);
            log_i("%s object not exist!", str_eVehicleState);
            return ADM_ECODE_JSON_GET_OBJECT_FAILED;
        }

        switch(eVehicleState)
        {
            case VEHICLE_STATUS_VSPD:
            case VEHICLE_STATUS_GEAR:
            case VEHICLE_STATUS_SOC:
            case VEHICLE_STATUS_POWER_SOC:
            case VEHICLE_STATUS_EPBLOCK:
            case VEHICLE_STATUS_CHARGING:
            case VEHICLE_STATUS_VOL_MV:
            case VEHICLE_STATUS_ENGINE_STATE:
            case VEHICLE_STATUS_OBD_STATE:
            case VEHICLE_STATUS_DISCHARGE:
            case VEHICLE_STATUS_READY:
            case VEHICLE_STATUS_POWER_BATTERY_SUP:
            case VEHICLE_STATUS_ECALL:
            case VEHICLE_STATUS_FORTIFY:
            case VEHICLE_STATUS_POWER_SIGNAL:
            case VEHICLE_STATUS_THEME:
            case VEHICLE_STATUS_ANTI_THEFT_STATUS:
            case VEHICLE_STATUS_POWER_TYPE:
            case VEHICLE_STATUS_SOC_OF_TRACTION_BAT:
            case VEHICLE_STATUS_SOC_OF_STORAGE_BAT:
            case VEHICLE_STATUS_COUNT_OF_SCREEN:
            case VEHICLE_STATUS_SCREEN_OF_OTA:
            case VEHICLE_STATUS_NETWORK_TYPE:
            case VEHICLE_STATUS_BKLIGHT_MODE:
            case VEHICLE_STATUS_ARCHITECTURE_MODE:
            case VEHICLE_OPEN_DOOR_STATUS:
            case VEHICLE_HIGH_VOLTAGE_STATUS:
            case VEHICLE_ENGINE_SPEED:
            case VEHICLE_PWR_OFF_SW_STS:
            {
                pValue->data_type = VAR_S32;
                pValue->data_length = sizeof(D32S);
                pValue->v.value_s32 = (D32S)json_object_get_int(tmpObj);
                adm_snprintf(aDescription, sizeof(aDescription), "[%s]=[%d]",str_eVehicleState, pValue->v.value_s32);
                adm_sysi_setKeylogString(ADM_KEYLOG_ID_037, iErrCode, aDescription);
                log_i("Get %s value=%d", str_eVehicleState, pValue->v.value_s32);
                break;
            }
            case VEHICLE_STATUS_VIN:
            case VEHICLE_STATUS_TIMEZONE:
            {
                pValue->data_type = VAR_STRING;

                const char* strTmp = json_object_get_string(tmpObj);
                if(strTmp != NULL)
                {
                    pValue->data_length = strlen(strTmp);
                    sprintf(pValue->v.value_s, "%s", strTmp);
                    adm_snprintf(aDescription, sizeof(aDescription), "[%s]=[%s]",str_eVehicleState, pValue->v.value_s);
                    adm_sysi_setKeylogString(ADM_KEYLOG_ID_037, iErrCode, aDescription);
                    log_i("Get %s value=%s", str_eVehicleState, pValue->v.value_s);
                }
                break;
            }
            default:
            {
                break;
            }
        }
    }

    /** 车云、车机之间信号值拉齐 */
    sysi4host_convertSignalValue(eVehicleState, pValue);

    (void)json_object_put(resObj);
    (void)json_object_put(pstJsonArrayStr);
    adm_free(pResponse);
    return iErrCode;
}

/// @brief 获取整车、显示大版本
/// @param eVehicleState 
/// @param pValue 
/// @return 
D32S sysi4host_getVehOSVersion(VEHICLE_STATUS_E eVehicleState, sysi4all_variable_pt pValue)
{
    D32S iErrCode = ADM_ECODE_FAILED;
    DCHAR *pResponse = NULL;
    const DCHAR* pVesionId = NULL;
    DCHAR strTmp[8] = {0};
    DCHAR aDescription[128] = {0};

    sprintf(strTmp, "0x%X", eVehicleState);
    log_i("getVehOSVersion[%s]", strTmp);

    CHECK_POINTER_RETURN_RET(stSysi4hostJniCb.getVehOSVersionCb, ADM_ECODE_INVALID_POINTER);
    iErrCode = stSysi4hostJniCb.getVehOSVersionCb(&pResponse);
    if (ADM_IS_ERR(iErrCode) || ADM_IS_NULL(pResponse))
    {
        log_e("getVehOSVersion error!");
        adm_snprintf(aDescription, sizeof(aDescription), "vehicle state=%s", strTmp);
        adm_sysi_setKeylogString(ADM_KEYLOG_ID_040, iErrCode, aDescription);
        iErrCode = -1;
        return iErrCode;
    }

    json_object* resObj = NULL;
    resObj = json_tokener_parse(pResponse);
    if (ADM_IS_NULL(resObj) || json_object_get_type(resObj) != json_type_object) 
    {
        (void)json_object_put(resObj);
        log_i("this is an error object json!");
        return ADM_ECODE_STRING_TO_JSON_FAILED;
    }

    json_object* resultObj = NULL;
    resultObj = json_object_object_get(resObj, "result");
    if(NULL == resultObj){
        (void)json_object_put(resObj);
        log_i("result object not exist!");
        return ADM_ECODE_JSON_GET_OBJECT_FAILED;
    }

    D32S iNum = json_object_array_length(resultObj);

    for (int i = 0; i < iNum; i++)
    {
        json_object* child_obj = json_object_array_get_idx(resultObj, i);
        json_object* child_obj_vehicle = json_object_object_get(child_obj, strTmp);
        if (NULL == child_obj_vehicle)
        {
            continue;
        }

        switch(eVehicleState)
        {
            case VEHICLE_STATUS_BIG_VERSION:
                /** no break */
            case VEHICLE_STATUS_DISPLAY_VERSION:
            {
                pValue->data_type = VAR_STRING;

                const char* strVersion = json_object_get_string(child_obj_vehicle);
                if(strVersion != NULL)
                {
                    pValue->data_length = strlen(strVersion);
                    sprintf(pValue->v.value_s, "%s", strVersion);
                    adm_snprintf(aDescription, sizeof(aDescription), "[%s]=[%s]", strTmp, pValue->v.value_s);
                    adm_sysi_setKeylogString(ADM_KEYLOG_ID_041, iErrCode, aDescription);
                    log_i("Get %s value=%s", strTmp, pValue->v.value_s);
                }
                break;
            }
            default:
            {
                break;
            }
        }
    }
    (void)json_object_put(resObj);
    adm_free(pResponse);
    return iErrCode;
}

/// @brief 设置整车、显示大版本
/// @param eVehicleState 
/// @param pVersion 
/// @return 
D32S sysi4host_setVehOSVersion(VEHICLE_STATUS_E eVehicleState, const DCHAR *pVersion)
{
    CHECK_POINTER_RETURN_RET(pVersion, ADM_ECODE_INVALID_PARAM);

    D32S iRet = ADM_ECODE_SUCCESSED;
    D32S iErrCode = ADM_ECODE_FAILED;
    json_object *jsOutTmp = NULL;
    json_object *jsVersion = NULL;
    json_object *jsOut = NULL;
    DCHAR strTmp[8];
    DCHAR aDescription[128] = {0};

    sprintf(strTmp, "0x%X", eVehicleState);

    jsOutTmp = json_object_new_array();
    if(ADM_IS_NULL(jsOutTmp))
    {
        log_e("json_object_new_array NULL");
        iRet = ADM_ECODE_NEW_JSON_OBJECT_FAILED;
        return iRet;
    }

    jsVersion = json_object_new_object();
    if(ADM_IS_NULL(jsVersion))
    {
        log_e("json_object_new_object NULL");
        iRet = ADM_ECODE_NEW_JSON_OBJECT_FAILED;
        return iRet;
    }

    iRet = json_object_object_add(jsVersion, strTmp, json_object_new_string(pVersion));
    if(ADM_IS_ERR(iRet))
    {
        log_e("json_object_object_add %s error %d",strTmp,iRet);
        return iRet;
    }

    iRet = json_object_array_add(jsOutTmp, jsVersion);
    if(ADM_IS_ERR(iRet))
    {
        log_e("json_object_array_add error %d",iRet);
        return iRet;
    }

    jsOut = json_object_new_object();
    iRet = json_object_object_add(jsOut, "result", jsOutTmp);
    if(ADM_IS_ERR(iRet))
    {
        log_e("json_object_object_add result error %s %d", strTmp, iRet);
        return iRet;
    }
    
    const char* versionListStr = json_object_to_json_string(jsOut);

    CHECK_POINTER_RETURN_RET(stSysi4hostJniCb.setVehOSVersionCb, ADM_ECODE_INVALID_POINTER);

    iRet = stSysi4hostJniCb.setVehOSVersionCb((DCHAR*)versionListStr, &iErrCode);
    log_i("write OSVersion=%s, iErrCode=%d", versionListStr, iErrCode);

    if (ADM_IS_ERR(iRet))
    {
        adm_snprintf(aDescription, sizeof(aDescription), "iErrCode=%d", iErrCode);
        adm_sysi_setKeylogString(ADM_KEYLOG_ID_042, iRet, aDescription);
    }
    else
    {
        adm_sysi_setKeylogString(ADM_KEYLOG_ID_043, iRet, "");
    }

    if(ADM_IS_ERR(iRet))
    {
        log_e("set VehOSVersion failed.");
    }

    json_object_put(jsOut);
    return iErrCode;
}

//车机霸屏模式控制
D32S sysi4host_OverscreenControl(D32S controlType)
{
    D32S iErrCode = ADM_ECODE_FAILED;
    CHECK_POINTER_RETURN_RET(stSysi4hostJniCb.screenOverControlCb, ADM_ECODE_INVALID_POINTER);
    (DVOID)stSysi4hostJniCb.screenOverControlCb(controlType, &iErrCode);
    log_i("controlType=%d, iErrCode[%d]", controlType, iErrCode);

    return iErrCode;
}

//通知车机断开wifi
static D32S lionOtaSetWifiDisLink()
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    D32S iErrCode = ADM_ECODE_FAILED;

    CHECK_POINTER_RETURN_RET(stSysi4hostJniCb.setWifiDisLinkCb, ADM_ECODE_INVALID_POINTER);
    iRet = stSysi4hostJniCb.setWifiDisLinkCb(&iErrCode);
    log_i("iErrCode[%d]", iErrCode);
    return iRet;
}


//通知车机当前OTA的升级模式
D32S sysi4host_setUpgradeMode(D32S iUpgradeMode)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    D32S iErrCode = ADM_ECODE_FAILED;
    DCHAR aDescription[128] = {0};

    CHECK_POINTER_RETURN_RET(stSysi4hostJniCb.setUpgradeModeCb, ADM_ECODE_INVALID_POINTER);

    iRet = stSysi4hostJniCb.setUpgradeModeCb(iUpgradeMode, &iErrCode);

    adm_snprintf(aDescription, sizeof(aDescription), "iUpgradeMode=%d,1:立即升级,2:预约升级,3: 静默升级,4: 产线升级,5: 救援升级,6: USB 升级,7: 正常模式 ", iUpgradeMode);
    if (ADM_IS_ERR(iErrCode))
    {
        adm_sysi_setKeylogString(ADM_KEYLOG_ID_024, iErrCode, aDescription);
    }
    else
    {
        adm_sysi_setKeylogString(ADM_KEYLOG_ID_023, iErrCode, aDescription);
    }

    log_e("iUpgradeMode=%d, iErrCode[%d]", iUpgradeMode, iErrCode);

    return iRet;
}


//获取车机零件信息
typedef struct
{
    DCHAR* pPartNum;
    DCHAR* pSoftwareVer;
    DCHAR* pHardwareVer;
    DCHAR* pSnNum;
    D32S iEcuType;
} ADM_SYSI4HOST_PARTINFO_T;

static D32S sysi4host_get_vehicleCtl(D32U msgId, sysi4all_variable_pt pValue)
{
    D32S iRet = ADM_ECODE_FAILED;
    VEHICLE_CTRL_E eVehicleCtl = (VEHICLE_CTRL_E)msgId;

    return iRet;

}
static D32S sysi4host_get_vehicleEvent(D32U msgId, sysi4all_variable_pt pValue)
{
    D32S iRet = ADM_ECODE_FAILED;

    return iRet;
}

static D32S sysi4host_get_vehicleState(D32U msgId, sysi4all_variable_pt pValue)
{
    D32S iRet = ADM_ECODE_FAILED;
    VEHICLE_STATUS_E eVehicleState = (VEHICLE_STATUS_E)msgId;

    switch(eVehicleState)
    {
        case VEHICLE_STATUS_VIN:
        case VEHICLE_STATUS_VSPD:
        case VEHICLE_STATUS_SOC:
        case VEHICLE_STATUS_POWER_SOC:
        case VEHICLE_STATUS_GEAR:
        case VEHICLE_STATUS_EPBLOCK:
        case VEHICLE_STATUS_CHARGING:
        case VEHICLE_STATUS_VOL_MV:
        case VEHICLE_STATUS_ENGINE_STATE:
        case VEHICLE_STATUS_OBD_STATE:
        case VEHICLE_STATUS_DISCHARGE:
        case VEHICLE_STATUS_READY:
        case VEHICLE_STATUS_POWER_BATTERY_SUP:
        case VEHICLE_STATUS_ECALL:
        case VEHICLE_STATUS_FORTIFY:
        case VEHICLE_STATUS_POWER_SIGNAL:
        case VEHICLE_STATUS_THEME:
        case VEHICLE_STATUS_ANTI_THEFT_STATUS:
        case VEHICLE_STATUS_POWER_TYPE:
        case VEHICLE_STATUS_SOC_OF_TRACTION_BAT:
        case VEHICLE_STATUS_SOC_OF_STORAGE_BAT:
        case VEHICLE_STATUS_COUNT_OF_SCREEN:
        case VEHICLE_STATUS_SCREEN_OF_OTA:
        case VEHICLE_STATUS_NETWORK_TYPE:
        case VEHICLE_STATUS_BKLIGHT_MODE:
        case VEHICLE_STATUS_TIMEZONE:
        case VEHICLE_STATUS_ARCHITECTURE_MODE:
        case VEHICLE_OPEN_DOOR_STATUS:
        case VEHICLE_HIGH_VOLTAGE_STATUS:
        case VEHICLE_ENGINE_SPEED:
        case VEHICLE_PWR_OFF_SW_STS:
        {
            iRet = sysi4host_getVehicleSignal(eVehicleState, pValue);
            break;
        }

        case VEHICLE_STATUS_SENTINEL_MODE:
        case VEHICLE_STATUS_CHILDPET_MODE:
        case VEHICLE_STATUS_TRAVEL_SCHEDULE_AVAIL:
        case VEHICLE_STATUS_CHARGE_SCHEDULE_AVAIL:
        case VEHICLE_STATUS_PRESENTATION_MODE:
        case VEHICLE_STATUS_FACTORY_MODE:
        case VEHICLE_STATUS_TRANSPORT_MODE:
        case VEHICLE_STATUS_TRAVEL_SCHEDULE_START:
        case VEHICLE_STATUS_TRAVEL_SCHEDULE_STOP:
        case VEHICLE_STATUS_CHARGE_SCHEDULE_START:
        case VEHICLE_STATUS_CHARGE_SCHEDULE_STOP:
        {
            iRet = sysi4host_getSmartScenario(eVehicleState, pValue);
            break;
        }

        case VEHICLE_STATUS_BIG_VERSION:
        case VEHICLE_STATUS_DISPLAY_VERSION:
        {
            iRet = sysi4host_getVehOSVersion(eVehicleState, pValue);
            break;
        }

        default:
        {
            break;
        }
    }

    return iRet;
}

static D32S sysi4host_set_vehicleCtl(D32U msgId)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    VEHICLE_CTRL_E eVehicleCtl = (VEHICLE_CTRL_E)msgId;

    switch(eVehicleCtl)
    {
        case VEHICLE_CTRL_ENTER_NORMAL_OTA_MODE:
        {
            iRet = sysi4host_OptOTAMode(1);
            break;
        }

        case VEHICLE_CTRL_EXIT_NORMAL_OTA_MODE:
        {
            iRet = sysi4host_OptOTAMode(2);
            break;
        }

        case VEHICLE_CTRL_ENTER_HV_MODE:
        {
            iRet = sysi4host_OptHighVoltage(1);
            break;
        }

        case VEHICLE_CTRL_EXIT_HV_MODE:
        {
            iRet = sysi4host_OptHighVoltage(2);
            break;
        }

        case VEHICLE_CTRL_NO_ACTIVE_HV_MODE:
        {
            iRet = sysi4host_OptHighVoltage(0);
            break;
        }

        // case VEHICLE_CTRL_ENTER_AWAKE:
        // {
        //     iRet = sysi4host_OptVehicleNetworkWakeUp(1);
        //     break;
        // }

        // case VEHICLE_CTRL_ENTER_SLEEP:
        // {
        //     iRet = sysi4host_OptVehicleNetworkWakeUp(0);
        //     break;
        // }

        case VEHICLE_CTRL_ENTER_OVERSCREEN:
        {
            iRet = sysi4host_OverscreenControl(1);
            break;
        }

        case VEHICLE_CTRL_EXIT_OVERSCREEN:
        {
            iRet = sysi4host_OverscreenControl(2);
            break;
        }

        case VEHICLE_CTRL_SET_WIFI_DISLINK:
        {
            iRet = lionOtaSetWifiDisLink();
            break;
        }

        default:
        {
            break;
        }
    }

    return iRet;
}
static D32S sysi4host_set_vehicleEvent(D32U msgId)
{
    D32S iRet = ADM_ECODE_FAILED;

    return iRet;
}
static D32S sysi4host_set_vehicleState(D32U msgId)
{
    D32S iRet = ADM_ECODE_FAILED;

    return iRet;
}

D32S adm_sysi4host_verifySignature(const DCHAR* signedData, const D32S signedDataLength, 
                        const DCHAR* orignalData, const D32S orignalDataLength, 
                        D32S* errorCode, void* priv)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    *errorCode = 0;
    log_i("adm_sysi4host_verifySignature success!");
    return iRet;
}

//适配层接口 arpc below
D32S adm_sysi4host_getVin(DCHAR** vin, D32S* errorCode, void* priv)
{
    //17个字符
    D32S iRet = ADM_ECODE_SUCCESSED;
    sysi4all_variable_pt pstMsgData = (sysi4all_variable_pt)adm_malloc(sizeof(sysi4all_variable_st) + 100);

    iRet = sysi4host_getVehicleSignal(VEHICLE_STATUS_VIN, pstMsgData);
    if(iRet)
    {
        log_e("get vin failed.iRet[%d]", iRet);
    }

    *errorCode = iRet;

    *vin = adm_strdup(pstMsgData->v.value_s);

    if (*vin != NULL)
    {
        log_i("vin=%s", *vin);
    }
    log_i("errorCode[%d]", *vin);

    adm_free(pstMsgData);
    return iRet;
}

/// @brief  获取显示大版本
/// @param pVehicleVer 
/// @param errorCode 
/// @param priv 
/// @return 
D32S adm_sysi4host_getVehicleVersion(DCHAR** pVehicleVer, D32S* errorCode, void* priv)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    sysi4all_variable_pt pstMsgData = (sysi4all_variable_pt)adm_malloc(sizeof(sysi4all_variable_st) + 100);

    iRet = sysi4host_getVehOSVersion(VEHICLE_STATUS_DISPLAY_VERSION, pstMsgData);
    if(iRet)
    {
        log_e("get VehicleVersion failed.iRet[%d]", iRet);
    }

    *errorCode = iRet;
    *pVehicleVer = adm_strdup(pstMsgData->v.value_s);

    log_i("vehicleVersion=%s, errorCode[%d]", *pVehicleVer, iRet);

    adm_free(pstMsgData);
    return iRet;
}

/// @brief 设置显示大版本
/// @param pVehicleVer 
/// @param errorCode 
/// @param priv 
/// @return 
D32S adm_sysi4host_setVehicleVersion(const DCHAR* pVehicleVer, D32S* errorCode, void* priv)
{
    CHECK_POINTER_RETURN_RET(pVehicleVer, ADM_ECODE_INVALID_PARAM);

    D32S iRet = ADM_ECODE_SUCCESSED;

    iRet = sysi4host_setVehOSVersion(VEHICLE_STATUS_DISPLAY_VERSION, pVehicleVer);
    log_i("write bigVer=%s, iErrCode=%d", pVehicleVer, iRet);
    if(ADM_IS_ERR(iRet))
    {
        log_e("set VehicleDisplayVersion failed.");
    }

    *errorCode = iRet;

    return iRet;
}


D32S adm_sysi4host_readVehicleStatue(D32U msgId, sysi4all_variable_pt value,
                                     void* priv, D32U priv_size)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    CHECK_POINTER_RETURN_RET(value, ADM_ECODE_INVALID_POINTER);

    if((VEHICLE_CTRL_BEGIN < (VEHICLE_CTRL_E)msgId) && (VEHICLE_CTRL_END > (VEHICLE_CTRL_E)msgId))
    {
        iRet = sysi4host_get_vehicleCtl(msgId, value);
    }
    else if((VEHICLE_EVENT_BEGIN < (VEHICLE_EVENT_E)msgId) && (VEHICLE_EVENT_END > (VEHICLE_EVENT_E)msgId))
    {
        iRet = sysi4host_get_vehicleEvent(msgId, value);
    }
    else if((VEHICLE_STATUS_BEGIN < (VEHICLE_STATUS_E)msgId) && (VEHICLE_STATUS_END > (VEHICLE_STATUS_E)msgId))
    {
        iRet = sysi4host_get_vehicleState(msgId, value);
    }

    return iRet;
}
//设置车辆状态
D32S adm_sysi4host_setVehicleStatue(D32U msgId, void* priv, D32U priv_size)
{
    D32S iRet = ADM_ECODE_SUCCESSED;

    if((VEHICLE_CTRL_BEGIN < (VEHICLE_CTRL_E)msgId) && (VEHICLE_CTRL_END > (VEHICLE_CTRL_E)msgId))
    {
        iRet = sysi4host_set_vehicleCtl(msgId);
    }
    else if((VEHICLE_EVENT_BEGIN < (VEHICLE_EVENT_E)msgId) && (VEHICLE_EVENT_END > (VEHICLE_EVENT_E)msgId))
    {
        iRet = sysi4host_set_vehicleEvent(msgId);
    }
    else if((VEHICLE_STATUS_BEGIN < (VEHICLE_STATUS_E)msgId) && (VEHICLE_STATUS_END > (VEHICLE_STATUS_E)msgId))
    {
        iRet = sysi4host_set_vehicleState(msgId);
    }

    return iRet;
}

//车机零件信息
D32S adm_sysi4host_readEcuInfo(D32S iInstallType,
                               ADM_SYSI4ALL_DID_DATA_ARRAY_T* didArrayRequest, D32S* errorCode, void* priv)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    DCHAR* pStrEcuInfo = NULL;
    D32S i = 0;
    D32S iErrCode = ADM_ECODE_FAILED;
    json_object *str_obj = NULL;
    json_object *child_obj = NULL;

    CHECK_POINTER_RETURN_RET(stSysi4hostJniCb.getHostVersionCb, ADM_ECODE_INVALID_POINTER);

    log_i("adm_sysi4host_readEcuInfo install type:[%d].", iInstallType);

    if(ADM_IS_NULL(didArrayRequest) || ADM_IS_NULL(errorCode))
    {
        log_e("input param is NULL");
        return ADM_ECODE_INVALID_PARAM;
    }

    DCHAR *pResponse = NULL;
    DCHAR strTmp[8] = {0};

    json_object* reqObj = json_object_new_array();
    if(ADM_IS_NULL(reqObj))
    {
        log_e("json_object_new_array NULL");
        iRet = ADM_ECODE_NEW_JSON_OBJECT_FAILED;
        return iRet;
    }

    for (; i < didArrayRequest->didArrayLength; i++)
    {
        sprintf(strTmp, "0x%X", didArrayRequest->didArray[i]->did);
        str_obj = json_object_new_string(strTmp);
        json_object_array_add(reqObj, str_obj);
    }

    const DCHAR* pReq = json_object_to_json_string(reqObj);

    iErrCode = stSysi4hostJniCb.getHostVersionCb(iInstallType, pReq, &pResponse);
    if (iErrCode)
    {
        log_e("getHostVersion error!");
        return iErrCode;
    }

    json_object* resObj = NULL;
    resObj = json_tokener_parse(pResponse);
    if (ADM_IS_NULL(resObj) || json_object_get_type(resObj) != json_type_object) 
    {
        (void)json_object_put(resObj);
        log_i("this is an error object json!");
        return ADM_ECODE_STRING_TO_JSON_FAILED;
    }

    json_object* resultObj = NULL;
    resultObj = json_object_object_get(resObj, "result");
    if(NULL == resultObj){
        (void)json_object_put(resObj);
        log_i("result object not exist!");
        return ADM_ECODE_JSON_GET_OBJECT_FAILED;
    }

    D32S iNum = json_object_array_length(resultObj);

    for (i = 0; i < iNum; i++)
    {
        child_obj = json_object_array_get_idx(resultObj, i);
        iRet = sysi4host_ParseEcuInfo(child_obj, didArrayRequest);
        if (ADM_IS_ERR(iRet))
        {
            break;
        }
    }

    *errorCode = iRet;
    
    (void)json_object_put(reqObj);
    (void)json_object_put(resObj);
    adm_free(pResponse);    
    return iRet;
}

//车机零件信息-sn号读取
D32S adm_sysi4host_readSnInfo(D32S* errorCode, DCHAR** ppSnInfo, void* priv)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    DCHAR* pStrEcuInfo = NULL;

    if(ADM_IS_NULL(ppSnInfo) || ADM_IS_NULL(errorCode))
    {
        log_e("input param is NULL");
        return ADM_ECODE_INVALID_PARAM;
    }

    CHECK_POINTER_RETURN_RET(stSysi4hostJniCb.getHostVersionCb, ADM_ECODE_INVALID_POINTER);

    do
    {
        json_object* reqObj = json_object_new_array();
        if(ADM_IS_NULL(reqObj))
        {
            log_e("json_object_new_array NULL");
            iRet = ADM_ECODE_NEW_JSON_OBJECT_FAILED;
            return iRet;
        }

        json_object_array_add(reqObj, json_object_new_string("0xF18C"));
        const DCHAR* pReq = json_object_to_json_string(reqObj);


        /** 后期根据根据项目要求,确定具体采集的是哪个枚举值的 */
        iRet = stSysi4hostJniCb.getHostVersionCb(ADM_CUSTOM_ECU_TYPE_SOCMCU, pReq, &pStrEcuInfo);
        log_i("string pStrEcuInfo=%s", pStrEcuInfo);
        if (ADM_IS_ERR(iRet))
        {
            log_e("call getHostVersionCb error. retValue[%d].", iRet);
            break;
        }

        //获取sn号
        iRet = sysi4host_ParseSnInfo(ppSnInfo, pStrEcuInfo);
        if (ADM_IS_ERR(iRet))
        {
            *ppSnInfo = NULL;
            log_e("call sysi4host_ParseSnInfo error. retValue[%d].", iRet);
            break;
        }
    } while (false);

    *errorCode = iRet;
    adm_free(pStrEcuInfo);


    return iRet;
}

//车机隐私路径
D32S adm_sysi4host_getPrivacyPath(DCHAR** privacyPath, D32S* errorCode, void* priv)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    DCHAR* pStrEcuInfo = NULL;

    if(ADM_IS_NULL(privacyPath) || ADM_IS_NULL(errorCode))
    {
        log_e("input param is NULL");
        return ADM_ECODE_INVALID_PARAM;
    }

    CHECK_POINTER_RETURN_RET(stSysi4hostJniCb.getPrivacyPathCb, ADM_ECODE_INVALID_POINTER);

    do
    {
       /** 后期根据根据项目要求,确定具体采集的是哪个枚举值的 */
        iRet = stSysi4hostJniCb.getPrivacyPathCb(&pStrEcuInfo);
        log_i("string pStrEcuInfo=%s", pStrEcuInfo);
        if (ADM_IS_ERR(iRet))
        {
            log_e("call getHostVersionCb error. retValue[%d].", iRet);
            break;
        }
        log_i("string *privacyPath:%s", *privacyPath);
		*privacyPath = adm_strdup(pStrEcuInfo);
   
    } while (false);

    *errorCode = iRet;
    adm_free(pStrEcuInfo);

    return iRet;
}

D32S adm_sysi4host_getNetworkType(D32S* networkType, D32S* errorCode, void* priv)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    sysi4all_variable_pt pstMsgData = NULL;

    do
    {
        if (ADM_IS_NULL(networkType) || ADM_IS_NULL(errorCode))
        {
            log_e("Invalid param.");
            iRet = ADM_ECODE_FAILED;
            break;
        }

        sysi4all_variable_pt pstMsgData = (sysi4all_variable_pt)adm_malloc(sizeof(sysi4all_variable_st) + 100);

        iRet = sysi4host_getVehicleSignal(VEHICLE_STATUS_NETWORK_TYPE, pstMsgData);
        if (ADM_IS_ERR(iRet))
        {
            log_e("get network failed.iRet[%d]", iRet);
            break;
        }

        *networkType = pstMsgData->v.value_s32;

        log_i("networkType=%d", *networkType);
    } while (FALSE);

    *errorCode = iRet;
    adm_free(pstMsgData);

    return iRet;
}

static D64U getFreeSpace(const DCHAR* path)
{
    D64U totalBlocks = 0LLU, freeDisk = 0LLU;

#ifdef __QNX__
    struct statvfs diskInfo;
    (void)statvfs(path, &diskInfo);
    totalBlocks = (D64U)diskInfo.f_bsize;
    freeDisk = (D64U)diskInfo.f_bfree * totalBlocks;
    log_e("freeDisk is %lld KB freeDisk is %lld MB freeDisk is %lld GB",
          freeDisk >> 10, freeDisk >> 20, freeDisk >> 30);
#else
    struct statfs diskInfo;
    (void)statfs(path, &diskInfo);
    totalBlocks = (D64U)diskInfo.f_bsize;
    freeDisk = (D64U)diskInfo.f_bfree * totalBlocks;
    log_e("freeDisk is %lld KB freeDisk is %lld MB freeDisk is %lld GB", freeDisk >> 10, freeDisk >> 20, freeDisk >> 30);
#endif

    return freeDisk;
}

D32S adm_sysi4host_verifySpaceIsEnough(const D64S needSize, D64S* realSize, 
                                           D32S* errorCode, void* priv)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    *errorCode = ADM_ECODE_SUCCESSED;

    const DCHAR* path = adm_cfg_iniParserGetString(ADM_CFG_SYSI_PKG_STORAGE_DIR, ADM_CFG_SYSI_PKG_STORAGE_DIR_DEFAULT_VAL);
    log_e("+++path=%s", path);
    *realSize = (D64S)getFreeSpace(path);

    log_i("needSize:%lld, realSize:%lld", needSize, *realSize);
    if(needSize > *realSize)
    {
        *errorCode = ADM_ECODE_FAILED;
    }
    else
    {
        *errorCode = ADM_ECODE_SUCCESSED;
    }

    return iRet;
}

/// @brief 获取整车大版本
/// @param vehicleBigVersion 
/// @param errorCode 
/// @param priv 
/// @return 
D32S adm_sysi4host_getVehicleBigVersion(DCHAR** vehicleBigVersion, D32S* errorCode, void* priv)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    sysi4all_variable_pt pstMsgData = (sysi4all_variable_pt)adm_malloc(sizeof(sysi4all_variable_st) + 100);

    iRet = sysi4host_getVehOSVersion(VEHICLE_STATUS_BIG_VERSION, pstMsgData);
    if(iRet)
    {
        log_e("get VehicleBigVersion failed.iRet[%d]", iRet);
    }

    *errorCode = iRet;
    *vehicleBigVersion = adm_strdup(pstMsgData->v.value_s);

    log_i("vehicleBigVersion=%s, errorCode[%d]", *vehicleBigVersion, iRet);

    adm_free(pstMsgData);
    return iRet;
}

/// @brief 设置整车大版本
/// @param vehicleBigVersion 
/// @param errorCode 
/// @param priv 
/// @return 
D32S adm_sysi4host_setVehicleBigVersion(const DCHAR* vehicleBigVersion, D32S* errorCode, void* priv)
{
    CHECK_POINTER_RETURN_RET(vehicleBigVersion, ADM_ECODE_INVALID_PARAM);

    D32S iRet = ADM_ECODE_SUCCESSED;

    iRet = sysi4host_setVehOSVersion(VEHICLE_STATUS_BIG_VERSION, vehicleBigVersion);
    log_i("write bigVer=%s, iErrCode=%d", vehicleBigVersion, iRet);
    if(ADM_IS_ERR(iRet))
    {
        log_e("set VehicleBigVersion failed.");
    }

    *errorCode = iRet;

    return iRet;
}


//pJsonStr表示key:valuejson字符串
D32S adm_sysi4host_setWifiLink(DCHAR* pJsonStr, D32S* iErrCode)
{
    log_i("pJsonStr=%s", pJsonStr);
    D32S iRet = ADM_ECODE_SUCCESSED;

    CHECK_POINTER_RETURN_RET(stSysi4hostJniCb.setWifiLinkCb, ADM_ECODE_INVALID_POINTER);
    iRet = stSysi4hostJniCb.setWifiLinkCb(pJsonStr, iErrCode);
    return iRet;
}

D32S adm_sysi4host_downExceptionNotify(D32S result, D32S* errorCode, void* priv)
{
    D32S iRet = ADM_ECODE_SUCCESSED;

    CHECK_POINTER_RETURN_RET(stSysi4hostJniCb.downExceptionNotifyCb, ADM_ECODE_INVALID_POINTER);
    iRet = stSysi4hostJniCb.downExceptionNotifyCb(result, errorCode);
    log_i("downExceptionNotifyCb,ret:%d,",iRet);

    return iRet;
}

D32S adm_sysi4host_setIcuPowerOff(D32S* pErrCode)
{
    D32S iRet = ADM_ECODE_SUCCESSED;

    CHECK_POINTER_RETURN_RET(stSysi4hostJniCb.setIcuPowerOffCallback, ADM_ECODE_INVALID_POINTER);
    iRet = stSysi4hostJniCb.setIcuPowerOffCallback(pErrCode);

    return iRet;
}

D32S adm_sysi4host_notifyOtaHmiStart(D32S* pErrCode)
{
    D32S iRet = ADM_ECODE_SUCCESSED;

    CHECK_POINTER_RETURN_RET(stSysi4hostJniCb.notifyOtaHmiStartCb, ADM_ECODE_INVALID_POINTER);
    iRet = stSysi4hostJniCb.notifyOtaHmiStartCb(pErrCode);

    return iRet;
}

/**
 * @brief 获取OTA模式状态
 * @note
 * @param out D32S* piOtaStatus  OTA模式状态 0:OTA模式未启动 1:OTA模式启动中
 * @return D32S 0:成功 非0:失败
 */
D32S adm_sysi4host_getOtaModeStatus(D32S *piOtaStatus)
{
    D32S iRet = ADM_ECODE_FAILED;
    DCHAR aDescription[128] = {0};

    log_d("adm_sysi4host_getOtaModeStatus Start");

    CHECK_POINTER_RETURN_RET(stSysi4hostJniCb.getOtaModeStatusCb, ADM_ECODE_INVALID_POINTER);

    iRet = stSysi4hostJniCb.getOtaModeStatusCb(piOtaStatus);

    adm_snprintf(aDescription, sizeof(aDescription), "OtaStatus=[%d], 0:OTA模式未启动 1:OTA模式启动中", *piOtaStatus);
    if (ADM_IS_ERR(iRet))
    {
        adm_sysi_setKeylogString(ADM_KEYLOG_ID_026, iRet, aDescription);
    }
    else
    {
        adm_sysi_setKeylogString(ADM_KEYLOG_ID_025, iRet, aDescription);
    }

    log_d("adm_sysi4host_getOtaModeStatus End");

    return iRet;
}

static D32S adm_sysi4host_getTboxRemoteDiaStatus(D32S *piTboxRemoteDiaStatus)
{
    D32S iTboxRemoteDiaStatus = -1;
    if (ADM_IS_NULL(piTboxRemoteDiaStatus))
    {
        log_e("Invalid param.");
        return ADM_ECODE_INVALID_PARAM;
    }

    log_d("adm_sysi4host_getTboxRemoteDiaStatus Start");
    CHECK_POINTER_RETURN_RET(stSysi4hostJniCb.getRemoteDiaStatusCb, ADM_ECODE_INVALID_POINTER);

    iTboxRemoteDiaStatus = stSysi4hostJniCb.getRemoteDiaStatusCb();
    *piTboxRemoteDiaStatus = iTboxRemoteDiaStatus;

    log_d("adm_sysi4host_getTboxRemoteDiaStatus End, status=%d.", iTboxRemoteDiaStatus);
    return ADM_ECODE_SUCCESSED;
}

//网络唤醒控制
D32S sysi4host_controlWakeUpSignMode(D32S iControlMode, D32S iTimeOut)
{
    D32S iErrCode = ADM_ECODE_FAILED;

    CHECK_POINTER_RETURN_RET(stSysi4hostJniCb.controlWakeUpSignModeCb, ADM_ECODE_INVALID_POINTER);
    (DVOID)stSysi4hostJniCb.controlWakeUpSignModeCb(iControlMode, iTimeOut, &iErrCode);

    log_i("iControlMode=%d, iTimeOut=%d, iErrCode=[%d]", iControlMode,iTimeOut, iErrCode);

    return iErrCode;
}


//适配层接口 arpc above
D32S adm_sysi4hostApp_handlerInit(void)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    ADM_SYSI4HOST_INIT_INFO_T stHostfn = {0};

    const DCHAR* sysi_lvm_modules = adm_cfg_iniParserGetString("SYSI:sysi_lvm_modules", "");
    if(strstr(sysi_lvm_modules, "host") != NULL)
    {
        log_d("adm_sysi4host_loadFunctions load lvm func");
        stHostfn.getVinCallback = adm_sysi4host_getVin_lvm;
        // stHostfn.execOtaTaskCallback = adm_sysi4host_execOtaTask_lvm;
        // stHostfn.setSchduleTimeCallback = adm_sysi4host_setSchduleWakeuptime_lvm;
        stHostfn.setVehicleBigVersionCallback = adm_sysi4host_setVehicleBigVersion_lvm;
        stHostfn.getVehicleBigVersionCallback = adm_sysi4host_getVehicleBigVersion_lvm;
        stHostfn.verifySpaceIsEnoughCallback = adm_sysi4host_verifySpaceIsEnough_lvm;
        stHostfn.verifySignatureCallback = adm_sysi4host_verifySignature_lvm;
        stHostfn.setDtcCallback = adm_sysi4host_setDtc_lvm;
        stHostfn.clearDtcCallback = adm_sysi4host_clearDtc_lvm;
        stHostfn.getCertAbPathCallback = adm_sysi4host_getCertAbsolutePath_lvm;
        stHostfn.readVehicleStatue = adm_sysi4host_readVehicleStatus_lvm;
        stHostfn.setVehicleStatue = adm_sysi4host_setVehicleStatus_lvm;
        stHostfn.setLogLevel = adm_sysi4host_setLogLevel_lvm;
        stHostfn.getsdkVersion = adm_sysi4host_getsdkVersion_lvm;
        stHostfn.getNetworkTypeCallback = adm_sysi4host_getNetworkType_lvm;
        stHostfn.setWifiLink = adm_sysi4host_setWifiLink_lvm;
        stHostfn.sendAuthenticateEventCallback = adm_sysi4host_AuthenticateEvent_lvm;
        stHostfn.notifyOtaHmiStartCallback = adm_sysi4host_notifyOtaHmiStart_lvm;

        (DVOID)adm_sysi4host_handlerInit_lvm();
    }
    else 
    {
        stHostfn.getVinCallback = adm_sysi4host_getVin;
        // stHostfn.execOtaTaskCallback = adm_sysi4host_execOtaTask;
        // stHostfn.setSchduleTimeCallback = adm_sysi4host_setSchduleWakeuptime;
        stHostfn.setVehicleBigVersionCallback = adm_sysi4host_setVehicleBigVersion;
        stHostfn.getVehicleBigVersionCallback = adm_sysi4host_getVehicleBigVersion;
        stHostfn.verifySpaceIsEnoughCallback = adm_sysi4host_verifySpaceIsEnough;
        stHostfn.readVehicleStatue = adm_sysi4host_readVehicleStatue;
        stHostfn.setVehicleStatue = adm_sysi4host_setVehicleStatue;
        stHostfn.getNetworkTypeCallback = adm_sysi4host_getNetworkType;
        stHostfn.getPrivacyPathCallback = adm_sysi4host_getPrivacyPath;
        //stHostfn.readEcuInfo = adm_sysi4host_readEcuInfo;
        stHostfn.setWifiLink = adm_sysi4host_setWifiLink;
        stHostfn.sendAuthenticateEventCallback = sysi4host_powerStateEvent;
        stHostfn.getVehicleVersion = adm_sysi4host_getVehicleVersion;
        stHostfn.setVehicleVersion = adm_sysi4host_setVehicleVersion;
        stHostfn.verifySignatureCallback = adm_sysi4host_verifySignature;
        stHostfn.setUpgradeModeCallback = sysi4host_setUpgradeMode;
        stHostfn.downExceptionNotifyCallback = adm_sysi4host_downExceptionNotify;
        stHostfn.getOtaStatusCallback = adm_sysi4host_getOtaModeStatus;
        stHostfn.setIcuPowerOffCallback = adm_sysi4host_setIcuPowerOff;
        stHostfn.notifyOtaHmiStartCallback = adm_sysi4host_notifyOtaHmiStart;
        stHostfn.controlWakeUpSignModeCallback = sysi4host_controlWakeUpSignMode;
        stHostfn.getTboxRemoteDiaStatusCallback = adm_sysi4host_getTboxRemoteDiaStatus;
    }

    iRet = adm_sysi4host_init_handle(&stHostfn, DNULL);
    if (ADM_IS_ERR(iRet))
    {
        log_e("adm_sysi4host_init_handle return failed");
        return iRet;
    }

    return iRet;
}

void adm_sysi4hostApp_handlerDeinit(void)
{
    return;
}

D32S adm_sysi4host_jni_Init(ADM_SYSI4HOST_JNI_CALLBACK_T* pstJniCb)
{
    D32S iRet = ADM_ECODE_SUCCESSED;

    CHECK_POINTER_RETURN_RET(pstJniCb, ADM_ECODE_INVALID_PARAM);

    stSysi4hostJniCb.sendAuthenticateEventCb   = pstJniCb->sendAuthenticateEventCb;
    stSysi4hostJniCb.sendAuthenticateMsgCb     = pstJniCb->sendAuthenticateMsgCb;
    stSysi4hostJniCb.controlOtaModeCb          = pstJniCb->controlOtaModeCb;
    stSysi4hostJniCb.controlHighVoltageCb      = pstJniCb->controlHighVoltageCb;
    stSysi4hostJniCb.screenOverControlCb       = pstJniCb->screenOverControlCb;
    stSysi4hostJniCb.setWifiLinkCb             = pstJniCb->setWifiLinkCb;
    stSysi4hostJniCb.setWifiDisLinkCb          = pstJniCb->setWifiDisLinkCb;
    stSysi4hostJniCb.getSkScInfoCb             = pstJniCb->getSkScInfoCb;
    stSysi4hostJniCb.setUpgradeModeCb          = pstJniCb->setUpgradeModeCb;
    stSysi4hostJniCb.getHostVersionCb          = pstJniCb->getHostVersionCb;
    stSysi4hostJniCb.getPrivacyPathCb          = pstJniCb->getPrivacyPathCb;
    stSysi4hostJniCb.getSmartScenarioCb        = pstJniCb->getSmartScenarioCb;
    stSysi4hostJniCb.getVehicleSignalCb        = pstJniCb->getVehicleSignalCb;
    stSysi4hostJniCb.getVehOSVersionCb         = pstJniCb->getVehOSVersionCb;
    stSysi4hostJniCb.setVehOSVersionCb         = pstJniCb->setVehOSVersionCb;
    stSysi4hostJniCb.downExceptionNotifyCb     = pstJniCb->downExceptionNotifyCb;
    stSysi4hostJniCb.getOtaModeStatusCb        = pstJniCb->getOtaModeStatusCb;
    stSysi4hostJniCb.setIcuPowerOffCallback    = pstJniCb->setIcuPowerOffCallback;
    stSysi4hostJniCb.notifyOtaHmiStartCb       = pstJniCb->notifyOtaHmiStartCb;
    stSysi4hostJniCb.controlWakeUpSignModeCb   = pstJniCb->controlWakeUpSignModeCb;
    stSysi4hostJniCb.getRemoteDiaStatusCb      = pstJniCb->getRemoteDiaStatusCb;

    iRet = adm_sysi4hostApp_handlerInit();

    if(ADM_IS_ERR(iRet))
    {
        log_e("adm_sysi4hostApp_handlerInit return failed");
        return iRet;
    }

    return iRet;
}




