#ifndef __ADM_UIM_IMPL_API_H__
#define __ADM_UIM_IMPL_API_H__

#include "adm_uim_check.h"
#include "adm_uim_icp.h"
#include "adm_uim_download.h"
#include "adm_uim_install.h"
#include "adm_uim_services.h"
#include "adm_uim_request.h"
#include "adm_uim_general.h"
#include "adm_uim_ems.h"

#ifdef __cplusplus
extern "C" {
#endif

struct _adm_uim_impls {
    adm_uim_checkProgressNotify_func	adm_uis_checkProgressNotify_impl;
    adm_uim_checkResultNotify_func	adm_uis_checkResultNotify_impl;
    adm_uim_checkInit_func adm_uis_checkInit_impl;
    adm_uim_checkDeinit_func adm_uis_checkDeinit_impl;
    adm_uim_icpInit_func	adm_uis_icpInit_impl;
    adm_uim_icpDeinit_func	adm_uis_icpDeinit_impl;

    adm_uim_downloadConditionsVerifyResultNotify_func	adm_uis_downloadConditionsVerifyResultNotify_impl;
    adm_uim_policyDownloadProgressNotify_func	adm_uis_policyDownloadProgressNotify_impl;
    adm_uim_policyDownloadResultNotify_func	adm_uis_policyDownloadResultNotify_impl;
    adm_uim_ecuDownloadProgressNotify_func	adm_uis_ecuDownloadProgressNotify_impl;
    adm_uim_ecuDownloadResultNotify_func	adm_uis_ecuDownloadResultNotify_impl;
    adm_uim_allEcuDownloadProgressNotify_func	adm_uis_allEcuDownloadProgressNotify_impl;
    adm_uim_allEcuDownloadResultNotify_func	adm_uis_allEcuDownloadResultNotify_impl;
    adm_uim_downloadInit_func	adm_uis_downloadInit_impl;
    adm_uim_downloadDeinit_func	adm_uis_downloadDeinit_impl;
    
    adm_uim_schduleTimeArrived_func	adm_uis_schduleTimeArrived_impl;
    adm_uim_tspSchduleTimestamp_func adm_uis_tspSchduleTimestamp_impl;
    adm_uim_installConditionsVerifyResultNotify_func	adm_uis_installConditionsVerifyResultNotify_impl;
    adm_uim_policyInstallProgressNotify_func	adm_uis_policyInstallProgressNotify_impl;
    adm_uim_policyInstallResultNotify_func	adm_uis_policyInstallResultNotify_impl;
    adm_uim_ecuInstallProgressNotify_func	adm_uis_ecuInstallProgressNotify_impl;
    adm_uim_ecuInstallResultNotify_func	adm_uis_ecuInstallResultNotify_impl;
    adm_uim_allEcuInstallProgressNotify_func	adm_uis_allEcuInstallProgressNotify_impl;
    adm_uim_allEcuInstallResultNotify_func	adm_uis_allEcuInstallResultNotify_impl;
    adm_uim_installInit_func	adm_uis_installInit_impl;
    adm_uim_installDeinit_func	adm_uis_installDeinit_impl;
    
    adm_uim_vsmStateNotify_func adm_uis_vsmStateNotify_impl;

    adm_uim_serviceInit_func	adm_uis_serviceInit_impl;
    adm_uim_serviceDeinit_func	adm_uis_serviceDeinit_impl;

    adm_uim_apkReadyNoticeInit_func	adm_uis_apkReadyNoticeInit_impl;
    adm_uim_apkReadyNoticeDeinit_func	adm_uis_apkReadyNoticeDeinit_impl;

    adm_uim_emsInit_func adm_uis_emsInit_impl;
    adm_uim_emsDeinit_func adm_uis_emsDeinit_impl;

    //request to hmi
    adm_uim_hmi_set_log_level_func                  adm_uis_hmi_set_log_level_impl;
    adm_uim_hmi_getSdkVersion_func                  adm_uis_hmi_getSdkVersion_impl;
    adm_uim_hmi_checkResult_RR_func             adm_uis_hmi_checkResult_RR_impl;
    adm_uim_hmi_conds_downloadResult_RR_func    adm_uis_hmi_conds_downloadResult_RR_impl;
    adm_uim_hmi_policy_downloadResult_RR_func   adm_uis_hmi_policy_downloadResult_RR_impl;
    adm_uim_hmi_ecu_downloadResult_RR_func      adm_uis_hmi_ecu_downloadResult_RR_impl;
    adm_uim_hmi_allecu_downloadResult_RR_func   adm_uis_hmi_allecu_downloadResult_RR_impl;
    adm_uim_hmi_downloadTaskStop_RR_func        adm_uis_hmi_downloadTaskStop_RR_impl;
    adm_uim_hmi_conds_installResult_RR_func     adm_uis_hmi_conds_installResult_RR_impl;
    adm_uim_hmi_policy_installResult_RR_func    adm_uis_hmi_policy_installResult_RR_impl;
    adm_uim_hmi_policy_installResult_RR_func    adm_uis_hmi_repeat_policy_installResult_RR_impl;
    adm_uim_hmi_stage_installResult_RR_func     adm_uis_hmi_stage_installResult_RR_impl;
    adm_uim_hmi_ecu_installResult_RR_func       adm_uis_hmi_ecu_installResult_RR_impl;
    adm_uim_hmi_allecu_installResult_RR_func    adm_uis_hmi_allecu_installResult_RR_impl;
};
typedef struct _adm_uim_impls  ADM_UIM_IMPLS;

#ifdef __cplusplus
}
#endif

#endif
//__ADM_UIM_IMPL_API_H__
