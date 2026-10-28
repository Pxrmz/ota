/**
 * @file adm_apc_core.c
 * @author mengwei@abupdate.com
 * @brief OTA应用程序入口，包括：各模块初始化、注册、获取配置等流程。
 * @version 0.2
 * @date 2021-09-24
 * 
 * @copyright Copyright © 2018 All rights 上海艾拉比智能科技有限公司
 * 
 */
#include "adm_apc_coreAchieve.h"
#include "adm_apc_event.h"

#ifdef ADM_DEBUG_CVM_MODULE
    #include "adm_cvm_checkVersion.h"
#endif //ADM_DEBUG_CVM_MODULE

#ifdef SUPPORT_LION_PKI
#include "adm_oca_httpCommon.h"
#endif

 /**
 * Print usage information of tool.
 */
void usage(void)
{
    (void)printf("Usage: admfotaapp [options]...  [param]... \n");
    (void)printf("Options:\n");
    (void)printf("  -c  filename          fota config absolute\n");
    return;
}

D32S main(D32S argc, DCHAR* argv[])
{
    /**
     * TODO:
     * 0. 初始化程序启动阶段的事件上报功能
     * 1. 解析otaconfig.ini配置文件，需要修改配置路径；
     * 2. 创建OTA过程中使用的目录；
     * 3. 初始化日志模块；
     * 4. 初始化Tracer Tool；
     * 5. 初始化core service；
     * 6. 初始化车云通信接口；
     * 7. 初始化DIM；
     * 8. 初始化UIM；
     * 9. 输出各模块版本号，包括：SYSI、Foundation、vus、ewebserver等；
     * 10. 各模块初始化，包括：CVM、DPM、IPM等；
     * 11. 判断状态是否为安装中，如果为安装中，恢复安装；
     * 12. 等待60s用于网络的恢复；
     * 13. 请求注册，包括：重试注册、周期注册等；
     * 14. 请求获取配置，包括：重试获取配置，周期获取配置等；
     * 15. 工程模式输入，跳过注册阶段，直接到获取配置阶段，且预留工程模式下获取配置的逻辑；
     * 16. 工程模式退出后，回到注册阶段，继续执行上述注册和获取配置的逻辑，回到第11步；
     * 17. 从平台获取配置成功后，请求收集零件信息；
     * 18. 根据状态机Resume任务；
     * 19. while(1);
     * 
     */
    D32S iRet = ADM_ECODE_FAILED;
    D32S intVal = 0;
    DCHAR* otaConfigPath = NULL;
    D32S i = 0;
    const DCHAR* itsArg = NULL;
    const DCHAR* soFilenNameTmp = NULL;
    const DCHAR* configFileNameTmp = NULL;
    DCHAR* soFilenName = NULL;
    DCHAR* configFileName = NULL;
    size_t nSize = 0;
    D32S c = 0;
    D32S endMark =0;

    do { 

        //!< 参数个数判断
        if (1 == argc) {
            usage();
            return -1;    /*for parasoft */
        }

        //!< 接收入参
        while ((c = getopt (argc, argv, "c:")) != -1)
        {
            switch (c)
            {
                case (int)'c' :
                    otaConfigPath = optarg;
                    (void)printf("optarg:%s\n", optarg);
                    break;
                case (int)'?':
                {
                    if ((char)optopt == 'o')
                    {
                        (void)fprintf (stderr, "Option -%c requires an argument.\n", optopt);
                    }
                    else if (0u != isprint ((unsigned char)optopt))
                    {
                        (void)fprintf (stderr, "Unknown option `-%c'.\n", optopt);
                    }
                    else
                    {
                        (void)fprintf (stderr, "Unknown option character `\\x%x'.\n", optopt);
                    }

                    /* unknown or wrong option used, show usage information and terminate */
                    endMark =-1;
                    usage();
                    break;
                }
                default:
                {
                    endMark =-1;
                    abort ();
                    break;
                }
            }
            if(-1 == endMark)
            {
                return -1;
            }
        }

        /**0. 初始化程序启动阶段的事件上报模块 */
        adm_apc_reportEventInit();

        //!< InitP_001
        adm_apc_eventAdd(ADM_APC_EV_STAGE_PRE, 1, ADM_OCS_EVENT_LEVEL_TRACE, 0, NULL);

        /** 1. 解析otaconfig.ini 配置文件 */
        iRet = adm_apc_otaConfigInit(otaConfigPath);
        if (ADM_ECODE_SUCCESSED != iRet) {
            (void)printf("adm_apc_otaConfigInit failed, iRet:%d\n", iRet);
            // 配置文件初始化失败，可以使用每个字段的默认配置，因此不退出程序；
            // exit(-1);
            //!< InitP_002
            adm_apc_eventAdd(ADM_APC_EV_STAGE_PRE, 2, ADM_OCS_EVENT_LEVEL_ERROR, iRet, "adm_apc_otaConfigInit failed");
        } else {
            (void)printf("adm_apc_otaConfigInit successed\n");
        }

        /** 2. 创建OTA过程中使用的目录  */ 
        iRet = adm_apc_dirInit();
        if (ADM_ECODE_SUCCESSED != iRet) {
            (void)printf("adm_apc_dirInit failed, iRet:%d\n", iRet);
            //!< InitP_002
            adm_apc_eventAdd(ADM_APC_EV_STAGE_PRE, 2, ADM_OCS_EVENT_LEVEL_ERROR, iRet, "adm_apc_dirInit failed");
            exit(-1);
        } else {
            (void)printf("adm_apc_dirInit successed\n");
            //!< InitP_003
            adm_apc_eventAdd(ADM_APC_EV_STAGE_PRE, 3, ADM_OCS_EVENT_LEVEL_TRACE, 0, NULL);
        }

        //!< 防止主程序的重复拉起
        // iRet = adm_apc_singleProgram();
        // if (ADM_ECODE_SUCCESSED != iRet)
        // {
        //     (void)printf("adm_apc_singleProgram failed, iRet:%d\n", iRet);
        //     return -1;
        // }
        // else
        // {
        //     (void)printf("adm_apc_singleProgram success");
        // }

        /**  配置coredump   */ 
		(void)adm_apc_otaCoredumpConfig();
        
        //TODO: 待添加事件
        /** 3. 初始化日志模块 */
	    soFilenNameTmp = adm_cfg_iniParserGetString(ADM_APC_DTS_SHARED_LIB_DIR, ADM_APC_DTS_SHARED_LIB_DIR_DEFAULT_VAL);
        configFileNameTmp = adm_cfg_iniParserGetString(ADM_APC_DTS_CONFIG_DIR, ADM_APC_DTS_CONFIG_DIR_DEFAULT_VAL);
        soFilenName = adm_strdup(soFilenNameTmp);
        configFileName = adm_strdup(configFileNameTmp);
#ifdef __MACRO_DTS_TOOL_SWITCH__
        (void)adm_tracerDaemonStart(soFilenName,configFileName);
#endif
        
        iRet = adm_apc_logInit();
        if (ADM_ECODE_SUCCESSED != iRet) {
            (void)printf("adm_apc_logInit failed, iRet:%d\n", iRet);
            exit(-2);
        } else {
            log_i("adm_apc_logInit successed");
        }

        /** 4. 初始化Tracer Tool */
        adm_apc_tracerInit();
        log_i("adm_acp_tracerInit done");

        /** 5. 初始化core service */
        iRet = adm_apc_coreServiceInit();
        if (ADM_ECODE_SUCCESSED != iRet) {
            log_e("adm_apc_coreServiceInit failed, err:%d", iRet);
            break;
        } else {
            log_i("adm_apc_coreServiceInit successed");
        }
        
        /** 记录pid,防止数据异常被多次重复拉起*/
        iRet = adm_apc_checkProcess();
        if (ADM_ECODE_SUCCESSED != iRet) 
        {
            log_e("adm_apc_checkProcess failed, iRet:%d", iRet);
            break;
        } 
        else 
        {
            log_i("adm_apc_checkProcess successed");
        }

        /** 启动网络代理*/
        (void)adm_apc_httpproxyInit();

        /** 9. 各管理模块初始化，包括：CVM、DPM、IPM等 */
        iRet = adm_apc_managerInit();
        if (ADM_ECODE_SUCCESSED != iRet) {
            log_e("adm_apc_managerInit failed, iRet:%d", iRet);
            break;
        } else {
            log_i("adm_apc_managerInit successed");
        }
        
        /** 6. 初始化DIM */
        iRet = adm_apc_dimInit();
        if (ADM_ECODE_SUCCESSED != iRet) {
            log_e("adm_apc_dimInit failed, iRet:%d", iRet);
            //break;
        } else {
            log_i("adm_apc_dimInit successed");
        }

        /** 7. 初始化UIM */
        iRet = adm_apc_uimInit();
        if (ADM_ECODE_SUCCESSED != iRet) {
            log_e("adm_apc_uimInit failed, iRet:%d", iRet);
            //break;
        } else {
            log_i("adm_apc_uimInit successed");
        }

        /** 8. 输出各模块的版本号 */
        adm_apc_printSdkVersion();
        log_i("adm_apc_printSdkVersion done");

		/**10. 初始化ocm */
        iRet = adm_apc_ocm_init();
		if (ADM_ECODE_SUCCESSED != iRet) {
            log_e("adm_apc_ocm_init failed, iRet:%d", iRet);
            break;
        } else {
            log_i("adm_apc_ocm_init successed");
        }
        /*初始化IRM*/
        iRet = adm_apc_irmInit();
        if (ADM_ECODE_SUCCESSED != iRet) {
            log_e("adm_apc_irmInit failed, iRet:%d", iRet);
            break;
        } else {
            log_i("adm_apc_irmInit successed");
        }

        /** 10. 判断当前是否为安装中状态 */
        if (TRUE == adm_apc_isInstallingState()) {
            log_i("adm_apc_isInstallingState is TRUE, resume task into installing");

            adm_apc_vehicleConfigReset(TRUE);
            
            /** 16. 根据状态机Resume任务； */
            (void)adm_apc_resumeTask();
        } else {
            log_i("adm_apc_isInstallingState is FALSE");
        }

        // adm_apc_triggerReportEvent();

        /** 11. 等待60s用于网络的恢复； */
        intVal = adm_cfg_iniParserGetInt(ADM_CFG_VUS_WAIT_NETWORK_OK_SEC, ADM_CFG_VUS_WAIT_NETWORK_OK_SEC_DEFAULT_VAL);
        adm_apc_waitNetworkOk((D32U)intVal);

        /** 12-16. 为OTA注入第一滴血，包括：注册和获取配置信息 */
        iRet = adm_apc_otaFirstBlood();
        if (ADM_ECODE_SUCCESSED != iRet) {
            log_e("adm_apc_otaFirstBlood failed, iRet:%d", iRet);
            break;
        } else {
            log_i("adm_apc_otaFirstBlood successed");
        }

        /** 17. 根据状态机Resume任务； */
        (void)adm_apc_resumeTask();

#ifdef ADM_DEBUG_CVM_MODULE
    while (TRUE) {
        log_i("appcore checkVersion enter");
        D32S retVal = ADM_ECODE_FAILED;
        ADM_OCS_CHECK_RESP_ADAPTER_T* resp = NULL;

        retVal = adm_cvm_checkVersionAdaptorNotify(ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_USER, NULL, NULL, &resp);
        if (ADM_ECODE_SUCCESSED == retVal || ADM_ECODE_NO_NEW_VERSION == retVal || ADM_ECODE_CHECK_LATEST_VERSION == retVal) {
            log_i("check success");
        } else {
            log_e("check error[%d]", retVal);
        }
        adm_ocs_deleteCheckRespAdapterBody(resp);
        log_i("appcore checkVersion exit");

        if (ADM_ECODE_SUCCESSED == retVal) {
            break;
        }
        sleep(5);
    }
#endif//ADM_DEBUG_CVM_MODULE

// TODO: debug dpm module --- cvm debug打开时也可触发 dpm 下载动作
#if ((defined ADM_DEBUG_CVM_MODULE) || (defined ADM_DEBUG_DPM_MODULE)) 
        log_e("Entry debug dpm module.");
        iRet = adm_apc_triggerDownload();
        if (ADM_IS_ERR(iRet)) 
        {
            log_e("adm_apc_triggerDownload failed, iRet:%d", iRet);
            break;
        } 
        log_i("adm_apc_triggerDownload successed.");
#endif
        /** 18. while(1) */
        adm_apc_waitTask();

    } while(FALSE);

    adm_apc_managerDeinit();
    adm_apc_uimDeinit();
    adm_apc_dimDeinit();
    adm_apc_irmDeinit();
    adm_apc_coreServiceDeinit();
    adm_apc_tracerDeinit();
    adm_apc_logDeinit();
    adm_apc_otaConfigDeinit();
#ifdef SUPPORT_LION_PKI
    (DVOID)adm_oca_scDeinit();
#endif

#ifdef __MACRO_DTS_TOOL_SWITCH__
    (void)adm_tracerDaemonStop();
#endif

    adm_free(soFilenName);
    adm_free(configFileName);
    adm_apc_closelockfileHandle();

}
