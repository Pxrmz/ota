/**
 * @file adm_vrs_registerInfo.c
 * @brief 注册信息处理，包括：获取，写入等
 * @details 注册信息处理，包括：解析平台反馈的注册信息，存储注册信息
 * @author Mengwei
 * @version v0.1.0
 * @date 2020-12-16
 * @copyright www.abupdate.com
 */
#include "adm_vrs_log.h"
#include <unistd.h>
#include <string.h>
#include <pthread.h>

#include "adm_vrs_registerInfo.h"
#include "adm_vrs_registerInfoIn.h"
#include "adm_vrs_registerManager.h"
#include "adm_com_kvstorage.h"

static ADM_VRS_VEHICLE_POWER_TYPE_E g_ePowerType = VEHICLE_INVALID;
static ADM_VRM_GET_VEHICLE_POWER_TYPE_CALLBACK_F g_getVehiclePowerTypeCb = NULL;
static DBOOL g_bPowerTypeConfirmed = FALSE;

struct kvLock
{
    pthread_mutex_t lock;
    D32S initState;
};
static struct kvLock _lock = {.initState = 0};

static void lockInitOnce(void)
{
    if (_lock.initState == 1)
    {
        return;
    }
    (void)pthread_mutex_init(&_lock.lock, NULL);
    _lock.initState = 1;
}

static void lockKv(void)
{
    lockInitOnce();
    (void)pthread_mutex_lock(&_lock.lock);
}

static void unlockKv(void)
{
    lockInitOnce();
    (void)pthread_mutex_unlock(&_lock.lock);
}


static D32S adm_vrs_loadRegisterInfoFromKeyValue(ADM_VRS_REGISTER_INFO_T* pstRegisterInfo)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    KvStorage_t* pstKv = NULL;
    size_t valLength = 0;
    ADM_KV_STORAGE_VALUE_TYPE_E eValueType = ADM_KV_STORAGE_VALUE_TYPE_INVALID;

    if (NULL != pstRegisterInfo) {
        pstKv = adm_core_service_getBusinessDatabaseDb();
        if (NULL == pstKv) {
            log_e("adm_core_service_getBusinessDatabaseDb failed");
            iRet = ADM_ECODE_KV_OPEN_FAILED;
            // return ADM_ECODE_KV_OPEN_FAILED;
        } else {
            if ((DBOOL)0 == adm_com_kvstorage_get_string(pstKv, "vin", &pstRegisterInfo->vinCode, &valLength, &eValueType)) {
                iRet = ADM_ECODE_KV_GET_STRING_FAILED;
                log_w( "adm_com_kvstorage_get_string get failed");
            } 
            // else {
            //     log_i( "adm_com_kvstorage_get_string vin:%s, valLength:%ld, eValueType:%d", pstRegisterInfo->vinCode, valLength, eValueType);
            // }
            
            if ((DBOOL)0 == adm_com_kvstorage_get_string(pstKv, "deviceId", &pstRegisterInfo->deviceId, &valLength, &eValueType)) {
                iRet = ADM_ECODE_KV_GET_STRING_FAILED;
                log_w( "adm_com_kvstorage_get_string get failed");
            } 
            // else {
            //     log_i( "adm_com_kvstorage_get_string deviceId:%s, valLength:%ld, eValueType:%d", pstRegisterInfo->deviceId, valLength, eValueType);
            // }

            if ((DBOOL)0 == adm_com_kvstorage_get_string(pstKv, "secret", &pstRegisterInfo->secret, &valLength, &eValueType)) {
                iRet = ADM_ECODE_KV_GET_STRING_FAILED;
                log_w( "adm_com_kvstorage_get_string get failed");
            } 
            // else {
            //     log_i( "adm_com_kvstorage_get_string secret:%s, valLength:%ld, eValueType:%d",pstRegisterInfo->secret, valLength, eValueType);
            // }

            if ((DBOOL)0 == adm_com_kvstorage_get_string(pstKv, "vehlType", &pstRegisterInfo->pcVehlType, &valLength, &eValueType))
            {
                iRet = ADM_ECODE_KV_GET_STRING_FAILED;
                log_w("adm_com_kvstorage_get_string get failed");
            }
            // else
            // {
            //     log_i("adm_com_kvstorage_get_string vehlType:%s, valLength:%ld, eValueType:%d", pstRegisterInfo->pcVehlType, valLength, eValueType);
            // }
        }
    } else {
        iRet = ADM_ECODE_INVALID_POINTER;
        log_e( "Parameter ppstRegisterInfo verification failed");
    }

    return iRet;

}

static D32S adm_vrs_removeRegisterInfoFromKeyValue()
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    KvStorage_t* pstKv = NULL;

    pstKv = adm_core_service_getBusinessDatabaseDb();
    if (NULL == pstKv)
    {
        log_e("adm_core_service_getBusinessDatabaseDb failed");
        iRet = ADM_ECODE_KV_OPEN_FAILED;
    }
     
    else 
    {
        if (!adm_com_kvstorage_remove_key(pstKv, "vin")) 
        {
            iRet = ADM_ECODE_KV_REMOVE_KEY_FAILED;
            log_w( "adm_com_kvstorage_get_string get failed");
        } 
        else 
        {
            log_i( "adm_com_kvstorage_remove_key vin ok");
        }
        
        if (!adm_com_kvstorage_remove_key(pstKv, "deviceId")) 
        {
            iRet = ADM_ECODE_KV_REMOVE_KEY_FAILED;
            log_w( "adm_com_kvstorage_get_string get failed");
        } 
        else 
        {
            log_i( "adm_com_kvstorage_remove_key deviceId ok");
        }

        if (!adm_com_kvstorage_remove_key(pstKv, "secret")) 
        {
            iRet = ADM_ECODE_KV_REMOVE_KEY_FAILED;
            log_w( "adm_com_kvstorage_remove_key remove failed");
        } 
        else 
        {
            log_i( "adm_com_kvstorage_remove_key secret ok");
        }

        if (!adm_com_kvstorage_remove_key(pstKv, "vehlType")) 
        {
            iRet = ADM_ECODE_KV_REMOVE_KEY_FAILED;
            log_w( "adm_com_kvstorage_remove_key remove failed");
        } 
        else 
        {
            log_i( "adm_com_kvstorage_remove_key vehlType ok");
        }
    }

    return iRet;

}




void adm_vrs_setTimestampInfo(ADM_OCS_REGISTER_RESPONSE_INFO_T* pstRegisterResponseInfo)
{
    KvStorage_t* pstKv = NULL;
    DCHAR stimestamp[11] = {0};

    if (NULL == pstRegisterResponseInfo) 
    {
        log_e( "Param verification failed, pointer invalid");
        return;
    }

    //!< 将startTime 存储到kv中
    (void)adm_memset(stimestamp, 0x00, sizeof(stimestamp), sizeof(stimestamp));
    (void)adm_snprintf(stimestamp, sizeof(stimestamp), "%lld", pstRegisterResponseInfo->itimestamp);
    log_i( "kvstorate_set_string timestamp:%s", stimestamp);

    pstKv = adm_core_service_getBusinessDatabaseDb();
    if (NULL == pstKv) 
    {
        log_e("adm_core_service_getBusinessDatabaseDb failed");
        return;
    }

    if (FALSE == adm_com_kvstorage_set_string(pstKv, "timestamp", stimestamp, ADM_KV_STORAGE_VALUE_TYPE_INT64)) 
    {
        log_e( "adm_com_kvstorage_set_string timestamp failed");
    } 
    else 
    {
        log_i( "kvstorate_set_string timestamp:%lld successed", pstRegisterResponseInfo->itimestamp);
    }

    return ;
}

static D32S adm_vrs_saveRegisterInfoToKeyValue(const ADM_VRS_REGISTER_INFO_T* pRegInfo)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    KvStorage_t* pstKv = NULL;

    if (NULL == pRegInfo) {
        log_e( "Param (pRegInfo) verification failed, pointer invalid");
        return ADM_ECODE_INVALID_PARAM;
    }

    pstKv = adm_core_service_getBusinessDatabaseDb();
    if (NULL == pstKv) {
        log_e("adm_core_service_getBusinessDatabaseDb failed");
        return ADM_ECODE_KV_OPEN_FAILED;
    }

    if (NULL != pRegInfo->vinCode) {
        if (!adm_com_kvstorage_set_string(pstKv, "vin", pRegInfo->vinCode, ADM_KV_STORAGE_VALUE_TYPE_STRING)) {
            iRet = ADM_ECODE_KV_SET_STRING_FAILED;
            log_w( "adm_com_kvstorage_set_string string:%s failed, reason of error code is %d", pRegInfo->vinCode, iRet);
        } else {
            log_i( "kvstorate_set_string vin:%s successed", pRegInfo->vinCode);
        }
    } else {
        log_w( "Parameter (vinCode) verification failed, pointer invalid");
        iRet = ADM_ECODE_INVALID_POINTER;
    }

    if (NULL != pRegInfo->deviceId) {
        if (!adm_com_kvstorage_set_string(pstKv, "deviceId", pRegInfo->deviceId, ADM_KV_STORAGE_VALUE_TYPE_STRING)) {
            iRet = ADM_ECODE_KV_SET_STRING_FAILED;
            log_w( "adm_com_kvstorage_set_string string:%s failed, reason of error code is %d", pRegInfo->deviceId, iRet);
        } else {
            log_i( "kvstorate_set_string deviceId:%s successed", pRegInfo->deviceId);
        }
    } else {
        log_w( "Parameter (deviceId) verification failed, pointer invalid");
        iRet = ADM_ECODE_INVALID_POINTER;
    }

    if (NULL != pRegInfo->secret) {
        if (!adm_com_kvstorage_set_string(pstKv, "secret", pRegInfo->secret, ADM_KV_STORAGE_VALUE_TYPE_STRING)) {
            iRet = ADM_ECODE_KV_SET_STRING_FAILED;
            log_w( "adm_com_kvstorage_set_string string:%s failed, reason of error code is %d", pRegInfo->secret, iRet);
        } else {
            log_i( "kvstorate_set_string secret:%s successed", pRegInfo->secret);
        }
    } else {
        log_w( "Parameter (secret) verification failed, pointer invalid");
        iRet = ADM_ECODE_INVALID_POINTER;
    }

    if (NULL != pRegInfo->pcVehlType)
    {
        if (!adm_com_kvstorage_set_string(pstKv, "vehlType", pRegInfo->pcVehlType, ADM_KV_STORAGE_VALUE_TYPE_STRING))
        {
            iRet = ADM_ECODE_KV_SET_STRING_FAILED;
            log_w("adm_com_kvstorage_set_string pcVehlType:%s failed, reason of error code is %d", pRegInfo->pcVehlType, iRet);
        }
        else
        {
            log_i("kvstorate_set_string pcVehlType:%s successed", pRegInfo->pcVehlType);
        }
    }
    else
    {
        log_w("Parameter (secret) verification failed, pointer invalid");
        iRet = ADM_ECODE_INVALID_POINTER;
    }
/*    if (pstKv) {
        
    }
*/

    // TODO: 需要考虑，如果kvstorage存储失败，是否需要删除kv file
    // if (ADM_ECODE_SUCCESSED != iRet) {
    //     kvstorage_destroy(ADM_COM_REGISTER_RSP_INFO);
    // }

    return iRet;
}


ADM_VRS_REGISTER_INFO_T* adm_vrs_newRegisterInfo(void)
{
    ADM_VRS_REGISTER_INFO_T* pRegInfo = NULL;

    pRegInfo = adm_malloc(sizeof(ADM_VRS_REGISTER_INFO_T));
    if (NULL != pRegInfo) {
        (void)adm_memset(pRegInfo, 0x00, sizeof(ADM_VRS_REGISTER_INFO_T), sizeof(ADM_VRS_REGISTER_INFO_T));
    }

    return pRegInfo;
}

D32S adm_vrs_removeRegisterInfo()
{
    D32S iRet = ADM_ECODE_FAILED;

    lockKv();

    iRet = adm_vrs_removeRegisterInfoFromKeyValue();
    if (ADM_ECODE_SUCCESSED != iRet) 
    {
        log_e( "adm_vrs_removeRegisterInfoFromKeyValue failed, reason of error code is %d", iRet);
    } 
    else 
    {
        log_i( "adm_vrs_removeRegisterInfoFromKeyValue successed");
    }

    unlockKv();

    return iRet;
}

D32S adm_vrs_getRegisterInfo(ADM_VRS_REGISTER_INFO_T* pRegInfo)
{
    D32S iRet = ADM_ECODE_FAILED;

    lockKv();
    iRet = adm_vrs_loadRegisterInfoFromKeyValue(pRegInfo);
    if (ADM_ECODE_SUCCESSED != iRet) {
        log_e( "adm_vrs_loadRegisterInfoFromKeyValue failed, reason of error code is %d", iRet);
    } else {
        log_i( "adm_vrs_loadRegisterInfoFromKeyValue successed");
    }
    unlockKv();

    return iRet;
}

D32S adm_vrs_setRegisterInfo(const ADM_VRS_REGISTER_INFO_T* pRegInfo)
{
    D32S iRet = ADM_ECODE_FAILED;
    DCHAR *pJsonString = NULL;

    if (NULL == pRegInfo)
    {
        log_e("Parameter (pRegInfo) verification failed, pointer invalid");
        return ADM_ECODE_INVALID_PARAM;
    }

    if (NULL != pRegInfo->deviceId && 
        NULL != pRegInfo->secret && 
        NULL != pRegInfo->vinCode &&
        NULL != pRegInfo->pcVehlType)
    {
        log_i("deviceId:%s, secret:%s, vin:%s, vehlType:%s", pRegInfo->deviceId, 
                                                             pRegInfo->secret, 
                                                             pRegInfo->vinCode,
                                                             pRegInfo->pcVehlType);

        lockKv();
        iRet = adm_vrs_saveRegisterInfoToKeyValue(pRegInfo);
        if (ADM_ECODE_SUCCESSED != iRet)
        {
            log_e("adm_vrs_saveRegisterInfoToKeyValue failed, reason of error code is %d", iRet);
        }
        else
        {
            log_i("adm_vrs_saveRegisterInfoToKeyValue successed");
        }
        unlockKv();
    }
    else
    {
        log_e("Parameter (deviceId || secret || vinCode) verification failed, parameters invalid");
        return ADM_ECODE_INVALID_PARAM;
    }

    return iRet;
}

void adm_vrs_delRegisterInfo(ADM_VRS_REGISTER_INFO_T* pRegInfo)
{
    if (NULL != pRegInfo)
    {
        if (NULL != pRegInfo->vinCode)
        {
            adm_free((pRegInfo->vinCode));
        }

        if (NULL != pRegInfo->secret)
        {
            adm_free((pRegInfo->secret));
        }

        if (NULL != pRegInfo->deviceId)
        {
            adm_free((pRegInfo->deviceId));
        }

        if (NULL != pRegInfo->pcVehlType)
        {
            adm_free((pRegInfo->pcVehlType));
        }

        adm_free(pRegInfo);
    }
}

static DBOOL adm_vrs_savePowerTypeToKeyValue(ADM_VRS_VEHICLE_POWER_TYPE_E ePowerType)
{
    KvStorage_t *pstKv = NULL;

    pstKv = adm_core_service_getBusinessDatabaseDb();
    if (NULL == pstKv)
    {
        log_e("adm_core_service_getBusinessDatabaseDb failed");
        return FALSE;
    }

    ADM_KV_STORAGE_VALUE_TYPE_E eType = ADM_KV_STORAGE_VALUE_TYPE_UINT64;
    DCHAR aPowerType[4] = {0};

    (void)adm_memset(aPowerType, 0x00, sizeof(aPowerType), sizeof(aPowerType));
    (void)adm_snprintf(aPowerType, 4, "%u", (D32U)ePowerType);

    if (TRUE == adm_com_kvstorage_set_string(
                    adm_core_service_getBusinessDatabaseDb(), "vrs_vehicle_powerType", aPowerType, eType))
    {
        log_i("set power type success.");
        return TRUE;
    }
    else
    {
        log_e("set power type failed");
        return FALSE;
    }
}

static D32S adm_vrs_loadPowerTypeToKeyValue(ADM_VRS_VEHICLE_POWER_TYPE_E* pPowerType)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    DCHAR* pLoadPowerType = NULL;
    KvStorage_t *pstKv = NULL;
    size_t valLength = 0;
    ADM_KV_STORAGE_VALUE_TYPE_E eValueType = ADM_KV_STORAGE_VALUE_TYPE_INVALID;

    if (NULL != pPowerType)
    {
        pstKv = adm_core_service_getBusinessDatabaseDb();
        if (NULL == pstKv)
        {
            log_e("adm_core_service_getBusinessDatabaseDb failed");
            iRet = ADM_ECODE_KV_OPEN_FAILED;
        }
        else
        {
            if ((DBOOL)0 == adm_com_kvstorage_get_string(pstKv, "vrs_vehicle_powerType", &pLoadPowerType, &valLength, &eValueType))
            {
                iRet = ADM_ECODE_KV_GET_STRING_FAILED;
                log_w("adm_com_kvstorage_get_string get failed");
            }
            else 
            {
                (DVOID)adm_stringToUint32(pLoadPowerType, (D32U*)pPowerType);
            }
            
        }
    }
    else
    {
        iRet = ADM_ECODE_INVALID_POINTER;
        log_e("Invalid param.");
    }

    return iRet;
}

ADM_VRS_VEHICLE_POWER_TYPE_E adm_vrs_getVehicle()
{
    // 如果未确认且值为无效，返回无效值让调用方处理
    if (!g_bPowerTypeConfirmed && VEHICLE_INVALID == g_ePowerType)
    {
        log_e("Power type not confirmed yet!");
        return VEHICLE_INVALID;
    }

    return g_ePowerType;
}

DVOID adm_vrs_init(ADM_VRM_GET_VEHICLE_POWER_TYPE_CALLBACK_F fnGetVehiclePowerType)
{
    g_getVehiclePowerTypeCb = fnGetVehiclePowerType;

    (DVOID)adm_vrs_loadPowerTypeToKeyValue(&g_ePowerType);

    return;
}

/**
 * @brief 从车机主动同步获取动力类型并更新到全局变量和KV
 */
D32S adm_vrs_syncPowerTypeFromVehicle(DVOID)
{
    D32S iRet = ADM_ECODE_FAILED;
    ADM_VRS_VEHICLE_POWER_TYPE_E ePowerType = VEHICLE_INVALID;

    if (NULL == g_getVehiclePowerTypeCb)
    {
        log_e("Callback not registered");
        return ADM_ECODE_FAILED;
    }

    iRet = g_getVehiclePowerTypeCb(&ePowerType);
    if (ADM_IS_ERR(iRet))
    {
        log_e("Get power type from vehicle failed, iRet:%d", iRet);
        return iRet;
    }

    if (VEHICLE_INVALID >= ePowerType || VEHICLE_BEV < ePowerType)
    {
        log_e("Invalid power type value[%d]", ePowerType);
        return ADM_ECODE_FAILED;
    }

    g_ePowerType = ePowerType;
    g_bPowerTypeConfirmed = TRUE;

    (DVOID)adm_vrs_savePowerTypeToKeyValue(ePowerType);
    log_i("Power type synced from vehicle[%d]", ePowerType);

    return ADM_ECODE_SUCCESSED;
}

/**
 * @brief 标记动力类型为已确认状态
 * @note 用于有数据库值时，重试失败后直接使用数据库值的场景
 */
DVOID adm_vrs_confirmPowerType(DVOID)
{
    g_bPowerTypeConfirmed = TRUE;
    log_i("Power type confirmed from database[%d]", g_ePowerType);
    return;
}