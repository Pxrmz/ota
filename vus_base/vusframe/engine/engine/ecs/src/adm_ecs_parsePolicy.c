/**
 * @file    adm_ecs_parsePolicy.c
 * @brief   ecs module policy parse  
 * @details ecs模块策略解析
 * @author  wangyanpeng
 * @version v0.1.0
 * @date    2020-11-06
 * @copyright www.abupdate.com
 */
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include "adm_file.h"
#include "adm_typedefs.h"
#include "adm_memory.h"
#include "adm_string.h"
#include "adm_com_errorCode.h"

#include "adm_ecs_log.h"
#include "adm_com_lib.h"
#include "adm_ecs_rsConfig.h"
#include "adm_com_modDefine.h"
#include "adm_ecs_parsePolicy.h"
#include "adm_com_parseFileObject.h"

#include "adm_iss_textRsa2048.h"
#include "adm_ocs_airProtocolAdapter.h"
#include "adm_ocs_airProtocolSecurity.h"
#include "adm_configParse.h"
#include "adm_ecs_kvStorage.h"


#if 0
#endif


/*!
 *******************************************************************************
 * 申请ecs算法库列表结构内存.
 *
 * \note 申请的为连续内存
 *
 * \param[in]  num 算法库个数
 * \retval     ADM_ECS_SALIBINFO_T*
 *******************************************************************************
 */
ADM_ECS_SALIBINFO_T* adm_ecs_newSaLibList(D32S num)
{
    ADM_ECS_SALIBINFO_T *pstSaLibInfo = NULL;

    if (0 >= num)
    {
        return NULL;
    }

    pstSaLibInfo = (ADM_ECS_SALIBINFO_T *)adm_malloc((D32U)num * sizeof(*pstSaLibInfo));
    if (NULL != pstSaLibInfo)
    {
        (void)adm_memzero(pstSaLibInfo, (D32U)num * sizeof(*pstSaLibInfo));
    }

    return pstSaLibInfo;
}

/*!
 *******************************************************************************
 * 释放ecs算法库列表结构内存.
 *
 * \note    释放的为连续内存
 *
 * \param[in]  num 算法库个数
 * \param[in]  pstSaLibInfo 算法库列表信息
 * \retval     none
 *******************************************************************************
 */
void adm_ecs_delSaLibList
(
    D32S num, 
    ADM_ECS_SALIBINFO_T **pstSaLibInfo
)
{
    D32S i = 0;

    if ((0 >= num) || (ADM_IS_NULL(pstSaLibInfo)) || (ADM_IS_NULL(*pstSaLibInfo)))
    {
        return;
    }

    for (i = 0; i < num; i++)
    {
        adm_com_cleanFileObject(&(*pstSaLibInfo)->stSaLibFile);
    }
    adm_free(*pstSaLibInfo);

    return;
}

void adm_ecs_delSaMaskList
(
    ADM_ECS_SA_MASK_LIST_T** ppstSaMaskList
)
{
    if (NULL != ppstSaMaskList && NULL != *ppstSaMaskList)
    {
        ADM_ECS_SA_MASK_LIST_T* pSaMaskList = *ppstSaMaskList;

        for (D16U i = 0; i < pSaMaskList->num; i++)
        {
            adm_free(pSaMaskList->saMaskInfo[i].saMaskValue);
        }

        adm_free(pSaMaskList->saMaskInfo);
        pSaMaskList->num = 0;
        adm_free(*ppstSaMaskList);
    }

    return;
}

/*!
 *******************************************************************************
 * 申请ecs安全算法库编号列表结构内存.
 *
 * \note 申请的为连续内存
 *
 * \param[in]  num 算法库编号个数
 * \retval     D32S*
 *******************************************************************************
 */
D32S* adm_ecs_newSaLibNumberList(D32S num)
{
    D32S *pstSaLibNumberList = NULL;

    if (0 >= num)
    {
        return NULL;
    }

    pstSaLibNumberList = (D32S *)adm_malloc((D32U)num * sizeof(*pstSaLibNumberList));
    if (NULL != pstSaLibNumberList)
    {
        (void)adm_memzero(pstSaLibNumberList, (D32U)num * sizeof(*pstSaLibNumberList));
    }

    return pstSaLibNumberList;
}

/*!
 *******************************************************************************
 * 释放ecs算法库编号列表结构内存.
 *
 * \note    释放的为连续内存
 *
 * \param[in]  num 算法库编号个数
 * \param[in]  pstSaLibNumberList 算法库编号列表信息
 * \retval     none
 *******************************************************************************
 */
void adm_ecs_delSaLibNumberList
(
    D32S  num, 
    D32S **pstSaLibNumberList
)
{

    if ((0 >= num) || (ADM_IS_NULL(pstSaLibNumberList)) || (ADM_IS_NULL(*pstSaLibNumberList)))
    {
        return;
    }

    adm_free(*pstSaLibNumberList);

    return;
}

/*!
 *******************************************************************************
 * 申请ecs CRC算法库编号列表结构内存.
 *
 * \note 申请的为连续内存
 *
 * \param[in]  num 算法库编号个数
 * \retval     D32S*
 *******************************************************************************
 */
D32S* adm_ecs_newCrcLibNumberList(D32S num)
{
    D32S *pstCrcLibNumberList = NULL;

    if (0 >= num)
    {
        return NULL;
    }

    pstCrcLibNumberList = (D32S *)adm_malloc((D32U)num * sizeof(*pstCrcLibNumberList));
    if (NULL != pstCrcLibNumberList)
    {
        (void)adm_memzero(pstCrcLibNumberList, (D32U)num * sizeof(*pstCrcLibNumberList));
    }

    return pstCrcLibNumberList;
}

/*!
 *******************************************************************************
 * 释放ecs CRC算法库编号列表结构内存.
 *
 * \note    释放的为连续内存
 *
 * \param[in]  num 算法库编号个数
 * \param[in]  pstCrcLibNumberList CRC算法库编号列表信息
 * \retval     none
 *******************************************************************************
 */
void adm_ecs_delCrcLibNumberList
(
    D32S  num, 
    D32S **pstCrcLibNumberList
)
{

    if ((0 >= num) || (ADM_IS_NULL(pstCrcLibNumberList)) || (ADM_IS_NULL(*pstCrcLibNumberList)))
    {
        return;
    }

    adm_free(*pstCrcLibNumberList);

    return;
}


/*!
 *******************************************************************************
 * 申请crc算法库列表结构内存.
 *
 * \note 申请的为连续内存
 *
 * \param[in]  num 算法库个数
 * \retval     ADM_ECS_CRCLIBINFO_T*
 *******************************************************************************
 */
ADM_ECS_CRCLIBINFO_T* adm_ecs_newCrcLibList(D32S num)
{
    ADM_ECS_CRCLIBINFO_T *pstCrcLibInfo = NULL;

    if (0 >= num)
    {
        return NULL;
    }

    pstCrcLibInfo = (ADM_ECS_CRCLIBINFO_T *)adm_malloc((D32U)num * sizeof(*pstCrcLibInfo));
    if (NULL != pstCrcLibInfo)
    {
        (void)adm_memzero(pstCrcLibInfo, (D32U)num * sizeof(*pstCrcLibInfo));
    }

    return pstCrcLibInfo;
}

/*!
 *******************************************************************************
 * 释放crc算法库列表结构内存.
 *
 * \note    释放的为连续内存
 *
 * \param[in]  num           算法库个数
 * \param[in]  pstCrcLibInfo 算法库列表信息
 * \retval     none
 *******************************************************************************
 */
void adm_ecs_delCrcLibList
(
    D32S num, 
    ADM_ECS_CRCLIBINFO_T **pstCrcLibInfo
)
{
    D32S i = 0;

    if ((0 >= num) || (ADM_IS_NULL(pstCrcLibInfo)) || (ADM_IS_NULL(*pstCrcLibInfo)))
    {
        return;
    }

    for (i = 0; i < num; i++)
    {
        adm_com_cleanFileObject(&(*pstCrcLibInfo)->stCrcLibFile);
    }
    adm_free(*pstCrcLibInfo);

    return;
}


/*!
 *******************************************************************************
 * 申请ecs刷写脚本列表结构内存.
 *
 * \note 申请的为连续内存
 *
 * \param[in]  num 刷写脚本个数
 * \retval     ADM_ECS_FLASHSCRIPTINFO_T*
 *******************************************************************************
 */
ADM_ECS_FLASHSCRIPTINFO_T* adm_ecs_newFlashScriptList(D32S num)
{
    ADM_ECS_FLASHSCRIPTINFO_T *pstFlashScriptInfo = NULL;

    if (0 >= num)
    {
        return NULL;
    }

    pstFlashScriptInfo = (ADM_ECS_FLASHSCRIPTINFO_T *)adm_malloc((D32U)num * sizeof(*pstFlashScriptInfo));
    if (NULL != pstFlashScriptInfo)
    {
        (void)adm_memzero(pstFlashScriptInfo, (D32U)num * sizeof(*pstFlashScriptInfo));
    }

    return pstFlashScriptInfo;
}

/*!
 *******************************************************************************
 * 释放ecs刷写脚本列表结构内存.
 *
 * \note    释放的为连续内存
 *
 * \param[in]  num 刷写脚本个数
 * \param[in]  pstFlashScriptInfo 刷写脚本列表信息
 * \retval     none
 *******************************************************************************
 */
void adm_ecs_delFlashScriptList
(
    D32S num, 
    ADM_ECS_FLASHSCRIPTINFO_T **pstFlashScriptInfo
)
{
    D32S i = 0;

    if ((0 >= num) || (ADM_IS_NULL(pstFlashScriptInfo)) || (ADM_IS_NULL(*pstFlashScriptInfo)))
    {
        return;
    }

    for (i = 0; i < num; i++)
    {
        adm_com_cleanFileObject(&(*pstFlashScriptInfo)->stFlashScriptFile);
    }
    adm_free(*pstFlashScriptInfo);

    return;
}

/*!
 *******************************************************************************
 * 申请ecs业务脚本列表结构内存.
 *
 * \note 申请的为连续内存
 *
 * \param[in]  num 业务脚本个数
 * \retval     ADM_ECS_BUSINESSSCRIPTINFO_T*
 *******************************************************************************
 */
ADM_ECS_BUSINESSSCRIPTINFO_T* adm_ecs_newBusinessScriptList(D32S num)
{
    ADM_ECS_BUSINESSSCRIPTINFO_T *pstBusinessScriptInfo = NULL;

    if (0 >= num)
    {
        return NULL;
    }

    pstBusinessScriptInfo = (ADM_ECS_BUSINESSSCRIPTINFO_T *)adm_malloc((D32U)num * sizeof(*pstBusinessScriptInfo));
    if (NULL != pstBusinessScriptInfo)
    {
        (void)adm_memzero(pstBusinessScriptInfo, (D32U)num * sizeof(*pstBusinessScriptInfo));
    }

    return pstBusinessScriptInfo;
}

/*!
 *******************************************************************************
 * 释放ecs业务脚本列表结构内存.
 *
 * \note    释放的为连续内存
 *
 * \param[in]  num 业务脚本个数
 * \param[in]  pstBusinessScriptInfo 业务脚本列表信息
 * \retval     none
 *******************************************************************************
 */
void adm_ecs_delBusinessScriptList
(
    D32S num, 
    ADM_ECS_BUSINESSSCRIPTINFO_T **pstBusinessScriptInfo
)
{
    D32S i = 0;

    if ((0 >= num) || (ADM_IS_NULL(pstBusinessScriptInfo)) || (ADM_IS_NULL(*pstBusinessScriptInfo)))
    {
        return;
    }

    for (i = 0; i < num; i++)
    {
        adm_com_cleanFileObject(&(*pstBusinessScriptInfo)->stBusinessScriptFile);
    }
    adm_free(*pstBusinessScriptInfo);

    return;
}



/*!
 *******************************************************************************
 * 申请ecu did列表结构内存.
 *
 * \note 申请的为连续内存
 *
 * \param[in]  num 资源id个数
 * \retval     ADM_ECS_ECUDID_T*
 *******************************************************************************
 */
ADM_ECS_ECUDID_T* adm_ecs_newEcuDidList(D32S num)
{
    ADM_ECS_ECUDID_T *pstEcuDidInfo = NULL;

    if (0 >= num)
    {
        return NULL;
    }

    pstEcuDidInfo = (ADM_ECS_ECUDID_T *)adm_malloc((D32U)num * sizeof(*pstEcuDidInfo));
    if (NULL != pstEcuDidInfo)
    {
        (void)adm_memzero(pstEcuDidInfo, (D32U)num * sizeof(*pstEcuDidInfo));
    }

    return pstEcuDidInfo;
}

void adm_ecs_delExternEcuInfo
(
    ADM_ECS_EXTERN_ECU_INFO_T** ppstExternEcuInfo
)
{
    ADM_ECS_EXTERN_ECU_INFO_T* pstExternEcuInfo = NULL;
    if (ADM_IS_NULL(ppstExternEcuInfo) || ADM_IS_NULL(*ppstExternEcuInfo))
    {
        log_e("adm_ecs_delExternEcuInfo error.");
        return;
    }

    pstExternEcuInfo = *ppstExternEcuInfo;

    adm_free(pstExternEcuInfo->partHardwareNum);
    adm_free(pstExternEcuInfo->partName);
    adm_free(pstExternEcuInfo->partNumber);
    adm_free(pstExternEcuInfo->partSupplier);
    adm_free(pstExternEcuInfo->partSwid);
    adm_free(pstExternEcuInfo->softwarePartNumber);
    adm_free(pstExternEcuInfo->upgradeOrder);

    adm_free(*ppstExternEcuInfo);

    return;
}

/*!
 *******************************************************************************
 * 释放ecu资源id列表结构内存.
 *
 * \note    释放的为连续内存
 *
 * \param[in]  num           资源id个数
 * \param[in]  pstEcuDidInfo 资源id列表信息
 * \retval     none
 *******************************************************************************
 */
void adm_ecs_delEcuDidList
(
    D32S              num, 
    ADM_ECS_ECUDID_T **pstEcuDidInfo
)
{
    D32S i = 0;

    if ((0 >= num) || (ADM_IS_NULL(pstEcuDidInfo)) || (ADM_IS_NULL(*pstEcuDidInfo)))
    {
        return;
    }

    for (i = 0; i < num; i++)
    {
        adm_free((*pstEcuDidInfo)[i].did);
    }
    adm_free(*pstEcuDidInfo);

    return;
}


/*!
 *******************************************************************************
 * 申请ecs ecus配置列表结构内存.
 *
 * \note 申请的为连续内存
 *
 * \param[in]  num ecus配置个数
 * \retval     ADM_ECS_ECUCONFIG_T*
 *******************************************************************************
 */
ADM_ECS_ECUCONFIG_T* adm_ecs_newEcusConfigList(D32S num)
{
    ADM_ECS_ECUCONFIG_T *pstEcuList = NULL;

    if (0 >= num)
    {
        log_e("Invalid input param");
        return NULL;
    }

    pstEcuList = (ADM_ECS_ECUCONFIG_T *)adm_malloc((D32U)num * sizeof(*pstEcuList));
    if (NULL != pstEcuList)
    {
        (void)adm_memzero(pstEcuList, (D32U)num * sizeof(*pstEcuList));
    }

    return pstEcuList;
}

/*!
 *******************************************************************************
 * 释放ecs ecus配置列表结构内存.
 *
 * \note    释放的为连续内存
 *
 * \param[in]  num        ecus配置个数
 * \param[in]  pstEcuList ecus配置列表信息
 * \retval     none
 *******************************************************************************
 */
void adm_ecs_delEcusConfigList
(
    D32S num, 
    ADM_ECS_ECUCONFIG_T **pstEcuList
)
{
    D32S i = 0;

    if ((0 >= num) || (ADM_IS_NULL(pstEcuList)) || (ADM_IS_NULL(*pstEcuList)))
    {
        return;
    }

    for (i = 0; i < num; i++)
    {
        adm_free((*pstEcuList)[i].ecuReqId);
        adm_free((*pstEcuList)[i].ecuSwDid);
        adm_free((*pstEcuList)[i].ecuResponseId);
        adm_free((*pstEcuList)[i].ecuFunctionId);
        adm_free((*pstEcuList)[i].readInfoInstanceId);
        adm_free((*pstEcuList)[i].downloadInstanceId);        
        adm_free((*pstEcuList)[i].transferInstanceId);
        adm_free((*pstEcuList)[i].installInstanceId);
        adm_free((*pstEcuList)[i].saMask);
        adm_free((*pstEcuList)[i].stExternEcuInfo.partName);
        adm_free((*pstEcuList)[i].stExternEcuInfo.partNumber);
        adm_free((*pstEcuList)[i].stExternEcuInfo.partSwid);
        adm_free((*pstEcuList)[i].stExternEcuInfo.softwarePartNumber);
        adm_free((*pstEcuList)[i].stExternEcuInfo.partSupplier);
        adm_free((*pstEcuList)[i].stExternEcuInfo.partHardwareNum);
        adm_free((*pstEcuList)[i].stExternEcuInfo.upgradeOrder);

        adm_com_cleanExtsList(&(*pstEcuList)[i].stExtList);
        adm_ecs_delSaMaskList(&(*pstEcuList)[i].saMaskList);
        adm_ecs_delSaLibNumberList((*pstEcuList)[i].saLibNumberList.num, 
                &(*pstEcuList)[i].saLibNumberList.psaLibNumber);
        adm_ecs_delCrcLibNumberList((*pstEcuList)[i].crcLibNumberList.num, 
                &(*pstEcuList)[i].crcLibNumberList.pcrcLibNumber);
        //!< 该ecu特有的ResourceDid信息
        adm_ecs_delEcuDidList((*pstEcuList)[i].stEcuDidList.num, 
                &(*pstEcuList)[i].stEcuDidList.pstEcuDidInfo);
    }
    adm_free(*pstEcuList);

    return;
}


/*!
 *******************************************************************************
 * 清空ecs 通用配置结构内存.
 *
 * \note 不释放最外层的 pstGeneralConfig 结构内存
 *
 * \param[in]  pstGeneralConfig 配置配置结构信息
 * \retval     none
 *******************************************************************************
 */
void adm_ecs_cleanGeneralConfig
(
    ADM_ECS_GENERALCONFIG_T *pstGeneralConfig
)
{
    if (ADM_IS_NULL(pstGeneralConfig))
    {
        return;
    }

    adm_free(pstGeneralConfig->protocolVersion);
    adm_free(pstGeneralConfig->logReportUrl);
    adm_ecs_delSaLibList(pstGeneralConfig->stSaLibList.num, 
            &pstGeneralConfig->stSaLibList.pstSaLibInfo);
    adm_ecs_delCrcLibList(pstGeneralConfig->stCrcLibList.num, 
            &pstGeneralConfig->stCrcLibList.pstCrcLibInfo);
    adm_ecs_delFlashScriptList(pstGeneralConfig->stflashScriptList.num, 
            &pstGeneralConfig->stflashScriptList.pstFlashScriptInfo);
    adm_ecs_delBusinessScriptList(pstGeneralConfig->stBusinessScriptList.num, 
            &pstGeneralConfig->stBusinessScriptList.pstBusinessScriptInfo);
    adm_ecs_delEcuDidList(pstGeneralConfig->stEcuDidList.num, 
            &pstGeneralConfig->stEcuDidList.pstEcuDidInfo);
    adm_com_cleanFileObject(&pstGeneralConfig->stBusinessScript);
    adm_com_cleanExtsList(&pstGeneralConfig->stExtList);
    
    pstGeneralConfig->stSaLibList.num       = 0;
    pstGeneralConfig->stCrcLibList.num      = 0;
    pstGeneralConfig->stflashScriptList.num = 0;
    pstGeneralConfig->stEcuDidList.num      = 0;
    pstGeneralConfig->stBusinessScriptList.num = 0;

    return;
}


/*!
 *******************************************************************************
 * 清空ecs part配置结构内存.
 *
 * \note    不释放最外层的 pstPartCfg 结构内存
 *
 * \param[in]  pstPartCfg ecus配置信息
 * \retval     none
 *******************************************************************************
 */
void adm_ecs_cleanPartConfig
(
    ADM_ECS_PARTCONFIG_T *pstPartCfg
)
{
    if (ADM_IS_NULL(pstPartCfg))
    {
        return;
    }

    adm_com_cleanExtsList(&pstPartCfg->stExtList);
    adm_ecs_delEcusConfigList(pstPartCfg->stEcuCfgList.num, &pstPartCfg->stEcuCfgList.pstEcuCfg);
    pstPartCfg->stEcuCfgList.num = 0;

    return;
}

#if 0
#endif

static D32S __adm_ecs_parseSaMaskListInfo(json_object *jsonRoot, 
    ADM_ECS_SA_MASK_LIST_T **ppstSaMaskList)
{
     D32S iRet = ADM_ECODE_SUCCESSED;
    D32S i = 0;
    json_type    jsonType      = json_type_null;
    json_object *jsonArrayIndexObj = NULL;
    json_object *jsonSaMaskList = NULL;
    ADM_ECS_SA_MASK_LIST_T* pstSaMaskList = NULL;

    if (ADM_IS_NULL(jsonRoot) || ADM_IS_NULL(ppstSaMaskList))
    {
        log_e("Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }

    //!< 取json中saLibList节点信息
    if (!json_object_object_get_ex(jsonRoot, ADM_ECS_CFG_SAMASKLIST, &jsonSaMaskList))
    {
        log_e("Failed to get json value, key[%s]", ADM_ECS_CFG_SAMASKLIST);
        return ADM_ECODE_JSON_GET_KEY_FAILED;
    }

    jsonType = json_object_get_type(jsonSaMaskList);
    if (jsonType != json_type_array)
    {
        log_e("Json key[%s] type error", ADM_ECS_CFG_SAMASKLIST);
        return ADM_ECODE_JSON_KEY_TYPE_ERROR;
    }

    *ppstSaMaskList = (ADM_ECS_SA_MASK_LIST_T*)adm_malloc(sizeof(ADM_ECS_SA_MASK_LIST_T));
    pstSaMaskList = *ppstSaMaskList;

    pstSaMaskList->num = (D32S)json_object_array_length(jsonSaMaskList);
    if (0 == pstSaMaskList->num)
    {
        log_i("Json key[%s] array is zero", ADM_ECS_CFG_SAMASKLIST);
        return ADM_ECODE_SUCCESSED;
    }

    pstSaMaskList->saMaskInfo = (ADM_ECS_SA_MASK_T*)adm_malloc(pstSaMaskList->num * sizeof(ADM_ECS_SA_MASK_T));
    if (ADM_IS_NULL(pstSaMaskList->saMaskInfo))
    {
        log_e("Failed to new ecu saMaskInfo");
        return ADM_ECODE_NO_MEMORY;
    }

    for (i = 0; i < pstSaMaskList->num; ++i) 
    {
        jsonArrayIndexObj = json_object_array_get_idx(jsonSaMaskList, (D32U)i);
        if (ADM_IS_NULL(jsonArrayIndexObj)) 
        {
            iRet = ADM_ECODE_JSON_GET_ARRAYOBJECT_FAILED;
            log_e("Failed to backtracking Json array, arrayIndex[%d]", i);
            break;
        }

        iRet = adm_com_getJsonIntVal(jsonArrayIndexObj, ADM_ECS_CFG_SAMASKLEVEL, (D32S*)(&pstSaMaskList->saMaskInfo[i].saMaskLevel));
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to get json value, key[%s], err[%d]", 
                        ADM_ECS_CFG_SAMASKLEVEL, iRet);
            break;
        }

        pstSaMaskList->saMaskInfo[i].saMaskValue  = adm_com_getJsonStrVal(jsonArrayIndexObj, ADM_ECS_CFG_SAMASKVALUE);
        if (ADM_IS_NULL(pstSaMaskList->saMaskInfo[i].saMaskValue))
        {
            log_e("Failed to get json value, key[%s]", ADM_ECS_CFG_SAMASKVALUE);
            iRet = ADM_ECODE_JSON_GET_STRING_FAILED;
            break;
        }
    }

    if (ADM_IS_ERR(iRet))
    {
        adm_ecs_delSaMaskList(ppstSaMaskList);
    }

    return iRet;
}

/*!
 *******************************************************************************
 * 解析算法库编号列表信息.
 *
 * \note
 * 
 * \param[in]   jsonRoot           Json 节点句柄
 * \param[out]  pstSaLibNumberList 算法库编号列表信息
 * \retval      D32S
 *******************************************************************************
 */
static D32S __adm_ecs_parseSaLibNumberListInfo
(
    json_object               *jsonRoot, 
    ADM_ECS_SALIBNUMBERLIST_T *pstSaLibNumberList
)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    D32S i = 0;
    json_type    jsonType      = json_type_null;
    json_object *jsonArrayIndexObj = NULL;
    json_object *jsonSaLibNumberList = NULL;

    if (ADM_IS_NULL(jsonRoot) || ADM_IS_NULL(pstSaLibNumberList))
    {
        log_e("Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }

    //!< 取json中saLibList节点信息
    if (!json_object_object_get_ex(jsonRoot, ADM_ECS_CFG_SALIBNUMBERLIST, &jsonSaLibNumberList))
    {
        log_e("Failed to get json value, key[%s]", ADM_ECS_CFG_SALIBNUMBERLIST);
        return ADM_ECODE_JSON_GET_KEY_FAILED;
    }

    jsonType = json_object_get_type(jsonSaLibNumberList);
    if (jsonType != json_type_array)
    {
        log_e("Json key[%s] type error", ADM_ECS_CFG_SALIBNUMBERLIST);
        return ADM_ECODE_JSON_KEY_TYPE_ERROR;
    }

    pstSaLibNumberList->num = (D32S)json_object_array_length(jsonSaLibNumberList);
    if (0 == pstSaLibNumberList->num)
    {
        log_i("Json key[%s] array is zero", ADM_ECS_CFG_SALIBNUMBERLIST);
        return ADM_ECODE_SUCCESSED;
    }

    pstSaLibNumberList->psaLibNumber = adm_ecs_newSaLibNumberList(pstSaLibNumberList->num);
    if (ADM_IS_NULL(pstSaLibNumberList->psaLibNumber))
    {
        log_e("Failed to new ecu saLiblist");
        return ADM_ECODE_NO_MEMORY;
    }

    for (i = 0; i < pstSaLibNumberList->num; ++i) 
    {
        jsonArrayIndexObj = json_object_array_get_idx(jsonSaLibNumberList, (D32U)i);
        if (ADM_IS_NULL(jsonArrayIndexObj)) 
        {
            iRet = ADM_ECODE_JSON_GET_ARRAYOBJECT_FAILED;
            log_e("Failed to backtracking Json array, arrayIndex[%d]", i);
            break;
        }

        //!< jsonArrayIndexObj 里面就一个成员，这里直接解
        iRet = adm_com_getJsonIntVal(jsonArrayIndexObj, ADM_ECS_CFG_SALIBNUMBER, &pstSaLibNumberList->psaLibNumber[i]);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to get json value, key[%s], err[%d]", 
                        ADM_ECS_CFG_SALIBNUMBER, iRet);
            break;
        }
    }

    if (ADM_IS_ERR(iRet))
    {
        adm_ecs_delSaLibNumberList(pstSaLibNumberList->num, &pstSaLibNumberList->psaLibNumber);
        pstSaLibNumberList->num = 0;
    }

    return iRet;
}


/*!
 *******************************************************************************
 * 解析CRC算法库编号列表信息.
 *
 * \note
 * 
 * \param[in]   jsonRoot           Json 节点句柄
 * \param[out]  pstCrcLibNumberList CRC算法库编号列表信息
 * \retval      D32S
 *******************************************************************************
 */
static D32S __adm_ecs_parseCrcLibNumberListInfo
(
    json_object               *jsonRoot, 
    ADM_ECS_CRCLIBNUMBERLIST_T *pstCrcLibNumberList
)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    D32S i = 0;
    json_type    jsonType      = json_type_null;
    json_object *jsonArrayIndexObj = NULL;
    json_object *jsonCrcLibNumberList = NULL;

    if (ADM_IS_NULL(jsonRoot) || ADM_IS_NULL(pstCrcLibNumberList))
    {
        log_e("Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }

    //!< 取json中crcLibList节点信息
    if (!json_object_object_get_ex(jsonRoot, ADM_ECS_CFG_CRCLIBNUMBERLIST, &jsonCrcLibNumberList))
    {
        log_e("Failed to get json value, key[%s]", ADM_ECS_CFG_CRCLIBNUMBERLIST);
        return ADM_ECODE_JSON_GET_KEY_FAILED;
    }

    jsonType = json_object_get_type(jsonCrcLibNumberList);
    if (jsonType != json_type_array)
    {
        log_e("Json key[%s] type error", ADM_ECS_CFG_CRCLIBNUMBERLIST);
        return ADM_ECODE_JSON_KEY_TYPE_ERROR;
    }

    pstCrcLibNumberList->num = (D32S)json_object_array_length(jsonCrcLibNumberList);
    if (0 == pstCrcLibNumberList->num)
    {
        log_i("Json key[%s] array is zero", ADM_ECS_CFG_CRCLIBNUMBERLIST);
        return ADM_ECODE_SUCCESSED;
    }

    pstCrcLibNumberList->pcrcLibNumber = adm_ecs_newCrcLibNumberList(pstCrcLibNumberList->num);
    if (ADM_IS_NULL(pstCrcLibNumberList->pcrcLibNumber))
    {
        log_e("Failed to new ecu CrcLibNumberList");
        return ADM_ECODE_NO_MEMORY;
    }

    for (i = 0; i < pstCrcLibNumberList->num; ++i) 
    {
        jsonArrayIndexObj = json_object_array_get_idx(jsonCrcLibNumberList, (D32U)i);
        if (ADM_IS_NULL(jsonArrayIndexObj)) 
        {
            iRet = ADM_ECODE_JSON_GET_ARRAYOBJECT_FAILED;
            log_e("Failed to backtracking Json array, arrayIndex[%d]", i);
            break;
        }

        //!< jsonArrayIndexObj 里面就一个成员，这里直接解
        iRet = adm_com_getJsonIntVal(jsonArrayIndexObj, ADM_ECS_CFG_CRCLIBNUMBER, &pstCrcLibNumberList->pcrcLibNumber[i]);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to get json value, key[%s], err[%d]", 
                        ADM_ECS_CFG_CRCLIBNUMBER, iRet);
            break;
        }
    }

    if (ADM_IS_ERR(iRet))
    {
        adm_ecs_delCrcLibNumberList(pstCrcLibNumberList->num, &pstCrcLibNumberList->pcrcLibNumber);
        pstCrcLibNumberList->num = 0;
    }

    return iRet;
}

/*!
 *******************************************************************************
 * 解析算法库信息.
 *
 * \note
 * 
 * \param[in]   jsonSaLib    Json SaLib节点句柄
 * \param[out]  pstSaLibInfo 算法库信息
 * \retval      D32S
 *******************************************************************************
 */
static D32S __adm_ecs_parseSaLibInfo
(
    json_object         *jsonSaLib, 
    DCHAR               *confFileDir, 
    ADM_ECS_SALIBINFO_T *pstSaLibInfo
)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    DCHAR szBuf[ADM_COM_FILEPATH_LEN] = {0};
    json_type    jsonType = json_type_null;
    json_object *jsonSaLibFile = NULL;

    if (ADM_IS_NULL(jsonSaLib) || ADM_IS_NULL(confFileDir) || ADM_IS_NULL(pstSaLibInfo))
    {
        log_e("Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }

    iRet = adm_com_getJsonIntVal(jsonSaLib, ADM_ECS_CFG_SALIBNUMBER, &pstSaLibInfo->saLibNumber);
    if (ADM_IS_ERR(iRet))
    {
        log_e("Failed to get json value, key[%s], err[%d]", 
                    ADM_ECS_CFG_SALIBNUMBER, iRet);
        return iRet;
    }

    //!< 取json中 saLibFile 节点信息
    if (!json_object_object_get_ex(jsonSaLib, ADM_ECS_CFG_SALIBFILE, &jsonSaLibFile))
    {
        log_e("Failed to get json value, key[%s]", ADM_ECS_CFG_SALIBFILE);
        return ADM_ECODE_JSON_GET_KEY_FAILED;
    }

    jsonType = json_object_get_type(jsonSaLibFile);
    if (jsonType != json_type_object)
    {
        log_e("Json key[%s] type error", ADM_ECS_CFG_BUSINESSSCRIPT);
        return ADM_ECODE_JSON_KEY_TYPE_ERROR;
    }

    iRet = adm_com_parseJsonFileObject(jsonSaLibFile, &pstSaLibInfo->stSaLibFile);
    if (ADM_IS_ERR(iRet))
    {
        log_e("Failed to parse business script file object", iRet);
        return iRet;
    }
    //!< 更新SaLib在车端的存储路径 
    adm_free(pstSaLibInfo->stSaLibFile.filePath);
    (void)adm_snprintf(szBuf, sizeof(szBuf), "%s%s", confFileDir, pstSaLibInfo->stSaLibFile.fileName);
    pstSaLibInfo->stSaLibFile.filePath = adm_strdup(szBuf);
    log_d("SaLibPath[%s]", szBuf);

    return iRet;
}


/*!
 *******************************************************************************
 * 解析算法库列表信息.
 *
 * \note
 * 
 * \param[in]   jsonRoot     Json 根节点句柄
 * \param[in]   confFileDir  配置文件目录
 * \param[out]  pstSaLibList 算法库列表信息
 * \retval      D32S
 *******************************************************************************
 */
static D32S __adm_ecs_parseSaLibListInfo
(
    json_object         *jsonRoot, 
    DCHAR               *confFileDir, 
    ADM_ECS_SALIBLIST_T *pstSaLibList
)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    D32S i = 0;
    json_type    jsonType      = json_type_null;
    json_object *jsonSaLibList = NULL;
    json_object *jsonArrayIndexObj = NULL;

    if (ADM_IS_NULL(jsonRoot) || ADM_IS_NULL(confFileDir) || ADM_IS_NULL(pstSaLibList))
    {
        log_e("Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }

    //!< 取json中saLibList节点信息
    if (!json_object_object_get_ex(jsonRoot, ADM_ECS_CFG_SALIBLIST, &jsonSaLibList))
    {
        log_e("Failed to get json value, key[%s]", ADM_ECS_CFG_SALIBLIST);
        return ADM_ECODE_JSON_GET_KEY_FAILED;
    }

    jsonType = json_object_get_type(jsonSaLibList);
    if (jsonType != json_type_array)
    {
        log_e("Json key[%s] type error", ADM_ECS_CFG_SALIBLIST);
        return ADM_ECODE_JSON_KEY_TYPE_ERROR;
    }

    pstSaLibList->num = (D32S)json_object_array_length(jsonSaLibList);
    if (0 == pstSaLibList->num)
    {
        log_i("Json key[%s] array is zero", ADM_ECS_CFG_SALIBLIST);
        return ADM_ECODE_SUCCESSED;
    }

    pstSaLibList->pstSaLibInfo = adm_ecs_newSaLibList(pstSaLibList->num);
    if (ADM_IS_NULL(pstSaLibList->pstSaLibInfo))
    {
        log_e("Failed to new ecu saLiblist");
        return ADM_ECODE_NO_MEMORY;
    }

    for (i = 0; i < pstSaLibList->num; ++i) 
    {
        jsonArrayIndexObj = json_object_array_get_idx(jsonSaLibList, (D32U)i);
        if (ADM_IS_NULL(jsonArrayIndexObj)) 
        {
            iRet = ADM_ECODE_JSON_GET_ARRAYOBJECT_FAILED;
            log_e("Failed to backtracking Json array, arrayIndex[%d]", i);
            break;
        }

        iRet = __adm_ecs_parseSaLibInfo(jsonArrayIndexObj, confFileDir, &pstSaLibList->pstSaLibInfo[i]);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to parse saLib info, err[%d]", iRet);
            break;
        }
    }

    if (ADM_IS_ERR(iRet))
    {
        adm_ecs_delSaLibList(pstSaLibList->num, &pstSaLibList->pstSaLibInfo);
        pstSaLibList->num = 0;
    }

    return iRet;
}

/*!
 *******************************************************************************
 * 解析刷写脚本信息.
 *
 * \note
 * 
 * \param[in]   jsonFlashScript    Json flashScript节点句柄
 * \param[in]   confFileDir        配置文件目录
 * \param[out]  pstFlashScriptInfo 刷写脚本信息
 * \retval      D32S
 *******************************************************************************
 */
static D32S __adm_ecs_parseFlashScriptInfo
(
    json_object               *jsonFlashScript, 
    DCHAR                     *confFileDir, 
    ADM_ECS_FLASHSCRIPTINFO_T *pstFlashScriptInfo
)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    DCHAR szBuf[ADM_COM_FILEPATH_LEN] = {0};
    json_type    jsonType = json_type_null;
    json_object *jsonFlashScriptFile = NULL;

    if (ADM_IS_NULL(jsonFlashScript) || ADM_IS_NULL(confFileDir) || ADM_IS_NULL(pstFlashScriptInfo))
    {
        log_e("Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }

    iRet = adm_com_getJsonIntVal(jsonFlashScript, ADM_ECS_CFG_FLASHSCRIPTNUMBER, 
                &pstFlashScriptInfo->flashScriptNumber);
    if (ADM_IS_ERR(iRet))
    {
        log_e("Failed to get json value, key[%s], err[%d]", 
                    ADM_ECS_CFG_FLASHSCRIPTNUMBER, iRet);
        return iRet;
    }

    //!< 取json中 flashScriptFile 节点信息
    if (!json_object_object_get_ex(jsonFlashScript, ADM_ECS_CFG_FLASHSCRIPTFILE, &jsonFlashScriptFile))
    {
        log_e("Failed to get json value, key[%s]", ADM_ECS_CFG_FLASHSCRIPTFILE);
        return ADM_ECODE_JSON_GET_KEY_FAILED;
    }

    jsonType = json_object_get_type(jsonFlashScriptFile);
    if (jsonType != json_type_object)
    {
        log_e("Json key[%s] type error", ADM_ECS_CFG_FLASHSCRIPTFILE);
        return ADM_ECODE_JSON_KEY_TYPE_ERROR;
    }

    iRet = adm_com_parseJsonFileObject(jsonFlashScriptFile, 
                &pstFlashScriptInfo->stFlashScriptFile);
    if (ADM_IS_ERR(iRet))
    {
        log_e("Failed to parse business script file object", iRet);
        return iRet;
    }
    //!< 更新FlashScript在车端的存储路径 
    adm_free(pstFlashScriptInfo->stFlashScriptFile.filePath);
    (void)adm_snprintf(szBuf, sizeof(szBuf), "%s%s", confFileDir, pstFlashScriptInfo->stFlashScriptFile.fileName);
    pstFlashScriptInfo->stFlashScriptFile.filePath = adm_strdup(szBuf);
    log_d("FlashScriptPath[%s]", szBuf);

    return iRet;
}


/*!
 *******************************************************************************
 * 解析刷写脚本列表信息.
 *
 * \note
 * 
 * \param[in]   jsonRoot           Json 根节点句柄
 * \param[in]   confFileDir        配置文件目录
 * \param[out]  pstFlashScriptList 刷写脚本列表信息
 * \retval      D32S
 *******************************************************************************
 */
static D32S __adm_ecs_parseFlashScriptListInfo
(
    json_object               *jsonRoot, 
    DCHAR                     *confFileDir, 
    ADM_ECS_FLASHSCRIPTLIST_T *pstFlashScriptList
)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    D32S i = 0;
    json_type    jsonType = json_type_null;
    json_object *jsonFlashScriptList = NULL;
    json_object *jsonArrayIndexObj = NULL;

    if (ADM_IS_NULL(jsonRoot) || ADM_IS_NULL(confFileDir) || ADM_IS_NULL(pstFlashScriptList))
    {
        log_e("Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }

    //!< 取json中flashScriptList节点信息
    if (!json_object_object_get_ex(jsonRoot, ADM_ECS_CFG_FLASHSCRIPTLIST, &jsonFlashScriptList))
    {
        log_e("Failed to get json value, key[%s]", ADM_ECS_CFG_FLASHSCRIPTLIST);
        return ADM_ECODE_JSON_GET_KEY_FAILED;
    }

    jsonType = json_object_get_type(jsonFlashScriptList);
    if (jsonType != json_type_array)
    {
        log_e("Json key[%s] type error", ADM_ECS_CFG_FLASHSCRIPTLIST);
        return ADM_ECODE_JSON_KEY_TYPE_ERROR;
    }

    pstFlashScriptList->num = (D32S)json_object_array_length(jsonFlashScriptList);
    if (0 == pstFlashScriptList->num)
    {
        log_i("Json key[%s] array is zero", ADM_ECS_CFG_FLASHSCRIPTLIST);
        return ADM_ECODE_SUCCESSED;
    }

    pstFlashScriptList->pstFlashScriptInfo = adm_ecs_newFlashScriptList(pstFlashScriptList->num);
    if (ADM_IS_NULL(pstFlashScriptList->pstFlashScriptInfo))
    {
        log_e("Failed to new ecu saLiblist");
        return ADM_ECODE_NO_MEMORY;
    }

    for (i = 0; i < pstFlashScriptList->num; ++i) 
    {
        jsonArrayIndexObj = json_object_array_get_idx(jsonFlashScriptList, (D32U)i);
        if (ADM_IS_NULL(jsonArrayIndexObj)) 
        {
            iRet = ADM_ECODE_JSON_GET_ARRAYOBJECT_FAILED;
            log_e("Failed to backtracking Json array, arrayIndex[%d]", i);
            break;
        }

        iRet = __adm_ecs_parseFlashScriptInfo(jsonArrayIndexObj, confFileDir, &pstFlashScriptList->pstFlashScriptInfo[i]);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to parse saLib info, err[%d]", iRet);
            break;
        }
    }

    if (ADM_IS_ERR(iRet))
    {
        adm_ecs_delFlashScriptList(pstFlashScriptList->num, &pstFlashScriptList->pstFlashScriptInfo);
        pstFlashScriptList->num = 0;
    }

    return iRet;
}


/*!
 *******************************************************************************
 * 解析业务脚本对象列表信息.
 *
 * \note
 * 
 * \param[in]   jsonBusinessScript    Json businessScripts节点句柄
 * \param[in]   confFileDir           配置文件目录
 * \param[out]  pstBusinessScriptInfo 业务脚本信息
 * \retval      D32S
 *******************************************************************************
 */
static D32S __adm_ecs_parseBusinessScriptsInfo
(
    json_object                  *jsonBusinessScript, 
    DCHAR                        *confFileDir, 
    ADM_ECS_BUSINESSSCRIPTINFO_T *pstBusinessScriptInfo
)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    DCHAR szBuf[ADM_COM_FILEPATH_LEN] = {0};
    json_type    jsonType = json_type_null;
    json_object *jsonBusinessScriptFile = NULL;

    if (ADM_IS_NULL(jsonBusinessScript) || ADM_IS_NULL(confFileDir) || ADM_IS_NULL(pstBusinessScriptInfo))
    {
        log_e("Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }

    iRet = adm_com_getJsonIntVal(jsonBusinessScript, ADM_ECS_CFG_BUSINESSSCRIPTNUMBER, 
                &pstBusinessScriptInfo->businessScriptNumber);
    if (ADM_IS_ERR(iRet))
    {
        log_e("Failed to get json value, key[%s], err[%d]", 
                    ADM_ECS_CFG_BUSINESSSCRIPTNUMBER, iRet);
        return iRet;
    }

    //!< 取json中 businessScriptFile 节点信息
    if (!json_object_object_get_ex(jsonBusinessScript, ADM_ECS_CFG_BUSINESSSCRIPTFILE, &jsonBusinessScriptFile))
    {
        log_e("Failed to get json value, key[%s]", ADM_ECS_CFG_BUSINESSSCRIPTFILE);
        return ADM_ECODE_JSON_GET_KEY_FAILED;
    }

    jsonType = json_object_get_type(jsonBusinessScriptFile);
    if (jsonType != json_type_object)
    {
        log_e("Json key[%s] type error", ADM_ECS_CFG_BUSINESSSCRIPTFILE);
        return ADM_ECODE_JSON_KEY_TYPE_ERROR;
    }

    iRet = adm_com_parseJsonFileObject(jsonBusinessScriptFile, 
                &pstBusinessScriptInfo->stBusinessScriptFile);
    if (ADM_IS_ERR(iRet))
    {
        log_e("Failed to parse business script file object", iRet);
        return iRet;
    }
    //!< 更新BusinessScript在车端的存储路径 
    adm_free(pstBusinessScriptInfo->stBusinessScriptFile.filePath);
    (void)adm_snprintf(szBuf, sizeof(szBuf), "%s%s", confFileDir, pstBusinessScriptInfo->stBusinessScriptFile.fileName);
    pstBusinessScriptInfo->stBusinessScriptFile.filePath = adm_strdup(szBuf);
    log_d("BusinessScriptPath[%s]", szBuf);

    return iRet;
}

/*!
 *******************************************************************************
 * 解析业务脚本对象列表信息.
 *
 * \note
 * 
 * \param[in]   jsonRoot           Json 根节点句柄
 * \param[in]   confFileDir        配置文件目录
 * \param[out]  pstBusinessScriptList 业务脚本对象列表
 * \retval      D32S
 *******************************************************************************
 */
static D32S __adm_ecs_parseBusinessScriptListInfo
(
    json_object                   *jsonRoot, 
    DCHAR                         *confFileDir, 
    ADM_ECS_BUSINESSSCRIPTLIST_T  *pstBusinessScriptList
)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    D32S i = 0;
    json_type    jsonType = json_type_null;
    json_object *jsonBusinessScriptList = NULL;
    json_object *jsonArrayIndexObj = NULL;

    if (ADM_IS_NULL(jsonRoot) || ADM_IS_NULL(confFileDir) || ADM_IS_NULL(pstBusinessScriptList))
    {
        log_e("Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }

    //!< 取json中businessScriptList节点信息
    if (!json_object_object_get_ex(jsonRoot, ADM_ECS_CFG_BUSINESSSCRIPTLIST, &jsonBusinessScriptList))
    {
        log_e("Failed to get json value, key[%s]", ADM_ECS_CFG_BUSINESSSCRIPTLIST);
        return ADM_ECODE_JSON_GET_KEY_FAILED;
    }

    jsonType = json_object_get_type(jsonBusinessScriptList);
    if (jsonType != json_type_array)
    {
        log_e("Json key[%s] type error", ADM_ECS_CFG_BUSINESSSCRIPTLIST);
        return ADM_ECODE_JSON_KEY_TYPE_ERROR;
    }

    pstBusinessScriptList->num = (D32S)json_object_array_length(jsonBusinessScriptList);
    if (0 == pstBusinessScriptList->num)
    {
        log_i("Json key[%s] array is zero", ADM_ECS_CFG_BUSINESSSCRIPTLIST);
        return ADM_ECODE_SUCCESSED;
    }

    pstBusinessScriptList->pstBusinessScriptInfo = adm_ecs_newBusinessScriptList(pstBusinessScriptList->num);
    if (ADM_IS_NULL(pstBusinessScriptList->pstBusinessScriptInfo))
    {
        log_e("Failed to new ecu businessLiblist");
        return ADM_ECODE_NO_MEMORY;
    }

    for (i = 0; i < pstBusinessScriptList->num; ++i) 
    {
        jsonArrayIndexObj = json_object_array_get_idx(jsonBusinessScriptList, (D32U)i);
        if (ADM_IS_NULL(jsonArrayIndexObj)) 
        {
            iRet = ADM_ECODE_JSON_GET_ARRAYOBJECT_FAILED;
            log_e("Failed to backtracking Json array, arrayIndex[%d]", i);
            break;
        }

        iRet = __adm_ecs_parseBusinessScriptsInfo(jsonArrayIndexObj, confFileDir, &pstBusinessScriptList->pstBusinessScriptInfo[i]);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to parse businessscript info, err[%d]", iRet);
            break;
        }
    }

    if (ADM_IS_ERR(iRet))
    {
        adm_ecs_delBusinessScriptList(pstBusinessScriptList->num, &pstBusinessScriptList->pstBusinessScriptInfo);
    }

    return iRet;
}



/*!
 *******************************************************************************
 * 解析ecu资源id信息.
 *
 * \note
 * 
 * \param[in]   jsonResDid    Json resourceDid节点句柄
 * \param[out]  pstEcuDidInfo ecu资源id信息
 * \retval      D32S
 *******************************************************************************
 */
static D32S __adm_ecs_parseEcuDidInfo
(
    json_object      *jsonResDid, 
    ADM_ECS_ECUDID_T *pstEcuDidInfo
)
{
    D32S iRet = ADM_ECODE_SUCCESSED;

    if (ADM_IS_NULL(jsonResDid) || ADM_IS_NULL(pstEcuDidInfo))
    {
        log_e("Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }

    pstEcuDidInfo->did = adm_com_getJsonStrVal(jsonResDid, ADM_ECS_CFG_DID);
    if (ADM_IS_NULL(pstEcuDidInfo->did))
    {
        log_e("Failed to get json value, key[%s]", ADM_ECS_CFG_DID);
        return ADM_ECODE_JSON_GET_STRING_FAILED;
    }

    iRet = adm_com_getJsonIntVal(jsonResDid, ADM_ECS_CFG_START, &pstEcuDidInfo->start);
    if (ADM_IS_ERR(iRet))
    {
        log_e("Failed to get json value, key[%s], err[%d]", ADM_ECS_CFG_START, iRet);
        return iRet;
    }
    
    iRet = adm_com_getJsonIntVal(jsonResDid, ADM_ECS_CFG_END, &pstEcuDidInfo->end);
    if (ADM_IS_ERR(iRet))
    {
        log_e("Failed to get json value, key[%s], err[%d]", ADM_ECS_CFG_END, iRet);
        return iRet;
    }

    iRet = adm_com_getJsonIntVal(jsonResDid, ADM_ECS_CFG_DIDVALTYPE, &pstEcuDidInfo->didValType);
    if (ADM_IS_ERR(iRet))
    {
        log_e("Failed to get json value, key[%s], err[%d]", ADM_ECS_CFG_DIDVALTYPE, iRet);
        return iRet;
    }

    return iRet;
}


/*!
 *******************************************************************************
 * 解析资源id列表信息.
 *
 * \note
 * 
 * \param[in]   jsonRoot      Json 根节点句柄
 * \param[out]  pstResDidList 资源id列表信息
 * \retval      D32S
 *******************************************************************************
 */
static D32S __adm_ecs_parseEcuDidListInfo
(
    json_object          *jsonRoot, 
    ADM_ECS_ECUDIDLIST_T *pstEcuDidList
)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    D32S i = 0;
    json_type    jsonType      = json_type_null;
    json_object *jsonResDidList = NULL;
    json_object *jsonArrayIndexObj = NULL;

    if (ADM_IS_NULL(jsonRoot) || ADM_IS_NULL(pstEcuDidList))
    {
        log_e("Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }

    //!< 取json中resourceDidList节点信息
    if (!json_object_object_get_ex(jsonRoot, ADM_ECS_CFG_ECUDIDLIST, &jsonResDidList))
    {
        log_e("Failed to get json value, key[%s]", ADM_ECS_CFG_ECUDIDLIST);
        return ADM_ECODE_JSON_GET_KEY_FAILED;
    }

    jsonType = json_object_get_type(jsonResDidList);
    if (jsonType != json_type_array)
    {
        log_e("Json key[%s] type error", ADM_ECS_CFG_ECUDIDLIST);
        return ADM_ECODE_JSON_KEY_TYPE_ERROR;
    }

    pstEcuDidList->num = (D32S)json_object_array_length(jsonResDidList);
    if (0 == pstEcuDidList->num)
    {
        log_e("Json key[%s] array is zero", ADM_ECS_CFG_ECUDIDLIST);
        return ADM_ECODE_SUCCESSED;
    }

    pstEcuDidList->pstEcuDidInfo = adm_ecs_newEcuDidList(pstEcuDidList->num);
    if (ADM_IS_NULL(pstEcuDidList->pstEcuDidInfo))
    {
        log_e("Failed to new ecu ecuDidlist");
        return ADM_ECODE_NO_MEMORY;
    }

    for (i = 0; i < pstEcuDidList->num; ++i) 
    {
        jsonArrayIndexObj = json_object_array_get_idx(jsonResDidList, (D32U)i);
        if (ADM_IS_NULL(jsonArrayIndexObj)) 
        {
            iRet = ADM_ECODE_JSON_GET_ARRAYOBJECT_FAILED;
            log_e("Failed to backtracking Json array, arrayIndex[%d]", i);
            break;
        }

        iRet = __adm_ecs_parseEcuDidInfo(jsonArrayIndexObj, &pstEcuDidList->pstEcuDidInfo[i]);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to parse ecuDid info, err[%d]", iRet);
            break;
        }
    }

    if (ADM_IS_ERR(iRet))
    {
        adm_ecs_delEcuDidList(pstEcuDidList->num, &pstEcuDidList->pstEcuDidInfo);
        pstEcuDidList->num = 0;
    }

    return iRet;
}

/*!
 *******************************************************************************
 * 解析业务脚本对象信息.
 *
 * \note
 * 
 * \param[in]   jsonRoot          Json 根节点句柄
 * \param[in]   confFileDir       配置文件目录
 * \param[out]  pstBusinessScript 业务脚本对象信息
 * \retval      D32S
 *******************************************************************************
 */
static D32S __adm_ecs_parseBusinessScriptInfo
(
    json_object          *jsonRoot, 
    DCHAR                *confFileDir, 
    ADM_COM_FILEOBJECT_T *pstBusinessScript
)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    DCHAR szBuf[ADM_COM_FILEPATH_LEN] = {0};
    json_type    jsonType = json_type_null;
    json_object *jsonBusinessScript = NULL;

    if (ADM_IS_NULL(jsonRoot) || ADM_IS_NULL(confFileDir) || ADM_IS_NULL(pstBusinessScript))
    {
        log_e("Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }

    //!< 取json中businessScript节点信息
    if (!json_object_object_get_ex(jsonRoot, ADM_ECS_CFG_BUSINESSSCRIPT, &jsonBusinessScript))
    {
        log_e("Failed to get json value, key[%s]", ADM_ECS_CFG_BUSINESSSCRIPT);
        return ADM_ECODE_JSON_GET_KEY_FAILED;
    }

    jsonType = json_object_get_type(jsonBusinessScript);
    if (jsonType != json_type_object)
    {
        log_e("Json key[%s] type error", ADM_ECS_CFG_BUSINESSSCRIPT);
        return ADM_ECODE_JSON_KEY_TYPE_ERROR;
    }

    iRet = adm_com_parseJsonFileObject(jsonBusinessScript, pstBusinessScript);
    if (ADM_IS_ERR(iRet))
    {
        log_e("Failed to parse business script file object", iRet);
        return iRet;
    }
    //!< 更新业务脚本在车端的存储路径 
    adm_free(pstBusinessScript->filePath);
    (void)adm_snprintf(szBuf, sizeof(szBuf), "%s%s", confFileDir, pstBusinessScript->fileName);
    pstBusinessScript->filePath = adm_strdup(szBuf);
    log_d("BusinessScriptPath[%s]", szBuf);

    return iRet;
}

/*!
 *******************************************************************************
 * 解析算法库信息.
 *
 * \note
 * 
 * \param[in]   jsonSaLib    Json SaLib节点句柄
 * \param[in]   confFileDir  配置文件目录
 * \param[out]  pstSaLibInfo 算法库信息
 * \retval      D32S
 *******************************************************************************
 */
static D32S __adm_ecs_parseCrcLibInfo
(
    json_object          *jsonCrcLib, 
    DCHAR                *confFileDir, 
    ADM_ECS_CRCLIBINFO_T *pstCrcLibInfo
)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    DCHAR szBuf[ADM_COM_FILEPATH_LEN] = {0};
    json_type    jsonType = json_type_null;
    json_object *jsonCrcLibFile = NULL;

    if (ADM_IS_NULL(jsonCrcLib) || ADM_IS_NULL(confFileDir) || ADM_IS_NULL(pstCrcLibInfo))
    {
        log_e("Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }

    iRet = adm_com_getJsonIntVal(jsonCrcLib, ADM_ECS_CFG_CRCLIBNUMBER, &pstCrcLibInfo->crcLibNumber);
    if (ADM_IS_ERR(iRet))
    {
        log_e("Failed to get json value, key[%s], err[%d]", 
                    ADM_ECS_CFG_CRCLIBNUMBER, iRet);
        return iRet;
    }

    //!< 取json中 saLibFile 节点信息
    if (!json_object_object_get_ex(jsonCrcLib, ADM_ECS_CFG_CRCLIBFILE, &jsonCrcLibFile))
    {
        log_e("Failed to get json value, key[%s]", ADM_ECS_CFG_CRCLIBFILE);
        return ADM_ECODE_JSON_GET_KEY_FAILED;
    }

    jsonType = json_object_get_type(jsonCrcLibFile);
    if (jsonType != json_type_object)
    {
        log_e("Json key[%s] type error", ADM_ECS_CFG_CRCLIBFILE);
        return ADM_ECODE_JSON_KEY_TYPE_ERROR;
    }

    iRet = adm_com_parseJsonFileObject(jsonCrcLibFile, &pstCrcLibInfo->stCrcLibFile);
    if (ADM_IS_ERR(iRet))
    {
        log_e("Failed to parse business script file object", iRet);
        return iRet;
    }
    //!< 更新CrcLib在车端的存储路径 
    adm_free(pstCrcLibInfo->stCrcLibFile.filePath);
    (void)adm_snprintf(szBuf, sizeof(szBuf), "%s%s", confFileDir, pstCrcLibInfo->stCrcLibFile.fileName);
    pstCrcLibInfo->stCrcLibFile.filePath = adm_strdup(szBuf);
    log_d("CrcLibPath[%s]", szBuf);

    return iRet;
}


/*!
 *******************************************************************************
 * 解析crc算法库列表信息.
 *
 * \note
 * 
 * \param[in]   jsonRoot      Json 根节点句柄
 * \param[in]   confFileDir   配置文件目录
 * \param[out]  pstCrcLibList 算法库列表信息
 * \retval      D32S
 *******************************************************************************
 */
static D32S __adm_ecs_parseCrcLibListInfo
(
    json_object          *jsonRoot, 
    DCHAR                *confFileDir, 
    ADM_ECS_CRCLIBLIST_T *pstCrcLibList
)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    D32S i = 0;
    json_type    jsonType      = json_type_null;
    json_object *jsonCrcLibList = NULL;
    json_object *jsonArrayIndexObj = NULL;

    if (ADM_IS_NULL(jsonRoot) || ADM_IS_NULL(confFileDir) || ADM_IS_NULL(pstCrcLibList))
    {
        log_e("Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }

    //!< 取json中saLibList节点信息
    if (!json_object_object_get_ex(jsonRoot, ADM_ECS_CFG_CRCLIBLIST, &jsonCrcLibList))
    {
        log_e("Failed to get json value, key[%s]", ADM_ECS_CFG_CRCLIBLIST);
        return ADM_ECODE_JSON_GET_KEY_FAILED;
    }

    jsonType = json_object_get_type(jsonCrcLibList);
    if (jsonType != json_type_array)
    {
        log_e("Json key[%s] type error", ADM_ECS_CFG_CRCLIBLIST);
        return ADM_ECODE_JSON_KEY_TYPE_ERROR;
    }

    pstCrcLibList->num = (D32S)json_object_array_length(jsonCrcLibList);
    if (0 == pstCrcLibList->num)
    {
        log_i("Json key[%s] array is zero", ADM_ECS_CFG_CRCLIBLIST);
        return ADM_ECODE_SUCCESSED;
    }

    pstCrcLibList->pstCrcLibInfo = adm_ecs_newCrcLibList(pstCrcLibList->num);
    if (ADM_IS_NULL(pstCrcLibList->pstCrcLibInfo))
    {
        log_e("Failed to new ecu saLiblist");
        return ADM_ECODE_NO_MEMORY;
    }

    for (i = 0; i < pstCrcLibList->num; ++i) 
    {
        jsonArrayIndexObj = json_object_array_get_idx(jsonCrcLibList, (D32U)i);
        if (ADM_IS_NULL(jsonArrayIndexObj)) 
        {
            iRet = ADM_ECODE_JSON_GET_ARRAYOBJECT_FAILED;
            log_e("Failed to backtracking Json array, arrayIndex[%d]", i);
            break;
        }

        iRet = __adm_ecs_parseCrcLibInfo(jsonArrayIndexObj, confFileDir, &pstCrcLibList->pstCrcLibInfo[i]);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to parse saLib info, err[%d]", iRet);
            break;
        }
    }

    if (ADM_IS_ERR(iRet))
    {
        adm_ecs_delCrcLibList(pstCrcLibList->num, &pstCrcLibList->pstCrcLibInfo);
        pstCrcLibList->num = 0;
    }

    return iRet;
}

/*!
 *******************************************************************************
 * 解析ecu配置信息.
 *
 * \note
 * 
 * \param[in]   jsonEcus   Json ecus节点句柄
 * \param[out]  pstEcuInfo ecu配置信息
 * \retval      D32S
 *******************************************************************************
 */
static D32S __adm_ecs_parseEcuConfigInfo
(
    json_object         *jsonEcus, 
    ADM_ECS_ECUCONFIG_T *pstEcuInfo
)
{
    D32S iRet = ADM_ECODE_SUCCESSED;

    if (ADM_IS_NULL(jsonEcus) || ADM_IS_NULL(pstEcuInfo))
    {
        log_e("Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }

    pstEcuInfo->ecuReqId = adm_com_getJsonStrVal(jsonEcus, ADM_COM_AIR_PROTOCOL_FIELD_ECUREQID);
    if (ADM_IS_NULL(pstEcuInfo->ecuReqId))
    {
        log_e("Failed to get json value, key[%s]", 
                    ADM_COM_AIR_PROTOCOL_FIELD_ECUREQID);
        return ADM_ECODE_JSON_GET_STRING_FAILED;
    }

    pstEcuInfo->ecuSwDid = adm_com_getJsonStrVal(jsonEcus, ADM_COM_AIR_PROTOCOL_FIELD_ECUSWDID);
    if (ADM_IS_NULL(pstEcuInfo->ecuSwDid))
    {
        log_e("Failed to get json value, key[%s]", 
                    ADM_COM_AIR_PROTOCOL_FIELD_ECUSWDID);
        return ADM_ECODE_JSON_GET_STRING_FAILED;
    }
    
    pstEcuInfo->ecuResponseId = adm_com_getJsonStrVal(jsonEcus, ADM_ECS_CFG_ECURSPID);
    if (ADM_IS_NULL(pstEcuInfo->ecuResponseId))
    {
        log_e("Failed to get json value, key[%s]", ADM_ECS_CFG_ECURSPID);
        return ADM_ECODE_JSON_GET_STRING_FAILED;
    }

    pstEcuInfo->ecuFunctionId = adm_com_getJsonStrVal(jsonEcus, ADM_ECS_CFG_ECUFUNCID);
    if (ADM_IS_NULL(pstEcuInfo->ecuFunctionId))
    {
        log_e("Failed to get json value, key[%s]", ADM_ECS_CFG_ECUFUNCID);
        return ADM_ECODE_JSON_GET_STRING_FAILED;
    }

    pstEcuInfo->readInfoInstanceId = adm_com_getJsonStrVal(jsonEcus, ADM_ECS_CFG_READINFOISTID);
    if (ADM_IS_NULL(pstEcuInfo->readInfoInstanceId))
    {
        log_e("Failed to get json value, key[%s]", ADM_ECS_CFG_READINFOISTID);
        return ADM_ECODE_JSON_GET_STRING_FAILED;
    }

    pstEcuInfo->downloadInstanceId = adm_com_getJsonStrVal(jsonEcus, ADM_ECS_CFG_DOWNLOADISTID);
    if (ADM_IS_NULL(pstEcuInfo->downloadInstanceId))
    {
        log_e("Failed to get json value, key[%s]", ADM_ECS_CFG_DOWNLOADISTID);
        return ADM_ECODE_JSON_GET_STRING_FAILED;
    }

    pstEcuInfo->transferInstanceId = adm_com_getJsonStrVal(jsonEcus, ADM_ECS_CFG_TRANSFERISTID);
    if (ADM_IS_NULL(pstEcuInfo->transferInstanceId))
    {
        log_e("Failed to get json value, key[%s]", ADM_ECS_CFG_TRANSFERISTID);
        return ADM_ECODE_JSON_GET_STRING_FAILED;
    }

    pstEcuInfo->installInstanceId = adm_com_getJsonStrVal(jsonEcus, ADM_ECS_CFG_INSTALLISTID);
    if (ADM_IS_NULL(pstEcuInfo->installInstanceId))
    {
        log_e("Failed to get json value, key[%s]", ADM_ECS_CFG_INSTALLISTID);
        return ADM_ECODE_JSON_GET_STRING_FAILED;
    }

    //!< get selfBackup
    iRet = adm_com_getJsonIntVal(jsonEcus, ADM_ECS_CFG_SELFBACKUP, &pstEcuInfo->selfBackup);
    if (ADM_IS_ERR(iRet))
    {
        log_e("Failed to get json value, key[%s], err[%d]", 
                    ADM_ECS_CFG_SELFBACKUP, iRet);
        return iRet;
    }

    //!< get selfBackup
    iRet = adm_com_getJsonIntVal(jsonEcus, ADM_ECS_CFG_ISSELFUPDATE, &pstEcuInfo->isSelfUpdate);
    if (ADM_IS_ERR(iRet))
    {
        log_e("Failed to get json value, key[%s], err[%d]", 
                    ADM_ECS_CFG_ISSELFUPDATE, iRet);
        return iRet;
    }

    iRet = adm_com_getJsonIntVal(jsonEcus, ADM_ECS_CFG_BUSTYPE, &pstEcuInfo->busType);
    if (ADM_IS_ERR(iRet))
    {
        log_e("Failed to get json value, key[%s], err[%d]", 
                    ADM_ECS_CFG_BUSTYPE, iRet);
        return iRet;
    }

    iRet = adm_com_getJsonIntVal(jsonEcus, ADM_ECS_CFG_CHANNELTYPE, &pstEcuInfo->channelType);
    if (ADM_IS_ERR(iRet))
    {
        log_e("Failed to get json value, key[%s], err[%d]", 
                    ADM_ECS_CFG_CHANNELTYPE, iRet);
        return iRet;
    }

    iRet = adm_com_getJsonIntVal(jsonEcus, ADM_ECS_CFG_CRCMASK, &pstEcuInfo->crcMask);
    if (ADM_IS_ERR(iRet))
    {
        log_e("Failed to get json value, key[%s], err[%d]", 
                    ADM_ECS_CFG_CRCMASK, iRet);
        return iRet;
    }

    iRet = adm_com_getJsonIntVal(jsonEcus, ADM_ECS_CFG_CRCLIBNUMBER, &pstEcuInfo->crcLibNumber);
    if (ADM_IS_ERR(iRet))
    {
        log_e("Failed to get json value, key[%s], err[%d]", 
                    ADM_ECS_CFG_CRCLIBNUMBER, iRet);
        return iRet;
    }

    iRet = adm_com_getJsonIntVal(jsonEcus, ADM_ECS_CFG_FLASHSCRIPTNUMBER, &pstEcuInfo->flashScriptNumber);
    if (ADM_IS_ERR(iRet))
    {
        log_e("Failed to get json value, key[%s], err[%d]", 
                    ADM_ECS_CFG_FLASHSCRIPTNUMBER, iRet);
        return iRet;
    }

    iRet = adm_com_getJsonIntVal(jsonEcus, ADM_ECS_CFG_ISCGWROUTED, &pstEcuInfo->isCGWRouted);
    if (ADM_IS_ERR(iRet))
    {
        log_e("Failed to get json value, key[%s], err[%d]", 
                    ADM_ECS_CFG_ISCGWROUTED, iRet);
        return iRet;
    }

    iRet = adm_com_getJsonIntVal(jsonEcus, ADM_ECS_CFG_SERVICETYPE, &pstEcuInfo->serviceType);
    if (ADM_IS_ERR(iRet))
    {
        log_e("Failed to get json value, key[%s], err[%d]", 
                    ADM_ECS_CFG_SERVICETYPE, iRet);
        return iRet;
    }

    iRet = adm_com_getJsonIntVal(jsonEcus, ADM_ECS_CFG_FLASHPROGRESSTYPE, &pstEcuInfo->flashProgressType);
    if (ADM_IS_ERR(iRet))
    {
        log_w("Failed to get json value, key[%s], err[%d]", 
                    ADM_ECS_CFG_FLASHPROGRESSTYPE, iRet);

        if (1 == pstEcuInfo->serviceType)
        {
            return iRet;
        }
    }

    iRet = adm_com_getJsonIntVal(jsonEcus, ADM_ECS_CFG_INSTCONDITION, &pstEcuInfo->instCondition);
    if (ADM_IS_ERR(iRet))
    {
        log_e("Failed to get json value, key[%s], err[%d]", 
                    ADM_ECS_CFG_INSTCONDITION, iRet);
        return iRet;
    }

    iRet = adm_com_getJsonIntVal(jsonEcus, ADM_ECS_CFG_SECURITYLEVEL, &pstEcuInfo->securityLevel);
    if (ADM_IS_ERR(iRet))
    {
        log_e("Failed to get json value, key[%s], err[%d]", 
                    ADM_ECS_CFG_SECURITYLEVEL, iRet);
        return iRet;
    }

    //!< 解析saMaskList 
    iRet = __adm_ecs_parseSaMaskListInfo(jsonEcus, &pstEcuInfo->saMaskList);
    if (ADM_IS_ERR(iRet))
    {
        log_w("Failed to get json value, key[%s], err[%d]", 
                    ADM_ECS_CFG_SAMASKLIST, iRet);
    }

    /** saMaskList为空则再取saMask */
    pstEcuInfo->saMask = adm_com_getJsonStrVal(jsonEcus, ADM_ECS_CFG_SAMASK);
    if (ADM_IS_NULL(pstEcuInfo->saMask) && (ADM_IS_NULL(pstEcuInfo->saMaskList) || 0 == pstEcuInfo->saMaskList->num))
    {
        log_e("Failed to get json value, key[%s]", ADM_ECS_CFG_SAMASK);
        return ADM_ECODE_JSON_GET_STRING_FAILED;
    }

    //!< 解析 saLibNumbers 列表信息
    iRet = __adm_ecs_parseSaLibNumberListInfo(jsonEcus, &pstEcuInfo->saLibNumberList);
    if (ADM_IS_ERR(iRet))
    {
        log_e("Failed to get json value, key[%s], err[%d]", 
                    ADM_ECS_CFG_SALIBNUMBER, iRet);
        return iRet;
    }

    //!< 解析 crcLibNumbers 列表信息
    iRet = __adm_ecs_parseCrcLibNumberListInfo(jsonEcus, &pstEcuInfo->crcLibNumberList);
    if (ADM_IS_ERR(iRet))
    {
        log_e("Failed to get json value, key[%s], err[%d]", 
                    ADM_ECS_CFG_CRCLIBNUMBERLIST, iRet);
        return iRet;
    }


    //!< 解析ecu id列表信息
    iRet = __adm_ecs_parseEcuDidListInfo(jsonEcus, &pstEcuInfo->stEcuDidList);
    if (ADM_IS_ERR(iRet))
    {
        log_e("Failed to parse ResourceDid, key[%s], err[%d]", 
                    ADM_ECS_CFG_ECUDIDLIST, iRet);
        return iRet;
    }

    //!< 取扩展字段解析
    iRet = adm_com_parseJsonExtsInfo(jsonEcus, &pstEcuInfo->stExtList);
    if (ADM_IS_ERR(iRet))
    {
        log_e("Failed to parse exts, key[%s], err[%d]", 
                    ADM_COM_AIR_PROTOCOL_FIELD_EXT, iRet);
        return iRet;
    }

    pstEcuInfo->stExternEcuInfo.partName = adm_com_getJsonStrVal(jsonEcus, ADM_COM_AIR_PROTOCOL_FIELD_PARTNAME);
    if (ADM_IS_NULL(pstEcuInfo->stExternEcuInfo.partName))
    {
        log_e("Failed to get json value, key[%s]", ADM_COM_AIR_PROTOCOL_FIELD_PARTNAME);
        return ADM_ECODE_JSON_GET_STRING_FAILED;
    }

    pstEcuInfo->stExternEcuInfo.partNumber = adm_com_getJsonStrVal(jsonEcus, ADM_COM_AIR_PROTOCOL_FIELD_PARTNUM);
    if (ADM_IS_NULL(pstEcuInfo->stExternEcuInfo.partNumber))
    {
        log_e("Failed to get json value, key[%s]", ADM_COM_AIR_PROTOCOL_FIELD_PARTNUM);
        return ADM_ECODE_JSON_GET_STRING_FAILED;
    }

    pstEcuInfo->stExternEcuInfo.partSwid = adm_com_getJsonStrVal(jsonEcus, ADM_COM_AIR_PROTOCOL_FIELD_PARTSWID);
    if (ADM_IS_NULL(pstEcuInfo->stExternEcuInfo.partSwid))
    {
        log_e("Failed to get json value, key[%s]", ADM_COM_AIR_PROTOCOL_FIELD_PARTSWID);
        return ADM_ECODE_JSON_GET_STRING_FAILED;
    }

    pstEcuInfo->stExternEcuInfo.softwarePartNumber = adm_com_getJsonStrVal(jsonEcus, ADM_COM_AIR_PROTOCOL_FIELD_SOFTWAREPARTNUMBER);
    if (ADM_IS_NULL(pstEcuInfo->stExternEcuInfo.softwarePartNumber))
    {
        log_i("key[%s] option", ADM_COM_AIR_PROTOCOL_FIELD_SOFTWAREPARTNUMBER);
    }

    pstEcuInfo->stExternEcuInfo.partSupplier = adm_com_getJsonStrVal(jsonEcus, ADM_COM_AIR_PROTOCOL_FIELD_PARTSUPPLIER);
    if (ADM_IS_NULL(pstEcuInfo->stExternEcuInfo.partSupplier))
    {
        log_e("Failed to get json value, key[%s]", ADM_COM_AIR_PROTOCOL_FIELD_PARTSUPPLIER);
        return ADM_ECODE_JSON_GET_STRING_FAILED;
    }

    pstEcuInfo->stExternEcuInfo.partHardwareNum = adm_com_getJsonStrVal(jsonEcus, ADM_COM_AIR_PROTOCOL_FIELD_PARTHARDWARENUM);
    if (ADM_IS_NULL(pstEcuInfo->stExternEcuInfo.partHardwareNum))
    {
        log_e("Failed to get json value, key[%s]", ADM_COM_AIR_PROTOCOL_FIELD_PARTHARDWARENUM);
        return ADM_ECODE_JSON_GET_STRING_FAILED;
    }

    pstEcuInfo->stExternEcuInfo.upgradeOrder = adm_com_getJsonStrVal(jsonEcus, ADM_COM_AIR_PROTOCOL_FIELD_UPGRADEORDER);
    if (ADM_IS_NULL(pstEcuInfo->stExternEcuInfo.upgradeOrder))
    {
        log_e("Failed to get json value, key[%s]", ADM_COM_AIR_PROTOCOL_FIELD_UPGRADEORDER);
        return ADM_ECODE_JSON_GET_STRING_FAILED;
    }

    return iRet;
}

#if 0
#endif

/*!
 *******************************************************************************
 * 请求参数用json序列化.
 *
 * \note 此函数不添加sign信息
 * 
 * \param[out]  pstReqParam 模块基础配置参数
 * \retval      DCHAR*
 *******************************************************************************
 */
static DCHAR* __adm_ecs_createJsonValConfigData(ADM_ECS_REQPARAM_T *pstReqParam)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    DCHAR szEventBuf[ADM_COM_LEN_256] = {0};  //mod by wl 20220318
    DCHAR *messageBody = NULL;
    json_object *getCfgObject = NULL;
    json_object* stringObject = NULL;
    json_object* d64Object = NULL;

    //mod by wl 20220318
    (void)adm_memzero(szEventBuf, sizeof(szEventBuf));
    adm_ecs_eventReport(ADM_COM_MOD_ECS, FALSE, ADM_EVENT_ID_GI_010, ADM_OCS_EVENT_LEVEL_TRACE, "vehicle create get config request body msg.", NULL);

    if (ADM_IS_NULL(pstReqParam))
    {
        log_e("Invalid input param");
        //mod by wl 20220318
        adm_ecs_eventReport(ADM_COM_MOD_ECS, TRUE, ADM_EVENT_ID_GI_011, ADM_OCS_EVENT_LEVEL_ERROR, 
                "vehicle create get config request body msg failed!", "Invalid input param, [pstReqParam].");
        return NULL;
    }

    getCfgObject = json_object_new_object();
    if (ADM_IS_NULL(getCfgObject))
    {
        log_e("Faile to new getCfgObject json, err[%d]", ADM_ECODE_NEW_JSON_OBJECT_FAILED);
        //mod by wl 20220318
        (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), 
                "Faile to new getCfgObject json, err[%d]", ADM_ECODE_NEW_JSON_OBJECT_FAILED);
        adm_ecs_eventReport(ADM_COM_MOD_ECS, TRUE, ADM_EVENT_ID_GI_011, ADM_OCS_EVENT_LEVEL_ERROR, 
                "vehicle create get config request body msg failed!", szEventBuf);
        return NULL;
    }

    do
    {
        //!< add timestamp
        d64Object = json_object_new_int64(pstReqParam->timestamp);
        if (ADM_IS_NULL(d64Object)) 
        {
            log_e("Failed to new timestamp json object");
            iRet = ADM_ECODE_NEW_JSON_OBJECT_FAILED;
            //mod by wl 20220318
            (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), "Failed to new timestamp json object, err[%d]", iRet);
            break;
        }
        iRet = json_object_object_add(getCfgObject, ADM_COM_AIR_PROTOCOL_FIELD_TIMESTAMP, d64Object);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to add timestamp json object");
            iRet = ADM_ECODE_JSON_ADD_INT64_FAILED;
            //mod by wl 20220318
            (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), "Failed to add timestamp json object, err[%d]", iRet);
            break;
        }

        //!< add sdkVersion
        stringObject = NULL;
        if (NULL != pstReqParam->sdkVersion)
        {
            stringObject = json_object_new_string(pstReqParam->sdkVersion);
        }
        if (ADM_IS_NULL(stringObject)) 
        {
            log_e("Failed to new sdkVersion json object");
            iRet = ADM_ECODE_NEW_JSON_OBJECT_FAILED;
            //mod by wl 20220318
            (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), "Failed to new sdkVersion json object, err[%d]", iRet);
            break;
        }
        iRet = json_object_object_add(getCfgObject, ADM_COM_AIR_PROTOCOL_FIELD_SDKVERSION, stringObject);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to add sdkVersion json object");
            iRet = ADM_ECODE_JSON_ADD_STRING_FAILED;
            //mod by wl 20220318
            (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), "Failed to add sdkVersion json object, err[%d]", iRet);
            break;
        }

        //!< json to string
        messageBody = adm_strdup(json_object_to_json_string(getCfgObject));
        if (ADM_IS_NULL(messageBody))
        {
            log_e("Failed to json to string");
            iRet = ADM_ECODE_JSON_TO_STRING_FAILED;
            //mod by wl 20220318
            (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), "Failed to json to string, err[%d]", iRet);
            break;
        }

        // TODO: exts 扩展参数有需要时使用

    } while (FALSE);

    (void)json_object_put(getCfgObject);
    if (ADM_IS_ERR(iRet))
    {
        adm_ecs_eventReport(ADM_COM_MOD_ECS, TRUE, ADM_EVENT_ID_GI_011, ADM_OCS_EVENT_LEVEL_ERROR, 
                "vehicle create get config request body msg failed!", szEventBuf);
        return NULL;
    }

    //mod by wl 20220318
    adm_ecs_eventReport(ADM_COM_MOD_ECS, FALSE, ADM_EVENT_ID_GI_012, ADM_OCS_EVENT_LEVEL_DEBUG, 
                "vehicle create get config request body msg successed.", messageBody);

    return messageBody;
}

/*!
 *******************************************************************************
 * 请求参数用json序列化.
 *
 * \note 此函数不添加sign信息
 * 
 * \param[out]  pstReqParam 模块基础配置参数
 * \retval      DCHAR*
 *******************************************************************************
 */
static DCHAR* __adm_ecs_configDataJsonAddSign(DCHAR *messageBody, DCHAR *msgSign)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    DCHAR *signMsg = NULL;
    json_object* jsonRoot = NULL;
    json_object* stringObject = NULL;

    if (ADM_IS_NULL(messageBody) || ADM_IS_NULL(msgSign))
    {
        log_e("Invalid input param");
        return NULL;
    }

    jsonRoot = json_tokener_parse(messageBody);
    if (ADM_IS_NULL(jsonRoot)) 
    {
        log_e("Failed string to json, string[%s]", messageBody);
        return NULL;
    }

    //!< add sign
    stringObject = json_object_new_string(msgSign);
    if (ADM_IS_NULL(stringObject)) 
    {
        log_e("Failed to new sign json object, err[%d]", 
            ADM_ECODE_NEW_JSON_OBJECT_FAILED);
        (void)json_object_put(jsonRoot);
        return NULL;
    }
    iRet = json_object_object_add(jsonRoot, ADM_COM_AIR_PROTOCOL_FIELD_SIGN, stringObject);
    if (ADM_IS_ERR(iRet))
    {
        log_e("Failed to add sign json object, err[%d]", 
            ADM_ECODE_JSON_ADD_STRING_FAILED);
        (void)json_object_put(stringObject);
        (void)json_object_put(jsonRoot);
        return NULL;
    }

    //!< json to string
    signMsg = adm_strdup(json_object_to_json_string(jsonRoot));
    if (ADM_IS_NULL(signMsg))
    {
        log_e("Failed to json to string, err[%d]", 
                ADM_ECODE_JSON_TO_STRING_FAILED);
        (void)json_object_put(jsonRoot);
        return NULL;
    }
    (void)json_object_put(jsonRoot);

    return signMsg;
}

/*!
 *******************************************************************************
 * 请求参数用json序列化.
 *
 * \note 此函数不添加sign信息
 * 
 * \param[out]  pstReqParam 模块基础配置参数
 * \retval      DCHAR*
 *******************************************************************************
 */
static DCHAR* __adm_ecs_configDataJsonGetSign(DCHAR *messageBody)
{
    DCHAR *signMsg = NULL;
    json_object* jsonRoot = NULL;
    json_object *jsonStr = NULL;

    if (ADM_IS_NULL(messageBody))
    {
        log_e("Invalid input param");
        return NULL;
    }

    jsonRoot = json_tokener_parse(messageBody);
    if (ADM_IS_NULL(jsonRoot)) 
    {
        log_e("Failed string to json, string[%s]", messageBody);
        return NULL;
    }

    if (!json_object_object_get_ex(jsonRoot, ADM_COM_AIR_PROTOCOL_FIELD_SIGN, &jsonStr))
    {
        log_e("Failed to get json key[%s] value", ADM_COM_AIR_PROTOCOL_FIELD_SIGN);
        return NULL;
    }

    signMsg = adm_strdup(json_object_get_string(jsonStr));
    (void)json_object_put(jsonStr);
    (void)json_object_put(jsonRoot);

    return signMsg;
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
static D32S __adm_ecs_signAndEnCrypt
(
    DCHAR *messageBody, 
    DCHAR *secret, 
    DCHAR *deviceId,
    DCHAR **enMessageBody
)
{
    DCHAR *msgSign = NULL;
    DCHAR *signMsg = NULL;
    DCHAR szEventBuf[ADM_COM_LEN_256] = {0};  //mod by wl 20220318

    if (ADM_IS_NULL(messageBody) || ADM_IS_NULL(secret) || ADM_IS_NULL(deviceId) || ADM_IS_NULL(enMessageBody))
    {
        log_e("Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }

    //mod by wl 20220318
    adm_ecs_eventReport(ADM_COM_MOD_ECS, FALSE, ADM_EVENT_ID_GI_013, ADM_OCS_EVENT_LEVEL_TRACE, "vehicle sign config request body msg.", messageBody);

    //!< 计算msg的sign
    msgSign = adm_ocs_ariProtocolSign(messageBody, secret, deviceId);
    if (ADM_IS_NULL(msgSign))
    {
        log_e("Failed to calculate message sign, err[%d]",
            ADM_ECODE_OTACFG_MESSAGE_SIGN_FAILED);
        //mod by wl 20220318
        (void)adm_memzero(szEventBuf, sizeof(szEventBuf));
        (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), 
                "Failed to calculate message sign, err[%d]", ADM_ECODE_OTACFG_MESSAGE_SIGN_FAILED);
        adm_ecs_eventReport(ADM_COM_MOD_ECS, TRUE, ADM_EVENT_ID_GI_014, ADM_OCS_EVENT_LEVEL_ERROR, 
                "vehicle sign get config request body msg failed!", szEventBuf);
        return ADM_ECODE_OTACFG_MESSAGE_SIGN_FAILED;
    }

    //mod by wl 20220318
    adm_ecs_eventReport(ADM_COM_MOD_ECS, FALSE, ADM_EVENT_ID_GI_015, ADM_OCS_EVENT_LEVEL_DEBUG, "vehicle sign config request body msg successed.", msgSign);

    //mod by wl 20220318
    adm_ecs_eventReport(ADM_COM_MOD_ECS, FALSE, ADM_EVENT_ID_GI_016, ADM_OCS_EVENT_LEVEL_TRACE, "vehicle config request body add sign info.", messageBody);

    //!< msg中添加sign
    signMsg = __adm_ecs_configDataJsonAddSign(messageBody, msgSign);
    if (ADM_IS_NULL(signMsg))
    {
        adm_free(msgSign);
        log_e("Failed to add sign json to message");

        //mod by wl 20220318
        (void)adm_memzero(szEventBuf, sizeof(szEventBuf));
        (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), "vehicle config request body add sign info failed, err[%d]", ADM_ECODE_JSON_ADD_STRING_FAILED);
        adm_ecs_eventReport(ADM_COM_MOD_ECS, TRUE, ADM_EVENT_ID_GI_017, ADM_OCS_EVENT_LEVEL_ERROR, szEventBuf, NULL);

        return ADM_ECODE_JSON_ADD_STRING_FAILED;
    }
    adm_free(msgSign);
    //mod by wl 20220318
    adm_ecs_eventReport(ADM_COM_MOD_ECS, FALSE, ADM_EVENT_ID_GI_018, ADM_OCS_EVENT_LEVEL_DEBUG, "vehicle config request body add sign info successed.", signMsg);


    // TODO: 未加密
    *enMessageBody = signMsg;
    log_d("===> signMsg[%s]", signMsg);

#if 0
    //mod by wl 20220318
    adm_ecs_eventReport(ADM_COM_MOD_ECS, FALSE, ADM_EVENT_ID_GI_019, ADM_OCS_EVENT_LEVEL_TRACE, "vehicle encrypt config request body info.", signMsg);
    //!< msg加密
    *enMessageBody = (DCHAR *)adm_iss_rsaPubEncrypt((D8U *)secret, (D8U *)signMsg, RSA_PKCS1_PADDING);
    if (ADM_IS_NULL(*enMessageBody))
    {
        adm_free(msgSign);
        adm_free(signMsg);
        log_e("Failed to enCrypt signMsg");

        //mod by wl 20220318
        (void)adm_memzero(szEventBuf, sizeof(szEventBuf));
        adm_snprintf(szEventBuf, sizeof(szEventBuf), "vehicle encrypt config request body info failed! err[%d]", ADM_ECODE_CHECK_ENCRYPT_FAILED);
        adm_ecs_eventReport(ADM_COM_MOD_ECS, TRUE, ADM_EVENT_ID_GI_020, ADM_OCS_EVENT_LEVEL_ERROR, szEventBuf, NULL);

        return ADM_ECODE_CHECK_ENCRYPT_FAILED;
    }
    adm_free(msgSign);
    adm_free(signMsg);

    //mod by wl 20220318
    adm_ecs_eventReport(ADM_COM_MOD_ECS, FALSE, ADM_EVENT_ID_GI_021, ADM_OCS_EVENT_LEVEL_DEBUG, "vehicle encrypt config request body info successed.", *enMessageBody);
#endif

    return ADM_ECODE_SUCCESSED;
}

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
static D32S __adm_ecs_deCryptAndSign
(
    DCHAR *enMessage, 
    DCHAR *secret,
    DCHAR *deviceId,
    DCHAR **deMessage
)
{
    DCHAR *msgSign    = NULL;
    DCHAR *originSign = NULL;
    DCHAR szEventBuf[ADM_COM_LEN_256] = {0};  //mod by wl 20220318

    if (ADM_IS_NULL(enMessage) || ADM_IS_NULL(secret) || ADM_IS_NULL(deMessage))
    {
        log_e("Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }

    //mod by wl 20220318
    adm_ecs_eventReport(ADM_COM_MOD_ECS, FALSE, ADM_EVENT_ID_GI_028, ADM_OCS_EVENT_LEVEL_TRACE, "vehicle decrypt config respond body msg", enMessage);

    //!< 解密msg
    *deMessage = (DCHAR *)adm_iss_rsaPubDecrypt((D8U *)secret, (D8U *)enMessage, RSA_PKCS1_PADDING);
    if (ADM_IS_NULL(*deMessage))
    {
        log_e("Failed to deCrypt signMsg");

        //mod by wl 20220318
        (void)adm_memzero(szEventBuf, sizeof(szEventBuf));
        (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), 
                "Failed to deCrypt signMsg, err[%d]", ADM_ECODE_CHECK_DECRYPT_FAILED);
        adm_ecs_eventReport(ADM_COM_MOD_ECS, TRUE, ADM_EVENT_ID_GI_029, ADM_OCS_EVENT_LEVEL_ERROR, "vehicle decrypt config respond body msg failed!", szEventBuf);

        return ADM_ECODE_CHECK_DECRYPT_FAILED;
    }
    //mod by wl 20220318
    adm_ecs_eventReport(ADM_COM_MOD_ECS, FALSE, ADM_EVENT_ID_GI_030, ADM_OCS_EVENT_LEVEL_DEBUG, "vehicle decrypt config respond body msg successed.", *deMessage);

    //mod by wl 20220318
    adm_ecs_eventReport(ADM_COM_MOD_ECS, FALSE, ADM_EVENT_ID_GI_031, ADM_OCS_EVENT_LEVEL_TRACE, "vehicle calc config respond body msg sign.", *deMessage);

    //!< 计算msg的sign
    msgSign = adm_ocs_ariProtocolSign(*deMessage, secret, NULL);
    if (ADM_IS_NULL(msgSign))
    {
        adm_free(*deMessage);
        log_e("Failed to calculate message sign");

        //mod by wl 20220318
        (void)adm_memzero(szEventBuf, sizeof(szEventBuf));
        (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), "Failed to calculate message sign, err[%d]", ADM_ECODE_CHECK_ENCRYPT_FAILED);
        adm_ecs_eventReport(ADM_COM_MOD_ECS, TRUE, ADM_EVENT_ID_GI_032, ADM_OCS_EVENT_LEVEL_ERROR, 
                "vehicle calc config respond body msg sign failed!", szEventBuf);

        return ADM_ECODE_CHECK_ENCRYPT_FAILED;
    }
    //mod by wl 20220318
    adm_ecs_eventReport(ADM_COM_MOD_ECS, FALSE, ADM_EVENT_ID_GI_033, ADM_OCS_EVENT_LEVEL_DEBUG, "vehicle calc config respond body msg sign successed.", msgSign);

    //mod by wl 20220318
    adm_ecs_eventReport(ADM_COM_MOD_ECS, FALSE, ADM_EVENT_ID_GI_034, ADM_OCS_EVENT_LEVEL_TRACE, "vehicle get config respond body msg sign.", *deMessage);

    //!< 获取msg中的sign
    originSign = __adm_ecs_configDataJsonGetSign(*deMessage);
    if (ADM_IS_NULL(originSign))
    {
        adm_free(msgSign);
        adm_free(*deMessage);
        log_e("Failed to get sign in json data");

        //mod by wl 20220318
        (void)adm_memzero(szEventBuf, sizeof(szEventBuf));
        (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), 
                "Failed to get sign in json data, err[%d]", ADM_ECODE_JSON_GET_STRING_FAILED);
        adm_ecs_eventReport(ADM_COM_MOD_ECS, TRUE, ADM_EVENT_ID_GI_035, ADM_OCS_EVENT_LEVEL_ERROR, 
                "vehicle get config respond body msg sign failed!", szEventBuf);

        return ADM_ECODE_JSON_GET_STRING_FAILED;
    }
    //mod by wl 20220318
    adm_ecs_eventReport(ADM_COM_MOD_ECS, FALSE, ADM_EVENT_ID_GI_036, ADM_OCS_EVENT_LEVEL_DEBUG, "vehicle get config respond body msg sign successed.", originSign);

    //mod by wl 20220318
    (void)adm_memzero(szEventBuf, sizeof(szEventBuf));
    (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), "originSign[%s], sign[%s]", originSign, msgSign);
    adm_ecs_eventReport(ADM_COM_MOD_ECS, FALSE, ADM_EVENT_ID_GI_037, ADM_OCS_EVENT_LEVEL_TRACE, "vehicle cmp config respond body msg sign.", szEventBuf);

    //!< 对比计算的sign与msg中的sign是否一致
    if (0 != adm_strcasecmp(originSign, msgSign))
    {
        adm_free(msgSign);
        adm_free(originSign);
        adm_free(*deMessage);
        log_e("Failed to check sign, originSign[%s], sign[%s], err[%d]", originSign, msgSign, ADM_ECODE_CHECK_RESPMESSAGE_SIGN_FAILED);

        //mod by wl 20220318
        (void)adm_memzero(szEventBuf, sizeof(szEventBuf));
        (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), 
                "originSign[%s], sign[%s], err[%d]", originSign, msgSign, ADM_ECODE_CHECK_RESPMESSAGE_SIGN_FAILED);
        adm_ecs_eventReport(ADM_COM_MOD_ECS, TRUE, ADM_EVENT_ID_GI_038, ADM_OCS_EVENT_LEVEL_ERROR, 
                "vehicle cmp config respond body msg sign failed!", szEventBuf);

        return ADM_ECODE_CHECK_RESPMESSAGE_SIGN_FAILED;
    }
    //mod by wl 20220318
    adm_ecs_eventReport(ADM_COM_MOD_ECS, FALSE, ADM_EVENT_ID_GI_039, ADM_OCS_EVENT_LEVEL_DEBUG, "vehicle cmp config respond body msg sign successed.", NULL);

    adm_free(msgSign);
    adm_free(originSign);

    return ADM_ECODE_SUCCESSED;
}



#if 0
#endif

/*!
 *******************************************************************************
 * ecs创建请求平台配置消息体.
 *
 * \note
 * 
 * \param[in]   pstReqParam 请求参数
 * \param[out]  messageBody 创建好的消息体
 * \retval      D32S
 *******************************************************************************
 */
D32S adm_ecs_createMessageBody(ADM_ECS_REQPARAM_T *pstReqParam, DCHAR **messageBody)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    DCHAR *msgJson = NULL;
    DCHAR szEventBuf[ADM_COM_LEN_256] = {0};  //mod by wl 20220318

    //mod by wl 20220318
    (void)adm_memzero(szEventBuf, sizeof(szEventBuf));
    adm_ecs_eventReport(ADM_COM_MOD_ECS, FALSE, ADM_EVENT_ID_GI_007, ADM_OCS_EVENT_LEVEL_TRACE, "vehicle create get config request message.", NULL);

    do
    {
        if (ADM_IS_NULL(pstReqParam) || ADM_IS_NULL(messageBody))
        {
            iRet = ADM_ECODE_INVALID_PARAM;
            log_e("Invalid input param");
            //mod by wl 20220318
            (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), "Invalid input param, err[%d].", ADM_ECODE_INVALID_PARAM);
            break;
        }
        
        if (ADM_IS_NULL(pstReqParam->sdkVersion))
        {
            iRet = ADM_ECODE_INVALID_PARAM;
            log_e("createMsg sdkVersion is null");
            //mod by wl 20220318
            (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), "createMsg sdkVersion is null, err[%d].", ADM_ECODE_INVALID_PARAM);
            break;
        }
        
        if (0 == pstReqParam->timestamp)
        {
            iRet = ADM_ECODE_INVALID_PARAM;
            log_e("createMsg timestamp is zero");
            //mod by wl 20220318
            (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), "createMsg timestamp is zero, err[%d].", ADM_ECODE_INVALID_PARAM);
            break;
        }
        
        msgJson = __adm_ecs_createJsonValConfigData(pstReqParam);
        if (ADM_IS_NULL(msgJson))
        {
            iRet = ADM_ECODE_NO_MEMORY;
            log_e("Failed to create json from config data, err[%d]", iRet);
            //mod by wl 20220318
            (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), "Failed to create json from config data, err[%d].", iRet);
            break;
        }
        *messageBody = msgJson;
        
        //!< 暂时取消http签名与验签
        iRet = __adm_ecs_signAndEnCrypt(msgJson, pstReqParam->secret, 
                    pstReqParam->deviceId, messageBody);
        if (ADM_IS_ERR(iRet))
        {
            adm_free(msgJson);
            log_e("Failed to signAndEncrypt message, err[%d]", iRet);        
            //mod by wl 20220318
            (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), "Failed to signAndEncrypt message, err[%d].", iRet);
            break;
        }
        adm_free(msgJson);
    }while (FALSE);

    if (ADM_IS_ERR(iRet))
    {
        adm_ecs_eventReport(ADM_COM_MOD_ECS, TRUE, ADM_EVENT_ID_GI_008, ADM_OCS_EVENT_LEVEL_ERROR, 
                    "vehicle create get config request message failed!", szEventBuf);
    }
    else
    {
        //mod by wl 20220318
        adm_ecs_eventReport(ADM_COM_MOD_ECS, FALSE, ADM_EVENT_ID_GI_009, ADM_OCS_EVENT_LEVEL_DEBUG, 
                    "vehicle create get config request message successed.", *messageBody);
    }
    
    return ADM_ECODE_SUCCESSED;
}


/*!
 *******************************************************************************
 * ecs解析平台响应的消息.
 *
 * \note
 * 
 * \param[in]   messageBody   平台响应的消息体
 * \param[in]   pstReqParam   请求参数(secret、deviceId)
 * \param[out]  pstRsCfgData  平台响应的配置结构化数据
 * \retval      D32S
 *******************************************************************************
 */
D32S adm_ecs_parseMessageBody
(
    DCHAR                  *messageBody, 
    ADM_ECS_REQPARAM_T     *pstReqParam,
    ADM_ECS_RSCONFIGDATA_T *pstRsCfgData
)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    D32S status = 0;
    DCHAR buf[ADM_COM_FILEPATH_LEN] = {0};
    DCHAR szEventBuf[ADM_COM_LEN_256] = {0};  //mod by wl 20220318
    DCHAR *deMsgBody = NULL;
    json_object* jsonRoot = NULL;
    json_object *jsonData = NULL;
    json_object *jsonFileObj = NULL;

    json_type jsonType = json_type_null;

    if (ADM_IS_NULL(messageBody) || ADM_IS_NULL(pstReqParam) || ADM_IS_NULL(pstRsCfgData))
    {
        log_e("Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }

    //mod by wl 20220318
    adm_ecs_eventReport(ADM_COM_MOD_ECS, FALSE, ADM_EVENT_ID_GI_025, ADM_OCS_EVENT_LEVEL_TRACE, "vehicle parse get config respond info.", messageBody);

    // TODO: debug暂时取消解密与验签
    #if 0
    iRet = __adm_ecs_deCryptAndSign(messageBody, pstReqParam->secret, 
                pstReqParam->deviceId, &deMsgBody);
    if (ADM_IS_ERR(iRet) || ADM_IS_NULL(deMsgBody))
    {
        log_e("Failed to signAndEncrypt message, err[%d]", iRet);

        //mod by wl 20220318
        (void)adm_memzero(szEventBuf, sizeof(szEventBuf));
        (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), 
                "Failed to signAndEncrypt message, err[%d]", iRet);
        adm_ecs_eventReport(ADM_COM_MOD_ECS, TRUE, ADM_EVENT_ID_GI_026, ADM_OCS_EVENT_LEVEL_ERROR, 
                "vehicle parse get config respond info failed!", szEventBuf);

        return iRet;
    }
    jsonRoot = json_tokener_parse(deMsgBody);
    #endif

    //mod by wl 20220318
    adm_ecs_eventReport(ADM_COM_MOD_ECS, FALSE, ADM_EVENT_ID_GI_040, ADM_OCS_EVENT_LEVEL_TRACE, "vehicle parse config respond body msg.", messageBody);

    log_d("===> messageBody[%s]", messageBody);
    jsonRoot = json_tokener_parse(messageBody);
    if (ADM_IS_NULL(jsonRoot)) 
    {
        adm_free(deMsgBody);
        log_e("Failed string to json, string[%s]", deMsgBody);

        //mod by wl 20220318
        (void)adm_memzero(szEventBuf, sizeof(szEventBuf));
        (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), "Failed string to json, string[%s]", deMsgBody);
        adm_ecs_eventReport(ADM_COM_MOD_ECS, TRUE, ADM_EVENT_ID_GI_026, ADM_OCS_EVENT_LEVEL_ERROR, 
                "vehicle parse get config respond info failed!", szEventBuf);

        return ADM_ECODE_STRING_TO_JSON_FAILED;
    }
    adm_free(deMsgBody);

    do 
    {
        //mod by wl 20220318
        (void)adm_memzero(szEventBuf, sizeof(szEventBuf));

        //!< 先校验msg响应status
        iRet = adm_com_getJsonIntVal(jsonRoot, ADM_COM_AIR_PROTOCOL_FIELD_STATUS, &status);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to get json value, key[%s], err[%d]", 
                        ADM_COM_AIR_PROTOCOL_FIELD_STATUS, iRet);
            //mod by wl 20220318
            (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), "Failed to get json value, key[%s], err[%d]", 
                            ADM_COM_AIR_PROTOCOL_FIELD_STATUS, iRet);
            break;
        }
        
        //!< 解析云端响应的状态码
        iRet = adm_ocs_otaStatusCodeTransfer(status);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to check status[%d], iRet[%d]", status, iRet);
            //mod by wl 20220318
            (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), "Failed to check status[%d], err[%d]", status, iRet);
            break;
        }
        
        //!< 解析msg中data
        if (!json_object_object_get_ex(jsonRoot, ADM_COM_AIR_PROTOCOL_FIELD_DATA, &jsonData))
        {
            log_e("Failed to get json value, key[%s]", 
                        ADM_COM_AIR_PROTOCOL_FIELD_DATA);
            iRet = ADM_ECODE_JSON_GET_KEY_FAILED;
            //mod by wl 20220318
            (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), "Failed to get json value, key[%s], err[%d]", ADM_COM_AIR_PROTOCOL_FIELD_DATA, iRet);
            break;
        }
        
        jsonType = json_object_get_type(jsonData);
        if (jsonType != json_type_object)
        {
            log_e("Json key[%s] type error", ADM_COM_AIR_PROTOCOL_FIELD_DATA);
            iRet = ADM_ECODE_JSON_KEY_TYPE_ERROR;
            //mod by wl 20220318
            (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), "Json key[%s] type error, key[%s], err[%d]", ADM_COM_AIR_PROTOCOL_FIELD_DATA, iRet);
            break;
        }

        //!< 解析msg - data - confVersion
        pstRsCfgData->confVersion = adm_com_getJsonStrVal(jsonData, ADM_COM_AIR_PROTOCOL_FIELD_CONFVERSION);
        if (ADM_IS_NULL(pstRsCfgData->confVersion))
        {
            log_e("Failed to get json value, key[%s]", ADM_COM_AIR_PROTOCOL_FIELD_CONFVERSION);
            iRet = ADM_ECODE_JSON_GET_STRING_FAILED;
            //mod by wl 20220318
            (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), "Failed to get json value, key[%s], iRet[%d]", ADM_COM_AIR_PROTOCOL_FIELD_CONFVERSION, iRet);
            break;
        }
        
        //!< 解析msg - data - fileObject
        if (!json_object_object_get_ex(jsonData, ADM_COM_AIR_PROTOCOL_FIELD_FILEOBJECT, &jsonFileObj))
        {
            log_e("Failed to get json value, key[%s]", 
                        ADM_COM_AIR_PROTOCOL_FIELD_FILEOBJECT);
            iRet = ADM_ECODE_JSON_GET_KEY_FAILED;
            //mod by wl 20220318
            (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), "Failed to get json value, key[%s], err[%d]", ADM_COM_AIR_PROTOCOL_FIELD_FILEOBJECT, iRet);
            break;
        }
        
        jsonType = json_object_get_type(jsonFileObj);
        if (jsonType != json_type_object)
        {
            log_e("Json key[%s] type error", ADM_COM_AIR_PROTOCOL_FIELD_FILEOBJECT);
            iRet = ADM_ECODE_JSON_KEY_TYPE_ERROR;
            //mod by wl 20220318
            (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), "Json key[%s] type error, key[%s], err[%d]", ADM_COM_AIR_PROTOCOL_FIELD_FILEOBJECT, iRet);
            break;
        }
        
        iRet = adm_com_parseJsonFileObject(jsonFileObj, &pstRsCfgData->stRsCfgFileObj);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to parse rs cfg file object info, err[%d]", iRet);
            //mod by wl 20220318
            (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), "Failed to parse rs cfg file object info, err[%d]", iRet);
            break;
        }
        //!< 初始化配置文件存储路径
        (void)adm_memzero(buf, sizeof(buf));
        (void)adm_snprintf(buf, sizeof(buf), "%s%s", pstReqParam->confFileDir, pstRsCfgData->stRsCfgFileObj.fileName);
        pstRsCfgData->stRsCfgFileObj.filePath = adm_strdup(buf);
        log_d("ECS module confFilePath[%s]", buf);
    } while (FALSE);

    (void)json_object_put(jsonRoot);
    if (ADM_IS_ERR(iRet))
    {
        adm_ecs_eventReport(ADM_COM_MOD_ECS, TRUE, ADM_EVENT_ID_GI_026, ADM_OCS_EVENT_LEVEL_ERROR, 
                "vehicle parse config respond info failed!", szEventBuf);
    }
    else
    {
        adm_ecs_eventReport(ADM_COM_MOD_ECS, FALSE, ADM_EVENT_ID_GI_027, ADM_OCS_EVENT_LEVEL_DEBUG, 
                "vehicle parse config respond info successed.", NULL);
    }

    return iRet;
}

/*!
 *******************************************************************************
 * 解析ecus配置信息.
 *
 * \note
 * 
 * \param[in]   secret       解密密钥
 * \param[in]   ecusCfgData  ecus配置数据
 * \param[out]  pstEcsConfig ecus配置结构信息
 * \retval      D32S
 *******************************************************************************
 */
D32S adm_ecs_parsePartConfigInfo
(
    DCHAR                *secret,
    DCHAR                *ecusCfgData, 
    ADM_ECS_PARTCONFIG_T *pstPartCfg
)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    D32S i = 0;
    DCHAR* ecusCfgDataDec = NULL;
    DSIZE ecusCfgLengthDec = 0;
    json_type    jsonType = json_type_null;
    json_object *jsonRoot = NULL;
    json_object *jsonEcus = NULL;
    json_object *jsonArrayIndexObj = NULL;

    if (ADM_IS_NULL(secret) || ADM_IS_NULL(ecusCfgData) || ADM_IS_NULL(pstPartCfg))
    {
        log_e("Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }

    do
    {
        /** 对称解密出原始零件配置信息 */
        iRet = adm_parseConfigContext((const D8U*)secret, (const D8U*)ecusCfgData, adm_strlen(ecusCfgData), (D8U**)&ecusCfgDataDec, &ecusCfgLengthDec);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Call adm_parseConfigContext error.iRet[%d]", iRet);
            break;
        }

        jsonRoot = json_tokener_parse(ecusCfgDataDec);
        if (ADM_IS_NULL(jsonRoot)) 
        {
            log_e("Failed string to json, string[%s]", ecusCfgDataDec);
            iRet = ADM_ECODE_STRING_TO_JSON_FAILED;
            break;
        }

        //!< 取json中ecus节点信息
        if (!json_object_object_get_ex(jsonRoot, ADM_ECS_CFG_ECULIST, &jsonEcus))
        {
            log_e("Failed to get json value, key[%s]", ADM_ECS_CFG_ECULIST);
            iRet = ADM_ECODE_JSON_GET_KEY_FAILED;
            break;
        }

        jsonType = json_object_get_type(jsonEcus);
        if (jsonType != json_type_array)
        {
            log_e("Json key[%s] type error", ADM_ECS_CFG_ECULIST);
            iRet = ADM_ECODE_JSON_KEY_TYPE_ERROR;
            break;
        }

        pstPartCfg->stEcuCfgList.num = (D32S)json_object_array_length(jsonEcus);
        if (0 == pstPartCfg->stEcuCfgList.num)
        {
            log_e("Json key[%s] array is zero", ADM_ECS_CFG_ECULIST);
            iRet = ADM_ECODE_SUCCESSED;
            break;
        }

        pstPartCfg->stEcuCfgList.pstEcuCfg = adm_ecs_newEcusConfigList(pstPartCfg->stEcuCfgList.num);
        if (ADM_IS_NULL(pstPartCfg->stEcuCfgList.pstEcuCfg))
        {
            pstPartCfg->stEcuCfgList.num = 0;
            log_e("Failed to new ecu config list");
            iRet = ADM_ECODE_NO_MEMORY;
            break;
        }

        for (i = 0; i < pstPartCfg->stEcuCfgList.num; ++i) 
        {
            jsonArrayIndexObj = json_object_array_get_idx(jsonEcus, (D32U)i);
            if (ADM_IS_NULL(jsonArrayIndexObj)) 
            {
                iRet = ADM_ECODE_JSON_GET_ARRAYOBJECT_FAILED;
                log_e("Failed to backtracking Json array, arrayIndex[%d]", i);
                break;
            }

            iRet = __adm_ecs_parseEcuConfigInfo(jsonArrayIndexObj, &pstPartCfg->stEcuCfgList.pstEcuCfg[i]);
            if (ADM_IS_ERR(iRet))
            {
                log_e("Failed to parse saLib info, err[%d]", iRet);
                break;
            }
        }

        if (ADM_IS_ERR(iRet))
        {
            adm_ecs_delEcusConfigList(pstPartCfg->stEcuCfgList.num, &pstPartCfg->stEcuCfgList.pstEcuCfg);
            pstPartCfg->stEcuCfgList.num = 0;
            break;
        }

        //!< 取扩展字段解析
        iRet = adm_com_parseJsonExtsInfo(jsonRoot, &pstPartCfg->stExtList);
        if (ADM_IS_ERR(iRet))
        {
            adm_ecs_delEcusConfigList(pstPartCfg->stEcuCfgList.num, &pstPartCfg->stEcuCfgList.pstEcuCfg);
            pstPartCfg->stEcuCfgList.num = 0;
            pstPartCfg->stEcuCfgList.pstEcuCfg = NULL;
            log_e("Failed to parse exts, key[%s], err[%d]", 
                        ADM_COM_AIR_PROTOCOL_FIELD_EXT, iRet);
            break;
        }
    } while (FALSE);

    (void)json_object_put(jsonRoot);

    return iRet;
}



/*!
 *******************************************************************************
 * 解析ecus配置文件信息.
 *
 * \note
 * 
 * \param[in]   secret         解密密钥
 * \param[in]   configFilePath 零件配置文件路径
 * \param[out]  pstPartCfg     ecu零件配置信息
 * \retval      D32S
 *******************************************************************************
 */
D32S adm_ecs_parsePartConfigFile
(
    DCHAR                *secret,
    DCHAR                *configFilePath, 
    ADM_ECS_PARTCONFIG_T *pstPartCfg
)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    DCHAR *pcFileData = NULL;

    if (ADM_IS_NULL(secret) || ADM_IS_NULL(configFilePath) || ADM_IS_NULL(pstPartCfg))
    {
        log_e("Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }

    //!< 读 downloadPolicy 文件
    iRet = adm_com_readFileData(configFilePath, &pcFileData);
    if (ADM_IS_ERR(iRet))
    {
        log_e("Failed to read download policy file, err[%d]", iRet);
        return iRet;
    }

    //!< 解析零件配置文件信息
    iRet = adm_ecs_parsePartConfigInfo(secret, pcFileData, pstPartCfg);
    if (ADM_IS_ERR(iRet))
    {

        adm_free(pcFileData);
        log_e("Failed to parse part config, err[%d]", iRet);
        return iRet;
    }
    adm_free(pcFileData);

    return iRet;
}

/*!
 *******************************************************************************
 * 解析通用配置信息.
 *
 * \note
 * 
 * \param[in]   secret           解密密钥
 * \param[in]   confFileDir      配置文件目录
 * \param[in]   GeneralCfgData   通用配置数据
 * \param[out]  pstGeneralConfig 通用配置结构信息
 * \retval      D32S
 *******************************************************************************
 */
D32S adm_ecs_parseGeneralConfigInfo
(
    DCHAR                   *secret,
    DCHAR                   *confFileDir,
    DCHAR                   *generalCfgData, 
    ADM_ECS_GENERALCONFIG_T *pstGeneralConfig
)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    DSIZE uiGeneralCfgLengthDec;
    DCHAR* pGeneralCfgDataDec = NULL;
    json_object *jsonRoot = NULL;

    if (ADM_IS_NULL(secret) || ADM_IS_NULL(confFileDir) || ADM_IS_NULL(generalCfgData) || ADM_IS_NULL(pstGeneralConfig))
    {
        log_e("Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }

    do
    {
        /** 对称解密出原始通用配置信息 */
        iRet = adm_parseConfigContext((const D8U*)secret, (const D8U*)generalCfgData, adm_strlen(generalCfgData), 
                                (D8U**)&pGeneralCfgDataDec, &uiGeneralCfgLengthDec);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Call adm_parseConfigContext error.iRet[%d]", iRet);
            break;
        }
        
        jsonRoot = json_tokener_parse(pGeneralCfgDataDec);
        if (ADM_IS_NULL(jsonRoot)) 
        {
            log_e("Failed string to json, string[%s]", generalCfgData);
            iRet = ADM_ECODE_STRING_TO_JSON_FAILED;
            break;
        }

        pstGeneralConfig->protocolVersion = adm_com_getJsonStrVal(jsonRoot, ADM_ECS_CFG_PTLVERSION);
        if (ADM_IS_NULL(pstGeneralConfig->protocolVersion))
        {
            iRet = ADM_ECODE_JSON_GET_STRING_FAILED;
            log_e("Failed to get json value, key[%s]", ADM_ECS_CFG_PTLVERSION);
            break;
        }

        //!< get logReportUrl --- 可选字段 
        pstGeneralConfig->logReportUrl = adm_com_getJsonStrVal(jsonRoot, ADM_ECS_CFG_LOGREPORTURL);
        if (ADM_IS_NULL(pstGeneralConfig->logReportUrl))
        {
            log_i("Failed to get json value, key[%s]", ADM_ECS_CFG_LOGREPORTURL);
        }

        //!< get pollingCycle
        iRet = adm_com_getJsonIntVal(jsonRoot, ADM_ECS_CFG_POLLINGCYCLE, &pstGeneralConfig->pollingCycle);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to get json value, key[%s], err[%d]", 
                        ADM_ECS_CFG_POLLINGCYCLE, iRet);
            break;
        }
        
        iRet = adm_com_getJsonIntVal(jsonRoot, ADM_ECS_CFG_LOGREPORTCYC, &pstGeneralConfig->logReportCycle);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to get json value, key[%s], err[%d]", 
                        ADM_ECS_CFG_LOGREPORTCYC, iRet);
            break;
        }
    
        iRet = adm_com_getJsonIntVal(jsonRoot, ADM_ECS_CFG_LOGPRINTLEVEL, &pstGeneralConfig->logPrintLevel);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to get json value, key[%s], err[%d]", 
                        ADM_ECS_CFG_LOGPRINTLEVEL, iRet);
            break;
        }
    
        iRet = adm_com_getJsonIntVal(jsonRoot, ADM_ECS_CFG_ISLOGFILEENCRY, &pstGeneralConfig->isLogFileEncrypted);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to get json value, key[%s], err[%d]", 
                        ADM_ECS_CFG_ISLOGFILEENCRY, iRet);
            break;
        }
    
        iRet = adm_com_getJsonIntVal(jsonRoot, ADM_ECS_CFG_ISREPORTUPSUCCLOGFILE, 
                                        &pstGeneralConfig->isReportUpSuccLogFile);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to get json value, key[%s], err[%d]", 
                        ADM_ECS_CFG_ISREPORTUPSUCCLOGFILE, iRet);
            break;
        }

		iRet = adm_com_getJsonIntVal(jsonRoot, ADM_ECS_CFG_EVENTREPORTLEVEL, &pstGeneralConfig->eventReportLevel);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to get json value, key[%s], err[%d]", 
                        ADM_ECS_CFG_EVENTREPORTLEVEL, iRet);
#if 1 //for debug,20220722
            pstGeneralConfig->eventReportLevel = ADM_OCS_EVENT_LEVEL_INFO;
			log_i("for debug use sample data, key[%s], pstGeneralConfig->eventReportLevel[%d]", 
                        ADM_ECS_CFG_EVENTREPORTLEVEL, pstGeneralConfig->eventReportLevel);
#else
            break;
#endif
        }
    
        iRet = adm_com_getJsonIntVal(jsonRoot, ADM_ECS_CFG_EVENTREPORTCYC, &pstGeneralConfig->eventReportCycle);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to get json value, key[%s], err[%d]", 
                        ADM_ECS_CFG_EVENTREPORTCYC, iRet);
            break;
        }
    
        iRet = adm_com_getJsonIntVal(jsonRoot, ADM_ECS_CFG_STATEREPORTCYC, &pstGeneralConfig->stateReportCycle);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to get json value, key[%s], err[%d]", 
                        ADM_ECS_CFG_STATEREPORTCYC, iRet);
            break;
        }

        iRet = adm_com_getJsonIntVal(jsonRoot, ADM_ECS_CFG_SUMMARYREPORTCYC, &pstGeneralConfig->summaryReportCycle);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to get json value, key[%s], err[%d]", 
                        ADM_ECS_CFG_SUMMARYREPORTCYC, iRet);
            break;
        }
    
        iRet = adm_com_getJsonIntVal(jsonRoot, ADM_ECS_CFG_PROGRESSREPORTCYC, &pstGeneralConfig->progressReportCycle);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to get json value, key[%s], err[%d]", 
                        ADM_ECS_CFG_PROGRESSREPORTCYC, iRet);
            break;
        }

        iRet = adm_com_getJsonIntVal(jsonRoot, ADM_ECS_CFG_CYCLEPERIOD, &pstGeneralConfig->cyclePeriod);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to get json value, key[%s], err[%d]", 
                        ADM_ECS_CFG_CYCLEPERIOD, iRet);
            break;
        }
    
        iRet = adm_com_getJsonIntVal(jsonRoot, ADM_ECS_CFG_MAXFIRETRIGGER, &pstGeneralConfig->maxFireTrigger);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to get json value, key[%s], err[%d]", 
                        ADM_ECS_CFG_MAXFIRETRIGGER, iRet);
            break;
        }

        //!< 解析业务脚本对象信息
        iRet = __adm_ecs_parseBusinessScriptInfo(jsonRoot, confFileDir, &pstGeneralConfig->stBusinessScript);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to parse BusinessScript, key[%s], err[%d]", 
                        ADM_ECS_CFG_BUSINESSSCRIPT, iRet);
            break;
        }

        //!< 解析业务脚本对象列表
        iRet = __adm_ecs_parseBusinessScriptListInfo(jsonRoot, confFileDir, &pstGeneralConfig->stBusinessScriptList);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to parse businessScriptList, key[%s], err[%d]", 
                        ADM_ECS_CFG_BUSINESSSCRIPTLIST, iRet);
            break;
        }

        //!< 解析资源id列表信息
        iRet = __adm_ecs_parseEcuDidListInfo(jsonRoot, &pstGeneralConfig->stEcuDidList);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to parse ecuDid list, err[%d]", iRet);
            break;
        }

        //!< 解析sa算法库列表信息
        iRet = __adm_ecs_parseSaLibListInfo(jsonRoot, confFileDir, &pstGeneralConfig->stSaLibList);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to parse saLibList, key[%s], err[%d]", 
                        ADM_ECS_CFG_SALIBLIST, iRet);
            break;
        }

        //!< 解析crc算法库列表信息
        iRet = __adm_ecs_parseCrcLibListInfo(jsonRoot, confFileDir, &pstGeneralConfig->stCrcLibList);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to parse saLibList, key[%s], err[%d]", 
                        ADM_ECS_CFG_SALIBLIST, iRet);
            break;
        }
    
        //!< 解析刷写脚本列表信息
        iRet = __adm_ecs_parseFlashScriptListInfo(jsonRoot, confFileDir, &pstGeneralConfig->stflashScriptList);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to parse FlashScriptList, key[%s], err[%d]", 
                        ADM_ECS_CFG_FLASHSCRIPTLIST, iRet);
            break;
        }

        //!< 扩展字段需要使用时在解析
        iRet = adm_com_parseJsonExtsInfo(jsonRoot, &pstGeneralConfig->stExtList);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to parse exts, key[%s], err[%d]", 
                        ADM_COM_AIR_PROTOCOL_FIELD_EXT, iRet);
            break;
        }
    } while (FALSE);

    (void)json_object_put(jsonRoot);
    if (ADM_IS_ERR(iRet))
    {
        adm_ecs_cleanGeneralConfig(pstGeneralConfig);        
    }

    return iRet;
}

D32S adm_ecs_parseSecretConfigFile(DCHAR* pSecretFilePath, DCHAR** ppSecret)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    DCHAR* pFileData = NULL;
    DSIZE uiSecretDecLength = 0;

    if (ADM_IS_NULL(pSecretFilePath) || ADM_IS_NULL(ppSecret))
    {
        log_e("Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }

    do
    {
        ///< 读 secret 文件
        iRet = adm_com_readFileData(pSecretFilePath, &pFileData);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to read secret policy file, err[%d]", iRet);
            break;
        }

        iRet = adm_parseCryptoKey(pFileData, adm_strlen(pFileData), (D8U**)ppSecret, &uiSecretDecLength);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to decrypt secret, err[%d]", iRet);
            break;
        }
    } while (FALSE);

    adm_free(pFileData);

    return iRet;
}

/*!
 *******************************************************************************
 * 解析通用配置文件信息.
 *
 * \note
 * 
 * \param[in]   confFileDir      配置文件目录
 * \param[in]   configFilePath   通用配置文件路径
 * \param[out]  pstGeneralConfig ecu通用配置信息
 * \retval      D32S
 *******************************************************************************
 */
D32S adm_ecs_parseGeneralConfigFile
(
    DCHAR                   *secret,
    DCHAR                   *confFileDir,
    DCHAR                   *configFilePath, 
    ADM_ECS_GENERALCONFIG_T *pstGeneralConfig
)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    DCHAR *pcFileData = NULL;

    if (ADM_IS_NULL(secret) || ADM_IS_NULL(confFileDir) || ADM_IS_NULL(configFilePath) || ADM_IS_NULL(pstGeneralConfig))
    {
        log_e("Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }

    //!< 读 downloadPolicy 文件
    iRet = adm_com_readFileData(configFilePath, &pcFileData);
    if (ADM_IS_ERR(iRet))
    {
        log_e("Failed to read download policy file, err[%d]", iRet);
        return iRet;
    }

    //!< 解析通用配置文件json信息
    iRet = adm_ecs_parseGeneralConfigInfo(secret, confFileDir, pcFileData, pstGeneralConfig);
    if (ADM_IS_ERR(iRet))
    {
        adm_free(pcFileData);
        log_e("Failed to parse GeneralConfigInfo, err[%d]", iRet);
        return iRet;
    }
    adm_free(pcFileData);

    return iRet;
}

D32S adm_ecs_parseConfigPackage
(
    DCHAR                  *confFilePath, 
    DCHAR                  *confPackOutDir,
    DCHAR                  *confPackOutFileName
)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    DBOOL isExist = FALSE;
    DCHAR aDecryptPackPath[ADM_COM_LEN_256] = {0};

    if (ADM_IS_NULL(confFilePath) || ADM_IS_NULL(confPackOutDir) || ADM_IS_NULL(confPackOutFileName))
    {
        log_e("Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }

    do 
    {
        isExist = adm_com_isDirExist(confPackOutDir);
        if (FALSE == isExist)
        {
            iRet = adm_com_createDir(confPackOutDir);
            if (ADM_IS_ERR(iRet))
            {
                log_e("create configDir[%s] error.iRet[%d].",confPackOutDir, iRet);
                break;
            }
        }

        adm_memset(aDecryptPackPath, '\0', sizeof(aDecryptPackPath), sizeof(aDecryptPackPath));
        adm_snprintf(aDecryptPackPath, sizeof(aDecryptPackPath), "%s/%s", confPackOutDir, confPackOutFileName);

        iRet = adm_parseConfigPackage(confFilePath, aDecryptPackPath);
        if (ADM_IS_ERR(iRet))
        {
            log_e("parse config package from [%s] to [%s] error.iRet[%d].",confFilePath, aDecryptPackPath, iRet);
            break;
        }
    } while (FALSE);

    return iRet;
}

static D32S ecsFileSyncTransfercrcLib(ADM_ECS_CRCLIBLIST_T *pstCrcLibList)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    ADM_COM_FILEOBJECT_T *pstCrcLibFile = NULL;

    if (ADM_IS_NULL(pstCrcLibList))
    {
        log_e("Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }

    for (D32S i = 0; i < pstCrcLibList->num; i++)
    {
        pstCrcLibFile = &(pstCrcLibList->pstCrcLibInfo[i].stCrcLibFile);
        iRet = adm_ecs_fileSyncTransfer(pstCrcLibFile->fileName, pstCrcLibFile->filePath, pstCrcLibFile->size, pstCrcLibFile->hash);
        if (ADM_IS_ERR(iRet))
        {
            log_e("file transfer failed,err[%d]", iRet);
            break;
        }
        log_i("transfer file[%s] success", pstCrcLibFile->fileName);
    }

    return iRet;
}

static D32S ecsFileSyncTransferBusinessScript(ADM_ECS_BUSINESSSCRIPTLIST_T *pstBusinessScriptList)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    ADM_COM_FILEOBJECT_T *pstBusinessScriptFile = NULL;

    if (ADM_IS_NULL(pstBusinessScriptList))
    {
        log_e("Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }

    for (D32S i = 0; i < pstBusinessScriptList->num; i++)
    {
        pstBusinessScriptFile = &(pstBusinessScriptList->pstBusinessScriptInfo[i].stBusinessScriptFile);
        iRet = adm_ecs_fileSyncTransfer(pstBusinessScriptFile->fileName,
                                        pstBusinessScriptFile->filePath, pstBusinessScriptFile->size,
                                        pstBusinessScriptFile->hash);
        if (ADM_IS_ERR(iRet))
        {
            log_e("file transfer failed,err[%d]", iRet);
            break;
        }
        log_i("transfer file[%s] success", pstBusinessScriptFile->fileName);
    }

    return iRet;
}

static D32S ecsFileSyncTransferSaLib(ADM_ECS_SALIBLIST_T *pstSaLibList)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    ADM_COM_FILEOBJECT_T *pstSaLibFile = NULL;

    if (ADM_IS_NULL(pstSaLibList))
    {
        log_e("Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }

    for (D32S i = 0; i < pstSaLibList->num; i++)
    {
        pstSaLibFile = &(pstSaLibList->pstSaLibInfo[i].stSaLibFile);
        iRet = adm_ecs_fileSyncTransfer(pstSaLibFile->fileName, pstSaLibFile->filePath,
                                        pstSaLibFile->size, pstSaLibFile->hash);
        if (ADM_IS_ERR(iRet))
        {
            log_e("file transfer failed,err[%d]", iRet);
            break;
        }
        log_i("transfer file[%s] success", pstSaLibFile->fileName);
    }

    return iRet;
}

static D32S ecsFileSyncTransferFlashScript(ADM_ECS_FLASHSCRIPTLIST_T *pstflashScriptList)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    ADM_COM_FILEOBJECT_T *pstflashScriptFile = NULL;

    if (ADM_IS_NULL(pstflashScriptList))
    {
        log_e("Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }

    for (D32S i = 0; i < pstflashScriptList->num; i++)
    {
        pstflashScriptFile = &(pstflashScriptList->pstFlashScriptInfo[i].stFlashScriptFile);
        iRet = adm_ecs_fileSyncTransfer(pstflashScriptFile->fileName, pstflashScriptFile->filePath,
                                        pstflashScriptFile->size, pstflashScriptFile->hash);
        if (ADM_IS_ERR(iRet))
        {
            log_e("file transfer failed,err[%d]", iRet);
            break;
        }
        log_i("transfer file[%s] success", pstflashScriptFile->fileName);
    }

    return iRet;
}

static D32S ecsFileSyncTransfer(ADM_ECS_GENERALCONFIG_T *pstGeneralCfg)
{
    D32S iRet = ADM_ECODE_SUCCESSED;

    if (ADM_IS_NULL(pstGeneralCfg))
    {
        log_e("Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }

    do
    {
        iRet = ecsFileSyncTransfercrcLib(&pstGeneralCfg->stCrcLibList);
        if (ADM_IS_ERR(iRet))
        {
            log_e("ecsFileSyncTransfercrcLib failed,err[%d]", iRet);
            break;
        }

        iRet = ecsFileSyncTransferBusinessScript(&pstGeneralCfg->stBusinessScriptList);
        if (ADM_IS_ERR(iRet))
        {
            log_e("ecsFileSyncTransferBusinessScript failed,err[%d]", iRet);
            break;
        }

        iRet = ecsFileSyncTransferSaLib(&pstGeneralCfg->stSaLibList);
        if (ADM_IS_ERR(iRet))
        {
            log_e("ecsFileSyncTransferSaLib failed,err[%d]", iRet);
            break;
        }

        iRet = ecsFileSyncTransferFlashScript(&pstGeneralCfg->stflashScriptList);
        if (ADM_IS_ERR(iRet))
        {
            log_e("ecsFileSyncTransferFlashScript failed,err[%d]", iRet);
            break;
        }
    } while (FALSE);

    return iRet;
}

/*!
 *******************************************************************************
 * ecs解析压缩包中的配置文件信息.
 *
 * \note
 * 
 * \param[in]   confFilePath     压缩包配置文件路径
 * \param[in]   confFileUnzipDir 压缩包配置文件解压目录
 * \param[out]  pstRsCfgInfo     模块配置信息
 * \retval      D32S
 *******************************************************************************
 */
D32S adm_ecs_parseConfigFile
(
    DCHAR                  *confFilePath, 
    DCHAR                  *confFileUnzipDir, 
    ADM_ECS_RSCONFIGINFO_T *pstRsCfgInfo
)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    DBOOL isExist = FALSE;
    DCHAR genCfgFilePath[ADM_COM_FILEPATH_LEN] = {0};
    DCHAR partCfgFilePath[ADM_COM_FILEPATH_LEN] = {0};
    DCHAR secretFilePath[ADM_COM_FILEPATH_LEN] = {0};
    DCHAR szEventBuf[ADM_COM_LEN_256] = {0};  //mod by wl 20220318

    if (ADM_IS_NULL(confFilePath) || ADM_IS_NULL(confFileUnzipDir) || 
        ADM_IS_NULL(pstRsCfgInfo))
    {
        log_e("Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }

    //mod by wl 20220318
    adm_ecs_eventReport(ADM_COM_MOD_ECS, FALSE, ADM_EVENT_ID_GI_046, ADM_OCS_EVENT_LEVEL_DEBUG, "vehicle parse download config file.", confFilePath);

    isExist = adm_com_isDirExist(confFileUnzipDir);
    if (TRUE == isExist)
    {
        (void)adm_fremove(confFileUnzipDir);
    }

    //mod by wl 20220318
    adm_ecs_eventReport(ADM_COM_MOD_ECS, FALSE, ADM_EVENT_ID_GI_049, ADM_OCS_EVENT_LEVEL_TRACE, "vehicle create download config file unzip dir.", confFileUnzipDir);
    
    log_i("create dir[%s]", confFileUnzipDir);
    iRet = adm_com_createDir(confFileUnzipDir);
    if (ADM_IS_ERR(iRet))
    {
        log_e("Failed to create dir[%s], err[%d]", confFileUnzipDir, iRet);

        //mod by wl 20220318
        (void)adm_memzero(szEventBuf, sizeof(szEventBuf));
        (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), "Failed to create unzip Dir[%s], err[%d]", confFileUnzipDir, iRet);
        adm_ecs_eventReport(ADM_COM_MOD_ECS, TRUE, ADM_EVENT_ID_GI_047, ADM_OCS_EVENT_LEVEL_ERROR, "vehicle parse download config file failed!", szEventBuf);

        return iRet;
    }
    log_i("create dir[%s] ok.", confFileUnzipDir);
    //mod by wl 20220318
    adm_ecs_eventReport(ADM_COM_MOD_ECS, FALSE, ADM_EVENT_ID_GI_051, ADM_OCS_EVENT_LEVEL_DEBUG, "vehicle get download config file unzip dir successed.", NULL);

    //mod by wl 20220318
    adm_ecs_eventReport(ADM_COM_MOD_ECS, FALSE, ADM_EVENT_ID_GI_052, ADM_OCS_EVENT_LEVEL_TRACE, "vehicle unzip download config file.", confFilePath);
    iRet = adm_com_unZipFile(confFilePath, confFileUnzipDir);
    if (ADM_IS_ERR(iRet))
    {
        log_e("Failed to unZipFile[%s], unZipDir[%s], err[%d]", confFilePath, confFileUnzipDir, iRet);

        //mod by wl 20220318
        (void)adm_memzero(szEventBuf, sizeof(szEventBuf));
        (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), "Failed to unZipFile[%s], unZipDir[%s], err[%d]",
                confFilePath, confFileUnzipDir, iRet);
        adm_ecs_eventReport(ADM_COM_MOD_ECS, TRUE, ADM_EVENT_ID_GI_047, ADM_OCS_EVENT_LEVEL_ERROR, "vehicle parse download config file failed!", szEventBuf);

        return iRet;
    }
    //mod by wl 20220318
    adm_ecs_eventReport(ADM_COM_MOD_ECS, FALSE, ADM_EVENT_ID_GI_054, ADM_OCS_EVENT_LEVEL_DEBUG, "vehicle unzip download config file successed.", NULL);


    ///< 校验存储密钥的文件
    (void)adm_snprintf(secretFilePath, sizeof(secretFilePath), "%s%s", confFileUnzipDir, ADM_ECS_SECRET_CFGFILENAME);
    isExist = adm_com_isDirExist(secretFilePath);
    if (FALSE == isExist)
    {
        log_e("Failed to find file[%s]", secretFilePath);

        return ADM_ECODE_OTACFG_FILE_NOEXIST;
    }
    log_d("check secret file ok.");

    //!< 校验通用配置文件是否存在

    (void)adm_snprintf(genCfgFilePath, sizeof(genCfgFilePath), "%s%s", confFileUnzipDir, ADM_ECS_GENERAL_CFGFILENAME);
    //mod by wl 20220318
    adm_ecs_eventReport(ADM_COM_MOD_ECS, FALSE, ADM_EVENT_ID_GI_055, ADM_OCS_EVENT_LEVEL_TRACE, "vehicle check download general config file.", genCfgFilePath);
    isExist = adm_com_isDirExist(genCfgFilePath);
    if (FALSE == isExist)
    {
        log_e("Failed to find file[%s]", genCfgFilePath);
        //mod by wl 20220318
        (void)adm_memzero(szEventBuf, sizeof(szEventBuf));
        (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), "Failed to find file[%s], err[%d]",
                        genCfgFilePath, ADM_ECODE_OTACFG_FILE_NOEXIST);
        adm_ecs_eventReport(ADM_COM_MOD_ECS, TRUE, ADM_EVENT_ID_GI_047, ADM_OCS_EVENT_LEVEL_ERROR, 
                "vehicle parse download config file failed!", szEventBuf);
        return ADM_ECODE_OTACFG_FILE_NOEXIST;
    }
    //mod by wl 20220318
    adm_ecs_eventReport(ADM_COM_MOD_ECS, FALSE, ADM_EVENT_ID_GI_057, ADM_OCS_EVENT_LEVEL_DEBUG, "vehicle check download general config file successed.", NULL);

    //!< 校验零件配置文件是否存在
    (void)adm_snprintf(partCfgFilePath, sizeof(partCfgFilePath), "%s%s", confFileUnzipDir, ADM_ECS_PART_CFGFILENAME);
    //mod by wl 20220318
    adm_ecs_eventReport(ADM_COM_MOD_ECS, FALSE, ADM_EVENT_ID_GI_058, ADM_OCS_EVENT_LEVEL_TRACE, "vehicle check download part config file.", partCfgFilePath);
    isExist = adm_com_isDirExist(partCfgFilePath);
    if (FALSE == isExist)
    {
        log_e("Failed to find file[%s]", partCfgFilePath);

        //mod by wl 20220318
        (void)adm_memzero(szEventBuf, sizeof(szEventBuf));
        (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), "Failed to find file[%s], err[%d]",
                        partCfgFilePath, ADM_ECODE_OTACFG_FILE_NOEXIST);
        adm_ecs_eventReport(ADM_COM_MOD_ECS, TRUE, ADM_EVENT_ID_GI_047, ADM_OCS_EVENT_LEVEL_ERROR, 
                "vehicle parse download config file failed!", szEventBuf);

        return ADM_ECODE_OTACFG_FILE_NOEXIST;
    }
    log_d("check config file ok.");
    //mod by wl 20220318
    adm_ecs_eventReport(ADM_COM_MOD_ECS, FALSE, ADM_EVENT_ID_GI_060, ADM_OCS_EVENT_LEVEL_DEBUG, "vehicle check download part config file successed.", NULL);

    ///< 解析密钥文件
    iRet = adm_ecs_parseSecretConfigFile(secretFilePath, &pstRsCfgInfo->secret);
    if (ADM_IS_ERR(iRet))
    {
        log_e("Failed to secret general config file, err[%d]", iRet);
        return iRet;
    }
    log_d("parse secret config file ok.");

    //!< 解析通用配置文件信息
    //mod by wl 20220318
    adm_ecs_eventReport(ADM_COM_MOD_ECS, FALSE, ADM_EVENT_ID_GI_061, ADM_OCS_EVENT_LEVEL_TRACE, "vehicle parse download general config file.", genCfgFilePath);
    iRet = adm_ecs_parseGeneralConfigFile(pstRsCfgInfo->secret, confFileUnzipDir, genCfgFilePath, &pstRsCfgInfo->stGeneralCfg);
    if (ADM_IS_ERR(iRet))
    {
        log_e("Failed to parse general config file, err[%d]", iRet);
        //mod by wl 20220318
        (void)adm_memzero(szEventBuf, sizeof(szEventBuf));
        (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), "Failed to parse general config file, err[%d]", iRet);
        adm_ecs_eventReport(ADM_COM_MOD_ECS, TRUE, ADM_EVENT_ID_GI_047, ADM_OCS_EVENT_LEVEL_ERROR, 
                "vehicle parse download config file failed!", szEventBuf);
        return iRet;
    }
    log_d("parse general config file ok.");

    /**文件传输*/
    iRet = ecsFileSyncTransfer(&pstRsCfgInfo->stGeneralCfg);
    if (ADM_IS_ERR(iRet))
    {
        log_e("Failed to transfer file, err[%d]", iRet);
        (void)adm_memzero(szEventBuf, sizeof(szEventBuf));
        (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), "Failed to parse general config file, err[%d]", iRet);
        adm_ecs_eventReport(ADM_COM_MOD_ECS, TRUE, ADM_EVENT_ID_GI_047, ADM_OCS_EVENT_LEVEL_ERROR,
                            "vehicle parse download config file failed!", szEventBuf);
        return iRet;
    }
    log_d("transfer config file ok.");

    adm_ecs_eventReport(ADM_COM_MOD_ECS, FALSE, ADM_EVENT_ID_GI_063, ADM_OCS_EVENT_LEVEL_DEBUG, "vehicle parse download general config file successed.", NULL);

    //!< 解析零件配置文件信息

    //mod by wl 20220318
    adm_ecs_eventReport(ADM_COM_MOD_ECS, FALSE, ADM_EVENT_ID_GI_064, ADM_OCS_EVENT_LEVEL_TRACE, "vehicle parse download part config file.", partCfgFilePath);
    iRet = adm_ecs_parsePartConfigFile(pstRsCfgInfo->secret, partCfgFilePath, &pstRsCfgInfo->stPartCfg);
    if (ADM_IS_ERR(iRet))
    {
        log_e("Failed to parse ecus config file, err[%d]", iRet);
        //mod by wl 20220318
        (void)adm_memzero(szEventBuf, sizeof(szEventBuf));
        (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), "Failed to parse ecus config file, err[%d]", iRet);
        adm_ecs_eventReport(ADM_COM_MOD_ECS, TRUE, ADM_EVENT_ID_GI_047, ADM_OCS_EVENT_LEVEL_ERROR, 
                "vehicle parse download config file failed!", szEventBuf);
        return iRet;
    }
    log_d("parse part config file ok.");

    //mod by wl 20220318
    adm_ecs_eventReport(ADM_COM_MOD_ECS, FALSE, ADM_EVENT_ID_GI_066, ADM_OCS_EVENT_LEVEL_DEBUG, "vehicle parse download part config file successed.", NULL);
    adm_ecs_eventReport(ADM_COM_MOD_ECS, FALSE, ADM_EVENT_ID_GI_048, ADM_OCS_EVENT_LEVEL_DEBUG, "vehicle parse download config file successed.", confFileUnzipDir);

    return iRet;
}
