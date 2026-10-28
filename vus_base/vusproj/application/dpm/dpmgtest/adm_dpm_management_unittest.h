/**
 * @file    adm_dpm_management_unittest.h
 * @brief   Download-Policy Management 
 * @details 模块单元测试用例
 * @author  wanglin
 * @version v0.1.0
 * @date    2022-11-30
 * @copyright www.abupdate.com
 */


// #ifdef __cplusplus
// extern "C++" {
// #endif

#ifndef __ADM_DPM_UNITTEST_H_
#define __ADM_DPM_UNITTEST_H_

#include "adm_typedefs.h"
#include "gtest/gtest.h"

class CDpmAPITest
{
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    //virtual void SetUp(){};
    //virtual void TearDown(){};

    CDpmAPITest(){};
    ~CDpmAPITest(){};
    
    static D32S createCoreServiceRunDir(void);
    static void destroyCoreServiceRunDir(void);
    static void setDownParameter(void);
    static void getdownTrigerType(D32S *pidownTrigerType);
    static D32S cpSrcfileToDstfile(DCHAR *pSrcfilePath,DCHAR *pDstfilePath);
    static D32S dpmOtaConfigInit(const DCHAR* filename);
};

class CDPM_TEST_Download:public testing::Test
{
protected:
    void SetUp() override 
    {
    }
    void TearDown() override 
    {
    }
};

class CDPM_TEST_DownloadEnd:public testing::Test
{
protected:
    void SetUp() override 
    {
    }
    void TearDown() override 
    {
    }
};

#endif //__ADM_DPM_UNITTEST_H_