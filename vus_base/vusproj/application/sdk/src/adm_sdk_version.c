#if !defined(LOG_TAG)
#define LOG_TAG             "SDK"         /*!< 宏函数，用于通过模块名的枚举值类型获取模块名称     */
#endif
#include "adm_debug.h"
#include "adm_sdk_version.h"
#include "adm_memory.h"
#include "adm_string.h"
#include "adm_engine_common.h"
#include "adm_getFoundationVersion.h"
#include "adm_tracer_interface.h"
#include "adm_arpc_service.h"
#include "json.h"
#include "adm_com_errorCode.h"
#include "adm_com_kvstorage.h"
#include "adm_engine_common.h"
#include "adm_cfg_api.h"
#include "adm_dim_host.h"
#include "adm_dim_transHost.h"
#include "adm_dim_vdi.h"
#include "adm_dim_ucl.h"
#include "adm_uim_request.h"
#include "adm_soa_cfg.h"
//!< 断言 ewebserver 的支持 
#ifdef CMAKE_CUSTOM_EWEBSERVER_SUPPORT
    #include "ab_ewebsrv_api.h"
#endif

#define ADM_SDK_VERSION_NAME_JSON_KEY               "moduleName"           //!< 定义云端请求的versionName json字段
#define ADM_SDK_VERSION_NUMBER_JSON_KEY             "moduleValue"          //!< 定义云端请求的versionValue json字段
#define ADM_SDK_VERSION_MODULE_NAME_ENGINE          "engine"                //!< 模块名 engine
#define ADM_SDK_VERSION_MODULE_NAME_FOUNDATION      "foundation"            //!< 模块名 foundation
#define ADM_SDK_VERSION_MODULE_NAME_EWEBSERVER      "ewebserver"            //!< 模块名 ewebserver
#define ADM_SDK_VERSION_MODULE_NAME_TOOLS           "tools"                 //!< 模块名 tools
#define ADM_SDK_VERSION_MODULE_NAME_ARPC            "arpc"                  //!< 模块名 arpc
#define ADM_SDK_VERSION_MODULE_NAME_FOTAAPP         "admfotaapp"            //!< 模块名 admfotaapp
#define ADM_SDK_VERSION_MODULE_NAME_SYSI4HOST       "SYSI4HOST"             //!< 模块名 SYSI4HOST
#define ADM_SDK_VERSION_MODULE_NAME_SYSI4TRANS      "SYSI4TRANS"             //!< 模块名 SYSI4TRANS
#define ADM_SDK_VERSION_MODULE_NAME_SYSI4VDI        "SYSI4VDI"              //!< 模块名 SYSI4VDI
#define ADM_SDK_VERSION_MODULE_NAME_SYSI4UCL       "SYSI4UCL"             //!< 模块名 SYSI4UCL
#define ADM_SDK_VERSION_MODULE_NAME_SYSI4HMI       "SYSI4HMI"             //!< 模块名 SYSI4HMI
#define ADM_SDK_VERSION_MODULE_NAME_HMI             "HMI"             //!< 模块名 HMI
#define ADM_SDK_VERSION_MODULE_NAME_SYSI            "sysi"                  //!< 模块名 sysi    ??? SYSI会有多个模块，需要通过DIM获取

static const DCHAR* g_allModulesVers = "allModulesVers";    //!< 所有模块的版本号，存储到KV中的KEY
#define ADM_SDK_VERSION_GET_FROM_DIM_TIME_OUT        1       //!< 通过ARPC获取版本号的超时时间，默认为1s

/**
 * @brief 创建版本号的json object，包含字段：versionName和versionNumber
 * 
 * @param jsonVersionList   [IN]    json 数组
 * @param versionName       [IN]    模块的名称
 * @param versionNumber     [IN]    模块的版本号值
 */
static void __adm_sdk_addSingleModuleVersion(json_object* jsonVersionList, const DCHAR* versionName, const DCHAR* versionNumber)
{
    json_object* versionObj = NULL;

    do {
        //!< 参数校验
        if (NULL == versionName || NULL == versionNumber) {
            log_e("Input param versionName or versionNumber is nil");
            break;
        }

        //!< 申请json_object的空间
        versionObj = json_object_new_object();
        if (NULL == versionObj) {
            log_e("json_object_new_object failed");
            break;
        }

        //!< 将versionName的key和value添加到json object中
        if (ADM_ECODE_SUCCESSED != json_object_object_add(versionObj, ADM_SDK_VERSION_NAME_JSON_KEY, json_object_new_string(versionName))) {
            log_e("json_object_object_add key:%s failed", ADM_SDK_VERSION_NAME_JSON_KEY);
            break;
        }

        //!< 将versionValue的key和value添加到json object中
        if (ADM_ECODE_SUCCESSED != json_object_object_add(versionObj, ADM_SDK_VERSION_NUMBER_JSON_KEY, json_object_new_string(versionNumber))) {
            log_e("json_object_object_add key:%s failed", ADM_SDK_VERSION_NUMBER_JSON_KEY);
            break;
        }

        (void)json_object_array_add(jsonVersionList, versionObj);
    } while(FALSE);

    return;
}

/**
 * @brief 获取所有模块的版本号
 * @note 
 * 
 * @return const DCHAR*     NULL:Failed, None-NULL: json list 格式字符串，用于上传所有模块版本号
 * @note 返回值示例：
 *  [
 *      {
 *          "versionName":"admotaapp",
 *          "versionValue":"v1.1.1"
 *      },
 *      {
 *          "versionName":"engine",
 *          "versionValue":"v1.1.1"
 *      }
 *  ]
 */
DCHAR* adm_sdk_getAllModulesVersionFormatJsonStr(void)
{
#ifndef ADM_DEBUG_MODULE_VERS
    //TODO: 4月25号，云端版本发布后，放开此字段
    return NULL;
#endif//ADM_DEBUG_MODULE_VERS

    DCHAR sdkVersion[128] = {0};
    json_object* jsonVersionList = NULL;
    DCHAR* allModuleVersionStr = NULL;
    const DCHAR* jsonValStr = NULL;
    D16U instanceId = 0;
    const DCHAR* cfgInstanceIdsStr = NULL;
    DCHAR* instanceIdStr = NULL;
    DCHAR* subInstanceIdStr = NULL;
    DCHAR* pSdkVersion = NULL;
    DCHAR* pApkVersion = NULL;
    D32S errCode = ADM_ECODE_FAILED;
    
    do {
        //!< 初始化数组
        jsonVersionList = json_object_new_array();
        if (NULL == jsonVersionList) {
            log_e("json_object_new_array failed");
            break;
        }
    
        //!< 获取engine模块的版本号
        (void)adm_memset(sdkVersion, 0x00, sizeof(sdkVersion), sizeof(sdkVersion));
        (void)adm_snprintf(sdkVersion, sizeof(sdkVersion), "%s", adm_core_service_sdkVersion());
        log_i("sdk version of engine module:%s", sdkVersion);
        __adm_sdk_addSingleModuleVersion(jsonVersionList, ADM_SDK_VERSION_MODULE_NAME_ENGINE, sdkVersion);

        //!< 获取foundation模块的版本号
        (void)adm_memset(sdkVersion, 0x00, sizeof(sdkVersion), sizeof(sdkVersion));
        (void)adm_snprintf(sdkVersion, sizeof(sdkVersion), "%s", adm_getFoundationVersion());
        log_i("sdk version of foundation module:%s", sdkVersion);
        __adm_sdk_addSingleModuleVersion(jsonVersionList, ADM_SDK_VERSION_MODULE_NAME_FOUNDATION, sdkVersion);

//!< 断言 ewebserver 的支持 
#ifdef CMAKE_CUSTOM_EWEBSERVER_SUPPORT
        //!< 获取ewebserver模块的版本号
        (void)adm_memset(sdkVersion, 0x00, sizeof(sdkVersion), sizeof(sdkVersion));
        (void)adm_snprintf(sdkVersion, sizeof(sdkVersion), "%s", abEwebsrv_GetSdkVersion());
        log_i("sdk version of ewebserver module:%s", sdkVersion);
        __adm_sdk_addSingleModuleVersion(jsonVersionList, ADM_SDK_VERSION_MODULE_NAME_EWEBSERVER, sdkVersion);
#endif

        //!< 获取Tools 模块的版本号
#ifdef __MACRO_DTS_TOOL_SWITCH__
        (void)adm_memset(sdkVersion, 0x00, sizeof(sdkVersion), sizeof(sdkVersion));
        (void)adm_snprintf(sdkVersion, sizeof(sdkVersion), "%s", adm_dtsNodeGetSdkVersion());
        log_i("sdk version of tracer tools module:%s", sdkVersion);
        __adm_sdk_addSingleModuleVersion(jsonVersionList, ADM_SDK_VERSION_MODULE_NAME_TOOLS, sdkVersion);
#endif
        //!< 获取 ARPC 模块的版本号
        (void)adm_memset(sdkVersion, 0x00, sizeof(sdkVersion), sizeof(sdkVersion));
        (void)adm_snprintf(sdkVersion, sizeof(sdkVersion), "%s", adm_arpc_getSdkVersion());
        log_i("sdk version of arpc module:%s", sdkVersion);
        __adm_sdk_addSingleModuleVersion(jsonVersionList, ADM_SDK_VERSION_MODULE_NAME_ARPC, sdkVersion);

        //!< 获取 admfotaapp 模块的版本号
        (void)adm_memset(sdkVersion, 0x00, sizeof(sdkVersion), sizeof(sdkVersion));
        (void)adm_snprintf(sdkVersion, sizeof(sdkVersion), "%s", adm_sdk_getSDKVersion());
        log_i("sdk version of admfotaapp module:%s", sdkVersion);
        __adm_sdk_addSingleModuleVersion(jsonVersionList, ADM_SDK_VERSION_MODULE_NAME_FOTAAPP, sdkVersion);

        //!< 获取 SYSI4HOST 模块的版本号
        cfgInstanceIdsStr = adm_cfg_iniParserGetString(ADM_CFG_VUS_HOST_INSTANCEID, "");
        instanceIdStr = adm_strdup(cfgInstanceIdsStr);
        if (NULL != instanceIdStr) {
            subInstanceIdStr = strsep(&instanceIdStr, ",");
            log_i("subInstanceIdStr:%s", subInstanceIdStr);
            while(NULL != subInstanceIdStr) {
                instanceId = (D16U)atoi(subInstanceIdStr);
                adm_dim_host_getSdkVersion(instanceId, ADM_SDK_VERSION_GET_FROM_DIM_TIME_OUT, &pSdkVersion, &errCode);
                if (ADM_ECODE_SUCCESSED == errCode && NULL != pSdkVersion) {
                    (void)adm_memset(sdkVersion, 0x00, sizeof(sdkVersion), sizeof(sdkVersion));
                    (void)adm_snprintf(sdkVersion, sizeof(sdkVersion), "%s", pSdkVersion);
                    log_i("sdk version of SYSI4HOST module:%s", sdkVersion);
                    __adm_sdk_addSingleModuleVersion(jsonVersionList, ADM_SDK_VERSION_MODULE_NAME_SYSI4HOST, sdkVersion);
                    adm_free(pSdkVersion);
                }
                subInstanceIdStr = strsep(&instanceIdStr, ",");
            }
            adm_free(instanceIdStr);
        }

        // !< 获取 SYSI4VDI 模块的版本号
        cfgInstanceIdsStr = adm_cfg_iniParserGetString(ADM_CFG_VUS_VDI_INSTANCEID, "");
        instanceIdStr = adm_strdup(cfgInstanceIdsStr);
        if (NULL != instanceIdStr) {
            subInstanceIdStr = strsep(&instanceIdStr, ",");
            log_i("subInstanceIdStr:%s", subInstanceIdStr);
            while(NULL != subInstanceIdStr) {
                instanceId = (D16U)atoi(subInstanceIdStr);
                adm_dim_vdi_getSdkVersion(instanceId, ADM_SDK_VERSION_GET_FROM_DIM_TIME_OUT, &pSdkVersion, &errCode);
                if (ADM_ECODE_SUCCESSED == errCode && NULL != pSdkVersion) {
                    (void)adm_memset(sdkVersion, 0x00, sizeof(sdkVersion), sizeof(sdkVersion));
                    (void)adm_snprintf(sdkVersion, sizeof(sdkVersion), "%s", pSdkVersion);
                    log_i("sdk version of SYSI4VDI module:%s", sdkVersion);
                    __adm_sdk_addSingleModuleVersion(jsonVersionList, ADM_SDK_VERSION_MODULE_NAME_SYSI4VDI, sdkVersion);
                    adm_free(pSdkVersion);
                }
                   subInstanceIdStr = strsep(&instanceIdStr, ",");
            }
            adm_free(instanceIdStr);
        }

        //!< 获取 SYSI4UCL 模块的版本号
        cfgInstanceIdsStr = adm_cfg_iniParserGetString(ADM_CFG_VUS_UCL_INSTANCEID, "");
        instanceIdStr = adm_strdup(cfgInstanceIdsStr);
        if (NULL != instanceIdStr) {
            subInstanceIdStr = strsep(&instanceIdStr, ",");
            log_i("subInstanceIdStr:%s", subInstanceIdStr);
            while(NULL != subInstanceIdStr) {
                instanceId = (D16U)atoi(subInstanceIdStr);
                adm_dim_ucl_getSdkVersion(instanceId, ADM_SDK_VERSION_GET_FROM_DIM_TIME_OUT, &pSdkVersion, &errCode);
                if (ADM_ECODE_SUCCESSED == errCode && NULL != pSdkVersion) {
                    (void)adm_memset(sdkVersion, 0x00, sizeof(sdkVersion), sizeof(sdkVersion));
                    (void)adm_snprintf(sdkVersion, sizeof(sdkVersion), "%s", pSdkVersion);
                    log_i("sdk version of SYSI4VDI module:%s", sdkVersion);
                    __adm_sdk_addSingleModuleVersion(jsonVersionList, ADM_SDK_VERSION_MODULE_NAME_SYSI4UCL, sdkVersion);
                    adm_free(pSdkVersion);
                }
                subInstanceIdStr = strsep(&instanceIdStr, ",");
            }
            adm_free(instanceIdStr);
        }

        //!< 获取 SYSI4HMI 模块的版本号
        instanceId = SYSI4HMI2HUT_INTSTANCE_ID;
        adm_uim_hmi_getSdkVersion(instanceId, ADM_SDK_VERSION_GET_FROM_DIM_TIME_OUT, &pSdkVersion, &pApkVersion,&errCode);
        if (ADM_ECODE_SUCCESSED == errCode && NULL != pSdkVersion) {
            (void)adm_memset(sdkVersion, 0x00, sizeof(sdkVersion), sizeof(sdkVersion));
            (void)adm_snprintf(sdkVersion, sizeof(sdkVersion), "%s", pSdkVersion);
            log_i("sdk version of SYSI4HMI module:%s", sdkVersion);
            __adm_sdk_addSingleModuleVersion(jsonVersionList, ADM_SDK_VERSION_MODULE_NAME_SYSI4HMI, sdkVersion);

            (void)adm_memset(sdkVersion, 0x00, sizeof(sdkVersion), sizeof(sdkVersion));
            (void)adm_snprintf(sdkVersion, sizeof(sdkVersion), "%s", pApkVersion);
            __adm_sdk_addSingleModuleVersion(jsonVersionList, ADM_SDK_VERSION_MODULE_NAME_HMI, sdkVersion);
        }

        //! 将jsonList的object，转换为json字符串
        jsonValStr = json_object_to_json_string(jsonVersionList);
        allModuleVersionStr = adm_strdup(jsonValStr);

    } while(FALSE);

    //!< 将json array object释放，注：此处由数组使用者释放
    if ((D32U)0 != json_object_array_length(jsonVersionList)) {
        (void)json_object_put(jsonVersionList);
    }

    //!< 将所有模块的版本号，存储到KV中
    if (NULL != allModuleVersionStr) {
        (void)adm_com_kvstorage_set_string(adm_core_service_getBusinessDatabaseDb(), g_allModulesVers, allModuleVersionStr, ADM_KV_STORAGE_VALUE_TYPE_JSON);
    }

    log_i("allModuleVersionStr:%s", allModuleVersionStr);
    return allModuleVersionStr;
}

/**
 * @brief 获取OTA程序的主版本号，包括：APP、foundation、vrpc、eweb、sysi等模块的主版本号
 * @note 返回值使用完成后，需要执行内存释放
 * 
 * @return DCHAR*   NULL:Failed，None-NULL:返回的主版本号
 */
const DCHAR* adm_sdk_getMajorVersion(void)
{
    static DCHAR version[128] = {0};

    (void)adm_memset(version, 0x00, sizeof(version), sizeof(version) * sizeof(DCHAR));
    // TODO: 定义主版本号

    return version;
}

/**
 * @brief 获取当前sdk的版本号，例如：APP、foundation、vrpm、eweb、sysi等各sdk
 * 
 * @note 返回值使用完成后，需要执行内存释放
 * 
 * @return DCHAR*   NULL:Failed，None-NULL:返回的sdk版本号
 */
const DCHAR* adm_sdk_getSDKVersion(void)
{
    static DCHAR version[128] = {0};

    (void)adm_memset(version, 0x00, sizeof(version), sizeof(version) * sizeof(DCHAR));
#ifdef ADM_SDK_VERSION
    (void)adm_snprintf(version, sizeof(version), "%s", ADM_SDK_VERSION);    
#else//ADM_SDK_VERSION
    (void)adm_snprintf(version, sizeof(version), "%s", ADM_OTA_DEFAULT_VERSION);
#endif//ADM_SDK_VERSION

    return version;
}
