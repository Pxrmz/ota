#include "adm_irm_gtest.h"
#include "adm_adm_prase_reportconfig.h"
#include "adm_adm_prase_testconfig.h"
#include "adm_ocm_interface.h"
#include "adm_com_errorCode.h"
#include "adm_irm_report.h"
#include "adm_irm_logFile.h"
#include <unistd.h>
#include <time.h>
#include <sys/stat.h>
#include "adm_typedefs.h"
#include "adm_memory.h"
#include "adm_debug.h"
#include "adm_string.h"
#include "adm_irm_log.h"
#include "adm_cfg_api.h"
#include "adm_com_kvstorage.h"
#include "adm_ocs_airProtocolAdapter.h"
#include "adm_com_hash.h"
#include "adm_com_lib.h"
#include <stdlib.h>
#include <dirent.h>
#include <sys/types.h>
#include "adm_file.h"


CIRSTest::CIRSTest()
{
    
}

CIRSTest::~CIRSTest()
{
    
}

ADM_TEST_TESTCONFIG_T gstTestConfigData= {0};
ADM_CONFIG_REPORTFILE_T gstReportConfigData= {0};
ADM_IRM_LOG_FILE_REPORT_INIT_T g_stLogFileReportInfo = {0};
#define ADM_IRM_TEST_CONFIGJSON_FILEPATH 256

#ifdef __cplusplus
extern "C" {

/*-----------------------------step 1、代码重构-------------------------------*/

/*
* @brief ReWrite function adm_ecs_getEventReportCycle
*   
*
*/
//D32S __wrap_adm_ecs_getEventReportCycle(D32S *ps32EventReportCycle)
//{
//    *ps32EventReportCycle = gstReportConfigData.eventReportCycle;
//    if (0 <= *ps32EventReportCycle)
//    {
//        log_i("Get EventReportCycle is [%d]",*ps32EventReportCycle);
//        return ADM_ECODE_SUCCESSED;
//    }
//
//    return ADM_ECODE_FAILED;
//}

D32S __wrap_adm_ecs_getLogReportCycle(D32S *ps32LogReportCycle)
{
    *ps32LogReportCycle = gstReportConfigData.logReportCycle;
    if (0 <= *ps32LogReportCycle)
    {
        log_i("Get logReportCycle is [%d]",*ps32LogReportCycle);
        return ADM_ECODE_SUCCESSED;
    }

    return ADM_ECODE_FAILED;
}

const char* adm_sdk_getSDKVersion(void)
{
    log_i("Get sdkVersion is [%s]", gstReportConfigData.sdkVersion);
    return gstReportConfigData.sdkVersion;
}

D32S adm_cvs_getStrategyId(D64S* strategyId)
{
    *strategyId = gstReportConfigData.strategyId;
//    if (0 >= *strategyId)
//    {
//        return ADM_ECODE_FAILED;
//    }

    log_i("Get strategyId is [%lld]", *strategyId);
    return ADM_ECODE_SUCCESSED;
}

D32S adm_cvs_getTaskId(D64S* taskId)
{
    *taskId = gstReportConfigData.taskId;
//    if (0 >= *taskId)
//    {
//        return ADM_ECODE_FAILED;
//    }

    log_i("Get taskId is [%lld]", *taskId);
    return ADM_ECODE_SUCCESSED;
}

D32S adm_cvs_getSessionId(DCHAR** sessionId)
{
    *sessionId = adm_strdup(gstReportConfigData.sessionId);
    if (NULL == *sessionId)
    {
        return ADM_ECODE_FAILED;
    }

    log_i("Get sessionId is [%s]", *sessionId);
    return ADM_ECODE_SUCCESSED;
}

D32S __wrap_adm_ecs_getIsLogFileEncrypted(D32S *ps32IsLogFileEncrypted)
{
    *ps32IsLogFileEncrypted = gstReportConfigData.isLogFileEncrypted;
    if (1 != *ps32IsLogFileEncrypted && 2 != *ps32IsLogFileEncrypted)
    {
        return ADM_ECODE_FAILED;
    }

    log_i("Get IsLogFileEncrypted is [%d]", *ps32IsLogFileEncrypted);
    return ADM_ECODE_SUCCESSED;
}

D32S adm_ocm_businessCommInterface(const ADM_OCS_BUSINESS_TYPE_E eBusinessType, 
                                                const DCHAR* reqMessageBody, 
                                                DCHAR** rspMessageBody,
                                                DSIZE* rspStatusCode)
{
    *rspMessageBody = adm_strdup(gstTestConfigData.pstOcmBusInter->pcRspMessageBody);
    *rspStatusCode = gstTestConfigData.pstOcmBusInter->iRspStatusCode;

    return gstTestConfigData.pstOcmBusInter->iFuncValue;
}

D32S adm_ocm_fileUploadInterface(ADM_OCA_FILE_UPLOAD_INFO_T* uploadInfo, 
                                             DCHAR** rspMessageBody,
                                             ADM_OCS_UPLOADFILE_T* pstUploadInfo)
{
    *rspMessageBody = adm_strdup(gstTestConfigData.pstOcmFileInter->pcRspMessageBody);

    return gstTestConfigData.pstOcmFileInter->iFuncValue;
}

D32S __wrap_adm_ocs_uploadProtocolParse(DCHAR* respBodyFromCloud, D32S* status)
{
    *status = ADM_OCS_AIR_STATUS_CODE_SUCCESSED;

    return ADM_ECODE_SUCCESSED;
}

D32S __wrap_adm_ocs_otaStatusCodeTransfer(const D32S statusCode)
{
    return ADM_ECODE_SUCCESSED;
}

D32S __wrap_adm_ocs_eventProtocolAdapter(const DCHAR* sessionId,
                                         D64S taskId,
                                         D64S strategyId,
                                         D64S timestamp,
                                         const DCHAR* sdkVersion,
                                         const DCHAR* extsReqJsonStr,
                                         ADM_OCS_EVENTS_INFO_LIST_T* eventsList,
                                         DCHAR** generateMessageBody)
{
    if( 0 != gstTestConfigData.pstOcsEventAdapter->iFuncValue )
    {
        return gstTestConfigData.pstOcsEventAdapter->iFuncValue;
    }
    DCHAR acNumber[ADM_IRM_TEST_CONFIGJSON_FILEPATH]={0};
    static int k;
    (void)adm_snprintf(acNumber, sizeof(acNumber), "%d", k++); 
    *generateMessageBody = adm_strdup(acNumber);

    return gstTestConfigData.pstOcsEventAdapter->iFuncValue;
}
D32S __wrap_adm_ocs_vehicleProgressProtocolAdapter(const DCHAR* sessionId,
                                                                        D64S taskId,
                                                                        D64S strategyId,
                                                                        D64S timestamp,
                                                                        const DCHAR* sdkVersion,
                                                                        D64S sequenceId,
                                                                        const DCHAR* extsReqJsonStr,
                                                                        ADM_OCS_ECU_PROGRESS_INFO_LIST_T* ecusProgressList,
                                                                        DCHAR** generateMessageBody)
{
    if( 0 != gstTestConfigData.pstOcsProgressAdapter->iFuncValue)
    {
        return gstTestConfigData.pstOcsProgressAdapter->iFuncValue;
    }
    *generateMessageBody = adm_strdup(gstTestConfigData.pstOcsProgressAdapter->pcMessageBody);

    return gstTestConfigData.pstOcsProgressAdapter->iFuncValue;
}

D32S __wrap_adm_ocs_ecuStateProtocolAdapter(const DCHAR* sessionId,
                                                              D64S taskId,
                                                              D64S strategyId,
                                                              D64S timestamp,
                                                              const DCHAR* sdkVersion,
                                                              D64S sequenceId,
                                                              const DCHAR* extsReqJsonStr,
                                                              ADM_OCS_ECU_STATE_INFO_LIST_T* ecusList,
                                                              DCHAR** generateMessageBody)
{
    if( 0 != gstTestConfigData.pstOcsEcuStateAdapter->iFuncValue)
    {
        return gstTestConfigData.pstOcsEcuStateAdapter->iFuncValue;
    }
    *generateMessageBody = adm_strdup(gstTestConfigData.pstOcsEcuStateAdapter->pcMessageBody);

    return gstTestConfigData.pstOcsEcuStateAdapter->iFuncValue;
}

D32S __wrap_adm_ocs_vehicleStateProtocolAdapter(D64S timestamp,
                                                                   const DCHAR* sdkVersion,
                                                                   const DCHAR* sessionId,
                                                                   D64S taskId,
                                                                   D64S strategyId,
                                                                   D64S sequenceId,
                                                                   const DCHAR* state,
                                                                   const DCHAR* reason,
                                                                   const DCHAR* versionNumber,
                                                                   const DCHAR* extsReqJsonStr,
                                                                   DCHAR** generateMessageBody)
{
    if( 0 != gstTestConfigData.pstOcsVehStateAdapter->iFuncValue)
    {
        return gstTestConfigData.pstOcsVehStateAdapter->iFuncValue;
    }
    DCHAR acNumber[ADM_IRM_TEST_CONFIGJSON_FILEPATH]={0};
    static int j;
    (void)adm_snprintf(acNumber, sizeof(acNumber), "%d", j++); 
    *generateMessageBody = adm_strdup(acNumber);

    return gstTestConfigData.pstOcsVehStateAdapter->iFuncValue;
}

D32S __wrap_adm_ocs_strSign(const DCHAR* str, DCHAR** sign)
{
    *sign = adm_strdup("sign");

    return gstTestConfigData.iIsFileToKvFail;
}

D32S __wrap_adm_tpm_CollectLogFromSysiToHost(DCHAR *pcTransInstId, DCHAR **ppcLogPath)
{
    if( 0 != gstTestConfigData.pstCollectSysiLog->iFuncValue)
    {
        return gstTestConfigData.pstCollectSysiLog->iFuncValue;
    }
    D64S timestamp = time(NULL);
    static int num;
    DCHAR sysiZipFileName[ADM_IRM_TEST_CONFIGJSON_FILEPATH] = {0};
    FILE *fin = NULL, *fout = NULL;
    D32S iRet = ADM_ECODE_FAILED;
    //!< 拼接解密后的日志文件名, log_enc_timestamp.zip
    (void)adm_memset(sysiZipFileName, 0x00, sizeof(sysiZipFileName), sizeof(sysiZipFileName));
    (void)adm_snprintf(sysiZipFileName, sizeof(sysiZipFileName), "%s_%lld_%d.zip", "/tmp/sysi", timestamp, num++);
    //将log_sysi.zip 拷贝一份名为log_sysi_1_时间戳.zip
    fin = fopen(gstTestConfigData.pstCollectSysiLog->pcFilePath, "rb");
    if(NULL == fin)
    {
        log_e("fopen file %s error", gstTestConfigData.pstCollectSysiLog->pcFilePath);
        return iRet;
    }

    fout = fopen(sysiZipFileName, "wb");
    if(NULL == fout)
    {
        log_e("fopen file %s error", sysiZipFileName);
        fclose(fin);
        return iRet;
    }
    
    char buf[1024];
    int n;
    while(1)
    {
        adm_memset(buf,0,sizeof(buf),sizeof(buf));
        n = fread(buf,1,sizeof(buf),fin);
        if(n==0)
        {
            break;
        }
        fwrite(buf,1,n,fout);
    }
    fclose(fin);
    fclose(fout);
    
    if (FALSE == adm_fisexist(sysiZipFileName)) 
    {
        log_e("file not exist.error[%d]", ADM_ECODE_IRS_UPLOADFILE_NOTEXIST);
        return ADM_ECODE_IRS_UPLOADFILE_NOTEXIST;
    }

    *ppcLogPath = adm_strdup(sysiZipFileName);
    return gstTestConfigData.pstCollectSysiLog->iFuncValue;
}

int __wrap_FILE_AES_Encrypt(const char *finPath, char* keyStr, const char *foutPath)
{
    if( 0 != gstTestConfigData.pstEncryptLogFile->iFuncValue)
    {
        return gstTestConfigData.pstEncryptLogFile->iFuncValue;
    }
    FILE *fin = NULL, *fout = NULL;
    D32S iRet = ADM_ECODE_FAILED;
    //!< 拼接解密后的日志文件名, log_enc_timestamp.zip
    //将log_enc.zip 拷贝一份名为log_enc_1_时间戳.zip

    fin = fopen(finPath, "rb");
    if(NULL == fin)
    {
        log_e("fopen file %s error", finPath);
        return iRet;
    }

    fout = fopen(foutPath, "wb");
    if(NULL == fout)
    {
        log_e("fopen file %s error", foutPath);
        fclose(fin);
        return iRet;
    }
    
    char buf[1024];
    int n;
    while(1)
    {
        adm_memset(buf,0,sizeof(buf),sizeof(buf));
        n = fread(buf,1,sizeof(buf),fin);
        if(n==0)
        {
            break;
        }
        fwrite(buf,1,n,fout);
    }
    fclose(fin);
    fclose(fout);
    
    if (FALSE == adm_fisexist(foutPath)) 
    {
        log_e("file not exist.error[%d]", ADM_ECODE_IRS_UPLOADFILE_NOTEXIST);
        return ADM_ECODE_IRS_UPLOADFILE_NOTEXIST;
    }

    return gstTestConfigData.pstEncryptLogFile->iFuncValue;
}


}
#endif

/*-----------------------------step 2、测试环境初始化-------------------------------*/

/**
* @brief OTA应用程序所使用的目录初始化
* 
* @return D32S     0:Successed, None-0:Failed
*/
D32S adm_irm_dirInit(void)
{
   D32S iRet = ADM_ECODE_FAILED;
   const DCHAR* dirStr = NULL;

   do
   {

       /** 初始化运行时主目录 */
       dirStr = adm_cfg_iniParserGetString(ADM_CFG_VUS_RUNTIME_ROOT_DIR, ADM_CFG_VUS_RUNTIME_ROOT_DIR_DEFAULT_VAL);
       iRet = adm_com_createDir(dirStr);
       if (ADM_ECODE_SUCCESSED != iRet) 
       {
           printf("adm_com_createDir %s failed, iRet:%d\n", dirStr, iRet);
           break;
       }
       else 
       {
           printf("adm_com_createDir %s successed\n", dirStr);
       }
       /** 初始化日志存储目录 */
       dirStr = adm_cfg_iniParserGetString(ADM_CFG_VUS_LOG_STORAGE_DIR, ADM_CFG_VUS_LOG_STORAGE_DIR_DEFAULT_VAL);
       iRet = adm_com_createDir(dirStr);
       if (ADM_ECODE_SUCCESSED != iRet) 
       {
           printf("adm_com_createDir %s failed, iRet:%d\n", dirStr, iRet);
           break;
       } 
       else 
       {
           printf("adm_com_createDir %s successed\n", dirStr);
       }

       adm_debug_logInitWithFileName(dirStr, "admfotaapp", 1045504,5,ELOG_LVL_DEBUG,FALSE);

       /** 初始化日志收集存储目录 */
       dirStr = adm_cfg_iniParserGetString(ADM_CFG_VUS_LOG_COLLECT_DIR, ADM_CFG_VUS_LOG_COLLECT_DIR_DEFAULT_VAL);
       iRet = adm_com_createDir(dirStr);
       if (ADM_ECODE_SUCCESSED != iRet) 
       {
           printf("adm_com_createDir %s failed, iRet:%d\n", dirStr, iRet);
           break;
       } 
       else 
       {
           printf("adm_com_createDir %s successed\n", dirStr);
       }

       /** 初始化日志打包的存储目录 */
       dirStr = adm_cfg_iniParserGetString(ADM_CFG_VUS_LOG_PACK_DIR, ADM_CFG_VUS_LOG_PACK_DIR_DEFAULT_VAL);
       iRet = adm_com_createDir(dirStr);
       if (ADM_ECODE_SUCCESSED != iRet) 
       {
           printf("adm_com_createDir %s failed, iRet:%d\n", dirStr, iRet);
           break;
       } 
       else 
       {
           printf("adm_com_createDir %s successed\n", dirStr);
       }

   } while(FALSE);
   return iRet;
}

void __irmTestInit(void)
{
   const DCHAR* configVal = NULL;
  //!< 初始化日志目录
   D32S iRet = ADM_ECODE_SUCCESSED;
   (void)adm_irm_dirInit();

   //!< 初始化Kv storage
   //!< 初始化KV目录
   configVal = adm_cfg_iniParserGetString(ADM_CFG_VUS_KV_STORAGE_DIR, ADM_CFG_VUS_KV_STORAGE_DIR_DEFAULT_VAL);
   log_i("adm_core_service_initRuntimeDir eType:%d, dir:%s ", ADM_CORE_SERVICE_RUNTIME_DIR_TYPE_KV_STORAGE, configVal);
   if (FALSE == adm_core_service_initRuntimeDir(ADM_CORE_SERVICE_RUNTIME_DIR_TYPE_KV_STORAGE, configVal)) 
   {
      log_e("adm_core_service_initRuntimeDir eType:%d, dir:%s failed", ADM_CORE_SERVICE_RUNTIME_DIR_TYPE_KV_STORAGE, configVal);
   }
   else
   {
       adm_com_kvstorageInit(NULL, NULL);
       //!< 初始化core中 kvstorage数据库
       iRet = adm_core_service_init();
       if (ADM_ECODE_SUCCESSED != iRet) 
       {
           log_e("adm_core_service_init failed, err:%d", iRet);
           return ;
       }

   }

}


D32S adm_apc_logInit(void)
{
    adm_debug_logInitWithFileName("/tmp/admota/ucm/log/storage/", "admfotaapp", 1045504,5,ELOG_LVL_ERROR,FALSE);
    return ADM_ECODE_SUCCESSED;
}
void adm_apc_logDeinit(void)
{
    // 反初始化日志模块
    adm_debug_logDeinit();

}

void irmApcLogFileReportInit()
{
    const DCHAR* dirStr = NULL;

    (void)adm_memset(&g_stLogFileReportInfo, 0x00, sizeof(ADM_IRM_LOG_FILE_REPORT_INIT_T), sizeof(ADM_IRM_LOG_FILE_REPORT_INIT_T));
    g_stLogFileReportInfo.logInitCb = adm_apc_logInit;
    g_stLogFileReportInfo.logDeinitCb = adm_apc_logDeinit;
    g_stLogFileReportInfo.getSessionIdCb = adm_cvs_getSessionId;
    g_stLogFileReportInfo.getTaskIdCb = adm_cvs_getTaskId;
    g_stLogFileReportInfo.getStrategyIdCb = adm_cvs_getStrategyId;
    g_stLogFileReportInfo.getSdkVerCb = adm_sdk_getSDKVersion;
    g_stLogFileReportInfo.getLogReportCycle = adm_ecs_getLogReportCycle;

    //!< 从车云协议获取上报日志文件是否需要加密操作成功    
    if(ADM_ECODE_SUCCESSED == adm_ecs_getIsLogFileEncrypted(&g_stLogFileReportInfo.isEncrypt))
    {
        //!< 从车云协议获取上报日志文件是否加密不在取值范围中，则填充默认值
        if((ADM_IRM_LOG_FILE_ENCRYPT_REPORT != g_stLogFileReportInfo.isEncrypt) &&
           (ADM_IRM_LOG_FILE_NOTENCRYPT_REPORT != g_stLogFileReportInfo.isEncrypt))
        {
            log_e("Error value of isEncrypt, value = [%d], Fill defaults.", g_stLogFileReportInfo.isEncrypt);
            g_stLogFileReportInfo.isEncrypt = ADM_IRM_LOG_FILE_ENCRYPT_REPORT;
        }
    }
    //!< 从车云协议获取上报日志文件是否需要加密操作失败，填充默认值
    else
    {
        log_e("Failed to get config of isEncrypt and Fill defaults.");
        g_stLogFileReportInfo.isEncrypt = ADM_IRM_LOG_FILE_ENCRYPT_REPORT;
    }

    //!> 加密相关变量参数初始化
    g_stLogFileReportInfo.secret = NULL;
    g_stLogFileReportInfo.secretLen = 0;
    g_stLogFileReportInfo.segmentEncLen = 0;
    g_stLogFileReportInfo.fileEncCb = NULL;

    //!< 初始化日志存储目录
    dirStr = NULL;
    dirStr = adm_cfg_iniParserGetString(ADM_CFG_VUS_LOG_STORAGE_DIR, ADM_CFG_VUS_LOG_STORAGE_DIR_DEFAULT_VAL);
    if (NULL != dirStr) 
    {
        g_stLogFileReportInfo.logStorageDir = adm_strdup(dirStr);
    }

    //!< 初始化日志收集存储目录
    dirStr = NULL;
    dirStr = adm_cfg_iniParserGetString(ADM_CFG_VUS_LOG_COLLECT_DIR, ADM_CFG_VUS_LOG_COLLECT_DIR_DEFAULT_VAL);
    if (NULL != dirStr) 
    {
        g_stLogFileReportInfo.logCollectDir = adm_strdup(dirStr);
    }

    //!< 初始化日志打包的存储目录
    dirStr = NULL;
    dirStr = adm_cfg_iniParserGetString(ADM_CFG_VUS_LOG_PACK_DIR, ADM_CFG_VUS_LOG_PACK_DIR_DEFAULT_VAL);
    if (NULL != dirStr) 
    {
        g_stLogFileReportInfo.logPackDir = adm_strdup(dirStr);
    }

}


D32S adm_irmgtest_testEnvironmentInit(void)
{
    D32S iret = ADM_ECODE_FAILED;
    DCHAR acConfigFileFullPath[ADM_IRM_TEST_CONFIGJSON_FILEPATH] = {0};
    DCHAR *pcConfigFileDir = NULL;

    // 目录初始化
    printf("init IRM gtest environment start\n");
    __irmTestInit();
    adm_memset(&gstReportConfigData,0,sizeof(ADM_CONFIG_REPORTFILE_T), sizeof(ADM_CONFIG_REPORTFILE_T));
    adm_memset(&gstTestConfigData,0,sizeof(ADM_TEST_TESTCONFIG_T), sizeof(ADM_TEST_TESTCONFIG_T));

    // 获取json配置
    log_i("Get IRM test report config\n");

    (void)adm_snprintf(acConfigFileFullPath, sizeof(acConfigFileFullPath), "%s%s", getcwd(pcConfigFileDir, ADM_IRM_TEST_CONFIGJSON_FILEPATH), "/../config/jsonfile/");
//    (void)adm_snprintf(acConfigFileFullPath, sizeof(acConfigFileFullPath), "%s%s", getcwd(pcConfigFileDir, ADM_IRM_TEST_CONFIGJSON_FILEPATH), "/../etc/");
    log_i("pcConfigFileDir= %s\n", acConfigFileFullPath);

    adm_irm_GtestSetRepConfFileDir(acConfigFileFullPath);
    iret = adm_irm_GtestGetRepConfData(&gstReportConfigData);
    if(ADM_ECODE_SUCCESSED!=iret)
    {
        log_e("adm_irm_GtestGetTestConfData iret is [%d]\n", iret);
        return -1;
    }

    log_i("Get IRM test test config\n");
    adm_irm_GtestSetTestConfFileDir(acConfigFileFullPath);
    iret = adm_irm_GtestGetTestConfData(&gstTestConfigData);
    if(ADM_ECODE_SUCCESSED!=iret)
    {
        log_e("adm_irm_GtestGetTestConfData iret is [%d]\n", iret);
        return -1;
    }
    irmApcLogFileReportInit();
    if (!adm_com_kvstorage_set_string(adm_core_service_getBusinessDatabaseDb(),
                                      "secret", "174444a7b7e09408b47fbde81a849d9b", ADM_KV_STORAGE_VALUE_TYPE_STRING))
    {
        log_e( "adm_com_kvstorage_set_string string:%s failed", "174444a7b7e09408b47fbde81a849d9b");
    } 
    else
    {
        log_i( "kvstorate_set_string secret:%s successed", "174444a7b7e09408b47fbde81a849d9b");
    }

    log_i("init IRM gtest environment finish\n");
    return iret;
}


/*-----------------------------step 3、测试用例-------------------------------*/

// 上报初始化&周期上报
D32S CIRSTest::adm_irm_init_test(void)
{
    log_i("irmgtest adm_irm_init test start\n");
    D32S iret = adm_irm_init(adm_ocm_businessCommInterface, adm_ocm_fileUploadInterface, NULL);
    EXPECT_EQ(0, iret);

    return iret;
}

/* 外部调用日志文件上报 */
void CIRSTest::adm_irm_triggerLogFileReport_test()
{

    log_i("irmgtest adm_irm_triggerLogFileReport test start\n");
    ADM_IRM_LOG_FILE_TG_REPORT_INFO_T stReportInfo;
    adm_memset(&stReportInfo, 0, sizeof(ADM_IRM_LOG_FILE_TG_REPORT_INFO_T), sizeof(ADM_IRM_LOG_FILE_TG_REPORT_INFO_T));

    stReportInfo.cmdId = gstTestConfigData.pstTriggerLogFileReport->cmdId;
    stReportInfo.pushId = gstTestConfigData.pstTriggerLogFileReport->pushId;
    stReportInfo.extsReqJsonStr = gstTestConfigData.pstTriggerLogFileReport->pcExtsReqJsonStr;

    adm_irm_triggerLogFileReport(&stReportInfo);
}

void CIRSTest::adm_irm_logFileReportInit_test()
{

    log_i("irmgtest adm_irm_logFileReportInit test start\n");
    //!< 初始化日志上报模块
    adm_irm_logFileReportInit(&g_stLogFileReportInfo);
}

/* 外部调用日志文件上报 */
void CIRSTest::adm_irm_irmMessageReport_test()
{

    log_i("irmgtest adm_irm_CacheGreater100_test test start\n");
    D64S timestamp = time(NULL);

    for(int i=1; i<= gstTestConfigData.iVehicleControlNum; i++)
    {
        adm_irm_reportVehicleState(timestamp,
                                   gstReportConfigData.sdkVersion,
                                   gstReportConfigData.sessionId,
                                   gstReportConfigData.taskId,
                                   gstReportConfigData.strategyId,
                                   gstTestConfigData.pstReportVehicleState->pcState,
                                   gstTestConfigData.pstReportVehicleState->pcReason,
                                   gstTestConfigData.pstReportVehicleState->pcVersionNumber,
                                   gstTestConfigData.pstReportVehicleState->pcExtsReqJsonStr);
    }
    for(int i=1; i<= gstTestConfigData.iEventControlNum; i++)
    {
        /*events*/
        ADM_OCS_EVENTS_INFO_LIST_T* eventsList = NULL;
        eventsList = (ADM_OCS_EVENTS_INFO_LIST_T*)adm_malloc(sizeof(ADM_OCS_EVENTS_INFO_LIST_T));
        adm_memset(eventsList, 0, sizeof(ADM_OCS_EVENTS_INFO_LIST_T), sizeof(ADM_OCS_EVENTS_INFO_LIST_T));
        eventsList->reportEventsLen = gstTestConfigData.pstReportEvent->eventsList->reportEventsLen;
        eventsList->reportEvents = (ADM_OCS_EVENTS_INFO_T*)adm_malloc(sizeof(ADM_OCS_EVENTS_INFO_T) * eventsList->reportEventsLen);
        adm_memset(eventsList->reportEvents, 0,
                   sizeof(ADM_OCS_EVENTS_INFO_T) * eventsList->reportEventsLen,\
                   sizeof(ADM_OCS_EVENTS_INFO_T) * eventsList->reportEventsLen);

        for(D32S i = 0; i < eventsList->reportEventsLen; i++)
        {
            eventsList->reportEvents[i].eventSequenceId = adm_irm_getEventSequenceId();
            eventsList->reportEvents[i].eventId = strdup(gstTestConfigData.pstReportEvent->eventsList->reportEvents[i].eventId);
            eventsList->reportEvents[i].eventLevel = gstTestConfigData.pstReportEvent->eventsList->reportEvents[i].eventLevel;
            eventsList->reportEvents[i].eventDesc = strdup(gstTestConfigData.pstReportEvent->eventsList->reportEvents[i].eventDesc);
            eventsList->reportEvents[i].eventTime = time(NULL);
        }
        adm_irm_reportEvent(gstReportConfigData.sessionId, 
                            gstReportConfigData.taskId, 
                            gstReportConfigData.strategyId, 
                            timestamp,
                            gstReportConfigData.sdkVersion, 
                            gstTestConfigData.pstReportEvent->pcExtsReqJsonStr,  
                            eventsList);
    }
    for(int i=1; i<= gstTestConfigData.iProgressControlNum; i++)
    {
        ADM_OCS_ECU_PROGRESS_INFO_LIST_T* ecusProgressList = NULL;
        ecusProgressList = (ADM_OCS_ECU_PROGRESS_INFO_LIST_T*)adm_malloc(sizeof(ADM_OCS_ECU_PROGRESS_INFO_LIST_T));
        adm_memset(ecusProgressList, 0, sizeof(ADM_OCS_ECU_PROGRESS_INFO_LIST_T), sizeof(ADM_OCS_ECU_PROGRESS_INFO_LIST_T));
        ecusProgressList->reportEcusLen = gstTestConfigData.pstReportVehicleProgress->pstEcusProgressList->reportEcusLen;
        ecusProgressList->reportEcus = (ADM_OCS_ECU_PROGRESS_INFO_T*)adm_malloc(
                                        sizeof(ADM_OCS_ECU_PROGRESS_INFO_T) * ecusProgressList->reportEcusLen);
        adm_memset(ecusProgressList->reportEcus,0,\
                   sizeof(ADM_OCS_ECU_PROGRESS_INFO_T) * ecusProgressList->reportEcusLen,\
                   sizeof(ADM_OCS_ECU_PROGRESS_INFO_T) * ecusProgressList->reportEcusLen);

        for(D32S i = 0; i< ecusProgressList->reportEcusLen; i++)
        {
            ecusProgressList->reportEcus[i].ecuReqId = 
                            strdup(gstTestConfigData.pstReportVehicleProgress->pstEcusProgressList->reportEcus[i].ecuReqId);
            ecusProgressList->reportEcus[i].ecuSwDid = 
                            strdup(gstTestConfigData.pstReportVehicleProgress->pstEcusProgressList->reportEcus[i].ecuSwDid);
            ecusProgressList->reportEcus[i].progressType = 
                            strdup(gstTestConfigData.pstReportVehicleProgress->pstEcusProgressList->reportEcus[i].progressType); 
            ecusProgressList->reportEcus[i].progress = 
                            gstTestConfigData.pstReportVehicleProgress->pstEcusProgressList->reportEcus[i].progress;
        }

        adm_irm_reportVehicleProgress(gstReportConfigData.sessionId, 
                                      gstReportConfigData.taskId, 
                                      gstReportConfigData.strategyId, 
                                      timestamp,
                                      gstReportConfigData.sdkVersion, 
                                      gstTestConfigData.pstReportVehicleProgress->pcExtsReqJsonStr,
                                      ecusProgressList);
    }
    for(int i=1; i<= gstTestConfigData.iEcuControlNum; i++)
    {
        /*ecu state*/
        ADM_OCS_ECU_STATE_INFO_LIST_T* ecusList = NULL;
        ecusList = (ADM_OCS_ECU_STATE_INFO_LIST_T*)adm_malloc(sizeof(ADM_OCS_ECU_STATE_INFO_LIST_T));
        adm_memset(ecusList,0,sizeof(ADM_OCS_ECU_STATE_INFO_LIST_T), sizeof(ADM_OCS_ECU_STATE_INFO_LIST_T));

        ecusList->reportEcusLen = gstTestConfigData.pstReportEcuState->pstEcusList->reportEcusLen;
        ecusList->reportEcus= (ADM_OCS_ECU_STATE_INFO_T*)adm_malloc(sizeof(ADM_OCS_ECU_STATE_INFO_T) * (D32U)ecusList->reportEcusLen);
        adm_memset(ecusList->reportEcus,0,\
                   sizeof(ADM_OCS_ECU_STATE_INFO_T) * ecusList->reportEcusLen,\
                   sizeof(ADM_OCS_ECU_STATE_INFO_T) * ecusList->reportEcusLen);

        for(D32S i = 0; i< ecusList->reportEcusLen; i++)
        {
            ecusList->reportEcus[i].beginTime = gstTestConfigData.pstReportEcuState->pstEcusList->reportEcus[i].beginTime;
            ecusList->reportEcus[i].endTime = gstTestConfigData.pstReportEcuState->pstEcusList->reportEcus[i].endTime;
            ecusList->reportEcus[i].ecuReqId = adm_strdup(gstTestConfigData.pstReportEcuState->pstEcusList->reportEcus[i].ecuReqId);
            ecusList->reportEcus[i].ecuSwDid = adm_strdup(gstTestConfigData.pstReportEcuState->pstEcusList->reportEcus[i].ecuSwDid);
            ecusList->reportEcus[i].ecuState = adm_strdup(gstTestConfigData.pstReportEcuState->pstEcusList->reportEcus[i].ecuState);
            ecusList->reportEcus[i].ecuReason = adm_strdup(gstTestConfigData.pstReportEcuState->pstEcusList->reportEcus[i].ecuReason);
            ecusList->reportEcus[i].ecuDidLists = (ADM_OCS_ECU_DID_LIST_T*)adm_malloc(sizeof(ADM_OCS_ECU_DID_LIST_T));
            ecusList->reportEcus[i].ecuDidLists->ecuDidsLen = 
                                gstTestConfigData.pstReportEcuState->pstEcusList->reportEcus[i].ecuDidLists->ecuDidsLen;
            ecusList->reportEcus[i].ecuDidLists->ecuDids = (ADM_OCS_ECU_DID_T*)adm_malloc(
                                                            sizeof(ADM_OCS_ECU_DID_T) * (D32U)ecusList->reportEcus[i].ecuDidLists->ecuDidsLen);
            for(D32S j = 0; j< ecusList->reportEcusLen; j++)
            {
                ecusList->reportEcus[i].ecuDidLists->ecuDids[j].did = 
                                adm_strdup(gstTestConfigData.pstReportEcuState->pstEcusList->reportEcus[i].ecuDidLists->ecuDids[j].did);
                ecusList->reportEcus[i].ecuDidLists->ecuDids[j].didValue = 
                                adm_strdup(gstTestConfigData.pstReportEcuState->pstEcusList->reportEcus[i].ecuDidLists->ecuDids[j].didValue);
                ecusList->reportEcus[i].ecuDidLists->ecuDids[j].start = 
                                gstTestConfigData.pstReportEcuState->pstEcusList->reportEcus[i].ecuDidLists->ecuDids[j].start;
                ecusList->reportEcus[i].ecuDidLists->ecuDids[j].end = 
                                gstTestConfigData.pstReportEcuState->pstEcusList->reportEcus[i].ecuDidLists->ecuDids[j].end;
            }
        }
        adm_irm_reportEcuState(gstReportConfigData.sessionId, 
                               gstReportConfigData.taskId, 
                               gstReportConfigData.strategyId, 
                               timestamp,
                               gstReportConfigData.sdkVersion,
                               gstTestConfigData.pstReportEcuState->pcExtsReqJsonStr,
                               ecusList);
   }

}

/* 外部调用日志文件上报 */
void CIRSTest::adm_irm_irmFileReport_test()
{

    log_i("irmgtest adm_irm_CacheGreaterFile100 test start\n");
    DIR* dir = NULL;
    struct dirent* ptr = NULL;
    DCHAR* dirPath = gstTestConfigData.pFilePaths;
    DCHAR fullFileName[ADM_IRM_TEST_CONFIGJSON_FILEPATH] = {0};
    D32U len = 0;
    int i=0;

    if(ADM_OCS_FILE_ENCRYPTED != gstReportConfigData.isLogFileEncrypted && ADM_OCS_FILE_ENCRYPTED_NOT != gstReportConfigData.isLogFileEncrypted)
    {
        log_e("adm_irm_CacheGreater100_test isLogFileEncrypted is illegal");
        return ;
    }

    dir = opendir(dirPath);
    if (NULL == dir) 
    {
        log_e("opendir logStorageDir:%s failed", dirPath);
        return;
    }
    //!< 读取目录下的所有文件
    while(NULL != (ptr = readdir(dir)))
    {

        //过滤.和..
        if( strcmp(ptr->d_name,".")==0 || strcmp(ptr->d_name,"..") == 0 )
        {
            continue;
        }

        //!< 循环上报日志文件
        if (NULL != ptr->d_name) 
        {
            (void)adm_memset(fullFileName, 0x00, sizeof(fullFileName), sizeof(fullFileName));
            len = adm_strlen(dirPath);
            if ('/' == dirPath[len - (D32U)1]) 
            {
             (void)adm_snprintf(fullFileName, sizeof(fullFileName), "%s%s", dirPath, ptr->d_name);
            }
            else 
            {
             (void)adm_snprintf(fullFileName, sizeof(fullFileName), "%s/%s", dirPath, ptr->d_name);
            }
            D64S timestamp = time(NULL);
            adm_irm_reportFile(gstReportConfigData.sessionId, 
                               gstReportConfigData.taskId, 
                               gstReportConfigData.strategyId, 
                               timestamp,
                               gstTestConfigData.pstReportFile->startTime, 
                               gstTestConfigData.pstReportFile->endTime, 
                               gstReportConfigData.sdkVersion,
                               fullFileName, 
                               (ADM_OCS_UPLOAD_LOGFILE_ENCRYPTED_T)gstReportConfigData.isLogFileEncrypted,  
                               "fileHash",
                               12345, 
                               gstTestConfigData.pstReportFile->fileType,
                               gstTestConfigData.pstReportFile->cmdId,
                               gstTestConfigData.pstReportFile->pushId,
                               gstTestConfigData.pstReportFile->pcExtsReqJsonStr);
           i++;
        }
    }
    (void)closedir(dir);
    printf("adm_irm_CacheGreaterFile100_test report file num is:%d ", i);

}

TEST_F(CIRS_ADD_irmInitTest, Normal)
{
    CIRSTest aa;
    aa.adm_irm_init_test();
    sleep(gstTestConfigData.iSleepTime);
}

TEST_F(CIRS_ADD_irmLogFileInitTest, Normal)
{
    CIRSTest aa;
    aa.adm_irm_logFileReportInit_test();
    sleep(150);
    sleep(gstTestConfigData.iSleepTime);
}

TEST_F(CIRS_ADD_irmMessageReportTest, Normal)
{
    CIRSTest aa;
    aa.adm_irm_irmMessageReport_test();
    sleep(gstTestConfigData.iSleepTime);
}

TEST_F(CIRS_ADD_irmFileReportTest, Normal)
{
    CIRSTest aa;
    aa.adm_irm_irmFileReport_test();
    sleep(gstTestConfigData.iSleepTime);
}

TEST_F(CIRS_ADD_irmTriggerLogFileTest, Normal)
{
    CIRSTest aa;
    aa.adm_irm_triggerLogFileReport_test();
    sleep(gstTestConfigData.iSleepTime);
}

/**
 * @brief 初始化OTA配置文件
 * 
 * @param filename [in]     配置文件的绝对路径
 * @return D32S     0:Successed, None-0:Failed
 */
D32S adm_apc_otaConfigInit(const DCHAR* filename)
{
    D32S iRet = ADM_ECODE_FAILED;
    ADM_CFG_API_INFO_T stApiData = {0};

    do 
    {

        if (NULL == filename) 
        {
            iRet = ADM_ECODE_INVALID_PARAM;
            (void)printf("Input paramater [filename] is nil, iRet:%d\n", iRet);
            break;
        }

        stApiData.isFileExistCb = adm_com_isFileExist;
        stApiData.writeFileDataCb = adm_com_writeFileData;
        iRet = adm_cfg_iniParserInit(filename, &stApiData);
        if (ADM_ECODE_SUCCESSED != iRet) 
        {
            (void)printf("adm_cfg_iniParserInit %s failed, iRet:%d\n", filename, iRet);
            break;
        } 
        else 
        {
            (void)printf("adm_cfg_iniParserInit %s successed", filename);
        }
    } while(FALSE);

    return iRet;
}

void usage(void)
{
    (void)printf("Usage: admfotaapp [options]...  [param]... \n");
    (void)printf("Options:\n");
    (void)printf("  -c  filename          fota config absolute\n");
    return;
}

#ifndef _WIN32
int main(int argc, char** argv)
{
    D32S c = 0;
    D32S endMark =0;
    DCHAR* otaConfigPath = NULL;

    if (0 != adm_irmgtest_testEnvironmentInit())
    {   
        return 0;
    }
    
    //!< 接收入参
    while ((c = getopt(argc, argv, "c:")) != -1)
    {
        switch (c)
        {
            case (int)'c' :
                otaConfigPath = optarg;
                (void)printf("optarg:%s\n", optarg);
                break;
            case (int)'?':
            {
                if ((char)optopt == 'o')
                {
                    (void)fprintf(stderr, "Option -%c requires an argument.\n", optopt);
                }
                else if (0u != isprint((unsigned char)optopt))
                {
                    (void)fprintf(stderr, "Unknown option `-%c'.\n", optopt);
                }
                else
                {
                    (void)fprintf(stderr, "Unknown option character `\\x%x'.\n", optopt);
                }
                /* unknown or wrong option used, show usage information and terminate */
                endMark =-1;
                usage();
                break;
            }
            default:
            {
                endMark =-1;
                abort();
                break;
            }
        }
        if(-1 == endMark)
        {
            return -1;
        }
    }
    
    /** 1. 解析otaconfig.ini 配置文件 */
    if (ADM_ECODE_SUCCESSED != adm_apc_otaConfigInit(otaConfigPath)) 
    {
        (void)printf("adm_apc_otaConfigInit failed, iRet:%d\n", adm_apc_otaConfigInit(otaConfigPath));
        // 配置文件初始化失败，可以使用每个字段的默认配置，因此不退出程序；
    }
    else 
    {
        (void)printf("adm_apc_otaConfigInit successed\n");
    }

    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
#endif


