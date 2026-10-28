#include "adm_dim_log.h"
#include "adm_com_errorCode.h"
#include "adm_typedefs.h"
#include "adm_memory.h"
#include "adm_string.h"
#include "adm_com_lib.h"

#include <dlfcn.h>
#include "adm_dim_api.h"
#include "adm_dim_apiImpl.h"

#include "adm_cfg_api.h"
#include "adm_cfg_default.h"

#ifndef __DLSYM__
#include "adm_dis_dpo.h"
#include "adm_dis_host.h"
#include "adm_dis_transHost.h"
#include "adm_dis_ucl.h"
#include "adm_dis_vdi.h"
#endif

static ADM_DIM_IMPL_INFO_T* dim_impl_info = NULL;
static void* handle = NULL;
static dim_func_list_t* dim_func_list = NULL;

#ifdef __DLSYM__
#define DIM_DLSYM(a, x, t)  do { \
        char *error = NULL; \
        (a) = (t)dlsym(handle , #x); \
        if((error = dlerror()) != NULL) { \
            log_e("%s", error); \
        } \
    } while(TRUE == FALSE)
#else
#define DIM_DLSYM(a, x, t)  (a) = (t)(x);
#endif

static D32S init_func_list(void)
{
    if(dim_func_list == NULL)
    {
        dim_func_list = (dim_func_list_t*)adm_malloc(sizeof(dim_func_list_t));

        if(dim_func_list == NULL)
        {
            log_e("malloc init func list fail");
            return ADM_ECODE_FAILED;
        }

        (void)adm_memset(dim_func_list, 0, sizeof(dim_func_list_t), sizeof(dim_func_list_t));
    }

    /*ucl*/
    DIM_DLSYM(dim_func_list->adm_dis_ucl_init_handle,
              adm_dis_ucl_init_impl, adm_dim_ucl_init_func);
    DIM_DLSYM(dim_func_list->adm_dis_ucl_deinit_handle,
              adm_dis_ucl_deinit_impl, adm_dim_ucl_deinit_func);
    DIM_DLSYM(dim_func_list->adm_dis_ucl_getSdkVersion_handle,
              adm_dis_ucl_getSdkVersion_impl, adm_dim_ucl_getSdkVersion_func);
    DIM_DLSYM(dim_func_list->adm_dis_ucl_readPartInformation_handle,
              adm_dis_ucl_readPartInformation_impl, adm_dim_ucl_readPartInformation_func);
    DIM_DLSYM(dim_func_list->adm_dis_ucl_readSnInfo_handle,
              adm_dis_ucl_readSnInfo_impl, adm_dim_ucl_readSnInfo_func);          
    DIM_DLSYM(dim_func_list->adm_dis_ucl_parallelCollectEcu_handle,
              adm_dis_ucl_parallelCollectEcu_impl, adm_dim_ucl_parallelCollectEcu_func);
    DIM_DLSYM(dim_func_list->adm_dis_ucl_requestDoUpgrade_handle,
              adm_dis_ucl_requestDoUpgrade_impl, adm_dim_ucl_requestDoUpgrade_func);
    DIM_DLSYM(dim_func_list->adm_dis_ucl_requestDoUpgrade_icc_handle,
              adm_dis_ucl_requestDoUpgrade_icc_impl, adm_dim_ucl_requestDoUpgrade_icc_func);
    DIM_DLSYM(dim_func_list->adm_dis_ucl_requestSetChangeSlot_icc_handle,
              adm_dis_ucl_requestSetChangeSlot_icc_impl, adm_dim_ucl_requestSetChangeSlot_icc_func);
    DIM_DLSYM(dim_func_list->adm_dis_ucl_requestUpdateAndReboot_handle,
              adm_dis_ucl_requestUpdateAndReboot_impl, adm_dim_ucl_requestUpdateAndReboot_func);
    DIM_DLSYM(dim_func_list->adm_dis_ucl_queryUpgradeProgress_handle,
              adm_dis_ucl_queryUpgradeProgress_impl, adm_dim_ucl_queryUpgradeProgress_func);
    DIM_DLSYM(dim_func_list->adm_dis_ucl_queryUpgradeResult_handle,
              adm_dis_ucl_queryUpgradeResult_impl, adm_dim_ucl_queryUpgradeResult_func);
    DIM_DLSYM(dim_func_list->adm_dis_ucl_requestDecryptPackage_handle,
              adm_dis_ucl_requestDecryptPackage_impl, adm_dim_ucl_requestDecryptPackage_func);
    DIM_DLSYM(dim_func_list->adm_dis_ucl_getDecryptPackageProgress_handle,
              adm_dis_ucl_getDecryptPackageProgress_impl, adm_dim_ucl_getDecryptPackageProgress_func);
    DIM_DLSYM(dim_func_list->adm_dis_ucl_getDecryptPackageResult_handle,
              adm_dis_ucl_getDecryptPackageResult_impl, adm_dim_ucl_getDecryptPackageResult_func);
    DIM_DLSYM(dim_func_list->adm_dis_ucl_requestVerifyHash_handle,
              adm_dis_ucl_requestVerifyHash_impl, adm_dim_ucl_requestVerifyHash_func);
    DIM_DLSYM(dim_func_list->adm_dis_ucl_verifySpaceIsEnouth_handle,
              adm_dis_ucl_verifySpaceIsEnouth_impl, adm_dim_ucl_verifySpaceIsEnouth_func);
    DIM_DLSYM(dim_func_list->adm_dis_ucl_request_Verify_Sign_handle,
              adm_dis_ucl_ReqVerify_Sign_impl, adm_dim_ucl_request_Verify_Sign_func);
    DIM_DLSYM(dim_func_list->adm_dis_ucl_set_log_level_handle,
              adm_dis_ucl_set_log_level_impl, adm_dim_ucl_set_log_level_func);
    DIM_DLSYM(dim_func_list->adm_dis_ucl_readVehicleStatus_handle,
              adm_dis_ucl_readVehicleStatus_impl, adm_dim_ucl_readVehicleStatus_func);
    DIM_DLSYM(dim_func_list->adm_dis_ucl_setVehicleStatus_handle,
              adm_dis_ucl_setVehicleStatus_impl, adm_dim_ucl_setVehicleStatus_func);
    DIM_DLSYM(dim_func_list->adm_dis_ucl_getApkVersion_handle,
              adm_dis_ucl_getApkVersion_impl, adm_dim_ucl_getApkVersion_func);
    DIM_DLSYM(dim_func_list->adm_dis_ucl_doApkSelfUpgrade_handle,
              adm_dis_ucl_doApkSelfUpgrade_impl, adm_dim_ucl_doApkUpgrade_func);
    DIM_DLSYM(dim_func_list->adm_dis_ucl_queryApkSelfUpgradeProgress_handle,
              adm_dis_ucl_queryApkSelfUpgradeProgress_impl, adm_dim_ucl_queryApkSelfUpgradeProgress_func);
    DIM_DLSYM(dim_func_list->adm_dis_ucl_queryApkSelfUpgradeResult_handle,
             adm_dis_ucl_queryApkSelfUpgradeResult_impl, adm_dim_ucl_queryApkSelfUpgradeResult_func);
    DIM_DLSYM(dim_func_list->adm_dis_ucl_requestSyncDecryptPackage_handle,
              adm_dis_ucl_requestSyncDecryptPackage_impl, adm_dim_ucl_requestSyncDecryptPackage_func);
    DIM_DLSYM(dim_func_list->adm_dis_ucl_requestRmFile_handle,
              adm_dis_ucl_requestRmFile_impl, adm_dim_ucl_requestRmFile_func);

    /*vdi*/
    DIM_DLSYM(dim_func_list->adm_dis_vdi_init_handle,
              adm_dis_vdi_init_impl, adm_dim_vdi_init_func);
    DIM_DLSYM(dim_func_list->adm_dis_vdi_deinit_handle,
              adm_dis_vdi_deinit_impl, adm_dim_vdi_deinit_func);
    DIM_DLSYM(dim_func_list->adm_dis_vdi_getSdkVersion_handle,
              adm_dis_vdi_getSdkVersion_impl, adm_dim_vdi_getSdkVersion_func);
    DIM_DLSYM(dim_func_list->adm_dis_vdi_readDiagnosticInfo_handle,
              adm_dis_vdi_readDiagnosticInfo_impl, adm_dim_vdi_readDiagnosticInfo_func);
    DIM_DLSYM(dim_func_list->adm_dis_vdi_parallelCollectEcu_handle,
              adm_dis_vdi_parallelCollectEcu_impl, adm_dim_vdi_parallelCollectEcu_func);
    DIM_DLSYM(dim_func_list->adm_dis_vdi_reflashEcusTrigger_handle,
              adm_dis_vdi_reflashEcusTrigger_impl, adm_dim_vdi_reflashEcusTrigger_func);
    DIM_DLSYM(dim_func_list->adm_dis_vdi_AbReflashEcusTrigger_handle,
              adm_dis_vdi_AbReflashEcusTrigger_impl, adm_dim_vdi_AbReflashEcusTrigger_func);
    DIM_DLSYM(dim_func_list->adm_dis_vdi_notifyFlashState_handle,
              adm_dis_vdi_notifyFlashState_impl, adm_dim_vdi_notifyFlashState_func);
    DIM_DLSYM(dim_func_list->adm_dis_vdi_clearDtc_handle,
              adm_dis_vdi_clearDtc_impl, adm_dim_vdi_clearDtc_func);
    DIM_DLSYM(dim_func_list->adm_dis_vdi_queryReflashProgress_handle,
              adm_dis_vdi_queryReflashProgress_impl, adm_dim_vdi_queryReflashProgress_func);
    DIM_DLSYM(dim_func_list->adm_dis_vdi_queryReflashResult_handle,
              adm_dis_vdi_queryReflashResult_impl, adm_dim_vdi_queryReflashResult_func);
    DIM_DLSYM(dim_func_list->adm_dis_vdi_performSpecDiagnosticTask_handle,
              adm_dis_vdi_performSpecDiagnosticTask_impl, adm_dim_vdi_performSpecDiagnosticTask_func);
    DIM_DLSYM(dim_func_list->adm_dis_vdi_readVehicleStatus_handle,
              adm_dis_vdi_readVehicleStatus_impl, adm_dim_vdi_readVehicleStatus_func);
    DIM_DLSYM(dim_func_list->adm_dis_vdi_setSpyVehicleStatus_handle,
              adm_dis_vdi_setSpyVehicleStatus_impl, adm_dim_vdi_setSpyVehicleStatus_func);
    DIM_DLSYM(dim_func_list->adm_dis_vdi_clearSpyVehicleStatus_handle,
              adm_dis_vdi_clearSpyVehicleStatus_impl, adm_dim_vdi_clearSpyVehicleStatus_func);
    DIM_DLSYM(dim_func_list->adm_dis_vdi_controlVehicleStatus_handle,
              adm_dis_vdi_controlVehicleStatus_impl, adm_dim_vdi_controlVehicleStatus_func);
    DIM_DLSYM(dim_func_list->adm_dis_vdi_softwareVersionCheckProcess_handle,
              adm_dis_vdi_softwareVersionCheckProcess_impl, adm_dim_vdi_softwareVersionCheckProcess_func);
    DIM_DLSYM(dim_func_list->adm_dis_vdi_set_log_level_handle,
              adm_dis_vdi_set_log_level_impl, adm_dim_vdi_set_log_level_func);
    DIM_DLSYM(dim_func_list->adm_dis_vdi_programm_trigger_handle,
              adm_dis_vdi_programEcusTrigger_impl, adm_dim_vdi_programm_trigger_func);
    DIM_DLSYM(dim_func_list->adm_dis_vdi_readAbInfo_handle,
              adm_dis_vdi_readAbInfo_impl, adm_dim_vdi_readAbInfo_func);
    DIM_DLSYM(dim_func_list->adm_dis_vdi_switchAb_handle,
              adm_dis_vdi_switchAb_impl, adm_dim_vdi_switchAb_func);
    DIM_DLSYM(dim_func_list->adm_dis_vdi_programm_reboot_handle,
              adm_dis_vdi_programReboot_impl, adm_dim_vdi_programm_reboot_func);
    DIM_DLSYM(dim_func_list->adm_dis_vdi_setDiagTp_handle,
              adm_dis_vdi_setDiagTp_impl, adm_dim_vdi_setDiagTp_func);
			  
    /*trans*/
    DIM_DLSYM(dim_func_list->adm_dis_transferOnHost_init_handle,
              adm_dis_transferOnHost_init_impl, adm_dim_transferOnHost_init_func);
    DIM_DLSYM(dim_func_list->adm_dis_transferOnHost_deinit_handle,
              adm_dis_transferOnHost_deinit_impl, adm_dim_transferOnHost_deinit_func);
    DIM_DLSYM(dim_func_list->adm_dis_trans_getSdkVersion_handle,
              adm_dis_trans_getSdkVersion_impl, adm_dim_trans_getSdkVersion_func);
    DIM_DLSYM(dim_func_list->adm_dis_trans_requestTransferDownloadFromHost_handle,
              adm_dis_trans_requestTransferDownloadFromHost_impl,
              adm_dim_trans_requestTransferDownloadFromHost_func);
    DIM_DLSYM(dim_func_list->adm_dis_trans_queryTransferDownloadFromHostProgress_handle,
              adm_dis_trans_queryProgressTransDownFromHost_impl,
              adm_dim_trans_queryTransferDownloadFromHostProgress_func);
    DIM_DLSYM(dim_func_list->adm_dis_trans_queryTransferDownloadFromHostResult_handle,
              adm_dis_trans_queryResultTransDownFromHost_impl,
              adm_dim_trans_queryTransferDownloadFromHostResult_func);
    DIM_DLSYM(dim_func_list->adm_dis_trans_requestTransferUploadToHost_handle,
              adm_dis_trans_requestTransferUploadToHost_impl,
              adm_dim_trans_requestTransferUploadToHost_func);
    DIM_DLSYM(dim_func_list->adm_dis_trans_requestTransferSyncDownloadFromHost_handle,
              adm_dis_trans_requestTransferSyncDownloadFromHost_impl,
              adm_dim_trans_requestTransferSyncDownloadFromHost_func);
    DIM_DLSYM(dim_func_list->adm_dis_trans_queryTransferUploadToHostProgress_handle,
              adm_dis_trans_queryProgressUpToHost_impl,
              adm_dim_trans_queryTransferUploadToHostProgress_func);
    DIM_DLSYM(dim_func_list->adm_dis_trans_queryTransferUploadToHostResult_handle,
              adm_dis_trans_queryResultUpToHost_impl,
              adm_dim_trans_queryTransferUploadToHostResult_func);
    DIM_DLSYM(dim_func_list->adm_dis_trans_set_log_level_handle,
              adm_dis_trans_set_log_level_impl, adm_dim_trans_set_log_level_func);
    DIM_DLSYM(dim_func_list->adm_dis_trans_switchTransMode_handle,
              adm_dis_trans_switchTransMode_impl,
              adm_dim_trans_switchTransMode_func);
    /*host*/
    DIM_DLSYM(dim_func_list->adm_dis_host_init_handle,
              adm_dis_host_init_impl, adm_dim_host_init_func);
    DIM_DLSYM(dim_func_list->adm_dis_host_deinit_handle,
              adm_dis_host_deinit_impl, adm_dim_host_deinit_func);
    DIM_DLSYM(dim_func_list->adm_dis_host_getSdkVersion_handle,
              adm_dis_host_getSdkVersion_impl, adm_dim_host_getSdkVersion_func);
    DIM_DLSYM(dim_func_list->adm_dis_getVin_handle,
              adm_dis_getVin_impl, adm_dim_getVin_func);
    DIM_DLSYM(dim_func_list->adm_dis_getVehicleVersion_handle,
              adm_dis_getVehicleVersion_impl, adm_dim_getVehicleVersion_func);
    DIM_DLSYM(dim_func_list->adm_dis_setVehicleVersion_handle,
              adm_dis_setVehicleVersion_impl, adm_dim_setVehicleVersion_func);
    DIM_DLSYM(dim_func_list->adm_dis_host_execOtaTask_handle,
              adm_dis_host_execOtaTask_impl, adm_dim_host_execOtaTask_func);
    DIM_DLSYM(dim_func_list->adm_dis_host_setSchduleWakeUpTime_handle,
              adm_dis_host_setSchduleWakeUpTime_impl,
              adm_dim_host_setSchduleWakeUpTime_func);
    DIM_DLSYM(dim_func_list->adm_dis_host_getSchduleWakeUpTime_handle,
              adm_dis_host_getSchduleWakeUpTime_impl,
              adm_dim_host_getSchduleWakeUpTime_func);
    DIM_DLSYM(dim_func_list->adm_dis_host_setVehicleBigVersion_handle,
              adm_dis_host_setVehicleBigVersion_impl,
              adm_dim_host_setVehicleBigVersion_func);
    DIM_DLSYM(dim_func_list->adm_dis_host_getVehicleBigVersion_handle,
              adm_dis_host_getVehicleBigVersion_impl,
              adm_dim_host_getVehicleBigVersion_func);
    DIM_DLSYM(dim_func_list->adm_dis_host_verifySpaceIsEnough_handle,
              adm_dis_host_verifySpaceIsEnough_impl,
              adm_dim_host_verifySpaceIsEnough_func);
    DIM_DLSYM(dim_func_list->adm_dis_host_verifySignature_handle,
              adm_dis_host_verifySignature_impl, adm_dim_host_verifySignature_func);
    DIM_DLSYM(dim_func_list->adm_dis_host_getCertAbsolutePath_handle,
              adm_dis_host_getCertAbsolutePath_impl,
              adm_dim_host_getCertAbsolutePath_func);
	DIM_DLSYM(dim_func_list->adm_dis_host_getPrivacyPath_handle,
              adm_dis_host_getPrivacyPath_impl,
              adm_dim_host_getPrivacy_func);
    DIM_DLSYM(dim_func_list->adm_dis_host_setDiagTroubleCode_handle,
              adm_dis_host_setDiagTroubleCode_impl,
              adm_dim_host_setDiagTroubleCode_func);
    DIM_DLSYM(dim_func_list->adm_dis_host_clearDiagTroubleCode_handle,
              adm_dis_host_clearDiagTroubleCode_impl,
              adm_dim_host_clearDiagTroubleCode_func);
    DIM_DLSYM(dim_func_list->adm_dis_host_readVehicleStatus_handle,
              adm_dis_host_readVehicleStatus_impl, adm_dim_host_readVehicleStatus_func);
    DIM_DLSYM(dim_func_list->adm_dis_host_setVehicleStatus_handle,
              adm_dis_host_setVehicleStatus_impl, adm_dim_host_setVehicleStatus_func);
    DIM_DLSYM(dim_func_list->adm_dis_host_set_log_level_handle,
              adm_dis_host_set_log_level_impl, adm_dim_host_set_log_level_func);
    DIM_DLSYM(dim_func_list->adm_dim_host_getNetworkType_handle,
              adm_dis_host_get_NetworkType_impl, adm_dim_host_getNetworkType_func);
    DIM_DLSYM(dim_func_list->adm_dim_host_setWifiLink_handle,
              adm_dis_host_setWifiLink_impl, adm_dim_host_setWifiLink_func);
    DIM_DLSYM(dim_func_list->adm_dim_host_setPower_handle,
              adm_dis_host_setPower_impl, adm_dim_host_setPower_func);
    DIM_DLSYM(dim_func_list->adm_dim_host_setUpgradeMode_handle,
              adm_dis_host_setUpgradeMode_ipml, adm_dim_host_setUpgradeMode_func);
    DIM_DLSYM(dim_func_list->adm_dim_host_reboot_handle,
              adm_dis_host_reboot_impl, adm_dim_host_reboot_func);
    DIM_DLSYM(dim_func_list->adm_dim_host_downExceptionNotify_handle,
              adm_dis_host_downExceptionNotify_impl, adm_dim_host_downExceptionNotify_func);
    DIM_DLSYM(dim_func_list->adm_dim_host_getOtaModeStatus_handle,
              adm_dis_host_getOtaModeStatus_impl, adm_dim_host_getOtaModeStatus_func);
    DIM_DLSYM(dim_func_list->adm_dim_host_getTboxRemoteDiaStatus_handle,
              adm_dis_host_getTboxRemoteDiaStatus_impl, adm_dim_host_getTboxRemoteDiaStatus_func);
    DIM_DLSYM(dim_func_list->adm_dim_host_setIcuPowerOff_handle,
            adm_dis_host_setIcuPowerOff_impl, adm_dim_host_setIcuPowerOff_func);
	DIM_DLSYM(dim_func_list->adm_dim_host_notifyOtaHmiStart_handle,
			  adm_dis_host_notifyOtaHmiStart_impl, adm_dim_host_notifyOtaHmiStart_func);

	/*dpo*/
    DIM_DLSYM(dim_func_list->adm_dis_dpo_init_handle,
              adm_dis_dpo_init_impl, adm_dim_dpo_init_func);
    DIM_DLSYM(dim_func_list->adm_dis_dpo_deinit_handle,
              adm_dis_dpo_deinit_impl, adm_dim_dpo_deinit_func);
    DIM_DLSYM(dim_func_list->adm_dis_dpo_getSdkVersion_handle,
              adm_dis_dpo_getSdkVersion_impl, adm_dim_dpo_getSdkVersion_func);
    DIM_DLSYM(dim_func_list->adm_dis_dpo_requestRemoteDownloadPackage_handle,
              adm_dis_dpo_requestRemoteDownloadPackage_impl,
              adm_dim_dpo_requestRemoteDownloadPackage_func);
    DIM_DLSYM(dim_func_list->adm_dis_dpo_getRemoteDownloadProgress_handle,
              adm_dis_dpo_getRemoteDownloadProgress_impl,
              adm_dim_dpo_getRemoteDownloadProgress_func);
    DIM_DLSYM(dim_func_list->adm_dis_dpo_getRemoteDownloadResult_handle,
              adm_dis_dpo_getRemoteDownloadResult_impl,
              adm_dim_dpo_getRemoteDownloadResult_func);
    DIM_DLSYM(dim_func_list->adm_dis_dpo_cleanDownloadDir_handle,
              adm_dis_dpo_cleanDownloadDir_impl, adm_dim_dpo_cleanDownloadDir_func);
    DIM_DLSYM(dim_func_list->adm_dis_dpo_interuptDownload_handle,
              adm_dis_dpo_interuptDownload_impl, adm_dim_dpo_interuptDownload_func);
    DIM_DLSYM(dim_func_list->adm_dis_dpo_set_log_level_handle,
              adm_dis_dpo_set_log_level_impl, adm_dim_dpo_set_log_level_func);

    return ADM_ECODE_SUCCESSED;
}

D32S adm_dim_init(void* priv)
{
    D32S nRet = ADM_ECODE_FAILED;

    if(NULL != dim_impl_info)
    {
        return ADM_ECODE_SUCCESSED;
    }

    dim_impl_info = (ADM_DIM_IMPL_INFO_T*)adm_malloc(sizeof(ADM_DIM_IMPL_INFO_T));

    if(dim_impl_info == NULL)
    {
        (void)adm_dim_deInit();
        return ADM_ECODE_NO_MEMORY;
    }

    (void)adm_memzero(dim_impl_info, sizeof(ADM_DIM_IMPL_INFO_T));

#ifdef __DLSYM__
    // load dim impl so
    // 读类型, 加载路径, 为空, 加载默认路径, 都没有退出
    //dim_impl_info->dim_impl_type = (ADM_DIM_IMPL_TYPE_E)adm_cfg_iniParserGetInt(ADM_CFG_VUS_DIM_IMPL_TYPE, 0);
    dim_impl_info->dim_impl_type = ADM_DIM_IMPL_BY_INVALID;

    dim_impl_info->dim_impl_library_path = adm_strdup(adm_cfg_iniParserGetString(
            ADM_CFG_VUS_DIM_IMPL_LIB_PATH, ""));

    if(dim_impl_info->dim_impl_library_path == NULL)
    {
        log_e("invalid param [ info or info->dim_impl_libaray_path ]");
        return ADM_ECODE_INVALID_PARAM;
    }

    log_i("dim_impl_type[ %d ] dim_impl_libaray_path[ %s ]", dim_impl_info->dim_impl_type,
          dim_impl_info->dim_impl_library_path);

    handle = dlopen(dim_impl_info->dim_impl_library_path, RTLD_NOW);

    if(NULL == handle)
    {
        log_e("%s", dlerror());
        return nRet;
    }

#endif

    nRet = init_func_list();

    if(nRet != ADM_ECODE_SUCCESSED)
    {
        (void)adm_dim_deInit();
        return nRet;
    }

    return ADM_ECODE_SUCCESSED;
}

D32S adm_dim_isInit(void)
{
    D32S nRet = ADM_ECODE_FAILED;

    if(NULL == handle)
    {
        nRet = adm_dim_init(NULL);

        if(ADM_ECODE_SUCCESSED != nRet)
        {
            log_e("ota init interface fail\n");
            return nRet;
        }
    }

    return nRet;
}

dim_func_list_t* adm_getFuncList(void)
{
    return dim_func_list;
}

D32S adm_dim_deInit(void)
{
    if(handle != NULL)
    {
#ifdef __DLSYM__
        (void)dlclose(handle);
#endif
        handle = NULL;
    }

    if(dim_func_list != NULL)
    {
        adm_free(dim_func_list);
        dim_func_list = NULL;
    }

    return 0;
}
