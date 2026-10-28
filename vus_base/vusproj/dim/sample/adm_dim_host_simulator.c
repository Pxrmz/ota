#include "adm_dim_log.h"
#include "adm_dim_host_simulator.h"

#include <signal.h>
#include <unistd.h>

#include "adm_typedefs.h"
#include "adm_memory.h"
#include "adm_string.h"

#include "adm_dim_host.h"
#include "adm_dim_ucl.h"

#include "adm_com_errorCode.h"
#include "adm_soa.h"
#include "adm_soa_cfg.h"
#include "adm_cfg_api.h"
#include "adm_dim_common.h"


#define TIME_WAIT 5

static D32S getVinTest()
{
    log_i("getVinTeat Begin");
    DCHAR* vin = NULL;

    D32S retCode = 0;
    D32S errorCode = 0;
    retCode = adm_dim_getVin(adm_soa_getHostInstanceId(), TIME_WAIT, &vin, &errorCode);

    log_i("ret code: %d", retCode);
    log_i("resp vin: %s", vin);

    adm_free(vin);

    log_i("getVinTeat End");

    return retCode;
}

static void req_dim_getVehicleVer()
{
    log_i("req_dim_getVehicleVer Begin");
    DCHAR* pVehicleVer = NULL;

    D32S retCode = 0;
    D32S errorCode = 0;
    retCode = adm_dim_getVehicleVersion(adm_soa_getHostInstanceId(), TIME_WAIT, &pVehicleVer, &errorCode);

    log_i("ret code: %d", retCode);
    log_i("resp vehicleVersion: %s", pVehicleVer);

    adm_free(pVehicleVer);

    log_i("req_dim_getVehicleVer End");

    return;
}
static D32S req_dim_setVehicleVer()
{
    log_i("setVehicleVer Begin");

    D32S retCode = 0;
    D32S errorCode = 0;

    DCHAR* pVehicleVer = "vehicleVersion54321";
    retCode = adm_dim_setVehicleVersion(adm_soa_getHostInstanceId(), TIME_WAIT, pVehicleVer, &errorCode);

    log_i("ret code: %d", retCode);

    log_i("setVehicleVer End");

    return retCode;

}



static D32S execOtaTaskTest()
{
    log_i("getVinTeat Begin");

    D32S retCode = 0;
    D32S errorCode = 0;

    //进入OTA模式
    ADM_DIM_SOA_HOST_OTA_TASK_TYPE_E eOtaTaskType = ADM_DIM_SOA_HOST_OTA_TASK_TYPE_START_OTA_ACTION;
    retCode = adm_dim_host_execOtaTask(adm_soa_getHostInstanceId(), TIME_WAIT, eOtaTaskType, 0, &errorCode);
    //退出OTA模式
    eOtaTaskType = ADM_DIM_SOA_HOST_OTA_TASK_TYPE_STOP_OTA_ACTION;
    retCode = adm_dim_host_execOtaTask(adm_soa_getHostInstanceId(), TIME_WAIT, eOtaTaskType, 0, &errorCode);
    //维持整车网络唤醒
    eOtaTaskType = ADM_DIM_SOA_HOST_OTA_TASK_TYPE_START_WAKE_UP;
    retCode = adm_dim_host_execOtaTask(adm_soa_getHostInstanceId(), TIME_WAIT, eOtaTaskType, 0, &errorCode);
    //退出整车网络唤醒
    eOtaTaskType = ADM_DIM_SOA_HOST_OTA_TASK_TYPE_STOP_WAKE_UP;
    retCode = adm_dim_host_execOtaTask(adm_soa_getHostInstanceId(), TIME_WAIT, eOtaTaskType, 0, &errorCode);
    //进入整车静默
    eOtaTaskType = ADM_DIM_SOA_HOST_OTA_TASK_TYPE_ENTER_VEHICLE_SILENT;
    retCode = adm_dim_host_execOtaTask(adm_soa_getHostInstanceId(), TIME_WAIT, eOtaTaskType, 0, &errorCode);
    //退出整车静默
    eOtaTaskType = ADM_DIM_SOA_HOST_OTA_TASK_TYPE_EXIT_VEHICLE_SILENT;
    retCode = adm_dim_host_execOtaTask(adm_soa_getHostInstanceId(), TIME_WAIT, eOtaTaskType, 0, &errorCode);
    //停止整车静默
    eOtaTaskType = ADM_DIM_SOA_HOST_OTA_TASK_TYPE_STOP_VEHICLE_SILENT;
    retCode = adm_dim_host_execOtaTask(adm_soa_getHostInstanceId(), TIME_WAIT, eOtaTaskType, 0, &errorCode);
    //远程上电
    eOtaTaskType = ADM_DIM_SOA_HOST_OTA_TASK_TYPE_REMOTE_POWER_ON;
    retCode = adm_dim_host_execOtaTask(adm_soa_getHostInstanceId(), TIME_WAIT, eOtaTaskType, 0, &errorCode);
    //远程下电
    eOtaTaskType = ADM_DIM_SOA_HOST_OTA_TASK_TYPE_REMOTE_POWER_OFF;
    retCode = adm_dim_host_execOtaTask(adm_soa_getHostInstanceId(), TIME_WAIT, eOtaTaskType, 0, &errorCode);
    //进入升级模式
    eOtaTaskType = ADM_DIM_SOA_HOST_OTA_TASK_TYPE_ENTER_UPGRADE_MODE;
    retCode = adm_dim_host_execOtaTask(adm_soa_getHostInstanceId(), TIME_WAIT, eOtaTaskType, 0, &errorCode);
    //进入升级模式
    eOtaTaskType = ADM_DIM_SOA_HOST_OTA_TASK_TYPE_EXIT_UPGRADE_MODE;
    retCode = adm_dim_host_execOtaTask(adm_soa_getHostInstanceId(), TIME_WAIT, eOtaTaskType, 0, &errorCode);

    log_i("ret code: %d", retCode);
    /*
        log_i("resp errorcode: %d", errorCode);
    */
    log_i("getVinTeat End");

    return retCode;
}

static D32S setSchduleWakeupTimeTest()
{
    log_i("setSchduleWakeuptime Begin");

    D32S retCode = 0;
    D32S errorCode = 0;

    D64S schduleTime = 1000;

    retCode = adm_dim_host_setSchduleWakeUpTime(adm_soa_getHostInstanceId(), TIME_WAIT,
              ADM_DIM_HOST_SCHDULE_TYPE_SCHDULED, schduleTime, &errorCode);

    retCode = adm_dim_host_setSchduleWakeUpTime(adm_soa_getHostInstanceId(), TIME_WAIT,
              ADM_DIM_HOST_SCHDULE_TYPE_CANCELED, schduleTime, &errorCode);

    log_i("ret code: %d", retCode);

    log_i("setSchduleWakeuptime End");

    return retCode;
}
static D32S getSchduleWakeupTimeTest()
{
    log_i("setSchduleWakeuptime Begin");

    D32S retCode = 0;
    D32S errorCode = 0;

    D64S schduleTime = 0;

    retCode = adm_dim_host_getSchduleWakeUpTime(adm_soa_getHostInstanceId(), TIME_WAIT, &schduleTime, &errorCode);

    log_i("ret code: %d", retCode);

    log_i("setSchduleWakeuptime End");

    return retCode;
}


static D32S setVehicleBigversionTest()
{
    log_i("setSchduleWakeuptime Begin");

    D32S retCode = 0;
    D32S errorCode = 0;

    DCHAR* vehicleBigVersion = "v1.0.1";
    retCode = adm_dim_host_setVehicleBigVersion(adm_soa_getHostInstanceId(), TIME_WAIT,
              vehicleBigVersion, &errorCode);

    log_i("ret code: %d", retCode);

    log_i("setSchduleWakeuptime End");

    return retCode;

}

static D32S getVehicleBigversionTest()
{
    log_i("getVehicleBigversion Begin");

    D32S retCode = 0;
    D32S errorCode = 0;

    DCHAR* vehicleBigVersion;
    retCode = adm_dim_host_getVehicleBigVersion(adm_soa_getHostInstanceId(), TIME_WAIT,
              &vehicleBigVersion, &errorCode);

    log_i("vehicleBigVersion=%s, ret code: %d", vehicleBigVersion, retCode);
    log_i("getVehicleBigversion End");

    return retCode;

}

static D32S verifySpaceisenoughTest()
{
    log_i("verifySpaceisenough Begin");

    D32S retCode = 0;
    D32S errorCode = 0;

    const D64S needSize = 1000;
    D64S realSize = 0;
    retCode = adm_dim_host_verifySpaceIsEnough(adm_soa_getHostInstanceId(), TIME_WAIT, needSize,
              &realSize, &errorCode);

    log_i("ret code: %d", retCode);
    log_i("verifySpaceisenough End");

    return retCode;

}

static D32S verifySignatureTest()
{
    log_i("verifySignature Begin");

    D32S retCode = 0;
    D32S errorCode = 0;

    const DCHAR* signedData = "12345";
    const D32S signedDataLength = 5;
    const DCHAR* orignalData = "abcdef";
    const D32S orignalDataLength = 6;
    D64S realSize = 0 ;
    retCode = adm_dim_host_verifySignature(adm_soa_getHostInstanceId(), TIME_WAIT, signedData,
                                           signedDataLength, orignalData, orignalDataLength, &errorCode);

    log_i("ret code: %d", retCode);
    log_i("verifySignature End");

    return retCode;

}

static D32S getCertabsolutepathTest()
{
    log_i("getCertabsolutepath Begin");

    D32S retCode = 0;
    D32S errorCode = 0;

    const ADM_DIM_SOA_HOST_CERT_TYPE_E eCertType = ADM_DIM_SOA_HOST_CERT_TYPE_CLIENT_CERT;
    DCHAR* certPath = "" ;
    retCode = adm_dim_host_getCertAbsolutePath(adm_soa_getHostInstanceId(), TIME_WAIT, eCertType,
              &certPath, &errorCode);

    if(ADM_ECODE_SUCCESSED != retCode)
    {
        log_i("call fail");
        return retCode;
    }

    adm_free(certPath);
    log_i("ret code: %d", retCode);
    log_i("getCertabsolutepath End");

    return retCode;

}

static D32S setDtcTest()
{
    log_i("setDtc Begin");

    D32S retCode = 0;
    D32S errorCode = 0;

    const D32S diagTroubleCode = 3;
    const DCHAR* dtcSnapShop = "abc";
    retCode = adm_dim_host_setDiagTroubleCode(adm_soa_getHostInstanceId(), TIME_WAIT, diagTroubleCode,
              dtcSnapShop, &errorCode);

    log_i("ret code: %d", retCode);
    log_i("setDtc End");

    return retCode;

}

static D32S clearDtcTest()
{
    log_i("clearDtc Begin");

    D32S retCode = 0;
    D32S errorCode = 0;

    const D32S diagTroubleCode = 3;
    retCode = adm_dim_host_clearDiagTroubleCode(adm_soa_getHostInstanceId(), TIME_WAIT, diagTroubleCode,
              &errorCode);

    log_i("ret code: %d", retCode);
    log_i("clearDtc End");

    return retCode;

}

static void request_dim_host_set_log_level()
{
    log_i("request_dim_host_set_log_level********host test");
    D16U instanceId = 0X3001;
    D32S errorCode = 1;
    D32U logLevel = 3;
    log_i("req errorCode = %d", errorCode);
    D32S ret = adm_dim_host_set_log_level(instanceId, TIME_WAIT, logLevel, &errorCode);
    log_i("ret = %d", ret);
    log_i("resp errorCode = %d", errorCode);
}

static void request_dim_host_GetSdkVersion()
{
    log_i("request_dim_host getSdkVersion ********host test");
    D16U instanceId = 0X3001;
    DCHAR* sdkVersion = NULL;
    D32S errorCode = 1;
    adm_dim_host_getSdkVersion(instanceId, TIME_WAIT, &sdkVersion, &errorCode);

    log_i("sdkVersion = %s", sdkVersion);
    log_i("errorCode = %d", errorCode);

    adm_free(sdkVersion);

    log_i("request_dim_host getSdkVersion end");
}

static void request_dim_host_GetNetworkType()
{
    D16U instanceId = 0X3001;
    D32S networkType = 0;
    D32S errorCode = 1;

    (void)adm_dim_host_getNetworkType(instanceId, TIME_WAIT, &networkType, &errorCode);

    log_i("networkType = %d", networkType);
    log_i("errorCode = %d", errorCode);
}


//recv notify
static D32S signalEventNotify(const ADM_DIM_SOA_HOST_SIGNAL_EVENT_TYPE_E eSignalEventType, void* priv)
{
    log_i("signalEventNotify begin");

    log_i("notify eSignalEventType = %d", eSignalEventType);

    log_i("signalEventNotify end");
    return 0;
}

static D32S triggerEventNotify(const ADM_DIM_SOA_HOST_TRIGGER_EVENT_TYPE_E eTriggerEventType, void* priv)
{
    log_i("triggerEventNotify begin");

    log_i("notify eTriggerEventType = %d", eTriggerEventType);

    log_i("triggerEventNotify end");
    return 0;
}

static D32S request_dim_host_readVehicleStatus_Test()
{
    log_i("************ begin ************");

    D32S errorCode = 0;

    ADM_DIM_SOA_MSGID_DATA_ARRAY_T msgIdListRequest;
    ADM_DIM_SOA_MSGID_DATA_ARRAY_T* msgIdListResponse = NULL;
    msgIdListRequest.msgIdArrayLength = 24;
    msgIdListRequest.msgIdArray = (ADM_DIM_SOA_MSGID_T**)adm_malloc(sizeof(ADM_DIM_SOA_MSGID_T*) *
                                  msgIdListRequest.msgIdArrayLength);

    //获取挡位状态
    msgIdListRequest.msgIdArray[0] = (ADM_DIM_SOA_MSGID_T*)adm_malloc(sizeof(ADM_DIM_SOA_MSGID_T));
    msgIdListRequest.msgIdArray[0]->msgId = VEHICLE_STATUS_GEAR;
    msgIdListRequest.msgIdArray[0]->msgIdData = (ADM_DIM_SOA_DATA_T*)adm_malloc(sizeof(ADM_DIM_SOA_DATA_T));
    msgIdListRequest.msgIdArray[0]->msgIdData->dataType = VAR_S32;
    msgIdListRequest.msgIdArray[0]->msgIdData->dataLength = sizeof(D32S);
    //获取电源状态
    msgIdListRequest.msgIdArray[1] = (ADM_DIM_SOA_MSGID_T*)adm_malloc(sizeof(ADM_DIM_SOA_MSGID_T));
    msgIdListRequest.msgIdArray[1]->msgId = VEHICLE_STATUS_POWER_SIGNAL;
    msgIdListRequest.msgIdArray[1]->msgIdData = (ADM_DIM_SOA_DATA_T*)adm_malloc(sizeof(ADM_DIM_SOA_DATA_T));
    msgIdListRequest.msgIdArray[1]->msgIdData->dataType = VAR_S32;
    msgIdListRequest.msgIdArray[1]->msgIdData->dataLength = sizeof(D32S);
    //获取车速
    msgIdListRequest.msgIdArray[2] = (ADM_DIM_SOA_MSGID_T*)adm_malloc(sizeof(ADM_DIM_SOA_MSGID_T));
    msgIdListRequest.msgIdArray[2]->msgId = VEHICLE_STATUS_VSPD;
    msgIdListRequest.msgIdArray[2]->msgIdData = (ADM_DIM_SOA_DATA_T*)adm_malloc(sizeof(ADM_DIM_SOA_DATA_T));
    msgIdListRequest.msgIdArray[2]->msgIdData->dataType = VAR_S32;
    msgIdListRequest.msgIdArray[2]->msgIdData->dataLength = sizeof(D32S);
    //获取解防设防状态
    msgIdListRequest.msgIdArray[3] = (ADM_DIM_SOA_MSGID_T*)adm_malloc(sizeof(ADM_DIM_SOA_MSGID_T));
    msgIdListRequest.msgIdArray[3]->msgId = VEHICLE_STATUS_FORTIFY;
    msgIdListRequest.msgIdArray[3]->msgIdData = (ADM_DIM_SOA_DATA_T*)adm_malloc(sizeof(ADM_DIM_SOA_DATA_T));
    msgIdListRequest.msgIdArray[3]->msgIdData->dataType = VAR_S32;
    msgIdListRequest.msgIdArray[3]->msgIdData->dataLength = sizeof(D32S);
    //获取发动机状态
    msgIdListRequest.msgIdArray[4] = (ADM_DIM_SOA_MSGID_T*)adm_malloc(sizeof(ADM_DIM_SOA_MSGID_T));
    msgIdListRequest.msgIdArray[4]->msgId = VEHICLE_STATUS_ENGINE_STATE;
    msgIdListRequest.msgIdArray[4]->msgIdData = (ADM_DIM_SOA_DATA_T*)adm_malloc(sizeof(ADM_DIM_SOA_DATA_T));
    msgIdListRequest.msgIdArray[4]->msgIdData->dataType = VAR_S32;
    msgIdListRequest.msgIdArray[4]->msgIdData->dataLength = sizeof(D32S);
    //获取是否哨兵模式
    msgIdListRequest.msgIdArray[5] = (ADM_DIM_SOA_MSGID_T*)adm_malloc(sizeof(ADM_DIM_SOA_MSGID_T));
    msgIdListRequest.msgIdArray[5]->msgId = VEHICLE_STATUS_SENTINEL_MODE;
    msgIdListRequest.msgIdArray[5]->msgIdData = (ADM_DIM_SOA_DATA_T*)adm_malloc(sizeof(ADM_DIM_SOA_DATA_T));
    msgIdListRequest.msgIdArray[5]->msgIdData->dataType = VAR_S32;
    msgIdListRequest.msgIdArray[5]->msgIdData->dataLength = sizeof(D32S);
    //获取是否童宠模式
    msgIdListRequest.msgIdArray[6] = (ADM_DIM_SOA_MSGID_T*)adm_malloc(sizeof(ADM_DIM_SOA_MSGID_T));
    msgIdListRequest.msgIdArray[6]->msgId = VEHICLE_STATUS_CHILDPET_MODE;
    msgIdListRequest.msgIdArray[6]->msgIdData = (ADM_DIM_SOA_DATA_T*)adm_malloc(sizeof(ADM_DIM_SOA_DATA_T));
    msgIdListRequest.msgIdArray[6]->msgIdData->dataType = VAR_S32;
    msgIdListRequest.msgIdArray[6]->msgIdData->dataLength = sizeof(D32S);
    //获取OBD状态
    msgIdListRequest.msgIdArray[7] = (ADM_DIM_SOA_MSGID_T*)adm_malloc(sizeof(ADM_DIM_SOA_MSGID_T));
    msgIdListRequest.msgIdArray[7]->msgId = VEHICLE_STATUS_OBD_STATE;
    msgIdListRequest.msgIdArray[7]->msgIdData = (ADM_DIM_SOA_DATA_T*)adm_malloc(sizeof(ADM_DIM_SOA_DATA_T));
    msgIdListRequest.msgIdArray[7]->msgIdData->dataType = VAR_S32;
    msgIdListRequest.msgIdArray[7]->msgIdData->dataLength = sizeof(D32S);
    //获取ESP驻车状态
    msgIdListRequest.msgIdArray[8] = (ADM_DIM_SOA_MSGID_T*)adm_malloc(sizeof(ADM_DIM_SOA_MSGID_T));
    msgIdListRequest.msgIdArray[8]->msgId = VEHICLE_STATUS_ESPPARK_STATE;
    msgIdListRequest.msgIdArray[8]->msgIdData = (ADM_DIM_SOA_DATA_T*)adm_malloc(sizeof(ADM_DIM_SOA_DATA_T));
    msgIdListRequest.msgIdArray[8]->msgIdData->dataType = VAR_S32;
    msgIdListRequest.msgIdArray[8]->msgIdData->dataLength = sizeof(D32S);
    //获取动力电池电量
    msgIdListRequest.msgIdArray[9] = (ADM_DIM_SOA_MSGID_T*)adm_malloc(sizeof(ADM_DIM_SOA_MSGID_T));
    msgIdListRequest.msgIdArray[9]->msgId = VEHICLE_STATUS_POWER_SOC;
    msgIdListRequest.msgIdArray[9]->msgIdData = (ADM_DIM_SOA_DATA_T*)adm_malloc(sizeof(ADM_DIM_SOA_DATA_T));
    msgIdListRequest.msgIdArray[9]->msgIdData->dataType = VAR_S32;
    msgIdListRequest.msgIdArray[9]->msgIdData->dataLength = sizeof(D32S);
    //获取小电池电量
    msgIdListRequest.msgIdArray[10] = (ADM_DIM_SOA_MSGID_T*)adm_malloc(sizeof(ADM_DIM_SOA_MSGID_T));
    msgIdListRequest.msgIdArray[10]->msgId = VEHICLE_STATUS_SOC;
    msgIdListRequest.msgIdArray[10]->msgIdData = (ADM_DIM_SOA_DATA_T*)adm_malloc(sizeof(ADM_DIM_SOA_DATA_T));
    msgIdListRequest.msgIdArray[10]->msgIdData->dataType = VAR_S32;
    msgIdListRequest.msgIdArray[10]->msgIdData->dataLength = sizeof(D32S);
    //获取充电状态
    msgIdListRequest.msgIdArray[11] = (ADM_DIM_SOA_MSGID_T*)adm_malloc(sizeof(ADM_DIM_SOA_MSGID_T));
    msgIdListRequest.msgIdArray[11]->msgId = VEHICLE_STATUS_CHARGING;
    msgIdListRequest.msgIdArray[11]->msgIdData = (ADM_DIM_SOA_DATA_T*)adm_malloc(sizeof(ADM_DIM_SOA_DATA_T));
    msgIdListRequest.msgIdArray[11]->msgIdData->dataType = VAR_S32;
    msgIdListRequest.msgIdArray[11]->msgIdData->dataLength = sizeof(D32S);
    //获取车辆动力类型
    msgIdListRequest.msgIdArray[12] = (ADM_DIM_SOA_MSGID_T*)adm_malloc(sizeof(ADM_DIM_SOA_MSGID_T));
    msgIdListRequest.msgIdArray[12]->msgId = VEHICLE_STATUS_DRIVE_MODE;
    msgIdListRequest.msgIdArray[12]->msgIdData = (ADM_DIM_SOA_DATA_T*)adm_malloc(sizeof(ADM_DIM_SOA_DATA_T));
    msgIdListRequest.msgIdArray[12]->msgIdData->dataType = VAR_S32;
    msgIdListRequest.msgIdArray[12]->msgIdData->dataLength = sizeof(D32S);
    //获取车辆蓄电池电压
    msgIdListRequest.msgIdArray[13] = (ADM_DIM_SOA_MSGID_T*)adm_malloc(sizeof(ADM_DIM_SOA_MSGID_T));
    msgIdListRequest.msgIdArray[13]->msgId = VEHICLE_STATUS_VOL_MV;
    msgIdListRequest.msgIdArray[13]->msgIdData = (ADM_DIM_SOA_DATA_T*)adm_malloc(sizeof(ADM_DIM_SOA_DATA_T));
    msgIdListRequest.msgIdArray[13]->msgIdData->dataType = VAR_S32;
    msgIdListRequest.msgIdArray[13]->msgIdData->dataLength = sizeof(D32S);
    //获取车辆高压状态
    msgIdListRequest.msgIdArray[14] = (ADM_DIM_SOA_MSGID_T*)adm_malloc(sizeof(ADM_DIM_SOA_MSGID_T));
    msgIdListRequest.msgIdArray[14]->msgId = VEHICLE_STATUS_HIGH_VOLTAGE;
    msgIdListRequest.msgIdArray[14]->msgIdData = (ADM_DIM_SOA_DATA_T*)adm_malloc(sizeof(ADM_DIM_SOA_DATA_T));
    msgIdListRequest.msgIdArray[14]->msgIdData->dataType = VAR_S32;
    msgIdListRequest.msgIdArray[14]->msgIdData->dataLength = sizeof(D32S);
    //获取车内网络类型
    msgIdListRequest.msgIdArray[15] = (ADM_DIM_SOA_MSGID_T*)adm_malloc(sizeof(ADM_DIM_SOA_MSGID_T));
    msgIdListRequest.msgIdArray[15]->msgId = VEHICLE_STATUS_ETHERENT_TYPE;
    msgIdListRequest.msgIdArray[15]->msgIdData = (ADM_DIM_SOA_DATA_T*)adm_malloc(sizeof(ADM_DIM_SOA_DATA_T));
    msgIdListRequest.msgIdArray[15]->msgIdData->dataType = VAR_S32;
    msgIdListRequest.msgIdArray[15]->msgIdData->dataLength = sizeof(D32S);
    //获取车外网络类型
    msgIdListRequest.msgIdArray[16] = (ADM_DIM_SOA_MSGID_T*)adm_malloc(sizeof(ADM_DIM_SOA_MSGID_T));
    msgIdListRequest.msgIdArray[16]->msgId = VEHICLE_STATUS_TBOXNETWORK_TYPE;
    msgIdListRequest.msgIdArray[16]->msgIdData = (ADM_DIM_SOA_DATA_T*)adm_malloc(sizeof(ADM_DIM_SOA_DATA_T));
    msgIdListRequest.msgIdArray[16]->msgIdData->dataType = VAR_S32;
    msgIdListRequest.msgIdArray[16]->msgIdData->dataLength = sizeof(D32S);
    //获取国家码
    msgIdListRequest.msgIdArray[17] = (ADM_DIM_SOA_MSGID_T*)adm_malloc(sizeof(ADM_DIM_SOA_MSGID_T));
    msgIdListRequest.msgIdArray[17]->msgId = VEHICLE_STATUS_COUNTRY_CODE;
    msgIdListRequest.msgIdArray[17]->msgIdData = (ADM_DIM_SOA_DATA_T*)adm_malloc(sizeof(ADM_DIM_SOA_DATA_T) + 20);
    msgIdListRequest.msgIdArray[17]->msgIdData->dataType = VAR_STRING;
    msgIdListRequest.msgIdArray[17]->msgIdData->dataLength = 20;
    //获取整车模式类型
    msgIdListRequest.msgIdArray[18] = (ADM_DIM_SOA_MSGID_T*)adm_malloc(sizeof(ADM_DIM_SOA_MSGID_T));
    msgIdListRequest.msgIdArray[18]->msgId = VEHICLE_STATUS_MODE_TYPE;
    msgIdListRequest.msgIdArray[18]->msgIdData = (ADM_DIM_SOA_DATA_T*)adm_malloc(sizeof(ADM_DIM_SOA_DATA_T));
    msgIdListRequest.msgIdArray[18]->msgIdData->dataType = VAR_S32;
    msgIdListRequest.msgIdArray[18]->msgIdData->dataLength = sizeof(D32S);
    //获取车辆锁车状态
    msgIdListRequest.msgIdArray[19] = (ADM_DIM_SOA_MSGID_T*)adm_malloc(sizeof(ADM_DIM_SOA_MSGID_T));
    msgIdListRequest.msgIdArray[19]->msgId = VEHICLE_STATUS_LOCK;
    msgIdListRequest.msgIdArray[19]->msgIdData = (ADM_DIM_SOA_DATA_T*)adm_malloc(sizeof(ADM_DIM_SOA_DATA_T));
    msgIdListRequest.msgIdArray[19]->msgIdData->dataType = VAR_S32;
    msgIdListRequest.msgIdArray[19]->msgIdData->dataLength = sizeof(D32S);
    //获取车辆放电状态
    msgIdListRequest.msgIdArray[20] = (ADM_DIM_SOA_MSGID_T*)adm_malloc(sizeof(ADM_DIM_SOA_MSGID_T));
    msgIdListRequest.msgIdArray[20]->msgId = VEHICLE_STATUS_DISCHARGE;
    msgIdListRequest.msgIdArray[20]->msgIdData = (ADM_DIM_SOA_DATA_T*)adm_malloc(sizeof(ADM_DIM_SOA_DATA_T));
    msgIdListRequest.msgIdArray[20]->msgIdData->dataType = VAR_S32;
    msgIdListRequest.msgIdArray[20]->msgIdData->dataLength = sizeof(D32S);
    //获取车辆READY状态
    msgIdListRequest.msgIdArray[21] = (ADM_DIM_SOA_MSGID_T*)adm_malloc(sizeof(ADM_DIM_SOA_MSGID_T));
    msgIdListRequest.msgIdArray[21]->msgId = VEHICLE_STATUS_READY;
    msgIdListRequest.msgIdArray[21]->msgIdData = (ADM_DIM_SOA_DATA_T*)adm_malloc(sizeof(ADM_DIM_SOA_DATA_T));
    msgIdListRequest.msgIdArray[21]->msgIdData->dataType = VAR_S32;
    msgIdListRequest.msgIdArray[21]->msgIdData->dataLength = sizeof(D32S);
    //获取动力电池供电状态
    msgIdListRequest.msgIdArray[22] = (ADM_DIM_SOA_MSGID_T*)adm_malloc(sizeof(ADM_DIM_SOA_MSGID_T));
    msgIdListRequest.msgIdArray[22]->msgId = VEHICLE_STATUS_POWER_BATTERY_SUP;
    msgIdListRequest.msgIdArray[22]->msgIdData = (ADM_DIM_SOA_DATA_T*)adm_malloc(sizeof(ADM_DIM_SOA_DATA_T));
    msgIdListRequest.msgIdArray[22]->msgIdData->dataType = VAR_S32;
    msgIdListRequest.msgIdArray[22]->msgIdData->dataLength = sizeof(D32S);
    //获取ECALL状态
    msgIdListRequest.msgIdArray[23] = (ADM_DIM_SOA_MSGID_T*)adm_malloc(sizeof(ADM_DIM_SOA_MSGID_T));
    msgIdListRequest.msgIdArray[23]->msgId = VEHICLE_STATUS_ECALL;
    msgIdListRequest.msgIdArray[23]->msgIdData = (ADM_DIM_SOA_DATA_T*)adm_malloc(sizeof(ADM_DIM_SOA_DATA_T));
    msgIdListRequest.msgIdArray[23]->msgIdData->dataType = VAR_S32;
    msgIdListRequest.msgIdArray[23]->msgIdData->dataLength = sizeof(D32S);


    D32S retVal = adm_dim_host_readVehicleStatus(adm_soa_getHostInstanceId(), 5,
                  &msgIdListRequest, &msgIdListResponse, &errorCode);

    log_i("resp errorcode = %d", errorCode);

    if(retVal != ADM_ECODE_SUCCESSED)
    {
        log_w("call adm_dim_host_readVehicleStatus fail,retVal = %d", retVal);

        for(int index = 0; index < msgIdListRequest.msgIdArrayLength; ++index)
        {
            adm_free(msgIdListRequest.msgIdArray[index]->msgIdData);
            adm_free(msgIdListRequest.msgIdArray[index]);
        }

        adm_free(msgIdListRequest.msgIdArray);

        return retVal;
    }

    if(msgIdListResponse)
    {
        for(int index = 0; index < msgIdListResponse->msgIdArrayLength; ++index)
        {
            if(VAR_S32 == msgIdListResponse->msgIdArray[index]->msgIdData->dataType)
            {
                log_i("resp data:msgId[%#x],value=%d", msgIdListResponse->msgIdArray[index]->msgId,
                      msgIdListResponse->msgIdArray[index]->msgIdData->v.valueS32);
            }
            else if(VAR_STRING == msgIdListResponse->msgIdArray[index]->msgIdData->dataType)
            {
                log_i("resp data:msgId[%#x],value=%s", msgIdListResponse->msgIdArray[index]->msgId,
                      msgIdListResponse->msgIdArray[index]->msgIdData->v.valueS);
            }

            adm_free(msgIdListResponse->msgIdArray[index]->msgIdData);
            adm_free(msgIdListResponse->msgIdArray[index]);
        }

        adm_free(msgIdListResponse->msgIdArray);
        adm_free(msgIdListResponse);
    }

    log_i("************ end ************");

    return ADM_ECODE_SUCCESSED;
}

static D32S request_dim_host_setVehicleStatus_Test()
{
    ADM_DIM_SOA_MSGID_DATA_ARRAY_T msgIdListRequest = {0};
    msgIdListRequest.msgIdArrayLength = 11;

    msgIdListRequest.msgIdArray =
        (ADM_DIM_SOA_MSGID_T**)adm_malloc(sizeof(ADM_DIM_SOA_MSGID_T*) * msgIdListRequest.msgIdArrayLength);

    //中控屏和仪表亮屏
    msgIdListRequest.msgIdArray[0] = (ADM_DIM_SOA_MSGID_T*)adm_malloc(sizeof(ADM_DIM_SOA_MSGID_T));
    msgIdListRequest.msgIdArray[0]->msgId = VEHICLE_CTRL_ON_CONTROL_SCREEN;
    msgIdListRequest.msgIdArray[0]->msgIdData = NULL;
    //中控屏和仪表息屏
    msgIdListRequest.msgIdArray[1] = (ADM_DIM_SOA_MSGID_T*)adm_malloc(sizeof(ADM_DIM_SOA_MSGID_T));
    msgIdListRequest.msgIdArray[1]->msgId = VEHICLE_CTRL_OFF_CONTROL_SCREEN;
    msgIdListRequest.msgIdArray[1]->msgIdData = NULL;

    //进入OTA模式
    //退出OTA模式
    //维持网络唤醒
    //停止网络唤醒
    //这四个原接口已存在adm_dim_host_execOtaTask
    //也可以使用新接口
    msgIdListRequest.msgIdArray[2] = (ADM_DIM_SOA_MSGID_T*)adm_malloc(sizeof(ADM_DIM_SOA_MSGID_T));
    msgIdListRequest.msgIdArray[2]->msgId = VEHICLE_CTRL_ENTER_NORMAL_OTA_MODE;
    msgIdListRequest.msgIdArray[2]->msgIdData = NULL;

    msgIdListRequest.msgIdArray[3] = (ADM_DIM_SOA_MSGID_T*)adm_malloc(sizeof(ADM_DIM_SOA_MSGID_T));
    msgIdListRequest.msgIdArray[3]->msgId = VEHICLE_CTRL_EXIT_NORMAL_OTA_MODE;
    msgIdListRequest.msgIdArray[3]->msgIdData = NULL;

    msgIdListRequest.msgIdArray[4] = (ADM_DIM_SOA_MSGID_T*)adm_malloc(sizeof(ADM_DIM_SOA_MSGID_T));
    msgIdListRequest.msgIdArray[4]->msgId = VEHICLE_CTRL_ENTER_AWAKE;
    msgIdListRequest.msgIdArray[4]->msgIdData = NULL;

    msgIdListRequest.msgIdArray[5] = (ADM_DIM_SOA_MSGID_T*)adm_malloc(sizeof(ADM_DIM_SOA_MSGID_T));
    msgIdListRequest.msgIdArray[5]->msgId = VEHICLE_CTRL_ENTER_SLEEP;
    msgIdListRequest.msgIdArray[5]->msgIdData = NULL;

    //进入高压状态
    msgIdListRequest.msgIdArray[6] = (ADM_DIM_SOA_MSGID_T*)adm_malloc(sizeof(ADM_DIM_SOA_MSGID_T));
    msgIdListRequest.msgIdArray[6]->msgId = VEHICLE_CTRL_ENTER_HV_MODE;
    msgIdListRequest.msgIdArray[6]->msgIdData = NULL;
    //退出高压状态
    msgIdListRequest.msgIdArray[7] = (ADM_DIM_SOA_MSGID_T*)adm_malloc(sizeof(ADM_DIM_SOA_MSGID_T));
    msgIdListRequest.msgIdArray[7]->msgId = VEHICLE_CTRL_EXIT_HV_MODE;
    msgIdListRequest.msgIdArray[7]->msgIdData = NULL;
    //进入霸屏模式
    msgIdListRequest.msgIdArray[8] = (ADM_DIM_SOA_MSGID_T*)adm_malloc(sizeof(ADM_DIM_SOA_MSGID_T));
    msgIdListRequest.msgIdArray[8]->msgId = VEHICLE_CTRL_ENTER_OVERSCREEN;
    msgIdListRequest.msgIdArray[8]->msgIdData = NULL;
    //退出霸屏模式
    msgIdListRequest.msgIdArray[9] = (ADM_DIM_SOA_MSGID_T*)adm_malloc(sizeof(ADM_DIM_SOA_MSGID_T));
    msgIdListRequest.msgIdArray[9]->msgId = VEHICLE_CTRL_EXIT_OVERSCREEN;
    msgIdListRequest.msgIdArray[9]->msgIdData = NULL;

    //请求车机断开wifi
    msgIdListRequest.msgIdArray[10] = (ADM_DIM_SOA_MSGID_T*)adm_malloc(sizeof(ADM_DIM_SOA_MSGID_T));
    msgIdListRequest.msgIdArray[10]->msgId = VEHICLE_CTRL_SET_WIFI_DISLINK;
    msgIdListRequest.msgIdArray[10]->msgIdData = NULL;

    D32S errorCode;
    D32S retVal = adm_dim_host_setVehicleStatus(adm_soa_getHostInstanceId(), 5, &msgIdListRequest, &errorCode);
    log_i("errorCode = %u", errorCode);

    if(retVal != ADM_ECODE_SUCCESSED)
    {
        log_w("call adm_dim_host_setVehicleStatus fail");

        log_i("adm_dim_host_setVehicleStatus end");
        return ADM_ECODE_FAILED;
    }

    for(D16U index = 0; index < msgIdListRequest.msgIdArrayLength; ++index)
    {
        msgIdListRequest.msgIdArray[index];
    }

    log_i("adm_dim_host_setVehicleStatus_Test end");

    return ADM_ECODE_SUCCESSED;
}

static D32S request_dim_host_readPartInfo(void)
{
    D32S retCode = 0;
    D32S errorCode = 0;
    ADM_DIM_SOA_DID_DATA_ARRAY_T* didListResponse = NULL;
    ADM_DIM_SOA_ECU_IDEN_INFO_T pstEcuIdenInfo = {0};
    pstEcuIdenInfo.requestId = ADM_CUSTOM_ECU_TYPE_SOCMCU;

    ADM_DIM_SOA_DID_DATA_ARRAY_T didArrayRequest = {0};
    didArrayRequest.didArrayLength = 5;

    didArrayRequest.didArray =
        (ADM_DIM_SOA_DID_DATA_T**)adm_malloc(sizeof(ADM_DIM_SOA_DID_DATA_T*) * didArrayRequest.didArrayLength);
    didArrayRequest.didArray[0] = (ADM_DIM_SOA_DID_DATA_T*)adm_malloc(sizeof(ADM_DIM_SOA_DID_DATA_T));
    didArrayRequest.didArray[0]->did = 0xF187;
    didArrayRequest.didArray[0]->startByte = 0;
    didArrayRequest.didArray[0]->endByte = 13;
    didArrayRequest.didArray[0]->didData = (ADM_DIM_SOA_DATA_T*)adm_malloc(sizeof(ADM_DIM_SOA_DATA_T));
    didArrayRequest.didArray[0]->didData->dataLength = 14;
    didArrayRequest.didArray[0]->didData->dataType = VAR_STRING;

    didArrayRequest.didArray[1] = (ADM_DIM_SOA_DID_DATA_T*)adm_malloc(sizeof(ADM_DIM_SOA_DID_DATA_T));
    didArrayRequest.didArray[1]->did = 0xF189;
    didArrayRequest.didArray[1]->startByte = 0;
    didArrayRequest.didArray[1]->endByte = 13;
    didArrayRequest.didArray[1]->didData = (ADM_DIM_SOA_DATA_T*)adm_malloc(sizeof(ADM_DIM_SOA_DATA_T));
    didArrayRequest.didArray[1]->didData->dataLength = 14;
    didArrayRequest.didArray[1]->didData->dataType = VAR_STRING;

    didArrayRequest.didArray[2] = (ADM_DIM_SOA_DID_DATA_T*)adm_malloc(sizeof(ADM_DIM_SOA_DID_DATA_T));
    didArrayRequest.didArray[2]->did = 0xF089;
    didArrayRequest.didArray[2]->startByte = 0;
    didArrayRequest.didArray[2]->endByte = 13;
    didArrayRequest.didArray[2]->didData = (ADM_DIM_SOA_DATA_T*)adm_malloc(sizeof(ADM_DIM_SOA_DATA_T));
    didArrayRequest.didArray[2]->didData->dataLength = 14;
    didArrayRequest.didArray[2]->didData->dataType = VAR_STRING;

    didArrayRequest.didArray[3] = (ADM_DIM_SOA_DID_DATA_T*)adm_malloc(sizeof(ADM_DIM_SOA_DID_DATA_T));
    didArrayRequest.didArray[3]->did = 0xF18C;
    didArrayRequest.didArray[3]->startByte = 0;
    didArrayRequest.didArray[3]->endByte = 13;
    didArrayRequest.didArray[3]->didData = (ADM_DIM_SOA_DATA_T*)adm_malloc(sizeof(ADM_DIM_SOA_DATA_T));
    didArrayRequest.didArray[3]->didData->dataLength = 14;
    didArrayRequest.didArray[3]->didData->dataType = VAR_STRING;

    didArrayRequest.didArray[4] = (ADM_DIM_SOA_DID_DATA_T*)adm_malloc(sizeof(ADM_DIM_SOA_DID_DATA_T));
    didArrayRequest.didArray[4]->did = 0xF013;
    didArrayRequest.didArray[4]->startByte = 0;
    didArrayRequest.didArray[4]->endByte = 13;
    didArrayRequest.didArray[4]->didData = (ADM_DIM_SOA_DATA_T*)adm_malloc(sizeof(ADM_DIM_SOA_DATA_T));
    didArrayRequest.didArray[4]->didData->dataLength = 14;
    didArrayRequest.didArray[4]->didData->dataType = VAR_STRING;

    //走ucl代码
    retCode = adm_dim_ucl_readPartInformation(adm_soa_getUclInstanceId(), TIME_WAIT,
              &pstEcuIdenInfo, &didArrayRequest, &didListResponse, &errorCode);
    log_i("ret code: %d\n", retCode);

    if(retCode == 0)
    {
        log_e("dim get sysi resp,readPartInformation succeed");
    }
    else
    {
        log_e("dim not get sysi resp,readPartInformation failed");
    }

    for(D16U index = 0; index < didArrayRequest.didArrayLength; ++index)
    {
        if((NULL != didArrayRequest.didArray) && (NULL != didArrayRequest.didArray[index]))
        {
            adm_free(didArrayRequest.didArray[index]->didData);
            adm_free(didArrayRequest.didArray[index]);
        }
    }

    adm_free(didArrayRequest.didArray);
    return retCode;
}

static D32S request_dim_host_wifiLink(void)
{
    D32S retCode = 0;

    D32U uiArrLen = 3;

    ADM_HOST_WIFI_INFO_ARRAY_T* pstWifiArr = (ADM_HOST_WIFI_INFO_ARRAY_T*)adm_malloc(sizeof(ADM_HOST_WIFI_INFO_ARRAY_T) +
            sizeof(ADM_HOST_WIFI_INFO_T) * uiArrLen);
    pstWifiArr->uArrLen = uiArrLen;

    pstWifiArr->pWifiList[0].pSsid = adm_strdup("wifi1_name1");
    pstWifiArr->pWifiList[0].pPassword = adm_strdup("wifi1_password1");

    pstWifiArr->pWifiList[1].pSsid = adm_strdup("wifi2_name2");
    pstWifiArr->pWifiList[1].pPassword = adm_strdup("wifi2_password2");

    pstWifiArr->pWifiList[2].pSsid = adm_strdup("wifi3_name3");
    pstWifiArr->pWifiList[2].pPassword = adm_strdup("wifi3_password3");

    D32S errorCode = 0;
    retCode = adm_dim_host_setWifiLink(adm_soa_getHostInstanceId(), TIME_WAIT, pstWifiArr, &errorCode);
    log_i("ret code=%d, errorCode=%d", retCode, errorCode);

    for(int i = 0; i < pstWifiArr->uArrLen; ++i)
    {
        adm_free(pstWifiArr->pWifiList[i].pSsid);
        adm_free(pstWifiArr->pWifiList[i].pPassword);
    }

    adm_free(pstWifiArr);
}

static D32S req_dim_host_getApnName(void)
{
    D32S errorCode = 0;
    ADM_DIM_SOA_MSGID_DATA_ARRAY_T* msgIdListResponse = NULL;
    ADM_DIM_SOA_MSGID_DATA_ARRAY_T msgIdListRequest = {0};
    msgIdListRequest.msgIdArrayLength = 1;
    msgIdListRequest.msgIdArray = adm_malloc(sizeof(ADM_DIM_SOA_MSGID_T*) * msgIdListRequest.msgIdArrayLength);
    msgIdListRequest.msgIdArray[0] = adm_malloc(sizeof(ADM_DIM_SOA_MSGID_T));
    msgIdListRequest.msgIdArray[0]->msgId = VEHICLE_STATUS_APN_NAME;
    msgIdListRequest.msgIdArray[0]->msgIdData = adm_malloc(sizeof(ADM_DIM_SOA_DATA_T));
    msgIdListRequest.msgIdArray[0]->msgIdData->dataType = VAR_STRING;
    msgIdListRequest.msgIdArray[0]->msgIdData->dataLength = 20;

    D32S retVal = adm_dim_host_readVehicleStatus(adm_soa_getHostInstanceId(), 5,
                  &msgIdListRequest, &msgIdListResponse, &errorCode);

    log_i("resp errorcode = %d", errorCode);

    for(int index = 0; index < msgIdListRequest.msgIdArrayLength; ++index)
    {
        adm_free(msgIdListRequest.msgIdArray[index]->msgIdData);
        adm_free(msgIdListRequest.msgIdArray[index]);
    }

    adm_free(msgIdListRequest.msgIdArray);

    if(retVal != ADM_ECODE_SUCCESSED)
    {
        log_w("call adm_dim_host_readVehicleStatus fail,retVal = %d", retVal);
    }

    if(msgIdListResponse)
    {
        for(int index = 0; index < msgIdListResponse->msgIdArrayLength; ++index)
        {
            if(VAR_S32 == msgIdListResponse->msgIdArray[index]->msgIdData->dataType)
            {
                log_i("resp data:msgId[%#x],value=%d", msgIdListResponse->msgIdArray[index]->msgId,
                      msgIdListResponse->msgIdArray[index]->msgIdData->v.valueS32);
            }
            else if(VAR_STRING == msgIdListResponse->msgIdArray[index]->msgIdData->dataType)
            {
                log_i("resp data:msgId[%#x],value=%s", msgIdListResponse->msgIdArray[index]->msgId,
                      msgIdListResponse->msgIdArray[index]->msgIdData->v.valueS);
            }

            adm_free(msgIdListResponse->msgIdArray[index]->msgIdData);
            adm_free(msgIdListResponse->msgIdArray[index]);
        }

        adm_free(msgIdListResponse->msgIdArray);
        adm_free(msgIdListResponse);
    }
}

D32S adm_host_simulatorInit()
{
    log_i("adm_host_simulatorInit begin");

    ADM_DIM_SOA_HOST_INIT_INFO_T initInfo;
    initInfo.signalEventNotifyCallback = signalEventNotify;
    initInfo.triggerEventNotifyCallback = triggerEventNotify;

    void* priv = NULL;
    adm_dim_host_init(&initInfo, priv);

    log_i("adm_host_simulatorInit end");

    return 0;
}

D32S adm_host_simulatorExec(void)
{
    log_i("adm_host_simulatorExec begin");

    //request_dim_host_setVehicleStatus_Test();
    //request_dim_host_readVehicleStatus_Test();
    //request_dim_host_wifiLink();
    //request_dim_host_readPartInfo();
    //getSchduleWakeupTimeTest();
    //setSchduleWakeupTimeTest();
    //req_dim_host_getApnName();
    req_dim_getVehicleVer();
    req_dim_setVehicleVer();
#if 0
    request_dim_host_readVehicleStatus_Test();

    getVinTest();

    execOtaTaskTest();

    setSchduleWakeupTimeTest();

    setVehicleBigversionTest();

    getVehicleBigversionTest();

    verifySpaceisenoughTest();

    verifySignatureTest();

    getCertabsolutepathTest();

    setDtcTest();

    clearDtcTest();

    request_dim_host_set_log_level();

    request_dim_host_GetSdkVersion();

    request_dim_host_GetNetworkType();

    log_i("adm_host_simulatorExec end");
#endif
    return 0;
}

D32S adm_host_simulatorDeinit(void)
{
    log_i("adm_host_simulatorDeinit begin");

    adm_dim_host_deinit();

    log_i("adm_host_simulatorDeinit begin");

    return 0;
}



