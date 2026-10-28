#include "adm_cvs_reportVehicleInfo.h"
#include "adm_cvs_checkVersionImpl.h"

#include "adm_com_errorCode.h"
#include "adm_cvs_log.h"
#include "adm_string.h"

D32S adm_cvs_reportVehicleInfo(ADM_OCS_CHECK_REQ_ADAPTER_T* checkReq,
    D32S* status,
    DCHAR** extsObjectStr,
    DCHAR** versionNumber)
{
    if (NULL == checkReq) {
        log_e("checkReq input param invalid");
        return ADM_ECODE_INVALID_PARAM;
    }

    D32S retVal = ADM_ECODE_FAILED;
    ADM_CVS_IMPL_T* impl = NULL;
    ADM_OCS_CHECK_RESP_ADAPTER_T* checkResp = NULL;
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
            checkResp = adm_ocs_newCheckRespAdapterBody();
            if (NULL == checkResp) {
                log_e("adm_cvs_newPolicyFieldBody failed");
                retVal = ADM_ECODE_APPLY_MEMORY_FAILED;
                break;
            }
        }

        impl->checkReq = checkReq;
        impl->checkResp = checkResp;
        impl->eBusinessCommType = ADM_OCS_BUSINESS_TYPE_REPORT_VEHICLE_INFO;
        log_i("eBusinessCommType = %d", impl->eBusinessCommType);

        impl->implF->submitPost_F = checkReq->fnBusinessCommCb;
        impl->implF->createMessageBody_F = adm_ocs_checkProtocolAdapter;
        impl->implF->parseVehicleInfoMessageBody_F = adm_ocs_reportVehicleInfoProtocolParse;
        impl->notifyF = NULL;

        log_i("--------call adm_cvs_checkVersionImpl-------------");
        retVal = adm_cvs_checkVersionImpl(ADM_CVS_VEHICLEINFO, impl);
        if (ADM_ECODE_SUCCESSED == retVal) {
            *status = impl->status;
            if (NULL != impl->checkResp->extsObjectStr) {
                *extsObjectStr = adm_strdup(impl->checkResp->extsObjectStr);
            } else {
                *extsObjectStr = NULL;
            }
            if (NULL != impl->versionNumber) {
                *versionNumber = adm_strdup(impl->versionNumber);
            } else {
                *versionNumber = NULL;
            }
        }
        log_i("--------exit adm_cvs_checkVersionImpl-------------");
    } while (FALSE);

    if(NULL != impl)
    {
        adm_cvs_deleteImplFBody(impl->implF);
    }
    adm_cvs_deleteImplBody(impl);
    adm_ocs_deleteCheckRespAdapterBody(checkResp);
    return retVal;
}