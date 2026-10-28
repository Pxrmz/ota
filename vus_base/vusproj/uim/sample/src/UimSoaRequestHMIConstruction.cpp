#include "UimSoaRequestConstruction.h"
#include "adm_uim_request.h"
#include "adm_uim_log.h"
#include "adm_uim_general.h"

#include <stdlib.h>
#include <adm_memory.h>
#include "adm_string.h"

#ifdef WIN32
    #include <windows.h>
#else
    #include <unistd.h>
#endif

static D16U TIME_WAIT = 5;
static void wait_sleep(D32U secs)
{
#ifdef WIN32
    (void)Sleep((DWORD)secs * 1000);
#else
    (void)sleep(secs);
#endif
}

static void request_uim_hmi_set_log_level(void)
{
    log_i("request_uim_hmi_set_log_level********uim request test");
    D16U instanceId = 0X3;    //
    D32S errorCode = 1;
    D32U logLevel = 3;
    D32S ret = adm_uim_hmi_set_log_level(instanceId, TIME_WAIT, logLevel, &errorCode);
    log_i("ret = %d", ret);
    log_i("resp errorCode = %d", errorCode);
}

static void request_uim_hmi_get_sdkVersion(void)
{
    log_i("request_uim_hmi_get_sdkVersion********uim request test");
    D16U instanceId = 0X103;    //
    D32S errorCode = 1;
    DCHAR* sdkVersion = NULL;
	DCHAR* apkVersion = NULL;
    adm_uim_hmi_getSdkVersion(instanceId, TIME_WAIT, &sdkVersion,&apkVersion,&errorCode);

    log_i("sdkVersion = %s", sdkVersion);
	log_i("aplVersion = %s", apkVersion);
    log_i("resp errorCode = %d", errorCode);
}


void simulateRequsetExe(void)
{
	D32S respCode=-1;
	D32S buildRet = -1;
	D16U listSize = 0;
	DCHAR *pReqStr = NULL;
	char url[21];
	char hash[21];
	char version[21];
	

//	ADM_UIM_SOA_CHECK_HMI_POLICY_INFO_T pReq = {NULL,NULL,ADM_UIM_UPGRADE_MODE_NORMAL_DOWN_NORMAL_INS};
	ADM_UIM_HMI_FILE_T 	hmiFile ;
	ADM_UIM_HMI_INFO 	hmiInfo ;
	ADM_UIM_UPGRADE_MODE_E	upgradeMode = ADM_UIM_UPGRADE_MODE_SILENT_DPWN_SILENT_INS;
	ADM_UIM_SOA_CONDITION_DATA_ARRAY_T *condsArrarList;
 	ADM_UIM_SOA_DOWNLOAD_RESULT_INFO_T *pstPolicyDownloadResult;
	ADM_UIM_SOA_ECU_IDEN_INFO_T *pstEcuIdenInfo;
	ADM_UIM_SOA_DOWNLOAD_RESULT_INFO_T *pstEcuDownloadResult;
	ADM_UIM_SOA_ECU_DOWNLOAD_RESULT_ARRAY_INFO_T *pstEcuDownloadResultArrayInfo;
	int verifyResult = -1;
	ADM_UIM_SOA_ECU_INSTALL_RESULT_ARRAY_INFO_T *pstEcuInstallResultArrayInfo;
	ADM_UIM_SOA_INSTALL_RESULT_INFO_T *pstEcuInstallResultInfo;
	ADM_UIM_SOA_INSTALL_RESULT_INFO_T *pstPolicyInstallResult;
/*
	do
	{
		upgradeMode = ADM_UIM_UPGRADE_MODE_NORMAL_DOWN_NORMAL_INS;	//1

		strcpy(url,"www.test.com");
		strcpy(hash,"1234567890");
		hmiFile.policyFileUrl=url;
		hmiFile.policyFileHash=hash;
		hmiFile.policyFileSize = 1;
		
		strcpy(version,"V1.0.1");
		hmiInfo.vehicleVersion = version;
		hmiInfo.totalSize = 1;
		hmiInfo.ecuInfoList = (ADM_UIM_ECU_INFO_ARRAY_T *)adm_malloc(sizeof(ADM_UIM_ECU_INFO_ARRAY_T));
		hmiInfo.ecuInfoList->hmiEcuInfoArrayLength=0;
	}while(0);
	pReq.hmiFile = &hmiFile;
	pReq.hmiInfo = &hmiInfo;
	pReq.upgradeMode = upgradeMode;
	
    	log_i("uim request begin,pReqStr[%s]",pReq);
    	wait_sleep((D32U)5);
	log_w("uim request wait ********uim request test");
*/

    	//request_uim_hmi_set_log_level();
    //	log_i("uim request begin,pReqStr[%s]",pReq);
    	(void)sleep(20);
    //	request_uim_hmi_get_sdkVersion();
        adm_uim_vsmStateNotify(1);
	    log_w("uim request wait ********uim request test");
/*
    	adm_uim_hmi_checkResult_RR(ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_TSP,"12345","1",&pReq,0,&respCode);
	adm_free(hmiInfo.ecuInfoList);
    	log_w("uim check request end respCode[%d]",respCode);

    	wait_sleep((D32U)5);
    	adm_uim_hmi_conds_downloadResult_RR(ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_TSP,"12345","1",condsArrarList,verifyResult,&respCode);
    	log_i("uim condsDownload request end respCode[%d]",respCode);

    	wait_sleep((D32U)3);
    	adm_uim_hmi_policy_downloadResult_RR(ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_TSP,"12345","1",pstPolicyDownloadResult,&respCode);
    	log_i("uim policyDownload request end respCode[%d]",respCode);

    	wait_sleep((D32U)3);
    	adm_uim_hmi_ecu_downloadResult_RR(ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_TSP,"12345","1",pstEcuIdenInfo,pstEcuDownloadResult,&respCode);
    	log_i("uim ecuDownload request end respCode[%d]",respCode);

    	wait_sleep((D32U)3);
    	adm_uim_hmi_allecu_downloadResult_RR(ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_TSP,"12345","1",pstEcuDownloadResultArrayInfo,pstPolicyDownloadResult,&respCode);
    	log_i("uim allecuDownload request end respCode[%d]",respCode);

    	wait_sleep((D32U)3);
    	adm_uim_hmi_conds_installResult_RR(ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_TSP,"12345","1",condsArrarList,verifyResult,&respCode);
    	log_i("uim condsInstall request end respCode[%d]",respCode);

    	wait_sleep((D32U)3);
    	adm_uim_hmi_policy_installResult_RR(ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_TSP,"12345","1",pstPolicyInstallResult,&respCode);
    	log_i("uim policyInstall request end respCode[%d]",respCode);

    	wait_sleep((D32U)3);
    	adm_uim_hmi_ecu_installResult_RR(ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_TSP,"12345","1",pstEcuIdenInfo,pstEcuInstallResultInfo,&respCode);
    	log_i("uim ecuInstall request end respCode[%d]",respCode);

    	wait_sleep((D32U)3);
    	adm_uim_hmi_allecu_installResult_RR(ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_TSP,"12345","1",pstEcuInstallResultArrayInfo,pstPolicyInstallResult,&respCode);
    	log_i("uim allecuInstall request end respCode[%d]",respCode);
        */
	
	return;
}


