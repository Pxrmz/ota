#include "adm_uim_log.h"

#include "adm_uim_services.h"
#include "adm_memory.h"
#include "adm_com_errorCode.h"

#include "adm_soa.h"

#include "adm_uim_api.h"
#include "adm_uim_apiImpl.h"

void adm_uim_vsmStateNotify(const D32S state)
{
	log_i("adm_uim_vsmStateNotify begin");
    const struct _adm_uim_impls* pImpls = adm_uim_getImpls();
	if(NULL == pImpls || NULL == pImpls->adm_uis_vsmStateNotify_impl)
	{
		log_e("_adm_uim_impls or adm_uis_vsmStateNotify_impl pointer invalid");
		return ;
	}
    (pImpls->adm_uis_vsmStateNotify_impl)(state);
    log_i("adm_uim_vsmStateNotify end");
    return;
}
