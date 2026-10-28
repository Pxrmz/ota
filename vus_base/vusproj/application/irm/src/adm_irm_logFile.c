/**
 * @file adm_irm_logFile.c
 * @author mengwei@abupdate.com
 * @brief 日志采集的整个流程：
 *  1. 日志采集是个耗时的操作，所以会记录当前日志采集的状态； 包括：IDLE、COLLECTING、PACKING状态
 *  2. 日志上报开始执行后，先采集日志，按照ucmconfig.ini中配置的instanceId，挨个采集，并记录每个instanceId的状态信息，包括：FALSE和TRUE；
 *      2.1 SYSI 各模块的日志采集到 /xxx/admota/ucm/log/collect/ 目录下，命名为：sysi_instanceid_timestamp.zip 
 *      2.2 采集主控UCM的日志，将日志压缩到 /xxx/admota/ucm/log/collect/ 目录下；命名为 admfotaapp_timestamp.zip
 *  3. 日志采集结束后，需要将收集到的所有日志，压缩到 /xxx/admota/ucm/log/pack/ 目录下，命名为：log_timestamp.zip；
 *  4. 执行上报，上报时，log_timestamp.zip 会被上报模块修改名字；
 *      4.1 上报成功后，会删除日志文件压缩包
 *      4.2 上报失败，上报模块会自动执行重试或周期上报
 * @version 0.1
 * @date 2022-05-20
 * 
 * @copyright Copyright © 2018 All rights 上海艾拉比智能科技有限公司
 * 
 */
#include "adm_irm_log.h"
#include "adm_se_crypt.h"
#include "adm_irm_logFile.h"
#include "adm_irm_report.h"
#include "adm_com_kvstorage.h"
#include "adm_engine_common.h"
#include "adm_com_errorCode.h"
#include "adm_com_lib.h"
#include "adm_memory.h"
#include "adm_string.h"
#include "adm_com_hash.h"
#include "adm_cfg_api.h"
#include "adm_file.h"
#include "adm_soa.h"
#include "adm_dim_transHost.h"
#include "adm_pub_keylog.h"


//!< tpm 依赖 ewebserver 的支持 
#ifdef CMAKE_CUSTOM_EWEBSERVER_SUPPORT
    #include "adm_tpm_transfer.h"
#endif

#include <stdlib.h>
#include <pthread.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

static ADM_IRM_LOG_FILE_REPORT_INIT_T* g_initInfo = NULL;         //!< 日志上报初始化信息
static DBOOL g_initOk = FALSE;                                    //!< 日志上报信息是否已初始化ok
static const DCHAR* g_LogReportEncryptsecert = "secret";          //!< 日志上报文件加密秘钥
#define ADM_ENGINEER_MODE_LOG_COLLECT_DEFAULT_NAME    "Engineer_logPackage.zip"   /// 获取到的日志文件名默认为
#define ADM_ENGINEER_MODE_KEYLOG_COLLECT_DEFAULT_NAME    "Engineer_keylogPackage.zip"   /// 获取到的日志文件名默认为
#define ADM_KEYLOG_FILE_NAME "KEYLOG_FILE"
static DBOOL g_isEmsCollect = FALSE;          //!< 标识工程模式下的日志采集
static DCHAR *g_zipFileName = NULL;


struct listLock_irm {
    pthread_mutex_t lock;
    int initState;
};

static struct listLock_irm _lock = {.initState = 0};

static void lockInitOnce(void)
{
    if (_lock.initState == 1) {
        return;
    }

    (void)pthread_mutex_init(&_lock.lock, NULL);
    _lock.initState = 1;
}

static D32S logFileLock(void)
{
    lockInitOnce();
    return pthread_mutex_lock(&_lock.lock);
}

static D32S logFileUnlock(void)
{
    lockInitOnce();
    return pthread_mutex_unlock(&_lock.lock);
}


/*************************************************************KV 数据增删改查***********************************************************************/
static const DCHAR* g_logFileStartTimeKvKey = "irm_log_file_start_time";                      //!< 日志文件记录的开始时间
static const DCHAR* g_logFileReportStartTimeKvKey = "irm_log_file_report_start_time";     //!< 用于上报的日志文件的开始时间
static const DCHAR* g_logFileReportEndTimeKvKey = "irm_log_file_report_end_time";         //!< 用于上报的日志文件记录的结束时间
static const DCHAR* g_logFileNextReportPeriodicTimeKvKey = "irm_log_file_next_report_periodic_time";  //!< 触发下次周期上报的时间
static const DCHAR* g_logFileReportPeriodicTimeKvKey = "irm_log_file_report_periodic_time";   //!< 用于上报日志的周期间隔
static const DCHAR* g_logFileStateKvKey = "irm_log_file_state";     //!< 保存在KV中的，日志上报的状态
static const DCHAR* g_logFileSysiCollectInitedKey = "irm_log_file_sysi_collect_inited";    //!< 用于标识SYSI模块日志采集是否已经初始化
static const DCHAR* g_packLogFileNameKey = "irm_log_file_pack_name";           //!< 打包后的日志文件名

#define ADM_IRM_UCM_LOG_FILE_PREFIX     "admfotaapp"    //!< 主控日志压缩文件名前缀
#define ADM_IRM_APK_LOG_FILE_PREFIX     "apk"           //!< APK日志压缩文件名前缀
#define ADM_IRM_SYSI_LOG_FILE_PREFIX    "sysi"          //!< SYSI 各模块日志文件名前缀
#define ADM_IRM_LOG_FILE_PREFIX         "log"           //!< 压缩的日志文件名前缀
#define ADM_IRM_KEY_LOG_FILE_PREFIX     "keylog"           //!< 压缩的日志文件名前缀
#define ADM_IRM_ENC_LOG_FILE_SUFFIX     "enc"           //!< 加密日志文件名的后缀
#define ADM_IRM_KV_STORAGE_FILE_PREFIX  "kvstorage"     //!< 数据库文件前缀
#define ADM_IRM_kV_BUSINESS_FILE_PREFIX "business"      //!< 业务数据库
#define ADM_IRM_KV_REPORT_FILE_PREFIX   "report"        //!< 上报数据库
#define ADM_IRM_PKI_LOG_FILE_PREFIX     "pki"    //!< PKI日志压缩文件名前缀
//#define ADM_IRM_REMOVE_FILE_IGNORE_NAME     "admfotaapp.log"        //!< 忽略删除的日志文件名
//#define ADM_IRM_APK_REMOVE_FILE_IGNORE_NAME     "ota.log"        //!< APK忽略删除的日志文件名
#define ADM_IRM_SYSI_LOG_COLLECT_STATE_PREFIX   "irm_log_file_sysi_collect_state_"      //!< SYSI 各模块日志采集状态key的前缀
#define ADM_IRM_FILENAME_LENGTH               256     //!< 文件名长度
#define ADM_IRM_LOGSTATE_INIT_WAIT_SYSI_TIME  20      //!< 日志状态初始化中恢复日志上报需要等待20s，sysi4uclstart启动

static DBOOL adm_irm_collectKeyLogFileDeal(void);

/**
 * @brief 校验此KEY是否已经存储在KV中
 * @param startTimeKey  [IN]    存储在KV中，startTime的key
 * 
 * @return DBOOL    TRUE:Successed, FALSE: Failed
 */
static DBOOL __adm_irm_logFileTimeIsExist(const DCHAR* startTimeKey)
{
    return adm_com_kvstorage_has_key(adm_core_service_getBusinessDatabaseDb(), startTimeKey);
}

/**
 * @brief 获取存储的日志文件开始的时间
 * 
 * @param startTimeKey  [IN]    存储在KV中，startTime的key
 * @param startTime     [OUT]       日志初始化的时间
 * @return DBOOL        TRUE: 获取成功，FALSE: 获取失败
 */
static DBOOL __adm_irm_logFileTimeGet(const DCHAR* startTimeKey, D64S* startTime)
{
    DBOOL isGetOk = FALSE;
    ADM_KV_STORAGE_VALUE_TYPE_E eType = ADM_KV_STORAGE_VALUE_TYPE_INVALID;
    DCHAR* startTimeStr = NULL;
    size_t startTimeStrLen = 0;

    do {
        //!< 从kvstorage中获取irm_log_file_start_time字段
        if (FALSE == adm_com_kvstorage_get_string(adm_core_service_getBusinessDatabaseDb(), startTimeKey, 
            &startTimeStr, &startTimeStrLen, &eType)) {
            log_e("get %s failed", startTimeKey);
            break;
        }

        //!< 判断获取到的值是否有效
        if (NULL == startTimeStr || (D32U)0 >= startTimeStrLen || ADM_KV_STORAGE_VALUE_TYPE_INT64 != eType) {
            log_e("startTimeStr is nil || startTimeStrLen: %d is invalid || eType != %d", startTimeStrLen, ADM_KV_STORAGE_VALUE_TYPE_INT64);
            break;
        }

        *startTime = atol(startTimeStr);
        log_i("%s get successed startTimeStr:%s, startTime:%lld", startTimeKey, startTimeStr, *startTime);
        isGetOk = TRUE;
        adm_free(startTimeStr);
    } while(FALSE);

    return isGetOk;
}

/**
 * @brief 存储日志文件开始的时间
 * 
 * @param startTimeKey  [IN]    存储在KV中，startTime的key
 * @param startTime     [IN]    日志初始化的时间
 * @return DBOOL        TRUE: 存储成功，FALSE: 存储失败
 */
static DBOOL __adm_irm_logFileTimeSet(const DCHAR* startTimeKey, const D64S startTime)
{
    DBOOL isSetOk = FALSE;
    ADM_KV_STORAGE_VALUE_TYPE_E eType = ADM_KV_STORAGE_VALUE_TYPE_INT64;
    DCHAR startTimeStr[16] = {0};

    //!< 将startTime 存储到kv中
    (void)adm_memset(startTimeStr, 0x00, sizeof(startTimeStr), sizeof(startTimeStr));
    (void)adm_snprintf(startTimeStr, sizeof(startTimeStr), "%lld", startTime);
    log_i("set startTimeStr = %s", startTimeStr);
    log_i("set startTime = %lld", startTime);
    if (TRUE == adm_com_kvstorage_set_string(adm_core_service_getBusinessDatabaseDb(), startTimeKey, 
            startTimeStr, eType)) {
        isSetOk = TRUE;
    }

    return isSetOk;
}

/**
 * @brief 从kvstorage中移除此字段
 * 
 * @param startTimeKey  [IN]    存储在KV中，startTime的key
 * @return DBOOL    TRUE: 移除成功， FALSE: 移除失败
 */
static DBOOL __adm_irm_logFileTimeDel(const DCHAR* startTimeKey)
{
    return adm_com_kvstorage_remove_key(adm_core_service_getBusinessDatabaseDb(), startTimeKey);
}

/**
 * @brief 更新日志上报的周期间隔和下次上报的时间点
 * 
 * @param logReportPeriodic 
 * @return DBOOL 
 */
static void __adm_irm_logFilePeriodicTimeUpdate(void)
{
    D64S logReportPeriodic = 0;
    D32S s32LogReportCycle = 0;
    D64S lSavedLogReportPeriodic = 0;

    do {
        //!< 检验参数是否为空
        if (NULL == g_initInfo || NULL == g_initInfo->getLogReportCycle) {
            log_e("Input param g_initInfo || g_initInfo->getLogReportCycle nil");
            //!< 如果参数为空，则置为默认值
            logReportPeriodic = ADM_IRM_LOG_REPORT_PERIODIC_DEFAULT;
        } else {
            //!< 从ECS中获取的值为无效值，则置为默认值
            (void)g_initInfo->getLogReportCycle(&s32LogReportCycle);
            if (0 >= s32LogReportCycle) {
                logReportPeriodic = ADM_IRM_LOG_REPORT_PERIODIC_DEFAULT;
            } else {
                logReportPeriodic = s32LogReportCycle;
            }
        }

        //!< 不存在，如果没有存储，则通过接口获取并更新（间隔和下次触发时间）
        if (FALSE == __adm_irm_logFileTimeIsExist(g_logFileReportPeriodicTimeKvKey)) {
            (void)__adm_irm_logFileTimeSet(g_logFileReportPeriodicTimeKvKey, logReportPeriodic);
            (void)__adm_irm_logFileTimeSet(g_logFileNextReportPeriodicTimeKvKey, logReportPeriodic + time(NULL));
        } else {
            //!< 如果存在则获取出来，并判断是否相同，如果不同，则通过接口获取并更新（间隔和下次触发时间）
            (void)__adm_irm_logFileTimeGet(g_logFileReportPeriodicTimeKvKey, &lSavedLogReportPeriodic);
            if (lSavedLogReportPeriodic != logReportPeriodic) {
                (void)__adm_irm_logFileTimeSet(g_logFileReportPeriodicTimeKvKey, logReportPeriodic);
                (void)__adm_irm_logFileTimeSet(g_logFileNextReportPeriodicTimeKvKey, logReportPeriodic + time(NULL));
            }
            //!< 注：不用判断下次触发上报的时间：
            // 1. 如果kv中保存的下次触发时间，已经小于当前时间，不需要更新，等待周期触发之后，再更新即可
            // 2. 如果kv中保存的下次触发时间，大于当前的时间，也不需要更新，等待周期触发之后，再更新即可
        }
    } while(FALSE);

    return;

}

/**
 * @brief 存储日志文件开始的时间
 * @note 为防止断电，如果已经存在了此字段，说明时间已经存储，不用再次存储
 * @note  先判断是否已经存储了startTime字段, 如果已经存储，则返回成功，如果没有存储则重新存储到kvstorage中
 * 
 * @param startTime     [IN]    日志初始化的时间
 * @return DBOOL        TRUE: 存储成功，FALSE: 存储失败
 */
DBOOL adm_irm_logFileStartTimeStore(const D64S startTime)
{
    DBOOL isStoreOk = FALSE;

    do {
        //!< 先判断是否已经存储了startTime字段, 如果已经存储，则返回成功，如果没有存储则重新存储到kvstorage中
        if (TRUE == __adm_irm_logFileTimeIsExist(g_logFileStartTimeKvKey)) {
            log_i("key:%s is existed", g_logFileStartTimeKvKey);
            isStoreOk = TRUE;
            break;
        }

        //!< 日志开始记录的时间，存储数据到kvstorage中
        if (FALSE == __adm_irm_logFileTimeSet(g_logFileStartTimeKvKey, startTime)) {
            log_e("key:%s value:%lld storage to kvstorage failed", g_logFileStartTimeKvKey, startTime);
            break;
        }

        //!< 设置存储成功的标识
        isStoreOk = TRUE;

    } while(FALSE);

    return isStoreOk;
}

/**
 * @brief 将状态值设置到kvstroage中
 * 
 * @param eState    [IN]    状态值
 * @return DBOOL    TRUE:设置成功, FALSE: 设置失败
 */
static DBOOL __adm_irm_logFileStateSet(const ADM_IRM_LOG_FILE_STATE_E eState)
{
    DBOOL isSetOk = FALSE;
    DCHAR stateStr[16] = {0};

    do {
        //!< 校验状态值
        if (ADM_IRM_LOG_FILE_STATE_IDLE > eState || ADM_IRM_LOG_FILE_STATE_INVALID <= eState) {
            log_e("eState:%d is not invalid", eState);
            break;
        }

        //!< 将状态值转为字符串，存储到kv中
        (void)adm_memset(stateStr, 0x00, sizeof(stateStr), sizeof(stateStr));
        (void)adm_snprintf(stateStr, sizeof(stateStr), "%d", eState);
        log_i("set stateStr = %s", stateStr);
        log_i("set eState = %d", eState);
        if (FALSE == adm_com_kvstorage_set_string(adm_core_service_getBusinessDatabaseDb(), g_logFileStateKvKey, 
            stateStr, ADM_KV_STORAGE_VALUE_TYPE_UINT32)) {
                log_e("set eState failed");
                break;
        }

        isSetOk = TRUE;
        log_i("set eState successed");
    } while(FALSE);

    return isSetOk;
}

/**
 * @brief 获取状态值
 * 
 * @param eState    [OUT]   获取到的状态值
 * @return DBOOL    TRUE: 获取成功, FALSE: 获取失败
 */
static DBOOL __adm_irm_logFileStateGet(ADM_IRM_LOG_FILE_STATE_E* eState)
{
    //!< 获取状态值
    DBOOL isGetOk = FALSE;
    ADM_KV_STORAGE_VALUE_TYPE_E eType = ADM_KV_STORAGE_VALUE_TYPE_INVALID;
    DCHAR* pValue = NULL;
    size_t len = 0;

    do {
        //!< 无法获取到状态key, 初始化状态值为 ADM_IRM_LOG_FILE_STATE_IDLE
        if (FALSE == adm_com_kvstorage_get_string(adm_core_service_getBusinessDatabaseDb(), g_logFileStateKvKey,
            &pValue, &len, &eType)) {
                log_w("adm_com_kvstorage_get_string key:%s failed", g_logFileStateKvKey);
                break;
        }

        //!< 获取到的状态值为空，或者长度为0，则说明获取到的值错误，也初始化状态值为 ADM_IRM_LOG_FILE_STATE_IDLE
        if (NULL == pValue || (D32U)0 >= len || ADM_KV_STORAGE_VALUE_TYPE_UINT32 != eType) {
            log_w("adm_com_kvstorage_get_string key:%s failed [pValue = NULL | len：%d <= 0 || eType:%d]", g_logFileStateKvKey, len, eType);
            break;
        }

        //!< 将获取到的string，转换为 整型值
        *eState = (ADM_IRM_LOG_FILE_STATE_E)atoi(pValue);
        //!< 校验状态值
        if (ADM_IRM_LOG_FILE_STATE_IDLE > *eState || ADM_IRM_LOG_FILE_STATE_INVALID <= *eState) 
        {
            log_e("eState:%d is not invalid", *eState);
            break;
        }
        isGetOk = TRUE;
        log_i("adm_com_kvstorage_get_string successed, key:%s, value:%s, eState:%d", g_logFileStateKvKey, pValue, *eState);
        adm_free(pValue);
    } while(FALSE);

    return isGetOk;
}

/**
 * @brief 将打包后的日志文件名的全路径存储到kvstorage中
 * 
 * @param fullZipfileName   [IN]    打包后的日志文件全路径名
 * @return DBOOL            TRUE:Successed, FALSE: Failed
 */
static DBOOL __adm_irm_logFilePackNameSet(const DCHAR* fullZipfileName)
{
    DBOOL isSetOk = FALSE;

    do {
        //!< 校验参数
        if (NULL == fullZipfileName) {
            log_e("Input param fullZipFileName is nil");
            break;
        }

        if (FALSE == adm_com_kvstorage_set_string(adm_core_service_getBusinessDatabaseDb(), g_packLogFileNameKey, 
            fullZipfileName, ADM_KV_STORAGE_VALUE_TYPE_STRING)) {
                log_e("set fullZipfileName:%s failed", fullZipfileName);
                break;
        }

        isSetOk = TRUE;
        log_i("set fullZipfileName:%s successed", fullZipfileName);
    } while(FALSE);

    return isSetOk;
}

/**
 * @brief 获取保存在KV中的日志压缩文件路径信息
 * 
 * @param fullZipfileName   [OUT]   日志压缩文件路径信息， 
 * @note fullSipFileName 参数需要释放空间   
 * @return DBOOL    TRUE:Successed, FALSE: Failed
 */
static DBOOL __adm_irm_logFilePackNameGet(DCHAR** fullZipfileName)
{
    DBOOL isGetOk = FALSE;
    DCHAR* pValue = NULL;
    size_t len = 0;
    ADM_KV_STORAGE_VALUE_TYPE_E eType = ADM_KV_STORAGE_VALUE_TYPE_STRING;

    do {
        if (FALSE == adm_com_kvstorage_get_string(adm_core_service_getBusinessDatabaseDb(), g_packLogFileNameKey,
            &pValue, &len, &eType)) {
                log_e("adm_com_kvstorage_get_string key:%s failed", g_packLogFileNameKey);
                break;
        }

        isGetOk = TRUE;
        log_i("adm_com_kvstorage_get_string key:%s successed  fullZipfileName:%s", g_packLogFileNameKey, pValue);
    } while(FALSE);

    *fullZipfileName = pValue;

    return isGetOk;
}

/**
 * @brief 清除保存在KV中的，日志压缩文件路径信息
 * 
 * @return DBOOL    TRUE:Successed, FALSE: TRUE
 */
static DBOOL __adm_irm_logFilePackNameDel(void)
{
    return adm_com_kvstorage_remove_key(adm_core_service_getBusinessDatabaseDb(), g_packLogFileNameKey);
}

/*************************************************************KV 数据增删改查***********************************************************************/

/*************************************************************日志文件处理***********************************************************************/
/**
 * @brief 删除目录下的所有日志文件
 * 
 * @param dirPath       [IN]    日志文件存储目录
 * @param notContainFile    [IN]    删除的日志文件不包含
 * @note 
 * 1. notContainFile:  NULL: 所有的文件都需要删除
 * 2. notContainFile:  None-NULL: 文件名与notContainFile相同的不需要删除
 * 
 */
static void __adm_irm_removeLogFiles(const DCHAR* dirPath, const DCHAR* notContainFile)
{
    DIR* dir = NULL;
    struct dirent* ptr = NULL;
    struct stat statbuf;
    DCHAR fullFileName[256] = {0};
    D32U len = 0;

    do {
        //!< 校验入参
        if (NULL == dirPath) {
            log_e("Input param dirPath is nil");
            break;
        }

        //!< 打开日志文件存储的目录
        dir = opendir(dirPath);
        if (NULL == dir) {
            log_e("opendir logStorageDir:%s failed", dirPath);
            break;
        }

        //!< 读取目录下的所有文件
        while(NULL != (ptr = readdir(dir))) {
            (void)adm_memset(fullFileName, 0x00, sizeof(fullFileName), sizeof(fullFileName));
            len = adm_strlen(dirPath);
            if ('/' == dirPath[len - (D32U)1]) {
                (void)adm_snprintf(fullFileName, sizeof(fullFileName), "%s%s", dirPath, ptr->d_name);
            } else {
                (void)adm_snprintf(fullFileName, sizeof(fullFileName), "%s/%s", dirPath, ptr->d_name);
            }

            //!< 判断类型如果不是文件，执行下一个文件类型的类型判断
            if (stat(fullFileName, &statbuf) == -1) 
            {
                log_e("%s,stat error", fullFileName);
                continue;
            }
            if (!S_ISREG(statbuf.st_mode)) {
                log_w("%s: is not file\n", fullFileName);
                continue;
            } 
            //if ((D32U)8 != ptr->d_type) {
            //    log_w("ptr->d_type:%d is not file", (D32U)ptr->d_type);
            //    continue;
            //}

            //!< 循环删除日志文件
            //if (NULL != ptr->d_name) {
                //!< 处理需要被删除的文件，将不需要被删除的文件剔除去
            if (NULL != notContainFile && 0 == adm_strcmp(ptr->d_name, notContainFile)) {
                log_i("pst->d_name:%s %sis not contain file, do not need to delete", ptr->d_name, notContainFile);
            } else {
                (void)adm_memset(fullFileName, 0x00, sizeof(fullFileName), sizeof(fullFileName));
                len = adm_strlen(dirPath);
                if (0 != len ) {
                    if ('/' == dirPath[len - (D32U)1]) {
                        (void)adm_snprintf(fullFileName, sizeof(fullFileName), "%s%s", dirPath, ptr->d_name);
                    } else {
                        (void)adm_snprintf(fullFileName, sizeof(fullFileName), "%s/%s", dirPath, ptr->d_name);
                    }
                    (void)adm_fremove(fullFileName);
                    log_i("adm_com_rmFile %s", fullFileName);
                }
                else {
                    log_e("get dirPath length is error");
                }
            }
            //}
        }
        (void)closedir(dir);

    } while(FALSE);
    
    return;
}

/**
 * @brief 采集每个instanceId 对应的日志文件，并将日志文件移动到 log/collect/目录下
 * @note 重命名为，sysi_instanceId_timestamp.zip; eg:sysi_0X2001_160000000.zip
 * 
 * @param instanceId    [IN]    SYSI 模块的instanceId
 * @return DBOOL        TRUE:Successed, FALSE:Failed
 */
static DBOOL __adm_irm_collecySysiLogFileEach(const D32S instanceId,ADM_DIM_SOA_TRANS_FILE_TYPE_E eFileType)
{
    DCHAR cInstanceId[12] = {0};
    DCHAR* pcLogPath = NULL;
    D32S iRet = ADM_ECODE_FAILED;
    DBOOL isCollectOk = FALSE;
    DCHAR cNewLogPath[256] = {0};
    size_t len = 0;

    do {
        //!< 校验入参
        if (NULL == g_initInfo) {
            log_e("Input param g_initInfo is nil");
            break;
        }

        //!< 将整型instanceId转换为字符串格式
        (void)adm_memzero(cInstanceId, sizeof(cInstanceId));
        (void)adm_snprintf(cInstanceId, sizeof(cInstanceId), "0X%X", instanceId);

        log_i("InstanceId:%s doing collect log file", cInstanceId);

//!< sysi侧日志收集依赖 tpm，tpm 又依赖 ewebserver 的支持 
#ifdef CMAKE_CUSTOM_EWEBSERVER_SUPPORT
    //!< 采集日志
    iRet = adm_tpm_CollectLogFromSysiToHost(cInstanceId, eFileType, &pcLogPath);
    if (ADM_ECODE_SUCCESSED != iRet) {
        log_e("adm_tpm_CollectLogFromSysiToHost instanceId:%s failed, err:%d", cInstanceId, iRet);
        break;
    } else {
        log_i("adm_tpm_CollectLogFromSysiToHost instanceId:%s successed", cInstanceId);
    }
    
    if (NULL != g_initInfo->logCollectDir) {
        log_i("");
        len = adm_strlen(g_initInfo->logCollectDir);
        if (0 != len ) {
            (void)adm_memzero(cNewLogPath, sizeof(cNewLogPath));
            if ('/' == g_initInfo->logCollectDir[len - (D32U)1]) 
            {
                if (ADM_DIM_SOA_TRANS_FILE_TYPE_KEYLOG == eFileType)
                {
                    (void)adm_snprintf(cNewLogPath, sizeof(cNewLogPath), "%skeylog_%s_%lld.log", g_initInfo->logCollectDir, cInstanceId, time(NULL));
                }
                else
                {
                    (void)adm_snprintf(cNewLogPath, sizeof(cNewLogPath), "%ssysi_%s_%lld.zip", g_initInfo->logCollectDir, cInstanceId, time(NULL));
                }
                
            } 
            else 
            {
                if (ADM_DIM_SOA_TRANS_FILE_TYPE_KEYLOG == eFileType)
                {
                    (void)adm_snprintf(cNewLogPath, sizeof(cNewLogPath), "%s/keylog_%s_%lld.log", g_initInfo->logCollectDir, cInstanceId, time(NULL));
                }
                else
                {
                    (void)adm_snprintf(cNewLogPath, sizeof(cNewLogPath), "%s/sysi_%s_%lld.zip", g_initInfo->logCollectDir, cInstanceId, time(NULL));
                }
            }
        }
        else {
            log_e("get log collect dir length is error");
        }
    
        iRet = rename(pcLogPath, cNewLogPath);
        if (ADM_ECODE_SUCCESSED == iRet) {
            log_i("rename %s to %s successed", pcLogPath, cNewLogPath);
            isCollectOk = TRUE;
        } else {
            log_w("rename %s to %s failed", pcLogPath, cNewLogPath);
        }
    }
#endif

    } while(FALSE);

    return isCollectOk;
}

/**
 * @brief 根据instanceId字段，生成SYSI日志采集模块的状态key字段
 * 
 * @param instanceId    [IN]    SYSI 模块的实例ID
 * @param cSysiLogCollectStateKey   [OUT]   输出生成的 key信息
 * @param keyLen    [IN]    KEY 数组长度
 */
static void __adm_irm_collectSysisLogFileStateKeyGenearte(const D32S instanceId, DCHAR cSysiLogCollectStateKey[], size_t keyLen)
{
    DCHAR cInstanceId[12] = {0};

    (void)adm_memzero(cInstanceId, sizeof(cInstanceId));
    (void)adm_snprintf(cInstanceId, sizeof(cInstanceId), "0X%X", instanceId);
    
    (void)adm_memzero(cSysiLogCollectStateKey, keyLen);
    (void)adm_snprintf(cSysiLogCollectStateKey, keyLen, ADM_IRM_SYSI_LOG_COLLECT_STATE_PREFIX"%s", cInstanceId);

    return;
}

/**
 * @brief 定义SYSI模块，日志采集状态值
 * 
 */
typedef enum {
    ADM_IRM_SYSI_LOG_COLLECT_STATE_INIT = 0x01,     //!< SYSI 模块日志采集初始化
    ADM_IRM_SYSI_LOG_COLLECT_STATE_TRANS = 0x02,    //!< SYSI 模块日志采集传输中
    ADM_IRM_SYSI_LOG_COLLECT_STATE_DEINIT = 0x03,   //!< SYSI 模块日志采集反初始化
} ADM_IRM_SYSI_LOG_COLLECT_STATE_E;

/**
 * @brief 循环解析各模块的instanceIds，并根据传入的状态值，执行不同的操作
 * 
 * @param pcInstanceidsKey  [IN]    ucmconfig.ini中，各模块的名称
 * @param eStateType        [IN]    日志采集的状态值
 */
static void __adm_irm_collectSysiLogFileParserMultiInstanceidDoState(const DCHAR* pcInstanceidsKey, const ADM_IRM_SYSI_LOG_COLLECT_STATE_E eStateType)
{
    DCHAR* instanceIdStr = NULL;
    DCHAR* subInstanceIdStr = NULL;
    D32S instanceId = 0;
    DCHAR cSysiLogCollectStateKey[128] = {0};
    DCHAR* pcSysiLogCollectStateVal = NULL;
    const DCHAR* cfgInstanceIdsStr = NULL;
    ADM_KV_STORAGE_VALUE_TYPE_E eValType = ADM_KV_STORAGE_VALUE_TYPE_INVALID;
    size_t len = 0;

    log_i("Do state = %d for %s", eStateType, pcInstanceidsKey);
    do {
        cfgInstanceIdsStr = adm_cfg_iniParserGetString(pcInstanceidsKey, "");
        instanceIdStr = adm_strdup(cfgInstanceIdsStr);
        if (NULL != instanceIdStr) {
            subInstanceIdStr = strsep(&instanceIdStr, ",");
            while(NULL != subInstanceIdStr) {
                log_i("subInstanceIdStr:%s", subInstanceIdStr);
                instanceId = atoi(subInstanceIdStr);
                __adm_irm_collectSysisLogFileStateKeyGenearte(instanceId, cSysiLogCollectStateKey, sizeof(cSysiLogCollectStateKey));
                switch (eStateType)
                {
                case ADM_IRM_SYSI_LOG_COLLECT_STATE_INIT:
                    (void)adm_com_kvstorage_set_string(adm_core_service_getBusinessDatabaseDb(),
                                            cSysiLogCollectStateKey,
                                            "FALSE",
                                            ADM_KV_STORAGE_VALUE_TYPE_STRING);
                    break;
                case ADM_IRM_SYSI_LOG_COLLECT_STATE_TRANS:
                     if (TRUE == adm_com_kvstorage_get_string(adm_core_service_getBusinessDatabaseDb(),
                                                cSysiLogCollectStateKey,
                                                &pcSysiLogCollectStateVal,
                                                &len,
                                                &eValType)) {
                        log_i("adm_com_kvstorage_get_string successed %s = %s", cSysiLogCollectStateKey, pcSysiLogCollectStateVal);
                        if (0 == adm_strncmp(pcSysiLogCollectStateVal, "FALSE", adm_strlen("FALSE"))) {
                            (void)__adm_irm_collecySysiLogFileEach(instanceId,ADM_DIM_SOA_TRANS_FILE_TYPE_LOG);
                            (void)adm_com_kvstorage_set_string(adm_core_service_getBusinessDatabaseDb(),
                                                        cSysiLogCollectStateKey,
                                                        "TRUE",
                                                        ADM_KV_STORAGE_VALUE_TYPE_STRING);
                        }
                        adm_free(pcSysiLogCollectStateVal);
                    } else {
                        log_w("adm_com_kvstorage_get_string %s failed", cSysiLogCollectStateKey);
                    }
                    break;
                case ADM_IRM_SYSI_LOG_COLLECT_STATE_DEINIT:
                    (void)adm_com_kvstorage_remove_key(adm_core_service_getBusinessDatabaseDb(), cSysiLogCollectStateKey);
                    break;
                default:
                    log_w("eStateType:%d is nivalid", eStateType);
                    break;
                }
                subInstanceIdStr = strsep(&instanceIdStr, ",");
            }
            adm_free(instanceIdStr);
        }
    } while(FALSE);

    return;
}

/**
 * @brief 为防止断电，在采集各SYSI模块日志的时候，保存每个日志采集的状态
 * @note 
 *  1. irm_log_file_sysi_collect_inited 标识，TRUE/FALSE
 *  2. irm_log_file_sysi_instanceId_collected 标识，TRUE/FALSE    每个instanceId是否已经采集的标识
 * 
 * @return DBOOL    TRUE:Successed, FALSE: Failed
 */
static DBOOL __adm_irm_collectSysiLogFileStateInit(void)
{
    DBOOL isInitedOk = FALSE;
    DCHAR* isInitedStr = NULL;
    size_t len = 0;
    ADM_KV_STORAGE_VALUE_TYPE_E eValType = ADM_KV_STORAGE_VALUE_TYPE_INVALID;

    do {
        //!< 增加是否已经初始化的标识
        if (TRUE == adm_com_kvstorage_get_string(adm_core_service_getBusinessDatabaseDb(),
                                        g_logFileSysiCollectInitedKey,
                                        &isInitedStr,
                                        &len,
                                        &eValType)) {
            //!< 判断已经初始化了，直接返回即可
            if (0 == adm_strncmp(isInitedStr, "TRUE", adm_strlen("TRUE"))) {
                log_i("%s is %s", g_logFileSysiCollectInitedKey, isInitedStr);
                isInitedOk = TRUE;
                break;
            }
        }

        //!< 循环读取所有的ucl_InstanceId，然后存储KV状态, 初始化UCL模块的状态
        __adm_irm_collectSysiLogFileParserMultiInstanceidDoState(ADM_CFG_VUS_UCL_INSTANCEID, ADM_IRM_SYSI_LOG_COLLECT_STATE_INIT);
        __adm_irm_collectSysiLogFileParserMultiInstanceidDoState(ADM_CFG_VUS_VDI_INSTANCEID, ADM_IRM_SYSI_LOG_COLLECT_STATE_INIT);

        //!< 标识SYSI日志采集已经初始化成功
        if (FALSE == adm_com_kvstorage_set_string(adm_core_service_getBusinessDatabaseDb(),
                                    g_logFileSysiCollectInitedKey,
                                    "TRUE",
                                    ADM_KV_STORAGE_VALUE_TYPE_STRING)) {
            log_e("adm_com_kvstorage_set_string %s to %s failed", g_logFileSysiCollectInitedKey, "TRUE");
            break;
        }

        isInitedOk = TRUE;
    } while(FALSE);

    adm_free(isInitedStr);

    return isInitedOk;
}

/**
 * @brief 循环读取所有的instanceId，并获取所有的日志文件，读取结束后，将对应的状态置为 "FALSE"
 * 
 */
static void __adm_irm_collectSysiLogFileCycle(void)
{
    DCHAR* isInitedStr = NULL;
    size_t len = 0;
    ADM_KV_STORAGE_VALUE_TYPE_E eValType = ADM_KV_STORAGE_VALUE_TYPE_INVALID;

    do {
        //!< 增加是否已经初始化的标识
        if (TRUE == adm_com_kvstorage_get_string(adm_core_service_getBusinessDatabaseDb(),
                                        g_logFileSysiCollectInitedKey,
                                        &isInitedStr,
                                        &len,
                                        &eValType)) {
            //!< 判断已经初始化了，直接返回即可
            if (0 == adm_strncmp(isInitedStr, "FALSE", adm_strlen("FALSE"))) {
                log_i("%s is %s", g_logFileSysiCollectInitedKey, isInitedStr);
                break;
            }
        }

        //!< 循环获取每个ucl_instanceId 的日志文件
        __adm_irm_collectSysiLogFileParserMultiInstanceidDoState(ADM_CFG_VUS_UCL_INSTANCEID, ADM_IRM_SYSI_LOG_COLLECT_STATE_TRANS);
        __adm_irm_collectSysiLogFileParserMultiInstanceidDoState(ADM_CFG_VUS_VDI_INSTANCEID, ADM_IRM_SYSI_LOG_COLLECT_STATE_TRANS);

    } while(FALSE);

    adm_free(isInitedStr);
    return;
}

/**
 * @brief 反初始化SYSI模块的日志采集状态
 * 
 */
static void __adm_irm_collectSysiLogFileStateDeinit(void)
{
    do {
        //!< 循环读取所有的ucl_InstanceId，然后存储KV状态, 反初始化UCL模块的状态
        __adm_irm_collectSysiLogFileParserMultiInstanceidDoState(ADM_CFG_VUS_UCL_INSTANCEID, ADM_IRM_SYSI_LOG_COLLECT_STATE_DEINIT);
        __adm_irm_collectSysiLogFileParserMultiInstanceidDoState(ADM_CFG_VUS_VDI_INSTANCEID, ADM_IRM_SYSI_LOG_COLLECT_STATE_DEINIT);

        //!< 标识SYSI日志采集已经初始化成功
        if (FALSE == adm_com_kvstorage_remove_key(adm_core_service_getBusinessDatabaseDb(),
                                                    g_logFileSysiCollectInitedKey)) {
            log_e("adm_com_kvstorage_remove_key %s failed", g_logFileSysiCollectInitedKey);
            break;
        }

    } while(FALSE);

    return;
}

/**
 * @brief 收集SYSI进程的日志信息，包括：SYSI4HMI、SYSI4HOST、SYSI4VDI、SYSI4UCL(各个零件上的UCL)
 * 
 */
static D32S __adm_irm_collectSysiLogFile(void)
{
    //!< 
    do {
        //!< SYSI模块，日志采集状态初始化
        if (FALSE == __adm_irm_collectSysiLogFileStateInit()) {
            log_e("__adm_irm_collectSysiLogFileStateInit failed");
            break;
        }

        //!< 触发执行各模块日志的收集，收集完成每一个后，状态置为 TRUE
        __adm_irm_collectSysiLogFileCycle();

        //!< 收集完成后，SYSI模块的采集状态反初始化
        __adm_irm_collectSysiLogFileStateDeinit();

    } while(FALSE);

    return 0;
}

static D32S __adm_irm_collectKvStorage(void)
{
    D32S iRet = ADM_ECODE_FAILED;
    D64S startTime = 0;
    D64S lTimeStamp = 0;
    DCHAR zipFileName[128] = {0};
    DCHAR pKvBusinessDir[128] = {0};
    DCHAR pKvReportDir[128] = {0};

    const DCHAR* pKvStorageDir = adm_cfg_iniParserGetString(ADM_CFG_VUS_KV_STORAGE_DIR, ADM_CFG_VUS_KV_STORAGE_DIR_DEFAULT_VAL);

    do {
        //!< 校验初始化变量
        if (NULL == g_initInfo) {
            iRet = ADM_ECODE_INVALID_PARAM;
            log_e("log file report module init falied, [err:%d]", iRet);
            break;
        }

        //!< 日志文件名默认为： admfotaapp_timestamp.zip
        (void)adm_memset(zipFileName, 0x00, sizeof(zipFileName), sizeof(zipFileName));
        lTimeStamp = time(NULL);
        (void)adm_snprintf(zipFileName, sizeof(zipFileName), "%s_%s_%lld.zip", ADM_IRM_KV_STORAGE_FILE_PREFIX, ADM_IRM_kV_BUSINESS_FILE_PREFIX, lTimeStamp);

        (void)adm_snprintf(pKvBusinessDir, sizeof(pKvBusinessDir), "%s/%s", pKvStorageDir, ADM_IRM_kV_BUSINESS_FILE_PREFIX);
        if(adm_com_isDirExist(pKvBusinessDir))
        {
            //!< 收集主控的日志文件，将主控的日志文件打包到 collect 目录下
            if (0 != adm_com_zipFile(pKvBusinessDir, g_initInfo->logCollectDir, zipFileName)) {
                log_e("adm_com_zipFile logStorageDir:%s, logCollectDir:%s, zipFileName:%s failed", pKvBusinessDir, g_initInfo->logCollectDir, zipFileName);
                break;
            }
        }
        else
        {
            log_i("%s: kv storage dir not exist", pKvBusinessDir);
            return ADM_ECODE_SUCCESSED;
        }

        //!< 日志文件名默认为： admfotaapp_timestamp.zip
        (void)adm_memset(zipFileName, 0x00, sizeof(zipFileName), sizeof(zipFileName));
        lTimeStamp = time(NULL);
        (void)adm_snprintf(zipFileName, sizeof(zipFileName), "%s_%s_%lld.zip", ADM_IRM_KV_STORAGE_FILE_PREFIX, ADM_IRM_KV_REPORT_FILE_PREFIX, lTimeStamp);

        (void)adm_snprintf(pKvReportDir, sizeof(pKvReportDir), "%s/%s", pKvStorageDir, ADM_IRM_KV_REPORT_FILE_PREFIX);
        if(adm_com_isDirExist(pKvReportDir))
        {
            //!< 收集主控的日志文件，将主控的日志文件打包到 collect 目录下
            if (0 != adm_com_zipFile(pKvReportDir, g_initInfo->logCollectDir, zipFileName)) {
                log_e("adm_com_zipFile logStorageDir:%s, logCollectDir:%s, zipFileName:%s failed", pKvReportDir, g_initInfo->logCollectDir, zipFileName);
                break;
            }
        }
        else
        {
            log_i("%s: kv storage dir not exist", pKvReportDir);
            return ADM_ECODE_SUCCESSED;
        }
        //!< remove UCM日志存储目录下的所有， xxx/admota/log/ucm/storage/ 日志文件
        //__adm_irm_removeLogFiles(g_initInfo->logStorageDir, ADM_IRM_REMOVE_FILE_IGNORE_NAME);

        iRet = ADM_ECODE_SUCCESSED;

    } while(FALSE);
    return iRet;
}

/**
 * @brief 收集UCM进程的日志信息
 * 
 */
static D32S __adm_irm_collectUcmLogFile(void)
{
    D32S iRet = ADM_ECODE_FAILED;
    D64S startTime = 0;
    D64S lTimeStamp = 0;
    DCHAR zipFileName[128] = {0};
    DCHAR pkiLogName[128] = {0};
    
    do {
        //!< 校验初始化变量
        if (NULL == g_initInfo) {
            iRet = ADM_ECODE_INVALID_PARAM;
            log_e("log file report module init falied, [err:%d]", iRet);
            break;
        }
        
        //!< 日志文件名默认为： admfotaapp_timestamp.zip
        (void)adm_memset(zipFileName, 0x00, sizeof(zipFileName), sizeof(zipFileName));
        lTimeStamp = time(NULL);
        (void)adm_snprintf(zipFileName, sizeof(zipFileName), "%s_%lld.zip", ADM_IRM_UCM_LOG_FILE_PREFIX, lTimeStamp);

        //!< 收集主控的日志文件，将主控的日志文件打包到 collect 目录下
        if (0 != adm_com_zipFile(g_initInfo->logStorageDir, g_initInfo->logCollectDir, zipFileName)) {
            log_e("adm_com_zipFile logStorageDir:%s, logCollectDir:%s, zipFileName:%s failed", g_initInfo->logStorageDir, g_initInfo->logCollectDir, zipFileName);
            break;
        }

#ifdef SUPPORT_LION_PKI
        const DCHAR* dirStr = adm_cfg_iniParserGetString(ADM_CFG_VUS_PKI_DIR, ADM_CFG_VUS_PKI_DIR_DEFAULT_VAL);

        //!< 日志文件名默认为： pki_timestamp.zip
        (void)adm_memset(zipFileName, 0x00, sizeof(zipFileName), sizeof(zipFileName));
        lTimeStamp = time(NULL);
        (void)adm_snprintf(zipFileName, sizeof(zipFileName), "%s_%lld.zip", ADM_IRM_PKI_LOG_FILE_PREFIX, lTimeStamp);

        adm_snprintf(pkiLogName,sizeof(pkiLogName),"%s/file/",dirStr);

        //!< 收集主控的日志文件，将主控的日志文件打包到 collect 目录下
        if (0 != adm_com_zipFile(pkiLogName, g_initInfo->logCollectDir, zipFileName)) {
            log_e("adm_com_zipFile logStorageDir:%s, logCollectDir:%s, zipFileName:%s failed", pkiLogName, g_initInfo->logCollectDir, zipFileName);
            break;
        }
#endif
        //!< remove UCM日志存储目录下的所有， xxx/admota/log/ucm/storage/ 日志文件
        //__adm_irm_removeLogFiles(g_initInfo->logStorageDir, ADM_IRM_REMOVE_FILE_IGNORE_NAME);

        //!< 将日志记录的startTime，备份给到reportStartTime，写入到kv中
        if (TRUE == __adm_irm_logFileTimeGet(g_logFileStartTimeKvKey, &startTime)) {
            (void)__adm_irm_logFileTimeSet(g_logFileReportStartTimeKvKey, startTime);
        }
        //!< 将日志记录的endtime，写入打kv中
        lTimeStamp = time(NULL);
        (void)__adm_irm_logFileTimeSet(g_logFileReportEndTimeKvKey, lTimeStamp);

        //!< 更新startTime
        lTimeStamp = time(NULL);
        (void)__adm_irm_logFileTimeSet(g_logFileStartTimeKvKey, lTimeStamp);

        iRet = ADM_ECODE_SUCCESSED;

    } while(FALSE);
    return iRet;
}

/**
 * @brief 收集UCM进程的APK日志信息
 * 
 */
static D32S irmCollectAPKLogFile(void)
{
    D32S iRet = ADM_ECODE_FAILED;
    D64S iTimeStamp = 0;
    DCHAR acZipFileName[ADM_IRM_FILENAME_LENGTH] = {0};
    const DCHAR *pcApkLogStorageDir = NULL;
    
    do 
    {
        //!< 校验初始化变量
        if (NULL == g_initInfo) 
        {
            iRet = ADM_ECODE_INVALID_PARAM;
            log_e("log file report module init falied, [err:%d]", iRet);
            break;
        }
        
        //!< apk日志文件名默认为： apk_timestamp.zip
        (void)adm_memset(acZipFileName, 0x00, sizeof(acZipFileName), sizeof(acZipFileName));
        iTimeStamp = time(NULL);
        (void)adm_snprintf(acZipFileName, sizeof(acZipFileName), "%s_%lld.zip", ADM_IRM_APK_LOG_FILE_PREFIX, iTimeStamp);
        //!< 从ucmconfig.ini配置文件中获取APK日志文件存储目录
        pcApkLogStorageDir = adm_cfg_iniParserGetString(ADM_CFG_VUS_APK_LOG_STORAGE_DIR, ADM_CFG_VUS_APK_LOG_STORAGE_DIR_DEFAULT_VAL);
        if(adm_com_isDirExist(pcApkLogStorageDir))
        {
            //!< 收集主控的apk日志文件，将主控的apk日志文件打包到 collect 目录下
            if (0 != adm_com_zipFile(pcApkLogStorageDir, g_initInfo->logCollectDir, acZipFileName))
            {
                log_e("adm_com_zipFile apkLogStorageDir:%s, logCollectDir:%s, zipFileName:%s failed", pcApkLogStorageDir, g_initInfo->logCollectDir, acZipFileName);
                break;
            }
        }
        else
        {
            log_i("%s: APK log storage dir not exist", pcApkLogStorageDir);
            return ADM_ECODE_SUCCESSED;
        }
        //!< remove APK日志存储目录下xxx/cache/logger/所有除了主日志的文件
        //__adm_irm_removeLogFiles(pcApkLogStorageDir, ADM_IRM_APK_REMOVE_FILE_IGNORE_NAME);
        
        iRet = ADM_ECODE_SUCCESSED;
    } while(FALSE);

    return iRet;
}

/**
 * @brief 收集日志到Collect目录
 * 
 */
static DBOOL __adm_irm_collectLogFile(void)
{
    DBOOL isCollectOk = FALSE;


    /*收集日志前先清空日志收集目录，防止出现程序频繁重启导致多次向从控发起日志采集请求
      但是没有执行到日志打包阶段，导致不断采集日志占满车机磁盘空间*/
    __adm_irm_removeLogFiles(g_initInfo->logCollectDir, NULL);

    do
    {
        //!< 先采集SYSI的日志
        if (ADM_ECODE_SUCCESSED != __adm_irm_collectSysiLogFile()) {
            log_e("__adm_irm_collectSysiLogFile failed");
            //!< 没有采集到SYSI的日志不用退出，只有等UCM的日志也没有收集到，才退出
            // break;
        }

        //!< 采集APK的日志
        if(ADM_ECODE_SUCCESSED != irmCollectAPKLogFile())
        {
            log_e("__adm_irm_collectAPKLogFile failed");
            //!< 没有采集到APK的日志不用退出，只有等UCM的日志也没有收集到，才退出
            // break;
        }

        //!< 采集kv数据库文件
        if (ADM_ECODE_SUCCESSED != __adm_irm_collectKvStorage())
        {
            log_e("__adm_irm_collectKvStorage failed");
            // break;
        }

        //!< 再采集UCM的日志
        if (ADM_ECODE_SUCCESSED != __adm_irm_collectUcmLogFile()) {
            log_e("__adm_irm_collectUcmLogFile failed");
            break;
        }

        isCollectOk = TRUE;
        //!< 日志采集成功后，状态置为 ADM_IRM_LOG_FILE_STATE_COLLECT_END
        (void)__adm_irm_logFileStateSet(ADM_IRM_LOG_FILE_STATE_COLLECT_END);
    } while(FALSE);

    return isCollectOk;
}

/**
 * @brief 对打包后的日志文件执行加密动作
 * 
 * @return DBOOL 
 */
static DBOOL __adm_irm_logFileEncrypt(ADM_DIM_SOA_TRANS_FILE_TYPE_E eFileType)
{
    DBOOL isEncryptOK = FALSE;
    DCHAR* fullZipfileName = NULL;
    DCHAR zipFullZipfileName[256] = {0};
    DCHAR fullZipfileName_enc[256] = {0};
    DCHAR encZipFileName[128] = {0};
    D64S lTimeStamp = 0;
    D32U len = 0;
    D32S ret = ADM_ECODE_FAILED;
    ADM_KV_STORAGE_VALUE_TYPE_E eValType = ADM_KV_STORAGE_VALUE_TYPE_INVALID;
    size_t sercretLen = 0;

    do {
        //!< 从KV中获取注册响应secret字段作为加密秘钥
        if(FALSE == adm_com_kvstorage_get_string(adm_core_service_getBusinessDatabaseDb(),
                                                 g_LogReportEncryptsecert,
                                                 &g_initInfo->secret, 
                                                 &sercretLen,
                                                 &eValType))
        {
            log_e("get register resp %s failed", g_LogReportEncryptsecert);
            g_initInfo->secret= NULL;
            break;
        }
        //!> g_initInfo->secretLen变量在加密时没有起到实际作用，这里暂时就赋值为0
        g_initInfo->segmentEncLen = 0;
        g_initInfo->secretLen = (D32U)sercretLen;
        g_initInfo->fileEncCb = FILE_AES_Encrypt;
                
        //!< 获取压缩后的日志文件
        // if (ADM_DIM_SOA_TRANS_FILE_TYPE_KEYLOG == eFileType)
        // {
        //     (void)adm_memset(zipFullZipfileName, 0x00, sizeof(zipFullZipfileName), sizeof(zipFullZipfileName));
        //     len = adm_strlen(g_initInfo->logPackDir);
        //     if (0 != len ) 
        //     {
        //         if ('/' == g_initInfo->logPackDir[len - (D32U)1]) {
        //             (void)adm_snprintf(zipFullZipfileName, sizeof(zipFullZipfileName), "%s%s", g_initInfo->logPackDir, g_zipFileName);
        //         } else {
        //             (void)adm_snprintf(zipFullZipfileName, sizeof(zipFullZipfileName), "%s/%s", g_initInfo->logPackDir, g_zipFileName);
        //         }
        //     }
        //     adm_free(g_zipFileName);
        //     g_zipFileName = adm_strdup(zipFullZipfileName);
        // }
        if (ADM_DIM_SOA_TRANS_FILE_TYPE_LOG == eFileType)
        {
            if (FALSE == __adm_irm_logFilePackNameGet(&fullZipfileName)) 
            {
                log_e("__adm_irm_logFilePackNameGet failed");
                break;
            }
        }

        //!< 拼接解密后的日志文件名, log_enc_timestamp.zip
        (void)adm_memset(encZipFileName, 0x00, sizeof(encZipFileName), sizeof(encZipFileName));
        lTimeStamp = time(NULL);
        if (ADM_DIM_SOA_TRANS_FILE_TYPE_KEYLOG == eFileType)
        {
            (void)adm_snprintf(encZipFileName, sizeof(encZipFileName), "%s_%s_%lld.zip", ADM_IRM_KEY_LOG_FILE_PREFIX, ADM_IRM_ENC_LOG_FILE_SUFFIX, lTimeStamp);
        }
        else
        {
            (void)adm_snprintf(encZipFileName, sizeof(encZipFileName), "%s_%s_%lld.zip", ADM_IRM_LOG_FILE_PREFIX, ADM_IRM_ENC_LOG_FILE_SUFFIX, lTimeStamp);
        }

        //!< 加密后输出的日志文件全路径信息
        (void)adm_memset(fullZipfileName_enc, 0x00, sizeof(fullZipfileName_enc), sizeof(fullZipfileName_enc));
        len = adm_strlen(g_initInfo->logPackDir);
        if (0 != len ) {
            if ('/' == g_initInfo->logPackDir[len - (D32U)1]) {
                (void)adm_snprintf(fullZipfileName_enc, sizeof(fullZipfileName_enc), "%s%s", g_initInfo->logPackDir, encZipFileName);
            } else {
                (void)adm_snprintf(fullZipfileName_enc, sizeof(fullZipfileName_enc), "%s/%s", g_initInfo->logPackDir, encZipFileName);
            }
        }
        else {
            log_e("get log Pack Dir length is error");
        }

        if (ADM_DIM_SOA_TRANS_FILE_TYPE_KEYLOG == eFileType)
        {
            //!< 执行加密动作
            ret = g_initInfo->fileEncCb(g_zipFileName, g_initInfo->secret, fullZipfileName_enc);
            if (ADM_ECODE_SUCCESSED != ret) {
                log_e("fileEncCb failed, [err:%d]", ret);
                break;
            }
        }
        else
        {
            //!< 执行加密动作
            ret = g_initInfo->fileEncCb(fullZipfileName, g_initInfo->secret, fullZipfileName_enc);
            if (ADM_ECODE_SUCCESSED != ret) {
                log_e("fileEncCb failed, [err:%d]", ret);
                break;
            }
        }

        //!< 日志加密后删除pack目录下原日志打包文件， xxx/admota/ucm/log/pack/log_timestamp.zip
        if (ADM_DIM_SOA_TRANS_FILE_TYPE_KEYLOG == eFileType)
        {
            //TODO: 临时注释，暂时不删除该文件后续验证完成，关闭此功能 (void)adm_fremove(g_zipFileName);

            adm_free(g_zipFileName);
            g_zipFileName = adm_strdup(fullZipfileName_enc);
            log_i("g_zipFileName:%s",g_zipFileName);
        }
        else
        {
            //!< 将加密后的日志全路径存储到kv中
            if (FALSE == __adm_irm_logFilePackNameSet(fullZipfileName_enc)) {
                log_e("__adm_irm_logFilePackNameSet set fullZipfileName_enc:%s failed", fullZipfileName_enc);
                break;
            }

            (void)adm_fremove(fullZipfileName);
        }
        
        isEncryptOK = TRUE;

    } while(FALSE);

    if (NULL != fullZipfileName) {
        adm_free(fullZipfileName);
    }

    return isEncryptOK;
}

/**
 * @brief 将收集到的日志打包到pack目录
 * 
 */
static DBOOL __adm_irm_packCollectLogFile(const DCHAR* zipFileName, ADM_DIM_SOA_TRANS_FILE_TYPE_E eFileType)
{
    DBOOL isPackOk = FALSE;
    DCHAR fullZipfileName[256] = {0};
    D32U len = 0;

    do {
        //!< 校验初始化变量
        if (NULL == g_initInfo || NULL == zipFileName) {
            log_e("Input param g_initInfo || zipFileName is nil");
            break;
        }

        //!< 校验日志初始化和反初始化接口
        if (NULL == g_initInfo->logPackDir || NULL == g_initInfo->logCollectDir) {
            log_e("logPackDir || logCollectDir is nil");
            break;
        }

        //!< 压缩文件
        if (0 != adm_com_zipFile(g_initInfo->logCollectDir, g_initInfo->logPackDir, zipFileName)) {
            log_e("adm_com_zipFile logCollectDir:%s, logPackDir:%s, zipFileName:%s failed", g_initInfo->logCollectDir, g_initInfo->logPackDir, zipFileName);
            break;
        }

        //!< 存储到日志采集的目录
        (void)adm_memset(fullZipfileName, 0x00, sizeof(fullZipfileName), sizeof(fullZipfileName));
        len = adm_strlen(g_initInfo->logPackDir);
        if (0 != len ) {
            if ('/' == g_initInfo->logPackDir[len - (D32U)1]) {
                (void)adm_snprintf(fullZipfileName, sizeof(fullZipfileName), "%s%s", g_initInfo->logPackDir, zipFileName);
            } else {
                (void)adm_snprintf(fullZipfileName, sizeof(fullZipfileName), "%s/%s", g_initInfo->logPackDir, zipFileName);
            }
        }
        else {
            log_e("get log Pack Dir length is error");
        }

        if(ADM_DIM_SOA_TRANS_FILE_TYPE_KEYLOG == eFileType) /**keylog*/
        {
            adm_free(g_zipFileName);
            g_zipFileName = adm_strdup(fullZipfileName);
            log_i("g_zipFileName:%s",g_zipFileName);
        }
        else
        {
            if (FALSE == __adm_irm_logFilePackNameSet(fullZipfileName)) 
            {
            log_e("__adm_irm_logFilePackNameSet set fullZipfileName:%s failed", fullZipfileName);
            break;
            }
            log_i("__adm_irm_logFilePackNameSet set fullZipfileName:%s successed", fullZipfileName);
        }
        isPackOk = TRUE;
    } while(FALSE);

    return isPackOk;
}

static DBOOL __adm_irm_packLogFile(ADM_DIM_SOA_TRANS_FILE_TYPE_E eFileType)
{
    DBOOL isPackOk = FALSE;
    DCHAR zipFileName[128] = {0};
    D64S lTimeStamp = 0;

    do {
        //!< 日志文件名默认为： log_timestamp.zip
        (void)adm_memset(zipFileName, 0x00, sizeof(zipFileName), sizeof(zipFileName));
        lTimeStamp = time(NULL);
        if (ADM_DIM_SOA_TRANS_FILE_TYPE_KEYLOG == eFileType)
        {
            (void)adm_snprintf(zipFileName, sizeof(zipFileName), "%s_%lld.zip", ADM_IRM_KEY_LOG_FILE_PREFIX, lTimeStamp);
            adm_free(g_zipFileName);
            g_zipFileName = adm_strdup(zipFileName);
        }
        else
        {
            (void)adm_snprintf(zipFileName, sizeof(zipFileName), "%s_%lld.zip", ADM_IRM_LOG_FILE_PREFIX, lTimeStamp);
        }
        
        //!< 将收集到的日志打包到 xxx/admota/ucm/log/pack/ 目录下，命名为 log_timestamp.zip
        if (FALSE == __adm_irm_packCollectLogFile(zipFileName, eFileType)) {
            log_e("__adm_irm_packCollectLogFile failed");
            break;
        }

        /**keylog不进行加密*/
        if (ADM_DIM_SOA_TRANS_FILE_TYPE_KEYLOG != eFileType)
        {
            //!< 判断文件是否需要加密，如果需要加密，执行对文件加密
            if (ADM_IRM_LOG_FILE_ENCRYPT_REPORT == g_initInfo->isEncrypt) {
                if (DTRUE == g_isEmsCollect)
                {
                    log_i("Engineer Mode collect log, do not need to encrypt");
                }
                else
                {
                    (void)__adm_irm_logFileEncrypt(eFileType);
                }
                //TODO: 加密失败，暂不作为必选项
                // break;
            }
        }

        //!< 打包完成删除collect目录下的所有日志文件， xxx/admota/ucm/log/collect/
        __adm_irm_removeLogFiles(g_initInfo->logCollectDir, NULL);

        //!< 日志打包成功，状态置为 ADM_IRM_LOG_FILE_STATE_PACK_END
        if (ADM_DIM_SOA_TRANS_FILE_TYPE_KEYLOG != eFileType)
        {
            (void)__adm_irm_logFileStateSet(ADM_IRM_LOG_FILE_STATE_PACK_END);
        }
        
        isPackOk = TRUE;

    } while(FALSE);

    return isPackOk;
}

/**
 * @brief 获取文件大小
 * 
 * @param fullFileNamePath  [IN]        文件路径
 * @return D64S     -1: Failed
 */
static D64S __adm_irm_getFileSize(const DCHAR* fullFileNamePath)
{
    D64S fileSize = -1;
    struct stat sb;

    do {
        //!< 校验参数
        if (NULL == fullFileNamePath) {
            log_e("Input param fullFileNamePath is nil");
            break;
        }

        //!< 获取文件的属性
        if (-1 == lstat(fullFileNamePath, &sb)) {
            log_e("lstat %s failed", fullFileNamePath);
            break;
        }

        fileSize = sb.st_size;
        log_i("filesize of %s is %lld", fullFileNamePath, fileSize);

    } while(FALSE);

    return fileSize;
}

//1. startTime 什么时候更新呢？日志采集之后，deinit 重新set了startTime，会跟上报的时间冲突
//2. 记录一个日志startTime，在日志初始化的时候记录。  Collect的时候，备份此时间，上报时，用备份的时间，
    // - 上报时，使用备份的startTime；
    // - 上报完成后，删除备份的startTime；   ---- 可以不用删除，每次更新即可
//3. 记录一个日志endTime
    // - 收集日志的时候（deintLog），记录endTime
    // - 上报完成后，删除此endTime；         ---- 可以不用删除，每次更新即可

/**
 * @brief 将收集到的日志文件上报到云端
 * 
 */
static DBOOL __adm_irm_reportLogFile(const ADM_IRM_LOG_FILE_TG_REPORT_INFO_T* tgReportInfo)
{
    DBOOL isReportOk = FALSE;
    DCHAR* fullZipfileName = NULL;
    DCHAR* sessionId = NULL;
    D64S taskId = 0;
    D64S strategyId = 0;
    D64S lTimeStamp = 0;
    D64S startTime = 0;
    D64S endTime = 0;
    DCHAR fileHash[256] = {0};
    D64S fileSize = 0;
    D64S lPeriodicTimeInterval = 0;

    do {
        //!< 校验参数
        if (NULL == g_initInfo->getTaskIdCb || NULL == g_initInfo->getStrategyIdCb 
            || NULL == tgReportInfo || NULL == g_initInfo->getSdkVerCb) {
            log_e("Input param getTaskIdCb || getStrategyIdCb || tgReportInfo || getSdkVerCb is nil");
            break;
        }

        //!< 读取kv中保存的日志压缩文件路径信息
        if (FALSE == __adm_irm_logFilePackNameGet(&fullZipfileName)) {
            log_e("__adm_irm_logFilePackNameGet failed");
            break;
        }
        //!< 读取kv中保存的startTime信息, 非必填
        (void)__adm_irm_logFileTimeGet(g_logFileReportStartTimeKvKey, &startTime);

        //!< 读取kv中保存的endTime信息，非必填
        (void)__adm_irm_logFileTimeGet(g_logFileReportEndTimeKvKey, &endTime);

        //!< 生成日志上报需要的各种信息
        (void)g_initInfo->getSessionIdCb(&sessionId);
        (void)g_initInfo->getTaskIdCb(&taskId);
        (void)g_initInfo->getStrategyIdCb(&strategyId);
        lTimeStamp = time(NULL);
        adm_com_sha256File(fullZipfileName, fileHash, (D32U)(sizeof(fileHash) / sizeof(fileHash[0])));
        fileSize = __adm_irm_getFileSize(fullZipfileName);

        //!< 调用日志上报接口，触发日志上报
        if (ADM_ECODE_SUCCESSED != adm_irm_reportFile(sessionId, 
                taskId,
                strategyId,
                lTimeStamp,
                startTime,
                endTime,
                g_initInfo->getSdkVerCb(),
                fullZipfileName,
                (ADM_OCS_UPLOAD_LOGFILE_ENCRYPTED_T)g_initInfo->isEncrypt,
                fileHash,
                fileSize,
                ADM_OCS_UCLOG_FILE,
                tgReportInfo->cmdId,
                tgReportInfo->pushId,
                tgReportInfo->extsReqJsonStr)) {
            log_e("adm_irm_reportFile failed");
            break;
        }

        isReportOk = TRUE;
        //!< 日志上报调用成功后，状态置为 ADM_IRM_LOG_FILE_STATE_IDLE
        (void)__adm_irm_logFileStateSet(ADM_IRM_LOG_FILE_STATE_IDLE);
        // //!< 上报接口会rename文件，所以触发上报后，就可删除我的压缩包
        // adm_com_rmFile(fullZipfileName);
        // //!< 日志上报调用成功后，清除保存的待上报的日志压缩文件路径
        // __adm_irm_logFilePackNameDel();
        // //!< 日志上报调用成功后，清除保存在KV中的startTime字段
        // __adm_irm_logFileTimeDel();
        //!< 获取周期上报的间隔时间
        (void)__adm_irm_logFileTimeGet(g_logFileReportPeriodicTimeKvKey, &lPeriodicTimeInterval);
        //!< 更新下次触发上报的时间点
        (void)__adm_irm_logFileTimeSet(g_logFileNextReportPeriodicTimeKvKey, lPeriodicTimeInterval + time(NULL));
    } while(FALSE);

    if (NULL != sessionId) {
        adm_free(sessionId);
    }
    if (NULL != fullZipfileName) {
        adm_free(fullZipfileName);
    }

    return isReportOk;
}
/*************************************************************日志文件处理***********************************************************************/

/**
 * @brief 将收集到的日志文件上报到云端
 * 
 */
static DBOOL __adm_irm_reportKeyLogFile(const ADM_IRM_LOG_FILE_TG_REPORT_INFO_T* tgReportInfo)
{
    DBOOL isReportOk = FALSE;
    DCHAR* sessionId = NULL;
    D64S taskId = 0;
    D64S strategyId = 0;
    D64S lTimeStamp = 0;
    D64S startTime = 0;
    D64S endTime = 0;
    DCHAR fileHash[256] = {0};
    D64S fileSize = 0;

    do {
        //!< 校验参数
        if (NULL == g_initInfo->getTaskIdCb || NULL == g_initInfo->getStrategyIdCb 
            || NULL == tgReportInfo || NULL == g_initInfo->getSdkVerCb) {
            log_e("Input param getTaskIdCb || getStrategyIdCb || tgReportInfo || getSdkVerCb is nil");
            break;
        }

        //!< 生成日志上报需要的各种信息
        (void)g_initInfo->getSessionIdCb(&sessionId);
        (void)g_initInfo->getTaskIdCb(&taskId);
        (void)g_initInfo->getStrategyIdCb(&strategyId);
        lTimeStamp = time(NULL);
        adm_com_sha256File(g_zipFileName, fileHash, (D32U)(sizeof(fileHash) / sizeof(fileHash[0])));
        fileSize = __adm_irm_getFileSize(g_zipFileName);

        //!< 调用日志上报接口，触发日志上报
        if (ADM_ECODE_SUCCESSED != adm_irm_reportFile(sessionId, 
                taskId,
                strategyId,
                lTimeStamp,
                startTime,
                endTime,
                g_initInfo->getSdkVerCb(),
                g_zipFileName,
                (ADM_OCS_UPLOAD_LOGFILE_ENCRYPTED_T)g_initInfo->isEncrypt,
                fileHash,
                fileSize,
                ADM_OCS_UCKEYLOG_FILE,
                tgReportInfo->cmdId,
                tgReportInfo->pushId,
                tgReportInfo->extsReqJsonStr)) {
            log_e("adm_irm_reportFile failed");
            break;
        }

        isReportOk = TRUE;

    } while(FALSE);

    if (NULL != sessionId) {
        adm_free(sessionId);
    }

    return isReportOk;
}


/*************************************************************状态处理***********************************************************************/
/**
 * @brief 处理日志文件上报的状态，状态值 = 空闲
 * 
 * @return DBOOL    TRUE:Successed, FALSE: Failed
 */
static DBOOL __adm_irm_logFileStateDealIdle(void)
{
    DBOOL isDealOk = FALSE;

    do {
    
        //!< 日志开始采集前，状态置为开始收集 ADM_IRM_LOG_FILE_STATE_COLLECT_START
        (void)__adm_irm_logFileStateSet(ADM_IRM_LOG_FILE_STATE_COLLECT_START);

        //!< 先收集
        if (FALSE == __adm_irm_collectLogFile()) {
            log_e("__adm_irm_collectLogFile failed");
            break;
        }

        //!< 再打包
        if (FALSE == __adm_irm_packLogFile(ADM_DIM_SOA_TRANS_FILE_TYPE_LOG)) {
            log_e("__adm_irm_packLogFile failed，state is reset to idle");
            (void)__adm_irm_logFileStateSet(ADM_IRM_LOG_FILE_STATE_IDLE);
            break;
        }

        isDealOk = TRUE;
        log_i("__adm_irm_logFileStateDealIdle successed");
    } while(FALSE);

    return isDealOk;
}

/**
 * @brief 处理日志文件上报的状态，状态值 = 采集完成
 * 
 * @return DBOOL    TRUE:Successed, FALSE: Failed
 */
static DBOOL __adm_irm_logFileStateDealCollectEnd(void)
{
    DBOOL isDealOk = FALSE;

    do {
        //!< 已收集完成，直接打包
        if (FALSE == __adm_irm_packLogFile(ADM_DIM_SOA_TRANS_FILE_TYPE_LOG)) {
            log_e("__adm_irm_packLogFile failed，state is reset to idle");
            (void)__adm_irm_logFileStateSet(ADM_IRM_LOG_FILE_STATE_IDLE);
            break;
        }

        isDealOk = TRUE;
        log_i("__adm_irm_logFileStateDealCollectEnd successed");
    } while(FALSE);

    return isDealOk;
}

/**
 * @brief 处理日志上报的状态
 * 
 * @return DBOOL    TRUE: 处理成功， FALSE: 处理失败
 */
static DBOOL __adm_irm_logFileStateDeal(void)
{
    DBOOL isDealOk = FALSE;
    ADM_IRM_LOG_FILE_STATE_E eState = ADM_IRM_LOG_FILE_STATE_INVALID;


    do {
        //!< 获取日志的状态
        if (FALSE == __adm_irm_logFileStateGet(&eState)) {
            log_e("adm_irm_logFileStateGet failed");
            break;
        }

        log_i("eState is %d", eState);
        switch (eState)
        {
            case ADM_IRM_LOG_FILE_STATE_IDLE:
            case ADM_IRM_LOG_FILE_STATE_COLLECT_START:
                isDealOk = __adm_irm_logFileStateDealIdle();
                break;
            case ADM_IRM_LOG_FILE_STATE_COLLECT_END:
                isDealOk = __adm_irm_logFileStateDealCollectEnd();
                break;
            case ADM_IRM_LOG_FILE_STATE_PACK_END:
                log_i("ecuState:%d is pack end", eState);
                isDealOk = TRUE;
                break;
            case ADM_IRM_LOG_FILE_STATE_INVALID:
            default:
                log_w("eState:%d is invalid", eState);
                break; 
        }

    } while(FALSE);
    

    return isDealOk;
}
/*************************************************************状态处理***********************************************************************/

/*************************************************************执行上报***********************************************************************/

/**
 * @brief 用于定义触发日志文件上报需要的信息
 * 
 */
typedef struct {
    DBOOL isInited;                                         //!< 是否已经初始化
    ADM_IRM_LOG_FILE_TG_REPORT_INFO_T* tgReportInfo;        //!< 触发上报需要的信息
} ADM_IRM_LOG_FILE_REPORT_INFO_T;

static ADM_IRM_LOG_FILE_REPORT_INFO_T  g_reportInfo = {0};    //!< 全局变量，用于存储触发日志文件上报需要的信息

/**
 * @brief 用于初始化触发日志文件上报需要的信息
 * 
 * @param tgReportInfo      [IN]    传入的触发上报需要的信息
 * @return DBOOL            TRUE:Successed, FALSE: Failed
 */
static DBOOL __adm_irm_logFileReportInfoInit(const ADM_IRM_LOG_FILE_TG_REPORT_INFO_T* tgReportInfo)
{
    DBOOL isInitOk = FALSE;
    ADM_IRM_LOG_FILE_TG_REPORT_INFO_T* pstTriggerReportInfo = NULL;

    do {
         log_i("g_reportInfo.isInited = %d", g_reportInfo.isInited);
        //!< 校验是否已经初始化
        if (TRUE == g_reportInfo.isInited) {
            log_i("Log file report infomation already init");
            isInitOk = TRUE;
            break;
        }

        //!< 校验入参
        if (NULL == tgReportInfo) {
            log_e("Input param tgReprotInfo  is nil");
            break;
        }

        //!< 申请空间
        pstTriggerReportInfo = (ADM_IRM_LOG_FILE_TG_REPORT_INFO_T*)adm_malloc(sizeof(ADM_IRM_LOG_FILE_TG_REPORT_INFO_T));
        if (NULL == pstTriggerReportInfo) {
            log_e("Failed to malloc ADM_IRM_LOG_FILE_TG_REPORT_INFO_T");
            break;
        }

        //!< 填充数据
        (void)adm_memset(pstTriggerReportInfo, 0x00, sizeof(ADM_IRM_LOG_FILE_TG_REPORT_INFO_T), sizeof(ADM_IRM_LOG_FILE_TG_REPORT_INFO_T));
        pstTriggerReportInfo->cmdId = tgReportInfo->cmdId;
        if (NULL != tgReportInfo->extsReqJsonStr) {
            pstTriggerReportInfo->extsReqJsonStr = adm_strdup(tgReportInfo->extsReqJsonStr);
        }
        if (NULL != tgReportInfo->pushId) {
            pstTriggerReportInfo->pushId = adm_strdup(tgReportInfo->pushId);
        }
        pstTriggerReportInfo->iKeyLogFlag = tgReportInfo->iKeyLogFlag;

        g_reportInfo.tgReportInfo = pstTriggerReportInfo;
        g_reportInfo.isInited = TRUE;
        isInitOk = TRUE;
    } while(FALSE);

    return isInitOk;
}

/**
 * @brief 用于释放触发日志文件上报需要的信息
 * 
 */
static void __adm_irm_logFileReportInfoDeinit(void)
{
    if (NULL != g_reportInfo.tgReportInfo) {
        adm_free(g_reportInfo.tgReportInfo->extsReqJsonStr);
        adm_free(g_reportInfo.tgReportInfo->pushId);
        g_reportInfo.tgReportInfo->cmdId = ADM_OCS_CMDID_INVALID;
        adm_free(g_reportInfo.tgReportInfo);
        g_reportInfo.isInited = FALSE;
    }
}

static D32S adm_irm_cloneTgReportInfo(const ADM_IRM_LOG_FILE_TG_REPORT_INFO_T* pSrc,ADM_IRM_LOG_FILE_TG_REPORT_INFO_T** pDst)
{
    if (ADM_IS_NULL(pSrc))
    {
        log_e("invalid param");
        return ADM_ECODE_INVALID_PARAM;
    }

    log_d("adm_irm_cloneTgReportInfo begin");

    *pDst = (ADM_IRM_LOG_FILE_TG_REPORT_INFO_T *)adm_malloc(sizeof(ADM_IRM_LOG_FILE_TG_REPORT_INFO_T));
    if (ADM_IS_NULL(*pDst))
    {
        log_e(" malloc failed");
        return ADM_ECODE_APPLY_MEMORY_FAILED;
    }

    (*pDst)->cmdId = ADM_OCS_CMDID_INVALID;
    (*pDst)->extsReqJsonStr = NULL;
    (*pDst)->pushId = NULL;


    (*pDst)->cmdId = pSrc->cmdId;
    (*pDst)->iKeyLogFlag = pSrc->iKeyLogFlag;

    if (pSrc->extsReqJsonStr)
    {
        (*pDst)->extsReqJsonStr = adm_strdup(pSrc->extsReqJsonStr);
    }

    if (pSrc->pushId)
    {
        (*pDst)->pushId = adm_strdup(pSrc->pushId);
    }
    
    log_d("adm_irm_cloneTgReportInfo end");
        
    return ADM_ECODE_SUCCESSED;
}


static DVOID adm_irm_freeTgReportInfo(ADM_IRM_LOG_FILE_TG_REPORT_INFO_T* pSrc)
{
    if (NULL != pSrc)
    {   
        if (NULL != pSrc->extsReqJsonStr)
        {
            adm_free(pSrc->extsReqJsonStr);
        }

        if (NULL != pSrc->pushId)
        {
            adm_free(pSrc->pushId);
        }
            
        adm_free(pSrc);
    }
    
    return;
}

/**
 * @brief 日志收集和上报的处理函数
 * 
 * @param argv      [IN/OUT]    输入或输出参数
 * @return void*    默认返回NULL
 */
static void* __adm_irm_logFileReportThreadProc(void* argv)
{
    D32S errorCode = ADM_ECODE_FAILED;
    D32S ret = ADM_ECODE_FAILED;
    DBOOL bIsReport = DFALSE;

    ADM_IRM_LOG_FILE_TG_REPORT_INFO_T *tgReportInfo = (ADM_IRM_LOG_FILE_TG_REPORT_INFO_T *)argv;

    (void)logFileLock();

    //!< 判断数据是否已经初始化成功 初始化放在线程中防止其他线程任务结束释放后崩溃
    (void)__adm_irm_logFileReportInfoInit(tgReportInfo);

    do {
        //!< 开始处理状态
        if (FALSE == __adm_irm_logFileStateDeal()) {
            log_e("__adm_irm_logFileStateDeal failed");
            break;
        }

        //!< 执行上报
        if (FALSE == __adm_irm_reportLogFile(g_reportInfo.tgReportInfo)) {
            log_e("__adm_irm_reportLogFile failed");
            break;
        }

        bIsReport = adm_cfg_iniParserGetBoolean(ADM_CFG_VUS_IS_REPORT_KEYLOG, ADM_CFG_VUS_IS_REPORT_KEYLOG_DEFAULT_VAL);
        if (DTRUE == bIsReport)
        {
            if (NULL != g_reportInfo.tgReportInfo)
            {
                if (1 == g_reportInfo.tgReportInfo->iKeyLogFlag)
                {
                    log_d("not need to report keylog file");
                    break;
                }
            }

            if (DTRUE == adm_irm_collectKeyLogFileDeal())
            {
                /**删除所有的keylog文件*/
                adm_irm_rmAllKeyLogFile();

                if (FALSE == __adm_irm_reportKeyLogFile(g_reportInfo.tgReportInfo))
                {
                    log_e("__adm_irm_reportLogFile failed");
                    break;
                }
            }
            else
            {
                log_e("adm_irm_collectKeyLogFileDeal failed");
            }
        }
    } while(FALSE);

    if (NULL != g_reportInfo.tgReportInfo && ADM_OCS_CMDID_TSP == g_reportInfo.tgReportInfo->cmdId)
    {
        //!< 无论设置是否成功，都需要调用TBOX进入休眠的接口
        ret = adm_dim_host_execOtaTask(adm_soa_getHostTboxInstanceId(), 5, ADM_DIM_SOA_HOST_OTA_TASK_TYPE_STOP_WAKE_UP, 0, &errorCode);
        ret = (ret == ADM_ECODE_SUCCESSED) ? errorCode : ret;
        log_i("TSP Ota Log Report finish, stop wakeup, iRet[%d], errorCode[%d]", ret, errorCode);
    }
    
    __adm_irm_logFileReportInfoDeinit();
    adm_irm_freeTgReportInfo(tgReportInfo);
    (void)logFileUnlock();

    return NULL;
}

/**
 * @brief 上报日志文件到云端
 * @note  如果某次日志文件未上报成功，需要保存在本地，重试上报
 * 
 * @param tgReportInfo  [IN]    触发上报需要传入的信息
 * 
 */
void adm_irm_triggerLogFileReport(const ADM_IRM_LOG_FILE_TG_REPORT_INFO_T* tgReportInfo)
{
    pthread_t threadId = 0;
    ADM_IRM_LOG_FILE_TG_REPORT_INFO_T * pTgReportInfo = NULL;

    if (ADM_ECODE_SUCCESSED !=  adm_irm_cloneTgReportInfo(tgReportInfo,&pTgReportInfo))
    {
        log_d("adm_irm_cloneTgReportInfo failed");
        return ;
    }

    do {
        if (0 == pthread_create(&threadId, NULL, __adm_irm_logFileReportThreadProc, (DVOID *)pTgReportInfo)) {
            log_i("pthread_create successed");
            (void)pthread_detach(threadId);
        } else {
            adm_irm_freeTgReportInfo(pTgReportInfo);
            log_e("pthread_create failed");
        }
    } while(FALSE);

    return;
}

#define USB_COLLECT_LOG_DIR "/storage/usb0"
D32S adm_irm_triggerLogCollect(const ADM_IRM_LOG_FILE_TG_REPORT_INFO_T* tgReportInfo)
{
    DCHAR* fullZipfileName = DNULL;
    DCHAR newFullFilePath[128] = {0};
    const DCHAR* cfgFullDir = DNULL;
    D32S ret = ADM_ECODE_FAILED;

    //!< 判断数据是否已经初始化成功
    (void)__adm_irm_logFileReportInfoInit(tgReportInfo);
    
    (void)logFileLock();
    g_isEmsCollect = TRUE;
    do {
        //!< 开始处理状态
        if (FALSE == __adm_irm_logFileStateDeal()) {
            log_e("__adm_irm_logFileStateDeal failed");
            break;
        }

        //!< 读取kv中保存的日志压缩文件路径信息
        if (FALSE == __adm_irm_logFilePackNameGet(&fullZipfileName)) {
            log_e("__adm_irm_logFilePackNameGet failed");
            break;
        }

        //!< 拼接路径，修改文件名
        cfgFullDir = adm_cfg_iniParserGetString(ADM_CFG_EMS_LOG_FILE_FULL_DIR, ADM_CFG_EMS_LOG_FILE_FULL_DIR_DEFAULT_VAL);
        if (DFALSE == adm_com_isDirExist(cfgFullDir))
        {
            adm_com_createDir(cfgFullDir);
        }
        adm_memset(newFullFilePath, 0x00, sizeof(newFullFilePath), sizeof(newFullFilePath));
        adm_snprintf(newFullFilePath, sizeof(newFullFilePath), "%s/%s", cfgFullDir, ADM_ENGINEER_MODE_LOG_COLLECT_DEFAULT_NAME);
        rename(fullZipfileName, newFullFilePath);

        log_i("rename fullZipFileName:%s to newFullFilePaht:%s", fullZipfileName, newFullFilePath);

        /** 日志拷贝到U盘收集的路径 */
        if(DFALSE == adm_com_isDirExist(USB_COLLECT_LOG_DIR))
        {
            log_w("USB collect path[%s] not exits.", USB_COLLECT_LOG_DIR);
        }
        else 
        {
            ret = adm_com_copyFile(newFullFilePath, USB_COLLECT_LOG_DIR, ADM_ENGINEER_MODE_LOG_COLLECT_DEFAULT_NAME);

            log_i("Copy file to Usb result[%d]", ret);
        }

        //!< 状态置为 IDLE 
        (void)__adm_irm_logFileStateSet(ADM_IRM_LOG_FILE_STATE_IDLE);
        
        adm_free(fullZipfileName);

        /**采集keylog*/
        if (FALSE == adm_irm_collectKeyLogFileDeal())
        {
            log_e("adm_irm_collectKeyLogFileDeal failed");
            break;
        }

        //!< 拼接路径，修改文件名
        if (DFALSE == adm_com_isDirExist(cfgFullDir))
        {
            adm_com_createDir(cfgFullDir);
        }
        adm_memset(newFullFilePath, 0x00, sizeof(newFullFilePath), sizeof(newFullFilePath));
        adm_snprintf(newFullFilePath, sizeof(newFullFilePath), "%s/%s", cfgFullDir, ADM_ENGINEER_MODE_KEYLOG_COLLECT_DEFAULT_NAME);
        rename(g_zipFileName, newFullFilePath);

        log_i("rename fullZipFileName:%s to newFullFilePaht:%s", g_zipFileName, newFullFilePath);

        /** 日志拷贝到U盘收集的路径 */
        if(DFALSE == adm_com_isDirExist(USB_COLLECT_LOG_DIR))
        {
            log_w("USB collect path[%s] not exits.", USB_COLLECT_LOG_DIR);
        }
        else 
        {
            ret = adm_com_copyFile(newFullFilePath, USB_COLLECT_LOG_DIR, ADM_ENGINEER_MODE_KEYLOG_COLLECT_DEFAULT_NAME);

            log_i("Copy file to Usb result[%d]", ret);
        }

    } while(FALSE);
    
    g_isEmsCollect = FALSE;
    (void)logFileUnlock();
    __adm_irm_logFileReportInfoDeinit();

    return ret;

}

/*************************************************************初始化和反初始化***********************************************************************/
/**
 * @brief 初始化日志文件模块的状态
 * 
 * @return  TRUE: 初始化成功, FALSE: 初始化失败
 */
static DBOOL __adm_irm_logFileStateInit(void)
{
    ADM_IRM_LOG_FILE_STATE_E eState = ADM_IRM_LOG_FILE_STATE_INVALID;
    DBOOL isInitOk = FALSE;
    ADM_IRM_LOG_FILE_TG_REPORT_INFO_T stgReportInfo = {0};
    (void)adm_memset(&stgReportInfo, 0x00, sizeof(ADM_IRM_LOG_FILE_TG_REPORT_INFO_T), sizeof(ADM_IRM_LOG_FILE_TG_REPORT_INFO_T));
    stgReportInfo.cmdId = ADM_OCS_CMDID_CONFIG;
    stgReportInfo.extsReqJsonStr = NULL;
    stgReportInfo.pushId = NULL;
    stgReportInfo.iKeyLogFlag = 0;

    do {
        //!< 获取成功，状态初始化成功
        if (TRUE == __adm_irm_logFileStateGet(&eState)) {
            log_i("eState = %d", eState);
            isInitOk = TRUE;
            break;
        }

        //!< 获取失败，状态初始化为IDLE
        eState = ADM_IRM_LOG_FILE_STATE_IDLE;
        if (FALSE == __adm_irm_logFileStateSet(eState)) {
            log_i("eState set to %d failed", eState);
            break;
        }

        isInitOk = TRUE;
    } while(FALSE);
    
    if (ADM_IRM_LOG_FILE_STATE_IDLE != eState) 
    {
        log_i("LogReport resume, current log state is :%d", eState);
        log_i("sleep 20s to wait sysi4uclstart!");
        (void)sleep((D32U)ADM_IRM_LOGSTATE_INIT_WAIT_SYSI_TIME);
        adm_irm_triggerLogFileReport(&stgReportInfo);

    }

    return isInitOk;
}

/**
 * @brief 线程处理函数，处理日志上报的周期
 * 
 * @param argv      [IN/OUT]    默认为NULL
 * @return void*    NULL
 */
static void* __adm_irm_logFilePeriodicReportThreadProc(void* argv)
{
    D64S lSavedTimestamp = 0;
    D64S lCurrTimestamp = 0;
    D64S lPeriodicTimeInterval = 0;
    D32S i = 0;
    ADM_IRM_LOG_FILE_TG_REPORT_INFO_T tgReportInfo = {0};
    
    do {
        //!< 获取触发下次上报的时间点
        (void)__adm_irm_logFileTimeGet(g_logFileReportPeriodicTimeKvKey, &lPeriodicTimeInterval);

        while(TRUE) {
            (void)__adm_irm_logFileTimeGet(g_logFileNextReportPeriodicTimeKvKey, &lSavedTimestamp);
            lCurrTimestamp = time(NULL);
            //!< 跟当前的时间戳比较
            if (lCurrTimestamp >= lSavedTimestamp) {
                i++;        //!< 为防止时间戳误判，次数超过5次后，触发上报
            }

            log_i("currentTime:%lld lSavedTimestamp:%lld", lCurrTimestamp, lSavedTimestamp);
            if (i >= 5) {
                //!< 移动更新周期上报的时间到触发之后，产品建议，每次触发后，周期的时间都需要更新
                //!< 触发上报
                i = 0;
                (void)adm_memset(&tgReportInfo, 0x00, sizeof(ADM_IRM_LOG_FILE_TG_REPORT_INFO_T), sizeof(ADM_IRM_LOG_FILE_TG_REPORT_INFO_T));
                tgReportInfo.cmdId = ADM_OCS_CMDID_CONFIG;
                tgReportInfo.extsReqJsonStr = NULL;
                tgReportInfo.pushId = NULL;
                tgReportInfo.iKeyLogFlag = 0;
                adm_irm_triggerLogFileReport(&tgReportInfo);
            }

            (void)sleep(600);   //!< 每次默认sleep 60 * 10 秒
        }
        
    } while(FALSE);

//    return NULL;
}

/**
 * @brief 创建线程处理周期上报日志
 * 
 */
static void __adm_irm_logFileReportPeriodicReportDeal(void)
{
    static DBOOL isDoing = FALSE;
    pthread_t threadId = 0;

    //!< 处理当前的周期
    do {
        if (FALSE == isDoing) {
            __adm_irm_logFilePeriodicTimeUpdate();

            if (0 == pthread_create(&threadId, NULL, __adm_irm_logFilePeriodicReportThreadProc, NULL)) {
                log_i("pthread_create successed");
                isDoing = TRUE;
                (void)pthread_detach(threadId);
            } else {
                log_e("pthread_create failed");
            }
        }
    } while(FALSE);
}

/**
 * @brief 日志文件上报的初始化
 * @note 因为日志上报的初始化需要记录数据到KV中，所以需要在kvstorage初始化结束后，才能初始化此模块
 * 
 * @param initInfo [IN]    日志收集并上报需要的信息
 */
void adm_irm_logFileReportInit(const ADM_IRM_LOG_FILE_REPORT_INIT_T* initInfo)
{
    (void)logFileLock();
    do {
        //!< 校验参数
        if (NULL == initInfo) {
            log_e("Input parm initInfo is nil");
            break;
        }

        //!< 判断是否已经初始化
        if (TRUE == g_initOk) {
            log_i("It has inited successed");
            break;
        }

        //!< 申请初始化信息空间
        g_initInfo = adm_malloc(sizeof(ADM_IRM_LOG_FILE_REPORT_INIT_T));
        if (NULL == g_initInfo) {
            log_e("adm_malloc g_initInfo failed");
            break;
        }

        (void)adm_memset(g_initInfo, 0x00, sizeof(ADM_IRM_LOG_FILE_REPORT_INIT_T), sizeof(ADM_IRM_LOG_FILE_REPORT_INIT_T));
        //!< 填充初始化信息
        g_initInfo->logInitCb = initInfo->logInitCb;
        g_initInfo->logDeinitCb = initInfo->logDeinitCb;
        g_initInfo->fileEncCb = initInfo->fileEncCb;
        g_initInfo->getSessionIdCb = initInfo->getSessionIdCb;
        g_initInfo->getStrategyIdCb = initInfo->getStrategyIdCb;
        g_initInfo->getTaskIdCb = initInfo->getTaskIdCb;
        g_initInfo->getSdkVerCb = initInfo->getSdkVerCb;
        g_initInfo->isEncrypt = initInfo->isEncrypt;
        if (NULL != initInfo->secret) {
            g_initInfo->secret = adm_strdup(initInfo->secret);
        }
        g_initInfo->secretLen = initInfo->secretLen;
        g_initInfo->segmentEncLen = initInfo->segmentEncLen;
        if (NULL != initInfo->logCollectDir) {
            g_initInfo->logCollectDir = adm_strdup(initInfo->logCollectDir);
        }
        if (NULL != initInfo->logPackDir) {
            g_initInfo->logPackDir = adm_strdup(initInfo->logPackDir);
        }
        if (NULL != initInfo->logStorageDir) {
            g_initInfo->logStorageDir = adm_strdup(initInfo->logStorageDir);
        }
        g_initInfo->getLogReportCycle = initInfo->getLogReportCycle;

        //!< 日志收集状态初始化
        (void)__adm_irm_logFileStateInit();

        //!< 周期上报的触发
        __adm_irm_logFileReportPeriodicReportDeal();

        g_initOk = TRUE;
    } while(FALSE);
    (void)logFileUnlock();

    return;
}

/**
 * @brief 日志文件上报模块的反初始化
 * 
 */
void adm_irm_logFileReportDeinit(void)
{
    (void)logFileLock();
    if (TRUE == g_initOk) {
        if (NULL != g_initInfo) {
            g_initInfo->logInitCb = NULL;
            g_initInfo->logDeinitCb = NULL;
            g_initInfo->fileEncCb = NULL;
            g_initInfo->getSessionIdCb = NULL;
            g_initInfo->getStrategyIdCb = NULL;
            g_initInfo->getTaskIdCb = NULL;
            g_initInfo->getSdkVerCb = NULL;
            g_initInfo->isEncrypt = 2;
            if (NULL != g_initInfo->secret) {
                adm_free(g_initInfo->secret);
            }
            g_initInfo->secretLen = 0;
            g_initInfo->segmentEncLen = 0;
            if (NULL != g_initInfo->logCollectDir) {
                adm_free(g_initInfo->logCollectDir);
            }
            if (NULL != g_initInfo->logPackDir) {
                adm_free(g_initInfo->logPackDir);
            }
            if (NULL != g_initInfo->logStorageDir) {
                adm_free(g_initInfo->logStorageDir);
            }
            g_initInfo->getLogReportCycle = NULL;
            adm_free(g_initInfo);
        }
        g_initOk = FALSE;
    }
    (void)logFileUnlock();

    return;
}
/*************************************************************初始化和反初始化***********************************************************************/


static D32S adm_irm_collectUcmKeyLogFile(void)
{
    D32S iRet = ADM_ECODE_FAILED;
    D64S startTime = 0;
    D64S lTimeStamp = 0;
    DCHAR zipFileName[128] = {0};
    DCHAR pkiLogName[128] = {0};
    
    do {
        //!< 校验初始化变量
        if (NULL == g_initInfo) {
            iRet = ADM_ECODE_INVALID_PARAM;
            log_e("log file report module init falied, [err:%d]", iRet);
            break;
        }
        
        //!< 日志文件名默认为： admfotaapp_timestamp.zip
        (void)adm_memset(zipFileName, 0x00, sizeof(zipFileName), sizeof(zipFileName));
        lTimeStamp = time(NULL);
        (void)adm_snprintf(zipFileName, sizeof(zipFileName), "%s_%lld.zip", ADM_IRM_UCM_LOG_FILE_PREFIX, lTimeStamp);

        //!< 收集主控的日志文件，将主控的日志文件打包到 collect 目录下
        if (0 != adm_com_zipFile(g_initInfo->logStorageDir, g_initInfo->logCollectDir, zipFileName)) {
            log_e("adm_com_zipFile logStorageDir:%s, logCollectDir:%s, zipFileName:%s failed", g_initInfo->logStorageDir, g_initInfo->logCollectDir, zipFileName);
            break;
        }

#ifdef SUPPORT_LION_PKI
        const DCHAR* dirStr = adm_cfg_iniParserGetString(ADM_CFG_VUS_PKI_DIR, ADM_CFG_VUS_PKI_DIR_DEFAULT_VAL);

        //!< 日志文件名默认为： pki_timestamp.zip
        (void)adm_memset(zipFileName, 0x00, sizeof(zipFileName), sizeof(zipFileName));
        lTimeStamp = time(NULL);
        (void)adm_snprintf(zipFileName, sizeof(zipFileName), "%s_%lld.zip", ADM_IRM_PKI_LOG_FILE_PREFIX, lTimeStamp);

        adm_snprintf(pkiLogName,sizeof(pkiLogName),"%s/file/",dirStr);

        //!< 收集主控的日志文件，将主控的日志文件打包到 collect 目录下
        if (0 != adm_com_zipFile(pkiLogName, g_initInfo->logCollectDir, zipFileName)) {
            log_e("adm_com_zipFile logStorageDir:%s, logCollectDir:%s, zipFileName:%s failed", pkiLogName, g_initInfo->logCollectDir, zipFileName);
            break;
        }
#endif
        //!< remove UCM日志存储目录下的所有， xxx/admota/log/ucm/storage/ 日志文件
        //__adm_irm_removeLogFiles(g_initInfo->logStorageDir, ADM_IRM_REMOVE_FILE_IGNORE_NAME);

        //!< 将日志记录的startTime，备份给到reportStartTime，写入到kv中
        if (TRUE == __adm_irm_logFileTimeGet(g_logFileStartTimeKvKey, &startTime)) {
            (void)__adm_irm_logFileTimeSet(g_logFileReportStartTimeKvKey, startTime);
        }
        //!< 将日志记录的endtime，写入打kv中
        lTimeStamp = time(NULL);
        (void)__adm_irm_logFileTimeSet(g_logFileReportEndTimeKvKey, lTimeStamp);

        //!< 更新startTime
        lTimeStamp = time(NULL);
        (void)__adm_irm_logFileTimeSet(g_logFileStartTimeKvKey, lTimeStamp);

        iRet = ADM_ECODE_SUCCESSED;

    } while(FALSE);
    return iRet;
}

/**
 * @brief 循环解析各模块的instanceIds，并根据传入的状态值，执行不同的操作
 * 
 * @param pcInstanceidsKey  [IN]    ucmconfig.ini中，各模块的名称
 * @param eStateType        [IN]    日志采集的状态值
 */
static void adm_irm_collectSysiKeyLogFile()
{
    DCHAR* instanceIdStr = NULL;
    DCHAR* subInstanceIdStr = NULL;
    D32S instanceId = 0;
    DCHAR cSysiLogCollectStateKey[128] = {0};
    DCHAR* pcSysiLogCollectStateVal = NULL;
    const DCHAR* cfgInstanceIdsStr = NULL;
    const DCHAR* vdicfgInstanceIdsStr = NULL;
    ADM_KV_STORAGE_VALUE_TYPE_E eValType = ADM_KV_STORAGE_VALUE_TYPE_INVALID;
    size_t len = 0;
    DCHAR cNewLogPath[256] = {0};
    
    /**1、采集从控keylog*/
    do {
        cfgInstanceIdsStr = adm_cfg_iniParserGetString(ADM_CFG_VUS_UCL_INSTANCEID, "");
        instanceIdStr = adm_strdup(cfgInstanceIdsStr);
        if (NULL != instanceIdStr) 
        {
            subInstanceIdStr = strsep(&instanceIdStr, ",");
            while(NULL != subInstanceIdStr) 
            {
                log_i("subInstanceIdStr:%s", subInstanceIdStr);
                instanceId = atoi(subInstanceIdStr);
                (void)__adm_irm_collecySysiLogFileEach(instanceId,ADM_DIM_SOA_TRANS_FILE_TYPE_KEYLOG);

                subInstanceIdStr = strsep(&instanceIdStr, ",");
            }
            adm_free(instanceIdStr);
        }
    } while(FALSE);

    /**2、采集VDI keylog*/
    do {
        vdicfgInstanceIdsStr = adm_cfg_iniParserGetString(ADM_CFG_VUS_VDI_INSTANCEID, "");
        instanceIdStr = adm_strdup(vdicfgInstanceIdsStr);
        if (NULL != instanceIdStr) 
        {
            subInstanceIdStr = strsep(&instanceIdStr, ",");
            while(NULL != subInstanceIdStr) 
            {
                log_i("subInstanceIdStr:%s", subInstanceIdStr);
                instanceId = atoi(subInstanceIdStr);
                (void)__adm_irm_collecySysiLogFileEach(instanceId,ADM_DIM_SOA_TRANS_FILE_TYPE_KEYLOG);

                subInstanceIdStr = strsep(&instanceIdStr, ",");
            }
            adm_free(instanceIdStr);
        }
    } while(FALSE);

    /**3、采集主控keylog*/
    if (NULL != g_initInfo->logCollectDir) 
    {
        len = adm_strlen(g_initInfo->logCollectDir);
        if (0 != len) 
        {
            (void)adm_memzero(cNewLogPath, sizeof(cNewLogPath));

            if ('/' == g_initInfo->logCollectDir[len - (D32U)1]) 
            {
                (void)adm_snprintf(cNewLogPath, sizeof(cNewLogPath), "%skeylog_ucm_%lld.log", g_initInfo->logCollectDir, time(NULL));
            } 
            else 
            {
                (void)adm_snprintf(cNewLogPath, sizeof(cNewLogPath), "%s/keylog_ucm_%lld.log", g_initInfo->logCollectDir, time(NULL));
            }
        }
        else 
        {
            log_e("get log collect dir length is error");
        }
    }

    const DCHAR* pKeylogPath =  adm_cfg_iniParserGetString(ADM_CFG_VUS_KEYLOG_PATH, ADM_CFG_VUS_KEYLOG_PATH_DEFAULT_VAL);
    if (NULL != pKeylogPath)
    {
        rename(pKeylogPath, cNewLogPath);
    }

    return;
}

void adm_irm_rmAllKeyLogFile()
{
    DCHAR* instanceIdStr = NULL;
    DCHAR* subInstanceIdStr = NULL;
    D32S instanceId = 0;
    DCHAR cSysiLogCollectStateKey[128] = {0};
    DCHAR* pcSysiLogCollectStateVal = NULL;
    const DCHAR* cfgInstanceIdsStr = NULL;
    const DCHAR* vdicfgInstanceIdsStr = NULL;
    ADM_KV_STORAGE_VALUE_TYPE_E eValType = ADM_KV_STORAGE_VALUE_TYPE_INVALID;
    size_t len = 0;
    DCHAR cNewLogPath[256] = {0};
    D32S iErrorCode = 0;
    
    /**1、删除从控keylog*/
    do {
        cfgInstanceIdsStr = adm_cfg_iniParserGetString(ADM_CFG_VUS_UCL_INSTANCEID, "");
        instanceIdStr = adm_strdup(cfgInstanceIdsStr);
        if (NULL != instanceIdStr) 
        {
            subInstanceIdStr = strsep(&instanceIdStr, ",");
            while(NULL != subInstanceIdStr) 
            {
                log_i("subInstanceIdStr:%s", subInstanceIdStr);
                instanceId = atoi(subInstanceIdStr);
                (void)adm_dim_ucl_requestRmFile(instanceId, 30, ADM_KEYLOG_FILE_NAME, &iErrorCode);

                subInstanceIdStr = strsep(&instanceIdStr, ",");
            }
            adm_free(instanceIdStr);
        }
    } while(FALSE);

    /**2、删除VDI keylog*/
    do {
        vdicfgInstanceIdsStr = adm_cfg_iniParserGetString(ADM_CFG_VUS_VDI_INSTANCEID, "");
        instanceIdStr = adm_strdup(vdicfgInstanceIdsStr);
        if (NULL != instanceIdStr) 
        {
            subInstanceIdStr = strsep(&instanceIdStr, ",");
            while(NULL != subInstanceIdStr) 
            {
                log_i("subInstanceIdStr:%s", subInstanceIdStr);
                instanceId = atoi(subInstanceIdStr);
                (void)adm_dim_ucl_requestRmFile(instanceId, 30, ADM_KEYLOG_FILE_NAME, &iErrorCode);
                subInstanceIdStr = strsep(&instanceIdStr, ",");
            }
            adm_free(instanceIdStr);
        }
    } while(FALSE);

    /**3、删除主控keylog*/
    const DCHAR* pKeylogPath =  adm_cfg_iniParserGetString(ADM_CFG_VUS_KEYLOG_PATH, ADM_CFG_VUS_KEYLOG_PATH_DEFAULT_VAL);
    if (NULL != pKeylogPath)
    {
        adm_pub_rmKeylogFile();
    }

    return;
}

static DBOOL adm_irm_collectKeyLogFileDeal(void)
{
    DBOOL isDealOk = FALSE;

    do {
    
        //!< 先收集
        adm_irm_collectSysiKeyLogFile();
        //!< 再打包
        if (FALSE == __adm_irm_packLogFile(ADM_DIM_SOA_TRANS_FILE_TYPE_KEYLOG)) 
        {
            log_e("__adm_irm_packLogFile failed");
            break;
        }

        isDealOk = TRUE;
        log_i("adm_irm_collectLogFileDeal successed");
    } while(FALSE);

    return isDealOk;
}
