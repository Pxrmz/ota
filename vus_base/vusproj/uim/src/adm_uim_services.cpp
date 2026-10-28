#include "adm_uim_log.h"

#include "adm_uim_services.h"
#include "adm_memory.h"
#include "adm_com_errorCode.h"

#include "adm_soa.h"

#include "adm_uim_api.h"
#include "adm_uim_apiImpl.h"

D32S adm_uim_serviceInit(void* priv)
{
	log_i("adm_uim_serviceInit begin");
	D32S retCode = ADM_ECODE_FAILED;
    const struct _adm_uim_impls* pImpls = adm_uim_getImpls();
	if(NULL == pImpls || NULL == pImpls->adm_uis_serviceInit_impl)
	{
		log_e("_adm_uim_impls or adm_uis_serviceInit_impl pointer invalid, nRet[ %d ]", retCode);
		return retCode;
	}
    retCode = (pImpls->adm_uis_serviceInit_impl)(priv);
    log_i("adm_uim_serviceInit end, retCode:%d", retCode);
    return retCode;
}

D32S adm_uim_serviceDeinit(void)
{
	log_i("start adm_uim_serviceDeinit begin");
	D32S retCode = ADM_ECODE_FAILED;
    const struct _adm_uim_impls* pImpls = adm_uim_getImpls();
	if(NULL == pImpls || NULL == pImpls->adm_uis_serviceDeinit_impl)
	{
		log_e("_adm_uim_impls or adm_uis_serviceDeinit_impl pointer invalid, nRet[ %d ]", retCode);
		return retCode;
	}
    retCode = (pImpls->adm_uis_serviceDeinit_impl)();
    log_i("start adm_uim_serviceDeinit end, retCode:%d", retCode);
    return retCode;
}
