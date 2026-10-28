#include "adm_cvs_checkVersion.h"
#include "adm_cvs_checkVersionImpl.h"
#include "adm_com_errorCode.h"
#include "adm_cvs_log.h"
#include "adm_cvs_common.h"
#include "adm_cvs_versionPolicy.h"
#include "adm_com_lib.h"
#include "adm_cfg_default.h"
#include "adm_cfg_api.h"
#include "adm_com_kvstorage.h"
#include "adm_memory.h"
#include "adm_string.h"
#include <time.h>
#include <sys/time.h>

/**
 * @brief 检查给定的事件ID是否对应一个关键事件。
 * 
 * @param pcEventId 事件ID字符串的指针。
 * @return 返回DBOOL类型的值，指示该事件是否为关键事件。
 */
static DBOOL cvsIsCriticalEvent(const DCHAR *pcEventId)
{
    DBOOL bIsCritical = FALSE;

    if (NULL == pcEventId) 
    {
        return FALSE;
    }

    if (0 == adm_strcasecmp(pcEventId, "CT_001") ||
        0 == adm_strcasecmp(pcEventId, "CT_002") ||
        0 == adm_strcasecmp(pcEventId, "CT_003") ||
        0 == adm_strcasecmp(pcEventId, "CT_004") ||
        0 == adm_strcasecmp(pcEventId, "CT_005") ||
        0 == adm_strcasecmp(pcEventId, "CT_006") ||
        0 == adm_strcasecmp(pcEventId, "CT_007") ||
        0 == adm_strcasecmp(pcEventId, "CT_008") ||
        0 == adm_strcasecmp(pcEventId, "CT_009") ||
        0 == adm_strcasecmp(pcEventId, "CT_010") ||
        0 == adm_strcasecmp(pcEventId, "CP_002") ||
        0 == adm_strcasecmp(pcEventId, "CP_004") ||
        0 == adm_strcasecmp(pcEventId, "CP_006") ||
        0 == adm_strcasecmp(pcEventId, "CP_008") ||
        0 == adm_strcasecmp(pcEventId, "CP_010") ||
        0 == adm_strcasecmp(pcEventId, "CI_002") ||
        0 == adm_strcasecmp(pcEventId, "CI_004") ||
        0 == adm_strcasecmp(pcEventId, "CI_006") ||
        0 == adm_strcasecmp(pcEventId, "CI_009") ||
        0 == adm_strcasecmp(pcEventId, "CI_011") ||
        0 == adm_strcasecmp(pcEventId, "CI_013") ||
        0 == adm_strcasecmp(pcEventId, "S_CE_001") ||
        0 == adm_strcasecmp(pcEventId, "S_CE_002") ||
        0 == adm_strcasecmp(pcEventId, "S_CE_003"))
    {
        bIsCritical = TRUE;
    }

    return bIsCritical;
}

D32S adm_cvs_checkVersion(ADM_OCS_CHECK_REQ_ADAPTER_T* checkReq,
    ADM_CVS_NOTIFY_T* notifyF,
    ADM_OCS_CHECK_RESP_ADAPTER_T* checkResp)
{
    if (NULL == checkReq || NULL == notifyF || NULL == notifyF->progressNotifyHmi || NULL == notifyF->stateReportCloud
        || NULL == notifyF->eventReportCloud || NULL == notifyF->getEventSequenceId || NULL == checkResp) {
        log_e("adm_cvs_checkVersion input param invalid");
        return ADM_ECODE_INVALID_PARAM;
    }

    D32S retVal = ADM_ECODE_FAILED;
    D32S getSessionIdRetVal = ADM_ECODE_FAILED;
    ADM_CVS_IMPL_T* impl = NULL;
    ADM_KV_STORAGE_VALUE_TYPE_E eType = ADM_KV_STORAGE_VALUE_TYPE_STRING;
    DCHAR* currentVersionSessionId = NULL;

    do {
        /*malloc memory*/
        {
            impl = adm_cvs_newImplBody();
            if (NULL == impl) {
                log_e("call adm_cvs_newImplBody failed");
                retVal = ADM_ECODE_APPLY_MEMORY_FAILED;
                break;
            }
            impl->implF = adm_cvs_newImplFBody();
            if (NULL == impl->implF) {
                log_e("call adm_cvs_newImplBody failed");
                retVal = ADM_ECODE_APPLY_MEMORY_FAILED;
                break;
            }
        }

        impl->checkReq = checkReq;
        impl->checkResp = checkResp;
        impl->eBusinessCommType = ADM_OCS_BUSINESS_TYPE_CHECK_VER;
        log_i("eBusinessCommType = %d", impl->eBusinessCommType);

        impl->implF->submitPost_F = checkReq->fnBusinessCommCb;
        impl->implF->createMessageBody_F = adm_ocs_checkProtocolAdapter;
        impl->implF->parseMessageBody_F = adm_ocs_checkProtocolParse;
        impl->notifyF = notifyF;

        log_i("--------call adm_cvs_checkVersionImpl-------------");
        retVal = adm_cvs_checkVersionImpl(ADM_CVS_CHECK, impl);
        if (ADM_ECODE_SUCCESSED == retVal) {
            getSessionIdRetVal = adm_cvs_getSessionId(&currentVersionSessionId);
            if (ADM_ECODE_SUCCESSED == getSessionIdRetVal) {
                if (0 == adm_strcmp(currentVersionSessionId, impl->checkResp->sessionId)) {
                    log_i("the current version is the latest");
                    retVal = ADM_ECODE_CHECK_LATEST_VERSION;
                    break;
                }
            } else {
                log_w("no get sessionId");
            }
            /*sava resp data to KV*/
            log_i("sava resp data to KV");
            if (TRUE
                == adm_com_kvstorage_set_string(adm_core_service_getBusinessDatabaseDb(),
                    ADM_CVS_KV_RESPMESSAGBODY_KEY,
                    checkResp->respMessageBodyStr,
                    eType)) {
                log_i("kv storage respMessage successful");
                log_i("checking even.check information stored succeeded");
                (void)adm_cvs_addEvent(impl->notifyF,
                    impl->notifyF->getEventSequenceId(),
                    "CI_012",
                    ADM_OCS_EVENT_LEVEL_TRACE,
                    "check information stored succeeded",
                    "check information stored succeeded",
                    time(NULL));
            } else {
                log_e("kv storage respMessage failed");
                log_i("checking event.check information stored failed");
                (void)adm_cvs_addEvent(impl->notifyF,
                    impl->notifyF->getEventSequenceId(),
                    "CI_013",
                    ADM_OCS_EVENT_LEVEL_ERROR,
                    "check information stored failed",
                    "check information stored failed",
                    time(NULL));
                break;
            }
        }
        log_i("--------exit adm_cvs_checkVersionImpl-------------");
    } while (FALSE);
    notifyF->eventReportCloud(notifyF->eventLists);
    adm_cvs_clearEvent(notifyF);
    if(NULL != impl)
    {
        adm_cvs_deleteImplFBody(impl->implF);
        adm_cvs_deleteImplBody(impl);    
    }
    return retVal;
}

ADM_CVS_NOTIFY_T* adm_cvs_initNotifyCb(checkProgressNotifyHmi_F progressNotifyHmi,
    checkStateReport_F stateReportCloud,
    checkEventReport_F eventReportCloud,
    checkgetEventSequenceId_F getEventSequenceId,
    D16U eventNum,
    D32S ucmConfigReportEventLevel)
{
    if (NULL == progressNotifyHmi || NULL == stateReportCloud || NULL == eventReportCloud
        || NULL == getEventSequenceId) {
        return NULL;
    }
    ADM_CVS_NOTIFY_T* notifyF = NULL;
    notifyF = (ADM_CVS_NOTIFY_T*)adm_malloc(sizeof(ADM_CVS_NOTIFY_T));
    if (NULL != notifyF) {
        (void)adm_memset(notifyF, 0x00, sizeof(ADM_CVS_NOTIFY_T), sizeof(ADM_CVS_NOTIFY_T));
        notifyF->progressNotifyHmi = progressNotifyHmi;
        notifyF->stateReportCloud = stateReportCloud;
        notifyF->eventReportCloud = eventReportCloud;
        notifyF->getEventSequenceId = getEventSequenceId;
        notifyF->ucmConfigReportEventLevel = ucmConfigReportEventLevel;
        if (eventNum > 0U) {
            notifyF->eventLists = (ADM_OCS_EVENTS_INFO_LIST_T*)adm_malloc(sizeof(ADM_OCS_EVENTS_INFO_LIST_T));
            (void)adm_memset(
                notifyF->eventLists, 0x00, sizeof(ADM_OCS_EVENTS_INFO_LIST_T), sizeof(ADM_OCS_EVENTS_INFO_LIST_T));

            notifyF->eventLists->reportEvents
                = (ADM_OCS_EVENTS_INFO_T*)adm_malloc(sizeof(ADM_OCS_EVENTS_INFO_T) * eventNum);
            (void)adm_memset(notifyF->eventLists->reportEvents,
                0x00,
                sizeof(ADM_OCS_EVENTS_INFO_T) * eventNum,
                sizeof(ADM_OCS_EVENTS_INFO_T) * eventNum);
            notifyF->eventLists->reportEventsLen = 0;
        }
    }
    return notifyF;
}

void adm_cvs_deInitNotifyCb(ADM_CVS_NOTIFY_T* notifyF)
{
    if (NULL == notifyF) {
        return;
    }
    if (NULL != notifyF->eventLists) {
        for (D32S i = 0; i < notifyF->eventLists->reportEventsLen; ++i) {
            if (NULL != notifyF->eventLists->reportEvents) {
                adm_free(notifyF->eventLists->reportEvents[i].eventId);
                if (NULL != notifyF->eventLists->reportEvents[i].eventContext) {
                    adm_free(notifyF->eventLists->reportEvents[i].eventContext);
                }
                adm_free(notifyF->eventLists->reportEvents[i].eventDesc);
            }
        }
    }
    adm_free(notifyF->eventLists->reportEvents);
    adm_free(notifyF->eventLists);
    adm_free(notifyF);
}

void adm_cvs_clearEvent(ADM_CVS_NOTIFY_T* notifyF)
{
    if (NULL == notifyF) {
        return;
    }
    if (NULL != notifyF->eventLists) {
        for (D32S i = 0; i < notifyF->eventLists->reportEventsLen; ++i) {
            if (NULL != notifyF->eventLists->reportEvents) {
                adm_free(notifyF->eventLists->reportEvents[i].eventId);
                if (NULL != notifyF->eventLists->reportEvents[i].eventContext) {
                    adm_free(notifyF->eventLists->reportEvents[i].eventContext);
                }
                adm_free(notifyF->eventLists->reportEvents[i].eventDesc);
                (void)adm_memset(notifyF->eventLists->reportEvents + i,
                    0x00,
                    sizeof(ADM_OCS_EVENTS_INFO_T),
                    sizeof(ADM_OCS_EVENTS_INFO_T));
            }
        }
        notifyF->eventLists->reportEventsLen = 0;
    }
    return;
}

/**
 * @brief 获取实时的时间字符串
 * 
 * @return DCHAR* 
 */
static DCHAR* __adm_cvs_getTimeStr(void)
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

D32S adm_cvs_addEvent(ADM_CVS_NOTIFY_T* notifyF,
    D64S eventSequenceId,
    const DCHAR* eventId,
    D32S eventLevel,
    const DCHAR* eventContext,
    const DCHAR* eventDesc,
    D64S eventTime)
{
    DCHAR eventDescTmp[256] = {0};

    if (NULL == notifyF || NULL == notifyF->eventLists || NULL == notifyF->eventLists->reportEvents || NULL == eventId
        || NULL == eventDesc) {
        return ADM_ECODE_INVALID_PARAM;
    }
    
    (void)adm_snprintf(eventDescTmp, 512, "[%s][%s](%s)", "CVM", __adm_cvs_getTimeStr(), eventDesc);

    log_evt("eventID:%s, eventLevel:%d, eventSequenceId:%lld, eventTime:%lld, eventDesc:%s, eventContext:%s", 
            eventId,
            eventLevel,
            eventSequenceId,
            eventTime,
            eventDesc,
            (NULL == eventContext) ? "NULL" : eventContext);

    #ifdef ADM_REPORT_EVENT_OPTIMIZE
        if (FALSE == cvsIsCriticalEvent(eventId))
        {
            //!< 非重要事件不需要上报，只需要在日志中打印即可
            log_w("event is Non Critical");
            return ADM_ECODE_SUCCESSED;
        }
    #endif

    /** 等级不满足，或者事件不重要 */
    if (eventLevel < notifyF->ucmConfigReportEventLevel ) 
    {
        log_w("event level is less than configration report level");
        return ADM_ECODE_SUCCESSED;
    }


    ADM_OCS_EVENTS_INFO_T* tmpReportEvents = notifyF->eventLists->reportEvents + notifyF->eventLists->reportEventsLen;
    if (NULL != tmpReportEvents) {
        tmpReportEvents->eventSequenceId = eventSequenceId;
        tmpReportEvents->eventId = adm_strdup(eventId);
        tmpReportEvents->eventLevel = eventLevel;
        if (NULL != eventContext) {
            tmpReportEvents->eventContext = adm_strdup(eventContext);
        }
        tmpReportEvents->eventDesc = adm_strdup(eventDescTmp);
        tmpReportEvents->eventTime = eventTime;
        notifyF->eventLists->reportEventsLen++;
        return ADM_ECODE_SUCCESSED;
    } else {
        log_e("insufficient event memory size");
        return ADM_ECODE_NO_MEMORY;
    }
}


D32S adm_cvs_usbCheckVersion(const DCHAR *checkFilePath,
                             ADM_CVS_NOTIFY_T *notifyF,
                             ADM_OCS_CHECK_RESP_ADAPTER_T *checkResp)
{
    if (NULL == checkFilePath || 
        NULL == notifyF || 
        NULL == notifyF->progressNotifyHmi || 
        NULL == notifyF->stateReportCloud || 
        NULL == notifyF->eventReportCloud || 
        NULL == notifyF->getEventSequenceId || 
        NULL == checkResp)
    {
        log_e("adm_cvs_usbCheckVersion input param invalid");
        return ADM_ECODE_INVALID_PARAM;
    }

    D32S retVal = ADM_ECODE_FAILED;
    D32S getSessionIdRetVal = ADM_ECODE_FAILED;
    ADM_KV_STORAGE_VALUE_TYPE_E eType = ADM_KV_STORAGE_VALUE_TYPE_STRING;
    DCHAR *currentVersionSessionId = NULL;
    DCHAR *respMessageBody = NULL;

    do
    {
        log_i("load policy resp message body");
        ///< 从U盘读取策略信息
        retVal = adm_com_readFileData(checkFilePath, &respMessageBody);
        if (ADM_ECODE_SUCCESSED != retVal)
        {
            log_e("adm_cvs_usbCheckVersion read policy content failed.[%d]", retVal);
            break;
        }

        log_i("parse resp message body");
        retVal = adm_ocs_usbCheckProtocolParse(respMessageBody, checkResp);
        if (ADM_ECODE_SUCCESSED != retVal)
        {
            log_e("parseMessageBody_F error[%d]", retVal);
            if (ADM_ECODE_AES_ECB_DECRYPT_FAILED == retVal)
            {
                log_i("checking event.Failed to decrypt check response message");
                (void)adm_cvs_addEvent(notifyF,
                                       notifyF->getEventSequenceId(),
                                       "CI_009",
                                       ADM_OCS_EVENT_LEVEL_ERROR,
                                       "Failed to decrypt check response message",
                                       "Failed to decrypt check response message",
                                       time(NULL));
            }
            else if (ADM_ECODE_SIGN_VERIFY_FAILED == retVal)
            {
                log_i("checking event.Failed to compare signature check response message");
                (void)adm_cvs_addEvent(notifyF,
                                       notifyF->getEventSequenceId(),
                                       "CI_011",
                                       ADM_OCS_EVENT_LEVEL_ERROR,
                                       "Failed to compare signature check response message",
                                       "Failed to compare signature check response message",
                                       time(NULL));
            }
            else
            {
                log_i("retVal = %d", retVal);
            }
            break;
        }

        log_i("checking event.Successfully decrypt the check response message");
        (void)adm_cvs_addEvent(notifyF,
                               notifyF->getEventSequenceId(),
                               "CI_008",
                               ADM_OCS_EVENT_LEVEL_TRACE,
                               "Successfully decrypt the check response message",
                               "Successfully decrypt the check response message",
                               time(NULL));
        log_i("checking event.Successful compare signature check response message");
        (void)adm_cvs_addEvent(notifyF,
                               notifyF->getEventSequenceId(),
                               "CI_010",
                               ADM_OCS_EVENT_LEVEL_TRACE,
                               "Successful compare signature check response message",
                               "Successful compare signature check response message",
                               time(NULL));

        /*sava resp data to KV*/
        log_i("sava resp data to KV");
        if (TRUE == adm_com_kvstorage_set_string(adm_core_service_getBusinessDatabaseDb(),
                                                 ADM_CVS_KV_RESPMESSAGBODY_KEY,
                                                 checkResp->respMessageBodyStr,
                                                 eType))
        {
            log_i("kv storage respMessage successful");
            log_i("checking even.check information stored succeeded");
            (void)adm_cvs_addEvent(notifyF,
                                   notifyF->getEventSequenceId(),
                                   "CI_012",
                                   ADM_OCS_EVENT_LEVEL_TRACE,
                                   "check information stored succeeded",
                                   "check information stored succeeded",
                                   time(NULL));
        }
        else
        {
            log_e("kv storage respMessage failed");
            log_i("checking event.check information stored failed");
            (void)adm_cvs_addEvent(notifyF,
                                   notifyF->getEventSequenceId(),
                                   "CI_013",
                                   ADM_OCS_EVENT_LEVEL_ERROR,
                                   "check information stored failed",
                                   "check information stored failed",
                                   time(NULL));
            break;
        }
    } while (FALSE);

    notifyF->eventReportCloud(notifyF->eventLists);
    adm_cvs_clearEvent(notifyF);
    adm_free(respMessageBody);

    return retVal;
}

/**
 * @brief USB 挂载的目录设置
 * 
 * @param[in] pUsbDir     描述:USB挂载的路径存储 \n
 * 范围值: \n
 * 单位: \n
 * 
 * @return D32S 0:Successed None-0:Failed
 */
D32S adm_cvs_setUsbDir(const DCHAR* pUsbDir)
{
    D32S iRet = ADM_ECODE_FAILED;
    ADM_KV_STORAGE_VALUE_TYPE_E eType = ADM_KV_STORAGE_VALUE_TYPE_STRING;
    DCHAR cUsbDir[ADM_COM_LEN_256] = {0};
    DCHAR cUsbDownloadDir[ADM_COM_LEN_256] = {0};

    do
    {
        if (DNULL == pUsbDir)
        {
            iRet = ADM_ECODE_INVALID_PARAM;
            log_e("Input param is invalid, [err:%d]", iRet);
            break;
        }

        /** 判断最后是否有'/'，如果没有则加上，否则不处理 */
        if ('/' != pUsbDir[adm_strlen(pUsbDir)-1])
        {
            adm_snprintf(cUsbDir, sizeof(cUsbDir), "%s/", pUsbDir);
        }
        else
        {
            adm_snprintf(cUsbDir, sizeof(cUsbDir), "%s", pUsbDir);
        }
        if (FALSE
            == adm_com_kvstorage_set_string(adm_core_service_getBusinessDatabaseDb(),
                (const DCHAR*)ADM_CVS_USB_DIR_KV_KEY,
                cUsbDir,
                eType)) 
        {
            iRet = ADM_ECODE_KV_SET_STRING_FAILED;
            log_e("kv storage usb dir %s failed, [err:%d]", cUsbDir, iRet);
        }
        else
        {
            log_i("kv storage usb dir %s successed", cUsbDir);
        }

        //!< 存储USB挂载处的下载路径
        adm_snprintf(cUsbDownloadDir, sizeof(cUsbDownloadDir), "%s%s/", cUsbDir, ADM_CVS_USB_DOWNLOAD_PREFIX);
        if (FALSE
            == adm_com_kvstorage_set_string(adm_core_service_getBusinessDatabaseDb(),
                (const DCHAR*)ADM_CVS_USB_DOWNLOAD_DIR_KV_KEY,
                cUsbDownloadDir,
                eType)) 
        {
            iRet = ADM_ECODE_KV_SET_STRING_FAILED;
            log_e("kv storage usb download dir %s failed, [err:%d]", cUsbDownloadDir, iRet);
        }
        else
        {
            log_i("kv storage usb downlaod dir %s successed", cUsbDownloadDir);
        }
    } while (FALSE);

    return iRet;
}

DCHAR* adm_cvs_getUsbDir(DVOID)
{
    D32S iRet = ADM_ECODE_FAILED;
    ADM_KV_STORAGE_VALUE_TYPE_E eType = ADM_KV_STORAGE_VALUE_TYPE_STRING;
    DCHAR* kvUsbDirPath = NULL;
    size_t kvUsbDirPathLen = 0;

    do
    {
        if (TRUE == adm_com_kvstorage_get_string(
            adm_core_service_getBusinessDatabaseDb(),
            (const DCHAR*)ADM_CVS_USB_DIR_KV_KEY,
            &kvUsbDirPath,
            &kvUsbDirPathLen,
            &eType)
            ) 
        {
            log_i("usb dir %s get from kv successed", kvUsbDirPath);
            return kvUsbDirPath;
        } 
        else 
        {
            iRet = ADM_ECODE_KV_GET_STRING_FAILED;
            log_e("usb dir get from kv failed, [err:%d]", iRet);
        }

    } while (FALSE);
    
    return DNULL;
}


DCHAR* adm_cvs_getUsbDownloadDir(DVOID)
{
    D32S iRet = ADM_ECODE_FAILED;
    ADM_KV_STORAGE_VALUE_TYPE_E eType = ADM_KV_STORAGE_VALUE_TYPE_STRING;
    DCHAR* kvUsbDirPath = NULL;
    size_t kvUsbDirPathLen = 0;

    do
    {
        if (TRUE == adm_com_kvstorage_get_string(
            adm_core_service_getBusinessDatabaseDb(),
            (const DCHAR*)ADM_CVS_USB_DOWNLOAD_DIR_KV_KEY,
            &kvUsbDirPath,
            &kvUsbDirPathLen,
            &eType)
            ) 
        {
            log_i("usb download dir %s get from kv successed", kvUsbDirPath);
            return kvUsbDirPath;
        } 
        else 
        {
            iRet = ADM_ECODE_KV_GET_STRING_FAILED;
            log_e("usb download dir get from kv failed, [err:%d]", iRet);
        }

    } while (FALSE);
    
    return DNULL;    
}

D32S adm_cvs_taskVerify(const D64S ulTaskId,
                        const D64S ulStrategyId,
                        const D64S ulTimestamp,
                        const DCHAR *pcSdkVersion,
                        const DCHAR* extsObjectStr,
                        ADM_OCS_BUSINESS_COMM_INTERFACE_CALLBACK_F fnBusinessCommCb)
{
    D32S iRet = ADM_ECODE_FAILED;
    DCHAR *pcReqBuf = NULL;
    DCHAR *pcRespBuf = NULL;
    DSIZE rspStatusCode = 0;
    
    ADM_COM_FUNC_ENTRY();

    if (ADM_IS_NULL(pcSdkVersion) || ADM_IS_NULL(fnBusinessCommCb))
    {
        log_e("Input param is invalid");
        return ADM_ECODE_INVALID_PARAM;
    }
    
    do
    {
        iRet = adm_ocs_taskVerifyProtocolAdapter(ulTaskId, ulStrategyId, ulTimestamp, pcSdkVersion, extsObjectStr, &pcReqBuf);
        if (ADM_ECODE_SUCCESSED != iRet)
        {
            log_e("adm_ocs_taskVerifyProtocolAdapter failed, [err:%d]", iRet);
            break;
        }
        
        iRet = fnBusinessCommCb(ADM_OCS_BUSINESS_TYPE_TASK_VERIFY, pcReqBuf, &pcRespBuf, &rspStatusCode);
        if (ADM_ECODE_SUCCESSED != iRet)
        {
            log_e("fnBusinessCommCb failed, [err:%d]", iRet);

            /** 防止重启但OCA依赖的PKI参数未设置 */
            for (D32S i = 0; i < 10; i++)
            {
                sleep(6);
                iRet = fnBusinessCommCb(ADM_OCS_BUSINESS_TYPE_TASK_VERIFY, pcReqBuf, &pcRespBuf, &rspStatusCode);
                if (ADM_ECODE_SUCCESSED != iRet)
                {
                    log_e("fnBusinessCommCb failed, [err:%d] retry[%d]", iRet, i);
                }
                else 
                {
                    log_i("Retry success.");
                    break;
                }
            }

            if (ADM_ECODE_SUCCESSED != iRet)
            {
                break;
            }
        }
        
        iRet = adm_ocs_taskVerifyProtocolAdapterResponse(pcRespBuf);
        if (ADM_ECODE_SUCCESSED != iRet)
        {
            log_e("adm_ocs_taskVerifyProtocolAdapterResponse failed, [err:%d]", iRet);
            break;
        }
    } while (FALSE);
    
    ADM_COM_FUNC_EXIT();
    
    return iRet;
}