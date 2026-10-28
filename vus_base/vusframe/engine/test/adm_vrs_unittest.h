/**
 * @file    adm_vrs_unittest.h
 * @brief   vrs module unit test  
 * @details vrs 模块单元测试用例
 * @author  wangyanpeng
 * @version v0.1.0
 * @date    2020-11-06
 * @copyright www.abupdate.com
 */
#ifndef __ADM_VRS_UNITTEST_H_
#define __ADM_VRS_UNITTEST_H_

// #ifdef __cplusplus
// extern "C++" {
// #endif

#include "adm_vrs_registerManager.h"
#include "adm_ocs_airProtocolAdapter.h"
#include "gtest/gtest.h"



class CVrsAPITest:public testing::Test
{
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    virtual void SetUp(){};
    virtual void TearDown(){};

    CVrsAPITest(){};
    ~CVrsAPITest(){};

    static D32S vrs_createCoreServiceRunDir(void);
    static void vrs_destroyCoreServiceRunDir(void);
    static D32S vrs_ctorRegReqParam(ADM_OCS_REGISTER_REQUEST_INFO_T* pstRegisterRequestInfo);
    static D32S vrs_regReqCloudAPi(const ADM_OCS_BUSINESS_TYPE_E eBusinessType, const DCHAR* reqMessageBody, DCHAR** rspMessageBody, DSIZE* rspStatusCode);
};




// #ifdef __cplusplus
// }
// #endif

#endif  //__ADM_VRS_UNITTEST_H_