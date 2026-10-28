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
#include "adm_engine_common.h"
#include "adm_dps_unittest.h"
#include "adm_dps_parsePolicy.h"
#include "adm_dps_parsePackageConfig.h"
#include "adm_pis_package.h"
#include "json/json.h"
#include "adm_string.h"
#include "adm_memory.h"


#include <fstream>
#include <iostream>

using namespace std;

/*!
 *******************************************************************************
 * \note 依赖外部模拟数据清单.
 *      ./data/getDpsCheckPolicy.json          --- 构造检测响应策略(集合包)
 *      ./data/getDpsDownEcuPolicy.json        --- 构造检测响应策略(零件包)
 *      ./data/getDpsCombinePackageConfig.json --- 构造集合包中的配置文件
 * 
 *******************************************************************************
 */
#define CORESERVICE_RUNDIR                     "./data/runDir/"
static const DCHAR *gpcCheckPolicyJsonPath   = "./data/getDpsCheckPolicy.json";   //!< 检测结果(下载策略中是集合包)
static const DCHAR *gpcDownEcuPolicyJsonPath = "./data/getDpsDownEcuPolicy.json"; //!< 下载策略中是零件包
static const DCHAR *gpcCombinePackageConfJsonPath = "./data/getDpsCombinePackageConfig.json";   //!< 集合包配置文件


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

void CDpsAPITest::SetUpTestCase()
{
    D32S iRet = ADM_ECODE_SUCCESSED;

    iRet = CDpsAPITest::createCoreServiceRunDir();
    EXPECT_EQ(ADM_ECODE_SUCCESSED, iRet);
    
    iRet = adm_dps_moduleInit(NULL);
    EXPECT_EQ(ADM_ECODE_SUCCESSED, iRet);

    adm_pis_packageInit();

    iRet = adm_dps_checkAndUpdateDPSDownInfo(CDpsAPITest::dps_ctorCheckDownloadPolicy,CDpsAPITest::dps_ctorCleanSlaveDownPackage);
    EXPECT_EQ(ADM_ECODE_SUCCESSED, iRet);

    // cout << "SetUpTestCase" << endl;
}

void CDpsAPITest::TearDownTestCase()
{
    adm_dps_moduleDeInit();
    CDpsAPITest::destroyCoreServiceRunDir();
    adm_pis_packageDeInit();

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
D32S CDpsAPITest::createCoreServiceRunDir(void)
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
void CDpsAPITest::destroyCoreServiceRunDir(void)
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
 * 构造检测结果中下载相关策略信息.
 *
 * \note 
 *
 * \param[inout] pstVersionPolicy  检测结果策略信息
 * \retval 	     D32S
 *******************************************************************************
 */
D32S CDpsAPITest::dps_ctorCheckDownloadPolicy(ADM_CVS_VERSION_POLICY_T *pstVersionPolicy)
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
    // cout << "[----------] ===> downPolicy[" << policyRoot["downPolicy"].toStyledString() << "]" << endl;
    pstVersionPolicy->downloadMember.downloadPolicyObjectStr = adm_strdup(policyRoot["downPolicy"].toStyledString().c_str());

    return ADM_ECODE_SUCCESSED;
}

/*!
 *******************************************************************************
 * 构造检测结果中下载相关策略信息.
 *
 * \note 
 *
 * \param[inout] pstVersionPolicy  检测结果策略信息
 * \retval 	     void
 *******************************************************************************
 */
void CDpsAPITest::dps_ctorCleanSlaveDownPackage()
{
    return; 
}

/*!
 *******************************************************************************
 * 构造新的检测结果中下载相关策略信息.
 *
 * \note 构造 checkAndUpdateDownInfo_Normal 测试用例
 *
 * \param[inout] pstVersionPolicy  检测结果策略信息
 * \retval 	     D32S
 *******************************************************************************
 */
D32S CDpsAPITest::dps_ctorCheckNewDownloadPolicy(ADM_CVS_VERSION_POLICY_T *pstVersionPolicy)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    Json::Value policyRoot;

    EXPECT_NE(nullptr, pstVersionPolicy);

    iRet = module_readFile2Json(gpcDownEcuPolicyJsonPath, &policyRoot);
    if (ADM_IS_ERR(iRet))
    {
        cout << "failed to read file[" << gpcDownEcuPolicyJsonPath << "] to json, err[" << iRet << "]"<< endl;
        return iRet;
    }

    pstVersionPolicy->taskId     = policyRoot["taskId"].asInt64();
    pstVersionPolicy->strategyId = policyRoot["strategyId"].asInt64();
    pstVersionPolicy->sessionId  = adm_strdup(policyRoot["sessionId"].asString().c_str());
    // cout << "[----------] ===> downPolicy[" << policyRoot["downPolicy"].toStyledString() << "]" << endl;
    pstVersionPolicy->downloadMember.downloadPolicyObjectStr = adm_strdup(policyRoot["downPolicy"].toStyledString().c_str());

    return ADM_ECODE_SUCCESSED;
}

/*!
 *******************************************************************************
 * 构造检测结果中获取集合包文件描述信息.
 *
 * \note 
 *
 * \param[inout] pstPkgFileObj  集合包文件描述信息
 * \retval 	     D32S
 *******************************************************************************
 */
D32S CDpsAPITest::dps_ctorGetCombinePkgFileObj(ADM_COM_FILEOBJECT_T *pstPkgFileObj)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    Json::Value policyRoot;

    EXPECT_NE(nullptr, pstPkgFileObj);

    iRet = module_readFile2Json(gpcCheckPolicyJsonPath, &policyRoot);
    if (ADM_IS_ERR(iRet))
    {
        cout << "failed to read file[" << gpcCheckPolicyJsonPath << "] to json, err[" << iRet << "]"<< endl;
        return iRet;
    }

    pstPkgFileObj->size     = policyRoot["downPolicy"]["downPkgInfos"][0]["pkgFile"]["size"].asInt64();
    pstPkgFileObj->url      = 
        adm_strdup(policyRoot["downPolicy"]["downPkgInfos"][0]["pkgFile"]["url"].asString().c_str());
    pstPkgFileObj->hash     = 
        adm_strdup(policyRoot["downPolicy"]["downPkgInfos"][0]["pkgFile"]["hash"].asString().c_str());
    pstPkgFileObj->fileName = 
        adm_strdup(policyRoot["downPolicy"]["downPkgInfos"][0]["pkgFile"]["fileName"].asString().c_str());

    return ADM_ECODE_SUCCESSED;
}

/*!
 *******************************************************************************
 * 构造更新dps中集合包的配置信息.
 *
 * \note 
 *
 * \param   none  
 * \retval 	D32S
 *******************************************************************************
 */
D32S CDpsAPITest::dps_ctorUpdateDownCombinePackageCfgInfo(void)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    ADM_COM_FILEOBJECT_T stPkgFileObj    = {0};
    ADM_DPS_PACKAGECFG_T stSrcPkgCfgInfo = {0};

    //!< 获取实际解析到的构造数据
    adm_memzero(&stSrcPkgCfgInfo, sizeof(stSrcPkgCfgInfo));
    iRet = adm_dps_parsePackageConfigFile((DCHAR *)gpcCombinePackageConfJsonPath, &stSrcPkgCfgInfo);
    EXPECT_EQ(ADM_ECODE_SUCCESSED, iRet);

    //!< 获取对应的集合包
    adm_memzero(&stPkgFileObj, sizeof(stPkgFileObj));
    iRet = CDpsAPITest::dps_ctorGetCombinePkgFileObj(&stPkgFileObj);
    EXPECT_EQ(ADM_ECODE_SUCCESSED, iRet);

    //!< 更新pkgConfInfo
    iRet = adm_dps_updateDownCombinePackageCfgInfo(&stPkgFileObj, &stSrcPkgCfgInfo);
    EXPECT_EQ(ADM_ECODE_SUCCESSED, iRet);

    adm_com_cleanFileObject(&stPkgFileObj);
    adm_dps_cleanPackageCfgInfo(&stSrcPkgCfgInfo);
    return ADM_ECODE_SUCCESSED;
}


#if 0
#endif


TEST_F(CDpsAPITest, moduleInit_Normal)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    
    iRet = adm_dps_moduleInit(NULL);
    EXPECT_EQ(ADM_ECODE_SUCCESSED, iRet);

    return;
}

//!< 测试解析策略中集合包及下载前置条件规则
TEST_F(CDpsAPITest, parseCombinePolicy_Normal)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    Json::Value policyRoot;
    ADM_DPS_DOWNLOADINFO_T stDownloadInfo = {0};

    iRet = module_readFile2Json(gpcCheckPolicyJsonPath, &policyRoot);
    if (ADM_IS_ERR(iRet))
    {
        cout << "failed to read file[" << gpcCheckPolicyJsonPath << "] to json, err[" << iRet << "]"<< endl;
        return;
    }

    //!< 解析策略
    adm_memzero(&stDownloadInfo, sizeof(stDownloadInfo));
    // cout << "[----------] ===> downPolicy[" << policyRoot["downPolicy"].toStyledString() << "]" << endl;
    iRet = adm_dps_parseDownloadPolicy((DCHAR *)policyRoot["downPolicy"].toStyledString().c_str(), &stDownloadInfo);
    EXPECT_EQ(ADM_ECODE_SUCCESSED, iRet);

    //!< 1: 集合包, 2: 零件包 --- 校验集合包
    EXPECT_EQ(ADM_DPS_DOWNPACKAGE_COMBINE, stDownloadInfo.stDownECUInfo.eDownPkgFormat);
    EXPECT_EQ(1, stDownloadInfo.stDownECUInfo.ecuPkgListNum);
    //!< pkgFile
    EXPECT_EQ(stDownloadInfo.stDownECUInfo.unPkgDataList.pstEcuCombinePkg[0].stPkgFileObj.size, 
                    policyRoot["downPolicy"]["downPkgInfos"][0]["pkgFile"]["size"].asInt64());
    EXPECT_STREQ(stDownloadInfo.stDownECUInfo.unPkgDataList.pstEcuCombinePkg[0].stPkgFileObj.fileName, 
                    policyRoot["downPolicy"]["downPkgInfos"][0]["pkgFile"]["fileName"].asString().c_str());
    EXPECT_STREQ(stDownloadInfo.stDownECUInfo.unPkgDataList.pstEcuCombinePkg[0].stPkgFileObj.url, 
                    policyRoot["downPolicy"]["downPkgInfos"][0]["pkgFile"]["url"].asString().c_str());
    EXPECT_STREQ(stDownloadInfo.stDownECUInfo.unPkgDataList.pstEcuCombinePkg[0].stPkgFileObj.hash, 
                    policyRoot["downPolicy"]["downPkgInfos"][0]["pkgFile"]["hash"].asString().c_str());

    //!< ecuInfos
    EXPECT_EQ(1, stDownloadInfo.stDownECUInfo.unPkgDataList.pstEcuCombinePkg[0].stEcuIdList.num);
    EXPECT_STREQ(stDownloadInfo.stDownECUInfo.unPkgDataList.pstEcuCombinePkg[0].stEcuIdList.pstEcuIdInfo[0].ecuReqid, 
                    policyRoot["downPolicy"]["downPkgInfos"][0]["ecuInfos"][0]["ecuReqId"].asString().c_str());
    EXPECT_STREQ(stDownloadInfo.stDownECUInfo.unPkgDataList.pstEcuCombinePkg[0].stEcuIdList.pstEcuIdInfo[0].ecuSwDid, 
                    policyRoot["downPolicy"]["downPkgInfos"][0]["ecuInfos"][0]["ecuSwDid"].asString().c_str());
    //TODO:ADM_DPS_ECUIDINFO_T结构变更,影响test，后续完善
    //if (policyRoot["downPolicy"]["downPkgInfos"][0]["ecuInfos"][0]["srcPkgName"].isString())
    //{
    //    EXPECT_STREQ(stDownloadInfo.stDownECUInfo.unPkgDataList.pstEcuCombinePkg[0].stEcuIdList.pstEcuIdInfo[0].srcPkgName, 
    //                policyRoot["downPolicy"]["downPkgInfos"][0]["ecuInfos"][0]["srcPkgName"].asString().c_str());
    //}
    //if (policyRoot["downPolicy"]["downPkgInfos"][0]["ecuInfos"][0]["dstPkgName"].isString())
    //{
    //    EXPECT_STREQ(stDownloadInfo.stDownECUInfo.unPkgDataList.pstEcuCombinePkg[0].stEcuIdList.pstEcuIdInfo[0].dstPkgName, 
    //                policyRoot["downPolicy"]["downPkgInfos"][0]["ecuInfos"][0]["dstPkgName"].asString().c_str());
    //}

    //!< 下载前置条件规则校验
    EXPECT_EQ(3, stDownloadInfo.stDownCondition.trigNum);
    EXPECT_EQ(1, stDownloadInfo.stDownCondition.pstTrigDownCondList[0].iType);
    EXPECT_EQ(8, stDownloadInfo.stDownCondition.pstTrigDownCondList[0].iCondNum);
    EXPECT_EQ(2, stDownloadInfo.stDownCondition.pstTrigDownCondList[1].iType);
    EXPECT_EQ(5, stDownloadInfo.stDownCondition.pstTrigDownCondList[2].iType);

    return;
}

//!< 测试解析策略中零件包
TEST_F(CDpsAPITest, parseDownEcuPolicy_Normal)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    Json::Value policyRoot;
    ADM_DPS_DOWNLOADINFO_T stDownloadInfo = {0};

    iRet = module_readFile2Json(gpcDownEcuPolicyJsonPath, &policyRoot);
    if (ADM_IS_ERR(iRet))
    {
        cout << "failed to read file[" << gpcDownEcuPolicyJsonPath << "] to json, err[" << iRet << "]"<< endl;
        return;
    }

    //!< 解析策略
    adm_memzero(&stDownloadInfo, sizeof(stDownloadInfo));
    // cout << "[----------] ===> downPolicy[" << policyRoot["downPolicy"].toStyledString() << "]" << endl;
    iRet = adm_dps_parseDownloadPolicy((DCHAR *)policyRoot["downPolicy"].toStyledString().c_str(), &stDownloadInfo);
    EXPECT_EQ(ADM_ECODE_SUCCESSED, iRet);

    //!< 1: 集合包, 2: 零件包 --- 校验集合包
    EXPECT_EQ(ADM_DPS_DOWNPACKAGE_PART, stDownloadInfo.stDownECUInfo.eDownPkgFormat);
    EXPECT_EQ(2, stDownloadInfo.stDownECUInfo.ecuPkgListNum);
    //!< array[0] src and dst
    EXPECT_STREQ(stDownloadInfo.stDownECUInfo.unPkgDataList.pstEcuPartPkg[0].ecuReqid, 
                    policyRoot["downPolicy"]["downEcuInfos"][0]["ecuReqId"].asString().c_str());
    EXPECT_STREQ(stDownloadInfo.stDownECUInfo.unPkgDataList.pstEcuPartPkg[0].ecuSwDid, 
                    policyRoot["downPolicy"]["downEcuInfos"][0]["ecuSwDid"].asString().c_str());
    //!< array[0] src and dst --- src
    EXPECT_NE(nullptr, stDownloadInfo.stDownECUInfo.unPkgDataList.pstEcuPartPkg[0].pstSrcEcuPkgInfo);
    EXPECT_EQ(stDownloadInfo.stDownECUInfo.unPkgDataList.pstEcuPartPkg[0].pstSrcEcuPkgInfo->ePkgType, 
                    policyRoot["downPolicy"]["downEcuInfos"][0]["srcPkg"]["pkgType"].asInt());
    EXPECT_NE(nullptr, stDownloadInfo.stDownECUInfo.unPkgDataList.pstEcuPartPkg[0].pstSrcEcuPkgInfo->srcVer);
    EXPECT_STREQ(stDownloadInfo.stDownECUInfo.unPkgDataList.pstEcuPartPkg[0].pstSrcEcuPkgInfo->srcVer, 
                    policyRoot["downPolicy"]["downEcuInfos"][0]["srcPkg"]["srcVer"].asString().c_str());
    EXPECT_NE(nullptr, stDownloadInfo.stDownECUInfo.unPkgDataList.pstEcuPartPkg[0].pstSrcEcuPkgInfo->dstVer);
    EXPECT_STREQ(stDownloadInfo.stDownECUInfo.unPkgDataList.pstEcuPartPkg[0].pstSrcEcuPkgInfo->dstVer, 
                    policyRoot["downPolicy"]["downEcuInfos"][0]["srcPkg"]["dstVer"].asString().c_str());
    EXPECT_NE(nullptr, stDownloadInfo.stDownECUInfo.unPkgDataList.pstEcuPartPkg[0].pstSrcEcuPkgInfo->stPkgFileObj.fileName);
    EXPECT_STREQ(stDownloadInfo.stDownECUInfo.unPkgDataList.pstEcuPartPkg[0].pstSrcEcuPkgInfo->stPkgFileObj.fileName, 
                    policyRoot["downPolicy"]["downEcuInfos"][0]["srcPkg"]["pkgFile"]["fileName"].asString().c_str());
    //!< array[0] src and dst --- dst
    EXPECT_EQ(stDownloadInfo.stDownECUInfo.unPkgDataList.pstEcuPartPkg[0].stDstEcuPkgInfo.ePkgType, 
                    policyRoot["downPolicy"]["downEcuInfos"][0]["dstPkg"]["pkgType"].asInt());
    EXPECT_NE(nullptr, stDownloadInfo.stDownECUInfo.unPkgDataList.pstEcuPartPkg[0].stDstEcuPkgInfo.srcVer);
    EXPECT_STREQ(stDownloadInfo.stDownECUInfo.unPkgDataList.pstEcuPartPkg[0].stDstEcuPkgInfo.srcVer, 
                    policyRoot["downPolicy"]["downEcuInfos"][0]["dstPkg"]["srcVer"].asString().c_str());
    EXPECT_NE(nullptr, stDownloadInfo.stDownECUInfo.unPkgDataList.pstEcuPartPkg[0].stDstEcuPkgInfo.dstVer);
    EXPECT_STREQ(stDownloadInfo.stDownECUInfo.unPkgDataList.pstEcuPartPkg[0].stDstEcuPkgInfo.dstVer, 
                    policyRoot["downPolicy"]["downEcuInfos"][0]["dstPkg"]["dstVer"].asString().c_str());
    EXPECT_NE(nullptr, stDownloadInfo.stDownECUInfo.unPkgDataList.pstEcuPartPkg[0].stDstEcuPkgInfo.stPkgFileObj.fileName);
    EXPECT_STREQ(stDownloadInfo.stDownECUInfo.unPkgDataList.pstEcuPartPkg[0].stDstEcuPkgInfo.stPkgFileObj.fileName, 
                    policyRoot["downPolicy"]["downEcuInfos"][0]["dstPkg"]["pkgFile"]["fileName"].asString().c_str());

    //!< array[0] only dst  
    EXPECT_STREQ(stDownloadInfo.stDownECUInfo.unPkgDataList.pstEcuPartPkg[1].ecuReqid, 
                    policyRoot["downPolicy"]["downEcuInfos"][1]["ecuReqId"].asString().c_str());
    EXPECT_STREQ(stDownloadInfo.stDownECUInfo.unPkgDataList.pstEcuPartPkg[1].ecuSwDid, 
                    policyRoot["downPolicy"]["downEcuInfos"][1]["ecuSwDid"].asString().c_str());
    //!< array[0] only dst --- dst
    EXPECT_EQ(stDownloadInfo.stDownECUInfo.unPkgDataList.pstEcuPartPkg[1].stDstEcuPkgInfo.ePkgType, 
                    policyRoot["downPolicy"]["downEcuInfos"][1]["dstPkg"]["pkgType"].asInt());
    EXPECT_EQ(nullptr, stDownloadInfo.stDownECUInfo.unPkgDataList.pstEcuPartPkg[1].stDstEcuPkgInfo.srcVer);
    EXPECT_NE(nullptr, stDownloadInfo.stDownECUInfo.unPkgDataList.pstEcuPartPkg[1].stDstEcuPkgInfo.dstVer);
    EXPECT_STREQ(stDownloadInfo.stDownECUInfo.unPkgDataList.pstEcuPartPkg[1].stDstEcuPkgInfo.dstVer, 
                    policyRoot["downPolicy"]["downEcuInfos"][1]["dstPkg"]["dstVer"].asString().c_str());
    EXPECT_NE(nullptr, stDownloadInfo.stDownECUInfo.unPkgDataList.pstEcuPartPkg[1].stDstEcuPkgInfo.stPkgFileObj.fileName);
    EXPECT_STREQ(stDownloadInfo.stDownECUInfo.unPkgDataList.pstEcuPartPkg[1].stDstEcuPkgInfo.stPkgFileObj.fileName, 
                    policyRoot["downPolicy"]["downEcuInfos"][1]["dstPkg"]["pkgFile"]["fileName"].asString().c_str());

    return;
}

//!< 解析集合包配置文件
TEST_F(CDpsAPITest, parseCombinePkgConf_Normal)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    Json::Value pkgConfRoot;
    ADM_DPS_PACKAGECFG_T stPkgCfgInfo = {0};

    //!< 获取构造的数据    
    iRet = module_readFile2Json(gpcCombinePackageConfJsonPath, &pkgConfRoot);
    if (ADM_IS_ERR(iRet))
    {
        cout << "failed to read file[" << gpcCombinePackageConfJsonPath << "] to json, err[" << iRet << "]"<< endl;
        return;
    }

    //!< 获取实际解析到的构造数据
    adm_memzero(&stPkgCfgInfo, sizeof(stPkgCfgInfo));
    iRet = adm_dps_parsePackageConfigFile((DCHAR *)gpcCombinePackageConfJsonPath, &stPkgCfgInfo);
    EXPECT_EQ(ADM_ECODE_SUCCESSED, iRet);

    //!< 校验构造是数据与解析的是否一致，验证解析是否正确
    EXPECT_NE(nullptr, stPkgCfgInfo.packageDesc);
    EXPECT_STREQ(stPkgCfgInfo.packageDesc, pkgConfRoot["packageDesc"].asString().c_str());
    EXPECT_NE(nullptr, stPkgCfgInfo.srcVer);
    EXPECT_STREQ(stPkgCfgInfo.srcVer, pkgConfRoot["bigSrcVer"].asString().c_str());
    EXPECT_NE(nullptr, stPkgCfgInfo.dstVer);
    EXPECT_STREQ(stPkgCfgInfo.dstVer, pkgConfRoot["bigDstVer"].asString().c_str());

    EXPECT_EQ(2, stPkgCfgInfo.stPackageList.num);
    EXPECT_NE(nullptr, stPkgCfgInfo.stPackageList.pstPkgInfo);
    //!< array[0]
    EXPECT_NE(nullptr, stPkgCfgInfo.stPackageList.pstPkgInfo[0].fileName);
    EXPECT_STREQ(stPkgCfgInfo.stPackageList.pstPkgInfo[0].fileName, 
                 pkgConfRoot["packageInfos"][0]["fileName"].asString().c_str());
    EXPECT_NE(nullptr, stPkgCfgInfo.stPackageList.pstPkgInfo[0].fileHash);
    EXPECT_STREQ(stPkgCfgInfo.stPackageList.pstPkgInfo[0].fileHash, 
                 pkgConfRoot["packageInfos"][0]["fileHash"].asString().c_str());
    EXPECT_NE(nullptr, stPkgCfgInfo.stPackageList.pstPkgInfo[0].updateObj);
    EXPECT_STREQ(stPkgCfgInfo.stPackageList.pstPkgInfo[0].updateObj, 
                 pkgConfRoot["packageInfos"][0]["updateObj"].asString().c_str());
    EXPECT_NE(nullptr, stPkgCfgInfo.stPackageList.pstPkgInfo[0].srcVer);
    EXPECT_STREQ(stPkgCfgInfo.stPackageList.pstPkgInfo[0].srcVer, 
                 pkgConfRoot["packageInfos"][0]["srcVer"].asString().c_str());
    EXPECT_NE(nullptr, stPkgCfgInfo.stPackageList.pstPkgInfo[0].dstVer);
    EXPECT_STREQ(stPkgCfgInfo.stPackageList.pstPkgInfo[0].dstVer, 
                 pkgConfRoot["packageInfos"][0]["dstVer"].asString().c_str());
    //!< array[1]
    EXPECT_NE(nullptr, stPkgCfgInfo.stPackageList.pstPkgInfo[1].fileName);
    EXPECT_STREQ(stPkgCfgInfo.stPackageList.pstPkgInfo[1].fileName, 
                 pkgConfRoot["packageInfos"][1]["fileName"].asString().c_str());
    EXPECT_NE(nullptr, stPkgCfgInfo.stPackageList.pstPkgInfo[1].fileHash);
    EXPECT_STREQ(stPkgCfgInfo.stPackageList.pstPkgInfo[1].fileHash, 
                 pkgConfRoot["packageInfos"][1]["fileHash"].asString().c_str());
    EXPECT_NE(nullptr, stPkgCfgInfo.stPackageList.pstPkgInfo[1].updateObj);
    EXPECT_STREQ(stPkgCfgInfo.stPackageList.pstPkgInfo[1].updateObj, 
                 pkgConfRoot["packageInfos"][1]["updateObj"].asString().c_str());
    EXPECT_NE(nullptr, stPkgCfgInfo.stPackageList.pstPkgInfo[1].srcVer);
    EXPECT_STREQ(stPkgCfgInfo.stPackageList.pstPkgInfo[1].srcVer, 
                 pkgConfRoot["packageInfos"][1]["srcVer"].asString().c_str());
    EXPECT_NE(nullptr, stPkgCfgInfo.stPackageList.pstPkgInfo[1].dstVer);
    EXPECT_STREQ(stPkgCfgInfo.stPackageList.pstPkgInfo[1].dstVer, 
                 pkgConfRoot["packageInfos"][1]["dstVer"].asString().c_str());

    return;
}

//!< 更新集合包配置文件到dps模块
TEST_F(CDpsAPITest, updateDownCombinePackageCfgInfo_Normal)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    D32S iOut = 0;
    ADM_COM_FILEOBJECT_T stPkgFileObj    = {0};
    ADM_DPS_PACKAGECFG_T stSrcPkgCfgInfo = {0};
    ADM_DPS_DOWN_ECUCOMBINEPKG_T stDownCombinePkgInfo = {0};

    //!< 获取实际解析到的构造数据
    adm_memzero(&stSrcPkgCfgInfo, sizeof(stSrcPkgCfgInfo));
    iRet = adm_dps_parsePackageConfigFile((DCHAR *)gpcCombinePackageConfJsonPath, &stSrcPkgCfgInfo);
    EXPECT_EQ(ADM_ECODE_SUCCESSED, iRet);

    //!< 获取对应的集合包
    adm_memzero(&stPkgFileObj, sizeof(stPkgFileObj));
    iRet = CDpsAPITest::dps_ctorGetCombinePkgFileObj(&stPkgFileObj);
    EXPECT_EQ(ADM_ECODE_SUCCESSED, iRet);

    //!< 更新pkgConfInfo
    iRet = adm_dps_updateDownCombinePackageCfgInfo(&stPkgFileObj, &stSrcPkgCfgInfo);
    EXPECT_EQ(ADM_ECODE_SUCCESSED, iRet);

    //!< 获取更新后的的pkgConfInfo
    adm_memzero(&stDownCombinePkgInfo, sizeof(stDownCombinePkgInfo));
    iRet = adm_dps_getTobeDownloadEcuCombinePackage(1, &stDownCombinePkgInfo, &iOut);
    EXPECT_EQ(ADM_ECODE_SUCCESSED, iRet);
    EXPECT_EQ(1, iOut);

    //!< 比较更新获取的是否与之前的一致
    EXPECT_STREQ(stSrcPkgCfgInfo.srcVer, stDownCombinePkgInfo.stPkgCfgInfo.srcVer);
    EXPECT_STREQ(stSrcPkgCfgInfo.dstVer, stDownCombinePkgInfo.stPkgCfgInfo.dstVer);
    EXPECT_STREQ(stSrcPkgCfgInfo.packageDesc, stDownCombinePkgInfo.stPkgCfgInfo.packageDesc);
    EXPECT_EQ(stSrcPkgCfgInfo.stPackageList.num, stDownCombinePkgInfo.stPkgCfgInfo.stPackageList.num);
    //!< 构造的配置文件中有两个包配置信息
    EXPECT_EQ(2, stDownCombinePkgInfo.stPkgCfgInfo.stPackageList.num);
    EXPECT_NE(nullptr, stDownCombinePkgInfo.stPkgCfgInfo.stPackageList.pstPkgInfo);
    //!< array[0]
    EXPECT_STREQ(stSrcPkgCfgInfo.stPackageList.pstPkgInfo[0].srcVer, 
            stDownCombinePkgInfo.stPkgCfgInfo.stPackageList.pstPkgInfo[0].srcVer);
    EXPECT_STREQ(stSrcPkgCfgInfo.stPackageList.pstPkgInfo[0].dstVer, 
            stDownCombinePkgInfo.stPkgCfgInfo.stPackageList.pstPkgInfo[0].dstVer);
    EXPECT_STREQ(stSrcPkgCfgInfo.stPackageList.pstPkgInfo[0].fileName, 
            stDownCombinePkgInfo.stPkgCfgInfo.stPackageList.pstPkgInfo[0].fileName);
    EXPECT_STREQ(stSrcPkgCfgInfo.stPackageList.pstPkgInfo[0].fileHash, 
            stDownCombinePkgInfo.stPkgCfgInfo.stPackageList.pstPkgInfo[0].fileHash);
    EXPECT_STREQ(stSrcPkgCfgInfo.stPackageList.pstPkgInfo[0].updateObj, 
            stDownCombinePkgInfo.stPkgCfgInfo.stPackageList.pstPkgInfo[0].updateObj);
    EXPECT_EQ(stSrcPkgCfgInfo.stPackageList.pstPkgInfo[0].fileSize, 
            stDownCombinePkgInfo.stPkgCfgInfo.stPackageList.pstPkgInfo[0].fileSize);
    EXPECT_EQ(stSrcPkgCfgInfo.stPackageList.pstPkgInfo[0].packageType, 
            stDownCombinePkgInfo.stPkgCfgInfo.stPackageList.pstPkgInfo[0].packageType);
    //!< array[1]
    EXPECT_STREQ(stSrcPkgCfgInfo.stPackageList.pstPkgInfo[1].srcVer, 
            stDownCombinePkgInfo.stPkgCfgInfo.stPackageList.pstPkgInfo[1].srcVer);
    EXPECT_STREQ(stSrcPkgCfgInfo.stPackageList.pstPkgInfo[1].dstVer, 
            stDownCombinePkgInfo.stPkgCfgInfo.stPackageList.pstPkgInfo[1].dstVer);
    EXPECT_STREQ(stSrcPkgCfgInfo.stPackageList.pstPkgInfo[1].fileName, 
            stDownCombinePkgInfo.stPkgCfgInfo.stPackageList.pstPkgInfo[1].fileName);
    EXPECT_STREQ(stSrcPkgCfgInfo.stPackageList.pstPkgInfo[1].fileHash, 
            stDownCombinePkgInfo.stPkgCfgInfo.stPackageList.pstPkgInfo[1].fileHash);
    EXPECT_STREQ(stSrcPkgCfgInfo.stPackageList.pstPkgInfo[1].updateObj, 
            stDownCombinePkgInfo.stPkgCfgInfo.stPackageList.pstPkgInfo[1].updateObj);
    EXPECT_EQ(stSrcPkgCfgInfo.stPackageList.pstPkgInfo[1].fileSize, 
            stDownCombinePkgInfo.stPkgCfgInfo.stPackageList.pstPkgInfo[1].fileSize);
    EXPECT_EQ(stSrcPkgCfgInfo.stPackageList.pstPkgInfo[1].packageType, 
            stDownCombinePkgInfo.stPkgCfgInfo.stPackageList.pstPkgInfo[1].packageType);

    adm_com_cleanFileObject(&stPkgFileObj);
    adm_dps_cleanPackageCfgInfo(&stSrcPkgCfgInfo);
    adm_dps_cleanEcuCombinePackage(&stDownCombinePkgInfo);

    return;
}

//!< 检查更新
TEST_F(CDpsAPITest, checkAndUpdateDownInfo_Normal)
{
    /*
     *  dps的下载策略由集合包检查更新后变成零件包 
    */
    D32S iRet = ADM_ECODE_SUCCESSED;
    ADM_DPS_DOWNTSAKINFO_T stCurDownTaskInfo = {0};
    ADM_DPS_DOWNTSAKINFO_T stNewDownTaskInfo = {0};
    ADM_DPS_DOWNPACKAGEFORMAT_E enCurPkgFormat = ADM_DPS_DOWNPACKAGE_PART;
    ADM_DPS_DOWNPACKAGEFORMAT_E enNewPkgFormat = ADM_DPS_DOWNPACKAGE_COMBINE;

    iRet = adm_dps_EcuPackageFormat(&enCurPkgFormat);
    EXPECT_EQ(ADM_ECODE_SUCCESSED, iRet);
    EXPECT_EQ(ADM_DPS_DOWNPACKAGE_COMBINE, enCurPkgFormat);

    //!< 获取当前下载任务信息
    adm_memzero(&stCurDownTaskInfo, sizeof(stCurDownTaskInfo));
    iRet = adm_dps_getDownloadTaskInfo(&stCurDownTaskInfo);
    EXPECT_EQ(ADM_ECODE_SUCCESSED, iRet);
    EXPECT_NE(0, stCurDownTaskInfo.taskId);
    EXPECT_NE(0, stCurDownTaskInfo.strategyId);
    EXPECT_NE(nullptr, stCurDownTaskInfo.sessionId);

    //!< 检查更新下载策略
    iRet = adm_dps_checkAndUpdateDPSDownInfo(CDpsAPITest::dps_ctorCheckNewDownloadPolicy,CDpsAPITest::dps_ctorCleanSlaveDownPackage);
    EXPECT_EQ(ADM_ECODE_SUCCESSED, iRet);

    //!< 获取更新后升级包格式
    iRet = adm_dps_EcuPackageFormat(&enNewPkgFormat);
    EXPECT_EQ(ADM_ECODE_SUCCESSED, iRet);
    EXPECT_EQ(ADM_DPS_DOWNPACKAGE_PART, enNewPkgFormat);

    //!< 获取检查更新后的下载任务信息
    adm_memzero(&stNewDownTaskInfo, sizeof(stNewDownTaskInfo));
    iRet = adm_dps_getDownloadTaskInfo(&stNewDownTaskInfo);
    EXPECT_EQ(ADM_ECODE_SUCCESSED, iRet);
    EXPECT_NE(0, stNewDownTaskInfo.taskId);
    EXPECT_NE(0, stNewDownTaskInfo.strategyId);
    EXPECT_NE(nullptr, stNewDownTaskInfo.sessionId);

    //!< 比较上次的与更新后的下载任务信息
    EXPECT_NE(stCurDownTaskInfo.taskId, stNewDownTaskInfo.taskId);
    EXPECT_NE(stCurDownTaskInfo.strategyId, stNewDownTaskInfo.strategyId);
    EXPECT_STRNE(stCurDownTaskInfo.sessionId, stNewDownTaskInfo.sessionId);

    //!< 因后续的测试依赖于 gpcCheckPolicyJsonPath 这个策略，所以此时需要更新会该策略，避免影响后续的测试数据
    iRet = adm_dps_checkAndUpdateDPSDownInfo(CDpsAPITest::dps_ctorCheckDownloadPolicy,CDpsAPITest::dps_ctorCleanSlaveDownPackage);
    EXPECT_EQ(ADM_ECODE_SUCCESSED, iRet);

    adm_dps_cleanDownTaskInfo(&stCurDownTaskInfo);
    adm_dps_cleanDownTaskInfo(&stNewDownTaskInfo);

    return;
}

#if 0
/** 具体的策略解析后的数据获取API **/
#endif

TEST_F(CDpsAPITest, getDownPackageFormat_Normal)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    ADM_DPS_DOWNPACKAGEFORMAT_E enPkgFormat = ADM_DPS_DOWNPACKAGE_PART;

    iRet = adm_dps_EcuPackageFormat(&enPkgFormat);
    EXPECT_EQ(ADM_ECODE_SUCCESSED, iRet);
    EXPECT_EQ(ADM_DPS_DOWNPACKAGE_COMBINE, enPkgFormat);

    return;
}

TEST_F(CDpsAPITest, getDownloadTaskInfo_Normal)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    ADM_DPS_DOWNTSAKINFO_T stCurDownTaskInfo = {0};

    //!< 获取当前下载任务信息
    adm_memzero(&stCurDownTaskInfo, sizeof(stCurDownTaskInfo));
    iRet = adm_dps_getDownloadTaskInfo(&stCurDownTaskInfo);
    EXPECT_EQ(ADM_ECODE_SUCCESSED, iRet);
    EXPECT_NE(0, stCurDownTaskInfo.taskId);
    EXPECT_NE(0, stCurDownTaskInfo.strategyId);
    EXPECT_NE(nullptr, stCurDownTaskInfo.sessionId);

    adm_dps_cleanDownTaskInfo(&stCurDownTaskInfo);
    return;
}

TEST_F(CDpsAPITest, getDownloadCondition_Normal)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    Json::Value policyRoot;
    ADM_DPS_DOWNCONDITION_T stDownCondition = {0};

    iRet = module_readFile2Json(gpcCheckPolicyJsonPath, &policyRoot);
    if (ADM_IS_ERR(iRet))
    {
        cout << "failed to read file[" << gpcCheckPolicyJsonPath << "] to json, err[" << iRet << "]"<< endl;
        return;
    }

    adm_memzero(&stDownCondition, sizeof(stDownCondition));
    iRet = adm_dps_getDownloadCondition(&stDownCondition);
    EXPECT_EQ(ADM_ECODE_SUCCESSED, iRet);

    //!< 下载前置条件规则校验
    EXPECT_EQ(3, stDownCondition.trigNum);
    EXPECT_EQ(stDownCondition.trigNum, policyRoot["downPolicy"]["downConditions"].size());
    EXPECT_EQ(stDownCondition.pstTrigDownCondList[0].iType, policyRoot["downPolicy"]["downConditions"][0]["type"].asInt());
    EXPECT_EQ(stDownCondition.pstTrigDownCondList[0].iCondNum, policyRoot["downPolicy"]["downConditions"][0]["conditions"].size());
    EXPECT_STREQ(stDownCondition.pstTrigDownCondList[0].pCondtionS[0].messageId, 
            policyRoot["downPolicy"]["downConditions"][0]["conditions"][0]["messageId"].asString().c_str());
    EXPECT_EQ(stDownCondition.pstTrigDownCondList[1].iType, policyRoot["downPolicy"]["downConditions"][1]["type"].asInt());
    EXPECT_EQ(stDownCondition.pstTrigDownCondList[1].iCondNum, policyRoot["downPolicy"]["downConditions"][1]["conditions"].size());
    EXPECT_EQ(stDownCondition.pstTrigDownCondList[1].pCondtionS[0].connectRelation, 
            policyRoot["downPolicy"]["downConditions"][1]["conditions"][0]["connectRelation"].asInt());
    EXPECT_EQ(stDownCondition.pstTrigDownCondList[2].iType, policyRoot["downPolicy"]["downConditions"][2]["type"].asInt());
    EXPECT_EQ(stDownCondition.pstTrigDownCondList[2].iCondNum, policyRoot["downPolicy"]["downConditions"][2]["conditions"].size());
    EXPECT_STREQ(stDownCondition.pstTrigDownCondList[2].pCondtionS[0].messageId, 
            policyRoot["downPolicy"]["downConditions"][2]["conditions"][0]["messageId"].asString().c_str());

    adm_dps_cleanEcuDownCondition(&stDownCondition);
    return;
}

TEST_F(CDpsAPITest, getTobeDownloadEcuCombinePackage_Normal)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    D32S tobeDownloadNum = 0;
    Json::Value policyRoot;
    ADM_DPS_ECUIDLIST_T stEcuIdList = {0};
    ADM_DPS_DOWN_ECUCOMBINEPKG_T *pstDownCombinePkgList = nullptr;

    iRet = module_readFile2Json(gpcCheckPolicyJsonPath, &policyRoot);
    if (ADM_IS_ERR(iRet))
    {
        cout << "failed to read file[" << gpcCheckPolicyJsonPath << "] to json, err[" << iRet << "]"<< endl;
        return;
    }

    pstDownCombinePkgList = adm_dps_newEcuCombinePackageList(3);
    EXPECT_NE(nullptr, pstDownCombinePkgList);

    //!< 获取待下载集合包信息
    adm_memzero(pstDownCombinePkgList, 3 * sizeof(*pstDownCombinePkgList));
    iRet = adm_dps_getTobeDownloadEcuCombinePackage(3, pstDownCombinePkgList, &tobeDownloadNum);
    EXPECT_EQ(ADM_ECODE_SUCCESSED, iRet);
    EXPECT_EQ(1, tobeDownloadNum);

    //!< 比较(pstDownCombinePkgList->stPkgFileObj/stEcuIdList)与构造数据中的集合包信息是否一致
    //!< pkgFile
    EXPECT_EQ(pstDownCombinePkgList->stPkgFileObj.size, 
                    policyRoot["downPolicy"]["downPkgInfos"][0]["pkgFile"]["size"].asInt64());
    EXPECT_STREQ(pstDownCombinePkgList->stPkgFileObj.fileName, 
                    policyRoot["downPolicy"]["downPkgInfos"][0]["pkgFile"]["fileName"].asString().c_str());
    EXPECT_STREQ(pstDownCombinePkgList->stPkgFileObj.url, 
                    policyRoot["downPolicy"]["downPkgInfos"][0]["pkgFile"]["url"].asString().c_str());
    EXPECT_STREQ(pstDownCombinePkgList->stPkgFileObj.hash, 
                    policyRoot["downPolicy"]["downPkgInfos"][0]["pkgFile"]["hash"].asString().c_str());
    //!< ecuInfos
    EXPECT_EQ(1, pstDownCombinePkgList->stEcuIdList.num);
    EXPECT_STREQ(pstDownCombinePkgList->stEcuIdList.pstEcuIdInfo[0].ecuReqid, 
                    policyRoot["downPolicy"]["downPkgInfos"][0]["ecuInfos"][0]["ecuReqId"].asString().c_str());
    EXPECT_STREQ(pstDownCombinePkgList->stEcuIdList.pstEcuIdInfo[0].ecuSwDid, 
                    policyRoot["downPolicy"]["downPkgInfos"][0]["ecuInfos"][0]["ecuSwDid"].asString().c_str());
    //TODO:ADM_DPS_ECUIDINFO_T结构变更,影响test，后续完善
    //if (policyRoot["downPolicy"]["downPkgInfos"][0]["ecuInfos"][0]["srcPkgName"].isString())
    //{
    //    EXPECT_NE(nullptr, pstDownCombinePkgList->stEcuIdList.pstEcuIdInfo[0].srcPkgName);
    //    EXPECT_STREQ(pstDownCombinePkgList->stEcuIdList.pstEcuIdInfo[0].srcPkgName, 
    //                policyRoot["downPolicy"]["downPkgInfos"][0]["ecuInfos"][0]["srcPkgName"].asString().c_str());
    //}
    //if (policyRoot["downPolicy"]["downPkgInfos"][0]["ecuInfos"][0]["dstPkgName"].isString())
    //{
    //    EXPECT_STREQ(pstDownCombinePkgList->stEcuIdList.pstEcuIdInfo[0].dstPkgName, 
    //                policyRoot["downPolicy"]["downPkgInfos"][0]["ecuInfos"][0]["dstPkgName"].asString().c_str());
    //}

    //!< 验证一下 adm_dps_getDownEcuCombinePackageEcuIdList 接口
    EXPECT_NE(nullptr, pstDownCombinePkgList->stPkgFileObj.fileName);
    iRet = adm_dps_getDownEcuCombinePackageEcuIdList(pstDownCombinePkgList->stPkgFileObj.fileName, &stEcuIdList);
    EXPECT_EQ(ADM_ECODE_SUCCESSED, iRet);
    EXPECT_EQ(1, stEcuIdList.num);
    EXPECT_STREQ(pstDownCombinePkgList->stEcuIdList.pstEcuIdInfo[0].ecuReqid, stEcuIdList.pstEcuIdInfo[0].ecuReqid);
    EXPECT_STREQ(pstDownCombinePkgList->stEcuIdList.pstEcuIdInfo[0].ecuSwDid, stEcuIdList.pstEcuIdInfo[0].ecuSwDid);

    //TODO:ADM_DPS_ECUIDINFO_T结构变更,影响test，后续完善
    //EXPECT_STREQ(pstDownCombinePkgList->stEcuIdList.pstEcuIdInfo[0].srcPkgName, stEcuIdList.pstEcuIdInfo[0].srcPkgName);
    //EXPECT_STREQ(pstDownCombinePkgList->stEcuIdList.pstEcuIdInfo[0].dstPkgName, stEcuIdList.pstEcuIdInfo[0].dstPkgName);

    adm_dps_delEcuIdList((D32U)stEcuIdList.num, &stEcuIdList.pstEcuIdInfo);
    adm_dps_delEcuCombinePackageList(3, &pstDownCombinePkgList);
    return;
}

TEST_F(CDpsAPITest, getTobeDownloadEcuPartPackage_Normal)
{
    /*
     *  先将策略更新成零件包，在测试零件包信息，最后更新成集合包
     */
    D32S iRet = ADM_ECODE_SUCCESSED;
    D32S tobeDownloadNum = 0;
    Json::Value policyRoot;
    ADM_DPS_DOWN_ECUPARTPKG_T *pstDownPartPkgList = nullptr;

    //!< 将下载策略更新成零件包
    iRet = adm_dps_checkAndUpdateDPSDownInfo(CDpsAPITest::dps_ctorCheckNewDownloadPolicy,CDpsAPITest::dps_ctorCleanSlaveDownPackage);
    EXPECT_EQ(ADM_ECODE_SUCCESSED, iRet);

    iRet = module_readFile2Json(gpcDownEcuPolicyJsonPath, &policyRoot);
    if (ADM_IS_ERR(iRet))
    {
        cout << "failed to read file[" << gpcDownEcuPolicyJsonPath << "] to json, err[" << iRet << "]"<< endl;
        return;
    }

    pstDownPartPkgList = adm_dps_newEcuPartPackageList(3);
    EXPECT_NE(nullptr, pstDownPartPkgList);

    //!< 获取待下载零件包信息
    iRet = adm_dps_getTobeDownloadEcuPartPackage(3, pstDownPartPkgList, &tobeDownloadNum);
    EXPECT_EQ(ADM_ECODE_SUCCESSED, iRet);
    EXPECT_EQ(2, tobeDownloadNum);

    //!< array[0] src and dst
    EXPECT_STREQ(pstDownPartPkgList[0].ecuReqid, policyRoot["downPolicy"]["downEcuInfos"][0]["ecuReqId"].asString().c_str());
    EXPECT_STREQ(pstDownPartPkgList[0].ecuSwDid, policyRoot["downPolicy"]["downEcuInfos"][0]["ecuSwDid"].asString().c_str());
    //!< array[0] src and dst --- src
    EXPECT_NE(nullptr, pstDownPartPkgList[0].pstSrcEcuPkgInfo);
    EXPECT_EQ(pstDownPartPkgList[0].pstSrcEcuPkgInfo->ePkgType, policyRoot["downPolicy"]["downEcuInfos"][0]["srcPkg"]["pkgType"].asInt());
    EXPECT_NE(nullptr, pstDownPartPkgList[0].pstSrcEcuPkgInfo->srcVer);
    EXPECT_STREQ(pstDownPartPkgList[0].pstSrcEcuPkgInfo->srcVer, policyRoot["downPolicy"]["downEcuInfos"][0]["srcPkg"]["srcVer"].asString().c_str());
    EXPECT_NE(nullptr, pstDownPartPkgList[0].pstSrcEcuPkgInfo->dstVer);
    EXPECT_STREQ(pstDownPartPkgList[0].pstSrcEcuPkgInfo->dstVer, policyRoot["downPolicy"]["downEcuInfos"][0]["srcPkg"]["dstVer"].asString().c_str());
    EXPECT_NE(nullptr, pstDownPartPkgList[0].pstSrcEcuPkgInfo->stPkgFileObj.fileName);
    EXPECT_STREQ(pstDownPartPkgList[0].pstSrcEcuPkgInfo->stPkgFileObj.fileName, 
        policyRoot["downPolicy"]["downEcuInfos"][0]["srcPkg"]["pkgFile"]["fileName"].asString().c_str());
    //!< array[0] src and dst --- dst
    EXPECT_EQ(pstDownPartPkgList[0].stDstEcuPkgInfo.ePkgType, policyRoot["downPolicy"]["downEcuInfos"][0]["dstPkg"]["pkgType"].asInt());
    EXPECT_NE(nullptr, pstDownPartPkgList[0].stDstEcuPkgInfo.srcVer);
    EXPECT_STREQ(pstDownPartPkgList[0].stDstEcuPkgInfo.srcVer, policyRoot["downPolicy"]["downEcuInfos"][0]["dstPkg"]["srcVer"].asString().c_str());
    EXPECT_NE(nullptr, pstDownPartPkgList[0].stDstEcuPkgInfo.dstVer);
    EXPECT_STREQ(pstDownPartPkgList[0].stDstEcuPkgInfo.dstVer, policyRoot["downPolicy"]["downEcuInfos"][0]["dstPkg"]["dstVer"].asString().c_str());
    EXPECT_NE(nullptr, pstDownPartPkgList[0].stDstEcuPkgInfo.stPkgFileObj.fileName);
    EXPECT_STREQ(pstDownPartPkgList[0].stDstEcuPkgInfo.stPkgFileObj.fileName, 
                    policyRoot["downPolicy"]["downEcuInfos"][0]["dstPkg"]["pkgFile"]["fileName"].asString().c_str());

    //!< array[1] only dst  
    EXPECT_STREQ(pstDownPartPkgList[1].ecuReqid, policyRoot["downPolicy"]["downEcuInfos"][1]["ecuReqId"].asString().c_str());
    EXPECT_STREQ(pstDownPartPkgList[1].ecuSwDid, policyRoot["downPolicy"]["downEcuInfos"][1]["ecuSwDid"].asString().c_str());
    //!< array[1] only dst --- dst
    EXPECT_EQ(pstDownPartPkgList[1].stDstEcuPkgInfo.ePkgType, policyRoot["downPolicy"]["downEcuInfos"][1]["dstPkg"]["pkgType"].asInt());
    EXPECT_EQ(nullptr, pstDownPartPkgList[1].stDstEcuPkgInfo.srcVer);
    EXPECT_NE(nullptr, pstDownPartPkgList[1].stDstEcuPkgInfo.dstVer);
    EXPECT_STREQ(pstDownPartPkgList[1].stDstEcuPkgInfo.dstVer, policyRoot["downPolicy"]["downEcuInfos"][1]["dstPkg"]["dstVer"].asString().c_str());
    EXPECT_NE(nullptr, pstDownPartPkgList[1].stDstEcuPkgInfo.stPkgFileObj.fileName);
    EXPECT_STREQ(pstDownPartPkgList[1].stDstEcuPkgInfo.stPkgFileObj.fileName, 
                    policyRoot["downPolicy"]["downEcuInfos"][1]["dstPkg"]["pkgFile"]["fileName"].asString().c_str());

    adm_dps_delEcuPartPackageList(3, &pstDownPartPkgList);

    return;
}

TEST_F(CDpsAPITest, updateDownPackageProgressTest_CombinePackage)
{
    /*
     *  更新后再获取出来看看获取到的是否与设置值一致
     */
    D32S iRet = ADM_ECODE_SUCCESSED;
    D32S tobeDownloadNum = 0;
    Json::Value policyRoot;
    ADM_DPS_PKGATTR_T      stPkgAttr  = {0};
    ADM_DPS_DOWNPROGRESS_T stProgress = {0};    //!< 填充的下载进度信息
    ADM_DPS_DOWNPKGPROGRESS_T *pstDownPkgProgress = NULL; //!< 获取的下载进度信息 
    ADM_DPS_DOWNPACKAGEFORMAT_E enPkgFormat = ADM_DPS_DOWNPACKAGE_PART;
    ADM_DPS_DOWN_ECUCOMBINEPKG_T *pstDownCombinePkgList = nullptr;

    //!< 将下载策略更新成集合包
    iRet = adm_dps_checkAndUpdateDPSDownInfo(CDpsAPITest::dps_ctorCheckDownloadPolicy,CDpsAPITest::dps_ctorCleanSlaveDownPackage);
    EXPECT_EQ(ADM_ECODE_SUCCESSED, iRet);

    iRet = module_readFile2Json(gpcCheckPolicyJsonPath, &policyRoot);
    if (ADM_IS_ERR(iRet))
    {
        cout << "failed to read file[" << gpcCheckPolicyJsonPath << "] to json, err[" << iRet << "]"<< endl;
        return;
    }

    //!< 获取更新后升级包格式
    iRet = adm_dps_EcuPackageFormat(&enPkgFormat);
    EXPECT_EQ(ADM_ECODE_SUCCESSED, iRet);
    EXPECT_EQ(ADM_DPS_DOWNPACKAGE_COMBINE, enPkgFormat);

    //!< 构造下载元素属性信息
    adm_memzero(&stPkgAttr, sizeof(stPkgAttr));
    stPkgAttr.fileName = adm_strdup(policyRoot["downPolicy"]["downPkgInfos"][0]["pkgFile"]["fileName"].asString().c_str());
    EXPECT_NE(nullptr, stPkgAttr.fileName);
    stPkgAttr.baseSize     = 0;
    stPkgAttr.enPkgFormat  = ADM_DPS_DOWNPACKAGE_COMBINE;
    stPkgAttr.wholePkgSize = policyRoot["downPolicy"]["downPkgInfos"][0]["pkgFile"]["size"].asInt64();

    //!< 构造下载进度 --- 50%
    stProgress.speed      = 100;
    stProgress.totalTime  = 200;
    stProgress.remainTime = 102;
    stProgress.saveBytes  = stPkgAttr.wholePkgSize;
    stProgress.totalBytes = stPkgAttr.wholePkgSize;

    pstDownCombinePkgList = adm_dps_newEcuCombinePackageList(3);
    EXPECT_NE(nullptr, pstDownCombinePkgList);

    //!< 顺便测试一下获取接口是否有问题
    adm_memzero(pstDownCombinePkgList, 3 * sizeof(*pstDownCombinePkgList));
    iRet = adm_dps_getTobeDownloadEcuCombinePackage(3, pstDownCombinePkgList, &tobeDownloadNum);
    EXPECT_EQ(ADM_ECODE_SUCCESSED, iRet);
    EXPECT_EQ(1, tobeDownloadNum);
    adm_dps_cleanEcuCombinePackageList(3, pstDownCombinePkgList);

    //!< 更新下载进度
    iRet = adm_dps_updateDownPackageProgress(&stPkgAttr, &stProgress);
    EXPECT_EQ(ADM_ECODE_SUCCESSED, iRet);

    //!< 获取下载进度信息
    pstDownPkgProgress = adm_dps_getEcuDownloadProgress();
    EXPECT_NE(nullptr, pstDownPkgProgress);

    //!< 校验下载进度信息
    EXPECT_EQ(1, pstDownPkgProgress->num);
    EXPECT_NE(nullptr, pstDownPkgProgress->pstProgressList);
    EXPECT_STREQ(pstDownPkgProgress->pstProgressList[0].ecuReqid, 
                    policyRoot["downPolicy"]["downPkgInfos"][0]["ecuInfos"][0]["ecuReqId"].asString().c_str());
    EXPECT_STREQ(pstDownPkgProgress->pstProgressList[0].ecuSwDid, 
                    policyRoot["downPolicy"]["downPkgInfos"][0]["ecuInfos"][0]["ecuSwDid"].asString().c_str());
    EXPECT_EQ(pstDownPkgProgress->pstProgressList[0].stProgress.speed,      stProgress.speed);
    EXPECT_EQ(pstDownPkgProgress->pstProgressList[0].stProgress.totalTime,  stProgress.totalTime);
    EXPECT_EQ(pstDownPkgProgress->pstProgressList[0].stProgress.remainTime, stProgress.remainTime);
    EXPECT_EQ(pstDownPkgProgress->pstProgressList[0].stProgress.saveBytes,  stProgress.saveBytes);
    EXPECT_EQ(pstDownPkgProgress->pstProgressList[0].stProgress.totalBytes, stProgress.totalBytes);

    adm_dps_cleanPackageAttribute(&stPkgAttr);
    adm_dps_delDownPackageProgress(&pstDownPkgProgress);
    adm_dps_delEcuCombinePackageList(3, &pstDownCombinePkgList);
    return;
}

TEST_F(CDpsAPITest, updateDownPackageResultTest_CombinePackage)
{
    /*
     *  更新后再获取出来看看获取到的是否与设置值一致
     */
    D32S iRet = ADM_ECODE_SUCCESSED;
    Json::Value policyRoot;
    ADM_DPS_PKGATTR_T    stPkgAttr = {0};
    ADM_DPS_DOWNRESULT_T stResult  = {0};    //!< 填充的下载结果信息
    ADM_DPS_DOWNPKGRESULT_T *pstDownPkgResult = NULL; //!< 获取的下载结果信息 
    ADM_DPS_DOWNPACKAGEFORMAT_E enPkgFormat = ADM_DPS_DOWNPACKAGE_PART;
    const DCHAR *pcPkgPath   = "/tmp/xxx.zip";

    //!< 将下载策略更新成集合包
    iRet = adm_dps_checkAndUpdateDPSDownInfo(CDpsAPITest::dps_ctorCheckDownloadPolicy,CDpsAPITest::dps_ctorCleanSlaveDownPackage);
    EXPECT_EQ(ADM_ECODE_SUCCESSED, iRet);

    iRet = module_readFile2Json(gpcCheckPolicyJsonPath, &policyRoot);
    if (ADM_IS_ERR(iRet))
    {
        cout << "failed to read file[" << gpcCheckPolicyJsonPath << "] to json, err[" << iRet << "]"<< endl;
        return;
    }
    //!< 获取更新后升级包格式
    iRet = adm_dps_EcuPackageFormat(&enPkgFormat);
    EXPECT_EQ(ADM_ECODE_SUCCESSED, iRet);
    EXPECT_EQ(ADM_DPS_DOWNPACKAGE_COMBINE, enPkgFormat);

    //!< 构造下载元素属性信息
    adm_memzero(&stPkgAttr, sizeof(stPkgAttr));
    stPkgAttr.fileName = adm_strdup(policyRoot["downPolicy"]["downPkgInfos"][0]["pkgFile"]["fileName"].asString().c_str());
    EXPECT_NE(nullptr, stPkgAttr.fileName);
    stPkgAttr.baseSize     = 0;
    stPkgAttr.enPkgFormat  = ADM_DPS_DOWNPACKAGE_COMBINE;
    stPkgAttr.wholePkgSize = policyRoot["downPolicy"]["downPkgInfos"][0]["pkgFile"]["size"].asInt64();

    //!< 构造下载结果 --- success
    stResult.result     = 0;
    stResult.eDownState = (ADM_IS_ERR(stResult.result) ? 
                            (ADM_DPS_DOWNPKG_STATE_FAILED) : (ADM_DPS_DOWNPKG_STATE_FINISH));
    stResult.failReason = (ADM_IS_ERR(stResult.result) ? 
                            adm_strdup("DownResult: err. (test)") : adm_strdup("DownResult: ok. (test)"));

    //!< 更新下载结果
    iRet = adm_dps_updateDownPackageResult(&stPkgAttr, &stResult, pcPkgPath);
    EXPECT_EQ(ADM_ECODE_SUCCESSED, iRet);

    //!< 获取下载进度信息
    pstDownPkgResult = adm_dps_getEcuDownloadResult();
    EXPECT_NE(nullptr, pstDownPkgResult);

    //!< 校验下载进度信息
    EXPECT_EQ(1, pstDownPkgResult->num);
    EXPECT_NE(nullptr, pstDownPkgResult->pstResultList);
    EXPECT_STREQ(pstDownPkgResult->pstResultList[0].ecuReqid, 
                    policyRoot["downPolicy"]["downPkgInfos"][0]["ecuInfos"][0]["ecuReqId"].asString().c_str());
    EXPECT_STREQ(pstDownPkgResult->pstResultList[0].ecuSwDid, 
                    policyRoot["downPolicy"]["downPkgInfos"][0]["ecuInfos"][0]["ecuSwDid"].asString().c_str());
    EXPECT_EQ(pstDownPkgResult->pstResultList[0].stResult.result,     stResult.result);
    EXPECT_EQ(pstDownPkgResult->pstResultList[0].stResult.eDownState, stResult.eDownState);
    EXPECT_STREQ(pstDownPkgResult->pstResultList[0].stResult.failReason, stResult.failReason);

    adm_free(stResult.failReason);
    adm_dps_cleanPackageAttribute(&stPkgAttr);
    adm_dps_delDownPackageResult(&pstDownPkgResult);
    return;
}

TEST_F(CDpsAPITest, updateDownPackageProgressTest_EcuPackage)
{
    /*
     *  更新后再获取出来看看获取到的是否与设置值一致
     */
    D32S i = 0;
    D32S iRet = ADM_ECODE_SUCCESSED;
    D32S tobeDownloadNum = 0;
    Json::Value policyRoot;
    ADM_DPS_PKGATTR_T      stPkgAttr  = {0};
    ADM_DPS_DOWNPROGRESS_T stProgress = {0};    //!< 填充的下载进度信息
    ADM_DPS_DOWNPKGPROGRESS_T *pstDownPkgProgress = NULL; //!< 获取的下载进度信息 
    ADM_DPS_DOWNPACKAGEFORMAT_E enPkgFormat = ADM_DPS_DOWNPACKAGE_COMBINE;
    ADM_DPS_DOWN_ECUPARTPKG_T *pstDownPartPkgList = nullptr;

    //!< 将下载策略更新成集合包
    iRet = adm_dps_checkAndUpdateDPSDownInfo(CDpsAPITest::dps_ctorCheckNewDownloadPolicy,CDpsAPITest::dps_ctorCleanSlaveDownPackage);
    EXPECT_EQ(ADM_ECODE_SUCCESSED, iRet);

    iRet = module_readFile2Json(gpcDownEcuPolicyJsonPath, &policyRoot);
    if (ADM_IS_ERR(iRet))
    {
        cout << "failed to read file[" << gpcDownEcuPolicyJsonPath << "] to json, err[" << iRet << "]"<< endl;
        return;
    }

    //!< 获取更新后升级包格式
    iRet = adm_dps_EcuPackageFormat(&enPkgFormat);
    EXPECT_EQ(ADM_ECODE_SUCCESSED, iRet);
    EXPECT_EQ(ADM_DPS_DOWNPACKAGE_PART, enPkgFormat);

    //!< 构造下载元素属性信息 --- src package
    adm_memzero(&stPkgAttr, sizeof(stPkgAttr));
    stPkgAttr.fileName = nullptr;
    stPkgAttr.baseSize   = 0;
    stPkgAttr.isRollback = TRUE;
    stPkgAttr.ecuReqid = adm_strdup(policyRoot["downPolicy"]["downEcuInfos"][0]["ecuReqId"].asString().c_str());
    EXPECT_NE(nullptr, stPkgAttr.ecuReqid);
    stPkgAttr.ecuSwDid = adm_strdup(policyRoot["downPolicy"]["downEcuInfos"][0]["ecuSwDid"].asString().c_str());
    EXPECT_NE(nullptr, stPkgAttr.ecuSwDid);
    stPkgAttr.enPkgFormat  = ADM_DPS_DOWNPACKAGE_PART;
    stPkgAttr.wholePkgSize = policyRoot["downPolicy"]["downEcuInfos"][0]["srcPkg"]["pkgFile"]["size"].asInt64() + 
                                policyRoot["downPolicy"]["downEcuInfos"][0]["dstPkg"]["pkgFile"]["size"].asInt64();

    //!< 构造下载进度 --- 50%
    stProgress.speed      = 100;
    stProgress.totalTime  = 200;
    stProgress.remainTime = 102;
    stProgress.saveBytes  = stPkgAttr.wholePkgSize;
    stProgress.totalBytes = stPkgAttr.wholePkgSize;

    //!< 顺便测试一下获取接口是否有问题
    pstDownPartPkgList = adm_dps_newEcuPartPackageList(3);
    EXPECT_NE(nullptr, pstDownPartPkgList);

    //!< 获取待下载零件包信息
    iRet = adm_dps_getTobeDownloadEcuPartPackage(3, pstDownPartPkgList, &tobeDownloadNum);
    EXPECT_EQ(ADM_ECODE_SUCCESSED, iRet);
    EXPECT_EQ(2, tobeDownloadNum);
    adm_dps_cleanEcuPartPackageList(3, pstDownPartPkgList);

    //!< 更新下载进度
    iRet = adm_dps_updateDownPackageProgress(&stPkgAttr, &stProgress);
    EXPECT_EQ(ADM_ECODE_SUCCESSED, iRet);

    //!< 获取下载进度信息
    pstDownPkgProgress = adm_dps_getEcuDownloadProgress();
    EXPECT_NE(nullptr, pstDownPkgProgress);

    //!< 校验下载进度信息
    EXPECT_EQ(2, pstDownPkgProgress->num);
    EXPECT_NE(nullptr, pstDownPkgProgress->pstProgressList);
    for (i = 0; i < pstDownPkgProgress->num; i++)
    {
        if ((0 == adm_strcasecmp(stPkgAttr.ecuReqid, pstDownPkgProgress->pstProgressList[i].ecuReqid)) &&
            (0 == adm_strcasecmp(stPkgAttr.ecuSwDid, pstDownPkgProgress->pstProgressList[i].ecuSwDid)))
        {
            EXPECT_EQ(pstDownPkgProgress->pstProgressList[0].stProgress.speed,      stProgress.speed);
            EXPECT_EQ(pstDownPkgProgress->pstProgressList[0].stProgress.totalTime,  stProgress.totalTime);
            EXPECT_EQ(pstDownPkgProgress->pstProgressList[0].stProgress.remainTime, stProgress.remainTime);
            EXPECT_EQ(pstDownPkgProgress->pstProgressList[0].stProgress.saveBytes,  stProgress.saveBytes);
            EXPECT_EQ(pstDownPkgProgress->pstProgressList[0].stProgress.totalBytes, stProgress.totalBytes);
            break;
        }
    }
    //!< 校验上面未找到的情况
    EXPECT_NE(i, pstDownPkgProgress->num);

    adm_dps_cleanPackageAttribute(&stPkgAttr);
    adm_dps_delDownPackageProgress(&pstDownPkgProgress);
    adm_dps_delEcuPartPackageList(3, &pstDownPartPkgList);
    return;
}

TEST_F(CDpsAPITest, updateDownPackageResultTest_EcuPackage)
{
    /*
     *  更新后再获取出来看看获取到的是否与设置值一致
     */
    D32S i = 0;
    D32S iRet = ADM_ECODE_SUCCESSED;
    Json::Value policyRoot;
    ADM_DPS_PKGATTR_T    stPkgAttr = {0};
    ADM_DPS_DOWNRESULT_T stResult  = {0};    //!< 填充的下载结果信息
    ADM_DPS_DOWNPKGRESULT_T *pstDownPkgResult = NULL; //!< 获取的下载结果信息 
    ADM_DPS_DOWNPACKAGEFORMAT_E enPkgFormat = ADM_DPS_DOWNPACKAGE_PART;
    const DCHAR *pcPkgPath   = "/tmp/xxx.zip";

    //!< 将下载策略更新成集合包
    iRet = adm_dps_checkAndUpdateDPSDownInfo(CDpsAPITest::dps_ctorCheckNewDownloadPolicy,CDpsAPITest::dps_ctorCleanSlaveDownPackage);
    EXPECT_EQ(ADM_ECODE_SUCCESSED, iRet);

    iRet = module_readFile2Json(gpcDownEcuPolicyJsonPath, &policyRoot);
    if (ADM_IS_ERR(iRet))
    {
        cout << "failed to read file[" << gpcDownEcuPolicyJsonPath << "] to json, err[" << iRet << "]"<< endl;
        return;
    }
    //!< 获取更新后升级包格式
    iRet = adm_dps_EcuPackageFormat(&enPkgFormat);
    EXPECT_EQ(ADM_ECODE_SUCCESSED, iRet);
    EXPECT_EQ(ADM_DPS_DOWNPACKAGE_PART, enPkgFormat);

    //!< 构造下载元素属性信息
    adm_memzero(&stPkgAttr, sizeof(stPkgAttr));
    stPkgAttr.fileName = nullptr;
    stPkgAttr.baseSize   = 0;
    stPkgAttr.isRollback = TRUE;
    stPkgAttr.ecuReqid = adm_strdup(policyRoot["downPolicy"]["downEcuInfos"][0]["ecuReqId"].asString().c_str());
    EXPECT_NE(nullptr, stPkgAttr.ecuReqid);
    stPkgAttr.ecuSwDid = adm_strdup(policyRoot["downPolicy"]["downEcuInfos"][0]["ecuSwDid"].asString().c_str());
    EXPECT_NE(nullptr, stPkgAttr.ecuSwDid);
    stPkgAttr.enPkgFormat  = ADM_DPS_DOWNPACKAGE_PART;
    stPkgAttr.wholePkgSize = policyRoot["downPolicy"]["downEcuInfos"][0]["srcPkg"]["pkgFile"]["size"].asInt64() + 
                                policyRoot["downPolicy"]["downEcuInfos"][0]["dstPkg"]["pkgFile"]["size"].asInt64();

    //!< 构造下载结果 --- success
    stResult.result     = 0;
    stResult.eDownState = (ADM_IS_ERR(stResult.result) ? 
                            (ADM_DPS_DOWNPKG_STATE_FAILED) : (ADM_DPS_DOWNPKG_STATE_FINISH));
    stResult.failReason = (ADM_IS_ERR(stResult.result) ? 
                            adm_strdup("DownResult: err. (test)") : adm_strdup("DownResult: ok. (test)"));

    //!< 更新下载结果
    iRet = adm_dps_updateDownPackageResult(&stPkgAttr, &stResult, pcPkgPath);
    EXPECT_EQ(ADM_ECODE_SUCCESSED, iRet);

    //!< 获取下载进度信息
    pstDownPkgResult = adm_dps_getEcuDownloadResult();
    EXPECT_NE(nullptr, pstDownPkgResult);

    //!< 校验下载进度信息
    EXPECT_EQ(2, pstDownPkgResult->num);
    EXPECT_NE(nullptr, pstDownPkgResult->pstResultList);
    for (i = 0; i < pstDownPkgResult->num; i++)
    {
        if ((0 == adm_strcasecmp(stPkgAttr.ecuReqid, pstDownPkgResult->pstResultList[i].ecuReqid)) &&
            (0 == adm_strcasecmp(stPkgAttr.ecuSwDid, pstDownPkgResult->pstResultList[i].ecuSwDid)))
        {
            EXPECT_EQ(pstDownPkgResult->pstResultList[0].stResult.result,     stResult.result);
            EXPECT_EQ(pstDownPkgResult->pstResultList[0].stResult.eDownState, stResult.eDownState);
            EXPECT_STREQ(pstDownPkgResult->pstResultList[0].stResult.failReason, stResult.failReason);
            break;
        }
    }
    //!< 校验上面未找到的情况
    EXPECT_NE(i, pstDownPkgResult->num);

    adm_free(stResult.failReason);
    adm_dps_cleanPackageAttribute(&stPkgAttr);
    adm_dps_delDownPackageResult(&pstDownPkgResult);
    return;
}

TEST_F(CDpsAPITest, updateEcuPkgSavePath2PIS_Normal)
{
    /*
     *  更新后再获取出来看看获取到的是否与设置值一致
     */
    D32S iRet = ADM_ECODE_SUCCESSED;
    Json::Value policyRoot;
    ADM_DPS_PKGATTR_T stPkgAttr  = {0};
    ADM_COM_FILEOBJECT_T  stPkgFileObj = {0};
    ADM_COM_FILESCYINFO_T stFileSecurityInfo = {0};
    DCHAR *pcFilePath = "/tmp/xxx.zip";
    DCHAR *pcFileHash = "d1f97c0939b05e2734606e5f4a6561892412229c76fb0840e9d639defe34c63e";

    //!< 随便找个ecu
    iRet = module_readFile2Json(gpcDownEcuPolicyJsonPath, &policyRoot);
    if (ADM_IS_ERR(iRet))
    {
        cout << "failed to read file[" << gpcDownEcuPolicyJsonPath << "] to json, err[" << iRet << "]"<< endl;
        return;
    }

    //!< 构造数据
    stFileSecurityInfo.oriHash = "d1f97c0939b05e2734606e5f4a6561892412229c76fb0840e9d639defe34c63e";
    stFileSecurityInfo.pkgKey = "d1f97c0939b05e2734606e5f4a6561892412229c76fb0840e9d639defe34c63e";
    stFileSecurityInfo.isEncrypt = 0;
    stFileSecurityInfo.oriSize = 1000;
    stFileSecurityInfo.segmentDecryptLen = 0;
    stFileSecurityInfo.fileMetaNum = 0;
    stFileSecurityInfo.pstFileMetaList = NULL;

    stPkgFileObj.filePath = "/tmp/xxx.zip";
    stPkgFileObj.hash = "d1f97c0939b05e2734606e5f4a6561892412229c76fb0840e9d639defe34c63e";
    stPkgFileObj.size = 1000;
    stPkgFileObj.pstFileSecurityInfo = &stFileSecurityInfo;

    //!< 构造下载元素属性信息 
    adm_memzero(&stPkgAttr, sizeof(stPkgAttr));
    stPkgAttr.fileName = nullptr;
    stPkgAttr.ecuReqid = adm_strdup(policyRoot["downPolicy"]["downEcuInfos"][0]["ecuReqId"].asString().c_str());
    EXPECT_NE(nullptr, stPkgAttr.ecuReqid);
    stPkgAttr.ecuSwDid = adm_strdup(policyRoot["downPolicy"]["downEcuInfos"][0]["ecuSwDid"].asString().c_str());
    EXPECT_NE(nullptr, stPkgAttr.ecuSwDid);
    stPkgAttr.wholePkgSize = 1010;

    //!< 更新升级包存储路径 -- 内部调用 adm_pis_packageSet 具体成功与否由 adm_pis_packageSet 保证
    iRet = adm_dps_updateEcuPkgSavePath2PIS(stPkgAttr.ecuReqid, stPkgAttr.ecuSwDid, 
                TRUE, &stPkgFileObj);
    EXPECT_EQ(ADM_ECODE_SUCCESSED, iRet);

    adm_dps_cleanPackageAttribute(&stPkgAttr);

    return;
}

