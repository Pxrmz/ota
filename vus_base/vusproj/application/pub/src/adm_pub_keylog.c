/**
 * @file    adm_pub_func.c
 * @brief   pub Management 
 * @details 
 * @author  wanglin
 * @version v1.0.0
 * @date    2022-05-20
 * @copyright www.abupdate.com
 */


#include "adm_pub_log.h"
#include "adm_com_lib.h"
#include "adm_com_modDefine.h"
#include "adm_com_errorCode.h"
#include "adm_cfg_api.h"
#include "adm_keyLog.h"
#include "adm_pub_keylog.h"
#include "adm_string.h"
#include "adm_memory.h"


static adm_keyLog_header* g_pKeyLogHeader = NULL;
static const DCHAR* g_pModuleName = NULL;

#define ADM_PUB_KEYLOG_LEVEL_TRACE 1
#define ADM_PUB_KEYLOG_LEVEL_DEBUG 2
#define ADM_PUB_KEYLOG_LEVEL_INFO 3
#define ADM_PUB_KEYLOG_LEVEL_WARN 4
#define ADM_PUB_KEYLOG_LEVEL_ERROR 5

typedef struct
{    
    DCHAR* keyLogId; /*!< 日志ID */
    D32S keyLogLevel; /*!< 日志级别：1:表示TRACE级别 2:表示DEBUG级别 3:表示INFO级别  4:表示WARN级别 5:表示ERROR级别*/
    DCHAR* pIfDesp;    /*!< API接口描述 */
    DCHAR* pApiIf;    /*!< API接口 */
    DCHAR* pSolution; /*!< 解决方式 */
} ADM_PUB_KEYLOG_INFO_T;

static ADM_PUB_KEYLOG_INFO_T pub_keylog_list[] = {
    {ADM_PUB_KEYLOG_ID_001, ADM_PUB_KEYLOG_LEVEL_INFO, "获取VIN码", "adm_dim_getVin", ""},
    {ADM_PUB_KEYLOG_ID_002, ADM_PUB_KEYLOG_LEVEL_ERROR, "获取VIN码", "adm_dim_getVin", "请检查VIN获取异常问题"},
    {ADM_PUB_KEYLOG_ID_003, ADM_PUB_KEYLOG_LEVEL_INFO, "获取SN信息", "adm_dim_ucl_readSnInfo", ""},
    {ADM_PUB_KEYLOG_ID_004, ADM_PUB_KEYLOG_LEVEL_ERROR, "获取SN信息", "adm_dim_ucl_readSnInfo", "请检查SN获取异常问题"},
    {ADM_PUB_KEYLOG_ID_005, ADM_PUB_KEYLOG_LEVEL_INFO, "PKI初始化", "IW_InitService", ""},
    {ADM_PUB_KEYLOG_ID_006, ADM_PUB_KEYLOG_LEVEL_ERROR, "PKI初始化", "IW_InitService", "请检查网络和PKI初始化异常问题"},
    {ADM_PUB_KEYLOG_ID_007, ADM_PUB_KEYLOG_LEVEL_INFO, "申请PKI证书","IW_ApplyCertificate", ""},
    {ADM_PUB_KEYLOG_ID_008, ADM_PUB_KEYLOG_LEVEL_ERROR, "申请PKI证书", "IW_ApplyCertificate", "请检查TSP平台是否配置车型信息"},
    {ADM_PUB_KEYLOG_ID_009, ADM_PUB_KEYLOG_LEVEL_INFO, "触发智能键升级", "adm_dim_ucl_requestDoUpgrade", ""},
    {ADM_PUB_KEYLOG_ID_010, ADM_PUB_KEYLOG_LEVEL_ERROR, "触发智能键升级", "adm_dim_ucl_requestDoUpgrade", "请检查智能键触发升级异常问题"},
    {ADM_PUB_KEYLOG_ID_011, ADM_PUB_KEYLOG_LEVEL_INFO, "获取智能键升级结果", "adm_dim_ucl_queryUpgradeResult", ""},
    {ADM_PUB_KEYLOG_ID_012, ADM_PUB_KEYLOG_LEVEL_ERROR, "获取智能键升级结果", "adm_dim_ucl_queryUpgradeResult", "请检查获取升级结果异常问题"},
    {ADM_PUB_KEYLOG_ID_013, ADM_PUB_KEYLOG_LEVEL_INFO, "智能键升级结果通知", "reflashPostResult", ""},
    {ADM_PUB_KEYLOG_ID_014, ADM_PUB_KEYLOG_LEVEL_ERROR, "智能键升级结果通知", "reflashPostResult", "请检查零件升级结果异常问题"},
    {ADM_PUB_KEYLOG_ID_015, ADM_PUB_KEYLOG_LEVEL_INFO, "触发车机升级", "adm_dim_ucl_requestDoUpgrade_icc", ""},
    {ADM_PUB_KEYLOG_ID_016, ADM_PUB_KEYLOG_LEVEL_ERROR, "触发车机升级", "adm_dim_ucl_requestDoUpgrade_icc", "请检查触发车机升级异常问题"},
    {ADM_PUB_KEYLOG_ID_017, ADM_PUB_KEYLOG_LEVEL_INFO, "车机升级结果通知","onLionOTAUpgradeFinished", ""},
    {ADM_PUB_KEYLOG_ID_018, ADM_PUB_KEYLOG_LEVEL_ERROR, "车机升级结果通知", "onLionOTAUpgradeFinished", "请检查车机升级结果异常问题"},
    {ADM_PUB_KEYLOG_ID_019, ADM_PUB_KEYLOG_LEVEL_INFO, "控制OTA模式", "adm_dim_host_setVehicleStatus", ""},
    {ADM_PUB_KEYLOG_ID_020, ADM_PUB_KEYLOG_LEVEL_ERROR, "控制OTA模式", "adm_dim_host_setVehicleStatus", "请车机检查OTA模式控制异常问题"},
    {ADM_PUB_KEYLOG_ID_021, ADM_PUB_KEYLOG_LEVEL_INFO, "控制高压模式", "adm_dim_host_setVehicleStatus", ""},
    {ADM_PUB_KEYLOG_ID_022, ADM_PUB_KEYLOG_LEVEL_ERROR, "控制高压模式", "adm_dim_host_setVehicleStatus", "请检查执行高压控制异常问题"},
    {ADM_PUB_KEYLOG_ID_023, ADM_PUB_KEYLOG_LEVEL_INFO, "设置车机升级模式", "adm_dim_host_setUpgradeMode", ""},
    {ADM_PUB_KEYLOG_ID_024, ADM_PUB_KEYLOG_LEVEL_ERROR, "设置车机升级模式", "adm_dim_host_setUpgradeMode", "请车机检查OTA升级模式控制异常问题"},
    {ADM_PUB_KEYLOG_ID_025, ADM_PUB_KEYLOG_LEVEL_INFO, "获取OTA模式", "adm_dim_host_getOtaModeStatus", ""},
    {ADM_PUB_KEYLOG_ID_026, ADM_PUB_KEYLOG_LEVEL_ERROR, "获取OTA模式", "adm_dim_host_getOtaModeStatus", "请检查获取OTA模式异常问题"},
    {ADM_PUB_KEYLOG_ID_027, ADM_PUB_KEYLOG_LEVEL_INFO, "触发远程上下电","adm_dim_host_setPower", ""},
    {ADM_PUB_KEYLOG_ID_028, ADM_PUB_KEYLOG_LEVEL_ERROR, "触发远程上下电", "adm_dim_host_setPower", "请检查触发远程上下电异常问题"},
    {ADM_PUB_KEYLOG_ID_029, ADM_PUB_KEYLOG_LEVEL_INFO, "触发非智能键升级", "adm_dim_vdi_reflashEcusTrigger", ""},
    {ADM_PUB_KEYLOG_ID_030, ADM_PUB_KEYLOG_LEVEL_ERROR, "触发非智能键升级", "adm_dim_vdi_reflashEcusTrigger", "请检查触发非智能键升级异常问题"},
    {ADM_PUB_KEYLOG_ID_031, ADM_PUB_KEYLOG_LEVEL_INFO, "获取非智能键升级结果", "adm_dim_vdi_queryReflashResult", ""},
    {ADM_PUB_KEYLOG_ID_032, ADM_PUB_KEYLOG_LEVEL_ERROR, "获取非智能键升级结果", "adm_dim_vdi_queryReflashResult", "请检查获取非智能键升级结果异常问题"},
    {ADM_PUB_KEYLOG_ID_033, ADM_PUB_KEYLOG_LEVEL_INFO, "非智能键升级结果通知", "adm_ipm_dimReflashResultSend", ""},
    {ADM_PUB_KEYLOG_ID_034, ADM_PUB_KEYLOG_LEVEL_ERROR, "非智能键升级结果通知", "adm_ipm_dimReflashResultSend", "请非智能键升级结果通知异常问题"},
    {ADM_PUB_KEYLOG_ID_035, ADM_PUB_KEYLOG_LEVEL_ERROR, "触发预/后编程", "adm_dim_vdi_programEcusTrigger", ""},
    {ADM_PUB_KEYLOG_ID_036, ADM_PUB_KEYLOG_LEVEL_ERROR, "触发预/后编程", "adm_dim_vdi_programEcusTrigger", "请触发预/后编程异常问题"},
    {ADM_PUB_KEYLOG_ID_037, ADM_PUB_KEYLOG_LEVEL_DEBUG, "读取车辆信号信息", "adm_dim_host_readVehicleStatus", ""},
    {ADM_PUB_KEYLOG_ID_038, ADM_PUB_KEYLOG_LEVEL_ERROR, "读取车辆版本信息", "adm_dim_getVehicleVersion", "请检查读取车辆版本信息异常问题"},
    {ADM_PUB_KEYLOG_ID_039, ADM_PUB_KEYLOG_LEVEL_INFO, "读取车辆版本信息", "adm_dim_getVehicleVersion", ""},
    {ADM_PUB_KEYLOG_ID_040, ADM_PUB_KEYLOG_LEVEL_ERROR, "设置车辆版本信息", "adm_dim_getVehicleVersion", "请检查设置车辆版本信息异常问题"},
    {ADM_PUB_KEYLOG_ID_041, ADM_PUB_KEYLOG_LEVEL_INFO, "设置车辆版本信息", "adm_dim_getVehicleVersion", ""}
};

void adm_pub_keylogInit(void)
{
    DBOOL bIsReport = DFALSE;

    bIsReport = adm_cfg_iniParserGetBoolean(ADM_CFG_VUS_IS_REPORT_KEYLOG, ADM_CFG_VUS_IS_REPORT_KEYLOG_DEFAULT_VAL);
    if (DTRUE == bIsReport)
    {
        /**关键日志*/
        const DCHAR* pKeylogPath =  adm_cfg_iniParserGetString(ADM_CFG_VUS_KEYLOG_PATH, ADM_CFG_VUS_KEYLOG_PATH_DEFAULT_VAL);
        if (NULL != pKeylogPath)
        {
            D32S iLogMaxSize = adm_cfg_iniParserGetInt(ADM_CFG_VUS_KEY_LOG_MAX_SIZE, ADM_CFG_VUS_KEY_LOG_MAX_SIZE_DEFAULT_VAL);

            g_pKeyLogHeader = adm_keyLog_init(pKeylogPath,iLogMaxSize);
            if (NULL != g_pKeyLogHeader)
            {
                log_i("adm_keyLog_init successed.");
            }
            else
            {
                log_e("adm_keyLog_init failed.");
            }

            (void)adm_com_createFileDirAndFile(pKeylogPath, DFALSE);
        }
    }

    g_pModuleName = adm_cfg_iniParserGetString(ADM_CFG_VUS_MODULE_NAME, ADM_CFG_VUS_MODULE_NAME_DEFAULT_VAL);
    
    return;
}

void adm_pub_setKeylogString(DCHAR* keyLogId, D32S iEreCode, const DCHAR* pDesp)
{
    DCHAR* pKeyLogStr = NULL;

    for (D32S i = 0; i < sizeof(pub_keylog_list)/ sizeof(ADM_PUB_KEYLOG_INFO_T); i++)
    {
        if (0 == adm_strcmp(keyLogId, pub_keylog_list[i].keyLogId))
        {
            pKeyLogStr = adm_com_getKeylogStr(g_pModuleName, pub_keylog_list[i].pIfDesp, pub_keylog_list[i].pApiIf, iEreCode, pDesp, pub_keylog_list[i].pSolution);
            adm_add_keyLogInfo(g_pKeyLogHeader,pKeyLogStr,adm_strlen(pKeyLogStr));
            adm_free(pKeyLogStr);
        }
    }
    return;
}


void adm_pub_rmKeylogFile()
{
    adm_keyLog_rmFile(g_pKeyLogHeader);
    
    return;
}