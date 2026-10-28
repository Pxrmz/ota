#include "adm_cvs_checkVersionImpl.h"
#include "adm_cvs_versionPolicy.h"
#include "adm_cvs_common.h"
#include "adm_com_errorCode.h"
#include "adm_cvs_log.h"

#include "adm_memory.h"
#include "adm_string.h"
#include <time.h>

ADM_CVS_IMPL_F_T* adm_cvs_newImplFBody(void)
{
    ADM_CVS_IMPL_F_T* implF = (ADM_CVS_IMPL_F_T*)adm_malloc(sizeof(ADM_CVS_IMPL_F_T));
    return implF;
}

void adm_cvs_deleteImplFBody(ADM_CVS_IMPL_F_T* implF)
{
    if (NULL == implF) {
        return;
    }

    adm_free(implF);
}

ADM_CVS_IMPL_T* adm_cvs_newImplBody(void)
{
    ADM_CVS_IMPL_T* impl = (ADM_CVS_IMPL_T*)adm_malloc(sizeof(ADM_CVS_IMPL_T));
    return impl;
}

void adm_cvs_deleteImplBody(ADM_CVS_IMPL_T* impl)
{
    if (NULL == impl)
    {
        return;
    }
    if (NULL != impl->versionNumber) {
        adm_free(impl->versionNumber);
    }
    adm_free(impl);
}

D32S adm_cvs_checkVersionImpl(ADM_CVS_POST_TYPE_T postType, ADM_CVS_IMPL_T* impl)
{
    /*Parameter validity check*/
    if (NULL == impl->implF || NULL == impl->implF->createMessageBody_F || NULL == impl->implF->submitPost_F) {
        log_e("impl input param invalid");
        return ADM_ECODE_INVALID_PARAM;
    }

    if ((ADM_CVS_CHECK == postType && NULL == impl->implF->parseMessageBody_F)
        || (ADM_CVS_VEHICLEINFO == postType && NULL == impl->implF->parseVehicleInfoMessageBody_F)) {
        log_e("impl input param invalid");
        return ADM_ECODE_INVALID_PARAM;
    }

    DCHAR* reqMessageBody = NULL;
    DCHAR* respMessageBody = NULL;
    // ADM_OCS_CHECK_REQ_ADAPTER_T* reqAdapter = NULL;
    DCHAR* pVehicleVersion = NULL;
    D32S retVal = ADM_ECODE_SUCCESSED;
    DSIZE rspStatusCode = 0;

//     adm_cvs_getHmiVehicleVersion(&pVehicleVersion);
//     if (NULL == pVehicleVersion)
//     {
// #ifdef AIR_PROTOCOL_VEHICLE_VERSION
//             pVehicleVersion = adm_strdup(AIR_PROTOCOL_VEHICLE_VERSION);
// #endif            
//     }

//     /*malloc memory*/
//     reqAdapter = adm_ocs_newCheckReqAdapterBody(impl->checkReq->cmdId,
//         impl->checkReq->timestamp,
//         impl->checkReq->netType,
//         impl->checkReq->versionNumber,
//         impl->checkReq->sdkVersion,
//         impl->checkReq->confVersion,
//         impl->checkReq->allEcuInfoObjectStr,
//         impl->checkReq->allModuleVerInfoStr,
//         pVehicleVersion,
//         impl->checkReq->extsObjectStr,
//         impl->checkReq->fnBusinessCommCb);
//     if (NULL == reqAdapter) {
//         log_e("adm_ocs_newCheckReqAdapterBody.failed");
//         return ADM_ECODE_APPLY_MEMORY_FAILED;
//     }

    do {
        /*creat request message body sign secret*/
        log_i("creat request message body");
        if (NULL != impl->notifyF && NULL != impl->notifyF->progressNotifyHmi)
        {
            impl->notifyF->progressNotifyHmi(
                impl->checkReq->cmdId, "ENCRYPT", ADM_CVS_COMMON_CHECK_PROGRESS_TYPE_ENCRYPT);
        }
        retVal = impl->implF->createMessageBody_F(impl->checkReq, &reqMessageBody);
        if (ADM_ECODE_SUCCESSED != retVal) {
            log_e("createMessageBody_F failed");
            if (ADM_ECODE_CHECK_MESSAGE_SIGN_FAILED == retVal && NULL != impl->notifyF
                && NULL != impl->notifyF->getEventSequenceId) {
                log_i("checking event.Failed to signature check message");
                (void)adm_cvs_addEvent(impl->notifyF,
                    impl->notifyF->getEventSequenceId(),
                    "CI_002",
                    ADM_OCS_EVENT_LEVEL_ERROR,
                    "Failed to signature check message",
                    "Failed to signature check message",
                    time(NULL));
            } else if (ADM_ECODE_CHECK_ENCRYPT_FAILED == retVal && NULL != impl->notifyF
                && NULL != impl->notifyF->getEventSequenceId) {
                log_i("checking event.Failed to encrypt the check message");
                (void)adm_cvs_addEvent(impl->notifyF,
                    impl->notifyF->getEventSequenceId(),
                    "CI_004",
                    ADM_OCS_EVENT_LEVEL_ERROR,
                    "Failed to encrypt the check message",
                    "Failed to encrypt the check message",
                    time(NULL));
            } else 
            {
                log_i("retVal = %d", retVal);
            }
            break;
        }

        if (NULL != impl->notifyF && NULL != impl->notifyF->getEventSequenceId) {
            log_i("checking event.Successful signature check message");
            (void)adm_cvs_addEvent(impl->notifyF,
                impl->notifyF->getEventSequenceId(),
                "CI_001",
                ADM_OCS_EVENT_LEVEL_TRACE,
                "Successful signature check message",
                "Successful signature check message",
                time(NULL));
            log_i("checking event.Successfully encrypt the check message");
            (void)adm_cvs_addEvent(impl->notifyF,
                impl->notifyF->getEventSequenceId(),
                "CI_003",
                ADM_OCS_EVENT_LEVEL_TRACE,
                "Successfully encrypt the check message",
                "Successfully encrypt the check message",
                time(NULL));
        }

        /*http(s) post request*/
        log_i("http(s) post request");
        if (NULL != impl->notifyF && NULL != impl->notifyF->progressNotifyHmi) {
            impl->notifyF->progressNotifyHmi(
                impl->checkReq->cmdId, "REQUEST_TO_SERVER", ADM_CVS_COMMON_CHECK_PROGRESS_TYPE_REQUEST_TO_SERVER);
        }

        if (NULL != impl->notifyF && NULL != impl->notifyF->getEventSequenceId) {
            log_i("checking event.Submit check request to platform");
            (void)adm_cvs_addEvent(impl->notifyF,
                impl->notifyF->getEventSequenceId(),
                "CI_005",
                ADM_OCS_EVENT_LEVEL_TRACE,
                "Submit check request to platform",
                "Submit check request to platform",
                time(NULL));
        }
        retVal = impl->implF->submitPost_F(impl->eBusinessCommType, reqMessageBody, &respMessageBody, &rspStatusCode);
        if (ADM_ECODE_SUCCESSED != retVal) {
            if (NULL != impl->notifyF && NULL != impl->notifyF->getEventSequenceId) {
                log_i("checking event.Submit response failed");
                (void)adm_cvs_addEvent(impl->notifyF,
                    impl->notifyF->getEventSequenceId(),
                    "CI_006",
                    ADM_OCS_EVENT_LEVEL_ERROR,
                    "Submit response failed",
                    "Submit response failed",
                    time(NULL));
            }
            log_e("http(s) post failed");
            break;
        }
        if (NULL != impl->notifyF && NULL != impl->notifyF->getEventSequenceId) {
            log_i("checking event.Submit response succeeded");
            (void)adm_cvs_addEvent(impl->notifyF,
                impl->notifyF->getEventSequenceId(),
                "CI_007",
                ADM_OCS_EVENT_LEVEL_TRACE,
                "Submit response succeeded",
                "Submit response succeeded",
                time(NULL));
        }

        if (NULL != impl->notifyF && NULL != impl->notifyF->progressNotifyHmi) {
            impl->notifyF->progressNotifyHmi(
                impl->checkReq->cmdId, "RESP_FROM_SERVER", ADM_CVS_COMMON_CHECK_PROGRESS_TYPE_RESP_FROM_SERVER);
        }

        /*parse resp message body desecret */
        log_i("parse resp message body");
        if (ADM_CVS_CHECK == postType) {
            retVal = impl->implF->parseMessageBody_F(respMessageBody, impl->checkResp);
            if (ADM_ECODE_SUCCESSED != retVal) {
                log_e("parseMessageBody_F error[%d]", retVal);
                if (ADM_ECODE_AES_ECB_DECRYPT_FAILED == retVal && NULL != impl->notifyF
                    && NULL != impl->notifyF->getEventSequenceId) {
                    log_i("checking event.Failed to decrypt check response message");
                    (void)adm_cvs_addEvent(impl->notifyF,
                        impl->notifyF->getEventSequenceId(),
                        "CI_009",
                        ADM_OCS_EVENT_LEVEL_ERROR,
                        "Failed to decrypt check response message",
                        "Failed to decrypt check response message",
                        time(NULL));
                } else if (ADM_ECODE_SIGN_VERIFY_FAILED == retVal && NULL != impl->notifyF
                    && NULL != impl->notifyF->getEventSequenceId) {
                    log_i("checking event.Failed to compare signature check response message");
                    (void)adm_cvs_addEvent(impl->notifyF,
                        impl->notifyF->getEventSequenceId(),
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
            if (NULL != impl->notifyF && NULL != impl->notifyF->getEventSequenceId) {
                log_i("checking event.Successfully decrypt the check response message");
                (void)adm_cvs_addEvent(impl->notifyF,
                    impl->notifyF->getEventSequenceId(),
                    "CI_008",
                    ADM_OCS_EVENT_LEVEL_TRACE,
                    "Successfully decrypt the check response message",
                    "Successfully decrypt the check response message",
                    time(NULL));
                log_i("checking event.Successful compare signature check response message");
                (void)adm_cvs_addEvent(impl->notifyF,
                    impl->notifyF->getEventSequenceId(),
                    "CI_010",
                    ADM_OCS_EVENT_LEVEL_TRACE,
                    "Successful compare signature check response message",
                    "Successful compare signature check response message",
                    time(NULL));
            }
        } else if (ADM_CVS_VEHICLEINFO == postType) {
            retVal = impl->implF->parseVehicleInfoMessageBody_F(respMessageBody, &impl->status, &impl->versionNumber);
            if (ADM_ECODE_SUCCESSED != retVal) {
                log_e("parseVehicleInfoMessageBody_F error[%d]", retVal);
                break;
            }
        }
        else
        {
            log_i("postType = %d", postType);
        }
    } while (FALSE);
    adm_free(reqMessageBody);
    adm_free(respMessageBody);
    // adm_ocs_deleteCheckReqAdapterBody(reqAdapter);
    return retVal;
}