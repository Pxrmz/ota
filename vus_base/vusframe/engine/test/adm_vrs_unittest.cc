/**
 * @file    adm_vrs_unittest.cc
 * @brief   vrs module unit test  
 * @details vrs 模块单元测试用例
 * @author  wangyanpeng
 * @version v0.1.0
 * @date    2020-11-06
 * @copyright www.abupdate.com
 */
#include "adm_com_errorCode.h"
#include "adm_engine_common.h"
#include "adm_vrs_registerInfo.h"
#include "adm_vrs_unittest.h"
#include "json/json.h"
#include "adm_string.h"
#include "adm_memory.h"


#include <fstream>
#include <iostream>

using namespace std;

/*!
 *******************************************************************************
 * \note 依赖外部模拟数据清单.
 *      ./data/getVrsReqParam.json --- 构造请求数据
 *      ./data/getVrsRsp.json      --- 构造返回的数据
 *      ./data/runDir/             --- 测试过程临时目录
 * 
 *******************************************************************************
 */
static const DCHAR *gpcVrsReqJsonPath = "./data/getVrsReqParam.json";
static const DCHAR *gpcVrsRspJsonPath = "./data/getVrsRsp.json";
#define CORESERVICE_RUNDIR  "./data/runDir/"

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

void CVrsAPITest::SetUpTestCase()
{
    D32S iRet = ADM_ECODE_SUCCESSED;

    iRet = CVrsAPITest::vrs_createCoreServiceRunDir();
    EXPECT_EQ(ADM_ECODE_SUCCESSED, iRet);
    
    // cout << "SetUpTestCase" << endl;
}

void CVrsAPITest::TearDownTestCase()
{
    CVrsAPITest::vrs_destroyCoreServiceRunDir();

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
D32S CVrsAPITest::vrs_createCoreServiceRunDir(void)
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
void CVrsAPITest::vrs_destroyCoreServiceRunDir(void)
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
 * 构造注册请求参数.
 *
 * \note 
 *
 * \param[inout] pstRegisterRequestInfo  注册请求参数
 * \retval 	     D32S
 *******************************************************************************
 */
D32S CVrsAPITest::vrs_ctorRegReqParam(ADM_OCS_REGISTER_REQUEST_INFO_T* pstRegisterRequestInfo)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    Json::Value reqRoot;

    iRet = module_readFile2Json(gpcVrsReqJsonPath, &reqRoot);
    if (ADM_IS_ERR(iRet))
    {
        cout << "failed to read file[" << gpcVrsReqJsonPath << "] to json, err[" << iRet << "]"<< endl;
        return iRet;
    }

    pstRegisterRequestInfo->timeStamp  = 0;
    pstRegisterRequestInfo->vinCode    = adm_strdup(reqRoot["Vin"].asString().c_str());
    pstRegisterRequestInfo->sdkVersion = adm_strdup(reqRoot["sdkVersion"].asString().c_str());
    pstRegisterRequestInfo->timeStamp  = reqRoot["timestamp"].asInt();
    //!< 注册请求云端回调函数
    pstRegisterRequestInfo->fnBusinessCommCb = CVrsAPITest::vrs_regReqCloudAPi;

    return ADM_ECODE_SUCCESSED;
}

/*!
 *******************************************************************************
 * 构造注册请求云端动作回调函数.
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
D32S CVrsAPITest::vrs_regReqCloudAPi(const ADM_OCS_BUSINESS_TYPE_E eBusinessType, const DCHAR* reqMessageBody, DCHAR** rspMessageBody, DSIZE* rspStatusCode)
{
    std::ifstream ifs;
    D32S length = 0;

    EXPECT_NE(nullptr, reqMessageBody);
    EXPECT_NE(nullptr, rspMessageBody);
    EXPECT_NE(nullptr, rspStatusCode);

    ifs.open(gpcVrsRspJsonPath, ifstream::in);
    if (!ifs.is_open())
    {
        cout << "failed to open filePath[" << gpcVrsRspJsonPath << "], err[" << strerror(errno) << "]" << endl;
        return ADM_ECODE_FAILED;
    }
    ifs.seekg(0, std::ios::end);
    length = ifs.tellg();
    ifs.seekg(0, std::ios::beg);

    *rspMessageBody = (DCHAR*)adm_malloc(sizeof(DCHAR) * (length + 1));
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

#if 0
#endif


TEST_F(CVrsAPITest, doRegister_InvalidParam)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    ADM_VRS_REGISTER_RSP_INFO_T *pstResp = NULL;
    ADM_OCS_REGISTER_REQUEST_INFO_T *pstReqInfo = NULL;

    pstResp = adm_vrs_newRegisterRspInfo();
    EXPECT_NE(nullptr, pstResp);
    adm_memzero(pstResp, sizeof(*pstResp));

    pstReqInfo = adm_ocs_newRegisterRequestInfo();
    EXPECT_NE(nullptr, pstReqInfo);
    adm_memzero(pstReqInfo, sizeof(*pstReqInfo));

    //!< 构造正常注册请求参数
    iRet = vrs_ctorRegReqParam(pstReqInfo);
    EXPECT_EQ(ADM_ECODE_SUCCESSED, iRet);

    //!< 执行注册 pstReqInfo = NULL 
    iRet = adm_vrs_doRegister(NULL, pstResp);
    EXPECT_NE(ADM_ECODE_SUCCESSED, iRet);

    //!< 执行注册
    adm_free(pstReqInfo->vinCode);
    iRet = adm_vrs_doRegister(pstReqInfo, pstResp);
    EXPECT_NE(ADM_ECODE_SUCCESSED, iRet);

    //!< 清空请求参数 --- 内存回收
    adm_vrs_delRegisterRspInfo(pstResp);
    adm_ocs_delRegisterRequestInfo(pstReqInfo);
    return;
}


TEST_F(CVrsAPITest, doRegister_Normal)
{
    Json::Value reqRoot;
    Json::Value rspRoot;
    D32S iRet = ADM_ECODE_SUCCESSED;
    ADM_VRS_REGISTER_INFO_T *pstRegisterInfo = NULL;
    ADM_VRS_REGISTER_RSP_INFO_T *pstResp = NULL;
    ADM_OCS_REGISTER_REQUEST_INFO_T *pstReqInfo = NULL;

    pstRegisterInfo = adm_vrs_newRegisterInfo();
    EXPECT_NE(nullptr, pstRegisterInfo);
    adm_memzero(pstRegisterInfo, sizeof(*pstRegisterInfo));

    pstResp = adm_vrs_newRegisterRspInfo();
    EXPECT_NE(nullptr, pstResp);
    adm_memzero(pstResp, sizeof(*pstResp));

    pstReqInfo = adm_ocs_newRegisterRequestInfo();
    EXPECT_NE(nullptr, pstReqInfo);
    adm_memzero(pstReqInfo, sizeof(*pstReqInfo));

    //!< 构造正常注册请求参数
    iRet = vrs_ctorRegReqParam(pstReqInfo);
    EXPECT_EQ(ADM_ECODE_SUCCESSED, iRet);

    //!< 执行注册
    iRet = adm_vrs_doRegister(pstReqInfo, pstResp);
    EXPECT_EQ(ADM_ECODE_SUCCESSED, iRet);

    //!< 获取注册信息、获取模拟的注册信息
    iRet = adm_vrs_getRegisterInfo(pstRegisterInfo);
    EXPECT_EQ(ADM_ECODE_SUCCESSED, iRet);
    iRet = module_readFile2Json(gpcVrsReqJsonPath, &reqRoot);
    EXPECT_EQ(ADM_ECODE_SUCCESSED, iRet);
    iRet = module_readFile2Json(gpcVrsRspJsonPath, &rspRoot);
    EXPECT_EQ(ADM_ECODE_SUCCESSED, iRet);

    //!< 校验注册结果
    EXPECT_STREQ(pstRegisterInfo->vinCode, reqRoot["Vin"].asString().c_str());
    EXPECT_STREQ(pstRegisterInfo->secret, rspRoot["data"]["secret"].asString().c_str());
    EXPECT_STREQ(pstRegisterInfo->deviceId, rspRoot["data"]["deviceId"].asString().c_str());
    
    //!< 清空请求参数 --- 内存回收
    adm_vrs_delRegisterInfo(pstRegisterInfo);
    adm_vrs_delRegisterRspInfo(pstResp);
    adm_ocs_delRegisterRequestInfo(pstReqInfo);
    return;
}

TEST_F(CVrsAPITest, isRegisteredOK_Normal)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    Json::Value reqRoot;
    string vin = "";
    ADM_VRS_REGISTER_RSP_INFO_T *pstResp = NULL;
    ADM_OCS_REGISTER_REQUEST_INFO_T *pstReqInfo = NULL;

    iRet = module_readFile2Json(gpcVrsReqJsonPath, &reqRoot);
    EXPECT_EQ(ADM_ECODE_SUCCESSED, iRet);

    vin = reqRoot["Vin"].asString();
    vin += "_xxxx";

    //!< 构造非法的 vin 校验 adm_vrs_isRegisteredOK 接口
    iRet = adm_vrs_isRegisteredOK((DCHAR *)vin.c_str());
    EXPECT_NE(ADM_ECODE_SUCCESSED, iRet);

    //!< 正常注册后，检验 adm_vrs_isRegisteredOK 接口
    pstResp = adm_vrs_newRegisterRspInfo();
    EXPECT_NE(nullptr, pstResp);
    adm_memzero(pstResp, sizeof(*pstResp));

    pstReqInfo = adm_ocs_newRegisterRequestInfo();
    EXPECT_NE(nullptr, pstReqInfo);
    adm_memzero(pstReqInfo, sizeof(*pstReqInfo));

    //!< 构造正常注册请求参数
    iRet = vrs_ctorRegReqParam(pstReqInfo);
    EXPECT_EQ(ADM_ECODE_SUCCESSED, iRet);

    //!< 执行注册
    iRet = adm_vrs_doRegister(pstReqInfo, pstResp);
    EXPECT_EQ(ADM_ECODE_SUCCESSED, iRet);

    //!< 正常 vin 校验 adm_vrs_isRegisteredOK 接口
    vin = reqRoot["Vin"].asString();
    iRet = adm_vrs_isRegisteredOK((DCHAR *)vin.c_str());
    EXPECT_EQ(ADM_ECODE_SUCCESSED, iRet);

    adm_vrs_delRegisterRspInfo(pstResp);
    adm_ocs_delRegisterRequestInfo(pstReqInfo);

    return;
}
