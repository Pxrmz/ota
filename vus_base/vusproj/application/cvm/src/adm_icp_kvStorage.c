/**
 * @file    adm_icp_kvStorage.c
 * @brief   icp module key value storage  
 * @details icp 模块 kv 存储
 * @author  wangqi
 * @version v0.1.0
 * @date    2025-1-12
 * @copyright chery
 */
#include <stddef.h>
#include "adm_string.h"
#include "adm_memory.h"
#include "adm_com_lib.h"
#include "adm_com_errorCode.h"
#include "adm_com_modDefine.h"
#include "adm_icp_kvStorage.h"
#include "adm_engine_common.h"
#include "adm_com_kvstorage.h"


/*!
 *******************************************************************************
 * 从缓存中获取icp数据信息.
 *
 * \note
 * 
 * \param[out]  pstRsIcpData 模块配置数据信息
 * \retval      D32S
 *******************************************************************************
 */
ADM_ICP_RSDATA_T *adm_icp_getOtaIcpDataFromKV(void)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    DBOOL bIsOk = FALSE;
    size_t valLength = 0;
    KvStorage_t *pstOtaCfgKvHandle;
    ADM_ICP_RSDATA_T *pstRsIcpData = NULL;
	ADM_KV_STORAGE_VALUE_TYPE_E eValType;
    do
    {
         
        pstOtaCfgKvHandle = adm_core_service_getBusinessDatabaseDb();
        if (ADM_IS_NULL(pstOtaCfgKvHandle))
        {
            iRet = ADM_ECODE_KV_OPEN_FAILED;
            log_e("Failed to get ota config kv handle.");
 
            break;
        }

        pstRsIcpData = adm_icp_newRsDataInfo();
        if (ADM_IS_NULL(pstRsIcpData))
        {
            iRet = ADM_ECODE_NO_MEMORY;
            log_e("Failed to new ota config data info, err[%d]", iRet);
 
            break;
        }

        //!< icpVersion 
        bIsOk = adm_com_kvstorage_get_string(pstOtaCfgKvHandle, ADM_COM_OTAICP_KVSTORAGE_VER_KEY, 
                                &(pstRsIcpData->icpVersion), &valLength,&eValType);
        if (!bIsOk)
        {
            iRet = ADM_ECODE_KV_GET_STRING_FAILED;
            log_e("Failed to get adm_com_kvstorage_get_string, key[%s]", ADM_COM_OTAICP_KVSTORAGE_VER_KEY);
 
            break;
        }
         log_e("adm_com_kvstorage_get_string, key[%s],value[%s]", ADM_COM_OTAICP_KVSTORAGE_VER_KEY,pstRsIcpData->icpVersion);
 
        //!< icpInfo
        bIsOk = adm_com_kvstorage_get_string(pstOtaCfgKvHandle, ADM_COM_OTAICP_KVSTORAGE_INFO_KEY, 
                                &(pstRsIcpData->icpInfo), &valLength,&eValType);
        if (!bIsOk)
        {
            iRet = ADM_ECODE_KV_GET_STRING_FAILED;
            log_e("Failed to get adm_com_kvstorage_get_string, key[%s]", ADM_COM_OTAICP_KVSTORAGE_INFO_KEY);
            break;
        }
         log_e("adm_com_kvstorage_get_string, key[%s],value[%s]", ADM_COM_OTAICP_KVSTORAGE_INFO_KEY,pstRsIcpData->icpInfo);

    } while (FALSE);
    
    if (ADM_IS_ERR(iRet))
    {
        adm_icp_delRsIcpDataInfo(&pstRsIcpData);
        pstRsIcpData = NULL;
    }
    return pstRsIcpData;
}

/*!
 *******************************************************************************
 * 从缓存中获取ecu配置数据信息.
 *
 * \note
 * 
 * \param[out]  pstRsIcpData 模块配置数据信息
 * \retval      D32S
 *******************************************************************************
 */
D32S adm_icp_setOtaIcpData2KV(ADM_ICP_RSDATA_T *pstRsIcpData)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    DBOOL bIsOk = FALSE;
    KvStorage_t *pstOtaCfgKvHandle;

    if (ADM_IS_NULL(pstRsIcpData))
    {
        log_e("Invalid input param, err[%d]", ADM_ECODE_INVALID_PARAM);
        return ADM_ECODE_INVALID_PARAM;
    }
    do
    {
        pstOtaCfgKvHandle = adm_core_service_getBusinessDatabaseDb();
        if (ADM_IS_NULL(pstOtaCfgKvHandle))
        {
            iRet = ADM_ECODE_KV_OPEN_FAILED;

            break;
        }

        //!< icpVersion 
        bIsOk = adm_com_kvstorage_set_string(pstOtaCfgKvHandle, ADM_COM_OTAICP_KVSTORAGE_VER_KEY, 
                                pstRsIcpData->icpVersion, ADM_KV_STORAGE_VALUE_TYPE_STRING);
        if (!bIsOk)
        {
            iRet = ADM_ECODE_KV_SET_STRING_FAILED;
            log_e( "Failed to set adm_com_kvstorage_set_string, key[%s]", ADM_COM_OTAICP_KVSTORAGE_VER_KEY);

            break;
        }
        //!< icpInfo
        bIsOk = adm_com_kvstorage_set_string(pstOtaCfgKvHandle, ADM_COM_OTAICP_KVSTORAGE_INFO_KEY, 
                                pstRsIcpData->icpInfo, ADM_KV_STORAGE_VALUE_TYPE_STRING);
        if (!bIsOk)
        {
            iRet = ADM_ECODE_KV_SET_STRING_FAILED;
            log_e( "Failed to set adm_com_kvstorage_set_string, key[%s]", ADM_COM_OTAICP_KVSTORAGE_INFO_KEY);
            break;
        }
       
    } while (FALSE);
    return iRet;
}

#if 0
#endif



