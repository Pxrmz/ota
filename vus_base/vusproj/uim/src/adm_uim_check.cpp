#include "adm_com_errorCode.h"
#include "adm_uim_log.h"
#include "adm_uim_check.h"

#include "adm_memory.h"
#include "adm_soa.h"

#include "adm_uim_api.h"
#include "adm_uim_apiImpl.h"

D32S adm_uim_checkInit(const ADM_UIM_CHECK_INIT_INFO_T* initInfo, void* priv)
{
	log_i("adm_uim_checkInit begin");
	D32S retCode = ADM_ECODE_FAILED;
    const struct _adm_uim_impls* pImpls = adm_uim_getImpls();
	if(NULL == pImpls || NULL == pImpls->adm_uis_checkInit_impl)
	{
		log_e("_adm_uim_impls or adm_uis_checkInit_impl pointer invalid, nRet[ %d ]", retCode);
		return retCode;
	}
    retCode =  (pImpls->adm_uis_checkInit_impl)(initInfo, priv);
    log_i("adm_uim_checkInit end, %d", retCode);
    return retCode;
}

D32S adm_uim_checkDeinit(void)
{
	log_i("adm_uim_checkDeinit begin");
	D32S retCode = ADM_ECODE_FAILED;
    const struct _adm_uim_impls* pImpls = adm_uim_getImpls();
	if(NULL == pImpls || NULL == pImpls->adm_uis_checkDeinit_impl)
	{
		log_e("_adm_uim_impls or adm_uis_checkDeinit_impl pointer invalid, nRet[ %d ]", retCode);
		return retCode;
	}
    retCode = (pImpls->adm_uis_checkDeinit_impl)();
    log_i("adm_uim_checkDeinit end, %d", retCode);
    return retCode;
}

void adm_uim_checkResultNotify(const ADM_APPLICATION_COMMON_CMDID_TYPE_E eCmdidType, 
                                  const DCHAR* sessionId, 
                                  const DCHAR* state, 
                                  const ADM_UIM_SOA_CHECK_HMI_POLICY_INFO_T* pstPolicyInfo, 
                                  const D32S errorCode)
{
    log_i("before adm_uim_checkResultNotify");

    const struct _adm_uim_impls* pImpls = adm_uim_getImpls();
	if(NULL == pImpls || NULL == pImpls->adm_uis_checkResultNotify_impl)
	{
        log_e("_adm_uim_impls or adm_uis_checkResultNotify_impl pointer invalid");
		return ;
	}
    (pImpls->adm_uis_checkResultNotify_impl)(eCmdidType, sessionId, state, pstPolicyInfo, errorCode);
    
    log_i("after adm_uim_checkResultNotify");
    return;
}

void adm_uim_checkProgressNotify(const ADM_APPLICATION_COMMON_CMDID_TYPE_E eCmdidTYpe, 
                                    const DCHAR* state, 
                                    const ADM_APPLICATION_COMMON_CHECK_PROGRESS_TYPE_T eCheckProgressType)
{
    log_i("befroe adm_uim_checkProgressNotify");
    
    const struct _adm_uim_impls* pImpls = adm_uim_getImpls();
	if(NULL == pImpls || NULL == pImpls->adm_uis_checkProgressNotify_impl)
	{
		log_e("_adm_uim_impls or adm_uis_checkProgressNotify_impl pointer invalid");
		return ;
	}
    (pImpls->adm_uis_checkProgressNotify_impl)(eCmdidTYpe, state, eCheckProgressType);
    
    log_i("after adm_uim_checkProgressNotify");
    return ;
}
