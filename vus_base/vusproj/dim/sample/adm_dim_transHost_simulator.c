#include "adm_dim_transHost_simulator.h"
#include "adm_dim_transHost.h"
#include "adm_soa.h"
#include "adm_soa_cfg.h"
#include "adm_dim_log.h"

#define TIME_WAIT 5

static void request_dim_trans_GetSdkVersion()
{
    log_i("request_dim_trans_GetSdkVersion********trans test");
    D16U instanceId = 0X2001;
    DCHAR *sdkVersion = NULL;
    D32S errorCode = 1;
    adm_dim_trans_getSdkVersion(instanceId, TIME_WAIT, &sdkVersion, &errorCode);

    log_i("sdkVersion = %s", sdkVersion);
    log_i("resp errorCode = %d", errorCode);

    adm_free(sdkVersion);
    
    log_i("request_dim_trans_GetSdkVersion end");
}

static void requestTransferDownloadFromHost_test()
{
    log_i("requestTransferDownloadFromHost_test begin");
    DCHAR *fileID = "fileID80";
    DCHAR *url = "www.baidu.com";
    DCHAR *filename = "111111";
    DCHAR *hash = "xjakdalsd";
    
    ADM_DIM_SOA_TRANS_FILE_TYPE_E eFileType;
    eFileType = ADM_DIM_SOA_TRANS_FILE_TYPE_PKG;
    D64S size = 100;
    D32S errcode = 10;
    adm_dim_trans_requestTransferDownloadFromHost(adm_soa_getUclInstanceId(),
            TIME_WAIT, fileID, eFileType, url, filename, size, hash, &errcode);
    log_i("errcode:%d", errcode);

    log_i("requestTransferDownloadFromHost_test end");
}
static void requestTransferDownloadFromHost_Progress_test()
{
    log_i("begin");
    
    DCHAR *fileID = "fileID80";
    D32S errorCode = 10;

    ADM_DIM_SOA_TRANS_PROGRESS_INFO_T *ppstTransProgress = NULL;
    
    adm_dim_trans_queryProgressDownloadFromHost(adm_soa_getUclInstanceId(), 
            TIME_WAIT, fileID, &ppstTransProgress, &errorCode);
    if(ppstTransProgress)
    {
        log_i("savedSize = %lld", ppstTransProgress->savedSize);
        log_i("totalSize = %lld", ppstTransProgress->totalSize);
        log_i("speed = %d", ppstTransProgress->speed);
        log_i("remainTime = %d", ppstTransProgress->remainTime);
        adm_free(ppstTransProgress);
    }
    log_i("errorCode = %d", errorCode);
    
    log_i("end");
}
static void requestTransferDownloadFromHost_Result_test()
{
    log_i("begin");
    
    DCHAR *fileID = "fileID80";
    D32S errorCode = 10;
    DCHAR *storagePath = NULL;
    ADM_DIM_SOA_TRANS_RESULT_INFO_T *ppstTransResult = NULL;

    adm_dim_trans_queryResultDownloadFromHost(adm_soa_getUclInstanceId(), 
        TIME_WAIT, fileID, &storagePath, &ppstTransResult, &errorCode);

    log_i("storagePath = %s", storagePath);
    if(ppstTransResult)
    {
        log_i("result = %d", ppstTransResult->result);
        log_i("expectVal = %s", ppstTransResult->expectVal);
        log_i("expectVal = %s", ppstTransResult->realVal);
        adm_free(ppstTransResult);
    }
    log_i("errorCode = %d", errorCode);

    log_i("end");
}
static void requestUploadToHost_test()
{
    log_i("begin");
    DCHAR *fileID = "fileID80";
    ADM_DIM_SOA_TRANS_FILE_TYPE_E eFileType;
    eFileType = ADM_DIM_SOA_TRANS_FILE_TYPE_LOG;
    DCHAR* url = "/tmp1/tmp2/tmp3";

    DCHAR *filename = NULL;
    DCHAR* hash =NULL;
    D64S size = 0;
    D32S errorCode = 10;
    adm_dim_trans_requestTransferUploadToHost(adm_soa_getUclInstanceId(), 
        TIME_WAIT, fileID, eFileType, url, &filename, &hash, &size, &errorCode);
    log_i("filename = %s", filename);
    log_i("hash = %s", hash);
    log_i("size = %lld", size);
    log_i("errorCode = %d", errorCode);
    adm_free(filename);
    adm_free(hash);
    
    log_i("end");
}
static void requestUploadToHost_Progress_test()
{
    log_i("begin");
    DCHAR *fileID = "fileID80";
    ADM_DIM_SOA_TRANS_PROGRESS_INFO_T *ppstTransProgress = NULL;
    D32S errorCode = 10;
    adm_dim_trans_queryProgressUploadToHost(adm_soa_getUclInstanceId(), 
        TIME_WAIT, fileID, &ppstTransProgress, &errorCode);

    if(ppstTransProgress)
    {
        log_i("savedSize = %lld", ppstTransProgress->savedSize);
        log_i("totalSize = %lld", ppstTransProgress->totalSize);
        log_i("speed = %d", ppstTransProgress->speed);
        log_i("remainTime = %d", ppstTransProgress->remainTime);
        adm_free(ppstTransProgress);
    }
    log_i("errorCode = %d", errorCode);

    log_i("end");
}
static void requestUploadToHost_Result_test()
{
    log_i("begins");

    DCHAR *fileID = "fileID80";
    
    DCHAR *filename = NULL;
    ADM_DIM_SOA_TRANS_RESULT_INFO_T *ppstTransResult = NULL;
    D32S errorCode = 10;
    adm_dim_trans_queryResultUploadToHost(adm_soa_getUclInstanceId(), 
        TIME_WAIT, fileID, &filename, &ppstTransResult, &errorCode);

    log_i("filename = %s", filename);
    if(ppstTransResult)
    {
        log_i("result = %d", ppstTransResult->result);
        log_i("expectVal = %s", ppstTransResult->expectVal);
        log_i("realVal = %s", ppstTransResult->realVal);
        adm_free(ppstTransResult->expectVal);
        adm_free(ppstTransResult->realVal);
        adm_free(ppstTransResult);
    }
    log_i("errorCode = %d", errorCode);
    
    log_i("end");
}

static void request_dim_trans_set_log_level()
{
    log_i("request_dim_trans_set_log_level********trans test");
    D16U instanceId = 0X2001;
    D32S errorCode = 1;
    D32U logLevel = 3;
    log_i("req errorCode = %d", errorCode);
    D32S ret = adm_dim_trans_set_log_level(instanceId, TIME_WAIT, logLevel, &errorCode);
    log_i("ret = %d", ret);
    log_i("resp errorCode = %d", errorCode);
}

//recv notify
static D32S transDownloadFromHostProgress_notify(const DCHAR *fileID,
        const ADM_DIM_SOA_TRANS_PROGRESS_INFO_T *pstTransProgress, void *priv)
{
    log_i("transDownloadFromHostProgress_notify begin");
    log_i("notify fileID = %s", fileID);
    if(pstTransProgress)
    {
        log_i("notify savedSize = %lld", pstTransProgress->savedSize);
        log_i("notify totalSize = %lld", pstTransProgress->totalSize);
        log_i("notify speed = %d", pstTransProgress->speed);
        log_i("notify remainTime = %d", pstTransProgress->remainTime);
    }
    
    log_i("adm_readDiagInfoCallback_notify end");
    return 0;
}

static D32S transDownloadFromHostResult_notify(const DCHAR *fileID,
        const DCHAR *storePath, const ADM_DIM_SOA_TRANS_RESULT_INFO_T *pstTransResult,
        void *priv)
{
    log_i("transDownloadFromHostResult_notify begin");
    
    log_i("notify fileID = %s", fileID);
    log_i("notify storePath = %s", storePath);
    if(pstTransResult)
    {
        log_i("notify result = %d", pstTransResult->result);
        log_i("notify expectVal = %s", pstTransResult->expectVal);
        log_i("notify realVal = %s", pstTransResult->realVal);
    }
    
    log_i("transDownloadFromHostResult_notify end");
    return 0;
}

static D32S transUploadToHostProgress_notify(const DCHAR *fileID,
        const ADM_DIM_SOA_TRANS_PROGRESS_INFO_T *pstTransProgress, void *priv)
{
    log_i("transUploadToHostProgress_notify begin");

    log_i("notify fileID = %s", fileID);
    if(pstTransProgress)
    {
        log_i("notify savedSize = %lld", pstTransProgress->savedSize);
        log_i("notify totalSize = %lld", pstTransProgress->totalSize);
        log_i("notify speed = %d", pstTransProgress->speed);
        log_i("notify remainTime = %d", pstTransProgress->remainTime);
    }
    log_i("transUploadToHostProgress_notify end");

    return 0;
}

static D32S transUploadToHostResult_notify(const DCHAR *fileID,
        const DCHAR* filename, const ADM_DIM_SOA_TRANS_RESULT_INFO_T *pstTransResult, void *priv)
{
    log_i("transUploadToHostResult_notify begin");
    
    log_i("notify fileID = %s", fileID);
    log_i("notify filename = %s", filename);
    if(pstTransResult)
    {
        log_i("notify result = %d", pstTransResult->result);
        log_i("notify expectVal = %s", pstTransResult->expectVal);
        log_i("notify realVal = %s", pstTransResult->realVal);
    }
    
    log_i("transUploadToHostResult_notify end");
    return 0;
}

D32S adm_transHost_simulatorInit()
{
    log_i("adm_transHost_simulatorInit begin");

    ADM_DIM_SOA_TRANS_HOST_INIT_INFO_T *initInfo  = NULL;
    initInfo = (ADM_DIM_SOA_TRANS_HOST_INIT_INFO_T *)adm_malloc(sizeof(
                   ADM_DIM_SOA_TRANS_HOST_INIT_INFO_T));
    adm_memzero(initInfo, sizeof(ADM_DIM_SOA_TRANS_HOST_INIT_INFO_T));

    initInfo->transDownloadFromHostProgressNotifyCallback = transDownloadFromHostProgress_notify;
    initInfo->transDownloadFromHostResultNotifyCallback = transDownloadFromHostResult_notify;
    initInfo->transUploadToHostProgressNotifyCallback = transUploadToHostProgress_notify;
    initInfo->transUploadToHostResultNotifyCallback = transUploadToHostResult_notify;

    void *priv = NULL;

    adm_dim_transferOnHost_init(initInfo, priv); //订阅注册

    log_i("adm_transHost_simulatorInit end");

    return 0;
}

D32S adm_transHost_simulatorExec()
{
    log_i("adm_transHost_simulatorExec begin");

    requestTransferDownloadFromHost_test();
    requestTransferDownloadFromHost_Progress_test();
    requestTransferDownloadFromHost_Result_test();
    requestUploadToHost_test();
    requestUploadToHost_Progress_test();
    requestUploadToHost_Result_test();
    request_dim_trans_set_log_level();
    request_dim_trans_GetSdkVersion();
    
    log_i("adm_transHost_simulatorExec end");

    return 0;
}

D32S adm_transHost_simulatorDeinit(void)
{
    log_i("adm_transHost_simulatorDeinit begin");
    adm_dim_transferOnHost_deinit();
    log_i("adm_transHost_simulatorDeinit end");

    return 0;
}



