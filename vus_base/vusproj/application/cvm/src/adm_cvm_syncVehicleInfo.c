#include "adm_cvm_log.h"
#include "adm_cvm_syncVehicleInfo.h"
#include "adm_cvs_reportVehicleInfo.h"
#include "adm_cvs_versionPolicy.h"
#include "adm_ocm_interface.h"
#include "adm_ocs_airProtocolAdapter.h"
#include "adm_cvm_checkVersion.h"
#include "adm_eim_ecuInfoManager.h"
#include "adm_eim_getEcuInfo.h"
#include "adm_memory.h"
#include "adm_string.h"

#include <pthread.h>

typedef struct ADM_CVM_VERSIONNUMBER_ARGV_
{
    ADM_APPLICATION_COMMON_CMDID_TYPE_E cmdId;
    DCHAR* reqExt;
} ADM_CVM_VERSIONNUMBER_ARGV_T;

static DBOOL g_isSyncVersionNumberIng = FALSE;

static void* asyncVersionNumberCB(void* argv)
{
    DCHAR* respExt = NULL;
    ADM_CVM_VERSIONNUMBER_ARGV_T* data = (ADM_CVM_VERSIONNUMBER_ARGV_T*)argv;

    do {
        if (NULL == data) {
            break;
        }

        (void)adm_cvm_syncVersionNumber(data->cmdId, data->reqExt, &respExt);
    } while (FALSE);

    if (NULL != respExt) {
        adm_free(respExt);
    }
    if (NULL != data->reqExt) {
        adm_free(data->reqExt);
    }
    adm_free(data);
    g_isSyncVersionNumberIng = FALSE;
    return NULL;
}

D32S adm_cvm_asyncVersionNumber(ADM_APPLICATION_COMMON_CMDID_TYPE_E cmdId, const DCHAR* reqExt)
{
    if (cmdId >= ADM_APPLICATION_COMMON_CMDID_TYPE_INVALID) {
        log_e("param notifyF is NULL");
        return ADM_ECODE_INVALID_PARAM;
    }

    if (FALSE == g_isSyncVersionNumberIng) {
        pthread_t id;
        ADM_CVM_VERSIONNUMBER_ARGV_T* argv = NULL;
        argv = (ADM_CVM_VERSIONNUMBER_ARGV_T*)adm_malloc(sizeof(ADM_CVM_VERSIONNUMBER_ARGV_T));
        argv->cmdId = cmdId;

        if (NULL != reqExt) {
            argv->reqExt = adm_strdup(reqExt);
        }

        if (0 == pthread_create(&id, NULL, asyncVersionNumberCB, (void*)argv)) {
            (void)pthread_detach(id);
            g_isSyncVersionNumberIng = TRUE;
        } else {
            log_e("creat thread failed");
        }
    } else {
        log_w("adm_cvm_asyncVersionNumber has start");
    }
    return ADM_ECODE_SUCCESSED;
}

D32S adm_cvm_syncVersionNumber(ADM_APPLICATION_COMMON_CMDID_TYPE_E cmdId, const DCHAR* reqExt, DCHAR** respExt)
{
    if (cmdId >= ADM_APPLICATION_COMMON_CMDID_TYPE_INVALID) {
        log_e("param notifyF is NULL");
        return ADM_ECODE_INVALID_PARAM;
    }

    ADM_OCS_CHECK_REQ_ADAPTER_T* checkReq = NULL;
    D64S timestamp;
    DCHAR* netType = NULL;
    DCHAR* versionNumber = NULL;
    DCHAR* sdkVersion = NULL;
    DCHAR* allEcuInfoStr = NULL;
    DCHAR* allModuleVerInfoStr = NULL;
    DCHAR* confVersion = NULL;
    D32S retVal = ADM_ECODE_FAILED;
    D32S status;
    DCHAR* respVersionNumber = NULL;
    DCHAR* pVehicleVersion = NULL;

    do {
        log_i("get confVersion from ecs");
        confVersion = adm_ecs_getRsConfigVersion();
        if (NULL == confVersion) {
            retVal = ADM_ECODE_APPLY_MEMORY_FAILED;
            log_e("get confVersion ,error[%d]", retVal);
            break;
        }

        retVal = adm_eim_getEcuInfoManagerKvGetStr(&allEcuInfoStr);
        if (ADM_ECODE_SUCCESSED != retVal) {
            retVal = adm_eim_getEcuInfoManagerDimGetStr(&allEcuInfoStr, COLLECT_TRIGGER_TYPE_ING);
            if (ADM_ECODE_SUCCESSED != retVal) {
                retVal = ADM_ECODE_EIM_GET_ECU_INFO_FAILED;
                log_e("get ecu info str. error[%d]", retVal);
                break;
            }
        }
        log_i("allEcuInfoStr = %s", allEcuInfoStr);

        retVal = adm_cvm_getReqFieldValue(&timestamp, &netType, &versionNumber, &sdkVersion, &allModuleVerInfoStr);
        if (ADM_ECODE_SUCCESSED != retVal) {
            log_e("get Req field value. failed");
            break;
        }

        //adm_cvs_getHmiVehicleVersion(&pVehicleVersion);
        adm_eim_getVehicleVersion(&pVehicleVersion);
        if (NULL == pVehicleVersion)
        {
#ifdef AIR_PROTOCOL_VEHICLE_VERSION
            pVehicleVersion = adm_strdup(AIR_PROTOCOL_VEHICLE_VERSION);
#endif            
        }

        log_i("new check req body");
        checkReq = adm_ocs_newCheckReqAdapterBody((D32S)cmdId,
            timestamp,
            netType,
            versionNumber,
            sdkVersion,
            confVersion,
            allEcuInfoStr,
            allModuleVerInfoStr,
            pVehicleVersion,
            0,
            reqExt,
            adm_ocm_businessCommInterface);
        if (NULL == checkReq) {
            retVal = ADM_ECODE_APPLY_MEMORY_FAILED;
            log_e("call adm_ocs_newCheckReqAdapterBody.failed");
            break;
        }

        retVal = adm_cvs_reportVehicleInfo(checkReq, &status, respExt, &respVersionNumber);
        if (ADM_ECODE_SUCCESSED == retVal) {
            log_i("sync vehicle info success");
            if (NULL != respVersionNumber) {
                log_i("Cloud return versionNumber = %s", respVersionNumber);
                retVal = adm_eim_setVehicleBigVersion(respVersionNumber);
                if (ADM_ECODE_SUCCESSED == retVal) {
                    log_e("set vehicle Number error[%d]", retVal);
                    break;
                }
            } else {
                log_i("Cloud no return versionNumber,it is NULL");
            }
        } else {
            log_e("sync vehicle info error[%d]", retVal);
            break;
        }
        retVal = ADM_ECODE_SUCCESSED;
    } while (FALSE);

    adm_free(netType);
    adm_free(versionNumber);
    adm_free(sdkVersion);
    adm_free(allEcuInfoStr);
    adm_free(allModuleVerInfoStr);
    adm_free(confVersion);
    adm_ocs_deleteCheckReqAdapterBody(checkReq);
    if (NULL != respVersionNumber) {
        adm_free(respVersionNumber);
    }
    return retVal;
}
