#include "adm_vrm_log.h"

#include <string.h>
#include <time.h>


#include "adm_vrm_register.h"
#include "adm_vrs_registerInfo.h"
#include "adm_ocs_airProtocolAdapter.h"
#include "adm_soa.h"
#include "adm_soa_cfg.h"
#include "adm_dim_host.h"
#include "adm_cfg_api.h"
#include "adm_cfg_default.h"
#include "adm_string.h"
#include "adm_oca_connect.h"
#include "adm_ocm_interface.h"
#include "adm_vrm_event.h"

#define ADM_VRM_VIN_LENGTH  (17)

static D32S adm_vrm_getVin(DCHAR** vin)
{
    D32S iRet = ADM_ECODE_FAILED;
    D32S errCode = ADM_ECODE_FAILED;
    DBOOL isRealVehVer = FALSE;
    DCHAR evnetContext[128] = {0};
    // *vin = strdup(VEHICLE_VIN);

    do {
        isRealVehVer = adm_cfg_iniParserGetBoolean(ADM_CFG_VUS_REAL_VEHICLE_VER, (DBOOL)ADM_CFG_VUS_REAL_VEHICLE_VER_DEFAULT_VAL);
        if (TRUE == isRealVehVer) {
            //!< ZP_001
            adm_vrm_eventAdd((D32S)ADM_VRM_EV_STAGE_PRE, 1, ADM_OCS_EVENT_LEVEL_TRACE, 0, NULL);
            log_i("VUS:is_real_vehicle_ver is TRUE, get vin from dim");
            iRet = adm_dim_getVin(adm_soa_getHostInstanceId(), 3, vin, &errCode);
            if (ADM_ECODE_SUCCESSED != iRet) {
                log_e("adm_dim_soa_getVin failed, iRet:%d", iRet);
                break;
            }

            if (ADM_ECODE_SUCCESSED != errCode) {
                iRet = errCode;
                log_e("adm_dim_soa_getVin failed, errCode:%d", errCode);
                break;
            }

            if (NULL == *vin) {
                iRet = ADM_ECODE_INVALID_POINTER;
                log_e("adm_dim_soa_getVin vin is nil, iRet:%d", iRet);
                break;
            }
        } else {
            log_i("VUS:is_real_vehicle_ver is FALSE, get vin from otaconfig.ini");
            //!< ZP_002
            adm_vrm_eventAdd((D32S)ADM_VRM_EV_STAGE_PRE, 2, ADM_OCS_EVENT_LEVEL_TRACE, 0, NULL);
            *vin = adm_strdup(adm_cfg_iniParserGetString(ADM_CFG_VUS_DEF_VEHICLE_IDEN, ADM_CFG_VUS_DEF_VEHICLE_IDEN_DEFAULT_VAL));
        }
        
        if ((D32U)ADM_VRM_VIN_LENGTH != adm_strlen(*vin)) {
            iRet = ADM_ECODE_INVALID_PARAM;
            log_e("adm_vrm_getVin failed, iRet:%d", iRet);
            //!< ZP_004
            adm_vrm_eventAdd((D32S)ADM_VRM_EV_STAGE_PRE, 4, ADM_OCS_EVENT_LEVEL_ERROR, iRet, NULL);
            break;
        } else {
            //!< ZP_003
            (void)adm_memset(evnetContext, 0x00, sizeof(evnetContext), sizeof(evnetContext));
            (void)adm_snprintf(evnetContext, sizeof(evnetContext), "vin:%s", *vin);
            adm_vrm_eventAdd((D32S)ADM_VRM_EV_STAGE_PRE, 3, ADM_OCS_EVENT_LEVEL_TRACE, 0, evnetContext);
            iRet = ADM_ECODE_SUCCESSED;
            log_i("adm_vrm_getVin successed, vin:%s", *vin);
        }
    } while(FALSE);
    
    return iRet;
}

D32S adm_getSdkVersion(DCHAR** sdkVersion)
{
    DCHAR evnetContext[128] = {0};

    //!< ZP_005
    adm_vrm_eventAdd((D32S)ADM_VRM_EV_STAGE_PRE, 5, ADM_OCS_EVENT_LEVEL_TRACE, 0, NULL);
    *sdkVersion = strdup(SDK_VERSION);

    if (NULL != *sdkVersion) {
        //!< ZP_006
        (void)adm_memset(evnetContext, 0x00, sizeof(evnetContext), sizeof(evnetContext));
        (void)adm_snprintf(evnetContext, sizeof(evnetContext), "sdkVersion:%s", *sdkVersion);
        adm_vrm_eventAdd((D32S)ADM_VRM_EV_STAGE_PRE, 6, ADM_OCS_EVENT_LEVEL_TRACE, 0, evnetContext);
    }
    return ADM_ECODE_SUCCESSED;
}

D32S adm_vrm_register(const DCHAR* ext, ADM_VRS_REGISTER_RSP_INFO_T* rspInfo)
{
    D32S iRet = ADM_ECODE_FAILED;
    DCHAR* vin = NULL;
    DCHAR* sdkVersion = NULL;
    ADM_OCS_REGISTER_REQUEST_INFO_T* reqInfo = NULL;
    const DCHAR* registerUrl = NULL;

    log_i("[ABUP-Procedure] register Start");

    do {
        //!< 获取车辆VIN码
        iRet = adm_vrm_getVin(&vin);
        if (ADM_ECODE_SUCCESSED != iRet) {
            log_e("adm_vrm_getVin failed, reason of error code is %d", iRet);
            break;
        } else {
            log_i("adm_vrm_getVin successed, vin:%s", vin);
        }

        //!< 获取当前软件版本号
        iRet = adm_getSdkVersion(&sdkVersion);
        if (ADM_ECODE_SUCCESSED != iRet) {
            log_w("adm_getSdkVersion failed, reason of error code is %d", iRet);
            //!< ZP_007
            adm_vrm_eventAdd((D32S)ADM_VRM_EV_STAGE_PRE, 7, ADM_OCS_EVENT_LEVEL_ERROR, iRet, NULL);
            break;
        } else {
            log_i("adm_getSdkVersion successed, sdkVersion:%s", sdkVersion);
        }

        //!< ZP_008
        adm_vrm_eventAdd((D32S)ADM_VRM_EV_STAGE_PRE, 8, ADM_OCS_EVENT_LEVEL_TRACE, iRet, NULL);
        //!< 初始化注册请求信息
        reqInfo = adm_ocs_newRegisterRequestInfo();
        if (NULL == reqInfo) {
            iRet = ADM_ECODE_APPLY_MEMORY_FAILED;
            log_e("adm_ocs_newRegisterRequestInfo failed, reason of errror code is %d", iRet);
            //!< ZP_009
            adm_vrm_eventAdd((D32S)ADM_VRM_EV_STAGE_PRE, 9, ADM_OCS_EVENT_LEVEL_ERROR, iRet, NULL);
            break;
        }
        //!< ZP_010
        adm_vrm_eventAdd((D32S)ADM_VRM_EV_STAGE_PRE, 10, ADM_OCS_EVENT_LEVEL_DEBUG, 0, NULL);
        

        //!< ZP_011
        adm_vrm_eventAdd((D32S)ADM_VRM_EV_STAGE_PRE, 11, ADM_OCS_EVENT_LEVEL_DEBUG, 0, NULL);
        //!< 填充注册请求信息
        reqInfo->sdkVersion = adm_strdup(sdkVersion);
        reqInfo->fnBusinessCommCb = adm_ocm_businessCommInterface;
        reqInfo->fnEventAddCb = adm_vrm_eventAdd;
        reqInfo->timeStamp = time(NULL);
        reqInfo->vinCode = adm_strdup(vin);
        if (NULL != reqInfo->ext) {
            reqInfo->ext = adm_strdup(ext);
        }

        //!< 触发请求注册
        //!< ZI_006
        adm_vrm_eventAdd((D32S)ADM_VRM_EV_STAGE_ING, 6, ADM_OCS_EVENT_LEVEL_TRACE, 0, NULL);
        iRet = adm_vrs_doRegister(reqInfo, rspInfo);
        if (ADM_ECODE_SUCCESSED != iRet) {
            log_w( "adm_vrs_doRegister failed, reason of error code is %d", iRet);
            //!< S_ZE_002
            adm_vrm_eventAdd((D32S)ADM_VRM_EV_STAGE_END, 2, ADM_OCS_EVENT_LEVEL_INFO, iRet, NULL);
            break;
        } else {
            log_i( "adm_vrs_doRegister successed");
            //!< S_ZE_001
            adm_vrm_eventAdd((D32S)ADM_VRM_EV_STAGE_END, 1, ADM_OCS_EVENT_LEVEL_INFO, 0, NULL);
        }
    } while(FALSE);

    //!< 释放内存空间
    adm_ocs_delRegisterRequestInfo(reqInfo);
    if (NULL != sdkVersion) {
        adm_free(sdkVersion);
    }
    if (NULL != vin) {
        adm_free(vin);
    }
    //!< S_ZE_003
    adm_vrm_eventAdd((D32S)ADM_VRM_EV_STAGE_END, 3, ADM_OCS_EVENT_LEVEL_TRACE, 0, NULL);

    log_i("[ABUP-Procedure] register End. [iRet = %d]", iRet);

    return iRet;
}

static D32S vrmGetVehiclePowerType(ADM_VRS_VEHICLE_POWER_TYPE_E* pPowerType)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    D32S iPowerType = -1;
    D32S iRetryCnt = 0;
    D32S iCount = 0;
    D32S iErrCode = -1;
    ADM_DIM_SOA_MSGID_DATA_ARRAY_T stReqMsgData = {};
    ADM_DIM_SOA_MSGID_DATA_ARRAY_T *pstRespMsgData = NULL;

    if (ADM_IS_NULL(pPowerType))
    {
        log_e("Invlaid param.");
        return ADM_ECODE_INVALID_PARAM;
    }

    stReqMsgData.msgIdArrayLength = 1;
    stReqMsgData.msgIdArray = (ADM_DIM_SOA_MSGID_T **)adm_malloc(sizeof(ADM_DIM_SOA_MSGID_T*) * (DSIZE)stReqMsgData.msgIdArrayLength);

    (void)adm_memset(stReqMsgData.msgIdArray, 0, sizeof(ADM_DIM_SOA_MSGID_T*) * stReqMsgData.msgIdArrayLength,
                     sizeof(ADM_DIM_SOA_MSGID_T*) * stReqMsgData.msgIdArrayLength);

    stReqMsgData.msgIdArray[0] = (ADM_DIM_SOA_MSGID_T*)adm_malloc((DSIZE)sizeof(ADM_DIM_SOA_MSGID_T));

    stReqMsgData.msgIdArray[0]->msgId = VEHICLE_STATUS_POWER_TYPE;
    stReqMsgData.msgIdArray[0]->msgIdData = NULL;

    do
    {
        /** 获取车辆动力类型 */
        iRet = adm_dim_host_readVehicleStatus(adm_soa_getHostInstanceId(), 15, &stReqMsgData, &pstRespMsgData, &iErrCode);
        iRet = ADM_IS_SUCCESS(iRet) ? iErrCode : iRet;

        if (ADM_IS_ERR(iRet))
        {
            log_w("Get vehicle power type error.iRet[%d], errorCode[%d]", iRet, iErrCode);
            if (ADM_ECODE_RPC_INSTANCE_OFFLINE == iRet || ADM_ECODE_RPC_SERVICE_OFFLINE == iRet || 
                ADM_ECODE_RPC_REQSERV_TIMEOUT == iRet)
            {
                /** 对端不在线或者超时无限次重试 */
				if (ADM_ECODE_RPC_REQSERV_TIMEOUT == iRet)
				{
					/** 超时的次数适配减少 */
					sleep(3);
					if(iCount >= 15)
					{
						log_w("iCount[%d] break flow", iCount);
						iCount = 0;
						break;
					}
					else
					{
						iCount++;
	                	continue;
					}
				}
				else
				{
	                sleep(3);
					if(iCount >= 40)
					{
						log_w("iCount[%d] break flow", iCount);
						iCount = 0;
						break;
					}
					else
					{
						iCount++;
	                	continue;
					}
				}
            }
            else 
            {
                /** 普通失败尝试三次 */
                iRetryCnt++;
                break;
            }
        }
        else 
        {
            break;
        }
    } while (iRetryCnt < 3);

    if (ADM_IS_ERR(iRet))
    {
        adm_free(stReqMsgData.msgIdArray[0]);
        adm_free(stReqMsgData.msgIdArray);
        //不设置默认值，直接返回错误
        *pPowerType = VEHICLE_INVALID;
        return iRet;
    }

    if (NULL != pstRespMsgData && NULL != pstRespMsgData->msgIdArray &&
        VEHICLE_STATUS_POWER_TYPE == pstRespMsgData->msgIdArray[0]->msgId &&
        NULL != pstRespMsgData->msgIdArray[0]->msgIdData)
    {
        do
        {
            if (VAR_S32 != pstRespMsgData->msgIdArray[0]->msgIdData->dataType)
            {
                log_e("Value type invalid.type[%u]", pstRespMsgData->msgIdArray[0]->msgIdData->dataType);
                *pPowerType = VEHICLE_INVALID;     ///< 如果数据类型有误，默认无效
                iRet = ADM_ECODE_SUCCESSED;
                break;
            }
            iPowerType = pstRespMsgData->msgIdArray[0]->msgIdData->v.valueS32;
            log_i("Get vehicle power type[%d].", iPowerType);

            if (VEHICLE_INVALID < iPowerType && VEHICLE_BEV >= iPowerType)
            {
                *pPowerType = iPowerType;

                /** 增程和纯电按照混动来处理 */
                if (VEHICLE_EREV == iPowerType || VEHICLE_BEV == iPowerType)
                {
                    *pPowerType = VEHICLE_PHEV; // 1- 上电
                }
            }
            else
            {
                //不识别的结果应返回失败
                *pPowerType = VEHICLE_INVALID;
                log_e("Unrecognized power type value[%d]", iPowerType);
                iRet = ADM_ECODE_FAILED;
            }
        } while (FALSE);

        for (D16U i = 0; i < pstRespMsgData->msgIdArrayLength; i++)
        {
            if (NULL != pstRespMsgData->msgIdArray[i]->msgIdData)
            {
                if (VAR_STRING == pstRespMsgData->msgIdArray[i]->msgIdData->dataType)
                {
                    adm_free(pstRespMsgData->msgIdArray[i]->msgIdData->v.valueS);
                }
            }
            adm_free(pstRespMsgData->msgIdArray[i]->msgIdData);
            adm_free(pstRespMsgData->msgIdArray[i]);
        }
        adm_free(pstRespMsgData->msgIdArray);
        adm_free(pstRespMsgData);
    }
    else 
    {
        log_w("Get data abnormal.");
        //不设置默认值，返回失败
        *pPowerType = VEHICLE_INVALID;
        iRet = ADM_ECODE_FAILED;
    }

    adm_free(stReqMsgData.msgIdArray[0]);
    adm_free(stReqMsgData.msgIdArray);

    return iRet;
}

void adm_vrm_init(ADM_VRM_IRM_GET_EV_SEQID_CALLBACK_F getSeqIdCb, 
                                    ADM_VRM_IRM_REPORT_EVENT_CALLBACK_F reportEventCb)
{
    //!< 初始化事件上报接口
    adm_vrm_initReportEvent(getSeqIdCb, reportEventCb);

    adm_vrs_init(vrmGetVehiclePowerType);

    return;
}

/**
 * @brief 同步获取车辆动力类型（启动时使用）
 */
D32S adm_vrm_syncVehiclePowerType(DVOID)
{
    D32S iRet = ADM_ECODE_FAILED;
    DBOOL bHasDbData = FALSE;
    D32U retryCnt = 0;
    const D32U MAX_RETRY_WITH_DB = 3;       // 有数据库时的最大重试
    const D32U RETRY_INTERVAL_S = 3;   // 有数据库时的间隔
    const D32U BLOCK_RETRY_INTERVAL_S = 30; // 无数据库时的间隔

    log_i("Start sync vehicle power type...");

    // 检查数据库（通过接口获取当前值）
    ADM_VRS_VEHICLE_POWER_TYPE_E eCurrentType = adm_vrs_getVehicle();
    if (VEHICLE_INVALID < eCurrentType && VEHICLE_BEV >= eCurrentType)
    {
        bHasDbData = TRUE;
        log_i("Database has power type[%d]", eCurrentType);
    }

    // 循环获取（可能无限阻塞）
    while (TRUE)
    {
        iRet = adm_vrs_syncPowerTypeFromVehicle();
        if (ADM_IS_SUCCESS(iRet))
        {
            log_i("Success sync power type from vehicle");
            return ADM_ECODE_SUCCESSED;
        }

        retryCnt++;
        log_w("Sync power type failed, retryCnt[%u]", retryCnt);

        if (bHasDbData)
        {
            // 数据库有值：重试3次后使用数据库值
            if (retryCnt >= MAX_RETRY_WITH_DB)
            {
                log_i("Use database power type after %u retries", MAX_RETRY_WITH_DB);
                // 标记为已确认（adm_vrs_init已加载数据库值到g_ePowerType）
                adm_vrs_confirmPowerType();
                return ADM_ECODE_SUCCESSED;
            }
            sleep(RETRY_INTERVAL_S);
        }
        else
        {
            // 数据库无值：无限阻塞
            log_w("No database value, blocking and retry after %us...",
                  BLOCK_RETRY_INTERVAL_S);
            sleep(BLOCK_RETRY_INTERVAL_S);
        }
    }

    return ADM_ECODE_FAILED;  // 不会执行到这里
}

