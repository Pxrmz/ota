/**
 * @file    adm_ecs_getOtaConfigureSample.c
 * @brief   get ecus remote server config sample  
 * @details 获取OTA平台配置信息用例
 * @author  wangyanpeng
 * @version v0.1.0
 * @date    2020-11-06
 * @copyright www.abupdate.com
 */
#include "adm_ecm_log.h"
#include <stdio.h>

#include "adm_debug.h"
#include "adm_memory.h"
#include "adm_ecs_rsConfig.h"
#include "adm_ecm_management.h"
#include "adm_com_lib.h"
#include "adm_com_errorCode.h"
#include "adm_com_modDefine.h"
#include "adm_cfg_api.h"
#include "adm_pub_func.h"

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
static void adm_ecs_printfRSConfigInfo(ADM_ECS_RSCONFIGINFO_T *pstRsCfgInfo)
{
    D32S i = 0;

    if (ADM_IS_NULL(pstRsCfgInfo))
    {
        return;
    }

    log_e("protocolVersion[%s]", pstRsCfgInfo->stGeneralCfg.protocolVersion);
    log_e("ecu num[%d]", pstRsCfgInfo->stPartCfg.stEcuCfgList.num);
    for (i = 0; i < pstRsCfgInfo->stPartCfg.stEcuCfgList.num; i++)
    {
        log_e("    ecuReqId[%s]", pstRsCfgInfo->stPartCfg.stEcuCfgList.pstEcuCfg[i].ecuReqId);
        log_e("    ecuSwDid[%s]", pstRsCfgInfo->stPartCfg.stEcuCfgList.pstEcuCfg[i].ecuSwDid);
        log_e("    ecuResponseId[%s]", pstRsCfgInfo->stPartCfg.stEcuCfgList.pstEcuCfg[i].ecuResponseId);
        log_e("    ecuFunctionId[%s]", pstRsCfgInfo->stPartCfg.stEcuCfgList.pstEcuCfg[i].ecuFunctionId);
        log_e("    ecuReadInfoInstanceId[%s]", pstRsCfgInfo->stPartCfg.stEcuCfgList.pstEcuCfg[i].readInfoInstanceId);
        log_e("    ecuDownloadInstanceId[%s]", pstRsCfgInfo->stPartCfg.stEcuCfgList.pstEcuCfg[i].downloadInstanceId);
        log_e("<===>");
    }

    return;
}




D32S main(void)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    DCHAR *confVersion = NULL;
    ADM_ECS_RSCONFIGINFO_T *pstRsCfgInfo = NULL;
    const DCHAR* dirStr = NULL;
    //adm_initLog();
    dirStr = adm_cfg_iniParserGetString(ADM_CFG_VUS_LOG_STORAGE_DIR, ADM_CFG_VUS_LOG_STORAGE_DIR_DEFAULT_VAL);
    (void)adm_com_createDir(dirStr);
    adm_debug_logInit(dirStr, (1 * 1024 * 1024), 5);

    (void)adm_core_service_init();

    iRet = adm_ecm_moduleInit();
    if (ADM_IS_ERR(iRet)) 
    {
        log_e("adm_ecm_moduleInit failed, iRet:%d", iRet);
        return iRet;
    } 
    log_i("adm_ecm_moduleInit successed");

    pstRsCfgInfo = adm_ecs_newRSConfigInfo();
    if (ADM_IS_NULL(pstRsCfgInfo))
    {
        log_e("Failed to newRSConfigInfo, err[%d]", ADM_ECODE_NO_MEMORY);
        return ADM_ECODE_NO_MEMORY;
    }

    log_e("Get ecs config info.");
    iRet = adm_ecs_getConfigInfo(pstRsCfgInfo);
    if (ADM_IS_ERR(iRet))
    {
        adm_ecs_delRSConfigInfo(&pstRsCfgInfo);
        log_e("Failed to get configure info, err[%d]", iRet);
        return iRet;
    }
    adm_ecs_printfRSConfigInfo(pstRsCfgInfo);
    
    confVersion = adm_ecs_getRsConfigVersion();
    if (ADM_IS_NULL(confVersion))
    {
        log_e("Failed to get configure version");
        return ADM_ECODE_FAILED;
    }
    log_e("===> conVersion[%s]", confVersion);
    adm_free(confVersion);
    
    adm_ecs_cleanRSConfigInfo(pstRsCfgInfo);
    log_e("Update ecs config info.");
    iRet = adm_ecs_updateAndGetConfigInfo(NULL);
    if (ADM_IS_ERR(iRet))
    {
        adm_ecs_delRSConfigInfo(&pstRsCfgInfo);
        log_e("Failed to get configure info, err[%d]", iRet);
        return iRet;
    }

    log_e("Get ecs config info.");
    iRet = adm_ecs_getConfigInfo(pstRsCfgInfo);
    if (ADM_IS_ERR(iRet))
    {
        adm_ecs_delRSConfigInfo(&pstRsCfgInfo);
        log_e("Failed to get configure info, err[%d]", iRet);
        return iRet;
    }
    adm_ecs_printfRSConfigInfo(pstRsCfgInfo);
    
    confVersion = adm_ecs_getRsConfigVersion();
    if (ADM_IS_NULL(confVersion))
    {
        log_e("Failed to get configure version");
        return ADM_ECODE_FAILED;
    }
    log_e("===> conVersion[%s]", confVersion);
    adm_free(confVersion);
    adm_ecs_delRSConfigInfo(&pstRsCfgInfo);


    adm_deinitLog();
    adm_core_service_deinit();

    return ADM_ECODE_SUCCESSED;
}







