/**
 * @file    adm_icp_info.c
 * @brief   get icp info
 * @details 获取OTA平台ICP信息
 * @author  wangqi
 * @version v0.1.0
 * @date    2025-1-12
 * @copyright chery
 */
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <pthread.h>

#include "adm_file.h"
#include "adm_typedefs.h"
#include "adm_string.h"
#include "adm_list.h"
#include "adm_memory.h"
#include "adm_com_lib.h"
#include "adm_com_modDefine.h"
#include "adm_icp_info.h"
#include "adm_icp_param.h"
#include "adm_com_kvstorage.h"
#include "adm_ocs_airProtocolAdapter.h"
#include "adm_com_errorCode.h"
#include "adm_dim_host.h"
#include "adm_cfg_api.h"
#include "adm_pub_func.h"
#include "adm_icp_kvStorage.h"
#include "adm_vrs_registerInfo.h"


static ADM_ICP_MODULEINFO_T gstIcpModuleInfo = {0};

/*!
 * @brief ecm模块句柄
 */
typedef struct stICPHandle
{
    DBOOL            isInited;  //!< 模块是否初始化
    pthread_mutex_t  stMutex;   //!< 互斥锁 
} ADM_ICP_HANDLE_T;



#if 0
#endif

static ADM_ICP_HANDLE_T     gstICPHandle     = {0};


/*!
 *******************************************************************************
 * ICP基础信息参数初始化.
 *
 * \note
 * 
 * \param[out]  pstReqParam 模块基础参数
 * \retval      D32S
 *******************************************************************************
 */
static D32S __adm_icp_submitPost
(
    DCHAR  *reqMessageBody, 
    DCHAR **rspMessageBody
)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    DSIZE statusCode = 0;
    if (ADM_IS_NULL(reqMessageBody) ||ADM_IS_NULL(rspMessageBody))
    {
        log_e("Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }
    if (ADM_IS_NULL(gstIcpModuleInfo.fnReqCloudApi))
    {
        log_e("ICP module not register request cloud api!, err[%d]", ADM_ECODE_MODULE_UNINIT);
        return ADM_ECODE_MODULE_UNINIT;
    }
    do 
    {
        iRet = gstIcpModuleInfo.fnReqCloudApi(ADM_OCS_BUSINESS_TYPE_GET_ICP, reqMessageBody, rspMessageBody, &statusCode);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to submit http request, err[%d]", iRet);
            return iRet;
        }
    } while(FALSE);
    return iRet;
}

#if 0
#endif

/*!
 *******************************************************************************
 * 生成基础配置信息结构.
 *
 * \note 
 * 
 * \retval ADM_ICP_BASEINFO_T
 *******************************************************************************
 */
static ADM_ICP_BASEINFO_T *__adm_icp_newBaseIcpInfo(void)
{
    ADM_ICP_BASEINFO_T *pstBaseIcpInfo = NULL;

    pstBaseIcpInfo = (ADM_ICP_BASEINFO_T *)adm_malloc(sizeof(*pstBaseIcpInfo));
    if (NULL != pstBaseIcpInfo)
    {
        (void)adm_memset(pstBaseIcpInfo, 0x00, sizeof(*pstBaseIcpInfo), sizeof(*pstBaseIcpInfo));
    }

    return pstBaseIcpInfo;
}

/*!
 *******************************************************************************
 * 释放基础ICP信息结构.
 *
 * \note
 * 
 * \param[out]  pstBaseIcpInfo 基础icp信息
 * \retval      D32S
 *******************************************************************************
 */
static void __adm_icp_delBaseIcpInfo(ADM_ICP_BASEINFO_T **pstBaseIcpInfo)
{
    if (ADM_IS_NULL(pstBaseIcpInfo) || ADM_IS_NULL(*pstBaseIcpInfo))
    {
        return;
    }

    adm_free((*pstBaseIcpInfo)->stReqParam.secret);
    adm_free((*pstBaseIcpInfo)->stReqParam.deviceId);
    adm_free((*pstBaseIcpInfo)->stReqParam.sdkVersion);
    adm_free(*pstBaseIcpInfo);

    return;
}


/*!
 *******************************************************************************
 * icp信息初始化.
 *
 * \note
 * 
 * \param[out]  pstBaseIcpInfo ICP模块基础信息
 * \retval      D32S
 *******************************************************************************
 */
//__adm_ecs_configInit  --OK
static D32S __adm_icp_init(ADM_ICP_BASEINFO_T *pstBaseIcpInfo)
{
    if (ADM_IS_NULL(pstBaseIcpInfo))
    {
        log_e("Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }

    if (ADM_IS_NULL(gstIcpModuleInfo.fnGetReqCloudParamApi))
    {
        log_e("ICP module not reg getReqCloudParam, err[%d]", ADM_ECODE_MODULE_UNINIT);
        return ADM_ECODE_MODULE_UNINIT;
    }
    (void)gstIcpModuleInfo.fnGetReqCloudParamApi(&pstBaseIcpInfo->stReqParam);

    //!< 注册业务请求函数
    pstBaseIcpInfo->ifOpt.createMessageBody_f  = adm_icp_createMessageBody;
    pstBaseIcpInfo->ifOpt.submitPost_f         = __adm_icp_submitPost;
    pstBaseIcpInfo->ifOpt.parseMessageBody_f   = adm_icp_parseMessageBody;

    return ADM_ECODE_SUCCESSED;
}

/*!
 *******************************************************************************
 * 从平台获取ICP信息.
 *
 * \note 从平台获取ICP信息，不会存储到kv
 * 
 * \param[out]  ppstOtaIcpData    OTA平台ICP信息
 * \retval      D32S
 *******************************************************************************
 */
static D32S _adm_icp_getFromServer(ADM_ICP_RSDATA_T **ppstOtaIcpData)
{
    D32S iRet  = ADM_ECODE_SUCCESSED;
    DCHAR *reqMsgBody = NULL;
    DCHAR *rspMsgBody = NULL;
    ADM_ICP_RSDATA_T   *pstOtaIcpData = NULL;
    ADM_ICP_BASEINFO_T *pstBaseIcpInfo = NULL;
    if (ADM_IS_NULL(ppstOtaIcpData))
    {
        log_e("Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }
    do
    {   
        pstOtaIcpData = adm_icp_newRsDataInfo();
        *ppstOtaIcpData=pstOtaIcpData;
        if (ADM_IS_NULL(pstOtaIcpData))
        {
            iRet = ADM_ECODE_NO_MEMORY;
            log_e("Failed to new ota icp data info, err[%d]", iRet);            
            break;
        }

        pstBaseIcpInfo = __adm_icp_newBaseIcpInfo();
        if (ADM_IS_NULL(pstBaseIcpInfo))
        {
            iRet = ADM_ECODE_NO_MEMORY;
            log_e("Failed to new base icp info, err[%d]", ADM_ECODE_NO_MEMORY);
            break;
        }
    
        iRet = __adm_icp_init(pstBaseIcpInfo);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to init base icp info, err[%d]", iRet);
            break;
        }
        log_e("Success to init base icp info", iRet);
        if (ADM_IS_NULL(&pstBaseIcpInfo->stReqParam))
        {
            /* code */
            log_e("Failed to get stReqParam");
        }
        (pstBaseIcpInfo->stReqParam).icpVersion=pstOtaIcpData->icpVersion;
        //!< 创建消息
        iRet = pstBaseIcpInfo->ifOpt.createMessageBody_f(&pstBaseIcpInfo->stReqParam, &reqMsgBody);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to create get icp meassage body, err[%d]", iRet);
            break;
        }
        log_i("reqMsgBody: %s", reqMsgBody);

        //!< 提交消息
        iRet = pstBaseIcpInfo->ifOpt.submitPost_f(reqMsgBody, &rspMsgBody);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to submit get icp request, err[%d]", iRet);
            break;
        }
        log_i("rspMsgBody: %s", rspMsgBody);

        //!< 解析消息体
        iRet = pstBaseIcpInfo->ifOpt.parseMessageBody_f(rspMsgBody, &pstBaseIcpInfo->stReqParam, pstOtaIcpData);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to parse icp meassage body, err[%d]", iRet);
            break;
        }

    } while (FALSE);

    adm_free(reqMsgBody);
    adm_free(rspMsgBody);
    __adm_icp_delBaseIcpInfo(&pstBaseIcpInfo);

    if (ADM_IS_ERR(iRet))
    {
        adm_icp_delRsIcpDataInfo(&pstOtaIcpData);
        log_e("Failed to get rs config info from rs cloud, err[%d]", iRet);

        return iRet;
    }
    return iRet;
}

/*!
 *******************************************************************************
 * 获取ota缓存的ICP信息.
 *
 * \note 外部不需要释放此返回的缓存
 * 
 * \param   none
 * \retval  ADM_ECS_RSCONFIGDATA_T *
 *******************************************************************************
 */
static ADM_ICP_RSDATA_T * __adm_icp_getOtaCacheInfo(void)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    ADM_ICP_RSDATA_T *pstIcpData = NULL;
   
    //pstIcpData = adm_ecs_getOtaConfigDataFromKV();
      pstIcpData = adm_icp_getOtaIcpDataFromKV();
    //TODO 此处要修改成数据库读取icp信息
    if (NULL != pstIcpData)
    {
        log_i("Get ota config data from kv storage ok.");
        //return pstIcpData;
    }
    //log_i("Failed get ota config data from kv storage");

    iRet = _adm_icp_getFromServer(&pstIcpData);
    if (ADM_IS_ERR(iRet))
    {
        log_e("Failed to get ota icp data from server, err[%d]", iRet);
        return NULL;
    }
    iRet = adm_icp_setOtaIcpData2KV(pstIcpData);
    //TODO 此处要修改成数据库写入icp信息
    if (ADM_IS_ERR(iRet))
    {
        log_i("Failed to set ota icp data to kvStorage, err[%d]", iRet);
    }
    return pstIcpData;
}

/*!
 *******************************************************************************
 * 生成icp数据信息结构.
 *
 * \note 
 * 
 * \retval ADM_ICP_RSDATA_T
 *******************************************************************************
 */
ADM_ICP_RSDATA_T *adm_icp_newRsDataInfo(void)
{
    ADM_ICP_RSDATA_T *pstIcpDataInfo = NULL;

    pstIcpDataInfo = (ADM_ICP_RSDATA_T *)adm_malloc(sizeof(*pstIcpDataInfo));
    if (NULL != pstIcpDataInfo)
    {
        (void)adm_memzero(pstIcpDataInfo, sizeof(*pstIcpDataInfo));
    }

    return pstIcpDataInfo;
}

/*!
 *******************************************************************************
 * 释放ICP数据信息结构.
 *
 * \note
 * 
 * \param[in]  pstIcpDataInfo ICP数据信息
 * \retval     D32S
 *******************************************************************************
 */
void adm_icp_delRsIcpDataInfo(ADM_ICP_RSDATA_T **pstIcpDataInfo)
{
    if (ADM_IS_NULL(pstIcpDataInfo) || ADM_IS_NULL(*pstIcpDataInfo))
    {
        return;
    }

    adm_free((*pstIcpDataInfo)->icpVersion);
    adm_free((*pstIcpDataInfo)->icpInfo);
    adm_free(*pstIcpDataInfo);
    return;
}



/*!
 *******************************************************************************
 * 获取ecs模块车云协议请求参数.
 *
 * \note
 * 
 * \param[out]  pstReqParam 模块请求云协议参数
 * \retval      D32S
 *******************************************************************************
 */

//D32S adm_ecs_getReqCloudParam(ADM_ECS_REQPARAM_T *pstReqParam)
D32S adm_icp_getReqCloudParam(ADM_ICP_REQPARAM_T *pstReqParam)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    ADM_VRS_REGISTER_INFO_T stRegisterInfo = {0};

    if (ADM_IS_NULL(pstReqParam))
    {
        log_e("Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }
    (void)adm_memzero(&stRegisterInfo, sizeof(stRegisterInfo));
    iRet = adm_vrs_getRegisterInfo(&stRegisterInfo);
    if (ADM_IS_ERR(iRet))
    {
        log_e("Failed to get RegisterInfo, err[%d]", iRet);
        return iRet;
    }
    pstReqParam->secret = adm_strdup(stRegisterInfo.secret);
    pstReqParam->deviceId = adm_strdup(stRegisterInfo.deviceId);
    //TODO 此处添加从数据库中获取当前的icp版本号
    adm_free(stRegisterInfo.secret);
    adm_free(stRegisterInfo.vinCode);
    adm_free(stRegisterInfo.deviceId);

#ifdef ADM_SDK_VERSION
    pstReqParam->sdkVersion = adm_strdup(ADM_SDK_VERSION);
#else
    pstReqParam->sdkVersion = adm_strdup("V1.0.1");
#endif
    pstReqParam->timestamp   = time(NULL);

    log_d("ICP, secret:%s",      pstReqParam->secret);
    log_d("ICP, deviceId:%s",    pstReqParam->deviceId);
    log_d("ICP, sdkVersion:%s",  pstReqParam->sdkVersion);

    return iRet;
}
/*!
 *******************************************************************************
 * 填充ICP数据信息.
 *
 * \note
 * 
 * \param[out]  pstDst 
 * \param[in]   pstSrc 
 * \retval      D32S
 *******************************************************************************
 */
static D32S __adm_icp_fillRsDataInfo
(
    ADM_ICP_RSDATA_T * pstDst,
    ADM_ICP_RSDATA_T *pstSrc
)
{
    D32S iRet = ADM_ECODE_SUCCESSED;

    if (ADM_IS_NULL(pstDst) || ADM_IS_NULL(pstSrc))
    {
        log_e("Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }

    do 
    {
        //!< 拷贝版本信息 
        if (NULL != pstSrc->icpVersion)
        {
            pstDst->icpVersion = adm_strdup(pstSrc->icpVersion);
        }
       //!< 拷贝icp信息 
        if (NULL != pstSrc->icpInfo)
        {
            pstDst->icpInfo = adm_strdup(pstSrc->icpInfo);
        }
    } while (FALSE);

    if (ADM_IS_ERR(iRet))
    {
        adm_icp_delRsIcpDataInfo(&pstDst);
    }

    return iRet;
}
/*!
 *******************************************************************************
 * 更新ota缓存的平台配置信息.
 *
 * \note 函数内部会从平台重新获取一次配置信息，并将获取到的配置信息存储到kv
 *       
 * \param[inout]  pstRsCfgInfo OTA平台配置信息 --- 可选字段
 *                             --- 不为空则拷贝新的配置信息出去
 *                             --- 为空则不拷贝
 * \retval  D32S
 *******************************************************************************
 */
//D32S adm_ecs_updateAndGetConfigInfo(ADM_ECS_RSCONFIGDATA_T **ppstOtaConfigData)
D32S adm_icp_upateAndGetInfo(ADM_ICP_RSDATA_T **ppstOtaIcpData)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    DCHAR szEventBuf[ADM_COM_LEN_256] = {0};  //mod by wl 20220318
    ADM_ICP_RSDATA_T *pstRsIcpData = NULL;

    //!< 模块未初始化，则直接报错
    if (FALSE == gstIcpModuleInfo.isInited)
    {
        log_e("Icp module not init, err[%d]", ADM_ECODE_MODULE_UNINIT);
        return ADM_ECODE_MODULE_UNINIT;
    }

    //!< 从平台获取icp数据
    iRet = _adm_icp_getFromServer(&pstRsIcpData);
    if (ADM_IS_ERR(iRet))
    {
        log_e("Failed to get ota config data from server, err[%d]", iRet);
        return iRet;
    }

    //!< 更新缓存
    (void)pthread_rwlock_wrlock(&gstIcpModuleInfo.stRWlock);
    adm_icp_delRsIcpDataInfo(&gstIcpModuleInfo.pstIcpConfig);
    gstIcpModuleInfo.pstIcpConfig = pstRsIcpData;
    gstIcpModuleInfo.isDataInited = TRUE;
    (void)pthread_rwlock_unlock(&gstIcpModuleInfo.stRWlock);


    iRet = adm_icp_setOtaIcpData2KV(gstIcpModuleInfo.pstIcpConfig);
    if (ADM_IS_ERR(iRet))
    {
        log_i("Failed to set ota icp data to kvStorage, err[%d]", iRet);
    }
    //!< 不为空则拷贝到出参
    if (NULL != ppstOtaIcpData)
    {        
        *ppstOtaIcpData = adm_icp_newRsDataInfo();
        if (ADM_IS_NULL(*ppstOtaIcpData))
        {
            log_e("Failed to new icp data info, err[%d]", ADM_ECODE_NO_MEMORY);
            return ADM_ECODE_NO_MEMORY;
        }  
        iRet = __adm_icp_fillRsDataInfo(*ppstOtaIcpData, gstIcpModuleInfo.pstIcpConfig);
        if (ADM_IS_ERR(iRet))
        {
            adm_free(*ppstOtaIcpData);
            log_e("Failed to fill icp info, err[%d]", iRet);
        }
    }
    return ADM_ECODE_SUCCESSED;
}


/*!
 *******************************************************************************
 * ICP模块初始化.
 *
 * \note 需要注册对应的回调函数
 * \param[in] fnGetReqCloudParamCb      注册获取请求云端配置请求参数回调函数
 * \param[in] fnRegReqCloudCb           注册向云端获取配置请求回调函数
 * \retval    D32S                 0: 成功 non-0：失败
 *******************************************************************************
 */
D32S adm_icp_info_Init
(
    ADM_ICP_GETREQCLOUDPARAM_CALLBACK_F        fnGetReqCloudParamCb,
    ADM_OCS_BUSINESS_COMM_INTERFACE_CALLBACK_F fnRegReqCloudCb)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    ADM_ICP_RSDATA_T *pstOtaIcpInfo = NULL;
    D32S errNum = 0;

    if (ADM_IS_NULL(fnGetReqCloudParamCb) || ADM_IS_NULL(fnRegReqCloudCb))
    {
        log_e("Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }
    
    if (TRUE == gstIcpModuleInfo.isInited)
    {
        return iRet;
    }

    do
    {
        iRet = pthread_rwlock_init(&gstIcpModuleInfo.stRWlock, NULL);
        if (ADM_IS_ERR(iRet))
        {
            errNum = errno;
            iRet = ADM_ECODE_RWLOCK_INIT_FAILED;
            log_e("Failed to init rwlock, err[%d](%s)", iRet, strerror(errNum));
            break;
        }
        gstIcpModuleInfo.fnGetReqCloudParamApi = fnGetReqCloudParamCb; 
        gstIcpModuleInfo.fnReqCloudApi         = fnRegReqCloudCb; 
        gstIcpModuleInfo.isInited     = TRUE;
        gstIcpModuleInfo.pstIcpConfig = NULL;
        gstIcpModuleInfo.isDataInited = FALSE;

        //!< 尝试从kv中获取缓存
        pstOtaIcpInfo = __adm_icp_getOtaCacheInfo();
        if (NULL != pstOtaIcpInfo)
        {
            gstIcpModuleInfo.pstIcpConfig = pstOtaIcpInfo;
            gstIcpModuleInfo.isDataInited = TRUE;     
        }   
    } while (FALSE);

    return iRet;
}
/*!
 *******************************************************************************
 * icp模块销毁.
 *
 * \note
 *
 * \param   none
 * \retval  none
 *******************************************************************************
 */
void adm_icp_info_DeInit(void)
{
    if (TRUE == gstIcpModuleInfo.isInited)
    {
        (void)pthread_rwlock_wrlock(&gstIcpModuleInfo.stRWlock);
        adm_icp_delRsIcpDataInfo(&gstIcpModuleInfo.pstIcpConfig);
        gstIcpModuleInfo.pstIcpConfig = NULL;
        gstIcpModuleInfo.isDataInited = FALSE;
        (void)pthread_rwlock_unlock(&gstIcpModuleInfo.stRWlock);
        
        (void)pthread_rwlock_destroy(&gstIcpModuleInfo.stRWlock);
        gstIcpModuleInfo.isInited = FALSE;      
    }

    return;
}

/*!
 *******************************************************************************
 * icp模块初始化.
 *
 * \note
 *
 * \param   none
 * \retval  D32S    0: 成功 non-0：失败
 *******************************************************************************
 */
D32S adm_icp_moduleInit(void)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    D32S errNum = 0;

    if (TRUE == gstICPHandle.isInited)
    {
        log_i("icp module already init.");
        return iRet;
    }

    do
    {
        iRet = pthread_mutex_init(&gstICPHandle.stMutex, NULL);
        if (ADM_IS_ERR(iRet))
        {
            iRet = ADM_ECODE_RWLOCK_INIT_FAILED;
            errNum = errno;
            log_e("Failed to init mutex, err[%d](%s)", iRet, strerror(errNum));
            break;
        }
        gstICPHandle.isInited = TRUE;

        iRet = adm_icp_info_Init(adm_icp_getReqCloudParam, 
                        adm_ocm_businessCommInterface);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to init icp module, err[%d]", iRet);
            break;
        }
        log_e("Success to init icp module", iRet);

    } while (FALSE);

    if (ADM_IS_ERR(iRet))
    {
        (void)adm_icp_moduleDeInit();
    }

    log_d("===> icp init module ok.");

    return iRet;
}

/*!
 *******************************************************************************
 * icp模块销毁.
 *
 * \note
 *
 * \param   none
 * \retval  none
 *******************************************************************************
 */
void adm_icp_moduleDeInit(void)
{
    if (TRUE == gstICPHandle.isInited)
    {
        gstICPHandle.isInited = FALSE;
    
        adm_icp_info_DeInit();

        (void)pthread_mutex_destroy(&gstICPHandle.stMutex);
      
    }
    
    return;
}