/**
 * @file    adm_dps_unittest.h
 * @brief   dps module unit test  
 * @details dps 模块单元测试用例
 * @author  wangyanpeng
 * @version v0.1.0
 * @date    2020-11-06
 * @copyright www.abupdate.com
 */
#ifndef __ADM_DPS_UNITTEST_H_
#define __ADM_DPS_UNITTEST_H_

// #ifdef __cplusplus
// extern "C++" {
// #endif

#include "adm_dps_downloadService.h"
#include "adm_cvs_versionPolicy.h"
#include "gtest/gtest.h"



class CDpsAPITest:public testing::Test
{
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    virtual void SetUp(){};
    virtual void TearDown(){};

    CDpsAPITest(){};
    ~CDpsAPITest(){};

    static D32S createCoreServiceRunDir(void);
    static void destroyCoreServiceRunDir(void);
    static D32S dps_ctorCheckDownloadPolicy(ADM_CVS_VERSION_POLICY_T *pstVersionPolicy);
    static D32S dps_ctorCheckNewDownloadPolicy(ADM_CVS_VERSION_POLICY_T *pstVersionPolicy);
    static D32S dps_ctorGetCombinePkgFileObj(ADM_COM_FILEOBJECT_T *pstPkgFileObj);
    static void dps_ctorCleanSlaveDownPackage(void);
    static D32S dps_ctorUpdateDownCombinePackageCfgInfo(void);

};




// #ifdef __cplusplus
// }
// #endif

#endif  //__ADM_DPS_UNITTEST_H_