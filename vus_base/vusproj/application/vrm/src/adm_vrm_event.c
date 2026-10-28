/**
 * @file adm_vrm_event.c
 * @author mengwei@abupdate.com
 * @brief 
 * @version 0.1
 * @date 2022-03-22
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#include "adm_vrm_event.h"
#include "adm_list.h"
#include "adm_com_errorCode.h"
#include "adm_ocs_airProtocolAdapter.h"
#include "adm_vrm_log.h"
#include "adm_string.h"
#include "adm_sdk_version.h"
#include "adm_memory.h"
#include "adm_cfg_api.h"
#include "adm_ecs_rsConfig.h"
#include <pthread.h>
#include <sys/time.h>

/**
 * @brief 定义事件存储的链表
 * 
 */
typedef struct {
    ADM_OCS_EVENTS_INFO_T* eventInfo;   //!< 链表待存储的对象
    struct list_head entry;             //!< 链表的入口指针
} ADM_VRM_EV_NODE_INFO_T;

static struct list_head g_head;            //!< list 表头定义
static DBOOL g_isReportInit;                  //!< 事件上报模块是否初始化
static ADM_VRM_IRM_GET_EV_SEQID_CALLBACK_F g_getSeqIdCb = NULL;         //!< 获取事件顺序ID
static ADM_VRM_IRM_REPORT_EVENT_CALLBACK_F g_reportEventCb = NULL;      //!< 执行事件上报
static D32S g_reportLevel = 0;              //!< 事件上报的 Event Level 定义

struct listLockVrm {
    pthread_mutex_t lock;
    int initState;
};

static struct listLockVrm _lock = {.initState = 0};

/**
 * @brief 检查给定的事件ID是否对应一个关键事件。
 * 
 * @param pcEventId 事件ID字符串的指针。
 * @return 返回DBOOL类型的值，指示该事件是否为关键事件。
 */
static DBOOL vrmIsCriticalEvent(const DCHAR *pcEventId)
{
    DBOOL bIsCritical = FALSE;

    if (NULL == pcEventId) 
    {
        return FALSE;
    }

    if (0 == adm_strcasecmp(pcEventId, "ZT_001") ||
        0 == adm_strcasecmp(pcEventId, "ZT_002") ||
        0 == adm_strcasecmp(pcEventId, "ZT_003") ||
        0 == adm_strcasecmp(pcEventId, "ZT_004") ||
        0 == adm_strcasecmp(pcEventId, "ZP_004") ||
        0 == adm_strcasecmp(pcEventId, "ZP_007") ||
        0 == adm_strcasecmp(pcEventId, "ZP_009") ||
        0 == adm_strcasecmp(pcEventId, "ZI_004") ||
        0 == adm_strcasecmp(pcEventId, "ZI_007") ||
        0 == adm_strcasecmp(pcEventId, "ZI_009") ||
        0 == adm_strcasecmp(pcEventId, "ZI_020") ||
        0 == adm_strcasecmp(pcEventId, "ZI_022") ||
        0 == adm_strcasecmp(pcEventId, "ZI_025") ||
        0 == adm_strcasecmp(pcEventId, "S_ZE_001") ||
        0 == adm_strcasecmp(pcEventId, "S_ZE_002"))
    {
        bIsCritical = TRUE;
    }

    return bIsCritical;
}

static void lockInitOnce(void)
{
    if (_lock.initState == 1) {
        return;
    }

    (void)pthread_mutex_init(&_lock.lock, NULL);
    _lock.initState = 1;
}

static D32S eventNodeLock(void)
{
    lockInitOnce();
    return pthread_mutex_lock(&_lock.lock);
}

static D32S eventNodeUnlock(void)
{
    lockInitOnce();
    return pthread_mutex_unlock(&_lock.lock);
}

/**
 * @brief 初始化静态全局链表
 * 
 */
static void __adm_vrm_evNodeListInit(void)
{
    (void)adm_memset(&g_head, 0x00, sizeof(struct list_head), sizeof(struct list_head));
    INIT_LIST_HEAD(&g_head);
    return;
}

/**
 * @brief 创建新的事件节点
 * 
 * @param eventId       [IN]    事件ID
 * @param eventLevel    [IN]    事件级别
 * @param eventDesc     [IN]    事件描述，用来描述事件信息
 * @param eventContext  [IN]    事件上下文，用于记录事件产生时的详细上下文
 * @return ADM_VRM_EV_NODE_INFO_T*  NULL: failed, None-NULL: Successed
 */
static ADM_VRM_EV_NODE_INFO_T* __adm_vrm_evNodeNew(const DCHAR* eventId, const D32S eventLevel, const DCHAR* eventDesc, const DCHAR* eventContext)
{
    ADM_VRM_EV_NODE_INFO_T* evNode = NULL;

    do {
        //!< 申请事件节点的内存
        evNode = (ADM_VRM_EV_NODE_INFO_T*)adm_malloc(sizeof(ADM_VRM_EV_NODE_INFO_T));
        if (NULL == evNode) {
            log_e("adm_malloc ADM_VRM_EV_NODE_INFO_T failed");
            break;
        }
        (void)adm_memset(evNode, 0x00, sizeof(ADM_VRM_EV_NODE_INFO_T), sizeof(ADM_VRM_EV_NODE_INFO_T));

        //!< 申请事件信息的内存
        evNode->eventInfo = (ADM_OCS_EVENTS_INFO_T*)adm_malloc(sizeof(ADM_OCS_EVENTS_INFO_T));
        if (NULL == evNode->eventInfo) {
            adm_free(evNode);
            log_e("adm_malloc ADM_OCS_EVENTS_INFO_T failed");
            break;
        }
        (void)adm_memset(evNode->eventInfo, 0x00, sizeof(ADM_OCS_EVENTS_INFO_T), sizeof(ADM_OCS_EVENTS_INFO_T));

        //!< 数据填充
        if (NULL != eventContext) {
            evNode->eventInfo->eventContext = adm_strdup(eventContext);
        }

        if (NULL != eventDesc) {
            evNode->eventInfo->eventDesc = adm_strdup(eventDesc);
        }

        if (NULL != eventId) {
            evNode->eventInfo->eventId = adm_strdup(eventId);
        }

        evNode->eventInfo->eventLevel = eventLevel;
        evNode->eventInfo->eventSequenceId = g_getSeqIdCb();
        evNode->eventInfo->eventTime = time(NULL);

        log_evt("eventID:%s, eventLevel:%d, eventSequenceId:%lld, eventTime:%lld, eventDesc:%s, eventContext:%s", 
                evNode->eventInfo->eventId,
                evNode->eventInfo->eventLevel,
                evNode->eventInfo->eventSequenceId,
                evNode->eventInfo->eventTime,
                evNode->eventInfo->eventDesc,
                (NULL == evNode->eventInfo->eventContext) ? "NULL" : evNode->eventInfo->eventContext);
    } while(FALSE);

    return evNode;
}

static DCHAR* __adm_vrm_getTimeStr(void)
{
    static DCHAR timeStr[20] = {0};
    time_t timep;
    struct tm *p;
    struct timeval tv;

    (void)time(&timep);
    p = localtime(&timep);
    (void)gettimeofday(&tv, NULL);
    if (p == NULL) {
        return NULL;
    }
    (void)snprintf(timeStr, sizeof(timeStr), "%04d-%02d-%02d %02d:%02d:%02d", 1900 + p->tm_year, p->tm_mon + 1, p->tm_mday,
            p->tm_hour, p->tm_min, p->tm_sec);
            
    return timeStr;
}

/**
 * @brief 增加事件给到事件列表
 * 
 * @param eventId       [IN]    事件ID
 * @param eventLevel    [IN]    事件级别
 * @param eventDesc     [IN]    事件描述，用来描述事件信息
 * @param eventContext  [IN]    事件上下文，用于记录事件产生时的详细上下文
 * @return D32S         0:Successed, None-0:Failed
 */
static D32S __adm_vrm_addReportEvent(const DCHAR* eventId, const D32S eventLevel, const DCHAR* eventDesc, const DCHAR* eventContext)
{
    D32S iRet = ADM_ECODE_FAILED;
    ADM_VRM_EV_NODE_INFO_T* evNode = NULL;

    (void)eventNodeLock();
    do {
        //!< 必填字段判断
        if (NULL == eventId || NULL == eventDesc) {
            iRet = ADM_ECODE_INVALID_PARAM;
            log_e("Input param evnetId | eventDesc is nil, [err:%d]", iRet);
            break;
        }

        //!< 创建事件节点
        evNode = __adm_vrm_evNodeNew(eventId, eventLevel, eventDesc, eventContext);
        if (NULL == evNode) {
            iRet = ADM_ECODE_APPLY_MEMORY_FAILED;
            log_e("__adm_vrm_evNodeNew failed, [err:%d]", iRet);
            break;
        }

        //!< 将事件节点，添加到链表的尾部
        list_add_tail(&evNode->entry, &g_head);
        iRet = ADM_ECODE_SUCCESSED;
    } while(FALSE);
    (void)eventNodeUnlock();

    return iRet;
}

/**
 * @brief 根据上层传入的事件阶段和事件序号生成事件ID
 * 
 * @param eStage        [IN]    事件阶段
 * @param serNum        [IN]    事件序号
 * @return DCHAR*       NULL:Failed, None-NULL: 事件ID
 */
#define EVENT_ID_LEN    10      //!< 定义事件ID的长度
static DCHAR* __adm_vrm_generateEventId(const ADM_VRM_EV_STAGE_E eStage, const D32U serNumber)
{
    DCHAR* eventId = NULL;

    (void)eventNodeLock();
    do {
        eventId = (DCHAR*)adm_malloc(EVENT_ID_LEN);
        if (NULL == eventId) {
            log_e("adm_malloc eventId size:%d failed", EVENT_ID_LEN);
            break;
        }
        (void)adm_memset(eventId, 0x00, EVENT_ID_LEN, EVENT_ID_LEN);

        switch (eStage)
        {
        case ADM_VRM_EV_STAGE_TG:
            (void)adm_snprintf(eventId, EVENT_ID_LEN, "ZT_%03d", serNumber);
            break;
        case ADM_VRM_EV_STAGE_PRE:
            (void)adm_snprintf(eventId, EVENT_ID_LEN, "ZP_%03d", serNumber);
            break;
        case ADM_VRM_EV_STAGE_ING:
            (void)adm_snprintf(eventId, EVENT_ID_LEN, "ZI_%03d", serNumber);
            break;
        case ADM_VRM_EV_STAGE_END:
            (void)adm_snprintf(eventId, EVENT_ID_LEN, "S_ZE_%03d", serNumber);
            break;
        case ADM_VRM_EV_STAGE_INV:
        default:
            log_w("event stage %d invalid", eStage);
            //!< 事件ID无效，置为NULL
            adm_free(eventId);
            eventId = NULL;
            break;
        }
    } while(FALSE);
    (void)eventNodeUnlock();

    return eventId;
}

#define EVNET_DESC_LEN  512    //!< 事件描述信息的长度
#define EVENT_MODELE_NAME   "VRM" //!< 产生当前事件的模块名

static void __adm_vrm_doEventReport(const ADM_VRM_EV_STAGE_E eStage, const D32U serNumber, DBOOL  isReportImmediate)
{
    DBOOL isDoReport = FALSE;
    static D32S repotCount = 0;
    /**
     * @brief 上报规则
     * 1. 大于默认条数需要触发上报
     * 2. 因为是初始化进程，需要等IRM初始化成功后上报；
     * 3. 因为是守护进程，所以应该在ing成功时上报；
     * 4. end是否需要上报？？？
     */
    do {
        repotCount += 1;
        if (repotCount >= ADM_VRM_EVENT_REPORT_COUNT) {
            isDoReport = TRUE;
        }

        if (ADM_VRM_EV_STAGE_END == eStage && (D32U)ADM_VRM_EV_INIT_ING_MAX == serNumber) {
            isDoReport = TRUE;
        }
		if(isReportImmediate) {
            isDoReport = TRUE;
        }

        //TODO: ADM_VRM_EV_STAGE_END == eStage ???

        if (TRUE == isDoReport) {
            (void)adm_vrm_triggerReportEvent();
            isDoReport = FALSE;
            repotCount = 0;
        }
    } while(FALSE);

    return;
}

/**
 * @brief 根据序号生成触发阶段的eventDesc
 * 
 * @param serNumber     [IN]    事件序号
 * @param errCode       [IN]    事件错误码
 * @param eventDesc     [OUT]   NULL:Failed, None-NULL:eventDest
 */
static void __adm_vrm_generateEventTgDesc(const D32U serNumber, const D32S errCode, DCHAR** eventDesc)
{
    //!< 注册触发阶段的事件描述信息获取
    switch (serNumber)
    {
    case 1:
        (void)adm_snprintf(*eventDesc, EVNET_DESC_LEN, "[%s][%s](%s)", 
                EVENT_MODELE_NAME, 
                __adm_vrm_getTimeStr(), 
                "After the program is successfully initialized, registration is triggered.");
        break;
    case 2:
        (void)adm_snprintf(*eventDesc, EVNET_DESC_LEN, "[%s][%s](%s%d)", 
                EVENT_MODELE_NAME, 
                __adm_vrm_getTimeStr(), 
                "Start retry registration.",
                errCode);
        break;
    case 3:
        (void)adm_snprintf(*eventDesc, EVNET_DESC_LEN, "[%s][%s](%s)", 
                EVENT_MODELE_NAME, 
                __adm_vrm_getTimeStr(), 
                "Start periodic registration.");
        break;
    case 4:
        (void)adm_snprintf(*eventDesc, EVNET_DESC_LEN, "[%s][%s](%s)", 
                EVENT_MODELE_NAME, 
                __adm_vrm_getTimeStr(), 
                "Cloud response specified status code: 2003. Registration needs to be requested again.");
        break;
    default:
        log_i("Stage of event trigger serNumber:%d is not defined, please go to define this event, thx", serNumber);
        break;
    }

    return;
}

/**
 * @brief 根据序号生成准备阶段的eventDesc
 * 
 * @param serNumber     [IN]    事件序号
 * @param errCode       [IN]    事件错误码
 * @param eventDesc     [OUT]   NULL:Failed, None-NULL:eventDest
 */
static void __adm_vrm_generateEventPreDesc(const D32U serNumber, const D32S errCode, DCHAR** eventDesc)
{
    //!< 注册触发阶段的事件描述信息获取
    switch (serNumber)
    {
    case 1:
        (void)adm_snprintf(*eventDesc, EVNET_DESC_LEN, "[%s][%s](%s)", 
                EVENT_MODELE_NAME, 
                __adm_vrm_getTimeStr(), 
                "Request the DIM module to get the VIN.");
        break;
    case 2:
        (void)adm_snprintf(*eventDesc, EVNET_DESC_LEN, "[%s][%s](%s)", 
                EVENT_MODELE_NAME, 
                __adm_vrm_getTimeStr(), 
                "Test vehicle and get the VIN code from the configuration file.");
        break;
    case 3:
        (void)adm_snprintf(*eventDesc, EVNET_DESC_LEN, "[%s][%s](%s)", 
                EVENT_MODELE_NAME, 
                __adm_vrm_getTimeStr(), 
                "Get VIN successed.");
        break;
    case 4:
        (void)adm_snprintf(*eventDesc, EVNET_DESC_LEN, "[%s][%s](%s%d)", 
                EVENT_MODELE_NAME, 
                __adm_vrm_getTimeStr(), 
                "Request the DIM module to get the VIN failed, err:",
                errCode);
        break;
    case 5:
        (void)adm_snprintf(*eventDesc, EVNET_DESC_LEN, "[%s][%s](%s)", 
                EVENT_MODELE_NAME, 
                __adm_vrm_getTimeStr(), 
                "Request to get SDK version of UCM.");
        break;
    case 6:
        (void)adm_snprintf(*eventDesc, EVNET_DESC_LEN, "[%s][%s](%s)", 
                EVENT_MODELE_NAME, 
                __adm_vrm_getTimeStr(), 
                "Get SDK version of UCM successed. ");
        break;
    case 7:
        (void)adm_snprintf(*eventDesc, EVNET_DESC_LEN, "[%s][%s](%s%d)", 
                EVENT_MODELE_NAME, 
                __adm_vrm_getTimeStr(), 
                "Get SDK version of UCM failed. err:",
                errCode);
        break;
    case 8:
        (void)adm_snprintf(*eventDesc, EVNET_DESC_LEN, "[%s][%s](%s)", 
                EVENT_MODELE_NAME, 
                __adm_vrm_getTimeStr(), 
                "Initialize registration request information and apply for registration request information space.");
        break;
    case 9:
        (void)adm_snprintf(*eventDesc, EVNET_DESC_LEN, "[%s][%s](%s%d)", 
                EVENT_MODELE_NAME, 
                __adm_vrm_getTimeStr(), 
                "Apply for registration request information space failed, err:",
                errCode);
        break;
    case 10:
        (void)adm_snprintf(*eventDesc, EVNET_DESC_LEN, "[%s][%s](%s)", 
                EVENT_MODELE_NAME, 
                __adm_vrm_getTimeStr(), 
                "Apply for registration request information space successed.");
        break;
    case 11:
        (void)adm_snprintf(*eventDesc, EVNET_DESC_LEN, "[%s][%s](%s)", 
                EVENT_MODELE_NAME, 
                __adm_vrm_getTimeStr(), 
                "Fill in the registration request information.");
        break;
    default:
        log_i("Stage of event prepare serNumber:%d is not defined, please go to define this event, thx", serNumber);
        break;
    }

    return;
}


/**
 * @brief 根据序号生成进行阶段的eventDesc
 * 
 * @param serNumber     [IN]    事件序号
 * @param errCode       [IN]    事件错误码
 * @param eventDesc     [OUT]   NULL:Failed, None-NULL:eventDest
 */
static void __adm_vrm_generateEventIngDesc(const D32U serNumber, const D32S errCode, DCHAR** eventDesc)
{
    switch (serNumber)
    {
    case 1:
        (void)adm_snprintf(*eventDesc, EVNET_DESC_LEN, "[%s][%s](%s)", 
                EVENT_MODELE_NAME, 
                __adm_vrm_getTimeStr(), 
                "Request to perform registration.");
        break;
    case 2:
        (void)adm_snprintf(*eventDesc, EVNET_DESC_LEN, "[%s][%s](%s)", 
                EVENT_MODELE_NAME, 
                __adm_vrm_getTimeStr(), 
                "Request OCS module to serialize registration request data.");
        break;
    case 3:
        (void)adm_snprintf(*eventDesc, EVNET_DESC_LEN, "[%s][%s](%s)", 
                EVENT_MODELE_NAME, 
                __adm_vrm_getTimeStr(), 
                "Sign the registration request data. Sign string:");
        break;
    case 4:
        (void)adm_snprintf(*eventDesc, EVNET_DESC_LEN, "[%s][%s](%s)", 
                EVENT_MODELE_NAME, 
                __adm_vrm_getTimeStr(), 
                "Sign the registration request data failed.");
        break;
    case 5:
        (void)adm_snprintf(*eventDesc, EVNET_DESC_LEN, "[%s][%s](%s)", 
                EVENT_MODELE_NAME, 
                __adm_vrm_getTimeStr(), 
                "Use asymmetric encryption of registration request data.");
        break;
    case 6:
        (void)adm_snprintf(*eventDesc, EVNET_DESC_LEN, "[%s][%s](%s)", 
                EVENT_MODELE_NAME, 
                __adm_vrm_getTimeStr(), 
                "Send the registration request to the cloud server.");
        break;
    case 7:
        (void)adm_snprintf(*eventDesc, EVNET_DESC_LEN, "[%s][%s](%s%d)", 
                EVENT_MODELE_NAME, 
                __adm_vrm_getTimeStr(), 
                "Registration response failed.err:",
                errCode);
        break;
    case 8:
        (void)adm_snprintf(*eventDesc, EVNET_DESC_LEN, "[%s][%s](%s)", 
                EVENT_MODELE_NAME, 
                __adm_vrm_getTimeStr(), 
                "Registration response successed.");
        break;
    case 9:
        (void)adm_snprintf(*eventDesc, EVNET_DESC_LEN, "[%s][%s](%s%d)", 
                EVENT_MODELE_NAME, 
                __adm_vrm_getTimeStr(), 
                "Apply for registration response information sapce failed, err:",
                errCode);
        break;
    case 10:
        (void)adm_snprintf(*eventDesc, EVNET_DESC_LEN, "[%s][%s](%s)", 
                EVENT_MODELE_NAME, 
                __adm_vrm_getTimeStr(), 
                "Apply for registration response information sapce successed.");
        break;
    case 11:
        (void)adm_snprintf(*eventDesc, EVNET_DESC_LEN, "[%s][%s](%s)", 
                EVENT_MODELE_NAME, 
                __adm_vrm_getTimeStr(), 
                "Request OCS module to deserialization registration request data.");
        break;
    case 12:
        (void)adm_snprintf(*eventDesc, EVNET_DESC_LEN, "[%s][%s](%s)", 
                EVENT_MODELE_NAME, 
                __adm_vrm_getTimeStr(), 
                "Obtain the secret required for signature verification through the status code responded by the cloud.");
        break;
    case 13:
        (void)adm_snprintf(*eventDesc, EVNET_DESC_LEN, "[%s][%s](%s)", 
                EVENT_MODELE_NAME, 
                __adm_vrm_getTimeStr(), 
                "Obtain the secret required for signature verification through the status code responded by the cloud.status code");
        break;
    case 14:
        (void)adm_snprintf(*eventDesc, EVNET_DESC_LEN, "[%s][%s](%s)", 
                EVENT_MODELE_NAME, 
                __adm_vrm_getTimeStr(), 
                "Use asymmetric decryption of registration request data.");
        break;
    case 15:
        (void)adm_snprintf(*eventDesc, EVNET_DESC_LEN, "[%s][%s](%s)", 
                EVENT_MODELE_NAME, 
                __adm_vrm_getTimeStr(), 
                "Get the status code in the cloud response information.");
        break;
    case 16:
        (void)adm_snprintf(*eventDesc, EVNET_DESC_LEN, "[%s][%s](%s)", 
                EVENT_MODELE_NAME, 
                __adm_vrm_getTimeStr(), 
                "Asynchronous processing of cloud response status codes.");
        break;
    case 17:
        (void)adm_snprintf(*eventDesc, EVNET_DESC_LEN, "[%s][%s](%s)", 
                EVENT_MODELE_NAME, 
                __adm_vrm_getTimeStr(), 
                "Transfer cloud status code to local status code.clode status code");
        break;
    case 18:
        (void)adm_snprintf(*eventDesc, EVNET_DESC_LEN, "[%s][%s](%s)", 
                EVENT_MODELE_NAME, 
                __adm_vrm_getTimeStr(), 
                "Calculate the signature information of the cloud response data.");
        break;
    case 19:
        (void)adm_snprintf(*eventDesc, EVNET_DESC_LEN, "[%s][%s](%s)", 
                EVENT_MODELE_NAME, 
                __adm_vrm_getTimeStr(), 
                "Signature verification succeeded.");
        break;
    case 20:
        (void)adm_snprintf(*eventDesc, EVNET_DESC_LEN, "[%s][%s](%s)", 
                EVENT_MODELE_NAME, 
                __adm_vrm_getTimeStr(), 
                "Signature verification failed.");
        break;
    case 21:
        (void)adm_snprintf(*eventDesc, EVNET_DESC_LEN, "[%s][%s](%s)", 
                EVENT_MODELE_NAME, 
                __adm_vrm_getTimeStr(), 
                "Parse the cloud response successfully.");
        break;
    case 22:
        (void)adm_snprintf(*eventDesc, EVNET_DESC_LEN, "[%s][%s](%s)", 
                EVENT_MODELE_NAME, 
                __adm_vrm_getTimeStr(), 
                "Parse the cloud response failed.");
        break;
    case 23:
        (void)adm_snprintf(*eventDesc, EVNET_DESC_LEN, "[%s][%s](%s)", 
                EVENT_MODELE_NAME, 
                __adm_vrm_getTimeStr(), 
                "Process registration response data and store registration response data.");
        break;
    case 24:
        (void)adm_snprintf(*eventDesc, EVNET_DESC_LEN, "[%s][%s](%s)", 
                EVENT_MODELE_NAME, 
                __adm_vrm_getTimeStr(), 
                "Process registration response data and store registration response data successed.");
        break;
    case 25:
        (void)adm_snprintf(*eventDesc, EVNET_DESC_LEN, "[%s][%s](%s%d)", 
                EVENT_MODELE_NAME, 
                __adm_vrm_getTimeStr(), 
                "Process registration response data and store registration response data failed. Error code:",
                errCode);
        break;
    default:
        log_i("Stage of event prepare serNumber:%d is not defined, please go to define this event, thx", serNumber);
        break;
    }
}

/**
 * @brief 根据序号生成结束阶段的eventDesc
 * 
 * @param serNumber     [IN]    事件序号
 * @param errCode       [IN]    事件错误码
 * @param eventDesc     [OUT]   NULL:Failed, None-NULL:eventDest
 */
static void __adm_vrm_generateEventEndDesc(const D32U serNumber, const D32S errCode, DCHAR** eventDesc)
{
    switch (serNumber)
    {
    case 1:
        (void)adm_snprintf(*eventDesc, EVNET_DESC_LEN, "[%s][%s](%s)", 
                EVENT_MODELE_NAME, 
                __adm_vrm_getTimeStr(), 
                "Registration success.");
        break;
    case 2:
        (void)adm_snprintf(*eventDesc, EVNET_DESC_LEN, "[%s][%s](%s%d)", 
                EVENT_MODELE_NAME, 
                __adm_vrm_getTimeStr(), 
                "Registration failed. Error code:",
                errCode);
        break;
    case 3:
        (void)adm_snprintf(*eventDesc, EVNET_DESC_LEN, "[%s][%s](%s)", 
                EVENT_MODELE_NAME, 
                __adm_vrm_getTimeStr(), 
                "Free up space applied for during the registration process.");
        break;
    default:
        break;
    }
}

/**
 * @brief 通过事件ID，获取事件描述信息
 * 
 * @param eStage        [IN]    事件阶段
 * @param serNum        [IN]    事件序号
 * @param errCode       [IN]    事件错误码
 * @param eventContext  [IN]    事件详细上下文，用于记录事件产生时的详细上下文
 * @return DCHAR*   NULL:Failed, None-NULL: eventDesc
 */
static DCHAR* __adm_vrm_generateEventDesc(const ADM_VRM_EV_STAGE_E eStage, const D32U serNumber, const D32S errCode, const DCHAR* eventContext)
{
    DCHAR* eventDesc = NULL;
	DCHAR* eventDescTmp = NULL;

    (void)eventNodeLock();
    do {
        //!< 申请1K空间，用于存储事件描述信息

		eventDescTmp = (DCHAR*)adm_malloc(EVNET_DESC_LEN);
        if (NULL == eventDescTmp) {
            log_e("adm_malloc size:%d  failed", EVNET_DESC_LEN);
            break;
        }
        (void)adm_memset(eventDescTmp, 0x00, EVNET_DESC_LEN, EVNET_DESC_LEN);
        eventDesc = (DCHAR*)adm_malloc(EVNET_DESC_LEN);
        if (NULL == eventDesc) {
            log_e("adm_malloc size:%d  failed", EVNET_DESC_LEN);
            break;
        }
        (void)adm_memset(eventDesc, 0x00, EVNET_DESC_LEN, EVNET_DESC_LEN);

        switch (eStage)
        {
        case ADM_VRM_EV_STAGE_TG:
            __adm_vrm_generateEventTgDesc(serNumber, errCode, &eventDescTmp);
            break;
        case ADM_VRM_EV_STAGE_PRE:
            __adm_vrm_generateEventPreDesc(serNumber, errCode, &eventDescTmp);
            break;
        case ADM_VRM_EV_STAGE_ING:
            __adm_vrm_generateEventIngDesc(serNumber, errCode, &eventDescTmp);
            break;
        case ADM_VRM_EV_STAGE_END:
            __adm_vrm_generateEventEndDesc(serNumber, errCode, &eventDescTmp);
            break;
        case ADM_VRM_EV_STAGE_INV:
        default:
            log_w("event stage %d invalid", eStage);
            //!< 事件Desc无效，置为NULL
            adm_free(eventDescTmp);
            eventDescTmp = NULL;
            adm_free(eventDesc);
            eventDesc = NULL;
            break;
        }
    } while(FALSE);
		
	if ((NULL != eventDescTmp)&&(NULL != eventDesc)) {
		if (NULL != eventContext) {
			if (*(eventDescTmp+adm_strnlen(eventDescTmp, EVNET_DESC_LEN)-1) == ')') {
                *(eventDescTmp+adm_strnlen(eventDescTmp, EVNET_DESC_LEN)-1) = '\0';
			}
		    (void)adm_snprintf(eventDesc, EVNET_DESC_LEN, "%s%s)", eventDescTmp, eventContext);
		}
		else{
            (void)adm_snprintf(eventDesc, EVNET_DESC_LEN, "%s", eventDescTmp);
		}
		
    }

	adm_free(eventDescTmp);
    (void)eventNodeUnlock();


    return eventDesc;
}

/**
 * @brief 增加事件给到事件列表
 * 
 * @param eStage        [IN]    事件阶段
 * @param serNum        [IN]    事件序号
 * @param eventLevel    [IN]    事件级别
 * @param errCode       [IN]    事件错误码，成功：0，失败：错误码
 * @param eventContext  [IN]    事件详细上下文，用于记录事件产生时的详细上下文
 */
void adm_vrm_eventAdd(const D32S eStage, const D32U serNumber, const D32S eventLevel, const D32S errCode, const DCHAR* eventContext)
{
    DCHAR* eventId = NULL;
    DCHAR* eventDesc = NULL;

    do {
        //!< 先生成事件ID
        eventId = __adm_vrm_generateEventId((ADM_VRM_EV_STAGE_E)eStage, serNumber);
        if (NULL == eventId) {
            log_e("eStage:%d, serNumber:%d generateEventId failed", eStage, serNumber);
            break;
        }

        //!< 再生成事件描述信息
        eventDesc = __adm_vrm_generateEventDesc((ADM_VRM_EV_STAGE_E)eStage, serNumber, errCode, eventContext);
        if (NULL == eventDesc) {
            log_e("eStage:%d, serNumber:%d generateEventDesc failed", eStage, serNumber);
            break;
        }

		if (ADM_ECODE_SUCCESSED != adm_ecs_getEventReportLevel(&g_reportLevel))
		{
	        g_reportLevel = adm_cfg_iniParserGetInt(ADM_CFG_VUS_EVENT_REPORT_LEVEL, ADM_CFG_VUS_EVENT_REPORT_LEVEL_DEFAULT_VAL);
		}

    #ifdef ADM_REPORT_EVENT_OPTIMIZE
        if (FALSE == vrmIsCriticalEvent(eventId))
        {
            //!< 非重要事件不需要上报，只需要在日志中打印即可
            log_evt("eventID:%s, eventLevel:%d, eventReportLevel:%d, eventDesc:%s, eventContext:%s",
                    eventId,
                    eventLevel,
                    g_reportLevel,
                    eventDesc,
                    (NULL == eventContext) ? "NULL" : eventContext);
            break;
        }
    #endif

        if (g_reportLevel <= eventLevel)
        {
            //!< 将事件放到链表中，只有当前事件的级别 >= 定义需要上报的级别，才需要上报
            (void)__adm_vrm_addReportEvent(eventId, eventLevel, eventDesc, NULL);
        }
        else
        {
            //!< 事件不需要上报，只需要在日志中打印即可
            log_evt("eventID:%s, eventLevel:%d, eventReportLevel:%d, eventDesc:%s, eventContext:%s",
                    eventId,
                    eventLevel,
                    g_reportLevel,
                    eventDesc,
                    "NULL");
        }

    } while(FALSE);

    //!< 释放内存
    adm_free(eventId);
    adm_free(eventDesc);

    //!< 执行当前上报规则的判断，然后执行上报
    if(eventLevel == ADM_OCS_EVENT_LEVEL_ERROR)
    {
        __adm_vrm_doEventReport((ADM_VRM_EV_STAGE_E)eStage, serNumber, TRUE);
    }
	else
	{
        __adm_vrm_doEventReport((ADM_VRM_EV_STAGE_E)eStage, serNumber, FALSE);
    }
	
    return;
}

/**
 * @brief 获取链表的长度
 * 
 * @param head      [IN]    链表头
 * @return D32U     链表长度
 */
static D32U __adm_vrm_getEvNodeListLen(struct list_head head)
{
    D32U len = 0;
    ADM_VRM_EV_NODE_INFO_T* pos = NULL;
    ADM_VRM_EV_NODE_INFO_T* next = NULL;

    list_for_each_entry_safe(pos, next, &g_head, ADM_VRM_EV_NODE_INFO_T, entry) {
        len++;
    }

    log_i("Length of event node list is :%d", len);

    return len;
}

/**
 * @brief 释放事件信息
 * 
 * @param eventInfo     [IN]    事件信息
 */
static void __adm_vrm_delEventInfo(ADM_OCS_EVENTS_INFO_T* eventInfo)
{
    if (NULL != eventInfo) {
        if (NULL != eventInfo->eventContext) {
            adm_free(eventInfo->eventContext);
        }

        if (NULL != eventInfo->eventDesc) {
            adm_free(eventInfo->eventDesc);
        }

        if (NULL != eventInfo->eventId) {
            adm_free(eventInfo->eventId);
        }
        eventInfo->eventLevel = 0;
        eventInfo->eventSequenceId = 0;
        eventInfo->eventTime = 0;
    }
}


/**
 * @brief 将存储在链表里的数据，填充到事件上报的接口中，并删除链表中已经存在的信息
 * 
 * @return ADM_OCS_EVENTS_INFO_LIST_T*  NULL:Failed, None-NULL:Successed
 */
static ADM_OCS_EVENTS_INFO_LIST_T* __adm_vrm_fieldEventsListForReport(void)
{
    D32U len = 0;
    D32S i = 0;
    ADM_VRM_EV_NODE_INFO_T* pos = NULL;
    ADM_VRM_EV_NODE_INFO_T* next = NULL;
    ADM_OCS_EVENTS_INFO_LIST_T* eventsList = NULL;

    do {
        len = __adm_vrm_getEvNodeListLen(g_head);
        
        //!< 申请事件上报列表的空间
        eventsList = (ADM_OCS_EVENTS_INFO_LIST_T*)adm_malloc(sizeof(ADM_OCS_EVENTS_INFO_LIST_T));
        (void)adm_memset(eventsList, 0x00, sizeof(ADM_OCS_EVENTS_INFO_LIST_T), sizeof(ADM_OCS_EVENTS_INFO_LIST_T));
        eventsList->reportEventsLen = (D32S)len;
        eventsList->reportEvents = (ADM_OCS_EVENTS_INFO_T*)adm_malloc(sizeof(ADM_OCS_EVENTS_INFO_T) * (D32U)eventsList->reportEventsLen);
        (void)adm_memset(eventsList->reportEvents, 0x00, sizeof(ADM_OCS_EVENTS_INFO_T) * (D32U)eventsList->reportEventsLen, sizeof(ADM_OCS_EVENTS_INFO_T) * (D32U)eventsList->reportEventsLen);

        list_for_each_entry_safe(pos, next, &g_head, ADM_VRM_EV_NODE_INFO_T, entry) {
            if (NULL != pos->eventInfo) {
                if (NULL != pos->eventInfo->eventContext) {
                    eventsList->reportEvents[i].eventContext = adm_strdup(pos->eventInfo->eventContext);
                }
                if (NULL != pos->eventInfo->eventDesc) {
                    eventsList->reportEvents[i].eventDesc = adm_strdup(pos->eventInfo->eventDesc);
                }
                if (NULL != pos->eventInfo->eventId) {
                    eventsList->reportEvents[i].eventId = adm_strdup(pos->eventInfo->eventId);
                }
                eventsList->reportEvents[i].eventLevel = pos->eventInfo->eventLevel;
                eventsList->reportEvents[i].eventTime = pos->eventInfo->eventTime;
                eventsList->reportEvents[i].eventSequenceId = pos->eventInfo->eventSequenceId;
            }
            i++;
            list_del(&pos->entry);
            __adm_vrm_delEventInfo(pos->eventInfo);
        }
    } while(FALSE);

    return eventsList;
}

/**
 * @brief 释放上报的事件列表信息
 * 
 * @param eventsList    [IN]    事件列表信息
 */
static void __adm_vrm_delEventsList(ADM_OCS_EVENTS_INFO_LIST_T* eventsList)
{
    D32S i = 0;

    if (NULL != eventsList) {
        for (i = 0; i < eventsList->reportEventsLen; i++) {
            if (NULL != eventsList->reportEvents[i].eventContext) {
                adm_free(eventsList->reportEvents[i].eventContext);
            }

            if (NULL != eventsList->reportEvents[i].eventDesc) {
                adm_free(eventsList->reportEvents[i].eventDesc);
            }

            if (NULL != eventsList->reportEvents[i].eventId) {
                adm_free(eventsList->reportEvents[i].eventId);
            }
            eventsList->reportEvents[i].eventLevel = 0;
            eventsList->reportEvents[i].eventSequenceId = 0;
            eventsList->reportEvents[i].eventTime = 0;
        }

        adm_free(eventsList->reportEvents);
        eventsList->reportEventsLen = 0;
        adm_free(eventsList);
    }

    return ;
}

/**
 * @brief 触发上报接口，上报APP CORE模块产生的事件信息
 * 
 */
D32S adm_vrm_triggerReportEvent(void)
{
    D32S iRet = ADM_ECODE_FAILED;
    ADM_OCS_EVENTS_INFO_LIST_T* eventsList = NULL;
    D32S i = 0;
    D64S lTimeStamp = time(NULL);
    
    (void)eventNodeLock();
    do {
        //!< 将链表里的内容填充到事件上报的接口中
        eventsList = __adm_vrm_fieldEventsListForReport();
        if (NULL == eventsList) {
            log_e("__adm_vrm_fieldEventsListForReport failed");
            break;
        }

        log_i("eventsList length:%d", eventsList->reportEventsLen);
        for (i = 0; i < eventsList->reportEventsLen; i++) {
            log_i("The reported eventId:%s", eventsList->reportEvents[i].eventId);
        }

        //!< 初始化阶段是没有sessionId, strategyId和taskId, 所以填充默认值
        if (NULL != g_reportEventCb) {
            iRet = g_reportEventCb(NULL, 0, 0, lTimeStamp, adm_sdk_getSDKVersion(), NULL, eventsList);
        }

        __adm_vrm_delEventsList(eventsList);
    } while(FALSE);
    (void)eventNodeUnlock();

    return iRet;
}

/**
 * @brief 初始化程序启动阶段的事件上报模块
 * 
 */
void adm_vrm_initReportEvent(ADM_VRM_IRM_GET_EV_SEQID_CALLBACK_F getSeqIdCb, 
                                    ADM_VRM_IRM_REPORT_EVENT_CALLBACK_F reportEventCb)

{
    //!< 初始化链表和上报接口
    if (FALSE == g_isReportInit) {
        __adm_vrm_evNodeListInit();
        g_reportEventCb = reportEventCb;
        g_getSeqIdCb = getSeqIdCb;
        g_isReportInit = TRUE;
        g_reportLevel = adm_cfg_iniParserGetInt(ADM_CFG_VUS_EVENT_REPORT_LEVEL, ADM_CFG_VUS_EVENT_REPORT_LEVEL_DEFAULT_VAL);
    }

    return;
}
