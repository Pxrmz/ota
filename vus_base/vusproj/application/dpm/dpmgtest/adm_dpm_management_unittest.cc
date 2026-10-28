/**
 * @file    adm_dpm_management_unittest.cc
 * @brief   Download-Policy Management 
 * @details 模块单元测试用例
 * @author  wanglin
 * @version v0.1.0
 * @date    2022-11-30
 * @copyright www.abupdate.com
 */

#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <fstream>
#include <iostream>

#include "adm_file.h"
#include "adm_dpm_log.h"
#include "adm_ecs_rsConfig.h"
#include "adm_cvs_versionPolicy.h"
#include "adm_dpm_report.h"
#include "adm_dpm_reportCloud.h"
#include "adm_dpm_download.h"
#include "adm_dpm_uim.h"
#include "adm_com_lib.h"
#include "adm_irm_report.h"
#include "adm_cdm_verify_interface.h"
#include "adm_ecm_management.h"
#include "adm_pub_func.h"
#include "adm_dim_host.h"
#include "adm_dim_ucl.h"
#include "adm_cfg_api.h"
#include "adm_dpm_management_unittest.h"
#include "adm_dpm_downloadManagement.h"
#include "json/json.h"
#include "adm_string.h"
#include "adm_memory.h"
#include "adm_pis_package.h"
#include "adm_integration_test.h"
#include "adm_irm_logFile.h"

using namespace std;

ADM_ECS_RSCONFIGDATA_T gstEcsRsConfigData = {0};
ADM_IRM_LOG_FILE_REPORT_INIT_T g_stLogFileReportInfo = {0};

#define CORESERVICE_RUNDIR  "../dpm_data/runDir/"
#define DPM_DOWNPARTPACKAGE 1

DCHAR *gpcCheckPolicyJsonPath = NULL;
DCHAR *gpcConfigDataJsonPath = NULL;
DCHAR *gpcOtherInfoConfJsonPath = "../dpm_data/getDpmOtherInfoConfig.json";   //!< 杂项配置
DBOOL gbfirstdowntime = TRUE;
DBOOL gbStopMonitor = FALSE;     //停止下载过程监控
DBOOL gbNetchgPause = FALSE;     //!< 由于网络切换导致的暂停
DBOOL gbCondAbnormalPause = FALSE; //!< 由于下载条件异常导致的暂停

/*!
 *******************************************************************************
 * 读取json数据的文件并转为json root.
 *
 * \note 
 *
 * \param[in]	path  文件路径
 * \param[out]  root  文件json根节点
 * \retval 	    D32S
 *******************************************************************************
 */
static D32S module_readFile2Json(const DCHAR *path, Json::Value *root)
{
    ifstream ifs;
    JSONCPP_STRING errs;
    Json::CharReaderBuilder builder;

    ifs.open(path, ifstream::in);
    if (!ifs.is_open())
    {
        cout << "failed to open filePath[" << path << "], err[" << strerror(errno) << "]" << endl;
        return ADM_ECODE_FAILED;
    }

    if (!parseFromStream(builder, ifs, root, &errs)) 
    {
        cout << "parseFromStream failed " << errs << endl;
        return ADM_ECODE_FAILED;
    }
    ifs.close();

    return ADM_ECODE_SUCCESSED;
}


#ifdef __cplusplus
extern "C" {
#endif


/*!
 *******************************************************************************
 * 构造检测结果中下载相关策略信息.
 *
 * \note 
 *
 * \param[inout] pstVersionPolicy  检测结果策略信息
 * \retval 	     D32S
 *******************************************************************************
 */
D32S __wrap_adm_cvs_getVersionPolicy(ADM_CVS_VERSION_POLICY_T *pstVersionPolicy)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    Json::Value policyRoot;

    EXPECT_NE(nullptr, pstVersionPolicy);

    iRet = module_readFile2Json(gpcCheckPolicyJsonPath, &policyRoot);
    if (ADM_IS_ERR(iRet))
    {
        cout << "failed to read file[" << gpcCheckPolicyJsonPath << "] to json, err[" << iRet << "]"<< endl;
        return iRet;
    }

    pstVersionPolicy->taskId     = policyRoot["taskId"].asInt64();
    pstVersionPolicy->strategyId = policyRoot["strategyId"].asInt64();
    pstVersionPolicy->sessionId  = adm_strdup(policyRoot["sessionId"].asString().c_str());
    //cout << "[----------] ===> downPolicy[" << policyRoot["downPolicy"].toStyledString() << "]" << endl;
    pstVersionPolicy->downloadMember.downloadPolicyObjectStr = adm_strdup(policyRoot["downPolicy"].toStyledString().c_str());
    cout << "[----------] ===> get downpolicy success " << endl;
    return ADM_ECODE_SUCCESSED;
}

/*!
 *******************************************************************************
 * 获取主控网络类型.
 *
 * \note 
 *
 * \param[inout] pinetworkType  主控网络类型
 * \retval 	     D32S
 *******************************************************************************
 */
D32S __wrap_adm_dim_host_getNetworkType(const D16U instanceId, const D16U timeout, D32S *pinetworkType, D32S *errorCode)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    Json::Value OtherConfRoot;

    iRet = module_readFile2Json(gpcOtherInfoConfJsonPath, &OtherConfRoot);
    if (ADM_IS_ERR(iRet))
    {
        cout << "failed to read file[" << gpcOtherInfoConfJsonPath << "] to json, err[" << iRet << "]"<< endl;
        return iRet;
    }

    *pinetworkType = OtherConfRoot["adm_dim_host_getNetworkType"]["networkType"].asInt();
    *errorCode = OtherConfRoot["adm_dim_host_getNetworkType"]["result"].asInt();

    return ADM_ECODE_SUCCESSED;
}


D32S __wrap_adm_dim_host_verifySpaceIsEnough(const D16U instanceId, const D16U timeout, const D64S needSize, D64S *realSize, D32S *errorCode)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    Json::Value OtherConfRoot;

    iRet = module_readFile2Json(gpcOtherInfoConfJsonPath, &OtherConfRoot);
    if (ADM_IS_ERR(iRet))
    {
        cout << "failed to read file[" << gpcOtherInfoConfJsonPath << "] to json, err[" << iRet << "]"<< endl;
        return iRet;
    }

    *errorCode = OtherConfRoot["adm_dim_host_verifySpaceIsEnough"].asInt();

    return ADM_ECODE_SUCCESSED;
}

D32S __wrap_adm_dim_ucl_verifySpaceIsEnouth(const D16U instanceId, const D16U timeout, const D64S needSize, D64S *realSize, D32S *errorCode)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    Json::Value OtherConfRoot;

    iRet = module_readFile2Json(gpcOtherInfoConfJsonPath, &OtherConfRoot);
    if (ADM_IS_ERR(iRet))
    {
        cout << "failed to read file[" << gpcOtherInfoConfJsonPath << "] to json, err[" << iRet << "]"<< endl;
        return iRet;
    }

    *errorCode = OtherConfRoot["adm_dim_ucl_verifySpaceIsEnouth"].asInt();

    return ADM_ECODE_SUCCESSED;
}

D32S __wrap_adm_cdm_verify_Compare(const ADM_COND_POLICY_T* pCondReq,ADM_UIM_SOA_CONDITION_DATA_ARRAY_T** pStCondResults,DBOOL* pbIsConform)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    Json::Value OtherConfRoot;

    iRet = module_readFile2Json(gpcOtherInfoConfJsonPath, &OtherConfRoot);
    if (ADM_IS_ERR(iRet))
    {
        cout << "failed to read file[" << gpcOtherInfoConfJsonPath << "] to json, err[" << iRet << "]"<< endl;
        return iRet;
    }

    iRet = OtherConfRoot["adm_cdm_verify_Compare"].asInt();

    *pStCondResults = (ADM_UIM_SOA_CONDITION_DATA_ARRAY_T*)adm_malloc((DSIZE)sizeof(ADM_UIM_SOA_CONDITION_DATA_ARRAY_T));
    if (NULL == *pStCondResults)
    {
        return iRet;
    }

    return iRet;
}


static void *dpmMonitorThreadProc_test(void *param)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    D32S iCycleTime = 0;
    Json::Value OtherConfRoot;
    ADM_UIM_SOA_CONDITION_DATA_ARRAY_T *pStCondResults = NULL;
    DBOOL bIsConform = FALSE;
    struct timespec tsNowend;
    struct timespec tsNowbegin;
    D32S iTimes = 10;

    iRet = module_readFile2Json(gpcOtherInfoConfJsonPath, &OtherConfRoot);
    if (ADM_IS_ERR(iRet))
    {
        cout << "failed to read file[" << gpcOtherInfoConfJsonPath << "] to json, err[" << iRet << "]"<< endl;
        return NULL;
    }

    ADM_CDM_NOTIFY_Fn fnMonitorNotify = (ADM_CDM_NOTIFY_Fn)param;

    iCycleTime = OtherConfRoot["adm_cdm_LoopVerify"]["cycletimefailed"].asInt();
    if(0 == iCycleTime)
    {
        iCycleTime = 9999;
    }

    pStCondResults = (ADM_UIM_SOA_CONDITION_DATA_ARRAY_T*)adm_malloc((DSIZE)sizeof(ADM_UIM_SOA_CONDITION_DATA_ARRAY_T));

    cout << "======>download process monitor begin "<< endl;
    while(iCycleTime)
    {
        if(TRUE == gbStopMonitor)
        {
            fnMonitorNotify(pStCondResults,&bIsConform);
            return NULL;
        }
        fnMonitorNotify(pStCondResults,&bIsConform);
        sleep(1);
        iCycleTime--;
    }

    bIsConform = TRUE;
    fnMonitorNotify(pStCondResults,&bIsConform);
    cout << "======>download process monitor abnormal "<< endl;

    return NULL;
}

D32S __wrap_adm_cdm_LoopFinish(adm_handle_t timerHandle)
{
    cout << "======>download process monitor end "<< endl;
    gbStopMonitor = TRUE;
    return ADM_ECODE_SUCCESSED;
}


adm_handle_t __wrap_adm_cdm_LoopVerify(const ADM_COND_POLICY_T* pCondtionList,D32S iCycle,ADM_CDM_NOTIFY_Fn callback)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    Json::Value OtherConfRoot;
    pthread_t stThreadHandle = {0};
    
    iRet = module_readFile2Json(gpcOtherInfoConfJsonPath, &OtherConfRoot);
    if (ADM_IS_ERR(iRet))
    {
        cout << "failed to read file[" << gpcOtherInfoConfJsonPath << "] to json, err[" << iRet << "]"<< endl;
        return NULL;
    }

    iRet = OtherConfRoot["adm_cdm_LoopVerify"]["result"].asInt();
    EXPECT_EQ(ADM_ECODE_SUCCESSED, iRet);
    if (ADM_IS_ERR(iRet))
    {
        return NULL;
    }

    iRet = adm_dpm_pthreadCreate(&stThreadHandle, dpmMonitorThreadProc_test, (void*)callback);
    EXPECT_EQ(ADM_ECODE_SUCCESSED, iRet);
    if (ADM_IS_ERR(iRet))
    {
        return NULL;
    }

    return (adm_handle_t)stThreadHandle;
}
#if 0
D32S __wrap_adm_ocm_fileDownloadInterface(const DCHAR* downloadUrl, ADM_OCA_FILE_DOWNLOAD_INFO_T1* pstDownloadInfo)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    Json::Value OtherConfRoot;
    ADM_OCA_FILE_PROGRESS_T stProgress = {0};
    string runDir = "";
    string downpackageDir = "";
    D32S iProgressNotifyTimes = 10;

    iRet = module_readFile2Json(gpcOtherInfoConfJsonPath, &OtherConfRoot);
    if (ADM_IS_ERR(iRet))
    {
        cout << "failed to read file[" << gpcOtherInfoConfJsonPath << "] to json, err[" << iRet << "]"<< endl;
        return iRet;
    }

    iRet = OtherConfRoot["adm_ocm_fileDownloadInterface"].asInt();
    EXPECT_EQ(ADM_ECODE_SUCCESSED, iRet);
    //TODO:下载过程进度通知

    if(OtherConfRoot["upgradePackageType"].asInt() == DPM_DOWNPARTPACKAGE)
    {
        downpackageDir = "../dpm_data/drtp/downDir/";
        (void)adm_memzero(&stProgress, sizeof(stProgress));
        while( iProgressNotifyTimes-- && FALSE == gbNetchgPause && FALSE == gbCondAbnormalPause)  
        {
            cout << "===>download progress notify"<< endl;
            sleep(2);
            stProgress.speed = 1234;
            stProgress.savedSize += 10;
            stProgress.remainTime = 1;
            pstDownloadInfo->cbDownloadProgress(&stProgress,pstDownloadInfo->priv);
        }

        if(FALSE == gbNetchgPause && FALSE == gbCondAbnormalPause)
        {
            if(TRUE == gbfirstdowntime)
            {
                //原包
                downpackageDir +="f0895161425f444f9be310a4f231a0d5-merge-sign-en";
                iRet = CDpmAPITest::cpSrcfileToDstfile((DCHAR *)downpackageDir.c_str(), pstDownloadInfo->savePath);
                EXPECT_EQ(ADM_ECODE_SUCCESSED, iRet);
            }
            else 
            {
                //目的包
                downpackageDir +="f018dca5d18a4226bdf45c6613321b1c-merge-sign-en";
                iRet = CDpmAPITest::cpSrcfileToDstfile((DCHAR *)downpackageDir.c_str(), pstDownloadInfo->savePath);
                EXPECT_EQ(ADM_ECODE_SUCCESSED, iRet);
            }
            gbfirstdowntime = FALSE;
        }
    }
    else
    {
        downpackageDir = "../dpm_data/combine/downDir/";
        (void)adm_memzero(&stProgress, sizeof(stProgress));
        while( iProgressNotifyTimes-- && FALSE == gbNetchgPause && FALSE == gbCondAbnormalPause)  
        {
            cout << "===>download combine progress notify "<< endl;
            sleep(2);
            stProgress.speed = 1234;
            stProgress.savedSize += 10;
            stProgress.remainTime = 1;
            pstDownloadInfo->cbDownloadProgress(&stProgress, pstDownloadInfo->priv);
        }
        
        if(FALSE == gbNetchgPause && FALSE == gbCondAbnormalPause)
        {
            downpackageDir +="d2fa8ff8a3304ca9a1ba10350a03b480-merge-sign";
            iRet = CDpmAPITest::cpSrcfileToDstfile((DCHAR *)downpackageDir.c_str(), pstDownloadInfo->savePath);
            EXPECT_EQ(ADM_ECODE_SUCCESSED, iRet);
        }
    }

    if(TRUE == gbNetchgPause)
    {
        return ADM_ECODE_NET_CHG_PAUSE_DOWNLOAD;
    }

    if(TRUE == gbCondAbnormalPause)
    {
        return ADM_ECODE_DOWN_COND_ABNORMAL_PAUSE_DOWNLOAD;
    }

    return iRet;
}
#endif

D32S __wrap_adm_irm_reportEvent(const DCHAR* sessionId, D64S taskId, D64S strategyId, 
                         D64S timestamp, const DCHAR* sdkVersion, const DCHAR* extsReqJsonStr,
                         ADM_OCS_EVENTS_INFO_LIST_T* eventsList)
{
    return ADM_ECODE_SUCCESSED;
}

D32S __wrap_adm_dim_dpo_cleanDownloadDir(const D16U instanceId, const D16U timeout, D32S *errorCode)
{
    return ADM_ECODE_SUCCESSED;
}

D32S __wrap_adm_eim_getVehicleBigVersion(DCHAR** vehicleBigVersion)
{
    return ADM_ECODE_SUCCESSED;
}

D32S __wrap_adm_irm_reportVehicleState(D64S timestamp,
    const DCHAR* sdkVersion,
    const DCHAR* sessionId,
    D64S taskId,
    D64S strategyId,
    const DCHAR* state,
    const DCHAR* reason,
    const DCHAR* versionNumber,
    const DCHAR* extsReqJsonStr)
{
    return ADM_ECODE_SUCCESSED;
}


DBOOL __wrap_adm_vsm_getHmiOnline(void)
{
    return TRUE;
}

void __wrap_adm_uim_hmi_conds_downloadResult_RR(const ADM_APPLICATION_COMMON_CMDID_TYPE_E eCmdidType,const DCHAR *sessionId,const DCHAR *state,
                const ADM_UIM_SOA_CONDITION_DATA_ARRAY_T *condsArrarList,const D32S verifyResult,D32S *respCode)
{
    *respCode = 0;
    return;
}

void __wrap_adm_uim_hmi_ecu_downloadResult_RR(const ADM_APPLICATION_COMMON_CMDID_TYPE_E eCmdidType,const DCHAR *sessionId,const DCHAR *state,
                const ADM_UIM_SOA_ECU_IDEN_INFO_T *pstEcuIdenInfo,const ADM_UIM_SOA_DOWNLOAD_RESULT_INFO_T *pstEcuDownloadResult,D32S *respCode)
{
    *respCode = 0;
    return;
}

void __wrap_adm_uim_hmi_policy_downloadResult_RR(const ADM_APPLICATION_COMMON_CMDID_TYPE_E eCmdidType,const DCHAR *sessionId,const DCHAR *state,
                const ADM_UIM_SOA_DOWNLOAD_RESULT_INFO_T *pstPolicyDownloadResult,D32S *respCode)
{
    *respCode = 0;
    return;
}

D32S __wrap_adm_irm_syncReportVehicleState(D64S timestamp,
    const DCHAR* sdkVersion,
    const DCHAR* sessionId,
    D64S taskId,
    D64S strategyId,
    const DCHAR* state,
    const DCHAR* reason,
    const DCHAR* versionNumber,
    const DCHAR* extsReqJsonStr)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    Json::Value OtherConfRoot;

    iRet = module_readFile2Json(gpcOtherInfoConfJsonPath, &OtherConfRoot);
    if (ADM_IS_ERR(iRet))
    {
        cout << "failed to read file[" << gpcOtherInfoConfJsonPath << "] to json, err[" << iRet << "]"<< endl;
        return iRet;
    }

    iRet = OtherConfRoot["adm_irm_syncReportVehicleState"].asInt();
    EXPECT_EQ(ADM_ECODE_SUCCESSED, iRet);

    return iRet;
}

D32S __wrap_adm_ecs_getEventReportLevel(D32S *ps32EventReportLevel)
{
    *ps32EventReportLevel = ADM_CFG_VUS_EVENT_REPORT_LEVEL_DEFAULT_VAL;
    return ADM_ECODE_SUCCESSED;
}

static void *dpmDownProAndRtNotifyThreadProc_test(void *param)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    Json::Value OtherConfRoot;
    string runDir = "";
    string downpackageDir = "";
    D32S i = 0;
    void *priv=NULL;
    DCHAR *pFileid = NULL;
    DCHAR *pstorePath = NULL;
    ADM_DIM_SOA_DPO_DOWNLOAD_PROGRESS_INFO_T stDownloadProgress = {0};
    ADM_DIM_SOA_DPO_DOWNLOAD_RESULT_INFO_T stDownloadResultInfo = {0};


    pFileid = (DCHAR *)param;
    (void)adm_memzero(&stDownloadProgress, sizeof(stDownloadProgress));
    for(i=0; i<5; i++)
    {
        cout << "===>download progress notify"<< endl;
        sleep(2);
        stDownloadProgress.speed = 1234;
        stDownloadProgress.savedSize += 20;
        stDownloadProgress.totalSize = 100;
        stDownloadProgress.remainTime = 1;
        iRet = adm_dpm_dimSlaveDownProgressHandle(pFileid,&stDownloadProgress,priv);
        EXPECT_EQ(ADM_ECODE_SUCCESSED, iRet);
    }

    sleep(2);
    cout << "===>download result notify"<< endl;
    iRet = module_readFile2Json(gpcOtherInfoConfJsonPath, &OtherConfRoot);
    if (ADM_IS_ERR(iRet))
    {
        cout << "failed to read file[" << gpcOtherInfoConfJsonPath << "] to json, err[" << iRet << "]"<< endl;
        return NULL;
    }

    (void)adm_memzero(&stDownloadResultInfo, sizeof(stDownloadResultInfo));
    stDownloadResultInfo.result = OtherConfRoot["adm_ocm_fileDownloadInterface"].asInt();
    stDownloadResultInfo.realVal = adm_strdup("TransDownResult: ok. (realVal) --- test");
    stDownloadResultInfo.expectVal = adm_strdup("TransDownResult: ok. (expectVal) --- test");
    
    downpackageDir = "../dpm_data/drtp/downDir/";
    runDir = CORESERVICE_RUNDIR;
    runDir += "package/downPkg/";
    iRet = adm_com_createDir((DCHAR *)runDir.c_str());
    EXPECT_EQ(ADM_ECODE_SUCCESSED, iRet);

    if(TRUE == gbfirstdowntime)
    {
        //原包
        downpackageDir +="f0895161425f444f9be310a4f231a0d5-merge-sign-en";
        runDir += "f0895161425f444f9be310a4f231a0d5-merge-sign-en";
        iRet = CDpmAPITest::cpSrcfileToDstfile((DCHAR *)downpackageDir.c_str(), (DCHAR *)runDir.c_str());
        EXPECT_EQ(ADM_ECODE_SUCCESSED, iRet);
    }
    else 
    {
        //目的包
        downpackageDir +="f018dca5d18a4226bdf45c6613321b1c-merge-sign-en";
        runDir += "f018dca5d18a4226bdf45c6613321b1c-merge-sign-en";
        iRet = CDpmAPITest::cpSrcfileToDstfile((DCHAR *)downpackageDir.c_str(), (DCHAR *)runDir.c_str());
        EXPECT_EQ(ADM_ECODE_SUCCESSED, iRet);
    }
    iRet = adm_dpm_dimSlaveDownResultHandle(pFileid,(DCHAR *)runDir.c_str(),&stDownloadResultInfo,priv);
    EXPECT_EQ(ADM_ECODE_SUCCESSED, iRet);

    gbfirstdowntime = FALSE;
    return NULL;
}


D32S __wrap_adm_dim_dpo_requestRemoteDownloadPackage(const D16U instanceId,
        const D16U timeout, const DCHAR *fileID, const DCHAR *url, const D64S size,
        const DCHAR *hash, D32S *errorCode)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    Json::Value OtherConfRoot;
    pthread_t stThreadHandle = {0};

    iRet = module_readFile2Json(gpcOtherInfoConfJsonPath, &OtherConfRoot);
    if (ADM_IS_ERR(iRet))
    {
        cout << "failed to read file[" << gpcOtherInfoConfJsonPath << "] to json, err[" << iRet << "]"<< endl;
        return iRet;
    }

    *errorCode = 0;
    iRet = OtherConfRoot["adm_dim_dpo_requestRemoteDownloadPackage"].asInt();
    EXPECT_EQ(ADM_ECODE_SUCCESSED, iRet);

    if(iRet == ADM_ECODE_SUCCESSED)
    {
        iRet = adm_dpm_pthreadCreate(&stThreadHandle, dpmDownProAndRtNotifyThreadProc_test, (void*)fileID);
        EXPECT_EQ(ADM_ECODE_SUCCESSED, iRet);
    }

    return iRet;
}

D32S __wrap_adm_pub_downPackageDecrypt(ADM_COM_FILEOBJECT_T *pstPkgFileObj,DCHAR **ppcDecryptPath,DCHAR *pcDownInstId)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    Json::Value OtherConfRoot;
    string downpackageDir = "";
    string runDir = "";

    iRet = module_readFile2Json(gpcOtherInfoConfJsonPath, &OtherConfRoot);
    if (ADM_IS_ERR(iRet))
    {
        cout << "failed to read file[" << gpcOtherInfoConfJsonPath << "] to json, err[" << iRet << "]"<< endl;
        return iRet;
    }

    iRet = OtherConfRoot["adm_pub_downPackageDecrypt"].asInt();
    if (ADM_IS_ERR(iRet))
    {
        return iRet;
    }

    downpackageDir = "../dpm_data/drtp/decryptDir/";
    downpackageDir += pstPkgFileObj->fileName;
    runDir = CORESERVICE_RUNDIR;
    runDir += "package/decryptPkg/";
    iRet = adm_com_createDir((DCHAR *)runDir.c_str());
    EXPECT_EQ(ADM_ECODE_SUCCESSED, iRet);

    iRet =CDpmAPITest::cpSrcfileToDstfile((DCHAR *)downpackageDir.c_str(),(DCHAR *)runDir.c_str());
    EXPECT_EQ(ADM_ECODE_SUCCESSED, iRet);

    runDir += pstPkgFileObj->fileName;
    *ppcDecryptPath = adm_strdup((DCHAR *)runDir.c_str());

    return iRet;
}

D32S __wrap_adm_dim_host_verifySignature(const D16U instanceId, const D16U timeout,
    const DCHAR* signedData, const D32S signedDataLength,
    const DCHAR* orignalData, const D32S orignalDataLength,
    D32S* errorCode)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    Json::Value OtherConfRoot;

    iRet = module_readFile2Json(gpcOtherInfoConfJsonPath, &OtherConfRoot);
    if (ADM_IS_ERR(iRet))
    {
        cout << "failed to read file[" << gpcOtherInfoConfJsonPath << "] to json, err[" << iRet << "]"<< endl;
        return iRet;
    }

    iRet = OtherConfRoot["adm_dim_host_verifySignature"].asInt();
    EXPECT_EQ(ADM_ECODE_SUCCESSED, iRet);

    *errorCode = 0;
    return iRet;
}

D32S __wrap_adm_pub_downPackageVerifySign(
    ADM_COM_FILEOBJECT_T  *pstPkgFileObj,
    DCHAR *pcDownInstId,
    DCHAR *pfileid,
    DBOOL isUnpick,
    ADM_SYSI4UCL_VERIFY_SIGN_RESULT_INFO_T *pstVerifySignResultInfo
)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    Json::Value OtherConfRoot;

    iRet = module_readFile2Json(gpcOtherInfoConfJsonPath, &OtherConfRoot);
    if (ADM_IS_ERR(iRet))
    {
        cout << "failed to read file[" << gpcOtherInfoConfJsonPath << "] to json, err[" << iRet << "]"<< endl;
        return iRet;
    }

    iRet = OtherConfRoot["adm_pub_downPackageVerifySign"].asInt();
    EXPECT_EQ(ADM_ECODE_SUCCESSED, iRet);
    return iRet;
}

DCHAR *__wrap_adm_ecs_getDownloadInstanceId(DCHAR *ecuReqId, DCHAR *ecuSwDid)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    D32S i = 0;
    DCHAR *pcDownloadInstanceId = NULL;
    ADM_ECS_ECUCONFIG_T *pstEcuCfg = NULL;

    EXPECT_NE(nullptr, ecuReqId);
    EXPECT_NE(nullptr, ecuSwDid);

    for (i = 0; i< gstEcsRsConfigData.stRsCfgInfo.stPartCfg.stEcuCfgList.num; i++)
    {
        pstEcuCfg = &gstEcsRsConfigData.stRsCfgInfo.stPartCfg.stEcuCfgList.pstEcuCfg[i];
        if ((0 == adm_strcasecmp(ecuReqId, pstEcuCfg->ecuReqId)) &&
            (0 == adm_strcasecmp(ecuSwDid, pstEcuCfg->ecuSwDid)))
        {
            pcDownloadInstanceId = adm_strdup(pstEcuCfg->downloadInstanceId);
            break;
        }
    }
    return pcDownloadInstanceId;
}

void __wrap_adm_pis_packageImageSet(const ADM_PIS_PACKAGE_HEAD_T *head, const ADM_PIS_FILE_T *imagePackage)
{
    return;
}

void __wrap_adm_pis_packageSecuritySet(const ADM_PIS_PACKAGE_HEAD_T *head, const ADM_COM_FILESCYINFO_T *fileSecurityInfo)
{
    return;
}

D32S __wrap_adm_tpm_triggerTransferPkg(ADM_APPLICATION_COMMON_CMDID_TYPE_E eTriggerType)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    Json::Value OtherConfRoot;

    iRet = module_readFile2Json(gpcOtherInfoConfJsonPath, &OtherConfRoot);
    if (ADM_IS_ERR(iRet))
    {
        cout << "failed to read file[" << gpcOtherInfoConfJsonPath << "] to json, err[" << iRet << "]"<< endl;
        return iRet;
    }

    iRet = OtherConfRoot["adm_tpm_triggerTransferPkg"].asInt();
    EXPECT_EQ(ADM_ECODE_SUCCESSED, iRet);
    return iRet;
}

D32S __wrap_adm_cdm_verify_free(ADM_UIM_SOA_CONDITION_DATA_ARRAY_T* pStCondResults)
{
    return ADM_ECODE_SUCCESSED;
}

DBOOL __wrap_adm_vsm_triggerDownload(ADM_APPLICATION_COMMON_CMDID_TYPE_E eTriggerType)
{
    return TRUE;
}

void __wrap_adm_uim_policyDownloadProgressNotify(const
        ADM_APPLICATION_COMMON_CMDID_TYPE_E eCmdidType,
        const DCHAR *sessionId,
        const DCHAR *state,
        const ADM_UIM_SOA_DOWNLOAD_PROGRESS_INFO_T *pstPolicyDownloadProgress)
{
    return ;
}

D32S __wrap_adm_uim_ecuDownloadProgressNotify(const ADM_APPLICATION_COMMON_CMDID_TYPE_E
                                       eCmdidType,
                                       const DCHAR *sessionId,
                                       const DCHAR *state,
                                       const ADM_UIM_SOA_ECU_IDEN_INFO_T *pstEcuIdenInfo,
                                       const ADM_UIM_SOA_DOWNLOAD_PROGRESS_INFO_T *pstEcuDownloadProgress)
{
    return ADM_ECODE_SUCCESSED;
}

D32S __wrap_adm_uim_allEcuDownloadProgressNotify(const
        ADM_APPLICATION_COMMON_CMDID_TYPE_E eCmdidType,
        const DCHAR *sessionId,
        const DCHAR *state,
        const ADM_UIM_SOA_ECU_DOWNLOAD_PROGRESS_ARRAY_INFO_T
        *pstEcuDownloadProgressArrayInfo,
        const ADM_UIM_SOA_DOWNLOAD_PROGRESS_INFO_T *pstPolicyDownloadProgress)
{
    return ADM_ECODE_SUCCESSED;
}

void __wrap_adm_uim_hmi_allecu_downloadResult_RR(const ADM_APPLICATION_COMMON_CMDID_TYPE_E eCmdidType,const DCHAR *sessionId,const DCHAR *state,
                const ADM_UIM_SOA_ECU_DOWNLOAD_RESULT_ARRAY_INFO_T *pstEcuDownloadResultArrayInfo,const ADM_UIM_SOA_DOWNLOAD_RESULT_INFO_T *pstPolicyDownloadResult,D32S *respCode)
{
    return ;
}

void __wrap_adm_vsm_hmiIsAutoDownload(DBOOL isAutoDownload, D32S *errorCode)
{
    return ;
}

#if 0
D32S __wrap_adm_oca_interuptDownload(const D32S actionType)
{

    if(actionType == (D32S)ADM_DPM_DOWNLOAD_ACTION_TYPE_NET_CHG_PAUSE) //!< 由于网络切换导致的暂停
    {
        cout << "===============>NET CHG PAUSE "<< endl;
        gbNetchgPause = TRUE;
    }
    else if(actionType == (D32S)ADM_OCA_DOWNLOAD_ACTION_TYPE_COND_ABNORMAL_PAUSE)  //!< 由于下载条件异常导致的暂停
    {
        cout << "===============>COND ABNORMAL PAUSE "<< endl;
        gbCondAbnormalPause = TRUE;
    }
    else
    {
        ;
    }
    return ADM_ECODE_SUCCESSED;
}
#endif

D64S __wrap_adm_irm_getEventSequenceId(void)
{
    return ADM_ECODE_SUCCESSED;
}

D32S __wrap_adm_irm_reportEcuState(const DCHAR* sessionId,
    D64S taskId,
    D64S strategyId,
    D64S timestamp,
    const DCHAR* sdkVersion,
    const DCHAR* extsReqJsonStr,
    ADM_OCS_ECU_STATE_INFO_LIST_T* ecusList)
{
    return ADM_ECODE_SUCCESSED;
}

D32S __wrap_adm_irm_reportVehicleProgress(const DCHAR* sessionId,
    D64S taskId,
    D64S strategyId,
    D64S timestamp,
    const DCHAR* sdkVersion,
    const DCHAR* extsReqJsonStr,
    ADM_OCS_ECU_PROGRESS_INFO_LIST_T* ecusProgressList)
{
    return ADM_ECODE_SUCCESSED;
}

D32S __wrap_adm_getSdkVersion(DCHAR** sdkVersion)
{
    return ADM_ECODE_SUCCESSED;
}

D32S __wrap_adm_ecs_getOneEcuDidListInfo(DCHAR *ecuReqId, DCHAR *ecuSwDid, ADM_ECS_ECUDIDLIST_T *pstEcuDidList)
{
    D32S i = 0;
    D32S j = 0;
    D32S s32Index = 0;
    ADM_ECS_ECUDIDLIST_T *pstTmpEcuDidList = NULL;
    //!< 计算ecudid的个数（通用配置中的ecuDidList + 对应零件配置中的ecuDidList）
    pstEcuDidList->num = gstEcsRsConfigData.stRsCfgInfo.stGeneralCfg.stEcuDidList.num;
    for (i = 0; i < gstEcsRsConfigData.stRsCfgInfo.stPartCfg.stEcuCfgList.num; i++)
    {
        if ((0 == adm_strcasecmp(ecuReqId, gstEcsRsConfigData.stRsCfgInfo.stPartCfg.stEcuCfgList.pstEcuCfg[i].ecuReqId)) &&
            (0 == adm_strcasecmp(ecuSwDid, gstEcsRsConfigData.stRsCfgInfo.stPartCfg.stEcuCfgList.pstEcuCfg[i].ecuSwDid)))
        {
            pstEcuDidList->num += gstEcsRsConfigData.stRsCfgInfo.stPartCfg.stEcuCfgList.pstEcuCfg[i].stEcuDidList.num;
            break;
        }
    }

    pstEcuDidList->pstEcuDidInfo = (ADM_ECS_ECUDID_T *)malloc((D32U)pstEcuDidList->num * sizeof(ADM_ECS_ECUDID_T));
    if (NULL != pstEcuDidList->pstEcuDidInfo)
    {
        (void)adm_memzero(pstEcuDidList->pstEcuDidInfo, (D32U)pstEcuDidList->num * sizeof(ADM_ECS_ECUDID_T));
    }

    if ((0 < pstEcuDidList->num) && ADM_IS_NULL(pstEcuDidList->pstEcuDidInfo))
    {
        pstEcuDidList->num = 0;
        log_e("Failed to new ResDidIlist, err[%d]", ADM_ECODE_NO_MEMORY);
        return ADM_ECODE_NO_MEMORY;
    }

    //!< 填充通用配置中的dids
    pstTmpEcuDidList = &gstEcsRsConfigData.stRsCfgInfo.stGeneralCfg.stEcuDidList;
    for (i = 0; i < pstTmpEcuDidList->num; i++)
    {
        pstEcuDidList->pstEcuDidInfo[s32Index].did   = adm_strdup(pstTmpEcuDidList->pstEcuDidInfo[i].did);
        pstEcuDidList->pstEcuDidInfo[s32Index].end   = pstTmpEcuDidList->pstEcuDidInfo[i].end;
        pstEcuDidList->pstEcuDidInfo[s32Index].start = pstTmpEcuDidList->pstEcuDidInfo[i].start;
        pstEcuDidList->pstEcuDidInfo[s32Index].didValType = pstTmpEcuDidList->pstEcuDidInfo[i].didValType;
        ++s32Index;
    }

    //!< 填充对应零件配置中的dids
    for (i = 0; i < gstEcsRsConfigData.stRsCfgInfo.stPartCfg.stEcuCfgList.num; i++)
    {
        if ((0 == adm_strcasecmp(ecuReqId, gstEcsRsConfigData.stRsCfgInfo.stPartCfg.stEcuCfgList.pstEcuCfg[i].ecuReqId)) &&
            (0 == adm_strcasecmp(ecuSwDid, gstEcsRsConfigData.stRsCfgInfo.stPartCfg.stEcuCfgList.pstEcuCfg[i].ecuSwDid)))
        {
            pstTmpEcuDidList = &gstEcsRsConfigData.stRsCfgInfo.stPartCfg.stEcuCfgList.pstEcuCfg[i].stEcuDidList;
            for (j = 0; j < pstTmpEcuDidList->num; j++)
            {
                pstEcuDidList->pstEcuDidInfo[s32Index].did   = adm_strdup(pstTmpEcuDidList->pstEcuDidInfo[j].did);
                pstEcuDidList->pstEcuDidInfo[s32Index].end   = pstTmpEcuDidList->pstEcuDidInfo[j].end;
                pstEcuDidList->pstEcuDidInfo[s32Index].start = pstTmpEcuDidList->pstEcuDidInfo[j].start;
                pstEcuDidList->pstEcuDidInfo[s32Index].didValType = pstTmpEcuDidList->pstEcuDidInfo[j].didValType;
                ++s32Index;
            }
            break;
        } 
    }

    return ADM_ECODE_SUCCESSED;
}


#ifdef __cplusplus
}
#endif

void CDpmAPITest::SetUpTestCase()
{
    cout << "===================================beign================================" << endl;
    D32S iRet = ADM_ECODE_SUCCESSED;
    D32S idownTrigerType = 0;

    //清空目录
    cout << "=======>destroy RunDir" << endl;
    CDpmAPITest::destroyCoreServiceRunDir();
    cout << "=======>destroy RunDir success" << endl;

    //创建目录
    cout << "=======>create RunDir" << endl;
    iRet = CDpmAPITest::createCoreServiceRunDir();
    EXPECT_EQ(ADM_ECODE_SUCCESSED, iRet);
    cout << "=======>create RunDir success" << endl;

    //设置下载参数
    cout << "=======>Set Down Param" << endl;
    CDpmAPITest::setDownParameter();
    cout << "=======>Set Down Param success" << endl;

    //获取配置信息
    adm_integrationtest_setConfigFileDir(gpcConfigDataJsonPath);

    iRet = adm_integrationtest_getEcsRsConfigData(&gstEcsRsConfigData);
    EXPECT_EQ(ADM_ECODE_SUCCESSED, iRet);

    //获取下载触发类型
    CDpmAPITest::getdownTrigerType(&idownTrigerType);

    cout << "=======>trigger Download" << endl;
    iRet = adm_dpm_triggerDownload((ADM_APPLICATION_COMMON_CMDID_TYPE_E)idownTrigerType);
    EXPECT_EQ(ADM_ECODE_SUCCESSED, iRet);
    sleep(3);

    cout << "=======================================end=====================================" << endl;
}



/*!
 *******************************************************************************
 * 拷贝文件.
 * 
 * \note
 *
 * \param[in]   pSrcfilePath    原文件路径
 * \param[in]   pDstfilePath    目标文件路径
 * \retval      D32S
 *******************************************************************************
 */
D32S CDpmAPITest::cpSrcfileToDstfile(DCHAR *pSrcfilePath,DCHAR *pDstfilePath)
{
    EXPECT_NE(nullptr, pSrcfilePath);
    EXPECT_NE(nullptr, pDstfilePath);

    DCHAR szCmd[256] = {0};

    (void)adm_snprintf(szCmd, sizeof(szCmd), "cp %s %s 1>/dev/null 2>&1 ", pSrcfilePath, pDstfilePath);
    (void)system(szCmd);

    return ADM_ECODE_SUCCESSED;
}


void CDpmAPITest::getdownTrigerType(D32S *pidownTrigerType)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    Json::Value OtherConfRoot;

    iRet = module_readFile2Json(gpcOtherInfoConfJsonPath, &OtherConfRoot);
    if (ADM_IS_ERR(iRet))
    {
        cout << "failed to read file[" << gpcOtherInfoConfJsonPath << "] to json, err[" << iRet << "]"<< endl;
        return;
    }

    *pidownTrigerType = OtherConfRoot["downTrigerType"].asInt(); 
    cout << "=======>downTrigerType["<< *pidownTrigerType <<"] "<< endl;

    return;
}

void CDpmAPITest::setDownParameter()
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    D32S iupgradePackageType = 0;
    Json::Value OtherConfRoot;

    iRet = module_readFile2Json(gpcOtherInfoConfJsonPath, &OtherConfRoot);
    if (ADM_IS_ERR(iRet))
    {
        cout << "failed to read file[" << gpcOtherInfoConfJsonPath << "] to json, err[" << iRet << "]"<< endl;
        return;
    }

    iupgradePackageType = OtherConfRoot["upgradePackageType"].asInt();

    if(iupgradePackageType == DPM_DOWNPARTPACKAGE)
    {
        gpcCheckPolicyJsonPath = adm_strdup("../dpm_data/drtp/getDownPartPolicy.json");   //!< 检测结果(下载策略中是零件包)
        gpcConfigDataJsonPath  = adm_strdup("../dpm_data/drtp/config/");
    }
    else
    {
        gpcCheckPolicyJsonPath = adm_strdup("../dpm_data/combine/getDownCombinePolicy.json");   //!< 检测结果(下载策略中是集合包)
        gpcConfigDataJsonPath  = adm_strdup("../dpm_data/combine/config/");
    }

    //设置日志目录
    adm_debug_logInitWithFileName("../dpm_data/runDir/log", "admfotaapp", ADM_CFG_VUS_LOG_MAX_SIZE_DEFAULT_VAL, ADM_CFG_VUS_LOG_MAX_ROTATE_DEFAULT_VAL, ELOG_LVL_DEBUG);

    return;
    cout << "SetUpTestCase" << endl;
}

/*!
 *******************************************************************************
 * 创建coreService运行目录.
 * 
 * \note vrs kv存储路径
 *
 * \retval 	     D32S
 *******************************************************************************
 */
D32S CDpmAPITest::createCoreServiceRunDir(void)
{
    DBOOL bIsOk = FALSE;
    string runDir = "";
    D32S iRet = ADM_ECODE_SUCCESSED;

    runDir = CORESERVICE_RUNDIR;
    runDir += "kv/";
    iRet = adm_com_createDir((DCHAR *)runDir.c_str());
    EXPECT_EQ(ADM_ECODE_SUCCESSED, iRet);
    bIsOk = adm_core_service_initRuntimeDir(ADM_CORE_SERVICE_RUNTIME_DIR_TYPE_KV_STORAGE, runDir.c_str());
    EXPECT_EQ(TRUE, bIsOk);

    runDir = CORESERVICE_RUNDIR;
    runDir += "log/";
    iRet = adm_com_createDir((DCHAR *)runDir.c_str());
    EXPECT_EQ(ADM_ECODE_SUCCESSED, iRet);
    bIsOk = adm_core_service_initRuntimeDir(ADM_CORE_SERVICE_RUNTIME_DIR_TYPE_LOG, runDir.c_str());
    EXPECT_EQ(TRUE, bIsOk);
    
    runDir = CORESERVICE_RUNDIR;
    runDir += "package/";
    iRet = adm_com_createDir((DCHAR *)runDir.c_str());
    EXPECT_EQ(ADM_ECODE_SUCCESSED, iRet);
    bIsOk = adm_core_service_initRuntimeDir(ADM_CORE_SERVICE_RUNTIME_DIR_TYPE_PACKAGE, runDir.c_str());
    EXPECT_EQ(TRUE, bIsOk);
    
    runDir = CORESERVICE_RUNDIR;
    runDir += "cert/";
    iRet = adm_com_createDir((DCHAR *)runDir.c_str());
    EXPECT_EQ(ADM_ECODE_SUCCESSED, iRet);
    bIsOk = adm_core_service_initRuntimeDir(ADM_CORE_SERVICE_RUNTIME_DIR_TYPE_CERT, runDir.c_str());
    EXPECT_EQ(TRUE, bIsOk);
    
    runDir = CORESERVICE_RUNDIR;
    runDir += "unzip/";
    iRet = adm_com_createDir((DCHAR *)runDir.c_str());
    EXPECT_EQ(ADM_ECODE_SUCCESSED, iRet);
    bIsOk = adm_core_service_initRuntimeDir(ADM_CORE_SERVICE_RUNTIME_DIR_TYPE_UNZIP, runDir.c_str());
    EXPECT_EQ(TRUE, bIsOk);

    //!< coreService init
    iRet = adm_core_service_init();
    EXPECT_EQ(ADM_ECODE_SUCCESSED, iRet);

    return ADM_ECODE_SUCCESSED;
}

/*!
 *******************************************************************************
 * 销毁coreService运行目录.
 * 
 * \note vrs kv存储路径
 *
 * \retval none
 *******************************************************************************
 */
void CDpmAPITest::destroyCoreServiceRunDir(void)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    string command = "rm -rf ";
    D32S i = ADM_CORE_SERVICE_RUNTIME_DIR_TYPE_KV_STORAGE;
    D32S isclearKvData = 0;
    Json::Value OtherConfRoot;

    iRet = module_readFile2Json(gpcOtherInfoConfJsonPath, &OtherConfRoot);
    if (ADM_IS_ERR(iRet))
    {
        cout << "failed to read file[" << gpcOtherInfoConfJsonPath << "] to json, err[" << iRet << "]"<< endl;
        return;
    }

    isclearKvData = OtherConfRoot["isclearKvData"].asInt();
    
    for (i = ADM_CORE_SERVICE_RUNTIME_DIR_TYPE_KV_STORAGE; i <= ADM_CORE_SERVICE_RUNTIME_DIR_TYPE_UNZIP; ++i)
    {
        adm_core_service_deinitRuntimeDir((ADM_CORE_SERVICE_RUNTIME_DIR_TYPE_E)i);
    }

    //!< coreService deInit
    adm_core_service_deinit();

    command += CORESERVICE_RUNDIR;
    command += "log/";
    cout << "[----------] ===> command " << command << endl;
    //!< 删除文件夹
    system((DCHAR*)command.c_str());

    command = "rm -rf ";
    command += CORESERVICE_RUNDIR;
    command += "package/";
    cout << "[----------] ===> command " << command << endl;
    //!< 删除文件夹
    system((DCHAR*)command.c_str());

    command = "rm -rf ";
    command += CORESERVICE_RUNDIR;
    command += "cert/";
    cout << "[----------] ===> command " << command << endl;
    //!< 删除文件夹
    system((DCHAR*)command.c_str());

    command = "rm -rf ";
    command += CORESERVICE_RUNDIR;
    command += "unzip/";
    cout << "[----------] ===> command " << command << endl;
    //!< 删除文件夹
    system((DCHAR*)command.c_str());

    if(isclearKvData != 0)
    {
        command = "rm -rf ";
        command += CORESERVICE_RUNDIR;
        command += "kv/";
        cout << "[----------] ===> command " << command << endl;
        //!< 删除文件夹
        system((DCHAR*)command.c_str());
    }
    
    return;
}


void CDpmAPITest::TearDownTestCase()
{
    adm_dpm_moduleDeInit();

    cout<<"TearDownTestCase"<<endl;
}

/**
 * @brief 初始化OTA配置文件
 * 
 * @param filename [in]     配置文件的绝对路径
 * @return D32S     0:Successed, None-0:Failed
 */
D32S CDpmAPITest::dpmOtaConfigInit(const DCHAR* filename)
{
    D32S iRet = ADM_ECODE_FAILED;
    ADM_CFG_API_INFO_T pstApiData = {0};

    do {
        if (NULL == filename) {
            iRet = ADM_ECODE_INVALID_PARAM;
            break;
        }

        pstApiData.isFileExistCb = adm_com_isFileExist;
        pstApiData.writeFileDataCb = adm_com_writeFileData;
        iRet = adm_cfg_iniParserInit(filename, &pstApiData);
        if (ADM_ECODE_SUCCESSED != iRet) {
            break;
        }
    } while(FALSE);

    return iRet;
}


TEST_F(CDPM_TEST_Download, Normal)
{
    CDpmAPITest aa;
    aa.SetUpTestCase();
}

TEST_F(CDPM_TEST_DownloadEnd, Normal)
{
    CDpmAPITest aa;
    aa.TearDownTestCase();
}


#ifndef _WIN32
int main(int argc, char** argv)
{
    D32S iRet = ADM_ECODE_FAILED;
    D32S intVal = 0;
    DCHAR* otaConfigPath = NULL;
    size_t nSize = 0;
    D32S c = 0;
    D32S endMark =0;

    //!< 参数个数判断
    if (1 == argc) {
        return -1;
    }

    //!< 接收入参
    while ((c = getopt (argc, argv, "c:")) != -1)
    {
        switch (c)
        {
            case (int)'c' :
                otaConfigPath = optarg;
                (void)printf("optarg:%s\n", optarg);
                break;
            case (int)'?':
            {
                if ((char)optopt == 'o')
                {
                    (void)fprintf (stderr, "Option -%c requires an argument.\n", optopt);
                }
                else if (0u != isprint ((unsigned char)optopt))
                {
                    (void)fprintf (stderr, "Unknown option `-%c'.\n", optopt);
                }
                else
                {
                    (void)fprintf (stderr, "Unknown option character `\\x%x'.\n", optopt);
                }
                endMark =-1;
                break;
            }
            default:
            {
                endMark =-1;
                abort ();
                break;
            }
        }
        if(-1 == endMark)
        {
            return -1;
        }
    }

    //解析配置文件 
    iRet = CDpmAPITest::dpmOtaConfigInit(otaConfigPath);
    EXPECT_EQ(ADM_ECODE_SUCCESSED, iRet);

    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
#endif


