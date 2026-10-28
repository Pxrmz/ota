#include "adm_uim_log.h"
#include "adm_uim_api.h"
#include "adm_com_errorCode.h"
#include "adm_memory.h"
#include "adm_string.h"
#include <dlfcn.h>
#include <unistd.h>

#include "adm_uim_apiImpl.h" 

#include "adm_cfg_api.h"
#include "adm_cfg_default.h"


#ifndef __DLSYM__
#include "adm_uis_check.h"
#include "adm_uis_icp.h"
#include "adm_uis_download.h"
#include "adm_uis_install.h"
#include "adm_uis_ems.h"
#include "adm_uis_services.h"
#include "adm_uis_request.h"
#include "adm_uis_general.h"
#endif

typedef struct {
    void* handle;
    ADM_UIM_IMPLS* impls;
} ADM_UIM_IMPL_INFO;

#ifdef __DLSYM__
#define UIM_DLSYM(type, member)                                                         \
            do {                                                                        \
                char *error = NULL;                                                     \
                s_implsInfo->impls->member = (type)dlsym(s_implsInfo->handle , #member);   \
                if((error = dlerror()) != NULL) {                                       \
                    log_e("%s", error);                                                 \
                    _exit(1);                                                           \
                }                                                                       \
            } while(FALSE)
#else
	#define UIM_DLSYM(type, member)	s_implsInfo->impls->member = (type)member;
#endif

static ADM_UIM_IMPL_INFO* s_implsInfo = NULL;

static void initImpls(void)
{
    UIM_DLSYM(adm_uim_checkProgressNotify_func, adm_uis_checkProgressNotify_impl);
    UIM_DLSYM(adm_uim_checkResultNotify_func, adm_uis_checkResultNotify_impl);
    UIM_DLSYM(adm_uim_checkInit_func, adm_uis_checkInit_impl);
    UIM_DLSYM(adm_uim_checkDeinit_func, adm_uis_checkDeinit_impl);
    UIM_DLSYM(adm_uim_icpInit_func, adm_uis_icpInit_impl);
    UIM_DLSYM(adm_uim_icpDeinit_func, adm_uis_icpDeinit_impl);
    UIM_DLSYM(adm_uim_downloadConditionsVerifyResultNotify_func, adm_uis_downloadConditionsVerifyResultNotify_impl);
    UIM_DLSYM(adm_uim_policyDownloadProgressNotify_func, adm_uis_policyDownloadProgressNotify_impl);
    UIM_DLSYM(adm_uim_policyDownloadResultNotify_func, adm_uis_policyDownloadResultNotify_impl);
    UIM_DLSYM(adm_uim_ecuDownloadProgressNotify_func, adm_uis_ecuDownloadProgressNotify_impl);
    UIM_DLSYM(adm_uim_ecuDownloadResultNotify_func, adm_uis_ecuDownloadResultNotify_impl);
    UIM_DLSYM(adm_uim_allEcuDownloadProgressNotify_func, adm_uis_allEcuDownloadProgressNotify_impl);
    UIM_DLSYM(adm_uim_allEcuDownloadResultNotify_func, adm_uis_allEcuDownloadResultNotify_impl);
    UIM_DLSYM(adm_uim_downloadInit_func, adm_uis_downloadInit_impl);
    UIM_DLSYM(adm_uim_downloadDeinit_func, adm_uis_downloadDeinit_impl);
    
    UIM_DLSYM(adm_uim_schduleTimeArrived_func, adm_uis_schduleTimeArrived_impl);
    UIM_DLSYM(adm_uim_tspSchduleTimestamp_func, adm_uis_tspSchduleTimestamp_impl);
    UIM_DLSYM(adm_uim_installConditionsVerifyResultNotify_func, adm_uis_installConditionsVerifyResultNotify_impl);
    UIM_DLSYM(adm_uim_policyInstallProgressNotify_func, adm_uis_policyInstallProgressNotify_impl);
    UIM_DLSYM(adm_uim_policyInstallResultNotify_func, adm_uis_policyInstallResultNotify_impl);
    UIM_DLSYM(adm_uim_ecuInstallProgressNotify_func, adm_uis_ecuInstallProgressNotify_impl);
    UIM_DLSYM(adm_uim_ecuInstallResultNotify_func, adm_uis_ecuInstallResultNotify_impl);
    UIM_DLSYM(adm_uim_allEcuInstallProgressNotify_func, adm_uis_allEcuInstallProgressNotify_impl);
    UIM_DLSYM(adm_uim_allEcuInstallResultNotify_func, adm_uis_allEcuInstallResultNotify_impl);
    UIM_DLSYM(adm_uim_installInit_func, adm_uis_installInit_impl);
    UIM_DLSYM(adm_uim_installDeinit_func, adm_uis_installDeinit_impl);
    
    UIM_DLSYM(adm_uim_vsmStateNotify_func, adm_uis_vsmStateNotify_impl);

    UIM_DLSYM(adm_uim_serviceInit_func, adm_uis_serviceInit_impl);
    UIM_DLSYM(adm_uim_serviceDeinit_func, adm_uis_serviceDeinit_impl);

    UIM_DLSYM(adm_uim_apkReadyNoticeInit_func, adm_uis_apkReadyNoticeInit_impl);
    UIM_DLSYM(adm_uim_apkReadyNoticeDeinit_func, adm_uis_apkReadyNoticeDeinit_impl);

    UIM_DLSYM(adm_uim_emsInit_func, adm_uis_emsInit_impl);
    UIM_DLSYM(adm_uim_emsDeinit_func, adm_uis_emsDeinit_impl);

    //uim request to hmi
    UIM_DLSYM(adm_uim_hmi_set_log_level_func, adm_uis_hmi_set_log_level_impl);
    UIM_DLSYM(adm_uim_hmi_getSdkVersion_func, adm_uis_hmi_getSdkVersion_impl);
    UIM_DLSYM(adm_uim_hmi_checkResult_RR_func, adm_uis_hmi_checkResult_RR_impl);
    UIM_DLSYM(adm_uim_hmi_conds_downloadResult_RR_func, adm_uis_hmi_conds_downloadResult_RR_impl);
    UIM_DLSYM(adm_uim_hmi_policy_downloadResult_RR_func, adm_uis_hmi_policy_downloadResult_RR_impl);
    UIM_DLSYM(adm_uim_hmi_ecu_downloadResult_RR_func, adm_uis_hmi_ecu_downloadResult_RR_impl);
    UIM_DLSYM(adm_uim_hmi_allecu_downloadResult_RR_func, adm_uis_hmi_allecu_downloadResult_RR_impl);
    UIM_DLSYM(adm_uim_hmi_downloadTaskStop_RR_func, adm_uis_hmi_downloadTaskStop_RR_impl);
    UIM_DLSYM(adm_uim_hmi_conds_installResult_RR_func, adm_uis_hmi_conds_installResult_RR_impl);
    UIM_DLSYM(adm_uim_hmi_policy_installResult_RR_func, adm_uis_hmi_policy_installResult_RR_impl);
    UIM_DLSYM(adm_uim_hmi_policy_installResult_RR_func, adm_uis_hmi_repeat_policy_installResult_RR_impl);
    UIM_DLSYM(adm_uim_hmi_stage_installResult_RR_func, adm_uis_hmi_stage_installResult_RR_impl);
    UIM_DLSYM(adm_uim_hmi_ecu_installResult_RR_func, adm_uis_hmi_ecu_installResult_RR_impl);
    UIM_DLSYM(adm_uim_hmi_allecu_installResult_RR_func, adm_uis_hmi_allecu_installResult_RR_impl);
}


D32S adm_uim_init(void)
{
	log_i("adm_uim_init begin");
	D32S ret = ADM_ECODE_FAILED;

    if(NULL != s_implsInfo)
    {
        return ADM_ECODE_SUCCESSED;
    }
    do
    {
        s_implsInfo = (ADM_UIM_IMPL_INFO*)adm_malloc(sizeof(ADM_UIM_IMPL_INFO));
        if(NULL == s_implsInfo)
        {
            log_e("create mpls info failed");
            break;
        }

#ifdef __DLSYM__
		const DCHAR* libPath = adm_strdup(adm_cfg_iniParserGetString(ADM_CFG_VUS_UIM_IMPL_LIB_PATH, ""));
		log_i("uim_impl_libaray_path[ %s ]", libPath);
        s_implsInfo->handle = dlopen(libPath, RTLD_NOW|RTLD_GLOBAL);
        if(NULL == s_implsInfo->handle)
        {
            log_e("%s", dlerror());
            break;
        }  
#endif

        s_implsInfo->impls = (ADM_UIM_IMPLS*)adm_malloc(sizeof(ADM_UIM_IMPLS));
        if(NULL == s_implsInfo->impls)
        {
            log_e("create mpls set failed");
            break;
        }


        initImpls();
        ret = ADM_ECODE_SUCCESSED;
    } while(FALSE);   
    if(ret != ADM_ECODE_SUCCESSED)
    {
        (void)adm_uim_deInit();
    }
    
    log_i("adm_uim_init end, retCode: %d", ret);
    return ret;
}

D32S adm_uim_deInit(void)
{
    log_i("uim deinit begin");
    
    if(s_implsInfo != NULL)
    {
#ifdef __DLSYM__    
        if(s_implsInfo->handle != NULL)
        {
            (void)dlclose(s_implsInfo->handle);
            s_implsInfo->handle = NULL;
        }
#endif        
        if(s_implsInfo->impls != NULL)
        {
            adm_free(s_implsInfo->impls);
            s_implsInfo->impls = NULL;
        }
        adm_free(s_implsInfo);
        s_implsInfo = NULL;
    }
    log_i("adm_uim_init end");
    return 0;
}

const struct _adm_uim_impls* adm_uim_getImpls(void)
{
	if(s_implsInfo != NULL)
	{
    	return s_implsInfo->impls;
	}
	else
	{
		return NULL;
	}
}
