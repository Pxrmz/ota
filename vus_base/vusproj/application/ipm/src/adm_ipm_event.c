/**
 * @file    adm_ipm_event.c
 * @brief   ipm report event cloud 
 * @details ipm 上报事件到 cloud
 * @author  
 * @version v0.1.0
 * @date    2020-11-06
 * @copyright www.abupdate.com
 */
#include "adm_ipm_log.h"
#include "adm_ipm_impl.h"
#include "adm_typedefs.h"
#include "adm_com_errorCode.h"
#include "adm_com_hash.h"
#include "adm_application_errorCode.h"
#include "adm_string.h"
#include "adm_memory.h"
#include "adm_list.h"
#include "adm_vrs_registerInfo.h"
#include "adm_ocs_airProtocolSecurity.h"
#include "adm_application_common.h"
#include "adm_cfg_api.h"
#include "adm_file.h"
#include "adm_pub_func.h"





#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <time.h>
#include <pthread.h>





#include "json-c/json.h"
#include "adm_com_modDefine.h"

//#include "adm_vrs_registerInfo.h"
//#include "adm_vrm_register.h"
//#include "adm_ipm_uim.h"
//#include "adm_ipm_reportCloud.h"
#include "adm_ecs_rsConfig.h"
#include "adm_ecs_parsePolicy.h"

#include "adm_irm_report.h"
#include "adm_eim_getEcuInfo.h"




#if 0
#endif

/** 事件缓存的最大上报个数 */
#define adm_ipm_EVENT_MAX_NUM   (10) 
//!< 事件描述信息的长度
#define EVNET_DESC_LEN  (1024*2)

#define     IPM_EV_TAG                          "##VUS"
#define     IPM_EV_TAG_BEGIN                    IPM_EV_TAG"##BEGIN##"       // 模块开始事件
#define     IPM_EV_TAG_END_PASS                 IPM_EV_TAG"##END##PASS##"   // 模块通过事件
#define     IPM_EV_TAG_END_FAIL                 IPM_EV_TAG"##END##FAIL##"   // 模块失败事件


#define     IPM_EV_TAG_DOING(stage, state)      IPM_EV_TAG"##DOING##"stage"##"#state"##"
#define     IPM_EV_TAG_DOING_BEGIN(stage)       IPM_EV_TAG_DOING(stage, BEGIN)      // 模块关键阶段开始事件
#define     IPM_EV_TAG_DOING_PASS(stage)        IPM_EV_TAG_DOING(stage, PASS)       // 模块关键阶段通过事件
#define     IPM_EV_TAG_DOING_FAIL(stage)        IPM_EV_TAG_DOING(stage, FAIL)       // 模块关键阶段失败事件

/*模块关键阶段定义*/
#define     IPM_EV_STAGE_ECU_INSTALL            "ECU INSTALLATION"      // ECU安装阶段


/*!
 * @brief 事件节点
 */
typedef struct stIPMEventNode
{
    struct list_head stHead;            //!< 双向链节点  

    ADM_OCS_EVENTS_INFO_T stEventInfo;  //!< list长度 
} adm_ipm_EVENTNODE_T;

/*!
 * @brief 事件队列
 */
typedef struct stIpmEventList
{
    pthread_mutex_t  stMutex;   //!< 互斥锁 
    struct list_head stHead;    //!< 双向链表头  
    D32S             s32Cnt;    //!< list长度 
} adm_ipm_EVENTLIST_T;


#if 0
#endif


/*!
 * @brief 获取平台配置请求参数
 */
typedef struct stReportEventParam_
{
    DCHAR *sdkVersion;                  //!< SDK版本号
    DCHAR *sessionId;                   //!< 会话Id(check接口响应的)
    D64S   taskId;                      //!< 任务Id(check接口响应的)
    D64S   strategyId;                  //!< 策略Id(check接口响应的)
} adm_ipm_RPTCLOUDPARAM_T;

/*!
 * @brief ipm 上报云端信息
 */
typedef struct stReportEventInfo_
{
    DBOOL                    isInited;      //!< 模块是否初始化

    pthread_rwlock_t         stRWlock;      //!< 读写锁 --- 锁 stRptParam 
    adm_ipm_RPTCLOUDPARAM_T  stRptParam;    //!< 上报参数 
    adm_ipm_EVENTLIST_T      stEventList;   //!< 事件队列 
} adm_ipm_REPORTCLOUDINFO_T;

#if 0
#endif

static adm_ipm_REPORTCLOUDINFO_T gstIpmRptCloudHandle = {0};

//!< 进度上报类型 
//#define adm_ipm_RPT_PROGRESS_TYPE_DOWN      "1"
//#define adm_ipm_RPT_PROGRESS_TYPE_TRANS     "2"

	




typedef struct
{    
    DCHAR* eventId; /*!< 事件ID */
    D32S eventLevel; /*!< 事件级别：1:表示TRACE级别 2:表示DEBUG级别 3:表示INFO级别  4:表示WARN级别 5:表示ERROR级别*/
    DCHAR* eventContext; /*!< 事件上下文 */
    DCHAR* eventDesc; /*!< 事件描述 */
} ADM_IPM_REPORT_EVENT_INFO_T;


static ADM_IPM_REPORT_EVENT_INFO_T m_ipm_report_event_list[] = {
{"IT_001", ADM_OCS_EVENT_LEVEL_TRACE, IPM_EV_TAG_BEGIN, "Installation priority processing, vehicle power on, non active installation into active installation"},
{"IT_002", ADM_OCS_EVENT_LEVEL_TRACE, IPM_EV_TAG_BEGIN, "Installation trigger, general installation"},
{"IT_003", ADM_OCS_EVENT_LEVEL_TRACE, IPM_EV_TAG_BEGIN, "Installation trigger, automatic installation"},
{"IT_004", ADM_OCS_EVENT_LEVEL_TRACE, IPM_EV_TAG_BEGIN, "Installation trigger, appointment for installation"},
{"IT_005", ADM_OCS_EVENT_LEVEL_TRACE, IPM_EV_TAG_BEGIN, "Installation trigger, USB flash disk installation"},
{"IT_006", ADM_OCS_EVENT_LEVEL_TRACE, IPM_EV_TAG_BEGIN, "Installation trigger, remote installation of mobile app"},
{"IT_007", ADM_OCS_EVENT_LEVEL_TRACE, IPM_EV_TAG_BEGIN, "Installation trigger, silent installation"},


{"IP_001", ADM_OCS_EVENT_LEVEL_DEBUG, NULL, "Request the whole vehicle to be powered on"},
{"IP_002", ADM_OCS_EVENT_LEVEL_ERROR, NULL, "Installation enable, request master control to power on the whole vehicle failed, error code is :"},
{"IP_003", ADM_OCS_EVENT_LEVEL_TRACE, NULL, "Installation enabling, owner authorization confirmation"},
{"IP_004", ADM_OCS_EVENT_LEVEL_TRACE, NULL, "Installation enable, disclaimer confirmation"},
{"IP_005", ADM_OCS_EVENT_LEVEL_TRACE, NULL, "Installation enable, installation secondary confirmation"},
{"IP_006", ADM_OCS_EVENT_LEVEL_TRACE, NULL, "Installation enable, vehicle status check"},
{"IP_007", ADM_OCS_EVENT_LEVEL_TRACE, NULL, "Installation brush write, task validity verification succeeded"},
{"IP_008", ADM_OCS_EVENT_LEVEL_ERROR, NULL, "Installation brush write, task validity verification failed, error code is :"},
{"IP_009", ADM_OCS_EVENT_LEVEL_TRACE, NULL, "After installation, the software package is successfully checked and signed"},
{"IP_010", ADM_OCS_EVENT_LEVEL_ERROR, NULL, "Installation brush writing, package signature verification failed, error code is :"},
{"IP_011", ADM_OCS_EVENT_LEVEL_TRACE, NULL, "After installation, the software package is successfully decompressed"},
{"IP_012", ADM_OCS_EVENT_LEVEL_ERROR, NULL, "Installation brush write, package decompression failed"},
{"IP_013", ADM_OCS_EVENT_LEVEL_TRACE, IPM_EV_TAG_DOING_BEGIN(IPM_EV_STAGE_ECU_INSTALL), "obtain the parts to be upgraded successfully"},
{"IP_014", ADM_OCS_EVENT_LEVEL_WARN, NULL, "Failed to obtain the parts to be upgraded"},
{"IP_015", ADM_OCS_EVENT_LEVEL_INFO, NULL, "Install the brush write, and the brush write request is successful"},
{"IP_016", ADM_OCS_EVENT_LEVEL_ERROR, NULL, "Installation brush write, brush write request failed"},

{"II_001", ADM_OCS_EVENT_LEVEL_INFO, NULL, "Prepare the installation environment, enter the diagnostic mode successfully"},
{"II_002", ADM_OCS_EVENT_LEVEL_ERROR, NULL, "Installation environment preparation, failed to enter diagnostic mode, error code is :"},
{"II_003", ADM_OCS_EVENT_LEVEL_INFO, NULL, "Installation environment preparation, local OTA wake-up event is successful"},
{"II_004", ADM_OCS_EVENT_LEVEL_ERROR, NULL, "Installation environment preparation, local OTA wake-up event failed, , error code is :"},
{"II_005", ADM_OCS_EVENT_LEVEL_INFO, NULL, "Prepare the installation environment, enter OTA mode successfully"},
{"II_006", ADM_OCS_EVENT_LEVEL_ERROR, NULL, "Installation environment preparation, entering OTA mode failed, , error code is :"},
{"II_007", ADM_OCS_EVENT_LEVEL_INFO, NULL, "After installation, the pre condition check is successful"},
{"II_008", ADM_OCS_EVENT_LEVEL_ERROR, NULL, "Installation brush, precondition check failed (msgid = 0x123, realval = XXX, expectval = XXX)"},
{"II_009", ADM_OCS_EVENT_LEVEL_TRACE, NULL, "After installing brush writing, the intelligent power supply is successful"},
{"II_010", ADM_OCS_EVENT_LEVEL_ERROR, NULL, "Installation of brush writing, intelligent power supply failure, error code is :"},
{"II_011", ADM_OCS_EVENT_LEVEL_TRACE, NULL, "The brush is installed and the high voltage is successfully applied"},
{"II_012", ADM_OCS_EVENT_LEVEL_ERROR, NULL, "Installation of brush writing, high voltage failure, error code is :"},
{"II_013", ADM_OCS_EVENT_LEVEL_TRACE, NULL, "Install the brush and write down the high voltage successfully"},
{"II_014", ADM_OCS_EVENT_LEVEL_ERROR, NULL, "Failed to write down the high voltage after installing the brush, error code is :"},
{"II_015", ADM_OCS_EVENT_LEVEL_TRACE, NULL, "Installation brush write, brush write progress return"},
{"II_016", ADM_OCS_EVENT_LEVEL_ERROR, NULL, "Installation brush write, brush write progress return failure, ECU, error code is :"},
{"II_017", ADM_OCS_EVENT_LEVEL_ERROR, NULL, "After start installation, the pre condition check result notice hmi failed, error code is :"},
{"II_018", ADM_OCS_EVENT_LEVEL_ERROR, NULL, "After installation, vehicle install result notice hmi failed, error code is :"},
{"II_019", ADM_OCS_EVENT_LEVEL_ERROR, NULL, "After installation, single ecu install result notice hmi failed, error code is :"},
{"II_020", ADM_OCS_EVENT_LEVEL_ERROR, NULL, "After hmi online, all ecu install result notice hmi failed, error code is :"},
{"II_021", ADM_OCS_EVENT_LEVEL_INFO, NULL, "Remote power-on succeeded"},
{"II_022", ADM_OCS_EVENT_LEVEL_ERROR, NULL, "Remote power-on failed.  error code :"},
{"II_023", ADM_OCS_EVENT_LEVEL_INFO, NULL, "Remote power-off succeeded"},
{"II_024", ADM_OCS_EVENT_LEVEL_ERROR, NULL, "Remote power-off fails.  error code :"},

{"S_IE_001", ADM_OCS_EVENT_LEVEL_INFO, IPM_EV_TAG_END_PASS, "Installation successful"},
{"S_IE_002", ADM_OCS_EVENT_LEVEL_INFO, IPM_EV_TAG_END_FAIL, "Installation termination,  error code is :"},
//{"S_IE_001", ADM_OCS_EVENT_LEVEL_INFO, NULL, "Install the brush write, and the brush write result returns"},
//{"S_IE_002", ADM_OCS_EVENT_LEVEL_ERROR, NULL, "Install the brush write, and the brush write result is returned. ECU: 0xFFFF, 0xFFFF"},
{"S_IE_003", ADM_OCS_EVENT_LEVEL_TRACE, NULL, "The installation environment exits, and the OTA mode exits successfully"},
{"S_IE_004", ADM_OCS_EVENT_LEVEL_ERROR, NULL, "Installation environment exit, OTA mode exit failed , error code is :"},
{"S_IE_005", ADM_OCS_EVENT_LEVEL_TRACE, NULL, "The installation environment exits, and the diagnostic mode exits successfully"},
{"S_IE_006", ADM_OCS_EVENT_LEVEL_ERROR, NULL, "Exit environment, diagnostic mode , error code is :"},
{"S_IE_007", ADM_OCS_EVENT_LEVEL_TRACE, NULL, "The installation environment exits and the OTA local event is cleared successfully"},
{"S_IE_008", ADM_OCS_EVENT_LEVEL_ERROR, NULL, "Clear environment, event, error code is :"},
{"S_IE_009", ADM_OCS_EVENT_LEVEL_TRACE, NULL, "The installation environment is exited, and the high voltage of the whole vehicle is successfully exited"},
{"S_IE_010", ADM_OCS_EVENT_LEVEL_ERROR, NULL, "The installation environment exited and the vehicle failed to return to high voltage, error code is :"},
{"S_IE_011", ADM_OCS_EVENT_LEVEL_TRACE, NULL, "The installation environment exits and the whole vehicle is powered off successfully"},
{"S_IE_012", ADM_OCS_EVENT_LEVEL_ERROR, NULL, "The installation environment exits, and the power off of the whole vehicle fails, error code is :"},
{"S_IE_013", ADM_OCS_EVENT_LEVEL_INFO, NULL, "Integration of installation results"},
{"S_IE_014", ADM_OCS_EVENT_LEVEL_INFO, NULL, "The installation results show that the browser mode is successfully entered"},
{"S_IE_015", ADM_OCS_EVENT_LEVEL_ERROR, NULL, "As a result of the installation, it failed to enter the broker mode, error code is :"},
{"S_IE_016", ADM_OCS_EVENT_LEVEL_INFO, NULL, "As a result of the installation, the browser mode is exited successfully"},
{"S_IE_017", ADM_OCS_EVENT_LEVEL_ERROR, NULL, "As a result of the installation, exiting the broker mode failed , error code is :"},

{"S_IE_018", ADM_OCS_EVENT_LEVEL_INFO, IPM_EV_TAG_DOING_PASS(IPM_EV_STAGE_ECU_INSTALL), "ecu Install success"},
{"S_IE_019", ADM_OCS_EVENT_LEVEL_ERROR, IPM_EV_TAG_DOING_FAIL(IPM_EV_STAGE_ECU_INSTALL), "ecu Install fail, error code is :"},
{"S_IE_020", ADM_OCS_EVENT_LEVEL_INFO, NULL, "Check ecu version successfully after ecu Installation"},
{"S_IE_021", ADM_OCS_EVENT_LEVEL_ERROR, NULL, "Check ecu version failed after ecu installation"},

{"S_IE_022",ADM_OCS_EVENT_LEVEL_ERROR,NULL,"APP setting appointment upgrade failed"},
{"S_IE_023",ADM_OCS_EVENT_LEVEL_ERROR,NULL,"HMI setting appointment upgrade failed"},
{"S_IE_024",ADM_OCS_EVENT_LEVEL_ERROR,NULL,"APP cancellation of appointment upgrade failed"},
{"S_IE_025",ADM_OCS_EVENT_LEVEL_ERROR,NULL,"HMI cancellation of appointment upgrade failed"},

{"PS_INST_001", ADM_OCS_EVENT_LEVEL_INFO, NULL, "[ecuReqId + ecuSwDid][start Time](INST_ Local signature verify：progress[%d], fileSize[%lld]kb)"},
{"PS_INST_002", ADM_OCS_EVENT_LEVEL_INFO, NULL, "[ecuReqId + ecuSwDid][end Time](INST_ Local signature verify：progress[%d], fileSize[%lld]kb)"},
{"PS_INST_003", ADM_OCS_EVENT_LEVEL_INFO, NULL, "[ecuReqId + ecuSwDid][expend Time](INST_ Local signature verify：progress[%d], fileSize[%lld]kb)"},
{"PS_INST_004", ADM_OCS_EVENT_LEVEL_INFO, NULL, "[ecuReqId + ecuSwDid][start Time](INST_ Remote signature verify：progress[%d], fileSize[%lld]kb)"},
{"PS_INST_005", ADM_OCS_EVENT_LEVEL_INFO, NULL, "[ecuReqId + ecuSwDid][end Time](INST_ Remote signature verify：progress[%d], fileSize[%lld]kb)"},
{"PS_INST_006", ADM_OCS_EVENT_LEVEL_INFO, NULL, "[ecuReqId + ecuSwDid][expend Time](INST_ Remote signature verify：progress[%d], fileSize[%lld]kb)"},
{"PS_INST_007", ADM_OCS_EVENT_LEVEL_INFO, NULL, "[ecuReqId + ecuSwDid][start Time](INST_ unzip：progress[%d], fileSize[%lld]kb)"},
{"PS_INST_008", ADM_OCS_EVENT_LEVEL_INFO, NULL, "[ecuReqId + ecuSwDid][end Time](INST_ unzip：progress[%d], fileSize[%lld]kb)"},
{"PS_INST_009", ADM_OCS_EVENT_LEVEL_INFO, NULL, "[ecuReqId + ecuSwDid][expend Time](INST_ unzip：progress[%d], fileSize[%lld]kb)"},
{"PS_INST_010", ADM_OCS_EVENT_LEVEL_INFO, NULL, "[ecuReqId + ecuSwDid][start Time](INST_ Parse package config file：progress[%d], fileSize[%lld]kb)"},
{"PS_INST_011", ADM_OCS_EVENT_LEVEL_INFO, NULL, "[ecuReqId + ecuSwDid][end Time](INST_ Parse package config file：progress[%d], fileSize[%lld]kb)"},
{"PS_INST_012", ADM_OCS_EVENT_LEVEL_INFO, NULL, "[ecuReqId + ecuSwDid][expend Time](INST_ Parse package config file：progress[%d], fileSize[%lld]kb)"},
{"PS_INST_013", ADM_OCS_EVENT_LEVEL_INFO, NULL, "[ecuReqId + ecuSwDid][start Time](INST_ VDI flash：progress[%d], fileSize[%lld]kb)"},
{"PS_INST_014", ADM_OCS_EVENT_LEVEL_INFO, NULL, "[ecuReqId + ecuSwDid][end Time](INST_ VDI flash：progress[%d], fileSize[%lld]kb)"},
{"PS_INST_015", ADM_OCS_EVENT_LEVEL_INFO, NULL, "[ecuReqId + ecuSwDid][expend Time](INST_ VDI flash：progress[%d], fileSize[%lld]kb)"},
{"PS_INST_016", ADM_OCS_EVENT_LEVEL_INFO, NULL, "[ecuReqId + ecuSwDid][start Time](INST_ UCM flash：progress[%d], fileSize[%lld]kb)"},
{"PS_INST_017", ADM_OCS_EVENT_LEVEL_INFO, NULL, "[ecuReqId + ecuSwDid][end Time](INST_ UCM flash：progress[%d], fileSize[%lld]kb)"},
{"PS_INST_018", ADM_OCS_EVENT_LEVEL_INFO, NULL, "[ecuReqId + ecuSwDid][expend Time](INST_ UCM flash：progress[%d], fileSize[%lld]kb)"},
{"PS_INST_019", ADM_OCS_EVENT_LEVEL_INFO, NULL, "[ecuReqId + ecuSwDid][start Time](INST_ VDI rollback：progress[%d], fileSize[%lld]kb)"},
{"PS_INST_020", ADM_OCS_EVENT_LEVEL_INFO, NULL, "[ecuReqId + ecuSwDid][end Time](INST_ VDI rollback：progress[%d], fileSize[%lld]kb)"},
{"PS_INST_021", ADM_OCS_EVENT_LEVEL_INFO, NULL, "[ecuReqId + ecuSwDid][expend Time](INST_ VDI roback：progress[%d], fileSize[%lld]kb)"},
{"PS_INST_022", ADM_OCS_EVENT_LEVEL_INFO, NULL, "[ecuReqId + ecuSwDid][start Time](INST_ UCM rollback：progress[%d], fileSize[%lld]kb)"},
{"PS_INST_023", ADM_OCS_EVENT_LEVEL_INFO, NULL, "[ecuReqId + ecuSwDid][end Time](INST_ UCM rollback：progress[%d], fileSize[%lld]kb)"},
{"PS_INST_024", ADM_OCS_EVENT_LEVEL_INFO, NULL, "[ecuReqId + ecuSwDid][expend Time](INST_ UCM rollback：progress[%d], fileSize[%lld]kb)"},
{"PS_INST_025", ADM_OCS_EVENT_LEVEL_INFO, NULL, "[ecuReqId + ecuSwDid][all Time](INST：ecuNum[%d], instFormat[%d](1: UCM, 2: VDI))"}

};


/**
 * @brief 检查给定的事件ID是否对应一个非关键事件。
 * 
 * @param pcEventId 事件ID字符串的指针。
 * @return 返回DBOOL类型的值，指示该事件是否为非关键事件。
 */
static DBOOL ipmIsNonCriticalEvent(const DCHAR *pcEventId)
{
    DBOOL bIsNonCritical = FALSE;

    if (NULL == pcEventId) 
    {
        return FALSE;
    }

    if (0 == adm_strcasecmp(pcEventId, "IT_002") ||
        0 == adm_strcasecmp(pcEventId, "IT_003") ||
        0 == adm_strcasecmp(pcEventId, "IT_004") ||
        0 == adm_strcasecmp(pcEventId, "IT_005") ||
        0 == adm_strcasecmp(pcEventId, "IT_006") ||
        0 == adm_strcasecmp(pcEventId, "IT_007") ||
        0 == adm_strcasecmp(pcEventId, "IP_003") ||
        0 == adm_strcasecmp(pcEventId, "IP_004") ||
        0 == adm_strcasecmp(pcEventId, "IP_005") ||
        0 == adm_strcasecmp(pcEventId, "II_009") ||
        0 == adm_strcasecmp(pcEventId, "S_IE_007"))
    {
        bIsNonCritical = TRUE;
    }

    return bIsNonCritical;
}



/**
 * @brief 判断是否统计事件
 * @param[in]  pcEventId 事件id
 * @retval    DBOOL
 */
static DBOOL ipmIsStatisticsisEvent(const DCHAR *pcEventId)
{
    DBOOL bIsStatisticsisEvent = FALSE;

    // log_d("eventId[%s]", pcEventId);

    /** 判断事件ID中是否包含PS_INST_ */
    if (ADM_IS_NULL(adm_strstr(pcEventId, "PS_INST_")))
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
 * 创建事件节点.
 *
 * \note 
 *
 * @param [in]  pcEventId      事件id
 * @param [in]  i32EventLevel  事件级别：1:表示TRACE级别 2:表示DEBUG级别 3:表示INFO级别 4:表示WARN级别 5:表示ERROR级别
 * @param [in]  pcEventDesc    事件描述
 * @param [in]  pcEventContext 事件上下文(非必填字段)
 * @param [in]  ieventSequenceId 事件序列id
 * \retval      adm_ipm_EVENTNODE_T *
 *******************************************************************************
 */
static adm_ipm_EVENTNODE_T *__adm_ipm_createEventNode
(    
    const DCHAR *pcEventId,
    D32S   i32EventLevel,    
    DCHAR *pcEventDesc,
    const DCHAR *pcEventContext,
    D64S   ieventSequenceId
)
{
    adm_ipm_EVENTNODE_T *pstEventNode = NULL;

    if (ADM_IS_NULL(pcEventId) || ADM_IS_NULL(pcEventDesc))
    {
        log_e("Invalid input param");
        return NULL;
    }

    pstEventNode = (adm_ipm_EVENTNODE_T *)adm_malloc(sizeof(*pstEventNode));
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
static void __adm_ipm_freeEventNode(adm_ipm_EVENTNODE_T *pstEventNode)
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
static void __adm_ipm_cleanEventListInfo
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
static void __adm_ipm_cleanEventCache(void)
{
    struct list_head *pstNode     = NULL;
    struct list_head *pstNextNode = NULL;
    adm_ipm_EVENTNODE_T *pstEventNode = NULL;
    
    //!< 模块未初始化，则直接报错
    if (FALSE == gstIpmRptCloudHandle.isInited)
    {
        return;
    }

    (void)pthread_mutex_lock(&gstIpmRptCloudHandle.stEventList.stMutex);
    list_for_each_safe(pstNode, pstNextNode, &gstIpmRptCloudHandle.stEventList.stHead)
    {
        //!< 取节点
        pstEventNode = list_entry(pstNode, adm_ipm_EVENTNODE_T, stHead);

        //摘链 and 释放节点 
        list_del(&pstEventNode->stHead);
        __adm_ipm_freeEventNode(pstEventNode);
        pstEventNode = NULL;
    }
    gstIpmRptCloudHandle.stEventList.s32Cnt = 0;
    (void)pthread_mutex_unlock(&gstIpmRptCloudHandle.stEventList.stMutex);

    return;
}


static DCHAR* __adm_ipm_getDateAndTimeStr(void)
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

static DCHAR* __adm_ipm_getNewEventDescInfo(DCHAR* moduleName,const DCHAR* eventDesc)
{
    DCHAR* peventDesc = NULL;

    peventDesc = (DCHAR*)adm_malloc(EVNET_DESC_LEN);
    if (NULL == peventDesc) 
    {
        log_e("Failed to malloc, err[%d]", ADM_ECODE_NO_MEMORY);
        return NULL;
    }
    (void)adm_memset(peventDesc, 0x00, EVNET_DESC_LEN, EVNET_DESC_LEN);

    (void)adm_snprintf(peventDesc, EVNET_DESC_LEN, "[%s][%s]%s", 
            moduleName, 
            __adm_ipm_getDateAndTimeStr(), 
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
void adm_ipm_fflushEventReport(void)
{
    D32S iRet = ADM_ECODE_FAILED;
    D32S i = 0;
    struct list_head *pstNode     = NULL;
    struct list_head *pstNextNode = NULL;
    adm_ipm_EVENTNODE_T *pstEventNode = NULL;
	ADM_OCS_EVENTS_INFO_T* pReportEvents = NULL;
    ADM_OCS_EVENTS_INFO_LIST_T stEventsList = {0};

    //!< 模块未初始化，则直接报错
    if (FALSE == gstIpmRptCloudHandle.isInited)
    {
        log_e("ECm module not init, err[%d]", ADM_ECODE_MODULE_UNINIT);
        return;
    }

    (void)pthread_mutex_lock(&gstIpmRptCloudHandle.stEventList.stMutex);
    (void)adm_memzero(&stEventsList, sizeof(stEventsList));
    stEventsList.reportEvents = (ADM_OCS_EVENTS_INFO_T *)adm_malloc((D32U)gstIpmRptCloudHandle.stEventList.s32Cnt * sizeof(ADM_OCS_EVENTS_INFO_T));
    if (ADM_IS_NULL(stEventsList.reportEvents))
    {
        (void)pthread_mutex_unlock(&gstIpmRptCloudHandle.stEventList.stMutex);
        log_e("Failed to malloc, err[%d]", ADM_ECODE_NO_MEMORY);
        return;
    }
    (void)adm_memzero(stEventsList.reportEvents, ((D32U)gstIpmRptCloudHandle.stEventList.s32Cnt * sizeof(ADM_OCS_EVENTS_INFO_T)));
    stEventsList.reportEventsLen = gstIpmRptCloudHandle.stEventList.s32Cnt;

    list_for_each_safe(pstNode, pstNextNode, &gstIpmRptCloudHandle.stEventList.stHead)
    {
        //!< 取节点
        pstEventNode = list_entry(pstNode, adm_ipm_EVENTNODE_T, stHead);
		
		pReportEvents = &stEventsList.reportEvents[i];        
        pReportEvents->eventTime       = pstEventNode->stEventInfo.eventTime;
        pReportEvents->eventLevel      = pstEventNode->stEventInfo.eventLevel;
        pReportEvents->eventId         = adm_strdup(pstEventNode->stEventInfo.eventId);
        pReportEvents->eventDesc       = adm_strdup(pstEventNode->stEventInfo.eventDesc);
        pReportEvents->eventSequenceId = pstEventNode->stEventInfo.eventSequenceId;
        pReportEvents->eventContext = 
                    ((NULL != pstEventNode->stEventInfo.eventContext) ? (adm_strdup(pstEventNode->stEventInfo.eventContext)) : (adm_strdup("None")));

        //!< debug
        //adm_ecs_AddEventsinfotofile_test(stEventsList.reportEvents[i].eventId,
        //                                 stEventsList.reportEvents[i].eventLevel,
        //                                 stEventsList.reportEvents[i].eventDesc,
        //                                 stEventsList.reportEvents[i].eventContext);

        ++i;

        //摘链 and 释放节点 
        list_del(&pstEventNode->stHead);
        __adm_ipm_freeEventNode(pstEventNode);
        pstEventNode = NULL;

        //!< 做个保护，其实没必要 
        if (i == stEventsList.reportEventsLen)
        {
            break;
        }
    }

    gstIpmRptCloudHandle.stEventList.s32Cnt = 0;
    (void)pthread_mutex_unlock(&gstIpmRptCloudHandle.stEventList.stMutex);

    //!< 正式上报 
    if (0 < stEventsList.reportEventsLen)
    {
        (void)pthread_rwlock_rdlock(&gstIpmRptCloudHandle.stRWlock);
        iRet = adm_irm_reportEvent(gstIpmRptCloudHandle.stRptParam.sessionId, 
                    gstIpmRptCloudHandle.stRptParam.taskId,
                    gstIpmRptCloudHandle.stRptParam.strategyId,
                    time(NULL), 
                    gstIpmRptCloudHandle.stRptParam.sdkVersion, 
                    NULL, 
                    &stEventsList);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to report Event, err[%d]", iRet);
        }
        (void)pthread_rwlock_unlock(&gstIpmRptCloudHandle.stRWlock);

        //!< 清空内存 
        __adm_ipm_cleanEventListInfo(&stEventsList);
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
static void adm_ipm_addEventAndReport
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
    adm_ipm_EVENTNODE_T *pstEventNode = NULL;
    DCHAR *pcNewEventDesc = NULL;
    D32S ievent_report_level = ADM_CFG_VUS_EVENT_REPORT_LEVEL_DEFAULT_VAL;

    if (ADM_IS_NULL(pcEventId) || ADM_IS_NULL(pcEventDesc))
    {
        log_e("Invalid input param");
        return;
    }

    //!< 模块未初始化，则直接报错
    if (FALSE == gstIpmRptCloudHandle.isInited)
    {
        log_e("ipm report cloud module not init, err[%d]", ADM_ECODE_MODULE_UNINIT);
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
    pcNewEventDesc = __adm_ipm_getNewEventDescInfo(MOD_NAME(eModuleID), pcEventDesc);
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
    if (TRUE == ipmIsStatisticsisEvent(pcEventId))
    {
        // log_d(" skip report statisticsis event[%s]", pcEventId);
        adm_free(pcNewEventDesc);
        return ;
    }

#ifdef ADM_REPORT_EVENT_OPTIMIZE
    /** 非重要统计事件，只需要打印即可，暂时不进行上报 */
    if (TRUE == ipmIsNonCriticalEvent(pcEventId))
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
        pstEventNode = __adm_ipm_createEventNode(pcEventId, i32EventLevel, pcNewEventDesc, pcEventContext, iEventSequenceId);
        if (ADM_IS_NULL(pstEventNode))
        {
            log_e("Failed to malloc, err[%d]", ADM_ECODE_NO_MEMORY);
            adm_free(pcNewEventDesc);
            return;
        }

        //!< 事件节点加链表 
        (void)pthread_mutex_lock(&gstIpmRptCloudHandle.stEventList.stMutex); 
        gstIpmRptCloudHandle.stEventList.s32Cnt++;
        list_add_tail(&pstEventNode->stHead, &gstIpmRptCloudHandle.stEventList.stHead);  //!< 添加到控制链表中 

        iBakCnt = gstIpmRptCloudHandle.stEventList.s32Cnt;  //!< 备份当前缓存数量，减少下面使用锁 
        (void)pthread_mutex_unlock(&gstIpmRptCloudHandle.stEventList.stMutex);

        //!< 缓存数据量到达 or 事件结束-关键点，则刷新缓存 
        if (adm_ipm_EVENT_MAX_NUM <= iBakCnt
          || (FALSE != isReportImmediate)) 
        {
            adm_ipm_fflushEventReport();
        }
    }
    adm_free(pcNewEventDesc);

    return;
}






/*!
 *******************************************************************************
 * ipm report cloud 模块初始化.
 *
 * \note
 * 
 * \param  none
 * \retval D32S
 *******************************************************************************
 */
static D32S adm_ipm_reportEventInit(void)
{
    D32S iRet = ADM_ECODE_SUCCESSED;

    if (TRUE == gstIpmRptCloudHandle.isInited)
    {
        log_i("ipm report event module already init.");
        return iRet;
    }

    do
    {
        //!< 事件队列 
        iRet = pthread_mutex_init(&gstIpmRptCloudHandle.stEventList.stMutex, NULL);
        if (ADM_IS_ERR(iRet))
        {
            iRet = ADM_ECODE_RWLOCK_INIT_FAILED;
            log_e("Failed to init mutex, err[%d]", iRet);
            break;
        }
        INIT_LIST_HEAD(&gstIpmRptCloudHandle.stEventList.stHead);
        gstIpmRptCloudHandle.stEventList.s32Cnt = 0;

        iRet = pthread_rwlock_init(&gstIpmRptCloudHandle.stRWlock, NULL);
        if (ADM_IS_ERR(iRet))
        {
            iRet = ADM_ECODE_RWLOCK_INIT_FAILED;
            log_e("Failed to init rwlock, err[%d]", iRet);
            (void)pthread_mutex_destroy(&gstIpmRptCloudHandle.stEventList.stMutex);
            break;
        }
        
        //!< 相关参数信息在api: adm_ipm_checkAndUpdateDownReportInfo 中更新，此处赋初值
        gstIpmRptCloudHandle.stRptParam.sessionId  = NULL;
        gstIpmRptCloudHandle.stRptParam.sdkVersion = NULL;
        gstIpmRptCloudHandle.stRptParam.taskId     = 0;
        gstIpmRptCloudHandle.stRptParam.strategyId = 0;

        gstIpmRptCloudHandle.isInited = TRUE;
    } while (FALSE);

    if (ADM_IS_ERR(iRet))
    {
        (void)adm_ipm_reportEventDeInit();
    }

//    log_d("===> ipm init report event module ok.");

    return iRet;
}

/*!
 *******************************************************************************
 * ipm report cloud 模块销毁.
 *
 * \note
 * 
 * \param  none 模块信息
 * \retval none
 *******************************************************************************
 */
void adm_ipm_reportEventDeInit(void)
{
    if (TRUE == gstIpmRptCloudHandle.isInited)
    {
        gstIpmRptCloudHandle.isInited = FALSE;
    
        /** 释放事件上报链表 */
        __adm_ipm_cleanEventCache();
        (void)pthread_mutex_destroy(&gstIpmRptCloudHandle.stEventList.stMutex);
        gstIpmRptCloudHandle.stEventList.s32Cnt = 0;

        (void)pthread_rwlock_wrlock(&gstIpmRptCloudHandle.stRWlock);
        adm_free(gstIpmRptCloudHandle.stRptParam.sessionId);
        adm_free(gstIpmRptCloudHandle.stRptParam.sdkVersion);
        gstIpmRptCloudHandle.stRptParam.taskId     = 0;
        gstIpmRptCloudHandle.stRptParam.strategyId = 0;
        (void)pthread_rwlock_unlock(&gstIpmRptCloudHandle.stRWlock);
        (void)pthread_rwlock_destroy(&gstIpmRptCloudHandle.stRWlock);
    }

    return;
}

/*!
 *******************************************************************************
 * ipm report cloud模块内部接口初始化.
 *
 * \note
 * 
 * \param[out]  pstOptIf 模块接口信息
 * \retval      D32S
 *******************************************************************************
 */
D32S adm_ipm_eventInfoInit(void)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
	ADM_IPS_POLICY_TASK_T task;

    if (FALSE == gstIpmRptCloudHandle.isInited)
    {
        iRet = adm_ipm_reportEventInit();
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to init ipm report cloud module, err[%d]", iRet);
            return iRet;
        }
    }

    do
    {

        //!< 缓存任务头信息更新
        (void)pthread_rwlock_wrlock(&gstIpmRptCloudHandle.stRWlock);
        adm_free(gstIpmRptCloudHandle.stRptParam.sdkVersion); 
        gstIpmRptCloudHandle.stRptParam.sdkVersion  = adm_strdup(ADM_SDK_VERSION);
        (void)pthread_rwlock_unlock(&gstIpmRptCloudHandle.stRWlock);

		(void)adm_memzero(&task, sizeof(ADM_IPS_POLICY_TASK_T));
		adm_ips_policyTaskInit(&task);
		iRet = adm_ips_policyTaskGet(&task);
	    if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to get ipm report task info, err[%d]", iRet);
            return iRet;
        }
		
		if ((NULL == task.sessionId) || (0 == task.taskId) || (0 == task.strategyId))
		{
		    adm_ips_policyTaskDeInit(&task);
			log_w("ReportEvent task parameter error");
			return ADM_ECODE_INVALID_PARAM;
		}
   
        //!< 缓存任务头信息更新
        (void)pthread_rwlock_wrlock(&gstIpmRptCloudHandle.stRWlock);
        adm_free(gstIpmRptCloudHandle.stRptParam.sessionId);
        gstIpmRptCloudHandle.stRptParam.sessionId  = adm_strdup(task.sessionId);
        gstIpmRptCloudHandle.stRptParam.taskId     = task.taskId;
        gstIpmRptCloudHandle.stRptParam.strategyId = task.strategyId;
        (void)pthread_rwlock_unlock(&gstIpmRptCloudHandle.stRWlock);

		adm_ips_policyTaskDeInit(&task);
    } while (FALSE);


    log_d("===> ipm init report event module ok.");
    return iRet;
}


static D32S ipmFindReportEventInfoEventId(ADM_OCS_EVENTS_INFO_T* pEvent, const DCHAR* pEventId)
{
    D32U i = 0;
    D32S iRet = ADM_ECODE_FAILED;
    ADM_IPM_REPORT_EVENT_INFO_T* pEventInfo = NULL;

    if(pEvent == NULL || pEventId == NULL)
    {
        log_w("event[ %p ] eventId[ %p ]", pEvent, pEventId);
        return ADM_ECODE_INVALID_PARAM;
    }
    //todo: get the event->eventSequenceId from IRS,but the function not define now 
//    pEvent->eventSequenceId = adm_irm_getEventSequenceId();
	
    for (i = 0; i < sizeof(m_ipm_report_event_list)/sizeof(ADM_IPM_REPORT_EVENT_INFO_T); ++i)
    {
        pEventInfo = &m_ipm_report_event_list[i];
		if (0 == adm_strncmp(pEventId, pEventInfo->eventId, (D32U)strlen(pEventId)))
        {
            pEvent->eventId = pEventInfo->eventId;
            pEvent->eventLevel = (D32S)pEventInfo->eventLevel;
            pEvent->eventContext = pEventInfo->eventContext;
            pEvent->eventDesc = pEventInfo->eventDesc;
//            pEvent->eventTime = time(NULL);
			
            iRet = ADM_ECODE_SUCCESSED;
			break;
        }
    }

    if(iRet != ADM_ECODE_SUCCESSED)
    {
        log_w("eventId [ %s ] can not find in m_ipm_report_event_list", pEventId);
        
    }

    return iRet;
}
///根据事件ID上报事件
void ipmReportEventDependencyEventId(const DCHAR *pEventId, const DCHAR *pEventDesc, D32S iErroCode)
{
	D32S iRet = ADM_ECODE_FAILED;

	if(pEventId == NULL)
	{
		log_w("ReportEvent eventId error [ %p ] ", pEventId);
		return ;
	}
	
	//init event
	ADM_OCS_EVENTS_INFO_T event = {0};	
	if(ADM_ECODE_SUCCESSED != ipmFindReportEventInfoEventId(&event, pEventId))
	{
		log_w("can not find pEventId");
		return ;
	}

	D32S	iEventDescLen = 0;
	DCHAR*	pEventDescBuf = adm_malloc(EVNET_DESC_LEN);
	if (NULL == pEventDescBuf)
	{
		log_w("malloc pEventDescBuf failed");
		return ;
	}
	(void)adm_memzero(pEventDescBuf, EVNET_DESC_LEN);

	if(pEventDesc != NULL)
	{
	    if (ADM_ECODE_SUCCESSED == iErroCode)
    	{
            (void)adm_snprintf(pEventDescBuf, (DSIZE)EVNET_DESC_LEN, "(%s)", pEventDesc);
    	}
		else
		{
            (void)adm_snprintf(pEventDescBuf, (DSIZE)EVNET_DESC_LEN, "(%s %d)", pEventDesc, iErroCode);
    	}	
	}
	else
	{
	    if (ADM_ECODE_SUCCESSED == iErroCode)
    	{
            (void)adm_snprintf(pEventDescBuf, (DSIZE)EVNET_DESC_LEN, "(%s)", event.eventDesc);
    	}
		else
		{
            (void)adm_snprintf(pEventDescBuf, (DSIZE)EVNET_DESC_LEN, "(%s %d)", event.eventDesc, iErroCode);
    	}	
	}	

	DBOOL  isReportImmediate = FALSE;
	if(event.eventLevel == ADM_OCS_EVENT_LEVEL_ERROR)
	{
		isReportImmediate = TRUE;

	}
		//S_IE_001安装开始和S_IE_002安装完成事件需要立即上报
	if((0 == adm_strncmp(pEventId, "S_IE_001", (D32U)strlen(pEventId)))||(0 == adm_strncmp(pEventId, "S_IE_002", (D32U)strlen(pEventId))))
	{
		isReportImmediate = TRUE;

	}

    /** 远程上下电事件需要立即上报 */
    if ((0 == adm_strncmp(pEventId, "II_021", (D32U)strlen(pEventId))) || 
        (0 == adm_strncmp(pEventId, "II_022", (D32U)strlen(pEventId))) ||
        (0 == adm_strncmp(pEventId, "II_023", (D32U)strlen(pEventId))) ||
        (0 == adm_strncmp(pEventId, "II_024", (D32U)strlen(pEventId))))
    {
        isReportImmediate = TRUE;
    }

    adm_ipm_addEventAndReport(ADM_COM_MOD_IPM, isReportImmediate, event.eventId, event.eventLevel, pEventDescBuf,  event.eventContext);

    log_i("[ABUP-Procedure] Event Id:%s, Level:%d, Desc:%s", event.eventId, event.eventLevel, pEventDescBuf);
	adm_free(pEventDescBuf);	   
	return ;
}

void ipmReportTriggerInstallEventDependencyTriggerType(ADM_APPLICATION_COMMON_CMDID_TYPE_E eTriggerType)
{
 //   D32S nRet = ADM_ECODE_FAILED;
    switch(eTriggerType){
	case ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_USER:
    case ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_USB:
	    ipmReportEventDependencyEventId("IT_002", NULL, ADM_ECODE_SUCCESSED);
	    break;
	case ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_IGN:
	    ipmReportEventDependencyEventId("IT_001", NULL, ADM_ECODE_SUCCESSED);
	    break;
	case ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_TSP:
	    ipmReportEventDependencyEventId("IT_003", NULL, ADM_ECODE_SUCCESSED);
	    break;
	case ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_PHONE:
	    ipmReportEventDependencyEventId("IT_006", NULL, ADM_ECODE_SUCCESSED);
	    break;
//	case ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_CYCLE:
//	    ipmReportEventDependencyEventId("IT_004", NULL, ADM_ECODE_SUCCESSED);
//	    break;
	case ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_SCHDULE:
	    ipmReportEventDependencyEventId("IT_004", NULL, ADM_ECODE_SUCCESSED);
	    break;
	case ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_SILENCE:
	    ipmReportEventDependencyEventId("IT_007", NULL, ADM_ECODE_SUCCESSED);
	    break;
    case ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_RESCUE:
        ipmReportEventDependencyEventId("IT_008", NULL, ADM_ECODE_SUCCESSED);
        break;
    case ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_PROD_LINE:
        ipmReportEventDependencyEventId("IT_009", NULL, ADM_ECODE_SUCCESSED);
        break;
    case ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_APK_SELF_UPGRADE:
        ipmReportEventDependencyEventId("IT_010", NULL, ADM_ECODE_SUCCESSED);
        break;
    case ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_FORCE:
        ipmReportEventDependencyEventId("IT_011", NULL, ADM_ECODE_SUCCESSED);
        break;
	default:
		log_w("unknown trigger type %d", eTriggerType);
		break;
    }
    return ;
}

static const DCHAR* ipmTime2Str(D64S iTime, DCHAR* pOut, D32S iOutSize)
{
    time_t Time = (time_t)iTime;
    struct tm* pTm = localtime(&Time);
    if (pTm == NULL) {
        return "";
    }

    (void)adm_snprintf(pOut, (D32U)iOutSize, "%04d-%02d-%02d %02d:%02d:%02d", 
        1900 + pTm->tm_year, pTm->tm_mon + 1, pTm->tm_mday,
        pTm->tm_hour, pTm->tm_min, pTm->tm_sec);

    return pOut;
}

static const DCHAR* ipmSecondsHourStr(D32U uiSeconds, DCHAR* pOut, D32S iOutSize)
{
    D32U uiSecond = uiSeconds % 60U;
    D32U uiMinute = (uiSeconds % 3600U) / 60U;
    D32U uiHour = (uiSeconds % (3600U * 24U)) / 3600U;
    D32S iOffset = 0;

    if (uiHour > 0U)
    {
        iOffset = adm_snprintf(pOut, (D32U)iOutSize, "%02u:", uiHour);
    }

    (void)adm_snprintf(&pOut[iOffset], (DSIZE)iOutSize - (DSIZE)iOffset, "%02u:%02u", uiMinute, uiSecond);
    return pOut;
}


static void ipmReportStatEventByEventId(const DCHAR* pEventId, DCHAR* pEventDesc)
{
    ipmReportEventDependencyEventId(pEventId, pEventDesc, ADM_ECODE_SUCCESSED);    
    return;
}


static const DCHAR* ipmGeStatEventTypeStr(IPM_STATISTICS_EVENT_E itype)
{
    const DCHAR* pStr = "";

    switch (itype)
    {
    case STAT_EVENT_LOCAL_VERIFY_SIGN:
        pStr = "Local signatrue verify";
        break;
    case STAT_EVENT_REMOTE_VERIFY_SIGN:
        pStr = "Remote signature verify";
        break;
    case STAT_EVENT_UNZIP_PACKAGE:
        pStr = "unzip";
        break;
    case STAT_EVENT_PARSE_PAKAGE_CFG:
        pStr = "Parse package config file";
        break;
    case STAT_EVENT_VDI_FLASH:
        pStr = "VDI flash";
        break;
    case STAT_EVENT_UCM_FLASH:
        pStr = "UCM flash";
        break;
    case STAT_EVENT_VDI_ROLLBACK:
        pStr = "VDI rollback";
        break;
    case STAT_EVENT_UCM_ROLLBACK:
        pStr = "UCM rollback";
        break;
    default:
        pStr = "Invalid";
        break;
    }

    return pStr;
}



static void ipmReportStatisticsEvent(D32S iEventId, const DCHAR* pEventName, 
    const DCHAR* pEcuReqId, const DCHAR* pEcuSwDid, 
    const DCHAR* pTimeStr, D32S iProgress, D64S iFileSize)
{
    DCHAR   eventId[32] = {0};
    DCHAR   eventDesc[256] = {0};

    (void)adm_snprintf(eventId, sizeof(eventId), "PS_INST_%03d", iEventId);
    (void)adm_snprintf(eventDesc, (D32U)sizeof(eventDesc), 
                       "[%s + %s][%s](INST_ %s: progress[%d] fileSize[%lld]kb)", 
                       pEcuReqId, pEcuSwDid, pTimeStr, pEventName, iProgress, iFileSize);
    ipmReportStatEventByEventId(eventId, eventDesc);
}

void ipmStatEventReport(const IPM_STAT_EVENT_T* pStatEvent)
{
	DCHAR			acTimeStr[24] = {0};
	const DCHAR*	pEventName = ipmGeStatEventTypeStr((IPM_STATISTICS_EVENT_E)pStatEvent->eventId);
	D64S			iFileSize = (pStatEvent->fileSize + 1023) / 1024;

	ipmReportStatisticsEvent(pStatEvent->eventId, pEventName, pStatEvent->ecuReqId, pStatEvent->ecuSwDid,
					   ipmTime2Str((D64S)pStatEvent->startTime, acTimeStr, (D32S)sizeof(acTimeStr)), 0, iFileSize);
	ipmReportStatisticsEvent(STAT_EVENT_EVENT_END_ID(pStatEvent->eventId), pEventName, pStatEvent->ecuReqId, pStatEvent->ecuSwDid,
					   ipmTime2Str((D64S)pStatEvent->endTime, acTimeStr, (D32S)sizeof(acTimeStr)), pStatEvent->progress, iFileSize);
	ipmReportStatisticsEvent(STAT_EVENT_EVENT_STAT_ID(pStatEvent->eventId), pEventName, pStatEvent->ecuReqId, pStatEvent->ecuSwDid,
					   ipmSecondsHourStr((D32U)(pStatEvent->endTime - pStatEvent->startTime), acTimeStr, (D32S)sizeof(acTimeStr)), 
					   pStatEvent->progress, iFileSize);
}

void ipmStatEventReportReport(ADM_IPM_STAT_EVENT_REPORT_T* pReport, const IPM_STAT_EVENT_T* pStatEvent)
{
	ADM_IPM_STAT_EVENT_SUMARY_T* pSumary = NULL;

	ipmStatEventReport(pStatEvent);

	for (D32S i = 0; i < (D32S)(sizeof(pReport->sumarys) / sizeof(pReport->sumarys[0])); i++)
	{
		pSumary = &pReport->sumarys[i];
		if (pStatEvent->serviceType == pSumary->serviceType)
		{
			pSumary->totalTime += (D32U)pStatEvent->endTime - (D32U)pStatEvent->startTime;
			break;
		}
	}
}


static void ipmStatEventSumaryInit(ADM_IPM_STAT_EVENT_SUMARY_T* pSumary, D32S iServiceType)
{
    pSumary->serviceType = iServiceType;
    pSumary->ecuNum = 0;
    pSumary->totalTime = 0;
}

void ipmStatEventReportInit(ADM_IPM_STAT_EVENT_REPORT_T* pReport)
{
    ipmStatEventSumaryInit(&pReport->sumarys[0], 1);
    ipmStatEventSumaryInit(&pReport->sumarys[1], 2);
}

void ipmStatEventReportDeInit(ADM_IPM_STAT_EVENT_REPORT_T* pReport)
{
}

static void ipmStatEventReportSetEcuNum(ADM_IPM_STAT_EVENT_REPORT_T* pReport)
{
    ADM_ECS_ECUCONFIG_T* pEcsEcu = NULL;
    ADM_IPM_STAT_EVENT_SUMARY_T* pSumary = NULL;

    ADM_IPS_PROGRESS_T progress;
    adm_ips_progressInit(&progress);
    adm_ips_progressGet(&progress);

    for (D32S i = 0; i < progress.ecus.ecusLen; ++i) {
        ADM_IPS_ECU_PROGRESS_T* pEcu = &progress.ecus.ecus[i];
        if (ADM_ECODE_SUCCESSED != adm_ecs_getEcuConfig(pEcu->ecuReqId, pEcu->ecuSwDid, &pEcsEcu)) {
            log_e("Failed to getEcuConfig");
            continue;
        }

        for (D32S j = 0; j < (D32S)(sizeof(pReport->sumarys) / sizeof(pReport->sumarys[0])); j++)
        {
            pSumary = &pReport->sumarys[j];
            if (pEcsEcu->serviceType == pSumary->serviceType)
            {
                pSumary->ecuNum++;
                break;
            }
        }

        adm_ecs_delEcuConfigInfo(&pEcsEcu);
    }

    adm_ips_progressDeInit(&progress);
}


void ipmStatEventReportEnd(ADM_IPM_STAT_EVENT_REPORT_T* pReport)
{
    DCHAR   acTimeStr[24] = { 0 };
    DCHAR   eventDesc[256] = { 0 };
    const ADM_IPM_STAT_EVENT_SUMARY_T*   pSumary = NULL;

    ipmStatEventReportSetEcuNum(pReport);

    for (D32S i = 0; i < (D32S)(sizeof(pReport->sumarys) / sizeof(pReport->sumarys[0])); i++)
    {
        pSumary = &pReport->sumarys[i];
        if (pSumary->ecuNum > 0)
        {
            (void)adm_snprintf(eventDesc, (D32U)sizeof(eventDesc), 
                               "[%s](INST: ecuNum[%d], instFormat[%d](1: VDI 2: UCM)",
                               ipmSecondsHourStr((pSumary->totalTime), acTimeStr, (D32S)sizeof(acTimeStr)),
                               pSumary->ecuNum, pSumary->serviceType);
            ipmReportStatEventByEventId("PS_INST_025", eventDesc);
        }
    }
}


