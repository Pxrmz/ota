/**
 * @file    adm_ecs_rsConfig.c
 * @brief   ecu remote server config service  
 * @details 获取OTA平台配置信息
 * @author  wangyanpeng
 * @version v0.1.0
 * @date    2020-11-06
 * @copyright www.abupdate.com
 */
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <pthread.h>

#include "adm_file.h"
#include "adm_typedefs.h"
#include "adm_string.h"
#include "adm_list.h"
#include "adm_memory.h"
#include "adm_ecs_log.h"
#include "adm_com_lib.h"
#include "adm_com_modDefine.h"
#include "adm_ecs_rsConfig.h"
#include "adm_ecs_parsePolicy.h"
#include "adm_com_parseFileObject.h"
#include "adm_com_kvstorage.h"
#include "adm_dpo_download.h"
#include "adm_vrs_registerInfo.h"
#include "adm_ecs_kvStorage.h"




#define ADM_COM_OTACFG_KVSTORAGE_NODE_INFO_KEY     "[ECS]_[sameNodeList]" /** 高低配置节点信息列表 */

#if 0
#endif

/*!
 * @brief 预分包信息
 */
typedef struct stEcsModuleInfo_
{
    DBOOL isInited;            //!< 模块是否初始化
    pthread_rwlock_t stRWlock; //!< 读写锁

    DBOOL isDataInited;        //!< 配置数据是否初始化
    ADM_ECS_RSCONFIGDATA_T *pstEcsConfig;  //!< ecs配置信息

    //!< 上层注册对外请求 api 
    ADM_ECS_GETREQCLOUDPARAM_CALLBACK_F        fnGetReqCloudParamApi;       //!< 获取云端配置请求参数
    ADM_OCS_BUSINESS_COMM_INTERFACE_CALLBACK_F fnReqCloudApi;               //!< 请求云端配置接口
    ADM_OCM_FILEDOWN_API_CALLBACK_F            fnDownloadApi;               //!< 文件下载接口
    ADM_ECS_REPORTEVENT_CALLBACK_F             fnReportEventApi;            ///< 事件上报接口
    ADM_ECS_DECRYPTANDSIGNATURE_CALLBACK_F     fnDecryptAndSignatureApi;    ///< 解密和验签接口
    ADM_ECS_FILESYNCTRANS_CALLBACK_F           fnFileSyncTransferApi;       ///< 文件传输接口
} ADM_ECS_MODULEINFO_T;

/**
 * @brief 相同配置ECU列表
 * @note
 */
typedef struct ADM_ECS_SAME_CFG_LIST_
{
    D32S num;
    struct list_head head;
    pthread_mutex_t mutex;
    DBOOL bIsInit;
} ADM_ECS_SAME_CFG_LIST_T;

/**
 * @brief 同一配置ECU节点
 * @note
 */
typedef struct ADM_ECS_SAME_CFG_NODE_
{
    DCHAR *ecuReqId;                    //!< 诊断请求ID 
    DCHAR *ecuSwDid;                    //!< 升级对象软件版本 did
    DBOOL bIsGetEcuInfo;                ///< 已获取到ECU信息
    DBOOL bIsCompareSucceed;            ///< 获取到的ECU信息是否比对成功
    ADM_ECS_DID_VALUE_LIST_T stDidList;
    struct list_head entry;
}ADM_ECS_SAME_CFG_NODE_T;

static ADM_ECS_MODULEINFO_T gstEcsModuleInfo = {0};

static ADM_ECS_SAME_CFG_LIST_T g_stSameCfgList = {0};

static DBOOL g_bIsUpdateGlobalConfig = FALSE;

#if 0
#endif

static void ecsSameCfgEcuListInit(void)
{
    ADM_COM_FUNC_ENTRY();

    if (FALSE == g_stSameCfgList.bIsInit)
    {
        g_stSameCfgList.num = 0;
        INIT_LIST_HEAD(&g_stSameCfgList.head);
        g_stSameCfgList.bIsInit = TRUE;
        pthread_mutex_init(&g_stSameCfgList.mutex, NULL);
    }
    else
    {
        log_w("list is init!!!");
    }

    ADM_COM_FUNC_EXIT();

    return;
}

void ecsSameCfgEcuNodeAddTail(struct list_head *entry)
{
    ADM_COM_FUNC_ENTRY();

    pthread_mutex_lock(&g_stSameCfgList.mutex);
    list_add_tail(entry, &g_stSameCfgList.head);
    g_stSameCfgList.num++;
    log_i("same cfg node num is %d", g_stSameCfgList.num);
    pthread_mutex_unlock(&g_stSameCfgList.mutex);

    ADM_COM_FUNC_EXIT();

    return;
}

static void ecsDidValueListFree(ADM_ECS_DID_VALUE_LIST_T *pstDidList)
{
    D32S i = 0;
    
    ADM_COM_FUNC_ENTRY();

    for (i = 0; i < pstDidList->iDidNum; i++)
    {
        adm_free(pstDidList->stValueList[i].pcDidValue);
    }

    ADM_COM_FUNC_EXIT();

    return;
}

static void ecsSameCfgEcuNodeDel(ADM_ECS_SAME_CFG_NODE_T *pstCfgNode)
{
    ADM_COM_FUNC_ENTRY();

    if (pstCfgNode)
    {
        adm_free(pstCfgNode->ecuReqId);
        adm_free(pstCfgNode->ecuSwDid);
    }

    ecsDidValueListFree(&pstCfgNode->stDidList);

    adm_free(pstCfgNode);

    ADM_COM_FUNC_EXIT();

    return;
}

static void ecsSameCfgEcuListDeInit(void)
{
    ADM_ECS_SAME_CFG_NODE_T *pos = NULL;
    ADM_ECS_SAME_CFG_NODE_T *next = NULL;
    
    ADM_COM_FUNC_ENTRY();

    pthread_mutex_lock(&g_stSameCfgList.mutex);

    list_for_each_entry_safe(pos, next, &g_stSameCfgList.head, ADM_ECS_SAME_CFG_NODE_T, entry)
    {
        if (ADM_IS_NULL(pos))
        {
            log_e("ADM_ECS_SAME_CFG_NODE_T list pos NULL");
            continue;
        }

        list_del(&pos->entry);
        g_stSameCfgList.num--;
        ecsSameCfgEcuNodeDel(pos);
    }

    pthread_mutex_unlock(&g_stSameCfgList.mutex);

    ADM_COM_FUNC_EXIT();

    return;
}

static ADM_ECS_SAME_CFG_NODE_T *ecsSameCfgEcuNodeNew(DCHAR *ecuReqId, DCHAR *ecuSwDid)
{
    ADM_ECS_SAME_CFG_NODE_T *pstCfgNode = NULL;
    
    ADM_COM_FUNC_ENTRY();

    if (ADM_IS_NULL(ecuReqId) || ADM_IS_NULL(ecuSwDid))
    {
        log_e("invalid param");
        return NULL;
    }
    
    pstCfgNode = (ADM_ECS_SAME_CFG_NODE_T *)adm_malloc(sizeof(ADM_ECS_SAME_CFG_NODE_T));
    if (ADM_IS_NULL(pstCfgNode))
    {
        log_e("memory apply failed");
        return NULL;
    }
    
    pstCfgNode->ecuReqId = adm_strdup(ecuReqId);
    pstCfgNode->ecuSwDid = adm_strdup(ecuSwDid);

    ADM_COM_FUNC_EXIT();

    return pstCfgNode;
}

static DBOOL ecsIsExistNode(DCHAR *ecuReqId, DCHAR *ecuSwDid)
{
    DBOOL bIsExist = FALSE;
    ADM_ECS_SAME_CFG_NODE_T *pos = NULL;
    ADM_ECS_SAME_CFG_NODE_T *next = NULL;
    
    ADM_COM_FUNC_ENTRY();

    if (ADM_IS_NULL(ecuReqId) || ADM_IS_NULL(ecuSwDid))
    {
        log_e("invalid param");
        return bIsExist;
    }

    log_i("pcEcuReqId = %s, pcEcuSwId = %s", ecuReqId, ecuSwDid);

    pthread_mutex_lock(&g_stSameCfgList.mutex);

    list_for_each_entry_safe(pos, next, &g_stSameCfgList.head, ADM_ECS_SAME_CFG_NODE_T, entry)
    {
        if (ADM_IS_NULL(pos))
        {
            log_e("ADM_ECS_SAME_CFG_NODE_T list pos NULL");
            continue;
        }

        if (ADM_IS_NULL(pos->ecuReqId) || 
            ADM_IS_NULL(pos->ecuSwDid))
        {
            log_e("ADM_ECS_SAME_CFG_NODE_T list pcReqId or pcSwId is NULL");
            continue;
        }

        if (0 == adm_strcasecmp(ecuReqId, pos->ecuReqId) &&
            0 == adm_strcasecmp(ecuSwDid, pos->ecuSwDid))
        {
            log_i("ecu pcEcuReqId = %s, pcEcuSwId = %s is exist!!!", pos->ecuReqId, pos->ecuSwDid);
            bIsExist = TRUE;
            break;
        }
    }

    pthread_mutex_unlock(&g_stSameCfgList.mutex);

    ADM_COM_FUNC_EXIT();

    return bIsExist;
}

static void ecsEcuDidListClone(ADM_ECS_DID_VALUE_LIST_T *pstSrcDidList, ADM_ECS_DID_VALUE_LIST_T *pstDstDidList)
{
    D32S i = 0;

    ADM_COM_FUNC_ENTRY();

    if (ADM_IS_NULL(pstSrcDidList) || ADM_IS_NULL(pstDstDidList))
    {
        log_e("invalid param");
        return ;
    }

    pstDstDidList->iDidNum = pstSrcDidList->iDidNum;

    for (i = 0; i < pstSrcDidList->iDidNum; i++)
    {
        pstDstDidList->stValueList[i].pcDidValue = adm_strdup(pstSrcDidList->stValueList[i].pcDidValue);
    }

    ADM_COM_FUNC_EXIT();

    return;
}

static void ecsNodeInfoClone(ADM_ECS_SAME_CFG_NODE_T *pstSrcNodeInfo, ADM_ECS_SAME_CFG_NODE_T *pstDstNodeInfo)
{
    ADM_COM_FUNC_ENTRY();

    if (ADM_IS_NULL(pstSrcNodeInfo) || ADM_IS_NULL(pstDstNodeInfo))
    {
        log_e("invalid param");
        return ;
    }

    pstDstNodeInfo->ecuReqId = adm_strdup(pstSrcNodeInfo->ecuReqId);
    pstDstNodeInfo->ecuSwDid = adm_strdup(pstSrcNodeInfo->ecuSwDid);
    pstDstNodeInfo->bIsGetEcuInfo = pstSrcNodeInfo->bIsGetEcuInfo;
    pstDstNodeInfo->bIsCompareSucceed = pstSrcNodeInfo->bIsCompareSucceed;
    ecsEcuDidListClone(&pstSrcNodeInfo->stDidList, &pstDstNodeInfo->stDidList);

    ADM_COM_FUNC_EXIT();

    return;
}

static ADM_ECS_SAME_CFG_NODE_T *ecsGetNodeInfo(DCHAR *ecuReqId, DCHAR *ecuSwDid)
{
    DBOOL bIsExist = FALSE;
    ADM_ECS_SAME_CFG_NODE_T *pos = NULL;
    ADM_ECS_SAME_CFG_NODE_T *next = NULL;
    ADM_ECS_SAME_CFG_NODE_T *pstNodeInfo = NULL;
    
    ADM_COM_FUNC_ENTRY();

    if (ADM_IS_NULL(ecuReqId) || ADM_IS_NULL(ecuSwDid))
    {
        log_e("invalid param");
        return NULL;
    }

    log_d("pcEcuReqId = %s, pcEcuSwId = %s", ecuReqId, ecuSwDid);

    pthread_mutex_lock(&g_stSameCfgList.mutex);

    list_for_each_entry_safe(pos, next, &g_stSameCfgList.head, ADM_ECS_SAME_CFG_NODE_T, entry)
    {
        if (ADM_IS_NULL(pos))
        {
            log_e("ADM_ECS_SAME_CFG_NODE_T list pos NULL");
            continue;
        }

        if (ADM_IS_NULL(pos->ecuReqId) || 
            ADM_IS_NULL(pos->ecuSwDid))
        {
            log_e("ADM_ECS_SAME_CFG_NODE_T list pcReqId or pcSwId is NULL");
            continue;
        }

        if (0 == adm_strcasecmp(ecuReqId, pos->ecuReqId) &&
            0 == adm_strcasecmp(ecuSwDid, pos->ecuSwDid))
        {
            pstNodeInfo = (ADM_ECS_SAME_CFG_NODE_T *)adm_malloc(sizeof(ADM_ECS_SAME_CFG_NODE_T));
            if (ADM_IS_NULL(pstNodeInfo))
            {
                log_e("memory apply failed");
            }
            else
            {
                ecsNodeInfoClone(pos, pstNodeInfo);
            }

            break;
        }
    }

    pthread_mutex_unlock(&g_stSameCfgList.mutex);

    ADM_COM_FUNC_EXIT();

    return pstNodeInfo;
}

static void ecsUpdateNodeInfo(DCHAR *ecuReqId, DCHAR *ecuSwDid, ADM_ECS_DID_VALUE_LIST_T *pstDidList, 
        DBOOL bIsUpdateDidList, DBOOL bIsCompareSucceed)
{
    ADM_ECS_SAME_CFG_NODE_T *pos = NULL;
    ADM_ECS_SAME_CFG_NODE_T *next = NULL;
    
    ADM_COM_FUNC_ENTRY();

    if (ADM_IS_NULL(ecuReqId) || ADM_IS_NULL(ecuSwDid) || ADM_IS_NULL(pstDidList))
    {
        log_e("invalid param");
        return ;
    }

    log_d("pcEcuReqId = %s, pcEcuSwId = %s updateFlag[%d] compareFlag[%d]", ecuReqId, ecuSwDid, bIsUpdateDidList, bIsCompareSucceed);

    pthread_mutex_lock(&g_stSameCfgList.mutex);

    list_for_each_entry_safe(pos, next, &g_stSameCfgList.head, ADM_ECS_SAME_CFG_NODE_T, entry)
    {
        if (ADM_IS_NULL(pos))
        {
            log_e("ADM_ECS_SAME_CFG_NODE_T list pos NULL");
            continue;
        }

        if (ADM_IS_NULL(pos->ecuReqId) || 
            ADM_IS_NULL(pos->ecuSwDid))
        {
            log_e("ADM_ECS_SAME_CFG_NODE_T list pcReqId or pcSwId is NULL");
            continue;
        }

        if (0 == adm_strcasecmp(ecuReqId, pos->ecuReqId) &&
            0 == adm_strcasecmp(ecuSwDid, pos->ecuSwDid))
        {
            if (DTRUE == bIsUpdateDidList)
            {
                pos->bIsGetEcuInfo = TRUE;
                ecsEcuDidListClone(pstDidList, &pos->stDidList);
            }

            pos->bIsCompareSucceed = bIsCompareSucceed;
            
            break;
        }
    }

    pthread_mutex_unlock(&g_stSameCfgList.mutex);

    ADM_COM_FUNC_EXIT();

    return ;
}

static D32S ecsDidValueCompare(ADM_ECS_EXTERN_ECU_INFO_T *pstExternEcuInfo,
                               ADM_ECS_DID_VALUE_LIST_T *pstDidList, 
                               DBOOL *pIsCompareSucceed)
{
    D32S iRet = ADM_ECODE_FAILED;
    DBOOL bIsPartNum = FALSE;
    DBOOL bIsHardwareNum = FALSE;
    DBOOL bIsSupplier = FALSE;
    DBOOL bIsGetDefault = FALSE;
    DBOOL bIsSoftwarePartNumber = FALSE;
    D32S i = 0;

    ADM_COM_FUNC_ENTRY();

    for (i = 0; i < pstDidList->iDidNum; i++)
    {
        if (0 == adm_strcmp("default", pstDidList->stValueList[i].pcDidValue))
        {
            bIsGetDefault = TRUE;
            break;
        }
    }


    for (i = 0; i < pstDidList->iDidNum; i++)
    {
        if (0 == adm_strcmp(pstExternEcuInfo->partNumber, pstDidList->stValueList[i].pcDidValue))
        {
            bIsPartNum = TRUE;
            break;
        }
    }

    for (i = 0; i < pstDidList->iDidNum; i++)
    {
        if (0 == adm_strcmp(pstExternEcuInfo->partSupplier, pstDidList->stValueList[i].pcDidValue))
        {
            bIsSupplier = TRUE;
            break;
        }
    }

    for (i = 0; i < pstDidList->iDidNum; i++)
    {
        if (0 == adm_strcmp(pstExternEcuInfo->partHardwareNum, pstDidList->stValueList[i].pcDidValue))
        {
            bIsHardwareNum = TRUE;
            break;
        }
    }

    if (NULL != pstExternEcuInfo->softwarePartNumber)
    {
        log_d("softwarePartNumber = %s", pstExternEcuInfo->softwarePartNumber);

        for (i = 0; i < pstDidList->iDidNum; i++)
        {
            if (0 == adm_strcmp(pstExternEcuInfo->softwarePartNumber, pstDidList->stValueList[i].pcDidValue))
            {
                bIsSoftwarePartNumber = TRUE;
                break;
            }
        }
    }
    else
    {
        bIsSoftwarePartNumber = TRUE;
    }

    if (TRUE == bIsPartNum && TRUE == bIsSupplier && TRUE == bIsHardwareNum && TRUE == bIsSoftwarePartNumber) 
    {
        log_i("Compare succeed.");
        iRet = ADM_ECODE_SUCCESSED;
        if (pIsCompareSucceed != NULL)
        {
            *pIsCompareSucceed = DTRUE;
        }
    }
    else
    {
        if (TRUE == bIsGetDefault)
        {
            log_d("get default did value");
            iRet = ADM_ECODE_SUCCESSED;
        }
    }

    if (ADM_IS_ERR(iRet))
    {
        log_d("expect partNumber = %s, partSupplier = %s, partHardwareNum = %s", pstExternEcuInfo->partNumber,
                                                                                 pstExternEcuInfo->partSupplier,
                                                                                 pstExternEcuInfo->partHardwareNum);
        log_d("real did value: = ");
        for (i = 0; i < pstDidList->iDidNum; i++)
        {
            log_d("value[%d] = %s", i, pstDidList->stValueList[i].pcDidValue);
        }
    }

    ADM_COM_FUNC_EXIT();

    return iRet;
}

static D32S ecsParseSameCfgDidObject(json_object *pEcuNode, ADM_ECS_DID_VALUE_LIST_T *pstDidList)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    json_object *pEcuDidArray = NULL;
    json_object *pEcuDid = NULL;
    D32S i = 0;

    ADM_COM_FUNC_ENTRY();

    if (ADM_IS_NULL(pEcuNode) || ADM_IS_NULL(pstDidList))
    {
        log_e("invalid param");
        return ADM_ECODE_INVALID_PARAM;
    }

    do
    {
        if (!json_object_object_get_ex(pEcuNode, ADM_COM_AIR_PROTOCOL_FIELD_ECUDIDS, &pEcuDidArray))
        {
            iRet = ADM_ECODE_JSON_GET_OBJECT_FAILED;
            log_i("Json node no key[%s]", ADM_COM_AIR_PROTOCOL_FIELD_ECUDIDS);
            break;
        }

        pstDidList->iDidNum = json_object_array_length(pEcuDidArray);
        for (i = 0; i < pstDidList->iDidNum; i++)
        {
            pEcuDid = json_object_array_get_idx(pEcuDidArray, (D32U)i);
            if (ADM_IS_NULL(pEcuDid)) 
            {
                iRet = ADM_ECODE_JSON_GET_ARRAYOBJECT_FAILED;
                log_e("Failed to Json array, arrayIndex[%d]", i);
                break;
            }

            pstDidList->stValueList[i].pcDidValue = adm_com_getJsonStrVal(pEcuDid, "value");
            if (ADM_IS_NULL(pstDidList->stValueList[i].pcDidValue))
            {
                iRet = ADM_ECODE_JSON_GET_KEY_FAILED;
                log_e("Failed to get json value, key[value]");
                break;
            }
        }
    } while (FALSE);

    ADM_COM_FUNC_EXIT();

    return iRet;
}

static void ecsSameCfgEcuListGetFromKv()
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    ADM_KV_STORAGE_VALUE_TYPE_E eValType = ADM_KV_STORAGE_VALUE_TYPE_INVALID;
    json_object *pEcuArray = NULL;
    json_object *pEcuNode = NULL;
    json_object *pEcuDidArray = NULL;
    DCHAR *pcEcuNodeInfo = NULL;
    size_t uiLength = 0;
    ADM_ECS_SAME_CFG_NODE_T *pstCfgNode = NULL;
    DCHAR *pcReqId = NULL;
    DCHAR *pcSwdid = NULL;
    DBOOL bIsOk = TRUE;
    D32S i = 0;

    ADM_COM_FUNC_ENTRY();

    do
    {
        bIsOk = adm_com_kvstorage_get_string(adm_core_service_getBusinessDatabaseDb(), ADM_COM_OTACFG_KVSTORAGE_NODE_INFO_KEY, 
                                             &pcEcuNodeInfo, &uiLength, &eValType);

        if (!bIsOk)
        {
            iRet = ADM_ECODE_KV_SET_STRING_FAILED;
            log_e( "Failed to get, key[%s]", ADM_COM_OTACFG_KVSTORAGE_NODE_INFO_KEY);
            break;
        }

        pEcuArray = json_tokener_parse(pcEcuNodeInfo);
        if (ADM_IS_NULL(pEcuArray)) 
        {
            log_e("Failed string to json, string[%s]", pcEcuNodeInfo);
            iRet = ADM_ECODE_STRING_TO_JSON_FAILED;
            break;
        }

        for (i = 0; i < json_object_array_length(pEcuArray); i++)
        {
            pEcuNode = json_object_array_get_idx(pEcuArray, (D32U)i);
            if (ADM_IS_NULL(pEcuNode)) 
            {
                iRet = ADM_ECODE_JSON_GET_ARRAYOBJECT_FAILED;
                log_e("Failed to Json array, arrayIndex[%d]", i);
                break;
            }

            pcReqId = adm_com_getJsonStrVal(pEcuNode, ADM_COM_AIR_PROTOCOL_FIELD_ECUREQID);
            if (ADM_IS_NULL(pcReqId))
            {
                iRet = ADM_ECODE_JSON_GET_KEY_FAILED;
                log_e("Failed to get json value, key[%s]", ADM_COM_AIR_PROTOCOL_FIELD_ECUREQID);
                break;
            }

            pcSwdid = adm_com_getJsonStrVal(pEcuNode, ADM_COM_AIR_PROTOCOL_FIELD_ECUSWDID);
            if (ADM_IS_NULL(pcSwdid))
            {
                iRet = ADM_ECODE_JSON_GET_KEY_FAILED;
                log_e("Failed to get json value, key[%s]", ADM_COM_AIR_PROTOCOL_FIELD_ECUSWDID);
                adm_free(pcReqId);
                break;
            }

            pstCfgNode = ecsSameCfgEcuNodeNew(pcReqId, pcSwdid);
            if (ADM_IS_NULL(pstCfgNode))
            {
                iRet = ADM_ECODE_JSON_GET_KEY_FAILED;
                log_e("ecsSameCfgEcuNodeNew Failed ");
                adm_free(pcReqId);
                adm_free(pcSwdid);
                break;
            }

            adm_free(pcReqId);
            adm_free(pcSwdid);
            iRet = adm_com_getJsonIntVal(pEcuNode, "getEcuInfoFlag", (D32S *)&pstCfgNode->bIsGetEcuInfo);
            if (ADM_IS_ERR(iRet))
            {
                log_e("get getEcuInfoFlag Failed");
                ecsSameCfgEcuNodeDel(pstCfgNode);
                pstCfgNode = NULL;
                break;
            }

            iRet = adm_com_getJsonIntVal(pEcuNode, "ecuInfoCompareResult", (D32S *)&pstCfgNode->bIsCompareSucceed);
            if (ADM_IS_ERR(iRet))
            {
                log_e("get ecuInfoCompareResult Failed");
                ecsSameCfgEcuNodeDel(pstCfgNode);
                pstCfgNode = NULL;
                break;
            }

            iRet = ecsParseSameCfgDidObject(pEcuNode, &pstCfgNode->stDidList);
            if (ADM_IS_ERR(iRet))
            {
                log_e("ecsParseSameCfgDidObject Failed");
                ecsSameCfgEcuNodeDel(pstCfgNode);
                pstCfgNode = NULL;
                break;
            }

            ecsSameCfgEcuNodeAddTail(&pstCfgNode->entry);

            pstCfgNode = NULL;
        }

    } while (FALSE);
    
    ADM_COM_FUNC_EXIT();

    return;
}

static json_object *ecsCreateSameCfgDidObject(ADM_ECS_DID_VALUE_LIST_T *pstDidList)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    D32S i = 0;
    json_object *pEcuDidArray = NULL;
    json_object *pEcuDid = NULL;

    ADM_COM_FUNC_ENTRY();

    if (ADM_IS_NULL(pstDidList))
    {
        log_e("invalid param");
        return NULL;
    }

    pEcuDidArray = json_object_new_array();
    if (ADM_IS_NULL(pEcuDidArray))
    {
        log_e("new array failed");
        return pEcuDidArray;
    }

    for (i = 0; i < pstDidList->iDidNum; i++)
    {
        pEcuDid = json_object_new_object();
        if (ADM_IS_NULL(pEcuDid))
        {
            log_e("new array failed");
            continue ;
        }

        iRet = json_object_object_add(pEcuDid, ADM_COM_AIR_PROTOCOL_FIELD_VALUE, json_object_new_string(pstDidList->stValueList[i].pcDidValue));
        if (ADM_IS_ERR(iRet))
        {
            iRet = ADM_ECODE_JSON_ADD_STRING_FAILED;
            log_e("Failed to add value json object, err[%d]", iRet);
            (void)json_object_put(pEcuDid);
            break;
        }

        iRet = json_object_array_add(pEcuDidArray, pEcuDid);
        if (ADM_IS_ERR(iRet))
        {
            iRet = ADM_ECODE_JSON_ADD_INT_FAILED;
            log_e("Failed to add ecuDids json object, err[%d]", iRet);
            (void)json_object_put(pEcuDid);
        }
    }

    if (ADM_IS_ERR(iRet))
    {
        (void)json_object_put(pEcuDidArray);
        pEcuDidArray = NULL;
    }

    ADM_COM_FUNC_EXIT();

    return pEcuDidArray;
}

static void ecsSameCfgEcuListSetToKv(void)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    ADM_ECS_SAME_CFG_NODE_T *pos = NULL;
    ADM_ECS_SAME_CFG_NODE_T *next = NULL;
    json_object *pEcuArray = NULL;
    json_object *pEcuNode = NULL;
    json_object *pEcuDidArray = NULL;
    DCHAR *pcEcuNodeInfo = NULL;
    DBOOL bIsOk = TRUE;

    ADM_COM_FUNC_ENTRY();

    pEcuArray = json_object_new_array();
    if (ADM_IS_NULL(pEcuArray))
    {
        log_e("new array failed");
        return ;
    }

    pthread_mutex_lock(&g_stSameCfgList.mutex);

    list_for_each_entry_safe(pos, next, &g_stSameCfgList.head, ADM_ECS_SAME_CFG_NODE_T, entry)
    {
        if (ADM_IS_NULL(pos))
        {
            log_e("ADM_ECS_SAME_CFG_NODE_T list pos NULL");
            continue;
        }

        pEcuNode = json_object_new_object();
        if (ADM_IS_NULL(pEcuNode))
        {
            log_e("new array failed");
            continue ;
        }

        iRet = json_object_object_add(pEcuNode, ADM_COM_AIR_PROTOCOL_FIELD_ECUREQID, json_object_new_string(pos->ecuReqId));
        if (ADM_IS_ERR(iRet))
        {
            iRet = ADM_ECODE_JSON_ADD_STRING_FAILED;
            log_e("Failed to add ecuReqId json object, err[%d]", iRet);
            (void)json_object_put(pEcuNode);
            break;
        }

        iRet = json_object_object_add(pEcuNode, ADM_COM_AIR_PROTOCOL_FIELD_ECUSWDID, json_object_new_string(pos->ecuSwDid));
        if (ADM_IS_ERR(iRet))
        {
            iRet = ADM_ECODE_JSON_ADD_STRING_FAILED;
            log_e("Failed to add ecuSwDid json object, err[%d]", iRet);
            (void)json_object_put(pEcuNode);
            break;
        }

        iRet = json_object_object_add(pEcuNode, "getEcuInfoFlag", json_object_new_int((D32S)pos->bIsGetEcuInfo));
        if (ADM_IS_ERR(iRet))
        {
            iRet = ADM_ECODE_JSON_ADD_INT_FAILED;
            log_e("Failed to add getEcuInfoFlag json object, err[%d]", iRet);
            (void)json_object_put(pEcuNode);
            break;
        }

        iRet = json_object_object_add(pEcuNode, "ecuInfoCompareResult", json_object_new_int((D32S)pos->bIsCompareSucceed));
        if (ADM_IS_ERR(iRet))
        {
            iRet = ADM_ECODE_JSON_ADD_INT_FAILED;
            log_e("Failed to add ecuInfoCompareResult json object, err[%d]", iRet);
            (void)json_object_put(pEcuNode);
            break;
        }

        pEcuDidArray = ecsCreateSameCfgDidObject(&pos->stDidList);
        if (ADM_IS_NULL(pEcuDidArray))
        {
            log_e("new array failed");
            (void)json_object_put(pEcuNode);
            continue ;
        }

        iRet = json_object_object_add(pEcuNode, ADM_COM_AIR_PROTOCOL_FIELD_ECUDIDS, pEcuDidArray);
        if (ADM_IS_ERR(iRet))
        {
            iRet = ADM_ECODE_JSON_ADD_INT_FAILED;
            log_e("Failed to add ecuDids json object, err[%d]", iRet);
            (void)json_object_put(pEcuDidArray);
            (void)json_object_put(pEcuNode);
            continue;
        }

        iRet = json_object_array_add(pEcuArray, pEcuNode);
        if (ADM_IS_ERR(iRet))
        {
            iRet = ADM_ECODE_JSON_ADD_INT_FAILED;
            log_e("Failed to add ecuDids json object, err[%d]", iRet);
            (void)json_object_put(pEcuNode);
        }
    }

    pthread_mutex_unlock(&g_stSameCfgList.mutex);

    if (ADM_IS_SUCCESS(iRet))
    {
        pcEcuNodeInfo = (DCHAR *)json_object_to_json_string(pEcuArray);
        log_d("pcEcuNodeInfo = %s", pcEcuNodeInfo);
        bIsOk = adm_com_kvstorage_set_string(adm_core_service_getBusinessDatabaseDb(), 
                                             ADM_COM_OTACFG_KVSTORAGE_NODE_INFO_KEY, 
                                             pcEcuNodeInfo, 
                                             ADM_KV_STORAGE_VALUE_TYPE_STRING);
        if (!bIsOk)
        {
            iRet = ADM_ECODE_KV_SET_STRING_FAILED;
            log_e( "Failed to set, key[%s]", ADM_COM_OTACFG_KVSTORAGE_NODE_INFO_KEY);
        }
    }

    if (NULL != pEcuArray)
    {
        json_object_put(pEcuArray);
    }

    ADM_COM_FUNC_EXIT();

    return ;
}

/*!
 *******************************************************************************
 * 生成ecu配置信息结构.
 *
 * \note 
 * 
 * \retval ADM_ECS_ECUCONFIG_T *
 *******************************************************************************
 */
ADM_ECS_ECUCONFIG_T *adm_ecs_newEcuConfigInfo(void)
{
    ADM_ECS_ECUCONFIG_T *pstEcuCfgInfo = NULL;

    pstEcuCfgInfo = (ADM_ECS_ECUCONFIG_T *)adm_malloc(sizeof(*pstEcuCfgInfo));
    if (NULL != pstEcuCfgInfo)
    {
        (void)adm_memzero(pstEcuCfgInfo, sizeof(*pstEcuCfgInfo));
    }

    return pstEcuCfgInfo;
}

/*!
 *******************************************************************************
 * 释放ecu配置信息结构.
 *
 * \note
 * 
 * \param[inout]  pstEcuCfgInfo ecu配置信息
 * \retval      D32S
 *******************************************************************************
 */
void adm_ecs_delEcuConfigInfo(ADM_ECS_ECUCONFIG_T **pstEcuCfgInfo)
{
    if (ADM_IS_NULL(pstEcuCfgInfo) || ADM_IS_NULL(*pstEcuCfgInfo))
    {
        return;
    }

    adm_free((*pstEcuCfgInfo)->ecuReqId);
    adm_free((*pstEcuCfgInfo)->ecuSwDid);
    adm_free((*pstEcuCfgInfo)->ecuResponseId);
    adm_free((*pstEcuCfgInfo)->ecuFunctionId);
    adm_free((*pstEcuCfgInfo)->readInfoInstanceId);
    adm_free((*pstEcuCfgInfo)->downloadInstanceId);        
    adm_free((*pstEcuCfgInfo)->transferInstanceId);
    adm_free((*pstEcuCfgInfo)->installInstanceId);

    adm_com_cleanExtsList(&(*pstEcuCfgInfo)->stExtList);
    adm_free((*pstEcuCfgInfo)->saLibNumberList.psaLibNumber);
    adm_free((*pstEcuCfgInfo)->crcLibNumberList.pcrcLibNumber);
    //!< 该ecu特有的ResourceDid信息
    adm_ecs_delEcuDidList((*pstEcuCfgInfo)->stEcuDidList.num, 
            &(*pstEcuCfgInfo)->stEcuDidList.pstEcuDidInfo);
    
    adm_free(*pstEcuCfgInfo);

    return;
}



/*!
 *******************************************************************************
 * 生成配置信息结构.
 *
 * \note 
 * 
 * \retval ADM_ECS_RSCONFIGINFO_T
 *******************************************************************************
 */
ADM_ECS_RSCONFIGINFO_T *adm_ecs_newRSConfigInfo(void)
{
    ADM_ECS_RSCONFIGINFO_T *pstRsCfgInfo = NULL;

    pstRsCfgInfo = (ADM_ECS_RSCONFIGINFO_T *)adm_malloc(sizeof(*pstRsCfgInfo));
    if (NULL != pstRsCfgInfo)
    {
        (void)adm_memset(pstRsCfgInfo, 0x00, sizeof(*pstRsCfgInfo), sizeof(*pstRsCfgInfo));
    }

    return pstRsCfgInfo;
}

/*!
 *******************************************************************************
 * 释放配置信息结构.
 *
 * \note
 * 
 * \param[out]  pstRsCfgInfo    OTA平台配置信息
 * \retval      D32S
 *******************************************************************************
 */
void adm_ecs_delRSConfigInfo(ADM_ECS_RSCONFIGINFO_T **pstRsCfgInfo)
{
    if (ADM_IS_NULL(pstRsCfgInfo) || ADM_IS_NULL(*pstRsCfgInfo))
    {
        return;
    }

    adm_ecs_cleanPartConfig(&(*pstRsCfgInfo)->stPartCfg);
    adm_ecs_cleanGeneralConfig(&(*pstRsCfgInfo)->stGeneralCfg);
    adm_free((*pstRsCfgInfo)->secret);
    adm_free(*pstRsCfgInfo);

    return;
}

/*!
 *******************************************************************************
 * 清空配置信息结构.
 *
 * \note 不释放最外层的节点内存
 * 
 * \param[out]  pstRsCfgInfo    OTA平台配置信息
 * \retval      D32S
 *******************************************************************************
 */
void adm_ecs_cleanRSConfigInfo(ADM_ECS_RSCONFIGINFO_T *pstRsCfgInfo)
{
    if (ADM_IS_NULL(pstRsCfgInfo))
    {
        return;
    }

    adm_ecs_cleanPartConfig(&pstRsCfgInfo->stPartCfg);
    adm_ecs_cleanGeneralConfig(&pstRsCfgInfo->stGeneralCfg);

    return;
}


/*!
 *******************************************************************************
 * 生成基础配置信息结构.
 *
 * \note 
 * 
 * \retval ADM_ECS_BASECONFIGINFO_T
 *******************************************************************************
 */
static ADM_ECS_BASECONFIGINFO_T *__adm_ecs_newBaseCfgInfo(void)
{
    ADM_ECS_BASECONFIGINFO_T *pstBaseCfgInfo = NULL;

    pstBaseCfgInfo = (ADM_ECS_BASECONFIGINFO_T *)adm_malloc(sizeof(*pstBaseCfgInfo));
    if (NULL != pstBaseCfgInfo)
    {
        (void)adm_memset(pstBaseCfgInfo, 0x00, sizeof(*pstBaseCfgInfo), sizeof(*pstBaseCfgInfo));
    }

    return pstBaseCfgInfo;
}

/*!
 *******************************************************************************
 * 释放基础配置信息结构.
 *
 * \note
 * 
 * \param[out]  pstBaseCfgInfo 基础配置信息
 * \retval      D32S
 *******************************************************************************
 */
static void __adm_ecs_delBaseCfgInfo(ADM_ECS_BASECONFIGINFO_T **pstBaseCfgInfo)
{
    if (ADM_IS_NULL(pstBaseCfgInfo) || ADM_IS_NULL(*pstBaseCfgInfo))
    {
        return;
    }

    adm_free((*pstBaseCfgInfo)->stReqParam.secret);
    adm_free((*pstBaseCfgInfo)->stReqParam.deviceId);
    adm_free((*pstBaseCfgInfo)->stReqParam.sdkVersion);
    adm_free((*pstBaseCfgInfo)->stReqParam.confFileDir);
    adm_free(*pstBaseCfgInfo);

    return;
}


/*!
 *******************************************************************************
 * 生成配置数据信息结构.
 *
 * \note 
 * 
 * \retval ADM_ECS_RSCONFIGDATA_T
 *******************************************************************************
 */
ADM_ECS_RSCONFIGDATA_T *adm_ecs_newRsCfgDataInfo(void)
{
    ADM_ECS_RSCONFIGDATA_T *pstCfgDataInfo = NULL;

    pstCfgDataInfo = (ADM_ECS_RSCONFIGDATA_T *)adm_malloc(sizeof(*pstCfgDataInfo));
    if (NULL != pstCfgDataInfo)
    {
        (void)adm_memzero(pstCfgDataInfo, sizeof(*pstCfgDataInfo));
    }

    return pstCfgDataInfo;
}

/*!
 *******************************************************************************
 * 释放配置数据信息结构.
 *
 * \note
 * 
 * \param[in]  pstCfgDataInfo 配置数据信息
 * \retval     D32S
 *******************************************************************************
 */
void adm_ecs_delRsCfgDataInfo(ADM_ECS_RSCONFIGDATA_T **pstCfgDataInfo)
{
    if (ADM_IS_NULL(pstCfgDataInfo) || ADM_IS_NULL(*pstCfgDataInfo))
    {
        return;
    }

    adm_free((*pstCfgDataInfo)->confVersion);
    adm_com_cleanFileObject(&(*pstCfgDataInfo)->stRsCfgFileObj);
    adm_ecs_cleanRSConfigInfo(&(*pstCfgDataInfo)->stRsCfgInfo);
    adm_free(*pstCfgDataInfo);

    return;
}

/*!
 *******************************************************************************
 * 清空配置数据信息结构.
 *
 * \note
 * 
 * \param[in]  pstCfgDataInfo 配置数据信息
 * \retval     D32S
 *******************************************************************************
 */
void adm_ecs_cleanRsCfgDataInfo(ADM_ECS_RSCONFIGDATA_T *pstCfgDataInfo)
{
    if (ADM_IS_NULL(pstCfgDataInfo))
    {
        return;
    }

    adm_free(pstCfgDataInfo->confVersion);
    adm_com_cleanFileObject(&pstCfgDataInfo->stRsCfgFileObj);
    adm_ecs_cleanRSConfigInfo(&pstCfgDataInfo->stRsCfgInfo);

    return;
}


#if 0
#endif


/*!
 *******************************************************************************
 * ecs填充下载配置文件参数.
 *
 * \note
 *  
 * \param[in]   pstRsCfgFileObj  配置文件信息
 * \param[out]  pstDownReqInfo   下载请求参数
 * \retval      D32S
 *******************************************************************************
 */
static void __adm_ecs_fillDownReqParamInfo
(
    ADM_COM_FILEOBJECT_T        *pstRsCfgFileObj,
    ADM_DPO_DOWNLOAD_REQPARAM_T *pstDownReqInfo
)
{
    
    if (ADM_IS_NULL(pstRsCfgFileObj) || ADM_IS_NULL(pstDownReqInfo))
    {
        log_e("Invalid input param");
        return;
    }

    pstDownReqInfo->url = adm_strdup(pstRsCfgFileObj->url);
    pstDownReqInfo->filePath = adm_strdup(pstRsCfgFileObj->filePath);
    pstDownReqInfo->hash = adm_strdup(pstRsCfgFileObj->hash);

    pstDownReqInfo->fileSize = pstRsCfgFileObj->size;
    pstDownReqInfo->downRetriesCnt = 3;
    pstDownReqInfo->IsResumeDown = TRUE;
    pstDownReqInfo->IsUnlimitretry = FALSE;
    pstDownReqInfo->networkRetriesCnt = 0;
    pstDownReqInfo->priv = NULL;
    pstDownReqInfo->fnProgressNotify = NULL;
    pstDownReqInfo->fnDownloadCb = gstEcsModuleInfo.fnDownloadApi; //!< 上层注册的下载接口

    return;
}

/*!
 *******************************************************************************
 * 填充资源id list信息.
 *
 * \note
 * 
 * \param[out]  pstDstList 目的资源id list信息
 * \param[in]   pstSrcList 源资源id list信息
 * \retval      D32S
 *******************************************************************************
 */
static D32S __adm_ecs_fillEcuDidList
(
    ADM_ECS_ECUDIDLIST_T *pstDstList,
    ADM_ECS_ECUDIDLIST_T *pstSrcList
)
{
    D32S i = 0;

    if (ADM_IS_NULL(pstDstList) || ADM_IS_NULL(pstSrcList))
    {
        log_e("Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }

    //!< 申请相应内存空间
    pstDstList->num = pstSrcList->num;
    pstDstList->pstEcuDidInfo = adm_ecs_newEcuDidList(pstSrcList->num);
    if ((0 < pstDstList->num) && ADM_IS_NULL(pstDstList->pstEcuDidInfo))
    {
        log_e("Failed to new ResDidIlist, err[%d]", ADM_ECODE_NO_MEMORY);
        return ADM_ECODE_NO_MEMORY;
    }
    
    //!< 赋值
    for (i = 0; i < pstSrcList->num; i++)
    {
        pstDstList->pstEcuDidInfo[i].did   = adm_strdup(pstSrcList->pstEcuDidInfo[i].did);
        pstDstList->pstEcuDidInfo[i].end   = pstSrcList->pstEcuDidInfo[i].end;
        pstDstList->pstEcuDidInfo[i].start = pstSrcList->pstEcuDidInfo[i].start;
        pstDstList->pstEcuDidInfo[i].didValType = pstSrcList->pstEcuDidInfo[i].didValType;
    }

    return ADM_ECODE_SUCCESSED;
}

static D32S __adm_ecs_fillEcuSaMaskList
(
    ADM_ECS_SA_MASK_LIST_T* pstDstSaMaskList,
    ADM_ECS_SA_MASK_LIST_T* pstSrcSaMaskList
)
{
    if (NULL != pstDstSaMaskList && NULL != pstSrcSaMaskList)
    {

        pstDstSaMaskList->num = pstSrcSaMaskList->num;

        if (0 < pstDstSaMaskList->num)
        {
            pstDstSaMaskList->saMaskInfo = (ADM_ECS_SA_MASK_T*)adm_malloc(pstDstSaMaskList->num * sizeof(ADM_ECS_SA_MASK_T));

            for (D8U i = 0; i < pstDstSaMaskList->num; i++)
            {
                pstDstSaMaskList->saMaskInfo[i].saMaskLevel = pstSrcSaMaskList->saMaskInfo[i].saMaskLevel;
                pstDstSaMaskList->saMaskInfo[i].saMaskValue = adm_strdup(pstSrcSaMaskList->saMaskInfo[i].saMaskValue);
            }
        }
    }

    return ADM_ECODE_SUCCESSED;
}

/*!
 *******************************************************************************
 * 填充ecu安全算法脚本编号列表信息.
 *
 * \note
 * 
 * \param[out]  pstDstSaLibNumberList 目的安全算法脚本编号 list信息
 * \param[in]   pstSrcSaLibNumberList 源安全算法脚本编号 list信息
 * \retval      D32S
 *******************************************************************************
 */
static D32S __adm_ecs_fillEcuSaLibNumberList
(
    ADM_ECS_SALIBNUMBERLIST_T *pstDstSaLibNumberList,
    ADM_ECS_SALIBNUMBERLIST_T *pstSrcSaLibNumberList
)
{
    D32S i = 0;

    if (ADM_IS_NULL(pstDstSaLibNumberList) || ADM_IS_NULL(pstSrcSaLibNumberList))
    {
        log_e("Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }

    //!< 申请相应内存空间
    pstDstSaLibNumberList->num = pstSrcSaLibNumberList->num;
    pstDstSaLibNumberList->psaLibNumber = adm_ecs_newSaLibNumberList(pstSrcSaLibNumberList->num);
    if ((0 < pstDstSaLibNumberList->num) && ADM_IS_NULL(pstDstSaLibNumberList->psaLibNumber))
    {
        log_e("Failed to new ResDidIlist, err[%d]", ADM_ECODE_NO_MEMORY);
        return ADM_ECODE_NO_MEMORY;
    }
    
    //!< 赋值
    for (i = 0; i < pstSrcSaLibNumberList->num; i++)
    {
        pstDstSaLibNumberList->psaLibNumber[i] = pstSrcSaLibNumberList->psaLibNumber[i];            
    }

    return ADM_ECODE_SUCCESSED;
}

/*!
 *******************************************************************************
 * 填充ecu CRC算法脚本编号列表信息.
 *
 * \note
 * 
 * \param[out]  pstDstCrcLibNumberList 目的CRC算法脚本编号 list信息
 * \param[in]   pstSrcCrcLibNumberList 源CRC算法脚本编号 list信息
 * \retval      D32S
 *******************************************************************************
 */
static D32S __adm_ecs_fillEcuCrcLibNumberList
(
    ADM_ECS_CRCLIBNUMBERLIST_T *pstDstCrcLibNumberList,
    ADM_ECS_CRCLIBNUMBERLIST_T *pstSrcCrcLibNumberList
)
{

    D32S i = 0;

    if (ADM_IS_NULL(pstDstCrcLibNumberList) || ADM_IS_NULL(pstSrcCrcLibNumberList))
    {
        log_e("Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }

    //!< 申请相应内存空间
    pstDstCrcLibNumberList->num = pstSrcCrcLibNumberList->num;
    pstDstCrcLibNumberList->pcrcLibNumber = adm_ecs_newCrcLibNumberList(pstSrcCrcLibNumberList->num);
    if ((0 < pstDstCrcLibNumberList->num) && ADM_IS_NULL(pstDstCrcLibNumberList->pcrcLibNumber))
    {
        log_e("Failed to new ResDidIlist, err[%d]", ADM_ECODE_NO_MEMORY);
        return ADM_ECODE_NO_MEMORY;
    }
    
    //!< 赋值
    for (i = 0; i < pstSrcCrcLibNumberList->num; i++)
    {
        pstDstCrcLibNumberList->pcrcLibNumber[i] = pstSrcCrcLibNumberList->pcrcLibNumber[i];
    }

    return ADM_ECODE_SUCCESSED;
}


/*!
 *******************************************************************************
 * 填充ecus配置信息.
 *
 * \note
 * 
 * \param[out]  pstDstConfig 目的模块配置结构
 * \param[in]   pstSrcConfig 源模块配置结构
 * \retval      D32S
 *******************************************************************************
 */
static D32S __adm_ecs_fillPartConfig
(
    ADM_ECS_PARTCONFIG_T *pstDstConfig,
    ADM_ECS_PARTCONFIG_T *pstSrcConfig

)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    D32S i = 0;

    if (ADM_IS_NULL(pstDstConfig) || ADM_IS_NULL(pstSrcConfig))
    {
        log_e("Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }

    //!< 申请相应内存空间
    do
    {
        //!< copy exts
        iRet = adm_com_copyExtsObject(&pstDstConfig->stExtList, &pstSrcConfig->stExtList);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to copy exts info, err[%d]", iRet);
            break;
        }
    
        pstDstConfig->stEcuCfgList.num = pstSrcConfig->stEcuCfgList.num;
        pstDstConfig->stEcuCfgList.pstEcuCfg = adm_ecs_newEcusConfigList(pstDstConfig->stEcuCfgList.num);
        if ((0 < pstDstConfig->stEcuCfgList.num) && 
            ADM_IS_NULL(pstDstConfig->stEcuCfgList.pstEcuCfg))
        {
            iRet = ADM_ECODE_NO_MEMORY;
            log_e("Failed to new ecu config list, err[%d]", iRet);
            break;
        }

        //!< 赋值
        for (i = 0; i < pstSrcConfig->stEcuCfgList.num; i++)
        {
            pstDstConfig->stEcuCfgList.pstEcuCfg[i].ecuReqId = 
                adm_strdup(pstSrcConfig->stEcuCfgList.pstEcuCfg[i].ecuReqId);
            pstDstConfig->stEcuCfgList.pstEcuCfg[i].ecuSwDid = 
                adm_strdup(pstSrcConfig->stEcuCfgList.pstEcuCfg[i].ecuSwDid);
            pstDstConfig->stEcuCfgList.pstEcuCfg[i].ecuResponseId = 
                adm_strdup(pstSrcConfig->stEcuCfgList.pstEcuCfg[i].ecuResponseId);
            pstDstConfig->stEcuCfgList.pstEcuCfg[i].ecuFunctionId = 
                adm_strdup(pstSrcConfig->stEcuCfgList.pstEcuCfg[i].ecuFunctionId);
            pstDstConfig->stEcuCfgList.pstEcuCfg[i].readInfoInstanceId = 
                adm_strdup(pstSrcConfig->stEcuCfgList.pstEcuCfg[i].readInfoInstanceId);
            pstDstConfig->stEcuCfgList.pstEcuCfg[i].downloadInstanceId = 
                adm_strdup(pstSrcConfig->stEcuCfgList.pstEcuCfg[i].downloadInstanceId);
            pstDstConfig->stEcuCfgList.pstEcuCfg[i].transferInstanceId = 
                adm_strdup(pstSrcConfig->stEcuCfgList.pstEcuCfg[i].transferInstanceId);
            pstDstConfig->stEcuCfgList.pstEcuCfg[i].installInstanceId = 
                adm_strdup(pstSrcConfig->stEcuCfgList.pstEcuCfg[i].installInstanceId);
            pstDstConfig->stEcuCfgList.pstEcuCfg[i].stExternEcuInfo.partName = 
                adm_strdup(pstSrcConfig->stEcuCfgList.pstEcuCfg[i].stExternEcuInfo.partName);
            pstDstConfig->stEcuCfgList.pstEcuCfg[i].stExternEcuInfo.partNumber = 
                adm_strdup(pstSrcConfig->stEcuCfgList.pstEcuCfg[i].stExternEcuInfo.partNumber);
            pstDstConfig->stEcuCfgList.pstEcuCfg[i].stExternEcuInfo.partSwid = 
                adm_strdup(pstSrcConfig->stEcuCfgList.pstEcuCfg[i].stExternEcuInfo.partSwid);
            pstDstConfig->stEcuCfgList.pstEcuCfg[i].stExternEcuInfo.softwarePartNumber = 
                adm_strdup(pstSrcConfig->stEcuCfgList.pstEcuCfg[i].stExternEcuInfo.softwarePartNumber);
            pstDstConfig->stEcuCfgList.pstEcuCfg[i].stExternEcuInfo.partSupplier = 
                adm_strdup(pstSrcConfig->stEcuCfgList.pstEcuCfg[i].stExternEcuInfo.partSupplier);
            pstDstConfig->stEcuCfgList.pstEcuCfg[i].stExternEcuInfo.partHardwareNum = 
                adm_strdup(pstSrcConfig->stEcuCfgList.pstEcuCfg[i].stExternEcuInfo.partHardwareNum);
            pstDstConfig->stEcuCfgList.pstEcuCfg[i].stExternEcuInfo.upgradeOrder = 
                adm_strdup(pstSrcConfig->stEcuCfgList.pstEcuCfg[i].stExternEcuInfo.upgradeOrder);
            
            pstDstConfig->stEcuCfgList.pstEcuCfg[i].selfBackup = 
                pstSrcConfig->stEcuCfgList.pstEcuCfg[i].selfBackup;
            pstDstConfig->stEcuCfgList.pstEcuCfg[i].isSelfUpdate = 
                pstSrcConfig->stEcuCfgList.pstEcuCfg[i].isSelfUpdate;
            pstDstConfig->stEcuCfgList.pstEcuCfg[i].busType = 
                pstSrcConfig->stEcuCfgList.pstEcuCfg[i].busType;
            pstDstConfig->stEcuCfgList.pstEcuCfg[i].channelType = 
                pstSrcConfig->stEcuCfgList.pstEcuCfg[i].channelType;
            pstDstConfig->stEcuCfgList.pstEcuCfg[i].crcMask = 
                pstSrcConfig->stEcuCfgList.pstEcuCfg[i].crcMask;
            pstDstConfig->stEcuCfgList.pstEcuCfg[i].crcLibNumber = 
                pstSrcConfig->stEcuCfgList.pstEcuCfg[i].crcLibNumber;
            pstDstConfig->stEcuCfgList.pstEcuCfg[i].saMask = 
                adm_strdup(pstSrcConfig->stEcuCfgList.pstEcuCfg[i].saMask);
            pstDstConfig->stEcuCfgList.pstEcuCfg[i].flashProgressType = 
                pstSrcConfig->stEcuCfgList.pstEcuCfg[i].flashProgressType;
            pstDstConfig->stEcuCfgList.pstEcuCfg[i].flashScriptNumber = 
                pstSrcConfig->stEcuCfgList.pstEcuCfg[i].flashScriptNumber;
            pstDstConfig->stEcuCfgList.pstEcuCfg[i].isCGWRouted = 
                pstSrcConfig->stEcuCfgList.pstEcuCfg[i].isCGWRouted;
            pstDstConfig->stEcuCfgList.pstEcuCfg[i].serviceType = 
                pstSrcConfig->stEcuCfgList.pstEcuCfg[i].serviceType;
            pstDstConfig->stEcuCfgList.pstEcuCfg[i].instCondition = 
                pstSrcConfig->stEcuCfgList.pstEcuCfg[i].instCondition;
            pstDstConfig->stEcuCfgList.pstEcuCfg[i].securityLevel = 
                pstSrcConfig->stEcuCfgList.pstEcuCfg[i].securityLevel;

            pstDstConfig->stEcuCfgList.pstEcuCfg[i].saMaskList = (ADM_ECS_SA_MASK_LIST_T*)adm_malloc(sizeof(ADM_ECS_SA_MASK_LIST_T));
            iRet = __adm_ecs_fillEcuSaMaskList(pstDstConfig->stEcuCfgList.pstEcuCfg[i].saMaskList, 
                            pstSrcConfig->stEcuCfgList.pstEcuCfg[i].saMaskList);
            if (ADM_IS_ERR(iRet))
            {
                log_e("Failed to fill saMask list, err[%d]", iRet);
                break;
            }

            iRet = __adm_ecs_fillEcuSaLibNumberList(&pstDstConfig->stEcuCfgList.pstEcuCfg[i].saLibNumberList, 
                            &pstSrcConfig->stEcuCfgList.pstEcuCfg[i].saLibNumberList);
            if (ADM_IS_ERR(iRet))
            {
                log_e("Failed to fill saLibNumber list, err[%d]", iRet);
                break;
            }

            iRet = __adm_ecs_fillEcuCrcLibNumberList(&pstDstConfig->stEcuCfgList.pstEcuCfg[i].crcLibNumberList, 
                            &pstSrcConfig->stEcuCfgList.pstEcuCfg[i].crcLibNumberList);
            if (ADM_IS_ERR(iRet))
            {
                log_e("Failed to fill crcLibNumber list, err[%d]", iRet);
                break;
            }

            iRet = __adm_ecs_fillEcuDidList(&pstDstConfig->stEcuCfgList.pstEcuCfg[i].stEcuDidList, 
                            &pstSrcConfig->stEcuCfgList.pstEcuCfg[i].stEcuDidList);
            if (ADM_IS_ERR(iRet))
            {
                log_e("Failed to fill resource did list, err[%d]", iRet);
                break;
            }
        }

        //!< 若for-break，则while-break
        if (ADM_IS_ERR(iRet))
        {
            break;
        }
    } while (FALSE);
    
    if (ADM_IS_ERR(iRet))
    {
        adm_ecs_cleanPartConfig(pstDstConfig);
        return iRet;
    }

    return ADM_ECODE_SUCCESSED;
}

/*!
 *******************************************************************************
 * 填充通用配置信息.
 *
 * \note
 * 
 * \param[out]  pstDstConfig 目的模块配置结构
 * \param[in]   pstSrcConfig 源模块配置结构
 * \retval      D32S
 *******************************************************************************
 */
static D32S __adm_ecs_fillgeneralConfig
(
    ADM_ECS_GENERALCONFIG_T *pstDstConfig,
    ADM_ECS_GENERALCONFIG_T *pstSrcConfig

)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    D32S i = 0;

    if (ADM_IS_NULL(pstDstConfig) || ADM_IS_NULL(pstSrcConfig))
    {
        log_e("Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }

    pstDstConfig->protocolVersion       = adm_strdup(pstSrcConfig->protocolVersion);

    pstDstConfig->pollingCycle   = pstSrcConfig->pollingCycle;
    pstDstConfig->logReportUrl = NULL;
    if (NULL != pstSrcConfig->logReportUrl)
    {
        pstDstConfig->logReportUrl = adm_strdup(pstSrcConfig->logReportUrl);
    }
    pstDstConfig->logPrintLevel         = pstSrcConfig->logPrintLevel;
    pstDstConfig->logReportCycle        = pstSrcConfig->logReportCycle;
    pstDstConfig->isLogFileEncrypted    = pstSrcConfig->isLogFileEncrypted;
    pstDstConfig->isReportUpSuccLogFile = pstSrcConfig->isReportUpSuccLogFile;
	pstDstConfig->eventReportLevel      = pstSrcConfig->eventReportLevel;
    pstDstConfig->eventReportCycle      = pstSrcConfig->eventReportCycle;
    pstDstConfig->stateReportCycle      = pstSrcConfig->stateReportCycle;
    pstDstConfig->summaryReportCycle    = pstSrcConfig->summaryReportCycle;
    pstDstConfig->progressReportCycle   = pstSrcConfig->progressReportCycle;
    pstDstConfig->cyclePeriod           = pstSrcConfig->cyclePeriod;
    pstDstConfig->maxFireTrigger        = pstSrcConfig->maxFireTrigger;

    //!< 申请相应内存空间
    do
    {
        pstDstConfig->stSaLibList.num = pstSrcConfig->stSaLibList.num;
        pstDstConfig->stSaLibList.pstSaLibInfo = adm_ecs_newSaLibList(pstDstConfig->stSaLibList.num);
        if ((0 < pstDstConfig->stSaLibList.num) && ADM_IS_NULL(pstDstConfig->stSaLibList.pstSaLibInfo))
        {
            iRet = ADM_ECODE_NO_MEMORY;
            log_e("Failed to new sa lib list, err[%d]", iRet);
            break;
        }

        pstDstConfig->stCrcLibList.num = pstSrcConfig->stCrcLibList.num;
        pstDstConfig->stCrcLibList.pstCrcLibInfo = adm_ecs_newCrcLibList(pstDstConfig->stCrcLibList.num);
        if ((0 < pstDstConfig->stCrcLibList.num) && ADM_IS_NULL(pstDstConfig->stCrcLibList.pstCrcLibInfo))
        {
            iRet = ADM_ECODE_NO_MEMORY;
            log_e("Failed to new crc lib list, err[%d]", iRet);
            break;
        }

        pstDstConfig->stflashScriptList.num = pstSrcConfig->stflashScriptList.num;
        pstDstConfig->stflashScriptList.pstFlashScriptInfo = 
                        adm_ecs_newFlashScriptList(pstDstConfig->stflashScriptList.num);
        if ((0 < pstDstConfig->stflashScriptList.num) && 
            ADM_IS_NULL(pstDstConfig->stflashScriptList.pstFlashScriptInfo))
        {
            iRet = ADM_ECODE_NO_MEMORY;
            log_e("Failed to new FlashScriptlist, err[%d]", iRet);
            break;
        }      

        pstDstConfig->stBusinessScriptList.num = pstSrcConfig->stBusinessScriptList.num;
        pstDstConfig->stBusinessScriptList.pstBusinessScriptInfo = 
                        adm_ecs_newBusinessScriptList(pstDstConfig->stBusinessScriptList.num);
        if ((0 < pstDstConfig->stBusinessScriptList.num) && 
            ADM_IS_NULL(pstDstConfig->stBusinessScriptList.pstBusinessScriptInfo))
        {
            iRet = ADM_ECODE_NO_MEMORY;
            log_e("Failed to new businessScriptlist, err[%d]", iRet);
            break;
        }

    } while (FALSE);
    
    if (ADM_IS_ERR(iRet))
    {
        adm_ecs_cleanGeneralConfig(pstDstConfig);
        return iRet;
    }

    do
    {
        //!< 赋值
        iRet = adm_com_copyFileObject(&pstDstConfig->stBusinessScript, 
                    &pstSrcConfig->stBusinessScript);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to copy BusinessScript info, err[%d]", iRet);
            break;
        }
        
        for (i = 0; i < pstSrcConfig->stSaLibList.num; i++)
        { 
            iRet = adm_com_copyFileObject(&pstDstConfig->stSaLibList.pstSaLibInfo[i].stSaLibFile, 
                        &pstSrcConfig->stSaLibList.pstSaLibInfo[i].stSaLibFile);
            if (ADM_IS_ERR(iRet))
            {
                log_e("Failed to copy saLibList info, err[%d]", iRet);
                break;
            }
        
            pstDstConfig->stSaLibList.pstSaLibInfo[i].saLibNumber = 
                    pstSrcConfig->stSaLibList.pstSaLibInfo[i].saLibNumber;
        }

        for (i = 0; i < pstSrcConfig->stCrcLibList.num; i++)
        { 
            iRet = adm_com_copyFileObject(&pstDstConfig->stCrcLibList.pstCrcLibInfo[i].stCrcLibFile, 
                        &pstSrcConfig->stCrcLibList.pstCrcLibInfo[i].stCrcLibFile);
            if (ADM_IS_ERR(iRet))
            {
                log_e("Failed to copy crcLibList info, err[%d]", iRet);
                break;
            }
        
            pstDstConfig->stCrcLibList.pstCrcLibInfo[i].crcLibNumber = 
                    pstSrcConfig->stCrcLibList.pstCrcLibInfo[i].crcLibNumber;
        }

        for (i = 0; i < pstSrcConfig->stflashScriptList.num; i++)
        {
            iRet = adm_com_copyFileObject(
                        &pstDstConfig->stflashScriptList.pstFlashScriptInfo[i].stFlashScriptFile, 
                        &pstSrcConfig->stflashScriptList.pstFlashScriptInfo[i].stFlashScriptFile);
            if (ADM_IS_ERR(iRet))
            {
                log_e("Failed to copy flashScriptList info, err[%d]", iRet);
                break;
            }
        
            pstDstConfig->stflashScriptList.pstFlashScriptInfo[i].flashScriptNumber = 
                    pstSrcConfig->stflashScriptList.pstFlashScriptInfo[i].flashScriptNumber;
        }

        for (i = 0; i < pstSrcConfig->stBusinessScriptList.num; i++)
        {
            iRet = adm_com_copyFileObject(
                        &pstDstConfig->stBusinessScriptList.pstBusinessScriptInfo[i].stBusinessScriptFile, 
                        &pstSrcConfig->stBusinessScriptList.pstBusinessScriptInfo[i].stBusinessScriptFile);
            if (ADM_IS_ERR(iRet))
            {
                log_e("Failed to copy businessScriptList info, err[%d]", iRet);
                break;
            }
        
            pstDstConfig->stBusinessScriptList.pstBusinessScriptInfo[i].businessScriptNumber = 
                    pstSrcConfig->stBusinessScriptList.pstBusinessScriptInfo[i].businessScriptNumber;
        }

        iRet = __adm_ecs_fillEcuDidList(&pstDstConfig->stEcuDidList, &pstSrcConfig->stEcuDidList);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to fill resource did list, err[%d]", iRet);
            break;
        }

        //!< copy exts
        iRet = adm_com_copyExtsObject(&pstDstConfig->stExtList, &pstSrcConfig->stExtList);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to copy exts info, err[%d]", iRet);
            break;
        }
    } while (FALSE);

    if (ADM_IS_ERR(iRet))
    {
        adm_ecs_cleanGeneralConfig(pstDstConfig);
        return iRet;
    }

    return ADM_ECODE_SUCCESSED;
}

/*!
 *******************************************************************************
 * 填充配置信息.
 *
 * \note
 * 
 * \param[out]  pstDstConfig 目的模块配置结构
 * \param[in]   pstSrcConfig 源模块配置结构
 * \retval      D32S
 *******************************************************************************
 */
static D32S __adm_ecs_fillRsConfigInfo
(
    ADM_ECS_RSCONFIGINFO_T *pstDstConfig,
    ADM_ECS_RSCONFIGINFO_T *pstSrcConfig

)
{
    D32S iRet = ADM_ECODE_SUCCESSED;

    if (ADM_IS_NULL(pstDstConfig) || ADM_IS_NULL(pstSrcConfig))
    {
        log_e("Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }

    iRet = __adm_ecs_fillPartConfig(&pstDstConfig->stPartCfg, &pstSrcConfig->stPartCfg);
    if (ADM_IS_ERR(iRet))
    {
        log_e("Failed to fill part config info, err[%d]", iRet);
        return iRet;
    }

    iRet = __adm_ecs_fillgeneralConfig(&pstDstConfig->stGeneralCfg, &pstSrcConfig->stGeneralCfg);
    if (ADM_IS_ERR(iRet))
    {
        adm_ecs_cleanPartConfig(&pstDstConfig->stPartCfg);
        log_e("Failed to fill general config info, err[%d]", iRet);
        return iRet;
    }

    return ADM_ECODE_SUCCESSED;
}


/*!
 *******************************************************************************
 * 填充ota配置数据信息.
 *
 * \note
 * 
 * \param[out]  pstDstConfig 目的模块配置结构
 * \param[in]   pstSrcConfig 源模块配置结构
 * \retval      D32S
 *******************************************************************************
 */
static D32S __adm_ecs_fillRsConfigDataInfo
(
    ADM_ECS_RSCONFIGDATA_T *pstDstConfig,
    ADM_ECS_RSCONFIGDATA_T *pstSrcConfig
)
{
    D32S iRet = ADM_ECODE_SUCCESSED;

    if (ADM_IS_NULL(pstDstConfig) || ADM_IS_NULL(pstSrcConfig))
    {
        log_e("Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }

    do 
    {
        //!< 拷贝配置版本信息 
        if (NULL != pstSrcConfig->confVersion)
        {
            pstDstConfig->confVersion = adm_strdup(pstSrcConfig->confVersion);
        }

        //!< 拷贝文件对象信息 
        iRet = adm_com_copyFileObject(&pstDstConfig->stRsCfgFileObj, &pstSrcConfig->stRsCfgFileObj);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to fill part config info, err[%d]", iRet);
            break;
        }

        iRet = __adm_ecs_fillRsConfigInfo(&pstDstConfig->stRsCfgInfo, &pstSrcConfig->stRsCfgInfo);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to fill part config info, err[%d]", iRet);
            break;
        }
    } while (FALSE);

    if (ADM_IS_ERR(iRet))
    {
        adm_ecs_cleanRsCfgDataInfo(pstDstConfig);
    }

    return iRet;
}


#if 0
#endif


/*!
 *******************************************************************************
 * ecs基础配置信息参数初始化.
 *
 * \note
 * 
 * \param[out]  pstReqParam 模块基础配置参数
 * \retval      D32S
 *******************************************************************************
 */
static D32S __adm_ecs_submitPost
(
    DCHAR  *reqMessageBody, 
    DCHAR **rspMessageBody
)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    DSIZE statusCode = 0;
    DCHAR szEventBuf[ADM_COM_LEN_256] = {0};  //mod by wl 20220318

    if (ADM_IS_NULL(reqMessageBody) ||ADM_IS_NULL(rspMessageBody))
    {
        log_e("Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }

    //mod by wl 20220318
    adm_ecs_eventReport(ADM_COM_MOD_ECS, FALSE, ADM_EVENT_ID_GI_022, ADM_OCS_EVENT_LEVEL_DEBUG, "vehicle submit config request info.", NULL);

    if (ADM_IS_NULL(gstEcsModuleInfo.fnReqCloudApi))
    {
        log_e("ECS module not register request cloud api!, err[%d]", ADM_ECODE_MODULE_UNINIT);

        //mod by wl 20220318
        (void)adm_memzero(szEventBuf, sizeof(szEventBuf));
        (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), 
                "ECS module not register request cloud api!, err[%d]", ADM_ECODE_MODULE_UNINIT);
        adm_ecs_eventReport(ADM_COM_MOD_ECS, TRUE, ADM_EVENT_ID_GI_023, ADM_OCS_EVENT_LEVEL_ERROR, 
                "vehicle submit config request info failed!", szEventBuf);

        return ADM_ECODE_MODULE_UNINIT;
    }

    do 
    {
        // TODO: statusCode 应该也是需要判断的
        iRet = gstEcsModuleInfo.fnReqCloudApi(ADM_OCS_BUSINESS_TYPE_GET_CFG, reqMessageBody, rspMessageBody, &statusCode);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to submit http request, err[%d]", iRet);

            //mod by wl 20220318
            (void)adm_memzero(szEventBuf, sizeof(szEventBuf));
            (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), "Failed to submit http request, err[%d]", iRet);
            adm_ecs_eventReport(ADM_COM_MOD_ECS, TRUE, ADM_EVENT_ID_GI_023, ADM_OCS_EVENT_LEVEL_ERROR, 
                    "vehicle submit config request info failed!", szEventBuf);
            return iRet;
        }
    } while(FALSE);

    //mod by wl 20220318
    adm_ecs_eventReport(ADM_COM_MOD_ECS, FALSE, ADM_EVENT_ID_GI_024, ADM_OCS_EVENT_LEVEL_DEBUG, "vehicle submit config request info successed.", NULL);
    return iRet;
}

/*!
 *******************************************************************************
 * ecs下载配置文件.
 *
 * \note
 * 
 * \param[in]  pstRsCfgFileObj 配置文件对象
 * \retval     D32S
 *******************************************************************************
 */
static D32S __adm_ecs_downloadFile(ADM_COM_FILEOBJECT_T *pstRsCfgFileObj)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    DCHAR szEventBuf[ADM_COM_LEN_256] = {0};  //mod by wl 20220318
    ADM_DPO_DOWNLOAD_REQPARAM_T stDownReqInfo = {0};

    if (ADM_IS_NULL(pstRsCfgFileObj))
    {
        log_e("Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }

    //mod by wl 20220318
    (void)adm_memzero(szEventBuf, sizeof(szEventBuf));
    (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), 
            "download info: url[%s], filePath[%s], size[%lld]", pstRsCfgFileObj->url, pstRsCfgFileObj->filePath, pstRsCfgFileObj->size);
    adm_ecs_eventReport(ADM_COM_MOD_ECS, FALSE, ADM_EVENT_ID_GI_043, ADM_OCS_EVENT_LEVEL_DEBUG, "vehicle download config file.", szEventBuf);

    (void)adm_memzero(&stDownReqInfo, sizeof(stDownReqInfo));
    __adm_ecs_fillDownReqParamInfo(pstRsCfgFileObj, &stDownReqInfo);
    log_d("download url[%s]", stDownReqInfo.url);
    log_d("download filepath[%s]", stDownReqInfo.filePath);
    
    iRet = adm_dpo_doDownloadFile(&stDownReqInfo);
    if(ADM_IS_ERR(iRet))
    {
        log_e("Failed to download config file, url[%s]", stDownReqInfo.url);
        //mod by wl 20220318
        (void)adm_memzero(szEventBuf, sizeof(szEventBuf));
        (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), "Failed to download config file, url[%s], err[%d]", stDownReqInfo.url, iRet);
        adm_ecs_eventReport(ADM_COM_MOD_ECS, TRUE, ADM_EVENT_ID_GI_044, ADM_OCS_EVENT_LEVEL_ERROR, "vehicle download get config file failed!", szEventBuf);

        adm_dpo_cleanDownReqParamInfo(&stDownReqInfo);
        return iRet;
    }
    adm_dpo_cleanDownReqParamInfo(&stDownReqInfo);
    

    //解密验签处理 
    iRet = adm_ecs_decryptAndVerificationSignature(pstRsCfgFileObj);
    if(ADM_IS_ERR(iRet))
    {
        log_e("Verify signature failed, err[%d]", iRet);
        (void)adm_memzero(szEventBuf, sizeof(szEventBuf));
        (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), "Verify signature failed, err[%d]", iRet);
        adm_ecs_eventReport(ADM_COM_MOD_ECS, TRUE, ADM_EVENT_ID_GI_044, ADM_OCS_EVENT_LEVEL_ERROR, "vehicle download get config file failed!", szEventBuf);
        return iRet;
    }

    //mod by wl 20220318
    (void)adm_memzero(szEventBuf, sizeof(szEventBuf));
    (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), "download config filePath[%s].", pstRsCfgFileObj->filePath);
    adm_ecs_eventReport(ADM_COM_MOD_ECS, FALSE, ADM_EVENT_ID_GI_045, ADM_OCS_EVENT_LEVEL_DEBUG, "vehicle download config file successed.", szEventBuf);

    return iRet;
}


#if 0
#endif

/**
 * @brief   事件上报信息记录文件 for test
 * @detail  事件上报记录文件 for test
 *
 * @note    
 *
 * @param [in]  pcEventId      事件ID
 * @param [in]  i32EventLevel  事件级别：1:表示INFO级别 2:表示WARN级别 3:表示ERROR级别
 * @param [in]  pcEventDesc    事件描述
 * @param [in]  pcEventContext 事件上下文
 * @return      none 
 */
void adm_ecs_AddEventsinfotofile_test
(
    DCHAR *pcEventId,
    D32S   i32EventLevel,    
    DCHAR *pcEventDesc,
    DCHAR *pcEventContext
)
{
    char szEventBuf[4096] = {0};
    FILE *fp;

    if (ADM_IS_NULL(pcEventId) || ADM_IS_NULL(pcEventDesc))
    {
        log_e("Invalid input param");
        return;
    }

    (void)adm_memzero(szEventBuf, sizeof(szEventBuf));
    (void)adm_snprintf(szEventBuf, sizeof(szEventBuf),
        "EventId:[%s], EventLevel:[%d], EventDesc:[%s], EventContext:[%s]\n",
        pcEventId, i32EventLevel, pcEventDesc, (NULL != pcEventContext) ? (pcEventContext) : ("None"));

    fp=fopen("/tmp/EventRepot.txt","a+");
    if(fp==NULL)
    {
        (void)printf("can't open the EventRepot.txt!\n");
        return;
    }

    (void)fwrite(szEventBuf, strlen(szEventBuf) , 1, fp);
    (void)fflush(fp);
    (void)fclose(fp);

    return;
}

/**
 * @brief   ecs 模块封装的事件上报接口 
 * @detail  事件上报
 *
 * @note 
 *
 * @param [in]  eModuleID 模块ID
 * @param [in]  isReportImmediate 是否需要立即上报
 * @param [in]  pcEventId      事件id
 * @param [in]  i32EventLevel  事件级别：1:表示INFO级别 2:表示WARN级别 3:表示ERROR级别
 * @param [in]  pcEventDesc    事件描述
 * @param [in]  pcEventContext 事件上下文(非必填字段)
 * @return      none 
 */
void adm_ecs_eventReport
(
    ADM_COM_MOD_E eModuleID,
    DBOOL  isReportImmediate,
    const DCHAR *pcEventId,
    D32S   i32EventLevel,    
    const DCHAR *pcEventDesc,
    const DCHAR *pcEventContext
)
{
    if (ADM_IS_NULL(pcEventId) || ADM_IS_NULL(pcEventDesc))
    {
        log_e("Invalid input param");
        return;
    }
    
    //!< 模块未初始化，则直接报错
    if (FALSE == gstEcsModuleInfo.isInited)
    {
        log_e("ECS module not init, err[%d]", ADM_ECODE_MODULE_UNINIT);
        return;
    }

    if (NULL != gstEcsModuleInfo.fnReportEventApi)
    {
        gstEcsModuleInfo.fnReportEventApi(eModuleID, isReportImmediate, pcEventId, i32EventLevel, pcEventDesc, pcEventContext);
    }

    return;
}

/*!
 *******************************************************************************
 * ecs模块解密验签
 *
 * \note 
 *
 * \param[in]   pstFileObject      文件对象信息
 * \retval      D32S
 *******************************************************************************
 */

D32S adm_ecs_decryptAndVerificationSignature(ADM_COM_FILEOBJECT_T *pstFileObject)
{
    D32S iRet = ADM_ECODE_SUCCESSED;

    if (ADM_IS_NULL(pstFileObject))
    {
        log_e("Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }
    
    //!< 模块未初始化，则直接报错
    if (FALSE == gstEcsModuleInfo.isInited)
    {
        iRet = ADM_ECODE_MODULE_UNINIT;
        log_e("ECS module not init, err[%d]", ADM_ECODE_MODULE_UNINIT);
        return iRet;
    }

    if (NULL != gstEcsModuleInfo.fnDecryptAndSignatureApi)
    {
        iRet = gstEcsModuleInfo.fnDecryptAndSignatureApi(pstFileObject);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Verify signature failed, err[%d]", iRet);
        }
    }

    return iRet;
}

D32S adm_ecs_fileSyncTransfer(DCHAR *fileName, const DCHAR *url, const D64S size, const DCHAR *hash)
{
    D32S iRet = ADM_ECODE_SUCCESSED;

    if (ADM_IS_NULL(fileName) || ADM_IS_NULL(url) || ADM_IS_NULL(hash) || (0 == size))
    {
        log_e("Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }

    //!< 模块未初始化，则直接报错
    if (FALSE == gstEcsModuleInfo.isInited)
    {
        iRet = ADM_ECODE_MODULE_UNINIT;
        log_e("ECS module not init, err[%d]", ADM_ECODE_MODULE_UNINIT);
        return iRet;
    }

    if (NULL != gstEcsModuleInfo.fnFileSyncTransferApi)
    {
        iRet = gstEcsModuleInfo.fnFileSyncTransferApi(fileName, url, size, hash);
        if (ADM_IS_ERR(iRet))
        {
            log_e("file transfer failed, err[%d]", iRet);
        }
    }

    return iRet;
}

#if 0
#endif

/*!
 *******************************************************************************
 * ecs基础配置信息初始化.
 *
 * \note
 * 
 * \param[out]  pstBaseCfgInfo 模块基础配置信息
 * \retval      D32S
 *******************************************************************************
 */
static D32S __adm_ecs_configInit(ADM_ECS_BASECONFIGINFO_T *pstBaseCfgInfo)
{
    if (ADM_IS_NULL(pstBaseCfgInfo))
    {
        log_e("Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }

    if (ADM_IS_NULL(gstEcsModuleInfo.fnGetReqCloudParamApi))
    {
        log_e("ECS module not reg getReqCloudParam, err[%d]", ADM_ECODE_MODULE_UNINIT);
        return ADM_ECODE_MODULE_UNINIT;
    }
    (void)gstEcsModuleInfo.fnGetReqCloudParamApi(&pstBaseCfgInfo->stReqParam);

    //!< 注册业务请求函数
    pstBaseCfgInfo->ifOpt.createMessageBody_f  = adm_ecs_createMessageBody;
    pstBaseCfgInfo->ifOpt.submitPost_f         = __adm_ecs_submitPost;
    pstBaseCfgInfo->ifOpt.parseMessageBody_f   = adm_ecs_parseMessageBody;
    pstBaseCfgInfo->ifOpt.downloadFile_f       = __adm_ecs_downloadFile;
    pstBaseCfgInfo->ifOpt.parseConfigureFile_f = adm_ecs_parseConfigFile;
    pstBaseCfgInfo->ifOpt.parseConfigurePackage_f = adm_ecs_parseConfigPackage;

    //__adm_ecs_baseCfgInterfaceInit(&pstBaseCfgInfo->ifOpt);

    return ADM_ECODE_SUCCESSED;
}


/*!
 *******************************************************************************
 * 从平台获取配置信息.
 *
 * \note 从平台获取配置信息，不会存储到kv
 * 
 * \param[out]  pstRsCfgInfo    OTA平台配置信息
 * \retval      D32S
 *******************************************************************************
 */
static D32S __adm_ecs_getConfigFromServer(ADM_ECS_RSCONFIGDATA_T **ppstOtaConfigData)
{
    D32S iRet  = ADM_ECODE_SUCCESSED;
    DCHAR *reqMsgBody = NULL;
    DCHAR *rspMsgBody = NULL;
    DCHAR *pcDownConfFileDir = NULL;
    DCHAR aDecryptPackDir[ADM_COM_LEN_256] = {0};
    DCHAR aDecryptPackPath[ADM_COM_LEN_256] = {0};
    DCHAR szEventBuf[ADM_COM_LEN_256] = {0};  //mod by wl 20220318
    ADM_ECS_RSCONFIGDATA_T   *pstOtaCfgData = NULL;
    ADM_ECS_BASECONFIGINFO_T *pstBaseCfgInfo = NULL;

    //mod by wl 20220318
    adm_ecs_eventReport(ADM_COM_MOD_ECS, FALSE, ADM_EVENT_ID_GI_004, ADM_OCS_EVENT_LEVEL_DEBUG, "get vehicle config info from Server", ADM_ECS_MARKER_EV_SUB_DOING_CLOUD_BEGIN);

    if (ADM_IS_NULL(ppstOtaConfigData))
    {
        log_e("Invalid input param");
        //mod by wl 20220318
        (void)adm_memzero(szEventBuf, sizeof(szEventBuf));
        (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), "Invalid input param, err[%d]", ADM_ECODE_INVALID_PARAM);
        adm_ecs_eventReport(ADM_COM_MOD_ECS, TRUE, ADM_EVENT_ID_GI_005, ADM_OCS_EVENT_LEVEL_ERROR, 
                    "get vehicle config info from Server failed!", szEventBuf);
        return ADM_ECODE_INVALID_PARAM;
    }

    do
    {   
        //mod by wl 20220318
        (void)adm_memzero(szEventBuf, sizeof(szEventBuf));
        pstOtaCfgData = adm_ecs_newRsCfgDataInfo();
        if (ADM_IS_NULL(pstOtaCfgData))
        {
            iRet = ADM_ECODE_NO_MEMORY;
            log_e("Failed to new ota config data info, err[%d]", iRet);            
            //mod by wl 20220318
            (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), "Failed to new ota config data info, err[%d]", iRet);
            break;
        }

        pstBaseCfgInfo = __adm_ecs_newBaseCfgInfo();
        if (ADM_IS_NULL(pstBaseCfgInfo))
        {
            iRet = ADM_ECODE_NO_MEMORY;
            log_e("Failed to new base config info, err[%d]", ADM_ECODE_NO_MEMORY);
            //mod by wl 20220318
            (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), "Failed to new base config info, err[%d]", iRet);
            break;
        }
    
        iRet = __adm_ecs_configInit(pstBaseCfgInfo);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to init base config info, err[%d]", iRet);
            //mod by wl 20220318
            (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), "Failed to new base config info, err[%d]", iRet);
            break;
        }

        //!< 创建消息
        iRet = pstBaseCfgInfo->ifOpt.createMessageBody_f(&pstBaseCfgInfo->stReqParam, &reqMsgBody);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to create get config meassage body, err[%d]", iRet);
            //mod by wl 20220318
            (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), "Failed to create get config meassage body, err[%d]", iRet);
            break;
        }
        log_i("reqMsgBody: %s", reqMsgBody);

        //!< 提交消息
        iRet = pstBaseCfgInfo->ifOpt.submitPost_f(reqMsgBody, &rspMsgBody);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to submit get config request, err[%d]", iRet);
            //mod by wl 20220318
            (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), "Failed to submit get config request, err[%d]", iRet);
            break;
        }
        log_i("rspMsgBody: %s", rspMsgBody);

        //!< 解析消息体
        iRet = pstBaseCfgInfo->ifOpt.parseMessageBody_f(rspMsgBody, &pstBaseCfgInfo->stReqParam, pstOtaCfgData);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to parse config meassage body, err[%d]", iRet);
            //mod by wl 20220318
            (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), "Failed to parse config meassage body, err[%d]", iRet);
            break;
        }
        log_i("cfg file url[%s], filePath[%s]", pstOtaCfgData->stRsCfgFileObj.url, pstOtaCfgData->stRsCfgFileObj.filePath);

        //!< 下载配置文件 & 车云解密验签拆分签名文件
        iRet = pstBaseCfgInfo->ifOpt.downloadFile_f(&pstOtaCfgData->stRsCfgFileObj);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to download config file, err[%d]", iRet);
            //mod by wl 20220318
            (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), "Failed to download config file, err[%d]", iRet);
            break;
        }
        log_i("Download config file ok.");

        ///< 车-工具链解密
        adm_memset(aDecryptPackDir, '\0', sizeof(aDecryptPackDir), sizeof(aDecryptPackDir));
        adm_memset(aDecryptPackPath, '\0', sizeof(aDecryptPackPath), sizeof(aDecryptPackPath));
        adm_snprintf(aDecryptPackDir, sizeof(aDecryptPackDir), "%s%s", pstBaseCfgInfo->stReqParam.confFileDir,"tmpDec");
        adm_snprintf(aDecryptPackPath, sizeof(aDecryptPackPath), "%s/%s", aDecryptPackDir, pstOtaCfgData->stRsCfgFileObj.fileName);
        log_i("config decrypt path[%s] full path[%s]", aDecryptPackDir, aDecryptPackPath);
        iRet = pstBaseCfgInfo->ifOpt.parseConfigurePackage_f(pstOtaCfgData->stRsCfgFileObj.filePath, aDecryptPackDir, pstOtaCfgData->stRsCfgFileObj.fileName);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to decrypt package, err[%d]", iRet);
            (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), "Failed to decrypt config package, err[%d]", iRet);
            break;
        }
        log_i("Toolchain decrypt package ok.");
        // 在工具链解密成功后删除解密前的原始文件
        if (pstOtaCfgData->stRsCfgFileObj.filePath != NULL)
        {
            log_i("Deleting original config file after toolchain decrypt: %s", pstOtaCfgData->stRsCfgFileObj.filePath);
            if (0 != adm_fremove(pstOtaCfgData->stRsCfgFileObj.filePath))
            {
                log_w("Failed to delete original config file after toolchain decrypt: %s, but continue", pstOtaCfgData->stRsCfgFileObj.filePath);
            }
            else
            {
                log_i("Successfully deleted original config file after toolchain decrypt: %s", pstOtaCfgData->stRsCfgFileObj.filePath);
            }
        }

        //!< 设置配置文件解压目录
        pcDownConfFileDir = adm_ecs_getDownConfigFileUnzipDir();
        if (ADM_IS_NULL(pcDownConfFileDir))
        {
            iRet = ADM_ECODE_CORESERVICE_GETRUNDIR_FAILED;
            log_e("Failed to get coreService config package save dir, err[%d]", iRet);
            //mod by wl 20220318
            (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), "Failed to get coreService config package save dir, err[%d]", iRet);
            break;
        }
        log_i("Config file unzip dir[%s].", pcDownConfFileDir);

        //!< 解析配置文件 
        iRet = pstBaseCfgInfo->ifOpt.parseConfigureFile_f(aDecryptPackPath, 
                        pcDownConfFileDir, &pstOtaCfgData->stRsCfgInfo);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to parse config file, err[%d]", iRet);
            //mod by wl 20220318
            (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), "Failed to parse config file, err[%d]", iRet);
            break;
        }
        log_i("Parse configure file ok.");
        // 在解析配置文件后删除解密后的临时文件
        if (aDecryptPackPath[0] != '\0')
        {
            log_i("Deleting decrypted temp file after parsing: %s", aDecryptPackPath);
            if (0 != adm_fremove(aDecryptPackPath))
            {
                log_w("Failed to delete decrypted temp file after parsing: %s, but continue", aDecryptPackPath);
            }
            else
            {
                log_i("Successfully deleted decrypted temp file after parsing: %s", aDecryptPackPath);
            }
        }

    } while (FALSE);

    adm_free(reqMsgBody);
    adm_free(rspMsgBody);
    adm_free(pcDownConfFileDir);
    __adm_ecs_delBaseCfgInfo(&pstBaseCfgInfo);

    if (ADM_IS_ERR(iRet))
    {
        adm_ecs_delRsCfgDataInfo(&pstOtaCfgData);
        log_e("Failed to get rs config info from rs cloud, err[%d]", iRet);

        adm_ecs_eventReport(ADM_COM_MOD_ECS, TRUE, ADM_EVENT_ID_GI_005, ADM_OCS_EVENT_LEVEL_ERROR, 
                "get vehicle config info from Server failed!", szEventBuf);
        return iRet;
    }
    *ppstOtaConfigData = pstOtaCfgData;

    //mod by wl 20220318
    adm_ecs_eventReport(ADM_COM_MOD_ECS, FALSE, ADM_EVENT_ID_GI_006, ADM_OCS_EVENT_LEVEL_DEBUG, 
                "get vehicle config info from Server successed.", ADM_ECS_MARKER_EV_SUB_DOING_CLOUD_PASS);

    return iRet;
}

/*!
 *******************************************************************************
 * 检查ECS模块是否初始化
 * 
 * \note   
 *
 * \param   none
 * \retval  D32S
 *******************************************************************************
 */
static D32S __adm_ecs_checkAndInitModule(void)
{
    //!< 模块未初始化，则直接报错
    if (FALSE == gstEcsModuleInfo.isInited)
    {
        log_e("ECS module not init, err[%d]", ADM_ECODE_MODULE_UNINIT);
        return ADM_ECODE_MODULE_UNINIT;
    }

    //!< 数据未初始化，则直接报错
    if (FALSE == gstEcsModuleInfo.isDataInited)
    {
        log_e("ECS module data not init, err[%d]", ADM_ECODE_ECS_CFGDATA_NOUPLOAD);
        return ADM_ECODE_ECS_CFGDATA_NOUPLOAD;
    }

    return ADM_ECODE_SUCCESSED;
}


/*!
 *******************************************************************************
 * 获取ota缓存的平台配置信息.
 *
 * \note 外部不需要释放此返回的缓存
 * 
 * \param   none
 * \retval  ADM_ECS_RSCONFIGDATA_T *
 *******************************************************************************
 */
static ADM_ECS_RSCONFIGDATA_T *__adm_ecs_getOtaConfigCacheInfo(void)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    ADM_ECS_RSCONFIGDATA_T *pstEcsConfigData = NULL;
    DCHAR szEventBuf[ADM_COM_LEN_256] = {0};  //mod by wl 20220318

    //mod by wl 20220317
    //(void)adm_memzero(szEventBuf, sizeof(szEventBuf));
    //adm_ecs_eventReport(ADM_COM_MOD_ECS, FALSE, ADM_EVENT_ID_GT_003, ADM_OCS_EVENT_LEVEL_DEBUG, "trigger get ota config data.", NULL);

    pstEcsConfigData = adm_ecs_getOtaConfigDataFromKV();
    if (NULL != pstEcsConfigData)
    {
        log_i("Get ota config data from kv storage ok.");
        //mod by wl 20220318
        //adm_ecs_eventReport(ADM_COM_MOD_ECS, TRUE, ADM_EVENT_ID_GE_005, ADM_OCS_EVENT_LEVEL_INFO, "trigger get ota config data successed!", "Get ota config data from kv storage ok.");
        return pstEcsConfigData;
    }
    log_i("Failed get ota config data from kv storage");

    iRet = __adm_ecs_getConfigFromServer(&pstEcsConfigData);
    if (ADM_IS_ERR(iRet))
    {
        log_e("Failed to get ota config data from server, err[%d]", iRet);
        //mod by wl 20220318
        (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), "Failed to get ota config data from server, err[%d]", iRet);
        adm_ecs_eventReport(ADM_COM_MOD_ECS, TRUE, ADM_EVENT_ID_GE_005, ADM_OCS_EVENT_LEVEL_INFO, "trigger get ota config data failed!", szEventBuf);
        return NULL;
    }

    //!< 此处不用返回错误，即使kv存储不可用也可保证功能是正常的
    iRet = adm_ecs_setOtaConfigData2KV(pstEcsConfigData);
    if (ADM_IS_ERR(iRet))
    {
        log_i("Failed to set ota config data to kvStorage, err[%d]", iRet);
    }

    //mod by wl 20220318
    adm_ecs_eventReport(ADM_COM_MOD_ECS, TRUE, ADM_EVENT_ID_GE_006, ADM_OCS_EVENT_LEVEL_INFO, "trigger get ota config data successed!", szEventBuf);

    return pstEcsConfigData;
}

/*!
 *******************************************************************************
 * 清空下载的配置文件及解压的配置文件
 * 
 * \note 
 *
 * \param   none
 * \retval  none
 *******************************************************************************
 */
static void __adm_ecs_cleanDownAndUnzipFile(void)
{
    DCHAR *pcDownFileDir = NULL;

    //!< 清除上次下载任务下载的升级包及解压包
    pcDownFileDir = adm_ecs_getDownConfigFileDir();
    if (NULL != pcDownFileDir)
    {
        (void)adm_fremove(pcDownFileDir);
        adm_free(pcDownFileDir);
    }

    pcDownFileDir = adm_ecs_getDownConfigFileUnzipDir();
    if (NULL != pcDownFileDir)
    {
        (void)adm_fremove(pcDownFileDir);
        adm_free(pcDownFileDir);
    }

    return;
}


#if 0
#endif

/*!
 *******************************************************************************
 * 获取ecs模块车云协议请求参数.
 *
 * \note
 * 
 * \param[out]  pstReqParam 模块请求云协议参数
 * \retval      D32S
 *******************************************************************************
 */
D32S adm_ecs_getReqCloudParam(ADM_ECS_REQPARAM_T *pstReqParam)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    ADM_VRS_REGISTER_INFO_T stRegisterInfo = {0};
    DCHAR szBuf[ADM_COM_LEN_256] = {0};  //mod by wl 20220318

    if (ADM_IS_NULL(pstReqParam))
    {
        log_e("Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }

    //mod by wl 20220318
    adm_ecs_eventReport(ADM_COM_MOD_ECS, FALSE, ADM_EVENT_ID_GP_001, ADM_OCS_EVENT_LEVEL_DEBUG, "get vehicle download config file dir.", NULL);

    //!< 获取下载升级包的存储路径
    pstReqParam->confFileDir = adm_ecs_getDownConfigFileDir();
    if (ADM_IS_NULL(pstReqParam->confFileDir))
    {        
        //mod by wl 20220318
        (void)adm_memzero(szBuf, sizeof(szBuf));
        (void)adm_snprintf(szBuf, sizeof(szBuf), "get vehicle download config file dir failed! err[%d].", ADM_ECODE_CORESERVICE_GETRUNDIR_FAILED);
        adm_ecs_eventReport(ADM_COM_MOD_ECS, TRUE, ADM_EVENT_ID_GP_002, ADM_OCS_EVENT_LEVEL_ERROR, szBuf, NULL);
    
        log_e("Failed to get coreService config package save dir, err[%d]", ADM_ECODE_CORESERVICE_GETRUNDIR_FAILED);
        return ADM_ECODE_CORESERVICE_GETRUNDIR_FAILED;    
    }
    //mod by wl 20220318
    (void)adm_memzero(szBuf, sizeof(szBuf));
    (void)adm_snprintf(szBuf, sizeof(szBuf), "vehicle config file dir[%s].", pstReqParam->confFileDir);
    adm_ecs_eventReport(ADM_COM_MOD_ECS, FALSE, ADM_EVENT_ID_GP_003, ADM_OCS_EVENT_LEVEL_DEBUG, "get vehicle download config file dir successed.", szBuf);

    //mod by wl 20220318
    adm_ecs_eventReport(ADM_COM_MOD_ECS, FALSE, ADM_EVENT_ID_GP_004, ADM_OCS_EVENT_LEVEL_DEBUG, "get vehicle register info.", NULL);
    (void)adm_memzero(&stRegisterInfo, sizeof(stRegisterInfo));
    iRet = adm_vrs_getRegisterInfo(&stRegisterInfo);
    if (ADM_IS_ERR(iRet))
    {
        adm_free(pstReqParam->confFileDir);

        //mod by wl 20220318
        (void)adm_memzero(szBuf, sizeof(szBuf));
        (void)adm_snprintf(szBuf, sizeof(szBuf), "get vehicle register info failed, err[%d]", iRet);
        adm_ecs_eventReport(ADM_COM_MOD_ECS, TRUE, ADM_EVENT_ID_GP_005, ADM_OCS_EVENT_LEVEL_ERROR, szBuf, NULL);

        log_e("Failed to get RegisterInfo, err[%d]", iRet);
        return iRet;
    }
    //mod by wl 20220318
    adm_ecs_eventReport(ADM_COM_MOD_ECS, FALSE, ADM_EVENT_ID_GP_006, ADM_OCS_EVENT_LEVEL_DEBUG, "get vehicle register info successed", NULL);

    pstReqParam->secret = adm_strdup(stRegisterInfo.secret);
    pstReqParam->deviceId = adm_strdup(stRegisterInfo.deviceId);
    adm_free(stRegisterInfo.secret);
    adm_free(stRegisterInfo.vinCode);
    adm_free(stRegisterInfo.deviceId);


#ifdef ADM_SDK_VERSION
    pstReqParam->sdkVersion = adm_strdup(ADM_SDK_VERSION);
#else
    pstReqParam->sdkVersion = adm_strdup("V1.0.1");
#endif
    pstReqParam->timestamp   = time(NULL);

    (void)adm_memzero(szBuf, sizeof(szBuf));
    (void)adm_snprintf(szBuf, sizeof(szBuf), "vehicle UCM SDK version[%s].", pstReqParam->sdkVersion);
    adm_ecs_eventReport(ADM_COM_MOD_ECS, FALSE, ADM_EVENT_ID_GP_007, ADM_OCS_EVENT_LEVEL_DEBUG, "get vehicle UCM SDK version.", szBuf);
    
    log_d("ECS, secret:%s",      pstReqParam->secret);
    log_d("ECS, deviceId:%s",    pstReqParam->deviceId);
    log_d("ECS, sdkVersion:%s",  pstReqParam->sdkVersion);
    log_d("ECS, confFileDir:%s", pstReqParam->confFileDir);

    return iRet;
}

/*!
 *******************************************************************************
 * 更新ota缓存的平台配置信息.
 *
 * \note 函数内部会从平台重新获取一次配置信息，并将获取到的配置信息存储到kv
 *       
 * \param[inout]  pstRsCfgInfo OTA平台配置信息 --- 可选字段
 *                             --- 不为空则拷贝新的配置信息出去
 *                             --- 为空则不拷贝
 * \retval  D32S
 *******************************************************************************
 */
D32S adm_ecs_updateAndGetConfigInfo(ADM_ECS_RSCONFIGDATA_T **ppstOtaConfigData)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    DCHAR szEventBuf[ADM_COM_LEN_256] = {0};  //mod by wl 20220318
    ADM_ECS_RSCONFIGDATA_T *pstRsCfgData = NULL;

    log_i("[ABUP-Procedure] update vehicle config Start");

    //mod by wl 20220317
    (void)adm_memzero(szEventBuf, sizeof(szEventBuf));
    adm_ecs_eventReport(ADM_COM_MOD_ECS, FALSE, ADM_EVENT_ID_GT_002, ADM_OCS_EVENT_LEVEL_DEBUG, "trigger update vehicle config info.", ADM_ECS_MARKER_EV_SUB_DOING_UPDATE_BEGIN);

    //!< 模块未初始化，则直接报错
    if (FALSE == gstEcsModuleInfo.isInited)
    {
        log_e("ECS module not init, err[%d]", ADM_ECODE_MODULE_UNINIT);
        //mod by wl 20220318
        (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), "ECS module not init, err[%d]", ADM_ECODE_MODULE_UNINIT);
        adm_ecs_eventReport(ADM_COM_MOD_ECS, TRUE, ADM_EVENT_ID_GE_002, ADM_OCS_EVENT_LEVEL_INFO, "update vehicle config info failed!", szEventBuf);
        
        log_i("[ABUP-Procedure] update vehicle config End. [iRet = %d]", ADM_ECODE_MODULE_UNINIT);
        return ADM_ECODE_MODULE_UNINIT;
    }

    //!< 清除老配置文件及解压的文件
    __adm_ecs_cleanDownAndUnzipFile();

    //!< 从平台获取配置
    iRet = __adm_ecs_getConfigFromServer(&pstRsCfgData);
    if (ADM_IS_ERR(iRet))
    {
        log_e("Failed to get ota config data from server, err[%d]", iRet);
        //mod by wl 20220318
        (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), "Failed to get ota config data from server, err[%d], %s", iRet, ADM_ECS_MARKER_EV_SUB_DOING_CLOUD_FAIL);
        adm_ecs_eventReport(ADM_COM_MOD_ECS, TRUE, ADM_EVENT_ID_GE_002, ADM_OCS_EVENT_LEVEL_INFO, "update vehicle config info failed!", szEventBuf);
        
        log_i("[ABUP-Procedure] update vehicle config End. [iRet = %d]", iRet);
        return iRet;
    }

    //!< 更新缓存
    (void)pthread_rwlock_wrlock(&gstEcsModuleInfo.stRWlock);
    adm_ecs_delRsCfgDataInfo(&gstEcsModuleInfo.pstEcsConfig);
    gstEcsModuleInfo.pstEcsConfig = pstRsCfgData;
    gstEcsModuleInfo.isDataInited = TRUE;
    (void)pthread_rwlock_unlock(&gstEcsModuleInfo.stRWlock);

    //!< 存储失败不用报错，大不了下次继续从平台获取
    iRet = adm_ecs_setOtaConfigData2KV(gstEcsModuleInfo.pstEcsConfig);
    if (ADM_IS_ERR(iRet))
    {
        log_i("Failed to set ota config data to kvStorage, err[%d]", iRet);
    }

    //!< 不为空则拷贝到出参
    if (NULL != ppstOtaConfigData)
    {        
        *ppstOtaConfigData = adm_ecs_newRsCfgDataInfo();
        if (ADM_IS_NULL(*ppstOtaConfigData))
        {
            log_e("Failed to new ota config data info, err[%d]", ADM_ECODE_NO_MEMORY);

            log_i("[ABUP-Procedure] update vehicle config End. [iRet = %d]", ADM_ECODE_NO_MEMORY);
            return ADM_ECODE_NO_MEMORY;
        }
        
        iRet = __adm_ecs_fillRsConfigDataInfo(*ppstOtaConfigData, gstEcsModuleInfo.pstEcsConfig);
        if (ADM_IS_ERR(iRet))
        {
            adm_free(*ppstOtaConfigData);
            log_e("Failed to fill ota config info, err[%d]", iRet);
        }
    }

    //mod by wl 20220318
    adm_ecs_eventReport(ADM_COM_MOD_ECS, TRUE, ADM_EVENT_ID_GE_004, ADM_OCS_EVENT_LEVEL_INFO, "update vehicle config info successed.", ADM_ECS_MARKER_EV_SUB_END_PASS);

    log_i("[ABUP-Procedure] update vehicle config End. [iRet = %d]", ADM_ECODE_SUCCESSED);

    return ADM_ECODE_SUCCESSED;
}

/*!
 *******************************************************************************
 * 获取配置信息.
 *
 * \note    从内存中获取配置信息，若内存没有则从平台获取 
 * 
 * \param[out]  pstRsCfgInfo    OTA平台配置信息
 * \retval      D32S
 *******************************************************************************
 */
D32S adm_ecs_getConfigInfo(ADM_ECS_RSCONFIGINFO_T *pstRsCfgInfo)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
	DCHAR szEventBuf[ADM_COM_LEN_256] = {0}; //mod by wl 20220318
    ADM_ECS_RSCONFIGDATA_T *pstOtaConfig = NULL;

    //mod by wl 20220318
    //adm_ecs_eventReport(ADM_COM_MOD_ECS, FALSE, ADM_EVENT_ID_GT_001, ADM_OCS_EVENT_LEVEL_DEBUG, "trigger get vehicle config info", ADM_ECS_MARKER_EV_SUB_BEGIN);

    if (ADM_IS_NULL(pstRsCfgInfo))
    {
        log_e("Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }

    //!< 模块未初始化，则直接报错
    if (FALSE == gstEcsModuleInfo.isInited)
    {
        //mod by wl 20220318
        //(void)adm_memzero(szEventBuf, sizeof(szEventBuf));
        //(void)adm_snprintf(szEventBuf, sizeof(szEventBuf), "get vehicle config info failed! err[%d].", ADM_ECODE_MODULE_UNINIT);
        //adm_ecs_eventReport(ADM_COM_MOD_ECS, TRUE, ADM_EVENT_ID_GE_001, ADM_OCS_EVENT_LEVEL_ERROR, szEventBuf, "ECS module not init");
        
        log_e("ECS module not init, err[%d]", ADM_ECODE_MODULE_UNINIT);
        return iRet;
    }

    //!< 数据未初始化，则需要初始化数据段
    if (FALSE == gstEcsModuleInfo.isDataInited)
    {
        pstOtaConfig = __adm_ecs_getOtaConfigCacheInfo();
        if (ADM_IS_NULL(pstOtaConfig))
        {
            //mod by wl 20220318
            //(void)adm_memzero(szEventBuf, sizeof(szEventBuf));
            //(void)adm_snprintf(szEventBuf, sizeof(szEventBuf), "get vehicle config info failed, err[%d].", ADM_ECODE_OTACFG_GET_CACHE_FAILED);
            //adm_ecs_eventReport(ADM_COM_MOD_ECS, TRUE, ADM_EVENT_ID_GE_001, ADM_OCS_EVENT_LEVEL_ERROR, szEventBuf, "get vehicle config info from KV failed!");

            log_e("Failed to get ota Config info, err[%d]", ADM_ECODE_OTACFG_GET_CACHE_FAILED);
            return ADM_ECODE_OTACFG_GET_CACHE_FAILED;
        }
        
        (void)pthread_rwlock_wrlock(&gstEcsModuleInfo.stRWlock);
        gstEcsModuleInfo.pstEcsConfig = pstOtaConfig;
        gstEcsModuleInfo.isDataInited = TRUE;
        (void)pthread_rwlock_unlock(&gstEcsModuleInfo.stRWlock);
    }

    (void)pthread_rwlock_rdlock(&gstEcsModuleInfo.stRWlock);
    iRet = __adm_ecs_fillRsConfigInfo(pstRsCfgInfo, &gstEcsModuleInfo.pstEcsConfig->stRsCfgInfo);
    if (ADM_IS_ERR(iRet))
    {
        adm_ecs_cleanRSConfigInfo(pstRsCfgInfo);
        log_e("Failed to fill ota config info, err[%d]", iRet);

        //mod by wl 20220318
        //(void)adm_memzero(szEventBuf, sizeof(szEventBuf));
        //(void)adm_snprintf(szEventBuf, sizeof(szEventBuf), "get vehicle config info failed, err[%d].", iRet);
        //adm_ecs_eventReport(ADM_COM_MOD_ECS, TRUE, ADM_EVENT_ID_GE_001, ADM_OCS_EVENT_LEVEL_ERROR, szEventBuf, "Failed to fill ota config info!");

    }

    (void)pthread_rwlock_unlock(&gstEcsModuleInfo.stRWlock);
    
    //mod by wl 20220318
    //adm_ecs_eventReport(ADM_COM_MOD_ECS, TRUE, ADM_EVENT_ID_GE_003, ADM_OCS_EVENT_LEVEL_INFO, "get vehicle config info successed.", NULL);

    return iRet;
}



#if 0
#endif

/*!
 *******************************************************************************
 * ecs模块初始化.
 *
 * \note 需要注册对应的回调函数
 *
 * \param[in] fnGetReqCloudParamCb      注册获取请求云端配置请求参数回调函数
 * \param[in] fnRegReqCloudCb           注册向云端获取配置请求回调函数
 * \param[in] fnRegDownloadCb           注册配置文件下载回调函数
 * \param[in] fnReportEventCb           注册事件上报回调函数
 * \param[in] fnDecryptAndSignatureCb   解密和验签回调函数
 * \param[in] fnFileSyncTransferCb      文件传输回调函数
 * \retval    D32S                 0: 成功 non-0：失败
 *******************************************************************************
 */
D32S adm_ecs_moduleInit
(
    ADM_ECS_GETREQCLOUDPARAM_CALLBACK_F        fnGetReqCloudParamCb,
    ADM_OCS_BUSINESS_COMM_INTERFACE_CALLBACK_F fnRegReqCloudCb,
    ADM_OCM_FILEDOWN_API_CALLBACK_F            fnRegDownloadCb,
    ADM_ECS_REPORTEVENT_CALLBACK_F             fnReportEventCb,
    ADM_ECS_DECRYPTANDSIGNATURE_CALLBACK_F     fnDecryptAndSignatureCb,
    ADM_ECS_FILESYNCTRANS_CALLBACK_F           fnFileSyncTransCb
)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    ADM_ECS_RSCONFIGDATA_T *pstOtaConfig = NULL;
    D32S errNum = 0;

    if (ADM_IS_NULL(fnGetReqCloudParamCb) || ADM_IS_NULL(fnRegReqCloudCb) || ADM_IS_NULL(fnRegDownloadCb))
    {
        log_e("Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }
    
    if (TRUE == gstEcsModuleInfo.isInited)
    {
        return iRet;
    }

    do
    {
        iRet = pthread_rwlock_init(&gstEcsModuleInfo.stRWlock, NULL);
        if (ADM_IS_ERR(iRet))
        {
            errNum = errno;
            iRet = ADM_ECODE_RWLOCK_INIT_FAILED;
            log_e("Failed to init rwlock, err[%d](%s)", iRet, strerror(errNum));
            break;
        }

        gstEcsModuleInfo.fnGetReqCloudParamApi = fnGetReqCloudParamCb; 
        gstEcsModuleInfo.fnReqCloudApi         = fnRegReqCloudCb; 
        gstEcsModuleInfo.fnDownloadApi         = fnRegDownloadCb; 
        gstEcsModuleInfo.fnReportEventApi      = fnReportEventCb; 
        gstEcsModuleInfo.fnDecryptAndSignatureApi = fnDecryptAndSignatureCb;
        gstEcsModuleInfo.fnFileSyncTransferApi = fnFileSyncTransCb;
        gstEcsModuleInfo.isInited     = TRUE;
        gstEcsModuleInfo.pstEcsConfig = NULL;
        gstEcsModuleInfo.isDataInited = FALSE;

        //!< 尝试从kv中获取缓存
        pstOtaConfig = __adm_ecs_getOtaConfigCacheInfo();
        if (NULL != pstOtaConfig)
        {
            gstEcsModuleInfo.pstEcsConfig = pstOtaConfig;
            gstEcsModuleInfo.isDataInited = TRUE;     
        }   
    } while (FALSE);

    return iRet;
}


/*!
 *******************************************************************************
 * ecs模块销毁.
 *
 * \note
 *
 * \param   none
 * \retval  none
 *******************************************************************************
 */
void adm_ecs_moduleDeInit(void)
{
    if (TRUE == gstEcsModuleInfo.isInited)
    {
        (void)pthread_rwlock_wrlock(&gstEcsModuleInfo.stRWlock);
        adm_ecs_delRsCfgDataInfo(&gstEcsModuleInfo.pstEcsConfig);
        gstEcsModuleInfo.pstEcsConfig = NULL;
        gstEcsModuleInfo.isDataInited = FALSE;
        (void)pthread_rwlock_unlock(&gstEcsModuleInfo.stRWlock);
        
        (void)pthread_rwlock_destroy(&gstEcsModuleInfo.stRWlock);
        gstEcsModuleInfo.isInited = FALSE;      
    }

    return;
}


/*!
 *******************************************************************************
 * 获取ecs下载的配置文件存储目录.
 *
 * \note 
 *
 * \retval  DCHAR *
 *******************************************************************************
 */
DCHAR *adm_ecs_getDownConfigFileDir(void)
{
    DCHAR szPkgRunDIr[ADM_COM_FILEPATH_LEN] = {0};
    DCHAR *pcCoreServiceRuningDir = NULL;

    //!< 获取系统配置的存储升级包目录
    pcCoreServiceRuningDir = adm_core_service_getRuntimeDir(ADM_CORE_SERVICE_RUNTIME_DIR_TYPE_PACKAGE);
    if (ADM_IS_NULL(pcCoreServiceRuningDir))
    {
        log_e("Failed to get coreService package runingDir, err[%d]", ADM_ECODE_CORESERVICE_GETRUNDIR_FAILED);
        return NULL;    
    }
    //!< 系统配置的存储包目录继续细化配置文件目录
    (void)adm_snprintf(szPkgRunDIr, sizeof(szPkgRunDIr), "%s%s/", 
                 pcCoreServiceRuningDir, ADM_ECS_DOWNCONF_RUNDIR_SUFFIX);
    log_d("Config package save dir[%s].", szPkgRunDIr);
    adm_free(pcCoreServiceRuningDir);

    return adm_strdup(szPkgRunDIr);
}


/*!
 *******************************************************************************
 * 获取ecs下载的配置文件解压目录.
 *
 * \note 
 *
 * \retval  DCHAR *
 *******************************************************************************
 */
DCHAR *adm_ecs_getDownConfigFileUnzipDir(void)
{
    DCHAR szPkgRunDIr[ADM_COM_FILEPATH_LEN] = {0};
    DCHAR *pcCoreServiceRuningDir = NULL;

    //!< 获取系统配置的存储升级包目录
    pcCoreServiceRuningDir = adm_core_service_getRuntimeDir(ADM_CORE_SERVICE_RUNTIME_DIR_TYPE_UNZIP);
    if (ADM_IS_NULL(pcCoreServiceRuningDir))
    {
        log_e("Failed to get coreService package runingDir, err[%d]", ADM_ECODE_CORESERVICE_GETRUNDIR_FAILED);
        return NULL;    
    }
    //!< 系统配置的存储包目录继续细化配置文件解压目录
    (void)adm_snprintf(szPkgRunDIr, sizeof(szPkgRunDIr), "%s%s/", 
                 pcCoreServiceRuningDir, ADM_ECS_DOWNCONF_RUNDIR_SUFFIX);
    log_d("Config package unzip dir[%s].", szPkgRunDIr);
    adm_free(pcCoreServiceRuningDir);

    return adm_strdup(szPkgRunDIr);
}


#if 0
#endif


/*!
 *******************************************************************************
 * 获取配置信息版本号.
 *
 * \note 获取的配置版本号外部需要调用 adm_free() 释放
 * 
 * \param   none
 * \retval  DCHAR *
 *******************************************************************************
 */
DCHAR *adm_ecs_getRsConfigVersion(void)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    DCHAR *confVersion = NULL;

    iRet = __adm_ecs_checkAndInitModule();
    if (ADM_IS_ERR(iRet))
    {
        log_e("Failed to check module info, err[%d]", iRet);
        return NULL;
    }

    (void)pthread_rwlock_rdlock(&gstEcsModuleInfo.stRWlock);
    confVersion = adm_strdup(gstEcsModuleInfo.pstEcsConfig->confVersion);
    (void)pthread_rwlock_unlock(&gstEcsModuleInfo.stRWlock);

    return confVersion;
}


/*!
 *******************************************************************************
 * 获取通用配置信息协议版本.
 *
 * \note 获取的配置版本号外部需要调用 adm_free() 释放
 * 
 * \param   none
 * \retval  DCHAR *
 *******************************************************************************
 */
DCHAR *adm_ecs_getRsCfgGeneralVersion(void)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    DCHAR *protocolVersion = NULL;

    iRet = __adm_ecs_checkAndInitModule();
    if (ADM_IS_ERR(iRet))
    {
        log_e("Failed to check module info, err[%d]", iRet);
        return NULL;
    }

    (void)pthread_rwlock_rdlock(&gstEcsModuleInfo.stRWlock);    
    protocolVersion = adm_strdup(gstEcsModuleInfo.pstEcsConfig->stRsCfgInfo.stGeneralCfg.protocolVersion);
    (void)pthread_rwlock_unlock(&gstEcsModuleInfo.stRWlock);

    return protocolVersion;
}

/*!
 *******************************************************************************
 * 获取系统巡检周期.
 *
 * \note    
 * 
 * \param[out] ps32PollingCycle 巡检周期,单位：小时, 范围 [0, 255]
 * \retval     D32S
 *******************************************************************************
 */
D32S adm_ecs_getPollingCycle(D32S *ps32PollingCycle)
{
    D32S iRet = ADM_ECODE_SUCCESSED;

    if (ADM_IS_NULL(ps32PollingCycle))
    {
        log_e("Invalid input param.");        
        return ADM_ECODE_INVALID_PARAM;
    }

    iRet = __adm_ecs_checkAndInitModule();
    if (ADM_IS_ERR(iRet))
    {
        log_e("Failed to check module info, err[%d]", iRet);
        return iRet;
    }

    (void)pthread_rwlock_rdlock(&gstEcsModuleInfo.stRWlock);
    *ps32PollingCycle = gstEcsModuleInfo.pstEcsConfig->stRsCfgInfo.stGeneralCfg.pollingCycle;
    (void)pthread_rwlock_unlock(&gstEcsModuleInfo.stRWlock);

    return ADM_ECODE_SUCCESSED;
}


/*!
 *******************************************************************************
 * 获取系统日志文件接口地址，仅域名部分
 *
 * \note 注:可选字段，调用者需判断是否为 NULL
 * \note 获取的信息外部需要调用 adm_free() 释放
 * 
 * \retval DCHAR *
 *******************************************************************************
 */

DCHAR *adm_ecs_getLogReportUrl(void)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    DCHAR *pcLogReportUrl = NULL;

    iRet = __adm_ecs_checkAndInitModule();
    if (ADM_IS_ERR(iRet))
    {
        log_e("Failed to check module info, err[%d]", iRet);
        return NULL;
    }

    (void)pthread_rwlock_rdlock(&gstEcsModuleInfo.stRWlock);
    if (NULL != gstEcsModuleInfo.pstEcsConfig->stRsCfgInfo.stGeneralCfg.logReportUrl)
    {
        pcLogReportUrl = adm_strdup(gstEcsModuleInfo.pstEcsConfig->stRsCfgInfo.stGeneralCfg.logReportUrl);
    }
    (void)pthread_rwlock_unlock(&gstEcsModuleInfo.stRWlock);

    return pcLogReportUrl;
}

/*!
 *******************************************************************************
 * 获取系统日志上报周期.
 *
 * \note    
 * 
 * \param[out] ps32LogReportCycle 日志上报周期,单位：s
 * \retval     D32S
 *******************************************************************************
 */
D32S adm_ecs_getLogReportCycle(D32S *ps32LogReportCycle)
{
    D32S iRet = ADM_ECODE_SUCCESSED;

    if (ADM_IS_NULL(ps32LogReportCycle))
    {
        log_e("Invalid input param.");        
        return ADM_ECODE_INVALID_PARAM;
    }

    iRet = __adm_ecs_checkAndInitModule();
    if (ADM_IS_ERR(iRet))
    {
        log_e("Failed to check module info, err[%d]", iRet);
        return iRet;
    }

    (void)pthread_rwlock_rdlock(&gstEcsModuleInfo.stRWlock);
    *ps32LogReportCycle = gstEcsModuleInfo.pstEcsConfig->stRsCfgInfo.stGeneralCfg.logReportCycle;
    (void)pthread_rwlock_unlock(&gstEcsModuleInfo.stRWlock);

    return ADM_ECODE_SUCCESSED;
}

/*!
 *******************************************************************************
 * 获取系统日志打印等级.
 *
 * \note
 * 
 * \param[out] ps32LogPrintLevel 日志打印等级
 * \retval     D32S
 *******************************************************************************
 */
D32S adm_ecs_getLogPrintLevel(D32S *ps32LogPrintLevel)
{
    D32S iRet = ADM_ECODE_SUCCESSED;

    if (ADM_IS_NULL(ps32LogPrintLevel))
    {
        log_e("Invalid input param.");        
        return ADM_ECODE_INVALID_PARAM;
    }

    iRet = __adm_ecs_checkAndInitModule();
    if (ADM_IS_ERR(iRet))
    {
        log_e("Failed to check module info, err[%d]", iRet);
        return iRet;
    }


    (void)pthread_rwlock_rdlock(&gstEcsModuleInfo.stRWlock);
    *ps32LogPrintLevel = gstEcsModuleInfo.pstEcsConfig->stRsCfgInfo.stGeneralCfg.logPrintLevel;
    (void)pthread_rwlock_unlock(&gstEcsModuleInfo.stRWlock);

    return ADM_ECODE_SUCCESSED;
}

/*!
 *******************************************************************************
 * 获取车端上报日志文件是否需要加密.
 *
 * \note
 * 
 * \param[out] ps32IsLogFileEncrypted 车端上报日志文件是否需要加密
 * \retval     D32S
 *******************************************************************************
 */
D32S adm_ecs_getIsLogFileEncrypted(D32S *ps32IsLogFileEncrypted)
{
    D32S iRet = ADM_ECODE_SUCCESSED;

    if (ADM_IS_NULL(ps32IsLogFileEncrypted))
    {
        log_e("Invalid input param.");        
        return ADM_ECODE_INVALID_PARAM;
    }

    iRet = __adm_ecs_checkAndInitModule();
    if (ADM_IS_ERR(iRet))
    {
        log_e("Failed to check module info, err[%d]", iRet);
        return iRet;
    }


    (void)pthread_rwlock_rdlock(&gstEcsModuleInfo.stRWlock);
    *ps32IsLogFileEncrypted = gstEcsModuleInfo.pstEcsConfig->stRsCfgInfo.stGeneralCfg.isLogFileEncrypted;
    (void)pthread_rwlock_unlock(&gstEcsModuleInfo.stRWlock);

    return ADM_ECODE_SUCCESSED;
}

/*!
 *******************************************************************************
 * 获取车端升级成功日志是否需要上报.
 *
 * \note
 * 
 * \param[out] ps32isLogFileEncrypted 车端升级成功日志是否需要上报
 * \retval     D32S
 *******************************************************************************
 */
D32S adm_ecs_getIsReportUpSuccLogFile(D32S *ps32IsReportUpSuccLogFile)
{
    D32S iRet = ADM_ECODE_SUCCESSED;

    if (ADM_IS_NULL(ps32IsReportUpSuccLogFile))
    {
        log_e("Invalid input param.");        
        return ADM_ECODE_INVALID_PARAM;
    }

    iRet = __adm_ecs_checkAndInitModule();
    if (ADM_IS_ERR(iRet))
    {
        log_e("Failed to check module info, err[%d]", iRet);
        return iRet;
    }


    (void)pthread_rwlock_rdlock(&gstEcsModuleInfo.stRWlock);
    *ps32IsReportUpSuccLogFile = gstEcsModuleInfo.pstEcsConfig->stRsCfgInfo.stGeneralCfg.isReportUpSuccLogFile;
    (void)pthread_rwlock_unlock(&gstEcsModuleInfo.stRWlock);

    return ADM_ECODE_SUCCESSED;
}

/*!
 *******************************************************************************
 * 获取事件上报等级.
 *
 * \note
 * 
 * \param[out] ps32EventReportLevel 事件上报等级
 * \retval     D32S
 *******************************************************************************
 */
D32S adm_ecs_getEventReportLevel(D32S *ps32EventReportLevel)
{
    D32S iRet = ADM_ECODE_SUCCESSED;

    if (ADM_IS_NULL(ps32EventReportLevel))
    {
        log_e("Invalid input param.");        
        return ADM_ECODE_INVALID_PARAM;
    }

    iRet = __adm_ecs_checkAndInitModule();
    if (ADM_IS_ERR(iRet))
    {
        log_e("Failed to check module info, err[%d]", iRet);
        return iRet;
    }

    *ps32EventReportLevel = gstEcsModuleInfo.pstEcsConfig->stRsCfgInfo.stGeneralCfg.eventReportLevel;

	log_i("ps32EventReportLevel [%d]", *ps32EventReportLevel);

    return ADM_ECODE_SUCCESSED;
}


/*!
 *******************************************************************************
 * 获取系统事件上报周期.
 *
 * \note    
 * 
 * \param[out] ps32EventReportCycle 事件上报周期,单位：s 
 * \retval     D32S
 *******************************************************************************
 */
D32S adm_ecs_getEventReportCycle(D32S *ps32EventReportCycle)
{
    D32S iRet = ADM_ECODE_SUCCESSED;

    if (ADM_IS_NULL(ps32EventReportCycle))
    {
        log_e("Invalid input param.");        
        return ADM_ECODE_INVALID_PARAM;
    }

    iRet = __adm_ecs_checkAndInitModule();
    if (ADM_IS_ERR(iRet))
    {
        log_e("Failed to check module info, err[%d]", iRet);
        return iRet;
    }


    (void)pthread_rwlock_rdlock(&gstEcsModuleInfo.stRWlock);
    *ps32EventReportCycle = gstEcsModuleInfo.pstEcsConfig->stRsCfgInfo.stGeneralCfg.eventReportCycle;
    (void)pthread_rwlock_unlock(&gstEcsModuleInfo.stRWlock);

    return ADM_ECODE_SUCCESSED;
}

/*!
 *******************************************************************************
 * 获取系统状态上报周期.
 *
 * \note    
 * 
 * \param[out] ps32StateReportCycle 状态上报周期,单位：s 
 * \retval     D32S
 *******************************************************************************
 */
D32S adm_ecs_getStateReportCycle(D32S *ps32StateReportCycle)
{
    D32S iRet = ADM_ECODE_SUCCESSED;

    if (ADM_IS_NULL(ps32StateReportCycle))
    {
        log_e("Invalid input param.");        
        return ADM_ECODE_INVALID_PARAM;
    }

    iRet = __adm_ecs_checkAndInitModule();
    if (ADM_IS_ERR(iRet))
    {
        log_e("Failed to check module info, err[%d]", iRet);
        return iRet;
    }

    (void)pthread_rwlock_rdlock(&gstEcsModuleInfo.stRWlock);
    *ps32StateReportCycle = gstEcsModuleInfo.pstEcsConfig->stRsCfgInfo.stGeneralCfg.stateReportCycle;
    (void)pthread_rwlock_unlock(&gstEcsModuleInfo.stRWlock);

    return ADM_ECODE_SUCCESSED;
}

/*!
 *******************************************************************************
 * 获取系统结果上报周期.
 *
 * \note    
 * 
 * \param[out] ps32SummaryReportCycle 结果上报周期,单位：s
 * \retval     D32S
 *******************************************************************************
 */
D32S adm_ecs_getSummaryReportCycle(D32S *ps32SummaryReportCycle)
{
    D32S iRet = ADM_ECODE_SUCCESSED;

    if (ADM_IS_NULL(ps32SummaryReportCycle))
    {
        log_e("Invalid input param.");        
        return ADM_ECODE_INVALID_PARAM;
    }

    iRet = __adm_ecs_checkAndInitModule();
    if (ADM_IS_ERR(iRet))
    {
        log_e("Failed to check module info, err[%d]", iRet);
        return iRet;
    }

    (void)pthread_rwlock_rdlock(&gstEcsModuleInfo.stRWlock);
    *ps32SummaryReportCycle = gstEcsModuleInfo.pstEcsConfig->stRsCfgInfo.stGeneralCfg.summaryReportCycle;
    (void)pthread_rwlock_unlock(&gstEcsModuleInfo.stRWlock);

    return ADM_ECODE_SUCCESSED;
}

/*!
 *******************************************************************************
 * 获取系统进度上报周期.
 *
 * \note    下载/传输/安装...
 * 
 * \param[out] ps32ProgressReportCycle 结果上报周期,单位：s
 * \retval     D32S
 *******************************************************************************
 */
D32S adm_ecs_getProgressReportCycle(D32S *ps32ProgressReportCycle)
{
    D32S iRet = ADM_ECODE_SUCCESSED;

    if (ADM_IS_NULL(ps32ProgressReportCycle))
    {
        log_e("Invalid input param.");        
        return ADM_ECODE_INVALID_PARAM;
    }

    iRet = __adm_ecs_checkAndInitModule();
    if (ADM_IS_ERR(iRet))
    {
        log_e("Failed to check module info, err[%d]", iRet);
        return iRet;
    }

    (void)pthread_rwlock_rdlock(&gstEcsModuleInfo.stRWlock);
    *ps32ProgressReportCycle = gstEcsModuleInfo.pstEcsConfig->stRsCfgInfo.stGeneralCfg.progressReportCycle;
    (void)pthread_rwlock_unlock(&gstEcsModuleInfo.stRWlock);

    return ADM_ECODE_SUCCESSED;
}


/*!
 *******************************************************************************
 * 获取触发版本检测的轮询周期.
 *
 * \note
 * 
 * \param[out] ps32CyclePeriod 触发版本检测的轮询周期（单位：s）
 * \retval     D32S
 *******************************************************************************
 */
D32S adm_ecs_getCheckCyclePeriod(D32S *ps32CyclePeriod)
{
    D32S iRet = ADM_ECODE_SUCCESSED;

    if (ADM_IS_NULL(ps32CyclePeriod))
    {
        log_e("Invalid input param.");        
        return ADM_ECODE_INVALID_PARAM;
    }

    iRet = __adm_ecs_checkAndInitModule();
    if (ADM_IS_ERR(iRet))
    {
        log_e("Failed to check module info, err[%d]", iRet);
        return iRet;
    }

    (void)pthread_rwlock_rdlock(&gstEcsModuleInfo.stRWlock);
    *ps32CyclePeriod = gstEcsModuleInfo.pstEcsConfig->stRsCfgInfo.stGeneralCfg.cyclePeriod;
    (void)pthread_rwlock_unlock(&gstEcsModuleInfo.stRWlock);

    return ADM_ECODE_SUCCESSED;
}

/*!
 *******************************************************************************
 * 获取最大点火触发检测上限.
 *
 * \note
 * 
 * \param[out] ps32FireTriggerLimit 点火触发检测上限（单位：次/天）
 * \retval     D32S
 *******************************************************************************
 */
D32S adm_ecs_getMaxFireTriggerLimit(D32S *ps32FireTriggerLimit)
{
    D32S iRet = ADM_ECODE_SUCCESSED;

    if (ADM_IS_NULL(ps32FireTriggerLimit))
    {
        log_e("Invalid input param.");        
        return ADM_ECODE_INVALID_PARAM;
    }

    iRet = __adm_ecs_checkAndInitModule();
    if (ADM_IS_ERR(iRet))
    {
        log_e("Failed to check module info, err[%d]", iRet);
        return iRet;
    }

    (void)pthread_rwlock_rdlock(&gstEcsModuleInfo.stRWlock);
    *ps32FireTriggerLimit = gstEcsModuleInfo.pstEcsConfig->stRsCfgInfo.stGeneralCfg.maxFireTrigger;
    (void)pthread_rwlock_unlock(&gstEcsModuleInfo.stRWlock);

    return ADM_ECODE_SUCCESSED;
}


#if 0
#endif

/*!
 *******************************************************************************
 * 获取零件信息采集实例ID(该指对该升级对象/单元的信息收集由哪个实例负责).
 *
 * \note 获取的信息外部需要调用 adm_free() 释放
 * 
 * \param[in]  ecuReqId    零件诊断请求ID
 * \param[in]  ecuSwDid    零件升级对象软件版本did 
 * \retval     DCHAR *
 *******************************************************************************
 */
DCHAR *adm_ecs_getReadInfoInstanceId(DCHAR *ecuReqId, DCHAR *ecuSwDid)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    D32S i = 0;
    DCHAR *pcReadInfoInstanceId = NULL;
    ADM_ECS_ECUCONFIG_T *pstEcuCfg = NULL;

    if (ADM_IS_NULL(ecuReqId) || ADM_IS_NULL(ecuSwDid))
    {
        log_e("Invalid input param.");        
        return NULL;
    }

    iRet = __adm_ecs_checkAndInitModule();
    if (ADM_IS_ERR(iRet))
    {
        log_e("Failed to check module info, err[%d]", iRet);
        return NULL;
    }

    (void)pthread_rwlock_rdlock(&gstEcsModuleInfo.stRWlock);
    for (i = 0; i< gstEcsModuleInfo.pstEcsConfig->stRsCfgInfo.stPartCfg.stEcuCfgList.num; i++)
    {
        pstEcuCfg = &gstEcsModuleInfo.pstEcsConfig->stRsCfgInfo.stPartCfg.stEcuCfgList.pstEcuCfg[i];
        if ((0 == adm_strcasecmp(ecuReqId, pstEcuCfg->ecuReqId)) &&
            (0 == adm_strcasecmp(ecuSwDid, pstEcuCfg->ecuSwDid)))
        {
            pcReadInfoInstanceId = adm_strdup(pstEcuCfg->readInfoInstanceId);
            break;
        }
    }
    (void)pthread_rwlock_unlock(&gstEcsModuleInfo.stRWlock);

    return pcReadInfoInstanceId;
}


/*!
 *******************************************************************************
 * 获取零件文件下载的实例ID(指对该文件的下载由哪个实例负责).
 *
 * \note
 * 
 * \param[in]  ecuReqId    零件诊断请求ID
 * \param[in]  ecuSwDid    零件升级对象软件版本did 
 * \retval     DCHAR *
 *******************************************************************************
 */
DCHAR *adm_ecs_getDownloadInstanceId(DCHAR *ecuReqId, DCHAR *ecuSwDid)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    D32S i = 0;
    DCHAR *pcDownloadInstanceId = NULL;
    ADM_ECS_ECUCONFIG_T *pstEcuCfg = NULL;

    if (ADM_IS_NULL(ecuReqId) || ADM_IS_NULL(ecuSwDid))
    {
        log_e("Invalid input param.");        
        return NULL;
    }

    iRet = __adm_ecs_checkAndInitModule();
    if (ADM_IS_ERR(iRet))
    {
        log_e("Failed to check module info, err[%d]", iRet);
        return NULL;
    }

    (void)pthread_rwlock_rdlock(&gstEcsModuleInfo.stRWlock);
    for (i = 0; i< gstEcsModuleInfo.pstEcsConfig->stRsCfgInfo.stPartCfg.stEcuCfgList.num; i++)
    {
        pstEcuCfg = &gstEcsModuleInfo.pstEcsConfig->stRsCfgInfo.stPartCfg.stEcuCfgList.pstEcuCfg[i];
        if ((0 == adm_strcasecmp(ecuReqId, pstEcuCfg->ecuReqId)) &&
            (0 == adm_strcasecmp(ecuSwDid, pstEcuCfg->ecuSwDid)))
        {
            pcDownloadInstanceId = adm_strdup(pstEcuCfg->downloadInstanceId);
            break;
        }
    }
    (void)pthread_rwlock_unlock(&gstEcsModuleInfo.stRWlock);

    return pcDownloadInstanceId;
}


/*!
 *******************************************************************************
 * 获取零件文件传输的实例ID(指对该文件的传输由哪个实例负责).
 *
 * \note
 * 
 * \param[in]  ecuReqId    零件诊断请求ID
 * \param[in]  ecuSwDid    零件升级对象软件版本did 
 * \retval     DCHAR *
 *******************************************************************************
 */
DCHAR *adm_ecs_getTransferInstanceId(DCHAR *ecuReqId, DCHAR *ecuSwDid)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    D32S i = 0;
    DCHAR *pcTransferInstanceId = NULL;
    ADM_ECS_ECUCONFIG_T *pstEcuCfg = NULL;

    if (ADM_IS_NULL(ecuReqId) || ADM_IS_NULL(ecuSwDid))
    {
        log_e("Invalid input param.");        
        return NULL;
    }

    iRet = __adm_ecs_checkAndInitModule();
    if (ADM_IS_ERR(iRet))
    {
        log_e("Failed to check module info, err[%d]", iRet);
        return NULL;
    }

    (void)pthread_rwlock_rdlock(&gstEcsModuleInfo.stRWlock);
    for (i = 0; i< gstEcsModuleInfo.pstEcsConfig->stRsCfgInfo.stPartCfg.stEcuCfgList.num; i++)
    {
        pstEcuCfg = &gstEcsModuleInfo.pstEcsConfig->stRsCfgInfo.stPartCfg.stEcuCfgList.pstEcuCfg[i];
        if ((0 == adm_strcasecmp(ecuReqId, pstEcuCfg->ecuReqId)) &&
            (0 == adm_strcasecmp(ecuSwDid, pstEcuCfg->ecuSwDid)))
        {
            pcTransferInstanceId = adm_strdup(pstEcuCfg->transferInstanceId);
            break;
        }
    }
    (void)pthread_rwlock_unlock(&gstEcsModuleInfo.stRWlock);

    return pcTransferInstanceId;
}


/*!
 *******************************************************************************
 * 获取零件升级的实例ID(指对该零件的升级由哪个实例负责).
 *
 * \note
 * 
 * \param[in]  ecuReqId    零件诊断请求ID
 * \param[in]  ecuSwDid    零件升级对象软件版本did 
 * \retval     DCHAR *
 *******************************************************************************
 */
DCHAR *adm_ecs_getInstallInstanceId(const DCHAR *ecuReqId, const DCHAR *ecuSwDid)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    D32S i = 0;
    DCHAR *pcInstallInstanceId = NULL;
    ADM_ECS_ECUCONFIG_T *pstEcuCfg = NULL;

    if (ADM_IS_NULL(ecuReqId) || ADM_IS_NULL(ecuSwDid))
    {
        log_e("Invalid input param.");        
        return NULL;
    }

    iRet = __adm_ecs_checkAndInitModule();
    if (ADM_IS_ERR(iRet))
    {
        log_e("Failed to check module info, err[%d]", iRet);
        return NULL;
    }

    (void)pthread_rwlock_rdlock(&gstEcsModuleInfo.stRWlock);
    for (i = 0; i< gstEcsModuleInfo.pstEcsConfig->stRsCfgInfo.stPartCfg.stEcuCfgList.num; i++)
    {
        pstEcuCfg = &gstEcsModuleInfo.pstEcsConfig->stRsCfgInfo.stPartCfg.stEcuCfgList.pstEcuCfg[i];
        if ((0 == adm_strcasecmp(ecuReqId, pstEcuCfg->ecuReqId)) &&
            (0 == adm_strcasecmp(ecuSwDid, pstEcuCfg->ecuSwDid)))
        {
            pcInstallInstanceId = adm_strdup(pstEcuCfg->installInstanceId);
            break;
        }
    }
    (void)pthread_rwlock_unlock(&gstEcsModuleInfo.stRWlock);

    return pcInstallInstanceId;
}

#if 0
#endif


/*!
 *******************************************************************************
 * 获取零件是否支持升级.
 *
 * \note    0:不支持自升级, 1:支持自升级 
 * 
 * \param[in]  ecuReqId     零件诊断请求ID
 * \param[in]  ecuSwDid     零件升级对象软件版本did 
 * \param[OUT] isSelfUpdate 零件是否支持自升级
 * \retval     D32S
 *******************************************************************************
 */
D32S adm_ecs_getEcuIsSelfUpdate(DCHAR *ecuReqId, DCHAR *ecuSwDid, D32S *isSelfUpdate)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    D32S i = 0;
    ADM_ECS_ECUCONFIG_T *pstEcuCfg = NULL;

    if (ADM_IS_NULL(ecuReqId) || ADM_IS_NULL(ecuSwDid) || ADM_IS_NULL(isSelfUpdate))
    {
        log_e("Invalid input param.");        
        return ADM_ECODE_INVALID_PARAM;
    }

    iRet = __adm_ecs_checkAndInitModule();
    if (ADM_IS_ERR(iRet))
    {
        log_e("Failed to check module info, err[%d]", iRet);
        return iRet;
    }

    (void)pthread_rwlock_rdlock(&gstEcsModuleInfo.stRWlock);
    for (i = 0; i< gstEcsModuleInfo.pstEcsConfig->stRsCfgInfo.stPartCfg.stEcuCfgList.num; i++)
    {
        pstEcuCfg = &gstEcsModuleInfo.pstEcsConfig->stRsCfgInfo.stPartCfg.stEcuCfgList.pstEcuCfg[i];
        if ((0 == adm_strcasecmp(ecuReqId, pstEcuCfg->ecuReqId)) &&
            (0 == adm_strcasecmp(ecuSwDid, pstEcuCfg->ecuSwDid)))
        {
            *isSelfUpdate = pstEcuCfg->isSelfUpdate;
            break;
        }
    }

    //!< 未找到对应的零件信息
    if (i == gstEcsModuleInfo.pstEcsConfig->stRsCfgInfo.stPartCfg.stEcuCfgList.num)
    {
        (void)pthread_rwlock_unlock(&gstEcsModuleInfo.stRWlock);
        log_e("Ecu part[reqId(%s), swDId(%s)] no find.", ecuReqId, ecuSwDid);    
        return ADM_ECODE_OTACFG_ECU_NONEXIST;
    }
    (void)pthread_rwlock_unlock(&gstEcsModuleInfo.stRWlock);

    return ADM_ECODE_SUCCESSED;
}


/*!
 *******************************************************************************
 * 获取零件是否有系统自备份.
 *
 * \note    1:有系统自备份, 2:无系统自备份 
 * 
 * \param[in]  ecuReqId     零件诊断请求ID
 * \param[in]  ecuSwDid     零件升级对象软件版本did 
 * \param[OUT] selfBackup   零件是否有自备份
 * \retval     D32S
 *******************************************************************************
 */
D32S adm_ecs_getEcuSelfBackUp(DCHAR *ecuReqId, DCHAR *ecuSwDid, DBOOL *selfBackup)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    D32S i = 0;
    ADM_ECS_ECUCONFIG_T *pstEcuCfg = NULL;

    if (ADM_IS_NULL(ecuReqId) || ADM_IS_NULL(ecuSwDid) || ADM_IS_NULL(selfBackup))
    {
        log_e("Invalid input param.");        
        return ADM_ECODE_INVALID_PARAM;
    }

    iRet = __adm_ecs_checkAndInitModule();
    if (ADM_IS_ERR(iRet))
    {
        log_e("Failed to check module info, err[%d]", iRet);
        return iRet;
    }

    (void)pthread_rwlock_rdlock(&gstEcsModuleInfo.stRWlock);
    for (i = 0; i< gstEcsModuleInfo.pstEcsConfig->stRsCfgInfo.stPartCfg.stEcuCfgList.num; i++)
    {
        pstEcuCfg = &gstEcsModuleInfo.pstEcsConfig->stRsCfgInfo.stPartCfg.stEcuCfgList.pstEcuCfg[i];
        if ((0 == adm_strcasecmp(ecuReqId, pstEcuCfg->ecuReqId)) &&
            (0 == adm_strcasecmp(ecuSwDid, pstEcuCfg->ecuSwDid)))
        {
            log_i("Self back[%d]", pstEcuCfg->selfBackup);
            if (1 == pstEcuCfg->selfBackup)
            {
               *selfBackup = DTRUE; 
            }
            else 
            {
                *selfBackup = DFALSE;
            }
            break;
        }
    }

    //!< 未找到对应的零件信息
    if (i == gstEcsModuleInfo.pstEcsConfig->stRsCfgInfo.stPartCfg.stEcuCfgList.num)
    {
        (void)pthread_rwlock_unlock(&gstEcsModuleInfo.stRWlock);
        log_e("Ecu part[reqId(%s), swDId(%s)] no find.", ecuReqId, ecuSwDid);    
        return ADM_ECODE_OTACFG_ECU_NONEXIST;
    }
    (void)pthread_rwlock_unlock(&gstEcsModuleInfo.stRWlock);

    return ADM_ECODE_SUCCESSED;
}



/*!
 *******************************************************************************
 * 获取零件传输协议.
 *
 * \note    1:CAN, 2:CANFD, 3:ETH(DoIP), 4:LIN 
 * 
 * \param[in]  ecuReqId  零件诊断请求ID
 * \param[in]  ecuSwDid  零件升级对象软件版本did 
 * \param[OUT] busType   零件刷写类型
 * \retval     D32S
 *******************************************************************************
 */
D32S adm_ecs_getBusType(const DCHAR *ecuReqId, const DCHAR *ecuSwDid, D32S *busType)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    D32S i = 0;
    ADM_ECS_ECUCONFIG_T *pstEcuCfg = NULL;

    if (ADM_IS_NULL(ecuReqId) || ADM_IS_NULL(ecuSwDid) || ADM_IS_NULL(busType))
    {
        log_e("Invalid input param.");        
        return ADM_ECODE_INVALID_PARAM;
    }

    iRet = __adm_ecs_checkAndInitModule();
    if (ADM_IS_ERR(iRet))
    {
        log_e("Failed to check module info, err[%d]", iRet);
        return iRet;
    }

    (void)pthread_rwlock_rdlock(&gstEcsModuleInfo.stRWlock);
    for (i = 0; i< gstEcsModuleInfo.pstEcsConfig->stRsCfgInfo.stPartCfg.stEcuCfgList.num; i++)
    {
        pstEcuCfg = &gstEcsModuleInfo.pstEcsConfig->stRsCfgInfo.stPartCfg.stEcuCfgList.pstEcuCfg[i];
        if ((0 == adm_strcasecmp(ecuReqId, pstEcuCfg->ecuReqId)) &&
            (0 == adm_strcasecmp(ecuSwDid, pstEcuCfg->ecuSwDid)))
        {
            *busType = pstEcuCfg->busType;
            break;
        }
    }

    //!< 未找到对应的零件信息
    if (i == gstEcsModuleInfo.pstEcsConfig->stRsCfgInfo.stPartCfg.stEcuCfgList.num)
    {
        (void)pthread_rwlock_unlock(&gstEcsModuleInfo.stRWlock);
        log_e("Ecu part[reqId(%s), swDId(%s)] no find.", ecuReqId, ecuSwDid);    
        return ADM_ECODE_OTACFG_ECU_NONEXIST;
    }
    (void)pthread_rwlock_unlock(&gstEcsModuleInfo.stRWlock);

    return ADM_ECODE_SUCCESSED;
}


/*!
 *******************************************************************************
 * 获取零件CAN网络通道类型.
 *
 * \note    1:ACAN, 2:BCAN, 3:PCAN 
 * 
 * \param[in]  ecuReqId     零件诊断请求ID
 * \param[in]  ecuSwDid     零件升级对象软件版本did 
 * \param[OUT] channelType  零件CAN网络通道类型
 * \retval     D32S
 *******************************************************************************
 */
D32S adm_ecs_getChannelType
(
    const DCHAR *ecuReqId, 
    const DCHAR *ecuSwDid, 
    D32S  *channelType
)
{
    D32S i = 0;
    D32S iRet = ADM_ECODE_SUCCESSED;
    ADM_ECS_ECUCONFIG_T *pstEcuCfg = NULL;

    if (ADM_IS_NULL(ecuReqId) || ADM_IS_NULL(ecuSwDid) || ADM_IS_NULL(channelType))
    {
        log_e("Invalid input param.");        
        return ADM_ECODE_INVALID_PARAM;
    }

    iRet = __adm_ecs_checkAndInitModule();
    if (ADM_IS_ERR(iRet))
    {
        log_e("Failed to check module info, err[%d]", iRet);
        return iRet;
    }

    (void)pthread_rwlock_rdlock(&gstEcsModuleInfo.stRWlock);
    for (i = 0; i< gstEcsModuleInfo.pstEcsConfig->stRsCfgInfo.stPartCfg.stEcuCfgList.num; i++)
    {
        pstEcuCfg = &gstEcsModuleInfo.pstEcsConfig->stRsCfgInfo.stPartCfg.stEcuCfgList.pstEcuCfg[i];
        if ((0 == adm_strcasecmp(ecuReqId, pstEcuCfg->ecuReqId)) &&
            (0 == adm_strcasecmp(ecuSwDid, pstEcuCfg->ecuSwDid)))
        {
            *channelType = pstEcuCfg->channelType;
            break;
        }
    }

    //!< 未找到对应的零件信息
    if (i == gstEcsModuleInfo.pstEcsConfig->stRsCfgInfo.stPartCfg.stEcuCfgList.num)
    {
        (void)pthread_rwlock_unlock(&gstEcsModuleInfo.stRWlock);
        log_e("Ecu part[reqId(%s), swDId(%s)] no find.", ecuReqId, ecuSwDid);   
        return ADM_ECODE_OTACFG_ECU_NONEXIST;
    }
    (void)pthread_rwlock_unlock(&gstEcsModuleInfo.stRWlock);

    return ADM_ECODE_SUCCESSED;
}

/*!
 *******************************************************************************
 * 获取零件信息.
 *
 * 
 * \param[in]  ecuReqId     零件诊断请求ID
 * \param[in]  ecuSwDid     零件升级对象软件版本did 
 * \param[out] ppResponseId 零件响应地址
 * \param[out] ppFunctionId 零件函数地址
 * \retval     D32S
 *******************************************************************************
 */
D32S adm_ecs_getEcuInfo
(
    const DCHAR* pEcuReqId, 
    const DCHAR* pEcuSwDid, 
    DCHAR** ppResponseId,
    DCHAR** ppFunctionId
)
{
    D32S i = 0;
    D32S iRet = ADM_ECODE_SUCCESSED;
    ADM_ECS_ECUCONFIG_T *pstEcuCfg = NULL;

    if (ADM_IS_NULL(pEcuReqId) || ADM_IS_NULL(pEcuSwDid) || ADM_IS_NULL(ppResponseId) || ADM_IS_NULL(ppFunctionId))
    {
        log_e("Invalid input param.");        
        return ADM_ECODE_INVALID_PARAM;
    }

    iRet = __adm_ecs_checkAndInitModule();
    if (ADM_IS_ERR(iRet))
    {
        log_e("Failed to check module info, err[%d]", iRet);
        return iRet;
    }

    (void)pthread_rwlock_rdlock(&gstEcsModuleInfo.stRWlock);
    for (i = 0; i< gstEcsModuleInfo.pstEcsConfig->stRsCfgInfo.stPartCfg.stEcuCfgList.num; i++)
    {
        pstEcuCfg = &gstEcsModuleInfo.pstEcsConfig->stRsCfgInfo.stPartCfg.stEcuCfgList.pstEcuCfg[i];
        if ((0 == adm_strcasecmp(pEcuReqId, pstEcuCfg->ecuReqId)) &&
            (0 == adm_strcasecmp(pEcuSwDid, pstEcuCfg->ecuSwDid)))
        {
            *ppResponseId = adm_strdup(pstEcuCfg->ecuResponseId);
            *ppFunctionId = adm_strdup(pstEcuCfg->ecuFunctionId);
            break;
        }
    }

    //!< 未找到对应的零件信息
    if (i == gstEcsModuleInfo.pstEcsConfig->stRsCfgInfo.stPartCfg.stEcuCfgList.num)
    {
        (void)pthread_rwlock_unlock(&gstEcsModuleInfo.stRWlock);
        log_e("Ecu part[reqId(%s), swDId(%s)] no find.", pEcuReqId, pEcuSwDid);   
        return ADM_ECODE_OTACFG_ECU_NONEXIST;
    }
    (void)pthread_rwlock_unlock(&gstEcsModuleInfo.stRWlock);

    return ADM_ECODE_SUCCESSED;
}


/*!
 *******************************************************************************
 * 获取零件crc算法掩码.
 *
 * \note
 * 
 * \param[in]  ecuReqId   零件诊断请求ID
 * \param[in]  ecuSwDid   零件升级对象软件版本did 
 * \param[OUT] crcMask    零件crc算法掩码
 * \retval     D32S
 *******************************************************************************
 */
D32S adm_ecs_getEcuCrcMask(DCHAR *ecuReqId, DCHAR *ecuSwDid, D32S *crcMask)
{
    D32S i = 0;
    D32S iRet = ADM_ECODE_SUCCESSED;
    ADM_ECS_ECUCONFIG_T *pstEcuCfg = NULL;

    if (ADM_IS_NULL(ecuReqId) || ADM_IS_NULL(ecuSwDid) || ADM_IS_NULL(crcMask))
    {
        log_e("Invalid input param.");        
        return ADM_ECODE_INVALID_PARAM;
    }

    iRet = __adm_ecs_checkAndInitModule();
    if (ADM_IS_ERR(iRet))
    {
        log_e("Failed to check module info, err[%d]", iRet);
        return iRet;
    }

    (void)pthread_rwlock_rdlock(&gstEcsModuleInfo.stRWlock);
    for (i = 0; i< gstEcsModuleInfo.pstEcsConfig->stRsCfgInfo.stPartCfg.stEcuCfgList.num; i++)
    {
        pstEcuCfg = &gstEcsModuleInfo.pstEcsConfig->stRsCfgInfo.stPartCfg.stEcuCfgList.pstEcuCfg[i];
        if ((0 == adm_strcasecmp(ecuReqId, pstEcuCfg->ecuReqId)) &&
            (0 == adm_strcasecmp(ecuSwDid, pstEcuCfg->ecuSwDid)))
        {
            *crcMask = pstEcuCfg->crcMask;
            break;
        }
    }

    //!< 未找到对应的零件信息
    if (i == gstEcsModuleInfo.pstEcsConfig->stRsCfgInfo.stPartCfg.stEcuCfgList.num)
    {
        (void)pthread_rwlock_unlock(&gstEcsModuleInfo.stRWlock);
        log_e("Ecu part[reqId(%s), swDId(%s)] no find.", ecuReqId, ecuSwDid);   
        return ADM_ECODE_OTACFG_ECU_NONEXIST;
    }
    (void)pthread_rwlock_unlock(&gstEcsModuleInfo.stRWlock);

    return ADM_ECODE_SUCCESSED;
}


/*!
 *******************************************************************************
 * 获取零件crc算法库编号.
 *
 * \note
 * 
 * \param[in]  ecuReqId     零件诊断请求ID
 * \param[in]  ecuSwDid     零件升级对象软件版本did 
 * \param[OUT] crcLibNumber 零件crc算法库编号
 * \retval     D32S
 *******************************************************************************
 */
D32S adm_ecs_getEcuCrcLibNumber
(
    DCHAR *ecuReqId, 
    DCHAR *ecuSwDid, 
    D32S  *crcLibNumber
)
{
    D32S i = 0;
    D32S iRet = ADM_ECODE_SUCCESSED;
    ADM_ECS_ECUCONFIG_T *pstEcuCfg = NULL;

    if (ADM_IS_NULL(ecuReqId) || ADM_IS_NULL(ecuSwDid) || ADM_IS_NULL(crcLibNumber))
    {
        log_e("Invalid input param.");        
        return ADM_ECODE_INVALID_PARAM;
    }

    iRet = __adm_ecs_checkAndInitModule();
    if (ADM_IS_ERR(iRet))
    {
        log_e("Failed to check module info, err[%d]", iRet);
        return iRet;
    }

    (void)pthread_rwlock_rdlock(&gstEcsModuleInfo.stRWlock);
    for (i = 0; i< gstEcsModuleInfo.pstEcsConfig->stRsCfgInfo.stPartCfg.stEcuCfgList.num; i++)
    {
        pstEcuCfg = &gstEcsModuleInfo.pstEcsConfig->stRsCfgInfo.stPartCfg.stEcuCfgList.pstEcuCfg[i];
        if ((0 == adm_strcasecmp(ecuReqId, pstEcuCfg->ecuReqId)) &&
            (0 == adm_strcasecmp(ecuSwDid, pstEcuCfg->ecuSwDid)))
        {
            *crcLibNumber = pstEcuCfg->crcLibNumber;
            break;
        }
    }

    //!< 未找到对应的零件信息
    if (i == gstEcsModuleInfo.pstEcsConfig->stRsCfgInfo.stPartCfg.stEcuCfgList.num)
    {
        (void)pthread_rwlock_unlock(&gstEcsModuleInfo.stRWlock);
        log_e("Ecu part[reqId(%s), swDId(%s)] no find.", ecuReqId, ecuSwDid);    
        return ADM_ECODE_OTACFG_ECU_NONEXIST;
    }
    (void)pthread_rwlock_unlock(&gstEcsModuleInfo.stRWlock);

    return ADM_ECODE_SUCCESSED;
}


/*!
 *******************************************************************************
 * 获取零件安全算法掩码.
 *
 * \note
 * 
 * \param[in]  ecuReqId   零件诊断请求ID
 * \param[in]  ecuSwDid   零件升级对象软件版本did 
 * \param[OUT] saMask     零件安全算法掩码
 * \retval     D32S
 *******************************************************************************
 */
D32S adm_ecs_getEcuSaMask
(
    const DCHAR *ecuReqId, 
    const DCHAR *ecuSwDid, 
    DCHAR **saMask
)
{
    D32S i = 0;
    D32S iRet = ADM_ECODE_SUCCESSED;
    ADM_ECS_ECUCONFIG_T *pstEcuCfg = NULL;

    if (ADM_IS_NULL(ecuReqId) || ADM_IS_NULL(ecuSwDid) || ADM_IS_NULL(saMask))
    {
        log_e("Invalid input param.");        
        return ADM_ECODE_INVALID_PARAM;
    }

    iRet = __adm_ecs_checkAndInitModule();
    if (ADM_IS_ERR(iRet))
    {
        log_e("Failed to check module info, err[%d]", iRet);
        return iRet;
    }

    (void)pthread_rwlock_rdlock(&gstEcsModuleInfo.stRWlock);
    for (i = 0; i< gstEcsModuleInfo.pstEcsConfig->stRsCfgInfo.stPartCfg.stEcuCfgList.num; i++)
    {
        pstEcuCfg = &gstEcsModuleInfo.pstEcsConfig->stRsCfgInfo.stPartCfg.stEcuCfgList.pstEcuCfg[i];
        if ((0 == adm_strcasecmp(ecuReqId, pstEcuCfg->ecuReqId)) &&
            (0 == adm_strcasecmp(ecuSwDid, pstEcuCfg->ecuSwDid)))
        {
            *saMask = adm_strdup(pstEcuCfg->saMask);
            break;
        }
    }

    //!< 未找到对应的零件信息
    if (i == gstEcsModuleInfo.pstEcsConfig->stRsCfgInfo.stPartCfg.stEcuCfgList.num)
    {
        (void)pthread_rwlock_unlock(&gstEcsModuleInfo.stRWlock);
        log_e("Ecu part[reqId(%s), swDId(%s)] no find.", ecuReqId, ecuSwDid);    
        return ADM_ECODE_OTACFG_ECU_NONEXIST;
    }
    (void)pthread_rwlock_unlock(&gstEcsModuleInfo.stRWlock);

    return ADM_ECODE_SUCCESSED;
}

/*!
 *******************************************************************************
 * 获取零件刷写流程类型.
 *
 * \note
 * 
 * \param[in]  pEcuReqId   零件诊断请求ID
 * \param[in]  pEcuReqId   零件升级对象软件版本did 
 * \param[out] pFlashProgressType     零件刷写流程类型
 * \retval     D32S
 *******************************************************************************
 */
D32S adm_ecs_getEcuFlashProgressType
(
    const DCHAR *pEcuReqId, 
    const DCHAR *pEcuSwDid, 
    D32S *pFlashProgressType
)
{
    D32S i = 0;
    D32S iRet = ADM_ECODE_SUCCESSED;
    ADM_ECS_ECUCONFIG_T *pstEcuCfg = NULL;

    if (ADM_IS_NULL(pEcuReqId) || ADM_IS_NULL(pEcuSwDid) || ADM_IS_NULL(pFlashProgressType))
    {
        log_e("Invalid input param.");        
        return ADM_ECODE_INVALID_PARAM;
    }

    iRet = __adm_ecs_checkAndInitModule();
    if (ADM_IS_ERR(iRet))
    {
        log_e("Failed to check module info, err[%d]", iRet);
        return iRet;
    }

    (void)pthread_rwlock_rdlock(&gstEcsModuleInfo.stRWlock);
    for (i = 0; i< gstEcsModuleInfo.pstEcsConfig->stRsCfgInfo.stPartCfg.stEcuCfgList.num; i++)
    {
        pstEcuCfg = &gstEcsModuleInfo.pstEcsConfig->stRsCfgInfo.stPartCfg.stEcuCfgList.pstEcuCfg[i];
        if ((0 == adm_strcasecmp(pEcuReqId, pstEcuCfg->ecuReqId)) &&
            (0 == adm_strcasecmp(pEcuSwDid, pstEcuCfg->ecuSwDid)))
        {
            *pFlashProgressType = pstEcuCfg->flashProgressType;
            break;
        }
    }

    //!< 未找到对应的零件信息
    if (i == gstEcsModuleInfo.pstEcsConfig->stRsCfgInfo.stPartCfg.stEcuCfgList.num)
    {
        (void)pthread_rwlock_unlock(&gstEcsModuleInfo.stRWlock);
        log_e("Ecu part[reqId(%s), swDId(%s)] can not find flashProgressType.", pEcuReqId, pEcuSwDid);    
        return ADM_ECODE_OTACFG_ECU_NONEXIST;
    }
    (void)pthread_rwlock_unlock(&gstEcsModuleInfo.stRWlock);

    return ADM_ECODE_SUCCESSED;
}

/*!
 *******************************************************************************
 * 获取零件安全算法掩码列表.
 *
 * \note
 * 
 * \param[in]  ecuReqId   零件诊断请求ID
 * \param[in]  ecuSwDid   零件升级对象软件版本did 
 * \param[OUT] saMask     零件安全算法掩码
 * \retval     D32S
 *******************************************************************************
 */
D32S adm_ecs_getEcuSaMaskList
(
    const DCHAR *ecuReqId, 
    const DCHAR *ecuSwDid, 
    ADM_ECS_SA_MASK_LIST_T **saMaskList
)
{
    D32S i = 0;
    D32S iRet = ADM_ECODE_SUCCESSED;
    ADM_ECS_ECUCONFIG_T *pstEcuCfg = NULL;
    ADM_ECS_SA_MASK_LIST_T* pSaMaskList = NULL;

    if (ADM_IS_NULL(ecuReqId) || ADM_IS_NULL(ecuSwDid) || ADM_IS_NULL(saMaskList) || ADM_IS_NULL(*saMaskList))
    {
        log_e("Invalid input param.");        
        return ADM_ECODE_INVALID_PARAM;
    }

    iRet = __adm_ecs_checkAndInitModule();
    if (ADM_IS_ERR(iRet))
    {
        log_e("Failed to check module info, err[%d]", iRet);
        return iRet;
    }

    pSaMaskList = *saMaskList;

    (void)pthread_rwlock_rdlock(&gstEcsModuleInfo.stRWlock);
    for (i = 0; i< gstEcsModuleInfo.pstEcsConfig->stRsCfgInfo.stPartCfg.stEcuCfgList.num; i++)
    {
        pstEcuCfg = &gstEcsModuleInfo.pstEcsConfig->stRsCfgInfo.stPartCfg.stEcuCfgList.pstEcuCfg[i];
        if ((0 == adm_strcasecmp(ecuReqId, pstEcuCfg->ecuReqId)) &&
            (0 == adm_strcasecmp(ecuSwDid, pstEcuCfg->ecuSwDid)))
        {
            if (NULL != pstEcuCfg->saMaskList && 0 < pstEcuCfg->saMaskList->num && 
                NULL != pstEcuCfg->saMaskList->saMaskInfo)
            {
                pSaMaskList->num = pstEcuCfg->saMaskList->num;
                pSaMaskList->saMaskInfo = (ADM_ECS_SA_MASK_T*)adm_malloc(pstEcuCfg->saMaskList->num * sizeof(ADM_ECS_SA_MASK_T));

                for (D16U j = 0; j < pstEcuCfg->saMaskList->num; j++)
                {
                    pSaMaskList->saMaskInfo[j].saMaskLevel = pstEcuCfg->saMaskList->saMaskInfo[j].saMaskLevel;
                    pSaMaskList->saMaskInfo[j].saMaskValue = adm_strdup(pstEcuCfg->saMaskList->saMaskInfo[j].saMaskValue);
                }
            }
            else 
            {
                log_w("Has no mask List");
                // adm_free(*saMaskList);
                (void)pthread_rwlock_unlock(&gstEcsModuleInfo.stRWlock);
                return ADM_ECODE_OTACFG_ECU_NONEXIST;
            }
            break;
        }
    }

    //!< 未找到对应的零件信息
    if (i == gstEcsModuleInfo.pstEcsConfig->stRsCfgInfo.stPartCfg.stEcuCfgList.num)
    {
        (void)pthread_rwlock_unlock(&gstEcsModuleInfo.stRWlock);
        log_e("Ecu part[reqId(%s), swDId(%s)] no find.", ecuReqId, ecuSwDid);    
        return ADM_ECODE_OTACFG_ECU_NONEXIST;
    }
    (void)pthread_rwlock_unlock(&gstEcsModuleInfo.stRWlock);

    return ADM_ECODE_SUCCESSED;
}

/*!
 *******************************************************************************
 * 获取零件安全算法库编号列表信息.
 *
 * \note
 * 
 * \param[in]  ecuReqId    零件诊断请求ID
 * \param[in]  ecuSwDid    零件升级对象软件版本did 
 * \param[OUT] pstsaLibNumberList 零件安全算法库编号列表
 * \retval     D32S
 *******************************************************************************
 */
D32S adm_ecs_getEcuSaLibNumberList
(
    DCHAR *ecuReqId, 
    DCHAR *ecuSwDid, 
    ADM_ECS_SALIBNUMBERLIST_T *pstsaLibNumberList
)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    D32S i = 0;
    ADM_ECS_ECUCONFIG_T *pstEcuCfg = NULL;

    if (ADM_IS_NULL(ecuReqId) || ADM_IS_NULL(ecuSwDid) || ADM_IS_NULL(pstsaLibNumberList))
    {
        log_e("Invalid input param.");        
        return ADM_ECODE_INVALID_PARAM;
    }

    iRet = __adm_ecs_checkAndInitModule();
    if (ADM_IS_ERR(iRet))
    {
        log_e("Failed to check module info, err[%d]", iRet);
        return iRet;
    }

    (void)pthread_rwlock_rdlock(&gstEcsModuleInfo.stRWlock);
    for (i = 0; i< gstEcsModuleInfo.pstEcsConfig->stRsCfgInfo.stPartCfg.stEcuCfgList.num; i++)
    {
        pstEcuCfg = &gstEcsModuleInfo.pstEcsConfig->stRsCfgInfo.stPartCfg.stEcuCfgList.pstEcuCfg[i];
        if ((0 == adm_strcasecmp(ecuReqId, pstEcuCfg->ecuReqId)) &&
            (0 == adm_strcasecmp(ecuSwDid, pstEcuCfg->ecuSwDid)))
        {
            iRet = __adm_ecs_fillEcuSaLibNumberList(pstsaLibNumberList, 
                        &pstEcuCfg->saLibNumberList);
            break;
        }
    }

    //!< 未找到对应的零件信息
    if (i == gstEcsModuleInfo.pstEcsConfig->stRsCfgInfo.stPartCfg.stEcuCfgList.num)
    {
        (void)pthread_rwlock_unlock(&gstEcsModuleInfo.stRWlock);
        log_e("Ecu part[reqId(%s), swDId(%s)] no find.", ecuReqId, ecuSwDid);    
        return ADM_ECODE_OTACFG_ECU_NONEXIST;
    }
    (void)pthread_rwlock_unlock(&gstEcsModuleInfo.stRWlock);

    if (ADM_IS_ERR(iRet))
    {
        log_e("Failed to copy saLibNumberList info, err[%d]", iRet);
        adm_ecs_delSaLibNumberList(pstsaLibNumberList->num, &pstsaLibNumberList->psaLibNumber);
        pstsaLibNumberList->num = 0;
    }

    return iRet;
}


/*!
 *******************************************************************************
 * 获取零件刷写脚本编号.
 *
 * \note
 * 
 * \param[in]  ecuReqId          零件诊断请求ID
 * \param[in]  ecuSwDid          零件升级对象软件版本did 
 * \param[OUT] flashScriptNumber 零件刷写脚本编号
 * \retval     D32S
 *******************************************************************************
 */
D32S adm_ecs_getEcuFlashScriptNumber
(
    DCHAR *ecuReqId, 
    DCHAR *ecuSwDid, 
    D32S  *flashScriptNumber
)
{
    D32S i = 0;
    D32S iRet = ADM_ECODE_SUCCESSED;
    ADM_ECS_ECUCONFIG_T *pstEcuCfg = NULL;

    if (ADM_IS_NULL(ecuReqId) || ADM_IS_NULL(ecuSwDid) || ADM_IS_NULL(flashScriptNumber))
    {
        log_e("Invalid input param.");        
        return ADM_ECODE_INVALID_PARAM;
    }

    iRet = __adm_ecs_checkAndInitModule();
    if (ADM_IS_ERR(iRet))
    {
        log_e("Failed to check module info, err[%d]", iRet);
        return iRet;
    }

    (void)pthread_rwlock_rdlock(&gstEcsModuleInfo.stRWlock);
    for (i = 0; i< gstEcsModuleInfo.pstEcsConfig->stRsCfgInfo.stPartCfg.stEcuCfgList.num; i++)
    {
        pstEcuCfg = &gstEcsModuleInfo.pstEcsConfig->stRsCfgInfo.stPartCfg.stEcuCfgList.pstEcuCfg[i];
        if ((0 == adm_strcasecmp(ecuReqId, pstEcuCfg->ecuReqId)) &&
            (0 == adm_strcasecmp(ecuSwDid, pstEcuCfg->ecuSwDid)))
        {
            *flashScriptNumber = pstEcuCfg->flashScriptNumber;
            break;
        }
    }

    //!< 未找到对应的零件信息
    if (i == gstEcsModuleInfo.pstEcsConfig->stRsCfgInfo.stPartCfg.stEcuCfgList.num)
    {
        (void)pthread_rwlock_unlock(&gstEcsModuleInfo.stRWlock);
        log_e("Ecu part[reqId(%s), swDId(%s)] no find.", ecuReqId, ecuSwDid);    
        return ADM_ECODE_OTACFG_ECU_NONEXIST;
    }
    (void)pthread_rwlock_unlock(&gstEcsModuleInfo.stRWlock);

    return ADM_ECODE_SUCCESSED;
}


/*!
 *******************************************************************************
 * 获取零件是否经网关路由.
 *
 * \note      1:Y, 2:N, 3:N/A 
 * 
 * \param[in]  ecuReqId    零件诊断请求ID
 * \param[in]  ecuSwDid    零件升级对象软件版本did 
 * \param[OUT] isCGWRouted 零件是否经网关路由
 * \retval     D32S
 *******************************************************************************
 */
D32S adm_ecs_getEcuIsCGWRouted
(
    DCHAR *ecuReqId, 
    DCHAR *ecuSwDid, 
    D32S  *isCGWRouted
)
{
    D32S i = 0;
    D32S iRet = ADM_ECODE_SUCCESSED;
    ADM_ECS_ECUCONFIG_T *pstEcuCfg = NULL;

    if (ADM_IS_NULL(ecuReqId) || ADM_IS_NULL(ecuSwDid) || ADM_IS_NULL(isCGWRouted))
    {
        log_e("Invalid input param.");        
        return ADM_ECODE_INVALID_PARAM;
    }

    iRet = __adm_ecs_checkAndInitModule();
    if (ADM_IS_ERR(iRet))
    {
        log_e("Failed to check module info, err[%d]", iRet);
        return iRet;
    }

    (void)pthread_rwlock_rdlock(&gstEcsModuleInfo.stRWlock);
    for (i = 0; i< gstEcsModuleInfo.pstEcsConfig->stRsCfgInfo.stPartCfg.stEcuCfgList.num; i++)
    {
        pstEcuCfg = &gstEcsModuleInfo.pstEcsConfig->stRsCfgInfo.stPartCfg.stEcuCfgList.pstEcuCfg[i];
        if ((0 == adm_strcasecmp(ecuReqId, pstEcuCfg->ecuReqId)) &&
            (0 == adm_strcasecmp(ecuSwDid, pstEcuCfg->ecuSwDid)))
        {
            *isCGWRouted = pstEcuCfg->isCGWRouted;
            break;
        }
    }

    //!< 未找到对应的零件信息
    if (i == gstEcsModuleInfo.pstEcsConfig->stRsCfgInfo.stPartCfg.stEcuCfgList.num)
    {
        (void)pthread_rwlock_unlock(&gstEcsModuleInfo.stRWlock);
        log_e("Ecu part[reqId(%s), swDId(%s)] no find.", ecuReqId, ecuSwDid);       
        return ADM_ECODE_OTACFG_ECU_NONEXIST;
    }
    (void)pthread_rwlock_unlock(&gstEcsModuleInfo.stRWlock);

    return ADM_ECODE_SUCCESSED;
}


/*!
 *******************************************************************************
 * 获取零件的刷写类型.
 *
 * \note   1:VDI, 2:UCM, 3:HOST
 * 
 * \param[in]  ecuReqId    零件诊断请求ID
 * \param[in]  ecuSwDid    零件升级对象软件版本did 
 * \param[OUT] serviceType 零件的刷写类型
 * \retval     D32S
 *******************************************************************************
 */
D32S adm_ecs_getEcuServiceType
(
    DCHAR *ecuReqId, 
    DCHAR *ecuSwDid, 
    D32S  *serviceType
)
{
    D32S i = 0;
    D32S iRet = ADM_ECODE_SUCCESSED;
    ADM_ECS_ECUCONFIG_T *pstEcuCfg = NULL;

    if (ADM_IS_NULL(ecuReqId) || ADM_IS_NULL(ecuSwDid) || ADM_IS_NULL(serviceType))
    {
        log_e("Invalid input param.");        
        return ADM_ECODE_INVALID_PARAM;
    }

    iRet = __adm_ecs_checkAndInitModule();
    if (ADM_IS_ERR(iRet))
    {
        log_e("Failed to check module info, err[%d]", iRet);
        return iRet;
    }

    (void)pthread_rwlock_rdlock(&gstEcsModuleInfo.stRWlock);
    for (i = 0; i< gstEcsModuleInfo.pstEcsConfig->stRsCfgInfo.stPartCfg.stEcuCfgList.num; i++)
    {
        pstEcuCfg = &gstEcsModuleInfo.pstEcsConfig->stRsCfgInfo.stPartCfg.stEcuCfgList.pstEcuCfg[i];
        if ((0 == adm_strcasecmp(ecuReqId, pstEcuCfg->ecuReqId)) &&
            (0 == adm_strcasecmp(ecuSwDid, pstEcuCfg->ecuSwDid)))
        {
            *serviceType = pstEcuCfg->serviceType;
            break;
        }
    }

    //!< 未找到对应的零件信息
    if (i == gstEcsModuleInfo.pstEcsConfig->stRsCfgInfo.stPartCfg.stEcuCfgList.num)
    {
        (void)pthread_rwlock_unlock(&gstEcsModuleInfo.stRWlock);
        log_e("Ecu part[reqId(%s), swDId(%s)] no find.", ecuReqId, ecuSwDid);      
        return ADM_ECODE_OTACFG_ECU_NONEXIST;
    }
    (void)pthread_rwlock_unlock(&gstEcsModuleInfo.stRWlock);

    return ADM_ECODE_SUCCESSED;
}


/*!
 *******************************************************************************
 * 获取零件安装约束.
 *
 * \note   零件需要处于低压/高压条件下才可以刷写(1:high, 2:low)
 * 
 * \param[in]  ecuReqId       零件诊断请求ID
 * \param[in]  ecuSwDid       零件升级对象软件版本did 
 * \param[OUT] instCondition  零件安装约束
 * \retval     D32S
 *******************************************************************************
 */
D32S adm_ecs_getEcuInstCondition
(
    DCHAR *ecuReqId, 
    DCHAR *ecuSwDid, 
    D32S  *instCondition
)
{
    D32S i = 0;
    D32S iRet = ADM_ECODE_SUCCESSED;
    ADM_ECS_ECUCONFIG_T *pstEcuCfg = NULL;

    if (ADM_IS_NULL(ecuReqId) || ADM_IS_NULL(ecuSwDid) || ADM_IS_NULL(instCondition))
    {
        log_e("Invalid input param.");        
        return ADM_ECODE_INVALID_PARAM;
    }

    iRet = __adm_ecs_checkAndInitModule();
    if (ADM_IS_ERR(iRet))
    {
        log_e("Failed to check module info, err[%d]", iRet);
        return iRet;
    }

    (void)pthread_rwlock_rdlock(&gstEcsModuleInfo.stRWlock);
    for (i = 0; i< gstEcsModuleInfo.pstEcsConfig->stRsCfgInfo.stPartCfg.stEcuCfgList.num; i++)
    {
        pstEcuCfg = &gstEcsModuleInfo.pstEcsConfig->stRsCfgInfo.stPartCfg.stEcuCfgList.pstEcuCfg[i];
        if ((0 == adm_strcasecmp(ecuReqId, pstEcuCfg->ecuReqId)) &&
            (0 == adm_strcasecmp(ecuSwDid, pstEcuCfg->ecuSwDid)))
        {
            *instCondition = pstEcuCfg->instCondition;
            break;
        }
    }

    //!< 未找到对应的零件信息
    if (i == gstEcsModuleInfo.pstEcsConfig->stRsCfgInfo.stPartCfg.stEcuCfgList.num)
    {
        (void)pthread_rwlock_unlock(&gstEcsModuleInfo.stRWlock);
        log_e("Ecu part[reqId(%s), swDId(%s)] no find.", ecuReqId, ecuSwDid); 
        return ADM_ECODE_OTACFG_ECU_NONEXIST;
    }
    (void)pthread_rwlock_unlock(&gstEcsModuleInfo.stRWlock);

    return ADM_ECODE_SUCCESSED;
}


/*!
 *******************************************************************************
 * 获取零件的安全等级.
 *
 * \note  1: 重要, 2: 一般
 * 
 * \param[in]  ecuReqId       零件诊断请求ID 
 * \param[in]  ecuSwDid       零件升级对象软件版本did 
 * \param[OUT] securityLevel  零件的安全等级
 * \retval     D32S
 *******************************************************************************
 */
D32S adm_ecs_getEcuSecurityLevel
(
    DCHAR *ecuReqId, 
    DCHAR *ecuSwDid, 
    D32S  *securityLevel
)
{
    D32S i = 0;
    D32S iRet = ADM_ECODE_SUCCESSED;
    ADM_ECS_ECUCONFIG_T *pstEcuCfg = NULL;

    if (ADM_IS_NULL(ecuReqId) || ADM_IS_NULL(ecuSwDid) || ADM_IS_NULL(securityLevel))
    {
        log_e("Invalid input param.");        
        return ADM_ECODE_INVALID_PARAM;
    }

    iRet = __adm_ecs_checkAndInitModule();
    if (ADM_IS_ERR(iRet))
    {
        log_e("Failed to check module info, err[%d]", iRet);
        return iRet;
    }

    (void)pthread_rwlock_rdlock(&gstEcsModuleInfo.stRWlock);
    for (i = 0; i< gstEcsModuleInfo.pstEcsConfig->stRsCfgInfo.stPartCfg.stEcuCfgList.num; i++)
    {
        pstEcuCfg = &gstEcsModuleInfo.pstEcsConfig->stRsCfgInfo.stPartCfg.stEcuCfgList.pstEcuCfg[i];
        if ((0 == adm_strcasecmp(ecuReqId, pstEcuCfg->ecuReqId)) &&
            (0 == adm_strcasecmp(ecuSwDid, pstEcuCfg->ecuSwDid)))
        {
            // TODO: del
            *securityLevel = pstEcuCfg->securityLevel;
            break;
        }
    }

    //!< 未找到对应的零件信息
    if (i == gstEcsModuleInfo.pstEcsConfig->stRsCfgInfo.stPartCfg.stEcuCfgList.num)
    {
        (void)pthread_rwlock_unlock(&gstEcsModuleInfo.stRWlock);
        log_e("Ecu part[reqId(%s), swDId(%s)] no find.", ecuReqId, ecuSwDid);        
        return ADM_ECODE_OTACFG_ECU_NONEXIST;
    }
    (void)pthread_rwlock_unlock(&gstEcsModuleInfo.stRWlock);

    return ADM_ECODE_SUCCESSED;
}

/*!
 *******************************************************************************
 * 获取车型配置中的do-ip server 信息.
 * @note FLZCU、TBOX、FCM、ADCC
 * 
 * \param[out] pNum do-ip server信息元素个数
 * \param[out] doIpServerInfo do-ip server信息
 * \retval     D32S
 *******************************************************************************
 */
D32S adm_ecs_getEcuDoIpServer
(
    D32U* pNum, 
    D32U* pDoIpServerInfo
)
{
    D32S i = 0;
    D32S j = 0;
    D32S iRet = ADM_ECODE_SUCCESSED;
    D32U uiNum = 0;
    DBOOL bIsExits = DFALSE;
    ADM_ECS_ECUCONFIG_T *pstEcuCfg = NULL;
    const DCHAR* pFlcuReqId = "0x0002";
    const DCHAR* pTboxReqId = "0x0001";
    const DCHAR* pFcmReqId = "0x0410";
    const DCHAR* pAdccReqId = "0x0400";

    if (ADM_IS_NULL(pNum) || ADM_IS_NULL(pDoIpServerInfo))
    {
        log_e("Invalid input param.");        
        return ADM_ECODE_INVALID_PARAM;
    }

    iRet = __adm_ecs_checkAndInitModule();
    if (ADM_IS_ERR(iRet))
    {
        log_e("Failed to check module info, err[%d]", iRet);
        return iRet;
    }

    if (NULL == gstEcsModuleInfo.pstEcsConfig || 
        0 >= gstEcsModuleInfo.pstEcsConfig->stRsCfgInfo.stPartCfg.stEcuCfgList.num)
    {
        log_i("Vehicle config info get failed.");
        return iRet;
    }

    (void)pthread_rwlock_rdlock(&gstEcsModuleInfo.stRWlock);
    for (i = 0; i< gstEcsModuleInfo.pstEcsConfig->stRsCfgInfo.stPartCfg.stEcuCfgList.num; i++)
    {
        pstEcuCfg = &gstEcsModuleInfo.pstEcsConfig->stRsCfgInfo.stPartCfg.stEcuCfgList.pstEcuCfg[i];
        if ((0 == adm_strcasecmp(pFlcuReqId, pstEcuCfg->ecuReqId)) || 
            (0 == adm_strcasecmp(pTboxReqId, pstEcuCfg->ecuReqId)) || 
            (0 == adm_strcasecmp(pFcmReqId, pstEcuCfg->ecuReqId)) || 
            (0 == adm_strcasecmp(pAdccReqId, pstEcuCfg->ecuReqId)))
        {
            for (j = 0; j < uiNum; j++)
            {
                /** 去重 */
                if ((D32U)htoi(pstEcuCfg->ecuReqId) == pDoIpServerInfo[j])
                {
                    bIsExits = DTRUE;
                    break;
                }
            }
            if (DFALSE == bIsExits)
            {
                pDoIpServerInfo[uiNum] = (D32U)htoi(pstEcuCfg->ecuReqId);
                uiNum++;
            }
            bIsExits = DFALSE;
        }
    }

    *pNum = uiNum;
    
    (void)pthread_rwlock_unlock(&gstEcsModuleInfo.stRWlock);

    return ADM_ECODE_SUCCESSED;
}


/*!
 *******************************************************************************
 * 获取ecu配置信息.
 *
 * \note
 * 
 * \param[in]  ecuReqId      零件诊断请求ID
 * \param[in]  ecuSwDid      零件升级对象软件版本did 
 * \param[OUT] ppstEcuConfig 零件配置信息
 * \retval     D32S
 *******************************************************************************
 */
D32S adm_ecs_getEcuConfig
(
    DCHAR *ecuReqId, 
    DCHAR *ecuSwDid, 
    ADM_ECS_ECUCONFIG_T **ppstEcuConfig
)
{
    D32S i = 0;
    D32S iRet = ADM_ECODE_SUCCESSED;
    ADM_ECS_ECUCONFIG_T *pstEcuCfg = NULL;

    if (ADM_IS_NULL(ecuReqId) || ADM_IS_NULL(ecuSwDid) || ADM_IS_NULL(ppstEcuConfig))
    {
        log_e("Invalid input param.");        
        return ADM_ECODE_INVALID_PARAM;
    }

    iRet = __adm_ecs_checkAndInitModule();
    if (ADM_IS_ERR(iRet))
    {
        log_e("Failed to check module info, err[%d]", iRet);
        return iRet;
    }

    (void)pthread_rwlock_rdlock(&gstEcsModuleInfo.stRWlock);
    for (i = 0; i< gstEcsModuleInfo.pstEcsConfig->stRsCfgInfo.stPartCfg.stEcuCfgList.num; i++)
    {
        pstEcuCfg = &gstEcsModuleInfo.pstEcsConfig->stRsCfgInfo.stPartCfg.stEcuCfgList.pstEcuCfg[i];
        if ((0 == adm_strcasecmp(ecuReqId, pstEcuCfg->ecuReqId)) &&
            (0 == adm_strcasecmp(ecuSwDid, pstEcuCfg->ecuSwDid)))
        {
            break;
        }
    }

    //!< 未找到对应的零件信息
    if (i == gstEcsModuleInfo.pstEcsConfig->stRsCfgInfo.stPartCfg.stEcuCfgList.num)
    {
        (void)pthread_rwlock_unlock(&gstEcsModuleInfo.stRWlock);
        log_e("Ecu part[reqId(%s), swDId(%s)] no find.", ecuReqId, ecuSwDid);   
        return ADM_ECODE_OTACFG_ECU_NONEXIST;
    }

    *ppstEcuConfig = adm_ecs_newEcuConfigInfo();
    if (ADM_IS_NULL(*ppstEcuConfig))
    {
        (void)pthread_rwlock_unlock(&gstEcsModuleInfo.stRWlock);
        log_e("Failed to new ecu config info, err[%d]", ADM_ECODE_NO_MEMORY);
        return ADM_ECODE_NO_MEMORY;
    }

    //!< copy exts
    iRet = adm_com_copyExtsObject(&(*ppstEcuConfig)->stExtList, &pstEcuCfg->stExtList);
    if (ADM_IS_ERR(iRet))
    {
        (void)pthread_rwlock_unlock(&gstEcsModuleInfo.stRWlock);
        adm_ecs_delEcuConfigInfo(ppstEcuConfig);
        log_e("Failed to copy exts info, err[%d]", iRet);
        return iRet;
    }

    //!< saMaskList
    (*ppstEcuConfig)->saMaskList = (ADM_ECS_SA_MASK_LIST_T*)adm_malloc(sizeof(ADM_ECS_SA_MASK_LIST_T));
    iRet = __adm_ecs_fillEcuSaMaskList((*ppstEcuConfig)->saMaskList, 
                    pstEcuCfg->saMaskList);
    if (ADM_IS_ERR(iRet))
    {
        (void)pthread_rwlock_unlock(&gstEcsModuleInfo.stRWlock);
        adm_ecs_delEcuConfigInfo(ppstEcuConfig);
        log_e("Failed to fill saMask list, err[%d]", iRet);
        return iRet;
    }

    //!< SaLibNumberList
    iRet = __adm_ecs_fillEcuSaLibNumberList(&(*ppstEcuConfig)->saLibNumberList, 
                    &pstEcuCfg->saLibNumberList);
    if (ADM_IS_ERR(iRet))
    {
        (void)pthread_rwlock_unlock(&gstEcsModuleInfo.stRWlock);
        adm_ecs_delEcuConfigInfo(ppstEcuConfig);
        log_e("Failed to fill saLibNumber list, err[%d]", iRet);
        return iRet;
    }

    //!< CrcLibNumberList
    iRet = __adm_ecs_fillEcuCrcLibNumberList(&(*ppstEcuConfig)->crcLibNumberList, 
                    &pstEcuCfg->crcLibNumberList);
    if (ADM_IS_ERR(iRet))
    {
        (void)pthread_rwlock_unlock(&gstEcsModuleInfo.stRWlock);
        adm_ecs_delEcuConfigInfo(ppstEcuConfig);
        log_e("Failed to fill crcLibNumber list, err[%d]", iRet);
        return iRet;
    }

    //!< EcuDidList
    iRet = __adm_ecs_fillEcuDidList(&(*ppstEcuConfig)->stEcuDidList, 
                    &pstEcuCfg->stEcuDidList);
    if (ADM_IS_ERR(iRet))
    {
        (void)pthread_rwlock_unlock(&gstEcsModuleInfo.stRWlock);
        adm_ecs_delEcuConfigInfo(ppstEcuConfig);
        log_e("Failed to fill resource did list, err[%d]", iRet);
        return iRet;
    }

    (*ppstEcuConfig)->ecuReqId = 
        adm_strdup(pstEcuCfg->ecuReqId);
    (*ppstEcuConfig)->ecuSwDid = 
        adm_strdup(pstEcuCfg->ecuSwDid);
    (*ppstEcuConfig)->ecuResponseId = 
        adm_strdup(pstEcuCfg->ecuResponseId);
    (*ppstEcuConfig)->ecuFunctionId = 
        adm_strdup(pstEcuCfg->ecuFunctionId);
    (*ppstEcuConfig)->readInfoInstanceId = 
        adm_strdup(pstEcuCfg->readInfoInstanceId);
    (*ppstEcuConfig)->downloadInstanceId = 
        adm_strdup(pstEcuCfg->downloadInstanceId);
    (*ppstEcuConfig)->transferInstanceId = 
        adm_strdup(pstEcuCfg->transferInstanceId);
    (*ppstEcuConfig)->installInstanceId = 
        adm_strdup(pstEcuCfg->installInstanceId);

    (*ppstEcuConfig)->selfBackup   = pstEcuCfg->selfBackup;
    (*ppstEcuConfig)->isSelfUpdate = pstEcuCfg->isSelfUpdate;
    (*ppstEcuConfig)->busType      = pstEcuCfg->busType;
    (*ppstEcuConfig)->channelType  = pstEcuCfg->channelType;
    (*ppstEcuConfig)->crcMask      = pstEcuCfg->crcMask;
    (*ppstEcuConfig)->crcLibNumber = pstEcuCfg->crcLibNumber;
    (*ppstEcuConfig)->saMask       = adm_strdup(pstEcuCfg->saMask);
    (*ppstEcuConfig)->flashProgressType = pstEcuCfg->flashProgressType;
    (*ppstEcuConfig)->flashScriptNumber = pstEcuCfg->flashScriptNumber;
    (*ppstEcuConfig)->isCGWRouted   = pstEcuCfg->isCGWRouted;
    (*ppstEcuConfig)->serviceType   = pstEcuCfg->serviceType;
    (*ppstEcuConfig)->instCondition = pstEcuCfg->instCondition;
    (*ppstEcuConfig)->securityLevel = pstEcuCfg->securityLevel;

    (void)pthread_rwlock_unlock(&gstEcsModuleInfo.stRWlock);

    return ADM_ECODE_SUCCESSED;
}


/*!
 *******************************************************************************
 * 获取通用配置信息中ecuDids信息.
 *
 * \note
 * 
 * \param[out]  pstRsCfgInfo    OTA平台配置信息
 * \retval      D32S
 *******************************************************************************
 */
D32S adm_ecs_getGeneralEcuDidListInfo(ADM_ECS_ECUDIDLIST_T *pstEcuDidList)
{
    D32S iRet = ADM_ECODE_SUCCESSED;

    if (ADM_IS_NULL(pstEcuDidList))
    {
        log_e("Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }
    
    iRet = __adm_ecs_checkAndInitModule();
    if (ADM_IS_ERR(iRet))
    {
        log_e("Failed to check module info, err[%d]", iRet);
        return iRet;
    }

    (void)pthread_rwlock_rdlock(&gstEcsModuleInfo.stRWlock);
    iRet = __adm_ecs_fillEcuDidList(pstEcuDidList, &gstEcsModuleInfo.pstEcsConfig->stRsCfgInfo.stGeneralCfg.stEcuDidList);
    if (ADM_IS_ERR(iRet))
    {   
        (void)pthread_rwlock_unlock(&gstEcsModuleInfo.stRWlock);
        log_e("Failed to fill resource ecu did list, err[%d]", iRet);
        return iRet;
    }
    (void)pthread_rwlock_unlock(&gstEcsModuleInfo.stRWlock);

    return iRet;
}


/*!
 *******************************************************************************
 * 获取配置信息中相应ecu的ecuDids信息（通用配置+零件配置中的dids）.
 *
 * \note 
 * 
 * \param[in]   ecuReqId      零件诊断请求ID
 * \param[in]   ecuSwDid      零件升级对象软件版本did 
 * \param[out]  pstEcuDidList 零件对应的ecudidlist
 * \retval      D32S
 *******************************************************************************
 */
D32S adm_ecs_getOneEcuDidListInfo
(
    DCHAR *ecuReqId, 
    DCHAR *ecuSwDid, 
    ADM_ECS_ECUDIDLIST_T *pstEcuDidList,
    ADM_ECS_EXTERN_ECU_INFO_T* pstExternEcuInfo
)
{
    D32S i = 0;
    D32S j = 0;
    D32S s32Index = 0;
    D32S iRet = ADM_ECODE_SUCCESSED;
    ADM_ECS_ECUDIDLIST_T *pstTmpEcuDidList = NULL;
    ADM_ECS_EXTERN_ECU_INFO_T* pstTmpExternEcuInfo = NULL;

    if (ADM_IS_NULL(ecuReqId) || ADM_IS_NULL(ecuSwDid) || ADM_IS_NULL(pstEcuDidList) || ADM_IS_NULL(pstExternEcuInfo))
    {
        log_e("Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }
    
    iRet = __adm_ecs_checkAndInitModule();
    if (ADM_IS_ERR(iRet))
    {
        log_e("Failed to check module info, err[%d]", iRet);
        return iRet;
    }

    (void)pthread_rwlock_rdlock(&gstEcsModuleInfo.stRWlock);

    //!< 计算ecudid的个数（通用配置中的ecuDidList + 对应零件配置中的ecuDidList）
    pstEcuDidList->num = gstEcsModuleInfo.pstEcsConfig->stRsCfgInfo.stGeneralCfg.stEcuDidList.num;
    for (i = 0; i < gstEcsModuleInfo.pstEcsConfig->stRsCfgInfo.stPartCfg.stEcuCfgList.num; i++)
    {
        if ((0 == adm_strcasecmp(ecuReqId, gstEcsModuleInfo.pstEcsConfig->stRsCfgInfo.stPartCfg.stEcuCfgList.pstEcuCfg[i].ecuReqId)) &&
            (0 == adm_strcasecmp(ecuSwDid, gstEcsModuleInfo.pstEcsConfig->stRsCfgInfo.stPartCfg.stEcuCfgList.pstEcuCfg[i].ecuSwDid)))
        {
            pstEcuDidList->num += gstEcsModuleInfo.pstEcsConfig->stRsCfgInfo.stPartCfg.stEcuCfgList.pstEcuCfg[i].stEcuDidList.num;
            break;
        }
    }

    pstEcuDidList->pstEcuDidInfo = adm_ecs_newEcuDidList(pstEcuDidList->num);
    if ((0 < pstEcuDidList->num) && ADM_IS_NULL(pstEcuDidList->pstEcuDidInfo))
    {
        (void)pthread_rwlock_unlock(&gstEcsModuleInfo.stRWlock);
        pstEcuDidList->num = 0;
        log_e("Failed to new ResDidIlist, err[%d]", ADM_ECODE_NO_MEMORY);
        return ADM_ECODE_NO_MEMORY;
    }

    //!< 填充通用配置中的dids
    pstTmpEcuDidList = &gstEcsModuleInfo.pstEcsConfig->stRsCfgInfo.stGeneralCfg.stEcuDidList;
    for (i = 0; i < pstTmpEcuDidList->num; i++)
    {
        pstEcuDidList->pstEcuDidInfo[s32Index].did   = adm_strdup(pstTmpEcuDidList->pstEcuDidInfo[i].did);
        pstEcuDidList->pstEcuDidInfo[s32Index].end   = pstTmpEcuDidList->pstEcuDidInfo[i].end;
        pstEcuDidList->pstEcuDidInfo[s32Index].start = pstTmpEcuDidList->pstEcuDidInfo[i].start;
        pstEcuDidList->pstEcuDidInfo[s32Index].didValType = pstTmpEcuDidList->pstEcuDidInfo[i].didValType;
        ++s32Index;
    }

    //!< 填充对应零件配置中的dids
    for (i = 0; i < gstEcsModuleInfo.pstEcsConfig->stRsCfgInfo.stPartCfg.stEcuCfgList.num; i++)
    {
        if ((0 == adm_strcasecmp(ecuReqId, gstEcsModuleInfo.pstEcsConfig->stRsCfgInfo.stPartCfg.stEcuCfgList.pstEcuCfg[i].ecuReqId)) &&
            (0 == adm_strcasecmp(ecuSwDid, gstEcsModuleInfo.pstEcsConfig->stRsCfgInfo.stPartCfg.stEcuCfgList.pstEcuCfg[i].ecuSwDid)))
        {
            pstTmpEcuDidList = &gstEcsModuleInfo.pstEcsConfig->stRsCfgInfo.stPartCfg.stEcuCfgList.pstEcuCfg[i].stEcuDidList;
            pstTmpExternEcuInfo = &gstEcsModuleInfo.pstEcsConfig->stRsCfgInfo.stPartCfg.stEcuCfgList.pstEcuCfg[i].stExternEcuInfo;

            pstExternEcuInfo->partHardwareNum = adm_strdup(pstTmpExternEcuInfo->partHardwareNum);
            pstExternEcuInfo->partName = adm_strdup(pstTmpExternEcuInfo->partName);
            pstExternEcuInfo->partNumber = adm_strdup(pstTmpExternEcuInfo->partNumber);
            pstExternEcuInfo->partSupplier = adm_strdup(pstTmpExternEcuInfo->partSupplier);
            pstExternEcuInfo->partSwid = adm_strdup(pstTmpExternEcuInfo->partSwid);
            pstExternEcuInfo->softwarePartNumber = adm_strdup(pstTmpExternEcuInfo->softwarePartNumber);
            pstExternEcuInfo->upgradeOrder = adm_strdup(pstTmpExternEcuInfo->upgradeOrder);

            for (j = 0; j < pstTmpEcuDidList->num; j++)
            {
                pstEcuDidList->pstEcuDidInfo[s32Index].did   = adm_strdup(pstTmpEcuDidList->pstEcuDidInfo[j].did);
                pstEcuDidList->pstEcuDidInfo[s32Index].end   = pstTmpEcuDidList->pstEcuDidInfo[j].end;
                pstEcuDidList->pstEcuDidInfo[s32Index].start = pstTmpEcuDidList->pstEcuDidInfo[j].start;
                pstEcuDidList->pstEcuDidInfo[s32Index].didValType = pstTmpEcuDidList->pstEcuDidInfo[j].didValType;
                ++s32Index;
            }
            break;
        } 
    }
    (void)pthread_rwlock_unlock(&gstEcsModuleInfo.stRWlock);

    return iRet;
}


/*!
 *******************************************************************************
 * 获取通用配置信息中Exts信息.
 *
 * \note
 * 
 * \param[out]  pstExtsList    通用配置信息中exts信息
 * \retval      D32S
 *******************************************************************************
 */
D32S adm_ecs_getGeneralEcuExtsInfo(ADM_COM_EXTPARAMLIST_T *pstExtsList)
{
    D32S iRet = ADM_ECODE_SUCCESSED;

    if (ADM_IS_NULL(pstExtsList))
    {
        log_e("Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }
    
    iRet = __adm_ecs_checkAndInitModule();
    if (ADM_IS_ERR(iRet))
    {
        log_e("Failed to check module info, err[%d]", iRet);
        return iRet;
    }

    (void)pthread_rwlock_rdlock(&gstEcsModuleInfo.stRWlock);    
    //!< copy exts
    iRet = adm_com_copyExtsObject(pstExtsList, &gstEcsModuleInfo.pstEcsConfig->stRsCfgInfo.stGeneralCfg.stExtList);
    if (ADM_IS_ERR(iRet))
    {
        (void)pthread_rwlock_unlock(&gstEcsModuleInfo.stRWlock);
        log_e("Failed to copy exts info, err[%d]", iRet);
        return iRet;
    }
    (void)pthread_rwlock_unlock(&gstEcsModuleInfo.stRWlock);

    return iRet;
}

/*!
 *******************************************************************************
 * 获取零件配置信息中外层Exts信息.
 *
 * \note
 * 
 * \param[out]  pstExtsList  零件配置信息中外层exts信息
 * \retval      D32S
 *******************************************************************************
 */
D32S adm_ecs_getPartCfgExternalExtsInfo(ADM_COM_EXTPARAMLIST_T *pstExtsList)
{
    D32S iRet = ADM_ECODE_SUCCESSED;

    if (ADM_IS_NULL(pstExtsList))
    {
        log_e("Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }
    
    iRet = __adm_ecs_checkAndInitModule();
    if (ADM_IS_ERR(iRet))
    {
        log_e("Failed to check module info, err[%d]", iRet);
        return iRet;
    }

    (void)pthread_rwlock_rdlock(&gstEcsModuleInfo.stRWlock);    
    //!< copy exts
    iRet = adm_com_copyExtsObject(pstExtsList, &gstEcsModuleInfo.pstEcsConfig->stRsCfgInfo.stPartCfg.stExtList);
    if (ADM_IS_ERR(iRet))
    {
        (void)pthread_rwlock_unlock(&gstEcsModuleInfo.stRWlock);
        log_e("Failed to copy exts info, err[%d]", iRet);
        return iRet;
    }
    (void)pthread_rwlock_unlock(&gstEcsModuleInfo.stRWlock);

    return iRet;
}

/*!
 *******************************************************************************
 * 获取零件配置信息中内层Exts信息.
 *
 * \note
 * 
 * \param[out]  pstExtsList 零件配置信息中内层exts信息
 * \retval      D32S
 *******************************************************************************
 */
D32S adm_ecs_getPartCfgInternalExtsInfo
(
    DCHAR *ecuReqId, 
    DCHAR *ecuSwDid, 
    ADM_COM_EXTPARAMLIST_T *pstExtsList
)
{
    D32S i = 0;
    D32S iRet = ADM_ECODE_SUCCESSED;
    ADM_ECS_ECUCONFIG_T *pstEcuCfg = NULL;

    if (ADM_IS_NULL(ecuReqId) || ADM_IS_NULL(ecuSwDid) || ADM_IS_NULL(pstExtsList))
    {
        log_e("Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }
    
    iRet = __adm_ecs_checkAndInitModule();
    if (ADM_IS_ERR(iRet))
    {
        log_e("Failed to check module info, err[%d]", iRet);
        return iRet;
    }

    (void)pthread_rwlock_rdlock(&gstEcsModuleInfo.stRWlock);    
    for (i = 0; i< gstEcsModuleInfo.pstEcsConfig->stRsCfgInfo.stPartCfg.stEcuCfgList.num; i++)
    {
        pstEcuCfg = &gstEcsModuleInfo.pstEcsConfig->stRsCfgInfo.stPartCfg.stEcuCfgList.pstEcuCfg[i];
        if ((0 == adm_strcasecmp(ecuReqId, pstEcuCfg->ecuReqId)) &&
            (0 == adm_strcasecmp(ecuSwDid, pstEcuCfg->ecuSwDid)))
        {
            break;
        }
    }

    //!< 未找到对应的零件信息
    if (i == gstEcsModuleInfo.pstEcsConfig->stRsCfgInfo.stPartCfg.stEcuCfgList.num)
    {
        (void)pthread_rwlock_unlock(&gstEcsModuleInfo.stRWlock);
        log_e("Ecu part[reqId(%s), swDId(%s)] no find.", ecuReqId, ecuSwDid);   
        return ADM_ECODE_OTACFG_ECU_NONEXIST;
    }

    //!< copy exts
    iRet = adm_com_copyExtsObject(pstExtsList, &pstEcuCfg->stExtList);
    if (ADM_IS_ERR(iRet))
    {
        (void)pthread_rwlock_unlock(&gstEcsModuleInfo.stRWlock);
        log_e("Failed to copy exts info, err[%d]", iRet);
        return iRet;
    }
    (void)pthread_rwlock_unlock(&gstEcsModuleInfo.stRWlock);

    return iRet;
}

static D32S ecsVehicleConfigCheck(void)
{
    D32S i = 0;
    D32S j = 0;
    D32S iRet = ADM_ECODE_SUCCESSED;
    ADM_ECS_ECUCONFIG_T *pstEcuCfg = NULL;
    ADM_ECS_ECUCONFIG_T *pstEcuOffsetCfg = NULL;
    ADM_ECS_SAME_CFG_NODE_T *pstCfgNode = NULL;
    
    ADM_COM_FUNC_ENTRY();

    iRet = __adm_ecs_checkAndInitModule();
    if (ADM_IS_ERR(iRet))
    {
        log_e("Failed to check module info, err[%d]", iRet);
        return iRet;
    }

    (void)pthread_rwlock_wrlock(&gstEcsModuleInfo.stRWlock);    
    for (i = 0; i< gstEcsModuleInfo.pstEcsConfig->stRsCfgInfo.stPartCfg.stEcuCfgList.num; i++)
    {
        pstEcuCfg = &gstEcsModuleInfo.pstEcsConfig->stRsCfgInfo.stPartCfg.stEcuCfgList.pstEcuCfg[i];
        for (j = i + 1; j < gstEcsModuleInfo.pstEcsConfig->stRsCfgInfo.stPartCfg.stEcuCfgList.num; j++)
        {
            pstEcuOffsetCfg = &gstEcsModuleInfo.pstEcsConfig->stRsCfgInfo.stPartCfg.stEcuCfgList.pstEcuCfg[j];
            if (0 == adm_strcasecmp(pstEcuCfg->ecuReqId, pstEcuOffsetCfg->ecuReqId) &&
                0 == adm_strcasecmp(pstEcuCfg->ecuSwDid, pstEcuOffsetCfg->ecuSwDid))
            {
                /** 判断对列中是否存在，不存在，则添加到对列，已存在则不处理 */
                if (FALSE == ecsIsExistNode(pstEcuCfg->ecuReqId, pstEcuCfg->ecuSwDid))
                {
                    pstCfgNode = ecsSameCfgEcuNodeNew(pstEcuCfg->ecuReqId, pstEcuCfg->ecuSwDid);
                    if (NULL != pstCfgNode)
                    {
                        ecsSameCfgEcuNodeAddTail(&pstCfgNode->entry);
                        break;
                    }

                    pstCfgNode = NULL;
                }
                else
                {
                    break;
                }
            }
        }
    }

    (void)pthread_rwlock_unlock(&gstEcsModuleInfo.stRWlock);

    ADM_COM_FUNC_EXIT();

    return iRet;
}

static void ecsSameConfigSetFlag(ADM_ECS_GET_ECU_DID_CALLBACK_F pfGetEcuDidCallback, DBOOL bIsInstalling)
{
    D32S i = 0;
    D32S iRet = ADM_ECODE_SUCCESSED;
    ADM_ECS_ECUCONFIG_T *pstEcuCfg = NULL;
    ADM_ECS_ECUCONFIG_T *pstEcuOffsetCfg = NULL;
    ADM_ECS_DID_VALUE_LIST_T stDidList = {0};
    ADM_ECS_SAME_CFG_NODE_T *pstNodeInfo = NULL;
    DBOOL bIsRetry = FALSE;
    DBOOL bIsCompareSucceed = DFALSE;

    ADM_COM_FUNC_ENTRY();

    (void)pthread_rwlock_wrlock(&gstEcsModuleInfo.stRWlock);
    for (i = 0; i < gstEcsModuleInfo.pstEcsConfig->stRsCfgInfo.stPartCfg.stEcuCfgList.num; i++)
    {
        bIsRetry = FALSE;
        pstEcuCfg = &gstEcsModuleInfo.pstEcsConfig->stRsCfgInfo.stPartCfg.stEcuCfgList.pstEcuCfg[i];

        if (TRUE == ecsIsExistNode(pstEcuCfg->ecuReqId, pstEcuCfg->ecuSwDid))
        {
            pstNodeInfo = ecsGetNodeInfo(pstEcuCfg->ecuReqId, pstEcuCfg->ecuSwDid);
            if (NULL != pstNodeInfo && TRUE == pstNodeInfo->bIsGetEcuInfo)
            {
                if (DTRUE == pstNodeInfo->bIsCompareSucceed)
                {
                    if (DFALSE == bIsInstalling)
                    {
                        /** 如果已经有收集成功的ecu，则删除零件配置中本条重复的高低配ecu信息
                            安装阶段使用的是已存在的sameList,所以不能直接删除,需要compare一次即可
                        */
                        log_i("requestId[%s] swDid[%s] had collected succeed. need del.", pstEcuCfg->ecuReqId, pstEcuCfg->ecuSwDid);
                        ecsSameCfgEcuNodeDel(pstNodeInfo);
                        pstNodeInfo = NULL;
                        pstEcuCfg->bIsNeedDel = DTRUE;
                        continue;
                    }
                }
                
                ecsEcuDidListClone(&pstNodeInfo->stDidList, &stDidList);

                iRet = ecsDidValueCompare(&pstEcuCfg->stExternEcuInfo, &stDidList, &pstNodeInfo->bIsCompareSucceed);
                if (ADM_IS_ERR(iRet))
                {
                    log_d("pcEcuReqId = %s, pcEcuSwId = %s need del", pstEcuCfg->ecuReqId, pstEcuCfg->ecuSwDid);
                    pstEcuCfg->bIsNeedDel = TRUE;
                }
                else 
                {
                    if (DTRUE == pstNodeInfo->bIsCompareSucceed)
                    {
                        if (DFALSE == bIsInstalling)
                        {
                            ecsUpdateNodeInfo(pstEcuCfg->ecuReqId, pstEcuCfg->ecuSwDid, &stDidList,
                                    DFALSE, pstNodeInfo->bIsCompareSucceed);
                        }
                    }
                }

                ecsSameCfgEcuNodeDel(pstNodeInfo);
                pstNodeInfo = NULL;
            }
            else
            {
                do
                {
                    iRet = pfGetEcuDidCallback(pstEcuCfg, 
                                               &gstEcsModuleInfo.pstEcsConfig->stRsCfgInfo.stGeneralCfg.stEcuDidList, 
                                               gstEcsModuleInfo.pstEcsConfig->stRsCfgInfo.stGeneralCfg.stBusinessScript.filePath,
                                               &stDidList);
                    if ((ADM_ECODE_RPC_INSTANCE_OFFLINE == iRet || 
                        ADM_ECODE_RPC_SERVICE_OFFLINE == iRet ||
                        ADM_ECODE_RPC_REQSERV_TIMEOUT == iRet) &&
                        FALSE == bIsRetry)
                    {
                        log_w("get failed, retry");
                        sleep(5);
                        bIsRetry = TRUE;
                    }
                    else
                    {
                        log_w("get finish, iRet = %d", iRet);
                        break;
                    }
                } while (TRUE);

                if (ADM_IS_ERR(iRet))
                {
                    log_d("pcEcuReqId = %s, pcEcuSwId = %s get failed", pstEcuCfg->ecuReqId, pstEcuCfg->ecuSwDid);
                    continue;
                }

                pstNodeInfo->bIsCompareSucceed = DFALSE;
                log_i("pcEcuReqId = %s, pcEcuSwId = %s do value compare.", pstEcuCfg->ecuReqId, pstEcuCfg->ecuSwDid);
                iRet = ecsDidValueCompare(&pstEcuCfg->stExternEcuInfo, &stDidList, &pstNodeInfo->bIsCompareSucceed);
                if (ADM_IS_ERR(iRet))
                {
                    log_d("pcEcuReqId = %s, pcEcuSwId = %s need del", pstEcuCfg->ecuReqId, pstEcuCfg->ecuSwDid);
                    pstEcuCfg->bIsNeedDel = TRUE;
                }

                ecsUpdateNodeInfo(pstEcuCfg->ecuReqId, pstEcuCfg->ecuSwDid, &stDidList, 
                        DTRUE, pstNodeInfo->bIsCompareSucceed);
            }

            ecsDidValueListFree(&stDidList);
        }
    }

    /** 高低配ecu中,如果第一次收集的零件信息未匹配但是有default字段,后续收集的其他高低配记录成功匹配的,需要删除本条未匹配的记录 */
    log_i("Begin delete default ecu info.");
    for (i = 0; i < gstEcsModuleInfo.pstEcsConfig->stRsCfgInfo.stPartCfg.stEcuCfgList.num; i++)
    {
        pstEcuCfg = &gstEcsModuleInfo.pstEcsConfig->stRsCfgInfo.stPartCfg.stEcuCfgList.pstEcuCfg[i];

        if (DFALSE == pstEcuCfg->bIsNeedDel)
        {
            if (TRUE == ecsIsExistNode(pstEcuCfg->ecuReqId, pstEcuCfg->ecuSwDid))
            {
                pstNodeInfo = ecsGetNodeInfo(pstEcuCfg->ecuReqId, pstEcuCfg->ecuSwDid);
                if (DTRUE == pstNodeInfo->bIsCompareSucceed)
                {
                    /** 删除有default值单并未匹配的高低配ecu信息 */
                    ecsEcuDidListClone(&pstNodeInfo->stDidList, &stDidList);

                    iRet = ecsDidValueCompare(&pstEcuCfg->stExternEcuInfo, &stDidList, &bIsCompareSucceed);
                    if (ADM_IS_SUCCESS(iRet) && DFALSE == bIsCompareSucceed)
                    {
                        log_i("requestId[%s] swDid[%s] had collected succeed. need del default value one.", pstEcuCfg->ecuReqId, pstEcuCfg->ecuSwDid);
                        pstEcuCfg->bIsNeedDel = DTRUE;
                    }

                    ecsDidValueListFree(&stDidList);
                    ecsSameCfgEcuNodeDel(pstNodeInfo);
                    pstNodeInfo = NULL;
                }
            }
        }
    }
    log_i("End delete default ecu info.");

    (void)pthread_rwlock_unlock(&gstEcsModuleInfo.stRWlock);

    /** 配置信息存储KV */
    ecsSameCfgEcuListSetToKv();

    ADM_COM_FUNC_EXIT();

    return ;
}

static ADM_ECS_ECUCONFIG_T *ecsNewConfigCreate(D32S iCfgCount, ADM_ECS_ECUCFGLIST_T *pstCfgList)
{
    D32S i = 0;
    D32S j = 0;
    D32S iRet = ADM_ECODE_SUCCESSED;
    ADM_ECS_ECUCONFIG_T *pstEcuCfg = NULL;

    ADM_COM_FUNC_ENTRY();

    pstEcuCfg = adm_ecs_newEcusConfigList(iCfgCount);
    if (ADM_IS_NULL(pstEcuCfg))
    {
        return pstEcuCfg;
    }

    for (i = 0; i < pstCfgList->num; i++)
    {
        if (TRUE == pstCfgList->pstEcuCfg[i].bIsNeedDel)
        {
            continue;
        }
        
        pstEcuCfg[j].ecuReqId = adm_strdup(pstCfgList->pstEcuCfg[i].ecuReqId);
        pstEcuCfg[j].ecuSwDid = adm_strdup(pstCfgList->pstEcuCfg[i].ecuSwDid);
        pstEcuCfg[j].ecuResponseId = adm_strdup(pstCfgList->pstEcuCfg[i].ecuResponseId);
        pstEcuCfg[j].ecuFunctionId = adm_strdup(pstCfgList->pstEcuCfg[i].ecuFunctionId);
        pstEcuCfg[j].readInfoInstanceId = adm_strdup(pstCfgList->pstEcuCfg[i].readInfoInstanceId);
        pstEcuCfg[j].downloadInstanceId = adm_strdup(pstCfgList->pstEcuCfg[i].downloadInstanceId);
        pstEcuCfg[j].transferInstanceId = adm_strdup(pstCfgList->pstEcuCfg[i].transferInstanceId);
        pstEcuCfg[j].installInstanceId = adm_strdup(pstCfgList->pstEcuCfg[i].installInstanceId);
        pstEcuCfg[j].stExternEcuInfo.partName = adm_strdup(pstCfgList->pstEcuCfg[i].stExternEcuInfo.partName);
        pstEcuCfg[j].stExternEcuInfo.partNumber = adm_strdup(pstCfgList->pstEcuCfg[i].stExternEcuInfo.partNumber);
        pstEcuCfg[j].stExternEcuInfo.partSwid = adm_strdup(pstCfgList->pstEcuCfg[i].stExternEcuInfo.partSwid);
        pstEcuCfg[j].stExternEcuInfo.softwarePartNumber = adm_strdup(pstCfgList->pstEcuCfg[i].stExternEcuInfo.softwarePartNumber);
        pstEcuCfg[j].stExternEcuInfo.partSupplier = adm_strdup(pstCfgList->pstEcuCfg[i].stExternEcuInfo.partSupplier);
        pstEcuCfg[j].stExternEcuInfo.partHardwareNum = adm_strdup(pstCfgList->pstEcuCfg[i].stExternEcuInfo.partHardwareNum);
        pstEcuCfg[j].stExternEcuInfo.upgradeOrder = adm_strdup(pstCfgList->pstEcuCfg[i].stExternEcuInfo.upgradeOrder);
        
        pstEcuCfg[j].selfBackup = pstCfgList->pstEcuCfg[i].selfBackup;
        pstEcuCfg[j].isSelfUpdate = pstCfgList->pstEcuCfg[i].isSelfUpdate;
        pstEcuCfg[j].busType = pstCfgList->pstEcuCfg[i].busType;
        pstEcuCfg[j].channelType = pstCfgList->pstEcuCfg[i].channelType;
        pstEcuCfg[j].crcMask = pstCfgList->pstEcuCfg[i].crcMask;
        pstEcuCfg[j].crcLibNumber = pstCfgList->pstEcuCfg[i].crcLibNumber;
        pstEcuCfg[j].saMask = adm_strdup(pstCfgList->pstEcuCfg[i].saMask);
        pstEcuCfg[j].flashProgressType = pstCfgList->pstEcuCfg[i].flashProgressType;
        pstEcuCfg[j].flashScriptNumber = pstCfgList->pstEcuCfg[i].flashScriptNumber;
        pstEcuCfg[j].isCGWRouted = pstCfgList->pstEcuCfg[i].isCGWRouted;
        pstEcuCfg[j].serviceType = pstCfgList->pstEcuCfg[i].serviceType;
        pstEcuCfg[j].instCondition = pstCfgList->pstEcuCfg[i].instCondition;
        pstEcuCfg[j].securityLevel = pstCfgList->pstEcuCfg[i].securityLevel;

        pstEcuCfg[j].saMaskList = (ADM_ECS_SA_MASK_LIST_T*)adm_malloc(sizeof(ADM_ECS_SA_MASK_LIST_T));
        iRet = __adm_ecs_fillEcuSaMaskList(pstEcuCfg[j].saMaskList, pstCfgList->pstEcuCfg[i].saMaskList);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to fill saMask list, err[%d]", iRet);
            break;
        }

        iRet = __adm_ecs_fillEcuSaLibNumberList(&pstEcuCfg[j].saLibNumberList,&pstCfgList->pstEcuCfg[i].saLibNumberList);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to fill saLibNumber list, err[%d]", iRet);
            break;
        }

        iRet = __adm_ecs_fillEcuCrcLibNumberList(&pstEcuCfg[j].crcLibNumberList, &pstCfgList->pstEcuCfg[i].crcLibNumberList);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to fill crcLibNumber list, err[%d]", iRet);
            break;
        }

        iRet = __adm_ecs_fillEcuDidList(&pstEcuCfg[j].stEcuDidList, &pstCfgList->pstEcuCfg[i].stEcuDidList);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to fill resource did list, err[%d]", iRet);
            break;
        }

        j++;
    }

    ADM_COM_FUNC_EXIT();

    return pstEcuCfg;
}

static void ecsSameConfigDel(void)
{
    D32S i = 0;
    D32S iRet = ADM_ECODE_SUCCESSED;
    ADM_ECS_ECUCONFIG_T *pstEcuCfg = NULL;
    ADM_ECS_ECUCONFIG_T *pstNewEcuCfg = NULL;
    D32S iNeedDelCount = 0;
    D32S iNewCfgCount = 0;
    
    ADM_COM_FUNC_ENTRY();

    do
    {
        (void)pthread_rwlock_wrlock(&gstEcsModuleInfo.stRWlock);
        for (i = 0; i < gstEcsModuleInfo.pstEcsConfig->stRsCfgInfo.stPartCfg.stEcuCfgList.num; i++)
        {
            pstEcuCfg = &gstEcsModuleInfo.pstEcsConfig->stRsCfgInfo.stPartCfg.stEcuCfgList.pstEcuCfg[i];
            if (TRUE == pstEcuCfg->bIsNeedDel)
            {
                iNeedDelCount++;
            }
        }

        if (0 == iNeedDelCount)
        {
            log_d("don't have config need delete");
            (void)pthread_rwlock_unlock(&gstEcsModuleInfo.stRWlock);
            break;
        }

        iNewCfgCount = gstEcsModuleInfo.pstEcsConfig->stRsCfgInfo.stPartCfg.stEcuCfgList.num - iNeedDelCount;
        pstNewEcuCfg = ecsNewConfigCreate(iNewCfgCount, &gstEcsModuleInfo.pstEcsConfig->stRsCfgInfo.stPartCfg.stEcuCfgList);
        if (ADM_IS_NULL(pstNewEcuCfg))
        {
            log_e("ecsNewConfigCreate failed");
            (void)pthread_rwlock_unlock(&gstEcsModuleInfo.stRWlock);
            break;
        }

        adm_ecs_delEcusConfigList(gstEcsModuleInfo.pstEcsConfig->stRsCfgInfo.stPartCfg.stEcuCfgList.num, 
                                  &gstEcsModuleInfo.pstEcsConfig->stRsCfgInfo.stPartCfg.stEcuCfgList.pstEcuCfg);
        gstEcsModuleInfo.pstEcsConfig->stRsCfgInfo.stPartCfg.stEcuCfgList.num = iNewCfgCount;
        gstEcsModuleInfo.pstEcsConfig->stRsCfgInfo.stPartCfg.stEcuCfgList.pstEcuCfg = pstNewEcuCfg;

        (void)pthread_rwlock_unlock(&gstEcsModuleInfo.stRWlock);
    } while (FALSE);

    ADM_COM_FUNC_EXIT();

    return;
}

void adm_ecs_sameConfigHandle(DBOOL bIsInstalling, ADM_ECS_GET_ECU_DID_CALLBACK_F pfGetEcuDidCallback)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    ADM_ECS_RSCONFIGINFO_T *pstRsCfgInfo = NULL;

    ADM_COM_FUNC_ENTRY();

    ecsSameCfgEcuListInit();

    do
    {
        if (TRUE == bIsInstalling)
        {
            ecsSameCfgEcuListGetFromKv();
        }
        else
        {
            iRet = ecsVehicleConfigCheck();
            if (ADM_IS_ERR(iRet))
            {
                log_e("Failed to check vehicle config, err[%d]", iRet);
                break ;
            }
        }

        ecsSameConfigSetFlag(pfGetEcuDidCallback, bIsInstalling);

        ecsSameConfigDel();

    } while (FALSE);

    ecsSameCfgEcuListDeInit();

    g_bIsUpdateGlobalConfig = TRUE;

    ADM_COM_FUNC_EXIT();

    return ;
}

DBOOL adm_ecs_isUpdateGlobalConfigFinish(void)
{
    return g_bIsUpdateGlobalConfig;
}

/*!
 *******************************************************************************
 * 获取ecu配置ECU ID信息.
 *
 * \note
 * 
 * \param[in]  ecuReqId      零件诊断请求ID
 * \param[in]  ecuSwDid      零件升级对象软件版本did 
 * \param[out]  pstEcsEcuId  配置信息中得ECUID信息 
 * \retval     D32S
 *******************************************************************************
 */
D32S adm_ecs_getEcuConfigEcuId(DCHAR *ecuReqId, DCHAR *ecuSwDid, ADM_ECS_ECU_ID_T* pstEcsEcuId)
{
    D32S i = 0;
    D32S iRet = ADM_ECODE_SUCCESSED;
    ADM_ECS_ECUCONFIG_T *pstEcuCfg = NULL;

    if (ADM_IS_NULL(ecuReqId) || ADM_IS_NULL(ecuSwDid) || ADM_IS_NULL(pstEcsEcuId))
    {
        log_e("Invalid input param.");
        return ADM_ECODE_INVALID_PARAM;
    }

    iRet = __adm_ecs_checkAndInitModule();
    if (ADM_IS_ERR(iRet))
    {
        log_e("Failed to check module info, err[%d]", iRet);
        return iRet;
    }

    (void)pthread_rwlock_rdlock(&gstEcsModuleInfo.stRWlock);
    for (i = 0; i< gstEcsModuleInfo.pstEcsConfig->stRsCfgInfo.stPartCfg.stEcuCfgList.num; i++)
    {
        pstEcuCfg = &gstEcsModuleInfo.pstEcsConfig->stRsCfgInfo.stPartCfg.stEcuCfgList.pstEcuCfg[i];
        if (htoi(ecuReqId) == htoi(pstEcuCfg->ecuReqId) &&
            htoi(ecuSwDid) == htoi(pstEcuCfg->ecuSwDid))
        {
            pstEcsEcuId->pEcuReqId = adm_strdup(pstEcuCfg->ecuReqId);
            pstEcsEcuId->pEcuSwDid = adm_strdup(pstEcuCfg->ecuSwDid);
            log_i("pEcuReqId:%s,pEcuSwDid:%s",pstEcsEcuId->pEcuReqId,pstEcsEcuId->pEcuSwDid);
            break;
        }
    }
    (void)pthread_rwlock_unlock(&gstEcsModuleInfo.stRWlock);

    return ADM_ECODE_SUCCESSED;
}