/**
 * @file    adm_dpm_downloadManagement.c
 * @brief   Download-Policy Management 
 * @details 下载策略管理
 * @author  wangyanpeng
 * @version v0.1.0
 * @date    2020-11-06
 * @copyright www.abupdate.com
 */

#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>

#include "adm_ocm_interface.h"
#include "adm_file.h"
#include "adm_dpm_log.h"
#include "adm_dpm_downloadManagement.h"
#include "adm_ecs_rsConfig.h"
#include "adm_cvs_versionPolicy.h"
#include "adm_cvs_checkVersion.h"
#include "adm_dpm_download.h"
#include "adm_dpm_report.h"
#include "adm_dpm_reportCloud.h"
#include "adm_dpm_uim.h"
#include "adm_com_lib.h"
#include "adm_irm_report.h"
#include "adm_cdm_verify_interface.h"
#include "adm_ecm_management.h"
#include "adm_pub_func.h"
#include "adm_dim_host.h"
#include "adm_dim_ucl.h"
#include "adm_cfg_api.h"
#include "adm_soa.h"
#include "adm_sdk_version.h"
#include "adm_application_errorCode.h"
#include "adm_ipm_install.h"
//!< tpm 依赖 ewebserver 的支持 
#ifdef CMAKE_CUSTOM_EWEBSERVER_SUPPORT
    #include "adm_tpm_transferManagement.h"
#endif


#define ADM_DPM_DEFDOWN_GETTOBELIST_CNT (3)
#define ADM_DPM_DEFDOWN_CYCLE_CNT (5)
#define ADM_DPM_SET_WIFI_TIMEOUT 300


static ADM_DPM_DOWNLOADINFO_T gstDPMdownInfo = {0};

//!< debug 下载策略文件  --- proj/build/output/admvus/etc/dpm_data/
static const DCHAR *gpcDownCombinePolicyJsonPath = "../etc/dpm_data/getDownCombinePolicy.json";   
static const DCHAR *gpcDownPartPolicyJsonPath    = "../etc/dpm_data/getDownPartPolicy.json"; 

static ADM_DPM_FUNC_CALL_BACK_T g_stFuncCallbackInfo = {0};

#if 0
#endif

ADM_DPM_DOWNLOADINFO_T *adm_dpm_getDownInfo(void)
{
    return &gstDPMdownInfo;
}

/*!
 *******************************************************************************
 * 判断是否是下载暂停错误
 *
 * \note 
 * 
 * \param[in]   iError 
 * \retval      D32S
 *******************************************************************************
 */
static D32S dpmIsDownloadPause(D32S iError)
{
    D32S iRet = ADM_ECODE_SUCCESSED;

    switch (iError)
    {
        case ADM_ECODE_NET_TYPE_VERIFY_FAILED:
        case ADM_ECODE_CONDITION_VERIFY_FAILED:
            iRet = ADM_ECODE_PAUSE_DOWNLOAD;
            break;
        default:
            iRet = iError;
            break;
    }

    return iRet;
}

/*!
 *******************************************************************************
 * 检查ecu的零件升级包已经下载的大小
 *
 * \note 
 * 
 * \param[in]  pstEcuPkgInfo  ecu零件包信息
 * \retval     D64S           文件已下载大小
 *******************************************************************************
 */
static D64S __adm_dpm_checkPartPackageDownSize
(
    ADM_DPS_ECUPARTPKGINFO_T *pstEcuPkgInfo
)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    D32S errNum = 0;
    struct stat fileStat = {0};

    if (ADM_IS_NULL(pstEcuPkgInfo))
    {
        return 0;
    }
    
    iRet = stat(pstEcuPkgInfo->stPkgFileObj.filePath, &fileStat);
    if (ADM_IS_ERR(iRet))
    {
        errNum = errno;
        log_e("Failed to stat file info, filePath[%s], err[%d], strerror[%s]", 
                (NULL != pstEcuPkgInfo->stPkgFileObj.filePath) ? (pstEcuPkgInfo->stPkgFileObj.filePath) : ("null"), 
                iRet, strerror(errNum));
        return 0;
    }

    //!< 若实测文件包大小 > 文件包的理论大小，则返回 0
    if (fileStat.st_size > pstEcuPkgInfo->stPkgFileObj.size)
    {
        log_i("check File really size[%lld] gt assume size[%lld], filePath[%s]", 
            (D64S)fileStat.st_size, pstEcuPkgInfo->stPkgFileObj.size, 
            pstEcuPkgInfo->stPkgFileObj.filePath);
        return 0;
    }

    return (D64S)fileStat.st_size;
}

/*!
 *******************************************************************************
 * 填充dpm下载请求参数通过part包信息
 *
 * \note
 * 
 * \param[in]   enPkgType          包类型[src/dst]
 * \param[in]   pstDownPartPkgInfo part包信息
 * \param[out]  pstDpmDownReqInfo  dpm下载请求参数
 * \retval      ADM_DPO_DOWNLOAD_REQPARAM_T*
 *******************************************************************************
 */
static D32S __adm_dpm_fillPartPkgDPMDownReqParam
(
    ADM_DPS_PARTPKGTYPE_E      enPkgType,
    ADM_DPS_DOWN_ECUPARTPKG_T *pstDownPartPkgInfo,
    ADM_DPM_DOWNREQPARAM_T    *pstDpmDownReqInfo
)
{
    if (ADM_IS_NULL(pstDownPartPkgInfo) || ADM_IS_NULL(pstDpmDownReqInfo))
    {
        log_e("Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }

    //!< 填充dpm下载包参数 --- 目标包优先，增加if命中率
    if (ADM_PARTPKG_TYPE_DST == enPkgType)
    {
        pstDpmDownReqInfo->stPkgAttr.isRollback = FALSE;
        pstDpmDownReqInfo->fileID = adm_dps_createDownFileIDByEcuInfo(
                                        pstDownPartPkgInfo->ecuReqid, 
                                        pstDownPartPkgInfo->ecuSwDid, 
                                        pstDpmDownReqInfo->stPkgAttr.isRollback,
                                        pstDownPartPkgInfo->stDstEcuPkgInfo.stPkgFileObj.fileName);
        if (ADM_IS_NULL(pstDpmDownReqInfo->fileID))
        {
            log_e("Failed to create down file id, err[%d]", ADM_ECODE_NO_MEMORY);
            return ADM_ECODE_NO_MEMORY;
        }

        pstDpmDownReqInfo->url = adm_strdup(pstDownPartPkgInfo->stDstEcuPkgInfo.stPkgFileObj.url);
        pstDpmDownReqInfo->hash = adm_strdup(pstDownPartPkgInfo->stDstEcuPkgInfo.stPkgFileObj.hash); 
        pstDpmDownReqInfo->filePath = adm_strdup(pstDownPartPkgInfo->stDstEcuPkgInfo.stPkgFileObj.filePath);
        pstDpmDownReqInfo->size = pstDownPartPkgInfo->stDstEcuPkgInfo.stPkgFileObj.size;
        pstDpmDownReqInfo->stPkgAttr.fileName = adm_strdup(pstDownPartPkgInfo->stDstEcuPkgInfo.stPkgFileObj.fileName);

        //!< 注释部分虽然可以不区分 srcPkg/dstPkg 顺序下载，但对slave下载无法获取已下载的大小，
        //!< 这个简单点约定先下载 srcPkg, 再下载 dstPkg 包
        //!< 叠加另一个包的大小，便于 srcPkg/dstPkg 进度融合 --- 下目标包则说明源包已经下完或者没有源包
        //pstDpmDownReqInfo->stPkgAttr.baseSize = __adm_dpm_checkPartPackageDownSize(pstDownPartPkgInfo->pstSrcEcuPkgInfo);
        pstDpmDownReqInfo->stPkgAttr.baseSize = 
                ((NULL != pstDownPartPkgInfo->pstSrcEcuPkgInfo) ? (pstDownPartPkgInfo->pstSrcEcuPkgInfo->stPkgFileObj.size) : (0));
    }
    else
    {
        if (ADM_IS_NULL(pstDownPartPkgInfo->pstSrcEcuPkgInfo))
        {
            log_e("Invalid package type, no find srcPakage info, err[%d]", ADM_ECODE_INVALID_PARAM);
            return ADM_ECODE_INVALID_PARAM;
        }

        pstDpmDownReqInfo->stPkgAttr.isRollback = TRUE;
        pstDpmDownReqInfo->fileID = adm_dps_createDownFileIDByEcuInfo(
                                        pstDownPartPkgInfo->ecuReqid, 
                                        pstDownPartPkgInfo->ecuSwDid, 
                                        pstDpmDownReqInfo->stPkgAttr.isRollback,
                                        pstDownPartPkgInfo->pstSrcEcuPkgInfo->stPkgFileObj.fileName);
        if (ADM_IS_NULL(pstDpmDownReqInfo->fileID))
        {
            log_e("Failed to create down file id, err[%d]", ADM_ECODE_NO_MEMORY);
            return ADM_ECODE_NO_MEMORY;
        }

        pstDpmDownReqInfo->url = adm_strdup(pstDownPartPkgInfo->pstSrcEcuPkgInfo->stPkgFileObj.url);
        pstDpmDownReqInfo->hash = adm_strdup(pstDownPartPkgInfo->pstSrcEcuPkgInfo->stPkgFileObj.hash);
        pstDpmDownReqInfo->filePath = adm_strdup(pstDownPartPkgInfo->pstSrcEcuPkgInfo->stPkgFileObj.filePath);
        pstDpmDownReqInfo->size = pstDownPartPkgInfo->pstSrcEcuPkgInfo->stPkgFileObj.size;
        pstDpmDownReqInfo->stPkgAttr.fileName = adm_strdup(pstDownPartPkgInfo->pstSrcEcuPkgInfo->stPkgFileObj.fileName);

        //!< 叠加另一个包的大小，便于 srcPkg/dstPkg 进度融合 --- 下源包则说明目标包没开始下
        //pstDpmDownReqInfo->stPkgAttr.baseSize = __adm_dpm_checkPartPackageDownSize(&pstDownPartPkgInfo->stDstEcuPkgInfo);
        pstDpmDownReqInfo->stPkgAttr.baseSize = 0;
    }

    pstDpmDownReqInfo->stPkgAttr.enPkgFormat = ADM_DPS_DOWNPACKAGE_PART;
    pstDpmDownReqInfo->stPkgAttr.ecuReqid = adm_strdup(pstDownPartPkgInfo->ecuReqid);
    pstDpmDownReqInfo->stPkgAttr.ecuSwDid = adm_strdup(pstDownPartPkgInfo->ecuSwDid); 
    pstDpmDownReqInfo->stPkgAttr.wholePkgSize = pstDownPartPkgInfo->stDstEcuPkgInfo.stPkgFileObj.size;
    if (NULL != pstDownPartPkgInfo->pstSrcEcuPkgInfo)
    {
        pstDpmDownReqInfo->stPkgAttr.wholePkgSize += pstDownPartPkgInfo->pstSrcEcuPkgInfo->stPkgFileObj.size;
    }
    log_d("Adapter fileID[%s]", pstDpmDownReqInfo->fileID);

    return ADM_ECODE_SUCCESSED;
}


/*!
 *******************************************************************************
 * 填充dpm下载请求参数通过combine包信息
 *
 * \note
 * 
 * \param[in]   pstDownCombinePkg combine包信息
 * \param[out]  pstDpmDownReqInfo  dpm下载请求参数
 * \retval      ADM_DPO_DOWNLOAD_REQPARAM_T*
 *******************************************************************************
 */
static D32S __adm_dpm_fillCombinePkgDPMDownReqParam
(
    ADM_DPS_DOWN_ECUCOMBINEPKG_T *pstDownCombinePkg,
    ADM_DPM_DOWNREQPARAM_T       *pstDpmDownReqInfo
)
{
    if (ADM_IS_NULL(pstDownCombinePkg) || ADM_IS_NULL(pstDpmDownReqInfo))
    {
        log_e("Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }

    pstDpmDownReqInfo->stPkgAttr.isRollback = FALSE;

    //!< 填充dpm下载包参数
    pstDpmDownReqInfo->fileID = adm_dps_createDownFileIDByEcuInfo(NULL, NULL,
                                    pstDpmDownReqInfo->stPkgAttr.isRollback,
                                    pstDownCombinePkg->stPkgFileObj.fileName);
    if (ADM_IS_NULL(pstDpmDownReqInfo->fileID))
    {
        log_e("Failed to create down file id, err[%d]", ADM_ECODE_NO_MEMORY);
        return ADM_ECODE_NO_MEMORY;
    }

    pstDpmDownReqInfo->url = adm_strdup(pstDownCombinePkg->stPkgFileObj.url);
    pstDpmDownReqInfo->hash = adm_strdup(pstDownCombinePkg->stPkgFileObj.hash);
    pstDpmDownReqInfo->filePath = adm_strdup(pstDownCombinePkg->stPkgFileObj.filePath);
    pstDpmDownReqInfo->size = pstDownCombinePkg->stPkgFileObj.size;

    //!< 集合包时，零件包的属性不用填
    pstDpmDownReqInfo->stPkgAttr.ecuReqid = NULL;
    pstDpmDownReqInfo->stPkgAttr.ecuSwDid = NULL;
    pstDpmDownReqInfo->stPkgAttr.baseSize = 0;
    pstDpmDownReqInfo->stPkgAttr.enPkgFormat = ADM_DPS_DOWNPACKAGE_COMBINE;
    pstDpmDownReqInfo->stPkgAttr.fileName = adm_strdup(pstDownCombinePkg->stPkgFileObj.fileName);
    pstDpmDownReqInfo->stPkgAttr.wholePkgSize = pstDownCombinePkg->stPkgFileObj.size;

    return ADM_ECODE_SUCCESSED;
}

#if 0
#endif

/*!
 *******************************************************************************
 * 创建dpm下载请求参数结构
 *
 * \note        对应释放函数 __adm_dpm_freeDPMDownReqParam
 *  
 * \param[in]   downRetriesCnt  下载失败重试次数
 * \param[in]   isResumeDown    是否从断点处继续下载 
 * \retval      ADM_DPM_DOWNREQPARAM_T*
 *******************************************************************************
 */
static ADM_DPM_DOWNREQPARAM_T* __adm_dpm_createDPMDownReqParam
(
    D32S   downRetriesCnt,
    DBOOL  isResumeDown,
    ADM_DPM_RESULT_NOTIFY_CALLBACK_F   fnDownResultNotify,
    ADM_DPM_PROGRESS_NOTIFY_CALLBACK_F fnDownProgressNotify
)
{

    ADM_DPM_DOWNREQPARAM_T *pstDpmDownReqParam = NULL;

    pstDpmDownReqParam = (ADM_DPM_DOWNREQPARAM_T *)adm_malloc(sizeof(*pstDpmDownReqParam));
    if (ADM_IS_NULL(pstDpmDownReqParam))
    {
        log_e("Failed to malloc download param");
        return NULL;
    }
    (void)adm_memzero(pstDpmDownReqParam, sizeof(*pstDpmDownReqParam));

    pstDpmDownReqParam->downRetriesCnt = downRetriesCnt;
    pstDpmDownReqParam->isResumeDown   = isResumeDown;
    pstDpmDownReqParam->isUnlimitretry = TRUE;
    pstDpmDownReqParam->networkRetriesCnt = 0;
    pstDpmDownReqParam->fnDownResultNotify   = fnDownResultNotify;
    pstDpmDownReqParam->fnDownProgressNotify = fnDownProgressNotify;

    if (ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_PROD_LINE == gstDPMdownInfo.eTriggerType)
    {
        //产线升级，网络原因导致的失败，默认重试10次
        pstDpmDownReqParam->networkRetriesCnt = 10;
    }

    return pstDpmDownReqParam;
}

/*!
 *******************************************************************************
 * 清空dpm下载请求参数结构
 *
 * \note  不释放最外侧 pstDpmDownReqInfo 内存
 * 
 * \param[in]   pstDpmDownReqParam  下载包参数
 * \retval      none
 *******************************************************************************
 */
static void __adm_dpm_cleanDPMDownReqParam
(
    ADM_DPM_DOWNREQPARAM_T *pstDpmDownReqParam
)
{
    if (ADM_IS_NULL(pstDpmDownReqParam))
    {
        return;
    }

    adm_dps_freeDownFileID(&pstDpmDownReqParam->fileID);
    adm_free(pstDpmDownReqParam->url);
    adm_free(pstDpmDownReqParam->hash);
    adm_free(pstDpmDownReqParam->filePath);
    pstDpmDownReqParam->size = 0;

    adm_dps_cleanPackageAttribute(&pstDpmDownReqParam->stPkgAttr);

    return;
}


/*!
 *******************************************************************************
 * 释放dpm下载请求参数结构
 *
 * \note        对应创建函数 __adm_dpm_createDPMDownReqParam
 * 
 * \param[in]   pstDpmDownReqParam  下载包参数
 * \retval      none
 *******************************************************************************
 */
static void __adm_dpm_delDPMDownReqParam
(
    ADM_DPM_DOWNREQPARAM_T **pstDpmDownReqParam
)
{
    if (ADM_IS_NULL(pstDpmDownReqParam) || ADM_IS_NULL(*pstDpmDownReqParam))
    {
        return;
    }

    adm_free((*pstDpmDownReqParam)->fileID);
    adm_free((*pstDpmDownReqParam)->url);
    adm_free((*pstDpmDownReqParam)->hash);
    adm_free((*pstDpmDownReqParam)->filePath);
    adm_free((*pstDpmDownReqParam)->stPkgAttr.ecuReqid);
    adm_free((*pstDpmDownReqParam)->stPkgAttr.ecuSwDid);
    adm_free((*pstDpmDownReqParam)->stPkgAttr.fileName);
    adm_free(*pstDpmDownReqParam);

    return;
}



/*!
 *******************************************************************************
 * 更新集合包中各ecu的存储路径到pis模块
 *
 * \note 
 * 
 * \param[in]   pstEcuIdList 集合包中ecu id列表 
 * \retval      D32S
 *******************************************************************************
 */
static D32S __adm_dpm_updateCombinePkgAllEcuSavePath2Pis
(
    ADM_DPS_ECUIDLIST_T *pstEcuIdList
)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    D32S i = 0;
    DCHAR szEventBuf[ADM_COM_LEN_256] = {0};  //mod by wl 20220321

    //mod by wl 20220321
    adm_dpm_addEventAndReport(ADM_COM_MOD_DPM, FALSE, ADM_EVENT_ID_DI_040, ADM_OCS_EVENT_LEVEL_TRACE, "vehicle update combine package all ecu save path 2 pis.", NULL);
    
    if (ADM_IS_NULL(pstEcuIdList))
    {
        log_e("Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }

    //!< 4 更新集合包中各ecu升级包文件存储路径到pis
    for (i = 0; i < pstEcuIdList->num; i++)
    {
        //!< 若存在源包，则先更新源包位置
        if (NULL != pstEcuIdList->pstEcuIdInfo[i].psrcPkgFile)
        {
            log_d("Download package calc write src[%s][%s] path[%s] to pis.", 
                    pstEcuIdList->pstEcuIdInfo[i].ecuReqid, 
                    pstEcuIdList->pstEcuIdInfo[i].ecuSwDid, 
                    pstEcuIdList->pstEcuIdInfo[i].psrcPkgFile->filePath);
            iRet = adm_dps_updateEcuPkgSavePath2PIS(pstEcuIdList->pstEcuIdInfo[i].ecuReqid, 
                                                    pstEcuIdList->pstEcuIdInfo[i].ecuSwDid, 
                                                    TRUE,
                                                    pstEcuIdList->pstEcuIdInfo[i].psrcPkgFile);
            if (ADM_IS_ERR(iRet))
            {
                log_e("Failed to update combine package ecu src package save path 2 pis, ecuReqid[%s], ecuSwDid[%s], err[%d]",
                       pstEcuIdList->pstEcuIdInfo[i].ecuReqid, pstEcuIdList->pstEcuIdInfo[i].ecuSwDid, iRet);
                //mod by wl 20220321
                (void)adm_memzero(szEventBuf, sizeof(szEventBuf));
                (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), 
                        "Failed to update combine package ecu src package save path 2 pis, ecuReqid[%s], ecuSwDid[%s], err[%d]", 
                        pstEcuIdList->pstEcuIdInfo[i].ecuReqid, pstEcuIdList->pstEcuIdInfo[i].ecuSwDid, iRet);
                break;
            }
            log_d("Download package write src[%s][%s] path[%s] to pis ok.", 
                    pstEcuIdList->pstEcuIdInfo[i].ecuReqid, 
                    pstEcuIdList->pstEcuIdInfo[i].ecuSwDid, 
                    pstEcuIdList->pstEcuIdInfo[i].psrcPkgFile->filePath);
        }

        log_d("Download package try write dst[%s][%s] path[%s] to pis.", 
                    pstEcuIdList->pstEcuIdInfo[i].ecuReqid, 
                    pstEcuIdList->pstEcuIdInfo[i].ecuSwDid, 
                    pstEcuIdList->pstEcuIdInfo[i].dstPkgFile.filePath);
        iRet = adm_dps_updateEcuPkgSavePath2PIS(pstEcuIdList->pstEcuIdInfo[i].ecuReqid, 
                                                pstEcuIdList->pstEcuIdInfo[i].ecuSwDid, 
                                                FALSE,
                                                &pstEcuIdList->pstEcuIdInfo[i].dstPkgFile);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to update combine package ecu dst package save path 2 pis, ecuReqid[%s], ecuSwDid[%s], err[%d]",
                   pstEcuIdList->pstEcuIdInfo[i].ecuReqid, pstEcuIdList->pstEcuIdInfo[i].ecuSwDid, iRet);
            //mod by wl 20220321
            (void)adm_memzero(szEventBuf, sizeof(szEventBuf));
            (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), 
                    "Failed to update combine package ecu dst package save path 2 pis, ecuReqid[%s], ecuSwDid[%s], err[%d]", 
                    pstEcuIdList->pstEcuIdInfo[i].ecuReqid, pstEcuIdList->pstEcuIdInfo[i].ecuSwDid, iRet);
            break;
        }
        log_d("Download package write dst[%s][%s] path[%s] to pis ok.", 
                    pstEcuIdList->pstEcuIdInfo[i].ecuReqid, 
                    pstEcuIdList->pstEcuIdInfo[i].ecuSwDid, 
                    pstEcuIdList->pstEcuIdInfo[i].dstPkgFile.filePath);

    }

    //mod by wl 20220321
    if (ADM_IS_ERR(iRet))
    {
        adm_dpm_addEventAndReport(ADM_COM_MOD_DPM, TRUE, ADM_EVENT_ID_DI_041, ADM_OCS_EVENT_LEVEL_ERROR, 
                "vehicle update combine package all ecu save path 2 pis failed!", szEventBuf);
    }
    else
    {
        adm_dpm_addEventAndReport(ADM_COM_MOD_DPM, FALSE, ADM_EVENT_ID_DI_042, ADM_OCS_EVENT_LEVEL_DEBUG, 
                "vehicle update combine package all ecu save path 2 pis successed.", NULL);
    }

    return iRet;
}

/*!
 *******************************************************************************
 * 检查集合包中packageConfig.json对应的ecu升级包与check接口返回的数量是否符合
 *
 * \note
 * 
 * \param[in]   pstEcuIdList  ecu id列表 
 * \param[in]   pstPkgCfgInfo 集合包配置文件信息
 * \retval      D32S
 *******************************************************************************
 */
static D32S __adm_dpm_checkCombinePackageCfgEcuFileInfo
(
    ADM_DPS_ECUIDLIST_T  *pstEcuIdList,
    ADM_DPS_PACKAGECFG_T *pstPkgCfgInfo
)
{
    D32S i = 0;
    D32S j = 0;
    DCHAR szEventBuf[ADM_COM_LEN_256] = {0};  //mod by wl 20220321

    //mod by wl 20220321
    adm_dpm_addEventAndReport(ADM_COM_MOD_DPM, FALSE, ADM_EVENT_ID_DI_034, ADM_OCS_EVENT_LEVEL_TRACE, 
                    "vehicle check combine package config ecu is invalid.", NULL);

    if (ADM_IS_NULL(pstEcuIdList) || ADM_IS_NULL(pstPkgCfgInfo))
    {
        log_e("Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }

    for (i = 0; i < pstEcuIdList->num; i++)
    {
        //!< 若存在源包，则检测配置文件中是否有源包文件
        if (NULL != pstEcuIdList->pstEcuIdInfo[i].psrcPkgFile)
        {
            if(NULL != pstEcuIdList->pstEcuIdInfo[i].psrcPkgFile->fileName)
            {
                for (j = 0; j < pstPkgCfgInfo->stPackageList.num; j++)
                {
                    if (0 == adm_strcasecmp(pstEcuIdList->pstEcuIdInfo[i].psrcPkgFile->fileName, pstPkgCfgInfo->stPackageList.pstPkgInfo[j].fileName))
                    {
                        break;
                    }
                }

                //!< 未找到，则可以直接错误退出
                if (j == pstPkgCfgInfo->stPackageList.num)
                {
                    //mod by wl 20220321
                    log_e("Failed to find ecu src package file[%s] from combine package config.json,err[%d]", 
                            pstEcuIdList->pstEcuIdInfo[i].psrcPkgFile->fileName, ADM_ECODE_DOWN_CHECKPKGCFG_FAILED);
                    (void)adm_memzero(szEventBuf, sizeof(szEventBuf));
                    (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), 
                            "Failed to find ecu src package file[%s] from combine package config.json, err[%d]", 
                            pstEcuIdList->pstEcuIdInfo[i].psrcPkgFile->fileName, ADM_ECODE_DOWN_CHECKPKGCFG_FAILED);
                    adm_dpm_addEventAndReport(ADM_COM_MOD_DPM, TRUE, ADM_EVENT_ID_DI_035, ADM_OCS_EVENT_LEVEL_ERROR, 
                            "vehicle check combine package config ecu is invalid failed!", szEventBuf);

                    return ADM_ECODE_DOWN_CHECKPKGCFG_FAILED;
                }
            }
        }

        //!< 若存在目标包，则检测配置文件中是否有目标包文件
        if (NULL != pstEcuIdList->pstEcuIdInfo[i].dstPkgFile.fileName)
        {
            for (j = 0; j < pstPkgCfgInfo->stPackageList.num; j++)
            {
                if (0 == adm_strcasecmp(pstEcuIdList->pstEcuIdInfo[i].dstPkgFile.fileName, pstPkgCfgInfo->stPackageList.pstPkgInfo[j].fileName))
                {
                    break;
                }
            }

            //!< 未找到，则可以直接错误退出
            if (j == pstPkgCfgInfo->stPackageList.num)
            {
                //mod by wl 20220321
                log_e("Failed to find ecu dst package file[%s] from combine package config.json, err[%d]", 
                        pstEcuIdList->pstEcuIdInfo[i].dstPkgFile.fileName, ADM_ECODE_DOWN_CHECKPKGCFG_FAILED);
                (void)adm_memzero(szEventBuf, sizeof(szEventBuf));
                (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), 
                        "Failed to find ecu dst package file[%s] from combine package config.json, err[%d]", 
                        pstEcuIdList->pstEcuIdInfo[i].dstPkgFile.fileName, ADM_ECODE_DOWN_CHECKPKGCFG_FAILED);
                adm_dpm_addEventAndReport(ADM_COM_MOD_DPM, TRUE, ADM_EVENT_ID_DI_035, ADM_OCS_EVENT_LEVEL_ERROR, 
                        "vehicle check combine package config ecu is invalid failed!", szEventBuf);

                return ADM_ECODE_DOWN_CHECKPKGCFG_FAILED;
            }
        }
    }
    //mod by wl 20220321
    adm_dpm_addEventAndReport(ADM_COM_MOD_DPM, FALSE, ADM_EVENT_ID_DI_036, ADM_OCS_EVENT_LEVEL_DEBUG, 
                        "vehicle check combine package config ecu is invalid successed.", NULL);

    return ADM_ECODE_SUCCESSED;
}


/*!
 *******************************************************************************
 * 下载过程监控到异常信息通知
 *
 * \note
 *          
 * \param[in]   pStCondResults  下载监控条件列表
 * \param[in]   pbIsConform     是否异常
 * \retval      D32S
 *******************************************************************************
 */
static void *dpmDownMonitorNotify
(
    ADM_UIM_SOA_CONDITION_DATA_ARRAY_T *pStCondResults,
    DBOOL *pbIsConform
)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    D32S ierrorCode = ADM_ECODE_SUCCESSED;

    if (ADM_IS_NULL(pStCondResults) || ADM_IS_NULL(pbIsConform))
    {
        log_e("Invalid input param");
        return NULL;
    }

    //通知结果如果是成功则不需要暂停下载
    if(FALSE == (*pbIsConform))
    {
        log_i("====>Down cond verify normal notify");
        return NULL;
    }

    //mod by wl 20220321
    adm_dpm_addEventAndReport(ADM_COM_MOD_DPM, FALSE, ADM_EVENT_ID_DI_061, ADM_OCS_EVENT_LEVEL_DEBUG, "vehicle monitor download process abnormal notify.", NULL);

    log_e("====>Down cond verify abnormal download task pause!");

    if (adm_dpm_getDownInfo()->s32UCMDownInstId == adm_dpm_getCurDownloadIngInstId())
    {
        adm_dpm_InteruptDownload(ADM_OCA_DOWNLOAD_ACTION_TYPE_COND_ABNORMAL_PAUSE);
    }
    else
    {
        (void)adm_dim_dpo_interuptDownload((D16U)adm_dpm_getCurDownloadIngInstId(), 30, ADM_OCA_DOWNLOAD_ACTION_TYPE_COND_ABNORMAL_PAUSE, &ierrorCode);
    }

    (void)pthread_rwlock_wrlock(&gstDPMdownInfo.stRWlock);
    gstDPMdownInfo.eMonitorState = ADM_DPM_MONITOR_ABNORMAL;  //mod by wl 20220328
    (void)pthread_rwlock_unlock(&gstDPMdownInfo.stRWlock);

    //!< 通知 uim 下载过程监控异常信息
    iRet = adm_dpm_downCondVerifyResultNotify2Uim(gstDPMdownInfo.eTriggerType, 
                    "DownloadMonitorAbnormal", 
                    pStCondResults,
                    (D32S)*pbIsConform);
    if (ADM_IS_ERR(iRet))
    {
        log_e("Failed to notify down monitor abnormal info! err[%d]", iRet);
        return NULL;
    }
    
    return NULL;
}

/* mod by wl 20220329 !
 *******************************************************************************
 * 转换车云协议中前置条件类型
 *
 * \note    通过触发类型转换为车云协议中前置条件类型
 *          1：车机端立即升级
 *          2：预约升级
 *          3：夜间自动升级
 *          4：静默升级
 *          5：手机立即升级
 * 
 * \param[in] eTriggerType 触发下载类型
 * \retval    ADM_DPM_UPGRADE_CONDITION_TYPE_E  车云协议中的升级条件类型
 */
ADM_DPM_UPGRADE_CONDITION_TYPE_E adm_dpm_changeTriggerTypeVal(ADM_APPLICATION_COMMON_CMDID_TYPE_E eTriggerType)
{
    ADM_DPM_UPGRADE_CONDITION_TYPE_E  eUpgradeConditionType;

    switch(eTriggerType)
    {
        /** 手机触发立即升级 */
        case ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_TSP:
        case ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_PHONE:
            eUpgradeConditionType = ADM_DPM_PHONE_UPGRADE_NOW;
            break;

        /** 预约升级 */
        case ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_SCHDULE:
            eUpgradeConditionType = ADM_DPM_ORDER_UPGRADE;
            break;
        
        /** 静默升级 */
        case ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_SILENCE:
            eUpgradeConditionType = ADM_DPM_SILENCE_UPGRADE;
            break;

        /** USB升级 */
        case ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_USB:
            eUpgradeConditionType = ADM_DPM_USB_UPGRADE;
            break;

        /** 救援升级 */
        case ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_RESCUE:
            eUpgradeConditionType = ADM_DPM_RESCUE_UPGRADE;
            break;

        /** 产线升级 */
        case ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_PROD_LINE:
            eUpgradeConditionType = ADM_DPM_PROD_LINE_UPGRADE;
            break;
        
        /** APK 自升级 */
        case ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_APK_SELF_UPGRADE:
            eUpgradeConditionType = ADM_DPM_CONDITION_TYPE_APK;
            break;
        
        /** 强制升级 */
        case ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_FORCE:
            eUpgradeConditionType = ADM_DPM_CONDITION_TYPE_FORCE;
            break;

        /** 车机端立即升级 */
        default:
            eUpgradeConditionType = ADM_DPM_VEHICLE_UPGRADE_NOW;
            break;
    }

    return eUpgradeConditionType;
}

/*!
 *******************************************************************************
 * DPM模块升级包拆包,并删除签名文件
 *
 * \note 
 *
 * \param[in]   pstFileObject      文件对象信息
 * \retval      D32S
 *******************************************************************************
 */
static D32S __adm_dpm_separatePackageOriginAndDelSignFile(ADM_COM_FILEOBJECT_T *pstFileObject)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    DCHAR *pcMainFileName = NULL;   // 正文文件名
    DCHAR *pcSignFileName = NULL;   // 签名文件名
    DBOOL bIsPkiVerifySign = DFALSE;
    
    if (ADM_IS_NULL(pstFileObject))
    {
        log_e("Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }
    log_i("Entry separate down package origin And sign file, filePath[%s].", pstFileObject->filePath);

    // 校验是否需要拆包
    if (FALSE == adm_com_checkIsNeedToVerifySignature(pstFileObject))
    {
        iRet = ADM_ECODE_SUCCESSED;
        log_d("conditions dissatisfy, dont need to separate package");
        return iRet;
    }

#ifdef SUPPORT_LION_PKI
            bIsPkiVerifySign = DTRUE;
#endif
    // 拆分正文+签名文件 
    iRet = adm_com_SeparateOriginAndSignFile(pstFileObject, bIsPkiVerifySign, &pcMainFileName, &pcSignFileName);
    if (ADM_IS_ERR(iRet))
    {
        iRet = ADM_ECODE_SPLIT_SIGNTEXT_FAILED;
        log_e("split package main text and sign text failed! err[%d]", iRet);
        return iRet; 
    }
    log_d("split package main text and sign text OK. mainText[%s], signText[%s]", pcMainFileName, pcSignFileName);

    adm_free(pstFileObject->filePath);
    pstFileObject->filePath = pcMainFileName;
    // 删除签名文件
    (void)adm_fremove(pcSignFileName);
    
    adm_free(pcSignFileName);

    return iRet;
}

/*!
 *******************************************************************************
 * 集合包验签拆包
 *
 * \note 
 *
 * \param[in]   pstDownCombinePkg      文件对象信息
 * \retval      D32S
 *******************************************************************************
 */
static D32S __adm_dpm_combinepkgSignatureAndUnpacking
(
    ADM_DPS_DOWN_ECUCOMBINEPKG_T *pstDownCombinePkg
)
{
    D32S iRet = ADM_ECODE_SUCCESSED;

    if (ADM_IS_NULL(pstDownCombinePkg))
    {
        log_e("Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }

    log_d("Entry combine package verify signature.");
    //验签.
    iRet = adm_dpm_signatureVerification(ADM_DPM_COMBINE_PACKAGE, NULL, &pstDownCombinePkg->stPkgFileObj, NULL);
    if (ADM_IS_ERR(iRet))
    {
        log_e("Verify combine package signature failed, fileName[%s], err[%d]", pstDownCombinePkg->stPkgFileObj.fileName, iRet);
        return iRet;
    }
    log_d("Verify combine package signature ok.");

    //拆出原文件并删除签名文件.
    iRet = __adm_dpm_separatePackageOriginAndDelSignFile(&pstDownCombinePkg->stPkgFileObj);
    if (ADM_IS_ERR(iRet))
    {
        log_e("Separate combine package originfile and del signfile failed, err[%d]", iRet);
        return iRet; 
    }
    log_d("Separate combine package originfile and del signfile ok.");

    return iRet;
}

/*!
 *******************************************************************************
 * DPM模块part包验签
 *
 * \note 
 * \param[in]   pcDownInstId       下载实例id
 * \param[in]   pstDownPartPkg     零件包信息
 * \retval      D32S
 *******************************************************************************
 */
static D32S __adm_dpm_partpkgSignature(ADM_DPS_DOWN_ECUPARTPKG_T *pstDownPartPkg)
{
    D32S iRet = ADM_ECODE_SUCCESSED;

    if (ADM_IS_NULL(pstDownPartPkg))
    {
        log_e("Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }

    log_d("Entry part package verify signature.");

    if(NULL != pstDownPartPkg->pstSrcEcuPkgInfo)
    {
        iRet = adm_dpm_signatureVerification(ADM_DPM_PART_SRC_PACKAGE, pstDownPartPkg, NULL, NULL);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Verify src package signature failed, err[%d]", iRet);
            return iRet;
        }

        log_d("Verify src package signature ok.");
    }

    iRet = adm_dpm_signatureVerification(ADM_DPM_PART_DST_PACKAGE, pstDownPartPkg, NULL, NULL);
    if (ADM_IS_ERR(iRet))
    {
        log_e("Verify dst package signature failed, err[%d]", iRet);
        return iRet;
    }
    log_d("Verify dst package signature ok.");

    log_d("part package verify signature ok.");

    return iRet;
}

/*!
 *******************************************************************************
 * 解密验签性能统计事件上报
 *
 * \note 扩展性不大,主要解决解密验签的函数压力
 *
 * \param[in]     pcEventId             事件id
 * \param[in]     ePkgType              包类型
 * \param[in]     pstFileObject         文件信息
 * \retval        D32S
 *******************************************************************************
 */
static void __adm_dpm_decryptAndSignatureReport
(
    const DCHAR                 *pcEventId,
    DCHAR                       *ecuReqid,
    DCHAR                       *ecuSwDid,
    struct timespec             *tsNowbegin,
    ADM_DPM_DECRYPTPKG_TYPE_E    ePkgType,
    ADM_COM_FILEOBJECT_T        *pstFileObject
)
{
    DCHAR szEventBuf[ADM_COM_LONGDATA_LEN] = {0};
    struct timespec tsNowend;

    if (ADM_IS_NULL(pcEventId) || ADM_IS_NULL(pstFileObject))
    {
        log_e("Invalid input param");
        return ;
    }

    (void)clock_gettime(CLOCK_MONOTONIC, &tsNowend);
    (void)adm_memzero(szEventBuf, sizeof(szEventBuf));

    if(0 == adm_strcasecmp(ADM_EVENT_ID_DL_007, pcEventId))
    {
        if ((ADM_DPM_PART_SRC_PACKAGE == ePkgType) || (ADM_DPM_PART_DST_PACKAGE == ePkgType))
        {
            (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), "STATSPACK decryptTime:ecuReqid[%s], ecuSwDid[%s], pkgType[%s], fileName[%s], decryptTime[%d]%s, fileSize[%lld]",
            ecuReqid,ecuSwDid,
            ((ADM_DPM_PART_SRC_PACKAGE == ePkgType) ? ("part-srcPkg") : ("part-dstPkg")),
            pstFileObject->fileName,
            ((tsNowend.tv_sec - tsNowbegin->tv_sec) < 1 ? (tsNowend.tv_nsec / (1000 * 1000)) - (tsNowbegin->tv_nsec / (1000 * 1000)) : (tsNowend.tv_sec - tsNowbegin->tv_sec)),
            ((tsNowend.tv_sec - tsNowbegin->tv_sec) < 1 ? "ms" : "s"),
            pstFileObject->size);
            adm_dpm_addEventAndReport(ADM_COM_MOD_DPM, FALSE,ADM_EVENT_ID_DL_007, ADM_OCS_EVENT_LEVEL_INFO, szEventBuf, NULL);
        }
        else
        {
            (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), "STATSPACK decryptTime:ecuReqid[%s], ecuSwDid[%s], pkgType[%s], fileName[%s], decryptTime[%d]%s, fileSize[%lld]",
            ecuReqid, ecuSwDid,
            ((ADM_DPM_COMBINE_SRC_ECU_PACKAGE == ePkgType) ? ("combine-srcecuPkg") : ("combine-dstecuPkg")),
            pstFileObject->fileName, 
            ((tsNowend.tv_sec - tsNowbegin->tv_sec) < 1 ? (tsNowend.tv_nsec / (1000 * 1000)) - (tsNowbegin->tv_nsec / (1000 * 1000)) : (tsNowend.tv_sec - tsNowbegin->tv_sec)),
            ((tsNowend.tv_sec - tsNowbegin->tv_sec) < 1 ? "ms" : "s"),
            pstFileObject->size);
            adm_dpm_addEventAndReport(ADM_COM_MOD_DPM, FALSE,ADM_EVENT_ID_DL_007, ADM_OCS_EVENT_LEVEL_INFO, szEventBuf, NULL);
        }
    }
    else if(0 == adm_strcasecmp(ADM_EVENT_ID_DL_008, pcEventId))
    {
        (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), "STATSPACK decryptTime:pkgType[combine] fileName[%s], decryptTime[%d]%s, fileSize[%lld]",
        pstFileObject->fileName,
        ((tsNowend.tv_sec - tsNowbegin->tv_sec) < 1 ? (tsNowend.tv_nsec / (1000 * 1000)) - (tsNowbegin->tv_nsec / (1000 * 1000)) : (tsNowend.tv_sec - tsNowbegin->tv_sec)),
        ((tsNowend.tv_sec - tsNowbegin->tv_sec) < 1 ? "ms" : "s"),
        pstFileObject->size);
        adm_dpm_addEventAndReport(ADM_COM_MOD_DPM, FALSE, ADM_EVENT_ID_DL_008, ADM_OCS_EVENT_LEVEL_INFO, szEventBuf, NULL);
    }
    else if(0 == adm_strcasecmp(ADM_EVENT_ID_DL_009, pcEventId))
    {
        if ((ADM_DPM_PART_SRC_PACKAGE == ePkgType) || (ADM_DPM_PART_DST_PACKAGE == ePkgType))
        {
            (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), "STATSPACK verifySignTime:ecuReqid[%s], ecuSwDid[%s], pkgType[%s], fileName[%s], verifySignTime[%d]%s, fileSize[%lld]",
            ecuReqid, ecuSwDid,
            ((ADM_DPM_PART_SRC_PACKAGE == ePkgType) ? ("part-srcPkg") : ("part-dstPkg")),
            pstFileObject->fileName, 
            ((tsNowend.tv_sec - tsNowbegin->tv_sec) < 1 ? (tsNowend.tv_nsec / (1000 * 1000)) - (tsNowbegin->tv_nsec / (1000 * 1000)) : (tsNowend.tv_sec - tsNowbegin->tv_sec)),
            ((tsNowend.tv_sec - tsNowbegin->tv_sec) < 1 ? "ms" : "s"),
            pstFileObject->size);
            adm_dpm_addEventAndReport(ADM_COM_MOD_DPM, FALSE,ADM_EVENT_ID_DL_009, ADM_OCS_EVENT_LEVEL_INFO, szEventBuf, NULL);
        }
        else
        {
            (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), "STATSPACK verifySignTime:ecuReqid[%s], ecuSwDid[%s], pkgType[%s], fileName[%s], verifySignTime[%d]%s, fileSize[%lld]",
            ecuReqid, ecuSwDid,
            ((ADM_DPM_COMBINE_SRC_ECU_PACKAGE == ePkgType) ? ("combine-srcecuPkg") : ("combine-dstecuPkg")),
            pstFileObject->fileName, 
            ((tsNowend.tv_sec - tsNowbegin->tv_sec) < 1 ? (tsNowend.tv_nsec / (1000 * 1000)) - (tsNowbegin->tv_nsec / (1000 * 1000)) : (tsNowend.tv_sec - tsNowbegin->tv_sec)),
            ((tsNowend.tv_sec - tsNowbegin->tv_sec) < 1 ? "ms" : "s"),
            pstFileObject->size);
            adm_dpm_addEventAndReport(ADM_COM_MOD_DPM, FALSE,ADM_EVENT_ID_DL_009, ADM_OCS_EVENT_LEVEL_INFO, szEventBuf, NULL);
        }
    }
    else if(0 == adm_strcasecmp(ADM_EVENT_ID_DL_010, pcEventId))
    {
        (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), "STATSPACK verifySignTime:pkgType[combine], fileName[%s], verifySignTime[%d]%s, fileSize[%lld]",
        pstFileObject->fileName, 
        ((tsNowend.tv_sec - tsNowbegin->tv_sec) < 1 ? (tsNowend.tv_nsec / (1000 * 1000)) - (tsNowbegin->tv_nsec / (1000 * 1000)) : (tsNowend.tv_sec - tsNowbegin->tv_sec)),
        ((tsNowend.tv_sec - tsNowbegin->tv_sec) < 1 ? "ms" : "s"),
        pstFileObject->size);
        adm_dpm_addEventAndReport(ADM_COM_MOD_DPM, FALSE,ADM_EVENT_ID_DL_010, ADM_OCS_EVENT_LEVEL_INFO, szEventBuf, NULL);
    }
    else
    {
        log_e("Failed to switch pcEventId[%d].", pcEventId);
    }

    return ;
}

#if 0
#endif


/*!
 *******************************************************************************
 * 获取OTA下载状态.
 *
 * \note    此接口用于获取当前OTA的下载状态.
 * 
 * \param   none
 * \retval  ADM_DPM_DOWNTASK_STATE_E
 *******************************************************************************
 */
ADM_DPM_DOWNTASK_STATE_E adm_dpm_getDownloadState(void)
{
    return gstDPMdownInfo.eState;
}

/*!
 *******************************************************************************
 * 获取OTA下载状态.
 *
 * \note    此接口用于获取当前OTA的下载状态.
 * 
 * \param   [inout] ppcDownStateString 下载状态字符串 
 * \retval  D32S   0: 成功 non-0：失败
 *******************************************************************************
 */
D32S adm_dpm_getDownloadStateString(DCHAR **ppcDownStateString)
{
    if (ADM_IS_NULL(ppcDownStateString))
    {
        log_e("Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }

    if ((ADM_DPM_DOWNTASK_TO_BE > gstDPMdownInfo.eState) || 
        (ADM_DPM_DOWNTASK_TRANS_SUCCESSED < gstDPMdownInfo.eState))
    {
        log_e("Invalid download state.");
        return ADM_ECODE_FAILED;
    }

    *ppcDownStateString = adm_strdup(gastDownloadState2StringMap[gstDPMdownInfo.eState].pcStateString);

    return ADM_ECODE_SUCCESSED;
}


/*!
 *******************************************************************************
 * 进入下载中状态.
 *
 * \note    设置下载的状态 ADM_DPM_DOWNTASK_ING    
 *
 * \param   none
 * \retval  D32S   0: 成功 non-0：失败
 *******************************************************************************
 */
void adm_dpm_setDownloadIng(void)
{
    gstDPMdownInfo.eState = ADM_DPM_DOWNTASK_ING;
}

/*!
 *******************************************************************************
 * 进入下载暂停状态.
 *
 * \note    设置下载的状态 ADM_DPM_DOWNTASK_PAUSE  .
 * 
 * \param   none
 * \retval  none
 *******************************************************************************
 */
void adm_dpm_setDownloadPause(void)
{
    gstDPMdownInfo.eState = ADM_DPM_DOWNTASK_PAUSE;
}

/*!
 *******************************************************************************
 * 进入下载取消状态.
 *
 * \note    设置下载的状态 ADM_DPM_DOWNTASK_CANCEL  .
 * 
 * \param   none
 * \retval  none
 *******************************************************************************
 */
void adm_dpm_setDownloadCancel(void)
{
    gstDPMdownInfo.eState = ADM_DPM_DOWNTASK_CANCEL;
}

/*!
 *******************************************************************************
 * 进入下载失败状态.
 *
 * \note    设置下载的状态 ADM_DPM_DOWNTASK_FAILED.      
 * 
 * \param   none
 * \retval  D32S   0: 成功 non-0：失败
 *******************************************************************************
 */
void adm_dpm_setDownloadFailed(void)
{
    gstDPMdownInfo.eState = ADM_DPM_DOWNTASK_FAILED;
}

/*!
 *******************************************************************************
 * 进入下载完成状态.
 *
 * \note    设置下载的状态 ADM_DPM_DOWNTASK_FINISHED.      
 * 
 * \param   none
 * \retval  D32S   0: 成功 non-0：失败
 *******************************************************************************
 */
void adm_dpm_setDownloadFinished(void)
{
    gstDPMdownInfo.eState = ADM_DPM_DOWNTASK_FINISHED;
}

/*!
 *******************************************************************************
 * 进入下载解压准备状态.
 *
 * \note    设置下载的状态 ADM_DPM_DOWNTASK_UNPACK_PREPARE.      
 * 
 * \param   none
 * \retval  D32S   0: 成功 non-0：失败
 *******************************************************************************
 */
void adm_dpm_setDownloadUnpackPrepare(void)
{
    gstDPMdownInfo.eState = ADM_DPM_DOWNTASK_UNPACK_PREPARE;
}

/*!
 *******************************************************************************
 * 进入下载解压中状态.
 *
 * \note    设置下载的状态 ADM_DPM_DOWNTASK_UNPACK_ING.      
 * 
 * \param   none
 * \retval  D32S   0: 成功 non-0：失败
 *******************************************************************************
 */
void adm_dpm_setDownloadUnpackIng(void)
{
    gstDPMdownInfo.eState = ADM_DPM_DOWNTASK_UNPACK_ING;
}

/*!
 *******************************************************************************
 * 进入下载解压失败状态.
 *
 * \note    设置下载的状态 ADM_DPM_DOWNTASK_UNPACK_FAILED.      
 * 
 * \param   none
 * \retval  D32S   0: 成功 non-0：失败
 *******************************************************************************
 */
void adm_dpm_setDownloadUnpackFailed(void)
{
    gstDPMdownInfo.eState = ADM_DPM_DOWNTASK_UNPACK_FAILED;
}

/*!
 *******************************************************************************
 * 进入下载解压成功状态.
 *
 * \note    设置下载的状态 ADM_DPM_DOWNTASK_UNPACK_SUCCESSED.      
 * 
 * \param   none
 * \retval  D32S   0: 成功 non-0：失败
 *******************************************************************************
 */
void adm_dpm_setDownloadUnpackSuccessed(void)
{
    gstDPMdownInfo.eState = ADM_DPM_DOWNTASK_UNPACK_SUCCESSED;
}

/*!
 *******************************************************************************
 * 进入下载预分包中状态.
 *
 * \note    设置下载的状态 ADM_DPM_DOWNTASK_TRANS_ING.      
 * 
 * \param   none
 * \retval  D32S   0: 成功 non-0：失败
 *******************************************************************************
 */
void adm_dpm_setDownloadTransIng(void)
{
    gstDPMdownInfo.eState = ADM_DPM_DOWNTASK_TRANS_ING;
}

/*!
 *******************************************************************************
 * 进入下载预分包暂停状态.
 *
 * \note    设置下载的状态 ADM_DPM_DOWNTASK_TRANS_PAUSED.      
 * 
 * \param   none
 * \retval  D32S   0: 成功 non-0：失败
 *******************************************************************************
 */
void adm_dpm_setDownloadTransPaused(void)
{
    gstDPMdownInfo.eState = ADM_DPM_DOWNTASK_TRANS_PAUSED;
}

/*!
 *******************************************************************************
 * 进入下载预分包失败状态.
 *
 * \note    设置下载的状态 ADM_DPM_DOWNTASK_TRANS_FAILED.      
 * 
 * \param   none
 * \retval  D32S   0: 成功 non-0：失败
 *******************************************************************************
 */
void adm_dpm_setDownloadTransFailed(void)
{
    gstDPMdownInfo.eState = ADM_DPM_DOWNTASK_TRANS_FAILED;
}

/*!
 *******************************************************************************
 * 进入下载预分包完成状态.
 *
 * \note    设置下载的状态 ADM_DPM_DOWNTASK_TRANS_SUCCESSED.      
 * 
 * \param   none
 * \retval  D32S   0: 成功 non-0：失败
 *******************************************************************************
 */
void adm_dpm_setDownloadTransSuccessed(void)
{
    gstDPMdownInfo.eState = ADM_DPM_DOWNTASK_TRANS_SUCCESSED;
}

/*!
 *******************************************************************************
 * 下载网络类型校验
 *
 * \note   下载网络类型校验
 * 
 * \param   none
 * \retval  D32S
 *******************************************************************************
 */
D32S adm_dpm_setWifiLink()
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    D32S iucmNetworkType = 0;   //主控网络类型
    D32S s32hostInstId = 0;
    D32S errorCode = 0;
    ADM_DPS_DOWNWIFILIST_T stDownWifiInfos = {0};
    ADM_HOST_WIFI_INFO_ARRAY_T *pstWifiArray = NULL;
    D32S i = 0;

    /**获取wifi信息*/
    iRet = adm_dps_getDownWifiInfos(&stDownWifiInfos);
    if (ADM_IS_ERR(iRet))
    {
        log_e("Failed to get DownWifiInfos, err[%d]", iRet);
        return iRet;
    }

    if (0 == stDownWifiInfos.iNum)
    {
        log_i("down wifi Infos is zero");
        return ADM_ECODE_SUCCESSED;
    }

    //host实例id
    s32hostInstId = adm_cfg_iniParserGetInt(ADM_CFG_VUS_HOST_INSTANCEID,  ADM_CFG_VUS_HOST_INSTANCEID_DEFAULT_VAL);

    pstWifiArray = (ADM_HOST_WIFI_INFO_ARRAY_T*)adm_malloc(sizeof(ADM_HOST_WIFI_INFO_ARRAY_T) + sizeof(ADM_HOST_WIFI_INFO_T) * stDownWifiInfos.iNum);
    if (ADM_IS_NULL(pstWifiArray))
    {
        log_e("Failed to malloc pstWifiArray");
        return ADM_ECODE_NO_MEMORY;
    }
    
    pstWifiArray->uArrLen = stDownWifiInfos.iNum;
    for(i = 0; i < pstWifiArray->uArrLen; i++)
    {
        pstWifiArray->pWifiList[i].pSsid = adm_strdup(stDownWifiInfos.pstDownloadWifiInfo[i].name);
        pstWifiArray->pWifiList[i].pPassword = adm_strdup(stDownWifiInfos.pstDownloadWifiInfo[i].password);
    }

    //主控host获取网络类型 
    iRet = adm_dim_host_setWifiLink((D16U)s32hostInstId, ADM_DPM_SET_WIFI_TIMEOUT, pstWifiArray, &errorCode);
    if (ADM_IS_ERR(iRet) || ADM_IS_ERR(errorCode))
    {
        log_e("Failed to set WifiLink,err[%d],errcode[%d]", iRet, errorCode);
        iRet = ADM_ECODE_SET_WIFILINK_FAILED;
    }

    stDownWifiInfos.iNum = 0;
    adm_free(stDownWifiInfos.pstDownloadWifiInfo);
    for(i = 0; i < pstWifiArray->uArrLen; i++)
    {
        adm_free(pstWifiArray->pWifiList[i].pSsid);
        adm_free(pstWifiArray->pWifiList[i].pPassword);
    }

    return iRet;
}


#if 0
#endif

static D64S dpmDownInstNeedSize(ADM_DPS_DOWNINSTINFO *pstDownInstanceInfo)
{
    D64S needSize = 0;

    if (ADM_IS_NULL(pstDownInstanceInfo))
    {
        log_e("pDownInstId is null");
        return 0;
    }

    if ((D16U)htoi(pstDownInstanceInfo->downInstId) == adm_soa_getUclInstanceId())
    {
        /**检查是否加密、检查是否流拼接方式*/
        if (DTRUE == adm_dps_checkDstPackageIsDecrypt(pstDownInstanceInfo->downInstId)
            || DFALSE == adm_dps_checkIsFlowsplicingType(pstDownInstanceInfo->downInstId))
        {
            //!< 简单的存储空间计算方法: 2倍升级包(解密需要1倍) + 最大的一个包大小(解压预留) + 配置文件大小 
            needSize = pstDownInstanceInfo->totalPkgSize + pstDownInstanceInfo->maxPkgSize + 50*1024;
        }
        else
        {
            /**主机升级包使用流拼接方式*/
            needSize = pstDownInstanceInfo->totalPkgSize + (300 * 1024 * 1024);
        }
    }
    else if ((D16U)htoi(pstDownInstanceInfo->downInstId) == adm_soa_getHostTboxInstanceId())
    {
        /**检查是否加密、检查是否流拼接方式*/
        if (DTRUE == adm_dps_checkDstPackageIsDecrypt(pstDownInstanceInfo->downInstId)
            || DFALSE == adm_dps_checkIsFlowsplicingType(pstDownInstanceInfo->downInstId))
        {
            //!< 简单的存储空间计算方法: 2倍升级包(解密需要1倍) + 最大的一个包大小(解压预留) + 配置文件大小 
            needSize = pstDownInstanceInfo->totalPkgSize + pstDownInstanceInfo->maxPkgSize + 50*1024;
        }
        else
        {
            /**TBOX升级包使用流拼接方式,升级包大小+2M*/
            needSize = pstDownInstanceInfo->totalPkgSize + (2 * 1024 * 1024);
        }
    }
    else
    {
        //!< 简单的存储空间计算方法: 2倍升级包(解密需要1倍) + 最大的一个包大小(解压预留) + 配置文件大小 
        needSize = (2 * pstDownInstanceInfo->totalPkgSize) + pstDownInstanceInfo->maxPkgSize + 50*1024;
    }

    return needSize;
}

/*!
 *******************************************************************************
 * 检查下载环境
 *
 * \note    主要判断：1、下载网络类型是否满足
 *                   2、存储空间是否满足
 *                   
 * 
 * \param   none
 * \retval  D32S
 *******************************************************************************
 */
 static D32S adm_dpm_checkDownloadEnviroment()
 {
    D32S iRet      = ADM_ECODE_SUCCESSED;
    D32S errorCode = ADM_ECODE_FAILED;
    D64S realSize = 0;
    D64S needSize = 0;
    D32S s32hostInstId = 0;
    D32S idownNetworkType = 0;
    struct list_head *pstNode     = NULL;
    struct list_head *pstNextNode = NULL;
    ADM_DPS_DOWNINSTLIST *pstDownInstList = NULL;
    ADM_DPS_DOWNINSTINFO *pstDownInstanceInfo = NULL;
    ADM_DPS_DOWNPACKAGEFORMAT_E eDownPkgFormat = ADM_DPS_DOWNPACKAGE_PART;
    DCHAR szEventBuf[ADM_COM_LEN_256] = {0};
    DBOOL bIsUsbDownload = DFALSE;
    ADM_DPS_DOWNWIFILIST_T stDownWifiInfos = {0};

    adm_dpm_addEventAndReport(ADM_COM_MOD_DPM, FALSE, ADM_EVENT_ID_DP_025, ADM_OCS_EVENT_LEVEL_DEBUG, "vehicle check download enviroment.", NULL);

    //1、校验下载前网络类型是否满足
    iRet = adm_dps_getDownNetworkType(&idownNetworkType);
    if (ADM_IS_ERR(iRet))
    {
        log_e("Failed to get DownNetworkType, err[%d]", iRet);
        return iRet;
    }

    log_d("[ABUP-Procedure] Check download NetworkType Start, network type is %d", idownNetworkType);

    /**产线升级模式，需要连接wifi*/
    if (ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_PROD_LINE == gstDPMdownInfo.eTriggerType)
    {
        iRet = adm_dpm_setWifiLink();
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to set WifiLink, network type is %d, err[%d]", idownNetworkType, iRet);
            if((D32S)ADM_DPM_DOWNLOAD_NETWORK_TYPE_ALL != idownNetworkType)
            {
                /**如果非任意网络，则返回失败*/
                return iRet;
            }
            /**如果是任意网络，则返回成功*/
            iRet = ADM_ECODE_SUCCESSED;
        }
    }
 
 	/** 如果是U盘升级，校验U盘是否移除 */
    if(ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_USB == gstDPMdownInfo.eTriggerType)
    {
        if (FALSE == adm_dpm_isUsbDirExist())
        {
            iRet = ADM_ECODE_USB_DIR_IS_NOT_EXIST;
            log_e("Failed to check usb download, err[%d]", iRet);
            return iRet;
        }
    }
    else
    {
        //任意网络时不需要校验
        if((D32S)ADM_DPM_DOWNLOAD_NETWORK_TYPE_ALL != idownNetworkType)
        {
            iRet = adm_dpm_networkTypeVerify(idownNetworkType);
            if (ADM_IS_ERR(iRet))
            {
                if ((D32S)ADM_DPM_DOWNLOAD_NETWORK_TYPE_WIFI == idownNetworkType)
                {
                    iRet = ADM_ECODE_WIFI_NET_VERIFY_FAILED;
                }
                else if ((D32S)ADM_DPM_DOWNLOAD_NETWORK_TYPE_OPERATOR == idownNetworkType)
                {
                    iRet = ADM_ECODE_4G_NET_VERIFY_FAILED;
                }
                else
                {
                    //do Nothing;
                }
                log_i("[ABUP-Procedure] Check download NetworkType End, ret[%d]",iRet);
                log_e("Failed to Verify network, err[%d]", iRet);
                return iRet;
            }
        }
    }
    log_i("[ABUP-Procedure] Check download NetworkType End, ret[%d]",iRet);


    log_i("[ABUP-Procedure] Check download Space Start");
    //2、校验存储空间是否满足
    iRet = adm_dps_EcuPackageFormat(&eDownPkgFormat);
    if (ADM_IS_ERR(iRet))
    {
        log_e("Failed to get packageFormat[Combine/part Package], err[%d]", iRet);
        (void)adm_memzero(szEventBuf, sizeof(szEventBuf));
        (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), "Failed to get packageFormat[Combine/part Package], err[%d]", iRet);
        adm_dpm_addEventAndReport(ADM_COM_MOD_DPM, TRUE, ADM_EVENT_ID_DP_026, ADM_OCS_EVENT_LEVEL_ERROR, "vehicle check download enviroment failed!", szEventBuf);

        iRet = ADM_ECODE_REQ_STORAGE_SPACE_FAILED;
        return iRet;
    }
    
    if (ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_USB == gstDPMdownInfo.eTriggerType)
    {
        bIsUsbDownload = DTRUE;
    }
    pstDownInstList = adm_dps_getEcuDownInstanceList(eDownPkgFormat, bIsUsbDownload);
    if (ADM_IS_NULL(pstDownInstList))
    {
        log_e("Failed to get ecu down instance list, PkgFromat[%d], err[%d]", eDownPkgFormat, iRet);

        (void)adm_memzero(szEventBuf, sizeof(szEventBuf));
        (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), "Failed to get ecu down instance list, PkgFromat[%d], err[%d]",eDownPkgFormat, iRet);
        adm_dpm_addEventAndReport(ADM_COM_MOD_DPM, TRUE, ADM_EVENT_ID_DP_026, ADM_OCS_EVENT_LEVEL_ERROR, "vehicle check download enviroment failed!", szEventBuf);

        iRet = ADM_ECODE_REQ_STORAGE_SPACE_FAILED;
        return iRet;
    }
    log_d("ecu down instance list num[%d], PkgFromat[%d]", pstDownInstList->num, eDownPkgFormat);
    (void)adm_memzero(szEventBuf, sizeof(szEventBuf));
    (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), "ecu down instance list num[%d], PkgFromat[%d]", pstDownInstList->num, eDownPkgFormat);
    adm_dpm_addEventAndReport(ADM_COM_MOD_DPM, FALSE, ADM_EVENT_ID_DP_030, ADM_OCS_EVENT_LEVEL_TRACE, "vehicle get download instance list successed.", szEventBuf);

    list_for_each_safe(pstNode, pstNextNode, &pstDownInstList->stHead)
    {
        //!< 遍历校验下载受控对象下载的升级包大小与内存的大小
        pstDownInstanceInfo = list_entry(pstNode, ADM_DPS_DOWNINSTINFO, stHead);

        realSize  = 0;
        errorCode = ADM_ECODE_FAILED;

        needSize = dpmDownInstNeedSize(pstDownInstanceInfo);

        log_i("Calc downInstId[%s] need storage space[%lld]", pstDownInstanceInfo->downInstId, needSize);
        if (gstDPMdownInfo.s32UCMDownInstId == (D32S)htoi(pstDownInstanceInfo->downInstId))
        {
            s32hostInstId = adm_cfg_iniParserGetInt(ADM_CFG_VUS_HOST_INSTANCEID,  ADM_CFG_VUS_HOST_INSTANCEID_DEFAULT_VAL);
            
            (void)adm_memzero(szEventBuf, sizeof(szEventBuf));
            (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), "req downInstId[%s], hostInstId[%d], needSize[%lld].", 
                    pstDownInstanceInfo->downInstId, s32hostInstId, needSize);
            adm_dpm_addEventAndReport(ADM_COM_MOD_DPM, FALSE, ADM_EVENT_ID_DP_031, ADM_OCS_EVENT_LEVEL_TRACE, "vehicle verify UCM storage space is enouth.", szEventBuf);

            //!< 判断主控的存储空间是否满足
            iRet = adm_dim_host_verifySpaceIsEnough((D16U)s32hostInstId, 
                        ADM_DPM_SYNC_REQ_TIMEOUT, needSize, &realSize, &errorCode);
            if(ADM_IS_ERR(iRet))
            {
                log_e("Failed to request host verify storage space, err[%d][%d]", iRet, ADM_ECODE_REQ_STORAGE_SPACE_FAILED);
 
                (void)adm_memzero(szEventBuf, sizeof(szEventBuf));
                (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), "Failed to request host verify storage space, err[%d][%d]", 
                        iRet, ADM_ECODE_REQ_STORAGE_SPACE_FAILED);
                adm_dpm_addEventAndReport(ADM_COM_MOD_DPM, TRUE, ADM_EVENT_ID_DP_032, ADM_OCS_EVENT_LEVEL_ERROR, "vehicle verify UCM storage space is enouth failed!", szEventBuf);

                iRet = ADM_ECODE_REQ_STORAGE_SPACE_FAILED;
                break;
            }
            log_i("Check UCM storage space, rspCode[%d]", errorCode);

            (void)adm_memzero(szEventBuf, sizeof(szEventBuf));
            (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), "rsp downInstId[%s], hostInstId[%d], needSize[%lld], realSize[%lld].", 
                    pstDownInstanceInfo->downInstId, s32hostInstId, needSize, realSize);
            adm_dpm_addEventAndReport(ADM_COM_MOD_DPM, FALSE, ADM_EVENT_ID_DP_033, ADM_OCS_EVENT_LEVEL_DEBUG, "vehicle verify UCM storage space is enouth successed.", szEventBuf);

        }
        else 
        {
            (void)adm_memzero(szEventBuf, sizeof(szEventBuf));
            (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), "req downInstId[%s], uclInstId[%s], needSize[%lld].", 
                    pstDownInstanceInfo->downInstId, pstDownInstanceInfo->downInstId, needSize);
            adm_dpm_addEventAndReport(ADM_COM_MOD_DPM, FALSE, ADM_EVENT_ID_DP_034, ADM_OCS_EVENT_LEVEL_TRACE, "vehicle verify UCL storage space is enouth.", szEventBuf);

            //!< 判断从控的存储空间是否满足 
            iRet = adm_dim_ucl_verifySpaceIsEnouth((D16U)htoi(pstDownInstanceInfo->downInstId), 
                        ADM_DPM_SYNC_REQ_TIMEOUT, needSize, &realSize, &errorCode);
            if(ADM_IS_ERR(iRet))
            {
                log_e("Failed to request host verify storage space, err[%d][%d]", iRet, ADM_ECODE_REQ_STORAGE_SPACE_FAILED);

                (void)adm_memzero(szEventBuf, sizeof(szEventBuf));
                (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), "Failed to request host verify storage space, err[%d][%d]", 
                        iRet, ADM_ECODE_REQ_STORAGE_SPACE_FAILED);
                adm_dpm_addEventAndReport(ADM_COM_MOD_DPM, TRUE, ADM_EVENT_ID_DP_035, ADM_OCS_EVENT_LEVEL_ERROR, "vehicle verify UCL storage space is enouth failed!", szEventBuf);

                iRet = ADM_ECODE_REQ_STORAGE_SPACE_FAILED;
                break;
            }
            log_i("Check UCL storage space, rspCode[%d]", errorCode);
            (void)adm_memzero(szEventBuf, sizeof(szEventBuf));
            (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), "rsp downInstId[%s], uclInstId[%s], needSize[%lld], realSize[%lld].", 
                    pstDownInstanceInfo->downInstId, pstDownInstanceInfo->downInstId, needSize, realSize);
            adm_dpm_addEventAndReport(ADM_COM_MOD_DPM, FALSE, ADM_EVENT_ID_DP_036, ADM_OCS_EVENT_LEVEL_DEBUG, "vehicle verify UCL storage space is enouth successed.", szEventBuf);
        }

        log_i("Compare storage space, rspCode[%d], needSize[%lld], realSize[%lld]", errorCode, needSize, realSize);
        //!< host 和 ucl 已经比较过了 needSize 和         realSize 这里不用再比较了
        if (ADM_IS_SUCCESS(errorCode))
        {
            continue;
        }
        else
        {
            iRet = ((gstDPMdownInfo.s32UCMDownInstId == (D32S)htoi(pstDownInstanceInfo->downInstId)) ? \
                        (ADM_ECODE_UCM_STORAGE_SPACE_NOT_ENOUGH) : (ADM_ECODE_UCL_STORAGE_SPACE_NOT_ENOUGH));
            log_e("Check device storage space Failed, err[%d], rspCode[%d], needSize[%lld], realSize[%lld]", 
                    iRet, errorCode, needSize, realSize);

            (void)adm_memzero(szEventBuf, sizeof(szEventBuf));
            (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), "Check device storage space Failed, err[%d], rspCode[%d], needSize[%lld], realSize[%lld]", 
                    iRet, errorCode, needSize, realSize);
            break;
        }
    }

    //!< 释放下载受控对象list
    adm_dps_delEcuDownInstanceList(&pstDownInstList);

    log_i("[ABUP-Procedure] Check download Space End, ret[%d]",iRet);

    //mod by wl 20220321
    if(ADM_IS_ERR(iRet))
    {
        adm_dpm_addEventAndReport(ADM_COM_MOD_DPM, TRUE, ADM_EVENT_ID_DP_026, ADM_OCS_EVENT_LEVEL_ERROR, "vehicle check download enviroment failed!", szEventBuf);
    }
    else
    {
        adm_dpm_addEventAndReport(ADM_COM_MOD_DPM, FALSE, ADM_EVENT_ID_DP_027, ADM_OCS_EVENT_LEVEL_DEBUG, "vehicle check download enviroment successed.", NULL);
    }

    return iRet;
 }


/*!
 *******************************************************************************
 * 检查下载前置条件
 *
 * \note    主要判断：batteryPower/enNetType/MemorySpace(包含MP5、TBox)
 *          其中存储空间需要做空间安全性校验(sysi4host)
 * 
 * \param[in] eTriggerType 触发下载类型
 * \retval    D32S
 *******************************************************************************
 */
static D32S adm_dpm_checkDownloadPreCondition
(    
    ADM_APPLICATION_COMMON_CMDID_TYPE_E eTriggerType
)
{
    D32S i = 0;
    D32S iRet = ADM_ECODE_SUCCESSED;
    D32S iNotifyRet = ADM_ECODE_SUCCESSED;   //通知uim的返回值
    DBOOL bIsConform = TRUE;
    ADM_DPM_UPGRADE_CONDITION_TYPE_E eConvertVal;
    ADM_DPS_DOWNCONDITION_T stDownCond = {0};
    ADM_UIM_SOA_CONDITION_DATA_ARRAY_T *pstCondResults = NULL;
    DCHAR szEventBuf[ADM_COM_LEN_256] = {0};  //mod by wl 20220321

    //mod by wl 20220321
    (void)adm_memzero(szEventBuf, sizeof(szEventBuf));
    (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), "downlaod eTriggerType[%d]",eTriggerType);
    adm_dpm_addEventAndReport(ADM_COM_MOD_DPM, FALSE, ADM_EVENT_ID_DP_037, ADM_OCS_EVENT_LEVEL_DEBUG, "vehicle check download preCondition.", szEventBuf);

    /* 删除：当reboot重启设备后，前置条件变更了，此时跳过了前置条件校验，导致前置条件校验跳过了。
       因为只存在一个下载包，导致周期性条件校验时，也不会停止下载当前的升级包，导致下载成功
    //!< 若为车机触发则跳过前置条件校验 
    //!< 车机的下载前置条件校验在触发下载之前就校验了(adm_dpm_uimDownloadConditionsVerified)
    if (ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_USER == eTriggerType)
    {
        log_d("Trigger downlaod type is hut.");

        adm_dpm_addEventAndReport(ADM_COM_MOD_DPM, FALSE, ADM_EVENT_ID_DP_039, ADM_OCS_EVENT_LEVEL_DEBUG, 
                "vehicle check download preCondition successed.", "Trigger downlaod type is hut.");
        return ADM_ECODE_SUCCESSED;
    }*/

    do
    {
        //!< 获取下载条件
        iRet = adm_dps_getDownloadCondition(&stDownCond);
        if (ADM_IS_ERR(iRet))
        {
            log_e( "Failed to get downlaod condition, err[%d].", iRet);

            //mod by wl 20220321
            (void)adm_memzero(szEventBuf, sizeof(szEventBuf));
            (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), "Failed to get downlaod condition, err[%d]", iRet);
            adm_dpm_addEventAndReport(ADM_COM_MOD_DPM, TRUE, ADM_EVENT_ID_DP_038, ADM_OCS_EVENT_LEVEL_ERROR, "vehicle check download condition failed.", szEventBuf);
            break;
        }

        //!< 平台未配置相关下载前置条件，直接返回
        if (0 == stDownCond.trigNum)
        {
            iRet = ADM_ECODE_SUCCESSED;
            log_i("OTA Service no set download condition.");
            (void)adm_memzero(szEventBuf, sizeof(szEventBuf));
            (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), "OTA Service no set download condition.");
            adm_dpm_addEventAndReport(ADM_COM_MOD_DPM, FALSE, ADM_EVENT_ID_DP_045, ADM_OCS_EVENT_LEVEL_DEBUG, "vehicle check download condition successed.", szEventBuf);
            break;
        }

        //mod by wl 20220329 
        eConvertVal = adm_dpm_changeTriggerTypeVal(eTriggerType);

        //!< 找到车云协议对应模式的的前置条件
        for (i = 0; i < stDownCond.trigNum; i++)
        {
            if ((D32S)eConvertVal == stDownCond.pstTrigDownCondList[i].iType)
            {
                break;
            }
        }

        //!< 未找到车云协议对应模式的的前置条件、或者前置条件没有数据
        if (i == stDownCond.trigNum || 0 == stDownCond.pstTrigDownCondList[i].iCondNum)
        {
            iRet = ADM_ECODE_SUCCESSED;
            log_i("OTA Service not find download condition.");
            (void)adm_memzero(szEventBuf, sizeof(szEventBuf));
            (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), "OTA Service not find download condition.");
            adm_dpm_addEventAndReport(ADM_COM_MOD_DPM, FALSE, ADM_EVENT_ID_DP_045, ADM_OCS_EVENT_LEVEL_DEBUG, "vehicle check download condition successed.", szEventBuf);
            break;
        }
        
        //mod by wl 20220321
        adm_dpm_addEventAndReport(ADM_COM_MOD_DPM, FALSE, ADM_EVENT_ID_DP_043, ADM_OCS_EVENT_LEVEL_TRACE, "vehicle verify download condition.", NULL);
        iRet = adm_cdm_verify_Compare(&stDownCond.pstTrigDownCondList[i], &pstCondResults, &bIsConform);    
        if (ADM_IS_ERR(iRet))
        {
            log_e( "Failed to verify compare download condition, err[%d].", iRet);
            iRet = ADM_ECODE_CONDITION_VERIFY_FAILED;
            //mod by wl 20220321
            (void)adm_memzero(szEventBuf, sizeof(szEventBuf));
            (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), "Failed to verify compare download condition, err[%d].", iRet);
            adm_dpm_addEventAndReport(ADM_COM_MOD_DPM, TRUE, ADM_EVENT_ID_DP_044, ADM_OCS_EVENT_LEVEL_ERROR, "vehicle verify download condition failed!", szEventBuf);
            //失败不直接break，需要将校验结果通知uim.
        }
        //mod by wl 20220321
        adm_dpm_addEventAndReport(ADM_COM_MOD_DPM, FALSE, ADM_EVENT_ID_DP_045, ADM_OCS_EVENT_LEVEL_DEBUG, "vehicle verify download condition successed.", NULL);

        //!< 通知 uim 下载前置条件校验结果
        iNotifyRet = adm_dpm_downCondVerifyResultNotify2Uim(eTriggerType, 
                        "DownloadPreConditionVerify", 
                        pstCondResults,
                        (D32S)bIsConform);
        if (ADM_IS_ERR(iNotifyRet))
        {
            log_e("Failed to notify down condition verify result! err[%d]", iNotifyRet);
            break;
        }
    } while(FALSE);

    //!< 最后释放比较结果信息内存
    if (NULL != pstCondResults)
    {
        (void)adm_cdm_verify_free(pstCondResults);
    }
    adm_dps_cleanEcuDownCondition(&stDownCond);

    //mod by wl 20220321
    if (ADM_IS_ERR(iRet))
    {
        adm_dpm_addEventAndReport(ADM_COM_MOD_DPM, TRUE, ADM_EVENT_ID_DP_038, ADM_OCS_EVENT_LEVEL_ERROR, "vehicle check download preCondition failed!", szEventBuf);
    }
    else
    {
        adm_dpm_addEventAndReport(ADM_COM_MOD_DPM, FALSE, ADM_EVENT_ID_DP_039, ADM_OCS_EVENT_LEVEL_DEBUG, "vehicle check download preCondition successed.", NULL);
    }

    return iRet;
}



/*!
 *******************************************************************************
 * 启动下载过程监控
 *
 * \note
 *          
 * \param[in]   fnMonitorNotify     下载过程条件异常通知回调函数
 * \param[in]   eTriggerType        触发下载类型
 * \retval      D32S
 *******************************************************************************
 */
static D32S adm_dpm_startDownloadProcessMonitor
(
    ADM_CDM_NOTIFY_Fn fnMonitorNotify,
    ADM_APPLICATION_COMMON_CMDID_TYPE_E eTriggerType
)
{
    D32S i = 0; //mod by wl 20220329
    ADM_DPM_UPGRADE_CONDITION_TYPE_E eConvertVal; //mod by wl 20220329
    D32S iRet = ADM_ECODE_SUCCESSED;
    adm_handle_t downMonitorHandle = NULL;
    ADM_DPS_DOWNCONDITION_T stDownCond = {0};
    DCHAR szEventBuf[ADM_COM_LEN_256] = {0};  //mod by wl 20220321

    //mod by wl 20220321
    adm_dpm_addEventAndReport(ADM_COM_MOD_DPM, FALSE, ADM_EVENT_ID_DP_046, ADM_OCS_EVENT_LEVEL_DEBUG, "vehicle start download process monitor.", NULL);

    (void)pthread_rwlock_rdlock(&gstDPMdownInfo.stRWlock);
    if (TRUE == gstDPMdownInfo.isMonitorRunning)
    {
        (void)pthread_rwlock_unlock(&gstDPMdownInfo.stRWlock);
        log_e("Download monitor thread alreadly running.");

        //mod by wl 20220321
        (void)adm_memzero(szEventBuf, sizeof(szEventBuf));
        (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), "Download monitor thread alreadly running, err[%d]", ADM_ECODE_PTHREAD_ALREADY_RUNNING);
        adm_dpm_addEventAndReport(ADM_COM_MOD_DPM, FALSE, ADM_EVENT_ID_DP_048, ADM_OCS_EVENT_LEVEL_DEBUG, "vehicle start download process monitor successed.", szEventBuf);

        return ADM_ECODE_SUCCESSED;
    }
    (void)pthread_rwlock_unlock(&gstDPMdownInfo.stRWlock);

    //!< 获取下载条件
    (void)adm_memzero(&stDownCond, sizeof(stDownCond));
    iRet = adm_dps_getDownloadCondition(&stDownCond);
    if (ADM_IS_ERR(iRet))
    {
        log_e( "Failed to get downlaod condition, err[%d].", iRet);

        //mod by wl 20220321
        (void)adm_memzero(szEventBuf, sizeof(szEventBuf));
        (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), "Failed to get downlaod condition, err[%d]",iRet);
        adm_dpm_addEventAndReport(ADM_COM_MOD_DPM, TRUE, ADM_EVENT_ID_DP_047, ADM_OCS_EVENT_LEVEL_ERROR, "vehicle start download process monitor failed!", szEventBuf);

        return iRet;
    }

    //!< 启动下载过程监控
    //mod by wl 20220321
    adm_dpm_addEventAndReport(ADM_COM_MOD_DPM, FALSE, ADM_EVENT_ID_DP_049, ADM_OCS_EVENT_LEVEL_TRACE, "vehicle loop verify download condition", NULL);

    //!< 平台未配置相关下载前置条件，直接返回
    if (0 == stDownCond.trigNum)
    {
        iRet = ADM_ECODE_SUCCESSED;
        log_i("OTA Service no set download condition.");
        (void)adm_memzero(szEventBuf, sizeof(szEventBuf));
        (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), "OTA Service no set download condition.");
        adm_dpm_addEventAndReport(ADM_COM_MOD_DPM, TRUE, ADM_EVENT_ID_DP_048, ADM_OCS_EVENT_LEVEL_ERROR, "vehicle check download monitor successed.", szEventBuf);
        adm_dps_cleanEcuDownCondition(&stDownCond);
        return iRet;
    }

    //找到车云协议对应模式的的前置条件 mod by wl 20220329 
    eConvertVal = adm_dpm_changeTriggerTypeVal(eTriggerType);
    for (i = 0; i < stDownCond.trigNum; i++)
    {
        if ((D32S)eConvertVal == stDownCond.pstTrigDownCondList[i].iType)
        {
            break;
        }
    }

    //!< 未找到车云协议对应模式的的前置条件\或者前置条件没有数据
    if (i == stDownCond.trigNum || 0 == stDownCond.pstTrigDownCondList[i].iCondNum)
    {
        iRet = ADM_ECODE_SUCCESSED;
        log_i("OTA Service not find download condition.");
        (void)adm_memzero(szEventBuf, sizeof(szEventBuf));
        (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), "OTA Service not find download condition.");
        adm_dpm_addEventAndReport(ADM_COM_MOD_DPM, TRUE, ADM_EVENT_ID_DP_048, ADM_OCS_EVENT_LEVEL_ERROR, "vehicle check download monitor successed.", szEventBuf);
        adm_dps_cleanEcuDownCondition(&stDownCond);
        return iRet;
    }

    downMonitorHandle = adm_cdm_LoopVerify(&stDownCond.pstTrigDownCondList[i], ADM_DPM_DEFDOWN_CYCLE_CNT, fnMonitorNotify);
    if (ADM_IS_NULL(downMonitorHandle))
    {
        iRet = ADM_ECODE_INVALID_PARAM;
        log_e( "Failed to start downlaod condition by cdm, err[%d].", iRet);

        //mod by wl 20220321
        (void)adm_memzero(szEventBuf, sizeof(szEventBuf));
        (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), "Failed to start downlaod condition by cdm, err[%d]",iRet);
        adm_dpm_addEventAndReport(ADM_COM_MOD_DPM, TRUE, ADM_EVENT_ID_DP_050, ADM_OCS_EVENT_LEVEL_ERROR, "vehicle loop verify download condition failed!", szEventBuf);
        adm_dpm_addEventAndReport(ADM_COM_MOD_DPM, TRUE, ADM_EVENT_ID_DP_047, ADM_OCS_EVENT_LEVEL_ERROR, "vehicle start download process monitor failed!", szEventBuf);
        adm_dps_cleanEcuDownCondition(&stDownCond);
        return iRet;
    }
    //mod by wl 20220321
    adm_dpm_addEventAndReport(ADM_COM_MOD_DPM, FALSE, ADM_EVENT_ID_DP_051, ADM_OCS_EVENT_LEVEL_DEBUG, "vehicle loop verify download condition successed.", NULL);

    (void)pthread_rwlock_wrlock(&gstDPMdownInfo.stRWlock);
    gstDPMdownInfo.monitorHandle = downMonitorHandle;
    gstDPMdownInfo.isMonitorRunning = TRUE;
    (void)pthread_rwlock_unlock(&gstDPMdownInfo.stRWlock);

    adm_dps_cleanEcuDownCondition(&stDownCond);

    //mod by wl 20220321
    adm_dpm_addEventAndReport(ADM_COM_MOD_DPM, FALSE, ADM_EVENT_ID_DP_048, ADM_OCS_EVENT_LEVEL_DEBUG, "vehicle start download process monitor successed.", NULL);

    return iRet;
}

/*!
 *******************************************************************************
 * 停止下载过程监控
 *
 * \note    
 * 
 * \param   none
 * \retval  none
 *******************************************************************************
 */
static void adm_dpm_stopDownloadProcessMonitor(void)
{
    D32S iRet = ADM_ECODE_SUCCESSED;

    //mod by wl 20220321
    adm_dpm_addEventAndReport(ADM_COM_MOD_DPM, TRUE, ADM_EVENT_ID_DE_001, ADM_OCS_EVENT_LEVEL_INFO, "vehicle stop download process monitor.", NULL);

    (void)pthread_rwlock_rdlock(&gstDPMdownInfo.stRWlock);
    if (FALSE == gstDPMdownInfo.isMonitorRunning)
    {
        (void)pthread_rwlock_unlock(&gstDPMdownInfo.stRWlock);
        log_i("Download monitor thread not exist.");
        return;
    }
    (void)pthread_rwlock_unlock(&gstDPMdownInfo.stRWlock);

    iRet = adm_cdm_LoopFinish(gstDPMdownInfo.monitorHandle);
    if (ADM_IS_ERR(iRet))
    {
        log_e("Failed to stop down monitor by cdm loop finish, err[%d]", iRet);
    }

    (void)pthread_rwlock_wrlock(&gstDPMdownInfo.stRWlock);
    gstDPMdownInfo.monitorHandle = NULL;
    gstDPMdownInfo.isMonitorRunning  = FALSE;
    gstDPMdownInfo.eMonitorState = ADM_DPM_MONITOR_NORMAL;  //mod by wl 20220328 
    (void)pthread_rwlock_unlock(&gstDPMdownInfo.stRWlock);

    return;
}

/*!
 *******************************************************************************
 * 确认下载任务有效性
 *
 * \note    
 * 
 * \param   none
 * \retval  D32
 *******************************************************************************
 */
static D32S adm_dpm_confirmDownloadTask(void)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    ADM_DPS_DOWNTSAKINFO_T stDownTaskInfo = {0};
    DCHAR szEventBuf[ADM_COM_LEN_256] = {0};

    adm_dpm_addEventAndReport(ADM_COM_MOD_DPM, FALSE, ADM_EVENT_ID_DP_013, ADM_OCS_EVENT_LEVEL_DEBUG, "vehicle confirm download task is invalid", NULL);
    log_i("[ABUP-Procedure] Task validity verification Start");

    do
    {
        //!< 获取下载任务信息 
        (void)adm_memzero(&stDownTaskInfo, sizeof(stDownTaskInfo));
        iRet = adm_dps_getDownloadTaskInfo(&stDownTaskInfo);
        if (ADM_IS_ERR(iRet))
        {
            log_e( "Failed to get download task info, err[%d]", iRet);
            (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), "Failed to get download task info, err[%d]", iRet);
            break;
        }
        
        (void)adm_memzero(szEventBuf, sizeof(szEventBuf));
        (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), 
                        "download task info: sessionId[%s], taskId[%lld], strategyId[%lld].", 
                        ADM_PRINT_STR(stDownTaskInfo.sessionId),
                        stDownTaskInfo.taskId,
                        stDownTaskInfo.strategyId);
        adm_dpm_addEventAndReport(ADM_COM_MOD_DPM, FALSE, ADM_EVENT_ID_DP_019, ADM_OCS_EVENT_LEVEL_TRACE, "vehicle check download task info.", szEventBuf);

        //!< 确认下载任务有效性 --- 上报车端状态代替确认任务有效性
        //!< 只有云端下发任务终止时，才默认为任务有效性校验失败，其他场景，不影响下载。
        iRet = adm_dpm_taskVerify();
        if (ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_USB == gstDPMdownInfo.eTriggerType)
        {
            ///< USB模式不进行任务有效性检查时,仍然把校验请求发到云端,但是不检查云端响应结果
            log_i( "usb no check task mode,task confirm result[%d].", iRet);
            iRet = ADM_ECODE_SUCCESSED;
            break;
        }
        else
        {
            if (ADM_ECODE_UPGRADE_TASK_STOPED == iRet)
            {
                log_e( "Failed to confirm downlaod task, err[%d].", iRet);
                (void)adm_memzero(szEventBuf, sizeof(szEventBuf));
                (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), "Failed to confirm downlaod task, err[%d]", iRet);
                adm_dpm_addEventAndReport(ADM_COM_MOD_DPM, TRUE, ADM_EVENT_ID_DP_020, ADM_OCS_EVENT_LEVEL_ERROR, "vehicle check download task info failed", szEventBuf);
                break;
            }
            else
            {
                log_i( "confirm downlaod task normal download, err[%d].", iRet);
                iRet = ADM_ECODE_SUCCESSED;
            }
            
            adm_dpm_addEventAndReport(ADM_COM_MOD_DPM, FALSE, ADM_EVENT_ID_DP_021, ADM_OCS_EVENT_LEVEL_DEBUG, "vehicle check download task info successed", NULL);
        }
    } while (FALSE);

    adm_dps_cleanDownTaskInfo(&stDownTaskInfo);

    log_i("[ABUP-Procedure] Task validity verification End, ret[%d]",iRet);

    if (ADM_IS_ERR(iRet))
    {
        adm_dpm_addEventAndReport(ADM_COM_MOD_DPM, TRUE, ADM_EVENT_ID_DP_014, ADM_OCS_EVENT_LEVEL_ERROR, "vehicle confirm download task is invalid failed", szEventBuf);
    }
    else
    {
        adm_dpm_addEventAndReport(ADM_COM_MOD_DPM, FALSE, ADM_EVENT_ID_DP_015, ADM_OCS_EVENT_LEVEL_DEBUG, "vehicle confirm download task is invalid successed", NULL);
    }

    return iRet;
}

#if 0
#endif


/*!
 *******************************************************************************
 * 转换文件对象为解密后的文件对象 
 *
 * \note 若文件对象不需要解密则返回成功
 * 
 * \param[inout] pstPkgFileObj  加密的文件对象 
 * \param[in]    pcDecryptPath  对象的解密后的文件路径
 * \retval       none
 *******************************************************************************
 */
static D32S adm_dpm_changeFileObj2DecryptFileObj
(
    ADM_COM_FILEOBJECT_T *pstPkgFileObj,
    DCHAR                *pcDecryptPath
)
{

    if (ADM_IS_NULL(pstPkgFileObj) || ADM_IS_NULL(pcDecryptPath))
    {
        log_e("Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }

    //!< 是否需要解密 
    if((NULL == pstPkgFileObj->pstFileSecurityInfo) || (pstPkgFileObj->pstFileSecurityInfo->isEncrypt == 0)) 
    {
        log_i("Filed dont need decrypt, file[%s]", pstPkgFileObj->filePath);
        return ADM_ECODE_SUCCESSED;
    }

    // 将解密后的信息填充到上层，并将 isEncrypt 置为 0，表示未加密 
    adm_free(pstPkgFileObj->filePath);
    pstPkgFileObj->filePath = adm_strdup(pcDecryptPath);
    adm_free(pstPkgFileObj->hash);
    pstPkgFileObj->hash     = adm_strdup(pstPkgFileObj->pstFileSecurityInfo->oriHash);
    pstPkgFileObj->size     = pstPkgFileObj->pstFileSecurityInfo->oriSize;

    pstPkgFileObj->pstFileSecurityInfo->isEncrypt = 0;

    return ADM_ECODE_SUCCESSED;
}

/*!
 *******************************************************************************
 * DPM模块验签
 *
 * \note
 * 
 * \param[in]     ePkgType              包类型
 * \param[inout]  pstDownPartPkg        零件包信息
 * \param[inout]  pstCombinePkgFileObj  集合包信息
 * \param[inout]  pstCombineEcuIdInfo   集合包中ecu信息
 * \retval        D32S
 *******************************************************************************
 */
D32S adm_dpm_signatureVerification
(
    ADM_DPM_DECRYPTPKG_TYPE_E  ePkgType,
    ADM_DPS_DOWN_ECUPARTPKG_T *pstDownPartPkg,
    ADM_COM_FILEOBJECT_T      *pstCombinePkgFileObj,
    ADM_DPS_ECUIDINFO_T       *pstCombineEcuIdInfo
)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    D32S ierrorCode = ADM_ECODE_SUCCESSED;
    D32S s32hostInstId = 0;
    ADM_COM_FILEOBJECT_T *pstFileObject = NULL;
    DCHAR *pcDownInstId = NULL;
    DCHAR *pfileId = NULL;
    struct timespec tsNowSignaturebegin;
    DBOOL bIsPkiVerifySign = DFALSE;

    ADM_COM_SIGNATUREINFO_T *pstSignatureInfo = NULL;
    
    //!< 参数校验 
    switch (ePkgType)
    {
        case ADM_DPM_PART_SRC_PACKAGE:
        case ADM_DPM_PART_DST_PACKAGE:
        {
            if (ADM_IS_NULL(pstDownPartPkg) || ADM_IS_NULL(pstDownPartPkg->ecuReqid) || ADM_IS_NULL(pstDownPartPkg->ecuSwDid))
            {
                log_e("Type[%d] Invalid input param", ePkgType);
                return ADM_ECODE_INVALID_PARAM;
            }
            // 若源包不存在则返回成功
            if ((ADM_DPM_PART_SRC_PACKAGE == ePkgType) && (NULL == pstDownPartPkg->pstSrcEcuPkgInfo))
            {
                return ADM_ECODE_SUCCESSED;
            }
            pstFileObject = ((ADM_DPM_PART_SRC_PACKAGE == ePkgType) ? \
                               (&pstDownPartPkg->pstSrcEcuPkgInfo->stPkgFileObj) : \
                               (&pstDownPartPkg->stDstEcuPkgInfo.stPkgFileObj));
            
            pcDownInstId = adm_ecs_getDownloadInstanceId(pstDownPartPkg->ecuReqid, pstDownPartPkg->ecuSwDid);
            if (ADM_IS_NULL(pcDownInstId))
            {
                log_e("Failed to get down instance id, reqID[%s], swDID[%s]", 
                        pstDownPartPkg->ecuReqid, pstDownPartPkg->ecuSwDid);
                return ADM_ECODE_NO_MEMORY;
            }

            pfileId = adm_dps_createDownFileIDByEcuInfo(
                                                    pstDownPartPkg->ecuReqid,
                                                    pstDownPartPkg->ecuSwDid,
                                                    (ADM_DPM_PART_SRC_PACKAGE == ePkgType) ? TRUE : FALSE,
                                                    pstFileObject->fileName);
            if (ADM_IS_NULL(pfileId))
            {
                log_e("Failed to create verify sign file id, err[%d]", ADM_ECODE_NO_MEMORY);
                return ADM_ECODE_NO_MEMORY;
            }
            log_d("Todo verify sign part package.");
            break;
        }
        case ADM_DPM_COMBINE_PACKAGE:
        {
            if (ADM_IS_NULL(pstCombinePkgFileObj))
            {
                log_e("Type[%d] Invalid input param", ePkgType);
                return ADM_ECODE_INVALID_PARAM;
            }
            pstFileObject = pstCombinePkgFileObj;
            pcDownInstId = adm_strdup(ADM_DPS_UCM_INSTID);
            log_d("Todo verify sign combine package.");
            break;
        }
        case ADM_DPM_COMBINE_SRC_ECU_PACKAGE:
        case ADM_DPM_COMBINE_DST_ECU_PACKAGE:
        {
            if (ADM_IS_NULL(pstCombinePkgFileObj) || ADM_IS_NULL(pstCombinePkgFileObj->url) || 
                ADM_IS_NULL(pstCombinePkgFileObj->fileName) || ADM_IS_NULL(pstCombineEcuIdInfo) || 
                ADM_IS_NULL(pstCombineEcuIdInfo->ecuReqid) || ADM_IS_NULL(pstCombineEcuIdInfo->ecuSwDid))
            {
                log_e("Type[%d] Invalid input param", ePkgType);
                return ADM_ECODE_INVALID_PARAM;
            }
            // 若源包不存在则返回成功
            if ((ADM_DPM_COMBINE_SRC_ECU_PACKAGE == ePkgType) && (NULL == pstCombineEcuIdInfo->psrcPkgFile))
            {
                return ADM_ECODE_SUCCESSED;
            }
            pstFileObject = ((ADM_DPM_COMBINE_SRC_ECU_PACKAGE == ePkgType) ? \
                   (pstCombineEcuIdInfo->psrcPkgFile) : \
                   (&pstCombineEcuIdInfo->dstPkgFile));

            pcDownInstId = adm_strdup(ADM_DPS_UCM_INSTID);
            log_d("Todo verify sign combine ecu package.");
            break;
        }
        default:
            log_e("Failed to switch package down state[%d].", ePkgType);
            return ADM_ECODE_INVALID_PARAM;
    }
    
    if (FALSE == adm_com_checkIsNeedToVerifySignature(pstFileObject))
    {
        //不需要验签
        iRet = ADM_ECODE_SUCCESSED;
        adm_free(pfileId);
        adm_free(pcDownInstId);
        log_d("conditions dissatisfy, dont need to Verify signature");
        return iRet;
    }

    //记录验签的开始时间
    (void)clock_gettime(CLOCK_MONOTONIC, &tsNowSignaturebegin);

    pstSignatureInfo = adm_com_newSignatureInfo();
    if (ADM_IS_NULL(pstSignatureInfo))
    {
        iRet = ADM_ECODE_NO_MEMORY;
        adm_free(pfileId);
        adm_free(pcDownInstId);
        log_e("Failed to malloc pstSignatureInfo info. err[%d]", ADM_ECODE_NO_MEMORY);
        return iRet;
    }
    
    do{

        if (gstDPMdownInfo.s32UCMDownInstId == (D32S)htoi(pcDownInstId))
        {
#ifdef SUPPORT_LION_PKI
            bIsPkiVerifySign = DTRUE;
#endif
            //!< 主控hots验签
            //!< 获取验证签名文件
            iRet = adm_com_getSignatureData(pstFileObject, bIsPkiVerifySign, pstSignatureInfo);
            if (ADM_IS_ERR(iRet))
            {
                iRet = ADM_ECODE_GET_SIGNATUREDATA_FAILED;
                log_e("Get Signature Data from package[%s] failed, err[%d]", pstFileObject->fileName, iRet);
                break; 
            }
            log_d("get Signature Data from package ok.");

            s32hostInstId = adm_cfg_iniParserGetInt(ADM_CFG_VUS_HOST_INSTANCEID,  ADM_CFG_VUS_HOST_INSTANCEID_DEFAULT_VAL);
            //dim验签接口
            iRet = adm_dim_host_verifySignature((D16U)s32hostInstId, ADM_DPM_SYNC_REQ_TIMEOUT, 
                                    pstSignatureInfo->signfileinfo, pstSignatureInfo->signfileinfolen, 
                                    pstSignatureInfo->beVerifiedHash, pstSignatureInfo->beVerifiedHashLen, &ierrorCode);
            if (ADM_IS_ERR(iRet) || ADM_IS_ERR(ierrorCode))
            {
                iRet = ADM_ECODE_VERIFY_SIGNATURE_FAILED;
                log_e("Verify signature failed, errorCode[%d], err[%d]", ierrorCode, iRet);
                break; 
            }
            
            log_d("verify down package signature data ok, signData[%s], signlen[%d], originhash[%s], originhashlen[%d]",
                pstSignatureInfo->signfileinfo, pstSignatureInfo->signfileinfolen, pstSignatureInfo->beVerifiedHash, pstSignatureInfo->beVerifiedHashLen);
        }
        else
        {
            //<! ucl远程验签接口
            iRet = adm_pub_downPackageVerifySign(pstFileObject, pcDownInstId, pfileId, FALSE, NULL);
            if (ADM_IS_ERR(iRet))
            {
                iRet = ADM_ECODE_VERIFY_SIGNATURE_FAILED;
                log_e("ucl Verify signature failed, err[%d]", iRet);
                break; 
            }
            log_d("ucl verify down package signature data ok");
        }
    }while (FALSE);

    if (ADM_IS_SUCCESS(iRet))
    {
        if ((ADM_DPM_PART_SRC_PACKAGE == ePkgType) || (ADM_DPM_PART_DST_PACKAGE == ePkgType))
        {
             __adm_dpm_decryptAndSignatureReport(ADM_EVENT_ID_DL_009, pstDownPartPkg->ecuReqid, pstDownPartPkg->ecuSwDid ,&tsNowSignaturebegin, ePkgType, pstFileObject);
        }
        else if (ADM_DPM_COMBINE_PACKAGE == ePkgType)
        {
            __adm_dpm_decryptAndSignatureReport(ADM_EVENT_ID_DL_010, NULL, NULL, &tsNowSignaturebegin, ePkgType, pstFileObject);
        }
        else
        {
            __adm_dpm_decryptAndSignatureReport(ADM_EVENT_ID_DL_009,pstCombineEcuIdInfo->ecuReqid, pstCombineEcuIdInfo->ecuSwDid, &tsNowSignaturebegin, ePkgType, pstFileObject);
        }
    }

    adm_free(pfileId);
    adm_free(pcDownInstId);
    adm_com_delSignatureInfo(&pstSignatureInfo);

    return iRet;

}

/*!
 *******************************************************************************
 * 解密并更新对象的文件对象信息
 *
 * \note
 * 
 * \param[in]     eDecryptPkgType       解密包类型
 * \param[inout]  pstDownPartPkg        零件包信息
 * \param[inout]  pstCombinePkgFileObj  集合包信息
 * \param[inout]  pstCombineEcuIdInfo   集合包中ecu信息
 * \retval        D32S
 *******************************************************************************
 */
static D32S adm_dpm_decryptAndUpdateFileObject
(
    ADM_DPM_DECRYPTPKG_TYPE_E  eDecryptPkgType,
    ADM_DPS_DOWN_ECUPARTPKG_T *pstDownPartPkg,
    ADM_COM_FILEOBJECT_T      *pstCombinePkgFileObj,
    ADM_DPS_ECUIDINFO_T       *pstCombineEcuIdInfo
)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    DCHAR *pcDecryptPath = NULL;
    DCHAR *pcFilePathBak = NULL;
    ADM_COM_FILEOBJECT_T *pstFileObj = NULL;
    DCHAR *pcDownInstId = NULL;
    struct timespec tsNowDecryptbegin;
    D32S iErrorCode = ADM_ECODE_SUCCESSED;

    //!< 参数校验 
    switch (eDecryptPkgType)
    {
        case ADM_DPM_PART_SRC_PACKAGE:
        case ADM_DPM_PART_DST_PACKAGE:
        {
            if (ADM_IS_NULL(pstDownPartPkg) || ADM_IS_NULL(pstDownPartPkg->ecuReqid) || ADM_IS_NULL(pstDownPartPkg->ecuSwDid))
            {
                log_e("Type[%d] Invalid input param", eDecryptPkgType);
                return ADM_ECODE_INVALID_PARAM;
            }
            // 若源包不存在则返回成功
            if ((ADM_DPM_PART_SRC_PACKAGE == eDecryptPkgType) && (NULL == pstDownPartPkg->pstSrcEcuPkgInfo))
            {
                return ADM_ECODE_SUCCESSED;
            }
            pstFileObj = ((ADM_DPM_PART_SRC_PACKAGE == eDecryptPkgType) ? \
                               (&pstDownPartPkg->pstSrcEcuPkgInfo->stPkgFileObj) : \
                               (&pstDownPartPkg->stDstEcuPkgInfo.stPkgFileObj));
            
            pcDownInstId = adm_ecs_getDownloadInstanceId(pstDownPartPkg->ecuReqid, pstDownPartPkg->ecuSwDid);
            if (ADM_IS_NULL(pcDownInstId))
            {
                log_e("Failed to get down instance id, reqID[%s], swDID[%s]", 
                        pstDownPartPkg->ecuReqid, pstDownPartPkg->ecuSwDid);
                return ADM_ECODE_NO_MEMORY;
            }
            log_d("Todo decrypt part package.");
            break;
        }
        case ADM_DPM_COMBINE_PACKAGE:
        {
            if (ADM_IS_NULL(pstCombinePkgFileObj))
            {
                log_e("Type[%d] Invalid input param", eDecryptPkgType);
                return ADM_ECODE_INVALID_PARAM;
            }
            pstFileObj = pstCombinePkgFileObj;
            pcDownInstId = adm_strdup(ADM_DPS_UCM_INSTID);
            log_d("Todo decrypt combine package.");
            break;
        }
        case ADM_DPM_COMBINE_SRC_ECU_PACKAGE:
        case ADM_DPM_COMBINE_DST_ECU_PACKAGE:
        {
            if (ADM_IS_NULL(pstCombinePkgFileObj) || ADM_IS_NULL(pstCombinePkgFileObj->url) || 
                ADM_IS_NULL(pstCombinePkgFileObj->fileName) || ADM_IS_NULL(pstCombineEcuIdInfo) || 
                ADM_IS_NULL(pstCombineEcuIdInfo->ecuReqid) || ADM_IS_NULL(pstCombineEcuIdInfo->ecuSwDid))
            {
                log_e("Type[%d] Invalid input param", eDecryptPkgType);
                return ADM_ECODE_INVALID_PARAM;
            }
            // 若源包不存在则返回成功
            if ((ADM_DPM_COMBINE_SRC_ECU_PACKAGE == eDecryptPkgType) && (NULL == pstCombineEcuIdInfo->psrcPkgFile))
            {
                return ADM_ECODE_SUCCESSED;
            }
            pstFileObj = ((ADM_DPM_COMBINE_SRC_ECU_PACKAGE == eDecryptPkgType) ? \
                   (pstCombineEcuIdInfo->psrcPkgFile) : \
                   (&pstCombineEcuIdInfo->dstPkgFile));

            pcDownInstId = adm_ecs_getDownloadInstanceId(pstCombineEcuIdInfo->ecuReqid, pstCombineEcuIdInfo->ecuSwDid);
            if (ADM_IS_NULL(pcDownInstId))
            {
                log_e("Failed to get down instance id, reqID[%s], swDID[%s]", 
                        pstCombineEcuIdInfo->ecuReqid, pstCombineEcuIdInfo->ecuSwDid);
                return ADM_ECODE_NO_MEMORY;
            }
            log_d("Todo decrypt combine ecu package.");
            break;
        }
        default:
            log_e("Failed to switch package down state[%d].", eDecryptPkgType);
            return ADM_ECODE_INVALID_PARAM;

    }

    log_d("package Decrypt, filePath[%s]",  pstFileObj->filePath);

    //!< 是否需要解密 
    if((NULL == pstFileObj->pstFileSecurityInfo) || (pstFileObj->pstFileSecurityInfo->isEncrypt == 0)) 
    {
        log_i("package dont need decrypt, file[%s]", pstFileObj->filePath);
        adm_free(pcDownInstId);
        return ADM_ECODE_SUCCESSED;
    }

    //记录解密的开始时间
    (void)clock_gettime(CLOCK_MONOTONIC, &tsNowDecryptbegin);
    //!< 解密
    iRet = adm_pub_downPackageDecrypt(pstFileObj, &pcDecryptPath, pcDownInstId);
    if (ADM_IS_ERR(iRet))
    {
        log_e("UCL decrypt package failed, fileName[%s], err[%d]", pstFileObj->fileName, iRet);
        adm_free(pcDownInstId);
        return ADM_ECODE_AES_ECB_DECRYPT_FAILED;
    }

    //!< 先备份源文件路径 
    pcFilePathBak = adm_strdup(pstFileObj->filePath);
    //!< 重置待解密的文件对象 --- 将加密的 fileObje 转成未加密的 fileObj
    (void)adm_dpm_changeFileObj2DecryptFileObj(pstFileObj, pcDecryptPath);
    adm_free(pcDecryptPath);

    //!< 更新到dps及kv
    if ((ADM_DPM_PART_SRC_PACKAGE == eDecryptPkgType) || (ADM_DPM_PART_DST_PACKAGE == eDecryptPkgType))
    {
        iRet = adm_dps_updatePartPackageDecryptFileObj(pstDownPartPkg->ecuReqid, 
                    pstDownPartPkg->ecuSwDid, 
                    ((ADM_DPM_PART_SRC_PACKAGE == eDecryptPkgType) ? (TRUE) : (FALSE)), 
                    pstFileObj);

        __adm_dpm_decryptAndSignatureReport(ADM_EVENT_ID_DL_007,pstDownPartPkg->ecuReqid, pstDownPartPkg->ecuSwDid,&tsNowDecryptbegin,eDecryptPkgType,pstFileObj);

    }
    else if (ADM_DPM_COMBINE_PACKAGE == eDecryptPkgType)
    {
        iRet = adm_dps_updateCombinePackageDecryptFileObj(pstFileObj);

        __adm_dpm_decryptAndSignatureReport(ADM_EVENT_ID_DL_008, NULL, NULL ,&tsNowDecryptbegin,eDecryptPkgType,pstFileObj);
    }
    else
    {
        iRet = adm_dps_updateCombinePackageEcuDecryptFileObj(pstCombinePkgFileObj->url,  
                    pstCombinePkgFileObj->fileName,
                    pstCombineEcuIdInfo->ecuReqid, 
                    pstCombineEcuIdInfo->ecuSwDid, 
                    ((ADM_DPM_COMBINE_SRC_ECU_PACKAGE == eDecryptPkgType) ? (TRUE) : (FALSE)), 
                    pstFileObj);  

        __adm_dpm_decryptAndSignatureReport(ADM_EVENT_ID_DL_007, pstCombineEcuIdInfo->ecuReqid, pstCombineEcuIdInfo->ecuSwDid,&tsNowDecryptbegin, eDecryptPkgType,pstFileObj);
    }
    if (ADM_IS_ERR(iRet))
    {
        log_e("Failed to update package decrypt file object to coreServer,filePath[%s], eDecryptPkgType[%d], err[%d]", 
                pstFileObj->filePath, eDecryptPkgType, iRet);
        adm_free(pcDownInstId);
        return ADM_ECODE_UPDATE_DECRYPTFILEOBJ_FAILED;
    }

    //!< 若解密前的文件与解密后的文件名(路径+文件名)不同，则将加密文件删除 
    if (0 != adm_strcmp(pcFilePathBak, pstFileObj->filePath))
    {
        if (gstDPMdownInfo.s32UCMDownInstId == (D32S)htoi(pcDownInstId))
        {
            (void)adm_fremove(pcFilePathBak);
        }
        else
        {
            (void)adm_dim_ucl_requestRmFile((D16U)htoi(pcDownInstId), 30, pcFilePathBak, &iErrorCode);
        }
    }

    adm_free(pcDownInstId);

    return ADM_ECODE_SUCCESSED;
}




/*!
 *******************************************************************************
 * 更新下载结果状态
 *
 * \note 
 * 
 * \param[in]   iResult            下载结果 
 * \param[in]   eDownState         下载状态
 * \param[in]   pstDpmDownReqParam dpm下载包参数 
 * \retval      none
 *******************************************************************************
 */
static void adm_dpm_updateDownResult
(
    D32S                    iResult, 
    ADM_DPS_DOWNPKGSTATE_E  eDownState, 
    ADM_DPM_DOWNREQPARAM_T *pstDpmDownReqParam
)
{
    ADM_DPS_DOWNRESULT_T *pstResult = NULL;

    if (ADM_IS_NULL(pstDpmDownReqParam))
    {
        log_e("Invalid input param");
        return;
    }

    //!< 更新下载结果
    if (NULL != pstDpmDownReqParam->fnDownResultNotify)
    {
        pstResult = adm_dpm_createDownResultInfo(iResult, &pstDpmDownReqParam->stPkgAttr);
        if (ADM_IS_NULL(pstResult))
        {
            log_e("Failed to create down result info, err[%d]", ADM_ECODE_NO_MEMORY);
            return;
        }
        pstResult->eDownState = eDownState;
        
        pstDpmDownReqParam->fnDownResultNotify(pstResult, NULL, &pstDpmDownReqParam->stPkgAttr);
        adm_dpm_freeDownResultInfo(pstResult);
    }

    return;
}


/*!
 *******************************************************************************
 * ecu part包下载后处理
 *
 * \note 升级包路径回写到 pis 
 * 
 * \param[in]   pstDownPartPkg     待下载ecu part包信息 
 * \retval      D32S
 *******************************************************************************
 */
static D32S adm_dpm_downPartPackagePost
(
    ADM_DPS_DOWN_ECUPARTPKG_T *pstDownPartPkg
)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    DCHAR szEventBuf[ADM_COM_LEN_256] = {0};  //mod by wl 20220321

    //mod by wl 20220321
    adm_dpm_addEventAndReport(ADM_COM_MOD_DPM, FALSE, ADM_EVENT_ID_DI_058, ADM_OCS_EVENT_LEVEL_DEBUG, "vehicle download part package post proc.", NULL);

    if (ADM_IS_NULL(pstDownPartPkg))
    {
        log_e("Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }

    //!< 源升级包存储路径更新到 pis 
    if ((NULL != pstDownPartPkg->pstSrcEcuPkgInfo))
    {
        iRet = adm_dps_updateEcuPkgSavePath2PIS(pstDownPartPkg->ecuReqid,
                    pstDownPartPkg->ecuSwDid,
                    TRUE,
                    &pstDownPartPkg->pstSrcEcuPkgInfo->stPkgFileObj);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to updateEcuPkgSavePath2PIS isDstPkg[%d] reqID[%s], swDID[%s], err:[%d]", 
                             FALSE, pstDownPartPkg->ecuReqid, pstDownPartPkg->ecuSwDid, iRet);
            //mod by wl 20220321
            (void)adm_memzero(szEventBuf, sizeof(szEventBuf));
            (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), 
                    "Failed to updateEcuPkgSavePath2PIS isDstPkg[%d] reqID[%s], swDID[%s], err[%d]", 
                    FALSE, pstDownPartPkg->ecuReqid, pstDownPartPkg->ecuSwDid, iRet);
            adm_dpm_addEventAndReport(ADM_COM_MOD_DPM, TRUE, ADM_EVENT_ID_DI_059, ADM_OCS_EVENT_LEVEL_ERROR, 
                    "vehicle download part package post proc failed!", szEventBuf);
            return iRet;
        }
    }
    
    //!< 目标升级包存储路径更新到 pis 
    iRet = adm_dps_updateEcuPkgSavePath2PIS(pstDownPartPkg->ecuReqid,
                pstDownPartPkg->ecuSwDid,
                FALSE,
                &pstDownPartPkg->stDstEcuPkgInfo.stPkgFileObj);
    if (ADM_IS_ERR(iRet))
    {
        log_e("Failed to updateEcuPkgSavePath2PIS isDstPkg[%d] reqID[%s], swDID[%s], err:[%d]", 
                         TRUE, pstDownPartPkg->ecuReqid, pstDownPartPkg->ecuSwDid, iRet);
        //mod by wl 20220321
        (void)adm_memzero(szEventBuf, sizeof(szEventBuf));
        (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), 
                    "Failed to updateEcuPkgSavePath2PIS isDstPkg[%d] reqID[%s], swDID[%s], err[%d]", 
                    TRUE, pstDownPartPkg->ecuReqid, pstDownPartPkg->ecuSwDid, iRet);
        adm_dpm_addEventAndReport(ADM_COM_MOD_DPM, TRUE, ADM_EVENT_ID_DI_059, ADM_OCS_EVENT_LEVEL_ERROR, 
                    "vehicle download part package post proc failed!", szEventBuf);
        return iRet;
    }
    //mod by wl 20220321
    adm_dpm_addEventAndReport(ADM_COM_MOD_DPM, FALSE, ADM_EVENT_ID_DI_060, ADM_OCS_EVENT_LEVEL_DEBUG, 
                "vehicle download part package post proc successed.", NULL);
    
    return iRet;
}


/*!
 *******************************************************************************
 * ecu part包下载处理
 *
 * \note 一定要先下载源包再下载目标包(有源包的情况下) 
 * 
 * \param[in]   pcDownInstId       零件文件下载的实例ID
 * \param[in]   pstDownPartPkg     待下载ecu part包信息 
 * \param[in]   pstDpmDownReqParam dpm下载包参数 
 * \retval      D32S
 *******************************************************************************
 */
static D32S adm_dpm_downloadPartPackage
(
    DCHAR                     *pcDownInstId,
    ADM_DPS_DOWN_ECUPARTPKG_T *pstDownPartPkg,
    ADM_DPM_DOWNREQPARAM_T    *pstDpmDownReqParam
)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    DCHAR szEventBuf[ADM_COM_LEN_256] = {0};  //mod by wl 20220321

    if (ADM_IS_NULL(pcDownInstId) || ADM_IS_NULL(pstDownPartPkg) || ADM_IS_NULL(pstDpmDownReqParam))
    {
        log_e("Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }
    log_d("Todo downloading one package, downInstanceId[%s].", pcDownInstId);

    //!< 下载源包 --- 源包存在且源包没有下载 
    if ((NULL != pstDownPartPkg->pstSrcEcuPkgInfo) && 
        (ADM_DPS_DOWNPKG_STATE_SRCFINISH > pstDownPartPkg->stDownProcess.stResult.eDownState))
    {
        //mod by wl 20220321
        (void)adm_memzero(szEventBuf, sizeof(szEventBuf));
        (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), 
                "down src pkg, reqId[%s], swDid[%s], url[%s], size[%lld], path[%s], hash[%s]", 
                pstDownPartPkg->ecuReqid, 
                pstDownPartPkg->ecuSwDid, 
                pstDownPartPkg->pstSrcEcuPkgInfo->stPkgFileObj.url,
                pstDownPartPkg->pstSrcEcuPkgInfo->stPkgFileObj.size,
                pstDownPartPkg->pstSrcEcuPkgInfo->stPkgFileObj.filePath,
                pstDownPartPkg->pstSrcEcuPkgInfo->stPkgFileObj.hash);
        adm_dpm_addEventAndReport(ADM_COM_MOD_DPM, FALSE, ADM_EVENT_ID_DI_049, ADM_OCS_EVENT_LEVEL_DEBUG, 
                    "vehicle download once src part package.", szEventBuf);

        log_d("Todo downloading src package.");
        //!< 创建下载所需参数
        __adm_dpm_cleanDPMDownReqParam(pstDpmDownReqParam);
        iRet = __adm_dpm_fillPartPkgDPMDownReqParam(ADM_PARTPKG_TYPE_SRC, 
                                            pstDownPartPkg, pstDpmDownReqParam);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to fill part pkg dpm down req param, err[%d]", iRet);
            //mod by wl 20220321
            (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), 
                    "Failed to fill part pkg dpm down req param, url[%s], err[%d]", 
                    pstDownPartPkg->pstSrcEcuPkgInfo->stPkgFileObj.url, iRet);
            adm_dpm_addEventAndReport(ADM_COM_MOD_DPM, TRUE, ADM_EVENT_ID_DI_050, ADM_OCS_EVENT_LEVEL_ERROR, 
                    "vehicle download once src part package failed!", szEventBuf);
            return iRet;
        }

        if (ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_USB == gstDPMdownInfo.eTriggerType)
        {
            iRet = adm_dpm_usbDownload(pcDownInstId, 
                                       pstDpmDownReqParam, 
                                       pstDownPartPkg->pstSrcEcuPkgInfo->stPkgFileObj.fileName);
        }
        else 
        {
            iRet = adm_dpm_Download(pcDownInstId, pstDpmDownReqParam);
        }
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to down part pkg downInstId[%s], fileID[%s], err:[%d]", 
                                    pcDownInstId, pstDpmDownReqParam->fileID, iRet);
            //mod by wl 20220321
            (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), 
                    "Failed to down part pkg downInstId[%s], fileID[%s], url[%s], err[%d]",
                    pcDownInstId, pstDpmDownReqParam->fileID, pstDpmDownReqParam->url, iRet);
            adm_dpm_addEventAndReport(ADM_COM_MOD_DPM, TRUE, ADM_EVENT_ID_DI_050, ADM_OCS_EVENT_LEVEL_ERROR, 
                    "vehicle download once src part package failed!", szEventBuf);

            return iRet; 
        }
        //!< 若是slave下载则会更改原设定的下载路径, 此处更正缓存的路径
        adm_free(pstDownPartPkg->pstSrcEcuPkgInfo->stPkgFileObj.filePath);
        pstDownPartPkg->pstSrcEcuPkgInfo->stPkgFileObj.filePath = adm_strdup(pstDpmDownReqParam->filePath);
        
        log_d("Download src package ok.");
        //mod by wl 20220321
        (void)adm_memzero(szEventBuf, sizeof(szEventBuf));
        (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), 
                "down src pkg, reqId[%s], swDid[%s], url[%s], size[%lld], path[%s], hash[%s]", 
                pstDownPartPkg->ecuReqid, 
                pstDownPartPkg->ecuSwDid, 
                pstDownPartPkg->pstSrcEcuPkgInfo->stPkgFileObj.url,
                pstDownPartPkg->pstSrcEcuPkgInfo->stPkgFileObj.size,
                pstDownPartPkg->pstSrcEcuPkgInfo->stPkgFileObj.filePath,
                pstDownPartPkg->pstSrcEcuPkgInfo->stPkgFileObj.hash);
        adm_dpm_addEventAndReport(ADM_COM_MOD_DPM, FALSE, ADM_EVENT_ID_DI_051, ADM_OCS_EVENT_LEVEL_DEBUG, "vehicle download once src part package successed.", szEventBuf);
    }

    //mod by wl 20220321
    (void)adm_memzero(szEventBuf, sizeof(szEventBuf));
    (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), 
            "down dst pkg, reqId[%s], swDid[%s], url[%s], size[%lld], path[%s], hash[%s]", 
            pstDownPartPkg->ecuReqid, 
            pstDownPartPkg->ecuSwDid, 
            pstDownPartPkg->stDstEcuPkgInfo.stPkgFileObj.url,
            pstDownPartPkg->stDstEcuPkgInfo.stPkgFileObj.size,
            pstDownPartPkg->stDstEcuPkgInfo.stPkgFileObj.filePath,
            pstDownPartPkg->stDstEcuPkgInfo.stPkgFileObj.hash);
    adm_dpm_addEventAndReport(ADM_COM_MOD_DPM, FALSE, ADM_EVENT_ID_DI_052, ADM_OCS_EVENT_LEVEL_DEBUG, "vehicle download once dst part package", szEventBuf);
    
    //!< 下载目的包
    log_d("Todo downloading dst package.");
    //!< 创建下载所需参数
    __adm_dpm_cleanDPMDownReqParam(pstDpmDownReqParam);
    iRet = __adm_dpm_fillPartPkgDPMDownReqParam(ADM_PARTPKG_TYPE_DST, 
                        pstDownPartPkg, pstDpmDownReqParam);
    if (ADM_IS_ERR(iRet))
    {
        log_e("Failed to fill part pkg dpm down req param, err[%d]", iRet);
        //mod by wl 20220321
        (void)adm_memzero(szEventBuf, sizeof(szEventBuf));
        (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), 
                "Failed to fill part pkg dpm down req param, url[%s], err[%d]", 
                pstDownPartPkg->stDstEcuPkgInfo.stPkgFileObj.url, iRet);
        adm_dpm_addEventAndReport(ADM_COM_MOD_DPM, TRUE, ADM_EVENT_ID_DI_053, ADM_OCS_EVENT_LEVEL_ERROR, 
                "vehicle download once dst part package failed!", szEventBuf);

        return iRet;
    }

    if (ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_USB == gstDPMdownInfo.eTriggerType)
    {
        iRet = adm_dpm_usbDownload(pcDownInstId, 
                                   pstDpmDownReqParam,
                                   pstDownPartPkg->stDstEcuPkgInfo.stPkgFileObj.fileName);
    }
    else 
    {
        iRet = adm_dpm_Download(pcDownInstId, pstDpmDownReqParam);
    }
    if (ADM_IS_ERR(iRet))
    {
        log_e("Failed to down part pkg downInstId[%s], fileID[%s], err:[%d]", 
                                pcDownInstId, pstDpmDownReqParam->fileID, iRet);
        //mod by wl 20220321
        (void)adm_memzero(szEventBuf, sizeof(szEventBuf));
        (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), 
                "Failed to down part pkg downInstId[%s], fileID[%s], url[%s], err[%d]",
                pcDownInstId, pstDpmDownReqParam->fileID, pstDpmDownReqParam->url, iRet);
        adm_dpm_addEventAndReport(ADM_COM_MOD_DPM, TRUE, ADM_EVENT_ID_DI_053, ADM_OCS_EVENT_LEVEL_ERROR, "vehicle download once dst part package failed!", szEventBuf);

        return iRet; 
    }
    //!< 若是slave下载则会更改原设定的下载路径, 此处更正缓存的路径
    adm_free(pstDownPartPkg->stDstEcuPkgInfo.stPkgFileObj.filePath);
    pstDownPartPkg->stDstEcuPkgInfo.stPkgFileObj.filePath = adm_strdup(pstDpmDownReqParam->filePath);
    log_d("Download dst package ok.");
    //mod by wl 20220321
    (void)adm_memzero(szEventBuf, sizeof(szEventBuf));
    (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), 
            "down dst pkg, reqId[%s], swDid[%s], url[%s], size[%lld], path[%s], hash[%s]", 
            pstDownPartPkg->ecuReqid, 
            pstDownPartPkg->ecuSwDid, 
            pstDownPartPkg->stDstEcuPkgInfo.stPkgFileObj.url,
            pstDownPartPkg->stDstEcuPkgInfo.stPkgFileObj.size,
            pstDownPartPkg->stDstEcuPkgInfo.stPkgFileObj.filePath,
            pstDownPartPkg->stDstEcuPkgInfo.stPkgFileObj.hash);
    adm_dpm_addEventAndReport(ADM_COM_MOD_DPM, FALSE, ADM_EVENT_ID_DI_054, ADM_OCS_EVENT_LEVEL_DEBUG, "vehicle download once dst part package successed.", szEventBuf);

    return iRet;
}

/*!
 *******************************************************************************
 * 解密下载的零件包信息
 *
 * \note
 * 
 * \param[in]   pstDownCombinePkg 已下载ecu part包信息
 * \retval      D32S
 *******************************************************************************
 */
static D32S adm_dpm_downPartPackageDecrypt
(
    ADM_DPS_DOWN_ECUPARTPKG_T *pstDownPartPkg
)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    DCHAR szEventBuf[ADM_COM_LEN_256] = {0};  //mod by wl 20220321

    //mod by wl 20220321
    adm_dpm_addEventAndReport(ADM_COM_MOD_DPM, FALSE, ADM_EVENT_ID_DI_055, ADM_OCS_EVENT_LEVEL_DEBUG, "vehicle decrypt download part package.", NULL);

    if (ADM_IS_NULL(pstDownPartPkg))
    {
        log_e("Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }

    //!< 解密原包
    iRet = adm_dpm_decryptAndUpdateFileObject(ADM_DPM_PART_SRC_PACKAGE, pstDownPartPkg, NULL, NULL);
    if (ADM_IS_ERR(iRet))
    {
        log_e("UCL decrypt package failed, fileName[%s], err[%d]", pstDownPartPkg->pstSrcEcuPkgInfo->stPkgFileObj.fileName, iRet);
    
        //mod by wl 20220321
        (void)adm_memzero(szEventBuf, sizeof(szEventBuf));
        (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), 
                "UCL decrypt package failed, fileName[%s], err[%d]", pstDownPartPkg->pstSrcEcuPkgInfo->stPkgFileObj.fileName, iRet);
        adm_dpm_addEventAndReport(ADM_COM_MOD_DPM, TRUE, ADM_EVENT_ID_DI_056, ADM_OCS_EVENT_LEVEL_ERROR, "vehicle decrypt download part src package failed!", szEventBuf);
        return iRet;
    }

    //!< 解密目标文件 
    iRet = adm_dpm_decryptAndUpdateFileObject(ADM_DPM_PART_DST_PACKAGE, pstDownPartPkg, NULL, NULL);
    if (ADM_IS_ERR(iRet))
    {
        log_e("UCL decrypt package failed, fileName[%s], err[%d]", pstDownPartPkg->stDstEcuPkgInfo.stPkgFileObj.fileName, iRet);
    
        //mod by wl 20220321
        (void)adm_memzero(szEventBuf, sizeof(szEventBuf));
        (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), 
                "UCL decrypt package failed, fileName[%s], err[%d]", pstDownPartPkg->stDstEcuPkgInfo.stPkgFileObj.fileName, iRet);
        adm_dpm_addEventAndReport(ADM_COM_MOD_DPM, TRUE, ADM_EVENT_ID_DI_056, ADM_OCS_EVENT_LEVEL_ERROR, "vehicle decrypt download part src package failed!", szEventBuf);
        return iRet;
    }

    //mod by wl 20220321
    adm_dpm_addEventAndReport(ADM_COM_MOD_DPM, FALSE, ADM_EVENT_ID_DI_057, ADM_OCS_EVENT_LEVEL_DEBUG, "vehicle decrypt download part package successed.", NULL);

    return ADM_ECODE_SUCCESSED;
}


/*!
 *******************************************************************************
 * 下载处理一个ecu的零件包
 *
 * \note 同步接口
 * 
 * \param[in]   pstDownPartPkg     待下载ecu part包信息 
 * \param[in]   pstDpmDownReqParam dpm下载包参数 
 * \retval      D32S
 *******************************************************************************
 */
static D32S adm_dpm_downOncePartPackageProc
(
    ADM_DPS_DOWN_ECUPARTPKG_T *pstDownPartPkg,
    ADM_DPM_DOWNREQPARAM_T    *pstDpmDownReqParam
)
{
    D32S iRet = ADM_ECODE_INVALID_PARAM;
    DCHAR *pcDownInstId = NULL;

    if (ADM_IS_NULL(pstDownPartPkg) || ADM_IS_NULL(pstDpmDownReqParam))
    {
        log_e("Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }

    pcDownInstId = adm_ecs_getDownloadInstanceId(pstDownPartPkg->ecuReqid, pstDownPartPkg->ecuSwDid);
    if (ADM_IS_NULL(pcDownInstId))
    {
        log_e("Failed to get down instance id, reqID[%s], swDID[%s]", 
                pstDownPartPkg->ecuReqid, pstDownPartPkg->ecuSwDid);
        return ADM_ECODE_INVALID_POINTER;
    }
    log_d("Todo downloading part package.");

    //!< 创建下载所需参数
    __adm_dpm_cleanDPMDownReqParam(pstDpmDownReqParam);
    iRet = __adm_dpm_fillPartPkgDPMDownReqParam(ADM_PARTPKG_TYPE_DST, pstDownPartPkg, pstDpmDownReqParam);
    if (ADM_IS_ERR(iRet))
    {
        adm_free(pcDownInstId);
        log_e("Failed to fill paart pkg dpm down req param, err[%d]", iRet);
        return iRet;    
    }

    switch (pstDownPartPkg->stDownProcess.stResult.eDownState)
    {
        case ADM_DPS_DOWNPKG_STATE_NONE:
        case ADM_DPS_DOWNPKG_STATE_NORMAL:
        case ADM_DPS_DOWNPKG_STATE_PAUSE:
        case ADM_DPS_DOWNPKG_STATE_CANCEL:
        case ADM_DPS_DOWNPKG_STATE_FAILED:
        case ADM_DPS_DOWNPKG_STATE_SRCFINISH:
        {
            //!< 下载升级包
            log_i("[ABUP-Procedure] Download part package Start,reqid[%s],swdid[%s]",pstDownPartPkg->ecuReqid,pstDownPartPkg->ecuSwDid);
            iRet = adm_dpm_downloadPartPackage(pcDownInstId, pstDownPartPkg, pstDpmDownReqParam);
            if (ADM_IS_ERR(iRet))
            {
                log_e("Download part package[%s][%s] failed, err:[%d]", pstDownPartPkg->ecuReqid, pstDownPartPkg->ecuSwDid, iRet);
                log_i("[ABUP-Procedure] Download part package End,ret[%d]",iRet);
                break; 
            }
            log_i("[ABUP-Procedure] Download part package End,ret[%d]",iRet);
            log_d("Download part package ok.");
        } //!< 此处 case 不需要      break, 下载完成后需要继续后续的解密、解压和解析等处理
        case ADM_DPS_DOWNPKG_STATE_WAITDECRYPT:
        case ADM_DPS_DOWNPKG_STATE_DECRYPTFAILED:
        {
            log_d("Entry part package decrypt.");
            //!< 解密升级包
            iRet = adm_dpm_downPartPackageDecrypt(pstDownPartPkg);
            adm_dpm_updateDownResult(iRet, 
                                     (ADM_IS_SUCCESS(iRet) ? (ADM_DPS_DOWNPKG_STATE_CHECKSIGN) : (ADM_DPS_DOWNPKG_STATE_DECRYPTFAILED)), 
                                     pstDpmDownReqParam);
            if (ADM_IS_ERR(iRet))
            {
                log_e("Decrypt part package[%s][%s] failed, err[%d]", pstDownPartPkg->ecuReqid, pstDownPartPkg->ecuSwDid, iRet);
                iRet = ADM_ECODE_UPDATE_DECRYPTFILEOBJ_FAILED;
                break; 
            }
            log_d("Download part package decrypt ok.");
        }
        //mod by wl 20220411
        case ADM_DPS_DOWNPKG_STATE_CHECKSIGN:
        case ADM_DPS_DOWNPKG_STATE_CHECKSIGNFAILED:
        {
            log_d("Entry part package verify signature.");
            //验签
            iRet = __adm_dpm_partpkgSignature(pstDownPartPkg);
            adm_dpm_updateDownResult(iRet, 
                    (ADM_IS_SUCCESS(iRet) ? (ADM_DPS_DOWNPKG_STATE_WAITPARSE) : (ADM_DPS_DOWNPKG_STATE_CHECKSIGNFAILED)), 
                    pstDpmDownReqParam);
            if (ADM_IS_ERR(iRet))
            {
                log_e("Verify part package signature failed, err[%d]", iRet);
                break; 
                iRet = ADM_ECODE_VERIFY_SIGNATURE_FAILED;
            }
            log_d("Verify part package signature ok.");
        }
        case ADM_DPS_DOWNPKG_STATE_WAITPARSE:
        case ADM_DPS_DOWNPKG_STATE_PARSEFAILED:
        {
            iRet = adm_dpm_downPartPackagePost(pstDownPartPkg); 
            adm_dpm_updateDownResult(iRet, 
                                     (ADM_IS_SUCCESS(iRet) ? (ADM_DPS_DOWNPKG_STATE_FINISH) : (ADM_DPS_DOWNPKG_STATE_PARSEFAILED)), 
                                     pstDpmDownReqParam);
            if (ADM_IS_ERR(iRet))
            {
                log_e("Post download part package[%s][%s] failed, err[%d]", pstDownPartPkg->ecuReqid, pstDownPartPkg->ecuSwDid, iRet);
                break; 
            }
            log_d("Download part package post ok.");
            break;
        }
        default:
            iRet = ADM_ECODE_INVALID_PARAM;
            log_e("Failed to switch package[%s][%s] down state[%d].", pstDownPartPkg->ecuReqid, 
                        pstDownPartPkg->ecuSwDid, pstDownPartPkg->stDownProcess.stResult.eDownState);
            break;
    }
    adm_free(pcDownInstId);
    return iRet;
}

/*!
 *******************************************************************************
 * 设置监控异常状态.
 *
 * \note    设置监控状态 ADM_DPM_MONITOR_ABNORMAL
 * 
 * \param   none
 * \retval  none
 *******************************************************************************
 */
void adm_dpm_setMonitorAbnormal(void)
{
    (void)pthread_rwlock_wrlock(&gstDPMdownInfo.stRWlock);
    gstDPMdownInfo.eMonitorState = ADM_DPM_MONITOR_ABNORMAL;
    (void)pthread_rwlock_unlock(&gstDPMdownInfo.stRWlock);
}


/*!
 *******************************************************************************
 * 下载监控异常是否异常
 *
 * \note    异常：返回ADM_ECODE_FAILED ； 无异常返回：ADM_ECODE_SUCCESSED.
 * 
 * \param[in]   
 * \retval      D32S
 *******************************************************************************
 */

static D32S adm_dpm_checkDownIsAbnormal(void)
{
    D32S iRet = ADM_ECODE_SUCCESSED;   //默认监控无异常

    (void)pthread_rwlock_rdlock(&gstDPMdownInfo.stRWlock);
    if (ADM_DPM_MONITOR_ABNORMAL == gstDPMdownInfo.eMonitorState)
    {
        iRet = ADM_ECODE_MONITOR_ABNORMAL_DOWN_BREAK;
    }
    else if(ADM_DPM_PAUSE_DOWNLOAD == gstDPMdownInfo.eMonitorState)
    {
        iRet = ADM_ECODE_PAUSE_DOWNLOAD;
    }
    else if(ADM_DPM_CANCEL_DOWNLOAD == gstDPMdownInfo.eMonitorState)
    {
        iRet = ADM_ECODE_CANCEL_DOWNLOAD;
    }
    else
    {
        ;/* 无任何必要行为 - ";"是可选的 */
    }

    (void)pthread_rwlock_unlock(&gstDPMdownInfo.stRWlock);
    
    return iRet;
}

/*!
 *******************************************************************************
 * 下载处理ecu part包
 *
 * \note    
 * 
 * \param[in]   pstDpmDownReqParam   dpm下载请求参数信息
 * \retval      D32S
 *******************************************************************************
 */
static D32S adm_dpm_downloadPartPackageProc(ADM_DPM_DOWNREQPARAM_T *pstDpmDownReqParam)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    D32S i = 0;
    D32S tobeDownloadNum = 0;
    ADM_DPS_DOWN_ECUPARTPKG_T *pstDownPartPkgList = NULL;
    DCHAR szEventBuf[ADM_COM_LEN_256] = {0}; 
    struct timespec tsNowbegin;
    struct timespec tsNowend;
    D32S packageNum = 0;

    if (ADM_IS_NULL(pstDpmDownReqParam))
    {
        log_e("Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }

    pstDownPartPkgList = adm_dps_newEcuPartPackageList(ADM_DPM_DEFDOWN_GETTOBELIST_CNT);
    if (ADM_IS_NULL(pstDownPartPkgList))
    {
        log_e("Failed to new ecu part package list");
        return ADM_ECODE_NO_MEMORY;
    }

    //!< 记录零件包下载的起始时间
    (void)clock_gettime(CLOCK_MONOTONIC, &tsNowbegin);
    //!< 循环遍历待下载包列表下载包
    do
    {
        //mod by wl 20220321
        (void)adm_memzero(szEventBuf, sizeof(szEventBuf));
        adm_dpm_addEventAndReport(ADM_COM_MOD_DPM, FALSE, ADM_EVENT_ID_DI_046, ADM_OCS_EVENT_LEVEL_DEBUG, "vehicle get tobe download ecu part package.", NULL);
        
        iRet = adm_dps_getTobeDownloadEcuPartPackage(ADM_DPM_DEFDOWN_GETTOBELIST_CNT, 
                    pstDownPartPkgList, &tobeDownloadNum);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to get tobe download ecu part package, err[%d]", iRet);
            (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), 
                    "Failed to get tobe download ecu part package, err[%d]", iRet);
            adm_dpm_addEventAndReport(ADM_COM_MOD_DPM, TRUE, ADM_EVENT_ID_DI_047, ADM_OCS_EVENT_LEVEL_ERROR, 
                    "vehicle get tobe download ecu part package failed!", szEventBuf);
            break;
        }
        //!< 记录零件包总个数
        packageNum += tobeDownloadNum;
        log_d("Tobe download part package Num[%d]", tobeDownloadNum);
        (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), "Tobe download part package Num[%d]", tobeDownloadNum);
        adm_dpm_addEventAndReport(ADM_COM_MOD_DPM, FALSE, ADM_EVENT_ID_DI_048, ADM_OCS_EVENT_LEVEL_DEBUG, 
                "vehicle get tobe download ecu part package successed.", szEventBuf);

        for (i = 0; i < tobeDownloadNum; i++)
        {
            //下载过程监控到异常信息需要中断下载 mod by wl 20220328 
            iRet = adm_dpm_checkDownIsAbnormal();
            if (ADM_IS_ERR(iRet))
            {
                log_e("Notify down monitor abnormal failed to download part package, reqId[%s], swId[%s], err[%d]", 
                        pstDownPartPkgList[i].ecuReqid, pstDownPartPkgList[i].ecuSwDid, iRet);
                break;
            }

            //!< 下载单个ecu的所有包信息
            iRet = adm_dpm_downOncePartPackageProc(&pstDownPartPkgList[i], pstDpmDownReqParam);
            if (ADM_IS_ERR(iRet))
            {
                log_e("Failed to download part package, reqId[%s], swId[%s], err[%d]", 
                        pstDownPartPkgList[i].ecuReqid, pstDownPartPkgList[i].ecuSwDid, iRet);
                break;
            }
        }
        //!< 若for-break，则while-break
        if (ADM_IS_ERR(iRet))
        {
            break;
        }

        //!< 清空本次的内存脏数据
        adm_dps_cleanEcuPartPackageList(ADM_DPM_DEFDOWN_GETTOBELIST_CNT, pstDownPartPkgList);
    } while (0 < tobeDownloadNum);

    if (ADM_IS_SUCCESS(iRet))
    {
        //!< 记录零件包下载的结束时间
        (void)clock_gettime(CLOCK_MONOTONIC, &tsNowend);
        (void)adm_memzero(szEventBuf, sizeof(szEventBuf));
        (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), "STATSPACK downAllTime:downTime[%d]%s, ecuNum[%d], pkgType[2](1: combine, 2: part)", 
        ((tsNowend.tv_sec - tsNowbegin.tv_sec) < 1 ? (tsNowend.tv_nsec / (1000 * 1000)) - (tsNowbegin.tv_nsec / (1000 * 1000)) : (tsNowend.tv_sec - tsNowbegin.tv_sec)),
        ((tsNowend.tv_sec - tsNowbegin.tv_sec) < 1 ? "ms" : "s"),
        packageNum);
        adm_dpm_addEventAndReport(ADM_COM_MOD_DPM, FALSE, ADM_EVENT_ID_DL_005, ADM_OCS_EVENT_LEVEL_INFO, szEventBuf, NULL);
    }

    adm_dps_delEcuPartPackageList(ADM_DPM_DEFDOWN_GETTOBELIST_CNT, &pstDownPartPkgList);
    return iRet;
}

/*!
 *******************************************************************************
 * 解密下载的集合包信息
 *
 * \note
 * 
 * \param[in]   pstDownCombinePkg 已下载集合包信息
 * \retval      D32S
 *******************************************************************************
 */
static D32S adm_dpm_downCombinePackageDecrypt
(
    ADM_DPS_DOWN_ECUCOMBINEPKG_T *pstDownCombinePkg
)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    DCHAR  szEventBuf[ADM_COM_LEN_256] = {0};  //mod by wl 20220321

    if (ADM_IS_NULL(pstDownCombinePkg))
    {
        log_e("Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }

    //!< 进入解压准备 --- 解密
    adm_dpm_setDownloadUnpackPrepare();

    //mod by wl 20220321
    (void)adm_memzero(szEventBuf, sizeof(szEventBuf));
    (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), "file[%s/%s] isEncrypt[%d]", 
            pstDownCombinePkg->stPkgFileObj.filePath, pstDownCombinePkg->stPkgFileObj.fileName, 
            ((NULL != pstDownCombinePkg->stPkgFileObj.pstFileSecurityInfo) ? (pstDownCombinePkg->stPkgFileObj.pstFileSecurityInfo->isEncrypt) : (0)));
    adm_dpm_addEventAndReport(ADM_COM_MOD_DPM, FALSE, ADM_EVENT_ID_DI_016, ADM_OCS_EVENT_LEVEL_DEBUG, "vehicle decrypt download combine package.", szEventBuf);

    log_e("pstDownCombinePkg{%s}", szEventBuf);

    
    //!< 解密  
    iRet = adm_dpm_decryptAndUpdateFileObject(ADM_DPM_COMBINE_PACKAGE, NULL, &pstDownCombinePkg->stPkgFileObj, NULL);
    if (ADM_IS_ERR(iRet))
    {
        log_e("UCL decrypt combine package failed, fileName[%s], err[%d]", pstDownCombinePkg->stPkgFileObj.fileName, iRet);

        (void)adm_memzero(szEventBuf, sizeof(szEventBuf));
        (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), 
                "UCL decrypt combine package failed, fileName[%s], err[%d]", pstDownCombinePkg->stPkgFileObj.fileName, iRet);
        adm_dpm_addEventAndReport(ADM_COM_MOD_DPM, TRUE, ADM_EVENT_ID_DI_017, ADM_OCS_EVENT_LEVEL_ERROR, "vehicle decrypt download combine package failed!", szEventBuf);
        return iRet;
    }

    adm_dpm_addEventAndReport(ADM_COM_MOD_DPM, FALSE, ADM_EVENT_ID_DI_018, ADM_OCS_EVENT_LEVEL_DEBUG, "vehicle decrypt download combine package successed.", NULL);

    return iRet;
}

/*!
 *******************************************************************************
 * 下载的集合包中ecu包解密和验签
 *
 * \note
 * 
 * \param[in]   pstCombinePkgFileObj 集合包文件对象信息
 * \param[in]   pstEcuIdList         集合包中ecu列表信息
 * \retval      D32S
 *******************************************************************************
 */
static D32S adm_dpm_downCombinePackageEcuDecryptAndVerifySign
(
    ADM_COM_FILEOBJECT_T *pstCombinePkgFileObj,
    ADM_DPS_ECUIDLIST_T  *pstEcuIdList
)
{
    D32S i = 0;
    D32S iRet = ADM_ECODE_SUCCESSED;

    if (ADM_IS_NULL(pstCombinePkgFileObj) || ADM_IS_NULL(pstEcuIdList))
    {
        log_e("Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }

    log_d("Decrypt and Verify sign combine package ecu file.");
    for (i = 0; i < pstEcuIdList->num; i++)
    {
        //!<解密目的包
        iRet = adm_dpm_decryptAndUpdateFileObject(ADM_DPM_COMBINE_DST_ECU_PACKAGE, NULL, 
                    pstCombinePkgFileObj, &pstEcuIdList->pstEcuIdInfo[i]);
        if (ADM_IS_ERR(iRet))
        {
            log_e("UCL decrypt package failed, ecuReqid[%s], ecuSwDid[%s], dstfileName[%s], err[%d]", 
                    pstEcuIdList->pstEcuIdInfo[i].ecuReqid, 
                    pstEcuIdList->pstEcuIdInfo[i].ecuSwDid, 
                    pstEcuIdList->pstEcuIdInfo[i].dstPkgFile.fileName, iRet);
            return iRet;
        }

        //!<验签目的包
                iRet = adm_dpm_signatureVerification(ADM_DPM_COMBINE_DST_ECU_PACKAGE, NULL, 
                    pstCombinePkgFileObj, &pstEcuIdList->pstEcuIdInfo[i]);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Verify signature failed, ecuReqid[%s], ecuSwDid[%s], dstfileName[%s], err[%d]", 
                    pstEcuIdList->pstEcuIdInfo[i].ecuReqid, 
                    pstEcuIdList->pstEcuIdInfo[i].ecuSwDid, 
                    pstEcuIdList->pstEcuIdInfo[i].dstPkgFile.fileName, iRet);
            return iRet;
        }

        //!< 若存在源包，解密、验签原包
        if (NULL != pstEcuIdList->pstEcuIdInfo[i].psrcPkgFile)
        {
            iRet = adm_dpm_decryptAndUpdateFileObject(ADM_DPM_COMBINE_SRC_ECU_PACKAGE, NULL, 
                        pstCombinePkgFileObj, &pstEcuIdList->pstEcuIdInfo[i]);
            if (ADM_IS_ERR(iRet))
            {
                log_e("UCL decrypt package failed, ecuReqid[%s], ecuSwDid[%s], srcfileName[%s], err[%d]", 
                    pstEcuIdList->pstEcuIdInfo[i].ecuReqid, 
                    pstEcuIdList->pstEcuIdInfo[i].ecuSwDid,
                    pstEcuIdList->pstEcuIdInfo[i].psrcPkgFile->fileName, iRet);
                return ADM_ECODE_AES_ECB_DECRYPT_FAILED;
            }

            iRet = adm_dpm_signatureVerification(ADM_DPM_COMBINE_SRC_ECU_PACKAGE, NULL, 
                        pstCombinePkgFileObj, &pstEcuIdList->pstEcuIdInfo[i]);
            if (ADM_IS_ERR(iRet))
            {
                log_e("Verify signature failed, ecuReqid[%s], ecuSwDid[%s], srcfileName[%s], err[%d]", 
                    pstEcuIdList->pstEcuIdInfo[i].ecuReqid,
                    pstEcuIdList->pstEcuIdInfo[i].ecuSwDid,
                    pstEcuIdList->pstEcuIdInfo[i].psrcPkgFile->fileName, iRet);
                return iRet;
            }
        }
    }
    log_d("Decrypt and Verify sign combine package ecu file ok.");

    return ADM_ECODE_SUCCESSED;
}

/*!
 *******************************************************************************
 * 处理已解密下载的集合包信息
 *
 * \note  解压/解析对应的配置文件
 * 
 * \param[in]   pstDownCombinePkg 已下载集合包信息
 * \retval      D32S
 *******************************************************************************
 */
static D32S adm_dpm_downCombinePackageUnzipAndParse
(
    ADM_DPS_DOWN_ECUCOMBINEPKG_T *pstDownCombinePkg
)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    DBOOL isExist = FALSE;
    DCHAR szCfgFilePath[ADM_COM_LEN_256] = {0};
    DCHAR szEventBuf[ADM_COM_LEN_256] = {0};  //mod by wl 20220321
    DCHAR *pcDownPkgUnzipDir = NULL;
    struct timespec tsNowunzipbegin;
    struct timespec tsNowunzipend;
    
    //mod by wl 20220321
    adm_dpm_addEventAndReport(ADM_COM_MOD_DPM, FALSE, ADM_EVENT_ID_DI_019, ADM_OCS_EVENT_LEVEL_DEBUG, "vehicle unzip and parse download combine package.", NULL);

    //!< 进入解压中
    adm_dpm_setDownloadUnpackIng();

    do
    {
        //!< 获取下载升级包的解压路径
        pcDownPkgUnzipDir = adm_dps_getDownPackageUnzipDir();
        if (ADM_IS_NULL(pcDownPkgUnzipDir))
        {
            //mod by wl 20220321
            iRet = ADM_ECODE_CORESERVICE_GETRUNDIR_FAILED;
            log_e("Failed to get coreService package unzip dir, err[%d]", iRet);
            (void)adm_memzero(szEventBuf, sizeof(szEventBuf));
            (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), "Failed to get coreService package unzip dir, err[%d]", iRet);
            break;
        }
        log_d("Download package unzip dir[%s]", pcDownPkgUnzipDir);
        (void)adm_memzero(szEventBuf, sizeof(szEventBuf));  //mod by wl 20220321
        (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), "Download package unzip dir[%s]", pcDownPkgUnzipDir);
        adm_dpm_addEventAndReport(ADM_COM_MOD_DPM, FALSE, ADM_EVENT_ID_DI_024, ADM_OCS_EVENT_LEVEL_DEBUG, "vehicle get download package unzip dir successed.", szEventBuf);

        isExist = adm_com_isDirExist(pcDownPkgUnzipDir);
        if (TRUE == isExist)
        {
            (void)adm_fremove(pcDownPkgUnzipDir);
        }
        
        log_d("create dir[%s]", pcDownPkgUnzipDir);
        iRet = adm_com_createDir(pcDownPkgUnzipDir);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to create dir[%s], err[%d]", pcDownPkgUnzipDir, iRet);
            //mod by wl 20220321
            (void)adm_memzero(szEventBuf, sizeof(szEventBuf));  //mod by wl 20220321
            (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), "Failed to create dir[%s], err[%d]", pcDownPkgUnzipDir, iRet);
            break;
        }
        log_d("create dir[%s] ok.", pcDownPkgUnzipDir);

        //mod by wl 20220321
        (void)adm_memzero(szEventBuf, sizeof(szEventBuf));  //mod by wl 20220321
        (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), "Download package unzip dir[%s]", pcDownPkgUnzipDir);
        adm_dpm_addEventAndReport(ADM_COM_MOD_DPM, FALSE, ADM_EVENT_ID_DI_025, ADM_OCS_EVENT_LEVEL_TRACE, "vehicle unzip combine package.", szEventBuf);

        // 记录解压集合包的开始时间
        (void)clock_gettime(CLOCK_MONOTONIC, &tsNowunzipbegin);
        iRet = adm_com_unZipFile(pstDownCombinePkg->stPkgFileObj.filePath, pcDownPkgUnzipDir);
        if (ADM_IS_ERR(iRet))
        {
            
            log_e("Failed to unZipFile[%s], unZipDir[%s], err[%d]", 
                        pstDownCombinePkg->stPkgFileObj.filePath, pcDownPkgUnzipDir, iRet);
            //mod by wl 20220321
            (void)adm_memzero(szEventBuf, sizeof(szEventBuf));
            (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), "Failed to unZipFile[%s], unZipDir[%s], err[%d]",
                        pstDownCombinePkg->stPkgFileObj.filePath, pcDownPkgUnzipDir, iRet);
            break;
        }
        // 记录解压集合包的结束时间
        (void)clock_gettime(CLOCK_MONOTONIC, &tsNowunzipend);
        (void)adm_memzero(szEventBuf, sizeof(szEventBuf));
        (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), "STATSPACK unzipTime:pkgFormat[combine] fileName[%s], unzipTime[%d]%s, fileSize[%lld]",
        pstDownCombinePkg->stPkgFileObj.fileName,
        ((tsNowunzipend.tv_sec - tsNowunzipbegin.tv_sec) < 1 ? (tsNowunzipend.tv_nsec / (1000 * 1000)) - (tsNowunzipbegin.tv_nsec / (1000 * 1000)) : (tsNowunzipend.tv_sec - tsNowunzipbegin.tv_sec)),
        ((tsNowunzipend.tv_sec - tsNowunzipbegin.tv_sec) < 1 ? "ms" : "s"),
        pstDownCombinePkg->stPkgFileObj.size);
        adm_dpm_addEventAndReport(ADM_COM_MOD_DPM, FALSE,ADM_EVENT_ID_DL_011, ADM_OCS_EVENT_LEVEL_INFO, szEventBuf, NULL);

        log_d("Unzip file[%s] to[%s] ok.", pstDownCombinePkg->stPkgFileObj.filePath, pcDownPkgUnzipDir);
        //mod by wl 20220321
        adm_dpm_addEventAndReport(ADM_COM_MOD_DPM, FALSE, ADM_EVENT_ID_DI_027, ADM_OCS_EVENT_LEVEL_DEBUG, "vehicle unzip combine package successed.", szEventBuf);

        //!< 校验通用配置文件是否存在
        (void)adm_snprintf(szCfgFilePath, sizeof(szCfgFilePath), "%s%s", pcDownPkgUnzipDir, ADM_DPM_DOWN_PACKAGE_CFGFILE_NAME);
        //mod by wl 20220321
        adm_dpm_addEventAndReport(ADM_COM_MOD_DPM, FALSE, ADM_EVENT_ID_DI_028, ADM_OCS_EVENT_LEVEL_TRACE, "vehicle check combine package config file.", szCfgFilePath);
        isExist = adm_com_isDirExist(szCfgFilePath);
        if (FALSE == isExist)
        {
            //mod by wl 20220321 
            iRet = ADM_ECODE_DOWN_PACKAGE_CFGFILE_NO_FIND;
            log_e("Failed to find file[%s]", szCfgFilePath);
            (void)adm_memzero(szEventBuf, sizeof(szEventBuf));
            (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), "Failed to find file[%s], err[%d]", szCfgFilePath, iRet);
            adm_dpm_addEventAndReport(ADM_COM_MOD_DPM, TRUE, ADM_EVENT_ID_DI_029, ADM_OCS_EVENT_LEVEL_ERROR, "vehicle check combine package config file failed!", szEventBuf);
            break;
        }
        //mod by wl 20220321
        adm_dpm_addEventAndReport(ADM_COM_MOD_DPM, FALSE, ADM_EVENT_ID_DI_030, ADM_OCS_EVENT_LEVEL_DEBUG, "vehicle check combine package config file successed.", szCfgFilePath);

        //!< 2. 解析配置文件    
        iRet = adm_dps_parsePackageConfigFile(szCfgFilePath, &pstDownCombinePkg->stPkgCfgInfo);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to parse down package config file[%s], err[%d]", szCfgFilePath, iRet);
            //mod by wl 20220321
            (void)adm_memzero(szEventBuf, sizeof(szEventBuf));
            (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), "Failed to parse down package config file, err[%d]", szCfgFilePath, iRet);
            break;
        }
        log_i("combine package config file parse ok.");

        //!< 3. 检查配置文件中ecu文件是否与check返回的ecu文件一致
        iRet = __adm_dpm_checkCombinePackageCfgEcuFileInfo(&pstDownCombinePkg->stEcuIdList, &pstDownCombinePkg->stPkgCfgInfo);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to check combine package config.son ecu file info, err[%d]", iRet);
            //mod by wl 20220321
            (void)adm_memzero(szEventBuf, sizeof(szEventBuf));
            (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), "Failed to check combine package config.son ecu file info, err[%d]",iRet);
            break;
        }
        log_i("combine package config file check ok.");

        //!< 4. 验签：验证ecu文件的签名
        iRet = adm_dpm_downCombinePackageEcuDecryptAndVerifySign(&pstDownCombinePkg->stPkgFileObj, &pstDownCombinePkg->stEcuIdList);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to decrypt and verify file signature combine package, err[%d]", iRet);
            (void)adm_memzero(szEventBuf, sizeof(szEventBuf));
            (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), "Failed to decrypt and verify file signature combine package, err[%d]",iRet);
            break;
        }

        //!< 5. 回写ecu配置信息到dps
        iRet = adm_dps_updateDownCombinePackageCfgInfo(&pstDownCombinePkg->stPkgFileObj,
                    &pstDownCombinePkg->stPkgCfgInfo);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to update down combine package cfg info, err[%d]", iRet);
            //mod by wl 20220321
            (void)adm_memzero(szEventBuf, sizeof(szEventBuf));
            (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), "Failed to update down combine package cfg info, err[%d]",iRet);
            break;
        }

        //!< 6. 更新集合包中各ecu升级包文件存储路径到pis
        iRet = __adm_dpm_updateCombinePkgAllEcuSavePath2Pis(&pstDownCombinePkg->stEcuIdList);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to update combine package all ecu save path 2 pis, err[%d]", iRet);
            //mod by wl 20220321
            (void)adm_memzero(szEventBuf, sizeof(szEventBuf));
            (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), "Failed to update combine package all ecu save path 2 pis, err[%d]",iRet);
            break;
        }   
    } while (FALSE);

    adm_free(pcDownPkgUnzipDir);

    //mod by wl 20220321
    if (ADM_IS_ERR(iRet))
    {
        //!< 进入解压失败
        adm_dpm_setDownloadUnpackFailed();
        adm_dpm_addEventAndReport(ADM_COM_MOD_DPM, TRUE, ADM_EVENT_ID_DI_020, ADM_OCS_EVENT_LEVEL_ERROR, "vehicle unzip and parse download combine package failed!", szEventBuf);
    }
    else
    {
        //!< 进入解压成功
        adm_dpm_setDownloadUnpackSuccessed();
        adm_dpm_addEventAndReport(ADM_COM_MOD_DPM, FALSE, ADM_EVENT_ID_DI_021, ADM_OCS_EVENT_LEVEL_DEBUG, "vehicle unzip and parse download combine package successed.", szEventBuf);
    }
    return iRet;
}

/*!
 *******************************************************************************
 * 下载处理一个ecu的集合包
 *
 * \note 同步接口，集合包仅支持主控下载
 * 
 * \param[in]   pstDownPartPkgInfo 待下载ecu part包信息 
 * \param[in]   pstDpmDownReqParam dpm下载包参数 
 * \retval      D32S
 *******************************************************************************
 */
static D32S adm_dpm_downOnceCombinePackageProc
(
    ADM_DPS_DOWN_ECUCOMBINEPKG_T *pstDownCombinePkg,
    ADM_DPM_DOWNREQPARAM_T       *pstDpmDownReqParam
)
{
    D32S iRet = ADM_ECODE_INVALID_PARAM;
    DCHAR szEventBuf[ADM_COM_LEN_256] = {0};  //mod by wl 20220321
    
    //mod by wl 20220321
    (void)adm_memzero(szEventBuf, sizeof(szEventBuf));  //mod by wl 20220321
    adm_dpm_addEventAndReport(ADM_COM_MOD_DPM, FALSE, ADM_EVENT_ID_DI_007, ADM_OCS_EVENT_LEVEL_DEBUG, "vehicle download once combine package.", NULL);

    if (ADM_IS_NULL(pstDownCombinePkg) || ADM_IS_NULL(pstDpmDownReqParam))
    {
        log_e("Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }

    log_d("Todo downloading combine package.");
    //!< 创建下载所需参数
    __adm_dpm_cleanDPMDownReqParam(pstDpmDownReqParam);
    iRet = __adm_dpm_fillCombinePkgDPMDownReqParam(pstDownCombinePkg, pstDpmDownReqParam);
    if (ADM_IS_ERR(iRet))
    {
        log_e("Failed to fill combine pkg dpm down req param, err[%d]", iRet);
        //mod by wl 20220321
        (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), "Failed to fill combine pkg dpm down req param, err[%d]", iRet);
        adm_dpm_addEventAndReport(ADM_COM_MOD_DPM, TRUE, ADM_EVENT_ID_DI_008, ADM_OCS_EVENT_LEVEL_ERROR, "vehicle download once combine package failed!", szEventBuf);
        return iRet;    
    }

    switch (pstDownCombinePkg->stDownProcess.stResult.eDownState)
    {
        case ADM_DPS_DOWNPKG_STATE_NONE:
        case ADM_DPS_DOWNPKG_STATE_NORMAL:
        case ADM_DPS_DOWNPKG_STATE_PAUSE:
        case ADM_DPS_DOWNPKG_STATE_CANCEL:
        case ADM_DPS_DOWNPKG_STATE_FAILED:
        case ADM_DPS_DOWNPKG_STATE_SRCFINISH:
        {
            log_d("Entry combine package download.");
            
            //!< 下载升级包
            if (ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_USB == gstDPMdownInfo.eTriggerType)
            {
                iRet = adm_dpm_usbDownload(ADM_DPS_UCM_INSTID, pstDpmDownReqParam, pstDownCombinePkg->stPkgFileObj.fileName);
            }
            else 
            {
                // TODO: 集合包暂时由master下载，后期支持slave下载
                iRet = adm_dpm_Download(ADM_DPS_UCM_INSTID, pstDpmDownReqParam);
            }

            if (ADM_IS_ERR(iRet))
            {
                log_e("Download combine package[%s] failed, err:[%d]", pstDownCombinePkg->stPkgFileObj.fileName, iRet);
                //mod by wl 20220321
                (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), "Download combine package[%s] failed, err[%d]", pstDownCombinePkg->stPkgFileObj.fileName, iRet);
                break; 
            }
            log_d("Download combine package ok.");
        } //!< 此处 case 不需要      break, 下载完成后需要继续后续的解密、解压和解析等处理
        case ADM_DPS_DOWNPKG_STATE_WAITDECRYPT:
        case ADM_DPS_DOWNPKG_STATE_DECRYPTFAILED:
        {
            log_d("Entry combine package decrypt.");
            //!< 解密升级包
            iRet = adm_dpm_downCombinePackageDecrypt(pstDownCombinePkg);
            //mod by wl 20220406 
            adm_dpm_updateDownResult(iRet, 
                                     (ADM_IS_SUCCESS(iRet) ? (ADM_DPS_DOWNPKG_STATE_CHECKSIGN) : (ADM_DPS_DOWNPKG_STATE_DECRYPTFAILED)), 
                                     pstDpmDownReqParam);
            if (ADM_IS_ERR(iRet))
            {
                log_e("Decrypt combine package[%s] failed, err[%d]", pstDownCombinePkg->stPkgFileObj.fileName, iRet);
                //mod by wl 20220321
                (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), "Decrypt combine package[%s] failed, err[%d]", 
                        pstDownCombinePkg->stPkgFileObj.fileName, iRet);
                break; 
            }
            log_d("Download combine package decrypt ok.");
        }
        //mod by wl 20220411
        case ADM_DPS_DOWNPKG_STATE_CHECKSIGN:
        case ADM_DPS_DOWNPKG_STATE_CHECKSIGNFAILED:
        {
            //集合包验签和拆包
            iRet = __adm_dpm_combinepkgSignatureAndUnpacking(pstDownCombinePkg);
            adm_dpm_updateDownResult(iRet, 
                            (ADM_IS_SUCCESS(iRet) ? (ADM_DPS_DOWNPKG_STATE_WAITPARSE) : (ADM_DPS_DOWNPKG_STATE_CHECKSIGNFAILED)), 
                            pstDpmDownReqParam);
            if (ADM_IS_ERR(iRet))
            {
                log_e("verify signature, Separate combine package failed, err[%d]", iRet);
                (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), "verify signature, Separate combine package[%s] failed, err[%d]", 
                        pstDownCombinePkg->stPkgFileObj.fileName, iRet);
                break; 
            }
            log_d("verify signature, Separate combine package ok.");
            
        }
        case ADM_DPS_DOWNPKG_STATE_WAITPARSE:
        case ADM_DPS_DOWNPKG_STATE_PARSEFAILED:
        {
            (void)adm_dpm_vehicleStateReport2Cloud(ADM_IRM_REPORT_SERVER_STATUS_Task_Unpack_ing, 
                            adm_dpm_errorCode2String(iRet));
            //!< 解压及解析配置文件
            iRet = adm_dpm_downCombinePackageUnzipAndParse(pstDownCombinePkg); 
            adm_dpm_updateDownResult(iRet, 
                                     (ADM_IS_SUCCESS(iRet) ? (ADM_DPS_DOWNPKG_STATE_FINISH) : (ADM_DPS_DOWNPKG_STATE_PARSEFAILED)), 
                                     pstDpmDownReqParam);
            if (ADM_IS_ERR(iRet))
            {
                (void)adm_dpm_vehicleStateReport2Cloud(ADM_IRM_REPORT_SERVER_STATUS_Task_Unpack_Failed, 
                            adm_dpm_errorCode2String(iRet));
                log_e("Unzip or parse combine package[%s] failed, err[%d]", pstDownCombinePkg->stPkgFileObj.fileName, iRet);
                //mod by wl 20220321
                (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), "Unzip or parse combine package[%s] failed, err[%d]", 
                        pstDownCombinePkg->stPkgFileObj.fileName, iRet);
                break; 
            }
            log_d("Download combine package unzip and parse ok.");
            (void)adm_dpm_vehicleStateReport2Cloud(ADM_IRM_REPORT_SERVER_STATUS_Task_Unpack_Successed, 
                            adm_dpm_errorCode2String(iRet));
            break;
        }
        default:
            iRet = ADM_ECODE_INVALID_PARAM;
            log_e("Failed to switch package down state[%d].", pstDownCombinePkg->stDownProcess.stResult.eDownState);
            //mod by wl 20220321
            (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), "Failed to switch package down state[%d], err[%d]", 
                        pstDownCombinePkg->stDownProcess.stResult.eDownState, iRet);
            break;
    }
    
    //mod by wl 20220321
    if (ADM_IS_ERR(iRet))
    {
        adm_dpm_addEventAndReport(ADM_COM_MOD_DPM, TRUE, ADM_EVENT_ID_DI_008, ADM_OCS_EVENT_LEVEL_ERROR, "vehicle download once combine package failed!", szEventBuf);
    }
    else
    {
        adm_dpm_addEventAndReport(ADM_COM_MOD_DPM, FALSE, ADM_EVENT_ID_DI_009, ADM_OCS_EVENT_LEVEL_DEBUG, "vehicle download once combine package successed.", NULL);
    }

    return iRet;
}

/*!
 *******************************************************************************
 * 下载处理ecu集合包
 *
 * \note    同步接口
 * 
 * \param[in]   pstDpmDownReqParam   dpm下载请求参数信息
 * \retval      D32S
 *******************************************************************************
 */
static D32S adm_dpm_downloadCombinePackageProc(ADM_DPM_DOWNREQPARAM_T *pstDpmDownReqParam)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    D32S i = 0;
    D32S tobeDownloadNum = 0;
    ADM_DPS_DOWN_ECUCOMBINEPKG_T *pstDownCombinePkgList = NULL;
    DCHAR szEventBuf[ADM_COM_LEN_256] = {0}; 
    struct timespec tsNowbegin;
    struct timespec tsNowend;
    D32S packageNum = 0;

    if (ADM_IS_NULL(pstDpmDownReqParam))
    {
        log_e("Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }

    pstDownCombinePkgList = adm_dps_newEcuCombinePackageList(ADM_DPM_DEFDOWN_GETTOBELIST_CNT);
    if (ADM_IS_NULL(pstDownCombinePkgList))
    {
        log_e("Failed to new ecu combine package list");
        return ADM_ECODE_NO_MEMORY;
    }
    //!< 记录零件包下载的起始时间
    (void)clock_gettime(CLOCK_MONOTONIC, &tsNowbegin);

    //!< 循环遍历待下载包列表下载包
    do
    {
        //mod by wl 20220321
        (void)adm_memzero(szEventBuf, sizeof(szEventBuf));
        adm_dpm_addEventAndReport(ADM_COM_MOD_DPM, FALSE, ADM_EVENT_ID_DI_004, ADM_OCS_EVENT_LEVEL_DEBUG, "vehicle get tobe download ecu combine package.", NULL);
        
        (void)adm_memzero(pstDownCombinePkgList, (D32U)ADM_DPM_DEFDOWN_GETTOBELIST_CNT * sizeof(*pstDownCombinePkgList));
        iRet = adm_dps_getTobeDownloadEcuCombinePackage(ADM_DPM_DEFDOWN_GETTOBELIST_CNT, 
                    pstDownCombinePkgList, &tobeDownloadNum);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to get tobe download ecu combine package, err[%d]", iRet);
            (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), "Failed to get tobe download ecu combine package, err[%d]", iRet);
            adm_dpm_addEventAndReport(ADM_COM_MOD_DPM, TRUE, ADM_EVENT_ID_DI_005, ADM_OCS_EVENT_LEVEL_ERROR, "vehicle get tobe download ecu combine package failed!", szEventBuf);
            break;
        }
        //!< 记录集合包的总个数
        packageNum += tobeDownloadNum;
        log_d("Tobe download combine package Num[%d]", tobeDownloadNum);
        (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), "Tobe download combine package Num[%d]", tobeDownloadNum);
        adm_dpm_addEventAndReport(ADM_COM_MOD_DPM, FALSE, ADM_EVENT_ID_DI_006, ADM_OCS_EVENT_LEVEL_DEBUG, "vehicle get tobe download ecu combine package successed.", szEventBuf);

        for (i = 0; i < tobeDownloadNum; i++)
        {
            //下载过程监控到异常信息需要中断下载 mod by wl 20220328 
            iRet = adm_dpm_checkDownIsAbnormal();
            if (ADM_IS_ERR(iRet))
            {
                log_e("Notify down monitor abnormal failed to download combine package, pkgName[%s], err[%d]", 
                      pstDownCombinePkgList[i].stPkgFileObj.fileName, iRet);
                break; 
            }
            
            //!< 下载单个集合包信息
            iRet = adm_dpm_downOnceCombinePackageProc(&pstDownCombinePkgList[i], pstDpmDownReqParam);            
            if (ADM_IS_ERR(iRet))
            {
                log_e("Failed to download combine package, pkgName[%s], err[%d]", 
                      pstDownCombinePkgList[i].stPkgFileObj.fileName, iRet);
                break; 
            }
            log_d("Download once combine package proc ok.");
        }
        //!< 若for-break，则while-break
        if (ADM_IS_ERR(iRet))
        {
            break;
        }

        //!< 清空本次的内存脏数据
        adm_dps_cleanEcuCombinePackageList(ADM_DPM_DEFDOWN_GETTOBELIST_CNT, pstDownCombinePkgList);
    } while (0 < tobeDownloadNum);

    if (ADM_IS_SUCCESS(iRet))
    {
        //!< 记录零件包下载的结束时间
        (void)clock_gettime(CLOCK_MONOTONIC, &tsNowend);
        (void)adm_memzero(szEventBuf, sizeof(szEventBuf));
        (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), "STATSPACK downAllTime:downTime[%d]%s, combineNum[%d], pkgType[1](1: combine, 2: part)",
        ((tsNowend.tv_sec - tsNowbegin.tv_sec) < 1 ? (tsNowend.tv_nsec / (1000 * 1000)) - (tsNowbegin.tv_nsec / (1000 * 1000)) : (tsNowend.tv_sec - tsNowbegin.tv_sec)),
        ((tsNowend.tv_sec - tsNowbegin.tv_sec) < 1 ? "ms" : "s"),
        packageNum);
        adm_dpm_addEventAndReport(ADM_COM_MOD_DPM, FALSE,ADM_EVENT_ID_DL_006, ADM_OCS_EVENT_LEVEL_INFO, szEventBuf, NULL);
    }

    adm_dps_delEcuCombinePackageList(ADM_DPM_DEFDOWN_GETTOBELIST_CNT, &pstDownCombinePkgList);
    return iRet;
}

/*!
 *******************************************************************************
 * 下载处理升级包
 *
 * \note    
 * 
 * \param   none
 * \retval  D32S
 *******************************************************************************
 */
static D32S adm_dpm_downloadPackageProc(void)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    ADM_DPM_DOWNREQPARAM_T *pstDpmDownReqParam = NULL;
    ADM_DPS_DOWNPACKAGEFORMAT_E eDownPkgFormat = ADM_DPS_DOWNPACKAGE_PART;
    DCHAR szEventBuf[ADM_COM_LEN_256] = {0};  //mod by wl 20220321

    do
    {
        // TODO: 后续参数改为 FALSE 
        pstDpmDownReqParam = __adm_dpm_createDPMDownReqParam(3, 
                                                             TRUE, 
                                                             adm_dpm_downResultHandler, 
                                                             adm_dpm_downProgressHandler);
        if (ADM_IS_NULL(pstDpmDownReqParam))
        {
            iRet = ADM_ECODE_NO_MEMORY;
            log_e("Failed to new ecu part package list, err[%d]", iRet);
            break;
        }

        //mod by wl 20220321
        (void)adm_memzero(szEventBuf, sizeof(szEventBuf));
        adm_dpm_addEventAndReport(ADM_COM_MOD_DPM, FALSE, ADM_EVENT_ID_DI_001, ADM_OCS_EVENT_LEVEL_DEBUG, "vehicle get download package format.", NULL);

        iRet = adm_dps_EcuPackageFormat(&eDownPkgFormat);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to get package Format[combine/part], err[%d]", iRet);
            (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), "Failed to get package Format[combine/part], err[%d]", iRet);
            adm_dpm_addEventAndReport(ADM_COM_MOD_DPM, FALSE, ADM_EVENT_ID_DI_002, ADM_OCS_EVENT_LEVEL_DEBUG, "vehicle get download package format failed!", szEventBuf);
            break;
        }
        log_d("eDownPkgFormat[%d]", eDownPkgFormat);
        (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), "eDownPkgFormat[%d]", eDownPkgFormat);
        adm_dpm_addEventAndReport(ADM_COM_MOD_DPM, FALSE, ADM_EVENT_ID_DI_003, ADM_OCS_EVENT_LEVEL_DEBUG, "vehicle get download package format successed.", szEventBuf);
            
        if (ADM_DPS_DOWNPACKAGE_PART == eDownPkgFormat)
        {
            iRet = adm_dpm_downloadPartPackageProc(pstDpmDownReqParam);
        }
        else
        {
            iRet = adm_dpm_downloadCombinePackageProc(pstDpmDownReqParam);
        }

        if (ADM_IS_SUCCESS(iRet))
        {
            /**设置下载成功*/
            adm_dpm_setUimDownloadStatus(1);
        }
        else
        {
            /*设置下载失败*/
            adm_dpm_setUimDownloadStatus(2);
        }

        (void)adm_dpm_vehicleStateReport2Cloud(ADM_IRM_REPORT_SERVER_STATUS_Task_Download_Completed, 
                adm_dpm_errorCode2String(iRet));

        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to download PkgFormat[%d], err[%d]", eDownPkgFormat, iRet);
            break;
        }
    } while (FALSE);

    __adm_dpm_delDPMDownReqParam(&pstDpmDownReqParam);
    return iRet;
}

/*!
 *******************************************************************************
 * 创建线程
 *
 * \note 封装 pthread_create 函数(创建的线程为与主线程分离)
 * 
 * \param[in]  pThreadHandle  子线程句柄
 * \param[in]  start_routine  子线程运行函数
 * \param[in]  arg            子线程运行函数参数
 * \retval     D32S           0: 成功 non-0：失败
 *******************************************************************************
 */
D32S adm_dpm_pthreadCreate
(
    pthread_t *pThreadHandle,
    void *(*start_routine)(void *param),
    void *arg
)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    D32S errNum = 0;
    pthread_attr_t stThreadAttr = {0};

    /* 获取线程属性句柄 */
    iRet = pthread_attr_init(&stThreadAttr);
    if (ADM_IS_ERR(iRet))
    {
        errNum = errno;
        log_e("pthread_attr_init failed. err[%d], strerror[%s]", iRet, strerror(errNum));
        return ADM_ECODE_PTHREAD_CREATE_FAILED;
    }

    /* 设置线程分离属性 */
    iRet = pthread_attr_setdetachstate(&stThreadAttr, PTHREAD_CREATE_DETACHED);
    if (ADM_IS_ERR(iRet))
    {
        errNum = errno;
        log_e("pthread_attr_setdetachstate failed. iRet[%u], strerror[%s]", iRet, strerror(errNum));
        (void)pthread_attr_destroy(&stThreadAttr);
        return ADM_ECODE_PTHREAD_CREATE_FAILED;
    }

    //!< 启下载任务线程
    iRet = pthread_create(pThreadHandle,
                          &stThreadAttr,
                          start_routine,
                          arg);
    if (ADM_IS_ERR(iRet))
    {
        errNum = errno;
        log_e("Failed to create thread, err[%d], strerror[%s]", iRet, strerror(errNum));
        (void)pthread_attr_destroy(&stThreadAttr);
        return ADM_ECODE_PTHREAD_CREATE_FAILED;
    }

    /* 销毁线程属性句柄 */
    (void)pthread_attr_destroy(&stThreadAttr);

    return iRet;
}

#if 0
#endif

/*!
 *******************************************************************************
 * dpm 模拟获取检测结果
 *
 * \note    gpcDownCombinePolicyJsonPath --- 模拟集合包策略
 *          gpcDownPartPolicyJsonPath    --- 模拟零件包策略
 *   
 * \param[in] pstVersionPolicy 检测结果策略信息
 * \retval  D32S
 *******************************************************************************
 */
static D32S adm_dpm_debugGetVersionPolicy(ADM_CVS_VERSION_POLICY_T *pstVersionPolicy)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    DCHAR *pcFileData = NULL;

    if (ADM_IS_NULL(pstVersionPolicy))
    {
        log_e("Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }

#if 1
    pstVersionPolicy->taskId = 1517421443843952641;
    iRet = adm_com_readFileData(gpcDownCombinePolicyJsonPath, &pcFileData);
#else
    pstVersionPolicy->taskId = 1517421443843952641;
    iRet = adm_com_readFileData(gpcDownPartPolicyJsonPath, &pcFileData);
#endif
    if (ADM_IS_ERR(iRet))
    {
        log_e("Failed to read download policy file, err[%d]", iRet);
        return iRet;
    }
    
    pstVersionPolicy->strategyId = 1517421201924022273;
    pstVersionPolicy->sessionId  = adm_strdup("1650784243376ovg54s4g");
    // cout << "[----------] ===> downPolicy[" << policyRoot["downPolicy"].toStyledString() << "]" << endl;
    pstVersionPolicy->downloadMember.downloadPolicyObjectStr = pcFileData;

    return iRet;
}

/*!
 *******************************************************************************
 * 清空从控下载升级包
 * 
 * \note 
 *
 * \retval  none
 *******************************************************************************
 */
static void adm_dpm_cleanSlaveDownPackage(void)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    D32S ierrorCode = 0;
    D32S s32UclInstId = 0;
    struct list_head *pstNode     = NULL;
    struct list_head *pstNextNode = NULL;
    ADM_DPS_DOWNINSTLIST *pstDownInstList = NULL;
    ADM_DPS_DOWNINSTINFO *pstDownInstanceInfo = NULL;
    ADM_DPS_DOWNPACKAGEFORMAT_E eDownPkgFormat = ADM_DPS_DOWNPACKAGE_PART;

    iRet = adm_dps_EcuPackageFormat(&eDownPkgFormat);
    if (ADM_IS_ERR(iRet))
    {
        log_e("Failed to get packageFormat part Package, err[%d]", iRet);
        return ;
    }
    //下载零件包时处理
    if (ADM_DPS_DOWNPACKAGE_PART == eDownPkgFormat)
    {
        pstDownInstList = adm_dps_getEcuDownInstanceList(eDownPkgFormat, DFALSE);
        if (ADM_IS_NULL(pstDownInstList))
        {
            log_e("Failed to get ecu down instance list, PkgFromat[%d]", eDownPkgFormat);
            return ;
        }
        log_d("ecu down instance list num[%d], PkgFromat[%d]", pstDownInstList->num, eDownPkgFormat);

        list_for_each_safe(pstNode, pstNextNode, &pstDownInstList->stHead)
        {
            //!< 遍历下载受控对象
            pstDownInstanceInfo = list_entry(pstNode, ADM_DPS_DOWNINSTINFO, stHead);
            
            //<! 调用从控接口去清除
            if (gstDPMdownInfo.s32UCMDownInstId == (D32S)htoi(pstDownInstanceInfo->downInstId))
            {
                s32UclInstId = adm_cfg_iniParserGetInt(ADM_CFG_VUS_UCL_INSTANCEID,  ADM_CFG_VUS_UCL_INSTANCEID_DEFAULT_VAL);
            }
            else
            {
                s32UclInstId = htoi(pstDownInstanceInfo->downInstId);
            }
            log_d("clean ecu download dir, uclInstId[%d]", s32UclInstId);
            iRet = adm_dim_dpo_cleanDownloadDir((D16U)s32UclInstId, 30, &ierrorCode);
            if (ADM_IS_ERR(iRet) || ADM_IS_ERR(ierrorCode))
            {
                log_e("Failed to clean slave package, errorCode[%d], err[%d]", ierrorCode, iRet);
            }
        }
    }
    //!< 释放下载受控对象list
    adm_dps_delEcuDownInstanceList(&pstDownInstList);

    log_d("clean Slave Download Package ok");

    //兼容主控代下载vdi目录，qnx未进行清理
    D32S errorCode = ADM_ECODE_INVALID_PARAM;
    adm_dim_dpo_cleanDownloadDir(adm_soa_getVdiInstanceId(),30,&errorCode);
    log_i("vdi instanceid is [%d]",adm_soa_getVdiInstanceId());
    
    // adm_dim_dpo_cleanDownloadDir(adm_soa_getUclAdccInstanceId(),30,&errorCode);
    // log_i("ucl adcc instanceid is [%d]",adm_soa_getUclAdccInstanceId());

    return ;
}

#if 0
#endif

/*!
 *******************************************************************************
 * dpm模块初始化.
 *
 * \note
 *
 * \param   none
 * \retval  D32S    0: 成功 non-0：失败
 *******************************************************************************
 */
D32S adm_dpm_moduleInit(ADM_DPM_FUNC_CALL_BACK_T* pstCallbackInfo)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    D32S errNum = 0;

    if (TRUE == gstDPMdownInfo.isInited)
    {
        log_i("dpm module already init.");
        return iRet;
    }

    do
    {
        iRet = adm_dps_moduleInit(adm_dpm_addEventAndReport);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to init dps module, err[%d]", iRet);
            break;
        }
        
        iRet = adm_dpm_reportModuleInit();
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to init dpm report module, err[%d]", iRet);
            break;
        }
        
        iRet = pthread_rwlock_init(&gstDPMdownInfo.stRWlock, NULL);
        if (ADM_IS_ERR(iRet))
        {
            iRet = ADM_ECODE_RWLOCK_INIT_FAILED;
            errNum = errno;
            log_e("Failed to init rwlock, err[%d](%s)", iRet, strerror(errNum));
            break;
        }
        
        gstDPMdownInfo.eState = ADM_DPM_DOWNTASK_TO_BE;
        gstDPMdownInfo.monitorHandle     = NULL;
        gstDPMdownInfo.isMonitorRunning  = FALSE; 
        gstDPMdownInfo.downThreadHandle  = 0;
        gstDPMdownInfo.eMonitorState = ADM_DPM_MONITOR_NORMAL;  //mod by wl 20220328
        gstDPMdownInfo.eTriggerType      = ADM_APPLICATION_COMMON_CMDID_TYPE_INVALID;

        //!< ucm 的实例id --- 后面区分主控下载还从控下载
        gstDPMdownInfo.s32UCMDownInstId = adm_cfg_iniParserGetInt(ADM_CFG_VUS_UIM_INSTANCEID,  ADM_CFG_VUS_UIM_INSTANCEID_DEFAULT_VAL);
        log_d("pcUCMDownInstId[%d]", gstDPMdownInfo.s32UCMDownInstId);

        g_stFuncCallbackInfo.schduleInstallCallback = pstCallbackInfo->schduleInstallCallback;
        g_stFuncCallbackInfo.schduleNotifyHmi = pstCallbackInfo->schduleNotifyHmi;
        g_stFuncCallbackInfo.getTspTriggerTypeCallback = pstCallbackInfo->getTspTriggerTypeCallback;
        g_stFuncCallbackInfo.tspPowerOnCallback = pstCallbackInfo->tspPowerOnCallback;
        g_stFuncCallbackInfo.tspPowerOffCallback = pstCallbackInfo->tspPowerOffCallback;
        g_stFuncCallbackInfo.exitTspEnvCallback = pstCallbackInfo->exitTspEnvCallback;

        gstDPMdownInfo.isInited = TRUE;
    } while (FALSE);

    if (ADM_IS_ERR(iRet))
    {
        (void)adm_dpm_moduleDeInit();
    }

    log_d("===> dpm init module ok.");

    return iRet;
}

/*!
 *******************************************************************************
 * dpm模块销毁.
 *
 * \note
 *
 * \param   none
 * \retval  D32S    0: 成功 non-0：失败
 *******************************************************************************
 */
D32S adm_dpm_moduleDeInit(void)
{
    D32S iRet = ADM_ECODE_SUCCESSED;

    if (TRUE == gstDPMdownInfo.isInited)
    {
        gstDPMdownInfo.isInited = FALSE;
    
        adm_dps_moduleDeInit();
        
        iRet = adm_dpm_reportModuleDeInit();
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to Deinit dpm report module, err[%d]", iRet);
        }
        
        (void)pthread_rwlock_destroy(&gstDPMdownInfo.stRWlock);
        gstDPMdownInfo.eState = ADM_DPM_DOWNTASK_TO_BE;
        gstDPMdownInfo.monitorHandle     = NULL;
        gstDPMdownInfo.isMonitorRunning  = FALSE; 
        gstDPMdownInfo.downThreadHandle  = 0;
        gstDPMdownInfo.eMonitorState = ADM_DPM_MONITOR_NORMAL;  //mod by wl 20220328
        gstDPMdownInfo.eTriggerType = ADM_APPLICATION_COMMON_CMDID_TYPE_INVALID;
        gstDPMdownInfo.s32UCMDownInstId = 0;
        //!< 其他待初始化值
        g_stFuncCallbackInfo.schduleNotifyHmi = NULL;
        g_stFuncCallbackInfo.schduleInstallCallback = NULL;
    }

    return iRet;
}


/*!
 *******************************************************************************
 * 检查更新下载信息
 * 
 * \note   模块初始化完成后，需调用此接口更新下载信息
 *
 * \param     none 
 * \retval    D32S
 *******************************************************************************
 */
D32S adm_dpm_checkAndUpdateDownInfo(void)
{
    D32S iRet = ADM_ECODE_SUCCESSED;

    log_i("[ABUP-Procedure] Check and update download policy info Start");
    /*
     *  模拟 下载集合包策略/零件包策略
     *  iRet = adm_dps_checkAndUpdateDPSDownInfo(adm_dpm_debugGetVersionPolicy,adm_dpm_cleanSlaveDownPackage);
     *
     */
#ifdef ADM_DEBUG_DPM_MODULE
    //!< 模拟，使用文件读取下载策略信息
    iRet = adm_dps_checkAndUpdateDPSDownInfo(adm_dpm_debugGetVersionPolicy,adm_dpm_cleanSlaveDownPackage);
#else
    //!< 实际，使用cvs获取下载策略信息
    iRet = adm_dps_checkAndUpdateDPSDownInfo(adm_cvs_getVersionPolicy,adm_dpm_cleanSlaveDownPackage);
#endif
    if (ADM_IS_ERR(iRet))
    {
        log_e("Failed to check and update DPS down info, err[%d]", iRet);
        return iRet;            
    }

    //!< 更新上报的会话信息
    iRet = adm_dpm_checkAndUpdateDownReportInfo();
    if (ADM_IS_ERR(iRet))
    {
        log_e("Failed to check and update down report info, err[%d]", iRet);
        return iRet;            
    }

    log_i("[ABUP-Procedure] Check and update download policy info End, ret[%d]",iRet);

    return iRet;
}

static D32U convertToSeconds(const char *timeStr) 
{
    D32S hours = (timeStr[0] - '0') * 10 + (timeStr[1] - '0');
    D32S minutes = (timeStr[2] - '0') * 10 + (timeStr[3] - '0');
    D32S seconds = (timeStr[4] - '0') * 10 + (timeStr[5] - '0');
    return hours * 3600 + minutes * 60 + seconds;
}
static D64U dpmTransTime(D64U uiTimestamp)
{
    D64U nowTimeStamp = 0;
    DCHAR aTime[7]={0};
    DCHAR aTimeNow[7]={0};
    D64U timeIn = 0;
    D64U timeNow = 0;
    D64U timeResult = 0;

    if (0 == uiTimestamp)
    {
        log_e("trans timestamp is invalid.");
        return 0;
    }
    struct tm *pstTimeInfo = gmtime((time_t*)&uiTimestamp);
    if (NULL == pstTimeInfo)
    {
        log_e("uiTimestamp error uiTimestamp[%llu].", uiTimestamp);
        return 0;
    }
    strftime(aTime, sizeof(aTime), "%H%M%S", pstTimeInfo);

    nowTimeStamp = time(NULL);
    struct tm *pstTimeNow = localtime((time_t*)&nowTimeStamp);
    strftime(aTimeNow, sizeof(aTimeNow), "%H%M%S", pstTimeNow);

    timeIn = convertToSeconds(aTime);
    timeNow = convertToSeconds(aTimeNow);

    if (timeNow < timeIn)
    {
        timeResult = timeIn - timeNow;
    }
    else if(timeNow == timeIn)
    {
        timeResult = 10;        ///< 如果时间一致,默认预约10s后
    }
    else 
    {
        timeResult = 24 * 3600 - (timeNow - timeIn);    ///< 顺延一天时间
    }

    return timeResult;
}

DBOOL adm_dpm_getIsAnyInstallTime(D64U *pAutoStartTime)
{
    D32S nRet = ADM_ECODE_FAILED;
    D64U uiTime = 0;

    nRet = adm_cvs_getSilenceUpgradeTime(&uiTime);
    if (ADM_IS_SUCCESS(nRet) && 0 < uiTime)
    {
        if (NULL != pAutoStartTime)
        {
            *pAutoStartTime = uiTime;
        }
        return DTRUE;
    }

    return DFALSE;
}

/*!
 *******************************************************************************
 * 触发下载
 *
 * \note
 * 
 * \param[in] eTriggerType 触发下载类型
 * \retval    D32S
 *******************************************************************************
 */
D32S adm_dpm_triggerDownload
(
    ADM_APPLICATION_COMMON_CMDID_TYPE_E eTriggerType
)
{
    D32S iRet  = ADM_ECODE_FAILED;
    D32S iiRet = ADM_ECODE_FAILED;
    D32S nRet = ADM_ECODE_FAILED;
    DBOOL bIsHasInstallTime = DFALSE;
    D32S errorCode = ADM_ECODE_FAILED;
    D32S i32WaitDownEcuNum = 0;
    ADM_DPS_DOWNRESULT_T *pstResult = NULL;
    DCHAR szEventBuf[ADM_COM_LEN_256] = {0};
    D32S iDownMaxTimeConsuming = 0;
    D64U uiAutoStartTime = 0;
    DBOOL bIsUsbeWebStart = DFALSE;
    D64S ulTimestamp = 0;
    ADM_CVS_UPGRADE_MODE_E eUpgradeMode = ADM_CVS_UPGRADE_MODE_INVALID;
    ADM_APPLICATION_COMMON_CMDID_TYPE_E eTspTriggerType = ADM_APPLICATION_COMMON_CMDID_TYPE_INVALID;
    DBOOL bNeedStopWakeup = DFALSE;


    struct timespec tsNowbegin;
    struct timespec tsNowend;

    //!< 记录下载的起始时间
    (void)clock_gettime(CLOCK_MONOTONIC, &tsNowbegin);

    log_i("Entry trigger download process.");

    log_i("[ABUP-Procedure] trigger download Start. eTriggerType:%d",eTriggerType);

    do 
    {
        /** 获取是否tsp任务类型 */
        eTspTriggerType = g_stFuncCallbackInfo.getTspTriggerTypeCallback();

        //!< 进入下载中 --- 其实应该是下载准备
        adm_dpm_setDownloadIng();
        adm_dpm_setUimDownloadStatus(0);  /**初始化下载结果状态*/
    
        (void)adm_memzero(szEventBuf, sizeof(szEventBuf));
        //!< 保护模块已初始化
        if (FALSE == gstDPMdownInfo.isInited)
        {
            log_e("dpm module not init, err[%d].", iRet);
            (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), "Failed to init dpm module, err[%d]", iRet);
            adm_dpm_addEventAndReport(ADM_COM_MOD_DPM, FALSE, ADM_EVENT_ID_DT_001, ADM_OCS_EVENT_LEVEL_TRACE, szEventBuf, ADM_DPM_MARKER_EV_SUB_BEGIN);
            break;
        }

        (void)adm_memzero(szEventBuf, sizeof(szEventBuf));
        (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), "trigger vehicle downlaod package, type[%d].", eTriggerType);
        adm_dpm_addEventAndReport(ADM_COM_MOD_DPM, FALSE, ADM_EVENT_ID_DT_001, ADM_OCS_EVENT_LEVEL_TRACE, szEventBuf, ADM_DPM_MARKER_EV_SUB_BEGIN);

        (void)pthread_rwlock_wrlock(&gstDPMdownInfo.stRWlock);
        gstDPMdownInfo.eTriggerType = eTriggerType;
        (void)pthread_rwlock_unlock(&gstDPMdownInfo.stRWlock);

        //!< 检查更新下载任务
        iRet = adm_dpm_checkAndUpdateDownInfo();
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to check and update DPS down info, err[%d]", iRet);
            (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), "Failed to check and update DPS down info, err[%d]", iRet);
            break;            
        }
        
        (void)adm_dpm_vehicleStateReport2Cloud(ADM_IRM_REPORT_SERVER_STATUS_Task_Download_Prepare, 
            adm_dpm_errorCode2String(ADM_ECODE_SUCCESSED));

        //!< 确认下载任务有效性
        iRet = adm_dpm_confirmDownloadTask();
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to confirm download task, err[%d][%d].", iRet, ADM_ECODE_CONFIRM_DOWNLOAD_TASK_FAILED);
            iRet = ADM_ECODE_CONFIRM_DOWNLOAD_TASK_FAILED;
            (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), "Failed to confirm download task, err[%d][%d]", iRet, ADM_ECODE_CONFIRM_DOWNLOAD_TASK_FAILED);
            break;
        }

        //开启下载模拟,只有达到一定条件才会进行下载进度模拟
        adm_dpm_startDownloadProgressSimulate(TRUE);

        //!< 提前获取待下载ecu数量，减少不必要的流程判断 
        iRet = adm_dps_getWaitDownloadEcuNum(&i32WaitDownEcuNum);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to get wait download Ecu num, err[%d].", iRet);
            (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), "Failed to get wait download Ecu num, err[%d]", iRet);
            break;
        }
        log_d("Wait download Ecu num[%d].", i32WaitDownEcuNum);
        (void)adm_memzero(szEventBuf, sizeof(szEventBuf));
        (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), "Wait download Ecu num[%d].", i32WaitDownEcuNum);
        adm_dpm_addEventAndReport(ADM_COM_MOD_DPM, FALSE, ADM_EVENT_ID_DP_024, ADM_OCS_EVENT_LEVEL_TRACE, "vehicle get wait transfer ecu num successed.", szEventBuf);

        if (0 == i32WaitDownEcuNum)
        {
            iRet = ADM_ECODE_SUCCESSED;
            log_i("No ecu waiting to download.");
            break;
        }

        //!< 检查下载环境是否满足
        iRet = adm_dpm_checkDownloadEnviroment();
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to check downlaod enviroment, err[%d].", iRet);
            (void)adm_memzero(szEventBuf, sizeof(szEventBuf));
            (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), "Failed to check downlaod enviroment, err[%d]", iRet);
            break;
        }

        //!< 检查下载前置条件是否满足
        log_i("[ABUP-Procedure] Check download Condition Start");
        iRet = adm_dpm_checkDownloadPreCondition(eTriggerType);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to check downlaod preCondition, err[%d].", iRet);
            (void)adm_memzero(szEventBuf, sizeof(szEventBuf));
            (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), "Failed to check downlaod preCondition, err[%d]", iRet);
            log_i("[ABUP-Procedure] Check download Condition End, ret[%d]",iRet);
            break;
        }
        log_i("[ABUP-Procedure] Check download Condition End, ret[%d]",iRet);

        //!< 启动下载过程监控
        log_i("[ABUP-Procedure] Download process monitor Start");
        iRet = adm_dpm_startDownloadProcessMonitor(dpmDownMonitorNotify, eTriggerType);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to start downlaod monitor, err[%d].", iRet);
            (void)adm_memzero(szEventBuf, sizeof(szEventBuf));
            (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), "Failed to start downlaod monitor, err[%d]", iRet);
            log_i("[ABUP-Procedure] Download process monitor End, ret[%d]",iRet);
            break;
        }
        log_i("[ABUP-Procedure] Download process monitor End, ret[%d]",iRet);

        (void)adm_dpm_vehicleStateReport2Cloud(ADM_IRM_REPORT_SERVER_STATUS_Task_Download_Ing, 
                    adm_dpm_errorCode2String(ADM_ECODE_SUCCESSED));
        
        /** U盘升级时，在下载前启动U盘对应的eweb服务，用于智能件的下载 */
        if (ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_USB == eTriggerType)
        {
            iRet = adm_tpm_startUsbEwebServer();
            if (ADM_IS_ERR(iRet))
            {
                log_e("Failed to start usb eweb server, err[%d]", iRet);
                (void)adm_memzero(szEventBuf, sizeof(szEventBuf));
                (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), "Failed to start usb eweb server, err[%d]",iRet);
                log_i("[ABUP-Procedure] Start usb ewebServer End, ret[%d]",iRet);
                break;
            }

            bIsUsbeWebStart = TRUE;

            /** 下载前判断U盘是否移除 */
            if (FALSE == adm_dpm_isUsbDirExist())
            {
                log_d("Udisk is removed, stop usb download");
                iRet = ADM_ECODE_USB_DIR_IS_NOT_EXIST;
                break;
            }
        }

        
        
        //!< 下载升级包
        iRet = adm_dpm_downloadPackageProc();
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to download package proc, err[%d]", iRet);
            (void)adm_memzero(szEventBuf, sizeof(szEventBuf));
            (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), "Failed to download package proc, err[%d]", iRet);
            break;
        }
    } while (FALSE);

    if (ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_USB == eTriggerType)
    {
        /** 如果是下载失败，判断是否是U盘移除导致 */
        if (ADM_IS_ERR(iRet))
        {
            /** 下载失败时判断U盘是否移除 */
            if (FALSE == adm_dpm_isUsbDirExist())
            {
                log_d("Udisk is removed, usb download Failed");
                iRet = ADM_ECODE_USB_DIR_IS_NOT_EXIST;
            }
        }

        if (TRUE == bIsUsbeWebStart)
        {
            /** U盘升级时，在下载结束后关闭U盘对应的eweb服务 */
            (DVOID)adm_tpm_stopUsbEwebServer();
        }
    }

    //!< 停止下载过程监控
    log_i("[ABUP-Procedure] stop Download process monitor Start");
    adm_dpm_stopDownloadProcessMonitor();
    log_i("[ABUP-Procedure] stop Download process monitor End");
    
    log_i("End download process func, err[%d]", iRet);

//!< tpm 依赖 ewebserver 的支持 
#ifdef CMAKE_CUSTOM_EWEBSERVER_SUPPORT
    log_i("download entry trigger transfer process, err[%d]", iRet);
    //!< 预分包作为下载的一部分 
    //!< 下载成功触发预分包流程 
    if (ADM_IS_SUCCESS(iRet))
    {
        iRet = adm_tpm_triggerTransferPkg(eTriggerType);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to trigger transfer pkg, err[%d]", iRet);
            (void)adm_memzero(szEventBuf, sizeof(szEventBuf));
            (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), "Failed to trigger transfer pkg, err[%d]", iRet);
        }
    }
    log_i("End trigger transfer process, err[%d]", iRet);
#endif

    

    do
    {
        adm_cvs_getUpgradeMode((D32S *)&eUpgradeMode);

        if (ADM_IS_SUCCESS(iRet) &&
           (ADM_CVS_UPGRADE_MODE_USB_CHECK != eUpgradeMode))
        {
            /** 确认下载任务有效性:只有云端下发任务终止时，才默认为任务有效性校验失败，其他场景，不影响下载。 */
            iRet = adm_dpm_taskVerify();
            if (ADM_ECODE_UPGRADE_TASK_STOPED == iRet)
            {
                log_e("Task has been stopped, err[%d]", iRet);
            }
            else
            {
                iRet = ADM_ECODE_SUCCESSED;
            }
        }

        //!< 产品定义：预分包完成才报下载完成
        if (ADM_ECODE_SUCCESSED == iRet)
        {
            //!< 进入下载完成
            adm_dpm_setDownloadFinished();
            
            //!< 记录下载的起始时间
            (void)clock_gettime(CLOCK_MONOTONIC, &tsNowend);
            
            /** 有安装时间点, 设置时间，进行远程下电 V23/D01车型*/
            bIsHasInstallTime = adm_dpm_getIsAnyInstallTime(&uiAutoStartTime);
            if (DTRUE == bIsHasInstallTime)
            {
                D64U relativeTime = dpmTransTime(uiAutoStartTime);

                log_i("relativeTime:%llu", relativeTime);

                ulTimestamp = (D64S)relativeTime + time(NULL);

                log_i("schduleInstallCallback ulTimestamp:%lld", ulTimestamp);
                /** 通知tbox设置预约时间 若设置失败报下载失败*/
                ADM_UIM_SOA_ACTION_AUTH_TYPE_E eAuthType = ADM_UIM_SOA_ACTION_AUTH_TYPE_ALLOW;
                iRet = g_stFuncCallbackInfo.schduleInstallCallback(ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_TSP,
                                                                ADM_APPLICATION_SCHEDULE_INSTALL_APP,
                                                                ADM_UIM_SOA_INSTALL_SCHDULE_TYPE_SCHDULED,
                                                                ulTimestamp,
                                                                &eAuthType,
                                                                DNULL);
                if (ADM_IS_ERR(iRet))
                {
                    log_i("Schdule failed. retValue[%d].", iRet);
                    iRet = ADM_ECODE_SCHEDULE_FAILED;
                }

                /** 下高压、下电、退出ota模式 */
                g_stFuncCallbackInfo.exitTspEnvCallback();
                bNeedStopWakeup = DTRUE;
            }
            else 
            {
                log_d("eUpgradeMode [%d]",eUpgradeMode);
                if (ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_TSP == eTspTriggerType)
                {
                    /** 无安装时间点,只要是tsp任务直接先下电再上电*/
                    (DVOID)g_stFuncCallbackInfo.tspPowerOffCallback();
                    nRet = g_stFuncCallbackInfo.tspPowerOnCallback();
                    if (ADM_IS_ERR(nRet))
                    {
                        iRet = ADM_ECODE_REMOTE_POWER_ON_FAIL_DOWNLOAD;

                        //!< 进入下载失败  
                        adm_dpm_setDownloadFailed();
                            
                        (void)adm_dpm_vehicleStateReport2Cloud(ADM_IRM_REPORT_SERVER_STATUS_Task_Download_Failed, 
                                    adm_dpm_errorCode2String(iRet));
                        
                        adm_dpm_addEventAndReport(ADM_COM_MOD_DPM, TRUE, ADM_EVENT_ID_DE_002, ADM_OCS_EVENT_LEVEL_INFO, "remote power on download failed!", szEventBuf);
                        adm_dpm_addEventAndReport(ADM_COM_MOD_DPM, TRUE, ADM_EVENT_ID_DE_002, ADM_OCS_EVENT_LEVEL_INFO, "remote power on download failed!", ADM_DPM_MARKER_EV_SUB_END_FAIL);

                        /** 执行上电失败，需要执行下电 */
                        (DVOID)g_stFuncCallbackInfo.tspPowerOffCallback();
                        break;
                    }
                }
            }
        }
        else
        {
            //!< 进入下载失败  
            adm_dpm_setDownloadFailed();
                
            (void)adm_dpm_vehicleStateReport2Cloud(ADM_IRM_REPORT_SERVER_STATUS_Task_Download_Failed, 
                        adm_dpm_errorCode2String(iRet));
            
            adm_dpm_addEventAndReport(ADM_COM_MOD_DPM, TRUE, ADM_EVENT_ID_DE_002, ADM_OCS_EVENT_LEVEL_INFO, "vehicle downlaod package failed!", szEventBuf);
            adm_dpm_addEventAndReport(ADM_COM_MOD_DPM, TRUE, ADM_EVENT_ID_DE_002, ADM_OCS_EVENT_LEVEL_INFO, "vehicle downlaod package failed!", ADM_DPM_MARKER_EV_SUB_END_FAIL);
        }
    } while (FALSE);

    if (ADM_IS_SUCCESS(iRet))
    {
        (void)adm_dpm_vehicleStateReport2Cloud(ADM_IRM_REPORT_SERVER_STATUS_Task_Download_Successed,
                                            adm_dpm_errorCode2String(iRet));
        adm_dpm_addEventAndReport(ADM_COM_MOD_DPM, TRUE, ADM_EVENT_ID_DE_003, ADM_OCS_EVENT_LEVEL_INFO, "vehicle downlaod package successed.", ADM_DPM_MARKER_EV_SUB_END_PASS);
    }
    else
    {
        log_d("eUpgradeMode [%d]",eUpgradeMode);
        /** 静默升级任务（强制升级，静默下载静默安装，OTA自升级)，下载阶段、安装阶段出现任何失败，上报云端"任务失败" */
        if (ADM_CVS_UPGRADE_MODE_FORCE == eUpgradeMode || 
            ADM_CVS_UPGRADE_MODE_SLIENCE_INSTALL == eUpgradeMode ||
            ADM_CVS_UPGRADE_MODE_APK_SELF == eUpgradeMode)
        {
            (void)adm_dpm_vehicleStateReport2Cloud(ADM_IRM_REPORT_SERVER_STATUS_Task_Failed_UC_IDLE, adm_dpm_errorCode2String(iRet));
        }
    }

    adm_dpm_stopDownloadProgressSimulate(); //验签传输成功后，模拟进度间隔改为0s
    
    //!< 上报当前下载结果信息
    pstResult = adm_dpm_createDownResultInfo(iRet, NULL);
    iiRet = adm_dpm_policyDownResultNotify2Uim(pstResult, "downloading");
    if (ADM_IS_ERR(iiRet))
    {
        log_e("Failed to notify download policy result, err[%d]", iiRet);
    }
    adm_dpm_freeDownResultInfo(pstResult);

    log_i("[ABUP-Procedure] trigger download End, ret[%d]",iRet);
    log_i("End trigger download process, err[%d]", iRet);

    if (DTRUE == bNeedStopWakeup)
    {
        sleep(5);
        (DVOID)adm_dim_host_execOtaTask(adm_soa_getHostTboxInstanceId(), ADM_WAKE_UP_CONTROL_TIMEOUT, ADM_DIM_SOA_HOST_OTA_TASK_TYPE_STOP_WAKE_UP, 0, &errorCode);
    }

    return iRet;
}
