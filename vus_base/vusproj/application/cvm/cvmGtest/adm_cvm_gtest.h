

#ifndef __ADM_CVM_GTEST_H_
#define __ADM_CVM_GTEST_H_
#include "gtest/gtest.h"
#include "adm_typedefs.h"

#define ADM_CVM_TEST_LOGFILE_MAX_LENGTH              (5*1024*1024)          // 最大Log文件大小
#define ADM_CVM_TEST_LOGFILE_MAX_NUM                 (5)          // 最多Log文件数目

#define ADM_CVM_GTEST_CHECK_REQ_FILE                 "checkReq.json"   //!< 检测请求文件
#define ADM_CVM_GTEST_CHECK_EIM_CONF_FILE            "eimConfig.json"   //!< 检测请求文件
#define ADM_CVM_GTEST_CHECK_RES_SUCCESSED_FILE       "checkResSuccessed.json"   //!< 检测请求文件
#define ADM_CVM_GTEST_CHECK_RES_FAILED_FILE          "checkResFailed.json"   //!< 检测请求文件

#define ADM_CVM_TEST_CFGFILEDIR          "./filecfg"          //!< 测试配置文件所在目录
#define ADM_CVM_TEST_LOGDIR              "./debug"            //!< log文件输出目录
#define ADM_CVM_TEST_UCMCONFIG_FILE      "ucmconfig.ini"      //!< UCM配置信息文件

class CCVMTest
{
public:
    CCVMTest();
    ~CCVMTest();
public:
    void adm_cvm_check_test();
};

class adm_cvm_check_gtest:public testing::Test
{
protected:
    void SetUp() override 
    {
    }
    void TearDown() override 
    {
    }
};

#endif //__ADM_CVM_GTEST_H_