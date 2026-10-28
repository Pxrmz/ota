#include "adm_irs_file.h"

#include "adm_com_errorCode.h"
#include "adm_file.h"
#include "adm_memory.h"
#include "adm_string.h"
#include "adm_irs_common.h"
#include "json-c/json.h"
#include "adm_com_errorCode.h"
#include "adm_com_modDefine.h"
#include "adm_irs_log.h"

static const D32U kTimestampArrarLength = 128;

static const DCHAR* g_uploadTypeFile = "[IRS]_[FILE]";
static const DCHAR* g_fileHeadKey = "[IRS]_[fileHeadKey]";
static const DCHAR* g_fileTailKey = "[IRS]_[fileTailKey]";

pthread_mutex_t g_fileMutex;
KvStorage_t* g_fileDB = NULL;
ADM_FILE_UPLOAD_INTERFACE_F g_postFile = NULL;
ADM_OCA_FILE_UPLOAD_INFO_T* g_uploadInfo = NULL;
#define ADM_IRS_FILE_POSITION_INDEXLENTH 40

ADM_OCS_UPLOADFILE_T* adm_irs_newFileBody(const DCHAR* sessionId,
    D64S taskId,
    D64S strategyId,
    const D64S timestamp,
    D64S startTime,
    D64S endTime,
    const DCHAR* sdkVersion,
    ADM_OCS_UPLOAD_FILE_CMDID_TYPE_T cmdId,
    const ADM_OCS_UPLOAD_FILE_TYPE_T fileType,
    const DCHAR* fullFileNamePath,
    ADM_OCS_UPLOAD_LOGFILE_ENCRYPTED_T isFileEncrypted,
    const DCHAR* fileHash,
    D64S fileSize,
    const DCHAR* pushId,
    const DCHAR* extsReqJsonStr)
{
    if (NULL == sdkVersion || NULL == fileHash || NULL == fullFileNamePath || ADM_OCS_CMDID_INVALID == cmdId
        || ADM_OCS_FILE_TYPE_INVALID == fileType || ADM_OCS_FILE_ENCRYPTED_INVALID == isFileEncrypted) {
        log_e("adm_irs_newFileBody input param invalid.error");
        return NULL;
    }

    ADM_OCS_UPLOADFILE_T* uploadFile = NULL;
    uploadFile = (ADM_OCS_UPLOADFILE_T*)adm_malloc(sizeof(ADM_OCS_UPLOADFILE_T));
    if (NULL != uploadFile) {
        if (NULL != sessionId) {
            uploadFile->sessionId = adm_strdup(sessionId);
        }

        uploadFile->taskId = taskId;
        uploadFile->strategyId = strategyId;
        uploadFile->timestamp = timestamp;
        uploadFile->startTime = startTime;
        uploadFile->endTime = endTime;
        uploadFile->sdkVersion = adm_strdup(sdkVersion);
        uploadFile->cmdId = cmdId;
        uploadFile->fileType = fileType;
        uploadFile->fullFileNamePath = adm_strdup(fullFileNamePath);
        uploadFile->isFileEncrypted = isFileEncrypted;
        uploadFile->fileHash = adm_strdup(fileHash);
        uploadFile->fileSize = fileSize;
        if (NULL != pushId) {
            uploadFile->pushId = adm_strdup(pushId);
        }
        if (NULL != extsReqJsonStr) {
            uploadFile->extsReqJsonStr = adm_strdup(extsReqJsonStr);
        }
    }
    return uploadFile;
}

void adm_irs_deleteFileBody(ADM_OCS_UPLOADFILE_T* message)
{
    if (NULL == message) {
        return;
    }

    if (NULL != message->sessionId) {
        adm_free(message->sessionId);
    }
    if (NULL != message->pushId) {
        adm_free(message->pushId);
    }
    if (NULL != message->extsReqJsonStr) {
        adm_free(message->extsReqJsonStr);
    }
    adm_free(message->sign);
    adm_free(message->fullFileNamePath);
    adm_free(message->sdkVersion);
    adm_free(message->fileHash);
    adm_free(message);
}

static DBOOL adm_irs_getJsonStrVal(const json_object* obj, const DCHAR* key, DCHAR** str)
{
    if (NULL == obj || NULL == key) {
        return FALSE;
    }

    json_object* jsonStr = NULL;
    // json_bool failed:return 0
    if ((json_bool)0 == json_object_object_get_ex(obj, key, &jsonStr)) {
        return FALSE;
    }
    *str = adm_strdup(json_object_get_string(jsonStr));
    if (NULL != *str) {
        return TRUE;
    }
    return FALSE;
}

static DBOOL adm_irs_getJsonIntVal(const json_object* obj, const DCHAR* key, D32S* val)
{
    if (NULL == obj || NULL == key) {
        return FALSE;
    }

    json_object* jsonStr = NULL;
    // json_bool failed:return 0
    if ((json_bool)0 == json_object_object_get_ex(obj, key, &jsonStr)) {
        return FALSE;
    }
    *val = json_object_get_int(jsonStr);
    return TRUE;
}

static DBOOL adm_irs_getJsonInt64Val(const json_object* obj, const DCHAR* key, D64S* val)
{
    if (NULL == obj || NULL == key) {
        return FALSE;
    }

    json_object* jsonStr = NULL;
    // json_bool failed:return 0
    if ((json_bool)0 == json_object_object_get_ex(obj, key, &jsonStr)) {
        return FALSE;
    }
    *val = json_object_get_int64(jsonStr);
    return TRUE;
}

/**
 * @brief 设置文件尾索引值到KV
 *
 * @param [IN]  *positionTailVal 尾索引值
 *
 */
static void setFileTailIndex(D32S iPositionTailVal)
{
    DCHAR aTailVal[ADM_IRS_FILE_POSITION_INDEXLENTH];
    (void)adm_memzero(aTailVal, ADM_IRS_FILE_POSITION_INDEXLENTH);
    (void)adm_snprintf(aTailVal, ADM_IRS_FILE_POSITION_INDEXLENTH, "%d", iPositionTailVal);
    if (FALSE == adm_com_kvstorage_set_string(g_messageDB, g_fileTailKey, aTailVal, ADM_KV_STORAGE_VALUE_TYPE_INT32)) {
        log_e("save tail.failed");
    }
    return;
}

static void getFilePositionIndex(D32S * positionHeadVal, D32S * positionTailVal)
{
    adm_irs_getPositionIndex(g_fileDB, g_fileHeadKey, positionHeadVal, g_fileTailKey, positionTailVal);
}

static void setFilePositionIndex(D32S positionHeadVal, D32S positionTailVal)
{
    (void)adm_irs_setPositionIndex(g_fileDB, g_fileHeadKey, positionHeadVal, g_fileTailKey, positionTailVal);
}

static D32S adm_irs_fileInfoToStr(DCHAR** uploadFIleInfoStr, ADM_OCS_UPLOADFILE_T* uploadFIleInfo)
{
    if (NULL == uploadFIleInfo) {
        log_e("input param invalid");
        return ADM_ECODE_INVALID_PARAM;
    }

    json_object* fIleInfoObject = NULL;
    json_object* intObject = NULL;
    json_object* d64Object = NULL;
    json_object* strObject = NULL;
    json_object* extsObject = NULL;
    D32S jsonObjectAddRet = 0;
    D32S retVal = ADM_ECODE_SUCCESSED;
    DCHAR* sign = NULL;

    do {
        fIleInfoObject = json_object_new_object();
        if (NULL == fIleInfoObject) {
            log_e("fIleInfoObject new object.failed");
            retVal = ADM_ECODE_NEW_JSON_OBJECT_FAILED;
            break;
        }
        /*option set sessionId*/
        if (NULL != uploadFIleInfo->sessionId) {
            log_i("set sessionId");
            strObject = json_object_new_string(uploadFIleInfo->sessionId);
            if (NULL == strObject) {
                log_e("sessionId new json object failed");
            }
            jsonObjectAddRet = json_object_object_add(fIleInfoObject, ADM_COM_AIR_PROTOCOL_FIELD_SESSIONID, strObject);
            if (0 != jsonObjectAddRet) {
                log_e("sessionId add json object failed");
            }
        }

        /*option set startTime*/
        if (0 != uploadFIleInfo->startTime) {
            log_i("set startTime");
            d64Object = json_object_new_int64(uploadFIleInfo->startTime);
            if (NULL == d64Object) {
                log_e("startTime new json object failed");
            }
            jsonObjectAddRet = json_object_object_add(fIleInfoObject, ADM_COM_AIR_PROTOCOL_FIELD_STARTTIME, d64Object);
            if (0 != jsonObjectAddRet) {
                log_e("startTime add json object failed");
            }
        }
        /*option set endTime*/
        if (0 != uploadFIleInfo->endTime) {
            log_i("set endTime");
            d64Object = json_object_new_int64(uploadFIleInfo->endTime);
            if (NULL == d64Object) {
                log_e("endTime new json object failed");
            }
            jsonObjectAddRet = json_object_object_add(fIleInfoObject, ADM_COM_AIR_PROTOCOL_FIELD_ENDTIME, d64Object);
            if (0 != jsonObjectAddRet) {
                log_e("endTime add json object failed");
            }
        }
        /*option set taskId*/
        if (0 != uploadFIleInfo->taskId) {
            log_i("set taskId");
            d64Object = json_object_new_int64(uploadFIleInfo->taskId);
            if (NULL == d64Object) {
                log_e("taskId new json object failed");
            }
            jsonObjectAddRet = json_object_object_add(fIleInfoObject, ADM_COM_AIR_PROTOCOL_FIELD_TASKID, d64Object);
            if (0 != jsonObjectAddRet) {
                log_e("taskId add json object failed");
            }
        }
        /*option set strategyId*/
        if (0 !=uploadFIleInfo->strategyId) {
            log_i("set strategyId");
            d64Object = json_object_new_int64(uploadFIleInfo->strategyId);
            if (NULL == d64Object) {
                log_e("strategyId new json object failed");
            }
            jsonObjectAddRet = json_object_object_add(fIleInfoObject, ADM_COM_AIR_PROTOCOL_FIELD_STRATEGYID, d64Object);
            if (0 != jsonObjectAddRet) {
                log_e("strategyId add json object failed");
            }
        }
        /*option set pushId*/
        if (NULL != uploadFIleInfo->pushId) {
            log_i("set pushId");
            strObject = json_object_new_string(uploadFIleInfo->pushId);
            if (NULL == strObject) {
                log_e("pushId new json object failed");
            }
            jsonObjectAddRet = json_object_object_add(fIleInfoObject, ADM_COM_AIR_PROTOCOL_FIELD_PUSHID, strObject);
            if (0 != jsonObjectAddRet) {
                log_e("pushId add json object failed");
            }
        }
        /*option set exts*/
        if (NULL != uploadFIleInfo->extsReqJsonStr) {
            log_i("set exts to checkVersionObject");
            extsObject = json_tokener_parse(uploadFIleInfo->extsReqJsonStr);
            if (NULL != extsObject) {
                if (0 != json_object_object_add(fIleInfoObject, ADM_COM_AIR_PROTOCOL_FIELD_EXT, extsObject)) {
                    log_e("exts add json object.failed");
                }
            } else {
                log_e("exts str to object.failed");
            }
        }
        /*timestamp*/
        intObject = json_object_new_int64(uploadFIleInfo->timestamp);
        if (NULL == intObject) {
            log_e("timestamp new json object failed");
            retVal = ADM_ECODE_NEW_JSON_OBJECT_FAILED;
            break;
        }
        jsonObjectAddRet = json_object_object_add(fIleInfoObject, ADM_COM_AIR_PROTOCOL_FIELD_TIMESTAMP, intObject);
        if (0 != jsonObjectAddRet) {
            log_e("timestamp add json object failed");
            retVal = ADM_ECODE_JSON_ADD_INT_FAILED;
            break;
        }
        /*sdkVersion*/
        strObject = NULL;
        if (NULL != uploadFIleInfo->sdkVersion)
        {
            strObject = json_object_new_string(uploadFIleInfo->sdkVersion);
        }
        if (NULL == strObject) {
            log_e("sdkVersion new json object failed");
            retVal = ADM_ECODE_NEW_JSON_OBJECT_FAILED;
            break;
        }
        jsonObjectAddRet = json_object_object_add(fIleInfoObject, ADM_COM_AIR_PROTOCOL_FIELD_SDKVERSION, strObject);
        if (0 != jsonObjectAddRet) {
            log_e("sdkVersion add json object failed");
            retVal = ADM_ECODE_JSON_ADD_INT64_FAILED;
            break;
        }
        /*cmdId*/
        intObject = json_object_new_int((int32_t)uploadFIleInfo->cmdId);
        if (NULL == intObject) {
            log_e("cmdId new json object failed");
            retVal = ADM_ECODE_NEW_JSON_OBJECT_FAILED;
            break;
        }
        jsonObjectAddRet = json_object_object_add(fIleInfoObject, ADM_COM_AIR_PROTOCOL_FIELD_CMDID, intObject);
        if (0 != jsonObjectAddRet) {
            log_e("cmdId add json object failed");
            retVal = ADM_ECODE_JSON_ADD_INT_FAILED;
            break;
        }
        /*type*/
        intObject = json_object_new_int((int32_t)uploadFIleInfo->fileType);
        if (NULL == intObject) {
            log_e("type new json object failed");
            retVal = ADM_ECODE_NEW_JSON_OBJECT_FAILED;
            break;
        }
        jsonObjectAddRet = json_object_object_add(fIleInfoObject, ADM_COM_AIR_PROTOCOL_FIELD_TYPE, intObject);
        if (0 != jsonObjectAddRet) {
            log_e("type add json object failed");
            retVal = ADM_ECODE_JSON_ADD_INT_FAILED;
            break;
        }
        /*isLogFileEncrypted*/
        intObject = json_object_new_int((int32_t)uploadFIleInfo->isFileEncrypted);
        if (NULL == intObject) {
            log_e("isLogFileEncrypted new json object failed");
            retVal = ADM_ECODE_NEW_JSON_OBJECT_FAILED;
            break;
        }
        jsonObjectAddRet = json_object_object_add(fIleInfoObject, ADM_COM_AIR_PROTOCOL_FIELD_FILEENCRYPTED, intObject);
        if (0 != jsonObjectAddRet) {
            log_e("isLogFileEncrypted add json object failed");
            retVal = ADM_ECODE_JSON_ADD_INT_FAILED;
            break;
        }
        /*logFileSize*/
        intObject = json_object_new_int64(uploadFIleInfo->fileSize);
        if (NULL == intObject) {
            log_e("logFileSize new json object failed");
            retVal = ADM_ECODE_NEW_JSON_OBJECT_FAILED;
            break;
        }
        jsonObjectAddRet = json_object_object_add(fIleInfoObject, ADM_COM_AIR_PROTOCOL_FIELD_LOGFILESIZE, intObject);
        if (0 != jsonObjectAddRet) {
            log_e("logFileSize add json object failed");
            retVal = ADM_ECODE_JSON_ADD_INT_FAILED;
            break;
        }
        /*logFileHash*/
        strObject = NULL;
        if (NULL != uploadFIleInfo->fileHash)
        {
            strObject = json_object_new_string(uploadFIleInfo->fileHash);
        }
        if (NULL == strObject) {
            log_e("logFileHash new json object failed");
            retVal = ADM_ECODE_NEW_JSON_OBJECT_FAILED;
            break;
        }
        jsonObjectAddRet = json_object_object_add(fIleInfoObject, ADM_COM_AIR_PROTOCOL_FIELD_LOGFILEHASH, strObject);
        if (0 != jsonObjectAddRet) {
            log_e("logFileHash add json object failed");
            retVal = ADM_ECODE_JSON_ADD_INT64_FAILED;
            break;
        }
        retVal = adm_ocs_strSign(json_object_to_json_string(fIleInfoObject), &sign);
        if (ADM_ECODE_SUCCESSED != retVal) {
            log_e("adm_ocs_strSign return error[%d]", retVal);
            break;
        }
        uploadFIleInfo->sign = sign;
        /*sign*/
        strObject = NULL;
        if (NULL != uploadFIleInfo->sign)
        {
            strObject = json_object_new_string(uploadFIleInfo->sign);
        }
        if (NULL == strObject) {
            log_e("sign new json object failed");
            retVal = ADM_ECODE_NEW_JSON_OBJECT_FAILED;
            break;
        }
        jsonObjectAddRet = json_object_object_add(fIleInfoObject, ADM_COM_AIR_PROTOCOL_FIELD_SIGN, strObject);
        if (0 != jsonObjectAddRet) {
            log_e("sign add json object failed");
            retVal = ADM_ECODE_JSON_ADD_INT64_FAILED;
            break;
        }
        /*logFile*/
        strObject = NULL;
        if (NULL != uploadFIleInfo->fullFileNamePath)
        {
            strObject = json_object_new_string(uploadFIleInfo->fullFileNamePath);
        }
        if (NULL == strObject) {
            log_e("logFile new json object failed");
            retVal = ADM_ECODE_NEW_JSON_OBJECT_FAILED;
            break;
        }
        jsonObjectAddRet = json_object_object_add(fIleInfoObject, ADM_COM_AIR_PROTOCOL_FIELD_LOGFILE, strObject);
        if (0 != jsonObjectAddRet) {
            log_e("logFile add json object failed");
            retVal = ADM_ECODE_JSON_ADD_INT64_FAILED;
            break;
        }

        *uploadFIleInfoStr = adm_strdup(json_object_to_json_string(fIleInfoObject));
        if (NULL == *uploadFIleInfoStr) {
            retVal = ADM_ECODE_JSON_TO_STRING_FAILED;
            log_e("kv storage get upload message. failed");
            break;
        }
        log_i("fileInfoStr = %s", *uploadFIleInfoStr);
    } while (FALSE);
    (void)json_object_put(fIleInfoObject);
    return retVal;
}

static D32S adm_irs_fullFilePathRename(const DCHAR* oldpath, DCHAR** newpath)
{
    D64S timestamp = 0;
    DCHAR timeStr[kTimestampArrarLength];
    DCHAR* position = NULL;
    DCHAR* newPathTmp = NULL;
    DSIZE newPathLength = 0;

    timestamp = time(NULL);
    (void)adm_memset(timeStr, 0x00, kTimestampArrarLength, kTimestampArrarLength);

    position = adm_strrchr(oldpath, '/');
    if (NULL == position) {
        log_e("get position .failed");
        return ADM_ECODE_IRS_UPLOADFILEPATHFORMAT_FAILED;
    }
    ++position;
    (void)adm_snprintf(timeStr, kTimestampArrarLength, "report_file_%lld_%s", timestamp, position); /*时间戳给文件重命名*/

    newPathLength = (DSIZE)position - (DSIZE)oldpath + (DSIZE)adm_strlen(timeStr);
    log_i("newPath length is %d", newPathLength);

    newPathTmp = (DCHAR*)adm_malloc(newPathLength + (DSIZE)1);
    if (NULL == newPathTmp) {
        log_e("newPathTmp adm_malloc return.failed");
        return ADM_ECODE_APPLY_MEMORY_FAILED;
    }
    (void)adm_memset(newPathTmp, 0, newPathLength, newPathLength);

    if (NULL == adm_strncpy(newPathTmp, oldpath, (DSIZE)position - (DSIZE)oldpath, newPathLength)) {
        log_e("newPathTmp adm_strncpy return.failed");
        adm_free(newPathTmp);
        return ADM_ECODE_INVALID_POINTER;
    }

    if (NULL == adm_strncat(newPathTmp, timeStr, adm_strlen(timeStr), newPathLength + (DSIZE)1)) {
        log_e("newPathTmp adm_strncat return.failed");
        adm_free(newPathTmp);
        return ADM_ECODE_INVALID_POINTER;
    }

    if (ADM_ERR_OK == adm_frename(oldpath, newPathTmp)) {
        *newpath = newPathTmp;
        newPathTmp = NULL;
        return ADM_ECODE_SUCCESSED;
    } else {
        adm_free(newPathTmp);
        return ADM_ECODE_IRS_UPLOADFILEPATHRENAME_FAILED;
    }
}
static D32S adm_irs_uploadFilePathRename(DCHAR** fullFilePath)
{
    DCHAR* newpath = NULL;
    D32S retVal = ADM_ECODE_SUCCESSED;

    retVal = adm_irs_fullFilePathRename(*fullFilePath, &newpath);
    if (ADM_ECODE_SUCCESSED != retVal) {
        log_e("file path rename.error[%d]", retVal);
        return retVal;
    }
    adm_free(*fullFilePath);
    *fullFilePath = NULL;
    *fullFilePath = newpath;
    return retVal;
}

D32S adm_irs_uploadInfoJsonToStruct(const DCHAR* uploadInfoStr, ADM_OCS_UPLOADFILE_T** uploadInfoStruct)
{
    if (NULL == uploadInfoStr) {
        log_e("uploadInfoStr is NULL,error");
        return ADM_ECODE_INVALID_PARAM;
    }

    D32S retVal = ADM_ECODE_SUCCESSED;
    DCHAR* sessionId = NULL;
    D64S taskId = 0;
    D64S strategyId = 0;
    D64S timestamp = 0;
    D64S startTime = 0;
    D64S endTime = 0;
    DCHAR* sdkVersion = NULL;
    D32S cmdId;
    D32S fileType;
    DCHAR* fullFileNamePath = NULL;
    D32S isFileEncrypted;
    DCHAR* fileHash = NULL;
    D64S fileSize = 0;
    DCHAR* pushId = NULL;
    DCHAR* extsReqJsonStr = NULL;
    DCHAR* sign = NULL;
    json_object* extsObject = NULL;

    json_object* fileObject = json_tokener_parse(uploadInfoStr);
    if (NULL == fileObject) {
        log_e("file str to json.failed");
        retVal = ADM_ECODE_STRING_TO_JSON_FAILED;
        return retVal;
    }
    log_i("fileInfoStr= %s", uploadInfoStr);

    do {
        /*option field*/
        (void)adm_irs_getJsonStrVal(fileObject, ADM_COM_AIR_PROTOCOL_FIELD_SESSIONID, &sessionId);
        (void)adm_irs_getJsonInt64Val(fileObject, ADM_COM_AIR_PROTOCOL_FIELD_TASKID, &taskId);
        (void)adm_irs_getJsonInt64Val(fileObject, ADM_COM_AIR_PROTOCOL_FIELD_STRATEGYID, &strategyId);
        (void)adm_irs_getJsonStrVal(fileObject, ADM_COM_AIR_PROTOCOL_FIELD_PUSHID, &pushId);
        (void)adm_irs_getJsonInt64Val(fileObject, ADM_COM_AIR_PROTOCOL_FIELD_STARTTIME, &startTime);
        (void)adm_irs_getJsonInt64Val(fileObject, ADM_COM_AIR_PROTOCOL_FIELD_ENDTIME, &endTime);
        /*option:get exts*/
        // json_bool failed:return 0
        if ((json_bool)0 != json_object_object_get_ex(fileObject, ADM_COM_AIR_PROTOCOL_FIELD_EXT, &extsObject)) {
            extsReqJsonStr = adm_strdup(json_object_to_json_string(extsObject));
            log_i("fileObject contains exts");
        } else {
            log_w("fileObject is not contains exts");
        }

        if (FALSE == adm_irs_getJsonInt64Val(fileObject, ADM_COM_AIR_PROTOCOL_FIELD_TIMESTAMP, &timestamp)) {
            retVal = ADM_ECODE_JSON_GET_INT64_FAILED;
            log_e("get timestamp failed");
            break;
        }
        if (FALSE == adm_irs_getJsonStrVal(fileObject, ADM_COM_AIR_PROTOCOL_FIELD_SDKVERSION, &sdkVersion)) {
            retVal = ADM_ECODE_JSON_GET_STRING_FAILED;
            log_e("get sdkVersion failed");
            break;
        }
        if (FALSE == adm_irs_getJsonIntVal(fileObject, ADM_COM_AIR_PROTOCOL_FIELD_CMDID, &cmdId)) {
            retVal = ADM_ECODE_JSON_GET_INT_FAILED;
            log_e("get cmdId failed");
            break;
        }
        if (FALSE == adm_irs_getJsonIntVal(fileObject, ADM_COM_AIR_PROTOCOL_FIELD_TYPE, &fileType)) {
            retVal = ADM_ECODE_JSON_GET_INT_FAILED;
            log_e("get type failed");
            break;
        }
        if (FALSE == adm_irs_getJsonStrVal(fileObject, ADM_COM_AIR_PROTOCOL_FIELD_LOGFILE, &fullFileNamePath)) {
            retVal = ADM_ECODE_JSON_GET_STRING_FAILED;
            log_e("get logFile failed");
            break;
        }
        if (FALSE == adm_irs_getJsonIntVal(fileObject, ADM_COM_AIR_PROTOCOL_FIELD_FILEENCRYPTED, &isFileEncrypted)) {
            retVal = ADM_ECODE_JSON_GET_INT_FAILED;
            log_e("get isLogFileEncrypted failed");
            break;
        }
        if (FALSE == adm_irs_getJsonStrVal(fileObject, ADM_COM_AIR_PROTOCOL_FIELD_LOGFILEHASH, &fileHash)) {
            retVal = ADM_ECODE_JSON_GET_STRING_FAILED;
            log_e("get logFileHash failed");
            break;
        }
        if (FALSE == adm_irs_getJsonInt64Val(fileObject, ADM_COM_AIR_PROTOCOL_FIELD_LOGFILESIZE, &fileSize)) {
            retVal = ADM_ECODE_JSON_GET_INT_FAILED;
            log_e("get logFileSize failed");
            break;
        }
        if (FALSE == adm_irs_getJsonStrVal(fileObject, ADM_COM_AIR_PROTOCOL_FIELD_SIGN, &sign)) {
            retVal = ADM_ECODE_JSON_GET_STRING_FAILED;
            log_e("get sign failed");
            break;
        }
    } while (FALSE);

    (void)json_object_put(fileObject);

    if (ADM_ECODE_SUCCESSED == retVal) {
        *uploadInfoStruct = adm_irs_newFileBody(sessionId,
            taskId,
            strategyId,
            timestamp,
            startTime,
            endTime,
            sdkVersion,
            (ADM_OCS_UPLOAD_FILE_CMDID_TYPE_T)cmdId,
            (ADM_OCS_UPLOAD_FILE_TYPE_T)fileType,
            fullFileNamePath,
            (ADM_OCS_UPLOAD_LOGFILE_ENCRYPTED_T)isFileEncrypted,
            fileHash,
            fileSize,
            pushId,
            extsReqJsonStr);
        if (NULL == *uploadInfoStruct) {
            log_e("uploadInfoStruct is NULL,error");
            retVal = ADM_ECODE_APPLY_MEMORY_FAILED;
        }
        else
        {
            (*uploadInfoStruct)->sign = adm_strdup(sign);
        }
    }
    adm_free(sessionId);
    adm_free(pushId);
    adm_free(extsReqJsonStr);
    adm_free(fullFileNamePath);
    adm_free(fileHash);
    adm_free(sign);
    adm_free(sdkVersion);
    return retVal;
}


/*############################################ storage message to KV ###########################################*/
/*环形数据库 头结点插入数据但是当前没有数据 空间用于下一次存数据*/
ADM_IRS_SAVE_UPLOADTOKV_E adm_irs_handleFile(ADM_OCS_UPLOADFILE_T* pArgvData)
{
    D32S iRetVal = ADM_ECODE_FAILED;
    DCHAR* pUploadFIleInfoStr = NULL;
    D32S iPositionHeadVal = 0;
    D32S iPositionTailVal = 0;
    DCHAR aMessageKey[MESSAGEKEYSIZE];
    (void)adm_memzero(aMessageKey, (DSIZE)MESSAGEKEYSIZE);
    DBOOL isSetRetVal = FALSE;
    DBOOL isSaveSuccess = FALSE;
    D32S iRetHttpPostVal = ADM_ECODE_FAILED;
    DCHAR* aMessageResp = NULL;
    D32S iStatus = 0;

    if (NULL == pArgvData)
    {
        log_e("Input param is nil");
        return ADM_IRS_SAVE_INVAILD;
    }
    log_i("file name rename");
    iRetVal = adm_irs_uploadFilePathRename(&(pArgvData->fullFileNamePath));
    if (ADM_ECODE_SUCCESSED != iRetVal) {
        log_e("file path rename.error[%d]", iRetVal);
    }
    log_i("rename fullFileNamePath = %s", pArgvData->fullFileNamePath);

    if (ADM_ECODE_SUCCESSED != adm_irs_fileInfoToStr(&pUploadFIleInfoStr, pArgvData)) {
        log_e("file info to str.error");
    }

    /*get the head and tail node position from the database*/
    getFilePositionIndex(&iPositionHeadVal, &iPositionTailVal);
    log_i("current position: head = %d , tail = %d", iPositionHeadVal, iPositionTailVal);

    /*messageKey = uploadType + positionHeadVal*/
    adm_irs_generateMessageKey(aMessageKey, MESSAGEKEYSIZE, g_uploadTypeFile, iPositionHeadVal);
    log_i("generate file message key = %s", aMessageKey);

    isSetRetVal = adm_com_kvstorage_set_string(g_fileDB, aMessageKey, pUploadFIleInfoStr, ADM_KV_STORAGE_VALUE_TYPE_STRING);
    if (TRUE == isSetRetVal) {
        log_i("save file message key to database.success");
        /*forward head node*/
        adm_irs_forwardHead(g_fileDB, &iPositionHeadVal, &iPositionTailVal, g_kRingMaxSize, g_uploadTypeFile, TRUE);
        isSetRetVal = adm_irs_setPositionIndex(g_fileDB, g_fileHeadKey, iPositionHeadVal, g_fileTailKey, iPositionTailVal);
        if (TRUE == isSetRetVal) {
            log_i("save file message node to database.success. head = %d; tail = %d", iPositionHeadVal, iPositionTailVal);
            isSaveSuccess = TRUE;
            adm_free(pUploadFIleInfoStr);
            return ADM_IRS_SAVE_SUCCESS;
        } else {
            log_e("save file message node to database.failed");
            if (FALSE == adm_com_kvstorage_remove_key(g_fileDB, aMessageKey)) {
                log_e("remove messageKey.failed");
            }
        }
    } else {
        log_e("save file message to database.failed");
    }
    adm_free(pUploadFIleInfoStr);
    
    /*save message and head tail node fail*/
    if (FALSE == isSaveSuccess) {
        log_i("save file to Kv failed");
        if (NULL != g_postFile) {
            iRetHttpPostVal = g_postFile(g_uploadInfo, &aMessageResp, pArgvData);
            if (ADM_ECODE_SUCCESSED == iRetHttpPostVal) {
                log_i("ADM_OCS_BUSINESS_TYPE_E = %d , post success", ADM_OCS_BUSINESS_TYPE_REPORT_FILE);
                (void)adm_ocs_uploadProtocolParse(aMessageResp, &iStatus);
                adm_free(aMessageResp);
                log_i("remove file : %s", pArgvData->fullFileNamePath);
                (void)adm_fremove(pArgvData->fullFileNamePath);
                return ADM_IRS_SAVE_FAIL_UPSUCCESS;
            } else {
                log_e("post.failed");
                return ADM_IRS_SAVE_FAIL_UPFAIL;
            }
        } else {
            log_e("post interface is NULL.failed");
            return ADM_IRS_SAVE_FAIL_UPFAIL;
        }
    }
}


/**
 * @brief 判断KV中是否有文件
 *
 * @return DBOOL    TRUE: 是，FALSE: 否
 *
 */
DBOOL adm_irs_hasFile(void)
{
    D32S iPositionHeadVal = 0;
    D32S iPositionTailVal = 0;
    getFilePositionIndex(&iPositionHeadVal, &iPositionTailVal);
    if (TRUE == adm_irs_ringDatabaseIsEmpty(iPositionHeadVal, iPositionTailVal)) {
        log_i("positionHeadVal = %d ; positionTailVal = %d", iPositionHeadVal, iPositionTailVal);
        if (0 != iPositionHeadVal && 0 != iPositionTailVal) {
            setFilePositionIndex(0, 0);
        }
        return FALSE;
    } else {
        return TRUE;
    }
}


/**
 * @brief 上报文件
 *
 * @return  0: 成功
 *        非0: 失败(失败原因需要处理，如果超时需要设置重试等待时间)
 *
 */
D32S adm_irs_reportFile(void)
{

    D32S iPositionHeadVal = 0;
    D32S iPositionTailVal = 0;
    D32S iPositionHeadValCurrent = 0;
    D32S iPositionTailValCurrent = 0;
    DCHAR aMessageKey[MESSAGEKEYSIZE];
    (void)adm_memzero(aMessageKey, (DSIZE)MESSAGEKEYSIZE);
    D32S iRetHttpPostVal = ADM_ECODE_FAILED;
    DSIZE statusCode = 0;
    DCHAR* pMessageResp = NULL;
    size_t iLength = 0;
    ADM_KV_STORAGE_VALUE_TYPE_E eValType = ADM_KV_STORAGE_VALUE_TYPE_INVALID;
    DCHAR* pMessageVal = NULL;
    D32S iStatus = 0;
    ADM_OCS_UPLOADFILE_T* pUploadInfoStruct = NULL;

    /*获取消息的头尾结点值进行上报*/
    getFilePositionIndex(&iPositionHeadVal, &iPositionTailVal);
    while (FALSE == adm_irs_ringDatabaseIsEmpty(iPositionHeadVal, iPositionTailVal)) {
        adm_irs_generateMessageKey(aMessageKey, MESSAGEKEYSIZE, g_uploadTypeFile, iPositionTailVal);
        log_i("generate message key = %s", aMessageKey);
        if (TRUE == adm_com_kvstorage_get_string(g_messageDB, aMessageKey, &pMessageVal, &iLength, &eValType))
        {
             if (ADM_ECODE_SUCCESSED == adm_irs_uploadInfoJsonToStruct(pMessageVal, &pUploadInfoStruct)
                 && TRUE == adm_fisexist(pUploadInfoStruct->fullFileNamePath)) 
             {
                 iRetHttpPostVal = g_postFile(g_uploadInfo, &pMessageResp, pUploadInfoStruct);
                 adm_free(pMessageVal);
                 if (ADM_ECODE_SUCCESSED != iRetHttpPostVal) 
                 {
                     log_e("message post.failed. stop upload, iRet is [%d]", iRetHttpPostVal);
                     adm_irs_deleteFileBody(pUploadInfoStruct);
                     return iRetHttpPostVal;
                 }
                 /*上报成功，获取新更新的消息的头尾结点值，存储线程可能有新数据存入*/
                 getFilePositionIndex(&iPositionHeadValCurrent, &iPositionTailValCurrent);
                 // 尾没变，先删除上报完成的KV数据，然后按照原来的计划尾+1继续上报
                 if (iPositionTailValCurrent == iPositionTailVal) {
                     (void)adm_ocs_uploadProtocolParse(pMessageResp, &iStatus);
                     log_i("ADM_OCS_BUSINESS_TYPE_E = %d , post success", ADM_OCS_BUSINESS_TYPE_REPORT_FILE);
                     log_i("remove file : %s", pUploadInfoStruct->fullFileNamePath);
                     (void)adm_fremove(pUploadInfoStruct->fullFileNamePath);
                     adm_irs_deleteFileBody(pUploadInfoStruct);
                     adm_free(pMessageResp);
                     log_i("remove file message key = %s from database", aMessageKey);
                     if (FALSE == adm_com_kvstorage_remove_key(g_messageDB, aMessageKey)) {
                         log_e("remove messageKey.failed");
                     }
                     adm_irs_forwardTail(&iPositionTailVal, MESSAGEKEYSIZE);
                 }
                 // 尾变了，需要用新的尾结点数据进行上报
                 else {
                     iPositionTailVal = iPositionTailValCurrent;
                 }
             }
            else
            {
                 log_e("report fullFileNamePath is empty ,remove file message key = %s from database", aMessageKey);
                 adm_irs_deleteFileBody(pUploadInfoStruct);
                 if (FALSE == adm_com_kvstorage_remove_key(g_messageDB, aMessageKey)) {
                     log_e("remove messageKey.failed");
                 }
                 adm_irs_forwardTail(&iPositionTailVal, MESSAGEKEYSIZE);
            }
         }else {
            log_w("no [%s] data in KV", aMessageKey);
            adm_irs_forwardTail(&iPositionTailVal, MESSAGEKEYSIZE);
        }
        setFileTailIndex(iPositionTailVal);
        iPositionHeadVal = iPositionHeadValCurrent;
    }
    return ADM_ECODE_SUCCESSED;
    
}

/**
 * @brief 向云端请求上传url
 * @param in const D64S ulTimestamp 时间戳
 * @param in const DCHAR *pcSdkVersion SDK版本号
 * @param in const DCHAR *extsObjectStr 请求用于扩展字段json类型的字符串
 * @param in ADM_OCS_BUSINESS_COMM_INTERFACE_CALLBACK_F fnBusinessCommCb 回调函数
 * @param out ADM_OCS_UPLOAD_URL_INFO_T *pstUploadUrlInfo 结果
 * @return D32S 0: 成功 非0: 失败
 */
D32S adm_irs_requestUploadUrl(const D64S ulTimestamp,
                              const DCHAR *pcSdkVersion,
                              const DCHAR *extsObjectStr,
                              ADM_OCS_BUSINESS_COMM_INTERFACE_CALLBACK_F fnBusinessCommCb,
                              ADM_OCS_UPLOAD_URL_INFO_T *pstUploadUrlInfo)
{
    D32S iRet = ADM_ECODE_FAILED;
    DCHAR *pcReqBuf = NULL;
    DCHAR *pcRespBuf = NULL;
    DSIZE rspStatusCode = 0;
    
    ADM_COM_FUNC_ENTRY();

    /** 参数校验 */
    if (ADM_IS_NULL(pcSdkVersion) || ADM_IS_NULL(fnBusinessCommCb))
    {
        log_e("Input param is invalid");
        return ADM_ECODE_INVALID_PARAM;
    }
    
    do
    {
        /** 请求协议适配 */
        iRet = adm_ocs_requestUploadUrlProtocolAdapter(ulTimestamp, pcSdkVersion, extsObjectStr, &pcReqBuf);
        if (ADM_ECODE_SUCCESSED != iRet)
        {
            log_e("adm_ocs_requestUploadUrlProtocolAdapter failed, [err:%d]", iRet);
            break;
        }
        
        /** 发送请求 */
        iRet = fnBusinessCommCb(ADM_OCS_BUSINESS_TYPE_REQUEST_UPLOAD_URL, pcReqBuf, &pcRespBuf, &rspStatusCode);
        if (ADM_ECODE_SUCCESSED != iRet)
        {
            log_e("fnBusinessCommCb failed, [err:%d]", iRet);
            break;
        }
        
        /** 响应协议适配 */
        iRet = adm_ocs_requestUploadUrlResponseParse(pcRespBuf, pstUploadUrlInfo);
        if (ADM_ECODE_SUCCESSED != iRet)
        {
            log_e("adm_ocs_requestUploadUrlResponseParse failed, [err:%d]", iRet);
            break;
        }
    } while (FALSE);
    
    ADM_COM_FUNC_EXIT();
    
    return iRet;
}

/**
 * @brief 计算文件上传数据签名
 * @param in ADM_OCS_UPLOADFILE_T* uploadFIleInfo 文件上传信息
 * @param in ADM_OCS_UPLOAD_URL_INFO_T *pstUploadUrlInfo 文件上传url信息
 * @return D32S 0: 成功 非0: 失败
 */
D32S adm_irs_calcFileUploadSign(ADM_OCS_UPLOADFILE_T* uploadFIleInfo, ADM_OCS_UPLOAD_URL_INFO_T *pstUploadUrlInfo)
{
    json_object* fIleInfoObject = NULL;
    json_object* intObject = NULL;
    json_object* d64Object = NULL;
    json_object* strObject = NULL;
    json_object* extsObject = NULL;
    D32S jsonObjectAddRet = 0;
    D32S retVal = ADM_ECODE_SUCCESSED;
    DCHAR* sign = NULL;

    if (NULL == uploadFIleInfo || NULL == pstUploadUrlInfo)
    {
        log_e("input param invalid");
        return ADM_ECODE_INVALID_PARAM;
    }

    do
    {
        fIleInfoObject = json_object_new_object();
        if (NULL == fIleInfoObject)
        {
            log_e("fIleInfoObject new object.failed");
            retVal = ADM_ECODE_NEW_JSON_OBJECT_FAILED;
            break;
        }
        /*option set sessionId*/
        if (NULL != uploadFIleInfo->sessionId)
        {
            log_i("set sessionId");
            strObject = json_object_new_string(uploadFIleInfo->sessionId);
            if (NULL == strObject)
            {
                log_e("sessionId new json object failed");
            }
            jsonObjectAddRet = json_object_object_add(fIleInfoObject, ADM_COM_AIR_PROTOCOL_FIELD_SESSIONID, strObject);
            if (0 != jsonObjectAddRet)
            {
                log_e("sessionId add json object failed");
            }
        }

        /*option set startTime*/
        if (0 != uploadFIleInfo->startTime)
        {
            log_i("set startTime");
            d64Object = json_object_new_int64(uploadFIleInfo->startTime);
            if (NULL == d64Object)
            {
                log_e("startTime new json object failed");
            }
            jsonObjectAddRet = json_object_object_add(fIleInfoObject, ADM_COM_AIR_PROTOCOL_FIELD_STARTTIME, d64Object);
            if (0 != jsonObjectAddRet)
            {
                log_e("startTime add json object failed");
            }
        }
        /*option set endTime*/
        if (0 != uploadFIleInfo->endTime)
        {
            log_i("set endTime");
            d64Object = json_object_new_int64(uploadFIleInfo->endTime);
            if (NULL == d64Object)
            {
                log_e("endTime new json object failed");
            }
            jsonObjectAddRet = json_object_object_add(fIleInfoObject, ADM_COM_AIR_PROTOCOL_FIELD_ENDTIME, d64Object);
            if (0 != jsonObjectAddRet)
            {
                log_e("endTime add json object failed");
            }
        }
        /*option set taskId*/
        if (0 != uploadFIleInfo->taskId)
        {
            log_i("set taskId");
            d64Object = json_object_new_int64(uploadFIleInfo->taskId);
            if (NULL == d64Object)
            {
                log_e("taskId new json object failed");
            }
            jsonObjectAddRet = json_object_object_add(fIleInfoObject, ADM_COM_AIR_PROTOCOL_FIELD_TASKID, d64Object);
            if (0 != jsonObjectAddRet)
            {
                log_e("taskId add json object failed");
            }
        }
        /*option set strategyId*/
        if (0 != uploadFIleInfo->strategyId)
        {
            log_i("set strategyId");
            d64Object = json_object_new_int64(uploadFIleInfo->strategyId);
            if (NULL == d64Object)
            {
                log_e("strategyId new json object failed");
            }
            jsonObjectAddRet = json_object_object_add(fIleInfoObject, ADM_COM_AIR_PROTOCOL_FIELD_STRATEGYID, d64Object);
            if (0 != jsonObjectAddRet)
            {
                log_e("strategyId add json object failed");
            }
        }
        /*option set pushId*/
        if (NULL != uploadFIleInfo->pushId)
        {
            log_i("set pushId");
            strObject = json_object_new_string(uploadFIleInfo->pushId);
            if (NULL == strObject)
            {
                log_e("pushId new json object failed");
            }
            jsonObjectAddRet = json_object_object_add(fIleInfoObject, ADM_COM_AIR_PROTOCOL_FIELD_PUSHID, strObject);
            if (0 != jsonObjectAddRet)
            {
                log_e("pushId add json object failed");
            }
        }
        /*option set exts*/
        if (NULL != uploadFIleInfo->extsReqJsonStr)
        {
            log_i("set exts to checkVersionObject");
            extsObject = json_tokener_parse(uploadFIleInfo->extsReqJsonStr);
            if (NULL != extsObject)
            {
                if (0 != json_object_object_add(fIleInfoObject, ADM_COM_AIR_PROTOCOL_FIELD_EXT, extsObject))
                {
                    log_e("exts add json object.failed");
                }
            }
            else
            {
                log_e("exts str to object.failed");
            }
        }
        /*timestamp*/
        intObject = json_object_new_int64(uploadFIleInfo->timestamp);
        if (NULL == intObject)
        {
            log_e("timestamp new json object failed");
            retVal = ADM_ECODE_NEW_JSON_OBJECT_FAILED;
            break;
        }
        jsonObjectAddRet = json_object_object_add(fIleInfoObject, ADM_COM_AIR_PROTOCOL_FIELD_TIMESTAMP, intObject);
        if (0 != jsonObjectAddRet)
        {
            log_e("timestamp add json object failed");
            retVal = ADM_ECODE_JSON_ADD_INT_FAILED;
            break;
        }
        /*sdkVersion*/
        strObject = NULL;
        if (NULL != uploadFIleInfo->sdkVersion)
        {
            strObject = json_object_new_string(uploadFIleInfo->sdkVersion);
        }
        if (NULL == strObject)
        {
            log_e("sdkVersion new json object failed");
            retVal = ADM_ECODE_NEW_JSON_OBJECT_FAILED;
            break;
        }
        jsonObjectAddRet = json_object_object_add(fIleInfoObject, ADM_COM_AIR_PROTOCOL_FIELD_SDKVERSION, strObject);
        if (0 != jsonObjectAddRet)
        {
            log_e("sdkVersion add json object failed");
            retVal = ADM_ECODE_JSON_ADD_INT64_FAILED;
            break;
        }
        /*cmdId*/
        intObject = json_object_new_int((int32_t)uploadFIleInfo->cmdId);
        if (NULL == intObject)
        {
            log_e("cmdId new json object failed");
            retVal = ADM_ECODE_NEW_JSON_OBJECT_FAILED;
            break;
        }
        jsonObjectAddRet = json_object_object_add(fIleInfoObject, ADM_COM_AIR_PROTOCOL_FIELD_CMDID, intObject);
        if (0 != jsonObjectAddRet)
        {
            log_e("cmdId add json object failed");
            retVal = ADM_ECODE_JSON_ADD_INT_FAILED;
            break;
        }
        /*type*/
        intObject = json_object_new_int((int32_t)uploadFIleInfo->fileType);
        if (NULL == intObject)
        {
            log_e("type new json object failed");
            retVal = ADM_ECODE_NEW_JSON_OBJECT_FAILED;
            break;
        }
        jsonObjectAddRet = json_object_object_add(fIleInfoObject, ADM_COM_AIR_PROTOCOL_FIELD_TYPE, intObject);
        if (0 != jsonObjectAddRet)
        {
            log_e("type add json object failed");
            retVal = ADM_ECODE_JSON_ADD_INT_FAILED;
            break;
        }
        /*isLogFileEncrypted*/
        intObject = json_object_new_int((int32_t)uploadFIleInfo->isFileEncrypted);
        if (NULL == intObject)
        {
            log_e("isLogFileEncrypted new json object failed");
            retVal = ADM_ECODE_NEW_JSON_OBJECT_FAILED;
            break;
        }
        jsonObjectAddRet = json_object_object_add(fIleInfoObject, ADM_COM_AIR_PROTOCOL_FIELD_FILEENCRYPTED, intObject);
        if (0 != jsonObjectAddRet)
        {
            log_e("isLogFileEncrypted add json object failed");
            retVal = ADM_ECODE_JSON_ADD_INT_FAILED;
            break;
        }
        /*logFileSize*/
        intObject = json_object_new_int64(uploadFIleInfo->fileSize);
        if (NULL == intObject)
        {
            log_e("logFileSize new json object failed");
            retVal = ADM_ECODE_NEW_JSON_OBJECT_FAILED;
            break;
        }
        jsonObjectAddRet = json_object_object_add(fIleInfoObject, ADM_COM_AIR_PROTOCOL_FIELD_LOGFILESIZE, intObject);
        if (0 != jsonObjectAddRet)
        {
            log_e("logFileSize add json object failed");
            retVal = ADM_ECODE_JSON_ADD_INT_FAILED;
            break;
        }
        /*logFileHash*/
        strObject = NULL;
        if (NULL != uploadFIleInfo->fileHash)
        {
            strObject = json_object_new_string(uploadFIleInfo->fileHash);
        }
        if (NULL == strObject)
        {
            log_e("logFileHash new json object failed");
            retVal = ADM_ECODE_NEW_JSON_OBJECT_FAILED;
            break;
        }

        jsonObjectAddRet = json_object_object_add(fIleInfoObject, ADM_COM_AIR_PROTOCOL_FIELD_LOGFILEHASH, strObject);
        if (0 != jsonObjectAddRet)
        {
            log_e("logFileHash add json object failed");
            retVal = ADM_ECODE_JSON_ADD_INT64_FAILED;
            break;
        }

        jsonObjectAddRet = json_object_object_add(fIleInfoObject, "filePath", json_object_new_string(pstUploadUrlInfo->pcFilePath));
        if (0 != jsonObjectAddRet)
        {
            log_e("filePath add json object failed");
            retVal = ADM_ECODE_JSON_ADD_INT64_FAILED;
            break;
        }

        retVal = adm_ocs_strSign(json_object_to_json_string(fIleInfoObject), &sign);
        if (ADM_ECODE_SUCCESSED != retVal)
        {
            log_e("adm_ocs_strSign return error[%d]", retVal);
            break;
        }

        adm_free(uploadFIleInfo->sign);
        uploadFIleInfo->sign = adm_strdup(sign);
        adm_free(sign);
    } while (FALSE);

    (void)json_object_put(fIleInfoObject);

    return retVal;
}