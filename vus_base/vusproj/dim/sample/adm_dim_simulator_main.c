
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>
#include <ctype.h>
//#include <execinfo.h>     /* for backtrace() */

#include "adm_dim_log.h"

#include "adm_string.h"
#include "adm_memory.h"
#include "adm_debug.h"

//#include "adm_com_config.h"
#include "adm_tracer_interface.h"
#include "adm_com_errorCode.h"
#include "adm_soa.h"
#include "adm_cfg_api.h"
#include "adm_com_lib.h"
#include "adm_dim_api.h"

//host
#ifdef __CUSTOM_HOST__
#include "adm_dim_host.h"
#include "adm_dim_host_simulator.h"
#endif

//trans
#ifdef __CUSTOM_TRANS__
#include "adm_dim_transHost.h"
#include "adm_dim_transHost_simulator.h"
#endif

//ucl
#ifdef __CUSTOM_UCL__
#include "adm_dim_ucl.h"
#include "adm_dim_ucl_simulator.h"
#endif

//vdi
#ifdef __CUSTOM_VDI__
#include "adm_dim_vdi.h"
#include "adm_dim_vdi_simulator.h"
#endif

//dpo
#ifdef __CUSTOM_DPO__
#include "adm_dim_dpo.h"
#include "adm_dim_dpo_simulator.h"
#endif

#include "adm_dim_calHash.h"

static int s_break = 0;

static void wait_sleep(int secs)
{
    sleep(secs);
}

static void handle_signal(int _signal)
{
    if(_signal == SIGINT || _signal == SIGTERM)
    {
        log_i("\n CTL-C Break \n");
    }

    s_break = 1;
}

/**
* Print usage information of tool.
*/
static void usage()
{
    log_i("Usage: admfotaapp [options]...  [param]... \n");
    log_i("Options:\n");
    log_i("  -i  ip				unicast ip for someip\n");
    log_i("  -c  filename          fota config absolute\n");
}

int main(int argc, char* argv[])
{
    void* priv = NULL;
    D32S iRet = ADM_ECODE_FAILED;
    DCHAR* otaConfigPath = NULL;
    D32S c = 0;
    DCHAR* ip_unicast = NULL;
    ADM_SOA_SESSION_T* soaSession = NULL;

    //!< 参数个数判断
    if(1 == argc)
    {
        usage();
        return -1;    /*for parasoft */
    }

    //!< 接收入参
    while((c = getopt(argc, argv, "c:i:")) != -1)
    {
        switch(c)
        {
            case 'c' :
                otaConfigPath = optarg;
                log_i("optarg:%s\n", optarg);
                break;

            case 'i':
                ip_unicast = optarg;
                log_i("optarg:%s\n", optarg);
                break;

            case '?':
            {
                if(optopt == 'o')
                    fprintf(stderr, "Option -%c requires an argument.\n", optopt);
                else if(isprint((unsigned char)optopt))
                    fprintf(stderr, "Unknown option `-%c'.\n", optopt);
                else
                    fprintf(stderr, "Unknown option character `\\x%x'.\n", optopt);

                /* unknown or wrong option used, show usage information and terminate */
                usage();
                return -1;
            }

            default:
            {
                abort();
                return -1;    /*for parasoft */
            }
        }
    }

    do
    {
        adm_debug_logInit("/tmp/", 1045504, 5);


        D32S iRet = ADM_ECODE_FAILED;
        ADM_CFG_API_INFO_T pstApiData = {0};

        if(NULL == otaConfigPath)
        {
            iRet = ADM_ECODE_INVALID_PARAM;
            log_i("Input paramater [filename] is nil, iRet:%d\n", iRet);
            break;
        }

        pstApiData.isFileExistCb = adm_com_isFileExist;
        pstApiData.writeFileDataCb = adm_com_writeFileData;
        iRet = adm_cfg_iniParserInit(otaConfigPath, &pstApiData);

        if(ADM_ECODE_SUCCESSED != iRet)
        {
            log_i("adm_cfg_iniParserInit %s failed, iRet:%d\n", otaConfigPath, iRet);
            break;
        }
        else
        {
            log_i("adm_cfg_iniParserInit %s successed", otaConfigPath);
        }

        /**************************************************************************************************
        */

        //SOA init
        log_i("====>Initializing SOA ...\n");
        soaSession = adm_soa_init(NULL);
        log_i("<====Initializing soa end\n");


        //dim loader
        adm_dim_init(NULL);

        //SOA launch
        log_i("====>Initializing Launch ...\n");
        adm_soa_start();
        log_i("====>Initializing Launch end\n");


        //sleep(10);

        /**************************************************************************************************
        */
        //host service init
#ifdef __CUSTOM_HOST__
        adm_host_simulatorInit();
#endif
        // trans service init
#ifdef __CUSTOM_TRANS__
        adm_transHost_simulatorInit();
#endif
        // ucl service init
#ifdef __CUSTOM_UCL__
        adm_ucl_simulatorInit();
#endif
        //vdi service init
#ifdef __CUSTOM_VDI__
        adm_vdi_simulatorInit();
#endif
        //dpo
#ifdef __CUSTOM_DPO__
        adm_dpo_simulatorInit();
#endif
        /**************************************************************************************************
        */

        /**************************************************************************************************
        */

        wait_sleep(30);
        log_i("after sleep");

        //host service exec
#ifdef __CUSTOM_HOST__
        adm_host_simulatorExec();
#endif
        // trans service exec
#ifdef __CUSTOM_TRANS__
        //adm_transHost_simulatorExec();
#endif
        // ucl service exec
#ifdef __CUSTOM_UCL__
        //adm_ucl_simulatorExec();
#endif
        //vdi service exec
#ifdef __CUSTOM_VDI__
        //adm_vdi_simulatorExec();
#endif

#ifdef __CUSTOM_DPO__
        //adm_dpo_simulatorExec();
#endif

    //计算hash测试
    //calHash();

    }
    while(0);


    /**************************************************************************************************
    */

    //sleep and break
    for(int i = 0; !s_break; i++)
    {
        log_i("wait...........");
        wait_sleep(5);
    }

    /**************************************************************************************************
    */
    //host service deinit
#ifdef __CUSTOM_HOST__
    adm_host_simulatorDeinit();
#endif
    // trans service deinit
#ifdef __CUSTOM_TRANS__
    adm_transHost_simulatorDeinit();
#endif
    // ucl service deinit
#ifdef __CUSTOM_UCL__
    adm_ucl_simulatorDeinit();
#endif
    //vdi service deinit
#ifdef __CUSTOM_VDI__
    adm_vdi_simulatorDeinit();
#endif
#ifdef __CUSTOM_DPO__
    adm_dpo_simulatorDeinit();
#endif

    adm_soa_deinit();

    log_i("main exit\r\n");

    adm_debug_logDeinit();

    return 0;
}

