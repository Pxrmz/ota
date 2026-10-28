
#include "adm_typedefs.h"
#include "adm_uim_api.h"
#include "adm_uim_services.h"
#include "adm_uim_check.h"
#include "UimSoaCheckConstruction.h"
#include "UimSoaDownloadConstruction.h"
#include "UimSoaInstallConstruction.h"
#include "UimSoaRequestConstruction.h"
#include "adm_string.h"
#include "adm_com_errorCode.h"
#include "adm_memory.h"
#include "adm_uim_log.h"

#include <stdlib.h>
#include <unistd.h>
//#include <signal.h>

#include "adm_soa.h"
#include "adm_tracer_interface.h"
#include "adm_arpc_service.h"
#include "adm_cfg_api.h"
#include "adm_com_lib.h"
#include "adm_soa_cfg.h"


static ADM_UIM_CHECK_INIT_INFO_T*    m_pSoaCheck = NULL;
static ADM_UIM_DOWNLOAD_INIT_INFO_T* m_pSoaDownLoad = NULL;
static ADM_UIM_INSTALL_INIT_INFO_T*  m_pSoaInstall = NULL;

#define ADM_APC_SOA_CONFIG_PATH     "./otaconfig.ini"

static D32S s_break = 0;

/*static void handle_signal(int _signal)
{
	if (_signal == SIGINT || _signal == SIGTERM) {
        (void)printf("\n CTL-C Break \n");
        s_break = 1;
	}
}
*/

void InitCHeckSoaService(void)
{
    m_pSoaCheck = (ADM_UIM_CHECK_INIT_INFO_T*)adm_malloc(sizeof(ADM_UIM_CHECK_INIT_INFO_T));
    (void)adm_memset(m_pSoaCheck,0,sizeof(ADM_UIM_CHECK_INIT_INFO_T),sizeof(ADM_UIM_CHECK_INIT_INFO_T));
    m_pSoaCheck->checkTriggerCallback = UIMTEST_ADM_UIM_SOA_CHECK_TRIGGER_CALLBACK_F;
    m_pSoaCheck->queryResultCallback  = UIMTEST_ADM_UIM_SOA_CHECK_QUERY_RESULT_CALLBACK_F;  
    
    (void)adm_uim_checkInit(m_pSoaCheck,NULL);
    return ;
}

void InitDownLoadSoaService(void)
{
    m_pSoaDownLoad = (ADM_UIM_DOWNLOAD_INIT_INFO_T*)adm_malloc(sizeof(ADM_UIM_DOWNLOAD_INIT_INFO_T));
    (void)adm_memset(m_pSoaDownLoad,0,sizeof(ADM_UIM_DOWNLOAD_INIT_INFO_T),sizeof(ADM_UIM_DOWNLOAD_INIT_INFO_T));
    
    m_pSoaDownLoad->autoDownloadSwitchSetCallback = UIMTEST_ADM_UIM_SOA_DOWNLOAD_AUTO_CALLBACK_F;
    m_pSoaDownLoad->downloadTriggerCallback = UIMTEST_ADM_UIM_SOA_DOWNLOAD_TRIGGER_CALLBACK_F;
    m_pSoaDownLoad->downloadCancelCallback  = UIMTEST_ADM_UIM_SOA_DOWNLOAD_CALCEL_CALLBACK_F;
    m_pSoaDownLoad->downloadPauseCallback   = UIMTEST_ADM_UIM_SOA_DOWNLOAD_PAUSED_CALLBACK_F;
    m_pSoaDownLoad->conditionsVerifiedCallback = UIMTEST_ADM_UIM_SOA_DOWNLOAD_CONDITIONS_VERIFIED_CALLBACK_F;
    m_pSoaDownLoad->allEcuDownloadProgressCallback = UIMTEST_ADM_UIM_SOA_DOWNLOAD_QUERY_ALL_ECU_PROGRESS_CALLBACK_F;
    m_pSoaDownLoad->allEcuDownloadResultCallback = UIMTEST_ADM_UIM_DOWNLOAD_QUERY_ALL_ECU_RESULT_CALLBACK_F;
   
    (void)adm_uim_downloadInit(m_pSoaDownLoad,NULL);
}

void InitInstallSoaService(void)
{
    m_pSoaInstall = (ADM_UIM_INSTALL_INIT_INFO_T*)adm_malloc(sizeof(ADM_UIM_INSTALL_INIT_INFO_T));
    (void)adm_memset(m_pSoaInstall,0,sizeof(ADM_UIM_INSTALL_INIT_INFO_T),sizeof(ADM_UIM_INSTALL_INIT_INFO_T));
    m_pSoaInstall->schduleInstallCallback        =UIMTEST_ADM_UIM_SOA_INSTALL_SCHDULED_CALLBACK_F;                 
    m_pSoaInstall->cancelInstallCallback         =UIMTEST_ADM_UIM_SOA_INSTALL_CANCELD_CALLBACK_F; 
    m_pSoaInstall->refuseInstallCallback         =UIMTEST_ADM_UIM_SOA_INSTALL_REFUSED_CALLBACK_F;                  
    m_pSoaInstall->installTriggerCallback        =UIMTEST_ADM_UIM_SOA_INSTALL_TRIGGER_CALLBACK_F;                
    m_pSoaInstall->conditionsVerifiedCallback    =UIMTEST_ADM_UIM_SOA_INSTALL_CONDITIONS_VERIFIED_CALLBACK_F; 
    m_pSoaInstall->allEcuInstallProgressCallback =UIMTEST_ADM_UIM_SOA_INSTALL_QUERY_ALL_ECU_PROGRESS_CALLBACK_F;
    m_pSoaInstall->allEcuInstallResultCallback   =UIMTEST_ADM_UIM_SOA_INSTALL_ALL_ECU_RESULT_CALLBACK_F;    
    
    (void)adm_uim_installInit(m_pSoaInstall,NULL);
    return;  
}
void InitApkReadySoaService(void)
{
    return;
}

int main(int argc, DCHAR* argv[])
{
//	signal(SIGINT, handle_signal);
//    signal(SIGTERM, handle_signal);

    adm_debug_logInit("/tmp/arpc", 1*1024*1024, 5);

	DCHAR* otaConfigPath = NULL;
	D32S c = 0;
    while ((c = getopt(argc, argv, "c:")) != -1)
    {
        if(c = (int)'c')
        {
            otaConfigPath = optarg;
            log_i("optarg:%s", optarg);
        }
        else
        {
            abort ();
            return -1;    /*for parasoft */
        }    
	}        

    ADM_CFG_API_INFO_T pstApiData = {0};
    pstApiData.isFileExistCb = adm_com_isFileExist;
    pstApiData.writeFileDataCb = adm_com_writeFileData;
    D32S iRet = adm_cfg_iniParserInit(otaConfigPath, &pstApiData);
    if (ADM_ECODE_SUCCESSED != iRet) 
    {
        log_e("adm_cfg_iniParserInit %s failed, iRet:%d\n", otaConfigPath, iRet);
        return 0;
    }

//	D16U instanceId = 1;
//	if(NULL == adm_soa_init(&instanceId))
	if(NULL == adm_soa_init(NULL))
	{
		return 0;
	}

	if(ADM_ECODE_SUCCESSED != adm_soa_start())
	{
		return 0;
	}

	if(ADM_ECODE_SUCCESSED != adm_uim_init())
	{
		return 0;
	}

	(void)adm_uim_serviceInit(NULL);

    //构造check
    InitCHeckSoaService();
    //构造download
    InitDownLoadSoaService();
    //构造install
    InitInstallSoaService();
    //构造apk ready
//    InitApkReadySoaService();


    //request simul
    simulateRequsetExe();

    while(!s_break)
    {
    	log_i("sleep");
        (void)sleep(1);
    }

    (void)adm_arpc_stopService();

    adm_debug_logDeinit();
	
    return 0;
}
