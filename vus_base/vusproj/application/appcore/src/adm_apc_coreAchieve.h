/**
 * @file adm_apc_coreAchieve.h
 * @author mengwei@abupdate.com
 * @brief 为OTA应用程序入口，提供各初始化或模块流程的接口实现。
 * @version 0.1
 * @date 2021-09-24
 * 
 * @copyright Copyright © 2018 All rights 上海艾拉比智能科技有限公司
 * 
 */
#ifndef __ADM_APC_CORE_ACHIEVE_H__
#define __ADM_APC_CORE_ACHIEVE_H__

#ifdef __cplusplus
extern "C" {
#endif

#define EXPORT_FUN __attribute__((visibility("default")))

#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>

#include "adm_string.h"
#include "adm_memory.h"

#include "adm_typedefs.h"
#include "adm_apc_log.h"
#include "adm_com_errorCode.h"
#include "adm_com_lib.h"
#include "adm_cfg_api.h"
#include "adm_application_common.h"
#include "adm_tracer_interface.h"
#include "adm_uim_icp.h"


#define ADM_APC_CYCLE_OF_WAITTING_TASK      60      //!< 等待任务的周期间隔
#define ADM_APC_RETRY_REGISTER_NUMBERS      5       //!< 重试注册的次数
#define ADM_APC_RETRY_REGISTER_CYCLE        60      //!< 重试注册的间隔
#define ADM_ACP_PERIODIC_REGISTER_TIME      (10 * 60) //!< 周期注册的间隔
#define ADM_APC_RETRY_GET_CONFIG_FROM_SRV_NUM   5   //!< 重试从平台获取配置的次数
#define ADM_APC_RETRY_GET_CONFIG_FROM_USB_NUM   3   //!< 重试从USB中获取配置的次数
#define ADM_APC_RETRY_GET_CONFIG_CYCLE 10           //!< 获取配置重试间隔
#define ADM_APC_PERIODIC_GET_CFG_TIME       (10 * 60)  //!< 周期获取配置的间隔

#define ADM_APC_RETRY_GETSN_CYCLE       10      //!< 重试获取sn信息的间隔 s
#define ADM_APC_RETRY_GETVIN_CYCLE      10      //!< 重试获取vin码信息的间隔 s
#define ADM_APC_RETRY_PKIINIT_CYCLE     10      //!< PKI初始化间隔 s
#define ADM_APC_GETSN_TIME_OUT          3*60    //!< 获取sn信息的超时时间 s

#define ADM_APC_OTA_CONFIG_PATH     "./otaconfig.ini"

#define ADM_APC_DTS_SHARED_LIB_DIR    ADM_CFG_OWNER_VUS":dts_lib_path"      //!< <KEY>  日志模块动态库目录
#define ADM_APC_DTS_CONFIG_DIR        ADM_CFG_OWNER_VUS":dts_config_path"    //!< <KEY>  日志模块配置文件目录
#define ADM_APC_DTS_SHARED_LIB_DIR_DEFAULT_VAL  "../lib/libadmdts.so"       //!< <VALUE>    日志模块的动态库路径
#define ADM_APC_DTS_CONFIG_DIR_DEFAULT_VAL      "../etc/admdts.conf"        //!< <VALUE>     日志模块获取配置信息的默认配置文件

#define ADM_APC_PROCESS_RESTART_MINIMUN_TIMES   2               //!< 进程重启最小次数
#define ADM_APC_PROCESS_RESTART_MINIMUN_TOTALTIME   60          //!< 进程重启最小次数，使用的总时间 <单位：s>
#define ADM_APC_PROCESS_START_INTERVAL   300                    //!< 设备启动时间间隔 <单位：s>
#define ADM_APC_UPTINEFILE_DATA_SIZE   30                       //!< 设备启动时间文件(/proc/uptime)文件内容大小
#define ADM_APC_SINGLE_LOCK_DIR    "tmp/"                       //!< 文件锁需要存储目录
#define ADM_APC_SINGLE_LOCK        "single_proc.lock"           //!< 文件锁名称
#define ADM_APC_PROCESS_INFO_KV_KEY       "processinfo"         //!< 进程信息关键字KEY
#define ADM_APC_PROCESS_INFOS             "processinfos"        //!< 进程信息
#define ADM_APC_PID                       "pid"                 //!< 进程号
#define ADM_APC_RECORD_TIME               "recordtime"          //!< 时间戳

/*!
 * @brief 进程记录信息
 */
typedef struct stApcRecordInfo_
{
    D32S iPid;           /*!< 进程id */
    D64S iRecordTime;     /*!< 设备启动时间间隔 */;
} ADM_APC_RECORD_INFO_T;

/*!
 * @brief 进程记录
 */
typedef struct stApcProRecordInfo_
{
    D32S                  iRecordNum;              //!< 记录数
    ADM_APC_RECORD_INFO_T *pstApcRecordInfoList;  //!< 进程记录信息
} ADM_APC_PRO_RECORD_T;


/**
 * @brief 初始化程序启动阶段的事件上报
 * 
 */
extern void adm_apc_reportEventInit(void);

/*!
 *******************************************************************************
 * Exit the tracerDaemon module
 * return	D32S  0:successed, non-0:failed 
 *******************************************************************************
 */
//extern D32S adm_tracerDaemonStop(void);

/**
 *******************************************************************************
 * @brief traerDaemon模块主入口
 * @note traerDaemon模块主入口。
 * 
 * @param soFileName	    [In] char * 日志桥接服务
 * 程序动态库的全路径文件名称
 * @param configFileName	[In] char * 日志桥接服务程序需要的配置文件全路文件名称
 *                           Note: support gw and passive mode. 
 *                                 suggest HMI/PC as gw;tbox as passive.
 * @return D32S 		0:successed, non-0: failed
 *******************************************************************************
 */
//extern D32S adm_tracerDaemonStart(DCHAR *soFilenName, DCHAR *configFileName);

/**
 * @brief 初始化OTA配置文件
 * 
 * @param filename [in]     配置文件的绝对路径
 * @return D32S     0:Successed, None-0:Failed
 */
extern D32S adm_apc_otaConfigInit(const DCHAR* filename);

/**
 * @brief 反初始化OTA配置文件
 * 
 */
extern void adm_apc_otaConfigDeinit(void);

/**
 * @brief OTA应用程序所使用的目录初始化
 * 
 * @return D32S     0:Successed, None-0:Failed
 */
extern D32S adm_apc_dirInit(void);

/**
 * @brief 初始化coredump配置
 * 
 * @param [in]     无
 * @return D32S     0:Successed, None-0:Failed
 */
D32S adm_apc_otaCoredumpConfig(void);


/**
 * @brief OTA应用程序日志初始化
 * 
 * @return D32S     0:Successed, None-0:Failed
 */
extern D32S adm_apc_logInit(void);

/**
 * @brief 日志模块的反初始化
 * 
 */
extern void adm_apc_logDeinit(void);

/**
 * @brief Tracer工具初始化
 * 
 */
extern void adm_apc_tracerInit(void);

/**
 * @brief Tracer工具反初始化
 * 
 */
extern void adm_apc_tracerDeinit(void);

/**
 * @brief 打印所有模块的版本信息，包括：VUS/SYSI/Foundataion/ewebserver/vrpc等
 * 
 */
extern void adm_apc_printSdkVersion(void);

/**
 * @brief Core service初始化
 * 
 * @return D32S     0:Successed, None-0:Failed
 */
extern D32S adm_apc_coreServiceInit(void);

/**
 * @brief Core service反初始化
 * 
 */
extern void adm_apc_coreServiceDeinit(void);

/**
 * @brief kv storage初始化
 * 
 * @return D32S     0:Successed, None-0:Failed
 */
extern D32S adm_apc_kvStorageInit(void);

/**
 * @brief kv storage反初始化
 * 
 */
extern void adm_apc_kvStorageDeinit(void);

/**
 * @brief OCM 模块初始化
 * 
 * @return D32S     0:Successed, None-0:Failed
 */
extern D32S adm_apc_ocm_init(void);

/**
 * @brief DIM 模块初始化
 * 
 * @return D32S     0:Successed, None-0:Failed
 */
extern D32S adm_apc_dimInit(void);

/**
 * @brief DIM 模块反初始化
 * 
 */
extern void adm_apc_dimDeinit(void);

/**
 * @brief UIM 模块初始化
 * 
 * @return D32S     0:Successed, None-0:Failed
 */
extern D32S adm_apc_uimInit(void);

/**
 * @brief UIM 模块反初始化
 * 
 */
extern void adm_apc_uimDeinit(void);

/**
 * @brief 各模块初始化，包括：CVM、DPM、IPM等
 * 
 * @return D32S     0:Successed, None-0:Failed
 */
extern D32S adm_apc_managerInit(void);

/**
 * @brief 各模块反初始化，包括：CVM、DPM、IPM等
 * 
 */
extern void adm_apc_managerDeinit(void);

/**
 * @brief 判断当前状态是否为安装中
 * 
 * @return DBOOL    TRUE:在安装中，FALSE:非安装中
 */
extern DBOOL adm_apc_isInstallingState(void);

/**
 * @brief 根据状态恢复任务
 * 
 * @return D32S     0:Successed, None-0:Failed
 */
extern D32S adm_apc_resumeTask(void);

/**
 * @brief 任务等待状态，While(1)
 * 
 */
extern void adm_apc_waitTask(void);

/**
 * @brief 上电，等待网络恢复OK
 * 
 * @param waitTime [IN] const D32U  等待的时间
 */
extern void adm_apc_waitNetworkOk(const D32U waitTime);

/**
 * @brief 执行注册的逻辑，包括：重试注册、周期注册等
 * 
 * @return D32S     0:Successed, None-0:Failed
 */
extern D32S adm_apc_register(void);

/**
 * @brief 执行获取配置的逻辑，包括：重试获取、周期获取等
 * 
 * @return D32S     0:Successed, None-0:Failed
 */
extern D32S adm_apc_getConfig(void);

/**
 * @brief 为OTA注入第一滴血，包括：注册和获取配置信息
 * 
 * @return D32S     0:Successed, None-0:Failed
 */
extern D32S adm_apc_otaFirstBlood(void);

/*!
 *******************************************************************************
 * apc 模拟触发下载动作
 *
 * \note
 * 
 * \retval    D32S
 *******************************************************************************
 */
extern D32S adm_apc_triggerDownload(void);

/*!
 *******************************************************************************
 * apc 上报模块初始化
 *
 * \note
 *
 * \retval    D32S
 *******************************************************************************
 */
extern D32S adm_apc_irmInit(void);

/**
 * @brief IRM 上报模块反初始化
 *
 */
extern void adm_apc_irmDeinit(void);

/*!
*******************************************************************************
* 关闭文件锁
* 
* \note   
*
* \retval      void
*******************************************************************************
*/
void adm_apc_closelockfileHandle(void);

/*!
 *******************************************************************************
 * apc 防止主程序的重复拉起
 *
 * \note
 * 
 * \retval    D32S 0 成功，非 0 失败
 *******************************************************************************
*/
extern D32S adm_apc_singleProgram(void);

/*!
 *******************************************************************************
 * 检查进程信息,并设置进程信息到数据库.
 *
 * \note
 *
 * \retval    D32S 0 成功，非 0 失败
 *******************************************************************************
 */
extern D32S adm_apc_checkProcess(void);

/*!
 *******************************************************************************
 * 日志上报
 *
 * \note
 *
 * \retval        void
 *******************************************************************************
 */
extern void adm_apc_logReport(void);

/*!
 *******************************************************************************
 * 日志收集
 *
 * \note
 *
 * \retval        void
 *******************************************************************************
 */
extern D32S adm_apc_logCollect(void);

/*!
 *******************************************************************************
 * 删除升级包
 *
 * \note
 *
 * \retval        D32S
 *******************************************************************************
 */
extern D32S adm_apc_cleanDownladPackage(void);

/*!
 *******************************************************************************
 * 清空数据，退出进程
 *
 * \note
 *
 * \retval        void
 *******************************************************************************
 */
extern void adm_apc_resetOTA(void);

/*!
 *******************************************************************************
 * 清空数据，退出进程
 *
 * \note 升级中的状态是不允许退出进程的
 *
 * \retval        D32S
 *******************************************************************************
 */
extern D32S adm_apc_exitUCM();

/**
 * @brief 网络代理初始化
 * 
 * @return D32S     0:Successed, None-0:Failed
 */
extern D32S adm_apc_httpproxyInit(void);

void __adm_apc_handleUimOnline(void *priv);
void usage(void);

extern D32S adm_apc_vehicleConfigReset(DBOOL bIsInstalling);

extern void adm_apc_taskVerify(void);

#ifdef __cplusplus  // close out "C" linkage in case of c++ compiling
}
#endif

#endif//__ADM_APC_CORE_ACHIEVE_H__
