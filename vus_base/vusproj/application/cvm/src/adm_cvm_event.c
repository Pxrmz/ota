#include "adm_cvm_event.h"
#include "adm_cvm_log.h"
#include "adm_irm_report.h"

#include <time.h>

typedef struct
{
    DCHAR* eventId; /*!< 事件ID */
    D32S eventLevel; /*!< 事件级别：1:表示TRACE级别 2:表示DEBUG级别 3:表示INFO级别  4:表示WARN级别 5:表示ERROR级别*/ 
    DCHAR* eventContext; /*!< 事件上下文 */
    DCHAR* eventDesc; /*!< 事件描述 */
} ADM_CVM_REPORT_EVENT_INFO_T;

typedef enum ADM_CVM_EVENT_INDEX_
{
    CT_001 = 0,
    CT_002,
    CT_003,
    CT_004,
    CT_005,
    CT_006,
    CT_007,
    CT_008,
    CT_009,
    CT_010 = 9,
    CP_001 = 10,
    CP_002,
    CP_003,
    CP_004,
    CP_005,
    CP_006,
    CP_007,
    CP_008,
    CP_009,
    CP_010 = 19,
    CI_001 = 20,
    CI_002,
    CI_003,
    CI_004,
    CI_005,
    CI_006,
    CI_007,
    CI_008,
    CI_009 = 28,
    S_CE_001 = 29,
    S_CE_002,
    S_CE_003 = 31
} ADM_CVM_EVENT_INDEX_T;

static ADM_CVM_REPORT_EVENT_INFO_T g_eventArray[] = {
    {"CT_001", ADM_OCS_EVENT_LEVEL_TRACE, "HMI triggered check", "HMI triggered check"},
    {"CT_002", ADM_OCS_EVENT_LEVEL_TRACE, "TSP triggered check", "TSP triggered check"},
    {"CT_003", ADM_OCS_EVENT_LEVEL_TRACE, "CYCLE triggered check", "CYCLE triggered check"},
    {"CT_004", ADM_OCS_EVENT_LEVEL_TRACE, "IGN triggered check", "IGN triggered check"},
    {"CT_005", ADM_OCS_EVENT_LEVEL_TRACE, "PHONE triggered check", "PHONE triggered check"},
    {"CT_006", ADM_OCS_EVENT_LEVEL_TRACE, "USB triggered check", "USB triggered check"},
    {"CT_007", ADM_OCS_EVENT_LEVEL_TRACE, "RESCUE triggered check", "RESCUE triggered check"},
    {"CT_008", ADM_OCS_EVENT_LEVEL_TRACE, "PROD LINE triggered check", "PROD LINE triggered check"},
    {"CT_009", ADM_OCS_EVENT_LEVEL_TRACE, "APK SELF UPGRADE triggered check", "APK SELF UPGRADE triggered check"},
    {"CT_010", ADM_OCS_EVENT_LEVEL_TRACE, "FORCE triggered check", "FORCE triggered check"},
    {"CP_001",
        ADM_OCS_EVENT_LEVEL_TRACE,
        "Successfully get registration information",
        "Successfully get registration information"},
    {"CP_002",
        ADM_OCS_EVENT_LEVEL_ERROR,
        "Failed to get registration information",
        "Failed to get registration information"},
    {"CP_003",
        ADM_OCS_EVENT_LEVEL_TRACE,
        "Successfully get ECU configuration information",
        "Successfully get ECU configuration information"},
    {"CP_004",
        ADM_OCS_EVENT_LEVEL_ERROR,
        "Failed to get ECU configuration information",
        "Failed to get ECU configuration information"},
    {"CP_005",
        ADM_OCS_EVENT_LEVEL_TRACE,
        "Successfully get general configuration information",
        "Successfully get general configuration information"},
    {"CP_006",
        ADM_OCS_EVENT_LEVEL_ERROR,
        "Failed to get general configuration information",
        "Failed to get general configuration information"},
    {"CP_007",
        ADM_OCS_EVENT_LEVEL_TRACE,
        "Successfully get ECU DID information",
        "Successfully get ECU DID information"},
    {"CP_008", ADM_OCS_EVENT_LEVEL_ERROR, "Failed to get ECU DID information", "Failed to get ECU DID information"},
    {"CP_009",
        ADM_OCS_EVENT_LEVEL_TRACE,
        "Successfully to get network information",
        "Successfully to get network information"},
    {"CP_010", ADM_OCS_EVENT_LEVEL_ERROR, "Failed to get network information", "Failed to get network information"},
    {"CI_001", ADM_OCS_EVENT_LEVEL_TRACE, "Successful signature check message", "Successful signature check message"},
    {"CI_002", ADM_OCS_EVENT_LEVEL_ERROR, "Failed to signature check message", "Failed to signature check message"},
    {"CI_003",
        ADM_OCS_EVENT_LEVEL_TRACE,
        "Successfully encrypt the check message",
        "Successfully encrypt the check message"},
    {"CI_004", ADM_OCS_EVENT_LEVEL_ERROR, "Failed to encrypt the check message", "Failed to encrypt the check message"},
    {"CI_005", ADM_OCS_EVENT_LEVEL_TRACE, "Submit check request to platform", "Submit check request to platform"},
    {"CI_006", ADM_OCS_EVENT_LEVEL_ERROR, "check response failed", "check response failed"},
    {"CI_007", ADM_OCS_EVENT_LEVEL_TRACE, "check response succeeded", "check response succeeded"},
    {"CI_008", ADM_OCS_EVENT_LEVEL_TRACE, "check information stored succeeded", "check information stored succeeded"},
    {"CI_009", ADM_OCS_EVENT_LEVEL_ERROR, "check information stored failed", "check information stored failed"},
    {"S_CE_001", ADM_OCS_EVENT_LEVEL_INFO, "check successful", "check successful"},
    {"S_CE_002", ADM_OCS_EVENT_LEVEL_INFO, "check no new version", "check no new version"},
    {"S_CE_003", ADM_OCS_EVENT_LEVEL_ERROR, "check failed", "check failed"}};

void adm_cvm_triggerEventReport(ADM_APPLICATION_COMMON_CMDID_TYPE_E cmdId, ADM_CVS_NOTIFY_T* notifyF)
{
    if (ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_USER == cmdId || 
        ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_USB == cmdId) {
        log_i("trigger event.USER");
        (void)adm_cvs_addEvent(notifyF,
            adm_irm_getEventSequenceId(),
            g_eventArray[CT_001].eventId,
            g_eventArray[CT_001].eventLevel,
            g_eventArray[CT_001].eventContext,
            g_eventArray[CT_001].eventDesc,
            time(NULL));
    } else if (ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_IGN == cmdId) {
        log_i("trigger event.IGN");
        (void)adm_cvs_addEvent(notifyF,
            adm_irm_getEventSequenceId(),
            g_eventArray[CT_004].eventId,
            g_eventArray[CT_004].eventLevel,
            g_eventArray[CT_004].eventContext,
            g_eventArray[CT_004].eventDesc,
            time(NULL));
    } else if (ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_TSP == cmdId) {
        log_i("trigger event.TSP");
        (void)adm_cvs_addEvent(notifyF,
            adm_irm_getEventSequenceId(),
            g_eventArray[CT_002].eventId,
            g_eventArray[CT_002].eventLevel,
            g_eventArray[CT_002].eventContext,
            g_eventArray[CT_002].eventDesc,
            time(NULL));
    } else if (ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_PHONE == cmdId) {
        log_i("trigger event.PHONE");
        (void)adm_cvs_addEvent(notifyF,
            adm_irm_getEventSequenceId(),
            g_eventArray[CT_005].eventId,
            g_eventArray[CT_005].eventLevel,
            g_eventArray[CT_005].eventContext,
            g_eventArray[CT_005].eventDesc,
            time(NULL));
    } else if (ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_CYCLE == cmdId) {
        log_i("trigger event.CYCLE");
        (void)adm_cvs_addEvent(notifyF,
            adm_irm_getEventSequenceId(),
            g_eventArray[CT_003].eventId,
            g_eventArray[CT_003].eventLevel,
            g_eventArray[CT_003].eventContext,
            g_eventArray[CT_003].eventDesc,
            time(NULL));
    }else if (ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_USB == cmdId) {
        log_i("trigger event.USB");
        (void)adm_cvs_addEvent(notifyF,
            adm_irm_getEventSequenceId(),
            g_eventArray[CT_006].eventId,
            g_eventArray[CT_006].eventLevel,
            g_eventArray[CT_006].eventContext,
            g_eventArray[CT_006].eventDesc,
            time(NULL));
    }else if (ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_RESCUE == cmdId) {
        log_i("trigger event.RESCUE");
        (void)adm_cvs_addEvent(notifyF,
            adm_irm_getEventSequenceId(),
            g_eventArray[CT_007].eventId,
            g_eventArray[CT_007].eventLevel,
            g_eventArray[CT_007].eventContext,
            g_eventArray[CT_007].eventDesc,
            time(NULL));
    }else if (ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_PROD_LINE == cmdId) {
        log_i("trigger event.PROD_LINE");
        (void)adm_cvs_addEvent(notifyF,
            adm_irm_getEventSequenceId(),
            g_eventArray[CT_008].eventId,
            g_eventArray[CT_008].eventLevel,
            g_eventArray[CT_008].eventContext,
            g_eventArray[CT_008].eventDesc,
            time(NULL));
    }else if (ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_APK_SELF_UPGRADE == cmdId) {
        log_i("trigger event.APK_SELF_UPGRADE");
        (void)adm_cvs_addEvent(notifyF,
            adm_irm_getEventSequenceId(),
            g_eventArray[CT_009].eventId,
            g_eventArray[CT_009].eventLevel,
            g_eventArray[CT_009].eventContext,
            g_eventArray[CT_009].eventDesc,
            time(NULL));
    }else if (ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_FORCE == cmdId) {
        log_i("trigger event.FORCE");
        (void)adm_cvs_addEvent(notifyF,
            adm_irm_getEventSequenceId(),
            g_eventArray[CT_010].eventId,
            g_eventArray[CT_010].eventLevel,
            g_eventArray[CT_010].eventContext,
            g_eventArray[CT_010].eventDesc,
            time(NULL));
    }
     else{
        log_i("cmdId Out of range!!!");
    }
    
    notifyF->eventReportCloud(notifyF->eventLists);
    adm_cvs_clearEvent(notifyF);
    return;
}

void adm_cvm_prepareEventReport(D32S retVal, ADM_CVS_NOTIFY_T* notifyF)
{
    if (ADM_ECODE_SUCCESSED == retVal) {
        log_i("prepare state event.Successfully get registration information");
        (void)adm_cvs_addEvent(notifyF,
            adm_irm_getEventSequenceId(),
            g_eventArray[CP_001].eventId,
            g_eventArray[CP_001].eventLevel,
            g_eventArray[CP_001].eventContext,
            g_eventArray[CP_001].eventDesc,
            time(NULL));
        log_i("prepare state event.Successfully get ECU configuration information");
        (void)adm_cvs_addEvent(notifyF,
            adm_irm_getEventSequenceId(),
            g_eventArray[CP_003].eventId,
            g_eventArray[CP_003].eventLevel,
            g_eventArray[CP_003].eventContext,
            g_eventArray[CP_003].eventDesc,
            time(NULL));
        log_i("prepare state event.Successfully get ECU DID information");
        (void)adm_cvs_addEvent(notifyF,
            adm_irm_getEventSequenceId(),
            g_eventArray[CP_007].eventId,
            g_eventArray[CP_007].eventLevel,
            g_eventArray[CP_007].eventContext,
            g_eventArray[CP_007].eventDesc,
            time(NULL));
    } else if (ADM_ECODE_EIM_GET_ECU_INFO_FAILED == retVal) {
        log_i("prepare state event.Successfully get registration information");
        (void)adm_cvs_addEvent(notifyF,
            adm_irm_getEventSequenceId(),
            g_eventArray[CP_001].eventId,
            g_eventArray[CP_001].eventLevel,
            g_eventArray[CP_001].eventContext,
            g_eventArray[CP_001].eventDesc,
            time(NULL));
        log_i("prepare state event.Successfully get ECU configuration information");
        (void)adm_cvs_addEvent(notifyF,
            adm_irm_getEventSequenceId(),
            g_eventArray[CP_003].eventId,
            g_eventArray[CP_003].eventLevel,
            g_eventArray[CP_003].eventContext,
            g_eventArray[CP_003].eventDesc,
            time(NULL));
        log_i("prepare state event.Failed to get ECU DID information");
        (void)adm_cvs_addEvent(notifyF,
            adm_irm_getEventSequenceId(),
            g_eventArray[CP_008].eventId,
            g_eventArray[CP_008].eventLevel,
            g_eventArray[CP_008].eventContext,
            g_eventArray[CP_008].eventDesc,
            time(NULL));
    } else {
        log_i("retVal Out of range!!!");
    }
    // TODO:get network info failed
    notifyF->eventReportCloud(notifyF->eventLists);
    adm_cvs_clearEvent(notifyF);
    return;
}

void adm_cvm_endEventReport(D32S retVal, ADM_CVS_NOTIFY_T* notifyF)
{
    if (ADM_ECODE_SUCCESSED == retVal) {
        log_i("end state event.check successful");
        (void)adm_cvs_addEvent(notifyF,
            adm_irm_getEventSequenceId(),
            g_eventArray[S_CE_001].eventId,
            g_eventArray[S_CE_001].eventLevel,
            g_eventArray[S_CE_001].eventContext,
            g_eventArray[S_CE_001].eventDesc,
            time(NULL));
    } else if (ADM_ECODE_NO_NEW_VERSION == retVal) {
        log_i("end state event.check no new version");
        (void)adm_cvs_addEvent(notifyF,
            adm_irm_getEventSequenceId(),
            g_eventArray[S_CE_002].eventId,
            g_eventArray[S_CE_002].eventLevel,
            g_eventArray[S_CE_002].eventContext,
            g_eventArray[S_CE_002].eventDesc,
            time(NULL));
    } else {
        log_i("end state event.checkfailed");
        (void)adm_cvs_addEvent(notifyF,
            adm_irm_getEventSequenceId(),
            g_eventArray[S_CE_003].eventId,
            g_eventArray[S_CE_003].eventLevel,
            g_eventArray[S_CE_003].eventContext,
            g_eventArray[S_CE_003].eventDesc,
            time(NULL));
    }
    notifyF->eventReportCloud(notifyF->eventLists);
    adm_cvs_clearEvent(notifyF);
    return;
}