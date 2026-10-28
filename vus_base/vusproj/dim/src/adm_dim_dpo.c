#include "adm_dim_log.h"

#include "adm_dim_dpo.h"
#include "adm_com_errorCode.h"
#include "adm_typedefs.h"

#include "adm_memory.h"
#include "adm_string.h"

#include "adm_dim_api.h"
#include "adm_dim_apiImpl.h"

#include "adm_cfg_api.h"
#include "adm_cfg_default.h"


void adm_dim_dpo_getSdkVersion(const D16U instanceId, const D16U timeout,
                               DCHAR **sdkVersion, D32S *errorCode)
{
    log_i("adm_dim_dpo_getSdkVersion begin");

    dim_func_list_t *func = adm_getFuncList();

    if(func == NULL || func->adm_dis_dpo_getSdkVersion_handle == NULL)
    {
        log_e("func or adm_dis_dpo_getSdkVersion_handle pointer invalid");
        return ;
    }

    func->adm_dis_dpo_getSdkVersion_handle(instanceId, timeout, sdkVersion,
                                           errorCode);
    log_i("adm_dim_dpo_getSdkVersion end");
}

D32S adm_dim_dpo_requestRemoteDownloadPackage(const D16U instanceId,
        const D16U timeout, const DCHAR *fileID, const DCHAR *url, const D64S size,
        const DCHAR *hash, const D32S networkRetriesCnt, D32S *errorCode)
{
    log_i("adm_dim_dpo_requestRemoteDownloadPackage begin");

    D32S nRet = ADM_ECODE_FAILED;
    dim_func_list_t *func = adm_getFuncList();

    if(func == NULL
            || func->adm_dis_dpo_requestRemoteDownloadPackage_handle == NULL)
    {
        nRet = ADM_ECODE_INVALID_POINTER;
        log_e("func or adm_dis_dpo_requestRemoteDownloadPackage_handle pointer invalid, nRet[ %d ]",
              nRet);
        return nRet;
    }

    nRet = func->adm_dis_dpo_requestRemoteDownloadPackage_handle(instanceId,
            timeout, fileID, url, size, hash, networkRetriesCnt, errorCode);
    log_i("adm_dim_dpo_requestRemoteDownloadPackage end");

    return nRet;
}


D32S adm_dim_dpo_getRemoteDownloadProgress(const D16U instanceId,
        const D16U timeout, const DCHAR *fileID,
        ADM_DIM_SOA_DPO_DOWNLOAD_PROGRESS_INFO_T **pstDownloadProgressInfo,
        D32S *errorCode)
{
    log_i("adm_dim_dpo_getRemoteDownloadProgress begin");

    D32S nRet = ADM_ECODE_FAILED;
    dim_func_list_t *func = adm_getFuncList();

    if(func == NULL || func->adm_dis_dpo_getRemoteDownloadProgress_handle == NULL)
    {
        nRet = ADM_ECODE_INVALID_POINTER;
        log_e("func or adm_dis_dpo_getRemoteDownloadProgress_handle pointer invalid, nRet[ %d ]",
              nRet);
        return nRet;
    }

    nRet = func->adm_dis_dpo_getRemoteDownloadProgress_handle(instanceId, timeout,
            fileID, pstDownloadProgressInfo, errorCode);
    log_i("adm_dim_dpo_getRemoteDownloadProgress end");

    return nRet;
}


D32S adm_dim_dpo_getRemoteDownloadResult(const D16U instanceId,
        const D16U timeout, const DCHAR *fileID, DCHAR **storePath,
        ADM_DIM_SOA_DPO_DOWNLOAD_RESULT_INFO_T **pstDownloadResultInfo, D32S *errorCode)
{
    log_i("adm_dim_dpo_getRemoteDownloadResult begin");

    D32S nRet = ADM_ECODE_FAILED;
    dim_func_list_t *func = adm_getFuncList();

    if(func == NULL || func->adm_dis_dpo_getRemoteDownloadResult_handle == NULL)
    {
        nRet = ADM_ECODE_INVALID_POINTER;
        log_e("func or adm_dis_dpo_getRemoteDownloadResult_handle pointer invalid, nRet[ %d ]",
              nRet);
        return nRet;
    }

    nRet = func->adm_dis_dpo_getRemoteDownloadResult_handle(instanceId, timeout,
            fileID, storePath, pstDownloadResultInfo, errorCode);
    log_i("adm_dim_dpo_getRemoteDownloadResult end");

    return nRet;
}

D32S adm_dim_dpo_cleanDownloadDir(const D16U instanceId, const D16U timeout, D32S *errorCode)
{
    log_i("adm_dim_dpo_cleanDownloadDir begin");

    D32S nRet = ADM_ECODE_FAILED;
    dim_func_list_t *func = adm_getFuncList();

    if(func == NULL || func->adm_dis_dpo_cleanDownloadDir_handle == NULL)
    {
        nRet = ADM_ECODE_INVALID_POINTER;
        log_e("func or adm_dis_dpo_getRemoteDownloadResult_handle pointer invalid, nRet[ %d ]",
              nRet);
        return nRet;
    }

    nRet = func->adm_dis_dpo_cleanDownloadDir_handle(instanceId, timeout, errorCode);
    log_i("adm_dim_dpo_cleanDownloadDir end");

    return nRet;
}

D32S adm_dim_dpo_interuptDownload(const D16U instanceId, const D16U timeout, D32S actionType, D32S *errorCode)
{
    log_i("adm_dim_dpo_interuptDownload begin");

    D32S nRet = ADM_ECODE_FAILED;
    dim_func_list_t *func = adm_getFuncList();

    if(func == NULL || func->adm_dis_dpo_interuptDownload_handle == NULL)
    {
        nRet = ADM_ECODE_INVALID_POINTER;
        log_e("func or adm_dim_dpo_interuptDownload pointer invalid, nRet[ %d ]",nRet);
        return nRet;
    }

    nRet = func->adm_dis_dpo_interuptDownload_handle(instanceId, timeout, actionType, errorCode);
    log_i("adm_dim_dpo_interuptDownload end");

    return nRet;
}


D32S adm_dim_dpo_set_log_level(const D16U instanceId, const D16U timeout,
                               D32U logLevel, D32S *errorCode)
{
    log_i("adm_dim_dpo_set_log_level begin");

    D32S nRet = ADM_ECODE_FAILED;
    dim_func_list_t *func = adm_getFuncList();

    if(func == NULL || func->adm_dis_dpo_set_log_level_handle == NULL)
    {
        nRet = ADM_ECODE_INVALID_POINTER;
        log_e("func or adm_dis_dpo_set_log_level_handle pointer invalid, nRet[ %d ]",
              nRet);
        return nRet;
    }

    nRet = func->adm_dis_dpo_set_log_level_handle(instanceId, timeout, logLevel, errorCode);
    log_i("adm_dim_dpo_set_log_level end");

    return nRet;
}

D32S adm_dim_dpo_init(ADM_DIM_SOA_DPO_INIT_INFO_T *pstInitInfo, void *priv)
{
    log_i("adm_dim_dpo_init begin");

    log_i("appPriv: %p ", priv);

    D32S nRet = ADM_ECODE_FAILED;
    dim_func_list_t *func = adm_getFuncList();

    if(func == NULL || func->adm_dis_dpo_init_handle == NULL)
    {
        nRet = ADM_ECODE_INVALID_POINTER;
        log_e("func or adm_dim_dpo_init_handle pointer invalid, nRet[ %d ]", nRet);
        return nRet;
    }

    DCHAR *instance = NULL;
    DCHAR *cfgInstances = adm_strdup(adm_cfg_iniParserGetString(ADM_CFG_VUS_UCL_INSTANCEID,  ""));

    DCHAR *subInstanceIdStr = NULL;
    subInstanceIdStr = strsep(&cfgInstances, ",");
    log_i("subInstanceIdStr:%s", subInstanceIdStr);

    while(NULL != subInstanceIdStr)
    {
        log_i("subInstanceIdStr:%s", subInstanceIdStr);
        nRet = func->adm_dis_dpo_init_handle(atoi(subInstanceIdStr), pstInitInfo, priv);
        subInstanceIdStr = strsep(&cfgInstances, ",");
    }

    adm_free(cfgInstances);

    log_i("adm_dim_dpo_init end");

    return nRet;
}

void adm_dim_dpo_deinit(void)
{
    log_i("adm_dim_dpo_deinit begin");

    dim_func_list_t *func = adm_getFuncList();

    if(func == NULL || func->adm_dis_dpo_deinit_handle == NULL)
    {
        log_e("func or adm_dim_dpo_deinit_handle pointer invalid");
        return ;
    }

    DCHAR *cfgInstances = adm_strdup(adm_cfg_iniParserGetString(ADM_CFG_VUS_UCL_INSTANCEID,  ""));

    DCHAR *subInstanceIdStr = NULL;
    subInstanceIdStr = strsep(&cfgInstances, ",");

    while(subInstanceIdStr != NULL)
    {
        log_i("subInstanceIdStr:%s", subInstanceIdStr);
        func->adm_dis_dpo_deinit_handle(atoi(subInstanceIdStr));
        subInstanceIdStr = strsep(&cfgInstances, ",");
    }

    adm_free(cfgInstances);
    log_i("adm_dim_dpo_deinit end");
}

