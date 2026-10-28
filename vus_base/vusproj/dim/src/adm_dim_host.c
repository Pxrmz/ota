#include "adm_dim_log.h"

#include "adm_dim_host.h"
#include "adm_com_errorCode.h"
#include "adm_typedefs.h"
#include "adm_memory.h"
#include "adm_string.h"
#include "adm_com_lib.h"

#include "adm_dim_api.h"
#include "adm_dim_apiImpl.h"

#include "adm_cfg_api.h"
#include "adm_cfg_default.h"
#include "adm_oca_httpCommon.h"
#include "pub/inc/adm_pub_func.h"
#include "pub/inc/adm_pub_keylog.h"

#include "base64.h"

#ifdef SUPPORT_LION_PKI
extern SC_HANDLE handle_sc;
#endif

void adm_dim_host_getSdkVersion(const D16U instanceId, const D16U timeout, DCHAR** sdkVersion,
                                D32S* errorCode)
{
    log_i("adm_dim_host_getSdkVersion begin");
    D32S nRet = ADM_ECODE_FAILED;
    dim_func_list_t* func = adm_getFuncList();

    if(func == NULL || func->adm_dis_host_getSdkVersion_handle == NULL)
    {
        nRet = ADM_ECODE_INVALID_POINTER;
        log_e("func or adm_dim_host_getSdkVersion_handle pointer invalid, nRet[ %d ]", nRet);
        return;
    }

    func->adm_dis_host_getSdkVersion_handle(instanceId, timeout, sdkVersion, errorCode);
    log_i("adm_dim_host_getSdkVersion end");

    return;
}

D32S adm_dim_getVin(const D16U instanceId, const D16U timeout, DCHAR** vin, D32S* errorCode)
{
    log_i("adm_dim_getVin begin");
    D32S nRet = ADM_ECODE_FAILED;
    DCHAR aDescription[128] = {0};
    dim_func_list_t* func = adm_getFuncList();

    if(func == NULL || func->adm_dis_getVin_handle == NULL)
    {
        nRet = ADM_ECODE_INVALID_POINTER;
        log_e("func or adm_dim_getVin_handle pointer invalid, nRet[ %d ]", nRet);
        return nRet;
    }

    nRet = func->adm_dis_getVin_handle(instanceId, timeout, vin, errorCode);

    if (ADM_IS_NULL(*vin))
    {
        adm_snprintf(aDescription, sizeof(aDescription), "vin is null,errorCode=[%d] ", *errorCode);
        adm_pub_setKeylogString(ADM_PUB_KEYLOG_ID_002, nRet, aDescription);
    }
    else
    {
        adm_snprintf(aDescription, sizeof(aDescription), "vin=%s,errorCode=[%d] ", ADM_COM_STR_SAFE_OUTPUT(*vin), *errorCode);
        adm_pub_setKeylogString(ADM_PUB_KEYLOG_ID_001, nRet, aDescription);
    }

    log_i("adm_dim_getVin end");

    return nRet;
}

D32S adm_dim_getVehicleVersion(const D16U instanceId, const D16U timeout, DCHAR** pVehicleVer, D32S* errorCode)
{
    log_i("adm_dim_getVehicleVersion begin");
    D32S nRet = ADM_ECODE_FAILED;
    DCHAR aDescription[128] = {0};
    dim_func_list_t* func = adm_getFuncList();

    if(func == NULL || func->adm_dis_getVehicleVersion_handle == NULL)
    {
        nRet = ADM_ECODE_INVALID_POINTER;
        log_e("func or adm_dim_getVin_handle pointer invalid, nRet[ %d ]", nRet);
        return nRet;
    }

    nRet = func->adm_dis_getVehicleVersion_handle(instanceId, timeout, pVehicleVer, errorCode);

    if (ADM_IS_ERR(nRet))
    {
        adm_snprintf(aDescription, sizeof(aDescription), "errorCode=[%d] ", *errorCode);
        adm_pub_setKeylogString(ADM_PUB_KEYLOG_ID_038, nRet, aDescription);
    }
    // else
    // {
    //     adm_snprintf(aDescription, sizeof(aDescription), "version[%s],errorCode=[%d] ", ADM_COM_STR_SAFE_OUTPUT(*pVehicleVer), *errorCode);
    //     adm_pub_setKeylogString(ADM_PUB_KEYLOG_ID_039, nRet, aDescription);
    // }

    log_i("adm_dim_getVehicleVersion end");

    return nRet;
}
D32S adm_dim_setVehicleVersion(const D16U instanceId, const D16U timeout,
                                       const DCHAR* pVehicleVer, D32S* errorCode)
{
    log_i("adm_dim_setVehicleVersion begin");
    D32S nRet = ADM_ECODE_FAILED;
    DCHAR aDescription[128] = {0};
    dim_func_list_t* func = adm_getFuncList();

    if(func == NULL || func->adm_dis_setVehicleVersion_handle == NULL)
    {
        nRet = ADM_ECODE_INVALID_POINTER;
        log_e("func or adm_dis_setVehicleVersion_handle pointer invalid, nRet[ %d ]", nRet);
        return nRet;
    }

    nRet = func->adm_dis_setVehicleVersion_handle(instanceId, timeout, pVehicleVer, errorCode);

    if (ADM_IS_ERR(nRet))
    {
        adm_snprintf(aDescription, sizeof(aDescription), "errorCode=[%d] ", *errorCode);
        adm_pub_setKeylogString(ADM_PUB_KEYLOG_ID_040, nRet, aDescription);
    }
    else
    {
        adm_snprintf(aDescription, sizeof(aDescription), "version[%s],errorCode=[%d] ", ADM_COM_STR_SAFE_OUTPUT(pVehicleVer), *errorCode);
        adm_pub_setKeylogString(ADM_PUB_KEYLOG_ID_041, nRet, aDescription);
    }

    log_i("adm_dim_setVehicleVersion end");

    return nRet;
}

D32S adm_dim_host_execOtaTask(const D16U instanceId, const D16U timeout,
                              const ADM_DIM_SOA_HOST_OTA_TASK_TYPE_E eOtaTaskType, const D32U keeptime, D32S* errorCode)
{
    log_i("adm_dim_host_execOtaTask begin");
    D32S nRet = ADM_ECODE_FAILED;
    dim_func_list_t* func = adm_getFuncList();

    if(func == NULL || func->adm_dis_host_execOtaTask_handle == NULL)
    {
        nRet = ADM_ECODE_INVALID_POINTER;
        log_e("func or adm_dim_host_execOtaTask_handle pointer invalid, nRet[ %d ]", nRet);
        return nRet;
    }

    nRet = func->adm_dis_host_execOtaTask_handle(instanceId, timeout, eOtaTaskType, keeptime,
            errorCode);

    log_i("adm_dim_host_execOtaTask end");

    return nRet;
}

D32S adm_dim_host_setSchduleWakeUpTime(const D16U instanceId, const D16U timeout,
                                       const ADM_DIM_HOST_SCHDULE_TYPE_E eSchduleType, const D64S schduleTime, D32S* errorCode)
{
    log_i("adm_dim_host_setSchduleWakeUpTime begin");
    D32S nRet = ADM_ECODE_FAILED;
    dim_func_list_t* func = adm_getFuncList();

    if(func == NULL || func->adm_dis_host_setSchduleWakeUpTime_handle == NULL)
    {
        nRet = ADM_ECODE_INVALID_POINTER;
        log_e("func or adm_dim_host_setSchduleWakeUpTime_handle pointer invalid, nRet[ %d ]", nRet);
        return nRet;
    }

    nRet = func->adm_dis_host_setSchduleWakeUpTime_handle(instanceId, timeout, eSchduleType,
            schduleTime, errorCode);
    log_i("adm_dim_host_setSchduleWakeUpTime end");

    return nRet;
}

D32S adm_dim_host_getSchduleWakeUpTime(const D16U instanceId, const D16U timeout, D64S* schduleTime,
                                       D32S* errorCode)
{
    log_i("adm_dim_host_getSchduleWakeUpTime begin");
    D32S nRet = ADM_ECODE_FAILED;
    dim_func_list_t* func = adm_getFuncList();

    if(func == NULL || func->adm_dis_host_getSchduleWakeUpTime_handle == NULL)
    {
        nRet = ADM_ECODE_INVALID_POINTER;
        log_e("func or adm_dim_host_getSchduleWakeUpTime_handle pointer invalid, nRet[ %d ]", nRet);
        return nRet;
    }

    nRet = func->adm_dis_host_getSchduleWakeUpTime_handle(instanceId, timeout, schduleTime, errorCode);
    log_i("adm_dim_host_getSchduleWakeUpTime end");

    return nRet;
}

D32S adm_dim_host_setVehicleBigVersion(const D16U instanceId, const D16U timeout,
                                       const DCHAR* vehicleBigVersion, D32S* errorCode)
{
    log_i("adm_dim_host_setVehicleBigVersion begin");
    D32S nRet = ADM_ECODE_FAILED;
    dim_func_list_t* func = adm_getFuncList();

    if(func == NULL || func->adm_dis_host_setVehicleBigVersion_handle == NULL)
    {
        nRet = ADM_ECODE_INVALID_POINTER;
        log_e("func or adm_dim_host_setVehicleBigVersion_handle pointer invalid, nRet[ %d ]", nRet);
        return nRet;
    }

    nRet = func->adm_dis_host_setVehicleBigVersion_handle(instanceId, timeout, vehicleBigVersion,
            errorCode);
    log_i("adm_dim_host_setVehicleBigVersion end");

    return nRet;
}

D32S adm_dim_host_getVehicleBigVersion(const D16U instanceId, const D16U timeout,
                                       DCHAR** vehicleBigVersion, D32S* errorCode)
{
    log_i("adm_dim_host_getVehicleBigVersion begin");
    D32S nRet = ADM_ECODE_FAILED;
    dim_func_list_t* func = adm_getFuncList();

    if(func == NULL || func->adm_dis_host_getVehicleBigVersion_handle == NULL)
    {
        nRet = ADM_ECODE_INVALID_POINTER;
        log_e("func or adm_dim_host_getVehicleBigVersion_handle pointer invalid, nRet[ %d ]", nRet);
        return nRet;
    }

    nRet = func->adm_dis_host_getVehicleBigVersion_handle(instanceId, timeout, vehicleBigVersion,
            errorCode);
    log_i("adm_dim_host_getVehicleBigVersion end");

    return nRet;
}

D32S adm_dim_host_verifySpaceIsEnough(const D16U instanceId, const D16U timeout,
                                      const D64S needSize, D64S* realSize, D32S* errorCode)
{
    log_i("adm_dim_host_verifySpaceIsEnough begin");
    D32S nRet = ADM_ECODE_FAILED;
    dim_func_list_t* func = adm_getFuncList();

    if(func == NULL || func->adm_dis_host_verifySpaceIsEnough_handle == NULL)
    {
        nRet = ADM_ECODE_INVALID_POINTER;
        log_e("func or adm_dim_host_verifySpaceIsEnough_handle pointer invalid, nRet[ %d ]", nRet);
        return nRet;
    }

    nRet = func->adm_dis_host_verifySpaceIsEnough_handle(instanceId, timeout, needSize, realSize,
            errorCode);
    log_i("adm_dim_host_verifySpaceIsEnough end");

    return nRet;
}

D32S dimHostHexStringToByteArray(const DCHAR *pInStr, D8U *pOutData) 
{
    D32S iLen = adm_strlen(pInStr);
    if (iLen % 2 != 0) 
    {
        log_e("in str length invalid.");
        return ADM_ECODE_FAILED; // 字符串长度不是偶数
    }

    for (D32S i = 0; i < iLen; i += 2) 
    {
        sscanf(&pInStr[i], "%2hhx", &pOutData[i / 2]);
    }

    return iLen / 2;
}
#ifdef SUPPORT_LION_PKI
/**
 * @brief 本地验签
 * 
 * @param[in] instanceId     描述: 实例ID \n
 * 范围值: 无符号短整型 \n
 * 单位: N/A \n
 * 
 * @param[in] timeout     描述: 超时时间 \n
 * 范围值: 无符号短整型 \n
 * 单位: N/A \n
 * 
 * @param[in] signedData     描述: 签名文件内容（PKI签名值） \n
 * 范围值: 指针 \n
 * 单位: N/A \n
 * 
 * @param[in] signedDataLength     描述: 签名文件长度 \n
 * 范围值: 整型 \n
 * 单位: N/A \n
 * 
 * @param[in] orignalData     描述: 待PKI验证内容 \n
 * 范围值: 指针 \n
 * 单位: N/A \n
 * 
 * @param[in/out] orignalDataLength     描述: 待PKI验证内容长度 \n
 * 范围值: 整型\n
 * 单位: N/A \n
 * 
 * @param[in/out] errorCode     描述: 响应码 \n
 * 范围值: 整型 \n
 * 单位: N/A \n
 * 
 * @return D32S 
 */
D32S adm_dim_host_verifySignature(const D16U instanceId, const D16U timeout,
                                  const DCHAR* signedData, const D32S signedDataLength,
                                  const DCHAR* orignalData, const D32S orignalDataLength,
                                  D32S* errorCode)
{
    log_i("adm_dim_host_verifySignature begin");
    D32S nRet = ADM_ECODE_SUCCESSED;
    D8U* pCertificate = NULL;
    D32U uiCertLen = 3072;//PKI反馈，要先赋值，PKI内部会先判断这个值（不理解但执行）
    D8U* pPkiSignData = NULL;
    D8U* pByteStr = NULL;
    D32S iPkiSignLength = 0;

    do
    {
        /** 检查验签证书 */
        nRet = adm_oca_checkSignCert();
        if (SC_SUCCESS != nRet)
        {
            log_i("adm_dim_checkSignCert err[%d]", nRet);
            break;
        }

        /** 获取证书 */
        pCertificate = (D8U*)adm_malloc(3072);
        nRet = IW_ExportCertificate(handle_sc, USER_ID_OTA, pCertificate, &uiCertLen);
        if (SC_SUCCESS != nRet)
        {
            log_e("IW_ExportCertificate err[%d]", nRet);
            break;
        }

        /** 对签名值进行base64解码 */
        iPkiSignLength = base64x_decoded_size(signedDataLength);
        if (0 >= iPkiSignLength)
        {
            log_e("cal base64 length error.[%d][%d]", iPkiSignLength, signedDataLength);
            nRet = ADM_ECODE_VERIFY_SIGNATURE_FAILED;
            break;
        }

        pPkiSignData = (D8U*)adm_malloc(sizeof(D8U) * (iPkiSignLength + 1));
        if ((iPkiSignLength = base64x_decode_binary((D8U *)pPkiSignData, signedData)) < 0) 
        {
            log_e("call base64x_decode_binary error.[%d][%d]", iPkiSignLength, signedDataLength);
            nRet = ADM_ECODE_VERIFY_SIGNATURE_FAILED;
            break;
        }
        pPkiSignData[iPkiSignLength] = '\0';

        pByteStr = (D8U*)adm_malloc(orignalDataLength / 2);
        D32S byteLength = dimHostHexStringToByteArray(orignalData, pByteStr);
        if (0 >= byteLength)
        {
            nRet = ADM_ECODE_VERIFY_SIGNATURE_FAILED;
            break;
        }

        nRet = IW_VerifySignByCert(SC_SHA512_RSA_PSS, pCertificate, uiCertLen, (D8U*)pByteStr, byteLength, pPkiSignData, iPkiSignLength);
        if (nRet == SC_SUCCESS) 
        {
            log_i("IW_VerifyFileSignature Success.\n");
        } 
        else 
        {
            log_e("IW_VerifyFileSignature Fail, Fail Code: %d.\n", nRet);
            nRet = ADM_ECODE_VERIFY_SIGNATURE_FAILED;
            break;
        }
    } while (DFALSE);

    if (ADM_IS_ERR(nRet))
    {
        log_e("certificate context[%s]", pCertificate);
        log_e("base64 context[%s] base64 context length[%d]", signedData, signedDataLength);
        log_e("cert length[%u], sign context length[%d]", uiCertLen, iPkiSignLength);
        log_e("verify context[%s] verify context length[%d]", orignalData, orignalDataLength);
    }

    *errorCode = nRet;
    adm_free(pCertificate);
    adm_free(pPkiSignData);
    adm_free(pByteStr);

    log_i("adm_dim_host_verifySignature end");

    return nRet;
}
#else
D32S adm_dim_host_verifySignature(const D16U instanceId, const D16U timeout,
                                  const DCHAR* signedData, const D32S signedDataLength,
                                  const DCHAR* orignalData, const D32S orignalDataLength,
                                  D32S* errorCode)
{
    log_i("adm_dim_host_verifySignature begin");
    D32S nRet = ADM_ECODE_FAILED;
    dim_func_list_t* func = adm_getFuncList();

    if(func == NULL || func->adm_dis_host_verifySignature_handle == NULL)
    {
        nRet = ADM_ECODE_INVALID_POINTER;
        log_e("func or adm_dim_host_verifySignature pointer invalid, nRet[ %d ]", nRet);
        return nRet;
    }

    nRet = func->adm_dis_host_verifySignature_handle(instanceId, timeout, signedData, signedDataLength,
            orignalData, orignalDataLength, errorCode);
    log_i("adm_dim_host_verifySignature end");

    return nRet;
}
#endif

D32S adm_dim_host_getCertAbsolutePath(const D16U instanceId, const D16U timeout,
                                      const ADM_DIM_SOA_HOST_CERT_TYPE_E eCertType, DCHAR** certPath, D32S* errorCode)
{
    log_i("adm_dim_host_getCertAbsolutePath begin");

    D32S nRet = ADM_ECODE_FAILED;
    dim_func_list_t* func = adm_getFuncList();

    if(func == NULL || func->adm_dis_host_getCertAbsolutePath_handle == NULL)
    {
        nRet = ADM_ECODE_INVALID_POINTER;
        log_e("func or adm_dim_host_getCertAbsolutePath_handle pointer invalid, nRet[ %d ]", nRet);
        return nRet;
    }

    nRet = func->adm_dis_host_getCertAbsolutePath_handle(instanceId, timeout, eCertType, certPath,
            errorCode);

    log_i("adm_dim_host_getCertAbsolutePath end");
    return nRet;
}

D32S adm_dim_host_setDiagTroubleCode(const D16U instanceId, const D16U timeout,
                                     const D32S diagTroubleCode, const DCHAR* dtcSnapShop, D32S* errorCode)
{
    log_i("adm_dim_host_setDiagTroubleCode begin");


    D32S nRet = ADM_ECODE_FAILED;
    dim_func_list_t* func = adm_getFuncList();

    if(func == NULL || func->adm_dis_host_setDiagTroubleCode_handle == NULL)
    {
        nRet = ADM_ECODE_INVALID_POINTER;
        log_e("func or adm_dim_host_init_handle pointer invalid, nRet[ %d ]", nRet);
        return nRet;
    }

    nRet = func->adm_dis_host_setDiagTroubleCode_handle(instanceId, timeout, diagTroubleCode,
            dtcSnapShop, errorCode);

    log_i("adm_dim_host_init end");
    return nRet;
}

D32S adm_dim_host_clearDiagTroubleCode(const D16U instanceId, const D16U timeout,
                                       const D32S diagTroubleCode, D32S* errorCode)
{
    log_i("adm_dim_host_init begin");

    D32S nRet = ADM_ECODE_FAILED;
    dim_func_list_t* func = adm_getFuncList();

    if(func == NULL || func->adm_dis_host_clearDiagTroubleCode_handle == NULL)
    {
        nRet = ADM_ECODE_INVALID_POINTER;
        log_e("func or adm_dim_host_init_handle pointer invalid, nRet[ %d ]", nRet);
        return nRet;
    }

    nRet = func->adm_dis_host_clearDiagTroubleCode_handle(instanceId, timeout, diagTroubleCode,
            errorCode);

    log_i("adm_dim_host_init end");
    return nRet;
}

D32S adm_dim_host_readVehicleStatus(const D16U instanceId,
                                    const D16U timeout,
                                    const ADM_DIM_SOA_MSGID_DATA_ARRAY_T* msgIdListRequest,
                                    ADM_DIM_SOA_MSGID_DATA_ARRAY_T** msgIdListResponse, D32S* errorCode)
{
    log_i("************ begin ************");
    D32S nRet = ADM_ECODE_FAILED;
    DCHAR aDescription[128] = {0};
    dim_func_list_t* func = adm_getFuncList();

    if(func == NULL || func->adm_dis_host_readVehicleStatus_handle == NULL)
    {
        nRet = ADM_ECODE_INVALID_POINTER;
        log_e("func or adm_dis_host_readVehicleStatus_handle invalid, nRet[ %d ]", nRet);
        return nRet;
    }

    nRet = func->adm_dis_host_readVehicleStatus_handle(instanceId, timeout, msgIdListRequest,
            msgIdListResponse, errorCode);

    if (ADM_IS_ERR(nRet))
    {
        adm_snprintf(aDescription, sizeof(aDescription), "errorCode=[%d] ", *errorCode);
        adm_pub_setKeylogString(ADM_PUB_KEYLOG_ID_037, nRet, aDescription);
    }
    else
    {
        adm_pub_setKeylogString(ADM_PUB_KEYLOG_ID_037, nRet, "");
    }

    log_i("************ end ************");

    return nRet;

}

static void setVehicleStatusKeyLog(D32S iRet, D32U msgType, D32S iErrcode)
{
    DCHAR aDescription[128] = {0};
    if (ADM_IS_ERR(iRet))
    {
        if (VEHICLE_CTRL_ENTER_NORMAL_OTA_MODE == msgType || VEHICLE_CTRL_EXIT_NORMAL_OTA_MODE == msgType)
        {
            adm_snprintf(aDescription, sizeof(aDescription), "type=[%d],进入OTA模式-259,退出OTA模式-260,errorCode=[%d] ", msgType,iErrcode);
            adm_pub_setKeylogString(ADM_PUB_KEYLOG_ID_020, iRet, aDescription);
        }
        else if (VEHICLE_CTRL_ENTER_HV_MODE == msgType || VEHICLE_CTRL_EXIT_HV_MODE == msgType || VEHICLE_CTRL_NO_ACTIVE_HV_MODE == msgType)
        {
            adm_snprintf(aDescription, sizeof(aDescription), "type=[%d],上高压-257 下高压-258 noActive-283,errorCode=[%d] ", msgType,iErrcode);
            adm_pub_setKeylogString(ADM_PUB_KEYLOG_ID_022, iRet, aDescription);
        }
    }
    else
    {
        if (VEHICLE_CTRL_ENTER_NORMAL_OTA_MODE == msgType || VEHICLE_CTRL_EXIT_NORMAL_OTA_MODE == msgType)
        {
            adm_snprintf(aDescription, sizeof(aDescription), "type=[%d],进入OTA模式-259,退出OTA模式-260,errorCode=[%d] ", msgType,iErrcode);
            adm_pub_setKeylogString(ADM_PUB_KEYLOG_ID_019, iRet, aDescription);
        }
        else if (VEHICLE_CTRL_ENTER_HV_MODE == msgType  || VEHICLE_CTRL_EXIT_HV_MODE == msgType || VEHICLE_CTRL_NO_ACTIVE_HV_MODE == msgType)
        {
            adm_snprintf(aDescription, sizeof(aDescription), "type=[%d],上高压-257 下高压-258 noActive-283,errorCode=[%d] ", msgType,iErrcode);
            adm_pub_setKeylogString(ADM_PUB_KEYLOG_ID_021, iRet, aDescription);
        }
    }

    return ;
}


D32S adm_dim_host_setVehicleStatus(const D16U instanceId, const D16U timeout, D32U msgType, D32S* errorCode)
{
    log_i("************ begin ************");
    D32S nRet = ADM_ECODE_FAILED;
    
    dim_func_list_t* func = adm_getFuncList();

    if(func == NULL || func->adm_dis_host_setVehicleStatus_handle == NULL)
    {
        nRet = ADM_ECODE_INVALID_POINTER;
        log_e("func or adm_dis_host_setVehicleStatus_handle invalid, nRet[ %d ]", nRet);
        return nRet;
    }

    nRet = func->adm_dis_host_setVehicleStatus_handle(instanceId, timeout, msgType, errorCode);

    setVehicleStatusKeyLog(nRet,msgType,*errorCode);

    log_i("************ end ************");

    return nRet;
}


D32S adm_dim_host_set_log_level(const D16U instanceId, const D16U timeout,
                                D32U logLevel, D32S* errorCode)
{
    log_i("adm_dim_host_set_log_level begin");

    D32S nRet = ADM_ECODE_FAILED;
    dim_func_list_t* func = adm_getFuncList();

    if(func == NULL || func->adm_dis_host_set_log_level_handle == NULL)
    {
        nRet = ADM_ECODE_INVALID_POINTER;
        log_e("func or adm_dis_host_set_log_level_handle pointer invalid, nRet[ %d ]", nRet);
        return nRet;
    }

    nRet = func->adm_dis_host_set_log_level_handle(instanceId, timeout, logLevel, errorCode);
    log_i("adm_dim_host_set_log_level end");

    return nRet;
}

D32S adm_dim_host_getNetworkType(const D16U instanceId, const D16U timeout, D32S* networkType,
                                 D32S* errorCode)
{
    log_i("adm_dim_host_getNetworkType begin");

    D32S nRet = ADM_ECODE_FAILED;
    dim_func_list_t* func = adm_getFuncList();

    if(func == NULL || func->adm_dim_host_getNetworkType_handle == NULL)
    {
        nRet = ADM_ECODE_INVALID_POINTER;
        log_e("func or adm_dim_host_getNetworkType_handle pointer invalid, nRet[ %d ]", nRet);
        return nRet;
    }

    nRet = func->adm_dim_host_getNetworkType_handle(instanceId, timeout, networkType, errorCode);

    log_i("adm_dim_host_getNetworkType end");

    return nRet;
}

D32S adm_dim_host_getPrivacyPath(const D16U instanceId, const D16U timeout, DCHAR** privacyPath, D32S* errorCode)
{
    log_i("adm_dim_host_getPrivacyPath begin");

    D32S nRet = ADM_ECODE_FAILED;
    dim_func_list_t* func = adm_getFuncList();

    if(func == NULL || func->adm_dis_host_getPrivacyPath_handle == NULL)
    {
        nRet = ADM_ECODE_INVALID_POINTER;
        log_e("func or adm_dim_host_getPrivacyPath pointer invalid, nRet[ %d ]", nRet);
        return nRet;
    }

    nRet = func->adm_dis_host_getPrivacyPath_handle(instanceId, timeout, privacyPath,
            errorCode);

    log_i("adm_dim_host_getPrivacyPath end");
    return nRet;
}
								 
D32S adm_dim_host_setWifiLink(const D16U instanceId, const D16U timeout,
                              const ADM_HOST_WIFI_INFO_ARRAY_T* pWifiArray, D32S* errorCode)
{
    log_i("adm_dim_host_setWifiLink begin");

    D32S nRet = ADM_ECODE_FAILED;
    dim_func_list_t* func = adm_getFuncList();

    if(func == NULL || func->adm_dim_host_setWifiLink_handle == NULL)
    {
        nRet = ADM_ECODE_INVALID_POINTER;
        log_e("func or adm_dim_host_setWifiLink pointer invalid, nRet[ %d ]", nRet);
        return nRet;
    }

    nRet = func->adm_dim_host_setWifiLink_handle(instanceId, timeout, pWifiArray, errorCode);

    log_i("adm_dim_host_setWifiLink end");

    return nRet;
}

D32S adm_dim_host_setUpgradeMode(const D16U instanceId, const D16U timeout, 
                                    D32S iUpgradeMode, D32S* errorCode)
{
    log_i("adm_dim_host_setUpgradeMode begin");
    D32S nRet = ADM_ECODE_FAILED;
    DCHAR aDescription[128] = {0};
    dim_func_list_t* func = adm_getFuncList();

    if(func == NULL || func->adm_dim_host_setUpgradeMode_handle == NULL)
    {
        nRet = ADM_ECODE_INVALID_POINTER;
        log_e("func or adm_dim_host_setUpgradeMode_handle pointer invalid, nRet[ %d ]", nRet);
        return nRet;
    }

    nRet = func->adm_dim_host_setUpgradeMode_handle(instanceId, timeout, iUpgradeMode, errorCode);

    if (ADM_IS_ERR(nRet))
    {
        adm_snprintf(aDescription, sizeof(aDescription), "errorCode=[%d] ", *errorCode);
        adm_pub_setKeylogString(ADM_PUB_KEYLOG_ID_024, nRet, aDescription);
    }
    else
    {
        adm_pub_setKeylogString(ADM_PUB_KEYLOG_ID_023, nRet, "");
    }

    log_i("adm_dim_host_setUpgradeMode end");

    return nRet;
}

D32S adm_dim_host_setPower(const D16U instanceId, const D16U timeout,
                              D32S msgType, D32S* errorCode)
{
    log_i("adm_dim_host_setPower begin");

    D32S nRet = ADM_ECODE_FAILED;
    DCHAR aDescription[128] = {0};
    dim_func_list_t* func = adm_getFuncList();

    if(func == NULL || func->adm_dim_host_setPower_handle == NULL)
    {
        nRet = ADM_ECODE_INVALID_POINTER;
        log_e("func or adm_dim_host_getNetworkType_handle pointer invalid, nRet[ %d ]", nRet);
        return nRet;
    }

    nRet = func->adm_dim_host_setPower_handle(instanceId, timeout, msgType, errorCode);
    if (ADM_IS_ERR(nRet))
    {
        adm_snprintf(aDescription, sizeof(aDescription), "type=[%d], 上电-8 下电-9 errorCode=[%d] ", *errorCode);
        adm_pub_setKeylogString(ADM_PUB_KEYLOG_ID_028, nRet, aDescription);
    }
    else
    {
        adm_pub_setKeylogString(ADM_PUB_KEYLOG_ID_027, nRet, "");
    }

    log_i("adm_dim_host_setPower end");

    return nRet;
}

/**
 * @brief 重启从控客户端
 * @note 
 * @param in const D16U instanceId  客户端实例Id
 * @param in const D16U timeout  超时时间，单位：秒
 * @param out D32S* errorCode  错误码
 * @return D32S  0:successed, Non-0:failed
 */
D32S adm_dim_host_reboot(const D16U instanceId, const D16U timeout, D32S* errorCode)
{
    ADM_COM_FUNC_ENTRY();

    D32S iRet = ADM_ECODE_FAILED;
    dim_func_list_t* func = adm_getFuncList();

    if(func == NULL || func->adm_dim_host_reboot_handle == NULL)
    {
        iRet = ADM_ECODE_INVALID_POINTER;
        log_e("func or adm_dim_host_reboot_handle pointer invalid, nRet[ %d ]", iRet);
        return iRet;
    }

    iRet = func->adm_dim_host_reboot_handle(instanceId, timeout, errorCode);

    ADM_COM_FUNC_EXIT();

    return iRet;
}

D32S adm_dim_host_downExceptionNotify(const D16U instanceId, const D16U timeout, 
                                    D32S iResult, D32S* errorCode)
{
    log_i("adm_dim_host_downExceptionNotify begin");
    D32S nRet = ADM_ECODE_FAILED;
    dim_func_list_t* func = adm_getFuncList();

    if(func == NULL || func->adm_dim_host_downExceptionNotify_handle == NULL)
    {
        nRet = ADM_ECODE_INVALID_POINTER;
        log_e("func or adm_dim_host_downExceptionNotify_handle pointer invalid, nRet[ %d ]", nRet);
        return nRet;
    }

    nRet = func->adm_dim_host_downExceptionNotify_handle(instanceId, timeout, iResult, errorCode);

    log_i("adm_dim_host_downExceptionNotify end");

    return nRet;
}

/**
 * @brief 获取OTA模式状态
 * @note
 * @param in const D16U instanceId  实例ID
 * @param in const D16U timeout  超时时间
 * @param out D32S *piOtaModeStatus  OTA模式状态 0:OTA模式OFF 1:OTA模式ON
 * @param out D32S *errorCode  错误码
 * @return D32S  返回值
 */
D32S adm_dim_host_getOtaModeStatus(const D16U instanceId, const D16U timeout, D32S *piOtaModeStatus, D32S* errorCode)
{
    D32S nRet = ADM_ECODE_FAILED;
    DCHAR aDescription[128] = {0};
    dim_func_list_t* func = adm_getFuncList();

    ADM_COM_FUNC_ENTRY();

    if(func == NULL || func->adm_dim_host_getOtaModeStatus_handle == NULL)
    {
        nRet = ADM_ECODE_INVALID_POINTER;
        log_e("func or adm_dim_host_getOtaModeStatus_handle pointer invalid, nRet[ %d ]", nRet);
        return nRet;
    }

    nRet = func->adm_dim_host_getOtaModeStatus_handle(instanceId, timeout, piOtaModeStatus, errorCode);
    if (ADM_IS_ERR(nRet))
    {
        adm_snprintf(aDescription, sizeof(aDescription), "errorCode=[%d] ", *errorCode);
        adm_pub_setKeylogString(ADM_PUB_KEYLOG_ID_026, nRet, aDescription);
    }
    else
    {
        adm_pub_setKeylogString(ADM_PUB_KEYLOG_ID_025, nRet, "");
    }

    ADM_COM_FUNC_EXIT();

    return nRet;
}

D32S adm_dim_host_getTboxRemoteDiaStatus(const D16U uiInstanceId, const D16U uiTimeout, D32S *pTboxRemoteDiaStatus, D32S* pErrorCode)
{
    D32S nRet = ADM_ECODE_FAILED;
    dim_func_list_t* func = adm_getFuncList();

    ADM_COM_FUNC_ENTRY();

    if(func == NULL || func->adm_dim_host_getTboxRemoteDiaStatus_handle == NULL)
    {
        nRet = ADM_ECODE_INVALID_POINTER;
        log_e("func or adm_dim_host_getTboxRemoteDiaStatus_handle pointer invalid, nRet[ %d ]", nRet);
        return nRet;
    }

    nRet = func->adm_dim_host_getTboxRemoteDiaStatus_handle(uiInstanceId, uiTimeout, pTboxRemoteDiaStatus, pErrorCode);

    ADM_COM_FUNC_EXIT();

    return nRet;
}

/**
 * @brief 紧急下电
 * @note
 * @param in const D16U instanceId  实例ID
 * @param in const D16U timeout  超时时间
 * @param out D32S *errorCode  错误码
 * @return D32S  返回值
 */
D32S adm_dim_host_setIcuPowerOff(const D16U instanceId, const D16U timeout, D32S* errorCode)
{
    D32S nRet = ADM_ECODE_FAILED;
    dim_func_list_t* func = adm_getFuncList();

    ADM_COM_FUNC_ENTRY();

    if(func == NULL || func->adm_dim_host_setIcuPowerOff_handle == NULL)
    {
        nRet = ADM_ECODE_INVALID_POINTER;
        log_e("func or adm_dim_host_setIcuPowerOff_handle pointer invalid, nRet[ %d ]", nRet);
        return nRet;
    }

    nRet = func->adm_dim_host_setIcuPowerOff_handle(instanceId, timeout, errorCode);

    ADM_COM_FUNC_EXIT();

    return nRet;
}

/**
 * @brief 让hmi通知车机OTA已准备就绪
 * @note
 * @param in const D16U instanceId  实例ID
 * @param in const D16U timeout  超时时间
 * @param out D32S *errorCode  错误码
 * @return D32S  返回值
 */
D32S adm_dim_host_notifyOtaHmiStart(const D16U instanceId, const D16U timeout, D32S *errorCode)
{
	D32S nRet = ADM_ECODE_FAILED;
	dim_func_list_t *func = adm_getFuncList();

	ADM_COM_FUNC_ENTRY();

	if (func == NULL || func->adm_dim_host_notifyOtaHmiStart_handle == NULL)
	{
		nRet = ADM_ECODE_INVALID_POINTER;
		log_e("func or adm_dim_host_notifyOtaHmiStart_handle pointer invalid, nRet[ %d ]", nRet);
		return nRet;
	}

	nRet = func->adm_dim_host_notifyOtaHmiStart_handle(instanceId, timeout, errorCode);

	ADM_COM_FUNC_EXIT();

	return nRet;
}

D32S adm_dim_host_init(ADM_DIM_SOA_HOST_INIT_INFO_T* initInfo, void* priv)
{
    log_i("adm_dim_host_init begin");
    log_i("appPriv: %p ", priv);

    D32S nRet = ADM_ECODE_FAILED;
    dim_func_list_t* func = adm_getFuncList();

    if(func == NULL || func->adm_dis_host_init_handle == NULL)
    {
        nRet = ADM_ECODE_INVALID_POINTER;
        log_e("func or adm_dis_host_init_handle pointer invalid, nRet[ %d ]", nRet);
        return nRet;
    }

    DCHAR* cfgInstances = adm_strdup(adm_cfg_iniParserGetString(ADM_CFG_VUS_HOST_INSTANCEID,  ""));

    DCHAR* subInstanceIdStr = NULL;
    subInstanceIdStr = strsep(&cfgInstances, ",");
    log_i("subInstanceIdStr:%s", subInstanceIdStr);

    while(NULL != subInstanceIdStr)
    {
        log_i("subInstanceIdStr:%s", subInstanceIdStr);
        nRet = func->adm_dis_host_init_handle(atoi(subInstanceIdStr), initInfo, priv);
        subInstanceIdStr = strsep(&cfgInstances, ",");
    }

    adm_free(cfgInstances);

    log_i("adm_dim_host_init end");
    return nRet;
}

void adm_dim_host_deinit(void)
{

    log_i("adm_dim_host_deinit begin");

    dim_func_list_t* func = adm_getFuncList();

    if(func == NULL || func->adm_dis_host_deinit_handle == NULL)
    {
        log_e("func or adm_dim_host_deinit pointer invalid");
        return ;
    }

    DCHAR* instance = NULL;
    DCHAR* cfgInstances = adm_strdup(adm_cfg_iniParserGetString(ADM_CFG_VUS_HOST_INSTANCEID,  ""));

    instance = strsep(&cfgInstances, ",");

    while(instance != NULL)
    {
        func->adm_dis_host_deinit_handle(atoi(instance));
        instance = strsep(&cfgInstances, ",");
    }

    adm_free(cfgInstances);

    log_i("adm_dim_host_deinit end");

    return;
}