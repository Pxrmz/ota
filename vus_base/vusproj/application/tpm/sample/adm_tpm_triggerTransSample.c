/**
 * @file    adm_tpm_triggerTransSample.c
 * @brief   Trigger Transfer Sample 
 * @details 触发预分包测试用例
 * @author  wangyanpeng
 * @version v0.1.0
 * @date    2020-11-06
 * @copyright www.abupdate.com
 */
#include "unistd.h"
#include "adm_tpm_log.h"
#include "adm_debug.h"
#include "adm_com_lib.h"

#include "adm_com_errorCode.h"
#include "adm_com_modDefine.h"
#include "adm_engine_common.h"
#include "adm_tpm_transferManagement.h"
#include "adm_soa.h"
#include "adm_memory.h"
#include "adm_cfg_api.h"


void adm_tpm_transNotifyShow(void *pstTransResult)
{
    log_e("===> transNotify. ==========");

    return;
}



D32S main(void)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    DCHAR ip_unicast[32] = {0};
    const DCHAR* dirStr = NULL;
        
    adm_core_service_init();
    
    dirStr = adm_cfg_iniParserGetString(ADM_CFG_VUS_LOG_STORAGE_DIR, ADM_CFG_VUS_LOG_STORAGE_DIR_DEFAULT_VAL);
    adm_com_createDir(dirStr);
    adm_debug_logInit(dirStr, (1 * 1024 * 1024), 5);
    //adm_initLog();

    do {
        /** 初始化文件传输主目录 */
        dirStr = adm_cfg_iniParserGetString(ADM_CFG_EWEB_DOC_ROOT_DIR, ADM_CFG_EWEB_DOC_ROOT_DIR_DEFAULT_VAL);
        iRet = adm_com_createDir(dirStr);
        if (ADM_ECODE_SUCCESSED != iRet) {
            log_e("adm_com_createDir %s failed, iRet:%d\n", dirStr, iRet);
            break;
        } else {
            log_i("adm_com_createDir %s successed\n", dirStr);
        }

        /** 初始化文件传输的上传目录 */
        dirStr = adm_cfg_iniParserGetString(ADM_CFG_EWEB_DOC_UPLOAD_DIR, ADM_CFG_EWEB_DOC_UPLOAD_DIR_DEFAULT_VAL);
        iRet = adm_com_createDir(dirStr);
        if (ADM_ECODE_SUCCESSED != iRet) {
            log_e("adm_com_createDir %s failed, iRet:%d\n", dirStr, iRet);
            break;
        } else {
            log_i("adm_com_createDir %s successed\n", dirStr);
        }

        //!< 初始化文件传输的下载目录
        dirStr = adm_cfg_iniParserGetString(ADM_CFG_EWEB_DOC_DOWNLOAD_DIR, ADM_CFG_EWEB_DOC_DOWNLOAD_DIR_DEFAULT_VAL);
        iRet = adm_com_createDir(dirStr);
        if (ADM_ECODE_SUCCESSED != iRet) {
            log_e("adm_com_createDir %s failed, iRet:%d\n", dirStr, iRet);
            break;
        } else {
            log_i("adm_com_createDir %s successed\n", dirStr);
        }
    } while(0);


    log_e("adm_dim_soa_init entry.");
    (void)adm_memzero(ip_unicast, sizeof(ip_unicast));
    (void)adm_soa_init((void *)ip_unicast);
    log_e("adm_dim_soa_init ok.");

    log_i("Start to trigger transfer.");

    iRet = adm_tpm_triggerTransferPkg(1);
    if (ADM_IS_ERR(iRet))
    {
        log_e("Failed to trigger transfer, err[%d]", iRet);
        return iRet;
    }
    log_i("Start to trigger transfer OK.");

    while (TRUE)
    {
        sleep(60);
        log_i("========> Sample Wakeup.");
    }
    
    adm_core_service_deinit();

    return ADM_ECODE_SUCCESSED;    
}


















