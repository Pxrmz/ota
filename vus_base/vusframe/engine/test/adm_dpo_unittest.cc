/**
 * @file    adm_dpo_unittest.cc
 * @brief   dpo module unit test  
 * @details dpo 模块单元测试用例
 * @author  wangyanpeng
 * @version v0.1.0
 * @date    2020-11-06
 * @copyright www.abupdate.com
 */
#include "adm_com_errorCode.h"
#include "adm_oca_fileStruct.h"
#include "adm_dpo_unittest.h"
#include "json/json.h"
#include "adm_string.h"
#include "adm_memory.h"


#include <fstream>
#include <iostream>

using namespace std;

/*!
 *******************************************************************************
 * \note 依赖外部模拟数据清单.
 *      ./data/getConfReqParam.json --- 构造下载请求参数
 *      ./data/getConfRsp.json      --- 构造下载的文件描述
 * 
 *******************************************************************************
 */
static const DCHAR *gpcConfReqJsonPath = "./data/getConfReqParam.json";
static const DCHAR *gpcConfRspJsonPath = "./data/getConfRsp.json";
static const DCHAR *gpcDownFileSuffix = "_gtest";

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
 * 构造下载请求参数.
 *
 * \note 
 *
 * \param[inout] pstDownReqInfo  下载请求参数
 * \retval 	     D32S
 *******************************************************************************
 */
static D32S ecs_ctorDownParam(ADM_DPO_DOWNLOAD_REQPARAM_T *pstDownReqInfo)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    Json::Value reqRoot;
    Json::Value rspRoot;
    string filePath = "";

    iRet = module_readFile2Json(gpcConfReqJsonPath, &reqRoot);
    if (ADM_IS_ERR(iRet))
    {
        cout << "failed to read file[" << gpcConfReqJsonPath << "] to json, err[" << iRet << "]"<< endl;
        return iRet;
    }

    iRet = module_readFile2Json(gpcConfRspJsonPath, &rspRoot);
    if (ADM_IS_ERR(iRet))
    {
        cout << "failed to read file[" << gpcConfRspJsonPath << "] to json, err[" << iRet << "]"<< endl;
        return iRet;
    }

    pstDownReqInfo->url = adm_strdup(rspRoot["data"]["fileObject"]["url"].asString().c_str());
    pstDownReqInfo->hash = adm_strdup(rspRoot["data"]["fileObject"]["hash"].asString().c_str());
    pstDownReqInfo->fileSize = rspRoot["data"]["fileObject"]["size"].asInt64();
    //!< 真实文件包的后缀加 "_gtest", 方便后面模拟下载时使用
    filePath = reqRoot["confFileDir"].asString() + rspRoot["data"]["fileObject"]["fileName"].asString() + gpcDownFileSuffix;
    pstDownReqInfo->filePath = adm_strdup(filePath.c_str());

    pstDownReqInfo->IsResumeDown   = 0;
    pstDownReqInfo->downRetriesCnt = 3;
    pstDownReqInfo->IsUnlimitretry = FALSE;

    return ADM_ECODE_SUCCESSED;
}

/*!
 *******************************************************************************
 * 构造dpo正常下载文件动作回调函数.
 *
 * \note 
 *
 * \param[in]	downloadUrl     下载url
 * \param[in]	pstDownloadInfo 下载消息
 * \retval 	    D32S
 *******************************************************************************
 */
D32S CDpoAPITest::ecs_regNormalDownloadAPI(const DCHAR* downloadUrl, ADM_OCA_FILE_DOWNLOAD_INFO_T* pstDownloadInfo)
{
    string dstFilePath = "";
    string srcFilePath = "";
    string command = "cp -f ";
    D32S i = 0;
    ADM_OCA_FILE_PROGRESS_T stProgress = {0};

    cout << "[----------] ===> Entry download API callback." << endl;
    EXPECT_NE(nullptr, pstDownloadInfo);
    EXPECT_NE(nullptr, pstDownloadInfo->savePath);
    for (i = 0; i < 1; i++)
    {
        stProgress.speed = i + 1;
        stProgress.savedSize = i + 1;
        stProgress.remainTime = i;
        pstDownloadInfo->cbDownloadProgress(&stProgress, pstDownloadInfo->priv);
        usleep(20 * 1000); // Y * 1000 = Y ms
    }

    srcFilePath = pstDownloadInfo->savePath;
    dstFilePath = pstDownloadInfo->savePath;

    // cout << "[----------] ===> 111 dstFilePath[" << dstFilePath << "]"<< endl;
    EXPECT_GT(dstFilePath.size(), strlen(gpcDownFileSuffix));
    dstFilePath.erase(dstFilePath.size() - strlen(gpcDownFileSuffix), strlen(gpcDownFileSuffix));
    // cout << "[----------] ===> 222 dstFilePath[" << srcFilePath << "]"<< endl;

    command += dstFilePath + " " + srcFilePath;
    // cout << "[----------] ===> 333 command[" << command << "]"<< endl;
    //!< 拷贝文件模拟下载
    system((DCHAR*)command.c_str());

    cout << "[----------] ===> download API callback ok." << endl;
    return ADM_ECODE_SUCCESSED;
}


/*!
 *******************************************************************************
 * 构造dpo重试下载文件动作回调函数.
 *
 * \note 该函数第一次调用返回下载失败，第二次会成功
 *
 * \param[in]	downloadUrl     下载url
 * \param[in]	pstDownloadInfo 下载消息
 * \retval 	    D32S
 *******************************************************************************
 */
D32S CDpoAPITest::ecs_regRetryDownloadAPI(const DCHAR* downloadUrl, ADM_OCA_FILE_DOWNLOAD_INFO_T* pstDownloadInfo)
{
    string dstFilePath = "";
    string srcFilePath = "";
    string command = "cp -f ";
    static D32S iIsSecondRuning = 0;
    ADM_OCA_FILE_PROGRESS_T stProgress = {0};

    // cout << "[----------] ===> Entry download API callback." << endl;
    EXPECT_NE(nullptr, pstDownloadInfo);
    EXPECT_NE(nullptr, pstDownloadInfo->savePath);

    if (0 == iIsSecondRuning)
    {
        iIsSecondRuning = 1;
        cout << "[----------] ===> First download return Failed!!!" << endl;
        cout << "[  sleep.  ] ===> Wait download Retry(10s) ..." << endl;
        return ADM_ECODE_FAILED;
    }
    cout << "[----------] ===> second download Retry Successed." << endl;

    stProgress.speed = 100;
    stProgress.savedSize = 100;
    stProgress.remainTime = 0;
    pstDownloadInfo->cbDownloadProgress(&stProgress, pstDownloadInfo->priv);
    
    srcFilePath = pstDownloadInfo->savePath;
    dstFilePath = pstDownloadInfo->savePath;
    // cout << "[----------] ===> 111 dstFilePath[" << dstFilePath << "]"<< endl;
    EXPECT_GT(dstFilePath.size(), strlen(gpcDownFileSuffix));
    dstFilePath.erase(dstFilePath.size() - strlen(gpcDownFileSuffix), strlen(gpcDownFileSuffix));
    // cout << "[----------] ===> 222 dstFilePath[" << srcFilePath << "]"<< endl;

    command += dstFilePath + " " + srcFilePath;
    // cout << "[----------] ===> 333 command[" << command << "]"<< endl;
    //!< 拷贝文件模拟下载
    system((DCHAR*)command.c_str());

    cout << "[----------] ===> download API callback ok." << endl;
    return ADM_ECODE_SUCCESSED;
}


/*!
 *******************************************************************************
 * 构造dpo下载动作进度通知回调函数.
 *
 * \note 
 *
 * \param[in]	pstNotifyInfo  下载进度信息
 * \param[in]	priv           私有参数
 * \retval 	    none
 *******************************************************************************
 */
void CDpoAPITest::ecs_regDownProgressNotifyAPI(ADM_DPO_PROGRESS_NOTIFY_T *pstNotifyInfo, void *priv)
{
    EXPECT_NE(nullptr, pstNotifyInfo);
    cout << "[----------] ===> download Progress Notify API ok." << endl;

    return;
}


#if 0
#endif


TEST_F(CDpoAPITest, downloadFile_Normal)
{
    string command = "rm -rf ";
    D32S iRet = ADM_ECODE_SUCCESSED;
    ADM_DPO_DOWNLOAD_REQPARAM_T stDownReqInfo = {0};

    //!< 构造正常下载请求参数
    iRet = ecs_ctorDownParam(&stDownReqInfo);
    EXPECT_EQ(ADM_ECODE_SUCCESSED, iRet);

    //!< 注册对应的回调处理函数
    stDownReqInfo.fnDownloadCb = CDpoAPITest::ecs_regNormalDownloadAPI;
    stDownReqInfo.fnProgressNotify = CDpoAPITest::ecs_regDownProgressNotifyAPI;

    //!< 下载相关文件
    iRet = adm_dpo_doDownloadFile(&stDownReqInfo);
    EXPECT_EQ(ADM_ECODE_SUCCESSED, iRet);

    command += stDownReqInfo.filePath;
    // cout << "[----------] ===> filePath[" << command << "]"<< endl;
    //!< 清除模拟下载的文件
    system((DCHAR*)command.c_str());
    //!< 清空请求参数 --- 内存回收
    adm_dpo_cleanDownReqParamInfo(&stDownReqInfo);
    return;
}


TEST_F(CDpoAPITest, downloadFile_InvalidParam)
{
    string command = "rm -rf ";
    D32S iRet = ADM_ECODE_SUCCESSED;
    ADM_DPO_DOWNLOAD_REQPARAM_T stDownReqInfo = {0};

    //!< 构造正常下载请求参数
    iRet = ecs_ctorDownParam(&stDownReqInfo);
    EXPECT_EQ(ADM_ECODE_SUCCESSED, iRet);

    //!< 注册对应的回调处理函数
    stDownReqInfo.fnDownloadCb = CDpoAPITest::ecs_regNormalDownloadAPI;
    stDownReqInfo.fnProgressNotify = CDpoAPITest::ecs_regDownProgressNotifyAPI;

    //!< 构造url非法
    adm_free(stDownReqInfo.url);
    //!< 下载相关文件
    iRet = adm_dpo_doDownloadFile(&stDownReqInfo);
    EXPECT_NE(ADM_ECODE_SUCCESSED, iRet);

    //!< 清空请求参数 --- 内存回收
    adm_dpo_cleanDownReqParamInfo(&stDownReqInfo);
    return;
}

TEST_F(CDpoAPITest, downloadFile_BreakPoints)
{
    string dstFilePath = "";
    D32S iRet = ADM_ECODE_SUCCESSED;
    ADM_DPO_DOWNLOAD_REQPARAM_T stDownReqInfo = {0};

    //!< 构造正常下载请求参数
    iRet = ecs_ctorDownParam(&stDownReqInfo);
    EXPECT_EQ(ADM_ECODE_SUCCESSED, iRet);

    //!< 开启断点继续下载
    stDownReqInfo.IsResumeDown = 1;
    //!< 将要下载的文件包名改为模拟包名，这样验证断点在下载完成是否成功
    dstFilePath = stDownReqInfo.filePath;
    EXPECT_GT(dstFilePath.size(), strlen(gpcDownFileSuffix));
    dstFilePath.erase(dstFilePath.size() - strlen(gpcDownFileSuffix), strlen(gpcDownFileSuffix));
    adm_free(stDownReqInfo.filePath);
    stDownReqInfo.filePath = adm_strdup(dstFilePath.c_str());
    // cout << "[----------] ===> filePath[" << stDownReqInfo.filePath << "]"<< endl;

    //!< 注册对应的回调处理函数
    stDownReqInfo.fnDownloadCb = CDpoAPITest::ecs_regNormalDownloadAPI;
    stDownReqInfo.fnProgressNotify = CDpoAPITest::ecs_regDownProgressNotifyAPI;

    //!< 下载相关文件
    iRet = adm_dpo_doDownloadFile(&stDownReqInfo);
    EXPECT_EQ(ADM_ECODE_SUCCESSED, iRet);

    //!< 清空请求参数 --- 内存回收
    adm_dpo_cleanDownReqParamInfo(&stDownReqInfo);
    return;
}

TEST_F(CDpoAPITest, downloadFile_Retry)
{
    string command = "rm -rf ";
    D32S iRet = ADM_ECODE_SUCCESSED;
    ADM_DPO_DOWNLOAD_REQPARAM_T stDownReqInfo = {0};

    //!< 构造正常下载请求参数
    iRet = ecs_ctorDownParam(&stDownReqInfo);
    EXPECT_EQ(ADM_ECODE_SUCCESSED, iRet);

    //!< 注册对应的回调处理函数
    stDownReqInfo.fnDownloadCb = CDpoAPITest::ecs_regRetryDownloadAPI;
    stDownReqInfo.fnProgressNotify = CDpoAPITest::ecs_regDownProgressNotifyAPI;

    //!< 下载相关文件
    iRet = adm_dpo_doDownloadFile(&stDownReqInfo);
    EXPECT_EQ(ADM_ECODE_SUCCESSED, iRet);

    command += stDownReqInfo.filePath;
    // cout << "[----------] ===> filePath[" << command << "]"<< endl;
    //!< 清除模拟下载的文件
    system((DCHAR*)command.c_str());
    //!< 清空请求参数 --- 内存回收
    adm_dpo_cleanDownReqParamInfo(&stDownReqInfo);
    return;
}


TEST_F(CDpoAPITest, downloadFile_BigFile)
{
    // TODO: 待添加大文件下载 --- 如何做呢 ???

    return;
}