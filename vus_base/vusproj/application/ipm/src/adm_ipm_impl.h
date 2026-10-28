#ifndef	__ADM_IPM_IMPL_H_
#define __ADM_IPM_IMPL_H_

#include "adm_typedefs.h"
#include "adm_ipm_install.h"
#include "adm_ips_install.h"
#include "adm_pis_package.h"
//#include "adm_workerqueue.h"
#include "adm_list.h"
#include "adm_ips_install.h"
#include "adm_uim_install.h"
#include "adm_dim_vdi.h"
#include "adm_dim_host.h"
#include "adm_dim_ucl.h"
#include "adm_ecs_rsConfig.h"
#include "json-c/json.h"
#include "adm_ocs_airProtocolAdapter.h"
#include "adm_dps_parsePackageConfig.h"
#include "adm_ipm_impl_lion.h"


#include <pthread.h>//TODO use platform api
#include <semaphore.h>

#ifdef __cplusplus
extern "C" {
#endif

#define ADM_IPM_CONDITION_KEEP_NETWORK_TIMES (180U) //前置条件检查需要维持网络的最大时间值，单位：（秒）
#define ADM_IPM_INSTALL_KEEP_NETWORK_TIMES (180U) //安装过程需要维持网络的最大时间值，单位：（秒）

/** 编程执行超时时间（5分钟） */
#define ADM_IPM_PROGRAM_TIMEOUT             5*60

/** 预编程预置脚本路径 */
#define ADM_IPM_PRE_PROGRAM_SCRIPT_PATH     "/emmc/svp/tpa/sysi/etc/Chery_Pre_Program.ttb"

/** 后编程后置脚本路径 */
#define ADM_IPM_POST_PROGRAM_SCRIPT_PATH    "/emmc/svp/tpa/sysi/etc/Chery_Post_Program.ttb"


#define ADM_IPM_DIM_ECU_MEMBER_SIZE	128
#define ADM_IPM_DIM_REQUEST_TIMEOUT	15

//!< 解压的升级包存储路径后缀名, g_unzipDir/IpmPkg/
#define ADM_IPM_INSTALL_UNZIPDIR_SUFFIX     "IpmPkg"
//!< 升级包中配置文件名
#define ADM_IPM_ECU_PACKAGE_CFGFILE_NAME   "packageConfig.json"   

#define REPORT_VEHICLE_STATE_EXTS_CMDID "cmdId"
#define REPORT_VEHICLE_STATE_EXTS_SCHEDULETIME "scheduleTime"

/* 并行数量限制M和N配置名 */
#define ADM_CFG_IPM_UPGRADE_PARALLEL_LIMIT_M			ADM_CFG_OWNER_VUS":upgrade_parallel_num_limit_m"
#define ADM_CFG_IPM_UPGRADE_PARALLEL_LIMIT_N			ADM_CFG_OWNER_VUS":upgrade_parallel_num_limit_n"

#define ADM_IPM_UPGRADE_PARALLEL_NUM_LIMIT              99999999        // 并行数量限制
#define ADM_IPM_UPGRADE_PARALLEL_LIMIT_M_DEFAULT        ADM_IPM_UPGRADE_PARALLEL_NUM_LIMIT      // 并行数量M
#define ADM_IPM_UPGRADE_PARALLEL_LIMIT_N_DEFAULT        ADM_IPM_UPGRADE_PARALLEL_NUM_LIMIT      // 并行数量N

#define	ADM_IPM_SCHDULE_INFO_KV_KEY "ipm_schdule_info"
#define ENVIRONMENT_WAIT_PROGRESS "environment_wait_progress"

#define IPM_NOTIFY_FLAG_KEY_TXT   "ipm_notifyFlag"

/*
typedef enum{
    TRACE_LEVEL = 1,
    DEBUG_LEVEL,		 
    INFO_LEVEL,
	WARN_LEVEL,
	ERROR_LEVEL			
}ADM_IPM_REPORT_EVENT_LEVEL_E;
*/

typedef enum 
{
    UPGRADE_FAILED = -1,            ///< 升级失败
    UPGRADE_SUCCEED = 0,            ///< 升级成功（重启并且升级成功后返回）
    UPGRADE_IDEL = 1,               ///< 初始状态
    UPGRADE_UPGRADING = 2,          ///< 升级中
    UPGRADE_WRITEDONE = 3,          ///< 刷写完成状态（进行重启时要求的状态）
    UPGRADE_BACKUP = 4             ///< 备份状态（备用字段）
} ADM_IPM_UPGRADE_RESULT_E;



//!< 验签请求超时时间 
#define ADM_IPM_SIGN_REQ_TIMEOUT        60

/**
 * @brief 安装前置条件类型
 * 
 */
typedef enum {
    ADM_IPM_CONDITION_TYPE_USER = 0x01,                 //!< 车机端立即升级
    ADM_IPM_CONDITION_TYPE_SCHDULE = 0x02,              //!< 预约升级
    ADM_IPM_CONDITION_TYPE_NIGHT_AUTO = 0x03,           //!< 夜间自动升级
    ADM_IPM_CONDITION_TYPE_SLIENCE = 0x04,              //!< 静默升级
    ADM_IPM_CONDITION_TYPE_PHONE = 0x05,                //!< 手机立即升级
    ADM_IPM_CONDITION_TYPE_USB = 0x06,                  //!< USB升级
    ADM_IPM_CONDITION_TYPE_PROD_LINE = 0x07,            //!< 产线升级
    ADM_IPM_CONDITION_TYPE_RESCUE = 0x08,               //!< 救援升级
    ADM_IPM_CONDITION_TYPE_APK_SELF = 0x09,             //!< APK
    ADM_IPM_CONDITION_TYPE_FORCE = 0x10,                //!< 强制升级
    ADM_IPM_CONDITION_TYPE_INVALID = 0x11               //!< 无效值
} ADM_IPM_CONDITION_TYPE_E;

/**
 * @brief 编程类型
 * @note
 */
typedef enum ADM_IPM_PROGRAM_TYPE_
{
    IPM_PROGRAM_TYPE_PRE = 0,       /** 预编程 */
    IPM_PROGRAM_TYPE_POST,          /** 后编程 */
    IPM_PROGRAM_TYPE_INVALID        /** 无效值 */
} ADM_IPM_PROGRAM_TYPE_E;

/**
 * @brief 编程ECU列表头结构
 * @note
 */
typedef struct ADM_IPM_PROGRAM_ECU_LIST_HEAD_
{
    D32S num;                   /** 节点数量 */
    struct list_head head;      /** 链表头 */
    pthread_mutex_t mutex;      /** 互斥锁 */
    DBOOL bIsInit;              /** 初始化标志 */
} ADM_IPM_PROGRAM_ECU_LIST_HEAD_T;

/** 
 * @brief 编程ECU信息节点结构
 * @note
 */
typedef struct ADM_IPM_PROGRAM_ECU_INFO_
{
    D32U uiRequestId;           /** 请求ID */
    D32U uiResponseId;          /** 响应ID */
    D32S iPreResult;            /** 预编程结果 */
    struct list_head entry;     /** 链表节点 */
} ADM_IPM_PROGRAM_ECU_INFO_T;

/**
 * @brief 编程请求基础参数结构
 * @note
 */
typedef struct ADM_IPM_PROGRAM_REQ_PARAM_
{
    ADM_IPM_PROGRAM_TYPE_E eProgramType;        /** 编程类型 */
    DCHAR *pcScriptPath;                        /** 编程脚本路径 */
    D32U uiFunctionId;                          /** 功能ID */
}ADM_IPM_PROGRAM_REQ_PARAM_T;


typedef struct
{
    D32S iResult;
    D32U uiRequestId;
    D32U uiResponseId;
    DCHAR* pResultDescrip;
} ADM_IPM_PREGRAM_ECU_INFO_T;

typedef struct
{
    D16U iNum;
    ADM_IPM_PREGRAM_ECU_INFO_T* pPreEcuInfo;
}ADM_IPM_PREGRAM_ECU_DYNARRAY_T;


typedef struct {
    D16U requestTimeout;/*!< sec */
    struct ADM_IPM_DO_FLASH_ *doFlash;/* TODO ??? member here */
}ADM_IPM_DIM_T;

#define	ADM_IPM_UIM_COND_NOTIFY_CYCLE_TIME_SECOND_WIDE	(1)
typedef struct {
    ADM_UIM_CMD_DRIVER_CLICK_F *fnCmdDriverClick;
    void *priv;

    DCHAR condStop;
    pthread_mutex_t condMutex;
    ADM_UIM_SOA_CONDITIONS_VERIFIED_TYPE_E eVerifiedType;
    D16U cycleTime;
    D16U timeout;
}ADM_IPM_UIM_T;

typedef struct {
    DCHAR *extKey;/*!< 扩展字段名 */
    DCHAR *extValue;/*!< 扩展字段值 */
}ADM_IPM_REPORT_SERVER_DATA_EXT_T;
typedef struct {
    ADM_IPM_REPORT_SERVER_DATA_EXT_T *exts;
    D32U extsLen;
}ADM_IPM_REPORT_SERVER_DATA_EXT_LIST_T;

/**
 * @struct  IPM_STATISTICS_EVENT_E
 * @brief   安装升级统计事件类型，每类事件有三个事件ID(依次递增)，
 *          分别对应事件起始ID、事件结束ID和事件统计ID。
 * @details
 */
typedef enum
{
    STAT_EVENT_INVALID = -1,
    STAT_EVENT_LOCAL_VERIFY_SIGN = 1,       // 本地验签
    STAT_EVENT_REMOTE_VERIFY_SIGN = 4,      // 远程验签
    STAT_EVENT_UNZIP_PACKAGE = 7,           // 安装包解压
    STAT_EVENT_PARSE_PAKAGE_CFG = 10,       // 配置文件解析
    STAT_EVENT_VDI_FLASH = 13,              // VDI刷写
    STAT_EVENT_UCM_FLASH = 16,              // UCM刷写
    STAT_EVENT_VDI_ROLLBACK = 19,           // VDI回滚刷写
    STAT_EVENT_UCM_ROLLBACK = 22            // UCM回滚刷写
} IPM_STATISTICS_EVENT_E;

#define STAT_EVENT_EVENT_START_ID(event)     (event)        // 统计事件起始事件ID
#define STAT_EVENT_EVENT_END_ID(event)       ((event) + 1)  // 统计事件结束事件ID
#define STAT_EVENT_EVENT_STAT_ID(event)      ((event) + 2)  // 统计事件统计事件ID

/**
 * @struct  IPM_STAT_EVENT_T
 * @brief   安装升级统计事件
 * @details
 */
typedef struct
{
    const DCHAR*    ecuReqId;       // ECU请求ID
    const DCHAR*    ecuSwDid;       // ECU版本软件ID
    D32S            eventId;        // 事件ID
    D32S            serviceType;    // ECU刷写类型
    D64U            startTime;      // 事件开始时间
    D64U            endTime;        // 事件结束时间
    D64S            fileSize;       // 文件大小
    D32S            progress;       // 进度
} IPM_STAT_EVENT_T;


/**
 * @struct  ADM_IPM_STAT_EVENT_SUMARY_T
 * @brief   ECU刷写性能统计
 * @details
 */
typedef struct
{
    D32S    serviceType;    // ECU刷写类型
    D32S    ecuNum;         // ECU数量
    D32U    totalTime;      // 总耗时
} ADM_IPM_STAT_EVENT_SUMARY_T;

/**
 * @struct  ADM_IPM_REPORT_STAT_EVENT_T
 * @brief   ECU刷写性能统计上报信息
 * @details
 */
typedef struct
{
    ADM_IPM_STAT_EVENT_SUMARY_T sumarys[2]; // 对应VDI刷写、UCM刷写
} ADM_IPM_STAT_EVENT_REPORT_T;


#define ADM_IPM_REPORT_SERVER_DATA_ECU_PROGRESS_TYPE_DOWNLOAD	"1"
#define ADM_IPM_REPORT_SERVER_DATA_ECU_PROGRESS_TYPE_TRANSFER	"2"
#define ADM_IPM_REPORT_SERVER_DATA_ECU_PROGRESS_TYPE_INSTALL	"3"


#define	ADM_IPM_REPORT_HMI_ECU_PROGRESS_WIDE_TIME_MSEC	600U
typedef struct {
    D64U lastTime;
    DCHAR *sessionId;
    DBOOL *ecusIsReportEnd;
    D32S ecusLen;
    ADM_IPM_UIM_T *uim;
    DCHAR *pcConditionResult;
}ADM_IPM_REPORT_HMI_T;

#define	ADM_IPM_REPORT_OTA_SERVER_ECU_PROGRESS_WIDE_TIME_MSEC	(5U * 1000U)
typedef struct {
//    ADM_IRS_SESSION_T *session;
    D64U lastTime;
}ADM_IPM_REPORT_OTA_SERVER_T;

typedef struct {
    ADM_IPM_REPORT_HMI_T reportHmi;
    ADM_IPM_REPORT_OTA_SERVER_T reportOtaServer;
    ADM_IPM_STAT_EVENT_REPORT_T reportStatEvent;
}ADM_IPM_REPORT_T;

typedef struct {
    struct list_head entry;
    ADM_IPS_INSTALL_ECUINFO_T ipsEcu;
    ADM_ECS_ECUCONFIG_T *ecsEcu;
    ADM_PIS_PACKAGE_DATA_T pisData;
    /*D64S endTime;*/
    D64U ecuTimeout;/*!< ms */
    D64U lastTime;  /*!< ms */
    D64U startTime; /*!< ms */
    DBOOL bIsNeedResume;    /** 是否需要恢复安装 */
}ADM_IPM_ECU_INFO_NODE_T;

#define	ADM_IPM_FLASH_GET_IPS_ECU_TIME_USEC_WIDE	(200U * 1000U)
#define ADM_IPM_FLASH_GET_IPS_ECU_TIME_SEC_WIDE     (1U)
#define	ADM_IPM_FLASH_QUERY_ECU_RESULT_TIME_MSEC_WIDE	(60U * 1000U)
#define	ADM_IPM_FLASH_ECU_LIST_STORE_TIME_MSEC_WIDE	(10U * 1000U)

#define	ADM_IPM_DO_FLASH_ECU_LIST_KV_KEY	"ipm_do_flash_ecu_list"

typedef struct ADM_IPM_DO_FLASH_{
    pthread_mutex_t mutex;
    struct list_head head;
    D32S ecusCount;
    DBOOL isNeedGetEcus;
    ADM_IPM_DIM_T *dim;
    ADM_IPM_REPORT_T *report;
    D64U ecuListStoreLastTime;/*!< ms */
}ADM_IPM_DO_FLASH_T;

typedef enum{
    ADM_IPM_FLASH_STATE_IDLE,
    ADM_IPM_FLASH_STATE_VERIFT_TASK,
    ADM_IPM_FLASH_STATE_VERIFY_CONDITION,
    ADM_IPM_FLASH_STATE_ENV_ENTRY,
    ADM_IPM_FLASH_STATE_REMOTE_AUTH_ON,            ///< 远程上电
    ADM_IPM_FLASH_STATE_VERIFT_SIGN_PACKAGE,
    ADM_IPM_FLASH_STATE_VERIFT_ECUS_VERSION,
    ADM_IPM_FLASH_STATE_UPGRADE,
    ADM_IPM_FLASH_STATE_ACTIVE,                     ///< 智能件单独激活流程
    ADM_IPM_FLASH_STATE_POST_UPGRADE,
    ADM_IPM_FLASH_STATE_ENV_EXIT,
    ADM_IPM_FLASH_STATE_REMOTE_AUTH_OFF,            ///< 远程下电
    ADM_IPM_FLASH_STATE_REPORT,
    ADM_IPM_FLASH_STATE_INVALID = 999
}ADM_IPM_FLASH_STATE_E;

/**
 * @brief 刷写阶段枚举
 * @note
 */
typedef enum
{
    IPM_FLASH_STAGE_DEFAULT,        /** 初始阶段 */
    IPM_FLASH_STAGE_RETRY,          /** 重试阶段 */
    IPM_FLASH_STAGE_ROLLBACK,       /** 回滚阶段 */
    IPM_FLASH_STAGE_ROLLBACK_RETRY, /** 回滚重试阶段 */
    IPM_FLASH_STAGE_EXIT,           /** 退出阶段 */
    IPM_FLASH_STAGE_INVALID
} ADM_IPM_FLASH_STAGE_E;

#define	ADM_IPM_FLASH_KV_KEY "ipm_flash"
#define ADM_IPM_ALL_STAGE_RESULT_KV_KEY "ipm_allStageResult"
#define ADM_IPM_NEED_REPEAT_NOTIFY_POLICY_RESULT "ipm_needReNotiPolicy"

typedef struct {
    pthread_mutex_t mutex;
    ADM_IPM_FLASH_STATE_E state;
    ADM_IPM_FLASH_STAGE_E eStage;
    ADM_IPM_DO_FLASH_T doFlash;
    ADM_IPM_REPORT_T *report;
    ADM_IPM_DIM_T *dim;
    D32S result;
}ADM_IPM_FLASH_T;

/**
 * @struct  ADM_QUEUE_T
 * @brief   队列
 */
typedef struct
{
    void**  elements;   // 元素
    D32S    capacity;   // 容量
    D32S    count;      // 元素个数
    D32S    first;      // 起始元素
    D32S    last;       // 结尾元素
} ADM_QUEUE_T;

/**
 * @struct  ADM_MSG_QUEUE_T
 * @brief   消息队列
 */
typedef struct
{
    ADM_QUEUE_T         queue;
    pthread_mutex_t     mutex;
} ADM_MSG_QUEUE_T;

/**
 * @struct  ADM_MQ_MSG_T
 * @brief   消息队列消息
 */
typedef struct
{
    D32S    msgType;        // 消息类型
    D32U    msgDataLen;     // 消息数据长度
    DCHAR   msgData[1];     // 消息数据
} ADM_MQ_MSG_T;


typedef D32S(*ADM_MSG_HANDLER)(void* pMsgData, D32U uiMsgDataLen);
/**
 * @struct  ADM_MSG_HANDLER_T
 * @brief   消息处理者
 */
typedef struct
{
    D32S            msgType;    // 消息类型
    ADM_MSG_HANDLER handler;    // 消息处理函数
} ADM_MSG_HANDLER_T;

/**
 * @struct  ADM_MSG_DISPATCHER_T
 * @brief   消息分发器
 * @details 消息处理分发
 */
typedef struct
{
    ADM_MSG_HANDLER_T   handlers[16];   // 消息处理函数
    D32S                handlerCount;   // handler数量
} ADM_MSG_DISPATCHER_T;

/**
 * @struct  ADM_IPM_MSG_CENTER_T
 * @brief   消息中心
 * @details 
 */
typedef struct
{
    ADM_MSG_QUEUE_T         msgQueue;       // 消息队列
    ADM_MSG_DISPATCHER_T    msgDispatcher;  // 消息分发器
    DBOOL                   bIsOver;        // 消息处理线程是否结束
    sem_t                   sem;            // 用于等待线程结束的信号量
} ADM_IPM_MSG_CENTER_T;

typedef struct {
    ADM_IPM_DIM_T dim;
    ADM_IPM_UIM_T uim;
    ADM_IPM_REPORT_T report;
    ADM_IPM_FLASH_T flash;
    ADM_IPM_MSG_CENTER_T    msgCenter;
}ADM_IPM_CONTEXT_T;

typedef struct {
    D32S verifyVersion;/*!< 升级前是否校验版本号，取值范围：1:校验 2:不校验 */
    /*! upgradeMode为3时，自动安装开始时间必填 格式：yyyy-MM-dd HH:mm:ss */
    D64S upgradeAutoStart;
    D32S upgradeRetryCount;/*!< 升级重试次数 */
    D32S rollbackRetryCount;/*!< 回滚重试次数 */
//    ADM_IPS_POLICY_UPGRADE_CONDITION_ARRAY_T upgradeConditions;/*!< 升级前置条件判断规则 */
    D32S totalTimeConsuming;/*!< 总计预计升级耗时 单位：秒 */
//    ADM_IPS_POLICY_DEPENDENCY_ARRAY_T dependencies;/*!< Ecu升级版本依赖 */
//    ADM_IPS_POLICY_ECU_GROUP_ORDER_ARRAY_T ecuGroupOrders;/*!< 零件组、升级顺序信息 */
}ADM_IPM_POLICY_T;

/** 
 * 各阶段结果存储
*/
typedef struct 
{
    ADM_IPM_FLASH_STATE_E eState;   //当前阶段
    D32S iRet;                      //当前阶段处理的响应码值
} ADM_ALL_STAGE_RESULT_T;

#define ADM_MAX_STAGE_NUMBER 4

typedef struct 
{
    D16U uiIndex;                                                       //当前stAllStageResult填充了几个元素
    DBOOL bIsFlashSuccess;                                              //执行刷写是否成功,若未执行到刷写,默认失败
    ADM_ALL_STAGE_RESULT_T stAllStageResult[ADM_MAX_STAGE_NUMBER];      //当前所有阶段的码值信息
} ADM_ALL_STAGE_RESULT_INFO_T;

static D32S ipmTaskVerify(void);
/* ADM_IPM_CONTEXT_T */
static void ipmContextInit(ADM_IPM_CONTEXT_T *c);
static void  ipmAllStageResultMemoryInit(ADM_ALL_STAGE_RESULT_INFO_T* pstAllStageResult);
static void ipmAllStageResultInit(ADM_ALL_STAGE_RESULT_INFO_T* pstAllStageResult);
static D32S ipmHmiProgressSimulateInit(KvStorage_t* pstOtaKvHandle, D32U* curProgress);
static void ipmContextDeInit(ADM_IPM_CONTEXT_T *c);
//static void ipmContextStop(ADM_IPM_CONTEXT_T *c);
//static void ipmContextProgressGet(ADM_IPM_CONTEXT_T *c);
static D32S ipmContextInstall(ADM_IPM_CONTEXT_T *c);
static void ipmContextSetUimCallback(ADM_IPM_CONTEXT_T *c, ADM_UIM_CMD_DRIVER_CLICK_F *fnCmdDriverClick, void *priv);
static D32S ipmContextUimAllEcuInstallResultNotify(ADM_IPM_CONTEXT_T *c);

/* ADM_IPM_FLASH_T */
static void ipmFlashInit(ADM_IPM_FLASH_T *f, ADM_IPM_DIM_T *dim, ADM_IPM_REPORT_T *report);
static void ipmFlashDeInit(ADM_IPM_FLASH_T *f);
static D32S ipmApkUpgradeRun(ADM_IPM_FLASH_T* f);
static D32S ipmFlashRun(ADM_IPM_FLASH_T *f);
static struct json_object* ipmFlashJsonCreate(const ADM_IPM_FLASH_T *f);
static void ipmFlashJsonParse(ADM_IPM_FLASH_T *f, const struct json_object* root);
static void ipmFlashLoad(ADM_IPM_FLASH_T *f);
static void ipmFlashStore(const ADM_IPM_FLASH_T *f);
static void ipmFlashClear(ADM_IPM_FLASH_T* f);

static struct json_object *ipmAllStageResultJsonCreate(const ADM_ALL_STAGE_RESULT_INFO_T* pstAllStageResult);
static void ipmAllStageResultJsonParse(ADM_ALL_STAGE_RESULT_INFO_T* pstAllStageResult, const struct json_object *root);
static void ipmAllStageResultLoad(ADM_ALL_STAGE_RESULT_INFO_T* pstAllStageResult);
static void ipmAllStageResultStore(const ADM_ALL_STAGE_RESULT_INFO_T* pstAllStageResult);
static void ipmAllStageResultClear(ADM_ALL_STAGE_RESULT_INFO_T* pstAllStageResult);
static void ipmNeedReNotiPolicyResultStore(D32S iSumResult);

/* ADM_IPM_FLASH_STATE_E */
static const DCHAR *ipmFlashStateString(ADM_IPM_FLASH_STATE_E state);

static DVOID ipmGroupFlashSwitchAb(D32U uiGroupId);
static DVOID ipmGroupRollbackSwitchAb(D32U uiGroupId);

/* ADM_IPM_DO_FLASH_T */
static void ipmDoFlashInit(ADM_IPM_DO_FLASH_T *f, ADM_IPM_DIM_T *dim, ADM_IPM_REPORT_T *report);
static void ipmDoFlashDeInit(ADM_IPM_DO_FLASH_T *f);
static void ipmDoFlashClear(ADM_IPM_DO_FLASH_T* f);
static D32S ipmDoFlashEcus(ADM_IPM_DO_FLASH_T *f);
static void ipmDoFlashEcuSetProgress(ADM_IPM_DO_FLASH_T *f, const DCHAR *ecuReqId, const DCHAR *ecuSwDid, D32S progress);
static void ipmDoFlashEcuSetResult(ADM_IPM_DO_FLASH_T *f, const DCHAR *ecuReqId, const DCHAR *ecuSwDid, D32S result, const DCHAR* pResultDescrip);
static void ipmDoFlashEcusVerifyEndTime(ADM_IPM_DO_FLASH_T *f);
static void ipmDoFlashEcusQueryResult(ADM_IPM_DO_FLASH_T *f, D64U *queryTime);
//static void ipmDoFlashTempEcusInit(ADM_IPM_DO_FLASH_T *f);
//static void ipmDoFlashTempEcusDeInit(ADM_IPM_DO_FLASH_T *f);
//static void ipmDoFlashTempEcusClear(ADM_IPM_DO_FLASH_T *f);
static D32S ipmDoFlashEcusGetIps(ADM_IPM_DO_FLASH_T *f, ADM_IPS_INSTALL_ECUINFO_T *buffEcus, D32S buffEcusLen);
static void ipmDoFlashEcuEnd(ADM_IPM_DO_FLASH_T *f, ADM_IPM_ECU_INFO_NODE_T *node, D32S result,const DCHAR* pResultDescrip);
static void ipmDoFlashEcuProgress(ADM_IPM_DO_FLASH_T *f, ADM_IPM_ECU_INFO_NODE_T *node, D32S progress);
static ADM_IPM_ECU_INFO_NODE_T *ipmDoFlashEcusFind(ADM_IPM_DO_FLASH_T *f, const DCHAR *ecuReqId, const DCHAR *ecuSwDid);
static struct json_object* ipmDoFlashEcusListJsonCreate(const ADM_IPM_DO_FLASH_T *f);
static void ipmDoFlashEcusListJsonParse(ADM_IPM_DO_FLASH_T *f, const struct json_object* root);
static void ipmDoFlashEcusListAutoFill(ADM_IPM_DO_FLASH_T *f);
static void ipmDoFlashEcusListLoad(ADM_IPM_DO_FLASH_T *f);
static void ipmDoFlashEcusListStore(ADM_IPM_DO_FLASH_T *f, DBOOL debug);
static void ipmDoFlashEcusListStoreKeep(ADM_IPM_DO_FLASH_T *f, DBOOL force);

/* ADM_IPM_ECU_INFO_NODE_T */
static ADM_IPM_ECU_INFO_NODE_T *ipmEcuInfoNodeNew(const ADM_IPS_INSTALL_ECUINFO_T *ecu);
static void ipmEcuInfoNodeDestroy(ADM_IPM_ECU_INFO_NODE_T** node);
static void ipmEcuInfoNodeDel(ADM_IPM_ECU_INFO_NODE_T *n);
//static D32S ipmEcuInfoNodeFlashAsync(const ADM_IPM_ECU_INFO_NODE_T *n);
static void ipmEcuInfoNodeLastTimeReset(ADM_IPM_ECU_INFO_NODE_T *n);
static DBOOL ipmEcuInfoNodeIsTimeOut(ADM_IPM_ECU_INFO_NODE_T *n);
static ADM_IPM_ECU_INFO_NODE_T *ipmEcuInfoNodeNewWithoutParam(void);
static void ipmEcuInfoNodeAutoFill(ADM_IPM_ECU_INFO_NODE_T *node);
static struct json_object* ipmEcuInfoNodeJsonCreate(const ADM_IPM_ECU_INFO_NODE_T *node);
static ADM_IPM_ECU_INFO_NODE_T* ipmEcuInfoNodeJsonParse(const struct json_object* root);

/* ADM_IPM_REPORT_T */
static void ipmReportInit(ADM_IPM_REPORT_T *r);
static void ipmReportDeInit(ADM_IPM_REPORT_T *r);
//static void ipmReportCondition(ADM_IPM_REPORT_T *r);
static D32S ipmReportOtaServerCancelInstall(ADM_IPM_REPORT_T *r);
static D32S ipmReportOtaServerTaskVerify(ADM_IPM_REPORT_T *r);
static void ipmReportProgressBegin(ADM_IPM_REPORT_T *r);
static void ipmReportProgressEnd(ADM_IPM_REPORT_T *r);
static void ipmReportProgressKeep(ADM_IPM_REPORT_T *r);
static D32S ipmReportResult(ADM_IPM_REPORT_T *r, D32S result);
//static void ipmReportEvent(ADM_IPM_REPORT_T *r, const ADM_OCS_EVENTS_INFO_T* event);
static D32S ipmReportScheduleInstall(ADM_IPM_REPORT_T *r, ADM_APPLICATION_COMMON_CMDID_TYPE_E eType, D64S timestamp,ADM_APPLICATION_SCHEDULE_INSTALL_SOURCE_E eSource);

/* ADM_IPM_REPORT_OTA_SERVER_T */
static void ipmReportOtaServerInit(ADM_IPM_REPORT_OTA_SERVER_T *s);
static void ipmReportOtaServerDeInit(ADM_IPM_REPORT_OTA_SERVER_T *s);
//static void ipmReportOtaServerCondition(ADM_IPM_REPORT_OTA_SERVER_T *s);
static void ipmReportOtaServerProgressBegin(ADM_IPM_REPORT_OTA_SERVER_T *s);
static void ipmReportOtaServerProgressEnd(ADM_IPM_REPORT_OTA_SERVER_T *s);
static void ipmReportOtaServerProgressKeep(ADM_IPM_REPORT_OTA_SERVER_T *s);
static void ipmReportOtaServerProgress(ADM_IPM_REPORT_OTA_SERVER_T *s, DBOOL force);
static void ipmReportOtaServerResult(ADM_IPM_REPORT_OTA_SERVER_T *s);
static DBOOL ipmReportOtaServerProgressTimeOut(ADM_IPM_REPORT_OTA_SERVER_T *s, DBOOL force);
static void ipmReportOtaServerTaskStatus(ADM_IPM_REPORT_OTA_SERVER_T *s, const DCHAR *state, const DCHAR *reason, const DCHAR *exts);
static D32S ipmSyncReportOtaServerTaskStatus(ADM_IPM_REPORT_OTA_SERVER_T *s, const DCHAR *state, const DCHAR *reason, const DCHAR *exts);
static D32S ipmReportOtaServerTaskStatusEnterOtaModeFailed(ADM_IPM_REPORT_OTA_SERVER_T *s);
static D32S ipmReportOtaServerTaskStatusTaskStop(ADM_IPM_REPORT_OTA_SERVER_T *s, D32S iResult);
static D32S ipmReportOtaServerTaskScheduleInstall(ADM_IPM_REPORT_OTA_SERVER_T *s, const DCHAR *exts, int reason);
static void ipmReportOtaServerTaskScheduleInstallFailed(ADM_IPM_REPORT_OTA_SERVER_T *s,int reason);
static void ipmReportOtaServerTaskScheduleInstallCancel(ADM_IPM_REPORT_OTA_SERVER_T *s,int reason);
static void ipmReportOtaServerTaskScheduleInstallCancelFailed(ADM_IPM_REPORT_OTA_SERVER_T *s,int reason);
static D32S ipmReportOtaServerTaskCancelScheduleInstall(ADM_IPM_REPORT_OTA_SERVER_T *s);
//static void ipmReportOtaServerTaskStatusInstallPrepare(ADM_IPM_REPORT_OTA_SERVER_T *s);
static D32S ipmSyncReportOtaServerTaskStatusInstallPrepare(ADM_IPM_REPORT_OTA_SERVER_T *s);
static void ipmReportOtaServerTaskStatusInstallStart(ADM_IPM_REPORT_OTA_SERVER_T *s);
static void ipmReportOtaServerTaskStatusInstallEnd(ADM_IPM_REPORT_OTA_SERVER_T *s);
static void ipmReportOtaServerEcuStatus(ADM_IPM_REPORT_OTA_SERVER_T *s);
static void ipmReportOtaServerEcuProgress(ADM_IPM_REPORT_OTA_SERVER_T *s);
//static void ipmReportOtaServerEvent(ADM_IPM_REPORT_OTA_SERVER_T *s, const ADM_OCS_EVENTS_INFO_T* event);

/* ADM_IPM_REPORT_SERVER_DATA_EXT_LIST_T */
static void ipmReportServerDataExtListInit(ADM_IPM_REPORT_SERVER_DATA_EXT_LIST_T *exts);
static void ipmReportServerDataExtListDeInit(ADM_IPM_REPORT_SERVER_DATA_EXT_LIST_T *exts);
static struct json_object* ipmReportServerDataExtListPackage(const ADM_IPM_REPORT_SERVER_DATA_EXT_LIST_T *exts);
static void ipmReportServerDataExtDeInit(ADM_IPM_REPORT_SERVER_DATA_EXT_T *ext);
static void ipmReportServerDataExtCreate(ADM_IPM_REPORT_SERVER_DATA_EXT_T *ext, const DCHAR *key, DCHAR *value);
static struct json_object* ipmReportServerDataExtPackage(const ADM_IPM_REPORT_SERVER_DATA_EXT_T *ext);
/* ADM_IPM_REPORT_HMI_T */
static void ipmReportHmiInit(ADM_IPM_REPORT_HMI_T *h);
static void ipmReportHmiDeInit(ADM_IPM_REPORT_HMI_T *h);
static void ipmReportHmiProgressBegin(ADM_IPM_REPORT_HMI_T *h);
static void ipmReportHmiProgressEnd(ADM_IPM_REPORT_HMI_T *h);
static void ipmReportHmiProgressKeep(ADM_IPM_REPORT_HMI_T *h);
static void ipmReportHmiProgress(ADM_IPM_REPORT_HMI_T *h, DBOOL force);
static void* ipmReportHmiProgressSimulate(void *arg);
static void ipmReportProgressSimulate(ADM_IPM_FLASH_STATE_E eState, ADM_IPM_REPORT_HMI_T *h);
static D32S ipmReportHmiResult(ADM_IPM_REPORT_HMI_T *h, D32S result);
static DBOOL ipmReportHmiProgressUpdate(ADM_IPM_REPORT_HMI_T *h, const ADM_IPS_PROGRESS_T *progress);
static DBOOL ipmReportHmiProgressTimeOut(ADM_IPM_REPORT_HMI_T *h, DBOOL force);
static void ipmReportHmiProgressClear(ADM_IPM_REPORT_HMI_T *h);

/* ADM_IPM_UIM_T */
static void ipmUimInit(ADM_IPM_UIM_T *uim);
static void ipmUimDeInit(ADM_IPM_UIM_T *uim);
static DBOOL ipmUimConditionsVerified(ADM_IPM_UIM_T *uim, const ADM_UIM_SOA_CONDITIONS_VERIFIED_TYPE_E eVerifiedType, const D16U cycleTime, const D16U timeout);
static void ipmUimSetCallback(ADM_IPM_UIM_T *uim, ADM_UIM_CMD_DRIVER_CLICK_F *fnCmdDriverClick, void *priv);
static void ipmUimSoaInstallConditionsVerifyResultNotify(ADM_IPM_UIM_T *uim, const ADM_UIM_SOA_CONDITION_DATA_ARRAY_T* condsArrarList, const D32S verifyResult);
static void ipmUimSoaPolicyInstallProgressNotify(ADM_IPM_UIM_T *uim, const ADM_IPS_PROGRESS_T *progress);
static D32S ipmUimSoaPolicyInstallResultNotify(ADM_IPM_UIM_T *uim, const ADM_IPS_PROGRESS_T* progress, D32S result, DCHAR *pcConditionResult);
static void ipmUimSoaEcuInstallProgressNotify(ADM_IPM_UIM_T *uim, const ADM_IPS_ECU_PROGRESS_T *progress);
static void ipmUimSoaEcuInstallResultNotify(ADM_IPM_UIM_T *uim, const ADM_IPS_ECU_PROGRESS_T *progress);
static D32S ipmUimAllEcuInstallResultNotify(ADM_IPM_UIM_T *uim, const ADM_IPS_PROGRESS_T *progress, D32S result);

/* ADM_UIM_SOA_ECU_INSTALL_PROGRESS_ARRAY_INFO_T */
static void uimSoaEcuInstallProgressArrayInfoInit(ADM_UIM_SOA_ECU_INSTALL_PROGRESS_ARRAY_INFO_T *uimEcusProgress, const ADM_IPS_ECU_PROGRESS_ARRAY_T *ipsEcusProgress);
//static void uimSoaEcuInstallProgressArrayInfoDeInit(ADM_UIM_SOA_ECU_INSTALL_PROGRESS_ARRAY_INFO_T *uimEcusProgress);
static ADM_UIM_SOA_ECU_INSTALL_PROGRESS_ARRAY_INFO_T* uimSoaEcuInstallProgressArrayInfoNew(const ADM_IPS_ECU_PROGRESS_ARRAY_T *ipsEcusProgress);
//static void uimSoaEcuInstallProgressArrayInfoDel(ADM_UIM_SOA_ECU_INSTALL_PROGRESS_ARRAY_INFO_T *uimEcusProgress);

/* ADM_UIM_SOA_ECU_INSTALL_PROGRESS_INFO_T */
static void uimSoaEcuInstallProgressInfoInit(ADM_UIM_SOA_ECU_INSTALL_PROGRESS_INFO_T *uimEcuProgress, const ADM_IPS_ECU_PROGRESS_T *ipsEcuProgress);
//static void uimSoaEcuInstallProgressInfoDeInit(ADM_UIM_SOA_ECU_INSTALL_PROGRESS_INFO_T *uimEcuProgress);

/* uim soa ecu state */
static const DCHAR* uimSoaEcuState(const ADM_IPS_ECU_PROGRESS_T *progress);

/* ADM_UIM_SOA_ECU_IDEN_INFO_T */
static void uimSoaEcuIdenInfoInit(ADM_UIM_SOA_ECU_IDEN_INFO_T *pstEcuIdenInfo, const ADM_IPS_ECU_PROGRESS_T *ipsEcuProgress);
static void uimSoaEcuIdenInfoDeInit(ADM_UIM_SOA_ECU_IDEN_INFO_T *pstEcuIdenInfo);

/* ADM_UIM_SOA_INSTALL_PROGRESS_INFO_T */
static void uimSoaInstallProgressInfoInitEcu(ADM_UIM_SOA_INSTALL_PROGRESS_INFO_T *pstEcuInstallProgress, const ADM_IPS_ECU_PROGRESS_T *ipsEcuProgress);
static void uimSoaInstallProgressInfoInitVehicle(ADM_UIM_SOA_INSTALL_PROGRESS_INFO_T *pstInstallProgress, const ADM_IPS_PROGRESS_T *ipsProgress);
static void uimSoaInstallProgressInfoDeInit(ADM_UIM_SOA_INSTALL_PROGRESS_INFO_T *pstProgress);
static ADM_UIM_SOA_INSTALL_PROGRESS_INFO_T* uimSoaInstallProgressInfoNewVehicle(const ADM_IPS_PROGRESS_T *ipsProgress);
//static void uimSoaInstallProgressInfoDel(ADM_UIM_SOA_INSTALL_PROGRESS_INFO_T *pstProgress);

/* ADM_APPLICATION_COMMON_CMDID_TYPE_E */
ADM_APPLICATION_COMMON_CMDID_TYPE_E uimSoaCmdidTypeGet(void);

/* uim soa vehicle state */
static const DCHAR* uimSoaVehicleState(const ADM_IPS_PROGRESS_T *progress);

/* uim soa vehicle sessionId */
DCHAR* uimSoaVehicleSessionId(void);

/* uim callback */
//static void ipmUimInstallTriggerCallback(ADM_UIM_SOA_ACTION_AUTH_TYPE_E* eActionType, void* priv);
static void ipmUimConditionsVerifiedCallback(const ADM_UIM_SOA_CONDITIONS_VERIFIED_TYPE_E eVerifiedType, const D16U cycleTime, const D16U timeout, D32S* errorCode, void* priv);
static D32S ipmUimAllEcuInstallProgressCallback(ADM_APPLICATION_COMMON_CMDID_TYPE_E* eCmdidType, DCHAR** sessionId, DCHAR** state, ADM_UIM_SOA_ECU_INSTALL_PROGRESS_ARRAY_INFO_T** pstEcuInstallProgressArrayInfo,  ADM_UIM_SOA_INSTALL_PROGRESS_INFO_T** pstPolicyInstallProgress, D32S* errorCode, void* priv);
static D32S ipmUimAllEcuInstallResultCallback(ADM_APPLICATION_COMMON_CMDID_TYPE_E* eCmdidType, DCHAR** sessionId, DCHAR** state, ADM_UIM_SOA_ECU_INSTALL_RESULT_ARRAY_INFO_T** pstEcuInstallResultArrayInfo, ADM_UIM_SOA_INSTALL_RESULT_INFO_T** pstPolicyInstallResult, D32S* errorCode, void* priv);

/* ADM_UIM_SOA_ECU_INSTALL_RESULT_ARRAY_INFO_T */
static void uimSoaEcuInstallResultArrayInfoInit(ADM_UIM_SOA_ECU_INSTALL_RESULT_ARRAY_INFO_T *pstEcuInstallResultArrayInfo, const ADM_IPS_ECU_PROGRESS_ARRAY_T *ipsEcusProgress);
static void uimSoaEcuInstallResultArrayInfoDeInit(ADM_UIM_SOA_ECU_INSTALL_RESULT_ARRAY_INFO_T *pstEcuInstallResultArrayInfo);
static ADM_UIM_SOA_ECU_INSTALL_RESULT_ARRAY_INFO_T* uimSoaEcuInstallResultArrayInfoNew(const ADM_IPS_ECU_PROGRESS_ARRAY_T *ipsEcusProgress);
//static void uimSoaEcuInstallResultArrayInfoDel(ADM_UIM_SOA_ECU_INSTALL_RESULT_ARRAY_INFO_T *pstEcuInstallResultArrayInfo);

/* ADM_UIM_SOA_ECU_INSTALL_RESULT_INFO_T */
static void uimSoaEcuInstallResultInfoInit(ADM_UIM_SOA_ECU_INSTALL_RESULT_INFO_T *ecuInstallResult, const ADM_IPS_ECU_PROGRESS_T *ipsEcuProgress);
static void uimSoaEcuInstallResultInfoDeInit(ADM_UIM_SOA_ECU_INSTALL_RESULT_INFO_T *ecuInstallResult);

/* ADM_UIM_SOA_INSTALL_RESULT_INFO_T */
static void uimSoaInstallResultInfoInitEcu(ADM_UIM_SOA_INSTALL_RESULT_INFO_T *pstEcuInstallResult, const ADM_IPS_ECU_PROGRESS_T *ipsEcuProgress);
void uimSoaInstallResultInfoInitVehicle(ADM_UIM_SOA_INSTALL_RESULT_INFO_T *pstInstallResult, D32S result);
static void uimSoaInstallResultInfoDeInit(ADM_UIM_SOA_INSTALL_RESULT_INFO_T *pstResult);
static ADM_UIM_SOA_INSTALL_RESULT_INFO_T* uimSoaInstallResultInfoNewVehicle(D32S result);
//static void uimSoaInstallResultInfoDel(ADM_UIM_SOA_INSTALL_RESULT_INFO_T *pstResult);

/* ADM_IPS_ECU_PROGRESS_T ecu index */
static D32S ipsEcuProgressIndexGet(const ADM_IPS_ECU_PROGRESS_T *ecuProgress);

/* ADM_IPM_DIM_T vdi handle */
static D32S ipmDimReflashProgressNotify(const ADM_DIM_SOA_ECU_IDEN_INFO_T* pstIdenInfo, const D16U* reflashProgress, void* priv);
static D32S ipmDimReflashResultSend(const ADM_DIM_SOA_ECU_IDEN_INFO_T* pstIdenInfo, const ADM_DIM_SOA_VDI_REFLASH_RESULT_INFO_T* pstReflashResultSendInfo, void* priv);

/* ADM_IPM_DIM_T ucm handle */
static D32S ipmDimSoaUcmUpgradeProgressNotify(const ADM_DIM_SOA_ECU_IDEN_INFO_T* pstEcuIdenInfo, const D16U progress, void* priv);
static D32S ipmDimSoaUcmUpgradeResultNotify(const ADM_DIM_SOA_ECU_IDEN_INFO_T* pstIdenInfo, const ADM_DIM_SOA_UCL_UPGRADE_RESULT_INFO_T* pstUpgradeResultInfo, void* priv);

static D32S ipmDimSoaUcmUpgradeIccProgressNotify(D32S installType, D32S progress, void* priv);
static D32S ipmDimSoaUcmUpgradeIccResultNotify(D32S installType, D32S result, void* priv);

/* ADM_IPM_DIM_T */
static void ipmDimInit(ADM_IPM_DIM_T *dim, ADM_IPM_DO_FLASH_T *doFlash);
static void ipmDimDeInit(ADM_IPM_DIM_T *dim);
static D32S ipmDimFlashEcuRequest(ADM_IPM_DIM_T *dim, const ADM_IPM_ECU_INFO_NODE_T *node);
static DBOOL ipmDimFlashEcuQueryProgress(ADM_IPM_DIM_T *dim, const ADM_IPM_ECU_INFO_NODE_T *node, D32S *progress);
static DBOOL ipmDimFlashEcuQueryResult(ADM_IPM_DIM_T *dim, const ADM_IPM_ECU_INFO_NODE_T *node, D32S *result, DCHAR **pResultDesc);
//static DBOOL ipmDimEcusPackageDecrypt(ADM_IPM_DIM_T *dim);
//static DBOOL ipmDimEcusPackageVerifySign(ADM_IPM_DIM_T *dim);
static D32S ipmDimEnvEntry(ADM_IPM_DIM_T *dim);
static D32S ipmDimEnvExit(ADM_IPM_DIM_T *dim);
static D32S ipmDimSoaVdiControlVehicleStatus(ADM_IPM_DIM_T *dim, ADM_DIM_SOA_VDI_CTRL_VEHICLE_STATUS_TYPE_E eVechielStatusType);
static D32S ipmDimSoaVdiControlVehicleStatusOTAModeEntry(ADM_IPM_DIM_T *dim);
static D32S ipmDimSoaVdiControlVehicleStatusOTAModeExit(ADM_IPM_DIM_T *dim);
static D32S ipmDimSchduleWakeUpTimeSet(ADM_IPM_DIM_T *dim, const ADM_UIM_SOA_INSTALL_SCHDULE_TYPE_E eSchduleType, D64S schduleTime);
//add by wj 20211208
static D32S ipmDimSchduleWakeUpTimeGet(ADM_IPM_DIM_T *dim, D64S *schduleTime);
//end
static D32S ipmDimSoaVdiPerformSpecDiagnosticTask(ADM_IPM_DIM_T *dim, ADM_DIM_SOA_VDI_SPEC_DIAGNOSTIC_TASK_TYPE_E eSpecDiagTaskType);
static D32S ipmDimSoaVdiPerformSpecDiagnosticEnterDiagMode(ADM_IPM_DIM_T *dim);
static D32S ipmDimSoaVdiPerformSpecDiagnosticExitDiagMode(ADM_IPM_DIM_T *dim);
static D32S ipmDimSoaHostExecOtaTask(ADM_IPM_DIM_T *dim, ADM_DIM_SOA_HOST_OTA_TASK_TYPE_E eOtaTaskType, const D32U keeptime);
static D32S ipmDimSoaHostExecOtaTaskStartWakeUp(ADM_IPM_DIM_T *dim);
static D32S ipmDimSoaHostExecOtaTaskStopWakeUp(ADM_IPM_DIM_T *dim);
//add by wj 20211209
static D32S ipmDimSoaHostVerifyConditionStartWakeUp(ADM_IPM_DIM_T *dim);
//static D32S ipmDimSoaHostVerifyConditionStopWakeUp(ADM_IPM_DIM_T *dim);
//end
static VEHICLE_VOLTAGE_STATE_E ipmGetCurrentHvMode(void);
static D32S ipmSwitchHvMode(const ADM_IPM_ECU_INFO_NODE_T *node,
                            ADM_DIM_SOA_VDI_CTRL_VEHICLE_STATUS_TYPE_E eControlType,
                            VEHICLE_VOLTAGE_STATE_E eExpectState);

DCHAR *ipmDimgetEcuPackageUnzipDir(const DCHAR *ecuReqId,const DCHAR *ecuSwDid);
extern DCHAR* adm_ipm_reportConditionsNotMet(D32S ret, const ADM_COND_POLICY_T* pCondReq, const ADM_UIM_SOA_CONDITION_DATA_ARRAY_T* pConditions);


/* ADM_DIM_SOA_VDI_REFLASH_INFO_ARRAY_T */
static D32S dimSoaVdiReflashInfoArrayInit(ADM_DIM_SOA_VDI_REFLASH_INFO_ARRAY_T *reflashInfoList, const ADM_IPM_ECU_INFO_NODE_T *node);
static void dimSoaVdiReflashInfoArrayDeInit(ADM_DIM_SOA_VDI_REFLASH_INFO_ARRAY_T *reflashInfoList);

/* ADM_DIM_SOA_VDI_IMAGE_INFO_T */
#ifndef ADM_IPM_UNZIP_ECU_PACKAGE
static void dimSoaVdiImageInfoInit(ADM_DIM_SOA_VDI_IMAGE_INFO_T *imageInfo, const ADM_PIS_PACKAGE_DATA_T *pisData);
#endif
static void dimSoaVdiImageInfoDeInit(ADM_DIM_SOA_VDI_IMAGE_INFO_T *imageInfo);

/* ADM_DIM_SOA_VDI_IMAGE_INFO_ARRAY_T */
#ifndef ADM_IPM_UNZIP_ECU_PACKAGE
static D32S dimSoaVdiImageInfoArrayInit(ADM_DIM_SOA_VDI_IMAGE_INFO_ARRAY_T *imageInfoList, const ADM_PIS_PACKAGE_DATA_T *pisData);
#else
static D32S dimSoaVdiImageInfoArrayInit(ADM_DIM_SOA_VDI_IMAGE_INFO_ARRAY_T *imageInfoList, const ADM_IPM_ECU_INFO_NODE_T *node);
#endif
static void dimSoaVdiImageInfoArrayDeInit(ADM_DIM_SOA_VDI_IMAGE_INFO_ARRAY_T *imageInfoList);

/* ADM_DIM_SOA_VDI_REFLASH_INFO_T */
static D32S dimSoaVdiReflashInfoInit(ADM_DIM_SOA_VDI_REFLASH_INFO_T *reflashInfo, const ADM_IPM_ECU_INFO_NODE_T *node);
static void dimSoaVdiReflashInfoDeInit(ADM_DIM_SOA_VDI_REFLASH_INFO_T *reflashInfo);

/* ADM_SYSI4VDI_SA_LIB_INFO_ARRAY_T */
//static void sysi4vdiSaLibInfoArrayInit(ADM_SYSI4VDI_SA_LIB_INFO_ARRAY_T *saLibList);
static void sysi4vdiSaLibInfoArrayDeInit(ADM_SYSI4VDI_SA_LIB_INFO_ARRAY_T *saLibList);

/* ADM_SYSI4VDI_SA_LIB_INFO_T */
//static void sysi4vdiSaLibInfoInit(ADM_SYSI4VDI_SA_LIB_INFO_T *saLib);
static void sysi4vdiSaLibInfoDeInit(ADM_SYSI4VDI_SA_LIB_INFO_T *saLib);

/* ADM_DIM_SOA_ECU_IDEN_INFO_T */
static void dimSoaVdiEcuIdenInfoInit(ADM_DIM_SOA_ECU_IDEN_INFO_T* pstEcuIdenInfo, const ADM_ECS_ECUCONFIG_T *pstEcuCfg);
static void dimSoaVdiEcuIdenInfoDeInit(ADM_DIM_SOA_ECU_IDEN_INFO_T* pstEcuIdenInfo);

/* ADM_DIM_SOA_UCL_UPGRADE_INFO_ARRAY_T */
static D32S dimSoaUcmUpgradeInfoArrayInit(ADM_DIM_SOA_UCL_UPGRADE_INFO_ARRAY_T *upgradeInfoList, const ADM_IPM_ECU_INFO_NODE_T *node);
static void dimSoaUcmUpgradeInfoArrayDeInit(ADM_DIM_SOA_UCL_UPGRADE_INFO_ARRAY_T *upgradeInfoList);

/* ADM_DIM_SOA_UCL_UPGRADE_INFO_T */
static D32S dimSoaUcmUpgradeInfoInit(ADM_DIM_SOA_UCL_UPGRADE_INFO_T *upgradeInfo, const ADM_IPM_ECU_INFO_NODE_T *node);
static void dimSoaUcmUpgradeInfoDeInit(ADM_DIM_SOA_UCL_UPGRADE_INFO_T *upgradeInfo);

/* ADM_DIM_SOA_ECU_IDEN_INFO_T */
static void dimSoaUcmEcuIdenInfoInit(ADM_DIM_SOA_ECU_IDEN_INFO_T *pstEcuIdenInfo, const ADM_IPS_INSTALL_ECUINFO_T *ecu);
static void dimSoaUcmEcuIdenInfoDeInit(ADM_DIM_SOA_ECU_IDEN_INFO_T *pstEcuIdenInfo);

/* ADM_DIM_SOA_UCL_IMAGE_INFO_ARRAY_T */
#if 1//#ifndef ADM_IPM_UNZIP_ECU_PACKAGE	
static D32S dimSoaUcmImageInfoArrayInit(ADM_DIM_SOA_UCL_IMAGE_INFO_ARRAY_T *imageInfoList, const ADM_PIS_PACKAGE_DATA_T *pisData);
#else
static D32S dimSoaUcmImageInfoArrayInit(ADM_DIM_SOA_UCL_IMAGE_INFO_ARRAY_T *imageInfoList, const ADM_IPM_ECU_INFO_NODE_T *node);
#endif

static void dimSoaUcmImageInfoArrayDeInit(ADM_DIM_SOA_UCL_IMAGE_INFO_ARRAY_T *imageInfoList);

/* ADM_DIM_SOA_UCL_IMAGE_INFO_T */
static void dimSoaUcmImageInfoInit(ADM_DIM_SOA_UCL_IMAGE_INFO_T *imageInfo, const ADM_PIS_PACKAGE_DATA_T *pisData);
static void dimSoaUcmImageInfoDeInit(ADM_DIM_SOA_UCL_IMAGE_INFO_T *imageInfo);

/* ADM_IPS_INSTALL_ECUINFO_T */
static void ipsInstallEcuInfoArrayInit(ADM_IPS_INSTALL_ECUINFO_T *ecus, D32S ecusLen);
static void ipsInstallEcuInfoArrayDeInit(ADM_IPS_INSTALL_ECUINFO_T *ecus, D32S ecusLen);

/* ADM_DIM_SOA_UCL_UPGRADE_RESULT_INFO_T */
static void dimSoaUclUpgradeResultInfoDel(ADM_DIM_SOA_UCL_UPGRADE_RESULT_INFO_T *pstUpgradeResultInfo);

/* ADM_DIM_SOA_VDI_REFLASH_RESULT_INFO_T */
static void dimSoaVdiReflashResultInfoDel(ADM_DIM_SOA_VDI_REFLASH_RESULT_INFO_T *pstReflashResultSendInfo);

/* ADM_OCS_ECU_STATE_INFO_LIST_T */
static D32S ocsEcuStateInfoListGetecuDids(ADM_OCS_ECU_STATE_INFO_LIST_T *ecuStateList);
static void ocsEcuStateInfoListInit(ADM_OCS_ECU_STATE_INFO_LIST_T *ecuStateList, const ADM_IPS_PROGRESS_T *progress);
static void ocsEcuStateInfoListDeInit(ADM_OCS_ECU_STATE_INFO_LIST_T *ecuStateList);

/* ADM_OCS_ECU_STATE_INFO_T */
static void ocsEcuStateInfoInit(ADM_OCS_ECU_STATE_INFO_T *ecuState, const ADM_IPS_ECU_PROGRESS_T *progress);
static void ocsEcuStateInfoDeInit(ADM_OCS_ECU_STATE_INFO_T *ecuState);

/* ADM_OCS_ECU_PROGRESS_INFO_LIST_T */
static void ocsEcuProgressInfoListInit(ADM_OCS_ECU_PROGRESS_INFO_LIST_T *ecuProgressList, const ADM_IPS_PROGRESS_T *progress);
static void ocsEcuProgressInfoListDeInit(ADM_OCS_ECU_PROGRESS_INFO_LIST_T *ecuProgressList);

/* ADM_OCS_ECU_PROGRESS_INFO_T */
static void ocsEcuProgressInfoInit(ADM_OCS_ECU_PROGRESS_INFO_T *ecuProgress, const ADM_IPS_ECU_PROGRESS_T *progress);
static void ocsEcuProgressInfoDeInit(ADM_OCS_ECU_PROGRESS_INFO_T *ecuProgress);

/* ADM_OCS_ECU_DID_LIST_T */
//static void ocsEcuDidListInit(ADM_OCS_ECU_DID_LIST_T *ecuDidList);
static void ocsEcuDidListDeInit(ADM_OCS_ECU_DID_LIST_T *ecuDidList);

/* ADM_OCS_ECU_DID_T */
//static void ocsEcuDidInit(ADM_OCS_ECU_DID_T *ecuDid);
static void ocsEcuDidDeInit(ADM_OCS_ECU_DID_T *ecuDid);

/* others */
static D64U ipmMonotonicTimeMSecGet(void) ;
static D32S ipmHtoi(const DCHAR *hex);
//static void ipmSemTimedwaitMsecs(sem_t *sem, D64S msecs);
//static D64S ipmSequenceIdGet();
static DVOID ipmHandleActivePre(ADM_IPM_DO_FLASH_T *f, const ADM_IPS_UCL_ECU_ARRAY_ACTIVE_T *pActiveInfo);
static DVOID ipmHandleActiveIng(const ADM_IPS_UCL_ECU_ARRAY_ACTIVE_T *pActiveInfo);
static DVOID ipmHandleActiveEnd(const ADM_IPS_UCL_ECU_ARRAY_ACTIVE_T *pActiveInfo);

D32S adm_ipm_getUpgradePolicy(ADM_IPM_POLICY_T *policy);
D32S adm_ipm_getUpgradeAutoStartTimstamp(D64S *nTimestamp);


void ipmReportEventDependencyEventId(const DCHAR *pEventId, const DCHAR *pEventDesc, D32S iErroCode);
void ipmReportTriggerInstallEventDependencyTriggerType(ADM_APPLICATION_COMMON_CMDID_TYPE_E eTriggerType);
void adm_ipm_fflushEventReport(void);
D32S adm_ipm_eventInfoInit(void);
void adm_ipm_reportEventDeInit(void);



void ipmStatEventReport(const IPM_STAT_EVENT_T* pStatEvent);
void ipmStatEventReportReport(ADM_IPM_STAT_EVENT_REPORT_T* pReport, const IPM_STAT_EVENT_T* pStatEvent);
void ipmStatEventReportInit(ADM_IPM_STAT_EVENT_REPORT_T* pReport);
void ipmStatEventReportDeInit(ADM_IPM_STAT_EVENT_REPORT_T* pReport);
void ipmStatEventReportEnd(ADM_IPM_STAT_EVENT_REPORT_T* pReport);



D32S adm_ipm_stepVerifyConditionBeforeInstall(ADM_APPLICATION_COMMON_CMDID_TYPE_E eCmdidType);

//const DCHAR* ipmSecondsHourStr(D32U uiSeconds, DCHAR* pOut, D32S iOutSize);


#ifdef __cplusplus
}
#endif

#endif//__ADM_IPM_IMPL_H_
