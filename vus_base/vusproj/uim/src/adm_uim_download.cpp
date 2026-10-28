#include "adm_uim_log.h"

#include "adm_com_errorCode.h"
#include "adm_memory.h"
#include "adm_soa.h"

#include "adm_uim_api.h"
#include "adm_uim_apiImpl.h"


D32S adm_uim_downloadInit(const ADM_UIM_DOWNLOAD_INIT_INFO_T *initInfo,
                          void *priv)
{
    log_i("adm_uim_downloadInit begin");
    D32S retCode = ADM_ECODE_FAILED;
    const struct _adm_uim_impls *pImpls = adm_uim_getImpls();
	if(NULL == pImpls || NULL == pImpls->adm_uis_downloadInit_impl)
	{
		log_e("_adm_uim_impls or adm_uis_downloadInit_impl pointer invalid, nRet[ %d ]", retCode);
		return retCode;
	}
    retCode = (pImpls->adm_uis_downloadInit_impl)(initInfo, priv);
    log_i("adm_uim_downloadInit end, %d", retCode);
    return retCode;

}

D32S adm_uim_downloadDeinit(void)
{
    log_i("adm_uim_downloadDeinit begin");
    D32S retCode = ADM_ECODE_FAILED;
    const struct _adm_uim_impls *pImpls = adm_uim_getImpls();
	if(NULL == pImpls || NULL == pImpls->adm_uis_downloadDeinit_impl)
	{
		log_e("_adm_uim_impls or adm_uis_downloadDeinit_impl pointer invalid, nRet[ %d ]", retCode);
		return retCode;
	}
    retCode = (pImpls->adm_uis_downloadDeinit_impl)();
    log_i("adm_uim_downloadDeinit end, %d", retCode);
    return retCode;
}

void adm_uim_downloadConditionsVerifyResultNotify(const
        ADM_APPLICATION_COMMON_CMDID_TYPE_E eCmdidType, const DCHAR *sessionId,
        const DCHAR *state,
        const ADM_UIM_SOA_CONDITION_DATA_ARRAY_T *condsArrarList,
        const D32S verifyResult)
{
    log_i("before adm_uim_downloadConditionsVerifyResultNotify");
	
    const struct _adm_uim_impls *pImpls = adm_uim_getImpls();
	if(NULL == pImpls || NULL == pImpls->adm_uis_downloadConditionsVerifyResultNotify_impl)
	{
		log_e("_adm_uim_impls or adm_uis_downloadConditionsVerifyResultNotify_impl pointer invalid");
		return ;
	}
    (pImpls->adm_uis_downloadConditionsVerifyResultNotify_impl)(eCmdidType,
            sessionId, state, condsArrarList, verifyResult);

    log_i("Finish adm_uim_downloadConditionsVerifyResultNotify");
    return;
}

void adm_uim_policyDownloadProgressNotify(const
        ADM_APPLICATION_COMMON_CMDID_TYPE_E eCmdidType,
        const DCHAR *sessionId,
        const DCHAR *state,
        const ADM_UIM_SOA_DOWNLOAD_PROGRESS_INFO_T *pstPolicyDownloadProgress)
{
    log_i("before adm_uim_policyDownloadProgressNotify");
    const struct _adm_uim_impls *pImpls = adm_uim_getImpls();
	if(NULL == pImpls || NULL == pImpls->adm_uis_policyDownloadProgressNotify_impl)
	{
		log_e("_adm_uim_impls or adm_uis_policyDownloadProgressNotify_impl pointer invalid");
		return ;
	}
    (pImpls->adm_uis_policyDownloadProgressNotify_impl)(eCmdidType, sessionId,
            state, pstPolicyDownloadProgress);

    log_i("Finish adm_uim_policyDownloadProgressNotify");
    return;
}

D32S adm_uim_policyDownloadResultNotify(const
                                        ADM_APPLICATION_COMMON_CMDID_TYPE_E eCmdidType,
                                        const DCHAR *sessionId,
                                        const DCHAR *state,
                                        const ADM_UIM_SOA_DOWNLOAD_RESULT_INFO_T *pstPolicyDownloadResult)
{
    log_i("before adm_uim_policyDownloadResultNotify");
	D32S retCode = ADM_ECODE_FAILED;
    const struct _adm_uim_impls *pImpls = adm_uim_getImpls();
	if(NULL == pImpls || NULL == pImpls->adm_uis_policyDownloadResultNotify_impl)
	{
		log_e("_adm_uim_impls or adm_uis_policyDownloadResultNotify_impl pointer invalid, nRet[ %d ]", retCode);
		return retCode;
	}
    retCode = (pImpls->adm_uis_policyDownloadResultNotify_impl)(eCmdidType, sessionId,
            state, pstPolicyDownloadResult);

    log_i("Finish adm_uim_policyDownloadResultNotify");
    return retCode;
}

D32S adm_uim_ecuDownloadProgressNotify(const ADM_APPLICATION_COMMON_CMDID_TYPE_E
                                       eCmdidType,
                                       const DCHAR *sessionId,
                                       const DCHAR *state,
                                       const ADM_UIM_SOA_ECU_IDEN_INFO_T *pstEcuIdenInfo,
                                       const ADM_UIM_SOA_DOWNLOAD_PROGRESS_INFO_T *pstEcuDownloadProgress)
{
    log_i("before adm_uim_ecuDownloadProgressNotify");
	D32S retCode = ADM_ECODE_FAILED;
    const struct _adm_uim_impls *pImpls = adm_uim_getImpls();
	if(NULL == pImpls || NULL == pImpls->adm_uis_ecuDownloadProgressNotify_impl)
	{
		log_e("_adm_uim_impls or adm_uis_ecuDownloadProgressNotify_impl pointer invalid, nRet[ %d ]", retCode);
		return retCode;
	}
    retCode = (pImpls->adm_uis_ecuDownloadProgressNotify_impl)(eCmdidType, sessionId,
            state, pstEcuIdenInfo, pstEcuDownloadProgress);

    log_i("Finish adm_uim_ecuDownloadProgressNotify");
    return retCode;
}

D32S adm_uim_ecuDownloadResultNotify(const ADM_APPLICATION_COMMON_CMDID_TYPE_E
                                     eCmdidType,
                                     const DCHAR *sessionId,
                                     const DCHAR *state,
                                     const ADM_UIM_SOA_ECU_IDEN_INFO_T *pstEcuIdenInfo,
                                     const ADM_UIM_SOA_DOWNLOAD_RESULT_INFO_T *pstEcuDownloadResult)
{
    log_i("before adm_uim_ecuDownloadResultNotify");
	D32S retCode = ADM_ECODE_FAILED;
    const struct _adm_uim_impls *pImpls = adm_uim_getImpls();
	if(NULL == pImpls || NULL == pImpls->adm_uis_ecuDownloadResultNotify_impl)
	{
		log_e("_adm_uim_impls or adm_uis_ecuDownloadResultNotify_impl pointer invalid, nRet[ %d ]", retCode);
		return retCode;
	}
    retCode = (pImpls->adm_uis_ecuDownloadResultNotify_impl)(eCmdidType, sessionId,
            state, pstEcuIdenInfo, pstEcuDownloadResult);

    log_i("Finish adm_uim_ecuDownloadResultNotify");
    return retCode;
}


D32S adm_uim_allEcuDownloadProgressNotify(const
        ADM_APPLICATION_COMMON_CMDID_TYPE_E eCmdidType,
        const DCHAR *sessionId,
        const DCHAR *state,
        const ADM_UIM_SOA_ECU_DOWNLOAD_PROGRESS_ARRAY_INFO_T
        *pstEcuDownloadProgressArrayInfo,
        const ADM_UIM_SOA_DOWNLOAD_PROGRESS_INFO_T *pstPolicyDownloadProgress)
{
    log_i("before adm_uim_allEcuDownloadProgressNotify");
	D32S retCode = ADM_ECODE_FAILED;
    const struct _adm_uim_impls *pImpls = adm_uim_getImpls();
	if(NULL == pImpls || NULL == pImpls->adm_uis_allEcuDownloadProgressNotify_impl)
	{
		log_e("_adm_uim_impls or adm_uis_allEcuDownloadProgressNotify_impl pointer invalid, nRet[ %d ]", retCode);
		return retCode;
	}
    retCode = (pImpls->adm_uis_allEcuDownloadProgressNotify_impl)(eCmdidType,
            sessionId, state, pstEcuDownloadProgressArrayInfo, pstPolicyDownloadProgress);

    log_i("Finish adm_uim_allEcuDownloadProgressNotify");
    return retCode;
}

D32S adm_uim_allEcuDownloadResultNotify(const
                                        ADM_APPLICATION_COMMON_CMDID_TYPE_E eCmdidType,
                                        const DCHAR *sessionId,
                                        const DCHAR *state,
                                        const ADM_UIM_SOA_ECU_DOWNLOAD_RESULT_ARRAY_INFO_T
                                        *pstEcuDownloadResultArrayInfo,
                                        const ADM_UIM_SOA_DOWNLOAD_RESULT_INFO_T *pstPolicyDownloadResult)
{
    log_i("before adm_uim_allEcuDownloadResultNotify");
	D32S retCode = ADM_ECODE_FAILED;
    const struct _adm_uim_impls *pImpls = adm_uim_getImpls();
	if(NULL == pImpls || NULL == pImpls->adm_uis_allEcuDownloadResultNotify_impl)
	{
		log_e("_adm_uim_impls or adm_uis_allEcuDownloadResultNotify_impl pointer invalid, nRet[ %d ]", retCode);
		return retCode;
	}
    retCode = (pImpls->adm_uis_allEcuDownloadResultNotify_impl)(eCmdidType, sessionId,
            state, pstEcuDownloadResultArrayInfo, pstPolicyDownloadResult);

    log_i("Finish adm_uim_allEcuDownloadResultNotify");
    return retCode;
}


