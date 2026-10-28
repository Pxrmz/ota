#include "adm_dim_log.h"
#include "adm_dim_ucl.h"
#include "adm_com_errorCode.h"
#include "adm_typedefs.h"

#include "adm_memory.h"
#include "adm_string.h"

#include "adm_soa_cfg.h"
#include "adm_com_lib.h"
#include "adm_dim_apiImpl.h"

#include "adm_cfg_api.h"
#include "adm_cfg_default.h"
#include "pub/inc/adm_pub_keylog.h"

D32S adm_dim_ucl_getSdkVersion(const D16U instanceId, const D16U timeout,
                               DCHAR** sdkVersion, D32S* errorCode)
{
    dim_func_list_t* func = adm_getFuncList();
    D32S nRet = ADM_ECODE_FAILED;

    if(NULL == func)
    {
        log_i("adm_getFuncList =NULL\r\n");
        return nRet;
    }

    if(NULL == func->adm_dis_ucl_getSdkVersion_handle)
    {
        log_i("func->adm_dim_ucl_getSdkVersion_handle == NULL\r\n");
        return nRet;
    }

    nRet = func->adm_dis_ucl_getSdkVersion_handle(instanceId, timeout, sdkVersion,
            errorCode);
    log_i("call adm_dim_ucl_getSdkVersion_handle success\r\n");
    return nRet;
}

D32S adm_dim_ucl_readPartInformation(const D16U instanceId, const D16U timeout,
                                     const ADM_DIM_SOA_ECU_IDEN_INFO_T* pstEcuIdenInfo,
                                     const ADM_DIM_SOA_DID_DATA_ARRAY_T* didArrayRequest,
                                     ADM_DIM_SOA_DID_DATA_ARRAY_T** didListResponse, D32S* errorCode)
{
    log_i("adm_dim_ucl_readPartInformation begin");
    D32S nRet = ADM_ECODE_FAILED;
    dim_func_list_t* func = adm_getFuncList();

    if(func == NULL || func->adm_dis_ucl_readPartInformation_handle == NULL)
    {
        nRet = ADM_ECODE_INVALID_POINTER;
        log_e("func or adm_dim_ucl_readPartInformation_handle pointer invalid, nRet[ %d ]",
              nRet);
        return nRet;
    }

    nRet = func->adm_dis_ucl_readPartInformation_handle(instanceId,
            timeout,
            pstEcuIdenInfo,
            didArrayRequest,
            didListResponse,
            errorCode);
    log_i("adm_dim_ucl_readPartInformation end");
    return nRet;
}

D32S adm_dim_ucl_readSnInfo(const D16U instanceId, const D16U timeout, DCHAR** ppSnInfo, D32S* pErrorCode)
{
    log_i("adm_dim_ucl_readSnInfo begin");
    D32S nRet = ADM_ECODE_FAILED;
    DCHAR aDescription[128] = {0};
    dim_func_list_t* func = adm_getFuncList();

    if(func == NULL || func->adm_dis_ucl_readSnInfo_handle == NULL)
    {
        nRet = ADM_ECODE_INVALID_POINTER;
        log_e("func or adm_dis_ucl_readSnInfo_handle pointer invalid, nRet[ %d ]",
              nRet);
        return nRet;
    }

    nRet = func->adm_dis_ucl_readSnInfo_handle(instanceId,
            timeout,
            ppSnInfo,
            pErrorCode);

    if (ADM_IS_NULL(*ppSnInfo))
    {
        adm_snprintf(aDescription, sizeof(aDescription), "Sn is null,errorCode=[%d] ", *pErrorCode);
        adm_pub_setKeylogString(ADM_PUB_KEYLOG_ID_004, nRet, aDescription);
    }
    else
    {
        adm_snprintf(aDescription, sizeof(aDescription), "Sn=[%s],errorCode=[%d] ", ADM_COM_STR_SAFE_OUTPUT(*ppSnInfo), *pErrorCode);
        adm_pub_setKeylogString(ADM_PUB_KEYLOG_ID_003, nRet, aDescription);
    }

    log_i("adm_dim_ucl_readSnInfo end");
    return nRet;
}

D32S adm_dim_ucl_parallelCollectEcu(const D16U instanceId,
                                    const D16U timeout,
                                    const ADM_DIM_SOA_ECU_IDEN_INFO_T* pstEcuIdenInfo,
                                    const ADM_DIM_SOA_DID_DATA_ARRAY_T* didArrayRequest,
                                    D32S* errorCode)
{
    log_i("adm_dim_ucl_parallelCollectEcu begin");
    D32S nRet = ADM_ECODE_FAILED;
    dim_func_list_t* func = adm_getFuncList();

    if(func == NULL || func->adm_dis_ucl_parallelCollectEcu_handle == NULL)
    {
        nRet = ADM_ECODE_INVALID_POINTER;
        log_e("func or adm_dis_ucl_parallelCollectEcu_handle pointer invalid, nRet[ %d ]",
              nRet);
        return nRet;
    }

    nRet = func->adm_dis_ucl_parallelCollectEcu_handle(instanceId,
            timeout,
            pstEcuIdenInfo,
            didArrayRequest,
            errorCode);
    log_i("adm_dim_ucl_parallelCollectEcu end");
    return nRet;
}


D32S adm_dim_ucl_requestDoUpgrade(const D16U instanceId, const D16U timeout,
                                  const ADM_DIM_SOA_UCL_UPGRADE_INFO_ARRAY_T* upgradeInfoList, 
                                  DBOOL bIsControlActive, D32S* errorCode)
{
    log_i("adm_dim_ucl_requestDoUpgrade begin");
    D32S nRet = ADM_ECODE_FAILED;
    DCHAR aDescription[128] = {0};
    dim_func_list_t* func = adm_getFuncList();

    if(func == NULL || func->adm_dis_ucl_requestDoUpgrade_handle == NULL)
    {
        nRet = ADM_ECODE_INVALID_POINTER;
        log_e("func or adm_dim_ucl_requestDoUpgrade_handle pointer invalid, nRet[ %d ]",
              nRet);
        return nRet;
    }

    nRet = func->adm_dis_ucl_requestDoUpgrade_handle(instanceId, timeout,
        upgradeInfoList, bIsControlActive, errorCode);

    adm_snprintf(aDescription, sizeof(aDescription), "InstanceId[%d],errorCode=[%d] ", instanceId, *errorCode);
    if (ADM_IS_ERR(nRet))
    {
        adm_pub_setKeylogString(ADM_PUB_KEYLOG_ID_010, nRet, aDescription);
    }
    else
    {
        adm_pub_setKeylogString(ADM_PUB_KEYLOG_ID_009, nRet, aDescription);
    }

    log_i("adm_dim_ucl_requestDoUpgrade end");
    return nRet;
}

D32S adm_dim_ucl_requestDoUpgrade_icc(const D16U instanceId, const D16U timeout,
                                      const ADM_DIM_SOA_UCL_UPGRADE_INFO_ARRAY_T* upgradeInfoList, D32S installType, D32S bIsRollback, D32S* errorCode)
{
    log_i("adm_dim_ucl_requestDoUpgrade_icc begin");
    D32S nRet = ADM_ECODE_FAILED;
    DCHAR aDescription[128] = {0};
    dim_func_list_t* func = adm_getFuncList();

    if(func == NULL || func->adm_dis_ucl_requestDoUpgrade_icc_handle == NULL)
    {
        nRet = ADM_ECODE_INVALID_POINTER;
        log_e("func or adm_dis_ucl_requestDoUpgrade_icc_handle pointer invalid, nRet[ %d ]",
              nRet);
        return nRet;
    }

    //TODO:maxiangyang 时间先默认20min,后续改成异步通知
    nRet = func->adm_dis_ucl_requestDoUpgrade_icc_handle(instanceId, 60 * 20,
            upgradeInfoList, installType, bIsRollback, errorCode);

    adm_snprintf(aDescription, sizeof(aDescription), "InstanceId[%d],errorCode=[%d] ", instanceId, *errorCode);
    if (ADM_IS_ERR(nRet))
    {
        adm_pub_setKeylogString(ADM_PUB_KEYLOG_ID_016, nRet, aDescription);
    }
    else
    {
        adm_pub_setKeylogString(ADM_PUB_KEYLOG_ID_015, nRet, aDescription);
    }
    
    log_i("adm_dim_ucl_requestDoUpgrade_icc end");
    return nRet;
}

D32S adm_dim_ucl_requestSetChangeSlot_icc(const D16U instanceId, const D16U timeout, D32S* errorCode)
{
    log_i("adm_dim_ucl_requestSetChangeSlot_icc begin");
    D32S nRet = ADM_ECODE_FAILED;
    dim_func_list_t* func = adm_getFuncList();

    if(func == NULL || func->adm_dis_ucl_requestSetChangeSlot_icc_handle == NULL)
    {
        nRet = ADM_ECODE_INVALID_POINTER;
        log_e("func or adm_dis_ucl_requestSetChangeSlot_icc_handle pointer invalid, nRet[ %d ]",
              nRet);
        return nRet;
    }

    nRet = func->adm_dis_ucl_requestSetChangeSlot_icc_handle(instanceId, timeout, errorCode);
    log_i("adm_dim_ucl_requestSetChangeSlot_icc end");
    return nRet;
}

D32S adm_dim_ucl_requestUpdateAndReboot(const D16U instanceId, const D16U timeout, D32S* errorCode)
{
    log_i("adm_dim_ucl_requestUpdateAndReboot begin");
    D32S nRet = ADM_ECODE_FAILED;
    dim_func_list_t* func = adm_getFuncList();

    if(func == NULL || func->adm_dis_ucl_requestUpdateAndReboot_handle == NULL)
    {
        nRet = ADM_ECODE_INVALID_POINTER;
        log_e("func or adm_dis_ucl_requestUpdateAndReboot_handle pointer invalid, nRet[ %d ]",
              nRet);
        return nRet;
    }

    nRet = func->adm_dis_ucl_requestUpdateAndReboot_handle(instanceId, timeout, errorCode);
    log_i("adm_dim_ucl_requestUpdateAndReboot end");
    return nRet;
}

D32S adm_dim_ucl_queryUpgradeProgress(const D16U instanceId, const D16U timeout,
                                      const ADM_DIM_SOA_ECU_IDEN_INFO_T* pstEcuIdenInfo, D16U* progress,
                                      D32S* errorCode)
{
    log_i("adm_dim_ucl_queryUpgradeProgress begin");
    D32S nRet = ADM_ECODE_FAILED;
    dim_func_list_t* func = adm_getFuncList();

    if(func == NULL || func->adm_dis_ucl_queryUpgradeProgress_handle == NULL)
    {
        nRet = ADM_ECODE_INVALID_POINTER;
        log_e("func or adm_dim_ucl_queryUpgradeProgress_handle pointer invalid, nRet[ %d ]",
              nRet);
        return nRet;
    }

    nRet = func->adm_dis_ucl_queryUpgradeProgress_handle(instanceId, timeout,
            pstEcuIdenInfo, progress, errorCode);
    log_i("adm_dim_ucl_queryUpgradeProgress end");
    return nRet;
}

D32S adm_dim_ucl_queryUpgradeResult(const D16U instanceId, const D16U timeout,
                                    const ADM_DIM_SOA_ECU_IDEN_INFO_T* pstEcuIdenInfo,
                                    ADM_DIM_SOA_UCL_UPGRADE_RESULT_INFO_T** pstUpgradeResultInfo, D32S* errorCode)
{
    log_i("adm_dim_ucl_queryUpgradeResult begin");
    D32S nRet = ADM_ECODE_FAILED;
    DCHAR aDescription[128] = {0};
    dim_func_list_t* func = adm_getFuncList();

    if(func == NULL || func->adm_dis_ucl_queryUpgradeResult_handle == NULL)
    {
        nRet = ADM_ECODE_INVALID_POINTER;
        log_e("func or adm_dim_ucl_queryUpgradeResult_handle pointer invalid, nRet[ %d ]",
              nRet);
        return nRet;
    }

    nRet = func->adm_dis_ucl_queryUpgradeResult_handle(instanceId, timeout,
            pstEcuIdenInfo, pstUpgradeResultInfo, errorCode);

    adm_snprintf(aDescription, sizeof(aDescription), "InstanceId[%d],errorCode=[%d] ", instanceId, *errorCode);
    if (ADM_IS_ERR(nRet))
    {
        adm_pub_setKeylogString(ADM_PUB_KEYLOG_ID_012, nRet, aDescription);
    }
    else
    {
        adm_pub_setKeylogString(ADM_PUB_KEYLOG_ID_011, nRet, aDescription);
    }


    log_i("adm_dim_ucl_queryUpgradeResult end");
    return nRet;
}

D32S adm_dim_ucl_requestDecryptPackage(const D16U instanceId,
                                       const D16U timeout, const DCHAR* fileID, const DCHAR* filePath,
                                       const DCHAR* decryptKey, const D64S decryptSegLength, const DCHAR* hash,
                                       D32S* errorCode)
{
    log_i("adm_dim_ucl_requestDecryptPackage begin");
    D32S nRet = ADM_ECODE_FAILED;
    dim_func_list_t* func = adm_getFuncList();

    if(func == NULL || func->adm_dis_ucl_requestDecryptPackage_handle == NULL)
    {
        nRet = ADM_ECODE_INVALID_POINTER;
        log_e("func or adm_dim_ucl_requestDecryptPackage_handle pointer invalid, nRet[ %d ]",
              nRet);
        return nRet;
    }

    nRet = func->adm_dis_ucl_requestDecryptPackage_handle(instanceId, timeout,
            fileID, filePath,
            decryptKey, decryptSegLength, hash, errorCode);
    log_i("adm_dim_ucl_requestDecryptPackage end");
    return nRet;
}

D32S adm_dim_ucl_getDecryptPackageProgress(const D16U instanceId,
        const D16U timeout, const DCHAR* fileID,
        ADM_DIM_SOA_UCL_DECRYPT_PROGRESS_INFO_T** pstDecryptProgressInfo,
        D32S* errorCode)
{
    log_i("adm_dim_ucl_getDecryptPackageProgress begin");
    D32S nRet = ADM_ECODE_FAILED;
    dim_func_list_t* func = adm_getFuncList();

    if(func == NULL || func->adm_dis_ucl_getDecryptPackageProgress_handle == NULL)
    {
        nRet = ADM_ECODE_INVALID_POINTER;
        log_e("func or adm_dim_ucl_getDecryptPackageProgress_handle pointer invalid, nRet[ %d ]",
              nRet);
        return nRet;
    }

    nRet = func->adm_dis_ucl_getDecryptPackageProgress_handle(instanceId, timeout,
            fileID, pstDecryptProgressInfo, errorCode);
    log_i("adm_dim_ucl_getDecryptPackageProgress end");
    return nRet;
}

D32S adm_dim_ucl_getDecryptPackageResult(const D16U instanceId,
        const D16U timeout, const DCHAR* fileID, DCHAR** storagePath,
        ADM_DIM_SOA_UCL_DECRYPT_RESULT_INFO_T** pstDecryptResultInfo, D32S* errorCode)
{
    log_i("adm_dim_ucl_getDecryptPackageResult begin");
    D32S nRet = ADM_ECODE_FAILED;
    dim_func_list_t* func = adm_getFuncList();

    if(func == NULL || func->adm_dis_ucl_getDecryptPackageResult_handle == NULL)
    {
        nRet = ADM_ECODE_INVALID_POINTER;
        log_e("func or adm_dis_ucl_getDecryptPackageResult_handle pointer invalid, nRet[ %d ]",
              nRet);
        return nRet;
    }

    nRet = func->adm_dis_ucl_getDecryptPackageResult_handle(instanceId, timeout,
            fileID, storagePath, pstDecryptResultInfo, errorCode);
    log_i("adm_dim_ucl_getDecryptPackageResult end");
    return nRet;
}


D32S adm_dim_ucl_requestVerifyHash(const D16U instanceId, const D16U timeout,
                                   const DCHAR* fileID, const DCHAR* filePath,
                                   const ADM_DIM_SOA_UCL_HASH_TYPE_E eHashType, const DCHAR* hash, D32S* errorCode)
{
    log_i("adm_dim_ucl_requestVerifyHash begin");
    D32S nRet = ADM_ECODE_FAILED;
    dim_func_list_t* func = adm_getFuncList();

    if(func == NULL || func->adm_dis_ucl_requestVerifyHash_handle == NULL)
    {
        nRet = ADM_ECODE_INVALID_POINTER;
        log_e("func or adm_dim_ucl_requestVerifyHash_handle pointer invalid, nRet[ %d ]",
              nRet);
        return nRet;
    }

    nRet = func->adm_dis_ucl_requestVerifyHash_handle(instanceId, timeout, fileID,
            filePath, eHashType, hash, errorCode);
    log_i("adm_dim_ucl_requestVerifyHash end");
    return nRet;
}

D32S adm_dim_ucl_verifySpaceIsEnouth(const D16U instanceId, const D16U timeout,
                                     const D64S needSize, D64S* realSize, D32S* errorCode)
{
    log_i("adm_dim_ucl_verifySpaceIsEnouth begin");
    D32S nRet = ADM_ECODE_FAILED;
    dim_func_list_t* func = adm_getFuncList();

    if(func == NULL || func->adm_dis_ucl_verifySpaceIsEnouth_handle == NULL)
    {
        nRet = ADM_ECODE_INVALID_POINTER;
        log_e("func or adm_dis_ucl_verifySpaceIsEnouth_handle pointer invalid, nRet[ %d ]", nRet);
        return nRet;
    }

    nRet = func->adm_dis_ucl_verifySpaceIsEnouth_handle(instanceId, timeout, needSize, realSize, errorCode);
    log_i("adm_dim_ucl_verifySpaceIsEnouth end");

    return nRet;
}

D32S adm_dim_ucl_request_Verify_Sign(const D16U instanceId,
                                     const D16U timeout,
                                     const DCHAR* fileID,
                                     const DCHAR* filePath,
                                     const FILEMETAS_INFO_ARRAY_T* fileMetasList,
                                     DBOOL isUnpick,
                                     D32S* errorCode)
{
    log_i("************ begin ************");
    D32S nRet = ADM_ECODE_FAILED;
    dim_func_list_t* func = adm_getFuncList();

    if(NULL == func || NULL == func->adm_dis_ucl_request_Verify_Sign_handle)
    {
        nRet = ADM_ECODE_INVALID_POINTER;
        log_e("func or adm_dis_ucl_request_Verify_Sign_handle pointer invalid, nRet[ %d ]",
              nRet);
        return nRet;
    }

    nRet = func->adm_dis_ucl_request_Verify_Sign_handle(instanceId, timeout,
            fileID, filePath, fileMetasList, isUnpick, errorCode);
    log_i("************ end ************");
    return nRet;
}

D32S adm_dim_ucl_set_log_level(const D16U instanceId, const D16U timeout,
                               D32U logLevel, D32S* errorCode)
{
    log_i("adm_dim_ucl_set_log_level begin");

    D32S nRet = ADM_ECODE_FAILED;
    dim_func_list_t* func = adm_getFuncList();

    if(func == NULL || func->adm_dis_ucl_set_log_level_handle == NULL)
    {
        nRet = ADM_ECODE_INVALID_POINTER;
        log_e("func or adm_dis_ucl_set_log_level_handle pointer invalid, nRet[ %d ]",
              nRet);
        return nRet;
    }

    nRet = func->adm_dis_ucl_set_log_level_handle(instanceId, timeout, logLevel,
            errorCode);
    log_i("adm_dim_ucl_set_log_level end");

    return nRet;
}

D32S adm_dim_ucl_readVehicleStatus(const D16U instanceId,
                                   const D16U timeout,
                                   const ADM_DIM_SOA_MSGID_DATA_ARRAY_T* msgIdListRequest,
                                   ADM_DIM_SOA_MSGID_DATA_ARRAY_T** msgIdListResponse, D32S* errorCode)
{
    log_i("************ begin ************");
    D32S nRet = ADM_ECODE_FAILED;
    dim_func_list_t* func = adm_getFuncList();

    if(func == NULL || func->adm_dis_ucl_readVehicleStatus_handle == NULL)
    {
        nRet = ADM_ECODE_INVALID_POINTER;
        log_e("func or adm_dis_ucl_readVehicleStatus_handle invalid, nRet[ %d ]", nRet);
        return nRet;
    }

    nRet = func->adm_dis_ucl_readVehicleStatus_handle(instanceId, timeout, msgIdListRequest,
            msgIdListResponse, errorCode);
    log_i("************ end ************");

    return nRet;

}

D32S adm_dim_ucl_setVehicleStatus(const D16U instanceId, const D16U timeout,
                                  const ADM_DIM_SOA_MSGID_DATA_ARRAY_T* msgIdListRequest, D32S* errorCode)
{
    log_i("************ begin ************");
    D32S nRet = ADM_ECODE_FAILED;
    dim_func_list_t* func = adm_getFuncList();

    if(func == NULL || func->adm_dis_ucl_setVehicleStatus_handle == NULL)
    {
        nRet = ADM_ECODE_INVALID_POINTER;
        log_e("func or adm_dis_ucl_setVehicleStatus_handle invalid, nRet[ %d ]", nRet);
        return nRet;
    }

    nRet = func->adm_dis_ucl_setVehicleStatus_handle(instanceId, timeout, msgIdListRequest, errorCode);
    log_i("************ end ************");

    return nRet;
}

D32S adm_dim_ucl_getApkVersion(const D16U instanceId, const D16U timeout,
        const ADM_DIM_SOA_ECU_IDEN_INFO_T* pstEcuIdenInfo, DCHAR** apkVersion, D32S* errorCode)
{
    log_i("adm_dim_ucl_getApkVersion begin");
    D32S nRet = ADM_ECODE_FAILED;
    dim_func_list_t* func = adm_getFuncList();

    if(func == NULL || func->adm_dis_ucl_getApkVersion_handle == NULL)
    {
        nRet = ADM_ECODE_INVALID_POINTER;
        log_e("func or adm_dis_ucl_getApkVersion_handle pointer invalid, nRet[ %d ]", nRet);
        return nRet;
    }

    nRet = func->adm_dis_ucl_getApkVersion_handle(instanceId, timeout, pstEcuIdenInfo, apkVersion,
            errorCode);
    log_i("adm_dim_ucl_getApkVersion end");

    return nRet;
}

D32S adm_dim_ucl_doApkUpgrade(const D16U instanceId, const D16U timeout,
        const ADM_DIM_SOA_ECU_IDEN_INFO_T* pstEcuIdenInfo, DCHAR* pApkPackPath, DCHAR* pApkVersion, D32S* errorCode)
{
    log_i("adm_dim_ucl_doApkUpgrade begin");
    D32S nRet = ADM_ECODE_FAILED;
    dim_func_list_t* func = adm_getFuncList();

    if(func == NULL || func->adm_dis_ucl_doApkSelfUpgrade_handle == NULL)
    {
        nRet = ADM_ECODE_INVALID_POINTER;
        log_e("func or adm_dis_ucl_doApkSelfUpgrade_handle pointer invalid, nRet[ %d ]", nRet);
        return nRet;
    }

    nRet = func->adm_dis_ucl_doApkSelfUpgrade_handle(instanceId, timeout, pstEcuIdenInfo, pApkPackPath, pApkVersion,
            errorCode);
    log_i("adm_dim_ucl_doApkUpgrade end");

    return nRet;
}

D32S adm_dim_ucl_queryApkSelfUpgradeProgress(const D16U instanceId, const D16U timeout,
        const ADM_DIM_SOA_ECU_IDEN_INFO_T* pstEcuIdenInfo, D16U* progress, D32S* errorCode)
{
    log_i("adm_dim_ucl_queryApkSelfUpgradeProgress begin");
    D32S nRet = ADM_ECODE_FAILED;
    dim_func_list_t* func = adm_getFuncList();

    if(func == NULL || func->adm_dis_ucl_queryApkSelfUpgradeProgress_handle == NULL)
    {
        nRet = ADM_ECODE_INVALID_POINTER;
        log_e("func or adm_dis_ucl_queryApkSelfUpgradeProgress_handle pointer invalid, nRet[ %d ]", nRet);
        return nRet;
    }

    nRet = func->adm_dis_ucl_queryApkSelfUpgradeProgress_handle(instanceId, timeout, pstEcuIdenInfo, progress, errorCode);

    log_i("adm_dim_ucl_queryApkSelfUpgradeProgress end");

    return nRet;
}

D32S adm_dim_ucl_queryApkSelfUpgradeResult(const D16U instanceId, const D16U timeout,
        const ADM_DIM_SOA_ECU_IDEN_INFO_T* pstEcuIdenInfo,  ADM_DIM_SOA_UCL_UPGRADE_RESULT_INFO_T** pstUpgradeResultInfo,
        D32S* errorCode)
{
    log_i("adm_dim_ucl_queryApkSelfUpgradeResult begin");
    D32S nRet = ADM_ECODE_FAILED;
    dim_func_list_t* func = adm_getFuncList();

    if(func == NULL || func->adm_dis_ucl_queryApkSelfUpgradeResult_handle == NULL)
    {
        nRet = ADM_ECODE_INVALID_POINTER;
        log_e("func or adm_dis_ucl_queryApkSelfUpgradeResult_handle pointer invalid, nRet[ %d ]", nRet);
        return nRet;
    }

    nRet = func->adm_dis_ucl_queryApkSelfUpgradeResult_handle(instanceId, timeout, pstEcuIdenInfo, pstUpgradeResultInfo, errorCode);

    log_i("adm_dim_ucl_queryApkSelfUpgradeResult end");

    return nRet;
}


/**
 * @brief 同步解密接口
 * @note
 * @param in const D16U usInstanceId        实例ID
 * @param in const D16U usTimeout           超时时间
 * @param in const DCHAR *pcSrcFilePath     待解密的文件路径
 * @param in const DCHAR **ppcDstFilePath   解密后的文件路径
 * @param in const DCHAR *pcDecryptKey      解密密钥
 * @param in const D64S ulDecryptSegLength  每次解密的长度
 * @param out D32S *errorCode 错误码
 * @retval D32S 0 成功，其他失败
 */
D32S adm_dim_ucl_requestSyncDecryptPackage(const D16U usInstanceId,
                                           const D16U usTimeout,
                                           const DCHAR *pcSrcFilePath,
                                           const DCHAR *pcDstFilePath,
                                           const DCHAR *pcDecryptKey,
                                           const D64S ulDecryptSegLength,
                                           D32S *errorCode)
{
    ADM_COM_FUNC_ENTRY();

    D32S nRet = ADM_ECODE_FAILED;
    dim_func_list_t* func = adm_getFuncList();

    if(func == NULL || func->adm_dis_ucl_requestSyncDecryptPackage_handle == NULL)
    {
        nRet = ADM_ECODE_INVALID_POINTER;
        log_e("func or adm_dis_ucl_requestSyncDecryptPackage_handle invalid, nRet[ %d ]", nRet);
        return nRet;
    }

    nRet = func->adm_dis_ucl_requestSyncDecryptPackage_handle(usInstanceId, 
                                                              usTimeout, 
                                                              pcSrcFilePath, 
                                                              pcDstFilePath, 
                                                              pcDecryptKey, 
                                                              ulDecryptSegLength, 
                                                              errorCode);
    ADM_COM_FUNC_EXIT();

    return nRet;
}

D32S adm_dim_ucl_requestRmFile(const D16U usInstanceId, const D16U usTimeout, DCHAR *pFilePath, D32S *errorCode)
{
    ADM_COM_FUNC_ENTRY();

    D32S nRet = ADM_ECODE_FAILED;
    dim_func_list_t* func = adm_getFuncList();

    if(func == NULL || func->adm_dis_ucl_requestRmFile_handle == NULL)
    {
        nRet = ADM_ECODE_INVALID_POINTER;
        log_e("func or adm_dis_ucl_requestRmFile_handle invalid, nRet[ %d ]", nRet);
        return nRet;
    }

    nRet = func->adm_dis_ucl_requestRmFile_handle(usInstanceId, usTimeout, pFilePath, errorCode);
    ADM_COM_FUNC_EXIT();

    return nRet;
}

D32S adm_dim_ucl_init(ADM_DIM_SOA_UCL_INIT_INFO_T* initInfo, void* priv)
{
    log_i("adm_dim_ucl_init begin");
    log_i("appPriv: %p ", priv);

    D32S nRet = ADM_ECODE_FAILED;
    dim_func_list_t* func = adm_getFuncList();

    if(func == NULL || func->adm_dis_ucl_init_handle == NULL)
    {
        nRet = ADM_ECODE_INVALID_POINTER;
        log_e("func or adm_dim_ucl_init_handle pointer invalid, nRet[ %d ]", nRet);
        return nRet;
    }

    DCHAR* cfgInstances = adm_strdup(adm_cfg_iniParserGetString(ADM_CFG_VUS_UCL_INSTANCEID,  ""));

    DCHAR* subInstanceIdStr = NULL;
    subInstanceIdStr = strsep(&cfgInstances, ",");
    log_i("subInstanceIdStr:%s", subInstanceIdStr);

    while(NULL != subInstanceIdStr)
    {
        log_i("subInstanceIdStr:%s", subInstanceIdStr);
        nRet = func->adm_dis_ucl_init_handle(atoi(subInstanceIdStr), initInfo, priv);
        subInstanceIdStr = strsep(&cfgInstances, ",");
    }

    adm_free(cfgInstances);

    log_i("adm_dim_ucl_init end");
    return nRet;
}

void adm_dim_ucl_deinit(void)
{
    log_i("adm_dim_ucl_deinit begin");
    dim_func_list_t* func = adm_getFuncList();

    if(func == NULL || func->adm_dis_ucl_deinit_handle == NULL)
    {
        log_e("func or adm_dim_ucl_deinit_handle pointer invalid");
        return ;
    }

    DCHAR* instance = NULL;
    DCHAR* cfgInstances = adm_strdup(adm_cfg_iniParserGetString(ADM_CFG_VUS_UCL_INSTANCEID,  ""));

    instance = strsep(&cfgInstances, ",");

    while(instance != NULL)
    {
        func->adm_dis_ucl_deinit_handle(atoi(instance));
        instance = strsep(&cfgInstances, ",");
    }

    adm_free(cfgInstances);
    log_i("adm_dim_ucl_deinit end");
}
