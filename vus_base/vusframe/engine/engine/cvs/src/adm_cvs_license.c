#include "adm_cvs_license.h"
#include "adm_cvs_log.h"
#include "adm_com_errorCode.h"
#include "adm_memory.h"

D32S adm_cvs_licenseCheck(const DCHAR* licenseHash,
    D64S timestamp,
    const DCHAR* sdkVersion,
    ADM_OCS_BUSINESS_COMM_INTERFACE_CALLBACK_F fnBusinessCommCb,
    D32S* status,
    DCHAR** respLicenseObjectStr)
{
    if (NULL == sdkVersion || NULL == fnBusinessCommCb) {
        log_e("param invalid");
        return ADM_ECODE_INVALID_PARAM;
    }

    DCHAR* licenseCheckadapterReq = NULL;
    DCHAR* licenseCheckRespStr = NULL;
    DSIZE rspStatusCode;
    D32S retVal = ADM_ECODE_SUCCESSED;

    log_i("adm_cvs_licenseCheck start");
    do {
        retVal = adm_ocs_licenseCheckProtocolAdapter(licenseHash, timestamp, sdkVersion, &licenseCheckadapterReq);
        if (ADM_ECODE_SUCCESSED != retVal) {
            log_e("adm_ocs_licenseCheckProtocolAdapter return error[%d],retVal");
            break;
        }

        retVal = fnBusinessCommCb(
            ADM_OCS_BUSINESS_TYPE_LICENSECHECK, licenseCheckadapterReq, &licenseCheckRespStr, &rspStatusCode);
        if (ADM_ECODE_SUCCESSED != retVal) {
            log_e("fnBusinessCommCb return error[%d]", retVal);
            break;
        }
        retVal = adm_ocs_licenseCheckProtocolParse(licenseCheckRespStr, status, respLicenseObjectStr);
        if (ADM_ECODE_SUCCESSED != retVal) {
            log_e("adm_ocs_licenseCheckProtocolParse return error[%d]", retVal);
            break;
        }
    } while (FALSE);
    adm_free(licenseCheckadapterReq);
    adm_free(licenseCheckRespStr);
    log_i("adm_cvs_licenseCheck end");
    return retVal;
}

D32S adm_cvs_licenseStateReport(const DCHAR* licenseHash,
    D64S timestamp,
    const DCHAR* sdkVersion,
    const DCHAR* state,
    const DCHAR* reason,
    const DCHAR* extsObjectStr,
    ADM_OCS_BUSINESS_COMM_INTERFACE_CALLBACK_F fnBusinessCommCb,
    D32S* status,
    DCHAR** licenseStateReportRespObjectStr)
{
    if (NULL == sdkVersion || NULL == state || NULL == fnBusinessCommCb) {
        log_e("input param invalid");
        return ADM_ECODE_INVALID_PARAM;
    }

    ADM_OCS_LICENSE_STATE_REPORT_REQ_ADAPTER_T* licenseStateReportReq = NULL;
    DCHAR* licenseStateReportReqStr = NULL;
    DCHAR* licenseStateReportRespStr = NULL;
    DCHAR* licenseStateReportRespTmp = NULL;
    DSIZE rspStatusCode;
    D32S retVal = ADM_ECODE_SUCCESSED;
    D32S respStatus;

    log_i("adm_cvs_licenseStateReport start");
    do {
        licenseStateReportReq = adm_ocs_newLicenseStateReportReqAdapterbody(
            licenseHash, timestamp, sdkVersion, state, reason, extsObjectStr);
        if (NULL == licenseStateReportReq) {
            retVal = ADM_ECODE_NO_MEMORY;
            log_e("licenseStateReportReq return failed");
            break;
        }
        retVal = adm_ocs_licenseReportProtocolAdapter(licenseStateReportReq, &licenseStateReportReqStr);
        if (ADM_ECODE_SUCCESSED != retVal) {
            log_e("adm_ocs_licenseReportProtocolAdapter return error[%d]", retVal);
            break;
        }
        retVal = fnBusinessCommCb(ADM_OCS_BUSINESS_TYPE_LICENSESTATEREPORT,
            licenseStateReportReqStr,
            &licenseStateReportRespStr,
            &rspStatusCode);
        if (ADM_ECODE_SUCCESSED != retVal) {
            log_e("fnBusinessCommCb return error[%d]", retVal);
            break;
        }
        retVal = adm_ocs_licenseStateReportProtocolParse(
            licenseStateReportRespStr, &respStatus, &licenseStateReportRespTmp);
        if (ADM_ECODE_SUCCESSED != retVal) {
            log_e("adm_ocs_baseProtocolParse return error[%d]", retVal);
            break;
        }
        *licenseStateReportRespObjectStr = licenseStateReportRespTmp;
        licenseStateReportRespTmp = NULL;

    } while (FALSE);
    adm_free(licenseStateReportReqStr);
    adm_free(licenseStateReportRespStr);
    log_i("adm_cvs_licenseStateReport end");
    return retVal;
}
