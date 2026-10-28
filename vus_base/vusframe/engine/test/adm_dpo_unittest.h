/**
 * @file    adm_dpo_unittest.h
 * @brief   dpo module unit test  
 * @details dpo 模块单元测试用例
 * @author  wangyanpeng
 * @version v0.1.0
 * @date    2020-11-06
 * @copyright www.abupdate.com
 */
#ifndef __ADM_DPO_UNITTEST_H_
#define __ADM_DPO_UNITTEST_H_

// #ifdef __cplusplus
// extern "C++" {
// #endif

#include "adm_dpo_download.h"
#include "gtest/gtest.h"



class CDpoAPITest:public testing::Test
{
public:
    static void SetUpTestCase(){};
    static void TearDownTestCase(){};
    virtual void SetUp(){};
    virtual void TearDown(){};

    CDpoAPITest(){};
    ~CDpoAPITest(){};

    static D32S ecs_regNormalDownloadAPI(const DCHAR* downloadUrl, ADM_OCA_FILE_DOWNLOAD_INFO_T* pstDownloadInfo);
    static D32S ecs_regRetryDownloadAPI(const DCHAR* downloadUrl, ADM_OCA_FILE_DOWNLOAD_INFO_T* pstDownloadInfo);
    static void ecs_regDownProgressNotifyAPI(ADM_DPO_PROGRESS_NOTIFY_T *pstNotifyInfo, void *priv);
};




// #ifdef __cplusplus
// }
// #endif

#endif  //__ADM_DPO_UNITTEST_H_