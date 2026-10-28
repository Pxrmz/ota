
#if !defined(LOG_TAG)
//#define LOG_TAG             MOD_NAME(ADM_COM_MOD_CVM)         /*!< 宏函数，用于通过模块名的枚举值类型获取模块名称 */
#    define LOG_TAG "CVMSAMPLE"
#endif

#if !defined(MOD_TAG_ID)
#    define MOD_TAG_ID ADM_COM_MOD_CVM
#endif

#include <unistd.h>
#include <stdio.h>
#include "adm_cvm_checkVersion.h"
#include "adm_com_modDefine.h"
#include "adm_debug.h"
#include "adm_com_errorCode.h"
#include "adm_engine_common.h"
#include "adm_irs_upload.h"
#include "adm_com_lib.h"
#include "adm_string.h"
#include "adm_memory.h"
#include "adm_soa.h"
#include "adm_dim_vdi.h"
#include "adm_dim_host.h"
#include "adm_cvs_versionPolicy.h"
#include "adm_ocs_airProtocolAdapter.h"
#include "adm_oca_connect.h"
#include "adm_cfg_api.h"
#include "adm_uim_check.h"
#include "adm_cvs_common.h"
#include "adm_irm_report.h"
#include "adm_ocm_interface.h"
#include "adm_eim_getEcuInfo.h"

static void checkProgressNotify(const ADM_CVS_COMMON_CMDID_TYPE_E eCmdidTYpe,
    const DCHAR* state,
    const ADM_CVS_COMMON_CHECK_PROGRESS_TYPE_T eCheckProgressType)
{
    return;
}

static void checkProgressUpload(const DCHAR* state, const DCHAR* reason)
{
    return;
}

static D32S ADM_DIM_SOA_HOST_SIGNAL_EVENT_NOTIFY_CALLBACK(const ADM_DIM_SOA_HOST_SIGNAL_EVENT_TYPE_E eSignalEventType,
    void* priv)
{
    return 0;
}

static D32S ADM_DIM_SOA_HOST_TRIGGER_EVENT_NOTIFY_CALLBACK(
    const ADM_DIM_SOA_HOST_TRIGGER_EVENT_TYPE_E eTriggerEventType,
    void* priv)
{
    return 0;
}

/**
 * @brief 车云通信接口初始化
 *
 * @return D32S     0:Successed, None-0:Failed
 */
D32S adm_apc_otaConnectInit(void)
{
    D32S iRet = ADM_ECODE_FAILED;
    DBOOL bRet = TRUE;
#if 0
    do {
        if (FALSE == adm_oca_registerSetConnectProtocolTypeHandler(adm_oca_setConnectProtocolType)) {
            log_e("adm_oca_registerSetConnectProtocolTypeHandler failed");
            bRet = FALSE;
            break;
        }
        if (FALSE == adm_oca_registerSetConnectCertPathHandler(adm_oca_setConnectCertPath)) {
            log_e("adm_oca_registerSetConnectCertPathHandler failed");
            bRet = FALSE;
            break;
        }
        if (FALSE == adm_oca_registerSetConnectAddressHandler(adm_oca_setConnectAddress)) {
            log_e("adm_oca_registerSetConnectAddressHandler failed");
            bRet = FALSE;
            break;
        }
        if (FALSE == adm_oca_registerSetMqttConnectApiNameHandler(adm_oca_setMqttConnectApiName)) {
            log_e("adm_oca_registerSetMqttConnectApiNameHandler failed");
            bRet = FALSE;
            break;
        }
        if (FALSE == adm_oca_registerSetMqttConnectTopicNameHandler(adm_oca_setMqttConnectTopicName)) {
            log_e("adm_oca_registerSetMqttConnectTopicTypeHandler failed");
            bRet = FALSE;
            break;
        }
        if (FALSE == adm_oca_registerSetConnectOutputApnHandler(adm_oca_setConnectOutputApn)) {
            log_e("adm_oca_registerSetConnectOutputApnHandler failed");
            bRet = FALSE;
            break;
        }
        if (FALSE == adm_oca_registerRequestConnectServerHandler(adm_oca_requestConnectServer)) {
            log_e("adm_oca_registerRequestConnectServerHandler failed");
            bRet = FALSE;
            break;
        }
        if (FALSE == adm_oca_registerConnectInitHandler(adm_oca_connectInit)) {
            log_e("adm_oca_registerConnectInitHandler failed");
            bRet = FALSE;
            break;
        }
        if (FALSE == adm_oca_registerConnectDeinitHandler(adm_oca_connectDeinit)) {
            log_e("adm_oca_registerConnectDeinitHandler failed");
            bRet = FALSE;
            break;
        }

        if (FALSE == adm_oca_registerPauseDownloadHandler(adm_oca_pauseDownload)) {
            log_e("adm_oca_registerPauseDownloadHandler failed");
            bRet = FALSE;
            break;
        }
        if (FALSE == adm_oca_registerCancelDownloadHandler(adm_oca_cancelDownload)) {
            log_e("adm_oca_registerCancelDownloadHandler failed");
            bRet = FALSE;
            break;
        }
        if (FALSE == adm_oca_registerStartDownloadHandler(adm_oca_startDownload)) {
            log_e("adm_oca_registerStartDownloadHandler failed");
            bRet = FALSE;
            break;
        }
        if (FALSE == adm_oca_registerNewDownloadInfoHandler(adm_oca_newDownloadInfo)) {
            log_e("adm_oca_registerNewDownloadInfoHandler failed");
            bRet = FALSE;
            break;
        }
        if (FALSE == adm_oca_registerDelDownloadInfoHandler(adm_oca_delDownloadInfo)) {
            log_e("adm_oca_registerDelDownloadInfoHandler failed");
            bRet = FALSE;
            break;
        }
        if (FALSE == adm_oca_registerFileUploadInitHandler(adm_oca_fileUploadInit)) {
            log_e("adm_oca_registerFileUploadInitHandler failed");
            bRet = FALSE;
            break;
        }
        if (FALSE == adm_oca_registerFileUploadDeinitHandler(adm_oca_fileUploadDeinit)) {
            log_e("adm_oca_registerFileUploadDeinitHandler failed");
            bRet = FALSE;
            break;
        }
        if (FALSE == adm_oca_registerAddSectionToHttpPostHandler(adm_oca_addSectionToHttpPost)) {
            log_e("adm_oca_registerAddSectionToHttpPostHandler failed");
            bRet = FALSE;
            break;
        }
        if (FALSE == adm_oca_registerNewUploadInfoHandler(adm_oca_newUploadInfo)) {
            log_e("adm_oca_registerNewUploadInfoHandler failed");
            bRet = FALSE;
            break;
        }
        if (FALSE == adm_oca_registerDelUploadInfoHandler(adm_oca_delUploadInfo)) {
            log_e("adm_oca_registerDelUploadInfoHandler failed");
            bRet = FALSE;
            break;
        }
        if (FALSE == adm_oca_registerStartUploadHandler(adm_oca_startUpload)) {
            log_e("adm_oca_registerStartUploadHandler failed");
            bRet = FALSE;
            break;
        }
    } while(0);

    if (TRUE == bRet) {
        iRet = ADM_ECODE_SUCCESSED;
        log_i("adm_apc_otaConnectInit successed");
    } else {
        log_e("adm_apc_otaConnectInit failed");
    }
#endif
    return iRet;
}

int main(int argc, char** argv)
{
    ADM_OCS_CHECK_RESP_ADAPTER_T* resp = NULL;
    D32S retVal;
    void* priv = NULL;
    ADM_SOA_SESSION_T* soaSession = NULL;
    ADM_CVS_NOTIFY_T* notifyF = NULL;
    const DCHAR* soaUnicastIp = NULL;
    const DCHAR* dirStr = NULL;
    ADM_UIM_SOA_CHECK_HMI_POLICY_INFO_T* ppstPolicyInfo = NULL;

    ADM_CFG_API_INFO_T pstApiData = {0};
    pstApiData.isFileExistCb = adm_com_isFileExist;
    pstApiData.writeFileDataCb = adm_com_writeFileData;
    D32S iRet = adm_cfg_iniParserInit("./otaconfig.ini", &pstApiData);
    if (ADM_ECODE_SUCCESSED != iRet) {
        // log_i("adm_cfg_iniParserInit %s failed, iRet:%d\n", filename, iRet);
    }
    ADM_CVS_VERSION_POLICY_T* versionPolicy = adm_cvs_newPolicyBody();
    adm_core_service_initRuntimeDir(
        ADM_CORE_SERVICE_RUNTIME_DIR_TYPE_KV_STORAGE, "/home/jibijun/project/tmp/admfota/kvstorage/");

    dirStr = adm_cfg_iniParserGetString(ADM_CFG_VUS_LOG_STORAGE_DIR, ADM_CFG_VUS_LOG_STORAGE_DIR_DEFAULT_VAL);
    iRet = adm_com_createDir(dirStr);
    if (ADM_ECODE_SUCCESSED != iRet) {
        log_i("adm_com_createDir %s failed, iRet:%d\n", dirStr, iRet);
    } else {
        log_i("adm_com_createDir %s successed\n", dirStr);
    }

    adm_debug_logInit(dirStr, (1 * 1024 * 1024), 5);
    adm_core_service_init();
    /*初始化IRM*/
    iRet = adm_irm_init(adm_ocm_businessCommInterface, adm_ocm_fileUploadInterface, NULL);
    if (ADM_ECODE_SUCCESSED != iRet) {
        log_e("adm_irm_init failed, iRet:%d", iRet);
    } else {
        log_i("adm_irm_init successed");
    }
    DCHAR* ecuInfoStr
        = "[ { \"ecuReqId\": \"0X16E2\", \
                            \"ecuSwDid\": \"0XD001\", \
                            \"ecuDids\": [ { \"did\": \"0XF187\", \"didValue\": \"0X01\", \"start\": 0, \"end\": 13 },\
                            { \"did\": \"0XF188\", \"didValue\": \"V2.0.2\", \"start\": 0, \"end\": 13 }, \
                            { \"did\": \"0XF18A\", \"didValue\": \"CONTI2021\", \"start\": 0, \"end\": 13 }, \
                            { \"did\": \"0XF18B\", \"didValue\": \"20210622\", \"start\": 0, \"end\": 13 }, \
                            { \"did\": \"0XF18C\", \"didValue\": \"MX06221375AB\", \"start\": 0, \"end\": 13 }, \
                            { \"did\": \"0XF193\", \"didValue\": \"V1.0.0\", \"start\": 0, \"end\": 13 } ] } ]";

    ADM_EIM_GETECUINFO_T* allEcuInfo = adm_eim_ecuInfoJsonStrParse(ecuInfoStr);
    adm_eim_deleteGetEcuInfoBody(allEcuInfo);

    // ADM_DIM_SOA_DID_DATA_ARRAY_T* SingleEcuInDidList = NULL;
    // if (ADM_ECODE_SUCCESSED != adm_eim_getSingleEcuInfoFromKv("0X16E2", "0XD001", &SingleEcuInDidList, ecuInfoStr)) {
    //     log_e("adm_eim_getSingleEcuInfoFromKv failed,");
    // } else {
    //     for (D16U i = 0; i < SingleEcuInDidList->didArrayLength; ++i) {
    //         if (NULL != SingleEcuInDidList->didArray + i && NULL != SingleEcuInDidList->didArray[i]->didData) {
    //             if (NULL != SingleEcuInDidList->didArray[i]->didData->v.valueS) {
    //                 log_i("did = %0x", SingleEcuInDidList->didArray[i]->did);
    //                 log_i("didValue = %s", SingleEcuInDidList->didArray[i]->didData->v.valueS);
    //                 log_i("start = %d", SingleEcuInDidList->didArray[i]->startByte);
    //                 log_i("end = %d", SingleEcuInDidList->didArray[i]->endByte);
    //                 adm_free(SingleEcuInDidList->didArray[i]->didData->v.valueS);
    //             }
    //             adm_free(SingleEcuInDidList->didArray[i]->didData);
    //         }
    //     }
    //     adm_free(SingleEcuInDidList->didArray);
    //     adm_free(SingleEcuInDidList);
    // }
    // adm_apc_otaConnectInit();

    // ADM_DIM_SOA_HOST_INIT_INFO_T* hostInitInfo = NULL;

    // hostInitInfo = (ADM_DIM_SOA_HOST_INIT_INFO_T*)adm_malloc(sizeof(ADM_DIM_SOA_HOST_INIT_INFO_T));

    // hostInitInfo->signalEventNotifyCallback = ADM_DIM_SOA_HOST_SIGNAL_EVENT_NOTIFY_CALLBACK;
    // hostInitInfo->triggerEventNotifyCallback = ADM_DIM_SOA_HOST_TRIGGER_EVENT_NOTIFY_CALLBACK;

    // soaUnicastIp
    //     = adm_cfg_iniParserGetString(ADM_CFG_VRPC_SOMEIP_UNICAST_IP, ADM_CFG_VRPC_SOMEIP_UNICAST_IP_DEFAULT_VAL);
    // soaSession = adm_soa_init(soaUnicastIp);
    // adm_dim_soa_vdi_init(soaSession, NULL, priv);
    // adm_dim_soa_host_init(soaSession, NULL, priv);

    retVal = adm_cvm_checkVersionAdaptorNotify(ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_USER, NULL, NULL, &resp);
    log_i("retVal = %d", retVal);
    if (ADM_ECODE_SUCCESSED == retVal) {
        log_i("check success");

        log_i("hmiFilePath = %s", resp->hmiMember->hmiShowFileUrl);
        log_i("hmiFileHash = %s", resp->hmiMember->hmiShowFileHash);
        log_i("hmiFileSize = %d", resp->hmiMember->hmiShowFileSize);
        log_i("hmiFileName = %s", resp->hmiMember->hmiShowFileName);

        ppstPolicyInfo = (ADM_UIM_SOA_CHECK_HMI_POLICY_INFO_T*)adm_malloc(sizeof(ADM_UIM_SOA_CHECK_HMI_POLICY_INFO_T));
        adm_memset(ppstPolicyInfo,
            0x00,
            sizeof(ADM_UIM_SOA_CHECK_HMI_POLICY_INFO_T),
            sizeof(ADM_UIM_SOA_CHECK_HMI_POLICY_INFO_T));

        ppstPolicyInfo->hmiFile = (ADM_UIM_HMI_FILE_T*)adm_malloc(sizeof(ADM_UIM_HMI_FILE_T));
        ppstPolicyInfo->hmiFile->policyFileUrl = adm_strdup(resp->hmiMember->hmiShowFileUrl);
        ppstPolicyInfo->hmiFile->policyFileHash = adm_strdup(resp->hmiMember->hmiShowFileHash);
        ppstPolicyInfo->hmiFile->policyFileSize = resp->hmiMember->hmiShowFileSize;

        ppstPolicyInfo->upgradeMode = ADM_UIM_UPGRADE_MODE_NORMAL_DOWN_NORMAL_INS;

        ppstPolicyInfo->upgradeMode = ADM_UIM_UPGRADE_MODE_NORMAL_DOWN_NORMAL_INS;

        ppstPolicyInfo->hmiInfo = (ADM_UIM_HMI_INFO*)adm_malloc(sizeof(ADM_UIM_HMI_INFO));

        ppstPolicyInfo->hmiInfo->ecuInfoList = (ADM_UIM_ECU_INFO_ARRAY_T*)adm_malloc(sizeof(ADM_UIM_ECU_INFO_ARRAY_T));
        ppstPolicyInfo->hmiInfo->ecuInfoList->hmiEcuInfoArrayLength = resp->hmiMember->hmiEcuInfoArrayLength;

        ppstPolicyInfo->hmiInfo->vehicleVersion = adm_strdup(resp->hmiMember->vehicleVersion);
        ppstPolicyInfo->hmiInfo->totalSize = resp->hmiMember->totalSize;

        ppstPolicyInfo->hmiInfo->ecuInfoList->hmiEcuInfoArray = (ADM_UIM_ECU_INFO_T**)adm_malloc(
            sizeof(ADM_UIM_ECU_INFO_T*) * ppstPolicyInfo->hmiInfo->ecuInfoList->hmiEcuInfoArrayLength);

        log_i("vehicleVersion = %s", ppstPolicyInfo->hmiInfo->vehicleVersion);
        log_i("totalSize = %d", ppstPolicyInfo->hmiInfo->totalSize);
        log_i("hmiEcuInfoArrayLength = %d", ppstPolicyInfo->hmiInfo->ecuInfoList->hmiEcuInfoArrayLength);

        for (D32S i = 0; i < ppstPolicyInfo->hmiInfo->ecuInfoList->hmiEcuInfoArrayLength; ++i) {

            ppstPolicyInfo->hmiInfo->ecuInfoList->hmiEcuInfoArray[i]
                = (ADM_UIM_ECU_INFO_T*)adm_malloc(sizeof(ADM_UIM_ECU_INFO_T));

            log_i("ecuSize[%d]  = %d", i, resp->hmiMember->hmiEcuInfoArray[i]->ecuSize);
            ppstPolicyInfo->hmiInfo->ecuInfoList->hmiEcuInfoArray[i]->ecuSize
                = resp->hmiMember->hmiEcuInfoArray[i]->ecuSize;

            log_i("ecuVersion[%d]  = %s", i, resp->hmiMember->hmiEcuInfoArray[i]->ecuVersion);
            ppstPolicyInfo->hmiInfo->ecuInfoList->hmiEcuInfoArray[i]->ecuVersion
                = adm_strdup(resp->hmiMember->hmiEcuInfoArray[i]->ecuVersion);

            log_i("requestId[%d]  = %s", i, resp->hmiMember->hmiEcuInfoArray[i]->requestId);
            ppstPolicyInfo->hmiInfo->ecuInfoList->hmiEcuInfoArray[i]->requestId
                = adm_strdup(resp->hmiMember->hmiEcuInfoArray[i]->requestId);

            log_i("swDid[%d]  = %s", i, resp->hmiMember->hmiEcuInfoArray[i]->swDid);
            ppstPolicyInfo->hmiInfo->ecuInfoList->hmiEcuInfoArray[i]->swDid
                = adm_strdup(resp->hmiMember->hmiEcuInfoArray[i]->swDid);
        }
        adm_cvs_getVersionPolicy(versionPolicy);
    }
    // adm_free(hostInitInfo);
    adm_cvs_deletePolicyBody(versionPolicy);
    adm_ocs_deleteCheckRespAdapterBody(resp);
    adm_deinitLog();
    adm_core_service_deinit();
    return 0;
}
