#include "adm_cvs_baseInteraction.h"
#if !defined(LOG_TAG)
//#define LOG_TAG             MOD_NAME(ADM_COM_MOD_CVM)         /*!< 宏函数，用于通过模块名的枚举值类型获取模块名称 */
#    define LOG_TAG "CVMSAMPLE"
#endif

#if !defined(MOD_TAG_ID)
#    define MOD_TAG_ID ADM_COM_MOD_CVM
#endif

#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include "adm_com_modDefine.h"
#include "adm_debug.h"
#include "adm_com_errorCode.h"
#include "adm_engine_common.h"
#include "adm_com_lib.h"
#include "adm_string.h"
#include "adm_memory.h"
#include "adm_ocs_airProtocolAdapter.h"
#include "adm_oca_connect.h"
#include "adm_cfg_api.h"
#include "adm_ocm_interface.h"
#include "adm_cvs_license.h"

static D32S perCheckCom(const ADM_OCS_BUSINESS_TYPE_E eBusinessType,
    const DCHAR* reqMessageBody,
    DCHAR** rspMessageBody,
    DSIZE* rspStatusCode)
{
    log_i("eBusinessType = %d; reqMessageBody = %s", eBusinessType, reqMessageBody);
    *rspMessageBody = adm_strdup(
        "{\
        \"status\": 7000,\
        \"msg\": \"exts\",\
        \"sign\": \"89f70c65fccf5b083738b62d163201be9137ee41679ebebab9325c6e131fb233\",\
        \"timestamp\": 1626056371,\
        \"data\": {\
            \"deviceId\": \"e0984588053b2873c6c5aebdac6840bd8b66a83ac83db0911846dfe13d9e39c9\",\
            \"secret\": \"0d5fdcf89147040a5cbc5257c2b89529\"\
        }\
    }");
    *rspStatusCode = 0;
    return 0;
}

int main()
{

    ADM_CFG_API_INFO_T pstApiData = {0};
    pstApiData.isFileExistCb = adm_com_isFileExist;
    pstApiData.writeFileDataCb = adm_com_writeFileData;
    D32S iRet = adm_cfg_iniParserInit("/home/jibijun/project/vusproj/rawdata/etc/ucmconfig.ini", &pstApiData);
    adm_core_service_initRuntimeDir(
        ADM_CORE_SERVICE_RUNTIME_DIR_TYPE_KV_STORAGE, "/home/jibijun/project/tmp/admota/ucm/kvstorage/");

    const DCHAR* dirStr
        = adm_cfg_iniParserGetString(ADM_CFG_VUS_LOG_STORAGE_DIR, ADM_CFG_VUS_LOG_STORAGE_DIR_DEFAULT_VAL);
    iRet = adm_com_createDir(dirStr);
    adm_debug_logInit(dirStr, (1 * 1024 * 1024), 5);
    adm_core_service_init();

    DCHAR* sdkVersion = "v1.2.0";
    DCHAR* extsObjectStr
        = "{\
        \"status\": 1000,\
        \"msg\": \"exts\",\
        \"sign\": \"89f70c65fccf5b083738b62d163201be9137ee41679ebebab9325c6e131fb233\",\
        \"timestamp\": 1626056371,\
        \"data\": {\
            \"deviceId\": \"e0984588053b2873c6c5aebdac6840bd8b66a83ac83db0911846dfe13d9e39c9\",\
            \"secret\": \"0d5fdcf89147040a5cbc5257c2b89529\"\
        }\
    }";

    D64S timestamp = time(NULL);
    /*per check*/
    ADM_OCS_BASE_RESP_ADAPTER_T* baseResp = adm_ocs_newBaseRespAdapterBody();
    iRet = adm_cvs_preCheck(timestamp, sdkVersion, extsObjectStr, perCheckCom, baseResp);
    if (ADM_ECODE_SUCCESSED != iRet) {
        log_i("adm_cvs_preCheck failed, iRet:%d", iRet);
    } else {
        log_i("adm_cvs_preCheck successed");
        log_i("baseResp->status = %d, baseResp->respDataStr = %s,baseResp->extsObjectStr = %s",
            baseResp->status,
            baseResp->respDataStr,
            baseResp->extsObjectStr);
    }
    adm_ocs_deleteBaseRespAdapterBody(baseResp);
    /*get upgrade history*/
    baseResp = adm_ocs_newBaseRespAdapterBody();
    iRet = adm_cvs_getUpgradeHistory(timestamp, sdkVersion, extsObjectStr, perCheckCom, baseResp);
    if (ADM_ECODE_SUCCESSED != iRet) {
        log_i("adm_cvs_getUpgradeHistory failed, iRet:%d", iRet);
    } else {
        log_i("adm_cvs_getUpgradeHistory successed");
        log_i("baseResp->status = %d, baseResp->respDataStr = %s,baseResp->extsObjectStr = %s",
            baseResp->status,
            baseResp->respDataStr,
            baseResp->extsObjectStr);
    }
    adm_ocs_deleteBaseRespAdapterBody(baseResp);

    DCHAR* respLicenseObjectStr = NULL;
    D32S status;
    iRet = adm_cvs_licenseCheck(NULL, timestamp, sdkVersion, perCheckCom, &status, &respLicenseObjectStr);
    if (ADM_ECODE_SUCCESSED != iRet) {
        log_i("adm_cvs_licenseCheck failed, iRet:%d", iRet);
    } else {
        log_i("adm_cvs_licenseCheck successed");
        log_i("status = %d, respLicenseObjectStr = %s", status, respLicenseObjectStr);
    }

    iRet = adm_cvs_licenseStateReport(NULL,
        timestamp,
        sdkVersion,
        BRUSH_COMPLETED_STATE,
        DIGITAL_SIGNATURE_INVALID_REASON,
        extsObjectStr,
        perCheckCom,
        &status,
        &respLicenseObjectStr);
    if (ADM_ECODE_SUCCESSED != iRet) {
        log_i("adm_cvs_licenseStateReport failed, iRet:%d", iRet);
    } else {
        log_i("adm_cvs_licenseStateReport successed");
        log_i("status = %d, respLicenseObjectStr = %s", status, respLicenseObjectStr);
    }

    adm_deinitLog();
    adm_core_service_deinit();

    while (1) {
        sleep(3);
    }
    return 0;
}