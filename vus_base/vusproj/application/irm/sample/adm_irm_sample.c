
#include "adm_irm_report.h"
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include "adm_debug.h"
#include "adm_memory.h"
#include "adm_string.h"
#include "adm_ocm_interface.h"
#include "adm_com_errorCode.h"

/*TODO：必须异步接口*/
static void adm_irm_cloudRespStatus(const D32S statusCode)
{
    log_i("++++++statusCode = %d++++++++++++++", statusCode);
}

int main(void)
{
    D32S retVal = ADM_ECODE_FAILED;
    D64S timestamp = time(NULL);
    (void)adm_core_service_initRuntimeDir(
        ADM_CORE_SERVICE_RUNTIME_DIR_TYPE_KV_STORAGE, "/home/jibijun/project/tmp/vus/kvstorage/");
    (void)adm_core_service_init();
    adm_debug_logInit("/home/jibijun/project/log", 100, 5);

    /*云端返回异常状态处理*/
    (void)adm_ocs_init(adm_irm_cloudRespStatus);

    /*初始化IRM*/
    retVal = adm_irm_init(adm_ocm_businessCommInterface, adm_ocm_fileUploadInterface, NULL);
    if (ADM_ECODE_SUCCESSED == retVal) {
        /*vehicle state*/
        retVal = adm_irm_reportVehicleState(timestamp,
            "1.1.1_a6a46bb_1640172450",
            "1640224772508p14ng53c",
            387,
            1024,
            "Vehicle_Register_Prepare",
            "NA",
            "V1.1.4",
            NULL,
            NULL);
        if (ADM_ECODE_SUCCESSED != retVal) {
            log_e("adm_irm_reportVehicleState return failed");
        }

        /*ecu state*/
        ADM_OCS_ECU_STATE_INFO_LIST_T* ecusList = NULL;
        ecusList = (ADM_OCS_ECU_STATE_INFO_LIST_T*)adm_malloc(sizeof(ADM_OCS_ECU_STATE_INFO_LIST_T));
        ecusList->reportEcusLen = 1;
        ecusList->reportEcus
            = (ADM_OCS_ECU_STATE_INFO_T*)adm_malloc(sizeof(ADM_OCS_ECU_STATE_INFO_T) * (D32U)ecusList->reportEcusLen);

        ecusList->reportEcus[0].beginTime = 0;
        ecusList->reportEcus[0].endTime = 1;
        ecusList->reportEcus[0].ecuReqId = adm_strdup("0X16E2");
        ecusList->reportEcus[0].ecuSwDid = adm_strdup("0XD001");
        ecusList->reportEcus[0].ecuState = adm_strdup("Download_To_Be");
        ecusList->reportEcus[0].ecuReason = adm_strdup("0");

        ecusList->reportEcus[0].ecuDidLists = (ADM_OCS_ECU_DID_LIST_T*)adm_malloc(sizeof(ADM_OCS_ECU_DID_LIST_T));
        ecusList->reportEcus[0].ecuDidLists->ecuDidsLen = 2;
        ecusList->reportEcus[0].ecuDidLists->ecuDids = (ADM_OCS_ECU_DID_T*)adm_malloc(
            sizeof(ADM_OCS_ECU_DID_T) * (D32U)ecusList->reportEcus[0].ecuDidLists->ecuDidsLen);

        ecusList->reportEcus[0].ecuDidLists->ecuDids[0].did = adm_strdup("0XF187");
        ecusList->reportEcus[0].ecuDidLists->ecuDids[0].didValue = adm_strdup("0X01");
        ecusList->reportEcus[0].ecuDidLists->ecuDids[0].start = 0;
        ecusList->reportEcus[0].ecuDidLists->ecuDids[0].end = 13;

        ecusList->reportEcus[0].ecuDidLists->ecuDids[1].did = adm_strdup("0XF188");
        ecusList->reportEcus[0].ecuDidLists->ecuDids[1].didValue = adm_strdup("V1.1.3");
        ecusList->reportEcus[0].ecuDidLists->ecuDids[1].start = 0;
        ecusList->reportEcus[0].ecuDidLists->ecuDids[1].end = 13;
        retVal = adm_irm_reportEcuState(
            "1640224772508p14ng53c", 387, 1024, timestamp, "1.1.1_a6a46bb_1640172450", NULL, ecusList);
        if (ADM_ECODE_SUCCESSED != retVal) {
            log_e("adm_irm_reportEcuState return failed");
        }
        /*progress*/
        ADM_OCS_ECU_PROGRESS_INFO_LIST_T* ecusProgressList = NULL;
        ecusProgressList = (ADM_OCS_ECU_PROGRESS_INFO_LIST_T*)adm_malloc(sizeof(ADM_OCS_ECU_PROGRESS_INFO_LIST_T));
        ecusProgressList->reportEcusLen = 1;
        ecusProgressList->reportEcus = (ADM_OCS_ECU_PROGRESS_INFO_T*)adm_malloc(
            sizeof(ADM_OCS_ECU_PROGRESS_INFO_T) * (D32U)ecusProgressList->reportEcusLen);

        ecusProgressList->reportEcus[0].ecuReqId = adm_strdup("0X16E2");
        ecusProgressList->reportEcus[0].ecuSwDid = adm_strdup("0XD001");
        ecusProgressList->reportEcus[0].progressType = adm_strdup("1");
        ecusProgressList->reportEcus[0].progress = 80;
        retVal = adm_irm_reportVehicleProgress(
            "1640224772508p14ng53c", 387, 1024, timestamp, "1.1.1_a6a46bb_1640172450", NULL, ecusProgressList);
        if (ADM_ECODE_SUCCESSED != retVal) {
            log_e("adm_irm_reportVehicleProgress return failed");
        }
        /*events*/
        ADM_OCS_EVENTS_INFO_LIST_T* eventsList = NULL;
        eventsList = (ADM_OCS_EVENTS_INFO_LIST_T*)adm_malloc(sizeof(ADM_OCS_EVENTS_INFO_LIST_T));
        eventsList->reportEventsLen = 1;
        eventsList->reportEvents
            = (ADM_OCS_EVENTS_INFO_T*)adm_malloc(sizeof(ADM_OCS_EVENTS_INFO_T) * (D32U)eventsList->reportEventsLen);

        eventsList->reportEvents[0].eventContext = adm_strdup("eventContext");
        eventsList->reportEvents[0].eventSequenceId = adm_irm_getEventSequenceId();
        eventsList->reportEvents[0].eventId = adm_strdup("0x05010001");
        eventsList->reportEvents[0].eventLevel = 1;
        eventsList->reportEvents[0].eventDesc = adm_strdup("eventDesc");
        eventsList->reportEvents[0].eventTime = timestamp;

        retVal = adm_irm_reportEvent(
            "1640224772508p14ng53c", 387, 1024, timestamp, "1.1.1_a6a46bb_1640172450", NULL, eventsList);
        if (ADM_ECODE_SUCCESSED != retVal) {
            log_e("adm_irm_reportEvent return failed");
        }
        const DCHAR* exts
            = "{\
        \"status\": 1000,\
        \"msg\": \"exts\",\
        \"sign\": \"89f70c65fccf5b083738b62d163201be9137ee41679ebebab9325c6e131fb233\",\
        \"timestamp\": 1626056371,\
        \"data\": {\
            \"deviceId\": \"e0984588053b2873c6c5aebdac6840bd8b66a83ac83db0911846dfe13d9e39c9\",\
            \"secret\": \"0d5fdcf89147040a5cbc5257c2b89529\"\
        }\
    }";
        (void)adm_irm_reportFile("sessionId",
            0,
            0,
            0,
            0,
            0,
            "v2.22",
            "/home/jibijun/project/vusproj/build/application/irm/sample/123.zip",
            ADM_OCS_FILE_ENCRYPTED,
            "4856544",
            4088,
            ADM_OCS_UCLOG_FILE,
            ADM_OCS_CMDID_TSP,
            "pushId",
            exts);
        (void)sleep(1);
        (void)adm_irm_reportFile(NULL,
            0,
            0,
            0,
            0,
            0,
            "v2.22",
            "/home/jibijun/project/vusproj/build/application/irm/sample/456.zip",
            ADM_OCS_FILE_ENCRYPTED,
            "4856544",
            4088,
            ADM_OCS_UCLOG_FILE,
            ADM_OCS_CMDID_TSP,
            "pushId",
            exts);
    }

    while (TRUE) {
        (void)sleep(3);
    }

}
