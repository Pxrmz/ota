#include "adm_dim_log.h"
#include "adm_dim_vdi_simulator.h"

#include "adm_dim_vdi.h"
#include "adm_soa.h"
#include "adm_soa_cfg.h"
#include <unistd.h>

static void wait_sleep(int secs)
{
    sleep(secs);
}

D32S adm_dim_vdi_readDiagnosticInfo_test()
{
    log_i("adm_dim_vdi_readDiagnosticInfo_test begin");

    ADM_DIM_SOA_ECU_IDEN_INFO_T *pstEcuIdenInfo = NULL;
    pstEcuIdenInfo = (ADM_DIM_SOA_ECU_IDEN_INFO_T *)adm_malloc(sizeof(
                         ADM_DIM_SOA_ECU_IDEN_INFO_T));
    pstEcuIdenInfo->requestId = 0x6E2;
    pstEcuIdenInfo->responseId = 0x6D9;
    pstEcuIdenInfo->functionId = 0x400;
    pstEcuIdenInfo->swDid = 0x0001;

    ADM_DIM_SOA_DID_DATA_ARRAY_T didListRequest;
    didListRequest.didArrayLength = 9;

    didListRequest.didArray = (ADM_DIM_SOA_DID_DATA_T **)adm_malloc(
                                  sizeof(ADM_DIM_SOA_DID_DATA_T *) * didListRequest.didArrayLength);

    didListRequest.didArray[0] = (ADM_DIM_SOA_DID_DATA_T *)adm_malloc(sizeof(
                                     ADM_DIM_SOA_DID_DATA_T));
    didListRequest.didArray[0]->did = 0xf188;
    didListRequest.didArray[0]->startByte = 0x00;
    didListRequest.didArray[0]->endByte = 0x0D;
    didListRequest.didArray[0]->errorCode = 0;
    didListRequest.didArray[0]->didData = (ADM_DIM_SOA_DATA_T*)adm_malloc(sizeof(ADM_DIM_SOA_DATA_T));
    didListRequest.didArray[0]->didData->dataType = VAR_STRING;
    //didListRequest.didArray[0]->didData->dataLength = 0xD;

    didListRequest.didArray[1] = (ADM_DIM_SOA_DID_DATA_T *)adm_malloc(sizeof(
                                     ADM_DIM_SOA_DID_DATA_T));
    didListRequest.didArray[1]->did = 0xf18c;
    didListRequest.didArray[1]->startByte = 0x00;
    didListRequest.didArray[1]->endByte = 0x0D;
    didListRequest.didArray[1]->errorCode = 0;
    didListRequest.didArray[1]->didData = (ADM_DIM_SOA_DATA_T*)adm_malloc(sizeof(ADM_DIM_SOA_DATA_T));
    didListRequest.didArray[1]->didData->dataType = VAR_STRING;

    didListRequest.didArray[2] = (ADM_DIM_SOA_DID_DATA_T *)adm_malloc(sizeof(
                                     ADM_DIM_SOA_DID_DATA_T));
    didListRequest.didArray[2]->did = 0xf193;
    didListRequest.didArray[2]->startByte = 0x00;
    didListRequest.didArray[2]->endByte = 0x0D;
    didListRequest.didArray[2]->errorCode = 0;
    didListRequest.didArray[2]->didData = (ADM_DIM_SOA_DATA_T*)adm_malloc(sizeof(ADM_DIM_SOA_DATA_T));
    didListRequest.didArray[2]->didData->dataType = VAR_STRING;

    didListRequest.didArray[3] = (ADM_DIM_SOA_DID_DATA_T *)adm_malloc(sizeof(
                                     ADM_DIM_SOA_DID_DATA_T));
    didListRequest.didArray[3]->did = 0xf180;
    didListRequest.didArray[3]->startByte = 0x00;
    didListRequest.didArray[3]->endByte = 0x0D;
    didListRequest.didArray[3]->errorCode = 0;
    didListRequest.didArray[3]->didData = (ADM_DIM_SOA_DATA_T*)adm_malloc(sizeof(ADM_DIM_SOA_DATA_T));
    didListRequest.didArray[3]->didData->dataType = VAR_STRING;

    didListRequest.didArray[4] = (ADM_DIM_SOA_DID_DATA_T *)adm_malloc(sizeof(
                                     ADM_DIM_SOA_DID_DATA_T));
    didListRequest.didArray[4]->did = 0xf182;
    didListRequest.didArray[4]->startByte = 0x00;
    didListRequest.didArray[4]->endByte = 0x0D;
    didListRequest.didArray[4]->errorCode = 0;
    didListRequest.didArray[4]->didData = (ADM_DIM_SOA_DATA_T*)adm_malloc(sizeof(ADM_DIM_SOA_DATA_T));
    didListRequest.didArray[4]->didData->dataType = VAR_STRING;

    didListRequest.didArray[5] = (ADM_DIM_SOA_DID_DATA_T *)adm_malloc(sizeof(
                                     ADM_DIM_SOA_DID_DATA_T));
    didListRequest.didArray[5]->did = 0xf187;
    didListRequest.didArray[5]->startByte = 0x00;
    didListRequest.didArray[5]->endByte = 0x0D;
    didListRequest.didArray[5]->errorCode = 0;
    didListRequest.didArray[5]->didData = (ADM_DIM_SOA_DATA_T*)adm_malloc(sizeof(ADM_DIM_SOA_DATA_T));
    didListRequest.didArray[5]->didData->dataType = VAR_STRING;

    didListRequest.didArray[6] = (ADM_DIM_SOA_DID_DATA_T *)adm_malloc(sizeof(
                                     ADM_DIM_SOA_DID_DATA_T));
    didListRequest.didArray[6]->did = 0xf18b;
    didListRequest.didArray[6]->startByte = 0x00;
    didListRequest.didArray[6]->endByte = 0x0D;
    didListRequest.didArray[6]->errorCode = 0;
    didListRequest.didArray[6]->didData = (ADM_DIM_SOA_DATA_T*)adm_malloc(sizeof(ADM_DIM_SOA_DATA_T));
    didListRequest.didArray[6]->didData->dataType = VAR_STRING;

    didListRequest.didArray[7] = (ADM_DIM_SOA_DID_DATA_T *)adm_malloc(sizeof(
                                     ADM_DIM_SOA_DID_DATA_T));
    didListRequest.didArray[7]->did = 0xf18a;
    didListRequest.didArray[7]->startByte = 0x00;
    didListRequest.didArray[7]->endByte = 0x0D;
    didListRequest.didArray[7]->errorCode = 0;
    didListRequest.didArray[7]->didData = (ADM_DIM_SOA_DATA_T*)adm_malloc(sizeof(ADM_DIM_SOA_DATA_T));
    didListRequest.didArray[7]->didData->dataType = VAR_STRING;
    
    didListRequest.didArray[8] = (ADM_DIM_SOA_DID_DATA_T *)adm_malloc(sizeof(
                                     ADM_DIM_SOA_DID_DATA_T));
    didListRequest.didArray[8]->did = 0xf191;
    didListRequest.didArray[8]->startByte = 0x00;
    didListRequest.didArray[8]->endByte = 0x0D;
    didListRequest.didArray[8]->errorCode = 0;
    didListRequest.didArray[8]->didData = (ADM_DIM_SOA_DATA_T*)adm_malloc(sizeof(ADM_DIM_SOA_DATA_T));
    didListRequest.didArray[8]->didData->dataType = VAR_STRING;

    char *diagScriptPath = NULL;
    //diagScriptPath = adm_strdup("/data/fota/lite/etc/vditestconfig/Read_ECUinfo.ttb");
    //diagScriptPath = adm_strdup("/sdcard/app/ota/admota/ucm/unzip/confPkg/Read_ECUinfo_14.ttb");
    diagScriptPath = adm_strdup("/renzhendi/test/vditest/vditestconfig/Read_ECUinfo.ttb");
    ADM_DIM_SOA_VDI_BUS_TYPE_E eBusType = VEHICLE_BUS_TYPE_CAN;
    ADM_DIM_SOA_PIN_CHANNEL_TYPE_E eChannelType = VEHICLE_BUS_PIN_TYPE_A_CAN;
    DBOOL isCGWRouted = FALSE;
    ADM_DIM_SOA_DID_DATA_ARRAY_T *didListResponse = NULL;
    D32S errcode;//无效出参

    D32S retVal = adm_dim_vdi_readDiagnosticInfo(adm_soa_getVdiInstanceId(), 5,
                  pstEcuIdenInfo, diagScriptPath, eBusType, eChannelType, isCGWRouted,
                  didListRequest, &didListResponse, &errcode);

    //req free
    for (int index = 0; index < didListRequest.didArrayLength; ++index)
    {
        adm_free(didListRequest.didArray[index]->didData);
        adm_free(didListRequest.didArray[index]);
    }
    adm_free(didListRequest.didArray);
    adm_free(diagScriptPath);

    //响应在外层释放 ?
    if (didListResponse)
    {
        log_i("notify msgIdArrayLength = %d", didListResponse->didArrayLength);

        for (int i = 0; i < didListResponse->didArrayLength; i++)
        {
            log_i("notify[%d] did = 0x%x", i, didListResponse->didArray[i]->did);
            log_i("notify startbyte = 0x%x", didListResponse->didArray[i]->startByte);
            log_i("notify endbyte = 0x%x", didListResponse->didArray[i]->endByte);
            log_i("notify dataLength = %d",
                  didListResponse->didArray[i]->didData->dataLength);
            log_i("notify dataType = %d", didListResponse->didArray[i]->didData->dataType);
            if(VAR_STRING == 
                didListResponse->didArray[i]->didData->dataType)
            {
                log_i("notify dataValue = %s", didListResponse->didArray[i]->didData->v.valueS);
            }
        }
        //resp free
        for (int index = 0; index < didListResponse->didArrayLength; ++index)
        {
            adm_free(didListResponse->didArray[index]->didData);
            adm_free(didListResponse->didArray[index]);
        }

        adm_free(didListResponse->didArray);
        adm_free(didListResponse);
    }

    if (retVal != ADM_ECODE_SUCCESSED)
    {
        log_w("call adm_dim_soa_vdi_readDiagnosticInfo fail retVal=%d", retVal);
        return  ADM_ECODE_FAILED;
    }

    log_i("adm_dim_vdi_readDiagnosticInfo_test end");

    return ADM_ECODE_SUCCESSED;
}

D32S adm_dim_vdi_readDiagnosticInfo_tes1()
{
    log_i("adm_dim_vdi_readDiagnosticInfo_test begin");

    ADM_DIM_SOA_ECU_IDEN_INFO_T *pstEcuIdenInfo = NULL;
    pstEcuIdenInfo = (ADM_DIM_SOA_ECU_IDEN_INFO_T *)adm_malloc(sizeof(
                         ADM_DIM_SOA_ECU_IDEN_INFO_T));
    pstEcuIdenInfo->requestId = 0x6E2;
    pstEcuIdenInfo->responseId = 0x6D9;
    pstEcuIdenInfo->functionId = 0x400;
    pstEcuIdenInfo->swDid = 0x0001;

    ADM_DIM_SOA_DID_DATA_ARRAY_T didListRequest;
    didListRequest.didArrayLength = 9;

    didListRequest.didArray = (ADM_DIM_SOA_DID_DATA_T **)adm_malloc(
                                  sizeof(ADM_DIM_SOA_DID_DATA_T *) * didListRequest.didArrayLength);

    didListRequest.didArray[0] = (ADM_DIM_SOA_DID_DATA_T *)adm_malloc(sizeof(
                                     ADM_DIM_SOA_DID_DATA_T));
    didListRequest.didArray[0]->did = 0xf188;
    didListRequest.didArray[0]->startByte = 0x00;
    didListRequest.didArray[0]->endByte = 0x0D;
    didListRequest.didArray[0]->errorCode = 0;
    didListRequest.didArray[0]->didData = (ADM_DIM_SOA_DATA_T*)adm_malloc(sizeof(ADM_DIM_SOA_DATA_T));
    didListRequest.didArray[0]->didData->dataType = VAR_STRING;
    //didListRequest.didArray[0]->didData->dataLength = 0xD;

    didListRequest.didArray[1] = (ADM_DIM_SOA_DID_DATA_T *)adm_malloc(sizeof(
                                     ADM_DIM_SOA_DID_DATA_T));
    didListRequest.didArray[1]->did = 0xf18c;
    didListRequest.didArray[1]->startByte = 0x00;
    didListRequest.didArray[1]->endByte = 0x0D;
    didListRequest.didArray[1]->errorCode = 0;
    didListRequest.didArray[1]->didData = (ADM_DIM_SOA_DATA_T*)adm_malloc(sizeof(ADM_DIM_SOA_DATA_T));
    didListRequest.didArray[1]->didData->dataType = VAR_STRING;

    didListRequest.didArray[2] = (ADM_DIM_SOA_DID_DATA_T *)adm_malloc(sizeof(
                                     ADM_DIM_SOA_DID_DATA_T));
    didListRequest.didArray[2]->did = 0xf193;
    didListRequest.didArray[2]->startByte = 0x00;
    didListRequest.didArray[2]->endByte = 0x0D;
    didListRequest.didArray[2]->errorCode = 0;
    didListRequest.didArray[2]->didData = (ADM_DIM_SOA_DATA_T*)adm_malloc(sizeof(ADM_DIM_SOA_DATA_T));
    didListRequest.didArray[2]->didData->dataType = VAR_STRING;

    didListRequest.didArray[3] = (ADM_DIM_SOA_DID_DATA_T *)adm_malloc(sizeof(
                                     ADM_DIM_SOA_DID_DATA_T));
    didListRequest.didArray[3]->did = 0xf180;
    didListRequest.didArray[3]->startByte = 0x00;
    didListRequest.didArray[3]->endByte = 0x0D;
    didListRequest.didArray[3]->errorCode = 0;
    didListRequest.didArray[3]->didData = (ADM_DIM_SOA_DATA_T*)adm_malloc(sizeof(ADM_DIM_SOA_DATA_T));
    didListRequest.didArray[3]->didData->dataType = VAR_STRING;

    didListRequest.didArray[4] = (ADM_DIM_SOA_DID_DATA_T *)adm_malloc(sizeof(
                                     ADM_DIM_SOA_DID_DATA_T));
    didListRequest.didArray[4]->did = 0xf182;
    didListRequest.didArray[4]->startByte = 0x00;
    didListRequest.didArray[4]->endByte = 0x0D;
    didListRequest.didArray[4]->errorCode = 0;
    didListRequest.didArray[4]->didData = (ADM_DIM_SOA_DATA_T*)adm_malloc(sizeof(ADM_DIM_SOA_DATA_T));
    didListRequest.didArray[4]->didData->dataType = VAR_STRING;

    didListRequest.didArray[5] = (ADM_DIM_SOA_DID_DATA_T *)adm_malloc(sizeof(
                                     ADM_DIM_SOA_DID_DATA_T));
    didListRequest.didArray[5]->did = 0xf187;
    didListRequest.didArray[5]->startByte = 0x00;
    didListRequest.didArray[5]->endByte = 0x0D;
    didListRequest.didArray[5]->errorCode = 0;
    didListRequest.didArray[5]->didData = (ADM_DIM_SOA_DATA_T*)adm_malloc(sizeof(ADM_DIM_SOA_DATA_T));
    didListRequest.didArray[5]->didData->dataType = VAR_STRING;

    didListRequest.didArray[6] = (ADM_DIM_SOA_DID_DATA_T *)adm_malloc(sizeof(
                                     ADM_DIM_SOA_DID_DATA_T));
    didListRequest.didArray[6]->did = 0xf18b;
    didListRequest.didArray[6]->startByte = 0x00;
    didListRequest.didArray[6]->endByte = 0x0D;
    didListRequest.didArray[6]->errorCode = 0;
    didListRequest.didArray[6]->didData = (ADM_DIM_SOA_DATA_T*)adm_malloc(sizeof(ADM_DIM_SOA_DATA_T));
    didListRequest.didArray[6]->didData->dataType = VAR_STRING;

    didListRequest.didArray[7] = (ADM_DIM_SOA_DID_DATA_T *)adm_malloc(sizeof(
                                     ADM_DIM_SOA_DID_DATA_T));
    didListRequest.didArray[7]->did = 0xf18a;
    didListRequest.didArray[7]->startByte = 0x00;
    didListRequest.didArray[7]->endByte = 0x0D;
    didListRequest.didArray[7]->errorCode = 0;
    didListRequest.didArray[7]->didData = (ADM_DIM_SOA_DATA_T*)adm_malloc(sizeof(ADM_DIM_SOA_DATA_T));
    didListRequest.didArray[7]->didData->dataType = VAR_STRING;
    
    didListRequest.didArray[8] = (ADM_DIM_SOA_DID_DATA_T *)adm_malloc(sizeof(
                                     ADM_DIM_SOA_DID_DATA_T));
    didListRequest.didArray[8]->did = 0xf191;
    didListRequest.didArray[8]->startByte = 0x00;
    didListRequest.didArray[8]->endByte = 0x0D;
    didListRequest.didArray[8]->errorCode = 0;
    didListRequest.didArray[8]->didData = (ADM_DIM_SOA_DATA_T*)adm_malloc(sizeof(ADM_DIM_SOA_DATA_T));
    didListRequest.didArray[8]->didData->dataType = VAR_STRING;

    char *diagScriptPath = NULL;
    diagScriptPath = adm_strdup("/renzhendi/test/vditest/vditestconfig/Read_ECUinfo.ttb");
    //diagScriptPath = adm_strdup("/sdcard/app/ota/admota/ucm/unzip/confPkg/Read_ECUinfo_14.ttb");
    ADM_DIM_SOA_VDI_BUS_TYPE_E eBusType = VEHICLE_BUS_TYPE_ETH;
    ADM_DIM_SOA_PIN_CHANNEL_TYPE_E eChannelType = VEHICLE_BUS_PIN_TYPE_A_CAN;
    DBOOL isCGWRouted = FALSE;
    ADM_DIM_SOA_DID_DATA_ARRAY_T *didListResponse = NULL;
    D32S errcode;//无效出参

    D32S retVal = adm_dim_vdi_readDiagnosticInfo(adm_soa_getVdiInstanceId(), 5,
                  pstEcuIdenInfo, diagScriptPath, eBusType, eChannelType, isCGWRouted,
                  didListRequest, &didListResponse, &errcode);

    //req free
    for (int index = 0; index < didListRequest.didArrayLength; ++index)
    {
        adm_free(didListRequest.didArray[index]->didData);
        adm_free(didListRequest.didArray[index]);
    }
    adm_free(didListRequest.didArray);
    adm_free(diagScriptPath);

    //响应在外层释放 ?
    if (didListResponse)
    {
        log_i("notify msgIdArrayLength = %d", didListResponse->didArrayLength);

        for (int i = 0; i < didListResponse->didArrayLength; i++)
        {
            log_i("notify[%d] did = 0x%x", i, didListResponse->didArray[i]->did);
            log_i("notify startbyte = 0x%x", didListResponse->didArray[i]->startByte);
            log_i("notify endbyte = 0x%x", didListResponse->didArray[i]->endByte);
            log_i("notify dataLength = %d",
                  didListResponse->didArray[i]->didData->dataLength);
            log_i("notify dataType = %d", didListResponse->didArray[i]->didData->dataType);
            if(VAR_STRING == 
                didListResponse->didArray[i]->didData->dataType)
            {
                log_i("notify dataValue = %s", didListResponse->didArray[i]->didData->v.valueS);
            }
        }
        //resp free
        for (int index = 0; index < didListResponse->didArrayLength; ++index)
        {
            adm_free(didListResponse->didArray[index]->didData);
            adm_free(didListResponse->didArray[index]);
        }

        adm_free(didListResponse->didArray);
        adm_free(didListResponse);
    }

    if (retVal != ADM_ECODE_SUCCESSED)
    {
        log_w("call adm_dim_soa_vdi_readDiagnosticInfo fail retVal=%d", retVal);
        return  ADM_ECODE_FAILED;
    }

    log_i("adm_dim_vdi_readDiagnosticInfo_test end");

    return ADM_ECODE_SUCCESSED;
}


D32S adm_dim_vdi_reflashEcusTrigger_Test()
{
    log_i("adm_dim_vdi_reflashEcusTrigger_Test begin");

    D32S errorCode = 0;
    ADM_DIM_SOA_VDI_REFLASH_INFO_ARRAY_T *reflashInfoList =
        (ADM_DIM_SOA_VDI_REFLASH_INFO_ARRAY_T *)adm_malloc(sizeof(
                    ADM_DIM_SOA_VDI_REFLASH_INFO_ARRAY_T));

    reflashInfoList->reflashInfoArrayLength = 1;
    reflashInfoList->reflashInfoArray = (ADM_DIM_SOA_VDI_REFLASH_INFO_T **)
                                        adm_malloc(sizeof(ADM_DIM_SOA_VDI_REFLASH_INFO_T));
    reflashInfoList->reflashInfoArray[0] = (ADM_DIM_SOA_VDI_REFLASH_INFO_T *)
                                           adm_malloc(sizeof(ADM_DIM_SOA_VDI_REFLASH_INFO_T));
    reflashInfoList->reflashInfoArray[0]->pstEcuIdenInfo =
        (ADM_DIM_SOA_ECU_IDEN_INFO_T *)adm_malloc(sizeof(
                    ADM_DIM_SOA_ECU_IDEN_INFO_T));
    reflashInfoList->reflashInfoArray[0]->pstEcuIdenInfo->requestId = 0x6E2;
    reflashInfoList->reflashInfoArray[0]->pstEcuIdenInfo->responseId = 0x6D9;
    reflashInfoList->reflashInfoArray[0]->pstEcuIdenInfo->functionId = 0x400;
    reflashInfoList->reflashInfoArray[0]->pstEcuIdenInfo->swDid = 0x0001;

    reflashInfoList->reflashInfoArray[0]->srcSoftVersion = adm_strdup("/opt/fota");
    reflashInfoList->reflashInfoArray[0]->dstSoftVersion = adm_strdup("/opt/fota");
    reflashInfoList->reflashInfoArray[0]->flashScriptPath = 
        adm_strdup("/renzhendi/test/vditest/vditestconfig/fresh.ttb");
    reflashInfoList->reflashInfoArray[0]->eBusType = VEHICLE_BUS_TYPE_CAN;
    reflashInfoList->reflashInfoArray[0]->isCGWRouted = 0x01;
    reflashInfoList->reflashInfoArray[0]->eChannelType = VEHICLE_BUS_PIN_TYPE_A_CAN;
    //reflashInfoList->reflashInfoArray[0]->saMask = 0xFFFFFFFF;
    reflashInfoList->reflashInfoArray[0]->saMask = 0x731203AB;

    reflashInfoList->reflashInfoArray[0]->saLibList =
        (ADM_SYSI4VDI_SA_LIB_INFO_ARRAY_T *)adm_malloc(sizeof(
                    ADM_SYSI4VDI_SA_LIB_INFO_ARRAY_T));
    reflashInfoList->reflashInfoArray[0]->saLibList->saLibArrayNum = 1;
    reflashInfoList->reflashInfoArray[0]->saLibList->saLibArray =
        (ADM_SYSI4VDI_SA_LIB_INFO_T **)adm_malloc(sizeof(ADM_SYSI4VDI_SA_LIB_INFO_T* ) * 1);
    reflashInfoList->reflashInfoArray[0]->saLibList->saLibArray[0] =
        (ADM_SYSI4VDI_SA_LIB_INFO_T *)adm_malloc(sizeof(ADM_SYSI4VDI_SA_LIB_INFO_T));
    
    reflashInfoList->reflashInfoArray[0]->saLibList->saLibArray[0]->saLibPath = 
        adm_strdup("/renzhendi/test/vditest/vditestconfig/libdpc_sa.hex");
    reflashInfoList->reflashInfoArray[0]->crcMask = CRC_TYPE_CRC32;

    //reflashInfoList->reflashInfoArray[0]->crcLibPath = adm_strdup("path11");
    reflashInfoList->reflashInfoArray[0]->crcLibPath = NULL;

    reflashInfoList->reflashInfoArray[0]->imageInfoList =
        (ADM_DIM_SOA_VDI_IMAGE_INFO_ARRAY_T *)adm_malloc(sizeof(ADM_DIM_SOA_VDI_IMAGE_INFO_ARRAY_T));
    
    reflashInfoList->reflashInfoArray[0]->imageInfoList->imageInfoArrayLength = 1;
    reflashInfoList->reflashInfoArray[0]->imageInfoList->imageInfoArray =
        (ADM_DIM_SOA_VDI_IMAGE_INFO_T **)adm_malloc(sizeof(ADM_DIM_SOA_VDI_IMAGE_INFO_T*) * 1);

    reflashInfoList->reflashInfoArray[0]->imageInfoList->imageInfoArray[0] =
        (ADM_DIM_SOA_VDI_IMAGE_INFO_T *)adm_malloc(sizeof(ADM_DIM_SOA_VDI_IMAGE_INFO_T));

    //无升级包vdi dump
    //解压是minizip,使用zip cbfTest.zip file1 file2压缩
    reflashInfoList->reflashInfoArray[0]->imageInfoList->imageInfoArray[0]->imagePath
        //= adm_strdup("/data/renzhendi/sysi/etc/vditestconfig/GE12A.s19");
        = adm_strdup("/renzhendi/tmp/cbfTest.zip");
    reflashInfoList->reflashInfoArray[0]->imageInfoList->imageInfoArray[0]->imageHash
        = adm_strdup("12345678");
    reflashInfoList->reflashInfoArray[0]->imageInfoList->imageInfoArray[0]->imageSize = 2408;

    D32S retVal = adm_dim_vdi_reflashEcusTrigger(adm_soa_getVdiInstanceId(), 5,
                  reflashInfoList, &errorCode);
    log_i("errorCode=%u", errorCode);

    if (retVal != ADM_ECODE_SUCCESSED)
    {
        log_w("call adm_dim_vdi_reflashEcusTrigger fail");
        return ADM_ECODE_FAILED;
    }

    for (int i = 0; i < reflashInfoList->reflashInfoArrayLength; i++)
    {
        // j, k
        adm_free(reflashInfoList->reflashInfoArray[i]->srcSoftVersion);
        adm_free(reflashInfoList->reflashInfoArray[i]->dstSoftVersion);
        adm_free(reflashInfoList->reflashInfoArray[i]->flashScriptPath);
        adm_free(reflashInfoList->reflashInfoArray[i]->crcLibPath);

        adm_free(reflashInfoList->reflashInfoArray[i]->imageInfoList->imageInfoArray[0]->imagePath);
        adm_free(reflashInfoList->reflashInfoArray[i]->imageInfoList->imageInfoArray[0]->imageHash);
        adm_free(reflashInfoList->reflashInfoArray[i]->imageInfoList->imageInfoArray[0]);
        adm_free(reflashInfoList->reflashInfoArray[i]->imageInfoList->imageInfoArray);
        adm_free(reflashInfoList->reflashInfoArray[i]->imageInfoList);

        adm_free(reflashInfoList->reflashInfoArray[i]->saLibList->saLibArray[0]->saLibPath);
        adm_free(reflashInfoList->reflashInfoArray[i]->saLibList->saLibArray[0]);
        adm_free(reflashInfoList->reflashInfoArray[i]->saLibList->saLibArray);
        adm_free(reflashInfoList->reflashInfoArray[i]->saLibList);
        adm_free(reflashInfoList->reflashInfoArray[i]);
    }
    adm_free(reflashInfoList->reflashInfoArray);
    adm_free(reflashInfoList);

    log_i("adm_dim_vdi_reflashEcusTrigger_Test end");
    return ADM_ECODE_SUCCESSED;
}


D32S adm_dim_vdi_reflashEcusTrigger_Test2()
{
    log_i("adm_dim_vdi_reflashEcusTrigger_Test begin");

    D32S errorCode = 0;
    ADM_DIM_SOA_VDI_REFLASH_INFO_ARRAY_T *reflashInfoList =
        (ADM_DIM_SOA_VDI_REFLASH_INFO_ARRAY_T *)adm_malloc(sizeof(
                    ADM_DIM_SOA_VDI_REFLASH_INFO_ARRAY_T));

    reflashInfoList->reflashInfoArrayLength = 1;
    reflashInfoList->reflashInfoArray = (ADM_DIM_SOA_VDI_REFLASH_INFO_T **)
                                        adm_malloc(sizeof(ADM_DIM_SOA_VDI_REFLASH_INFO_T));
    reflashInfoList->reflashInfoArray[0] = (ADM_DIM_SOA_VDI_REFLASH_INFO_T *)
                                           adm_malloc(sizeof(ADM_DIM_SOA_VDI_REFLASH_INFO_T));
    reflashInfoList->reflashInfoArray[0]->pstEcuIdenInfo =
        (ADM_DIM_SOA_ECU_IDEN_INFO_T *)adm_malloc(sizeof(
                    ADM_DIM_SOA_ECU_IDEN_INFO_T));
    reflashInfoList->reflashInfoArray[0]->pstEcuIdenInfo->requestId = 0x6E1;
    reflashInfoList->reflashInfoArray[0]->pstEcuIdenInfo->responseId = 0x6D8;
    reflashInfoList->reflashInfoArray[0]->pstEcuIdenInfo->functionId = 0x400;
    reflashInfoList->reflashInfoArray[0]->pstEcuIdenInfo->swDid = 0x0001;

    reflashInfoList->reflashInfoArray[0]->srcSoftVersion = adm_strdup("/opt/fota");
    reflashInfoList->reflashInfoArray[0]->dstSoftVersion = adm_strdup("/opt/fota");
    reflashInfoList->reflashInfoArray[0]->flashScriptPath = NULL;
        //adm_strdup("/renzhendi/test/vditest/fresh.ttb");
    reflashInfoList->reflashInfoArray[0]->eBusType = VEHICLE_BUS_TYPE_CAN;
    reflashInfoList->reflashInfoArray[0]->isCGWRouted = 0x01;
    reflashInfoList->reflashInfoArray[0]->eChannelType =
        VEHICLE_BUS_PIN_TYPE_A_CAN;
    reflashInfoList->reflashInfoArray[0]->saMask = 0x731203AB;

    reflashInfoList->reflashInfoArray[0]->saLibList =
        (ADM_SYSI4VDI_SA_LIB_INFO_ARRAY_T *)adm_malloc(sizeof(
                    ADM_SYSI4VDI_SA_LIB_INFO_ARRAY_T));
    reflashInfoList->reflashInfoArray[0]->saLibList->saLibArrayNum = 1;
    reflashInfoList->reflashInfoArray[0]->saLibList->saLibArray =
        (ADM_SYSI4VDI_SA_LIB_INFO_T **)adm_malloc(sizeof(ADM_SYSI4VDI_SA_LIB_INFO_T* ) * 1);
    reflashInfoList->reflashInfoArray[0]->saLibList->saLibArray[0] =
        (ADM_SYSI4VDI_SA_LIB_INFO_T *)adm_malloc(sizeof(ADM_SYSI4VDI_SA_LIB_INFO_T));
    
    reflashInfoList->reflashInfoArray[0]->saLibList->saLibArray[0]->saLibPath = 
        adm_strdup("/renzhendi/vdicode/x86_vdicode/3th/libdpc_sa.hex");
    //reflashInfoList->reflashInfoArray[0]->saLibList->saLibArray[0]->saLibPath = NULL;
    reflashInfoList->reflashInfoArray[0]->crcMask = CRC_TYPE_CRC32;

    //reflashInfoList->reflashInfoArray[0]->crcLibPath = adm_strdup("path11");
    reflashInfoList->reflashInfoArray[0]->crcLibPath = NULL;

    reflashInfoList->reflashInfoArray[0]->imageInfoList =
        (ADM_DIM_SOA_VDI_IMAGE_INFO_ARRAY_T *)adm_malloc(sizeof(ADM_DIM_SOA_VDI_IMAGE_INFO_ARRAY_T));
    
    reflashInfoList->reflashInfoArray[0]->imageInfoList->imageInfoArrayLength = 1;
    reflashInfoList->reflashInfoArray[0]->imageInfoList->imageInfoArray =
        (ADM_DIM_SOA_VDI_IMAGE_INFO_T **)adm_malloc(sizeof(ADM_DIM_SOA_VDI_IMAGE_INFO_T*) * 1);

    reflashInfoList->reflashInfoArray[0]->imageInfoList->imageInfoArray[0] =
        (ADM_DIM_SOA_VDI_IMAGE_INFO_T *)adm_malloc(sizeof(ADM_DIM_SOA_VDI_IMAGE_INFO_T));
    
    reflashInfoList->reflashInfoArray[0]->imageInfoList->imageInfoArray[0]->imagePath =
        adm_strdup("/renzhendi/test/vditest/vditestconfig/GE12A.s19");
    reflashInfoList->reflashInfoArray[0]->imageInfoList->imageInfoArray[0]->imageHash
        = adm_strdup("12345678");
    reflashInfoList->reflashInfoArray[0]->imageInfoList->imageInfoArray[0]->imageSize = 2408;

    D32S retVal = adm_dim_vdi_reflashEcusTrigger(adm_soa_getVdiInstanceId(), 5,
                  reflashInfoList, &errorCode);
    log_i("errorCode=%u", errorCode);

    if (retVal != ADM_ECODE_SUCCESSED)
    {
        log_w("call adm_dim_vdi_reflashEcusTrigger fail");
        return ADM_ECODE_FAILED;
    }

    for (int i = 0; i < reflashInfoList->reflashInfoArrayLength; i++)
    {
        // j, k
        adm_free(reflashInfoList->reflashInfoArray[i]->srcSoftVersion);
        adm_free(reflashInfoList->reflashInfoArray[i]->dstSoftVersion);
        adm_free(reflashInfoList->reflashInfoArray[i]->flashScriptPath);
        adm_free(reflashInfoList->reflashInfoArray[i]->crcLibPath);

        adm_free(reflashInfoList->reflashInfoArray[i]->imageInfoList->imageInfoArray[0]->imagePath);
        adm_free(reflashInfoList->reflashInfoArray[i]->imageInfoList->imageInfoArray[0]->imageHash);
        adm_free(reflashInfoList->reflashInfoArray[i]->imageInfoList->imageInfoArray[0]);
        adm_free(reflashInfoList->reflashInfoArray[i]->imageInfoList->imageInfoArray);
        adm_free(reflashInfoList->reflashInfoArray[i]->imageInfoList);

        adm_free(reflashInfoList->reflashInfoArray[i]->saLibList->saLibArray[0]->saLibPath);
        adm_free(reflashInfoList->reflashInfoArray[i]->saLibList->saLibArray[0]);
        adm_free(reflashInfoList->reflashInfoArray[i]->saLibList->saLibArray);
        adm_free(reflashInfoList->reflashInfoArray[i]->saLibList);
        adm_free(reflashInfoList->reflashInfoArray[i]);
    }
    adm_free(reflashInfoList->reflashInfoArray);
    adm_free(reflashInfoList);

    log_i("adm_dim_vdi_reflashEcusTrigger_Test end");
    return ADM_ECODE_SUCCESSED;
}


D32S adm_dim_vdi_queryReflashProgress_Test()
{
    log_i("adm_dim_vdi_queryReflashProgress_Test begin");
    ADM_DIM_SOA_ECU_IDEN_INFO_T *pstEcuIdenInfo = NULL;
    pstEcuIdenInfo = (ADM_DIM_SOA_ECU_IDEN_INFO_T *)adm_malloc(sizeof(
                         ADM_DIM_SOA_ECU_IDEN_INFO_T));
    pstEcuIdenInfo->requestId = 0x16e2;
    pstEcuIdenInfo->responseId = 0x1000;
    pstEcuIdenInfo->functionId = 0xe400;
    pstEcuIdenInfo->swDid = 0x0001;
    D16U reflashProgress = 0;
    D32S errorCode = 0;
    D32S retVal = adm_dim_vdi_queryReflashProgress(adm_soa_getVdiInstanceId(), 5,
                  pstEcuIdenInfo, &reflashProgress, &errorCode);
    if (retVal != ADM_ECODE_SUCCESSED)
    {
        log_w("call adm_dim_vdi_queryReflashProgress fail");
        log_i("adm_dim_vdi_queryReflashProgress_Test end");
        return ADM_ECODE_FAILED;
    }
    log_i("reflashProgress = %d", reflashProgress);
    log_i("adm_dim_vdi_queryReflashProgress_Test end");
    return ADM_ECODE_SUCCESSED;
}


D32S adm_dim_vdi_queryReflashResult_Test()
{
    log_i("adm_dim_vdi_queryReflashResult_Test begin");
    ADM_DIM_SOA_ECU_IDEN_INFO_T *pstEcuIdenInfo = NULL;
    pstEcuIdenInfo = (ADM_DIM_SOA_ECU_IDEN_INFO_T *)adm_malloc(sizeof(
                         ADM_DIM_SOA_ECU_IDEN_INFO_T));
    if (!pstEcuIdenInfo)
    {
        log_e("malloc fail: pstEcuIdenInfo");
        return ADM_ECODE_FAILED;
    }

    pstEcuIdenInfo->requestId = 0x16e2;
    pstEcuIdenInfo->responseId = 0x1000;
    pstEcuIdenInfo->functionId = 0xe400;
    pstEcuIdenInfo->swDid = 1;

    D32S errorCode = 0;

    ADM_DIM_SOA_VDI_REFLASH_RESULT_INFO_T *pRefreshResultInfo = NULL;
    D32S retVal = adm_dim_vdi_queryReflashResult(adm_soa_getVdiInstanceId(), 5,
                  pstEcuIdenInfo, &pRefreshResultInfo, &errorCode);
    if (retVal != ADM_ECODE_SUCCESSED)
    {
        log_w("call adm_dim_vdi_queryReflashResult fail");
        adm_free(pstEcuIdenInfo);
        adm_free(pRefreshResultInfo);

        log_i("adm_dim_vdi_queryReflashResult_Test end");
        return ADM_ECODE_FAILED;
    }
    if (!pRefreshResultInfo)
    {
        log_e("malloc fail: pRefreshResultInfo");
        adm_free(pstEcuIdenInfo);
        return ADM_ECODE_INVALID_POINTER;
    }

    log_i("resp errorcode = %d", errorCode);
    log_i("reflashResult result = %d", pRefreshResultInfo->result);
    if (pRefreshResultInfo->expectVal)
        log_i("reflashResult expectVal = %s", pRefreshResultInfo->expectVal);
    else
        log_e("reflashResult->expectVal NULL");

    if (pRefreshResultInfo->realVal)
        log_i("reflashResult realVal = %s", pRefreshResultInfo->realVal);
    else
        log_e("reflashResult->realVal NULL");

    adm_free(pRefreshResultInfo->expectVal);
    adm_free(pRefreshResultInfo->realVal);
    adm_free(pRefreshResultInfo);

    log_i("adm_dim_vdi_queryReflashResult_Test end");
    return ADM_ECODE_SUCCESSED;
}




D32S adm_dim_vdi_performSpecDiagnosticTask_Test()
{
    log_i("adm_dim_vdi_performSpecDiagnosticTask_Test begin");

    ADM_DIM_SOA_VDI_SPEC_DIAGNOSTIC_TASK_TYPE_E eSpecDiagTaskType = ADM_DIM_SOA_VDI_SPEC_DIAGNOSTIC_TASK_TYPE_FUNC_SAFE_VEFIFY;
    DCHAR* absolutePath = adm_strdup("/aaa/bbb");

    D32S errorCode;
    D32S retVal = adm_dim_vdi_performSpecDiagnosticTask(adm_soa_getVdiInstanceId(), 5,eSpecDiagTaskType, absolutePath, &errorCode);
    if( retVal != ADM_ECODE_SUCCESSED)
    {
        log_w("call adm_dim_vdi_performSpecDiagnosticTask fail");
        return ADM_ECODE_FAILED;
    }
    adm_free(absolutePath);

    log_i("adm_dim_vdi_performSpecDiagnosticTask_Test end");

    return ADM_ECODE_SUCCESSED;
}


D32S adm_dim_vdi_readVehicleStatus_Test()
{
    log_i("adm_dim_vdi_readVehicleStatus_Test begin");

    D32S errorCode = 0;
    
    ADM_DIM_SOA_VDI_MSGID_DATA_ARRAY_T msgIdListRequest;
    ADM_DIM_SOA_VDI_MSGID_DATA_ARRAY_T* msgIdListResponse = NULL;
    msgIdListRequest.msgIdArrayLength = 10;

    msgIdListRequest.msgIdArray = (ADM_DIM_SOA_VDI_MSGID_T**)adm_malloc(sizeof(ADM_DIM_SOA_VDI_MSGID_T*) * 
        msgIdListRequest.msgIdArrayLength);
    
    //1--cds入参只带msgId,修改增加带msgIdData
    //2--cds修改增加带msgIdData
    msgIdListRequest.msgIdArray[0] = (ADM_DIM_SOA_VDI_MSGID_T*)adm_malloc(sizeof(ADM_DIM_SOA_VDI_MSGID_T));
    msgIdListRequest.msgIdArray[0]->msgId = VEHICLE_STATUS_DRIVE_MODE;
    msgIdListRequest.msgIdArray[0]->msgIdData = (ADM_DIM_SOA_DATA_T*)adm_malloc(sizeof(ADM_DIM_SOA_DATA_T));
    msgIdListRequest.msgIdArray[0]->msgIdData->dataType = 2;
    msgIdListRequest.msgIdArray[0]->msgIdData->dataLength = 4;
    msgIdListRequest.msgIdArray[0]->msgIdData->v.valueS32 = 1;

    msgIdListRequest.msgIdArray[1] = (ADM_DIM_SOA_VDI_MSGID_T*)adm_malloc(sizeof(ADM_DIM_SOA_VDI_MSGID_T));
    msgIdListRequest.msgIdArray[1]->msgId = VEHICLE_STATUS_VSPD;
    msgIdListRequest.msgIdArray[1]->msgIdData = (ADM_DIM_SOA_DATA_T*)adm_malloc(sizeof(ADM_DIM_SOA_DATA_T));
    msgIdListRequest.msgIdArray[1]->msgIdData->dataType = 2;
    msgIdListRequest.msgIdArray[1]->msgIdData->dataLength = 4;
    msgIdListRequest.msgIdArray[1]->msgIdData->v.valueS32 = 0;

    msgIdListRequest.msgIdArray[2] = (ADM_DIM_SOA_VDI_MSGID_T*)adm_malloc(sizeof(ADM_DIM_SOA_VDI_MSGID_T));
    msgIdListRequest.msgIdArray[2]->msgId = VEHICLE_STATUS_ENGINE_SPD;
    msgIdListRequest.msgIdArray[2]->msgIdData = (ADM_DIM_SOA_DATA_T*)adm_malloc(sizeof(ADM_DIM_SOA_DATA_T));
    msgIdListRequest.msgIdArray[2]->msgIdData->dataType = 2;
    msgIdListRequest.msgIdArray[2]->msgIdData->dataLength = 4;
    msgIdListRequest.msgIdArray[2]->msgIdData->v.valueS32 = 0;
    
    msgIdListRequest.msgIdArray[3] = (ADM_DIM_SOA_VDI_MSGID_T*)adm_malloc(sizeof(ADM_DIM_SOA_VDI_MSGID_T));
    msgIdListRequest.msgIdArray[3]->msgId = VEHICLE_STATUS_SOC;
    msgIdListRequest.msgIdArray[3]->msgIdData = (ADM_DIM_SOA_DATA_T*)adm_malloc(sizeof(ADM_DIM_SOA_DATA_T));
    msgIdListRequest.msgIdArray[3]->msgIdData->dataType = 2;
    msgIdListRequest.msgIdArray[3]->msgIdData->dataLength = 4;
    msgIdListRequest.msgIdArray[3]->msgIdData->v.valueS32 = 70;

    msgIdListRequest.msgIdArray[4] = (ADM_DIM_SOA_VDI_MSGID_T*)adm_malloc(sizeof(ADM_DIM_SOA_VDI_MSGID_T));
    msgIdListRequest.msgIdArray[4]->msgId = VEHICLE_STATUS_POWER_SOC;
    msgIdListRequest.msgIdArray[4]->msgIdData = (ADM_DIM_SOA_DATA_T*)adm_malloc(sizeof(ADM_DIM_SOA_DATA_T));
    msgIdListRequest.msgIdArray[4]->msgIdData->dataType = 2;
    msgIdListRequest.msgIdArray[4]->msgIdData->dataLength = 4;
    msgIdListRequest.msgIdArray[4]->msgIdData->v.valueS32 = 80;

    msgIdListRequest.msgIdArray[5] = (ADM_DIM_SOA_VDI_MSGID_T*)adm_malloc(sizeof(ADM_DIM_SOA_VDI_MSGID_T));
    msgIdListRequest.msgIdArray[5]->msgId = VEHICLE_STATUS_IGN_STATUS;
    msgIdListRequest.msgIdArray[5]->msgIdData = (ADM_DIM_SOA_DATA_T*)adm_malloc(sizeof(ADM_DIM_SOA_DATA_T));
    msgIdListRequest.msgIdArray[5]->msgIdData->dataType = 2;
    msgIdListRequest.msgIdArray[5]->msgIdData->dataLength = 4;
    msgIdListRequest.msgIdArray[5]->msgIdData->v.valueS32 = 1;

    msgIdListRequest.msgIdArray[6] = (ADM_DIM_SOA_VDI_MSGID_T*)adm_malloc(sizeof(ADM_DIM_SOA_VDI_MSGID_T));
    msgIdListRequest.msgIdArray[6]->msgId = VEHICLE_STATUS_GEAR;
    msgIdListRequest.msgIdArray[6]->msgIdData = (ADM_DIM_SOA_DATA_T*)adm_malloc(sizeof(ADM_DIM_SOA_DATA_T));
    msgIdListRequest.msgIdArray[6]->msgIdData->dataType = 2;
    msgIdListRequest.msgIdArray[6]->msgIdData->dataLength = 4;
    msgIdListRequest.msgIdArray[6]->msgIdData->v.valueS32 = 0;

    msgIdListRequest.msgIdArray[7] = (ADM_DIM_SOA_VDI_MSGID_T*)adm_malloc(sizeof(ADM_DIM_SOA_VDI_MSGID_T));
    msgIdListRequest.msgIdArray[7]->msgId = VEHICLE_STATUS_EPBLOCK;
    msgIdListRequest.msgIdArray[7]->msgIdData = (ADM_DIM_SOA_DATA_T*)adm_malloc(sizeof(ADM_DIM_SOA_DATA_T));
    msgIdListRequest.msgIdArray[7]->msgIdData->dataType = 2;
    msgIdListRequest.msgIdArray[7]->msgIdData->dataLength = 4;
    msgIdListRequest.msgIdArray[7]->msgIdData->v.valueS32 = 1;

    msgIdListRequest.msgIdArray[8] = (ADM_DIM_SOA_VDI_MSGID_T*)adm_malloc(sizeof(ADM_DIM_SOA_VDI_MSGID_T));
    msgIdListRequest.msgIdArray[8]->msgId = VEHICLE_STATUS_CHARGING;
    msgIdListRequest.msgIdArray[8]->msgIdData = (ADM_DIM_SOA_DATA_T*)adm_malloc(sizeof(ADM_DIM_SOA_DATA_T));
    msgIdListRequest.msgIdArray[8]->msgIdData->dataType = 2;
    msgIdListRequest.msgIdArray[8]->msgIdData->dataLength = 4;
    msgIdListRequest.msgIdArray[8]->msgIdData->v.valueS32 = 0;

    msgIdListRequest.msgIdArray[9] = (ADM_DIM_SOA_VDI_MSGID_T*)adm_malloc(sizeof(ADM_DIM_SOA_VDI_MSGID_T));
    msgIdListRequest.msgIdArray[9]->msgId = VEHICLE_STATUS_VOL_MV;
    msgIdListRequest.msgIdArray[9]->msgIdData = (ADM_DIM_SOA_DATA_T*)adm_malloc(sizeof(ADM_DIM_SOA_DATA_T));
    msgIdListRequest.msgIdArray[9]->msgIdData->dataType = 2;
    msgIdListRequest.msgIdArray[9]->msgIdData->dataLength = 4;
    msgIdListRequest.msgIdArray[9]->msgIdData->v.valueS32 = 20;
    
    D32S retVal = adm_dim_vdi_readVehicleStatus(adm_soa_getVdiInstanceId(), 5,
                  &msgIdListRequest, &msgIdListResponse, &errorCode);

    log_i("resp errorcode = %d", errorCode);
    log_i("call adm_dim_vdi_readVehicleStatus retVal = %d",retVal);

    for (int index = 0; index < msgIdListRequest.msgIdArrayLength; ++index)
    {
        adm_free(msgIdListRequest.msgIdArray[index]->msgIdData);
        adm_free(msgIdListRequest.msgIdArray[index]);
    }

    adm_free(msgIdListRequest.msgIdArray);
    
    if(msgIdListResponse)
    {
        for (int index = 0; index < msgIdListResponse->msgIdArrayLength; ++index)
        {
            //log_i("notify retData2 = %s",msgIdListResponse->msgIdArray[index]->msgIdData->v.valueS); //test string
            log_i("msgId=0x%x", msgIdListResponse->msgIdArray[index]->msgId);
            log_i("valueU32=%u", msgIdListResponse->msgIdArray[index]->msgIdData->v.valueU32);
            adm_free(msgIdListResponse->msgIdArray[index]->msgIdData);
            adm_free(msgIdListResponse->msgIdArray[index]);
        }

        adm_free(msgIdListResponse->msgIdArray);
        adm_free(msgIdListResponse);
    }

    log_i("adm_dim_vdi_readVehicleStatus_Test end");

    return ADM_ECODE_SUCCESSED;
}

//树莓派连续读取2次vdi crash 用例
D32S adm_dim_vdi_readVehicleStatus_Test2()
{
    log_i("adm_dim_vdi_readVehicleStatus_Test begin");

    D32S errorCode = 0;
    
    ADM_DIM_SOA_VDI_MSGID_DATA_ARRAY_T msgIdListRequest;
    ADM_DIM_SOA_VDI_MSGID_DATA_ARRAY_T* msgIdListResponse = NULL;
    msgIdListRequest.msgIdArrayLength = 10;

    msgIdListRequest.msgIdArray = (ADM_DIM_SOA_VDI_MSGID_T**)adm_malloc(sizeof(ADM_DIM_SOA_VDI_MSGID_T*) * 
        msgIdListRequest.msgIdArrayLength);
    
    //1--cds入参只带msgId,修改增加带msgIdData
    //2--cds修改增加带msgIdData
    msgIdListRequest.msgIdArray[0] = (ADM_DIM_SOA_VDI_MSGID_T*)adm_malloc(sizeof(ADM_DIM_SOA_VDI_MSGID_T));
    msgIdListRequest.msgIdArray[0]->msgId = VEHICLE_STATUS_DRIVE_MODE;
    msgIdListRequest.msgIdArray[0]->msgIdData = (ADM_DIM_SOA_DATA_T*)adm_malloc(sizeof(ADM_DIM_SOA_DATA_T));
    msgIdListRequest.msgIdArray[0]->msgIdData->dataType = 0;
    msgIdListRequest.msgIdArray[0]->msgIdData->dataLength = 0;
    //msgIdListRequest.msgIdArray[0]->msgIdData->v.valueS32 = 70;

    msgIdListRequest.msgIdArray[1] = (ADM_DIM_SOA_VDI_MSGID_T*)adm_malloc(sizeof(ADM_DIM_SOA_VDI_MSGID_T));
    msgIdListRequest.msgIdArray[1]->msgId = VEHICLE_STATUS_VSPD;
    msgIdListRequest.msgIdArray[1]->msgIdData = (ADM_DIM_SOA_DATA_T*)adm_malloc(sizeof(ADM_DIM_SOA_DATA_T));
    msgIdListRequest.msgIdArray[1]->msgIdData->dataType = VAR_U8;
    msgIdListRequest.msgIdArray[1]->msgIdData->dataLength = 3;
    //msgIdListRequest.msgIdArray[1]->msgIdData->v.valueS32 = 100;

    msgIdListRequest.msgIdArray[2] = (ADM_DIM_SOA_VDI_MSGID_T*)adm_malloc(sizeof(ADM_DIM_SOA_VDI_MSGID_T));
    msgIdListRequest.msgIdArray[2]->msgId = VEHICLE_STATUS_ENGINE_SPD;
    msgIdListRequest.msgIdArray[2]->msgIdData = (ADM_DIM_SOA_DATA_T*)adm_malloc(sizeof(ADM_DIM_SOA_DATA_T));
    msgIdListRequest.msgIdArray[2]->msgIdData->dataType = VAR_U32;
    msgIdListRequest.msgIdArray[2]->msgIdData->dataLength = sizeof(D32U);
    //msgIdListRequest.msgIdArray[2]->msgIdData->v.valueS32 = 100;
    
    msgIdListRequest.msgIdArray[3] = (ADM_DIM_SOA_VDI_MSGID_T*)adm_malloc(sizeof(ADM_DIM_SOA_VDI_MSGID_T));
    msgIdListRequest.msgIdArray[3]->msgId = VEHICLE_STATUS_SOC;
    msgIdListRequest.msgIdArray[3]->msgIdData = (ADM_DIM_SOA_DATA_T*)adm_malloc(sizeof(ADM_DIM_SOA_DATA_T));
    msgIdListRequest.msgIdArray[3]->msgIdData->dataType = VAR_FLOAT;
    msgIdListRequest.msgIdArray[3]->msgIdData->dataLength = sizeof(DFLOAT);
    //msgIdListRequest.msgIdArray[3]->msgIdData->v.valueS32 = 100;

    msgIdListRequest.msgIdArray[4] = (ADM_DIM_SOA_VDI_MSGID_T*)adm_malloc(sizeof(ADM_DIM_SOA_VDI_MSGID_T));
    msgIdListRequest.msgIdArray[4]->msgId = VEHICLE_STATUS_POWER_SOC;
    msgIdListRequest.msgIdArray[4]->msgIdData = (ADM_DIM_SOA_DATA_T*)adm_malloc(sizeof(ADM_DIM_SOA_DATA_T));
    msgIdListRequest.msgIdArray[4]->msgIdData->dataType = VAR_FLOAT;
    msgIdListRequest.msgIdArray[4]->msgIdData->dataLength = sizeof(DFLOAT);
    //msgIdListRequest.msgIdArray[4]->msgIdData->v.valueS32 = 100;

    msgIdListRequest.msgIdArray[5] = (ADM_DIM_SOA_VDI_MSGID_T*)adm_malloc(sizeof(ADM_DIM_SOA_VDI_MSGID_T));
    msgIdListRequest.msgIdArray[5]->msgId = VEHICLE_STATUS_IGN_STATUS;
    msgIdListRequest.msgIdArray[5]->msgIdData = (ADM_DIM_SOA_DATA_T*)adm_malloc(sizeof(ADM_DIM_SOA_DATA_T));
    msgIdListRequest.msgIdArray[5]->msgIdData->dataType = VAR_U32;
    msgIdListRequest.msgIdArray[5]->msgIdData->dataLength = sizeof(D32U);
    //msgIdListRequest.msgIdArray[5]->msgIdData->v.valueS32 = 100;

    msgIdListRequest.msgIdArray[6] = (ADM_DIM_SOA_VDI_MSGID_T*)adm_malloc(sizeof(ADM_DIM_SOA_VDI_MSGID_T));
    msgIdListRequest.msgIdArray[6]->msgId = VEHICLE_STATUS_GEAR;
    msgIdListRequest.msgIdArray[6]->msgIdData = (ADM_DIM_SOA_DATA_T*)adm_malloc(sizeof(ADM_DIM_SOA_DATA_T));
    msgIdListRequest.msgIdArray[6]->msgIdData->dataType = VAR_S32;
    msgIdListRequest.msgIdArray[6]->msgIdData->dataLength = sizeof(D32S);
    //msgIdListRequest.msgIdArray[6]->msgIdData->v.valueS32 = 100;

    msgIdListRequest.msgIdArray[7] = (ADM_DIM_SOA_VDI_MSGID_T*)adm_malloc(sizeof(ADM_DIM_SOA_VDI_MSGID_T));
    msgIdListRequest.msgIdArray[7]->msgId = VEHICLE_STATUS_EPBLOCK;
    msgIdListRequest.msgIdArray[7]->msgIdData = (ADM_DIM_SOA_DATA_T*)adm_malloc(sizeof(ADM_DIM_SOA_DATA_T));
    msgIdListRequest.msgIdArray[7]->msgIdData->dataType = VAR_U64;
    msgIdListRequest.msgIdArray[7]->msgIdData->dataLength = sizeof(D64U);
    //msgIdListRequest.msgIdArray[7]->msgIdData->v.valueS32 = 100;

    msgIdListRequest.msgIdArray[8] = (ADM_DIM_SOA_VDI_MSGID_T*)adm_malloc(sizeof(ADM_DIM_SOA_VDI_MSGID_T));
    msgIdListRequest.msgIdArray[8]->msgId = VEHICLE_STATUS_CHARGING;
    msgIdListRequest.msgIdArray[8]->msgIdData = (ADM_DIM_SOA_DATA_T*)adm_malloc(sizeof(ADM_DIM_SOA_DATA_T));
    msgIdListRequest.msgIdArray[8]->msgIdData->dataType = VAR_FLOAT;
    msgIdListRequest.msgIdArray[8]->msgIdData->dataLength = sizeof(DFLOAT);
    //msgIdListRequest.msgIdArray[8]->msgIdData->v.valueS32 = 100;

    msgIdListRequest.msgIdArray[9] = (ADM_DIM_SOA_VDI_MSGID_T*)adm_malloc(sizeof(ADM_DIM_SOA_VDI_MSGID_T));
    msgIdListRequest.msgIdArray[9]->msgId = VEHICLE_STATUS_VOL_MV;
    msgIdListRequest.msgIdArray[9]->msgIdData = (ADM_DIM_SOA_DATA_T*)adm_malloc(sizeof(ADM_DIM_SOA_DATA_T));
    msgIdListRequest.msgIdArray[9]->msgIdData->dataType = VAR_STRING;
    msgIdListRequest.msgIdArray[9]->msgIdData->dataLength = 10;
    //msgIdListRequest.msgIdArray[9]->msgIdData->v.valueS32 = 100;
    
    D32S retVal = adm_dim_vdi_readVehicleStatus(adm_soa_getVdiInstanceId(), 5,
                  &msgIdListRequest, &msgIdListResponse, &errorCode);

    log_i("resp errorcode = %d", errorCode);
    log_i("call adm_dim_vdi_readVehicleStatus retVal = %d",retVal);

    for (int index = 0; index < msgIdListRequest.msgIdArrayLength; ++index)
    {
        adm_free(msgIdListRequest.msgIdArray[index]->msgIdData);
        adm_free(msgIdListRequest.msgIdArray[index]);
    }

    adm_free(msgIdListRequest.msgIdArray);
    
    if(msgIdListResponse)
    {
        for (int index = 0; index < msgIdListResponse->msgIdArrayLength; ++index)
        {
            //log_i("notify retData2 = %s",msgIdListResponse->msgIdArray[index]->msgIdData->v.valueS); //test string
            log_i("msgId=0x%x", msgIdListResponse->msgIdArray[index]->msgId);
            log_i("valueU32=%u", msgIdListResponse->msgIdArray[index]->msgIdData->v.valueU32);
            adm_free(msgIdListResponse->msgIdArray[index]->msgIdData);
            adm_free(msgIdListResponse->msgIdArray[index]);
        }

        adm_free(msgIdListResponse->msgIdArray);
        adm_free(msgIdListResponse);
    }

    log_i("adm_dim_vdi_readVehicleStatus_Test end");

    return ADM_ECODE_SUCCESSED;
}


D32S adm_dim_vdi_setSpyVehicleStatus_Test()
{
    log_i("adm_dim_vdi_setSpyVehicleStatus_Test begin");

    ADM_SYSI4VDI_ENGINE_SPY_ARRAY_T *msgIdListRequest = NULL;

    D32U u32ReqLen = sizeof(ADM_SYSI4VDI_ENGINE_SPY_ARRAY_T) + sizeof(
                         ADM_SYSI4VDI_ENGINE_SPY_T) * 2;

    msgIdListRequest = (ADM_SYSI4VDI_ENGINE_SPY_ARRAY_T *)adm_malloc(u32ReqLen);

    if (!(msgIdListRequest))
    {
        log_e("malloc fail: msgIdListRequest");
        return ADM_ECODE_INVALID_POINTER;
    }
    adm_memset(msgIdListRequest, 0, u32ReqLen, u32ReqLen);

    msgIdListRequest->msgIdArrayLength = 2;
    /* mod struct need mod data */
    msgIdListRequest->spyArray[0].vehicle_id = VEHICLE_STATUS_SOC;
    msgIdListRequest->spyArray[0].compare_symbol = 2;
    msgIdListRequest->spyArray[0].multi_relate = 1;

    //msgIdListRequest->spyArray[0].firstSpyData.dataType = VAR_U32;
    //msgIdListRequest->spyArray[0].firstSpyData.dataLength = sizeof(D32U);
    //msgIdListRequest->spyArray[0].firstSpyData.v.valueU32 = 80;
    msgIdListRequest->spyArray[0].firstSpyData.dataType = VAR_FLOAT;
    msgIdListRequest->spyArray[0].firstSpyData.dataLength = sizeof(DFLOAT);
    msgIdListRequest->spyArray[0].firstSpyData.v.valueF = 80;

    msgIdListRequest->spyArray[1].vehicle_id = VEHICLE_STATUS_POWER_SOC;
    msgIdListRequest->spyArray[1].compare_symbol = 2;
    msgIdListRequest->spyArray[1].multi_relate = 1;
    //msgIdListRequest->spyArray[1].firstSpyData.dataType = VAR_U32;
    //msgIdListRequest->spyArray[1].firstSpyData.dataLength = sizeof(D32U);
    //msgIdListRequest->spyArray[1].firstSpyData.v.valueU32 = 82;
    msgIdListRequest->spyArray[1].firstSpyData.dataType = VAR_FLOAT;
    msgIdListRequest->spyArray[1].firstSpyData.dataLength = sizeof(DFLOAT);
    msgIdListRequest->spyArray[1].firstSpyData.v.valueF = 82;

    D64U u64TaskId = 0x01;
    D32S errorCode;
    D32S retVal = adm_dim_vdi_setSpyVehicleStatus(adm_soa_getVdiInstanceId(), 5,
                  u64TaskId,
                  msgIdListRequest, &errorCode);
    log_i("errorCode = %u", errorCode);

    if (retVal != ADM_ECODE_SUCCESSED)
    {
        log_w("call adm_dim_vdi_setSpyVehicleStatus fail");

        log_i("adm_dim_vdi_setSpyVehicleStatus_Test end");
        return ADM_ECODE_FAILED;
    }

    adm_free(msgIdListRequest);

    log_i("adm_dim_vdi_setSpyVehicleStatus_Test end");

    return ADM_ECODE_SUCCESSED;
}


D32S adm_dim_vdi_clearSpyVehicleStatus_Test()
{
    log_i("adm_dim_vdi_clearSpyVehicleStatus_Test begin");

    D32S errorCode;

    D64U u64TaskId = 1;
    D32S retVal = adm_dim_vdi_clearSpyVehicleStatus(adm_soa_getVdiInstanceId(), 5,
                  u64TaskId, &errorCode);
    log_i("errorCode = %u", errorCode);

    if (retVal != ADM_ECODE_SUCCESSED)
    {
        log_w("call adm_dim_vdi_clearSpyVehicleStatus fail");
        return ADM_ECODE_FAILED;
    }

    log_i("adm_dim_vdi_clearSpyVehicleStatus_Test end");

    return ADM_ECODE_SUCCESSED;

}



D32S adm_dim_vdi_controlVehicleStatus_Test()
{
    log_i("adm_dim_vdi_controlVehicleStatus_Test begin");

    ADM_DIM_SOA_VDI_CTRL_VEHICLE_STATUS_TYPE_E eVechielStatusType =
        VEHICLE_CTRL_ENTER_HV_MODE;

    ADM_DIM_SOA_DATA_T *variable = NULL;//reserved is null
    //variable = (ADM_DIM_SOA_DATA_T*)adm_malloc(sizeof(ADM_DIM_SOA_DATA_T));
    //variable->dataType = VAR_U32;
    //variable->dataLength = sizeof(D32U);
    //variable->v.valueU32 = 0x10;

    D32S errorCode;
    D32S retVal = adm_dim_vdi_controlVehicleStatus(adm_soa_getVdiInstanceId(), 5,
                  eVechielStatusType, variable,
                  0, &errorCode);
    log_i("errorCode=%d", errorCode);
    if (retVal != ADM_ECODE_SUCCESSED)
    {
        log_w("call adm_dim_vdi_controlVehicleStatus fail");

        return ADM_ECODE_FAILED;
    }

    adm_free(variable);

    log_i("adm_dim_vdi_controlVehicleStatus_Test end");

    return ADM_ECODE_SUCCESSED;
}


D32S adm_dim_vdi_softwareVersionCheckProcess_Test()
{
    log_i("adm_dim_vdi_softwareVersionCheckProcess_Test begin");

    ADM_DIM_SOA_ECU_IDEN_INFO_T *pstEcuIdenInfo =
        (ADM_DIM_SOA_ECU_IDEN_INFO_T *)adm_malloc(sizeof(
                    ADM_DIM_SOA_ECU_IDEN_INFO_T));
    pstEcuIdenInfo->requestId = 10;
    pstEcuIdenInfo->responseId = 20;
    pstEcuIdenInfo->swDid = 30;

    const ADM_DIM_SOA_VDI_SOFT_VERSION_CHECK_TYPE_E eCheckType =
        ADM_DIM_SOA_VDI_SOFT_VERSION_CHECK_TYPE_OLD;
    const DBOOL isEnterBorkenMode = 1;
    D32S errorCode = 0;

    adm_dim_vdi_softwareVersionCheckProcess(adm_soa_getVdiInstanceId(), 5,
                                            pstEcuIdenInfo, eCheckType, isEnterBorkenMode, &errorCode);

    log_i("adm_dim_vdi_softwareVersionCheckProcess_Test end");

    return ADM_ECODE_SUCCESSED;
}

static void request_dim_vdi_set_log_level()
{
    log_i("request_dim_vdi_set_log_level********vdi test");
    D16U instanceId = 0X1001;
    D32S errorCode = 1;
    D32U logLevel = 3;
    log_i("req errorCode = %d", errorCode);
    D32S ret = adm_dim_vdi_set_log_level(instanceId, 5, logLevel, &errorCode);
    log_i("ret = %d", ret);
    log_i("resp errorCode = %d", errorCode);
}

static void request_dim_vdi_GetSdkVersion()
{
    log_i("request_dim_vdi_GetSdkVersion********vdi test");
    D16U instanceId = 0X1001;
    DCHAR *sdkVersion = NULL;
    D32S errorCode = 1;
    adm_dim_vdi_getSdkVersion(instanceId, 5, &sdkVersion, &errorCode);

    log_i("sdkVersion = %s", sdkVersion);
    log_i("resp errorCode = %d", errorCode);

    adm_free(sdkVersion);
    
    log_i("request_dim_vdi_GetSdkVersion end");
}


D32S adm_dim_reflash_progress_notify(const ADM_DIM_SOA_ECU_IDEN_INFO_T
                                     *pstIdenInfo, const D16U *reflashProgress, void *priv)
{
    log_i("adm_dim_reflash_progress_notify begin");

    log_i("notify requestId = 0x%x", pstIdenInfo->requestId);
    log_i("notify responseId = 0x%x", pstIdenInfo->responseId);
    log_i("notify functionId = 0x%x", pstIdenInfo->functionId);
    log_i("notify swDid = 0x%x", pstIdenInfo->swDid);
    log_i("notify reflashProgress = %d", *reflashProgress);

    log_i("adm_dim_reflash_progress_notify end");
    return 0;
}

D32S adm_dim_reflash_result_notify(const ADM_DIM_SOA_ECU_IDEN_INFO_T
                                   *pstIdenInfo, const ADM_DIM_SOA_VDI_REFLASH_RESULT_INFO_T
                                   *pstReflashResultSendInfo, void *priv)
{
    log_i("adm_dim_reflash_result_notify begin");

    log_i("notify requestId = 0x%x", pstIdenInfo->requestId);
    log_i("notify responseId = 0x%x", pstIdenInfo->responseId);
    log_i("notify functionId = 0x%x", pstIdenInfo->functionId);
    log_i("notify swDid = 0x%x", pstIdenInfo->swDid);
    log_i("notify result = %d", pstReflashResultSendInfo->result);
    log_i("notify expectVal = %s", pstReflashResultSendInfo->expectVal);
    log_i("notify realVal = %s", pstReflashResultSendInfo->realVal);

    log_i("adm_dim_reflash_result_notify end");
    return 0;
}

D32S adm_dim_spy_vehicle_status_notify(D64U u64TaskId, void *priv)
{
    log_i("adm_dim_spy_vehicle_status_notify begin");

    log_i("adm_dim_spy_vehicle_status_notify u64TaskId=%llu", u64TaskId);

    log_i("adm_dim_spy_vehicle_status_notify end");
    return 0;
}

D32S adm_dim_vehicle_signal_event_notify(const
        ADM_DIM_SOA_VDI_VEHICLE_SIGNAL_TRIGGER_TYPE_E eVehicleSignalTriggerType)
{
    log_i("adm_dim_vehicle_signal_event_notify begin");

    log_i("notify vehicle signal = 0x%x", eVehicleSignalTriggerType);

    log_i("adm_dim_vehicle_signal_event_notify end");
    return 0;
}

void adm_sysi4vdi_read_diagnostic_info_notify(const
        ADM_DIM_SOA_ECU_IDEN_INFO_T *pstEcuIdenInfo,
        const ADM_DIM_SOA_DID_DATA_ARRAY_T *didListResponse)
{
    log_i("adm_sysi4vdi_read_diagnostic_info_notify begin");

    log_i("notify msgIdArrayLength = %d", didListResponse->didArrayLength);

    for (int i = 0; i < didListResponse->didArrayLength; i++)
    {
        log_i("notify did = %d", didListResponse->didArray[i]->did);
        log_i("notify startbyte = %d", didListResponse->didArray[i]->startByte);
        log_i("notify endbyte = %d", didListResponse->didArray[i]->endByte);
        log_i("notify dataLength = %d",
              didListResponse->didArray[i]->didData->dataLength);
        log_i("notify dataType = %d", didListResponse->didArray[i]->didData->dataType);
        log_i("notify valueS32 = %s", didListResponse->didArray[i]->didData->v.valueS);
    }

    log_i("adm_sysi4vdi_read_diagnostic_info_notify end");
}

D32S adm_vdi_simulatorInit()
{
    log_i("adm_vdi_simulatorInit begin");

    ADM_DIM_SOA_VDI_INIT_INFO_T initInfo;
    initInfo.readDiagInfoCallback = adm_sysi4vdi_read_diagnostic_info_notify;
    initInfo.reflashProgressCallback = adm_dim_reflash_progress_notify;
    initInfo.reflashResultCallback = adm_dim_reflash_result_notify;
    initInfo.spyVehicleStatusCallback = adm_dim_spy_vehicle_status_notify;
    initInfo.vehicleSignalEventCallback = adm_dim_vehicle_signal_event_notify;

    void *priv = NULL;
    adm_dim_vdi_init(&initInfo, priv);

    log_i("adm_vdi_simulatorInit end");

    return 0;
}

D32S adm_vdi_simulatorExec()
{
    adm_dim_vdi_reflashEcusTrigger_Test();

    #if 0
    adm_dim_vdi_reflashEcusTrigger_Test();
    
    adm_dim_vdi_reflashEcusTrigger_Test();    //根据vdi提供库调试日志显示，刷写触发通知约17s
    wait_sleep(2);
    adm_dim_vdi_queryReflashProgress_Test();  //刷写没有触发通知时，请求Progress
    adm_dim_vdi_queryReflashResult_Test();    //刷写没有触发通知时，请求Result
    wait_sleep(10);
    adm_dim_vdi_queryReflashProgress_Test(); //刷写触发Progress通知后，请求Progress
    adm_dim_vdi_queryReflashResult_Test();   //刷写触发Progress通知后，没到100，请求Result
    adm_dim_vdi_readDiagnosticInfo_test();    //half ok
    wait_sleep(10);

    adm_dim_vdi_reflashEcusTrigger_Test();
    wait_sleep(15);
    adm_dim_vdi_readDiagnosticInfo_test();
    wait_sleep(10);

    adm_dim_vdi_readVehicleStatus_Test();     //OK
    adm_dim_vdi_setSpyVehicleStatus_Test();     //ok
    wait_sleep(8);
    adm_dim_vdi_clearSpyVehicleStatus_Test();  //OK
    adm_dim_vdi_controlVehicleStatus_Test();  //OK

    request_dim_vdi_set_log_level();
    
    request_dim_vdi_GetSdkVersion();
    #endif

    log_i("adm_vdi_simulatorExec end");

    return 0;
}

D32S adm_vdi_simulatorDeinit()
{
    log_i("adm_vdi_simulatorDeinit begin");

    adm_dim_vdi_deinit();

    adm_debug_logDeinit();
    //adm_trace_exit();

    log_i("adm_vdi_simulatorDeinit end");

    return 0;
}

