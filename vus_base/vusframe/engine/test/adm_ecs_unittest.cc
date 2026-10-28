/**
 * @file    adm_ecs_unittest.cc
 * @brief   ecs module unit test  
 * @details ecs 模块单元测试用例
 * @author  wangyanpeng
 * @version v0.1.0
 * @date    2020-11-06
 * @copyright www.abupdate.com
 */
#include "adm_com_errorCode.h"
#include "adm_com_lib.h"
#include "adm_ecs_unittest.h"
#include "json/json.h"
#include "adm_string.h"
#include "adm_memory.h"

#include <fstream>
#include <iostream>

using namespace std;

/*!
 *******************************************************************************
 * \note 依赖外部模拟数据清单.
 *      ./data/getConfReqParam.json     --- 构造配置请求数据
 *      ./data/getConfRsp.json          --- 构造云端配置返回的数据
 *      ./data/HD_CFG.zip               --- 返回数据的配置文件压缩包
 *      ./data/unzip/PartConfig.json    --- 配置文件中的零件信息
 *      ./data/unzip/GeneralConfig.json --- 配置文件中的通用信息
 * 
 *******************************************************************************
 */
#define CORESERVICE_RUNDIR               "./data/runDir/"
static const DCHAR *gpcConfReqJsonPath = "./data/getConfReqParam.json";
static const DCHAR *gpcConfRspJsonPath = "./data/getConfRsp.json";
static const DCHAR *gpcPartConfFilePath    = "./data/runDir/unzip/confPkg/PartConfig.json";
static const DCHAR *gpcGeneralConfFilePath = "./data/runDir/unzip/confPkg/GeneralConfig.json";


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
    std::ifstream ifs;
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
        std::cout << "parseFromStream failed " << errs << std::endl;
        return ADM_ECODE_FAILED;
    }
    ifs.close();

    return ADM_ECODE_SUCCESSED;
}

#if 0
#endif

void CEcsAPITest::SetUpTestCase()
{
    D32S iRet = ADM_ECODE_SUCCESSED;

    iRet = CEcsAPITest::createCoreServiceRunDir();
    EXPECT_EQ(ADM_ECODE_SUCCESSED, iRet);

    iRet = adm_ecs_moduleInit(ecs_regGetReqCloudParamAPI, 
                              ecs_regReqCloudAPi, 
                              ecs_regDownloadAPI,
                              NULL,
                              NULL);
    EXPECT_EQ(ADM_ECODE_SUCCESSED, iRet);

    iRet = adm_ecs_updateAndGetConfigInfo(NULL);
    EXPECT_EQ(ADM_ECODE_SUCCESSED, iRet);

    // cout << "SetUpTestCase" << endl;
}

void CEcsAPITest::TearDownTestCase()
{
    adm_ecs_moduleDeInit();
    CEcsAPITest::destroyCoreServiceRunDir();

    // cout<<"TearDownTestCase"<<endl;
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
D32S CEcsAPITest::createCoreServiceRunDir(void)
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
void CEcsAPITest::destroyCoreServiceRunDir(void)
{
    string command = "rm -rf ";
    D32S i = ADM_CORE_SERVICE_RUNTIME_DIR_TYPE_KV_STORAGE;
    
    for (i = ADM_CORE_SERVICE_RUNTIME_DIR_TYPE_KV_STORAGE; i <= ADM_CORE_SERVICE_RUNTIME_DIR_TYPE_UNZIP; ++i)
    {
        adm_core_service_deinitRuntimeDir((ADM_CORE_SERVICE_RUNTIME_DIR_TYPE_E)i);
    }

    //!< coreService deInit
    adm_core_service_deinit();

    command += CORESERVICE_RUNDIR;
    // cout << "[----------] ===> command " << command << endl;
    //!< 删除文件夹
    system((DCHAR*)command.c_str());
    return;
}

/*!
 *******************************************************************************
 * 构造ecs获取请求云端参数信息回调函数.
 *
 * \note 
 *
 * \param[in]	pstReqParam  请求云端参数信息
 * \retval 	    D32S
 *******************************************************************************
 */
D32S CEcsAPITest::ecs_regGetReqCloudParamAPI(ADM_ECS_REQPARAM_T *pstReqParam)
{
    Json::Value root;
    std::ifstream ifs;
    JSONCPP_STRING errs;
    Json::CharReaderBuilder builder;

    ifs.open(gpcConfReqJsonPath, ifstream::in);
    if (!ifs.is_open())
    {
        cout << "failed to open filePath[" << gpcConfReqJsonPath << "], err[" << strerror(errno) << "]" << endl;
        return ADM_ECODE_FAILED;
    }

    if (!parseFromStream(builder, ifs, &root, &errs)) 
    {
        std::cout << "parseFromStream failed " << errs << std::endl;
    }
    ifs.close();

    pstReqParam->secret      = adm_strdup(root["secret"].asString().c_str());
    pstReqParam->deviceId    = adm_strdup(root["deviceId"].asString().c_str());
    pstReqParam->sdkVersion  = adm_strdup(root["sdkVersion"].asString().c_str());
    pstReqParam->confFileDir = adm_strdup(root["confFileDir"].asString().c_str());
    pstReqParam->timestamp      = root["timestamp"].asInt64();

    return ADM_ECODE_SUCCESSED;
}

/*!
 *******************************************************************************
 * 构造ecs请求云端动作回调函数.
 *
 * \note 
 *
 * \param[in]	eBusinessType   业务类型
 * \param[in]	reqMessageBody  请求消息
 * \param[out]	rspMessageBody  云端响应信息
 * \param[out]	rspStatusCode   云端响应状态码
 * \retval 	    D32S
 *******************************************************************************
 */
D32S CEcsAPITest::ecs_regReqCloudAPi(const ADM_OCS_BUSINESS_TYPE_E eBusinessType, const DCHAR* reqMessageBody, DCHAR** rspMessageBody, DSIZE* rspStatusCode)
{
    std::ifstream ifs;
    D32S length = 0;

    EXPECT_NE(nullptr, reqMessageBody);
    EXPECT_NE(nullptr, rspMessageBody);
    EXPECT_NE(nullptr, rspStatusCode);

    ifs.open(gpcConfRspJsonPath, ifstream::in);
    if (!ifs.is_open())
    {
        cout << "failed to open filePath[" << gpcConfRspJsonPath << "], err[" << strerror(errno) << "]" << endl;
        return ADM_ECODE_FAILED;
    }
    ifs.seekg(0, std::ios::end);
    length = ifs.tellg();
    ifs.seekg(0, std::ios::beg);

    *rspMessageBody = (DCHAR*)adm_malloc((D32U)sizeof(DCHAR) * (D32U)(length + 1));
    if (NULL == *rspMessageBody)
    {
        ifs.close();
        cout << "failed to malloc" << endl;
        return ADM_ECODE_FAILED;
    }
    adm_memzero(*rspMessageBody, sizeof(DCHAR) * (length + 1));

    ifs.read(*rspMessageBody, length);
    ifs.close();

    *rspStatusCode = 200;
    // cout << "rspMessageBody = " << *rspMessageBody << endl;
    // cout << "[----------] ===> Request Cloud API callback ok." << endl;

    return ADM_ECODE_SUCCESSED;
}

/*!
 *******************************************************************************
 * 构造ecs下载配置文件动作回调函数.
 *
 * \note 
 *
 * \param[in]	downloadUrl     下载url
 * \param[in]	pstDownloadInfo 下载消息
 * \retval 	    D32S
 *******************************************************************************
 */
D32S CEcsAPITest::ecs_regDownloadAPI(const DCHAR* downloadUrl, ADM_OCA_FILE_DOWNLOAD_INFO_T1* pstDownloadInfo)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    string command = "cp -f ";
    Json::Value root;

    cout << "[----------] ===> Entry download API callback." << endl;
    // iRet = adm_com_createFileDirAndFile(pstDownloadInfo->savePath, TRUE);
    // if (ADM_IS_ERR(iRet))
    // {
    //     cout << "failed to create file dir or file, err[" << iRet << "]"<< endl;
    //     return iRet;
    // }
    cout << "[----------] ===> download savePath[" << pstDownloadInfo->savePath << "]"<< endl;

    // iRet = module_readFile2Json(gpcConfRspJsonPath, &root);
    // if (ADM_IS_ERR(iRet))
    // {
    //     cout << "failed to read file to json, err[" << iRet << "]"<< endl;
    //     return iRet;
    // }

    // command += "./data/" + root["data"]["fileObject"]["fileName"].asString() + " ";
    // command += pstDownloadInfo->savePath;

    // //!< 移动文件
    // system((DCHAR*)command.c_str());
    cout << "[----------] ===> download API callback ok." << endl;

    return ADM_ECODE_SUCCESSED;
}

#if 0
#endif

TEST_F(CEcsAPITest, moudleInit_Normal)
{
    D32S iRet = ADM_ECODE_SUCCESSED;

    iRet = adm_ecs_moduleInit(NULL, 
                              CEcsAPITest::ecs_regReqCloudAPi, 
                              CEcsAPITest::ecs_regDownloadAPI,
                              NULL,
                              NULL);
    EXPECT_NE(iRet, ADM_ECODE_SUCCESSED);

    iRet = adm_ecs_moduleInit(NULL, NULL, 
                              CEcsAPITest::ecs_regDownloadAPI,
                              NULL,NULL);
    EXPECT_NE(iRet, ADM_ECODE_SUCCESSED);

    iRet = adm_ecs_moduleInit(NULL, NULL, NULL, NULL,NULL);
    EXPECT_NE(iRet, ADM_ECODE_SUCCESSED);

    iRet = adm_ecs_moduleInit(CEcsAPITest::ecs_regGetReqCloudParamAPI, 
                              CEcsAPITest::ecs_regReqCloudAPi, 
                              CEcsAPITest::ecs_regDownloadAPI,
                              NULL,NULL);
    EXPECT_EQ(iRet, ADM_ECODE_SUCCESSED);
}

TEST_F(CEcsAPITest, getConfigInfo_Normal)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    Json::Value stPartConfRoot;
    Json::Value stGeneralConfRoot;
    ADM_ECS_RSCONFIGINFO_T stRsConfInfo = {0};

    adm_memzero(&stRsConfInfo, sizeof(stRsConfInfo));
    iRet = adm_ecs_getConfigInfo(&stRsConfInfo);
    EXPECT_EQ(ADM_ECODE_SUCCESSED, iRet);

    //!< 获取模拟数据
    iRet = module_readFile2Json(gpcPartConfFilePath, &stPartConfRoot);
    EXPECT_EQ(ADM_ECODE_SUCCESSED, iRet);
    iRet = module_readFile2Json(gpcGeneralConfFilePath, &stGeneralConfRoot);
    EXPECT_EQ(ADM_ECODE_SUCCESSED, iRet);

    //!< 校验获取的值与模拟的值是否一致
    EXPECT_EQ(stRsConfInfo.stGeneralCfg.logReportCycle, 
                 stGeneralConfRoot["logReportCycle"].asInt());
    EXPECT_STREQ(stRsConfInfo.stGeneralCfg.protocolVersion, 
                 stGeneralConfRoot["protocolVersion"].asString().c_str());

    EXPECT_EQ(1, stRsConfInfo.stPartCfg.stEcuCfgList.num);
    EXPECT_EQ(stRsConfInfo.stPartCfg.stEcuCfgList.pstEcuCfg[0].serviceType, 
                 stPartConfRoot["ecus"][0]["serviceType"].asInt());
    EXPECT_STREQ(stRsConfInfo.stPartCfg.stEcuCfgList.pstEcuCfg[0].ecuReqId, 
                 stPartConfRoot["ecus"][0]["ecuReqId"].asString().c_str());

    adm_ecs_cleanRSConfigInfo(&stRsConfInfo);
    return;
}

TEST_F(CEcsAPITest, updateConfInfo_Normal)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
 
    iRet = adm_ecs_updateAndGetConfigInfo(NULL);
    EXPECT_EQ(ADM_ECODE_SUCCESSED, iRet);

    return;
}

TEST_F(CEcsAPITest, getRsConfigVersion_Normal)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    DCHAR *pcPtr = nullptr;
    Json::Value stCloudRspRoot;

    pcPtr = adm_ecs_getRsConfigVersion();
    EXPECT_NE(nullptr, pcPtr);

    //!< 获取模拟数据
    iRet = module_readFile2Json(gpcConfRspJsonPath, &stCloudRspRoot);
    EXPECT_EQ(ADM_ECODE_SUCCESSED, iRet);

    //!< 校验值
    EXPECT_STREQ(pcPtr, stCloudRspRoot["data"]["confVersion"].asString().c_str());

    adm_free(pcPtr);
    return;
}

TEST_F(CEcsAPITest, getRsCfgGeneralVersion_Normal)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    DCHAR *pcPtr = nullptr;
    Json::Value stRoot;

    pcPtr = adm_ecs_getRsCfgGeneralVersion();
    EXPECT_NE(nullptr, pcPtr);

    //!< 获取模拟数据
    iRet = module_readFile2Json(gpcGeneralConfFilePath, &stRoot);
    EXPECT_EQ(ADM_ECODE_SUCCESSED, iRet);

    //!< 校验值
    EXPECT_STREQ(pcPtr, stRoot["protocolVersion"].asString().c_str());

    adm_free(pcPtr);
    return;
}

TEST_F(CEcsAPITest, getLogReportCycle_Normal)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    D32S iGetValue = ADM_ECODE_SUCCESSED;
    Json::Value stRoot;

    iRet = adm_ecs_getLogReportCycle(&iGetValue);
    EXPECT_EQ(ADM_ECODE_SUCCESSED, iRet);

    //!< 获取模拟数据
    iRet = module_readFile2Json(gpcGeneralConfFilePath, &stRoot);
    EXPECT_EQ(ADM_ECODE_SUCCESSED, iRet);

    //!< 校验值
    EXPECT_EQ(iGetValue, stRoot["logReportCycle"].asInt());

    return;
}

TEST_F(CEcsAPITest, getLogPrintLevel_Normal)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    D32S iGetValue = ADM_ECODE_SUCCESSED;
    Json::Value stRoot;

    iRet = adm_ecs_getLogPrintLevel(&iGetValue);
    EXPECT_EQ(ADM_ECODE_SUCCESSED, iRet);

    //!< 获取模拟数据
    iRet = module_readFile2Json(gpcGeneralConfFilePath, &stRoot);
    EXPECT_EQ(ADM_ECODE_SUCCESSED, iRet);

    //!< 校验值
    EXPECT_EQ(iGetValue, stRoot["logPrintLevel"].asInt());

    return;
}

TEST_F(CEcsAPITest, getIsLogFileEncrypted_Normal)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    D32S iGetValue = ADM_ECODE_SUCCESSED;
    Json::Value stRoot;

    iRet = adm_ecs_getIsLogFileEncrypted(&iGetValue);
    EXPECT_EQ(ADM_ECODE_SUCCESSED, iRet);

    //!< 获取模拟数据
    iRet = module_readFile2Json(gpcGeneralConfFilePath, &stRoot);
    EXPECT_EQ(ADM_ECODE_SUCCESSED, iRet);

    //!< 校验值
    EXPECT_EQ(iGetValue, stRoot["isLogFileEncrypted"].asInt());

    return;
}

TEST_F(CEcsAPITest, getIsReportUpSuccLogFile_Normal)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    D32S iGetValue = ADM_ECODE_SUCCESSED;
    Json::Value stRoot;

    iRet = adm_ecs_getIsReportUpSuccLogFile(&iGetValue);
    EXPECT_EQ(ADM_ECODE_SUCCESSED, iRet);

    //!< 获取模拟数据
    iRet = module_readFile2Json(gpcGeneralConfFilePath, &stRoot);
    EXPECT_EQ(ADM_ECODE_SUCCESSED, iRet);

    //!< 校验值
    EXPECT_EQ(iGetValue, stRoot["isReportUpSuccLogFile"].asInt());

    return;
}

TEST_F(CEcsAPITest, getEventReportCycle_Normal)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    D32S iGetValue = ADM_ECODE_SUCCESSED;
    Json::Value stRoot;

    iRet = adm_ecs_getEventReportCycle(&iGetValue);
    EXPECT_EQ(ADM_ECODE_SUCCESSED, iRet);

    //!< 获取模拟数据
    iRet = module_readFile2Json(gpcGeneralConfFilePath, &stRoot);
    EXPECT_EQ(ADM_ECODE_SUCCESSED, iRet);

    //!< 校验值
    EXPECT_EQ(iGetValue, stRoot["eventReportCycle"].asInt());

    return;
}

TEST_F(CEcsAPITest, getStateReportCycle_Normal)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    D32S iGetValue = ADM_ECODE_SUCCESSED;
    Json::Value stRoot;

    iRet = adm_ecs_getStateReportCycle(&iGetValue);
    EXPECT_EQ(ADM_ECODE_SUCCESSED, iRet);

    //!< 获取模拟数据
    iRet = module_readFile2Json(gpcGeneralConfFilePath, &stRoot);
    EXPECT_EQ(ADM_ECODE_SUCCESSED, iRet);

    //!< 校验值
    EXPECT_EQ(iGetValue, stRoot["stateReportCycle"].asInt());

    return;
}

TEST_F(CEcsAPITest, getSummaryReportCycle_Normal)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    D32S iGetValue = ADM_ECODE_SUCCESSED;
    Json::Value stRoot;

    iRet = adm_ecs_getSummaryReportCycle(&iGetValue);
    EXPECT_EQ(ADM_ECODE_SUCCESSED, iRet);

    //!< 获取模拟数据
    iRet = module_readFile2Json(gpcGeneralConfFilePath, &stRoot);
    EXPECT_EQ(ADM_ECODE_SUCCESSED, iRet);

    //!< 校验值
    EXPECT_EQ(iGetValue, stRoot["summaryReportCycle"].asInt());

    return;
}

TEST_F(CEcsAPITest, getProgressReportCycle_Normal)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    D32S iGetValue = ADM_ECODE_SUCCESSED;
    Json::Value stRoot;

    iRet = adm_ecs_getProgressReportCycle(&iGetValue);
    EXPECT_EQ(ADM_ECODE_SUCCESSED, iRet);

    //!< 获取模拟数据
    iRet = module_readFile2Json(gpcGeneralConfFilePath, &stRoot);
    EXPECT_EQ(ADM_ECODE_SUCCESSED, iRet);

    //!< 校验值
    EXPECT_EQ(iGetValue, stRoot["progressReportCycle"].asInt());

    return;
}

TEST_F(CEcsAPITest, getCheckCyclePeriod_Normal)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    D32S iGetValue = ADM_ECODE_SUCCESSED;
    Json::Value stRoot;

    iRet = adm_ecs_getCheckCyclePeriod(&iGetValue);
    EXPECT_EQ(ADM_ECODE_SUCCESSED, iRet);

    //!< 获取模拟数据
    iRet = module_readFile2Json(gpcGeneralConfFilePath, &stRoot);
    EXPECT_EQ(ADM_ECODE_SUCCESSED, iRet);

    //!< 校验值
    EXPECT_EQ(iGetValue, stRoot["cyclePeriod"].asInt());

    return;
}

TEST_F(CEcsAPITest, getMaxFireTriggerLimit_Normal)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    D32S iGetValue = ADM_ECODE_SUCCESSED;
    Json::Value stRoot;

    iRet = adm_ecs_getMaxFireTriggerLimit(&iGetValue);
    EXPECT_EQ(ADM_ECODE_SUCCESSED, iRet);

    //!< 获取模拟数据
    iRet = module_readFile2Json(gpcGeneralConfFilePath, &stRoot);
    EXPECT_EQ(ADM_ECODE_SUCCESSED, iRet);

    //!< 校验值
    EXPECT_EQ(iGetValue, stRoot["maxFireTrigger"].asInt());

    return;
}

TEST_F(CEcsAPITest, getReadInfoInstanceId_Normal)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    DCHAR *pcPtr = nullptr;
    Json::Value stRoot;

    //!< 获取模拟数据
    iRet = module_readFile2Json(gpcPartConfFilePath, &stRoot);
    EXPECT_EQ(ADM_ECODE_SUCCESSED, iRet);

    //!< 读待测试数据
    pcPtr = adm_ecs_getReadInfoInstanceId((DCHAR *)stRoot["ecus"][0]["ecuReqId"].asString().c_str(), 
                                         (DCHAR *)stRoot["ecus"][0]["ecuSwDid"].asString().c_str());
    EXPECT_NE(nullptr, pcPtr);

    //!< 校验值
    EXPECT_STREQ(pcPtr, (DCHAR *)stRoot["ecus"][0]["readInfoInstanceId"].asString().c_str());

    adm_free(pcPtr);
    return;
}

TEST_F(CEcsAPITest, getDownloadInstanceId_Normal)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    DCHAR *pcPtr = nullptr;
    Json::Value stRoot;

    //!< 获取模拟数据
    iRet = module_readFile2Json(gpcPartConfFilePath, &stRoot);
    EXPECT_EQ(ADM_ECODE_SUCCESSED, iRet);

    //!< 读待测试数据
    pcPtr = adm_ecs_getDownloadInstanceId((DCHAR *)stRoot["ecus"][0]["ecuReqId"].asString().c_str(), 
                                         (DCHAR *)stRoot["ecus"][0]["ecuSwDid"].asString().c_str());
    EXPECT_NE(nullptr, pcPtr);

    //!< 校验值
    EXPECT_STREQ(pcPtr, (DCHAR *)stRoot["ecus"][0]["downloadInstanceId"].asString().c_str());

    adm_free(pcPtr);
    return;
}

TEST_F(CEcsAPITest, getTransferInstanceId_Normal)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    DCHAR *pcPtr = nullptr;
    Json::Value stRoot;

    //!< 获取模拟数据
    iRet = module_readFile2Json(gpcPartConfFilePath, &stRoot);
    EXPECT_EQ(ADM_ECODE_SUCCESSED, iRet);

    //!< 读待测试数据
    pcPtr = adm_ecs_getTransferInstanceId((DCHAR *)stRoot["ecus"][0]["ecuReqId"].asString().c_str(), 
                                         (DCHAR *)stRoot["ecus"][0]["ecuSwDid"].asString().c_str());
    EXPECT_NE(nullptr, pcPtr);

    //!< 校验值
    EXPECT_STREQ(pcPtr, (DCHAR *)stRoot["ecus"][0]["transferInstanceId"].asString().c_str());

    adm_free(pcPtr);
    return;
}

TEST_F(CEcsAPITest, getInstallInstanceId_Normal)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    DCHAR *pcPtr = nullptr;
    Json::Value stRoot;

    //!< 获取模拟数据
    iRet = module_readFile2Json(gpcPartConfFilePath, &stRoot);
    EXPECT_EQ(ADM_ECODE_SUCCESSED, iRet);

    //!< 读待测试数据
    pcPtr = adm_ecs_getInstallInstanceId((DCHAR *)stRoot["ecus"][0]["ecuReqId"].asString().c_str(), 
                                         (DCHAR *)stRoot["ecus"][0]["ecuSwDid"].asString().c_str());
    EXPECT_NE(nullptr, pcPtr);

    //!< 校验值
    EXPECT_STREQ(pcPtr, (DCHAR *)stRoot["ecus"][0]["installInstanceId"].asString().c_str());

    adm_free(pcPtr);
    return;
}

TEST_F(CEcsAPITest, getBusType_Normal)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    D32S iGetValue = ADM_ECODE_SUCCESSED;
    Json::Value stRoot;

    //!< 获取模拟数据
    iRet = module_readFile2Json(gpcPartConfFilePath, &stRoot);
    EXPECT_EQ(ADM_ECODE_SUCCESSED, iRet);

    //!< 读待测试数据
    iRet = adm_ecs_getBusType((DCHAR *)stRoot["ecus"][0]["ecuReqId"].asString().c_str(), 
                              (DCHAR *)stRoot["ecus"][0]["ecuSwDid"].asString().c_str(),
                              &iGetValue);
    EXPECT_EQ(ADM_ECODE_SUCCESSED, iRet);

    //!< 校验值
    EXPECT_EQ(iGetValue, stRoot["ecus"][0]["busType"].asInt());

    return;
}

TEST_F(CEcsAPITest, getChannelType_Normal)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    DCHAR *pcPtr = nullptr;
    D32S iGetValue = ADM_ECODE_SUCCESSED;
    Json::Value stRoot;

    //!< 获取模拟数据
    iRet = module_readFile2Json(gpcPartConfFilePath, &stRoot);
    EXPECT_EQ(ADM_ECODE_SUCCESSED, iRet);

    //!< 读待测试数据
    iRet = adm_ecs_getChannelType((DCHAR *)stRoot["ecus"][0]["ecuReqId"].asString().c_str(), 
                              (DCHAR *)stRoot["ecus"][0]["ecuSwDid"].asString().c_str(),
                              &iGetValue);
    EXPECT_EQ(ADM_ECODE_SUCCESSED, iRet);

    //!< 校验值
    EXPECT_EQ(iGetValue, stRoot["ecus"][0]["channelType"].asInt());

    return;
}

TEST_F(CEcsAPITest, getEcuCrcMask_Normal)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    DCHAR *pcPtr = nullptr;
    D32S iGetValue = ADM_ECODE_SUCCESSED;
    Json::Value stRoot;

    //!< 获取模拟数据
    iRet = module_readFile2Json(gpcPartConfFilePath, &stRoot);
    EXPECT_EQ(ADM_ECODE_SUCCESSED, iRet);

    //!< 读待测试数据
    iRet = adm_ecs_getEcuCrcMask((DCHAR *)stRoot["ecus"][0]["ecuReqId"].asString().c_str(), 
                              (DCHAR *)stRoot["ecus"][0]["ecuSwDid"].asString().c_str(),
                              &iGetValue);
    EXPECT_EQ(ADM_ECODE_SUCCESSED, iRet);

    //!< 校验值
    EXPECT_EQ(iGetValue, stRoot["ecus"][0]["crcMask"].asInt());

    return;
}

TEST_F(CEcsAPITest, getEcuCrcLibNumber_Normal)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    DCHAR *pcPtr = nullptr;
    D32S iGetValue = ADM_ECODE_SUCCESSED;
    Json::Value stRoot;

    //!< 获取模拟数据
    iRet = module_readFile2Json(gpcPartConfFilePath, &stRoot);
    EXPECT_EQ(ADM_ECODE_SUCCESSED, iRet);

    //!< 读待测试数据
    iRet = adm_ecs_getEcuCrcLibNumber((DCHAR *)stRoot["ecus"][0]["ecuReqId"].asString().c_str(), 
                              (DCHAR *)stRoot["ecus"][0]["ecuSwDid"].asString().c_str(),
                              &iGetValue);
    EXPECT_EQ(ADM_ECODE_SUCCESSED, iRet);

    //!< 校验值
    EXPECT_EQ(iGetValue, stRoot["ecus"][0]["crcLibNumber"].asInt());

    return;
}

TEST_F(CEcsAPITest, getEcuSaMask_Normal)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    DCHAR *pcPtr = nullptr;
    D32S iGetValue = ADM_ECODE_SUCCESSED;
    Json::Value stRoot;

    //!< 获取模拟数据
    iRet = module_readFile2Json(gpcPartConfFilePath, &stRoot);
    EXPECT_EQ(ADM_ECODE_SUCCESSED, iRet);

    //!< 读待测试数据
    iRet = adm_ecs_getEcuSaMask((DCHAR *)stRoot["ecus"][0]["ecuReqId"].asString().c_str(), 
                              (DCHAR *)stRoot["ecus"][0]["ecuSwDid"].asString().c_str(),
                              &iGetValue);
    EXPECT_EQ(ADM_ECODE_SUCCESSED, iRet);

    //!< 校验值
    EXPECT_EQ(iGetValue, stRoot["ecus"][0]["saMask"].asInt());

    return;
}

TEST_F(CEcsAPITest, getEcuSaLibNumberList_Normal)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    DCHAR *pcPtr = nullptr;
    ADM_ECS_SALIBNUMBERLIST_T stsaLibNumberList = {0};
    Json::Value stRoot;

    //!< 获取模拟数据
    iRet = module_readFile2Json(gpcPartConfFilePath, &stRoot);
    EXPECT_EQ(ADM_ECODE_SUCCESSED, iRet);

    //!< 读待测试数据
    adm_memzero(&stsaLibNumberList, sizeof(stsaLibNumberList));
    iRet = adm_ecs_getEcuSaLibNumberList((DCHAR *)stRoot["ecus"][0]["ecuReqId"].asString().c_str(), 
                              (DCHAR *)stRoot["ecus"][0]["ecuSwDid"].asString().c_str(),
                              &stsaLibNumberList);
    EXPECT_EQ(ADM_ECODE_SUCCESSED, iRet);

    //!< 校验值
    EXPECT_EQ(stsaLibNumberList.num, stRoot["ecus"][0]["saLibNumbers"].size());

    return;
}

TEST_F(CEcsAPITest, getEcuFlashScriptNumber_Normal)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    D32S iGetValue = ADM_ECODE_SUCCESSED;
    Json::Value stRoot;

    //!< 获取模拟数据
    iRet = module_readFile2Json(gpcPartConfFilePath, &stRoot);
    EXPECT_EQ(ADM_ECODE_SUCCESSED, iRet);

    //!< 读待测试数据
    iRet = adm_ecs_getEcuFlashScriptNumber((DCHAR *)stRoot["ecus"][0]["ecuReqId"].asString().c_str(), 
                              (DCHAR *)stRoot["ecus"][0]["ecuSwDid"].asString().c_str(),
                              &iGetValue);
    EXPECT_EQ(ADM_ECODE_SUCCESSED, iRet);

    //!< 校验值
    EXPECT_EQ(iGetValue, stRoot["ecus"][0]["flashScriptNumber"].asInt());

    return;
}

TEST_F(CEcsAPITest, getEcuIsCGWRouted_Normal)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    D32S iGetValue = ADM_ECODE_SUCCESSED;
    Json::Value stRoot;

    //!< 获取模拟数据
    iRet = module_readFile2Json(gpcPartConfFilePath, &stRoot);
    EXPECT_EQ(ADM_ECODE_SUCCESSED, iRet);

    //!< 读待测试数据
    iRet = adm_ecs_getEcuIsCGWRouted((DCHAR *)stRoot["ecus"][0]["ecuReqId"].asString().c_str(), 
                              (DCHAR *)stRoot["ecus"][0]["ecuSwDid"].asString().c_str(),
                              &iGetValue);
    EXPECT_EQ(ADM_ECODE_SUCCESSED, iRet);

    //!< 校验值
    EXPECT_EQ(iGetValue, stRoot["ecus"][0]["isCGWRouted"].asInt());

    return;
}

TEST_F(CEcsAPITest, getEcuServiceType_Normal)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    D32S iGetValue = ADM_ECODE_SUCCESSED;
    Json::Value stRoot;

    //!< 获取模拟数据
    iRet = module_readFile2Json(gpcPartConfFilePath, &stRoot);
    EXPECT_EQ(ADM_ECODE_SUCCESSED, iRet);

    //!< 读待测试数据
    iRet = adm_ecs_getEcuServiceType((DCHAR *)stRoot["ecus"][0]["ecuReqId"].asString().c_str(), 
                              (DCHAR *)stRoot["ecus"][0]["ecuSwDid"].asString().c_str(),
                              &iGetValue);
    EXPECT_EQ(ADM_ECODE_SUCCESSED, iRet);

    //!< 校验值
    EXPECT_EQ(iGetValue, stRoot["ecus"][0]["serviceType"].asInt());

    return;
}

TEST_F(CEcsAPITest, getEcuInstCondition_Normal)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    D32S iGetValue = ADM_ECODE_SUCCESSED;
    Json::Value stRoot;

    //!< 获取模拟数据
    iRet = module_readFile2Json(gpcPartConfFilePath, &stRoot);
    EXPECT_EQ(ADM_ECODE_SUCCESSED, iRet);

    //!< 读待测试数据
    iRet = adm_ecs_getEcuInstCondition((DCHAR *)stRoot["ecus"][0]["ecuReqId"].asString().c_str(), 
                              (DCHAR *)stRoot["ecus"][0]["ecuSwDid"].asString().c_str(),
                              &iGetValue);
    EXPECT_EQ(ADM_ECODE_SUCCESSED, iRet);

    //!< 校验值
    EXPECT_EQ(iGetValue, stRoot["ecus"][0]["instCondition"].asInt());

    return;
}

TEST_F(CEcsAPITest, getEcuSecurityLevel_Normal)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    D32S iGetValue = ADM_ECODE_SUCCESSED;
    Json::Value stRoot;

    //!< 获取模拟数据
    iRet = module_readFile2Json(gpcPartConfFilePath, &stRoot);
    EXPECT_EQ(ADM_ECODE_SUCCESSED, iRet);

    //!< 读待测试数据
    iRet = adm_ecs_getEcuSecurityLevel((DCHAR *)stRoot["ecus"][0]["ecuReqId"].asString().c_str(), 
                              (DCHAR *)stRoot["ecus"][0]["ecuSwDid"].asString().c_str(),
                              &iGetValue);
    EXPECT_EQ(ADM_ECODE_SUCCESSED, iRet);

    //!< 校验值
    EXPECT_EQ(iGetValue, stRoot["ecus"][0]["securityLevel"].asInt());

    return;
}

TEST_F(CEcsAPITest, getEcuConfig_Normal)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    ADM_ECS_ECUCONFIG_T *pstEcuConfig = nullptr;
    Json::Value stRoot;

    //!< 获取模拟数据
    iRet = module_readFile2Json(gpcPartConfFilePath, &stRoot);
    EXPECT_EQ(ADM_ECODE_SUCCESSED, iRet);

    //!< 读待测试数据
    iRet = adm_ecs_getEcuConfig((DCHAR *)stRoot["ecus"][0]["ecuReqId"].asString().c_str(), 
                              (DCHAR *)stRoot["ecus"][0]["ecuSwDid"].asString().c_str(),
                              &pstEcuConfig);
    EXPECT_EQ(ADM_ECODE_SUCCESSED, iRet);
    EXPECT_NE(nullptr, pstEcuConfig);

    //!< 校验值
    EXPECT_STREQ(pstEcuConfig->ecuReqId, (DCHAR *)stRoot["ecus"][0]["ecuReqId"].asString().c_str());
    EXPECT_STREQ(pstEcuConfig->ecuSwDid, (DCHAR *)stRoot["ecus"][0]["ecuSwDid"].asString().c_str());
    EXPECT_STREQ(pstEcuConfig->ecuResponseId, (DCHAR *)stRoot["ecus"][0]["ecuResponseId"].asString().c_str());
    EXPECT_STREQ(pstEcuConfig->ecuFunctionId, (DCHAR *)stRoot["ecus"][0]["ecuFunctionId"].asString().c_str());

    EXPECT_EQ(pstEcuConfig->saLibNumberList.num, stRoot["ecus"][0]["saLibNumbers"].size());
    EXPECT_EQ(pstEcuConfig->stEcuDidList.num, stRoot["ecus"][0]["ecuDids"].size());
    EXPECT_EQ(pstEcuConfig->stEcuDidList.num, 1);
    EXPECT_STREQ(pstEcuConfig->stEcuDidList.pstEcuDidInfo[0].did, (DCHAR *)stRoot["ecus"][0]["ecuDids"][0]["did"].asString().c_str());
    EXPECT_EQ(pstEcuConfig->stEcuDidList.pstEcuDidInfo[0].start, stRoot["ecus"][0]["ecuDids"][0]["start"].asInt());
    EXPECT_EQ(pstEcuConfig->stEcuDidList.pstEcuDidInfo[0].end,   stRoot["ecus"][0]["ecuDids"][0]["end"].asInt());

    EXPECT_EQ(pstEcuConfig->stExtList.num, stRoot["ecus"][0]["exts"].size());
    EXPECT_EQ(pstEcuConfig->stExtList.num, 1);
    EXPECT_STREQ(pstEcuConfig->stExtList.pstExtParamInfo[0].extKey, (DCHAR *)stRoot["ecus"][0]["exts"][0]["extKey"].asString().c_str());
    EXPECT_STREQ(pstEcuConfig->stExtList.pstExtParamInfo[0].extValue, (DCHAR *)stRoot["ecus"][0]["exts"][0]["extValue"].asString().c_str());

    adm_ecs_delEcuConfigInfo(&pstEcuConfig);
    return;
}


TEST_F(CEcsAPITest, getGeneralEcuExtsInfo_Normal)
{
    D32S i = 0;
    D32S iRet = ADM_ECODE_SUCCESSED;
    ADM_COM_EXTPARAMLIST_T stExtsList = {0};
    Json::Value stRoot;

    //!< 获取模拟数据
    iRet = module_readFile2Json(gpcGeneralConfFilePath, &stRoot);
    EXPECT_EQ(ADM_ECODE_SUCCESSED, iRet);

    //!< 读待测试API
    adm_memzero(&stExtsList, sizeof(stExtsList));
    iRet = adm_ecs_getGeneralEcuExtsInfo(&stExtsList);
    EXPECT_EQ(ADM_ECODE_SUCCESSED, iRet);

    //!< 校验值
    EXPECT_EQ(stExtsList.num, stRoot["exts"].size());
    EXPECT_NE(nullptr, stExtsList.pstExtParamInfo);
    for (i = 0; i < stExtsList.num; i++)
    {
        EXPECT_STREQ(stExtsList.pstExtParamInfo[i].extKey, (DCHAR *)stRoot["exts"][i]["extKey"].asString().c_str());
        EXPECT_STREQ(stExtsList.pstExtParamInfo[i].extValue, (DCHAR *)stRoot["exts"][i]["extValue"].asString().c_str());
    }

    adm_com_cleanExtsList(&stExtsList);
    return;
}

TEST_F(CEcsAPITest, getPartCfgExternalExtsInfo_Normal)
{
    D32S i = 0;
    D32S iRet = ADM_ECODE_SUCCESSED;
    ADM_COM_EXTPARAMLIST_T stExtsList = {0};
    Json::Value stRoot;

    //!< 获取模拟数据
    iRet = module_readFile2Json(gpcPartConfFilePath, &stRoot);
    EXPECT_EQ(ADM_ECODE_SUCCESSED, iRet);

    //!< 读待测试API
    adm_memzero(&stExtsList, sizeof(stExtsList));
    iRet = adm_ecs_getPartCfgExternalExtsInfo(&stExtsList);
    EXPECT_EQ(ADM_ECODE_SUCCESSED, iRet);

    //!< 校验值
    EXPECT_EQ(stExtsList.num, stRoot["exts"].size());
    EXPECT_NE(nullptr, stExtsList.pstExtParamInfo);
    for (i = 0; i < stExtsList.num; i++)
    {
        EXPECT_STREQ(stExtsList.pstExtParamInfo[i].extKey, (DCHAR *)stRoot["exts"][i]["extKey"].asString().c_str());
        EXPECT_STREQ(stExtsList.pstExtParamInfo[i].extValue, (DCHAR *)stRoot["exts"][i]["extValue"].asString().c_str());
    }

    adm_com_cleanExtsList(&stExtsList);
    return;
}


TEST_F(CEcsAPITest, getPartCfgInternalExtsInfo_Normal)
{
    D32S i = 0;
    D32S iRet = ADM_ECODE_SUCCESSED;
    ADM_COM_EXTPARAMLIST_T stExtsList = {0};
    Json::Value stRoot;

    //!< 获取模拟数据
    iRet = module_readFile2Json(gpcPartConfFilePath, &stRoot);
    EXPECT_EQ(ADM_ECODE_SUCCESSED, iRet);

    //!< 读待测试API
    adm_memzero(&stExtsList, sizeof(stExtsList));
    iRet = adm_ecs_getPartCfgInternalExtsInfo((DCHAR *)stRoot["ecus"][0]["ecuReqId"].asString().c_str(), 
                                              (DCHAR *)stRoot["ecus"][0]["ecuSwDid"].asString().c_str(),
                                              &stExtsList);
    EXPECT_EQ(ADM_ECODE_SUCCESSED, iRet);

    //!< 校验值
    EXPECT_EQ(stExtsList.num, stRoot["ecus"][0]["exts"].size());
    EXPECT_NE(nullptr, stExtsList.pstExtParamInfo);
    for (i = 0; i < stExtsList.num; i++)
    {
        EXPECT_STREQ(stExtsList.pstExtParamInfo[i].extKey, (DCHAR *)stRoot["ecus"][0]["exts"][i]["extKey"].asString().c_str());
        EXPECT_STREQ(stExtsList.pstExtParamInfo[i].extValue, (DCHAR *)stRoot["ecus"][0]["exts"][i]["extValue"].asString().c_str());
    }

    adm_com_cleanExtsList(&stExtsList);
    return;
}

// ############## 5.0 平台新增内容 字段
TEST_F(CEcsAPITest, getPollingCycle_Normal)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    D32S iGetValue = ADM_ECODE_SUCCESSED;
    Json::Value stRoot;

    //!< 读待测试API
    iRet = adm_ecs_getPollingCycle(&iGetValue);
    EXPECT_EQ(ADM_ECODE_SUCCESSED, iRet);

    //!< 获取模拟数据
    iRet = module_readFile2Json(gpcGeneralConfFilePath, &stRoot);
    EXPECT_EQ(ADM_ECODE_SUCCESSED, iRet);

    //!< 校验值
    EXPECT_EQ(iGetValue, stRoot["pollingCycle"].asInt());

    return;
}

TEST_F(CEcsAPITest, getLogReportUrl_Normal)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    DCHAR *pcReportUrl = NULL;
    Json::Value stRoot;

    //!< 获取模拟数据
    iRet = module_readFile2Json(gpcGeneralConfFilePath, &stRoot);
    EXPECT_EQ(ADM_ECODE_SUCCESSED, iRet);

    //!< 读待测试API
    pcReportUrl = adm_ecs_getLogReportUrl();
    EXPECT_NE(nullptr, pcReportUrl);
    cout << "[ RUN      ] pcReportUrl[" << pcReportUrl  << endl;
    
    //!< 校验值
    EXPECT_STREQ(pcReportUrl, (DCHAR *)stRoot["logReportUrl"].asString().c_str());

    return;
}

// #############################

TEST_F(CEcsAPITest, getEcuIsSelfUpdate_Normal)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    D32S iGetValue = ADM_ECODE_SUCCESSED;
    Json::Value stRoot;

    //!< 获取模拟数据
    iRet = module_readFile2Json(gpcPartConfFilePath, &stRoot);
    EXPECT_EQ(ADM_ECODE_SUCCESSED, iRet);

    //!< 读待测试数据
    iRet = adm_ecs_getEcuIsSelfUpdate((DCHAR *)stRoot["ecus"][0]["ecuReqId"].asString().c_str(), 
                                         (DCHAR *)stRoot["ecus"][0]["ecuSwDid"].asString().c_str(),
                                         &iGetValue);
    EXPECT_EQ(ADM_ECODE_SUCCESSED, iRet);

    //!< 校验值
    EXPECT_EQ(iGetValue, stRoot["ecus"][0]["isSelfUpdate"].asInt());

    return;
}

TEST_F(CEcsAPITest, getEcuSelfBackUp_Normal)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    D32S iGetValue = ADM_ECODE_SUCCESSED;
    Json::Value stRoot;

    //!< 获取模拟数据
    iRet = module_readFile2Json(gpcPartConfFilePath, &stRoot);
    EXPECT_EQ(ADM_ECODE_SUCCESSED, iRet);

    //!< 读待测试数据
    iRet = adm_ecs_getEcuSelfBackUp((DCHAR *)stRoot["ecus"][0]["ecuReqId"].asString().c_str(), 
                                         (DCHAR *)stRoot["ecus"][0]["ecuSwDid"].asString().c_str(),
                                         &iGetValue);
    EXPECT_EQ(ADM_ECODE_SUCCESSED, iRet);

    //!< 校验值
    EXPECT_EQ(iGetValue, stRoot["ecus"][0]["selfBackup"].asInt());

    return;
}
