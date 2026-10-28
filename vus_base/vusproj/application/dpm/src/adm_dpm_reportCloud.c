/**
 * @file    adm_dpm_report.c
 * @brief   dpm report msg to uim or cloud 
 * @details dpm 上报消息到 uim 或 cloud
 * @author  wangyanpeng
 * @version v0.1.0
 * @date    2020-11-06
 * @copyright www.abupdate.com
 */
#include "adm_dpm_log.h"
//#include "adm_dpm_report.h"
//#include "adm_thread.h"
//#include "pthread.h"
//#include <unistd.h>
//#include <string.h>
//#include <errno.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include "json-c/json.h"
#include "adm_com_modDefine.h"

#include "adm_vrs_registerInfo.h"
#include "adm_vrm_register.h"
#include "adm_dpm_uim.h"
#include "adm_dpm_reportCloud.h"
#include "adm_ecs_rsConfig.h"
#include "adm_ecs_parsePolicy.h"

#include "adm_irm_report.h"
#include "adm_eim_getEcuInfo.h"
#include "adm_cfg_api.h"
#include "adm_eim_ecuInfoManager.h"


#if 0
#endif

/** 事件缓存的最大上报个数 */
#define ADM_DPM_EVENT_MAX_NUM   (10) 
//!< 事件描述信息的长度
#define EVNET_DESC_LEN  (512)

/*!
 * @brief 事件节点
 */
typedef struct stDPMEventNode
{
    struct list_head stHead;            //!< 双向链节点  

    ADM_OCS_EVENTS_INFO_T stEventInfo;  //!< list长度 
} ADM_DPM_EVENTNODE_T;

/*!
 * @brief 事件队列
 */
typedef struct stDpmEventList
{
    pthread_mutex_t  stMutex;   //!< 互斥锁 
    struct list_head stHead;    //!< 双向链表头  
    D32S             s32Cnt;    //!< list长度 
} ADM_DPM_EVENTLIST_T;


#if 0
#endif


/*!
 * @brief 获取平台配置请求参数
 */
typedef struct stReportCloudParam_
{
    DCHAR *sdkVersion;                  //!< SDK版本号
    DCHAR *sessionId;                   //!< 会话Id(check接口响应的)
    D64S   taskId;                      //!< 任务Id(check接口响应的)
    D64S   strategyId;                  //!< 策略Id(check接口响应的)
} ADM_DPM_RPTCLOUDPARAM_T;

/*!
 * @brief dpm 上报云端信息
 */
typedef struct stReportCloudInfo_
{
    DBOOL                    isInited;      //!< 模块是否初始化

    pthread_rwlock_t         stRWlock;      //!< 读写锁 --- 锁 stRptParam 
    ADM_DPM_RPTCLOUDPARAM_T  stRptParam;    //!< 上报参数 
    ADM_DPM_EVENTLIST_T      stEventList;   //!< 事件队列 
} ADM_DPM_REPORTCLOUDINFO_T;

#if 0
#endif

static ADM_DPM_REPORTCLOUDINFO_T gstDpmRptCloudHandle = {0};

//!< 进度上报类型 
#define ADM_DPM_RPT_PROGRESS_TYPE_DOWN      "1"
#define ADM_DPM_RPT_PROGRESS_TYPE_TRANS     "2"


#if 0
#endif

/**
 * @brief 检查给定的事件ID是否对应一个关键事件。
 * 
 * @param pcEventId 事件ID字符串的指针。
 * @return 返回DBOOL类型的值，指示该事件是否为关键事件。
 */
static DBOOL dpmIsCriticalEvent(const DCHAR *pcEventId)
{
    DBOOL bIsCritical = FALSE;

    if (NULL == pcEventId) 
    {
        return FALSE;
    }

    if (0 == adm_strcasecmp(pcEventId, "DT_001") ||
        0 == adm_strcasecmp(pcEventId, "DP_002") ||
        0 == adm_strcasecmp(pcEventId, "DP_005") ||
        0 == adm_strcasecmp(pcEventId, "DP_011") ||
        0 == adm_strcasecmp(pcEventId, "DP_014") ||
        0 == adm_strcasecmp(pcEventId, "DP_020") ||
        0 == adm_strcasecmp(pcEventId, "DP_026") ||
        0 == adm_strcasecmp(pcEventId, "DP_032") ||
        0 == adm_strcasecmp(pcEventId, "DP_035") ||
        0 == adm_strcasecmp(pcEventId, "DP_038") ||
        0 == adm_strcasecmp(pcEventId, "DP_044") ||
        0 == adm_strcasecmp(pcEventId, "DP_047") ||
        0 == adm_strcasecmp(pcEventId, "DP_050") ||
        0 == adm_strcasecmp(pcEventId, "DI_005") ||
        0 == adm_strcasecmp(pcEventId, "DI_008") ||
        0 == adm_strcasecmp(pcEventId, "DI_011") ||
        0 == adm_strcasecmp(pcEventId, "DI_014") ||
        0 == adm_strcasecmp(pcEventId, "DI_017") ||
        0 == adm_strcasecmp(pcEventId, "DI_020") ||
        0 == adm_strcasecmp(pcEventId, "DI_029") ||
        0 == adm_strcasecmp(pcEventId, "DI_032") ||
        0 == adm_strcasecmp(pcEventId, "DI_035") ||
        0 == adm_strcasecmp(pcEventId, "DI_038") ||
        0 == adm_strcasecmp(pcEventId, "DI_041") ||
        0 == adm_strcasecmp(pcEventId, "DI_047") ||
        0 == adm_strcasecmp(pcEventId, "DI_050") ||
        0 == adm_strcasecmp(pcEventId, "DI_053") ||
        0 == adm_strcasecmp(pcEventId, "DI_056") ||
        0 == adm_strcasecmp(pcEventId, "DI_059") ||
        0 == adm_strcasecmp(pcEventId, "DI_060") ||
        0 == adm_strcasecmp(pcEventId, "DI_061") ||
        0 == adm_strcasecmp(pcEventId, "DI_062") ||
        0 == adm_strcasecmp(pcEventId, "DI_063") ||
        0 == adm_strcasecmp(pcEventId, "DI_064") ||
        0 == adm_strcasecmp(pcEventId, "S_DE_001") ||
        0 == adm_strcasecmp(pcEventId, "S_DE_002") ||
        0 == adm_strcasecmp(pcEventId, "S_DE_003"))
    {
        bIsCritical = TRUE;
    }

    return bIsCritical;
}

/**
 * @brief 检查给定的事件ID是否对应一个关键事件。
 * 
 * @param pcEventId 事件ID字符串的指针。
 * @return 返回DBOOL类型的值，指示该事件是否为关键事件。
 */
static DBOOL tpmIsCriticalEvent(const DCHAR *pcEventId)
{
    DBOOL bIsCritical = FALSE;

    if (NULL == pcEventId) 
    {
        return FALSE;
    }

    if (0 == adm_strcasecmp(pcEventId, "TT_001") ||
        0 == adm_strcasecmp(pcEventId, "TP_002") ||
        0 == adm_strcasecmp(pcEventId, "TP_005") ||
        0 == adm_strcasecmp(pcEventId, "TP_008") ||
        0 == adm_strcasecmp(pcEventId, "TP_011") ||
        0 == adm_strcasecmp(pcEventId, "TP_014") ||
        0 == adm_strcasecmp(pcEventId, "TP_017") ||
        0 == adm_strcasecmp(pcEventId, "TP_019") ||
        0 == adm_strcasecmp(pcEventId, "TP_022") ||
        0 == adm_strcasecmp(pcEventId, "TP_025") ||
        0 == adm_strcasecmp(pcEventId, "TP_027") ||
        0 == adm_strcasecmp(pcEventId, "TP_028") ||
        0 == adm_strcasecmp(pcEventId, "TI_002") ||
        0 == adm_strcasecmp(pcEventId, "TI_005") ||
        0 == adm_strcasecmp(pcEventId, "TI_008") ||
        0 == adm_strcasecmp(pcEventId, "TI_010") ||
        0 == adm_strcasecmp(pcEventId, "TI_011") ||
        0 == adm_strcasecmp(pcEventId, "TI_020") ||
        0 == adm_strcasecmp(pcEventId, "S_TE_001") ||
        0 == adm_strcasecmp(pcEventId, "S_TE_002") ||
        0 == adm_strcasecmp(pcEventId, "S_TE_003") ||
        0 == adm_strcasecmp(pcEventId, "UP_002"))
    {
        bIsCritical = TRUE;
    }

    return bIsCritical;
}

/**
 * @brief 判断是否统计事件
 * @param[in]  pcEventId 事件id
 * @retval    DBOOL
 */
static DBOOL dpmIsStatisticsisEvent(const DCHAR *pcEventId)
{
    DBOOL bIsStatisticsisEvent = FALSE;

    // log_d("eventId[%s]", pcEventId);

    /** 判断事件ID中是否包含PS_DL */
    if (ADM_IS_NULL(adm_strstr(pcEventId, "PS_DL")))
    {
        bIsStatisticsisEvent = FALSE;
    }
    else
    {
        bIsStatisticsisEvent = TRUE;
        // log_d("eventId[%s] is statisticsisEvent", pcEventId);
    }

    return bIsStatisticsisEvent;
}

/*!
 *******************************************************************************
 * ecu下载状态转云端可识别的字符串状态.
 *
 * \note 返回堆上内存字符串，调用者需要调用 adm_free() 释放 
 *
 * \param[in] pstResult ecs下载结果
 * \retval    DCHAR *
 *******************************************************************************
 */
static DCHAR * __adm_dpm_switchDwonEcuState(ADM_DPS_DOWNRESULT_T *pstResult)
{
    DCHAR *pcEcuState = NULL;

    if (ADM_IS_NULL(pstResult))
    {
        log_e("Invalid input param");
        return NULL;
    }

    switch (pstResult->eDownState)
    {
        case ADM_DPS_DOWNPKG_STATE_NONE:
            pcEcuState = adm_strdup(ADM_IRM_REPORT_SERVER_STATUS_Ecu_Downlaod_To_Be);
            break;
        case ADM_DPS_DOWNPKG_STATE_NORMAL:
        case ADM_DPS_DOWNPKG_STATE_SRCFINISH:
            pcEcuState = adm_strdup(ADM_IRM_REPORT_SERVER_STATUS_Ecu_Download_Ing);
            break;
        case ADM_DPS_DOWNPKG_STATE_PAUSE:
            pcEcuState = adm_strdup(ADM_IRM_REPORT_SERVER_STATUS_Ecu_Download_Paused);
            break;
        case ADM_DPS_DOWNPKG_STATE_CANCEL:
        case ADM_DPS_DOWNPKG_STATE_FAILED:
        case ADM_DPS_DOWNPKG_STATE_DECRYPTFAILED:
            pcEcuState = adm_strdup(ADM_IRM_REPORT_SERVER_STATUS_Ecu_Download_Failed);
            break;
        case ADM_DPS_DOWNPKG_STATE_WAITDECRYPT:
        case ADM_DPS_DOWNPKG_STATE_WAITPARSE:
            pcEcuState = adm_strdup(ADM_IRM_REPORT_SERVER_STATUS_Ecu_Unpack_Prepare);
            break;
        case ADM_DPS_DOWNPKG_STATE_PARSEFAILED:
            pcEcuState = adm_strdup(ADM_IRM_REPORT_SERVER_STATUS_Ecu_Unpack_Failed);
            break;
        case ADM_DPS_DOWNPKG_STATE_FINISH:
            pcEcuState = adm_strdup(ADM_IRM_REPORT_SERVER_STATUS_Ecu_Download_Finished);
            break;
        default:
            pcEcuState = adm_strdup(ADM_IRM_REPORT_SERVER_STATUS_Ecu_Download_Ing);
            log_e("UnKnown to switch iError[%d].", pstResult->eDownState);
            break;
    }

    return pcEcuState;
}


/*!
 *******************************************************************************
 * 删除ocs ecu did list对象内存信息.
 *
 * \note
 *
 * \param[in]   pstOcsEcuDidLists 对象信息
 * \retval      none
 *******************************************************************************
 */
static void __adm_dpm_delOcsEcusDidList(ADM_OCS_ECU_DID_LIST_T **pstOcsEcuDidLists)
{
    D32S i = 0;

    if (ADM_IS_NULL(pstOcsEcuDidLists) || ADM_IS_NULL(*pstOcsEcuDidLists))
    {
        return;
    }

    for (i = 0; i < (*pstOcsEcuDidLists)->ecuDidsLen; i++)
    {
        adm_free((*pstOcsEcuDidLists)->ecuDids[i].did);
        adm_free((*pstOcsEcuDidLists)->ecuDids[i].didValue);
    }
    adm_free((*pstOcsEcuDidLists)->ecuDids);
    adm_free(*pstOcsEcuDidLists);

    return;
}

/*!
 *******************************************************************************
 * 创建 ocs ecu did list 对象内存信息.
 *
 * \note 这里与平台约定 didvalue 填默认值，后续换成实际值
 *
 * \param[in] ecuReqId ecsEcuDid信息
 * \param[in] ecuSwDid ecsEcuDid信息
 * \retval    ADM_OCS_ECU_DID_LIST_T *
 *******************************************************************************
 */
static ADM_OCS_ECU_DID_LIST_T * __adm_dpm_createOcsEcusDidList(DCHAR *ecuReqId, DCHAR *ecuSwDid)
{
    D32S i = 0;
    D32S iRet = ADM_ECODE_SUCCESSED;
    ADM_OCS_ECU_DID_LIST_T *pstOcsEcuDidLists = NULL;
    ADM_DIM_SOA_DID_DATA_ARRAY_T *singleEcuDidList = NULL;
    ADM_EIM_GETECUINFO_T* allEcuInfoList = NULL;

    if (ADM_IS_NULL(ecuReqId) || ADM_IS_NULL(ecuSwDid))
    {
        log_e("Invalid input param");
        return NULL;
    }

    iRet = adm_eim_getEcuInfoManagerKvGet(&allEcuInfoList);
    if (ADM_IS_ERR(iRet))
    {          
        log_e("Failed to get general ecu dids, err[%d]", iRet);
        return NULL ;
    }

    for(i=0; i<allEcuInfoList->num; i++)
    {
        if ((0 == adm_strcasecmp(ecuReqId, allEcuInfoList->ecuInfo[i]->ecuReqId)) &&
            (0 == adm_strcasecmp(ecuSwDid, allEcuInfoList->ecuInfo[i]->ecuSwDid)))
        {
            singleEcuDidList = allEcuInfoList->ecuInfo[i]->vdiDidListResponse;
            break;
        }

    }

    if (ADM_IS_NULL(singleEcuDidList))
    {   
        log_e("Failed to get ecu config, err[%d]", ADM_ECODE_EIM_GETECUINFO_FAILED);
        return NULL;
    }
    log_i("ecus did list num[%d]", singleEcuDidList->didArrayLength);


    pstOcsEcuDidLists = (ADM_OCS_ECU_DID_LIST_T *)adm_malloc(sizeof(*pstOcsEcuDidLists));
    if (ADM_IS_NULL(pstOcsEcuDidLists))
    {
        log_e("Failed to malloc, err[%d]", ADM_ECODE_NO_MEMORY);
        return NULL;
    }
    (void)adm_memzero(pstOcsEcuDidLists, sizeof(*pstOcsEcuDidLists));
    
    pstOcsEcuDidLists->ecuDidsLen = (D32S)singleEcuDidList->didArrayLength;
    if (0 >= pstOcsEcuDidLists->ecuDidsLen)
    {
        pstOcsEcuDidLists->ecuDids = NULL;
        return pstOcsEcuDidLists;
    }
    
    pstOcsEcuDidLists->ecuDids = 
            (ADM_OCS_ECU_DID_T *)adm_malloc(sizeof(ADM_OCS_ECU_DID_T) * (D32U)pstOcsEcuDidLists->ecuDidsLen);
    if (ADM_IS_NULL(pstOcsEcuDidLists->ecuDids))
    {
        log_e("Failed to malloc, err[%d]", ADM_ECODE_NO_MEMORY);
        return pstOcsEcuDidLists;
    }
    (void)adm_memzero(pstOcsEcuDidLists->ecuDids, sizeof(ADM_OCS_ECU_DID_T) * (D32U)pstOcsEcuDidLists->ecuDidsLen);
    
    for (i = 0; i < pstOcsEcuDidLists->ecuDidsLen; i++)
    {
        DCHAR did[128] = {0};
        (void)adm_snprintf(did, sizeof(did)/sizeof(DCHAR), "0X%X", singleEcuDidList->didArray[i]->did);
        pstOcsEcuDidLists->ecuDids[i].did = adm_strdup(did);
        
        adm_memzero(did, sizeof(did));
        switch ((VARIABLE_TYPE_EN)singleEcuDidList->didArray[i]->didData->dataType)
        {
        case VAR_U8:
            (void)adm_snprintf(did, sizeof(did)/sizeof(DCHAR), "%d", singleEcuDidList->didArray[i]->didData->v.value[0]);
            pstOcsEcuDidLists->ecuDids[i].didValue = adm_strdup(did);
            break;
        case VAR_U32:
            (void)adm_snprintf(did, sizeof(did)/sizeof(DCHAR), "%d", singleEcuDidList->didArray[i]->didData->v.valueU32);
            pstOcsEcuDidLists->ecuDids[i].didValue = adm_strdup(did);
            break;
        case VAR_S32:
            (void)adm_snprintf(did, sizeof(did)/sizeof(DCHAR), "%d", singleEcuDidList->didArray[i]->didData->v.valueS32);
            pstOcsEcuDidLists->ecuDids[i].didValue = adm_strdup(did);
            break;
        case VAR_U64:
            (void)adm_snprintf(did, sizeof(did)/sizeof(DCHAR), "%ld", singleEcuDidList->didArray[i]->didData->v.valueU64);
            pstOcsEcuDidLists->ecuDids[i].didValue = adm_strdup(did);
            break;
        case VAR_S64:
            (void)adm_snprintf(did, sizeof(did)/sizeof(DCHAR), "%ld", singleEcuDidList->didArray[i]->didData->v.valueS64);
            pstOcsEcuDidLists->ecuDids[i].didValue = adm_strdup(did);
            break;
        case VAR_FLOAT:
            (void)adm_snprintf(did, sizeof(did)/sizeof(DCHAR), "%f", singleEcuDidList->didArray[i]->didData->v.valueF);
            pstOcsEcuDidLists->ecuDids[i].didValue = adm_strdup(did);
            break;
        case VAR_STRING:
            pstOcsEcuDidLists->ecuDids[i].didValue = adm_strdup((DCHAR*)singleEcuDidList->didArray[i]->didData->v.valueS);
            break;

        default:
            break;
        }

        pstOcsEcuDidLists->ecuDids[i].start = (D32S)singleEcuDidList->didArray[i]->startByte;
        pstOcsEcuDidLists->ecuDids[i].end = (D32S)singleEcuDidList->didArray[i]->endByte;
    }

    return pstOcsEcuDidLists;
}

/*!
 *******************************************************************************
 * 删除 ocs ecu state list 对象内存信息..
 *
 * \note
 *
 * \param[in]   pstEcusStateList 对象信息
 * \retval      none
 *******************************************************************************
 */
static void __adm_dpm_delEcusStateList(ADM_OCS_ECU_STATE_INFO_LIST_T **pstEcusStateList)
{
    D32S i = 0;

    if (ADM_IS_NULL(pstEcusStateList) || ADM_IS_NULL(*pstEcusStateList))
    {
        return;
    }

    for (i = 0; i < (*pstEcusStateList)->reportEcusLen; i++)
    {
        adm_free((*pstEcusStateList)->reportEcus[i].ecuReqId);
        adm_free((*pstEcusStateList)->reportEcus[i].ecuSwDid);
        adm_free((*pstEcusStateList)->reportEcus[i].ecuState);
        adm_free((*pstEcusStateList)->reportEcus[i].ecuReason);
        adm_free((*pstEcusStateList)->reportEcus[i].partNumber);
        adm_free((*pstEcusStateList)->reportEcus[i].softwarePartNumber);
        adm_free((*pstEcusStateList)->reportEcus[i].partSupplier);
        adm_free((*pstEcusStateList)->reportEcus[i].partHardwareNum);
        __adm_dpm_delOcsEcusDidList(&(*pstEcusStateList)->reportEcus[i].ecuDidLists);
    }
    adm_free((*pstEcusStateList)->reportEcus);
    adm_free(*pstEcusStateList);

    return;
}

/*!
 *******************************************************************************
 * 创建 ocs ecu state list 对象内存信息.
 *
 * \note 
 *
 * \param[in] pstDownPkgResult 下载升级包结果信息
 * \retval    ADM_OCS_ECU_STATE_INFO_LIST_T * 
 *******************************************************************************
 */
static ADM_OCS_ECU_STATE_INFO_LIST_T * __adm_dpm_createEcusStateList
(
    ADM_DPS_DOWNPKGRESULT_T *pstDownPkgResult
)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    D32S i = 0;
    ADM_ECS_ECUDIDLIST_T stEcuDidList = {0};
    ADM_ECS_EXTERN_ECU_INFO_T* pstExternEcuInfo = NULL;
    ADM_OCS_ECU_STATE_INFO_LIST_T* pstEcusStateList = NULL;

    if (ADM_IS_NULL(pstDownPkgResult))
    {
        log_e("Invalid input param");
        return NULL;
    }

    pstEcusStateList = (ADM_OCS_ECU_STATE_INFO_LIST_T *)adm_malloc(sizeof(*pstEcusStateList));
    if (ADM_IS_NULL(pstEcusStateList))
    {
        log_e("Failed to malloc, err[%d]", ADM_ECODE_NO_MEMORY);
        return NULL;
    }
    (void)adm_memzero(pstEcusStateList, sizeof(*pstEcusStateList));
    
    pstEcusStateList->reportEcusLen = pstDownPkgResult->num;
    if (0 >= pstEcusStateList->reportEcusLen)
    {
        pstEcusStateList->reportEcusLen = 0;
        pstEcusStateList->reportEcus = NULL;
        return pstEcusStateList;
    }
    
    pstEcusStateList->reportEcus = 
            (ADM_OCS_ECU_STATE_INFO_T *)adm_malloc(sizeof(ADM_OCS_ECU_STATE_INFO_T) * (D32U)pstEcusStateList->reportEcusLen);
    if (ADM_IS_NULL(pstEcusStateList))
    {
        adm_free(pstEcusStateList);
        log_e("Failed to malloc, err[%d]", ADM_ECODE_NO_MEMORY);
        return NULL;
    }
    (void)adm_memzero(pstEcusStateList->reportEcus, sizeof(ADM_OCS_ECU_STATE_INFO_T) * (D32U)pstEcusStateList->reportEcusLen);
        
    for (i = 0; i < pstEcusStateList->reportEcusLen; i++)
    {
        // TODO:   待完善 ecuDid didvalue 信息
        (void)adm_memzero(&stEcuDidList, sizeof(stEcuDidList));
        pstExternEcuInfo = (ADM_ECS_EXTERN_ECU_INFO_T*)adm_malloc(sizeof(ADM_ECS_EXTERN_ECU_INFO_T));
        if (ADM_IS_NULL(pstExternEcuInfo))
        {
            return NULL;
        }

        iRet = adm_ecs_getOneEcuDidListInfo(pstDownPkgResult->pstResultList[i].ecuReqid,
                                            pstDownPkgResult->pstResultList[i].ecuSwDid,
                                            &stEcuDidList,
                                            pstExternEcuInfo);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to get ecu dids, ecuReqid[%s], ecuSwDid[%s], err[%d]", 
                    pstDownPkgResult->pstResultList[i].ecuReqid, 
                    pstDownPkgResult->pstResultList[i].ecuSwDid, iRet);
            break;
        }
        log_i("ecus did list num[%d]", stEcuDidList.num);

        pstEcusStateList->reportEcus[i].ecuDidLists = __adm_dpm_createOcsEcusDidList(pstDownPkgResult->pstResultList[i].ecuReqid,
                                                                                    pstDownPkgResult->pstResultList[i].ecuSwDid);
        if (ADM_IS_NULL(pstEcusStateList->reportEcus[i].ecuDidLists))
        {
            iRet = ADM_ECODE_NO_MEMORY;
            adm_ecs_delEcuDidList(stEcuDidList.num, &stEcuDidList.pstEcuDidInfo);
            log_e("Failed to create ocs ecus did list, err[%d]", iRet);
            break;
        }
        adm_ecs_delEcuDidList(stEcuDidList.num, &stEcuDidList.pstEcuDidInfo);

        pstEcusStateList->reportEcus[i].endTime   = 0;
        pstEcusStateList->reportEcus[i].beginTime = 0;
        pstEcusStateList->reportEcus[i].ecuReqId  = adm_strdup(pstDownPkgResult->pstResultList[i].ecuReqid);
        pstEcusStateList->reportEcus[i].ecuSwDid  = adm_strdup(pstDownPkgResult->pstResultList[i].ecuSwDid);
        pstEcusStateList->reportEcus[i].partNumber = adm_strdup
        (pstExternEcuInfo->partNumber);
        pstEcusStateList->reportEcus[i].partSupplier = adm_strdup
        (pstExternEcuInfo->partSupplier);
        pstEcusStateList->reportEcus[i].softwarePartNumber = adm_strdup
        (pstExternEcuInfo->softwarePartNumber);
        pstEcusStateList->reportEcus[i].partHardwareNum = adm_strdup
        (pstExternEcuInfo->partHardwareNum);

        adm_ecs_delExternEcuInfo(&pstExternEcuInfo);
    
        pstEcusStateList->reportEcus[i].ecuState = __adm_dpm_switchDwonEcuState(&pstDownPkgResult->pstResultList[i].stResult);
        pstEcusStateList->reportEcus[i].ecuReason = 
            (ADM_IS_NULL(pstDownPkgResult->pstResultList[i].stResult.failReason) ? adm_strdup("0") : \
            adm_strdup(pstDownPkgResult->pstResultList[i].stResult.failReason));
    }        
    
    //!< for 循环中有错误
    if (ADM_IS_ERR(iRet))
    {
        __adm_dpm_delEcusStateList(&pstEcusStateList);
        return NULL;
    }

    return pstEcusStateList;
}


/*!
 *******************************************************************************
 * 删除 ocs ecu progress list 对象内存信息.
 *
 * \note
 *
 * \param[in]   pstEcusProgressList 对象信息
 * \retval      none
 *******************************************************************************
 */
static void __adm_dpm_delEcusProgressList
(
    ADM_OCS_ECU_PROGRESS_INFO_LIST_T **pstEcusProgressList
)
{
    D32S i = 0;

    if (ADM_IS_NULL(pstEcusProgressList) || ADM_IS_NULL(*pstEcusProgressList))
    {
        return;
    }

    for (i = 0; i < (*pstEcusProgressList)->reportEcusLen; i++)
    {
        adm_free((*pstEcusProgressList)->reportEcus[i].ecuReqId);
        adm_free((*pstEcusProgressList)->reportEcus[i].ecuSwDid);
        adm_free((*pstEcusProgressList)->reportEcus[i].progressType);
        adm_free((*pstEcusProgressList)->reportEcus[i].partHardwareNum);
        adm_free((*pstEcusProgressList)->reportEcus[i].partNumber);
        adm_free((*pstEcusProgressList)->reportEcus[i].partSupplier);
        adm_free((*pstEcusProgressList)->reportEcus[i].softwarePartNumber);
        //__adm_dpm_delOcsEcusDidList(&(*pstEcusProgressList)->reportEcus[i].ecuDidLists);
    }
    adm_free((*pstEcusProgressList)->reportEcus);
    adm_free((*pstEcusProgressList)->progressType);
    adm_free(*pstEcusProgressList);

    return;
}

/*!
 *******************************************************************************
 * 创建 ocs ecu progress list 对象内存信息.
 *
 * \note 
 *
 * \param[in] pstDownPkgProgress 下载升级包进度信息
 * \retval    ADM_OCS_ECU_PROGRESS_INFO_LIST_T
 *******************************************************************************
 */
static ADM_OCS_ECU_PROGRESS_INFO_LIST_T *__adm_dpm_createEcusProgressList
(
    ADM_DPS_DOWNPKGPROGRESS_T *pstDownPkgProgress
)
{
    D32S i = 0;
    D32S iRet = ADM_ECODE_SUCCESSED;
    ADM_OCS_ECU_PROGRESS_INFO_LIST_T *pstEcusProgressList = NULL;
    ADM_ECS_EXTERN_ECU_INFO_T* pstExternEcuInfo = NULL;
    ADM_ECS_ECUDIDLIST_T stEcuDidList = {0};
    D64S ulTotalSize = 0;
    D64S ulSaveSize = 0;

    if (ADM_IS_NULL(pstDownPkgProgress))
    {
        log_e("Invalid input param");
        return NULL;
    }

    pstEcusProgressList = (ADM_OCS_ECU_PROGRESS_INFO_LIST_T *)adm_malloc(sizeof(*pstEcusProgressList));
    if (ADM_IS_NULL(pstEcusProgressList))
    {
        log_e("Failed to malloc, err[%d]", ADM_ECODE_NO_MEMORY);
        return NULL;
    }
    (void)adm_memzero(pstEcusProgressList, sizeof(*pstEcusProgressList));
    
    pstEcusProgressList->reportEcusLen = pstDownPkgProgress->num;
    if (0 >= pstEcusProgressList->reportEcusLen)
    {
        pstEcusProgressList->reportEcus = NULL;
        return pstEcusProgressList;
    }
    
    for (i = 0; i < pstDownPkgProgress->num; i++)
    {
        ulTotalSize = ulTotalSize + pstDownPkgProgress->pstProgressList[i].stProgress.totalBytes;
    }

    for (i = 0; i < pstDownPkgProgress->num; i++)
    {
        ulSaveSize = ulSaveSize + pstDownPkgProgress->pstProgressList[i].stProgress.saveBytes;
    }

    pstEcusProgressList->progressType = adm_strdup(ADM_DPM_RPT_PROGRESS_TYPE_DOWN);
    pstEcusProgressList->iTotalProgress = (D32S)((ulSaveSize * 100) / ulTotalSize);
    pstEcusProgressList->iTotalProgress = g_DownloadProgressSimulate;

    pstEcusProgressList->reportEcus = 
            (ADM_OCS_ECU_PROGRESS_INFO_T *)adm_malloc(sizeof(ADM_OCS_ECU_PROGRESS_INFO_T) * (D32U)pstEcusProgressList->reportEcusLen);
    if (ADM_IS_NULL(pstEcusProgressList->reportEcus))
    {
        log_e("Failed to malloc, err[%d]", ADM_ECODE_NO_MEMORY);
        return NULL;
    }
    (void)adm_memzero(pstEcusProgressList->reportEcus, sizeof(ADM_OCS_ECU_PROGRESS_INFO_T) * (D32U)pstEcusProgressList->reportEcusLen);

    for (i = 0; i < pstEcusProgressList->reportEcusLen; i++)
    {
        pstEcusProgressList->reportEcus[i].ecuReqId = adm_strdup(pstDownPkgProgress->pstProgressList[i].ecuReqid);
        pstEcusProgressList->reportEcus[i].ecuSwDid = adm_strdup(pstDownPkgProgress->pstProgressList[i].ecuSwDid);
        pstEcusProgressList->reportEcus[i].progressType = adm_strdup(ADM_DPM_RPT_PROGRESS_TYPE_DOWN);

        //!< add progress 
        pstEcusProgressList->reportEcus[i].progress = (D32S)((pstDownPkgProgress->pstProgressList[i].stProgress.saveBytes * 100) / 
                                pstDownPkgProgress->pstProgressList[i].stProgress.totalBytes);
    } 

    /** 获取零件硬件号等信息 */
    for (i = 0; i < pstEcusProgressList->reportEcusLen; i++)
    {
        // TODO:   待完善 ecuDid didvalue 信息
        (void)adm_memzero(&stEcuDidList, sizeof(stEcuDidList));
        pstExternEcuInfo = (ADM_ECS_EXTERN_ECU_INFO_T*)adm_malloc(sizeof(ADM_ECS_EXTERN_ECU_INFO_T));
        if (ADM_IS_NULL(pstExternEcuInfo))
        {
            return NULL;
        }

        iRet = adm_ecs_getOneEcuDidListInfo(pstEcusProgressList->reportEcus[i].ecuReqId,
                                            pstEcusProgressList->reportEcus[i].ecuSwDid,
                                            &stEcuDidList,
                                            pstExternEcuInfo);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to get ecu dids, ecuReqid[%s], ecuSwDid[%s], err[%d]", 
                    pstEcusProgressList->reportEcus[i].ecuReqId, 
                    pstEcusProgressList->reportEcus[i].ecuSwDid, iRet);
            break;
        }
        log_i("ecus did list num[%d]", stEcuDidList.num);
        adm_ecs_delEcuDidList(stEcuDidList.num, &stEcuDidList.pstEcuDidInfo);

        pstEcusProgressList->reportEcus[i].partNumber = adm_strdup
        (pstExternEcuInfo->partNumber);
        pstEcusProgressList->reportEcus[i].partSupplier = adm_strdup
        (pstExternEcuInfo->partSupplier);
        pstEcusProgressList->reportEcus[i].softwarePartNumber = adm_strdup
        (pstExternEcuInfo->softwarePartNumber);
        pstEcusProgressList->reportEcus[i].partHardwareNum = adm_strdup
        (pstExternEcuInfo->partHardwareNum);

        adm_ecs_delExternEcuInfo(&pstExternEcuInfo);
    }   

    return pstEcusProgressList;
}

#if 0
#endif

/*!
 *******************************************************************************
 * 创建事件节点.
 *
 * \note 
 *
 * @param [in]  pcEventId      事件id
 * @param [in]  i32EventLevel  事件级别：1:表示INFO级别 2:表示WARN级别 3:表示ERROR级别
 * @param [in]  pcEventDesc    事件描述
 * @param [in]  pcEventContext 事件上下文(非必填字段)
 * @param [in]  ieventSequenceId 事件序列id
 * \retval      ADM_DPM_EVENTNODE_T *
 *******************************************************************************
 */
static ADM_DPM_EVENTNODE_T *__adm_dpm_createEventNode
(    
    const DCHAR *pcEventId,
    D32S   i32EventLevel,    
    DCHAR *pcEventDesc,
    const DCHAR *pcEventContext,
    D64S   ieventSequenceId
)
{
    ADM_DPM_EVENTNODE_T *pstEventNode = NULL;

    if (ADM_IS_NULL(pcEventId) || ADM_IS_NULL(pcEventDesc))
    {
        log_e("Invalid input param");
        return NULL;
    }

    pstEventNode = (ADM_DPM_EVENTNODE_T *)adm_malloc(sizeof(*pstEventNode));
    if (ADM_IS_NULL(pstEventNode))
    {
        log_e("Failed to malloc, err[%d]", ADM_ECODE_NO_MEMORY);
        return NULL;
    }
    (void)adm_memzero(pstEventNode, sizeof(*pstEventNode));
    
    INIT_LIST_HEAD(&pstEventNode->stHead);
    pstEventNode->stEventInfo.eventTime       = time(NULL);
    pstEventNode->stEventInfo.eventLevel      = i32EventLevel;
    pstEventNode->stEventInfo.eventId         = adm_strdup(pcEventId);
    pstEventNode->stEventInfo.eventDesc       = adm_strdup(pcEventDesc);
    pstEventNode->stEventInfo.eventSequenceId = ieventSequenceId;
    pstEventNode->stEventInfo.eventContext    = ((NULL != pcEventContext) ? (adm_strdup(pcEventContext)) : (adm_strdup("None")));

    return pstEventNode;
}

/*!
 *******************************************************************************
 * 释放事件节点.
 *
 * \note 
 *
 * @param [inout]  pstEventNode  事件节点
 * \retval         none
 *******************************************************************************
 */
static void __adm_dpm_freeEventNode(ADM_DPM_EVENTNODE_T *pstEventNode)
{
    if (ADM_IS_NULL(pstEventNode))
    {
        return;
    }

    adm_free(pstEventNode->stEventInfo.eventId);
    adm_free(pstEventNode->stEventInfo.eventDesc);
    adm_free(pstEventNode->stEventInfo.eventContext);
    adm_free(pstEventNode);

    return;
}

/*!
 *******************************************************************************
 * 清空事件列表内存.
 *
 * \note 
 *
 * @param [in]  pstEventsList  事件列表
 * \retval      none
 *******************************************************************************
 */
static void __adm_dpm_cleanEventListInfo
(
    ADM_OCS_EVENTS_INFO_LIST_T *pstEventsList
)
{
    D32S i = 0;
    
    if (ADM_IS_NULL(pstEventsList))
    {
        return;
    }

    for (i = 0; i < pstEventsList->reportEventsLen; i++)
    {
        adm_free(pstEventsList->reportEvents[i].eventId);
        adm_free(pstEventsList->reportEvents[i].eventDesc);
        adm_free(pstEventsList->reportEvents[i].eventContext);
    }
    adm_free(pstEventsList->reportEvents);

    return;
}

/*!
 *******************************************************************************
 * 清空事件列表内存.
 *
 * \note 
 *
 * @param [in]  pstEventsList  事件列表
 * \retval      none
 *******************************************************************************
 */
static void __adm_dpm_cleanEventCache(void)
{
    struct list_head *pstNode     = NULL;
    struct list_head *pstNextNode = NULL;
    ADM_DPM_EVENTNODE_T *pstEventNode = NULL;
    
    //!< 模块未初始化，则直接报错
    if (FALSE == gstDpmRptCloudHandle.isInited)
    {
        return;
    }

    (void)pthread_mutex_lock(&gstDpmRptCloudHandle.stEventList.stMutex);
    list_for_each_safe(pstNode, pstNextNode, &gstDpmRptCloudHandle.stEventList.stHead)
    {
        //!< 取节点
        pstEventNode = list_entry(pstNode, ADM_DPM_EVENTNODE_T, stHead);

        //摘链 and 释放节点 
        list_del(&pstEventNode->stHead);
        __adm_dpm_freeEventNode(pstEventNode);
        pstEventNode = NULL;
    }
    gstDpmRptCloudHandle.stEventList.s32Cnt = 0;
    (void)pthread_mutex_unlock(&gstDpmRptCloudHandle.stEventList.stMutex);

    return;
}


static DCHAR* __adm_dpm_getDateAndTimeStr(void)
{
    static DCHAR sTimeStr[20] = {0};
    time_t timep;
    struct tm *p;

    (void)time(&timep);
    p = localtime(&timep);

    (void)adm_snprintf(sTimeStr, sizeof(sTimeStr), "%04d-%02d-%02d %02d:%02d:%02d", 1900 + p->tm_year, p->tm_mon + 1, p->tm_mday,
            p->tm_hour, p->tm_min, p->tm_sec);
            
    return sTimeStr;
}

static DCHAR* __adm_dpm_getNewEventDescInfo(DCHAR* moduleName,const DCHAR* eventDesc)
{
    DCHAR* peventDesc = NULL;

    peventDesc = (DCHAR*)adm_malloc(EVNET_DESC_LEN);
    if (NULL == peventDesc) 
    {
        log_e("Failed to malloc, err[%d]", ADM_ECODE_NO_MEMORY);
        return NULL;
    }
    (void)adm_memset(peventDesc, 0x00, EVNET_DESC_LEN, EVNET_DESC_LEN);

    (void)adm_snprintf(peventDesc, EVNET_DESC_LEN, "[%s][%s](%s)", 
            moduleName, 
            __adm_dpm_getDateAndTimeStr(), 
            eventDesc);

    return peventDesc;
}

#if 0
#endif

/**
 * @brief   刷新事件上报缓存 
 * @detail  事件上报
 *
 * @note 
 *
 * @return  none 
 */
static void adm_dpm_fflushEventReport(void)
{
    D32S iRet = ADM_ECODE_FAILED;
    D32S i = 0;
    struct list_head *pstNode     = NULL;
    struct list_head *pstNextNode = NULL;
    ADM_DPM_EVENTNODE_T *pstEventNode = NULL;
    ADM_OCS_EVENTS_INFO_LIST_T stEventsList = {0};

    //!< 模块未初始化，则直接报错
    if (FALSE == gstDpmRptCloudHandle.isInited)
    {
        log_e("ECm module not init, err[%d]", ADM_ECODE_MODULE_UNINIT);
        return;
    }

    (void)pthread_mutex_lock(&gstDpmRptCloudHandle.stEventList.stMutex);
    (void)adm_memzero(&stEventsList, sizeof(stEventsList));
    stEventsList.reportEvents = (ADM_OCS_EVENTS_INFO_T *)adm_malloc((D32U)gstDpmRptCloudHandle.stEventList.s32Cnt * sizeof(ADM_OCS_EVENTS_INFO_T));
    if (ADM_IS_NULL(stEventsList.reportEvents))
    {
        (void)pthread_mutex_unlock(&gstDpmRptCloudHandle.stEventList.stMutex);
        log_e("Failed to malloc, err[%d]", ADM_ECODE_NO_MEMORY);
        return;
    }
    (void)adm_memzero(stEventsList.reportEvents, ((D32U)gstDpmRptCloudHandle.stEventList.s32Cnt * sizeof(ADM_OCS_EVENTS_INFO_T)));
    stEventsList.reportEventsLen = gstDpmRptCloudHandle.stEventList.s32Cnt;

    list_for_each_safe(pstNode, pstNextNode, &gstDpmRptCloudHandle.stEventList.stHead)
    {
        //!< 取节点
        pstEventNode = list_entry(pstNode, ADM_DPM_EVENTNODE_T, stHead);
        
        stEventsList.reportEvents[i].eventTime       = pstEventNode->stEventInfo.eventTime;
        stEventsList.reportEvents[i].eventLevel      = pstEventNode->stEventInfo.eventLevel;
        stEventsList.reportEvents[i].eventId         = adm_strdup(pstEventNode->stEventInfo.eventId);
        stEventsList.reportEvents[i].eventDesc       = adm_strdup(pstEventNode->stEventInfo.eventDesc);
        stEventsList.reportEvents[i].eventSequenceId = pstEventNode->stEventInfo.eventSequenceId;
        stEventsList.reportEvents[i].eventContext = 
                    ((NULL != pstEventNode->stEventInfo.eventContext) ? (adm_strdup(pstEventNode->stEventInfo.eventContext)) : (adm_strdup("None")));

        //!< debug
        //adm_ecs_AddEventsinfotofile_test(stEventsList.reportEvents[i].eventId,
        //                                 stEventsList.reportEvents[i].eventLevel,
        //                                 stEventsList.reportEvents[i].eventDesc,
        //                                 stEventsList.reportEvents[i].eventContext);

        ++i;

        //摘链 and 释放节点 
        list_del(&pstEventNode->stHead);
        __adm_dpm_freeEventNode(pstEventNode);
        pstEventNode = NULL;

        //!< 做个保护，其实没必要 
        if (i == stEventsList.reportEventsLen)
        {
            break;
        }
    }

    gstDpmRptCloudHandle.stEventList.s32Cnt = 0;
    (void)pthread_mutex_unlock(&gstDpmRptCloudHandle.stEventList.stMutex);

    //!< 正式上报 
    if (0 < stEventsList.reportEventsLen)
    {
        (void)pthread_rwlock_rdlock(&gstDpmRptCloudHandle.stRWlock);
        iRet = adm_irm_reportEvent(gstDpmRptCloudHandle.stRptParam.sessionId, 
                    gstDpmRptCloudHandle.stRptParam.taskId,
                    gstDpmRptCloudHandle.stRptParam.strategyId,
                    time(NULL), 
                    gstDpmRptCloudHandle.stRptParam.sdkVersion, 
                    NULL, 
                    &stEventsList);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to report Event, err[%d]", iRet);
        }
        (void)pthread_rwlock_unlock(&gstDpmRptCloudHandle.stRWlock);

        //!< 清空内存 
        __adm_dpm_cleanEventListInfo(&stEventsList);
    }

    return;
}

/**
 * @brief   模块封装的事件上报接口 
 * @detail  事件上报
 *
 * @note 
 * @param [in]  eModuleID 模块ID
 * @param [in]  isReportImmediate 是否需要立即上报
 * @param [in]  pcEventId      事件id
 * @param [in]  i32EventLevel  事件级别：1:表示TRACE级别 2:表示DEBUG级别 3:表示INFO级别  4:表示WARN级别 5:表示ERROR级别
 * @param [in]  pcEventDesc    事件描述
 * @param [in]  pcEventContext 事件上下文(非必填字段)
 * @return      none 
 */
void adm_dpm_addEventAndReport
(
    ADM_COM_MOD_E eModuleID,
    DBOOL  isReportImmediate,
    const DCHAR *pcEventId,
    D32S   i32EventLevel,    
    const DCHAR *pcEventDesc,
    const DCHAR *pcEventContext
)
{
    D64S iEventSequenceId = 0;
    D32S iBakCnt = 0;
    ADM_DPM_EVENTNODE_T *pstEventNode = NULL;
    DCHAR *pcNewEventDesc = NULL;
    D32S ievent_report_level = ADM_CFG_VUS_EVENT_REPORT_LEVEL_DEFAULT_VAL;

    if (ADM_IS_NULL(pcEventId) || ADM_IS_NULL(pcEventDesc))
    {
        log_e("Invalid input param");
        return;
    }

    //!< 模块未初始化，则直接报错
    if (FALSE == gstDpmRptCloudHandle.isInited)
    {
        log_e("dpm report cloud module not init, err[%d]", ADM_ECODE_MODULE_UNINIT);
        return;
    }

    //!< 获取事件上报级别
    if (ADM_ECODE_SUCCESSED != adm_ecs_getEventReportLevel(&ievent_report_level))
	{
        ievent_report_level = adm_cfg_iniParserGetInt(ADM_CFG_VUS_EVENT_REPORT_LEVEL, ADM_CFG_VUS_EVENT_REPORT_LEVEL_DEFAULT_VAL);
	}
    

    //!< 获取时间序列id
    iEventSequenceId = adm_irm_getEventSequenceId();

    //!< 获取neweventDesc
    pcNewEventDesc = __adm_dpm_getNewEventDescInfo(MOD_NAME(eModuleID), pcEventDesc);
    if(NULL == pcNewEventDesc)
    {
        log_e("Invalid input param");
        return;
    }

    //!< 打印日志信息
    log_evt("eventID:%s, eventLevel:%d, eventSequenceId:%lld, eventTime:%lld, eventDesc:%s, eventContext:%s",
        pcEventId, 
        i32EventLevel,
        iEventSequenceId,
        (D64S)time(NULL), 
        pcNewEventDesc,
        (NULL == pcEventContext) ? "NULL" : pcEventContext);

    /** 统计事件，只需要打印即可，暂时不进行上报*/
    if (TRUE == dpmIsStatisticsisEvent(pcEventId))
    {
        // log_d(" skip report statisticsis event[%s]", pcEventId);
        adm_free(pcNewEventDesc);
        return ;
    }

#ifdef ADM_REPORT_EVENT_OPTIMIZE
    /** 下载阶段和传输阶段非重要事件，只需要打印即可，暂时不进行上报 */
    if (FALSE == dpmIsCriticalEvent(pcEventId) && FALSE == tpmIsCriticalEvent(pcEventId))
    {
        log_d(" skip Non Critical event[%s]", pcEventId);
        adm_free(pcNewEventDesc);
        return ;
    }
#endif

    //只有产生的事件级别 >= 配置文件中的 Level级别. 才需要上报到云端
    //产生事件的级别，小于需要上报的事件级别，只需要打印即可
    if(i32EventLevel >= ievent_report_level)
    {
        //!< 创建事件节点 
        pstEventNode = __adm_dpm_createEventNode(pcEventId, i32EventLevel, pcNewEventDesc, pcEventContext, iEventSequenceId);
        if (ADM_IS_NULL(pstEventNode))
        {
            log_e("Failed to malloc, err[%d]", ADM_ECODE_NO_MEMORY);
            adm_free(pcNewEventDesc);
            return;
        }

        //!< 事件节点加链表 
        (void)pthread_mutex_lock(&gstDpmRptCloudHandle.stEventList.stMutex); 
        gstDpmRptCloudHandle.stEventList.s32Cnt++;
        list_add_tail(&pstEventNode->stHead, &gstDpmRptCloudHandle.stEventList.stHead);  //!< 添加到下载控制链表中 

        iBakCnt = gstDpmRptCloudHandle.stEventList.s32Cnt;  //!< 备份当前缓存数量，减少下面使用锁 
        (void)pthread_mutex_unlock(&gstDpmRptCloudHandle.stEventList.stMutex);

        //!< 缓存数据量到达 or 事件结束-关键点，则刷新缓存 
        if (ADM_DPM_EVENT_MAX_NUM <= iBakCnt
          || (FALSE != isReportImmediate)) 
        {
            adm_dpm_fflushEventReport();
        }
    }
    adm_free(pcNewEventDesc);

    return;
}



#if 0
#endif


/*!
 *******************************************************************************
 * 模块运行中车辆状态上报到云端
 *
 * \note 
 *
 * \param[in] pcVehicleState 车辆状态
 * \param[in] pcAbnlReason   车辆异常原因 --- 可选字段
 * \retval    D32
 *******************************************************************************
 */
D32S adm_dpm_vehicleStateReport2Cloud(const DCHAR *pcVehicleState, const DCHAR *pcAbnlReason)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    DCHAR *pcVersionNumber = NULL;
    
    if (ADM_IS_NULL(pcVehicleState))
    {
        log_e("Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }

    //!< 模块未初始化，则直接报错
    if (FALSE == gstDpmRptCloudHandle.isInited)
    {
        log_e("dpm report cloud module not init, err[%d]", ADM_ECODE_MODULE_UNINIT);
        return ADM_ECODE_MODULE_UNINIT;
    }

    do
    {
        iRet = adm_eim_getVehicleBigVersion(&pcVersionNumber);
        if (ADM_IS_ERR(iRet)) 
        {
            log_e("get Vehicle Big Version,error[%d]", iRet);
            break;
        }
        
        (void)pthread_rwlock_rdlock(&gstDpmRptCloudHandle.stRWlock);
        iRet = adm_irm_reportVehicleState(time(NULL), 
                                          gstDpmRptCloudHandle.stRptParam.sdkVersion,
                                          gstDpmRptCloudHandle.stRptParam.sessionId,
                                          gstDpmRptCloudHandle.stRptParam.taskId,
                                          gstDpmRptCloudHandle.stRptParam.strategyId,
                                          pcVehicleState,
                                          (ADM_IS_NULL(pcAbnlReason) ? ("0") : (pcAbnlReason)),
                                          pcVersionNumber,
                                          NULL,
                                          NULL);
        (void)pthread_rwlock_unlock(&gstDpmRptCloudHandle.stRWlock);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to report vehicle statue, err[%d]", iRet);
            break;
        }
    } while (FALSE);

    adm_free(pcVersionNumber);
    adm_free(pcAbnlReason);
    log_d("Report vehicle state, iRet[%d](0: ok, other: failed)", iRet);

    return iRet;
}


/*!
 *******************************************************************************
 * 下载结果上报到云
 *
 * \note 
 *
 * \param[in] pstDownPkgResult 下载升级包结果信息
 * \retval    D32
 *******************************************************************************
 */
D32S adm_dpm_downResultReport2Cloud
(
    ADM_DPS_DOWNPKGRESULT_T *pstDownPkgResult
)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    ADM_OCS_ECU_STATE_INFO_LIST_T *pstEcusStateList = NULL;

    if (ADM_IS_NULL(pstDownPkgResult))
    {
        log_e("Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }
    
    //!< 模块未初始化，则直接报错
    if (FALSE == gstDpmRptCloudHandle.isInited)
    {
        log_e("dpm report cloud module not init, err[%d]", ADM_ECODE_MODULE_UNINIT);
        return ADM_ECODE_MODULE_UNINIT;
    }

    do
    {
        pstEcusStateList = __adm_dpm_createEcusStateList(pstDownPkgResult);
        if (ADM_IS_NULL(pstEcusStateList))
        {
            iRet = ADM_ECODE_NO_MEMORY;
            log_e("Failed to create ecus progress list struct, err[%d]", iRet);
            break;
        }
        
        (void)pthread_rwlock_rdlock(&gstDpmRptCloudHandle.stRWlock);
        iRet = adm_irm_reportEcuState(gstDpmRptCloudHandle.stRptParam.sessionId, 
                                     gstDpmRptCloudHandle.stRptParam.taskId,
                                     gstDpmRptCloudHandle.stRptParam.strategyId,
                                     time(NULL), 
                                     gstDpmRptCloudHandle.stRptParam.sdkVersion,
                                     NULL,
                                     pstEcusStateList);
        (void)pthread_rwlock_unlock(&gstDpmRptCloudHandle.stRWlock);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to create request msg header, err[%d]", iRet);
            break;
        }
    } while (FALSE);

    __adm_dpm_delEcusStateList(&pstEcusStateList);
    log_d("down report result, state[%d](0: ok, other: failed)", iRet);
    return iRet;
}


/*!
 *******************************************************************************
 * 下载结果上报到UIM和云(线程处理)
 *
 * \note 
 *
 * \param[in] param 下载升级包结果信息
 * \retval    void *
 *******************************************************************************
 */
void *adm_dpm_downResultReport2UimAndCloudProc(void *param)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    ADM_OCS_ECU_STATE_INFO_LIST_T *pstEcusStateList = NULL;
    ADM_DPS_DOWNPKGRESULT_T *pstDownPkgResult = NULL;

    if (ADM_IS_NULL(param))
    {
        log_e("Invalid input param");
        return NULL;
    }

    do
    {
        pstDownPkgResult = (ADM_DPS_DOWNPKGRESULT_T *)param;
        
        //!< report 2 uim
        iRet = adm_dpm_ecuDownResultNotify2Uim(pstDownPkgResult->pstResultList);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to report down package result info 2 uim, err[%d]", iRet);   
        }

        //!< report 2 cloud
        iRet = adm_dpm_downResultReport2Cloud(pstDownPkgResult);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to report down package result info 2 Cloud, err[%d]", iRet);
        }


    } while (FALSE);

    adm_dps_delDownPackageResult(&pstDownPkgResult);
    log_d("down report result, state[%d](0: ok, other: failed)", iRet);

    return NULL;
}




/*!
 *******************************************************************************
 * 下载进度上报到云
 *
 * \note 
 *
 * \param[in] pstDownPkgProgress 下载升级包进度信息
 * \retval    D32
 *******************************************************************************
 */
D32S adm_dpm_downProgressReport2Cloud
(
    ADM_DPS_DOWNPKGPROGRESS_T *pstDownPkgProgress
)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    ADM_OCS_ECU_PROGRESS_INFO_LIST_T *pstEcusProgressList = NULL;
    ADM_DPS_DOWNPKGPROGRESS_T *pstAllEcuProgress = NULL;

    if (ADM_IS_NULL(pstDownPkgProgress))
    {
        log_e("Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }

    //!< 模块未初始化，则直接报错
    if (FALSE == gstDpmRptCloudHandle.isInited)
    {
        log_e("dpm report cloud module not init, err[%d]", ADM_ECODE_MODULE_UNINIT);
        return ADM_ECODE_MODULE_UNINIT;
    }

    do
    {
        pstAllEcuProgress = adm_dps_getEcuDownloadProgress();
        if (ADM_IS_NULL(pstAllEcuProgress))
        {
            iRet = ADM_ECODE_NO_MEMORY;
            log_e("Failed to get all ecu down progress info, err[%d]", iRet);
            break;
        }

        pstEcusProgressList = __adm_dpm_createEcusProgressList(pstAllEcuProgress);
        if (ADM_IS_NULL(pstEcusProgressList))
        {
            iRet = ADM_ECODE_NO_MEMORY;
            log_e("Failed to create ecus progress list struct, err[%d]", iRet);
            break;
        }

        (void)pthread_rwlock_rdlock(&gstDpmRptCloudHandle.stRWlock);
        iRet = adm_irm_reportVehicleProgress(gstDpmRptCloudHandle.stRptParam.sessionId, 
                                            gstDpmRptCloudHandle.stRptParam.taskId,
                                            gstDpmRptCloudHandle.stRptParam.strategyId,
                                            time(NULL), 
                                            gstDpmRptCloudHandle.stRptParam.sdkVersion,
                                            NULL,
                                            pstEcusProgressList);
        (void)pthread_rwlock_unlock(&gstDpmRptCloudHandle.stRWlock);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to report vehicle progress, err[%d]", iRet);
            break;
        }
    } while (FALSE);

    __adm_dpm_delEcusProgressList(&pstEcusProgressList);
    adm_dps_delDownPackageProgress(&pstAllEcuProgress);
    log_d("down report progress, state[%d](0: ok, other: failed)", iRet);

    return iRet;
}

#if 0
#endif

/*!
 *******************************************************************************
 * 模块运行中执行错误码转字符串
 *
 * \note 返回常量字符串，外部无需释放
 *
 * \param[in] iError  错误码
 * \retval    DCHAR*  异常原因
 *******************************************************************************
 */
DCHAR *adm_dpm_errorCode2String(D32S iError)
{
    DCHAR *pcErrMsg = NULL;
    DCHAR szReason[128] = {0};
#if 1
    adm_snprintf(szReason, 128, "%d", iError);

    pcErrMsg = adm_strdup(szReason);

    return pcErrMsg;
#else
    switch (iError)
    {
        case ADM_ECODE_SUCCESSED:
            //!< 根据vus 5.0车云协议约定，如果状态正常，则错误码为 0
            pcErrMsg = "0";
            break;
        case ADM_ECODE_INVALID_PARAM:
            pcErrMsg = "invalid param";
            break;
        case ADM_ECODE_INVALID_POINTER:
            pcErrMsg = "invalid pointer";
            break;
        case ADM_ECODE_AES_ECB_DECRYPT_FAILED:
            pcErrMsg = "decrypt failed!";
            break;
        case ADM_ECODE_DOWN_PACKAGE_TYPE_FAILED:
            pcErrMsg = "download package type failed";
            break;
        case ADM_ECODE_DOWN_RESOURCE_NOFIND:
            pcErrMsg = "download resource no find";
            break;
        case ADM_ECODE_DOWN_VERIFY_SIZE_FAIL:
            pcErrMsg = "download verify file size failed";
            break;
        case ADM_ECODE_DOWN_CALC_HASH_FAIL:
            pcErrMsg = "download calc file hash failed";
            break;
        case ADM_ECODE_DOWN_VERIFY_HASH_FAILED:
            pcErrMsg = "download verify file hash error";
            break;
        case ADM_ECODE_DOWN_CHECKPKGCFG_FAILED:
            pcErrMsg = "download check package config file error";
            break;
        case ADM_ECODE_NEW_JSON_OBJECT_FAILED:
        case ADM_ECODE_JSON_TO_STRING_FAILED:
        case ADM_ECODE_JSON_ADD_INT_FAILED:
        case ADM_ECODE_JSON_ADD_INT64_FAILED:
        case ADM_ECODE_JSON_ADD_STRING_FAILED:
        case ADM_ECODE_JSON_ADD_DOUBLE_FAILED:
        case ADM_ECODE_JSON_ADD_BOOL_FAILED:
        case ADM_ECODE_STRING_TO_JSON_FAILED:
        case ADM_ECODE_JSON_GET_INT_FAILED:
        case ADM_ECODE_JSON_GET_INT64_FAILED:
        case ADM_ECODE_JSON_GET_STRING_FAILED:
        case ADM_ECODE_JSON_GET_DOUBLE_FAILED:
        case ADM_ECODE_JSON_GET_BOOL_FAILED:
        case ADM_ECODE_JSON_GET_OBJECT_FAILED:
        case ADM_ECODE_JSON_GET_ARRAYOBJECT_FAILED:
        case ADM_ECODE_DEL_JSON_OBJECT_FAILED:
        case ADM_ECODE_JSON_GET_KEY_FAILED:
        case ADM_ECODE_JSON_KEY_TYPE_ERROR:
        case ADM_ECODE_JSON_ADD_ARRAY_FAILED:
        case ADM_ECODE_JSON_OBJECT_TYPE_NOT_ARRAY:
        case ADM_ECODE_JSON_ADD_SUBOBJECT_FAILED:
        case ADM_ECODE_JSON_KEY_FIELD_IS_NOT_EXIST:
            pcErrMsg = "json operation error";
            break;
       case ADM_ECODE_PAUSE_DOWNLOAD:
            pcErrMsg = "download pause";
            break;
       case ADM_ECODE_CANCEL_DOWNLOAD:
            pcErrMsg = "download cancel";
            break;
       case ADM_ECODE_PAUSE_TRANSFER:
            pcErrMsg = "transfer pause";
            break;
       case ADM_ECODE_CANCEL_TRANSFER:
            pcErrMsg = "transfer cancel";
            break;
       case ADM_ECODE_REQ_STORAGE_SPACE_FAILED:
            pcErrMsg = "downlaod request check storage space failed!";
            break;
       case ADM_ECODE_UCM_STORAGE_SPACE_NOT_ENOUGH:
            pcErrMsg = "downlaod check UCM device storage space error or not enough!";
            break;
       case ADM_ECODE_UCL_STORAGE_SPACE_NOT_ENOUGH:
            pcErrMsg = "downlaod check UCL device storage space error or not enough!";
            break;
       case ADM_ECODE_CONFIRM_DOWNLOAD_TASK_FAILED:
            pcErrMsg = "confirm download task info failed!";
            break;
       case ADM_ECODE_TRANS_REQ_UPLOAD_FAILED:
            pcErrMsg = "transfer request peer upload failed!";
            break;
       case ADM_ECODE_TRANS_REQ_DOWNLOAD_FAILED:
            pcErrMsg = "transfer request peer download failed!";
            break;
       case ADM_ECODE_MONITOR_ABNORMAL_DOWN_BREAK:
            pcErrMsg = "down monitor abnormal download failed!";
            break;
       case ADM_ECODE_NET_CAUSE_DOWN_FAILED:
            pcErrMsg = "net problems cause download failed!";
            break;
       case ADM_ECODE_GET_SIGNATUREDATA_FAILED:
            pcErrMsg = "get package signature data failed!";
            break;
       case ADM_ECODE_VERIFY_SIGNATURE_FAILED:
            pcErrMsg = "package verify signature failed!";
            break;
       case ADM_ECODE_SPLIT_SIGNTEXT_FAILED:
            pcErrMsg = "split package main text and sign text failed!";
            break;
       case ADM_ECODE_UPDATE_DECRYPTFILEOBJ_FAILED:
            pcErrMsg = "update package decrypt file object failed!";
            break; 
       case ADM_ECODE_HAVE_DECRYPT_ING:
            pcErrMsg = "other module has decrypting!";
            break; 
        case ADM_ECODE_NET_CHG_PAUSE_DOWNLOAD:
            pcErrMsg = "network type change download failed!";
            break; 
        case ADM_ECODE_NET_TYPE_VERIFY_FAILED:
            pcErrMsg = "network type verify failed!";
            break; 
        case ADM_ECODE_DOWN_COND_ABNORMAL_PAUSE_DOWNLOAD:
            pcErrMsg = "down cond verify abnormal download failed!";
            break;
        case ADM_ECODE_REMOTE_POWER_ON_FAIL_DOWNLOAD:
            pcErrMsg = "remote power on download failed!";
            break;
       // TODO: 待补充 


        default:
            pcErrMsg = "unKnown error!";
            log_e("UnKnown to switch iError[%d].", iError);
            break;
    }

    return pcErrMsg;
    #endif
}

#if 0
#endif

/*!
 *******************************************************************************
 * dpm report cloud 模块初始化.
 *
 * \note
 * 
 * \param  none
 * \retval D32S
 *******************************************************************************
 */
D32S adm_dpm_reportCloudInit(void)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    D32S errNum = 0;

    if (TRUE == gstDpmRptCloudHandle.isInited)
    {
        log_i("dpm report module already init.");
        return iRet;
    }

    do
    {
        //!< 事件队列 
        iRet = pthread_mutex_init(&gstDpmRptCloudHandle.stEventList.stMutex, NULL);
        if (ADM_IS_ERR(iRet))
        {
            iRet = ADM_ECODE_RWLOCK_INIT_FAILED;
            errNum = errno;
            log_e("Failed to init mutex, err[%d](%s)", iRet, strerror(errNum));
            break;
        }
        INIT_LIST_HEAD(&gstDpmRptCloudHandle.stEventList.stHead);
        gstDpmRptCloudHandle.stEventList.s32Cnt = 0;

        iRet = pthread_rwlock_init(&gstDpmRptCloudHandle.stRWlock, NULL);
        if (ADM_IS_ERR(iRet))
        {
            iRet = ADM_ECODE_RWLOCK_INIT_FAILED;
            errNum = errno;
            log_e("Failed to init rwlock, err[%d](%s)", iRet, strerror(errNum));
            (void)pthread_mutex_destroy(&gstDpmRptCloudHandle.stEventList.stMutex);
            break;
        }
        
        //!< 相关参数信息在api: adm_dpm_checkAndUpdateDownReportInfo 中更新，此处赋初值
        gstDpmRptCloudHandle.stRptParam.sessionId  = NULL;
        gstDpmRptCloudHandle.stRptParam.sdkVersion = NULL;
        gstDpmRptCloudHandle.stRptParam.taskId     = 0;
        gstDpmRptCloudHandle.stRptParam.strategyId = 0;

        gstDpmRptCloudHandle.isInited = TRUE;
    } while (FALSE);

    if (ADM_IS_ERR(iRet))
    {
        (void)adm_dpm_reportCloudDeInit();
    }

    log_d("===> dpm init report cloud module ok.");

    return iRet;
}

/*!
 *******************************************************************************
 * dpm report cloud 模块销毁.
 *
 * \note
 * 
 * \param  none 模块信息
 * \retval none
 *******************************************************************************
 */
void adm_dpm_reportCloudDeInit(void)
{
    if (TRUE == gstDpmRptCloudHandle.isInited)
    {
        gstDpmRptCloudHandle.isInited = FALSE;
    
        /** 释放事件上报链表 */
        __adm_dpm_cleanEventCache();
        (void)pthread_mutex_destroy(&gstDpmRptCloudHandle.stEventList.stMutex);
        gstDpmRptCloudHandle.stEventList.s32Cnt = 0;

        (void)pthread_rwlock_wrlock(&gstDpmRptCloudHandle.stRWlock);
        adm_free(gstDpmRptCloudHandle.stRptParam.sessionId);
        adm_free(gstDpmRptCloudHandle.stRptParam.sdkVersion);
        gstDpmRptCloudHandle.stRptParam.taskId     = 0;
        gstDpmRptCloudHandle.stRptParam.strategyId = 0;
        (void)pthread_rwlock_unlock(&gstDpmRptCloudHandle.stRWlock);
        (void)pthread_rwlock_destroy(&gstDpmRptCloudHandle.stRWlock);
    }

    return;
}

/*!
 *******************************************************************************
 * dpm report cloud模块内部接口初始化.
 *
 * \note
 * 
 * \param[out]  pstOptIf 模块接口信息
 * \retval      D32S
 *******************************************************************************
 */
D32S adm_dpm_checkAndUpdateDownReportInfo(void)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    ADM_DPS_DOWNTSAKINFO_T stDownTaskInfo  = {0};

    if (FALSE == gstDpmRptCloudHandle.isInited)
    {
        iRet = adm_dpm_reportCloudInit();
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to init dpm report cloud module, err[%d]", iRet);
            return iRet;
        }
    }

    do
    {
        (void)adm_memzero(&stDownTaskInfo, sizeof(stDownTaskInfo));
        iRet = adm_dps_getDownloadTaskInfo(&stDownTaskInfo);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to get download task info, err[%d]", iRet);
            break;
        }
       
        //!< 缓存任务头信息更新
        (void)pthread_rwlock_wrlock(&gstDpmRptCloudHandle.stRWlock);
        adm_free(gstDpmRptCloudHandle.stRptParam.sdkVersion);    
        (void)adm_getSdkVersion(&gstDpmRptCloudHandle.stRptParam.sdkVersion);

        adm_free(gstDpmRptCloudHandle.stRptParam.sessionId);
        gstDpmRptCloudHandle.stRptParam.sessionId  = adm_strdup(stDownTaskInfo.sessionId);
        gstDpmRptCloudHandle.stRptParam.taskId     = stDownTaskInfo.taskId;
        gstDpmRptCloudHandle.stRptParam.strategyId = stDownTaskInfo.strategyId;
        (void)pthread_rwlock_unlock(&gstDpmRptCloudHandle.stRWlock);
    } while (FALSE);

    adm_dps_cleanDownTaskInfo(&stDownTaskInfo);

    return iRet;
}


