#include "adm_dim_log.h"
#include "adm_dim_dpo_simulator.h"
#include "adm_dim_dpo.h"
#include "adm_soa.h"
#include "adm_soa_cfg.h"


static D16U TIME_WAIT = 5;

static void queryDownloadPkg(void)
{
    log_i("queryDownloadPkg begin");

    D32S errorCode = 1;

    DCHAR* fileId = "fileid12345";
    DCHAR* url    = "/tmp1/tmp2/tmp3/test";
    D64U fileSize = 1024520;
    DCHAR* hash   = "hash1024520";

    //tmp UclInstanceId
    D32S ret = adm_dim_dpo_requestRemoteDownloadPackage(adm_soa_getUclInstanceId(), TIME_WAIT, fileId, url, 
                fileSize, hash,0, &errorCode);
    log_i("ret = %d", ret);
    log_i("errorCode = %d", errorCode);

    log_i("queryDownloadPkg end");
}
static void queryDownloadPkgProgress(void)
{
    const DCHAR* fileId = "fileid12345";
    D32S errorCode = 1;
    ADM_DIM_SOA_DPO_DOWNLOAD_PROGRESS_INFO_T* pDownloadPkgProgressInfo = NULL;
    
    D32S retCode = adm_dim_dpo_getRemoteDownloadProgress(adm_soa_getUclInstanceId(),
                   TIME_WAIT, fileId, &pDownloadPkgProgressInfo, &errorCode);
    if(NULL != pDownloadPkgProgressInfo)
    {
        log_i("savedSize = %lld", pDownloadPkgProgressInfo->savedSize);
        log_i("totalSize = %lld", pDownloadPkgProgressInfo->totalSize);
        log_i("speed = %d", pDownloadPkgProgressInfo->speed);
        log_i("remainTime = %d", pDownloadPkgProgressInfo->remainTime);

        adm_free(pDownloadPkgProgressInfo);
    }
    log_i("errorCode = %d", errorCode);
}
static void queryDownloadPkgResult(void)
{
    const DCHAR* fileId = "fileid12345";
    D32S errorCode = 1;

    DCHAR* storagePath = NULL;
    ADM_DIM_SOA_DPO_DOWNLOAD_RESULT_INFO_T* pDownloadPkgResultInfo = NULL;
    
    D32S retCode = adm_dim_dpo_getRemoteDownloadResult(adm_soa_getUclInstanceId(), TIME_WAIT, 
                fileId, &storagePath, &pDownloadPkgResultInfo, &errorCode);

    log_i("errorCode = %d", errorCode);
    log_i("storagePath = %s", storagePath);
    if(NULL != pDownloadPkgResultInfo)
    {
        log_i("result = %d", pDownloadPkgResultInfo->result);
        log_i("expectVal = %s", pDownloadPkgResultInfo->expectVal);
        log_i("realVal = %s", pDownloadPkgResultInfo->realVal);
        adm_free(pDownloadPkgResultInfo->expectVal);
        adm_free(pDownloadPkgResultInfo->realVal);
        adm_free(pDownloadPkgResultInfo);
    }
    adm_free(storagePath);
}

static void requestCleanDownloadPkgDir(void)
{
    D16U instanceId = 0X2001;
    D32S errorCode = 1;
    D32S ret = adm_dim_dpo_cleanDownloadDir(instanceId, TIME_WAIT, &errorCode);
    log_i("ret = %d", ret);
    log_i("errorCode = %d", errorCode);
}

static void request_dim_dpo_set_log_level(void)
{
    log_i("request_dim_dpo_set_log_level********dpo test");
    D16U instanceId = 0X2001;
    D32S errorCode = 1;
    D32U logLevel = 3;
    log_i("req errorCode = %d", errorCode);
    D32S ret = adm_dim_dpo_set_log_level(instanceId, TIME_WAIT, logLevel, &errorCode);
    log_i("ret = %d", ret);
    log_i("resp errorCode = %d", errorCode);
}

static void request_dim_dpo_GetSdkVersion(void)
{
    log_i("request_dim_dpo getSdkVersion ********dpo test");
    D16U instanceId = 0X2001;
    DCHAR *sdkVersion = NULL;
    D32S errorCode = 1;
    adm_dim_dpo_getSdkVersion(instanceId, TIME_WAIT, &sdkVersion, &errorCode);

    log_i("sdkVersion = %s", sdkVersion);
    log_i("errorCode = %d", errorCode);

    adm_free(sdkVersion);
    
    log_i("request_dim_dpo getSdkVersion end");
}


//notify simul
static D32S download_pkg_Progress_Notify_simul(const DCHAR *fileID,
        const ADM_DIM_SOA_DPO_DOWNLOAD_PROGRESS_INFO_T *pUclDownLoadProgress, void* priv)
{
    log_i("fileID = %s", fileID);
    
    if(NULL != pUclDownLoadProgress)
    {
        log_i("savedSize = %lld", pUclDownLoadProgress->savedSize);
        log_i("totalSize = %lld", pUclDownLoadProgress->totalSize);
        log_i("speed = %d", pUclDownLoadProgress->speed);
        log_i("remainTime = %d", pUclDownLoadProgress->remainTime);
    }

    return 0;
}

static D32S download_pkg_Result_Notify_simul(const DCHAR *fileID,
        const DCHAR *storagePath, const ADM_DIM_SOA_DPO_DOWNLOAD_RESULT_INFO_T *pUclDownLoadResults, void* priv)
{
    log_i("fileID = %s", fileID);
    log_i("storagePath = %s", storagePath);
    if(NULL != pUclDownLoadResults)
    {
        log_i("result = %d", pUclDownLoadResults->result);
        log_i("expectVal = %s", pUclDownLoadResults->expectVal);
        log_i("realVal = %s", pUclDownLoadResults->realVal);
    }

    return 0;
}


D32S adm_dpo_simulatorInit(void)
{
	log_i("adm_dpo_simulatorInit begin");

	ADM_DIM_SOA_DPO_INIT_INFO_T* initInfo = NULL;

    initInfo = (ADM_DIM_SOA_DPO_INIT_INFO_T *)adm_malloc(sizeof(ADM_DIM_SOA_DPO_INIT_INFO_T));
    adm_memset(initInfo, 0x0, sizeof(ADM_DIM_SOA_DPO_INIT_INFO_T),
               sizeof(ADM_DIM_SOA_DPO_INIT_INFO_T));
    initInfo->downProgressNotifyCallback = download_pkg_Progress_Notify_simul;
    initInfo->downloadResultNotifyCallback = download_pkg_Result_Notify_simul;

	void* priv = NULL;
	adm_dim_dpo_init(initInfo, priv);

	log_i("adm_dpo_simulatorInit end");
	return ADM_ECODE_SUCCESSED;
}

D32S adm_dpo_simulatorExec(void)
{
	log_i("adm_dpo_simulatorExec begin");

    queryDownloadPkg();
    queryDownloadPkgProgress();
    queryDownloadPkgResult();
    requestCleanDownloadPkgDir();
    request_dim_dpo_set_log_level();
    request_dim_dpo_GetSdkVersion();
    
	log_i("adm_dpo_simulatorExec end");
	return ADM_ECODE_SUCCESSED;
}

D32S adm_dpo_simulatorDeinit(void)
{
	log_i("adm_dpo_simulatorDeinit begin");

	//adm_dim_dpo_deinit(SYSI4UCL2TBOX_INSTANCE_ID);
	adm_dim_dpo_deinit();

	log_i("adm_dpo_simulatorDeinit end");
	return ADM_ECODE_SUCCESSED;
}




