#include "adm_irs_upload.h"
#include "adm_irs_file.h"
#include "adm_irs_message.h"
#include "adm_memory.h"
#include "adm_string.h"
#include "adm_com_errorCode.h"
#include "adm_irs_common.h"
#include "adm_irs_log.h"

static const DCHAR* g_uploadTypeVehicleState = "[IRS]_[VEHICLE_STATE]";
static const DCHAR* g_uploadTypeEcuState = "[IRS]_[ECU_STATE]";
static const DCHAR* g_uploadTypeProgress = "[IRS]_[PROGRESS]";
static const DCHAR* g_uploadTypeEvent = "[IRS]_[EVENT]";

static const DCHAR* g_vehicleStateHeadKey = "[IRS]_[vehicleStateHeadKey]";
static const DCHAR* g_vehicleStateTailKey = "[IRS]_[vehicleStateTailKey]";
static const DCHAR* g_ecuStateHeadKey = "[IRS]_[ecuStateHeadKey]";
static const DCHAR* g_ecuStateTailKey = "[IRS]_[ecuStateTailKey]";
static const DCHAR* g_progressHeadKey = "[IRS]_[progressHeadKey]";
static const DCHAR* g_progressTailKey = "[IRS]_[progressTailKey]";
static const DCHAR* g_eventHeadKey = "[IRS]_[eventHeadKey]";
static const DCHAR* g_eventTailKey = "[IRS]_[eventStateTailKey]";

KvStorage_t* g_messageDB = NULL;
ADM_OCS_BUSINESS_COMM_INTERFACE_CALLBACK_F g_post = NULL;
#define ADM_IRS_MESSAGE_POSITION_INDEXLENTH 40

void adm_irs_deleteMessageBody(DCHAR* message)
{
    if (NULL == message) {
        return;
    }
    adm_free(message);
}

/**
 * @brief 设置车辆状态消息尾索引值到KV
 *
 * @param [IN]  *positionTailVal 尾索引值
 *
 */
static void setVehicleStateTailIndex(D32S iPositionTailVal)
{
    DCHAR aTailVal[ADM_IRS_MESSAGE_POSITION_INDEXLENTH];
    (void)adm_memzero(aTailVal, ADM_IRS_MESSAGE_POSITION_INDEXLENTH);
    (void)adm_snprintf(aTailVal, ADM_IRS_MESSAGE_POSITION_INDEXLENTH, "%d", iPositionTailVal);
    if (FALSE
        == adm_com_kvstorage_set_string(g_messageDB, g_vehicleStateTailKey, aTailVal, ADM_KV_STORAGE_VALUE_TYPE_INT32)) {
        log_e("save tail.failed");
    }
    return;
}

/**
 * @brief 获取车辆状态消息头尾索引值
 *
 * @param [IN/OUT]  *positionHeadVal 头索引值
 * @param [IN/OUT]  *positionTailVal 尾索引值
 *
 */
static void getVehicleStatePositionIndex(D32S* positionHeadVal, D32S* positionTailVal)
{
    adm_irs_getPositionIndex(
        g_messageDB, g_vehicleStateHeadKey, positionHeadVal, g_vehicleStateTailKey, positionTailVal);
}

/**
 * @brief 设置车辆状态消息头尾索引值到KV
 *
 * @param [IN]  *positionHeadVal 头索引值
 * @param [IN]  *positionTailVal 尾索引值
 *
 */
static void setVehicleStatePositionIndex(D32S positionHeadVal, D32S positionTailVal)
{
    (void)adm_irs_setPositionIndex(
        g_messageDB, g_vehicleStateHeadKey, positionHeadVal, g_vehicleStateTailKey, positionTailVal);
}

/**
 * @brief 设置ECU状态消息尾索引值到KV
 *
 * @param [IN]  *positionTailVal 尾索引值
 *
 */
static void setEcuStateTailIndex(D32S iPositionTailVal)
{
    DCHAR aTailVal[ADM_IRS_MESSAGE_POSITION_INDEXLENTH];
    (void)adm_memzero(aTailVal, ADM_IRS_MESSAGE_POSITION_INDEXLENTH);
    (void)adm_snprintf(aTailVal, ADM_IRS_MESSAGE_POSITION_INDEXLENTH, "%d", iPositionTailVal);
    if (FALSE
        == adm_com_kvstorage_set_string(g_messageDB, g_ecuStateTailKey, aTailVal, ADM_KV_STORAGE_VALUE_TYPE_INT32)) {
        log_e("save tail.failed");
    }
    return;
}

/**
 * @brief 获取ECU状态消息头尾索引值
 *
 * @param [IN/OUT]  *positionHeadVal 头索引值
 * @param [IN/OUT]  *positionTailVal 尾索引值
 *
 */
static void getEcuStatePositionIndex(D32S* positionHeadVal, D32S* positionTailVal)
{
    adm_irs_getPositionIndex(g_messageDB, g_ecuStateHeadKey, positionHeadVal, g_ecuStateTailKey, positionTailVal);
}

/**
 * @brief 设置ECU状态消息头尾索引值到KV
 *
 * @param [IN]  *positionHeadVal 头索引值
 * @param [IN]  *positionTailVal 尾索引值
 *
 */
static void setEcuStatePositionIndex(D32S positionHeadVal, D32S positionTailVal)
{
    (void)adm_irs_setPositionIndex(g_messageDB, g_ecuStateHeadKey, positionHeadVal, g_ecuStateTailKey, positionTailVal);
}

/**
 * @brief 设置车辆进度消息尾索引值到KV
 *
 * @param [IN]  *positionTailVal 尾索引值
 *
 */
static void setProgressTailIndex(D32S iPositionTailVal)
{
    DCHAR aTailVal[ADM_IRS_MESSAGE_POSITION_INDEXLENTH];
    (void)adm_memzero(aTailVal, ADM_IRS_MESSAGE_POSITION_INDEXLENTH);
    (void)adm_snprintf(aTailVal, ADM_IRS_MESSAGE_POSITION_INDEXLENTH, "%d", iPositionTailVal);
    if (FALSE
        == adm_com_kvstorage_set_string(g_messageDB, g_progressTailKey, aTailVal, ADM_KV_STORAGE_VALUE_TYPE_INT32)) {
        log_e("save tail.failed");
    }
    return;
}

/**
 * @brief 获取车辆进度消息头尾索引值
 *
 * @param [IN/OUT]  *positionHeadVal 头索引值
 * @param [IN/OUT]  *positionTailVal 尾索引值
 *
 */
static void getProgressPositionIndex(D32S* positionHeadVal, D32S* positionTailVal)
{
    adm_irs_getPositionIndex(g_messageDB, g_progressHeadKey, positionHeadVal, g_progressTailKey, positionTailVal);
}

/**
 * @brief 设置车辆进度消息头尾索引值到KV
 *
 * @param [IN]  *positionHeadVal 头索引值
 * @param [IN]  *positionTailVal 尾索引值
 *
 */
static void setProgressPositionIndex(D32S positionHeadVal, D32S positionTailVal)
{
    (void)adm_irs_setPositionIndex(g_messageDB, g_progressHeadKey, positionHeadVal, g_progressTailKey, positionTailVal);
}

/**
 * @brief 设置事件消息尾索引值到KV
 *
 * @param [IN]  *positionTailVal 尾索引值
 *
 */
static void setEventTailIndex(D32S iPositionTailVal)
{
    DCHAR aTailVal[ADM_IRS_MESSAGE_POSITION_INDEXLENTH];
    (void)adm_memzero(aTailVal, ADM_IRS_MESSAGE_POSITION_INDEXLENTH);
    (void)adm_snprintf(aTailVal, ADM_IRS_MESSAGE_POSITION_INDEXLENTH, "%d", iPositionTailVal);
    if (FALSE == adm_com_kvstorage_set_string(g_messageDB, g_eventTailKey, aTailVal, ADM_KV_STORAGE_VALUE_TYPE_INT32)) {
        log_e("save tail.failed");
    }
    return;
}

/**
 * @brief 获取事件消息头尾索引值
 *
 * @param [IN/OUT]  *positionHeadVal 头索引值
 * @param [IN/OUT]  *positionTailVal 尾索引值
 *
 */
static void getEventPositionIndex(D32S* positionHeadVal, D32S* positionTailVal)
{
    adm_irs_getPositionIndex(g_messageDB, g_eventHeadKey, positionHeadVal, g_eventTailKey, positionTailVal);
}

/**
 * @brief 设置事件消息头尾索引值到KV
 *
 * @param [IN]  *positionHeadVal 头索引值
 * @param [IN]  *positionTailVal 尾索引值
 *
 */
static void setEventPositionIndex(D32S positionHeadVal, D32S positionTailVal)
{
    (void)adm_irs_setPositionIndex(g_messageDB, g_eventHeadKey, positionHeadVal, g_eventTailKey, positionTailVal);
}

/*############################################ storage message to KV ###########################################*/
/*环形数据库 头结点插入数据但是当前没有数据 空间用于下一次存数据*/
static ADM_IRS_SAVE_UPLOADTOKV_E adm_irs_handleMessage(const DCHAR* pUploadMeassage,
    const DCHAR* pUploadType,
    const DCHAR* pRingDatabaseHeadKey,
    const DCHAR* pRingDatabaseTailKey,
    D32S iMaxRingSize,
    ADM_OCS_BUSINESS_TYPE_E enOcsType)
{
    DCHAR aMessageKey[MESSAGEKEYSIZE];
    (void)adm_memzero(aMessageKey, (DSIZE)MESSAGEKEYSIZE);
    D32S iPositionHeadVal = 0;
    D32S iPositionTailVal = 0;
    DBOOL isSetRetVal = FALSE;
    DBOOL isSaveSuccess = FALSE;
    D32S iRetHttpPostVal = ADM_ECODE_FAILED;
    DCHAR* pMessageResp = NULL;
    DSIZE statusCode = 0;
    D32S iStatus = 0;

    if (NULL == pUploadMeassage || NULL == pUploadType || NULL == pRingDatabaseHeadKey || NULL == pRingDatabaseTailKey) {
        log_e("input invalid param");
        return ADM_IRS_SAVE_INVAILD;
    }
    /*get the head and tail node position from the database*/
    adm_irs_getPositionIndex(g_messageDB, pRingDatabaseHeadKey, &iPositionHeadVal, pRingDatabaseTailKey, &iPositionTailVal);
    log_i("get current position: head = %d , tail = %d", iPositionHeadVal, iPositionTailVal);
    /*messageKey = uploadType + positionHeadVal*/
    adm_irs_generateMessageKey(aMessageKey, MESSAGEKEYSIZE, pUploadType, iPositionHeadVal);
    log_i("generate message key = %s", aMessageKey);

    /*save message and head tail node*/
    isSetRetVal = adm_com_kvstorage_set_string(g_messageDB, aMessageKey, pUploadMeassage, ADM_KV_STORAGE_VALUE_TYPE_STRING);
    if (TRUE == isSetRetVal) {
        log_i("save message key to database.success");
        /*forward head node*/
        adm_irs_forwardHead(g_messageDB, &iPositionHeadVal, &iPositionTailVal, iMaxRingSize, pUploadType, FALSE);
        isSetRetVal = adm_irs_setPositionIndex(
            g_messageDB, pRingDatabaseHeadKey, iPositionHeadVal, pRingDatabaseTailKey, iPositionTailVal);
        if (TRUE == isSetRetVal) {
            log_i("save node to database.success. head = %d; tail = %d", iPositionHeadVal, iPositionTailVal);
            isSaveSuccess = TRUE;
            return ADM_IRS_SAVE_SUCCESS;
        } else {
            log_e("save node to database.failed. head = %d; tail = %d", iPositionHeadVal, iPositionTailVal);
            log_i("remove message key");
            isSetRetVal = adm_com_kvstorage_remove_key(g_messageDB, aMessageKey);
            if (FALSE == isSetRetVal) {
                log_e("remove message key.failed");
            }
        }
    } else {
        log_e("save message to database.failed");
    }
    /*save message and head tail node fail*/
    if (FALSE == isSaveSuccess) {
        log_i("save message to Kv failed");
        if (NULL != g_post) {
            iRetHttpPostVal = g_post(enOcsType, pUploadMeassage, &pMessageResp, &statusCode);
            if (ADM_ECODE_SUCCESSED == iRetHttpPostVal) {
                log_i("ADM_OCS_BUSINESS_TYPE_E = %d , post success", enOcsType);
                (void)adm_ocs_uploadProtocolParse(pMessageResp, &iStatus);
                adm_free(pMessageResp);
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
 * @brief 保存消息到KV处理回调
 *
 * @param [IN]  void* argv 消息参数
 * retval 1:存储成功, 2:存储失败上报失败 ,3:存储失败上报成功
 */
ADM_IRS_SAVE_UPLOADTOKV_E adm_irs_UploadCallBack(void* pArgv, ADM_IRS_UPLOAD_TYPE_E *pUploadType)
{

    ADM_IRS_SAVE_UPLOADTOKV_E enRetVal = ADM_IRS_SAVE_INVAILD;
    ADM_IRS_UPLOAD_T *pUploadData = NULL;
    if (NULL == pArgv || NULL == pUploadType) {
        log_e("Input param is nil");
        return enRetVal;
    }
    pUploadData = (ADM_IRS_UPLOAD_T *)pArgv;
    DCHAR* pArgvMessageData = NULL;
    ADM_OCS_UPLOADFILE_T* pArgvFileData = NULL;
    *pUploadType = pUploadData->enMessageType;
    
    if (ADM_IRS_UPLOAD_FILE == pUploadData->enMessageType){
        pArgvFileData = (ADM_OCS_UPLOADFILE_T *)pUploadData->pArgvData;
        if (NULL == pArgvFileData || NULL == pArgvFileData->fullFileNamePath || NULL == pArgvFileData->sdkVersion
        || NULL == pArgvFileData->fileHash) {
            log_e("upload file info input param invalid");
            return enRetVal;
        }
        log_i("handle message type : file");
        enRetVal = adm_irs_handleFile(pArgvFileData);
        log_i("finish handle message type : file");
        
    }else{
        pArgvMessageData = pUploadData->pArgvData;
        if (NULL == pArgvMessageData) {
            log_e("ADM_IRS_UPLOADMESSAGE_T input param invalid.error[%d]", ADM_ECODE_INVALID_PARAM);
            return enRetVal;
        }
        
        if (ADM_IRS_UPLOAD_VEHICLE_STATE == pUploadData->enMessageType) {
            log_i("handle message type : vehicle state");
            enRetVal = adm_irs_handleMessage(pArgvMessageData,
                g_uploadTypeVehicleState,
                g_vehicleStateHeadKey,
                g_vehicleStateTailKey,
                g_kRingMaxSize,
                ADM_OCS_BUSINESS_TYPE_REPORT_VEHICLE_STATE);
            log_i("finish save message type : vehicle state");
        } else if (ADM_IRS_UPLOAD_ECU_STATE == pUploadData->enMessageType) {
            log_i("handle message type : ecu state");
            enRetVal = adm_irs_handleMessage(pArgvMessageData,
                g_uploadTypeEcuState,
                g_ecuStateHeadKey,
                g_ecuStateTailKey,
                g_kRingMaxSize,
                ADM_OCS_BUSINESS_TYPE_REPORT_ECU_STATE);
            log_i("finish save message type : ecu state");
        } else if (ADM_IRS_UPLOAD_PROGRESS == pUploadData->enMessageType) {
            log_i("handle message type : progress");
            enRetVal = adm_irs_handleMessage(pArgvMessageData,
                g_uploadTypeProgress,
                g_progressHeadKey,
                g_progressTailKey,
                g_kRingMaxSize,
                ADM_OCS_BUSINESS_TYPE_REPORT_PROGRESS);
            log_i("finish save message type : progress");
        } else if (ADM_IRS_UPLOAD_EVENT == pUploadData->enMessageType) {
            log_i("handle message type : event");
            enRetVal = adm_irs_handleMessage(pArgvMessageData,
                g_uploadTypeEvent,
                g_eventHeadKey,
                g_eventTailKey,
                g_kRingMaxSize,
                ADM_OCS_BUSINESS_TYPE_REPORT_EVENT);
            log_i("finish save message type : event");
        }
        else {
            log_e("save message type : unknow");
        }

    }
    adm_irs_deleteUploadBody(pUploadData);
    return enRetVal;

}

/*############################################## report message to cloud
 * ################################################*/

/**
 * @brief 判断KV中是否有消息
 *
 * @return DBOOL    TRUE: 是，FALSE: 否
 *
 */
static DBOOL adm_irs_hasMessage(ADM_IRS_GETPOSITIONINDEX_CALLBACK_F pGetIndexFunc,
    ADM_IRS_SETPOSITIONINDEX_CALLBACK_F pSetIndexFunc)
{
    D32S positionHeadVal = 0;
    D32S positionTailVal = 0;
    pGetIndexFunc(&positionHeadVal, &positionTailVal);
    if (TRUE == adm_irs_ringDatabaseIsEmpty(positionHeadVal, positionTailVal)) {
        log_i("positionHeadVal = %d ; positionTailVal = %d", positionHeadVal, positionTailVal);
        if (0 != positionHeadVal && 0 != positionTailVal) {
            pSetIndexFunc(0, 0);
        }
        return FALSE;
    } else {
        return TRUE;
    }
}

/*############################################## report message to cloud
 * ################################################*/

/**
 * @brief 判断KV中是否有车辆状态消息
 *
 * @return DBOOL    TRUE: 是，FALSE: 否
 *
 */
DBOOL adm_irs_hasVehicleStateMessage(void)
{
    return adm_irs_hasMessage(getVehicleStatePositionIndex, setVehicleStatePositionIndex);
}

/**
 * @brief 判断KV中是否有ECU状态消息
 *
 * @return DBOOL    TRUE: 是，FALSE: 否
 *
 */
DBOOL adm_irs_hasEcuStateMessage(void)
{
    return adm_irs_hasMessage(getEcuStatePositionIndex, setEcuStatePositionIndex);
}

/**
 * @brief 判断KV中是否有车辆进度消息
 *
 * @return DBOOL    TRUE: 是，FALSE: 否
 *
 */
DBOOL adm_irs_hasProgressMessage(void)
{
    return adm_irs_hasMessage(getProgressPositionIndex, setProgressPositionIndex);
}

/**
 * @brief 判断KV中是否有事件消息
 *
 * @return DBOOL    TRUE: 是，FALSE: 否
 *
 */
DBOOL adm_irs_hasEventMessage(void)
{
    return adm_irs_hasMessage(getEventPositionIndex, setEventPositionIndex);
}

/**
 * @brief 上报消息
 *
 * @return  0: 成功
 *        非0: 失败(失败原因需要处理，如果超时需要设置重试等待时间)
 *
 */
static D32S adm_irs_reportMessage(ADM_IRS_GETPOSITIONINDEX_CALLBACK_F pGetIndexFunc,
    ADM_IRS_SETTAILINDEX_CALLBACK_F pSetTailFunc,
    const ADM_OCS_BUSINESS_TYPE_E eBusinessType,
    const DCHAR* pUploadType)
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

    /*获取消息的头尾结点值进行上报*/
    pGetIndexFunc(&iPositionHeadVal, &iPositionTailVal);
    while (FALSE == adm_irs_ringDatabaseIsEmpty(iPositionHeadVal, iPositionTailVal)) {
        adm_irs_generateMessageKey(aMessageKey, MESSAGEKEYSIZE, pUploadType, iPositionTailVal);
        log_i("generate message key = %s", aMessageKey);
        if (TRUE == adm_com_kvstorage_get_string(g_messageDB, aMessageKey, &pMessageVal, &iLength, &eValType)) {
            iRetHttpPostVal = g_post(eBusinessType, pMessageVal, &pMessageResp, &statusCode);
            (void)adm_ocs_uploadProtocolParse(pMessageResp, &iStatus);
            adm_free(pMessageVal);
            adm_free(pMessageResp);
            if (ADM_ECODE_SUCCESSED != iRetHttpPostVal) {
                log_e("message post.failed. stop report.iRet is [%d]", iRetHttpPostVal);
                return iRetHttpPostVal;  // 对返回结果做超时判断
            }
            /*上报成功，获取新更新的消息的头尾结点值，存储线程可能有新数据存入*/
            pGetIndexFunc(&iPositionHeadValCurrent, &iPositionTailValCurrent);
            // 尾没变，先删除上报完成的KV数据，然后按照原来的计划尾+1继续上报
            if (iPositionTailValCurrent == iPositionTailVal) {
                log_i("ADM_OCS_BUSINESS_TYPE_E = %d , post success", eBusinessType);
                if (FALSE == adm_com_kvstorage_remove_key(g_messageDB, aMessageKey)) {
                    log_e("remove messageKey.failed");
                }
                adm_irs_forwardTail(&iPositionTailVal, MESSAGEKEYSIZE);
            }
            // 尾变了，需要用新的尾结点数据进行上报
            else {
                iPositionTailVal = iPositionTailValCurrent;
            }
        } else {
            log_w("no [%s] data in KV", aMessageKey);
            adm_irs_forwardTail(&iPositionTailVal, MESSAGEKEYSIZE);
        }
        pSetTailFunc(iPositionTailVal);
    }
    return ADM_ECODE_SUCCESSED;
}

/**
 * @brief 上报车辆状态
 *
 * @return  0: 成功
 *        非0: 失败(失败原因需要处理，如果超时需要设置重试等待时间)
 *
 */
D32S adm_irs_reportVehicleState(void)
{
    return adm_irs_reportMessage(
        getVehicleStatePositionIndex, setVehicleStateTailIndex, ADM_OCS_BUSINESS_TYPE_REPORT_VEHICLE_STATE, g_uploadTypeVehicleState);
}

/**
 * @brief 上报ECU状态
 *
 * @return  0: 成功
 *        非0: 失败(失败原因需要处理，如果超时需要设置重试等待时间)
 *
 */
D32S adm_irs_reportEcuState(void)
{
    return adm_irs_reportMessage(
        getEcuStatePositionIndex, setEcuStateTailIndex, ADM_OCS_BUSINESS_TYPE_REPORT_ECU_STATE, g_uploadTypeEcuState);
}

/**
 * @brief 上报车辆进度
 *
 * @return  0: 成功
 *        非0: 失败(失败原因需要处理，如果超时需要设置重试等待时间)
 *
 */
D32S adm_irs_reportProgress(void)
{
    return adm_irs_reportMessage(getProgressPositionIndex, setProgressTailIndex, ADM_OCS_BUSINESS_TYPE_REPORT_PROGRESS, g_uploadTypeProgress);
}

/**
 * @brief 上报事件
 *
 * @return  0: 成功
 *        非0: 失败(失败原因需要处理，如果超时需要设置重试等待时间)
 *
 */
D32S adm_irs_reportEvent(void)
{
    return adm_irs_reportMessage(getEventPositionIndex, setEventTailIndex, ADM_OCS_BUSINESS_TYPE_REPORT_EVENT, g_uploadTypeEvent);
}
