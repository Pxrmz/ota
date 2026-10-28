#include "adm_ocm_log.h"
#include "adm_ocm_interface.h"
#include "adm_oca_connect.h"
#include "adm_oca_file.h"
#include "adm_cfg_api.h"
#include "adm_ecs_rsConfig.h"
#include "adm_ems_serverType.h"
#include "adm_sdk_version.h"

#include "adm_string.h"
#include "adm_memory.h"
#include "adm_com_errorCode.h"
#include "adm_vrs_registerInfo.h"
#include "adm_ecs_rsConfig.h"
#include "json-c/json.h"
#include "adm_irs_upload.h"
#include <pthread.h>

struct __Lock {
    pthread_mutex_t lock;
    D32S initState;
};

static struct __Lock _businessLock = {.initState = 0};
static struct __Lock _downloadLock = {.initState = 0};
static struct __Lock _uploadLock = {.initState = 0};
static struct __Lock _dealStatusLock = {.initState = 0};

static ADM_OCM_UPDATE_OTACONFIG_CACHE_INFO_CALLBACK_F g_updateCfgCb = NULL;
static ADM_OCM_APC_KVSTORAGE_INIT_CALLBACK_F g_kvInitCb = NULL;
static ADM_OCM_APC_OTA_FIRST_BLOOD_CALLBACK_F g_otaFirstBlCb = NULL;
static ADM_OCM_UPDATE_ECU_CACHE_INFO_CALLBACK_F g_updateEcuCacheCb = NULL;
static ADM_OCM_VSM_STATE_DEL_CALLBACK_F g_stateDelCb = NULL;
static ADM_OCM_APC_LOG_INIT_CALLBACK_F g_logInitCb = NULL;
static D32U g_uiServerType = 1;
static DBOOL g_bIsUpdateConfig = FALSE;
// static DCHAR* g_pDomainName1 = NULL; 
// static DCHAR* g_pDomainName2 = NULL;

// #define BRAZIL_DOMAINNAME_URL1 "api-ota-lioncloud-stg-br.lionaitech.com"   /**测试*/
// #define BRAZIL_DOMAINNAME_URL2 "api-ota-lioncloud.omodajaecoo.com.br"             /**生产*/
// #define MEXICO_DOMAINNAME_URL1 "api-ota-lioncloud-stg-mx.lionaitech.com"   /**测试*/
// #define MEXICO_DOMAINNAME_URL2 "api-ota-lioncloud.chirey.mx"             /**生产*/
// #define ASEAN_DOMAINNAME_URL1 "api-ota-lioncloud-stg-br.lionaitech.com"
// #define ASEAN_DOMAINNAME_URL2 "api-ota-lioncloud.mychery.com"


// typedef struct 
// {
//     ADM_COUNTRY_E eCountryCode[32];         ///< 国家码
//     ADM_BRAND_E  eBrandCode[10];            ///< 品牌码
//     const DCHAR* pTestDomainUrl;            ///< 映射测试环境域名
//     const DCHAR* pProductDomainUrl;         ///< 映射生产环境域名
// } ADM_OCM_DOMAIN_T;

// static ADM_OCM_DOMAIN_T g_aDomain[] = 
// {
// 	/**巴西*/
// 	{{COUNTRY_BR}, {CHERY_BRAND,OMODA_BRAND,JAECOO_BRAND,}, BRAZIL_DOMAINNAME_URL1, BRAZIL_DOMAINNAME_URL2},
// 	/**东盟 新加坡*/
// 	{{COUNTRY_SG}, {CHERY_BRAND,OMODA_BRAND,JAECOO_BRAND,EXLANTIX_BRAND}, ASEAN_DOMAINNAME_URL1, ASEAN_DOMAINNAME_URL2},
// 	/**东盟 马来西亚*/
// 	{{COUNTRY_MY}, {CHERY_BRAND,OMODA_BRAND,JAECOO_BRAND,EXLANTIX_BRAND}, ASEAN_DOMAINNAME_URL1, ASEAN_DOMAINNAME_URL2},
// 	/**东盟 泰国*/
// 	{{COUNTRY_TH}, {CHERY_BRAND,OMODA_BRAND,JAECOO_BRAND,EXLANTIX_BRAND}, ASEAN_DOMAINNAME_URL1, ASEAN_DOMAINNAME_URL2},
// 	/**东盟 印度尼西亚*/
// 	{{COUNTRY_ID}, {CHERY_BRAND,OMODA_BRAND,JAECOO_BRAND,EXLANTIX_BRAND}, ASEAN_DOMAINNAME_URL1, ASEAN_DOMAINNAME_URL2},
// 	/**墨西哥*/
//     {{COUNTRY_MX}, {CHERY_BRAND,OMODA_BRAND,JAECOO_BRAND,EXLANTIX_BRAND}, MEXICO_DOMAINNAME_URL1, MEXICO_DOMAINNAME_URL2},
 
// };

//!< 业务通信锁定义
static void __businessLockInitOnce(void)
{
    if (_businessLock.initState == 1)
    {
        return;
    }
    (void)pthread_mutex_init(&_businessLock.lock, NULL);
    _businessLock.initState = 1;
}

static void __businessLock(void)
{
    __businessLockInitOnce();
    (void)pthread_mutex_lock(&_businessLock.lock);
}

static void __businessUnlock(void)
{
    __businessLockInitOnce();
    (void)pthread_mutex_unlock(&_businessLock.lock);
}

//!< 文件下载锁定义
static void __downloadLockInitOnce(void)
{
    if (_downloadLock.initState == 1)
    {
        return;
    }
    (void)pthread_mutex_init(&_downloadLock.lock, NULL);
    _downloadLock.initState = 1;
}

static void __downloadLock(void)
{
    __downloadLockInitOnce();
    (void)pthread_mutex_lock(&_downloadLock.lock);
}

static void __downloadUnlock(void)
{
    __downloadLockInitOnce();
    (void)pthread_mutex_unlock(&_downloadLock.lock);
}

//!< 文件上传锁的定义
static void __uploadLockInitOnce(void)
{
    if (_uploadLock.initState == 1)
    {
        return;
    }
    (void)pthread_mutex_init(&_uploadLock.lock, NULL);
    _uploadLock.initState = 1;
}

static void __uploadLock(void)
{
    __uploadLockInitOnce();
    (void)pthread_mutex_lock(&_uploadLock.lock);
}

static void __uploadUnlock(void)
{
    __uploadLockInitOnce();
    (void)pthread_mutex_unlock(&_uploadLock.lock);
}

//!< 状态处理锁
static void __dealStatusLockInitOnce(void)
{
    if (_dealStatusLock.initState == 1)
    {
        return;
    }
    (void)pthread_mutex_init(&_dealStatusLock.lock, NULL);
    _dealStatusLock.initState = 1;
}

static void __dealStatusLock(void)
{
    __dealStatusLockInitOnce();
    (void)pthread_mutex_lock(&_dealStatusLock.lock);
}

static void __dealStatusUnlock(void)
{
    __dealStatusLockInitOnce();
    (void)pthread_mutex_unlock(&_dealStatusLock.lock);
}


// static D32S ocmGetCountryDomain(ADM_COUNTRY_E eCountryCode, ADM_BRAND_E eBrandCode, DCHAR** ppTestDomain, DCHAR** ppProductDomain)
// {
//     D32S iRet = ADM_ECODE_FAILED;
//     D32U i = 0;
//     D32U j = 0;
//     D32U k = 0;

//     log_i("Current coutry code:%d, Brand code:%d, to get PKI url.", (D32S)eCountryCode, (D32S)eBrandCode);

//     if (NULL == ppTestDomain || NULL == ppProductDomain)
//     {
//         log_e("Invalid param.");
//         return ADM_ECODE_INVALID_PARAM;
//     }

//     if (eCountryCode < COUNTRY_CN || eCountryCode >= COUNTRY_DEFAULT ||  eBrandCode < CHERY_BRAND || eBrandCode >= DEFAULT_BRAND)
//     {
//         log_e("Invalid param.");
//         return ADM_ECODE_INVALID_PARAM;
//     }

//     for (i = 0; i < sizeof(g_aDomain)/sizeof(g_aDomain[0]); i++)
//     {
//         for (j = 0; j < sizeof(g_aDomain[i].eCountryCode)/sizeof(g_aDomain[i].eCountryCode[0]); j++)
//         {
//             if (eCountryCode == g_aDomain[i].eCountryCode[j])
//             {
//                 for (k = 0; k < sizeof(g_aDomain[i].eBrandCode)/sizeof(g_aDomain[i].eBrandCode[0]); k++)
//                 {
//                     if (eBrandCode == g_aDomain[i].eBrandCode[k])
//                     {
//                         /**车云 URL地址*/
//                         *ppTestDomain = adm_strdup(g_aDomain[i].pTestDomainUrl);
//                         *ppProductDomain = adm_strdup(g_aDomain[i].pProductDomainUrl);
//                         iRet = ADM_ECODE_SUCCESSED;
//                         break;
//                     }
//                 }
//             }
//         }
//     }

//     return iRet;
// }

/**
 * @brief 从注册信息中，获取deviceId，用于URL的生成
 * 
 * @return DCHAR*   NULL:Failed(deviceId获取失败), None-NULL:Successed(deviceId获取成功)
 */
static DCHAR* __adm_ocm_getDeviceId(void)
{
    ADM_VRS_REGISTER_INFO_T* pstRegisterInfo = NULL;
    DCHAR* deviceId = NULL;
    D32S iRet = ADM_ECODE_FAILED;

    do {
        //!< 申请注册信息空间
        pstRegisterInfo = adm_vrs_newRegisterInfo();
        if (NULL == pstRegisterInfo) {
            log_e("adm_vrs_newRegisterInfo failed");
            break;
        }

        //!< 获取注册信息
        iRet = adm_vrs_getRegisterInfo(pstRegisterInfo);
        if (ADM_ECODE_SUCCESSED != iRet) {
            log_e("adm_vrs_getRegisterInfo failed, err:%d", iRet);
            break;
        }

        //!< 判断获取到的deviceId是否为空
        if (NULL == pstRegisterInfo->deviceId) {
            iRet = ADM_ECODE_INVALID_POINTER;
            log_e("Got deviceId failed, err:%d", iRet);
            break;
        }

        //!< deviceId copy
        deviceId = adm_strdup(pstRegisterInfo->deviceId);

    } while(FALSE);

    //!< 释放注册信息空间
    adm_vrs_delRegisterInfo(pstRegisterInfo);

    return deviceId;
}

/**
 * @brief 通过车云通信协议中的接口ID标识，获取或拼接URL
 * 
 * @param eBusinessType     [IN]    车云通信协议中的接口ID标识
 * @return DCHAR*           NULL:URL获取失败，None-URL:待请求的URL
 * @note 返回的URL，使用完成后需要释放空间
 */
DCHAR* adm_ocm_getCommUrl(const ADM_OCS_BUSINESS_TYPE_E eBusinessType)
{
    DCHAR* url = NULL;
    const DCHAR* cfgUrlStr = NULL;
    DCHAR* deviceId = NULL;
    DCHAR urlStrBuf[256] = {0};
    // DCHAR *pDomainName = NULL;

    // if (ADM_EMS_SERVER_TYPE_DEFAULT_2 == g_uiServerType)
    // {
    //     pDomainName = adm_strdup(g_pDomainName2);
    // }
    // else
    // {
    //     pDomainName = adm_strdup(g_pDomainName1);
    // }
    // log_i("domainName:%s",pDomainName);

    //!< 根据车云通信协议中定义的标识，获取不同的URL
    switch (eBusinessType)
    {
    case ADM_OCS_BUSINESS_TYPE_REGISTER:
        if (ADM_EMS_SERVER_TYPE_DEFAULT_2 == g_uiServerType)
        {
            cfgUrlStr = adm_cfg_iniParserGetString(ADM_CFG_VUS_OTA_SRV_REGISTER_URL_2, ADM_CFG_VUS_OTA_SRV_REGISTER_URL_DEFAULT_VAL_2);
        }
        else
        {
            cfgUrlStr = adm_cfg_iniParserGetString(ADM_CFG_VUS_OTA_SRV_REGISTER_URL, ADM_CFG_VUS_OTA_SRV_REGISTER_URL_DEFAULT_VAL);
        }
        break;
    case ADM_OCS_BUSINESS_TYPE_GET_CFG:
        if (ADM_EMS_SERVER_TYPE_DEFAULT_2 == g_uiServerType)
        {
            cfgUrlStr = adm_cfg_iniParserGetString(ADM_CFG_VUS_OTA_SRV_GET_CONFIG_URL_2, ADM_CFG_VUS_OTA_SRV_GET_CONFIG_URL_DEFAULT_VAL_2);
        }
        else
        {
            cfgUrlStr = adm_cfg_iniParserGetString(ADM_CFG_VUS_OTA_SRV_GET_CONFIG_URL, ADM_CFG_VUS_OTA_SRV_GET_CONFIG_URL_DEFAULT_VAL);
        }
        break;
    case ADM_OCS_BUSINESS_TYPE_GET_ICP:
        if (ADM_EMS_SERVER_TYPE_DEFAULT_2 == g_uiServerType)
        {
            cfgUrlStr = adm_cfg_iniParserGetString(ADM_CFG_VUS_OTA_SRV_GET_ICP_URL_2, ADM_CFG_VUS_OTA_SRV_GET_ICP_URL_DEFAULT_VAL_2);
        }
        else
        {
            cfgUrlStr = adm_cfg_iniParserGetString(ADM_CFG_VUS_OTA_SRV_GET_ICP_URL, ADM_CFG_VUS_OTA_SRV_GET_ICP_URL_DEFAULT_VAL);
        }
        break;   
    case ADM_OCS_BUSINESS_TYPE_REPORT_VEHICLE_INFO:
        if (ADM_EMS_SERVER_TYPE_DEFAULT_2 == g_uiServerType)
        {
            cfgUrlStr = adm_cfg_iniParserGetString(ADM_CFG_VUS_OTA_SRV_REPORT_VEHICLE_INFO_URL_2, ADM_CFG_VUS_OTA_SRV_REPORT_VEHICLE_INFO_URL_DEFAULT_VAL_2);
        }
        else
        {
            cfgUrlStr = adm_cfg_iniParserGetString(ADM_CFG_VUS_OTA_SRV_REPORT_VEHICLE_INFO_URL, ADM_CFG_VUS_OTA_SRV_REPORT_VEHICLE_INFO_URL_DEFAULT_VAL);
        }
        break;
    case ADM_OCS_BUSINESS_TYPE_CHECK_VER:
        if (ADM_EMS_SERVER_TYPE_DEFAULT_2 == g_uiServerType)
        {
            cfgUrlStr = adm_cfg_iniParserGetString(ADM_CFG_VUS_OTA_SRV_CHECK_URL_2, ADM_CFG_VUS_OTA_SRV_CHECK_URL_DEFAULT_VAL_2);
        }
        else
        {
            cfgUrlStr = adm_cfg_iniParserGetString(ADM_CFG_VUS_OTA_SRV_CHECK_URL, ADM_CFG_VUS_OTA_SRV_CHECK_URL_DEFAULT_VAL);
        }
        break;
    case ADM_OCS_BUSINESS_TYPE_REPORT_VEHICLE_STATE:
        if (ADM_EMS_SERVER_TYPE_DEFAULT_2 == g_uiServerType)
        {
            cfgUrlStr = adm_cfg_iniParserGetString(ADM_CFG_VUS_OTA_SRV_VEHICLE_STATE_REPORT_URL_2, ADM_CFG_VUS_OTA_SRV_VEHICLE_STATE_REPORT_URL_DEFAULT_VAL_2);
        }
        else
        {
            cfgUrlStr = adm_cfg_iniParserGetString(ADM_CFG_VUS_OTA_SRV_VEHICLE_STATE_REPORT_URL, ADM_CFG_VUS_OTA_SRV_VEHICLE_STATE_REPORT_URL_DEFAULT_VAL);
        }
        break;
    case ADM_OCS_BUSINESS_TYPE_REPORT_ECU_STATE:
        if (ADM_EMS_SERVER_TYPE_DEFAULT_2 == g_uiServerType)
        {
            cfgUrlStr = adm_cfg_iniParserGetString(ADM_CFG_VUS_OTA_SRV_ECU_STATE_REPORT_URL_2, ADM_CFG_VUS_OTA_SRV_ECU_STATE_REPORT_URL_DEFAULT_VAL_2);
        }
        else
        {
            cfgUrlStr = adm_cfg_iniParserGetString(ADM_CFG_VUS_OTA_SRV_ECU_STATE_REPORT_URL, ADM_CFG_VUS_OTA_SRV_ECU_STATE_REPORT_URL_DEFAULT_VAL);
        }
        break;
    case ADM_OCS_BUSINESS_TYPE_REPORT_PROGRESS:
        if (ADM_EMS_SERVER_TYPE_DEFAULT_2 == g_uiServerType)
        {
            cfgUrlStr = adm_cfg_iniParserGetString(ADM_CFG_VUS_OTA_SRV_PROGRESS_REPORT_URL_2, ADM_CFG_VUS_OTA_SRV_PROGRESS_REPORT_URL_DEFAULT_VAL_2);
        }
        else
        {
            cfgUrlStr = adm_cfg_iniParserGetString(ADM_CFG_VUS_OTA_SRV_PROGRESS_REPORT_URL, ADM_CFG_VUS_OTA_SRV_PROGRESS_REPORT_URL_DEFAULT_VAL);
        }
        break;
    case ADM_OCS_BUSINESS_TYPE_REPORT_FILE:
        if (ADM_EMS_SERVER_TYPE_DEFAULT_2 == g_uiServerType)
        {
            cfgUrlStr = adm_cfg_iniParserGetString(ADM_CFG_VUS_OTA_SRV_FILE_UPLOAD_URL_2, ADM_CFG_VUS_OTA_SRV_FILE_UPLOAD_URL_DEFAULT_VAL_2);
        }
        else
        {
            cfgUrlStr = adm_cfg_iniParserGetString(ADM_CFG_VUS_OTA_SRV_FILE_UPLOAD_URL, ADM_CFG_VUS_OTA_SRV_FILE_UPLOAD_URL_DEFAULT_VAL);   
        }
        break;
    case ADM_OCS_BUSINESS_TYPE_REPORT_EVENT:
        if (ADM_EMS_SERVER_TYPE_DEFAULT_2 == g_uiServerType)
        {
            cfgUrlStr = adm_cfg_iniParserGetString(ADM_CFG_VUS_OTA_SRV_EVENT_REPORT_URL_2, ADM_CFG_VUS_OTA_SRV_EVENT_REPORT_URL_DEFAULT_VAL_2);
        }
        else
        {
            cfgUrlStr = adm_cfg_iniParserGetString(ADM_CFG_VUS_OTA_SRV_EVENT_REPORT_URL, ADM_CFG_VUS_OTA_SRV_EVENT_REPORT_URL_DEFAULT_VAL);
        }
        break;
    case ADM_OCS_BUSINESS_TYPE_PERCHECK_VER:
        if (ADM_EMS_SERVER_TYPE_DEFAULT_2 == g_uiServerType)
        {
            cfgUrlStr = adm_cfg_iniParserGetString(ADM_CFG_VUS_OTA_SRV_PRE_CHECK_URL_2, ADM_CFG_VUS_OTA_SRV_PRE_CHECK_URL_DEFAULT_VAL_2);
        }
        else
        {
            cfgUrlStr = adm_cfg_iniParserGetString(ADM_CFG_VUS_OTA_SRV_PRE_CHECK_URL, ADM_CFG_VUS_OTA_SRV_PRE_CHECK_URL_DEFAULT_VAL);
        }
        break;
    case ADM_OCS_BUSINESS_TYPE_GETUPGRADEHISTORY:
        if (ADM_EMS_SERVER_TYPE_DEFAULT_2 == g_uiServerType)
        {
            cfgUrlStr = adm_cfg_iniParserGetString(ADM_CFG_VUS_OTA_SRV_GET_HISTORY_URL_2, ADM_CFG_VUS_OTA_SRV_GET_HISTORY_DEFAULT_VAL_2);
        }
        else
        {
            cfgUrlStr = adm_cfg_iniParserGetString(ADM_CFG_VUS_OTA_SRV_GET_HISTORY_URL, ADM_CFG_VUS_OTA_SRV_GET_HISTORY_DEFAULT_VAL);
        }
        break;
    case ADM_OCS_BUSINESS_TYPE_SHEDULE_INFO_SYNC:
        if (ADM_EMS_SERVER_TYPE_DEFAULT_2 == g_uiServerType)
        {
            cfgUrlStr = adm_cfg_iniParserGetString(ADM_CFG_VUS_OTA_SRV_SYNC_SCHDULE_URL_2, ADM_CFG_VUS_OTA_SRV_SYNC_SCHDULE_URL_DEFAULT_VAL_2);
        }
        else
        {
            cfgUrlStr = adm_cfg_iniParserGetString(ADM_CFG_VUS_OTA_SRV_SYNC_SCHDULE_URL, ADM_CFG_VUS_OTA_SRV_SYNC_SCHDULE_URL_DEFAULT_VAL);
        }
        break;

    case ADM_OCS_BUSINESS_TYPE_TASK_VERIFY:
        if (ADM_EMS_SERVER_TYPE_DEFAULT_2 == g_uiServerType)
        {
            cfgUrlStr = adm_cfg_iniParserGetString(ADM_CFG_VUS_OTA_SRV_TASK_VERIFY_URL_2, ADM_CFG_VUS_OTA_SRV_TASK_VERIFY_URL_DEFAULT_VAL_2);
        }
        else
        {
            cfgUrlStr = adm_cfg_iniParserGetString(ADM_CFG_VUS_OTA_SRV_TASK_VERIFY_URL, ADM_CFG_VUS_OTA_SRV_TASK_VERIFY_URL_DEFAULT_VAL);
        }
        break;

    case ADM_OCS_BUSINESS_TYPE_REQUEST_UPLOAD_URL:
        if (ADM_EMS_SERVER_TYPE_DEFAULT_2 == g_uiServerType)
        {
            cfgUrlStr = adm_cfg_iniParserGetString(ADM_CFG_VUS_OTA_SRV_REQUEST_UPLOAD_URL_2, ADM_CFG_VUS_OTA_SRV_REQUEST_UPLOAD_URL_DEFAULT_VAL_2);
        }
        else
        {
            cfgUrlStr = adm_cfg_iniParserGetString(ADM_CFG_VUS_OTA_SRV_REQUEST_UPLOAD_URL, ADM_CFG_VUS_OTA_SRV_REQUEST_UPLOAD_URL_DEFAULT_VAL);
        }
        break;

    case ADM_OCS_BUSINESS_TYPE_INVALID:
    default:
        log_w("eBusinessType:%d is invalid", eBusinessType);
        break;
    }

    do {
        if (NULL == cfgUrlStr) {
            log_e("Get url from config failed");
            break;
        }

        //!< URL的动态生成
        (void)adm_memset(urlStrBuf, 0x00, sizeof(urlStrBuf), sizeof(urlStrBuf) * sizeof(DCHAR));
        if (ADM_OCS_BUSINESS_TYPE_REGISTER < eBusinessType && ADM_OCS_BUSINESS_TYPE_INVALID > eBusinessType) {
            //!< 从注册信息中获取deviceId
            deviceId = __adm_ocm_getDeviceId();
            if (NULL != deviceId) {
                (void)adm_snprintf(urlStrBuf, sizeof(urlStrBuf), cfgUrlStr, deviceId);
            }
        } else {
            (void)adm_snprintf(urlStrBuf, sizeof(urlStrBuf), cfgUrlStr);
        }

        url = adm_strdup(urlStrBuf);
        log_i("eBusinessType:%d get url successed, url:%s", eBusinessType, url);
    } while(FALSE);

    adm_free(deviceId);

    return url;
}

/**
 * @brief 处理云端返回车辆不存在的状态码
 * @note 车辆不存在，1. 删除本地信息；2. 状态机置为IDLE；3. 重新注册、获取配置信息
 */
static void __adm_ocm_dealWithVehicleNotExist(void)
{
    const DCHAR* str = NULL;

    do {
        //!< 状态机清空，非ADM_VSM_STATE_UPGRADE_DOING/ADM_VSM_STATE_UPGRADE_PAUSE/ADM_VSM_STATE_UPGRADE_BROKEN
        if (NULL != g_stateDelCb) {
            g_stateDelCb();
        }
        //!< 重新注册，并获取配置，同步ECU信息
        if (NULL != g_otaFirstBlCb) {
            //TODO: 添加ZT_004事件
            (void)g_otaFirstBlCb();
        }

    } while(FALSE);

}

/**
 * @brief 处理云端返回配置信息不存在的状态码
 * @note 从云端再次获取配置信息更新，默认3次重试
 * 
 */
static void __adm_ocm_dealWithConfigDoesNotExist(void)
{
    D32S iRet = ADM_ECODE_FAILED;
    D32S i = 0;

    if (NULL == g_updateCfgCb) {
        log_e("Update config interface is nil");
        return;
    }

    for (i = 0; i < 3; i++) {
        iRet = g_updateCfgCb(NULL);
        if (ADM_ECODE_SUCCESSED == iRet) {
            break;
        }
    }

    //!< 因为配置文件已经更新了，所以需要读取配置文件中的 Level 值，并更新给到日志模块
    if (ADM_ECODE_SUCCESSED == iRet && NULL != g_logInitCb) {
        (void)g_logInitCb();
    }

    return;
}

typedef struct {
    D32S statusCode;
} ADM_OCM_MY_PARAM_T;

/**
 * @brief 异步接口，平台反馈的状态码为：车辆配置信息需要更新
 * @note 产品反馈确认：不需要再次执行版本检测
 */
static void __adm_ocm_dealWithConfigNeedUpdate(void)
{
    if (NULL == g_updateCfgCb || NULL == g_updateEcuCacheCb) {
        log_e("g_updateCfgCb || g_updateEcuCacheCb is nil");
        return;
    }

    g_bIsUpdateConfig = TRUE;
    //!< 配置更新成功之后，执行更新零件缓存信息
    //!< 更新配置信息
    if (ADM_ECODE_SUCCESSED == g_updateCfgCb(NULL)) {
        //!< 需要更新配置信息，需要更新零件缓存信息
        g_bIsUpdateConfig = FALSE;
        g_updateEcuCacheCb();
    } else {
        log_e("g_updateCfgCb failed");
        g_bIsUpdateConfig = FALSE;
    }

    return;
}

#define ADM_OCM_MAX_STATUS_CODE_NUMBER      16      //!< 最大的状态处理个数
/**
 * @brief 定义状态处理的数组，用于处理状态码
 * 
 */
static ADM_OCM_STATUS_CODE_INFO_T g_statusCodeInfos[16] = {
    {ADM_OCS_AIR_STATUS_CODE_SUCCESSED,                 (DBOOL)FALSE, NULL},
    {ADM_OCS_AIR_STATUS_CODE_NO_NEW_VERSION,            (DBOOL)FALSE, NULL},
    {ADM_OCS_AIR_STATUS_CODE_VEHICLE_NOT_EXIST,         (DBOOL)FALSE, __adm_ocm_dealWithVehicleNotExist},
    {ADM_OCS_AIR_STATUS_CODE_OTA_DISABLED,              (DBOOL)FALSE, NULL},
    {ADM_OCS_AIR_STATUS_CODE_CONFIG_NEED_UPDATE,        (DBOOL)FALSE, __adm_ocm_dealWithConfigNeedUpdate},
    {ADM_OCS_AIR_STATUS_CODE_CONFIG_NOT_EXIST,          (DBOOL)FALSE, __adm_ocm_dealWithConfigDoesNotExist},
    {ADM_OCS_AIR_STATUS_CODE_PARAM_JSON_FORMAT_FAILED,  (DBOOL)FALSE, NULL},
    {ADM_OCS_AIR_STATUS_CODE_PARAM_VERIFY_FAILED,       (DBOOL)FALSE, NULL},
    {ADM_OCS_AIR_STATUS_CODE_SIGN_VERIFY_FAILED,        (DBOOL)FALSE, NULL},
    {ADM_OCS_AIR_STATUS_CODE_FILE_SAVED_FAILED,         (DBOOL)FALSE, NULL},
    {ADM_OCS_AIR_STATUS_CODE_DECRYPT_VERIFY_FAILED,     (DBOOL)FALSE, NULL},
    {ADM_OCS_AIR_STATUS_CODE_UPGRADE_TASK_STOPED,       (DBOOL)FALSE, NULL},
    {ADM_OCS_AIR_STATUS_CODE_HAVE_UPGRADE_TASK,         (DBOOL)FALSE, NULL},
    {ADM_OCS_AIR_STATUS_CODE_NO_UPGRADE_TASK,           (DBOOL)FALSE, NULL},
    {ADM_OCS_AIR_STATUS_CODE_SERVER_INTERNAL_EXCEPTION, (DBOOL)FALSE, NULL},
    {ADM_OCS_AIR_STATUS_CODE_NO_SHORT_TIME_ACCESS,      (DBOOL)FALSE, NULL}
};

/**
 * @brief 判断状态码是否在处理中
 * 
 * @param statusCode    [IN]    云端响应的状态码
 * @return DBOOL        TRUE: 需要处理， FALSE: 不需要处理
 */
static DBOOL __adm_ocm_isStatusCodeNeedToDeal(const D32S statusCode)
{
    D32S i = 0;
    DBOOL isNeedDeal = FALSE;

    __dealStatusLock();
    for (i = 0; i < ADM_OCM_MAX_STATUS_CODE_NUMBER; i++) {
        //!< 对应的statusCode，不在处理中且处理函数不为NULL
        if (g_statusCodeInfos[i].statusCode == statusCode && FALSE == g_statusCodeInfos[i].isDoing && NULL != g_statusCodeInfos[i].dealCb) {
            isNeedDeal = TRUE;
            break;
        }
    }
    __dealStatusUnlock();

    return isNeedDeal;
}

/**
 * @brief 根据状态码，获取处理的接口
 * 
 * @param statusCode        [IN]        状态码
 * @return ADM_OCM_DEAL_STATUSCODE_CALLBACK_F       NULL: 未定义, None-NULL: 处理接口
 */
static ADM_OCM_DEAL_STATUSCODE_CALLBACK_F __adm_ocm_getStatusCodeCb(const D32S statusCode)
{
    D32S i = 0;
    static ADM_OCM_DEAL_STATUSCODE_CALLBACK_F dealCb = NULL;

    __dealStatusLock();
    for (i = 0; i < ADM_OCM_MAX_STATUS_CODE_NUMBER; i++) {
        if (g_statusCodeInfos[i].statusCode == statusCode && NULL != g_statusCodeInfos[i].dealCb) {
            dealCb = g_statusCodeInfos[i].dealCb;
            break;
        }
    }
    __dealStatusUnlock();

    return dealCb;
}

/**
 * @brief 更新此状态码是否在处理中
 * 
 * @param statusCode        [IN]        状态码
 * @param isDealing     [IN]    TRUE: 处理中，FALSE: 未处理中
 */
static void __adm_ocm_updateIsDealing(const D32S statusCode, const DBOOL isDealing)
{
    D32S i = 0;

    __dealStatusLock();
    for (i = 0; i < ADM_OCM_MAX_STATUS_CODE_NUMBER; i++) {
        if (g_statusCodeInfos[i].statusCode == statusCode) {
            g_statusCodeInfos[i].isDoing = isDealing;
            break;
        }
    }
    __dealStatusUnlock();

    return;
}

/**
 * @brief 异步接口，用于处理云端返回的状态码
 * 
 * @param arg   [IN]    云端返回的状态码信息
 * @return void*    NULL
 */
static void* __adm_ocm_dealWithcloudResponseStatusCb(void* arg)
{
    ADM_OCM_MY_PARAM_T* param = (ADM_OCM_MY_PARAM_T*)arg;
    D32S statusCode = 0;
    ADM_OCM_DEAL_STATUSCODE_CALLBACK_F dealCb = NULL;
    
    do {
        statusCode = param->statusCode;
        log_i("Cloud response status code:%d - %s", statusCode, adm_ocs_getErrMessage(statusCode));
        
        //!< 获取状态处理接口
        dealCb = __adm_ocm_getStatusCodeCb(statusCode);
        if (NULL == dealCb) {
            log_w("statusCode is %d, do not need to deal with", statusCode);
            break;
        }

        //!< 处理状态码
        __adm_ocm_updateIsDealing(statusCode, TRUE);
        dealCb();
        __adm_ocm_updateIsDealing(statusCode, FALSE);

    } while(FALSE);

    if (NULL != param) {
        adm_free(param);
    }

    return NULL;
}

/**
 * @brief 定义接口用于处理云端返回的状态码信息
 * @note 此接口必须是异步接口
 * @warning: 此判断不会记录正在处理中的状态码，只会记录上一个正在处理中的状态码。
 * TODO: 考虑是否需要增加statusCodeList， 如果list中已经在处理，就不再处理此code;
 * 遗留问题：
 *  1. 目前只能判断上一个正在处理中的statusCode；
 *  2. 如果是 再前一个 正在处理的stausCode，与此次响应的statusCode相同的话，无法判断。会再次创建线程处理；
 * 处理：
 *  1. 后续可能需要增加 正在处理的statusCode List；
 *  2. 并且需要针对每个statusCode 增加一个状态；
 *    - 例如：包括正在处理中、处理完成、IDLE等；
 * 
 * DONE: 
 *  1. 增加了状态处理的结构体数组；
 *  2. 结构体数组中记录每一个状态码处理的信息；
 *  3. 判断当前结构体数组中的状态码对应的信息，处于未处理状态、且处理函数不为NULL的情况，创建线程处理
 *  4. 已经在处理的状态码，不需要再执行处理；
 * 
 * @param statusCode [IN]   云端返回的状态码信息
 */
static void __adm_ocm_dealWithcloudResponseStatus(const D32S statusCode)
{
    D32S statusCodeInProcess = 0;    //!< 记录正在处理中的状态码
    pthread_t pth = 0;
    ADM_OCM_MY_PARAM_T* param = NULL;

    do {
        //!< 判断此状态是否需要处理
        if (FALSE == __adm_ocm_isStatusCodeNeedToDeal(statusCode)) {
            log_i("statusCode:%d, do not need to deal with", statusCode);
            break;
        }

        //!< 申请空间
        param = (ADM_OCM_MY_PARAM_T*)adm_malloc(sizeof(ADM_OCM_MY_PARAM_T));
        if (NULL == param) {
            log_e("adm_malloc failed");
            break;
        }

        //!< 创建线程处理此状态码
        param->statusCode = statusCode;
        if (0 != pthread_create(&pth, NULL, __adm_ocm_dealWithcloudResponseStatusCb, (void*)param)) {    
            log_e("pthread_create failed");
            break;
        }

        //!< 线程分离
        (void)pthread_detach(pth);
        log_i("pthread_create successed");
        
    } while(FALSE);

    return;
}

D32S ocm_cdn_certSplice(D32U uiServerType)
{
    const DCHAR* cfgCdnDir = DNULL;
    const DCHAR* cfgCdnPath = DNULL;

    if (ADM_EMS_SERVER_TYPE_DEFAULT_2 == uiServerType)
    {
        cfgCdnDir = adm_cfg_iniParserGetString(ADM_CFG_VUS_CDN_CERT_DIR_2, ADM_CFG_VUS_CDN_CERT_DIR_DEFAULT_VAL_2);
        cfgCdnPath = adm_cfg_iniParserGetString(ADM_CFG_VUS_CDN_CERT_PATH_2, ADM_CFG_VUS_CDN_CERT_PATH_2);
    }
    else
    {
        cfgCdnDir = adm_cfg_iniParserGetString(ADM_CFG_VUS_CDN_CERT_DIR, ADM_CFG_VUS_CDN_CERT_DIR_DEFAULT_VAL);
        cfgCdnPath = adm_cfg_iniParserGetString(ADM_CFG_VUS_CDN_CERT_PATH, ADM_CFG_VUS_CDN_CERT_PATH);
    }

    return adm_com_filesSplice(cfgCdnDir, cfgCdnPath);
}

/**
 * @brief 车云业务通信的初始化
 * 
 * @param updateCfgCb   [IN]        更新ota缓存的平台配置信息，从平台重新获取一次配置信息
 * @param kvInitCb      [IN]        kvstorage数据库初始化接口
 * @param otaFirstBlCb  [IN]        注册和获取配置的初始化接口
 * @param updateEcuCacheCb [IN]     更新ECU缓存信息的接口
 * @param stateDelCb [IN]           状态机清除的接口
 * @param logInitCb     [IN]        日志初始化的接口
 * @param iServerType[in]           服务器类型标识：1: ini文件中的默认服务器配置; 2: ini文件中的备用服务器配置   
 * @return D32S         0:Successed, None-0:Failed
 */
D32S adm_ocm_init(ADM_OCM_UPDATE_OTACONFIG_CACHE_INFO_CALLBACK_F updateCfgCb, 
                        ADM_OCM_APC_KVSTORAGE_INIT_CALLBACK_F kvInitCb,
                        ADM_OCM_APC_OTA_FIRST_BLOOD_CALLBACK_F otaFirstBlCb,
                        ADM_OCM_UPDATE_ECU_CACHE_INFO_CALLBACK_F updateEcuCacheCb,
                        ADM_OCM_VSM_STATE_DEL_CALLBACK_F stateDelCb,
                        ADM_OCM_APC_LOG_INIT_CALLBACK_F logInitCb,
                        D32U uiServerType)
{
	D32S iRet = ADM_ECODE_SUCCESSED;
    DCHAR* strUsename = NULL;
    DCHAR* strPasswd = NULL;
	D32S protocolType = 0;

    if (ADM_EMS_SERVER_TYPE_DEFAULT_2 == uiServerType)
    {
        protocolType = adm_cfg_iniParserGetInt(ADM_CFG_VUS_OTA_SRV_BUSINESS_PROTOCOL_TYPE_2,ADM_CFG_VUS_OTA_SRV_BUSINESS_PROTOCOL_TYPE_DEFAULT_VAL_2);
    }
    else
    {
        protocolType = adm_cfg_iniParserGetInt(ADM_CFG_VUS_OTA_SRV_BUSINESS_PROTOCOL_TYPE,ADM_CFG_VUS_OTA_SRV_BUSINESS_PROTOCOL_TYPE_DEFAULT_VAL);
    }

    //!< 初始化处理状态码需要的接口
    g_updateCfgCb = updateCfgCb;
    g_kvInitCb = kvInitCb;
    g_otaFirstBlCb = otaFirstBlCb;
    g_updateEcuCacheCb = updateEcuCacheCb;
    g_stateDelCb = stateDelCb;
    g_logInitCb = logInitCb;
    g_uiServerType = uiServerType;

    //!< 初始化处理云端响应状态码的接口
    iRet = adm_ocs_init(__adm_ocm_dealWithcloudResponseStatus);
    if (ADM_ECODE_SUCCESSED != iRet) {
        log_e("adm_ocs_init failed, [err:%d]", iRet);
    } else {
        log_i("adm_ocs_init successed");
    }

	return iRet;
}

/**
 * @brief 车云业务通信接口的释放
 * 
 * @param void     [IN]    void
 * @return D32S    0:Successed, None-0:Failed
 */
D32S adm_ocm_destroy(void)
{
    //!< 初始化处理状态码需要的接口
    g_updateCfgCb = NULL;
    g_kvInitCb = NULL;
    g_otaFirstBlCb = NULL;
    g_updateEcuCacheCb = NULL;
    g_stateDelCb = NULL;
    g_logInitCb = NULL;

    return 0;
}

/**
 * @brief 通过车云通信协议中的接口ID标识，获取Topic
 * 
 * @param eBusinessType     [IN]    车云通信协议中的接口ID标识
 * @return DCHAR*        NULL:topicName获取失败，None:待请求的topicName
 */
static DCHAR* adm_ocm_getCommTopicName(const ADM_OCS_BUSINESS_TYPE_E eBusinessType)
{
	 DCHAR* topicName = NULL;
	 DCHAR* deviceId = NULL;

	 DCHAR topicNameStrBuf[128] = {0};

	 const DCHAR* vin = adm_cfg_iniParserGetString(ADM_CFG_VUS_DEF_VEHICLE_IDEN,ADM_CFG_VUS_DEF_VEHICLE_IDEN_DEFAULT_VAL);
	 
	//!< 根据车云通信协议中定义的标识，获取不同的TopicName
    switch (eBusinessType)
    {
    case ADM_OCS_BUSINESS_TYPE_REGISTER:
		(void)adm_memset(topicNameStrBuf, 0x00, sizeof(topicNameStrBuf), sizeof(topicNameStrBuf) * sizeof(DCHAR));
		(void)adm_snprintf(topicNameStrBuf,sizeof(topicNameStrBuf), "/ota/business/register/up/%s",vin);
        break;
    case ADM_OCS_BUSINESS_TYPE_GET_CFG:       
    case ADM_OCS_BUSINESS_TYPE_REPORT_VEHICLE_INFO:       
    case ADM_OCS_BUSINESS_TYPE_CHECK_VER:      
    case ADM_OCS_BUSINESS_TYPE_REPORT_VEHICLE_STATE:     
    case ADM_OCS_BUSINESS_TYPE_REPORT_ECU_STATE:     
    case ADM_OCS_BUSINESS_TYPE_REPORT_PROGRESS:    
    case ADM_OCS_BUSINESS_TYPE_REPORT_FILE:     
    case ADM_OCS_BUSINESS_TYPE_REPORT_EVENT:
		 deviceId = __adm_ocm_getDeviceId();
		 if(deviceId == NULL)
		 {
		 	log_e("deviceId is null\r\n");
			return topicName;
		 }
		(void)adm_memset(topicNameStrBuf, 0x00, sizeof(topicNameStrBuf), sizeof(topicNameStrBuf) * sizeof(DCHAR));
		(void)adm_snprintf(topicNameStrBuf,sizeof(topicNameStrBuf), "/ota/business/up/%s",deviceId);
        break;        
    case ADM_OCS_BUSINESS_TYPE_INVALID:
    default:
        log_w("eBusinessType:%d is invalid", eBusinessType);
        break;
    }

	topicName = adm_strdup(topicNameStrBuf);
	
	if(NULL != deviceId)
	{
		adm_free(deviceId);
	}
	 
	return topicName;
}

static D32S adm_ocm_setHttpAuthCertInfoForBusiness(ADM_OCA_COMMON_INIT_HANDLER_T* commHandler,DBOOL isMulAuthen)
{
    D32S iRet = ADM_ECODE_FAILED;
    ADM_OCA_CONNECT_PROTOCOL_TYPE_E authenType= ADM_OCA_CONNECT_PROTOCOL_TYPE_HTTPS_ONE_AUTH; 
    const DCHAR* certPath = NULL;
    const DCHAR* clientCert = NULL;
    const DCHAR* clientPrivateKey = NULL;

    do {
        if (NULL == commHandler) {
            iRet = ADM_ECODE_INVALID_PARAM;
            log_e("Input param commHandler is nil");
            break;
        }

        if (ADM_EMS_SERVER_TYPE_DEFAULT_2 == g_uiServerType)
        {
            certPath = adm_cfg_iniParserGetString(ADM_CFG_VUS_COMM_CERT_PATH_2, ADM_CFG_VUS_COMM_CERT_PATH_DEFAULT_VAL_2);
        }
        else
        {
            certPath = adm_cfg_iniParserGetString(ADM_CFG_VUS_COMM_CERT_PATH, ADM_CFG_VUS_COMM_CERT_PATH_DEFAULT_VAL);
        }
        if (NULL == certPath) {
            log_e("adm_cfg_iniParserGetString %s failed", ADM_CFG_VUS_COMM_CERT_PATH);
            break;
        }

        /** 默认单向认证,如果开关开启则需双向认证,客户端需提供自己的证书和私钥 */
        if(TRUE == isMulAuthen)
        {
            authenType = ADM_OCA_CONNECT_PROTOCOL_TYPE_HTTPS_MUL_AUTH; 
            if (ADM_EMS_SERVER_TYPE_DEFAULT_2 == g_uiServerType)
            {
                clientCert = adm_cfg_iniParserGetString(ADM_CFG_VUS_CLIENT_CERT_PATH_2,ADM_CFG_VUS_CLI_CERT_PATH_DEFAULT_VAL_2);
                clientPrivateKey = adm_cfg_iniParserGetString(ADM_CFG_VUS_CLIENT_PRIVATE_KEY_PATH_2,ADM_CFG_VUS_CLI_PRIVATE_KEY_PATH_DEFAULT_VAL_2);
            }
            else
            {
                clientCert = adm_cfg_iniParserGetString(ADM_CFG_VUS_CLIENT_CERT_PATH,ADM_CFG_VUS_CLI_CERT_PATH_DEFAULT_VAL);
                clientPrivateKey = adm_cfg_iniParserGetString(ADM_CFG_VUS_CLIENT_PRIVATE_KEY_PATH,ADM_CFG_VUS_CLI_PRIVATE_KEY_PATH_DEFAULT_VAL);
            }
        }

        iRet = adm_oca_setConnectProtocolType(commHandler, authenType);
        if (ADM_ECODE_SUCCESSED != iRet) {
            log_e("adm_oca_setConnectProtocolType %d failed, [err:%d]", authenType, iRet);
            break;
        }

        iRet = adm_oca_setConnectCertPath(commHandler, certPath, clientCert, clientPrivateKey);
        if (ADM_ECODE_SUCCESSED != iRet) {
            log_e("adm_oca_setConnectCertPath [%s][%s][%s] failed, [err:%d]", certPath, clientCert,clientPrivateKey,iRet);
            break;
        }
    } while(FALSE);

    return iRet;
}

static D32S adm_ocm_setProtocolAndCertInfoForBusiness(ADM_OCA_COMMON_INIT_HANDLER_T* commHandler)
{
    D32S protocolType = 0;
    D32S iRet = ADM_ECODE_FAILED;
    DBOOL isMulAuthen = FALSE;
    
    if (NULL == commHandler) {
        iRet = ADM_ECODE_INVALID_PARAM;
        log_e("Input param commHandler is nil");
        return iRet;
    }

    if (ADM_EMS_SERVER_TYPE_DEFAULT_2 == g_uiServerType)
    {
        protocolType = adm_cfg_iniParserGetInt(ADM_CFG_VUS_OTA_SRV_BUSINESS_PROTOCOL_TYPE_2, ADM_CFG_VUS_OTA_SRV_BUSINESS_PROTOCOL_TYPE_DEFAULT_VAL_2);
        isMulAuthen = adm_cfg_iniParserGetBoolean(ADM_CFG_VUS_OTA_SRV_SSL_VERIFY_PEER_2, (DBOOL)ADM_CFG_VUS_OTA_SRV_ALLOW_MUL_AUTHEN_DEFAULT_VAL_2);

    }
    else
    {
        protocolType = adm_cfg_iniParserGetInt(ADM_CFG_VUS_OTA_SRV_BUSINESS_PROTOCOL_TYPE, ADM_CFG_VUS_OTA_SRV_BUSINESS_PROTOCOL_TYPE_DEFAULT_VAL);
        isMulAuthen = adm_cfg_iniParserGetBoolean(ADM_CFG_VUS_OTA_SRV_SSL_VERIFY_PEER, (DBOOL)ADM_CFG_VUS_OTA_SRV_ALLOW_MUL_AUTHEN_DEFAULT_VAL);
    }

    log_i("protocolType is %d", protocolType);
    switch (protocolType)
    {
    case 1:  // HTTP
        //!< 填充通信使用的数据，包括：URL、通信类型、通信证书等所有信息
	    iRet = adm_oca_setConnectProtocolType(commHandler, ADM_OCA_CONNECT_PROTOCOL_TYPE_HTTP);
        break;
    case 2: // HTTPS
        iRet = adm_ocm_setHttpAuthCertInfoForBusiness(commHandler,isMulAuthen);
        break;
    case 3:
    case 4:
    default:
        break;
    }

    return iRet;

}

/**
 * @brief 车云业务通信的接口，包含：注册、检测、获取配置、事件上报、状态上报、进度上报等
 * @note rspMessageBody 使用后需释放空间
 * 
 * @param eBusinessType     [IN]    车云通信协议中的接口ID标识
 * @param reqMessageBody    [IN]    请求平台的数据
 * @param rspMessageBody    [OUT]   平台响应的数据
 * @param rspStatusCode     [OUT]   平台响应的状态码 
 * @return D32S             0:Successed, None-0:Failed(根据不同的状态码区分消息)
 */
D32S adm_ocm_businessCommInterface(const ADM_OCS_BUSINESS_TYPE_E eBusinessType, const DCHAR* reqMessageBody, DCHAR** rspMessageBody, DSIZE* rspStatusCode)
{
    D32S iRet = ADM_ECODE_FAILED;
    DCHAR* url = NULL;
    ADM_OCA_COMMON_INIT_HANDLER_T* commHandler = NULL;
    D32S protocolType = 0;

    __businessLock();
    do {
        //!< 校验入参是否为空
        if (NULL == reqMessageBody) {
            iRet = ADM_ECODE_INVALID_PARAM;
            log_e("Input param reqMessageBody is nil, err:%d", iRet);
            break;
        }

        //!< 初始化与OTA平台通信的句柄
        commHandler = adm_oca_connectInit();
        if (NULL == commHandler) {
            iRet = ADM_ECODE_APPLY_MEMORY_FAILED;
            log_e("adm_oca_connectInit failed, reason of error code is %d", iRet);
            break;
        }

        if (ADM_EMS_SERVER_TYPE_DEFAULT_2 == g_uiServerType)
        {
            protocolType = adm_cfg_iniParserGetInt(ADM_CFG_VUS_OTA_SRV_BUSINESS_PROTOCOL_TYPE_2,ADM_CFG_VUS_OTA_SRV_BUSINESS_PROTOCOL_TYPE_DEFAULT_VAL_2);
        }
        else
        {
            protocolType = adm_cfg_iniParserGetInt(ADM_CFG_VUS_OTA_SRV_BUSINESS_PROTOCOL_TYPE,ADM_CFG_VUS_OTA_SRV_BUSINESS_PROTOCOL_TYPE_DEFAULT_VAL);
        }

        //!< 根据交互业务类型，获取请求的URL地址
        url = adm_ocm_getCommUrl(eBusinessType);
        if (NULL == url) {
            iRet = ADM_ECODE_URL_MALFORMAT;
            log_e("adm_ocm_getCommUrl failed, err:%d", iRet);
            break;
        }

        log_i("url:%s", url);

        //!< 填充通信使用的数据，包括：URL、通信类型、通信证书等所有信息
        (void)adm_ocm_setProtocolAndCertInfoForBusiness(commHandler);
        (void)adm_oca_setConnectAddress(commHandler, ADM_OCA_CONNECT_ADDRESS_TYPE_URL, url);

        iRet = adm_oca_requestConnectServer(commHandler, reqMessageBody, rspMessageBody, rspStatusCode);
        if (ADM_ECODE_SUCCESSED != iRet) {
            log_e("adm_oca_requestConnectServer failed, err:%d", iRet);
            break;
        } else {
            log_i("adm_oca_requestConnectServer successed");
        }
    } while(FALSE);

    //!< 空间释放
    if (NULL != url) {
        adm_free(url);
    }
    adm_oca_connectDeinit(commHandler);
    __businessUnlock();

    return iRet;
}

static D32S adm_ocm_setProtocolAndCertInfoForDownload(ADM_OCA_COMMON_INIT_HANDLER_T* commHandler)
{
    static DBOOL s_bisCdnCertInit = DFALSE;
    D32S protocolType = 0;
    D32S iRet = ADM_ECODE_FAILED;
    DBOOL isMulAuthen = FALSE;
    ADM_OCA_CONNECT_PROTOCOL_TYPE_E authenType = ADM_OCA_CONNECT_PROTOCOL_TYPE_HTTPS_ONE_AUTH;
    const DCHAR* certPath = NULL;
    const DCHAR* clientCert = NULL;
    const DCHAR* clientPrivateKey = NULL;

    if (NULL == commHandler) {
        iRet = ADM_ECODE_INVALID_PARAM;
        log_e("Input param commHandler is nil");
        return iRet;
    }

    if (DFALSE == s_bisCdnCertInit)
    {
        iRet = ocm_cdn_certSplice(ADM_EMS_SERVER_TYPE_DEFAULT);
        if (ADM_IS_SUCCESS(iRet))
        {
            s_bisCdnCertInit = DTRUE;
            log_i("ocm_cdn_certSplice successed");
        }
        else
        {
            log_e("ocm_cdn_certSplice faild [err:%d]", iRet);
        }
        usleep(500000);
    }

    if (ADM_EMS_SERVER_TYPE_DEFAULT_2 == g_uiServerType)
    {
       protocolType = adm_cfg_iniParserGetInt(ADM_CFG_VUS_OTA_SRV_DOWNLOAD_PROTOCOL_TYPE_2, ADM_CFG_VUS_OTA_SRV_DOWNLOAD_PROTOCOL_TYPE_DEFAULT_VAL_2);
        isMulAuthen = adm_cfg_iniParserGetBoolean(ADM_CFG_VUS_CDN_SRV_SSL_VERIFY_PEER_2, (DBOOL)ADM_CFG_VUS_CDN_SRV_ALLOW_MUL_AUTHEN_DEFAULT_VAL_2); 
    }
    else
    {
        protocolType = adm_cfg_iniParserGetInt(ADM_CFG_VUS_OTA_SRV_DOWNLOAD_PROTOCOL_TYPE, ADM_CFG_VUS_OTA_SRV_DOWNLOAD_PROTOCOL_TYPE_DEFAULT_VAL);
        isMulAuthen = adm_cfg_iniParserGetBoolean(ADM_CFG_VUS_CDN_SRV_SSL_VERIFY_PEER, (DBOOL)ADM_CFG_VUS_CDN_SRV_ALLOW_MUL_AUTHEN_DEFAULT_VAL);
    }

    log_i("protocolType is %d", protocolType);
    switch (protocolType)
    {
    case 1:  // HTTP
        //!< 填充通信使用的数据，包括：URL、通信类型、通信证书等所有信息
	    iRet = adm_oca_setConnectProtocolType(commHandler, ADM_OCA_CONNECT_PROTOCOL_TYPE_HTTP);
        break;
    case 2: // HTTPS
        /** 开启双向认证,客户端需提供自己的证书和私钥 */
        if(TRUE == isMulAuthen)
        {
            authenType = ADM_OCA_CONNECT_PROTOCOL_TYPE_HTTPS_MUL_AUTH;
            if (ADM_EMS_SERVER_TYPE_DEFAULT_2 == g_uiServerType)
            {
                clientCert = adm_cfg_iniParserGetString(ADM_CFG_VUS_CDN_CLIENT_CERT_PATH_2,ADM_CFG_VUS_CLI_CERT_PATH_DEFAULT_VAL_2);
                clientPrivateKey = adm_cfg_iniParserGetString(ADM_CFG_VUS_CDN_CLIENT_PRIVATE_KEY_PATH_2,ADM_CFG_VUS_CLI_PRIVATE_KEY_PATH_DEFAULT_VAL_2);
            }
            else
            {
                clientCert = adm_cfg_iniParserGetString(ADM_CFG_VUS_CDN_CLIENT_CERT_PATH,ADM_CFG_VUS_CLI_CDN_CERT_PATH_DEFAULT_VAL);
            	clientPrivateKey = adm_cfg_iniParserGetString(ADM_CFG_VUS_CDN_CLIENT_PRIVATE_KEY_PATH,ADM_CFG_VUS_CLI_CDN_PRIVATE_KEY_PATH_DEFAULT_VAL);
            }
        }
        if (ADM_EMS_SERVER_TYPE_DEFAULT_2 == g_uiServerType)
        {
            certPath = adm_cfg_iniParserGetString(ADM_CFG_VUS_CDN_CERT_PATH_2, ADM_CFG_VUS_CDN_CERT_PATH_DEFAULT_VAL_2);
        }
        else
        {
            certPath = adm_cfg_iniParserGetString(ADM_CFG_VUS_CDN_CERT_PATH, ADM_CFG_VUS_CDN_CERT_PATH_DEFAULT_VAL);
        }
        iRet = adm_oca_setConnectProtocolType(commHandler, authenType);
        
        iRet = adm_oca_setConnectCertPath(commHandler, certPath, clientCert, clientPrivateKey); 
        break;
    case 3:
    case 4:
    default:
        break;
    }

    return iRet;

}

/**
 * @brief 通过此接口注册给到core service执行文件下载
 * 
 * @param downloadUrl       [IN]    待下载文件的URL
 * @param pstDownloadInfo   [IN]    文件下载需要的信息
 * @return D32S             0:Successed, None-0:Failed(根据不同的状态码区分消息)
 */
D32S adm_ocm_fileDownloadInterface(const DCHAR* downloadUrl, ADM_OCA_FILE_DOWNLOAD_INFO_T1* pstDownloadInfo)
{
    D32S iRet = ADM_ECODE_FAILED;
    DCHAR *pcProtocolPrefix = NULL;
    ADM_OCA_COMMON_INIT_HANDLER_T* commHandler = NULL;

    __downloadLock();
    do {
        //!< 入参校验
        if (NULL == downloadUrl) {
            iRet = ADM_ECODE_INVALID_PARAM;
            log_e("Input param downloadUrl is nil, err:%d", iRet);
            break;
        }

        log_i("downloadUrl:%s", downloadUrl);

        //!< 入参校验
        if (NULL == pstDownloadInfo) {
            iRet = ADM_ECODE_INVALID_PARAM;
            log_e("Input param pstDownload is nil, err:%d", iRet);
            break;
        }

        //!< 入参校验
        if (NULL == pstDownloadInfo->savePath) {
            iRet = ADM_ECODE_INVALID_PARAM;
            log_e("Input param pstDownloadInfo savePath is nil, err:%d", iRet);
            break;
        }

        //!< 通信句柄初始化
        commHandler = adm_oca_connectInit();
        if (NULL == commHandler) {
            iRet = ADM_ECODE_INVALID_POINTER;
            log_e("adm_oca_connectInit failed, err:%d", iRet);
            break;
        }

        //!< 设置下载参数
#ifdef ADM_DEBUG_DPM_MODULE
        //!< 模拟，使用的是 http 模拟数据  
        pcProtocolPrefix = adm_strstr(downloadUrl, "http:");
        if (NULL != pcProtocolPrefix)
        {
            (void)adm_oca_setConnectProtocolType(commHandler, ADM_OCA_CONNECT_PROTOCOL_TYPE_HTTP);
        }
        else
        {
            (void)adm_ocm_setProtocolAndCertInfoForDownload(commHandler);
        }
#else
        //!< 实际，根据配置信息设置对应的协议及证书
        (void)adm_ocm_setProtocolAndCertInfoForDownload(commHandler);
#endif
        (void)adm_oca_setConnectAddress(commHandler, ADM_OCA_CONNECT_ADDRESS_TYPE_URL, downloadUrl);

        //!< 执行文件下载
        iRet = adm_oca_startDownload(commHandler, pstDownloadInfo, DTRUE);
        if (ADM_ECODE_SUCCESSED != iRet) {
            log_e("adm_oca_startDownload failed, err:%d", iRet);
            break;
        } else {
            log_i("adm_oca_startDownload successed");
        }

    } while(FALSE);

    adm_oca_connectDeinit(commHandler);
    __downloadUnlock();

    return iRet;
}

#define ADM_OCM_FILE_UPLOAD_INFO_EXTS       "exts"
#define ADM_OCM_FILE_UPLOAD_INFO_EXT_KEY    "extKey"
#define ADM_OCM_FILE_UPLOAD_INFO_EXT_VAL    "extValue"

/**
 * @brief 文件上传设置扩展参数，扩展参数为Json List 字符串
 * @note
 *  "exts":[
 *      {
 *          "extKey":"fileData",
 *          "extValue":"xxxxxxxxxxxxxxxxxxxxxx"
 *      },
 *      {
 *          "extKey":"fileSign",
 *          "extValue":"xxxxxxxxxxxxxxxxxxxxxx"
 *      },
 *      {
 *          "extKey":"fileSecret",
 *          "extValue":"xxxxxxxxxxxxxxxxxxxxxx"
 *      }
 *  ]
 * 
 * @param uploadHandler [IN]    设置formdata需要的句柄
 * @param extsField     [IN]    可转换为Json List的字符串数据
 * @return D32S         0:Successed, None-0:Failed
 */
static D32S __adm_ocm_setUploadInfoExtsFieldToFormdata(ADM_OCA_FILE_UPLOAD_HANDLE_T* uploadHandler, const DCHAR* extsField)
{
    D32S iRet = ADM_ECODE_FAILED;
    json_object* rootJsonObj = NULL;
    json_object* extsJsonObj = NULL;
    json_object* arrJsonObj = NULL;
    json_object* extJsonObj = NULL;
    const DCHAR* extKey = NULL;
    const DCHAR* extVal = NULL;
    D16U i = 0;
    DBOOL isExists = FALSE;
    DCHAR sectionName[32] = {0};

    do {
        //!< 入参校验
        if (NULL == uploadHandler) {
            iRet = ADM_ECODE_INVALID_PARAM;
            log_e("Input param uploadHandler is nil, err:%d", iRet);
            break;
        }

        //!< 入参检验，如果扩展字段 extsField 为空，说明没有扩展字段需要填充，默认返回成功
        if (NULL == extsField) {
            iRet = ADM_ECODE_SUCCESSED;
            log_i("Input param extsField is ni, the extended field is optional, so return successed");
            break;
        }

        //!< Json 字符串转换为Json 对象数据
        rootJsonObj = json_tokener_parse(extsField);
        if (NULL == rootJsonObj) {
            iRet = ADM_ECODE_STRING_TO_JSON_FAILED;
            log_e("extsField:%s string to json failed, err:%d", iRet);
            break;
        }

        //!< 判断数组字段 exts 是否存在, 如果存在，数组结构数据写入extsJsonObj
        isExists = json_object_object_get_ex(rootJsonObj, ADM_OCM_FILE_UPLOAD_INFO_EXTS, &extsJsonObj);
        if (FALSE == isExists) {
            iRet = ADM_ECODE_JSON_KEY_FIELD_IS_NOT_EXIST;
            log_e("rootJsonObject field:[%s] is not exist, err:%d", iRet);
            break;
        }

        //!< 判断extsJsonObj 是否为array类型
        if (json_type_array != json_object_get_type(extsJsonObj)) {
            iRet = ADM_ECODE_JSON_OBJECT_TYPE_NOT_ARRAY;
            log_e("extsJsonObj is not json array, err:%d", iRet);
            break;
        }

        //!< 循环解析json数组
        for (i = 0; i < json_object_array_length(extsJsonObj); i++) {
            arrJsonObj = json_object_array_get_idx(extsJsonObj, i);
            if (NULL == arrJsonObj) {
                log_w("json_object_array_get_idx %d is nil", i);
                continue;
            }

            //!< 获取 extKey对象
            extJsonObj = json_object_object_get(arrJsonObj, ADM_OCM_FILE_UPLOAD_INFO_EXT_KEY);
            if (NULL == extJsonObj) {
                log_w("json_object_object_get [extKey] failed");
                continue;
            }
            extKey = json_object_get_string(extJsonObj);
            if (NULL == extKey) {
                log_w("json_object_get_string [extKey] failed");
                continue;
            }

            //!< 获取 extValue对象
            extJsonObj = json_object_object_get(arrJsonObj, ADM_OCM_FILE_UPLOAD_INFO_EXT_VAL);
            if (NULL == extJsonObj) {
                log_w("json_object_object_get [extValue] failed");
                continue;
            }
            extVal = json_object_get_string(extJsonObj);
            if (NULL == extVal) {
                log_w("json_object_get_string [extVal] failed");
                continue;
            }

            //!< 添加formdata，sectionName = exts[i].extKey; section = extKey
            (void)adm_memset(sectionName, 0x00, sizeof(sectionName), sizeof(sectionName) * sizeof(DCHAR));
            (void)adm_snprintf(sectionName, sizeof(sectionName), "exts[%d].%s", i, ADM_OCM_FILE_UPLOAD_INFO_EXT_KEY);
            if (ADM_ECODE_SUCCESSED != adm_oca_addSectionToHttpPost(uploadHandler, sectionName, extKey, ADM_OCA_HTTP_POST_SECTION_TYPE_COPYCONTENTS)) {
                log_w("adm_oca_addSectionToHttpPost sectionName:%s, section:%s failed", sectionName, extKey);
                continue;
            }

            //!< 添加formdata，sectionName = exts[i].extValue; section = extVal
            (void)adm_memset(sectionName, 0x00, sizeof(sectionName), sizeof(sectionName) * sizeof(DCHAR));
            (void)adm_snprintf(sectionName, sizeof(sectionName), "exts[%d].%s", i, ADM_OCM_FILE_UPLOAD_INFO_EXT_VAL);
            if (ADM_ECODE_SUCCESSED != adm_oca_addSectionToHttpPost(uploadHandler, sectionName, extVal, ADM_OCA_HTTP_POST_SECTION_TYPE_COPYCONTENTS)) {
                log_w("adm_oca_addSectionToHttpPost sectionName:%s, section:%s failed", sectionName, extVal);
                continue;
            }
        }
    } while(FALSE);

    //!< 释放json对象
    if (NULL != rootJsonObj) {
        (void)json_object_put(rootJsonObj);
    }

    return iRet;
}

/**
 * @brief 根据车云协议定义的字段，填充上传的section信息
 * 
 * @param commHandler   [IN]    上传信息句柄
 * @param pstUploadInfo [IN]    上传需要的section信息
 * @return D32S         0:Successed, None-0:Failed
 */
static D32S __adm_ocm_setUploadInfoToFormdata(ADM_OCA_FILE_UPLOAD_HANDLE_T* uploadHandler, 
                                              ADM_OCS_UPLOADFILE_T* pstUploadInfo,
                                              ADM_OCS_UPLOAD_URL_INFO_T *pstUploadUrlInfo)
{
    D32S iRet = ADM_ECODE_FAILED;
    DCHAR sectionVal[32] = {0};

    do {
        //!< 入参校验
        if (NULL == uploadHandler) {
            iRet = ADM_ECODE_INVALID_PARAM;
            log_e("Input param uploadHandler is nil, err:%d", iRet);
            break;
        }

        if (NULL == pstUploadInfo) {
            iRet = ADM_ECODE_INVALID_PARAM;
            log_e("Input param pstUploadInfo is nil, err:%d", iRet);
            break;
        }

        //TODO: Sign字段缺失、非必填字段缺失
        //!< 必填字段校验
        if (NULL == pstUploadInfo->sdkVersion || NULL == pstUploadInfo->fullFileNamePath || NULL == pstUploadInfo->fileHash) {
            iRet = ADM_ECODE_INVALID_PARAM;
            log_e("Input param sdkVersion | fullFileNamePath || fileHash is nil, err:%d", iRet);
            break;
        }

        //!< sessionId
        if (NULL != pstUploadInfo->sessionId) {
            iRet = adm_oca_addSectionToHttpPost(uploadHandler, "sessionId", pstUploadInfo->sessionId, ADM_OCA_HTTP_POST_SECTION_TYPE_COPYCONTENTS);
            if (ADM_ECODE_SUCCESSED != iRet) {
                log_e("adm_oca_addSectionToHttpPost sessionId:%s is failed, err:%d", pstUploadInfo->sessionId, iRet);
            }
        }
        //!< taskId
        if (0 != pstUploadInfo->taskId) {
            (void)adm_memset(sectionVal, 0x00, sizeof(sectionVal), sizeof(sectionVal) * sizeof(DCHAR));
            (void)adm_snprintf(sectionVal, sizeof(sectionVal), "%lld", pstUploadInfo->taskId);
            iRet = adm_oca_addSectionToHttpPost(uploadHandler, "taskId", sectionVal, ADM_OCA_HTTP_POST_SECTION_TYPE_COPYCONTENTS);
            if (ADM_ECODE_SUCCESSED != iRet) {
                log_e("adm_oca_addSectionToHttpPost taskId:%lld is failed, err:%d", pstUploadInfo->taskId, iRet);
            }
        }
        //!< strategyId
        if (0 != pstUploadInfo->strategyId) {
            (void)adm_memset(sectionVal, 0x00, sizeof(sectionVal), sizeof(sectionVal) * sizeof(DCHAR));
            (void)adm_snprintf(sectionVal, sizeof(sectionVal), "%lld", pstUploadInfo->strategyId);
            iRet = adm_oca_addSectionToHttpPost(uploadHandler, "strategyId", sectionVal, ADM_OCA_HTTP_POST_SECTION_TYPE_COPYCONTENTS);
            if (ADM_ECODE_SUCCESSED != iRet) {
                log_e("adm_oca_addSectionToHttpPost strategyId:%lld is failed, err:%d", pstUploadInfo->strategyId, iRet);
            }
        }

        //!< timestamp
        (void)adm_memset(sectionVal, 0x00, sizeof(sectionVal), sizeof(sectionVal) * sizeof(DCHAR));
        (void)adm_snprintf(sectionVal, sizeof(sectionVal), "%lld", pstUploadInfo->timestamp);
        iRet = adm_oca_addSectionToHttpPost(uploadHandler, "timestamp", sectionVal, ADM_OCA_HTTP_POST_SECTION_TYPE_COPYCONTENTS);
        if (ADM_ECODE_SUCCESSED != iRet) {
            log_e("adm_oca_addSectionToHttpPost timestamp:%lld is failed, err:%d", pstUploadInfo->timestamp, iRet);
            break;
        }

        //!< startTime
        if (pstUploadInfo->startTime != (D64S)0) {
            (void)adm_memset(sectionVal, 0x00, sizeof(sectionVal), sizeof(sectionVal) * sizeof(DCHAR));
            (void)adm_snprintf(sectionVal, sizeof(sectionVal), "%lld", pstUploadInfo->startTime);
            iRet = adm_oca_addSectionToHttpPost(uploadHandler, "startTime", sectionVal, ADM_OCA_HTTP_POST_SECTION_TYPE_COPYCONTENTS);
            if (ADM_ECODE_SUCCESSED != iRet) {
                log_e("adm_oca_addSectionToHttpPost startTime:%lld is failed, err:%d", pstUploadInfo->startTime, iRet);
                break;
            }
        }

        //!< endTime
        if (pstUploadInfo->endTime != (D64S)0) {
            (void)adm_memset(sectionVal, 0x00, sizeof(sectionVal), sizeof(sectionVal) * sizeof(DCHAR));
            (void)adm_snprintf(sectionVal, sizeof(sectionVal), "%lld", pstUploadInfo->endTime);
            iRet = adm_oca_addSectionToHttpPost(uploadHandler, "endTime", sectionVal, ADM_OCA_HTTP_POST_SECTION_TYPE_COPYCONTENTS);
            if (ADM_ECODE_SUCCESSED != iRet) {
                log_e("adm_oca_addSectionToHttpPost endTime:%lld is failed, err:%d", pstUploadInfo->endTime, iRet);
                break;
            }
        }

        //!< sdkVersion
        iRet = adm_oca_addSectionToHttpPost(uploadHandler, "sdkVersion", pstUploadInfo->sdkVersion, ADM_OCA_HTTP_POST_SECTION_TYPE_COPYCONTENTS);
        if (ADM_ECODE_SUCCESSED != iRet) {
            log_e("adm_oca_addSectionToHttpPost sdkVersion:%s is failed, err:%d", pstUploadInfo->sdkVersion, iRet);
            break;
        }

        if (2 == pstUploadUrlInfo->iUploadType)
        {
            //!< filePath
            iRet = adm_oca_addSectionToHttpPost(uploadHandler, "filePath", pstUploadUrlInfo->pcFilePath, ADM_OCA_HTTP_POST_SECTION_TYPE_COPYCONTENTS);
            if (ADM_ECODE_SUCCESSED != iRet) {
                log_e("adm_oca_addSectionToHttpPost filePath:%s is failed, err:%d", pstUploadUrlInfo->pcFilePath, iRet);
                break;
            }

            iRet = adm_irs_calcFileUploadSign(pstUploadInfo, pstUploadUrlInfo);
            if (ADM_ECODE_SUCCESSED != iRet) {
                log_e("adm_irs_calcFileUploadSign failed, err:%d", iRet);
                break;
            }
        }

        //!< sign
        if (NULL != pstUploadInfo->sign) {
            iRet = adm_oca_addSectionToHttpPost(uploadHandler, "sign", pstUploadInfo->sign, ADM_OCA_HTTP_POST_SECTION_TYPE_COPYCONTENTS);
            if (ADM_ECODE_SUCCESSED != iRet) {
                log_e("adm_oca_addSectionToHttpPost sign:%s is failed, err:%d", pstUploadInfo->sign, iRet);
            }
        }

        //!< exts
        //!< 循环填充exts的list信息
        //!< sample  sectionName: exts[0].extKey, section:xxxx
        //!< sample  sectionName: exts[0].extValue, section:xxxx
        //!< sample  sectionName: exts[1].extKey, section:xxxx
        //!< sample  sectionName: exts[1].extValue, section:xxxx
        if (NULL != pstUploadInfo->extsReqJsonStr) {
            iRet = __adm_ocm_setUploadInfoExtsFieldToFormdata(uploadHandler, pstUploadInfo->extsReqJsonStr);
            if (ADM_ECODE_SUCCESSED != iRet) {
                log_e("__adm_ocm_setUploadInfoExtsFieldToFormdata failed, err:%d", iRet);
            }
        }
        
        //!< cmdId
        (void)adm_memset(sectionVal, 0x00, sizeof(sectionVal), sizeof(sectionVal) * sizeof(DCHAR));
        (void)adm_snprintf(sectionVal, sizeof(sectionVal), "%d", pstUploadInfo->cmdId);
        iRet = adm_oca_addSectionToHttpPost(uploadHandler, "cmdId", sectionVal, ADM_OCA_HTTP_POST_SECTION_TYPE_COPYCONTENTS);
        if (ADM_ECODE_SUCCESSED != iRet) {
            log_e("adm_oca_addSectionToHttpPost cmdId:%d is failed, err:%d", pstUploadInfo->cmdId, iRet);
            break;
        }

        //!< type
        (void)adm_memset(sectionVal, 0x00, sizeof(sectionVal), sizeof(sectionVal) * sizeof(DCHAR));
        (void)adm_snprintf(sectionVal, sizeof(sectionVal), "%d", pstUploadInfo->fileType);
        iRet = adm_oca_addSectionToHttpPost(uploadHandler, "type", sectionVal, ADM_OCA_HTTP_POST_SECTION_TYPE_COPYCONTENTS);
        if (ADM_ECODE_SUCCESSED != iRet) {
            log_e("adm_oca_addSectionToHttpPost type:%d is failed, err:%d", pstUploadInfo->fileType, iRet);
            break;
        }
        //!< pushId
        if (NULL != pstUploadInfo->pushId) {
            iRet = adm_oca_addSectionToHttpPost(uploadHandler, "pushId", pstUploadInfo->pushId, ADM_OCA_HTTP_POST_SECTION_TYPE_COPYCONTENTS);
            if (ADM_ECODE_SUCCESSED != iRet) {
                log_e("adm_oca_addSectionToHttpPost pushId:%s is failed, err:%d", pstUploadInfo->pushId, iRet);
            }
        }

        if (1 == pstUploadUrlInfo->iUploadType)
        {
            //!< logFile
            iRet = adm_oca_addSectionToHttpPost(uploadHandler, "logFile", pstUploadInfo->fullFileNamePath, ADM_OCA_HTTP_POST_SECTION_TYPE_FILE);
            if (ADM_ECODE_SUCCESSED != iRet) {
                log_e("adm_oca_addSectionToHttpPost logFile:%s is failed, err:%d", pstUploadInfo->fullFileNamePath, iRet);
                break;
            }
        }

        //!< isLogFileEncrypted
        (void)adm_memset(sectionVal, 0x00, sizeof(sectionVal), sizeof(sectionVal) * sizeof(DCHAR));
        (void)adm_snprintf(sectionVal, sizeof(sectionVal), "%d", pstUploadInfo->isFileEncrypted);
        iRet = adm_oca_addSectionToHttpPost(uploadHandler, "isLogFileEncrypted", sectionVal, ADM_OCA_HTTP_POST_SECTION_TYPE_COPYCONTENTS);
        if (ADM_ECODE_SUCCESSED != iRet) {
            log_e("adm_oca_addSectionToHttpPost isFileEncrypted:%d is failed, err:%d", pstUploadInfo->isFileEncrypted, iRet);
            break;
        }

        //!< logFileSize
        (void)adm_memset(sectionVal, 0x00, sizeof(sectionVal), sizeof(sectionVal) * sizeof(DCHAR));
        (void)adm_snprintf(sectionVal, sizeof(sectionVal), "%lld", pstUploadInfo->fileSize);
        iRet = adm_oca_addSectionToHttpPost(uploadHandler, "logFileSize", sectionVal, ADM_OCA_HTTP_POST_SECTION_TYPE_COPYCONTENTS);
        if (ADM_ECODE_SUCCESSED != iRet) {
            log_e("adm_oca_addSectionToHttpPost fileSize:%lld is failed, err:%d", pstUploadInfo->fileSize, iRet);
            break;
        }
        //!< logFileHash
        iRet = adm_oca_addSectionToHttpPost(uploadHandler, "logFileHash", pstUploadInfo->fileHash, ADM_OCA_HTTP_POST_SECTION_TYPE_COPYCONTENTS);
        if (ADM_ECODE_SUCCESSED != iRet) {
            log_e("adm_oca_addSectionToHttpPost logFileHash:%s is failed, err:%d", pstUploadInfo->fileHash, iRet);
            break;
        }
    } while(FALSE);

    return iRet;
}

static D32S __adm_ocm_setProtocolAndCertInfoForUpload(ADM_OCA_COMMON_INIT_HANDLER_T* commHandler)
{
    D32S protocolType = 0;
    D32S iRet = ADM_ECODE_FAILED;
    DBOOL isMulAuthen = FALSE;
    ADM_OCA_CONNECT_PROTOCOL_TYPE_E authenType = ADM_OCA_CONNECT_PROTOCOL_TYPE_HTTPS_ONE_AUTH;
    const DCHAR* certPath = NULL;
    const DCHAR* clientCert = NULL;
    const DCHAR* clientPrivateKey = NULL;

    if (NULL == commHandler) {
        iRet = ADM_ECODE_INVALID_PARAM;
        log_e("Input param commHandler is nil");
        return iRet;
    }

    if (ADM_EMS_SERVER_TYPE_DEFAULT_2 == g_uiServerType)
    {
        protocolType = adm_cfg_iniParserGetInt(ADM_CFG_VUS_OTA_SRV_UPLOAD_PROTOCOL_TYPE_2, ADM_CFG_VUS_OTA_SRV_UPLOAD_PROTOCOL_TYPE_DEFAULT_VAL_2);
        isMulAuthen = adm_cfg_iniParserGetBoolean(ADM_CFG_VUS_OTA_SRV_SSL_VERIFY_PEER_2, (DBOOL)ADM_CFG_VUS_OTA_SRV_ALLOW_MUL_AUTHEN_DEFAULT_VAL_2);
    }
    else
    {
        protocolType = adm_cfg_iniParserGetInt(ADM_CFG_VUS_OTA_SRV_UPLOAD_PROTOCOL_TYPE, ADM_CFG_VUS_OTA_SRV_UPLOAD_PROTOCOL_TYPE_DEFAULT_VAL);
        isMulAuthen = adm_cfg_iniParserGetBoolean(ADM_CFG_VUS_OTA_SRV_SSL_VERIFY_PEER, (DBOOL)ADM_CFG_VUS_OTA_SRV_ALLOW_MUL_AUTHEN_DEFAULT_VAL);
    }

    log_i("protocolType is %d", protocolType);
    switch (protocolType)
    {
    case 1:  // HTTP
        //!< 填充通信使用的数据，包括：URL、通信类型、通信证书等所有信息
	    iRet = adm_oca_setConnectProtocolType(commHandler, ADM_OCA_CONNECT_PROTOCOL_TYPE_HTTP);
        break;
    case 2: // HTTPS
        /** 开启双向认证,客户端需提供自己的证书和私钥 */
        if(TRUE == isMulAuthen)
        {
            authenType = ADM_OCA_CONNECT_PROTOCOL_TYPE_HTTPS_MUL_AUTH;

            if (ADM_EMS_SERVER_TYPE_DEFAULT_2 == g_uiServerType)
            {
                clientCert = adm_cfg_iniParserGetString(ADM_CFG_VUS_CLIENT_CERT_PATH_2,ADM_CFG_VUS_CLI_CERT_PATH_DEFAULT_VAL_2);
                clientPrivateKey = adm_cfg_iniParserGetString(ADM_CFG_VUS_CLIENT_PRIVATE_KEY_PATH_2,ADM_CFG_VUS_CLI_PRIVATE_KEY_PATH_DEFAULT_VAL_2);
            }
            else
            {
                clientCert = adm_cfg_iniParserGetString(ADM_CFG_VUS_CLIENT_CERT_PATH,ADM_CFG_VUS_CLI_CERT_PATH_DEFAULT_VAL);
                clientPrivateKey = adm_cfg_iniParserGetString(ADM_CFG_VUS_CLIENT_PRIVATE_KEY_PATH,ADM_CFG_VUS_CLI_PRIVATE_KEY_PATH_DEFAULT_VAL);
            }
        }
        if (ADM_EMS_SERVER_TYPE_DEFAULT_2 == g_uiServerType)
        {
            certPath = adm_cfg_iniParserGetString(ADM_CFG_VUS_COMM_CERT_PATH_2, ADM_CFG_VUS_COMM_CERT_PATH_DEFAULT_VAL_2);
        }
        else
        {
            certPath = adm_cfg_iniParserGetString(ADM_CFG_VUS_COMM_CERT_PATH, ADM_CFG_VUS_COMM_CERT_PATH_DEFAULT_VAL);
        }
        iRet = adm_oca_setConnectProtocolType(commHandler, authenType);
        iRet = adm_oca_setConnectCertPath(commHandler, certPath, clientCert, clientPrivateKey);
        break;
    case 3:
    case 4:
    default:
        break;
    }

    return iRet;

}

/**
 * @brief 通过此接口注册给到core service执行文件上传
 * 
 * @param uploadInfo        [IN]    文件上传需要的信息
 * @param rspMessageBody    [OUT]   平台响应的状态码
 * @param priv              [IN/OUT]    请求云端上传文件需要的参数
 * @return D32S             0:Successed, None-0:Failed
 */
D32S adm_ocm_fileUploadInterface(ADM_OCA_FILE_UPLOAD_INFO_T* uploadInfo, DCHAR** rspMessageBody, ADM_OCS_UPLOADFILE_T* pstUploadInfo)
{
    D32S iRet = ADM_ECODE_FAILED;
    DCHAR* uploadUrl = NULL;
    ADM_OCA_COMMON_INIT_HANDLER_T* commHandler = NULL;
    ADM_OCA_FILE_UPLOAD_HANDLE_T* uploadHandler = NULL;
    ADM_OCS_UPLOAD_URL_INFO_T stUploadUrlInfo = {0};
    const DCHAR *cfgCdnPath = NULL;
    DCHAR *pCfgCdnPath = NULL;

    __uploadLock();

    do 
    {
        //!< 入参校验
        if (NULL == uploadInfo) {
            iRet = ADM_ECODE_INVALID_PARAM;
            log_w("Input param upload is nil, do not need to focus on progress, err:%d", iRet);
            // break;
        }

        iRet = adm_irs_requestUploadUrl(time(NULL), 
                                        ADM_SDK_VERSION, 
                                        NULL, 
                                        adm_ocm_businessCommInterface, 
                                        &stUploadUrlInfo);
        if (ADM_ECODE_SUCCESSED != iRet) 
        {
            log_w("adm_irs_requestUploadUrl failed, err:%d", iRet);
            stUploadUrlInfo.iUploadType = 1;        ///< 默认上传云端
            // break;
        }

        if (2 == stUploadUrlInfo.iUploadType)
        {
            iRet = ocm_cdn_certSplice(ADM_EMS_SERVER_TYPE_DEFAULT);
            if (ADM_ECODE_SUCCESSED != iRet)
            {
                log_e("ocm_cdn_certSplice failed, err:%d", iRet);
                break;
            }

            cfgCdnPath = adm_cfg_iniParserGetString(ADM_CFG_VUS_CDN_CERT_PATH, ADM_CFG_VUS_CDN_CERT_PATH_DEFAULT_VAL);
            pCfgCdnPath = adm_strdup(cfgCdnPath);
            iRet = adm_oca_startPutFile(pCfgCdnPath, &stUploadUrlInfo, pstUploadInfo);
            adm_free(pCfgCdnPath);
        }

        /** 上报OTA平台，或者上报华为OBS并且上报成功 */
        if ((1 == stUploadUrlInfo.iUploadType) ||
            (2 == stUploadUrlInfo.iUploadType && ADM_ECODE_SUCCESSED == iRet))
        {
            //!< 通信句柄初始化
            commHandler = adm_oca_connectInit();
            if (NULL == commHandler)
            {
                iRet = ADM_ECODE_APPLY_MEMORY_FAILED;
                log_e("adm_oca_connectInit failed, err:%d", iRet);
                break;
            }

            //!< 文件上传的句柄初始化
            uploadHandler = adm_oca_fileUploadInit();
            if (NULL == uploadHandler)
            {
                iRet = ADM_ECODE_APPLY_MEMORY_FAILED;
                log_e("adm_oca_fileUploadInit failed, err:%d", iRet);
                break;
            }

            uploadUrl = adm_ocm_getCommUrl(ADM_OCS_BUSINESS_TYPE_REPORT_FILE);
            if (NULL == uploadUrl)
            {
                iRet = ADM_ECODE_URL_MALFORMAT;
                log_e("adm_ocm_getCommUrl failed, err:%d", iRet);
                break;
            }

            log_i("uploadUrl:%s", uploadUrl);

            //!< 设置文件上传需要的参数
            (void)__adm_ocm_setProtocolAndCertInfoForUpload(commHandler);
            (void)adm_oca_setConnectAddress(commHandler, ADM_OCA_CONNECT_ADDRESS_TYPE_URL, uploadUrl);

            //!< 上传文件填充表单信息
            iRet = __adm_ocm_setUploadInfoToFormdata(uploadHandler, pstUploadInfo, &stUploadUrlInfo);
            if (ADM_ECODE_SUCCESSED != iRet)
            {
                log_e("__adm_ocm_setUploadInfoToFormdata failed, err:%d", iRet);
                break;
            }

            //!< 请求执行文件上传
            iRet = adm_oca_startUpload(commHandler, uploadHandler, uploadInfo, rspMessageBody);
            if (ADM_ECODE_SUCCESSED != iRet)
            {
                log_e("adm_oca_startUpload failed, err:%d", iRet);
                break;
            }
            else
            {
                log_i("adm_oca_startUpload successed");
            }
        }
    } while(FALSE);

    if (NULL != uploadUrl) 
    {
        adm_free(uploadUrl);
    }

    adm_oca_fileUploadDeinit(uploadHandler);
    adm_oca_connectDeinit(commHandler);

    adm_free(stUploadUrlInfo.pcUploadUrl);
    adm_free(stUploadUrlInfo.pcFilePath);

    __uploadUnlock();

    return iRet;
}

#if 0
#endif

/**
 * @brief 设置UC-Lite从UC-Master下载文件的协议类型和证书信息
 * 
 * @param commHandler       [IN]    文件下载的句柄
 * @return D32S             0:Successed, None-0:Failed
 */
static D32S __adm_ocm_ewebDownloadProtocolAndCertSet(ADM_OCA_COMMON_INIT_HANDLER_T* commHandler)
{
    D32S protocolType = 0;
    D32S iRet = ADM_ECODE_FAILED;
    DBOOL isMulAuthen = FALSE;
    ADM_OCA_CONNECT_PROTOCOL_TYPE_E authenType = ADM_OCA_CONNECT_PROTOCOL_TYPE_HTTPS_ONE_AUTH;
    const DCHAR* certPath = NULL;
    const DCHAR* clientCert = NULL;
    const DCHAR* clientPrivateKey = NULL;

    do {
        if (NULL == commHandler) {
            iRet = ADM_ECODE_INVALID_PARAM;
            log_e("Input param commhandler is nil");
            break;
        }

        //!< 修改为新的协议类型
        protocolType = adm_cfg_iniParserGetInt(ADM_CFG_EWEB_FILE_DOWN_PROTOCOL_TYPE, ADM_CFG_EWEB_FILE_DOWN_PROTOCOL_TYPE_DEFAULT_VAL);
        isMulAuthen = adm_cfg_iniParserGetBoolean(ADM_CFG_EWEB_SSL_VERIFY_PEER, (DBOOL)ADM_CFG_EWEB_SSL_VERIFY_PEER_DEFAULT_VAL);

        log_i("protocolType is %d", protocolType);

        switch (protocolType)
        {
        case 1:  // HTTP
            //!< 填充通信使用的数据，包括：URL、通信类型、通信证书等所有信息
            iRet = adm_oca_setConnectProtocolType(commHandler, ADM_OCA_CONNECT_PROTOCOL_TYPE_HTTP);
            break;
        case 2: ///<HTTPS 
            /** 开启双向认证,客户端需提供自己的证书和私钥,eweb证书和私钥一起 */
            if(TRUE == isMulAuthen)
            {
                authenType = ADM_OCA_CONNECT_PROTOCOL_TYPE_HTTPS_MUL_AUTH;
                clientCert = adm_cfg_iniParserGetString(ADM_CFG_EWEB_CLIENT_CERT_PATH,ADM_CFG_EWEB_SSL_CLI_CERT_PATH_DEFAULT_VAL);
                clientPrivateKey = adm_cfg_iniParserGetString(ADM_CFG_EWEB_CLIENT_PRIVATE_KEY_PATH,ADM_CFG_EWEB_SSL_CLI_PRI_KEY_PATH_DEFAULT_VAL);
            }
            certPath = adm_cfg_iniParserGetString(ADM_CFG_EWEB_SSL_CA_PATH, ADM_CFG_EWEB_SSL_CERT_PATH_DEFAULT_VAL);
            iRet = adm_oca_setConnectProtocolType(commHandler, authenType);
            iRet = adm_oca_setConnectCertPath(commHandler, certPath, clientCert, clientPrivateKey);
            break;
        case 3:
        case 4:
        default:
            break;
        }

    } while(FALSE);
    
    return iRet;
}

static D32S __adm_ocm_ewebUploadProtocolAndCertSet(ADM_OCA_COMMON_INIT_HANDLER_T* commHandler)
{
    D32S protocolType = 0;
    D32S iRet = ADM_ECODE_FAILED;
    DBOOL isMulAuthen = FALSE;
    ADM_OCA_CONNECT_PROTOCOL_TYPE_E authenType = ADM_OCA_CONNECT_PROTOCOL_TYPE_HTTPS_ONE_AUTH;
    const DCHAR* certPath = NULL;
    const DCHAR* clientCert = NULL;      
    const DCHAR* clientPrivateKey = NULL; 

    do {
        if (NULL == commHandler) {
            iRet = ADM_ECODE_INVALID_PARAM;
            log_e("Input param commhandler is nil");
            break;
        }

        //!< 修改为新的协议类型
        protocolType = adm_cfg_iniParserGetInt(ADM_CFG_EWEB_FILE_UPLOAD_PROTOCOL_TYPE, ADM_CFG_EWEB_FILE_UPLOAD_PROTOCOL_TYPE_DEFAULT_VAL);
        isMulAuthen = adm_cfg_iniParserGetBoolean(ADM_CFG_EWEB_SSL_VERIFY_PEER, (DBOOL)ADM_CFG_EWEB_SSL_VERIFY_PEER_DEFAULT_VAL);
        
        log_i("protocolType is %d", protocolType);

        switch (protocolType)
        {
        case 1:  // HTTP
            //!< 填充通信使用的数据，包括：URL、通信类型、通信证书等所有信息
            iRet = adm_oca_setConnectProtocolType(commHandler, ADM_OCA_CONNECT_PROTOCOL_TYPE_HTTP);
            break;
        case 2: // HTTPS
            /** 开启双向认证,客户端需提供自己的证书和私钥 */
            if(TRUE == isMulAuthen)
            {
                authenType = ADM_OCA_CONNECT_PROTOCOL_TYPE_HTTPS_MUL_AUTH;
                clientCert = adm_cfg_iniParserGetString(ADM_CFG_EWEB_CLIENT_CERT_PATH,ADM_CFG_EWEB_SSL_CLI_CERT_PATH_DEFAULT_VAL);
                clientPrivateKey = adm_cfg_iniParserGetString(ADM_CFG_EWEB_CLIENT_PRIVATE_KEY_PATH,ADM_CFG_EWEB_SSL_CLI_PRI_KEY_PATH_DEFAULT_VAL);
            }
            certPath = adm_cfg_iniParserGetString(ADM_CFG_EWEB_SSL_CA_PATH, ADM_CFG_EWEB_SSL_CERT_PATH_DEFAULT_VAL);
            iRet = adm_oca_setConnectProtocolType(commHandler, authenType);
            iRet = adm_oca_setConnectCertPath(commHandler, certPath, clientCert, clientPrivateKey);
            break;
        case 3:
        case 4:
        default:
            break;
        }

    } while(FALSE);
    
    return iRet;
}

/**
 * @brief 通过此接口注册给到core service 执行文件下载(用于UC-Lite从UC-Master下载文件)
 * 
 * @param downloadUrl       [IN]    待下载文件的URL
 * @param pstDownloadInfo   [IN]    文件下载需要的信息
 * @return D32S             0:Successed, None-0:Failed(根据不同的状态码区分消息)
 */
D32S adm_ocm_ewebDownloadFileInterface(const DCHAR* downloadUrl, ADM_OCA_FILE_DOWNLOAD_INFO_T1* pstDownloadInfo)
{
    D32S iRet = ADM_ECODE_FAILED;
    DCHAR *pcProtocolPrefix = NULL;
    ADM_OCA_COMMON_INIT_HANDLER_T* commHandler = NULL;

    __downloadLock();
    do {
        //!< 入参校验
        if (NULL == downloadUrl) {
            iRet = ADM_ECODE_INVALID_PARAM;
            log_e("Input param downloadUrl is nil, err:%d", iRet);
            break;
        }

        log_i("downloadUrl:%s", downloadUrl);

        //!< 入参校验
        if (NULL == pstDownloadInfo) {
            iRet = ADM_ECODE_INVALID_PARAM;
            log_e("Input param pstDownload is nil, err:%d", iRet);
            break;
        }

        //!< 入参校验
        if (NULL == pstDownloadInfo->savePath) {
            iRet = ADM_ECODE_INVALID_PARAM;
            log_e("Input param pstDownloadInfo savePath is nil, err:%d", iRet);
            break;
        }

        //!< 通信句柄初始化
        commHandler = adm_oca_connectInit();
        if (NULL == commHandler) {
            iRet = ADM_ECODE_INVALID_POINTER;
            log_e("adm_oca_connectInit failed, err:%d", iRet);
            break;
        }

        //!< 设置下载参数
#ifdef ADM_DEBUG_DPM_MODULE
        //!< 模拟，使用的是 http 模拟数据  
        pcProtocolPrefix = adm_strstr(downloadUrl, "http:");
        if (NULL != pcProtocolPrefix)
        {
            (void)adm_oca_setConnectProtocolType(commHandler, ADM_OCA_CONNECT_PROTOCOL_TYPE_HTTP);
        }
        else
        {
            (void)__adm_ocm_ewebDownloadProtocolAndCertSet(commHandler);
        }
#else
        //!< 实际，根据配置信息设置对应的协议及证书
        (void)__adm_ocm_ewebDownloadProtocolAndCertSet(commHandler);
#endif
        (void)adm_oca_setConnectAddress(commHandler, ADM_OCA_CONNECT_ADDRESS_TYPE_URL, downloadUrl);

        //!< 执行文件下载
        iRet = adm_oca_startDownload(commHandler, pstDownloadInfo, DFALSE);
        if (ADM_ECODE_SUCCESSED != iRet) {
            log_e("adm_oca_startDownload failed, err:%d", iRet);
            break;
        } else {
            log_i("adm_oca_startDownload successed");
        }

    } while(FALSE);

    adm_oca_connectDeinit(commHandler);
    __downloadUnlock();

    return iRet;
}

/**
 * @brief 检查参数合法性
 * 
 * @param pstUploadInfo     [IN]    文件上传需要的信息
 * @return D32S             0:Successed, None-0:Failed
 */
static D32S __adm_ocm_ewebCheckUploadFileParam(ADM_OCA_SIMPLE_FILEUPLOAD_INFO_T1 *pstUploadInfo)
{
    //!< 入参校验
    if (ADM_IS_NULL(pstUploadInfo)) 
    {
        log_e("Input param upload is nil, err:%d", ADM_ECODE_INVALID_PARAM);
        return ADM_ECODE_INVALID_PARAM;
    }
    
    if (ADM_IS_NULL(pstUploadInfo->url)) 
    {
        log_e("Input param upload url is nil, err:%d", ADM_ECODE_INVALID_PARAM);
        return ADM_ECODE_INVALID_PARAM;
    }
    if (ADM_IS_NULL(pstUploadInfo->fileName)) 
    {
        log_e("Input param upload fileName is nil, err:%d", ADM_ECODE_INVALID_PARAM);
        return ADM_ECODE_INVALID_PARAM;
    }
    if (ADM_IS_NULL(pstUploadInfo->filePath)) 
    {
        log_e("Input param upload filePath is nil, err:%d", ADM_ECODE_INVALID_PARAM);
        return ADM_ECODE_INVALID_PARAM;
    }
    if (ADM_IS_NULL(pstUploadInfo->fileHash)) 
    {
        log_e("Input param upload fileHash is nil, err:%d", ADM_ECODE_INVALID_PARAM);
        return ADM_ECODE_INVALID_PARAM;
    }
    if (0 >= pstUploadInfo->fileSize) 
    {
        log_e("Input param upload fileSize is invalied(%d), err:%d", pstUploadInfo->fileSize, ADM_ECODE_INVALID_PARAM);
        return ADM_ECODE_INVALID_PARAM;
    }
    
    return ADM_ECODE_SUCCESSED;
}


/**
 * @brief 填充ewebServer服务器 fromdata信息
 * 
 * @param commHandler   [IN]    上传信息句柄
 * @param pstUploadInfo [IN]    上传需要的section信息
 * @return D32S         0:Successed, None-0:Failed
 */
static D32S __adm_ocm_ewebFillUploadInfoToFormdata(ADM_OCA_FILE_UPLOAD_HANDLE_T* uploadHandler, ADM_OCA_SIMPLE_FILEUPLOAD_INFO_T1 *pstUploadInfo)
{
    D32S iRet = ADM_ECODE_FAILED;
    DCHAR sectionVal[32] = {0};

    do {
        //!< 入参校验
        if (NULL == uploadHandler) {
            iRet = ADM_ECODE_INVALID_PARAM;
            log_e("Input param uploadHandler is nil, err:%d", iRet);
            break;
        }

        if (NULL == pstUploadInfo) {
            iRet = ADM_ECODE_INVALID_PARAM;
            log_e("Input param pstUploadInfo is nil, err:%d", iRet);
            break;
        }

        //!< fileName
        iRet = adm_oca_addSectionToHttpPost(uploadHandler, "filename", pstUploadInfo->fileName, ADM_OCA_HTTP_POST_SECTION_TYPE_COPYCONTENTS);
        if (ADM_ECODE_SUCCESSED != iRet) {
            log_e("adm_oca_addSectionToHttpPost fileName:%s is failed, err:%d", pstUploadInfo->fileName, iRet);
            break;
        }

        //!< timestamp
        (void)adm_memzero(sectionVal, sizeof(sectionVal));
        (void)adm_snprintf(sectionVal, sizeof(sectionVal), "%lld", time(NULL));
        iRet = adm_oca_addSectionToHttpPost(uploadHandler, "timestamp", sectionVal, ADM_OCA_HTTP_POST_SECTION_TYPE_COPYCONTENTS);
        if (ADM_ECODE_SUCCESSED != iRet) {
            log_e("adm_oca_addSectionToHttpPost timestamp:%s is failed, err:%d", sectionVal, iRet);
            break;
        }

        //!< filePath
        iRet = adm_oca_addSectionToHttpPost(uploadHandler, "logFile", pstUploadInfo->filePath, ADM_OCA_HTTP_POST_SECTION_TYPE_FILE);
        if (ADM_ECODE_SUCCESSED != iRet) {
            log_e("adm_oca_addSectionToHttpPost filePath:%s is failed, err:%d", pstUploadInfo->filePath, iRet);
            break;
        }
        
        // //!< fileHash
        // iRet = adm_oca_addSectionToHttpPost(uploadHandler, "fileHash", pstUploadInfo->fileHash, ADM_OCA_HTTP_POST_SECTION_TYPE_COPYCONTENTS);
        // if (ADM_ECODE_SUCCESSED != iRet) {
        //     log_e("adm_oca_addSectionToHttpPost fileHash:%s is failed, err:%d", pstUploadInfo->fileHash, iRet);
        //     break;
        // }
        
        // //!< fileSize
        // (void)adm_memset(sectionVal, 0x00, sizeof(sectionVal), sizeof(sectionVal) * sizeof(DCHAR));
        // (void)adm_snprintf(sectionVal, sizeof(sectionVal), "%lld", pstUploadInfo->fileSize);
        // iRet = adm_oca_addSectionToHttpPost(uploadHandler, "logFileSize", sectionVal, ADM_OCA_HTTP_POST_SECTION_TYPE_COPYCONTENTS);
        // if (ADM_ECODE_SUCCESSED != iRet) {
        //     log_e("adm_oca_addSectionToHttpPost fileSize:%lld is failed, err:%d", pstUploadInfo->fileSize, iRet);
        //     break;
        // }
    } while(FALSE);

    return iRet;
}


/**
 * @brief 通过此接口注册给到core service执行文件上传
 * @note: 当前仅支持http协议
 *
 * @param pstUploadInfo      [IN/OUT]    请求云端上传文件需要的参数
 * @param rspMessageBody     [OUT]       平台响应的状态码
 * @return D32S              0:Successed, None-0:Failed
 */
D32S adm_ocm_ewebUploadFileInterface(ADM_OCA_SIMPLE_FILEUPLOAD_INFO_T1 *pstUploadInfo, DCHAR** rspMessageBody)
{
    D32S iRet = ADM_ECODE_FAILED;
    ADM_OCA_COMMON_INIT_HANDLER_T* commHandler = NULL;
    ADM_OCA_FILE_UPLOAD_HANDLE_T* uploadHandler = NULL;
    ADM_OCA_FILE_UPLOAD_INFO_T stUploadCallBack = {0};

    //!< 入参校验
    iRet = __adm_ocm_ewebCheckUploadFileParam(pstUploadInfo);
    if (ADM_IS_ERR(iRet))
    {
        return iRet;
    }

    __uploadLock();
    do {
        //!< 通信句柄初始化
        commHandler = adm_oca_connectInit();
        if (NULL == commHandler) {
            iRet = ADM_ECODE_APPLY_MEMORY_FAILED;
            log_e("adm_oca_connectInit failed, err:%d", iRet);
            break;
        }

        //!< 文件上传的句柄初始化
        uploadHandler = adm_oca_fileUploadInit();
        if (NULL == uploadHandler) {
            iRet = ADM_ECODE_APPLY_MEMORY_FAILED;
            log_e("adm_oca_fileUploadInit failed, err:%d", iRet);
            break;
        }

        log_i("uploadUrl[%s]", pstUploadInfo->url);

        //!< 设置文件上传需要的参数
        (void)__adm_ocm_ewebUploadProtocolAndCertSet(commHandler);
        (void)adm_oca_setConnectAddress(commHandler, ADM_OCA_CONNECT_ADDRESS_TYPE_URL, pstUploadInfo->url);

        //!< 上传文件填充表单信息
        iRet = __adm_ocm_ewebFillUploadInfoToFormdata(uploadHandler, pstUploadInfo);
        if (ADM_ECODE_SUCCESSED != iRet) {
            log_e("__adm_ocm_setUploadInfoToFormdata failed, err:%d", iRet);
            break;
        }

        stUploadCallBack.priv             = pstUploadInfo->priv;
        stUploadCallBack.cbUploadProgress = pstUploadInfo->cbUploadProgress;

        //!< 请求执行文件上传
        iRet = adm_oca_startUpload(commHandler, uploadHandler, &stUploadCallBack, rspMessageBody);
        if (ADM_ECODE_SUCCESSED != iRet) {
            log_e("adm_oca_startUpload failed, err:%d", iRet);
            break;
        } else {
            log_i("adm_oca_startUpload successed");
        }
    } while(FALSE);

    adm_oca_fileUploadDeinit(uploadHandler);
    adm_oca_connectDeinit(commHandler);
    __uploadUnlock();

    return iRet;
}

DBOOL adm_ocm_isUpdateConfig(void)
{
    return g_bIsUpdateConfig;
}

// D32S adm_ocm_setDomainName(D32S iCountryCode, D32S iBrand)
// {
//     D32S iRet = ADM_ECODE_SUCCESSED;
//     adm_free(g_pDomainName1);
//     adm_free(g_pDomainName2);

// #ifdef CLOUD_ENV_TEST       /**测试环境*/
//     iRet = ocmGetCountryDomain((ADM_COUNTRY_E)iCountryCode, (ADM_BRAND_E)iBrand, &g_pDomainName1, &g_pDomainName2);
// #else /**生产环境*/
//     iRet = ocmGetCountryDomain((ADM_COUNTRY_E)iCountryCode, (ADM_BRAND_E)iBrand, &g_pDomainName2, &g_pDomainName1);
// #endif
//     if (ADM_IS_ERR(iRet)) 
//     {
//         log_e("ocmGetCountryDomain failed, err:%d", iRet);
//     }
//     else
//     {
//         log_i("g_pDomainName1 : %s", g_pDomainName1);
//         log_i("g_pDomainName2 : %s", g_pDomainName2);
//     }

//     return iRet;
// }

