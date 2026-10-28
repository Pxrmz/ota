#include "adm_com_modDefine.h"

#define MOD_TAG_ID          ADM_COM_MOD_VRM
#define LOG_TAG             MOD_NAME(ADM_COM_MOD_VRM)         /*!< 宏函数，用于通过模块名的枚举值类型获取模块名称     */

#include "adm_vrm_register.h"
#include "adm_debug.h"
#include "adm_engine_common.h"
#include "adm_com_lib.h"
#include "adm_tracer_interface.h"


int main(void)
{
    D32S iRet = ADM_ECODE_FAILED;
    ADM_VRS_REGISTER_RSP_INFO_T *resp =  NULL;
    // adm_initLog();

    (void)adm_com_createDir("/tmp/debug");
    adm_debug_logInit("/tmp/debug/", (1 * 1024 * 1024), 5);

    //adm_tracer_entry("VUS", modInfo);

    //log_i("xxxxxxxxxxxxxxxxxxxxx log_file_name:%s", ADM_DEBUG_FILE_PATH);
    (void)adm_core_service_init();

    resp = adm_vrs_newRegisterRspInfo();

    iRet = adm_vrm_register(NULL, resp);
    if (ADM_ECODE_SUCCESSED == iRet) {
        if (NULL != resp->ext) {
            log_i("resp->ext:%s", resp->ext);
        }
    }
    adm_vrs_delRegisterRspInfo(resp);
    
    adm_core_service_deinit();
    return 0;
}
