/**
 * @file    adm_dpm_triggerDownSample.c
 * @brief   Trigger Download Sample 
 * @details 触发下载测试用例
 * @author  wangyanpeng
 * @version v0.1.0
 * @date    2020-11-06
 * @copyright www.abupdate.com
 */
#include "unistd.h"
#include "adm_dpm_log.h"
#include "adm_debug.h"
#include "adm_com_lib.h"
#include "adm_com_errorCode.h"
#include "adm_com_modDefine.h"
#include "adm_dpm_downloadManagement.h"


void adm_dpm_downNotifyShow(void *pstDownResult)
{
    log_e("===> downNotify. ==========");

    return;
}



D32S main(void)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    
    adm_core_service_init();
    
    adm_com_createDir(ADM_COM_ROOT_DIR"log/dpm/");
    adm_debug_logInit(ADM_COM_ROOT_DIR"log/dpm/", (1 * 1024 * 1024), 5);
    //adm_initLog();

    log_i("Start to trigger download.");

    iRet = adm_dpm_triggerDownload(1);
    if (ADM_IS_ERR(iRet))
    {
        log_e("Failed to trigger download, err[%d]", iRet);
        return iRet;
    }
    log_i("Start to trigger download OK.");

    while (TRUE)
    {
        sleep(60);
        log_i("========> Sample Wakeup.");
    }
    
    adm_core_service_deinit();

    return ADM_ECODE_SUCCESSED;    
}


