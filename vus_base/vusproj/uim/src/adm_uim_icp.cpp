#include "adm_com_errorCode.h"
#include "adm_uim_log.h"
#include "adm_uim_icp.h"

#include "adm_memory.h"
#include "adm_soa.h"

#include "adm_uim_api.h"
#include "adm_uim_apiImpl.h"


/**
 * @brief The SOA-UIM-ICP module initializes the interface 
 */
D32S adm_uim_icpInit(const ADM_UIM_ICP_INIT_INFO_T* initInfo, void* priv)
{
	log_i("adm_uim_icpInit begin");
	D32S retCode = ADM_ECODE_FAILED;
    const struct _adm_uim_impls* pImpls = adm_uim_getImpls();
	if(NULL == pImpls || NULL == pImpls->adm_uis_icpInit_impl)
	{
		log_e("_adm_uim_impls or adm_uis_icpInit_impl pointer invalid, nRet[ %d ]", retCode);
		return retCode;
	}
    retCode =  (pImpls->adm_uis_icpInit_impl)(initInfo, priv);
    log_i("adm_uim_icpInit end, %d", retCode);
    return retCode;
}

/**
 * @brief Release initialization information
 */
D32S adm_uim_icpDeinit(void)
{
	log_i("adm_uim_icpDeinit begin");
	D32S retCode = ADM_ECODE_FAILED;
    const struct _adm_uim_impls* pImpls = adm_uim_getImpls();
	if(NULL == pImpls || NULL == pImpls->adm_uis_icpDeinit_impl)
	{
		log_e("_adm_uim_impls or adm_uis_icpDeinit_impl pointer invalid, nRet[ %d ]", retCode);
		return retCode;
	}
    retCode = (pImpls->adm_uis_icpDeinit_impl)();
    log_i("adm_uim_icpDeinit end, %d", retCode);
    return retCode;
}

