#include "adm_uim_request.h"
#include "adm_com_errorCode.h"
#include "adm_uim_log.h"

#include "adm_memory.h"
#include "adm_soa.h"

#include "adm_uim_api.h"
#include "adm_uim_apiImpl.h"

D32S adm_uim_hmi_set_log_level(const D16U instanceId, const D16U timeout, 
        D32U logLevel, D32S* errorCodes)
{
    log_i("adm_uim_hmi_set_log_level begin");
    
    D32S retCode = ADM_ECODE_FAILED;
    const struct _adm_uim_impls* pImpls = adm_uim_getImpls();
	if(NULL == pImpls || NULL == pImpls->adm_uis_hmi_set_log_level_impl)
	{
		log_e("_adm_uim_impls or adm_uis_hmi_set_log_level_impl pointer invalid, nRet[ %d ]", retCode);
		return retCode;
	}
    retCode =  (pImpls->adm_uis_hmi_set_log_level_impl)(instanceId, timeout, logLevel, errorCodes);

    log_i("adm_uim_hmi_set_log_level end, %d", retCode);
    return retCode;
}

void adm_uim_hmi_getSdkVersion(const D16U instanceId, const D16U timeout, 
        DCHAR **sdkVersion, DCHAR **apkVersion,D32S *errorCode)
{
    log_i("adm_uim_hmi_getSdkVersion begin");

    const struct _adm_uim_impls* pImpls = adm_uim_getImpls();
	if(NULL == pImpls || NULL == pImpls->adm_uis_hmi_getSdkVersion_impl)
	{
		log_e("_adm_uim_impls or adm_uis_hmi_getSdkVersion_impl pointer invalid");
		return ;
	}
    (pImpls->adm_uis_hmi_getSdkVersion_impl)(instanceId, timeout, sdkVersion,apkVersion, errorCode);

    log_i("adm_uim_hmi_getSdkVersion end");
}

void adm_uim_hmi_checkResult_RR(const ADM_APPLICATION_COMMON_CMDID_TYPE_E eCmdidType,const DCHAR *sessionId,const DCHAR *state,
                const ADM_UIM_SOA_CHECK_HMI_POLICY_INFO_T *pstPolicyInfo,const D32S reqCode,D32S *respCode)
{
    log_i("adm_uim_hmi_checkResult_RR begin");

    const struct _adm_uim_impls* pImpls = adm_uim_getImpls();
	if(NULL == pImpls || NULL == pImpls->adm_uis_hmi_checkResult_RR_impl)
	{
		log_e("_adm_uim_impls or adm_uis_hmi_checkResult_RR_impl pointer invalid");
		return ;
	}
    (pImpls->adm_uis_hmi_checkResult_RR_impl)(eCmdidType,sessionId,state, pstPolicyInfo,reqCode,respCode);

    log_i("adm_uim_hmi_checkResult_RR end");
    return;
}

void adm_uim_hmi_conds_downloadResult_RR(const ADM_APPLICATION_COMMON_CMDID_TYPE_E eCmdidType,const DCHAR *sessionId,const DCHAR *state,
                const ADM_UIM_SOA_CONDITION_DATA_ARRAY_T *condsArrarList,const D32S verifyResult,D32S *respCode)
{
    log_i("before adm_uim_hmi_conds_downloadResult_RR");

    const struct _adm_uim_impls *pImpls = adm_uim_getImpls();
	if(NULL == pImpls || NULL == pImpls->adm_uis_hmi_conds_downloadResult_RR_impl)
	{
		log_e("_adm_uim_impls or adm_uis_hmi_conds_downloadResult_RR_impl pointer invalid");
		return ;
	}
    (pImpls->adm_uis_hmi_conds_downloadResult_RR_impl)(eCmdidType, sessionId,state, condsArrarList,verifyResult,respCode);

    log_i("Finish adm_uim_hmi_conds_downloadResult_RR");
    return;
}

void adm_uim_hmi_policy_downloadResult_RR(const ADM_APPLICATION_COMMON_CMDID_TYPE_E eCmdidType,const DCHAR *sessionId,const DCHAR *state,
                const ADM_UIM_SOA_DOWNLOAD_RESULT_INFO_T *pstPolicyDownloadResult,D32S *respCode)
{
    log_i("before adm_uim_hmi_policy_downloadResult_RR");

    const struct _adm_uim_impls *pImpls = adm_uim_getImpls();
	if(NULL == pImpls || NULL == pImpls->adm_uis_hmi_policy_downloadResult_RR_impl)
	{
		log_e("_adm_uim_impls or adm_uis_hmi_policy_downloadResult_RR_impl pointer invalid");
		return ;
	}
    (pImpls->adm_uis_hmi_policy_downloadResult_RR_impl)(eCmdidType, sessionId,state, pstPolicyDownloadResult,respCode);

    log_i("Finish adm_uim_hmi_policy_downloadResult_RR");
    return;
}

void adm_uim_hmi_ecu_downloadResult_RR(const ADM_APPLICATION_COMMON_CMDID_TYPE_E eCmdidType,const DCHAR *sessionId,const DCHAR *state,
                const ADM_UIM_SOA_ECU_IDEN_INFO_T *pstEcuIdenInfo,const ADM_UIM_SOA_DOWNLOAD_RESULT_INFO_T *pstEcuDownloadResult,D32S *respCode)
{
    log_i("before adm_uim_hmi_ecu_downloadResult_RR");

    const struct _adm_uim_impls *pImpls = adm_uim_getImpls();
	if(NULL == pImpls || NULL == pImpls->adm_uis_hmi_ecu_downloadResult_RR_impl)
	{
		log_e("_adm_uim_impls or adm_uis_hmi_ecu_downloadResult_RR_impl pointer invalid");
		return ;
	}
    (pImpls->adm_uis_hmi_ecu_downloadResult_RR_impl)(eCmdidType, sessionId,state, pstEcuIdenInfo,pstEcuDownloadResult,respCode);

    log_i("Finish adm_uim_hmi_ecu_downloadResult_RR");
    return;
}

void adm_uim_hmi_allecu_downloadResult_RR(const ADM_APPLICATION_COMMON_CMDID_TYPE_E eCmdidType,const DCHAR *sessionId,const DCHAR *state,
                const ADM_UIM_SOA_ECU_DOWNLOAD_RESULT_ARRAY_INFO_T *pstEcuDownloadResultArrayInfo,const ADM_UIM_SOA_DOWNLOAD_RESULT_INFO_T *pstPolicyDownloadResult,D32S *respCode)
{
    log_i("before adm_uim_hmi_allecu_downloadResult_RR");

    const struct _adm_uim_impls *pImpls = adm_uim_getImpls();
	if(NULL == pImpls || NULL == pImpls->adm_uis_hmi_allecu_downloadResult_RR_impl)
	{
		log_e("_adm_uim_impls or adm_uis_hmi_allecu_downloadResult_RR_impl pointer invalid");
		return ;
	}
    (pImpls->adm_uis_hmi_allecu_downloadResult_RR_impl)(eCmdidType, sessionId,state, pstEcuDownloadResultArrayInfo,pstPolicyDownloadResult,respCode);

    log_i("Finish adm_uim_hmi_allecu_downloadResult_RR");
    return;
}

void adm_uim_hmi_downloadTaskStop_RR(const ADM_APPLICATION_COMMON_CMDID_TYPE_E eCmdidType, const DCHAR *sessionId, D32S *respCode)
{
    log_i("before adm_uim_hmi_downloadTaskStop_RR");

    const struct _adm_uim_impls *pImpls = adm_uim_getImpls();
    if (NULL == pImpls || NULL == pImpls->adm_uis_hmi_downloadTaskStop_RR_impl)
    {
        log_e("_adm_uim_impls or adm_uis_hmi_downloadTaskStop_RR_impl pointer invalid");
        return;
    }
    (pImpls->adm_uis_hmi_downloadTaskStop_RR_impl)(eCmdidType, sessionId, respCode);

    log_i("Finish adm_uim_hmi_downloadTaskStop_RR");
    return;
}

void adm_uim_hmi_conds_installResult_RR(const ADM_APPLICATION_COMMON_CMDID_TYPE_E eCmdidType,const DCHAR *sessionId,const DCHAR *state,
                const ADM_UIM_SOA_CONDITION_DATA_ARRAY_T *condsArrarList,const D32S verifyResult,D32S *respCode)
{
    log_i("before adm_uim_hmi_conds_installResult_RR");

    const struct _adm_uim_impls *pImpls = adm_uim_getImpls();
	if(NULL == pImpls || NULL == pImpls->adm_uis_hmi_conds_installResult_RR_impl)
	{
		log_e("_adm_uim_impls or adm_uis_hmi_conds_installResult_RR_impl pointer invalid");
		return ;
	}
    (pImpls->adm_uis_hmi_conds_installResult_RR_impl)(eCmdidType, sessionId,state, condsArrarList,verifyResult,respCode);

    log_i("Finish adm_uim_hmi_conds_installResult_RR");
    return;
}

void adm_uim_hmi_policy_installResult_RR(const ADM_APPLICATION_COMMON_CMDID_TYPE_E eCmdidType,const DCHAR *sessionId,const DCHAR *state,
                const ADM_UIM_SOA_INSTALL_RESULT_INFO_T *pstPolicyInstallResult,D32S *respCode)
{
    log_i("before adm_uim_hmi_policy_installResult_RR");

    const struct _adm_uim_impls *pImpls = adm_uim_getImpls();
	if(NULL == pImpls || NULL == pImpls->adm_uis_hmi_policy_installResult_RR_impl)
	{
		log_e("_adm_uim_impls or adm_uis_hmi_policy_installResult_RR_impl pointer invalid");
		return ;
	}
    (pImpls->adm_uis_hmi_policy_installResult_RR_impl)(eCmdidType, sessionId,state, pstPolicyInstallResult,respCode);

    log_i("Finish adm_uim_hmi_policy_installResult_RR");
    return;
}

void adm_uim_hmi_repeat_policy_installResult_RR(const ADM_APPLICATION_COMMON_CMDID_TYPE_E eCmdidType,const DCHAR *sessionId,const DCHAR *state,
                const ADM_UIM_SOA_INSTALL_RESULT_INFO_T *pstPolicyInstallResult,D32S *respCode)
{
    log_i("before adm_uim_hmi_repeat_policy_installResult_RR");

    const struct _adm_uim_impls *pImpls = adm_uim_getImpls();
	if(NULL == pImpls || NULL == pImpls->adm_uis_hmi_repeat_policy_installResult_RR_impl)
	{
		log_e("_adm_uim_impls or adm_uis_hmi_repeat_policy_installResult_RR_impl pointer invalid");
		return ;
	}
    (pImpls->adm_uis_hmi_repeat_policy_installResult_RR_impl)(eCmdidType, sessionId,state, pstPolicyInstallResult,respCode);

    log_i("Finish adm_uim_hmi_repeat_policy_installResult_RR");
    return;
}

void adm_uim_hmi_stage_installResult_RR(const ADM_APPLICATION_COMMON_CMDID_TYPE_E eCmdidType,const DCHAR *sessionId,const DCHAR *state,
                const ADM_UIM_SOA_INSTALL_RESULT_INFO_T *pstPolicyInstallResult,D32S *respCode)
{
    log_i("before adm_uim_hmi_stage_installResult_RR");

    const struct _adm_uim_impls *pImpls = adm_uim_getImpls();
	if(NULL == pImpls || NULL == pImpls->adm_uis_hmi_stage_installResult_RR_impl)
	{
		log_e("_adm_uim_impls or adm_uis_hmi_stage_installResult_RR_impl pointer invalid");
		return ;
	}
    (pImpls->adm_uis_hmi_stage_installResult_RR_impl)(eCmdidType, sessionId,state, pstPolicyInstallResult,respCode);

    log_i("Finish adm_uim_hmi_stage_installResult_RR");
    return;
}

void adm_uim_hmi_ecu_installResult_RR(const ADM_APPLICATION_COMMON_CMDID_TYPE_E eCmdidType,const DCHAR *sessionId,const DCHAR *state,
                const ADM_UIM_SOA_ECU_IDEN_INFO_T *pstEcuIdenInfo,const ADM_UIM_SOA_INSTALL_RESULT_INFO_T *pstEcuInstallResultInfo,D32S *respCode)
{
    log_i("before adm_uim_hmi_ecu_installResult_RR");

    const struct _adm_uim_impls *pImpls = adm_uim_getImpls();
	if(NULL == pImpls || NULL == pImpls->adm_uis_hmi_ecu_installResult_RR_impl)
	{
		log_e("_adm_uim_impls or adm_uis_hmi_ecu_installResult_RR_impl pointer invalid");
		return ;
	}
    (pImpls->adm_uis_hmi_ecu_installResult_RR_impl)(eCmdidType, sessionId,state,pstEcuIdenInfo,pstEcuInstallResultInfo,respCode);

    log_i("Finish adm_uim_hmi_ecu_installResult_RR");
    return;
}

void adm_uim_hmi_allecu_installResult_RR(const ADM_APPLICATION_COMMON_CMDID_TYPE_E eCmdidType,const DCHAR *sessionId,const DCHAR *state,
                const ADM_UIM_SOA_ECU_INSTALL_RESULT_ARRAY_INFO_T *pstEcuInstallResultArrayInfo,const ADM_UIM_SOA_INSTALL_RESULT_INFO_T *pstPolicyInstallResult,D32S *respCode)
{
    log_i("before adm_uim_hmi_allecu_hmi_installResult_RR");

    const struct _adm_uim_impls *pImpls = adm_uim_getImpls();
	if(NULL == pImpls || NULL == pImpls->adm_uis_hmi_allecu_installResult_RR_impl)
	{
		log_e("_adm_uim_impls or adm_uis_hmi_allecu_installResult_RR_impl pointer invalid");
		return ;
	}
    (pImpls->adm_uis_hmi_allecu_installResult_RR_impl)(eCmdidType, sessionId,state, pstEcuInstallResultArrayInfo,pstPolicyInstallResult,respCode);

    log_i("Finish adm_uim_hmi_allecu_hmi_installResult_RR");
    return;
}
