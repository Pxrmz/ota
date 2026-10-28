

/**
 * @file adm_hut_interface.c
 * @brief 对外接口文件
 * @details HUT模块对外接口文件
 * @author yyf
 * @version v0.1.0
 * @date 2021-3-15
 * @copyright www.abupdate.com
 */

#include "adm_hut_interface.h"
#include "adm_hut_public.h"
#include "adm_hut_manager.h"
#include "adm_vrm_register.h"
#include "adm_debug.h"
#include "adm_engine_common.h"
#include "adm_com_modDefine.h"

ADM_VRS_REGISTER_RSP_INFO_T *resp =  NULL;

/**
 * @brief 模块初始化
 * 
 * @return D32S     0:successed, non-0:failed 
 */
D32S adm_hut_init(iNodeInfo_HUT* pNodeInfo)
{
    if(NULL == pNodeInfo)
    {
        return -1;
    }
  
    D32S iRet = ADM_ECODE_FAILED;

    adm_initLog();
    adm_core_service_init();

    resp = adm_vrs_newRegisterRspInfo();

    iRet = adm_vrm_register(NULL, resp);
    if (ADM_ECODE_SUCCESSED == iRet) 
    {
        if (NULL != resp->ext) 
        {
            printf("resp->ext=%s\r\n", resp->ext);
        }
    }

    InitManager();
    
    D32S iret = 0;
    iret = StartComm(pNodeInfo);
    if(iret != 0)
    {
        return -1;
    }
    return 0;
}

/**
 * @brief 模块卸载销毁
 * 
 * @return D32S     0:successed, non-0:failed 
 */
D32S adm_hut_destroy()
{
    adm_vrs_delRegisterRspInfo(resp);
    
    adm_core_service_deinit();    
    StopComm();
    return 0;
}


