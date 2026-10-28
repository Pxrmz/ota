#include "adm_irm_report.h"
#include "adm_irs_upload.h"
#include "adm_com_errorCode.h"
#include "adm_irm_log.h"
#include "adm_memory.h"
#include "adm_com_kvstorage.h"
#include "adm_string.h"
#include "adm_ecs_rsConfig.h"
#include "adm_cvs_versionPolicy.h"

#include <pthread.h>

static ADM_IRS_SESSION_T* g_irsSession = NULL;
static pthread_mutex_t g_getSequenceIdMutex;
static pthread_mutex_t g_eventReportMutex;

D32S adm_irm_init(ADM_OCS_BUSINESS_COMM_INTERFACE_CALLBACK_F postMessage,
    ADM_FILE_UPLOAD_INTERFACE_F postFile,
    ADM_OCA_FILE_UPLOAD_INFO_T* uploadInfo)
{
    if (NULL == postFile || NULL == postMessage) {
        log_e("param postFile or postMessage is NULL");
        return ADM_ECODE_INVALID_PARAM;
    }

    g_irsSession = adm_irs_new(postMessage, postFile, uploadInfo);
    if (NULL != g_irsSession) {
        (void)pthread_mutex_init(&g_getSequenceIdMutex, NULL);
        (void)pthread_mutex_init(&g_eventReportMutex, NULL);
        
        log_i("adm_irm_init success");
        return ADM_ECODE_SUCCESSED;
    } else {
        log_e("adm_irs_new return failed");
        return ADM_ECODE_NO_MEMORY;
    }
}

void adm_irm_deinit(void)
{
    if (NULL != g_irsSession) {
        adm_irs_delete(g_irsSession);
        g_irsSession = NULL;
    }
    (void)pthread_mutex_destroy(&g_getSequenceIdMutex);
    (void)pthread_mutex_destroy(&g_eventReportMutex);
}

/*#########sequenceId########*/
static const DCHAR* g_sequenceIdKvKey = "[IRM]_[reportSequenceId]";
static const DCHAR* g_eventSequenceIdKvKey = "[IRM]_[reportEventSequenceId]";

static D64S adm_irm_getSequenceIdFromKv(const DCHAR* sequenceIdKvKey)
{
    DCHAR* sequenceIdStr = NULL;
    size_t sequenceIdStrLen = 0;
    ADM_KV_STORAGE_VALUE_TYPE_E eType;
    D64S sequenceId = 0;

    if (TRUE
        == adm_com_kvstorage_get_string(
            adm_core_service_getBusinessDatabaseDb(), sequenceIdKvKey, &sequenceIdStr, &sequenceIdStrLen, &eType)) {
        (void)adm_stringToInt64(sequenceIdStr, &sequenceId);
        log_i("get sequenceId success, sequenceIdStr = %s , sequenceId = %d , value_type = %d",
            sequenceIdStr,
            sequenceId,
            eType);
        adm_free(sequenceIdStr);
    } else {
        log_e("get failed,init sequenceId = 0");
        sequenceId = 0;
    }
    return sequenceId;
}

static DBOOL adm_irm_setSequenceIdToKv(const DCHAR* sequenceIdKvKey, D64S sequenceId)
{
    ADM_KV_STORAGE_VALUE_TYPE_E eType = ADM_KV_STORAGE_VALUE_TYPE_UINT64;
    DCHAR sequenceIdStr[64];

    (void)adm_memset(sequenceIdStr, 0x00, 64, 64);
    (void)adm_snprintf(sequenceIdStr, 64, "%lld", sequenceId);

    if (TRUE
        == adm_com_kvstorage_set_string(
            adm_core_service_getBusinessDatabaseDb(), sequenceIdKvKey, sequenceIdStr, eType)) {
        log_i("set sequenceId success, sequenceIdStr = %s , sequenceId = %lld", sequenceIdStr, sequenceId);
        return TRUE;
    } else {
        log_e("set sequenceId failed");
        return FALSE;
    }
}

static D64S adm_irm_getSequenceId(void)
{
    D64S sequenceId = 0;
    (void)pthread_mutex_lock(&g_getSequenceIdMutex);
    sequenceId = adm_irm_getSequenceIdFromKv(g_sequenceIdKvKey);
    ++sequenceId;
    (void)adm_irm_setSequenceIdToKv(g_sequenceIdKvKey, sequenceId);
    (void)pthread_mutex_unlock(&g_getSequenceIdMutex);
    return sequenceId;
}

// 在adm_irm_reportEvent 统一生成
D64S adm_irm_getEventSequenceId(void)
{
    // D64S sequenceId = 0;
    // (void)pthread_mutex_lock(&g_getSequenceIdMutex);
    // sequenceId = adm_irm_getSequenceIdFromKv(g_eventSequenceIdKvKey);
    // ++sequenceId;
    // (void)adm_irm_setSequenceIdToKv(g_eventSequenceIdKvKey, sequenceId);
    // (void)pthread_mutex_unlock(&g_getSequenceIdMutex);
    return 1;
}

static D64S adm_irm_getEventSequenceId2(void)
{
    D64S sequenceId = 0;
    (void)pthread_mutex_lock(&g_getSequenceIdMutex);
    sequenceId = adm_irm_getSequenceIdFromKv(g_eventSequenceIdKvKey);
    ++sequenceId;
    (void)adm_irm_setSequenceIdToKv(g_eventSequenceIdKvKey, sequenceId);
    (void)pthread_mutex_unlock(&g_getSequenceIdMutex);
    return sequenceId;
}

/*#########sequenceId########*/

D32S adm_irm_reportVehicleState(D64S timestamp,
    const DCHAR* sdkVersion,
    const DCHAR* sessionId,
    D64S taskId,
    D64S strategyId,
    const DCHAR* state,
    const DCHAR* reason,
    const DCHAR* versionNumber,
    const DCHAR* vehicleVersion,
    const DCHAR* extsReqJsonStr)
{
    if (NULL == g_irsSession) {
        log_e("no adm_irm_init");
        return ADM_ECODE_INVALID_PARAM;
    }
    if (NULL == sdkVersion || NULL == state || NULL == reason) {
        log_e("input param invalid");
        return ADM_ECODE_INVALID_PARAM;
    }

    D32S retVal = ADM_ECODE_FAILED;
    DCHAR* generateMessageBody = NULL;
    D64S sequenceId = adm_irm_getSequenceId();

    do {
        retVal = adm_ocs_vehicleStateProtocolAdapter(timestamp,
            sdkVersion,
            sessionId,
            taskId,
            strategyId,
            sequenceId,
            state,
            reason,
            versionNumber,
            vehicleVersion,
            extsReqJsonStr,
            &generateMessageBody);

        if (ADM_ECODE_SUCCESSED != retVal) {
            log_e("adm_ocs_vehicleStateProtocolAdapter error[%d]", retVal);
            break;
        }

        retVal = adm_irs_addUploadMessage(g_irsSession, ADM_IRS_UPLOAD_VEHICLE_STATE, generateMessageBody);
        if (ADM_ECODE_SUCCESSED != retVal) {
            log_e("adm_irs_addUploadMessage error[%d]", retVal);
            break;
        }
    } while (FALSE);
    adm_free(generateMessageBody);
    return retVal;
}

D32S adm_irm_syncReportVehicleState(D64S timestamp,
    const DCHAR* sdkVersion,
    const DCHAR* sessionId,
    D64S taskId,
    D64S strategyId,
    const DCHAR* state,
    const DCHAR* reason,
    const DCHAR* versionNumber,
    const DCHAR* extsReqJsonStr)
{
    if (NULL == g_irsSession) {
        log_e("no adm_irm_init");
        return ADM_ECODE_INVALID_PARAM;
    }
    if (NULL == sdkVersion || NULL == state || NULL == reason) {
        log_e("input param invalid");
        return ADM_ECODE_INVALID_PARAM;
    }

    D32S retVal = ADM_ECODE_FAILED;
    DCHAR* generateMessageBody = NULL;
    D64S sequenceId = adm_irm_getSequenceId();

    do {
        retVal = adm_ocs_vehicleStateProtocolAdapter(timestamp,
            sdkVersion,
            sessionId,
            taskId,
            strategyId,
            sequenceId,
            state,
            reason,
            versionNumber,
            NULL,
            extsReqJsonStr,
            &generateMessageBody);

        if (ADM_ECODE_SUCCESSED != retVal) {
            log_e("adm_ocs_vehicleStateProtocolAdapter error[%d]", retVal);
            break;
        }

        retVal = adm_irs_syncReportMessage(g_irsSession, ADM_IRS_UPLOAD_VEHICLE_STATE, generateMessageBody);
        if (ADM_ECODE_SUCCESSED != retVal) {
            log_e("adm_irs_syncReportMessage error[%d]", retVal);
            break;
        }
    } while (FALSE);
    adm_free(generateMessageBody);
    return retVal;
}

D32S adm_irm_reportEcuState(const DCHAR* sessionId,
    D64S taskId,
    D64S strategyId,
    D64S timestamp,
    const DCHAR* sdkVersion,
    const DCHAR* extsReqJsonStr,
    ADM_OCS_ECU_STATE_INFO_LIST_T* ecusList)
{
    if (NULL == g_irsSession) {
        log_e("no adm_irm_init");
        return ADM_ECODE_INVALID_PARAM;
    }
    if (0 == taskId || 0 == strategyId || NULL == sdkVersion || NULL == ecusList) {
        log_e("input param invalid");
        return ADM_ECODE_INVALID_PARAM;
    }
    D32S retVal = ADM_ECODE_FAILED;
    DCHAR* generateMessageBody = NULL;
    D64S sequenceId = adm_irm_getSequenceId();

    do {
        retVal = adm_ocs_ecuStateProtocolAdapter(sessionId,
            taskId,
            strategyId,
            timestamp,
            sdkVersion,
            sequenceId,
            extsReqJsonStr,
            ecusList,
            &generateMessageBody);
        if (ADM_ECODE_SUCCESSED != retVal) {
            log_e("adm_ocs_ecuStateProtocolAdapter error[%d]", retVal);
            break;
        }
        retVal = adm_irs_addUploadMessage(g_irsSession, ADM_IRS_UPLOAD_ECU_STATE, generateMessageBody);
        if (ADM_ECODE_SUCCESSED != retVal) {
            log_e("adm_irs_addUploadMessage error[%d]", retVal);
            break;
        }
    } while (FALSE);
    adm_free(generateMessageBody);
    return retVal;
}

D32S adm_irm_reportVehicleProgress(const DCHAR* sessionId,
    D64S taskId,
    D64S strategyId,
    D64S timestamp,
    const DCHAR* sdkVersion,
    const DCHAR* extsReqJsonStr,
    ADM_OCS_ECU_PROGRESS_INFO_LIST_T* ecusProgressList)
{
    if (NULL == g_irsSession) {
        log_e("no adm_irm_init");
        return ADM_ECODE_INVALID_PARAM;
    }
    if (0 == taskId || 0 == strategyId || NULL == sdkVersion || NULL == ecusProgressList) {
        log_e("input param invalid");
        return ADM_ECODE_INVALID_PARAM;
    }

    D32S retVal = ADM_ECODE_FAILED;
    DCHAR* generateMessageBody = NULL;
    D64S sequenceId = adm_irm_getSequenceId();

    do {
        retVal = adm_ocs_vehicleProgressProtocolAdapter(sessionId,
            taskId,
            strategyId,
            timestamp,
            sdkVersion,
            sequenceId,
            extsReqJsonStr,
            ecusProgressList,
            &generateMessageBody);
        if (ADM_ECODE_SUCCESSED != retVal) {
            log_e("adm_ocs_vehicleProgressProtocolAdapter error[%d]", retVal);
            break;
        }
        retVal = adm_irs_addUploadMessage(g_irsSession, ADM_IRS_UPLOAD_PROGRESS, generateMessageBody);
        if (ADM_ECODE_SUCCESSED != retVal) {
            log_e("adm_irs_addUploadMessage error[%d]", retVal);
            break;
        }
    } while (FALSE);
    adm_free(generateMessageBody);
    return retVal;
}

D32S adm_irm_reportEvent(const DCHAR* sessionId,
    D64S taskId,
    D64S strategyId,
    D64S timestamp,
    const DCHAR* sdkVersion,
    const DCHAR* extsReqJsonStr,
    ADM_OCS_EVENTS_INFO_LIST_T* eventsList)
{
    D32S i = 0;

    if (NULL == g_irsSession) {
        log_e("no adm_irm_init");
        return ADM_ECODE_INVALID_PARAM;
    }
    if (NULL == sdkVersion || NULL == eventsList) {
        log_e("input param invalid");
        return ADM_ECODE_INVALID_PARAM;
    }

    D32S retVal = ADM_ECODE_FAILED;
    DCHAR* generateMessageBody = NULL;


    // 避免多线程同时上报事件,eventSequenceId先生成后上报，导致后台接收eventSequenceId序列错误
    (void)pthread_mutex_lock(&g_eventReportMutex);

    for (i = 0; i < eventsList->reportEventsLen; ++i) {
        eventsList->reportEvents[i].eventSequenceId = adm_irm_getEventSequenceId2();        
    }
        
    do {
        retVal = adm_ocs_eventProtocolAdapter(
            sessionId, taskId, strategyId, timestamp, sdkVersion, extsReqJsonStr, eventsList, EVENT_SOURCE_TYPE_UC, &generateMessageBody);
        if (ADM_ECODE_SUCCESSED != retVal) {
            log_e("adm_ocs_vehicleProgressProtocolAdapter error[%d]", retVal);
            break;
        }
        retVal = adm_irs_addUploadMessage(g_irsSession, ADM_IRS_UPLOAD_EVENT, generateMessageBody);
        if (ADM_ECODE_SUCCESSED != retVal) {
            log_e("adm_irs_addUploadMessage error[%d]", retVal);
            break;
        }
    } while (FALSE);

    (void)pthread_mutex_unlock(&g_eventReportMutex);

    adm_free(generateMessageBody);
    return retVal;
}

D32S adm_irm_reportHmiEvent(const DCHAR *sessionId,
                            D64S taskId,
                            D64S strategyId,
                            D64S timestamp,
                            const DCHAR *sdkVersion,
                            const DCHAR *extsReqJsonStr,
                            ADM_OCS_EVENTS_INFO_LIST_T *eventsList)
{
    D32S i = 0;
    D32S retVal = ADM_ECODE_FAILED;
    DCHAR *generateMessageBody = NULL;

    if (NULL == g_irsSession)
    {
        log_e("no adm_irm_init");
        return ADM_ECODE_INVALID_PARAM;
    }

    if (NULL == sdkVersion || NULL == eventsList)
    {
        log_e("input param invalid");
        return ADM_ECODE_INVALID_PARAM;
    }

    // 避免多线程同时上报事件,eventSequenceId先生成后上报，导致后台接收eventSequenceId序列错误
    (void)pthread_mutex_lock(&g_eventReportMutex);

    for (i = 0; i < eventsList->reportEventsLen; ++i)
    {
        eventsList->reportEvents[i].eventSequenceId = adm_irm_getEventSequenceId2();
    }

    do
    {
        retVal = adm_ocs_eventProtocolAdapter(sessionId, taskId, strategyId, timestamp,
                                              sdkVersion, extsReqJsonStr, eventsList, EVENT_SOURCE_TYPE_HMI, &generateMessageBody);
        if (ADM_ECODE_SUCCESSED != retVal)
        {
            log_e("adm_ocs_vehicleProgressProtocolAdapter error[%d]", retVal);
            break;
        }

        retVal = adm_irs_addUploadMessage(g_irsSession, ADM_IRS_UPLOAD_EVENT, generateMessageBody);
        if (ADM_ECODE_SUCCESSED != retVal)
        {
            log_e("adm_irs_addUploadMessage error[%d]", retVal);
            break;
        }
    } while (FALSE);

    (void)pthread_mutex_unlock(&g_eventReportMutex);

    adm_free(generateMessageBody);

    return retVal;
}

D32S adm_irm_reportFile(const DCHAR* sessionId,
    D64S taskId,
    D64S strategyId,
    D64S timestamp,
    D64S startTime,
    D64S endTime,
    const DCHAR* sdkVersion,
    const DCHAR* fullFileNamePath,
    const ADM_OCS_UPLOAD_LOGFILE_ENCRYPTED_T isFileEncrypted,
    const DCHAR* fileHash,
    const D64S fileSize,
    const ADM_OCS_UPLOAD_FILE_TYPE_T fileType,
    const ADM_OCS_UPLOAD_FILE_CMDID_TYPE_T cmdId,
    const DCHAR* pushId,
    const DCHAR* extsReqJsonStr)
{
    if (NULL == g_irsSession) {
        log_e("no adm_irm_init");
        return ADM_ECODE_INVALID_PARAM;
    }
    if (NULL == sdkVersion || NULL == fullFileNamePath || NULL == fileHash
        || ADM_OCS_FILE_ENCRYPTED_INVALID == isFileEncrypted || ADM_OCS_FILE_TYPE_INVALID == fileType
        || ADM_OCS_CMDID_INVALID == cmdId) {
        log_e("input param invalid");
        return ADM_ECODE_INVALID_PARAM;
    }

    D32S retVal = ADM_ECODE_FAILED;
    retVal = adm_irs_addUploadFile(g_irsSession,
        sessionId,
        taskId,
        strategyId,
        timestamp,
        startTime,
        endTime,
        sdkVersion,
        fullFileNamePath,
        isFileEncrypted,
        fileHash,
        fileSize,
        fileType,
        cmdId,
        pushId,
        extsReqJsonStr);

    if (ADM_ECODE_SUCCESSED != retVal) {
        log_e("adm_irsadm_irs_addUploadFile error[%d]", retVal);
    }
    return retVal;
}