#include "adm_cvs_baseInteraction.h"
#include "adm_cvs_log.h"
#include "adm_com_errorCode.h"
#include "adm_memory.h"

static D32S adm_cvs_baseInteraction(D64S timestamp,
    DCHAR* sdkVersion,
    DCHAR* extsObjectStr,
    ADM_OCS_BUSINESS_COMM_INTERFACE_CALLBACK_F fnBusinessCommCb,
    const ADM_OCS_BUSINESS_TYPE_E eBusinessType,
    ADM_OCS_BASE_RESP_ADAPTER_T* baseResp)
{
    if (NULL == sdkVersion || NULL == fnBusinessCommCb || NULL == baseResp) {
        log_e("param invalid");
        return ADM_ECODE_INVALID_PARAM;
    }

    ADM_OCS_BASE_REQ_ADAPTER_T* req = NULL;
    DCHAR* adapterBaseReq = NULL;
    DCHAR* adapterPerCheckResp = NULL;
    DSIZE rspStatusCode;
    D32S retVal = ADM_ECODE_SUCCESSED;

    do {
        req = adm_ocs_newBaseReqAdapterBody(timestamp, sdkVersion, extsObjectStr, fnBusinessCommCb);
        if (NULL == req) {
            log_e("adm_ocs_newBaseReqAdapterBody return NULL");
            break;
        }

        retVal = adm_ocs_baseProtocolAdapter(req, &adapterBaseReq);
        if (ADM_ECODE_SUCCESSED != retVal) {
            log_e("adm_ocs_baseProtocolAdapter return error[%d]", retVal);
            break;
        }

        retVal = req->fnBusinessCommCb(eBusinessType, adapterBaseReq, &adapterPerCheckResp, &rspStatusCode);
        if (ADM_ECODE_SUCCESSED != retVal) {
            log_e("fnBusinessCommCb return error[%d]", retVal);
            break;
        }

        retVal = adm_ocs_baseProtocolParse(adapterPerCheckResp, baseResp);
        if (ADM_ECODE_SUCCESSED != retVal) {
            log_e("adm_ocs_baseProtocolParse return error[%d]", retVal);
            break;
        }
    } while (FALSE);
    adm_free(adapterBaseReq);
    adm_free(adapterPerCheckResp);
    adm_ocs_deleteBaseReqAdapterBody(req);
    return retVal;
}

D32S adm_cvs_preCheck(D64S timestamp,
    DCHAR* sdkVersion,
    DCHAR* extsObjectStr,
    ADM_OCS_BUSINESS_COMM_INTERFACE_CALLBACK_F fnBusinessCommCb,
    ADM_OCS_BASE_RESP_ADAPTER_T* baseResp)
{

    return adm_cvs_baseInteraction(
        timestamp, sdkVersion, extsObjectStr, fnBusinessCommCb, ADM_OCS_BUSINESS_TYPE_PERCHECK_VER, baseResp);
}

D32S adm_cvs_getUpgradeHistory(D64S timestamp,
    DCHAR* sdkVersion,
    DCHAR* extsObjectStr,
    ADM_OCS_BUSINESS_COMM_INTERFACE_CALLBACK_F fnBusinessCommCb,
    ADM_OCS_BASE_RESP_ADAPTER_T* baseResp)
{
    return adm_cvs_baseInteraction(
        timestamp, sdkVersion, extsObjectStr, fnBusinessCommCb, ADM_OCS_BUSINESS_TYPE_GETUPGRADEHISTORY, baseResp);
}