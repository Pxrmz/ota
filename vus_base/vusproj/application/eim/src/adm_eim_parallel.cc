/**
 * @file    adm_parallel.cc
 * @brief   并行策略,当前用于采集零件信息
 * @details detail description
 * @version v0.1.0
 * @author  renzhendi
 * @date    Created in the 2024-7-5
 * @copyright  Copyright (C) 2018 All rights Abup Technology Co., Ltd.
 *
 * @par     Function List :
 *
 * @history
  1.Date        : 2024-7-2
    Author      : renzhendi
    Modification: modification record
 *
 */

#include <list>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <ctype.h>
#include <time.h>
#include <unordered_set>
#include <string>

#include "adm_cfg_default.h"
#include "adm_cfg_api.h"
#include "adm_eim_log.h"
#include "adm_string.h"
#include "adm_memory.h"
#include "adm_eim_parallel.h"
#include "adm_vsm_fsm.h"
#include "adm_soa.h"

#include "adm_ecs_rsConfig.h"
#include "adm_dim_vdi.h"
#include "adm_dim_ucl.h"
#include "adm_dim_host.h"

#include "adm_com_kvstorage.h"
#include "adm_eim_getEcuInfo.h"
#include "adm_eim_ecuInfoManager.h"
#include "adm_eim_event.h"
#include "adm_ocm_interface.h"
#include "adm_ecs_rsConfig.h"

extern struct ECU_PARALLEL_COLLECT_COND_T g_stCollectCond;
extern struct ADM_EIM_BACK_COLLECT_T g_stBackCollectInfo;

D32U uiCollectCount = 1;  //采集失败重试一次
#define SINGLE_READECUINFO_TIMEOUT 120 /*读零件信息超时时间*/
#define EVENT_ECU_INFO_PARALLEL_ARRAY_LEN 256
#define ADM_CVM_EVENT_LIST_COUNT 30
#define ADM_BACK_COLLECT_COND_NUM 3
#define ADM_BACK_COLLECT_COND_MAX_TIME 40
#define ADM_BACK_COLLECT_COND_PER_WAIT 3

/** 获取零件信息请求重试次数 */
#define ADM_DIM_REQUEST_RETRY_COUNT 4

/** 获取零件信息请求重试间隔时间 */
#define ADM_DIM_REQUEST_RETRY_INTERVAL 15

static DBOOL g_bIsCollectThreadRuning = FALSE;
static ADM_OCS_EVENTS_INFO_LIST_T *g_pstEventsList = NULL;
static DBOOL g_bIsCollectRuning = FALSE;
static DBOOL g_bIsBackRuning = FALSE;
static DBOOL g_bPauseBackRuning = FALSE;
static D32S g_bIsStopByCond = FALSE;

D32S adm_eim_asyncGetApkVersion(const D32S instanceId, 
                                const ADM_ECS_ECUCONFIG_T* pstEcuCfg, 
                                const ADM_DIM_SOA_ECU_IDEN_INFO_T* pstEcuIdenInfo,
                                const ADM_DIM_SOA_DID_DATA_ARRAY_T* didArrayRequest,
                                D32S* errorCode);

//改为用eim原来的宏
#define ECU_INFO_ID_LEN 16
#define ECU_INFO_KV_KEY_LEN     64
#define ECU_INFO_KV_KEY_TXT    "[EIM]_[ECU_ID_%s_%s]"
#define ALL_ECU_INFO_KEY_TXT   "[EIM]_[allEcuInfoStr]"
#define ALL_ECU_CACHE_LIST_KEY "[EIM]_[allEcuCacheList]"
#define EIM_SET_ECU_TIMESTAMP  "[EIM]_[timestamp]"
#define ECU_PARTNUM_LEN 64

enum ADM_ECU_SINGLE_COLLECT_STATE_E
{
    ADM_ECU_SINGLE_COLLECT_BEGIN,  //default
    ADM_ECU_SINGLE_COLLECT_ING,
    ADM_ECU_SINGLE_COLLECT_SUCC,
    ADM_ECU_SINGLE_COLLECT_FAIL,
};

/** DID 与 ECS 关键信息字段比对结果 */
typedef enum ADM_EIM_KEY_CMP_RESULT_E
{
    EIM_KEY_CMP_NONE = 0,
    EIM_KEY_CMP_OK,             ///< 比对通过
    EIM_KEY_CMP_INCOMPLETE,     ///< 有did值未匹配上 || 所有did值为空
    EIM_KEY_CMP_MISMATCH,       ///< 所有did值均未匹配上
} ADM_EIM_KEY_CMP_RESULT_E;

enum ADM_ECU_ALL_COLLECT_STATE_E
{
    ADM_ECU_ALL_COLLECT_BEGIN,  //default
    ADM_ECU_ALL_COLLECT_ING,
    ADM_ECU_ALL_COLLECT_FINISH,
    ADM_ECU_ALL_COLLECT_CONFLICT,
};

struct ADM_ECU_SINGLE_INFO_T
{
    DCHAR acRequestId[ECU_INFO_ID_LEN];
    DCHAR acResponseId[ECU_INFO_ID_LEN];
    DCHAR acFunctionId[ECU_INFO_ID_LEN];
    DCHAR acSwDid[ECU_INFO_ID_LEN];
    DCHAR acPartNumber[ECU_PARTNUM_LEN];
    DCHAR acPartSupplier[ECU_INFO_ID_LEN];
    DCHAR acPartHardwareNum[ECU_INFO_ID_LEN];
    DCHAR acSsoftwarePartNumber[ECU_INFO_ID_LEN];
    DCHAR acSingleEcuKvKey[ECU_INFO_KV_KEY_LEN];
    DCHAR acUpOrder[ECU_INFO_ID_LEN];                  //0xABCDEF
    D32S iUpGroup;                    //AB
    D32S iPartGroup;                 //CD
    D32S iPriority;                 //EF
    ADM_ECU_SINGLE_COLLECT_STATE_E eCollectState;
    D32S uiCollectCount;  //采集次数记录
    struct timespec ts;  //记录当前时间
    D32U uiTimeOut;      //记录超时时长
    ADM_EIM_KEY_CMP_RESULT_E eLastKeyCmpResult;
    D32U uiKeyFieldRetryRound;

    ADM_ECU_SINGLE_INFO_T()
    {
        adm_memzero(acRequestId, ECU_INFO_ID_LEN);
        adm_memzero(acResponseId, ECU_INFO_ID_LEN);
        adm_memzero(acFunctionId, ECU_INFO_ID_LEN);
        adm_memzero(acSwDid, ECU_INFO_ID_LEN);
        adm_memzero(acPartNumber, ECU_PARTNUM_LEN);
        adm_memzero(acPartSupplier, ECU_INFO_ID_LEN);
        adm_memzero(acPartHardwareNum, ECU_INFO_ID_LEN);
        adm_memzero(acSsoftwarePartNumber, ECU_INFO_ID_LEN);
        adm_memzero(acSingleEcuKvKey, ECU_INFO_KV_KEY_LEN);
        adm_memzero(acUpOrder, ECU_INFO_ID_LEN);
        iUpGroup = 0;
        iPartGroup = 0;
        iPriority = 0;
        eCollectState = ADM_ECU_SINGLE_COLLECT_BEGIN;
        uiCollectCount = 0;
        adm_memzero(&ts, sizeof(struct timespec));
        uiTimeOut = 0;
        eLastKeyCmpResult = EIM_KEY_CMP_NONE;
        uiKeyFieldRetryRound = 0;
    }
};

struct ADM_ECU_CFG_INFO_CTL_T
{
    ADM_ECU_ALL_COLLECT_STATE_E eCollectAllState;
    pthread_mutex_t ecuCfgMutex;
    std::list<ADM_ECU_SINGLE_INFO_T*> listEcuSingleInfo;
};
static ADM_ECU_CFG_INFO_CTL_T stEcuCfgCtl;

typedef struct ADM_EIM_COLLECT_CACHE_INFO_
{
    std::list<ADM_EIM_ECU_IDEN_INFO_T*> ecuCacheList;
    pthread_mutex_t ecuCacheMutex;
}ADM_EIM_COLLECT_CACHE_INFO_T;

static ADM_EIM_COLLECT_CACHE_INFO_T g_stEcuCollectCache;

//采集器
struct ECU_PARALLEL_COLLECT_CTL_T
{
    pthread_mutex_t collectMutex;
    sem_t  collectSig;
    std::list<ADM_ECU_SINGLE_INFO_T*> collectEcuListM;
};
static ECU_PARALLEL_COLLECT_CTL_T stEcuParallelCollectCtl;

//从ecm拿配置
static ADM_ECS_RSCONFIGINFO_T* gpstRsCfgInfo = NULL;

static D32U g_uiWaitResultCount = 0;

static ADM_EIM_COLLECT_TRIGGER_TYPE_E g_eTriggerType = COLLECT_TRIGGER_TYPE_ING;

/** 成功比对信息通过的ecu信息 收集成功 + 高低配信息比对成功 begin */
#define MAX_COLLECT_ECU_NUM 200
typedef struct 
{
    DCHAR *ecuReqId; ///< 诊断请求ID
    DCHAR *ecuSwDid; ///< 升级对象软件版本 did
} ADM_EIM_ECU_COLLECT_SUCCESS_RECORD_T;

typedef struct 
{
    pthread_mutex_t ecuMutex;
    D32U uiIndex; ///< 当前已存储ecu数量
    ADM_EIM_ECU_COLLECT_SUCCESS_RECORD_T aEcuCollectSuccessRecord[MAX_COLLECT_ECU_NUM];
} ADM_EIM_ECU_COLLECT_SUCCESS_RECORD_ARRY_T;

static ADM_EIM_ECU_COLLECT_SUCCESS_RECORD_ARRY_T g_aEcuCollectSuccessRecord = {0};
/** 成功比对信息通过的ecu信息 收集成功 + 高低配信息比对成功 end */

static D32S adapterHtoI(DCHAR* pCh)
{
    if(ADM_IS_NULL(pCh))
    {
        return 0;
    }
    else
    {
        //uiValue = adm_atoD32U();
        return htoi(pCh);
    }
}
static D32U adapterItoH(D32S uiNum, D32U uiBase, DCHAR ac[], D32U acLen)
{
    if(10 == uiBase)
    {
        (void)adm_snprintf(ac, acLen, "%d", uiNum);
    }
    else if(16 == uiBase)
    {
        (void)adm_snprintf(ac, acLen, "0X%X", uiNum);  //注意字符串匹配时X为大写
    }
    else
    {
        adm_memset(ac, 0, acLen, acLen);
    }

    return 0;
}

/**
 * @brief ecu缓存信息节点释放
 * @note
 * @param 无
 * @return 无
 */
static void ecuCacheInfoNodeFree(void)
{
    ADM_COM_FUNC_ENTRY();

    pthread_mutex_lock(&g_stEcuCollectCache.ecuCacheMutex);

    /** 释放所有节点 */
    for(auto iter = g_stEcuCollectCache.ecuCacheList.begin(); iter != g_stEcuCollectCache.ecuCacheList.end(); ++iter)
    {
        ADM_EIM_ECU_IDEN_INFO_T* pstEcuSingleInifoTmp = *iter;

        if(ADM_IS_NULL(pstEcuSingleInifoTmp))
        {
            log_e("pstEcuSingleInifoTmp is NULL");
            continue;
        }

        adm_free(pstEcuSingleInifoTmp->pcRequestId);
        adm_free(pstEcuSingleInifoTmp->pcSwDid);
        adm_free(pstEcuSingleInifoTmp);
    }

    /** 清空列表 */
    if(!g_stEcuCollectCache.ecuCacheList.empty())
    {
        g_stEcuCollectCache.ecuCacheList.clear();
    }

    pthread_mutex_unlock(&g_stEcuCollectCache.ecuCacheMutex);

    ADM_COM_FUNC_EXIT();

    return;
}

/**
 * @brief ecu缓存信息节点添加
 * @note
 * @param in ADM_ECS_ECUCONFIG_T* pstEcuCfg 配置信息
 * @return 无
 */
static void ecuCacheInfoNodeAdd(ADM_ECS_ECUCONFIG_T* pstEcuCfg)
{
    ADM_EIM_ECU_IDEN_INFO_T *pstEcuIden = NULL;
    
    ADM_COM_FUNC_ENTRY();

    /** 检查参数 */
    if (ADM_IS_NULL(pstEcuCfg))
    {
        log_e("invalid param");
        return ;
    }
    
    /** 分配内存 */
    pstEcuIden = (ADM_EIM_ECU_IDEN_INFO_T *)adm_malloc(sizeof(ADM_EIM_ECU_IDEN_INFO_T));
    if (ADM_IS_NULL(pstEcuIden))
    {
        log_e("malloc failed");
        return ;
    }
    
    /** 初始化数据 */
    adm_memzero(pstEcuIden, sizeof(ADM_EIM_ECU_IDEN_INFO_T));
    pstEcuIden->pcRequestId = adm_strdup(pstEcuCfg->ecuReqId);
    pstEcuIden->pcSwDid = adm_strdup(pstEcuCfg->ecuSwDid);
    
    /** 添加到列表 */
    g_stEcuCollectCache.ecuCacheList.push_back(pstEcuIden);
    
    ADM_COM_FUNC_EXIT();
    
    return;
}

/**
 * @brief ecu缓存列表设置到KV
 * @note
 * @param 无
 * @return 无
 */
static void ecuCacheListSetToKV(void)
{
    ADM_EIM_ECU_IDEN_INFO_T *pstEcuIden = NULL;
    json_object *pEcuArrayObject = NULL;
    json_object *pSingleEcuObject = NULL;
    DCHAR *pcCacheStr = NULL;
    DBOOL bIsOk = FALSE;
    
    ADM_COM_FUNC_ENTRY();

    /** 初始化对象 */
    pEcuArrayObject = json_object_new_array();
    if (ADM_IS_NULL(pEcuArrayObject))
    {
        log_e("json_object_new_array failed");
        return ;
    }
    
    pthread_mutex_lock(&g_stEcuCollectCache.ecuCacheMutex);

    /** 遍历列表，添加到数组对象中 */
    for(auto iter = g_stEcuCollectCache.ecuCacheList.begin(); iter != g_stEcuCollectCache.ecuCacheList.end(); ++iter)
    {
        pstEcuIden = *iter;

        if(ADM_IS_NULL(pstEcuIden))
        {
            log_e("invalid pointer");
            continue;
        }
        
        /** 创建对象，并添加到数组对象中 */
        pSingleEcuObject = json_object_new_object();
        json_object_object_add(pSingleEcuObject, "requestId", json_object_new_string(pstEcuIden->pcRequestId));
        json_object_object_add(pSingleEcuObject, "swDid", json_object_new_string(pstEcuIden->pcSwDid));

        (void)json_object_array_add(pEcuArrayObject, pSingleEcuObject);

        pstEcuIden = DNULL;
    }
    pthread_mutex_unlock(&g_stEcuCollectCache.ecuCacheMutex);

    /** 转换为字符串，并设置到KV中 */
    if(0U != json_object_array_length(pEcuArrayObject))
    {
        pcCacheStr = (DCHAR*)json_object_to_json_string(pEcuArrayObject);
        log_i("key = %s ,pcCacheStr = %s", ALL_ECU_CACHE_LIST_KEY, pcCacheStr);
        bIsOk = adm_com_kvstorage_set_string(adm_core_service_getBusinessDatabaseDb(), 
                                             ALL_ECU_CACHE_LIST_KEY, 
                                             pcCacheStr, 
                                             ADM_KV_STORAGE_VALUE_TYPE_STRING);

        if(FALSE == bIsOk)
        {
            log_e("set all ecu info string return failed");
        }
    }
    else
    {
        log_w("ecu info no cache");
    }
    
    /** 内存释放 */
    json_object_put(pEcuArrayObject);
    ecuCacheInfoNodeFree();

    ADM_COM_FUNC_EXIT();
    
    return;
}

/**
 * @brief 从KV获取ecu缓存列表
 * @note
 * @param 无
 * @return DCHAR* 缓存列表字符串
 */
static DCHAR *ecuCacheListGetFromKV(void)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    DCHAR *pcCacheStr = NULL;
    DBOOL bIsOk = FALSE;
    size_t uilength = 0;
    ADM_KV_STORAGE_VALUE_TYPE_E eValType;

    ADM_COM_FUNC_ENTRY();

    do
    {
        /** 从KV获取缓存列表字符串 */
        bIsOk = adm_com_kvstorage_get_string(adm_core_service_getBusinessDatabaseDb(),
                                               ALL_ECU_CACHE_LIST_KEY,
                                               &pcCacheStr,
                                               &uilength,
                                               &eValType);
        if(FALSE == bIsOk)
        {
            log_e("get ecuInfo[%s] string return failed", ALL_ECU_CACHE_LIST_KEY);
            iRet = ADM_ECODE_KV_GET_STRING_FAILED;
            break;
        }

        log_i("key = %s ,pcCacheStr = %s", ALL_ECU_CACHE_LIST_KEY, pcCacheStr);

    }while (FALSE);

    ADM_COM_FUNC_EXIT();

    return pcCacheStr;
}

static void eim_addParallelEcuEventList(ADM_OCS_EVENTS_INFO_LIST_T* eventsList,
    const DCHAR* eventId,
    const DCHAR* ecuReqId,
    const DCHAR* ecuSwDid,
    const DCHAR* eventDesc)
{
    DCHAR eventDescArray[EVENT_ECU_INFO_PARALLEL_ARRAY_LEN];
    (void)adm_memset(eventDescArray, 0x00, EVENT_ECU_INFO_PARALLEL_ARRAY_LEN, EVENT_ECU_INFO_PARALLEL_ARRAY_LEN);
    (void)adm_snprintf(eventDescArray,
        EVENT_ECU_INFO_PARALLEL_ARRAY_LEN,
        "[%s + %s][%s](%s)",
        ecuReqId,
        ecuSwDid,
        adm_eim_getDateAndTimeStr(),
        eventDesc);
    (void)adm_eim_addEventList(eventsList, eventId, (D32S)ADM_OCS_EVENT_LEVEL_INFO, NULL, eventDescArray);
}

#if 0
/**
 * @brief 清除ECU信息
 * @note
 * @param in DVOID
 * @retval void
 */
void clearEcuSingleInfo(void)
{
    D32S i = 0;
    D32S iRet = ADM_ECODE_SUCCESSED;
    json_object *pEcuArrayObject = NULL;
    json_object *pSingleEcuObject = NULL;
    DCHAR acEcuKey[ECU_INFO_KV_KEY_LEN] = {0};
    DCHAR *pcCacheStr = NULL;
    ADM_EIM_ECU_IDEN_INFO_T stEcuIden = {0};

    ADM_COM_FUNC_ENTRY();

    pcCacheStr = ecuCacheListGetFromKV();
    if (ADM_IS_NULL(pcCacheStr))
    {
        log_e("pcCacheStr is NULL");
        return;
    }

    pEcuArrayObject = json_tokener_parse(pcCacheStr);

    /** 判断json类型是否为数组 */
    if(json_object_get_type(pEcuArrayObject) != json_type_array)
    {
        log_e("json_object_get_type failed");
        return;
    }

    /** 循环解析json数组 */
    for(i = 0; i < json_object_array_length(pEcuArrayObject); i++)
    {
        pSingleEcuObject = json_object_array_get_idx(pEcuArrayObject, i);
        stEcuIden.pcRequestId = adm_com_getJsonStrVal(pSingleEcuObject, "requestId");
        stEcuIden.pcSwDid = adm_com_getJsonStrVal(pSingleEcuObject, "swDid");

        adm_memzero(acEcuKey, ECU_INFO_KV_KEY_LEN);
        (void)adm_snprintf(acEcuKey, ECU_INFO_KV_KEY_LEN, ECU_INFO_KV_KEY_TXT, stEcuIden.pcRequestId, stEcuIden.pcSwDid);

        if(TRUE == adm_com_kvstorage_has_key(adm_core_service_getBusinessDatabaseDb(), acEcuKey))
        {
            log_d("delete ecu info from kv, key:%s", acEcuKey);
            if (FALSE == adm_com_kvstorage_remove_key(adm_core_service_getBusinessDatabaseDb(), acEcuKey))
            {
                log_e("RemoveKey[%s] return failed", acEcuKey);
            }
        }

        adm_free(stEcuIden.pcRequestId);
        adm_free(stEcuIden.pcSwDid);
    }

    /** 内存释放 */
    adm_free(pcCacheStr);

    ADM_COM_FUNC_EXIT();

    return ;
}
#endif

/**
 * @brief 清除车型配置文件中不存在的ECU信息
 * @note
 * @param in DVOID
 * @retval void
 */
static void clearEcuSingleInfo(void)
{
    D32S i = 0;
    D32S iRet = ADM_ECODE_SUCCESSED;
    json_object *pEcuArrayObject = NULL;
    json_object *pSingleEcuObject = NULL;
    DCHAR acEcuKey[ECU_INFO_KV_KEY_LEN] = {0};
    DCHAR *pcCacheStr = NULL;
    ADM_EIM_ECU_IDEN_INFO_T stEcuIden = {0};
    DBOOL bIsFind = DFALSE;
    ADM_ECS_ECUCONFIG_T *pstPartEcuOneCfg = NULL;

    ADM_COM_FUNC_ENTRY();

    pcCacheStr = ecuCacheListGetFromKV();
    if (ADM_IS_NULL(pcCacheStr))
    {
        log_e("pcCacheStr is NULL");
        return;
    }

    pEcuArrayObject = json_tokener_parse(pcCacheStr);

    /** 判断json类型是否为数组 */
    if (json_object_get_type(pEcuArrayObject) != json_type_array)
    {
        log_e("json_object_get_type failed");
        return;
    }

    /** 循环解析json数组 */
    for (i = 0; i < json_object_array_length(pEcuArrayObject); i++)
    {
        pSingleEcuObject = json_object_array_get_idx(pEcuArrayObject, i);
        stEcuIden.pcRequestId = adm_com_getJsonStrVal(pSingleEcuObject, "requestId");
        stEcuIden.pcSwDid = adm_com_getJsonStrVal(pSingleEcuObject, "swDid");

        for (D32S iIndex = 0; iIndex < gpstRsCfgInfo->stPartCfg.stEcuCfgList.num; ++iIndex)
        {
            pstPartEcuOneCfg = &(gpstRsCfgInfo->stPartCfg.stEcuCfgList.pstEcuCfg[iIndex]);

            if (ADM_IS_NULL(pstPartEcuOneCfg))
            {
                log_w("pstPartEcuOneCfg index[%d] is NULL", iIndex);
                continue;
            }

            if (0 == adm_strcmp(pstPartEcuOneCfg->ecuReqId, stEcuIden.pcRequestId) && 0 == adm_strcmp(pstPartEcuOneCfg->ecuSwDid, stEcuIden.pcSwDid))
            {
                bIsFind = DTRUE;
                break;
            }
        }

        if (DFALSE == bIsFind)
        {
            adm_memzero(acEcuKey, ECU_INFO_KV_KEY_LEN);
            (void)adm_snprintf(acEcuKey, ECU_INFO_KV_KEY_LEN, ECU_INFO_KV_KEY_TXT, stEcuIden.pcRequestId, stEcuIden.pcSwDid);

            log_i("Key:%s can not find in vehicle config.", acEcuKey);

            if (DFALSE != adm_com_kvstorage_has_key(adm_core_service_getBusinessDatabaseDb(), acEcuKey))
            {
                if (FALSE == adm_com_kvstorage_remove_key(adm_core_service_getBusinessDatabaseDb(), acEcuKey))
                {
                    log_e("RemoveKey[%s] return failed", acEcuKey);
                }
                else
                {
                    log_i("RemoveKey[%s] return success", acEcuKey);
                }
            }
        }

        bIsFind = DFALSE;

        adm_free(stEcuIden.pcRequestId);
        adm_free(stEcuIden.pcSwDid);
    }

    /** 内存释放 */
    adm_free(pcCacheStr);

    ADM_COM_FUNC_EXIT();

    return;
}

static DVOID eimCreateSuccessEcuInfo()
{
    g_aEcuCollectSuccessRecord.uiIndex = 0;
    pthread_mutex_init(&g_aEcuCollectSuccessRecord.ecuMutex, DNULL);

    for (D32U i = 0; i < MAX_COLLECT_ECU_NUM; i++)
    {
        g_aEcuCollectSuccessRecord.aEcuCollectSuccessRecord[i].ecuReqId = NULL;
        g_aEcuCollectSuccessRecord.aEcuCollectSuccessRecord[i].ecuSwDid = NULL;
    }

    return;
}

static DVOID eimFreeSuccessEcuInfo()
{
    for (D32U i = 0; i < g_aEcuCollectSuccessRecord.uiIndex; i++)
    {
        adm_free(g_aEcuCollectSuccessRecord.aEcuCollectSuccessRecord[i].ecuReqId);
        adm_free(g_aEcuCollectSuccessRecord.aEcuCollectSuccessRecord[i].ecuSwDid);
    }

    g_aEcuCollectSuccessRecord.uiIndex = 0;
    pthread_mutex_destroy(&g_aEcuCollectSuccessRecord.ecuMutex);

    return;
}

static DBOOL eimGetSuccessEcuInfo(const DCHAR* pEcuReqId, const DCHAR* pEcuSwDid)
{
    DBOOL bIsFind = DFALSE;

    pthread_mutex_lock(&g_aEcuCollectSuccessRecord.ecuMutex);

    for (D32U i = 0; i < g_aEcuCollectSuccessRecord.uiIndex; i++)
    {
        if (NULL == g_aEcuCollectSuccessRecord.aEcuCollectSuccessRecord[i].ecuReqId)
        {
            continue;
        }

        if (0 == adm_strcmp(pEcuReqId, g_aEcuCollectSuccessRecord.aEcuCollectSuccessRecord[i].ecuReqId) &&
            0 == adm_strcmp(pEcuSwDid, g_aEcuCollectSuccessRecord.aEcuCollectSuccessRecord[i].ecuSwDid))
        {
            bIsFind = DTRUE;
            break;
        }
    }

    pthread_mutex_unlock(&g_aEcuCollectSuccessRecord.ecuMutex);

    return bIsFind;
}

static DVOID eimInsertSuccessEcuInfo(const DCHAR *pEcuReqId, const DCHAR *pEcuSwDid)
{
    pthread_mutex_lock(&g_aEcuCollectSuccessRecord.ecuMutex);

    g_aEcuCollectSuccessRecord.aEcuCollectSuccessRecord[g_aEcuCollectSuccessRecord.uiIndex].ecuReqId = adm_strdup(pEcuReqId);
    g_aEcuCollectSuccessRecord.aEcuCollectSuccessRecord[g_aEcuCollectSuccessRecord.uiIndex].ecuSwDid = adm_strdup(pEcuSwDid);

    g_aEcuCollectSuccessRecord.uiIndex++;
    pthread_mutex_unlock(&g_aEcuCollectSuccessRecord.ecuMutex);

    return;
}

//比较函数,升级组值越小优先级越高,升级顺序值越小优先级越高,            升序排序
DBOOL custom_sort(const ADM_ECU_SINGLE_INFO_T* st1, const ADM_ECU_SINGLE_INFO_T* st2)
{
    if(st1->iUpGroup != st2->iUpGroup)
    {
        return st1->iUpGroup < st2->iUpGroup;
    }
    else
    {
        return st1->iPriority < st2->iPriority;
    }
}

//转换升级char* pUpOrder to int
static D32S parallelCollect_str2Int(DCHAR* pStrNum)
{
    if(ADM_IS_NULL(pStrNum))
    {
        log_e("pStrNum is NULL");
        return 0;
    }

    return atoi(pStrNum);
}
static DVOID parallelCollect_uporder_trans(ADM_ECU_SINGLE_INFO_T* pstEcuSingle)
{
    DCHAR aCh[3] = {0, 0, 0};

    if(ADM_IS_NULL(pstEcuSingle) || ADM_IS_NULL(pstEcuSingle->acUpOrder))
    {
        log_e("pstEcuSingle is NULL");
        return;
    }

    if((('0' == pstEcuSingle->acUpOrder[0]) && ('x' == pstEcuSingle->acUpOrder[1])) ||
            (('0' == pstEcuSingle->acUpOrder[0]) && ('X' == pstEcuSingle->acUpOrder[1])))
    {
        //取串行升级组AB
        aCh[0] = pstEcuSingle->acUpOrder[2];
        aCh[1] = pstEcuSingle->acUpOrder[3];
        pstEcuSingle->iUpGroup = parallelCollect_str2Int(aCh);
        //log_i("RequestId[%s], UpGroup = %d", pstEcuSingle->pRequestId, pstEcuSingle->iUpGroup);

        //取并行零件组CD
        aCh[0] = pstEcuSingle->acUpOrder[4];
        aCh[1] = pstEcuSingle->acUpOrder[5];
        pstEcuSingle->iPartGroup = parallelCollect_str2Int(aCh);
        //log_i("RequestId[%s], PartGroup = %d", pstEcuSingle->pRequestId, pstEcuSingle->iPartGroup);

        //取零件优先级EF
        aCh[0] = pstEcuSingle->acUpOrder[6];
        aCh[1] = pstEcuSingle->acUpOrder[7];
        pstEcuSingle->iPriority = parallelCollect_str2Int(aCh);
        //log_i("RequestId[%s], Priority = %d", pstEcuSingle->pRequestId, pstEcuSingle->iPriority);
    }
    else
    {
        //取串行升级组AB
        aCh[0] = pstEcuSingle->acUpOrder[0];
        aCh[1] = pstEcuSingle->acUpOrder[1];
        pstEcuSingle->iUpGroup = parallelCollect_str2Int(aCh);

        //取并行零件组CD
        aCh[0] = pstEcuSingle->acUpOrder[2];
        aCh[1] = pstEcuSingle->acUpOrder[3];
        pstEcuSingle->iPartGroup = parallelCollect_str2Int(aCh);

        //取零件优先级EF
        aCh[0] = pstEcuSingle->acUpOrder[4];
        aCh[1] = pstEcuSingle->acUpOrder[5];
        pstEcuSingle->iPriority = parallelCollect_str2Int(aCh);
    }

    return;
}

//异步采集ecu
static D32S clearOneEcuDidList(ADM_DIM_SOA_DID_DATA_ARRAY_T* pstDidList)
{
    ADM_DIM_SOA_DID_DATA_T* pstDidData = DNULL;

    if(ADM_IS_NULL(pstDidList))
    {
        return ADM_ECODE_SUCCESSED;
    }

    if(ADM_IS_NULL(pstDidList->didArray))
    {
        //不释放 pstDidList
        return ADM_ECODE_SUCCESSED;
    }

    for(D16U index = 0; index < pstDidList->didArrayLength; ++index)
    {
        pstDidData = pstDidList->didArray[index];

        if(ADM_IS_NULL(pstDidData))
        {
            continue;
        }

        adm_free(pstDidData->didData);
        adm_free(pstDidList->didArray[index]);
    }

    adm_free(pstDidList->didArray);

    return ADM_ECODE_SUCCESSED;
}
static D32S fileOneEcuDidList(ADM_ECS_ECUCONFIG_T* pstPartEcuOneCfg, ADM_DIM_SOA_DID_DATA_ARRAY_T* pstDidList)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    D32S iEcuDidNum = 0;

    do
    {
        if(ADM_IS_NULL(pstPartEcuOneCfg) || ADM_IS_NULL(pstDidList))
        {
            log_e("input param is NULL");
            iRet = ADM_ECODE_INVALID_PARAM;
            break;
        }

        iEcuDidNum = pstPartEcuOneCfg->stEcuDidList.num;
        log_i("ecuReqId[%s], ecuSwDid[%s], iEcuDidNum=%d", pstPartEcuOneCfg->ecuReqId, pstPartEcuOneCfg->ecuSwDid, iEcuDidNum);

        pstDidList->didArrayLength = (D16U)iEcuDidNum;
        pstDidList->didArray = (ADM_DIM_SOA_DID_DATA_T**)adm_malloc(
                                   sizeof(ADM_DIM_SOA_DID_DATA_T*) * pstDidList->didArrayLength);

        for(D16U index = 0; index < pstDidList->didArrayLength; ++index)
        {
            pstDidList->didArray[index] = (ADM_DIM_SOA_DID_DATA_T*)adm_malloc(sizeof(ADM_DIM_SOA_DID_DATA_T));

            pstDidList->didArray[index]->did = adapterHtoI(pstPartEcuOneCfg->stEcuDidList.pstEcuDidInfo[index].did);
            pstDidList->didArray[index]->startByte = (D16U)pstPartEcuOneCfg->stEcuDidList.pstEcuDidInfo[index].start;
            pstDidList->didArray[index]->endByte = (D16U)pstPartEcuOneCfg->stEcuDidList.pstEcuDidInfo[index].end;

            pstDidList->didArray[index]->didData = (ADM_DIM_SOA_DATA_T*)adm_malloc(sizeof(ADM_DIM_SOA_DATA_T));
            pstDidList->didArray[index]->didData->dataType = (D16U)pstPartEcuOneCfg->stEcuDidList.pstEcuDidInfo[index].didValType;
        }
    }
    while(false);

    return iRet;
}

static DBOOL isExistInParallelCollectGroup(ADM_ECU_SINGLE_INFO_T* pstSingleEcu)
{
    DBOOL bRet = DFALSE;

    if(ADM_IS_NULL(pstSingleEcu))
    {
        log_w("pstSingleEcu is NULL");
        return DTRUE;
    }

    for(ADM_ECU_SINGLE_INFO_T* pstEcuSingle : stEcuParallelCollectCtl.collectEcuListM)
    {
        if(ADM_IS_NULL(pstEcuSingle))
        {
            log_w("pstEcuSingle is NULL");
            continue;
        }

        if((0 == adm_strcmp(pstSingleEcu->acRequestId, pstEcuSingle->acRequestId)) &&
           (0 == adm_strcmp(pstSingleEcu->acSwDid, pstEcuSingle->acSwDid)) &&
            0 == adm_strcasecmp(pstSingleEcu->acUpOrder, pstEcuSingle->acUpOrder))
        {
            log_w("RequestId[%s], SwDid[%s], upOrder[%s] is exist in ParallelCollectGroup", pstSingleEcu->acRequestId, 
                                                                                            pstSingleEcu->acSwDid,
                                                                                            pstSingleEcu->acUpOrder);
            bRet = DTRUE;
            break;
        }
    }

    return bRet;
}

static DBOOL isExistOtherParallelGroup(ADM_ECU_SINGLE_INFO_T* pstSingleEcu)
{
    DBOOL bIsExist = FALSE;

    if(ADM_IS_NULL(pstSingleEcu))
    {
        log_w("pstSingleEcu is NULL");
        return DTRUE;
    }

    for(ADM_ECU_SINGLE_INFO_T* pstEcuSingle : stEcuParallelCollectCtl.collectEcuListM)
    {
        if(pstSingleEcu->iUpGroup != pstEcuSingle->iUpGroup)
        {
            bIsExist = TRUE;
            log_i("input:RequestId[%s],SwDid[%s],UpGroup[%d]. collectGroup:RequestId[%s],SwDid[%s],UpGroup[%d]",
                  pstSingleEcu->acRequestId, pstSingleEcu->acSwDid, pstSingleEcu->iUpGroup,
                  pstEcuSingle->acRequestId, pstEcuSingle->acSwDid, pstEcuSingle->iUpGroup);
            break;
        }
    }

    return bIsExist;
}

static DBOOL isMeetInParallelCollectGroup(ADM_ECU_SINGLE_INFO_T* pstSingleEcu)
{
    DBOOL bRet = DFALSE;
    D32U uiCuontN = 0;
    D32U uiLimitN = 0;

    if(ADM_IS_NULL(pstSingleEcu))
    {
        log_w("pstSingleEcu is NULL");
        return DTRUE;
    }

    uiLimitN = adm_cfg_iniParserGetInt(ADM_CFG_VUS_UPGRADE_PARALLEL_LIMIT_N, ADM_CFG_VUS_UPGRADE_PARALLEL_LIMIT_N_DEFAULT_VAL);
    log_d("uiLimitN=%d", uiLimitN);

    for(ADM_ECU_SINGLE_INFO_T* pstEcuSingle : stEcuParallelCollectCtl.collectEcuListM)
    {
        if(pstSingleEcu->iPartGroup == pstEcuSingle->iPartGroup)
        {
            uiCuontN++;
            log_i("input:RequestId[%s],SwDid[%s],PartGroup[%d]. collectGroup:RequestId[%s],SwDid[%s],PartGroup[%d]",
                  pstSingleEcu->acRequestId, pstSingleEcu->acSwDid, pstSingleEcu->iPartGroup,
                  pstEcuSingle->acRequestId, pstEcuSingle->acSwDid, pstEcuSingle->iPartGroup);
        }

        if(uiLimitN <= uiCuontN)
        {
            bRet = DTRUE;
            break;
        }
    }

    return bRet;
}

/**
 * @brief 获取一个ecu配置信息
 * @note
 * @param in ADM_ECU_SINGLE_INFO_T* pstEcuSingle ecu配置信息
 * @retval ADM_ECU_SINGLE_INFO_T*
 */
static ADM_ECU_SINGLE_INFO_T* getOneEcuCfgCtl(ADM_ECU_SINGLE_INFO_T* pstEcuSingle)
{
    ADM_ECU_SINGLE_INFO_T* pstOneEcuCfg = DNULL;

    ADM_COM_FUNC_ENTRY();

    /** 入参检查 */
    if(ADM_IS_NULL(pstEcuSingle))
    {
        log_e("invalid param");
        return pstOneEcuCfg;
    }

    do
    {
        pthread_mutex_lock(&stEcuCfgCtl.ecuCfgMutex);

        for(auto iter = stEcuCfgCtl.listEcuSingleInfo.begin(); iter != stEcuCfgCtl.listEcuSingleInfo.end(); ++iter)
        {
            pstOneEcuCfg = *iter;

            if(ADM_IS_NULL(pstOneEcuCfg))
            {
                log_e("invalid pointer");
                continue;
            }

            if (adapterHtoI(pstEcuSingle->acRequestId) == adapterHtoI(pstOneEcuCfg->acRequestId) &&
                adapterHtoI(pstEcuSingle->acSwDid) == adapterHtoI(pstOneEcuCfg->acSwDid) &&
                ADM_ECU_SINGLE_COLLECT_ING == pstOneEcuCfg->eCollectState)
            {
                log_d("RequestId[%s],SwDid[%s] is exist in EcuCfgCtl", pstEcuSingle->acRequestId, pstEcuSingle->acSwDid);
                break;
            }

            pstOneEcuCfg = DNULL;
        }

        pthread_mutex_unlock(&stEcuCfgCtl.ecuCfgMutex);
    }while(FALSE);

    ADM_COM_FUNC_EXIT();

    return pstOneEcuCfg;
}

static ADM_ECU_SINGLE_INFO_T* getCollectingEcuCfgCtl(ADM_ECU_SINGLE_INFO_T* pstEcuSingle)
{
    ADM_ECU_SINGLE_INFO_T* pstOneEcuCfg = DNULL;

    ADM_COM_FUNC_ENTRY();

    /** 入参检查 */
    if(ADM_IS_NULL(pstEcuSingle))
    {
        log_e("invalid param");
        return pstOneEcuCfg;
    }

    do
    {
        pthread_mutex_lock(&stEcuCfgCtl.ecuCfgMutex);

        for(auto iter = stEcuCfgCtl.listEcuSingleInfo.begin(); iter != stEcuCfgCtl.listEcuSingleInfo.end(); ++iter)
        {
            pstOneEcuCfg = *iter;

            if(ADM_IS_NULL(pstOneEcuCfg))
            {
                log_e("invalid pointer");
                continue;
            }

            if (adapterHtoI(pstEcuSingle->acRequestId) == adapterHtoI(pstOneEcuCfg->acRequestId) &&
                adapterHtoI(pstEcuSingle->acSwDid) == adapterHtoI(pstOneEcuCfg->acSwDid) &&
                0 == adm_strcmp(pstEcuSingle->acUpOrder, pstOneEcuCfg->acUpOrder) &&
                ADM_ECU_SINGLE_COLLECT_ING == pstOneEcuCfg->eCollectState)
            {
                log_d("RequestId[%s],SwDid[%s],acUpOrder[%s] is exist in EcuCfgCtl", pstEcuSingle->acRequestId, pstEcuSingle->acSwDid, pstEcuSingle->acUpOrder);
                break;
            }

            pstOneEcuCfg = DNULL;
        }

        pthread_mutex_unlock(&stEcuCfgCtl.ecuCfgMutex);
    }while(FALSE);

    ADM_COM_FUNC_EXIT();

    return pstOneEcuCfg;
}

static ADM_ECU_SINGLE_INFO_T* getParallelCollectOneEcu(D32U &uiNum)
{
    D32U uiCount = 0;  //待采集计数
    ADM_ECU_SINGLE_INFO_T* pstSingleEcu = NULL;

    if(ADM_ECU_ALL_COLLECT_FINISH == stEcuCfgCtl.eCollectAllState)
    {
        log_i("collect finish.");
        uiNum = uiCount;
        return pstSingleEcu;
    }

    for(auto iter = stEcuCfgCtl.listEcuSingleInfo.begin(); iter != stEcuCfgCtl.listEcuSingleInfo.end(); ++iter)
    {
        pstSingleEcu = *iter;

        if(ADM_IS_NULL(pstSingleEcu))
        {
            log_w("pstSingleEcu is NULL");
            continue;
        }

        /** 跳过获取成功ECU */
        if(ADM_ECU_SINGLE_COLLECT_SUCC == pstSingleEcu->eCollectState)
        {
            log_w("RequestId[%s],SwDid[%s] ecu collect succeed", pstSingleEcu->acRequestId, pstSingleEcu->acSwDid);
            continue;
        }

        /** 跳过采集次数超过限制的ECU */
        if(uiCollectCount <= pstSingleEcu->uiCollectCount)
        {
            log_w("RequestId[%s],SwDid[%s] ecu collect count[%d]",
                  pstSingleEcu->acRequestId, pstSingleEcu->acSwDid, pstSingleEcu->uiCollectCount);
            continue;
        }

        /** 跳过已存在采集组的ECU */
        if(DTRUE == isExistInParallelCollectGroup(pstSingleEcu))
        {
            log_w("RequestId[%s],SwDid[%s], upOrder[%s] is exist ParallelCollectGroup", pstSingleEcu->acRequestId, 
                                                                                        pstSingleEcu->acSwDid,
                                                                                        pstSingleEcu->acUpOrder);
            continue;
        }

        /** 判断是否存在其他采集组的ECU */
        if (DTRUE == isExistOtherParallelGroup(pstSingleEcu))
        {
            log_w("RequestId[%s],SwDid[%s], upOrder[%s] exist other upGroup  ParallelCollectGroup", pstSingleEcu->acRequestId, 
                                                                                                    pstSingleEcu->acSwDid,
                                                                                                    pstSingleEcu->acUpOrder);
            continue;
        }

        /** 跳过未达到并行采集条件的ECU */
        if(DTRUE == isMeetInParallelCollectGroup(pstSingleEcu))
        {
            log_w("RequestId[%s],SwDid[%s], upOrder[%s] beyond uiLimitN  ParallelCollectGroup", pstSingleEcu->acRequestId, 
                                                                                                pstSingleEcu->acSwDid,
                                                                                                pstSingleEcu->acUpOrder);
            continue;
        }

        //满足条件,加入采集组
        uiCount++;
        break;
    }

    uiNum = uiCount;
    return pstSingleEcu;
}
static DBOOL updateParallelCollectGroup(DVOID)
{
    D32U uiCollectCount = 0;
    ADM_ECU_SINGLE_INFO_T* pstSingleEcu = NULL;
    DBOOL bUpdate = DFALSE;
    D32U uiLimitM = 0;

    uiLimitM = adm_cfg_iniParserGetInt(ADM_CFG_VUS_UPGRADE_PARALLEL_LIMIT_M, ADM_CFG_VUS_UPGRADE_PARALLEL_LIMIT_M_DEFAULT_VAL);
    log_d("uiLimitM=%u", uiLimitM);

    while(true)
    {
        log_i("updateParallelCollectGroup::collectEcuListM.size()=%u",
              stEcuParallelCollectCtl.collectEcuListM.size());

        if(stEcuParallelCollectCtl.collectEcuListM.size() >= uiLimitM)
        {
            log_w("updateParallelCollectGroup::collectEcuListM.size()=%u",
                  stEcuParallelCollectCtl.collectEcuListM.size());
            break;
        }

        pthread_mutex_lock(&stEcuParallelCollectCtl.collectMutex);
        pstSingleEcu = getParallelCollectOneEcu(uiCollectCount);
        log_i("uiCollectCount=%u", uiCollectCount);

        if(0 == uiCollectCount)
        {
            pthread_mutex_unlock(&stEcuParallelCollectCtl.collectMutex);
            break;
        }

        stEcuParallelCollectCtl.collectEcuListM.push_back(pstSingleEcu);
        pthread_mutex_unlock(&stEcuParallelCollectCtl.collectMutex);
        bUpdate = DTRUE;
        uiCollectCount = 0;
    }

    pthread_mutex_lock(&stEcuParallelCollectCtl.collectMutex);

    for(ADM_ECU_SINGLE_INFO_T* pstEcuSingleTest : stEcuParallelCollectCtl.collectEcuListM)
    {
        log_d("RequestId[%s], SwDid[%s], UpOrder[%s]",
              pstEcuSingleTest->acRequestId, pstEcuSingleTest->acSwDid, pstEcuSingleTest->acUpOrder);
    }

    pthread_mutex_unlock(&stEcuParallelCollectCtl.collectMutex);

    //如果update触发
    //如果最后一次通知回来前sem_wait需要再触发正常结束
    if(bUpdate || 0 == stEcuParallelCollectCtl.collectEcuListM.size())
    {
        //触发采集组
        sem_post(&stEcuParallelCollectCtl.collectSig);
    }

    return bUpdate;
}
static D32S delMemberParallelCollectGroup(ADM_ECU_SINGLE_INFO_T* stEcuSingle)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    ADM_ECU_SINGLE_INFO_T* pstEcuSingleInfo = NULL;

    if(ADM_IS_NULL(stEcuSingle))
    {
        log_e("stEcuSingle is NULL");
        return ADM_ECODE_INVALID_PARAM;
    }

    pthread_mutex_lock(&stEcuParallelCollectCtl.collectMutex);

    for(auto iter = stEcuParallelCollectCtl.collectEcuListM.begin();
            iter != stEcuParallelCollectCtl.collectEcuListM.end();)
    {
        pstEcuSingleInfo = *iter;

        if(ADM_IS_NULL(pstEcuSingleInfo))
        {
            log_w("pstEcuSingleInfo is NULL");
            ++iter;
            continue;
        }

        if (adapterHtoI(stEcuSingle->acRequestId) == adapterHtoI(pstEcuSingleInfo->acRequestId) &&
            adapterHtoI(stEcuSingle->acSwDid) == adapterHtoI(pstEcuSingleInfo->acSwDid) &&
            0 == adm_strcasecmp(stEcuSingle->acUpOrder, pstEcuSingleInfo->acUpOrder))
        {
            log_i("stEcuParallelCollectCtl erase RequestId[%s], SwDid[%s], upOrder[%s]", pstEcuSingleInfo->acRequestId, 
                                                                                         pstEcuSingleInfo->acSwDid,
                                                                                         pstEcuSingleInfo->acUpOrder);
            iter = stEcuParallelCollectCtl.collectEcuListM.erase(iter);
            break;
        }
        else 
        {
            ++iter;
        }
    }

    pthread_mutex_unlock(&stEcuParallelCollectCtl.collectMutex);

    return iRet;
}

static void parallelCollectGroupDestroy(void)
{
    ADM_ECU_SINGLE_INFO_T* pstEcuSingleInfo = NULL;

    log_i("begin Clear collectEcuListM size[%u]", stEcuParallelCollectCtl.collectEcuListM.size());

    pthread_mutex_lock(&stEcuParallelCollectCtl.collectMutex);

    for(auto iter = stEcuParallelCollectCtl.collectEcuListM.begin();
            iter != stEcuParallelCollectCtl.collectEcuListM.end();)
    {
        pstEcuSingleInfo = *iter;

        if(ADM_IS_NULL(pstEcuSingleInfo))
        {
            log_w("pstEcuSingleInfo is NULL");
            ++iter;
            continue;
        }

        iter = stEcuParallelCollectCtl.collectEcuListM.erase(iter);
    }
    log_i("end Clear collectEcuListM size[%u]", stEcuParallelCollectCtl.collectEcuListM.size());

    pthread_mutex_unlock(&stEcuParallelCollectCtl.collectMutex);

    return ;
}

//配置删除无调
static D32S delMemberParallelCollectCfg(ADM_ECU_SINGLE_INFO_T* pstEcuOneCfg)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    ADM_ECU_SINGLE_INFO_T* pstEcuSingleInfo = NULL;

    if(ADM_IS_NULL(pstEcuOneCfg))
    {
        log_e("pstEcuOneCfg is NULL");
        return ADM_ECODE_INVALID_PARAM;
    }

    pthread_mutex_lock(&stEcuCfgCtl.ecuCfgMutex);

    for(auto iter = stEcuCfgCtl.listEcuSingleInfo.begin(); iter != stEcuCfgCtl.listEcuSingleInfo.end();)
    {
        pstEcuSingleInfo = *iter;

        if(ADM_IS_NULL(pstEcuSingleInfo))
        {
            log_w("pstEcuSingleInfo is NULL");
            ++iter;
            continue;
        }

        if (adapterHtoI(pstEcuOneCfg->acRequestId) == adapterHtoI(pstEcuSingleInfo->acRequestId) &&
            adapterHtoI(pstEcuOneCfg->acSwDid) == adapterHtoI(pstEcuSingleInfo->acSwDid) &&
            0 == adm_strcasecmp(pstEcuOneCfg->acUpOrder, pstEcuSingleInfo->acUpOrder))
        {
            log_i("delMemberParallelCollectCfg erase RequestId[%s], SwDid[%s], UpOrder[%s] ", pstEcuSingleInfo->acRequestId, 
                                                                                              pstEcuSingleInfo->acSwDid,
                                                                                              pstEcuSingleInfo->acUpOrder);

            iter = stEcuCfgCtl.listEcuSingleInfo.erase(iter);

            break;
        }
        else 
        {
            ++iter;
        }
    }

    pthread_mutex_unlock(&stEcuCfgCtl.ecuCfgMutex);

    return iRet;
}



static D32S updateSingleEcuCfg(ADM_ECU_SINGLE_INFO_T* pstSingleEcu)
{
    ADM_ECU_SINGLE_INFO_T* pstEcuSingleInfo = NULL;

    for(auto iter = stEcuCfgCtl.listEcuSingleInfo.begin(); iter != stEcuCfgCtl.listEcuSingleInfo.end(); ++iter)
    {
        pstEcuSingleInfo = *iter;

        if(ADM_IS_NULL(pstEcuSingleInfo))
        {
            log_w("pstEcuSingleInfo is NULL");
            continue;
        }

        if (adapterHtoI(pstSingleEcu->acRequestId) == adapterHtoI(pstEcuSingleInfo->acRequestId) &&
            adapterHtoI(pstSingleEcu->acSwDid) == adapterHtoI(pstEcuSingleInfo->acSwDid) &&
            0 == adm_strcasecmp(pstSingleEcu->acUpOrder, pstEcuSingleInfo->acUpOrder))
        {
            pstEcuSingleInfo->eCollectState = pstSingleEcu->eCollectState;
            pstEcuSingleInfo->uiCollectCount = pstSingleEcu->uiCollectCount;
            log_i("RequestId[%s],SwDid[%s], eCollectState[%d], uiCollectCount[%d], acUpOrder[%s]", pstEcuSingleInfo->acRequestId, 
                                                                                                   pstEcuSingleInfo->acSwDid,
                                                                                                   pstEcuSingleInfo->eCollectState, 
                                                                                                   pstEcuSingleInfo->uiCollectCount,
                                                                                                   pstEcuSingleInfo->acUpOrder);
            break;
        }
    }

    return ADM_ECODE_SUCCESSED;
}

/**
 * @brief try to begin key field retry round for ECUs that need retry
 * 尝试开启一轮key字段重试循环,DTURE表示有需要重试的ECU, FALSE表示没有需要重试的ECU
 * @note 需要在stEcuCfgCtl.ecuCfgMutex锁保护下调用
 */
static DBOOL eimTryBeginKeyFieldRetryRoundLocked(void)
{
    ADM_ECU_SINGLE_INFO_T* pstEcu = NULL;
    DBOOL bAny = DFALSE;
    D32U uiMaxRetry = (D32U)adm_cfg_iniParserGetInt(
        ADM_CFG_VUS_EIM_KEY_FIELD_COLLECT_RETRY_MAX, ADM_CFG_VUS_EIM_KEY_FIELD_COLLECT_RETRY_MAX_DEFAULT_VAL);

    if (0U == uiMaxRetry)
    {
        return DFALSE;
    }
    //遍历所有ECU,尝试开启重试循环
    for (auto iter = stEcuCfgCtl.listEcuSingleInfo.begin(); iter != stEcuCfgCtl.listEcuSingleInfo.end(); ++iter)
    {
        pstEcu = *iter;
        if (ADM_IS_NULL(pstEcu))
        {
            continue;
        }

        if (ADM_ECU_SINGLE_COLLECT_SUCC == pstEcu->eCollectState)
        {
            continue;
        }

        if (pstEcu->eLastKeyCmpResult == EIM_KEY_CMP_MISMATCH)
        {
            continue;
        }
        //如果有ECU的key字段重试轮数小于最大重试轮数,则开启新一轮重试循环
        if (pstEcu->uiKeyFieldRetryRound < uiMaxRetry)
        {
            pstEcu->uiKeyFieldRetryRound++;
            pstEcu->eLastKeyCmpResult = EIM_KEY_CMP_NONE;
            bAny = DTRUE;
        }
    }

    return bAny;
}

static D32S updateAllEcuStateCfg()
{
    ADM_ECU_SINGLE_INFO_T* pstEcuSingleInfo = NULL;
    DBOOL bAllEcuNeedCollect = DFALSE;

    pthread_mutex_lock(&stEcuCfgCtl.ecuCfgMutex);

    for(auto iter = stEcuCfgCtl.listEcuSingleInfo.begin(); iter != stEcuCfgCtl.listEcuSingleInfo.end(); ++iter)
    {
        pstEcuSingleInfo = *iter;

        if(ADM_IS_NULL(pstEcuSingleInfo))
        {
            log_w("pstEcuSingleInfo is NULL");
            continue;
        }

        if(ADM_ECU_SINGLE_COLLECT_SUCC == pstEcuSingleInfo->eCollectState)
        {
            continue;
        }

        if(uiCollectCount <= pstEcuSingleInfo->uiCollectCount)
        {
            continue;
        }

        //需要继续采集
        bAllEcuNeedCollect = DTRUE;
        break;
    }

    if(!bAllEcuNeedCollect)
    {
        stEcuCfgCtl.eCollectAllState = ADM_ECU_ALL_COLLECT_FINISH;
        log_i("all ecu collect finish,update stEcuCfgCtl.eCollectAllState");
    }

    pthread_mutex_unlock(&stEcuCfgCtl.ecuCfgMutex);
    return 0;
}

static DVOID updateAllEcu2KV()
{
    DCHAR acEcuKey[ECU_INFO_KV_KEY_LEN] = {0};
    DBOOL boolRet = FALSE;

    DCHAR* singleEcuJsonStr = NULL;
    size_t uilength = 0;
    ADM_KV_STORAGE_VALUE_TYPE_E eValType;

    json_object* allEcuObjectArray = NULL;
    json_object* singleEcuObject = NULL;

    allEcuObjectArray = json_object_new_array();
    std::unordered_set<std::string> element_set;

    //循环读取kv中单个零件
    for(D32S index = 0; index < gpstRsCfgInfo->stPartCfg.stEcuCfgList.num; ++index)
    {
        ADM_ECS_ECUCONFIG_T* pstEcuCfg = &(gpstRsCfgInfo->stPartCfg.stEcuCfgList.pstEcuCfg[index]);

        if(ADM_IS_NULL(pstEcuCfg))
        {
            continue;
        }

        (void)adm_snprintf(acEcuKey, ECU_INFO_KV_KEY_LEN, ECU_INFO_KV_KEY_TXT, pstEcuCfg->ecuReqId, pstEcuCfg->ecuSwDid);

        if (element_set.find(acEcuKey) != element_set.end())
        {
            log_i("key = %s has already insert.", acEcuKey);
            continue;
        }

        boolRet = adm_com_kvstorage_get_string(adm_core_service_getBusinessDatabaseDb(),
                                               acEcuKey,
                                               &singleEcuJsonStr,
                                               &uilength,
                                               &eValType);

        if(FALSE == boolRet)
        {
            log_e("get single ecuInfo[%s] string return failed", acEcuKey);
            continue;
        }

        log_i("key = %s ,singleEcuJsonStr = %s", acEcuKey, singleEcuJsonStr);
        singleEcuObject = json_tokener_parse(singleEcuJsonStr);

        adm_free(singleEcuJsonStr);
        singleEcuJsonStr = NULL;

        if(ADM_IS_NULL(singleEcuObject))
        {
            log_e("json_tokener_parse return failed");
            continue;
        }

        element_set.insert(acEcuKey);

        (void)json_object_array_add(allEcuObjectArray, singleEcuObject);

        /** 缓存ECU信息 */
        ecuCacheInfoNodeAdd(pstEcuCfg);
    }

    /** ECU信息列表缓存到KV */
    ecuCacheListSetToKV();
    
    //所有零件存KV
    log_i("set all ecu info to KV");

    if(0U != json_object_array_length(allEcuObjectArray))
    {
        DCHAR* allEcuObjectArrayStr = (DCHAR*)json_object_to_json_string(allEcuObjectArray);  //shallow cp
        log_i("key = %s ,allEcuInfo = %s", ALL_ECU_INFO_KEY_TXT, allEcuObjectArrayStr);
        boolRet = adm_com_kvstorage_set_string(
                      adm_core_service_getBusinessDatabaseDb(), ALL_ECU_INFO_KEY_TXT, allEcuObjectArrayStr, ADM_KV_STORAGE_VALUE_TYPE_STRING);

        if(FALSE == boolRet)
        {
            log_e("set all ecu info string return failed");
        }

        ///< 设置时间戳
        time_t iNow;
        time(&iNow);
        DCHAR aTimestamp[11] = {0};
        (void)adm_memset(aTimestamp, 0x00, sizeof(aTimestamp), sizeof(aTimestamp));
        (void)adm_snprintf(aTimestamp, sizeof(aTimestamp), "%lld", iNow);
        boolRet = adm_com_kvstorage_set_string(
            adm_core_service_getBusinessDatabaseDb(), EIM_SET_ECU_TIMESTAMP, aTimestamp, ADM_KV_STORAGE_VALUE_TYPE_INT64);
        if (FALSE == boolRet)
        {
            log_e("set eim timestamp return failed");
        }
    }
    else
    {
        log_w("ecu info no cache");
    }

    (void)json_object_put(allEcuObjectArray);

    return;
}

/**
 * @brief 非工厂模式、OBD未占用、电源状态为ON才满足条件
 * 
 * @param[in] pstRespMsgData     描述: 信号集合 \n
 * 范围值: 指针 \n
 * 单位: N/A \n
 * 
 * @return DBOOL 
 */
static DBOOL eimParsePreCondition(ADM_DIM_SOA_MSGID_DATA_ARRAY_T* pstRespMsgData)
{
    D32S iValue = -1;
    D32S iRet = ADM_ECODE_FAILED;
    DBOOL bIsNeedBreak = DFALSE;
    DBOOL bIsStatisfy = DTRUE;

    if (NULL != pstRespMsgData && NULL != pstRespMsgData->msgIdArray)
    {
        if (pstRespMsgData->msgIdArrayLength != ADM_BACK_COLLECT_COND_NUM)
        {
            log_e("Cond number not fit.expect num[%d], real num[%d]", ADM_BACK_COLLECT_COND_NUM, 
                pstRespMsgData->msgIdArrayLength);
            bIsStatisfy = DFALSE;
        }
        else 
        {
            for (D16U i = 0; i < pstRespMsgData->msgIdArrayLength; i++)
            {
                if (NULL != pstRespMsgData->msgIdArray[i] && NULL != pstRespMsgData->msgIdArray[i]->msgIdData &&
                    VAR_S32 == pstRespMsgData->msgIdArray[i]->msgIdData->dataType)
                {
                    iValue = pstRespMsgData->msgIdArray[i]->msgIdData->v.valueS32;

                    switch (pstRespMsgData->msgIdArray[i]->msgId)
                    {
                        case VEHICLE_STATUS_POWER_SIGNAL:
                        if (1 != iValue)    ///< 非ON
                        {
                            bIsNeedBreak = DTRUE;
                            bIsStatisfy = DFALSE;
                        }
                        break;

                        case VEHICLE_STATUS_OBD_STATE:
                        if (0 != iValue)    ///< 不是未被占用状态
                        {
                            bIsNeedBreak = DTRUE;
                            bIsStatisfy = DFALSE;
                        }
                        break;

                        case VEHICLE_STATUS_FACTORY_MODE:
                        if (0 != iValue)    ///< 不是非工厂模式
                        {
                            bIsNeedBreak = DTRUE;
                            bIsStatisfy = DFALSE;
                        }
                        break;

                        default:
                        log_i("Can not find type[%d]", pstRespMsgData->msgIdArray[i]->msgId);
                        bIsNeedBreak = DTRUE;
                        bIsStatisfy = DFALSE;
                        break;
                    }
                }

                if (DTRUE == bIsNeedBreak)
                {
                    break;
                }
            }
        }
    }

    return bIsStatisfy;
}

/**
 * @brief 等待前置条件符合
 * 
 * @return D32S 
 */
static D32S eimWaitPreCondition()
{
    ADM_DIM_SOA_MSGID_DATA_ARRAY_T stReqMsgData = {};
    ADM_DIM_SOA_MSGID_DATA_ARRAY_T* pstRespMsgData=NULL;
    D32S iRet = ADM_ECODE_FAILED;
    D32S iErrCode = ADM_ECODE_FAILED;
    DBOOL bIsSatisfy = DFALSE;
    D16U i = 0;

    VEHICLE_STATUS_E aVehicleStatus[ADM_BACK_COLLECT_COND_NUM] = {VEHICLE_STATUS_POWER_SIGNAL, VEHICLE_STATUS_OBD_STATE, 
                    VEHICLE_STATUS_FACTORY_MODE};

    stReqMsgData.msgIdArrayLength = ADM_BACK_COLLECT_COND_NUM;
    stReqMsgData.msgIdArray = (ADM_DIM_SOA_VDI_MSGID_T**)adm_malloc(sizeof(ADM_DIM_SOA_VDI_MSGID_T*) * (DSIZE)stReqMsgData.msgIdArrayLength);
    
    for (i = 0; i < stReqMsgData.msgIdArrayLength; i++)
    {
        stReqMsgData.msgIdArray[i] = (ADM_DIM_SOA_VDI_MSGID_T*)adm_malloc((DSIZE)sizeof(ADM_DIM_SOA_VDI_MSGID_T));
        stReqMsgData.msgIdArray[i]->msgIdData = NULL;
        stReqMsgData.msgIdArray[i]->msgId = aVehicleStatus[i];
    }

    /** 如果2min内条件不满足,则终止本次上电后台零件收集 */
    for (i = 0; i < ADM_BACK_COLLECT_COND_MAX_TIME; i++)
    {
        if (ADM_VSM_STATE_IDLE != adm_vsm_stateGet() || TRUE == g_bPauseBackRuning)
        {
            log_e("upgrade/check doing, not allow back collect.");
            iRet = ADM_ECODE_EIM_STOP_BACK_COLLECT;
            break;
        }

        iRet = adm_dim_host_readVehicleStatus(adm_soa_getHostInstanceId(), 15, &stReqMsgData, &pstRespMsgData, &iErrCode);
        if (ADM_IS_ERR(iRet) || ADM_IS_ERR(iErrCode))
        {
            /** 获取失败尝试重试获取 */
            if (0 == i % 10)
            {
                log_i("adm_dim_host_readVehicleStatus call error.iRet[%d], errCode[%d]", iRet, iErrCode);
            }
            sleep(ADM_BACK_COLLECT_COND_PER_WAIT);
        }
        else 
        {
            bIsSatisfy = eimParsePreCondition(pstRespMsgData);
            if (TRUE == bIsSatisfy)
            {
                break;
            }
            else 
            {
                if (0 == i % 10)
                {
                    log_i("Parse pre condition not satisfied.");
                }
                sleep(ADM_BACK_COLLECT_COND_PER_WAIT);
            }
        }
    }

    if (ADM_BACK_COLLECT_COND_MAX_TIME == i)
    {
        g_bIsStopByCond = TRUE;
    }
    
    if (FALSE == bIsSatisfy)
    {
        iRet = ADM_ECODE_EIM_STOP_BACK_COLLECT;
    }

    for (D16U i = 0; i < pstRespMsgData->msgIdArrayLength; i++)
    {
        adm_free(pstRespMsgData->msgIdArray[i]->msgIdData);
        adm_free(pstRespMsgData->msgIdArray[i]);
    }
    adm_free(pstRespMsgData->msgIdArray);
    adm_free(pstRespMsgData);

    for (D16U i = 0; i < stReqMsgData.msgIdArrayLength; i++)
    {
        adm_free(stReqMsgData.msgIdArray[i]);
    }
    adm_free(stReqMsgData.msgIdArray);

    return iRet;
}

static D32S asyncReqDimParallelCollectOne(ADM_ECU_SINGLE_INFO_T* pstEcuSingle, D32S* pErrorCod)
{
    D32S iRet = ADM_ECODE_FAILED;

    D32S iInstanceId = 0;
    DBOOL bCGWRouted = DFALSE;

    ADM_ECS_ECUCONFIG_T* pstPartEcuOneCfg = DNULL;
    ADM_DIM_SOA_ECU_IDEN_INFO_T stEcuIdenInfo = {};
    ADM_DIM_SOA_DID_DATA_ARRAY_T stDidList = {};

    D32S eventNum = 30;
    DCHAR eventDesc[EVENT_ECU_INFO_PARALLEL_ARRAY_LEN];


    if(ADM_IS_NULL(pstEcuSingle))
    {
        log_e("pstEcuSingle is NULL");
        return ADM_ECODE_FAILED;
    }

    for(D32S iIndex = 0; iIndex < gpstRsCfgInfo->stPartCfg.stEcuCfgList.num; ++iIndex)
    {
        pstPartEcuOneCfg = &(gpstRsCfgInfo->stPartCfg.stEcuCfgList.pstEcuCfg[iIndex]);

        if(ADM_IS_NULL(pstPartEcuOneCfg))
        {
            log_w("pstPartEcuOneCfg index[%d] is NULL", iIndex);
            continue;
        }

        /** 非IDEL/其他零件收集模式中断情况下 不允许后台采集 */
        if (COLLECT_TRIGGER_TYPE_BACK == g_eTriggerType)
        {
            if (ADM_VSM_STATE_IDLE != adm_vsm_stateGet() || TRUE == g_bPauseBackRuning)
            {
                log_e("upgrade/check doing, not allow back collect.");
                iRet = ADM_ECODE_EIM_STOP_BACK_COLLECT;
                break;
            }

            iRet = eimWaitPreCondition();
            if (ADM_IS_ERR(iRet))
            {
                break;
            }
        }

        if(0 == adm_strcmp(pstPartEcuOneCfg->ecuReqId, pstEcuSingle->acRequestId)
                && 0 == adm_strcmp(pstPartEcuOneCfg->ecuSwDid, pstEcuSingle->acSwDid))
        {
            stEcuIdenInfo.requestId = adapterHtoI(pstPartEcuOneCfg->ecuReqId);
            stEcuIdenInfo.responseId = adapterHtoI(pstPartEcuOneCfg->ecuResponseId);
            stEcuIdenInfo.functionId = adapterHtoI(pstPartEcuOneCfg->ecuFunctionId);
            stEcuIdenInfo.swDid = adapterHtoI(pstPartEcuOneCfg->ecuSwDid);

            //file didlist
            iRet = fileOneEcuDidList(pstPartEcuOneCfg, &stDidList);

            if(ADM_IS_ERR(iRet))
            {
                log_e("fileOneEcuDidList failed");
                break;
            }

            iInstanceId = adapterHtoI(pstPartEcuOneCfg->readInfoInstanceId);

            /** 单ECU真正采集前再删除ecu信息 */
            if (DFALSE != adm_com_kvstorage_has_key(adm_core_service_getBusinessDatabaseDb(), pstEcuSingle->acSingleEcuKvKey))
            {
                /** 先删除，再采集 */
                if (FALSE == adm_com_kvstorage_remove_key(adm_core_service_getBusinessDatabaseDb(), pstEcuSingle->acSingleEcuKvKey))
                {
                    log_e("RemoveKey[%s] return failed", pstEcuSingle->acSingleEcuKvKey);
                }
                else
                {
                    log_i("RemoveKey[%s] return success", pstEcuSingle->acSingleEcuKvKey);
                }
            }

            if(1 == pstPartEcuOneCfg->serviceType)
            {
                //表示vdi采集,不再使用通用配置中的BusinessScript诊断脚本,使用json诊断
                if(1 == pstPartEcuOneCfg->isCGWRouted)
                {
                    bCGWRouted = DTRUE;
                }

                (void)adm_memset(eventDesc, 0x00, EVENT_ECU_INFO_PARALLEL_ARRAY_LEN, EVENT_ECU_INFO_PARALLEL_ARRAY_LEN);
                (void)adm_snprintf(eventDesc, EVENT_ECU_INFO_PARALLEL_ARRAY_LEN, "start get ecuInfo from VDI");
                eim_addParallelEcuEventList(
                    g_pstEventsList, (const DCHAR*)"PS_GET_ECUINFO_001", 
                    pstPartEcuOneCfg->ecuReqId, 
                    pstPartEcuOneCfg->ecuSwDid, 
                    eventDesc);
                log_d("%s", eventDesc);

                iRet = adm_dim_vdi_parallelCollectEcu((D16U)iInstanceId,
                                                      SINGLE_READECUINFO_TIMEOUT,
                                                      &stEcuIdenInfo,
                                                      gpstRsCfgInfo->stGeneralCfg.stBusinessScript.filePath,
                                                      (ADM_DIM_SOA_VDI_BUS_TYPE_E)pstPartEcuOneCfg->busType,
                                                      (ADM_DIM_SOA_PIN_CHANNEL_TYPE_E)pstPartEcuOneCfg->channelType,
                                                      bCGWRouted,
                                                      stDidList,
                                                      pErrorCod);

                if (ADM_ECODE_SUCCESSED != iRet) {
                    (void)adm_memset(eventDesc, 0x00, EVENT_ECU_INFO_PARALLEL_ARRAY_LEN, EVENT_ECU_INFO_PARALLEL_ARRAY_LEN);
                    (void)adm_snprintf(eventDesc, EVENT_ECU_INFO_PARALLEL_ARRAY_LEN, "end get ecuInfo from VDI:failed");
                    eim_addParallelEcuEventList(
                        g_pstEventsList, (const DCHAR*)"PS_GET_ECUINFO_002", 
                        pstPartEcuOneCfg->ecuReqId, 
                        pstPartEcuOneCfg->ecuSwDid, 
                        eventDesc);
                    log_d("%s", eventDesc);
                } 
            }
            else if(2 == pstPartEcuOneCfg->serviceType)
            {
                //表示ucl采集
                (void)adm_memset(eventDesc, 0x00, EVENT_ECU_INFO_PARALLEL_ARRAY_LEN, EVENT_ECU_INFO_PARALLEL_ARRAY_LEN);
                (void)adm_snprintf(eventDesc, EVENT_ECU_INFO_PARALLEL_ARRAY_LEN, "start get ecuInfo from UCL");
                eim_addParallelEcuEventList(
                    g_pstEventsList, (const DCHAR*)"PS_GET_ECUINFO_001", 
                    pstPartEcuOneCfg->ecuReqId,
                    pstPartEcuOneCfg->ecuSwDid, 
                    eventDesc);
                log_d("%s", eventDesc);

                iRet = adm_dim_ucl_parallelCollectEcu((D16U)iInstanceId,
                                                      SINGLE_READECUINFO_TIMEOUT,
                                                      &stEcuIdenInfo,
                                                      &stDidList,
                                                      pErrorCod);

                if (ADM_ECODE_SUCCESSED != iRet ) {
                    (void)adm_memset(eventDesc, 0x00, EVENT_ECU_INFO_PARALLEL_ARRAY_LEN, EVENT_ECU_INFO_PARALLEL_ARRAY_LEN);
                    (void)adm_snprintf(eventDesc, EVENT_ECU_INFO_PARALLEL_ARRAY_LEN, "end get ecuInfo from UCL:failed");
                    eim_addParallelEcuEventList(
                        g_pstEventsList, (const DCHAR*)"PS_GET_ECUINFO_002", 
                        pstPartEcuOneCfg->ecuReqId,
                        pstPartEcuOneCfg->ecuSwDid,
                        eventDesc);
                    log_d("%s", eventDesc);
                } 
            }
            else if(3 == pstPartEcuOneCfg->serviceType)
            {

                (void)adm_memset(eventDesc, 0x00, EVENT_ECU_INFO_PARALLEL_ARRAY_LEN, EVENT_ECU_INFO_PARALLEL_ARRAY_LEN);
                (void)adm_snprintf(eventDesc, EVENT_ECU_INFO_PARALLEL_ARRAY_LEN, "start get ecuInfo from APK ");
                    eim_addParallelEcuEventList(
                    g_pstEventsList, (const DCHAR*)"PS_GET_ECUINFO_001", 
                    pstPartEcuOneCfg->ecuReqId, 
                    pstPartEcuOneCfg->ecuSwDid, 
                    eventDesc);
                log_d("%s", eventDesc);

                iRet = adm_eim_asyncGetApkVersion((D16U)iInstanceId, 
                                                pstPartEcuOneCfg,
                                                &stEcuIdenInfo,
                                                &stDidList,
                                                pErrorCod);

                if (ADM_ECODE_SUCCESSED != iRet) {
                    (void)adm_memset(eventDesc, 0x00, EVENT_ECU_INFO_PARALLEL_ARRAY_LEN, EVENT_ECU_INFO_PARALLEL_ARRAY_LEN);
                    (void)adm_snprintf(eventDesc, EVENT_ECU_INFO_PARALLEL_ARRAY_LEN, "end get ecuInfo from APK:failed");
                    eim_addParallelEcuEventList(
                        g_pstEventsList, (const DCHAR*)"PS_GET_ECUINFO_002", pstPartEcuOneCfg->ecuReqId, pstPartEcuOneCfg->ecuSwDid, eventDesc);
                    log_d("%s", eventDesc);
                }

            }
            else
            {
                iRet = ADM_ECODE_FAILED;
                log_e("not found InfoInstanceId 0x%x", iInstanceId);
                log_e("serviceType:%d is invalid", pstPartEcuOneCfg->serviceType);
            }

            if (COLLECT_TRIGGER_TYPE_BACK == g_eTriggerType 
                && ADM_ECODE_SUCCESSED == iRet && ADM_ECODE_SUCCESSED == (*pErrorCod))
            {
                (void)pthread_mutex_lock(&g_stBackCollectInfo.stMutex);
                g_stBackCollectInfo.iBackCollectEcuCnt++;
                log_i("Back collect number[%d].", g_stBackCollectInfo.iBackCollectEcuCnt);
                (void)pthread_mutex_unlock(&g_stBackCollectInfo.stMutex);
            }

            break;
        }
    }

    adm_eim_getEcuInfoReport(g_pstEventsList);
    adm_eim_clearEventList(g_pstEventsList);

    //释放stDidList
    clearOneEcuDidList(&stDidList);
    return iRet;
}

/**
 * @brief 判断是否超时
 * @note
 * @param in struct timespec *pstTs 开始时间
 * @param in D32U uiTimeWide        超时时间
 * @retval DBOOL TRUE 超时 FALSE 未超时
 */
static DBOOL isOverCollectTimeWide(struct timespec *pstStartTime, D32U uiTimeWide)
{
    DBOOL bRet = FALSE;
    struct timespec tsNow;
    D64S ulTimeWide = 0;

    if (ADM_IS_NULL(pstStartTime))
    {
        log_e("Invalid input param");
        return bRet;
    }

    /** 时间转换成毫秒 */
    ulTimeWide = uiTimeWide * 1000;
    (void)clock_gettime(CLOCK_MONOTONIC, &tsNow);

    /** 判断是否超时 */
    if(((tsNow.tv_sec - pstStartTime->tv_sec) * 1000 +
        (tsNow.tv_nsec - pstStartTime->tv_nsec) / (1000 * 1000)) > ulTimeWide)
    {
        bRet = TRUE;
    }

    return bRet;
}

/**
 * @brief ECU采集结果监控
 * @note
 * @param in DVOID* argv 线程参数
 * @retval DVOID*
 */

static DVOID* collectResultMonitorHandle(DVOID* argv)
{
    ADM_ECU_SINGLE_INFO_T *pstEcuSingle = NULL;
    ADM_ECU_SINGLE_INFO_T *pstCurEcuInfo = NULL;
    DCHAR eventDesc[EVENT_ECU_INFO_PARALLEL_ARRAY_LEN];

    ADM_COM_FUNC_ENTRY();

    if (ADM_IS_NULL(argv))
    {
        log_e("Invalid input param");
        return NULL;
    }

    g_uiWaitResultCount++;

    log_d("waitResultCount = %d", g_uiWaitResultCount);
    pstEcuSingle = (ADM_ECU_SINGLE_INFO_T*)argv;

    log_i("[ABUP-Procedure] parallel collect %s_%s_%s monitor Start.", pstEcuSingle->acRequestId,
                                                                       pstEcuSingle->acSwDid,
                                                                       pstEcuSingle->acUpOrder);
    do
    {
        sleep(2);

        /** 查询正在采集中的ECU */
        pstCurEcuInfo = getCollectingEcuCfgCtl(pstEcuSingle);
        if (ADM_IS_NULL(pstCurEcuInfo))
        {
            log_d("collect don't exist. RequestId[%s], SwDid[%s], acUpOrder[%s]", pstEcuSingle->acRequestId,
                                                                                  pstEcuSingle->acSwDid,
                                                                                  pstEcuSingle->acUpOrder);
            break;
        }

        /** 超时 */
        if (TRUE == isOverCollectTimeWide(&pstCurEcuInfo->ts, pstCurEcuInfo->uiTimeOut))
        {
            log_i("[ABUP-Procedure] parallel collect %s_%s_%s timeout.", pstCurEcuInfo->acRequestId, 
                                                                         pstCurEcuInfo->acSwDid,
                                                                         pstCurEcuInfo->acUpOrder);

            (void)adm_memset(eventDesc, 0x00, EVENT_ECU_INFO_PARALLEL_ARRAY_LEN, EVENT_ECU_INFO_PARALLEL_ARRAY_LEN);
            (void)adm_snprintf(eventDesc, EVENT_ECU_INFO_PARALLEL_ARRAY_LEN, "end get ecuInfo timeout");
            eim_addParallelEcuEventList(
                    g_pstEventsList, (const DCHAR*)"PS_GET_ECUINFO_002", 
                    pstCurEcuInfo->acRequestId, 
                    pstCurEcuInfo->acSwDid, 
                    eventDesc);
            log_d("%s", eventDesc);

            adm_eim_getEcuInfoReport(g_pstEventsList);
            adm_eim_clearEventList(g_pstEventsList);

            pstCurEcuInfo->eCollectState = ADM_ECU_SINGLE_COLLECT_FAIL;
            updateSingleEcuCfg(pstCurEcuInfo);
            delMemberParallelCollectGroup(pstCurEcuInfo);
            updateAllEcuStateCfg();
            updateParallelCollectGroup();
            break;
        }

        pstCurEcuInfo = NULL;
    } while (TRUE);
    
    adm_free(pstEcuSingle);

    g_uiWaitResultCount--;

    log_d("waitResultCount = %d", g_uiWaitResultCount);

    ADM_COM_FUNC_EXIT();

    return NULL;
}

/**
 * @brief 开启ECU采集结果监控
 * @note
 * @param in ADM_ECU_SINGLE_INFO_T *pstEcuInfo ECU信息
 * @retval D32S  0 成功, 其他失败
 */
static D32S eimParallelCollectResultMonitorStart(ADM_ECU_SINGLE_INFO_T *pstEcuInfo)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    ADM_ECU_SINGLE_INFO_T *pstEcuSingle = NULL;
    pthread_t pthreadId;

    ADM_COM_FUNC_ENTRY();

    pstEcuSingle = (ADM_ECU_SINGLE_INFO_T *)adm_malloc(sizeof(ADM_ECU_SINGLE_INFO_T));
    if (ADM_IS_NULL(pstEcuSingle))
    {
        log_e("malloc failed");
        return ADM_ECODE_APPLY_MEMORY_FAILED;
    }

    adm_memzero(pstEcuSingle, sizeof(ADM_ECU_SINGLE_INFO_T));
    adm_memcpy(pstEcuSingle, pstEcuInfo, sizeof(ADM_ECU_SINGLE_INFO_T), sizeof(ADM_ECU_SINGLE_INFO_T));

    iRet = pthread_create(&pthreadId, NULL, collectResultMonitorHandle, pstEcuSingle);
    if(ADM_IS_SUCCESS(iRet))
    {
        (void)pthread_detach(pthreadId);
        log_d("pthread create succ");
    }
    else
    {
        log_e("pthread create fail, iRet %d", iRet);
        iRet = ADM_ECODE_PTHREAD_CREATE_FAILED;
        adm_free(pstEcuSingle);
    }

    return iRet;
}

/**
 * @brief 采集线程处理函数
 * @note
 * @param in DVOID* argv 线程参数
 * @retval DVOID*
 */
static DVOID* asyncParallelCollect(DVOID* argv)
{
    ADM_ECU_SINGLE_INFO_T* pstEcuSingle = DNULL;
    DBOOL bFindCollectEcu = DFALSE;
    D32S iRet = ADM_ECODE_FAILED;
    DBOOL bReqBreak = DFALSE;
    D32S iErrCode = 0;
    D32S iRetryCount = 0;

    g_bIsCollectThreadRuning = DTRUE;
    
    while(true)
    {
        if((0 == stEcuParallelCollectCtl.collectEcuListM.size()) &&
           (ADM_ECU_ALL_COLLECT_FINISH == stEcuCfgCtl.eCollectAllState))
        {
            //采集线程退出
            log_d("collect thread exit");
            break;
        }

        if(bReqBreak)
        {
            //表示req失败,更新采集组
            log_w("req failed, need updateParallelCollectGroup");

            if(DFALSE == updateParallelCollectGroup())
            {
                log_w("all cfg ecu is collected or collecting");
            }

            bReqBreak = DFALSE;
        }

        sem_wait(&stEcuParallelCollectCtl.collectSig);

        while(true)
        {
            pthread_mutex_lock(&stEcuParallelCollectCtl.collectMutex);
            log_i("stEcuParallelCollectCtl.collectEcuListM.size[%u]", stEcuParallelCollectCtl.collectEcuListM.size());
            if(stEcuParallelCollectCtl.collectEcuListM.size() > 0)
            {
                for(ADM_ECU_SINGLE_INFO_T* pstEcuSingleOne : stEcuParallelCollectCtl.collectEcuListM)
                {
                    if((ADM_ECU_SINGLE_COLLECT_ING != pstEcuSingleOne->eCollectState) &&
                       (ADM_ECU_SINGLE_COLLECT_SUCC != pstEcuSingleOne->eCollectState) &&
                       (uiCollectCount > pstEcuSingleOne->uiCollectCount))
                    {
                        pstEcuSingleOne->eCollectState = ADM_ECU_SINGLE_COLLECT_ING;
                        pstEcuSingleOne->uiCollectCount++;

                        pstEcuSingle = pstEcuSingleOne;
                        bFindCollectEcu = DTRUE;
                        break;
                    }
                    else
                    {
                        log_i("parallel collect %s_%s_%s_%d_%d ignore", pstEcuSingleOne->acRequestId, 
                                                                        pstEcuSingleOne->acSwDid, 
                                                                        pstEcuSingleOne->acUpOrder,
                                                                        pstEcuSingleOne->uiCollectCount,
                                                                        pstEcuSingleOne->eCollectState);
                    }
                }
            }
            else
            {
                //采集组为0
                pthread_mutex_unlock(&stEcuParallelCollectCtl.collectMutex);
                break;
            }
            
            pthread_mutex_unlock(&stEcuParallelCollectCtl.collectMutex);
            
            if(ADM_IS_NULL(pstEcuSingle) || !bFindCollectEcu)
            {
                break;
            }

            if (DTRUE == eimGetSuccessEcuInfo(pstEcuSingle->acRequestId, pstEcuSingle->acSwDid))
            {
                log_i("[ABUP-Procedure] parallel collect %s_%s do not need collect, already collect successfully.", pstEcuSingle->acRequestId, pstEcuSingle->acSwDid);
                bReqBreak = DTRUE;
                pstEcuSingle->eCollectState = ADM_ECU_SINGLE_COLLECT_FAIL;
                updateSingleEcuCfg(pstEcuSingle);
                delMemberParallelCollectGroup(pstEcuSingle);
                updateAllEcuStateCfg();

                pstEcuSingle = DNULL;
                bFindCollectEcu = DFALSE;
                continue;
            }

            //req dim 采集 todo
            log_i("[ABUP-Procedure] parallel collect %s_%s_%s request Start", pstEcuSingle->acRequestId, 
                                                                              pstEcuSingle->acSwDid, 
                                                                              pstEcuSingle->acUpOrder);
			iRetryCount = 0;
            do
            {
                iRet = asyncReqDimParallelCollectOne(pstEcuSingle, &iErrCode);

                /** 错误码为ADM_ECODE_RPC_INSTANCE_OFFLINE或ADM_ECODE_RPC_SERVICE_OFFLINE，则重试 */
                if ((ADM_ECODE_RPC_INSTANCE_OFFLINE == iRet ||
                     ADM_ECODE_RPC_SERVICE_OFFLINE == iRet) &&
                    (iRetryCount < ADM_DIM_REQUEST_RETRY_COUNT))
                {
                    iRetryCount++;
                    sleep(ADM_DIM_REQUEST_RETRY_INTERVAL);
                    log_d("[ABUP-Procedure] parallel collect request failed [%s], iRet = %d, retryCount = %d", pstEcuSingle->acRequestId, iRet, iRetryCount);
                    continue;
                }
                else
                {
                    log_d("request finish, iRet = %d", iRet);
                    break;
                }
            } while (TRUE);

            if (ADM_ECODE_EIM_STOP_BACK_COLLECT == iRet)
            {
                log_e("Pause back collect.");
                break;
            }


            if(ADM_IS_ERR(iRet) || ADM_IS_ERR(iErrCode))
            {
                log_i("[ABUP-Procedure] parallel collect %s_%s_%s request End. [iRet = %d]", pstEcuSingle->acRequestId, 
                                                                                             pstEcuSingle->acSwDid,
                                                                                             pstEcuSingle->acUpOrder, 
                                                                                             (ADM_IS_ERR(iRet) ? iRet : iErrCode));

                bReqBreak = DTRUE;
                pstEcuSingle->eCollectState = ADM_ECU_SINGLE_COLLECT_FAIL;
                updateSingleEcuCfg(pstEcuSingle);
                delMemberParallelCollectGroup(pstEcuSingle);
                updateAllEcuStateCfg();
            }
            else
            {
                log_i("[ABUP-Procedure] parallel collect %s_%s_%s request End. [iRet = %d]", pstEcuSingle->acRequestId, 
                                                                                             pstEcuSingle->acSwDid,
                                                                                             pstEcuSingle->acUpOrder, 
                                                                                             iRet);
                /** 设置超时时间 */
                (void)clock_gettime(CLOCK_MONOTONIC, &pstEcuSingle->ts);
                pstEcuSingle->uiTimeOut = SINGLE_READECUINFO_TIMEOUT;
                updateSingleEcuCfg(pstEcuSingle);

                /** 监听结果 */
                eimParallelCollectResultMonitorStart(pstEcuSingle);
            }

            pstEcuSingle = DNULL;
            bFindCollectEcu = DFALSE;
        }

        if (ADM_ECODE_EIM_NOT_ALLOW_CYC_COLLECT == iRet || ADM_ECODE_EIM_STOP_BACK_COLLECT == iRet)
        {
            parallelCollectGroupDestroy();
            stEcuCfgCtl.eCollectAllState = ADM_ECU_ALL_COLLECT_CONFLICT;
            break;
        }
    }

    if (ADM_ECODE_EIM_NOT_ALLOW_CYC_COLLECT != iRet && ADM_ECODE_EIM_STOP_BACK_COLLECT != iRet)
    {
        //eim方式更新kv
        log_i("asyncParallelCollect finish. set kv begin");
        updateAllEcu2KV();
    }

    g_bIsCollectThreadRuning = DFALSE;

    return NULL;
}

/**
 * @brief 等待并行采集结束
 * @note
 * @param in DVOID
 * @retval DVOID
 */
static DVOID eimWaitCollectFinish(DVOID)
{
    D32U uiPrintCycle = 0;
    D32S iEcuNum = 0;
    D32S iTimeout = 0;
    D32U uiCount = 0;

    iEcuNum = gpstRsCfgInfo->stPartCfg.stEcuCfgList.num;
    iTimeout = (iEcuNum + 1 ) * 60;
    log_d("wait parallel collect ecu info finish, ecu num = %d, timeout = %d", iEcuNum, iTimeout);

    while (TRUE)
    {
        /** 采集冲突，退出采集流程 */
        if (ADM_ECU_ALL_COLLECT_CONFLICT == stEcuCfgCtl.eCollectAllState)
        {
            if (COLLECT_TRIGGER_TYPE_BACK == g_eTriggerType)
            {
                while (g_stBackCollectInfo.iBackCollectEcuCnt > 0 && uiCount < 12)
                {
                    log_i("Wait back collect finished.");
                    sleep(5);
                    uiCount++;
                }
            }
            log_w("parallel collect conflict, exit collect process");
            break;
        }
        
        /** 判断是否结束 */
        if (ADM_ECU_ALL_COLLECT_FINISH == stEcuCfgCtl.eCollectAllState &&
            0 == g_uiWaitResultCount &&
            DFALSE == g_bIsCollectThreadRuning)
        {
            log_w("parallel collect ecu info finish");
            break;
        }

        uiPrintCycle++;
        if (0 == uiPrintCycle % 10)
        {
            log_d("wait parallel collect ecu info finish");
        }

        if (uiPrintCycle >= iTimeout)
        {
            log_w("parallel collect ecu info timeout");
            break;
        }

        sleep(1);
    }

    return;
}

/**
 * @brief 校验采集结果
 * @note 所有ECU采集结果都为失败，才返回失败
 * @param in DVOID
 * @retval D32S 0 : 成功; 其他 : 失败
 */
static D32S eimCollectResultCheck(DVOID)
{
    D32S iRet = ADM_ECODE_FAILED;
    DBOOL bIsExistSuccess = DFALSE;
    ADM_ECU_SINGLE_INFO_T* pstOneEcuCfg = DNULL;

    ADM_COM_FUNC_ENTRY();

    pthread_mutex_lock(&stEcuCfgCtl.ecuCfgMutex);

    do
    {
        if (COLLECT_TRIGGER_TYPE_INSTALL != g_eTriggerType &&
            ADM_ECU_ALL_COLLECT_CONFLICT == stEcuCfgCtl.eCollectAllState)
        {
            log_d("collect conflict, exit collect process");
            iRet = ADM_ECODE_EIM_NOT_ALLOW_CYC_COLLECT;
            break;
        }
        
        for(auto iter = stEcuCfgCtl.listEcuSingleInfo.begin(); iter != stEcuCfgCtl.listEcuSingleInfo.end(); ++iter)
        {
            pstOneEcuCfg = *iter;

            if(ADM_IS_NULL(pstOneEcuCfg))
            {
                log_e("invalid pointer");
                continue;
            }

            /** 判断是否采集成功 */
            if (ADM_ECU_SINGLE_COLLECT_SUCC == pstOneEcuCfg->eCollectState)
            {
                bIsExistSuccess = DTRUE;
                break;
            }

            pstOneEcuCfg = DNULL;
        }

        if (DTRUE == bIsExistSuccess)
        {
            iRet = ADM_ECODE_SUCCESSED;
        }
        else
        {
            iRet = ADM_ECODE_EIM_GET_ECU_INFO_FAILED;
        }
    }while(FALSE);

    pthread_mutex_unlock(&stEcuCfgCtl.ecuCfgMutex);

    ADM_COM_FUNC_EXIT();

    return iRet;
}

/***********************************************/
/***********************************************/
/***********************************************/
/***********************************************/
/***********************************************/
typedef struct ADM_EIM_MY_APK_REQ_INFO_
{
    D32S iInstanceId;
    ADM_ECS_ECUCONFIG_T* pstPartEcuOneCfg;
    ADM_DIM_SOA_ECU_IDEN_INFO_T stEcuIdenInfo;
    ADM_DIM_SOA_DID_DATA_ARRAY_T stEcuDidList;
} ADM_EIM_MY_APK_REQ_INFO_T;

static D32S eimDidValueValidity(const ADM_DIM_SOA_DID_DATA_ARRAY_T *pstDidArray)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    D32S i = 0;
    D32S index = 0;
    DBOOL bIsInvaild = FALSE;

    ADM_COM_FUNC_ENTRY();

    for (i = 0; i < pstDidArray->didArrayLength; i++)
    {
        for (index = 0; index < adm_strlen((DCHAR *)pstDidArray->didArray[i]->didData->v.valueS); index++)
        {
            if (!isascii(pstDidArray->didArray[i]->didData->v.valueS[index]))
            {
                log_e("didValue of did:0X%02X has Illegal character", pstDidArray->didArray[i]->did);
                bIsInvaild = DTRUE;
                iRet = ADM_ECODE_EIM_GET_ECU_INFO_FAILED;
                break;
            }
        }
        if (DTRUE == bIsInvaild)
        {
            break;
        }
    }

    ADM_COM_FUNC_EXIT();

    return iRet;
}

static D32S eimDidValueCompare(ADM_ECU_SINGLE_INFO_T* pstEcuOneCfg,
                               const ADM_DIM_SOA_DID_DATA_ARRAY_T *pstDidArray)
{
    D32S iRet = ADM_ECODE_FAILED;
    DBOOL bIsPartNum = FALSE;
    DBOOL bIsHardwareNum = FALSE;
    DBOOL bIsSupplier = FALSE;
    DBOOL bIsSoftWarePartNum = FALSE;

    D32S i = 0;

    ADM_COM_FUNC_ENTRY();

    for (i = 0; i < pstDidArray->didArrayLength; i++)
    {
        if (0 == adm_strcmp(pstEcuOneCfg->acPartNumber, (DCHAR *)pstDidArray->didArray[i]->didData->v.valueS))
        {
            bIsPartNum = TRUE;
            break;
        }
    }

    for (i = 0; i < pstDidArray->didArrayLength; i++)
    {
        if (0 == adm_strcmp(pstEcuOneCfg->acPartSupplier, (DCHAR *)pstDidArray->didArray[i]->didData->v.valueS))
        {
            bIsSupplier = TRUE;
            break;
        }
    }

    for (i = 0; i < pstDidArray->didArrayLength; i++)
    {
        if (0 == adm_strcmp(pstEcuOneCfg->acPartHardwareNum, (DCHAR *)pstDidArray->didArray[i]->didData->v.valueS))
        {
            bIsHardwareNum = TRUE;
            break;
        }
    }

    if ('\0' != pstEcuOneCfg->acSsoftwarePartNumber[0])
    {
        for (i = 0; i < pstDidArray->didArrayLength; i++)
        {
            if (0 == adm_strcmp(pstEcuOneCfg->acSsoftwarePartNumber, (DCHAR *)pstDidArray->didArray[i]->didData->v.valueS))
            {
                bIsSoftWarePartNum = TRUE;
                break;
            }
        }

        if (TRUE == bIsPartNum && TRUE == bIsSupplier && TRUE == bIsHardwareNum && TRUE == bIsSoftWarePartNum)
        {
            iRet = ADM_ECODE_SUCCESSED;
        }

        if (ADM_IS_ERR(iRet))
        {
            log_d("reqid = %s, swdid = %s expect partNumber = %s, partSupplier = %s, partHardwareNum = %s, softwarePartNumber = %s",
                                                                                                            pstEcuOneCfg->acRequestId,
                                                                                                            pstEcuOneCfg->acSwDid,
                                                                                                            pstEcuOneCfg->acPartNumber,
                                                                                                            pstEcuOneCfg->acPartSupplier,
                                                                                                            pstEcuOneCfg->acPartHardwareNum,
                                                                                                            pstEcuOneCfg->acSsoftwarePartNumber);
            log_d("real did value: = ");
            for (i = 0; i < pstDidArray->didArrayLength; i++)
            {
                log_d("did[%0x] value[%d] = %s", pstDidArray->didArray[i]->did ,i, pstDidArray->didArray[i]->didData->v.valueS);
            }
        }
    }
    else
    {
        if (TRUE == bIsPartNum && TRUE == bIsSupplier && TRUE == bIsHardwareNum)
        {
            iRet = ADM_ECODE_SUCCESSED;
        }

        if (ADM_IS_ERR(iRet))
        {
            log_d("reqid = %s, swdid = %s expect partNumber = %s, partSupplier = %s, partHardwareNum = %s", 
                                                                                    pstEcuOneCfg->acRequestId,
                                                                                    pstEcuOneCfg->acSwDid,
                                                                                    pstEcuOneCfg->acPartNumber,
                                                                                    pstEcuOneCfg->acPartSupplier,
                                                                                    pstEcuOneCfg->acPartHardwareNum);
            log_d("real did value: = ");
            for (i = 0; i < pstDidArray->didArrayLength; i++)
            {
                log_d("did[%0x] value[%d] = %s", pstDidArray->didArray[i]->did ,i, pstDidArray->didArray[i]->didData->v.valueS);
            }
        }
    }

    ADM_COM_FUNC_EXIT();

    return iRet;
}

static void eimDelMyApkReqInfo(ADM_EIM_MY_APK_REQ_INFO_T* pstMyApkReqInfo)
{
    D32S i = 0;

    if (ADM_IS_NULL(pstMyApkReqInfo))
    {
        return;
    }

    for (i = 0; i < pstMyApkReqInfo->stEcuDidList.didArrayLength; i++)
    {
        if (ADM_IS_NULL(pstMyApkReqInfo->stEcuDidList.didArray) || ADM_IS_NULL(pstMyApkReqInfo->stEcuDidList.didArray[i]))
        {
            continue;;
        }

        adm_free(pstMyApkReqInfo->stEcuDidList.didArray[i]->didData);
        adm_free(pstMyApkReqInfo->stEcuDidList.didArray[i]);

    }

    if (DNULL != pstMyApkReqInfo->stEcuDidList.didArray)
    {
        adm_free(pstMyApkReqInfo->stEcuDidList.didArray);
    }

    if (DNULL != pstMyApkReqInfo->pstPartEcuOneCfg->stExternEcuInfo.partNumber)
    {
        adm_free(pstMyApkReqInfo->pstPartEcuOneCfg->stExternEcuInfo.partNumber);
    }
    if (DNULL != pstMyApkReqInfo->pstPartEcuOneCfg)
    {
        adm_free(pstMyApkReqInfo->pstPartEcuOneCfg);
    }
    adm_free(pstMyApkReqInfo);

    return;
}

static void* asyncGetApkVersion(void* argv)
{
    ADM_EIM_MY_APK_REQ_INFO_T* pstMyApkReqInfo = (ADM_EIM_MY_APK_REQ_INFO_T*)argv;
    D32S iRet = 0;
    ADM_DIM_SOA_DID_DATA_ARRAY_T *pstDidArray = DNULL;
    D32S errorCode = 0;
    D32S i = 0;

    do
    {
        if (DNULL == pstMyApkReqInfo)
        {
            log_e("Input param is nil");
            break;
        }

        usleep(500000);
        iRet = adm_eim_getApkVersion(pstMyApkReqInfo->iInstanceId, pstMyApkReqInfo->pstPartEcuOneCfg, &pstMyApkReqInfo->stEcuIdenInfo, &pstMyApkReqInfo->stEcuDidList, &pstDidArray, &errorCode);
        if (ADM_ECODE_SUCCESSED != iRet)
        {
            log_e("adm_eim_getApkVersion failed, [err:%d]", iRet);
            break;
        }

        iRet = adm_eim_parallelCollectEcuNotifyHandle(&(pstMyApkReqInfo->stEcuIdenInfo), pstDidArray);
        if (ADM_ECODE_SUCCESSED != iRet)
        {
            log_e("adm_eim_parallelCollectEcuNotifyHandle failed, [err:%d]", iRet);
            break;
        }

    } while (FALSE);

    eimDelMyApkReqInfo(pstMyApkReqInfo);
    if (DNULL != pstDidArray)
    {
        for (i = 0; i < pstDidArray->didArrayLength; i++)
        {
            if (DNULL != pstDidArray && DNULL != pstDidArray->didArray[i])
            {
                adm_free(pstDidArray->didArray[i]);
            }
        }

        adm_free(pstDidArray->didArray);
        adm_free(pstDidArray);
    }
    
    return DNULL;
}

/**
 * @brief ECU缓存信息删除
 * @note
 * @param in ADM_IPS_POLICY_DEPENDENCY_ARRAY_T *pstDependencyInfo 安装阶段版本校验依赖信息
 * @retval void
 */
static void delEcuCacheInfo(ADM_IPS_POLICY_DEPENDENCY_ARRAY_T *pstDependencyInfo)
{
    ADM_IPS_POLICY_DEPENDENCY_T *pstTmpDependency = NULL;
    D32S iRet = ADM_ECODE_SUCCESSED;
    DCHAR acSingleEcuKvKey[ECU_INFO_KV_KEY_LEN] = {0};
    D32S i = 0;
    
    ADM_COM_FUNC_ENTRY();

    if (ADM_IS_NULL(pstDependencyInfo))
    {
        log_e("pstDependencyInfo is nil");
        iRet = ADM_ECODE_INVALID_POINTER;
        return ;
    }

    /** 遍历依赖关系，获取所有ECU信息 */
    for (i = 0; i < pstDependencyInfo->dependenciesLen; i++)
    {
        pstTmpDependency = &(pstDependencyInfo->dependencies[i]);
        if (ADM_IS_NULL(pstTmpDependency->ecuReqId) ||
            ADM_IS_NULL(pstTmpDependency->ecuSwDid))
        {
            log_e("pstTmpDependency is nil");
            iRet = ADM_ECODE_INVALID_POINTER;
            break;
        }

        /** 拼接ECU信息缓存key */
        adm_memzero(acSingleEcuKvKey, ECU_INFO_KV_KEY_LEN);
        (void)adm_snprintf(acSingleEcuKvKey, ECU_INFO_KV_KEY_LEN, ECU_INFO_KV_KEY_TXT, pstTmpDependency->ecuReqId, 
                                                                                       pstTmpDependency->ecuSwDid);

        /** kv中存在则删除 */
        if(TRUE == adm_com_kvstorage_has_key(adm_core_service_getBusinessDatabaseDb(), acSingleEcuKvKey))
        {
            log_d("delete ecu info from kv, key:%s", acSingleEcuKvKey);
            adm_com_kvstorage_remove_key(adm_core_service_getBusinessDatabaseDb(), acSingleEcuKvKey);
        }
    }

    ADM_COM_FUNC_EXIT();

    return ;
}

/**
 * @brief 安装阶段采集信息对列构建
 * @note
 * @param in ADM_IPS_POLICY_DEPENDENCY_ARRAY_T *pstDependencyInfo 安装阶段版本校验依赖信息
 * @retval D32S 0 成功，其他失败
 */
static D32S installParallelCollectCreate(ADM_IPS_POLICY_DEPENDENCY_ARRAY_T *pstDependencyInfo)
{
    ADM_IPS_POLICY_DEPENDENCY_T *pstTmpDependency = NULL;
    ADM_ECS_ECUCONFIG_T *pstEcuCfg = NULL;
    ADM_ECU_SINGLE_INFO_T *pstEcuSingle = NULL;
    D32S iRet = ADM_ECODE_SUCCESSED;
    D32S i = 0;
    D32S j = 0;
    
    ADM_COM_FUNC_ENTRY();

    /** 正在采集中，忽略本次触发 */
    if(ADM_ECU_ALL_COLLECT_ING == stEcuCfgCtl.eCollectAllState)
    {
        log_e("parallel is collecting... ");
        iRet = ADM_ECODE_FAILED;
        return iRet;
    }
    
    /** 入参校验 */
    if (ADM_IS_NULL(pstDependencyInfo))
    {
        log_e("pstDependencyInfo is nil");
        iRet = ADM_ECODE_INVALID_POINTER;
        return iRet;
    }

    /** 删除需要重新采集的ECU缓存信息 */
    // delEcuCacheInfo(pstDependencyInfo);
    
    /** 遍历依赖关系，获取所有ECU信息 */
    for (i = 0; i < pstDependencyInfo->dependenciesLen; i++)
    {
        pstTmpDependency = &(pstDependencyInfo->dependencies[i]);
        if (ADM_IS_NULL(pstTmpDependency->ecuReqId) ||
            ADM_IS_NULL(pstTmpDependency->ecuSwDid))
        {
            log_e("pstTmpDependency is nil");
            iRet = ADM_ECODE_INVALID_POINTER;
            break;
        }

        iRet = ADM_ECODE_FAILED;
        
        for(j = 0; j < gpstRsCfgInfo->stPartCfg.stEcuCfgList.num; j++)
        {
            pstEcuCfg = &(gpstRsCfgInfo->stPartCfg.stEcuCfgList.pstEcuCfg[j]);
            if (ADM_IS_NULL(pstEcuCfg->ecuReqId) ||
                ADM_IS_NULL(pstEcuCfg->ecuSwDid))
            {
                log_e("pstEcuCfg is nil");
                iRet = ADM_ECODE_INVALID_POINTER;
                break;
            }
            
            if((0 == adm_strcasecmp(pstTmpDependency->ecuReqId, pstEcuCfg->ecuReqId)) &&
               (0 == adm_strcasecmp(pstTmpDependency->ecuSwDid, pstEcuCfg->ecuSwDid)))
            {
                pstEcuSingle = (ADM_ECU_SINGLE_INFO_T*)adm_malloc(sizeof(ADM_ECU_SINGLE_INFO_T));
                if(ADM_IS_NULL(pstEcuSingle))
                {
                    log_e("adm_malloc failed");
                    iRet = ADM_ECODE_APPLY_MEMORY_FAILED;
                    break;
                }

                adm_memzero(pstEcuSingle, sizeof(ADM_ECU_SINGLE_INFO_T));
                adm_snprintf(pstEcuSingle->acSingleEcuKvKey, ECU_INFO_KV_KEY_LEN, ECU_INFO_KV_KEY_TXT, pstEcuCfg->ecuReqId, pstEcuCfg->ecuSwDid);
                log_d("ecuReqid = %s, ecuSwDid = %s", pstEcuCfg->ecuReqId, pstEcuCfg->ecuSwDid);
                adm_memcpy(pstEcuSingle->acRequestId, pstEcuCfg->ecuReqId, adm_strlen(pstEcuCfg->ecuReqId), ECU_INFO_ID_LEN);
                adm_memcpy(pstEcuSingle->acResponseId, pstEcuCfg->ecuResponseId, adm_strlen(pstEcuCfg->ecuResponseId), ECU_INFO_ID_LEN);
                adm_memcpy(pstEcuSingle->acFunctionId, pstEcuCfg->ecuFunctionId, adm_strlen(pstEcuCfg->ecuFunctionId), ECU_INFO_ID_LEN);
                adm_memcpy(pstEcuSingle->acSwDid, pstEcuCfg->ecuSwDid, adm_strlen(pstEcuCfg->ecuSwDid), ECU_INFO_ID_LEN);
                adm_memcpy(pstEcuSingle->acPartNumber, pstEcuCfg->stExternEcuInfo.partNumber, 
                                                       adm_strlen(pstEcuCfg->stExternEcuInfo.partNumber), 
                                                       ECU_PARTNUM_LEN);
                adm_memcpy(pstEcuSingle->acPartSupplier, pstEcuCfg->stExternEcuInfo.partSupplier, 
                                                         adm_strlen(pstEcuCfg->stExternEcuInfo.partSupplier), 
                                                         ECU_INFO_ID_LEN);
                adm_memcpy(pstEcuSingle->acPartHardwareNum, pstEcuCfg->stExternEcuInfo.partHardwareNum, 
                                                            adm_strlen(pstEcuCfg->stExternEcuInfo.partHardwareNum), 
                                                            ECU_INFO_ID_LEN);
                adm_memcpy(pstEcuSingle->acSsoftwarePartNumber, pstEcuCfg->stExternEcuInfo.softwarePartNumber, 
                                                                adm_strlen(pstEcuCfg->stExternEcuInfo.softwarePartNumber), 
                                                                ECU_INFO_ID_LEN);
                log_d("acPartSupplier = %s, acPartHardwareNum = %s, acPartNumber = %s", pstEcuSingle->acPartSupplier, 
                                                                                        pstEcuSingle->acPartHardwareNum,
                                                                                        pstEcuSingle->acPartNumber);
                adm_memcpy(pstEcuSingle->acUpOrder, pstEcuCfg->stExternEcuInfo.upgradeOrder, adm_strlen(pstEcuCfg->stExternEcuInfo.upgradeOrder), ECU_INFO_ID_LEN);

                parallelCollect_uporder_trans(pstEcuSingle);
                pstEcuSingle->eCollectState = ADM_ECU_SINGLE_COLLECT_BEGIN;
                pstEcuSingle->uiCollectCount = 0;

                stEcuCfgCtl.listEcuSingleInfo.push_back(pstEcuSingle);
                pstEcuSingle = NULL;
                iRet = ADM_ECODE_SUCCESSED;
            }
        }

        /** 遍历结束，未找到依赖的ECU信息 */
        if (ADM_IS_ERR(iRet))
        {
            log_d("ecuReqid = %s , ecuSwDid = %s not found config", pstTmpDependency->ecuReqId, pstTmpDependency->ecuSwDid);
            break;
        }
    }

    if(ADM_IS_ERR(iRet))
    {
        log_e("adm_parallelCollectEcu_create fail");
        adm_eim_parallelCollectEcuDestroy();
        return iRet;
    }

    /** 按升级顺序排序 */
    stEcuCfgCtl.listEcuSingleInfo.sort(custom_sort);

    /** 调试log */
    for(auto iter = stEcuCfgCtl.listEcuSingleInfo.begin(); iter != stEcuCfgCtl.listEcuSingleInfo.end(); ++iter)
    {
        ADM_ECU_SINGLE_INFO_T* pstEcuSingleInfoTmp = *iter;

        if(NULL != pstEcuSingleInfoTmp)
        {
            log_d("[ABUP-Procedure] parallel collect RequestId[%s], UpOrder[%s]", pstEcuSingleInfoTmp->acRequestId, 
                                                                                  pstEcuSingleInfoTmp->acUpOrder);
        }
    }

    ADM_COM_FUNC_EXIT();

    return iRet;
}

D32S adm_eim_asyncGetApkVersion(const D32S instanceId, 
                                const ADM_ECS_ECUCONFIG_T* pstEcuCfg, 
                                const ADM_DIM_SOA_ECU_IDEN_INFO_T* pstEcuIdenInfo,
                                const ADM_DIM_SOA_DID_DATA_ARRAY_T* didArrayRequest,
                                D32S* errorCode)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    ADM_EIM_MY_APK_REQ_INFO_T* pstMyApkReqInfo = DNULL;
    D32S i = 0;
    pthread_t pthreadId;
    log_i("adm_eim_asyncGetApkVersion begin");

    do
    {
        if (DNULL == pstEcuIdenInfo || DNULL == pstEcuCfg || DNULL == didArrayRequest)
        {
            iRet = ADM_ECODE_INVALID_PARAM;
            log_e("Input param pstEcuIdenInfo || pstEcuCfg || didArrayRequest is nil, [err:%d]", iRet);
            break;
        }

        pstMyApkReqInfo = (ADM_EIM_MY_APK_REQ_INFO_T*)adm_malloc(sizeof(ADM_EIM_MY_APK_REQ_INFO_T));
        if (DNULL == pstMyApkReqInfo)
        {
            iRet = ADM_ECODE_APPLY_MEMORY_FAILED;
            log_e("My apk request info apply failed, [err:%d]", iRet);
            break;
        }
        
        pstMyApkReqInfo->iInstanceId = instanceId;
        pstMyApkReqInfo->pstPartEcuOneCfg = (ADM_ECS_ECUCONFIG_T*)adm_malloc(sizeof(ADM_ECS_ECUCONFIG_T));
        if (DNULL != pstEcuCfg->stExternEcuInfo.partNumber)
        {
            pstMyApkReqInfo->pstPartEcuOneCfg->stExternEcuInfo.partNumber = adm_strdup(pstEcuCfg->stExternEcuInfo.partNumber);
        }

        pstMyApkReqInfo->stEcuIdenInfo.requestId = pstEcuIdenInfo->requestId;
        pstMyApkReqInfo->stEcuIdenInfo.responseId = pstEcuIdenInfo->responseId;
        pstMyApkReqInfo->stEcuIdenInfo.functionId = pstEcuIdenInfo->functionId;
        pstMyApkReqInfo->stEcuIdenInfo.swDid = pstEcuIdenInfo->swDid;

        pstMyApkReqInfo->stEcuDidList.didArrayLength = didArrayRequest->didArrayLength;

        pstMyApkReqInfo->stEcuDidList.didArray = (ADM_DIM_SOA_DID_DATA_T**)adm_malloc(sizeof(ADM_DIM_SOA_DID_DATA_T*) * (didArrayRequest->didArrayLength));
        for (i = 0; i < pstMyApkReqInfo->stEcuDidList.didArrayLength; i++)
        {
            if (ADM_IS_NULL(didArrayRequest->didArray) || ADM_IS_NULL(didArrayRequest->didArray[i]))
            {
                iRet = ADM_ECODE_INVALID_PARAM;
                log_i("did index:%d is nil, [err:%d]", i, iRet);
                break;
            }

            log_i("did:%d, startByte:%d, endByte:%d", didArrayRequest->didArray[i]->did, didArrayRequest->didArray[i]->startByte, didArrayRequest->didArray[i]->endByte);
            pstMyApkReqInfo->stEcuDidList.didArray[i] = (ADM_DIM_SOA_DID_DATA_T*)adm_malloc(sizeof(ADM_DIM_SOA_DID_DATA_T));
            pstMyApkReqInfo->stEcuDidList.didArray[i]->did = didArrayRequest->didArray[i]->did;
            pstMyApkReqInfo->stEcuDidList.didArray[i]->startByte = didArrayRequest->didArray[i]->startByte;
            pstMyApkReqInfo->stEcuDidList.didArray[i]->endByte = didArrayRequest->didArray[i]->endByte;

            // pstMyApkReqInfo->stEcuDidList.didArray[i]->didData = (ADM_DIM_SOA_DATA_T*)adm_malloc(sizeof(ADM_DIM_SOA_DATA_T));
            // pstMyApkReqInfo->stEcuDidList.didArray[index]->didData->dataType = (D16U)didArrayRequest->didArray[i]->didValType;

            log_i("did:%d, startByte:%d, endByte:%d", pstMyApkReqInfo->stEcuDidList.didArray[i]->did, pstMyApkReqInfo->stEcuDidList.didArray[i]->startByte, pstMyApkReqInfo->stEcuDidList.didArray[i]->endByte);
        }

        if (ADM_ECODE_SUCCESSED != iRet)
        {
            log_e("adm_eim_asyncGetApkVersion request failed, [err:%d]", iRet);
            break;
        }
        else 
        {
            if (NULL != errorCode)
            {
                *errorCode = ADM_ECODE_SUCCESSED;
            }
        }

        iRet = pthread_create(&pthreadId, DNULL, asyncGetApkVersion, (void*)pstMyApkReqInfo);
        if (ADM_IS_SUCCESS(iRet))
        {
            (void)pthread_detach(pthreadId);
            log_d("pthread create succ");        
        }
        else
        {
            log_e("pthread create fail, iRet %d", iRet);
            iRet = ADM_ECODE_PTHREAD_CREATE_FAILED;
            eimDelMyApkReqInfo(pstMyApkReqInfo);
        }

    } while (FALSE);
    
    return iRet;
}

/**
 * @brief
 * @details    以下对外接口
 * @author  renzhendi
 * @date    Created in the 2024-7-8
 */

/**
 * @brief ECU信息通知处理
 * @note
 * @param in const ADM_DIM_SOA_ECU_IDEN_INFO_T *pstEcuIden          ECU信息
 * @param in const ADM_DIM_SOA_DID_DATA_ARRAY_T *pstDidArray        ECU DID数据
 * @retval D32S 0 - 成功;其他 - 失败
 */
D32S adm_eim_parallelCollectEcuNotifyHandle(const ADM_DIM_SOA_ECU_IDEN_INFO_T *pstEcuIden,
                                            const ADM_DIM_SOA_DID_DATA_ARRAY_T *pstDidArray)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    ECU_INFO_T stEcuInfo = {};
    D64S endTime = 0;
    DCHAR eventDesc[EVENT_ECU_INFO_PARALLEL_ARRAY_LEN];
    ADM_ECU_SINGLE_INFO_T* pstEcuOneCfg = DNULL;
    ADM_OCS_EVENTS_INFO_LIST_T *pstNotifyEventsList = NULL;

    if(ADM_IS_NULL(pstEcuIden))
    {
        log_e("unknown ecu notify, pstEcuIden is NULL");
        return ADM_ECODE_FAILED;
    }

    do
    {
        pstNotifyEventsList = adm_eim_initEventList(ADM_CVM_EVENT_LIST_COUNT);
        if (ADM_IS_NULL(pstNotifyEventsList))
        {
            log_e("adm_eim_initEventList fail");
            iRet = ADM_ECODE_INVALID_POINTER;
            break;
        }

        //数据转换todo
        ADM_ECU_SINGLE_INFO_T stEcuSingle = {};
        adapterItoH(pstEcuIden->requestId, 16, stEcuSingle.acRequestId, ECU_INFO_ID_LEN);
        adapterItoH(pstEcuIden->swDid, 16, stEcuSingle.acSwDid, ECU_INFO_ID_LEN);

        //获取配置组中当前成员
        pstEcuOneCfg = getOneEcuCfgCtl(&stEcuSingle);

        if(ADM_IS_NULL(pstEcuOneCfg))
        {
            log_e("unknown ecu notify, acRequestId[%s], acSwDid[%s]", stEcuSingle.acRequestId, stEcuSingle.acSwDid);
            iRet = ADM_ECODE_FAILED;
            break;
        }

        if(ADM_IS_NULL(pstDidArray) || (0 == pstDidArray->didArrayLength) ||
                (ADM_IS_NULL(pstDidArray->didArray)))
        {
            pstEcuOneCfg->eCollectState = ADM_ECU_SINGLE_COLLECT_FAIL;
        }
        else
        {
            if (ADM_ECODE_FAILED == eimDidValueValidity(pstDidArray))
            {
                log_i("[ABUP-Procedure] parallel collect %s_%s Failed. [Did is not ascii]", stEcuSingle.acRequestId, 
                                                                                            stEcuSingle.acSwDid);
                pstEcuOneCfg->eCollectState = ADM_ECU_SINGLE_COLLECT_FAIL;
            }
            else
            {
                if (ADM_ECODE_SUCCESSED != eimDidValueCompare(pstEcuOneCfg, pstDidArray))
                {
                    log_i("[ABUP-Procedure] parallel collect %s_%s Failed. [Did not match the cfg]", stEcuSingle.acRequestId, 
                                                                                                     stEcuSingle.acSwDid);
                    pstEcuOneCfg->eCollectState = ADM_ECU_SINGLE_COLLECT_FAIL;
                }
                else
                {
                    pstEcuOneCfg->eCollectState = ADM_ECU_SINGLE_COLLECT_SUCC;
                    log_i("[ABUP-Procedure] parallel collect %s_%s Success.", stEcuSingle.acRequestId, stEcuSingle.acSwDid);
                }
            }
        }

        if(ADM_ECU_SINGLE_COLLECT_FAIL == pstEcuOneCfg->eCollectState)
        {
            //采集失败
            //配置stEcuCfgCtl置状态
            iRet = updateSingleEcuCfg(pstEcuOneCfg);

            (void)adm_memset(eventDesc, 0x00, EVENT_ECU_INFO_PARALLEL_ARRAY_LEN, EVENT_ECU_INFO_PARALLEL_ARRAY_LEN);
            (void)adm_snprintf(eventDesc, EVENT_ECU_INFO_PARALLEL_ARRAY_LEN, "end get ecuInfo :failed");
            eim_addParallelEcuEventList(
                        pstNotifyEventsList, (const DCHAR*)"PS_GET_ECUINFO_002", 
                        pstEcuOneCfg->acRequestId, 
                        pstEcuOneCfg->acSwDid, 
                        eventDesc);
            log_d("%s", eventDesc);
        }
        else
        {
            //采集成功,以eim方式存kv todo
            DCHAR* pEcuInfoJosnStr = DNULL;
            stEcuInfo.ecuReqId = adm_strdup(pstEcuOneCfg->acRequestId);
            stEcuInfo.ecuSwDid = adm_strdup(pstEcuOneCfg->acSwDid);
            stEcuInfo.partHardwareNum = adm_strdup(pstEcuOneCfg->acPartHardwareNum);
            stEcuInfo.partNumber = adm_strdup(pstEcuOneCfg->acPartNumber);
            stEcuInfo.partSupplier = adm_strdup(pstEcuOneCfg->acPartSupplier);
            stEcuInfo.softwarePartNumber = adm_strdup(pstEcuOneCfg->acSsoftwarePartNumber);
            stEcuInfo.vdiDidListResponse = (ADM_DIM_SOA_DID_DATA_ARRAY_T*)pstDidArray;  //shallow copy
            pEcuInfoJosnStr = adm_eim_singleEcuInfoJsonStr(&stEcuInfo);

            if(NULL != pEcuInfoJosnStr)
            {
                log_i("acSingleEcuKvKey = %s ,pEcuInfoJosnStr = %s", pstEcuOneCfg->acSingleEcuKvKey, pEcuInfoJosnStr);

                if(FALSE == adm_com_kvstorage_set_string(adm_core_service_getBusinessDatabaseDb(),
                        pstEcuOneCfg->acSingleEcuKvKey,
                        pEcuInfoJosnStr,
                        ADM_KV_STORAGE_VALUE_TYPE_STRING))
                {
                    log_e("set single ecuInfo string return failed");
                }
            }
            else
            {
                log_e("adm_eim_singleEcuInfoJsonStr return failed");
            }

            eimInsertSuccessEcuInfo(stEcuInfo.ecuReqId, stEcuInfo.ecuSwDid);

            endTime = time(NULL);
            (void)adm_memset(eventDesc, 0x00, EVENT_ECU_INFO_PARALLEL_ARRAY_LEN, EVENT_ECU_INFO_PARALLEL_ARRAY_LEN);
            (void)adm_snprintf(eventDesc, EVENT_ECU_INFO_PARALLEL_ARRAY_LEN, "end get ecuInfo successful[%ld]",endTime);
            eim_addParallelEcuEventList(
                        pstNotifyEventsList, (const DCHAR*)"PS_GET_ECUINFO_002", 
                        pstEcuOneCfg->acRequestId, 
                        pstEcuOneCfg->acSwDid, 
                        eventDesc);
            log_d("%s", eventDesc);

            //free
            adm_free(stEcuInfo.ecuReqId);
            adm_free(stEcuInfo.ecuSwDid);
            adm_free(stEcuInfo.partHardwareNum);
            adm_free(stEcuInfo.partNumber);
            adm_free(stEcuInfo.partSupplier);
            adm_free(stEcuInfo.softwarePartNumber);
            adm_free(pEcuInfoJosnStr);
        }
    }while(false);

    if (NULL != pstEcuOneCfg)
    {
        //采集结果已到,不管成功或失败采集组删除该成员,等下次添加采集
        delMemberParallelCollectGroup(pstEcuOneCfg);

        //update采集组
        updateAllEcuStateCfg();

        /** 收集零件冲突不再更新待收集的零件信息 */
        if (stEcuCfgCtl.eCollectAllState != ADM_ECU_ALL_COLLECT_CONFLICT)
        {
            updateParallelCollectGroup();
        }
        
        adm_eim_getEcuInfoReport(pstNotifyEventsList);
        adm_eim_clearEventList(pstNotifyEventsList);
    }

    adm_eim_deInitEventsList(pstNotifyEventsList);

    if (COLLECT_TRIGGER_TYPE_BACK == g_eTriggerType && TRUE == g_bPauseBackRuning)
    {
        (void)pthread_mutex_lock(&g_stBackCollectInfo.stMutex);
        g_stBackCollectInfo.iBackCollectEcuCnt--;
        log_i("Handle result, back collect number[%s].", g_stBackCollectInfo.iBackCollectEcuCnt);
        (void)pthread_mutex_unlock(&g_stBackCollectInfo.stMutex);
    }

    return iRet;
}

/**
 * @brief 并行采集器销毁
 * @note
 * @param in DVOID
 * @retval DVOID 
 */
DVOID adm_eim_parallelCollectEcuDestroy(DVOID)
{
    pthread_mutex_lock(&stEcuCfgCtl.ecuCfgMutex);

    for(auto iter = stEcuCfgCtl.listEcuSingleInfo.begin(); iter != stEcuCfgCtl.listEcuSingleInfo.end(); ++iter)
    {
        ADM_ECU_SINGLE_INFO_T* pstEcuSingleInifoTmp = *iter;

        if(ADM_IS_NULL(pstEcuSingleInifoTmp))
        {
            log_e("pstEcuSingleInifoTmp is NULL");
            continue;
        }

        adm_free(pstEcuSingleInifoTmp);
    }

    if(!stEcuCfgCtl.listEcuSingleInfo.empty())
    {
        stEcuCfgCtl.listEcuSingleInfo.clear();
    }

    stEcuCfgCtl.eCollectAllState = ADM_ECU_ALL_COLLECT_BEGIN;

    pthread_mutex_unlock(&stEcuCfgCtl.ecuCfgMutex);

    //调试阶段先注释
    gpstRsCfgInfo = DNULL;

    return;
}

/**
 * @brief 并行采集器创建
 * @note
 * @param in DVOID
 * @retval D32S 0 - 成功;其他 - 失败 
 */
D32S adm_eim_parallelCollectEcuCreate(DVOID)
{
    D32S iRet = ADM_ECODE_SUCCESSED;

    ADM_COM_FUNC_ENTRY();

    do
    {
        if(ADM_ECU_ALL_COLLECT_ING == stEcuCfgCtl.eCollectAllState)
        {
            log_e("parallel is collecting... ");
            iRet = ADM_ECODE_FAILED;
            break;
        }

        //init stEcuCfgCtl
        pthread_mutex_init(&stEcuCfgCtl.ecuCfgMutex, NULL);
        stEcuCfgCtl.eCollectAllState = ADM_ECU_ALL_COLLECT_ING;

        for(D32S index = 0; index < gpstRsCfgInfo->stPartCfg.stEcuCfgList.num; ++index)
        {
            ADM_ECU_SINGLE_INFO_T* pstEcuSingle = (ADM_ECU_SINGLE_INFO_T*)adm_malloc(sizeof(ADM_ECU_SINGLE_INFO_T));

            if(ADM_IS_NULL(pstEcuSingle))
            {
                log_e("adm_malloc failed");
                iRet = ADM_ECODE_FAILED;
                break;
            }

            ADM_ECS_ECUCONFIG_T* pstEcuCfg = &(gpstRsCfgInfo->stPartCfg.stEcuCfgList.pstEcuCfg[index]);

            if(ADM_IS_NULL(pstEcuCfg))
            {
                log_w("pstEcuCfg is NULL");
                adm_free(pstEcuSingle);
                continue;
            }

            (void)adm_snprintf(pstEcuSingle->acSingleEcuKvKey, ECU_INFO_KV_KEY_LEN, ECU_INFO_KV_KEY_TXT, pstEcuCfg->ecuReqId, pstEcuCfg->ecuSwDid);

            // if(DFALSE != adm_com_kvstorage_has_key(adm_core_service_getBusinessDatabaseDb(), pstEcuSingle->acSingleEcuKvKey))
            // {
            //     /** 先删除，再采集 */
            //     if (FALSE == adm_com_kvstorage_remove_key(adm_core_service_getBusinessDatabaseDb(), pstEcuSingle->acSingleEcuKvKey))
            //     {
            //         log_e("RemoveKey[%s] return failed", pstEcuSingle->acSingleEcuKvKey);
            //     }
            //     else
            //     {
            //         log_i("RemoveKey[%s] return success", pstEcuSingle->acSingleEcuKvKey);
            //     }
            // }

            log_d("ecuReqid = %s, ecuSwDid = %s", pstEcuCfg->ecuReqId, pstEcuCfg->ecuSwDid);
            adm_memcpy(pstEcuSingle->acRequestId, pstEcuCfg->ecuReqId, adm_strlen(pstEcuCfg->ecuReqId), ECU_INFO_ID_LEN);
            adm_memcpy(pstEcuSingle->acResponseId, pstEcuCfg->ecuResponseId, adm_strlen(pstEcuCfg->ecuResponseId), ECU_INFO_ID_LEN);
            adm_memcpy(pstEcuSingle->acFunctionId, pstEcuCfg->ecuFunctionId, adm_strlen(pstEcuCfg->ecuFunctionId), ECU_INFO_ID_LEN);
            adm_memcpy(pstEcuSingle->acSwDid, pstEcuCfg->ecuSwDid, adm_strlen(pstEcuCfg->ecuSwDid), ECU_INFO_ID_LEN);
            adm_memcpy(pstEcuSingle->acPartNumber, pstEcuCfg->stExternEcuInfo.partNumber, 
                                                   adm_strlen(pstEcuCfg->stExternEcuInfo.partNumber), 
                                                   ECU_PARTNUM_LEN);
            adm_memcpy(pstEcuSingle->acPartSupplier, pstEcuCfg->stExternEcuInfo.partSupplier, 
                                                     adm_strlen(pstEcuCfg->stExternEcuInfo.partSupplier), 
                                                     ECU_INFO_ID_LEN);
            adm_memcpy(pstEcuSingle->acPartHardwareNum, pstEcuCfg->stExternEcuInfo.partHardwareNum, 
                                                        adm_strlen(pstEcuCfg->stExternEcuInfo.partHardwareNum), 
                                                        ECU_INFO_ID_LEN);
            adm_memcpy(pstEcuSingle->acSsoftwarePartNumber, pstEcuCfg->stExternEcuInfo.softwarePartNumber, 
                                                            adm_strlen(pstEcuCfg->stExternEcuInfo.softwarePartNumber), 
                                                            ECU_INFO_ID_LEN);
            log_d("acPartSupplier = %s, acPartHardwareNum = %s, acPartNumber = %s", pstEcuSingle->acPartSupplier, 
                                                                                    pstEcuSingle->acPartHardwareNum,
                                                                                    pstEcuSingle->acPartNumber);
            adm_memcpy(pstEcuSingle->acUpOrder, pstEcuCfg->stExternEcuInfo.upgradeOrder, adm_strlen(pstEcuCfg->stExternEcuInfo.upgradeOrder), ECU_INFO_ID_LEN);

            parallelCollect_uporder_trans(pstEcuSingle);
            pstEcuSingle->eCollectState = ADM_ECU_SINGLE_COLLECT_BEGIN;
            pstEcuSingle->uiCollectCount = 0;

            stEcuCfgCtl.listEcuSingleInfo.push_back(pstEcuSingle);
        }
    }
    while(false);

    //fail
    if(ADM_IS_ERR(iRet))
    {
        log_e("adm_parallelCollectEcu_create fail");
        adm_eim_parallelCollectEcuDestroy();
        return iRet;
    }

    //succ
    stEcuCfgCtl.listEcuSingleInfo.sort(custom_sort);

    //调试log
    for(auto iter = stEcuCfgCtl.listEcuSingleInfo.begin(); iter != stEcuCfgCtl.listEcuSingleInfo.end(); ++iter)
    {
        ADM_ECU_SINGLE_INFO_T* pstEcuSingleInfoTmp = *iter;

        if(NULL != pstEcuSingleInfoTmp)
        {
            log_i("[ABUP-Procedure] parallel collect RequestId[%s], UpOrder[%s]", pstEcuSingleInfoTmp->acRequestId, 
                                                                                  pstEcuSingleInfoTmp->acUpOrder);
        }
    }

    g_stBackCollectInfo.iBackCollectEcuCnt = 0;

    ADM_COM_FUNC_EXIT();

    return iRet;
}

/**
 * @brief 并行采集器启动
 * @note
 * @param in DVOID
 * @retval D32S 0 - 成功;其他 - 失败 
 */
D32S adm_eim_parallelCollectEcuStart(D32S uiSecond)
{
    pthread_t id;

    ADM_COM_FUNC_ENTRY();

    if(0 == pthread_create(&id, NULL, asyncParallelCollect, NULL))
    {
        (void)pthread_detach(id);
        log_d("adm_parallelCollectEcu_start succ");
    }
    else
    {
        log_e("adm_parallelCollectEcu_start fail");
        return ADM_ECODE_PTHREAD_CREATE_FAILED;
    }

    //update采集组M
    updateParallelCollectGroup();
    
    ADM_COM_FUNC_EXIT();

    return ADM_ECODE_SUCCESSED;
}

/**
 * @brief 设置ECU配置信息
 * @note
 * @param in DVOID *pRsCfgInfo 配置信息
 * @retval DVOID 
 */
DVOID adm_eim_setEcmEcuCfg(DVOID* pstRsCfgInfo)
{
    gpstRsCfgInfo = (ADM_ECS_RSCONFIGINFO_T*)pstRsCfgInfo;
}

/**
 * @brief 并行采集ECU信息
 * @note
 * @param in DVOID *pRsCfgInfo 配置信息
 * @param in ADM_EIM_COLLECT_TRIGGER_TYPE_E eTriggerType 触发方式
 * @param in ADM_IPS_POLICY_DEPENDENCY_ARRAY_T *pstDependencyInfo 依赖信息（安装阶段触发采集必填，其他触发类型填NULL）
 * @retval D32S 0 - 成功;其他 - 失败
 */
D32S adm_eim_parallelCollectEcuInfo(DVOID *pRsCfgInfo, 
                                    ADM_EIM_COLLECT_TRIGGER_TYPE_E eTriggerType,
                                    ADM_IPS_POLICY_DEPENDENCY_ARRAY_T *pstDependencyInfo)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    D32S iWaitCount = 0;

    ADM_COM_FUNC_ENTRY();

    if (ADM_IS_NULL(pRsCfgInfo))
    {
        log_e("pRsCfgInfo is NULL");
        return ADM_ECODE_INVALID_PARAM;
    }

    log_i("[ABUP-Procedure] parallel collect Start, triggerType[%d]", eTriggerType);
    
    if(TRUE == g_bIsCollectRuning)
    {
        if (FALSE == g_bIsBackRuning)
        {
            /** 安装阶段触发零件采集，若前有正在采集的，等待采集完成后，安装再采集 */
            if (COLLECT_TRIGGER_TYPE_INSTALL == eTriggerType)
            {
                do
                {
                    log_d("wait parallel collect finish");
                    if (FALSE == g_bIsCollectRuning)
                    {
                        log_d("last parallel collect finish");
                        break;
                    }

                    /** 5分钟仍未更新配置文件，等待超时退出 */
                    if (iWaitCount >= 5 * 60)
                    {
                        log_e("wait last collect finish timeout");
                        iRet = ADM_ECODE_EIM_GET_ECU_INFO_FAILED;
                        return iRet;
                    }

                    sleep(1);
                    iWaitCount++;
                } while (TRUE);
            }
            else 
            {
                /** 如果已经有非后台收集零件,则直接退出收集 */
                log_w("parallel is collecting... ");
                log_i("[ABUP-Procedure] parallel collect End. [iRet = %d]", ADM_ECODE_EIM_NOT_ALLOW_CYC_COLLECT);
                return ADM_ECODE_EIM_NOT_ALLOW_CYC_COLLECT;
            }
        }
        else 
        {
            /** 后台收集优先级最低，直接中断后台收集流程 */
            log_i("Wait stopping back collect start.");
            g_bPauseBackRuning = TRUE;  /** 终止后台收集零件 */
            (void)pthread_mutex_lock(&g_stCollectCond.stMutex);
            (void)pthread_cond_wait(&g_stCollectCond.stCond, &g_stCollectCond.stMutex);
            (void)pthread_mutex_unlock(&g_stCollectCond.stMutex);
            log_i("Wait stopping back collect end.");
        }
    }

    if (ADM_VSM_STATE_IDLE != adm_vsm_stateGet() &&
        COLLECT_TRIGGER_TYPE_CYCLE == eTriggerType)
    {
        log_w("state is conflict...[state = %d] ", adm_vsm_stateGet());
        log_i("[ABUP-Procedure] parallel collect End. [iRet = %d]", ADM_ECODE_EIM_NOT_ALLOW_CYC_COLLECT);
        return ADM_ECODE_EIM_NOT_ALLOW_CYC_COLLECT;
    }

    if (COLLECT_TRIGGER_TYPE_BACK == eTriggerType &&
        ADM_VSM_STATE_IDLE != adm_vsm_stateGet() &&
        ADM_VSM_STATE_CHECK_DOING != adm_vsm_stateGet())
    {
        log_i("[ABUP-Procedure] parallel collect End. triggerType[%d]", COLLECT_TRIGGER_TYPE_BACK);
        return ADM_ECODE_SUCCESSED;
    }
    

    if (COLLECT_TRIGGER_TYPE_BACK == eTriggerType)
    {
        g_bIsBackRuning = TRUE;
    }

    iWaitCount = 0;
    g_bIsCollectRuning = TRUE;

    do
    {
        log_d("parallel collect wait config update finish");
        if (FALSE == adm_ocm_isUpdateConfig() &&
            TRUE == adm_ecs_isUpdateGlobalConfigFinish())
        {
            log_d("parallel collect config update finish");
            break;
        }
        
        /** 5分钟仍未更新配置文件，等待超时退出 */
        if (iWaitCount >= 5 * 60)
        {
            log_e("parallel collect wait config update timeout");
            g_bIsCollectRuning = FALSE;
            iRet = ADM_ECODE_EIM_GET_ECU_INFO_FAILED;
            return iRet;
        }

        sleep(1);
        iWaitCount++;
    } while (TRUE);
    
    do
    {
        g_pstEventsList = adm_eim_initEventList(ADM_CVM_EVENT_LIST_COUNT);
        if (ADM_IS_NULL(g_pstEventsList))
        {
            log_e("adm_eim_initEventList fail");
            iRet = ADM_ECODE_INVALID_POINTER;
            break;
        }

        gpstRsCfgInfo = (ADM_ECS_RSCONFIGINFO_T*)pRsCfgInfo;

        g_eTriggerType = eTriggerType;

        /** 并行采集器创建 */
        if (COLLECT_TRIGGER_TYPE_INSTALL == eTriggerType)
        {
            eimCreateSuccessEcuInfo();
            iRet = installParallelCollectCreate(pstDependencyInfo);
        }
        else
        {
            eimCreateSuccessEcuInfo();
            /** 清除单个ECU缓存 */
            clearEcuSingleInfo();

            iRet = adm_eim_parallelCollectEcuCreate();
        }
        
        if (ADM_IS_ERR(iRet))
        {
            log_w("adm_eim_parallelCollectEcuCreate iRet[%d]", iRet);
            break;
        }

        /** 启动并行采集线程 */
        iRet = adm_eim_parallelCollectEcuStart(30);
        if (ADM_IS_ERR(iRet))
        {
            log_w("adm_eim_parallelCollectEcuStart iRet[%d]", iRet);
            break;
        }

        /** 阻塞等待采集结束 */
        eimWaitCollectFinish();

        /** 校验采集结果 */
        iRet = eimCollectResultCheck();
    } while (FALSE);

    /** 销毁ECU信息 */
    eimFreeSuccessEcuInfo();
    adm_eim_parallelCollectEcuDestroy();
    adm_eim_deInitEventsList(g_pstEventsList);
    g_eTriggerType = COLLECT_TRIGGER_TYPE_ING;

    ADM_COM_FUNC_EXIT();

    if (COLLECT_TRIGGER_TYPE_BACK == eTriggerType)
    {
        if (TRUE == g_bPauseBackRuning)
        {
            log_i("Start singal cond.");
            (void)pthread_mutex_lock(&g_stCollectCond.stMutex);
            (void)pthread_cond_signal(&g_stCollectCond.stCond);
            (void)pthread_mutex_unlock(&g_stCollectCond.stMutex);
            log_i("End singal cond.");
            g_bPauseBackRuning = FALSE;
        }
        g_bIsBackRuning = FALSE;

        if (TRUE == g_bIsStopByCond)
        {
            iRet = ADM_ECODE_EIM_BACK_COLLECT_COND_FAILED;
        }
    }
    g_bIsCollectRuning = FALSE;

    log_i("[ABUP-Procedure] parallel collect End. [iRet = %d]", iRet);

    return iRet;
}

DBOOL adm_eim_parallelCollectIsFinish(void)
{
    if (ADM_ECU_ALL_COLLECT_BEGIN == stEcuCfgCtl.eCollectAllState)
    {
        return TRUE;
    }

    return FALSE;
}

void adm_eim_parallelInit(void)
{
    ADM_COM_FUNC_ENTRY();

    pthread_mutex_init(&stEcuParallelCollectCtl.collectMutex, DNULL);
    sem_init(&stEcuParallelCollectCtl.collectSig, 0, 0);

    ADM_COM_FUNC_EXIT();
}



