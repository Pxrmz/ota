#include "adm_uim_log.h"

#include "adm_com_errorCode.h"
#include "adm_memory.h"
#include "adm_soa.h"

#include "adm_uim_api.h"
#include "adm_uim_apiImpl.h"


D32S adm_uim_installInit(const ADM_UIM_INSTALL_INIT_INFO_T *initInfo,
                         void *priv)
{
    log_i("adm_uim_installInit begin");
    D32S retCode = ADM_ECODE_FAILED;
    const struct _adm_uim_impls *pImpls = adm_uim_getImpls();
	if(NULL == pImpls || NULL == pImpls->adm_uis_installInit_impl)
	{
		log_e("_adm_uim_impls or adm_uis_installInit_impl pointer invalid, nRet[ %d ]", retCode);
		return retCode;
	}
    retCode = (pImpls->adm_uis_installInit_impl)(initInfo, priv);
    log_i("adm_uim_installInit end, %d", retCode);
    return retCode;
}


D32S adm_uim_installDeinit(void)
{
    log_i("adm_uim_installDeinit begin");
    D32S retCode = ADM_ECODE_FAILED;
    const struct _adm_uim_impls *pImpls = adm_uim_getImpls();
	if(NULL == pImpls || NULL == pImpls->adm_uis_installDeinit_impl)
	{
		log_e("_adm_uim_impls or adm_uis_installDeinit_impl pointer invalid, nRet[ %d ]", retCode);
		return retCode;
	}
    retCode = (pImpls->adm_uis_installDeinit_impl)();
    log_i("adm_uim_installDeinit end, %d", retCode);
    return retCode;
}

D32S adm_uim_apkReadyNoticeInit(const ADM_UIM_APK_READY_INIT_INFO_T *initInfo,
                         void *priv)
{
    log_i("adm_uim_apkReadyNoticeInit begin");
    D32S retCode = ADM_ECODE_FAILED;
    const struct _adm_uim_impls *pImpls = adm_uim_getImpls();
	if(NULL == pImpls || NULL == pImpls->adm_uis_apkReadyNoticeInit_impl)
	{
		log_e("_adm_uim_impls or adm_uis_apkReadyNoticeInit_impl pointer invalid, nRet[ %d ]", retCode);
		return retCode;
	}
    retCode = (pImpls->adm_uis_apkReadyNoticeInit_impl)(initInfo, priv);
    log_i("adm_uim_apkReadyNoticeInit end, %d", retCode);
    return retCode;
}

D32S adm_uim_apkReadyNoticeDeinit(void)
{
    log_i("adm_uim_apkReadyNoticeDeinit begin");
    D32S retCode = ADM_ECODE_FAILED;
    const struct _adm_uim_impls *pImpls = adm_uim_getImpls();
	if(NULL == pImpls || NULL == pImpls->adm_uis_apkReadyNoticeDeinit_impl)
	{
		log_e("_adm_uim_impls or adm_uis_apkReadyNoticeDeinit_impl pointer invalid, nRet[ %d ]", retCode);
		return retCode;
	}
    retCode = (pImpls->adm_uis_apkReadyNoticeDeinit_impl)();
    log_i("adm_uim_apkReadyNoticeDeinit end, %d", retCode);
    return retCode;
}

void adm_uim_schduleTimeArrived(DBOOL bCondSatisfy, DBOOL bIsTaskStop)
{
    log_i("adm_uim_soa_schduleTimeArrived begin");
    const struct _adm_uim_impls *pImpls = adm_uim_getImpls();
	if(NULL == pImpls || NULL == pImpls->adm_uis_schduleTimeArrived_impl)
	{
		log_e("_adm_uim_impls or adm_uis_schduleTimeArrived_impl pointer invalid");
		return ;
	}
    (pImpls->adm_uis_schduleTimeArrived_impl)(bCondSatisfy, bIsTaskStop);
    log_i("adm_uim_soa_schduleTimeArrived end");
}


void adm_uim_tspSchduleTime(ADM_UIM_SOA_TSP_OPERATION_TYPE_E eOperationType, D64S appointmentTimestamp)
{
    log_i("adm_uim_tspSchduleTime begin");
    const struct _adm_uim_impls * pImpls = adm_uim_getImpls();
    if (NULL == pImpls || NULL == pImpls->adm_uis_tspSchduleTimestamp_impl)
    {
		log_e("_adm_uim_impls or adm_uis_tspSchduleTimestamp_impl pointer invalid");
		return ;
    }

    (pImpls->adm_uis_tspSchduleTimestamp_impl)(eOperationType, appointmentTimestamp);
    
    log_i("adm_uim_tspSchduleTime end");
}

D32S adm_uim_installConditionsVerifyResultNotify(const
        ADM_APPLICATION_COMMON_CMDID_TYPE_E eCmdidType,
        const DCHAR *sessionId,
        const DCHAR *state,
        const ADM_UIM_SOA_CONDITION_DATA_ARRAY_T *condsArrarList,
        const D32S verifyResult)
{
    log_i("adm_uim_installConditionsVerifyResultNotify begin");
    D32S retCode = ADM_ECODE_FAILED;
    const struct _adm_uim_impls *pImpls = adm_uim_getImpls();
	if(NULL == pImpls || NULL == pImpls->adm_uis_installConditionsVerifyResultNotify_impl)
	{
		log_e("_adm_uim_impls or adm_uis_installConditionsVerifyResultNotify_impl pointer invalid, nRet[ %d ]", retCode);
		return retCode;
	}
    retCode = (pImpls->adm_uis_installConditionsVerifyResultNotify_impl)(eCmdidType,
              sessionId, state, condsArrarList, verifyResult);

    log_i("adm_uim_installConditionsVerifyResultNotify end, retcode:%d", retCode);
    return retCode;
}

D32S adm_uim_policyInstallProgressNotify(const
        ADM_APPLICATION_COMMON_CMDID_TYPE_E eCmdidType,
        const DCHAR *sessionId,
        const DCHAR *state,
        const ADM_UIM_SOA_INSTALL_PROGRESS_INFO_T *pstPolicyInstallProgress)
{
    log_i("adm_uim_policyInstallProgressNotify begin");
    D32S retCode = ADM_ECODE_FAILED;
    const struct _adm_uim_impls *pImpls = adm_uim_getImpls();
	if(NULL == pImpls || NULL == pImpls->adm_uis_policyInstallProgressNotify_impl)
	{
		log_e("_adm_uim_impls or adm_uis_policyInstallProgressNotify_impl pointer invalid, nRet[ %d ]", retCode);
		return retCode;
	}
    retCode = (pImpls->adm_uis_policyInstallProgressNotify_impl)(eCmdidType,
              sessionId, state, pstPolicyInstallProgress);
    log_i("adm_uim_policyInstallProgressNotify end, retcode:%d", retCode);
    return retCode;
}

D32S adm_uim_policyInstallResultNotify(const ADM_APPLICATION_COMMON_CMDID_TYPE_E
                                       eCmdidType,
                                       const DCHAR *sessionId,
                                       const DCHAR *state,
                                       const ADM_UIM_SOA_INSTALL_RESULT_INFO_T *pstPolicyInstallResult)
{
    log_i("adm_uim_policyInstallResultNotify begin");
    D32S retCode = ADM_ECODE_FAILED;
    const struct _adm_uim_impls *pImpls = adm_uim_getImpls();
	if(NULL == pImpls || NULL == pImpls->adm_uis_policyInstallResultNotify_impl)
	{
		log_e("_adm_uim_impls or adm_uis_policyInstallResultNotify_impl pointer invalid, nRet[ %d ]", retCode);
		return retCode;
	}
    retCode = (pImpls->adm_uis_policyInstallResultNotify_impl)(eCmdidType,
              sessionId, state, pstPolicyInstallResult);
    log_i("adm_uim_policyInstallResultNotify end, retcode:%d", retCode);
    return retCode;
}

D32S adm_uim_ecuInstallProgressNotify(const ADM_APPLICATION_COMMON_CMDID_TYPE_E
                                      eCmdidType,
                                      const DCHAR *sessionId,
                                      const DCHAR *state,
                                      const ADM_UIM_SOA_ECU_IDEN_INFO_T *pstEcuIdenInfo,
                                      const ADM_UIM_SOA_INSTALL_PROGRESS_INFO_T *pstEcuInstallProgressInfo)
{
    log_i("adm_uim_ecuInstallProgressNotify begin");
    D32S retCode = ADM_ECODE_FAILED;
    const struct _adm_uim_impls *pImpls = adm_uim_getImpls();
	if(NULL == pImpls || NULL == pImpls->adm_uis_ecuInstallProgressNotify_impl)
	{
		log_e("_adm_uim_impls or adm_uis_ecuInstallProgressNotify_impl pointer invalid, nRet[ %d ]", retCode);
		return retCode;
	}
    retCode = (pImpls->adm_uis_ecuInstallProgressNotify_impl)(eCmdidType, sessionId,
              state, pstEcuIdenInfo, pstEcuInstallProgressInfo);
    log_i("adm_uim_ecuInstallProgressNotify end, retcode:%d", retCode);
    return retCode;
}

D32S adm_uim_ecuInstallResultNotify(const ADM_APPLICATION_COMMON_CMDID_TYPE_E
                                    eCmdidType,
                                    const DCHAR *sessionId,
                                    const DCHAR *state,
                                    const ADM_UIM_SOA_ECU_IDEN_INFO_T *pstEcuIdenInfo,
                                    const ADM_UIM_SOA_INSTALL_RESULT_INFO_T *pstEcuInstallResultInfo)
{
    log_i("adm_uim_ecuInstallResultNotify begin");
    D32S retCode = ADM_ECODE_FAILED;
    const struct _adm_uim_impls *pImpls = adm_uim_getImpls();
	if(NULL == pImpls || NULL == pImpls->adm_uis_ecuInstallResultNotify_impl)
	{
		log_e("_adm_uim_impls or adm_uis_ecuInstallResultNotify_impl pointer invalid, nRet[ %d ]", retCode);
		return retCode;
	}
    retCode = (pImpls->adm_uis_ecuInstallResultNotify_impl)(eCmdidType, sessionId,
              state, pstEcuIdenInfo, pstEcuInstallResultInfo);
    log_i("adm_uim_ecuInstallResultNotify end, retcode:%d", retCode);
    return retCode;
}

D32S adm_uim_allEcuInstallProgressNotify(const
        ADM_APPLICATION_COMMON_CMDID_TYPE_E eCmdidType,
        const DCHAR *sessionId,
        const DCHAR *state,
        const ADM_UIM_SOA_ECU_INSTALL_PROGRESS_ARRAY_INFO_T
        *pstEcuInstallProgressArrayInfo,
        const ADM_UIM_SOA_INSTALL_PROGRESS_INFO_T *pstPolicyInstallProgress)
{
    log_i("adm_uim_allEcuInstallProgressNotify begin");
    D32S retCode = ADM_ECODE_FAILED;
    const struct _adm_uim_impls *pImpls = adm_uim_getImpls();
	if(NULL == pImpls || NULL == pImpls->adm_uis_allEcuInstallProgressNotify_impl)
	{
		log_e("_adm_uim_impls or adm_uis_allEcuInstallProgressNotify_impl pointer invalid, nRet[ %d ]", retCode);
		return retCode;
	}
    retCode = (pImpls->adm_uis_allEcuInstallProgressNotify_impl)(eCmdidType,
              sessionId, state, pstEcuInstallProgressArrayInfo, pstPolicyInstallProgress);
    log_i("adm_uim_allEcuInstallProgressNotify end, retcode:%d", retCode);
    return retCode;
}

D32S adm_uim_allEcuInstallResultNotify(const ADM_APPLICATION_COMMON_CMDID_TYPE_E
                                       eCmdidType,
                                       const DCHAR *sessionId,
                                       const DCHAR *state,
                                       const ADM_UIM_SOA_ECU_INSTALL_RESULT_ARRAY_INFO_T *pstEcuInstallResultArrayInfo,
                                       const ADM_UIM_SOA_INSTALL_RESULT_INFO_T *pstPolicyInstallResult)
{
    log_i("adm_uim_allEcuInstallResultNotify begin");
    D32S retCode = ADM_ECODE_FAILED;
    const struct _adm_uim_impls *pImpls = adm_uim_getImpls();
	if(NULL == pImpls || NULL == pImpls->adm_uis_allEcuInstallResultNotify_impl)
	{
		log_e("_adm_uim_impls or adm_uis_allEcuInstallResultNotify_impl pointer invalid, nRet[ %d ]", retCode);
		return retCode;
	}
    retCode = (pImpls->adm_uis_allEcuInstallResultNotify_impl)(eCmdidType,
              sessionId, state, pstEcuInstallResultArrayInfo, pstPolicyInstallResult);
    log_i("adm_uim_allEcuInstallResultNotify end, retcode:%d", retCode);
    return retCode;
}
