#pragma once

#include "adm_typedefs.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct adm_kvStorage_ KvStorage_t;

/**
 * @brief 初始化OTA过程中的kv数据库
 * @note  1. OTA的待存储数据量级较小，所以分为两个数据库
 *        2. business(业务): 注册、配置、检测、下载、安装等过程中产生的需持久存储的信息
 *        3. report(上报)：进度、结果、状态、事件等的上报，产生的需持久存储的信息
 *
 * @return D32S     0:Successed, None-0:Failed
 */
extern D32S adm_core_service_init(void);

/**
 * @brief 释放kvstorage数据库
 *
 */
extern void adm_core_service_deinit(void);

/**
 * @brief 删除kvstorage数据库
 * 
 */
extern void adm_core_service_deeplyDestroy(void);

/**
 * @brief 删除kvstorage数据库，并重新创建
 *
 */
extern void adm_core_service_delAndCreatKv(void);

/**
 * @brief 获取OTA过程中业务信息存储的kv数据库
 *
 * @return KvStorage_t*     NULL:Failed, None-NULL:获取到的数据库句柄
 */
extern KvStorage_t* adm_core_service_getBusinessDatabaseDb();
/**
 * @brief 获取OTA过程中上报信息存储的kv数据库
 *
 * @return KvStorage_t*     NULL:Failed, None-NULL:获取到的数据库句柄
 */
extern KvStorage_t* adm_core_service_getReportDatabaseDb();

/**
 * @brief 待设置运行时目录的类型
 *
 */
typedef enum ADM_CORE_SERVICE_RUNTIME_DIR_TYPE_
{
    ADM_CORE_SERVICE_RUNTIME_DIR_TYPE_KV_STORAGE = 0x01,  //!< KV Storage 目录
    ADM_CORE_SERVICE_RUNTIME_DIR_TYPE_LOG = 0x02,  //!< 日志存储目录
    ADM_CORE_SERVICE_RUNTIME_DIR_TYPE_PACKAGE = 0x03,  //!< 升级包存储目录
    ADM_CORE_SERVICE_RUNTIME_DIR_TYPE_CERT = 0x04,  //!< 证书存储目录
    ADM_CORE_SERVICE_RUNTIME_DIR_TYPE_UNZIP = 0x05,  //!< 文件解压存储目录
    ADM_CORE_SERVICE_RUNTIME_DIR_TYPE_INVALID = 0xFF,  //!< 无效的类型
} ADM_CORE_SERVICE_RUNTIME_DIR_TYPE_E;

/**
 * @brief 数据库目录
 *
 */
typedef enum ADM_CORE_SERVICE_DIR_TYPE_
{
    ADM_CORE_SERVICE_BUSINESS_DIR = 0x01,  //!< business数据库
    ADM_CORE_SERVICE_REPORE_DIR = 0x02,  //!< report数据库
    ADM_CORE_SERVICE_DIR_INVALID = 0xFF,  //!< 无效的类型
} ADM_CORE_SERVICE_DIR_TYPE_E;

/**
 * @brief 设置OTA程序运行时目录，包括：Kv、log、package、trans等
 *
 * @param eType     [IN]    待设置运行时目录的类型
 * @param runtimeDir    [IN]    运行时目录
 * @return DBOOL        TRUE:设置成功，FALSE:设置失败
 */
extern DBOOL adm_core_service_initRuntimeDir(const ADM_CORE_SERVICE_RUNTIME_DIR_TYPE_E eType, const DCHAR* runtimeDir);

/**
 * @brief 获取运行时目录
 *
 * @param eType     [IN]    运行时目录的类型
 * @return DCHAR*   NULL:Failes, None-NULL: 获取到的运行时目录信息
 * @note 运行时目录信息，使用结束后，需释放内存
 */
extern DCHAR* adm_core_service_getRuntimeDir(const ADM_CORE_SERVICE_RUNTIME_DIR_TYPE_E eType);

/**
 * @brief 释放OTA程序运行时目录信息
 *
 * @param eType     [IN]    运行时目录类型
 */
extern void adm_core_service_deinitRuntimeDir(const ADM_CORE_SERVICE_RUNTIME_DIR_TYPE_E eType);

/**
 * @brief 初始化车云通信使用的默认密钥
 * 
 * @param secret    [IN]    车云通信使用的默认密钥
 * @return DBOOL    TRUE: Successed, FALSE: Failed
 */
extern DBOOL adm_core_service_initOcsDefaultSecret(const DCHAR* secret);

/**
 * @brief 获取车云通信使用的默认密钥
 * 
 * @return DCHAR*   NULL:Failed, None-NULL:车云通信使用的默认密钥
 * @note 默认密钥使用完成后需要释放空间
 */
extern DCHAR* adm_core_service_getOcsDefaultSecret(void);

/**
 * @brief 释放车云通信使用的默认密钥
 * 
 */
extern void adm_core_service_deinitOcsDefaultSecret(void);

#define ADM_CORE_SERVICE_DEFAULT_VERSION    "engine_default_v1.0.0"     //!< 默认版本号，如果无法获取版本号，即返回默认值

/**
 * @brief 获取core service的版本号
 * 
 * @return DCHAR*   NULL:Failed, None-NULL: core service 的版本号
 */
extern DCHAR* adm_core_service_sdkVersion(void);

#ifdef __cplusplus
}
#endif
