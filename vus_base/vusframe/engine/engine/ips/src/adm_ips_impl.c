#include "adm_ips_impl.h"
#include "adm_ips_log.h"
#include "adm_ips_debug.h"
#include "adm_typedefs.h"
#include "adm_string.h"
#include "adm_memory.h"
#include "adm_debug.h"
#include "adm_com_errorCode.h"
#include "json-c/json.h"
#include "adm_debug.h"
#include "adm_com_kvstorage.h"
#include "adm_engine_common.h"
#include "adm_cfg_default.h"
#include "adm_cfg_api.h"
#include "adm_ecs_rsConfig.h"
#include <time.h>


/*
 * TODO init object then load from kv, load root only
 */


 /**
  * @struct  IPS_UPGRADE_GROUP_CREATE_CTX_T
  * @brief   创建升级组所需内容
  * @details
  */
typedef struct
{
    ADM_IPS_ECU_UPGRADE_ORDER_T**   upgradeOrders;      // ECU升级序列信息
    D32S                            upgradeOrderCount;  // ECU升级序列信息数量
    D32S                            upgradeOrderPos;    // 已处理ECU升级序列信息位置
    D32S                            curBBEndPos;        // 当前处理的BB结束位置
    D32S                            curCCEndPos;        // 当前处理的CC结束位置

    const ADM_IPS_POLICY_T* policy;             // 安装策略，用于查询ECU是否可升级
} IPS_UPGRADE_GROUP_CREATE_CTX_T;

struct UPGRADE_GROUP_CREATOR_T;
typedef D32S(*UPGRADE_GROUP_CREATOR_CREATE_GROUP)(ADM_IPS_ARRAY_T* array, D32S num, D32S onceFetchNum);
typedef D32S(*UPGRADE_GROUP_CREATOR_INIT_GROUP)(struct UPGRADE_GROUP_CREATOR_T* creator, IPS_UPGRADE_GROUP_T* group, IPS_UPGRADE_GROUP_CREATE_CTX_T* ctx);

/**
 * @struct UPGRADE_GROUP_CREATOR_T
 * @brief  升级组创建器
 * @details 用于创建升级组，并创建初始组中子元素
 */
typedef struct UPGRADE_GROUP_CREATOR_T
{
    UPGRADE_GROUP_CREATOR_CREATE_GROUP  createGroup;    // 升级组创建函数
    UPGRADE_GROUP_CREATOR_INIT_GROUP    initGroup;      // 升级组初始化函数
    struct UPGRADE_GROUP_CREATOR_T*     child;          // 升级组子元素创建器
} IPS_UPGRADE_GROUP_CREATOR_T;



static ADM_IPS_CONTEXT_T m_context;
static DBOOL m_isInitContext = DFALSE;
static ADM_IPS_UCL_ECU_ARRAY_ACTIVE_T g_uclEcuActiveContext;

static D32S ipsPolicyDependencyGet(const ADM_IPS_POLICY_DEPENDENCY_ARRAY_T* src,
                                   const DCHAR* ecuReqId, const DCHAR* ecuSwDid, const D32S type,
                                   ADM_IPS_POLICY_DEPENDENCY_T* dst);

static void ipsEcuUpgradeGroupManagerInit(ADM_IPS_ECU_UPGRADE_GROUP_MANAGER_T* manager);
static void ipsEcuUpgradeGroupManagerDeinit(ADM_IPS_ECU_UPGRADE_GROUP_MANAGER_T* manager);
static void ipsEcuUpgradeGroupManagerClear(ADM_IPS_ECU_UPGRADE_GROUP_MANAGER_T* manager);
static D32S ipsEcuUpgradeGroupArrayGetInstallEcusEx(ADM_IPS_ECU_UPGRADE_GROUP_MANAGER_T* manager, const ADM_IPS_POLICY_T* policy, ADM_IPS_PROGRESS_T* progress, ADM_IPS_INSTALL_ECUINFO_T* ecus, D32S ecuMaxNum);
static D32S ipsEcuUpgradeGroupsCreateEx(ADM_IPS_ECU_UPGRADE_GROUP_MANAGER_T* manager, const ADM_IPS_POLICY_T* policy);
static void upgradeGroupsFuncsInit(void);


void adm_ips_init(void){
    if(DFALSE == m_isInitContext){
        ipsContextInit(&m_context);
        m_isInitContext = DTRUE;
        ADM_DEBUG_RETURN_VOID();
    }
}
void adm_ips_deInit(void){
    if(DFALSE != m_isInitContext){
        ipsContextDeInit(&m_context);
        m_isInitContext = DFALSE;
        ADM_DEBUG_RETURN_VOID();
    }
}
void adm_ips_stepInstallPrepare(const ADM_CVS_VERSION_POLICY_T* versionPolicy, D32S parallelNumLimitM, D32S parallelNumLimitN){
    adm_ips_init();
    if(versionPolicy != NULL){
        ipsContextStepInstallPrepare(&m_context, versionPolicy, parallelNumLimitM, parallelNumLimitN);
    }
    ADM_DEBUG_RETURN_VOID();
}
void adm_ips_stepInstallStart(void){
    adm_ips_init();
    ipsContextStepInstallStart(&m_context);
    ADM_DEBUG_RETURN_VOID();
}
void adm_ips_stepPostInstall(D32S result){
    adm_ips_init();
    ipsContextStepPostInstall(&m_context, result);
    ADM_DEBUG_RETURN_VOID();
}
D32S adm_ips_maxEcusCountGet(void){
    adm_ips_init();
    D32S ecusLen = ipsContextGetMaxEcusCount(&m_context);
    log_d("ecusLen:%d", ecusLen);
    return ecusLen;
}
void adm_ips_installEcuInfoInit(ADM_IPS_INSTALL_ECUINFO_T *ecu){
    if(ecu != NULL){
        ipsInstallEcuInfoInit(ecu);
    }
}
void adm_ips_installEcuInfoDeInit(ADM_IPS_INSTALL_ECUINFO_T *ecu){
    if(ecu != NULL){
        ipsInstallEcuInfoDeInit(ecu);
    }
}
void adm_ips_installEcuInfoClone(const ADM_IPS_INSTALL_ECUINFO_T *src, ADM_IPS_INSTALL_ECUINFO_T *dst){
    if(src != NULL && dst != NULL){
	    ipsInstallEcuInfoClone(src, dst);
    }
}
struct json_object* adm_ips_installEcuInfoJsonCreate(const ADM_IPS_INSTALL_ECUINFO_T *ecu){
    struct json_object* root = NULL;
    if(NULL != ecu){
	    root = ipsInstallEcuInfoJsonCreate(ecu);
    }
    return root;
}
void adm_ips_installEcuInfoJsonParse(ADM_IPS_INSTALL_ECUINFO_T *ecu, const struct json_object* root){
    if(NULL != ecu && NULL != root){
	    ipsInstallEcuInfoJsonParse(ecu, root);
    }
}
D32S adm_ips_installEcuInfoArrayGet(ADM_IPS_INSTALL_ECUINFO_T *ecus, D32S ecuMaxNum){
    adm_ips_init();
    D32S count = 0;
    if(ecus != NULL){
	    count = ipsContextGetInstallEcus(&m_context, ecus, ecuMaxNum);
    }
    log_d("count:%d\n", count);
    return count;
}
void adm_ips_currentProgressSet(const ADM_IPS_ECU_CURRENT_PROGRESS_T *progress){
    adm_ips_init();
    if(progress != NULL){
        ipsContextSetInstallEcuProgress(&m_context, progress);
    }
    /*ADM_DEBUG_RETURN_VOID();*/
}

void adm_ips_progressAbInfoSet(const ADM_IPS_ECU_PROGRESS_T *progress){
    adm_ips_init();
    if(progress != NULL){
        ipsContextSetEcuProgressAbInfo(&m_context, progress);
    }

    return;
}

void adm_ips_policyTaskInit(ADM_IPS_POLICY_TASK_T *task){
    if(task != NULL){
        ipsPolicyTaskInit(task);
    }
}

void adm_ips_policyTaskDeInit(ADM_IPS_POLICY_TASK_T *task){
    if(task != NULL){
        ipsPolicyTaskDeInit(task);
    }
}

D32S adm_ips_policyTaskGet(ADM_IPS_POLICY_TASK_T *task){
    adm_ips_init();
    if(task != NULL){
        return ipsContextGetPolicyTask(&m_context, task);
    }

    return ADM_ECODE_FAILED;
}

D32S adm_ips_policyTaskVerifyVersionGet(void)
{
    return m_context.task.verifyVersion;
}

void adm_ips_ecuProgressInit(ADM_IPS_ECU_PROGRESS_T *progress){
    if(progress != NULL){
        ipsEcuProgressInit(progress);
    }
}

void adm_ips_ecuProgressDeInit(ADM_IPS_ECU_PROGRESS_T *progress){
    if(progress != NULL){
        ipsEcuProgressDeInit(progress);
    }
}

D32S adm_ips_ecuProgressGet(ADM_IPS_ECU_PROGRESS_T *progress, const DCHAR *ecuReqId, const DCHAR *ecuSwDid){
    adm_ips_init();
    if(ecuReqId != NULL && ecuSwDid != NULL){
        return ipsContextGetEcuProgress(&m_context, ecuReqId, ecuSwDid, progress);
    }

    return ADM_ECODE_FAILED;
}

void adm_ips_progressInit(ADM_IPS_PROGRESS_T *progress){
    if(progress != NULL){
        ipsProgressInit(progress);
    }
}
void adm_ips_progressDeInit(ADM_IPS_PROGRESS_T *progress){
    if(progress != NULL){
        ipsProgressDeInit(progress);
    }
}
void adm_ips_progressGet(ADM_IPS_PROGRESS_T *progress){
    adm_ips_init();
    if(progress != NULL){
        ipsContextGetProgress(&m_context, progress);
    }
}
D32S adm_ips_cmdIdGet(void){
    adm_ips_init();
    return ipsContextGetCmdId(&m_context);
}
void adm_ips_cmdIdSet(D32S cmdId){
    adm_ips_init();
    ipsContextSetCmdId(&m_context, cmdId);
}

D32S adm_ips_policyConditionGet(D32S iType, ADM_COND_POLICY_T *upgradeConditions){
    adm_ips_init();
    if(upgradeConditions != NULL){
	    return ipsContextGetPolicyCondition(&m_context, iType, upgradeConditions);
    }

    return ADM_ECODE_FAILED;
}

/**
 * @brief       从版本策略中获取安装前置条件策略。
 * @param  in   D32S iInstallType                 
 * @param  in   ADM_COND_POLICY_T *pstConditions  
 * @retval      D32S
 */
D32S adm_ips_getConditionFromPolicy(D32S iType, ADM_COND_POLICY_T *upgradeConditions)
{

    D32S iRet = ADM_ECODE_SUCCESSED;
    D32S i = 0;
    json_object* pInstallJson = NULL;
    ADM_CVS_VERSION_POLICY_T *pstVersionPolicy = NULL;
    ADM_IPS_POLICY_T stPolicy = {0};
    ADM_IPS_POLICY_UPGRADE_CONDITION_ARRAY_T *pstConds = NULL;

    if (ADM_IS_NULL(upgradeConditions))
    {
            log_e("invalid param");
            return ADM_ECODE_INVALID_PARAM;
    }

    pstVersionPolicy = adm_cvs_newPolicyBody();
    if (ADM_IS_NULL(pstVersionPolicy))
    {
        log_e("adm_cvs_newPolicyBody err");
        return ADM_ECODE_APPLY_MEMORY_FAILED;
    }

    do 
    {
        iRet = adm_cvs_getVersionPolicy(pstVersionPolicy);
        if (ADM_IS_ERR(iRet))
        {
            log_e("adm_cvs_getVersionPolicy err, iRet = %d", iRet);
            break;
        }

        if (ADM_IS_NULL(pstVersionPolicy->installMember.installPolicyObjectStr))
        {
            log_e("installPolicyObjectStr is nil");
            iRet = ADM_ECODE_INVALID_POINTER;
            break;
        }

        pInstallJson = json_tokener_parse(pstVersionPolicy->installMember.installPolicyObjectStr);
        if (ADM_IS_NULL(pInstallJson)) 
        {
            log_e("str to son_object. failed");
            iRet = ADM_ECODE_STRING_TO_JSON_FAILED;
            break;
        }

        ipsPolicyJsonParse(&stPolicy, pInstallJson);
        pstConds = &stPolicy.upgradeConditions;
        if(pstConds->upgradeConditions)
        {
            for(i = 0; i < pstConds->upgradeConditionsLen; i++)
            {
                if(pstConds->upgradeConditions[i].iType == iType)
                {
                    adm_condPolicyClone(&pstConds->upgradeConditions[i], upgradeConditions);
                    iRet = ADM_ECODE_SUCCESSED;
                    break;
                }
            }
        }
    } while(FALSE);

    if (pInstallJson)
    {
        json_object_put(pInstallJson);
    }
    adm_cvs_deletePolicyBody(pstVersionPolicy);
    ipsPolicyDeInit(&stPolicy);
    
    return iRet;
}

void adm_ips_policyDependencyArrayInit(ADM_IPS_POLICY_DEPENDENCY_ARRAY_T *policy){
    if(NULL != policy){
	    ipsPolicyDependencyArrayInit(policy);
    }
}

void adm_ips_policyDependencyArrayDeInit(ADM_IPS_POLICY_DEPENDENCY_ARRAY_T *policy){
    if(NULL != policy){
	    ipsPolicyDependencyArrayDeInit(policy);
    }
}

D32S adm_ips_policyDependencyArrayGet(ADM_IPS_POLICY_DEPENDENCY_ARRAY_T *policy){
    adm_ips_init();
    if(NULL != policy){
	    ipsContextGetPolicyDependencies(&m_context, policy);
        return ADM_ECODE_SUCCESSED;
    }

    return ADM_ECODE_FAILED;
}

D32S adm_ips_policyDependenciesGetWithType(ADM_IPS_POLICY_DEPENDENCY_ARRAY_T *policy, const D32S type){
    adm_ips_init();
    if(NULL != policy){
	    ipsContextGetPolicyDependenciesWithType(&m_context, type, policy);
        return ADM_ECODE_SUCCESSED;
    }

    return ADM_ECODE_FAILED;
}

D32S adm_ips_policyDependencyGetWithType(const DCHAR* ecuReqId, const DCHAR* ecuSwDid, const D32S type, ADM_IPS_POLICY_DEPENDENCY_T* dependency) {
    adm_ips_init();
    if (NULL != dependency) {
        return ipsPolicyDependencyGet(&m_context.policy.dependencies, ecuReqId, ecuSwDid, type, dependency);
    }

    return ADM_ECODE_FAILED;
}

void adm_ips_policyEcuDidInit(ADM_IPS_POLICY_ECU_DID_T *policy){
    if(NULL != policy){
	    ipsPolicyEcuDidInit(policy);
    }
}

void adm_ips_policyEcuDidDeInit(ADM_IPS_POLICY_ECU_DID_T *policy){
    if(NULL != policy){
	    ipsPolicyEcuDidDeInit(policy);
    }
}

D32S adm_ips_policyDependenciesEcuDidGet(const D32S type, const DCHAR *ecuReqId, const DCHAR *ecuSwDid, 
	                                     const DCHAR *did, ADM_IPS_POLICY_ECU_DID_T *ecuDid){
    
    adm_ips_init();
    if(NULL != ecuReqId && NULL != ecuSwDid && NULL != did && NULL != ecuDid){
	    return ipsContextGetPolicyDependenciesEcuDid(&m_context, type, ecuReqId, ecuSwDid, did, ecuDid);
    }

    return ADM_ECODE_FAILED;
}

DVOID adm_ips_silenceUpgradeTimeGet(D64U* iTime)
{
    D32S iRet = ADM_ECODE_FAILED;
    ADM_CVS_VERSION_POLICY_T* pstVersionPolicy = NULL;
    
    do
    {
        pstVersionPolicy = adm_cvs_newPolicyBody();
        if (ADM_IS_NULL(pstVersionPolicy))
        {
            log_e("adm_cvs_newPolicyBody err");
            iRet = ADM_ECODE_APPLY_MEMORY_FAILED;
            break;
        }

        iRet = adm_cvs_getVersionPolicy(pstVersionPolicy);
        if (ADM_IS_ERR(iRet))
        {
            log_e("adm_cvs_getVersionPolicy err, iRet = %d", iRet);
            break;
        }

        if(NULL == pstVersionPolicy->installMember.installPolicyObjectStr)
        {
            log_e("versionPolicy installMember installPolicyObjectStr:nullptr");
            break;
        }
        else
        {
            struct json_object* pstRoot = json_tokener_parse(pstVersionPolicy->installMember.installPolicyObjectStr);
            if(NULL == pstRoot)
            {
                log_e("content:[%s] to json failed", pstVersionPolicy->installMember.installPolicyObjectStr);
                break;
            }
            else
            {
                struct json_object* pstUpgradeAutoStartVal = json_object_object_get(pstRoot, "upgradeAutoStart");
                if(NULL != pstUpgradeAutoStartVal)
                {
                    *iTime = (D64U)json_object_get_int64(pstUpgradeAutoStartVal);
                }
                else 
                {
                    *iTime = (D64U)0;
                }
                
                (void)json_object_put(pstRoot);
            }
        }
    } while (false);

    if (ADM_IS_ERR(iRet))
    {
        *iTime = (D64U)0;
    }

    adm_free(pstVersionPolicy);
    return;
}

/* ADM_IPS_CONTEXT_T */
static void ipsContextInit(ADM_IPS_CONTEXT_T *c){
    (void)pthread_mutex_init(&c->mutex, NULL);
    ipsPolicyTaskInit(&c->task);
    ipsPolicyInit(&c->policy);
    ipsProgressInit(&c->progress);
    // ipsEcuUpgradeGroupArrayInit(&c->groups);
	upgradeGroupsFuncsInit();
    ipsEcuUpgradeGroupManagerInit(&c->upgradeGroupManager);
    c->cmdId = 0;
	ipsContextLoad(c);

    (void)pthread_mutex_init(&g_uclEcuActiveContext.mutex, NULL);
    ipsLoadActiveEcuInfoFromDb();

    IPS_CONTEXT_EXEC(c, IPS_CONTEXT_EXEC_FLAG_SHOW);

    return;
}
static void ipsContextDeInit(ADM_IPS_CONTEXT_T *c){
    (void)pthread_mutex_destroy(&c->mutex);
    ipsPolicyTaskDeInit(&c->task);
    ipsPolicyDeInit(&c->policy);
    ipsProgressDeInit(&c->progress);
    // ipsEcuUpgradeGroupArrayDeInit(&c->groups);
	ipsEcuUpgradeGroupManagerDeinit(&c->upgradeGroupManager);
    (void)pthread_mutex_destroy(&g_uclEcuActiveContext.mutex);

    return;
}
static void ipsContextJsonParse(ADM_IPS_CONTEXT_T *c, const struct json_object* root){
    struct json_object* taskVal = json_object_object_get(root, "task");
    if(NULL != taskVal){
        ipsPolicyTaskJsonParse(&c->task, taskVal);
    }
    struct json_object* policyVal = json_object_object_get(root, "policy");
    if(NULL != policyVal){
	    ipsPolicyJsonParse(&c->policy, policyVal);
    }
    struct json_object* progressVal = json_object_object_get(root, "progress");
    if(NULL != progressVal){
	    ipsProgressJsonParse(&c->progress, progressVal);
    }
 //   struct json_object* groupsVal = json_object_object_get(root, "groups");
 //   if(groupsVal){
	//ipsEcuUpgradeGroupArrayJsonParse(&c->groups, groupsVal);
 //   }
    (void)ipsEcuUpgradeGroupsCreateEx(&c->upgradeGroupManager, &c->policy);
    c->cmdId = json_object_get_int(json_object_object_get(root, "cmdId"));
}
static struct json_object* ipsContextJsonCreate(const ADM_IPS_CONTEXT_T *c){
    struct json_object* root = json_object_new_object();
    if(NULL != root){
        struct json_object* taskVal = ipsPolicyTaskJsonCreate(&c->task);
        if (NULL != taskVal) {
            (void)json_object_object_add(root, "task", taskVal);
        }
        struct json_object* policyVal = ipsPolicyJsonCreate(&c->policy);
        if (NULL != policyVal) {
            (void)json_object_object_add(root, "policy", policyVal);
        }
        struct json_object* progressVal = ipsProgressJsonCreate(&c->progress);
        if (NULL != progressVal) {
            (void)json_object_object_add(root, "progress", progressVal);
        }
        /*struct json_object* groupsVal = ipsEcuUpgradeGroupArrayJsonCreate(&c->groups);
        if (groupsVal) {
            (void)json_object_object_add(root, "groups", groupsVal);
        }*/
        (void)json_object_object_add(root, "cmdId", json_object_new_int(c->cmdId));
    }
    return root;
}

static void ipsContextStepInstallPrepare(ADM_IPS_CONTEXT_T *c, const ADM_CVS_VERSION_POLICY_T* versionPolicy, D32S parallelNumLimitM, D32S parallelNumLimitN){
    (void)pthread_mutex_lock(&c->mutex);
    ipsPolicyTaskClear(&c->task);
    ipsPolicyClear(&c->policy);
    ipsProgressClear(&c->progress);
    ipsEcuUpgradeGroupManagerClear(&c->upgradeGroupManager);
    // ipsEcuUpgradeGroupArrayClear(&c->groups);

    if(NULL == versionPolicy->installMember.installPolicyObjectStr){
	    log_w("versionPolicy installMember installPolicyObjectStr:nullptr");
    }else{
        struct json_object *root = json_tokener_parse(versionPolicy->installMember.installPolicyObjectStr);
        if(NULL == root){
            log_w("path:%s to json failed", versionPolicy->installMember.installPolicyObjectStr);
        }else{
            ipsPolicyJsonParse(&c->policy, root);
            c->policy.parallelNumLimitM = parallelNumLimitM;
            c->policy.parallelNumLimitN = parallelNumLimitN;

            ADM_IPS_POLICY_TASK_T task = {
                .sessionId = versionPolicy->sessionId,
                .taskId = versionPolicy->taskId,
                .strategyId = versionPolicy->strategyId,
                .versionNumber = versionPolicy->versionNumber,
                .upgradeMode = versionPolicy->upgradeMode,
                .upgradeAutoStart = c->policy.upgradeAutoStart,
                .totalTimeConsuming = c->policy.totalTimeConsuming,
                .verifyVersion = c->policy.verifyVersion
            };
            ipsPolicyTaskClone(&task, &c->task);

            ipsProgressCreate(&c->progress, &c->task, &c->policy);
            // ipsEcuUpgradeGroupArrayCreate(&c->groups, &c->policy);
            (void)ipsEcuUpgradeGroupsCreateEx(&c->upgradeGroupManager, &c->policy);
            (void)json_object_put(root);
        }
    }
    ipsProgressInstallPrepare(&c->progress);
    IPS_CONTEXT_EXEC(c, IPS_CONTEXT_EXEC_FLAG_STORE | IPS_CONTEXT_EXEC_FLAG_SHOW);
    (void)pthread_mutex_unlock(&c->mutex);
}

static void ipsContextStepInstallStart(ADM_IPS_CONTEXT_T *c){
    (void)pthread_mutex_lock(&c->mutex);
    ipsProgressInstallStart(&c->progress);
    IPS_CONTEXT_EXEC(c, IPS_CONTEXT_EXEC_FLAG_STORE | IPS_CONTEXT_EXEC_FLAG_SHOW);
    (void)pthread_mutex_unlock(&c->mutex);
}

static void ipsContextStepPostInstall(ADM_IPS_CONTEXT_T *c, D32S result){
    (void)pthread_mutex_lock(&c->mutex);
    ipsProgressPostInstall(&c->progress, result);
    IPS_CONTEXT_EXEC(c, IPS_CONTEXT_EXEC_FLAG_STORE | IPS_CONTEXT_EXEC_FLAG_SHOW);
    (void)pthread_mutex_unlock(&c->mutex);
}

static D32S ipsContextGetPolicyTask(ADM_IPS_CONTEXT_T *c, ADM_IPS_POLICY_TASK_T *task){
    D32S ret = ADM_ECODE_FAILED;

    (void)pthread_mutex_lock(&c->mutex);
    if (NULL != c->task.sessionId) {
        ipsPolicyTaskClone(&c->task, task);
        ret = ADM_ECODE_SUCCESSED;
    }
    (void)pthread_mutex_unlock(&c->mutex);

    return ret;
}

static D32S ipsContextGetInstallEcus(ADM_IPS_CONTEXT_T *c, ADM_IPS_INSTALL_ECUINFO_T *ecus, D32S ecuMaxNum){
    (void)pthread_mutex_lock(&c->mutex);
    // D32S ecusLen = ipsEcuUpgradeGroupArrayGetInstallEcus(&c->groups, &c->policy, &c->progress, ecus);
    D32S ecusLen = ipsEcuUpgradeGroupArrayGetInstallEcusEx(&c->upgradeGroupManager, &c->policy, &c->progress, ecus, ecuMaxNum);
    IPS_CONTEXT_EXEC(c, IPS_CONTEXT_EXEC_FLAG_STORE | IPS_CONTEXT_EXEC_FLAG_SHOW);
    (void)pthread_mutex_unlock(&c->mutex);
    return ecusLen;
}

static void ipsContextSetInstallEcuProgress(ADM_IPS_CONTEXT_T *c, const ADM_IPS_ECU_CURRENT_PROGRESS_T *progress){
    (void)pthread_mutex_lock(&c->mutex);
    DBOOL dRet = ipsProgressEcuSet(&c->progress, progress, &c->policy);
    if(progress->isEnd == DFALSE){
        if (DTRUE == dRet) {
            IPS_CONTEXT_EXEC(c, IPS_CONTEXT_EXEC_FLAG_STORE);
        }
    }else{
	    IPS_CONTEXT_EXEC(c, IPS_CONTEXT_EXEC_FLAG_STORE | IPS_CONTEXT_EXEC_FLAG_SHOW);
    }
    (void)pthread_mutex_unlock(&c->mutex);
}

static void ipsContextSetEcuProgressAbInfo(ADM_IPS_CONTEXT_T *c, const ADM_IPS_ECU_PROGRESS_T *progress)
{

    if (c->progress.state == ADM_IPS_ECU_PHASE_INSTALL)
    {
        ADM_IPS_ECU_PROGRESS_T* ecuProgress = ipsProgressFindEcu(&c->progress, progress->ecuReqId, progress->ecuSwDid);

        if (NULL != ecuProgress)
        {
            (void)pthread_mutex_lock(&c->mutex);

            ecuProgress->bIsAb = progress->bIsAb;
            ecuProgress->bBlockSwitchAb = progress->bBlockSwitchAb;
            ecuProgress->eDestAbInfo = progress->eDestAbInfo;
            ecuProgress->bIsFLashHasSwitchAb = progress->bIsFLashHasSwitchAb;
            ecuProgress->bIsRollbackHasSwitchAB = progress->bIsRollbackHasSwitchAB;
            ecuProgress->progress[ADM_IPS_ECU_PHASE_INDEX_UPGRADE].result = 
                progress->progress[ADM_IPS_ECU_PHASE_INDEX_UPGRADE].result;
            ecuProgress->progress[ADM_IPS_ECU_PHASE_INDEX_ROLLBACK].result = 
                progress->progress[ADM_IPS_ECU_PHASE_INDEX_ROLLBACK].result;
            ecuProgress->progress[ADM_IPS_ECU_PHASE_INDEX_ROLLBACK].upgradeNumbers = 
                progress->progress[ADM_IPS_ECU_PHASE_INDEX_ROLLBACK].upgradeNumbers;
            ecuProgress->progress[ADM_IPS_ECU_PHASE_INDEX_ROLLBACK].endTime = 
                progress->progress[ADM_IPS_ECU_PHASE_INDEX_ROLLBACK].endTime;

            IPS_CONTEXT_EXEC(c, IPS_CONTEXT_EXEC_FLAG_STORE);

            (void)pthread_mutex_unlock(&c->mutex);
        }
        else 
        {
            log_e("Can not find progress data.");
        }
    }
    else 
    {
        log_i("Current state[%d] need not update ab info.", c->progress.state);
    }

    return;
}

static D32S ipsContextGetMaxEcusCount(ADM_IPS_CONTEXT_T *c){
    (void)pthread_mutex_lock(&c->mutex);
    D32S count = ipsPolicyGetMaxEcusCount(&c->policy);
    (void)pthread_mutex_unlock(&c->mutex);
    return count;
}

static void ipsContextGetProgress(ADM_IPS_CONTEXT_T *c, ADM_IPS_PROGRESS_T *progress){
    (void)pthread_mutex_lock(&c->mutex);
    ipsProgressClone(&c->progress, progress);
    (void)pthread_mutex_unlock(&c->mutex);
}

static D32S ipsContextGetEcuProgress(ADM_IPS_CONTEXT_T *c, const DCHAR *ecuReqId, const DCHAR *ecuSwDid, ADM_IPS_ECU_PROGRESS_T *progress){
    D32S ret = ADM_ECODE_FAILED;

    (void)pthread_mutex_lock(&c->mutex);
    if(ecuReqId != NULL && ecuSwDid != NULL){
        ADM_IPS_ECU_PROGRESS_T *p = ipsProgressFindEcu(&c->progress, ecuReqId, ecuSwDid);
        if (p != NULL) {
            ipsEcuProgressClone(p, progress);
            ret = ADM_ECODE_SUCCESSED;
        }
    }
    (void)pthread_mutex_unlock(&c->mutex);

    return ret;
}
static D32S ipsContextGetCmdId(ADM_IPS_CONTEXT_T *c){
    (void)pthread_mutex_lock(&c->mutex);
    D32S cmdId = c->cmdId;
    (void)pthread_mutex_unlock(&c->mutex);
    return cmdId;
}
static void ipsContextSetCmdId(ADM_IPS_CONTEXT_T *c, D32S cmdId){
    (void)pthread_mutex_lock(&c->mutex);
    if(c->cmdId != cmdId){
        c->cmdId = cmdId;
        log_d("cmdId:%d", cmdId);
        IPS_CONTEXT_EXEC(c, IPS_CONTEXT_EXEC_FLAG_STORE);
    }
    (void)pthread_mutex_unlock(&c->mutex);
}

/**
 * @brief 更新ECU升级状态
 * @note
 * @param in ADM_IPS_CONTEXT_T *pstContext   升级上下文
 * @param in DCHAR *pcEcuReqId               ECU请求ID
 * @param in DCHAR *pcEcuSwDid               ECU软件ID
 * @param in ADM_IPS_ECU_PHASE_E eState      升级状态
 * @retval D32S 0 成功，非0 失败
 */
static D32S ipsContextUpdateEcuState(ADM_IPS_CONTEXT_T *pstContext, 
                                     DCHAR *pcEcuReqId,
                                     DCHAR *pcEcuSwDid, 
                                     ADM_IPS_ECU_PHASE_E eState)
{
    ADM_IPS_ECU_PROGRESS_T *pstEcuInfo = NULL;
    D32S iRet = ADM_ECODE_FAILED;

    ADM_COM_FUNC_ENTRY();

    /** 入参校验 */
    if (ADM_IS_NULL(pstContext) || ADM_IS_NULL(pcEcuReqId) || ADM_IS_NULL(pcEcuSwDid))
    {
        log_e("invalid param");
        return ADM_ECODE_INVALID_PARAM;
    }

    (void)pthread_mutex_lock(&pstContext->mutex);

    /** 查找ECU信息 */
    pstEcuInfo = ipsProgressFindEcu(&pstContext->progress, pcEcuReqId, pcEcuSwDid);
    if (pstEcuInfo != NULL)
    {
        log_d("ecu flash state change to %d", eState);
        pstEcuInfo->state = eState;
        iRet = ADM_ECODE_SUCCESSED;
    }
    else
    {
        log_e("ecu info not found");
        iRet = ADM_ECODE_INVALID_POINTER;
    }

    /** 存储上下文 */
    IPS_CONTEXT_EXEC(pstContext, IPS_CONTEXT_EXEC_FLAG_STORE);

    (void)pthread_mutex_unlock(&pstContext->mutex);

    ADM_COM_FUNC_EXIT();
    
    return iRet;
}

static D32S ipsContextGetPolicyCondition(ADM_IPS_CONTEXT_T *c, D32S iType, ADM_COND_POLICY_T *upgradeConditions){
    D32S ret = ADM_ECODE_FAILED;

    (void)pthread_mutex_lock(&c->mutex);
    const ADM_IPS_POLICY_UPGRADE_CONDITION_ARRAY_T *conds = &c->policy.upgradeConditions;
    if(conds->upgradeConditions != NULL){
	    for(D32S i=0; i<conds->upgradeConditionsLen; ++i){
	        if(conds->upgradeConditions[i].iType == iType){
                adm_condPolicyClone(&conds->upgradeConditions[i], upgradeConditions);
                ret = ADM_ECODE_SUCCESSED;
                break;
	        }
	    }
    }
    (void)pthread_mutex_unlock(&c->mutex);

    return ret;
}

static void ipsContextGetPolicyDependencies(ADM_IPS_CONTEXT_T *c, ADM_IPS_POLICY_DEPENDENCY_ARRAY_T *dependencies){
    (void)pthread_mutex_lock(&c->mutex);
    ipsPolicyDependencyArrayClone(&c->policy.dependencies, dependencies);
    (void)pthread_mutex_unlock(&c->mutex);
}

static void ipsContextGetPolicyDependenciesWithType(ADM_IPS_CONTEXT_T *c, const D32S type, ADM_IPS_POLICY_DEPENDENCY_ARRAY_T *dependencies){
    (void)pthread_mutex_lock(&c->mutex);
    ipsPolicyDependencyArrayGet(&c->policy.dependencies, type, dependencies);
    (void)pthread_mutex_unlock(&c->mutex);
}

static D32S ipsContextGetPolicyDependenciesEcuDid(ADM_IPS_CONTEXT_T *c, const D32S type,
                                                  const DCHAR *ecuReqId, const DCHAR *ecuSwDid,
                                                  const DCHAR* did, ADM_IPS_POLICY_ECU_DID_T* ecuDid) {
    D32S ret = ADM_ECODE_FAILED;

    (void)pthread_mutex_lock(&c->mutex);
    const ADM_IPS_POLICY_DEPENDENCY_ARRAY_T *dependencies = &c->policy.dependencies;
    for(D32S i=0; i<dependencies->dependenciesLen; ++i){
        const ADM_IPS_POLICY_DEPENDENCY_T *dependency = &dependencies->dependencies[i];
        if(type == dependency->type){
            if(NULL != dependency->ecuReqId && NULL != dependency->ecuSwDid){
    	    if(0 == adm_strcmp(dependency->ecuReqId, ecuReqId) && 
               0 == adm_strcmp(dependency->ecuSwDid, ecuSwDid)){
    	        const ADM_IPS_POLICY_ECU_DID_ARRAY_T *ecuDids = &dependency->ecuDids;
    	        for(D32S j=0; j<ecuDids->ecuDidsLen; ++j){
                    const ADM_IPS_POLICY_ECU_DID_T* srcEcuDid = &ecuDids->ecuDids[j];
                    if (NULL != srcEcuDid->did) {
                        if (0 == adm_strcmp(did, srcEcuDid->did)) {
                            ipsPolicyEcuDidClone(srcEcuDid, ecuDid);
                            ret = ADM_ECODE_SUCCESSED;
                            break;
                        }
                    }
    	        }
    	    }
            }
        }

        if(ADM_ECODE_SUCCESSED == ret){
            break;
        }
    }
    (void)pthread_mutex_unlock(&c->mutex);

    return ret;
}
static void ipsContextLoad(ADM_IPS_CONTEXT_T *c){
    ADM_KV_STORAGE_VALUE_TYPE_E eType = ADM_KV_STORAGE_VALUE_TYPE_INVALID;
    DCHAR *contextJsonStr = NULL;
    size_t contextJsonStrLen = 0;
    if(adm_com_kvstorage_get_string(adm_core_service_getBusinessDatabaseDb(), ADM_IPS_CONTEXT_KV_KEY,
                &contextJsonStr, &contextJsonStrLen, &eType)){
        if (NULL != contextJsonStr) {
            struct json_object* contextVal = json_tokener_parse(contextJsonStr);
            if (NULL != contextVal) {
                ipsContextJsonParse(c, contextVal);
                (void)json_object_put(contextVal);
            }
            adm_free(contextJsonStr);
        }
    }
}
#if 0
static void ipsContextStore(const ADM_IPS_CONTEXT_T *c){
    struct json_object* root = ipsContextJsonCreate(c);
    if(NULL != root){
	    const DCHAR *contextJsonStr = json_object_to_json_string(root);
	    if(NULL != contextJsonStr){
	        ipsContextStoreSimple(c, contextJsonStr);
	    }
	    (void)json_object_put(root);
    }
}
#endif
static void ipsContextStoreSimple(const ADM_IPS_CONTEXT_T *c, const DCHAR *cStr){
    (void)adm_com_kvstorage_set_string(adm_core_service_getBusinessDatabaseDb(), ADM_IPS_CONTEXT_KV_KEY,
        cStr, ADM_KV_STORAGE_VALUE_TYPE_STRING);
}
static void IPS_CONTEXT_EXEC(ADM_IPS_CONTEXT_T *context, D32S flags){
    struct json_object* root = ipsContextJsonCreate(context);
    if(NULL != root){
        const DCHAR* str = json_object_to_json_string(root);
        if (NULL != str && IPS_CONTEXT_EXEC_FLAG_STORE == (flags & IPS_CONTEXT_EXEC_FLAG_STORE)) {
            ipsContextStoreSimple(context, str);
        }
        if (IPS_CONTEXT_EXEC_FLAG_SHOW == (flags & IPS_CONTEXT_EXEC_FLAG_SHOW)) {
            log_d("n%s", ADM_STR_SAFE_OUTPUT(str));
        }
        (void)json_object_put(root);
    }
}


/* ADM_IPS_ECU_UPGRADE_ORDER_T */
static void ipsEcuUpgradeOrderInit(ADM_IPS_ECU_UPGRADE_ORDER_T *u){
    ipsEcuIndexInit(&u->index);
    u->upgradeId = ADM_IPS_ECU_UPGRADE_ORDER_MEMBER_DEFAULT_ID;
    u->netId = ADM_IPS_ECU_UPGRADE_ORDER_MEMBER_DEFAULT_ID;
    u->sequenceId = ADM_IPS_ECU_UPGRADE_ORDER_MEMBER_DEFAULT_ID;
}
static void ipsEcuUpgradeOrderDeInit(ADM_IPS_ECU_UPGRADE_ORDER_T *u){
    ipsEcuIndexDeInit(&u->index);
}
static void ipsEcuUpgradeOrderCreate(ADM_IPS_ECU_UPGRADE_ORDER_T *u, D32S groupIndex, D32S memberIndex, const ADM_IPS_POLICY_UPGRADE_ECU_T *ecu){
    ipsEcuIndexCreate(&u->index, groupIndex, memberIndex, ecu->upgradeOrder);
    u->upgradeId = ipsPolicyUpgradeEcuGetUpgradeId(ecu->upgradeOrder);
    u->netId = ipsPolicyUpgradeEcuGetNetId(ecu->upgradeOrder);
    u->sequenceId = ipsPolicyUpgradeEcuGetSequenceId(ecu->upgradeOrder);
}
/*! return 1,0,-1 */
static D32S ipsEcuUpgradeOrderCompare(ADM_IPS_ECU_UPGRADE_ORDER_T *a, ADM_IPS_ECU_UPGRADE_ORDER_T *b){
    D32S res = 0;
    if(a->upgradeId > b->upgradeId){
	    res = 1;
    }else if(a->upgradeId == b->upgradeId){
        if (a->netId > b->netId) {
            res = 1;
        }
        else if (a->netId == b->netId) {
            if (a->sequenceId > b->sequenceId) {
                res = 1;
            }
            else if (a->sequenceId == b->sequenceId) {
                res = 0;
            }
            else {
                res = -1;
            }
        }
        else {
            res = -1;
        }
    }else{
	    res = -1;
    }
    return res;
}
static void ipsEcuUpgradeOrderBubbleSort(ADM_IPS_ECU_UPGRADE_ORDER_T *us[], D32S n){
    ADM_IPS_ECU_UPGRADE_ORDER_T *temp;
    for(D32S i=0; i<n; ++i){
        for (D32S j = 1; j < (n - i); ++j) {
            if (us[j - 1] != NULL && us[j] != NULL) {
                if (ipsEcuUpgradeOrderCompare(us[j - 1], us[j]) > 0) {
                    temp = us[j - 1];
                    us[j - 1] = us[j];
                    us[j] = temp;
                }
            }
        }
    }
}
static D32S ipsEcuUpgradeOrderGetGroupCount(ADM_IPS_ECU_UPGRADE_ORDER_T *const us[], D32S n, ADM_IPS_ECU_UPGRADE_ORDER_GROUP_ID_E type){
    D32S i=0;
    D32S len = 0;
    while(len < n){
        D32S tempLen = ipsEcuUpgradeOrderGetGroup(&us[len], n - len, type);
        if (tempLen > 0) {
            len += tempLen;
        }
        else {
            break;
        }

        ++i;
    }
    return i;
}

/* ADM_IPS_ECU_UPGRADE_ORDER_GROUP_ID_E */
static D32S ipsEcuUpgradeOrderGetGroup(ADM_IPS_ECU_UPGRADE_ORDER_T *const us[], D32S n, ADM_IPS_ECU_UPGRADE_ORDER_GROUP_ID_E type){
    D32S len = 0;
    D32S tempA, tempB;
    for(D32S i=0; i<n; ++i){
        const ADM_IPS_ECU_UPGRADE_ORDER_T* u = us[i];
        if (u != NULL) {
            switch (type) {
            case ADM_IPS_ECU_UPGRADE_ORDER_GROUP_ID_UPGRADE:tempA = u->upgradeId; break;
            case ADM_IPS_ECU_UPGRADE_ORDER_GROUP_ID_NET:tempA = u->netId; break;
            case ADM_IPS_ECU_UPGRADE_ORDER_GROUP_ID_SEQUENCE:tempA = u->sequenceId; break;
            default:tempA = ADM_IPS_ECU_UPGRADE_ORDER_MEMBER_DEFAULT_ID; break;
            }

            if (len == 0) {
                tempB = tempA;
            }
            else {
                if (tempB != tempA) {
                    break;
                }
            }
            len++;
        }
    }
    return len;
}

/* ADM_IPS_ECU_UPGRADE_ORDER_ARRAY_T */
static void ipsEcuUpgradeOrderArrayInit(ADM_IPS_ECU_UPGRADE_ORDER_ARRAY_T *u){
    u->ecus = NULL;
    u->ecusLen = 0;
    u->pEcus = NULL;
}
static void ipsEcuUpgradeOrderArrayDeInit(ADM_IPS_ECU_UPGRADE_ORDER_ARRAY_T *u){
    if(NULL != u->ecus){
        for (D32S i = 0; i < u->ecusLen; ++i) {
            ipsEcuUpgradeOrderDeInit(&u->ecus[i]);
        }
        adm_free(u->ecus);
    }
    if(NULL != u->pEcus){
	    adm_free(u->pEcus);
    }
}
static void ipsEcuUpgradeOrderArrayCreate(ADM_IPS_ECU_UPGRADE_ORDER_ARRAY_T *u, const ADM_IPS_POLICY_T *policy){
    D32S maxEcus = ipsPolicyGetMaxEcusCount(policy);
    if(maxEcus > 0){
        u->ecus = adm_malloc((DSIZE)maxEcus * sizeof(u->ecus[0]));
        if (u->ecus != NULL) {
            u->ecusLen = maxEcus;
            for (D32S i = 0; i < u->ecusLen; ++i) {
                ipsEcuUpgradeOrderInit(&u->ecus[i]);
            }

            const ADM_IPS_POLICY_ECU_GROUP_ORDER_ARRAY_T* ecuGroupOrders = &policy->ecuGroupOrders;
            if (ecuGroupOrders->ecuGroupOrders != NULL) {
                D32S k = 0;
                for (D32S i = 0; i < ecuGroupOrders->ecuGroupOrdersLen; ++i) {
                    const ADM_IPS_POLICY_UPGRADE_ECU_ARRAY_T* groupEcus = &ecuGroupOrders->ecuGroupOrders[i].groupEcus;
                    if (groupEcus->ecus != NULL) {
                        for (D32S j = 0; j < groupEcus->ecusLen; ++j) {
                            const ADM_IPS_POLICY_UPGRADE_ECU_T* ecu = &groupEcus->ecus[j];
                            ipsEcuUpgradeOrderCreate(&u->ecus[k], i, j, ecu);
                            ++k;
                        }
                    }
                }
            }

            u->pEcus = adm_malloc((DSIZE)u->ecusLen * sizeof(*(u->pEcus)));
            if (u->pEcus != NULL) {
                for (D32S i = 0; i < u->ecusLen; ++i) {
                    u->pEcus[i] = &u->ecus[i];
                }
                ipsEcuUpgradeOrderBubbleSort(u->pEcus, u->ecusLen);
            }
        }
    }
}

static void ipsEcuUpgradeGroupManagerInit(ADM_IPS_ECU_UPGRADE_GROUP_MANAGER_T* manager)
{
    manager->group = NULL;
    manager->isRollback = DFALSE;
}

static void ipsEcuUpgradeGroupManagerDeinit(ADM_IPS_ECU_UPGRADE_GROUP_MANAGER_T* manager)
{
    if (NULL != manager->group)
    {
        manager->group->func->deinit(manager->group);
    }
}

static void ipsEcuUpgradeGroupManagerClear(ADM_IPS_ECU_UPGRADE_GROUP_MANAGER_T* manager)
{
    ipsEcuUpgradeGroupManagerDeinit(manager);
    ipsEcuUpgradeGroupManagerInit(manager);
}

/* ADM_IPS_ECU_UPGRADE_GROUP_ARRAY_T */
static void ipsEcuUpgradeGroupArrayInit(ADM_IPS_ECU_UPGRADE_GROUP_ARRAY_T *g){
    g->ecus = NULL;
    g->ecusLen = 0;
    g->current = 0;
    g->isRollback = DFALSE;
}
static void ipsEcuUpgradeGroupArrayDeInit(ADM_IPS_ECU_UPGRADE_GROUP_ARRAY_T *g){
    if(NULL != g->ecus){
        for (D32S i = 0; i < g->ecusLen; ++i) {
            ipsEcuUpgradeGroupDeInit(&g->ecus[i]);
        }
        adm_free(g->ecus);
    }
}
static void ipsEcuUpgradeGroupArrayClear(ADM_IPS_ECU_UPGRADE_GROUP_ARRAY_T *g){
    ipsEcuUpgradeGroupArrayDeInit(g);
    ipsEcuUpgradeGroupArrayInit(g);
}

static void ipsEcuUpgradeGroupArrayJsonParse(ADM_IPS_ECU_UPGRADE_GROUP_ARRAY_T *g, const struct json_object *root){
    struct json_object *ecusVal = json_object_object_get(root, "ecus");
    if(ecusVal != NULL){
        if (json_object_get_type(ecusVal) == json_type_array) {
            D32S ecusLen = (D32S)json_object_array_length(ecusVal);
            if (ecusLen > 0) {
                g->ecus = adm_malloc((DSIZE)ecusLen * sizeof(g->ecus[0]));
                if (g->ecus != NULL) {
                    g->ecusLen = ecusLen;
                    for (D32S i = 0; i < g->ecusLen; ++i) {
                        ipsEcuUpgradeGroupInit(&g->ecus[i]);
                        struct json_object* ecuVal = json_object_array_get_idx(ecusVal, (size_t)i);
                        if (ecuVal != NULL) {
                            ipsEcuUpgradeGroupJsonParse(&g->ecus[i], ecuVal);
                        }
                    }
                }
            }
        }
    }
    g->current = json_object_get_int(json_object_object_get(root, "current"));
    g->isRollback = json_object_get_boolean(json_object_object_get(root, "isRollback"));
}
static struct json_object *ipsEcuUpgradeGroupArrayJsonCreate(const ADM_IPS_ECU_UPGRADE_GROUP_ARRAY_T *g){
    struct json_object *root = json_object_new_object();
    if(root != NULL){
        if (g->ecus != NULL && g->ecusLen != 0) {
            struct json_object* ecusVal = json_object_new_array();
            if (ecusVal != NULL) {
                for (D32S i = 0; i < g->ecusLen; ++i) {
                    struct json_object* ecuVal = ipsEcuUpgradeGroupJsonCreate(&g->ecus[i]);
                    if (ecuVal != NULL) {
                        (void)json_object_array_add(ecusVal, ecuVal);
                    }
                }
                (void)json_object_object_add(root, "ecus", ecusVal);
            }
        }

        (void)json_object_object_add(root, "current", json_object_new_int(g->current));
        (void)json_object_object_add(root, "isRollback", json_object_new_boolean(g->isRollback));
    }
    return root;
}
static void ipsEcuUpgradeGroupArrayCreate(ADM_IPS_ECU_UPGRADE_GROUP_ARRAY_T *g, const ADM_IPS_POLICY_T *policy){
    ADM_IPS_ECU_UPGRADE_ORDER_ARRAY_T ecusUpgradeOrder;
    ipsEcuUpgradeOrderArrayInit(&ecusUpgradeOrder);
    ipsEcuUpgradeOrderArrayCreate(&ecusUpgradeOrder, policy);
    if(ecusUpgradeOrder.pEcus != NULL && ecusUpgradeOrder.ecusLen != 0){
        D32S count = ipsEcuUpgradeOrderGetGroupCount(ecusUpgradeOrder.pEcus, ecusUpgradeOrder.ecusLen, ADM_IPS_ECU_UPGRADE_ORDER_GROUP_ID_UPGRADE);
        if (count > 0) {
            g->ecus = adm_malloc((DSIZE)count * sizeof(g->ecus[0]));
            if (g->ecus != NULL) {
                g->ecusLen = count;
                for (D32S i = 0; i < g->ecusLen; ++i) {
                    ipsEcuUpgradeGroupInit(&g->ecus[i]);
                }

                D32S len = 0;
                for (D32S i = 0; i < g->ecusLen; ++i) {
                    D32S n = ipsEcuUpgradeOrderGetGroup(&ecusUpgradeOrder.pEcus[len], ecusUpgradeOrder.ecusLen - len, ADM_IPS_ECU_UPGRADE_ORDER_GROUP_ID_UPGRADE);
                    if (n > 0) {
                        ipsEcuUpgradeGroupCreate(&g->ecus[i], &ecusUpgradeOrder.pEcus[len], n);
                        len += n;
                        if (len >= ecusUpgradeOrder.ecusLen)
                        {
                            break;
                        }
                    }
                    else {
                        break;
                    }
                }
            }
        }
    }
    ipsEcuUpgradeOrderArrayDeInit(&ecusUpgradeOrder);
}
static D32S ipsEcuUpgradeGroupArrayGetInstallEcusCycle(ADM_IPS_ECU_UPGRADE_GROUP_ARRAY_T *g, const ADM_IPS_POLICY_T *policy, ADM_IPS_PROGRESS_T *progress, ADM_IPS_INSTALL_ECUINFO_T *ecus){
    D32S resLen = 0;
    if(g->isRollback == DFALSE){
        for(; g->current < g->ecusLen; ++g->current){
            resLen = ipsEcuUpgradeGroupGetInstallEcus(&g->ecus[g->current], policy, progress, ecus, g->isRollback);
            if(resLen > 0){
        	    break;
            }
        }
    }else{
        for(; g->current >= 0; --g->current){
            resLen = ipsEcuUpgradeGroupGetInstallEcus(&g->ecus[g->current], policy, progress, ecus, g->isRollback);
            if(resLen > 0){
        	    break;
            }
        }
    }
    return resLen;
}
static D32S ipsEcuUpgradeGroupArrayGetInstallEcus(ADM_IPS_ECU_UPGRADE_GROUP_ARRAY_T *g, const ADM_IPS_POLICY_T *policy, ADM_IPS_PROGRESS_T *progress, ADM_IPS_INSTALL_ECUINFO_T *ecus){
    D32S resLen = 0;
    if(g->isRollback == DFALSE){
        resLen = ipsEcuUpgradeGroupArrayGetInstallEcusCycle(g, policy, progress, ecus);
        if (resLen == 0) {
            if (g->current == g->ecusLen) {
                --g->current;
            }
            g->isRollback = DTRUE;
            resLen = ipsEcuUpgradeGroupArrayGetInstallEcusCycle(g, policy, progress, ecus);
        }
    }
    else {
        resLen = ipsEcuUpgradeGroupArrayGetInstallEcusCycle(g, policy, progress, ecus);
    }
    return resLen;
}

/* ADM_IPS_ECU_UPGRADE_GROUP_T */
static void ipsEcuUpgradeGroupInit(ADM_IPS_ECU_UPGRADE_GROUP_T *g){
    ipsEcuNetGroupArrayInit(&g->ecusNetGroup);
}
static void ipsEcuUpgradeGroupDeInit(ADM_IPS_ECU_UPGRADE_GROUP_T *g){
    ipsEcuNetGroupArrayDeInit(&g->ecusNetGroup);
}
static void ipsEcuUpgradeGroupJsonParse(ADM_IPS_ECU_UPGRADE_GROUP_T *g, const struct json_object *root){
    struct json_object *ecusNetGroupVal = json_object_object_get(root, "ecusNetGroup");
    if(ecusNetGroupVal != NULL){
	    ipsEcuNetGroupArrayJsonParse(&g->ecusNetGroup, ecusNetGroupVal);
    }
}
static struct json_object *ipsEcuUpgradeGroupJsonCreate(const ADM_IPS_ECU_UPGRADE_GROUP_T *g){
    struct json_object *root = json_object_new_object();
    if(root != NULL){
        struct json_object* ecusNetGroupVal = ipsEcuNetGroupArrayJsonCreate(&g->ecusNetGroup);
        if (ecusNetGroupVal != NULL) {
            (void)json_object_object_add(root, "ecusNetGroup", ecusNetGroupVal);
        }
    }
    return root;
}
static void ipsEcuUpgradeGroupCreate(ADM_IPS_ECU_UPGRADE_GROUP_T *g, ADM_IPS_ECU_UPGRADE_ORDER_T *const us[], D32S n){
    ipsEcuNetGroupArrayCreate(&g->ecusNetGroup, us, n);
}
static D32S ipsEcuUpgradeGroupGetInstallEcus(ADM_IPS_ECU_UPGRADE_GROUP_T *g, const ADM_IPS_POLICY_T *policy, ADM_IPS_PROGRESS_T *progress, ADM_IPS_INSTALL_ECUINFO_T *ecus, DBOOL isRollback){
    return ipsEcuNetGroupArrayGetInstallEcus(&g->ecusNetGroup, policy, progress, ecus, isRollback);
}

/* ADM_IPS_ECU_NET_GROUP_ARRAY_T */
static void ipsEcuNetGroupArrayInit(ADM_IPS_ECU_NET_GROUP_ARRAY_T *g){
    g->ecus = NULL;
    g->ecusLen = 0;
    //g->current = 0;
}
static void ipsEcuNetGroupArrayDeInit(ADM_IPS_ECU_NET_GROUP_ARRAY_T *g){
    if(NULL != g->ecus){
        for (D32S i = 0; i < g->ecusLen; ++i) {
            ipsEcuNetGroupDeInit(&g->ecus[i]);
        }
        adm_free(g->ecus);
    }
}
static void ipsEcuNetGroupArrayJsonParse(ADM_IPS_ECU_NET_GROUP_ARRAY_T *g, const struct json_object *root){
    if(json_object_get_type(root) == json_type_array){
        D32S ecusLen = (D32S)json_object_array_length(root);
        if (ecusLen != 0) {
            g->ecus = adm_malloc((DSIZE)ecusLen * sizeof(g->ecus[0]));
            if (g->ecus != NULL) {
                g->ecusLen = ecusLen;
                for (D32S i = 0; i < g->ecusLen; ++i) {
                    ipsEcuNetGroupInit(&g->ecus[i]);
                    struct json_object* ecuVal = json_object_array_get_idx(root, (size_t)i);
                    if (ecuVal != NULL) {
                        ipsEcuNetGroupJsonParse(&g->ecus[i], ecuVal);
                    }
                }
            }
        }
    }
}
static struct json_object* ipsEcuNetGroupArrayJsonCreate(const ADM_IPS_ECU_NET_GROUP_ARRAY_T *g){
    struct json_object* root = json_object_new_array();
    if(root != NULL){
        if (g->ecus != NULL && g->ecusLen != 0) {
            for (D32S i = 0; i < g->ecusLen; ++i) {
                struct json_object* ecuVal = ipsEcuNetGroupJsonCreate(&g->ecus[i]);
                if (ecuVal != NULL) {
                    (void)json_object_array_add(root, ecuVal);
                }
            }
        }
    }
    return root;
}
static void ipsEcuNetGroupArrayCreate(ADM_IPS_ECU_NET_GROUP_ARRAY_T *g, ADM_IPS_ECU_UPGRADE_ORDER_T *const us[], D32S n){
    D32S count = ipsEcuUpgradeOrderGetGroupCount(us, n, ADM_IPS_ECU_UPGRADE_ORDER_GROUP_ID_NET);
    if(count != 0){
        g->ecus = adm_malloc((DSIZE)count * sizeof(g->ecus[0]));
        if (g->ecus != NULL) {
            g->ecusLen = count;
            for (D32S i = 0; i < g->ecusLen; ++i) {
                ipsEcuNetGroupInit(&g->ecus[i]);
            }

            D32S len = 0;
            for (D32S i = 0; i < g->ecusLen; ++i) {
                D32S tempLen = ipsEcuUpgradeOrderGetGroup(&us[len], n - len, ADM_IPS_ECU_UPGRADE_ORDER_GROUP_ID_NET);
                if (tempLen > 0) {
                    ipsEcuNetGroupCreate(&g->ecus[i], &us[len], tempLen);
                    len += tempLen;
                    if (len >= n)
                    {
                        break;
                    }
                }
                else {
                    break;
                }
            }
        }
    }
}
static D32S ipsEcuNetGroupArrayGetInstallEcus(ADM_IPS_ECU_NET_GROUP_ARRAY_T *g, const ADM_IPS_POLICY_T *policy, ADM_IPS_PROGRESS_T *progress, ADM_IPS_INSTALL_ECUINFO_T *ecus, DBOOL isRollback){
    D32S len = 0;
    for(D32S i=0; i < g->ecusLen; ++i){
        len += ipsEcuNetGroupGetInstallEcus(&g->ecus[i], policy, progress, &ecus[len], isRollback);
    }
    return len;
}

/* ADM_IPS_ECU_NET_GROUP_T */
static void ipsEcuNetGroupInit(ADM_IPS_ECU_NET_GROUP_T *g){
    ipsEcuSequenceGroupArrayInit(&g->ecusSequenceGroup);
}
static void ipsEcuNetGroupDeInit(ADM_IPS_ECU_NET_GROUP_T *g){
    ipsEcuSequenceGroupArrayDeInit(&g->ecusSequenceGroup);
}
static void ipsEcuNetGroupJsonParse(ADM_IPS_ECU_NET_GROUP_T *g, const struct json_object* root){
    struct json_object* ecusSequenceGroupVal = json_object_object_get(root, "ecusSequenceGroup");
    if(ecusSequenceGroupVal != NULL){
	    ipsEcuSequenceGroupArrayJsonParse(&g->ecusSequenceGroup, ecusSequenceGroupVal);
    }
}
static struct json_object* ipsEcuNetGroupJsonCreate(const ADM_IPS_ECU_NET_GROUP_T *g){
    struct json_object* root = json_object_new_object();
    if(root != NULL){
        struct json_object* ecusSequenceGroupVal = ipsEcuSequenceGroupArrayJsonCreate(&g->ecusSequenceGroup);
        if (ecusSequenceGroupVal != NULL) {
            (void)json_object_object_add(root, "ecusSequenceGroup", ecusSequenceGroupVal);
        }
    }
    return root;
}
static void ipsEcuNetGroupCreate(ADM_IPS_ECU_NET_GROUP_T *g, ADM_IPS_ECU_UPGRADE_ORDER_T *const us[], D32S n){
    ipsEcuSequenceGroupArrayCreate(&g->ecusSequenceGroup, us, n);
}
static D32S ipsEcuNetGroupGetInstallEcus(ADM_IPS_ECU_NET_GROUP_T *g, const ADM_IPS_POLICY_T *policy, ADM_IPS_PROGRESS_T *progress, ADM_IPS_INSTALL_ECUINFO_T *ecus, DBOOL isRollback){
    return ipsEcuSequenceGroupArrayGetInstallEcus(&g->ecusSequenceGroup, policy, progress, ecus, isRollback);
}

/* ADM_IPS_ECU_SEQUENCE_GROUP_ARRAY_T */
static void ipsEcuSequenceGroupArrayInit(ADM_IPS_ECU_SEQUENCE_GROUP_ARRAY_T *g){
    g->ecus = NULL;
    g->ecusLen = 0;
    g->current = 0;
}
static void ipsEcuSequenceGroupArrayDeInit(ADM_IPS_ECU_SEQUENCE_GROUP_ARRAY_T *g){
    if(NULL != g->ecus){
        for (D32S i = 0; i < g->ecusLen; ++i) {
            ipsEcuSequenceGroupDeInit(&g->ecus[i]);
        }
        adm_free(g->ecus);
    }
}
static void ipsEcuSequenceGroupArrayJsonParse(ADM_IPS_ECU_SEQUENCE_GROUP_ARRAY_T *g, const struct json_object *root){
    struct json_object* ecusVal = json_object_object_get(root, "ecus");
    if(ecusVal != NULL){
        if (json_object_get_type(ecusVal) == json_type_array) {
            D32S ecusLen = (D32S)json_object_array_length(ecusVal);
            if (ecusLen > 0) {
                g->ecus = adm_malloc((DSIZE)ecusLen * sizeof(g->ecus[0]));
                if (g->ecus != NULL) {
                    g->ecusLen = ecusLen;
                    for (D32S i = 0; i < g->ecusLen; ++i) {
                        ipsEcuSequenceGroupInit(&g->ecus[i]);
                        struct json_object* ecuVal = json_object_array_get_idx(ecusVal, (size_t)i);
                        if (ecuVal != NULL) {
                            ipsEcuSequenceGroupJsonParse(&g->ecus[i], ecuVal);
                        }
                    }
                }
            }
        }
    }
    g->current = json_object_get_int(json_object_object_get(root, "current"));
}
static struct json_object* ipsEcuSequenceGroupArrayJsonCreate(const ADM_IPS_ECU_SEQUENCE_GROUP_ARRAY_T *g){
    struct json_object* root = json_object_new_object();
    if(root != NULL){
        if (g->ecus != NULL && g->ecusLen > 0) {
            struct json_object* ecusVal = json_object_new_array();
            if (ecusVal != NULL) {
                for (D32S i = 0; i < g->ecusLen; ++i) {
                    struct json_object* ecuVal = ipsEcuSequenceGroupJsonCreate(&g->ecus[i]);
                    if (ecuVal != NULL) {
                        (void)json_object_array_add(ecusVal, ecuVal);
                    }
                }
                (void)json_object_object_add(root, "ecus", ecusVal);
            }
        }

        (void)json_object_object_add(root, "current", json_object_new_int(g->current));
    }
    return root;
}
static void ipsEcuSequenceGroupArrayCreate(ADM_IPS_ECU_SEQUENCE_GROUP_ARRAY_T *g, ADM_IPS_ECU_UPGRADE_ORDER_T *const us[], D32S n){
    D32S count = ipsEcuUpgradeOrderGetGroupCount(us, n, ADM_IPS_ECU_UPGRADE_ORDER_GROUP_ID_SEQUENCE);
    if(count > 0){
        g->ecus = adm_malloc((DSIZE)count * sizeof(g->ecus[0]));
        if (g->ecus != NULL) {
            g->ecusLen = count;
            for (D32S i = 0; i < g->ecusLen; ++i) {
                ipsEcuSequenceGroupInit(&g->ecus[i]);
            }

            D32S len = 0;
            for (D32S i = 0; i < g->ecusLen; ++i) {
                D32S tempLen = ipsEcuUpgradeOrderGetGroup(&us[len], n - len, ADM_IPS_ECU_UPGRADE_ORDER_GROUP_ID_SEQUENCE);
                if (tempLen > 0) {
                    ipsEcuSequenceGroupCreate(&g->ecus[i], &us[len], tempLen);
                    len += tempLen;
                    if (len >= n)
                    {
                        break;
                    }
                }
                else {
                    break;
                }
            }
        }
    }
}
static D32S ipsEcuSequenceGroupArrayGetInstallEcus(ADM_IPS_ECU_SEQUENCE_GROUP_ARRAY_T *g, const ADM_IPS_POLICY_T *policy, ADM_IPS_PROGRESS_T *progress, ADM_IPS_INSTALL_ECUINFO_T *ecus, DBOOL isRollback){
    D32S resLen = 0;
    if(isRollback == DFALSE){
        for (; g->current < g->ecusLen; ++g->current) {
            resLen = ipsEcuSequenceGroupGetInstallEcus(&g->ecus[g->current], policy, progress, ecus, isRollback);
            if (resLen > 0) {
                break;
            }
        }
    }
    else {
        if (g->current == g->ecusLen) {
            --g->current;
        }
        for (; g->current >= 0; --g->current) {
            resLen = ipsEcuSequenceGroupGetInstallEcus(&g->ecus[g->current], policy, progress, ecus, isRollback);
            if (resLen > 0) {
                break;
            }
        }
    }
    return resLen;
}

/* ADM_IPS_ECU_SEQUENCE_GROUP_T */
static void ipsEcuSequenceGroupInit(ADM_IPS_ECU_SEQUENCE_GROUP_T *g){
    ipsEcuIndexArrayInit(&g->indexs);
}
static void ipsEcuSequenceGroupDeInit(ADM_IPS_ECU_SEQUENCE_GROUP_T *g){
    ipsEcuIndexArrayDeInit(&g->indexs);
}
static void ipsEcuSequenceGroupJsonParse(ADM_IPS_ECU_SEQUENCE_GROUP_T *g, const struct json_object *root){
    struct json_object *indexsVal = json_object_object_get(root, "indexs");
    if(indexsVal != NULL){
	    ipsEcuIndexArrayJsonParse(&g->indexs, indexsVal);
    }
}
static struct json_object* ipsEcuSequenceGroupJsonCreate(const ADM_IPS_ECU_SEQUENCE_GROUP_T *g){
    struct json_object* root = json_object_new_object();
    if(root != NULL){
        struct json_object* indexsVal = ipsEcuIndexArrayJsonCreate(&g->indexs);
        if (indexsVal != NULL) {
            (void)json_object_object_add(root, "indexs", indexsVal);
        }
    }
    return root;
}
static void ipsEcuSequenceGroupCreate(ADM_IPS_ECU_SEQUENCE_GROUP_T *g, ADM_IPS_ECU_UPGRADE_ORDER_T *const us[], D32S n){
    ipsEcuIndexArrayCreate(&g->indexs, us, n);
}
static D32S ipsEcuSequenceGroupGetInstallEcus(ADM_IPS_ECU_SEQUENCE_GROUP_T *g, const ADM_IPS_POLICY_T *policy, ADM_IPS_PROGRESS_T *progress, ADM_IPS_INSTALL_ECUINFO_T *ecus, DBOOL isRollback){
    return ipsEcuIndexArrayGetInstallEcus(&g->indexs, policy, progress, ecus, isRollback);
}

/* ADM_IPS_ECU_INDEX_ARRAY_T */
static void ipsEcuIndexArrayInit(ADM_IPS_ECU_INDEX_ARRAY_T *ia){
    ia->indexs = NULL;
}
static void ipsEcuIndexArrayDeInit(ADM_IPS_ECU_INDEX_ARRAY_T *ia){
    if(NULL != ia->indexs){
        for (D32S i = 0; i < ia->indexsLen; ++i) {
            ipsEcuIndexDeInit(&ia->indexs[i]);
        }
        adm_free(ia->indexs);
    }
}
static void ipsEcuIndexArrayJsonParse(ADM_IPS_ECU_INDEX_ARRAY_T *ia, const struct json_object* root){
    if(json_object_get_type(root) == json_type_array){
        D32S indexsLen = (D32S)json_object_array_length(root);
        if (indexsLen > 0) {
            ia->indexs = adm_malloc((DSIZE)indexsLen * sizeof(ia->indexs[0]));
            if (ia->indexs != NULL) {
                ia->indexsLen = indexsLen;
                for (D32S i = 0; i < ia->indexsLen; ++i) {
                    ipsEcuIndexInit(&ia->indexs[i]);
                    struct json_object* indexVal = json_object_array_get_idx(root, (size_t)i);
                    if (indexVal != NULL) {
                        ipsEcuIndexJsonParse(&ia->indexs[i], indexVal);
                    }
                }
            }
        }
    }
}
static struct json_object* ipsEcuIndexArrayJsonCreate(const ADM_IPS_ECU_INDEX_ARRAY_T *ia){
    struct json_object* root = json_object_new_array();
    if(root != NULL){
	    if(ia->indexs != NULL && ia->indexsLen > 0){
	        for(D32S i=0; i<ia->indexsLen; ++i){
                struct json_object* indexVal = ipsEcuIndexJsonCreate(&ia->indexs[i]);
                if (indexVal != NULL) {
                    (void)json_object_array_add(root, indexVal);
                }
	        }
	    }
    }
    return root;
}
static void ipsEcuIndexArrayCreate(ADM_IPS_ECU_INDEX_ARRAY_T *ia, ADM_IPS_ECU_UPGRADE_ORDER_T *const us[], D32S n){
    if(n > 0){
        ia->indexs = adm_malloc((DSIZE)n * sizeof(*(ia->indexs)));
        if (ia->indexs != NULL) {
            ia->indexsLen = n;
            for (D32S i = 0; i < ia->indexsLen; ++i) {
                ipsEcuIndexInit(&ia->indexs[i]);
                const ADM_IPS_ECU_UPGRADE_ORDER_T* u = us[i];
                if (u != NULL) {
                    ipsEcuIndexClone(&u->index, &ia->indexs[i]);
                }
            }
        }
    }
}
static D32S ipsEcuIndexArrayGetInstallEcus(ADM_IPS_ECU_INDEX_ARRAY_T *ia, const ADM_IPS_POLICY_T *policy, ADM_IPS_PROGRESS_T *progress, ADM_IPS_INSTALL_ECUINFO_T *ecus, DBOOL isRollback){
    D32S len = 0;
    for(D32S i=0; i<ia->indexsLen; ++i){
        if(ipsEcuIndexGetInstallEcu(&ia->indexs[i], policy, progress, &ecus[len], isRollback) == DFALSE){}else{
            ++len;
        }
    }
    return len;
}

/* ADM_IPS_ECU_INDEX_T */
static void ipsEcuIndexInit(ADM_IPS_ECU_INDEX_T *in){
    in->groupIndex = 0;
    in->memberIndex = 0;
#ifdef ADM_IPS_POLICY_UPGRADE_ORDER_OLD
    in->upgradeOrder = 0;
#else
    in->upgradeOrder = NULL;
#endif//ADM_IPS_POLICY_UPGRADE_ORDER_OLD
}
static void ipsEcuIndexDeInit(ADM_IPS_ECU_INDEX_T *in){
    adm_free(in->upgradeOrder);
}
static void ipsEcuIndexClone(const ADM_IPS_ECU_INDEX_T *src, ADM_IPS_ECU_INDEX_T *dst){
    dst->groupIndex = src->groupIndex;
    dst->memberIndex = src->memberIndex;
#ifdef ADM_IPS_POLICY_UPGRADE_ORDER_OLD
    dst->upgradeOrder = src->upgradeOrder;
#else
    if(src->upgradeOrder != NULL){dst->upgradeOrder = adm_strdup(src->upgradeOrder);}
#endif//ADM_IPS_POLICY_UPGRADE_ORDER_OLD
}
static void ipsEcuIndexJsonParse(ADM_IPS_ECU_INDEX_T *in, const json_object* root){
    in->groupIndex = json_object_get_int(json_object_object_get(root, "groupIndex"));
    in->memberIndex = json_object_get_int(json_object_object_get(root, "memberIndex"));
#ifdef ADM_IPS_POLICY_UPGRADE_ORDER_OLD
    in->upgradeOrder = json_object_get_int(json_object_object_get(root, "upgradeOrder"));
#else
    struct json_object* upgradeOrderVal = json_object_object_get(root, "upgradeOrder");
    if(upgradeOrderVal != NULL){
        const DCHAR* upgradeOrder = json_object_get_string(upgradeOrderVal);
        if (upgradeOrder != NULL) { in->upgradeOrder = adm_strdup(upgradeOrder); }
    }
#endif//ADM_IPS_POLICY_UPGRADE_ORDER_OLD
}
static struct json_object* ipsEcuIndexJsonCreate(const ADM_IPS_ECU_INDEX_T *in){
    struct json_object* root = json_object_new_object();
    if(root != NULL){
	(void)json_object_object_add(root, "groupIndex", json_object_new_int(in->groupIndex));
	(void)json_object_object_add(root, "memberIndex", json_object_new_int(in->memberIndex));
#ifdef ADM_IPS_POLICY_UPGRADE_ORDER_OLD
	(void)json_object_object_add(root, "upgradeOrder", json_object_new_int(in->upgradeOrder));
#else
	if(in->upgradeOrder != NULL){
	    struct json_object* upgradeOrderVal = json_object_new_string(in->upgradeOrder);
	    if(upgradeOrderVal != NULL){
		    (void)json_object_object_add(root, "upgradeOrder", upgradeOrderVal);
	    }
	}
#endif//ADM_IPS_POLICY_UPGRADE_ORDER_OLD
    }
    return root;
}
#ifdef ADM_IPS_POLICY_UPGRADE_ORDER_OLD
static void ipsEcuIndexCreate(ADM_IPS_ECU_INDEX_T *in, D32S groupIndex, D32S memberIndex, D32S upgradeOrder){
    in->groupIndex = groupIndex;
    in->memberIndex = memberIndex;
    in->upgradeOrder = upgradeOrder;
}
#else
static void ipsEcuIndexCreate(ADM_IPS_ECU_INDEX_T *in, D32S groupIndex, D32S memberIndex, const DCHAR* upgradeOrder){
    in->groupIndex = groupIndex;
    in->memberIndex = memberIndex;
    if(upgradeOrder != NULL){
        in->upgradeOrder = adm_strdup(upgradeOrder);
    }
}
#endif//ADM_IPS_POLICY_UPGRADE_ORDER_OLD
static DBOOL ipsEcuCanInstall(ADM_IPS_ECU_PHASE_E state);
static DBOOL ipsEcuIndexGetInstallEcu(ADM_IPS_ECU_INDEX_T *in, const ADM_IPS_POLICY_T *policy, ADM_IPS_PROGRESS_T *progress, ADM_IPS_INSTALL_ECUINFO_T *ecu, DBOOL isRollback){
    DBOOL dRet = DFALSE;
    const ADM_IPS_POLICY_UPGRADE_ECU_T *upgradeEcu = ipsPolicyFindUpgradeEcu(policy, in);
    if(upgradeEcu != NULL){
        if (upgradeEcu->ecuReqId != NULL && upgradeEcu->ecuSwDid != NULL) {
            ADM_IPS_ECU_PROGRESS_T* progressEcu = ipsProgressFindEcu(progress, upgradeEcu->ecuReqId, upgradeEcu->ecuSwDid);
            if (progressEcu != NULL && DTRUE == ipsEcuCanInstall(progressEcu->state)) {
                dRet = ipsEcuProgressGetInstallEcu(progressEcu, upgradeEcu, ecu, isRollback);
            }
        }
    }
    return dRet;
}

/* ADM_IPS_PROGRESS_T */
static void ipsProgressInit(ADM_IPS_PROGRESS_T *p){
    p->sessionId = NULL;
    p->estimateTotaltime = 0;
    p->state = ADM_IPS_PHASE_IDLE;
    p->progress = 0U;
    p->result = 0;
    ipsEcuProgressArrayInit(&p->ecus);
}
static void ipsProgressDeInit(ADM_IPS_PROGRESS_T *p){
    adm_free(p->sessionId);
    ipsEcuProgressArrayDeInit(&p->ecus);
}
static void ipsProgressClone(const ADM_IPS_PROGRESS_T *src, ADM_IPS_PROGRESS_T *dst){
    if(src->sessionId != NULL){dst->sessionId = adm_strdup(src->sessionId);}
    dst->estimateTotaltime = src->estimateTotaltime;
    dst->state = src->state;
    dst->progress = src->progress;
    dst->result = src->result;
    ipsEcuProgressArrayClone(&src->ecus, &dst->ecus);
}
static void ipsProgressClear(ADM_IPS_PROGRESS_T *p){
    ipsProgressDeInit(p);
    ipsProgressInit(p);
}
static void ipsProgressJsonParse(ADM_IPS_PROGRESS_T *progress, const struct json_object* root){
    const DCHAR *sessionId = json_object_get_string(json_object_object_get(root, "sessionId"));
    if(NULL != sessionId)
    {
        progress->sessionId = adm_strdup(sessionId);
    }
    progress->estimateTotaltime = json_object_get_int(json_object_object_get(root, "estimateTotaltime"));
    progress->state = (ADM_IPS_PHASE_E)json_object_get_int(json_object_object_get(root, "state"));
    progress->progress = (D32U)json_object_get_int(json_object_object_get(root, "progress"));
    progress->result = json_object_get_int(json_object_object_get(root, "result"));
    struct json_object* ecusVal = json_object_object_get(root, "ecus");
    if(NULL != ecusVal){
	    ipsEcuProgressArrayJsonParse(&progress->ecus, ecusVal);
    }
}
static struct json_object* ipsProgressJsonCreate(const ADM_IPS_PROGRESS_T *progress){
    struct json_object* root = json_object_new_object();
    if(NULL != root){
        if (progress->sessionId != NULL) { (void)json_object_object_add(root, "sessionId", json_object_new_string(progress->sessionId)); }
        (void)json_object_object_add(root, "estimateTotaltime", json_object_new_int(progress->estimateTotaltime));
        (void)json_object_object_add(root, "state", json_object_new_int((D32S)progress->state));
        (void)json_object_object_add(root, "progress", json_object_new_int((D32S)progress->progress));
        (void)json_object_object_add(root, "result", json_object_new_int(progress->result));
        struct json_object* ecusVal = ipsEcuProgressArrayJsonCreate(&progress->ecus);
        if (NULL != ecusVal) {
            (void)json_object_object_add(root, "ecus", ecusVal);
        }
    }
    return root;
}
static void ipsProgressCreate(ADM_IPS_PROGRESS_T *p, const ADM_IPS_POLICY_TASK_T *task, const ADM_IPS_POLICY_T *policy){
    if(task->sessionId != NULL){p->sessionId = adm_strdup(task->sessionId);}
    p->estimateTotaltime = policy->totalTimeConsuming;
    ipsEcuProgressArrayCreate(&p->ecus, policy);
}
static void ipsProgressInstallPrepare(ADM_IPS_PROGRESS_T *p){
    p->state = ADM_IPS_PHASE_PRE_INSTALL;
}
static void ipsProgressInstallStart(ADM_IPS_PROGRESS_T *p){
    p->state = ADM_IPS_PHASE_INSTALL;
}
static void ipsProgressPostInstall(ADM_IPS_PROGRESS_T *p, D32S result){
    p->result = result;
    p->state = ADM_IPS_PHASE_POST_INSTALL;
}
static DBOOL ipsProgressEcuSet(ADM_IPS_PROGRESS_T *p, const ADM_IPS_ECU_CURRENT_PROGRESS_T *progress, const ADM_IPS_POLICY_T *policy){
    DBOOL dRet = DFALSE;
    if(progress->ecuReqId != NULL && progress->ecuSwDid != NULL){
        ADM_IPS_ECU_PROGRESS_T* ecuProgress = ipsProgressFindEcu(p, progress->ecuReqId, progress->ecuSwDid);
        if (ecuProgress != NULL) {
            if (progress->isEnd == DFALSE) {
                dRet = ipsEcuProgressSetProgress(ecuProgress, progress, p);
            }
            else {
                ipsEcuProgressSetResult(ecuProgress, progress, policy, p);
                dRet = DTRUE;
            }
        }
    }
    return dRet;
}
static ADM_IPS_ECU_PROGRESS_T *ipsProgressFindEcu(ADM_IPS_PROGRESS_T *p, const DCHAR *ecuReqId, const DCHAR *ecuSwDid){
    ADM_IPS_ECU_PROGRESS_T *progress = NULL;
    if(p->ecus.ecus != NULL && ecuReqId != NULL && ecuSwDid != NULL){
        for (D32S i = 0; i < p->ecus.ecusLen; ++i) {
            ADM_IPS_ECU_PROGRESS_T* ecu = p->ecus.ecus + i;
            if (adm_strcmp(ecuReqId, ecu->ecuReqId) == 0 && adm_strcmp(ecuSwDid, ecu->ecuSwDid) == 0) {
                progress = ecu;
                break;
            }
        }
    }
    return progress;
}

static void ipsProgressUpdateProgress(ADM_IPS_PROGRESS_T *progress){
    const ADM_IPS_ECU_PROGRESS_ARRAY_T *ecus = &progress->ecus;
    if(ecus->ecus != NULL && ecus->ecusLen != 0)
    {
        D32U totalProgress = 0;
        for (D32S i = 0; i < ecus->ecusLen; ++i)
        {
            D32U A = ecus->ecus[i].progress[ADM_IPS_ECU_PHASE_INDEX_UPGRADE].progress;
            D32U B = ecus->ecus[i].progress[ADM_IPS_ECU_PHASE_INDEX_ROLLBACK].progress;
            totalProgress += ((B != 0U) ? B : A) * (D32U)ecus->ecus[i].precent;
        }

        D32U resprogress = totalProgress / 100U;
        if (progress->progress < resprogress)
        {
            progress->progress = resprogress;
            log_d("%s:%u", ADM_STR_FROM_PARAM(progress->progress), progress->progress);
        }
    }
}

static ADM_IPS_ECU_PHASE_E ipsProgressGetEcuState(ADM_IPS_PROGRESS_T* p, const DCHAR* ecuReqId, const DCHAR* ecuSwDid){
    ADM_IPS_ECU_PROGRESS_T* progress = ipsProgressFindEcu(p, ecuReqId, ecuSwDid);

    return (NULL != progress) ? progress->state : ADM_IPS_ECU_PHASE_INVALID;
}

static DBOOL ipsEcuIsIsInstalling(ADM_IPS_ECU_PHASE_E state)
{
    DBOOL iRet = DFALSE;
    switch (state){
    case ADM_IPS_ECU_PHASE_INSTALL:
        /*no break*/
    case ADM_IPS_ECU_PHASE_INSTALL_ROLLBACK:
        iRet = DTRUE;
        break;
    default:
        break;
    }

    return iRet;
}

static DBOOL ipsEcuCanInstall(ADM_IPS_ECU_PHASE_E state)
{
    DBOOL iRet;
    switch (state) {
    case ADM_IPS_ECU_PHASE_PRE_INSTALL:
        /*no break*/
    case ADM_IPS_ECU_PHASE_INSTALL:
        /*no break*/
    case ADM_IPS_ECU_PHASE_TIMELY_PRE_INSTALL_ROLLBACK:
        /*no break*/
    case ADM_IPS_ECU_PHASE_PRE_INSTALL_ROLLBACK:
        /*no break*/
    case ADM_IPS_ECU_PHASE_INSTALL_ROLLBACK:
        iRet = DTRUE;
        break;
    default:
        iRet = DFALSE;
        break;
    }

    return iRet;
}

/* ADM_IPS_ECU_PROGRESS_ARRAY_T */
static void ipsEcuProgressArrayInit(ADM_IPS_ECU_PROGRESS_ARRAY_T *p){
    p->ecus = NULL;
    p->ecusLen = 0;
}
static void ipsEcuProgressArrayDeInit(ADM_IPS_ECU_PROGRESS_ARRAY_T *p){
    if(NULL != p->ecus){
        for (D32S i = 0; i < p->ecusLen; ++i) {
            ipsEcuProgressDeInit(&p->ecus[i]);
        }
        adm_free(p->ecus);
    }
}
static void ipsEcuProgressArrayClone(const ADM_IPS_ECU_PROGRESS_ARRAY_T *src, ADM_IPS_ECU_PROGRESS_ARRAY_T *dst){
    if(src->ecus != NULL && src->ecusLen != 0){
        dst->ecus = adm_malloc((DSIZE)src->ecusLen * sizeof(dst->ecus[0]));
        if (dst->ecus != NULL) {
            dst->ecusLen = src->ecusLen;
            for (D32S i = 0; i < dst->ecusLen; ++i) {
                ipsEcuProgressInit(&dst->ecus[i]);
                ipsEcuProgressClone(&src->ecus[i], &dst->ecus[i]);
            }
        }
    }
}
static void ipsEcuProgressArrayJsonParse(ADM_IPS_ECU_PROGRESS_ARRAY_T *p, const struct json_object* root){
    if(json_object_get_type(root) == json_type_array){
        D32S ecusLen = (D32S)json_object_array_length(root);
        if (ecusLen > 0) {
            p->ecus = adm_malloc((DSIZE)ecusLen * sizeof(p->ecus[0]));
            if (NULL != p->ecus) {
                p->ecusLen = ecusLen;
                for (D32S i = 0; i < p->ecusLen; ++i) {
                    ipsEcuProgressInit(&p->ecus[i]);
                    struct json_object* ecuVal = json_object_array_get_idx(root, (size_t)i);
                    if (NULL != ecuVal) {
                        ipsEcuProgressJsonParse(&p->ecus[i], ecuVal);
                    }
                }
            }
        }
    }
}
static struct json_object* ipsEcuProgressArrayJsonCreate(const ADM_IPS_ECU_PROGRESS_ARRAY_T *p){
    struct json_object* root = json_object_new_array();
    if(NULL != root){
        if (NULL != p->ecus) {
            for (D32S i = 0; i < p->ecusLen; ++i) {
                struct json_object* ecuVal = ipsEcuProgressJsonCreate(&p->ecus[i]);
                if (NULL != ecuVal) {
                    (void)json_object_array_add(root, ecuVal);
                }
            }
        }
    }
    return root;
}
static void ipsEcuProgressArrayCreate(ADM_IPS_ECU_PROGRESS_ARRAY_T *p, const ADM_IPS_POLICY_T *policy){
    D32S count = ipsPolicyGetMaxEcusCount(policy);
    if(count > 0){
        p->ecus = adm_malloc((DSIZE)count * sizeof(p->ecus[0]));
        if (p->ecus != NULL) {
            p->ecusLen = count;
            for (D32S i = 0; i < p->ecusLen; ++i) {
                ipsEcuProgressInit(&p->ecus[i]);
            }

            const ADM_IPS_POLICY_ECU_GROUP_ORDER_ARRAY_T* ecuGroupOrders = &policy->ecuGroupOrders;
            D32S totalEcuTimeConsuming = ipsPolicyEcuGroupOrderArrayGetTotalEcuTimeConsuming(ecuGroupOrders);
            if (ecuGroupOrders->ecuGroupOrders != NULL) {
                D32S k = 0;
                for (D32S i = 0; i < ecuGroupOrders->ecuGroupOrdersLen; ++i) {
                    const ADM_IPS_POLICY_UPGRADE_ECU_ARRAY_T* groupEcus = &ecuGroupOrders->ecuGroupOrders[i].groupEcus;
                    if (groupEcus->ecus != NULL) {
                        for (D32S j = 0; j < groupEcus->ecusLen; ++j) {
                            const ADM_IPS_POLICY_UPGRADE_ECU_T* ecu = &groupEcus->ecus[j];
                            ipsEcuProgressCreate(&p->ecus[k], ecu, totalEcuTimeConsuming);
                            ++k;
                        }
                    }
                }
            }

            D32S totalPrecent = 0;
            for (D32S i = 0; i < p->ecusLen; ++i) {
                totalPrecent += p->ecus[i].precent;
            }
            ADM_IPS_ECU_PROGRESS_T* lastEcu = &p->ecus[p->ecusLen - 1];
            ipsEcuProgressSetPrecent(lastEcu, lastEcu->precent + 100 - totalPrecent);
        }
    }
}

/* ADM_IPS_ECU_PROGRESS_T */
static void ipsEcuProgressInit(ADM_IPS_ECU_PROGRESS_T *p){
    p->ecuReqId = NULL;
    p->ecuSwDid = NULL;
    p->state = ADM_IPS_ECU_PHASE_PRE_INSTALL;
    p->bIsAb = DFALSE;
    p->bBlockSwitchAb = DFALSE;
    p->bIsFLashHasSwitchAb = DFALSE;
    p->bIsRollbackHasSwitchAB = DFALSE;
    p->eDestAbInfo = 0;
    for(D32S i=0; i<(D32S)(sizeof(p->progress)/sizeof(p->progress[0])); ++i){
        p->progress[i].progress = 0;
        p->progress[i].result = 0;
        p->progress[i].upgradeNumbers = 0;
        p->progress[i].beginTime = 0;
        p->progress[i].endTime = 0;
        adm_memzero(p->progress[i].resultDescrip,RESULT_DESCRIP_LEN);
    }
    p->precent = 0;
}
static void ipsEcuProgressDeInit(ADM_IPS_ECU_PROGRESS_T *p){
    if(NULL != p->ecuReqId){adm_free(p->ecuReqId);}
    if(NULL != p->ecuSwDid){adm_free(p->ecuSwDid);}
}
static void ipsEcuProgressClone(const ADM_IPS_ECU_PROGRESS_T *src, ADM_IPS_ECU_PROGRESS_T *dst){
    if(src->ecuReqId != NULL){dst->ecuReqId = adm_strdup(src->ecuReqId);}
    if(src->ecuSwDid != NULL){dst->ecuSwDid = adm_strdup(src->ecuSwDid);}
    dst->state = src->state;
    dst->bBlockSwitchAb = src->bBlockSwitchAb;
    dst->bIsAb = src->bIsAb;
    dst->bIsFLashHasSwitchAb = src->bIsFLashHasSwitchAb;
    dst->bIsRollbackHasSwitchAB = src->bIsRollbackHasSwitchAB;
    dst->eDestAbInfo = src->eDestAbInfo;
    for(D32S i=0; i<(D32S)ADM_IPS_ECU_PHASE_INDEX_MAX; ++i){
        dst->progress[i].progress = src->progress[i].progress;
        dst->progress[i].result = src->progress[i].result;
        dst->progress[i].upgradeNumbers = src->progress[i].upgradeNumbers;
        dst->progress[i].beginTime = src->progress[i].beginTime;
        dst->progress[i].endTime = src->progress[i].endTime;
        adm_snprintf(dst->progress[i].resultDescrip, RESULT_DESCRIP_LEN, "%s", src->progress[i].resultDescrip);
    }
    dst->precent = src->precent;
}
static void ipsEcuProgressJsonParse(ADM_IPS_ECU_PROGRESS_T *p, const struct json_object* root){
    const DCHAR *tempStr;
    tempStr = json_object_get_string(json_object_object_get(root, "ecuReqId"));if(NULL != tempStr){p->ecuReqId = adm_strdup(tempStr);}
    tempStr = json_object_get_string(json_object_object_get(root, "ecuSwDid"));if(NULL != tempStr){p->ecuSwDid = adm_strdup(tempStr);}
    p->state = (ADM_IPS_ECU_PHASE_E)json_object_get_int(json_object_object_get(root, "state"));
    p->bIsAb = json_object_get_int(json_object_object_get(root, "isAb"));
    p->bBlockSwitchAb = json_object_get_int(json_object_object_get(root, "isBlockSwitchAb"));
    p->eDestAbInfo = json_object_get_int(json_object_object_get(root, "destAbInfo"));
    p->bIsFLashHasSwitchAb = json_object_get_int(json_object_object_get(root, "isFLashHasSwitchAb"));
    p->bIsRollbackHasSwitchAB = json_object_get_int(json_object_object_get(root, "isRollbackHasSwitchAB"));
    struct json_object* progressArrVal = json_object_object_get(root, "progress");
    if(NULL != progressArrVal){
        if(json_object_get_type(progressArrVal) == json_type_array){
            D32S progressLen = (D32S)json_object_array_length(progressArrVal);
            if(progressLen == (D32S)(sizeof(p->progress)/sizeof(p->progress[0]))){
                for(D32S i=0; i<progressLen; ++i){
                    struct json_object* progressVal = json_object_array_get_idx(progressArrVal, (size_t)i);
                    if(NULL != progressVal){
            	        p->progress[i].progress = (D32U)json_object_get_int(json_object_object_get(progressVal, "progress"));
            	        p->progress[i].result = json_object_get_int(json_object_object_get(progressVal, "result"));
            	        p->progress[i].upgradeNumbers = json_object_get_int(json_object_object_get(progressVal, "upgradeNumbers"));
            	        p->progress[i].beginTime = json_object_get_int64(json_object_object_get(progressVal, "beginTime"));
            	        p->progress[i].endTime = json_object_get_int64(json_object_object_get(progressVal, "endTime"));
                        tempStr = json_object_get_string(json_object_object_get(progressVal, "resultDescrip"));
                        adm_snprintf(p->progress[i].resultDescrip,RESULT_DESCRIP_LEN,"%s",tempStr);
                    }
                }
            }
        }
    }
    p->precent = json_object_get_int(json_object_object_get(root, "precent"));
}
static struct json_object* ipsEcuProgressJsonCreate(const ADM_IPS_ECU_PROGRESS_T *p){
    struct json_object* root = json_object_new_object();  
    if(NULL != root){
        if (NULL != p->ecuReqId) { (void)json_object_object_add(root, "ecuReqId", json_object_new_string(p->ecuReqId)); }
        if (NULL != p->ecuSwDid) { (void)json_object_object_add(root, "ecuSwDid", json_object_new_string(p->ecuSwDid)); }
        (void)json_object_object_add(root, "state", json_object_new_int((D32S)p->state));
        (void)json_object_object_add(root, "isAb", json_object_new_int((D32S)p->bIsAb));
        (void)json_object_object_add(root, "isBlockSwitchAb", json_object_new_int((D32S)p->bBlockSwitchAb));
        (void)json_object_object_add(root, "destAbInfo", json_object_new_uint64((D64U)p->eDestAbInfo));
        json_object_object_add(root, "isFLashHasSwitchAb", json_object_new_int(p->bIsFLashHasSwitchAb));
        json_object_object_add(root, "isRollbackHasSwitchAB", json_object_new_int(p->bIsRollbackHasSwitchAB));
        struct json_object* progressArrVal = json_object_new_array();
        if (NULL != progressArrVal) {
            D32S progressLen = (D32S)(sizeof(p->progress) / sizeof(p->progress[0]));
            for (D32S i = 0; i < progressLen; ++i) {
                struct json_object* progressVal = json_object_new_object();
                if (NULL != progressVal) {
                    (void)json_object_object_add(progressVal, "progress", json_object_new_int((D32S)p->progress[i].progress));
                    (void)json_object_object_add(progressVal, "result", json_object_new_int(p->progress[i].result));
                    (void)json_object_object_add(progressVal, "upgradeNumbers", json_object_new_int(p->progress[i].upgradeNumbers));
                    (void)json_object_object_add(progressVal, "beginTime", json_object_new_int64(p->progress[i].beginTime));
                    (void)json_object_object_add(progressVal, "endTime", json_object_new_int64(p->progress[i].endTime));
                    (void)json_object_object_add(progressVal, "resultDescrip", json_object_new_string(p->progress[i].resultDescrip));
                    (void)json_object_array_add(progressArrVal, progressVal);
                }
            }
            (void)json_object_object_add(root, "progress", progressArrVal);
        }
        (void)json_object_object_add(root, "precent", json_object_new_int(p->precent));
    }
    return root;
}
static void ipsEcuProgressCreate(ADM_IPS_ECU_PROGRESS_T *p, const ADM_IPS_POLICY_UPGRADE_ECU_T *ecu, D32S totalEcuTimeConsuming){
    if(NULL != ecu->ecuReqId){p->ecuReqId = adm_strdup(ecu->ecuReqId);}
    if(NULL != ecu->ecuSwDid){p->ecuSwDid = adm_strdup(ecu->ecuSwDid);}
    if(totalEcuTimeConsuming != 0){
	    p->precent = ecu->ecuTimeConsuming * 100 / totalEcuTimeConsuming;
    }
}
static void ipsEcuProgressSetPrecent(ADM_IPS_ECU_PROGRESS_T *p, D32S precent){
    if(p->precent != precent){
        p->precent = precent;
	log_d("%s:%s, %s:%s, %s:%d",
		ADM_STR_FROM_PARAM(p->ecuReqId), ADM_STR_SAFE_OUTPUT(p->ecuReqId),
		ADM_STR_FROM_PARAM(p->ecuSwDid), ADM_STR_SAFE_OUTPUT(p->ecuSwDid),
		ADM_STR_FROM_PARAM(p->precent), p->precent);
    }
}
static DBOOL ipsEcuProgressGetInstallEcu(ADM_IPS_ECU_PROGRESS_T *p, const ADM_IPS_POLICY_UPGRADE_ECU_T *upgradeEcu, ADM_IPS_INSTALL_ECUINFO_T *ecu, DBOOL isRollback){
    DBOOL dRet = DFALSE;
    if(isRollback == DFALSE){
        if (p->state == ADM_IPS_ECU_PHASE_PRE_INSTALL) {
            p->progress[ADM_IPS_ECU_PHASE_INDEX_UPGRADE].beginTime = time(NULL);
            p->state = ADM_IPS_ECU_PHASE_INSTALL;
            ipsInstallEcuInfoCreate(ecu, upgradeEcu, DFALSE);
            dRet = DTRUE;
        }
        else if (p->state == ADM_IPS_ECU_PHASE_INSTALL) {
            ipsInstallEcuInfoCreate(ecu, upgradeEcu, DFALSE);
            dRet = DTRUE;
        }
        else if (p->state == ADM_IPS_ECU_PHASE_TIMELY_PRE_INSTALL_ROLLBACK) {
            p->progress[ADM_IPS_ECU_PHASE_INDEX_ROLLBACK].beginTime = time(NULL);
            p->state = ADM_IPS_ECU_PHASE_INSTALL_ROLLBACK;
            ipsInstallEcuInfoCreate(ecu, upgradeEcu, DTRUE);
            dRet = DTRUE;
        }
        else if (p->state == ADM_IPS_ECU_PHASE_INSTALL_ROLLBACK) {
            ipsInstallEcuInfoCreate(ecu, upgradeEcu, DTRUE);
            dRet = DTRUE;
        }
        else
        {
            /*null*/
        }
    }
    else {
        if (p->state == ADM_IPS_ECU_PHASE_PRE_INSTALL_ROLLBACK) {
            p->progress[ADM_IPS_ECU_PHASE_INDEX_ROLLBACK].beginTime = time(NULL);
            p->state = ADM_IPS_ECU_PHASE_INSTALL_ROLLBACK;
            ipsInstallEcuInfoCreate(ecu, upgradeEcu, DTRUE);
            dRet = DTRUE;
        }
        else if (p->state == ADM_IPS_ECU_PHASE_INSTALL_ROLLBACK) {
            ipsInstallEcuInfoCreate(ecu, upgradeEcu, DTRUE);
            dRet = DTRUE;
        }
        else
        {
            /*null*/
        }
    }
    return dRet;
}
static DBOOL ipsEcuProgressSetProgress(ADM_IPS_ECU_PROGRESS_T *p, const ADM_IPS_ECU_CURRENT_PROGRESS_T *progress, ADM_IPS_PROGRESS_T *totalp)
{
    DBOOL dRet = DFALSE;
    if (p->state == ADM_IPS_ECU_PHASE_INSTALL)
    {
        if (p->progress[ADM_IPS_ECU_PHASE_INDEX_UPGRADE].progress != (D32U)progress->progress)
        {
            if (p->progress[ADM_IPS_ECU_PHASE_INDEX_UPGRADE].progress > (D32U)progress->progress)
            {
                log_d("current progress is less than last progress, ignore[%d, %d]", p->progress[ADM_IPS_ECU_PHASE_INDEX_UPGRADE].progress, 
                                                                                     progress->progress);
            }
            else
            {
                p->progress[ADM_IPS_ECU_PHASE_INDEX_UPGRADE].progress = (D32U)progress->progress;
            }

            ipsProgressUpdateProgress(totalp);
            log_d("%s:%s, %s:%s, %s:%u",
                  ADM_STR_FROM_PARAM(p->ecuReqId), ADM_STR_SAFE_OUTPUT(p->ecuReqId),
                  ADM_STR_FROM_PARAM(p->ecuSwDid), ADM_STR_SAFE_OUTPUT(p->ecuSwDid),
                  ADM_STR_FROM_PARAM(p->progress[ADM_IPS_ECU_PHASE_INDEX_UPGRADE].progress), p->progress[ADM_IPS_ECU_PHASE_INDEX_UPGRADE].progress);
            dRet = DTRUE;
        }
    }
    else if (p->state == ADM_IPS_ECU_PHASE_INSTALL_ROLLBACK)
    {
        if (p->progress[ADM_IPS_ECU_PHASE_INDEX_ROLLBACK].progress != (D32U)progress->progress)
        {
            if (p->progress[ADM_IPS_ECU_PHASE_INDEX_ROLLBACK].progress > (D32U)progress->progress)
            {
                log_d("current progress is less than last progress, ignore[%d, %d]", p->progress[ADM_IPS_ECU_PHASE_INDEX_ROLLBACK].progress, 
                                                                                     progress->progress);
            }
            else
            {
                p->progress[ADM_IPS_ECU_PHASE_INDEX_ROLLBACK].progress = (D32U)progress->progress;
            }

            ipsProgressUpdateProgress(totalp);
            log_d("%s:%s, %s:%s, %s:%u",
                  ADM_STR_FROM_PARAM(p->ecuReqId), ADM_STR_SAFE_OUTPUT(p->ecuReqId),
                  ADM_STR_FROM_PARAM(p->ecuSwDid), ADM_STR_SAFE_OUTPUT(p->ecuSwDid),
                  ADM_STR_FROM_PARAM(p->progress[ADM_IPS_ECU_PHASE_INDEX_ROLLBACK].progress), p->progress[ADM_IPS_ECU_PHASE_INDEX_ROLLBACK].progress);
            dRet = DTRUE;
        }
    }
    else
    {
        /*null*/
    }
    return dRet;
}
static void ipsEcuProgressSetResultActionForFailed(ADM_IPS_ECU_PROGRESS_T *p, const ADM_IPS_POLICY_T *policy, ADM_IPS_PROGRESS_T *totalp){
    if(p->ecuReqId != NULL && p->ecuSwDid != NULL){
        const ADM_IPS_POLICY_ECU_GROUP_ORDER_T *ecuGroupOrder = ipsPolicyFindGroupEcu(policy, p->ecuReqId, p->ecuSwDid);
        if(ecuGroupOrder != NULL){
	        if(ecuGroupOrder->actionForFailed == 1){
	            p->state = ADM_IPS_ECU_PHASE_TIMELY_PRE_INSTALL_ROLLBACK;
	        }
	        if(ecuGroupOrder->groupEcus.ecus != NULL){
                for (D32S i = 0; i < ecuGroupOrder->groupEcus.ecusLen; ++i) {
                    const ADM_IPS_POLICY_UPGRADE_ECU_T* ecu = &ecuGroupOrder->groupEcus.ecus[i];
                    if (ecu->ecuReqId != NULL && ecu->ecuSwDid != NULL) {
                        ADM_IPS_ECU_PROGRESS_T* tempP = ipsProgressFindEcu(totalp, ecu->ecuReqId, ecu->ecuSwDid);
                        if (tempP != NULL) {
                            if (ecuGroupOrder->actionForFailed == 1) {
                                ipsEcuProgressNeedRollback(tempP);
                            }
                            else if (ecuGroupOrder->actionForFailed == 2)
                            {
                                if (FALSE == adm_cfg_iniParserGetBoolean(ADM_CFG_VUS_IS_DETACH_RETRY, 
                                                                         ADM_CFG_VUS_IS_DETACH_RETRY_DEFAULT_VAL))
                                {
                                    ipsEcuProgressNeedStopUpgrade(tempP);
                                }
                            }
                            else if (ecuGroupOrder->actionForFailed == 3) {}
                            else{
                                /*null*/
                            }
                        }
                    }
                }
	        }
        }
    }
}
static void ipsEcuProgressSetResult(ADM_IPS_ECU_PROGRESS_T *p, const ADM_IPS_ECU_CURRENT_PROGRESS_T *progress, const ADM_IPS_POLICY_T *policy, ADM_IPS_PROGRESS_T *totalp){
    if(p->state == ADM_IPS_ECU_PHASE_INSTALL){
        p->progress[ADM_IPS_ECU_PHASE_INDEX_UPGRADE].result = progress->result;
        p->progress[ADM_IPS_ECU_PHASE_INDEX_UPGRADE].upgradeNumbers++;
        p->progress[ADM_IPS_ECU_PHASE_INDEX_UPGRADE].endTime = time(NULL);
        adm_snprintf(p->progress[ADM_IPS_ECU_PHASE_INDEX_UPGRADE].resultDescrip,RESULT_DESCRIP_LEN,"%s",progress->resultDescrip);
        log_d("%s:%s, %s:%s, %s:%d, %s:%d",
            ADM_STR_FROM_PARAM(p->ecuReqId), ADM_STR_SAFE_OUTPUT(p->ecuReqId),
            ADM_STR_FROM_PARAM(p->ecuSwDid), ADM_STR_SAFE_OUTPUT(p->ecuSwDid),
            ADM_STR_FROM_PARAM(p->progress[ADM_IPS_ECU_PHASE_INDEX_UPGRADE].upgradeNumbers), p->progress[ADM_IPS_ECU_PHASE_INDEX_UPGRADE].upgradeNumbers,
            ADM_STR_FROM_PARAM(p->progress[ADM_IPS_ECU_PHASE_INDEX_UPGRADE].result), p->progress[ADM_IPS_ECU_PHASE_INDEX_UPGRADE].result);
        if (progress->result == ADM_ECODE_SUCCESSED) {
            p->state = ADM_IPS_ECU_PHASE_POST_INSTALL;
        }
        else
        {
            if ((policy->upgradeRetryCount + 1) > p->progress[ADM_IPS_ECU_PHASE_INDEX_UPGRADE].upgradeNumbers)
            {
                if (TRUE == adm_cfg_iniParserGetBoolean(ADM_CFG_VUS_IS_DETACH_RETRY, 
                                         ADM_CFG_VUS_IS_DETACH_RETRY_DEFAULT_VAL))
                {
                    p->state = ADM_IPS_ECU_PHASE_INSTALL_RETRY;
                }
            }
            else
            {
                p->state = ADM_IPS_ECU_PHASE_POST_INSTALL;
                if (FALSE == adm_cfg_iniParserGetBoolean(ADM_CFG_VUS_IS_DETACH_RETRY, 
                                                         ADM_CFG_VUS_IS_DETACH_RETRY_DEFAULT_VAL))
                {
                    ipsEcuProgressSetResultActionForFailed(p, policy, totalp);
                }
            }
        }
    }
    else if (p->state == ADM_IPS_ECU_PHASE_INSTALL_ROLLBACK) {
        p->progress[ADM_IPS_ECU_PHASE_INDEX_ROLLBACK].result = progress->result;
        p->progress[ADM_IPS_ECU_PHASE_INDEX_ROLLBACK].upgradeNumbers++;
        p->progress[ADM_IPS_ECU_PHASE_INDEX_ROLLBACK].endTime = time(NULL);
        adm_snprintf(p->progress[ADM_IPS_ECU_PHASE_INDEX_ROLLBACK].resultDescrip,RESULT_DESCRIP_LEN,"%s",progress->resultDescrip);
        log_d("%s:%s, %s:%s, %s:%d, %s:%d",
            ADM_STR_FROM_PARAM(p->ecuReqId), ADM_STR_SAFE_OUTPUT(p->ecuReqId),
            ADM_STR_FROM_PARAM(p->ecuSwDid), ADM_STR_SAFE_OUTPUT(p->ecuSwDid),
            ADM_STR_FROM_PARAM(p->progress[ADM_IPS_ECU_PHASE_INDEX_ROLLBACK].upgradeNumbers), p->progress[ADM_IPS_ECU_PHASE_INDEX_ROLLBACK].upgradeNumbers,
            ADM_STR_FROM_PARAM(p->progress[ADM_IPS_ECU_PHASE_INDEX_ROLLBACK].result), p->progress[ADM_IPS_ECU_PHASE_INDEX_ROLLBACK].result);
        if (progress->result == ADM_ECODE_SUCCESSED) {
            p->state = ADM_IPS_ECU_PHASE_POST_INSTALL_ROLLBACK;
        }
        else {
            if ((policy->rollbackRetryCount + 1) > p->progress[ADM_IPS_ECU_PHASE_INDEX_ROLLBACK].upgradeNumbers)
            {
                if (TRUE == adm_cfg_iniParserGetBoolean(ADM_CFG_VUS_IS_DETACH_RETRY, 
                                                        ADM_CFG_VUS_IS_DETACH_RETRY_DEFAULT_VAL))
                {
                    p->state = ADM_IPS_ECU_PHASE_INSTALL_ROLLBACK_RETRY;
                }
            }
            else
            {
                p->state = ADM_IPS_ECU_PHASE_POST_INSTALL_ROLLBACK;
            }
        }
    }
    else
    {
        /*null*/
    }
}
/*! group action for faild rollback */
static void ipsEcuProgressNeedRollback(ADM_IPS_ECU_PROGRESS_T *p){
    if(p->state == ADM_IPS_ECU_PHASE_PRE_INSTALL){
	    p->state = ADM_IPS_ECU_PHASE_POST_INSTALL_JUMP;
    }else if(p->state == ADM_IPS_ECU_PHASE_POST_INSTALL){
	    p->state = ADM_IPS_ECU_PHASE_PRE_INSTALL_ROLLBACK;
    }
    else
    {
        /*null*/
    }
}
/*! group action for faild stop */
static void ipsEcuProgressNeedStopUpgrade(ADM_IPS_ECU_PROGRESS_T *p){
    if(p->state == ADM_IPS_ECU_PHASE_PRE_INSTALL){
	    p->state = ADM_IPS_ECU_PHASE_POST_INSTALL_JUMP;
    }
}

/* ADM_IPS_POLICY_T */
static void ipsPolicyInit(ADM_IPS_POLICY_T *p){
    p->verifyVersion = 0;
    p->upgradeAutoStart = 0;
    p->upgradeRetryCount = 0;
    p->rollbackRetryCount = 0;
    ipsPolicyUpgradeConditionArrayInit(&p->upgradeConditions);
    p->totalTimeConsuming = 0;
    ipsPolicyDependencyArrayInit(&p->dependencies);
    ipsPolicyEcuGroupOrderArrayInit(&p->ecuGroupOrders);
}
static void ipsPolicyDeInit(ADM_IPS_POLICY_T *p){
    ipsPolicyUpgradeConditionArrayDeInit(&p->upgradeConditions);
    ipsPolicyDependencyArrayDeInit(&p->dependencies);
    ipsPolicyEcuGroupOrderArrayDeInit(&p->ecuGroupOrders);
}
static void ipsPolicyClear(ADM_IPS_POLICY_T *p){
    ipsPolicyDeInit(p);
    ipsPolicyInit(p);
}
static void ipsPolicyJsonParse(ADM_IPS_POLICY_T *p, const struct json_object *root){
    struct json_object* verifyVersionVal = json_object_object_get(root, "verifyVersion");
    if(NULL !=  verifyVersionVal){
	    p->verifyVersion = json_object_get_int(verifyVersionVal);
    }
    struct json_object* upgradeAutoStartVal = json_object_object_get(root, "upgradeAutoStart");
    if(NULL != upgradeAutoStartVal){
	    p->upgradeAutoStart = json_object_get_int64(upgradeAutoStartVal);
    }
    p->upgradeRetryCount = json_object_get_int(json_object_object_get(root, "upgradeRetryCount"));
    p->rollbackRetryCount = json_object_get_int(json_object_object_get(root, "rollbackRetryCount"));
    struct json_object* upgradeConditionsVal = json_object_object_get(root, "upgradeConditions");;
    if(NULL != upgradeConditionsVal){
	    ipsPolicyUpgradeConditionArrayJsonParse(&p->upgradeConditions, upgradeConditionsVal);
    }
    p->totalTimeConsuming = json_object_get_int(json_object_object_get(root, "totalTimeConsuming"));
    struct json_object* dependenciesVal = json_object_object_get(root, "dependencies");
    if(NULL != dependenciesVal){
	    ipsPolicyDependencyArrayJsonParse(&p->dependencies, dependenciesVal);
    }
    p->parallelNumLimitM = json_object_get_int(json_object_object_get(root, "parallelNumLimitM"));
    p->parallelNumLimitN = json_object_get_int(json_object_object_get(root, "parallelNumLimitN"));
    struct json_object* ecuGroupOrdersVal = json_object_object_get(root, "ecuGroupOrders");
    if(NULL != ecuGroupOrdersVal){
	    ipsPolicyEcuGroupOrderArrayJsonParse(&p->ecuGroupOrders, ecuGroupOrdersVal);
    }
}
static struct json_object* ipsPolicyJsonCreate(const ADM_IPS_POLICY_T *p){
    struct json_object* root = json_object_new_object();
    if(NULL != root){
        struct json_object* verifyVersionVal = json_object_new_int(p->verifyVersion);
        if (NULL != verifyVersionVal) {
            (void)json_object_object_add(root, "verifyVersion", verifyVersionVal);
        }
        struct json_object* upgradeAutoStartVal = json_object_new_int64(p->upgradeAutoStart);
        if (NULL != upgradeAutoStartVal) {
            (void)json_object_object_add(root, "upgradeAutoStart", upgradeAutoStartVal);
        }
        (void)json_object_object_add(root, "upgradeRetryCount", json_object_new_int(p->upgradeRetryCount));
        (void)json_object_object_add(root, "rollbackRetryCount", json_object_new_int(p->rollbackRetryCount));
        struct json_object* upgradeConditionsVal = ipsPolicyUpgradeConditionArrayJsonCreate(&p->upgradeConditions);
        if (NULL != upgradeConditionsVal) {
            (void)json_object_object_add(root, "upgradeConditions", upgradeConditionsVal);
        }
        (void)json_object_object_add(root, "totalTimeConsuming", json_object_new_int(p->totalTimeConsuming));
        struct json_object* dependenciesVal = ipsPolicyDependencyArrayJsonCreate(&p->dependencies);
        if (NULL != dependenciesVal) {
            (void)json_object_object_add(root, "dependencies", dependenciesVal);
        }
        (void)json_object_object_add(root, "parallelNumLimitM", json_object_new_int(p->parallelNumLimitM));
        (void)json_object_object_add(root, "parallelNumLimitN", json_object_new_int(p->parallelNumLimitN));
        struct json_object* ecuGroupOrdersVal = ipsPolicyEcuGroupOrderArrayJsonCreate(&p->ecuGroupOrders);
        if (NULL != ecuGroupOrdersVal) {
            (void)json_object_object_add(root, "ecuGroupOrders", ecuGroupOrdersVal);
        }
    }
    return root;
}
static D32S ipsPolicyGetMaxEcusCount(const ADM_IPS_POLICY_T *p){
    D32S count = 0;
    const ADM_IPS_POLICY_ECU_GROUP_ORDER_ARRAY_T *ecuGroupOrders = &p->ecuGroupOrders;
    if(ecuGroupOrders->ecuGroupOrders != NULL){
        for (D32S i = 0; i < ecuGroupOrders->ecuGroupOrdersLen; ++i) {
            const ADM_IPS_POLICY_UPGRADE_ECU_ARRAY_T* groupEcus = &ecuGroupOrders->ecuGroupOrders[i].groupEcus;
            if (groupEcus->ecus != NULL) {
                for (D32S j = 0; j < groupEcus->ecusLen; ++j) {
                    ++count;
                }
            }
        }
    }
    return count;
}
static const ADM_IPS_POLICY_UPGRADE_ECU_T *ipsPolicyFindUpgradeEcu(const ADM_IPS_POLICY_T *p, const ADM_IPS_ECU_INDEX_T *index){
    const ADM_IPS_POLICY_UPGRADE_ECU_T *upgradeEcu = NULL;
    const ADM_IPS_POLICY_ECU_GROUP_ORDER_ARRAY_T *ecuGroupOrders = &p->ecuGroupOrders;
    if(index->groupIndex < ecuGroupOrders->ecuGroupOrdersLen){
        const ADM_IPS_POLICY_UPGRADE_ECU_ARRAY_T* groupEcus = &ecuGroupOrders->ecuGroupOrders[index->groupIndex].groupEcus;
        if (index->memberIndex < groupEcus->ecusLen) {
            upgradeEcu = &groupEcus->ecus[index->memberIndex];
        }
    }
    return upgradeEcu;
}
static const ADM_IPS_POLICY_ECU_GROUP_ORDER_T *ipsPolicyFindGroupEcu(const ADM_IPS_POLICY_T *p, const DCHAR *ecuReqId, const DCHAR *ecuSwDid){
    const ADM_IPS_POLICY_ECU_GROUP_ORDER_T *resEcuGroup = NULL;
    if(p->ecuGroupOrders.ecuGroupOrders != NULL){
        for (D32S i = 0; i < p->ecuGroupOrders.ecuGroupOrdersLen; ++i) {
            const ADM_IPS_POLICY_UPGRADE_ECU_ARRAY_T* groupEcus = &p->ecuGroupOrders.ecuGroupOrders[i].groupEcus;
            if (groupEcus->ecus != NULL) {
                for (D32S j = 0; j < groupEcus->ecusLen; ++j) {
                    if (groupEcus->ecus[j].ecuReqId != NULL && groupEcus->ecus[j].ecuSwDid != NULL) {
                        if (adm_strcmp(groupEcus->ecus[j].ecuReqId, ecuReqId) == 0
                            && adm_strcmp(groupEcus->ecus[j].ecuSwDid, ecuSwDid) == 0) {
                            resEcuGroup = &p->ecuGroupOrders.ecuGroupOrders[i];
                            break;
                        }
                    }
                }
            }
            if (resEcuGroup != NULL) {
                break;
            }
        }
    }
    return resEcuGroup;
}

/* ADM_IPS_POLICY_ECU_GROUP_ORDER_ARRAY_T */
static void ipsPolicyEcuGroupOrderArrayInit(ADM_IPS_POLICY_ECU_GROUP_ORDER_ARRAY_T *p){
    p->ecuGroupOrders = NULL;
    p->ecuGroupOrdersLen = 0;
}
static void ipsPolicyEcuGroupOrderArrayDeInit(ADM_IPS_POLICY_ECU_GROUP_ORDER_ARRAY_T *p){
    if(NULL != p->ecuGroupOrders){
        for (D32S i = 0; i < p->ecuGroupOrdersLen; ++i) {
            ipsPolicyEcuGroupOrderDeInit(p->ecuGroupOrders + i);
        }
        adm_free(p->ecuGroupOrders);
    }
}
static void ipsPolicyEcuGroupOrderArrayJsonParse(ADM_IPS_POLICY_ECU_GROUP_ORDER_ARRAY_T *p, const struct json_object* root){
    if(json_object_get_type(root) == json_type_array){
	D32S ecuGroupOrdersLen = (D32S)json_object_array_length(root);
	if(ecuGroupOrdersLen > 0){
	    p->ecuGroupOrders = adm_malloc((DSIZE)ecuGroupOrdersLen * sizeof(*(p->ecuGroupOrders)));
	    if(NULL != p->ecuGroupOrders){
            p->ecuGroupOrdersLen = ecuGroupOrdersLen;
            for (D32S i = 0; i < p->ecuGroupOrdersLen; ++i) {
                ipsPolicyEcuGroupOrderInit(p->ecuGroupOrders + i);
                struct json_object* ecuGroupOrderVal = json_object_array_get_idx(root, (size_t)i);
                if (NULL != ecuGroupOrderVal) {
                    ipsPolicyEcuGroupOrderJsonParse(&p->ecuGroupOrders[i], ecuGroupOrderVal);
                }
            }
	    }
	}
    }
}
static struct json_object* ipsPolicyEcuGroupOrderArrayJsonCreate(const ADM_IPS_POLICY_ECU_GROUP_ORDER_ARRAY_T *p){
    struct json_object* root = json_object_new_array();
    if(NULL != root){
        if (NULL != p->ecuGroupOrders) {
            for (D32S i = 0; i < p->ecuGroupOrdersLen; ++i) {
                struct json_object* ecuGroupOrderVal = ipsPolicyEcuGroupOrderJsonCreate(&p->ecuGroupOrders[i]);
                if (NULL != ecuGroupOrderVal) {
                    (void)json_object_array_add(root, ecuGroupOrderVal);
                }
            }
        }
    }
    return root;
}
static D32S ipsPolicyEcuGroupOrderArrayGetTotalEcuTimeConsuming(const ADM_IPS_POLICY_ECU_GROUP_ORDER_ARRAY_T *p){
    D32S totalEcuTimeConsuming = 0;
    if(p->ecuGroupOrders != NULL){
        for (D32S i = 0; i < p->ecuGroupOrdersLen; ++i) {
            const ADM_IPS_POLICY_UPGRADE_ECU_ARRAY_T* groupEcus = &p->ecuGroupOrders[i].groupEcus;
            if (groupEcus->ecus != NULL) {
                for (D32S j = 0; j < groupEcus->ecusLen; ++j) {
                    totalEcuTimeConsuming += groupEcus->ecus[j].ecuTimeConsuming;
                }
            }
        }
    }
    return totalEcuTimeConsuming;
}

/* ADM_IPS_POLICY_ECU_GROUP_ORDER_T */
static void ipsPolicyEcuGroupOrderInit(ADM_IPS_POLICY_ECU_GROUP_ORDER_T *policy){
    policy->groupFlag = 0;
    policy->actionForFailed = 0;
    ipsPolicyUpgradeEcuArrayInit(&policy->groupEcus);
}
static void ipsPolicyEcuGroupOrderDeInit(ADM_IPS_POLICY_ECU_GROUP_ORDER_T *policy){
    ipsPolicyUpgradeEcuArrayDeInit(&policy->groupEcus);
}
static void ipsPolicyEcuGroupOrderJsonParse(ADM_IPS_POLICY_ECU_GROUP_ORDER_T *policy, const struct json_object* root){
    policy->groupFlag = json_object_get_int(json_object_object_get(root, "groupFlag"));
    policy->actionForFailed = json_object_get_int(json_object_object_get(root, "actionForFailed"));
    struct json_object* groupEcusVal = json_object_object_get(root, "groupEcus");
    if(NULL != groupEcusVal){
        ipsPolicyUpgradeEcuArrayJsonParse(&policy->groupEcus, groupEcusVal);
    }
}
static struct json_object* ipsPolicyEcuGroupOrderJsonCreate(const ADM_IPS_POLICY_ECU_GROUP_ORDER_T *policy){
    struct json_object* root = json_object_new_object();
    if(NULL != root){
        (void)json_object_object_add(root, "groupFlag", json_object_new_int(policy->groupFlag));
        (void)json_object_object_add(root, "actionForFailed", json_object_new_int(policy->actionForFailed));
        struct json_object* groupEcusVal = ipsPolicyUpgradeEcuArrayJsonCreate(&policy->groupEcus);
        if (NULL != groupEcusVal) {
            (void)json_object_object_add(root, "groupEcus", groupEcusVal);;
        }
    }
    return root;
}

/* ADM_IPS_POLICY_UPGRADE_ECU_ARRAY_T */
static void ipsPolicyUpgradeEcuArrayInit(ADM_IPS_POLICY_UPGRADE_ECU_ARRAY_T *policy){
    policy->ecus = NULL;
    policy->ecusLen = 0;
}
static void ipsPolicyUpgradeEcuArrayDeInit(ADM_IPS_POLICY_UPGRADE_ECU_ARRAY_T *policy){
    if(NULL != policy->ecus){
        for (D32S i = 0; i < policy->ecusLen; ++i) {
            ipsPolicyUpgradeEcuDeInit(&policy->ecus[i]);
        }
        adm_free(policy->ecus);
    }
}
static void ipsPolicyUpgradeEcuArrayJsonParse(ADM_IPS_POLICY_UPGRADE_ECU_ARRAY_T *policy, const struct json_object* root){
    if(json_object_get_type(root) == json_type_array){
        D32S ecusLen = (D32S)json_object_array_length(root);
        if (ecusLen > 0) {
            policy->ecus = adm_malloc((DSIZE)ecusLen * sizeof(*(policy->ecus)));
            if (NULL != policy->ecus) {
                policy->ecusLen = ecusLen;
                for (D32S i = 0; i < policy->ecusLen; ++i) {
                    ipsPolicyUpgradeEcuInit(&policy->ecus[i]);
                    struct json_object* ecuVal = json_object_array_get_idx(root, (size_t)i);
                    if (NULL != ecuVal) {
                        ipsPolicyUpgradeEcuJsonParse(&policy->ecus[i], ecuVal);
                    }
                }
            }
        }
    }
}
static struct json_object* ipsPolicyUpgradeEcuArrayJsonCreate(const ADM_IPS_POLICY_UPGRADE_ECU_ARRAY_T *policy){
    struct json_object* root = json_object_new_array();
    if(NULL != root){
        if (NULL != policy->ecus) {
            for (D32S i = 0; i < policy->ecusLen; ++i) {
                struct json_object* ecuVal = ipsPolicyUpgradeEcuJsonCreate(&policy->ecus[i]);
                if (NULL != ecuVal) {
                    (void)json_object_array_add(root, ecuVal);
                }
            }
        }
    }
    return root;
}

/* ADM_IPS_POLICY_UPGRADE_ECU_T */
static void ipsPolicyUpgradeEcuInit(ADM_IPS_POLICY_UPGRADE_ECU_T *policy){
    policy->ecuReqId = NULL;
    policy->ecuSwDid = NULL;
#ifdef ADM_IPS_POLICY_UPGRADE_ORDER_OLD
    policy->upgradeOrder = 0;
#else
    policy->upgradeOrder = NULL;
#endif//ADM_IPS_POLICY_UPGRADE_ORDER_OLD
    policy->activeOrder = -1;
    policy->ecuTimeConsuming = 0;
    policy->ecuTimeout = 0;
}
static void ipsPolicyUpgradeEcuDeInit(ADM_IPS_POLICY_UPGRADE_ECU_T *policy){
    if(NULL != policy->ecuReqId){adm_free(policy->ecuReqId);}
    if(NULL != policy->ecuSwDid){adm_free(policy->ecuSwDid);}
    adm_free(policy->upgradeOrder);
}
static void ipsPolicyUpgradeEcuJsonParse(ADM_IPS_POLICY_UPGRADE_ECU_T *policy, const struct json_object* root){
    const DCHAR *tempStr;
    tempStr = json_object_get_string(json_object_object_get(root, "ecuReqId"));
    if(NULL != tempStr)
    {
        policy->ecuReqId = adm_strdup(tempStr);
    }
    tempStr = json_object_get_string(json_object_object_get(root, "ecuSwDid"));
    if(NULL != tempStr)
    {
        policy->ecuSwDid = adm_strdup(tempStr);
    }
#ifdef ADM_IPS_POLICY_UPGRADE_ORDER_OLD
    policy->upgradeOrder = json_object_get_int(json_object_object_get(root, "upgradeOrder"));
#else
    struct json_object* upgradeOrderVal = json_object_object_get(root, "upgradeOrder");
    if(upgradeOrderVal != NULL)
    {
	    const DCHAR *upgradeOrder = json_object_get_string(upgradeOrderVal);
        if (upgradeOrder != NULL)
        {
            policy->upgradeOrder = adm_strdup(upgradeOrder);
        }
    }
#endif//ADM_IPS_POLICY_UPGRADE_ORDER_OLD

    struct json_object* activeOrderVal = json_object_object_get(root, "activeOrder");
    if (NULL != activeOrderVal)
    {
        policy->activeOrder = json_object_get_int(activeOrderVal);
    }
    policy->ecuTimeConsuming = json_object_get_int(json_object_object_get(root, "ecuTimeConsuming"));
    policy->ecuTimeout = json_object_get_int(json_object_object_get(root, "ecuTimeout"));
}
static struct json_object* ipsPolicyUpgradeEcuJsonCreate(const ADM_IPS_POLICY_UPGRADE_ECU_T *policy){
    struct json_object* root = json_object_new_object();
    if(NULL != root)
    {
	    if(NULL != policy->ecuReqId)
        {
            (void)json_object_object_add(root, "ecuReqId", json_object_new_string(policy->ecuReqId));
        }
	    if(NULL != policy->ecuSwDid)
        {
            (void)json_object_object_add(root, "ecuSwDid", json_object_new_string(policy->ecuSwDid));
        }
#ifdef ADM_IPS_POLICY_UPGRADE_ORDER_OLD
	    (void)json_object_object_add(root, "upgradeOrder", json_object_new_int(policy->upgradeOrder));
#else
	    if(policy->upgradeOrder != NULL)
        {
	        struct json_object* upgradeOrderVal = json_object_new_string(policy->upgradeOrder);
	        if(upgradeOrderVal != NULL){
		    (void)json_object_object_add(root, "upgradeOrder", upgradeOrderVal);
	        }
	    }
#endif//ADM_IPS_POLICY_UPGRADE_ORDER_OLD

        if (-1 != policy->activeOrder)
        {
            (void)json_object_object_add(root, "activeOrder", json_object_new_int(policy->activeOrder));
        }
	    (void)json_object_object_add(root, "ecuTimeConsuming", json_object_new_int(policy->ecuTimeConsuming));
	    (void)json_object_object_add(root, "ecuTimeout", json_object_new_int(policy->ecuTimeout));
    }
    return root;
}
#ifdef ADM_IPS_POLICY_UPGRADE_ORDER_OLD
static D32S ipsPolicyUpgradeEcuGetUpgradeId(D32S upgradeOrder) {
    return upgradeOrder;
    //TODO
}
static D32S ipsPolicyUpgradeEcuGetNetId(D32S upgradeOrder) {
    return 0;
    //TODO
}
static D32S ipsPolicyUpgradeEcuGetSequenceId(D32S upgradeOrder) {
    return 0;
    //TODO
}
#else
static D32S ipsPolicyUpgradeEcuGetUpgradeId(const DCHAR* upgradeOrder) {
    D32S upgradeId = ADM_IPS_ECU_UPGRADE_ORDER_MEMBER_DEFAULT_ID;
    if (upgradeOrder != NULL) {
        D32S order = htoi(upgradeOrder);
        upgradeId = (order >> 16) & 0XFF;
    }
    return upgradeId;
}
static D32S ipsPolicyUpgradeEcuGetNetId(const DCHAR* upgradeOrder) {
    D32S netId = ADM_IPS_ECU_UPGRADE_ORDER_MEMBER_DEFAULT_ID;
    if (upgradeOrder != NULL) {
        D32S order = htoi(upgradeOrder);
        netId = (order >> 8) & 0XFF;
    }
    return netId;
}
static D32S ipsPolicyUpgradeEcuGetSequenceId(const DCHAR* upgradeOrder) {
    D32S sequenceId = ADM_IPS_ECU_UPGRADE_ORDER_MEMBER_DEFAULT_ID;
    if (upgradeOrder != NULL) {
        D32S order = htoi(upgradeOrder);
        sequenceId = order & 0XFF;
    }
    return sequenceId;
}
#endif//ADM_IPS_POLICY_UPGRADE_ORDER_OLD

/* ADM_IPS_POLICY_DEPENDENCY_ARRAY_T */
static void ipsPolicyDependencyArrayInit(ADM_IPS_POLICY_DEPENDENCY_ARRAY_T *policy){
    policy->dependencies = NULL;
    policy->dependenciesLen = 0;
}
static void ipsPolicyDependencyArrayDeInit(ADM_IPS_POLICY_DEPENDENCY_ARRAY_T *policy){
    if(NULL != policy->dependencies){
	for(D32S i=0; i<policy->dependenciesLen; ++i){
	    adm_ips_policyDependencyDeInit(policy->dependencies + i);
	}
	adm_free(policy->dependencies);
    }
}
static void ipsPolicyDependencyArrayClone(const ADM_IPS_POLICY_DEPENDENCY_ARRAY_T *src, ADM_IPS_POLICY_DEPENDENCY_ARRAY_T *dst){
    if(NULL != src->dependencies && src->dependenciesLen > 0){
        dst->dependencies = adm_malloc((DSIZE)src->dependenciesLen * sizeof(dst->dependencies[0]));
        if (NULL != dst->dependencies) {
            dst->dependenciesLen = src->dependenciesLen;
            for (D32S i = 0; i < dst->dependenciesLen; ++i) {
                adm_ips_policyDependencyInit(&dst->dependencies[i]);
                adm_ips_policyDependencyClone(&src->dependencies[i], &dst->dependencies[i]);
            }
        }
    }
}

static void ipsPolicyDependencyArrayGet(const ADM_IPS_POLICY_DEPENDENCY_ARRAY_T *src, const D32S type, ADM_IPS_POLICY_DEPENDENCY_ARRAY_T *dst){
    if(NULL != src->dependencies && src->dependenciesLen > 0){
        D32S len = 0;
        for (D32S i = 0; i < src->dependenciesLen; ++i) {
            if (type == src->dependencies[i].type) {
                len++;
            }
        }

        if (0 == len) {
            return;
        }

        dst->dependencies = adm_malloc((DSIZE)len * sizeof(*(dst->dependencies)));
        if (NULL != dst->dependencies) {
            D32S j = 0;
            for (D32S i = 0; i < src->dependenciesLen; ++i) {
                if (type == src->dependencies[i].type) {
                    adm_ips_policyDependencyInit(&dst->dependencies[j]);
                    adm_ips_policyDependencyClone(&src->dependencies[i], &dst->dependencies[j]);
                    ++j;
                }
            }

            dst->dependenciesLen = len;
        }
    }
}

static D32S ipsPolicyDependencyGet(const ADM_IPS_POLICY_DEPENDENCY_ARRAY_T* src,  
    const DCHAR* ecuReqId, const DCHAR* ecuSwDid, const D32S type,
    ADM_IPS_POLICY_DEPENDENCY_T* dst) {
    D32S res = ADM_ECODE_FAILED;
    const ADM_IPS_POLICY_DEPENDENCY_T* dependency = NULL;

    if (NULL != src->dependencies) {
        for (D32S i = 0; i < src->dependenciesLen; ++i) {
            dependency = &src->dependencies[i];
            if (type == dependency->type &&
                0 == adm_strcasecmp(ecuReqId, dependency->ecuReqId) &&
                0 == adm_strcasecmp(ecuSwDid, dependency->ecuSwDid)) {
                adm_ips_policyDependencyInit(dst);
                adm_ips_policyDependencyClone(dependency, dst);
                res = ADM_ECODE_SUCCESSED;
                break;
            }
        }
    }

    return res;
}

static void ipsPolicyDependencyArrayJsonParse(ADM_IPS_POLICY_DEPENDENCY_ARRAY_T *policy, const struct json_object* root){
    if(json_object_get_type(root) == json_type_array){
	D32S dependenciesLen = (D32S)json_object_array_length(root);
	if(dependenciesLen > 0){
	    policy->dependencies = adm_malloc((DSIZE)dependenciesLen * sizeof(*(policy->dependencies)));
	    if(NULL != policy->dependencies){
            policy->dependenciesLen = dependenciesLen;
            for (D32S i = 0; i < policy->dependenciesLen; ++i) {
                adm_ips_policyDependencyInit(&policy->dependencies[i]);
                struct json_object* dependencyVal = json_object_array_get_idx(root, (size_t)i);
                if (NULL != dependencyVal) {
                    ipsPolicyDependencyJsonParse(&policy->dependencies[i], dependencyVal);
                }
            }
	    }
	}
    }
}
static struct json_object* ipsPolicyDependencyArrayJsonCreate(const ADM_IPS_POLICY_DEPENDENCY_ARRAY_T *policy){
    struct json_object* root = json_object_new_array();
    if(NULL != root){
        if (NULL != policy->dependencies) {
            for (D32S i = 0; i < policy->dependenciesLen; ++i) {
                struct json_object* dependencyVal = ipsPolicyDependencyJsonCreate(&policy->dependencies[i]);
                if (NULL != dependencyVal) {
                    (void)json_object_array_add(root, dependencyVal);
                }
            }
        }
    }
    return root;
}

/* ADM_IPS_POLICY_DEPENDENCY_T */
void adm_ips_policyDependencyInit(ADM_IPS_POLICY_DEPENDENCY_T *policy){
    policy->type = 0;
    policy->ecuReqId = NULL;
    policy->ecuSwDid = NULL;
    ipsPolicyEcuDidArrayInit(&policy->ecuDids);
}
void adm_ips_policyDependencyDeInit(ADM_IPS_POLICY_DEPENDENCY_T *policy){
    if(NULL != policy->ecuReqId){adm_free(policy->ecuReqId);}
    if(NULL != policy->ecuSwDid){adm_free(policy->ecuSwDid);}
    ipsPolicyEcuDidArrayDeInit(&policy->ecuDids);
}
void adm_ips_policyDependencyClone(const ADM_IPS_POLICY_DEPENDENCY_T *src, ADM_IPS_POLICY_DEPENDENCY_T *dst){
    dst->type = src->type;
    if(NULL != src->ecuReqId){dst->ecuReqId = adm_strdup(src->ecuReqId);}
    if(NULL != src->ecuSwDid){dst->ecuSwDid = adm_strdup(src->ecuSwDid);}
    ipsPolicyEcuDidArrayClone(&src->ecuDids, &dst->ecuDids);
}
static void ipsPolicyDependencyJsonParse(ADM_IPS_POLICY_DEPENDENCY_T *policy, const struct json_object* root){
    const DCHAR *tempStr;
    policy->type = json_object_get_int(json_object_object_get(root, "type"));
    tempStr = json_object_get_string(json_object_object_get(root, "ecuReqId"));if(NULL != tempStr){policy->ecuReqId = adm_strdup(tempStr);}
    tempStr = json_object_get_string(json_object_object_get(root, "ecuSwDid"));if(NULL != tempStr){policy->ecuSwDid = adm_strdup(tempStr);}
    struct json_object* ecuDidsVal = json_object_object_get(root, "ecuDids");
    if(NULL != ecuDidsVal){
	    ipsPolicyEcuDidArrayJsonParse(&policy->ecuDids, ecuDidsVal);
    }
}
static struct json_object* ipsPolicyDependencyJsonCreate(const ADM_IPS_POLICY_DEPENDENCY_T *policy){
    struct json_object* root = json_object_new_object();
    if(NULL != root){
        (void)json_object_object_add(root, "type", json_object_new_int(policy->type));
        if (NULL != policy->ecuReqId) { (void)json_object_object_add(root, "ecuReqId", json_object_new_string(policy->ecuReqId)); }
        if (NULL != policy->ecuSwDid) { (void)json_object_object_add(root, "ecuSwDid", json_object_new_string(policy->ecuSwDid)); }
        struct json_object* ecuDidsVal = ipsPolicyEcuDidArrayJsonCreate(&policy->ecuDids);
        if (NULL != ecuDidsVal) {
            (void)json_object_object_add(root, "ecuDids", ecuDidsVal);
        }
    }
    return root;
}

/* ADM_IPS_POLICY_ECU_DID_ARRAY_T */
static void ipsPolicyEcuDidArrayInit(ADM_IPS_POLICY_ECU_DID_ARRAY_T *policy){
    policy->ecuDids = NULL;
    policy->ecuDidsLen = 0;
}
static void ipsPolicyEcuDidArrayDeInit(ADM_IPS_POLICY_ECU_DID_ARRAY_T *policy){
    if(NULL != policy->ecuDids){
        for (D32S i = 0; i < policy->ecuDidsLen; ++i) {
            ipsPolicyEcuDidDeInit(&policy->ecuDids[i]);
        }
        adm_free(policy->ecuDids);
    }
}
static void ipsPolicyEcuDidArrayClone(const ADM_IPS_POLICY_ECU_DID_ARRAY_T *src, ADM_IPS_POLICY_ECU_DID_ARRAY_T *dst){
    if(NULL != src->ecuDids && src->ecuDidsLen > 0){
        dst->ecuDids = adm_malloc((DSIZE)src->ecuDidsLen * sizeof(*(dst->ecuDids)));
        if (NULL != dst->ecuDids) {
            dst->ecuDidsLen = src->ecuDidsLen;
            for (D32S i = 0; i < dst->ecuDidsLen; ++i) {
                ipsPolicyEcuDidInit(&dst->ecuDids[i]);
                ipsPolicyEcuDidClone(&src->ecuDids[i], &dst->ecuDids[i]);
            }
        }
    }
}
static void ipsPolicyEcuDidArrayJsonParse(ADM_IPS_POLICY_ECU_DID_ARRAY_T *policy, const struct json_object* root){
    if(json_object_get_type(root) == json_type_array){
	D32S ecuDidsLen = (D32S)json_object_array_length(root);
	if(ecuDidsLen > 0){
	    policy->ecuDids = adm_malloc((DSIZE)ecuDidsLen * sizeof(*(policy->ecuDids)));
	    if(NULL != policy->ecuDids){
            policy->ecuDidsLen = ecuDidsLen;
            for (D32S i = 0; i < policy->ecuDidsLen; ++i) {
                ipsPolicyEcuDidInit(&policy->ecuDids[i]);
                struct json_object* ecuDidVal = json_object_array_get_idx(root, (size_t)i);
                if (NULL != ecuDidVal) {
                    ipsPolicyEcuDidJsonParse(&policy->ecuDids[i], ecuDidVal);
                }
            }
	    }
	}
    }
}
static struct json_object* ipsPolicyEcuDidArrayJsonCreate(const ADM_IPS_POLICY_ECU_DID_ARRAY_T *policy){
    struct json_object* root = json_object_new_array();
    if(NULL != root){
        if (NULL != policy->ecuDids) {
            for (D32S i = 0; i < policy->ecuDidsLen; ++i) {
                struct json_object* ecuDidVal = ipsPolicyEcuDidJsonCreate(&policy->ecuDids[i]);
                if (NULL != ecuDidVal) {
                    (void)json_object_array_add(root, ecuDidVal);
                }
            }
        }
    }
    return root;
}

/* ADM_IPS_POLICY_ECU_DID_T */
static void ipsPolicyEcuDidInit(ADM_IPS_POLICY_ECU_DID_T *policy){
    policy->did = NULL;
    policy->didValue = NULL;
}
static void ipsPolicyEcuDidDeInit(ADM_IPS_POLICY_ECU_DID_T *policy){
    if(NULL != policy->did){adm_free(policy->did);}
    if(NULL != policy->didValue){adm_free(policy->didValue);}
}
static void ipsPolicyEcuDidClone(const ADM_IPS_POLICY_ECU_DID_T *src, ADM_IPS_POLICY_ECU_DID_T *dst){
    if(NULL != src->did){dst->did = adm_strdup(src->did);}
    if(NULL != src->didValue){dst->didValue = adm_strdup(src->didValue);}
}
static void ipsPolicyEcuDidJsonParse(ADM_IPS_POLICY_ECU_DID_T *policy, const struct json_object* root){
    const DCHAR *tempStr;
    tempStr = json_object_get_string(json_object_object_get(root, "did"));if(NULL != tempStr){policy->did = adm_strdup(tempStr);}
    tempStr = json_object_get_string(json_object_object_get(root, "didValue"));if(NULL != tempStr){policy->didValue = adm_strdup(tempStr);}
}
static struct json_object* ipsPolicyEcuDidJsonCreate(const ADM_IPS_POLICY_ECU_DID_T *policy){
    struct json_object* root = json_object_new_object();
    if(NULL != root){
        if (NULL != policy->did) { (void)json_object_object_add(root, "did", json_object_new_string(policy->did)); }
        if (NULL != policy->didValue) { (void)json_object_object_add(root, "didValue", json_object_new_string(policy->didValue)); }
    }
    return root;
}

/* ADM_IPS_POLICY_UPGRADE_CONDITION_ARRAY_T */
static void ipsPolicyUpgradeConditionArrayInit(ADM_IPS_POLICY_UPGRADE_CONDITION_ARRAY_T *policy){
    policy->upgradeConditions = NULL;
    policy->upgradeConditionsLen = 0;
}
static void ipsPolicyUpgradeConditionArrayDeInit(ADM_IPS_POLICY_UPGRADE_CONDITION_ARRAY_T *policy){
    if(NULL != policy->upgradeConditions){
        for (D32S i = 0; i < policy->upgradeConditionsLen; ++i) {
            adm_condPolicyDeInit(&policy->upgradeConditions[i]);
        }
        adm_free(policy->upgradeConditions);
    }
}
static void ipsPolicyUpgradeConditionArrayJsonParse(ADM_IPS_POLICY_UPGRADE_CONDITION_ARRAY_T *policy, const struct json_object* root){
    if(json_object_get_type(root) == json_type_array){
        D32S upgradeConditionsLen = (D32S)json_object_array_length(root);
        if (upgradeConditionsLen > 0) {
            policy->upgradeConditions = adm_malloc((DSIZE)upgradeConditionsLen * sizeof(policy->upgradeConditions[0]));
            if (NULL != policy->upgradeConditions) {
                policy->upgradeConditionsLen = upgradeConditionsLen;
                for (D32S i = 0; i < policy->upgradeConditionsLen; ++i) {
                    adm_condPolicyInit(&policy->upgradeConditions[i]);
                    struct json_object* upgradeConditionVal = json_object_array_get_idx(root, (size_t)i);
                    if (NULL != upgradeConditionVal) {
                        adm_condPolicyJsonParse(&policy->upgradeConditions[i], upgradeConditionVal);
                    }
                }
            }
        }
    }
}
static struct json_object* ipsPolicyUpgradeConditionArrayJsonCreate(const ADM_IPS_POLICY_UPGRADE_CONDITION_ARRAY_T *policy){
    struct json_object* root = json_object_new_array();
    if(NULL != root){
        if (NULL != policy->upgradeConditions) {
            for (D32S i = 0; i < policy->upgradeConditionsLen; ++i) {
                struct json_object* upgradeConditionVal = adm_condPolicyJsonCreate(&policy->upgradeConditions[i]);
                if (NULL != upgradeConditionVal) {
                    (void)json_object_array_add(root, upgradeConditionVal);
                }
            }
        }
    }
    return root;
}

#if 0
/* ADM_IPS_POLICY_UPGRADE_CONDITION_T */
static void ipsPolicyUpgradeConditionInit(ADM_IPS_POLICY_UPGRADE_CONDITION_T *policy){
    policy->type = 0;
    ipsPolicyConditionArrayInit(&policy->conditions);
}
static void ipsPolicyUpgradeConditionDeInit(ADM_IPS_POLICY_UPGRADE_CONDITION_T *policy){
    ipsPolicyConditionArrayDeInit(&policy->conditions);
}
static void ipsPolicyUpgradeConditionClone(const ADM_IPS_POLICY_UPGRADE_CONDITION_T *src, ADM_IPS_POLICY_UPGRADE_CONDITION_T *dst){
    dst->type = src->type;
    ipsPolicyConditionArrayClone(&src->conditions, &dst->conditions);
}
static void ipsPolicyUpgradeConditionJsonParse(ADM_IPS_POLICY_UPGRADE_CONDITION_T *policy, const struct json_object* root){
    policy->type = json_object_get_int(json_object_object_get(root, "type"));
    struct json_object* conditionsVal = json_object_object_get(root, "conditions");
    if(NULL != conditionsVal){
	    ipsPolicyConditionArrayJsonParse(&policy->conditions, conditionsVal);
    }
}
static struct json_object* ipsPolicyUpgradeConditionJsonCreate(const ADM_IPS_POLICY_UPGRADE_CONDITION_T *policy){
    struct json_object* root = json_object_new_object();
    if(NULL != root){
        (void)json_object_object_add(root, "type", json_object_new_int(policy->type));
        struct json_object* conditionsVal = ipsPolicyConditionArrayJsonCreate(&policy->conditions);
        if (NULL != conditionsVal) {
            (void)json_object_object_add(root, "conditions", conditionsVal);
        }
    }
    return root;
}

/* ADM_IPS_POLICY_CONDITION_ARRAY_T */
static void ipsPolicyConditionArrayInit(ADM_IPS_POLICY_CONDITION_ARRAY_T *policy){
    policy->conditions = NULL;
    policy->conditionsLen = 0;
}
static void ipsPolicyConditionArrayDeInit(ADM_IPS_POLICY_CONDITION_ARRAY_T *policy){
    if(NULL != policy->conditions){
        for (D32S i = 0; i < policy->conditionsLen; ++i) {
            ipsPolicyConditionDeInit(policy->conditions + i);
        }
        adm_free(policy->conditions);
    }
}
static void ipsPolicyConditionArrayClone(const ADM_IPS_POLICY_CONDITION_ARRAY_T *src, ADM_IPS_POLICY_CONDITION_ARRAY_T *dst){
    if(NULL != src->conditions && src->conditionsLen > 0){
        dst->conditions = adm_malloc((DSIZE)src->conditionsLen * sizeof(*(dst->conditions)));
        if (NULL != dst->conditions) {
            dst->conditionsLen = src->conditionsLen;
            for (D32S i = 0; i < dst->conditionsLen; ++i) {
                ipsPolicyConditionInit(dst->conditions + i);
                ipsPolicyConditionClone(src->conditions + i, dst->conditions + i);
            }
        }
    }
}
static void ipsPolicyConditionArrayJsonParse(ADM_IPS_POLICY_CONDITION_ARRAY_T *policy, const struct json_object* root){
    if(json_object_get_type(root) == json_type_array){
        D32S conditionsLen = (D32S)json_object_array_length(root);
        if (conditionsLen > 0) {
            policy->conditions = adm_malloc((DSIZE)conditionsLen * sizeof(*(policy->conditions)));
            if (NULL != policy->conditions) {
                policy->conditionsLen = conditionsLen;
                for (D32S i = 0; i < policy->conditionsLen; ++i) {
                    ipsPolicyConditionInit(policy->conditions + i);
                    struct json_object* conditionVal = json_object_array_get_idx(root, (size_t)i);
                    if (NULL != conditionVal) {
                        ipsPolicyConditionJsonParse(policy->conditions + i, conditionVal);
                    }
                }
            }
        }
    }
}
static struct json_object* ipsPolicyConditionArrayJsonCreate(const ADM_IPS_POLICY_CONDITION_ARRAY_T *policy){
    struct json_object* root = json_object_new_array();
    if(NULL != root){
        if (NULL != policy->conditions) {
            for (D32S i = 0; i < policy->conditionsLen; ++i) {
                struct json_object* conditionVal = ipsPolicyConditionJsonCreate(policy->conditions + i);
                if (NULL != conditionVal) {
                    (void)json_object_array_add(root, conditionVal);
                }
            }
        }
    }
    return root;
}

/* ADM_IPS_POLICY_CONDITION_T */
static void ipsPolicyConditionInit(ADM_IPS_POLICY_CONDITION_T *policy){
    policy->messageId = NULL;
    policy->messageName = NULL;
    policy->messageDesc = NULL;
    ipsPolicyConditionExpressionInit(&policy->expression);
    ipsPolicyConditionOnFailStrategyInit(&policy->onFailStrategy);
}
static void ipsPolicyConditionDeInit(ADM_IPS_POLICY_CONDITION_T *policy){
    if(NULL != policy->messageId){adm_free(policy->messageId);}
    if(NULL != policy->messageName){adm_free(policy->messageName);}
    if(NULL != policy->messageDesc){adm_free(policy->messageDesc);}
    ipsPolicyConditionExpressionDeInit(&policy->expression);
    ipsPolicyConditionOnFailStrategyDeInit(&policy->onFailStrategy);
}
static void ipsPolicyConditionClone(const ADM_IPS_POLICY_CONDITION_T *src, ADM_IPS_POLICY_CONDITION_T *dst){
    if(NULL != src->messageId){dst->messageId = adm_strdup(src->messageId);}
    if(NULL != src->messageName){dst->messageName = adm_strdup(src->messageName);}
    if(NULL != src->messageDesc){dst->messageDesc = adm_strdup(src->messageDesc);}
    ipsPolicyConditionExpressionClone(&src->expression, &dst->expression);
    ipsPolicyConditionOnFailStrategyClone(&src->onFailStrategy, &dst->onFailStrategy);
}
static void ipsPolicyConditionJsonParse(ADM_IPS_POLICY_CONDITION_T *policy, const struct json_object* root){
    const DCHAR *tempStr;
    tempStr = json_object_get_string(json_object_object_get(root, "messageId"));if(NULL != tempStr){policy->messageId = adm_strdup(tempStr);}
    tempStr = json_object_get_string(json_object_object_get(root, "messageName"));if(NULL != tempStr){policy->messageName = adm_strdup(tempStr);}
    tempStr = json_object_get_string(json_object_object_get(root, "messageDesc"));if(NULL != tempStr){policy->messageDesc = adm_strdup(tempStr);}
    struct json_object* expressionVal = json_object_object_get(root, "expression");
    if(NULL != expressionVal){
	    ipsPolicyConditionExpressionJsonParse(&policy->expression, expressionVal);
    }
    struct json_object* onFailStrategyVal = json_object_object_get(root, "onFailStrategy");
    if(NULL != onFailStrategyVal){
	    ipsPolicyConditionOnFailStrategyJsonParse(&policy->onFailStrategy, onFailStrategyVal);
    }
}
static struct json_object* ipsPolicyConditionJsonCreate(const ADM_IPS_POLICY_CONDITION_T *policy){
    struct json_object* root = json_object_new_object();
    if(NULL != root){
        if (NULL != policy->messageId) { (void)json_object_object_add(root, "messageId", json_object_new_string(policy->messageId)); }
        if (NULL != policy->messageName) { (void)json_object_object_add(root, "messageName", json_object_new_string(policy->messageName)); }
        if (NULL != policy->messageDesc) { (void)json_object_object_add(root, "messageDesc", json_object_new_string(policy->messageDesc)); }
        struct json_object* expressionVal = ipsPolicyConditionExpressionJsonCreate(&policy->expression);
        if (NULL != expressionVal) {
            (void)json_object_object_add(root, "expression", expressionVal);
        }
        struct json_object* onFailStrategyVal = ipsPolicyConditionOnFailStrategyJsonCreate(&policy->onFailStrategy);
        if (NULL != onFailStrategyVal) {
            (void)json_object_object_add(root, "onFailStrategy", onFailStrategyVal);
        }
    }
    return root;
}

/* ADM_IPS_POLICY_CONDITION_ON_FAILsTRATEGY_T */
static void ipsPolicyConditionOnFailStrategyInit(ADM_IPS_POLICY_CONDITION_ON_FAILsTRATEGY_T *policy){
    policy->type = 0;
    policy->timeout = 0;
    policy->action = 0;
    ipsPolicyConditionExpressionInit(&policy->expression);
}
static void ipsPolicyConditionOnFailStrategyDeInit(ADM_IPS_POLICY_CONDITION_ON_FAILsTRATEGY_T *policy){
    ipsPolicyConditionExpressionDeInit(&policy->expression);
}
static void ipsPolicyConditionOnFailStrategyClone(const ADM_IPS_POLICY_CONDITION_ON_FAILsTRATEGY_T *src, ADM_IPS_POLICY_CONDITION_ON_FAILsTRATEGY_T *dst){
    dst->type = src->type;
    dst->timeout = src->timeout;
    dst->action = src->action;
    ipsPolicyConditionExpressionClone(&src->expression, &dst->expression);
}
static void ipsPolicyConditionOnFailStrategyJsonParse(ADM_IPS_POLICY_CONDITION_ON_FAILsTRATEGY_T *policy, const struct json_object* root){
    const DCHAR *tempStr;
    policy->type = json_object_get_int(json_object_object_get(root, "type"));
    policy->timeout = json_object_get_int(json_object_object_get(root, "timeout"));
    policy->action = json_object_get_int(json_object_object_get(root, "action"));
    struct json_object* expressionVal = json_object_object_get(root, "expression");
    if(NULL != expressionVal){
	    ipsPolicyConditionExpressionJsonParse(&policy->expression, expressionVal);
    }
}
static struct json_object* ipsPolicyConditionOnFailStrategyJsonCreate(const ADM_IPS_POLICY_CONDITION_ON_FAILsTRATEGY_T *policy){
    struct json_object* root = json_object_new_object();
    if(NULL != root){
        (void)json_object_object_add(root, "type", json_object_new_int(policy->type));
        (void)json_object_object_add(root, "timeout", json_object_new_int(policy->timeout));
        (void)json_object_object_add(root, "action", json_object_new_int(policy->action));
        struct json_object* expressionVal = ipsPolicyConditionExpressionJsonCreate(&policy->expression);
        if (NULL != expressionVal) {
            (void)json_object_object_add(root, "expression", expressionVal);
        }
    }
    return root;
}

/* ADM_IPS_POLICY_CONDITION_EXPRESSION_T */
static void ipsPolicyConditionExpressionInit(ADM_IPS_POLICY_CONDITION_EXPRESSION_T *policy){
    policy->logic = 0;
    policy->dataType = 0;
    policy->dataLength = 0;
    policy->data = NULL;
    policy->connectRelation = 0;
}
static void ipsPolicyConditionExpressionDeInit(ADM_IPS_POLICY_CONDITION_EXPRESSION_T *policy){
    if(NULL != policy->data){adm_free(policy->data);}
}
static void ipsPolicyConditionExpressionClone(const ADM_IPS_POLICY_CONDITION_EXPRESSION_T *src, ADM_IPS_POLICY_CONDITION_EXPRESSION_T *dst){
    dst->logic = src->logic;
    dst->dataType = src->dataType;
    dst->dataLength = src->dataLength;
    if(NULL != src->data){dst->data = adm_strdup(src->data);}
    dst->connectRelation = src->connectRelation;
}
static void ipsPolicyConditionExpressionJsonParse(ADM_IPS_POLICY_CONDITION_EXPRESSION_T *policy, const struct json_object* root){
    const DCHAR *tempStr;
    policy->logic = json_object_get_int(json_object_object_get(root, "logic"));
    policy->dataType = json_object_get_int(json_object_object_get(root, "dataType"));
    policy->dataLength = json_object_get_int(json_object_object_get(root, "dataLength"));
    tempStr = json_object_get_string(json_object_object_get(root, "data")); if(NULL != tempStr){policy->data = adm_strdup(tempStr);}
    policy->connectRelation = json_object_get_int(json_object_object_get(root, "connectRelation"));
}
static struct json_object* ipsPolicyConditionExpressionJsonCreate(const ADM_IPS_POLICY_CONDITION_EXPRESSION_T *policy){
    struct json_object* root = json_object_new_object();
    if(NULL != root){
        (void)json_object_object_add(root, "logic", json_object_new_int(policy->logic));
        (void)json_object_object_add(root, "dataType", json_object_new_int(policy->dataType));
        (void)json_object_object_add(root, "dataLength", json_object_new_int(policy->dataLength));
        if (NULL != policy->data) { (void)json_object_object_add(root, "data", json_object_new_string(policy->data)); }
        (void)json_object_object_add(root, "connectRelation", json_object_new_int(policy->connectRelation));
    }
    return root;
}
#endif

/* ADM_IPS_POLICY_TASK_T */
static void ipsPolicyTaskInit(ADM_IPS_POLICY_TASK_T *task){
    task->sessionId = NULL;
    task->taskId = 0;
    task->strategyId = 0;
    task->versionNumber = NULL;
    task->upgradeMode = 0;
    task->upgradeAutoStart = 0;
    task->totalTimeConsuming = 0;
    task->verifyVersion = 0;
}
static void ipsPolicyTaskDeInit(ADM_IPS_POLICY_TASK_T *task){
    if(NULL != task->sessionId){adm_free(task->sessionId);}
    if(NULL != task->versionNumber){adm_free(task->versionNumber);}
}
static void ipsPolicyTaskClone(const ADM_IPS_POLICY_TASK_T *src, ADM_IPS_POLICY_TASK_T *dst){
    if(NULL != src->sessionId){dst->sessionId = adm_strdup(src->sessionId);}
    dst->taskId = src->taskId;
    dst->strategyId = src->strategyId;
    if(NULL != src->versionNumber){dst->versionNumber = adm_strdup(src->versionNumber);}
    dst->upgradeMode = src->upgradeMode;
    dst->upgradeAutoStart = src->upgradeAutoStart;
    dst->totalTimeConsuming = src->totalTimeConsuming;
    dst->verifyVersion = src->verifyVersion;
}
static void ipsPolicyTaskClear(ADM_IPS_POLICY_TASK_T *task){
    ipsPolicyTaskDeInit(task);
    ipsPolicyTaskInit(task);
}
static void ipsPolicyTaskJsonParse(ADM_IPS_POLICY_TASK_T *task, const struct json_object* root){
    const DCHAR *tempStr;
    tempStr = json_object_get_string(json_object_object_get(root, "sessionId"));if(NULL != tempStr){task->sessionId = adm_strdup(tempStr);}
    task->taskId = json_object_get_int64(json_object_object_get(root, "taskId"));
    task->strategyId = json_object_get_int64(json_object_object_get(root, "strategyId"));
    tempStr = json_object_get_string(json_object_object_get(root, "versionNumber"));if(NULL != tempStr){task->versionNumber = adm_strdup(tempStr);}
    struct json_object* upgradeModeVal = json_object_object_get(root, "upgradeMode");
    if(NULL != upgradeModeVal){
	    task->upgradeMode = json_object_get_int(upgradeModeVal);
    }
    struct json_object* upgradeAutoStartVal = json_object_object_get(root, "upgradeAutoStart");
    if(NULL != upgradeAutoStartVal){
	    task->upgradeAutoStart = json_object_get_int64(upgradeAutoStartVal);
    }
    struct json_object* totalTimeConsumingVal = json_object_object_get(root, "totalTimeConsuming");
    if(NULL != totalTimeConsumingVal){
	    task->totalTimeConsuming = json_object_get_int(totalTimeConsumingVal);
    }
    struct json_object* verifyVersionVal = json_object_object_get(root, "verifyVersion");
    if(NULL != verifyVersionVal){
	    task->verifyVersion = json_object_get_int(verifyVersionVal);
    }
}
static struct json_object* ipsPolicyTaskJsonCreate(const ADM_IPS_POLICY_TASK_T *task){
    struct json_object* root = json_object_new_object();
    if(NULL != root){
	if(NULL != task->sessionId){(void)json_object_object_add(root, "sessionId", json_object_new_string(task->sessionId));}
	(void)json_object_object_add(root, "taskId", json_object_new_int64(task->taskId));
	(void)json_object_object_add(root, "strategyId", json_object_new_int64(task->strategyId));
	if(NULL != task->versionNumber){(void)json_object_object_add(root, "versionNumber", json_object_new_string(task->versionNumber));}
	struct json_object* upgradeModeVal = json_object_new_int(task->upgradeMode);
	if(NULL != upgradeModeVal){
	    (void)json_object_object_add(root, "upgradeMode", upgradeModeVal);
	}
	struct json_object* upgradeAutoStartVal = json_object_new_int64(task->upgradeAutoStart);
	if(NULL != upgradeAutoStartVal){
	    (void)json_object_object_add(root, "upgradeAutoStart", upgradeAutoStartVal);
	}
	struct json_object* totalTimeConsumingVal = json_object_new_int(task->totalTimeConsuming);
	if(NULL != totalTimeConsumingVal){
	    (void)json_object_object_add(root, "totalTimeConsuming", totalTimeConsumingVal);
	}
	struct json_object* verifyVersionVal = json_object_new_int(task->verifyVersion);
	if(NULL != verifyVersionVal){
	    (void)json_object_object_add(root, "verifyVersion", verifyVersionVal);
	}
    }
    return root;
}
#if 0
static void ipsPolicyTaskSet(ADM_IPS_POLICY_TASK_T *task, const DCHAR *sessionId, D64S taskId, D64S strategyId){
    if(NULL != sessionId){task->sessionId = adm_strdup(sessionId);}
    task->taskId = taskId;
    task->strategyId = strategyId;
}
#endif

/* ADM_IPS_INSTALL_ECUINFO_T */
static void ipsInstallEcuInfoInit(ADM_IPS_INSTALL_ECUINFO_T *e){
    e->ecuReqId = NULL;
    e->ecuSwDid = NULL;
    //e->srcVer = NULL;
    //e->dstVer = NULL;
    e->ecuTimeConsuming = 0;
    e->ecuTimeout = 0;
    e->isRollback = DFALSE;
}
static void ipsInstallEcuInfoDeInit(ADM_IPS_INSTALL_ECUINFO_T *e){
    if(e->ecuReqId != NULL){adm_free(e->ecuReqId);}
    if(e->ecuSwDid != NULL){adm_free(e->ecuSwDid);}
    //if(e->srcVer != NULL){adm_free(e->srcVer);}
    //if(e->dstVer != NULL){adm_free(e->dstVer);}
}
static void ipsInstallEcuInfoClone(const ADM_IPS_INSTALL_ECUINFO_T *src, ADM_IPS_INSTALL_ECUINFO_T *dst){
    if(src->ecuReqId != NULL){dst->ecuReqId = adm_strdup(src->ecuReqId);}
    if(src->ecuSwDid != NULL){dst->ecuSwDid = adm_strdup(src->ecuSwDid);}
    dst->ecuTimeConsuming = src->ecuTimeConsuming;
    dst->ecuTimeout = src->ecuTimeout;
    dst->isRollback = src->isRollback;
}
static void ipsInstallEcuInfoCreate(ADM_IPS_INSTALL_ECUINFO_T *e, const ADM_IPS_POLICY_UPGRADE_ECU_T *upgradeEcu, DBOOL isRollback){
    if(upgradeEcu->ecuReqId != NULL){e->ecuReqId = adm_strdup(upgradeEcu->ecuReqId);}
    if(upgradeEcu->ecuSwDid != NULL){e->ecuSwDid = adm_strdup(upgradeEcu->ecuSwDid);}
    e->ecuTimeConsuming = upgradeEcu->ecuTimeConsuming;
    e->ecuTimeout = upgradeEcu->ecuTimeout;
    e->isRollback = isRollback;
}
static struct json_object* ipsInstallEcuInfoJsonCreate(const ADM_IPS_INSTALL_ECUINFO_T *ecu){
    struct json_object* root = json_object_new_object();
    if(NULL != root){
	if(NULL != ecu->ecuReqId){
	    struct json_object* ecuReqIdVal = json_object_new_string(ecu->ecuReqId);
	    if(NULL != ecuReqIdVal){
		(void)json_object_object_add(root, "ecuReqId", ecuReqIdVal);
	    }
	}
	if(NULL != ecu->ecuSwDid){
	    struct json_object* ecuSwDidVal = json_object_new_string(ecu->ecuSwDid);
	    if(NULL != ecuSwDidVal){
		(void)json_object_object_add(root, "ecuSwDid", ecuSwDidVal);
	    }
	}
	struct json_object* ecuTimeConsumingVal = json_object_new_int(ecu->ecuTimeConsuming);
	if(NULL != ecuTimeConsumingVal){
	    (void)json_object_object_add(root, "ecuTimeConsuming", ecuTimeConsumingVal);
	}
	struct json_object* ecuTimeoutVal = json_object_new_int(ecu->ecuTimeout);
	if(NULL != ecuTimeoutVal){
	    (void)json_object_object_add(root, "ecuTimeout", ecuTimeoutVal);
	}
	struct json_object* isRollbackVal = json_object_new_int((D32S)ecu->isRollback);
	if(NULL != isRollbackVal){
	    (void)json_object_object_add(root, "isRollback", isRollbackVal);
	}
    }
    return root;
}
static void ipsInstallEcuInfoJsonParse(ADM_IPS_INSTALL_ECUINFO_T *ecu, const struct json_object* root){
    struct json_object* ecuReqIdVal = json_object_object_get(root, "ecuReqId");
    if(NULL != ecuReqIdVal){
        const DCHAR* ecuReqId = json_object_get_string(ecuReqIdVal);
        if (NULL != ecuReqId) {
            ecu->ecuReqId = adm_strdup(ecuReqId);
        }
    }
    struct json_object* ecuSwDidVal = json_object_object_get(root, "ecuSwDid");
    if(NULL != ecuSwDidVal){
        const DCHAR* ecuSwDid = json_object_get_string(ecuSwDidVal);
        if (NULL != ecuSwDid) {
            ecu->ecuSwDid = adm_strdup(ecuSwDid);
        }
    }
    struct json_object* ecuTimeConsumingVal = json_object_object_get(root, "ecuTimeConsuming");
    if(NULL != ecuTimeConsumingVal){
	    ecu->ecuTimeConsuming = json_object_get_int(ecuTimeConsumingVal);
    }
    struct json_object* ecuTimeoutVal = json_object_object_get(root, "ecuTimeout");
    if(NULL != ecuTimeoutVal){
	    ecu->ecuTimeout = json_object_get_int(ecuTimeoutVal);
    }
    struct json_object* isRollbackVal = json_object_object_get(root, "isRollback");
    if(NULL != isRollbackVal){
	    ecu->isRollback = (DBOOL)json_object_get_int(isRollbackVal);
    }
}


/**
 * IPS数组
 */
static void ipsArrayInit(ADM_IPS_ARRAY_T* array)
{
    array->eles = NULL;
    array->eleSize = 0;
    array->num = 0;
}

static D32S ipsArrayCreateElements(ADM_IPS_ARRAY_T* arr, D32S size, D32S count)
{
    arr->eles = (void*)adm_malloc((DSIZE)size * (DSIZE)count);
    if (NULL == arr->eles)
    {
        return ADM_ECODE_NO_MEMORY;
    }
    arr->eleSize = size;
    arr->num = count;

    return ADM_ECODE_SUCCESSED;
}

static void ipsArrayDeinit(ADM_IPS_ARRAY_T* arr)
{
    adm_free(arr->eles);
}

static void* ipsArrayGet(ADM_IPS_ARRAY_T* arr, D32S index)
{
    return (((DCHAR*)arr->eles) + index * arr->eleSize);
}

/**
 * @struct  ADM_IPS_ARRAY_ITER_T
 * @brief   数组迭代器
 * @details 获取数组当前元素，下一个元素，支持正向和逆向遍历
 */
struct IPS_ARRAY_ITER;
typedef void* (*ArrayIterNext)(struct IPS_ARRAY_ITER* iter);
typedef struct IPS_ARRAY_ITER
{
    ArrayIterNext       next;       // 下一元素获取方法
    ADM_IPS_ARRAY_T* array;      // 数组
    D32S                pos;        // 当前获取位置
} ADM_IPS_ARRAY_ITER_T;

static void* ipsArrayIterGet(ADM_IPS_ARRAY_ITER_T* iter)
{
    return ipsArrayGet(iter->array, iter->pos);
}

static void* ipsArrayIterNext(ADM_IPS_ARRAY_ITER_T* iter)
{
    return (++iter->pos) < iter->array->num ? ipsArrayGet(iter->array, iter->pos) : NULL;
}

static void* ipsArrayIterPrevious(ADM_IPS_ARRAY_ITER_T* iter)
{
    return (--iter->pos) >= 0 ? ipsArrayGet(iter->array, iter->pos) : NULL;
}

static void ipsArrayIterInit(ADM_IPS_ARRAY_ITER_T* iter, ADM_IPS_ARRAY_T* array, DBOOL reverse)
{
    iter->array = array;
    if (DFALSE == reverse)
    {
        iter->next = (ArrayIterNext)ipsArrayIterNext;
        iter->pos = 0;
    }
    else
    {
        iter->next = (ArrayIterNext)ipsArrayIterPrevious;
        iter->pos = array->num - 1;
    }
}

static DBOOL ipsArrayIterReverse(ADM_IPS_ARRAY_ITER_T* iter)
{
    DBOOL bRes = FALSE;

    if (iter->pos >= 0)
    {
        if (iter->pos == iter->array->num)
        {
            iter->pos--;
        }

        iter->next = (ArrayIterNext)ipsArrayIterPrevious;
        bRes = TRUE;
    }

    return bRes;
}


/**
 * @struct  IPS_BINARY_HEAP_T
 * @brief   二叉堆
 */
typedef struct
{
    void**  array;      // 数组元素
    D32S    capacity;   // 数组容量
    D32S    count;      // 元素个数
    DBOOL(*compare)(void* a, void* b);   // 元素比较函数
} IPS_BINARY_HEAP_T;

static D32S ipsBinaryHeapInit(IPS_BINARY_HEAP_T* heap, D32S capacity)
{
    void**  arr = (void**)adm_malloc((DSIZE)sizeof(void*) * (DSIZE)capacity);
    if (NULL == arr)
    {
        return ADM_ECODE_NO_MEMORY;
    }

    (void)adm_memset(arr, 0, sizeof(void*) * (DSIZE)capacity, sizeof(void*) * (DSIZE)capacity);
    heap->array = arr;
    heap->capacity = capacity;
    heap->count = 0;

    return ADM_ECODE_SUCCESSED;
}

static void ipsBinaryHeapDeinit(IPS_BINARY_HEAP_T* heap)
{
    adm_free(heap->array);
}

static D32S ipsBinaryHeapAdd(IPS_BINARY_HEAP_T* heap, void* ele)
{
    D32S    ret = ADM_ECODE_FAILED;
    D32S    childIdx = heap->count;
    D32S    parentIdx = 0;
    void**  arr = heap->array;

    if (childIdx < heap->capacity)
    {
        while (childIdx > 0)
        {
            parentIdx = (childIdx - 1) >> 1;

            if (DTRUE == heap->compare(arr[parentIdx], ele))     // 比较父节点是否满足，不满足父节点下沉
            {
                break;
            }

            /* 父节点下沉，子节点上浮 */
            arr[childIdx] = arr[parentIdx];
            childIdx = parentIdx;
        }

        arr[childIdx] = ele;
        heap->count++;

        ret = ADM_ECODE_SUCCESSED;
    }

    return ret;
}

static void* ipsBinaryHeapGet(IPS_BINARY_HEAP_T* heap)
{
    void*   ele = NULL;
    void**  arr = heap->array;

    if (heap->count > 0)
    {
        ele = arr[0];
        heap->count--;

        D32S    count = heap->count;
        void*   lastEle = arr[count];
        D32S    parentIdx = 0;
        D32S    childIdx = 1;

        while (childIdx < count)
        {
            if (childIdx + 1 < count
                && DTRUE == heap->compare(arr[childIdx + 1], arr[childIdx])) // 比较左右节点，获得与父节点比较的节点
            {
                childIdx++;
            }

            if (DTRUE == heap->compare(lastEle, arr[childIdx]))      // 父节点与子节点比较
            {
                break;
            }

            /* 父节点下沉，子节点上浮 */
            arr[parentIdx] = arr[childIdx];
            parentIdx = childIdx;
            childIdx = (parentIdx << 1) + 1;
        }

        arr[parentIdx] = lastEle;
    }

    return ele;
}

static DBOOL ipsBinaryHeapIsEmpty(IPS_BINARY_HEAP_T* heap)
{
    return 0 == heap->count ? DTRUE : DFALSE;
}

static DBOOL ipsBinaryHeapCanAdd(IPS_BINARY_HEAP_T* heap)
{
    return heap->count < heap->capacity ? DTRUE : DFALSE;
}


/**
 * @struct  IPS_UPGRADE_ECU_QUERIER_T
 * @brief   可安装ECU信息查询器
 */
typedef struct
{
    const ADM_IPS_POLICY_T*     policy;     // 安装策略
    ADM_IPS_PROGRESS_T*         progress;   // 安装进度

    ADM_IPS_INSTALL_ECUINFO_T*  ecus;           // 可安装ECU信息缓冲区
    D32S                        ecuMaxCount;    // 可安装ECU最大获取数量
    D32S                        ecuCount;       // 已获取的可安装ECU数量
} IPS_UPGRADE_ECU_QUERIER_T;

static DBOOL ipsUpgradeEcuQuerierFindEcu(IPS_UPGRADE_ECU_QUERIER_T* querier, ADM_IPS_ECU_INDEX_T* index, DBOOL isRollback)
{
    DBOOL ret = ipsEcuIndexGetInstallEcu(index, querier->policy, querier->progress, 
                                         &querier->ecus[querier->ecuCount], isRollback);
    if (DTRUE == ret)
    {
        querier->ecuCount++;
    }

    return ret;
}


static IPS_UPGRADE_GROUP_FUNC_T g_serialGroupFunc;
static IPS_UPGRADE_GROUP_FUNC_T g_parallelGroupFunc;
static IPS_UPGRADE_GROUP_FUNC_T g_indexGroupFunc;
static IPS_UPGRADE_GROUP_FUNC_T g_indexSerialGroupFunc;
static IPS_UPGRADE_GROUP_FUNC_T g_indexParallelGroupFunc;
static IPS_UPGRADE_GROUP_FUNC_T g_unorderBBGroupFunc;

#define TO_UPGRADE_GROUP(group)         ((IPS_UPGRADE_GROUP_T*)((void*)group))
#define TO_UPGRADE_GROUP_EX(group)      ((IPS_UPGRADE_GROUP_EX_T*)((void*)group))

/**
 * @struct  IPS_UPGRADE_GROUP_EX_T
 * @brief
 */
typedef struct
{
    IPS_UPGRADE_GROUP_T base;
    D32S                parallelMaxNum;   // 并行数量
} IPS_UPGRADE_GROUP_EX_T;

static void upgradeGroupExInit(IPS_UPGRADE_GROUP_T* group, D32S parallelMaxNum)
{
    ipsArrayInit(&TO_UPGRADE_GROUP(group)->array);
    TO_UPGRADE_GROUP(group)->func = NULL;
    TO_UPGRADE_GROUP_EX(group)->parallelMaxNum = parallelMaxNum;
}


/**
 * @struct  IPS_SERIAL_UPGRADE_GROUP_T
 * @brief   串行升级组
 */
typedef struct
{
    IPS_UPGRADE_GROUP_EX_T  base;
    ADM_IPS_ARRAY_ITER_T    iter;   // 迭代器
} IPS_SERIAL_UPGRADE_GROUP_T;

static void ipsSerialGroupInit(IPS_SERIAL_UPGRADE_GROUP_T* group, D32S parallelMaxNum)
{
    ipsArrayInit(&TO_UPGRADE_GROUP(group)->array);
    TO_UPGRADE_GROUP(group)->func = &g_serialGroupFunc;
    TO_UPGRADE_GROUP_EX(group)->parallelMaxNum = parallelMaxNum;
    ipsArrayIterInit(&group->iter, &TO_UPGRADE_GROUP(group)->array, DFALSE);
}

static D32S ipsSerialGroupFetch(IPS_SERIAL_UPGRADE_GROUP_T* group, void* param, D32S maxNum, DBOOL reverse)
{
    D32S                    count = 0;
    IPS_UPGRADE_GROUP_T*    child = NULL;
    D32S                    maxFetchNum = TO_UPGRADE_GROUP_EX(group)->parallelMaxNum;

    if (maxFetchNum > maxNum)
    {
        maxFetchNum = maxNum;
    }

    if (TRUE == reverse)
    {
        if (TRUE != ipsArrayIterReverse(&group->iter))
        {
            return 0;
        }
    }

    child = (IPS_UPGRADE_GROUP_T*)ipsArrayIterGet(&group->iter);
    while (NULL != child)
    {
        count = child->func->fetch(child, param, maxFetchNum, reverse);
        if (count > 0)
        {
            break;
        }

        child = (IPS_UPGRADE_GROUP_T*)group->iter.next(&group->iter);
    }
    
    return count;
}


/**
 * @struct  IPS_PARALLEL_UPGRADE_GROUP_T
 * @brief   并升级组
 */
typedef struct
{
    IPS_UPGRADE_GROUP_EX_T      base;
} IPS_PARALLEL_UPGRADE_GROUP_T;

static void ipsParallelGroupInit(IPS_PARALLEL_UPGRADE_GROUP_T* group, D32S parallelMaxNum)
{
    ipsArrayInit(&TO_UPGRADE_GROUP(group)->array);
    TO_UPGRADE_GROUP(group)->func = &g_parallelGroupFunc;
    TO_UPGRADE_GROUP_EX(group)->parallelMaxNum = parallelMaxNum;
}

static D32S ipsParallelGroupFetch(IPS_PARALLEL_UPGRADE_GROUP_T* group, void* param, D32S maxNum, DBOOL reverse)
{
    D32S                count = 0;
    IPS_UPGRADE_GROUP_T*child = NULL;
    D32S                maxFetchNum = TO_UPGRADE_GROUP_EX(group)->parallelMaxNum;

    if (maxFetchNum > maxNum)
    {
        maxFetchNum = maxNum;
    }

    ADM_IPS_ARRAY_ITER_T iter;
    ipsArrayIterInit(&iter, &TO_UPGRADE_GROUP(group)->array, reverse);

    child = (IPS_UPGRADE_GROUP_T*)ipsArrayIterGet(&iter);
    while (NULL != child)
    {
        count += child->func->fetch(child, param, maxFetchNum - count, reverse);
        if (count >= maxFetchNum)
        {
            break;
        }

        child = (IPS_UPGRADE_GROUP_T*)iter.next(&iter);
    }

    return count;
}

static void upgradeGroupGroupsDeinit(IPS_UPGRADE_GROUP_T* group)
{
    IPS_UPGRADE_GROUP_T* child = NULL;
    ADM_IPS_ARRAY_T* array = &group->array;

    for (D32S i = 0; i < array->num; i++)
    {
        child = (IPS_UPGRADE_GROUP_T*)ipsArrayGet(array, i);
        child->func->deinit(child);
    }

    adm_free(array->eles);
}

/**
 * 索引组
 */
static D32S ipsIndexGroupCreate(IPS_UPGRADE_GROUP_T* group, ADM_IPS_ECU_UPGRADE_ORDER_T** ecuOrders, D32S count)
{
    D32S ret = ipsArrayCreateElements(&group->array, (D32S)sizeof(ADM_IPS_ECU_INDEX_T), count);
    if (ADM_ECODE_SUCCESSED == ret)
    {
        for (D32S i = 0; i < count; i++)
        {
            ipsEcuIndexClone(&ecuOrders[i]->index, &((ADM_IPS_ECU_INDEX_T*)group->array.eles)[i]);
        }
    }

    return ret;
}

static D32S ipsIndexGroupFetch(IPS_UPGRADE_GROUP_T* group, IPS_UPGRADE_ECU_QUERIER_T* querier, D32S maxNum, DBOOL reverse)
{
    D32S                    count = 0;
    ADM_IPS_ECU_INDEX_T*    index = NULL;
    ADM_IPS_ARRAY_ITER_T    iter;

    ipsArrayIterInit(&iter, &group->array, reverse);
    index = (ADM_IPS_ECU_INDEX_T*)ipsArrayIterGet(&iter);
    while (NULL != index)
    {
        if (DTRUE == ipsUpgradeEcuQuerierFindEcu(querier, index, reverse))
        {
            count++;
            if (count >= maxNum)
            {
                break;
            }
        }

        index = (ADM_IPS_ECU_INDEX_T*)iter.next(&iter);
    }

    //for (D32S i = 0; i < array->num; i++) {
    //    index = (ADM_IPS_ECU_INDEX_T*)ipsArrayGet(array, i);
    //    if (DTRUE == ipsEcuQuerierFindEcu(querier, index, reverse)) {
    //        count++;
    //        if (count >= maxNum)
    //        {
    //            break;
    //        }
    //    }
    //}

    return count;
}

static void ipsIndexGroupInit(IPS_UPGRADE_GROUP_T* group)
{
    ipsArrayInit(&TO_UPGRADE_GROUP(group)->array);
    TO_UPGRADE_GROUP(group)->func = &g_indexGroupFunc;
}

static void ipsIndexGroupDeinit(IPS_UPGRADE_GROUP_T* group)
{
    for (D32S i = 0; i < group->array.num; i++)
    {
        ipsEcuIndexDeInit((ADM_IPS_ECU_INDEX_T*)ipsArrayGet(&group->array, i));
    }

    adm_free(group->array.eles);
}

static D32S ipsIndexGroupsCreate(ADM_IPS_ARRAY_T* array, D32S num)
{
    D32S ret = ipsArrayCreateElements(array, (D32S)sizeof(IPS_UPGRADE_GROUP_T), num);
    if (ADM_ECODE_SUCCESSED == ret)
    {
        IPS_UPGRADE_GROUP_T* groups = (IPS_UPGRADE_GROUP_T*)array->eles;
        for (D32S i = 0; i < num; i++)
        {
            ipsIndexGroupInit(&groups[i]);
        }
    }

    return ret;
}


/**
 * 串行索引组
 */
static D32S ipsSerialGroupsCreate(ADM_IPS_ARRAY_T* array, D32S num, D32S onceFetchNum)
{
    D32S ret = ipsArrayCreateElements(array, (D32S)sizeof(IPS_SERIAL_UPGRADE_GROUP_T), num);
    if (ADM_ECODE_SUCCESSED == ret)
    {
        IPS_SERIAL_UPGRADE_GROUP_T* groups = (IPS_SERIAL_UPGRADE_GROUP_T*)array->eles;
        for (D32S i = 0; i < num; i++)
        {
            ipsSerialGroupInit(&groups[i], onceFetchNum);
        }
    }

    return ret;
}

static void ipsIndexSerialGroupInit(IPS_SERIAL_UPGRADE_GROUP_T* group, D32S onceFetchNum)
{
    ipsArrayInit(&TO_UPGRADE_GROUP(group)->array);
    TO_UPGRADE_GROUP(group)->func = &g_indexSerialGroupFunc;
    TO_UPGRADE_GROUP_EX(group)->parallelMaxNum = onceFetchNum;
    ipsArrayIterInit(&group->iter, &TO_UPGRADE_GROUP(group)->array, DFALSE);
}

static D32S ipsIndexSerialGroupsCreate(ADM_IPS_ARRAY_T* array, D32S num, D32S onceFetchNum)
{
    D32S ret = ipsArrayCreateElements(array, (D32S)sizeof(IPS_SERIAL_UPGRADE_GROUP_T), num);
    if (ADM_ECODE_SUCCESSED == ret)
    {
        IPS_SERIAL_UPGRADE_GROUP_T* groups = (IPS_SERIAL_UPGRADE_GROUP_T*)array->eles;
        for (D32S i = 0; i < num; i++)
        {
            ipsIndexSerialGroupInit(&groups[i], onceFetchNum);
        }
    }

    return ret;
}


/**
 * 并行索引组
 */
static D32S ipsParallelGroupsCreate(ADM_IPS_ARRAY_T* array, D32S num, D32S onceFetchNum)
{
    D32S ret = ipsArrayCreateElements(array, (D32S)sizeof(IPS_PARALLEL_UPGRADE_GROUP_T), num);
    if (ADM_ECODE_SUCCESSED == ret)
    {
        IPS_PARALLEL_UPGRADE_GROUP_T* groups = (IPS_PARALLEL_UPGRADE_GROUP_T*)array->eles;
        for (D32S i = 0; i < num; i++)
        {
            ipsParallelGroupInit(&groups[i], onceFetchNum);
        }
    }

    return ret;
}

static void ipsIndexParallelGroupInit(IPS_PARALLEL_UPGRADE_GROUP_T* group, D32S onceFetchNum)
{
    ipsArrayInit(&TO_UPGRADE_GROUP(group)->array);
    TO_UPGRADE_GROUP(group)->func = &g_indexParallelGroupFunc;
    TO_UPGRADE_GROUP_EX(group)->parallelMaxNum = onceFetchNum;
}

static D32S ipsIndexParallelGroupsCreate(ADM_IPS_ARRAY_T* array, D32S num, D32S onceFetchNum)
{
    D32S ret = ipsArrayCreateElements(array, (D32S)sizeof(IPS_PARALLEL_UPGRADE_GROUP_T), num);
    if (ADM_ECODE_SUCCESSED == ret)
    {
        IPS_PARALLEL_UPGRADE_GROUP_T* groups = (IPS_PARALLEL_UPGRADE_GROUP_T*)array->eles;
        for (D32S i = 0; i < num; i++)
        {
            ipsIndexParallelGroupInit(&groups[i], onceFetchNum);
        }
    }

    return ret;
}


/**
 * 索引组初始化
 */
static D32S ipsUpgradeGroupCreatorInitIndexGroup(IPS_UPGRADE_GROUP_CREATOR_T* creator,
    IPS_UPGRADE_GROUP_T* group, IPS_UPGRADE_GROUP_CREATE_CTX_T* ctx)
{
    D32S c = ipsEcuUpgradeOrderGetGroup(&ctx->upgradeOrders[ctx->upgradeOrderPos],
                                        ctx->curCCEndPos - ctx->upgradeOrderPos,
                                        ADM_IPS_ECU_UPGRADE_ORDER_GROUP_ID_SEQUENCE);
    D32S ret = ipsIndexGroupCreate(group, &ctx->upgradeOrders[ctx->upgradeOrderPos], c);
    if (ADM_ECODE_SUCCESSED == ret)
    {
        ctx->upgradeOrderPos += c;
    }

    return ret;
}

/**
 * CC组初始化
 */
static D32S ipsUpgradeGroupCreatorInitCCGroup(IPS_UPGRADE_GROUP_CREATOR_T* creator,
    IPS_UPGRADE_GROUP_T* group, IPS_UPGRADE_GROUP_CREATE_CTX_T* ctx)
{
    IPS_UPGRADE_GROUP_CREATOR_T* child = creator->child;
    //D32S n = ipsEcuUpgradeOrderGetGroup(&ctx->upgradeOrders[ctx->upgradeOrderPos], 
    //                                    ctx->upgradeOrderCount - ctx->upgradeOrderPos,
    //                                    ADM_IPS_ECU_UPGRADE_ORDER_GROUP_ID_NET);
    D32S count = ipsEcuUpgradeOrderGetGroupCount(&ctx->upgradeOrders[ctx->upgradeOrderPos], 
                                                 ctx->curCCEndPos - ctx->upgradeOrderPos, 
                                                 ADM_IPS_ECU_UPGRADE_ORDER_GROUP_ID_SEQUENCE);
    D32S ret = child->createGroup(&group->array, count, ctx->policy->parallelNumLimitN);
    if (ADM_ECODE_SUCCESSED == ret)
    {
        for (D32S i = 0; i < count; i++)
        {
            ret = child->initGroup(child, (IPS_UPGRADE_GROUP_T*)ipsArrayGet(&group->array, i), ctx);
            if (ADM_ECODE_SUCCESSED != ret)
            {
                break;
            }
        }
    }

    return ret;
}

/**
 * CC组(索引组)初始化
 */
static D32S ipsUpgradeGroupCreatorInitCCGroupEx(IPS_UPGRADE_GROUP_CREATOR_T* creator,
    IPS_UPGRADE_GROUP_T* group, IPS_UPGRADE_GROUP_CREATE_CTX_T* ctx)
{
//     D32S n = ipsEcuUpgradeOrderGetGroup(&ctx->upgradeOrders[ctx->upgradeOrderPos],
//                                         ctx->upgradeOrderCount - ctx->upgradeOrderPos,
//                                         ADM_IPS_ECU_UPGRADE_ORDER_GROUP_ID_NET);
    D32S count = ipsEcuUpgradeOrderGetGroupCount(&ctx->upgradeOrders[ctx->upgradeOrderPos],
                                                ctx->curCCEndPos - ctx->upgradeOrderPos, 
                                                ADM_IPS_ECU_UPGRADE_ORDER_GROUP_ID_SEQUENCE);
    D32S ret = ipsIndexGroupCreate(group, &ctx->upgradeOrders[ctx->upgradeOrderPos], count);
    if (ADM_ECODE_SUCCESSED == ret)
    {
        ctx->upgradeOrderPos += count;
    }

    return ret;
}

/**
 * 升级组创建器初始化BB组
 */
static D32S ipsUpgradeGroupCreatorInitBBGroup(IPS_UPGRADE_GROUP_CREATOR_T* creator,
    IPS_UPGRADE_GROUP_T* group, IPS_UPGRADE_GROUP_CREATE_CTX_T* ctx)
{
    IPS_UPGRADE_GROUP_CREATOR_T* child = creator->child;
//     D32S n = ipsEcuUpgradeOrderGetGroup(&ctx->upgradeOrders[ctx->upgradeOrderPos], 
//                                         ctx->upgradeOrderCount - ctx->upgradeOrderPos, 
//                                         ADM_IPS_ECU_UPGRADE_ORDER_GROUP_ID_UPGRADE);         // 获取同一AA下ECU数量
    D32S count = ipsEcuUpgradeOrderGetGroupCount(&ctx->upgradeOrders[ctx->upgradeOrderPos], 
                                                 ctx->curBBEndPos - ctx->upgradeOrderPos, 
                                                 ADM_IPS_ECU_UPGRADE_ORDER_GROUP_ID_NET);       // 获取同一BB不同CC的数量
    D32S ret = child->createGroup(&group->array, count, ctx->policy->parallelNumLimitN);
    if (ADM_ECODE_SUCCESSED == ret)
    {
        for (D32S i = 0; i < count; i++)
        {
             D32S c = ipsEcuUpgradeOrderGetGroup(&ctx->upgradeOrders[ctx->upgradeOrderPos],
                                                 ctx->curBBEndPos - ctx->upgradeOrderPos,
                                                 ADM_IPS_ECU_UPGRADE_ORDER_GROUP_ID_NET);
             ctx->curCCEndPos = c + ctx->upgradeOrderPos;

             ret = child->initGroup(child, (IPS_UPGRADE_GROUP_T*)ipsArrayGet(&group->array, i), ctx);
            if (ADM_ECODE_SUCCESSED != ret)
            {
                break;
            }
        }
    }

    return ret;
}

/**
 * 升级组创建器初始化AA组
 */
static D32S ipsUpgradeGroupCreatorInitAAGroup(IPS_UPGRADE_GROUP_CREATOR_T* creator,
    IPS_UPGRADE_GROUP_T* group, IPS_UPGRADE_GROUP_CREATE_CTX_T* ctx)
{
    IPS_UPGRADE_GROUP_CREATOR_T* child = creator->child;
    D32S count = ipsEcuUpgradeOrderGetGroupCount(&ctx->upgradeOrders[ctx->upgradeOrderPos], 
                                                 ctx->upgradeOrderCount - ctx->upgradeOrderPos, 
                                                 ADM_IPS_ECU_UPGRADE_ORDER_GROUP_ID_UPGRADE);   // 获取不同AA的数量
    D32S ret = child->createGroup(&group->array, count, ctx->policy->parallelNumLimitM);        // 创建BB组
    if (ADM_ECODE_SUCCESSED == ret)
    {
        for (D32S i = 0; i < count; i++)
        {
            D32S c = ipsEcuUpgradeOrderGetGroup(&ctx->upgradeOrders[ctx->upgradeOrderPos],
                                                ctx->upgradeOrderCount - ctx->upgradeOrderPos,
                                                ADM_IPS_ECU_UPGRADE_ORDER_GROUP_ID_UPGRADE);
            ctx->curBBEndPos = ctx->upgradeOrderPos + c;

            ret = child->initGroup(child, (IPS_UPGRADE_GROUP_T*)ipsArrayGet(&group->array, i), ctx);    // 初始化BB
            if (ADM_ECODE_SUCCESSED != ret)
            {
                break;
            }
        }
    }

    return ret;
}

/**
 * 升级组创建器创建升级组
 */
static IPS_UPGRADE_GROUP_T* ipsUpgradeGroupCreatorCreateUpgradeGroup(IPS_UPGRADE_GROUP_CREATOR_T* creator, IPS_UPGRADE_GROUP_CREATE_CTX_T* ctx)
{
    ADM_IPS_ARRAY_T array;
    IPS_UPGRADE_GROUP_T* group = NULL;

    if (ADM_ECODE_SUCCESSED == creator->createGroup(&array, 1, ctx->policy->parallelNumLimitM)) // 创建AA组
    {
        group = (IPS_UPGRADE_GROUP_T*)array.eles;
        if (ADM_ECODE_SUCCESSED != creator->initGroup(creator, group, ctx)) // 初始化AA组
        {
            group->func->deinit(group);
            group = NULL;
        }
    }

    return group;
}

/**
* 版本一：AA为升级组，BB为网络组，CC为序列组，AA串行BB并行CC串行(相同CC并行)，与设计文档中对应，
* 与ADM_IPS_ECU_UPGRADE_GROUP_ARRAY_T实现原理一致，只是为了后续优化扩展实现的此方案。
* ECU升级序列格式为AABBCC，ECU按AA、BB、CC分组，不同AA为不同组，AA根据BB分组，BB根据CC分组。
* 根据ECU升级序列AABBCC，ECU间升级的顺序按需满足如下规则：
*   1. 不同的AA之间串行
*   2. 相同AA下不同BB之间并行
*   3. 相同BB下不同CC之间串行
*   4. 相同CC并行(不同BB下CC并行)
*   5. AA、BB和CC升序优先级由序号大小决定，序号越小优先级越高
*/
static IPS_UPGRADE_GROUP_T* ipsEcuUpgradeGroupsCreateV1(IPS_UPGRADE_GROUP_CREATE_CTX_T* ctx)
{
    IPS_UPGRADE_GROUP_CREATOR_T indexGroupCreator;
    IPS_UPGRADE_GROUP_CREATOR_T ccGroupCreator;
    IPS_UPGRADE_GROUP_CREATOR_T bbGroupCreator;
    IPS_UPGRADE_GROUP_CREATOR_T aaGroupCreator;

    indexGroupCreator.child = NULL;
    // indexGroupCreator.createGroup = (UPGRADE_GROUP_CREATOR_CREATE_GROUP)ipsIndexParallelGroupsCreate;       // 创建索引组
    indexGroupCreator.createGroup = (UPGRADE_GROUP_CREATOR_CREATE_GROUP)ipsIndexGroupsCreate;       // 创建索引组
    indexGroupCreator.initGroup = (UPGRADE_GROUP_CREATOR_INIT_GROUP)ipsUpgradeGroupCreatorInitIndexGroup;   // 初始化索引组

    ccGroupCreator.child = &indexGroupCreator;
    ccGroupCreator.createGroup = (UPGRADE_GROUP_CREATOR_CREATE_GROUP)ipsSerialGroupsCreate;         // 创建CC串行组
    ccGroupCreator.initGroup = (UPGRADE_GROUP_CREATOR_INIT_GROUP)ipsUpgradeGroupCreatorInitCCGroup; // 初始化CC组

    bbGroupCreator.child = &ccGroupCreator;
    bbGroupCreator.createGroup = (UPGRADE_GROUP_CREATOR_CREATE_GROUP)ipsParallelGroupsCreate;       // 创建BB并行组
    bbGroupCreator.initGroup = (UPGRADE_GROUP_CREATOR_INIT_GROUP)ipsUpgradeGroupCreatorInitBBGroup; // 初始化BB组

    aaGroupCreator.child = &bbGroupCreator;
    aaGroupCreator.createGroup = (UPGRADE_GROUP_CREATOR_CREATE_GROUP)ipsSerialGroupsCreate;         // 创建AA串行组
    aaGroupCreator.initGroup = (UPGRADE_GROUP_CREATOR_INIT_GROUP)ipsUpgradeGroupCreatorInitAAGroup; // 初始化AA组

    return ipsUpgradeGroupCreatorCreateUpgradeGroup(&aaGroupCreator, ctx);
}

/**
* 版本二：由Lotus并行升级策略开始讨论决定的第一个升级策略版本，该版本策略暂时没有项目使用。
* ECU升级序列格式为AABBCC，ECU按AA、BB、CC分组，不同AA为不同组，AA根据BB分组，BB根据CC分组。
* 根据ECU升级序列AABBCC，ECU间升级的顺序按需满足如下规则：
*   1. 不同的AA之间串行
*   2. 相同AA下不同BB之间并行
*   3. 相同BB下CC不同，CC之间并行
*   4. AA、BB和CC升序优先级由序号大小决定，序号越小优先级越高
*   5. 相同AA并行升级ECU数量为M(10)，相同BB并行升级ECU数量为N(3)
* 
* 有升级序列如以下的ECU：
*   AA  BB  CC
*   01  01  01-05
*   01  02  06-10
*   01  03  11-13
*   01  04  16-20
* M=10，N=3
* 第一批升级ECUs顺序如下：
*   AA  BB  CC
*   01  01  01,02,03
*   01  02  06,07,08
*   01  03  11,12,13
*   01  04  16
* 当CC为13的ECU首先升级完成，补充BB为04、CC为17的ECU升级
*   AA  BB  CC
*   01  01  01,02,03
*   01  02  06,07,08
*   01  03  11,12
*   01  04  16,17
*/
static IPS_UPGRADE_GROUP_T* ipsEcuUpgradeGroupsCreateV2(IPS_UPGRADE_GROUP_CREATE_CTX_T* ctx)
{
    IPS_UPGRADE_GROUP_CREATOR_T ccGroupCreator;
    IPS_UPGRADE_GROUP_CREATOR_T bbGroupCreator;
    IPS_UPGRADE_GROUP_CREATOR_T aaGroupCreator;

    ccGroupCreator.child = NULL;
    ccGroupCreator.createGroup = (UPGRADE_GROUP_CREATOR_CREATE_GROUP)ipsIndexParallelGroupsCreate;      // 创建CC并行索引组
    ccGroupCreator.initGroup = (UPGRADE_GROUP_CREATOR_INIT_GROUP)ipsUpgradeGroupCreatorInitCCGroupEx;   // 初始化CC组
    
    bbGroupCreator.child = &ccGroupCreator;
    bbGroupCreator.createGroup = (UPGRADE_GROUP_CREATOR_CREATE_GROUP)ipsParallelGroupsCreate;       // 创建BB并行组
    bbGroupCreator.initGroup = (UPGRADE_GROUP_CREATOR_INIT_GROUP)ipsUpgradeGroupCreatorInitBBGroup; // 初始化BB组
    
    aaGroupCreator.child = &bbGroupCreator;
    aaGroupCreator.createGroup = (UPGRADE_GROUP_CREATOR_CREATE_GROUP)ipsSerialGroupsCreate;         // 创建AA串行组
    aaGroupCreator.initGroup = (UPGRADE_GROUP_CREATOR_INIT_GROUP)ipsUpgradeGroupCreatorInitAAGroup; // 初始化AA组
    
    return ipsUpgradeGroupCreatorCreateUpgradeGroup(&aaGroupCreator, ctx);
}


/**
* 版本三：由Lotus并行升级策略后续讨论确定的升级策略版本。
* ECU升级序列格式为AABBCC，ECU按AA、BB、CC分组，不同AA为不同组，AA根据BB分组，BB根据CC分组。
* 根据ECU升级序列AABBCC，ECU间升级的顺序按需满足如下规则：
*   1. 不同的AA之间串行
*   2. 不同AA下不同BB之间并行
*   3. 不同BB下CC不同，CC之间并行
*   4. AA和CC升序优先级由序号大小决定，序号越小优先级越高，BB间无优先级，由CC的优先级确定
*   5. 不同AA并行升级ECU数量为M(10)，不同BB并行升级ECU数量为N(3)
*
* 有升级序列如以下的ECU：
*   AA  BB  CC
*   01  01  02,07-10
*   01  02  03,11-15
*   01  03  04,21-22
*   01  04  01,16-20
*   01  05  05,06
* M=10，N=3
* 第一批升级ECUs顺序如下：
*   AA  BB  CC
*   01  04  01
*   01  01  02,07,08
*   01  02  03,11,12
*   01  03  04
*   01  05  05,06
* 当CC为04的ECU首先升级完成，补充BB为04、CC为16的ECU升级
*   AA  BB  CC
*   01  04  01,16
*   01  01  02,07,08
*   01  02  03,11,12
*   01  05  05,06
*/

/**
 * @struct  IPS_UNORDER_BB_GROUP_ECU_INFO_T
 * @brief   无序BB组ECU升级信息
 * @details 用于获取可升级ECU。升级组BB的优先级由CC组的优先级决定，跟BB的序号大小无关
 */
typedef struct
{
    D32S                    ccOrderNo;      // CC组序列号
    ADM_IPS_ARRAY_ITER_T    ccIter;         // CC组迭代器

    D32S                fetchCount;         // 已获取的ECU数量
    D32S                maxFetchCount;      // 最大获取ECU数量

    const ADM_IPS_POLICY_UPGRADE_ECU_T* upgradeEcu; // ecu升级策略信息
    ADM_IPS_ECU_PROGRESS_T*             progress;   // ecu升级进度
} IPS_UNORDER_BB_GROUP_ECU_INFO_T;

static DBOOL ipsUnorderBBGroupInfoAsc(void* a, void* b)
{
    D32S ccOrderNo1 = ((IPS_UNORDER_BB_GROUP_ECU_INFO_T*)a)->ccOrderNo;
    D32S ccOrderNo2 = ((IPS_UNORDER_BB_GROUP_ECU_INFO_T*)b)->ccOrderNo;
    
    // 升级中的ECU优先级最高，待升级的ECU序号按位或0x40000000，降低升级优先级
    if (!ipsEcuIsIsInstalling(((IPS_UNORDER_BB_GROUP_ECU_INFO_T*)a)->progress->state))
    {
        ccOrderNo1 |= 0x40000000;
    }

    if (!ipsEcuIsIsInstalling(((IPS_UNORDER_BB_GROUP_ECU_INFO_T*)b)->progress->state))
    {
        ccOrderNo2 |= 0x40000000;
    }

    return  ccOrderNo1 <= ccOrderNo2 ? DTRUE : DFALSE;
}

static DBOOL ipsUnorderBBGroupInfoDesc(void* a, void* b)
{
    D32S ccOrderNo1 = ((IPS_UNORDER_BB_GROUP_ECU_INFO_T*)a)->ccOrderNo;
    D32S ccOrderNo2 = ((IPS_UNORDER_BB_GROUP_ECU_INFO_T*)b)->ccOrderNo;

    // 升级中的ECU优先级最高，升级中的ECU序号按位或0x40000000，提升升级优先级
    if (ipsEcuIsIsInstalling(((IPS_UNORDER_BB_GROUP_ECU_INFO_T*)a)->progress->state))
    {
        ccOrderNo1 |= 0x40000000;
    }

    if (ipsEcuIsIsInstalling(((IPS_UNORDER_BB_GROUP_ECU_INFO_T*)b)->progress->state))
    {
        ccOrderNo2 |= 0x40000000;
    }

    return ccOrderNo1 > ccOrderNo2 ? DTRUE : DFALSE;
}

/**
 * 无序BB组
 */
static void ipsUnorderBBGroupInit(IPS_UPGRADE_GROUP_EX_T* group, D32S onceFetchNum)
{
    ipsArrayInit(&TO_UPGRADE_GROUP(group)->array);
    TO_UPGRADE_GROUP(group)->func = &g_unorderBBGroupFunc;
    TO_UPGRADE_GROUP_EX(group)->parallelMaxNum = onceFetchNum;
}

static D32S ipsUnorderBBGroupsCreate(ADM_IPS_ARRAY_T* array, D32S num, D32S onceFetchNum)
{
    D32S ret = ipsArrayCreateElements(array, (D32S)sizeof(IPS_UPGRADE_GROUP_EX_T), num);
    if (ADM_ECODE_SUCCESSED == ret)
    {
        IPS_UPGRADE_GROUP_EX_T* groups = (IPS_UPGRADE_GROUP_EX_T*)array->eles;
        for (D32S i = 0; i < num; i++)
        {
            ipsUnorderBBGroupInit(&groups[i], onceFetchNum);
        }
    }

    return ret;
}

/**
 * 升级组创建器初始化无序BB组
 */
static D32S ipsUpgradeGroupCreatorInitUnorderBBGroup(IPS_UPGRADE_GROUP_CREATOR_T* creator,
    IPS_UPGRADE_GROUP_T* group, IPS_UPGRADE_GROUP_CREATE_CTX_T* ctx)
{
    IPS_UPGRADE_GROUP_CREATOR_T ccGroupCreator;
    IPS_UPGRADE_GROUP_CREATOR_T bbGroupCreator;

    ccGroupCreator.child = NULL;
    ccGroupCreator.initGroup = (UPGRADE_GROUP_CREATOR_INIT_GROUP)ipsUpgradeGroupCreatorInitCCGroupEx;
    ccGroupCreator.createGroup = (UPGRADE_GROUP_CREATOR_CREATE_GROUP)ipsIndexParallelGroupsCreate;       // 创建CC并行组，也是索引组

    bbGroupCreator.child = &ccGroupCreator;
    bbGroupCreator.initGroup = NULL;
    bbGroupCreator.createGroup = NULL;

    return ipsUpgradeGroupCreatorInitBBGroup(&bbGroupCreator, group, ctx);
}

static D32S ipsUnorderBBGroupFetch(IPS_UPGRADE_GROUP_T* group, IPS_UPGRADE_ECU_QUERIER_T* querier, D32S maxNum, DBOOL reverse)
{
    D32S                ret = ADM_ECODE_FAILED;
    D32S                totalCount = 0; // BB组获取的可升级ECU总数
    D32S                maxFetchNum = TO_UPGRADE_GROUP_EX(group)->parallelMaxNum;
    IPS_BINARY_HEAP_T       heap;       // 缓存BB组中每个BB可升级ECU信息，获取升级优先级最高的ECU
    ADM_IPS_ARRAY_ITER_T    bbIter;
    IPS_UPGRADE_GROUP_T* bb = NULL;
    IPS_UNORDER_BB_GROUP_ECU_INFO_T* bbEcuInfos = NULL;  // 缓存BB组中每个BB可升级ECU信息
    IPS_UNORDER_BB_GROUP_ECU_INFO_T* info = NULL;        // 每个BB可升级ECU信息
    ADM_IPS_ECU_INDEX_T* ecuIndex = NULL;
    const ADM_IPS_POLICY_UPGRADE_ECU_T* upgradeEcu = NULL;
    ADM_IPS_ECU_PROGRESS_T* ecuProgress = NULL;

    if (maxFetchNum > maxNum)
    {
        maxFetchNum = maxNum;
    }

    ret = ipsBinaryHeapInit(&heap, TO_UPGRADE_GROUP(group)->array.num);
    if (ADM_ECODE_SUCCESSED != ret)
    {
        return ret;
    }
    heap.compare = DFALSE == reverse ? ipsUnorderBBGroupInfoAsc : ipsUnorderBBGroupInfoDesc;

    bbEcuInfos = adm_malloc((DSIZE)sizeof(IPS_UNORDER_BB_GROUP_ECU_INFO_T) * (DSIZE)TO_UPGRADE_GROUP(group)->array.num);
    if (NULL == bbEcuInfos)
    {
        ipsBinaryHeapDeinit(&heap);
        return ADM_ECODE_NO_MEMORY;
    }

    /* 遍历BB组，获取BB中第一个可升级或升级中的ECU信息 */
    ipsArrayIterInit(&bbIter, &group->array, reverse);
    bb = (IPS_UPGRADE_GROUP_T*)ipsArrayIterGet(&bbIter);
    while (NULL != bb)
    {
        info = &bbEcuInfos[bbIter.pos];
        info->fetchCount = 0;
        info->maxFetchCount = TO_UPGRADE_GROUP_EX(bb)->parallelMaxNum;

        ipsArrayIterInit(&info->ccIter, &bb->array, reverse);
        ecuIndex = (ADM_IPS_ECU_INDEX_T*)ipsArrayIterGet(&info->ccIter);
        while (NULL != ecuIndex)
        {
            upgradeEcu = ipsPolicyFindUpgradeEcu(querier->policy, ecuIndex);
            ecuProgress = ipsProgressFindEcu(querier->progress, upgradeEcu->ecuReqId, upgradeEcu->ecuSwDid);
            if (ipsEcuCanInstall(ecuProgress->state))
            {
                info->ccOrderNo = ipsPolicyUpgradeEcuGetSequenceId(ecuIndex->upgradeOrder);
                info->progress = ecuProgress;
                info->upgradeEcu = upgradeEcu;

                (void)ipsBinaryHeapAdd(&heap, info);
                break;
            }

            ecuIndex = (ADM_IPS_ECU_INDEX_T*)info->ccIter.next(&info->ccIter);
        }

        bb = (IPS_UPGRADE_GROUP_T*)bbIter.next(&bbIter);
    }

    while (!ipsBinaryHeapIsEmpty(&heap))
    {
        /* 获取升级优先级最高的ECU */
        info = ipsBinaryHeapGet(&heap);
        if (ipsEcuProgressGetInstallEcu(info->progress, info->upgradeEcu, &querier->ecus[querier->ecuCount], reverse))
        {
            querier->ecuCount++;
            totalCount++;
            info->fetchCount++;
            if (totalCount >= maxFetchNum)  // 获取到的满足条件的ECU达到最大数量，结束获取
            {
                break;
            }
        }

        /* 同一BB中获取的ECU */
        if (info->fetchCount < info->maxFetchCount)
        {
            ecuIndex = (ADM_IPS_ECU_INDEX_T*)info->ccIter.next(&info->ccIter);
            while(NULL != ecuIndex)
            {
                upgradeEcu = ipsPolicyFindUpgradeEcu(querier->policy, ecuIndex);
                ecuProgress = ipsProgressFindEcu(querier->progress, upgradeEcu->ecuReqId, upgradeEcu->ecuSwDid);
                if (ipsEcuCanInstall(ecuProgress->state))
                {
                    info->ccOrderNo = ipsPolicyUpgradeEcuGetSequenceId(ecuIndex->upgradeOrder);
                    info->progress = ecuProgress;
                    info->upgradeEcu = upgradeEcu;

                    (void)ipsBinaryHeapAdd(&heap, info);
                    break;
                }

                ecuIndex = (ADM_IPS_ECU_INDEX_T*)info->ccIter.next(&info->ccIter);
            }
        }
    }

    ipsBinaryHeapDeinit(&heap);
    adm_free(bbEcuInfos);

    return totalCount;
}

static IPS_UPGRADE_GROUP_T* ipsEcuUpgradeGroupsCreateV3(IPS_UPGRADE_GROUP_CREATE_CTX_T* ctx)
{
    IPS_UPGRADE_GROUP_CREATOR_T bbGroupCreator;
    IPS_UPGRADE_GROUP_CREATOR_T aaGroupCreator;

    bbGroupCreator.child = NULL;
    bbGroupCreator.createGroup = (UPGRADE_GROUP_CREATOR_CREATE_GROUP)ipsUnorderBBGroupsCreate; // 创建BB并行组
    bbGroupCreator.initGroup = (UPGRADE_GROUP_CREATOR_INIT_GROUP)ipsUpgradeGroupCreatorInitUnorderBBGroup;
    
    aaGroupCreator.child = &bbGroupCreator;
    aaGroupCreator.createGroup = (UPGRADE_GROUP_CREATOR_CREATE_GROUP)ipsSerialGroupsCreate;    // 创建AA串行组
    aaGroupCreator.initGroup = (UPGRADE_GROUP_CREATOR_INIT_GROUP)ipsUpgradeGroupCreatorInitAAGroup;
    
    return ipsUpgradeGroupCreatorCreateUpgradeGroup(&aaGroupCreator, ctx);
}

static IPS_UPGRADE_GROUP_T* ipsEcuUpgradeGroupsCreateByVersion(D32S version, IPS_UPGRADE_GROUP_CREATE_CTX_T* ctx)
{
    IPS_UPGRADE_GROUP_T* group = NULL;
    log_i("Create upgrade group by version [%d].", version);
    switch (version)
    {
    case 1:
        group = ipsEcuUpgradeGroupsCreateV1(ctx);
        break;
    case 2:
        group = ipsEcuUpgradeGroupsCreateV2(ctx);
        break;
    case 3:
        group = ipsEcuUpgradeGroupsCreateV3(ctx);
        break;
    default:
        break;
    }

    return group;
}

static D32S ipsEcuUpgradeGroupsCreateEx(ADM_IPS_ECU_UPGRADE_GROUP_MANAGER_T* manager, const ADM_IPS_POLICY_T* policy)
{
    ADM_IPS_ECU_UPGRADE_ORDER_ARRAY_T ecusUpgradeOrder;
    ipsEcuUpgradeOrderArrayInit(&ecusUpgradeOrder);
    ipsEcuUpgradeOrderArrayCreate(&ecusUpgradeOrder, policy);

    IPS_UPGRADE_GROUP_CREATE_CTX_T ctx;
    ctx.upgradeOrders = ecusUpgradeOrder.pEcus;
    ctx.upgradeOrderCount = ecusUpgradeOrder.ecusLen;
    ctx.upgradeOrderPos = 0;
    ctx.policy = policy;

    D32S ret = ADM_ECODE_FAILED;
#ifndef UPGRADE_GROUP_POLICY_VERSION
    int version = 1;
#else
    int version = UPGRADE_GROUP_POLICY_VERSION;
#endif

    manager->group = ipsEcuUpgradeGroupsCreateByVersion(version, &ctx);

    ipsEcuUpgradeOrderArrayDeInit(&ecusUpgradeOrder);

    return ret;
}

static D32S ipsEcuUpgradeGroupArrayGetInstallEcusEx(
    ADM_IPS_ECU_UPGRADE_GROUP_MANAGER_T* manager,
    const ADM_IPS_POLICY_T* policy, ADM_IPS_PROGRESS_T* progress,
    ADM_IPS_INSTALL_ECUINFO_T* ecus, D32S ecuMaxNum)
{
    D32S count = 0;
    IPS_UPGRADE_ECU_QUERIER_T querier;
    querier.policy = policy;
    querier.progress = progress;
    querier.ecuCount = 0;
    querier.ecuMaxCount = ecuMaxNum;
    querier.ecus = ecus;

    if (NULL != manager->group)
    {
        count = manager->group->func->fetch(manager->group, &querier, ecuMaxNum, manager->isRollback);
        if (0 == count && DFALSE == manager->isRollback)     // 升级阶段无可升级ECU，进入回滚阶段
        {
            manager->isRollback = DTRUE;
            count = manager->group->func->fetch(manager->group, &querier, ecuMaxNum, manager->isRollback);
        }
    }

    return count;
}

static void upgradeGroupsFuncsInit(void)
{
    g_serialGroupFunc.fetch = (UPGRADE_GROUP_FETCH)((void*)ipsSerialGroupFetch);
    g_serialGroupFunc.deinit = upgradeGroupGroupsDeinit;

    g_parallelGroupFunc.fetch = (UPGRADE_GROUP_FETCH)ipsParallelGroupFetch;
    g_parallelGroupFunc.deinit = (UPGRADE_GROUP_DEINIT)upgradeGroupGroupsDeinit;

    g_indexGroupFunc.fetch = (UPGRADE_GROUP_FETCH)ipsIndexGroupFetch;
    g_indexGroupFunc.deinit = (UPGRADE_GROUP_DEINIT)ipsIndexGroupDeinit;
    g_indexSerialGroupFunc.fetch = (UPGRADE_GROUP_FETCH)ipsSerialGroupFetch;
    g_indexSerialGroupFunc.deinit = (UPGRADE_GROUP_DEINIT)ipsIndexGroupDeinit;
    g_indexParallelGroupFunc.fetch = (UPGRADE_GROUP_FETCH)ipsParallelGroupFetch;
    g_indexParallelGroupFunc.deinit = (UPGRADE_GROUP_DEINIT)ipsIndexGroupDeinit;

    g_unorderBBGroupFunc.fetch = (UPGRADE_GROUP_FETCH)ipsUnorderBBGroupFetch;
    g_unorderBBGroupFunc.deinit = (UPGRADE_GROUP_DEINIT)upgradeGroupGroupsDeinit;
}

/**
 * @brief 更新ECU升级状态
 * @note
 * @param in ADM_IPS_ECU_PHASE_E eSrcState        源状态
 * @param in ADM_IPS_ECU_PHASE_E eDstState        目标状态
 * @retval D32S 0 成功，非0 失败
 */
D32S adm_ips_updateEcuFlashState(ADM_IPS_ECU_PHASE_E eSrcState,
                                 ADM_IPS_ECU_PHASE_E eDstState)
{
    D32S iRet = ADM_ECODE_FAILED;
    ADM_IPS_PROGRESS_T stProgressInfo = {0};
    ADM_IPS_ECU_PROGRESS_T *pstEcuProgressInfo = NULL;
    D32S i = 0;

    ADM_COM_FUNC_ENTRY();

    adm_ips_progressInit(&stProgressInfo);
    adm_ips_progressGet(&stProgressInfo);

    for (i = 0; i < stProgressInfo.ecus.ecusLen; i++)
    {
        pstEcuProgressInfo = &stProgressInfo.ecus.ecus[i];
        if (eSrcState == pstEcuProgressInfo->state)
        {
            /** 更新ECU状态 */
            ipsContextUpdateEcuState(&m_context, pstEcuProgressInfo->ecuReqId, pstEcuProgressInfo->ecuSwDid, eDstState);
        }
    }

    adm_ips_progressDeInit(&stProgressInfo);

    ADM_COM_FUNC_EXIT();

    return iRet;
}

/**
 * @brief 设置ECU刷写失败后的动作
 * @note
 * @param in void
 * @retval D32S 0 成功，非0 失败
 */
void adm_ips_ecuSetActionForFailed(void)
{
    ADM_IPS_PROGRESS_T stProgressInfo = {0};
    ADM_IPS_ECU_PROGRESS_T *pstEcuProgressInfo = NULL;
    D32S i = 0;

    ADM_COM_FUNC_ENTRY();

    adm_ips_progressInit(&stProgressInfo);
    adm_ips_progressGet(&stProgressInfo);

    for (i = 0; i < stProgressInfo.ecus.ecusLen; i++)
    {
        pstEcuProgressInfo = &stProgressInfo.ecus.ecus[i];

        /** ECU升级失败，需要根据策略设置后续执行动作 */
        if (0 != pstEcuProgressInfo->progress[ADM_IPS_ECU_PHASE_INDEX_UPGRADE].result &&
            ADM_IPS_ECU_PHASE_POST_INSTALL == pstEcuProgressInfo->state)
        {
            /** 更新ECU状态 */
            ipsEcuProgressSetResultActionForFailed(pstEcuProgressInfo, &m_context.policy, &m_context.progress);
        }
    }

    adm_ips_progressDeInit(&stProgressInfo);

    ADM_COM_FUNC_EXIT();

    return ;
}

/**
 * @brief 重新创建升级组
 * @note
 * @param in void
 * @retval void
 */
void adm_ips_reCreateUpdateGroup(void)
{
    ADM_COM_FUNC_ENTRY();

    adm_ips_init();

    ipsEcuUpgradeGroupManagerClear(&m_context.upgradeGroupManager);

    ipsEcuUpgradeGroupsCreateEx(&m_context.upgradeGroupManager, &m_context.policy);

    ADM_COM_FUNC_EXIT();

    return;
}

D32S adm_ips_reportScheduleInfo(const DCHAR* pJsonStr, ADM_OCS_BUSINESS_COMM_INTERFACE_CALLBACK_F fnBusinessCommCb)
{
    D32S iRet = ADM_ECODE_FAILED;
    DCHAR* respMessageBody = NULL;
    DSIZE rspStatusCode = 0;

    if (NULL == pJsonStr || NULL == fnBusinessCommCb)
    {
        log_e("invalid param.");
        return ADM_ECODE_INVALID_PARAM;
    }

    do
    {
        iRet = fnBusinessCommCb(ADM_OCS_BUSINESS_TYPE_SHEDULE_INFO_SYNC, pJsonStr, &respMessageBody, &rspStatusCode);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Sync schdule info faild. ret[%d]", iRet);
            break;
        }

        /** 针对云端返回信息不做处理 */
        log_i("ReportScheduleInfo finished.statusCode[%lu]", rspStatusCode);
    } while (FALSE);

    return iRet;
}

static DCHAR* ipsContextGetPolicyEcuUpgradeOrder(ADM_IPS_CONTEXT_T *c, const DCHAR *ecuReqId, const DCHAR *ecuSwDid)
{
    D32S i = 0;
    D32S j = 0;
    DCHAR* upgradeOrder = NULL;

    (void)pthread_mutex_lock(&c->mutex);
    for (i=0; i< c->policy.ecuGroupOrders.ecuGroupOrdersLen; i++)
    {
        for (j=0; j < c->policy.ecuGroupOrders.ecuGroupOrders[i].groupEcus.ecusLen; j++)
        {
            if (0 == adm_strcasecmp(c->policy.ecuGroupOrders.ecuGroupOrders[i].groupEcus.ecus[j].ecuReqId, ecuReqId) &&
                0 == adm_strcasecmp(c->policy.ecuGroupOrders.ecuGroupOrders[i].groupEcus.ecus[j].ecuSwDid, ecuSwDid))
            {
                upgradeOrder = adm_strdup(c->policy.ecuGroupOrders.ecuGroupOrders[i].groupEcus.ecus[j].upgradeOrder);
                break; 
            }
        }
    }
    (void)pthread_mutex_unlock(&c->mutex);

    return upgradeOrder;
}

static D32S ipsContextGetPolicyEcuActiveOrder(ADM_IPS_CONTEXT_T *c, const DCHAR *ecuReqId, const DCHAR *ecuSwDid)
{
    D32S i = 0;
    D32S j = 0;
    D32S activeOrder = -1;

    (void)pthread_mutex_lock(&c->mutex);
    for (i=0; i< c->policy.ecuGroupOrders.ecuGroupOrdersLen; i++)
    {
        for (j=0; j < c->policy.ecuGroupOrders.ecuGroupOrders[i].groupEcus.ecusLen; j++)
        {
            if (0 == adm_strcasecmp(c->policy.ecuGroupOrders.ecuGroupOrders[i].groupEcus.ecus[j].ecuReqId, ecuReqId) &&
                0 == adm_strcasecmp(c->policy.ecuGroupOrders.ecuGroupOrders[i].groupEcus.ecus[j].ecuSwDid, ecuSwDid))
            {
                activeOrder = c->policy.ecuGroupOrders.ecuGroupOrders[i].groupEcus.ecus[j].activeOrder;
                break; 
            }
        }
    }
    (void)pthread_mutex_unlock(&c->mutex);

    return activeOrder;
}


DCHAR* adm_ips_ecuUpgradeOrderGet(const DCHAR *ecuReqId, const DCHAR *ecuSwDid)
{
    if (ADM_IS_NULL(ecuReqId) || ADM_IS_NULL(ecuSwDid))
    {
        return NULL;
    }

    adm_ips_init();

    return ipsContextGetPolicyEcuUpgradeOrder(&m_context, ecuReqId, ecuSwDid);
}

D32S adm_ips_ecuActiveOrderGet(const DCHAR *ecuReqId, const DCHAR *ecuSwDid)
{
    if (ADM_IS_NULL(ecuReqId) || ADM_IS_NULL(ecuSwDid))
    {
        log_e("Invalid param.");
        return -1;
    }

    adm_ips_init();

    return ipsContextGetPolicyEcuActiveOrder(&m_context, ecuReqId, ecuSwDid);
}


D32S adm_ips_getFlashResult()
{
    D32S iFlashResult = ADM_ECODE_FAILED;

    adm_ips_init();
    (void)pthread_mutex_lock(&m_context.mutex);

    if (ADM_IPS_PHASE_POST_INSTALL == m_context.progress.state)
    {
        iFlashResult = m_context.progress.result;
    }

    (void)pthread_mutex_unlock(&m_context.mutex);

    return iFlashResult;
}

DVOID adm_ips_updateEcuFlashResult(const DCHAR* pRequestId, const DCHAR* pSwdid, D32S iResult)
{
    ADM_IPS_ECU_PROGRESS_ARRAY_T* pstProgressArray = NULL;

    adm_ips_init();
    (void)pthread_mutex_lock(&m_context.mutex);

    pstProgressArray = &m_context.progress.ecus; 

    for (D32S i = 0; i < pstProgressArray->ecusLen; ++i)
    {
        ADM_IPS_ECU_PROGRESS_T *pstEcu = pstProgressArray->ecus + i;
        if (adm_strcasecmp(pRequestId, pstEcu->ecuReqId) == 0 &&
            adm_strcasecmp(pSwdid, pstEcu->ecuSwDid) == 0 &&
            ADM_IPS_ECU_PHASE_POST_INSTALL == pstEcu->state)
        {
            pstEcu->progress[ADM_IPS_ECU_PHASE_INDEX_UPGRADE].result = iResult;
            break;
        }
    }

    IPS_CONTEXT_EXEC(&m_context, IPS_CONTEXT_EXEC_FLAG_STORE | IPS_CONTEXT_EXEC_FLAG_SHOW);

    (void)pthread_mutex_unlock(&m_context.mutex);

    return;
}

static struct json_object *ipsActiveJsonCreate()
{
    struct json_object *pstRoot = NULL;
    if (g_uclEcuActiveContext.pstActiveEcuInfo != NULL && g_uclEcuActiveContext.uiEcuLen != 0)
    {
        pstRoot = json_object_new_object();
        if (pstRoot != NULL)
        {
            struct json_object *pstEcusArray = json_object_new_array();
            if (pstEcusArray != NULL)
            {
                for (D32U i = 0; i < g_uclEcuActiveContext.uiEcuLen; ++i)
                {
                    struct json_object *pstEachEcu = json_object_new_object();
                    if (NULL != pstEachEcu)
                    {
                        (void)json_object_object_add(pstEachEcu, "state", json_object_new_uint64((D64U)g_uclEcuActiveContext.pstActiveEcuInfo[i].etate));

                        (void)json_object_object_add(pstEachEcu, "activeOrder", json_object_new_int(g_uclEcuActiveContext.pstActiveEcuInfo[i].iActiveOrder));

                        (void)json_object_object_add(pstEachEcu, "activeResult", json_object_new_int(g_uclEcuActiveContext.pstActiveEcuInfo[i].iActiveResult));

                        if (NULL != g_uclEcuActiveContext.pstActiveEcuInfo[i].pRequestId)
                        {
                            (void)json_object_object_add(pstEachEcu, "requestId", json_object_new_string(g_uclEcuActiveContext.pstActiveEcuInfo[i].pRequestId));
                        }
                        if (NULL != g_uclEcuActiveContext.pstActiveEcuInfo[i].pSwdid)
                        {
                            (void)json_object_object_add(pstEachEcu, "swDid", json_object_new_string(g_uclEcuActiveContext.pstActiveEcuInfo[i].pSwdid));
                        }

                        if (NULL != g_uclEcuActiveContext.pstActiveEcuInfo[i].pInstallInstanceId)
                        {
                            (void)json_object_object_add(pstEachEcu, "installInstanceId", json_object_new_string(g_uclEcuActiveContext.pstActiveEcuInfo[i].pInstallInstanceId));
                        }
                    }

                    (void)json_object_array_add(pstEcusArray, pstEachEcu);
                }
                (void)json_object_object_add(pstRoot, "activeInfo", pstEcusArray);
            }
        }
    }

    return pstRoot;
}

static DVOID ipsClearActiveEcuInfoFromDb()
{
    if (TRUE == adm_com_kvstorage_has_key(adm_core_service_getBusinessDatabaseDb(), ADM_IPS_ACTIVE_KV_KEY))
    {
        adm_com_kvstorage_remove_key(adm_core_service_getBusinessDatabaseDb(), ADM_IPS_ACTIVE_KV_KEY);
    }

    return;
}

DVOID adm_ips_clearActiveEcuInfo()
{
    adm_ips_init();

    (void)pthread_mutex_lock(&g_uclEcuActiveContext.mutex);

    for (D32U i = 0; i < g_uclEcuActiveContext.uiEcuLen; i++)
    {
        g_uclEcuActiveContext.pstActiveEcuInfo[i].etate = ADM_IPS_ACTIVE_PRE;
        g_uclEcuActiveContext.pstActiveEcuInfo[i].iActiveOrder = -1;
        g_uclEcuActiveContext.pstActiveEcuInfo[i].iActiveResult = -1;
        adm_free(g_uclEcuActiveContext.pstActiveEcuInfo[i].pRequestId);
        adm_free(g_uclEcuActiveContext.pstActiveEcuInfo[i].pSwdid);
        adm_free(g_uclEcuActiveContext.pstActiveEcuInfo[i].pInstallInstanceId);
    }

    adm_free(g_uclEcuActiveContext.pstActiveEcuInfo);
    g_uclEcuActiveContext.uiEcuLen = 0;

    ipsClearActiveEcuInfoFromDb();

    (void)pthread_mutex_unlock(&g_uclEcuActiveContext.mutex);

    return;
}

static DVOID ipsSaveActiveEcuInfo2Db()
{
    struct json_object *pstRoot = ipsActiveJsonCreate();
    if (NULL != pstRoot)
    {
        const DCHAR *pStr = json_object_to_json_string(pstRoot);
        if (NULL != pStr)
        {
            (DVOID)adm_com_kvstorage_set_string(adm_core_service_getBusinessDatabaseDb(), ADM_IPS_ACTIVE_KV_KEY,
                                               pStr, ADM_KV_STORAGE_VALUE_TYPE_STRING);
            log_i("Current Active info:%s", pStr);
        }
        (DVOID)json_object_put(pstRoot);
    }
    return;
}

DVOID adm_ips_updateAllActiveState(ADM_ACTIVE_STATE_E etate)
{
    adm_ips_init();

    if (etate < ADM_IPS_ACTIVE_PRE || etate > ADM_IPS_ACTIVE_END)
    {
        log_e("Update state, active state [%d] is invalid.", (D32S)etate);
        return;
    }

    if (NULL == g_uclEcuActiveContext.pstActiveEcuInfo ||  0 == g_uclEcuActiveContext.uiEcuLen)
    {
        log_e("Update state, active ecu info is empty.");
        return;
    }

    (void)pthread_mutex_lock(&g_uclEcuActiveContext.mutex);

    for (D32U i = 0; i < g_uclEcuActiveContext.uiEcuLen; i++)
    {
        g_uclEcuActiveContext.pstActiveEcuInfo[i].etate = etate;
    }

    ipsSaveActiveEcuInfo2Db();

    (void)pthread_mutex_unlock(&g_uclEcuActiveContext.mutex);

    return;
}

DVOID adm_ips_updateEcuActiveState(ADM_ACTIVE_STATE_E etate, const DCHAR* pRequestId, const DCHAR* pSwdid)
{
    adm_ips_init();

    if (etate < ADM_IPS_ACTIVE_PRE || etate > ADM_IPS_ACTIVE_END)
    {
        log_e("Update state, ecu active state [%d] is invalid.", (D32S)etate);
        return;
    }

    if (NULL == g_uclEcuActiveContext.pstActiveEcuInfo || 0 == g_uclEcuActiveContext.uiEcuLen)
    {
        log_e("Update state, ecu active info is empty.");
        return;
    }

    for (D32U i = 0; i < g_uclEcuActiveContext.uiEcuLen; i++)
    {
        if (g_uclEcuActiveContext.pstActiveEcuInfo != NULL)
        {
            if (htoi(g_uclEcuActiveContext.pstActiveEcuInfo[i].pRequestId) == htoi(pRequestId) &&
                htoi(g_uclEcuActiveContext.pstActiveEcuInfo[i].pSwdid) == htoi(pSwdid))
            {
                (void)pthread_mutex_lock(&g_uclEcuActiveContext.mutex);
                g_uclEcuActiveContext.pstActiveEcuInfo[i].etate = etate;
                ipsSaveActiveEcuInfo2Db();
                (void)pthread_mutex_unlock(&g_uclEcuActiveContext.mutex);
                break;
            }
        }
    }

    return;
}

DVOID adm_ips_updateEcuActiveResult(D32S iActiveResult, const DCHAR *pRequestId, const DCHAR *pSwdid)
{
    adm_ips_init();

    if (NULL == g_uclEcuActiveContext.pstActiveEcuInfo || 0 == g_uclEcuActiveContext.uiEcuLen)
    {
        log_e("Update result, ecu active info is empty.");
        return;
    }

    for (D32U i = 0; i < g_uclEcuActiveContext.uiEcuLen; i++)
    {
        if (g_uclEcuActiveContext.pstActiveEcuInfo != NULL)
        {
            if (htoi(g_uclEcuActiveContext.pstActiveEcuInfo[i].pRequestId) == htoi(pRequestId) &&
                htoi(g_uclEcuActiveContext.pstActiveEcuInfo[i].pSwdid) == htoi(pSwdid))
            {
                (void)pthread_mutex_lock(&g_uclEcuActiveContext.mutex);
                g_uclEcuActiveContext.pstActiveEcuInfo[i].iActiveResult = iActiveResult;
                ipsSaveActiveEcuInfo2Db();
                (void)pthread_mutex_unlock(&g_uclEcuActiveContext.mutex);
                break;
            }
        }
    }

    return;
}

static DBOOL ipsIsSmartEcuInstance(const DCHAR* pInstanceId, const D16U* pAllInstanceId, D32U uiNum)
{
    DBOOL bIsFind = DFALSE;

    if (NULL != pInstanceId && NULL != pAllInstanceId)
    {
        for (D32U i = 0; i < uiNum; i++)
        {
            if (htoi(pInstanceId) == pAllInstanceId[i])
            {
                bIsFind = DTRUE;
                break;
            }
        }
    }

    return bIsFind;
}

DVOID adm_ips_loadActiveEcuInfo(const D16U* pInstanceId, D32U uiNum)
{
    adm_ips_init();
    DCHAR* pInstallInstanceId = NULL;

    D32U uiActiveEcuLength = 0;
    D32U k = 0;
    ADM_IPS_POLICY_ECU_GROUP_ORDER_ARRAY_T * pEcuGroupOrders = &m_context.policy.ecuGroupOrders;
    ADM_IPS_POLICY_UPGRADE_ECU_T* pUpgradeEcu = NULL;

    for (D32U i = 0; i < pEcuGroupOrders->ecuGroupOrdersLen; i++)
    {
        for (D32U j = 0; j < pEcuGroupOrders->ecuGroupOrders[i].groupEcus.ecusLen; j++)
        {
            /** 有单独激活顺序 */
            if (-1 != pEcuGroupOrders->ecuGroupOrders[i].groupEcus.ecus[j].activeOrder)
            {
                pInstallInstanceId = adm_ecs_getInstallInstanceId(pEcuGroupOrders->ecuGroupOrders[i].groupEcus.ecus[j].ecuReqId, pEcuGroupOrders->ecuGroupOrders[i].groupEcus.ecus[j].ecuSwDid);
                if (DTRUE == ipsIsSmartEcuInstance(pInstallInstanceId, pInstanceId, uiNum))
                {
                    uiActiveEcuLength++;
                }
                adm_free(pInstallInstanceId);
            }
        }
    }

    if (0 < uiActiveEcuLength)
    {
        g_uclEcuActiveContext.pstActiveEcuInfo = (ADM_IPS_UCL_ECU_ACTIVE_T *)malloc(uiActiveEcuLength * sizeof(ADM_IPS_UCL_ECU_ACTIVE_T));
        if (g_uclEcuActiveContext.pstActiveEcuInfo == NULL)
        {
            log_e("Can not malloc active ecu size[%u].", uiActiveEcuLength);
            return;
        }
        g_uclEcuActiveContext.uiEcuLen = uiActiveEcuLength;

        (void)pthread_mutex_lock(&g_uclEcuActiveContext.mutex);

        for (D32U i = 0; i < pEcuGroupOrders->ecuGroupOrdersLen; i++)
        {
            for (D32U j = 0; j < pEcuGroupOrders->ecuGroupOrders[i].groupEcus.ecusLen; j++)
            {
                /** 有单独激活顺序 */
                if (-1 != pEcuGroupOrders->ecuGroupOrders[i].groupEcus.ecus[j].activeOrder)
                {
                    g_uclEcuActiveContext.pstActiveEcuInfo[k].pInstallInstanceId = adm_ecs_getInstallInstanceId(pEcuGroupOrders->ecuGroupOrders[i].groupEcus.ecus[j].ecuReqId, pEcuGroupOrders->ecuGroupOrders[i].groupEcus.ecus[j].ecuSwDid);
                    if (DTRUE == ipsIsSmartEcuInstance(g_uclEcuActiveContext.pstActiveEcuInfo[k].pInstallInstanceId,
                        pInstanceId, uiNum))
                    {
                        g_uclEcuActiveContext.pstActiveEcuInfo[k].etate = ADM_IPS_ACTIVE_PRE;
                        g_uclEcuActiveContext.pstActiveEcuInfo[k].iActiveResult = -1; ///< 默认值
                        g_uclEcuActiveContext.pstActiveEcuInfo[k].iActiveOrder = pEcuGroupOrders->ecuGroupOrders[i].groupEcus.ecus[j].activeOrder;
                        g_uclEcuActiveContext.pstActiveEcuInfo[k].pRequestId = adm_strdup(pEcuGroupOrders->ecuGroupOrders[i].groupEcus.ecus[j].ecuReqId);
                        g_uclEcuActiveContext.pstActiveEcuInfo[k].pSwdid = adm_strdup(pEcuGroupOrders->ecuGroupOrders[i].groupEcus.ecus[j].ecuSwDid);
                        k++;
                    }
                    else 
                    {
                        adm_free(g_uclEcuActiveContext.pstActiveEcuInfo[k].pInstallInstanceId);
                    }
                }
            }
        }

        ipsSaveActiveEcuInfo2Db();
        (void)pthread_mutex_unlock(&g_uclEcuActiveContext.mutex);
    }
    else 
    {
        log_i("Has no active ecu.");
        (void)pthread_mutex_lock(&g_uclEcuActiveContext.mutex);
        g_uclEcuActiveContext.uiEcuLen = 0;
        (void)pthread_mutex_unlock(&g_uclEcuActiveContext.mutex);
    }

    return;
}

static DVOID ipsLoadActiveEcuInfoFromDb()
{
    ADM_KV_STORAGE_VALUE_TYPE_E eType = ADM_KV_STORAGE_VALUE_TYPE_INVALID;
    DCHAR *pContextJsonStr = NULL;
    size_t uiContextJsonStrLen = 0;
    DBOOL bIsGetSucceed = DFALSE;  
    struct json_object *pstRoot = NULL;

    do
    {
        g_uclEcuActiveContext.uiEcuLen = 0;

        bIsGetSucceed = adm_com_kvstorage_get_string(adm_core_service_getBusinessDatabaseDb(), ADM_IPS_ACTIVE_KV_KEY,
                                                &pContextJsonStr, &uiContextJsonStrLen, &eType);
        if (DFALSE == bIsGetSucceed || NULL == pContextJsonStr)
        {
            log_w("Load key:[%s] data failed.", ADM_IPS_ACTIVE_KV_KEY);
            break;
        }

        pstRoot = json_tokener_parse(pContextJsonStr);
        if (pstRoot == NULL)
        {
            log_e("Parse error.");
            break;
        }

        struct json_object *pstEcusArray = NULL;
        if (!json_object_object_get_ex(pstRoot, "activeInfo", &pstEcusArray))
        {
            log_e("Can not find activeInfo.");
            break;
        }

        if (!json_object_is_type(pstEcusArray, json_type_array))
        {
            log_e("ActiveInfo is not a array struct.");
            break;
        }

        D32U uiArrayLen = json_object_array_length(pstEcusArray);
        if (uiArrayLen == 0)
        {
            log_e("Array Size is zero.");
            break;
        }

        /* 分配新的内存 */
        g_uclEcuActiveContext.pstActiveEcuInfo = (ADM_IPS_UCL_ECU_ACTIVE_T *)malloc(uiArrayLen * sizeof(ADM_IPS_UCL_ECU_ACTIVE_T));
        if (g_uclEcuActiveContext.pstActiveEcuInfo == NULL)
        {
            log_e("Can not malloc size.");
            break;
        }
        g_uclEcuActiveContext.uiEcuLen = uiArrayLen;

        /* 解析每个ECU的信息 */
        for (D32U i = 0; i < uiArrayLen; ++i)
        {
            struct json_object *pstEachEcu = json_object_array_get_idx(pstEcusArray, i);
            if (pstEachEcu == NULL || !json_object_is_type(pstEachEcu, json_type_object))
            {
                log_w("Index [%u] empty.", i);
                continue;
            }

            struct json_object *pstValue = NULL;

            /* 解析state */
            if (json_object_object_get_ex(pstEachEcu, "state", &pstValue))
            {
                g_uclEcuActiveContext.pstActiveEcuInfo[i].etate = (ADM_ACTIVE_STATE_E)json_object_get_uint64(pstValue);
            }

            /* 解析activeOrder */
            if (json_object_object_get_ex(pstEachEcu, "activeOrder", &pstValue))
            {
                g_uclEcuActiveContext.pstActiveEcuInfo[i].iActiveOrder = json_object_get_int(pstValue);
            }

            /* 解析activeResult */
            if (json_object_object_get_ex(pstEachEcu, "activeResult", &pstValue))
            {
                g_uclEcuActiveContext.pstActiveEcuInfo[i].iActiveResult = json_object_get_int(pstValue);
            }

            /* 解析requestId */
            if (json_object_object_get_ex(pstEachEcu, "requestId", &pstValue))
            {
                const char *pcRequestId = json_object_get_string(pstValue);
                if (pcRequestId != NULL)
                {
                    g_uclEcuActiveContext.pstActiveEcuInfo[i].pRequestId = adm_strdup(pcRequestId);
                }
            }

            /* 解析swDid */
            if (json_object_object_get_ex(pstEachEcu, "swDid", &pstValue))
            {
                const char *pcSwdid = json_object_get_string(pstValue);
                if (pcSwdid != NULL)
                {
                    g_uclEcuActiveContext.pstActiveEcuInfo[i].pSwdid = adm_strdup(pcSwdid);
                }
            }

            /* 解析installInstanceId */
            if (json_object_object_get_ex(pstEachEcu, "installInstanceId", &pstValue))
            {
                const char *pcInstallInstanceId = json_object_get_string(pstValue);
                if (pcInstallInstanceId != NULL)
                {
                    g_uclEcuActiveContext.pstActiveEcuInfo[i].pInstallInstanceId = adm_strdup(pcInstallInstanceId);
                }
            }
        }
    }while (DFALSE);

    if (NULL != pstRoot)
    {
        json_object_put(pstRoot);
    }

    return;
}

const ADM_IPS_UCL_ECU_ARRAY_ACTIVE_T* adm_ips_getActiveEcuInfo()
{
    return &g_uclEcuActiveContext;
}

D32S adm_ips_progressGetUpgradeResult(ADM_IPS_PROGRESS_T *p, ADM_IPS_ECU_PHASE_E* pEcuUpgradeState, const DCHAR *pEcuReqId, const DCHAR *pEcuSwDid)
{
    ADM_IPS_ECU_PROGRESS_ARRAY_T *pstProgressArray = NULL;
    D32S iResult = ADM_ECODE_FAILED;

    pstProgressArray = &p->ecus;
    if (pstProgressArray->ecus != NULL && pEcuReqId != NULL && pEcuSwDid != NULL && NULL != pEcuUpgradeState)
    {
        for (D32S i = 0; i < pstProgressArray->ecusLen; ++i)
        {
            ADM_IPS_ECU_PROGRESS_T *ecu = pstProgressArray->ecus + i;
            if (adm_strcasecmp(pEcuReqId, ecu->ecuReqId) == 0 && adm_strcasecmp(pEcuSwDid, ecu->ecuSwDid) == 0)
            {
                *pEcuUpgradeState = ecu->state;
                iResult = ecu->progress[ADM_IPS_ECU_PHASE_INDEX_UPGRADE].result;
                break;
            }
        }
    }

    return iResult;
}

ADM_ACTIVE_STATE_E adm_ips_getActiveStage()
{
    DBOOL bIsFindIng = DFALSE;
    DBOOL bIsFindPre = DFALSE;
    ADM_ACTIVE_STATE_E eAcitveStage = ADM_IPS_ACTIVE_PRE;

    adm_ips_init();

    (void)pthread_mutex_lock(&g_uclEcuActiveContext.mutex);

    if (0 == g_uclEcuActiveContext.uiEcuLen || NULL == g_uclEcuActiveContext.pstActiveEcuInfo)
    {
        log_e("Get error active info.");
        eAcitveStage = ADM_IPS_ACTIVE_END;
        (void)pthread_mutex_unlock(&g_uclEcuActiveContext.mutex);
        return eAcitveStage;
    }

    for (D32U i = 0; i < g_uclEcuActiveContext.uiEcuLen; i++)
    {
        if (ADM_IPS_ACTIVE_PRE == g_uclEcuActiveContext.pstActiveEcuInfo[i].etate)
        {
            bIsFindPre = DTRUE;
            break;              ///< 如果有激活准备的直接返回
        }
        if (ADM_IPS_ACTIVE_ING == g_uclEcuActiveContext.pstActiveEcuInfo[i].etate)
        {
            bIsFindIng = DTRUE;
            continue;
        }
    }

    if (DTRUE == bIsFindPre)
    {
        eAcitveStage = ADM_IPS_ACTIVE_PRE;
    }
    else if (DTRUE == bIsFindIng)
    {
        eAcitveStage = ADM_IPS_ACTIVE_ING;
    }
    else
    {
        eAcitveStage = ADM_IPS_ACTIVE_END;
    }

    (void)pthread_mutex_unlock(&g_uclEcuActiveContext.mutex);

    return eAcitveStage;
}

DBOOL adm_ips_getIsActiveSmartEcu(const DCHAR *pRequestId, const DCHAR *pSwdid, ADM_ACTIVE_STATE_E* pActiveState)
{
    DBOOL bIsFind = DFALSE;
    if (NULL == pRequestId || NULL == pSwdid || NULL == pActiveState)
    {
        log_e("Invalid param.");
        return bIsFind;
    }

    (void)pthread_mutex_lock(&g_uclEcuActiveContext.mutex);

    for (D32U i = 0; i < g_uclEcuActiveContext.uiEcuLen; i++)
    {
        if (NULL == g_uclEcuActiveContext.pstActiveEcuInfo)
        {
            continue;
        }

        if (htoi(g_uclEcuActiveContext.pstActiveEcuInfo[i].pRequestId) == htoi(pRequestId) &&
               htoi(g_uclEcuActiveContext.pstActiveEcuInfo[i].pSwdid) == htoi(pSwdid))
        {
            bIsFind = DTRUE;
            *pActiveState = g_uclEcuActiveContext.pstActiveEcuInfo[i].etate;
            break;
        }
    }

    (void)pthread_mutex_unlock(&g_uclEcuActiveContext.mutex);

    log_i("Ecu requestId[%s] swDid[%s] find result[%d].", pRequestId, pSwdid, bIsFind);
    return bIsFind;
}