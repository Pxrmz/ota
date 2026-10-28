/**
 * @file adm_apc_apkCore.c
 * @author maxiangyang (maxiangyang@abupdate.com)
 * @brief apk拉取主控程序的入口模块
 * @version 0.1
 * @date 2024-05-30
 * 
 * @copyright Copyright (c) 2024 上海艾拉比智能科技有限公司
 * 
 * @par 修改日志:
 * <table>
 * <tr><th>Date       <th>Version <th>Author  <th>Description
 * <tr><td>2024-05-30 <td>1.0     <td>maxiangyang    <td>内容
 * </table>
 */

#include <pthread.h>
#include <sys/time.h>
#include "adm_apc_apkCore.h"
#include "adm_apc_coreAchieve.h"
#include "adm_apc_event.h"
#include "adm_vsm_fsm.h"

#ifdef SUPPORT_LION_PKI
#include "adm_oca_httpCommon.h"
#endif

#define LOGM(fmt, ...)   (void)printf("[%s] %s:%d "fmt"\n", __DATE__, __FILE__,  __LINE__, ##__VA_ARGS__)

static D32S writeFileData(const DCHAR* filePath, const DCHAR* fileData)
{
    DSIZE size = 0;
    FILE *fp  = NULL;
    DSIZE writeLen = 0;
    D32S iRet = -1;

    if ((NULL == filePath) || (NULL == fileData))
    {
        printf( "Invalid input param.\n");
        return -1;
    }

    do {
        fp = fopen(filePath, "wb+");
        if (NULL == fp) 
        {
            printf("fopen %s failed, iRet:%d\n", filePath, iRet);
            break;
        }

        size = adm_strlen(fileData);

        writeLen = fwrite(fileData, 1, size, fp);
        if (writeLen == size) 
        {
            iRet = 0;
            break;
        } 
    } while(FALSE);

    if (NULL != fp) 
    {
        (void)fclose(fp);
    }

    return iRet;
}

#define ECHO_PATH   	  "/data_fota/ota/ucmJniLog/echo.log"
static D32S echoString(const DCHAR *pString)
{
        D32S iRet = 0;

        iRet = writeFileData(ECHO_PATH, pString);

        return iRet;
}

static D32S apcApkCoreStart(DVOID)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    D32S iIntVal = 0;

    do
    {
         /** 获取ICC-sn号 */
#ifdef SUPPORT_LION_PKI
        if (FALSE == adm_apc_isInstallingState())
        {
            /**根据国家码和品牌处理域名*/
            // adm_apc_checkVehicleEnvironment();

            iRet = adm_apc_getPkiInfo();
            if (ADM_ECODE_SUCCESSED != iRet) 
            {
                log_e("adm_apc_getPkiInfo failed, iRet:%d", iRet);
                break;
            } 
            else 
            {
                log_i("adm_apc_getPkiInfo successed");
            }
        }
#endif

        /** 同步获取车辆动力类型，必须在注册前完成 */
        iRet = adm_apc_syncVehiclePowerType();
        if (ADM_IS_ERR(iRet)) 
        {
            /** 不会走到 */
            log_e("adm_apc_syncVehiclePowerType failed, iRet:%d", iRet);
            break;
        } else {
            log_i("adm_apc_syncVehiclePowerType successed");
        }


        adm_apc_taskVerify();

        /** 10. 判断当前是否为安装中状态 */
        if (TRUE == adm_apc_isInstallingState()) 
        {
            log_i("adm_apc_isInstallingState is TRUE, resume task into installing");

            adm_apc_vehicleConfigReset(TRUE);

            /** 16. 根据状态机Resume任务； */
            (void)adm_apc_resumeTask();

    #ifdef SUPPORT_LION_PKI
            // /**根据国家码和品牌处理域名*/
            // adm_apc_checkVehicleEnvironment();

            iRet = adm_apc_getPkiInfo();
            if (ADM_ECODE_SUCCESSED != iRet) 
            {
                log_e("adm_apc_getPkiInfo failed, iRet:%d", iRet);
                break;
            } 
            else 
            {
                log_i("adm_apc_getPkiInfo successed");
            }
    #endif
        } 
        else 
        {
            log_i("adm_apc_isInstallingState is FALSE");
        }

        /** 11. 等待60s用于网络的恢复； */
        iIntVal = adm_cfg_iniParserGetInt(ADM_CFG_VUS_WAIT_NETWORK_OK_SEC, ADM_CFG_VUS_WAIT_NETWORK_OK_SEC_DEFAULT_VAL);
        adm_apc_waitNetworkOk((D32U)iIntVal);

        /** 12-16. 为OTA注入第一滴血，包括：注册和获取配置信息 */
        iRet = adm_apc_otaFirstBlood();
        if (ADM_ECODE_SUCCESSED != iRet) 
        {
            log_e("adm_apc_otaFirstBlood failed, iRet:%d", iRet);
            break;
        } 
        else 
        {
            log_i("adm_apc_otaFirstBlood successed");
        }

        /** 17. 根据状态机Resume任务； */
        (void)adm_apc_resumeTask();
    } while (FALSE);

    return iRet;
}

static DVOID* apcStartThread(DVOID* pArg)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    ADM_UNUSED_ARG(pArg);

    iRet = apcApkCoreStart();
    if (ADM_IS_ERR(iRet))
    {
        log_e("APC: Failed to start ucm, err[%d].", iRet);
    }

    return DNULL;
}

D32S adm_apc_jniDirInit(const DCHAR* pDirPath)
{
    D32S iRet = ADM_ECODE_FAILED;

    iRet = adm_com_createDir(pDirPath);

    if(ADM_ECODE_SUCCESSED != iRet)
    {
        LOGM("adm_com_createDir %s failed, iRet:%d\n", pDirPath, iRet);
        return iRet;
    }

    LOGM("adm_com_createDir %s successed\n", pDirPath);

    return iRet;
}

D32S adm_apc_apkCore_init(const DCHAR* pConfigPath, ADM_APC_JNI_CALLBACK_T* pstJniCb)
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
    D32S iRet = ADM_ECODE_SUCCESSED;
    DCHAR* pSoFilenName = NULL;
    DCHAR* pConfigFileName = NULL;
    pthread_t startId;
    DCHAR aBuf[256] = {0};

    do { 
        /** 0. 初始化程序启动阶段的事件上报模块 */
        adm_apc_reportEventInit();

        /** InitP_001 */
        adm_apc_eventAdd(ADM_APC_EV_STAGE_PRE, 1, ADM_OCS_EVENT_LEVEL_TRACE, 0, NULL);

        /** 1. 解析otaconfig.ini 配置文件 */
        iRet = adm_apc_otaConfigInit(pConfigPath);
        if (ADM_ECODE_SUCCESSED != iRet) 
        {
            (DVOID)adm_snprintf(aBuf, sizeof(aBuf), "adm_apc_otaConfigInit failed, iRet:%d\n", iRet);
            (DVOID)echoString(aBuf);
            // (void)printf("adm_apc_otaConfigInit failed, iRet:%d\n", iRet);
            adm_apc_eventAdd(ADM_APC_EV_STAGE_PRE, 2, ADM_OCS_EVENT_LEVEL_ERROR, iRet, "adm_apc_otaConfigInit failed");
        } 
        else 
        {
            (void)echoString("adm_apc_otaConfigInit successed\n");
        }

        /** 2. 创建OTA过程中使用的目录  */ 
        iRet = adm_apc_dirInit();
        if (ADM_ECODE_SUCCESSED != iRet) 
        {
            (DVOID)adm_snprintf(aBuf, sizeof(aBuf), "adm_apc_dirInit failed, iRet:%d\n", iRet);
            (DVOID)echoString(aBuf);
            // (void)printf("adm_apc_dirInit failed, iRet:%d\n", iRet);
            ///< InitP_002
            adm_apc_eventAdd(ADM_APC_EV_STAGE_PRE, 2, ADM_OCS_EVENT_LEVEL_ERROR, iRet, "adm_apc_dirInit failed");
            exit(-1);
        } 
        else 
        {
            (void)echoString("adm_apc_dirInit successed\n");
            ///< InitP_003
            adm_apc_eventAdd(ADM_APC_EV_STAGE_PRE, 3, ADM_OCS_EVENT_LEVEL_TRACE, 0, NULL);
        }

        ///< 防止主程序的重复拉起
        //iRet = adm_apc_singleProgram();
        //if (ADM_ECODE_SUCCESSED != iRet)
        //{
        //    (DVOID)adm_snprintf(aBuf, sizeof(aBuf), "adm_apc_singleProgram failed, iRet:%d\n", iRet);
        //    (DVOID)echoString(aBuf);
            // (void)printf("adm_apc_singleProgram failed, iRet:%d\n", iRet);
        //    return -1;
        //}
        //else
        //{
        //    (void)echoString("adm_apc_singleProgram success");
        //}

        /**  配置coredump   */ 
		(void)adm_apc_otaCoredumpConfig();
        
        /** 3. 初始化日志模块 */
	    pSoFilenName = adm_strdup(adm_cfg_iniParserGetString(ADM_APC_DTS_SHARED_LIB_DIR, ADM_APC_DTS_SHARED_LIB_DIR_DEFAULT_VAL));
        pConfigFileName = adm_strdup(adm_cfg_iniParserGetString(ADM_APC_DTS_CONFIG_DIR, ADM_APC_DTS_CONFIG_DIR_DEFAULT_VAL));

#ifdef __MACRO_DTS_TOOL_SWITCH__
        (void)adm_tracerDaemonStart(pSoFilenName, pConfigFileName);
#endif
        
        iRet = adm_apc_logInit();
        if (ADM_ECODE_SUCCESSED != iRet) 
        {
            (DVOID)adm_snprintf(aBuf, sizeof(aBuf), "adm_apc_logInit failed, iRet:%d\n", iRet);
            (DVOID)echoString(aBuf);
            // (void)printf("adm_apc_logInit failed, iRet:%d\n", iRet);
            exit(-2);
        } 
        else 
        {
            echoString("adm_apc_logInit successed.\n");
            log_i("adm_apc_logInit successed");
        }

        log_i("[ABUP-Procedure] *******************************************************************");
        log_i("[ABUP-Procedure] OTA SDK Start......");
#ifdef ADM_SDK_VERSION
        log_i("[ABUP-Procedure] SDK version: %s", ADM_SDK_VERSION);
#endif
        log_i("[ABUP-Procedure] *******************************************************************");

        /** 4. 初始化Tracer Tool */
        adm_apc_tracerInit();
        log_i("adm_acp_tracerInit done");

        /** 5. 初始化core service */
        iRet = adm_apc_coreServiceInit();
        if (ADM_ECODE_SUCCESSED != iRet) 
        {
            log_e("adm_apc_coreServiceInit failed, err:%d", iRet);
            break;
        } 
        else 
        {
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
        iRet = adm_apc_managerInit(pstJniCb);
        if (ADM_ECODE_SUCCESSED != iRet) 
        {
            log_e("adm_apc_managerInit failed, iRet:%d", iRet);
            break;
        } 
        else 
        {
            log_i("adm_apc_managerInit successed");
        }
        
        /** 6. 初始化DIM */
        iRet = adm_apc_dimInit();
        if (ADM_ECODE_SUCCESSED != iRet) 
        {
            log_e("adm_apc_dimInit failed, iRet:%d", iRet);
        } 
        else 
        {
            log_i("adm_apc_dimInit successed");
        }

        /** 7. 初始化UIM */
        iRet = adm_apc_uimInit();
        if (ADM_ECODE_SUCCESSED != iRet) 
        {
            log_e("adm_apc_uimInit failed, iRet:%d", iRet);
        } 
        else 
        {
            log_i("adm_apc_uimInit successed");
        }

        /** 8. 输出各模块的版本号 */
        adm_apc_printSdkVersion();
        log_i("adm_apc_printSdkVersion done");

		/**10. 初始化ocm */
        iRet = adm_apc_ocm_init();
		if (ADM_ECODE_SUCCESSED != iRet) 
        {
            log_e("adm_apc_ocm_init failed, iRet:%d", iRet);
            break;
        } 
        else 
        {
            log_i("adm_apc_ocm_init successed");
        }

        /*初始化IRM*/
        iRet = adm_apc_irmInit();
        if (ADM_ECODE_SUCCESSED != iRet) 
        {
            log_e("adm_apc_irmInit failed, iRet:%d", iRet);
            break;
        } 
        else 
        {
            log_i("adm_apc_irmInit successed");
        }


        if (0 == pthread_create(&startId, NULL, apcStartThread, NULL)) 
        {
            log_i("pthread_create successed");
            pthread_detach(startId);
        } 
        else 
        {
            log_e("pthread_create failed");
            iRet = ADM_ECODE_PTHREAD_CREATE_FAILED;
            break;
        }
    } while(FALSE);

    if (ADM_IS_ERR(iRet))
    {
        adm_apc_apkCore_deInit();
    }
    adm_free(pSoFilenName);
    adm_free(pConfigFileName);

    return iRet;
}


DVOID adm_apc_apkCore_deInit()
{
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
    (DVOID)adm_tracerDaemonStop();
#endif
    adm_apc_closelockfileHandle();

    return;
}

D32S adm_apc_getVsmStat(DVOID)
{
    return adm_vsm_stateGet();
}