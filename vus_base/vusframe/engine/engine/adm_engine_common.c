#include "adm_engine_common.h"
#include "adm_com_kvstorage.h"
#include "adm_com_errorCode.h"
#include "adm_typedefs.h"
#include "adm_com_lib.h"
#include "adm_memory.h"
#include "adm_string.h"
#include "adm_file.h"
#include <stdio.h>

#include "adm_com_modDefine.h"

#if !defined(LOG_TAG)
#    define LOG_TAG MOD_NAME(ADM_COM_MOD_COM) /*!< 宏函数，用于通过模块名的枚举值类型获取模块名称     */
#endif

#if !defined(MOD_TAG_ID)
#    define MOD_TAG_ID ADM_COM_MOD_COM
#endif

#include "adm_debug.h"

#define ADM_ENGINE_COM_BUSNISS_DIR "business/"
#define ADM_ENGINE_COM_REPORT_DIR "report/"

#define ADM_ENGINE_COM_REPARE_TIME_MAX 3

static KvStorage_t* g_kvBusinessDatabase = NULL;  //!< 全局变量，OTA的业务数据库目录
static KvStorage_t* g_kvReportDatabase = NULL;  //!< 全局变量，OTA过程中需要上报的数据库目录

static DCHAR* g_kvStorageDir = NULL;  //!< 全局变量用于存储kv storage的目录
static DCHAR* g_logDir = NULL;  //!< 全局变量用于存储log的目录
static DCHAR* g_packageDir = NULL;  //!< 全局变量用于存储升级包的目录
static DCHAR* g_certDir = NULL;  //!< 全局变量用于存储证书的目录
static DCHAR* g_unzipDir = NULL;  //!< 全局变量用于存储解压后信息的存储目录
static DCHAR* g_defaultSecert = NULL;   //!< 全局变量，用于存储车云通信使用的默认key

static DBOOL __adm_core_service_isLastCharSlash(const DCHAR* str)
{
    DBOOL isLastCharSlashOk = FALSE;
    D16U strLen = 0;

    do {
        if (NULL == str) {
            log_e("Input param str is nil");
            break;
        }

        strLen = adm_strlen(str);
        if ('/' == str[strLen - 1]) {
            isLastCharSlashOk = TRUE;
            break;
        }
    } while (0);

    return isLastCharSlashOk;
}

static D32S __adm_core_service_kvstorageRepare(ADM_CORE_SERVICE_DIR_TYPE_E eDirType, const DCHAR* dirName)
{
    DBOOL bIsRepare = FALSE;
    D32S iRet = ADM_ECODE_FAILED;
    D32U i = 0;

    log_i("adm_com_kvstorage_open %s failed, try to repare", dirName);
    /** 运行时删除数据库或其他原因导致启动程序打开db失败，尝试修复 */
    for(i = 0; i < ADM_ENGINE_COM_REPARE_TIME_MAX; i++)
    {
        bIsRepare = adm_com_kvstorage_repare((const char*)dirName);
        if(bIsRepare == FALSE) 
        {
            iRet = ADM_ECODE_KV_OPEN_FAILED;
            log_e("time: %u, adm_com_kvstorage_repare %s failed, err:%d", i, dirName,iRet);
            continue;
        }
        
        if(eDirType == ADM_CORE_SERVICE_BUSINESS_DIR)
        {
            g_kvBusinessDatabase = adm_com_kvstorage_open((const char*)dirName);
            if (!g_kvBusinessDatabase) {
                iRet = ADM_ECODE_KV_OPEN_FAILED;
                log_e("time: %u, adm_com_kvstorage_open %s failed, err:%d", i, dirName, iRet);                    
                continue;
            }
        }
        else if(eDirType == ADM_CORE_SERVICE_REPORE_DIR)
        {
            g_kvReportDatabase = adm_com_kvstorage_open((const char*)dirName);
            if (!g_kvReportDatabase) {
                iRet = ADM_ECODE_KV_OPEN_FAILED;
                log_e("time: %u, adm_com_kvstorage_open %s failed, err:%d", i, dirName, iRet);                    
                continue;
            }
        }
        else
        {
            log_e("Invalid database type[%d]",eDirType);
            continue;
        }
        log_i("adm_com_kvstorage_repare && adm_com_kvstorage_open successful");
        iRet = ADM_ECODE_SUCCESSED;
        break;
    }
    return iRet;
}

static DBOOL __adm_core_service_OpenBusinessFailedDeal(const DCHAR* dirName)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    
    iRet = __adm_core_service_kvstorageRepare(ADM_CORE_SERVICE_BUSINESS_DIR, dirName);
    if(ADM_ECODE_SUCCESSED != iRet)
    {
        log_e("__adm_core_service_kvstorageRepare %s failed,need to delete/create dir, err:%d", dirName, iRet);
        adm_com_rmDirAndFiles(dirName);
        iRet = adm_com_createDir(dirName);
        if (ADM_ECODE_SUCCESSED != iRet) 
        {
            log_e("adm_com_createDir %s failed, err:%d", dirName, iRet);
            return DFALSE;
        }
        g_kvBusinessDatabase = adm_com_kvstorage_open((const char*)dirName);
        if (NULL == g_kvBusinessDatabase)
        {
            log_e("adm_com_kvstorage_open %s failed, err:%d", dirName, iRet);
            return DFALSE;
        }
    }
    
    return DTRUE;
}

static DBOOL __adm_core_service_OpenReportFailedDeal(const DCHAR* dirName)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    
    iRet = __adm_core_service_kvstorageRepare(ADM_CORE_SERVICE_REPORE_DIR, dirName);
    if(ADM_ECODE_SUCCESSED != iRet)
    {
        log_e("__adm_core_service_kvstorageRepare %s failed, err:%d", dirName, iRet);
        adm_com_rmDirAndFiles(dirName);
        iRet = adm_com_createDir(dirName);
        if (ADM_ECODE_SUCCESSED != iRet) 
        {
            log_e("adm_com_createDir %s failed, err:%d", dirName, iRet);
            return DFALSE;
        }
        g_kvReportDatabase = adm_com_kvstorage_open((const char*)dirName);
        if (NULL == g_kvReportDatabase)
        {
            log_e("adm_com_kvstorage_open %s failed, err:%d", dirName, iRet);
            return DFALSE;
        }
    }
    
    return DTRUE;
}


/**
 * @brief 初始化OTA过程中的kv数据库
 * @note  1. OTA的待存储数据量级较小，所以分为两个数据库
 *        2. business(业务): 注册、配置、检测、下载、安装等过程中产生的需持久存储的信息
 *        3. report(上报)：进度、结果、状态、事件等的上报，产生的需持久存储的信息
 *
 * @return D32S     0:Successed, None-0:Failed
 */
D32S adm_core_service_init(void)
{
    D32S iRet = ADM_ECODE_FAILED;
    DCHAR* kvStorageDir = NULL;
    DBOOL isDirCreateOk = FALSE;
    DCHAR tempDirName[255] = {0};
    DCHAR dirName[256] = {0};
    DBOOL isLastCharSlashOk = FALSE;

    do {
        //!< 从上层配置中获取kvstorage存储目录
        kvStorageDir = adm_core_service_getRuntimeDir(ADM_CORE_SERVICE_RUNTIME_DIR_TYPE_KV_STORAGE);
        if (NULL == kvStorageDir) {
            iRet = ADM_ECODE_INVALID_PARAM;
            log_e("adm_core_service_getRuntimeDir failed, eType:%d, err:%d",
                ADM_CORE_SERVICE_RUNTIME_DIR_TYPE_KV_STORAGE,
                iRet);
            break;
        }

        //!< 判断设置的目录中，最后一个字符是否为 '/'，如果不是需要添加最后一个字符为 '/'
        adm_memset(tempDirName, 0x00, sizeof(tempDirName), sizeof(tempDirName) * sizeof(DCHAR));
        isLastCharSlashOk = __adm_core_service_isLastCharSlash(kvStorageDir);
        if (TRUE == isLastCharSlashOk) {
            adm_snprintf(tempDirName, sizeof(tempDirName), "%s", kvStorageDir);
        } else {
            adm_snprintf(tempDirName, sizeof(tempDirName), "%s/", kvStorageDir);
        }

        //!< 创建kvstorage的具体业务目录，business
        adm_memset(dirName, 0x00, sizeof(dirName), sizeof(dirName) * sizeof(DCHAR));
        adm_snprintf(dirName, sizeof(dirName), "%s%s", tempDirName, ADM_ENGINE_COM_BUSNISS_DIR);
        iRet = adm_com_createDir(dirName);
        if (ADM_ECODE_SUCCESSED != iRet) {
            log_e("adm_com_createDir %s failed, err:%d", dirName, iRet);
            break;
        }

        //!< 初始化kvstorage business数据库目录
        if (NULL == g_kvBusinessDatabase) {
            g_kvBusinessDatabase = adm_com_kvstorage_open((const char*)dirName);
            if (!g_kvBusinessDatabase)
            {
                if(FALSE == __adm_core_service_OpenBusinessFailedDeal(dirName))
                {
                    iRet = ADM_ECODE_KV_OPEN_FAILED;
                    log_e("adm_com_kvstorage_open %s failed, err:%d", dirName, iRet);
                    break;
                }
            }
        }

        //!< 创建kvstorage的具体业务目录，report
        adm_memset(dirName, 0x00, sizeof(dirName), sizeof(dirName) * sizeof(DCHAR));
        adm_snprintf(dirName, sizeof(dirName), "%s%s", tempDirName, ADM_ENGINE_COM_REPORT_DIR);
        iRet = adm_com_createDir(dirName);
        if (ADM_ECODE_SUCCESSED != iRet) {
            log_e("adm_com_createDir %s failed, err:%d", dirName, iRet);
            break;
        }

        //!< 初始化kvstorage report数据库目录
        if (NULL == g_kvReportDatabase) {
            g_kvReportDatabase = adm_com_kvstorage_open((const char*)dirName);
            if (!g_kvReportDatabase) 
            {
                if(FALSE == __adm_core_service_OpenReportFailedDeal(dirName))
                {
                    iRet = ADM_ECODE_KV_OPEN_FAILED;
                    log_e("adm_com_kvstorage_open %s failed, err:%d", dirName, iRet);
                    break;
                }
            }
        }

        //!< kv storage 数据库初始化成功
        iRet = ADM_ECODE_SUCCESSED;
    } while (0);

    adm_free(kvStorageDir);

    return iRet;
}

/**
 * @brief 释放kvstorage数据库
 *
 */
void adm_core_service_deinit(void)
{
    adm_com_kvstorage_close(g_kvBusinessDatabase);
    g_kvBusinessDatabase = NULL;
    adm_com_kvstorage_close(g_kvReportDatabase);
    g_kvReportDatabase = NULL;
}

/**
 * @brief 删除kvstorage数据库
 * 
 */
void adm_core_service_deeplyDestroy(void)
{
    // DCHAR dirName[256] = {0};
    // DBOOL isLastCharSlashOk = FALSE;

    adm_core_service_deinit();

    // isLastCharSlashOk = __adm_core_service_isLastCharSlash(g_kvStorageDir);
    // adm_memset(dirName, 0x00, sizeof(dirName), sizeof(dirName));
    // if (TRUE == isLastCharSlashOk) {
    //     adm_snprintf(dirName, sizeof(dirName), "%s%s", g_kvStorageDir, ADM_ENGINE_COM_BUSNISS_DIR);
    // } else {
    //     adm_snprintf(dirName, sizeof(dirName), "%s/%s", g_kvStorageDir, ADM_ENGINE_COM_BUSNISS_DIR);
    // }
    adm_com_kvstorage_destroy(g_kvBusinessDatabase);


    // isLastCharSlashOk = __adm_core_service_isLastCharSlash(g_kvStorageDir);
    // adm_memset(dirName, 0x00, sizeof(dirName), sizeof(dirName));
    // if (TRUE == isLastCharSlashOk) {
    //     adm_snprintf(dirName, sizeof(dirName), "%s%s", g_kvStorageDir, ADM_ENGINE_COM_REPORT_DIR);
    // } else {
    //     adm_snprintf(dirName, sizeof(dirName), "%s/%s", g_kvStorageDir, ADM_ENGINE_COM_REPORT_DIR);
    // }
    adm_com_kvstorage_destroy(g_kvReportDatabase);
}

/**
 * @brief 删除kvstorage数据库，并重新创建
 *
 */
void adm_core_service_delAndCreatKv(void)
{
    D32S iRet = ADM_ECODE_FAILED;
    DCHAR* kvStorageDir = NULL;
    DCHAR tempDirName[ADM_COM_FILEPATH_LEN] = {0};
    DCHAR dirName[ADM_COM_FILEPATH_LEN] = {0};
    DBOOL isLastCharSlashOk = FALSE;

    adm_core_service_deinit();
    do {
        //!< 从上层配置中获取kvstorage存储目录
        kvStorageDir = adm_core_service_getRuntimeDir(ADM_CORE_SERVICE_RUNTIME_DIR_TYPE_KV_STORAGE);
        if (NULL == kvStorageDir) 
        {
            iRet = ADM_ECODE_INVALID_PARAM;
            log_e("adm_core_service_getRuntimeDir failed, eType:%d, err:%d",
                ADM_CORE_SERVICE_RUNTIME_DIR_TYPE_KV_STORAGE,
                iRet);
            break;
        }

        //!< 判断设置的目录中，最后一个字符是否为 '/'，如果不是需要添加最后一个字符为 '/'
        adm_memset(tempDirName, 0x00, sizeof(tempDirName), sizeof(tempDirName) * sizeof(DCHAR));
        isLastCharSlashOk = __adm_core_service_isLastCharSlash(kvStorageDir);
        if (TRUE == isLastCharSlashOk) 
        {
            adm_snprintf(tempDirName, sizeof(tempDirName), "%s", kvStorageDir);
        } 
        else 
        {
            adm_snprintf(tempDirName, sizeof(tempDirName), "%s/", kvStorageDir);
        }

        //!< 创建kvstorage的具体业务目录，business
        adm_memset(dirName, 0x00, sizeof(dirName), sizeof(dirName) * sizeof(DCHAR));
        adm_snprintf(dirName, sizeof(dirName), "%s%s", tempDirName, ADM_ENGINE_COM_BUSNISS_DIR);
        //删除business目录
        adm_com_rmDirAndFiles(dirName);
        //创建business目录
        iRet = adm_com_createDir(dirName);
        if (ADM_ECODE_SUCCESSED != iRet) 
        {
            log_e("adm_com_createDir %s failed, err:%d", dirName, iRet);
            break;
        }

        //!< 初始化kvstorage business数据库目录
        if (NULL == g_kvBusinessDatabase) 
        {
            g_kvBusinessDatabase = adm_com_kvstorage_open((const char*)dirName);
            if (NULL == g_kvBusinessDatabase)
            {
                if(FALSE == __adm_core_service_OpenBusinessFailedDeal(dirName))
                {
                    iRet = ADM_ECODE_KV_OPEN_FAILED;
                    log_e("adm_com_kvstorage_open %s failed, err:%d", dirName, iRet);
                    break;
                }
            }
        }

        //!< 创建kvstorage的具体业务目录，report
        adm_memset(dirName, 0x00, sizeof(dirName), sizeof(dirName) * sizeof(DCHAR));
        adm_snprintf(dirName, sizeof(dirName), "%s%s", tempDirName, ADM_ENGINE_COM_REPORT_DIR);
        //删除report目录
        adm_com_rmDirAndFiles(dirName);
        //创建report目录
        iRet = adm_com_createDir(dirName);
        if (ADM_ECODE_SUCCESSED != iRet) 
        {
            log_e("adm_com_createDir %s failed, err:%d", dirName, iRet);
            break;
        }

        //!< 初始化kvstorage report数据库目录
        if (NULL == g_kvReportDatabase) 
        {
            g_kvReportDatabase = adm_com_kvstorage_open((const char*)dirName);
            if (NULL == g_kvReportDatabase) 
            {
                if(FALSE == __adm_core_service_OpenReportFailedDeal(dirName))
                {
                    iRet = ADM_ECODE_KV_OPEN_FAILED;
                    log_e("adm_com_kvstorage_open %s failed, err:%d", dirName, iRet);
                    break;
                }
            }
        }
    }while(FALSE);

    adm_free(kvStorageDir);

    return ;
}



/**
 * @brief 获取OTA过程中业务信息存储的kv数据库
 *
 * @return KvStorage_t*     NULL:Failed, None-NULL:获取到的数据库句柄
 */
KvStorage_t* adm_core_service_getBusinessDatabaseDb()
{
    return g_kvBusinessDatabase;
}

/**
 * @brief 获取OTA过程中上报信息存储的kv数据库
 *
 * @return KvStorage_t*     NULL:Failed, None-NULL:获取到的数据库句柄
 */
KvStorage_t* adm_core_service_getReportDatabaseDb()
{
    return g_kvReportDatabase;
}

/**
 * @brief 设置OTA程序运行时目录，包括：Kv、log、package、trans等
 *
 * @param eType     [IN]    待设置运行时目录的类型
 * @param runtimeDir    [IN]    运行时目录
 * @return DBOOL        TRUE:设置成功，FALSE:设置失败
 */
DBOOL adm_core_service_initRuntimeDir(const ADM_CORE_SERVICE_RUNTIME_DIR_TYPE_E eType, const DCHAR* runtimeDir)
{
    DBOOL isSetOk = FALSE;

    //!< 校验入参
    if (NULL == runtimeDir) {
        log_e("Input param runtimeDir is nil");
        return FALSE;
    }

    switch (eType) {
    case ADM_CORE_SERVICE_RUNTIME_DIR_TYPE_KV_STORAGE:
        if (NULL != g_kvStorageDir) {
            adm_free(g_kvStorageDir);
        }
        g_kvStorageDir = adm_strdup(runtimeDir);
        isSetOk = TRUE;
        break;
    case ADM_CORE_SERVICE_RUNTIME_DIR_TYPE_LOG:
        if (NULL != g_logDir) {
            adm_free(g_logDir);
        }
        g_logDir = adm_strdup(runtimeDir);
        isSetOk = TRUE;
        break;
    case ADM_CORE_SERVICE_RUNTIME_DIR_TYPE_PACKAGE:
        if (NULL != g_packageDir) {
            adm_free(g_packageDir);
        }
        g_packageDir = adm_strdup(runtimeDir);
        isSetOk = TRUE;
        break;
    case ADM_CORE_SERVICE_RUNTIME_DIR_TYPE_CERT:
        if (NULL != g_certDir) {
            adm_free(g_certDir);
        }
        g_certDir = adm_strdup(runtimeDir);
        isSetOk = TRUE;
        break;
    case ADM_CORE_SERVICE_RUNTIME_DIR_TYPE_UNZIP:
        if (NULL != g_unzipDir) {
            adm_free(g_unzipDir);
        }
        g_unzipDir = adm_strdup(runtimeDir);
        isSetOk = TRUE;
        break;
    default:
        log_w("eType:%d is nil, runtimeDir:%s, isSetOk:%s", eType, runtimeDir, (isSetOk == TRUE) ? "TRUE" : "FALSE");
        isSetOk = FALSE;
        break;
    }

    log_i("eType:%d, runtimeDir:%s, isSetOk:%s", eType, runtimeDir, (isSetOk == TRUE) ? "TRUE" : "FALSE");

    return isSetOk;
}

/**
 * @brief 获取运行时目录
 *
 * @param eType     [IN]    运行时目录的类型
 * @return DCHAR*   NULL:Failes, None-NULL: 获取到的运行时目录信息
 * @note 运行时目录信息，使用结束后，需释放内存
 */
DCHAR* adm_core_service_getRuntimeDir(const ADM_CORE_SERVICE_RUNTIME_DIR_TYPE_E eType)
{
    DCHAR* runtimeDir = NULL;

    switch (eType) {
    case ADM_CORE_SERVICE_RUNTIME_DIR_TYPE_KV_STORAGE:
        if (NULL != g_kvStorageDir) {
            runtimeDir = adm_strdup(g_kvStorageDir);
        }
        break;
    case ADM_CORE_SERVICE_RUNTIME_DIR_TYPE_LOG:
        if (NULL != g_logDir) {
            runtimeDir = adm_strdup(g_logDir);
        }
        break;
    case ADM_CORE_SERVICE_RUNTIME_DIR_TYPE_PACKAGE:
        if (NULL != g_packageDir) {
            runtimeDir = adm_strdup(g_packageDir);
        }
        break;
    case ADM_CORE_SERVICE_RUNTIME_DIR_TYPE_CERT:
        if (NULL != g_certDir) {
            runtimeDir = adm_strdup(g_certDir);
        }
        break;
    case ADM_CORE_SERVICE_RUNTIME_DIR_TYPE_UNZIP:
        if (NULL != g_unzipDir) {
            runtimeDir = adm_strdup(g_unzipDir);
        }
        break;
    default:
        log_w("eType:%d is invalidd", eType);
        break;
    }

    return runtimeDir;
}

/**
 * @brief 释放OTA程序运行时目录信息
 *
 * @param eType     [IN]    运行时目录类型
 */
void adm_core_service_deinitRuntimeDir(const ADM_CORE_SERVICE_RUNTIME_DIR_TYPE_E eType)
{
    switch (eType) {
    case ADM_CORE_SERVICE_RUNTIME_DIR_TYPE_KV_STORAGE:
        if (NULL != g_kvStorageDir) {
            adm_free(g_kvStorageDir);
        }
        break;
    case ADM_CORE_SERVICE_RUNTIME_DIR_TYPE_LOG:
        if (NULL != g_logDir) {
            adm_free(g_logDir);
        }
        break;
    case ADM_CORE_SERVICE_RUNTIME_DIR_TYPE_PACKAGE:
        if (NULL != g_packageDir) {
            adm_free(g_packageDir);
        }
        break;
    case ADM_CORE_SERVICE_RUNTIME_DIR_TYPE_CERT:
        if (NULL != g_certDir) {
            adm_free(g_certDir);
        }
        break;
    case ADM_CORE_SERVICE_RUNTIME_DIR_TYPE_UNZIP:
        if (NULL != g_unzipDir) {
            adm_free(g_unzipDir);
        }
        break;
    default:
        log_w("eType:%d is invalidd", eType);
        break;
    }

    return;
}

/**
 * @brief 初始化车云通信使用的默认密钥
 * 
 * @param secret    [IN]    车云通信使用的默认密钥
 * @return DBOOL    TRUE: Successed, FALSE: Failed
 */
DBOOL adm_core_service_initOcsDefaultSecret(const DCHAR* secret)
{
    DBOOL isSetOk = FALSE;

    do {
        //!< 参数判空
        if (NULL == secret) {
            log_e("Input param secret is nil");
            break;
        }

        //!< 释放已经存在的secret
        if (NULL != g_defaultSecert) {
            adm_free(g_defaultSecert);
        }

        //!< 赋值secret
        g_defaultSecert = adm_strdup(secret);
        isSetOk = TRUE;
    } while(0);

    return isSetOk;
}

/**
 * @brief 获取车云通信使用的默认密钥
 * 
 * @return DCHAR*   NULL:Failed, None-NULL:车云通信使用的默认密钥
 * @note 默认密钥使用完成后需要释放空间
 */
DCHAR* adm_core_service_getOcsDefaultSecret(void)
{
    DCHAR* defaultSecret = NULL;

    if (NULL != g_defaultSecert) {
        defaultSecret = adm_strdup(g_defaultSecert);
    }

    return defaultSecret;
}

/**
 * @brief 释放车云通信使用的默认密钥
 * 
 */
void adm_core_service_deinitOcsDefaultSecret(void)
{
    if (NULL != g_defaultSecert) {
        adm_free(g_defaultSecert);
    }
}

/**
 * @brief 获取core service的版本号d
 * 
 * @return DCHAR*   NULL:Failed, None-NULL: core service 的版本号
 */
DCHAR* adm_core_service_sdkVersion(void)
{
    //TODO: 获取engine模块的版本号
    static DCHAR sdkVersion[128] = {0};

    adm_memset(sdkVersion, 0x00, sizeof(sdkVersion), sizeof(sdkVersion));
#ifdef ADM_ENGINE_SDK_VERSION
    adm_snprintf(sdkVersion, sizeof(sdkVersion), "%s", ADM_ENGINE_SDK_VERSION);
#else
    adm_snprintf(sdkVersion, sizeof(sdkVersion), "%s", ADM_CORE_SERVICE_DEFAULT_VERSION);
#endif
    log_i("sdk version of engine module:%s", sdkVersion);

    return sdkVersion;
}