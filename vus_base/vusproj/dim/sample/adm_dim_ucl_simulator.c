

#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#include "adm_dim_ucl_simulator.h"
#include "adm_typedefs.h"
#include "adm_memory.h"
#include "adm_string.h"
#include "adm_dim_log.h"

#include "adm_com_errorCode.h"
#include "adm_dim_log.h"

#include "adm_soa.h"
#include "adm_soa_cfg.h"
#include "adm_dim_common.h"

#include "adm_cfg_api.h"
#include "adm_com_lib.h"
#include "adm_dim_ucl.h"

#define TIME_WAIT 5

//request
D32S readPartInformation()
{
    D32S retCode = 0;
    D32S errorCode = 0;
    ADM_DIM_SOA_DID_DATA_ARRAY_T* didListResponse = NULL;
    ADM_DIM_SOA_ECU_IDEN_INFO_T pstEcuIdenInfo = {0};
    pstEcuIdenInfo.requestId = 0x1234;

    ADM_DIM_SOA_DID_DATA_ARRAY_T didArrayRequest = {0};
    didArrayRequest.didArrayLength = 5;

    didArrayRequest.didArray =
        (ADM_DIM_SOA_DID_DATA_T**)adm_malloc(sizeof(ADM_DIM_SOA_DID_DATA_T*) * didArrayRequest.didArrayLength);
    didArrayRequest.didArray[0] = (ADM_DIM_SOA_DID_DATA_T*)adm_malloc(sizeof(ADM_DIM_SOA_DID_DATA_T));
    didArrayRequest.didArray[0]->did = 0xF187;
    didArrayRequest.didArray[0]->startByte = 0;
    didArrayRequest.didArray[0]->endByte = 13;
    didArrayRequest.didArray[0]->didData = (ADM_DIM_SOA_DATA_T*)adm_malloc(sizeof(ADM_DIM_SOA_DATA_T));
    didArrayRequest.didArray[0]->didData->dataLength = 14;
    didArrayRequest.didArray[0]->didData->dataType = VAR_STRING;

    didArrayRequest.didArray[1] = (ADM_DIM_SOA_DID_DATA_T*)adm_malloc(sizeof(ADM_DIM_SOA_DID_DATA_T));
    didArrayRequest.didArray[1]->did = 0xF189;
    didArrayRequest.didArray[1]->startByte = 0;
    didArrayRequest.didArray[1]->endByte = 13;
    didArrayRequest.didArray[1]->didData = (ADM_DIM_SOA_DATA_T*)adm_malloc(sizeof(ADM_DIM_SOA_DATA_T));
    didArrayRequest.didArray[1]->didData->dataLength = 14;
    didArrayRequest.didArray[1]->didData->dataType = VAR_STRING;

    didArrayRequest.didArray[2] = (ADM_DIM_SOA_DID_DATA_T*)adm_malloc(sizeof(ADM_DIM_SOA_DID_DATA_T));
    didArrayRequest.didArray[2]->did = 0xF089;
    didArrayRequest.didArray[2]->startByte = 0;
    didArrayRequest.didArray[2]->endByte = 13;
    didArrayRequest.didArray[2]->didData = (ADM_DIM_SOA_DATA_T*)adm_malloc(sizeof(ADM_DIM_SOA_DATA_T));
    didArrayRequest.didArray[2]->didData->dataLength = 14;
    didArrayRequest.didArray[2]->didData->dataType = VAR_STRING;

    didArrayRequest.didArray[3] = (ADM_DIM_SOA_DID_DATA_T*)adm_malloc(sizeof(ADM_DIM_SOA_DID_DATA_T));
    didArrayRequest.didArray[3]->did = 0xF18C;
    didArrayRequest.didArray[3]->startByte = 0;
    didArrayRequest.didArray[3]->endByte = 13;
    didArrayRequest.didArray[3]->didData = (ADM_DIM_SOA_DATA_T*)adm_malloc(sizeof(ADM_DIM_SOA_DATA_T));
    didArrayRequest.didArray[3]->didData->dataLength = 14;
    didArrayRequest.didArray[3]->didData->dataType = VAR_STRING;

    didArrayRequest.didArray[4] = (ADM_DIM_SOA_DID_DATA_T*)adm_malloc(sizeof(ADM_DIM_SOA_DID_DATA_T));
    didArrayRequest.didArray[4]->did = 0xF013;
    didArrayRequest.didArray[4]->startByte = 0;
    didArrayRequest.didArray[4]->endByte = 13;
    didArrayRequest.didArray[4]->didData = (ADM_DIM_SOA_DATA_T*)adm_malloc(sizeof(ADM_DIM_SOA_DATA_T));
    didArrayRequest.didArray[4]->didData->dataLength = 14;
    didArrayRequest.didArray[4]->didData->dataType = VAR_STRING;

    //走ucl代码
    retCode = adm_dim_ucl_readPartInformation(adm_soa_getUclInstanceId(), TIME_WAIT,
              &pstEcuIdenInfo, &didArrayRequest, &didListResponse, &errorCode);
    log_i("ret code: %d\n", retCode);

    if(retCode == 0)
    {
        log_e("dim get sysi resp,readPartInformation succeed");
    }
    else
    {
        log_e("dim not get sysi resp,readPartInformation failed");
    }

    for(D16U index = 0; index < didArrayRequest.didArrayLength; ++index)
    {
        if((NULL != didArrayRequest.didArray) && (NULL != didArrayRequest.didArray[index]))
        {
            adm_free(didArrayRequest.didArray[index]->didData);
            adm_free(didArrayRequest.didArray[index]);
        }
    }

    adm_free(didArrayRequest.didArray);
    return retCode;
}
//adm_dim_ucl_requestDoUpgrade
D32S requestDoUpgrade()
{
    log_i("requestDoUpgrade begin");

    D32S errorCode  = 0;

    ADM_DIM_SOA_UCL_UPGRADE_INFO_ARRAY_T* doUpgradeList = NULL;

    do
    {
        //if 1
        D32U arr = 1;
        doUpgradeList = (ADM_DIM_SOA_UCL_UPGRADE_INFO_ARRAY_T*)adm_malloc(sizeof(ADM_DIM_SOA_UCL_UPGRADE_INFO_ARRAY_T));

        if(!doUpgradeList)
        {
            log_e("malloc fail: doUpgradeList");
            break;
        }

        adm_memset(doUpgradeList, 0x0,
                   sizeof(ADM_DIM_SOA_UCL_UPGRADE_INFO_ARRAY_T),
                   sizeof(ADM_DIM_SOA_UCL_UPGRADE_INFO_ARRAY_T));
        doUpgradeList->upgradeInfoArrayLength = arr;

        doUpgradeList->upgradeInfoArray = (ADM_DIM_SOA_UCL_UPGRADE_INFO_T**)adm_malloc(
                                              sizeof(ADM_DIM_SOA_UCL_UPGRADE_INFO_T*) *
                                              doUpgradeList->upgradeInfoArrayLength);

        if(!doUpgradeList->upgradeInfoArray)
        {
            log_e("malloc fail: doUpgradeList->upgradeInfoArray");
            break;
        }

        adm_memset(doUpgradeList->upgradeInfoArray, 0x0,
                   sizeof(ADM_DIM_SOA_UCL_UPGRADE_INFO_T*) * doUpgradeList->upgradeInfoArrayLength,
                   sizeof(ADM_DIM_SOA_UCL_UPGRADE_INFO_T*) * doUpgradeList->upgradeInfoArrayLength);
        //upgrade info 1
        doUpgradeList->upgradeInfoArray[0] =
            (ADM_DIM_SOA_UCL_UPGRADE_INFO_T*)adm_malloc(sizeof(ADM_DIM_SOA_UCL_UPGRADE_INFO_T));
        doUpgradeList->upgradeInfoArray[0]->pstEcuIdenInfo =
            (ADM_DIM_SOA_ECU_IDEN_INFO_T*)adm_malloc(sizeof(ADM_DIM_SOA_ECU_IDEN_INFO_T));
        doUpgradeList->upgradeInfoArray[0]->pstEcuIdenInfo->requestId = 0x1111;
        doUpgradeList->upgradeInfoArray[0]->pstEcuIdenInfo->responseId = 0x1112;
        doUpgradeList->upgradeInfoArray[0]->pstEcuIdenInfo->functionId = 0x1113;
        doUpgradeList->upgradeInfoArray[0]->pstEcuIdenInfo->swDid = 0x14;

        doUpgradeList->upgradeInfoArray[0]->srcSoftVersion = adm_strdup("V1.1.1");
        doUpgradeList->upgradeInfoArray[0]->dstSoftVersion = adm_strdup("V1.1.2");

        doUpgradeList->upgradeInfoArray[0]->imageInfoList =
            (ADM_DIM_SOA_UCL_IMAGE_INFO_ARRAY_T*)adm_malloc(sizeof(ADM_DIM_SOA_UCL_IMAGE_INFO_ARRAY_T));
        doUpgradeList->upgradeInfoArray[0]->imageInfoList->imageInfoArrayLength = arr;

        doUpgradeList->upgradeInfoArray[0]->imageInfoList->imageInfoArray =
            (ADM_DIM_SOA_UCL_IMAGE_INFO_T**)adm_malloc(sizeof(ADM_DIM_SOA_UCL_IMAGE_INFO_T*) *
                    doUpgradeList->upgradeInfoArray[0]->imageInfoList->imageInfoArrayLength);
        //upgrade info 1-image info 1
        doUpgradeList->upgradeInfoArray[0]->imageInfoList->imageInfoArray[0] =
            (ADM_DIM_SOA_UCL_IMAGE_INFO_T*)adm_malloc(sizeof(ADM_DIM_SOA_UCL_IMAGE_INFO_T));
        doUpgradeList->upgradeInfoArray[0]->imageInfoList->imageInfoArray[0]->imagePath
            = adm_strdup("/renzhendi/tmp/cbfTest.zip");
        doUpgradeList->upgradeInfoArray[0]->imageInfoList->imageInfoArray[0]->imageHash
            = adm_strdup("hash12345678901_1");
        doUpgradeList->upgradeInfoArray[0]->imageInfoList->imageInfoArray[0]->imageSize = 11011;

        /*
            //upgrade info 1-image info 2
            doUpgradeList->upgradeInfoArray[0]->imageInfoList->imageInfoArray[1] =
                        (ADM_DIM_SOA_UCL_IMAGE_INFO_T*)adm_malloc(sizeof(ADM_DIM_SOA_UCL_IMAGE_INFO_T));
            doUpgradeList->upgradeInfoArray[0]->imageInfoList->imageInfoArray[1]->imagePath =
                        adm_strdup("/renzhendi/tmpdir/ucltest/upgrade/upgpkt.zip1_2");
            doUpgradeList->upgradeInfoArray[0]->imageInfoList->imageInfoArray[1]->imageHash =
                        adm_strdup("hash12345678901_2");
            doUpgradeList->upgradeInfoArray[0]->imageInfoList->imageInfoArray[1]->imageSize = 11022;

            //upgrade info 2
            doUpgradeList->upgradeInfoArray[1] =
                    (ADM_DIM_SOA_UCL_UPGRADE_INFO_T*)adm_malloc(sizeof(ADM_DIM_SOA_UCL_UPGRADE_INFO_T));
            doUpgradeList->upgradeInfoArray[1]->pstEcuIdenInfo =
                    (ADM_DIM_SOA_ECU_IDEN_INFO_T*)adm_malloc(sizeof(ADM_DIM_SOA_ECU_IDEN_INFO_T));
            doUpgradeList->upgradeInfoArray[1]->pstEcuIdenInfo->requestId = 0x2221;
            doUpgradeList->upgradeInfoArray[1]->pstEcuIdenInfo->responseId = 0x2222;
            doUpgradeList->upgradeInfoArray[1]->pstEcuIdenInfo->functionId = 0x2223;
            doUpgradeList->upgradeInfoArray[1]->pstEcuIdenInfo->swDid = 0x24;

            doUpgradeList->upgradeInfoArray[1]->srcSoftVersion = adm_strdup("V2.1.1");
            doUpgradeList->upgradeInfoArray[1]->dstSoftVersion = adm_strdup("V2.1.2");

            doUpgradeList->upgradeInfoArray[1]->imageInfoList =
                    (ADM_DIM_SOA_UCL_IMAGE_INFO_ARRAY_T*)adm_malloc(sizeof(ADM_DIM_SOA_UCL_IMAGE_INFO_ARRAY_T));
            doUpgradeList->upgradeInfoArray[1]->imageInfoList->imageInfoArrayLength = 2;

            doUpgradeList->upgradeInfoArray[1]->imageInfoList->imageInfoArray =
                    (ADM_DIM_SOA_UCL_IMAGE_INFO_T**)adm_malloc(sizeof(ADM_DIM_SOA_UCL_IMAGE_INFO_T*) *
                    doUpgradeList->upgradeInfoArray[1]->imageInfoList->imageInfoArrayLength);
            //upgrade info 2-image info 1
            doUpgradeList->upgradeInfoArray[1]->imageInfoList->imageInfoArray[0] =
                        (ADM_DIM_SOA_UCL_IMAGE_INFO_T*)adm_malloc(sizeof(ADM_DIM_SOA_UCL_IMAGE_INFO_T));
            doUpgradeList->upgradeInfoArray[1]->imageInfoList->imageInfoArray[0]->imagePath =
                        adm_strdup("/renzhendi/tmpdir/ucltest/upgrade/upgpkt.zip2_1");
            doUpgradeList->upgradeInfoArray[1]->imageInfoList->imageInfoArray[0]->imageHash =
                        adm_strdup("hash12345678902_1");
            doUpgradeList->upgradeInfoArray[1]->imageInfoList->imageInfoArray[0]->imageSize = 22011;
            //upgrade info 2-image info 2
            doUpgradeList->upgradeInfoArray[1]->imageInfoList->imageInfoArray[1] =
                        (ADM_DIM_SOA_UCL_IMAGE_INFO_T*)adm_malloc(sizeof(ADM_DIM_SOA_UCL_IMAGE_INFO_T));
            doUpgradeList->upgradeInfoArray[1]->imageInfoList->imageInfoArray[1]->imagePath =
                        adm_strdup("/renzhendi/tmpdir/ucltest/upgrade/upgpkt.zip2_2");
            doUpgradeList->upgradeInfoArray[1]->imageInfoList->imageInfoArray[1]->imageHash =
                        adm_strdup("hash12345678902_2");
            doUpgradeList->upgradeInfoArray[1]->imageInfoList->imageInfoArray[1]->imageSize = 22022;
        */

    }
    while(0);

    D32S ret = adm_dim_ucl_requestDoUpgrade(adm_soa_getUclInstanceId(), TIME_WAIT,
                                            doUpgradeList, &errorCode);
    log_i("ret = %d", ret);
    log_i("errorCode = %d", errorCode);

    //free
    if(doUpgradeList)
    {
        if(doUpgradeList->upgradeInfoArray)
        {
            for(int i = 0; i < doUpgradeList->upgradeInfoArrayLength; i++)
            {
                if(doUpgradeList->upgradeInfoArray[i])
                {
                    adm_free(doUpgradeList->upgradeInfoArray[i]->pstEcuIdenInfo);
                    adm_free(doUpgradeList->upgradeInfoArray[i]->srcSoftVersion);
                    adm_free(doUpgradeList->upgradeInfoArray[i]->dstSoftVersion);
                }

                if(doUpgradeList->upgradeInfoArray[i]->imageInfoList)
                {
                    if(doUpgradeList->upgradeInfoArray[i]->imageInfoList->imageInfoArray)
                    {
                        for(int k = 0;
                                k < doUpgradeList->upgradeInfoArray[i]->imageInfoList->imageInfoArrayLength;
                                k++)
                        {
                            adm_free(doUpgradeList->upgradeInfoArray[i]->imageInfoList->imageInfoArray[k]->imageHash);
                            adm_free(doUpgradeList->upgradeInfoArray[i]->imageInfoList->imageInfoArray[k]->imagePath);
                            adm_free(doUpgradeList->upgradeInfoArray[i]->imageInfoList->imageInfoArray[k]);
                        }
                    }

                    adm_free(doUpgradeList->upgradeInfoArray[i]->imageInfoList->imageInfoArray);
                }

                adm_free(doUpgradeList->upgradeInfoArray[i]);
            }

            adm_free(doUpgradeList->upgradeInfoArray);
        }

        adm_free(doUpgradeList);
    }

    log_i("requestDoUpgrade end");

    return 0;
}

D32S queryUpgradeProgress()
{
    log_d("queryUpgradeProgress");
    ADM_DIM_SOA_ECU_IDEN_INFO_T pstEcuIdenInfo = {0};
    pstEcuIdenInfo.requestId = 0x1111;
    pstEcuIdenInfo.responseId = 0x1112;
    pstEcuIdenInfo.swDid = 0x14;
    D16U progress = 1;
    D32S errorCode = 1;
    D32S retCode = adm_dim_ucl_queryUpgradeProgress(adm_soa_getUclInstanceId(),
                   TIME_WAIT, &pstEcuIdenInfo, &progress, &errorCode);

    if(retCode == 0)
    {
        log_d("queryUpgradeProgress");
    }
    else
    {
        log_e("queryUpgradeProgress");
    }

    log_i("progress = %u", progress);

    return retCode;
}

D32S queryUpgradeResult()
{
    log_d("queryUpgradeResult");
    D32S errorCode = 1;
    ADM_DIM_SOA_ECU_IDEN_INFO_T pstEcuIdenInfo = {0};
    pstEcuIdenInfo.requestId = 0x1111;
    pstEcuIdenInfo.responseId = 0x1112;
    pstEcuIdenInfo.swDid = 0x14;
    ADM_DIM_SOA_UCL_UPGRADE_RESULT_INFO_T* pstUpgradeResultInfo = NULL;
    pstUpgradeResultInfo = (ADM_DIM_SOA_UCL_UPGRADE_RESULT_INFO_T*)adm_malloc(
                               sizeof(ADM_DIM_SOA_UCL_UPGRADE_RESULT_INFO_T));
    D32S retCode = adm_dim_ucl_queryUpgradeResult(adm_soa_getUclInstanceId(),
                   TIME_WAIT, &pstEcuIdenInfo, &pstUpgradeResultInfo, &errorCode);

    if(retCode == 0)
    {
        log_d("queryUpgradeResult");
    }
    else
    {
        log_e("queryUpgradeResult");
    }

    if(pstUpgradeResultInfo != NULL)
    {
        log_i("result = %d", pstUpgradeResultInfo->result);
        log_i("expectVal = %s", pstUpgradeResultInfo->expectVal);
        log_i("realVal = %s", pstUpgradeResultInfo->realVal);

        adm_free(pstUpgradeResultInfo->expectVal);
        adm_free(pstUpgradeResultInfo->realVal);
        adm_free(pstUpgradeResultInfo);
    }

    return retCode;
}

D32S requestDecryptPackage()
{
    log_e("dim开始发起请求 requestDecryptPackage\n");
    const DCHAR* fileID = "ugppkg.zip";
    //const DCHAR* filePath = "/renzhendi/tmpdir/ucltest/elog_file.log";
    //加密后文件
    const DCHAR* encryfile = "/renzhendi/test/ucltest/decrypt/2.zip";
    const DCHAR* decryptKey = "6c2cba8409a540eba7326e44f58696ce";
    D64S decryptSegLength = 1048576;
    DCHAR* hash = "f8901d73f1923d4699149c885f8d59be73993fda6693f55c06905493e207a61a";

    D32S errorCode = 1;
    D32S retCode = adm_dim_ucl_requestDecryptPackage(adm_soa_getUclInstanceId(),
                   TIME_WAIT, fileID, encryfile, decryptKey, decryptSegLength, hash, &errorCode);

    if(retCode == 0)
    {
        log_e("dim收到sysi回复， requestDecryptPackage 成功\n");
    }
    else
    {
        log_e("dim未收到sysi回复， requestDecryptPackage 失败\n");
    }

    return retCode;
}


D32S getDecryptPackageProgress()
{
    log_e("dim开始发起请求 getDecryptPackageProgress\n");
    const DCHAR* fileID = "fileId";
    ADM_DIM_SOA_UCL_DECRYPT_PROGRESS_INFO_T* pstDecryptProgressInfo = NULL;

    D32S errorCode = 1;
    D32S retCode = adm_dim_ucl_getDecryptPackageProgress(adm_soa_getUclInstanceId(),
                   TIME_WAIT, fileID, &pstDecryptProgressInfo, &errorCode);

    if(retCode == 0)
    {
        log_e("dim收到sysi回复， getDecryptPackageProgress 成功\n");
    }
    else
    {
        log_e("dim未收到sysi回复， getDecryptPackageProgress 失败\n");
    }

    if(pstDecryptProgressInfo != NULL)
    {
        log_i("Decrypt Progress Info. savedSize = %lld", pstDecryptProgressInfo->savedSize);
        log_i("Decrypt Progress Info. totalSize = %lld", pstDecryptProgressInfo->totalSize);
        log_i("Decrypt Progress Info. speed = %d", pstDecryptProgressInfo->speed);
        log_i("Decrypt Progress Info. remainTime = %d", pstDecryptProgressInfo->remainTime);

        adm_free(pstDecryptProgressInfo);
    }

    return retCode;
}

D32S getDecryptPackageResult()
{
    log_e("dim开始发起请求 getDecryptPackageResult\n");
    const DCHAR* fileID = "fileId";
    DCHAR* storagePath = NULL;
    ADM_DIM_SOA_UCL_DECRYPT_RESULT_INFO_T* pstDecryptResultInfo = NULL;

    D32S errorCode = 1;
    D32S retCode = adm_dim_ucl_getDecryptPackageResult(adm_soa_getUclInstanceId(),
                   TIME_WAIT, fileID, &storagePath, &pstDecryptResultInfo, &errorCode);

    if(retCode == 0)
    {
        log_e("dim收到sysi回复， getDecryptPackageResult 成功\n");
    }
    else
    {
        log_e("dim未收到sysi回复， getDecryptPackageResult 失败\n");
    }

    if(pstDecryptResultInfo != NULL)
    {
        log_i("storagePath = %s", storagePath);
        log_i("result = %d", pstDecryptResultInfo->result);
        log_i("expectVal = %s", pstDecryptResultInfo->expectVal);
        log_i("realVal = %s", pstDecryptResultInfo->realVal);

        adm_free(pstDecryptResultInfo);
        adm_free(storagePath);
    }

    return 0;
}


D32S requestVerifyHash()//adm_dim_ucl_requestVerifyHash
{
    log_e("dim开始发起请求 requestVerifyHash\n");
    const DCHAR* fileID = "fileId";
    const DCHAR* filePath = "/renzhendi/test/ucltest/verifyhash/VerifySign.log";
    const DCHAR* hash = "cedae7f3b360ca57077b009ea9d14aa3bd071c8a633d4e4b348d6cdd73efcbe9";
    D32S errorCode = 1;
    ADM_DIM_SOA_UCL_HASH_TYPE_E eHashType = ADM_DIM_SOA_UCL_HASH_TYPE_SHA256;
    D32S retCode = adm_dim_ucl_requestVerifyHash(adm_soa_getUclInstanceId(),
                   TIME_WAIT, fileID, filePath, eHashType, hash, &errorCode);

    if(retCode == 0)
    {
        log_e("dim收到sysi回复， requestVerifyHash 成功\n");
    }
    else
    {
        log_e("dim未收到sysi回复， requestVerifyHash 失败\n");
    }

    return 0;
}

D32S request_Verify_Sign()
{
    log_i("dim request Verify Sign begin ****");

    DCHAR* fileID = "1122VerifySign.log";
    DCHAR* filePath =
        "/renzhendi/test/ucltest/verifysign/VerifySign.log";   //size = 1048628

    DBOOL isUnpick = FALSE;    // TRUE需要拆包 FALSE不需要拆包

    D32S errorCode = 1;

    FILEMETAS_INFO_ARRAY_T* fileMetasList = adm_malloc(sizeof(
            FILEMETAS_INFO_ARRAY_T));

    if(NULL == fileMetasList)
    {
        log_e("fileMetasList is NULL");
        return ADM_ECODE_INVALID_POINTER;
    }

    adm_memset(fileMetasList, 0x0, sizeof(FILEMETAS_INFO_ARRAY_T),
               sizeof(FILEMETAS_INFO_ARRAY_T));

    fileMetasList->fileMetaNum = 2;//ori file + sign file
    fileMetasList->fileMetasArray = (FILEMETAS_INFO_T**)adm_malloc(sizeof(
                                        FILEMETAS_INFO_T*) * fileMetasList->fileMetaNum);
    adm_memset(fileMetasList->fileMetasArray, 0x0,
               sizeof(FILEMETAS_INFO_T*) * fileMetasList->fileMetaNum,
               sizeof(FILEMETAS_INFO_T*) * fileMetasList->fileMetaNum);

    fileMetasList->fileMetasArray[0] = adm_malloc(sizeof(FILEMETAS_INFO_T));
    adm_memset(fileMetasList->fileMetasArray[0], 0x0, sizeof(FILEMETAS_INFO_T),
               sizeof(FILEMETAS_INFO_T));

    fileMetasList->fileMetasArray[0]->size = 1048000;
    fileMetasList->fileMetasArray[0]->tObject.eAlgorthm =
        ADM_DIM_SOA_UCL_ALGORITHM_TYPE_SHA256;
    DCHAR* strOri =
        "cedae7f3b360ca57077b009ea9d14aa3bd071c8a633d4e4b348d6cdd73efcbe9";
    fileMetasList->fileMetasArray[0]->tObject.pAbstracValue = adm_strdup(strOri);
    fileMetasList->fileMetasArray[0]->efiletype = ADM_SIM_SOA_UCL_FILE_TYPE_ORI;

    fileMetasList->fileMetasArray[1] = adm_malloc(sizeof(FILEMETAS_INFO_T));
    adm_memset(fileMetasList->fileMetasArray[1], 0x0, sizeof(FILEMETAS_INFO_T),
               sizeof(FILEMETAS_INFO_T));

    fileMetasList->fileMetasArray[1]->size = 628;
    fileMetasList->fileMetasArray[1]->tObject.eAlgorthm =
        ADM_DIM_SOA_UCL_ALGORITHM_TYPE_SHA256;
    DCHAR* strSign =
        "a44bb7f1bd7de8d6ff93d5e0f8cdb92acd8d563131ae001172e037d5bb55d8e5";
    fileMetasList->fileMetasArray[1]->tObject.pAbstracValue = adm_strdup(strSign);
    fileMetasList->fileMetasArray[1]->efiletype = ADM_SIM_SOA_UCL_FILE_TYPE_SIGN;

    D32S retCode = adm_dim_ucl_request_Verify_Sign(adm_soa_getUclInstanceId(),
                   TIME_WAIT, fileID, filePath, fileMetasList, isUnpick, &errorCode);

    if(retCode == 0)
    {
        log_e("dim request_Verify_Sign success");
    }
    else
    {
        log_e("dim request_Verify_Sign failed, retCode=%d", retCode);
    }

    for(D32U i = 0; i < fileMetasList->fileMetaNum; ++i)
    {
        adm_free(fileMetasList->fileMetasArray[i]->tObject.pAbstracValue);
        fileMetasList->fileMetasArray[i];
    }

    adm_free(fileMetasList->fileMetasArray);
    adm_free(fileMetasList);

    log_i("dim request Verify Sign end ****");

    return 0;
}

static void request_dim_ucl_GetSdkVersion()
{
    log_i("request_dim_ucl_GetSdkVersion********ucl test");
    D16U instanceId = 0X2001;
    DCHAR* sdkVersion = NULL;
    D32S errorCode = 1;
    adm_dim_ucl_getSdkVersion(instanceId, TIME_WAIT, &sdkVersion, &errorCode);

    log_i("sdkVersion = %s", sdkVersion);
    log_i("resp errorCode = %d", errorCode);

    adm_free(sdkVersion);

    log_i("request_dim_ucl_GetSdkVersion end");
}

D32S verifySpaceIsEnouth()
{
    log_e("dim开始发起请求 verifySpaceIsEnouth\n");
    D64S needSize = 1;
    D64S realSize = 2;
    D32S errorCode = 3;
    D32S retCode = adm_dim_ucl_verifySpaceIsEnouth(adm_soa_getUclInstanceId(),
                   TIME_WAIT, needSize, &realSize, &errorCode);

    if(retCode == 0)
    {
        log_e("dim收到sysi回复， verifySpaceIsEnouth 成功\n");
    }
    else
    {
        log_e("dim未收到sysi回复， verifySpaceIsEnouth 失败\n");
    }

    return 0;
}

static void request_dim_ucl_set_log_level()
{
    log_i("request_dim_ucl_set_log_level********ucl test");
    D16U instanceId = 0X2001;
    D32S errorCode = 1;
    D32U logLevel = 3;
    log_i("req errorCode = %d", errorCode);
    D32S ret = adm_dim_ucl_set_log_level(instanceId, TIME_WAIT, logLevel, &errorCode);
    log_i("ret = %d", ret);
    log_i("resp errorCode = %d", errorCode);
}

static D32S req_dim_ucl_getCurrentSlot(void)
{
    D32S errorCode = 0;
    ADM_DIM_SOA_MSGID_DATA_ARRAY_T* msgIdListResponse = NULL;
    ADM_DIM_SOA_MSGID_DATA_ARRAY_T msgIdListRequest = {0};
    msgIdListRequest.msgIdArrayLength = 1;
    msgIdListRequest.msgIdArray = adm_malloc(sizeof(ADM_DIM_SOA_MSGID_T*) * msgIdListRequest.msgIdArrayLength);
    msgIdListRequest.msgIdArray[0] = adm_malloc(sizeof(ADM_DIM_SOA_MSGID_T));
    msgIdListRequest.msgIdArray[0]->msgId = VEHICLE_STATUS_SLOT;
    msgIdListRequest.msgIdArray[0]->msgIdData = adm_malloc(sizeof(ADM_DIM_SOA_DATA_T));
    msgIdListRequest.msgIdArray[0]->msgIdData->dataType = VAR_S32;
    msgIdListRequest.msgIdArray[0]->msgIdData->dataLength = 4;

    D32S retVal = adm_dim_ucl_readVehicleStatus(adm_soa_getUclInstanceId(), 5,
                  &msgIdListRequest, &msgIdListResponse, &errorCode);

    log_i("resp errorcode = %d", errorCode);

    for(int index = 0; index < msgIdListRequest.msgIdArrayLength; ++index)
    {
        adm_free(msgIdListRequest.msgIdArray[index]->msgIdData);
        adm_free(msgIdListRequest.msgIdArray[index]);
    }

    adm_free(msgIdListRequest.msgIdArray);

    if(retVal != ADM_ECODE_SUCCESSED)
    {
        log_w("call adm_dim_host_readVehicleStatus fail,retVal = %d", retVal);
    }

    if(msgIdListResponse)
    {
        for(int index = 0; index < msgIdListResponse->msgIdArrayLength; ++index)
        {
            if(VAR_S32 == msgIdListResponse->msgIdArray[index]->msgIdData->dataType)
            {
                log_i("resp data:msgId[%#x],value=%d", msgIdListResponse->msgIdArray[index]->msgId,
                      msgIdListResponse->msgIdArray[index]->msgIdData->v.valueS32);
            }
            else if(VAR_STRING == msgIdListResponse->msgIdArray[index]->msgIdData->dataType)
            {
                log_i("resp data:msgId[%#x],value=%s", msgIdListResponse->msgIdArray[index]->msgId,
                      msgIdListResponse->msgIdArray[index]->msgIdData->v.valueS);
            }

            adm_free(msgIdListResponse->msgIdArray[index]->msgIdData);
            adm_free(msgIdListResponse->msgIdArray[index]);
        }

        adm_free(msgIdListResponse->msgIdArray);
        adm_free(msgIdListResponse);
    }
}

static D32S request_dim_ucl_setVehicleStatus_Test()
{
    ADM_DIM_SOA_MSGID_DATA_ARRAY_T msgIdListRequest = {0};
    msgIdListRequest.msgIdArrayLength = 2;

    msgIdListRequest.msgIdArray =
        (ADM_DIM_SOA_MSGID_T**)adm_malloc(sizeof(ADM_DIM_SOA_MSGID_T*) * msgIdListRequest.msgIdArrayLength);

    //切换分区
    msgIdListRequest.msgIdArray[0] = (ADM_DIM_SOA_MSGID_T*)adm_malloc(sizeof(ADM_DIM_SOA_MSGID_T));
    msgIdListRequest.msgIdArray[0]->msgId = VEHICLE_CTRL_CHANGE_SLOT;
    msgIdListRequest.msgIdArray[0]->msgIdData = NULL;
    //升级后重启
    msgIdListRequest.msgIdArray[1] = (ADM_DIM_SOA_MSGID_T*)adm_malloc(sizeof(ADM_DIM_SOA_MSGID_T));
    msgIdListRequest.msgIdArray[1]->msgId = VEHICLE_CTRL_UPGEADE_REBOOT;
    msgIdListRequest.msgIdArray[1]->msgIdData = NULL;

    D32S errorCode;
    D32S retVal = adm_dim_ucl_setVehicleStatus(adm_soa_getUclInstanceId(), 5, &msgIdListRequest, &errorCode);
    log_i("errorCode = %u", errorCode);

    if(retVal != ADM_ECODE_SUCCESSED)
    {
        log_w("call adm_dim_ucl_setVehicleStatus fail");

        log_i("adm_dim_ucl_setVehicleStatus end");
        return ADM_ECODE_FAILED;
    }

    for(D16U index = 0; index < msgIdListRequest.msgIdArrayLength; ++index)
    {
        adm_free(msgIdListRequest.msgIdArray[index]);
    }

    adm_free(msgIdListRequest.msgIdArray);

    log_i("adm_dim_host_setVehicleStatus_Test end");

    return ADM_ECODE_SUCCESSED;
}


//recv notify
D32S readPartInfonotifyCallback(const ADM_DIM_SOA_ECU_IDEN_INFO_T
                                *pstEcuIdenInfo, const ADM_DIM_SOA_DID_DATA_ARRAY_T* didListResponse)
{
    log_e("dim收到sysi的notify并进入回调函数 readPartInfonotifyCallback");
    return 0;
}
D32S upgradeProgressNotifyCallback(const ADM_DIM_SOA_ECU_IDEN_INFO_T
                                   *pstEcuIdenInfo, const D16U progress, void* priv)
{
    log_e("dim收到sysi的notify并进入回调函数 upgradeProgressNotifyCallback");
    return 0;
}

D32S upgradeResultNotifyCallback(const ADM_DIM_SOA_ECU_IDEN_INFO_T* pstIdenInfo,
                                 const ADM_DIM_SOA_UCL_UPGRADE_RESULT_INFO_T* pstUpgradeResultInfo, void* priv)
{
    log_e("dim收到sysi的notify并进入回调函数 upgradeResultNotifyCallback");
    return 0;
}

D32S decryptProgressNotifyCallback(const DCHAR* fileID,
                                   const ADM_DIM_SOA_UCL_DECRYPT_PROGRESS_INFO_T* pstDecryptProgressInfo,
                                   void* priv)
{
    log_e("dim收到sysi的notify并进入回调函数 decryptProgressNotifyCallback");
    log_i("fileID=%s", fileID);

    if(NULL != pstDecryptProgressInfo)
    {
        log_i("savedSize=%d", pstDecryptProgressInfo->savedSize);
        log_i("totalSize=%d", pstDecryptProgressInfo->totalSize);
        log_i("speed=%d", pstDecryptProgressInfo->speed);
        log_i("remainTime=%d", pstDecryptProgressInfo->remainTime);
    }

    return 0;
}

D32S decryptResultNotifyCallback(const DCHAR* fileID, const DCHAR* storagePath,
                                 const ADM_DIM_SOA_UCL_DECRYPT_RESULT_INFO_T* pstDecryptResultInfo, void* priv)
{
    log_e("dim收到sysi的notify并进入回调函数 decryptResultNotifyCallback");
    log_i("fileID=%s", fileID);
    log_i("storagePath=%s", storagePath);

    if(NULL != pstDecryptResultInfo)
    {
        log_i("result=%d", pstDecryptResultInfo->result);
        log_i("expectVal=%s", pstDecryptResultInfo->expectVal);
        log_i("realVal=%s", pstDecryptResultInfo->realVal);
    }

    return 0;
}

D32S verifyPackageHashProgressNotifyCallback(const DCHAR* fileID,
        const D16U progress, void* priv)
{
    log_e("dim收到sysi的notify并进入回调函数 verifyPackageHashProgressNotifyCallback");
    return 0;
}

D32S verifyPackageHashResultNotifyCallback(const DCHAR* fileID,
        const ADM_DIM_SOA_UCL_VERIFY_PACKAGE_HASH_RESULT_INFO_T
        *pstVerifyPackageHashResultInfo, void* priv)
{
    log_e("dim收到sysi的notify并进入回调函数 verifyPackageHashResultNotifyCallback");
    return 0;
}

D32S verify_Sign_Progress_Notify_simul(const DCHAR* fileID, const D16U progress)
{
    log_i("fileID = %s", fileID);
    log_i("progress = %d", progress);

    return 0;
}
D32S verify_Sign_Result_Notify_simul(const DCHAR* fileID,
                                     const ADM_SYSI4UCL_VERIFY_SIGN_RESULT_INFO_T* pVerifySignResultInfo)
{
    log_i("fileID = %s", fileID);

    if(pVerifySignResultInfo)
    {
        log_i("result = %u", pVerifySignResultInfo->result);
        log_i("signature = %s", pVerifySignResultInfo->signature);
        log_i("signatureLen = %u", pVerifySignResultInfo->signatureLen);
        log_i("filePath = %s", pVerifySignResultInfo->filePath);
    }

    return 0;
}


D32S adm_ucl_simulatorInit()
{
    log_i("adm_ucl_simulatorInit begin");
    ADM_DIM_SOA_UCL_INIT_INFO_T* initInfo = (ADM_DIM_SOA_UCL_INIT_INFO_T*)
                                            adm_malloc(sizeof(ADM_DIM_SOA_UCL_INIT_INFO_T));
    adm_memset(initInfo, 0x0, sizeof(ADM_DIM_SOA_UCL_INIT_INFO_T),
               sizeof(ADM_DIM_SOA_UCL_INIT_INFO_T));
    initInfo->readPartInfoCallback = readPartInfonotifyCallback;
    initInfo->upgradeProgressNotifyCallback = upgradeProgressNotifyCallback;
    initInfo->upgradeResultNotifyCallback = upgradeResultNotifyCallback;
    initInfo->decryptProgressNotifyCallback = decryptProgressNotifyCallback;
    initInfo->decryptResultNotifyCallback = decryptResultNotifyCallback;
    initInfo->verifyPackageHashProgressNotifyCallback =
        verifyPackageHashProgressNotifyCallback;
    initInfo->verifyPackageHashResultNotifyCallback =
        verifyPackageHashResultNotifyCallback;
    initInfo->verifySignProgressNotifyCallback = verify_Sign_Progress_Notify_simul;
    initInfo->verifySignResultNotifyCallback   = verify_Sign_Result_Notify_simul;

    void* priv = NULL;
    adm_dim_ucl_init(initInfo, priv); //订阅注册
    log_i("adm_ucl_simulatorInit end");

    return 0;
}

D32S adm_ucl_simulatorExec()
{
    log_i("adm_ucl_simulatorExec begin");

    requestDecryptPackage();

#if 0
    req_dim_ucl_getCurrentSlot();
    request_dim_ucl_setVehicleStatus_Test();
    request_Verify_Sign(); //ok
    requestDecryptPackage(); //ok
    getDecryptPackageProgress();
    getDecryptPackageResult();
    requestDoUpgrade(); //ok
    queryUpgradeProgress();
    queryUpgradeResult();
    requestVerifyHash();  //ok
    request_dim_ucl_set_log_level();
    request_dim_ucl_GetSdkVersion();
    readPartInformation();
#endif

    log_i("adm_ucl_simulatorExec end");

    return 0;
}

D32S adm_ucl_simulatorDeinit()
{
    log_i("adm_ucl_simulatorDeinit begin");
    adm_dim_ucl_deinit();
    log_i("adm_ucl_simulatorDeinit end");

    return 0;
}


