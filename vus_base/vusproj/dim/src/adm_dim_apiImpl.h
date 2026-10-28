#ifndef __ADM_DIM_API_IMPL_H_
#define __ADM_DIM_API_IMPL_H_

#include "adm_typedefs.h"

#include "adm_dim_api.h"
#include "adm_dim_host.h"
#include "adm_dim_ucl.h"
#include "adm_dim_vdi.h"
#include "adm_dim_transHost.h"
#include "adm_dim_dpo.h"

#ifdef __cplusplus
extern "C" {
#endif

#define EXPORT_FUN __attribute__((visibility("default")))


typedef struct
{
    /*ucl*/
    adm_dim_ucl_init_func                                     adm_dis_ucl_init_handle                    ;
    adm_dim_ucl_deinit_func                                   adm_dis_ucl_deinit_handle                    ;
    adm_dim_ucl_getSdkVersion_func                            adm_dis_ucl_getSdkVersion_handle                             ;
    adm_dim_ucl_readPartInformation_func                      adm_dis_ucl_readPartInformation_handle                       ;
    adm_dim_ucl_readSnInfo_func                               adm_dis_ucl_readSnInfo_handle                             ;
    adm_dim_ucl_parallelCollectEcu_func                       adm_dis_ucl_parallelCollectEcu_handle                       ;
    adm_dim_ucl_requestDoUpgrade_func                         adm_dis_ucl_requestDoUpgrade_handle                          ;
    adm_dim_ucl_requestDoUpgrade_icc_func                     adm_dis_ucl_requestDoUpgrade_icc_handle                      ;
    adm_dim_ucl_requestSetChangeSlot_icc_func                 adm_dis_ucl_requestSetChangeSlot_icc_handle                  ;
    adm_dim_ucl_requestUpdateAndReboot_func               adm_dis_ucl_requestUpdateAndReboot_handle                ;
    adm_dim_ucl_queryUpgradeProgress_func                     adm_dis_ucl_queryUpgradeProgress_handle                      ;
    adm_dim_ucl_queryUpgradeResult_func                       adm_dis_ucl_queryUpgradeResult_handle                        ;
    adm_dim_ucl_requestDecryptPackage_func                    adm_dis_ucl_requestDecryptPackage_handle                     ;
    adm_dim_ucl_getDecryptPackageProgress_func                adm_dis_ucl_getDecryptPackageProgress_handle                 ;
    adm_dim_ucl_getDecryptPackageResult_func                  adm_dis_ucl_getDecryptPackageResult_handle                   ;
    adm_dim_ucl_requestVerifyHash_func                        adm_dis_ucl_requestVerifyHash_handle                         ;
    adm_dim_ucl_verifySpaceIsEnouth_func                      adm_dis_ucl_verifySpaceIsEnouth_handle                       ;
    adm_dim_ucl_request_Verify_Sign_func                      adm_dis_ucl_request_Verify_Sign_handle                       ;
    adm_dim_ucl_set_log_level_func                            adm_dis_ucl_set_log_level_handle;
    adm_dim_ucl_readVehicleStatus_func                        adm_dis_ucl_readVehicleStatus_handle                         ;
    adm_dim_ucl_setVehicleStatus_func                         adm_dis_ucl_setVehicleStatus_handle;
    adm_dim_ucl_getApkVersion_func                            adm_dis_ucl_getApkVersion_handle;
    adm_dim_ucl_doApkUpgrade_func                             adm_dis_ucl_doApkSelfUpgrade_handle;
    adm_dim_ucl_queryApkSelfUpgradeProgress_func              adm_dis_ucl_queryApkSelfUpgradeProgress_handle;
    adm_dim_ucl_queryApkSelfUpgradeResult_func                adm_dis_ucl_queryApkSelfUpgradeResult_handle;
    adm_dim_ucl_requestSyncDecryptPackage_func                adm_dis_ucl_requestSyncDecryptPackage_handle;                 
    adm_dim_ucl_requestRmFile_func                            adm_dis_ucl_requestRmFile_handle;                 

    /*vdi*/
    adm_dim_vdi_init_func                                     adm_dis_vdi_init_handle                    ;
    adm_dim_vdi_deinit_func                                   adm_dis_vdi_deinit_handle                    ;
    adm_dim_vdi_getSdkVersion_func                            adm_dis_vdi_getSdkVersion_handle                             ;
    adm_dim_vdi_readDiagnosticInfo_func                       adm_dis_vdi_readDiagnosticInfo_handle                        ;
    adm_dim_vdi_parallelCollectEcu_func                       adm_dis_vdi_parallelCollectEcu_handle;
    adm_dim_vdi_reflashEcusTrigger_func                       adm_dis_vdi_reflashEcusTrigger_handle                        ;
    adm_dim_vdi_AbReflashEcusTrigger_func                       adm_dis_vdi_AbReflashEcusTrigger_handle                        ;
    adm_dim_vdi_notifyFlashState_func                       adm_dis_vdi_notifyFlashState_handle                        ;
    adm_dim_vdi_clearDtc_func                               adm_dis_vdi_clearDtc_handle                                 ;
    adm_dim_vdi_queryReflashProgress_func                     adm_dis_vdi_queryReflashProgress_handle                      ;
    adm_dim_vdi_queryReflashResult_func                       adm_dis_vdi_queryReflashResult_handle                        ;
    adm_dim_vdi_performSpecDiagnosticTask_func                adm_dis_vdi_performSpecDiagnosticTask_handle                 ;
    adm_dim_vdi_readVehicleStatus_func                        adm_dis_vdi_readVehicleStatus_handle                         ;
    adm_dim_vdi_setSpyVehicleStatus_func                      adm_dis_vdi_setSpyVehicleStatus_handle                       ;
    adm_dim_vdi_clearSpyVehicleStatus_func                    adm_dis_vdi_clearSpyVehicleStatus_handle                     ;
    adm_dim_vdi_controlVehicleStatus_func                     adm_dis_vdi_controlVehicleStatus_handle                      ;
    adm_dim_vdi_softwareVersionCheckProcess_func              adm_dis_vdi_softwareVersionCheckProcess_handle               ;
    adm_dim_vdi_set_log_level_func                            adm_dis_vdi_set_log_level_handle;
    adm_dim_vdi_programm_trigger_func                         adm_dis_vdi_programm_trigger_handle;
    adm_dim_vdi_readAbInfo_func                               adm_dis_vdi_readAbInfo_handle;
    adm_dim_vdi_switchAb_func                                 adm_dis_vdi_switchAb_handle;
    adm_dim_vdi_programm_reboot_func                          adm_dis_vdi_programm_reboot_handle;
    adm_dim_vdi_setDiagTp_func                                adm_dis_vdi_setDiagTp_handle;
    
    /*trans*/
    adm_dim_transferOnHost_init_func                          adm_dis_transferOnHost_init_handle                    ;
    adm_dim_transferOnHost_deinit_func                        adm_dis_transferOnHost_deinit_handle                    ;
    adm_dim_trans_getSdkVersion_func                          adm_dis_trans_getSdkVersion_handle                           ;
    adm_dim_trans_requestTransferDownloadFromHost_func        adm_dis_trans_requestTransferDownloadFromHost_handle         ;
    adm_dim_trans_requestTransferSyncDownloadFromHost_func    adm_dis_trans_requestTransferSyncDownloadFromHost_handle;
    adm_dim_trans_queryTransferDownloadFromHostProgress_func  adm_dis_trans_queryTransferDownloadFromHostProgress_handle   ;
    adm_dim_trans_queryTransferDownloadFromHostResult_func    adm_dis_trans_queryTransferDownloadFromHostResult_handle     ;
    adm_dim_trans_requestTransferUploadToHost_func            adm_dis_trans_requestTransferUploadToHost_handle             ;
    adm_dim_trans_queryTransferUploadToHostProgress_func      adm_dis_trans_queryTransferUploadToHostProgress_handle       ;
    adm_dim_trans_queryTransferUploadToHostResult_func        adm_dis_trans_queryTransferUploadToHostResult_handle         ;
    adm_dim_trans_set_log_level_func    adm_dis_trans_set_log_level_handle;
    adm_dim_trans_switchTransMode_func adm_dis_trans_switchTransMode_handle;
    /*host*/
    adm_dim_host_init_func                                    adm_dis_host_init_handle                                      ;
    adm_dim_host_deinit_func                                  adm_dis_host_deinit_handle                                    ;
    adm_dim_host_getSdkVersion_func                           adm_dis_host_getSdkVersion_handle                            ;
    adm_dim_getVin_func                                       adm_dis_getVin_handle                                        ;
    adm_dim_getVehicleVersion_func                            adm_dis_getVehicleVersion_handle                             ;
    adm_dim_setVehicleVersion_func                            adm_dis_setVehicleVersion_handle                             ;
    adm_dim_host_execOtaTask_func                             adm_dis_host_execOtaTask_handle                              ;
    adm_dim_host_setSchduleWakeUpTime_func                    adm_dis_host_setSchduleWakeUpTime_handle                     ;
    adm_dim_host_getSchduleWakeUpTime_func                    adm_dis_host_getSchduleWakeUpTime_handle                     ;
    adm_dim_host_setVehicleBigVersion_func                    adm_dis_host_setVehicleBigVersion_handle                     ;
    adm_dim_host_getVehicleBigVersion_func                    adm_dis_host_getVehicleBigVersion_handle                     ;
    adm_dim_host_verifySpaceIsEnough_func                     adm_dis_host_verifySpaceIsEnough_handle                      ;
    adm_dim_host_verifySignature_func                         adm_dis_host_verifySignature_handle                          ;
    adm_dim_host_getCertAbsolutePath_func                     adm_dis_host_getCertAbsolutePath_handle                      ;
	adm_dim_host_getPrivacy_func					 		  adm_dis_host_getPrivacyPath_handle					   ;
    adm_dim_host_setDiagTroubleCode_func                      adm_dis_host_setDiagTroubleCode_handle                       ;
    adm_dim_host_clearDiagTroubleCode_func                    adm_dis_host_clearDiagTroubleCode_handle                     ;
    adm_dim_host_readVehicleStatus_func                       adm_dis_host_readVehicleStatus_handle                         ;
    adm_dim_host_setVehicleStatus_func                        adm_dis_host_setVehicleStatus_handle;
    adm_dim_host_set_log_level_func    adm_dis_host_set_log_level_handle;
    adm_dim_host_getNetworkType_func    adm_dim_host_getNetworkType_handle;
    adm_dim_host_setWifiLink_func       adm_dim_host_setWifiLink_handle;
    adm_dim_host_setPower_func          adm_dim_host_setPower_handle;
    adm_dim_host_setUpgradeMode_func    adm_dim_host_setUpgradeMode_handle;
    adm_dim_host_reboot_func          adm_dim_host_reboot_handle;
    adm_dim_host_downExceptionNotify_func   adm_dim_host_downExceptionNotify_handle;
    adm_dim_host_getOtaModeStatus_func     adm_dim_host_getOtaModeStatus_handle;
    adm_dim_host_getTboxRemoteDiaStatus_func adm_dim_host_getTboxRemoteDiaStatus_handle;
    adm_dim_host_setIcuPowerOff_func    adm_dim_host_setIcuPowerOff_handle;
	adm_dim_host_notifyOtaHmiStart_func adm_dim_host_notifyOtaHmiStart_handle;

    /*dpo*/
    adm_dim_dpo_init_func                                       adm_dis_dpo_init_handle                                         ;
    adm_dim_dpo_deinit_func                                     adm_dis_dpo_deinit_handle                                       ;
    adm_dim_dpo_getSdkVersion_func                             adm_dis_dpo_getSdkVersion_handle                             ;
    adm_dim_dpo_requestRemoteDownloadPackage_func              adm_dis_dpo_requestRemoteDownloadPackage_handle              ;
    adm_dim_dpo_getRemoteDownloadProgress_func                 adm_dis_dpo_getRemoteDownloadProgress_handle                 ;
    adm_dim_dpo_getRemoteDownloadResult_func                   adm_dis_dpo_getRemoteDownloadResult_handle                   ;
    adm_dim_dpo_cleanDownloadDir_func    adm_dis_dpo_cleanDownloadDir_handle;
    adm_dim_dpo_interuptDownload_func    adm_dis_dpo_interuptDownload_handle;
    adm_dim_dpo_set_log_level_func    adm_dis_dpo_set_log_level_handle;
} dim_func_list_t;

extern dim_func_list_t* adm_getFuncList(void);


#ifdef __cplusplus
}
#endif

#endif//__ADM_DIM_SOA_API_IMPL_H_
