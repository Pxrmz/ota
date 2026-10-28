#include "adm_ems_uim_impl.h"
#include "adm_ems_serverType.h"
#include "adm_ems_log.h"
#include "adm_com_errorCode.h"
#include "adm_dim_host.h"
#include "adm_soa.h"
#include "adm_cfg_api.h"
#include "adm_com_lib.h"
#include "adm_dim_vdi.h"
#include "adm_vrm_register.h"
#include "adm_cfg_default.h"
#include "adm_cfg_api.h"
#include "adm_ipm_install.h"


static ADM_EMS_INIT_INFO_T* g_emsInitInfo = DNULL;

#define ADM_EMS_VEHICLE_STATE_CTRL_TIME_OUT 30           ///< 车辆状态控制超时时间 5s


/**
 * @brief 车辆状态控制
 * 
 * @param[in] eCtrlType     描述:车辆状态控制类型定义 \n
 * 范围值: \n
 * 单位: \n
 * 
 * @param[out] errorCode     描述:车辆状态控制结果 0:Successed None-0:Failed \n
 * 范围值: \n
 * 单位: \n
 * 
 * @return D32S 
 */
D32S adm_ems_vehicleStateCtrlCallback(const ADM_UIM_EMS_VEHICLE_STATE_CTRL_TYPE_E eCtrlType, D32S* errorCode)
{
    ADM_DIM_SOA_HOST_OTA_TASK_TYPE_E eOtaTaskType = ADM_DIM_SOA_HOST_OTA_TASK_TYPE_INVALID;
    ADM_DIM_SOA_VDI_CTRL_VEHICLE_STATUS_TYPE_E eVehicleStatusType = VEHICLE_CTRL_INVALID;
    D32S ret = ADM_ECODE_SUCCESSED;
    D32S ret1 = ADM_ECODE_SUCCESSED;
    D32S iErrcode = ADM_ECODE_SUCCESSED;

    log_i("adm_ems_vehicleStateCtrlCallback eCtrlType = %d begin", eCtrlType);

    if (ADM_IS_NULL(errorCode))
    {
        log_e("Invalide param.");
        return ADM_ECODE_INVALID_PARAM;
    }

    switch (eCtrlType)
    {
    case ADM_UIM_EMS_VEHICLE_STATE_CTRL_HIGH_VOLTAGE_ON:
        eVehicleStatusType = VEHICLE_CTRL_ENTER_HV_MODE;
        ret = adm_dim_host_setVehicleStatus(adm_soa_getHostInstanceId(), ADM_EMS_VEHICLE_STATE_CTRL_TIME_OUT, (D32U)eVehicleStatusType, errorCode);
        break;
    case ADM_UIM_EMS_VEHICLE_STATE_CTRL_HIGH_VOLTAGE_OFF:
        eVehicleStatusType = VEHICLE_CTRL_EXIT_HV_MODE;
        ret = adm_dim_host_setVehicleStatus(adm_soa_getHostInstanceId(), ADM_EMS_VEHICLE_STATE_CTRL_TIME_OUT, (D32U)eVehicleStatusType, errorCode);
        ret1 = adm_dim_host_setVehicleStatus(adm_soa_getHostInstanceId(), ADM_EMS_VEHICLE_STATE_CTRL_TIME_OUT, (D32U)VEHICLE_CTRL_NO_ACTIVE_HV_MODE, &iErrcode);
        break;
    case ADM_UIM_EMS_VEHICLE_STATE_CTRL_OTA_MODE_ENTER:
        eVehicleStatusType = VEHICLE_CTRL_ENTER_NORMAL_OTA_MODE;
        ret = adm_dim_host_setVehicleStatus(adm_soa_getHostInstanceId(), ADM_EMS_VEHICLE_STATE_CTRL_TIME_OUT, (D32U)eVehicleStatusType, errorCode);
        break;
    case ADM_UIM_EMS_VEHICLE_STATE_CTRL_OTA_MODE_EXIT:
        eVehicleStatusType = VEHICLE_CTRL_EXIT_NORMAL_OTA_MODE;
        ret = adm_dim_host_setVehicleStatus(adm_soa_getHostInstanceId(), ADM_EMS_VEHICLE_STATE_CTRL_TIME_OUT, (D32U)eVehicleStatusType, errorCode);
        break;
    case ADM_UIM_EMS_VEHICLE_STATE_CTRL_REMOTE_POWER_ON:
        ret = adm_dim_host_setPower(adm_soa_getHostInstanceId(), ADM_EMS_VEHICLE_STATE_CTRL_TIME_OUT, (D32U)ADM_DIM_SOA_HOST_OTA_TASK_TYPE_REMOTE_POWER_ON, errorCode);
        adm_ipm_remoutePowerOnEventReport(ret);
        break;
    case ADM_UIM_EMS_VEHICLE_STATE_CTRL_REMOTE_POWER_OFF:
        ret = adm_dim_host_setPower(adm_soa_getHostInstanceId(), ADM_EMS_VEHICLE_STATE_CTRL_TIME_OUT, (D32U)ADM_DIM_SOA_HOST_OTA_TASK_TYPE_REMOTE_POWER_OFF, errorCode);
        adm_ipm_remoutePowerOffEventReport(ret);
        break;
    case ADM_UIM_EMS_VEHICLE_STATE_CTRL_EXT1:
        //TODO:
        break;
    case ADM_UIM_EMS_VEHICLE_STATE_CTRL_EXT2:
        //TODO:
        break;
    case ADM_UIM_EMS_VEHICLE_STATE_CTRL_EXT3:
        //TODO:
        break;
    case ADM_UIM_EMS_VEHICLE_STATE_CTRL_EXT4:
        //TODO:
        break;
    default:
        ret = ADM_ECODE_INVALID_PARAM;
        log_e("eCtrlType:%d is invalid, break", eCtrlType);
        break;
    }

    if (ADM_IS_SUCCESS(*errorCode))
    {
        if (ADM_IS_SUCCESS(ret))
        {
            ret = ADM_IS_ERR(ret1) ? ret1 : iErrcode;
        }
        *errorCode = ret;
    }

    log_d("eVehicleStatusType:%d, ret:%d, ret1:%d errorCode:%d", eVehicleStatusType, ret, ret1, *errorCode);

    return ret;
}

/**
 * @brief 切换OTA车端环境
 * 
 * @param[out] pServerType     描述:车端环境类型标识 1: ini中配置的默认环境; 2: ini中配置的第二套环境 \n
 * 范围值: \n
 * 单位: \n
 * 
 * @param[out] errorCode     描述:车端环境切换的结果 0:Successed None-0:Failed \n
 * 范围值: \n
 * 单位: \n
 * 
 * @return D32S 
 */
D32S adm_ems_otaServerSwitchCallback(D32U* pServerType, D32S* errorCode)
{
    D32S iRet = ADM_ECODE_FAILED;
    const DCHAR* pkiCertDir = DNULL;
    const DCHAR* kvDir = DNULL;

    iRet = adm_ems_switchServerType();
    if (ADM_ECODE_SUCCESSED != iRet)
    {
        log_e("adm_ems_switchServerType failed iRet:%d", iRet);
    }
    
    adm_ems_getServerType(pServerType);

    *errorCode = iRet;

    log_w("!!!!!!!!!!!!!!!!!!!! OTA Server switch to %d, UCM  EXIT By Engineer Mode", *pServerType);
    ///!< 服务器切换成功之后，需要退出程序
    if (DNULL != g_emsInitInfo->emsCtrlStopUcmCB)
    {
        pkiCertDir = adm_cfg_iniParserGetString(ADM_CFG_VUS_PKI_DIR, ADM_CFG_VUS_PKI_DIR_DEFAULT_VAL);
        kvDir = adm_cfg_iniParserGetString(ADM_CFG_VUS_KV_STORAGE_DIR,ADM_CFG_VUS_KV_STORAGE_DIR_DEFAULT_VAL);
        adm_com_rmDirAndFiles(pkiCertDir);
        adm_com_rmDirAndFiles(kvDir);
        log_w("!!!!!!!!!!!!!!!!!!!! OTA Server switch to %d, UCM  EXIT By Engineer Mode", *pServerType);
        iRet = g_emsInitInfo->emsCtrlStopUcmCB();
    }

    return iRet;
}

/**
 * @brief 校验 主控与OTA服务端之间连接是否OK
 * 
 * @param[out] errorCode     描述:结果 0:Successed None-0:Failed \n
 * 范围值: \n
 * 单位: \n
 * 
 * @return D32S 
 */
D32S adm_ems_otaServerConnectCallback(D32S* errorCode)
{
    D32S iRet = ADM_ECODE_FAILED;

    log_i("adm_ems_otaServerConnectCallback start");
    if (DNULL != g_emsInitInfo->emsOtaServerConnOkCB)
    {
        iRet = g_emsInitInfo->emsOtaServerConnOkCB();
    }

    *errorCode = iRet;
    log_i("adm_ems_otaServerConnectCallback result:%d", *errorCode);
    return iRet;
}


/**
 * @brief OTA 车辆软件控制接口
 * 
 * @param[in] eCtrlType     描述:OTA 车端软件控制类型 \n
 * 范围值: \n
 * 单位: \n
 * 
 * @param[out] errorCode     描述:OTA 车端软件控制结果， 0:Successed, None-0:Failed \n
 * 范围值: \n
 * 单位: \n
 * 注: ADM_UIM_EMS_OTA_APP_CTRL_CLEAN_DATA / ADM_UIM_EMS_OTA_APP_CTRL_STOP_UCM 两个类型无法返回结果哈  \n
 * 
 * 
 * @return D32S 
 */
D32S adm_ems_vehicleAppCtrlCallback(const ADM_UIM_EMS_OTA_APP_CTRL_TYPE_E eCtrlType, D32S* errorCode)
{
    D32S ret = ADM_ECODE_SUCCESSED;

    log_i("adm_ems_vehicleAppCtrlCallback eCtrlType:%d begin", eCtrlType);

    switch (eCtrlType)
    {
    case ADM_UIM_EMS_OTA_APP_CTRL_CLEAN_DATA:
        if (DNULL != g_emsInitInfo->emsResetOtaCB)
        {
            g_emsInitInfo->emsResetOtaCB();
        }
        break;
    case ADM_UIM_EMS_OTA_APP_CTRL_STOP_UCM:
        if (DNULL != g_emsInitInfo->emsCtrlStopUcmCB)
        {
            ret = g_emsInitInfo->emsCtrlStopUcmCB();
        }
        break;
    case ADM_UIM_EMS_OTA_APP_CTRL_GET_LOG:
        if (DNULL != g_emsInitInfo->emsGetOtaLogCB)
        {
            ret = g_emsInitInfo->emsGetOtaLogCB();
        }
        break;
    case ADM_UIM_EMS_OTA_APP_CTRL_DEL_PACKAGE:
        if (DNULL != g_emsInitInfo->emsDelPackageCB)
        {
            ret = g_emsInitInfo->emsDelPackageCB();
        }
        break;
    case ADM_UIM_EMS_OTA_APP_CTRL_EXT1:
        // break;
    case ADM_UIM_EMS_OTA_APP_CTRL_EXT2:
        // break;
    default:
        log_e("eCtrlType:%d is invalid, break", eCtrlType);
        break;
    }


    if (ADM_ECODE_SUCCESSED != ret)
    {
        *errorCode = ret;
    }
    log_d("eCtrlType:%d, ret:%d, errorCode:%d", eCtrlType, ret, *errorCode);

    return ret;
}


/**
 * @brief 初始化EMS模块
 * 
 * @param[in/out] pstInitInfo     描述: \n
 * 范围值: \n
 * 单位: \n
 * 
 * @param[in/out] priv     描述: \n
 * 范围值: \n
 * 单位: \n
 * 
 * @return D32S 
 */
D32S adm_ems_init(ADM_EMS_INIT_INFO_T* pstInitInfo, void* priv)
{
    if (NULL == pstInitInfo)
    {
        log_e("Input param is nil");
        return ADM_ECODE_INVALID_PARAM;
    }

    g_emsInitInfo = (ADM_EMS_INIT_INFO_T*)adm_malloc(sizeof(ADM_EMS_INIT_INFO_T));

    if (NULL != g_emsInitInfo)
    {
        g_emsInitInfo->emsResetOtaCB = pstInitInfo->emsResetOtaCB;
        g_emsInitInfo->emsGetOtaLogCB = pstInitInfo->emsGetOtaLogCB;
        g_emsInitInfo->emsDelPackageCB = pstInitInfo->emsDelPackageCB;
        g_emsInitInfo->emsOtaServerConnOkCB = pstInitInfo->emsOtaServerConnOkCB;
        g_emsInitInfo->emsCtrlStopUcmCB = pstInitInfo->emsCtrlStopUcmCB;
        g_emsInitInfo->emsCtrlExt1CB = pstInitInfo->emsCtrlExt1CB;
        g_emsInitInfo->emsCtrlExt2CB = pstInitInfo->emsCtrlExt2CB;
    }

    (DVOID)adm_ems_serverTypeInit();

    return ADM_ECODE_SUCCESSED;
}

/**
 * @brief 反初始化EMS模块
 * 
 */
void adm_ems_deinit(void)
{
    if (NULL != g_emsInitInfo)
    {
        adm_free(g_emsInitInfo);
        g_emsInitInfo = DNULL;
    }
}