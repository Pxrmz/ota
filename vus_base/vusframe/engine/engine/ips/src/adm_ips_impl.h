/**
 * @file
 * @brief 安装服务模块
 * @author huhongwei
 * @date 2020-12-30
 * @copyright www.abupdate.com
 */

#ifndef __ADM_IPS_IMPL_H_
#define __ADM_IPS_IMPL_H_

#include "adm_typedefs.h"
#include "adm_ips_install.h"
#include "json-c/json.h"
#include "adm_cds_parse.h"

#ifdef __cplusplus
extern "C" {
#endif

#if 0
typedef struct {
    D32S logic;/*!< 逻辑操作符 取值范围：1:> 2:>= 3:== 4:!= 5:< 6:<= */
    D32S dataType;/*!< 数据类型 取值范围：1:uint8_t 2:uint32_t 3:int32_t 4:uint64_t 5:float 6:string */
    D32S dataLength;/*!< 数据长度 取值范围：[0, 65535] */
    DCHAR *data;/*!< 数据值 */
    D32S connectRelation;/*!< 连接关系 取值范围：1:and 2:or */
}ADM_IPS_POLICY_CONDITION_EXPRESSION_T;
typedef struct {
    D32S type;/*!< 取值范围： 1：一次性 2：周期性 */
    D32S timeout;/*!< 当type是2时，周期处理的超时时间，单位：秒 */
    D32S action;/*!< 1：补电 2：上高压 3：下高压 */
    ADM_IPS_POLICY_CONDITION_EXPRESSION_T expression;/*!< 条件表达式 */
}ADM_IPS_POLICY_CONDITION_ON_FAILsTRATEGY_T;
typedef struct {
    DCHAR *messageId;/*!< 消息索引 取值范围：[0X0000, 0XFFFF] */
    DCHAR *messageName;/*!< 消息名称 */
    DCHAR *messageDesc;/*!< 消息描述 */
    ADM_IPS_POLICY_CONDITION_EXPRESSION_T expression;/*!< 条件表达式 */
    ADM_IPS_POLICY_CONDITION_ON_FAILsTRATEGY_T onFailStrategy;/*!< 条件失败的处理策略 */
}ADM_IPS_POLICY_CONDITION_T;
typedef struct {
    ADM_IPS_POLICY_CONDITION_T *conditions;
    D32S conditionsLen;
}ADM_IPS_POLICY_CONDITION_ARRAY_T;
#endif

#if 0
typedef struct {
    /*! 模式 取值范围： 1：驾驶员立即升级 2：预约升级 3：自动升级 4：静默升级 5：手机立即升级 */
    D32S type;
    ADM_IPS_POLICY_CONDITION_ARRAY_T conditions;/*!< 条件判断逻辑 */
}ADM_IPS_POLICY_UPGRADE_CONDITION_T;
#endif
typedef struct {
    //ADM_IPS_POLICY_UPGRADE_CONDITION_T *upgradeConditions;
    ADM_COND_POLICY_T *upgradeConditions;
    D32S upgradeConditionsLen;
}ADM_IPS_POLICY_UPGRADE_CONDITION_ARRAY_T;

#define	ADM_IPS_ECU_UPGRADE_ORDER_MEMBER_DEFAULT_ID (-0x1)

typedef struct {
    DCHAR *ecuReqId;/*!< 诊断请求id */
    DCHAR *ecuSwDid;/*!< 升级对象软件版本 DID */
#ifdef ADM_IPS_POLICY_UPGRADE_ORDER_OLD
    D32S upgradeOrder;/*!< 零件升级顺序（依据车型配置中的零件顺序） */
#else
    DCHAR* upgradeOrder;/*!< 零件升级顺序（依据车型配置中的零件顺序） */
#endif//ADM_IPS_POLICY_UPGRADE_ORDER_OLD
    D32S activeOrder;
    D32S ecuTimeConsuming;/*!< Ecu 预计升级耗时，单位：S */
    D32S ecuTimeout;/*!< Ecu 升级超时时间，单位：S */
}ADM_IPS_POLICY_UPGRADE_ECU_T;
typedef struct {
    ADM_IPS_POLICY_UPGRADE_ECU_T *ecus;
    D32S ecusLen;
}ADM_IPS_POLICY_UPGRADE_ECU_ARRAY_T;
typedef struct {
    /*! 零件分组标识，若多个零件的 groupFlag 若相同，则表明这多个零件属于同一个零件组 */
    D32S groupFlag;
    D32S actionForFailed;/*!< 1：rollback 2：stop 3：continue */
    ADM_IPS_POLICY_UPGRADE_ECU_ARRAY_T groupEcus;
}ADM_IPS_POLICY_ECU_GROUP_ORDER_T;
typedef struct {
    ADM_IPS_POLICY_ECU_GROUP_ORDER_T *ecuGroupOrders;
    D32S ecuGroupOrdersLen;
}ADM_IPS_POLICY_ECU_GROUP_ORDER_ARRAY_T;
typedef struct {
    D32S verifyVersion;/*!< 升级前是否校验版本号，取值范围：1:校验 2:不校验 */
    /*! upgradeMode为3时，自动安装开始时间必填 格式：yyyy-MM-dd HH:mm:ss */
    D64S upgradeAutoStart;
    D32S upgradeRetryCount;/*!< 升级重试次数 */
    D32S rollbackRetryCount;/*!< 回滚重试次数 */
    ADM_IPS_POLICY_UPGRADE_CONDITION_ARRAY_T upgradeConditions;/*!< 升级前置条件判断规则 */
    D32S totalTimeConsuming;/*!< 总计预计升级耗时 单位：秒 */
    ADM_IPS_POLICY_DEPENDENCY_ARRAY_T dependencies;/*!< Ecu升级版本依赖 */
    ADM_IPS_POLICY_ECU_GROUP_ORDER_ARRAY_T ecuGroupOrders;/*!< 零件组、升级顺序信息 */
    D32S parallelNumLimitM;             // ECU并行数量限制M
    D32S parallelNumLimitN;             // ECU并行数量限制N
}ADM_IPS_POLICY_T;

typedef struct {
    D32S groupIndex;
    D32S memberIndex;
#ifdef ADM_IPS_POLICY_UPGRADE_ORDER_OLD
    D32S upgradeOrder;
#else
    DCHAR *upgradeOrder;
#endif//ADM_IPS_POLICY_UPGRADE_ORDER_OLD
}ADM_IPS_ECU_INDEX_T;

typedef struct {
    ADM_IPS_ECU_INDEX_T *indexs;
    D32S indexsLen;
}ADM_IPS_ECU_INDEX_ARRAY_T;

typedef struct {
    ADM_IPS_ECU_INDEX_ARRAY_T indexs;
}ADM_IPS_ECU_SEQUENCE_GROUP_T;

typedef struct {
    ADM_IPS_ECU_SEQUENCE_GROUP_T *ecus;
    D32S ecusLen;
    D32S current;
}ADM_IPS_ECU_SEQUENCE_GROUP_ARRAY_T;

typedef struct {
    ADM_IPS_ECU_SEQUENCE_GROUP_ARRAY_T ecusSequenceGroup;
}ADM_IPS_ECU_NET_GROUP_T;

typedef struct {
    ADM_IPS_ECU_NET_GROUP_T *ecus;
    D32S ecusLen;
    //D32S current;
}ADM_IPS_ECU_NET_GROUP_ARRAY_T;

typedef struct {
    ADM_IPS_ECU_NET_GROUP_ARRAY_T ecusNetGroup;
}ADM_IPS_ECU_UPGRADE_GROUP_T;

typedef struct {
    ADM_IPS_ECU_UPGRADE_GROUP_T *ecus;
    D32S ecusLen;
    D32S current;
    DBOOL isRollback;
}ADM_IPS_ECU_UPGRADE_GROUP_ARRAY_T;

typedef struct {
    ADM_IPS_ECU_INDEX_T index;
    D32S upgradeId;
    D32S netId;
    D32S sequenceId;
}ADM_IPS_ECU_UPGRADE_ORDER_T;

typedef enum{
    ADM_IPS_ECU_UPGRADE_ORDER_GROUP_ID_UPGRADE,
    ADM_IPS_ECU_UPGRADE_ORDER_GROUP_ID_NET,
    ADM_IPS_ECU_UPGRADE_ORDER_GROUP_ID_SEQUENCE,
}ADM_IPS_ECU_UPGRADE_ORDER_GROUP_ID_E;

typedef struct {
    ADM_IPS_ECU_UPGRADE_ORDER_T* ecus;
    D32S ecusLen;
    ADM_IPS_ECU_UPGRADE_ORDER_T** pEcus;
}ADM_IPS_ECU_UPGRADE_ORDER_ARRAY_T;


/**
 * @struct  ADM_IPS_ARRAY_T
 * @brief   数组
 */
typedef struct
{
    DVOID*  eles;       // 数组元素
    D32S    eleSize;    // 元素大小
    D32S    num;        // 元素个数
} ADM_IPS_ARRAY_T;

/**
 * @struct  IPS_UPGRADE_GROUP_FUNC_T
 * @brief   升级组操作功能函数
 */
struct UPGRADE_GROUP;
typedef void(*UPGRADE_GROUP_DEINIT)(struct UPGRADE_GROUP* group);
typedef D32S(*UPGRADE_GROUP_FETCH)(struct UPGRADE_GROUP* group, void* param, D32S maxNum, DBOOL reverse);
typedef struct
{
    UPGRADE_GROUP_FETCH     fetch;      // 获取一个或多个元素
    UPGRADE_GROUP_DEINIT    deinit;     // 清理元素
} IPS_UPGRADE_GROUP_FUNC_T;

/**
 * @struct  IPS_UPGRADE_GROUP_T
 * @brief   升级组
 * @details 
 */
typedef struct UPGRADE_GROUP
{
    IPS_UPGRADE_GROUP_FUNC_T*   func;
    ADM_IPS_ARRAY_T             array;
} IPS_UPGRADE_GROUP_T;

/**
 * @struct  ADM_IPS_ECU_UPGRADE_GROUP_MANAGER_T
 * @brief   升级组管理
 */
typedef struct
{
    DBOOL                   isRollback;     // 是否是回滚阶段，[DFALSE]否 [DTRUE]是
    IPS_UPGRADE_GROUP_T*    group;          // 升级组
}ADM_IPS_ECU_UPGRADE_GROUP_MANAGER_T;

typedef struct ADM_IPS_CONTEXT_{
    pthread_mutex_t mutex;
    ADM_IPS_POLICY_TASK_T task;
    ADM_IPS_POLICY_T policy;
    ADM_IPS_PROGRESS_T progress;
    // ADM_IPS_ECU_UPGRADE_GROUP_ARRAY_T   groups;
    ADM_IPS_ECU_UPGRADE_GROUP_MANAGER_T upgradeGroupManager;
    D32S cmdId;
}ADM_IPS_CONTEXT_T;

/* ADM_IPS_CONTEXT_T */
static void ipsContextInit(ADM_IPS_CONTEXT_T *c);
static void ipsContextDeInit(ADM_IPS_CONTEXT_T *c);
static void ipsContextJsonParse(ADM_IPS_CONTEXT_T *c, const struct json_object* root);
static struct json_object* ipsContextJsonCreate(const ADM_IPS_CONTEXT_T *c);
static void ipsContextStepInstallPrepare(ADM_IPS_CONTEXT_T *c, const ADM_CVS_VERSION_POLICY_T* versionPolicy, D32S parallelNumLimitM, D32S parallelNumLimitN);
static void ipsContextStepInstallStart(ADM_IPS_CONTEXT_T *c);
static void ipsContextStepPostInstall(ADM_IPS_CONTEXT_T *c, D32S result);
static D32S ipsContextGetPolicyTask(ADM_IPS_CONTEXT_T *c, ADM_IPS_POLICY_TASK_T *task);
static D32S ipsContextGetInstallEcus(ADM_IPS_CONTEXT_T *c, ADM_IPS_INSTALL_ECUINFO_T *ecus, D32S ecuMaxNum);
static void ipsContextSetInstallEcuProgress(ADM_IPS_CONTEXT_T *c, const ADM_IPS_ECU_CURRENT_PROGRESS_T *progress);
static void ipsContextSetEcuProgressAbInfo(ADM_IPS_CONTEXT_T *c, const ADM_IPS_ECU_PROGRESS_T *progress);
static D32S ipsContextGetMaxEcusCount(ADM_IPS_CONTEXT_T *c);
static void ipsContextGetProgress(ADM_IPS_CONTEXT_T *c, ADM_IPS_PROGRESS_T *progress);
static D32S ipsContextGetEcuProgress(ADM_IPS_CONTEXT_T *c, const DCHAR *ecuReqId, const DCHAR *ecuSwDid, ADM_IPS_ECU_PROGRESS_T *progress);
static D32S ipsContextGetCmdId(ADM_IPS_CONTEXT_T *c);
static void ipsContextSetCmdId(ADM_IPS_CONTEXT_T *c, D32S cmdId);
static D32S ipsContextGetPolicyCondition(ADM_IPS_CONTEXT_T *c, D32S iType, ADM_COND_POLICY_T *upgradeConditions);
static void ipsContextGetPolicyDependencies(ADM_IPS_CONTEXT_T *c, ADM_IPS_POLICY_DEPENDENCY_ARRAY_T *dependencies);
static void ipsContextGetPolicyDependenciesWithType(ADM_IPS_CONTEXT_T *c, const D32S type, ADM_IPS_POLICY_DEPENDENCY_ARRAY_T *dependencies);
static D32S ipsContextGetPolicyDependenciesEcuDid(ADM_IPS_CONTEXT_T *c,
	const D32S type, const DCHAR *ecuReqId, const DCHAR *ecuSwDid,
	const DCHAR *did, 
	ADM_IPS_POLICY_ECU_DID_T *ecuDid);
#define	ADM_IPS_CONTEXT_KV_KEY	"ips_context"
static void ipsContextLoad(ADM_IPS_CONTEXT_T *c);
static void ipsContextStoreSimple(const ADM_IPS_CONTEXT_T *c, const DCHAR *cStr);
#define	IPS_CONTEXT_EXEC_FLAG_STORE	(0x1)
#define	IPS_CONTEXT_EXEC_FLAG_SHOW	(0x1 << 1)
static void IPS_CONTEXT_EXEC(ADM_IPS_CONTEXT_T *context, D32S flags);

/* ADM_IPS_ECU_UPGRADE_ORDER_T */
static void ipsEcuUpgradeOrderInit(ADM_IPS_ECU_UPGRADE_ORDER_T *u);
static void ipsEcuUpgradeOrderDeInit(ADM_IPS_ECU_UPGRADE_ORDER_T *u);
static void ipsEcuUpgradeOrderCreate(ADM_IPS_ECU_UPGRADE_ORDER_T *u, D32S groupIndex, D32S memberIndex, const ADM_IPS_POLICY_UPGRADE_ECU_T *ecu);
static D32S ipsEcuUpgradeOrderCompare(ADM_IPS_ECU_UPGRADE_ORDER_T *a, ADM_IPS_ECU_UPGRADE_ORDER_T *b);
static void ipsEcuUpgradeOrderBubbleSort(ADM_IPS_ECU_UPGRADE_ORDER_T *us[], D32S n);
static D32S ipsEcuUpgradeOrderGetGroupCount(ADM_IPS_ECU_UPGRADE_ORDER_T *const us[], D32S n, ADM_IPS_ECU_UPGRADE_ORDER_GROUP_ID_E type);

/* ADM_IPS_ECU_UPGRADE_ORDER_GROUP_ID_E */
static D32S ipsEcuUpgradeOrderGetGroup(ADM_IPS_ECU_UPGRADE_ORDER_T *const us[], D32S n, ADM_IPS_ECU_UPGRADE_ORDER_GROUP_ID_E type);

/* ADM_IPS_ECU_UPGRADE_ORDER_ARRAY_T */
static void ipsEcuUpgradeOrderArrayInit(ADM_IPS_ECU_UPGRADE_ORDER_ARRAY_T *u);
static void ipsEcuUpgradeOrderArrayDeInit(ADM_IPS_ECU_UPGRADE_ORDER_ARRAY_T *u);
static void ipsEcuUpgradeOrderArrayCreate(ADM_IPS_ECU_UPGRADE_ORDER_ARRAY_T *u, const ADM_IPS_POLICY_T *policy);

/* ADM_IPS_ECU_UPGRADE_GROUP_ARRAY_T */
static void ipsEcuUpgradeGroupArrayInit(ADM_IPS_ECU_UPGRADE_GROUP_ARRAY_T *g);
static void ipsEcuUpgradeGroupArrayDeInit(ADM_IPS_ECU_UPGRADE_GROUP_ARRAY_T *g);
static void ipsEcuUpgradeGroupArrayClear(ADM_IPS_ECU_UPGRADE_GROUP_ARRAY_T *g);
static void ipsEcuUpgradeGroupArrayJsonParse(ADM_IPS_ECU_UPGRADE_GROUP_ARRAY_T *g, const struct json_object *root);
static struct json_object *ipsEcuUpgradeGroupArrayJsonCreate(const ADM_IPS_ECU_UPGRADE_GROUP_ARRAY_T *g);
static void ipsEcuUpgradeGroupArrayCreate(ADM_IPS_ECU_UPGRADE_GROUP_ARRAY_T *g, const ADM_IPS_POLICY_T *policy);
static D32S ipsEcuUpgradeGroupArrayGetInstallEcusCycle(ADM_IPS_ECU_UPGRADE_GROUP_ARRAY_T *g, const ADM_IPS_POLICY_T *policy, ADM_IPS_PROGRESS_T *progress, ADM_IPS_INSTALL_ECUINFO_T *ecus);
static D32S ipsEcuUpgradeGroupArrayGetInstallEcus(ADM_IPS_ECU_UPGRADE_GROUP_ARRAY_T *g, const ADM_IPS_POLICY_T *policy, ADM_IPS_PROGRESS_T *progress, ADM_IPS_INSTALL_ECUINFO_T *ecus);

/* ADM_IPS_ECU_UPGRADE_GROUP_T */
static void ipsEcuUpgradeGroupInit(ADM_IPS_ECU_UPGRADE_GROUP_T *g);
static void ipsEcuUpgradeGroupDeInit(ADM_IPS_ECU_UPGRADE_GROUP_T *g);
static void ipsEcuUpgradeGroupJsonParse(ADM_IPS_ECU_UPGRADE_GROUP_T *g, const struct json_object *root);
static struct json_object *ipsEcuUpgradeGroupJsonCreate(const ADM_IPS_ECU_UPGRADE_GROUP_T *g);
static void ipsEcuUpgradeGroupCreate(ADM_IPS_ECU_UPGRADE_GROUP_T *g, ADM_IPS_ECU_UPGRADE_ORDER_T *const us[], D32S n);
static D32S ipsEcuUpgradeGroupGetInstallEcus(ADM_IPS_ECU_UPGRADE_GROUP_T *g, const ADM_IPS_POLICY_T *policy, ADM_IPS_PROGRESS_T *progress, ADM_IPS_INSTALL_ECUINFO_T *ecus, DBOOL isRollback);

/* ADM_IPS_ECU_NET_GROUP_ARRAY_T */
static void ipsEcuNetGroupArrayInit(ADM_IPS_ECU_NET_GROUP_ARRAY_T *g);
static void ipsEcuNetGroupArrayDeInit(ADM_IPS_ECU_NET_GROUP_ARRAY_T *g);
static void ipsEcuNetGroupArrayJsonParse(ADM_IPS_ECU_NET_GROUP_ARRAY_T *g, const struct json_object *root);
static struct json_object* ipsEcuNetGroupArrayJsonCreate(const ADM_IPS_ECU_NET_GROUP_ARRAY_T *g);
static void ipsEcuNetGroupArrayCreate(ADM_IPS_ECU_NET_GROUP_ARRAY_T *g, ADM_IPS_ECU_UPGRADE_ORDER_T *const us[], D32S n);
static D32S ipsEcuNetGroupArrayGetInstallEcus(ADM_IPS_ECU_NET_GROUP_ARRAY_T *g, const ADM_IPS_POLICY_T *policy, ADM_IPS_PROGRESS_T *progress, ADM_IPS_INSTALL_ECUINFO_T *ecus, DBOOL isRollback);

/* ADM_IPS_ECU_NET_GROUP_T */
static void ipsEcuNetGroupInit(ADM_IPS_ECU_NET_GROUP_T *g);
static void ipsEcuNetGroupDeInit(ADM_IPS_ECU_NET_GROUP_T *g);
static void ipsEcuNetGroupJsonParse(ADM_IPS_ECU_NET_GROUP_T *g, const struct json_object* root);
static struct json_object* ipsEcuNetGroupJsonCreate(const ADM_IPS_ECU_NET_GROUP_T *g);
static void ipsEcuNetGroupCreate(ADM_IPS_ECU_NET_GROUP_T *g, ADM_IPS_ECU_UPGRADE_ORDER_T *const us[], D32S n);
static D32S ipsEcuNetGroupGetInstallEcus(ADM_IPS_ECU_NET_GROUP_T *g, const ADM_IPS_POLICY_T *policy, ADM_IPS_PROGRESS_T *progress, ADM_IPS_INSTALL_ECUINFO_T *ecus, DBOOL isRollback);

/* ADM_IPS_ECU_SEQUENCE_GROUP_ARRAY_T */
static void ipsEcuSequenceGroupArrayInit(ADM_IPS_ECU_SEQUENCE_GROUP_ARRAY_T *g);
static void ipsEcuSequenceGroupArrayDeInit(ADM_IPS_ECU_SEQUENCE_GROUP_ARRAY_T *g);
static void ipsEcuSequenceGroupArrayJsonParse(ADM_IPS_ECU_SEQUENCE_GROUP_ARRAY_T *g, const struct json_object *root);
static struct json_object* ipsEcuSequenceGroupArrayJsonCreate(const ADM_IPS_ECU_SEQUENCE_GROUP_ARRAY_T *g);
static void ipsEcuSequenceGroupArrayCreate(ADM_IPS_ECU_SEQUENCE_GROUP_ARRAY_T *g, ADM_IPS_ECU_UPGRADE_ORDER_T *const us[], D32S n);
static D32S ipsEcuSequenceGroupArrayGetInstallEcus(ADM_IPS_ECU_SEQUENCE_GROUP_ARRAY_T *g, const ADM_IPS_POLICY_T *policy, ADM_IPS_PROGRESS_T *progress, ADM_IPS_INSTALL_ECUINFO_T *ecus, DBOOL isRollback);

/* ADM_IPS_ECU_SEQUENCE_GROUP_T */
static void ipsEcuSequenceGroupInit(ADM_IPS_ECU_SEQUENCE_GROUP_T *g);
static void ipsEcuSequenceGroupDeInit(ADM_IPS_ECU_SEQUENCE_GROUP_T *g);
static void ipsEcuSequenceGroupJsonParse(ADM_IPS_ECU_SEQUENCE_GROUP_T *g, const struct json_object *root);
static struct json_object* ipsEcuSequenceGroupJsonCreate(const ADM_IPS_ECU_SEQUENCE_GROUP_T *g);
static void ipsEcuSequenceGroupCreate(ADM_IPS_ECU_SEQUENCE_GROUP_T *g, ADM_IPS_ECU_UPGRADE_ORDER_T *const us[], D32S n);
static D32S ipsEcuSequenceGroupGetInstallEcus(ADM_IPS_ECU_SEQUENCE_GROUP_T *g, const ADM_IPS_POLICY_T *policy, ADM_IPS_PROGRESS_T *progress, ADM_IPS_INSTALL_ECUINFO_T *ecus, DBOOL isRollback);

/* ADM_IPS_ECU_INDEX_ARRAY_T */
static void ipsEcuIndexArrayInit(ADM_IPS_ECU_INDEX_ARRAY_T *ia);
static void ipsEcuIndexArrayDeInit(ADM_IPS_ECU_INDEX_ARRAY_T *ia);
static void ipsEcuIndexArrayJsonParse(ADM_IPS_ECU_INDEX_ARRAY_T *ia, const struct json_object* root);
static struct json_object* ipsEcuIndexArrayJsonCreate(const ADM_IPS_ECU_INDEX_ARRAY_T *ia);
static void ipsEcuIndexArrayCreate(ADM_IPS_ECU_INDEX_ARRAY_T *ia, ADM_IPS_ECU_UPGRADE_ORDER_T *const us[], D32S n);
static D32S ipsEcuIndexArrayGetInstallEcus(ADM_IPS_ECU_INDEX_ARRAY_T *ia, const ADM_IPS_POLICY_T *policy, ADM_IPS_PROGRESS_T *progress, ADM_IPS_INSTALL_ECUINFO_T *ecus, DBOOL isRollback);

/* ADM_IPS_ECU_INDEX_T */
static void ipsEcuIndexInit(ADM_IPS_ECU_INDEX_T *in);
static void ipsEcuIndexDeInit(ADM_IPS_ECU_INDEX_T *in);
static void ipsEcuIndexClone(const ADM_IPS_ECU_INDEX_T *src, ADM_IPS_ECU_INDEX_T *dst);
static void ipsEcuIndexJsonParse(ADM_IPS_ECU_INDEX_T *in, const json_object* root);
static struct json_object* ipsEcuIndexJsonCreate(const ADM_IPS_ECU_INDEX_T *in);
#ifdef ADM_IPS_POLICY_UPGRADE_ORDER_OLD
static void ipsEcuIndexCreate(ADM_IPS_ECU_INDEX_T *in, D32S groupIndex, D32S memberIndex, D32S upgradeOrder);
#else
static void ipsEcuIndexCreate(ADM_IPS_ECU_INDEX_T *in, D32S groupIndex, D32S memberIndex, const DCHAR* upgradeOrder);
#endif//ADM_IPS_POLICY_UPGRADE_ORDER_OLD
static DBOOL ipsEcuIndexGetInstallEcu(ADM_IPS_ECU_INDEX_T *in, const ADM_IPS_POLICY_T *policy, ADM_IPS_PROGRESS_T *progress, ADM_IPS_INSTALL_ECUINFO_T *ecu, DBOOL isRollback);

/* ADM_IPS_PROGRESS_T */
static void ipsProgressInit(ADM_IPS_PROGRESS_T *p);
static void ipsProgressDeInit(ADM_IPS_PROGRESS_T *p);
static void ipsProgressClone(const ADM_IPS_PROGRESS_T *src, ADM_IPS_PROGRESS_T *dst);
static void ipsProgressClear(ADM_IPS_PROGRESS_T *p);
static void ipsProgressJsonParse(ADM_IPS_PROGRESS_T *progress, const struct json_object* root);
static struct json_object* ipsProgressJsonCreate(const ADM_IPS_PROGRESS_T *progress);
static void ipsProgressCreate(ADM_IPS_PROGRESS_T *p, const ADM_IPS_POLICY_TASK_T *task, const ADM_IPS_POLICY_T *policy);
static void ipsProgressInstallPrepare(ADM_IPS_PROGRESS_T *p);
static void ipsProgressInstallStart(ADM_IPS_PROGRESS_T *p);
static void ipsProgressPostInstall(ADM_IPS_PROGRESS_T *p, D32S result);
static DBOOL ipsProgressEcuSet(ADM_IPS_PROGRESS_T *p, const ADM_IPS_ECU_CURRENT_PROGRESS_T *progress, const ADM_IPS_POLICY_T *policy);
static ADM_IPS_ECU_PROGRESS_T *ipsProgressFindEcu(ADM_IPS_PROGRESS_T *p, const DCHAR *ecuReqId, const DCHAR *ecuSwDid);
static void ipsProgressUpdateProgress(ADM_IPS_PROGRESS_T *progress);

/* ADM_IPS_ECU_PROGRESS_ARRAY_T */
static void ipsEcuProgressArrayInit(ADM_IPS_ECU_PROGRESS_ARRAY_T *p);
static void ipsEcuProgressArrayDeInit(ADM_IPS_ECU_PROGRESS_ARRAY_T *p);
static void ipsEcuProgressArrayClone(const ADM_IPS_ECU_PROGRESS_ARRAY_T *src, ADM_IPS_ECU_PROGRESS_ARRAY_T *dst);
static void ipsEcuProgressArrayJsonParse(ADM_IPS_ECU_PROGRESS_ARRAY_T *p, const struct json_object* root);
static struct json_object* ipsEcuProgressArrayJsonCreate(const ADM_IPS_ECU_PROGRESS_ARRAY_T *p);
static void ipsEcuProgressArrayCreate(ADM_IPS_ECU_PROGRESS_ARRAY_T *p, const ADM_IPS_POLICY_T *policy);

/* ADM_IPS_ECU_PROGRESS_T */
static void ipsEcuProgressInit(ADM_IPS_ECU_PROGRESS_T *p);
static void ipsEcuProgressDeInit(ADM_IPS_ECU_PROGRESS_T *p);
static void ipsEcuProgressClone(const ADM_IPS_ECU_PROGRESS_T *src, ADM_IPS_ECU_PROGRESS_T *dst);
static void ipsEcuProgressJsonParse(ADM_IPS_ECU_PROGRESS_T *p, const struct json_object* root);
static struct json_object* ipsEcuProgressJsonCreate(const ADM_IPS_ECU_PROGRESS_T *p);
static void ipsEcuProgressCreate(ADM_IPS_ECU_PROGRESS_T *p, const ADM_IPS_POLICY_UPGRADE_ECU_T *ecu, D32S totalEcuTimeConsuming);
static void ipsEcuProgressSetPrecent(ADM_IPS_ECU_PROGRESS_T *p, D32S precent);
static DBOOL ipsEcuProgressGetInstallEcu(ADM_IPS_ECU_PROGRESS_T *p, const ADM_IPS_POLICY_UPGRADE_ECU_T *upgradeEcu, ADM_IPS_INSTALL_ECUINFO_T *ecu, DBOOL isRollback);
static DBOOL ipsEcuProgressSetProgress(ADM_IPS_ECU_PROGRESS_T *p, const ADM_IPS_ECU_CURRENT_PROGRESS_T *progress, ADM_IPS_PROGRESS_T *totalp);
static void ipsEcuProgressSetResultActionForFailed(ADM_IPS_ECU_PROGRESS_T *p, const ADM_IPS_POLICY_T *policy, ADM_IPS_PROGRESS_T *totalp);
static void ipsEcuProgressSetResult(ADM_IPS_ECU_PROGRESS_T *p, const ADM_IPS_ECU_CURRENT_PROGRESS_T *progress, const ADM_IPS_POLICY_T *policy, ADM_IPS_PROGRESS_T *totalp);
/*! group action for faild rollback */
static void ipsEcuProgressNeedRollback(ADM_IPS_ECU_PROGRESS_T *p);
/*! group action for faild stop */
static void ipsEcuProgressNeedStopUpgrade(ADM_IPS_ECU_PROGRESS_T *p);

/* ADM_IPS_POLICY_T */
static void ipsPolicyInit(ADM_IPS_POLICY_T *p);
static void ipsPolicyDeInit(ADM_IPS_POLICY_T *p);
static void ipsPolicyClear(ADM_IPS_POLICY_T *p);
static void ipsPolicyJsonParse(ADM_IPS_POLICY_T *p, const struct json_object *root);
static struct json_object* ipsPolicyJsonCreate(const ADM_IPS_POLICY_T *p);
static D32S ipsPolicyGetMaxEcusCount(const ADM_IPS_POLICY_T *p);
static const ADM_IPS_POLICY_UPGRADE_ECU_T *ipsPolicyFindUpgradeEcu(const ADM_IPS_POLICY_T *p, const ADM_IPS_ECU_INDEX_T *index);
static const ADM_IPS_POLICY_ECU_GROUP_ORDER_T *ipsPolicyFindGroupEcu(const ADM_IPS_POLICY_T *p, const DCHAR *ecuReqId, const DCHAR *ecuSwDid);

/* ADM_IPS_POLICY_ECU_GROUP_ORDER_ARRAY_T */
static void ipsPolicyEcuGroupOrderArrayInit(ADM_IPS_POLICY_ECU_GROUP_ORDER_ARRAY_T *p);
static void ipsPolicyEcuGroupOrderArrayDeInit(ADM_IPS_POLICY_ECU_GROUP_ORDER_ARRAY_T *p);
static void ipsPolicyEcuGroupOrderArrayJsonParse(ADM_IPS_POLICY_ECU_GROUP_ORDER_ARRAY_T *p, const struct json_object* root);
static struct json_object* ipsPolicyEcuGroupOrderArrayJsonCreate(const ADM_IPS_POLICY_ECU_GROUP_ORDER_ARRAY_T *p);
static D32S ipsPolicyEcuGroupOrderArrayGetTotalEcuTimeConsuming(const ADM_IPS_POLICY_ECU_GROUP_ORDER_ARRAY_T *p);

/* ADM_IPS_POLICY_ECU_GROUP_ORDER_T */
static void ipsPolicyEcuGroupOrderInit(ADM_IPS_POLICY_ECU_GROUP_ORDER_T *policy);
static void ipsPolicyEcuGroupOrderDeInit(ADM_IPS_POLICY_ECU_GROUP_ORDER_T *policy);
static void ipsPolicyEcuGroupOrderJsonParse(ADM_IPS_POLICY_ECU_GROUP_ORDER_T *policy, const struct json_object* root);
static struct json_object* ipsPolicyEcuGroupOrderJsonCreate(const ADM_IPS_POLICY_ECU_GROUP_ORDER_T *policy);

/* ADM_IPS_POLICY_UPGRADE_ECU_ARRAY_T */
static void ipsPolicyUpgradeEcuArrayInit(ADM_IPS_POLICY_UPGRADE_ECU_ARRAY_T *policy);
static void ipsPolicyUpgradeEcuArrayDeInit(ADM_IPS_POLICY_UPGRADE_ECU_ARRAY_T *policy);
static void ipsPolicyUpgradeEcuArrayJsonParse(ADM_IPS_POLICY_UPGRADE_ECU_ARRAY_T *policy, const struct json_object* root);
static struct json_object* ipsPolicyUpgradeEcuArrayJsonCreate(const ADM_IPS_POLICY_UPGRADE_ECU_ARRAY_T *policy);

/* ADM_IPS_POLICY_UPGRADE_ECU_T */
static void ipsPolicyUpgradeEcuInit(ADM_IPS_POLICY_UPGRADE_ECU_T *policy);
static void ipsPolicyUpgradeEcuDeInit(ADM_IPS_POLICY_UPGRADE_ECU_T *policy);
static void ipsPolicyUpgradeEcuJsonParse(ADM_IPS_POLICY_UPGRADE_ECU_T *policy, const struct json_object* root);
static struct json_object* ipsPolicyUpgradeEcuJsonCreate(const ADM_IPS_POLICY_UPGRADE_ECU_T *policy);
#ifdef ADM_IPS_POLICY_UPGRADE_ORDER_OLD
static D32S ipsPolicyUpgradeEcuGetUpgradeId(D32S upgradeOrder);
static D32S ipsPolicyUpgradeEcuGetNetId(D32S upgradeOrder);
static D32S ipsPolicyUpgradeEcuGetSequenceId(D32S upgradeOrder);
#else
static D32S ipsPolicyUpgradeEcuGetUpgradeId(const DCHAR* upgradeOrder);
static D32S ipsPolicyUpgradeEcuGetNetId(const DCHAR* upgradeOrder);
static D32S ipsPolicyUpgradeEcuGetSequenceId(const DCHAR* upgradeOrder);
#endif

/* ADM_IPS_POLICY_DEPENDENCY_ARRAY_T */
static void ipsPolicyDependencyArrayInit(ADM_IPS_POLICY_DEPENDENCY_ARRAY_T *policy);
static void ipsPolicyDependencyArrayDeInit(ADM_IPS_POLICY_DEPENDENCY_ARRAY_T *policy);
static void ipsPolicyDependencyArrayClone(const ADM_IPS_POLICY_DEPENDENCY_ARRAY_T *src, ADM_IPS_POLICY_DEPENDENCY_ARRAY_T *dst);
static void ipsPolicyDependencyArrayGet(const ADM_IPS_POLICY_DEPENDENCY_ARRAY_T *src, const D32S type, ADM_IPS_POLICY_DEPENDENCY_ARRAY_T *dst);
static void ipsPolicyDependencyArrayJsonParse(ADM_IPS_POLICY_DEPENDENCY_ARRAY_T *policy, const struct json_object* root);
static struct json_object* ipsPolicyDependencyArrayJsonCreate(const ADM_IPS_POLICY_DEPENDENCY_ARRAY_T *policy);

///* ADM_IPS_POLICY_DEPENDENCY_T */
//void adm_ips_policyDependencyInit(ADM_IPS_POLICY_DEPENDENCY_T *policy);
//void adm_ips_policyDependencyDeInit(ADM_IPS_POLICY_DEPENDENCY_T *policy);
//void adm_ips_policyDependencyClone(const ADM_IPS_POLICY_DEPENDENCY_T *src, ADM_IPS_POLICY_DEPENDENCY_T *dst);
static void ipsPolicyDependencyJsonParse(ADM_IPS_POLICY_DEPENDENCY_T *policy, const struct json_object* root);
static struct json_object* ipsPolicyDependencyJsonCreate(const ADM_IPS_POLICY_DEPENDENCY_T *policy);

/* ADM_IPS_POLICY_ECU_DID_ARRAY_T */
static void ipsPolicyEcuDidArrayInit(ADM_IPS_POLICY_ECU_DID_ARRAY_T *policy);
static void ipsPolicyEcuDidArrayDeInit(ADM_IPS_POLICY_ECU_DID_ARRAY_T *policy);
static void ipsPolicyEcuDidArrayClone(const ADM_IPS_POLICY_ECU_DID_ARRAY_T *src, ADM_IPS_POLICY_ECU_DID_ARRAY_T *dst);
static void ipsPolicyEcuDidArrayJsonParse(ADM_IPS_POLICY_ECU_DID_ARRAY_T *policy, const struct json_object* root);
static struct json_object* ipsPolicyEcuDidArrayJsonCreate(const ADM_IPS_POLICY_ECU_DID_ARRAY_T *policy);

/* ADM_IPS_POLICY_ECU_DID_T */
static void ipsPolicyEcuDidInit(ADM_IPS_POLICY_ECU_DID_T *policy);
static void ipsPolicyEcuDidDeInit(ADM_IPS_POLICY_ECU_DID_T *policy);
static void ipsPolicyEcuDidClone(const ADM_IPS_POLICY_ECU_DID_T *src, ADM_IPS_POLICY_ECU_DID_T *dst);
static void ipsPolicyEcuDidJsonParse(ADM_IPS_POLICY_ECU_DID_T *policy, const struct json_object* root);
static struct json_object* ipsPolicyEcuDidJsonCreate(const ADM_IPS_POLICY_ECU_DID_T *policy);

/* ADM_IPS_POLICY_UPGRADE_CONDITION_ARRAY_T */
static void ipsPolicyUpgradeConditionArrayInit(ADM_IPS_POLICY_UPGRADE_CONDITION_ARRAY_T *policy);
static void ipsPolicyUpgradeConditionArrayDeInit(ADM_IPS_POLICY_UPGRADE_CONDITION_ARRAY_T *policy);
static void ipsPolicyUpgradeConditionArrayJsonParse(ADM_IPS_POLICY_UPGRADE_CONDITION_ARRAY_T *policy, const struct json_object* root);
static struct json_object* ipsPolicyUpgradeConditionArrayJsonCreate(const ADM_IPS_POLICY_UPGRADE_CONDITION_ARRAY_T *policy);

#if 0
/* ADM_IPS_POLICY_UPGRADE_CONDITION_T */
static void ipsPolicyUpgradeConditionInit(ADM_IPS_POLICY_UPGRADE_CONDITION_T *policy);
static void ipsPolicyUpgradeConditionDeInit(ADM_IPS_POLICY_UPGRADE_CONDITION_T *policy);
static void ipsPolicyUpgradeConditionClone(const ADM_IPS_POLICY_UPGRADE_CONDITION_T *src, ADM_IPS_POLICY_UPGRADE_CONDITION_T *dst);
static void ipsPolicyUpgradeConditionJsonParse(ADM_IPS_POLICY_UPGRADE_CONDITION_T *policy, const struct json_object* root);
static struct json_object* ipsPolicyUpgradeConditionJsonCreate(const ADM_IPS_POLICY_UPGRADE_CONDITION_T *policy);

/* ADM_IPS_POLICY_CONDITION_ARRAY_T */
static void ipsPolicyConditionArrayInit(ADM_IPS_POLICY_CONDITION_ARRAY_T *policy);
static void ipsPolicyConditionArrayDeInit(ADM_IPS_POLICY_CONDITION_ARRAY_T *policy);
static void ipsPolicyConditionArrayClone(const ADM_IPS_POLICY_CONDITION_ARRAY_T *src, ADM_IPS_POLICY_CONDITION_ARRAY_T *dst);
static void ipsPolicyConditionArrayJsonParse(ADM_IPS_POLICY_CONDITION_ARRAY_T *policy, const struct json_object* root);
static struct json_object* ipsPolicyConditionArrayJsonCreate(const ADM_IPS_POLICY_CONDITION_ARRAY_T *policy);

/* ADM_IPS_POLICY_CONDITION_T */
static void ipsPolicyConditionInit(ADM_IPS_POLICY_CONDITION_T *policy);
static void ipsPolicyConditionDeInit(ADM_IPS_POLICY_CONDITION_T *policy);
static void ipsPolicyConditionClone(const ADM_IPS_POLICY_CONDITION_T *src, ADM_IPS_POLICY_CONDITION_T *dst);
static void ipsPolicyConditionJsonParse(ADM_IPS_POLICY_CONDITION_T *policy, const struct json_object* root);
static struct json_object* ipsPolicyConditionJsonCreate(const ADM_IPS_POLICY_CONDITION_T *policy);

/* ADM_IPS_POLICY_CONDITION_ON_FAILsTRATEGY_T */
static void ipsPolicyConditionOnFailStrategyInit(ADM_IPS_POLICY_CONDITION_ON_FAILsTRATEGY_T *policy);
static void ipsPolicyConditionOnFailStrategyDeInit(ADM_IPS_POLICY_CONDITION_ON_FAILsTRATEGY_T *policy);
static void ipsPolicyConditionOnFailStrategyClone(const ADM_IPS_POLICY_CONDITION_ON_FAILsTRATEGY_T *src, ADM_IPS_POLICY_CONDITION_ON_FAILsTRATEGY_T *dst);
static void ipsPolicyConditionOnFailStrategyJsonParse(ADM_IPS_POLICY_CONDITION_ON_FAILsTRATEGY_T *policy, const struct json_object* root);
static struct json_object* ipsPolicyConditionOnFailStrategyJsonCreate(const ADM_IPS_POLICY_CONDITION_ON_FAILsTRATEGY_T *policy);

/* ADM_IPS_POLICY_CONDITION_EXPRESSION_T */
static void ipsPolicyConditionExpressionInit(ADM_IPS_POLICY_CONDITION_EXPRESSION_T *policy);
static void ipsPolicyConditionExpressionDeInit(ADM_IPS_POLICY_CONDITION_EXPRESSION_T *policy);
static void ipsPolicyConditionExpressionClone(const ADM_IPS_POLICY_CONDITION_EXPRESSION_T *src, ADM_IPS_POLICY_CONDITION_EXPRESSION_T *dst);
static void ipsPolicyConditionExpressionJsonParse(ADM_IPS_POLICY_CONDITION_EXPRESSION_T *policy, const struct json_object* root);
static struct json_object* ipsPolicyConditionExpressionJsonCreate(const ADM_IPS_POLICY_CONDITION_EXPRESSION_T *policy);
#endif

/* ADM_IPS_POLICY_TASK_T */
static void ipsPolicyTaskInit(ADM_IPS_POLICY_TASK_T *task);
static void ipsPolicyTaskDeInit(ADM_IPS_POLICY_TASK_T *task);
static void ipsPolicyTaskClone(const ADM_IPS_POLICY_TASK_T *src, ADM_IPS_POLICY_TASK_T *dst);
static void ipsPolicyTaskClear(ADM_IPS_POLICY_TASK_T *task);
static void ipsPolicyTaskJsonParse(ADM_IPS_POLICY_TASK_T *task, const struct json_object* root);
static struct json_object* ipsPolicyTaskJsonCreate(const ADM_IPS_POLICY_TASK_T *task);
#if 0
static void ipsPolicyTaskSet(ADM_IPS_POLICY_TASK_T *task, const DCHAR *sessionId, D64S taskId, D64S strategyId);
#endif

/* ADM_IPS_INSTALL_ECUINFO_T */
static void ipsInstallEcuInfoInit(ADM_IPS_INSTALL_ECUINFO_T *e);
static void ipsInstallEcuInfoDeInit(ADM_IPS_INSTALL_ECUINFO_T *e);
static void ipsInstallEcuInfoClone(const ADM_IPS_INSTALL_ECUINFO_T *src, ADM_IPS_INSTALL_ECUINFO_T *dst);
static void ipsInstallEcuInfoCreate(ADM_IPS_INSTALL_ECUINFO_T *e, const ADM_IPS_POLICY_UPGRADE_ECU_T *upgradeEcu, DBOOL isRollback);
static struct json_object* ipsInstallEcuInfoJsonCreate(const ADM_IPS_INSTALL_ECUINFO_T *ecu);
static void ipsInstallEcuInfoJsonParse(ADM_IPS_INSTALL_ECUINFO_T *ecu, const struct json_object* root);



/**
 * @brief 智能单独激活件机构json创建
 * 
 * @return struct json_object* 
 */
static struct json_object *ipsActiveJsonCreate();

/**
 * @brief 清除智能单独激活件的数据库数据
 * 
 * @return DVOID 
 */
static DVOID ipsClearActiveEcuInfoFromDb();

/**
 * @brief 存储智能单独激活件数据到数据库
 * 
 * @return DVOID 
 */
static DVOID ipsSaveActiveEcuInfo2Db();

/**
 * @brief 从数据库加载智能单独激活件的数据
 * 
 * @return DVOID 
 */
static DVOID ipsLoadActiveEcuInfoFromDb();
#ifdef __cplusplus
}
#endif

#endif//__ADM_IPS_IMPL_H_
