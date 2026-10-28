/**
 * @file    adm_ecs_unittest.h
 * @brief   ecs module unit test  
 * @details ecs 模块单元测试用例
 * @author  wangyanpeng
 * @version v0.1.0
 * @date    2020-11-06
 * @copyright www.abupdate.com
 */
#ifndef __ADM_ECS_UNITTEST_H_
#define __ADM_ECS_UNITTEST_H_

// #ifdef __cplusplus
// extern "C++" {
// #endif

#include "adm_ecs_rsConfig.h"
#include "adm_ecs_parsePolicy.h"
#include "gtest/gtest.h"


class CEcsAPITest:public testing::Test
{
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    virtual void SetUp(){};
    virtual void TearDown(){};

    CEcsAPITest(){};
    ~CEcsAPITest(){};

    static D32S createCoreServiceRunDir(void);
    static void destroyCoreServiceRunDir(void);
    // 上层注册回调函数 
    static D32S ecs_regGetReqCloudParamAPI(ADM_ECS_REQPARAM_T *pstReqParam);
    static D32S ecs_regReqCloudAPi(const ADM_OCS_BUSINESS_TYPE_E eBusinessType, const DCHAR* reqMessageBody, DCHAR** rspMessageBody, DSIZE* rspStatusCode);
    static D32S ecs_regDownloadAPI(const DCHAR* downloadUrl, ADM_OCA_FILE_DOWNLOAD_INFO_T1* pstDownloadInfo);
};




// #ifdef __cplusplus
// }
// #endif

#endif//__ADM_ECS_UNITTEST_H_
