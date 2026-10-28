#ifndef __ADM_CVM_PARSEJSON_H_
#define __ADM_CVM_PARSEJSON_H_
#include "adm_typedefs.h"
#include "json-c/json.h"
#include "adm_error.h"
#include "adm_com_errorCode.h"

#define RETURN_IF_NULL(p, ret)  if(NULL == (p))\
                                {\
                                    return ret;\
                                }
#define RETURN_IF_NEQ(a, b, ret)    if((a) != (b))\
                                    {\
                                        return ret;\
                                    }

#if !defined(LOG_TAG)
#define LOG_TAG             "TEST"         /*!< 宏函数，用于通过模块名的枚举值类型获取模块名称     */
#endif                        
                                
#include "adm_debug.h"

#define ADM_CVM_GTEST_FILE_MAX_LEN                    256                      // 文件路径最大长度
#define ADM_CVM_GTEST_TESTCONFIG_FILE                 "OtherInfoConfig.json"   //!< 上报配置文件

#define ADM_CVM_TEST_FUNCTION_RET                     "ret"

#define ADM_CVM_TEST_GET_EVENT_REPORT_RET             "adm_ecs_getEventReportLevel"
#define ADM_CVM_TEST_EVENT_REPORT_RET                 "reportLevel"

#define ADM_CVM_TEST_GET_RSCONFVERSION                "adm_ecs_getRsConfigVersion"
#define ADM_CVM_TEST_CONFVERSION                      "configVersion"

#define ADM_CVM_TEST_GET_VEHICLEBIGVERSION            "adm_eim_getVehicleBigVersion"
#define ADM_CVM_TEST_VEHICLEBIGVERSION                "vehicleBigVersion"

#define ADM_CVM_TEST_NEW_CHECK_RESP_ADAPBODY          "adm_ocs_newCheckRespAdapterBody"
#define ADM_CVM_TEST_NEW_CHECK_REQ_ADAPBODY           "adm_ocs_newCheckReqAdapterBody"
#define ADM_CVM_TEST_CHECK_PRO_ADP                    "adm_ocs_checkProtocolAdapter"
#define ADM_CVM_TEST_CHECK_PRO_PARSE                  "adm_ocs_checkProtocolParse"

#define ADM_CVM_TEST_BUSI_COMM_INTER                  "adm_ocm_businessCommInterface"
#define ADM_CVM_TEST_RSPSTATUSCODE                    "rspStatusCode"

#define ADM_CVM_TEST_DO_DOWNLOAD_FILE                 "adm_dpo_doDownloadFile"
#define ADM_CVM_TEST_DECRYPT_AND_VERIF                "adm_ecm_decryptAndVerificationSignature"

#define ADM_CVM_TEST_EIM_ECUINFO_KV                   "adm_eim_getEcuInfoManagerKvGetStr"
#define ADM_CVM_TEST_EIM_ECUINFO_DIM                  "adm_eim_getEcuInfoManagerDimGetStr"

typedef struct getEventReportLevel_
{
    D32S ret;
    D32S reportLevel;
}ADM_CVM_EVENT_REPORT_T;

typedef struct getRsConfigVersion_
{
    D32S ret;
    DCHAR* configVersion;
}ADM_CVM_ConfigVersion_T;

typedef struct cvmVehicleBigVersion_
{
    D32S ret;
    DCHAR* vehicleBigVersion;
}ADM_CVM_VehicleBigVersion_T;

typedef struct cvmBusinessCommInterface_
{
    D32S ret;
    DSIZE rspStatusCode;
}ADM_CVM_BUSINESS_COMM_INTE_T;

typedef struct cvmOtherInfoConf_
{
    ADM_CVM_EVENT_REPORT_T stEvent_Report;
    ADM_CVM_ConfigVersion_T stConfVer;
    ADM_CVM_VehicleBigVersion_T bigVersion;
    D32S newCheckRespAdapterBody_ret;
    D32S newCheckReqAdapterBody_ret;
    D32S checkProtocolAdapter_ret;
    D32S checkProtocolParse_ret;
    ADM_CVM_BUSINESS_COMM_INTE_T Busi_Comm_inter_ret;
    D32S doDownloadFile_ret;
    D32S decryptAndVerificationSignature_ret;
    D32S EcuInfoManagerKv_ret;
    D32S EcuInfoManagerDim_ret;
}ADM_CVM_TEST_OTHER_INFO_CONF_T;

void adm_cvm_GtestSetOtherConfFileDir(DCHAR* pcConfigFileDir);
DCHAR* adm_cvm_GtestReadConfFile(const DCHAR* pcFileName);
DCHAR* adm_cvm_GtestGetOtherConfFileDir(void);
D32S adm_cvm_GtestGetOtherInfoConfig(ADM_CVM_TEST_OTHER_INFO_CONF_T* pstOtherConfigData);

#endif