#include "adm_com_errorCode.h"
#include "adm_soa_log.h"
#include "adm_soa.h"
#include "adm_soa_apiimpl.h"

#include "adm_memory.h"
#include "adm_cfg_api.h"
#include "adm_soa_impl.h"
#include "adm_soa_cfg.h"
#include "adm_string.h"
#include <stdlib.h>
#include <dlfcn.h>
#include <unistd.h>



static ADM_SOA_SESSION_T* soaSession = NULL;
static soa_func_list_t* soa_func_list;
static void *handle = NULL;

static D16U hostInstanceId= 0;
static D16U hostTboxInstanceId = 0;
static D16U vdiInstanceId= 0;
static D16U uclInstanceId= 0;
static D16U uclAdccInstanceId = 0;
static D16U transInstanceId= 0;
static D16U dpoInstanceId= 0;
static D16U uimInstanceId= 0;
static D16U hmiInstanceId= 0;
static D16U idelTransInstanceId = 0;

			
#ifdef __DLSYM__
	#define SOA_DLSYM(type, member)                                                         \
            do {                                                                        \
                char *error = NULL;                                                     \
                soa_func_list->member = (type)dlsym(handle , #member);   \
                if((error = dlerror()) != NULL) {                                       \
                    log_e("%s", error);                                                 \
                    return ADM_ECODE_FAILED;                                                           \
                }                                                                       \
            } while(0)
#else
	#define SOA_DLSYM(type, member)	soa_func_list->member = (type)member;
#endif



ADM_SOA_SESSION_T* adm_soa_getSession(void)
{	
	return soaSession;
}

D16U adm_soa_getVdiInstanceId(void)
{
	return vdiInstanceId;
}

D16U adm_soa_getUclInstanceId(void)
{
	return uclInstanceId;
}

D16U adm_soa_getHostInstanceId(void)
{
	return hostInstanceId;
}

D16U adm_soa_getTransInstanceId(void)
{
	return transInstanceId;
}

D16U adm_soa_getDpoInstanceId(void)
{
	return dpoInstanceId;
}

D16U adm_soa_getHmiInstanceId(void)
{
	return hmiInstanceId;
}

D16U  adm_soa_getUimInstanceId(void)
{
	return uimInstanceId;
}

D16U adm_soa_getHostTboxInstanceId(void)
{
    if (0 == hostTboxInstanceId)
    {
        hostTboxInstanceId = SYSI4UCL2TBOX_INSTANCE_ID;
    }
    return hostTboxInstanceId;
}

D16U adm_soa_getUclAdccInstanceId(void)
{
    // if (0 == uclAdccInstanceId)
    // {
    //     uclAdccInstanceId = SYSI4UCL2CGW_INSTANCE_ID;
    // }

    return uclAdccInstanceId;
}

D16U adm_soa_getIdelTransInstanceId(void)
{
    return idelTransInstanceId;
}


static D32S adm_soa_initFuncList(void)
{
	if(soa_func_list == NULL)
	{
		soa_func_list = (soa_func_list_t*)adm_malloc(sizeof(soa_func_list_t));
		if(soa_func_list == NULL)
		{
			log_e("malloc init func list fail");
 			return ADM_ECODE_FAILED;
		}
	}
	
	SOA_DLSYM(adm_soa_init_func	 , adm_dis_soa_init_impl	);								
	SOA_DLSYM(adm_soa_start_func , adm_dis_soa_start_impl	); 								
	SOA_DLSYM(adm_soa_deinit_func, adm_dis_soa_deinit_impl 	);						

	return ADM_ECODE_SUCCESSED;
}

ADM_SOA_SESSION_T* adm_soa_init(void* priv)
{
	log_i("adm_soa_init begin");

	if(NULL != soaSession)
	{
		return soaSession;
	}

	
	D32S soaType = adm_cfg_iniParserGetInt(ADM_CFG_VUS_SOA_IMPL_TYPE,  ADM_CFG_VUS_SOA_IMPL_TYPE_DEFAULT_VAL);

#ifdef __DLSYM__

	// 加载so
	const DCHAR* soaImplLibPath = adm_cfg_iniParserGetString(ADM_CFG_VUS_SOA_IMPL_LIB_PATH,  ADM_CFG_VUS_SOA_IMPL_LIB_PATH_DEFAULT_VAL);
		
	// 读取函数指针
	if(soaImplLibPath == NULL)
	{
		log_e("invalid param [ soa_impl_libaray_path ]");
		return NULL;
	}
    log_i("soa_impl_libaray_path[ %s ]", soaImplLibPath);

	handle = dlopen(soaImplLibPath, RTLD_NOW);
	if(!handle)
    {
		log_e("%s", dlerror());
		return NULL;
	}

#endif

	D32S nRet = adm_soa_initFuncList();
	if(nRet != ADM_ECODE_SUCCESSED) 
	{
        adm_soa_deinit();
		return NULL;	
	}
	
	if(soa_func_list == NULL || soa_func_list->adm_dis_soa_init_impl == NULL)
	{
		log_e("func or adm_soa_init_handle pointer invalid");
		return NULL;
	}
	
	// uim, hmi 有指定instanceId, 需要传给arpc. uim上层appcore传入
// #ifdef __CUSTOM_HMI_CLIENT__
	// if(ADM_SOA_IMPL_BY_ARPC == soaType)
	// {
	// 	D32S instanceId = SYSI4HMI2HUT_INTSTANCE_ID;
	// 	priv = &instanceId;
	// }
// #endif

	D32S iWaitInterval = adm_cfg_iniParserGetInt(ADM_CFG_VUS_WAIT_NETWORK_OK_SEC,  ADM_CFG_VUS_WAIT_NETWORK_OK_SEC_DEFAULT_VAL);
	D32S iWaitCount = adm_cfg_iniParserGetInt(ADM_CFG_VUS_WAIT_NETWORK_OK_COUNT,  ADM_CFG_VUS_WAIT_NETWORK_OK_COUNT_DEFAULT_VAL);
	D32S i = 0;
	do
	{
		i++;
		log_i("wait network ok, try count:%d", i);
		soaSession = soa_func_list->adm_dis_soa_init_impl(priv);
		if(NULL != soaSession)
		{
			break;
		}
		(void)sleep((D32U)iWaitInterval);
	}while(i<iWaitCount);	
	
	// soa 实例加载
	hostInstanceId = (D16U)adm_cfg_iniParserGetInt(ADM_CFG_VUS_HOST_INSTANCEID,  ADM_CFG_VUS_HOST_INSTANCEID_DEFAULT_VAL);
	vdiInstanceId = (D16U)adm_cfg_iniParserGetInt(ADM_CFG_VUS_VDI_INSTANCEID,  ADM_CFG_VUS_VDI_INSTANCEID_DEFAULT_VAL);
	uclInstanceId = (D16U)adm_cfg_iniParserGetInt(ADM_CFG_VUS_UCL_INSTANCEID,  ADM_CFG_VUS_UCL_INSTANCEID_DEFAULT_VAL);
	transInstanceId = (D16U)adm_cfg_iniParserGetInt(ADM_CFG_VUS_TRANS_INSTANCEID,  ADM_CFG_VUS_TRANS_INSTANCEID_DEFAULT_VAL);
	dpoInstanceId = (D16U)adm_cfg_iniParserGetInt(ADM_CFG_VUS_DPO_INSTANCEID,  ADM_CFG_VUS_DPO_INSTANCEID_DEFAULT_VAL);
	uimInstanceId = (D16U)adm_cfg_iniParserGetInt(ADM_CFG_VUS_UIM_INSTANCEID,  ADM_CFG_VUS_UIM_INSTANCEID_DEFAULT_VAL);
	hmiInstanceId = (D16U)adm_cfg_iniParserGetInt(ADM_CFG_VUS_HMI_INSTANCEID,  ADM_CFG_VUS_HMI_INSTANCEID_DEFAULT_VAL);
    idelTransInstanceId = (D16U)adm_cfg_iniParserGetInt(ADM_CFG_VUS_IDEL_TRNAS_INSTANCEID, ADM_CFG_VUS_IDEL_TRNAS_INSTANCEID_DEFAULT_VAL);

    DCHAR* cfgInstances = adm_strdup(adm_cfg_iniParserGetString(ADM_CFG_VUS_HOST_INSTANCEID,  ""));
    if (NULL != cfgInstances)
    {
        DCHAR* subInstanceIdStr = NULL;
        subInstanceIdStr = strsep(&cfgInstances, ",");
        log_i("subInstanceIdStr:%s", subInstanceIdStr);

        /** 获取host配置中获取tbox的host实例 */
        for (D32S i =0; NULL != subInstanceIdStr; i++)
        {
            /** 第二个host实例 */
            if(i == 2)
            {
                break;
            }
            log_i("subInstanceIdStr:%s\n", subInstanceIdStr);
            hostTboxInstanceId = atoi(subInstanceIdStr);
            subInstanceIdStr = strsep(&cfgInstances, ",");
        }
    }
    adm_free(cfgInstances);

    // cfgInstances = adm_strdup(adm_cfg_iniParserGetString(ADM_CFG_VUS_UCL_INSTANCEID,  ""));
    // if (NULL != cfgInstances)
    // {
    //     DCHAR* subInstanceIdStr = NULL;
    //     subInstanceIdStr = strsep(&cfgInstances, ",");
    //     log_i("subInstanceIdStr:%s", subInstanceIdStr);

    //     /** 获取host配置中获取tbox的host实例 */
    //     for (D32S i =0; NULL != subInstanceIdStr; i++)
    //     {
    //         /** 第三个host实例 */
    //         if(i == 3)
    //         {
    //             break;
    //         }
    //         log_i("subInstanceIdStr:%s\n", subInstanceIdStr);
    //         uclAdccInstanceId = atoi(subInstanceIdStr);
    //         subInstanceIdStr = strsep(&cfgInstances, ",");
    //     }
    // }
    // adm_free(cfgInstances);
	
	log_i("adm_soa_init end");

	return soaSession;

}

D32S adm_soa_start(void)
{
	log_i("adm_soa_start begin");

	D32S nRet = ADM_ECODE_FAILED;
	if(soa_func_list == NULL || soa_func_list->adm_dis_soa_start_impl == NULL)
	{
		nRet = ADM_ECODE_INVALID_POINTER;
		log_e("func or adm_dis_soa_start_impl pointer invalid, nRet[ %d ]", nRet);
		return nRet;
	}

	nRet = soa_func_list->adm_dis_soa_start_impl(soaSession);
	
	log_i("adm_soa_start end");
	
	return nRet;
}


void adm_soa_deinit(void)
{
	log_i("adm_soa_deinit begin");

	if(NULL == soaSession)
	{
		log_i("adm_soa_deinit end");
		return ;
	}
	
	if(soa_func_list == NULL || soa_func_list->adm_dis_soa_deinit_impl == NULL)
	{
		
		log_e("func or adm_dis_soa_deinit_impl pointer invalid" );
		return ;
	}

	soa_func_list->adm_dis_soa_deinit_impl(soaSession);

	soaSession = NULL;
	
	log_i("adm_soa_deinit end");
}

