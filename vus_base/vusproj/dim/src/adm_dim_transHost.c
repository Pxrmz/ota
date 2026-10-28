#include "adm_dim_log.h"

#include "adm_dim_transHost.h"
#include "adm_com_errorCode.h"
#include "adm_typedefs.h"
#include "adm_memory.h"
#include "adm_string.h"

#include "adm_com_lib.h"
//#include "adm_dim_soa.h"

#include "adm_dim_api.h"
#include "adm_dim_apiImpl.h"

#include "adm_cfg_api.h"
#include "adm_cfg_default.h"

void adm_dim_trans_getSdkVersion(const D16U instanceId, const D16U timeout,
                                 DCHAR **sdkVersion, D32S *errorCode)
{
    log_i("adm_dim_trans_getSdkVersion begin");

    dim_func_list_t *func = adm_getFuncList();

    if(func == NULL || func->adm_dis_trans_getSdkVersion_handle == NULL)
    {
        log_e("func or adm_dis_trans_getSdkVersion_handle pointer invalid");
        return;
    }

    func->adm_dis_trans_getSdkVersion_handle(instanceId, timeout, sdkVersion,
            errorCode);
    log_i("adm_dim_trans_getSdkVersion end");

    return;

}

D32S adm_dim_trans_requestTransferDownloadFromHost(const D16U instanceId,
        const D16U timeout, const DCHAR *fileID,
        const ADM_DIM_SOA_TRANS_FILE_TYPE_E eFileType, const DCHAR *url,
        DCHAR *filename, const D64S size, const DCHAR *hash, D32S *errorCode)
{
    log_i("adm_dim_trans_requestTransferDownloadFromHost begin");
    D32S nRet = ADM_ECODE_FAILED;
    dim_func_list_t *func = adm_getFuncList();

    if(func == NULL
            || func->adm_dis_trans_requestTransferDownloadFromHost_handle == NULL)
    {
        nRet = ADM_ECODE_INVALID_POINTER;
        log_e("func or adm_dim_trans_requestTransferDownloadFromHost_handle pointer invalid, nRet[ %d ]",
              nRet);
        return nRet;
    }

    nRet = func->adm_dis_trans_requestTransferDownloadFromHost_handle(instanceId,
            timeout, fileID, eFileType, url, filename, size, hash, errorCode);
    log_i("adm_dim_trans_requestTransferDownloadFromHost end");

    return nRet;

}

D32S adm_dim_trans_requestTransferSyncDownloadFromHost(const D16U instanceId,
                                                       const D16U timeout, const DCHAR *fileID, const DCHAR *url,
                                                       DCHAR *filename, const D64S size, const DCHAR *hash, D32S *errorCode)
{
    log_i("adm_dim_trans_requestTransferSyncDownloadFromHost begin");
    D32S nRet = ADM_ECODE_FAILED;
    dim_func_list_t *func = adm_getFuncList();

    if (func == NULL || func->adm_dis_trans_requestTransferSyncDownloadFromHost_handle == NULL)
    {
        nRet = ADM_ECODE_INVALID_POINTER;
        log_e("func or adm_dim_trans_requestTransferSyncDownloadFromHost pointer invalid, nRet[ %d ]",
              nRet);
        return nRet;
    }

    nRet = func->adm_dis_trans_requestTransferSyncDownloadFromHost_handle(instanceId,
                                                                          timeout, fileID, url, filename, size, hash, errorCode);
    log_i("adm_dim_trans_requestTransferSyncDownloadFromHost end");

    return nRet;
}

D32S adm_dim_trans_queryProgressDownloadFromHost(const D16U instanceId,
        const D16U timeout, const DCHAR *fileID,
        ADM_DIM_SOA_TRANS_PROGRESS_INFO_T **ppstTransProgress,
        D32S *errorCode)
{
    log_i("adm_dim_trans_queryProgressDownloadFromHost begin");
    D32S nRet = ADM_ECODE_FAILED;
    dim_func_list_t *func = adm_getFuncList();

    if(func == NULL
            || func->adm_dis_trans_queryTransferDownloadFromHostProgress_handle == NULL)
    {
        nRet = ADM_ECODE_INVALID_POINTER;
        log_e("func or adm_dim_trans_queryProgressDownloadFromHost pointer invalid, nRet[ %d ]",
              nRet);
        return nRet;
    }

    nRet = func->adm_dis_trans_queryTransferDownloadFromHostProgress_handle(
               instanceId, timeout, fileID, ppstTransProgress, errorCode);
    log_i("adm_dim_trans_queryProgressDownloadFromHost end");

    return nRet;

}

D32S adm_dim_trans_queryResultDownloadFromHost(const D16U instanceId,
        const D16U timeout, const DCHAR *fileID, DCHAR **storagePath,
        ADM_DIM_SOA_TRANS_RESULT_INFO_T **ppstTransResult, D32S *errorCode)
{
    log_i("adm_dim_trans_queryResultDownloadFromHost begin");
    D32S nRet = ADM_ECODE_FAILED;
    dim_func_list_t *func = adm_getFuncList();

    if(func == NULL
            || func->adm_dis_trans_queryTransferDownloadFromHostResult_handle == NULL)
    {
        nRet = ADM_ECODE_INVALID_POINTER;
        log_e("func or adm_dim_trans_queryResultDownloadFromHost pointer invalid, nRet[ %d ]",
              nRet);
        return nRet;
    }

    nRet = func->adm_dis_trans_queryTransferDownloadFromHostResult_handle(
               instanceId, timeout, fileID, storagePath, ppstTransResult, errorCode);
    log_i("adm_dim_trans_queryResultDownloadFromHost end");

    return nRet;

}

D32S adm_dim_trans_requestTransferUploadToHost(const D16U instanceId,
        const D16U timeout, const DCHAR *fileID,
        const ADM_DIM_SOA_TRANS_FILE_TYPE_E eFileType, const DCHAR *url,
        DCHAR **filename, DCHAR **hash, D64S *size, D32S *errorCode)
{
    log_i("adm_dim_trans_requestTransferUploadToHost begin");
    D32S nRet = ADM_ECODE_FAILED;
    dim_func_list_t *func = adm_getFuncList();

    if(func == NULL
            || func->adm_dis_trans_requestTransferUploadToHost_handle == NULL)
    {
        nRet = ADM_ECODE_INVALID_POINTER;
        log_e("func or adm_dim_trans_requestTransferUploadToHost pointer invalid, nRet[ %d ]",
              nRet);
        return nRet;
    }

    nRet = func->adm_dis_trans_requestTransferUploadToHost_handle(instanceId,
            timeout, fileID, eFileType, url, filename, hash, size, errorCode);
    log_i("adm_dim_trans_requestTransferUploadToHost end");

    return nRet;

}

D32S adm_dim_trans_queryProgressUploadToHost(const D16U instanceId,
        const D16U timeout, const DCHAR *fileID,
        ADM_DIM_SOA_TRANS_PROGRESS_INFO_T **ppstTransProgress,
        D32S *errorCode)
{
    log_i("adm_dim_trans_queryProgressUploadToHost begin");
    D32S nRet = ADM_ECODE_FAILED;
    dim_func_list_t *func = adm_getFuncList();

    if(func == NULL
            || func->adm_dis_trans_queryTransferUploadToHostProgress_handle == NULL)
    {
        nRet = ADM_ECODE_INVALID_POINTER;
        log_e("func or adm_dis_trans_queryTransferUploadToHostProgress_handle pointer invalid, nRet[ %d ]",
              nRet);
        return nRet;
    }

    nRet = func->adm_dis_trans_queryTransferUploadToHostProgress_handle(instanceId,
            timeout, fileID, ppstTransProgress, errorCode);
    log_i("adm_dim_trans_queryProgressUploadToHost end");

    return nRet;

}

D32S adm_dim_trans_queryResultUploadToHost(const D16U instanceId,
        const D16U timeout, const DCHAR *fileID, DCHAR **filename,
        ADM_DIM_SOA_TRANS_RESULT_INFO_T **ppstTransResult, D32S *errorCode)
{
    log_i("adm_dim_trans_queryResultUploadToHost begin");
    D32S nRet = ADM_ECODE_FAILED;
    dim_func_list_t *func = adm_getFuncList();

    if(func == NULL
            || func->adm_dis_trans_queryTransferUploadToHostResult_handle == NULL)
    {
        nRet = ADM_ECODE_INVALID_POINTER;
        log_e("func or adm_dim_trans_queryResultUploadToHost_handle pointer invalid, nRet[ %d ]",
              nRet);
        return nRet;
    }

    nRet = func->adm_dis_trans_queryTransferUploadToHostResult_handle(instanceId,
            timeout, fileID, filename, ppstTransResult, errorCode);
    log_i("adm_dim_trans_queryResultUploadToHost end");

    return nRet;

}

D32S adm_dim_trans_set_log_level(const D16U instanceId, const D16U timeout,
                                 D32U logLevel, D32S *errorCode)
{
    log_i("adm_dim_trans_set_log_level begin");

    D32S nRet = ADM_ECODE_FAILED;
    dim_func_list_t *func = adm_getFuncList();

    if(func == NULL || func->adm_dis_trans_set_log_level_handle == NULL)
    {
        nRet = ADM_ECODE_INVALID_POINTER;
        log_e("func or adm_dis_trans_set_log_level_handle pointer invalid, nRet[ %d ]",
              nRet);
        return nRet;
    }

    nRet = func->adm_dis_trans_set_log_level_handle(instanceId, timeout, logLevel,
            errorCode);
    log_i("adm_dim_trans_set_log_level end");

    return nRet;
}


D32S adm_dim_transferOnHost_init(const ADM_DIM_SOA_TRANS_HOST_INIT_INFO_T
                                 *initInfo, void *priv)
{
    log_i("adm_dim_transferOnHost_init begin");
    log_i("appPriv: %p ", priv);

    D32S nRet = ADM_ECODE_FAILED;
    dim_func_list_t *func = adm_getFuncList();

    if(func == NULL || func->adm_dis_transferOnHost_init_handle == NULL)
    {
        nRet = ADM_ECODE_INVALID_POINTER;
        log_e("func or adm_dim_transferOnHost_init_handle pointer invalid, nRet[ %d ]",
              nRet);
        return nRet;
    }

    DCHAR *cfgInstances = adm_strdup(adm_cfg_iniParserGetString(ADM_CFG_VUS_UCL_INSTANCEID,  ""));

    DCHAR *subInstanceIdStr = NULL;
    subInstanceIdStr = strsep(&cfgInstances, ",");
    log_i("subInstanceIdStr:%s", subInstanceIdStr);

    while(NULL != subInstanceIdStr)
    {
        log_i("subInstanceIdStr:%s", subInstanceIdStr);
        nRet = func->adm_dis_transferOnHost_init_handle(atoi(subInstanceIdStr), initInfo, priv);
        subInstanceIdStr = strsep(&cfgInstances, ",");
    }

    adm_free(cfgInstances);

    log_i("adm_dim_transferOnHost_init end");

    return nRet;
}

void adm_dim_transferOnHost_deinit(void)
{
    log_i("adm_dim_transferOnHost_deinit begin");

    dim_func_list_t *func = adm_getFuncList();

    if(func == NULL || func->adm_dis_transferOnHost_deinit_handle == NULL)
    {
        log_e("func or adm_dim_transferOnHost_deinit_handle pointer invalid");
        return ;
    }

    DCHAR *instance = NULL;
    DCHAR *cfgInstances = adm_strdup(adm_cfg_iniParserGetString(ADM_CFG_VUS_UCL_INSTANCEID,  ""));

    instance = strsep(&cfgInstances, ",");

    while(instance != NULL)
    {
        func->adm_dis_transferOnHost_deinit_handle(atoi(instance));
        instance = strsep(&cfgInstances, ",");
    }

    adm_free(cfgInstances);
    log_i("adm_dim_transferOnHost_deinit end");
}

D32S adm_dim_trans_switchTransMode(const D32S switchType,
                                   const D16U uiInstanceId,
                                   const D16U uiTimeout, D32S* pErrorcode)
{
    log_i("adm_dim_trans_switchTransMode begin");
    D32S nRet = ADM_ECODE_FAILED;
    dim_func_list_t *func = adm_getFuncList();

    if (func == NULL || func->adm_dis_trans_switchTransMode_handle == NULL)
    {
        nRet = ADM_ECODE_INVALID_POINTER;
        log_e("func or adm_dim_trans_switchTransMode_handle pointer invalid, nRet[ %d ]",
              nRet);
        return nRet;
    }

    nRet = func->adm_dis_trans_switchTransMode_handle(switchType, uiInstanceId, uiTimeout, pErrorcode);
    log_i("adm_dim_trans_switchTransMode end");

    return nRet;
}