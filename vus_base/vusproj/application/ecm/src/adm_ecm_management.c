/**
 * @file    adm_ecm_management.c
 * @brief   ecm Management 
 * @details 获取配置管理
 * @author  wangyanpeng
 * @version v0.1.0
 * @date    2020-11-06
 * @copyright www.abupdate.com
 */
#include "adm_ecm_log.h"
#include "adm_ecs_rsConfig.h"
#include "adm_ecm_management.h"
#include "adm_ocm_interface.h"
#include "adm_ocs_airProtocolAdapter.h"
#include "adm_com_lib.h"
#include "adm_com_modDefine.h"
#include "adm_com_errorCode.h"
#include "adm_irm_report.h"
#include "adm_list.h"
#include "adm_string.h"
#include "adm_memory.h"
#include "adm_dim_host.h"
#include "adm_cfg_api.h"
#include "adm_pub_func.h"
#include "adm_file.h"
#include "adm_dim_transHost.h"

#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>


/** 事件缓存的最大上报个数 */
#define ADM_ECM_EVENT_MAX_NUM   (20) 
//!< 同步请求超时时间 
#define ADM_ECM_SYNC_REQ_TIMEOUT        60
//!< 事件描述信息的长度
#define EVNET_DESC_LEN  (512)

/*!
 * @brief ecm模块句柄
 */
typedef struct stECMEventNode
{
    struct list_head stHead;            //!< 双向链节点  

    ADM_OCS_EVENTS_INFO_T stEventInfo;  //!< list长度 
} ADM_ECM_EVENTNODE_T;

/*!
 * @brief ecm模块句柄
 */
typedef struct stECMHandle
{
    DBOOL            isInited;  //!< 模块是否初始化

    pthread_mutex_t  stMutex;   //!< 互斥锁 
    struct list_head stHead;    //!< 双向链表头，表节点为 ADM_ECM_EVENTNODE_T 
    D32S             s32Cnt;    //!< list长度 
} ADM_ECM_HANDLE_T;



#if 0
#endif

static ADM_ECM_HANDLE_T     gstECMHandle     = {0};


#if 0
#endif

/**
 * @brief 检查给定的事件ID是否对应一个关键事件。
 * 
 * @param pcEventId 事件ID字符串的指针。
 * @return 返回DBOOL类型的值，指示该事件是否为关键事件。
 */
static DBOOL ecmIsCriticalEvent(const DCHAR *pcEventId)
{
    DBOOL bIsCritical = FALSE;

    if (NULL == pcEventId) 
    {
        return FALSE;
    }

    if (0 == adm_strcasecmp(pcEventId, "GP_003") ||
        0 == adm_strcasecmp(pcEventId, "GP_005") ||
        0 == adm_strcasecmp(pcEventId, "GI_002") ||
        0 == adm_strcasecmp(pcEventId, "GI_005") ||
        0 == adm_strcasecmp(pcEventId, "GI_008") ||
        0 == adm_strcasecmp(pcEventId, "GI_011") ||
        0 == adm_strcasecmp(pcEventId, "GI_014") ||
        0 == adm_strcasecmp(pcEventId, "GI_017") ||
        0 == adm_strcasecmp(pcEventId, "GI_023") ||
        0 == adm_strcasecmp(pcEventId, "GI_026") ||
        0 == adm_strcasecmp(pcEventId, "GI_029") ||
        0 == adm_strcasecmp(pcEventId, "GI_032") ||
        0 == adm_strcasecmp(pcEventId, "GI_035") ||
        0 == adm_strcasecmp(pcEventId, "GI_038") ||
        0 == adm_strcasecmp(pcEventId, "GI_044") ||
        0 == adm_strcasecmp(pcEventId, "GI_047") ||
        0 == adm_strcasecmp(pcEventId, "GI_068") ||
        0 == adm_strcasecmp(pcEventId, "S_GE_001") ||
        0 == adm_strcasecmp(pcEventId, "S_GE_002") ||
        0 == adm_strcasecmp(pcEventId, "S_GE_003") ||
        0 == adm_strcasecmp(pcEventId, "S_GE_004") ||
        0 == adm_strcasecmp(pcEventId, "S_GE_005") ||
        0 == adm_strcasecmp(pcEventId, "S_GE_006"))
    {
        bIsCritical = TRUE;
    }

    return bIsCritical;
}

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
 * \retval      ADM_ECM_EVENTNODE_T *
 *******************************************************************************
 */
static ADM_ECM_EVENTNODE_T *__adm_ecm_createEventNode
(    
    const DCHAR *pcEventId,
    D32S   i32EventLevel,    
    DCHAR *pcEventDesc,
    const DCHAR *pcEventContext,
    D64S   ieventSequenceId
)
{
    ADM_ECM_EVENTNODE_T *pstEventNode = NULL;

    if (ADM_IS_NULL(pcEventId) || ADM_IS_NULL(pcEventDesc))
    {
        log_e("Invalid input param");
        return NULL;
    }

    pstEventNode = (ADM_ECM_EVENTNODE_T *)adm_malloc(sizeof(*pstEventNode));
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
static void __adm_ecm_freeEventNode(ADM_ECM_EVENTNODE_T *pstEventNode)
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
static void __adm_ecm_cleanEventListInfo
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
static void __adm_ecm_cleanEventCache(void)
{
    struct list_head *pstNode     = NULL;
    struct list_head *pstNextNode = NULL;
    ADM_ECM_EVENTNODE_T *pstEventNode = NULL;
    
    //!< 模块未初始化，则直接报错
    if (FALSE == gstECMHandle.isInited)
    {
        return;
    }

    (void)pthread_mutex_lock(&gstECMHandle.stMutex);
    list_for_each_safe(pstNode, pstNextNode, &gstECMHandle.stHead)
    {
        //!< 取节点
        pstEventNode = list_entry(pstNode, ADM_ECM_EVENTNODE_T, stHead);

        //摘链 and 释放节点 
        list_del(&pstEventNode->stHead);
        __adm_ecm_freeEventNode(pstEventNode);
        pstEventNode = NULL;
    }
    gstECMHandle.s32Cnt = 0;
    (void)pthread_mutex_unlock(&gstECMHandle.stMutex);

    return;
}

static DCHAR* __adm_ecm_getDateAndTimeStr(void)
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

static DCHAR* __adm_ecm_getNewEventDescInfo(DCHAR* moduleName,const DCHAR* eventDesc)
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
            __adm_ecm_getDateAndTimeStr(), 
            eventDesc);

    return peventDesc;
}

#if 0
#endif


/**
 * @brief   ecm 刷新事件上报缓存 
 * @detail  事件上报
 *
 * @note 
 *
 * @return  none 
 */
static void adm_ecm_fflushEventReport(void)
{
    D32S iRet = ADM_ECODE_FAILED;
    D32S i = 0;
    struct list_head *pstNode     = NULL;
    struct list_head *pstNextNode = NULL;
    ADM_ECM_EVENTNODE_T *pstEventNode = NULL;
    ADM_OCS_EVENTS_INFO_LIST_T stEventsList = {0};

    //!< 模块未初始化，则直接报错
    if (FALSE == gstECMHandle.isInited)
    {
        log_e("ECm module not init, err[%d]", ADM_ECODE_MODULE_UNINIT);
        return;
    }

    (void)pthread_mutex_lock(&gstECMHandle.stMutex);
    (void)adm_memzero(&stEventsList, sizeof(stEventsList));
    stEventsList.reportEvents = (ADM_OCS_EVENTS_INFO_T *)adm_malloc((D32U)gstECMHandle.s32Cnt * sizeof(ADM_OCS_EVENTS_INFO_T));
    if (ADM_IS_NULL(stEventsList.reportEvents))
    {
        (void)pthread_mutex_unlock(&gstECMHandle.stMutex);
        log_e("Failed to malloc, err[%d]", ADM_ECODE_NO_MEMORY);
        return;
    }
    (void)adm_memzero(stEventsList.reportEvents, ((D32U)gstECMHandle.s32Cnt * sizeof(ADM_OCS_EVENTS_INFO_T)));
    stEventsList.reportEventsLen = gstECMHandle.s32Cnt;

    list_for_each_safe(pstNode, pstNextNode, &gstECMHandle.stHead)
    {
        //!< 取节点
        pstEventNode = list_entry(pstNode, ADM_ECM_EVENTNODE_T, stHead);
        
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
        __adm_ecm_freeEventNode(pstEventNode);
        pstEventNode = NULL;

        //!< 做个保护，其实没必要 
        if (i == stEventsList.reportEventsLen)
        {
            break;
        }
    }

    gstECMHandle.s32Cnt = 0;
    (void)pthread_mutex_unlock(&gstECMHandle.stMutex);

    //!< 正式上报 
    if (0 < stEventsList.reportEventsLen)
    {
        iRet = adm_irm_reportEvent(NULL, 0, 0, time(NULL), ADM_SDK_VERSION, NULL, &stEventsList);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to report Event, err[%d]", iRet);
        }

        //!< 清空内存 
        __adm_ecm_cleanEventListInfo(&stEventsList);
    }

    return;
}

/**
 * @brief   ecm 模块封装的事件上报接口 
 * @detail  事件上报
 *
 * @note 
 *
 * @param [in]  eModuleID 模块ID
 * @param [in]  isReportImmediate 是否需要立即上报
 * @param [in]  pcEventId      事件id
 * @param [in]  i32EventLevel  事件级别：1:表示TRACE级别 2:表示DEBUG级别 3:表示INFO级别  4:表示WARN级别 5:表示ERROR级别
 * @param [in]  pcEventDesc    事件描述
 * @param [in]  pcEventContext 事件上下文(非必填字段)
 * @return      none 
 */
static void adm_ecm_addEventAndReport
(
    ADM_COM_MOD_E eModuleID,
    DBOOL  isReportImmediate,
    const DCHAR *pcEventId,
    D32S   i32EventLevel,    
    const DCHAR *pcEventDesc,
    const DCHAR *pcEventContext
)
{
    D32S iBakCnt = 0;
    D64S iEventSequenceId = 0;
    ADM_ECM_EVENTNODE_T *pstEventNode = NULL;
    DCHAR *pcNewEventDesc = NULL;
    D32S ievent_report_level = ADM_CFG_VUS_EVENT_REPORT_LEVEL_DEFAULT_VAL;

    if (ADM_IS_NULL(pcEventId) || ADM_IS_NULL(pcEventDesc))
    {
        log_e("Invalid input param");
        return;
    }
    
    //!< 模块未初始化，则直接报错
    if (FALSE == gstECMHandle.isInited)
    {
        log_e("ECm module not init, err[%d]", ADM_ECODE_MODULE_UNINIT);
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
    pcNewEventDesc = __adm_ecm_getNewEventDescInfo(MOD_NAME(eModuleID), pcEventDesc);
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

    #ifdef ADM_REPORT_EVENT_OPTIMIZE
        if (FALSE == ecmIsCriticalEvent(pcEventId))
        {
            //!< 非重要事件不需要上报，只需要在日志中打印即可
            log_w("event is Non Critical");
            adm_free(pcNewEventDesc);
            return ;
        }
    #endif

    //只有产生的事件级别 >= 配置文件中的 Level级别. 才需要上报到云端
    //产生事件的级别，小于需要上报的事件级别，只需要打印即可
    if(i32EventLevel >= ievent_report_level)
    {
        //!< 创建事件节点 
        pstEventNode = __adm_ecm_createEventNode(pcEventId, i32EventLevel, pcNewEventDesc, pcEventContext, iEventSequenceId);
        if (ADM_IS_NULL(pstEventNode))
        {
            log_e("Failed to malloc, err[%d]", ADM_ECODE_NO_MEMORY);
            adm_free(pcNewEventDesc);
            return;
        }

        //!< 事件节点加链表 
        (void)pthread_mutex_lock(&gstECMHandle.stMutex); 
        gstECMHandle.s32Cnt++;
        list_add_tail(&pstEventNode->stHead, &gstECMHandle.stHead);  //!< 添加到下载控制链表中 

        iBakCnt = gstECMHandle.s32Cnt;  //!< 备份当前缓存数量，减少下面使用锁 
        (void)pthread_mutex_unlock(&gstECMHandle.stMutex);

        //!< 缓存数据量到达 or 事件结束，则刷新缓存 
        if ((ADM_ECM_EVENT_MAX_NUM <= iBakCnt) 
          || (FALSE != isReportImmediate))
        {
            adm_ecm_fflushEventReport();
        }
    }
    adm_free(pcNewEventDesc);

    return;
}

#if 0
#endif


/*!
 *******************************************************************************
 * ecm模块 配置文件的解密和验签回调函数(ecm模块实现).
 *
 * \note 解密和验签后的文件路径会发生变化，外部使用时需要注意更新解密验签后的路径
 *           pstFileObject->filePath 会发生变化 
 *
 * \param[in]   pstFileObject      文件对象信息
 * \retval      D32S
 *******************************************************************************
 */
D32S adm_ecm_decryptAndVerificationSignature(ADM_COM_FILEOBJECT_T *pstFileObject)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    ADM_COM_SIGNATUREINFO_T *pstSignatureInfo = NULL; //mod by wl 20220411
    D32S ierrorCode = ADM_ECODE_SUCCESSED;
    D32S s32HostInstId = 0;
    DCHAR *pcMainFileName = NULL;   // 正文文件名
    DCHAR *pcSignFileName = NULL;   // 签名文件名
    DCHAR *pcDecryptPath  = NULL;
    D32S s32UclInstId = 0;
    DCHAR szUclInstId[10] = {0};
    DBOOL bIsPkiVerifySign = DFALSE;
    
    if (ADM_IS_NULL(pstFileObject))
    {
        log_e("Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }
    log_i("Entry down package decrypt and verf sign, filePath[%s].", pstFileObject->filePath);
    
    do{        

        s32UclInstId = adm_cfg_iniParserGetInt(ADM_CFG_VUS_UCL_INSTANCEID,  ADM_CFG_VUS_UCL_INSTANCEID_DEFAULT_VAL);
        (void)adm_snprintf(szUclInstId, (D32U)sizeof(szUclInstId), "%x", s32UclInstId);
        // 解密 
        iRet = adm_pub_downPackageDecrypt(pstFileObject, &pcDecryptPath, szUclInstId);
        if (ADM_IS_ERR(iRet))
        {
            log_e("down package decrypt failed, filePath[%s], err[%d]", pstFileObject->filePath, iRet);
            break; 
        }

        // 需要解密 
        if((NULL != pstFileObject->pstFileSecurityInfo) && (1 == pstFileObject->pstFileSecurityInfo->isEncrypt)) 
        {
            // 删除原始加密文件
            if (pstFileObject->filePath != NULL)
            {
                log_i("Deleting original encrypted file: %s", pstFileObject->filePath);
                if (0 != adm_fremove(pstFileObject->filePath))
                {
                    log_w("Failed to delete original encrypted file: %s, but continue", pstFileObject->filePath);
                }
                else
                {
                    log_i("Successfully deleted original encrypted file: %s", pstFileObject->filePath);
                }
            }
            // 将解密后的信息填充到上层，并将 isEncrypt 置为 0，表示未加密 
            adm_free(pstFileObject->filePath);
            pstFileObject->filePath = adm_strdup(pcDecryptPath);
            adm_free(pstFileObject->hash);
            pstFileObject->hash     = adm_strdup(pstFileObject->pstFileSecurityInfo->oriHash);
            pstFileObject->size     = pstFileObject->pstFileSecurityInfo->oriSize;
            
            pstFileObject->pstFileSecurityInfo->isEncrypt = 0;
            adm_free(pcDecryptPath);
        }
        log_d("down package decrypt ok.");

        //!< 验签的条件是否满足 
        if (FALSE == adm_com_checkIsNeedToVerifySignature(pstFileObject))
        {
            iRet = ADM_ECODE_SUCCESSED;
            log_d("conditions dissatisfy, dont need to verify signature");
            break;
        }

        pstSignatureInfo = adm_com_newSignatureInfo();
        if (ADM_IS_NULL(pstSignatureInfo))
        {
            iRet = ADM_ECODE_NO_MEMORY;
            log_e("Failed to malloc pstSignatureInfo info. err[%d]", ADM_ECODE_NO_MEMORY);
            break;
        }

#ifdef SUPPORT_LION_PKI
        bIsPkiVerifySign = DTRUE;
#endif
        // 获取验证签名数据
        iRet = adm_com_getSignatureData(pstFileObject, bIsPkiVerifySign, pstSignatureInfo);
        if (ADM_IS_ERR(iRet))
        {
            iRet = ADM_ECODE_GET_SIGNATUREDATA_FAILED;
            log_e("Get Signature Data from config package[%s] failed, err[%d]", pstFileObject->fileName, iRet);
            break; 
        }
        log_d("get Signature Data from config package ok.");
        
        s32HostInstId = adm_cfg_iniParserGetInt(ADM_CFG_VUS_HOST_INSTANCEID,  ADM_CFG_VUS_HOST_INSTANCEID_DEFAULT_VAL);
        
        // dim验签接口
        iRet = adm_dim_host_verifySignature((D16U)s32HostInstId, ADM_ECM_SYNC_REQ_TIMEOUT, 
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

        // 拆分正文+签名文件 
        iRet = adm_com_SeparateOriginAndSignFile(pstFileObject, bIsPkiVerifySign, &pcMainFileName, &pcSignFileName);
        if (ADM_IS_ERR(iRet))
        {
            iRet = ADM_ECODE_SPLIT_SIGNTEXT_FAILED;
            log_e("split package main text and sign text failed! err[%d]", iRet);
            break; 
        }
        log_i("split package main text and sign text OK. mainText[%s], signText[%s]", pcMainFileName, pcSignFileName);

        adm_free(pstFileObject->filePath);
        pstFileObject->filePath = pcMainFileName;
        (void)adm_fremove(pcSignFileName);
        adm_free(pcSignFileName);

    }while (FALSE);

    log_i("Return down package decrypt and verf sign, filePath[%s], err[%d].", pstFileObject->filePath, iRet);
    adm_com_delSignatureInfo(&pstSignatureInfo);

    return iRet;
}

/*!
 *******************************************************************************
 * 强校验字符串匹配
 *
 * \note    pcSubStr 与 pcStr 前 adm_strlen(pcSubStr) 字符强一致
 *              且返回匹配的末端字符位置指针
 *          return pcStr + adm_strlen(pcSubStr)
 *
 * \param[in]  pcStr     源字符串
 * \param[in]  pcSubStr  待匹配字符串
 * \retval     DCHAR     匹配后的字符串指针(若两个字符串相等，则返回 NULL)
 *******************************************************************************
 */
const DCHAR *ecmMatchLastSubStrInStr(const DCHAR *pcStr, const DCHAR *pcSubStr)
{
    D32U iStrLen = (D32U)0;
    D32U iSubLen = (D32U)0;

    if (ADM_IS_NULL(pcStr) || ADM_IS_NULL(pcSubStr))
    {
        log_e("Invalid input param");
        return NULL;
    }

    iStrLen = adm_strlen(pcStr);
    iSubLen = adm_strlen(pcSubStr);
    // Check if the substring is longer than the string being searched.
    if (iSubLen >= iStrLen)
    {
        log_e("subString gt(et) strString size!");
        return NULL;
    }

    if (0 == adm_memcmp(pcStr, pcSubStr, iSubLen))
    {
        return pcStr + iSubLen;
    }

    return NULL;
}

/*!
 *******************************************************************************
 * 创建传输url
 *
 * \note       ewebServer 和 package 均在主控侧
 *
 * \param[in]  pMasterPkgPath  主控下载的升级包文件存储路径
 * \retval     DCHAR*           生成的 ewebServer 传输的 url
 *******************************************************************************
 */
static DCHAR *ecmCreateTransUrlFromMaster(const DCHAR *pMasterPkgPath)
{
    DCHAR szTransURL[ADM_COM_FILEPATH_LEN] = {0};
    const DCHAR *pcDownPkgSuffixUrl = NULL;
    const DCHAR *pcEwebRootDir = NULL;
    const DCHAR *pcEwebPrefixUrl = NULL;

    if (ADM_IS_NULL(pMasterPkgPath))
    {
        log_e("Invalid input param");
        return NULL;
    }

    //!< 1. 获取eweb主目录
    pcEwebRootDir = adm_cfg_iniParserGetString(ADM_CFG_EWEB_DOC_ROOT_DIR, ADM_CFG_EWEB_DOC_ROOT_DIR_DEFAULT_VAL);
    log_d("pcMasterPkgPath[%s], pcEwebRootDir[%s]", pMasterPkgPath, pcEwebRootDir);

    //!< 2. 判断 pcMasterPkgPath 中是否包含 pcEwebRootDir 存储路径, 若不包含则无法使用eweb
    pcDownPkgSuffixUrl = ecmMatchLastSubStrInStr(pMasterPkgPath, pcEwebRootDir);
    if (ADM_IS_NULL(pcDownPkgSuffixUrl))
    {
        log_e("Failed to check package path[%s] user eweb[%s].", pMasterPkgPath, pcEwebRootDir);
        return NULL;
    }
    log_d("pcDownPkgSuffixUrl[%s]", pcDownPkgSuffixUrl);

    //!< 2. 序列化生成新的传输 url
    pcEwebPrefixUrl = adm_cfg_iniParserGetString(ADM_CFG_EWEB_ADDRESS_HEADER, ADM_CFG_EWEB_ADDRESS_HEADER_DEFAULT_VAL);
    (void)adm_memzero(szTransURL, sizeof(szTransURL));
    (void)adm_snprintf(szTransURL, sizeof(szTransURL), "%s%s", pcEwebPrefixUrl, pcDownPkgSuffixUrl);
    log_d("pcMasterPkgPath[%s], szTransURL[%s]", pMasterPkgPath, szTransURL);

    return adm_strdup(szTransURL);
}

/*!
 *******************************************************************************
 * ecm模块 传输脚本到qnx侧.
 *
 *
 * \param[in]   fileName      文件名称
 * \param[in]   filePath      文件路径
 * \param[in]   size          文件数据大小
 * \param[in]   hash          文件hash值
 * \retval      D32S
 *******************************************************************************
 */
static D32S adm_ecm_fileSyncTransfer(DCHAR *fileName, const DCHAR *filePath, const D64S size, const DCHAR *hash)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    D32S ierrorCode = ADM_ECODE_SUCCESSED;
    DCHAR *pUrl = NULL;
    D32S s32UclInstId = 0;

    if (ADM_IS_NULL(fileName) || ADM_IS_NULL(filePath) || ADM_IS_NULL(hash) || (0 == size))
    {
        log_e("Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }

    pUrl = ecmCreateTransUrlFromMaster(filePath);
    if (ADM_IS_NULL(pUrl))
    {
        log_e("create url failed");
        return ADM_ECODE_INVALID_PARAM;
    }
    log_i("trans file url:%s", pUrl);

    s32UclInstId = adm_cfg_iniParserGetInt(ADM_CFG_VUS_VDI_INSTANCEID, ADM_CFG_VUS_VDI_INSTANCEID_DEFAULT_VAL);
    // dim验签接口
    iRet = adm_dim_trans_requestTransferSyncDownloadFromHost((D16U)s32UclInstId, ADM_ECM_SYNC_REQ_TIMEOUT,
                                                             fileName, pUrl, fileName, size, hash, &ierrorCode);
    if (ADM_IS_ERR(iRet) || ADM_IS_ERR(ierrorCode))
    {
        iRet = ADM_ECODE_FAILED;
        log_e("transfer file failed, errorCode[%d], err[%d]", ierrorCode, iRet);
    }

    return iRet;
}

#if 0
#endif

/*!
 *******************************************************************************
 * ecm模块初始化.
 *
 * \note
 *
 * \param   none
 * \retval  D32S    0: 成功 non-0：失败
 *******************************************************************************
 */
D32S adm_ecm_moduleInit(void)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    D32S errNum = 0;

    if (TRUE == gstECMHandle.isInited)
    {
        log_i("ecm module already init.");
        return iRet;
    }

    do
    {
        iRet = pthread_mutex_init(&gstECMHandle.stMutex, NULL);
        if (ADM_IS_ERR(iRet))
        {
            iRet = ADM_ECODE_RWLOCK_INIT_FAILED;
            errNum = errno;
            log_e("Failed to init mutex, err[%d](%s)", iRet, strerror(errNum));
            break;
        }
        INIT_LIST_HEAD(&gstECMHandle.stHead);
        gstECMHandle.s32Cnt   = 0;
        gstECMHandle.isInited = TRUE;

        iRet = adm_ecs_moduleInit(adm_ecs_getReqCloudParam, 
                        adm_ocm_businessCommInterface, 
                        adm_ocm_fileDownloadInterface,
                        adm_ecm_addEventAndReport,
                        adm_ecm_decryptAndVerificationSignature,
                        adm_ecm_fileSyncTransfer);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to init ecs module, err[%d]", iRet);
            break;
        }

    } while (FALSE);

    if (ADM_IS_ERR(iRet))
    {
        (void)adm_ecm_moduleDeInit();
    }

    log_d("===> ecm init module ok.");

    return iRet;
}

/*!
 *******************************************************************************
 * ecm模块销毁.
 *
 * \note
 *
 * \param   none
 * \retval  none
 *******************************************************************************
 */
void adm_ecm_moduleDeInit(void)
{
    if (TRUE == gstECMHandle.isInited)
    {
        gstECMHandle.isInited = FALSE;
    
        adm_ecs_moduleDeInit();

        /** 释放事件上报链表 */
        __adm_ecm_cleanEventCache();
        (void)pthread_mutex_destroy(&gstECMHandle.stMutex);
        gstECMHandle.s32Cnt   = 0;
    }
    
    return;
}

