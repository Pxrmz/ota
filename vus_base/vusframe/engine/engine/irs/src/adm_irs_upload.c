#include "adm_irs_upload.h"

#include "adm_irs_log.h"
#include "adm_com_errorCode.h"
#include "adm_memory.h"
#include "adm_string.h"
#include "adm_workqueue.h"
#include "adm_file.h"
#include "adm_irs_message.h"
#include "adm_irs_file.h"
#include "adm_irs_messagehandle.h"

#include <pthread.h>
#include <time.h>
#include <unistd.h>

typedef struct ADM_IRS_SESSION_T
{
    ADM_LIB_Workqueue_T* uploadWorkQueue;
} ADM_IRS_SESSION_T;

static const D32S kUploadThreadCount = 1;
static pthread_mutex_t g_uploadMutex;


void adm_irs_deleteUploadBody(ADM_IRS_UPLOAD_T* uploadBody)
{
    if (NULL == uploadBody) {
        return;
    }
    if( ADM_IRS_UPLOAD_FILE == uploadBody->enMessageType)
    {
        adm_irs_deleteFileBody(uploadBody->pArgvData);
    }
    else {
        adm_free(uploadBody->pArgvData);
    }
    adm_free(uploadBody);
}


D32S adm_irs_addUploadMessage(ADM_IRS_SESSION_T* session,
    const ADM_IRS_UPLOAD_TYPE_E messageType,
    const DCHAR* uploadMessage)
{
    D32S retVal = ADM_ECODE_SUCCESSED;

    if (NULL == session || NULL == uploadMessage || ADM_IRS_UPLOAD_INVALID == messageType) {
        log_e("input param invalid");
        return ADM_ECODE_INVALID_PARAM;
    }

    log_i("add message type = %d", messageType);
    ADM_IRS_UPLOAD_T* argv = NULL;
    
    (void)pthread_mutex_lock(&g_uploadMutex);
    argv = (ADM_IRS_UPLOAD_T*)adm_malloc(sizeof(ADM_IRS_UPLOAD_T));
    argv->enMessageType = messageType;
    argv->pArgvData = adm_strdup(uploadMessage);

    if (0 != adm_WorkqueueAdd(session->uploadWorkQueue, (void*)argv)) {
        log_e("adm_WorkqueueAdd.failed");
        adm_irs_deleteUploadBody(argv);
        retVal = ADM_ECODE_APPLY_MEMORY_FAILED;
    }
    (void)pthread_mutex_unlock(&g_uploadMutex);
    
    log_i("add upload message to work queue.success");
    return retVal;
}

D32S adm_irs_syncReportMessage(ADM_IRS_SESSION_T* session,
    const ADM_IRS_UPLOAD_TYPE_E messageType,
    const DCHAR* uploadMessage)
{
    if (NULL == session || NULL == uploadMessage || ADM_IRS_UPLOAD_INVALID == messageType) {
        log_e("input param invalid");
        return ADM_ECODE_INVALID_PARAM;
    }

    D32S retVal = ADM_ECODE_SUCCESSED;
    ADM_OCS_BUSINESS_TYPE_E ocsType;
    DCHAR* messageResp = NULL;
    DSIZE statusCode = 0;
    D32S status = 0;

    if (ADM_IRS_UPLOAD_VEHICLE_STATE == messageType) {
        ocsType = ADM_OCS_BUSINESS_TYPE_REPORT_VEHICLE_STATE;
    } else if (ADM_IRS_UPLOAD_ECU_STATE == messageType) {
        ocsType = ADM_OCS_BUSINESS_TYPE_REPORT_ECU_STATE;
    } else if (ADM_IRS_UPLOAD_PROGRESS == messageType) {
        ocsType = ADM_OCS_BUSINESS_TYPE_REPORT_PROGRESS;
    } else if (ADM_IRS_UPLOAD_EVENT == messageType) {
        ocsType = ADM_OCS_BUSINESS_TYPE_REPORT_EVENT;
    } else {
        return ADM_ECODE_INVALID_PARAM;
    }

    log_i("post message to the cloud");
    if (NULL != g_post) {
        retVal = g_post(ocsType, uploadMessage, &messageResp, &statusCode);
        if (ADM_ECODE_SUCCESSED == retVal) {
            log_i("post success");
            retVal = adm_ocs_uploadProtocolParse(messageResp, &status);
            if (ADM_ECODE_SUCCESSED == retVal) {
                retVal = adm_ocs_otaStatusCodeTransfer(status);
            }
            adm_free(messageResp);
        } else {
            log_e("post.failed");
        }
    } else {
        log_e("post interface is NULL.failed");
        retVal = ADM_ECODE_INVALID_PARAM;
    }
    return retVal;
}

D32S adm_irs_addUploadFile(ADM_IRS_SESSION_T* session,
    const DCHAR* sessionId,
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
    if (NULL == session || NULL == sdkVersion || NULL == fullFileNamePath || NULL == fileHash
        || ADM_OCS_FILE_ENCRYPTED_INVALID == isFileEncrypted || ADM_OCS_FILE_TYPE_INVALID == fileType
        || ADM_OCS_CMDID_INVALID == cmdId) {
        log_e("input param invalid");
        return ADM_ECODE_INVALID_PARAM;
    }
    if (FALSE == adm_fisexist(fullFileNamePath)) {
        log_e("file not exist.error[%d]", ADM_ECODE_IRS_UPLOADFILE_NOTEXIST);
        return ADM_ECODE_IRS_UPLOADFILE_NOTEXIST;
    }

    ADM_IRS_UPLOAD_T* argv = NULL;
    argv = (ADM_IRS_UPLOAD_T*)adm_malloc(sizeof(ADM_IRS_UPLOAD_T));
    argv->enMessageType = ADM_IRS_UPLOAD_FILE;
    ADM_OCS_UPLOADFILE_T* argvFileData = NULL;
    D32S retVal = ADM_ECODE_SUCCESSED;

    (void)pthread_mutex_lock(&g_uploadMutex);
    do {
        argvFileData = adm_irs_newFileBody(sessionId,
            taskId,
            strategyId,
            timestamp,
            startTime,
            endTime,
            sdkVersion,
            cmdId,
            fileType,
            fullFileNamePath,
            isFileEncrypted,
            fileHash,
            fileSize,
            pushId,
            extsReqJsonStr);

        if (NULL == argvFileData) {
            log_e("adm_irs_newFileBody.failed");
            retVal = ADM_ECODE_APPLY_MEMORY_FAILED;
            break;
        }
        argv->pArgvData = (void *)argvFileData;
        if (0 != adm_WorkqueueAdd(session->uploadWorkQueue, (void*)argv)) {
            log_e("adm_WorkqueueAdd.failed");
            retVal = ADM_ECODE_APPLY_MEMORY_FAILED;
            break;
        }
        log_i("add upload file task to work queue.success");
    } while (FALSE);
    
    (void)pthread_mutex_unlock(&g_uploadMutex);

    if (ADM_ECODE_SUCCESSED != retVal) {
        adm_irs_deleteUploadBody(argv);
    }
    return retVal;
}

/*########################### new delete################################*/
ADM_IRS_SESSION_T* adm_irs_new(ADM_OCS_BUSINESS_COMM_INTERFACE_CALLBACK_F post,
    ADM_FILE_UPLOAD_INTERFACE_F postFile,
    ADM_OCA_FILE_UPLOAD_INFO_T* uploadInfo)
{
    if (NULL == post || NULL == postFile) {
        log_e("post input param invalid");
        return NULL;
    }
    if (NULL == adm_core_service_getReportDatabaseDb()) {
        log_e("message DB is NULL");
        return NULL;
    }

    ADM_IRS_SESSION_T* session = NULL;
    DBOOL retVal = TRUE;
    session = (ADM_IRS_SESSION_T*)adm_malloc(sizeof(ADM_IRS_SESSION_T));
    if (NULL == session) {
        log_e("session malloc return NULL");
        return NULL;
    }
    (void)adm_memset(session, 0x00, sizeof(ADM_IRS_SESSION_T), sizeof(ADM_IRS_SESSION_T));

    do {
        /*add upload to CB*/
        session->uploadWorkQueue = adm_WorkqueueNew(kUploadThreadCount, adm_irs_saveReportCallBack);
        if (NULL == session->uploadWorkQueue) {
            log_e("adm_WorkqueueNew return.failed");
            retVal = FALSE;
            break;
        }

        g_post = post;
        g_postFile = postFile;
        if (NULL != uploadInfo) {
            g_uploadInfo = uploadInfo;
        }
    } while (FALSE);

    if (FALSE == retVal) {
        adm_irs_delete(session);
        log_e("irs session initialize failed");
        return NULL;
    } else {
        (void)pthread_mutex_init(&g_uploadMutex, NULL);
        g_messageDB = adm_core_service_getReportDatabaseDb();
        g_fileDB = adm_core_service_getReportDatabaseDb();
        adm_irs_initReportMessageHandle(session);
        log_i("irs session initialize success");
        return session;
    }
}

void adm_irs_delete(ADM_IRS_SESSION_T* session)
{
    if (NULL == session) {
        return;
    }

    if (NULL != session->uploadWorkQueue) {
        (void)adm_WorkqueueDel(session->uploadWorkQueue);
    }

    adm_free(session);
    (void)pthread_mutex_destroy(&g_uploadMutex);
    g_messageDB = NULL;
    g_fileDB = NULL;
    g_post = NULL;
    g_postFile = NULL;
}

DBOOL adm_irs_getHasVehicleStateMessage()
{
    return adm_irs_hasVehicleStateMessage();
}
/*########################### new delete################################*/
