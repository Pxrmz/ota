#include "adm_eim_log.h"
#include "adm_eim_ecuInfoManager.h"
#include "adm_ecs_rsConfig.h"
#include "adm_string.h"
#include "adm_memory.h"
#include "adm_com_errorCode.h"
#include "adm_com_kvstorage.h"
#include "adm_vsm_fsm.h"
#include "adm_ecs_parsePolicy.h"
#include "adm_eim_parallel.h"
#include "adm_cfg_default.h"
#include "adm_cfg_api.h"
#include "adm_ocm_interface.h"

#include <pthread.h>
#include <unistd.h>

struct ECU_PARALLEL_COLLECT_COND_T g_stCollectCond;
struct ADM_EIM_BACK_COLLECT_T g_stBackCollectInfo;

/**************ecu info stuct************/
typedef struct EIM_ECUINFO_ID_
{
    DCHAR* ecuReqId;  //!< 诊断请求ID
    DCHAR* ecuSwDid;  //!< 升级对象软件版本 did
    DCHAR* ecuResponseId;  //!< 诊断响应ID
    DCHAR* ecuFunctionId;  //!< 诊断功能ID
    DCHAR* key; /*[EIM]_[requestId + swDid]*/
} EIM_ECUINFO_ID_T;

typedef struct EIM_ECUINFO_ID_LIST_
{
    EIM_ECUINFO_ID_T* ecsEcuInfoList; /*配置中所有的ecuID*/
    D32S ecsEcuInfoNum; /*配置中所有的ecuID数量*/
    EIM_ECUINFO_ID_T* diffEcuInfoList; /*配置中的和Kv中差异的ecuID*/
    D32S diffEcuInfoNum; /*配置中的和Kv中差异的ecuID数量*/
} EIM_ECUINFO_ID_LIST_T;

static EIM_ECUINFO_ID_LIST_T* adm_eim_newEcuInfoIdList(void)
{
    EIM_ECUINFO_ID_LIST_T* ecuInfoIdList = NULL;
    ecuInfoIdList = (EIM_ECUINFO_ID_LIST_T*)adm_malloc(sizeof(EIM_ECUINFO_ID_LIST_T));
    if (NULL != ecuInfoIdList) {
        ecuInfoIdList->ecsEcuInfoList = NULL;
        ecuInfoIdList->ecsEcuInfoNum = 0;
        ecuInfoIdList->diffEcuInfoList = NULL;
        ecuInfoIdList->diffEcuInfoNum = 0;
    }
    return ecuInfoIdList;
}

static void adm_eim_deleteEcuInfoIdList(EIM_ECUINFO_ID_LIST_T* ecuInfoIdList)
{
    if (NULL == ecuInfoIdList) {
        return;
    }
    for (D32S i = 0; i < ecuInfoIdList->ecsEcuInfoNum; ++i) {
        if (NULL != ecuInfoIdList->ecsEcuInfoList) {
            adm_free(ecuInfoIdList->ecsEcuInfoList[i].ecuReqId);
            adm_free(ecuInfoIdList->ecsEcuInfoList[i].ecuSwDid);
            adm_free(ecuInfoIdList->ecsEcuInfoList[i].ecuResponseId);
            adm_free(ecuInfoIdList->ecsEcuInfoList[i].ecuFunctionId);
            adm_free(ecuInfoIdList->ecsEcuInfoList[i].key);
        }
    }
    adm_free(ecuInfoIdList->ecsEcuInfoList);

    for (D32S i = 0; i < ecuInfoIdList->diffEcuInfoNum; ++i) {
        if (NULL != ecuInfoIdList->diffEcuInfoList) {
            adm_free(ecuInfoIdList->diffEcuInfoList[i].ecuReqId);
            adm_free(ecuInfoIdList->diffEcuInfoList[i].ecuSwDid);
            adm_free(ecuInfoIdList->diffEcuInfoList[i].ecuResponseId);
            adm_free(ecuInfoIdList->diffEcuInfoList[i].ecuFunctionId);
            adm_free(ecuInfoIdList->diffEcuInfoList[i].key);
        }
    }
    adm_free(ecuInfoIdList->diffEcuInfoList);
    adm_free(ecuInfoIdList);
}
/**************ecu info stuct************/

typedef struct ADM_EIM_GET_ECUINFO_MANAGER_
{
    pthread_mutex_t mutexGetEcuInfoForKv; /*from Kv get mutex*/
    pthread_mutex_t mutexGetEcuInfoForDim; /*from dim read mutex*/
    DBOOL isStartCycleThread;
} ADM_EIM_GET_ECUINFO_MANAGER_T;

static ADM_EIM_GET_ECUINFO_MANAGER_T g_ecuInfoManager;
static const DCHAR* g_allEcuInfoStrKey = "[EIM]_[allEcuInfoStr]";

void adm_eim_getEcuInfoManagerInit(void)
{
    (void)pthread_mutex_init(&g_ecuInfoManager.mutexGetEcuInfoForKv, NULL);
    (void)pthread_mutex_init(&g_ecuInfoManager.mutexGetEcuInfoForDim, NULL);
    (void)pthread_mutex_init(&g_stCollectCond.stMutex, NULL);
    (void)pthread_cond_init(&g_stCollectCond.stCond, NULL);
    g_ecuInfoManager.isStartCycleThread = FALSE;
    adm_eim_parallelInit();

    (void)pthread_mutex_init(&g_stBackCollectInfo.stMutex, NULL);
    g_stBackCollectInfo.iBackCollectEcuCnt = 0;

    return;
}

void adm_eim_getEcuInfoManagerDeInit(void)
{
    (void)pthread_mutex_destroy(&g_ecuInfoManager.mutexGetEcuInfoForKv);
    (void)pthread_mutex_destroy(&g_ecuInfoManager.mutexGetEcuInfoForDim);
    (void)pthread_mutex_destroy(&g_stCollectCond.stMutex);
    (void)pthread_cond_destroy(&g_stCollectCond.stCond);
    g_ecuInfoManager.isStartCycleThread = FALSE;

    (void)pthread_mutex_destroy(&g_stBackCollectInfo.stMutex);
    g_stBackCollectInfo.iBackCollectEcuCnt = 0;

    return;
}

/*!
 *******************************************************************************
 * 获取单个ecu信息
 *
 * \note 
 *
 * \param[in]   pstEcuCfg        平台ecu配置信息
 * \param[in]   pstEcuDidList    ecuDid列表
 * \param[in]   pstEcuIdenInfo   ecu identify
 * \param[in]   pDiagScriptPath  文件设备端存储路径
 * \param[out]  ppEcuInfo        从dim中获取到ecu信息
 * 
 * \retval      D32S  获取结果
 *******************************************************************************
 */
D32S adm_eim_getSingleEcuInfo(const ADM_ECS_ECUCONFIG_T* pstEcuCfg,
    const ADM_ECS_ECUDIDLIST_T* pstEcuDidList,
    const ADM_DIM_SOA_ECU_IDEN_INFO_T* pstEcuIdenInfo,
    const DCHAR* pDiagScriptPath, ECU_INFO_T** ppEcuInfo)
{
    D32S iRet = ADM_ECODE_FAILED;
	(void)pthread_mutex_lock(&g_ecuInfoManager.mutexGetEcuInfoForDim);
	
    iRet = adm_eim_getSingleEcuInfoForEcu(pstEcuCfg,pstEcuDidList,pstEcuIdenInfo,pDiagScriptPath,ppEcuInfo);
    (void)pthread_mutex_unlock(&g_ecuInfoManager.mutexGetEcuInfoForDim);
	
	return iRet;

}

static DBOOL eimKvStorageRemoveKey(const DCHAR* key)
{
    if (NULL == key) {
        log_e("key is NULL.failed");
        return FALSE;
    }

    DBOOL boolRet = FALSE;
    (void)pthread_mutex_lock(&g_ecuInfoManager.mutexGetEcuInfoForKv);
    boolRet = adm_com_kvstorage_remove_key(adm_core_service_getBusinessDatabaseDb(), key);
    (void)pthread_mutex_unlock(&g_ecuInfoManager.mutexGetEcuInfoForKv);
    return boolRet;
}

static DBOOL eimKvStorageSetAllEcuInfo(const DCHAR* val)
{
    if (NULL == val) {
        log_e("val is NULL.failed");
        return FALSE;
    }
    DBOOL boolRet = FALSE;

    (void)pthread_mutex_lock(&g_ecuInfoManager.mutexGetEcuInfoForKv);
    boolRet = adm_com_kvstorage_set_string(
        adm_core_service_getBusinessDatabaseDb(), g_allEcuInfoStrKey, val, ADM_KV_STORAGE_VALUE_TYPE_STRING);
    (void)pthread_mutex_unlock(&g_ecuInfoManager.mutexGetEcuInfoForKv);
    return boolRet;
}

static DBOOL eimKvStorageGetAllEcuInfo(DCHAR** val)
{
    ADM_KV_STORAGE_VALUE_TYPE_E eType = ADM_KV_STORAGE_VALUE_TYPE_INVALID;
    DCHAR* valStr = NULL;
    size_t valStrLen = 0;
    DBOOL boolRet = FALSE;

    (void)pthread_mutex_lock(&g_ecuInfoManager.mutexGetEcuInfoForKv);
    boolRet = adm_com_kvstorage_get_string(
        adm_core_service_getBusinessDatabaseDb(), g_allEcuInfoStrKey, &valStr, &valStrLen, &eType);
    (void)pthread_mutex_unlock(&g_ecuInfoManager.mutexGetEcuInfoForKv);
    if (TRUE == boolRet) {
        *val = valStr;
        valStr = NULL;
    }
    return boolRet;
}

static void eimGeneratedEcuInfoIdKey(DCHAR key[], D32S arrayLen, const DCHAR* ecuReqId, const DCHAR* ecuSwDid)
{
    if (NULL == ecuReqId || NULL == ecuSwDid){
        return;
    }
    (void)adm_snprintf(key, (DSIZE)arrayLen, "[EIM]_[ECU_ID_%s_%s]", ecuReqId, ecuSwDid);
}

static D32S eimGetEcuInfoId(ADM_ECS_ECUCFGLIST_T* stEcuCfgList, EIM_ECUINFO_ID_LIST_T* ecuReqSwIdList)
{
    if (NULL == stEcuCfgList || 0 == stEcuCfgList->num || NULL == ecuReqSwIdList) {
        log_e("eimGetEcuInfoId input param invalid");
        return ADM_ECODE_INVALID_PARAM;
    }

    D32S ret = ADM_ECODE_SUCCESSED;
    DCHAR ecuInfoIdKey[64];
    (void)adm_memset(ecuInfoIdKey, 0x00, 64, 64);
    ecuReqSwIdList->ecsEcuInfoList = (EIM_ECUINFO_ID_T*)adm_malloc(sizeof(EIM_ECUINFO_ID_T) * (size_t)stEcuCfgList->num);
    ecuReqSwIdList->ecsEcuInfoNum = 0;
    ecuReqSwIdList->diffEcuInfoList = (EIM_ECUINFO_ID_T*)adm_malloc(sizeof(EIM_ECUINFO_ID_T) * (size_t)stEcuCfgList->num);
    ecuReqSwIdList->diffEcuInfoNum = 0;

    for (D32S i = 0; i < stEcuCfgList->num; ++i) {
        if (NULL != stEcuCfgList->pstEcuCfg && NULL != stEcuCfgList->pstEcuCfg[i].ecuReqId
            && NULL != stEcuCfgList->pstEcuCfg[i].ecuSwDid && NULL != stEcuCfgList->pstEcuCfg[i].ecuResponseId
            && NULL != stEcuCfgList->pstEcuCfg[i].ecuFunctionId) {
            /*all config ecuinfo*/
            (void)adm_memset(ecuInfoIdKey, 0x00, 64, 64);
            eimGeneratedEcuInfoIdKey(
                ecuInfoIdKey, 64, stEcuCfgList->pstEcuCfg[i].ecuReqId, stEcuCfgList->pstEcuCfg[i].ecuSwDid);
            log_i("get ecuReqId ecuSwDid from pstEcuCfg");
            ecuReqSwIdList->ecsEcuInfoList[i].ecuReqId = adm_strdup(stEcuCfgList->pstEcuCfg[i].ecuReqId);
            ecuReqSwIdList->ecsEcuInfoList[i].ecuSwDid = adm_strdup(stEcuCfgList->pstEcuCfg[i].ecuSwDid);
            ecuReqSwIdList->ecsEcuInfoList[i].ecuResponseId = adm_strdup(stEcuCfgList->pstEcuCfg[i].ecuResponseId);
            ecuReqSwIdList->ecsEcuInfoList[i].ecuFunctionId = adm_strdup(stEcuCfgList->pstEcuCfg[i].ecuFunctionId);
            ecuReqSwIdList->ecsEcuInfoList[i].key = adm_strdup(ecuInfoIdKey);
            ecuReqSwIdList->ecsEcuInfoNum += 1;

            /*Compare ecuInfoId between ECS and KV.In ECS, find the ecuInfo that is not collected*/
            if (FALSE == adm_com_kvstorage_has_key(adm_core_service_getBusinessDatabaseDb(), ecuInfoIdKey)) {
                log_i("get diff ecuReqId ecuSwDid from pstEcuCfg and KV");
                ecuReqSwIdList->diffEcuInfoList[i].ecuReqId = adm_strdup(stEcuCfgList->pstEcuCfg[i].ecuReqId);
                ecuReqSwIdList->diffEcuInfoList[i].ecuSwDid = adm_strdup(stEcuCfgList->pstEcuCfg[i].ecuSwDid);
                ecuReqSwIdList->diffEcuInfoList[i].ecuResponseId = adm_strdup(stEcuCfgList->pstEcuCfg[i].ecuResponseId);
                ecuReqSwIdList->diffEcuInfoList[i].ecuFunctionId = adm_strdup(stEcuCfgList->pstEcuCfg[i].ecuFunctionId);
                ecuReqSwIdList->diffEcuInfoList[i].key = adm_strdup(ecuInfoIdKey);
                ecuReqSwIdList->diffEcuInfoNum += 1;
                log_i("not collected ecu, ecuReqId = %s, ecuSwDid = %s, ecuResponseId = %s, ecuFunctionId = %s",
                    stEcuCfgList->pstEcuCfg[i].ecuReqId,
                    stEcuCfgList->pstEcuCfg[i].ecuSwDid,
                    stEcuCfgList->pstEcuCfg[i].ecuResponseId,
                    stEcuCfgList->pstEcuCfg[i].ecuFunctionId);
            } else {
                log_i("ecuInfo = %s has collected", ecuInfoIdKey);
            }
        } else {
            log_w("stEcuCfgList num[%d]param invalid", i);
        }
    }
    log_i("ecsEcuInfoNum = %d, diffEcuInfoNum = %d", ecuReqSwIdList->ecsEcuInfoNum, ecuReqSwIdList->diffEcuInfoNum);
    if (0 != ecuReqSwIdList->ecsEcuInfoNum) {
        if (0 == ecuReqSwIdList->diffEcuInfoNum) {
            log_i("all ecu info is collected");
        }
    } else {
        ret = ADM_ECODE_FAILED;
        log_e("stEcuCfgList num is empty");
    }
    return ret;
}

static D32S eimGetEcuInfoDiff(ADM_ECS_RSCONFIGINFO_T *pstRsCfgInfo, EIM_ECUINFO_ID_LIST_T* ecuInfoIdList)
{
    if (NULL == pstRsCfgInfo && NULL == ecuInfoIdList) {
        log_e("eimGetEcuInfoDiff input param invalid");
        return ADM_ECODE_INVALID_PARAM;
    }
    /*no difference*/
    if (0 == ecuInfoIdList->diffEcuInfoNum) {
        log_i("there is no difference in ecuInfo ,diffEcuInfoNum = %d", ecuInfoIdList->diffEcuInfoNum);
        return ADM_ECODE_SUCCESSED;
    }

    const ADM_ECS_GENERALCONFIG_T* pstGeneralCfg = NULL;
    const ADM_ECS_ECUCFGLIST_T* stEcuCfgList = NULL;
    stEcuCfgList = &(pstRsCfgInfo->stPartCfg.stEcuCfgList);
    pstGeneralCfg = &(pstRsCfgInfo->stGeneralCfg);

    D32S ret = ADM_ECODE_SUCCESSED;
    ADM_DIM_SOA_ECU_IDEN_INFO_T stEcuIdenInfo = {0};
    ECU_INFO_T* singleEcuInfo = NULL;
    DCHAR* singleEcuInfoStr = NULL;

    for (D32S i = 0; i < ecuInfoIdList->diffEcuInfoNum; ++i) 
    {
        if (NULL == (ecuInfoIdList->diffEcuInfoList + i)) 
        {
            log_w("ecuInfoIdList->diffEcuInfoList[%d] is NULL", i);
            continue;
        }
        for (D32S j = 0; j < stEcuCfgList->num; ++j) 
        {
            if (NULL == (stEcuCfgList->pstEcuCfg + j)) 
            {
                log_w("stEcuCfgList->pstEcuCfg[%d] is NULL", j);
                continue;
            }

            /**安装中的状态不允许进行周期性采集*/
            if (ADM_VSM_STATE_UPGRADE_DOING == adm_vsm_stateGet())
            {
                log_e("upgrade doing, not allow cycle collect");
                ret = ADM_ECODE_EIM_NOT_ALLOW_CYC_COLLECT;
                break;
            }

            if (0 == adm_strcmp(ecuInfoIdList->diffEcuInfoList[i].ecuReqId, stEcuCfgList->pstEcuCfg[j].ecuReqId)
                && 0 == adm_strcmp(ecuInfoIdList->diffEcuInfoList[i].ecuSwDid, stEcuCfgList->pstEcuCfg[j].ecuSwDid)) 
            {
                stEcuIdenInfo.requestId = (D32S)htoi(ecuInfoIdList->diffEcuInfoList[i].ecuReqId);
                stEcuIdenInfo.responseId = (D32S)htoi(ecuInfoIdList->diffEcuInfoList[i].ecuResponseId);
                stEcuIdenInfo.functionId = (D32S)htoi(ecuInfoIdList->diffEcuInfoList[i].ecuFunctionId);
                stEcuIdenInfo.swDid = (D32S)htoi(ecuInfoIdList->diffEcuInfoList[i].ecuSwDid);
                log_i("get single ecuInfo for sysi");
                (void)pthread_mutex_lock(&g_ecuInfoManager.mutexGetEcuInfoForDim);
                ret = adm_eim_getSingleEcuInfoForEcu(&stEcuCfgList->pstEcuCfg[j],
                    &pstGeneralCfg->stEcuDidList,
                    &stEcuIdenInfo,
                    pstGeneralCfg->stBusinessScript.filePath,
                    &singleEcuInfo);
                (void)pthread_mutex_unlock(&g_ecuInfoManager.mutexGetEcuInfoForDim);

                if (ADM_ECODE_SUCCESSED == ret) 
                {
                    singleEcuInfoStr = adm_eim_singleEcuInfoJsonStr(singleEcuInfo);
                    if (NULL != singleEcuInfoStr) {
                        log_i("key = %s ,singleEcuInfo = %s", ecuInfoIdList->diffEcuInfoList[i].key, singleEcuInfoStr);
                        if (FALSE
                            == adm_com_kvstorage_set_string(adm_core_service_getBusinessDatabaseDb(),
                                ecuInfoIdList->diffEcuInfoList[i].key,
                                singleEcuInfoStr,
                                ADM_KV_STORAGE_VALUE_TYPE_STRING)) {
                            log_e("set single ecuInfo string return failed");
                        }
                    } 
                    else 
                    {
                        log_e("adm_eim_singleEcuInfoJsonStr return failed");
                    }
                } 
                else 
                {
                    log_e("adm_eim_getSingleEcuInfoForEcu return failed");
                }
                adm_free(singleEcuInfoStr);
                singleEcuInfoStr = NULL;
                adm_eim_deleteEcuInfoBody(singleEcuInfo);
                singleEcuInfo = NULL;
                break;
            }
        }

        if (ADM_ECODE_EIM_NOT_ALLOW_CYC_COLLECT == ret)
        {
            break;
        }
    }

    return ret;
}

static DBOOL eimAddPartInfoToEcuObject(json_object* ecuInfoObject, const DCHAR* partNum, const DCHAR* partSupplier, 
                                            const DCHAR* softwarePartNumber,const DCHAR* partHardwareNum)
{
    if (NULL == ecuInfoObject || NULL == partNum || NULL == partSupplier || NULL == partHardwareNum) 
    {
        return FALSE;
    }
    json_object* stringObject = NULL;
    /*set ecuReqId*/
    stringObject = json_object_new_string(partNum);
    (void)json_object_object_add(ecuInfoObject, ADM_COM_AIR_PROTOCOL_FIELD_PARTNUM, stringObject);
    /*set ecuSwDid*/
    stringObject = json_object_new_string(partSupplier);
    (void)json_object_object_add(ecuInfoObject, ADM_COM_AIR_PROTOCOL_FIELD_PARTSUPPLIER, stringObject);
    /*set ecuSwDid*/

    stringObject = json_object_new_string(partHardwareNum);
    (void)json_object_object_add(ecuInfoObject, ADM_COM_AIR_PROTOCOL_FIELD_PARTHARDWARENUM, stringObject);

    if (NULL != softwarePartNumber)
    {
        stringObject = json_object_new_string(softwarePartNumber);
        (void)json_object_object_add(ecuInfoObject, ADM_COM_AIR_PROTOCOL_FIELD_SOFTWAREPARTNUMBER, stringObject);
    }
    return TRUE;
}

static void eimUpdataAllEcuInfo(EIM_ECUINFO_ID_LIST_T* ecuInfoIdList)
{
    if (NULL == ecuInfoIdList || 0 == ecuInfoIdList->ecsEcuInfoNum) {
        log_e("eimUpdataAllEcuInfo input param invalid");
        return;
    }
    D32S iRet = ADM_ECODE_SUCCESSED;
    json_object* didObjectArray = NULL;
    json_object* didObject = NULL;
    size_t length;
    ADM_KV_STORAGE_VALUE_TYPE_E eValType;
    DCHAR* singleEcuInfoStr = NULL;
    DBOOL boolRet = FALSE;
    DCHAR* didObjectArrayStr = NULL;
    ADM_ECS_ECUDIDLIST_T stEcuDidList = {0};
    ADM_ECS_EXTERN_ECU_INFO_T* pstExternEcuInfo = NULL;

    didObjectArray = json_object_new_array();

    /*get single ecu info from kv*/
    log_i("get single ecu info from kv");
    for (D32S i = 0; i < ecuInfoIdList->ecsEcuInfoNum; ++i) {
        boolRet = adm_com_kvstorage_get_string(adm_core_service_getBusinessDatabaseDb(),
            ecuInfoIdList->ecsEcuInfoList[i].key,
            &singleEcuInfoStr,
            &length,
            &eValType);
        if (FALSE == boolRet) {
            log_e("get single ecuInfo string return failed");
            continue;
        }
        log_i("key = %s ,singleEcuInfo = %s", ecuInfoIdList->ecsEcuInfoList[i].key, singleEcuInfoStr);
        didObject = json_tokener_parse(singleEcuInfoStr);
        adm_free(singleEcuInfoStr);
        singleEcuInfoStr = NULL;
        if (NULL == didObject) {
            log_e("json_tokener_parse return failed");
            continue;
        }

        (void)adm_memzero(&stEcuDidList, sizeof(stEcuDidList));
        pstExternEcuInfo = (ADM_ECS_EXTERN_ECU_INFO_T*)adm_malloc(sizeof(ADM_ECS_EXTERN_ECU_INFO_T));
        if (ADM_IS_NULL(pstExternEcuInfo))
        {
            log_e("pstExternEcuInfo malloc return failed");
            continue;
        }

        iRet = adm_ecs_getOneEcuDidListInfo(ecuInfoIdList->ecsEcuInfoList[i].ecuReqId,ecuInfoIdList->ecsEcuInfoList[i].ecuSwDid,
            &stEcuDidList, pstExternEcuInfo);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to get ecu dids, ecuReqid[%s], ecuSwDid[%s], err[%d]", 
                    ecuInfoIdList->ecsEcuInfoList[i].ecuReqId, 
                    ecuInfoIdList->ecsEcuInfoList[i].ecuSwDid, iRet);
            adm_ecs_delEcuDidList(stEcuDidList.num, &stEcuDidList.pstEcuDidInfo);
            adm_ecs_delExternEcuInfo(&pstExternEcuInfo);
            continue;
        }

        /*add part info*/
        if (FALSE == eimAddPartInfoToEcuObject(didObject, pstExternEcuInfo->partNumber, 
                            pstExternEcuInfo->partSupplier, pstExternEcuInfo->softwarePartNumber,
                            pstExternEcuInfo->partHardwareNum)) 
        {
            log_e("eimAddPartInfoToEcuObject failed");
            adm_ecs_delEcuDidList(stEcuDidList.num, &stEcuDidList.pstEcuDidInfo);
            adm_ecs_delExternEcuInfo(&pstExternEcuInfo);
            didObject = NULL;
            continue;
        }

        (void)json_object_array_add(didObjectArray, didObject);

        adm_ecs_delEcuDidList(stEcuDidList.num, &stEcuDidList.pstEcuDidInfo);
        adm_ecs_delExternEcuInfo(&pstExternEcuInfo);
        didObject = NULL;
    }

    log_i("set all ecu info to KV");
    if (0U != json_object_array_length(didObjectArray)) {
        didObjectArrayStr = adm_strdup(json_object_to_json_string(didObjectArray));
        log_i("key = %s ,allEcuInfo = %s", g_allEcuInfoStrKey, didObjectArrayStr);
        boolRet = eimKvStorageSetAllEcuInfo(didObjectArrayStr);
        adm_free(didObjectArrayStr);
        didObjectArrayStr = NULL;
        if (FALSE == boolRet) {
            log_e("set all ecu info string return failed");
        }
    } else {
        log_w("ecu info no cache");
    }

    log_i("remove single ecu info all cache");
    for (D32S i = 0; i < ecuInfoIdList->ecsEcuInfoNum; ++i) {
        boolRet = eimKvStorageRemoveKey(ecuInfoIdList->ecsEcuInfoList[i].key);
        if (FALSE == boolRet) {
            log_e("eimKvStorageRemoveKey return failed");
        }
    }
    (void)json_object_put(didObjectArray);
    return;
}

static const DCHAR* g_cycleCollectKey = "[EIM]_[cycleCollectTimestamp]";

static DBOOL eim_setTimestampKv(D64S timestamp)
{
    DCHAR timestampStr[64];
    (void)adm_memset(timestampStr, 0x00, 64, 64);
    (void)adm_snprintf(timestampStr, 64, "%d", timestamp);
    return adm_com_kvstorage_set_string(
        adm_core_service_getBusinessDatabaseDb(), g_cycleCollectKey, timestampStr, ADM_KV_STORAGE_VALUE_TYPE_STRING);
}

static DBOOL eim_getTimestampKv(D64S* timestamp)
{
    DCHAR* timestampStr = NULL;
    size_t valStrLen;
    ADM_KV_STORAGE_VALUE_TYPE_E eType = ADM_KV_STORAGE_VALUE_TYPE_INVALID;
    DBOOL boolRet = FALSE;
    boolRet = adm_com_kvstorage_get_string(
        adm_core_service_getBusinessDatabaseDb(), g_cycleCollectKey, &timestampStr, &valStrLen, &eType);
    if (TRUE == boolRet) {
        (void)adm_stringToInt64(timestampStr, timestamp);
        adm_free(timestampStr);
    }
    return boolRet;
}

static DBOOL waitCyclePeriod(D32S ps32CyclePeriod)
{
    DBOOL bIsExitCycle = FALSE;

    while (TRUE)
    {
        if (TRUE == adm_ocm_isUpdateConfig())
        {
            bIsExitCycle = TRUE;
            log_d("configUpdate need exit");
            break;
        }

        if (ps32CyclePeriod <= 0)
        {
            log_d("period is arrival");
            break;
        }
        ps32CyclePeriod--;
        (void)sleep((D32U)1);
    }

    return bIsExitCycle;
}

static DBOOL eim_cycleSleep(D32S ps32CyclePeriod)
{
    D64S getKvTimestamp = 0;        /*上次周期采集的时间戳*/
    D64S getCurrentTimestamp = 0;   /*当前系统时间戳*/
    D64S periodPlusKvTimestamp = 0; /*上次周期采集的时间戳 + 云端配置的零件采集周期时间*/
    DBOOL bIsExitCycle = FALSE;
    D32S iTmpCyclePeriod = 0;

    /*获取当前系统时间戳*/
    getCurrentTimestamp = time(NULL);

    /*获取上次采集零件信息的时间戳*/
    if (TRUE == eim_getTimestampKv(&getKvTimestamp))
    {
        periodPlusKvTimestamp = getKvTimestamp + ps32CyclePeriod;
        log_i("last collect time[%lld] current time[%lld] cycle period[%d]", getKvTimestamp, getCurrentTimestamp, ps32CyclePeriod);
        if (getCurrentTimestamp <= periodPlusKvTimestamp)
        {
            log_i("cycle collect time not arrival, sleep %lld seconds", periodPlusKvTimestamp - getCurrentTimestamp);
            iTmpCyclePeriod = periodPlusKvTimestamp - getCurrentTimestamp;
            bIsExitCycle = waitCyclePeriod(iTmpCyclePeriod);
        }
        else
        {
            log_i("cycle collect time arrival");
            bIsExitCycle = waitCyclePeriod(10);
        }
    }
    else
    {
        /** 当缓存中不存在历史时间时，先已当前时间为上次采集时间（避免永远无法触发周期采集的问题） */
        (void)eim_setTimestampKv(getCurrentTimestamp);
        log_i("kv no timestamp sleep %d seconds cycle period[%d] %lld", ps32CyclePeriod, ps32CyclePeriod, getCurrentTimestamp);

        bIsExitCycle = waitCyclePeriod(ps32CyclePeriod);
    }

    return bIsExitCycle;
}

static void *eimGetEcuinfoManagerCycleCollect(void *argv)
{
    g_ecuInfoManager.isStartCycleThread = TRUE;

    ADM_ECS_RSCONFIGINFO_T *pstRsCfgInfo = NULL;
    EIM_ECUINFO_ID_LIST_T *ecuInfoIdList = NULL;
    D32S ret = ADM_ECODE_FAILED;
    const D32S iDefaultPerio = 24 * 60 * 60; /*default 24h*/
    D32S iReallCyclePeriod = 0;
    D32S s32PollingCycle = 0;
    DBOOL bIsExitCycle = FALSE;
    ret = adm_ecs_getPollingCycle(&s32PollingCycle);
    if (ADM_ECODE_SUCCESSED != ret || s32PollingCycle < 0)
    {
        iReallCyclePeriod = iDefaultPerio;
        log_w("adm_ecs_getCheckCyclePeriod return error[%d],use default cycle = %d", ret, iReallCyclePeriod);
    }
    else
    {
        if (0 == s32PollingCycle)
        {
            log_w("don't need cycle collect");
            return NULL;
        }
        else
        {
            /** 巡检周期单位是小时，需要转换成秒 */
            iReallCyclePeriod = s32PollingCycle * 60 * 60;
        }
    }

    log_i("cycle collect time = %d", iReallCyclePeriod);

    while (TRUE == g_ecuInfoManager.isStartCycleThread)
    {
        D64S iStartTime = 0;
        do
        {

            bIsExitCycle = eim_cycleSleep(iReallCyclePeriod);
            if (TRUE == bIsExitCycle)
            {
                g_ecuInfoManager.isStartCycleThread = FALSE;
                break;
            }

            if (FALSE == g_ecuInfoManager.isStartCycleThread)
            {
                log_w("end of cycle thread");
                return NULL;
            }

            iStartTime = (D64S)time(NULL);

            /**安装中的状态不允许进行周期性采集*/
            if (ADM_VSM_STATE_UPGRADE_DOING == adm_vsm_stateGet())
            {
                log_e("upgrade doing, not allow cycle collect");
                break;
            }

            log_i("cycle collect. adm_ecs_getConfigInfo");
            pstRsCfgInfo = adm_ecs_newRSConfigInfo();
            if (NULL == pstRsCfgInfo)
            {
                log_e("adm_ecs_newEcuConfigInfo failed");
                break;
            }

            ret = adm_ecs_getConfigInfo(pstRsCfgInfo);
            if (ADM_ECODE_SUCCESSED != ret)
            {
                log_w("adm_ecs_getConfigInfo ret:error[%d]", ret);
                break;
            }
            ecuInfoIdList = adm_eim_newEcuInfoIdList();
            if (NULL == ecuInfoIdList)
            {
                log_w("adm_eim_newEcuInfoIdList ret:failed");
                break;
            }
            log_i("cycle collect. eimGetEcuInfoId");
            ret = eimGetEcuInfoId(&(pstRsCfgInfo->stPartCfg.stEcuCfgList), ecuInfoIdList);
            if (ADM_ECODE_SUCCESSED != ret)
            {
                log_w("eimGetEcuInfoId ret:error[%d]", ret);
                break;
            }

        #ifdef ADM_PARALLEL_GET_ECU
            log_i("cycle parallel collect");
            ret = adm_eim_parallelCollectEcuInfo((DVOID *)pstRsCfgInfo, COLLECT_TRIGGER_TYPE_CYCLE, NULL);
            if (ADM_ECODE_SUCCESSED != ret)
            {
                log_w("adm_eim_parallelCollectEcuInfo ret:error[%d]", ret);
                break;
            }
        #else
            log_i("cycle collect. eimGetEcuInfoDiff");
            ret = eimGetEcuInfoDiff(pstRsCfgInfo, ecuInfoIdList);
            if (ADM_ECODE_SUCCESSED != ret)
            {
                log_w("eimGetEcuInfoDiff ret:error[%d]", ret);
                break;
            }
            
            log_i("cycle collect. eimUpdataAllEcuInfo");
            eimUpdataAllEcuInfo(ecuInfoIdList);
        #endif
        } while (FALSE);
        (void)eim_setTimestampKv(iStartTime);
        adm_ecs_delRSConfigInfo(&pstRsCfgInfo);
        adm_eim_deleteEcuInfoIdList(ecuInfoIdList);
        pstRsCfgInfo = NULL;
        ecuInfoIdList = NULL;

        log_i("single cycle.finish");
    }
    return NULL;
}

void adm_eim_getEcuInfoManagerCycleCollection(void)
{
    if (FALSE == g_ecuInfoManager.isStartCycleThread) {
        pthread_t id;
        if (0 == pthread_create(&id, NULL, eimGetEcuinfoManagerCycleCollect, NULL)) {
            (void)pthread_detach(id);
            g_ecuInfoManager.isStartCycleThread = TRUE;
        } else {
            log_e("creat thread failed");
        }
    } else {
        log_w("cycle collection has start");
    }
    return;
}

D32S adm_eim_getEcuInfoManagerKvGetStr(DCHAR** ecuInfoListJsonStr)
{
    DCHAR* mJsonStr = NULL;
    DBOOL boolRet = FALSE;

    boolRet = eimKvStorageGetAllEcuInfo(&mJsonStr);
    if (TRUE == boolRet) {
        if (NULL != mJsonStr)
        {
            *ecuInfoListJsonStr = mJsonStr;
        }
        mJsonStr = NULL;
        log_e("get ecu info str from KV,success");
        return ADM_ECODE_SUCCESSED;
    } else {
        log_e("kv no cache ecu info");
        return ADM_ECODE_FAILED;
    }
}

D32S adm_eim_getSingleEcuInfoFromKv(const DCHAR* ecuReqId,
    const DCHAR* ecuSwDid,
    ADM_DIM_SOA_DID_DATA_ARRAY_T** singleEcuInDidList)
{
    if (NULL == ecuReqId || NULL == ecuSwDid) {
        log_e("input param invalid");
        return ADM_ECODE_INVALID_PARAM;
    }
    D32S retVal = ADM_ECODE_FAILED;
    DCHAR* ecuInfoListJsonStr = NULL;
    ADM_EIM_GETECUINFO_T* ecuInfo = NULL;

    if (ADM_ECODE_SUCCESSED == adm_eim_getEcuInfoManagerKvGetStr(&ecuInfoListJsonStr)) {
        ecuInfo = adm_eim_ecuInfoJsonStrParse(ecuInfoListJsonStr);
        if (NULL != ecuInfo) {
            for (D32S i = 0; i < ecuInfo->num; ++i) {
                if (NULL == ecuInfo->ecuInfo || NULL == ecuInfo->ecuInfo[i]->ecuReqId
                    || NULL == ecuInfo->ecuInfo[i]->ecuSwDid) {
                    log_w("ecuInfo->ecuInfo[%d] is invalid", i);
                    continue;
                }

                if (0 == adm_strcmp(ecuReqId, ecuInfo->ecuInfo[i]->ecuReqId)
                    && 0 == adm_strcmp(ecuSwDid, ecuInfo->ecuInfo[i]->ecuSwDid)) {
                    *singleEcuInDidList = ecuInfo->ecuInfo[i]->vdiDidListResponse;
                    ecuInfo->ecuInfo[i]->vdiDidListResponse = NULL;
                    retVal = ADM_ECODE_SUCCESSED;
                    log_i("get SingleEcuInfo success, ecuReqId = %s ecuSwDid=%s", ecuReqId, ecuSwDid);
                    break;
                }
            }
            if (NULL == *singleEcuInDidList) {
                retVal = ADM_ECODE_FAILED;
                log_e("get SingleEcuInfo failed, ecuReqId = %s ecuSwDid=%s", ecuReqId, ecuSwDid);
            }
        } else {
            log_e("adm_eim_singleEcuInfoJsonStr return failed");
        }
    } else {
        log_e("adm_eim_getEcuInfoManagerKvGetStr return failed");
    }
    adm_free(ecuInfoListJsonStr);
    adm_eim_deleteGetEcuInfoBody(ecuInfo);
    return retVal;
}

D32S adm_eim_getEcuInfoManagerKvGet(ADM_EIM_GETECUINFO_T** ecuInfoList)
{
    DCHAR* ecuInfoListJsonStr = NULL;
    ADM_EIM_GETECUINFO_T* ecuInfo = NULL;
    D32S retVal = ADM_ECODE_FAILED;

    if (ADM_ECODE_SUCCESSED == adm_eim_getEcuInfoManagerKvGetStr(&ecuInfoListJsonStr)) {
        ecuInfo = adm_eim_ecuInfoJsonStrParse(ecuInfoListJsonStr);
        if (NULL != ecuInfo) {
            retVal = ADM_ECODE_SUCCESSED;
            *ecuInfoList = ecuInfo;
            ecuInfo = NULL;
            log_i("get ecuInfoList success");
        } else {
            log_e("get ecuInfoList failed");
        }
    } else {
        log_e("adm_eim_getEcuInfoManagerKvGetStr return failed");
    }
    adm_free(ecuInfoListJsonStr);
    return retVal;
}

static D32S eimGetEcuinfoManagerDimGet(ADM_EIM_GETECUINFO_T** ecuInfoList, DCHAR** ecuInfoListJsonStr)
{
    ADM_ECS_RSCONFIGINFO_T* pstRsCfgInfo = NULL;
    ADM_EIM_GETECUINFO_T* allEcuInfo = NULL;
    DCHAR* allEcuInfoStr = NULL;
    D32S ret = ADM_ECODE_FAILED;
    D32S removeCacheRet = ADM_ECODE_FAILED;
    DBOOL boolRet = FALSE;
    EIM_ECUINFO_ID_LIST_T* ecuInfoIdList = NULL;

    do {
        pstRsCfgInfo = adm_ecs_newRSConfigInfo();
        /*get config data*/
        ret = adm_ecs_getConfigInfo(pstRsCfgInfo);
        if (ADM_ECODE_SUCCESSED != ret) {
            log_e("adm_ecs_getConfigInfo ret:error[%d]", ret);
            ret = ADM_ECODE_EIM_GETCONFIG_FAILED;
            break;
        }
        if (NULL == pstRsCfgInfo) {
            log_e("config data is invalid");
            ret = ADM_ECODE_EIM_GETCONFIG_FAILED;
            break;
        }

        allEcuInfo = adm_eim_newGetEcuInfoBody();
        if (NULL == allEcuInfo) {
            log_e("adm_eim_newGetEcuInfoBody return,failed");
            break;
        }
        /*get ecu info for dim*/
        (void)pthread_mutex_lock(&g_ecuInfoManager.mutexGetEcuInfoForDim);
        ret = adm_eim_getEcuInfoForEcu(pstRsCfgInfo, allEcuInfo);
        (void)pthread_mutex_unlock(&g_ecuInfoManager.mutexGetEcuInfoForDim);
        if (ADM_ECODE_SUCCESSED != ret) {
            log_e("adm_eim_getEcuInfoForEcu ret:error[%d]", ret);
            ret = ADM_ECODE_EIM_GETECUINFO_FAILED;
            break;
        }
        /*set ecu info to KV*/
        allEcuInfoStr = adm_eim_ecuInfoJsonStr(allEcuInfo);
        if (NULL != allEcuInfoStr) {
            *ecuInfoListJsonStr = allEcuInfoStr;
            allEcuInfoStr = NULL;
            *ecuInfoList = allEcuInfo;
            allEcuInfo = NULL;
            ret = ADM_ECODE_SUCCESSED;
            log_i("get all ecuInfoStr from dim.success");
            log_i("set all ecuInfoStr to KV = %s", *ecuInfoListJsonStr);
            boolRet = eimKvStorageSetAllEcuInfo(*ecuInfoListJsonStr);
            if (FALSE == boolRet) {
                log_w("set ecu info to KV failed");
                break;
            }
        } else {
            log_e("adm_eim_ecuInfoJsonStr return failed");
            ret = ADM_ECODE_EIM_ECUINFOTOSTR_FAILED;
            break;
        }
        /*remove single ecu info all cache*/
        ecuInfoIdList = adm_eim_newEcuInfoIdList();
        removeCacheRet = eimGetEcuInfoId(&(pstRsCfgInfo->stPartCfg.stEcuCfgList), ecuInfoIdList);
        if (ADM_ECODE_SUCCESSED != removeCacheRet) {
            log_w("eimGetEcuInfoId removeCacheRet:error[%d]", removeCacheRet);
            break;
        }
        log_i("remove single ecu info all cache");
        for (D32S i = 0; i < ecuInfoIdList->ecsEcuInfoNum; ++i) {
            boolRet = eimKvStorageRemoveKey(ecuInfoIdList->ecsEcuInfoList[i].key);
            if (FALSE == boolRet) {
                log_w("eimKvStorageRemoveKey[%d] return failed", i);
            }
        }
    } while (FALSE);
    adm_free(allEcuInfoStr);
    adm_eim_deleteGetEcuInfoBody(allEcuInfo);
    adm_ecs_delRSConfigInfo(&pstRsCfgInfo);
    adm_eim_deleteEcuInfoIdList(ecuInfoIdList);
    return ret;
}

/**
 * @brief 并行采集ECU信息
 * @note
 * @param out ADM_EIM_GETECUINFO_T **ecuInfoList
 * @param out DCHAR **ecuInfoListJsonStr
 * @retval D32S 0 成功 其他 失败
 * 
 */
static D32S eimGetEcuinfoManagerParallelGet(ADM_EIM_GETECUINFO_T **ecuInfoList, DCHAR **ecuInfoListJsonStr, ADM_EIM_COLLECT_TRIGGER_TYPE_E eTriggerType)
{
    ADM_ECS_RSCONFIGINFO_T *pstRsCfgInfo = NULL;
    D32S iRet = ADM_ECODE_FAILED;

    ADM_COM_FUNC_ENTRY();

    do
    {
        /** 配置信息内存申请 */
        pstRsCfgInfo = adm_ecs_newRSConfigInfo();
        if (NULL == pstRsCfgInfo)
        {
            log_e("adm_ecs_newRSConfigInfo return,failed");
            break;
        }

        /** 获取配置信息 */
        iRet = adm_ecs_getConfigInfo(pstRsCfgInfo);
        if (ADM_IS_ERR(iRet))
        {
            log_e("adm_ecs_getConfigInfo ret:error[%d]", iRet);
            iRet = ADM_ECODE_EIM_GETCONFIG_FAILED;
            break;
        }

        /** 获取ecu信息 */
        if (eTriggerType != COLLECT_TRIGGER_TYPE_BACK)
        {
            (void)pthread_mutex_lock(&g_ecuInfoManager.mutexGetEcuInfoForDim);
        }

        iRet = adm_eim_parallelCollectEcuInfo(pstRsCfgInfo, eTriggerType, NULL);

        if (eTriggerType != COLLECT_TRIGGER_TYPE_BACK)
        {
            (void)pthread_mutex_unlock(&g_ecuInfoManager.mutexGetEcuInfoForDim);
        }
        if (ADM_IS_ERR(iRet))
        {
            log_e("adm_eim_getEcuInfoForEcu ret:error[%d]", iRet);
            break;
        }

        /** 从KV中获取ecuInfoList字符串 */
        adm_eim_getEcuInfoManagerKvGetStr(ecuInfoListJsonStr);

        /** 从KV中获取ecuInfoList */
        adm_eim_getEcuInfoManagerKvGet(ecuInfoList);
    } while (FALSE);

    /** 释放资源 */
    adm_ecs_delRSConfigInfo(&pstRsCfgInfo);

    ADM_COM_FUNC_EXIT();

    return iRet;
}

D32S adm_eim_getEcuInfoManagerDim1Get(ADM_EIM_GETECUINFO_T** ecuInfoList)
{
    DCHAR* ecuInfoListJsonStr = NULL;
    D32S ret = ADM_ECODE_FAILED;

    ADM_COM_FUNC_ENTRY();

#ifdef ADM_PARALLEL_GET_ECU
    ret = eimGetEcuinfoManagerParallelGet(ecuInfoList, &ecuInfoListJsonStr, COLLECT_TRIGGER_TYPE_INSTALL);
#else
    ret = eimGetEcuinfoManagerDimGet(ecuInfoList, &ecuInfoListJsonStr);
#endif

    adm_free(ecuInfoListJsonStr);

    ADM_COM_FUNC_EXIT();

    return ret;
}

D32S adm_eim_getEcuInfoManagerDimGetStr(DCHAR** ecuInfoListJsonStr, ADM_EIM_COLLECT_TRIGGER_TYPE_E eTriggerType)
{
    ADM_EIM_GETECUINFO_T* ecuInfoList = NULL;
    D32S ret = ADM_ECODE_FAILED;
    D32S iWaitCount = 0;

    ADM_COM_FUNC_ENTRY();

#ifdef ADM_PARALLEL_GET_ECU
    ret = eimGetEcuinfoManagerParallelGet(&ecuInfoList, ecuInfoListJsonStr, eTriggerType);
#else
    if (eTriggerType != COLLECT_TRIGGER_TYPE_BACK)
    {
        ret = eimGetEcuinfoManagerDimGet(&ecuInfoList, ecuInfoListJsonStr);
    }
    else 
    {
        log_e("Do not support back collect type.");
    }
#endif
    
    adm_eim_deleteGetEcuInfoBody(ecuInfoList);

    /** 如果采集冲突，需要等采集结束，再从缓存读取ECU信息 */
    if (ADM_ECODE_EIM_NOT_ALLOW_CYC_COLLECT == ret)
    {
        while (TRUE)
        {
            log_d("wait parallel collect finish");
            if (TRUE == adm_eim_parallelCollectIsFinish())
            {
                log_d("parallel collect finish");
                break;
            }

            /** 10分钟仍未采集完成，等待超时退出 */
            if (iWaitCount >= 10 * 60)
            {
                log_e("wait parallel collect finish timeout");
                ret = ADM_ECODE_EIM_GET_ECU_INFO_FAILED;
                break;
            }

            sleep(1);
            iWaitCount++;
        }

        if (ADM_ECODE_EIM_GET_ECU_INFO_FAILED != ret)
        {
            /** 从KV中获取ecuInfoList字符串 */
            ret = adm_eim_getEcuInfoManagerKvGetStr(ecuInfoListJsonStr);
        }
    }

    ADM_COM_FUNC_EXIT();

    return ret;
}

void adm_eim_getEcuInfoManagerRealTimeUpdate(void)
{
    D32S ret = ADM_ECODE_FAILED;
    DCHAR* ecuInfoListJsonStr = NULL;
    
    /** 设置配置更新标志，退出周期采集线程 */
    adm_ecs_sameConfigHandle(FALSE, adm_eim_getSingleEcuDidInfo);

    ret = adm_eim_getEcuInfoManagerDimGetStr(&ecuInfoListJsonStr, COLLECT_TRIGGER_TYPE_CONFIG_UPDATE);
    if (ADM_ECODE_SUCCESSED == ret) {
        log_i("real-time update ecuInfo successful");
        adm_free(ecuInfoListJsonStr);
    } else {
        log_e("adm_eim_getEcuInfoManagerDimGetStr return error[%d]", ret);
    }

    /** 重新开启周期采集线程 */
    adm_eim_getEcuInfoManagerCycleCollection();

    return;
}

void adm_eim_getEcuInfoManagerEcusStore(ADM_EIM_GETECUINFO_T* newEcusArray)
{
    DCHAR* allEcuInfoStr = NULL;
    DBOOL boolRet = FALSE;
    DCHAR ecuInfoIdKey[64];
    (void)adm_memset(ecuInfoIdKey, 0x00, 64, 64);
    /*set ecu info to KV*/
    allEcuInfoStr = adm_eim_ecuInfoJsonStr(newEcusArray);
    if (NULL != allEcuInfoStr) {
        log_i("set allEcuInfoStr to KV = %s", allEcuInfoStr);
        boolRet = eimKvStorageSetAllEcuInfo(allEcuInfoStr);
        if (FALSE == boolRet) {
            log_w("set ecu info to KV failed");
        }
    } else {
        log_e("adm_eim_ecuInfoJsonStr return failed");
        return;
    }

    log_i("remove single ecu info all cache");
    for (D32S i = 0; i < newEcusArray->num; ++i) {
        if (NULL == newEcusArray->ecuInfo || NULL == newEcusArray->ecuInfo[i]->ecuReqId
            || NULL == newEcusArray->ecuInfo[i]->ecuSwDid) {
            log_w("newEcusArray->ecuInfo[%d] is NULL", i);
            continue;
        }
        (void)adm_memset(ecuInfoIdKey, 0x00, 64, 64);
        eimGeneratedEcuInfoIdKey(
            ecuInfoIdKey, 64, newEcusArray->ecuInfo[i]->ecuReqId, newEcusArray->ecuInfo[i]->ecuSwDid);
        boolRet = eimKvStorageRemoveKey(ecuInfoIdKey);
        if (FALSE == boolRet) {
            log_e("eimKvStorageRemoveKey return failed");
        }
    }
    return;
}

D32S adm_eim_getSingleEcuDidInfo(const ADM_ECS_ECUCONFIG_T *pstEcuCfg,
                                 const ADM_ECS_ECUDIDLIST_T *pstEcuDidList,
                                 const DCHAR *pcScriptPath, 
                                 ADM_ECS_DID_VALUE_LIST_T *pstDidList)
{
    D32S i = 0;
    ECU_INFO_T *pstEcuInfo = NULL;
    D32S iRet = ADM_ECODE_FAILED;
    ADM_DIM_SOA_ECU_IDEN_INFO_T stEcuIdenInfo = {0};
    
    ADM_COM_FUNC_ENTRY();

    stEcuIdenInfo.requestId  = (D32S)htoi(pstEcuCfg->ecuReqId);
    stEcuIdenInfo.responseId = (D32S)htoi(pstEcuCfg->ecuResponseId);
    stEcuIdenInfo.functionId = (D32S)htoi(pstEcuCfg->ecuFunctionId);
    stEcuIdenInfo.swDid      = (D32S)htoi(pstEcuCfg->ecuSwDid);
    
    log_d("ecuReqId = %s, ecuSwDid = %s", pstEcuCfg->ecuReqId, pstEcuCfg->ecuSwDid);
    iRet = adm_eim_getSingleEcuInfoForEcu(pstEcuCfg, pstEcuDidList, &stEcuIdenInfo, pcScriptPath, &pstEcuInfo);
    if (ADM_IS_ERR(iRet))
    {
        log_e("adm_eim_getSingleEcuInfoForEcu ret:error[%d]", iRet);
        return iRet;
    }
    
    pstDidList->iDidNum = pstEcuInfo->vdiDidListResponse->didArrayLength;
    log_d("did num = %d", pstDidList->iDidNum);
    for (i = 0; i < pstDidList->iDidNum; i++)
    {
        pstDidList->stValueList[i].pcDidValue = adm_strdup((DCHAR*)pstEcuInfo->vdiDidListResponse->didArray[i]->didData->v.valueS);
        log_d("did value = %s", pstDidList->stValueList[i].pcDidValue);
    }
    
    adm_eim_deleteEcuInfoBody(pstEcuInfo);

    ADM_COM_FUNC_EXIT();

    return iRet;
}