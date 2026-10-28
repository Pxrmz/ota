#include "adm_com_errorCode.h"
#include "adm_uim_log.h"
#include "adm_uim_ems.h"
#include "adm_memory.h"
#include "adm_soa.h"
#include "adm_uim_api.h"
#include "adm_uim_apiImpl.h"

D32S adm_uim_emsInit(const ADM_UIM_EMS_INIT_INFO_T* initInfo, void* priv)
{
    log_i("adm_uim_emsInit begin");
	D32S retCode = ADM_ECODE_FAILED;

    const struct _adm_uim_impls* pImpls = adm_uim_getImpls();
	if(NULL == pImpls || NULL == pImpls->adm_uis_emsInit_impl)
	{
		log_e("_adm_uim_impls or adm_uis_emsInit_impl pointer invalid, nRet[ %d ]", retCode);
		return retCode;
	}
    retCode =  (pImpls->adm_uis_emsInit_impl)(initInfo, priv);
    log_i("adm_uim_emsInit end, %d", retCode);
    return retCode;
}

D32S adm_uim_emsDeinit(void)
{
    log_i("adm_uim_emsDeinit begin");
	D32S retCode = ADM_ECODE_FAILED;
    const struct _adm_uim_impls* pImpls = adm_uim_getImpls();
	if(NULL == pImpls || NULL == pImpls->adm_uis_emsDeinit_impl)
	{
		log_e("_adm_uim_impls or adm_uis_emsDeinit_impl pointer invalid, nRet[ %d ]", retCode);
		return retCode;
	}
    retCode = (pImpls->adm_uis_emsDeinit_impl)();
    log_i("adm_uim_emsDeinit end, %d", retCode);
    return retCode;
}

