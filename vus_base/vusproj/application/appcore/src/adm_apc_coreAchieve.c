#include "adm_apc_coreAchieve.h"
#include "adm_cfg_api.h"
#include "adm_vrm_register.h"
#include "adm_vrm_event.h"
#include "adm_com_lib.h"
#include "adm_tracer_interface.h"
#include "adm_engine_common.h"
#include "adm_soa.h"
#include "adm_dim_api.h"
#include "adm_dim_vdi.h"
#include "adm_dim_host.h"
#include "adm_dim_ucl.h"
#include "adm_dim_transHost.h"
#include "adm_uim_api.h"
#include "adm_uim_services.h"
#include "adm_uim_check.h"
#include "adm_uim_install.h"
#include "adm_uim_ems.h"
#include "adm_ems_uim_impl.h"
#include "adm_string.h"
#include "adm_memory.h"
#include "adm_dpm_uim.h"
#include "adm_cvm_uim.h"
#include "adm_dpm_download.h"
#include "adm_dpm_downloadManagement.h"
#include "adm_ipm_install.h"
#include "adm_vrm_register.h"
#include "adm_ecm_management.h"
#include "adm_sdk_version.h"
#include "adm_sdk_selfUpgrade.h"
#include "adm_getFoundationVersion.h"
#include "adm_vsm_fsm.h"
#include "adm_vsm_tspImpl.h"
#include "adm_pis_package.h"
//#include "adm_vsm_impl.h"
#include "adm_eim_ecuInfoManager.h"
#include "adm_ocm_interface.h"
#include "adm_apc_backtrace.h"
#include "adm_com_kvstorage.h"
#include "adm_iss_security.h"
#include "adm_irm_report.h"
#include "adm_irm_logFile.h"
#include "adm_cvm_checkVersion.h"
#include "adm_apc_event.h"
#include "adm_arpc_service.h"
#include "adm_dpm_report.h"
#include "adm_pub_func.h"
#include "adm_cvm_syncVehicleInfo.h"
#include "adm_uim_general.h"
#include "adm_dim_vdiSpy.h"
#include "adm_cvm_powerOncheck.h"
#include "adm_http_proxy.h"
#include "adm_eim_parallel.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/file.h>
#include <errno.h>
#include "adm_icp_info.h"
#include "adm_icpm_uim.h"
#include <stdio.h>
#include <stdlib.h>
#ifdef __QNX__
#else
    #include <sys/prctl.h>
#endif
#include <sys/resource.h>
#include <string.h>

#include <fcntl.h>
#include <time.h>
#include "adm_soa_cfg.h"

#ifdef SUPPORT_LION_PKI
#include "adm_oca_httpCommon.h"
#endif

//!< tpm 依赖 ewebserver 的支持 
#ifdef CMAKE_CUSTOM_EWEBSERVER_SUPPORT
    #include "adm_tpm_transfer.h"
    #include "adm_tpm_transferManagement.h"
#endif

#define ADM_APC_CORE_INIT_ENTRY     log_i("Entry...")
#define ADM_APC_CORE_INIT_EXIT      log_i("Exit ...")
#define LOGM(fmt, ...)   printf("[%s] %s:%d "fmt"\n", __DATE__, __FILE__,  __LINE__, ##__VA_ARGS__)

//!< coredump file 大小 
#define CORE_FILE_SIZE 1024*1024*5       
//!< coredump 配置文件路径 
#define KERNEL_PATTERN_FILE  "/proc/sys/kernel/core_pattern"
//!< coredump 文件前缀 
#define APC_COREDUMP_NAME_PREFIX  "admfotaappCoredump"



static ADM_SOA_SESSION_T* g_pstSoaSession = NULL;
static D32S g_ilock_file = 0;  //文件锁
static DBOOL g_bIsEndProcess = FALSE;
static DBOOL g_bIsVerifyFinish = FALSE;

static D32S apcGetVehicleType(ADM_DIM_SOA_VEHICLE_TYPE_E *pVehicleType)
{
    D32S iRet = ADM_ECODE_SUCCESSED;

    if (ADM_IS_NULL(pVehicleType))
    {
        log_e("Invalid param.");
        return ADM_ECODE_INVALID_PARAM;
    }

    *pVehicleType = (ADM_DIM_SOA_VEHICLE_TYPE_E)adm_cfg_iniParserGetInt(ADM_CFG_VUS_VEHICLE_TYPE, DIM_VEHICLE_INVALID);

    return iRet;
}

void adm_apc_reportEventInit(void)
{
    ADM_APC_CORE_INIT_ENTRY;
    
    do {
        adm_apc_initReportEvent(adm_irm_getEventSequenceId, adm_irm_reportEvent);
    } while(FALSE);
    ADM_APC_CORE_INIT_EXIT;

    return;
}

/**
 * @brief 初始化OTA配置文件
 * 
 * @param filename [in]     配置文件的绝对路径
 * @return D32S     0:Successed, None-0:Failed
 */
D32S adm_apc_otaConfigInit(const DCHAR* filename)
{
    ADM_APC_CORE_INIT_ENTRY;
    D32S iRet = ADM_ECODE_FAILED;
    ADM_CFG_API_INFO_T pstApiData = {0};

    do {
        //!< InitI_001
        adm_apc_eventAdd(ADM_APC_EV_STAGE_ING, 1, ADM_OCS_EVENT_LEVEL_TRACE, 0, NULL);

        if (NULL == filename) {
            iRet = ADM_ECODE_INVALID_PARAM;
            (void)LOGM("Input paramater [filename] is nil, iRet:%d\n", iRet);
            break;
        }

        pstApiData.isFileExistCb = adm_com_isFileExist;
        pstApiData.writeFileDataCb = adm_com_writeFileData;
        iRet = adm_cfg_iniParserInit(filename, &pstApiData);
        if (ADM_ECODE_SUCCESSED != iRet) {
            (void)LOGM("adm_cfg_iniParserInit %s failed, iRet:%d\n", filename, iRet);
            break;
        } else {
            (void)LOGM("adm_cfg_iniParserInit %s successed", filename);
        }
    } while(FALSE);
    ADM_APC_CORE_INIT_EXIT;

    //!< 添加事件
    if (ADM_ECODE_SUCCESSED != iRet) {
        //!< InitI_002
        adm_apc_eventAdd(ADM_APC_EV_STAGE_ING, 2, ADM_OCS_EVENT_LEVEL_WARN, iRet, "adm_apc_otaConfigInit failed");
    } else {
        //!< InitI_003
        adm_apc_eventAdd(ADM_APC_EV_STAGE_ING, 3, ADM_OCS_EVENT_LEVEL_TRACE, 0, NULL);
    }

    return iRet;
}
/**
 * @brief 初始化coredump配置
 * 
 * @param [in]     无
 * @return D32S     0:Successed, None-0:Failed
 */
D32S adm_apc_otaCoredumpConfig(void)
{
    ADM_APC_CORE_INIT_ENTRY;
    D32S iRet = ADM_ECODE_FAILED;
	struct rlimit stRlim = {0};	
	DBOOL isCoredumpUpload = FALSE;
    const DCHAR* pDirStr = NULL;
	DCHAR coredumpFilePath[ADM_COM_FILEPATH_LEN] = {'0'};
	

	do 
	{
	    isCoredumpUpload = adm_cfg_iniParserGetBoolean(ADM_CFG_VUS_IS_COREDUMP_UPLOAD, (DBOOL)ADM_CFG_VUS_IS_COREDUMP_UPLOAD_DEFAULT_VAL);
		if (!isCoredumpUpload) 
		{
			(void)LOGM("isCoredumpUpload is false. \n");
			iRet = ADM_ECODE_SUCCESSED;
			break;
		} 
		
		
	    iRet = getrlimit(RLIMIT_CORE, &stRlim);
		if(iRet != 0)
		{
	        (void)LOGM("getrlimit failed, iRet:%d\n", iRet);
	    }
		(void)LOGM("getrlimit rlim_cur=%ld, rlim_max:%ld\n", stRlim.rlim_cur,stRlim.rlim_max);
	    stRlim.rlim_cur = stRlim.rlim_max = CORE_FILE_SIZE;
	
	    iRet = setrlimit(RLIMIT_CORE, &stRlim);
	    if(iRet != 0)
		{
	        (void)LOGM("setrlimit failed, iRet:%d\n", iRet);
	         break;
	    }

		iRet = getrlimit(RLIMIT_CORE, &stRlim);
		if(iRet != 0)
		{
	        (void)LOGM("getrlimit failed, iRet:%d\n", iRet);
	    }
		(void)LOGM("getrlimit rlim_cur=%ld, rlim_max:%ld\n", stRlim.rlim_cur,stRlim.rlim_max);

		/** coredump文件存储于日志收集存储目录 */
        pDirStr = adm_cfg_iniParserGetString(ADM_CFG_VUS_LOG_COLLECT_DIR, ADM_CFG_VUS_LOG_COLLECT_DIR_DEFAULT_VAL);
        
        /*
		DCHAR* pLocaltime = __adm_apc_getTimeStr();
		if(NULL == pLocaltime)
		{
	        (void)LOGM("get time string failed ! \n");
	         break;
	    }
       
		(void)adm_snprintf(coredumpFilePath, sizeof(coredumpFilePath), "%s%s_%s", pDirStr, APC_COREDUMP_NAME_PREFIX, pLocaltime);
        */
		(void)adm_snprintf(coredumpFilePath, sizeof(coredumpFilePath), "%s%s", pDirStr, APC_COREDUMP_NAME_PREFIX);

	    D32S core_pattern_fd = open(KERNEL_PATTERN_FILE, O_RDWR|O_NDELAY|O_TRUNC, 0666);
	    if(core_pattern_fd < 0)
		{
            (void)LOGM("open %s failed ! \n",KERNEL_PATTERN_FILE);
            break;
	    }

	    iRet = write(core_pattern_fd, coredumpFilePath, adm_strlen(coredumpFilePath));
	    if(iRet < 0)
		{
            (void)LOGM("write %s failed ! \n",KERNEL_PATTERN_FILE);
			break;
	    }
	    close(core_pattern_fd);

		(void)LOGM("config coredump file successful ! \n");
		iRet = ADM_ECODE_SUCCESSED;
		
	}while(FALSE);
    ADM_APC_CORE_INIT_EXIT;
    return iRet;
}


/**
 * @brief 反初始化OTA配置文件
 * 
 */
void adm_apc_otaConfigDeinit(void)
{
    ADM_APC_CORE_INIT_ENTRY;
    adm_cfg_iniParserDeinit();
    ADM_APC_CORE_INIT_EXIT;
    //!< InitE_15
    // adm_apc_eventAdd(ADM_APC_EV_STAGE_END, 15, ADM_OCS_EVENT_LEVEL_TRACE, 0, NULL);
}

/**
 * @brief OTA应用程序所使用的目录初始化
 * 
 * @return D32S     0:Successed, None-0:Failed
 */
D32S adm_apc_dirInit(void)
{
    ADM_APC_CORE_INIT_ENTRY;
    D32S iRet = ADM_ECODE_FAILED;
    const DCHAR* dirStr = NULL;

    do {
        //!< InitI_004
        adm_apc_eventAdd(ADM_APC_EV_STAGE_ING, 4, ADM_OCS_EVENT_LEVEL_TRACE, 0, NULL);

        /** 初始化运行时主目录 */
        dirStr = adm_cfg_iniParserGetString(ADM_CFG_VUS_RUNTIME_ROOT_DIR, ADM_CFG_VUS_RUNTIME_ROOT_DIR_DEFAULT_VAL);
        iRet = adm_com_createDir(dirStr);
        if (ADM_ECODE_SUCCESSED != iRet) {
            (void)LOGM("adm_com_createDir %s failed, iRet:%d\n", dirStr, iRet);
            break;
        } else {
            (void)LOGM("adm_com_createDir %s successed\n", dirStr);
        }

        /** 初始化kv存储目录 */
        dirStr = adm_cfg_iniParserGetString(ADM_CFG_VUS_KV_STORAGE_DIR, ADM_CFG_VUS_KV_STORAGE_DIR_DEFAULT_VAL);
        iRet = adm_com_createDir(dirStr);
        if (ADM_ECODE_SUCCESSED != iRet) {
            (void)LOGM("adm_com_createDir %s failed, iRet:%d\n", dirStr, iRet);
            break;
        } else {
            (void)LOGM("adm_com_createDir %s successed\n", dirStr);
        }

        /** 初始化升级包存储目录 */
        dirStr = adm_cfg_iniParserGetString(ADM_CFG_VUS_PKG_STORAGE_DIR, ADM_CFG_VUS_PKG_STORAGE_DIR_DEFAULT_VAL);
        iRet = adm_com_createDir(dirStr);
        if (ADM_ECODE_SUCCESSED != iRet) {
            (void)LOGM("adm_com_createDir %s failed, iRet:%d\n", dirStr, iRet);
            break;
        } else {
            (void)LOGM("adm_com_createDir %s successed\n", dirStr);
        }

        /** 初始化日志存储目录 */
        dirStr = adm_cfg_iniParserGetString(ADM_CFG_VUS_LOG_STORAGE_DIR, ADM_CFG_VUS_LOG_STORAGE_DIR_DEFAULT_VAL);
        iRet = adm_com_createDir(dirStr);
        if (ADM_ECODE_SUCCESSED != iRet) {
            (void)LOGM("adm_com_createDir %s failed, iRet:%d\n", dirStr, iRet);
            break;
        } else {
            (void)LOGM("adm_com_createDir %s successed\n", dirStr);
        }
        /** 初始化日志收集存储目录 */
        dirStr = adm_cfg_iniParserGetString(ADM_CFG_VUS_LOG_COLLECT_DIR, ADM_CFG_VUS_LOG_COLLECT_DIR_DEFAULT_VAL);
        iRet = adm_com_createDir(dirStr);
        if (ADM_ECODE_SUCCESSED != iRet) {
            (void)LOGM("adm_com_createDir %s failed, iRet:%d\n", dirStr, iRet);
            break;
        } else {
            (void)LOGM("adm_com_createDir %s successed\n", dirStr);
        }

        /** 初始化日志打包的存储目录 */
        dirStr = adm_cfg_iniParserGetString(ADM_CFG_VUS_LOG_PACK_DIR, ADM_CFG_VUS_LOG_PACK_DIR_DEFAULT_VAL);
        iRet = adm_com_createDir(dirStr);
        if (ADM_ECODE_SUCCESSED != iRet) {
            (void)LOGM("adm_com_createDir %s failed, iRet:%d\n", dirStr, iRet);
            break;
        } else {
            (void)LOGM("adm_com_createDir %s successed\n", dirStr);
        }
        /** 初始化证书存储目录 */
        dirStr = adm_cfg_iniParserGetString(ADM_CFG_VUS_CERT_STORAGE_DIR, ADM_CFG_VUS_CERT_STORAGE_DIR_DEFAULT_VAL);
        iRet = adm_com_createDir(dirStr);
        if (ADM_ECODE_SUCCESSED != iRet) {
            (void)LOGM("adm_com_createDir %s failed, iRet:%d\n", dirStr, iRet);
            break;
        } else {
            (void)LOGM("adm_com_createDir %s successed\n", dirStr);
        }

        //!< 初始化文件解压的存储目录
        dirStr = adm_cfg_iniParserGetString(ADM_CFG_VUS_UNZIP_STORAGE_DIR, ADM_CFG_VUS_UNZIP_STORAGE_DIR_DEFAULT_VAL);
        iRet = adm_com_createDir(dirStr);
        if (ADM_ECODE_SUCCESSED != iRet) {
            (void)LOGM("adm_com_createDir %s failed, iRet:%d\n", dirStr, iRet);
            break;
        } else {
            (void)LOGM("adm_com_createDir %s successed\n", dirStr);
            if (FALSE == adm_core_service_initRuntimeDir(ADM_CORE_SERVICE_RUNTIME_DIR_TYPE_UNZIP, dirStr)) {
                (void)LOGM("adm_core_service_initRuntimeDir eType:%d failed\n", ADM_CORE_SERVICE_RUNTIME_DIR_TYPE_UNZIP);
                break;
            }
        }

        /** 初始化文件传输主目录 */
        dirStr = adm_cfg_iniParserGetString(ADM_CFG_EWEB_DOC_ROOT_DIR, ADM_CFG_EWEB_DOC_ROOT_DIR_DEFAULT_VAL);
        iRet = adm_com_createDir(dirStr);
        if (ADM_ECODE_SUCCESSED != iRet) {
            (void)LOGM("adm_com_createDir %s failed, iRet:%d\n", dirStr, iRet);
            break;
        } else {
            (void)LOGM("adm_com_createDir %s successed\n", dirStr);
        }

        /** 初始化文件传输的上传目录 */
        dirStr = adm_cfg_iniParserGetString(ADM_CFG_EWEB_DOC_UPLOAD_DIR, ADM_CFG_EWEB_DOC_UPLOAD_DIR_DEFAULT_VAL);
        iRet = adm_com_createDir(dirStr);
        if (ADM_ECODE_SUCCESSED != iRet) {
            (void)LOGM("adm_com_createDir %s failed, iRet:%d\n", dirStr, iRet);
            break;
        } else {
            (void)LOGM("adm_com_createDir %s successed\n", dirStr);
        }

        //!< 初始化文件传输的下载目录
        dirStr = adm_cfg_iniParserGetString(ADM_CFG_EWEB_DOC_DOWNLOAD_DIR, ADM_CFG_EWEB_DOC_DOWNLOAD_DIR_DEFAULT_VAL);
        iRet = adm_com_createDir(dirStr);
        if (ADM_ECODE_SUCCESSED != iRet) {
            (void)LOGM("adm_com_createDir %s failed, iRet:%d\n", dirStr, iRet);
            break;
        } else {
            (void)LOGM("adm_com_createDir %s successed\n", dirStr);
        }

        /** 初始化pki目录 */
#ifdef SUPPORT_LION_PKI
        dirStr = adm_cfg_iniParserGetString(ADM_CFG_VUS_PKI_DIR, ADM_CFG_VUS_PKI_DIR_DEFAULT_VAL);
        iRet = adm_com_createDir(dirStr);
        if (ADM_ECODE_SUCCESSED != iRet) {
            (void)LOGM("adm_com_createDir %s failed, iRet:%d\n", dirStr, iRet);
            break;
        } else {
            (void)LOGM("adm_com_createDir %s successed\n", dirStr);
        }
#endif

        //!< 初始化VRPC someip共享目录
        // dirStr = adm_cfg_iniParserGetString(ADM_CFG_VRPC_SOMEIP_RUNTIME_DIR, ADM_CFG_VRPC_SOMEIP_RUNTIME_DIR_DEFAULT_VAL);
        // iRet = adm_com_createDir(dirStr);
        // if (ADM_ECODE_SUCCESSED != iRet) {
        //     (void)LOGM("adm_com_createDir %s failed, iRet:%d\n", dirStr, iRet);
        //     break;
        // } else {
        //     (void)LOGM("adm_com_createDir %s successed\n", dirStr);
        // }

    } while(FALSE);

    if (ADM_ECODE_SUCCESSED != iRet) {
        //!< InitI_005
        adm_apc_eventAdd(ADM_APC_EV_STAGE_ING, 5, ADM_OCS_EVENT_LEVEL_ERROR, iRet, "adm_apc_dirInit failed");
    } else {
        //!< InitI_006
        adm_apc_eventAdd(ADM_APC_EV_STAGE_ING, 5, ADM_OCS_EVENT_LEVEL_TRACE, 0, NULL);
    }
    ADM_APC_CORE_INIT_EXIT;
    return iRet;
}
static D64S g_llogStartTime = 0;        //!< 日志开始记录的时间

/**
 * @brief OTA应用程序日志初始化
 * 
 * @return D32S     0:Successed, None-0:Failed
 */
D32S adm_apc_logInit(void)
{
    ADM_APC_CORE_INIT_ENTRY;
    // 初始化日志模块
    D32S iRet = ADM_ECODE_SUCCESSED;
    const DCHAR* logDir = NULL;
    D64U logMaxSize = 0;
    D32S logMaxRotate = 0;
    D32S logPrintLevel = -1;
	DBOOL bLogOutputTerminal = TRUE;//是否输出log至串口终端
    D8U u8cLogPrintLevel = ELOG_LVL_DEBUG;  //!< 默认日志过滤级别为 ELOG_LVL_DEBUG   

    //!< InitI_007
    adm_apc_eventAdd(ADM_APC_EV_STAGE_ING, 7, ADM_OCS_EVENT_LEVEL_TRACE, 0, NULL);
    logDir = adm_cfg_iniParserGetString(ADM_CFG_VUS_LOG_STORAGE_DIR, ADM_CFG_VUS_LOG_STORAGE_DIR_DEFAULT_VAL);
    logMaxSize = (D64U)adm_cfg_iniParserGetLongInt(ADM_CFG_VUS_LOG_MAX_SIZE, ADM_CFG_VUS_LOG_MAX_SIZE_DEFAULT_VAL);
    logMaxRotate = adm_cfg_iniParserGetInt(ADM_CFG_VUS_LOG_MAX_ROTATE, ADM_CFG_VUS_LOG_MAX_ROTATE_DEFAULT_VAL);
	bLogOutputTerminal = adm_cfg_iniParserGetBoolean(ADM_CFG_VUS_LOG_OUTPUT_TERMINAL, (DBOOL)ADM_CFG_VUS_LOG_OUTPUT_TERMINAL_DEFAULT_VAL);

    g_llogStartTime = time(NULL);
    // adm_debug_logInit(logDir, logMaxSize, logMaxRotate);

    if (ADM_ECODE_SUCCESSED == adm_ecs_getLogPrintLevel(&logPrintLevel)) {
        if (ELOG_LVL_ERROR == logPrintLevel) {
            u8cLogPrintLevel = ELOG_LVL_ERROR;
        } else if (ELOG_LVL_WARN == logPrintLevel) {
            u8cLogPrintLevel = ELOG_LVL_WARN;
        } else if (ELOG_LVL_INFO == logPrintLevel) {
            u8cLogPrintLevel = ELOG_LVL_INFO;
        } else if (ELOG_LVL_DEBUG == logPrintLevel) {
            u8cLogPrintLevel = ELOG_LVL_DEBUG;
        } else {
            log_w("logPrintLevel:%d in ecs config is invalid", logPrintLevel);
        }
    }

    (void)printf("Set logPrintLevel is %d\n", u8cLogPrintLevel);

    adm_debug_logInitWithFileName(logDir, "admfotaapp", logMaxSize, logMaxRotate, u8cLogPrintLevel, bLogOutputTerminal);

    ADM_APC_CORE_INIT_EXIT;

/*    if (ADM_ECODE_SUCCESSED != iRet) {
        //!< InitI_008
        adm_apc_eventAdd(ADM_APC_EV_STAGE_ING, 8, ADM_OCS_EVENT_LEVEL_ERROR, iRet, "adm_apc_logInit failed");
    } else {
        //!< InitI_009
        adm_apc_eventAdd(ADM_APC_EV_STAGE_ING, 9, ADM_OCS_EVENT_LEVEL_TRACE, 0, NULL);
    }
*/
    adm_apc_eventAdd(ADM_APC_EV_STAGE_ING, 9, ADM_OCS_EVENT_LEVEL_TRACE, 0, NULL);

    return iRet;
}

/**
 * @brief 日志模块的反初始化
 * 
 */
void adm_apc_logDeinit(void)
{
    ADM_APC_CORE_INIT_ENTRY;
    // 反初始化日志模块
    adm_debug_logDeinit();

    //!< InitE_14
    adm_apc_eventAdd(ADM_APC_EV_STAGE_END, 14, ADM_OCS_EVENT_LEVEL_TRACE, 0, NULL);
    ADM_APC_CORE_INIT_EXIT;
    return;
}

/**
 * @brief Tracer工具初始化
 * 
 */
void adm_apc_tracerInit(void)
{
    DCHAR *appId=NULL;
    DCHAR appIdStr[4]={0};
    
    (void)adm_strcpy(appIdStr,adm_cfg_iniParserGetString(ADM_CFG_VUS_APC_APPID, ADM_CFG_VUS_APC_APPID_DEFAULT_VAL),sizeof(appIdStr)); 

    appId = appIdStr;

    //!< InitI_010
    adm_apc_eventAdd(ADM_APC_EV_STAGE_ING, 10, ADM_OCS_EVENT_LEVEL_TRACE, 0, NULL);
    /**  捕捉系统信号：SIGSEGV SIGABRT SIGPIPE，并输出堆栈日志 */
    adm_backtrace();

    ADM_APC_CORE_INIT_ENTRY;
    // 初始化Tracer Tool
#ifdef __MACRO_DTS_TOOL_SWITCH__
    (void)adm_tracer_entry(appId, (DTS_MOD_INFO_T*)((void*)modInfo));
#endif
    
    ADM_APC_CORE_INIT_EXIT;

    //!< InitI_012
    adm_apc_eventAdd(ADM_APC_EV_STAGE_ING, 12, ADM_OCS_EVENT_LEVEL_TRACE, 0, NULL);
    
    return;
}

/**
 * @brief Tracer工具反初始化
 * 
 */
void adm_apc_tracerDeinit(void)
{
    ADM_APC_CORE_INIT_ENTRY;
    // 反初始化Tracer Tool
#ifdef __MACRO_DTS_TOOL_SWITCH__
    (void)adm_tracer_exit();
#endif
    ADM_APC_CORE_INIT_EXIT;

    //!< InitE_13
    // adm_apc_eventAdd(ADM_APC_EV_STAGE_END, 13, ADM_OCS_EVENT_LEVEL_TRACE, 0, NULL);
    return;
}

/**
 * @brief 打印所有模块的版本信息，包括：VUS/SYSI/Foundataion/ewebserver/vrpc等
 * 
 */
void adm_apc_printSdkVersion(void)
{
    DCHAR* allModuleVer = NULL;

    //!< InitI_022
    adm_apc_eventAdd(ADM_APC_EV_STAGE_ING, 22, ADM_OCS_EVENT_LEVEL_TRACE, 0, NULL);
    ADM_APC_CORE_INIT_ENTRY;
    //!< 输出所有模块的版本信息
    allModuleVer = adm_sdk_getAllModulesVersionFormatJsonStr();
    if (NULL != allModuleVer) {
        log_i("allModuleVersionStr:%s", allModuleVer);
    }

    //!< InitI_022
    adm_apc_eventAdd(ADM_APC_EV_STAGE_ING, 22, ADM_OCS_EVENT_LEVEL_TRACE, 0, allModuleVer);
    adm_free(allModuleVer);
    
    ADM_APC_CORE_INIT_EXIT;
}

/**
 * @brief Core service初始化
 * 
 * @return D32S     0:Successed, None-0:Failed
 */
D32S adm_apc_coreServiceInit(void)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    const DCHAR* configVal = NULL;


    do {
        //!< InitI_013
        adm_apc_eventAdd(ADM_APC_EV_STAGE_ING, 13, ADM_OCS_EVENT_LEVEL_TRACE, 0, NULL);

        //!< 初始化目录
        //!< 初始化KV目录
        configVal = adm_cfg_iniParserGetString(ADM_CFG_VUS_KV_STORAGE_DIR, ADM_CFG_VUS_KV_STORAGE_DIR_DEFAULT_VAL);
        if (FALSE == adm_core_service_initRuntimeDir(ADM_CORE_SERVICE_RUNTIME_DIR_TYPE_KV_STORAGE, configVal)) {
            log_e("adm_core_service_initRuntimeDir eType:%d, dir:%s failed", ADM_CORE_SERVICE_RUNTIME_DIR_TYPE_KV_STORAGE, configVal);
            break;
        }

        //!< 初始化日志目录
        configVal = adm_cfg_iniParserGetString(ADM_CFG_VUS_LOG_STORAGE_DIR, ADM_CFG_VUS_LOG_STORAGE_DIR_DEFAULT_VAL);
        if (FALSE == adm_core_service_initRuntimeDir(ADM_CORE_SERVICE_RUNTIME_DIR_TYPE_LOG, configVal)) {
            log_e("adm_core_service_initRuntimeDir eType:%d, dir:%s failed", ADM_CORE_SERVICE_RUNTIME_DIR_TYPE_LOG, configVal);
            break;
        }

        //!< 初始化升级包存储目录
        configVal = adm_cfg_iniParserGetString(ADM_CFG_VUS_PKG_STORAGE_DIR, ADM_CFG_VUS_PKG_STORAGE_DIR_DEFAULT_VAL);
        if (FALSE == adm_core_service_initRuntimeDir(ADM_CORE_SERVICE_RUNTIME_DIR_TYPE_PACKAGE, configVal)) {
            log_e("adm_core_service_initRuntimeDir eType:%d, dir:%s failed", ADM_CORE_SERVICE_RUNTIME_DIR_TYPE_PACKAGE, configVal);
            break;
        }

        //!< 初始化证书存储目录
        configVal = adm_cfg_iniParserGetString(ADM_CFG_VUS_CERT_STORAGE_DIR, ADM_CFG_VUS_CERT_STORAGE_DIR_DEFAULT_VAL);
        if (FALSE == adm_core_service_initRuntimeDir(ADM_CORE_SERVICE_RUNTIME_DIR_TYPE_CERT, configVal)) {
            log_e("adm_core_service_initRuntimeDir eType:%d, dir:%s failed", ADM_CORE_SERVICE_RUNTIME_DIR_TYPE_CERT, configVal);
            break;
        }

        //!< 初始化文件解压目录
        configVal = adm_cfg_iniParserGetString(ADM_CFG_VUS_UNZIP_STORAGE_DIR, ADM_CFG_VUS_UNZIP_STORAGE_DIR_DEFAULT_VAL);
        if (FALSE == adm_core_service_initRuntimeDir(ADM_CORE_SERVICE_RUNTIME_DIR_TYPE_UNZIP, configVal)) {
            log_e("adm_core_service_initRuntimeDir eType:%d, dir:%s failed", ADM_CORE_SERVICE_RUNTIME_DIR_TYPE_UNZIP, configVal);
            break;
        }

        //!< 初始化Kv storage
        iRet = adm_apc_kvStorageInit();
        if (ADM_ECODE_SUCCESSED != iRet) {
            iRet = ADM_ECODE_CORE_SERVICE_INIT_FAILED;
            log_e("adm_apc_kvStorageInit failed, err:%d", iRet);
            break;
        }

        //!< 初始化车云通信使用的密钥
        configVal = adm_cfg_iniParserGetString(ADM_CFG_VUS_OTA_SRV_DEFAULT_SECRET, ADM_CFG_VUS_OTA_SRV_DEFAULT_SECRET_DEFAULT_VAL);
        if (FALSE == adm_core_service_initOcsDefaultSecret(configVal)) {
            iRet = ADM_ECODE_CORE_SERVICE_INIT_FAILED;
            log_e("adm_core_service_initOcsDefaultSecret failed, err:%d", iRet);
            break;
        }
    } while(FALSE);

    if (ADM_ECODE_SUCCESSED != iRet) {
        //!< InitI_014
        adm_apc_eventAdd(ADM_APC_EV_STAGE_ING, 14, ADM_OCS_EVENT_LEVEL_ERROR, iRet, "adm_apc_coreServiceInit failed");
    } else {
        //!< Init_015
        adm_apc_eventAdd(ADM_APC_EV_STAGE_ING, 15, ADM_OCS_EVENT_LEVEL_TRACE, 0, NULL);
    }
    return iRet;
}

/**
 * @brief Core service反初始化
 * 
 */
void adm_apc_coreServiceDeinit(void)
{
    adm_core_service_deinitOcsDefaultSecret();

    adm_core_service_deinit();
    
    adm_core_service_deinitRuntimeDir(ADM_CORE_SERVICE_RUNTIME_DIR_TYPE_KV_STORAGE);
    adm_core_service_deinitRuntimeDir(ADM_CORE_SERVICE_RUNTIME_DIR_TYPE_LOG);
    adm_core_service_deinitRuntimeDir(ADM_CORE_SERVICE_RUNTIME_DIR_TYPE_PACKAGE);
    adm_core_service_deinitRuntimeDir(ADM_CORE_SERVICE_RUNTIME_DIR_TYPE_CERT);
    adm_core_service_deinitRuntimeDir(ADM_CORE_SERVICE_RUNTIME_DIR_TYPE_UNZIP);
    //!< InitE_12
    // adm_apc_eventAdd(ADM_APC_EV_STAGE_END, 12, ADM_OCS_EVENT_LEVEL_TRACE, 0, NULL);
}


/**
 * @brief kv storage初始化
 * 
 * @return D32S     0:Successed, None-0:Failed
 */
D32S adm_apc_kvStorageInit(void)
{
    ADM_APC_CORE_INIT_ENTRY;
    D32S iRet = ADM_ECODE_FAILED;
    DBOOL isKvstorageEnc = FALSE;

    do {
        //!< 为kvstorage初始化加解密接口，通过配置文件中获取是否需要加密
        isKvstorageEnc = adm_cfg_iniParserGetBoolean(ADM_CFG_VUS_IS_KVSTORAGE_ENCRYPT, (DBOOL)ADM_CFG_VUS_IS_KVSTORAGE_ENCRYPT_DEFAULT_VAL);
        if (FALSE == isKvstorageEnc) {
            adm_com_kvstorageInit(NULL, NULL);
        } else {
            adm_com_kvstorageInit(adm_iss_encryptString, adm_iss_decryptString);
        }

        //!< 初始化core中 kvstorage数据库
        iRet = adm_core_service_init();
        if (ADM_ECODE_SUCCESSED != iRet) {
            log_e("adm_core_service_init failed, err:%d", iRet);
            break;
        }
    } while(FALSE);


    ADM_APC_CORE_INIT_EXIT;
    return iRet;
}

/**
 * @brief kv storage反初始化
 * 
 */
void adm_apc_kvStorageDeinit(void)
{
    ADM_APC_CORE_INIT_ENTRY;
    //!< 反初始化core中 kvstorage数据库
    adm_core_service_deinit();
    //!< 反初始化kvstorage的加解密接口
    adm_com_kvstorageDeinit();
    ADM_APC_CORE_INIT_EXIT;
    return;
}

/**
 * @brief 初始化SOA Session
 * 
 * @return ADM_SOA_SESSION_T*   NULL:Failed, None-NULL:Soa Session handler 
 */
static ADM_SOA_SESSION_T* adm_apc_soaSessionInit(void)
{
    ADM_APC_CORE_INIT_ENTRY;
    static DBOOL isInited = FALSE;
    D32S iRet = ADM_ECODE_FAILED;
    D32S instanceId = 0;

    if (TRUE == isInited) {
        return g_pstSoaSession;
    }

	void* priv = NULL;
	D32S soaImplType = adm_cfg_iniParserGetInt(ADM_CFG_VUS_SOA_IMPL_TYPE,  ADM_CFG_VUS_SOA_IMPL_TYPE_DEFAULT_VAL);
	if(ADM_SOA_IMPL_BY_ARPC == (ADM_SOA_IMPL_TYPE_E)soaImplType)
	{
		instanceId = adm_cfg_iniParserGetInt(ADM_CFG_VUS_UIM_INSTANCEID,  ADM_CFG_VUS_UIM_INSTANCEID_DEFAULT_VAL);
		priv = &instanceId;
	}
	else if(ADM_SOA_IMPL_BY_SOMEIP == (ADM_SOA_IMPL_TYPE_E)soaImplType)
	{
		const DCHAR* unicastIp = adm_cfg_iniParserGetString(ADM_CFG_VRPC_SOMEIP_UNICAST_IP,  ADM_CFG_VRPC_SOMEIP_UNICAST_IP_DEFAULT_VAL);
		priv = adm_strdup(unicastIp);
	}
	else
	{
		log_e("invaild soa impl type");
		return NULL;
	}
	
    g_pstSoaSession = adm_soa_init(priv);
    if (NULL == g_pstSoaSession) {
        iRet = ADM_ECODE_SOA_INIT_FAILED;
        log_e("adm_soa_init failed, iRet:%d", iRet);
    }  else {
        log_i("adm_soa_init successed");
        isInited = TRUE;
    }

    ADM_APC_CORE_INIT_EXIT;
    if(ADM_SOA_IMPL_BY_SOMEIP == (ADM_SOA_IMPL_TYPE_E)soaImplType)
    {
        adm_free(priv);
    }
    return g_pstSoaSession;
}

/**
 * @brief 反初始化 SOA Session
 * 
 * @param pstSoaSession Handler of SOA session
 */
static void adm_apc_soaSessionDeinit(ADM_SOA_SESSION_T* pstSoaSession)
{
    ADM_APC_CORE_INIT_ENTRY;
    adm_soa_deinit();
    ADM_APC_CORE_INIT_EXIT;
}

/**
 * @brief ocm初始化
 * 
 * @return D32S     0:Successed, None-0:Failed
 */
D32S adm_apc_ocm_init(void)
{
    D32U uiServerType = 1;

	ADM_APC_CORE_INIT_ENTRY;

    (void)adm_ems_getServerType(&uiServerType);

    //!< InitI_050
    adm_apc_eventAdd(ADM_APC_EV_STAGE_ING, 50, ADM_OCS_EVENT_LEVEL_TRACE, 0, NULL);
	//初始化OCM
	D32S iRet = ADM_ECODE_FAILED;
	iRet = adm_ocm_init(adm_ecs_updateAndGetConfigInfo, 
                        adm_apc_kvStorageInit, 
                        adm_apc_otaFirstBlood, 
                        adm_eim_getEcuInfoManagerRealTimeUpdate, 
                        adm_vsm_stateDel,
                        adm_apc_logInit,
                        uiServerType);

	ADM_APC_CORE_INIT_EXIT;
    if (ADM_ECODE_SUCCESSED != iRet) {
        //!< InitI_051
        adm_apc_eventAdd(ADM_APC_EV_STAGE_ING, 51, ADM_OCS_EVENT_LEVEL_ERROR, iRet, "adm_ocm_init failed");
    } else {
        //!< InitI_052
        adm_apc_eventAdd(ADM_APC_EV_STAGE_ING, 52, ADM_OCS_EVENT_LEVEL_TRACE, 0, NULL);
    }

	return iRet;
}

//TODO:add vdi vehicleSignalEventCallback

/**
 * @brief DIM 模块初始化
 * 
 * @return D32S     0:Successed, None-0:Failed
 */
D32S adm_apc_dimInit(void)
{
    ADM_APC_CORE_INIT_ENTRY;
    // 初始化DIM
    D32S iRet = ADM_ECODE_FAILED;
    ADM_DIM_SOA_VDI_INIT_INFO_T vdiInitInfo = {0};
    ADM_DIM_SOA_HOST_INIT_INFO_T hostInitInfo = {0};
    ADM_DIM_SOA_UCL_INIT_INFO_T uclInitInfo = {0};
    ADM_DIM_SOA_TRANS_HOST_INIT_INFO_T transInitInfo = {0};
    ADM_DIM_SOA_DPO_INIT_INFO_T dpoinitInfo = {0};
    ADM_SOA_SESSION_T* pstSoaSession = NULL;

    do
    {
        //!< InitI_016
        adm_apc_eventAdd(ADM_APC_EV_STAGE_ING, 16, ADM_OCS_EVENT_LEVEL_TRACE, 0, NULL);
        pstSoaSession = adm_apc_soaSessionInit();
        if (NULL == pstSoaSession) {
            iRet = ADM_ECODE_DIM_INIT_FAILED;
            log_e("adm_apc_soaSessionInit dim failed, iRet:%d", iRet);
            break;
        } else {
            log_i("adm_apc_soaSessionInit dim successed");
        }

		if (ADM_ECODE_SUCCESSED != adm_dim_init(NULL)) {
            log_e("adm_dim_init failed, iRet:%d", iRet);
            break;
        } else {
            log_i("adm_dim_init successed");
        }    

        // TODO: 需要为各dim模块注册回调
        // TODO: 胡杰填充
        /** 初始化dim_vdi */
        vdiInitInfo.readDiagInfoCallback = adm_eim_parallelCollectEcuNotifyHandle;
        vdiInitInfo.reflashProgressCallback = adm_ipm_dimReflashProgressNotify;
        vdiInitInfo.reflashResultCallback = adm_ipm_dimReflashResultSend;
        vdiInitInfo.spyVehicleStatusCallback = adm_dim_vdiSpyVehicleStatusCallback;
        vdiInitInfo.vehicleSignalEventCallback = adm_vsm_dimSoaVdiSignalEventNotify;
        vdiInitInfo.getVehicleTypeCallback = apcGetVehicleType;
        iRet = adm_dim_vdi_init(&vdiInitInfo, adm_ipm_dimSoaUcmPrivGet());
        if (ADM_ECODE_SUCCESSED != iRet) {
            log_e("adm_dim_soa_vdi_init failed, iRet:%d", iRet);
            break;
        } else {
            log_i("adm_dim_soa_vdi_init successed");
        }

        // TODO: 胡杰填充
        /** 初始化 dim_host */
        hostInitInfo.signalEventNotifyCallback = adm_vsm_dimSoaHostSignalEventNotify;
        hostInitInfo.triggerEventNotifyCallback = adm_vsm_dimSoaHostTriggerEventNotify;
        hostInitInfo.tspTaskDealCallback = adm_vsm_dimSoaHostTspTaskDeal;
        iRet = adm_dim_host_init(&hostInitInfo, adm_vsm_dimSoaHostPrivGet());
        if (ADM_ECODE_SUCCESSED != iRet) {
            log_e("adm_dim_soa_host_init failed, iRet:%d", iRet);
            break;
        } else {
            log_i("adm_dim_soa_host_init successed");
        }

        // TODO: 佳佳填充
        /** 初始化 dim_ucl */
        uclInitInfo.decryptProgressNotifyCallback = adm_pub_dimUclDecryptProgressHandle;
        uclInitInfo.decryptResultNotifyCallback   = adm_pub_dimUclDecryptResultHandle;
        uclInitInfo.readPartInfoCallback = adm_eim_parallelCollectEcuNotifyHandle;
        uclInitInfo.upgradeProgressNotifyCallback = adm_ipm_dimSoaUcmUpgradeProgressNotify;
        uclInitInfo.upgradeResultNotifyCallback = adm_ipm_dimSoaUcmUpgradeResultNotify;
        uclInitInfo.upgradeIccProgressNotifyCallback = adm_ipm_dimSoaUcmUpgradeIccProgressNotify;
        uclInitInfo.upgradeIccResultNotifyCallback = adm_ipm_dimSoaUcmUpgradeIccResultNotify;
        uclInitInfo.verifyPackageHashProgressNotifyCallback = adm_dpm_dimVerifyHashProgressNotify;
        uclInitInfo.verifyPackageHashResultNotifyCallback = NULL;
		uclInitInfo.verifySignProgressNotifyCallback = adm_pub_dimUclVerifySignProgressHandle;
        uclInitInfo.verifySignResultNotifyCallback = adm_pub_dimUclVerifySignResultHandle;
        iRet = adm_dim_ucl_init(&uclInitInfo, adm_ipm_dimSoaUcmPrivGet());
        if (ADM_ECODE_SUCCESSED != iRet) {
            log_e("adm_dim_soa_ucl_init failed, iRet:%d", iRet);
            break;
        } else {
            log_i("adm_dim_soa_ucl_init successed");
        }

        //!< tpm 依赖 ewebserver 的支持 
#ifdef CMAKE_CUSTOM_EWEBSERVER_SUPPORT
    /** 初始化 dim_transfer */
    transInitInfo.transDownloadFromHostProgressNotifyCallback = adm_tpm_dimMasterTransProgressHandle;
    transInitInfo.transDownloadFromHostResultNotifyCallback   = adm_tpm_dimMasterTransResultHandle;
    transInitInfo.transUploadToHostProgressNotifyCallback     = adm_tpm_dimUploadToHostProgressHandle;
    transInitInfo.transUploadToHostResultNotifyCallback       = adm_tpm_dimUploadToHostResultHandle;
    iRet = adm_dim_transferOnHost_init(&transInitInfo, NULL);
    if (ADM_ECODE_SUCCESSED != iRet) {
        log_e("adm_dim_soa_transferOnHost failed, iRet:%d", iRet);
        break;
    } else {
        log_i("adm_dim_soa_transferOnHost successed");
    }
#endif

        /** 初始化 dim_dpo */
        dpoinitInfo.downProgressNotifyCallback = adm_dpm_dimSlaveDownProgressHandle;
        dpoinitInfo.downloadResultNotifyCallback = adm_dpm_dimSlaveDownResultHandle;
        iRet = adm_dim_dpo_init(&dpoinitInfo, NULL);
        if (ADM_ECODE_SUCCESSED != iRet) {
            log_e("adm_dim_dpo_init failed, iRet:%d", iRet);
            break;
        } else {
            log_i("adm_dim_dpo_init successed");
        }
        
    } while (FALSE);

    ADM_APC_CORE_INIT_EXIT;

    if (ADM_ECODE_SUCCESSED != iRet) {
        //!< InitI_017
        adm_apc_eventAdd(ADM_APC_EV_STAGE_ING, 17, ADM_OCS_EVENT_LEVEL_ERROR, iRet, "adm_dpm_moduleInit failed");
    } else {
        //!< InitI_018
        adm_apc_eventAdd(ADM_APC_EV_STAGE_ING, 18, ADM_OCS_EVENT_LEVEL_TRACE, 0, NULL);
    }

    return iRet;
}

/**
 * @brief DIM 模块反初始化
 * 
 */
void adm_apc_dimDeinit(void)
{
    ADM_APC_CORE_INIT_ENTRY;
    // 反初始化DIM
    adm_dim_transferOnHost_deinit();
    adm_dim_dpo_deinit();
    adm_dim_ucl_deinit();
    adm_dim_host_deinit();
    adm_dim_vdi_deinit();
    adm_soa_deinit();
    (void)adm_dim_deInit();
    ADM_APC_CORE_INIT_EXIT;
    //!< InitE_11
    adm_apc_eventAdd(ADM_APC_EV_STAGE_END, 11, ADM_OCS_EVENT_LEVEL_TRACE, 0, NULL);
}

/** 修正下载模块触发类型 */
static void __adm_apc_handleDpmTrigger()
{
    ADM_CVS_UPGRADE_MODE_E eMode = ADM_CVS_UPGRADE_MODE_INVALID;
    D32S iResult = ADM_ECODE_FAILED;
    ADM_APPLICATION_COMMON_CMDID_TYPE_E eCmdId = ADM_APPLICATION_COMMON_CMDID_TYPE_INVALID;

    if (NULL != adm_dpm_getDownInfo() && (ADM_APPLICATION_COMMON_CMDID_TYPE_INVALID == adm_dpm_getDownInfo()->eTriggerType))
    {
        iResult = adm_cvs_getUpgradeMode((D32S *)&eMode);
        if (ADM_IS_SUCCESS(iResult))
        {
            switch (eMode)
            {
                case ADM_CVS_UPGRADE_MODE_SLIENCE_DOWNLOAD:
                case ADM_CVS_UPGRADE_MODE_SLIENCE_INSTALL:
                    eCmdId = ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_SILENCE;
                    break;

                case ADM_CVS_UPGRADE_MODE_USB_CHECK:
                    eCmdId = ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_USB;
                    break;

                case ADM_CVS_UPGRADE_MODE_RESCUE:
                    eCmdId = ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_RESCUE;
                    break;

                case ADM_CVS_UPGRADE_MODE_PROD_LINE:
                    eCmdId = ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_PROD_LINE;
                    break;

                case ADM_CVS_UPGRADE_MODE_APK_SELF:
                    eCmdId = ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_APK_SELF_UPGRADE;
                    break;

                case ADM_CVS_UPGRADE_MODE_FORCE:
                    eCmdId = ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_FORCE;
                    break;

                default:
                    eCmdId = ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_USER;
                    break;
            }

            adm_dpm_getDownInfo()->eTriggerType = eCmdId;
        }
    }

    return;
}

static DVOID apcNotifyHmiOtaReady()
{
	D32S iRet = ADM_ECODE_FAILED;
	D32S iErrcode = ADM_ECODE_FAILED;

	while (DTRUE)
	{
		iRet = adm_dim_host_notifyOtaHmiStart(adm_soa_getHostInstanceId(), 15, &iErrcode);
		if (ADM_IS_ERR(iRet) || ADM_IS_ERR(iErrcode))
		{
			log_w("Notify hmi ota ready failed. iRet[%d] iErrcode[%d]", iRet, iErrcode);
		}
		else
		{
			log_i("Notify hmi ota ready success.");
			break;
		}
	}

	return;
}

/**
 * @brief 状态机处理函数，在HMI上线后，根据不同的状态机，通知HMI当前的状态
 * 
 * @param priv 保留参数
 */
void __adm_apc_handleUimOnline(void* priv)
{
    D32S iRet  = ADM_ECODE_FAILED;
    ADM_VSM_STATE_E eState = ADM_VSM_STATE_MAX;
    ADM_DPS_DOWNRESULT_T *pstResult = NULL;

    //TODO: 处理状态机，根据不同状态机，通知HMI我们的状态
    adm_vsm_setHmiOnline(TRUE);
    eState = adm_vsm_stateGet();
    log_i("HMI is online, notify HMI according to the current status:[%d]", eState);

    adm_uim_ensureCheckSubedEventSuccess(ADM_UIM_INSTALL_SERVICE_ID, ADM_UIM_TSP_SCHDULE_TIME_EVENT_ID);
    switch (eState)
    {
    case ADM_VSM_STATE_IDLE:
		adm_uim_vsmStateNotify((D32S)ADM_VSM_STATE_IDLE);
        adm_ipm_HandleRecover();
		apcNotifyHmiOtaReady();
        break;
    case ADM_VSM_STATE_CHECK_DOING:
		apcNotifyHmiOtaReady();
		break;
    case ADM_VSM_STATE_DOWNLOAD_READY:
        adm_cvm_checkResultNotify();
		apcNotifyHmiOtaReady();
		break;
    case ADM_VSM_STATE_DOWNLOAD_DOING:
    case ADM_VSM_STATE_DOWNLOAD_PAUSE:
        (void)adm_dpm_downloadResultReport();
		apcNotifyHmiOtaReady();
		break;
    case ADM_VSM_STATE_UPGRADE_READY:
    case ADM_VSM_STATE_UPGRADE_PAUSE:
    {
        __adm_apc_handleDpmTrigger();
        pstResult = adm_dpm_createDownResultInfo(ADM_ECODE_SUCCESSED, NULL);
        iRet = adm_dpm_policyDownResultNotify2Uim(pstResult, "downloading");
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to notify download policy result, err[%d]", iRet);
        }
        adm_dpm_freeDownResultInfo(pstResult);
		apcNotifyHmiOtaReady();
		break;
    }
    case ADM_VSM_STATE_UPGRADE_DOING:
    case ADM_VSM_STATE_UPGRADE_BROKEN:
        (void)adm_ipm_uimAllEcuInstallResultNotify();
		apcNotifyHmiOtaReady();
		break;
    case ADM_VSM_STATE_MAX:
    default:
        log_i("HMI is online, but eState:%d is not invalid", eState);
        break;
    }

    return;
}

/**
 * @brief 状态机处理函数，当HMI不在线时，通知状态机
 * 
 * @param priv 保留参数
 */
static void __adm_apc_handleUimOffline(void* priv)
{
    adm_vsm_setHmiOnline(FALSE);
    return;
}


/**
 * @brief UIM 模块初始化
 * 
 * @return D32S     0:Successed, None-0:Failed
 */
D32S adm_apc_uimInit(void)
{
    ADM_APC_CORE_INIT_ENTRY;
    D32S iRet = ADM_ECODE_FAILED;
    ADM_UIM_EMS_INIT_INFO_T emsInitInfo = {0};
    ADM_UIM_CHECK_INIT_INFO_T checkInitInfo = {0};
    ADM_UIM_DOWNLOAD_INIT_INFO_T downInitInfo = {0};
    ADM_UIM_INSTALL_INIT_INFO_T installInitInfo = {0};
    ADM_UIM_APK_READY_INIT_INFO_T apkReadyInitInfo= {0};
    ADM_UIM_ICP_INIT_INFO_T icpInitInfo = {0};
    ADM_SOA_SESSION_T* pstSoaSession = NULL;

    // 初始化UIM

    do {
        //!< InitI_019
        adm_apc_eventAdd(ADM_APC_EV_STAGE_ING, 19, ADM_OCS_EVENT_LEVEL_TRACE, 0, NULL);
        pstSoaSession = adm_apc_soaSessionInit();
        if (NULL == pstSoaSession) {
            iRet = ADM_ECODE_UIM_INIT_FAILED;
            log_e("adm_soa_init uim failed, iRet:%d", iRet);
            break;
        } else {
            log_i("adm_soa_init uim successed");
        }

        /** 启动服务 */
        if (ADM_ECODE_SUCCESSED != adm_uim_init()) {
            log_e("adm_uim_init failed, iRet:%d", iRet);
            break;
        } else {
            log_i("adm_uim_init successed");
        }
        
        (void)adm_uim_serviceInit((void*)pstSoaSession);
        (void)adm_soa_start();

        // TODO: 需要为各uim模块注册回调
        emsInitInfo.emsVehicleStateCtrlCallback = adm_ems_vehicleStateCtrlCallback;
        emsInitInfo.emsOTAServerSwitchCallback = adm_ems_otaServerSwitchCallback;
        emsInitInfo.emsOTAServerConnCallback = adm_ems_otaServerConnectCallback;
        emsInitInfo.emsOTAAppCtrlCallback = adm_ems_vehicleAppCtrlCallback;
        iRet = adm_uim_emsInit(&emsInitInfo, DNULL);
        if (ADM_ECODE_SUCCESSED != iRet)
        {
            log_e("adm_uim_emsInit failed, iRet:%d", iRet);
            break;
        }
        else
        {
            log_i("adm_uim_emsInit successed");
        }
        
        checkInitInfo.checkTriggerCallback = adm_cvm_checkTriggerCallback;
        checkInitInfo.usbCheckTriggerCallback = adm_cvm_usbCheckTriggerCallback;
        checkInitInfo.queryResultCallback = adm_cvm_queryResultCallback;
        checkInitInfo.checkQueryEcuInfoCallback = adm_cvm_checkQueryEcuInfoCallback;
        
        iRet = adm_uim_checkInit(&checkInitInfo, NULL);
        if (ADM_ECODE_SUCCESSED != iRet) {
            log_e("adm_uim_checkInit failed, iRet:%d", iRet);
            break;
        } else {
            log_i("adm_uim_checkInit successed");
        }
        icpInitInfo.queryResultCallback=adm_icpm_queryResultCallback;
        iRet = adm_uim_icpInit(&checkInitInfo, NULL);
        if (ADM_ECODE_SUCCESSED != iRet) {
            log_e("adm_uim_icpInit failed, iRet:%d", iRet);
            break;
        } else {
            log_i("adm_uim_icpInit successed");
        }

        // TODO: 1. 触发条件校验 conditionsVerifiedCallback ---> adm_dpm_uimDownloadConditionsVerified
        // TODO: 2. vus主控使用 adm_dpm_checkDownloadPreCondition 接口进行校验
        // TODO: 3. vus通知前置条件校验结果 adm_dpm_downCondVerifyResultNotify2Uim
        // TODO: 4. 触发下载 adm_dpm_uimTriggerDownload 
        downInitInfo.downloadPauseCallback          = adm_dpm_uimPauseDownload;
        downInitInfo.downloadCancelCallback         = adm_dpm_uimCancelDownload;
        downInitInfo.downloadTriggerCallback        = adm_dpm_uimTriggerDownload;
        downInitInfo.conditionsVerifiedCallback     = adm_dpm_uimDownloadConditionsVerified;
        downInitInfo.allEcuDownloadResultCallback   = adm_dpm_uimGetAllEcuDownloadResult;
        downInitInfo.allEcuDownloadProgressCallback = adm_dpm_uimGetAllEcuDownloadProgress;
        downInitInfo.autoDownloadSwitchSetCallback  = adm_dpm_uimSetAutoDownloadSwitch;
        iRet = adm_uim_downloadInit(&downInitInfo, NULL);
        if (ADM_ECODE_SUCCESSED != iRet) {
            log_e("adm_uim_downloadInit failed, iRet:%d", iRet);
            break;
        } else {
            log_i("adm_uim_downloadInit successed");
        }

        //apk ready消息回调
        apkReadyInitInfo.behaviorOnlineCallback = __adm_apc_handleUimOnline;
        iRet = adm_uim_apkReadyNoticeInit(&apkReadyInitInfo,NULL);
        if (ADM_ECODE_SUCCESSED != iRet) 
        {
            log_e("adm_uim_apkReadyNoticeInit, iRet:%d", iRet);
            break;
        }
        else 
        {
            log_i("adm_uim_apkReadyNoticeInit successed");
        }

        // TODO: 红伟填充
        installInitInfo.allEcuInstallProgressCallback = adm_ipm_uimAllEcuInstallProgressCallback;
        installInitInfo.allEcuInstallResultCallback = adm_ipm_uimAllEcuInstallResultCallback;
        installInitInfo.cancelInstallCallback = adm_ipm_uimCancelInstallCallback;
        installInitInfo.conditionsVerifiedCallback = adm_ipm_uimConditionsVerifiedCallback;
        installInitInfo.installTriggerCallback = adm_ipm_uimInstallTriggerCallback;
        installInitInfo.refuseInstallCallback = adm_ipm_uimRefuseInstallCallback;
        installInitInfo.schduleInstallCallback = adm_ipm_uimSchduleInstallCallback;//modify by wj 20211207
        installInitInfo.querySmartScenarioCallback = adm_ipm_getIsSmartScenario;
        installInitInfo.notifyConfirmCallback = adm_ipm_notifyConfirmHandle;
        iRet = adm_uim_installInit(&installInitInfo, adm_ipm_uimSoaPrivGet());
        if (ADM_ECODE_SUCCESSED != iRet) {
            log_e("adm_uim_installInit failed, iRet:%d", iRet);
            break;
        } else {
            log_i("adm_uim_installInit successed");
        }

        //TODO: 填充回调接口
        emsInitInfo.emsVehicleStateCtrlCallback = adm_ems_vehicleStateCtrlCallback;
        emsInitInfo.emsOTAServerConnCallback = adm_ems_otaServerConnectCallback;
        emsInitInfo.emsOTAServerSwitchCallback = adm_ems_otaServerSwitchCallback;
        emsInitInfo.emsOTAAppCtrlCallback = adm_ems_vehicleAppCtrlCallback;
        iRet = adm_uim_emsInit(&emsInitInfo, DNULL);
        if (ADM_ECODE_SUCCESSED != iRet)
        {
            log_e("adm_uim_emsInit failed, iRet:%d", iRet);
            break;
        }
        else
        {
            log_i("adm_uim_emsInit successed");
        }

        //!< 监听HMI是否已经上下，如果HMI上线，需要对应做相应的处理
//        adm_arpc_registerFindInstanceCb(adm_soa_getHmiInstanceId(), __adm_apc_handleUimOnline, NULL);
	    (void)adm_arpc_registerInstanceBehaviorCb(adm_soa_getHmiInstanceId(),INSTANCE_BEHAVIOR_ONLINE,__adm_apc_handleUimOnline,NULL);
        (void)adm_arpc_registerInstanceBehaviorCb(adm_soa_getHmiInstanceId(),INSTANCE_BEHAVIOR_OFFLINE,__adm_apc_handleUimOffline,NULL);
    } while(FALSE);

    ADM_APC_CORE_INIT_EXIT;
    if (ADM_ECODE_SUCCESSED != iRet) {
        //!< InitI_020
        adm_apc_eventAdd(ADM_APC_EV_STAGE_ING, 20, ADM_OCS_EVENT_LEVEL_ERROR, iRet, "adm_apc_uimInit");
    } else {
        //!< InitI_021
        adm_apc_eventAdd(ADM_APC_EV_STAGE_ING, 21, ADM_OCS_EVENT_LEVEL_TRACE, 0, NULL);
    }
    return iRet;
}

/**
 * @brief UIM 模块反初始化
 * 
 */
void adm_apc_uimDeinit(void)
{
    ADM_APC_CORE_INIT_ENTRY;

    // 反初始化UIM
    (void)adm_uim_serviceDeinit();
    (void)adm_uim_emsDeinit();
    (void)adm_uim_installDeinit();
    (void)adm_uim_downloadDeinit();
    (void)adm_uim_checkDeinit();
    (void)adm_uim_emsDeinit();
    adm_soa_deinit();
    (void)adm_uim_deInit();
    ADM_APC_CORE_INIT_EXIT;
    //!< InitE_10
    adm_apc_eventAdd(ADM_APC_EV_STAGE_END, 10, ADM_OCS_EVENT_LEVEL_TRACE, 0, NULL);
}


static D32S adm_apc_getApkVersion(DCHAR** ppApkVersion)
{
    *ppApkVersion = adm_strdup("ABUP_UCM_V0.1");

    return 0;
}

static D32S adm_apc_doApkSelfUpgrade(DCHAR* pApkPackPath, DCHAR* pApkVerion)
{
    return 0;
}

/**
 * @brief 向平台请求注册
 *
 * @return D32S
 */
static D32S adm_apc_doRegisterFromServer(void)
{
    D32S iRet = ADM_ECODE_FAILED;
    ADM_VRS_REGISTER_RSP_INFO_T *resp = NULL;

    do
    {
        resp = adm_vrs_newRegisterRspInfo();
        if (NULL == resp)
        {
            iRet = ADM_ECODE_NO_MEMORY;
            log_e("adm_vrs_newRegisterRspInfo new failed, iRet:%d", iRet);
            break;
        }

        iRet = adm_vrm_register(NULL, resp);
        if (ADM_ECODE_SUCCESSED == iRet)
        {
            if (NULL != resp->ext)
            {
                log_i("resp->ext:%s", resp->ext);
            }
            log_i("adm_vrm_register successed");
        }
        else
        {
            log_i("adm_vrm_register failed, iRet:%d", iRet);
        }

    } while (FALSE);

    adm_vrs_delRegisterRspInfo(resp);

    return iRet;
}

/**
 * @brief 探测当前网络状态
 * @note -1:连接失败,获取不到环境 10:测试环境连接成功， 20:生产环境连接成功，11:测试环境连接失败， 21:生产环境连接失败
 *
 * @return D32S
 */
static D32S adm_apc_detectConnect()
{
    D32S iResult = -1;
    // D32U uiServerType = ADM_EMS_SERVER_TYPE_DEFAULT; ///< 默认测试环境
    const DCHAR *pRegisterUrlStr = NULL;

    // (DVOID)adm_ems_getServerType(&uiServerType);

    // if (ADM_EMS_SERVER_TYPE_DEFAULT == uiServerType)
    // {
    //     pRegisterUrlStr = adm_cfg_iniParserGetString(ADM_CFG_VUS_OTA_SRV_REGISTER_URL, ADM_CFG_VUS_OTA_SRV_REGISTER_URL_DEFAULT_VAL);
    // }
    // else
    // {
    //     pRegisterUrlStr = adm_cfg_iniParserGetString(ADM_CFG_VUS_OTA_SRV_REGISTER_URL_2, ADM_CFG_VUS_OTA_SRV_REGISTER_URL_DEFAULT_VAL_2);
    // }

    pRegisterUrlStr = adm_ocm_getCommUrl(ADM_OCS_BUSINESS_TYPE_REGISTER);
    if (NULL == pRegisterUrlStr)
    {
        log_i("Can not find register url.");
        iResult = -1;
    }
    else
    {
        iResult = adm_apc_doRegisterFromServer();
        if (ADM_IS_SUCCESS(iResult))
        {
            log_i("Register is Ok.Get url type.");

            if (strstr(pRegisterUrlStr, "stg") != NULL)
            {
                log_i("Test envorinment.");
                iResult = 10;
            }
            else
            {
                log_i("Product envorinment.");
                iResult = 20;
            }
        }
        else
        {
            if (strstr(pRegisterUrlStr, "stg") != NULL)
            {
                log_i("Test envorinment.");
                iResult = 11;
            }
            else
            {
                log_i("Product envorinment.");
                iResult = 21;
            }
        }
    }

    adm_free(pRegisterUrlStr);

    return iResult;
}

/**
 * @brief 各模块初始化，包括：CVM、DPM、IPM等
 * 
 * @return D32S     0:Successed, None-0:Failed
 */
D32S adm_apc_managerInit(void)
{
    ADM_APC_CORE_INIT_ENTRY;
    D32S iRet = ADM_ECODE_FAILED;
    ADM_EMS_INIT_INFO_T stEmsInitInfo = {0};
    ADM_VSM_TSP_IMPL_T stTspImplInfo = {0};
    ADM_DPM_FUNC_CALL_BACK_T stDpmfnInfo = {0};
    ADM_SDK_APK_UPGRADE_INFO_T stApkUpgradeInfo = {0};

    // 各模块初始化，包括：CVM、DPM、IPM等
    do {
//        adm_vrm_init(adm_irm_getEventSequenceId, adm_irm_reportEvent);
        //!< InitI_023
        adm_apc_eventAdd(ADM_APC_EV_STAGE_ING, 23, ADM_OCS_EVENT_LEVEL_TRACE, 0, NULL);

        stApkUpgradeInfo.doApkSelfUpgradeCb = adm_apc_doApkSelfUpgrade;
        stApkUpgradeInfo.getApkVersionCb = adm_apc_getApkVersion;
        adm_sdk_selfUpgradeInit(&stApkUpgradeInfo);

        //!< 零件缓存初始化
	    adm_eim_getEcuInfoManagerInit();

        //!< InitI_026
        adm_apc_eventAdd(ADM_APC_EV_STAGE_ING, 26, ADM_OCS_EVENT_LEVEL_TRACE, 0, NULL);

//!< 断言 ewebserver 的支持 
#ifdef CMAKE_CUSTOM_EWEBSERVER_SUPPORT
    //!< ewebserver 初始化
    iRet = adm_tpm_startEwebServer();
    if (ADM_IS_ERR(iRet)) {
        log_e("adm_tpm_startEwebServer failed, iRet:%d", iRet);
        //!< InitI_027
        adm_apc_eventAdd(ADM_APC_EV_STAGE_ING, 27, ADM_OCS_EVENT_LEVEL_ERROR, iRet, "adm_tpm_startEwebServer failed");
        break;
    }
    //!< InitI_028
    adm_apc_eventAdd(ADM_APC_EV_STAGE_ING, 28, ADM_OCS_EVENT_LEVEL_TRACE, 0, NULL);
    log_i("adm_tpm_startEwebServer successed");
#endif

        //pub模块初始化
        iRet = adm_pub_moduleInit();
        if (ADM_IS_ERR(iRet)) 
        {
            log_e("adm_pub_moduleInit failed, iRet:%d", iRet);
            break;
        } 

        //!< InitI_029
        adm_apc_eventAdd(ADM_APC_EV_STAGE_ING, 29, ADM_OCS_EVENT_LEVEL_TRACE, 0, NULL);
        iRet = adm_ecm_moduleInit();
        if (ADM_IS_ERR(iRet)) 
        {
            log_e("adm_ecm_moduleInit failed, iRet:%d", iRet);
            //!< InitI_30
            adm_apc_eventAdd(ADM_APC_EV_STAGE_ING, 30, ADM_OCS_EVENT_LEVEL_ERROR, iRet, "adm_ecs_moduleInit failed");
            break;
        } 
        //!< InitI_031
        adm_apc_eventAdd(ADM_APC_EV_STAGE_ING, 31, ADM_OCS_EVENT_LEVEL_TRACE, 0, NULL);
        log_i("adm_ecm_moduleInit successed");

		adm_vrm_init(adm_irm_getEventSequenceId, adm_irm_reportEvent);

        //!< InitI_032
        adm_apc_eventAdd(ADM_APC_EV_STAGE_ING, 32, ADM_OCS_EVENT_LEVEL_TRACE, 0, NULL);
    	adm_pis_packageInit();
        //TODO: InitI_033
        //!< InitI_034
        adm_apc_eventAdd(ADM_APC_EV_STAGE_ING, 34, ADM_OCS_EVENT_LEVEL_TRACE, 0, NULL);

        //!< InitI_038
        adm_apc_eventAdd(ADM_APC_EV_STAGE_ING, 38, ADM_OCS_EVENT_LEVEL_TRACE, 0, NULL);
        //!< dpm 模块初始化
        stDpmfnInfo.schduleInstallCallback = adm_ipm_uimSchduleInstallCallback;
        stDpmfnInfo.schduleNotifyHmi = adm_vsm_dimSoaHostTspRRMessageNotify;
        iRet = adm_dpm_moduleInit(&stDpmfnInfo);
        if (ADM_IS_ERR(iRet)) 
        {
            log_e("adm_dpm_moduleInit failed, iRet:%d", iRet);
            //!< InitI_39
            adm_apc_eventAdd(ADM_APC_EV_STAGE_ING, 39, ADM_OCS_EVENT_LEVEL_ERROR, iRet, "adm_dpm_moduleInit failed");
            break;
        } 
        //!< InitI_040
        adm_apc_eventAdd(ADM_APC_EV_STAGE_ING, 40, ADM_OCS_EVENT_LEVEL_TRACE, 0, NULL);
        log_i("adm_dpm_moduleInit successed");

        //!< InitI_041
        adm_apc_eventAdd(ADM_APC_EV_STAGE_ING, 41, ADM_OCS_EVENT_LEVEL_TRACE, 0, NULL);

//!< tpm 依赖 ewebserver 的支持 
#ifdef CMAKE_CUSTOM_EWEBSERVER_SUPPORT
        //!< tpm 模块初始化
        iRet = adm_tpm_moduleInit();
        if (ADM_IS_ERR(iRet)) 
        {
            log_e("adm_tpm_moduleInit failed, iRet:%d", iRet);
            //!< InitI_42
            adm_apc_eventAdd(ADM_APC_EV_STAGE_ING, 42, ADM_OCS_EVENT_LEVEL_ERROR, iRet, "adm_tpm_moduleInit failed");
            break;
        } 
        //!< InitI_043
        adm_apc_eventAdd(ADM_APC_EV_STAGE_ING, 43, ADM_OCS_EVENT_LEVEL_TRACE, 0, NULL);
        log_i("adm_tpm_moduleInit successed");
#endif

        //!< InitI_044
        adm_apc_eventAdd(ADM_APC_EV_STAGE_ING, 44, ADM_OCS_EVENT_LEVEL_TRACE, 0, NULL);
        adm_ipm_init();
        //TODO: InitI_045
        //!< InitI_046
        adm_apc_eventAdd(ADM_APC_EV_STAGE_ING, 46, ADM_OCS_EVENT_LEVEL_TRACE, 0, NULL);

        //!< InitI_047
        adm_apc_eventAdd(ADM_APC_EV_STAGE_ING, 47, ADM_OCS_EVENT_LEVEL_TRACE, 0, NULL);
	    adm_vsm_init();
        
        stTspImplInfo.tspSchduleNotifyHmi = adm_vsm_dimSoaHostTspRRMessageNotify;
        stTspImplInfo.tspOneClickRescueCb = adm_apc_resetOTA;
        stTspImplInfo.schduleInstallCallback = adm_ipm_uimSchduleInstallCallback;
        adm_vsm_tspImplInit(&stTspImplInfo);
        //TODO: InitI_048
        //!< InitI_049
        adm_apc_eventAdd(ADM_APC_EV_STAGE_ING, 49, ADM_OCS_EVENT_LEVEL_TRACE, 0, NULL);

        stEmsInitInfo.emsResetOtaCB = adm_apc_resetOTA;
        stEmsInitInfo.emsOtaServerConnOkCB = adm_apc_detectConnect;
        stEmsInitInfo.emsGetOtaLogCB = adm_apc_logCollect;
        stEmsInitInfo.emsDelPackageCB = adm_apc_cleanDownladPackage;
        stEmsInitInfo.emsCtrlStopUcmCB = adm_apc_exitUCM;
        stEmsInitInfo.emsCtrlExt1CB = DNULL;
        stEmsInitInfo.emsCtrlExt2CB = DNULL;
        iRet = adm_ems_init(&stEmsInitInfo, DNULL);
        if (ADM_ECODE_SUCCESSED == iRet)
        {
            log_i("adm_ems_init successed");
        }
        else
        {
            log_e("adm_ems_init failed, iRet:%d", iRet);
        }

    } while(FALSE);
    ADM_APC_CORE_INIT_EXIT;

    if (ADM_ECODE_SUCCESSED != iRet) {
        //!< InitI_024
        adm_apc_eventAdd(ADM_APC_EV_STAGE_ING, 24, ADM_OCS_EVENT_LEVEL_ERROR, iRet, "adm_apc_managerInit failed");
    } else {
        //!< InitI_025
        adm_apc_eventAdd(ADM_APC_EV_STAGE_ING, 25, ADM_OCS_EVENT_LEVEL_TRACE, 0, NULL);
    }

    return iRet;
}

/**
 * @brief 各模块反初始化，包括：CVM、DPM、IPM等
 * 
 */
void adm_apc_managerDeinit(void)
{
    ADM_APC_CORE_INIT_ENTRY;
    
    adm_ems_deinit();

    // 各模块反初始化，包括：CVM、DPM、IPM等
//!< 断言 ewebserver 的支持 
#ifdef CMAKE_CUSTOM_EWEBSERVER_SUPPORT
    //!< InitE_1
    adm_apc_eventAdd(ADM_APC_EV_STAGE_END, 1, ADM_OCS_EVENT_LEVEL_TRACE, 0, NULL);
    (void)adm_tpm_stopEwebServer();
#endif

    //!< InitE_2
    adm_apc_eventAdd(ADM_APC_EV_STAGE_END, 2, ADM_OCS_EVENT_LEVEL_TRACE, 0, NULL);
    adm_vsm_tspImplDeinit();
    adm_vsm_deinit();

    //!< InitE_3
    adm_apc_eventAdd(ADM_APC_EV_STAGE_END, 3, ADM_OCS_EVENT_LEVEL_TRACE, 0, NULL);
    adm_ipm_deInit();

    //!< InitE_4
    adm_apc_eventAdd(ADM_APC_EV_STAGE_END, 4, ADM_OCS_EVENT_LEVEL_TRACE, 0, NULL);
    adm_ecm_moduleDeInit();
    adm_icp_moduleDeInit();
    //!< InitE_5
    adm_apc_eventAdd(ADM_APC_EV_STAGE_END, 5, ADM_OCS_EVENT_LEVEL_TRACE, 0, NULL);
    (void)adm_dpm_moduleDeInit();

    //!< tpm 依赖 ewebserver 的支持 
#ifdef CMAKE_CUSTOM_EWEBSERVER_SUPPORT
    //!< InitE_6
    adm_apc_eventAdd(ADM_APC_EV_STAGE_END, 6, ADM_OCS_EVENT_LEVEL_TRACE, 0, NULL);
    (void)adm_tpm_moduleDeInit();
#endif

    //!< InitE_7
    adm_apc_eventAdd(ADM_APC_EV_STAGE_END, 7, ADM_OCS_EVENT_LEVEL_TRACE, 0, NULL);
    //!< InitE_8
    adm_apc_eventAdd(ADM_APC_EV_STAGE_END, 8, ADM_OCS_EVENT_LEVEL_TRACE, 0, NULL);

    //PUB 模块反初始化
    adm_pub_moduleDeInit();
    
    adm_pis_packageDeInit();
    //!< InitE_9
    adm_apc_eventAdd(ADM_APC_EV_STAGE_END, 9, ADM_OCS_EVENT_LEVEL_TRACE, 0, NULL);
    //!< 零件缓存反初始化
    adm_eim_getEcuInfoManagerDeInit();
    adm_sdk_selfUpgradeDeinit();
    ADM_APC_CORE_INIT_EXIT;
}

/**
 * @brief 判断当前状态是否为安装中
 * 
 * @return DBOOL    TRUE:在安装中，FALSE:非安装中
 */
DBOOL adm_apc_isInstallingState(void)
{
    ADM_APC_CORE_INIT_ENTRY;
    // 判断是否为安装中状态
#if 1
    DBOOL isInstallIng = FALSE;
        if (adm_vsm_stateGet() == ADM_VSM_STATE_UPGRADE_READY ||
            adm_vsm_stateGet() == ADM_VSM_STATE_UPGRADE_DOING) {
            isInstallIng = TRUE;
        }
#else
    DCHAR* state = NULL;
    DBOOL isInstallIng = FALSE;

    do {
        state = adm_vsm_getState(ADM_VSM_STATE_TYPE_TASK);
        if (NULL == state) {
            log_e("adm_vsm_getState failed");
            break;
        } else {
            log_i("Current state is %s", state);
        }

        if (0 == adm_strcmp(state, ADM_VSM_TASK_STATE_INSTALL_ING)) {
            isInstallIng = TRUE;
        }

    } while(0);

    if (NULL != state) {
        adm_free(state);
    }
#endif

    ADM_APC_CORE_INIT_EXIT;
    //!< InitI_53
    adm_apc_eventAdd(ADM_APC_EV_STAGE_ING, 53, ADM_OCS_EVENT_LEVEL_TRACE, 0, (TRUE == isInstallIng) ? "Installing" : "Not installing");

    return isInstallIng;
}

/**
 * @brief 根据状态恢复任务
 * 
 * @return D32S     0:Successed, None-0:Failed
 */
D32S adm_apc_resumeTask(void)
{
    static DBOOL isResumeOk = FALSE;

    //!< InitI_54
    adm_apc_eventAdd(ADM_APC_EV_STAGE_ING, 54, ADM_OCS_EVENT_LEVEL_TRACE, 0, NULL);

    ADM_APC_CORE_INIT_ENTRY;
    // 恢复状态任务
    // TODO: 补充Trigger resume的接口
    if (FALSE == isResumeOk) {
        adm_vsm_resume();
        isResumeOk = TRUE;
    }
    ADM_APC_CORE_INIT_EXIT;

    return ADM_ECODE_SUCCESSED;
}

/**
 * @brief 任务等待状态，While(1)
 * 
 */
void adm_apc_waitTask(void)
{
    ADM_APC_CORE_INIT_ENTRY;
    // 进入等待任务的状态
    while (FALSE == g_bIsEndProcess)
    {
        (void)sleep(ADM_APC_CYCLE_OF_WAITTING_TASK);
        log_i("Waitting task...");
    }
    ADM_APC_CORE_INIT_EXIT;

    return;
}

static DBOOL adm_apc_isEngineerMode(void)
{
    // TODO: 判断工程模式
    DBOOL isEngineerMode = FALSE;

    return isEngineerMode;
}

static DBOOL adm_apc_setEngineerMode(const D8U cModeType)
{
    // TODO: 设置工程模式
    ADM_APC_CORE_INIT_ENTRY;
    DBOOL isSetOk = FALSE;
    isSetOk = TRUE;
    ADM_APC_CORE_INIT_EXIT;

    return isSetOk;
}

/**
 * @brief 为保证工程模式触发的实时性，增加每一秒判断一次工程模式，如果当前已经是工程模式则退出等待。
 * 
 * @param waitTime [in]     等待的时间
 * @return DBOOL    TRUE: 当前为工程模式，FALSE:当前为等待正常退出
 */
static DBOOL adm_apc_waitTimeAndEngineerMode(const D32U waitTime)
{
    D32S i = 0;
    DBOOL isBreak = FALSE;

    do {
        if (TRUE == adm_apc_isEngineerMode()) {
            isBreak = TRUE;
            break;
        }
        
        if (waitTime > (D32U)0) {
            (void)sleep(1);
        }
        i++;
    } while((D32U)i < waitTime);

    return isBreak;
}

/**
 * @brief 上电，等待网络恢复OK
 * 
 * @param waitTime [IN] const D32U  等待的时间
 */
void adm_apc_waitNetworkOk(const D32U waitTime)
{
    ADM_APC_CORE_INIT_ENTRY;
    // 等待waitTime s，用于主控ECU的网络和时间恢复
    (void)adm_apc_waitTimeAndEngineerMode(waitTime);
    ADM_APC_CORE_INIT_EXIT;
    //!< InitI_55
    adm_apc_eventAdd(ADM_APC_EV_STAGE_ING, 55, ADM_OCS_EVENT_LEVEL_TRACE, 0, "Wait network ok");
}

#ifdef SUPPORT_LION_PKI
#define ADM_VRM_VIN_LENGTH  (17)

static D32S adm_apc_getVin(DCHAR** vin)
{
    D32S iRet = ADM_ECODE_FAILED;
    D32S errCode = ADM_ECODE_FAILED;
    DBOOL isRealVehVer = FALSE;
    // *vin = strdup(VEHICLE_VIN);

    do {
        isRealVehVer = adm_cfg_iniParserGetBoolean(ADM_CFG_VUS_REAL_VEHICLE_VER, (DBOOL)ADM_CFG_VUS_REAL_VEHICLE_VER_DEFAULT_VAL);
        if (TRUE == isRealVehVer) 
        {
            log_i("VUS:is_real_vehicle_ver is TRUE, get vin from dim");
            iRet = adm_dim_getVin(adm_soa_getHostInstanceId(), 3, vin, &errCode);
            if (ADM_ECODE_SUCCESSED != iRet) 
            {
                log_e("adm_dim_soa_getVin failed, iRet:%d", iRet);
                break;
            }

            if (ADM_ECODE_SUCCESSED != errCode) 
            {
                iRet = errCode;
                log_e("adm_dim_soa_getVin failed, errCode:%d", errCode);
                break;
            }

            if (NULL == *vin) 
            {
                iRet = ADM_ECODE_INVALID_POINTER;
                log_e("adm_dim_soa_getVin vin is nil, iRet:%d", iRet);
                break;
            }
        } 
        else 
        {
            log_i("VUS:is_real_vehicle_ver is FALSE, get vin from otaconfig.ini");
            *vin = adm_strdup(adm_cfg_iniParserGetString(ADM_CFG_VUS_DEF_VEHICLE_IDEN, ADM_CFG_VUS_DEF_VEHICLE_IDEN_DEFAULT_VAL));
            iRet = ADM_ECODE_SUCCESSED;
        }
        
        if ((D32U)ADM_VRM_VIN_LENGTH != adm_strlen(*vin)) 
        {
            iRet = ADM_ECODE_INVALID_PARAM;
            log_e("adm_vrm_getVin length of vin not fit, iRet:%d", iRet);
            break;
        } 
    } while(FALSE);
    
    return iRet;
}

/**
 * @brief 获取PKI依赖项 vin、sn
 * @note PKI依赖于vin和sn 如果PKI初始化不能成功,将直接影响后面的注册等功能,必须无限循环直到成功
 * 
 * @return D32S     0:Successed, None-0:Failed
 */
static D32S adm_apc_getPkiInfo(void)
{
    ADM_APC_CORE_INIT_ENTRY;
    
    D64U i = 0;
    D32S iRet = ADM_ECODE_FAILED;
    D32S iErrcode = ADM_ECODE_FAILED;
    DCHAR* pSnInfo = NULL;
    DCHAR* pVin = NULL;
    D32U uiServerType = 1;

    (void)adm_ems_getServerType(&uiServerType);

    while (DTRUE)
    {
        /** 获取sn号 */
        i++;
        iRet = adm_dim_ucl_readSnInfo(adm_soa_getUclInstanceId(), ADM_APC_GETSN_TIME_OUT, &pSnInfo, &iErrcode);
        if (ADM_ECODE_SUCCESSED == iRet && ADM_ECODE_SUCCESSED == iErrcode && NULL != pSnInfo) 
        {
            log_i("Fine adm_dim_ucl_readSnInfo [%llu] time successed", i);
            break;
        }
        log_w("Keep waiting, get snInfo has not been successful adm_dim_ucl_readSnInfo retry [%llu] times failed, iRet:[%d], errcode[%d]", i, iRet, iErrcode);

        (void)sleep(ADM_APC_RETRY_GETSN_CYCLE);
        // pSnInfo = "V23testiccsn";
    }

    i = 0;
    while (DTRUE)
    {
        i++;
        /** 获取车辆VIN码 */
	    iRet = adm_apc_getVin(&pVin);
	    if (ADM_ECODE_SUCCESSED != iRet) 
        {
	        log_w("Keep waiting, get vin has not been successful adm_apc_getVin retry [%llu] times failed, iRet:[%d]", i, iRet);
	        (void)sleep(ADM_APC_RETRY_GETVIN_CYCLE);
	    } 
        else 
        {
	        log_i("Fine adm_apc_getVin [%llu] time successed", i);
			break;
	    }
    }

    i = 0;
    while (DTRUE)
    {
        i++;
        iRet = adm_oca_scInit((DCHAR*)pSnInfo, pVin, uiServerType);
        if (ADM_ECODE_SUCCESSED != iRet) 
        {
	        log_w("Keep waiting, oca sc init has not been successful adm_oca_scInit retry [%llu] times failed, iRet:[%d]", i, iRet);
	        (void)sleep(ADM_APC_RETRY_PKIINIT_CYCLE);
	    } 
        else 
        {
	        log_i("Fine adm_oca_scInit [%llu] time successed", i);
			break;
	    }
    }

    adm_free(pVin);
    // adm_free(pSnInfo);

    ADM_APC_CORE_INIT_EXIT;

    return iRet;
}
#endif

/**
 * @brief 执行注册的逻辑，包括：重试注册、周期注册等
 * 
 * @return D32S     0:Successed, None-0:Failed
 */
D32S adm_apc_register(void)
{
    ADM_APC_CORE_INIT_ENTRY;
    // 请求注册，包括：重试注册、周期注册；
    // 判断工程模式，如果为工程模式，默认返回注册成功；
    // 判断工程模式，如果非工程模式，像平台请求注册；
    D32S i = 0;
    D32S iRet = ADM_ECODE_FAILED;
    DBOOL isRegisterOk = FALSE;
    DCHAR eventContext[512] = {0};

    do {
        //!< InitI_59
        adm_apc_eventAdd(ADM_APC_EV_STAGE_ING, 59, ADM_OCS_EVENT_LEVEL_TRACE, 0, NULL);
        //!< ZT_001
        adm_vrm_eventAdd((D32S)ADM_VRM_EV_STAGE_TG, 1, ADM_OCS_EVENT_LEVEL_TRACE, 0, NULL);

        for (i = 0; i < ADM_APC_RETRY_REGISTER_NUMBERS; i++) {
            iRet = adm_apc_doRegisterFromServer();
            if (ADM_ECODE_SUCCESSED == iRet) {
                log_i("Fine adm_apc_doRegisterFromServer %d time successed", i);
                isRegisterOk = TRUE;
                break;
            }
            log_w("Keep waiting, register has not been successful adm_apc_doRegisterFromServer retry %d times failed, iRet:%d", i, iRet);
            //!< InitI_60
            adm_apc_eventAdd(ADM_APC_EV_STAGE_ING, 60, ADM_OCS_EVENT_LEVEL_WARN, iRet, "Keep waiting, register has not been successful adm_apc_doRegisterFromServer retry now");
            //!< ZT_003
            (void)adm_memset(eventContext, 0x00, sizeof(eventContext), sizeof(eventContext));
            (void)adm_snprintf(eventContext, sizeof(eventContext), "The total number of retries is %d time, and the retry interval is %ds.", ADM_APC_RETRY_REGISTER_NUMBERS, ADM_APC_RETRY_REGISTER_CYCLE);
            adm_vrm_eventAdd((D32S)ADM_VRM_EV_STAGE_TG, 3, ADM_OCS_EVENT_LEVEL_WARN, iRet, eventContext);
            if (TRUE == adm_apc_waitTimeAndEngineerMode(ADM_APC_RETRY_REGISTER_CYCLE)) {
                log_i("Currently in engineering mode, no need to retry register");
                iRet = ADM_ECODE_SUCCESSED;
                break;
            }
        }
        
        if (TRUE == isRegisterOk) {
            log_i("adm_apc_register ok!!!");
            break;
        }

        i = 0;
        while (TRUE)
        {
            i++;
            if (TRUE == adm_apc_waitTimeAndEngineerMode(ADM_ACP_PERIODIC_REGISTER_TIME)) {
                log_i("Currently in engineering mode, no need to periodic register");
                iRet = ADM_ECODE_SUCCESSED;
                break;
            }
            iRet = adm_apc_doRegisterFromServer();
            if (ADM_ECODE_SUCCESSED == iRet) {
                log_i("It’s too hard. I tried %d times and finally registered successfully.", i);
                break;
            }
            log_w("Keep Waitting, register has retry %d times, iRet:%d", i, iRet);
            //!< InitI_60
            adm_apc_eventAdd(ADM_APC_EV_STAGE_ING, 60, ADM_OCS_EVENT_LEVEL_WARN, iRet, "Keep waiting, register has not been successful adm_apc_doRegisterFromServer retry now");
            //!< ZT_002
            (void)adm_memset(eventContext, 0x00, sizeof(eventContext), sizeof(eventContext));
            (void)adm_snprintf(eventContext, sizeof(eventContext), "The total number of retries is %d time, and the periodic interval is %ds.", i, ADM_ACP_PERIODIC_REGISTER_TIME);
            adm_vrm_eventAdd((D32S)ADM_VRM_EV_STAGE_TG, 2, ADM_OCS_EVENT_LEVEL_WARN, iRet, eventContext);

        }
    } while(FALSE);

    ADM_APC_CORE_INIT_EXIT;
    //!< InitI_61
    adm_apc_eventAdd(ADM_APC_EV_STAGE_ING, 61, ADM_OCS_EVENT_LEVEL_TRACE, 0, NULL);

    return iRet;
}

static DBOOL adm_apc_isGetCfgOk(void)
{
    DBOOL isGetCfgOk = FALSE;
    DCHAR* cfgVersion = NULL;

    do {
        cfgVersion = adm_ecs_getRsConfigVersion();
        if (NULL == cfgVersion) {
            log_e("adm_ecs_getRsConfigVersion is failed");
            break;
        } else {
            log_i("adm_ecs_getRsConfigVersion successed, cfgVersion:%s", cfgVersion);
            isGetCfgOk = TRUE;
        }
    } while(FALSE);
    
    if (NULL != cfgVersion) {
        adm_free(cfgVersion);
    }

    return isGetCfgOk;
}

/**
 * @brief 从平台获取配置信息
 * 
 * @return D32S     0:Successed, None-0:Failed
 */
static D32S adm_apc_doGetCfgFromServer(void)
{
    D32S iRet = ADM_ECODE_FAILED;
    ADM_ECS_RSCONFIGINFO_T* pstRsCfgInfo = NULL;

    do {
        pstRsCfgInfo = adm_ecs_newRSConfigInfo();
        if (NULL == pstRsCfgInfo) {
            iRet = ADM_ECODE_NO_MEMORY;
            log_e("adm_ecs_newEcuConfigInfo failed, iRet:%d", iRet);
            break;
        } else {
            log_i("adm_ecs_newEcuConfigInfo successed");
        }

        iRet = adm_ecs_getConfigInfo(pstRsCfgInfo);
        if (ADM_ECODE_SUCCESSED != iRet) {
            log_e("adm_ecs_getConfigInfo failed, iRet:%d", iRet);
            break;
        } else {
            log_i("adm_ecs_getConfigInfo successed");
        }

    } while(FALSE);
    
    adm_ecs_delRSConfigInfo(&pstRsCfgInfo);

    return iRet;
}

/**
 * @brief 从USB中获取配置信息
 * 
 * @return D32S     0:Successed, None-0:Failed
 */
static D32S adm_apc_doGetCfgFromUsb(void)
{
    // TODO: 添加功能从USB中获取配置

    return ADM_ECODE_SUCCESSED;
}

/**
 * @brief 执行获取配置的逻辑，包括：重试获取、周期获取等
 * 
 * @return D32S     0:Successed, None-0:Failed
 */
D32S adm_apc_getConfig(void)
{
    ADM_APC_CORE_INIT_ENTRY;
    // 请求获取配置，包括：重试获取、周期获取；
    // 判断工程模式，当前为工程模式
        // 1. 如果为工程模式，且本地已经有配置文件，则不需要重新获取配置文件；
        // 2. 如果为工程模式，且本地没有配置文件，则需要从USB中获取配置文件；
    // 判断工程模式，当前非工程模式
        // 1. 本地存在配置文件，则不需要重新获取配置文件；
        // 2. 本地不存在配置文件，向平台请求获取配置文件
    // 从平台或USB 获取配置成功后，需要请求更新本地零件信息；
    // 工程模式下，从USB中获取配置失败后，需要退出工程模式，且回到注册阶段；
    D32S i = 0;
    D32S iRet = ADM_ECODE_FAILED;
    DBOOL isGetCfgOk = FALSE;

    do {
        //!< InitI_62
        adm_apc_eventAdd(ADM_APC_EV_STAGE_ING, 62, ADM_OCS_EVENT_LEVEL_TRACE, 0, NULL);
        if (TRUE == adm_apc_isGetCfgOk()) {
            iRet = ADM_ECODE_SUCCESSED;
            log_i("adm_apc_isGetCfgOk is TRUE");
            break;
        }

        if (TRUE == adm_apc_waitTimeAndEngineerMode(0)) {
            for (i = 0; i < ADM_APC_RETRY_GET_CONFIG_FROM_USB_NUM; i++) {
                iRet = adm_apc_doGetCfgFromUsb();
                if (ADM_ECODE_SUCCESSED == iRet) {
                    log_i("Fine adm_apc_doGetCfgFromUsb %d time successed", i);
                    isGetCfgOk = TRUE;
                    break;
                }
            }

            // 执行退出工程模式
            // 从注册开始继续执行
            if (TRUE == adm_apc_setEngineerMode(0)) {
                iRet = adm_apc_otaFirstBlood();
                if (ADM_ECODE_SUCCESSED == iRet) {
                    log_i("adm_apc_otaFirstBlood successed");
                } else {
                    log_e("adm_apc_otaFirstBlood failed, iRet:%d", iRet);
                }
            }
            
        } else {
            for (i = 0; i < ADM_APC_RETRY_GET_CONFIG_FROM_SRV_NUM; i++) {
                iRet = adm_apc_doGetCfgFromServer();
                if (ADM_ECODE_SUCCESSED == iRet) {
                    log_i("Fine adm_apc_doGetCfgFromServer %d time successed", i);
                    isGetCfgOk = TRUE;
                    break;
                }
                //!< InitI_63
                adm_apc_eventAdd(ADM_APC_EV_STAGE_ING, 63, ADM_OCS_EVENT_LEVEL_WARN, iRet, "Keeep waitting, get config has retry now");
                (void)sleep(ADM_APC_RETRY_GET_CONFIG_CYCLE);
            }
        }

        if (TRUE == isGetCfgOk) {
            log_i("adm_apc_getConfig ok");
            break;
        }

        i = 0;
        while (TRUE)
        {
            i++;
            (void)sleep(ADM_APC_PERIODIC_GET_CFG_TIME);
            iRet = adm_apc_doGetCfgFromServer();
            if (ADM_ECODE_SUCCESSED == iRet) {
                log_i("It's to hard, I tried %d times and finally get config from server successed");
                break;
            }

            log_w("Keeep waitting, get config has retry %d times, iRet:%d", i, iRet);
            //!< InitI_63
            adm_apc_eventAdd(ADM_APC_EV_STAGE_ING, 63, ADM_OCS_EVENT_LEVEL_WARN, iRet, "Keeep waitting, get config has retry now");
        }
    } while(FALSE);

    ADM_APC_CORE_INIT_EXIT;
    //!< InitI_64
    adm_apc_eventAdd(ADM_APC_EV_STAGE_ING, 64, ADM_OCS_EVENT_LEVEL_TRACE, 0, NULL);

    return iRet;
}

static D32S adm_apc_getEcuInfoFromVehicle(void)
{
    //!< InitI_68
    adm_apc_eventAdd(ADM_APC_EV_STAGE_ING, 68, ADM_OCS_EVENT_LEVEL_TRACE, 0, NULL);
    // TODO: 零件获取到零件配置信息，从整车上获取零件信息
    // TODO: 异步获取零件信息
    adm_eim_getEcuInfoManagerCycleCollection();
    //TODO: InitI_69
    //!< InitI_70
    adm_apc_eventAdd(ADM_APC_EV_STAGE_ING, 70, ADM_OCS_EVENT_LEVEL_TRACE, 0, NULL);
    return ADM_ECODE_SUCCESSED;
}

/**
 * @brief 为OTA注入第一滴血，包括：注册和获取配置信息
 * 
 * @return D32S     0:Successed, None-0:Failed
 */
D32S adm_apc_otaFirstBlood(void)
{
    ADM_APC_CORE_INIT_ENTRY;
    // adm_apc_register
    // adm_apc_getConfig  
    D32S iRet = ADM_ECODE_FAILED;

    do {
        //!< InitI_56
        adm_apc_eventAdd(ADM_APC_EV_STAGE_ING, 56, ADM_OCS_EVENT_LEVEL_TRACE, 0, NULL);

        /** 获取ICC-sn号 */
#ifdef SUPPORT_LION_PKI
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

        /** 11. 请求注册，包括：重试注册、周期注册等； */
        iRet = adm_apc_register();
        if (ADM_ECODE_SUCCESSED != iRet) {
            log_e("adm_apc_register failed, iRet:%d", iRet);
            break;
        } else {
            log_i("adm_apc_register successed");
        }
        //请求获取icp信息
        iRet=adm_icp_moduleInit();
         if (ADM_IS_ERR(iRet)) 
        {
            log_e("adm_icp_moduleInit failed, iRet:%d", iRet);
        } else{
            log_i("adm_icp_moduleInit successed");
        }
        /** 12. 请求获取配置，包括：重试获取配置，周期获取配置等； */
        iRet = adm_apc_getConfig();
        if (ADM_ECODE_SUCCESSED != iRet) {
            log_e("adm_apc_getConfig failed, iRet:%d", iRet);
            break;
        } else {
            log_i("adm_apc_getConfig successed");
            //!< 获取配置成功后，需要设置需要过滤的日志级别
            (void)adm_apc_logInit();
        }

        //!< InitI_065
        adm_apc_eventAdd(ADM_APC_EV_STAGE_ING, 65, ADM_OCS_EVENT_LEVEL_TRACE, 0, NULL);
        //TODO: InitI_66
        //!< InitI_067
        adm_apc_eventAdd(ADM_APC_EV_STAGE_ING, 67, ADM_OCS_EVENT_LEVEL_TRACE, 0, NULL);

        /** 非安装中，或者更新全局配置未完成，则更新全局配置（高低配适配） */
        if (FALSE == adm_apc_isInstallingState() ||
            FALSE == adm_ecs_isUpdateGlobalConfigFinish())
        {
            adm_apc_vehicleConfigReset(FALSE);
        }

        // 创建线程，异步执行零件信息获取
        (void)adm_apc_getEcuInfoFromVehicle();
        // 创建线程，异步执行同步车辆信息
        (void)adm_cvm_asyncVersionNumber(ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_IGN, NULL);
        // 监控车辆上电事件
        // adm_dmi_vdiSpyAddVehiclePowerOn();
    } while(FALSE);

    ADM_APC_CORE_INIT_EXIT;
    if (ADM_ECODE_SUCCESSED != iRet) {
        //!< InitI_57
        adm_apc_eventAdd(ADM_APC_EV_STAGE_ING, 57, ADM_OCS_EVENT_LEVEL_ERROR, iRet, "adm_apc_otaFirstBlood failed");
    } else {
        //!< InitI_58
        adm_apc_eventAdd(ADM_APC_EV_STAGE_ING, 58, ADM_OCS_EVENT_LEVEL_TRACE, 0, NULL);
    }
    return iRet;
}

/*!
 *******************************************************************************
 * apc 模拟触发下载动作
 *
 * \note
 * 
 * \retval    D32S
 *******************************************************************************
 */
D32S adm_apc_triggerDownload(void)
{
    D32S iRet = ADM_ECODE_FAILED;

    ADM_APC_CORE_INIT_ENTRY;
    iRet = adm_dpm_triggerDownload(ADM_APPLICATION_COMMON_CMDID_TYPE_INVALID);
    if (ADM_IS_ERR(iRet)) 
    {
        log_e("adm_dpm_triggerDownload failed, [err:%d]", iRet);
    }
    ADM_APC_CORE_INIT_EXIT;

    return iRet;
}

/**
 * @brief 初始化日志上报的模块
 * 
 */
static void __adm_apc_irmLogFileReportInit(void)
{
    ADM_IRM_LOG_FILE_REPORT_INIT_T stLogFileReportInfo = {0};
    const DCHAR* dirStr = NULL;

    (void)adm_memset(&stLogFileReportInfo, 0x00, sizeof(ADM_IRM_LOG_FILE_REPORT_INIT_T), sizeof(ADM_IRM_LOG_FILE_REPORT_INIT_T));
    stLogFileReportInfo.logInitCb = adm_apc_logInit;
    stLogFileReportInfo.logDeinitCb = adm_apc_logDeinit;
    stLogFileReportInfo.getSessionIdCb = adm_cvs_getSessionId;
    stLogFileReportInfo.getTaskIdCb = adm_cvs_getTaskId;
    stLogFileReportInfo.getStrategyIdCb = adm_cvs_getStrategyId;
    stLogFileReportInfo.getSdkVerCb = adm_sdk_getSDKVersion;
    stLogFileReportInfo.getLogReportCycle = adm_ecs_getLogReportCycle;

    //!< 从车云协议获取上报日志文件是否需要加密操作成功    
    if(ADM_ECODE_SUCCESSED == adm_ecs_getIsLogFileEncrypted(&stLogFileReportInfo.isEncrypt))
    {
        //!< 从车云协议获取上报日志文件是否加密不在取值范围中，则填充默认值
        if((ADM_IRM_LOG_FILE_ENCRYPT_REPORT != stLogFileReportInfo.isEncrypt) &&
           (ADM_IRM_LOG_FILE_NOTENCRYPT_REPORT != stLogFileReportInfo.isEncrypt))
        {
            log_e("Error value of isEncrypt, value = [%d], Fill defaults.", stLogFileReportInfo.isEncrypt);
            stLogFileReportInfo.isEncrypt = ADM_IRM_LOG_FILE_ENCRYPT_REPORT;
        }
    }
    //!< 从车云协议获取上报日志文件是否需要加密操作失败，填充默认值
    else
    {
        log_e("Failed to get config of isEncrypt and Fill defaults.");
        stLogFileReportInfo.isEncrypt = ADM_IRM_LOG_FILE_ENCRYPT_REPORT;
    }
    
    //!> 加密相关变量参数初始化
    stLogFileReportInfo.secret = NULL;
    stLogFileReportInfo.secretLen = 0;
    stLogFileReportInfo.segmentEncLen = 0;
    stLogFileReportInfo.fileEncCb = NULL;
    
    //!< 初始化日志存储目录
    dirStr = NULL;
    dirStr = adm_cfg_iniParserGetString(ADM_CFG_VUS_LOG_STORAGE_DIR, ADM_CFG_VUS_LOG_STORAGE_DIR_DEFAULT_VAL);
    if (NULL != dirStr) {
        stLogFileReportInfo.logStorageDir = adm_strdup(dirStr);
    }

    //!< 初始化日志收集存储目录
    dirStr = NULL;
    dirStr = adm_cfg_iniParserGetString(ADM_CFG_VUS_LOG_COLLECT_DIR, ADM_CFG_VUS_LOG_COLLECT_DIR_DEFAULT_VAL);
    if (NULL != dirStr) {
        stLogFileReportInfo.logCollectDir = adm_strdup(dirStr);
    }

    //!< 初始化日志打包的存储目录
    dirStr = NULL;
    dirStr = adm_cfg_iniParserGetString(ADM_CFG_VUS_LOG_PACK_DIR, ADM_CFG_VUS_LOG_PACK_DIR_DEFAULT_VAL);
    if (NULL != dirStr) {
        stLogFileReportInfo.logPackDir = adm_strdup(dirStr);
    }

    //!< 初始化日志上报模块
    adm_irm_logFileReportInit(&stLogFileReportInfo);

    //!< 日志上报模块初始化成功之后，填充日志初始化的时间
    if (FALSE == adm_irm_logFileStartTimeStore(g_llogStartTime)) {
        log_e("adm_irm_logFileStartTimeStore failed, g_llogStartTime:%lld", g_llogStartTime);
    }
    if (NULL != stLogFileReportInfo.secret) {
        adm_free(stLogFileReportInfo.secret);
    }
    if (NULL != stLogFileReportInfo.logStorageDir) {
        adm_free(stLogFileReportInfo.logStorageDir);
    }
    if (NULL != stLogFileReportInfo.logCollectDir) {
        adm_free(stLogFileReportInfo.logCollectDir);
    }
    if (NULL != stLogFileReportInfo.logPackDir) {
        adm_free(stLogFileReportInfo.logPackDir);
    }
}

static void __adm_apc_irmLogFileReportDeinit(void)
{
    adm_irm_logFileReportDeinit();
}

static void __adm_apc_testReportLogFile(void)
{
    ADM_IRM_LOG_FILE_TG_REPORT_INFO_T tgReportInfo = {0};

    tgReportInfo.cmdId = ADM_OCS_CMDID_CONFIG;
    tgReportInfo.pushId = NULL;
    tgReportInfo.extsReqJsonStr = NULL;

    log_i("Trigger report log file test sample");
    adm_irm_triggerLogFileReport(&tgReportInfo);
    log_i("Trigger report log file test sample");
}

D32S adm_apc_irmInit(void)
{
    D32S iRet = ADM_ECODE_FAILED;

    //!< InitI_035
    adm_apc_eventAdd(ADM_APC_EV_STAGE_ING, 35, ADM_OCS_EVENT_LEVEL_TRACE, 0, NULL);
    
    ADM_APC_CORE_INIT_ENTRY;
    iRet = adm_irm_init(adm_ocm_businessCommInterface, adm_ocm_fileUploadInterface, NULL);
    if (ADM_ECODE_SUCCESSED != iRet) {
        log_e("adm_irm_init failed, iRet:%d", iRet);
        //!< InitI_36
        adm_apc_eventAdd(ADM_APC_EV_STAGE_ING, 36, ADM_OCS_EVENT_LEVEL_ERROR, iRet, "adm_irm_init failed");
    } else {
        //!< InitI_037
        adm_apc_eventAdd(ADM_APC_EV_STAGE_ING, 37, ADM_OCS_EVENT_LEVEL_TRACE, 0, NULL);
    }
    
    //!< 日志上报模块初始化
    __adm_apc_irmLogFileReportInit();

#if 0 
    //!< 测试日志上报流程
    __adm_apc_testReportLogFile();
#endif
    ADM_APC_CORE_INIT_EXIT;
    return iRet;
}

void adm_apc_irmDeinit(void)
{
    ADM_APC_CORE_INIT_ENTRY;
    //!< 日志上报模块反初始化
    __adm_apc_irmLogFileReportDeinit();
    adm_irm_deinit();
    ADM_APC_CORE_INIT_EXIT;
}

/*!
*******************************************************************************
* 关闭文件锁
* 
* \note   
*
* \retval      void
*******************************************************************************
*/
void adm_apc_closelockfileHandle(void)
{
    close(g_ilock_file);
    return;
}

/*!
*******************************************************************************
* 创建并打开文件锁
* 
* \note   
*
* \retval      D32S 0成功,非0失败
*******************************************************************************
*/
D32S adm_apc_singleProgram(void)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    char tmpDir[ADM_COM_FILEPATH_LEN] = {0};
    char singleLockPath[ADM_COM_FILEPATH_LEN] = {0};
    const DCHAR* dirStr = NULL;
    D32S irc = 0;

    /** 初始化运行时主目录 */
    dirStr = adm_cfg_iniParserGetString(ADM_CFG_VUS_RUNTIME_ROOT_DIR, ADM_CFG_VUS_RUNTIME_ROOT_DIR_DEFAULT_VAL);
    (void)adm_snprintf(tmpDir, sizeof(tmpDir), "%s%s", dirStr, ADM_APC_SINGLE_LOCK_DIR);

    iRet = adm_com_createDir(tmpDir);
    if (ADM_ECODE_SUCCESSED != iRet) 
    {
        (void)printf("adm_com_createDir %s failed, iRet:%d\n", tmpDir, iRet);
        return iRet;
    }
    (void)adm_snprintf(singleLockPath, sizeof(singleLockPath), "%s%s", tmpDir, ADM_APC_SINGLE_LOCK);
    g_ilock_file = open(singleLockPath, O_CREAT|O_RDWR, 0666);
    irc = flock(g_ilock_file, LOCK_EX|LOCK_NB);
    if (irc)
    {
        if (EWOULDBLOCK == errno)
        {
            iRet = ADM_ECODE_FAILED;
            (void)printf("The process is running\n");
            return iRet;
        }
    }
    return iRet;
}


/*!
*******************************************************************************
* 生成进程信息列表信息
* 
* \note   
*
* \param[in]   num  列表个数
* \retval      ADM_APC_RECORD_INFO_T*
*******************************************************************************
*/
ADM_APC_RECORD_INFO_T *apcNewRecordInfoList(D32S num)
{
    ADM_APC_RECORD_INFO_T *pstApcRecordInfo = NULL;

    if (0 >= num)
    {
        log_e("Invalid input param");
        return NULL;
    }

    pstApcRecordInfo = (ADM_APC_RECORD_INFO_T *)adm_malloc((D32U)num * sizeof(ADM_APC_RECORD_INFO_T));
    if (NULL != pstApcRecordInfo)
    {
        (void)adm_memzero(pstApcRecordInfo, (D32U)num * sizeof(ADM_APC_RECORD_INFO_T));
    }

    return pstApcRecordInfo;
}

/*!
 *******************************************************************************
 * 反序列化json字符串到进程信息.
 *
 * \note
 * 
 * \param[in]   pcProcessInfo 进程信息的json字符串
 * \param[out]  pstProRecord  进程信息
 * \retval      D32S
 *******************************************************************************
 */
static D32S apcDealJsonChar2ProRecord(DCHAR *pcProcessInfo,ADM_APC_PRO_RECORD_T *pstProRecord)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    json_object *jsonRoot = NULL;
    D32S i = 0;
    json_type jsonType = json_type_null;
    json_object *jsonPocessInfo    = NULL;
    json_object *jsonArrayIndexObj = NULL;
    ADM_APC_RECORD_INFO_T *pstApcRecordInfoList = NULL;

    if (ADM_IS_NULL(pcProcessInfo) || ADM_IS_NULL(pstProRecord))
    {
        log_e("Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }
    
    do
    {
        jsonRoot = json_tokener_parse(pcProcessInfo);
        if (ADM_IS_NULL(jsonRoot)) 
        {
            iRet = ADM_ECODE_STRING_TO_JSON_FAILED;
            log_e("Failed string to json, string[%s]", pcProcessInfo);
            break;
        }

        if (!json_object_object_get_ex(jsonRoot, ADM_APC_PROCESS_INFOS, &jsonPocessInfo))
        {
            iRet = ADM_ECODE_JSON_GET_KEY_FAILED;
            log_e( "Failed to get json value, key[%s]", ADM_APC_PROCESS_INFOS);
            break;
        }

        jsonType = json_object_get_type(jsonPocessInfo);
        if (jsonType != json_type_array)
        {
            iRet = ADM_ECODE_JSON_KEY_TYPE_ERROR;
            log_e( "Json key[%s] type error", ADM_APC_PROCESS_INFOS);
            break;
        }

        pstProRecord->iRecordNum = (D32S)json_object_array_length(jsonPocessInfo);
        if (0 == pstProRecord->iRecordNum)
        {
            iRet = ADM_ECODE_SUCCESSED;
            log_i( "%s array list is zero", ADM_APC_PROCESS_INFOS);
            break;
        }

        pstApcRecordInfoList = apcNewRecordInfoList(pstProRecord->iRecordNum);
        if (ADM_IS_NULL(pstApcRecordInfoList))
        {
            iRet = ADM_ECODE_NO_MEMORY;
            log_e( "Failed to new record info list");
            break;
        }

        for (i = 0; i < pstProRecord->iRecordNum; i++) 
        {
            jsonArrayIndexObj = json_object_array_get_idx(jsonPocessInfo, (D32U)i);
            if (ADM_IS_NULL(jsonArrayIndexObj)) 
            {
                iRet = ADM_ECODE_JSON_GET_ARRAYOBJECT_FAILED;
                log_e( "Failed to backtracking record info Json array, arrayIndex[%d]", i);
                break;
            }

            iRet = adm_com_getJsonIntVal(jsonArrayIndexObj, ADM_APC_PID, &pstApcRecordInfoList[i].iPid);
            if (ADM_IS_ERR(iRet))
            {
                log_e("Failed to get json value, key[%s], err[%d]", ADM_APC_PID, iRet);
                break;
            }

            iRet = adm_com_getJsonInt64Val(jsonArrayIndexObj, ADM_APC_RECORD_TIME, &pstApcRecordInfoList[i].iRecordTime);
            if (ADM_IS_ERR(iRet))
            {
                log_e("Failed to get json value, key[%s], err[%d]", ADM_APC_RECORD_TIME, iRet);
                break;
            }
        }

        pstProRecord->pstApcRecordInfoList = pstApcRecordInfoList;

        if (ADM_IS_ERR(iRet))
        {
            pstProRecord->pstApcRecordInfoList = NULL;
            pstProRecord->iRecordNum = 0;
        }
        
    } while (FALSE);

    if (NULL != jsonRoot)
    {
        (void)json_object_put(jsonRoot);
    }

    return iRet;
}


/*!
 *******************************************************************************
 * 从KV中获取进程信息.
 *
 * \note
 *
 * \param[out]    pstProRecord    进程信息
 * \retval        D32S
 *******************************************************************************
 */
static D32S apcGetProcessInfoFromKV(ADM_APC_PRO_RECORD_T *pstProRecord)
{
    D32S iRet = ADM_ECODE_FAILED;
    DBOOL bIsOk = FALSE;
    size_t valLength = 0;
    DCHAR *pcProcessInfo = NULL;
	ADM_KV_STORAGE_VALUE_TYPE_E eValType;

    if (ADM_IS_NULL(pstProRecord))
    {
        log_e( "Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }

    //!<  processinfo
    bIsOk = adm_com_kvstorage_get_string(adm_core_service_getBusinessDatabaseDb(), ADM_APC_PROCESS_INFO_KV_KEY, 
                            &pcProcessInfo, &valLength, &eValType);
    if (FALSE == bIsOk)
    {
        iRet = ADM_ECODE_KV_GET_STRING_FAILED;
        log_e("Failed to get adm_com_kvstorage_get_string, key[%s]", ADM_APC_PROCESS_INFO_KV_KEY);
        return iRet;
    }
    log_i("===>Get pcProcessInfo[%s]", pcProcessInfo);

    iRet = apcDealJsonChar2ProRecord(pcProcessInfo, pstProRecord);
    if (ADM_IS_ERR(iRet))
    {
        adm_free(pcProcessInfo);
        log_e("Failed to deal process info from kv, err[%d]", iRet);
        return iRet;
    }
    adm_free(pcProcessInfo);
    
    return ADM_ECODE_SUCCESSED;
}

/*!
 *******************************************************************************
 * 创建进程信息json.
 *
 * \note
 *
 * \param[in]     pstRecordInfo      进程信息
 * \retval        json_object
 *******************************************************************************
 */
static struct json_object* apcRecordInfoJsonCreate(const ADM_APC_RECORD_INFO_T *pstRecordInfo)
{
    struct json_object* jsonroot = NULL;
    struct json_object *pPidVal = NULL;
    struct json_object *pRecordtimeVal = NULL;

    if(NULL == pstRecordInfo)
    {
        log_e("Invalid input param");
        return NULL;
    }

    jsonroot = json_object_new_object();
    if (ADM_IS_NULL(jsonroot))
    {
        log_e("json_object_new_object failed");
        return NULL;
    }

    pPidVal = json_object_new_int(pstRecordInfo->iPid);
    if(pPidVal != NULL)
    {
        (void)json_object_object_add(jsonroot, ADM_APC_PID, pPidVal);
    }

    pRecordtimeVal = json_object_new_int64(pstRecordInfo->iRecordTime);
    if(pRecordtimeVal != NULL)
    {
        (void)json_object_object_add(jsonroot, ADM_APC_RECORD_TIME, pRecordtimeVal);
    }

    return jsonroot;
}


/*!
 *******************************************************************************
 * 设置进程信息到KV中.
 *
 * \note
 *
 * \param[in]     pstProRecord       进程信息
 * \param[out]    pcProcessInfos     进程信息
 * \retval        D32S
 *******************************************************************************
 */
static D32S apcEnProcessInfos2JsonChar(ADM_APC_PRO_RECORD_T *pstProRecord, DCHAR **pcProcessInfos)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    D32S i = 0;
    json_object *jsonProInfos  = NULL;
    json_object *jsonProInfosArray = NULL;
    json_object *indexObjJson   = NULL;

    if (ADM_IS_NULL(pstProRecord) || ADM_IS_NULL(pcProcessInfos))
    {
        log_e("Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }

    do
    {
        jsonProInfosArray = json_object_new_array();
        if (ADM_IS_NULL(jsonProInfosArray)) 
        {
            iRet = ADM_ECODE_NEW_JSON_OBJECT_FAILED;
            log_e("Failed to new json array object, err[%d]", iRet);
            break;
        }

        for (i = 0; i < pstProRecord->iRecordNum; i++) 
        {
            //!< create json
            indexObjJson = apcRecordInfoJsonCreate(&pstProRecord->pstApcRecordInfoList[i]);
            if (ADM_IS_NULL(indexObjJson))
            {
                iRet = ADM_ECODE_NEW_JSON_OBJECT_FAILED;
                log_e("Failed to create conditions json object, err[%s]", iRet);
                break;
            }
            
            //!< add to array
            iRet = json_object_array_add(jsonProInfosArray, indexObjJson);
            if (ADM_IS_ERR(iRet))
            {
                iRet = ADM_ECODE_JSON_ADD_ARRAY_FAILED;
                log_e("Failed to add object to array, err[%d]", iRet);
                break;
            }
        }

        if (ADM_IS_ERR(iRet))
        {        
            (void)json_object_put(jsonProInfosArray);
        }

        // new object
        jsonProInfos = json_object_new_object();
        if (ADM_IS_NULL(jsonProInfos))
        {
            (void)json_object_put(jsonProInfosArray);
            iRet = ADM_ECODE_NEW_JSON_OBJECT_FAILED;
            log_e("Faile to new jsonProInfos json, err[%d]", iRet);
            break;
        }

        //!< array add object
        iRet = json_object_object_add(jsonProInfos, ADM_APC_PROCESS_INFOS, jsonProInfosArray);
        if (ADM_IS_ERR(iRet))
        {
            (void)json_object_put(jsonProInfosArray);
            iRet = ADM_ECODE_JSON_ADD_STRING_FAILED;
            log_e("Failed to add downConditions json object, err[%d]", iRet);
            break;
        }

        //!< json to string
        *pcProcessInfos = adm_strdup(json_object_to_json_string(jsonProInfos));
        if (ADM_IS_NULL(*pcProcessInfos))
        {
            log_e("Failed to json to string");
            iRet = ADM_ECODE_JSON_TO_STRING_FAILED;
            break;
        }

        log_d("=====> pcProcessInfos [%s]", *pcProcessInfos);
    } while(FALSE);

    //!< 只需要判断释放顶层的节点，中途的节点有错误上面会释放
    if (NULL != jsonProInfos)
    {
        (void)json_object_put(jsonProInfos);
    }

    return iRet;
}


/*!
 *******************************************************************************
 * 设置进程信息到KV中.
 *
 * \note
 *
 * \param[in]     pstProRecord         总进程记录
 * \param[in]     pstRecordInfo        需要记录的进程信息
 * \retval        D32S
 *******************************************************************************
 */
static D32S apcSetProcessInfo2KV(ADM_APC_PRO_RECORD_T *pstProRecord, ADM_APC_RECORD_INFO_T *pstRecordInfo)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    D32S iPid = 0;
    D32S i = 0;
    D32S iProResTimes = 0;
    D64S iTimestamp = 0;
    DBOOL bIsOk = FALSE;
    size_t valLength = 0;
    KvStorage_t *pstOtaKvHandle = NULL;
    ADM_KV_STORAGE_VALUE_TYPE_E eValType;
    DCHAR *pcProcessInfos = NULL;
    ADM_APC_RECORD_INFO_T stRecordInfo = {0};
    ADM_APC_RECORD_INFO_T *pstApcRecordInfoList = NULL;  //!< 进程记录信息
    DCHAR *piRecordTime = NULL;

    if (ADM_IS_NULL(pstProRecord))
    {
        log_e( "Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }

    pstOtaKvHandle = adm_core_service_getBusinessDatabaseDb();
    if (ADM_IS_NULL(pstOtaKvHandle))
    {
        log_e("Failed to get ota kv handle.");
        return ADM_ECODE_KV_OPEN_FAILED;
    }

    do
    {
        //获取配置文件中重启次数，当规定时间内重启次数大于该值时，则异常，估以这个作为记录最大条数
        iProResTimes = adm_cfg_iniParserGetInt(ADM_CFG_VUS_PROCESS_RESTART_TIMES, ADM_CFG_VUS_PROCESS_RESTART_TIMES_VAL);

        if(pstProRecord->iRecordNum < iProResTimes)
        {
            //小于重启次数记录时
            log_i("set proncess info to iRecordNum[%d]!",pstProRecord->iRecordNum);
            pstApcRecordInfoList = apcNewRecordInfoList(pstProRecord->iRecordNum + 1);
            if(NULL == pstApcRecordInfoList)
            {
                log_e( "Failed to new record info list");
                break;
            }

            for(i=0; i<pstProRecord->iRecordNum; i++)
            {
                pstApcRecordInfoList[i].iPid = pstProRecord->pstApcRecordInfoList[i].iPid;
                pstApcRecordInfoList[i].iRecordTime = pstProRecord->pstApcRecordInfoList[i].iRecordTime;
            }
            pstApcRecordInfoList[i].iPid = pstRecordInfo->iPid;
            pstApcRecordInfoList[i].iRecordTime = pstRecordInfo->iRecordTime;
            pstProRecord->pstApcRecordInfoList = NULL;
            pstProRecord->pstApcRecordInfoList = pstApcRecordInfoList;
            pstProRecord->iRecordNum ++;
        }
        else
        {
            //当iRecordNum等于重启次数时，需要将第1条删除，第2条拷贝给第1条记录·····第10条拷贝给第9条
            for(i=0; i < (pstProRecord->iRecordNum-1); i++)
            {
                pstProRecord->pstApcRecordInfoList[i].iPid = pstProRecord->pstApcRecordInfoList[i+1].iPid;
                pstProRecord->pstApcRecordInfoList[i].iRecordTime = pstProRecord->pstApcRecordInfoList[i+1].iRecordTime;
            }

            //最新的记录在第10条的位置
            pstProRecord->pstApcRecordInfoList[i].iPid = pstRecordInfo->iPid;
            pstProRecord->pstApcRecordInfoList[i].iRecordTime = pstRecordInfo->iRecordTime;
        }

        //清空之前存储的数据
        (void)adm_com_kvstorage_remove_key(pstOtaKvHandle, ADM_APC_PROCESS_INFO_KV_KEY);

        iRet = apcEnProcessInfos2JsonChar(pstProRecord, &pcProcessInfos);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to en down task info to char, err[%d]", iRet);
            break;
        }

        bIsOk = adm_com_kvstorage_set_string(pstOtaKvHandle, ADM_APC_PROCESS_INFO_KV_KEY, pcProcessInfos, ADM_KV_STORAGE_VALUE_TYPE_STRING);
        if (FALSE == bIsOk)
        {
            adm_free(pcProcessInfos);
            log_e("Failed to set adm_com_kvstorage_set_string, key[%s]", ADM_APC_PROCESS_INFO_KV_KEY);
            break;
        }
        adm_free(pcProcessInfos);

        log_i("set process info to pid[%d] timestamp[%lld] success!",stRecordInfo.iPid,stRecordInfo.iRecordTime);

    }while (FALSE);

    return ADM_ECODE_SUCCESSED;
}

/*!
 *******************************************************************************
 * 获取设备启动时间
 *
 * \note
 * \param[out]    pStartInterval        设备启动时间间隔
 * \retval        D32S
 *******************************************************************************
 */
static D32S apcGetEquipmentstartInterval(D64S *pStartInterval)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    DCHAR *pdotaddr = NULL;
    FILE *fp  = NULL;
    D32S errNum = 0;
    struct stat fileStat = {0};
    DCHAR aTimestr[ADM_APC_UPTINEFILE_DATA_SIZE + 1] = {0};
    DCHAR aUptimestr[ADM_APC_UPTINEFILE_DATA_SIZE] = {0};

    if (ADM_IS_NULL(pStartInterval))
    {
        log_e( "Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }

    fp = fopen("/proc/uptime", "r");
    if (NULL == fp)
    {
        errNum = errno;
        log_d("open /proc/uptime failed, error %s", strerror(errNum));
        return ADM_ECODE_INVALID_PARAM;
    }

    if (0 == (fread(aTimestr, sizeof(DCHAR), ADM_APC_UPTINEFILE_DATA_SIZE, fp)))
    {
        (void)fclose(fp);
        errNum = errno;
        log_e( "read file /proc/uptime, error %s", strerror(errNum));
        return ADM_ECODE_READ_FILE_FAILED;
    }

    log_d("filedata:%s",aTimestr);

    pdotaddr = adm_strchr(aTimestr, '.');
    if((pdotaddr - aTimestr) < ADM_APC_UPTINEFILE_DATA_SIZE)
    {
        adm_memcpy(aUptimestr, aTimestr, pdotaddr - aTimestr, ADM_APC_UPTINEFILE_DATA_SIZE);
    }
    else
    {
        (void)fclose(fp);
        log_e("Failed to get uptime info");
        return ADM_ECODE_FAILED;
    }
    aUptimestr[pdotaddr - aTimestr] = '\0';
    adm_stringToInt64(aUptimestr,pStartInterval);
    log_d("aUptimestr:%s , pStartInterval:%lld",aUptimestr, *pStartInterval);

    (void)fclose(fp);
    return iRet;
}
/*!
 *******************************************************************************
 * 检查是否需要记录进程信息
 *
 * \note
 * \param[in]     pstRecordInfo        需要记录的进程信息
 * \retval        D32S
 *******************************************************************************
 */
D32S apcIsRecordProcessInfo(ADM_APC_RECORD_INFO_T *pstRecordInfo)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    D64S iStartInterval = 0.0; //设备启动时间

    if (ADM_IS_NULL(pstRecordInfo))
    {
        log_e( "Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }

    //1、 获取设备启动时间
    iRet = apcGetEquipmentstartInterval(&iStartInterval);
    if (ADM_IS_ERR(iRet))
    {
        log_e("Failed to get equipment start interval info, err[%d]", iRet);
        return iRet;
    }
    
    if(iStartInterval < ADM_APC_PROCESS_START_INTERVAL)
    {
        //清空之前存储的数据
        (void)adm_com_kvstorage_remove_key(adm_core_service_getBusinessDatabaseDb(), ADM_APC_PROCESS_INFO_KV_KEY);

        log_d("start interval less than [%d]s", ADM_APC_PROCESS_START_INTERVAL);
        return ADM_ECODE_FAILED;
    }

    //获取进程信息
    pstRecordInfo->iPid = getpid();
    if(0 == pstRecordInfo->iPid)
    {
        log_e("pid is zero");
        iRet = ADM_ECODE_FAILED;
    }

    pstRecordInfo->iRecordTime = iStartInterval;
    log_d("iRecordTime [%lld]s", pstRecordInfo->iRecordTime);

    return iRet;
}

/*!
 *******************************************************************************
 * 检查进程信息,并设置进程信息到数据库.
 *
 * \note
 *
 * \retval        D32S
 *******************************************************************************
 */
D32S adm_apc_checkProcess(void)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    KvStorage_t *pstOtaKvHandle = NULL;
    ADM_APC_RECORD_INFO_T *pstApcRecordInfoBegin = NULL;
    ADM_APC_RECORD_INFO_T *pstApcRecordInfoEnd = NULL;
    ADM_APC_PRO_RECORD_T stProRecord = {0};
    ADM_APC_RECORD_INFO_T stProRecordInfo = {0};
    DBOOL bIsKvdataAbDeal = FALSE;
    D32S  iProResTimes = 0;
    D32S  iProResTotalTime = 0;
    D64S  iTempRecordtime = 0;
    D64S iTimeDiff = 0;   //时间差

    //是否进行异常处理
    bIsKvdataAbDeal = adm_cfg_iniParserGetBoolean(ADM_CFG_VUS_IS_SET_KVDATA_ABNORMAL_DEAL, (DBOOL)ADM_CFG_VUS_IS_SET_KVDATA_ABNORMAL_DEAL_VAL);
    if(FALSE == bIsKvdataAbDeal)
    {
        return ADM_ECODE_SUCCESSED;
    }

    //参数有效性校验
    iProResTimes = adm_cfg_iniParserGetInt(ADM_CFG_VUS_PROCESS_RESTART_TIMES, ADM_CFG_VUS_PROCESS_RESTART_TIMES_VAL);
    iProResTotalTime = adm_cfg_iniParserGetInt(ADM_CFG_VUS_PROCESS_RESTART_TOTALTIME, ADM_CFG_VUS_PROCESS_RESTART_TOTALTIME_VAL);
    if((iProResTimes < ADM_APC_PROCESS_RESTART_MINIMUN_TIMES) || (iProResTotalTime < ADM_APC_PROCESS_RESTART_MINIMUN_TOTALTIME))
    {
        //设置的参数无效,不处理
        log_d("config to get process restart times or totaltime is abnormal.");
        return ADM_ECODE_SUCCESSED;
    }

    do
    {
        //校验是否需要记录进行信息
        iRet = apcIsRecordProcessInfo(&stProRecordInfo);
        if (ADM_IS_ERR(iRet))
        {
            iRet = ADM_ECODE_SUCCESSED;
            log_e("no need to record for information to kv, err[%d]", iRet);
            break;
        }

        //获取进程关键信息 
        iRet = apcGetProcessInfoFromKV(&stProRecord);
        if (ADM_IS_ERR(iRet))
        {
            log_d("Failed to get process record info from kv, err[%d]", iRet);
        }

        //将最新进程信息设置到kv中
        iRet = apcSetProcessInfo2KV(&stProRecord, &stProRecordInfo);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to set process record info from kv, err[%d]", iRet);
            break;
        }

        //当存储次数到达重启校验次数时,校验时间
        if(iProResTimes == stProRecord.iRecordNum)
        {
            pstApcRecordInfoBegin = &stProRecord.pstApcRecordInfoList[0];
            pstApcRecordInfoEnd = &stProRecord.pstApcRecordInfoList[stProRecord.iRecordNum-1];
            iTimeDiff = pstApcRecordInfoEnd->iRecordTime - pstApcRecordInfoBegin->iRecordTime;
            
            if(iTimeDiff <= iProResTotalTime)
            {
                //该段时间内重启次数过多,防止数据库异样导致的重启限制，故删除数据库，重新创建
                log_e("Too many restarts need to destory KV!");
                adm_core_service_delAndCreatKv();

                //将最新进程信息重新设置到kv中
                (void)adm_memset(&stProRecord,0x00,sizeof(stProRecord),sizeof(stProRecord));
                (void)adm_memset(&stProRecordInfo,0x00,sizeof(stProRecordInfo),sizeof(stProRecordInfo));
                stProRecordInfo.iPid = pstApcRecordInfoEnd->iPid;
                stProRecordInfo.iRecordTime = pstApcRecordInfoEnd->iRecordTime;
                iRet = apcSetProcessInfo2KV(&stProRecord, &stProRecordInfo);
                if (ADM_IS_ERR(iRet))
                {
                    log_e("Failed to set process record info from kv, err[%d]", iRet);
                    break;
                }

                //删除数据库，重新创建后,继续升级流程
                
            }
        }
    }while (FALSE);

    adm_free(stProRecord.pstApcRecordInfoList);

    return iRet;
}


#if 0
#endif

static void adm_apc_delDir(void)
{
    const DCHAR* pDirStr = NULL;

    /** 初始化kv存储目录 */
    pDirStr = adm_cfg_iniParserGetString(ADM_CFG_VUS_KV_STORAGE_DIR, ADM_CFG_VUS_KV_STORAGE_DIR_DEFAULT_VAL);
    adm_com_rmDirAndFiles(pDirStr);

    /** 初始化升级包存储目录 */
    pDirStr = adm_cfg_iniParserGetString(ADM_CFG_VUS_PKG_STORAGE_DIR, ADM_CFG_VUS_PKG_STORAGE_DIR_DEFAULT_VAL);
    adm_com_rmDirAndFiles(pDirStr);

    /** 初始化日志存储目录 */
    // pDirStr = adm_cfg_iniParserGetString(ADM_CFG_VUS_LOG_STORAGE_DIR, ADM_CFG_VUS_LOG_STORAGE_DIR_DEFAULT_VAL);
    // adm_com_rmDirAndFiles(pDirStr);

    /** 初始化日志收集存储目录 */
    // pDirStr = adm_cfg_iniParserGetString(ADM_CFG_VUS_LOG_COLLECT_DIR, ADM_CFG_VUS_LOG_COLLECT_DIR_DEFAULT_VAL);
    // adm_com_rmDirAndFiles(pDirStr);

    /** 初始化日志打包的存储目录 */
    // pDirStr = adm_cfg_iniParserGetString(ADM_CFG_VUS_LOG_PACK_DIR, ADM_CFG_VUS_LOG_PACK_DIR_DEFAULT_VAL);
    // adm_com_rmDirAndFiles(pDirStr);

    //!< 初始化文件解压的存储目录
    pDirStr = adm_cfg_iniParserGetString(ADM_CFG_VUS_UNZIP_STORAGE_DIR, ADM_CFG_VUS_UNZIP_STORAGE_DIR_DEFAULT_VAL);
    adm_com_rmDirAndFiles(pDirStr);

    /** 初始化文件传输的上传目录 */
    pDirStr = adm_cfg_iniParserGetString(ADM_CFG_EWEB_DOC_UPLOAD_DIR, ADM_CFG_EWEB_DOC_UPLOAD_DIR_DEFAULT_VAL);
    adm_com_rmDirAndFiles(pDirStr);

    //!< 初始化文件传输的下载目录
    pDirStr = adm_cfg_iniParserGetString(ADM_CFG_EWEB_DOC_DOWNLOAD_DIR, ADM_CFG_EWEB_DOC_DOWNLOAD_DIR_DEFAULT_VAL);
    adm_com_rmDirAndFiles(pDirStr);

    //!< ServerType 标识文件
    pDirStr = adm_cfg_iniParserGetString(ADM_CFG_EMS_SERVER_FLAG_FILE_DIR, ADM_CFG_EMS_SERVER_FLAG_FILE_DIR_DEFAULT_VAL);
    adm_com_rmDirAndFiles(pDirStr);

    return;
}


/*!
 *******************************************************************************
 * 清空数据，退出进程
 *
 * \note
 *
 * \retval        void
 *******************************************************************************
 */
void adm_apc_resetOTA()
{
    DCHAR *pVersionNumber = NULL;
    D32S iRet = ADM_ECODE_SUCCESSED;
    log_d("===== enter restore factory settings =====");

    //TODO:1、TODO:退出OTA工作环境

    //2、上报"已恢复出厂设置"的状态
    iRet = adm_eim_getVehicleBigVersion(&pVersionNumber);
    if (ADM_IS_ERR(iRet)) 
    {
        log_e("get Vehicle Big Version,error[%d]", iRet);
    }

    iRet = adm_irm_reportVehicleState(time(NULL), adm_sdk_getSDKVersion(),
                                    NULL,0,0,
                                    ADM_IRM_REPORT_SERVER_STATUS_Restore_Factory_Settings,
                                    "0",
                                    pVersionNumber,
                                    NULL,
                                    NULL);
    if (ADM_IS_ERR(iRet)) 
    {
        log_e("adm_irm_reportVehicleState,error[%d]",iRet);
    }

    //3、清除数据（数据库、日志、升级包等）
    adm_apc_delDir();

    //4、设置退出进程处理
    g_bIsEndProcess = TRUE;

    adm_apc_exitUCM();

    return;
}

/*!
 *******************************************************************************
 * 清空数据，退出进程
 *
 * \note
 *
 * \retval        void
 *******************************************************************************
 */
D32S adm_apc_exitUCM()
{
    ADM_VSM_STATE_E eState = ADM_VSM_STATE_MAX;

    adm_vsm_setHmiOnline(TRUE);
    eState = adm_vsm_stateGet();

    if (ADM_VSM_STATE_UPGRADE_DOING == eState)
    {
        log_e("eState:%d is upgrading, not allow exit", eState);
        return ADM_ECODE_OTA_ALLOW_FAILED;
    }

    log_w("!!!!!!!!!!!!!!!!!!!! OTA APP exit by Engineer Mode");
    log_w("!!!!!!!!!!!!!!!!!!!! OTA APP exit by Engineer Mode");
    log_w("!!!!!!!!!!!!!!!!!!!! OTA APP exit by Engineer Mode");

    exit(0);

    return 0;
}

/*!
 *******************************************************************************
 * 日志上报
 *
 * \note
 *
 * \retval        void
 *******************************************************************************
 */
void adm_apc_logReport()
{
    ADM_IRM_LOG_FILE_TG_REPORT_INFO_T streportInfo = {0};

    streportInfo.cmdId = ADM_OCS_CMDID_DEBUG;
    streportInfo.extsReqJsonStr = NULL;
    streportInfo.pushId = NULL;
    
    adm_irm_triggerLogFileReport(&streportInfo);

    return;
}

/*!
 *******************************************************************************
 * 日志收集
 *
 * \note
 *
 * \retval        void
 *******************************************************************************
 */
D32S adm_apc_logCollect(void)
{
    ADM_IRM_LOG_FILE_TG_REPORT_INFO_T streportInfo = {0};

    streportInfo.cmdId = ADM_OCS_CMDID_DEBUG;
    streportInfo.extsReqJsonStr = NULL;
    streportInfo.pushId = NULL;
    
    return adm_irm_triggerLogCollect(&streportInfo);
}

/*!
 *******************************************************************************
 * 删除升级包
 *
 * \note
 *
 * \retval        D32S
 *******************************************************************************
 */
D32S adm_apc_cleanDownladPackage(void)
{
    const DCHAR* cfgDownPath = DNULL;
    const DCHAR* cfgInstanceIds = DNULL;
    DCHAR* cfgInstanceIdsTmp = DNULL;
    DCHAR* subInstanceIdStr = NULL;
    D32S instanceId = 0;
    D32S iRet = ADM_ECODE_SUCCESSED;
    D32S iErrorCode = ADM_ECODE_FAILED;

    cfgDownPath = adm_cfg_iniParserGetString(ADM_CFG_VUS_PKG_STORAGE_DIR, ADM_CFG_VUS_PKG_STORAGE_DIR_DEFAULT_VAL);

    adm_com_rmDirAndFiles(cfgDownPath);

    cfgInstanceIds = adm_cfg_iniParserGetString(ADM_CFG_VUS_UCL_INSTANCEID, ADM_CFG_VUS_UCL_INSTANCEID_DEFAULT_VAL);
    cfgInstanceIdsTmp = adm_strdup(cfgInstanceIds);
    if (DNULL != cfgInstanceIdsTmp)
    {
        subInstanceIdStr = strsep(&cfgInstanceIdsTmp, ",");
        while (DNULL != subInstanceIdStr)
        {
            log_i("subInstanceIdStr:%s", subInstanceIdStr);
            instanceId = atoi(subInstanceIdStr);
            adm_dim_dpo_cleanDownloadDir(instanceId, 3, &iErrorCode);

            subInstanceIdStr = strsep(&cfgInstanceIdsTmp, ",");
        }
        adm_free(cfgInstanceIdsTmp);
    }

    return iRet;
}

static void* startHttpproxy(void * priv)
{
    D32S iRet = ADM_ECODE_FAILED;
    /** 启动网络代理*/
    DBOOL isHttpProxy = FALSE;
    isHttpProxy = adm_cfg_iniParserGetBoolean(ADM_CFG_VUS_IS_START_HTTP_PROXY, ADM_CFG_VUS_IS_START_HTTP_PROXY_DEFAULT_VAL);
    if (DTRUE == isHttpProxy)
    {
        const DCHAR *pHttpProxyfgPath = adm_cfg_iniParserGetString(ADM_CFG_VUS_IS_HTTP_PROXY_CFG_PATH, ADM_CFG_VUS_IS_HTTP_PROXY_CFG_PATH_DEFAULT_VAL);
        iRet = adm_http_proxyStart(pHttpProxyfgPath);
        if (ADM_ECODE_SUCCESSED != iRet) 
        {
            log_e("adm_http_proxyStart failed, iRet:%d", iRet);
        } 
        else 
        {
            log_i("adm_http_proxyStart successed");
        }
    }

    return NULL;
}


/**
 * @brief 网络代理初始化
 * 
 * @return D32S     0:Successed, None-0:Failed
 */
D32S adm_apc_httpproxyInit(void)
{
    D32S iRet = ADM_ECODE_FAILED;
    pthread_attr_t stThreadAttr = {0};
    pthread_t stThreadHandle = {0};

    /* 获取线程属性句柄 */
    iRet = pthread_attr_init(&stThreadAttr);
    if (ADM_IS_ERR(iRet))
    {
        log_e("pthread_attr_init failed. err[%d]", iRet);
        return ADM_ECODE_PTHREAD_CREATE_FAILED;
    }

    /* 设置线程分离属性 */
    iRet = pthread_attr_setdetachstate(&stThreadAttr, PTHREAD_CREATE_DETACHED);
    if (ADM_IS_ERR(iRet))
    {
        log_e("pthread_attr_setdetachstate failed. iRet[%u]", iRet);
        (void)pthread_attr_destroy(&stThreadAttr);
        return ADM_ECODE_PTHREAD_CREATE_FAILED;
    }

    //!< 启下载任务线程
    iRet = pthread_create(&stThreadHandle,
                          &stThreadAttr,
                          startHttpproxy,
                          NULL);
    if (ADM_IS_ERR(iRet))
    {
        log_e("Failed to create thread, err[%d]", iRet);
        (void)pthread_attr_destroy(&stThreadAttr);
        return ADM_ECODE_PTHREAD_CREATE_FAILED;
    }

    /* 销毁线程属性句柄 */
    (void)pthread_attr_destroy(&stThreadAttr);

    return iRet;
}

D32S adm_apc_vehicleConfigReset(DBOOL bIsInstalling)
{
    D32S iRet = ADM_ECODE_SUCCESSED;

    ADM_COM_FUNC_ENTRY();

    adm_ecs_sameConfigHandle(bIsInstalling, adm_eim_getSingleEcuDidInfo);

    ADM_COM_FUNC_EXIT();

    return iRet;
}

void adm_apc_taskVerify(void)
{
    D32S iRet = ADM_ECODE_FAILED;
    ADM_VSM_STATE_E eState = ADM_VSM_STATE_IDLE;

    ADM_COM_FUNC_ENTRY();

    eState = adm_vsm_stateGet();

    /** 如果是下载完成状态，需要进行任务有效性校验 */
    if (ADM_VSM_STATE_UPGRADE_READY == eState || ADM_VSM_STATE_UPGRADE_PAUSE == eState)
    {
        iRet = adm_dpm_taskVerify();
        if (ADM_ECODE_UPGRADE_TASK_STOPED == iRet)
        {
            log_e("Task has been stopped, err[%d]", iRet);
            adm_vsm_stateSet(ADM_VSM_STATE_IDLE);
            adm_ipm_clearSchduleInfo();
        }
    }

    g_bIsVerifyFinish = TRUE;

    ADM_COM_FUNC_EXIT();

    return ;
}