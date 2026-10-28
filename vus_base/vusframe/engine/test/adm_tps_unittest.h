/**
 * @file    adm_tps_unittest.h
 * @brief   tps module unit test  
 * @details tps 模块单元测试用例
 * @author  wangyanpeng
 * @version v0.1.0
 * @date    2020-11-06
 * @copyright www.abupdate.com
 */
#ifndef __ADM_TPS_UNITTEST_H_
#define __ADM_TPS_UNITTEST_H_

// #ifdef __cplusplus
// extern "C++" {
// #endif

#include "adm_tps_transferService.h"
// #include "adm_ocs_airProtocolAdapter.h"
#include "gtest/gtest.h"



class CTpsAPITest:public testing::Test
{
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    virtual void SetUp(){};
    virtual void TearDown(){};

    CTpsAPITest(){};
    ~CTpsAPITest(){};

    static D32S createCoreServiceRunDir(void);
    static void destroyCoreServiceRunDir(void);

    // static D32S ecs_regGetReqCloudParamAPI(ADM_ECS_REQPARAM_T *pstReqParam);
    // static D32S ecs_regReqCloudAPi(const ADM_OCS_BUSINESS_TYPE_E eBusinessType, const DCHAR* reqMessageBody, DCHAR** rspMessageBody, DSIZE* rspStatusCode);
    // static D32S ecs_regDownloadAPI(const DCHAR* downloadUrl, ADM_OCA_FILE_DOWNLOAD_INFO_T* pstDownloadInfo);
    // static D32S dps_ctorCheckDownloadPolicy(ADM_CVS_VERSION_POLICY_T *pstVersionPolicy);
    // static D32S dps_ctorGetCombinePkgFileObj(ADM_COM_FILEOBJECT_T *pstPkgFileObj);
    // static D32S dps_ctorCheckNewDownloadPolicy(ADM_CVS_VERSION_POLICY_T *pstVersionPolicy);
    // static D32S dps_updateDownCombinePackageCfgInfo(void);
};




// #ifdef __cplusplus
// }
// #endif

#endif  //__ADM_TPS_UNITTEST_H_