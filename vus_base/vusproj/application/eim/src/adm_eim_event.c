#include "adm_eim_log.h"
#include "adm_eim_event.h"
#include "adm_irm_report.h"
#include "adm_memory.h"
#include "adm_string.h"
#include "adm_cfg_api.h"
#include "adm_ecs_rsConfig.h"

#include <time.h>

/**
 * @brief 判断是否统计事件
 * @param[in]  pcEventId 事件id
 * @retval    DBOOL
 */
static DBOOL eimIsStatisticsisEvent(const DCHAR *pcEventId)
{
    DBOOL bIsStatisticsisEvent = FALSE;

    // log_d("eventId[%s]", pcEventId);

    /** 判断事件ID中是否包含PS_GET_ */
    if (ADM_IS_NULL(adm_strstr(pcEventId, "PS_GET_")))
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

static void adm_eim_reportEvent(ADM_OCS_EVENTS_INFO_LIST_T* eventsList)
{
    if (NULL == eventsList || 0 == eventsList->reportEventsLen) {
        log_e("eventsList is NULL");
        return;
    }

    D32S retVal = ADM_ECODE_FAILED;
    DCHAR* sessionId = NULL;
    D64S taskId = 0;
    D64S strategyId = 0;
    D64S timestamp = time(NULL);
    DCHAR* sdkVersion = NULL;
    DCHAR* extsReqJsonStr = NULL;

#ifdef ADM_SDK_VERSION
    sdkVersion = adm_strdup(ADM_SDK_VERSION);
#else
    sdkVersion = adm_strdup("v5.0");
#endif
    retVal = adm_irm_reportEvent(sessionId, taskId, strategyId, timestamp, sdkVersion, extsReqJsonStr, eventsList);
    if (ADM_ECODE_SUCCESSED != retVal) {
        log_e("adm_irm_reportEvent, return error[%d]", retVal);
    }
    adm_free(sdkVersion);
    return;
}

static D32S g_configReportEventLevel = 0;

ADM_OCS_EVENTS_INFO_LIST_T* adm_eim_initEventList(D32S eventNum)
{
    ADM_OCS_EVENTS_INFO_LIST_T* eventList = NULL;
    if (eventNum > 0) {
        eventList = (ADM_OCS_EVENTS_INFO_LIST_T*)adm_malloc((D32U)sizeof(ADM_OCS_EVENTS_INFO_LIST_T));
        eventList->reportEvents = (ADM_OCS_EVENTS_INFO_T*)adm_malloc((D32U)(sizeof(ADM_OCS_EVENTS_INFO_T) * (D32U)eventNum));
        eventList->reportEventsLen = 0;
		if (ADM_ECODE_SUCCESSED != adm_ecs_getEventReportLevel(&g_configReportEventLevel))
		{
	        g_configReportEventLevel = adm_cfg_iniParserGetInt(ADM_CFG_VUS_EVENT_REPORT_LEVEL, ADM_CFG_VUS_EVENT_REPORT_LEVEL_DEFAULT_VAL);
		}

        log_i("configReportEventLevel = %d", g_configReportEventLevel);
        log_i("event list init success");
    } else {
        log_e("event list init failed,eventNum is 0");
    }
    return eventList;
}

D32S adm_eim_addEventList(ADM_OCS_EVENTS_INFO_LIST_T* eventsList,
    const DCHAR* eventId,
    D32S eventLevel,
    const DCHAR* eventContext,
    const DCHAR* eventDesc)
{
    if (NULL == eventsList || NULL == eventsList->reportEvents || NULL == eventId || NULL == eventDesc) {
        log_e("input param invalid");
        return ADM_ECODE_INVALID_PARAM;
    }

    //!< 打印日志信息
    log_evt("eventID:%s, eventLevel:%d, eventTime:%lld, eventDesc:%s, eventContext:%s",
            eventId,
            eventLevel,
            (D64S)time(NULL),
            eventDesc,
            (NULL == eventContext) ? "NULL" : eventContext);

    /** 统计事件，只需要打印即可，暂时不进行上报*/
    if (TRUE == eimIsStatisticsisEvent(eventId))
    {
        // log_d(" skip report statisticsis event[%s]", eventId);
        return ADM_ECODE_SUCCESSED;
    }

    if (eventLevel < g_configReportEventLevel) 
    {
        log_w("event level is less than configration report level");
        return ADM_ECODE_SUCCESSED;
    }

    ADM_OCS_EVENTS_INFO_T* tmpReportEvents = eventsList->reportEvents + eventsList->reportEventsLen;
    if (NULL != tmpReportEvents) {
        tmpReportEvents->eventSequenceId = adm_irm_getEventSequenceId();
        tmpReportEvents->eventId = adm_strdup(eventId);
        tmpReportEvents->eventLevel = eventLevel;
        if (NULL != eventContext) {
            tmpReportEvents->eventContext = adm_strdup(eventContext);
        }
        tmpReportEvents->eventDesc = adm_strdup(eventDesc);
        tmpReportEvents->eventTime = (D64S)time(NULL);
        eventsList->reportEventsLen++;
        return ADM_ECODE_SUCCESSED;
    } else {
        log_e("insufficient event memory size");
        return ADM_ECODE_NO_MEMORY;
    }
}

void adm_eim_clearEventList(ADM_OCS_EVENTS_INFO_LIST_T* eventsList)
{
    if (NULL == eventsList) {
        log_e("input param invalid. eventsList is NULL");
        return;
    }

    for (D32S i = 0; i < eventsList->reportEventsLen; ++i) {
        if (NULL != eventsList->reportEvents) {
            adm_free(eventsList->reportEvents[i].eventId);
            if (NULL != eventsList->reportEvents[i].eventContext) {
                adm_free(eventsList->reportEvents[i].eventContext);
            }
            adm_free(eventsList->reportEvents[i].eventDesc);
            (void)adm_memset(
                eventsList->reportEvents + i, 0x00, sizeof(ADM_OCS_EVENTS_INFO_T), sizeof(ADM_OCS_EVENTS_INFO_T));
        }
    }
    eventsList->reportEventsLen = 0;
    return;
}

void adm_eim_deInitEventsList(ADM_OCS_EVENTS_INFO_LIST_T* eventsList)
{
    if (NULL == eventsList) {
        return;
    }

    for (D32S i = 0; i < eventsList->reportEventsLen; ++i) {
        if (NULL != eventsList->reportEvents) {
            adm_free(eventsList->reportEvents[i].eventId);
            if (NULL != eventsList->reportEvents[i].eventContext) {
                adm_free(eventsList->reportEvents[i].eventContext);
            }
            adm_free(eventsList->reportEvents[i].eventDesc);
        }
    }
    adm_free(eventsList->reportEvents);
    adm_free(eventsList);
}

void adm_eim_getEcuInfoReport(ADM_OCS_EVENTS_INFO_LIST_T* eventsList)
{
    if (NULL == eventsList) {
        log_e("input param invalid. eventsList is NULL");
        return;
    }
    adm_eim_reportEvent(eventsList);
    return;
}

DCHAR* adm_eim_getDateAndTimeStr(void)
{
    static DCHAR sTimeStr[20] = {0};
    time_t timep;
    struct tm* p;

    (void)time(&timep);
    p = localtime(&timep);

    (void)adm_snprintf(sTimeStr,
        (D32U)sizeof(sTimeStr),
        "%04d-%02d-%02d %02d:%02d:%02d",
        1900 + p->tm_year,
        p->tm_mon + 1,
        p->tm_mday,
        p->tm_hour,
        p->tm_min,
        p->tm_sec);

    return sTimeStr;
}