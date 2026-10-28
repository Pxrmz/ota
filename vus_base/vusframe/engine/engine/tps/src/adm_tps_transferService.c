/**
 * @file    adm_tps_transferService.c
 * @brief   Transfer Package Service  
 * @details 升级包传输服务
 * @author  wangyanpeng
 * @version v0.1.0
 * @date    2020-11-06
 * @copyright www.abupdate.com
 */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "adm_string.h"
#include "adm_memory.h"
#include "adm_tps_log.h"
#include "adm_tps_kvStorage.h"
#include "adm_tps_transferService.h"
#include "adm_ecs_rsConfig.h"
#include "adm_com_errorCode.h"
#include "adm_pis_package.h"


static ADM_TPS_TRANSFERINFO_T  gstTPSTransferInfo;  //!< TPS模块信息


#if 0
#endif


/**
 * @brief   模块封装的事件上报接口 
 * @detail  事件上报
 *
 * @note 
 * @param [in]  eModuleID 模块ID
 * @param [in]  isReportImmediate 是否需要立即上报
 * @param [in]  pcEventId      事件id
 * @param [in]  i32EventLevel  事件级别：1:表示INFO级别 2:表示WARN级别 3:表示ERROR级别
 * @param [in]  pcEventDesc    事件描述
 * @param [in]  pcEventContext 事件上下文(非必填字段)
 * @return      none 
 */
void adm_tps_eventReport
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
    if (FALSE == gstTPSTransferInfo.isInited)
    {
        log_e("TPS module not init, err[%d]", ADM_ECODE_MODULE_UNINIT);
        return;
    }

    if (NULL != gstTPSTransferInfo.fnReportEventApi)
    {
        gstTPSTransferInfo.fnReportEventApi(eModuleID, isReportImmediate, pcEventId, i32EventLevel, pcEventDesc, pcEventContext);
    }

    return;
}

#if 0
#endif

/*!
 *******************************************************************************
 * 生成传包 ecu 信息
 * 
 * \note   
 *
 * \param   none
 * \retval  ADM_TPS_TRANSFERPKG_T*
 *******************************************************************************
 */
ADM_TPS_TRANSFERPKG_T* adm_tps_newTransferEcuPackage(void)
{
    ADM_TPS_TRANSFERPKG_T *pstTransferPkg = NULL;

    pstTransferPkg = (ADM_TPS_TRANSFERPKG_T *)adm_malloc(sizeof(*pstTransferPkg));
    if (NULL != pstTransferPkg)
    {
        (void)adm_memzero(pstTransferPkg, sizeof(*pstTransferPkg));
    }

    return pstTransferPkg;
}

/*!
 *******************************************************************************
 * 清空传包 ecu 信息
 * 
 * \note   
 *
 * \param[in]   pstTransferPkg  传包ecu信息
 * \retval      none
 *******************************************************************************
 */
void adm_tps_cleanTransferEcuPackage
(
    ADM_TPS_TRANSFERPKG_T *pstTransferPkg
)
{
    if (ADM_IS_NULL(pstTransferPkg))
    {
        return;
    }

    adm_free(pstTransferPkg->ecuReqid);
    adm_free(pstTransferPkg->ecuSwDid);
    adm_free(pstTransferPkg->downloadInstanceId);
    adm_free(pstTransferPkg->transferInstanceId);
    adm_free(pstTransferPkg->installInstanceId);

    adm_com_delFileObject(&pstTransferPkg->pstSrcPkgInfo);
    adm_com_cleanFileObject(&pstTransferPkg->stDstPkgInfo);

    adm_free(pstTransferPkg->stTransProcess.stResult.failReason);
    (void)adm_memzero(&pstTransferPkg->stTransProcess, sizeof(pstTransferPkg->stTransProcess));

    return;
}

/*!
 *******************************************************************************
 * 释放传包 ecu 信息
 * 
 * \note   
 *
 * \param[in]   pstTransferPkg  传包ecu信息
 * \retval      none
 *******************************************************************************
 */
void adm_tps_delTransferEcuPackage
(
    ADM_TPS_TRANSFERPKG_T **pstTransferPkg
)
{
    if (ADM_IS_NULL(pstTransferPkg) || ADM_IS_NULL(*pstTransferPkg))
    {
        return;
    }

    adm_tps_cleanTransferEcuPackage(*pstTransferPkg);
    adm_free(*pstTransferPkg);

    return;
}

/*!
 *******************************************************************************
 * 生成传包 ecu list 信息
 * 
 * \note   
 *
 * \param[in]   num  ecu个数
 * \retval      ADM_TPS_TRANSFERPKG_T*
 *******************************************************************************
 */
ADM_TPS_TRANSFERPKG_T* adm_tps_newTransferEcuPackageList(D32U num)
{
    ADM_TPS_TRANSFERPKG_T *pstTransferPkg = NULL;

    if ((D32U)0 >= num)
    {
        log_e("Invalid input param");
        return NULL;
    }

    pstTransferPkg = (ADM_TPS_TRANSFERPKG_T *)adm_malloc(num * sizeof(*pstTransferPkg));
    if (NULL != pstTransferPkg)
    {
        (void)adm_memzero(pstTransferPkg, num * sizeof(*pstTransferPkg));
    }

    return pstTransferPkg;
}

/*!
 *******************************************************************************
 * 清空传包 ecu list 信息
 * 
 * \note   不释放list内存
 *
 * \param[in]   transferPkgNum  part包数量
 * \param[in]   pstTransferPkg  part包列表信息
 * \retval      none
 *******************************************************************************
 */
void adm_tps_cleanTransEcuPackageList
(
    D32U                   transferPkgNum,
    ADM_TPS_TRANSFERPKG_T *pstTransferPkg
)
{
    D32S i = 0;

    if (ADM_IS_NULL(pstTransferPkg) || ((D32U)0 >= transferPkgNum))
    {
        return;
    }

    //!< 清空 List 信息
    for (i = 0; (D32U)i < transferPkgNum; i++)
    {
        adm_tps_cleanTransferEcuPackage(&pstTransferPkg[i]);
    }

    return;
}

/*!
 *******************************************************************************
 * 释放传包 ecu list 信息
 * 
 * \note   
 *
 * \param[in]   transferPkgNum  待传包个数
 * \param[in]   pstTransferPkg  待传包列表信息
 * \retval      none
 *******************************************************************************
 */
void adm_tps_delTransferEcuPackageList
(
    D32U                   transferPkgNum,
    ADM_TPS_TRANSFERPKG_T  **pstTransferPkg
)
{
    if (ADM_IS_NULL(pstTransferPkg) || ADM_IS_NULL(*pstTransferPkg) || ((D32U)0 >= transferPkgNum))
    {
        return;
    }

    adm_tps_cleanTransEcuPackageList(transferPkgNum, *pstTransferPkg);
    adm_free(*pstTransferPkg);

    return;
}


/*!
 *******************************************************************************
 * 生成升级包分包进度信息列表结构
 * 
 * \note   
 *
 * \param[in]  num  分包进度列表个数
 * \retval     ADM_TPS_TRANSPKGPROGRESSINFO_T*
 *******************************************************************************
 */
ADM_TPS_TRANSPKGPROGRESSINFO_T* adm_tps_newTransPkgProgressInfoList(D32U num)
{
    ADM_TPS_TRANSPKGPROGRESSINFO_T *pstPkgProgressList = NULL;

    pstPkgProgressList = (ADM_TPS_TRANSPKGPROGRESSINFO_T *)adm_malloc(num * sizeof(*pstPkgProgressList));
    if (NULL != pstPkgProgressList)
    {
        (void)adm_memzero(pstPkgProgressList, num * sizeof(*pstPkgProgressList));
    }

    return pstPkgProgressList;
}

/*!
 *******************************************************************************
 * 释放升级包分包进度信息列表结构
 * 
 * \note   
 *
 * \param[in]   pkgProgressListNum  升级包分包进度列表长度
 * \param[in]   pstPkgProgressList  升级包分包进度列表信息
 * \retval      none
 *******************************************************************************
 */
void adm_tps_cleanTransPackageProgressInfoList
(
    D32S                            pkgProgressListNum,  
    ADM_TPS_TRANSPKGPROGRESSINFO_T *pstPkgProgressList
)
{
    D32S i = 0;

    if (ADM_IS_NULL(pstPkgProgressList) || (0 >= pkgProgressListNum))
    {
        return;
    }

    for (i = 0; i < pkgProgressListNum; i++)
    {
        adm_free(pstPkgProgressList[i].ecuReqid);
        adm_free(pstPkgProgressList[i].ecuSwDid);
    }

    return;
}

/*!
 *******************************************************************************
 * 释放升级包分包进度信息列表结构
 * 
 * \note   
 *
 * \param[in]   pkgProgressListNum  升级包分包进度列表长度
 * \param[in]   pstPkgProgressList  升级包分包进度列表信息
 * \retval      none
 *******************************************************************************
 */
void adm_tps_delTransPkgProgressInfoList
(
    D32S                            pkgProgressListNum,  
    ADM_TPS_TRANSPKGPROGRESSINFO_T **pstPkgProgressList
)
{
    if (ADM_IS_NULL(pstPkgProgressList) || ADM_IS_NULL(*pstPkgProgressList) || (0 >= pkgProgressListNum))
    {
        return;
    }

    adm_tps_cleanTransPackageProgressInfoList(pkgProgressListNum, *pstPkgProgressList);
    adm_free(*pstPkgProgressList);

    return;
}

/*!
 *******************************************************************************
 * 生成升级包分包结果信息列表结构
 * 
 * \note   
 *
 * \param[in]  num  分包结果列表个数
 * \retval     ADM_TPS_TRANSPKGRESULTINFO_T*
 *******************************************************************************
 */
ADM_TPS_TRANSPKGRESULTINFO_T* adm_tps_newTransPackageResultInfoList(D32U num)
{
    ADM_TPS_TRANSPKGRESULTINFO_T *pstPkgResultList = NULL;

    pstPkgResultList = (ADM_TPS_TRANSPKGRESULTINFO_T *)adm_malloc(num * sizeof(*pstPkgResultList));
    if (NULL != pstPkgResultList)
    {
        (void)adm_memzero(pstPkgResultList, num * sizeof(*pstPkgResultList));
    }

    return pstPkgResultList;
}

/*!
 *******************************************************************************
 * 释放升级包分包结果信息列表结构
 * 
 * \note   
 *
 * \param[in]   pkgResultListNum    升级包分包结果列表长度
 * \param[in]   pstPkgProgressList  升级包分包结果列表信息
 * \retval      none
 *******************************************************************************
 */
void adm_tps_cleanTransPackageResultInfoList
(
    D32S                          pkgResultListNum,  
    ADM_TPS_TRANSPKGRESULTINFO_T *pstPkgResultList
)
{
    D32S i = 0;

    if (ADM_IS_NULL(pstPkgResultList) || (0 >= pkgResultListNum))
    {
        return;
    }

    for (i = 0; i < pkgResultListNum; i++)
    {
        adm_free(pstPkgResultList[i].ecuReqid);
        adm_free(pstPkgResultList[i].ecuSwDid);
        adm_free(pstPkgResultList[i].stResult.failReason);
    }

    return;
}

/*!
 *******************************************************************************
 * 释放升级包分包结果信息列表结构
 * 
 * \note   
 *
 * \param[in]   pkgResultListNum    升级包分包结果列表长度
 * \param[in]   pstPkgProgressList  升级包分包结果列表信息
 * \retval      none
 *******************************************************************************
 */
void adm_tps_delTransPackageResultInfoList
(
    D32S                          pkgResultListNum,  
    ADM_TPS_TRANSPKGRESULTINFO_T **pstPkgResultList
)
{
    if (ADM_IS_NULL(pstPkgResultList) || ADM_IS_NULL(*pstPkgResultList) || (0 >= pkgResultListNum))
    {
        return;
    }

    adm_tps_cleanTransPackageResultInfoList(pkgResultListNum, *pstPkgResultList);
    adm_free(*pstPkgResultList);

    return;
}


/*!
 *******************************************************************************
 * 生成升级包分包进度信息结构
 * 
 * \note   
 *
 * \param  none
 * \retval ADM_TPS_TRANSPKGPROGRESS_T*
 *******************************************************************************
 */
ADM_TPS_TRANSPKGPROGRESS_T* adm_tps_newTransPackageProgress(void)
{
    ADM_TPS_TRANSPKGPROGRESS_T *pstTransPkgProgress = NULL;

    pstTransPkgProgress = (ADM_TPS_TRANSPKGPROGRESS_T *)adm_malloc(sizeof(*pstTransPkgProgress));
    if (NULL != pstTransPkgProgress)
    {
        (void)adm_memzero(pstTransPkgProgress, sizeof(*pstTransPkgProgress));
    }

    return pstTransPkgProgress;
}

/*!
 *******************************************************************************
 * 释放升级包分包进度信息结构
 * 
 * \note   
 *
 * \param[in]   pstDownPkgProgress  分包进度信息
 * \retval      none
 *******************************************************************************
 */
void adm_tps_delTransPackageProgress(ADM_TPS_TRANSPKGPROGRESS_T **pstTransPkgProgress)
{
    D32S i = 0;

    if (ADM_IS_NULL(pstTransPkgProgress) || ADM_IS_NULL(*pstTransPkgProgress))
    {
        return;
    }

    for (i = 0; (D32U)i < (*pstTransPkgProgress)->num; i++)
    {
        adm_free((*pstTransPkgProgress)->pstProgressList[i].ecuReqid);
        adm_free((*pstTransPkgProgress)->pstProgressList[i].ecuSwDid);
    }
    adm_free((*pstTransPkgProgress)->pstProgressList);
    adm_free(*pstTransPkgProgress);

    return;
}

/*!
 *******************************************************************************
 * 生成升级包分包结果信息结构
 * 
 * \note   
 *
 * \param  none
 * \retval ADM_TPS_TRANSPKGRESULT_T*
 *******************************************************************************
 */
ADM_TPS_TRANSPKGRESULT_T* adm_tps_newTransPackageResult(void)
{
    ADM_TPS_TRANSPKGRESULT_T *pstTransPkgResult = NULL;

    pstTransPkgResult = (ADM_TPS_TRANSPKGRESULT_T *)adm_malloc(sizeof(*pstTransPkgResult));
    if (NULL != pstTransPkgResult)
    {
        (void)adm_memzero(pstTransPkgResult, sizeof(*pstTransPkgResult));
    }

    return pstTransPkgResult;
}

/*!
 *******************************************************************************
 * 释放升级包分包结果信息结构
 * 
 * \note   
 *
 * \param[in]   pstTransPkgResult  分包结果信息
 * \retval      none
 *******************************************************************************
 */
void adm_tps_delTransPackageResult(ADM_TPS_TRANSPKGRESULT_T **pstTransPkgResult)
{
    D32S i = 0;

    if (ADM_IS_NULL(pstTransPkgResult) || ADM_IS_NULL(*pstTransPkgResult))
    {
        return;
    }

    for (i = 0; (D32U)i < (*pstTransPkgResult)->num; i++)
    {
        adm_free((*pstTransPkgResult)->pstResultList[i].ecuReqid);
        adm_free((*pstTransPkgResult)->pstResultList[i].ecuSwDid);
        adm_free((*pstTransPkgResult)->pstResultList[i].stResult.failReason);
    }
    adm_free((*pstTransPkgResult)->pstResultList);
    adm_free(*pstTransPkgResult);

    return;
}


/*!
 *******************************************************************************
 * 清空传输模块缓存信息
 * 
 * \note   不销毁缓存读写锁
 *
 * \param[in]   pstTPSTransferInfo  传输缓存信息
 * \retval      none
 *******************************************************************************
 */
void adm_tps_cleanTransferCacheInfo(ADM_TPS_TRANSFERINFO_T *pstTPSTransferInfo)
{
    if (ADM_IS_NULL(pstTPSTransferInfo) || (FALSE == pstTPSTransferInfo->isInited))
    {
        return;
    }

    adm_dps_cleanDownTaskInfo(&pstTPSTransferInfo->stDownTaskInfo);
    adm_dps_cleanTransferCondition(&pstTPSTransferInfo->stTransferCondition);
    adm_tps_cleanTransEcuPackageList(pstTPSTransferInfo->stTransferECUList.num, 
                pstTPSTransferInfo->stTransferECUList.pstEcuPkg);
    pstTPSTransferInfo->stTransferECUList.num = 0;
    pstTPSTransferInfo->stTransferECUList.pstEcuPkg = NULL;

    return;
}


#if 0
#endif

/*!
 *******************************************************************************
 * 填充待下载的ecu.
 *
 * \note 
 * 
 * \param[in]  pstEcuPkgInfo      ecu升级包信息
 * \param[in]  pstEcuCfgInfo      ecu平台配置信息
 * \param[out] pstEcuTransPkgInfo 待传输ecu信息
 * \retval     D32S
 *******************************************************************************
 */
static D32S adm_tps_fillWaitTransEcuBaseInfo
(
    ADM_DPS_DOWN_ECUPKG_T      *pstEcuPkgInfo,
    ADM_ECS_ECUCONFIG_T        *pstEcuCfgInfo,
    ADM_TPS_TRANSFERPKG_T      *pstEcuTransPkgInfo
)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    
    if (ADM_IS_NULL(pstEcuPkgInfo) || ADM_IS_NULL(pstEcuCfgInfo) || ADM_IS_NULL(pstEcuTransPkgInfo))
    {
        log_e("Invalid input param");
        return iRet;
    }

    //!< copy src package
    if (NULL != pstEcuPkgInfo->pstSrcPkgFileObj)
    {
        pstEcuTransPkgInfo->pstSrcPkgInfo = adm_com_newFileObject();
        if (ADM_IS_NULL(pstEcuTransPkgInfo->pstSrcPkgInfo))
        {
            log_e("Failed to new src package file obj, err[%d]", ADM_ECODE_NO_MEMORY);
            return ADM_ECODE_NO_MEMORY;
        }
    
        iRet = adm_com_copyFileObject(pstEcuTransPkgInfo->pstSrcPkgInfo, pstEcuPkgInfo->pstSrcPkgFileObj);
        if (ADM_IS_ERR(iRet))
        {
            adm_com_delFileObject(&pstEcuTransPkgInfo->pstSrcPkgInfo);
            pstEcuTransPkgInfo->pstSrcPkgInfo = NULL;
            log_e("Failed to copy src package file object, err[%d]", iRet);
            return iRet;
        }
    }
    
    //!< copy dst package 
    iRet = adm_com_copyFileObject(&pstEcuTransPkgInfo->stDstPkgInfo, &pstEcuPkgInfo->stDstPkgFileObj);
    if (ADM_IS_ERR(iRet))
    {
        adm_com_delFileObject(&pstEcuTransPkgInfo->pstSrcPkgInfo);
        pstEcuTransPkgInfo->pstSrcPkgInfo = NULL;
        log_e("Failed to copy file object, err[%d]", iRet);
        return iRet;
    }
    
    pstEcuTransPkgInfo->ecuReqid = adm_strdup(pstEcuCfgInfo->ecuReqId);
    pstEcuTransPkgInfo->ecuSwDid = adm_strdup(pstEcuCfgInfo->ecuSwDid);
    pstEcuTransPkgInfo->downloadInstanceId = adm_strdup(pstEcuCfgInfo->downloadInstanceId);
    pstEcuTransPkgInfo->transferInstanceId = adm_strdup(pstEcuCfgInfo->transferInstanceId);
    pstEcuTransPkgInfo->installInstanceId  = adm_strdup(pstEcuCfgInfo->installInstanceId);

    return ADM_ECODE_SUCCESSED;
}


/*!
 *******************************************************************************
 * 过滤待下载的ecu.
 *
 * \note 从检测到的待下载ecu中过滤需要预分包的ecu 
 * 
 * \param[in]  pstEcuPkgList      ecu升级包信息列表
 * \param[in]  pstEcuCfgList      ecu平台配置信息列表
 * \param[out] pstTransferECUList 待传输ecu列表
 * \retval     D32S
 *******************************************************************************
 */
static D32S __adm_tps_filterWaitTransferEcu
(
    ADM_DPS_DOWN_ECUPKGLIST_T  *pstEcuPkgList,
    ADM_ECS_ECUCFGLIST_T       *pstEcuCfgList,
    ADM_TPS_TRANSFER_ECULIST_T *pstTransferECUList
)
{
    D32S i = 0;
    D32S j = 0;
    D32S waitTransEcuNum = 0;
    D32S iRet = ADM_ECODE_SUCCESSED;
    
    if (ADM_IS_NULL(pstEcuCfgList) || ADM_IS_NULL(pstEcuCfgList) || ADM_IS_NULL(pstTransferECUList))
    {
        log_e("Invalid input param");
        return iRet;
    }

    if (ADM_IS_NULL(gstTPSTransferInfo.fnCmpIsNeedTransApi))
    {
        log_e("TPS module not init, err[%d]", ADM_ECODE_MODULE_UNINIT);
        return 0;
    }

    //!< calc wait transfer ecu sum 
    for (i = 0; i < pstEcuPkgList->num; i++)
    {
        //!< loop ecu cfg info 
        for (j = 0; j < pstEcuCfgList->num; j++)
        {
            //!< search ecu
            if ((0 == adm_strcasecmp(pstEcuPkgList->pstEcuPkgInfo[i].ecuReqid, pstEcuCfgList->pstEcuCfg[j].ecuReqId)) &&
                (0 == adm_strcasecmp(pstEcuPkgList->pstEcuPkgInfo[i].ecuSwDid, pstEcuCfgList->pstEcuCfg[j].ecuSwDid)))
            {
                //!< calc ecu is need trans 
                if (gstTPSTransferInfo.fnCmpIsNeedTransApi(pstEcuCfgList->pstEcuCfg[j].downloadInstanceId, 
                        pstEcuCfgList->pstEcuCfg[j].transferInstanceId))
                {
                    ++waitTransEcuNum;
                }
            
                break;
            }
        }

        //!< ecu not in ecu cfg, return err 
        if (j == pstEcuCfgList->num)
        {
            iRet = ADM_ECODE_DOWN_RESOURCE_NOFIND;
            log_e("Failed to find ecu info from ecu cfg, ecuReqid[%s], ecuSwDid[%s], err[%d]", 
                pstEcuPkgList->pstEcuPkgInfo[i].ecuReqid, pstEcuPkgList->pstEcuPkgInfo[i].ecuSwDid, iRet);
            break;
        }
    }

    //!< calc failed 
    if (ADM_IS_ERR(iRet))
    {
        return iRet;
    }

    //!< new wait transfer ecu memory 
    pstTransferECUList->num = (D32U)waitTransEcuNum;
    if ((D32U)0 >= pstTransferECUList->num)
    {
        pstTransferECUList->num = (D32U)0;
        pstTransferECUList->pstEcuPkg = NULL;
        return ADM_ECODE_SUCCESSED;
    }

    pstTransferECUList->pstEcuPkg = adm_tps_newTransferEcuPackageList(pstTransferECUList->num);
    if (ADM_IS_NULL(pstTransferECUList->pstEcuPkg))
    {
        pstTransferECUList->num = (D32U)0;
        log_e("Failed to new transfer ecu list info, err[%d]", ADM_ECODE_NO_MEMORY);
        return ADM_ECODE_NO_MEMORY;
    }

    waitTransEcuNum = 0;
    //!< fill wait transfer ecu base info
    for (i = 0; i < pstEcuPkgList->num; i++)
    {
        for (j = 0; j < pstEcuCfgList->num; j++)
        {
            //!< search ecu
            if ((0 == adm_strcasecmp(pstEcuPkgList->pstEcuPkgInfo[i].ecuReqid, pstEcuCfgList->pstEcuCfg[j].ecuReqId)) &&
                (0 == adm_strcasecmp(pstEcuPkgList->pstEcuPkgInfo[i].ecuSwDid, pstEcuCfgList->pstEcuCfg[j].ecuSwDid)))
            {
                //!< calc ecu is need trans 
                if (gstTPSTransferInfo.fnCmpIsNeedTransApi(pstEcuCfgList->pstEcuCfg[j].downloadInstanceId, 
                        pstEcuCfgList->pstEcuCfg[j].transferInstanceId))
                {
                    iRet = adm_tps_fillWaitTransEcuBaseInfo(&pstEcuPkgList->pstEcuPkgInfo[i], 
                                &pstEcuCfgList->pstEcuCfg[j], 
                                &pstTransferECUList->pstEcuPkg[waitTransEcuNum]);
                    if (ADM_IS_ERR(iRet))
                    {
                        log_e("Failed to fill wait trans ecu base info, err[%d]", iRet);
                        break;
                    }

                    ++waitTransEcuNum;
                }
            
                break;
            }
        }

        if (ADM_IS_ERR(iRet))
        {
            adm_tps_delTransferEcuPackageList(pstTransferECUList->num, &pstTransferECUList->pstEcuPkg);
            pstTransferECUList->num = (D32S)0;
            pstTransferECUList->pstEcuPkg = NULL;
            break;
        }
    }

    return iRet;
}



#if 0
#endif

/*!
 *******************************************************************************
 * 填充待传输ecu升级包进度初始化.
 *
 * \note
 * 
 * \param[in/out]  pstTransferECUList  待传输ecu列表
 * \retval         D32S
 *******************************************************************************
 */
static D32S __adm_tps_transferEcuPackageProcessInit
(
    ADM_TPS_TRANSFER_ECULIST_T *pstTransferECUList
)
{

    D32S i = 0;
    ADM_TPS_TRANSFERPKG_T *pstEcuPkg = NULL;

    if (ADM_IS_NULL(pstTransferECUList))
    {
        log_e("Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }

    for (i = 0; (D32U)i < pstTransferECUList->num; i++)
    {
        pstEcuPkg = &pstTransferECUList->pstEcuPkg[i];

        pstEcuPkg->stTransProcess.stResult.result = (D32S)ADM_TPS_TRANSPKG_STATE_NONE;
        pstEcuPkg->stTransProcess.stProgress.totalBytes = pstEcuPkg->stDstPkgInfo.size;
        
        if (NULL != pstEcuPkg->pstSrcPkgInfo)
        {
            pstEcuPkg->stTransProcess.stProgress.totalBytes += pstEcuPkg->pstSrcPkgInfo->size;
        }
    }

    return ADM_ECODE_SUCCESSED;
}


/*!
 *******************************************************************************
 * 填充分包ecu分包进度/结果信息.
 *
 * \note 
 *
 * \param[out]  pstDstTransProcess   目的分包进度/结果信息
 * \param[in]   pstSrcTransProcess   源分包进度/结果信息
 * \retval      D32S                 0: 成功 non-0：失败
 *******************************************************************************
 */
static D32S __adm_tps_fillTransProcess
(
    ADM_TPS_TRANSPROCESS_T *pstDstTransProcess,
    ADM_TPS_TRANSPROCESS_T *pstSrcTransProcess
)
{
    
    if (ADM_IS_NULL(pstDstTransProcess) || ADM_IS_NULL(pstSrcTransProcess))
    {
        log_e("Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }

    pstDstTransProcess->stResult.failReason = 
            adm_strdup(pstSrcTransProcess->stResult.failReason);
    pstDstTransProcess->stResult.result      = pstSrcTransProcess->stResult.result;
    pstDstTransProcess->stResult.eTransState = pstSrcTransProcess->stResult.eTransState;

    pstDstTransProcess->stProgress.speed      = pstSrcTransProcess->stProgress.speed;
    pstDstTransProcess->stProgress.totalTime  = pstSrcTransProcess->stProgress.totalTime;
    pstDstTransProcess->stProgress.remainTime = pstSrcTransProcess->stProgress.remainTime;
    pstDstTransProcess->stProgress.saveBytes  = pstSrcTransProcess->stProgress.saveBytes;
    pstDstTransProcess->stProgress.totalBytes = pstSrcTransProcess->stProgress.totalBytes;

    return ADM_ECODE_SUCCESSED;
}

/*!
 *******************************************************************************
 * 填充待分包ecu.
 *
 * \note 
 *
 * \param[out]  pstDstTransPkg   目的分包信息
 * \param[in]   pstSrcTransPkg   源分包信息
 * \retval      D32S             0: 成功 non-0：失败
 *******************************************************************************
 */
static D32S __adm_tps_fillEcuTransPackage
(
    ADM_TPS_TRANSFERPKG_T *pstDstTransPkg,
    ADM_TPS_TRANSFERPKG_T *pstSrcTransPkg
)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    
    if (ADM_IS_NULL(pstDstTransPkg) || ADM_IS_NULL(pstSrcTransPkg))
    {
        log_e("Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }

    do
    {
        //!< src升级包信息 --- 申请相应内存空间
        if (NULL != pstSrcTransPkg->pstSrcPkgInfo)
        {
            pstDstTransPkg->pstSrcPkgInfo = adm_com_newFileObject();
            if (ADM_IS_NULL(pstDstTransPkg->pstSrcPkgInfo))
            {
                iRet = ADM_ECODE_NO_MEMORY;
                log_e("Falied to malloc");
                break;
            }

            //!< 填充src包信息
            iRet = adm_com_copyFileObject(pstDstTransPkg->pstSrcPkgInfo, 
                            pstSrcTransPkg->pstSrcPkgInfo);
            if (ADM_IS_ERR(iRet))
            {
                log_e("Failed to fill ecu wait transfer src package info, err[%d]", iRet);
                break;
            }
        }

        //!< 填充dst包信息
        iRet = adm_com_copyFileObject(&pstDstTransPkg->stDstPkgInfo, 
                        &pstSrcTransPkg->stDstPkgInfo);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to fill ecu wait transfer dst package info, err[%d]", iRet);
            break;
        }

        pstDstTransPkg->ecuReqid = adm_strdup(pstSrcTransPkg->ecuReqid);
        pstDstTransPkg->ecuSwDid = adm_strdup(pstSrcTransPkg->ecuSwDid);
        pstDstTransPkg->downloadInstanceId = adm_strdup(pstSrcTransPkg->downloadInstanceId);
        pstDstTransPkg->transferInstanceId = adm_strdup(pstSrcTransPkg->transferInstanceId);
        pstDstTransPkg->installInstanceId  = adm_strdup(pstSrcTransPkg->installInstanceId);

        (void)__adm_tps_fillTransProcess(&pstDstTransPkg->stTransProcess, 
                    &pstSrcTransPkg->stTransProcess);    
    } while (FALSE);

    if (ADM_IS_ERR(iRet))
    {
        adm_tps_cleanTransferEcuPackage(pstDstTransPkg);
    }

    return iRet;
}


/*!
 *******************************************************************************
 * 填充升级包分包结果信息.
 *
 * \note
 *
 * \param[inout]   pstTransPkgResult  升级包分包结果信息
 * \param[in]      pstTransferECUList ecu分包信息
 * \retval         D32S    0: 成功 non-0：失败
 *******************************************************************************
 */
static D32S __adm_tps_fillTransPackageResult
(
    ADM_TPS_TRANSPKGRESULT_T   *pstTransPkgResult,
    ADM_TPS_TRANSFER_ECULIST_T *pstTransferECUList
)
{
    D32S i = 0;
    D32S iRet = ADM_ECODE_SUCCESSED;

    if (ADM_IS_NULL(pstTransPkgResult) || ADM_IS_NULL(pstTransferECUList))
    {
        log_e( "Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }

    pstTransPkgResult->num = pstTransferECUList->num;
    if ((D32U)0 >= pstTransPkgResult->num)
    {
        pstTransPkgResult->num = 0;
        log_d("Transfer ecu list is zero.");
        return ADM_ECODE_SUCCESSED;
    }

    pstTransPkgResult->pstResultList = adm_tps_newTransPackageResultInfoList(pstTransPkgResult->num);
    if (ADM_IS_NULL(pstTransPkgResult->pstResultList))
    {
        log_e("Failed to new transfer package result info list, err[%d]", ADM_ECODE_NO_MEMORY);
        return ADM_ECODE_NO_MEMORY;
    }
  
    for (i = 0; (D32U)i < pstTransferECUList->num; i++)
    {
        pstTransPkgResult->pstResultList[i].ecuReqid = 
            adm_strdup(pstTransferECUList->pstEcuPkg[i].ecuReqid);
        pstTransPkgResult->pstResultList[i].ecuSwDid = 
            adm_strdup(pstTransferECUList->pstEcuPkg[i].ecuSwDid);
          
        pstTransPkgResult->pstResultList[i].stResult.result = 
            pstTransferECUList->pstEcuPkg[i].stTransProcess.stResult.result;
        pstTransPkgResult->pstResultList[i].stResult.eTransState = 
            pstTransferECUList->pstEcuPkg[i].stTransProcess.stResult.eTransState;               
        pstTransPkgResult->pstResultList[i].stResult.failReason = 
            adm_strdup(pstTransferECUList->pstEcuPkg[i].stTransProcess.stResult.failReason);
    }

    return iRet;
}


/*!
 *******************************************************************************
 * 填充升级包分包结果信息.
 *
 * \note
 *
 * \param[inout]   pstTransPkgProgress  升级包分包结果信息
 * \param[in]      pstTransferECUList   ecu分包信息
 * \retval         D32S    0: 成功 non-0：失败
 *******************************************************************************
 */
static D32S __adm_tps_fillTransPackageProgress
(
    ADM_TPS_TRANSPKGPROGRESS_T *pstTransPkgProgress,
    ADM_TPS_TRANSFER_ECULIST_T *pstTransferECUList
)
{
    D32S i = 0;
    D32S iRet = ADM_ECODE_SUCCESSED;

    if (ADM_IS_NULL(pstTransPkgProgress) || ADM_IS_NULL(pstTransferECUList))
    {
        log_e( "Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }

    pstTransPkgProgress->num = pstTransferECUList->num;
    if ((D32U)0 >= pstTransPkgProgress->num)
    {
        pstTransPkgProgress->num = 0;
        log_d("Transfer ecu list is zero.");
        return ADM_ECODE_SUCCESSED;
    }

    pstTransPkgProgress->pstProgressList = adm_tps_newTransPkgProgressInfoList(pstTransPkgProgress->num);
    if (ADM_IS_NULL(pstTransPkgProgress->pstProgressList))
    {
        log_e("Failed to new transfer package progress info list, err[%d]", ADM_ECODE_NO_MEMORY);
        return ADM_ECODE_NO_MEMORY;
    }
  
    for (i = 0; (D32U)i < pstTransferECUList->num; i++)
    {
        pstTransPkgProgress->pstProgressList[i].ecuReqid = 
            adm_strdup(pstTransferECUList->pstEcuPkg[i].ecuReqid);
        pstTransPkgProgress->pstProgressList[i].ecuSwDid = 
            adm_strdup(pstTransferECUList->pstEcuPkg[i].ecuSwDid);
          
        pstTransPkgProgress->pstProgressList[i].stProgress.speed = 
            pstTransferECUList->pstEcuPkg[i].stTransProcess.stProgress.speed;
        pstTransPkgProgress->pstProgressList[i].stProgress.totalTime = 
            pstTransferECUList->pstEcuPkg[i].stTransProcess.stProgress.totalTime;               
        pstTransPkgProgress->pstProgressList[i].stProgress.remainTime = 
            pstTransferECUList->pstEcuPkg[i].stTransProcess.stProgress.remainTime;
        pstTransPkgProgress->pstProgressList[i].stProgress.saveBytes = 
            pstTransferECUList->pstEcuPkg[i].stTransProcess.stProgress.saveBytes;
        pstTransPkgProgress->pstProgressList[i].stProgress.totalBytes = 
            pstTransferECUList->pstEcuPkg[i].stTransProcess.stProgress.totalBytes;
    }

    return iRet;
}


#if 0
#endif


/*!
 *******************************************************************************
 * 检查升级包是否需要分包.
 *
 * \note 检查分包状态，未分包完成的则需要进行分包
 *
 * \param[in]  pstTransProcess ecu包分包进度信息
 * \retval     DBOOL           TRUE: 需要分包, FALSE：不需要分包
 *******************************************************************************
 */
static DBOOL __adm_tps_checkPackageIsNeedTrans(ADM_TPS_TRANSPROCESS_T *pstTransProcess)
{
    if (ADM_IS_NULL(pstTransProcess))
    {
        log_e("Invalid input param");
        return FALSE;
    }

    //if (pstTransProcess->stProgress.totalBytes != pstTransProcess->stProgress.saveBytes)
    //{
    //    return TRUE;
    //}

    if (ADM_TPS_TRANSPKG_STATE_FINISH != pstTransProcess->stResult.eTransState)
    {
        return TRUE;
    }

    return FALSE;
}



#if 0
#endif

/*!
 *******************************************************************************
 * 获取预分包条件信息.
 * 
 * \note   
 *
 * \param[out]  pstTransCondition 预分包条件信息 
 * \retval      D32S
 *******************************************************************************
 */
D32S adm_tps_getTransferCondition(ADM_DPS_TRANSFERCONDITION_T *pstTransCondition)
{
    D32S iRet = ADM_ECODE_SUCCESSED;

    if (ADM_IS_NULL(pstTransCondition))
    {
        log_e( "Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }
    
    if (FALSE == gstTPSTransferInfo.isInited)
    {
        log_e("TPS module not init, err[%d]", ADM_ECODE_MODULE_UNINIT);
        return ADM_ECODE_MODULE_UNINIT;
    }

    //!< 条件赋值 从 gstTPSTransferInfo.stDownCondition 获取
    (void)pthread_rwlock_rdlock(&gstTPSTransferInfo.stRWlock);
    iRet = adm_dps_fillTransferConditionInfo(pstTransCondition, &gstTPSTransferInfo.stTransferCondition);
    if (ADM_IS_ERR(iRet))
    {
        (void)pthread_rwlock_unlock(&gstTPSTransferInfo.stRWlock);
        log_e("Failed to file transfer condition, err[%d]", iRet);
        return iRet;
    }
    (void)pthread_rwlock_unlock(&gstTPSTransferInfo.stRWlock);

    return ADM_ECODE_SUCCESSED;
}

/*!
 *******************************************************************************
 * 获取预分包ecu数量.
 * 
 * \note   
 *
 * \param[out]  pi32WaitTransEcuNum 预分包ecu数量 
 * \retval      D32S
 *******************************************************************************
 */
D32S adm_tps_getWaitTransferEcuNum(D32S *pi32WaitTransEcuNum)
{
    D32S i = 0;

    if (ADM_IS_NULL(pi32WaitTransEcuNum))
    {
        log_e( "Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }

    if (FALSE == gstTPSTransferInfo.isInited)
    {
        log_e("TPS module not init, err[%d]", ADM_ECODE_MODULE_UNINIT);
        return ADM_ECODE_MODULE_UNINIT;
    }

    *pi32WaitTransEcuNum = 0;
    (void)pthread_rwlock_rdlock(&gstTPSTransferInfo.stRWlock);
    for (i = 0; ((D32U)i < gstTPSTransferInfo.stTransferECUList.num); i++)
    {
        log_i("CheckPackageIsNeedTran: totalBytes[%lld], saveBytes[%lld], state[%d]", 
            gstTPSTransferInfo.stTransferECUList.pstEcuPkg[i].stTransProcess.stProgress.totalBytes, 
            gstTPSTransferInfo.stTransferECUList.pstEcuPkg[i].stTransProcess.stProgress.saveBytes,
            gstTPSTransferInfo.stTransferECUList.pstEcuPkg[i].stTransProcess.stResult.eTransState);
        if (__adm_tps_checkPackageIsNeedTrans(&gstTPSTransferInfo.stTransferECUList.pstEcuPkg[i].stTransProcess))
        {
            ++(*pi32WaitTransEcuNum);
        }
    }
    (void)pthread_rwlock_unlock(&gstTPSTransferInfo.stRWlock);

    return ADM_ECODE_SUCCESSED;
}

static D16U tpsHtoi(const DCHAR *pHex)
{
    D32S iHex = 0;
    if (NULL != pHex)
    {
        iHex = htoi(pHex);
    }
    return (D16U)iHex;
}
/*!
 *******************************************************************************
 * 获取待分包ECU信息
 *
 * \note
 *
 * \param[in]    inNum              想要获取的待下载升级包个数
 * \param[in]    bIsAllowIdelTrans  是否需要支持闲时传输
 * \param[in]    bIsInIdel          是否闲时传输操作
 * \param[inout] pstEcuTransPkgList 待分包ecu信息
 * \param[out]   outNum             实际返回的待下载升级包个数
 * \retval       D32S
 *******************************************************************************
 */
D32S adm_tps_getTobeTransferEcuPackageInfo
(
    D32S  inNum, 
    DBOOL bIsAllowIdelTrans,
    DBOOL bIsInIdel,
    ADM_TPS_TRANSFERPKG_T *pstEcuTransPkgList,
    D32S *outNum
)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    D32S i = 0;
    D32S j = 0;
    D16U uiIdelInstanceId = 0;

    if ((0 == inNum) || ADM_IS_NULL(outNum) || ADM_IS_NULL(pstEcuTransPkgList))
    {
        log_e("Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }

    if (FALSE == gstTPSTransferInfo.isInited)
    {
        log_e("TPS module not init, err[%d]", ADM_ECODE_MODULE_UNINIT);
        return ADM_ECODE_MODULE_UNINIT;
    }

    (void)pthread_rwlock_rdlock(&gstTPSTransferInfo.stRWlock);

    if (DTRUE == bIsAllowIdelTrans)
    {
        uiIdelInstanceId = adm_soa_getIdelTransInstanceId();
    }

    while(((D32U)i < gstTPSTransferInfo.stTransferECUList.num) && (j < inNum))
    {
        log_i("CheckPackageIsNeedTran: totalBytes[%lld], saveBytes[%lld], state[%d]", 
            gstTPSTransferInfo.stTransferECUList.pstEcuPkg[i].stTransProcess.stProgress.totalBytes, 
            gstTPSTransferInfo.stTransferECUList.pstEcuPkg[i].stTransProcess.stProgress.saveBytes,
            gstTPSTransferInfo.stTransferECUList.pstEcuPkg[i].stTransProcess.stResult.eTransState);
        if (__adm_tps_checkPackageIsNeedTrans(&gstTPSTransferInfo.stTransferECUList.pstEcuPkg[i].stTransProcess))
        {
            if (DFALSE == bIsInIdel)
            {
                if (DTRUE == bIsAllowIdelTrans && (tpsHtoi(gstTPSTransferInfo.stTransferECUList.pstEcuPkg[i].transferInstanceId) == uiIdelInstanceId))
                {
                    log_i("instanceId[%u] ignore", uiIdelInstanceId);
                    i++;
                    continue;
                }
                iRet = __adm_tps_fillEcuTransPackage(&pstEcuTransPkgList[j],
                                                     &gstTPSTransferInfo.stTransferECUList.pstEcuPkg[i]);
                if (ADM_IS_ERR(iRet))
                {
                    log_e("Failed to fill wait transfer ecu package info in normal mode, err[%d]", iRet);
                    break;
                }
            }
            else 
            {
                if (tpsHtoi(gstTPSTransferInfo.stTransferECUList.pstEcuPkg[i].transferInstanceId) != uiIdelInstanceId)
                {
                    i++;
                    continue;
                }
                iRet = __adm_tps_fillEcuTransPackage(&pstEcuTransPkgList[j], 
                                &gstTPSTransferInfo.stTransferECUList.pstEcuPkg[i]);
                if (ADM_IS_ERR(iRet))
                {
                    log_e( "Failed to fill wait transfer ecu package info in idel mode, err[%d]", iRet);
                    break;
                }
            }
            j++;
        }
        i++;
    }

    *outNum = j;
    //!< 中途有填充失败则释放其内部所申请的内存
    if (ADM_IS_ERR(iRet))
    {
        adm_tps_cleanTransEcuPackageList((D32U)inNum, pstEcuTransPkgList);
        *outNum = 0;
    }

    (void)pthread_rwlock_unlock(&gstTPSTransferInfo.stRWlock);

    return iRet;
}

/*!
 *******************************************************************************
 * 获取闲时传输ecu数量
 *
 * \note
 *
 * \param[out]   outNum             实际返回的待下载升级包个数
 * \retval       D32U
 *******************************************************************************
 */
D32S adm_tps_getIdelTransferEcuCnt(D32U *outNum)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    D32S i = 0;
    D32S j = 0;
    D16U uiIdelInstanceId = 0;

    if (ADM_IS_NULL(outNum))
    {
        log_e("Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }

    if (FALSE == gstTPSTransferInfo.isInited)
    {
        log_e("TPS module not init, err[%d]", ADM_ECODE_MODULE_UNINIT);
        return ADM_ECODE_MODULE_UNINIT;
    }

    (void)pthread_rwlock_rdlock(&gstTPSTransferInfo.stRWlock);

    uiIdelInstanceId = adm_soa_getIdelTransInstanceId();

    while (((D32U)i < gstTPSTransferInfo.stTransferECUList.num))
    {
        if (__adm_tps_checkPackageIsNeedTrans(&gstTPSTransferInfo.stTransferECUList.pstEcuPkg[i].stTransProcess))
        {
            if (tpsHtoi(gstTPSTransferInfo.stTransferECUList.pstEcuPkg[i].transferInstanceId) != uiIdelInstanceId)
            {
                i++;
                continue;
            }
            j++;
        }
        i++;
    }

    *outNum = j;
   
    if (ADM_IS_ERR(iRet))
    {
        *outNum = 0;
    }

    (void)pthread_rwlock_unlock(&gstTPSTransferInfo.stRWlock);

    return iRet;
}

/*!
 *******************************************************************************
 * 获取待分包ECU信息(空闲传输)
 *
 * \note
 *
 * \param[in]    inNum              想要获取的待下载升级包个数
 * \param[inout] pstEcuTransPkgList 待分包ecu信息
 * \param[out]   outNum             实际返回的待下载升级包个数
 * \retval       D32S
 *******************************************************************************
 */
D32S adm_tps_getIdelTobeTransferEcuPackageInfo(
    D32S inNum,
    ADM_TPS_TRANSFERPKG_T *pstEcuTransPkgList,
    D32S *outNum)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    D32S i = 0;
    D32S j = 0;

    if ((0 == inNum) || ADM_IS_NULL(outNum) || ADM_IS_NULL(pstEcuTransPkgList))
    {
        log_e("Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }

    if (FALSE == gstTPSTransferInfo.isInited)
    {
        log_e("TPS module not init, err[%d]", ADM_ECODE_MODULE_UNINIT);
        return ADM_ECODE_MODULE_UNINIT;
    }

    (void)pthread_rwlock_rdlock(&gstTPSTransferInfo.stRWlock);

    while(((D32U)i < gstTPSTransferInfo.stTransferECUList.num) && (j < inNum))
    {
        log_i("CheckPackageIsNeedTran: totalBytes[%lld], saveBytes[%lld], state[%d]",
              gstTPSTransferInfo.stTransferECUList.pstEcuPkg[i].stTransProcess.stProgress.totalBytes,
              gstTPSTransferInfo.stTransferECUList.pstEcuPkg[i].stTransProcess.stProgress.saveBytes,
              gstTPSTransferInfo.stTransferECUList.pstEcuPkg[i].stTransProcess.stResult.eTransState);
        if (__adm_tps_checkPackageIsNeedTrans(&gstTPSTransferInfo.stTransferECUList.pstEcuPkg[i].stTransProcess))
        {
            iRet = __adm_tps_fillEcuTransPackage(&pstEcuTransPkgList[j],
                                                 &gstTPSTransferInfo.stTransferECUList.pstEcuPkg[i]);
            if (ADM_IS_ERR(iRet))
            {
                log_e("Failed to fill wait transfer ecu package info, err[%d]", iRet);
                break;
            }
            j++;
        }
        i++;
    }

    *outNum = j;
    //!< 中途有填充失败则释放其内部所申请的内存
    if (ADM_IS_ERR(iRet))
    {
        adm_tps_cleanTransEcuPackageList((D32U)inNum, pstEcuTransPkgList);
        *outNum = 0;
    }

    (void)pthread_rwlock_unlock(&gstTPSTransferInfo.stRWlock);

    return iRet;
}

#if 0
#endif

/*!
 *******************************************************************************
 * 释放分包文件fileID内存信息.
 *
 * \note
 * 
 * \param[in]  pFileID 文件fileID
 * \retval     none
 *******************************************************************************
 */
void adm_tps_freeTransFileID(DCHAR **pFileID)
{
    adm_free(*pFileID);

    return;
}

/*!
 *******************************************************************************
 * 通过编码ecu信息生成预分包使用的fileID.
 *
 * \note  生成规则: ecuReqid_ecuSwDid_fileName
 *              若: ecuReqid/ecuSwDid/fileName 为 null，则用字符串 "null" 填充    
 * 
 * \param[in]  ecuReqid  诊断请求id
 * \param[in]  ecuSwDid  升级对象软件版本 DID
 * \param[in]  bIsRollback  是否是回滚包
 * \param[in]  fileName  文件名
 * \retval     DCHAR
 *******************************************************************************
 */
DCHAR* adm_tps_createTransFileIDByEcuInfo
(
    DCHAR *ecuReqid, 
    DCHAR *ecuSwDid, 
    DBOOL  bIsRollback,
    DCHAR *fileName
)
{
    D32U len = 0;
    DCHAR *pFileId = NULL;

    len += ((NULL != ecuReqid) ? adm_strlen(ecuReqid) : adm_strlen("null"));
    len += ((NULL != ecuSwDid) ? adm_strlen(ecuSwDid) : adm_strlen("null"));
    len += ((NULL != fileName) ? adm_strlen(fileName) : adm_strlen("null"));
    //!< 加 3 个"_"符号 + 3 个字符 "src"/"dst"，预留一个"\0"
    len += (D32U)7;

    pFileId = (DCHAR *)adm_malloc(len * sizeof(DCHAR));
    if (ADM_IS_NULL(pFileId))
    {
        log_e("Failed to malloc");
        return NULL;
    }
    (void)adm_memzero(pFileId, len * sizeof(DCHAR));

    if (FALSE == bIsRollback)
    {
        (void)adm_snprintf(pFileId, len * sizeof(DCHAR), "%s_%s_%s_%s", 
                          (NULL != ecuReqid) ? (ecuReqid) : ("null"),
                          (NULL != ecuSwDid) ? (ecuSwDid) : ("null"),
                          "dst",
                          (NULL != fileName) ? (fileName) : ("null"));
    }
    else
    {
        (void)adm_snprintf(pFileId, len * sizeof(DCHAR), "%s_%s_%s_%s", 
                          (NULL != ecuReqid) ? (ecuReqid) : ("null"),
                          (NULL != ecuSwDid) ? (ecuSwDid) : ("null"),
                          "src",
                          (NULL != fileName) ? (fileName) : ("null"));
    }

    return pFileId;
}


#if 0
#endif


/*!
 *******************************************************************************
 * 检查fileId是否符合该ecu的升级包描述
 *
 * \note 
 * 
 * \param[in]  fileId     分包文件id
 * \param[in]  pstEcuPkg  待分包ecu包信息
 * \param[in]  isSrcPkg   匹配到的是否是源包
 * \param[out] isConform  是否符合
 * \retval     D32S
 *******************************************************************************
 */
static D32S __adm_tps_checkTransPackageFileIdIsConform
(
    const DCHAR           *fileId,
    ADM_TPS_TRANSFERPKG_T *pstEcuPkg,
    DBOOL                 *isSrcPkg,
    DBOOL                 *isConform
)
{
    DCHAR *newFileId = NULL;

    if (ADM_IS_NULL(fileId) || ADM_IS_NULL(pstEcuPkg) || ADM_IS_NULL(isSrcPkg) || ADM_IS_NULL(isConform))
    {
        log_e( "Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }

    //!< 是否是原包的fileId
    if (NULL != pstEcuPkg->pstSrcPkgInfo)
    {
        newFileId = adm_tps_createTransFileIDByEcuInfo(
                                pstEcuPkg->ecuReqid, 
                                pstEcuPkg->ecuSwDid, 
                                TRUE,
                                pstEcuPkg->pstSrcPkgInfo->fileName);
        if (ADM_IS_NULL(newFileId))
        {
            log_e("Failed to create transfer file id, err[%d]", ADM_ECODE_NO_MEMORY);
            return ADM_ECODE_NO_MEMORY;
        }
        
        if (0 == adm_strcmp(fileId, newFileId))
        {
            *isSrcPkg  = TRUE;
            *isConform = TRUE;
            adm_tps_freeTransFileID(&newFileId);
            return ADM_ECODE_SUCCESSED;
        }
        *isConform = FALSE;
        adm_tps_freeTransFileID(&newFileId);
    }

    //!< 是否是目标包的fileId
    newFileId = adm_tps_createTransFileIDByEcuInfo(
                                pstEcuPkg->ecuReqid, 
                                pstEcuPkg->ecuSwDid, 
                                FALSE,
                                pstEcuPkg->stDstPkgInfo.fileName);
    if (ADM_IS_NULL(newFileId))
    {
        log_e("Failed to create transfer file id, err[%d]", ADM_ECODE_NO_MEMORY);
        return ADM_ECODE_NO_MEMORY;
    }

    *isSrcPkg  = FALSE;
    *isConform = ((0 == adm_strcmp(fileId, newFileId)) ? TRUE : FALSE);

    adm_tps_freeTransFileID(&newFileId);

    return ADM_ECODE_SUCCESSED;
}

/*!
 *******************************************************************************
 * 获取ecu分包结果信息
 *
 * \note
 * 
 * \param
 * \retval  ADM_TPS_TRANSPKGRESULT_T*
 *******************************************************************************
 */
ADM_TPS_TRANSPKGRESULT_T *adm_tps_getEcuTransResult(void)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    ADM_TPS_TRANSPKGRESULT_T *pstTransPkgResult = NULL;

    if (FALSE == gstTPSTransferInfo.isInited)
    {
        log_e("TPS module not init, err[%d]", ADM_ECODE_MODULE_UNINIT);
        return NULL;
    }

    pstTransPkgResult = adm_tps_newTransPackageResult();
    if (ADM_IS_NULL(pstTransPkgResult))
    {
        log_e("Failed to new down package process info, err[%d]", ADM_ECODE_NO_MEMORY);
        return NULL;
    }

    iRet = __adm_tps_fillTransPackageResult(pstTransPkgResult, &gstTPSTransferInfo.stTransferECUList);
    if (ADM_IS_ERR(iRet))
    {
        adm_tps_delTransPackageResult(&pstTransPkgResult);
        log_e( "Failed to fill down package result, err[%d]", iRet);
        return NULL;
    }

    return pstTransPkgResult;
}


/*!
 *******************************************************************************
 * 获取ecu分包进度信息
 *
 * \note
 * 
 * \param
 * \retval  ADM_TPS_TRANSPKGPROGRESS_T*
 *******************************************************************************
 */
ADM_TPS_TRANSPKGPROGRESS_T *adm_dps_getEcuTransProgress(void)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    ADM_TPS_TRANSPKGPROGRESS_T *psTransPkgProgress = NULL;

    if (FALSE == gstTPSTransferInfo.isInited)
    {
        log_e("TPS module not init, err[%d]", ADM_ECODE_MODULE_UNINIT);
        return NULL;
    }

    psTransPkgProgress = adm_tps_newTransPackageProgress();
    if (ADM_IS_NULL(psTransPkgProgress))
    {
        log_e("Failed to new transfer package process info, err[%d]", ADM_ECODE_NO_MEMORY);
        return NULL;
    }

    iRet = __adm_tps_fillTransPackageProgress(psTransPkgProgress, &gstTPSTransferInfo.stTransferECUList);
    if (ADM_IS_ERR(iRet))
    {
        adm_tps_delTransPackageProgress(&psTransPkgProgress);
        log_e( "Failed to fill transfer package progress, err[%d]", iRet);
        return NULL;
    }

    return psTransPkgProgress;
}


/*!
 *******************************************************************************
 * 更新升级包分包结果
 *
 * \note
 * 
 * \param[in]    fileId      分包文件id
 * \param[in]    pstResult   分包结果
 * \param[in]    pkgSavePath 升级包存储路径(选填)
 * \retval       D32S
 *******************************************************************************
 */
D32S adm_tps_updateTransPackageResult
(
    const DCHAR           *fileId,
    ADM_TPS_TRANSRESULT_T *pstResult,
    const DCHAR           *pkgSavePath
)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    D32S  i = 0;
    DCHAR *filePath = NULL;
    DBOOL isConform = FALSE;
    DBOOL isSrcPkg  = FALSE;
    ADM_TPS_TRANSFERPKG_T *pstEcuPkg = NULL;
    DCHAR szTransPkgKeyId[ADM_COM_KEYID_LEN] = {0};

    if (ADM_IS_NULL(fileId) || ADM_IS_NULL(pstResult))
    {
        log_e( "Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }

    if (FALSE == gstTPSTransferInfo.isInited)
    {
        log_e("TPS module not init, err[%d]", ADM_ECODE_MODULE_UNINIT);
        return ADM_ECODE_MODULE_UNINIT;
    }

    (void)pthread_rwlock_wrlock(&gstTPSTransferInfo.stRWlock);

    //!< 更新进度到缓存
    for (i = 0; (D32U)i< gstTPSTransferInfo.stTransferECUList.num; i++)
    {
        pstEcuPkg = &gstTPSTransferInfo.stTransferECUList.pstEcuPkg[i];

        iRet = __adm_tps_checkTransPackageFileIdIsConform(fileId, pstEcuPkg, &isSrcPkg, &isConform);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to check transfer package file id, err[%d]", iRet);
            break;
        }
        log_d("Find wait update reuslt ecu fileId[%s], isSrcPkg[%d], isConform[%d], err[%d]", fileId, isSrcPkg, isConform);

        if (TRUE == isConform)
        {
            //!< 更新下载状态
            pstEcuPkg->stTransProcess.stResult.result      = pstResult->result;
            pstEcuPkg->stTransProcess.stResult.eTransState = pstResult->eTransState;
            adm_free(pstEcuPkg->stTransProcess.stResult.failReason);
            pstEcuPkg->stTransProcess.stResult.failReason = adm_strdup(pstResult->failReason);

            //!< 更新到 kv
            (void)adm_memzero(&szTransPkgKeyId, sizeof(szTransPkgKeyId));
            (void)adm_snprintf(szTransPkgKeyId, sizeof(szTransPkgKeyId), "%s_%s", pstEcuPkg->ecuReqid, pstEcuPkg->ecuSwDid);
            iRet = adm_tps_setTransResult2KV(szTransPkgKeyId, &pstEcuPkg->stTransProcess.stResult);
            if (ADM_IS_ERR(iRet))
            {
                log_e("Failed to set trans result info to KV, keyId[%s], err[%d]", szTransPkgKeyId, iRet);
            }

            //!< 如果是slave下载的，则需要更新slave侧的存储包位置
            if (NULL != pkgSavePath)
            {
                if (TRUE == isSrcPkg)
                {
                    adm_free(pstEcuPkg->pstSrcPkgInfo->filePath);
                    pstEcuPkg->pstSrcPkgInfo->filePath = adm_strdup(pkgSavePath);
                    filePath = pstEcuPkg->pstSrcPkgInfo->filePath;
                }
                else
                {
                    adm_free(pstEcuPkg->stDstPkgInfo.filePath);
                    pstEcuPkg->stDstPkgInfo.filePath = adm_strdup(pkgSavePath);
                    filePath = pstEcuPkg->stDstPkgInfo.filePath;
                }

                //!< 更新存储路径到kv中 
                iRet = adm_tps_updateTransPkgPath2TransEcuInfoKV(pstEcuPkg->ecuReqid, pstEcuPkg->ecuSwDid, isSrcPkg, filePath);
                if (ADM_IS_ERR(iRet))
                {
                    log_e("Failed to set trans result info to KV, isSrcPkg[%d], err[%d]", isSrcPkg, iRet);
                }
            }

            log_d("updateTransResult: result[%d], state[%d], failReason[%s], filePath[%s]",
                    pstResult->result,
                    pstResult->eTransState,
                    ADM_PRINT_STR(pstResult->failReason),
                    (NULL != pkgSavePath) ? (pkgSavePath) : (ADM_PRINT_STR(filePath)));

            break;
        }
    }

    (void)pthread_rwlock_unlock(&gstTPSTransferInfo.stRWlock);

    if (ADM_IS_ERR(iRet))
    {
        log_e("Failed to update transfer package progress, fileId[%s], err[%d]", 
                fileId, iRet);
    }

    // TODO: 更新到kv存储

    return iRet;
}

/*!
 *******************************************************************************
 * 更新升级包分包进度信息
 *
 * \note 不会更新 totalBytes 字段，总进度在 init 时初始化
 * 
 * \param[in]    fileId      分包文件id
 * \param[in]    pstProgress 分包进度信息
 * \retval       D32S
 *******************************************************************************
 */
D32S adm_tps_updateTransPackageProgress
(
    DCHAR                   *fileId,
    ADM_TPS_TRANSPROGRESS_T *pstProgress
)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    D32S  i = 0;
    DBOOL isConform = FALSE;
    DBOOL isSrcPkg  = FALSE;
    ADM_TPS_TRANSFERPKG_T *pstEcuPkg = NULL;
    DCHAR szTransPkgKeyId[ADM_COM_KEYID_LEN] = {0};

    if (ADM_IS_NULL(fileId) || ADM_IS_NULL(pstProgress))
    {
        log_e( "Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }

    if (FALSE == gstTPSTransferInfo.isInited)
    {
        log_e("TPS module not init, err[%d]", ADM_ECODE_MODULE_UNINIT);
        return ADM_ECODE_MODULE_UNINIT;
    }

    (void)pthread_rwlock_wrlock(&gstTPSTransferInfo.stRWlock);

    //!< 更新进度到缓存
    for (i = 0; (D32U)i< gstTPSTransferInfo.stTransferECUList.num; i++)
    {
        pstEcuPkg = &gstTPSTransferInfo.stTransferECUList.pstEcuPkg[i];

        iRet = __adm_tps_checkTransPackageFileIdIsConform(fileId, pstEcuPkg, &isSrcPkg, &isConform);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to check transfer package file id, err[%d]", iRet);
            break;
        }

        if (TRUE == isConform)
        {
            //!< 此处不能更新 totalBytes 大小, tps 层 totalBytes 为 srcPkg + dstPkg
            //!< tpm 层 totalBytes 为 srcPkg or dstPkg 信息
            pstEcuPkg->stTransProcess.stProgress.speed      = pstProgress->speed;
            pstEcuPkg->stTransProcess.stProgress.totalTime  = pstProgress->totalTime;
            pstEcuPkg->stTransProcess.stProgress.remainTime = pstProgress->remainTime;
            pstEcuPkg->stTransProcess.stProgress.saveBytes  = pstProgress->saveBytes;

            //!< 更新到 kv
            (void)adm_memzero(&szTransPkgKeyId, sizeof(szTransPkgKeyId));
            (void)adm_snprintf(szTransPkgKeyId, sizeof(szTransPkgKeyId), "%s_%s", pstEcuPkg->ecuReqid, pstEcuPkg->ecuSwDid);
            iRet = adm_tps_setTransProgress2KV(szTransPkgKeyId, &pstEcuPkg->stTransProcess.stProgress);
            if (ADM_IS_ERR(iRet))
            {
                log_e("Failed to set trans progress info to KV, keyId[%s], err[%d]", szTransPkgKeyId, iRet);
            }

            log_i("UpdateTransProgress: progress[%d]",  
                   (D32S)((pstProgress->saveBytes * 100) / pstEcuPkg->stTransProcess.stProgress.totalBytes));
            break;
        }
    }

    (void)pthread_rwlock_unlock(&gstTPSTransferInfo.stRWlock);

    if (ADM_IS_ERR(iRet))
    {
        log_e("Failed to update transfer package progress, fileId[%s], err[%d]", 
                fileId, iRet);
    }

    return iRet;
}


#if 0
#endif


/*!
 *******************************************************************************
 * 获取ecu传输包信息
 *
 * \note 
 * 
 * \param[in]    fileId      分包文件id
 * \param[in]    pstProgress 分包进度信息
 * \retval       D32S
 *******************************************************************************
 */
static D32S __adm_tps_getEcuTransPackageInfo
(
    const DCHAR          *ecuReqid, 
    const DCHAR          *ecuSwDid,
    DBOOL                 isRollback,
    ADM_COM_FILEOBJECT_T *pstTransPkgInfo
)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    D32S  i = 0;
    ADM_TPS_TRANSFERPKG_T *pstEcuPkg = NULL;

    if (ADM_IS_NULL(ecuReqid) || ADM_IS_NULL(ecuSwDid) || ADM_IS_NULL(pstTransPkgInfo))
    {
        log_e( "Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }

    if (FALSE == gstTPSTransferInfo.isInited)
    {
        log_e("TPS module not init, err[%d]", ADM_ECODE_MODULE_UNINIT);
        return ADM_ECODE_MODULE_UNINIT;
    }

    (void)pthread_rwlock_wrlock(&gstTPSTransferInfo.stRWlock);

    //!< loop search
    for (i = 0; (D32U)i< gstTPSTransferInfo.stTransferECUList.num; i++)
    {
        pstEcuPkg = &gstTPSTransferInfo.stTransferECUList.pstEcuPkg[i];

        if ((0 == adm_strcasecmp(ecuReqid, pstEcuPkg->ecuReqid)) &&
            (0 == adm_strcasecmp(ecuSwDid, pstEcuPkg->ecuSwDid)))
        {
            if (TRUE == isRollback)
            {
                iRet = adm_com_copyFileObject(pstTransPkgInfo, pstEcuPkg->pstSrcPkgInfo);
            }
            else
            {
                iRet = adm_com_copyFileObject(pstTransPkgInfo, &pstEcuPkg->stDstPkgInfo);
            }
            break;
        }
    }

    //!< 未找到的情况 
    if ((D32U)i == gstTPSTransferInfo.stTransferECUList.num)
    {
        iRet = ADM_ECODE_DOWN_RESOURCE_NOFIND;
        log_e("Failed to find ecu info from transInfo, ecuReqid[%s], ecuSwDid[%s], err[%d]", 
                ecuReqid, ecuSwDid, iRet);
    }

    (void)pthread_rwlock_unlock(&gstTPSTransferInfo.stRWlock);

    if (ADM_IS_ERR(iRet))
    {
        log_e("Failed to get ecu transfer package info, ecuReqid[%s], ecuSwDid[%s], err[%d]", 
                ecuReqid, ecuSwDid, iRet);
    }

    return iRet;
}


/*!
 *******************************************************************************
 * 更新传输完成的ecu的升级包存储路径到 pis 模块
 *
 * \note 传输完成后会更新升级包存储路径为对端的存储路径，
 *           此处只需要获取在更新即可
 * 
 * \param[in]    ecuReqid      诊断请求id
 * \param[in]    ecuSwDid      升级对象软件版本 DID
 * \param[in]    isRollback    是否是回滚包
 * \retval       D32S
 *******************************************************************************
 */
D32S adm_tps_updateEcuPkgSavePath2PIS
(
    const DCHAR          *ecuReqid, 
    const DCHAR          *ecuSwDid,
    DBOOL                 isRollback
)
{
    D32S iRet = ADM_ECODE_FAILED;
    ADM_PIS_FILE_T         stImagePackage     = {0};
    ADM_PIS_PACKAGE_HEAD_T stPisPkgHead       = {0};
    ADM_COM_FILEOBJECT_T   stTransPkgInfo     = {0};
    DCHAR szEventBuf[ADM_COM_LEN_256] = {0};  //mod by wl 20220321

    if (ADM_IS_NULL(ecuReqid) || ADM_IS_NULL(ecuSwDid))
    {
        log_e("Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }

    (void)adm_memzero(szEventBuf, sizeof(szEventBuf));
    (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), 
            "update path 2 pis, ecuReqid[%s], ecuSwDid[%s], isRollback[%d], wait ecu trans pkg info.", 
            ecuReqid, ecuSwDid, isRollback);
    adm_tps_eventReport(ADM_COM_MOD_TPS, FALSE, ADM_EVENT_ID_TI_019, ADM_OCS_EVENT_LEVEL_TRACE, "vehicle update once ecu save path 2 pis.", szEventBuf);

    //!< 获取对应的ecu信息 
    (void)adm_memzero(&stTransPkgInfo, sizeof(stTransPkgInfo));
    iRet = __adm_tps_getEcuTransPackageInfo(ecuReqid, ecuSwDid, isRollback, &stTransPkgInfo);
    if (ADM_IS_ERR(iRet))
    {
        log_e("Failed to get ecu transfer package info, err[%d]", iRet);
        (void)adm_memzero(szEventBuf, sizeof(szEventBuf));
        (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), 
                "Failed to get ecu transfer package info, ecuReqid[%s], ecuSwDid[%s], isRollback[%d], err[%d]", 
                    ecuReqid, ecuSwDid, isRollback, iRet);
        adm_tps_eventReport(ADM_COM_MOD_TPS, TRUE, ADM_EVENT_ID_TI_020, ADM_OCS_EVENT_LEVEL_ERROR, 
                    "vehicle update once ecu save path 2 pis failed!", szEventBuf);
        return iRet;
    }

    //!< pis 包头
    (void)adm_memzero(&stPisPkgHead, sizeof(stPisPkgHead));
    stPisPkgHead.ecuReqId = adm_strdup(ecuReqid);
    stPisPkgHead.ecuSwDid = adm_strdup(ecuSwDid);
    stPisPkgHead.state = ADM_PIS_PACKAGE_STATE_DECRYPT_FINISH;
    stPisPkgHead.type  = ((TRUE == isRollback) ? \
        (ADM_PIS_PACKAGE_TYPE_ECU_SRC_PACKAGE) : (ADM_PIS_PACKAGE_TYPE_ECU_DST_PACKAGE));

    //!< pis 包数据    
    (void)adm_memzero(&stImagePackage, sizeof(stImagePackage));
    stImagePackage.path = adm_strdup(stTransPkgInfo.filePath);
    stImagePackage.hash = adm_strdup(stTransPkgInfo.hash);
    stImagePackage.size = stTransPkgInfo.size;
    //!< 设置升级包的文件信息 
    adm_pis_packageImageSet(&stPisPkgHead, &stImagePackage);

    //!< 设置 升级包的签名信息 
    if (NULL != stTransPkgInfo.pstFileSecurityInfo)
    {
        adm_pis_packageSecuritySet(&stPisPkgHead, stTransPkgInfo.pstFileSecurityInfo);
        adm_pis_packageMakeTypeSet(&stPisPkgHead, stTransPkgInfo.makePackageType);
    }

    (void)adm_memzero(szEventBuf, sizeof(szEventBuf));
    (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), 
            "update path 2 pis, ecuReqid[%s], ecuSwDid[%s], isRollback[%d], path[%s], hash[%s], size[%lld], haveFileScrt[%d]", 
            ecuReqid, ecuSwDid, isRollback, stImagePackage.path, stImagePackage.hash, stImagePackage.size,
            ((ADM_IS_NULL(stTransPkgInfo.pstFileSecurityInfo)) ? (0) : (1)));
    adm_tps_eventReport(ADM_COM_MOD_TPS, FALSE, ADM_EVENT_ID_TI_021, ADM_OCS_EVENT_LEVEL_DEBUG, "vehicle update once ecu save path 2 pis successed.", szEventBuf);

    //!< 收尾临时内存
    adm_free(stPisPkgHead.ecuReqId);
    adm_free(stPisPkgHead.ecuSwDid);
    adm_free(stImagePackage.path);
    adm_free(stImagePackage.hash);
    adm_com_cleanFileObject(&stTransPkgInfo);

    return ADM_ECODE_SUCCESSED;
}



#if 0
#endif

/*!
 *******************************************************************************
 * 启动进程.
 *
 * \note 
 *
 * \param[in]  pcProcCmd   待启动的进程命令
 * \retval     D32S        0: 成功 non-0：失败
 *******************************************************************************
 */
D32S adm_tps_startProgress(DCHAR *pcProcCmd)
{
    D32S iPid = 0;
    D32S iFd = 0;
    D32S istatus = 0;
    D32U u32StrLen = 0;
    struct rlimit stRL;
    DCHAR szSystemCmd[512] = {0};
    
    if (ADM_IS_NULL(pcProcCmd))
    {
        log_e( "Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }

    //!< 创建子进程 --- vfork可以保证子进程先运行
    iPid = vfork();
    if (iPid < 0)    //!< 创建子进程失败 
    {
        log_e("Create process[%s] by fork failed", pcProcCmd);
        return iPid;
    }
    else if (0 != iPid)    //!< 父进程 
    {
        /*
         * 父进程等待子进程结束
         * 第三个参数0是waitpid最常用的参数
         * 在这里只要第三个参数不是立刻返回的参数就对执行结果没有影响
         */
        if (iPid != waitpid(iPid, &istatus, 0))
        {
            log_e("Waipid for process[%s] failed.status[%d]", pcProcCmd, istatus);
            return iPid;
        }

        //!< 子进程exit(istatus)带回的参数istatus
        return istatus;
    }
    else
    {
        ; /* 无任何必要行为 - ";"是可选的 */
    }

    //!< 子进程
    //!< 获取有多少个文件句柄
    (void)adm_memzero(&stRL, sizeof(stRL));
    if (0 > getrlimit((D32S)RLIMIT_NOFILE, &stRL))
    {
        exit(ADM_ECODE_SYSCALL_FALIED);
    }

    //!< 关闭所有原来进程打开的文件描述符
    if (RLIM_INFINITY == stRL.rlim_max)
    {
        //!< 这里暂定资源为 1024
        stRL.rlim_max = 1024;
    }

    //!< 不关闭标准输入、标准输出、错误输出这3个描述符
    for (iFd = 3; iFd < (D32S)stRL.rlim_max; ++iFd)
    {
        (void)close(iFd);
    }

    //!< 设置子进程的group ID
    (void)setsid();
    u32StrLen = adm_strlen(pcProcCmd);
    if ((D32U)0 == u32StrLen)
    {
        exit(ADM_ECODE_INVALID_STATE);
    }

    if (';' == pcProcCmd[u32StrLen - (D32U)1])
    {
        (void)adm_snprintf(szSystemCmd, sizeof(szSystemCmd), "{ %s }", pcProcCmd);
    }
    else
    {
        (void)adm_snprintf(szSystemCmd, sizeof(szSystemCmd), "{ %s; }", pcProcCmd);
    }

    if (-1 == execl("/bin/sh", "sh", "-c", szSystemCmd, NULL))
    //!< 当命令行执行失败才会返回，出错信息存放在errno; 否则子进程只执行到此注释之前
    {
        exit(errno);
    }
    exit(ADM_ECODE_SUCCESSED);

}


/*!
 *******************************************************************************
 * 更新传输缓存信息.
 *
 * \note 
 *
 * \param[in] pstDownTaskInfo         下载任务信息
 * \param[in] fnGetEcsConfInfo        获取ecs模块中该车平台的配置信息回调函数
 * \param[in] fnGetTransCond          获取dps中传输条件规则回调函数
 * \param[in] fnGetDownEcuPkgListInfo 获取dps中下载的ECU信息回调函数
 * \retval    D32S
 *******************************************************************************
 */
static D32S adm_tps_updateTransCacheInfo
(    
    ADM_DPS_DOWNTSAKINFO_T                      *pstDownTaskInfo,
    ADM_ECS_GETCONFINFO_CALLBACK_F               fnGetEcsConfInfo,
    ADM_DPS_GETTRANSCONDITION_CALLBACK_F         fnGetTransCond,
    ADM_DPS_GETDOWNECUPACKAGELISTINFO_CALLBACK_F fnGetDownEcuPkgListInfo
)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    ADM_ECS_RSCONFIGINFO_T stRsCfgInfo = {0};
    DCHAR szEventBuf[ADM_COM_LEN_256] = {0}; 
    ADM_DPS_DOWN_ECUPKGLIST_T *pstEcuPkgList = NULL;

    if (ADM_IS_NULL(pstDownTaskInfo) || ADM_IS_NULL(fnGetEcsConfInfo) ||
        ADM_IS_NULL(fnGetTransCond)  || ADM_IS_NULL(fnGetDownEcuPkgListInfo))
    {
        log_e("Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }

    (void)adm_memzero(szEventBuf, sizeof(szEventBuf));
    adm_tps_eventReport(ADM_COM_MOD_TPS, FALSE, ADM_EVENT_ID_TP_004, ADM_OCS_EVENT_LEVEL_DEBUG, "vehicle update transfer task info.", NULL);

    do 
    {
        adm_tps_cleanTransferCacheInfo(&gstTPSTransferInfo);
        //!< 填充下载任务信息 
        gstTPSTransferInfo.stDownTaskInfo.taskId     = pstDownTaskInfo->taskId;
        gstTPSTransferInfo.stDownTaskInfo.strategyId = pstDownTaskInfo->strategyId;
        gstTPSTransferInfo.stDownTaskInfo.sessionId  = adm_strdup(pstDownTaskInfo->sessionId);

        //!< 获取传输条件 
        iRet = fnGetTransCond(&gstTPSTransferInfo.stTransferCondition);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to get transfer condition from ecs module, err[%d]", iRet);
            (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), "Failed to get transfer condition from ecs module, err[%d]", iRet);
            break;
        }

        //!< 计算待传输ecu信息 
        (void)adm_memzero(&stRsCfgInfo, sizeof(stRsCfgInfo));
        iRet = fnGetEcsConfInfo(&stRsCfgInfo);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to get ota config info from ecs module, err[%d]", iRet);
            (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), "Failed to get ota config info from ecs module, err[%d]", iRet);
            break;
        }

        pstEcuPkgList = fnGetDownEcuPkgListInfo();
        if (ADM_IS_NULL(pstEcuPkgList))
        {
            log_e("Failed to get download ecu package list info, err[%d]", iRet);
            (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), "Failed to get download ecu package list info, err[%d]", iRet);
            break;
        }
        
        iRet = __adm_tps_filterWaitTransferEcu(pstEcuPkgList, &stRsCfgInfo.stPartCfg.stEcuCfgList, &gstTPSTransferInfo.stTransferECUList);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to filter wait trans ecu from ecs module, err[%d]", iRet);
            (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), "Failed to filter wait trans ecu from ecs module, err[%d]", iRet);
            break;
        }
        log_i("===> Wait transfer ecu num[%d]", gstTPSTransferInfo.stTransferECUList.num);
        (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), "Wait transfer ecu num[%d]", gstTPSTransferInfo.stTransferECUList.num);

        if ((D32U)0 == gstTPSTransferInfo.stTransferECUList.num)
        {
            break;
        }

        //!< 初始化待分包ecu进度信息 
        (void)__adm_tps_transferEcuPackageProcessInit(&gstTPSTransferInfo.stTransferECUList);
    } while(FALSE);

    adm_dps_delEcuPackageList(&pstEcuPkgList);
    adm_ecs_cleanRSConfigInfo(&stRsCfgInfo);
    if (ADM_IS_ERR(iRet))
    {
        //adm_dps_cleanEcuDownCondition(&pstTPSTransferInfo->stTransferCondition);
        adm_tps_delTransferEcuPackageList(gstTPSTransferInfo.stTransferECUList.num, &gstTPSTransferInfo.stTransferECUList.pstEcuPkg);
        gstTPSTransferInfo.stTransferECUList.num = 0;
        gstTPSTransferInfo.stTransferECUList.pstEcuPkg = NULL;

        adm_tps_eventReport(ADM_COM_MOD_TPS, TRUE, ADM_EVENT_ID_TP_005, ADM_OCS_EVENT_LEVEL_ERROR, "vehicle update transfer task info failed!", szEventBuf);    
    }
    else
    {
        adm_tps_eventReport(ADM_COM_MOD_TPS, TRUE, ADM_EVENT_ID_TP_006, ADM_OCS_EVENT_LEVEL_DEBUG, "vehicle update transfer task info successed.", szEventBuf);
    }

    return iRet;
}


#if 0
#endif


/*!
 *******************************************************************************
 * 创建ecu传输实例信息
 * 
 * \note   
 *
 * \param[in]   pTransferInstId             传输受控对象实例id
 * \retval      ADM_TPS_TRANSFERINFO*       传输受控对象实例信息
 *******************************************************************************
 */
static ADM_TPS_TRANSFERINFO* __adm_tps_createEcuTransferInstanceInfo(DCHAR *pTransferInstId)
{
    ADM_TPS_TRANSFERINFO *pstTransferInstanceInfo = NULL;

    if (ADM_IS_NULL(pTransferInstId))
    {
        log_e("Invalid input param");
        return NULL;
    }

    pstTransferInstanceInfo = (ADM_TPS_TRANSFERINFO *)adm_malloc(sizeof(*pstTransferInstanceInfo));
    if (NULL != pstTransferInstanceInfo)
    {
        (void)adm_memzero(pstTransferInstanceInfo, sizeof(*pstTransferInstanceInfo));
        pstTransferInstanceInfo->transferInstanceId = adm_strdup(pTransferInstId);
    }

    return pstTransferInstanceInfo;
}


/*!
 *******************************************************************************
 * 查找ecu链表节点
 * 
 * \note   节点不摘链
 *
 * \param[in]   pstHead             传输受控实例表头(list head)
 * \param[in]   transferInstanceId  传输受控对象实例id
 * \retval      ADM_TPS_TRANSFERINFO*
 *******************************************************************************
 */
static ADM_TPS_TRANSFERINFO *__adm_tps_getEcuTransferInstanceInfo
(
    struct list_head    *pstHead, 
    DCHAR               *transferInstanceId
)
{
    struct list_head     *pstNode      = NULL;
    struct list_head     *pstNextNode  = NULL;
    ADM_TPS_TRANSFERINFO *pstTransferInstanceInfo = NULL;

    if (ADM_IS_NULL(pstHead) || ADM_IS_NULL(transferInstanceId))
    {
        log_e( "Invalid input param");
        return NULL;
    }

    list_for_each_safe(pstNode, pstNextNode, pstHead)
    {
        pstTransferInstanceInfo = list_entry(pstNode, ADM_TPS_TRANSFERINFO, stHead);

        if (adm_strcmp(pstTransferInstanceInfo->transferInstanceId, transferInstanceId) == 0)
        {
            return pstTransferInstanceInfo;
        }
    }

    return NULL;
}


/*!
 *******************************************************************************
 * 释放ecu传输受控实例信息
 * 
 * \note   
 *
 * \param[in]   pstTransferInstanceInfo 传输受控实例信息
 * \retval      none
 *******************************************************************************
 */
static void __adm_tps_freeEcuTransferInstanceInfo
(
    ADM_TPS_TRANSFERINFO *pstTransferInstanceInfo
)
{
    if (ADM_IS_NULL(pstTransferInstanceInfo))
    {
        return;
    }

    adm_free(pstTransferInstanceInfo->transferInstanceId);
    adm_free(pstTransferInstanceInfo);
    return;
}

/*!
 *******************************************************************************
 * 收集ecu传输受控对象实例信息到list
 * 
 * \note   
 *
 * \param[in]    ecuPkgListNum           传输包数量
 * \param[in]    pstEcuPkgList           传输包信息list
 * \param[inout] pstTransferInstanceList 传输受控对象实例list
 * \retval       D32S
 *******************************************************************************
 */
static D32S __adm_collectEcuPkgTransferInstancelist
(
    D32U                        ecuPkgListNum, 
    ADM_TPS_TRANSFERPKG_T      *pstEcuPkgList,
    ADM_TPS_TRANSFERLIST       *pstTransferInstanceList
)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    D32S i = 0;
    DCHAR *pTransferInstId = NULL;
    ADM_TPS_TRANSFERINFO *pstTransferInstanceInfo = NULL;

    if (ADM_IS_NULL(pstEcuPkgList) || ADM_IS_NULL(pstTransferInstanceList))
    {
        log_e( "Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }

    for (i = 0; (D32U)i < ecuPkgListNum; i++)
    {
        //>! 已经传输完成的，不需要再环境大小校验
        if (ADM_TPS_TRANSPKG_STATE_FINISH == pstEcuPkgList[i].stTransProcess.stResult.eTransState)
        {
            break;
        }
        else
        {
            pTransferInstId = adm_strdup(pstEcuPkgList[i].transferInstanceId);

            //!< find or create 下载受控对象节点
            pstTransferInstanceInfo = __adm_tps_getEcuTransferInstanceInfo(&pstTransferInstanceList->stHead, pTransferInstId);
            if (ADM_IS_NULL(pstTransferInstanceInfo))
            {
                pstTransferInstanceInfo = __adm_tps_createEcuTransferInstanceInfo(pTransferInstId);
                if (ADM_IS_NULL(pstTransferInstanceInfo))
                {
                    adm_free(pTransferInstId);
                    iRet = ADM_ECODE_NO_MEMORY;
                    log_e( "Failed to create ecu transfer ctrl obj node, err[%d]", iRet);
                    break;
                }

                pstTransferInstanceList->num++;
                //!< 添加到下载控制链表中
                list_add(&pstTransferInstanceInfo->stHead, &pstTransferInstanceList->stHead);

            }
            adm_free(pTransferInstId);
            //!> 剩余传输的大小
            pstTransferInstanceInfo->size = pstEcuPkgList[i].stTransProcess.stProgress.totalBytes - pstEcuPkgList[i].stTransProcess.stProgress.saveBytes;

        }
    }

    return iRet;
}


/*!
 *******************************************************************************
 * 删除ecu传输对象实例list
 * 
 * \note   
 *
 * \param[in]   pstTransferInstList  传输受控对象实例list
 * \retval      none
 *******************************************************************************
 */
void adm_tps_delEcuTransferInstanceList(ADM_TPS_TRANSFERLIST **pstTransferInstList)
{
    struct list_head *pstNode      = NULL;
    struct list_head *pstNextNode  = NULL;
    ADM_TPS_TRANSFERINFO *pstTransferInstInfo = NULL;

    if (ADM_IS_NULL(pstTransferInstList) || ADM_IS_NULL(*pstTransferInstList))
    {
        return;
    }

    list_for_each_safe(pstNode, pstNextNode, &(*pstTransferInstList)->stHead)
    {
        pstTransferInstInfo = list_entry(pstNode, ADM_TPS_TRANSFERINFO, stHead);

        list_del(&pstTransferInstInfo->stHead);
        __adm_tps_freeEcuTransferInstanceInfo(pstTransferInstInfo);
    }

    adm_free(*pstTransferInstList);
    return;
}


/*!
 *******************************************************************************
 * 获取ecu传输示例对象list
 * 
 *
 * \retval      ADM_TPS_TRANSFERLIST *
 *******************************************************************************
 */
ADM_TPS_TRANSFERLIST *adm_tps_getEcutransferInstanceList(void)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    ADM_TPS_TRANSFERLIST *pstTransferInstlist = NULL;

    pstTransferInstlist = (ADM_TPS_TRANSFERLIST *)adm_malloc(sizeof(*pstTransferInstlist));
    if (ADM_IS_NULL(pstTransferInstlist))
    {
        log_e( "Failed to malloc");
        return NULL;
    }
    (void)adm_memzero(pstTransferInstlist, sizeof(*pstTransferInstlist));
    INIT_LIST_HEAD(&pstTransferInstlist->stHead);

    (void)pthread_rwlock_rdlock(&gstTPSTransferInfo.stRWlock);

    iRet = __adm_collectEcuPkgTransferInstancelist(gstTPSTransferInfo.stTransferECUList.num, 
                                                   gstTPSTransferInfo.stTransferECUList.pstEcuPkg,
                                                   pstTransferInstlist);

    if (ADM_IS_ERR(iRet))
    {
        adm_tps_delEcuTransferInstanceList(&pstTransferInstlist);
        log_e( "Failed to get transfer instance list, err[%d]", iRet);
        return NULL;
    }

    (void)pthread_rwlock_unlock(&gstTPSTransferInfo.stRWlock);

    return pstTransferInstlist;
}


#if 0
#endif

/*!
 *******************************************************************************
 * 检查并更新TPS模块信息
 * 
 * \note   模块初始化完成后，M层检测到有触发任务时，需调用此接口更新下载信息
 *
 * \param[in] fnGetEcsConfInfo        获取ecs模块中该车平台的配置信息回调函数
 * \param[in] fnGetDownTaskInfo       获取dps中下载任务回调函数
 * \param[in] fnGetTransCond          获取dps中传输条件规则回调函数
 * \param[in] fnGetDownEcuPkgListInfo 获取dps中下载的ECU信息回调函数
 * \retval    D32S
 *******************************************************************************
 */
D32S adm_tps_checkAndUpdateTPSTransInfo
(
    DBOOL                                        bIsForceUpdate,
    ADM_ECS_GETCONFINFO_CALLBACK_F               fnGetEcsConfInfo,
    ADM_DPS_GEDOWNTASKINFO_CALLBACK_F            fnGetDownTaskInfo,
    ADM_DPS_GETTRANSCONDITION_CALLBACK_F         fnGetTransCond,
    ADM_DPS_GETDOWNECUPACKAGELISTINFO_CALLBACK_F fnGetDownEcuPkgListInfo
)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    ADM_DPS_DOWNTSAKINFO_T stDownTaskInfo = {0};
    DCHAR szEventBuf[ADM_COM_LEN_256] = {0}; 

    if (ADM_IS_NULL(fnGetEcsConfInfo) || ADM_IS_NULL(fnGetDownTaskInfo) ||
        ADM_IS_NULL(fnGetTransCond)   || ADM_IS_NULL(fnGetDownEcuPkgListInfo))
    {
        log_e("Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }

    if (FALSE == gstTPSTransferInfo.isInited)
    {
        log_e("TPS module not init, err[%d]", ADM_ECODE_MODULE_UNINIT);
        return ADM_ECODE_MODULE_UNINIT;
    }

    do 
    {
        (void)adm_memzero(szEventBuf, sizeof(szEventBuf));
        (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), 
                        "cache transfer task info: sessionId[%s], taskId[%lld], strategyId[%lld].", 
                        ADM_PRINT_STR(gstTPSTransferInfo.stDownTaskInfo.sessionId),
                        gstTPSTransferInfo.stDownTaskInfo.taskId,
                        gstTPSTransferInfo.stDownTaskInfo.strategyId);
        adm_tps_eventReport(ADM_COM_MOD_TPS, FALSE, ADM_EVENT_ID_TP_001, ADM_OCS_EVENT_LEVEL_DEBUG, "vehicle check and update transfer task info.", szEventBuf);
    
        //!< 获取下载任务信息
        (void)adm_memzero(&stDownTaskInfo, sizeof(stDownTaskInfo));
        iRet = fnGetDownTaskInfo(&stDownTaskInfo);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to get version policy, err[%d]", iRet);
            (void)adm_memzero(szEventBuf, sizeof(szEventBuf));
            (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), "Failed to get version policy, err[%d].", iRet);
            break;
        }
        
        (void)adm_memzero(szEventBuf, sizeof(szEventBuf));
        (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), 
                        "new transfer task info: sessionId[%s], taskId[%lld], strategyId[%lld].", 
                        ADM_PRINT_STR(stDownTaskInfo.sessionId),
                        stDownTaskInfo.taskId,
                        stDownTaskInfo.strategyId);
        adm_tps_eventReport(ADM_COM_MOD_TPS, FALSE, ADM_EVENT_ID_TP_004, ADM_OCS_EVENT_LEVEL_DEBUG, "vehicle get transfer task info successed.", szEventBuf);

        if (ADM_IS_NULL(stDownTaskInfo.sessionId))
        {
            iRet = ADM_ECODE_INVALID_PARAM;
            log_e("Get download policy data error");
            (void)adm_memzero(szEventBuf, sizeof(szEventBuf));
            (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), "Get download policy data error, sessionId is null!");
            break;
        }

        if (DFALSE == bIsForceUpdate)
        {
            //!< 检测传输信息是否需要更新 
            (void)pthread_rwlock_wrlock(&gstTPSTransferInfo.stRWlock);
            if ((NULL != gstTPSTransferInfo.stDownTaskInfo.sessionId)
                &&(0 == adm_strcasecmp(gstTPSTransferInfo.stDownTaskInfo.sessionId, stDownTaskInfo.sessionId)) 
                && (gstTPSTransferInfo.stDownTaskInfo.taskId == stDownTaskInfo.taskId)
                && (gstTPSTransferInfo.stDownTaskInfo.strategyId == stDownTaskInfo.strategyId))
            {
                (void)pthread_rwlock_unlock(&gstTPSTransferInfo.stRWlock);
                (void)adm_memzero(szEventBuf, sizeof(szEventBuf));
                (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), "transfer policy dont need to update.");
                //!< 检查下载信息未更新，则可直接退出
                break;
            }
        }
        (void)adm_memzero(szEventBuf, sizeof(szEventBuf));
        (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), "transfer policy need update.");
        adm_tps_eventReport(ADM_COM_MOD_TPS, FALSE, ADM_EVENT_ID_TP_006, ADM_OCS_EVENT_LEVEL_DEBUG, "vehicle cmp transfer task judge is need update task successed.", szEventBuf);

        //!< 检测下载信息有更新，则需要更新待传输ecu信息
        iRet = adm_tps_updateTransCacheInfo(&stDownTaskInfo, 
                    fnGetEcsConfInfo, fnGetTransCond, fnGetDownEcuPkgListInfo);
        if (ADM_IS_ERR(iRet))
        {
            (void)pthread_rwlock_unlock(&gstTPSTransferInfo.stRWlock);
            log_e("Failed to update trans policy, err[%d]", iRet);
            (void)adm_memzero(szEventBuf, sizeof(szEventBuf));
            (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), "Failed to update trans policy, err[%d]", iRet);
            break;
        }

        //!< 更新分包策略到 kv 
        iRet = adm_tps_setTransferInfo2KV(&gstTPSTransferInfo);
        if (ADM_IS_ERR(iRet))
        {
            // TODO: 存储失败暂时不返回 
            log_e("Failed to set transfer policy Info 2 KV, err[%d]", iRet);
        }
        log_d("Set transfer policy Info 2 KV ok.");
        
        (void)pthread_rwlock_unlock(&gstTPSTransferInfo.stRWlock);
    } while (FALSE);

    adm_dps_cleanDownTaskInfo(&stDownTaskInfo);

    if (ADM_IS_ERR(iRet))
    {
        adm_tps_eventReport(ADM_COM_MOD_TPS, TRUE, ADM_EVENT_ID_TP_002, ADM_OCS_EVENT_LEVEL_ERROR, "vehicle check and update transfer task info failed!", szEventBuf);
    }
    else
    {
        adm_tps_eventReport(ADM_COM_MOD_TPS, FALSE, ADM_EVENT_ID_TP_003, ADM_OCS_EVENT_LEVEL_DEBUG, "vehicle check and update transfer task info successed.", szEventBuf);
    }

    return iRet;
}

D32S adm_tps_setIdelControlInfo(ADM_TPS_TRNASFER_IDEL_CONTROL_T *pstTransIdelControlInfo)
{
    return adm_tps_setIdelControlInfo2KV(pstTransIdelControlInfo);
}

/*!
 *******************************************************************************
 * 模块初始化.
 *
 * \note
 *
 * \param[in] fnCmpIsNeedTransCb 比较是否需要分包回调函数
 * \param[in] fnReportEventCb    注册事件上报回调函数
 * \param[in] pstIdelControl     闲时传输信息
 * \retval    D32S               0: 成功 non-0：失败
 *******************************************************************************
 */
D32S adm_tps_moduleInit(
    ADM_TPS_CMPISNEEDTRANS_CALLBACK_F fnCmpIsNeedTransCb,
    ADM_TPS_REPORTEVENT_CALLBACK_F fnReportEventCb,
    ADM_TPS_TRNASFER_IDEL_CONTROL_T *pstIdelControl)
{
    D32S iRet = ADM_ECODE_FAILED;
    D32S errNum = 0;

    if (ADM_IS_NULL(fnCmpIsNeedTransCb) || ADM_IS_NULL(pstIdelControl))
    {
        log_e("Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }

    if (FALSE == gstTPSTransferInfo.isInited)
    {
        (void)adm_memzero(&gstTPSTransferInfo, sizeof(gstTPSTransferInfo));
        iRet = pthread_rwlock_init(&gstTPSTransferInfo.stRWlock, NULL);
        if (ADM_IS_ERR(iRet))
        {
            errNum = errno;
            log_e("Failed to init rwlock, err[%d](%s)", iRet, strerror(errNum));
            return ADM_ECODE_RWLOCK_INIT_FAILED;
        }
        (void)adm_memzero(&gstTPSTransferInfo.stDownTaskInfo,      sizeof(gstTPSTransferInfo.stDownTaskInfo));
        (void)adm_memzero(&gstTPSTransferInfo.stTransferECUList,   sizeof(gstTPSTransferInfo.stTransferECUList));
        (void)adm_memzero(&gstTPSTransferInfo.stTransferCondition, sizeof(gstTPSTransferInfo.stTransferCondition));

        //!< register callback 
        gstTPSTransferInfo.fnCmpIsNeedTransApi = fnCmpIsNeedTransCb;
        gstTPSTransferInfo.fnReportEventApi    = fnReportEventCb;

        //!< 尝试从kv中获取缓存
        iRet = adm_tps_getnTransferInfoFromKV(&gstTPSTransferInfo);
        if (ADM_IS_ERR(iRet))
        {
            log_i("First try read transfer info from kv, err[%d]", iRet);
        }

        //!< 尝试从kv中获取缓存
        iRet = adm_tps_getIdelControlInfoFromKV(pstIdelControl);
        if (ADM_IS_ERR(iRet))
        {
            pstIdelControl->eIdelControl = IDEL_TRANSFER_INVALID;
            log_i("First try read idel control info from kv, err[%d]", iRet);
        }

        gstTPSTransferInfo.isInited = TRUE;
    }
    log_d("===> tps init module ok.");

    return ADM_ECODE_SUCCESSED;
}


/*!
 *******************************************************************************
 * 模块销毁.
 *
 * \note 
 *
 * \param   none
 * \retval  none
 *******************************************************************************
 */
void adm_tps_moduleDeInit(void)
{
    gstTPSTransferInfo.isInited = FALSE;

    (void)pthread_rwlock_wrlock(&gstTPSTransferInfo.stRWlock);
    adm_tps_cleanTransferCacheInfo(&gstTPSTransferInfo);
    (void)pthread_rwlock_unlock(&gstTPSTransferInfo.stRWlock);
    (void)pthread_rwlock_destroy(&gstTPSTransferInfo.stRWlock);

    gstTPSTransferInfo.fnReportEventApi    = NULL;
    gstTPSTransferInfo.fnCmpIsNeedTransApi = NULL;

    return;
}

