#include "adm_eim_ecuInfoManager.h"
#include "adm_string.h"
#include "adm_file.h"
#include "adm_memory.h"
#include "adm_dim_vdi.h"
#include "adm_com_lib.h"
#include "adm_debug.h"
#include "adm_com_kvstorage.h"
#include <time.h>
#include <sys/unistd.h>
#include "gtest/gtest.h"

#ifdef __cplusplus
extern "C" {
#endif


static DCHAR s_acCnfDir[256] = {};

void testSetConfigDir(const DCHAR* pDir)
{
    adm_strcpy(s_acCnfDir, pDir, sizeof(s_acCnfDir));
}

D32S testReadCnfFile(const DCHAR* pFilename, DCHAR* pBuf, D32U uiBufSize)
{
    D32U uiSize = -1;
    void* pFile = NULL;
    DCHAR acPath[1024];

    adm_snprintf(acPath, sizeof(acPath), "%s/%s", s_acCnfDir, pFilename);

    adm_fopen(acPath, "a+", &pFile);
    if (NULL != pFile)
    {
        adm_fread(pBuf, uiBufSize - 1, &uiSize, pFile);
        pBuf[uiSize] = '\0';
        adm_fclose(&pFile);
        return (D32S)uiSize;
    }
    else
    {
        log_e("not found file[%s]", acPath);
    }

    return -1;
}


ADM_DIM_SOA_DID_DATA_T* parseEcuDidByJson(const json_object* pRoot)
{
    ADM_DIM_SOA_DID_DATA_T* pDids = NULL;
    const DCHAR* pDidValue = NULL;
    ADM_DIM_SOA_DATA_T* pDidData = NULL;

    if (NULL == pRoot)
    {
        return NULL;
    }

    pDids = (ADM_DIM_SOA_DID_DATA_T*)adm_malloc(sizeof(*pDids));
    if (NULL == pDids)
    {
        return NULL;
    }

    pDidData = (ADM_DIM_SOA_DATA_T*)adm_malloc(sizeof(*pDidData));
    if (NULL == pDidData)
    {
        return NULL;
    }

    pDids->did = htoi(json_object_get_string(json_object_object_get(pRoot, "did")));
    pDidValue = json_object_get_string(json_object_object_get(pRoot, "didVal"));

    pDids->startByte = 0;
    pDids->endByte = strlen(pDidValue);
    pDidData->dataLength = strlen(pDidValue);
    pDidData->dataType = VAR_STRING;
    pDidData->v.valueS = (D8U*)adm_strdup(pDidValue);
    pDids->didData = pDidData;

    return pDids;
}

ADM_DIM_SOA_DID_DATA_ARRAY_T* parseEcuDidsByJson(const json_object* pRoot)
{
    D32U uiDidCount = 0;
    ADM_DIM_SOA_DID_DATA_ARRAY_T* pDidArray = NULL;
    ADM_DIM_SOA_DID_DATA_T** pDidDatas = NULL;

    if (NULL == pRoot)
    {
        return NULL;
    }

    uiDidCount = json_object_array_length(pRoot);

    pDidArray = (ADM_DIM_SOA_DID_DATA_ARRAY_T*)adm_malloc(sizeof(*pDidArray));
    if (NULL == pDidArray)
    {
        return NULL;
    }

    pDidDatas = (ADM_DIM_SOA_DID_DATA_T**)adm_malloc(sizeof(*pDidDatas) * uiDidCount);
    if (NULL == pDidDatas)
    {
        return NULL;
    }

    for (D32U i = 0; i < uiDidCount; i++)
    {
        pDidDatas[i] = parseEcuDidByJson(json_object_array_get_idx(pRoot, i));
    }

    pDidArray->didArray = pDidDatas;
    pDidArray->didArrayLength = uiDidCount;

    return pDidArray;
}

ECU_INFO_T* parseEcuInfoByJson(const json_object* pRoot, ECU_INFO_T* pEcuInfo)
{
    if (NULL == pRoot)
    {
        return NULL;
    }
    pEcuInfo->ecuReqId = adm_strdup(json_object_get_string(json_object_object_get(pRoot, "ecuReqId")));
    pEcuInfo->ecuSwDid = adm_strdup(json_object_get_string(json_object_object_get(pRoot, "ecuSwDid")));
    pEcuInfo->vdiDidListResponse = parseEcuDidsByJson(json_object_object_get(pRoot, "didList"));
    return pEcuInfo;
}

D32U parseEcuInfosByJson(const json_object* pRoot, ECU_INFO_T** pOut)
{
    if (NULL == pRoot)
    {
        return 0;
    }

    D32U uiCount = json_object_array_length(pRoot);
    ECU_INFO_T* pEcuInfos = (ECU_INFO_T*)adm_malloc(sizeof(*pEcuInfos) * uiCount);

    if (NULL == pEcuInfos)
    {
        return 0;
    }

    for (D32U i = 0; i < uiCount; i++)
    {
        parseEcuInfoByJson(json_object_array_get_idx(pRoot, i), &pEcuInfos[i]);
    }

    *pOut = pEcuInfos;

    return uiCount;
}

D32U parseEcuInfosByJsonStr(const DCHAR* pStr, ECU_INFO_T** pEcuInfos)
{
    struct json_object* pRoot = json_tokener_parse(pStr);
    if (NULL == pRoot)
    {
        log_e("parse EcuInfos.json failed!");
        return 0;
    }

    D32U uiEcuCount = parseEcuInfosByJson(pRoot, pEcuInfos);
    json_object_put(pRoot);

    return uiEcuCount;
}

D32U parseEcuInfosByJsonFile(ECU_INFO_T** pEcuInfos)
{
    DCHAR acBuffer[4096 * 2];
    if (-1 == testReadCnfFile("EcuInfos.json", acBuffer, sizeof(acBuffer)))
    {
        return -1;
    }

    return parseEcuInfosByJsonStr(acBuffer, pEcuInfos);
}


void ecsConfigDataEcuDidInitByJson(const json_object* pRoot, ADM_ECS_ECUDID_T* pDid)
{
    if (NULL == pRoot)
    {
        return;
    }

    pDid->did = adm_strdup(json_object_get_string(json_object_object_get(pRoot, "did")));
    pDid->didValType = json_object_get_int(json_object_object_get(pRoot, "didValType"));
    pDid->start = json_object_get_int(json_object_object_get(pRoot, "start"));
    pDid->end = json_object_get_int(json_object_object_get(pRoot, "end"));
}

void ecsConfigDataEcuDidDestroy(ADM_ECS_ECUDID_T* pDid)
{
    adm_free(pDid->did);
}

void ecsConfigDataEcuDidsInitByJson(const json_object* pRoot, ADM_ECS_ECUDIDLIST_T* pEcuDids)
{
    D32S iDidCount = 0;
    ADM_ECS_ECUDID_T* pDids = NULL;

    if (NULL != pRoot)
    {
        iDidCount = json_object_array_length(pRoot);
        pDids = (ADM_ECS_ECUDID_T*)adm_malloc(sizeof(pDids[0]) * iDidCount);

        for (D32S i = 0; i < iDidCount; i++)
        {
            ecsConfigDataEcuDidInitByJson(json_object_array_get_idx(pRoot, i), &pDids[i]);
        }
    }

    pEcuDids->pstEcuDidInfo = pDids;
    pEcuDids->num = iDidCount;
}

void ecsConfigDataEcuDidsDestroy(ADM_ECS_ECUDIDLIST_T* pEcuDids)
{
    for (D32S i = 0; i < pEcuDids->num; i++)
    {
        ecsConfigDataEcuDidDestroy(&pEcuDids->pstEcuDidInfo[i]);
    }

    adm_free(pEcuDids->pstEcuDidInfo);
}

void ecsConfigDataEcuCfgInitByJson(const json_object* pRoot, ADM_ECS_ECUCONFIG_T* pEcuCfg)
{
    if (NULL == pRoot)
    {
        return;
    }

    pEcuCfg->ecuReqId = adm_strdup(json_object_get_string(json_object_object_get(pRoot, "ecuReqId")));
    pEcuCfg->ecuSwDid = adm_strdup(json_object_get_string(json_object_object_get(pRoot, "ecuSwDid")));
    pEcuCfg->ecuResponseId = adm_strdup(json_object_get_string(json_object_object_get(pRoot, "ecuResponseId")));
    pEcuCfg->ecuFunctionId = adm_strdup(json_object_get_string(json_object_object_get(pRoot, "ecuFunctionId")));
    pEcuCfg->installInstanceId = adm_strdup(json_object_get_string(json_object_object_get(pRoot, "installInstanceId")));
    pEcuCfg->transferInstanceId = adm_strdup(json_object_get_string(json_object_object_get(pRoot, "transferInstanceId")));
    pEcuCfg->readInfoInstanceId = adm_strdup(json_object_get_string(json_object_object_get(pRoot, "readInfoInstanceId")));
    pEcuCfg->serviceType = json_object_get_int(json_object_object_get(pRoot, "serviceType"));
    pEcuCfg->securityLevel = json_object_get_int(json_object_object_get(pRoot, "securityLevel"));
    pEcuCfg->busType = json_object_get_int(json_object_object_get(pRoot, "busType"));
    pEcuCfg->channelType = json_object_get_int(json_object_object_get(pRoot, "channelType"));
    pEcuCfg->isCGWRouted = json_object_get_int(json_object_object_get(pRoot, "isCGWRouted"));

    ecsConfigDataEcuDidsInitByJson(json_object_object_get(pRoot, "ecuDids"), &pEcuCfg->stEcuDidList);
}

void ecsConfigDataEcuCgfDestroy(ADM_ECS_ECUCONFIG_T* pEcuCfg)
{
    adm_free(pEcuCfg->ecuReqId);
    adm_free(pEcuCfg->ecuSwDid);
    adm_free(pEcuCfg->ecuResponseId);
    adm_free(pEcuCfg->ecuFunctionId);
    adm_free(pEcuCfg->installInstanceId);
}

void ecsConfigDataEcuCfgsInitByJson(const json_object* pRoot, ADM_ECS_ECUCFGLIST_T* pEcuCfgs)
{
    ADM_ECS_ECUCONFIG_T* pEcuConfigs = NULL;
    D32S iEcuCount = 0;

    if (NULL == pRoot)
    {
        return;
    }

    iEcuCount = json_object_array_length(pRoot);
    pEcuConfigs = (ADM_ECS_ECUCONFIG_T*)adm_malloc(sizeof(pEcuCfgs->pstEcuCfg[0]) * iEcuCount);
    if (NULL == pEcuConfigs)
    {
        return;
    }

    for (D32S i = 0; i < iEcuCount; i++)
    {
        ecsConfigDataEcuCfgInitByJson(json_object_array_get_idx(pRoot, i), &pEcuConfigs[i]);
    }

    pEcuCfgs->pstEcuCfg = pEcuConfigs;
    pEcuCfgs->num = iEcuCount;
}

void ecsConfigDataEcuCgfsDestroy(ADM_ECS_ECUCFGLIST_T* pEcuCfgs)
{
    for (D32S i = 0; i < pEcuCfgs->num; i++)
    {
        ecsConfigDataEcuCgfDestroy(&pEcuCfgs->pstEcuCfg[i]);
    }

    adm_free(pEcuCfgs->pstEcuCfg);
}

D32S ecsConfigFileObjectByJson(struct json_object* pRoot, ADM_COM_FILEOBJECT_T* pFileObject)
{
    if (NULL == pRoot)
    {
        return -1;
    }

    pFileObject->fileName = adm_strdup(json_object_get_string(json_object_object_get(pRoot, "fileName")));
    pFileObject->hash = adm_strdup(json_object_get_string(json_object_object_get(pRoot, "hash")));
    pFileObject->size = json_object_get_int(json_object_object_get(pRoot, "size"));
    pFileObject->filePath = adm_strdup(pFileObject->fileName);
    return 0;
}

void ecsConfigFileObjectDestroy(ADM_COM_FILEOBJECT_T* pFileObject)
{
    adm_free(pFileObject->fileName);
    adm_free(pFileObject->hash);
    adm_free(pFileObject->filePath);
}


D32S ecsParseGeneralConfigByJson(struct json_object* pRoot, ADM_ECS_GENERALCONFIG_T* pGeneralCnf)
{
    pGeneralCnf->cyclePeriod = json_object_get_int(json_object_object_get(pRoot, "cyclePeriod"));
    pGeneralCnf->eventReportLevel = json_object_get_int(json_object_object_get(pRoot, "eventReportLevel"));
    ecsConfigDataEcuDidsInitByJson(json_object_object_get(pRoot, "ecuDids"), &pGeneralCnf->stEcuDidList);
    ecsConfigFileObjectByJson(json_object_object_get(pRoot, "businessScript"), &pGeneralCnf->stBusinessScript);
    return 0;
}


ADM_DIM_SOA_DID_DATA_T* ecuInfoGetDidData(ECU_INFO_T* pEcuInfo, D32S iDid)
{
    ADM_DIM_SOA_DID_DATA_T* pOut = NULL;
    ADM_DIM_SOA_DID_DATA_ARRAY_T* pDidDatas = pEcuInfo->vdiDidListResponse;
    ADM_DIM_SOA_DID_DATA_T* pDidData = NULL;

    for (D32U i = 0; i < pDidDatas->didArrayLength; i++)
    {
        pDidData = pDidDatas->didArray[i];
        if (iDid == pDidData->did)
        {
            pOut = (ADM_DIM_SOA_DID_DATA_T*)adm_malloc(sizeof(*pOut));
            if (NULL == pOut)
            {
                break;
            }

            pOut->did = iDid;
            pOut->startByte = pDidData->startByte;
            pOut->endByte = pDidData->endByte;
            pOut->errorCode = 0;

            ADM_DIM_SOA_DATA_T* pSoaData = pDidData->didData;
            ADM_DIM_SOA_DATA_T* pOutSoaData = (ADM_DIM_SOA_DATA_T*)adm_malloc(sizeof(*pOutSoaData));
            switch (pSoaData->dataType)
            {
            case VAR_U8:
            case VAR_U32:
            case VAR_S32:
            case VAR_U64:
            case VAR_FLOAT:
            case VAR_S64:
                memcpy(&pOutSoaData->v, &pSoaData->v, sizeof(D64U));
                break;
            case VAR_STRING:
                pOutSoaData->v.valueS = (D8U*)adm_strdup((DCHAR*)pSoaData->v.valueS);
                break;
            default:
                break;
            }

            pOutSoaData->dataType = pSoaData->dataType;
            pOutSoaData->dataLength = pSoaData->dataLength;
            pOut->didData = pOutSoaData;
            break;
        }
    }

    return pOut;
}


typedef struct 
{
    D32S    iResult;
    D32U    uiCostTime;
} EcuGetCtrlRule;

typedef struct
{
    EcuGetCtrlRule* pCtrlRules;
    D32U            uiCtrlRuleCount;
} EcuGetCtrlRules;

typedef struct
{
    D32U            uiEcuReqId;
    D32U            uiEcuSwDId;
    EcuGetCtrlRules rules;
    D32U            uiCurrentPos;
} EcuGetCtrl;

typedef struct
{
    EcuGetCtrl* pCtrls;
    D32U        uiCtrlCount;
} EcuGetCtrls;

static EcuGetCtrls s_ecuGetCtrls;


void testEcuGetCtrlsInit(D32U uiCtrsCount)
{
    s_ecuGetCtrls.uiCtrlCount = 0;
    if (uiCtrsCount > 0)
    {
        s_ecuGetCtrls.pCtrls = (EcuGetCtrl*)adm_malloc(sizeof(EcuGetCtrl) * uiCtrsCount);
    }
    else
    {
        s_ecuGetCtrls.pCtrls = NULL;
    }
}

void testEcuGetCtrlDestroy(EcuGetCtrl* pCtrl)
{
    adm_free(pCtrl->rules.pCtrlRules);
}

void testEcuGetCtrlsDestroy()
{
    for (D32U i = 0; i < s_ecuGetCtrls.uiCtrlCount; i++)
    {
        testEcuGetCtrlDestroy(&s_ecuGetCtrls.pCtrls[i]);
    }

    adm_free(s_ecuGetCtrls.pCtrls);
}

const EcuGetCtrlRule* testEcuGetCtrlsGetCtrlRule(D32U uiEcuReqId, D32U uiEcuSwDId)
{
    EcuGetCtrlRule* pRule = NULL;
    EcuGetCtrl* pCtr = NULL;

    for (D32U i = 0; i < s_ecuGetCtrls.uiCtrlCount; i++)
    {
        pCtr = &s_ecuGetCtrls.pCtrls[i];
        if (uiEcuReqId == pCtr->uiEcuReqId && uiEcuSwDId == pCtr->uiEcuSwDId)
        {
            if (pCtr->uiCurrentPos < pCtr->rules.uiCtrlRuleCount)
            {
                pRule = &pCtr->rules.pCtrlRules[pCtr->uiCurrentPos++];
            }
            break;
        }
    }

    return pRule;
}


void testEcuGetCtrlRuleAddByJson(const struct json_object* pRoot, EcuGetCtrlRule* pRule)
{
    pRule->iResult = json_object_get_int(json_object_object_get(pRoot, "result"));
    pRule->uiCostTime = json_object_get_int(json_object_object_get(pRoot, "costTime"));
}

void testEcuGetCtrlRulesAddByJson(const struct json_object* pRoot, EcuGetCtrlRules* pRules)
{
    if (NULL == pRoot)
    {
        pRules->uiCtrlRuleCount = 0;
        pRules->pCtrlRules = NULL;
        return;
    }

    EcuGetCtrlRule* pCtrlRules = NULL;
    D32U uiCount = json_object_array_length(pRoot);

    pCtrlRules = (EcuGetCtrlRule*)adm_malloc(sizeof(EcuGetCtrlRule) * uiCount);
    for (D32U i = 0; i < uiCount; i++)
    {
        testEcuGetCtrlRuleAddByJson(json_object_array_get_idx(pRoot, i), &pCtrlRules[i]);
    }

    pRules->pCtrlRules = pCtrlRules;
    pRules->uiCtrlRuleCount = uiCount;
}

void testEcuGetCtrlAddByJson(const struct json_object* pRoot)
{
    const DCHAR* pEcuReqId = json_object_get_string(json_object_object_get(pRoot, "ecuReqId"));
    const DCHAR* pEcuSwDid = json_object_get_string(json_object_object_get(pRoot, "ecuSwDid"));
    EcuGetCtrl* pCtr = &s_ecuGetCtrls.pCtrls[s_ecuGetCtrls.uiCtrlCount];

    pCtr->uiEcuReqId = htoi(pEcuReqId);
    pCtr->uiEcuSwDId = htoi(pEcuSwDid);
    testEcuGetCtrlRulesAddByJson(json_object_object_get(pRoot, "controlRules"), &pCtr->rules);
    pCtr->uiCurrentPos = 0;
    s_ecuGetCtrls.uiCtrlCount++;
}


void testEcuGetCtrlsInitByJson(const struct json_object* pRoot)
{
    if (NULL == pRoot)
    {
        testEcuGetCtrlsInit(0U);
        return;
    }

    D32U uiCount = json_object_array_length(pRoot);
    testEcuGetCtrlsInit(uiCount);
    if (0 == uiCount)
    {
        return;
    }

    for (D32U i = 0; i < uiCount; i++)
    {
        testEcuGetCtrlAddByJson(json_object_array_get_idx(pRoot, i));
    }
}


D32S getEcuInfoDids(D32U uiEcuReqId, D32U uiEcuSwDId, const ADM_DIM_SOA_DID_DATA_ARRAY_T* pEcuReqDids, ADM_DIM_SOA_DID_DATA_ARRAY_T** pEcuRespDids)
{
    ECU_INFO_T* pEcuInfo = NULL;
    ECU_INFO_T* pEcuInfos = NULL;
    ADM_DIM_SOA_DID_DATA_T* pDidData = NULL;
    ADM_DIM_SOA_DID_DATA_ARRAY_T* pDataArray = NULL;
    D32U    uiDidDataCount = 0;
    ADM_DIM_SOA_DID_DATA_T** pDidDatas = NULL;

    pDataArray = (ADM_DIM_SOA_DID_DATA_ARRAY_T*)adm_malloc(sizeof(ADM_DIM_SOA_DID_DATA_ARRAY_T));
    if (NULL == pDataArray)
    {
        return -1;
    }

    pDidDatas = (ADM_DIM_SOA_DID_DATA_T**)adm_malloc(sizeof(ADM_DIM_SOA_DID_DATA_T*) * pEcuReqDids->didArrayLength);
    if (NULL == pDidDatas)
    {
        return -1;
    }

    D32U uiEcuCount = parseEcuInfosByJsonFile(&pEcuInfos);
    for (D32U i = 0; i < uiEcuCount; i++)
    {
        pEcuInfo = &pEcuInfos[i];
        if (uiEcuReqId == htoi(pEcuInfo->ecuReqId) && uiEcuSwDId == htoi(pEcuInfo->ecuSwDid))
        {
            break;
        }
        pEcuInfo = NULL;
    }

    if (NULL == pEcuInfo)
    {
        return -1;
    }


    for (D32U i = 0; i < pEcuReqDids->didArrayLength; i++)
    {
        pDidData = ecuInfoGetDidData(pEcuInfo, pEcuReqDids->didArray[i]->did);
        if (NULL != pDidData)
        {
            pDidDatas[uiDidDataCount++] = pDidData;
        }
    }

    if (uiDidDataCount > 0)
    {
        pDataArray->didArray = pDidDatas;
        pDataArray->didArrayLength = uiDidDataCount;
        *pEcuRespDids = pDataArray;
        return 0;
    }
    else
    {
        adm_free(pDidDatas);
        adm_free(pDataArray);
        return -1;
    }
}

D32S getEcuInfoDidsByRule(D32U uiEcuReqId, D32U uiEcuSwDId, const ADM_DIM_SOA_DID_DATA_ARRAY_T* pEcuReqDids, ADM_DIM_SOA_DID_DATA_ARRAY_T** pEcuRespDids)
{
    D32S iRet = -1;

    const EcuGetCtrlRule* pCtrRule = testEcuGetCtrlsGetCtrlRule(uiEcuReqId, uiEcuSwDId);
    if (NULL != pCtrRule)
    {
        sleep(pCtrRule->uiCostTime);
        if (0 != pCtrRule->iResult)
        {
            return pCtrRule->iResult;
        }
    }

    return getEcuInfoDids(uiEcuReqId, uiEcuSwDId, pEcuReqDids, pEcuRespDids);
}


// 
// ADM_ECS_RSCONFIGINFO_T *adm_ecs_newRSConfigInfo(void)
// {
//     ADM_ECS_RSCONFIGINFO_T *pstRsCfgInfo = NULL;
// 
//     pstRsCfgInfo = (ADM_ECS_RSCONFIGINFO_T *)adm_malloc(sizeof(*pstRsCfgInfo));
//     if (NULL != pstRsCfgInfo)
//     {
//         (void)adm_memset(pstRsCfgInfo, 0x00, sizeof(*pstRsCfgInfo), sizeof(*pstRsCfgInfo));
//     }
// 
//     return pstRsCfgInfo;
// }
// 
// void adm_ecs_delRSConfigInfo(ADM_ECS_RSCONFIGINFO_T **pstRsCfgInfo)
// {
//     if (ADM_IS_NULL(pstRsCfgInfo) || ADM_IS_NULL(*pstRsCfgInfo))
//     {
//         return;
//     }
// 
// //     adm_ecs_cleanPartConfig(&(*pstRsCfgInfo)->stPartCfg);
// //     adm_ecs_cleanGeneralConfig(&(*pstRsCfgInfo)->stGeneralCfg);
//     adm_free(*pstRsCfgInfo);
// }

D32S __wrap_adm_ecs_getEcuConfig(DCHAR* ecuReqId, DCHAR* ecuSwDid, ADM_ECS_ECUCONFIG_T** ppstEcuConfig)
{
    D32S iRet = -1;
    DCHAR acBuffer[4096 * 2];
    ADM_ECS_ECUCFGLIST_T ecuCfgList;
    ADM_ECS_ECUCONFIG_T* pEcuCnf = NULL;
    ADM_ECS_ECUCONFIG_T* pOut = NULL;

    if (-1 == testReadCnfFile("PartConfig.json", acBuffer, sizeof(acBuffer)))
    {
        return -1;
    }

    struct json_object* pRoot = json_tokener_parse(acBuffer);
    if (NULL == pRoot)
    {
        log_e("parse part config failed!");
        return -1;
    }
    ecsConfigDataEcuCfgsInitByJson(pRoot, &ecuCfgList);
    json_object_put(pRoot);

    for (D32S i = 0; i < ecuCfgList.num; i++)
    {
        pEcuCnf = &ecuCfgList.pstEcuCfg[i];
        if (0 == adm_strcmp(pEcuCnf->ecuReqId, ecuReqId) && 
            0 == adm_strcmp(pEcuCnf->ecuSwDid, ecuSwDid))
        {
           pOut = (ADM_ECS_ECUCONFIG_T*)adm_malloc(sizeof(*pOut));
           adm_memset(pOut, 0, sizeof(*pOut), sizeof(*pOut));

            pOut->ecuReqId = adm_strdup(pEcuCnf->ecuReqId);
            pOut->ecuSwDid = adm_strdup(pEcuCnf->ecuSwDid);
            pOut->busType = pEcuCnf->busType;
            pOut->channelType = pEcuCnf->channelType;
            pOut->isCGWRouted = pEcuCnf->isCGWRouted;
            pOut->securityLevel = pEcuCnf->securityLevel;
            pOut->serviceType = pEcuCnf->serviceType;
            pOut->ecuFunctionId = adm_strdup(pEcuCnf->ecuFunctionId);
            pOut->ecuResponseId = adm_strdup(pEcuCnf->ecuResponseId);
            pOut->installInstanceId = adm_strdup(pEcuCnf->installInstanceId);
            pOut->readInfoInstanceId = adm_strdup(pEcuCnf->readInfoInstanceId);
            pOut->transferInstanceId = adm_strdup(pEcuCnf->transferInstanceId);

            pOut->stEcuDidList.num = pEcuCnf->stEcuDidList.num;
            ADM_ECS_ECUDID_T* pEcuDids = (ADM_ECS_ECUDID_T*)adm_malloc(sizeof(*pEcuDids) * pEcuCnf->stEcuDidList.num);
            for (D32S i = 0; i < pEcuCnf->stEcuDidList.num; i++)
            {
                pEcuDids[i].did = adm_strdup(pEcuCnf->stEcuDidList.pstEcuDidInfo[i].did);
                pEcuDids[i].didValType = pEcuCnf->stEcuDidList.pstEcuDidInfo[i].didValType;
                pEcuDids[i].end = pEcuCnf->stEcuDidList.pstEcuDidInfo[i].end;
                pEcuDids[i].start = pEcuCnf->stEcuDidList.pstEcuDidInfo[i].start;
            }

            pOut->stEcuDidList.pstEcuDidInfo = pEcuDids;

            *ppstEcuConfig = pEcuCnf;
            iRet = 0;
            break;
        }
    }

    return iRet;
}

D32S __wrap_adm_ecs_getConfigInfo(ADM_ECS_RSCONFIGINFO_T* pstRsCfgInfo)
{
    DCHAR acBuffer[4096 * 2];

    if (-1 == testReadCnfFile("GeneralConfig.json", acBuffer, sizeof(acBuffer)))
    {
        return -1;
    }

    struct json_object* pRoot = json_tokener_parse(acBuffer);
    if (NULL == pRoot)
    {
        log_e("parse GeneralConfig.json failed!");
        return -1;
    }

    if (0 != ecsParseGeneralConfigByJson(pRoot, &pstRsCfgInfo->stGeneralCfg))
    {
        json_object_put(pRoot);
        return -1;
    }
    json_object_put(pRoot);

    if (-1 == testReadCnfFile("PartConfig.json", acBuffer, sizeof(acBuffer)))
    {
        return -1;
    }

    pRoot = json_tokener_parse(acBuffer);
    if (NULL == pRoot)
    {
        log_e("parse PartConfig.json failed!");
        return -1;
    }
    ecsConfigDataEcuCfgsInitByJson(pRoot, &pstRsCfgInfo->stPartCfg.stEcuCfgList);
    json_object_put(pRoot);

    return 0;
}

D32S __wrap_adm_ecs_getCheckCyclePeriod(D32S* pCyclePeriod)
{
    DCHAR acBuffer[4096 * 2];
    ADM_ECS_GENERALCONFIG_T stGeneralCfg = { 0 };

    if (-1 == testReadCnfFile("GeneralConfig.json", acBuffer, sizeof(acBuffer)))
    {
        *pCyclePeriod = 5;
        log_e("get check cycle period failed");
        return -1;
    }

    struct json_object* pRoot = json_tokener_parse(acBuffer);
    if (NULL == pRoot)
    {
        *pCyclePeriod = 5;
        log_e("parse GeneralConfig.json failed!");
        log_e("get check cycle period failed");
        return -1;
    }

    if (0 != ecsParseGeneralConfigByJson(pRoot, &stGeneralCfg))
    {
        *pCyclePeriod = 5;
        json_object_put(pRoot);
        log_e("get check cycle period failed");
        return -1;
    }

    json_object_put(pRoot);

    log_i("get cycle period[%d]", stGeneralCfg.cyclePeriod);
    *pCyclePeriod = stGeneralCfg.cyclePeriod;

    return 0;
}

D32S __wrap_adm_ecs_getEventReportLevel(D32S* pEventReportLevel)
{
    DCHAR acBuffer[4096];
    ADM_ECS_GENERALCONFIG_T stGeneralCfg = { 0 };

    if (-1 == testReadCnfFile("GeneralConfig.json", acBuffer, sizeof(acBuffer)))
    {
        return -1;
    }
    struct json_object* pRoot = json_tokener_parse(acBuffer);
    if (NULL == pRoot)
    {
        log_e("parse GeneralConfig.json failed!");
        return -1;
    }

    if (0 != ecsParseGeneralConfigByJson(pRoot, &stGeneralCfg))
    {
        json_object_put(pRoot);
        return -1;
    }
    json_object_put(pRoot);

    log_i("get event report level[%d]", stGeneralCfg.eventReportLevel);
    *pEventReportLevel = stGeneralCfg.eventReportLevel;
    return 0;
}
// 
// D32S adm_cfg_iniParserGetInt(const DCHAR* key, D32S notfound)
// {
//     return 0;
// }

D64S adm_irm_getEventSequenceId()
{
    static D64S sequenceId = 0;
    ++sequenceId;
    return sequenceId;
}

D32S adm_irm_reportEvent(const DCHAR* sessionId,
    D64S taskId,
    D64S strategyId,
    D64S timestamp,
    const DCHAR* sdkVersion,
    const DCHAR* extsReqJsonStr,
    ADM_OCS_EVENTS_INFO_LIST_T* eventsList)
{
    ADM_OCS_EVENTS_INFO_T* pEventInfo = NULL;

    for (D32S i = 0; i < eventsList->reportEventsLen; i++)
    {
        pEventInfo = &eventsList->reportEvents[i];
        log_i("report event event id[%s] event level[%d] event context[%s] event desc[%s]",
            pEventInfo->eventId, pEventInfo->eventLevel, pEventInfo->eventContext, pEventInfo->eventDesc);
    }

    return 0;
}

D32S adm_dim_ucl_readPartInformation(const D16U instanceId, const D16U timeout,
    const ADM_DIM_SOA_ECU_IDEN_INFO_T* pstEcuIdenInfo,
    const ADM_DIM_SOA_DID_DATA_ARRAY_T* didArrayRequest,
    ADM_DIM_SOA_DID_DATA_ARRAY_T** didListResponse, D32S* errorCode)
{
    D32S iRet = -1;
    log_i("instance id[%u]", instanceId);

    iRet = getEcuInfoDidsByRule(pstEcuIdenInfo->requestId, pstEcuIdenInfo->swDid, didArrayRequest, didListResponse);
    *errorCode = iRet;

    return 0;
}

D32S adm_dim_vdi_readDiagnosticInfo(const D16U instanceId, const D16U timeout,
    const ADM_DIM_SOA_ECU_IDEN_INFO_T* pstEcuIdenInfo,
    const DCHAR* diagScriptPath, const ADM_DIM_SOA_VDI_BUS_TYPE_E eBusType,
    const ADM_DIM_SOA_PIN_CHANNEL_TYPE_E eChannelType, const DBOOL isCGWRouted,
    const ADM_DIM_SOA_DID_DATA_ARRAY_T didListRequest,
    ADM_DIM_SOA_DID_DATA_ARRAY_T** didListResponse, D32S* errorCode)
{
    D32S iRet = -1;

    do
    {
        if (NULL == diagScriptPath || '\0' == diagScriptPath[0])
        {
            log_e("invalid diagScriptPath[%s]", diagScriptPath);
            break;
        }

        if (eBusType < 1 || eBusType > 4)
        {
            log_e("invalid busType[%d]", eBusType);
            break;
        }

        if (eChannelType < 1 || eChannelType > 3)
        {
            log_e("invalid channelType[%d]", eChannelType);
            break;
        }

        if (isCGWRouted < 1 || isCGWRouted > 2)
        {
            log_e("invalid isCGWRouted[%d]", isCGWRouted);
            break;
        }

        iRet = getEcuInfoDidsByRule(pstEcuIdenInfo->requestId, pstEcuIdenInfo->swDid, &didListRequest, didListResponse);
    } while (FALSE);

    *errorCode = iRet;

    return 0;
}

D32S adm_dim_host_setVehicleBigVersion(const D16U instanceId, const D16U timeout,
    const DCHAR* vehicleBigVersion, D32S* errorCode)
{
    if (NULL != vehicleBigVersion)
    {
        log_i("set vehicle big version[%s] sucessful", vehicleBigVersion);
        *errorCode = 0;
    }
    else
    {
        log_e("set vehicle big version failed");
        *errorCode = -1;
    }
    
    return *errorCode;
}

D32S adm_dim_host_getVehicleBigVersion(const D16U instanceId, const D16U timeout,
    DCHAR** vehicleBigVersion, D32S* errorCode)
{
    D32S    iRet = -1;
    DCHAR acBuffer[1024];

    if (-1 == testReadCnfFile("VehicleInfo.json", acBuffer, sizeof(acBuffer)))
    {
        return -1;
    }

    struct json_object* pRoot = json_tokener_parse(acBuffer);
    if (NULL != pRoot)
    {
        const DCHAR* pBigVersion = json_object_get_string(json_object_object_get(pRoot, "BigVersion"));
        if (NULL != pBigVersion)
        {
            *vehicleBigVersion = adm_strdup(pBigVersion);
            iRet = 0;
        }
        else
        {
            log_e("not configure vehicle big version in [VehicleInfo.json]");
        }
    }
    else
    {
        log_e("not find vehicle big version in [VehicleInfo.json]");
    }

    return iRet;
}

D16U adm_soa_getHostInstanceId()
{
    return 0;
}

struct json_object* testGetTestcaseParam()
{
    DCHAR acBuffer[1024];
    if (-1 == testReadCnfFile("param.json", acBuffer, sizeof(acBuffer)))
    {
        return NULL;
    }

    struct json_object* pRoot = json_tokener_parse(acBuffer);
    if (NULL == pRoot)
    {
        log_e("parse param.json failed!");
    }

    return pRoot;
}


void TestGetVehicleBigVersion()
{
    DCHAR* pBigVersion;
    D32S    iRet = -1;

    iRet = adm_eim_getVehicleBigVersion(&pBigVersion);
    if (0 == iRet)
    {
        log_i("get vehicle big version[%s] successful!", pBigVersion);
    }
    else
    {
        log_e("get vehicle big version failed!");
    }
}

void TestSetVehicleBigVersion()
{
    struct json_object* pRoot = testGetTestcaseParam();
    if (NULL == pRoot)
    {
        return;
    }

    const char* pBigVersion = json_object_get_string(json_object_object_get(pRoot, "bigVersion"));
    D32S iRet = adm_eim_setVehicleBigVersion(pBigVersion);
    if (0 == iRet)
    {
        log_i("set vehicle big version[%s] successful!", pBigVersion);
    }
    else
    {
        log_e("set vehicle big version[%s] failed!", pBigVersion);
    }

    json_object_put(pRoot);
}

void TestGetSingleEcuInfo()
{
    struct json_object* pRoot = testGetTestcaseParam();
    if (NULL == pRoot)
    {
        return;
    }

    const DCHAR* pEcuReqId = json_object_get_string(json_object_object_get(pRoot, "ecuReqId"));
    const DCHAR* pEcuSwDid = json_object_get_string(json_object_object_get(pRoot, "ecuSwDid"));
    if (NULL == pEcuReqId || NULL == pEcuSwDid)
    {
        log_e("not find ecuReqId or ecuSwDid in param.json");
        json_object_put(pRoot);
        return;
    }

    D32S iRet = -1;
    ADM_ECS_ECUCONFIG_T* pstEcuCfg = NULL;
    iRet = adm_ecs_getEcuConfig((DCHAR*)pEcuReqId, (DCHAR*)pEcuSwDid, &pstEcuCfg);
    if (0 != iRet)
    {
        log_e("not find ecu info from ecs. ecuReqId[%s] ecuSwDid[%s]", pEcuReqId, pEcuSwDid);
        json_object_put(pRoot);
        return;
    }

    testEcuGetCtrlsInit(1U);
    testEcuGetCtrlAddByJson(pRoot);

    json_object_put(pRoot);

    ADM_ECS_RSCONFIGINFO_T stRsCfgInfo = { 0 };
    iRet = adm_ecs_getConfigInfo(&stRsCfgInfo);
    if (0 != iRet)
    {
        log_e("not find ecu config from ecs");
        testEcuGetCtrlsDestroy();
        return;
    }

    ADM_DIM_SOA_ECU_IDEN_INFO_T stEcuIdenInfo = { 0 };
    stEcuIdenInfo.requestId = (D32S)htoi(pstEcuCfg->ecuReqId);
    stEcuIdenInfo.responseId = (D32S)htoi(pstEcuCfg->ecuResponseId);
    stEcuIdenInfo.functionId = (D32S)htoi(pstEcuCfg->ecuFunctionId);
    stEcuIdenInfo.swDid = (D32S)htoi(pstEcuCfg->ecuSwDid);


    ECU_INFO_T* pEcuInfo = NULL;
     iRet = adm_eim_getSingleEcuInfo(pstEcuCfg, &stRsCfgInfo.stGeneralCfg.stEcuDidList, &stEcuIdenInfo,
         stRsCfgInfo.stGeneralCfg.stBusinessScript.filePath, &pEcuInfo);
    if (0 == iRet && NULL != pEcuInfo)
    {
        DCHAR* pEcuInfoStr = (DCHAR*)adm_eim_singleEcuInfoJsonStr(pEcuInfo);
        log_i("get single ecu info successful! ecu info[%s]", pEcuInfoStr);
        adm_free(pEcuInfoStr);
    }
    else
    {
        log_e("get single ecu info failed!");
    }

    testEcuGetCtrlsDestroy();
}

void TestGetEcuInfos()
{
    struct json_object* pRoot = testGetTestcaseParam();
    if (NULL == pRoot)
    {
        return;
    }

    D32S iRet = -1;

    testEcuGetCtrlsInitByJson(json_object_object_get(pRoot, "ecusGetCtrls"));

    json_object_put(pRoot);

    ADM_ECS_RSCONFIGINFO_T stRsCfgInfo = { 0 };
    iRet = adm_ecs_getConfigInfo(&stRsCfgInfo);
    if (0 != iRet)
    {
        log_e("not find ecu config from ecs");
        testEcuGetCtrlsDestroy();
        return;
    }

    DCHAR* pEcuInfosStr = NULL;
    DSIZE uiEcuInfosStrSize = 0;
    ADM_KV_STORAGE_VALUE_TYPE_E type = ADM_KV_STORAGE_VALUE_TYPE_INVALID;
    adm_com_kvstorage_get_string(adm_core_service_getBusinessDatabaseDb(), "[EIM]_[allEcuInfoStr]", &pEcuInfosStr, &uiEcuInfosStrSize, &type);
    log_i("get all ecus info, %s: %s", "[EIM]_[allEcuInfoStr]", pEcuInfosStr);

    ADM_EIM_GETECUINFO_T ecuInfos;
    iRet = adm_eim_getEcuInfoForEcu(&stRsCfgInfo, &ecuInfos);
    if (0 == iRet)
    {
        log_i("get single ecu info successful!");
    }
    else
    {
        log_e("set single ecu info failed!");
    }

    testEcuGetCtrlsDestroy();
}

void TestEcuInfoCycleCollection()
{
    struct json_object* pRoot = testGetTestcaseParam();
    D32U    uiWaitTime = 30;
    if (NULL != pRoot)
    {
        uiWaitTime = json_object_get_int(json_object_object_get(pRoot, "waitTime"));
    }

    testEcuGetCtrlsInitByJson(json_object_object_get(pRoot, "ecusGetCtrls"));

    adm_eim_getEcuInfoManagerInit();

    DCHAR* pEcuInfosStr = NULL;
    DSIZE uiEcuInfosStrSize = 0;
    ADM_KV_STORAGE_VALUE_TYPE_E type = ADM_KV_STORAGE_VALUE_TYPE_INVALID;
    adm_com_kvstorage_get_string(adm_core_service_getBusinessDatabaseDb(), "[EIM]_[allEcuInfoStr]", &pEcuInfosStr, &uiEcuInfosStrSize, &type);
    log_i("get all ecus info, [EIM]_[allEcuInfoStr]: %s", pEcuInfosStr);

    adm_eim_getEcuInfoManagerCycleCollection();

    D64U uiStartTime = time(NULL);
    while (TRUE)
    {
        usleep(100000);
        D64U uiNow = time(NULL);
        if (uiNow - uiStartTime > uiWaitTime)
        {
            adm_eim_getEcuInfoManagerDeInit();

            pid_t pid = getpid();
            DCHAR acCmd[256];
            DCHAR acResult[64];
            adm_snprintf(acCmd, sizeof(acCmd), "ps -efT | awk '{print $2}' | grep %d | wc -l", pid);

            do 
            {
                FILE* pStream = popen(acCmd, "r");
                fgets(acResult, sizeof(acResult), pStream);
                pclose(pStream);
                if (atoi(acResult) <= 2)
                {
                    break;
                }
                usleep(100000);
            } while (TRUE);
            break;
        }
    }

    testEcuGetCtrlsDestroy();
}



#include <sys/types.h>
#include <dirent.h>
typedef struct
{
    DIR* dir;
    DCHAR srcPath[256];
    DCHAR childPath[256];
} TEST_DIR_READER_T;

D32S test_dir_reader_init(TEST_DIR_READER_T* reader, const DCHAR* path)
{
    D32S ret = ADM_ECODE_FAILED;
    DIR* dir = NULL;

    dir = opendir(path);
    if (NULL != dir)
    {
        adm_strcpy(reader->srcPath, path, sizeof(reader->srcPath));
        ret = ADM_ECODE_SUCCESSED;
    }

    reader->dir = dir;
    return ret;
}

D32S test_dir_reader_next_dir(TEST_DIR_READER_T* reader, DCHAR* dir, D32S dirMaxSize)
{
    D32S ret = ADM_ECODE_FAILED;
    struct dirent* entry = NULL;

    if (NULL != reader->dir)
    {
        while ((entry = readdir(reader->dir)) != NULL)
        {
            if (0 == strcmp(entry->d_name, ".") || 0 == strcmp(entry->d_name, ".."))
            {
                continue;
            }

            if (entry->d_type == DT_DIR)
            {
                adm_snprintf(dir, dirMaxSize, "%s/%s", reader->srcPath, entry->d_name);
                adm_strcpy(reader->childPath, entry->d_name, sizeof(reader->childPath));
                ret = ADM_ECODE_SUCCESSED;
                break;
            }
        }
    }

    return ret;
}

void test_dir_reader_destroy(TEST_DIR_READER_T* reader)
{
    closedir(reader->dir);
}




typedef void (*TestcaseWorker)();

void runTestcase(const DCHAR* pCaseName, TestcaseWorker fnWorker)
{
    TEST_DIR_READER_T dirReader;
    DCHAR acDir[256];
    DCHAR acCnfDir[1024];

    adm_snprintf(acCnfDir, sizeof(acCnfDir), "testcase/%s", pCaseName);

    dirent** pDirlist;
    D32S iCount = scandir(acCnfDir, &pDirlist, NULL, alphasort);
    if (iCount < 0)
    {
        log_e("not find testcase directory[%s]", acCnfDir);
        return;
    }

    dirent* pDir = NULL;
    for (D32S i = 0; i < iCount; i++)
    {
        pDir = pDirlist[i];
        if (DT_DIR != pDir->d_type)
        {
            continue;
        }

        if (0 == strcmp(pDir->d_name, ".") || 0 == strcmp(pDir->d_name, ".."))
        {
            continue;
        }

        adm_snprintf(acDir, sizeof(acDir), "%s/%s", acCnfDir, pDir->d_name);

        log_i("------------------------run testcase[%s::%s] begin------------------------", pCaseName, pDir->d_name);
        testSetConfigDir(acDir);
        fnWorker();

        log_i("------------------------run testcase[%s::%s] end------------------------", pCaseName, pDir->d_name);
    }

    adm_free(pDirlist);
}
// 
// void runTestcase(const DCHAR* pCaseName, TestcaseWorker fnWorker)
// {
//     TEST_DIR_READER_T dirReader;
//     DCHAR acDir[256];
//     DCHAR acCnfDir[1024];
// 
//     adm_snprintf(acCnfDir, sizeof(acCnfDir), "testcase/%s", pCaseName);
// 
//     if (0 != test_dir_reader_init(&dirReader, acCnfDir))
//     {
//         log_e("not find testcase[%s] directory!", pCaseName);
//         return;
//     }
// 
//     do
//     {
//         if (ADM_ECODE_SUCCESSED != test_dir_reader_next_dir(&dirReader, acDir, sizeof(acDir)))
//         {
//             break;
//         }
// 
//         log_i("------------------------run testcase[%s::%s] begin------------------------", pCaseName, dirReader.childPath);
//         testSetConfigDir(acDir);
//         fnWorker();
// 
//         log_i("------------------------run testcase[%s::%s] end------------------------", pCaseName, dirReader.childPath);
//     } while (TRUE);
// 
//     test_dir_reader_destroy(&dirReader);
// }

#ifdef __cplusplus
}
#endif


class CEIMTest:public testing::Test
{
protected:
    void SetUp() override 
    {
    }
    void TearDown() override 
    {
    }
};

TEST_F(CEIMTest, GetVehicleBigVersion)
{
    runTestcase("GetVehicleBigVersion", TestGetVehicleBigVersion);
}

TEST_F(CEIMTest, SetVehicleBigVersion)
{
    runTestcase("SetVehicleBigVersion", TestSetVehicleBigVersion);
}

TEST_F(CEIMTest, GetSingleEcuInfo)
{
    runTestcase("GetSingleEcuInfo", TestGetSingleEcuInfo);
}

TEST_F(CEIMTest, GetEcuInfos)
{
    runTestcase("GetEcuInfos", TestGetEcuInfos);
}

TEST_F(CEIMTest, EcuInfoCycleCollection)
{
    runTestcase("EcuInfoCycleCollection", TestEcuInfoCycleCollection);
}



int main()
{
//     D32S c = 0;
//     D32S endMark =0;
//     DCHAR* otaConfigPath = NULL;
// 
//     if (0 != adm_irmgtest_testEnvironmentInit())
//     {   
//         return 0;
//     }
//     
//     //!< 接收入参
//     while ((c = getopt(argc, argv, "c:")) != -1)
//     {
//         switch (c)
//         {
//             case (int)'c' :
//                 otaConfigPath = optarg;
//                 (void)printf("optarg:%s\n", optarg);
//                 break;
//             case (int)'?':
//             {
//                 if ((char)optopt == 'o')
//                 {
//                     (void)fprintf(stderr, "Option -%c requires an argument.\n", optopt);
//                 }
//                 else if (0u != isprint((unsigned char)optopt))
//                 {
//                     (void)fprintf(stderr, "Unknown option `-%c'.\n", optopt);
//                 }
//                 else
//                 {
//                     (void)fprintf(stderr, "Unknown option character `\\x%x'.\n", optopt);
//                 }
//                 /* unknown or wrong option used, show usage information and terminate */
//                 endMark =-1;
//                 usage();
//                 break;
//             }
//             default:
//             {
//                 endMark =-1;
//                 abort();
//                 break;
//             }
//         }
//         if(-1 == endMark)
//         {
//             return -1;
//         }
//     }
//     
//     /** 1. 解析otaconfig.ini 配置文件 */
//     if (ADM_ECODE_SUCCESSED != adm_apc_otaConfigInit(otaConfigPath)) 
//     {
//         (void)printf("adm_apc_otaConfigInit failed, iRet:%d\n", adm_apc_otaConfigInit(otaConfigPath));
//         // 配置文件初始化失败，可以使用每个字段的默认配置，因此不退出程序；
//     }
//     else 
//     {
//         (void)printf("adm_apc_otaConfigInit successed\n");
//     }

    adm_com_createDir("./debug");
    adm_com_createDir("./debug/kvStorage");
    adm_com_createDir("./debug/log");
    // adm_debug_logInit("./debug/", (1 * 1024 * 1024), 5);
    adm_debug_logInitWithFileName("./debug/log", "admfotaapp", 5 * 1024 * 1024, 5, ELOG_LVL_VERBOSE);

    adm_core_service_initRuntimeDir(ADM_CORE_SERVICE_RUNTIME_DIR_TYPE_KV_STORAGE, "./debug/kvStorage");
    adm_core_service_init();

    ::testing::InitGoogleTest();
    return RUN_ALL_TESTS();
}