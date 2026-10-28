#include <stdlib.h>
#include "adm_ems_serverType.h"
#include "adm_ems_log.h"
#include "adm_application_errorCode.h"
#include "adm_cfg_api.h"
#include "adm_cfg_default.h"
#include "adm_string.h"
#include "adm_memory.h"
#include "adm_com_lib.h"

static D32U g_uiServerType = ADM_EMS_SERVER_TYPE_DEFAULT;   ///< 默认环境

/**
 * @brief 更新OTA平台环境标识
 * 
 * @param[in] uiServerType     描述: \n
 * 范围值: \n
 * 单位: \n
 * 
 * @return D32S 
 */
D32S adm_ems_switchServerType(void)
{
    const DCHAR* dir = DNULL;
    const DCHAR* name = DNULL;
    DCHAR fullPath[256] = {0};
    DCHAR data[4] = {0};
    D32S iRet = ADM_ECODE_FAILED;
    D32S uiServerType = ADM_EMS_SERVER_TYPE_DEFAULT;

    dir = adm_cfg_iniParserGetString(ADM_CFG_EMS_SERVER_FLAG_FILE_DIR, ADM_CFG_EMS_SERVER_FLAG_FILE_DIR_DEFAULT_VAL);
    name = adm_cfg_iniParserGetString(ADM_CFG_EMS_SERVER_FLAG_FILE_NAME, ADM_CFG_EMS_SERVER_FLAG_FILE_NAME_DEFAULT_VAL);

    (DVOID)adm_memset(fullPath, 0x00, sizeof(fullPath), sizeof(fullPath));
    (DVOID)adm_snprintf(fullPath, sizeof(fullPath), "%s/%s", dir, name);

    log_i("current g_uiServerType is :%d", g_uiServerType);
    //!< 1 -> 2   2 -> 1
    if (ADM_EMS_SERVER_TYPE_DEFAULT == g_uiServerType)
    {
        uiServerType = ADM_EMS_SERVER_TYPE_DEFAULT_2;
    }
    else if (ADM_EMS_SERVER_TYPE_DEFAULT_2 == g_uiServerType)
    {
        uiServerType = ADM_EMS_SERVER_TYPE_DEFAULT;
    }

    adm_memset(data, 0x00, sizeof(data), sizeof(data));
    adm_snprintf(data, sizeof(data), "%d", uiServerType);
    
    iRet = adm_com_writeFileData(fullPath, data);
    if (ADM_ECODE_SUCCESSED == iRet)
    {
        ///< 写入成功之后，才需要更新
        g_uiServerType = uiServerType;
    }

    log_i("g_uiServerType switch to %d", uiServerType);

    return iRet;
}


/**
 * @brief 获取OTA平台环境标识
 * 
 * @param[out] uiServerType     描述: \n
 * 范围值: \n
 * 单位: \n
 * 
 * @return D32S 
 */
D32S adm_ems_getServerType(D32U* uiServerType)
{
    *uiServerType = g_uiServerType;

    return ADM_ECODE_SUCCESSED;
}

D32S adm_ems_serverTypeInit(void)
{
    const DCHAR* dir = DNULL;
    const DCHAR* name = DNULL;
    DCHAR fullPath[256] = {0};
    DCHAR* data = DNULL;
    D32S iRet = ADM_ECODE_FAILED;
    D32U uiServerType = ADM_EMS_SERVER_TYPE_DEFAULT;

    dir = adm_cfg_iniParserGetString(ADM_CFG_EMS_SERVER_FLAG_FILE_DIR, ADM_CFG_EMS_SERVER_FLAG_FILE_DIR_DEFAULT_VAL);
    name = adm_cfg_iniParserGetString(ADM_CFG_EMS_SERVER_FLAG_FILE_NAME, ADM_CFG_EMS_SERVER_FLAG_FILE_NAME_DEFAULT_VAL);

    (DVOID)adm_memset(fullPath, 0x00, sizeof(fullPath), sizeof(fullPath));
    (DVOID)adm_snprintf(fullPath, sizeof(fullPath), "%s/%s", dir, name);

    do
    {
        iRet = adm_com_readFileData(fullPath, &data);
        if (ADM_ECODE_SUCCESSED != iRet || DNULL == data)
        {
            log_e("fullPath:%s read failed", fullPath);
            break;
        }

        uiServerType = (D32U)atoi(data);
        if (ADM_EMS_SERVER_TYPE_DEFAULT == uiServerType || ADM_EMS_SERVER_TYPE_DEFAULT_2 == uiServerType)
        {
            g_uiServerType = uiServerType;
        }
    } while (false);
    
    return iRet;
}
