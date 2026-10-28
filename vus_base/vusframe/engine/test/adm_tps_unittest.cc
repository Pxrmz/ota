/**
 * @file    adm_tps_unittest.cc
 * @brief   tps module unit test  
 * @details tps 模块单元测试用例
 * @author  wangyanpeng
 * @version v0.1.0
 * @date    2020-11-06
 * @copyright www.abupdate.com
 */
#include "adm_com_errorCode.h"
#include "adm_com_lib.h"
// #include "adm_vrs_registerInfo.h"
#include "adm_tps_unittest.h"
#include "adm_ecs_unittest.h"
#include "adm_dps_unittest.h"
#include "adm_ecs_rsConfig.h"
#include "adm_dps_downloadService.h"
#include "adm_cvs_versionPolicy.h"
#include "json/json.h"
#include "adm_string.h"
#include "adm_memory.h"


#include <fstream>
#include <iostream>

using namespace std;

/*!
 *******************************************************************************
 * \note 依赖外部模拟数据清单.
 *      adm_ecs_unittest.h --- CEcsAPITest 类构造的数据
 *          |--- ecs_regGetReqCloudParamAPI()
 *          |--- ecs_regReqCloudAPi()
 *          |--- ecs_regDownloadAPI()
 *      adm_dps_unittest.h --- CDpsAPITest 类构造的数据
 *          |--- createCoreServiceRunDir()
 *          |--- destroyCoreServiceRunDir()
 *          |--- dps_ctorCheckDownloadPolicy()
 *          |--- dps_ctorCheckNewDownloadPolicy()
 *          |--- dps_ctorUpdateDownCombinePackageCfgInfo()
 * 
 *******************************************************************************
 */


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

/*!
 *******************************************************************************
 * 比较是否需要传包函数
 *
 * \note pcDownloadInstanceId != pcinstallInstanceId 时 使用 transferInstanceId 分包
 * 
 * \param[in]  pcDownloadInstanceId  升级包的下载实例id
 * \param[in]  pcinstallInstanceId   升级包的安装实例id
 * \retval     DBOOL                 true: 需要传输, false: 不需要传输
 *******************************************************************************
 */
static DBOOL adm_tpm_cmpIsNeedTrans(DCHAR *pcDownloadInstanceId, DCHAR *pcinstallInstanceId)
{
    D32S iRet = ADM_ECODE_SUCCESSED;

    if (ADM_IS_NULL(pcDownloadInstanceId) || ADM_IS_NULL(pcinstallInstanceId))
    {
        return FALSE;
    }

    //!< 下载实例id != 安装实例id
    if (0 != adm_strcasecmp(pcDownloadInstanceId, pcinstallInstanceId))
    {
        return TRUE;
    }

    return FALSE;
}


#if 0
#endif

void CTpsAPITest::SetUpTestCase()
{
    D32S iRet = ADM_ECODE_SUCCESSED;

    //!< tps 依赖的初始化
    iRet = CDpsAPITest::createCoreServiceRunDir();
    EXPECT_EQ(ADM_ECODE_SUCCESSED, iRet);
    iRet = adm_ecs_moduleInit(CEcsAPITest::ecs_regGetReqCloudParamAPI, 
                              CEcsAPITest::ecs_regReqCloudAPi, 
                              CEcsAPITest::ecs_regDownloadAPI,
                              NULL,
                              NULL);
    EXPECT_EQ(ADM_ECODE_SUCCESSED, iRet);
    // cout << "[----------] ecs_moduleInit ok" << endl;
    iRet = adm_ecs_updateAndGetConfigInfo(NULL);
    EXPECT_EQ(ADM_ECODE_SUCCESSED, iRet);
    // cout << "[----------] ecs_updateConfInfo ok" << endl;
    iRet = adm_dps_moduleInit(NULL);
    EXPECT_EQ(ADM_ECODE_SUCCESSED, iRet);
    // cout << "[----------] dps_moduleInit ok" << endl;
    iRet = adm_dps_checkAndUpdateDPSDownInfo(CDpsAPITest::dps_ctorCheckDownloadPolicy,CDpsAPITest::dps_ctorCleanSlaveDownPackage);
    EXPECT_EQ(ADM_ECODE_SUCCESSED, iRet);
    // cout << "[----------] dps_updateConfInfo ok" << endl;
    iRet = CDpsAPITest::dps_ctorUpdateDownCombinePackageCfgInfo();
    EXPECT_EQ(ADM_ECODE_SUCCESSED, iRet);
    // cout << "[----------] dps_updateDownCombinePackageCfgInfo ok" << endl;

    //!< tps 的初始化
    iRet = adm_tps_moduleInit(adm_tpm_cmpIsNeedTrans, NULL);
    EXPECT_EQ(ADM_ECODE_SUCCESSED, iRet);
    // cout << "[----------] tps_moduleInit ok" << endl;
    iRet = adm_tps_checkAndUpdateTPSTransInfo(adm_ecs_getConfigInfo, 
                                              adm_dps_getDownloadTaskInfo, 
                                              adm_dps_getTransCondition, 
                                              adm_dps_getDownloadEcuPackageListInfo);
    EXPECT_EQ(ADM_ECODE_SUCCESSED, iRet);
    // cout << "[----------] tps_updateConfInfo ok" << endl;

    // cout << "SetUpTestCase" << endl;
}

void CTpsAPITest::TearDownTestCase()
{
    adm_tps_moduleDeInit();
    adm_ecs_moduleDeInit();
    adm_dps_moduleDeInit();
    CDpsAPITest::destroyCoreServiceRunDir();

    // cout<<"TearDownTestCase"<<endl;
}


#if 0
#endif


TEST_F(CTpsAPITest, moduleInit_Normal)
{
    D32S iRet = ADM_ECODE_SUCCESSED;

    iRet = adm_tps_moduleInit(NULL, NULL);
    EXPECT_NE(ADM_ECODE_SUCCESSED, iRet);

    iRet = adm_tps_moduleInit(adm_tpm_cmpIsNeedTrans, NULL);
    EXPECT_EQ(ADM_ECODE_SUCCESSED, iRet);

    return;
}


TEST_F(CTpsAPITest, getWaitTransferEcuNum_Normal)
{
    D32S i32WaitTransEcuNum = 0;
    D32S iRet = ADM_ECODE_SUCCESSED;

    iRet = adm_tps_getWaitTransferEcuNum(&i32WaitTransEcuNum);
    EXPECT_EQ(ADM_ECODE_SUCCESSED, iRet);
    EXPECT_EQ(1, i32WaitTransEcuNum);

    return;
}

TEST_F(CTpsAPITest, getToBeTransfEcuList_Normal)
{
    D32S tobeTransNum = 0;
    D32S iRet = ADM_ECODE_SUCCESSED;
    ADM_TPS_TRANSFERPKG_T *pstTransPkgList = nullptr;

    pstTransPkgList = adm_tps_newTransferEcuPackageList(3);
    EXPECT_NE(nullptr, pstTransPkgList);

    iRet = adm_tps_getTobeTransferEcuPackageInfo(3, DFALSE, pstTransPkgList, &tobeTransNum);
    EXPECT_EQ(ADM_ECODE_SUCCESSED, iRet);
    EXPECT_EQ(1, tobeTransNum);

    adm_tps_cleanTransEcuPackageList((D32U)3, pstTransPkgList);
    adm_tps_delTransferEcuPackageList((D32U)3, &pstTransPkgList);

    return;
}

TEST_F(CTpsAPITest, checkAndUpdatePolicy_Normal)
{
    D32S tobeTransNum = 0;
    D32S iRet = ADM_ECODE_SUCCESSED;
    ADM_TPS_TRANSFERPKG_T *pstOldTransPkgList = nullptr;
    ADM_TPS_TRANSFERPKG_T *pstNewTransPkgList = nullptr;

    //!< 先获取老的待传输ecu信息 
    pstOldTransPkgList = adm_tps_newTransferEcuPackageList(3);
    EXPECT_NE(nullptr, pstOldTransPkgList);
    iRet = adm_tps_getTobeTransferEcuPackageInfo(3, DFALSE, pstOldTransPkgList, &tobeTransNum);
    EXPECT_EQ(ADM_ECODE_SUCCESSED, iRet);
    EXPECT_EQ(1, tobeTransNum);

    //!< 先将dps的策略更新 
    iRet = adm_dps_checkAndUpdateDPSDownInfo(CDpsAPITest::dps_ctorCheckNewDownloadPolicy,CDpsAPITest::dps_ctorCleanSlaveDownPackage);
    EXPECT_EQ(ADM_ECODE_SUCCESSED, iRet);
    //!< 更新 tps 策略
    iRet = adm_tps_checkAndUpdateTPSTransInfo(adm_ecs_getConfigInfo, 
                                              adm_dps_getDownloadTaskInfo, 
                                              adm_dps_getTransCondition, 
                                              adm_dps_getDownloadEcuPackageListInfo);
    EXPECT_EQ(ADM_ECODE_SUCCESSED, iRet);

    //!< 先获取新的待传输ecu信息 
    pstNewTransPkgList = adm_tps_newTransferEcuPackageList(3);
    EXPECT_NE(nullptr, pstNewTransPkgList);
    tobeTransNum = 0;
    iRet = adm_tps_getTobeTransferEcuPackageInfo(3, DFALSE, pstNewTransPkgList, &tobeTransNum);
    EXPECT_EQ(ADM_ECODE_SUCCESSED, iRet);
    EXPECT_EQ(1, tobeTransNum);

    //!< 两次获取的不一致则认为配置更新了
    EXPECT_STRNE(pstOldTransPkgList->stDstPkgInfo.hash, pstNewTransPkgList->stDstPkgInfo.hash);
    EXPECT_STRNE(pstOldTransPkgList->stDstPkgInfo.fileName, pstNewTransPkgList->stDstPkgInfo.fileName);
    adm_tps_delTransferEcuPackageList((D32U)3, &pstOldTransPkgList);
    adm_tps_delTransferEcuPackageList((D32U)3, &pstNewTransPkgList);

    //!< 最后在更新会集合包的策略，后续测试使用集合包的策略
    iRet = adm_dps_checkAndUpdateDPSDownInfo(CDpsAPITest::dps_ctorCheckDownloadPolicy,CDpsAPITest::dps_ctorCleanSlaveDownPackage);
    EXPECT_EQ(ADM_ECODE_SUCCESSED, iRet);
    iRet = CDpsAPITest::dps_ctorUpdateDownCombinePackageCfgInfo();
    EXPECT_EQ(ADM_ECODE_SUCCESSED, iRet);
    //!< 更新 tps 策略
    iRet = adm_tps_checkAndUpdateTPSTransInfo(adm_ecs_getConfigInfo, 
                                              adm_dps_getDownloadTaskInfo, 
                                              adm_dps_getTransCondition, 
                                              adm_dps_getDownloadEcuPackageListInfo);
    EXPECT_EQ(ADM_ECODE_SUCCESSED, iRet);

    return;
}

TEST_F(CTpsAPITest, getTransfCondition_Normal)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    ADM_DPS_TRANSFERCONDITION_T *pstTransCond = nullptr;

    pstTransCond = adm_dps_newTransferConditionInfo();
    EXPECT_NE(nullptr, pstTransCond);

    iRet = adm_tps_getTransferCondition(pstTransCond);
    EXPECT_EQ(ADM_ECODE_SUCCESSED, iRet);
    EXPECT_EQ(2, pstTransCond->trigNum);

    adm_dps_cleanTransferCondition(pstTransCond);
    adm_dps_deleteTransferConditionInfo(&pstTransCond);
    return;
}

TEST_F(CTpsAPITest, updateTransfProgress_Normal)
{
    D32S tobeTransNum = 0;
    D32S iRet = ADM_ECODE_SUCCESSED;
    DCHAR *fileId = nullptr;
    ADM_TPS_TRANSFERPKG_T *pstTransPkgList = nullptr;
    ADM_TPS_TRANSPROGRESS_T stProgress = {0};
    ADM_TPS_TRANSPKGPROGRESS_T *psTransPkgProgress = nullptr;

    //!< 先获取老的待传输ecu信息 
    pstTransPkgList = adm_tps_newTransferEcuPackageList(3);
    EXPECT_NE(nullptr, pstTransPkgList);
    iRet = adm_tps_getTobeTransferEcuPackageInfo(3, DFALSE, pstTransPkgList, &tobeTransNum);
    EXPECT_EQ(ADM_ECODE_SUCCESSED, iRet);
    EXPECT_EQ(1, tobeTransNum);

    //!< 构造ecu信息及进度 
    fileId = adm_tps_createTransFileIDByEcuInfo(pstTransPkgList[0].ecuReqid, 
                pstTransPkgList[0].ecuSwDid, FALSE, pstTransPkgList[0].stDstPkgInfo.fileName);
    EXPECT_NE(nullptr, fileId);
    // cout << "[----------] fileName " << pstTransPkgList[0].stDstPkgInfo.fileName << endl;
    // cout << "[----------] fileId " << fileId << endl;

    stProgress.speed = 100;
    stProgress.totalTime  = 1010;
    stProgress.remainTime = 10;
    stProgress.saveBytes  = pstTransPkgList[0].stDstPkgInfo.size;
    stProgress.totalBytes = pstTransPkgList[0].stDstPkgInfo.size;
    
    //!< 更新进度信息
    iRet = adm_tps_updateTransPackageProgress(fileId, &stProgress);
    EXPECT_EQ(ADM_ECODE_SUCCESSED, iRet);

    //!< todo: 获取传输进度信息 
    psTransPkgProgress = adm_dps_getEcuTransProgress();
    EXPECT_NE(nullptr, psTransPkgProgress);
    EXPECT_EQ(1, psTransPkgProgress->num);
    EXPECT_NE(nullptr, psTransPkgProgress->pstProgressList);

    EXPECT_STREQ(psTransPkgProgress->pstProgressList[0].ecuReqid, pstTransPkgList[0].ecuReqid);
    EXPECT_STREQ(psTransPkgProgress->pstProgressList[0].ecuSwDid, pstTransPkgList[0].ecuSwDid);
    EXPECT_EQ(psTransPkgProgress->pstProgressList[0].stProgress.speed,      stProgress.speed);
    EXPECT_EQ(psTransPkgProgress->pstProgressList[0].stProgress.totalTime,  stProgress.totalTime);
    EXPECT_EQ(psTransPkgProgress->pstProgressList[0].stProgress.remainTime, stProgress.remainTime);
    EXPECT_EQ(psTransPkgProgress->pstProgressList[0].stProgress.saveBytes,  stProgress.saveBytes);
    EXPECT_EQ(psTransPkgProgress->pstProgressList[0].stProgress.totalBytes, 
        stProgress.totalBytes + ((NULL != pstTransPkgList[0].pstSrcPkgInfo) ? (pstTransPkgList[0].pstSrcPkgInfo->size) : (0)));
    // cout << "[----------] getProcess " << psTransPkgProgress->pstProgressList[0].stProgress.totalBytes << endl;
    // cout << "[----------] ctorProcess(src) " << ((NULL != pstTransPkgList[0].pstSrcPkgInfo) ? (pstTransPkgList[0].pstSrcPkgInfo->fileSize) : (0)) << endl;
    // cout << "[----------] ctorProcess(dst) " << stProgress.totalBytes << endl;

    adm_tps_freeTransFileID(&fileId);
    adm_tps_delTransPackageProgress(&psTransPkgProgress);
    adm_tps_delTransferEcuPackageList((D32U)3, &pstTransPkgList);
    return;
}

TEST_F(CTpsAPITest, updateTransfResult_Normal)
{
    D32S tobeTransNum = 0;
    D32S iRet = ADM_ECODE_SUCCESSED;
    DCHAR *fileId = nullptr;
    ADM_TPS_TRANSRESULT_T stResult = {0};
    ADM_TPS_TRANSFERPKG_T    *pstTransPkgList   = nullptr;
    ADM_TPS_TRANSPKGRESULT_T *pstTransPkgResult = nullptr;
    const DCHAR *pcPkgPath   = "/tmp/xxx.zip";

    //!< 先将dps的策略更新 --- 用dps的零件包验证，和进度交叉验证dps
    iRet = adm_dps_checkAndUpdateDPSDownInfo(CDpsAPITest::dps_ctorCheckNewDownloadPolicy,CDpsAPITest::dps_ctorCleanSlaveDownPackage);
    EXPECT_EQ(ADM_ECODE_SUCCESSED, iRet);
    //!< 更新 tps 策略
    iRet = adm_tps_checkAndUpdateTPSTransInfo(adm_ecs_getConfigInfo, 
                                              adm_dps_getDownloadTaskInfo, 
                                              adm_dps_getTransCondition, 
                                              adm_dps_getDownloadEcuPackageListInfo);
    EXPECT_EQ(ADM_ECODE_SUCCESSED, iRet);

    //!< 先获取新的待传输ecu信息 
    pstTransPkgList = adm_tps_newTransferEcuPackageList(3);
    EXPECT_NE(nullptr, pstTransPkgList);
    iRet = adm_tps_getTobeTransferEcuPackageInfo(3, DFALSE, pstTransPkgList, &tobeTransNum);
    EXPECT_EQ(ADM_ECODE_SUCCESSED, iRet);
    EXPECT_EQ(1, tobeTransNum);

    //!< 构造ecu信息及进度 
    fileId = adm_tps_createTransFileIDByEcuInfo(pstTransPkgList[0].ecuReqid, 
                pstTransPkgList[0].ecuSwDid, FALSE, pstTransPkgList[0].stDstPkgInfo.fileName);
    EXPECT_NE(nullptr, fileId);
    // cout << "[----------] fileName " << pstTransPkgList[0].stDstPkgInfo.fileName << endl;
    // cout << "[----------] fileId " << fileId << endl;

    stResult.result      = 0;
    stResult.eTransState = (ADM_IS_ERR(stResult.result) ? 
                            (ADM_TPS_TRANSPKG_STATE_FAILED) : (ADM_TPS_TRANSPKG_STATE_FINISH));
    stResult.failReason  = (ADM_IS_ERR(stResult.result) ? 
                            adm_strdup("TransResult: err. (test)") : adm_strdup("TransResult: ok. (test)"));

    //!< 更新分包结果
    iRet = adm_tps_updateTransPackageResult(fileId, &stResult, pcPkgPath);
    EXPECT_EQ(ADM_ECODE_SUCCESSED, iRet);
    //!< 获取分包结果
    pstTransPkgResult = adm_tps_getEcuTransResult();
    EXPECT_NE(nullptr, pstTransPkgResult);
    //!< 校验分包结果信息
    EXPECT_EQ(1, pstTransPkgResult->num);
    EXPECT_NE(nullptr, pstTransPkgResult->pstResultList);
    EXPECT_STREQ(pstTransPkgResult->pstResultList[0].ecuReqid, pstTransPkgList[0].ecuReqid);
    EXPECT_STREQ(pstTransPkgResult->pstResultList[0].ecuSwDid, pstTransPkgList[0].ecuSwDid);
    EXPECT_EQ(pstTransPkgResult->pstResultList[0].stResult.result,     stResult.result);
    //!< 只有源包和目标包均传输完成才会更改传输状态
    // EXPECT_EQ(pstTransPkgResult->pstResultList[0].stResult.eTransState, stResult.eTransState);
    EXPECT_STREQ(pstTransPkgResult->pstResultList[0].stResult.failReason, stResult.failReason);

    adm_free(stResult.failReason);
    adm_tps_freeTransFileID(&fileId);
    adm_tps_delTransPackageResult(&pstTransPkgResult);
    adm_tps_delTransferEcuPackageList((D32U)3, &pstTransPkgList);
    return;
}

