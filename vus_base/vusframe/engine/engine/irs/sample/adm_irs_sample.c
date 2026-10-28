#include "adm_irs_upload.h"
#include <stdio.h>
#include <unistd.h>
#include "adm_debug.h"
#include "adm_memory.h"
#include "adm_string.h"

D32S post(const ADM_OCS_BUSINESS_TYPE_E eBusinessType,
    const DCHAR* reqMessageBody,
    DCHAR** rspMessageBody,
    DSIZE* rspStatusCode)
{
    printf("eBusinessType = %d, reqMessageBody = %s\n", eBusinessType, reqMessageBody);
    return 0;
}

D32S postFile(ADM_OCA_FILE_UPLOAD_INFO_T* uploadInfo, DCHAR** rspMessageBody, ADM_OCS_UPLOADFILE_T* priv)
{
    printf("cmdId = %d, fileHash = %s\n", priv->cmdId, priv->fileHash);
    printf("fileSize = %lld, fileType = %d\n", priv->fileSize, priv->fileType);
    printf("fullFileNamePath = %s, isFileEncrypted = %d\n", priv->fullFileNamePath, priv->isFileEncrypted);
    printf("sdkVersion = %s, timestamp = %lld\n", priv->sdkVersion, priv->timestamp);
    printf("sign = %s\n", priv->sign);
    printf("extsReqJsonStr = %s\n", priv->extsReqJsonStr);
    return 0;
}

void fileProgress(ADM_OCA_FILE_PROGRESS_T* progress, void* priv)
{}

D32S adm_apc_logInit(void)
{
    adm_debug_logInit("/home/jibijun/project/log", 100, 5);

    return 0;
}

int main()
{
    adm_core_service_initRuntimeDir(
        ADM_CORE_SERVICE_RUNTIME_DIR_TYPE_KV_STORAGE, "/home/jibijun/project/tmp/vus/kvstorage/");
    adm_core_service_init();
    adm_apc_logInit();
    ADM_OCA_FILE_UPLOAD_INFO_T uploadInfo;
    uploadInfo.priv = NULL;
    uploadInfo.cbUploadProgress = fileProgress;
    ADM_IRS_SESSION_T* session = adm_irs_new(post, postFile, &uploadInfo,3);

    // /*vehicle state*/
    // DCHAR* generateMessageBody = NULL;
    // adm_ocs_vehicleStateProtocolAdapter(
    //     123456, "v1.2", "456", 0, 0, 0, "Vehicle_Register_Prepare", "NA", "v3.12", &generateMessageBody);

    // printf("generateMessageBody= %s\n", generateMessageBody);
    // adm_irs_addUploadMessage(session, ADM_IRS_UPLOAD_VEHICLE_STATE, generateMessageBody);
    // adm_free(generateMessageBody);

    // /*ecu state*/
    // ADM_OCS_ECU_STATE_INFO_LIST_T* ecusList = NULL;
    // ecusList = (ADM_OCS_ECU_STATE_INFO_LIST_T*)adm_malloc(sizeof(ADM_OCS_ECU_STATE_INFO_LIST_T));
    // ecusList->reportEcusLen = 2;
    // ecusList->reportEcus
    //     = (ADM_OCS_ECU_STATE_INFO_T*)adm_malloc(sizeof(ADM_OCS_ECU_STATE_INFO_T) * ecusList->reportEcusLen);

    // ecusList->reportEcus[0].beginTime = 0;
    // ecusList->reportEcus[0].endTime = 1;
    // ecusList->reportEcus[0].ecuReqId = adm_strdup("ecuReqId");
    // ecusList->reportEcus[0].ecuSwDid = adm_strdup("ecuSwDid");
    // ecusList->reportEcus[0].ecuState = adm_strdup("ecuState");
    // ecusList->reportEcus[0].ecuReason = adm_strdup("ecuReason");

    // ecusList->reportEcus[0].ecuDidLists = (ADM_OCS_ECU_DID_LIST_T*)adm_malloc(sizeof(ADM_OCS_ECU_DID_LIST_T));
    // ecusList->reportEcus[0].ecuDidLists->ecuDidsLen = 1;
    // ecusList->reportEcus[0].ecuDidLists->ecuDids
    //     = (ADM_OCS_ECU_DID_T*)adm_malloc(sizeof(ADM_OCS_ECU_DID_T) *
    //     ecusList->reportEcus[0].ecuDidLists->ecuDidsLen);

    // ecusList->reportEcus[0].ecuDidLists->ecuDids[0].did = adm_strdup("0xF188");
    // ecusList->reportEcus[0].ecuDidLists->ecuDids[0].didValue = adm_strdup("didValue");
    // ecusList->reportEcus[0].ecuDidLists->ecuDids[0].start = 0;
    // ecusList->reportEcus[0].ecuDidLists->ecuDids[0].end = 13;

    // ecusList->reportEcus[1].beginTime = 0;
    // ecusList->reportEcus[1].endTime = 1;
    // ecusList->reportEcus[1].ecuReqId = adm_strdup("ecuReqId");
    // ecusList->reportEcus[1].ecuSwDid = adm_strdup("ecuSwDid");
    // ecusList->reportEcus[1].ecuState = adm_strdup("ecuState");
    // ecusList->reportEcus[1].ecuReason = adm_strdup("ecuReason");

    // ecusList->reportEcus[1].ecuDidLists = (ADM_OCS_ECU_DID_LIST_T*)adm_malloc(sizeof(ADM_OCS_ECU_DID_LIST_T));
    // ecusList->reportEcus[1].ecuDidLists->ecuDidsLen = 1;
    // ecusList->reportEcus[1].ecuDidLists->ecuDids
    //     = (ADM_OCS_ECU_DID_T*)adm_malloc(sizeof(ADM_OCS_ECU_DID_T) *
    //     ecusList->reportEcus[0].ecuDidLists->ecuDidsLen);

    // ecusList->reportEcus[1].ecuDidLists->ecuDids[0].did = adm_strdup("0xF188");
    // ecusList->reportEcus[1].ecuDidLists->ecuDids[0].didValue = adm_strdup("didValue");
    // ecusList->reportEcus[1].ecuDidLists->ecuDids[0].start = 0;
    // ecusList->reportEcus[1].ecuDidLists->ecuDids[0].end = 13;

    // adm_ocs_ecuStateProtocolAdapter("456", 0, 0, 123456, "v1.2", 2, ecusList, &generateMessageBody);
    // printf("generateMessageBody= %s\n", generateMessageBody);
    // adm_irs_addUploadMessage(session, ADM_IRS_UPLOAD_ECU_STATE, generateMessageBody);
    // adm_free(generateMessageBody);

    // /*progress*/
    // ADM_OCS_ECU_PROGRESS_INFO_LIST_T* ecusProgressList = NULL;
    // ecusProgressList = (ADM_OCS_ECU_PROGRESS_INFO_LIST_T*)adm_malloc(sizeof(ADM_OCS_ECU_PROGRESS_INFO_LIST_T));
    // ecusProgressList->reportEcusLen = 2;
    // ecusProgressList->reportEcus = (ADM_OCS_ECU_PROGRESS_INFO_T*)adm_malloc(
    //     sizeof(ADM_OCS_ECU_PROGRESS_INFO_T) * ecusProgressList->reportEcusLen);

    // ecusProgressList->reportEcus[0].ecuReqId = adm_strdup("ecuReqId");
    // ecusProgressList->reportEcus[0].ecuSwDid = adm_strdup("ecuSwDid");
    // ecusProgressList->reportEcus[0].progressType = adm_strdup("1");
    // ecusProgressList->reportEcus[0].progress = 80;

    // ecusProgressList->reportEcus[0].ecuDidLists =
    // (ADM_OCS_ECU_DID_LIST_T*)adm_malloc(sizeof(ADM_OCS_ECU_DID_LIST_T));
    // ecusProgressList->reportEcus[0].ecuDidLists->ecuDidsLen = 1;
    // ecusProgressList->reportEcus[0].ecuDidLists->ecuDids = (ADM_OCS_ECU_DID_T*)adm_malloc(
    //     sizeof(ADM_OCS_ECU_DID_T) * ecusProgressList->reportEcus[0].ecuDidLists->ecuDidsLen);

    // ecusProgressList->reportEcus[0].ecuDidLists->ecuDids[0].did = adm_strdup("0xF188");
    // ecusProgressList->reportEcus[0].ecuDidLists->ecuDids[0].didValue = adm_strdup("didValue");
    // ecusProgressList->reportEcus[0].ecuDidLists->ecuDids[0].start = 0;
    // ecusProgressList->reportEcus[0].ecuDidLists->ecuDids[0].end = 13;

    // ecusProgressList->reportEcus[1].ecuReqId = adm_strdup("ecuReqId");
    // ecusProgressList->reportEcus[1].ecuSwDid = adm_strdup("ecuSwDid");
    // ecusProgressList->reportEcus[1].progressType = adm_strdup("1");
    // ecusProgressList->reportEcus[1].progress = 90;

    // ecusProgressList->reportEcus[1].ecuDidLists =
    // (ADM_OCS_ECU_DID_LIST_T*)adm_malloc(sizeof(ADM_OCS_ECU_DID_LIST_T));
    // ecusProgressList->reportEcus[1].ecuDidLists->ecuDidsLen = 1;
    // ecusProgressList->reportEcus[1].ecuDidLists->ecuDids = (ADM_OCS_ECU_DID_T*)adm_malloc(
    //     sizeof(ADM_OCS_ECU_DID_T) * ecusProgressList->reportEcus[0].ecuDidLists->ecuDidsLen);

    // ecusProgressList->reportEcus[1].ecuDidLists->ecuDids[0].did = adm_strdup("0xF188");
    // ecusProgressList->reportEcus[1].ecuDidLists->ecuDids[0].didValue = adm_strdup("didValue");
    // ecusProgressList->reportEcus[1].ecuDidLists->ecuDids[0].start = 0;
    // ecusProgressList->reportEcus[1].ecuDidLists->ecuDids[0].end = 13;

    // adm_ocs_vehicleProgressProtocolAdapter("456", 0, 0, 123456, "v1.2", 3, ecusProgressList, &generateMessageBody);
    // adm_irs_addUploadMessage(session, ADM_IRS_UPLOAD_PROGRESS, generateMessageBody);
    // adm_free(generateMessageBody);

    // /*events*/
    // ADM_OCS_EVENTS_INFO_LIST_T* eventsList = NULL;
    // eventsList = (ADM_OCS_EVENTS_INFO_LIST_T*)adm_malloc(sizeof(ADM_OCS_EVENTS_INFO_LIST_T));
    // eventsList->reportEventsLen = 2;
    // eventsList->reportEvents
    //     = (ADM_OCS_EVENTS_INFO_T*)adm_malloc(sizeof(ADM_OCS_EVENTS_INFO_T) * eventsList->reportEventsLen);

    // ADM_OCS_ECU_ACCESS_INFO_T* ecuAccessInfo; /*!< 非必填，没有填NULL Ecu 信息 eventType 为 2 时必填 */
    // DCHAR* eventDesc; /*!< 事件描述 */
    // D64S eventTime; /*!< 事件发生时间时间戳，精确到秒 */

    // eventsList->reportEvents[0].eventType = 1;
    // eventsList->reportEvents[0].eventSequenceId = 2;
    // eventsList->reportEvents[0].eventId = adm_strdup("eventId");
    // eventsList->reportEvents[0].eventLevel = 1;
    // eventsList->reportEvents[0].eventDesc = adm_strdup("eventDesc");
    // eventsList->reportEvents[0].eventTime = 6;

    // eventsList->reportEvents[0].ecuAccessInfo
    //     = (ADM_OCS_ECU_ACCESS_INFO_T*)adm_malloc(sizeof(ADM_OCS_ECU_ACCESS_INFO_T));
    // eventsList->reportEvents[0].ecuAccessInfo->ecuReqId = adm_strdup("ecuReqId");
    // eventsList->reportEvents[0].ecuAccessInfo->ecuSwDid = adm_strdup("ecuSwDid");

    // eventsList->reportEvents[0].ecuAccessInfo->ecuDidLists
    //     = (ADM_OCS_ECU_DID_LIST_T*)adm_malloc(sizeof(ADM_OCS_ECU_DID_LIST_T));
    // eventsList->reportEvents[0].ecuAccessInfo->ecuDidLists->ecuDidsLen = 1;

    // eventsList->reportEvents[0].ecuAccessInfo->ecuDidLists->ecuDids = (ADM_OCS_ECU_DID_T*)adm_malloc(
    //     sizeof(ADM_OCS_ECU_DID_T) * eventsList->reportEvents[0].ecuAccessInfo->ecuDidLists->ecuDidsLen);

    // eventsList->reportEvents[0].ecuAccessInfo->ecuDidLists->ecuDids[0].did = adm_strdup("0xF188");
    // eventsList->reportEvents[0].ecuAccessInfo->ecuDidLists->ecuDids[0].didValue = adm_strdup("didValue");

    // eventsList->reportEvents[0].ecuAccessInfo->ecuDidLists->ecuDids[0].start = 0;
    // eventsList->reportEvents[0].ecuAccessInfo->ecuDidLists->ecuDids[0].end = 13;

    // eventsList->reportEvents[1].eventType = 1;
    // eventsList->reportEvents[1].eventSequenceId = 2;
    // eventsList->reportEvents[1].eventId = adm_strdup("eventId");
    // eventsList->reportEvents[1].eventLevel = 1;
    // eventsList->reportEvents[1].eventDesc = adm_strdup("eventDesc");
    // eventsList->reportEvents[1].eventTime = 6;

    // eventsList->reportEvents[1].ecuAccessInfo
    //     = (ADM_OCS_ECU_ACCESS_INFO_T*)adm_malloc(sizeof(ADM_OCS_ECU_ACCESS_INFO_T));
    // eventsList->reportEvents[1].ecuAccessInfo->ecuReqId = adm_strdup("ecuReqId");
    // eventsList->reportEvents[1].ecuAccessInfo->ecuSwDid = adm_strdup("ecuSwDid");

    // eventsList->reportEvents[1].ecuAccessInfo->ecuDidLists
    //     = (ADM_OCS_ECU_DID_LIST_T*)adm_malloc(sizeof(ADM_OCS_ECU_DID_LIST_T));
    // eventsList->reportEvents[1].ecuAccessInfo->ecuDidLists->ecuDidsLen = 1;

    // eventsList->reportEvents[1].ecuAccessInfo->ecuDidLists->ecuDids = (ADM_OCS_ECU_DID_T*)adm_malloc(
    //     sizeof(ADM_OCS_ECU_DID_T) * eventsList->reportEvents[0].ecuAccessInfo->ecuDidLists->ecuDidsLen);
    // eventsList->reportEvents[1].ecuAccessInfo->ecuDidLists->ecuDids[0].did = adm_strdup("0xF188");
    // eventsList->reportEvents[1].ecuAccessInfo->ecuDidLists->ecuDids[0].didValue = adm_strdup("didValue");
    // eventsList->reportEvents[1].ecuAccessInfo->ecuDidLists->ecuDids[0].start = 0;
    // eventsList->reportEvents[1].ecuAccessInfo->ecuDidLists->ecuDids[0].end = 13;

    // adm_ocs_eventProtocolAdapter("456", 0, 0, 123456, "v1.2", 3, eventsList, &generateMessageBody);
    // adm_irs_addUploadMessage(session, ADM_IRS_UPLOAD_EVENT, generateMessageBody);
    // adm_free(generateMessageBody);

    // sleep(3);
    DCHAR* exts
        = "{\
        \"status\": 1000,\
        \"msg\": \"exts\",\
        \"extsSign\": \"extsSign\",\
        \"timestamp\": 1626056371,\
        \"data\": {\
            \"deviceId\": \"e0984588053b2873c6c5aebdac6840bd8b66a83ac83db0911846dfe13d9e39c9\",\
            \"secret\": \"0d5fdcf89147040a5cbc5257c2b89529\"\
        }\
    }";

    adm_irs_addUploadFile(session,
        "sessionId",
        1,
        2,
        3,
        4,
        5,
        "v2.22",
        "/home/jibijun/project/vusframe/engine/build/engine/irs/sample/123.zip",
        ADM_OCS_FILE_ENCRYPTED,
        "4856544",
        4088,
        ADM_OCS_UCLOG_FILE,
        ADM_OCS_CMDID_TSP,
        "pushId",
        exts);
    (void)sleep(1);
    (void)adm_irs_addUploadFile(session,
        NULL,
        6,
        7,
        8,
        9,
        10,
        "v2.22",
        "/home/jibijun/project/vusframe/engine/build/engine/irs/sample/456.zip",
        ADM_OCS_FILE_ENCRYPTED,
        "48565448",
        4088,
        ADM_OCS_UCLOG_FILE,
        ADM_OCS_CMDID_TSP,
        "pushId",
        exts);

    // sleep(3);
    adm_irs_startPeriodReport(session, 5);
    while (TRUE) {
        (void)sleep(3);
    }
    adm_irs_stopPeriodReport(session);
    adm_irs_delete(session);
    adm_debug_logDeinit();
    adm_core_service_deinit();
    return 0;
}