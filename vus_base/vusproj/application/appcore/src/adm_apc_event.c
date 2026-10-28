/**
 * @file adm_apc_event.c
 * @author mengwei@abupdate.com
 * @brief 
 * @version 0.1
 * @date 2022-03-22
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#include "adm_apc_event.h"
#include "adm_list.h"
#include "adm_com_errorCode.h"
#include "adm_ocs_airProtocolAdapter.h"
#include "adm_apc_log.h"
#include "adm_string.h"
#include "adm_memory.h"
#include "adm_sdk_version.h"
#include "adm_cfg_api.h"
#include <pthread.h>
#include <sys/time.h>

/**
 * @brief 定义事件存储的链表
 * 
 */
typedef struct {
    ADM_OCS_EVENTS_INFO_T* eventInfo;   //!< 链表待存储的对象
    struct list_head entry;             //!< 链表的入口指针
} ADM_APC_EV_NODE_INFO_T;

static struct list_head g_head;            //!< list 表头定义
static DBOOL g_isReportInit;                  //!< 事件上报模块是否初始化
static ADM_APC_IRM_GET_EV_SEQID_CALLBACK_F g_getSeqIdCb = NULL;         //!< 获取事件顺序ID
static ADM_APC_IRM_REPORT_EVENT_CALLBACK_F g_reportEventCb = NULL;      //!< 执行事件上报
static D32S g_reportLevel = 0;              //!< 事件上报的 Event Level 定义

struct listLock {
    pthread_mutex_t lock;
    int initState;
};

static struct listLock _lock = {.initState = 0};

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
static void __adm_apc_evNodeListInit(void)
{
    (void)adm_memset(&g_head, 0x00, sizeof(struct list_head), sizeof(struct list_head));
    INIT_LIST_HEAD(&g_head);
    return;
}

static DCHAR* __adm_apc_getTimeStr(void)
{
    static DCHAR timeStr[20] = {0};
    time_t timep;
    struct tm *p;
    struct timeval tv;

    (void)time(&timep);
    p = localtime(&timep);
    (void)gettimeofday(&tv, NULL);
    if (p == NULL) {
        return (DCHAR *)0;
    }
    (void)snprintf(timeStr, sizeof(timeStr), "%04d-%02d-%02d %02d:%02d:%02d", 1900 + p->tm_year, p->tm_mon + 1, p->tm_mday,
            p->tm_hour, p->tm_min, p->tm_sec);
            
    return timeStr;
}

/**
 * @brief 创建新的事件节点
 * 
 * @param eventId       [IN]    事件ID
 * @param eventLevel    [IN]    事件级别
 * @param eventDesc     [IN]    事件描述，用来描述事件信息
 * @param eventContext  [IN]    事件上下文，用于记录事件产生时的详细上下文
 * @return ADM_APC_EV_NODE_INFO_T*  NULL: failed, None-NULL: Successed
 */
static ADM_APC_EV_NODE_INFO_T* __adm_apc_evNodeNew(const DCHAR* eventId, const D32S eventLevel, const DCHAR* eventDesc, const DCHAR* eventContext)
{
    ADM_APC_EV_NODE_INFO_T* evNode = NULL;

    do {
        //!< 申请事件节点的内存
        evNode = (ADM_APC_EV_NODE_INFO_T*)adm_malloc(sizeof(ADM_APC_EV_NODE_INFO_T));
        if (NULL == evNode) {
            log_e("adm_malloc ADM_APC_EV_NODE_INFO_T failed");
            break;;
        }
        (void)adm_memset(evNode, 0x00, sizeof(ADM_APC_EV_NODE_INFO_T), sizeof(ADM_APC_EV_NODE_INFO_T));

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

/**
 * @brief 增加事件给到事件列表
 * 
 * @param eventId       [IN]    事件ID
 * @param eventLevel    [IN]    事件级别
 * @param eventDesc     [IN]    事件描述，用来描述事件信息
 * @param eventContext  [IN]    事件上下文，用于记录事件产生时的详细上下文
 * @return D32S         0:Successed, None-0:Failed
 */
static D32S __adm_apc_addReportEvent(const DCHAR* eventId, const D32S eventLevel, const DCHAR* eventDesc, const DCHAR* eventContext)
{
    D32S iRet = ADM_ECODE_FAILED;
    ADM_APC_EV_NODE_INFO_T* evNode = NULL;

    (void)eventNodeLock();
    do {
        //!< 必填字段判断
        if (NULL == eventId || NULL == eventDesc) {
            iRet = ADM_ECODE_INVALID_PARAM;
            log_e("Input param evnetId | eventDesc is nil, [err:%d]", iRet);
            break;
        }

        //!< 创建事件节点
        evNode = __adm_apc_evNodeNew(eventId, eventLevel, eventDesc, eventContext);
        if (NULL == evNode) {
            iRet = ADM_ECODE_APPLY_MEMORY_FAILED;
            log_e("__adm_apc_evNodeNew failed, [err:%d]", iRet);
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
static DCHAR* __adm_apc_generateEventId(const ADM_APC_EV_STAGE_E eStage, const D32U serNumber)
{
    DCHAR* eventId = NULL;

    (void)eventNodeLock();
    do {
        eventId = (DCHAR*)adm_malloc(EVENT_ID_LEN);
        if (NULL == eventId) {
            log_e("adm_malloc eventId size:%d failed", EVENT_ID_LEN);
            break;;
        }
        (void)adm_memset(eventId, 0x00, EVENT_ID_LEN, EVENT_ID_LEN);

        switch (eStage)
        {
        case ADM_APC_EV_STAGE_PRE:
            (void)adm_snprintf(eventId, EVENT_ID_LEN, "InitP_%03d", serNumber);
            break;
        case ADM_APC_EV_STAGE_ING:
            (void)adm_snprintf(eventId, EVENT_ID_LEN, "InitI_%03d", serNumber);
            break;
        case ADM_APC_EV_STAGE_END:
            (void)adm_snprintf(eventId, EVENT_ID_LEN, "InitE_%03d", serNumber);
            break;
        case ADM_APC_EV_STAGE_INV:
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
#define EVENT_MODELE_NAME   "APC" //!< 产生当前事件的模块名
/**
 * @brief 根据序号生成准备阶段的eventDesc
 * 
 * @param serNumber     [IN]    事件序号
 * @param errCode       [IN]    事件错误码
 * @param eventDesc     [OUT]   NULL:Failed, None-NULL:eventDest
 */
static void __adm_apc_generateEventPreDesc(const D32U serNumber, const D32S errCode, DCHAR** eventDesc)
{
    //!< 初始化准备阶段的事件描述信息获取
    switch (serNumber)
    {
    case 1:
        (void)adm_snprintf(*eventDesc, EVNET_DESC_LEN, "[%s][%s](%s)", 
                EVENT_MODELE_NAME, 
                __adm_apc_getTimeStr(), 
                "The process 'admfotaapp' starts now.");
        break;
    case 2:
        (void)adm_snprintf(*eventDesc, EVNET_DESC_LEN, "[%s][%s](%s%d)", 
                EVENT_MODELE_NAME, 
                __adm_apc_getTimeStr(), 
                "The process 'admfotaapp' starts failed, err:",
                errCode);
        break;
    case 3:
        (void)adm_snprintf(*eventDesc, EVNET_DESC_LEN, "[%s][%s](%s)", 
                EVENT_MODELE_NAME, 
                __adm_apc_getTimeStr(), 
                "The process 'admfotaapp' starts successed.");
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
static void __adm_apc_generateEventIngDesc(const D32U serNumber, const D32S errCode, DCHAR** eventDesc)
{
    switch (serNumber)
    {
    case 1:
        (void)adm_snprintf(*eventDesc, EVNET_DESC_LEN, "[%s][%s](%s)", 
                EVENT_MODELE_NAME, 
                __adm_apc_getTimeStr(), 
                "admfotaapp init ucmconfig.ini file starts now.");
        break;
    case 2:
        (void)adm_snprintf(*eventDesc, EVNET_DESC_LEN, "[%s][%s](%s%d)", 
                EVENT_MODELE_NAME, 
                __adm_apc_getTimeStr(), 
                "admfotaapp init ucmconfig.ini file failed, file parse failed, err:",
                errCode);
        break;
    case 3:
        (void)adm_snprintf(*eventDesc, EVNET_DESC_LEN, "[%s][%s](%s)", 
                EVENT_MODELE_NAME, 
                __adm_apc_getTimeStr(), 
                "admfotaapp init ucmconfig.ini file successed.");
        break;
    case 4:
        (void)adm_snprintf(*eventDesc, EVNET_DESC_LEN, "[%s][%s](%s)", 
                EVENT_MODELE_NAME, 
                __adm_apc_getTimeStr(), 
                "admfotaapp create & init directory starts now.");
        break;
    case 5:
        (void)adm_snprintf(*eventDesc, EVNET_DESC_LEN, "[%s][%s](%s%d)", 
                EVENT_MODELE_NAME, 
                __adm_apc_getTimeStr(), 
                "admfotaapp init directory failed, err:",
                errCode);
        break;
    case 6:
        (void)adm_snprintf(*eventDesc, EVNET_DESC_LEN, "[%s][%s](%s)", 
                EVENT_MODELE_NAME, 
                __adm_apc_getTimeStr(), 
                "admfotaapp init directory successed.");
        break;
    case 7:
        (void)adm_snprintf(*eventDesc, EVNET_DESC_LEN, "[%s][%s](%s)", 
                EVENT_MODELE_NAME, 
                __adm_apc_getTimeStr(), 
                "admfotaapp init log starts now.");
        break;
    case 8:
        (void)adm_snprintf(*eventDesc, EVNET_DESC_LEN, "[%s][%s](%s%d)", 
                EVENT_MODELE_NAME, 
                __adm_apc_getTimeStr(), 
                "admfotaapp init log failed, err:",
                errCode);
        break;
    case 9:
        (void)adm_snprintf(*eventDesc, EVNET_DESC_LEN, "[%s][%s](%s)", 
                EVENT_MODELE_NAME, 
                __adm_apc_getTimeStr(), 
                "admfotaapp init log successed.");
        break;
    case 10:
        (void)adm_snprintf(*eventDesc, EVNET_DESC_LEN, "[%s][%s](%s)", 
                EVENT_MODELE_NAME, 
                __adm_apc_getTimeStr(), 
                "admfotaapp init tracer tool starts now.");
        break;
    case 11:
        (void)adm_snprintf(*eventDesc, EVNET_DESC_LEN, "[%s][%s](%s%d)", 
                EVENT_MODELE_NAME, 
                __adm_apc_getTimeStr(), 
                "admfotaapp init tracer tool failed, err:",
                errCode);
        break;
    case 12:
        (void)adm_snprintf(*eventDesc, EVNET_DESC_LEN, "[%s][%s](%s)", 
                EVENT_MODELE_NAME, 
                __adm_apc_getTimeStr(), 
                "admfotaapp init tracer tool successed.");
        break;
    case 13:
        (void)adm_snprintf(*eventDesc, EVNET_DESC_LEN, "[%s][%s](%s)", 
                EVENT_MODELE_NAME, 
                __adm_apc_getTimeStr(), 
                "admfotaapp init core service starts now.");
        break;
    case 14:
        (void)adm_snprintf(*eventDesc, EVNET_DESC_LEN, "[%s][%s](%s%d)", 
                EVENT_MODELE_NAME, 
                __adm_apc_getTimeStr(), 
                "admfotaapp init core service failed, err:",
                errCode);
        break;
    case 15:
        (void)adm_snprintf(*eventDesc, EVNET_DESC_LEN, "[%s][%s](%s)", 
                EVENT_MODELE_NAME, 
                __adm_apc_getTimeStr(), 
                "admfotaapp init core service successed.");
        break;
    case 16:
        (void)adm_snprintf(*eventDesc, EVNET_DESC_LEN, "[%s][%s](%s)", 
                EVENT_MODELE_NAME, 
                __adm_apc_getTimeStr(), 
                "admfotaapp init DIM(Device interaction manager) starts now.");
        break;
    case 17:
        (void)adm_snprintf(*eventDesc, EVNET_DESC_LEN, "[%s][%s](%s%d)", 
                EVENT_MODELE_NAME, 
                __adm_apc_getTimeStr(), 
                "admfotaapp init DIM(Device interaction manager) failed, err:",
                errCode);
        break;
    case 18:
        (void)adm_snprintf(*eventDesc, EVNET_DESC_LEN, "[%s][%s](%s)", 
                EVENT_MODELE_NAME, 
                __adm_apc_getTimeStr(), 
                "admfotaapp init DIM(Device interaction manager) successed.");
        break;
    case 19:
        (void)adm_snprintf(*eventDesc, EVNET_DESC_LEN, "[%s][%s](%s)", 
                EVENT_MODELE_NAME, 
                __adm_apc_getTimeStr(), 
                "admfotaapp init UIM(User interaction manager) starts now.");
        break;
    case 20:
        (void)adm_snprintf(*eventDesc, EVNET_DESC_LEN, "[%s][%s](%s%d)", 
                EVENT_MODELE_NAME, 
                __adm_apc_getTimeStr(), 
                "admfotaapp init UIM(User interaction manager) failed, err:",
                errCode);
        break;
    case 21:
        (void)adm_snprintf(*eventDesc, EVNET_DESC_LEN, "[%s][%s](%s)", 
                EVENT_MODELE_NAME, 
                __adm_apc_getTimeStr(), 
                "admfotaapp init UIM(User interaction manager) successed.");
        break;
    case 22:
        (void)adm_snprintf(*eventDesc, EVNET_DESC_LEN, "[%s][%s](%s)", 
                EVENT_MODELE_NAME, 
                __adm_apc_getTimeStr(), 
                "admfotaapp init print all sdk version.");
        break;
    case 23:
        (void)adm_snprintf(*eventDesc, EVNET_DESC_LEN, "[%s][%s](%s)", 
                EVENT_MODELE_NAME, 
                __adm_apc_getTimeStr(), 
                "admfotaapp init managers starts now.");
        break;
    case 24:
        (void)adm_snprintf(*eventDesc, EVNET_DESC_LEN, "[%s][%s](%s%d)", 
                EVENT_MODELE_NAME, 
                __adm_apc_getTimeStr(), 
                "admfotaapp init managers failed, err:",
                errCode);
        break;
    case 25:
        (void)adm_snprintf(*eventDesc, EVNET_DESC_LEN, "[%s][%s](%s)", 
                EVENT_MODELE_NAME, 
                __adm_apc_getTimeStr(), 
                "admfotaapp init managers successed.");
        break;
    case 26:
        (void)adm_snprintf(*eventDesc, EVNET_DESC_LEN, "[%s][%s](%s)", 
                EVENT_MODELE_NAME, 
                __adm_apc_getTimeStr(), 
                "admfotaapp init start eweb server now.");
        break;
    case 27:
        (void)adm_snprintf(*eventDesc, EVNET_DESC_LEN, "[%s][%s](%s%d)", 
                EVENT_MODELE_NAME, 
                __adm_apc_getTimeStr(), 
                "admfotaapp init start eweb server failed, err:",
                errCode);
        break;
    case 28:
        (void)adm_snprintf(*eventDesc, EVNET_DESC_LEN, "[%s][%s](%s)", 
                EVENT_MODELE_NAME, 
                __adm_apc_getTimeStr(), 
                "admfotaapp init start eweb server successed.");
        break;
    case 29:
        (void)adm_snprintf(*eventDesc, EVNET_DESC_LEN, "[%s][%s](%s)", 
                EVENT_MODELE_NAME, 
                __adm_apc_getTimeStr(), 
                "admfotaapp init ECS(ECU config service) start now.");
        break;
    case 30:
        (void)adm_snprintf(*eventDesc, EVNET_DESC_LEN, "[%s][%s](%s%d)", 
                EVENT_MODELE_NAME, 
                __adm_apc_getTimeStr(), 
                "admfotaapp init start eweb server failed, err:",
                errCode);
        break;
    case 31:
        (void)adm_snprintf(*eventDesc, EVNET_DESC_LEN, "[%s][%s](%s)", 
                EVENT_MODELE_NAME, 
                __adm_apc_getTimeStr(), 
                "admfotaapp init ECS(ECU config service) successed.");
        break;
    case 32:
        (void)adm_snprintf(*eventDesc, EVNET_DESC_LEN, "[%s][%s](%s)", 
                EVENT_MODELE_NAME, 
                __adm_apc_getTimeStr(), 
                "admfotaapp init PIS(Package information service) start now.");
        break;
    case 33:
        (void)adm_snprintf(*eventDesc, EVNET_DESC_LEN, "[%s][%s](%s%d)", 
                EVENT_MODELE_NAME, 
                __adm_apc_getTimeStr(), 
                "admfotaapp init PIS(Package information service) failed, err:",
                errCode);
        break;
    case 34:
        (void)adm_snprintf(*eventDesc, EVNET_DESC_LEN, "[%s][%s](%s)", 
                EVENT_MODELE_NAME, 
                __adm_apc_getTimeStr(), 
                "admfotaapp init PIS(Package information service) successed.");
        break;
    case 35:
        (void)adm_snprintf(*eventDesc, EVNET_DESC_LEN, "[%s][%s](%s)", 
                EVENT_MODELE_NAME, 
                __adm_apc_getTimeStr(), 
                "admfotaapp init IRS(Information report service) start now.");
        break;
    case 36:
        (void)adm_snprintf(*eventDesc, EVNET_DESC_LEN, "[%s][%s](%s%d)", 
                EVENT_MODELE_NAME, 
                __adm_apc_getTimeStr(), 
                "admfotaapp init IRS(Information report service) failed, err:",
                errCode);
        break;
    case 37:
        (void)adm_snprintf(*eventDesc, EVNET_DESC_LEN, "[%s][%s](%s)", 
                EVENT_MODELE_NAME, 
                __adm_apc_getTimeStr(), 
                "admfotaapp init IRS(Information report service) successed.");
        break;
    case 38:
        (void)adm_snprintf(*eventDesc, EVNET_DESC_LEN, "[%s][%s](%s)", 
                EVENT_MODELE_NAME, 
                __adm_apc_getTimeStr(), 
                "admfotaapp init DPM(Download package manager) start now.");
        break;
    case 39:
        (void)adm_snprintf(*eventDesc, EVNET_DESC_LEN, "[%s][%s](%s%d)", 
                EVENT_MODELE_NAME, 
                __adm_apc_getTimeStr(), 
                "admfotaapp init DPM(Download package manager) failed, err:",
                errCode);
        break;
    case 40:
        (void)adm_snprintf(*eventDesc, EVNET_DESC_LEN, "[%s][%s](%s)", 
                EVENT_MODELE_NAME, 
                __adm_apc_getTimeStr(), 
                "admfotaapp init DPM(Download package manager) successed.");
        break;
    case 41:
        (void)adm_snprintf(*eventDesc, EVNET_DESC_LEN, "[%s][%s](%s)", 
                EVENT_MODELE_NAME, 
                __adm_apc_getTimeStr(), 
                "admfotaapp init TPM(Transfer package manager) start now.");
        break;
    case 42:
        (void)adm_snprintf(*eventDesc, EVNET_DESC_LEN, "[%s][%s](%s%d)", 
                EVENT_MODELE_NAME, 
                __adm_apc_getTimeStr(), 
                "admfotaapp init TPM(Transfer package manager) failed, err:",
                errCode);
        break;
    case 43:
        (void)adm_snprintf(*eventDesc, EVNET_DESC_LEN, "[%s][%s](%s)", 
                EVENT_MODELE_NAME, 
                __adm_apc_getTimeStr(), 
                "admfotaapp init TPM(Transfer package manager) successed.");
        break;
    case 44:
        (void)adm_snprintf(*eventDesc, EVNET_DESC_LEN, "[%s][%s](%s)", 
                EVENT_MODELE_NAME, 
                __adm_apc_getTimeStr(), 
                "admfotaapp init IPM(Install package manager) start now.");
        break;
    case 45:
        (void)adm_snprintf(*eventDesc, EVNET_DESC_LEN, "[%s][%s](%s%d)", 
                EVENT_MODELE_NAME, 
                __adm_apc_getTimeStr(), 
                "admfotaapp init IPM(Install package manager) failed, err:",
                errCode);
        break;
    case 46:
        (void)adm_snprintf(*eventDesc, EVNET_DESC_LEN, "[%s][%s](%s)", 
                EVENT_MODELE_NAME, 
                __adm_apc_getTimeStr(), 
                "admfotaapp init IPM(Install package manager) successed.");
        break;
    case 47:
        (void)adm_snprintf(*eventDesc, EVNET_DESC_LEN, "[%s][%s](%s)", 
                EVENT_MODELE_NAME, 
                __adm_apc_getTimeStr(), 
                "admfotaapp init VSM(Vehicle state manager) start now.");
        break;
    case 48:
        (void)adm_snprintf(*eventDesc, EVNET_DESC_LEN, "[%s][%s](%s%d)", 
                EVENT_MODELE_NAME, 
                __adm_apc_getTimeStr(), 
                "admfotaapp init VSM(Vehicle state manager) failed, err:",
                errCode);
        break;
    case 49:
        (void)adm_snprintf(*eventDesc, EVNET_DESC_LEN, "[%s][%s](%s)", 
                EVENT_MODELE_NAME, 
                __adm_apc_getTimeStr(), 
                "admfotaapp init VSM(Vehicle state manager) successed.");
        break;
    case 50:
        (void)adm_snprintf(*eventDesc, EVNET_DESC_LEN, "[%s][%s](%s)", 
                EVENT_MODELE_NAME, 
                __adm_apc_getTimeStr(), 
                "admfotaapp init OCM(OTA-Client manager) start now.");
        break;
    case 51:
        (void)adm_snprintf(*eventDesc, EVNET_DESC_LEN, "[%s][%s](%s%d)", 
                EVENT_MODELE_NAME, 
                __adm_apc_getTimeStr(), 
                "admfotaapp init OCM(OTA-Client manager) failed, err:",
                errCode);
        break;
    case 52:
        (void)adm_snprintf(*eventDesc, EVNET_DESC_LEN, "[%s][%s](%s)", 
                EVENT_MODELE_NAME, 
                __adm_apc_getTimeStr(), 
                "admfotaapp init OCM(OTA-Client manager) successed.");
        break;
    case 53:
        //TODO: 从状态机中获取状态值:
        (void)adm_snprintf(*eventDesc, EVNET_DESC_LEN, "[%s][%s](%s)", 
                EVENT_MODELE_NAME, 
                __adm_apc_getTimeStr(), 
                "admfotaapp init check if it is in the installing state, state ");
        // (void)adm_snprintf(eventDesc, EVNET_DESC_LEN, "[%s][%s](%s%s)", 
        //         EVENT_MODELE_NAME, 
        //         __adm_apc_getTimeStr(), 
        //         "admfotaapp init check if it is in the installing state, state = ",
        //         errCode);
        break;
    case 54:
        (void)adm_snprintf(*eventDesc, EVNET_DESC_LEN, "[%s][%s](%s)", 
                EVENT_MODELE_NAME, 
                __adm_apc_getTimeStr(), 
                "admfotaapp resume tasks according to the state machine.");
        break;
    case 55:
        //TODO: 从配置文件中获取等待时间
        (void)adm_snprintf(*eventDesc, EVNET_DESC_LEN, "[%s][%s](%s)", 
                EVENT_MODELE_NAME, 
                __adm_apc_getTimeStr(), 
                "admfotaapp wait [60] seconds for network ok.");
        // (void)adm_snprintf(*eventDesc, EVNET_DESC_LEN, "[%s][%s](%s)", 
        //         EVENT_MODELE_NAME, 
        //         __adm_apc_getTimeStr(), 
        //         "admfotaapp wait [%d] seconds for network ok.");
        break;
    case 56:
        (void)adm_snprintf(*eventDesc, EVNET_DESC_LEN, "[%s][%s](%s)", 
                EVENT_MODELE_NAME, 
                __adm_apc_getTimeStr(), 
                "admfotaapp do OTA first blood now.");
        break;
    case 57:
        (void)adm_snprintf(*eventDesc, EVNET_DESC_LEN, "[%s][%s](%s%d)", 
                EVENT_MODELE_NAME, 
                __adm_apc_getTimeStr(), 
                "admfotaapp do OTA first blood failed, err:",
                errCode);
        break;
    case 58:
        (void)adm_snprintf(*eventDesc, EVNET_DESC_LEN, "[%s][%s](%s)", 
                EVENT_MODELE_NAME, 
                __adm_apc_getTimeStr(), 
                "admfotaapp do OTA first blood successed.");
        break;
    case 59:
        (void)adm_snprintf(*eventDesc, EVNET_DESC_LEN, "[%s][%s](%s)", 
                EVENT_MODELE_NAME, 
                __adm_apc_getTimeStr(), 
                "admfotaapp do register to OTA server now.");
        break;
    case 60:
        (void)adm_snprintf(*eventDesc, EVNET_DESC_LEN, "[%s][%s](%s%d)", 
                EVENT_MODELE_NAME, 
                __adm_apc_getTimeStr(), 
                "admfotaapp do register to OTA server failed err:",
                errCode);
        break;
    case 61:
        (void)adm_snprintf(*eventDesc, EVNET_DESC_LEN, "[%s][%s](%s)", 
                EVENT_MODELE_NAME, 
                __adm_apc_getTimeStr(), 
                "admfotaapp do register to OTA server successed.");
        break;
    case 62:
        (void)adm_snprintf(*eventDesc, EVNET_DESC_LEN, "[%s][%s](%s)", 
                EVENT_MODELE_NAME, 
                __adm_apc_getTimeStr(), 
                "admfotaapp get config from OTA server now.");
        break;
    case 63:
        (void)adm_snprintf(*eventDesc, EVNET_DESC_LEN, "[%s][%s](%s%d)", 
                EVENT_MODELE_NAME, 
                __adm_apc_getTimeStr(), 
                "admfotaapp get config from OTA server failed times, err:",
                errCode);
        break;
    case 64:
        (void)adm_snprintf(*eventDesc, EVNET_DESC_LEN, "[%s][%s](%s)", 
                EVENT_MODELE_NAME, 
                __adm_apc_getTimeStr(), 
                "admfotaapp get config from OTA server successed.");
        break;
    case 65:
        (void)adm_snprintf(*eventDesc, EVNET_DESC_LEN, "[%s][%s](%s)", 
                EVENT_MODELE_NAME, 
                __adm_apc_getTimeStr(), 
                "admfotaapp do period report now.");
        break;
    case 66:
        (void)adm_snprintf(*eventDesc, EVNET_DESC_LEN, "[%s][%s](%s%d)", 
                EVENT_MODELE_NAME, 
                __adm_apc_getTimeStr(), 
                "admfotaapp do period report failed times, err:",
                errCode);
        break;
    case 67:
        (void)adm_snprintf(*eventDesc, EVNET_DESC_LEN, "[%s][%s](%s)", 
                EVENT_MODELE_NAME, 
                __adm_apc_getTimeStr(), 
                "admfotaapp do period report successed.");
        break;
    case 68:
        (void)adm_snprintf(*eventDesc, EVNET_DESC_LEN, "[%s][%s](%s)", 
                EVENT_MODELE_NAME, 
                __adm_apc_getTimeStr(), 
                "admfotaapp get ECU Information from vehicle now.");
        break;
    case 69:
        (void)adm_snprintf(*eventDesc, EVNET_DESC_LEN, "[%s][%s](%s%d)", 
                EVENT_MODELE_NAME, 
                __adm_apc_getTimeStr(), 
                "admfotaapp get ECU Information from vehicle failed times, err:",
                errCode);
        break;
    case 70:
        (void)adm_snprintf(*eventDesc, EVNET_DESC_LEN, "[%s][%s](%s)", 
                EVENT_MODELE_NAME, 
                __adm_apc_getTimeStr(), 
                "admfotaapp get ECU Information from vehicle successed.");
        break;
    default:
        log_i("Stage of event end serNumber:%d is not defined, please go to define this event, thx", serNumber);
        break;
    }

    return;
}

/**
 * @brief 根据序号生成结束阶段的eventDesc
 * 
 * @param serNumber     [IN]    事件序号
 * @param errCode       [IN]    事件错误码
 * @param eventDesc     [OUT]   NULL:Failed, None-NULL:eventDest
 */
static void __adm_apc_generateEventEndDesc(const D32U serNumber, const D32S errCode, DCHAR** eventDesc)
{
    switch (serNumber)
    {
    case 1:
        (void)adm_snprintf(*eventDesc, EVNET_DESC_LEN, "[%s][%s](%s)", 
                EVENT_MODELE_NAME, 
                __adm_apc_getTimeStr(), 
                "admfotaapp deinit all managers.");
        break;
    case 2:
        (void)adm_snprintf(*eventDesc, EVNET_DESC_LEN, "[%s][%s](%s)", 
                EVENT_MODELE_NAME, 
                __adm_apc_getTimeStr(), 
                "admfotaapp deinit stop ewebserver.");
        break;
    case 3:
        (void)adm_snprintf(*eventDesc, EVNET_DESC_LEN, "[%s][%s](%s)", 
                EVENT_MODELE_NAME, 
                __adm_apc_getTimeStr(), 
                "admfotaapp deinit VSM.");
        break;
    case 4:
        (void)adm_snprintf(*eventDesc, EVNET_DESC_LEN, "[%s][%s](%s)", 
                EVENT_MODELE_NAME, 
                __adm_apc_getTimeStr(), 
                "admfotaapp deinit IPM.");
        break;
    case 5:
        (void)adm_snprintf(*eventDesc, EVNET_DESC_LEN, "[%s][%s](%s)", 
                EVENT_MODELE_NAME, 
                __adm_apc_getTimeStr(), 
                "admfotaapp deinit ECS.");
        break;
    case 6:
        (void)adm_snprintf(*eventDesc, EVNET_DESC_LEN, "[%s][%s](%s)", 
                EVENT_MODELE_NAME, 
                __adm_apc_getTimeStr(), 
                "admfotaapp deinit DPM.");
        break;
    case 7:
        (void)adm_snprintf(*eventDesc, EVNET_DESC_LEN, "[%s][%s](%s)", 
                EVENT_MODELE_NAME, 
                __adm_apc_getTimeStr(), 
                "admfotaapp deinit TPM.");
        break;
    case 8:
        (void)adm_snprintf(*eventDesc, EVNET_DESC_LEN, "[%s][%s](%s)", 
                EVENT_MODELE_NAME, 
                __adm_apc_getTimeStr(), 
                "admfotaapp deinit IRS.");
        break;
    case 9:
        (void)adm_snprintf(*eventDesc, EVNET_DESC_LEN, "[%s][%s](%s)", 
                EVENT_MODELE_NAME, 
                __adm_apc_getTimeStr(), 
                "admfotaapp deinit PIS.");
        break;
    case 10:
        (void)adm_snprintf(*eventDesc, EVNET_DESC_LEN, "[%s][%s](%s)", 
                EVENT_MODELE_NAME, 
                __adm_apc_getTimeStr(), 
                "admfotaapp deinit UIM.");
        break;
    case 11:
        (void)adm_snprintf(*eventDesc, EVNET_DESC_LEN, "[%s][%s](%s)", 
                EVENT_MODELE_NAME, 
                __adm_apc_getTimeStr(), 
                "admfotaapp deinit UIM.");
        break;
    case 12:
        (void)adm_snprintf(*eventDesc, EVNET_DESC_LEN, "[%s][%s](%s)", 
                EVENT_MODELE_NAME, 
                __adm_apc_getTimeStr(), 
                "admfotaapp deinit core service.");
        break;
    case 13:
        (void)adm_snprintf(*eventDesc, EVNET_DESC_LEN, "[%s][%s](%s)", 
                EVENT_MODELE_NAME, 
                __adm_apc_getTimeStr(), 
                "admfotaapp deinit tracer tool.");
        break;
    case 14:
        (void)adm_snprintf(*eventDesc, EVNET_DESC_LEN, "[%s][%s](%s)", 
                EVENT_MODELE_NAME, 
                __adm_apc_getTimeStr(), 
                "admfotaapp deinit log.");
        break;
    case 15:
        (void)adm_snprintf(*eventDesc, EVNET_DESC_LEN, "[%s][%s](%s)", 
                EVENT_MODELE_NAME, 
                __adm_apc_getTimeStr(), 
                "admfotaapp deinit ota config.");
        break;
    
    default:
        log_i("Stage of event ing serNumber:%d is not defined, please go to define this event, thx", serNumber);
        break;
    }

    return;
}

/**
 * @brief 通过事件ID，获取事件描述信息
 * 
 * @param eStage        [IN]    事件阶段
 * @param serNum        [IN]    事件序号
 * @param errCode       [IN]    事件错误码
 * @return DCHAR*   NULL:Failed, None-NULL: eventDesc
 */
static DCHAR* __adm_apc_generateEventDesc(const ADM_APC_EV_STAGE_E eStage, const D32U serNumber, const D32S errCode)
{
    DCHAR* eventDesc = NULL;

    (void)eventNodeLock();
    do {
        //!< 申请1K空间，用于存储事件描述信息
        eventDesc = (DCHAR*)adm_malloc(EVNET_DESC_LEN);
        if (NULL == eventDesc) {
            log_e("adm_malloc size:%d  failed", EVNET_DESC_LEN);
            break;
        }
        (void)adm_memset(eventDesc, 0x00, EVNET_DESC_LEN, EVNET_DESC_LEN);

        switch (eStage)
        {
        case ADM_APC_EV_STAGE_PRE:
            __adm_apc_generateEventPreDesc(serNumber, errCode, &eventDesc);
            break;
        case ADM_APC_EV_STAGE_ING:
            __adm_apc_generateEventIngDesc(serNumber, errCode, &eventDesc);
            break;
        case ADM_APC_EV_STAGE_END:
            __adm_apc_generateEventEndDesc(serNumber, errCode, &eventDesc);
            break;
        case ADM_APC_EV_STAGE_INV:
        default:
            log_w("event stage %d invalid", eStage);
            //!< 事件Desc无效，置为NULL
            adm_free(eventDesc);
            eventDesc = NULL;
            break;
        }
    } while(FALSE);
    (void)eventNodeUnlock();


    return eventDesc;
}

static void __adm_apc_doEventReport(const ADM_APC_EV_STAGE_E eStage, const D32U serNumber)
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
        if (repotCount >= ADM_APC_EVENT_REPORT_COUNT) {
            isDoReport = TRUE;
        }

        if (ADM_APC_EV_STAGE_ING == eStage && ((unsigned)ADM_APC_EV_INIT_ING_MAX == serNumber)) {
            isDoReport = TRUE;
        }

        //TODO: ADM_APC_EV_STAGE_END == eStage ???

        if (TRUE == isDoReport) {
            (void)adm_apc_triggerReportEvent();
            isDoReport = FALSE;
            repotCount = 0;
        }
    } while(FALSE);

    return;
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
void adm_apc_eventAdd(const ADM_APC_EV_STAGE_E eStage, const D32U serNumber, const D32S eventLevel, const D32S errCode, const DCHAR* eventContext)
{
    DCHAR* eventId = NULL;
    DCHAR* eventDesc = NULL;

    do {
        //!< 先生成事件ID
        eventId = __adm_apc_generateEventId(eStage, serNumber);
        if (NULL == eventId) {
            log_e("eStage:%d, serNumber:%d generateEventId failed", eStage, serNumber);
            break;
        }

        //!< 再生成事件描述信息
        eventDesc = __adm_apc_generateEventDesc(eStage, serNumber, errCode);
        if (NULL == eventDesc) {
            log_e("eStage:%d, serNumber:%d generateEventDesc failed", eStage, serNumber);
            break;
        }

        if (g_reportLevel <= eventLevel) {
            //!< 将事件放到链表中，只有当前事件的级别 >= 定义需要上报的级别，才需要上报
            (void)__adm_apc_addReportEvent(eventId, eventLevel, eventDesc, eventContext);
        } else {
            //!< 事件不需要上报，只需要在日志中打印即可
            log_evt("eventID:%s, eventLevel:%d, eventReportLevel:%d, eventDesc:%s, eventContext:%s", 
                eventId,
                eventLevel,
                g_reportLevel,
                eventDesc,
                (NULL == eventContext) ? "NULL" : eventContext);
        }

    } while(FALSE);

    //!< 释放内存
    adm_free(eventId);
    adm_free(eventDesc);

    //!< 执行当前上报规则的判断，然后执行上报
    __adm_apc_doEventReport(eStage, serNumber);
    return;
}

/**
 * @brief 获取链表的长度
 * 
 * @param head      [IN]    链表头
 * @return D32U     链表长度
 */
static D32U __adm_apc_getEvNodeListLen(struct list_head head)
{
    D32U len = 0;
    ADM_APC_EV_NODE_INFO_T* pos = NULL;
    ADM_APC_EV_NODE_INFO_T* next = NULL;

    list_for_each_entry_safe(pos, next, &g_head, ADM_APC_EV_NODE_INFO_T, entry) {
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
static void __adm_apc_delEventInfo(ADM_OCS_EVENTS_INFO_T* eventInfo)
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
static ADM_OCS_EVENTS_INFO_LIST_T* __adm_apc_fieldEventsListForReport(void)
{
    D32U len = 0;
    D32S i = 0;
    ADM_APC_EV_NODE_INFO_T* pos = NULL;
    ADM_APC_EV_NODE_INFO_T* next = NULL;
    ADM_OCS_EVENTS_INFO_LIST_T* eventsList = NULL;

    do {
        len = __adm_apc_getEvNodeListLen(g_head);
        
        //!< 申请事件上报列表的空间
        eventsList = (ADM_OCS_EVENTS_INFO_LIST_T*)adm_malloc(sizeof(ADM_OCS_EVENTS_INFO_LIST_T));
        (void)adm_memset(eventsList, 0x00, sizeof(ADM_OCS_EVENTS_INFO_LIST_T), sizeof(ADM_OCS_EVENTS_INFO_LIST_T));
        eventsList->reportEventsLen = (D32S)len;
        eventsList->reportEvents = (ADM_OCS_EVENTS_INFO_T*)adm_malloc(sizeof(ADM_OCS_EVENTS_INFO_T) * (D32U)eventsList->reportEventsLen);
        (void)adm_memset(eventsList->reportEvents, 0x00, sizeof(ADM_OCS_EVENTS_INFO_T) * (D32U)eventsList->reportEventsLen, sizeof(ADM_OCS_EVENTS_INFO_T) * (D32U)eventsList->reportEventsLen);

        list_for_each_entry_safe(pos, next, &g_head, ADM_APC_EV_NODE_INFO_T, entry) {
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
            __adm_apc_delEventInfo(pos->eventInfo);
            adm_free(pos);
        }
    } while(FALSE);

    return eventsList;
}

/**
 * @brief 释放上报的事件列表信息
 * 
 * @param eventsList    [IN]    事件列表信息
 */
static void __adm_apc_delEventsList(ADM_OCS_EVENTS_INFO_LIST_T* eventsList)
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
D32S adm_apc_triggerReportEvent(void)
{
    D32S iRet = ADM_ECODE_FAILED;
    ADM_OCS_EVENTS_INFO_LIST_T* eventsList = NULL;
    D32S i = 0;
    D64S lTimeStamp = time(NULL);
    
    (void)eventNodeLock();
    do {
        //!< 将链表里的内容填充到事件上报的接口中
        eventsList = __adm_apc_fieldEventsListForReport();
        if (NULL == eventsList) {
            log_e("__adm_apc_fieldEventsListForReport failed");
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

        __adm_apc_delEventsList(eventsList);
    } while(FALSE);
    (void)eventNodeUnlock();

    return iRet;
}

/**
 * @brief 初始化程序启动阶段的事件上报模块
 * 
 */
void adm_apc_initReportEvent(ADM_APC_IRM_GET_EV_SEQID_CALLBACK_F getSeqIdCb, 
                                    ADM_APC_IRM_REPORT_EVENT_CALLBACK_F reportEventCb)

{
    //!< 初始化链表和上报接口
    if (FALSE == g_isReportInit) {
        __adm_apc_evNodeListInit();
        g_reportEventCb = reportEventCb;
        g_getSeqIdCb = getSeqIdCb;
        g_isReportInit = TRUE;
        g_reportLevel = adm_cfg_iniParserGetInt(ADM_CFG_VUS_EVENT_REPORT_LEVEL, ADM_CFG_VUS_EVENT_REPORT_LEVEL_DEFAULT_VAL);
    }

    return;
}
