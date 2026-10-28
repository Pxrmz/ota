#include "adm_ipm_log.h"
#include "adm_ipm_impl_lion.h"
#include "adm_ips_install.h"
#include "adm_ips_debug.h"
#include "adm_cvs_versionPolicy.h"
#include "adm_cvs_checkVersion.h"
#include "adm_typedefs.h"
#include "adm_com_errorCode.h"
#include "adm_com_hash.h"
#include "adm_application_errorCode.h"
#include "adm_string.h"
#include "adm_memory.h"
#include "adm_list.h"
#include "adm_uim_install.h"
#include "adm_uim_request.h"
#include "adm_dim_vdi.h"
#include "adm_dim_host.h"
#include "adm_dim_ucl.h"
#include "adm_vrs_registerInfo.h"
#include "adm_ocs_airProtocolSecurity.h"
#include "adm_soa.h"
#include "adm_vsm_fsm.h"
#include "adm_cvm_checkVersion.h"
#include "adm_cdm_verify_interface.h"
#include "adm_application_common.h"
#include "adm_pis_package.h"
#include "adm_com_kvstorage.h"
#include "adm_com_kvstorage.h"
#include "adm_irm_report.h"
#include "adm_irm_logFile.h"
#include "adm_eim_ecuInfoManager.h"
#include "adm_eim_parallel.h"
#include "adm_ecs_kvStorage.h"
#include "adm_dis_gac_priv.h"
#include "adm_uim_download.h"
#include "adm_ecs_parsePolicy.h"
#include "adm_dps_downloadService.h"
#include "adm_cfg_api.h"
#include "adm_file.h"
#include "adm_pub_func.h"
#include "adm_ocm_interface.h"
#include "adm_eim_getEcuInfo.h"
#include "adm_sdk_selfUpgrade.h"
#include "adm_sdk_version.h"
#include "adm_cvm_syncVehicleInfo.h"
#include "adm_pub_keylog.h"


#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <time.h>
#include <pthread.h>
#include "adm_soa_cfg.h"

//#define    IPM_MODDULE_NAME                     "IPM"




#define IPM_MSG_ECU_FLASH_PROGRESS_NOTIFY      1
#define IPM_MSG_ECU_FLASH_RESULT_NOTIFY        2
#define IPM_MSG_ICC_FLASH_PROGRESS_NOTIFY      3
#define IPM_MSG_ICC_FLASH_RESULT_NOTIFY        4

/* ICC及其下挂件 */
#define ICC_REQ_ID        "0x788"              /** ICC本身 */
#define ICC_SW_DID        "0xD001"
#define ICC_INSTRUMENT_PANEL_REQ_ID "0xFFF4"    /** 仪表屏 */
#define ICC_INSTRUMENT_PANEL_SW_DID "0xD001"
#define ICC_CENTRAL_CTRL_SCREEN_REQ_ID "0xFFF5" /** 中控屏 */
#define ICC_CENTRAL_CTRL_SCREEN_SW_DID "0xD001"
#define ICC_AIR_CONDITION_SCREEN_REQ_ID "0xFFF6"    /** 二排空调屏 */
#define ICC_AIR_CONDITION_SCREEN_SW_DID "0xD001"
#define ICC_AIR_CEILING_MOUNT_REQ_ID "0xFFF7" /** 吸顶屏 */
#define ICC_AIR_CEILING_MOUNT_SW_DID "0xD001"

#define	ADM_IPM_ICC_TRIGGER_INFO_KEY		"IccInstallTriggerKey"
#define HMI_PROGRESS_SIMULATE 15
static D32U g_iSimulateProgress = 0;
static DBOOL g_bIsSimulateFinished = DFALSE;

#define ICC_SOC_INSTALL_END "soc_End"
#define ICC_MCU_INSTALL_START "mcu_Start"
#define ICC_MCU_INSTALL_END "mcu_End"
#define	ADM_IPM_ICC_TRIGGER_INFO_KEY		"IccInstallTriggerKey"
static pthread_mutex_t g_stTriggerIccInfoMutex;

static D32U g_uiLastInstallGroup = -1;
static D32U g_iProgress = 0;

/*! ICC_Android和ICC_QNX刷写进度占总进度的比例(80%) */
#define ADM_IPM_ANDROID_QNX_PROGRESS_RATIO		(80)

/*! MCU刷写进度占总进度的比例(20%) */
#define ADM_IPM_MCU_PROGRESS_RATIO		(100 - ADM_IPM_ANDROID_QNX_PROGRESS_RATIO)

typedef enum ADM_IPM_REFRESH_TYPE
{
    ICC_SOC_MCU = 1,                        //ICC_SOC 和 ICC_MCU
    ICC_SOC = 2,                            //ICC_S0C
    ICC_MCU = 3,                            //ICC_MCU
    ICC_INSTRUMENT_PANEL = 4,                   //仪表屏
    ICC_CENTRAL_CTRL_SCREEN = 5,                //中控屏
    ICC_SECOND_ROW_AIR_CONDITION_SCREEN = 6,    //二排空调屏
    ICC_CEILING_MOUNTED_SCREEN = 7,              //吸顶屏
    ICC_INSTALL_INVALID = 999                   //无效值
}ADM_IPM_ICC_INSTALL_TYPE_E;

/**
 * @struct  IPM_ECU_FLASH_RESULT_T
 * @brief   
 * @details 
 */
typedef struct
{
    ADM_IPM_DIM_T*  dim;
    DCHAR           ecuReqId[ADM_IPM_DIM_ECU_MEMBER_SIZE];
    DCHAR           ecuSwDid[ADM_IPM_DIM_ECU_MEMBER_SIZE];
    DCHAR           resultDescrip[RESULT_DESCRIP_LEN];
    D32S            result;
} IPM_ECU_FLASH_RESULT_T;

typedef struct
{
    ADM_IPM_DIM_T*  dim;
    DCHAR           ecuReqId[ADM_IPM_DIM_ECU_MEMBER_SIZE];
    DCHAR           ecuSwDid[ADM_IPM_DIM_ECU_MEMBER_SIZE];
    D32S            progress;
} IPM_ECU_FLASH_PROGRESS_T;

typedef struct
{
    ADM_IPM_DIM_T*  dim;
    D32S            installType;
    D32S            progress;
} IPM_ICC_FLASH_PROGRESS_T;

typedef struct
{
    ADM_IPM_DIM_T*  dim;
    D32S            installType;
    D32S            result;
} IPM_ICC_FLASH_RESULT_T;

/*
typedef struct
{
    ADM_IPS_POLICY_DEPENDENCY_T* pDependency;   // ECU升级后检验依赖
    D32U        uiCostTime;                     // 校验ECU已花费时间
    D64U        uiLastTryTime;                  // 上次尝试校验时间
    DBOOL       bIsDone;                        // 是否已完成校验
} IPM_ECU_POSTINSTALL_DENPENDENCY_VERIFY_INFO;
*/

#define ADM_STAGE_TIME_DELAY 3   ///< 单位s
static ADM_ALL_STAGE_RESULT_INFO_T g_stAllStageResult;

static void ipmReportUpLogFilePostInstall(D32S upgradeRes);

static D32S ipmMsgQueueInit(ADM_MSG_QUEUE_T* pQueue, D32S iCapacity);
static void ipmMsgQueueDestroy(ADM_MSG_QUEUE_T* pQueue);
static D32S ipmMsgQueuePush(ADM_MSG_QUEUE_T* pQueue, ADM_MQ_MSG_T* pMsg);
static ADM_MQ_MSG_T* ipmMsgQueuePop(ADM_MSG_QUEUE_T* pQueue);

static void ipmMsgCenterInit(ADM_IPM_MSG_CENTER_T* pMsgCenter);
static void ipmMsgCenterDestroy(ADM_IPM_MSG_CENTER_T* pMsgCenter);
static D32S ipmMsgCenterSendMsg(D32S iMsgType, void* pMsgData, D32U uiMsgDataLen);
static void ipmMsgCenterProcessMsg(ADM_IPM_MSG_CENTER_T* pMsgCenter);

static D32S ipmHandleEcuFlashProgressNotify(void* pData, D32U uiDataLen);
static D32S ipmHandleEcuFlashResultNotify(void* pData, D32U uiDataLen);
static D32S ipmHandleIccFlashProgressNotify(void* pData, D32U uiDataLen);
static D32S ipmHandleIccFlashResultNotify(void* pData, D32U uiDataLen);
static D32S ipmStepInstallPrepare(void);
static void ipmReportOtaServerTaskStatusInstallPrepare(ADM_IPM_REPORT_OTA_SERVER_T *s);
static D32S ipmDoPreProgramEcus(D32U uiParallelGroup, ADM_IPM_PREGRAM_ECU_DYNARRAY_T* pstPregramEcuInfo);
static D32S ipmDoPostProgramEcus(D32U uiParallelGroup);
static void ipmDoFlashEcusListStateUpdate(ADM_IPM_DO_FLASH_T *f);
static D32S ipmDimIgSwitchCtrl(const ADM_IPM_ECU_INFO_NODE_T *node);
static DVOID ipmDimWaitHmiOnline(void);
static DCHAR* ipmIccInstallTriggerInfoGet();
static void ipmIccInstallTriggerInfoDel();
static DBOOL ipmIccInstallTriggerInfoIsExist();
static D32S ipmIccInstallTriggerInfoSet(DCHAR *reqId);
static ADM_IPM_CONTEXT_T m_ipmContext;
static DBOOL m_isInitIpmContext = FALSE;

static ADM_IPM_PROGRAM_ECU_LIST_HEAD_T g_stProgramEcuListHead = {0};

#if 0
static DVOID ipmSetUpgradeMode()
{
    D32S iRet = ADM_ECODE_FAILED;
    D32S iErrCode = ADM_ECODE_FAILED;
    D32S iCmdidType = -1;
    D32S iUpgradeMode = -1;

    ADM_COM_FUNC_ENTRY();

    /** 1:立即升级
        2:预约升级
        3: 静默升级
        4: 产线升级
        5: 救援升级
        6: USB 升级
    */
    adm_cvs_getUpgradeMode(&iCmdidType);

    log_d("iCmdidType:%d", iCmdidType);
    switch (iCmdidType)
    {
        case ADM_CVM_UPGRADE_MODE_USER_CLICK:
            /** no break */
        case ADM_CVM_UPGRADE_MODE_SLIENCE_DOWNLOAD:
            iUpgradeMode = 1;
            break;
        case ADM_CVM_UPGRADE_MODE_FORCE: 
            /** no break */
        case ADM_CVM_UPGRADE_MODE_APK_SELF:
            /** no break */
        case ADM_CVM_UPGRADE_MODE_SLIENCE_INSTALL:
            iUpgradeMode = 3;           //!< 静默升级
            break;
        case ADM_CVM_UPGRADE_MODE_PROD_LINE:
            iUpgradeMode = 4;
            break;
        case ADM_CVM_UPGRADE_MODE_RESCUE:
            iUpgradeMode = 5;
            break;
        case ADM_CVM_UPGRADE_MODE_USB_CHECK:
            iUpgradeMode = 6;
            break;

        default:
            log_w("iUpgradeMode = %d is invalid", iUpgradeMode);
            break;
    }

    log_d("iUpgradeMode:%d", iUpgradeMode);
    if (-1 != iUpgradeMode)
    {
        iRet = adm_dim_host_setUpgradeMode(adm_soa_getHostInstanceId(), 5, iUpgradeMode, &iErrCode);
        if (ADM_IS_ERR(iRet) || ADM_IS_ERR(iErrCode))
        {
            log_e("adm_dim_host_setUpgradeMode iUpgradeMode:%d failed", iUpgradeMode);
            iRet = iErrCode;
        }
        else
        {
            log_i("adm_dim_host_setUpgradeMode iUpgradeMode:%d successed", iUpgradeMode);
        }
    }

    ADM_COM_FUNC_EXIT();

    return ;
}
#endif

static D32S ipmTaskVerify(void)
{
    D32S iRet = ADM_ECODE_FAILED;
    ADM_CVS_VERSION_POLICY_T *pstVersionPolicy = NULL;
    
    ADM_COM_FUNC_ENTRY();
    
    do
    {
        pstVersionPolicy = adm_cvs_newPolicyBody();
        if (ADM_IS_NULL(pstVersionPolicy))
        {
            iRet = ADM_ECODE_NO_MEMORY;
            log_e("no memory [versionPolicy]");
            break;
        }

        iRet = adm_cvs_getVersionPolicy(pstVersionPolicy);
        if (ADM_IS_ERR(iRet))
        {
            log_e("get version policy failed!");
            break;
        }

        iRet = adm_cvs_taskVerify(pstVersionPolicy->taskId,
                                  pstVersionPolicy->strategyId,
                                  time(NULL),
                                  adm_sdk_getSDKVersion(),
                                  NULL,
                                  adm_ocm_businessCommInterface);
    } while (FALSE);

    adm_cvs_deletePolicyBody(pstVersionPolicy);

    ADM_COM_FUNC_EXIT();

    return iRet;
}

/**
 * @brief 编程请求ECU信息结构内存申请
 * @note 适用于预编程和后编程
 * @param void
 * @retval ADM_IPM_PROGRAM_ECU_INFO_T * ECU信息结构体指针
 */
static ADM_IPM_PROGRAM_ECU_INFO_T *ipmProgramEcuInfoNew()
{
    ADM_IPM_PROGRAM_ECU_INFO_T *pstEcuInfo = NULL;

    pstEcuInfo = (ADM_IPM_PROGRAM_ECU_INFO_T *)adm_malloc(sizeof(ADM_IPM_PROGRAM_ECU_INFO_T));
    if (NULL != pstEcuInfo)
    {
        adm_memzero(pstEcuInfo, sizeof(ADM_IPM_PROGRAM_ECU_INFO_T));
    }

    return pstEcuInfo;
}

/**
 * @brief 编程请求ECU信息结构内存释放
 * @note 适用于预编程和后编程
 * @param in  ADM_IPM_PROGRAM_ECU_INFO_T *pstEcuInfo ECU信息结构体指针
 * @retval void
 */
static void ipmProgramEcuInfoDel(ADM_IPM_PROGRAM_ECU_INFO_T *pstEcuInfo)
{
    if (ADM_IS_NULL(pstEcuInfo))
    {
        return;
    }

    adm_free(pstEcuInfo);

    return;
}

/**
 * @brief 编程请求参数结构内存申请
 * @note 适用于预编程和后编程
 * @param void
 * @retval ADM_IPM_PROGRAM_REQ_PARAM_T * 编程请求参数结构体指针
 */
static ADM_IPM_PROGRAM_REQ_PARAM_T *ipmProgramParamNew(void)
{
    ADM_IPM_PROGRAM_REQ_PARAM_T *pstReqParam = NULL;

    pstReqParam = (ADM_IPM_PROGRAM_REQ_PARAM_T *)adm_malloc(sizeof(ADM_IPM_PROGRAM_REQ_PARAM_T));
    if (NULL != pstReqParam)
    {
        adm_memzero(pstReqParam, sizeof(ADM_IPM_PROGRAM_REQ_PARAM_T));
    }

    return pstReqParam;
}

/**
 * @brief 编程请求参数结构内存释放
 * @note 适用于预编程和后编程
 * @param in  ADM_IPM_PROGRAM_REQ_PARAM_T *pstReqParam 编程请求参数结构体指针
 * @retval void
 */
static void ipmProgramParamDel(ADM_IPM_PROGRAM_REQ_PARAM_T *pstReqParam)
{
    if (ADM_IS_NULL(pstReqParam))
    {
        return;
    }

    adm_free(pstReqParam->pcScriptPath);
    adm_free(pstReqParam);

    return;
}

/**
 * @brief 编程ECU列表初始化
 * @note
 * @param  in   void  
 * @retval      static void
 */
static void ipmProgramEcuListInit(void)
{
    ADM_DEBUG_TRACE();

    if (FALSE == g_stProgramEcuListHead.bIsInit)
    {
        g_stProgramEcuListHead.num = 0;
        INIT_LIST_HEAD(&g_stProgramEcuListHead.head);
        g_stProgramEcuListHead.bIsInit = TRUE;
        pthread_mutex_init(&g_stProgramEcuListHead.mutex, NULL);
    }
    else
    {
        log_w("list is init!!!");
    }

    ADM_DEBUG_RETURN_VOID();
}

/**
 * @brief 编程ECU列表销毁
 * @note
 * @param  in   void  
 * @retval      static void
 */
static void ipmProgramEcuListDestory(void)
{
    ADM_IPM_PROGRAM_ECU_INFO_T *pstPos = NULL;
    ADM_IPM_PROGRAM_ECU_INFO_T *pstNext = NULL;

    ADM_DEBUG_TRACE();

    pthread_mutex_lock(&g_stProgramEcuListHead.mutex);

    list_for_each_entry_safe(pstPos, pstNext, &g_stProgramEcuListHead.head, ADM_IPM_PROGRAM_ECU_INFO_T, entry)
    {
        if (ADM_IS_NULL(pstPos))
        {
            log_e("ADM_IPM_PROGRAM_ECU_INFO_T list pos NULL");
            continue;
        }

        list_del(&pstPos->entry);
        g_stProgramEcuListHead.num--;

        ipmProgramEcuInfoDel(pstPos);
        pstPos = NULL;
    }

    pthread_mutex_unlock(&g_stProgramEcuListHead.mutex);

    ADM_DEBUG_RETURN_VOID();
}

/**
 * @brief 编程ECU列表是否为空
 * @note
 * @param  in   void  
 * @retval static DBOOL
 */
static DBOOL ipmProgramEcuListIsEmpty(void)
{
    if (0 == g_stProgramEcuListHead.num)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

/**
 * @brief 编程ECU列表添加节点
 * @note
 * @param  in   struct list_head *entry  
 * @retval void
 */
static void ipmProgramEcuListAddTail(struct list_head *entry)
{
    ADM_DEBUG_TRACE();

    pthread_mutex_lock(&g_stProgramEcuListHead.mutex);
    list_add_tail(entry, &g_stProgramEcuListHead.head);
    g_stProgramEcuListHead.num++;
    log_i("program ecu list num is %d", g_stProgramEcuListHead.num);
    pthread_mutex_unlock(&g_stProgramEcuListHead.mutex);

    ADM_DEBUG_RETURN_VOID();
}

void adm_ipm_init(void) {
    if(!m_isInitIpmContext){
    ADM_DEBUG_TRACE();
    adm_ips_init();
//    if(NULL == irsSession){
//	log_w("irsSession nullptr");
//    }
    ipmContextInit(&m_ipmContext);
    ipmAllStageResultInit(&g_stAllStageResult);
    (DVOID)ipmHmiProgressSimulateInit(adm_core_service_getBusinessDatabaseDb(), &g_iSimulateProgress);
    (void)adm_ipm_eventInfoInit();
    m_isInitIpmContext = TRUE;
    log_d("return");
    }
}
void adm_ipm_deInit(void) {
    if(m_isInitIpmContext){
    ADM_DEBUG_TRACE();
    ipmContextDeInit(&m_ipmContext);
    adm_ips_deInit();
	adm_ipm_reportEventDeInit();
    m_isInitIpmContext = FALSE;
    log_d("return");
    }
}
/*
void adm_ipm_stop() {
    adm_ipm_init();
    ipmContextStop(&m_ipmContext);
}

void adm_ipm_progressGet() {
    adm_ipm_init();
    ADM_DEBUG_TRACE();
    ipmContextProgressGet(&m_ipmContext);
    ADM_DEBUG_RETURN_VOID();
}
*/

#if 0
void adm_ipm_conditionGet(ADM_UIM_SOA_CONDITION_DATA_ARRAY_T** pStCondResults) {
    adm_ipm_init(NULL);
    ADM_DEBUG_TRACE();
    if(pStCondResults != NULL){
        ipmContextConditionGet(&m_ipmContext, pStCondResults);
    }
    ADM_DEBUG_RETURN_VOID();
}
#endif
/*!
 *******************************************************************************
 * 触发安装
 *
 * \note
 * 
 * \param[in] eTriggerType 触发安装类型
 * \retval    D32S
 *******************************************************************************
 */
D32S adm_ipm_install(ADM_APPLICATION_COMMON_CMDID_TYPE_E eTriggerType) {
    adm_ipm_init();
	adm_ips_cmdIdSet((D32S)eTriggerType);
    ADM_DEBUG_TRACE();
    ADM_DEBUG_RETURN_D32S(ipmContextInstall(&m_ipmContext));
}

/*!
 *******************************************************************************
 * vdi刷写进度回调函数
 *
 * \note
 * 
 * \param[in]   pstIdenInfo  ECU id
 * \param[in]   reflashProgress 刷写进度
 * \param[in]   priv    私有信息
 * \retval      D32S
 *******************************************************************************
 */
D32S adm_ipm_dimReflashProgressNotify(const ADM_DIM_SOA_ECU_IDEN_INFO_T* pstIdenInfo, const D16U* reflashProgress, void* priv){
    adm_ipm_init();
    return ipmDimReflashProgressNotify(pstIdenInfo, reflashProgress, priv);
}

/*!
 *******************************************************************************
 * vdi刷写结果回调函数
 *
 * \note
 * 
 * \param[in]   pstIdenInfo  ECU id
 * \param[in]   pstReflashResultSendInfo 刷写结果
 * \param[in]   priv    私有信息
 * \retval      D32S
 *******************************************************************************
 */
D32S adm_ipm_dimReflashResultSend(const ADM_DIM_SOA_ECU_IDEN_INFO_T* pstIdenInfo, const ADM_DIM_SOA_VDI_REFLASH_RESULT_INFO_T* pstReflashResultSendInfo, void* priv){
    adm_ipm_init();
    return ipmDimReflashResultSend(pstIdenInfo, pstReflashResultSendInfo, priv);
}

/*!
 *******************************************************************************
 * ucl刷写进度回调函数
 *
 * \note
 * 
 * \param[in]   pstEcuIdenInfo  ECU id信息
 * \param[in]   progress 刷写进度
 * \param[in]   priv    私有信息
 * \retval      D32S
 *******************************************************************************
 */
D32S adm_ipm_dimSoaUcmUpgradeProgressNotify(const ADM_DIM_SOA_ECU_IDEN_INFO_T* pstEcuIdenInfo, const D16U progress, void* priv){
    adm_ipm_init();
    return ipmDimSoaUcmUpgradeProgressNotify(pstEcuIdenInfo, progress, priv);
}

/*!
 *******************************************************************************
 * ucl刷写结果回调函数
 *
 * \note
 * 
 * \param[in]   pstIdenInfo  ECU id信息
 * \param[in]   pstUpgradeResultInfo 刷写结果
 * \param[in]   priv    私有信息
 * \retval      D32S
 *******************************************************************************
 */
D32S adm_ipm_dimSoaUcmUpgradeResultNotify(const ADM_DIM_SOA_ECU_IDEN_INFO_T* pstIdenInfo, const ADM_DIM_SOA_UCL_UPGRADE_RESULT_INFO_T* pstUpgradeResultInfo, void* priv){
    adm_ipm_init();
    return ipmDimSoaUcmUpgradeResultNotify(pstIdenInfo, pstUpgradeResultInfo, priv);
}

D32S adm_ipm_dimSoaUcmUpgradeIccProgressNotify(D32S installType, D32S progress, void* priv){
    adm_ipm_init();
    return ipmDimSoaUcmUpgradeIccProgressNotify(installType, progress, priv);
}

D32S adm_ipm_dimSoaUcmUpgradeIccResultNotify(D32S installType, D32S result, void* priv){
    adm_ipm_init();
    return ipmDimSoaUcmUpgradeIccResultNotify(installType, result, priv);
}

/*!
 *******************************************************************************
 * ipm全局结构体数据dim参数指针获取.
 *
 * \note 
 *
 * \retval      none
 *******************************************************************************
 */
void* adm_ipm_dimSoaUcmPrivGet(void){
    adm_ipm_init();
    return ((void*)(&m_ipmContext.dim));
}

void adm_ipm_uimSetCallback(ADM_UIM_CMD_DRIVER_CLICK_F *fnCmdDriverClick, void *priv){
    adm_ipm_init();
    ipmContextSetUimCallback(&m_ipmContext, fnCmdDriverClick, priv);
}

#if 0
static void* installTriggerAsyncHandle(void *argv){
    ipmContextInstall(&m_ipmContext);
    return NULL;
}
#endif

/*!
 *******************************************************************************
 * 用户点击触发安装回调函数
 *
 * \note
 * 
 * \param[out]   eActionType  安装是否允许
 * \param[in]   priv    私有信息
 * \retval      none
 *******************************************************************************
 */
void adm_ipm_uimInstallTriggerCallback(ADM_UIM_SOA_ACTION_AUTH_TYPE_E* eActionType, void* priv){
    DBOOL bIsTrue = FALSE;
    
    adm_ipm_init();
    //ipmUimInstallTriggerCallback(eActionType, priv);
    log_d("%s:%p, %s:%p", 
	   ADM_STR_FROM_PARAM(eActionType), eActionType,
	   ADM_STR_FROM_PARAM(priv), priv);
    if((NULL != eActionType) && (NULL != priv)){
//        ADM_IPM_UIM_T *uim = (ADM_IPM_UIM_T *)priv;
#if 1
#if 1
	bIsTrue = adm_vsm_triggerInstall();
    if (TRUE == bIsTrue)
    {
        *eActionType = ADM_UIM_SOA_ACTION_AUTH_TYPE_ALLOW;
    }
    else
    {
        *eActionType = ADM_UIM_SOA_ACTION_AUTH_TYPE_NOT_ALLOW;
    }

#else
	pthread_t pth;
	if(pthread_create(&pth, NULL, installTriggerAsyncHandle, NULL) ==0 ){
	    pthread_detach(pth);
	}
#endif
#else
#if 1
	if(uim->fnCmdDriverClick){uim->fnCmdDriverClick(uim->priv);}
#else
        ADM_IPM_EVENT_DATA_TRIGGER_T trigger;
        ipmEventDataTriggerInit(&trigger, NULL);
	ipmEventSendTrigger(uim->event, ADM_IPM_EVENT_ID_CMD_START, &trigger);
        ipmEventDataTriggerDeInit(&trigger);
#endif
#endif
    }
}

/*!
 *******************************************************************************
 * 用户点击取消安装回调函数
 *
 * \note
 * 
 * \param[in]   eCmdidType  安装触发方式
 * \param[out]   eActionType 是否允许取消安装
 * \param[in]   priv    私有信息
 * \retval      none
 *******************************************************************************
 */
void adm_ipm_uimCancelInstallCallback(const ADM_APPLICATION_COMMON_CMDID_TYPE_E eCmdidType, 
                                                    ADM_UIM_SOA_ACTION_AUTH_TYPE_E* eActionType, 
                                                    void* priv){
//    log_d("canceld install");                                                 
    adm_ipm_init();
    ADM_VSM_STATE_E state = adm_vsm_stateGet();
	if((ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_USER == eCmdidType)&&(state != ADM_VSM_STATE_UPGRADE_DOING)){
		log_d("allow canceld install\r\n"); 
        *eActionType = ADM_UIM_SOA_ACTION_AUTH_TYPE_ALLOW;
	}
	else{
		log_d("not allow canceld install"); 
        *eActionType = ADM_UIM_SOA_ACTION_AUTH_TYPE_NOT_ALLOW;
	}
	(void)ipmReportOtaServerCancelInstall(&m_ipmContext.report);
    
}

/*!
 *******************************************************************************
 * 用户拒绝安装回调函数
 *
 * \note
 * 
 * \param[in]	eCmdidType	安装触发方式
 * \param[out]	 eActionType 是否允许取消安装
 * \retval		none
 *******************************************************************************
 */
void adm_ipm_uimRefuseInstallCallback(const ADM_APPLICATION_COMMON_CMDID_TYPE_E eCmdidType, ADM_UIM_SOA_ACTION_AUTH_TYPE_E* eActionType)
{
    adm_ipm_init();

    if (ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_USER == eCmdidType)
    {
        ADM_VSM_STATE_E state = adm_vsm_stateGet();
        if (ADM_VSM_STATE_UPGRADE_DOING != state && 
            ADM_VSM_STATE_UPGRADE_PAUSE != state)
        {
            log_d("allow refuse install");
            // 清除状态机状态
            adm_vsm_stateDel();
            *eActionType = ADM_UIM_SOA_ACTION_AUTH_TYPE_ALLOW;
            return;
        }
    }

    log_d("not allow refuse install");
    *eActionType = ADM_UIM_SOA_ACTION_AUTH_TYPE_NOT_ALLOW;
}

/*!
 *******************************************************************************
 * uim请求查询所有ecu安装进度回调函数
 *
 * \note
 * 
 * \param[in]   eCmdidType  安装触发方式
 * \param[out]   sessionId 安装任务ID
 * \param[out]   state 安装状态
 * \param[out]   pstEcuInstallProgressArrayInfo 安装数组进度信息
 * \param[out]   pstPolicyInstallProgress 安装总进度信息
 * \param[out]   errorCode 查询成功/失败
 * \param[in]   priv    私有信息
 * \retval      D32S
 *******************************************************************************
 */
D32S adm_ipm_uimAllEcuInstallProgressCallback(ADM_APPLICATION_COMMON_CMDID_TYPE_E* eCmdidType, DCHAR** sessionId, DCHAR** state, ADM_UIM_SOA_ECU_INSTALL_PROGRESS_ARRAY_INFO_T** pstEcuInstallProgressArrayInfo,  ADM_UIM_SOA_INSTALL_PROGRESS_INFO_T** pstPolicyInstallProgress, D32S* errorCode, void* priv){
    adm_ipm_init();
    return ipmUimAllEcuInstallProgressCallback(eCmdidType, sessionId, state, pstEcuInstallProgressArrayInfo,  pstPolicyInstallProgress, errorCode, priv);
}

/*!
 *******************************************************************************
 * uim请求查询所有ecu安装结果回调函数
 *
 * \note
 * 
 * \param[in]   eCmdidType  安装触发类型
 * \param[out]   sessionId 安装任务ID
 * \param[out]   state 安装状态
 * \param[out]   pstEcuInstallResultArrayInfo ecu安装数组结果信息
 * \param[out]   pstPolicyInstallResult 安装result信息
 * \param[out]   errorCode 查询成功/失败
 * \param[in]   priv    私有信息
 * \retval      D32S
 *******************************************************************************
 */
D32S adm_ipm_uimAllEcuInstallResultCallback(ADM_APPLICATION_COMMON_CMDID_TYPE_E* eCmdidType, DCHAR** sessionId, DCHAR** state, ADM_UIM_SOA_ECU_INSTALL_RESULT_ARRAY_INFO_T** pstEcuInstallResultArrayInfo, ADM_UIM_SOA_INSTALL_RESULT_INFO_T** pstPolicyInstallResult, D32S* errorCode, void* priv){
    adm_ipm_init();
    return ipmUimAllEcuInstallResultCallback(eCmdidType, sessionId, state, pstEcuInstallResultArrayInfo, pstPolicyInstallResult, errorCode, priv);
}

/*!
 *******************************************************************************
 * uim触发安装前置条件检查回调函数
 *
 * \note
 * 
 * \param[in]   eVerifiedType  条件检测触发类型
 * \param[in]   cycleTime 检测周期
 * \param[in]   timeout 检测超时时间
 * \param[out]   errorCode 触发成功/失败
 * \param[in]   priv    私有信息
 * \retval      none
 *******************************************************************************
 */
void adm_ipm_uimConditionsVerifiedCallback(const ADM_UIM_SOA_CONDITIONS_VERIFIED_TYPE_E eVerifiedType, const D16U cycleTime, const D16U timeout, D32S* errorCode, void* priv){
    adm_ipm_init();
    ipmUimConditionsVerifiedCallback(eVerifiedType, cycleTime, timeout, errorCode, priv);
}



/*!
 *******************************************************************************
 * ipm全局结构体uim参数指针获取.
 *
 * \note 
 *
 * \retval      none
 *******************************************************************************
 */
void* adm_ipm_uimSoaPrivGet(void){
    adm_ipm_init();
    return ((void*)(&m_ipmContext.uim));
}

/** 预约信息存储 */
static void ipmScheduleInfoStore(const ADM_APPLICATION_SCHEDULE_INSTALL_SOURCE_E eSource, const D64S iSchduleTime, const D64S iTimeStamp)
{
    struct json_object *root = json_object_new_object();

    if (NULL != root)
    {
        struct json_object *timeVal = json_object_new_int64(iSchduleTime);
        if (NULL != timeVal)
        {
            (void)json_object_object_add(root, "scheduleTime", timeVal);
        }

        struct json_object *sourceVal = json_object_new_int((int32_t)eSource);
        if (NULL != sourceVal)
        {
            (void)json_object_object_add(root, "source", sourceVal);
        }
        struct json_object *timeStampVal = json_object_new_int64(iTimeStamp);
        if (NULL!= timeStampVal)
        {
            log_i("timeStampVal:%d", iTimeStamp);
            (void)json_object_object_add(root, "timeStamp", timeStampVal);
        }
    }

    if (json_type_null != json_object_get_type(root))
    {
        const DCHAR *schduleJsonStr = json_object_to_json_string(root);
        if(NULL != schduleJsonStr)
        {
            (void)adm_com_kvstorage_set_string(adm_core_service_getBusinessDatabaseDb(), ADM_IPM_SCHDULE_INFO_KV_KEY,
                    schduleJsonStr, ADM_KV_STORAGE_VALUE_TYPE_STRING);
        }
    }

    (void)json_object_put(root);

    return;
}

/** 是否首次预约 */
static DBOOL ipmIsFirstSchedule()
{
    if (DTRUE == adm_com_kvstorage_has_key(adm_core_service_getBusinessDatabaseDb(), ADM_IPM_SCHDULE_INFO_KV_KEY))
    {
        return DFALSE;
    }
    else 
    {
        return DTRUE;
    }
}

/** 预约信息移除 */
static void ipmScheduleInfoRemove()
{
    if (DTRUE == adm_com_kvstorage_has_key(adm_core_service_getBusinessDatabaseDb(), ADM_IPM_SCHDULE_INFO_KV_KEY))
    {
        if (DTRUE == adm_com_kvstorage_remove_key(adm_core_service_getBusinessDatabaseDb(), ADM_IPM_SCHDULE_INFO_KV_KEY))
        {
            log_i("key:[%s] has removed success.", ADM_IPM_SCHDULE_INFO_KV_KEY);
        }
        else 
        {
            log_w("key:[%s] has removed failed.", ADM_IPM_SCHDULE_INFO_KV_KEY);
        }
    }

    return;
}


DBOOL adm_ipm_isScheduleInfoExist(void)
{
    if (DTRUE == adm_com_kvstorage_has_key(adm_core_service_getBusinessDatabaseDb(), ADM_IPM_SCHDULE_INFO_KV_KEY))
    {
        log_d("key:[%s] exist.", ADM_IPM_SCHDULE_INFO_KV_KEY);
        return DTRUE;
    }
    else
    {
        log_d("key:[%s] not exist.", ADM_IPM_SCHDULE_INFO_KV_KEY);
    }

    return DFALSE;
}

D32S ipmGetScheduleInfo(D32S* pSource, D64S* pSchduleTime ,D64S* pTimeStamp)
{
    DBOOL bIsOk = FALSE;
    size_t uiValLength = 0;
    DCHAR *pSchduleInfo = NULL;
    ADM_KV_STORAGE_VALUE_TYPE_E eValType = ADM_KV_STORAGE_VALUE_TYPE_INVALID;
    json_object *jsonRoot = NULL;
    D32S nRet = ADM_ECODE_FAILED;

    if (ADM_IS_NULL(pSource) || ADM_IS_NULL(pSchduleTime))
    {
        log_e("Invalid param.");
        return ADM_ECODE_INVALID_PARAM;
    }

    do
    {
        bIsOk = adm_com_kvstorage_get_string(adm_core_service_getBusinessDatabaseDb(), ADM_IPM_SCHDULE_INFO_KV_KEY, 
                                &pSchduleInfo, &uiValLength, &eValType);
        if (!bIsOk)
        {
            log_e("Failed to get adm_com_kvstorage_get_string, key[%s]", ADM_IPM_SCHDULE_INFO_KV_KEY);
            nRet =  ADM_ECODE_KV_GET_STRING_FAILED;
            break;
        }

        if (NULL == pSchduleInfo)
        {
            log_e("key[%s] - value is null.", ADM_IPM_SCHDULE_INFO_KV_KEY);
            break;
        }

        jsonRoot = json_tokener_parse(pSchduleInfo);
        if (ADM_IS_NULL(jsonRoot)) 
        {
            nRet = ADM_ECODE_STRING_TO_JSON_FAILED;
            log_e("Failed string to json, string[%s]", pSchduleInfo);
            break;
        }

        nRet = adm_com_getJsonIntVal(jsonRoot, "source", pSource);
        if (ADM_IS_ERR(nRet))
        {
            log_e( "Failed to json value, key[source], err[%d]", nRet);
            break;
        }

        nRet = adm_com_getJsonInt64Val(jsonRoot, "scheduleTime", pSchduleTime);
        if (ADM_IS_ERR(nRet))
        {
            log_e( "Failed to json value, key[scheduleTime], err[%d]", nRet);
            break;
        }

        nRet = adm_com_getJsonInt64Val(jsonRoot, "timeStamp", pTimeStamp);
        if (ADM_IS_ERR(nRet))
        {
            log_e( "Failed to json value, key[timeStamp], err[%d]", nRet);
            break;
        }
    } while (FALSE);

    return nRet;
}


static void* asynIpmReportScheduleInfo(void* argv)
{
    D32S retVal = ADM_ECODE_FAILED;
    DCHAR* data = (DCHAR*)argv;

    if (NULL == data)
    {
        log_e("get thread data failed.");
        return NULL;
    }

    retVal = adm_ips_reportScheduleInfo(data, adm_ocm_businessCommInterface);
    if (ADM_IS_ERR(retVal))
    {
        log_e("report schedule info failed. retVal[%d]", retVal);
        return NULL;
    }

    return NULL;
}
static D32S ipmHandleScheduleInfo(
    D64S taskId,
    D32S operaType,
    D32S source,
    D64S schduleTime,
    const DCHAR* extsReqJsonStr)
{
    D32S retVal = ADM_ECODE_FAILED;
    DCHAR* generateMessageBody = NULL;
    pthread_t id;

    do 
    {
        retVal = adm_ocs_scheduleProtocolAdapter(time(NULL),
            ADM_SDK_VERSION,
            taskId,
            operaType,
            source,
            schduleTime,
            extsReqJsonStr,
            &generateMessageBody);
        if (ADM_ECODE_SUCCESSED != retVal) 
        {
            log_e("adm_ocs_scheduleProtocolAdapter error[%d]", retVal);
            break;
        }

        if (0 == pthread_create(&id, NULL, asynIpmReportScheduleInfo, (void*)generateMessageBody)) 
        {
            retVal = ADM_ECODE_SUCCESSED;
            (void)pthread_detach(id);
        } 
        else 
        {
            retVal = ADM_ECODE_FAILED;
            log_e("creat thread for report schedule info failed.");
            break;
        }
    } while (FALSE);

    return retVal;
}

/** 上报预约信息到云端 */
static D32S ipmReportScheduleInfo(const D32S iOperaType)
{
    D32S nRet = ADM_ECODE_SUCCESSED;
    ADM_IPS_POLICY_TASK_T task = {0};
    D32S iSource = -1;
    D64S iSchduleTime = -1;
    D64S iTimeStamp = -1;
    D64S iRealTimeStamp = -1;

    do
    {
        log_i("[ABUP-Procedure] Report schdule info");
        /** 数据库有预约记录才处理 */
        if (DTRUE == adm_com_kvstorage_has_key(adm_core_service_getBusinessDatabaseDb(), ADM_IPM_SCHDULE_INFO_KV_KEY))
        {
            log_i("Handle ipmReportScheduleInfo.");
            adm_ips_policyTaskInit(&task);

            if(ADM_IS_SUCCESS(adm_ips_policyTaskGet(&task)))
            {
                nRet = ipmGetScheduleInfo(&iSource, &iSchduleTime,&iTimeStamp);
                if (ADM_IS_ERR(nRet))
                {
                    break;
                }

                if ((D32S)ADM_APPLICATION_SCHEDULE_INSTALL_CREATE == iOperaType || 
                    (D32S)ADM_APPLICATION_SCHEDULE_INSTALL_EDIT == iOperaType || 
                    (D32S)ADM_APPLICATION_SCHEDULE_INSTALL_QUERY == iOperaType)
                {
                    if (ADM_APPLICATION_SCHEDULE_INSTALL_APP == iSource)
                    {
                        iRealTimeStamp = iSchduleTime;
                    }
                    else
                    {
                        iRealTimeStamp = time(NULL) + iSchduleTime;
                    }
                }
                else 
                {
                    iRealTimeStamp = 0;
                }

                nRet = ipmHandleScheduleInfo(task.taskId, iOperaType, iSource, iRealTimeStamp, NULL);
                if (ADM_IS_ERR(nRet))
                {
                    break;
                }
            }
        }
    } while (FALSE);

    if(ADM_ECODE_SUCCESSED != nRet)
    {
	    log_w("ipmReportScheduleInfo failed %d", nRet);
    }

    adm_ips_policyTaskDeInit(&task);

    return nRet;
}

void adm_ipm_uimPolicyInstallResultNotify()
{
    D32S iRet = ADM_ECODE_FAILED;
    ADM_IPS_PROGRESS_T progress;
    ADM_UIM_SOA_INSTALL_RESULT_INFO_T stPolicyInstallResult = {};
    D32S respCode = ADM_ECODE_SUCCESSED;

    log_i("adm_ipm_uimPolicyInstallResultNotify begin");

    adm_ips_progressInit(&progress);
    adm_ips_progressGet(&progress);

    stPolicyInstallResult.result = progress.result;
    stPolicyInstallResult.realVal = NULL;
    stPolicyInstallResult.expectVal = NULL;

    adm_uim_hmi_policy_installResult_RR(ADM_APPLICATION_COMMON_CMDID_TYPE_SCHDULE_RESULT, uimSoaVehicleSessionId(), uimSoaVehicleState(&progress), &stPolicyInstallResult, &respCode);
    if(ADM_ECODE_SUCCESSED != respCode)
    {
        ipmReportEventDependencyEventId("II_018", NULL, respCode);
    }

    uimSoaInstallResultInfoDeInit(&stPolicyInstallResult);

    log_i("adm_ipm_uimPolicyInstallResultNotify end");

    return ;
}


/*!
 *******************************************************************************
 * 用户触发预约安装回调函数
 *
 * \note
 * 
 * \param[in]   eCmdidType  安装触发类型
 * \param[in]   eSource 预约来源
 * \param[in]   eSchduleType 预约类型
 * \param[in]   schduleTime 预约时间
 * \param[out]   eActionType 预约是否允许
 * \param[in]   priv    私有信息
 * \retval      none
 *******************************************************************************
 */
D32S adm_ipm_uimSchduleInstallCallback(const ADM_APPLICATION_COMMON_CMDID_TYPE_E eCmdidType, 
    const ADM_APPLICATION_SCHEDULE_INSTALL_SOURCE_E eSource, const ADM_UIM_SOA_INSTALL_SCHDULE_TYPE_E eSchduleType, 
    const D64S schduleTime, ADM_UIM_SOA_ACTION_AUTH_TYPE_E* eActionType, void* priv)
{
    D32S nRet = ADM_ECODE_FAILED;
	D64S timestamp = -1; //时间戳
    D32S iOperaType = -1;
    adm_ipm_init();

    //schduleTime hmi传入的时间差
    log_d("%s:%d, %s:%d, %s:%lld, %s:%p, %s:%p", 
        ADM_STR_FROM_PARAM(eCmdidType), eCmdidType,
        ADM_STR_FROM_PARAM(eSchduleType), eSchduleType,
        ADM_STR_FROM_PARAM(schduleTime), schduleTime,
        ADM_STR_FROM_PARAM(eActionType), eActionType,
        ADM_STR_FROM_PARAM(priv), priv);
    if(eActionType == NULL)
    {
        log_e("eActionType or priv is invalid param");
        return nRet;
    }
    
//    ADM_IPM_UIM_T *uim = (ADM_IPM_UIM_T *)priv;

    
	*eActionType = ADM_UIM_SOA_ACTION_AUTH_TYPE_NOT_ALLOW;

	if(ADM_ECODE_SUCCESSED != ipmStepInstallPrepare())//预约安装时间时需要增加该动作，预约上报时需要获取task信息
	{
		log_e("ipmStepInstallPrepare failed ");
		return nRet;
	}

    if(eSchduleType == ADM_UIM_SOA_INSTALL_SCHDULE_TYPE_SCHDULED)
    {
        if (ADM_APPLICATION_SCHEDULE_INSTALL_APP == eSource)
        {
            timestamp = schduleTime;
        }
        else
        {
            timestamp = time(NULL) + schduleTime;
        }
        log_i("schduleTime:[%lld] timestamp:[%lld]", schduleTime, timestamp);
        nRet = ipmDimSchduleWakeUpTimeSet(&m_ipmContext.dim, eSchduleType, timestamp);
		if(nRet == ADM_ECODE_SUCCESSED){
			*eActionType = ADM_UIM_SOA_ACTION_AUTH_TYPE_ALLOW;
		    //上报车辆状态Install_Schduled至云端
            (void)ipmReportScheduleInstall(&m_ipmContext.report, eCmdidType, timestamp,eSource);

            /** 存储预约信息并上报云端 */
            if (DTRUE == ipmIsFirstSchedule())
            {
                iOperaType = ADM_APPLICATION_SCHEDULE_INSTALL_CREATE; 
            }
            else 
            {
                iOperaType = ADM_APPLICATION_SCHEDULE_INSTALL_EDIT;
            }
            ipmScheduleInfoStore(eSource, schduleTime, timestamp);
            (void)ipmReportScheduleInfo(iOperaType);
    	}
        else
        {
            int reason = 0;
            if (ADM_APPLICATION_SCHEDULE_INSTALL_APP == eSource)
            {
                reason = APP_INSTALL_SCHEDULED_FAILED;
                ipmReportEventDependencyEventId("S_IE_022",NULL,ADM_ECODE_SUCCESSED);
            }
            else if (ADM_APPLICATION_SCHEDULE_INSTALL_HMI == eSource)
            {
                reason = HMI_INSTALL_SCHEDULED_FAILED;
                ipmReportEventDependencyEventId("S_IE_023",NULL,ADM_ECODE_SUCCESSED);
            }
            log_i("report reason[%d]", reason);
            ipmReportOtaServerTaskScheduleInstallFailed(&m_ipmContext.report.reportOtaServer,reason);
        }
		
    }
    else if(eSchduleType == ADM_UIM_SOA_INSTALL_SCHDULE_TYPE_CANCELED)
    {
        D64S schTime = -1;//时间戳
        nRet = ipmDimSchduleWakeUpTimeGet(&m_ipmContext.dim, &schTime);//for gac, 需要判断有预约记录
        if(nRet != ADM_ECODE_SUCCESSED)
        {
            log_e("Get schdule wakeup time fail %d", nRet);
            return nRet;
        }
        
        nRet = ipmDimSchduleWakeUpTimeSet(&m_ipmContext.dim, eSchduleType, 0);//忽略参数schduleTime
        if(nRet == ADM_ECODE_SUCCESSED)
        {
            *eActionType = ADM_UIM_SOA_ACTION_AUTH_TYPE_ALLOW;

            int reason = 0;
            if (ADM_APPLICATION_SCHEDULE_INSTALL_APP == eSource)
            {
                reason = APP_INSTALL_CANCELED_SCHEDULED;
            }
            else if (ADM_APPLICATION_SCHEDULE_INSTALL_HMI == eSource)
            {
                reason = HMI_INSTALL_CANCELED_SCHEDULED;
            }
            log_i("report reason[%d]", reason);
            ipmReportOtaServerTaskScheduleInstallCancel(&m_ipmContext.report.reportOtaServer,reason);

            /** 上报云端并删除预约信息 */
            iOperaType = ADM_APPLICATION_SCHEDULE_INSTALL_REMOVE;
            (void)ipmReportScheduleInfo(iOperaType);

            ipmScheduleInfoRemove();
    	}
        else
        {
            int reason = 0;
            if (ADM_APPLICATION_SCHEDULE_INSTALL_APP == eSource)
            {
                reason = APP_INSTALL_CANCELED_SCHEDULED_FAILED;
                ipmReportEventDependencyEventId("S_IE_024",NULL,ADM_ECODE_SUCCESSED);
            }
            else if (ADM_APPLICATION_SCHEDULE_INSTALL_HMI == eSource)
            {
                reason = HMI_INSTALL_CANCELED_SCHEDULED_FAILED;
                ipmReportEventDependencyEventId("S_IE_025",NULL,ADM_ECODE_SUCCESSED);
            }
            log_i("report reason[%d]", reason);
            ipmReportOtaServerTaskScheduleInstallCancelFailed(&m_ipmContext.report.reportOtaServer,reason);
        }
        
    }
    else
    {
        nRet = ADM_ECODE_INVALID_PARAM;
        log_w("eSchduleType is invalid param");
    }

    if(nRet != ADM_ECODE_SUCCESSED)
    {
        log_e("schdule type %d fail %d", eSchduleType, nRet);
    }

    return nRet;
}

/*!
 *******************************************************************************
 * ipm通知uim所有ecu安装结果函数
 *
 * \note
 * 
 * \param[in]   none
 * \retval      D32S
 *******************************************************************************
 */
D32S adm_ipm_uimAllEcuInstallResultNotify(void){
    adm_ipm_init();
	ipmReportHmiProgress(&m_ipmContext.flash.doFlash.report->reportHmi, DTRUE);	///< 重新向HMI同步下安装进度
	return ipmContextUimAllEcuInstallResultNotify(&m_ipmContext);
}

/* ADM_IPM_CONTEXT_T */
static void ipmContextInit(ADM_IPM_CONTEXT_T *c){
    ipmDimInit(&c->dim, &c->flash.doFlash);
    ipmUimInit(&c->uim);
    ipmReportInit(&c->report);
    ipmFlashInit(&c->flash, &c->dim, &c->report);
    ipmMsgCenterInit(&m_ipmContext.msgCenter);
}

static void  ipmAllStageResultMemoryInit(ADM_ALL_STAGE_RESULT_INFO_T* pstAllStageResult)
{
    if (NULL != pstAllStageResult)
    {
        pstAllStageResult->uiIndex = 0;
        pstAllStageResult->bIsFlashSuccess = DFALSE;

        for (D16U i = 0; i < ADM_MAX_STAGE_NUMBER; i++)
        {
            pstAllStageResult->stAllStageResult[i].eState = ADM_IPM_FLASH_STATE_INVALID;
            pstAllStageResult->stAllStageResult[i].iRet = ADM_ECODE_SUCCESSED;
        }
    }

    return;
}

static void ipmAllStageResultInit(ADM_ALL_STAGE_RESULT_INFO_T* pstAllStageResult)
{
    ipmAllStageResultMemoryInit(pstAllStageResult);

    /** 加载持久化存储数据 */
    ipmAllStageResultLoad(pstAllStageResult);

    return;
}

static void ipmContextDeInit(ADM_IPM_CONTEXT_T *c){
    ipmFlashDeInit(&c->flash);
    ipmReportDeInit(&c->report);
    ipmDimDeInit(&c->dim);
    ipmUimDeInit(&c->uim);
    ipmMsgCenterDestroy(&m_ipmContext.msgCenter);
}
#if 0
static void ipmContextStop(ADM_IPM_CONTEXT_T *c){
}
static void ipmContextProgressGet(ADM_IPM_CONTEXT_T *c){
    //TODO
}

static void ipmContextConditionGet(ADM_IPM_CONTEXT_T *c, ADM_UIM_SOA_CONDITION_DATA_ARRAY_T** pStCondResults){
    ADM_COND_POLICY_T cond;
    adm_condPolicyInit(&cond);
    D32S iType = 1;/* TODO value get */
    adm_ips_policyConditionGet(iType, &cond);
    DBOOL pbIsConform = 0;
    adm_cdm_verify_Compare(&cond, pStCondResults, &pbIsConform);
    adm_condPolicyDeInit(&cond);
}
#endif
static D32S ipmContextInstall(ADM_IPM_CONTEXT_T *c){
    ADM_CVS_UPGRADE_MODE_E eUpgradeMode = ADM_CVS_UPGRADE_MODE_INVALID; 

    log_i("[ABUP-Procedure] Install Start");
    adm_cvs_getUpgradeMode((D32S *)&eUpgradeMode);
    if (ADM_CVS_UPGRADE_MODE_APK_SELF == eUpgradeMode)
    {
        log_i("[ABUP-Procedure] Install Apk Self Upgrade End");
        return ipmApkUpgradeRun(&c->flash);
    }
    else
    {
        log_i("[ABUP-Procedure] Install Flash Upgrade End");
        return ipmFlashRun(&c->flash);
    }
}
static void ipmContextSetUimCallback(ADM_IPM_CONTEXT_T *c, ADM_UIM_CMD_DRIVER_CLICK_F *fnCmdDriverClick, void *priv){
    ipmUimSetCallback(&c->uim, fnCmdDriverClick, priv);
}
static D32S ipmContextUimAllEcuInstallResultNotify(ADM_IPM_CONTEXT_T *c){
    ADM_IPS_PROGRESS_T progress;
    adm_ips_progressInit(&progress);
    adm_ips_progressGet(&progress);
    D32S ret = ipmUimAllEcuInstallResultNotify(&c->uim, &progress, c->flash.result);
    adm_ips_progressDeInit(&progress);
    return ret;
}

/* ADM_IPM_FLASH_T */
static void ipmFlashInit(ADM_IPM_FLASH_T *f, ADM_IPM_DIM_T *dim, ADM_IPM_REPORT_T *report) {
    (void)pthread_mutex_init(&f->mutex, NULL);
    f->state = ADM_IPM_FLASH_STATE_IDLE;
    f->eStage = IPM_FLASH_STAGE_EXIT;
    f->result = 0;
    log_d("state:[%s]", ipmFlashStateString(f->state));
    ipmDoFlashInit(&f->doFlash, dim, report);
    f->report = report;
    f->dim = dim;
    ipmFlashLoad(f);
    log_d("state:%s", ipmFlashStateString(f->state));

    return;
}
static void ipmFlashDeInit(ADM_IPM_FLASH_T *f){
    (void)pthread_mutex_destroy(&f->mutex);
    ipmDoFlashDeInit(&f->doFlash);

    return;
}
static D32S ipmVersionPolicyCompare(const ADM_CVS_VERSION_POLICY_T* versionPolicy, const ADM_IPS_POLICY_TASK_T *task){
    D32S ret = -1;
    if(versionPolicy->sessionId != NULL && task->sessionId != NULL){
	if(adm_strcmp(versionPolicy->sessionId, task->sessionId) == 0
		&& versionPolicy->taskId == task->taskId
		&& versionPolicy->strategyId == task->strategyId){
	    ret = 0;
	}
    }
    return ret;
}

static DCHAR *ipmInstallConditionsVerifyResultEventDesc(const ADM_UIM_SOA_CONDITION_DATA_ARRAY_T *condsArrarList, const D32S verifyResult)
{
    ADM_IPS_PROGRESS_T progress;
    adm_ips_progressInit(&progress);
    adm_ips_progressGet(&progress);
    DCHAR *sessionId = uimSoaVehicleSessionId();
    ADM_UIM_CHECK_CONDITION_RESULT_T result;
    result.eCmdidType = uimSoaCmdidTypeGet();
    result.sessionId = sessionId;
    result.state = uimSoaVehicleState(&progress);
    result.condsArrarList = condsArrarList;
    result.verifyResult = verifyResult;

    DCHAR *pReqStr = adm_uim_buildConditionCheckResponseEx(&result);

    adm_free(sessionId);
    adm_ips_progressDeInit(&progress);
    return pReqStr;
}

static DCHAR *ipmInstallConditionsVerifyResultEventDescToHmi(const ADM_UIM_SOA_CONDITION_DATA_ARRAY_T *condsArrarList,
                                                             const D32S verifyResult)
{
    ADM_IPS_PROGRESS_T progress;
    adm_ips_progressInit(&progress);
    adm_ips_progressGet(&progress);
    DCHAR *sessionId = uimSoaVehicleSessionId();
    ADM_UIM_CHECK_CONDITION_RESULT_T result;
    result.eCmdidType = uimSoaCmdidTypeGet();
    result.sessionId = sessionId;
    result.state = uimSoaVehicleState(&progress);
    result.condsArrarList = condsArrarList;
    result.verifyResult = verifyResult;

    DCHAR *pReqStr = adm_uim_buildConditionCheckResponse(&result);

    adm_free(sessionId);
    adm_ips_progressDeInit(&progress);
    return pReqStr;
}

/*
static D32S ipmFindPolicyDependencyEcuDidValue(const ADM_IPS_POLICY_ECU_DID_ARRAY_T *arrDid, DCHAR *did, DCHAR *didValue)
{
    D32S nRet = ADM_ECODE_FAILED;

    if(arrDid == NULL  || did == NULL)
    {
        log_e("arrDid[ %p ] did[ %p ]", arrDid, did);
        return ADM_ECODE_INVALID_PARAM;
    }

    ADM_IPS_POLICY_ECU_DID_T *ecuDids = NULL;
    for(D32S i = 0; i < arrDid->ecuDidsLen; i++)
    {
        ecuDids = &(arrDid->ecuDids[i]);
        if(ecuDids != NULL && ecuDids->did != NULL)
        {
            if(adm_strcasecmp(ecuDids->did, did) == 0 && ecuDids->didValue != NULL)
            {
                didValue = adm_strdup(ecuDids->didValue);
                nRet = ADM_ECODE_SUCCESSED;
                log_i("find success");
                break;
            }
        }
    }

    return nRet;
}

static D32S ipmFindPolicyDependencyDidValue(const ADM_IPS_POLICY_DEPENDENCY_T *dependency, DCHAR *ecuReqId, DCHAR *did, DCHAR *didValue)
{
    D32S nRet = ADM_ECODE_FAILED;

    if(dependency == NULL || dependency->ecuReqId == json_type_null || ecuReqId == NULL || did == NULL)
    {
        log_e("dependency[ %p ] dependency->ecuReqId[ %p ] ecuReqId[ %p ] did[ %p ]", dependency, dependency->ecuReqId, ecuReqId, did);
        return ADM_ECODE_INVALID_PARAM;
    }

    if(adm_strcmp(dependency->ecuReqId, ecuReqId) != 0)
    {
        log_e("EcuReqId don't match, dependency->ecuReqId[ %s ] ecuReqId[ %s ]", dependency->ecuReqId, ecuReqId);
        return ADM_ECODE_FAILED;
    }

    return ipmFindPolicyDependencyEcuDidValue(&(dependency->ecuDids), did, didValue);
}

static const DCHAR * ipmPolicyDependencyGetDidValue(const ADM_IPS_POLICY_DEPENDENCY_T* dependency, const DCHAR* did)
{
    DCHAR* didValue = NULL;

    if (NULL == dependency || NULL == did)
    {
        log_e("dependency[ %p ] did[ %p ]", dependency, did);
        return NULL;
    }

    const ADM_IPS_POLICY_ECU_DID_T* ecuDids = NULL;
    const ADM_IPS_POLICY_ECU_DID_ARRAY_T* arrDid = &dependency->ecuDids;
    D32S didCount = arrDid->ecuDidsLen;
    for (D32S i = 0; i < didCount; i++)
    {
        ecuDids = &arrDid->ecuDids[i];
        if (NULL != ecuDids && NULL != ecuDids->did)
        {
            if (0 == adm_strcasecmp(ecuDids->did, did) && NULL != ecuDids->didValue)
            {
                didValue = adm_strdup(ecuDids->didValue);
                break;
            }
        }
    }

    return didValue;
}

static D32S ipmFindPolicyDependencyArrayDidValue(const ADM_IPS_POLICY_DEPENDENCY_ARRAY_T *arrDependency, DCHAR *ecuReqId, DCHAR *did, DCHAR *didValue)
{
    D32S nRet = ADM_ECODE_FAILED;

    if(arrDependency == NULL || ecuReqId == NULL || did == NULL)
    {
        log_e("arrDependency[ %p ] ecuReqId[ %p ] did[ %p ]", arrDependency, ecuReqId, did);
        return ADM_ECODE_INVALID_PARAM;
    }

    ADM_IPS_POLICY_DEPENDENCY_T *dependencies = NULL;
    for(D32S i = 0; i < arrDependency->dependenciesLen; i++)
    {
        dependencies = &(arrDependency->dependencies[i]);
        if(dependencies != NULL)
        {
            nRet = ipmFindPolicyDependencyDidValue(dependencies, ecuReqId, did, didValue);
            if(nRet == ADM_ECODE_SUCCESSED)
            {
                log_i("find success");
                break;
            }
        }
    }

    return nRet;
}
*/
#if 0
static D32S ipmFindDidValue(const ADM_DIM_SOA_DID_DATA_ARRAY_T *arrDid, DCHAR *did, DCHAR *didValue)
{
    D32S nRet = ADM_ECODE_FAILED;

    if(arrDid == NULL  || did == NULL)
    {
        log_e("arrDid[ %p ] did[ %p ]", arrDid, did);
        return ADM_ECODE_INVALID_PARAM;
    }

    ADM_DIM_SOA_DID_DATA_T* didArray = NULL;
    for(D16U i = 0; i < arrDid->didArrayLength; i++)
    {
        didArray = arrDid->didArray[i];
        if(didArray != NULL)
        {
            if((D32S)htoi(did) == didArray->did && didArray->didData != NULL)
            {
                switch (didArray->didData->dataType)//GAC_TODO: 其他类型
                {
                case VAR_U8:
                    /* code */
                    break;
                case VAR_U32:
                    /* code */
                    break;
                case VAR_S32:
                    /* code */
                    break;
                case VAR_U64:
                    /* code */
                    break;
                case VAR_FLOAT:
                    /* code */
                    break;
                case VAR_STRING:
                    /* code */
                    didValue = adm_strdup(didArray->didData->v.valueS);
                    break;
                
                default:
                    break;
                } 
            }
            nRet = ADM_ECODE_SUCCESSED;
            break;
        }
    }

    return nRet;
}
#endif
/*
static D32S ipmFindEcuInfoDidValue(const ECU_INFO_T* ecuInfo, DCHAR *ecuReqId, DCHAR *did, DCHAR *didValue)
{
    D32S nRet = ADM_ECODE_FAILED;

    if(ecuInfo == NULL || ecuInfo->ecuReqId == NULL || ecuReqId == NULL || did == NULL)
    {
        log_e("ecuInfo[ %p ] ecuInfo->ecuReqId[ %p ] ecuReqId[ %p ] did[ %p ]", ecuInfo, ecuInfo->ecuReqId, ecuReqId, did);
        return ADM_ECODE_INVALID_PARAM;
    }

    if(adm_strcmp(ecuInfo->ecuReqId, ecuReqId) != 0)
    {
        log_e("EcuReqId don't match, ecuInfo->ecuReqId[ %s ] ecuReqId[ %s ]", ecuInfo->ecuReqId, ecuReqId);
        return ADM_ECODE_FAILED;
    }

    if(ecuInfo->vdiDidListResponse != NULL)
    {
        nRet = ipmFindDidValue(ecuInfo->vdiDidListResponse, did, didValue);
    }

    return nRet;
}

static D32S ipmFindEcusInfoDidValue(const ADM_EIM_GETECUINFO_T* allEcuInfo, DCHAR* ecuReqId, DCHAR* did, DCHAR* didValue)
{
    D32S nRet = ADM_ECODE_FAILED;

    if (allEcuInfo == NULL || ecuReqId == NULL || did == NULL)
    {
        log_e("allEcuInfo[ %p ] ecuReqId[ %p ] did[ %p ]", allEcuInfo, ecuReqId, did);
        return ADM_ECODE_INVALID_PARAM;
    }

    ECU_INFO_T* ecuInfo = NULL;
    for (D32S i = 0; i < allEcuInfo->num; i++)
    {
        ecuInfo = allEcuInfo->ecuInfo[i];
        if (ecuInfo != NULL)
        {
            nRet = ipmFindEcuInfoDidValue(ecuInfo, ecuReqId, did, didValue);
            if (nRet == ADM_ECODE_SUCCESSED)
            {
                log_i("find success");
                break;
            }
        }
    }

    return nRet;
}
*/
static const ECU_INFO_T* ipmEcuInfosGetEcuInfoByReqId(const ADM_EIM_GETECUINFO_T* allEcuInfo, DCHAR* ecuReqId, DCHAR* ecuSwDid)
{
    ECU_INFO_T* ecuInfo = NULL;
    for (D32S i = 0; i < allEcuInfo->num; i++)
    {
        ecuInfo = allEcuInfo->ecuInfo[i];
        if ((0 == adm_strcasecmp(ecuInfo->ecuReqId, ecuReqId)) && (0 == adm_strcasecmp(ecuInfo->ecuSwDid, ecuSwDid)))
        {
            return ecuInfo;
        }
    }

    return NULL;
}


static DCHAR* ipmSoaDidsGetDidValue(const ADM_DIM_SOA_DID_DATA_ARRAY_T* arrDid, DCHAR* did)
{
//    D32S nRet = ADM_ECODE_FAILED;
    DCHAR* didValue = NULL;
	DCHAR didValueBuf[128] = {0};

    if (arrDid == NULL || did == NULL)
    {
        log_e("arrDid[ %p ] did[ %p ]", arrDid, did);
        return NULL;
    }

    ADM_DIM_SOA_DID_DATA_T* didArray = NULL;
    for (D32U i = 0; i < arrDid->didArrayLength; i++)
    {
        didArray = arrDid->didArray[i];
        if (didArray != NULL)
        {
            if ((D32S)htoi((char *)did) == didArray->did && didArray->didData != NULL)
            {
                adm_memzero(didValueBuf, sizeof(didValueBuf));
                switch ((VARIABLE_TYPE_EN)didArray->didData->dataType)//GAC_TODO: 其他类型
                {
                case VAR_U8:
                    (void)adm_snprintf(didValueBuf, sizeof(didValueBuf)/sizeof(DCHAR), "%d", didArray->didData->v.value[0]);
					didValue = adm_strdup(didValueBuf);
                    break;
                case VAR_U32:
                    (void)adm_snprintf(didValueBuf, sizeof(didValueBuf)/sizeof(DCHAR), "%d", didArray->didData->v.valueU32);
					didValue = adm_strdup(didValueBuf);
                    break;
                case VAR_S32:
                    (void)adm_snprintf(didValueBuf, sizeof(didValueBuf)/sizeof(DCHAR), "%d", didArray->didData->v.valueS32);
					didValue = adm_strdup(didValueBuf);
                    break;
                case VAR_U64:
                    (void)adm_snprintf(didValueBuf, sizeof(didValueBuf)/sizeof(DCHAR), "%ld", didArray->didData->v.valueU64);
					didValue = adm_strdup(didValueBuf);
                    break;
				case VAR_S64:
                    (void)adm_snprintf(didValueBuf, sizeof(didValueBuf)/sizeof(DCHAR), "%ld", didArray->didData->v.valueS64);
					didValue = adm_strdup(didValueBuf);
                    break;	
                case VAR_FLOAT:
                    (void)adm_snprintf(didValueBuf, sizeof(didValueBuf)/sizeof(DCHAR), "%f", didArray->didData->v.valueF);
					didValue = adm_strdup(didValueBuf);
                    break;
                case VAR_STRING:
                    /* code */
                    didValue = adm_strdup((DCHAR *)didArray->didData->v.valueS);
                    break;

                default:
                    break;
                }

                break;
            }
        }
    }

    return didValue;
}

static DCHAR * ipmEcuInfoGetDidValue(const ECU_INFO_T* ecuInfo, DCHAR* did)
{
//    D32S nRet = ADM_ECODE_FAILED;

    if (ecuInfo == NULL || did == NULL)
    {
        log_e("ecuInfo[ %p ] did[ %p ]", ecuInfo, did);
        return NULL;
    }

    return ipmSoaDidsGetDidValue(ecuInfo->vdiDidListResponse, did);
}

static D32S ipmCollectEcuInfobyId(DCHAR* ecuReqId, DCHAR* ecuSwDid, ECU_INFO_T** ppEcuInfo)
{
    if (NULL == ecuReqId || NULL == ecuSwDid)
    {
        log_e("ecuReqId[%p] ecuSwDid[%p]", ecuReqId, ecuSwDid);
        return ADM_ECODE_FAILED;
    }

    D32S iRet = ADM_ECODE_FAILED;
    ADM_ECS_ECUCONFIG_T* pstEcuCfg = NULL;
    ADM_DIM_SOA_ECU_IDEN_INFO_T stEcuIdenInfo = { 0 };
    ECU_INFO_T* ecuInfo = NULL;
//    ADM_EIM_GETECUINFO_T ecuInfos = { 0 };

    // 获取指定requestId的dids
    iRet = adm_ecs_getEcuConfig(ecuReqId, ecuSwDid, &pstEcuCfg);
    if (iRet != ADM_ECODE_SUCCESSED)
    {
        log_e("Failed to get ecs ecu config, ecuReqId[%s] ecuSwDid[%s] err[%d]", ecuReqId, ecuSwDid, iRet);
        return ADM_ECODE_FAILED;
    }

    do
    {
        iRet = ADM_ECODE_FAILED;
        ADM_ECS_RSCONFIGINFO_T stRsCfgInfo = { 0 };
        if (ADM_ECODE_SUCCESSED != adm_ecs_getConfigInfo(&stRsCfgInfo))
        {
            break;
        }

        stEcuIdenInfo.requestId = (D32S)htoi(pstEcuCfg->ecuReqId);
        stEcuIdenInfo.responseId = (D32S)htoi(pstEcuCfg->ecuResponseId);
        stEcuIdenInfo.functionId = (D32S)htoi(pstEcuCfg->ecuFunctionId);
        stEcuIdenInfo.swDid = (D32S)htoi(pstEcuCfg->ecuSwDid);

        iRet = adm_eim_getSingleEcuInfo(pstEcuCfg,
            &stRsCfgInfo.stGeneralCfg.stEcuDidList,
            &stEcuIdenInfo,
            stRsCfgInfo.stGeneralCfg.stBusinessScript.filePath,
            &ecuInfo);
        adm_ecs_cleanRSConfigInfo(&stRsCfgInfo);
        if (ADM_ECODE_SUCCESSED != iRet)
        {
            log_e("requestId 0x%X, responseId 0x%X, functionId 0x%X, swDid 0x%X error",
                stEcuIdenInfo.requestId,
                stEcuIdenInfo.responseId,
                stEcuIdenInfo.functionId,
                stEcuIdenInfo.swDid);
            break;
        }

        /*更新ECU信息KV缓存*/
        ADM_EIM_GETECUINFO_T* pKvEcuInfos = NULL;   // KV缓存中ECU信息
        if (ADM_ECODE_SUCCESSED == adm_eim_getEcuInfoManagerKvGet(&pKvEcuInfos))
        {
            ADM_EIM_GETECUINFO_T    stNewEcuInfos = { 0 };        // 存储最新的ECU信息，用于更新ECU KV缓存
            D32S iCurrentEcuIndex = pKvEcuInfos->num;   // 当前更新的ECU在pKvEcuInfos中的下标，默认没在pKvEcuInfos即pKvEcuInfos中ECU的数量作为下标

            stNewEcuInfos.ecuInfo = adm_malloc(sizeof(stNewEcuInfos.ecuInfo[0]) * ((D64U)pKvEcuInfos->num + 1UL));
			if (ADM_IS_NULL(stNewEcuInfos.ecuInfo))
	        {
	            log_e("adm_malloc error");
	            iRet = ADM_ECODE_APPLY_MEMORY_FAILED;
				adm_eim_deleteGetEcuInfoBody(pKvEcuInfos);
	            break;
	        }
			else
			{
	            stNewEcuInfos.num = pKvEcuInfos->num + 1;     // 初始默认ECU信息数量为pKvEcuInfos中ECU数量加1(当前ECU默认为新增)
	            for (D32S i = 0; i < pKvEcuInfos->num; i++)
	            {
	                if ((0 == adm_strcmp(ecuReqId, pKvEcuInfos->ecuInfo[i]->ecuReqId)) &&
	                    (0 == adm_strcmp(ecuSwDid, pKvEcuInfos->ecuInfo[i]->ecuSwDid)))
	                {
	                    iCurrentEcuIndex = i;
	                    stNewEcuInfos.num = pKvEcuInfos->num;     // KV缓存中有该ECU信息，更新的ECU数量应该为KV中ECU的数量
	                }

	                stNewEcuInfos.ecuInfo[i] = pKvEcuInfos->ecuInfo[i];
	            }
	            stNewEcuInfos.ecuInfo[iCurrentEcuIndex] = ecuInfo;    // 更新ECU信息中当前ECU的信息

	            adm_eim_getEcuInfoManagerEcusStore(&stNewEcuInfos);   // 更新ECU信息KV缓存
	            adm_free(stNewEcuInfos.ecuInfo);
			}
            
            adm_eim_deleteGetEcuInfoBody(pKvEcuInfos);
        }

//         
//         adm_eim_getEcuInfoManagerEcusStore();
        /* 不能只存储一个ecuinfo，需要存储所有ecuinfo
        ecuInfos.ecuInfo = adm_malloc((D32U)(sizeof(ecuInfo) * 1UL));
        ecuInfos.ecuInfo[0] = ecuInfo;
        ecuInfos.num = 1;
        adm_eim_getEcuInfoManagerEcusStore(&ecuInfos);
        adm_free(ecuInfos.ecuInfo);
        */

        iRet = ADM_ECODE_SUCCESSED;
        *ppEcuInfo = ecuInfo;
        log_i("ecuReqId %s ecuSwDid %s read part information success", ecuInfo->ecuReqId, ecuInfo->ecuSwDid);
    } while (FALSE);

    adm_ecs_delEcuConfigInfo(&pstEcuCfg);

    return iRet;
}

static D32S ipmCycleCollectEcuInfo(const ADM_ECS_RSCONFIGINFO_T* stRsCfgInfo, const ADM_IPS_POLICY_DEPENDENCY_ARRAY_T* dependency, ADM_EIM_GETECUINFO_T** ecuInfo)
{
    D32S retVal = ADM_ECODE_FAILED;
    D32S ecuNum = 0, dependencyNum = 0;
    const ADM_ECS_PARTCONFIG_T *pstPartCfg = NULL;
    const ADM_ECS_GENERALCONFIG_T *pstGeneralCfg = NULL;
    const ADM_ECS_ECUCONFIG_T *pstEcuCfg = NULL;
    ADM_IPS_POLICY_DEPENDENCY_T *pDependency = NULL;
    ADM_DIM_SOA_ECU_IDEN_INFO_T stEcuIdenInfo = {0};
    ECU_INFO_T *singleEcuInfo = NULL;
    ADM_EIM_GETECUINFO_T *getEcuInfo = NULL;
    D32U mallocLen = 0;
    D32S i = 0, j = 0;

    if (NULL == stRsCfgInfo || NULL == dependency || NULL == ecuInfo)
    {
        log_e("stRsCfgInfo %p, dependency %p, ecuInfo %p", stRsCfgInfo, dependency, ecuInfo);
        return ADM_ECODE_INVALID_PARAM;
    }

    /* 获取配置ECU的个数，如果是0，则无零件信息 */
    dependencyNum = dependency->dependenciesLen;
    log_i("dependencyNum is %d", dependencyNum);
    if (0 == dependencyNum)
    {
        log_e("dependencyNum is zero");
        retVal = ADM_ECODE_SUCCESSED;
        return retVal;
    }

    /* initialize variables */
    pstPartCfg = &(stRsCfgInfo->stPartCfg);
    pstGeneralCfg = &(stRsCfgInfo->stGeneralCfg);

    /* 获取配置ECU的个数，如果是0，则无零件信息 */
    ecuNum = pstPartCfg->stEcuCfgList.num;
    log_i("ecu num is %d", ecuNum);
    if (0 == ecuNum)
    {
        log_e("ecu num is zero, failed");
        retVal = ADM_ECODE_EIM_GET_ECU_CONFIG_NUM_ZERO;
        return retVal;
    }

    do
    {
        /* 为每个ECUINFO 分配内存 */
        mallocLen = (D32U)sizeof(*getEcuInfo);
        getEcuInfo = adm_malloc(mallocLen);
        if (ADM_IS_NULL(getEcuInfo))
        {
            log_e("adm_malloc error");
            retVal = ADM_ECODE_APPLY_MEMORY_FAILED;
            return retVal;
        }
        adm_memzero(getEcuInfo, mallocLen);

        mallocLen = (D32U)sizeof(ECU_INFO_T *) * (D32U)dependencyNum + 1U;
        getEcuInfo->ecuInfo = adm_malloc(mallocLen);
        if (ADM_IS_NULL(getEcuInfo->ecuInfo))
        {
            log_e("adm_malloc error");
            retVal = ADM_ECODE_APPLY_MEMORY_FAILED;
            break;
        }
        adm_memzero(getEcuInfo->ecuInfo, mallocLen);
        getEcuInfo->num = 0;

        /* 循环读每个ECU零件信息 */
        for (i = 0; i < dependencyNum; ++i)
        {
            pDependency = &(dependency->dependencies[i]);
            for (j = 0; j < ecuNum; j++)//判断依赖的ecu配置信息是否存在于零件配置中
            {
                pstEcuCfg = &(pstPartCfg->stEcuCfgList.pstEcuCfg[j]);
                if (adm_strcasecmp(pstEcuCfg->ecuReqId, pDependency->ecuReqId) == 0)
                {
                    log_i("Find part config of dependency success, ecuReqId[%s]", pDependency->ecuReqId);
                    break;
                }
            }

            if (j == ecuNum)//没有在零件配置中则终止
            {
                log_e("Failed to find part config of dependency, ecuReqId[%s]", pDependency->ecuReqId);
//                retVal = ADM_ECODE_FAILED;
                break;
            }

            stEcuIdenInfo.requestId = (D32S)htoi(pstEcuCfg->ecuReqId);
            stEcuIdenInfo.responseId = (D32S)htoi(pstEcuCfg->ecuResponseId);
            stEcuIdenInfo.functionId = (D32S)htoi(pstEcuCfg->ecuFunctionId);
            stEcuIdenInfo.swDid = (D32S)htoi(pstEcuCfg->ecuSwDid);

            retVal = adm_eim_getSingleEcuInfo(pstEcuCfg,
                                                        &pstGeneralCfg->stEcuDidList,
                                                        &stEcuIdenInfo,
                                                        pstGeneralCfg->stBusinessScript.filePath,
                                                        &singleEcuInfo);

            if (ADM_IS_ERR(retVal))
            {
                log_e("requestId 0x%X, responseId 0x%X, functionId 0x%X, swDid 0x%X adm_eim_getSingleEcuInfo error",
                    stEcuIdenInfo.requestId,
                    stEcuIdenInfo.responseId,
                    stEcuIdenInfo.functionId,
                    stEcuIdenInfo.swDid);
                continue;
            }

            log_i("ecuReqId %s ecuSwDid %s read part information success", singleEcuInfo->ecuReqId, singleEcuInfo->ecuSwDid);
            getEcuInfo->ecuInfo[getEcuInfo->num] = singleEcuInfo;
            getEcuInfo->num++;
        }

        if (0 == getEcuInfo->num)
        {
            log_e("no part information was read");
            retVal = ADM_ECODE_EIM_GET_ECU_INFO_FAILED;
            break;
        }

        *ecuInfo = getEcuInfo;
        retVal = ADM_ECODE_SUCCESSED;
    } while (FALSE);

    if(retVal == ADM_ECODE_SUCCESSED)
    {
        return ADM_ECODE_SUCCESSED;
    }
    
    adm_free(getEcuInfo->ecuInfo);
  
    adm_free(getEcuInfo);

    return retVal;
}


static D32S ipmPolicyDependCompareDidWithEcuInfo(const ADM_IPS_POLICY_DEPENDENCY_T* dependency, const ECU_INFO_T* ecuInfo)
{
    D32S ret = ADM_ECODE_SUCCESSED;
    DCHAR* dependencyDidValue = NULL;
    DCHAR* ecuInfoDidValue = NULL;
    D32S i = 0;
	DCHAR* did = NULL;
	
    for(i = 0; i < dependency->ecuDids.ecuDidsLen; i++ ){
        did = dependency->ecuDids.ecuDids[i].did;
        dependencyDidValue = dependency->ecuDids.ecuDids[i].didValue;
        if (NULL == dependencyDidValue)
        {
            log_e("Failed to find policy dependency did value, ecuReqId[%s] ecuSwDid[%s] did[%s]",
                dependency->ecuReqId, dependency->ecuSwDid, did);
			ret = ADM_ECODE_FAILED;
            break;
        }

        // 从ecu信息获取didValue
        ecuInfoDidValue = (DCHAR*)ipmEcuInfoGetDidValue(ecuInfo, did);
        if (NULL == ecuInfoDidValue)
        {
            log_e("Failed to find ecu info did value, ecuReqId[%s] ecuSwDid[%s] did[%s]",
                ecuInfo->ecuReqId, ecuInfo->ecuSwDid, did);
			ret = ADM_ECODE_FAILED;
            break;
        }

        // 比较didValue
        if (0 != adm_strcasecmp(dependencyDidValue, ecuInfoDidValue))
        {
            ret = ADM_ECODE_INSTALL_VERSION_VERIFY_FAILED;
            log_e("ecuReqId[%s] ecuSwDid[%s] did[%s] policy didValue[%s] ecu didValue[%s]",
                dependency->ecuReqId, dependency->ecuSwDid, did, dependencyDidValue, ecuInfoDidValue);
        }

        adm_free(ecuInfoDidValue);
    }

    return ret;
}


static D32S ipmVerifyEcuVersionByDenpendency(ADM_IPS_POLICY_DEPENDENCY_T* pDependency)
{
    D32S iRet = ADM_ECODE_FAILED;
    ECU_INFO_T* pEcuInfo = NULL;

    do
    {
        // 采集一个ecu信息
        iRet = ipmCollectEcuInfobyId(pDependency->ecuReqId, pDependency->ecuSwDid, &pEcuInfo);
        if (ADM_ECODE_SUCCESSED != iRet)
        {
            log_e("Failed to get ecus info, ecuReqId[%s] ecuSwDid[%s]", pDependency->ecuReqId, pDependency->ecuSwDid);
            break;
        }

        // 根据升级策略依赖信息校验ecu版本
        iRet = ipmPolicyDependCompareDidWithEcuInfo(pDependency, pEcuInfo);
        if (ADM_ECODE_SUCCESSED == iRet)
        {
            log_i("Verify ecus version successful, ecuReqId[%s] ecuSwDid[%s]", pDependency->ecuReqId, pDependency->ecuSwDid);
        }
        else
        {
            log_e("Verify ecus version failed, ecuReqId[%s] ecuSwDid[%s]", pDependency->ecuReqId, pDependency->ecuSwDid);
        }
        adm_eim_deleteEcuInfoBody(pEcuInfo);
    } while (FALSE);

    return iRet;
}

static D32S ipmVerifyEcuVersionAndReportByDenpendency(ADM_IPS_POLICY_DEPENDENCY_T* dependency)
{
    D32S ret = ipmVerifyEcuVersionByDenpendency(dependency);
    if (0 == ret)
    {
        log_i("Verify ecu version successful, ecuReqId[%s] ecuSwDid[%s]", dependency->ecuReqId, dependency->ecuSwDid);
        ipmReportEventDependencyEventId("S_IE_020", NULL, ADM_ECODE_SUCCESSED);
    }
    else
    {
        log_i("Verify ecu version failed, ecuReqId[%s] ecuSwDid[%s]", dependency->ecuReqId, dependency->ecuSwDid);
        ipmReportEventDependencyEventId("S_IE_021", NULL, ADM_ECODE_SUCCESSED);
    }

    return ret;
}


/*更新kv中的ecu信息，并获取所有ecu信息进行升级后的版本号和零件号的校验*/
static D32S ipmDoVerifyEcusVersionAfterInstall(ADM_IPM_DO_FLASH_T* f)
{
	D32S nRet = ADM_ECODE_FAILED;
	if (f == NULL)
	{
		log_e("invalid param f");
		return ADM_ECODE_INVALID_PARAM;
	}

	ADM_EIM_GETECUINFO_T* allEcuInfo = NULL;
	const ECU_INFO_T* ecuInfo = NULL;
    ADM_ECS_RSCONFIGINFO_T *pstRsCfgData = NULL;

    pstRsCfgData = adm_ecs_newRSConfigInfo();
    if (ADM_IS_NULL(pstRsCfgData))
    {
        nRet = ADM_ECODE_NO_MEMORY;
        log_e("Failed to new ota config data info, err[%d]", nRet);
        return nRet;
    }

    nRet = adm_ecs_getConfigInfo(pstRsCfgData);
    if (nRet != ADM_ECODE_SUCCESSED)
    {
        log_e("Failed get config info, nRet[%d]", nRet);
        adm_ecs_delRSConfigInfo(&pstRsCfgData);
        return nRet;
    }
    
	// 获取升级策略中升级后的依赖信息
	ADM_IPS_POLICY_DEPENDENCY_T* dependency = NULL;
	ADM_IPS_POLICY_DEPENDENCY_ARRAY_T dependencies;
	adm_ips_policyDependencyArrayInit(&dependencies);
	// adm_ips_policyDependencyArrayGet(&dependencies);
	(void)adm_ips_policyDependenciesGetWithType(&dependencies, (D32S)ADM_IPS_POLICY_DEPENDENCY_AFTER);
	if (dependencies.dependenciesLen == 0)
	{
		log_i("after install,the need check version ecu num is 0");
		adm_ips_policyDependencyArrayDeInit(&dependencies);
        adm_ecs_delRSConfigInfo(&pstRsCfgData);
		return ADM_ECODE_SUCCESSED;
	}

#ifdef ADM_PARALLEL_GET_ECU
    /** 并行获取ECU信息 */
    nRet = adm_eim_parallelCollectEcuInfo((DVOID *)pstRsCfgData, COLLECT_TRIGGER_TYPE_INSTALL, &dependencies);
    if (nRet != ADM_ECODE_SUCCESSED)
    {
        log_e("Failed to collect ecus info, nRet[%d]", nRet);
        adm_ips_policyDependencyArrayDeInit(&dependencies);
        adm_ecs_delRSConfigInfo(&pstRsCfgData);
        return nRet;
    }
#endif

	//读取所有ecu info
	nRet = adm_eim_getEcuInfoManagerKvGet(&allEcuInfo);
	if ((nRet != ADM_ECODE_SUCCESSED)||ADM_IS_NULL(allEcuInfo))
	{
		log_e("Failed to collect ecus info, nRet[%d]", nRet);
		adm_ips_policyDependencyArrayDeInit(&dependencies);
		adm_ecs_delRSConfigInfo(&pstRsCfgData);
		return nRet;
	}
 
	do
	{
		D32S res = ADM_ECODE_FAILED;
		D32S i = 0;
		D32S j = 0;

        if (DFALSE == adm_cvs_getIsVerifyVersion())
        {
            log_i("After install, need not verify ecu version.");
            i =  dependencies.dependenciesLen;
        }
        else
        {
            // 采集到的ecu信息与依赖依次比较
            for (i = 0; i < dependencies.dependenciesLen; i++)
            {
                dependency = &dependencies.dependencies[i];
                if(ADM_IPS_POLICY_DEPENDENCY_AFTER == (ADM_IPS_POLICY_DEPENDENCY_TYPE_E)dependency->type){
                    
                    // 获取ecu信息
                    ecuInfo = NULL;
                    for(j=0; j<allEcuInfo->num; ++j)
                    {
                        if ((0 == adm_strcasecmp(dependency->ecuReqId, allEcuInfo->ecuInfo[j]->ecuReqId)) &&
                            (0 == adm_strcasecmp(dependency->ecuSwDid, allEcuInfo->ecuInfo[j]->ecuSwDid)))
                        {
                            ecuInfo = allEcuInfo->ecuInfo[j];
                            break;
                        }
                    }

                    if (ADM_IS_NULL(ecuInfo))
                    {   
                        log_e("Failed to get ecu config, err[%d],ecuReqId[%s] ecuSwDid[%s]", ADM_ECODE_EIM_GETECUINFO_FAILED, dependency->ecuReqId, dependency->ecuSwDid);
    //		            nRet = ADM_ECODE_EIM_GETECUINFO_FAILED;
                        break ;
                    }    


                    // 校验ecu dids
                    log_i("Verify ecu dids, ecuReqId[%s] ecuSwDid[%s]", dependency->ecuReqId, dependency->ecuSwDid);
                    res = ipmPolicyDependCompareDidWithEcuInfo(dependency, ecuInfo);
                    if (ADM_ECODE_SUCCESSED != res)
                    {
                        log_e("Verify ecu dids failed, ecuReqId[%s] ecuSwDid[%s]", 
                            dependency->ecuReqId, dependency->ecuSwDid);
                        break;
                    }
                    
                }
            }
        }

		//版本号和零件号的校验成功后，需要保存最新的信息   （后续根据升级前后进行更新  （升级前无需更新，升级需要更新） 1：升级前 2：升级后）
		if (i == dependencies.dependenciesLen)
		{			 
			log_i("after install,the verify ecu dids successful");
		}
		else{
			nRet = ADM_ECODE_INSTALL_AFTERINSTALL_VERSION_VERIFY_FAILED;
			break;
		}
	} while (FALSE);

	//10.释放内存空间
	adm_ips_policyDependencyArrayDeInit(&dependencies);
	adm_eim_deleteGetEcuInfoBody(allEcuInfo);
    adm_ecs_delRSConfigInfo(&pstRsCfgData);

	return nRet;
}

#if 0
static D32S ipmVerifyEcusVersionPostInstall(void)
{
    D32S iRet = ADM_ECODE_FAILED;
    ADM_IPS_POLICY_DEPENDENCY_T* pDependency = NULL;
    ADM_IPS_POLICY_DEPENDENCY_ARRAY_T dependencies;
    adm_ips_policyDependencyArrayInit(&dependencies);

    /*获取安装后依赖信息*/
    iRet = adm_ips_policyDependenciesGetWithType(&dependencies, (D32S)ADM_IPS_POLICY_DEPENDENCY_AFTER);
    if (ADM_ECODE_SUCCESSED != iRet || 0 == dependencies.dependenciesLen)
    {
        return iRet;
    }

    IPM_ECU_POSTINSTALL_DENPENDENCY_VERIFY_INFO* pVerifyInfo = NULL;
    IPM_ECU_POSTINSTALL_DENPENDENCY_VERIFY_INFO* pVerifyInfos = NULL;
    pVerifyInfos = adm_malloc(sizeof(pVerifyInfos[0]) * dependencies.dependenciesLen);
    if (NULL == pVerifyInfos)
    {
        return ADM_ECODE_FAILED;
    }

    for (D32S i = 0; i < dependencies.dependenciesLen; i++)
    {
        pVerifyInfo = &pVerifyInfos[i];
        pVerifyInfo->pDependency = &dependencies.dependencies[i];
        pVerifyInfo->uiCostTime = 0U;
        pVerifyInfo->uiLastTryTime = 0;
        pVerifyInfo->bIsDone = DFALSE;
    }

    D64U uiStartTime = 0U;  // ECU校验开始时间
    D64U uiEndTime = 0U;    // ECU校验结束时间
    D32U uiDoneCount = 0;   // 已完成校验的ECU数量
    ECU_INFO_T* pEcuInfo = NULL;
    iRet = ADM_ECODE_SUCCESSED;

    do
    {
        for (D32S i = 0; i < dependencies.dependenciesLen; i++)
        {
            uiStartTime = (D64U)time(NULL);
            pVerifyInfo = &pVerifyInfos[i];
            /*ECU是未完成校验，并且上次尝试校验已过去5秒，再次获取ECU信息校验*/
            if (DFALSE == pVerifyInfo->bIsDone && uiStartTime - pVerifyInfo->uiLastTryTime >= 5)
            {
                pDependency = pVerifyInfo->pDependency;

                iRet = ipmCollectEcuInfobyId(pDependency->ecuReqId, pDependency->ecuSwDid, &pEcuInfo);
                uiEndTime = (D64U)time(NULL);

                switch (iRet)
                {
                case ADM_ECODE_SUCCESSED:
                    // 根据升级策略依赖信息校验ecu版本
                    iRet = ipmPolicyDependCompareDidWithEcuInfo(pDependency, pEcuInfo);
                    if (ADM_ECODE_SUCCESSED == iRet)
                    {
                        log_i("Verify ecus version successful, ecuReqId[%s] ecuSwDid[%s]",
                            pDependency->ecuReqId, pDependency->ecuSwDid);
                    }
                    else
                    {
                        log_e("Verify ecus version failed, ecuReqId[%s] ecuSwDid[%s]",
                            pDependency->ecuReqId, pDependency->ecuSwDid);
                    }

                    adm_eim_deleteEcuInfoBody(pEcuInfo);

                    pVerifyInfo->bIsDone = DTRUE;
                    uiDoneCount++;
                    break;
                case ADM_ECODE_RPC_INSTANCE_OFFLINE:
                    /*no break*/
                case ADM_ECODE_RPC_SERVICE_OFFLINE:
                    /*no break*/
                case ADM_ECODE_RPC_REQSERV_TIMEOUT:
                    log_i("get ecu info faild! ecuReqId[%s] ecuSwDid[%s] result[%s]", pDependency->ecuReqId, pDependency->ecuSwDid, iRet);

                    pVerifyInfo->uiCostTime += uiEndTime > uiStartTime ? uiEndTime - uiStartTime : 1U;  // 校验ECU信息未超过1秒按一秒计算
                    pVerifyInfo->uiLastTryTime = uiEndTime; // 以ECU校验结束时间作为本次校验尝试时间
                    /*ECU校验获取ECU信息因RPC实例或RPC服务或请求超时原因，可以在30s内尝试重新获取ECU信息校验*/
                    if (pVerifyInfo->uiCostTime >= 30U) // 校验失败
                    {
                        pVerifyInfo->bIsDone = DTRUE;
                        uiDoneCount++;
                        iRet = ADM_ECODE_FAILED;

                        log_e("get ecu info timeout! ecuReqId[%s] ecuSwDid[%s]", pDependency->ecuReqId, pDependency->ecuSwDid);
                    }
                    break;
                default:    // 其他原因，校验失败
                    pVerifyInfo->bIsDone = DTRUE;
                    uiDoneCount++;
                    iRet = ADM_ECODE_FAILED;
                    log_i("get ecu info faild! ecuReqId[%s] ecuSwDid[%s] result[%s]", pDependency->ecuReqId, pDependency->ecuSwDid, iRet);
                    break;
                }

                if (ADM_ECODE_SUCCESSED != iRet)
                {
                    iRet = ADM_ECODE_INSTALL_AFTERINSTALL_VERSION_VERIFY_FAILED;
                    break;
                }
            }
        }

        if (ADM_ECODE_SUCCESSED != iRet)
        {
            break;
        }

        usleep(20 * 1000);  // 等待20ms，防止循环导致CPU占用率过高
    } while (uiDoneCount < dependencies.dependenciesLen);

    adm_free(pVerifyInfos);
    adm_ips_policyDependencyArrayDeInit(&dependencies);

    return iRet;
}
#endif

/*一次性实时读取依赖中所有ECU信息，然后进行版本号和零件号的校验*/
static D32S ipmDoVerifyEcusVersionBeforeInstall(ADM_IPM_DO_FLASH_T* f)
{
    D32S nRet = ADM_ECODE_FAILED;

    if (f == NULL)
    {
        log_e("invalid param f");
        return ADM_ECODE_INVALID_PARAM;
    }

    ADM_EIM_GETECUINFO_T* allEcuInfo = NULL;
    const ECU_INFO_T* ecuInfo = NULL;
    ADM_ECS_RSCONFIGINFO_T *pstRsCfgData = NULL;

    pstRsCfgData = adm_ecs_newRSConfigInfo();
    if (ADM_IS_NULL(pstRsCfgData))
    {
        nRet = ADM_ECODE_NO_MEMORY;
        log_e("Failed to new ota config data info, err[%d]", nRet);
        return nRet;
    }

    nRet = adm_ecs_getConfigInfo(pstRsCfgData);
    if (nRet != ADM_ECODE_SUCCESSED)
    {
        log_e("Failed get config info, nRet[%d]", nRet);
        adm_ecs_delRSConfigInfo(&pstRsCfgData);
        return nRet;
    }

    // 获取升级策略中升级前的依赖信息
    ADM_IPS_POLICY_DEPENDENCY_T* dependency = NULL;
    ADM_IPS_POLICY_DEPENDENCY_ARRAY_T dependencies;
    adm_ips_policyDependencyArrayInit(&dependencies);
    // adm_ips_policyDependencyArrayGet(&dependencies);
    (void)adm_ips_policyDependenciesGetWithType(&dependencies, (D32S)ADM_IPS_POLICY_DEPENDENCY_BEFORE);
    if (dependencies.dependenciesLen == 0)
    {
        log_i("before install,the need check version ecu num is 0");
        adm_ips_policyDependencyArrayDeInit(&dependencies);
        adm_ecs_delRSConfigInfo(&pstRsCfgData);
        return ADM_ECODE_SUCCESSED;
    }


#ifdef ADM_PARALLEL_GET_ECU
    /** 并行获取ECU信息 */
    nRet = adm_eim_parallelCollectEcuInfo((DVOID *)pstRsCfgData, COLLECT_TRIGGER_TYPE_INSTALL, &dependencies);
    if (nRet != ADM_ECODE_SUCCESSED)
    {
        log_e("Failed to collect ecus info, nRet[%d]", nRet);
        adm_ips_policyDependencyArrayDeInit(&dependencies);
        adm_ecs_delRSConfigInfo(&pstRsCfgData);
        return nRet;
    }
    
    /** 获取ECU信息管理器KV数据 */
    adm_eim_getEcuInfoManagerKvGet(&allEcuInfo);
    if (nRet != ADM_ECODE_SUCCESSED)
    {
        log_e("Failed to collect ecus info, nRet[%d]", nRet);
        adm_ips_policyDependencyArrayDeInit(&dependencies);
        adm_ecs_delRSConfigInfo(&pstRsCfgData);
        return nRet;
    }
#else
     // 一次性实时读取依赖中所有ECU信息
    nRet = ipmCycleCollectEcuInfo(pstRsCfgData, &dependencies, &allEcuInfo);
    if (nRet != ADM_ECODE_SUCCESSED)
    {
        log_e("Failed to collect ecus info, nRet[%d]", nRet);
		adm_ips_policyDependencyArrayDeInit(&dependencies);
        adm_ecs_delRSConfigInfo(&pstRsCfgData);
        return nRet;
    }
#endif

    do
    {
        D32S res = 0;
        D32S i = 0;

        // 采集到的ecu信息与依赖依次比较
        for (i = 0; i < dependencies.dependenciesLen; i++)
        {
            dependency = &dependencies.dependencies[i];
            if(ADM_IPS_POLICY_DEPENDENCY_BEFORE == (ADM_IPS_POLICY_DEPENDENCY_TYPE_E)dependency->type){

	            // 获取ecu信息
	            ecuInfo = ipmEcuInfosGetEcuInfoByReqId(allEcuInfo, dependency->ecuReqId, dependency->ecuSwDid);
                if (NULL == ecuInfo)
                {
                    log_e("Failed to get ecu info, ecuReqId[%s]", dependency->ecuReqId, dependency->ecuSwDid);
                    break;
                }

                // 校验ecu dids
                log_i("Verify ecu dids, ecuReqId[%s] ecuSwDid[%s]", dependency->ecuReqId, dependency->ecuSwDid);
                res = ipmPolicyDependCompareDidWithEcuInfo(dependency, ecuInfo);
                if (ADM_ECODE_SUCCESSED != res)
                {
                    log_e("Verify ecu dids failed, ecuReqId[%s] ecuSwDid[%s]",
                          dependency->ecuReqId, dependency->ecuSwDid);
                    break;
                }
            }
        }

        // 版本号和零件号的校验成功后，需要保存最新的信息   （后续根据升级前后进行更新  （升级前无需更新，升级需要更新） 1：升级前 2：升级后）
        if (i == dependencies.dependenciesLen)
        {
            log_i("before install,the verify ecu dids successful");
        }
        else
        {
            log_e("The number of dependencies does not match,  num[%d], num[%i]", dependencies.dependenciesLen, i);
            nRet = ADM_ECODE_INSTALL_VERSION_VERIFY_FAILED;
            break;
        }
    } while (FALSE);

    //10.释放内存空间
    adm_ips_policyDependencyArrayDeInit(&dependencies);
    adm_eim_deleteGetEcuInfoBody(allEcuInfo);
    adm_ecs_delRSConfigInfo(&pstRsCfgData);

    return nRet;
}
/*
static void ipmDoVerifyAllEcusVersionBeforeInstall()
{
    ADM_IPS_POLICY_DEPENDENCY_ARRAY_T dependencies;
    adm_ips_policyDependencyArrayInit(&dependencies);
    adm_ips_policyDependenciesGetWithType(&dependencies, ADM_IPS_POLICY_DEPENDENCY_BEFORE);

    for (D32S i = 0; i < dependencies.dependenciesLen; i++)
    {
        ipmVerifyEcuVersionByDenpendency(&dependencies.dependencies[i]);
    }

    adm_ips_policyDependencyArrayDeInit(&dependencies);
}
*/
static D32S ipmDoVerifyEcuVersionPostInstall(const DCHAR* ecuReqId, const DCHAR* ecuSwDid)
{
    D32S ret = ADM_ECODE_SUCCESSED;
    ADM_IPS_ECU_PROGRESS_T progress;
    ADM_IPS_POLICY_DEPENDENCY_T dependency;

    ///!< 只有APK自升级的功能会执行此逻辑，通过校验版本号的方式获取此结果
    if (ADM_EIM_APK_UCM_REQUEST_ID == ipmHtoi(ecuReqId) || ADM_EIM_APK_HMI_REQUEST_ID == ipmHtoi(ecuReqId))
    {
        adm_ips_ecuProgressInit(&progress);
        (void)adm_ips_ecuProgressGet(&progress, ecuReqId, ecuSwDid);
        // ECU安装成功，获取安装策略依赖校验ECU版本
        if (ADM_IPS_ECU_PHASE_POST_INSTALL == progress.state
            && ADM_ECODE_SUCCESSED == progress.progress[ADM_IPS_ECU_PHASE_INDEX_UPGRADE].result)
        {
            adm_ips_policyDependencyInit(&dependency);
            // 获取升级后的安装策略依赖
            ret = adm_ips_policyDependencyGetWithType(progress.ecuReqId, progress.ecuSwDid,
                (D32S)ADM_IPS_POLICY_DEPENDENCY_AFTER, &dependency);
            if (ADM_ECODE_SUCCESSED == ret)
            {
                // 校验ECU版本并上报结果
                log_i("Verify ecu version after installation, ecuReqId[%s] ecuSwDid[%s]", progress.ecuReqId, progress.ecuSwDid);
                ret = ipmVerifyEcuVersionAndReportByDenpendency(&dependency);
            }

            adm_ips_policyDependencyDeInit(&dependency);
        }

        adm_ips_ecuProgressDeInit(&progress);
    }
    else
    {
#ifdef ADM_IPM_VERSION_VERIFY_INSTALL_HURRY
        adm_ips_ecuProgressInit(&progress);
        (void)adm_ips_ecuProgressGet(&progress, ecuReqId, ecuSwDid);
        // ECU安装成功，获取安装策略依赖校验ECU版本
        if (ADM_IPS_ECU_PHASE_POST_INSTALL == progress.state
            && ADM_ECODE_SUCCESSED == progress.progress[ADM_IPS_ECU_PHASE_INDEX_UPGRADE].result)
        {
            adm_ips_policyDependencyInit(&dependency);
            // 获取升级后的安装策略依赖
            ret = adm_ips_policyDependencyGetWithType(progress.ecuReqId, progress.ecuSwDid,
                (D32S)ADM_IPS_POLICY_DEPENDENCY_AFTER, &dependency);
            if (ADM_ECODE_SUCCESSED == ret)
            {
                // 校验ECU版本并上报结果
                log_i("Verify ecu version after installation, ecuReqId[%s] ecuSwDid[%s]", progress.ecuReqId, progress.ecuSwDid);
                ret = ipmVerifyEcuVersionAndReportByDenpendency(&dependency);
            }

            adm_ips_policyDependencyDeInit(&dependency);
        }

        adm_ips_ecuProgressDeInit(&progress);
#endif
    }

    return ret;
}
#if 0
static void ipmDoVerifyAllEcusVersionPostInstall()
{
    ADM_IPS_POLICY_DEPENDENCY_ARRAY_T dependencies;
    adm_ips_policyDependencyArrayInit(&dependencies);
    adm_ips_policyDependenciesGetWithType(&dependencies, ADM_IPS_POLICY_DEPENDENCY_AFTER);

    for (D32S i = 0; i < dependencies.dependenciesLen; i++)
    {
        ipmVerifyEcuVersionAndReportByDenpendency(&dependencies.dependencies[i]);
    }

    adm_ips_policyDependencyArrayDeInit(&dependencies);
}

/*一次性实时读取依赖中所有ECU信息，然后进行版本号和零件号的校验*/
static D32S ipmDoVerifyEcusVersionByAll(ADM_IPM_DO_FLASH_T *f)
{
    D32S nRet = ADM_ECODE_FAILED;

    if(f == NULL)
    {
        log_e("invalid param f");
        return ADM_ECODE_INVALID_PARAM;
    }

    ADM_ECS_RSCONFIGDATA_T *ppstOtaConfigData = NULL;
    ADM_EIM_GETECUINFO_T *allEcuInfo = NULL;
    ECU_INFO_T* ecuInfo = NULL;

    //1.获取配置用于ecu采集
    ppstOtaConfigData = adm_ecs_getOtaConfigDataFromKV();
    if (NULL == ppstOtaConfigData)
    {
        log_e("adm_ecs_getOtaConfigDataFromKV error");
        return ADM_ECODE_NO_MEMORY;
    }

    ADM_IPS_POLICY_DEPENDENCY_T* dependency = NULL;
    ADM_IPS_POLICY_DEPENDENCY_ARRAY_T dependencies;
    adm_ips_policyDependencyArrayInit(&dependencies);
    // adm_ips_policyDependencyArrayGet(&dependencies);
    adm_ips_policyDependenciesGetWithType(&dependencies, ADM_IPS_POLICY_DEPENDENCY_BEFORE);
    /* GAC_TODO：最好ips提供根据类型获取dependency的api，方便先获取所有升级前需要ecu信息，然后再统一校验
     * 不然app会写一些和ips重叠的代码
     * adm_ips_policyDependencyArrayGet(&dependency, 1);//1：升级前 2：升级后
     */

    //2.一次性实时读取依赖中所有ECU信息
    nRet = ipmCycleCollectEcuInfo(&(ppstOtaConfigData->stRsCfgInfo), &dependencies, &allEcuInfo);
    if(nRet != ADM_ECODE_SUCCESSED)
    {
        adm_ips_policyDependencyArrayDeInit(&dependencies);
        adm_ecs_delRsCfgDataInfo(&ppstOtaConfigData);
        log_e("Failed to get ecus info, nRet[ %d ]", nRet);
        return nRet;
    }

    D32S i = 0;
    DCHAR *strPolicyDidValue = NULL;
    DCHAR *strEcuDidValue = NULL;
    do
    {
        //3.判断依赖信息个数和采集到的ecu个数是否相等，如果不相等定然是校验不会通过
        if(dependencies.dependenciesLen != allEcuInfo->num)
        {
            log_e("The number of dependencies does not match, allEcuInfo num[ %d ]", allEcuInfo->num);
            nRet = ADM_ECODE_FAILED;
            break;
        }
    
        for(i = 0; i < dependencies.dependenciesLen; i++)//采集到的ecu信息与依赖依次比较
        {
            dependency = &dependencies.dependencies[i];
            if(dependency == NULL)
            {
                log_e("invalid pointer pDependency i[ %d ]", i);
                nRet = ADM_ECODE_INVALID_POINTER;
                break;
            };

            //4.根据requestId和did，获取策略文件中的依赖的didValue
            nRet = ipmFindPolicyDependencyDidValue(dependency, dependency->ecuReqId, ADM_DIM_STRING_SW_VERSION, strPolicyDidValue);
            if(nRet != ADM_ECODE_SUCCESSED)
            {
                log_e("Failed to find policy dependency did value, ecuReqId[ %s ] did[ %s ]", dependency->ecuReqId, ADM_DIM_STRING_SW_VERSION);
                break;
            }

            //5.根据requestId和did，获取采集到的ecu信息的didValue
            nRet = ipmFindEcusInfoDidValue(allEcuInfo, dependency->ecuReqId, ADM_DIM_STRING_SW_VERSION, strEcuDidValue);
            if(nRet != ADM_ECODE_SUCCESSED)
            {
                log_e("Failed to find ecu info did value, ecuReqId[ %s ] did[ %s ]", dependency->ecuReqId, ADM_DIM_STRING_SW_VERSION);
                break;
            }

            //6.比较软件版本号
            if(adm_strcmp(strPolicyDidValue, strEcuDidValue) != 0)
            {
                log_e("Failed to compare, strPolicyDidValue[ %s ] strEcuDidValue[ %s ]", strPolicyDidValue, strEcuDidValue);
                break;
            }

            // strPolicyDidValue 和 strEcuDidValue 需要释放
            adm_free(strPolicyDidValue);
            adm_free(strEcuDidValue);
            strPolicyDidValue = NULL;
            strEcuDidValue = NULL;
            //GAC_TODO: 校验零件号
            //7.根据requestId和did，获取策略文件中的依赖的didValue
            nRet = ipmFindPolicyDependencyDidValue(dependency, dependency->ecuReqId, ADM_DIM_STRING_PARTNUM, strPolicyDidValue);
            if(nRet != ADM_ECODE_SUCCESSED)
            {
                log_e("Failed to find policy dependency did value, ecuReqId[ %s ] did[ %s ]", dependency->ecuReqId, ADM_DIM_STRING_PARTNUM);
                break;
            }

            //8.根据requestId和did，获取采集到的ecu信息的didValue
            nRet = ipmFindEcusInfoDidValue(allEcuInfo, dependency->ecuReqId, ADM_DIM_STRING_PARTNUM, strEcuDidValue);
            if(nRet != ADM_ECODE_SUCCESSED)
            {
                log_e("Failed to find ecu info did value, ecuReqId[ %s ] did[ %s ]", dependency->ecuReqId, ADM_DIM_STRING_PARTNUM);
                break;
            }

            //9.比较零件号
            if(adm_strcmp(strPolicyDidValue, strEcuDidValue) != 0)
            {
                log_e("Failed to compare, strPolicyDidValue[ %s ] strEcuDidValue[ %s ]", strPolicyDidValue, strEcuDidValue);
                break;
            }
        }

        //版本号和零件号的校验成功后，需要保存最新的信息   （后续根据升级前后进行更新  （升级前无需更新，升级需要更新） 1：升级前 2：升级后）
        if(i == dependencies.dependenciesLen)
        {
            //云端获取配置信息 会更新本地KV数据
            nRet = adm_ecs_updateAndGetConfigInfo(&ppstOtaConfigData);
            if(nRet != ADM_ECODE_SUCCESSED)
            {
                log_e(" adm_ecs_updateAndGetConfigInfo failed,error[%d]", nRet);
                break;
            }
            /* 保存ecu info */
            adm_eim_getEcuInfoManagerEcusStore(allEcuInfo);
        }

    } while (FALSE);

    //10.释放内存空间
    adm_ips_policyDependencyArrayDeInit(&dependencies);
    adm_eim_deleteGetEcuInfoBody(allEcuInfo);
    adm_ecs_delRsCfgDataInfo(&ppstOtaConfigData);

    if (strPolicyDidValue != NULL)
    {
        adm_free(strPolicyDidValue);
        strPolicyDidValue = NULL;
    }
    if(strEcuDidValue != NULL)
    {
        adm_free(strEcuDidValue);
        strEcuDidValue = NULL;
    }

    return nRet;
}
#endif

static D32S ipmStepInstallPrepare(void)
{
    D32S ret = ADM_ECODE_FAILED;
    ADM_CVS_UPGRADE_MODE_E eUpgradeMode = ADM_CVS_UPGRADE_MODE_INVALID;
    ADM_CVS_VERSION_POLICY_T *versionPolicy = adm_cvs_newPolicyBody();

    (void)adm_cvs_getUpgradeMode((D32S *)&eUpgradeMode);

    if (versionPolicy == NULL)
    {
        ret = ADM_ECODE_NO_MEMORY;
    }
    else
    {
        // 清理加载到内存中的升级中的节点信息，如果不清理，上次升级的ECU信息会加载到内存中，导致刷写请求时误判ECU正在刷写中
        ipmFlashClear(&m_ipmContext.flash);

        ret = adm_cvs_getVersionPolicy(versionPolicy);
        log_d("adm_cvs_getVersionPolicy ret:%d, sessionId:%s, taskId:%lld, strategyId:%lld, installPolicyObjectStr:%s",
              ret, ADM_STR_SAFE_OUTPUT(versionPolicy->sessionId), versionPolicy->taskId, versionPolicy->strategyId,
              ADM_STR_SAFE_OUTPUT(versionPolicy->installMember.installPolicyObjectStr));
        if (ret == ADM_ECODE_SUCCESSED)
        {
            ADM_IPS_POLICY_TASK_T task;
            adm_ips_policyTaskInit(&task);
            //adm_ips_policyTaskGet(&task);
            if (ADM_IS_SUCCESS(adm_ips_policyTaskGet(&task)) && 
                (ipmVersionPolicyCompare(versionPolicy, &task) == 0) &&
                (D32S)ADM_CVS_UPGRADE_MODE_USB_CHECK != eUpgradeMode)
            {
            }
            else
            {
                adm_ips_stepInstallPrepare(versionPolicy,
                                           adm_cfg_iniParserGetInt(ADM_CFG_IPM_UPGRADE_PARALLEL_LIMIT_M, ADM_IPM_UPGRADE_PARALLEL_LIMIT_M_DEFAULT),
                                           adm_cfg_iniParserGetInt(ADM_CFG_IPM_UPGRADE_PARALLEL_LIMIT_N, ADM_IPM_UPGRADE_PARALLEL_LIMIT_N_DEFAULT));
                (void)adm_ipm_eventInfoInit();
            }
            adm_ips_policyTaskDeInit(&task);
        }
        adm_cvs_deletePolicyBody(versionPolicy);
    }

    ADM_ECODE_APPLICATION_CONVERSION(ret, ADM_ECODE_INSTALL_PREPARE_FAILED);
    return ret;
}

DCHAR* adm_ipm_reportConditionsNotMet(D32S ret, const ADM_COND_POLICY_T* pCondReq, const ADM_UIM_SOA_CONDITION_DATA_ARRAY_T* pConditions)
{
    if(ADM_ECODE_SUCCESSED == ret) {
        return NULL;
    }
	/*
    DCHAR* sessionId = uimSoaVehicleSessionId();
    adm_uim_soaInstallConditionsVerifyResultNotify(
       uimSoaCmdidTypeGet(),
       sessionId,
       ipmFlashStateString(ADM_IPM_FLASH_STATE_VERIFY_CONDITION),
       conditions,
       ret);
    */
    DCHAR* condsStr = NULL;
    json_object* pExtsObj = NULL;
    json_object* pItemObj = NULL;
    json_object* pCondObj = NULL;
    json_object* pCondList = NULL;
    json_object* pItem = NULL;
    json_object* messageId = NULL;
    json_object* result = NULL;
    json_object* currentData = NULL;
    json_object* messageName = NULL;
    DCHAR strTmp[32] = {0};
    do
    {
        if((NULL == pConditions)||(NULL == pCondReq)) {
            break;
        }
        pExtsObj = json_object_new_array_ext(1);
        if(NULL == pExtsObj) {
            break;
        }
        pCondList = json_object_new_array_ext((int)pConditions->condsArrayLength);
        if(NULL == pCondList) {
            break;
        }
        ADM_UIM_SOA_CONDITION_DATA_T**  condsArray = pConditions->condsArray;
        if(NULL == condsArray) {
            break;
        }
		
        for(D16U i=0; i<pConditions->condsArrayLength; i++) {
            pItem = json_object_new_object();
            if(NULL == pItem) {
                break;
            }
            (void)adm_snprintf(&strTmp[0], sizeof(strTmp), "0X%04x", condsArray[i]->msgId);
            messageId = json_object_new_string(&strTmp[0]);
            result = json_object_new_int(condsArray[i]->result);
#if 0			
            //目前只考虑字符串或数字
            if(0x06 == condsArray[i]->realVariable->data_type){
                currentData = json_object_new_string((DCHAR*)condsArray[i]->realVariable->v.value);    
            } else {
                snprintf(&strTmp[0], sizeof(strTmp), "%llu", condsArray[i]->realVariable->v.value_u64);
                currentData = json_object_new_string(strTmp);
            }  
#else			
            adm_memset(&strTmp[0], 0x00, sizeof(strTmp), sizeof(strTmp));
          
            switch ((VARIABLE_TYPE_EN)condsArray[i]->realVariable->data_type)
			{
                case VAR_U8:
                    (void)adm_snprintf(&strTmp[0], sizeof(strTmp), "%d", condsArray[i]->realVariable->v.value[0]);
					currentData = json_object_new_string(('\0' == strTmp[0]) ? "" : strTmp);
                    break;
                case VAR_U32:
				    (void)adm_snprintf(&strTmp[0], sizeof(strTmp), "%u", condsArray[i]->realVariable->v.value_u32);
					currentData = json_object_new_string(('\0' == strTmp[0]) ? "" : strTmp);
                    break;
                case VAR_S32:
                    (void)adm_snprintf(&strTmp[0], sizeof(strTmp), "%d", condsArray[i]->realVariable->v.value_s32);
					currentData = json_object_new_string(('\0' == strTmp[0]) ? "" : strTmp);
                    break;
                case VAR_U64:
				    (void)adm_snprintf(&strTmp[0], sizeof(strTmp), "%llu", condsArray[i]->realVariable->v.value_u64);
					currentData = json_object_new_string(('\0' == strTmp[0]) ? "" : strTmp);
                    break;
/*				case VAR_S64:
                    (void)adm_snprintf(didValueBuf, sizeof(didValueBuf)/sizeof(DCHAR), "%ld", didArray->didData->v.valueS64);
					didValue = adm_strdup(didValueBuf);
                    break;	
*/                    
                case VAR_FLOAT:
				    (void)adm_snprintf(&strTmp[0], sizeof(strTmp), "%f", condsArray[i]->realVariable->v.value_f);
					currentData = json_object_new_string(('\0' == strTmp[0]) ? "" : strTmp);
                    break;
                case VAR_STRING:
                    /* code */
					currentData = json_object_new_string(
                        ('\0' == condsArray[i]->realVariable->v.value[0]) ? "" : (const DCHAR*)condsArray[i]->realVariable->v.value);    
                    break;

                default:
					currentData = json_object_new_string("Invalid data");
                    break;
            }
			adm_memset(&strTmp[0], 0x00, sizeof(strTmp), sizeof(strTmp));
#endif      
            messageName = NULL;
            
            for(D32S j=0; j<pCondReq->iCondNum; j++){ 
				log_d("adm_ipm_reportConditionsNotMet messageId=%s",pCondReq->pCondtionS[j].messageId);
			    log_d("adm_ipm_reportConditionsNotMet msgId=%d",condsArray[i]->msgId);
				if ((D32S)htoi((char *)pCondReq->pCondtionS[j].messageId) == condsArray[i]->msgId)
	            {
	                if(pCondReq->pCondtionS[j].messageName != NULL){
	                    messageName = json_object_new_string(pCondReq->pCondtionS[j].messageName);
	                }
					else{
                        messageName = json_object_new_string("NULL");
					}
					break;
				}

        	}

			if(messageName == NULL){
                log_e("not found messageName for messageId [%d]",condsArray[i]->msgId); 
				messageName = json_object_new_string("NULL");
//				break;
			}

//            messageName = json_object_new_string(condsArray[i]->messageName);
            if(json_object_object_add(pItem, "messageId", messageId) != 0) {
                log_e("json_object_object_add failed");    
                break;
            }
	    messageId = NULL;
			
	    if(json_object_object_add(pItem, "messageName", messageName) != 0	) {
                log_e("json_object_object_add failed");    
                break;
            }
	    messageName = NULL;

	    if(json_object_object_add(pItem, "currentData", currentData) != 0	) {
                log_e("json_object_object_add failed");    
                break;
            }
	    currentData = NULL;

	    if(json_object_object_add(pItem, "result", result) != 0	) {
                log_e("json_object_object_add failed");    
                break;
            }
	    result = NULL;
            if(json_object_array_add(pCondList, pItem) == 0) {
                pItem = NULL;
            }
            else {
                log_e("json_object_array_add failed");
                break;
            }

        }

        pCondObj = json_object_new_object();
        if(NULL == pCondObj) {
            break;
        }
        if(currentData != NULL){
	    (void)json_object_put(currentData);
	    currentData = NULL;
        }

        currentData = json_object_new_string("conditions");

        if(json_object_object_add(pCondObj, "extKey", currentData) != 0) {            
            log_e("json_object_object_add failed");    
            if(currentData != NULL){
		(void)json_object_put(currentData);
		currentData = NULL;
            }
        }


		log_d("adm_ipm_reportConditionsNotMet pCondList = %s",ADM_STR_SAFE_OUTPUT(json_object_get_string(pCondList)));
        
        currentData = json_object_new_string(json_object_get_string(pCondList));

        if(json_object_object_add(pCondObj, "extValue", currentData) != 0) {
            log_e("json_object_object_add failed"); 
        }
        else{
            currentData = NULL;
        }

        if(json_object_array_add(pExtsObj, pCondObj) != 0) {
            log_e("json_object_array_add failed"); 
        }
        else {
            pCondObj = NULL;
        }
    }while(FALSE);

    if(pExtsObj != NULL) {        
        condsStr = adm_strdup(json_object_get_string(pExtsObj));
    }
    else {
        condsStr = adm_strdup("[]");
    }
 

    if(messageId != NULL){
        (void)json_object_put(messageId);
    }
	if(result != NULL){
        (void)json_object_put(result);
    }
	if(currentData != NULL){
        (void)json_object_put(currentData);
    }
	if(messageName != NULL){
        (void)json_object_put(messageName);
    }
	if(pItem != NULL){
        (void)json_object_put(pItem);
    }
	if(pCondList != NULL){
        (void)json_object_put(pCondList);
    }
	if(pCondObj != NULL){
        (void)json_object_put(pCondObj);
    }
	if(pExtsObj != NULL){
        (void)json_object_put(pExtsObj);
    }
		
    
    return condsStr;
}

/*!
 *******************************************************************************
 * 获取安装前置条件校验的类型
 *
 * \note 
 *
 * \retval      ADM_IPM_CONDITION_TYPE_E 
 *******************************************************************************
 */
static ADM_IPM_CONDITION_TYPE_E ipmVerifyConditionTypeGet(void)
{
    D32S iResult = ADM_ECODE_FAILED;
	ADM_CVS_UPGRADE_MODE_E eMode = ADM_CVS_UPGRADE_MODE_INVALID;
    ADM_IPM_CONDITION_TYPE_E  eType = ADM_IPM_CONDITION_TYPE_INVALID;

    //获取触发安装的类型
	ADM_APPLICATION_COMMON_CMDID_TYPE_E eCmdType = (ADM_APPLICATION_COMMON_CMDID_TYPE_E)adm_ips_cmdIdGet();

	switch (eCmdType)
	{
		case ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_USER:    //常规安装
			eType = ADM_IPM_CONDITION_TYPE_USER;
			break;
		case ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_IGN:     //常规安装
			eType = ADM_IPM_CONDITION_TYPE_USER;
			break;
		case ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_SILENCE: //静默安装
			eType = ADM_IPM_CONDITION_TYPE_SLIENCE;
			break;
		case ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_PHONE: //手机安装
			eType = ADM_IPM_CONDITION_TYPE_PHONE;
			break;
		case ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_SCHDULE: //预约安装
			eType = ADM_IPM_CONDITION_TYPE_SCHDULE;
			break;
		case ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_PROD_LINE: //产线安装
			eType = ADM_IPM_CONDITION_TYPE_PROD_LINE;
			break;
        case ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_RESCUE:  //救援升级
            eType = ADM_IPM_CONDITION_TYPE_RESCUE;
            break;
        case ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_APK_SELF_UPGRADE://APK 
            eType = ADM_IPM_CONDITION_TYPE_APK_SELF;
            break;
        case ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_FORCE:   //FORCE
            eType = ADM_IPM_CONDITION_TYPE_FORCE;
            break;
        case ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_USB:
            eType = ADM_IPM_CONDITION_TYPE_USB;
            break;
		default:
			eType = ADM_IPM_CONDITION_TYPE_USER;
			break;
	}

	log_d("iType[ %d ]", eType );  
	return eType;
}

static void ipmInstallConditionsVerifyResultPrase(DCHAR *pcFailedDesc, DCHAR **ppcConditionResult)
{
    json_object *pJsonVal = NULL;
    json_object *pJsonConditionVal = NULL;
    
    ADM_COM_FUNC_ENTRY();
    
    if (ADM_IS_NULL(pcFailedDesc) || ADM_IS_NULL(ppcConditionResult))
    {
        log_e("invalid param");
        return;
    }
    
    log_d("pcFailedDesc = %s", pcFailedDesc);
    
    do
    {
        pJsonVal = json_tokener_parse(pcFailedDesc);
        if (NULL == pJsonVal)
        {
            log_e("json_tokener_parse failed");
            break;
        }
        
        pJsonConditionVal = json_object_object_get(pJsonVal, "condsArrarList");
        if (NULL == pJsonConditionVal)
        {
            log_e("get condsArrarList failed");
            break;
        }

        *ppcConditionResult = adm_strdup(json_object_get_string(pJsonConditionVal));
        if (NULL == *ppcConditionResult)
        {
            log_e("adm_strdup failed");
            break;
        }
        
        log_d("*ppcConditionResult = %s", ADM_STR_SAFE_OUTPUT(*ppcConditionResult));
        
    } while (FALSE);
    
    if (NULL != pJsonVal)
    {
        (void)json_object_put(pJsonVal);
    }
    
    ADM_COM_FUNC_EXIT();

    return;
}

static D32S ipmStepVerifyCondition(ADM_IPM_FLASH_T *f){
    D32S result = ADM_ECODE_FAILED;
    DBOOL bCheck = DTRUE;
    DBOOL pbIsConform = FALSE;
    ADM_UIM_SOA_CONDITION_DATA_ARRAY_T* pStCondResults = NULL;
    DCHAR *pcHmiEventDesc = NULL;

    log_i("[ABUP-Procedure]: Install Start wakeup");
    if((result = ipmDimSoaHostVerifyConditionStartWakeUp(f->dim)) != ADM_ECODE_SUCCESSED)
    {
        log_e("[ABUP-Procedure]:Install Verify Condition Start WakeUp fail, error code [%d]", result);
        return result;
    }
    
    ADM_COND_POLICY_T cond;
    adm_condPolicyInit(&cond);
    D32S iType = (D32S)ipmVerifyConditionTypeGet();/* TODO value get */
    // (void)adm_ips_policyConditionGet(iType, &cond);
    (void)adm_ips_getConditionFromPolicy(iType, &cond);

    if ((D32S)ADM_IPM_CONDITION_TYPE_RESCUE == iType || (D32S)ADM_IPM_CONDITION_TYPE_APK_SELF == iType)
    {
        /** 救援升级 & APK自升级允许条件为空，其他模式自行添加 */
        if (0 == cond.iCondNum || NULL == cond.pCondtionS)
        {
            log_i("Rescue mode, get no condition list.");
            bCheck = DFALSE;
            result = ADM_ECODE_SUCCESSED;
        }
    }

    if (DTRUE == bCheck)
    {
    ////#ifdef ADM_IPM_TEMP
    //    result = ADM_ECODE_SUCCESSED;
    //#else
        log_i("[ABUP-Procedure]: Install Start verify conditions");
        result = adm_cdm_verify_Compare(&cond, &pStCondResults, &pbIsConform);
    //#endif//ADM_IPM_TEMP
    }

    if(ADM_ECODE_SUCCESSED != result){
		result = ADM_ECODE_INSTALL_CONDITION_FAILED;
		DCHAR* condsStr = adm_ipm_reportConditionsNotMet(result, &cond, pStCondResults);
	    DCHAR reason[128] = {0};
		(void)adm_snprintf(reason, sizeof(reason) / sizeof(DCHAR), "%d", result);
	    ipmReportOtaServerTaskStatus(&f->report->reportOtaServer, ADM_IRM_REPORT_SERVER_STATUS_INSTALL_CONDITIONS_NOT_MET, reason, condsStr);
		adm_free(condsStr);
		
        DCHAR* eventDesc = ipmInstallConditionsVerifyResultEventDesc(pStCondResults, result);
        if(eventDesc == NULL){
            log_w("getting Verify Condition descripe fail");
        }
		else{
			DCHAR eventDescTmp[2048] = {0};
	        (void)adm_memset(eventDescTmp, 0x00, sizeof(eventDescTmp), sizeof(eventDescTmp));	
            (void)adm_snprintf(eventDescTmp, sizeof(eventDescTmp), "Installation pre condition check failed.(%s)", eventDesc);
            log_d("eventDescTmp = %s", eventDescTmp);
            ipmReportEventDependencyEventId("II_008",  eventDescTmp, ADM_ECODE_SUCCESSED);
            pcHmiEventDesc = ipmInstallConditionsVerifyResultEventDescToHmi(pStCondResults, result);
            if(NULL != pcHmiEventDesc)
            {
                log_d("pcHmiEventDesc = %s", pcHmiEventDesc);
                ipmInstallConditionsVerifyResultPrase(pcHmiEventDesc, &f->report->reportHmi.pcConditionResult);
                adm_free(pcHmiEventDesc);
            }
            adm_free(eventDesc);
        }
    }
	else{
        ipmReportEventDependencyEventId("II_007",  NULL, ADM_ECODE_SUCCESSED);
	}
    log_i("[ABUP-Procedure]: Install verify conditions end, [err:%d]", result);

    if(pStCondResults != NULL){
        (void)adm_cdm_verify_free(pStCondResults);
    }
    adm_condPolicyDeInit(&cond);
    
    log_i("[ABUP-Procedure]: Install Start Stop wakeup");
    (void)ipmDimSoaHostExecOtaTaskStopWakeUp(f->dim);
    log_i("[ABUP-Procedure]: Install End stop wakeup");

    ADM_ECODE_APPLICATION_CONVERSION(result, ADM_ECODE_INSTALL_CONDITION_FAILED);
    return result;
}

/**
 * @brief 在进入安装前进行安装条件校验
 * @param f [in] ADM_APPLICATION_COMMON_CMDID_TYPE_E 安装类型
 * @return D32S 0:成功, 其他:失败
 */
D32S adm_ipm_stepVerifyConditionBeforeInstall(ADM_APPLICATION_COMMON_CMDID_TYPE_E eCmdidType)
{
    D32S iResult = ADM_ECODE_SUCCESSED;
    DCHAR *pcVerifyResult = NULL;
    DBOOL pbIsConform = FALSE;
    ADM_UIM_SOA_CONDITION_DATA_ARRAY_T *pStCondResults = NULL;
    DCHAR *pcHmiEventDesc = NULL;

    ADM_IPM_FLASH_T *f = &m_ipmContext.flash;

    do
    { 
        /** 1.产线任务可以通过上电/手动检测到,进入ota模式前需要判断车况条件是否满足 ,避免不满足而进入了ota模式
           2.tsp推送的强制、静默任务可以带预约时间，预约时间到达后，在进入OTA模式前需要判断车况是否符合条件 */
        if (ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_FORCE != eCmdidType &&
            ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_SILENCE != eCmdidType &&
            ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_PROD_LINE != eCmdidType)
        {
            log_d("Not prodLine/force/silence task, skip tsp condition verify before OTA.");
            iResult = ADM_ECODE_SUCCESSED;
            break;
        }

        log_i("[ABUP-Procedure]: Tsp task condition verify before OTA, cmdIdType[%d]", eCmdidType);

        (DVOID)ipmStepInstallPrepare(); ///< ips相关数据先重新加载下

        ADM_COND_POLICY_T cond;
        adm_condPolicyInit(&cond);
        D32S iType = (D32S)ipmVerifyConditionTypeGet(); 
        (void)adm_ips_getConditionFromPolicy(iType, &cond);

        iResult = adm_cdm_verify_Compare(&cond, &pStCondResults, &pbIsConform);
        if (ADM_ECODE_SUCCESSED != iResult)
        {
            iResult = ADM_ECODE_INSTALL_CONDITION_FAILED;
            DCHAR *condsStr = adm_ipm_reportConditionsNotMet(iResult, &cond, pStCondResults);
            DCHAR aReason[128] = {0};
            (void)adm_snprintf(aReason, sizeof(aReason) / sizeof(DCHAR), "%d", iResult);
            ipmReportOtaServerTaskStatus(&f->report->reportOtaServer, ADM_IRM_REPORT_SERVER_STATUS_Task_Install_Failed, aReason, condsStr);
            adm_free(condsStr);

            sleep(8); ///< 增加延迟,让hmi有时间处理下载结果
            /** HMI按照一般性任务失败处理，没有其他合理的可用的错误码 */
            ipmReportHmiResult(&f->report->reportHmi, ADM_ECODE_INSTALL_FAILED);

            ipmReportUpLogFilePostInstall(ADM_ECODE_INSTALL_FAILED);
        }

        if (pStCondResults != NULL)
        {
            (void)adm_cdm_verify_free(pStCondResults);
        }
        adm_condPolicyDeInit(&cond);
    } while (FALSE);

    log_i("[ABUP-Procedure]: Verify conditions before OTA end, [err:%d]", iResult);

    ADM_ECODE_APPLICATION_CONVERSION(iResult, ADM_ECODE_INSTALL_FAILED);

    return iResult;
}

static D32S ipmStepEnvEntry(ADM_IPM_FLASH_T *f){
    return ipmDimEnvEntry(f->dim);
}

/*!
 *******************************************************************************
 * IPM任务有效性校验结果处理：对错误类型为网络类型的错误设置为ADM_ECODE_INSTALL_TASK_VERIFY_NET_FAILED
 *
 * \note 
 *
 * \param[in]   iError      任务校验结果
 * 
 * \retval      D32S  封装后的校验结果
 *******************************************************************************
 */
static D32S ipmVerifyTaskResultHandle(D32S iError)
{
	D32S iRet = ADM_ECODE_FAILED;
	
	switch (iError)
	{
        case ADM_ECODE_UPGRADE_TASK_STOPED:  ///!< 只有任务终止的时候，才报任务校验失败
            iRet = ADM_ECODE_INSTALL_TASK_VERIFY_FAILED;
            break;
        default:
            iRet = ADM_ECODE_SUCCESSED;
            break;

		// case ADM_ECODE_SUCCESSED:
		// 	iRet = ADM_ECODE_SUCCESSED;
		// 	break;
        // case ADM_ECODE_OPERATION_TIMEDOUT:		                   //操作超时	
		// case ADM_ECODE_SEND_ERROR:
		// case ADM_ECODE_RECV_ERROR:
		// case ADM_ECODE_REQUEST_TIMEOUT:                            //请求超时
		// 	iRet = ADM_ECODE_INSTALL_TASK_VERIFY_NET_FAILED;       //任务有效性校验网络通讯失败
		// 	break;
		// default:
		// 	iRet = ADM_ECODE_INSTALL_TASK_VERIFY_FAILED;           //任务有效性校验任务无效
		// 	break;
	}

	return iRet;

}


static D32S ipmStepVerifyTask(ADM_IPM_FLASH_T *f)
{
    D32S nRet = ADM_ECODE_FAILED;
    D32S iCmdidType = -1;

    log_i("[ABUP-Procedure]: Install verify task start");
    /** 上报安装准备状态 */
    ipmReportOtaServerTaskStatusInstallPrepare(&f->report->reportOtaServer);

    nRet = adm_cvs_getUpgradeMode(&iCmdidType);

    /** U盘模式下，不进行有效性校验 */
    if(ADM_IS_SUCCESS(nRet) && 
       (D32S)ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_USB == iCmdidType)
    {
        log_w("U disk mode, skip verify task.");
        return ADM_ECODE_SUCCESSED;
    }

    /** 升级包有效性校验 */
    nRet = ipmTaskVerify();
    if (nRet != ADM_ECODE_SUCCESSED)
    {
        log_e("Failed to task verify before install, nRet[ %d ]", nRet);
        ipmReportEventDependencyEventId("IP_008", NULL, nRet);
    }
    else
    {
        ipmReportEventDependencyEventId("IP_007", NULL, ADM_ECODE_SUCCESSED);
    }

    log_i("[ABUP-Procedure]: Install verify task end, [err:%d]", nRet);
    return ipmVerifyTaskResultHandle(nRet);
}

/*!
 *******************************************************************************
 * IPM模块本地（未传输）升级包验签
 *
 * \note 
 *
 * \param[in]   data      pis包信息
 * \param[out]   originimagePackage      原包信息
 * \retval      D32S
 *******************************************************************************
 */
static D32S adm_ipm_local_signatureVerify(const ADM_PIS_PACKAGE_DATA_T *data, ADM_PIS_FILE_T *originimagePackage){
    D32S ret = ADM_ECODE_FAILED;
    ADM_COM_FILEOBJECT_T stFileObject = {0};
	ADM_COM_SIGNATUREINFO_T *pstSignatureInfo = NULL;
	DCHAR *pcoriginfileName = NULL;
	DCHAR *pcsignFileName = NULL;
	D32S errorCode = 0;
    DBOOL bIsPkiVerifySign = DFALSE;

	if (ADM_IS_NULL(originimagePackage)||ADM_IS_NULL(data)){
        ret = ADM_ECODE_INVALID_PARAM;
        log_e("Invalid input param");
        return ret;
    }

    pstSignatureInfo = adm_com_newSignatureInfo();
    if (ADM_IS_NULL(pstSignatureInfo)){
        ret = ADM_ECODE_NO_MEMORY;
        log_e("Failed to malloc pstSignatureInfo info. err[%d]", ADM_ECODE_NO_MEMORY);
        return ret;
    }

	stFileObject.fileName = NULL;
	stFileObject.url = NULL;
	stFileObject.hash = data->imagePackage.hash;
	stFileObject.filePath = data->imagePackage.path;
	stFileObject.size = data->imagePackage.size;
	stFileObject.pstFileSecurityInfo = data->fileSecurityInfo;
	
	//!< 验签的条件是否满足 
    if (FALSE == adm_com_checkIsNeedToVerifySignature(&stFileObject))
    {
        ret = ADM_ECODE_SUCCESSED;
        log_d("dont need to verify signature");
		adm_com_delSignatureInfo(&pstSignatureInfo);
		originimagePackage->size = data->imagePackage.size;
		originimagePackage->hash = adm_strdup(data->imagePackage.hash);
		originimagePackage->path = adm_strdup(data->imagePackage.path);
        return ret;
    }

#ifdef SUPPORT_LION_PKI
            bIsPkiVerifySign = DTRUE;
#endif

	ret = adm_com_getSignatureData(&stFileObject, bIsPkiVerifySign, pstSignatureInfo);
    if(ADM_IS_ERR(ret)){
		adm_com_delSignatureInfo(&pstSignatureInfo);
        return ret;	
    }
	ret = adm_dim_host_verifySignature(adm_soa_getHostInstanceId(), ADM_IPM_SIGN_REQ_TIMEOUT, 
                                pstSignatureInfo->signfileinfo, pstSignatureInfo->signfileinfolen, 
                                pstSignatureInfo->beVerifiedHash, pstSignatureInfo->beVerifiedHashLen, &errorCode);
    if (ADM_IS_ERR(ret)){
        log_e("verify sign failed !");
	    adm_com_delSignatureInfo(&pstSignatureInfo);
		return ret;	
    }
    log_d("verify install package signature data ok, signData[%s], signlen[%d], originhash[%s], originhashlen[%d]",
        pstSignatureInfo->signfileinfo, pstSignatureInfo->signfileinfolen, pstSignatureInfo->beVerifiedHash, pstSignatureInfo->beVerifiedHashLen);

	adm_com_delSignatureInfo(&pstSignatureInfo);

	ret = adm_com_SeparateOriginAndSignFile(&stFileObject, bIsPkiVerifySign, &pcoriginfileName, &pcsignFileName);
	if (ADM_IS_ERR(ret)){
        log_e("separate origin file failed !");
//	    adm_com_delSignatureInfo(&pstSignatureInfo);
		return ret;	
    }
	ret = ADM_ECODE_VERIFY_SIGNATURE_FAILED;
	for(D32S i=0; i< data->fileSecurityInfo->fileMetaNum; i++){
        if (ADM_SIM_SOA_UCL_FILE_TYPE_ORI == (FILE_TYPE_E)data->fileSecurityInfo->pstFileMetaList[i].type)
        {
            originimagePackage->size = data->fileSecurityInfo->pstFileMetaList[i].size;
			originimagePackage->hash = adm_strdup(data->fileSecurityInfo->pstFileMetaList[i].stCheckSum.value);
			ret = ADM_ECODE_SUCCESSED;
			break;
        }
        
	}
	
	originimagePackage->path = adm_strdup(pcoriginfileName);
//	originimagePackage->hash = adm_strdup(data->fileSecurityInfo->oriHash);
//	originimagePackage->size = data->fileSecurityInfo->oriSize;

	adm_free(pcoriginfileName);
	adm_free(pcsignFileName);
	return ret;	
    
}

#if 0
/* ADM_OCS_ECU_DID_T */
static void signVerifyInfoInit(ADM_IPM_REMOTE_VERIFY_SIGN_T *psigninfo){
    psigninfo->VerifySignInfo.fileId = NULL;
    psigninfo->VerifySignInfo.VerifySignResultInfo.result = 0;
    psigninfo->VerifySignInfo.VerifySignResultInfo.filePath = NULL;
	psigninfo->VerifySignInfo.VerifySignResultInfo.signature = NULL;
	psigninfo->VerifySignInfo.VerifySignResultInfo.signatureLen = 0;
	psigninfo->wait = 0;
	psigninfo->timeout = 0;//ADM_IPM_REMOTE_SIGN_TIMEOUT;
	psigninfo->lastTime = 0;

}
static void signVerifyInfoDeInit(ADM_IPM_REMOTE_VERIFY_SIGN_T *psigninfo){
    adm_free(psigninfo->VerifySignInfo.fileId);
    psigninfo->VerifySignInfo.VerifySignResultInfo.result = 0;
    adm_free(psigninfo->VerifySignInfo.VerifySignResultInfo.filePath);
	adm_free(psigninfo->VerifySignInfo.VerifySignResultInfo.signature);
	psigninfo->VerifySignInfo.VerifySignResultInfo.signatureLen = 0;
	psigninfo->wait = 0;
	psigninfo->timeout = 0;//ADM_IPM_REMOTE_SIGN_TIMEOUT;
	psigninfo->lastTime = 0;
}


/*!
*******************************************************************************
* 删除验签file meta 数据.
*
* \note
*
* \param[in]  pfileMetasList file list信息
* \retval	   none
*******************************************************************************
*/
void adm_ipm_delSignatureInfo(FILEMETAS_INFO_ARRAY_T* pfileMetasList)
{
	if (ADM_IS_NULL(pfileMetasList)){
		return;
	}
	for(D32U i = 0; i < pfileMetasList->fileMetaNum; ++i){
		if(pfileMetasList->fileMetasArray[i]){
            adm_free(pfileMetasList->fileMetasArray[i]->tObject.pAbstracValue);
		}
        adm_free(pfileMetasList->fileMetasArray[i]);
    }
    adm_free(pfileMetasList->fileMetasArray);
    adm_free(pfileMetasList);

	return;
}


  

static ADM_IPM_REMOTE_VERIFY_SIGN_T m_ipmVerifySign = {0};

static DBOOL ipmDoSignatureVerifyTimeOut(ADM_IPM_REMOTE_VERIFY_SIGN_T *pipmVerifySign){
    DBOOL dRet = 0;
    D64U curTime = ipmMonotonicTimeMSecGet();
    D64U deltaTime = curTime - pipmVerifySign->lastTime;
    if(deltaTime < pipmVerifySign->timeout){
	pipmVerifySign->timeout -= deltaTime;
    }else{
	pipmVerifySign->timeout = 0;
	dRet = 1;
    }
    pipmVerifySign->lastTime = curTime;
    return dRet;
}


D32S adm_ipm_remoteSignVerifyProgressNotifyCallback(const DCHAR* pfileID, const D16U progress){
    D32S ret = ADM_ECODE_FAILED;
    adm_ipm_init();
    pthread_mutex_lock(&m_ipmContext.flash.mutex);
	if (ADM_IS_NULL(pfileID)){
        ret = ADM_ECODE_INVALID_PARAM;
        log_e("pfileID is NULL");
	    pthread_mutex_unlock(&m_ipmContext.flash.mutex);
        return ret;
    }

	if (adm_strcmp(pfileID, m_ipmVerifySign.VerifySignInfo.fileId) != 0){
        ret = ADM_ECODE_INVALID_PARAM;
        log_d("remote verify fileID[%s] error, expected [%s]",ADM_STR_SAFE_OUTPUT(pfileID), ADM_STR_SAFE_OUTPUT(m_ipmVerifySign.VerifySignInfo.fileId));
	    pthread_mutex_unlock(&m_ipmContext.flash.mutex);
        return ret;
    }

	m_ipmVerifySign.timeout = ADM_IPM_REMOTE_SIGN_TIMEOUT;
	log_d("remote verify fileID[%s], progress[%d]",ADM_STR_SAFE_OUTPUT(pfileID), progress);

	ret = ADM_ECODE_SUCCESSED;
	pthread_mutex_unlock(&m_ipmContext.flash.mutex);
	return ret;
}


D32S adm_ipm_remoteSignVerifyResultNotifyCallback(const DCHAR* pfileID, const ADM_SYSI4UCL_VERIFY_SIGN_RESULT_INFO_T* pVerifySignResultInfo){
	D32S ret = ADM_ECODE_FAILED;
	adm_ipm_init();
	pthread_mutex_lock(&m_ipmContext.flash.mutex);
	if (ADM_IS_NULL(pfileID)){
		ret = ADM_ECODE_INVALID_PARAM;
		log_e("pfileID is NULL");
		pthread_mutex_unlock(&m_ipmContext.flash.mutex);
		return ret;
	}

	if (adm_strcmp(pfileID, m_ipmVerifySign.VerifySignInfo.fileId) != 0){
		ret = ADM_ECODE_INVALID_PARAM;
		log_d("remote verify fileID[%s] error, expected [%s]",ADM_STR_SAFE_OUTPUT(pfileID), ADM_STR_SAFE_OUTPUT(m_ipmVerifySign.VerifySignInfo.fileId));
		pthread_mutex_unlock(&m_ipmContext.flash.mutex);
		return ret;
	}
	
    m_ipmVerifySign.VerifySignInfo.VerifySignResultInfo.result = pVerifySignResultInfo->result;
    if((0x01 == pVerifySignResultInfo->result)||(0xff == pVerifySignResultInfo->result)){	
		m_ipmVerifySign.VerifySignInfo.VerifySignResultInfo.signature = adm_strdup(pVerifySignResultInfo->signature);
		m_ipmVerifySign.VerifySignInfo.VerifySignResultInfo.signatureLen = pVerifySignResultInfo->signatureLen;
		m_ipmVerifySign.VerifySignInfo.VerifySignResultInfo.filePath = adm_strdup(pVerifySignResultInfo->filePath);
    }
	m_ipmVerifySign.wait = 0;
	
	log_d("remote verify fileID[%s], result[%d]",ADM_STR_SAFE_OUTPUT(pfileID), pVerifySignResultInfo->result);

	ret = ADM_ECODE_SUCCESSED;
	pthread_mutex_unlock(&m_ipmContext.flash.mutex);
	return ret;
}



/*!
 *******************************************************************************
 * IPM模块远程（已传输）升级包验签
 *
 * \note 
 *
 * \param[in]   pFileId   
 * \param[in]   pfileMetasList      文件meta信息
 * \param[in]   data      pis包信息
 * \param[out]   originimagePackage      原包信息
 * \retval      D32S
 *******************************************************************************
 */
D32S adm_ipm_remoteDoSignatureVerify(DCHAR *pFileId, const FILEMETAS_INFO_ARRAY_T* pfileMetasList, const ADM_PIS_PACKAGE_DATA_T *data, ADM_PIS_FILE_T *originimagePackage){
    D32S ret = ADM_ECODE_FAILED;
	D32S errorCode = 0;

	if (ADM_IS_NULL(pFileId)||ADM_IS_NULL(pfileMetasList)||ADM_IS_NULL(data)||ADM_IS_NULL(originimagePackage)){
        ret = ADM_ECODE_INVALID_PARAM;
        log_e("Invalid input param");
        return ret;
    }

    pthread_mutex_lock(&m_ipmContext.flash.mutex);
	ret = adm_dim_ucl_request_Verify_Sign(adm_soa_getUclInstanceId(),
		                                  ADM_IPM_REMOTE_SIGN_REQ_TIMEOUT, 
                                          pFileId, 
                                          data->imagePackage.path, 
                                          pfileMetasList,
                                          TRUE,
                                          &errorCode);
    if(ret == 0){
		log_e("dim request_Verify_Sign success");
	}
	else{
		pthread_mutex_unlock(&m_ipmContext.flash.mutex);
		log_e("dim request_Verify_Sign failed, retCode=%d", ret);
		return ret;
	}

	m_ipmVerifySign.VerifySignInfo.fileId = adm_strdup(pFileId);
	m_ipmVerifySign.timeout = ADM_IPM_REMOTE_SIGN_TIMEOUT;
	m_ipmVerifySign.lastTime = ipmMonotonicTimeMSecGet();
	m_ipmVerifySign.wait = 1;
	
	while (m_ipmVerifySign.wait){
        if(0 != ipmDoSignatureVerifyTimeOut(&m_ipmVerifySign)){
	        ret = ADM_ECODE_MSG_WAIT_TIMEOUT;
			pthread_mutex_unlock(&m_ipmContext.flash.mutex);
			return ret;
	    }
		pthread_mutex_unlock(&m_ipmContext.flash.mutex);
        usleep(ADM_IPM_REMOTE_SIGN_TIME_USEC_WIDE);
        pthread_mutex_lock(&m_ipmContext.flash.mutex);
        
    }
										  
	pthread_mutex_unlock(&m_ipmContext.flash.mutex);

	ret = ADM_ECODE_VERIFY_SIGNATURE_FAILED;
	if(m_ipmVerifySign.VerifySignInfo.VerifySignResultInfo.result == 0x01){
		originimagePackage->path = adm_strdup(m_ipmVerifySign.VerifySignInfo.VerifySignResultInfo.filePath);
		for(D32S i=0; i< data->fileSecurityInfo->fileMetaNum; i++){
	        if (ADM_SIM_SOA_UCL_FILE_TYPE_ORI == data->fileSecurityInfo->pstFileMetaList[i].type)
	        {
	            originimagePackage->size = data->fileSecurityInfo->pstFileMetaList[i].size;
				originimagePackage->hash = adm_strdup(data->fileSecurityInfo->pstFileMetaList[i].stCheckSum.value);
				ret = ADM_ECODE_SUCCESSED;
				break;
	        }
	        
		}
//		originimagePackage->hash = adm_strdup(data->fileSecurityInfo->oriHash);
//		originimagePackage->size = data->fileSecurityInfo->oriSize;
		log_d("remote verify install package signature result 01, filePath[%s], size[%lld], originhash[%s], originhashlen[%lld]",
				ADM_STR_SAFE_OUTPUT(originimagePackage->path), originimagePackage->size, 
				ADM_STR_SAFE_OUTPUT(originimagePackage->hash), adm_strlen(originimagePackage->hash));
	}
    else if(m_ipmVerifySign.VerifySignInfo.VerifySignResultInfo.result == 0xff){
		for(D32S i=0; i< data->fileSecurityInfo->fileMetaNum; i++){
	        if (ADM_SIM_SOA_UCL_FILE_TYPE_ORI == data->fileSecurityInfo->pstFileMetaList[i].type)
	        {
	            originimagePackage->size = data->fileSecurityInfo->pstFileMetaList[i].size;
				originimagePackage->hash = adm_strdup(data->fileSecurityInfo->pstFileMetaList[i].stCheckSum.value);
				ret = ADM_ECODE_SUCCESSED;
				break;
	        }
	        
		}
	    log_d("remote verify install package signature data ok, signData[%s], signlen[%lld], originhash[%s], originhashlen[%lld]",
				ADM_STR_SAFE_OUTPUT(m_ipmVerifySign.VerifySignInfo.VerifySignResultInfo.signature), m_ipmVerifySign.VerifySignInfo.VerifySignResultInfo.signatureLen, 
				ADM_STR_SAFE_OUTPUT(originimagePackage->hash), adm_strlen(originimagePackage->hash));
        ret = adm_dim_host_verifySignature(adm_soa_getHostInstanceId(), ADM_IPM_SIGN_REQ_TIMEOUT, 
                                m_ipmVerifySign.VerifySignInfo.VerifySignResultInfo.signature, m_ipmVerifySign.VerifySignInfo.VerifySignResultInfo.signatureLen, 
                                originimagePackage->hash, adm_strlen(originimagePackage->hash), &errorCode);
		
		if (ADM_IS_ERR(ret)){
            log_e("verify sign failed !");
		    return ret;	
        }
        
		originimagePackage->path = adm_strdup(m_ipmVerifySign.VerifySignInfo.VerifySignResultInfo.filePath);
		
//		originimagePackage->hash = adm_strdup(data->fileSecurityInfo->oriHash);
//		originimagePackage->size = data->fileSecurityInfo->oriSize;	
    }
	else if((m_ipmVerifySign.VerifySignInfo.VerifySignResultInfo.result>=0x02)&&(m_ipmVerifySign.VerifySignInfo.VerifySignResultInfo.result<=0xfe)){
        log_e("remote verify sign failed !");
		ret = ADM_ECODE_VERIFY_SIGNATURE_FAILED;
	}
	else{
		log_e("uknown remote verify sign failed !");
		ret = ADM_ECODE_VERIFY_SIGNATURE_FAILED;
	}
	
	return ret;
}
#endif

/*!
 *******************************************************************************
 * IPM模块远程（已传输）升级包验签
 *
 * \note 
 *
 * \param[in]   head      pis包头信息
 * \param[in]   data      pis包信息
 * \param[out]   originimagePackage      原包信息
 * \retval      D32S
 *******************************************************************************
 */
static D32S adm_ipm_remote_signatureVerify(const ADM_PIS_PACKAGE_HEAD_T *head, const ADM_PIS_PACKAGE_DATA_T *data, DCHAR *installInstanceId, ADM_PIS_FILE_T *originimagePackage){
    ADM_COM_FILEOBJECT_T stFileObject = {0};
	ADM_SYSI4UCL_VERIFY_SIGN_RESULT_INFO_T stVerifySignResultInfo = {0};
	D32S i=0;
    D32S ret = ADM_ECODE_FAILED;
//	D32S errorCode = 0;

	DBOOL  bIsRollback = FALSE;
	DCHAR *pFileId = NULL;
//	D32S errorCode = 0;
	
	if (ADM_IS_NULL(originimagePackage)||ADM_IS_NULL(data)||ADM_IS_NULL(installInstanceId))
	{
        ret = ADM_ECODE_INVALID_PARAM;
        log_e("Invalid input param");
        return ret;
    }

	stFileObject.fileName = NULL;
	stFileObject.url = NULL;
	stFileObject.hash = data->imagePackage.hash;
	stFileObject.filePath = data->imagePackage.path;
	stFileObject.size = data->imagePackage.size;
	stFileObject.pstFileSecurityInfo = data->fileSecurityInfo;

	stVerifySignResultInfo.signature = NULL;
	stVerifySignResultInfo.filePath = NULL;

    do{
		//!< 验签的条件是否满足 
	    if (FALSE == adm_com_checkIsNeedToVerifySignature(&stFileObject))
	    {
			originimagePackage->size = data->imagePackage.size;
			originimagePackage->hash = adm_strdup(data->imagePackage.hash);
			originimagePackage->path = adm_strdup(data->imagePackage.path);
			ret = ADM_ECODE_SUCCESSED;
	        log_d("dont need to verify signature");
	        break;
	    }

		if(ADM_PIS_PACKAGE_TYPE_ECU_SRC_PACKAGE == head->type)
		{
	        bIsRollback = TRUE;
		}
		else
		{
	        bIsRollback = FALSE;
		}
	    pFileId = adm_dps_createDownFileIDByEcuInfo(head->ecuReqId, head->ecuSwDid, bIsRollback,data->imagePackage.path);
	    if (ADM_IS_NULL(pFileId))
		{
	        log_e("Failed to create verify sign file id, err[%d]", ADM_ECODE_NO_MEMORY);
	        ret = ADM_ECODE_NO_MEMORY;
		    break;
	    }

	    //<! ucl远程验签接口
	    ret = adm_pub_downPackageVerifySign(&stFileObject,installInstanceId,pFileId,TRUE, &stVerifySignResultInfo);
	    if (ADM_IS_ERR(ret))
	    {
	        log_e("ucl Verify signature failed, err[%d]", ret);
	        ret = ADM_ECODE_VERIFY_SIGNATURE_FAILED;	        
	        break; 
	    }
	    log_d("adm_pub_downPackageVerifySign ok");
		
        ret = ADM_ECODE_VERIFY_SIGNATURE_FAILED;
		if(0x01U == stVerifySignResultInfo.result)
		{		    
			for(i=0; i< data->fileSecurityInfo->fileMetaNum; i++)
			{
		        if (ADM_SIM_SOA_UCL_FILE_TYPE_ORI == (FILE_TYPE_E)data->fileSecurityInfo->pstFileMetaList[i].type)
		        {
		            originimagePackage->size = data->fileSecurityInfo->pstFileMetaList[i].size;
					originimagePackage->hash = adm_strdup(data->fileSecurityInfo->pstFileMetaList[i].stCheckSum.value);
					originimagePackage->path = adm_strdup(stVerifySignResultInfo.filePath);
					ret = ADM_ECODE_SUCCESSED;
					break;
		        }	        
			}
			log_d("remote verify install package signature result 01, filePath[%s], size[%lld], originhash[%s], originhashlen[%d]",
					ADM_STR_SAFE_OUTPUT(originimagePackage->path), originimagePackage->size, 
					ADM_STR_SAFE_OUTPUT(originimagePackage->hash), adm_strlen(originimagePackage->hash));
		}
		
		else if(0xffU == stVerifySignResultInfo.result)
		{
			for(i=0; i< data->fileSecurityInfo->fileMetaNum; i++)
			{
		        if (ADM_SIM_SOA_UCL_FILE_TYPE_ORI == (FILE_TYPE_E)data->fileSecurityInfo->pstFileMetaList[i].type)
		        {
		            log_d("remote verify install package signature , signData[%s], signlen[%d], originhash[%s], originhashlen[%d]",
						ADM_STR_SAFE_OUTPUT(stVerifySignResultInfo.signature), stVerifySignResultInfo.signatureLen, 
						ADM_STR_SAFE_OUTPUT(data->fileSecurityInfo->pstFileMetaList[i].stCheckSum.value), adm_strlen(data->fileSecurityInfo->pstFileMetaList[i].stCheckSum.value));
/*					
	                ret = adm_dim_host_verifySignature(adm_soa_getHostInstanceId(), ADM_IPM_SIGN_REQ_TIMEOUT, 
	                                stVerifySignResultInfo.signature, stVerifySignResultInfo.signatureLen, 
	                                data->fileSecurityInfo->pstFileMetaList[i].stCheckSum.value, adm_strlen(data->fileSecurityInfo->pstFileMetaList[i].stCheckSum.value), 
	                                &errorCode);
					if (ADM_IS_ERR(ret))
					{
			            log_e("adm_dim_host_verifySignature failed, err[%d]", ret);
					    break;	
			        }
*/					if (ADM_IS_NULL(stVerifySignResultInfo.filePath))
					{
			            log_e("stVerifySignResultInfo.filePath is NULL");
					    break;	
			        }
			        log_d(" verifySignature ok");
					originimagePackage->path = adm_strdup(stVerifySignResultInfo.filePath);
		            originimagePackage->size = data->fileSecurityInfo->pstFileMetaList[i].size;
					originimagePackage->hash = adm_strdup(data->fileSecurityInfo->pstFileMetaList[i].stCheckSum.value);
					ret = ADM_ECODE_SUCCESSED;
					break;
		        }
		        
			}		    
	    }
		else if((stVerifySignResultInfo.result >= 0x02U)&&(stVerifySignResultInfo.result <= 0xfeU))
		{
	        log_e("verify Signature failed, result[%d]", stVerifySignResultInfo.result);
		}
		else
		{
			log_e("uknown remote verify sign failed !");
		}
	}while (FALSE);
	
	adm_free(pFileId);
	adm_free(stVerifySignResultInfo.signature);
	adm_free(stVerifySignResultInfo.filePath);

    log_i("remote verify sign ret[%d]", ret);
	return ret;		
    
}

#if ((defined ADM_DEBUG_CVM_MODULE) || (defined ADM_DEBUG_IPM_MODULE)) 
D32S remote_request_Verify_Sign_test(void){
    D32S ret = ADM_ECODE_FAILED;
	log_i("dim request Verify Sign begin ****");
	ADM_PIS_PACKAGE_HEAD_T head;
	ADM_PIS_FILE_T originimagePackage = {0};

	head.ecuReqId = "0X700";
	head.ecuSwDid = "0XF189";
	head.state = ADM_PIS_PACKAGE_STATE_DECRYPT_FINISH;
	head.type = ADM_PIS_PACKAGE_TYPE_ECU_DST_PACKAGE;

	ADM_PIS_PACKAGE_DATA_T data;
	data.imagePackage.hash = "ef7ca14d88f6df616bf9c8112441919eb6477f5eced15abb14c7a06eef4c49f9";
	data.imagePackage.path = "/home/jerry/work/project/VUS/debug_/ipm_test/20220423_testsign/package_sign.zip";
	data.imagePackage.size = 27888963;

	data.fileSecurityInfo = (ADM_COM_FILESCYINFO_T *)adm_malloc(sizeof(ADM_COM_FILESCYINFO_T));
	if (ADM_IS_NULL(data.fileSecurityInfo))
        {
            log_e("Failed to new file meta list");
//            adm_com_delFileSecurityInfo(&pstDstFileSecurityInfo);
            return ADM_ECODE_FAILED;
        }
	
	data.fileSecurityInfo->oriHash = "e5acd986a3f68737f8d2d03dea898686e208576368849c7f7b8943eeb129c7d4";
	data.fileSecurityInfo->oriSize = 27888883;
	data.fileSecurityInfo->fileMetaNum = 2;

	data.fileSecurityInfo->pstFileMetaList = 
                adm_com_newFileMetaList(data.fileSecurityInfo->fileMetaNum);
        if (ADM_IS_NULL(data.fileSecurityInfo->pstFileMetaList))
        {
            log_e("Failed to new file meta list");
//            adm_com_delFileSecurityInfo(&pstDstFileSecurityInfo);
            
			adm_free(data.fileSecurityInfo);
            return ADM_ECODE_FAILED;
        }

	data.fileSecurityInfo->pstFileMetaList[0].stCheckSum.value = "e5acd986a3f68737f8d2d03dea898686e208576368849c7f7b8943eeb129c7d4";	
	data.fileSecurityInfo->pstFileMetaList[0].stCheckSum.algorithm = ADM_DIM_SOA_UCL_ALGORITHM_TYPE_SHA256;	
	data.fileSecurityInfo->pstFileMetaList[0].type = ADM_SIM_SOA_UCL_FILE_TYPE_ORI;	
	data.fileSecurityInfo->pstFileMetaList[0].size = 27888883;	

	data.fileSecurityInfo->pstFileMetaList[1].stCheckSum.value = "a161d8cdfacb9a651e8927948fcbade32de1de10fc94d951e91b1e017c4047d8";	
	data.fileSecurityInfo->pstFileMetaList[1].stCheckSum.algorithm = ADM_DIM_SOA_UCL_ALGORITHM_TYPE_SHA256;	
	data.fileSecurityInfo->pstFileMetaList[1].type = ADM_SIM_SOA_UCL_FILE_TYPE_SIGN;	
	data.fileSecurityInfo->pstFileMetaList[1].size = 80;	

	ret = adm_ipm_remote_signatureVerify(&head, &data, "0x2001", &originimagePackage);

    adm_free(data.fileSecurityInfo->pstFileMetaList);
	adm_free(data.fileSecurityInfo);

    
    log_i("dim request Verify Sign end ****");
}
#endif

static D32S ipmVerifySignPackageOnce(const ADM_PIS_PACKAGE_HEAD_T *head, const ADM_PIS_PACKAGE_DATA_T *data/*ADM_PIS_FILE_T *imagePackage*/){
    D32S ret = ADM_ECODE_FAILED;
	
	ADM_PIS_FILE_T originimagePackage = {0};
	
    /* TODO verify package */
    ADM_ECS_ECUCONFIG_T *pecsEcu = NULL;
//	D32S uimInstanceId = adm_cfg_iniParserGetInt(ADM_CFG_VUS_UIM_INSTANCEID,  ADM_CFG_VUS_UIM_INSTANCEID_DEFAULT_VAL);


    ret = adm_ecs_getEcuConfig(head->ecuReqId, head->ecuSwDid,  &pecsEcu);
    if(ADM_IS_ERR(ret)){
		log_e("Failed to getEcuConfig");
		return ret;	
    }

	// if((D32S)htoi(pecsEcu->installInstanceId) != uimInstanceId){
	if(pecsEcu->serviceType == 2/*UCL*/ || pecsEcu->serviceType == 1 /**VDI*/)
	{
		log_i("the package is Remote ");
		
/*
//temp add for debug,20220416     
		originimagePackage.path = adm_strdup(data->imagePackage.path);
		originimagePackage.hash = adm_strdup(data->imagePackage.hash);
		originimagePackage.size = data->imagePackage.size;
        {
	    log_i("%s:%s, %s:%s, %s:%d, %s:%s", 
		    ADM_STR_FROM_PARAM(originimagePackage.hash), ADM_STR_SAFE_OUTPUT(originimagePackage.hash),
		    ADM_STR_FROM_PARAM(originimagePackage.path), ADM_STR_SAFE_OUTPUT(originimagePackage.path),
		    ADM_STR_FROM_PARAM(originimagePackage.size), originimagePackage.size);
		}
//temp add end
*/
		ret = adm_ipm_remote_signatureVerify(head, data,pecsEcu->transferInstanceId, &originimagePackage);
        adm_ecs_delEcuConfigInfo(&pecsEcu);
    }
	else{
		log_i("the package is in local ");
		adm_ecs_delEcuConfigInfo(&pecsEcu);
		ret = adm_ipm_local_signatureVerify(data, &originimagePackage);
    }

	if(ADM_IS_ERR(ret)){
        return ret;	
    }

    ret = ADM_ECODE_SUCCESSED;
    ADM_PIS_PACKAGE_HEAD_T myhead = {
        .ecuReqId = head->ecuReqId,
        .ecuSwDid = head->ecuSwDid,
        .type = head->type,
        .state = ADM_PIS_PACKAGE_STATE_VERIFY_SIGN_FINISH
    };
    adm_pis_packageImageSet(&myhead, &originimagePackage);
    //流式拼接需要添加升级包的拼接信息
    if (NULL != data->fileSecurityInfo && data->makePackageType==FILE_MAKE_PKG_TYPE)
    {
        adm_pis_packageSecuritySet(&myhead, data->fileSecurityInfo);
        adm_pis_packageMakeTypeSet(&myhead, data->makePackageType);

    }

	adm_free(originimagePackage.hash);
	adm_free(originimagePackage.path);
    return ret;
}
static D32S ipmStepVerifySignPackage(void){
    D32S ret = ADM_ECODE_FAILED;
    ADM_IPS_PROGRESS_T progress;
    adm_ips_progressInit(&progress);
    adm_ips_progressGet(&progress);

    for(D32S i=0; i<progress.ecus.ecusLen; ++i){
        ADM_IPS_ECU_PROGRESS_T* ecu = &progress.ecus.ecus[i];
        ADM_PIS_PACKAGE_HEAD_T head = {
                .ecuReqId = ecu->ecuReqId,
                .ecuSwDid = ecu->ecuSwDid ,
                .state = ADM_PIS_PACKAGE_STATE_DECRYPT_FINISH,
        };

        D64U uiStartTime = (D64U)time(NULL);
        ADM_PIS_PACKAGE_DATA_T data;
        adm_pis_packageDataInit(&data);
        head.type = ADM_PIS_PACKAGE_TYPE_ECU_SRC_PACKAGE;
        if(NULL != adm_pis_packageGet(&head, &data)){
	        ret = ipmVerifySignPackageOnce(&head, &data);
	        adm_pis_packageDataDeInit(&data);
	        if (ADM_ECODE_SUCCESSED != ret) {
	            ipmReportEventDependencyEventId("IP_010", NULL, ret);
	            break;
	        }
	        else {
	            ipmReportEventDependencyEventId("IP_009", NULL, ADM_ECODE_SUCCESSED);
	        }
        }
		else{
            log_d("no src package ,no need verify src package  ");
		}

        adm_pis_packageDataInit(&data);
        head.type = ADM_PIS_PACKAGE_TYPE_ECU_DST_PACKAGE;
        if(NULL == adm_pis_packageGet(&head, &data)){
            ret = ADM_ECODE_FAILED;
			break;
        }
        ret = ipmVerifySignPackageOnce(&head, &data);
        adm_pis_packageDataDeInit(&data);
        if (ADM_ECODE_SUCCESSED != ret) {
            ipmReportEventDependencyEventId("IP_010", NULL, ret);
            break;
        }
        else {
            ipmReportEventDependencyEventId("IP_009", NULL, ADM_ECODE_SUCCESSED);
        }


        ADM_ECS_ECUCONFIG_T* pecsEcu = NULL;
        ret = adm_ecs_getEcuConfig(head.ecuReqId, head.ecuSwDid, &pecsEcu);
        if (ADM_ECODE_SUCCESSED != adm_ecs_getEcuConfig(head.ecuReqId, head.ecuSwDid, &pecsEcu)) {
            continue;
        }
        D32S iServiceType = pecsEcu->serviceType;
        IPM_STATISTICS_EVENT_E eventId = ((adm_strcmp(pecsEcu->transferInstanceId, pecsEcu->installInstanceId) != 0)
            ? STAT_EVENT_REMOTE_VERIFY_SIGN : STAT_EVENT_LOCAL_VERIFY_SIGN);
        adm_ecs_delEcuConfigInfo(&pecsEcu);

        IPM_STAT_EVENT_T statEvent = { 0 };
        statEvent.eventId = (D32S)eventId;
        statEvent.ecuReqId = head.ecuReqId;
        statEvent.ecuSwDid = head.ecuSwDid;
        statEvent.serviceType = iServiceType;
        statEvent.fileSize = data.imagePackage.size;
        statEvent.startTime = uiStartTime;
        statEvent.endTime = (D64U)time(NULL);
        statEvent.progress = 100;
        ipmStatEventReportReport(&m_ipmContext.report.reportStatEvent, &statEvent);
    }
    adm_ips_progressDeInit(&progress);

    ADM_ECODE_APPLICATION_CONVERSION(ret, ADM_ECODE_INSTALL_SIGN_VERIFY_FAILED);
    return ret;
}

static D32S ipmStepVerifyEcusVersion(ADM_IPM_FLASH_T *f)
{
    D32S nRet = ADM_ECODE_SUCCESSED;
    D32S iErrorCode = ADM_ECODE_FAILED;
    D32S iTboxRemoteDiaStatus = -1;
    D32U uiCnt = 0;
    ADM_CVS_UPGRADE_MODE_E eUpgradeMode = ADM_CVS_UPGRADE_MODE_INVALID;

    if(f == NULL)
    {
        log_e("invalid param f[ %p ]", f);
        return ADM_ECODE_INVALID_PARAM;
    }

    nRet = adm_cvs_getUpgradeMode((D32S *)&eUpgradeMode);
    if(nRet == ADM_ECODE_SUCCESSED)
    {
        /** 强刷、救援模式、U盘升级，升级前不检验版本，后续有其他模式，自行补充 */
        if (ADM_CVS_UPGRADE_MODE_FORCE != eUpgradeMode 
            && ADM_CVS_UPGRADE_MODE_RESCUE != eUpgradeMode
            && ADM_CVS_UPGRADE_MODE_USB_CHECK != eUpgradeMode)
        {
            /** 超时5min读取tbox是否完成远程诊断  */
            do
            {
                (DVOID)adm_dim_host_getTboxRemoteDiaStatus(adm_soa_getHostInstanceId(), 10, &iTboxRemoteDiaStatus, &iErrorCode);
                if (REMOTE_DIA_END_READ == iTboxRemoteDiaStatus)
                {
                    log_i("tbox remote dia end read.");
                    break;
                }
                uiCnt++;
                sleep(5);
            } while (uiCnt < 60);

            nRet = ipmDoVerifyEcusVersionBeforeInstall(&(f->doFlash));
            if(nRet != ADM_ECODE_SUCCESSED)
            {
                log_e("Failed to verify Ecus Version, nRet[ %d ]", nRet);
            }
        }
    }
    else 
    {
        log_e("Get upgradeMode error.nRet[%d]", nRet);
    }
	
    ADM_ECODE_APPLICATION_CONVERSION(nRet, ADM_ECODE_INSTALL_VERSION_VERIFY_FAILED);
    return nRet;
}

static const DCHAR *ipmFlashStageString(ADM_IPM_FLASH_STAGE_E eStage)
{
    const DCHAR *strStage = "ADM_IPM_FLASH_STAGE_UNKNOWN";

    switch (eStage)
    {
        case IPM_FLASH_STAGE_DEFAULT:
            strStage = ADM_STR_FROM_PARAM(IPM_FLASH_STAGE_DEFAULT);
            break;

        case IPM_FLASH_STAGE_RETRY:
            strStage = ADM_STR_FROM_PARAM(IPM_FLASH_STAGE_RETRY);
            break;

        case IPM_FLASH_STAGE_ROLLBACK:
            strStage = ADM_STR_FROM_PARAM(IPM_FLASH_STAGE_ROLLBACK);
            break;

        case IPM_FLASH_STAGE_ROLLBACK_RETRY:
            strStage = ADM_STR_FROM_PARAM(IPM_FLASH_STAGE_ROLLBACK_RETRY);
            break;

        default:
            break;
    }

    return strStage;
}

static const DCHAR *ipmActiveStageString(ADM_ACTIVE_STATE_E eAcitveStage)
{
    const DCHAR *strStage = "ADM_IPM_FLASH_ACTIVE_UNKNOWN";

    switch (eAcitveStage)
    {
    case ADM_IPS_ACTIVE_PRE:
        strStage = ADM_STR_FROM_PARAM(ADM_IPS_ACTIVE_PRE);
        break;

    case ADM_IPS_ACTIVE_ING:
        strStage = ADM_STR_FROM_PARAM(ADM_IPS_ACTIVE_ING);
        break;

    case ADM_IPS_ACTIVE_END:
        strStage = ADM_STR_FROM_PARAM(ADM_IPS_ACTIVE_END);
        break;

    default:
        break;
    }

    return strStage;
}


/**
 * @brief 升级状态检查
 * @note 检查是否存在指定状态的ECU
 * @param in ADM_IPS_ECU_PHASE_E eFlashState          升级状态
 * @retval DBOOL TRUE存在，FALSE不存在
 */
static DBOOL ipmDoFlashEcuStateCheck(ADM_IPS_ECU_PHASE_E eFlashState)
{
    ADM_IPS_PROGRESS_T stProgressInfo = {0};
    ADM_IPS_ECU_PROGRESS_T *pstEcuProgressInfo = NULL;
    D32S i = 0;
    DBOOL bIsExist = FALSE;

    ADM_COM_FUNC_ENTRY();

    adm_ips_progressInit(&stProgressInfo);
    adm_ips_progressGet(&stProgressInfo);

    for (i = 0; i < stProgressInfo.ecus.ecusLen; i++)
    {
        pstEcuProgressInfo = &stProgressInfo.ecus.ecus[i];
        log_d("ecu[ %s ] state[ %d ]", pstEcuProgressInfo->ecuReqId, pstEcuProgressInfo->state);
        if (eFlashState == pstEcuProgressInfo->state)
        {
            bIsExist = TRUE;
            break;
        }
    }

    adm_ips_progressDeInit(&stProgressInfo);

    ADM_COM_FUNC_EXIT();

    return bIsExist;
}

/**
 * @brief 回滚结果检查
 * @note
 * @param in void
 * @retval D32S 0 成功，其他失败
 */
static D32S ipmDoFlashEcuRollbackResultCheck(void)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    ADM_IPS_PROGRESS_T stProgressInfo = {0};
    ADM_IPS_ECU_PROGRESS_T *pstEcuProgressInfo = NULL;
    D32S i = 0;

    ADM_COM_FUNC_ENTRY();

    adm_ips_progressInit(&stProgressInfo);
    adm_ips_progressGet(&stProgressInfo);

    for (i = 0; i < stProgressInfo.ecus.ecusLen; i++)
    {
        pstEcuProgressInfo = &stProgressInfo.ecus.ecus[i];

        /** 存在刷写失败的ECU */
        if (ADM_IPS_ECU_PHASE_POST_INSTALL_ROLLBACK == pstEcuProgressInfo->state ||
            ADM_IPS_ECU_PHASE_INSTALL_ROLLBACK_RETRY == pstEcuProgressInfo->state)
        {
            if (ADM_ECODE_SUCCESSED != pstEcuProgressInfo->progress[ADM_IPS_ECU_PHASE_INDEX_ROLLBACK].result)
            {
                log_d("ecu %s %s rollback result = %d", pstEcuProgressInfo->ecuReqId, 
                                                        pstEcuProgressInfo->ecuSwDid, 
                                                        pstEcuProgressInfo->progress[ADM_IPS_ECU_PHASE_INDEX_ROLLBACK].result);
                iRet = ADM_ECODE_INSTALL_ROLLBACK_FAILED;
                break;
            }
        }
    }

    adm_ips_progressDeInit(&stProgressInfo);

    ADM_COM_FUNC_EXIT();

    return iRet;
}

/**
 * @brief 执行刷写ECU
 * @note
 * @param in ADM_IPM_FLASH_T *pstFlashInfo  刷写信息结构
 * @retval D32S 0 成功，其他失败
 */
static D32S ipmDoFlashEcusExec(ADM_IPM_FLASH_T *pstFlashInfo)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    D32S iPostRet = ADM_ECODE_SUCCESSED;

    ADM_COM_FUNC_ENTRY();

    /** 参数校验 */
    if (ADM_IS_NULL(pstFlashInfo))
    {
        log_e("invalid param");
        return ADM_ECODE_INVALID_PARAM;
    }

    /** 执行主编程 */
    iRet = ipmDoFlashEcus(&pstFlashInfo->doFlash);
    if (TRUE == adm_cfg_iniParserGetBoolean(ADM_CFG_VUS_IS_DETACH_RETRY, 
                                            ADM_CFG_VUS_IS_DETACH_RETRY_DEFAULT_VAL))
    {
        /** 回滚或回滚重试阶段，需要重新校验回滚结果 */
        if (IPM_FLASH_STAGE_ROLLBACK == pstFlashInfo->eStage ||
            IPM_FLASH_STAGE_ROLLBACK_RETRY == pstFlashInfo->eStage)
        {
            iRet = ipmDoFlashEcuRollbackResultCheck();
        }
    }

    log_d("main program result = %d", iRet);

    ADM_COM_FUNC_EXIT();

    return iRet ;
}

/**
 * @brief 默认刷写阶段执行
 * @note
 * @param in ADM_IPM_FLASH_T *pstFlashInfo  刷写信息结构
 * @retval D32S 0 成功，其他失败
 */
static D32S ipmDoFlashEcusDefault(ADM_IPM_FLASH_T *pstFlashInfo)
{
    D32S iRet = ADM_ECODE_SUCCESSED;

    ADM_COM_FUNC_ENTRY();

    /** 参数校验 */
    if (ADM_IS_NULL(pstFlashInfo))
    {
        log_e("invalid param");
        return ADM_ECODE_INVALID_PARAM;
    }

    /** 执行刷写 */
    iRet = ipmDoFlashEcusExec(pstFlashInfo);

    /** 升级失败，并且不是预编程失败 */
    if (ADM_IS_ERR(iRet) &&
        ADM_ECODE_INSTALL_PRE_PROGRAM_FAILED != iRet &&
        ADM_ECODE_INSTALL_ENTER_HIGH_VOLTAGE_FAILED != iRet &&
        ADM_ECODE_INSTALL_EXIT_HIGH_VOLTAGE_FAILED != iRet)
    {
        log_e("Failed to upgrade ecus, iRet[ %d ]", iRet);
        
        /** 如果存在待重试，则进入重试流程，否则直接进入回滚流程 */
        if (TRUE == ipmDoFlashEcuStateCheck(ADM_IPS_ECU_PHASE_INSTALL_RETRY))
        {
            pstFlashInfo->eStage = IPM_FLASH_STAGE_RETRY;
        }
        else
        {
            adm_ips_ecuSetActionForFailed();

            if (TRUE == ipmDoFlashEcuStateCheck(ADM_IPS_ECU_PHASE_PRE_INSTALL_ROLLBACK))
            {
                /** 重试失败，不需要重试，进入回滚流程 */
                pstFlashInfo->eStage = IPM_FLASH_STAGE_ROLLBACK;
            }
            else
            {
                log_d("exit upgrade");
                /** 退出刷写流程 */
                pstFlashInfo->eStage = IPM_FLASH_STAGE_EXIT;
            }
        }
    }
    else
    {
        log_d("exist upgrade");
        /** 升级成功或者不需要重试，退出刷写流程 */
        pstFlashInfo->eStage = IPM_FLASH_STAGE_EXIT;
    }

    ADM_COM_FUNC_EXIT();

    return iRet;
}

/**
 * @brief 重试刷写阶段执行
 * @note
 * @param in ADM_IPM_FLASH_T *pstFlashInfo  刷写信息结构
 * @retval D32S 0 成功，其他失败
 */
static D32S ipmDoFlashEcusRetry(ADM_IPM_FLASH_T *pstFlashInfo)
{
    D32S iRet = ADM_ECODE_SUCCESSED;

    ADM_COM_FUNC_ENTRY();

    /** 参数校验 */
    if (ADM_IS_NULL(pstFlashInfo))
    {
        log_e("invalid param");
        return ADM_ECODE_INVALID_PARAM;
    }

    /** 将状态更新为安装中 */
    adm_ips_updateEcuFlashState(ADM_IPS_ECU_PHASE_INSTALL_RETRY, ADM_IPS_ECU_PHASE_INSTALL);

    /** 重新创建升级组 */
    adm_ips_reCreateUpdateGroup();

    /** 执行重试刷写 */
    iRet = ipmDoFlashEcusExec(pstFlashInfo);
    if (ADM_IS_ERR(iRet) && 
        ADM_ECODE_INSTALL_PRE_PROGRAM_FAILED != iRet &&
        ADM_ECODE_INSTALL_ENTER_HIGH_VOLTAGE_FAILED != iRet &&
        ADM_ECODE_INSTALL_EXIT_HIGH_VOLTAGE_FAILED != iRet)
    {
        log_e("Failed to retry upgrade ecus, iRet[ %d ]", iRet);

        /** 重试失败，需要重试，继续进入重试流程 */
        if (TRUE == ipmDoFlashEcuStateCheck(ADM_IPS_ECU_PHASE_INSTALL_RETRY))
        {
            pstFlashInfo->eStage = IPM_FLASH_STAGE_RETRY;
        }
        else
        {
            adm_ips_ecuSetActionForFailed();

            if (TRUE == ipmDoFlashEcuStateCheck(ADM_IPS_ECU_PHASE_PRE_INSTALL_ROLLBACK))
            {
                /** 重试失败，不需要重试，进入回滚流程 */
                pstFlashInfo->eStage = IPM_FLASH_STAGE_ROLLBACK;
            }
            else
            {
                log_d("exit retry upgrade");
                /** 退出刷写流程 */
                pstFlashInfo->eStage = IPM_FLASH_STAGE_EXIT;
            }
        }
    }
    else
    {
        log_d("exit retry upgrade");
        /** 退出刷写流程 */
        pstFlashInfo->eStage = IPM_FLASH_STAGE_EXIT;
    }

    ADM_COM_FUNC_EXIT();

    return iRet;
}

/**
 * @brief 回滚刷写阶段执行
 * @note
 * @param in ADM_IPM_FLASH_T *pstFlashInfo  刷写信息结构
 * @retval D32S 0 成功，其他失败
 */
static D32S ipmDoFlashEcusRollback(ADM_IPM_FLASH_T *pstFlashInfo)
{
    D32S iRet = ADM_ECODE_SUCCESSED;

    ADM_COM_FUNC_ENTRY();
    
    /** 参数校验 */
    if (ADM_IS_NULL(pstFlashInfo))
    {
        log_e("invalid param");
        return ADM_ECODE_INVALID_PARAM;
    }

    /** 重新创建升级组 */
    adm_ips_reCreateUpdateGroup();

    /** 执行回滚刷写 */
    iRet = ipmDoFlashEcusExec(pstFlashInfo);
    if (ADM_IS_SUCCESS(iRet))
    {
        iRet = ADM_ECODE_INSTALL_ROLLBACK_SUCCESSED;
        log_d("rollback success, exit rollback");
        /** 退出刷写流程 */
        pstFlashInfo->eStage = IPM_FLASH_STAGE_EXIT;
    }
    else if (ADM_ECODE_INSTALL_PRE_PROGRAM_FAILED == iRet ||
             ADM_ECODE_INSTALL_ENTER_HIGH_VOLTAGE_FAILED == iRet ||
             ADM_ECODE_INSTALL_EXIT_HIGH_VOLTAGE_FAILED == iRet)
    {
        log_d("pre program failed, exit rollback");
        /** 退出刷写流程 */
        pstFlashInfo->eStage = IPM_FLASH_STAGE_EXIT;
    }
    else
    {
        log_e("Failed to rollback ecus, iRet[ %d ]", iRet);

        /** 回滚失败，存在需要重试回滚的ECU，进入回滚重试流程 */
        if (TRUE == ipmDoFlashEcuStateCheck(ADM_IPS_ECU_PHASE_INSTALL_ROLLBACK_RETRY))
        {
            pstFlashInfo->eStage = IPM_FLASH_STAGE_ROLLBACK_RETRY;
        }
        else
        {
            log_d("rollback failed, exit rollback");
            /** 退出刷写流程 */
            pstFlashInfo->eStage = IPM_FLASH_STAGE_EXIT;
        }
    }

    ADM_COM_FUNC_EXIT();

    return iRet;
}

/**
 * @brief 回滚重试刷写阶段执行
 * @note
 * @param in ADM_IPM_FLASH_T *pstFlashInfo  刷写信息结构
 * @retval D32S 0 成功，其他失败
 */
static D32S ipmDoFlashEcusRollbackRetry(ADM_IPM_FLASH_T *pstFlashInfo)
{
    D32S iRet = ADM_ECODE_SUCCESSED;

    ADM_COM_FUNC_ENTRY();

    /** 参数校验 */
    if (ADM_IS_NULL(pstFlashInfo))
    {
        log_e("invalid param");
        return ADM_ECODE_INVALID_PARAM;
    }

    /** 将状态更新为回滚中 */
    adm_ips_updateEcuFlashState(ADM_IPS_ECU_PHASE_INSTALL_ROLLBACK_RETRY, ADM_IPS_ECU_PHASE_INSTALL_ROLLBACK);

    /** 重新创建升级组 */
    adm_ips_reCreateUpdateGroup();

    /** 执行回滚重试刷写 */
    iRet = ipmDoFlashEcusExec(pstFlashInfo);
    if (ADM_IS_SUCCESS(iRet))
    {
        iRet = ADM_ECODE_INSTALL_ROLLBACK_SUCCESSED;
        log_d("rollback retry success, exit rollback");
        /** 退出刷写流程 */
        pstFlashInfo->eStage = IPM_FLASH_STAGE_EXIT;
    }
    else if (ADM_ECODE_INSTALL_PRE_PROGRAM_FAILED == iRet ||
             ADM_ECODE_INSTALL_ENTER_HIGH_VOLTAGE_FAILED == iRet ||
             ADM_ECODE_INSTALL_EXIT_HIGH_VOLTAGE_FAILED == iRet)
    {
        log_d("pre program failed, exit rollback");
        /** 退出刷写流程 */
        pstFlashInfo->eStage = IPM_FLASH_STAGE_EXIT;
    }
    else
    {
        log_e("Failed to retry rollback ecus, iRet[ %d ]", iRet);

        /** 回滚失败，存在需要重试回滚的ECU，进入回滚重试流程 */
        if (TRUE == ipmDoFlashEcuStateCheck(ADM_IPS_ECU_PHASE_INSTALL_ROLLBACK_RETRY))
        {
            pstFlashInfo->eStage = IPM_FLASH_STAGE_ROLLBACK_RETRY;
        }
        else
        {
            log_d("retry rollback failed, exit rollback");
            /** 退出刷写流程 */
            pstFlashInfo->eStage = IPM_FLASH_STAGE_EXIT;
        }
    }

    ADM_COM_FUNC_EXIT();

    return iRet;
}

static D32S ipmStepApkUpgrade(ADM_IPM_FLASH_T* f)
{
    D32S iRet = ADM_ECODE_FAILED;

    iRet = ipmDoFlashEcus(&f->doFlash);
    log_d("ipmDoFlashEcus result = %d", iRet);

    return iRet;
}

static DBOOL ipmMcuUpgradeResultQuery()
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    D32S errorCode = ADM_ECODE_SUCCESSED;
    DBOOL bIsRet = DFALSE;
    ADM_DIM_SOA_ECU_IDEN_INFO_T ecuIdenInfo = {};

    ecuIdenInfo.requestId = 0;
    ecuIdenInfo.responseId = 0;
    ecuIdenInfo.swDid = 0;
    ecuIdenInfo.requestId = ipmHtoi(ICC_REQ_ID);
    ecuIdenInfo.swDid = ipmHtoi(ICC_SW_DID);

    ADM_DIM_SOA_UCL_UPGRADE_RESULT_INFO_T* pstUpgradeResultInfo = NULL;
    iRet = adm_dim_ucl_queryUpgradeResult((D16U)adm_soa_getUclInstanceId(), 60, &ecuIdenInfo, &pstUpgradeResultInfo, &errorCode);
    if (NULL != pstUpgradeResultInfo)
    {
        if (ADM_ECODE_SUCCESSED == iRet && ADM_ECODE_SUCCESSED == errorCode) 
        {
            if (UPGRADE_SUCCEED == pstUpgradeResultInfo->result || UPGRADE_FAILED == pstUpgradeResultInfo->result)
            {
                log_i("result[%d]", pstUpgradeResultInfo->result);

                ADM_ECS_ECU_ID_T stEcsEcuId = {0};
                adm_ecs_getEcuConfigEcuId(ICC_REQ_ID,ICC_SW_DID,&stEcsEcuId);

                ADM_IPS_ECU_CURRENT_PROGRESS_T curProgress = {};
                curProgress.ecuReqId = adm_strdup(stEcsEcuId.pEcuReqId);
                curProgress.ecuSwDid = adm_strdup(stEcsEcuId.pEcuSwDid);
                curProgress.isEnd = TRUE;
                curProgress.result = pstUpgradeResultInfo->result;
                adm_ips_currentProgressSet(&curProgress);

                adm_free(curProgress.ecuReqId);
                adm_free(curProgress.ecuSwDid);
                adm_free(stEcsEcuId.pEcuReqId);
                adm_free(stEcsEcuId.pEcuSwDid);

                bIsRet = DTRUE;
            }
        }
        dimSoaUclUpgradeResultInfoDel(pstUpgradeResultInfo);
    }
    
    return bIsRet;
}

static D32S ipmMcuUpgradeResultDeal()
{
    D32S iRet = ADM_ECODE_FAILED;
    D32S iiRet = ADM_ECODE_FAILED;
    D32S errorCode = ADM_ECODE_SUCCESSED;
    D32S iCnt = 0;

    log_i("ipmMcuUpgradeResultDeal enter");

    do
    {
        // if (FALSE == ipmIccInstallTriggerInfoIsExist())
        if (0 == adm_strcmp(ICC_MCU_INSTALL_END, ipmIccInstallTriggerInfoGet()))
        {
            iRet = ADM_ECODE_SUCCESSED;
            log_d("recv MCU install result");
            break;
        }
        else
        {
            sleep(1);
            iCnt++;
        }
    } while (iCnt < 300);

    /**MCU升级结束之后执行后编程*/
    (void)ipmDoPostProgramEcus(g_uiLastInstallGroup);

    log_i("ipmMcuUpgradeResultDeal exit");

    return iRet;
}

static D32S ipmStepUpgradeMCU(ADM_IPM_FLASH_T* f)
{
    D32S iRet = ADM_ECODE_FAILED;
    D32S errorCode = ADM_ECODE_SUCCESSED;
    D32S iCnt = 0;

    iRet = ipmDoFlashEcus(&f->doFlash);
    log_d("ipmDoFlashEcus result = %d", iRet);

    do
    {
        /**触发MCU升级后，等待重启*/
        sleep(1);
        iCnt++;
    } while (iCnt < 300);

    return iRet;
}
static D32S ipmStepUpgrade(ADM_IPM_FLASH_T *f)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    DCHAR* pTriggerInfo = NULL;
    DCHAR* pTriggerInfoMcu = NULL;

    ADM_COM_FUNC_ENTRY();

    pTriggerInfo = ipmIccInstallTriggerInfoGet();
    if ((0 == adm_strcmp(pTriggerInfo,ICC_MCU_INSTALL_START)) || (0 == adm_strcmp(pTriggerInfo,ICC_MCU_INSTALL_END)))
    {   
        /**1、当TriggerInfo = ICC_MCU_INSTALL_START 时，代表MCU的升级结果仍未到达*/
        /**2、MCU升级已触发 查询升级结果*/
        iRet = ipmMcuUpgradeResultDeal();
    }
    else
    {
        do
        {
            log_d("eStage:%s", ipmFlashStageString(f->eStage));
            log_i("[ABUP-Procedure]: Install Entry Upgrade State:%s", ipmFlashStageString(f->eStage));
            switch (f->eStage)
            {
                case IPM_FLASH_STAGE_DEFAULT:
                    iRet = ipmDoFlashEcusDefault(f);
                    break;
                
                case IPM_FLASH_STAGE_RETRY:
                    iRet = ipmDoFlashEcusRetry(f);
                    break;

                case IPM_FLASH_STAGE_ROLLBACK:
                    iRet = ipmDoFlashEcusRollback(f);
                    break;
                
                case IPM_FLASH_STAGE_ROLLBACK_RETRY:
                    iRet = ipmDoFlashEcusRollbackRetry(f);
                    break;


                default:
                    log_w("invalid stage = %d", f->eStage);
                    iRet = ADM_ECODE_INVALID_PARAM;
                    break;
            }

            /** 更新缓存信息 */
            ipmFlashStore(f);
            log_i("[ABUP-Procedure]: Install Goto Upgrade State:%s", ipmFlashStageString(f->eStage));
        } while (IPM_FLASH_STAGE_EXIT != f->eStage);

        /** 执行MCU刷写 */
#ifdef ADM_ACTIVATION_SUPPORT
        pTriggerInfoMcu = ipmIccInstallTriggerInfoGet();
        if (0 == adm_strcmp(pTriggerInfoMcu,ICC_SOC_INSTALL_END))
        {
            /**SOC升级触发结束,需要触发MCU升级*/
            ipmIccInstallTriggerInfoSet(ICC_MCU_INSTALL_START);
            iRet = ipmStepUpgradeMCU(f);
        }
#endif
    }

    adm_free(pTriggerInfo);
    adm_free(pTriggerInfoMcu);

    ADM_COM_FUNC_EXIT();

    return iRet;
}

static DVOID ipmIntelEcuActive(ADM_IPM_DO_FLASH_T *f)
{
    const ADM_IPS_UCL_ECU_ARRAY_ACTIVE_T *pActiveInfo = NULL;
    ADM_ACTIVE_STATE_E eAcitveStage = ADM_IPS_ACTIVE_PRE;

    pActiveInfo = adm_ips_getActiveEcuInfo();
    if (NULL == pActiveInfo || NULL == pActiveInfo->pstActiveEcuInfo || 0 == pActiveInfo->uiEcuLen)
    {
        log_w("Get no active info.");
        return;
    }

    do
    {
        eAcitveStage = adm_ips_getActiveStage();
        log_d("[ABUP-Procedure]: Install Active State:%s", ipmActiveStageString(eAcitveStage));
        switch (eAcitveStage)
        {
        case ADM_IPS_ACTIVE_PRE:
            ipmHandleActivePre(f, pActiveInfo);
            break;

        case ADM_IPS_ACTIVE_ING:
            ipmHandleActiveIng(pActiveInfo);
            break;

        case ADM_IPS_ACTIVE_END:
            ipmHandleActiveEnd(pActiveInfo);
            break;

        default:
            log_w("invalid stage = %d", (D32S)eAcitveStage);
            eAcitveStage = ADM_IPS_ACTIVE_END;
            break;
        }
    } while (ADM_IPS_ACTIVE_END != eAcitveStage);

    return;
}

static D32S ipmStepEnvExit(ADM_IPM_FLASH_T *f){
    return ipmDimEnvExit(f->dim);
}
static D32S ipmStepReport(ADM_IPM_FLASH_T *f){
    return ipmReportResult(f->report, f->result);
}

/** 安装,阶段结果通知,操作退出OTA和下电后再通知总安装结果 */
static D32S ipmReportHmiStageResult(ADM_IPM_REPORT_HMI_T *h, D32S result)
{
    ADM_IPS_PROGRESS_T progress = {0};
    ADM_UIM_SOA_INSTALL_RESULT_INFO_T stEcuInstallResult = {0};
    DCHAR* sessionId = NULL;
    D32S respCode = ADM_ECODE_SUCCESSED;
    D32S ret = ADM_ECODE_SUCCESSED;

    adm_ips_progressInit(&progress);
    adm_ips_progressGet(&progress);
    
    uimSoaInstallResultInfoInitVehicle(&stEcuInstallResult, result);
    sessionId = uimSoaVehicleSessionId();
	
    if(adm_vsm_getHmiOnline())
    {
        (DVOID)adm_uim_hmi_stage_installResult_RR(uimSoaCmdidTypeGet(), sessionId, uimSoaVehicleState(&progress), &stEcuInstallResult, &respCode);
        if(ADM_ECODE_SUCCESSED != respCode)
	    {
	        ret = ADM_ECODE_FAILED;
    	}
	}
    adm_free(sessionId);
    uimSoaInstallResultInfoDeInit(&stEcuInstallResult);

    adm_ips_progressDeInit(&progress);
    return ret;
}

static D32S ipmUpdateVehicleBigVersion()
{
    D32S iRet = ADM_ECODE_FAILED;
    DCHAR* pVehicleBigVersion = DNULL;
    D32S iErrCode = ADM_ECODE_FAILED;

    adm_cvs_getVehicleBigVersion(&pVehicleBigVersion);
    if (ADM_IS_NULL(pVehicleBigVersion))
    {
        log_e("adm_cvs_getVehicleBigVersion failed, do not need to update vehicle big version");
        return iRet;
    }

    iRet = adm_dim_host_setVehicleBigVersion(adm_soa_getHostInstanceId(), 15, pVehicleBigVersion, &iErrCode);
    if (ADM_IS_SUCCESS(iRet) && ADM_IS_SUCCESS(iErrCode))
    {
        log_i("adm_dim_host_setVehicleBigVersion [%s] successed", pVehicleBigVersion);
    }
    else
    {
        log_e("adm_dim_host_setVehicleBigVersion [%s] failed, [iRet:%d, err:%d]", pVehicleBigVersion, iRet, iErrCode);
    }

    adm_free(pVehicleBigVersion);

    return iRet;
}

static D32S ipmUpdateVehicleVersionNumber()
{
    D32S iRet = ADM_ECODE_FAILED;
    DCHAR* pVehicleVersionNumber = DNULL;
    D32S iErrCode = ADM_ECODE_FAILED;

    adm_cvs_getHmiVehicleVersion(&pVehicleVersionNumber);
    if (ADM_IS_NULL(pVehicleVersionNumber))
    {
        log_e("adm_cvs_getHmiVehicleVersion failed, do not need to update display version number");
        return iRet;
    }

    iRet = adm_dim_setVehicleVersion(adm_soa_getHostInstanceId(), 15, pVehicleVersionNumber, &iErrCode);
    if (ADM_IS_SUCCESS(iRet) && ADM_IS_SUCCESS(iErrCode))
    {
        log_i("adm_dim_setVehicleVersion [%s] successed", pVehicleVersionNumber);
    }
    else
    {
        log_e("adm_dim_setVehicleVersion [%s] failed, [iRet:%d, err:%d]", pVehicleVersionNumber, iRet, iErrCode);
    }

    adm_free(pVehicleVersionNumber);

    return iRet;
}


static void ipmApkUpgradeResultHandler(ADM_IPM_FLASH_T *f,D32S result)
{
    if(ADM_ECODE_INSTALL_INVAL_POLICY == result)
	{
        f->state = ADM_IPM_FLASH_STATE_REMOTE_AUTH_OFF;
	}
	else
	{
	    adm_ipm_fflushEventReport();
        f->state = ADM_IPM_FLASH_STATE_POST_UPGRADE;
		ipmFlashStore(f);

		log_d("save upgrade result to kv ,result:%d", result);
		adm_ips_stepPostInstall(result);
		
		(void)usleep(10U*ADM_IPM_FLASH_GET_IPS_ECU_TIME_USEC_WIDE);//wait 200ms
#ifdef ADM_IPM_TEMP
        // (void)system("reboot");
        (void)usleep(50U*ADM_IPM_FLASH_GET_IPS_ECU_TIME_USEC_WIDE);//wait 10s
#endif		
	}

    return;
}

static void ipmUpgradeResultHandler(ADM_IPM_FLASH_T *f,D32S result)
{
    if(ADM_ECODE_INSTALL_INVAL_POLICY == result)
	{
        f->state = ADM_IPM_FLASH_STATE_REMOTE_AUTH_OFF;
	}
	else
	{
	    adm_ipm_fflushEventReport();
        f->state = ADM_IPM_FLASH_STATE_ACTIVE;
        ipmFlashStore(f);

        log_d("save upgrade result to kv ,result:%d", result);
		adm_ips_stepPostInstall(result);
		
// 		(void)usleep(10U*ADM_IPM_FLASH_GET_IPS_ECU_TIME_USEC_WIDE);//wait 200ms
// #ifdef ADM_IPM_TEMP
//         // (void)system("reboot");
//         (void)usleep(50U*ADM_IPM_FLASH_GET_IPS_ECU_TIME_USEC_WIDE);//wait 10s
// #endif		
	}

    return;
}

static void ipmCollectEcuInfoInPost(void)
{
    D32S iRet = ADM_ECODE_FAILED;
    ADM_ECS_RSCONFIGINFO_T *pstRsCfgData = NULL;
    ADM_IPS_POLICY_DEPENDENCY_ARRAY_T dependencies;

    do
    {
        pstRsCfgData = adm_ecs_newRSConfigInfo();
        if (ADM_IS_NULL(pstRsCfgData))
        {
            iRet = ADM_ECODE_NO_MEMORY;
            log_e("Failed to new ota config data info, err[%d]", iRet);
            break;
        }

        iRet = adm_ecs_getConfigInfo(pstRsCfgData);
        if (iRet != ADM_ECODE_SUCCESSED)
        {
            log_e("Failed get config info, nRet[%d]", iRet);
            break;
        }

        /** 获取升级策略中升级后的依赖信息 */
        adm_ips_policyDependencyArrayInit(&dependencies);

        (void)adm_ips_policyDependenciesGetWithType(&dependencies, (D32S)ADM_IPS_POLICY_DEPENDENCY_AFTER);
        if (dependencies.dependenciesLen == 0)
        {
            log_i("after install,the need check version ecu num is 0");
            break;
        }

        /** 并行获取ECU信息 */
        iRet = adm_eim_parallelCollectEcuInfo((DVOID *)pstRsCfgData, COLLECT_TRIGGER_TYPE_INSTALL, &dependencies);
        if (iRet != ADM_ECODE_SUCCESSED)
        {
            log_e("Failed to collect ecus info, nRet[%d]", iRet);
            break;
        }
    } while (FALSE);
    
    adm_ips_policyDependencyArrayDeInit(&dependencies);
    adm_ecs_delRSConfigInfo(&pstRsCfgData);

    return;
}

static D32S ipmStepUpgradePost(ADM_IPM_FLASH_T *f){
    ADM_EIM_GETECUINFO_T* allEcuInfo = NULL;
    D32S ret = ADM_ECODE_FAILED;
    ADM_CVS_UPGRADE_MODE_E eUpgradeMode = ADM_CVS_UPGRADE_MODE_INVALID;

    adm_cvs_getUpgradeMode((D32S *)&eUpgradeMode);

#ifdef ADM_IPM_TEMP
    //delay 10s wait sysi init ok ,add it to debug 
    (void)usleep(50U*ADM_IPM_FLASH_GET_IPS_ECU_TIME_USEC_WIDE);//wait 10s
#endif

    do
    {
    #ifndef ADM_PARALLEL_GET_ECU
        // 1.更新ecuinfo至kv
        ret = adm_eim_getEcuInfoManagerDim1Get(&allEcuInfo);
        adm_eim_deleteGetEcuInfoBody(allEcuInfo);
        if (ADM_ECODE_SUCCESSED != ret)
        {
            break;
        }
    #endif
    
#ifndef ADM_IPM_VERSION_VERIFY_INSTALL_HURRY
        ADM_IPS_PROGRESS_T progress;
        adm_ips_progressInit(&progress);
        adm_ips_progressGet(&progress);

        if (ADM_ECODE_SUCCESSED == progress.result)
        {
            /** APK 升级模式，等待hmi上线 */
            if (ADM_CVS_UPGRADE_MODE_APK_SELF == eUpgradeMode)
            {
                ipmDimWaitHmiOnline();
            }

            /** 升级成功，校验升级后的版本号 */
            ret = ipmDoVerifyEcusVersionAfterInstall(&f->doFlash);
            if (ADM_ECODE_SUCCESSED == ret)
            {
                ipmReportEventDependencyEventId("S_IE_020", NULL, ADM_ECODE_SUCCESSED);
            }
            else
            {
                ipmReportEventDependencyEventId("S_IE_021", NULL, ADM_ECODE_SUCCESSED);
            }

            if (ret != ADM_ECODE_SUCCESSED)
            {
                ret = ADM_ECODE_INSTALL_AFTERINSTALL_VERSION_VERIFY_FAILED;
                log_i("Verify ecu version failed.");
            }

            adm_ips_stepPostInstall(ret);
        }
        else
        {
            /** 升级失败，采集升级任务的零件信息 */
            ipmCollectEcuInfoInPost();
            ret = progress.result;
        }

        adm_ips_progressDeInit(&progress);
#endif
        // 3.上报ota server ecu 状态
        ipmReportOtaServerEcuStatus(&f->doFlash.report->reportOtaServer);

    } while (FALSE);

    return ret;
}

static DVOID ipmUpdateStageResult(ADM_IPM_FLASH_STATE_E eState, D32S iRet)
{
    D16U i = 0;
    D16U uiIndex = 0;

    uiIndex = g_stAllStageResult.uiIndex;

    if (uiIndex < ADM_MAX_STAGE_NUMBER)
    {
        log_i("ipmUpdateStageResult: state[%d],result[%d],index[%u]",
              eState, iRet, uiIndex);

        if (eState == g_stAllStageResult.stAllStageResult[uiIndex - 1].eState)
        {
            log_i("Find stage.");
            g_stAllStageResult.stAllStageResult[uiIndex - 1].iRet = iRet;

            if (ADM_ECODE_SUCCESSED != iRet && ADM_IPM_FLASH_STATE_UPGRADE == eState)
            {
                g_stAllStageResult.bIsFlashSuccess = DFALSE;
            }
            ipmAllStageResultStore(&g_stAllStageResult);
        }
        else 
        {
            log_i("Can not find stage.Current kv stage[%d].", g_stAllStageResult.stAllStageResult[uiIndex - 1].eState);
        }
    }
    else
    {
        log_w("The stage number is out controlled.");
    }

    return;
}


static DVOID ipmAddStageResult(ADM_IPM_FLASH_STATE_E eState, D32S iRet)
{
    D16U i =0;
    D16U uiIndex = 0;

    uiIndex = g_stAllStageResult.uiIndex;

    if (uiIndex < ADM_MAX_STAGE_NUMBER)
    {
        log_i("ipmAddStageResult: state[%d],result[%d],index[%u]",
              eState, iRet, uiIndex);

        if (ADM_IPM_FLASH_STATE_UPGRADE == eState)
        {
            if (uiIndex != 0)
            {
                if (ADM_IPM_FLASH_STATE_UPGRADE == g_stAllStageResult.stAllStageResult[uiIndex - 1].eState)
                {
                    log_i("Upgrade flash result exits. do not handle.");
                    return;
                }
            }

            if (ADM_IS_SUCCESS(iRet))
            {
                g_stAllStageResult.bIsFlashSuccess = DTRUE;
            }
            
        }

        g_stAllStageResult.stAllStageResult[uiIndex].eState = eState;
        g_stAllStageResult.stAllStageResult[uiIndex].iRet = iRet;
        g_stAllStageResult.uiIndex++;      ///< 存储当前有多少元素

        ipmAllStageResultStore(&g_stAllStageResult);
    }
    else 
    {
        log_w("The stage number is out controlled.");
    }

    return;
}

/** 二次前置条件校验失败，判断退出OTA模式和远程下电结果,先下电后退出OTA模式 */
static void ipmHandleVerifyCondResult(ADM_IPM_REPORT_T *pstReport, D16U uiIndex)
{
    D32S iResult = ADM_ECODE_FAILED;
    log_i("ipmHandleVerifyCondResult Handler result.[%d] [%d]", uiIndex, g_stAllStageResult.uiIndex);

    if (uiIndex < g_stAllStageResult.uiIndex)
    {
        log_i("ipmHandleVerifyCondResult state[%s], result[%d]", 
        ipmFlashStateString(g_stAllStageResult.stAllStageResult[uiIndex].eState), 
        g_stAllStageResult.stAllStageResult[uiIndex].iRet);

        switch (g_stAllStageResult.stAllStageResult[uiIndex].eState)
        {
            case ADM_IPM_FLASH_STATE_REMOTE_AUTH_OFF:
            if (ADM_ECODE_SUCCESSED == g_stAllStageResult.stAllStageResult[uiIndex].iRet)
            {
                /** 下电成功 */
                uiIndex++;
                if (ADM_IPM_FLASH_STATE_ENV_EXIT == g_stAllStageResult.stAllStageResult[uiIndex].eState)
                {
                    if (ADM_ECODE_SUCCESSED == g_stAllStageResult.stAllStageResult[uiIndex].iRet)
                    {
                        iResult = ADM_ECODE_INSTALL_FAILED_MODE2;
                    }
                    else 
                    {
                        iResult = ADM_ECODE_INSTALL_FAILED_MODE1;
                    }
                }
            }
            else 
            {
                /** 下电失败 */
                uiIndex++;
                if (ADM_IPM_FLASH_STATE_ENV_EXIT == g_stAllStageResult.stAllStageResult[uiIndex].eState)
                {
                    if (ADM_ECODE_SUCCESSED == g_stAllStageResult.stAllStageResult[uiIndex].iRet)
                    {
                        iResult = ADM_ECODE_INSTALL_FAILED_MODE4;
                    }
                    else 
                    {
                        iResult = ADM_ECODE_INSTALL_FAILED_MODE3;
                    }
                }
            }
            break;

            default:
            log_e("Do not find any stage[%d]", g_stAllStageResult.stAllStageResult[uiIndex].eState);
            break;
        }

        /** 兜底处理 */
        if (ADM_ECODE_FAILED == iResult)
        {
            log_w("ipmHandleVerifyCondResult can not find any match branch.");
            iResult = ADM_ECODE_INSTALL_GENERAL_FAILED;
        }

        ipmReportResult(pstReport, iResult);
    }

    return;
}

static void ipmHandleRollbackSuccResult(ADM_IPM_REPORT_T *pstReport, D16U uiIndex)
{
    D32S iResult = ADM_ECODE_FAILED;
    log_i("ipmHandleRollbackSuccResult Handler result.[%d] [%d]", uiIndex, g_stAllStageResult.uiIndex);

    if (uiIndex < g_stAllStageResult.uiIndex)
    {
        log_i("ipmHandleRollbackSuccResult state[%s], result[%d]", 
        ipmFlashStateString(g_stAllStageResult.stAllStageResult[uiIndex].eState), 
        g_stAllStageResult.stAllStageResult[uiIndex].iRet);

        switch (g_stAllStageResult.stAllStageResult[uiIndex].eState)
        {
            case ADM_IPM_FLASH_STATE_REMOTE_AUTH_OFF:
            if (ADM_ECODE_SUCCESSED == g_stAllStageResult.stAllStageResult[uiIndex].iRet)
            {
                /** 下电成功 */
                uiIndex++;
                if (ADM_IPM_FLASH_STATE_ENV_EXIT == g_stAllStageResult.stAllStageResult[uiIndex].eState)
                {
                    if (ADM_ECODE_SUCCESSED == g_stAllStageResult.stAllStageResult[uiIndex].iRet)
                    {
                        iResult = ADM_ECODE_INSTALL_FAILED_MODE8;
                    }
                    else 
                    {
                        iResult = ADM_ECODE_INSTALL_FAILED_MODE7;
                    }
                }
            }
            else 
            {
                /** 下电失败 */
                uiIndex++;
                if (ADM_IPM_FLASH_STATE_ENV_EXIT == g_stAllStageResult.stAllStageResult[uiIndex].eState)
                {
                    if (ADM_ECODE_SUCCESSED == g_stAllStageResult.stAllStageResult[uiIndex].iRet)
                    {
                        iResult = ADM_ECODE_INSTALL_FAILED_MODE6;
                    }
                    else 
                    {
                        iResult = ADM_ECODE_INSTALL_FAILED_MODE5;
                    }
                }
            }
            break;

            default:
            log_e("Do not find any stage[%d]", g_stAllStageResult.stAllStageResult[uiIndex].eState);
            break;
        }

        /** 兜底处理 */
        if (ADM_ECODE_FAILED == iResult)
        {
            log_w("ipmHandleRollbackSuccResult can not find any match branch.");
            iResult = ADM_ECODE_INSTALL_GENERAL_FAILED;
        }

        ipmReportResult(pstReport, iResult);
    }

    return;
}

static void ipmHandleInstallSuccResult(ADM_IPM_REPORT_T *pstReport, D16U uiIndex)
{
    D32S iResult = ADM_ECODE_FAILED;
    log_i("ipmHandleInstallSuccResult Handler result.[%d] [%d]", uiIndex, g_stAllStageResult.uiIndex);

    if (uiIndex < g_stAllStageResult.uiIndex)
    {
        log_i("ipmHandleInstallSuccResult state[%s], result[%d]", 
        ipmFlashStateString(g_stAllStageResult.stAllStageResult[uiIndex].eState), 
        g_stAllStageResult.stAllStageResult[uiIndex].iRet);

        switch (g_stAllStageResult.stAllStageResult[uiIndex].eState)
        {
            /** 后校验成功则存储下电 */
            case ADM_IPM_FLASH_STATE_REMOTE_AUTH_OFF:
            if (ADM_ECODE_SUCCESSED == g_stAllStageResult.stAllStageResult[uiIndex].iRet)
            {
                /** 下电成功 */
                uiIndex++;
                if (ADM_IPM_FLASH_STATE_ENV_EXIT == g_stAllStageResult.stAllStageResult[uiIndex].eState)
                {
                    if (ADM_ECODE_SUCCESSED == g_stAllStageResult.stAllStageResult[uiIndex].iRet)
                    {
                        iResult = ADM_ECODE_SUCCESSED;  ///< 安装成功、下电成功、退出OTA模式成功
                    }
                    else 
                    {
                        iResult = ADM_ECODE_INSTALL_FAILED_MODE11;
                    }
                }
            }
            else 
            {
                /** 下电失败 */
                uiIndex++;
                if (ADM_IPM_FLASH_STATE_ENV_EXIT == g_stAllStageResult.stAllStageResult[uiIndex].eState)
                {
                    if (ADM_ECODE_SUCCESSED == g_stAllStageResult.stAllStageResult[uiIndex].iRet)
                    {
                        iResult = ADM_ECODE_INSTALL_FAILED_MODE10;
                    }
                    else 
                    {
                        iResult = ADM_ECODE_INSTALL_FAILED_MODE9;
                    }
                }
            }
            break;

            /** 后校验失败则存储后校验结果 后检验失败，如果此时还有下电或者退出OTA模式失败,以这两者错误结果为准*/
            case ADM_IPM_FLASH_STATE_POST_UPGRADE:
            if (ADM_ECODE_SUCCESSED == g_stAllStageResult.stAllStageResult[uiIndex].iRet)
            {
                uiIndex++;
                if (g_stAllStageResult.stAllStageResult[uiIndex].eState == ADM_IPM_FLASH_STATE_REMOTE_AUTH_OFF)
                {
                    if (ADM_ECODE_SUCCESSED == g_stAllStageResult.stAllStageResult[uiIndex].iRet)
                    {
                        /** 下电成功 */
                        uiIndex++;
                        if (ADM_IPM_FLASH_STATE_ENV_EXIT == g_stAllStageResult.stAllStageResult[uiIndex].eState)
                        {
                            if (ADM_ECODE_SUCCESSED != g_stAllStageResult.stAllStageResult[uiIndex].iRet)
                            {
                                iResult = ADM_ECODE_INSTALL_FAILED_MODE11;
                            }
                            else 
                            {
                                /** 后检验失败 下电成功，退出OTA模式成功 则返回一般性错误 */
                                iResult = ADM_ECODE_INSTALL_GENERAL_FAILED;
                            }
                        }
                    }
                    else 
                    {
                        /** 下电失败 */
                        uiIndex++;
                        if (ADM_IPM_FLASH_STATE_ENV_EXIT == g_stAllStageResult.stAllStageResult[uiIndex].eState)
                        {
                            if (ADM_ECODE_SUCCESSED == g_stAllStageResult.stAllStageResult[uiIndex].iRet)
                            {
                                iResult = ADM_ECODE_INSTALL_FAILED_MODE10;
                            }
                            else 
                            {
                                iResult = ADM_ECODE_INSTALL_FAILED_MODE9;
                            }
                        }
                    }
                }
                else 
                {
                    log_w("Can not find auth off state.");
                    iResult = ADM_ECODE_INSTALL_GENERAL_FAILED;
                }
            }
            else 
            {
                log_i("Post upgrade failed.");
                iResult = ADM_ECODE_INSTALL_GENERAL_FAILED;
            }
            break;

            default:
            log_e("Do not find any stage[%d]", g_stAllStageResult.stAllStageResult[uiIndex].eState);
            break;
        }

        /** 兜底处理 */
        if (ADM_ECODE_FAILED == iResult)
        {
            log_w("ipmHandleInstallSuccResult can not find any match branch.");
            iResult = ADM_ECODE_INSTALL_GENERAL_FAILED;
        }

        ipmReportResult(pstReport, iResult);
    }

    return;
}

static void ipmHandleUpgradeStageResult(ADM_IPM_REPORT_T *pstReport, D16U uiIndex)
{
    D32S iResult = ADM_ECODE_FAILED;

    if (ADM_IPM_FLASH_STATE_UPGRADE == g_stAllStageResult.stAllStageResult[uiIndex].eState)
    {
        iResult = g_stAllStageResult.stAllStageResult[uiIndex].iRet;
        if (ADM_ECODE_SUCCESSED != iResult)
        {
            /** 刷写失败，回滚成功 */
            if (ADM_ECODE_INSTALL_ROLLBACK_SUCCESSED == iResult)
            {
                ipmReportHmiStageResult(&pstReport->reportHmi, iResult);    ///< 安装失败回滚成功，车辆正在下电
            }
            else if (ADM_ECODE_INSTALL_ROLLBACK_FAILED == iResult)
            {
                ipmReportHmiStageResult(&pstReport->reportHmi, iResult);    ///< 安装失败回滚失败，车辆正在下电
            }
            else 
            {
                /** 刷写失败，无回滚，认为刷写失败回滚成功 */
                ipmReportHmiStageResult(&pstReport->reportHmi, ADM_ECODE_INSTALL_ROLLBACK_SUCCESSED);
            }
        }
        else
        {
            log_i("uiIndex is [%d], g_stAllStageResult.uiIndex is [%d]", uiIndex, g_stAllStageResult.uiIndex);
            //当前有后校验的情况下才需要修改结果，不然直接上报升级成功。
            if(g_stAllStageResult.uiIndex >= 1)
            {
                ++uiIndex;
                if(g_stAllStageResult.stAllStageResult[uiIndex].eState == ADM_IPM_FLASH_STATE_POST_UPGRADE)
                {
                    iResult = g_stAllStageResult.stAllStageResult[uiIndex].iRet;
                    log_i("iResult is [%d]", iResult);
                }
            }
            ipmReportHmiStageResult(&pstReport->reportHmi, iResult);    ///< 安装成功，车辆正在下电或版本后校验失败；
        }
    }

    sleep(ADM_STAGE_TIME_DELAY);       ///< 防止最终结果覆盖正在下电的弹框
    return;
}

static void ipmHandleUpgradeResult(ADM_IPM_REPORT_T *pstReport, D16U uiIndex)
{
    D32S iResult = ADM_ECODE_FAILED;

    if (ADM_IPM_FLASH_STATE_UPGRADE == g_stAllStageResult.stAllStageResult[uiIndex].eState)
    {
        iResult = g_stAllStageResult.stAllStageResult[uiIndex].iRet;
        if (ADM_ECODE_SUCCESSED != iResult)
        {
            /** 刷写失败，回滚成功 */
            if (ADM_ECODE_INSTALL_ROLLBACK_SUCCESSED == iResult)
            {
                ipmHandleRollbackSuccResult(pstReport, uiIndex + 1);
            }
            else if (ADM_ECODE_INSTALL_ROLLBACK_FAILED == iResult)
            {
                iResult = ADM_ECODE_INSTALL_GENERAL_FAILED;
                ipmReportResult(pstReport, iResult);    ///< 上报最终结果
            }
            else if (ADM_ECODE_INSTALL_ENTER_HIGH_VOLTAGE_FAILED == iResult ||  ADM_ECODE_INSTALL_EXIT_HIGH_VOLTAGE_FAILED == iResult )
            {
                ipmReportResult(pstReport, iResult);    ///< 上报最终结果
            }
            else 
            {
                log_i("Install failed iResult[%d]", iResult);
                iResult = ADM_ECODE_INSTALL_GENERAL_FAILED;
                ipmReportResult(pstReport, iResult);    ///< 上报最终结果
            }
        }
        else
        {
            ipmHandleInstallSuccResult(pstReport, uiIndex + 1);
        }
    }

    return;
}

/** 处理下电弹窗 */
static void ipmHandleStageResult(ADM_IPM_REPORT_T *pstReport)
{
    D16U uiIndex = 0;

    adm_ipm_init();

    if (g_stAllStageResult.uiIndex > 0)
    {
        /** 经过上电后，操作失败，可以提示车辆正在进行下电弹窗 */
        if (ADM_IPM_FLASH_STATE_UPGRADE == g_stAllStageResult.stAllStageResult[uiIndex].eState)
        {
            ipmHandleUpgradeStageResult(pstReport, uiIndex);
        }
        else if (ADM_IPM_FLASH_STATE_VERIFY_CONDITION == g_stAllStageResult.stAllStageResult[uiIndex].eState ||
                 ADM_IPM_FLASH_STATE_VERIFT_SIGN_PACKAGE == g_stAllStageResult.stAllStageResult[uiIndex].eState ||
                 ADM_IPM_FLASH_STATE_VERIFT_ECUS_VERSION == g_stAllStageResult.stAllStageResult[uiIndex].eState)
        {
            D32S iResult = g_stAllStageResult.stAllStageResult[uiIndex].iRet;
            if (ADM_ECODE_SUCCESSED != iResult)
            {
                /** 未进行刷写，按照刷写失败回滚成功提示的窗口处理 */
                ipmReportHmiStageResult(&pstReport->reportHmi, ADM_ECODE_INSTALL_ROLLBACK_SUCCESSED);
                sleep(ADM_STAGE_TIME_DELAY); ///< 防止最终结果覆盖正在下电的弹框
            }
            else 
            {
                log_w("Current state[%d] result is successful,do not handle.", g_stAllStageResult.stAllStageResult[uiIndex].eState);
            }
        }
        else 
        {
            log_w("Current state[%d] do not handle.", g_stAllStageResult.stAllStageResult[uiIndex].eState);
        }
    }

    return;
}

/** 处理需要通知HMI的错误码值 */
static void ipmHandleResult(ADM_IPM_REPORT_T *pstReport, D32S *pSumResult)
{
    D16U uiIndex = 0;
    D32S iResult = ADM_ECODE_FAILED;

    adm_ipm_init();

    if (g_stAllStageResult.uiIndex > 0)
    {
        log_i("Handle result info. stage number:[%u], state[%s] result[%d]", 
        g_stAllStageResult.uiIndex,
        ipmFlashStateString(g_stAllStageResult.stAllStageResult[uiIndex].eState), 
        g_stAllStageResult.stAllStageResult[uiIndex].iRet);

        for (D16U i = 0; i < g_stAllStageResult.uiIndex; i++)
        {
            log_i("state[%u]:%s, result[%u]:%d", i, ipmFlashStateString(g_stAllStageResult.stAllStageResult[i].eState),
                i, g_stAllStageResult.stAllStageResult[i].iRet);
        }

        switch (g_stAllStageResult.stAllStageResult[uiIndex].eState)
        {
            case ADM_IPM_FLASH_STATE_IDLE:
                /** no break */
            case ADM_IPM_FLASH_STATE_VERIFT_ECUS_VERSION:
            if (ADM_ECODE_SUCCESSED != g_stAllStageResult.stAllStageResult[uiIndex].iRet)
            {
                iResult = ADM_ECODE_INSTALL_GENERAL_FAILED;
                ipmReportResult(pstReport, iResult);
            }
            break;

            case ADM_IPM_FLASH_STATE_VERIFT_SIGN_PACKAGE:
            if (ADM_ECODE_SUCCESSED != g_stAllStageResult.stAllStageResult[uiIndex].iRet)
            {
                if (g_stAllStageResult.stAllStageResult[uiIndex].iRet != ADM_ECODE_INSTALL_SIGN_VERIFY_FAILED)
                {
                    iResult = ADM_ECODE_INSTALL_GENERAL_FAILED;
                }
                else 
                {
                    iResult = g_stAllStageResult.stAllStageResult[uiIndex].iRet;
                }
                ipmReportResult(pstReport, iResult);
            }
            break;

            case ADM_IPM_FLASH_STATE_VERIFT_TASK:
            if (ADM_ECODE_SUCCESSED != g_stAllStageResult.stAllStageResult[uiIndex].iRet)
            {
                iResult = ADM_ECODE_INSTALL_TASK_VERIFY_FAILED;
                ipmReportResult(pstReport, iResult);
            }
            break;

            case ADM_IPM_FLASH_STATE_VERIFY_CONDITION:
            if (ADM_ECODE_SUCCESSED != g_stAllStageResult.stAllStageResult[uiIndex].iRet)
            {
                ipmHandleVerifyCondResult(pstReport, uiIndex + 1);
            }
            break;
            
            case ADM_IPM_FLASH_STATE_ENV_ENTRY:
            if (ADM_ECODE_SUCCESSED != g_stAllStageResult.stAllStageResult[uiIndex].iRet)
            {
                iResult = ADM_ECODE_INSTALL_ENTER_OTAMODE_FAILED;
                ipmReportResult(pstReport, iResult);
            }
            break;

            case ADM_IPM_FLASH_STATE_REMOTE_AUTH_ON:
            if (ADM_ECODE_SUCCESSED != g_stAllStageResult.stAllStageResult[uiIndex].iRet)
            {
                uiIndex++;
                if (ADM_IPM_FLASH_STATE_ENV_EXIT == g_stAllStageResult.stAllStageResult[uiIndex].eState)
                {
                    if (ADM_ECODE_SUCCESSED == g_stAllStageResult.stAllStageResult[uiIndex].iRet)
                    {
                        iResult = ADM_ECODE_INSTALL_REMOTE_POWER_ON_FAILED;
                    }
                    else 
                    {
                        iResult = ADM_ECODE_INSTALL_EXIT_OTAMODE_FAILED;
                    }
                    ipmReportResult(pstReport, iResult);
                }
            }
            break;

            case ADM_IPM_FLASH_STATE_UPGRADE:
            ipmHandleUpgradeResult(pstReport, uiIndex);
            break;

            default:
            log_e("Do not match any stage[%d]", g_stAllStageResult.stAllStageResult[uiIndex].eState);
            break;
        }
    }
    else 
    {
        log_e("Data is invalid.");
    }

    return;
}

static D32S ipmSwitchHvOn(void)
{
    D32S iRet = ADM_ECODE_FAILED;
    VEHICLE_VOLTAGE_STATE_E eExpectState = HV_ABNORMAL;
    VEHICLE_VOLTAGE_STATE_E eCurrentState = HV_ABNORMAL;
    ADM_IPM_ECU_INFO_NODE_T stNodeTemp = {0};

    ADM_COM_FUNC_ENTRY();

    do
    {
        eExpectState = HV_HIGH;
        // eCurrentState = ipmGetCurrentHvMode();
        // if (eExpectState == eCurrentState)
        // {
        //     iRet = ADM_ECODE_SUCCESSED;
        //     break;
        // }

        stNodeTemp.ipsEcu.ecuTimeout = ADM_IPM_DIM_REQUEST_TIMEOUT;
        iRet = ipmSwitchHvMode(&stNodeTemp, VEHICLE_CTRL_ENTER_HV_MODE, eExpectState);
    } while (FALSE);

    ADM_COM_FUNC_EXIT();

    return iRet;
}

static D32S ipmSwitchHvOff()
{
    D32S iRet = ADM_ECODE_FAILED;
    VEHICLE_VOLTAGE_STATE_E eExpectState = HV_ABNORMAL;
    VEHICLE_VOLTAGE_STATE_E eCurrentState = HV_ABNORMAL;
    ADM_IPM_ECU_INFO_NODE_T stNodeTemp = {0};

    do
    {
        eExpectState = HV_NO_HIGH;
        eCurrentState = ipmGetCurrentHvMode();
        if (eExpectState == eCurrentState)
        {
            iRet = ADM_ECODE_SUCCESSED;
            break;
        }

        stNodeTemp.ipsEcu.ecuTimeout = ADM_IPM_DIM_REQUEST_TIMEOUT;
        iRet = ipmSwitchHvMode(&stNodeTemp, VEHICLE_CTRL_EXIT_HV_MODE, eExpectState);
    } while (FALSE);

    return iRet;
}

/**  
 * 1、下电成功，校验是否HV OFF:
 *          （1）、是HV OFF 下电成功
 *          （2）、不是HV OFF 下电失败
 * 2、下电失败，调用紧急下电接口，然后校验电源挡位：
*                  （1）、电源非OFF档 下电失败
*                  （2）、电源OFF档，则校验HV OFF
*                      （a）、是HV OFF 下电成功
*                      （b）、不是HV OFF 下电失败
 *          
*/
static D32S ipmEnsurePowerOff(D32S iPowerOffResult)
{
    ADM_DIM_SOA_MSGID_DATA_ARRAY_T stReqMsgData = {};
    ADM_DIM_SOA_MSGID_DATA_ARRAY_T* pstRespMsgData = NULL;
    D32S iResult = ADM_ECODE_SUCCESSED;
    D32S iErrorCode = ADM_ECODE_FAILED;
    D32S modeSwitch = -1;
    ADM_APPLICATION_COMMON_CMDID_TYPE_E eTrigger = ADM_APPLICATION_COMMON_CMDID_TYPE_INVALID;
    D16U i = 0;
    DBOOL bIsFuel = DFALSE; 

    VEHICLE_VOLTAGE_STATE_E eCurrentState = HV_ABNORMAL;

    iResult = adm_vsm_getIsFuel(&bIsFuel);
    if (ADM_IS_ERR(iResult))
    {
        log_e("Get vehicle power type failed, iRet:%d", iResult);
        return iPowerOffResult;  // 直接返回失败
    }

    if (DTRUE == bIsFuel)
    {
		log_i("ICE Vechile.");
        return iPowerOffResult;
    }

    do
    {
        /** 救援升级需要根据策略判断是否需要紧急下电 */
        eTrigger = uimSoaCmdidTypeGet();
        if (ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_RESCUE == eTrigger)
        {
            iResult = adm_cvs_getModeSwitch(&modeSwitch);
            if (ADM_IS_SUCCESS(iResult) && 0 == modeSwitch)
            {
                log_i("mode[%d],need not ensure power off.", eTrigger);
                break;
            }
        }

        if (ADM_IS_SUCCESS(iPowerOffResult))
        {
            eCurrentState = ipmGetCurrentHvMode();
            if (HV_NO_HIGH != eCurrentState)
            {
                log_e("Check HV OFF failed.");
                iResult = ADM_ECODE_INSTALL_REMOTE_POWER_OFF_FAILED;
            }
        }
        else 
        {
            /** 调用紧急下电 */
            iResult = adm_dim_host_setIcuPowerOff(adm_soa_getHostInstanceId(), ADM_POWER_CONTROL_TIMEOUT, &iErrorCode);
            if (ADM_IS_ERR(iResult) || ADM_IS_ERR(iErrorCode))
            {
                log_e("adm_dim_host_setIcuPowerOff failed. ret[%d] errCode[%d].", iResult, iErrorCode);
                iResult = ADM_ECODE_INSTALL_REMOTE_POWER_OFF_FAILED;
                break;
            }

            /** 查询电源状态 */
            stReqMsgData.msgIdArrayLength = 1;
            stReqMsgData.msgIdArray = (ADM_DIM_SOA_VDI_MSGID_T**)adm_malloc(sizeof(ADM_DIM_SOA_VDI_MSGID_T*) * (DSIZE)stReqMsgData.msgIdArrayLength);

            stReqMsgData.msgIdArray[0] = (ADM_DIM_SOA_VDI_MSGID_T*)adm_malloc((DSIZE)sizeof(ADM_DIM_SOA_VDI_MSGID_T));
            stReqMsgData.msgIdArray[0]->msgIdData = NULL;
            stReqMsgData.msgIdArray[0]->msgId = VEHICLE_STATUS_POWER_SIGNAL;

            for (i = 0; i < 3; i++)
            {
                iResult = adm_dim_host_readVehicleStatus(adm_soa_getHostInstanceId(), 10, &stReqMsgData, &pstRespMsgData, &iErrorCode);
                if (ADM_IS_ERR(iResult) || ADM_IS_ERR(iErrorCode))
                {
                    log_e("adm_dim_host_readVehicleStatus failed. ret[%d] errCode[%d].", iResult, iErrorCode);
                    sleep(1);
                }
                else 
                {
                    break;
                }
            }
            if (3 == i || NULL == pstRespMsgData || 1 != pstRespMsgData->msgIdArrayLength || 
                NULL == pstRespMsgData->msgIdArray[0] || NULL == pstRespMsgData->msgIdArray[0]->msgIdData || 
                VAR_S32 != pstRespMsgData->msgIdArray[i]->msgIdData->dataType)
            {
                log_e("adm_dim_host_readVehicleStatus failed.");
                iResult = ADM_ECODE_INSTALL_REMOTE_POWER_OFF_FAILED;
                break;
            }
            
            /** 非OFF */
            if (2 != pstRespMsgData->msgIdArray[i]->msgIdData->v.valueS32)
            {
                log_e("Current power state[%d] is not off.", pstRespMsgData->msgIdArray[i]->msgIdData->v.valueS32);
                iResult = ADM_ECODE_INSTALL_REMOTE_POWER_OFF_FAILED;
                break;
            }

            eCurrentState = ipmGetCurrentHvMode();
            if (HV_NO_HIGH != eCurrentState)
            {
                log_e("Check HV OFF failed.");
                iResult = ADM_ECODE_INSTALL_REMOTE_POWER_OFF_FAILED;
                break;
            }
        }
    } while (FALSE);

    if (NULL != pstRespMsgData)
    {
        for (i = 0; i < pstRespMsgData->msgIdArrayLength; i++)
        {
            adm_free(pstRespMsgData->msgIdArray[i]->msgIdData);
            adm_free(pstRespMsgData->msgIdArray[i]);
        }
        adm_free(pstRespMsgData->msgIdArray);
        adm_free(pstRespMsgData);

        for (i = 0; i < stReqMsgData.msgIdArrayLength; i++)
        {
            adm_free(stReqMsgData.msgIdArray[i]);
        }
        adm_free(stReqMsgData.msgIdArray);
    }

    return iResult;
}

static D32S ipmControlRemotePower(ADM_DIM_SOA_HOST_OTA_TASK_TYPE_E eControlType, D32S* pErrorCode)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    D32S modeSwitch = -1;
    ADM_APPLICATION_COMMON_CMDID_TYPE_E eTrigger = ADM_APPLICATION_COMMON_CMDID_TYPE_INVALID;

    if (ADM_IS_NULL(pErrorCode) ||
         (ADM_DIM_SOA_HOST_OTA_TASK_TYPE_REMOTE_POWER_ON != eControlType &&
        ADM_DIM_SOA_HOST_OTA_TASK_TYPE_REMOTE_POWER_OFF != eControlType))
    {
        log_e("Invalid param.");
        iRet = ADM_ECODE_INVALID_PARAM;
        return iRet;
    }

    do
    {
        /** 救援升级需要根据策略判断是否上下电 */
        eTrigger = uimSoaCmdidTypeGet();
        if (ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_RESCUE == eTrigger)
        {
            iRet = adm_cvs_getModeSwitch(&modeSwitch);
            if (ADM_IS_SUCCESS(iRet) && 0 == modeSwitch)
            {
                log_i("mode[%d],need not remote power.", eTrigger);
                *pErrorCode = ADM_ECODE_SUCCESSED;
                break;
            }
        }

        iRet = adm_dim_host_setPower(adm_soa_getHostInstanceId(), ADM_POWER_CONTROL_TIMEOUT, eControlType, pErrorCode);
        log_i("Remote power on/off control type[%d] ret[%d] errorCode[%d].", eControlType, iRet, *pErrorCode);
    } while (DFALSE);

    return iRet;
}

static DBOOL ipmGetCurrentPowerState()
{
    DBOOL bIsPowerOn = DFALSE;
    ADM_DIM_SOA_MSGID_DATA_ARRAY_T stReqMsgData = {};
    ADM_DIM_SOA_MSGID_DATA_ARRAY_T *pstRespMsgData = NULL;
    D32S iPowerType = -1;
    D32S iErrCode = -1;
    D32S iRet = ADM_ECODE_SUCCESSED;

    stReqMsgData.msgIdArrayLength = 1;
    stReqMsgData.msgIdArray = (ADM_DIM_SOA_VDI_MSGID_T **)adm_malloc(sizeof(ADM_DIM_SOA_VDI_MSGID_T *) * (DSIZE)stReqMsgData.msgIdArrayLength);

    (void)adm_memset(stReqMsgData.msgIdArray, 0, sizeof(ADM_DIM_SOA_VDI_MSGID_T *) * stReqMsgData.msgIdArrayLength,
                     sizeof(ADM_DIM_SOA_VDI_MSGID_T *) * stReqMsgData.msgIdArrayLength);

    stReqMsgData.msgIdArray[0] = (ADM_DIM_SOA_VDI_MSGID_T *)adm_malloc((DSIZE)sizeof(ADM_DIM_SOA_VDI_MSGID_T));

    log_i("begin request readVehicleStatus.");
    stReqMsgData.msgIdArray[0]->msgId = VEHICLE_STATUS_POWER_SIGNAL;
    stReqMsgData.msgIdArray[0]->msgIdData = NULL;

    /** 判断是否非下电状态； 否则，退出升级 */
    iRet = adm_dim_host_readVehicleStatus(adm_soa_getHostInstanceId(), 15, &stReqMsgData, &pstRespMsgData, &iErrCode);
    if (ADM_IS_ERR(iRet) || ADM_IS_ERR(iErrCode))
    {
        /** 由触发源控制是否重试 */
        log_e("adm_dim_host_readVehicleStatus call error.iRet[%d], errCode[%d]", iRet, iErrCode);
        adm_free(stReqMsgData.msgIdArray[0]);
        adm_free(stReqMsgData.msgIdArray);
        return bIsPowerOn;
    }

    if (NULL != pstRespMsgData && NULL != pstRespMsgData->msgIdArray &&
        VEHICLE_STATUS_POWER_SIGNAL == pstRespMsgData->msgIdArray[0]->msgId &&
        NULL != pstRespMsgData->msgIdArray[0]->msgIdData)
    {
        do
        {
            if (VAR_S32 != pstRespMsgData->msgIdArray[0]->msgIdData->dataType)
            {
                /** 给触发源成功,不要进行重试 */
                log_e("Value type invalid.type[%u]", pstRespMsgData->msgIdArray[0]->msgIdData->dataType);
                iRet = ADM_ECODE_SUCCESSED;
                break;
            }
            iPowerType = pstRespMsgData->msgIdArray[0]->msgIdData->v.valueS32;

            if (OPWER_ON == iPowerType)
            {
                log_w("iPowerType = %d is PowerOn state.", iPowerType);
                bIsPowerOn = DTRUE;
                break;
            }
            else
            {
                log_i("iPowerType = %d is PowerOff state.", iPowerType);
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

    adm_free(stReqMsgData.msgIdArray[0]);
    adm_free(stReqMsgData.msgIdArray);

    return bIsPowerOn;
}

static D32S ipmGetInstallResult()
{
    D32S iResult = ADM_ECODE_FAILED;
    DBOOL bIsFindFailedResult = DFALSE;

    for (D16U i = 0; i < g_stAllStageResult.uiIndex; i++)
    {
        if (ADM_IPM_FLASH_STATE_INVALID == g_stAllStageResult.stAllStageResult[i].eState)
        {
            break;
        }

        if (ADM_ECODE_SUCCESSED != g_stAllStageResult.stAllStageResult[i].iRet)
        {
            log_i("Find stage[%d] result[%d].", g_stAllStageResult.stAllStageResult[i].eState, g_stAllStageResult.stAllStageResult[i].iRet);
            if (ADM_IPM_FLASH_STATE_REMOTE_AUTH_OFF != g_stAllStageResult.stAllStageResult[i].eState)
            {
                /** 下电结果不影响云端上报的总结果 */
                iResult = g_stAllStageResult.stAllStageResult[i].iRet;
                bIsFindFailedResult = DTRUE;
                break;
            }
        }
    }

    if (DFALSE == bIsFindFailedResult && g_stAllStageResult.uiIndex > 0)
    {
        if (ADM_IPM_FLASH_STATE_INVALID != g_stAllStageResult.stAllStageResult[0].eState)
        {
            iResult = ADM_ECODE_SUCCESSED;
        }
    }

    return iResult;
}

static D32S ipmApkUpgradeRun(ADM_IPM_FLASH_T *f)
{
    const DCHAR *eventDesc = NULL;
    D32S ret = ADM_ECODE_FAILED;
    D32S ret1 = ADM_ECODE_FAILED;
    D32S errorCode = ADM_ECODE_FAILED;
    D32S iErrorCode = ADM_ECODE_FAILED;
    D32S iPowerOffResult = 0;
    DBOOL bPowerOnSucceed = DFALSE;
    ADM_APPLICATION_COMMON_CMDID_TYPE_E eTspTriggerType = ADM_APPLICATION_COMMON_CMDID_TYPE_INVALID;

    eTspTriggerType = adm_vsm_getTspTriggerType();

    log_d("state:%s", ipmFlashStateString(f->state));
    log_i("[ABUP-Procedure] Install entry state:%s",  ipmFlashStateString(f->state));

    /** 通知vdi进入安装,不允许其他操作抢占件的通信链路 */
    // adm_ipm_notifyFlashState(ADM_DIM_SOA_VDI_FLASH_STATE_ENTER, ADM_IPM_DIM_REQUEST_TIMEOUT);

    /** 解决版本后校验后车机异常重启带来的云端上报结果误判 */
    if (f->state > ADM_IPM_FLASH_STATE_POST_UPGRADE)
    {
        ret = ipmGetInstallResult();
    }

    do
    {
        // ipmSetUpgradeMode();

        switch (f->state)
        {
        case ADM_IPM_FLASH_STATE_IDLE:
			ret = ipmStepInstallPrepare();
		    ipmReportTriggerInstallEventDependencyTriggerType((ADM_APPLICATION_COMMON_CMDID_TYPE_E)adm_ips_cmdIdGet());
	        if(ADM_ECODE_SUCCESSED == ret){
				f->state = ADM_IPM_FLASH_STATE_VERIFY_CONDITION;
            }else{
                ipmAddStageResult(f->state, ret);
		        f->state = ADM_IPM_FLASH_STATE_REPORT;
			    eventDesc = "Installation termination, Install Prepare error";
	        }
            break;
        case ADM_IPM_FLASH_STATE_VERIFT_TASK:       ///!< 正常执行任务校验的动作
	        if(ADM_ECODE_SUCCESSED == (ret = ipmStepVerifyTask(f))){
                f->state = ADM_IPM_FLASH_STATE_ENV_ENTRY;
            }else{
                ipmAddStageResult(f->state, ret);
                f->state = ADM_IPM_FLASH_STATE_REPORT;
				eventDesc = "Installation termination, verify task fail";
            }
            break;
        case ADM_IPM_FLASH_STATE_ENV_ENTRY:         ///!< 只需要维持网络唤醒，即维持车机唤醒即可
            if (ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_TSP != eTspTriggerType)
            {
                if(ADM_ECODE_SUCCESSED == (ret = ipmStepEnvEntry(f))){
                    f->state = ADM_IPM_FLASH_STATE_REMOTE_AUTH_ON;
                }else{
                    ipmAddStageResult(f->state, ret);
                    f->state = ADM_IPM_FLASH_STATE_REPORT;
                    eventDesc = "Installation termination, Env Entry fail";
                }
            }
            else
            {
                log_i("Tsp task default ota-mode set to success.");
                f->state = ADM_IPM_FLASH_STATE_REMOTE_AUTH_ON;
            }
            break;
        case ADM_IPM_FLASH_STATE_REMOTE_AUTH_ON:
            log_i("[ABUP-Procedure]: Install Start Entry Power On");
            if (ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_TSP != eTspTriggerType)
            {
#ifdef SUPPORT_REMOTE_AUTH
                ret = ipmControlRemotePower(ADM_DIM_SOA_HOST_OTA_TASK_TYPE_REMOTE_POWER_ON, &errorCode);
                if(ADM_ECODE_SUCCESSED == ret && errorCode == ADM_ECODE_SUCCESSED)
                {
                    /** 兜底PDCS件控制逻辑,上电20内必须要上高压操作，否则后续将无法进行上高压 */
                    ret = ipmSwitchHvOn();
                    if (ADM_IS_SUCCESS(ret))
                    {
                        bPowerOnSucceed = DTRUE;
                        f->state = ADM_IPM_FLASH_STATE_VERIFY_CONDITION;
                        adm_ipm_remoutePowerOnEventReport(ret);
                    }
                }

                if (DFALSE == bPowerOnSucceed)
                {
                    (DVOID)ipmSwitchHvOff();
                    ret = ADM_ECODE_INSTALL_REMOTE_POWER_ON_FAILED;
                    ipmAddStageResult(f->state, ret);
                    log_e("power on failed. ret[%d], errCode[%d]", ret, errorCode);
                    f->state = ADM_IPM_FLASH_STATE_ENV_EXIT;
                    eventDesc = "Installation termination, remote power on fail";
                    
                    ret = ADM_IS_ERR(ret) ? ret : errorCode;
                    adm_ipm_remoutePowerOnEventReport(ret);

                    /** 执行上电失败，需要执行下电，并且不计入总结果 */
                    log_d("Try to power off!");
                    iPowerOffResult = ipmControlRemotePower(ADM_DIM_SOA_HOST_OTA_TASK_TYPE_REMOTE_POWER_OFF, &errorCode);
                    iPowerOffResult = ADM_IS_ERR(iPowerOffResult) ? iPowerOffResult : errorCode;
                    adm_ipm_remoutePowerOffEventReport(iPowerOffResult);
                }
#else 
                log_i("Simulate power-on succeed.");
                ret = ADM_ECODE_SUCCESSED;
                adm_ipm_remoutePowerOnEventReport(ret);
                f->state = ADM_IPM_FLASH_STATE_VERIFY_CONDITION;
#endif    
            }
            else 
            {
                log_i("Tsp task default power-on set to success.");
                ret = ADM_ECODE_SUCCESSED;
                adm_ipm_remoutePowerOnEventReport(ret);
                f->state = ADM_IPM_FLASH_STATE_VERIFY_CONDITION;
            }
			log_i("[ABUP-Procedure]: Install End Entry Power On, [err:%d]", ret);
            break;
        case ADM_IPM_FLASH_STATE_VERIFY_CONDITION:  ///!< 条件为空，可以执行校验，默认成功即可 
	        if(ADM_ECODE_SUCCESSED == (ret = ipmStepVerifyCondition(f))){
                f->state = ADM_IPM_FLASH_STATE_VERIFT_SIGN_PACKAGE;
            }else{
                ipmAddStageResult(f->state, ret);
		        f->state = ADM_IPM_FLASH_STATE_REMOTE_AUTH_OFF;
				eventDesc = "Installation termination, Verify Condition fail";
	        }
            break;
        case ADM_IPM_FLASH_STATE_VERIFT_SIGN_PACKAGE:   ///!< 升级包校验也是需要做的
            log_i("[ABUP-Procedure]: Install Start Verify Sign Package");
	        if(ADM_ECODE_SUCCESSED == (ret = ipmStepVerifySignPackage())){
                f->state = ADM_IPM_FLASH_STATE_VERIFT_ECUS_VERSION;
            }else{
                ipmAddStageResult(f->state, ret);
		        f->state = ADM_IPM_FLASH_STATE_REMOTE_AUTH_OFF;
				eventDesc = "Installation termination, verify sign fail";
	        }
            log_i("[ABUP-Procedure]: Install End Verify Sign Package, [err:%d]", ret);
            break;
        case ADM_IPM_FLASH_STATE_VERIFT_ECUS_VERSION:   ///!< 升级前的版本信息需要校验
            log_i("[ABUP-Procedure]: Install Start Verify Ecus version");
	        if(ADM_ECODE_SUCCESSED == (ret = ipmStepVerifyEcusVersion(f))){
                f->state = ADM_IPM_FLASH_STATE_UPGRADE;
            }else{
                ipmAddStageResult(f->state, ret);
		        f->state = ADM_IPM_FLASH_STATE_REMOTE_AUTH_OFF;
				eventDesc = "Installation termination, verify ecu version fail";
	        }
            log_i("[ABUP-Procedure]: Install End Verify Ecus version, [err:%d]", ret);
            break;
        case ADM_IPM_FLASH_STATE_UPGRADE:               ///!< 执行APK自升级的逻辑控制
            ret = ipmStepApkUpgrade(f);
            ipmAddStageResult(f->state, ret);
			ipmApkUpgradeResultHandler(f,ret);
            break;
		case ADM_IPM_FLASH_STATE_POST_UPGRADE:          ///!< Post 是校验了版本号
            log_i("[ABUP-Procedure]: Install Start Post Upgrade");
            ret = ipmStepUpgradePost(f);
            if (DTRUE == g_stAllStageResult.bIsFlashSuccess && ADM_ECODE_SUCCESSED != ret)
            {
                ipmAddStageResult(f->state, ret);
            }
	        f->state = ADM_IPM_FLASH_STATE_REMOTE_AUTH_OFF;
            log_i("[ABUP-Procedure]: Install End Post Upgrade, [err:%d]", ret);
            break;	
        case ADM_IPM_FLASH_STATE_REMOTE_AUTH_OFF:
            /** 增加清理故障码的逻辑 不考虑超时,请求给到qnx从控即可*/
            (DVOID)adm_dim_vdi_clearDtc(adm_soa_getVdiInstanceId(), ADM_IPM_DIM_REQUEST_TIMEOUT, &iErrorCode);

            ipmHandleStageResult(f->report);     ///< 通知HMI下电弹框

            /** 下电前保持当前是下高压状态 */
            ret1 = ipmSwitchHvOff();
            log_i("Switch to hv off, [err:%d]", ret1);
#ifdef SUPPORT_REMOTE_AUTH
            sleep(5);       ///!< 等待5s
			log_i("[ABUP-Procedure]: Install Start Entry Power Off");
            ret1 = ipmControlRemotePower(ADM_DIM_SOA_HOST_OTA_TASK_TYPE_REMOTE_POWER_OFF, &errorCode);
			
            ret1 = ADM_IS_ERR(ret1) ? ret1 : errorCode;

            /** 强制校验当前下电状态是否成功,保证兜底下电 */
            ret1 = ipmEnsurePowerOff(ret1);	
            log_i("[ABUP-Procedure]: Install End Entry Power Off, [err:%d]", ret1);
			adm_ipm_remoutePowerOffEventReport(ret1);
            
#else 
            log_i("Simulate power-off succeed.");
            ret1 = ADM_ECODE_SUCCESSED;
            adm_ipm_remoutePowerOffEventReport(ret1);
#endif 
            ipmAddStageResult(f->state, ret1);
            f->state = ADM_IPM_FLASH_STATE_ENV_EXIT;
            log_i("[ABUP-Procedure]: Install End Post Upgrade, [err:%d]", ret);
            break;
        case ADM_IPM_FLASH_STATE_ENV_EXIT:              ///!< 环境退出
	        {
		        D32S result = ipmStepEnvExit(f);
		        if(ADM_ECODE_SUCCESSED != result){
                    ret = result;
					eventDesc = "Installation exit ota environment fail";
		        }
                ipmAddStageResult(f->state, result);
	        }
	        f->state = ADM_IPM_FLASH_STATE_REPORT;
            break;

        case ADM_IPM_FLASH_STATE_REPORT:
            f->result = ret;
	        adm_ips_stepPostInstall(ret);
	        // (void)ipmStepReport(f);
            D32S iSumResult = ADM_ECODE_FAILED;
            ipmHandleResult(f->report, &iSumResult);     ///< 通知HMI和cloud
		
            // 事件上报
		    if(ADM_ECODE_SUCCESSED == ret){
                ipmReportEventDependencyEventId("S_IE_001", NULL, ADM_ECODE_SUCCESSED);
		    }
			else{
				if(eventDesc == NULL){
                    ipmReportEventDependencyEventId("S_IE_002", eventDesc, ret);
				}
				else{
                    ipmReportEventDependencyEventId("S_IE_002", eventDesc, ADM_ECODE_SUCCESSED);
				}
			}
			
            // 日志文件上报
            log_i("[ABUP-Procedure]: Install Start report log file");
            ipmReportUpLogFilePostInstall(ret);
            log_i("[ABUP-Procedure]: Install End report log file");

            g_iProgress = 0;
            ipmAllStageResultClear(&g_stAllStageResult);     ///< 清空所有阶段结果记录
	        f->state = ADM_IPM_FLASH_STATE_IDLE;

            break;
			
        default:
            ret = ADM_ECODE_FAILED;
            f->state = ADM_IPM_FLASH_STATE_IDLE;
            break;
        }

        ipmFlashStore(f);
        log_i("[ABUP-Procedure] Install state goto :%s",  ipmFlashStateString(f->state));
    } while (f->state != ADM_IPM_FLASH_STATE_IDLE);

    /** 通知vdi退出安装,允许其他操作抢占件的通信链路 */
    // adm_ipm_notifyFlashState(ADM_DIM_SOA_VDI_FLASH_STATE_EXIT, ADM_IPM_DIM_REQUEST_TIMEOUT);

    return ret;
}

static void ipmReportProgressToEnverimentWait(ADM_IPM_FLASH_STATE_E eState, ADM_IPM_REPORT_HMI_T *h)
{
    pthread_t reportProgressThreadId;
    if (pthread_create(&reportProgressThreadId,NULL,ipmReportHmiProgressSimulate,(void*)h) == 0)
    {
        log_i("create thread success .");
        pthread_detach(reportProgressThreadId);
    }
    log_i("Start a thread to simulate progress");
}

static DBOOL ipmGetIsNeedTspCondCheck(ADM_APPLICATION_COMMON_CMDID_TYPE_E eCmdidType)
{
    D32S nRet = ADM_ECODE_FAILED;
    D64U uiTime = 0;
    DBOOL bIsNeedCheck = DFALSE;

    if (ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_SILENCE == eCmdidType ||
        ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_FORCE == eCmdidType)
    {
        /** 强制/静默任务可以带预约安装点,预约时间到达后判断是否符合tsp条件后再走安装流程 */
        nRet = adm_cvs_getSilenceUpgradeTime(&uiTime);
        if (ADM_IS_SUCCESS(nRet) && 0 < uiTime)
        {
            bIsNeedCheck = DTRUE;
        }
    }
    else if (ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_PROD_LINE == eCmdidType)
    {
        /** 产线任务,云端应捷途要求必须放开能手动/上电检测,股份需要兼容此种场景,安装前校验一次tsp条件 */
        bIsNeedCheck = DTRUE;
    }
    else 
    {
        bIsNeedCheck = DFALSE;
    }

    return bIsNeedCheck;
}

/**
 * @brief 加载待激活ECU信息
 *
 */
#define IPM_SMART_LITE_NUM 3
static DVOID ipmLoadActiveEcuInfo()
{
    D16U aInstanceId[IPM_SMART_LITE_NUM] = {adm_soa_getUclInstanceId(), adm_soa_getHostTboxInstanceId(), adm_soa_getUclAdccInstanceId()};

    adm_ips_loadActiveEcuInfo(aInstanceId, IPM_SMART_LITE_NUM);

    return;
}

static D32S ipmFlashRun(ADM_IPM_FLASH_T *f) {
    const DCHAR *eventDesc = NULL;
    D32S ret = ADM_ECODE_FAILED;
    D32S ret1 = ADM_ECODE_FAILED;
    D32S errorCode = ADM_ECODE_FAILED;
    D32S iErrorCode = ADM_ECODE_FAILED;
    D32S iPowerOffResult = 0;
    DBOOL bPowerOnSucceed = DFALSE;
    DBOOL bIsNeedCheck = DFALSE;
    ADM_APPLICATION_COMMON_CMDID_TYPE_E eCmdidType = ADM_APPLICATION_COMMON_CMDID_TYPE_INVALID;
    ADM_UIM_SOA_ACTION_AUTH_TYPE_E eActionType = ADM_UIM_SOA_ACTION_AUTH_TYPE_INVALID;
    ADM_APPLICATION_COMMON_CMDID_TYPE_E eTspTriggerType = ADM_APPLICATION_COMMON_CMDID_TYPE_INVALID;

    eCmdidType = (ADM_APPLICATION_COMMON_CMDID_TYPE_E)adm_ips_cmdIdGet();
    eTspTriggerType = adm_vsm_getTspTriggerType();

    log_d("state:%s", ipmFlashStateString(f->state));
    log_i("[ABUP-Procedure] Install entry state:%s",  ipmFlashStateString(f->state));

    /** 真正触发了升级，就取消预约，如果放在触发安装时清除会出现各种问题,比如远程上电失败后不清除就会有问题*/
    if (TRUE == adm_ipm_isScheduleInfoExist())
    {
        (DVOID)adm_ipm_uimSchduleInstallCallback(eCmdidType,
                                                  ADM_APPLICATION_SCHEDULE_INSTALL_HMI,
                                                  ADM_UIM_SOA_INSTALL_SCHDULE_TYPE_CANCELED,
                                                  0,
                                                  &eActionType,
                                                  NULL);
    }

    if (ADM_IPM_FLASH_STATE_IDLE == f->state)
    {
        /** 产线任务安装前需要重新判断车况条件是否满足  防止是上电检测/手动检测的方式绕开tsp条件判断*/
        bIsNeedCheck = ipmGetIsNeedTspCondCheck(eCmdidType);
        if (DTRUE == bIsNeedCheck)
        {
            ret = adm_ipm_stepVerifyConditionBeforeInstall(eCmdidType);
            if (ADM_IS_ERR(ret))
            {
                log_i("Install tsp condition verify failed.");
                if (ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_TSP == eTspTriggerType && ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_PROD_LINE == eCmdidType)
                {
                    // 退环境
                    adm_vsm_exitTspEnv();
                }
                return ret;
            }
        }
    }

    g_bIsSimulateFinished = DFALSE;

    /** 通知vdi进入安装,不允许其他操作抢占件的通信链路 */
    // adm_ipm_notifyFlashState(ADM_DIM_SOA_VDI_FLASH_STATE_ENTER, ADM_IPM_DIM_REQUEST_TIMEOUT);
    
    // ipmSetUpgradeMode();

    if (ADM_IPM_FLASH_STATE_UPGRADE <= f->state)
    {
        /** 断电重启或者车机激活重启后，需要同步hmi安装进度信息 */
        log_d("Force sync install progress.");
        ipmReportHmiProgress(&f->doFlash.report->reportHmi, DTRUE);
    }
    
    /** 安装中之前的状态，在恢复安装前，需要再次校验前置条件 */
    if ((ADM_IPM_FLASH_STATE_VERIFY_CONDITION < f->state) && (f->state <= ADM_IPM_FLASH_STATE_UPGRADE))
    {
        ret = ipmStepVerifyCondition(f);
        if (ADM_IS_ERR(ret))
        {
            log_e("[ABUP-Procedure]: Install Verify Conditions, [err:%d]", ret);
            ret = ADM_ECODE_INSTALL_GENERAL_FAILED;

            ipmAllStageResultClear(&g_stAllStageResult); ///< 清空所有阶段结果记录
            ipmAddStageResult(ADM_IPM_FLASH_STATE_VERIFY_CONDITION, ret);
            f->state = ADM_IPM_FLASH_STATE_REMOTE_AUTH_OFF;
        }
    }

    if (ADM_IPM_FLASH_STATE_VERIFT_SIGN_PACKAGE < f->state && f->state <= ADM_IPM_FLASH_STATE_UPGRADE &&
        g_iSimulateProgress < HMI_PROGRESS_SIMULATE)
    {
        /** 启动模拟通知进度线程 */
        ipmReportProgressToEnverimentWait(f->state,&f->doFlash.report->reportHmi);
    }

    /** 解决版本后校验后车机异常重启带来的云端上报结果误判 */
    if (f->state > ADM_IPM_FLASH_STATE_POST_UPGRADE)
    {
        ret = ipmGetInstallResult();
    }

    do
    {
        log_d("state:%s", ipmFlashStateString(f->state));
        log_i("[ABUP-Procedure] Install entry state:%s",  ipmFlashStateString(f->state));

        if (ADM_IPM_FLASH_STATE_UPGRADE < f->state && g_iSimulateProgress < HMI_PROGRESS_SIMULATE)
        {
            /** 升级完成后停止模拟进度(验签或者安装直接失败继续上报模拟进度的场景) 不计入KV */
            g_bIsSimulateFinished = DTRUE;
        }

        switch(f->state) {
        case ADM_IPM_FLASH_STATE_IDLE:
            ret = ipmStepInstallPrepare();
		    ipmReportTriggerInstallEventDependencyTriggerType((ADM_APPLICATION_COMMON_CMDID_TYPE_E)adm_ips_cmdIdGet());
	        if(ADM_ECODE_SUCCESSED == ret){
				f->state = ADM_IPM_FLASH_STATE_VERIFT_TASK;
            }else{
                ipmAddStageResult(f->state, ret);
		        f->state = ADM_IPM_FLASH_STATE_REPORT;
			    eventDesc = "Installation termination, Install Prepare error";
	        }
            break;

        case ADM_IPM_FLASH_STATE_VERIFT_TASK:
            if (ADM_ECODE_SUCCESSED == (ret = ipmStepVerifyTask(f)))
            {
                f->state = ADM_IPM_FLASH_STATE_ENV_ENTRY;
            }
            else
            {
                ipmAddStageResult(f->state, ret);
                f->state = ADM_IPM_FLASH_STATE_REPORT;
                eventDesc = "Installation termination, verify task fail";
            }
            break;

        case ADM_IPM_FLASH_STATE_ENV_ENTRY:
            if (ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_TSP != eTspTriggerType)
            {
                if (ADM_ECODE_SUCCESSED == (ret = ipmStepEnvEntry(f)))
                {
                    f->state = ADM_IPM_FLASH_STATE_REMOTE_AUTH_ON;
                }
                else
                {
                    ipmAddStageResult(f->state, ret);
                    ipmReportOtaServerTaskStatusEnterOtaModeFailed(&f->report->reportOtaServer);
                    f->state = ADM_IPM_FLASH_STATE_REPORT;
                    eventDesc = "Installation termination, Env Entry fail";
                }
            }
            else 
            {
                log_i("Tsp task default ota-mode set to success.");
                f->state = ADM_IPM_FLASH_STATE_REMOTE_AUTH_ON;
            }
            break;

        case ADM_IPM_FLASH_STATE_REMOTE_AUTH_ON:
            log_i("[ABUP-Procedure]: Install Start Entry Power On");
            if (ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_TSP != eTspTriggerType)
            {
#ifdef SUPPORT_REMOTE_AUTH
                ret = ipmControlRemotePower(ADM_DIM_SOA_HOST_OTA_TASK_TYPE_REMOTE_POWER_ON, &errorCode);
                if(ADM_ECODE_SUCCESSED == ret && errorCode == ADM_ECODE_SUCCESSED)
                {
                    /** 兜底PDCS件控制逻辑,上电20内必须要上高压操作，否则后续将无法进行上高压 */
                    ret = ipmSwitchHvOn();
                    if (ADM_IS_SUCCESS(ret))
                    {
                        bPowerOnSucceed = DTRUE;
                        f->state = ADM_IPM_FLASH_STATE_VERIFY_CONDITION;
                        adm_ipm_remoutePowerOnEventReport(ret);
                    }
                }

                if (DFALSE == bPowerOnSucceed)
                {
                    (DVOID)ipmSwitchHvOff();
                    ret = ADM_ECODE_INSTALL_REMOTE_POWER_ON_FAILED;
                    ipmAddStageResult(f->state, ret);
                    log_e("power on failed. ret[%d], errCode[%d]", ret, errorCode);
                    f->state = ADM_IPM_FLASH_STATE_ENV_EXIT;
                    eventDesc = "Installation termination, remote power on fail";
                    
                    ret = ADM_IS_ERR(ret) ? ret : errorCode;
                    adm_ipm_remoutePowerOnEventReport(ret);

                    /** 执行上电失败，需要执行下电 并且不计入总结果 */
                    log_d("Try to power off!");
                    iPowerOffResult = ipmControlRemotePower(ADM_DIM_SOA_HOST_OTA_TASK_TYPE_REMOTE_POWER_OFF, &errorCode);
                    iPowerOffResult = ADM_IS_ERR(iPowerOffResult) ? iPowerOffResult : errorCode;
                    adm_ipm_remoutePowerOffEventReport(iPowerOffResult);
                }
#else 
                log_i("Simulate power-on succeed.");
                ret = ADM_ECODE_SUCCESSED;
                adm_ipm_remoutePowerOnEventReport(ret);
                f->state = ADM_IPM_FLASH_STATE_VERIFY_CONDITION;
#endif    
            }
            else 
            {
                log_i("Tsp task default power-on set to success.");
                ret = ADM_ECODE_SUCCESSED;
                adm_ipm_remoutePowerOnEventReport(ret);
                f->state = ADM_IPM_FLASH_STATE_VERIFY_CONDITION;
            }
			log_i("[ABUP-Procedure]: Install End Entry Power On, [err:%d]", ret);
            break;

        case ADM_IPM_FLASH_STATE_VERIFY_CONDITION:
	        if(ADM_ECODE_SUCCESSED == (ret = ipmStepVerifyCondition(f))){
                f->state = ADM_IPM_FLASH_STATE_VERIFT_SIGN_PACKAGE;
            }else{
                ipmAddStageResult(f->state, ret);
                f->state = ADM_IPM_FLASH_STATE_REMOTE_AUTH_OFF;
				eventDesc = "Installation termination, Verify Condition fail";
	        }
            break;
			
        case ADM_IPM_FLASH_STATE_VERIFT_SIGN_PACKAGE:
            log_i("[ABUP-Procedure]: Install Start Verify Sign Package");
            ipmReportProgressToEnverimentWait(f->state,&f->doFlash.report->reportHmi);
	        if(ADM_ECODE_SUCCESSED == (ret = ipmStepVerifySignPackage())){
                f->state = ADM_IPM_FLASH_STATE_VERIFT_ECUS_VERSION;
            }else{
                ipmAddStageResult(f->state, ret);
                f->state = ADM_IPM_FLASH_STATE_REMOTE_AUTH_OFF;
				eventDesc = "Installation termination, verify sign fail";
	        }
            log_i("[ABUP-Procedure]: Install End Verify Sign Package, [err:%d]", ret);
            break;

        case ADM_IPM_FLASH_STATE_VERIFT_ECUS_VERSION:
            log_i("[ABUP-Procedure]: Install Start Verify Ecus version");
            if (ADM_ECODE_SUCCESSED == (ret = ipmStepVerifyEcusVersion(f)))
            {
                f->state = ADM_IPM_FLASH_STATE_UPGRADE;
                adm_ips_clearActiveEcuInfo(); ///< 清空智能件激活相关信息
                ipmLoadActiveEcuInfo();       ///< 加载待激活的智能件信息

                if (TRUE == adm_cfg_iniParserGetBoolean(ADM_CFG_VUS_IS_DETACH_RETRY, 
                                                        ADM_CFG_VUS_IS_DETACH_RETRY_DEFAULT_VAL))
                {
                    f->eStage = IPM_FLASH_STAGE_DEFAULT;
                }
            }
            else
            {
                ipmAddStageResult(f->state, ret);
                f->state = ADM_IPM_FLASH_STATE_REMOTE_AUTH_OFF;
                eventDesc = "Installation termination, verify ecu version fail";
            }
            log_i("[ABUP-Procedure]: Install End Verify Ecus version, [err:%d]", ret);
            break;

        case ADM_IPM_FLASH_STATE_UPGRADE:
            if (TRUE == adm_cfg_iniParserGetBoolean(ADM_CFG_VUS_IS_DETACH_RETRY,
                                                    ADM_CFG_VUS_IS_DETACH_RETRY_DEFAULT_VAL))
            {
                ret = ipmStepUpgrade(f);
            }
            else
            {
                ret = ipmDoFlashEcusExec(f);
            }

            ipmAddStageResult(f->state, ret);

			ipmUpgradeResultHandler(f,ret);

            break;

        case ADM_IPM_FLASH_STATE_ACTIVE:
            ipmIntelEcuActive(&f->doFlash);
            f->state = ADM_IPM_FLASH_STATE_POST_UPGRADE;
            break;

		case ADM_IPM_FLASH_STATE_POST_UPGRADE:
            log_i("[ABUP-Procedure]: Install Start Post Upgrade");
            ret = ipmStepUpgradePost(f);
            if (DTRUE == g_stAllStageResult.bIsFlashSuccess && ADM_ECODE_SUCCESSED != ret)
            {
                /** 刷写成功 && 后校验失败 */
                ipmAddStageResult(f->state, ret);
            }
            f->state = ADM_IPM_FLASH_STATE_REMOTE_AUTH_OFF;
            log_i("[ABUP-Procedure]: Install End Post Upgrade, [err:%d]", ret);
            break;
		 
        case ADM_IPM_FLASH_STATE_REMOTE_AUTH_OFF:
            /** 增加清理故障码的逻辑 不考虑超时,请求给到qnx从控即可*/
            (DVOID)adm_dim_vdi_clearDtc(adm_soa_getVdiInstanceId(), ADM_IPM_DIM_REQUEST_TIMEOUT, &iErrorCode);

            ipmHandleStageResult(f->report);     ///< 通知HMI下电弹框
            
            /** 下电前保持当前是下高压状态 */
            ret1 = ipmSwitchHvOff();
            log_i("Switch to hv off, [err:%d]", ret1);
#ifdef SUPPORT_REMOTE_AUTH
            sleep(5);       ///!< 等待5s
			log_i("[ABUP-Procedure]: Install Start Entry Power Off");
            ret1 = ipmControlRemotePower(ADM_DIM_SOA_HOST_OTA_TASK_TYPE_REMOTE_POWER_OFF, &errorCode);

            ret1 = ADM_IS_ERR(ret1) ? ret1 : errorCode;

            /** 强制校验当前下电状态是否成功,保证兜底下电 */
            ret1 = ipmEnsurePowerOff(ret1);
            log_i("[ABUP-Procedure]: Install End Entry Power Off, [err:%d]", ret1);
			adm_ipm_remoutePowerOffEventReport(ret1);
#else 
            log_i("Simulate power-off succeed.");
            ret1 = ADM_ECODE_SUCCESSED;
            adm_ipm_remoutePowerOffEventReport(ret1);
#endif 
            ipmAddStageResult(f->state, ret1);
            f->state = ADM_IPM_FLASH_STATE_ENV_EXIT;
            break;

        case ADM_IPM_FLASH_STATE_ENV_EXIT:
	        {
		        D32S result = ipmStepEnvExit(f);
		        if(ADM_ECODE_SUCCESSED != result){
                    ret = result;
					eventDesc = "Installation exit ota environment fail";
		        }
	        
                ipmAddStageResult(f->state, result);
            }
            f->state = ADM_IPM_FLASH_STATE_REPORT;
            break;			
        case ADM_IPM_FLASH_STATE_REPORT:
            f->result = ret;
	        adm_ips_stepPostInstall(ret);
            if (ADM_IS_SUCCESS(ret))
            {
                log_i("[ABUP-Procedure]: Install Start Upgrade big version & display version");
                ipmUpdateVehicleBigVersion();
                ipmUpdateVehicleVersionNumber();
                log_i("[ABUP-Procedure]: Install End Upgrade big version & display version");
            }
	        // (void)ipmStepReport(f);
            D32S iSumResult = ADM_ECODE_FAILED;
            ipmHandleResult(f->report, &iSumResult);     ///< 通知HMI和cloud
		
            if (ADM_IS_ERR(f->result))
            {
                ipmReportOtaServerTaskStatusTaskStop(&f->report->reportOtaServer, f->result);
            }

            // 事件上报
		    if(ADM_ECODE_SUCCESSED == ret){
                ipmReportEventDependencyEventId("S_IE_001", NULL, ADM_ECODE_SUCCESSED);
		    }
			else{
				if(eventDesc == NULL){
                    ipmReportEventDependencyEventId("S_IE_002", eventDesc, ret);
				}
				else{
                    ipmReportEventDependencyEventId("S_IE_002", eventDesc, ADM_ECODE_SUCCESSED);
				}
			}
			
            log_i("[ABUP-Procedure]: Install Start async version number");
            /** 升级完成后执行零件信息同步 */
            adm_cvm_asyncVersionNumber(ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_USER, NULL);
            log_i("[ABUP-Procedure]: Install End async version number");

            // 日志文件上报
            log_i("[ABUP-Procedure]: Install Start report log file");
            ipmReportUpLogFilePostInstall(ret);
            log_i("[ABUP-Procedure]: Install End report log file");
            g_iProgress = 0;
            ipmAllStageResultClear(&g_stAllStageResult);     ///< 清空所有阶段结果记录
            f->state = ADM_IPM_FLASH_STATE_IDLE;

            break; 

        default:
            ret = ADM_ECODE_FAILED;
            f->state = ADM_IPM_FLASH_STATE_IDLE;
            break;
			
        }

	    ipmFlashStore(f);
        log_i("[ABUP-Procedure] Install state goto :%s",  ipmFlashStateString(f->state));
    }while(f->state != ADM_IPM_FLASH_STATE_IDLE);

    /** 通知vdi退出安装,允许其他操作抢占件的通信链路 */
    // adm_ipm_notifyFlashState(ADM_DIM_SOA_VDI_FLASH_STATE_EXIT, ADM_IPM_DIM_REQUEST_TIMEOUT);

    return ret;
}

static struct json_object *ipmFlashJsonCreate(const ADM_IPM_FLASH_T *f)
{
    struct json_object *root = json_object_new_object();
    if (NULL != root)
    {
        struct json_object *stateVal = json_object_new_int((int32_t)f->state);
        if (NULL != stateVal)
        {
            (void)json_object_object_add(root, "state", stateVal);
        }

        struct json_object *stageVal = json_object_new_int((int32_t)f->eStage);
        if (NULL != stageVal)
        {
            (void)json_object_object_add(root, "stage", stageVal);
        }
    }
    return root;
}
static void ipmFlashJsonParse(ADM_IPM_FLASH_T *f, const struct json_object *root)
{
    struct json_object *stateVal = json_object_object_get(root, "state");
    if (NULL != stateVal)
    {
        f->state = (ADM_IPM_FLASH_STATE_E)json_object_get_int(stateVal);
    }

    struct json_object *stageVal = json_object_object_get(root, "stage");
    if (NULL != stageVal)
    {
        f->eStage = (ADM_IPM_FLASH_STAGE_E)json_object_get_int(stageVal);
    }

    return;
}
static void ipmFlashLoad(ADM_IPM_FLASH_T *f){
    ADM_KV_STORAGE_VALUE_TYPE_E eType = ADM_KV_STORAGE_VALUE_TYPE_INVALID;
    DCHAR *flashJsonStr = NULL;
    size_t flashJsonStrLen = 0;
    if(adm_com_kvstorage_get_string(adm_core_service_getBusinessDatabaseDb(), ADM_IPM_FLASH_KV_KEY,
                &flashJsonStr, &flashJsonStrLen, &eType)){
	if(NULL != flashJsonStr){
	    struct json_object* stateVal = json_tokener_parse(flashJsonStr);
	    if(NULL != stateVal){
		ipmFlashJsonParse(f, stateVal);
		(void)json_object_put(stateVal);
	    }
	    adm_free(flashJsonStr);
	}
    }
}
static void ipmFlashStore(const ADM_IPM_FLASH_T *f){
    struct json_object* root = ipmFlashJsonCreate(f);
    if(NULL != root){
	const DCHAR *flashJsonStr = json_object_to_json_string(root);
	if(NULL != flashJsonStr){
	    (void)adm_com_kvstorage_set_string(adm_core_service_getBusinessDatabaseDb(), ADM_IPM_FLASH_KV_KEY,
                flashJsonStr, ADM_KV_STORAGE_VALUE_TYPE_STRING);
	}
	(void)json_object_put(root);
    }
}

static void ipmFlashClear(ADM_IPM_FLASH_T* f)
{
    ipmDoFlashClear(&f->doFlash);
}

static struct json_object *ipmAllStageResultJsonCreate(const ADM_ALL_STAGE_RESULT_INFO_T* pstAllStageResult)
{
    json_object *root = json_object_new_object();
    json_object *allStageResult = json_object_new_array();
    struct json_object* strJson = NULL;

    if (NULL != root)
    {
        struct json_object *indexVal = json_object_new_uint64((uint64_t)pstAllStageResult->uiIndex);
        if (NULL != indexVal)
        {
            (void)json_object_object_add(root, "index", indexVal);
        }

        struct json_object *isFlashSuccVal = json_object_new_int((D32S)pstAllStageResult->bIsFlashSuccess);
        if (NULL != isFlashSuccVal)
        {
            (void)json_object_object_add(root, "flashResult", isFlashSuccVal);
        }

        if (pstAllStageResult->uiIndex > 0)
        {
            for (D16U i = 0; i < pstAllStageResult->uiIndex; i++)
            {
                json_object *eachStageResult = json_object_new_object();
                strJson = json_object_new_int((D32S)pstAllStageResult->stAllStageResult[i].eState);
                if(strJson != NULL)
                {
                    (void)json_object_object_add(eachStageResult, "state", strJson);
                }

                strJson = json_object_new_int(pstAllStageResult->stAllStageResult[i].iRet);
                if(strJson != NULL)
                {
                    (void)json_object_object_add(eachStageResult, "result", strJson);
                }

                (void)json_object_array_add(allStageResult, eachStageResult);
            }

            (void)json_object_object_add(root, "allStageResult", allStageResult);
        }
    }

    return root;
}

static void ipmAllStageResultJsonParse(ADM_ALL_STAGE_RESULT_INFO_T* pstAllStageResult, const struct json_object *root)
{
    json_object *indexVal = json_object_object_get(root, "index");
    if (NULL != indexVal)
    {
        pstAllStageResult->uiIndex = json_object_get_uint64(indexVal);
    }

    json_object *isFlashSuccVal = json_object_object_get(root, "flashResult");
    if (NULL != isFlashSuccVal)
    {
        pstAllStageResult->bIsFlashSuccess = (DBOOL)json_object_get_int(isFlashSuccVal);
    }

    if (0 == pstAllStageResult->uiIndex)
    {
        return;
    }

    json_object *allStageResultVal = json_object_object_get(root, "allStageResult");
    if (NULL != allStageResultVal)
    {
        D32S iNum = json_object_array_length(allStageResultVal);
        for (D32S i = 0; i < iNum; i++)
        {
            json_object * child_obj = json_object_array_get_idx(allStageResultVal, i);

            pstAllStageResult->stAllStageResult[i].eState = 
            (ADM_IPM_FLASH_STATE_E)json_object_get_int(json_object_object_get(child_obj, "state"));
            pstAllStageResult->stAllStageResult[i].iRet = 
            json_object_get_int(json_object_object_get(child_obj, "result"));
        }
    }

    return;
}

static void ipmAllStageResultLoad(ADM_ALL_STAGE_RESULT_INFO_T* pstAllStageResult)
{
    ADM_KV_STORAGE_VALUE_TYPE_E eType = ADM_KV_STORAGE_VALUE_TYPE_INVALID;
    DCHAR *flashJsonStr = NULL;
    size_t flashJsonStrLen = 0;
    if(adm_com_kvstorage_get_string(adm_core_service_getBusinessDatabaseDb(), ADM_IPM_ALL_STAGE_RESULT_KV_KEY,
                &flashJsonStr, &flashJsonStrLen, &eType))
    {
        if(NULL != flashJsonStr)
        {
            struct json_object* stateVal = json_tokener_parse(flashJsonStr);
            if(NULL != stateVal){
            ipmAllStageResultJsonParse(pstAllStageResult, stateVal);
            (void)json_object_put(stateVal);
            }
            adm_free(flashJsonStr);
        }
    }

    return;
}
static void ipmAllStageResultStore(const ADM_ALL_STAGE_RESULT_INFO_T* pstAllStageResult)
{
    struct json_object* root = ipmAllStageResultJsonCreate(pstAllStageResult);
    if(NULL != root)
    {
        const DCHAR *flashJsonStr = json_object_to_json_string(root);
        if(NULL != flashJsonStr)
        {
            log_i("ipmAllStageResultStore str[%s]", flashJsonStr);
            (void)adm_com_kvstorage_set_string(adm_core_service_getBusinessDatabaseDb(), ADM_IPM_ALL_STAGE_RESULT_KV_KEY,
                    flashJsonStr, ADM_KV_STORAGE_VALUE_TYPE_STRING);
	    }
	    (void)json_object_put(root);
    }

    return;
}

static void ipmAllStageResultClear(ADM_ALL_STAGE_RESULT_INFO_T* pstAllStageResult)
{
    ipmAllStageResultMemoryInit(pstAllStageResult);

    if(TRUE == adm_com_kvstorage_has_key(adm_core_service_getBusinessDatabaseDb(), ADM_IPM_ALL_STAGE_RESULT_KV_KEY))
	{
		adm_com_kvstorage_remove_key(adm_core_service_getBusinessDatabaseDb(), ADM_IPM_ALL_STAGE_RESULT_KV_KEY);	
	}	

    return;
}

static void ipmNeedReNotiPolicyResultStore(D32S iSumResult)
{
    ADM_APPLICATION_COMMON_CMDID_TYPE_E eCmdidType = uimSoaCmdidTypeGet();

    if (ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_SILENCE == eCmdidType
        || ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_APK_SELF_UPGRADE == eCmdidType
        || ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_FORCE == eCmdidType)
    {
        return;/**静默升级记录HMI升级结果*/
    }

    json_object *needNotifyJson = json_object_new_object();
    if(NULL != needNotifyJson)
    {
        (void)json_object_object_add(needNotifyJson, "result", json_object_new_int(iSumResult));
        const DCHAR *flashJsonStr = json_object_to_json_string(needNotifyJson);
        (void)adm_com_kvstorage_set_string(adm_core_service_getBusinessDatabaseDb(), ADM_IPM_NEED_REPEAT_NOTIFY_POLICY_RESULT,
                    flashJsonStr, ADM_KV_STORAGE_VALUE_TYPE_STRING);
	    (void)json_object_put(needNotifyJson);
    }

    return;
}

/* ADM_IPM_FLASH_STATE_E */
static const DCHAR *ipmFlashStateString(ADM_IPM_FLASH_STATE_E state){
    const DCHAR *strState = "ADM_IPM_FLASH_STATE_UNKNOWN";
    switch(state){
	case ADM_IPM_FLASH_STATE_IDLE: strState = ADM_STR_FROM_PARAM(ADM_IPM_FLASH_STATE_IDLE);break;
	case ADM_IPM_FLASH_STATE_VERIFY_CONDITION: strState = ADM_STR_FROM_PARAM(ADM_IPM_FLASH_STATE_VERIFY_CONDITION);break;
	case ADM_IPM_FLASH_STATE_ENV_ENTRY: strState = ADM_STR_FROM_PARAM(ADM_IPM_FLASH_STATE_ENV_ENTRY);break;
	case ADM_IPM_FLASH_STATE_VERIFT_TASK: strState = ADM_STR_FROM_PARAM(ADM_IPM_FLASH_STATE_VERIFT_TASK);break;
    case ADM_IPM_FLASH_STATE_REMOTE_AUTH_ON:strState = ADM_STR_FROM_PARAM(ADM_IPM_FLASH_STATE_REMOTE_AUTH_ON);break;
	case ADM_IPM_FLASH_STATE_VERIFT_SIGN_PACKAGE: strState = ADM_STR_FROM_PARAM(ADM_IPM_FLASH_STATE_VERIFT_SIGN_PACKAGE);break;
	case ADM_IPM_FLASH_STATE_VERIFT_ECUS_VERSION: strState = ADM_STR_FROM_PARAM(ADM_IPM_FLASH_STATE_VERIFT_ECUS_VERSION);break;
	case ADM_IPM_FLASH_STATE_UPGRADE: strState = ADM_STR_FROM_PARAM(ADM_IPM_FLASH_STATE_UPGRADE);break;
    case ADM_IPM_FLASH_STATE_ACTIVE: strState = ADM_STR_FROM_PARAM(ADM_IPM_FLASH_STATE_ACTIVE);break;
	case ADM_IPM_FLASH_STATE_POST_UPGRADE: strState = ADM_STR_FROM_PARAM(ADM_IPM_FLASH_STATE_POST_UPGRADE);break;
	case ADM_IPM_FLASH_STATE_ENV_EXIT: strState = ADM_STR_FROM_PARAM(ADM_IPM_FLASH_STATE_ENV_EXIT);break;
	case ADM_IPM_FLASH_STATE_REPORT: strState = ADM_STR_FROM_PARAM(ADM_IPM_FLASH_STATE_REPORT);break;
    case ADM_IPM_FLASH_STATE_REMOTE_AUTH_OFF:strState = ADM_STR_FROM_PARAM(ADM_IPM_FLASH_STATE_REMOTE_AUTH_OFF);break;
	default:break;
    }
    return strState;
}

/* ADM_IPM_DO_FLASH_T */
static void ipmDoFlashInit(ADM_IPM_DO_FLASH_T *f, ADM_IPM_DIM_T *dim, ADM_IPM_REPORT_T *report){
    (void)pthread_mutex_init(&f->mutex, NULL);
    INIT_LIST_HEAD(&f->head);
    f->ecusCount = 0;
    f->isNeedGetEcus = FALSE;
    f->dim = dim;
    f->report = report;
    f->ecuListStoreLastTime = ipmMonotonicTimeMSecGet();

    ipmDoFlashEcusListLoad(f);
    ipmDoFlashEcusListStateUpdate(f);
    ipmDoFlashEcusListAutoFill(f);
}
static void ipmDoFlashDeInit(ADM_IPM_DO_FLASH_T *f){
    (void)pthread_mutex_destroy(&f->mutex);
}

static void ipmDoFlashClear(ADM_IPM_DO_FLASH_T* f)
{
    ADM_IPM_ECU_INFO_NODE_T* pos = NULL;
    ADM_IPM_ECU_INFO_NODE_T* node = NULL;
    ADM_IPM_ECU_INFO_NODE_T* tmp = NULL;

    list_for_each_entry_safe(pos, tmp, &f->head, ADM_IPM_ECU_INFO_NODE_T, entry)
    {
        node = pos;
        list_del(&node->entry);
        ipmEcuInfoNodeDel(node);
    }
    f->ecusCount = 0;
}

static D32S ipmDoFlashEcusPost(ADM_IPM_DO_FLASH_T *f){
    D32S ret = ADM_ECODE_SUCCESSED;
    ADM_ECS_ECUCONFIG_T* ecuCfg = NULL;
    DBOOL bIsAllSameHvOnError = DTRUE;
    DBOOL bIsAllSameHvOffError = DTRUE;
	
    ADM_IPS_PROGRESS_T progress;
    adm_ips_progressInit(&progress);
    adm_ips_progressGet(&progress);

    if(0 != progress.ecus.ecusLen && NULL != progress.ecus.ecus){
#if 0
        DBOOL ecuFaild = 0;
        for (D32S i = 0; i < progress.ecus.ecusLen; ++i) {
            const ADM_IPS_ECU_PROGRESS_T* p = &progress.ecus.ecus[i];
            D32S result = p->progress[ADM_IPS_ECU_PHASE_INDEX_UPGRADE].result;
            if (ADM_IPS_ECU_PHASE_POST_INSTALL == p->state && ADM_ECODE_SUCCESSED == result) {}
            else {
                ecuFaild = 1;
                if (ADM_IPS_ECU_PHASE_POST_INSTALL == p->state
                    || ADM_IPS_ECU_PHASE_TIMELY_PRE_INSTALL_ROLLBACK == p->state
                    || ADM_IPS_ECU_PHASE_PRE_INSTALL_ROLLBACK == p->state
                    || ADM_IPS_ECU_PHASE_INSTALL_ROLLBACK == p->state
                    || ADM_IPS_ECU_PHASE_POST_INSTALL_ROLLBACK == p->state
                    ) {
                    if (ADM_ECODE_SUCCESSED != result) {
                        ret = result;
                        /* TODO cal broken result and cal dim broken for hengda */
                    }
                }
            }
            /* TODO dim flash ecu post. clear inval package or clear install success record. */
        }
        if (0 == ecuFaild) {
            ret = ADM_ECODE_SUCCESSED;
        }
#endif // 0

	    for(D32S i=0; i<progress.ecus.ecusLen; ++i){
	        const ADM_IPS_ECU_PROGRESS_T *p = &progress.ecus.ecus[i];
	        D32S result = p->progress[ADM_IPS_ECU_PHASE_INDEX_UPGRADE].result;

            /* 现在零件升级错误码不能区分是刷写失败还是升级失败，默认认为是刷写失败 */
            if (ADM_ECODE_SUCCESSED != result) {
                /** 记录是否所有组都是上高压后者下高压失败错误 */
                if (ADM_ECODE_INSTALL_ENTER_HIGH_VOLTAGE_FAILED != result)
                {
                    bIsAllSameHvOnError = DFALSE;
                }

                if (ADM_ECODE_INSTALL_EXIT_HIGH_VOLTAGE_FAILED != result)
                {
                    bIsAllSameHvOffError = DFALSE;
                }

                /* 只要有一个重要零件升级失败，错误码就为[重要零件刷写失败]，否则为[零件刷写失败] */
                if (ADM_ECODE_INSTALL_CRITICAL_ECU_FLASH_FAILED != ret) {
                    ecuCfg = NULL;
                    ret = ADM_ECODE_INSTALL_ECU_FLASH_FAILED;

                    result = adm_ecs_getEcuConfig(p->ecuReqId, p->ecuSwDid, &ecuCfg);
                    if (ADM_ECODE_SUCCESSED == result) {
                        if (1 == ecuCfg->securityLevel) {
                            ret = ADM_ECODE_INSTALL_CRITICAL_ECU_FLASH_FAILED;
                        }

                        adm_ecs_delEcuConfigInfo(&ecuCfg);
                    }
                    else {
                        log_e("Failed to get ecs ecu config, ecuReqId[%s] ecuSwDid[%s] err[%d]", p->ecuReqId, p->ecuSwDid, result);
                    }
                }
            }
            else {
                bIsAllSameHvOnError = DFALSE;
                bIsAllSameHvOffError = DFALSE;
                /* 升级成功并回滚成功，错误码就为[已升级零件回滚成功] */
                if (ADM_IPS_ECU_PHASE_POST_INSTALL_ROLLBACK == p->state && 
                    ADM_ECODE_SUCCESSED == p->progress[ADM_IPS_ECU_PHASE_INDEX_ROLLBACK].result &&
                    /* 只要有一个重要零件升级失败，错误码就为[重要零件刷写失败] */
                    ADM_ECODE_INSTALL_CRITICAL_ECU_FLASH_FAILED != ret) { 
                    ret = ADM_ECODE_INSTALL_ROLLBACK_SUCCESSED;
                }
                else {
					  /* 其他情况下，该ecu升级成功，则不改变整体返回结果（所有都成功，则返回成功，任意一个ecu升级失败，则返回失败 */
//                    ret = ADM_ECODE_FAILED;
                }
            }

	        /* TODO dim flash ecu post. clear inval package or clear install success record. */
	    }
    }

    ipmStatEventReportEnd(&f->report->reportStatEvent);

    adm_ips_progressDeInit(&progress);
    // ADM_DEBUG_RETURN_D32S(ret);

    if (DTRUE == bIsAllSameHvOnError)
    {
        log_i("All ecu enter high valtage failed.");
        ret = ADM_ECODE_INSTALL_ENTER_HIGH_VOLTAGE_FAILED;
    }
    if (DTRUE == bIsAllSameHvOffError)
    {
        log_i("All ecu exit high valtage failed.");
        ret = ADM_ECODE_INSTALL_EXIT_HIGH_VOLTAGE_FAILED;
    }

    return ret;
}

static D32S ipmWaitUclConnect(D16U uiInstanceId)
{
    D32S iRet = ADM_ECODE_FAILED;
    D32S iErrCode = ADM_ECODE_FAILED; 
    D32S iWaitCount = 0;
    DCHAR* pSdkVersion = NULL;

    do
    {
        iRet = adm_dim_ucl_getSdkVersion(uiInstanceId, 5, &pSdkVersion, &iErrCode);
        if (ADM_ECODE_SUCCESSED == iRet && NULL != pSdkVersion)
        {
            log_d("get instanceId[%u] ucl sdk version[%s]", uiInstanceId, pSdkVersion);
            adm_free(pSdkVersion);
            break;
        }

        /** 2分钟仍获取版本失败，等待超时退出 */
        if (iWaitCount >= 60)
        {
            log_e("get instanceId[%u] ucl sdk version failed", uiInstanceId);
            iRet = ADM_ECODE_FAILED;
            break;
        }

        iWaitCount++;

        sleep(3);
    } while (TRUE);

    return iRet;
}
static DVOID ipmHandleActivePre(ADM_IPM_DO_FLASH_T *f, const ADM_IPS_UCL_ECU_ARRAY_ACTIVE_T *pActiveInfo)
{
    ADM_IPS_PROGRESS_T progress;
    DCHAR* pIccRequestId = NULL;
    DCHAR* pIccSwdid = NULL;
    DCHAR* pInstallInstanceId = NULL;
    D16U uclInstanceId = 0;    ///< 主控的从控实例ID
    D16U tboxInstanceId = 0;   ///< tbox实例ID
    D16U adccInstanceId = 0;   ///< adcc实例ID
    DBOOL bIsMasterDeviceActive = DFALSE;
    DBOOL bIsTboxActive = DFALSE;
    DBOOL bIsAdccActive = DFALSE;
    D32S iErrcode = 0;
    ADM_IPS_ECU_PHASE_E eEcuUpgradeState = ADM_IPS_ECU_PHASE_INVALID;
    D32S iUpgradeResult = ADM_ECODE_FAILED;

    uclInstanceId = adm_soa_getUclInstanceId();
    tboxInstanceId = adm_soa_getHostTboxInstanceId();
    adccInstanceId = adm_soa_getUclAdccInstanceId();
    log_i("Active start, Ucl instanceId:0x%x, tboxInstanceId:0x%x, adccInstanceId:0x%x", uclInstanceId, tboxInstanceId, adccInstanceId);

    if (NULL == pActiveInfo)
    {
        log_w("Invalid param.");
        return;
    }

    adm_ips_progressInit(&progress);
    adm_ips_progressGet(&progress);

    if (0 == progress.ecus.ecusLen || NULL == progress.ecus.ecus)
    {
        log_w("Get no upgrade ecu info.");
        adm_ips_updateAllActiveState(ADM_IPS_ACTIVE_END);
        adm_ips_progressDeInit(&progress);
        return;
    }

    for (D32U i = 0; i < pActiveInfo->uiEcuLen; i++)
    {
        if (NULL == pActiveInfo->pstActiveEcuInfo)
        {
            continue;
        }

        iUpgradeResult = adm_ips_progressGetUpgradeResult(&progress, &eEcuUpgradeState,
                                                            pActiveInfo->pstActiveEcuInfo[i].pRequestId, pActiveInfo->pstActiveEcuInfo[i].pSwdid);
        /** 只有升级成功的单独激活件才需要激活 */
        if (ADM_IPS_ECU_PHASE_POST_INSTALL == eEcuUpgradeState && ADM_ECODE_SUCCESSED == iUpgradeResult)
        {
            if (ADM_IPS_ACTIVE_PRE == pActiveInfo->pstActiveEcuInfo[i].etate)
            {
                pInstallInstanceId = pActiveInfo->pstActiveEcuInfo[i].pInstallInstanceId;
                if (ipmHtoi(pInstallInstanceId) == uclInstanceId)
                {
                    pIccRequestId = pActiveInfo->pstActiveEcuInfo[i].pRequestId;
                    pIccSwdid = pActiveInfo->pstActiveEcuInfo[i].pSwdid;
                    bIsMasterDeviceActive = DTRUE;
                }
                else if (ipmHtoi(pInstallInstanceId) == tboxInstanceId ||
                        ipmHtoi(pInstallInstanceId) == adccInstanceId)
                {
                    if (ipmHtoi(pInstallInstanceId) == tboxInstanceId)
                    {
                        bIsTboxActive = DTRUE;
                    }
                    else
                    {
                        bIsAdccActive = DTRUE;
                    }
                    log_i("%s:%s, %s:%s, %s:%u",
                        ADM_STR_FROM_PARAM(ecuReqId), ADM_STR_SAFE_OUTPUT(pActiveInfo->pstActiveEcuInfo[i].pRequestId),
                        ADM_STR_FROM_PARAM(ecuSwDid), ADM_STR_SAFE_OUTPUT(pActiveInfo->pstActiveEcuInfo[i].pSwdid),
                        ADM_STR_FROM_PARAM(instanceId), (D32U)ipmHtoi(pInstallInstanceId));
                    log_i("[ABUP-Procedure]: Install Start Tbox/Adcc Reboot.");
                    /** 单独激活 */
                    adm_ips_updateEcuActiveState(ADM_IPS_ACTIVE_ING, pActiveInfo->pstActiveEcuInfo[i].pRequestId, pActiveInfo->pstActiveEcuInfo[i].pSwdid);
                    (DVOID)adm_dim_ucl_requestUpdateAndReboot(ipmHtoi(pInstallInstanceId), f->dim->requestTimeout, &iErrcode);
                    log_i("[ABUP-Procedure]: Install End Tbox/Adcc Reboot.");
                }
                else
                {
                    /** 如果有非智能件单独激活的默认成功 */
                    log_w("Invalid active instanceId[%s]", pInstallInstanceId);
                    adm_ips_updateEcuActiveState(ADM_IPS_ACTIVE_END, pActiveInfo->pstActiveEcuInfo[i].pRequestId, pActiveInfo->pstActiveEcuInfo[i].pSwdid);
                    adm_ips_updateEcuActiveResult(ADM_ECODE_SUCCESSED, pActiveInfo->pstActiveEcuInfo[i].pRequestId, pActiveInfo->pstActiveEcuInfo[i].pSwdid);
                    continue;
                }
            }
        }
        else 
        {
            /** 非升级成功的件默认激活完成，不需要激活 */
            log_i("Find ecu requestId[%s] swDid[%s] upgrade state[%d] result[%d].", pActiveInfo->pstActiveEcuInfo[i].pRequestId, pActiveInfo->pstActiveEcuInfo[i].pSwdid, (D32S)eEcuUpgradeState, iUpgradeResult);
            adm_ips_updateEcuActiveState(ADM_IPS_ACTIVE_END, pActiveInfo->pstActiveEcuInfo[i].pRequestId, pActiveInfo->pstActiveEcuInfo[i].pSwdid);
            adm_ips_updateEcuActiveResult(ADM_ECODE_SUCCESSED, pActiveInfo->pstActiveEcuInfo[i].pRequestId, pActiveInfo->pstActiveEcuInfo[i].pSwdid);
        }
    }

    if (DTRUE == bIsMasterDeviceActive)
    {
        log_i("[ABUP-Procedure]: Install Start Icc Reboot.");
        adm_ips_updateEcuActiveState(ADM_IPS_ACTIVE_ING, pIccRequestId, pIccSwdid);
        sleep(3);       ///< 兼容磁盘文件无法实时落盘
        (DVOID)adm_dim_ucl_requestUpdateAndReboot(uclInstanceId, f->dim->requestTimeout, &iErrcode);
        log_i("[ABUP-Procedure]: Install End Icc Reboot.");
    }
    else 
    {
        log_i("Icc do not need to be actived.");
    }

    /** 内存释放 */
    adm_ips_progressDeInit(&progress);

    return;
}

static DVOID ipmHandleActiveIng(const ADM_IPS_UCL_ECU_ARRAY_ACTIVE_T *pActiveInfo)
{
    D32U iWaitCnt = 0;

    if (NULL == pActiveInfo || NULL == pActiveInfo->pstActiveEcuInfo)
    {
        log_w("Invalid param.");
        return;
    }

    for (D32U i = 0; i < pActiveInfo->uiEcuLen; i++)
    {
        /** 等待激活结果，每个智能件最大超时时间5分钟 */
        if (ADM_IPS_ACTIVE_ING == pActiveInfo->pstActiveEcuInfo[i].etate)
        {
            do
            {
                iWaitCnt++;
                sleep(5);
                if (ADM_IPS_ACTIVE_ING != pActiveInfo->pstActiveEcuInfo[i].etate)
                {
                    break;
                }
                if (0 == iWaitCnt % 10)
                {
                    log_i("Wait for %s:%s, %s:%s, %s:%s active result...",
                          ADM_STR_FROM_PARAM(ecuReqId), ADM_STR_SAFE_OUTPUT(pActiveInfo->pstActiveEcuInfo[i].pRequestId),
                          ADM_STR_FROM_PARAM(ecuSwDid), ADM_STR_SAFE_OUTPUT(pActiveInfo->pstActiveEcuInfo[i].pSwdid),
                          ADM_STR_FROM_PARAM(instanceId), ADM_STR_SAFE_OUTPUT(pActiveInfo->pstActiveEcuInfo[i].pInstallInstanceId));
                }
            } while (iWaitCnt < 60);

            if (iWaitCnt >= 60)
            {
                log_e("Wait for %s:%s, %s:%s, %s:%s active result timedout.",
                      ADM_STR_FROM_PARAM(ecuReqId), ADM_STR_SAFE_OUTPUT(pActiveInfo->pstActiveEcuInfo[i].pRequestId),
                      ADM_STR_FROM_PARAM(ecuSwDid), ADM_STR_SAFE_OUTPUT(pActiveInfo->pstActiveEcuInfo[i].pSwdid),
                      ADM_STR_FROM_PARAM(instanceId), ADM_STR_SAFE_OUTPUT(pActiveInfo->pstActiveEcuInfo[i].pInstallInstanceId));

                adm_ips_updateEcuActiveState(ADM_IPS_ACTIVE_END, pActiveInfo->pstActiveEcuInfo[i].pRequestId, pActiveInfo->pstActiveEcuInfo[i].pSwdid);
                adm_ips_updateEcuActiveResult(ADM_ECODE_MSG_WAIT_TIMEOUT, pActiveInfo->pstActiveEcuInfo[i].pRequestId, pActiveInfo->pstActiveEcuInfo[i].pSwdid);
            }
        }
    }

    return;
}

static DVOID ipmHandleActiveEnd(const ADM_IPS_UCL_ECU_ARRAY_ACTIVE_T *pActiveInfo)
{
    DBOOL bIsFind = DFALSE;

    if (NULL == pActiveInfo || NULL == pActiveInfo->pstActiveEcuInfo)
    {
        log_w("Invalid param.");
        return;
    }

    /** 如果整体为刷写成功,则激活失败结果需要进行失败处理,否则不需要进行失败处理 */
    if (ADM_ECODE_SUCCESSED ==  adm_ips_getFlashResult())
    {
        for (D32U i = 0; i < pActiveInfo->uiEcuLen; i++)
        {
            if (ADM_ECODE_SUCCESSED != pActiveInfo->pstActiveEcuInfo[i].iActiveResult)
            {
                log_i("Find %s:%s, %s:%s, %s:%s %s:%d active result failed.",
                    ADM_STR_FROM_PARAM(ecuReqId), ADM_STR_SAFE_OUTPUT(pActiveInfo->pstActiveEcuInfo[i].pRequestId),
                    ADM_STR_FROM_PARAM(ecuSwDid), ADM_STR_SAFE_OUTPUT(pActiveInfo->pstActiveEcuInfo[i].pSwdid),
                    ADM_STR_FROM_PARAM(instanceId), ADM_STR_SAFE_OUTPUT(pActiveInfo->pstActiveEcuInfo[i].pInstallInstanceId),
                    ADM_STR_FROM_PARAM(activeResult), pActiveInfo->pstActiveEcuInfo[i].iActiveResult);

                bIsFind = DTRUE;

                /** 更新单ECU刷写结果为失败 */
                adm_ips_updateEcuFlashResult(pActiveInfo->pstActiveEcuInfo[i].pRequestId, pActiveInfo->pstActiveEcuInfo[i].pSwdid, pActiveInfo->pstActiveEcuInfo[i].iActiveResult);
            }
        }

        if (DTRUE == bIsFind)
        {
            log_d("Update active result to upgrade result.");
            adm_ips_stepPostInstall(ADM_ECODE_INSTALL_ECU_FLASH_FAILED);
            ipmUpdateStageResult(ADM_IPM_FLASH_STATE_UPGRADE, ADM_ECODE_INSTALL_ECU_FLASH_FAILED);
        }
    }

    return;
}

static D32S ipmDoFlashEcusReally(ADM_IPM_DO_FLASH_T *f, ADM_IPS_INSTALL_ECUINFO_T *buffEcus, D32S buffEcusLen) 
{
    D32S ret = ADM_ECODE_SUCCESSED;
    (void)pthread_mutex_lock(&f->mutex);
    if(ipmDoFlashEcusGetIps(f, buffEcus, buffEcusLen) != 0) {
        ipmReportProgressBegin(f->report);

	    D64U queryTime = 0;
        for(;;) {
            if(f->ecusCount != 0) {
                ipmDoFlashEcusQueryResult(f, &queryTime);
            }
            if(f->ecusCount != 0) {
                ipmDoFlashEcusVerifyEndTime(f);
            }
            if(f->isNeedGetEcus) {
                (void)ipmDoFlashEcusGetIps(f, buffEcus, buffEcusLen);
            }
            if (f->isNeedGetEcus || f->ecusCount != 0) {
                (void)pthread_mutex_unlock(&f->mutex);

                ///* 处理消息中心的消息，暂时放在这，后续优化。目前只有ECU刷写进度和刷写结果消息
                //   以解决DIM通知ECU刷写进度和刷写结果阻塞导致ECU刷写请求失败问题 */
                // ipmMsgCenterProcessMsg();       // add by wulin

                //(void)usleep(ADM_IPM_FLASH_GET_IPS_ECU_TIME_USEC_WIDE);

                /**放宽开锁时间，多余时间给让给进度处理 */
                (void)sleep(ADM_IPM_FLASH_GET_IPS_ECU_TIME_SEC_WIDE);

                (void)pthread_mutex_lock(&f->mutex);
                ipmReportProgressKeep(f->report);
                ipmDoFlashEcusListStoreKeep(f, FALSE);
            }
            else {
                break;
            }
        }
        ipmReportProgressEnd(f->report);
    }
	else  //只有第一次获取不到升级ecu才上报获取不到待升级ecu的事件
	{
        ipmReportEventDependencyEventId("IP_014", NULL, ADM_ECODE_SUCCESSED);	
	}

    /** 升级流程结束，判断是否需要进行后编程 */
    if (-1 != g_uiLastInstallGroup)
    {
        (void)ipmDoPostProgramEcus(g_uiLastInstallGroup);
        ipmGroupFlashSwitchAb(g_uiLastInstallGroup);
        g_uiLastInstallGroup = -1;
    }

    ret = ipmDoFlashEcusPost(f);
    (void)pthread_mutex_unlock(&f->mutex);
    // ADM_DEBUG_RETURN_D32S(ret);
    return ret;
}

/**
 * @brief 编程ECU列表数据构建json
 * @note 根据编程列表g_stProgramEcuListHead构建json
 * @param in void
 * @retval json_object *  json对象
 */
static json_object *ipmProgramEcuDataCreate(void)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    json_object *pEcuArryObj = NULL;
    json_object *pChild = NULL;
    ADM_IPM_PROGRAM_ECU_INFO_T *pstPos = NULL;
    ADM_IPM_PROGRAM_ECU_INFO_T *pstNext = NULL;

    ADM_DEBUG_TRACE();

    /** 创建json数组对象 */
    pEcuArryObj = json_object_new_array();
    if (ADM_IS_NULL(pEcuArryObj))
    {
        log_e("json_object_new_array err");
        iRet = ADM_ECODE_NEW_JSON_OBJECT_FAILED;
        return NULL;
    }

    pthread_mutex_lock(&g_stProgramEcuListHead.mutex);

    list_for_each_entry_safe(pstPos, pstNext, &g_stProgramEcuListHead.head, ADM_IPM_PROGRAM_ECU_INFO_T, entry)
    {
        if (ADM_IS_NULL(pstPos))
        {
            log_e("ADM_IPM_PROGRAM_ECU_INFO_T list pos NULL");
            continue;
        }

        /** 创建子对象 */
        pChild = json_object_new_object();
        if (ADM_IS_NULL(pChild))
        {
            log_e("json_object_new_object pChild %d", iRet);
            break;
        }

        /** 添加ECU请求ID */
        iRet = json_object_object_add(pChild,
                                      "requestId",
                                      json_object_new_int(pstPos->uiRequestId));
        if (ADM_IS_ERR(iRet))
        {
            log_e("json_object_new_int requestId error  %d", iRet);
            json_object_put(pChild);
            break;
        }
        
        /** 添加ECU响应ID */
        iRet = json_object_object_add(pChild,
                                      "responseId",
                                      json_object_new_int(pstPos->uiResponseId));
        if (ADM_IS_ERR(iRet))
        {
            log_e("json_object_new_int responseId error  %d", iRet);
            json_object_put(pChild);
            break;
        }

        /** 添加到数组 */
        iRet = json_object_array_add(pEcuArryObj, pChild);
        if (ADM_IS_ERR(iRet))
        {
            log_e("json_object_object_add : pChild error %d", iRet);
            json_object_put(pChild);
            break;
        }
    }

    pthread_mutex_unlock(&g_stProgramEcuListHead.mutex);

    if (ADM_IS_ERR(iRet))
    {
        json_object_put(pEcuArryObj);
        pEcuArryObj = NULL;
    }

    return pEcuArryObj;
}

/**
 * @brief 创建编程请求数据
 * @note json字符串
 * @param in ADM_IPM_PROGRAM_REQ_PARAM_T *pstReqParam 请求参数
 * @param out DCHAR **ppcRequestData 请求数据
 * @retval D32S 0 成功，其他失败
 */
static D32S ipmProgramRequestDataCreate(ADM_IPM_PROGRAM_REQ_PARAM_T *pstReqParam, DCHAR **ppcRequestData)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    json_object *pJsonObj = NULL;
    json_object *pJsonEcuObj = NULL;

    ADM_DEBUG_TRACE();

    /** 输入参数校验 */
    if (ADM_IS_NULL(pstReqParam) || ADM_IS_NULL(ppcRequestData))
    {
        log_e("invalid param");
        iRet = ADM_ECODE_INVALID_PARAM;
        return iRet;
    }

    do
    {
        /** 创建json对象 */
        pJsonObj = json_object_new_object();
        if (ADM_IS_NULL(pJsonObj))
        {
            log_e("json_object_new_object err");
            iRet = ADM_ECODE_NEW_JSON_OBJECT_FAILED;
            break;
        }

        /** 添加编程类型字段 */
        iRet = json_object_object_add(pJsonObj, "programType", json_object_new_int(pstReqParam->eProgramType));
        if (ADM_IS_ERR(iRet))
        {
            log_e("json_object_new_int programType: %d error %d", pstReqParam->eProgramType, iRet);
            break;
        }

        /** 添加functionId字段 */
        iRet = json_object_object_add(pJsonObj, "functionId", json_object_new_int(pstReqParam->uiFunctionId));
        if (ADM_IS_ERR(iRet))
        {
            log_e("json_object_new_int functionId: %d error %d", pstReqParam->uiFunctionId, iRet);
            break;
        }

        /** 添加脚本路径字段 */
        iRet = json_object_object_add(pJsonObj, "scriptPath", json_object_new_string(pstReqParam->pcScriptPath));
        if (ADM_IS_ERR(iRet))
        {
            log_e("json_object_new_string scriptPath: %s error %d", pstReqParam->pcScriptPath, iRet);
            break;
        }

        /** 构建ECU信息json对象 */
        pJsonEcuObj = ipmProgramEcuDataCreate();
        if (ADM_IS_NULL(pJsonEcuObj))
        {
            log_e("ipmProgramEcuDataCreate err");
            iRet = ADM_ECODE_NEW_JSON_OBJECT_FAILED;
            break;
        }

        /** 添加ECU信息字段 */
        iRet = json_object_object_add(pJsonObj, "ecuList", pJsonEcuObj);
        if (ADM_IS_ERR(iRet))
        {
            log_e("add ecuList: error %d", iRet);
            break;
        }

        *ppcRequestData = (DCHAR *)adm_strdup(json_object_to_json_string(pJsonObj));
    } while (FALSE);

    if (NULL != pJsonObj)
    {
        json_object_put(pJsonObj);
    }

    ADM_DEBUG_RETURN_D32S(iRet);
}

/**
 * @brief 预编程数据响应数据解析
 */
static void ipmProgramResponseDataParse(DCHAR* pResponseBuf, ADM_IPM_PREGRAM_ECU_DYNARRAY_T* pstPregramEcuInfo)
{
    json_object* respObj = NULL;
    json_object* ecuInfoArrObj = NULL;

    if (ADM_IS_NULL(pResponseBuf) || ADM_IS_NULL(pstPregramEcuInfo))
    {
        log_e("invalid param");
        return;
    }

    do
    {
        respObj = json_tokener_parse(pResponseBuf);

        ecuInfoArrObj = json_object_object_get(respObj, "ecuInfoList");
        if (ADM_IS_NULL(ecuInfoArrObj))
        {
            log_e("get ecuInfoList failed");
            break;
        }

        size_t msgIdArrayLength = json_object_array_length(ecuInfoArrObj);
        pstPregramEcuInfo->iNum = msgIdArrayLength;
        pstPregramEcuInfo->pPreEcuInfo = (ADM_IPM_PREGRAM_ECU_INFO_T*)adm_malloc(sizeof(ADM_IPM_PREGRAM_ECU_INFO_T) * msgIdArrayLength);
        if (ADM_IS_NULL(pstPregramEcuInfo->pPreEcuInfo))
        {
            log_e("malloc pPreEcuInfo failed");
            break;
        }

        for (size_t i = 0; i < msgIdArrayLength; i++)
        {
            json_object* ecuInfoObj = json_object_array_get_idx(ecuInfoArrObj, i); 
            
            pstPregramEcuInfo->pPreEcuInfo[i].iResult = json_object_get_int(json_object_object_get(ecuInfoObj, "check_result"));
            pstPregramEcuInfo->pPreEcuInfo[i].uiRequestId = json_object_get_int(json_object_object_get(ecuInfoObj, "requestId"));
            pstPregramEcuInfo->pPreEcuInfo[i].uiResponseId = json_object_get_int(json_object_object_get(ecuInfoObj, "responseId"));
            pstPregramEcuInfo->pPreEcuInfo[i].pResultDescrip = adm_strdup(json_object_get_string(json_object_object_get(ecuInfoObj, "check_result_descrip")));

            log_i("uiRequestId:%d,uiResponseId:%d,iResult:%d,pResultDescrip:%s",
                        pstPregramEcuInfo->pPreEcuInfo[i].uiRequestId,
                        pstPregramEcuInfo->pPreEcuInfo[i].uiResponseId,
                        pstPregramEcuInfo->pPreEcuInfo[i].iResult,
                        ADM_STR_SAFE_OUTPUT(pstPregramEcuInfo->pPreEcuInfo[i].pResultDescrip));
        }

    } while (DFALSE);
    
    if (NULL != respObj)
    {
        json_object_put(respObj);
    }

    return;
}

/**
 * @brief 获取响应ID
 * @note 通过配置信息获取响应ID
 * @param in DCHAR *ecuReqId ECU请求ID
 * @param in DCHAR *ecuSwDid ECU软件DID
 * @param out D32U *puiResponseId 响应ID
 * @retval D32S 0 成功  其他 失败
 */
static D32S ipmGetResponseIdByConfig(DCHAR *ecuReqId, DCHAR *ecuSwDid, D32U *puiResponseId)
{
    D32S iRet = ADM_ECODE_FAILED;
    D32S i = 0;
    ADM_ECS_ECUCONFIG_T *pstEcuConfig = NULL;
    ADM_ECS_RSCONFIGINFO_T *pstRsCfgInfo = NULL;
    
    ADM_DEBUG_TRACE();

    /** 输入参数校验 */
    if (ADM_IS_NULL(ecuReqId) || ADM_IS_NULL(ecuSwDid) || ADM_IS_NULL(puiResponseId))
    {
        log_e("invalid param");
        iRet = ADM_ECODE_INVALID_PARAM;
        return iRet;
    }

    do
    {
        /** 配置信息内存申请 */
        pstRsCfgInfo = adm_ecs_newRSConfigInfo();
        if (ADM_IS_NULL(pstRsCfgInfo))
        {
            log_e("adm_ecs_newRSConfigInfo err");
            iRet = ADM_ECODE_APPLY_MEMORY_FAILED;
            break;
        }

        /** 获取配置信息 */
        iRet = adm_ecs_getConfigInfo(pstRsCfgInfo);
        if (ADM_IS_ERR(iRet))
        {
            log_e("adm_ecs_getConfigInfo err");
            break;
        }

        /** ECU配置信息有效性校验 */
        if (ADM_IS_NULL(pstRsCfgInfo->stPartCfg.stEcuCfgList.pstEcuCfg))
        {
            log_e("pstEcuCfg is null");
            iRet = ADM_ECODE_INVALID_POINTER;
            break;
        }

        for (i = 0; i < pstRsCfgInfo->stPartCfg.stEcuCfgList.num; i++)
        {
            pstEcuConfig = &pstRsCfgInfo->stPartCfg.stEcuCfgList.pstEcuCfg[i];
            if (0 == adm_strcmp(ecuReqId, pstEcuConfig->ecuReqId) &&
                0 == adm_strcmp(ecuSwDid, pstEcuConfig->ecuSwDid))
            {
                *puiResponseId = (D32U)ipmHtoi(pstEcuConfig->ecuResponseId);
                iRet = ADM_ECODE_SUCCESSED;
                log_d("get responseId %d", *puiResponseId);
                break;
            }
        }
    } while (FALSE);
    
    /** 内存释放 */
    adm_ecs_delRSConfigInfo(&pstRsCfgInfo);

    ADM_DEBUG_RETURN_D32S(iRet);
}

static D32U ipmGetInstallGroup(DCHAR *pcParallelGroup)
{
    DCHAR aParallelGroup[3] = {0, 0, 0};

    ADM_COM_FUNC_ENTRY();

    if(ADM_IS_NULL(pcParallelGroup))
    {
        log_e("pstEcuSingle is NULL");
        return 0;
    }

    if((('0' == pcParallelGroup[0]) && ('x' == pcParallelGroup[1])) ||
       (('0' == pcParallelGroup[0]) && ('X' == pcParallelGroup[1])))
    {
        /** 取串行升级组AB */
        aParallelGroup[0] = pcParallelGroup[2];
        aParallelGroup[1] = pcParallelGroup[3];
    }
    else
    {
        /** 取串行升级组AB */
        aParallelGroup[0] = pcParallelGroup[0];
        aParallelGroup[1] = pcParallelGroup[1];
    }

    log_d("parallel group: %s", aParallelGroup);

    ADM_COM_FUNC_EXIT();

    return atoi(aParallelGroup);
}

static D32U ipmGetCurInstallGroup(DCHAR *ecuReqId, DCHAR *ecuSwDid)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    D32U uiParallelGroup = 0;
    ADM_ECS_RSCONFIGINFO_T *pstRsCfgData = NULL;
    ADM_ECS_ECUCONFIG_T *pstEcuCfg = NULL;
    DCHAR aParallelGroup[16] = {0};
    DBOOL bIsGetSuccess = FALSE;
    D32S i = 0;
    DCHAR *pEcuUpgradeOrder = NULL;

    ADM_COM_FUNC_ENTRY();

    do
    {
#if 0
        /*从配置文件中获取升级组信息*/
        pstRsCfgData = adm_ecs_newRSConfigInfo();
        if (ADM_IS_NULL(pstRsCfgData))
        {
            iRet = ADM_ECODE_NO_MEMORY;
            log_e("Failed to new ota config data info, err[%d]", iRet);
            break;
        }

        iRet = adm_ecs_getConfigInfo(pstRsCfgData);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed get config info, nRet[%d]", iRet);
            
            break;
        }

        for (i = 0; i < pstRsCfgData->stPartCfg.stEcuCfgList.num; i++)
        {
            pstEcuCfg = &pstRsCfgData->stPartCfg.stEcuCfgList.pstEcuCfg[i];
            if (0 == adm_strcasecmp(pstEcuCfg->ecuReqId, ecuReqId) &&
                0 == adm_strcasecmp(pstEcuCfg->ecuSwDid, ecuSwDid))
            {
                adm_memcpy(aParallelGroup, pstEcuCfg->stExternEcuInfo.upgradeOrder, 
                                           adm_strlen(pstEcuCfg->stExternEcuInfo.upgradeOrder), 
                                           16);
                log_d("get %s %s parallel group success. %s", ecuReqId, ecuSwDid, aParallelGroup);
                bIsGetSuccess = TRUE;
                break;
            }
        }

        if (bIsGetSuccess)
        {
            uiParallelGroup = ipmGetInstallGroup(aParallelGroup);
        }
#else
        /*从策略文件中获取升级组信息*/
        pEcuUpgradeOrder = adm_ips_ecuUpgradeOrderGet(ecuReqId,ecuSwDid);
        if (ADM_IS_NULL(pEcuUpgradeOrder))
        {
            log_e("get ecu upgrade order failed");
            iRet = ADM_ECODE_FAILED;
            break;
        }
        
        adm_memcpy(aParallelGroup, pEcuUpgradeOrder, adm_strlen(pEcuUpgradeOrder), 16);
        log_d("get %s %s parallel group success. %s", ecuReqId, ecuSwDid, aParallelGroup);

        uiParallelGroup = ipmGetInstallGroup(aParallelGroup);
#endif
    } while (FALSE);

    adm_free(pEcuUpgradeOrder);
    adm_ecs_delRSConfigInfo(&pstRsCfgData);

    ADM_COM_FUNC_EXIT();

    return uiParallelGroup;
}

/**
 * @brief 收集需要进行编程的ECU信息
 * @note 
 * @param in void
 * @retval D32S 0 成功，其他失败
 */
static D32S ipmProgramRequestEcuInfoCollect(ADM_IPM_PROGRAM_TYPE_E eProgramType, D32U uiParallelGroup)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    D32S i = 0;
    ADM_IPM_PROGRAM_ECU_INFO_T *pstEcuInfo = NULL;
    ADM_IPS_PROGRESS_T stProgressInfo = {0};
    ADM_IPS_ECU_PROGRESS_T *pstEcuProgress = 0;
    D32S iServiceType = 0;

    ADM_DEBUG_TRACE();

    adm_ips_progressInit(&stProgressInfo);
    adm_ips_progressGet(&stProgressInfo);

    for (i = 0; i < stProgressInfo.ecus.ecusLen; i++)
    {
        pstEcuProgress = &stProgressInfo.ecus.ecus[i];
        adm_ecs_getEcuServiceType(pstEcuProgress->ecuReqId, pstEcuProgress->ecuSwDid, &iServiceType);

        /** 跳过非VDI刷写的ECU */
        if (1 != iServiceType)
        {
            log_w("service type is %d, ecuReqId[%s] ecuSwDid[%s]", iServiceType, 
                                                                   pstEcuProgress->ecuReqId, 
                                                                   pstEcuProgress->ecuSwDid);
            continue;
        }

        /** 跳过非同一个并行组的ECU */
        if (uiParallelGroup == ipmGetCurInstallGroup(pstEcuProgress->ecuReqId, pstEcuProgress->ecuSwDid))
        {
            /** 预编程需要跳过已经完成安装的ECU，后编程不用 */
            if ((IPM_PROGRAM_TYPE_PRE == eProgramType) && 
                (ADM_IPS_ECU_PHASE_POST_INSTALL == pstEcuProgress->state ||
                ADM_IPS_ECU_PHASE_POST_INSTALL_ROLLBACK == pstEcuProgress->state))
            {
                log_d("ecuReqId[%s] ecuSwDid[%s] install finish state[%d]",pstEcuProgress->ecuReqId, 
                                                                           pstEcuProgress->ecuSwDid,
                                                                           pstEcuProgress->state);
                continue;
            }
        }
        else
        {
            log_w("ecuReqId[%s] ecuSwDid[%s] is not in parallel group[0x%x]", pstEcuProgress->ecuReqId, 
                                                                              pstEcuProgress->ecuSwDid, 
                                                                              uiParallelGroup);
            continue;
        }

        /** ECU信息节点内存申请 */
        pstEcuInfo = ipmProgramEcuInfoNew();
        if (ADM_IS_NULL(pstEcuInfo))
        {
            log_e("malloc failed");
            iRet = ADM_ECODE_APPLY_MEMORY_FAILED;
            break;
        }

        /** 填充ECU信息 */
        pstEcuInfo->uiRequestId = (D32U)ipmHtoi(pstEcuProgress->ecuReqId);
        iRet = ipmGetResponseIdByConfig(pstEcuProgress->ecuReqId, pstEcuProgress->ecuSwDid, &pstEcuInfo->uiResponseId);
        if (ADM_IS_ERR(iRet))
        {
            log_e("get response id failed");
            ipmProgramEcuInfoDel(pstEcuInfo);
            break;
        }

        /** 添加到列表 */
        ipmProgramEcuListAddTail(&pstEcuInfo->entry);
    }
    
    /** 内存释放 */
    adm_ips_progressDeInit(&stProgressInfo);

    ADM_DEBUG_RETURN_D32S(iRet);
}

/**
 * @brief 获取非APK自升级的FunctionId
 * 
 * @return D32U 
 */
static D32U ipmGetEcuFunctionId(ADM_ECS_ECUCFGLIST_T* pstEcuCfgList)
{
    D32S i = 0;

    /** 输入参数校验 */
    if (ADM_IS_NULL(pstEcuCfgList))
    {
        log_e("invalid param");
        return 0;
    }

    for (i = 0; i < pstEcuCfgList->num; i++)
    {
        //serviceType 为3 代表是APK自升级零件
        if (3 != pstEcuCfgList->pstEcuCfg[i].serviceType)
        {
            return (D32U)ipmHtoi(pstEcuCfgList->pstEcuCfg[i].ecuFunctionId);
        }
    }

    return 0;
}

static DCHAR* ipmGetEcuFlashScriptByNumber(ADM_ECS_GENERALCONFIG_T* pEcuGeneralCfg, D32S iFlashScriptNumber)
{
    DCHAR* pFlashScript = NULL;

    ADM_ECS_FLASHSCRIPTINFO_T* pFlashScriptInfo = NULL;
    ADM_ECS_FLASHSCRIPTLIST_T* pFlashScriptList = &pEcuGeneralCfg->stflashScriptList;
    for (D32S i = 0; i < pFlashScriptList->num; i++)
    {
        pFlashScriptInfo = &pFlashScriptList->pstFlashScriptInfo[i];
        if (iFlashScriptNumber == pFlashScriptInfo->flashScriptNumber)
        {
            pFlashScript = adm_strdup(pFlashScriptInfo->stFlashScriptFile.filePath);
            break;
        }
    }

    return pFlashScript;
}


/**
 * @brief 填充编程请求参数
 * @note
 * @param in ADM_IPM_PROGRAM_TYPE_E eProgramType        编程类型
 * @param out ADM_IPM_PROGRAM_REQ_PARAM_T *pstReqParam  编程请求参数
 * @retval D32S 0 成功，其他 失败
 */
static D32S ipmProgramRequestParamFill(ADM_IPM_PROGRAM_TYPE_E eProgramType, ADM_IPM_PROGRAM_REQ_PARAM_T *pstReqParam)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    ADM_ECS_ECUCONFIG_T *pstEcuConfig = NULL;
    ADM_ECS_RSCONFIGINFO_T *pstRsCfgInfo = NULL;
    
    ADM_DEBUG_TRACE();

    /** 输入参数校验 */
    if (ADM_IS_NULL(pstReqParam))
    {
        log_e("invalid param");
        iRet = ADM_ECODE_INVALID_PARAM;
        return iRet;
    }

    do
    {
        /** 填充编程类型 */
        pstReqParam->eProgramType = eProgramType;

        /** 配置信息内存申请 */
        pstRsCfgInfo = adm_ecs_newRSConfigInfo();
        if (ADM_IS_NULL(pstRsCfgInfo))
        {
            log_e("adm_ecs_newRSConfigInfo err");
            iRet = ADM_ECODE_APPLY_MEMORY_FAILED;
            break;
        }

        /** 获取配置信息 */
        iRet = adm_ecs_getConfigInfo(pstRsCfgInfo);
        if (ADM_IS_ERR(iRet))
        {
            log_e("adm_ecs_getConfigInfo err");
            break;
        }

        /** ECU配置信息有效性校验 */
        if (ADM_IS_NULL(pstRsCfgInfo->stPartCfg.stEcuCfgList.pstEcuCfg))
        {
            log_e("pstEcuCfg is null");
            iRet = ADM_ECODE_INVALID_POINTER;
            break;
        }

        /** 填充脚本路径 */
        if (IPM_PROGRAM_TYPE_PRE == eProgramType)
        {
            pstReqParam->pcScriptPath = ipmGetEcuFlashScriptByNumber(&pstRsCfgInfo->stGeneralCfg, ADM_PRE_FLASH_SCIPT_NUM);
            /** 未获取到脚本采用内置的脚本 */
            if (NULL == pstReqParam->pcScriptPath)
            {
                log_w("Can not find pre script.");
                pstReqParam->pcScriptPath = adm_strdup(ADM_IPM_PRE_PROGRAM_SCRIPT_PATH);
            }
        }
        else if (IPM_PROGRAM_TYPE_POST == eProgramType)
        {
            pstReqParam->pcScriptPath = ipmGetEcuFlashScriptByNumber(&pstRsCfgInfo->stGeneralCfg, ADM_POST_FLASH_SCIPT_NUM);
            if (NULL == pstReqParam->pcScriptPath)
            {
                /** 未获取到脚本采用内置的脚本 */
                log_w("Can not find post script.");
                pstReqParam->pcScriptPath = adm_strdup(ADM_IPM_POST_PROGRAM_SCRIPT_PATH);
            }
        }
        else
        {
            log_e("invalid program type");
            iRet = ADM_ECODE_INVALID_PARAM;
            break;
        }

        /** 所有零件的功能ID都一样，所以只取第一个零件的相关信息 */
        //pstEcuConfig = pstRsCfgInfo->stPartCfg.stEcuCfgList.pstEcuCfg;
        //if (pstEcuConfig->ecuFunctionId)
        //{
        //    pstReqParam->uiFunctionId = (D32U)ipmHtoi(pstEcuConfig->ecuFunctionId);
        //}
        pstReqParam->uiFunctionId = ipmGetEcuFunctionId(&pstRsCfgInfo->stPartCfg.stEcuCfgList);
    } while (FALSE);
    
    adm_ecs_delRSConfigInfo(&pstRsCfgInfo);

    ADM_DEBUG_RETURN_D32S(iRet);
}

/**
 * @brief 预编程处理
 * @note 针对非智能件刷写前的处理
 * @param void
 * @retval D32S 0 成功，其他 失败
 */
static D32S ipmDoPreProgramEcus(D32U uiParallelGroup, ADM_IPM_PREGRAM_ECU_DYNARRAY_T* pstPregramEcuInfo)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    ADM_IPM_PROGRAM_REQ_PARAM_T *pstReqParam = NULL;
    DCHAR *pcRequestBuf = NULL;
    DCHAR *pcResponseBuf = NULL;

    ADM_DEBUG_TRACE();

    do
    {
        log_i("[ABUP-Procedure]: Install Start Pre Program");
        ipmProgramEcuListInit();

        /** 请求数据内存申请 */
        pstReqParam = ipmProgramParamNew();
        if (ADM_IS_NULL(pstReqParam))
        {
            log_e("apply memory failed");
            iRet = ADM_ECODE_APPLY_MEMORY_FAILED;
            break;
        }

        /** 通用请求信息填充 */
        iRet = ipmProgramRequestParamFill(IPM_PROGRAM_TYPE_PRE, pstReqParam);
        if (ADM_IS_ERR(iRet))
        {
            log_e("fill request param failed");
            break;
        }

        /** ECU请求信息收集（添加到全局列表） */
        iRet = ipmProgramRequestEcuInfoCollect(IPM_PROGRAM_TYPE_PRE, uiParallelGroup);
        if (ADM_IS_ERR(iRet))
        {
            log_e("fill ecu info failed");
            break;
        }
        
        /** 列表为空，不需要进行预编程 */
        if (TRUE == ipmProgramEcuListIsEmpty())
        {
            iRet = ADM_ECODE_SUCCESSED;
            break;
        }

        /** 创建编程请求数据（json字符串） */
        iRet = ipmProgramRequestDataCreate(pstReqParam, &pcRequestBuf);
        if (ADM_IS_ERR(iRet))
        {
            log_e("create request data failed");
            break;
        }

        /** 发起编程请求 */
        iRet = adm_dim_vdi_programEcusTrigger(adm_soa_getVdiInstanceId(), ADM_IPM_PROGRAM_TIMEOUT, pcRequestBuf, &pcResponseBuf);
        if (ADM_ECODE_RPC_INSTANCE_OFFLINE == iRet)
        {
            ///<预编程响应服务不在线，休眠15s重试一次；arpc重连15s重试一次
            sleep(15);
            iRet = adm_dim_vdi_programEcusTrigger(adm_soa_getVdiInstanceId(), ADM_IPM_PROGRAM_TIMEOUT, pcRequestBuf, &pcResponseBuf);
        }
        log_i("[ABUP-Procedure]: Install End Pre Program, [err:%d]", iRet);

        /**预编程失败时，解析响应数据，需要上报云端*/
        if (ADM_IS_ERR(iRet) && (IPM_PROGRAM_TYPE_PRE == pstReqParam->eProgramType))
        {
            ipmProgramResponseDataParse(pcResponseBuf, pstPregramEcuInfo);
        }

    } while (FALSE);
    
    /** 内存释放 */
    ipmProgramParamDel(pstReqParam);
    adm_free(pcRequestBuf);
    adm_free(pcResponseBuf);
    ipmProgramEcuListDestory();

    ADM_DEBUG_RETURN_D32S(iRet);
}

/**
 * @brief 后编程处理
 * @note 针对非智能件刷写后的处理
 * @param void
 * @retval D32S 0 成功，其他 失败
 */
static D32S ipmDoPostProgramEcus(D32U uiParallelGroup)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    ADM_IPM_PROGRAM_REQ_PARAM_T *pstReqParam = NULL;
    DCHAR *pcRequestBuf = NULL;
    DCHAR *pcResponseBuf = NULL;

    ADM_DEBUG_TRACE();

    do
    {
        log_i("[ABUP-Procedure]: Install Start Post Program");
        ipmProgramEcuListInit();

        /** 请求数据内存申请 */
        pstReqParam = ipmProgramParamNew();
        if (ADM_IS_NULL(pstReqParam))
        {
            log_e("apply memory failed");
            iRet = ADM_ECODE_APPLY_MEMORY_FAILED;
            break;
        }

        /** 通用请求信息填充 */
        iRet = ipmProgramRequestParamFill(IPM_PROGRAM_TYPE_POST, pstReqParam);
        if (ADM_IS_ERR(iRet))
        {
            log_e("fill request param failed");
            break;
        }

        /** ECU请求信息收集（添加到全局列表） */
        iRet = ipmProgramRequestEcuInfoCollect(IPM_PROGRAM_TYPE_POST, uiParallelGroup);
        if (ADM_IS_ERR(iRet))
        {
            log_e("fill ecu info failed");
            break;
        }
        
        /** 列表为空，不需要进行后编程 */
        if (TRUE == ipmProgramEcuListIsEmpty())
        {
            iRet = ADM_ECODE_SUCCESSED;
            break;
        }

        /** 创建编程请求数据（json字符串） */
        iRet = ipmProgramRequestDataCreate(pstReqParam, &pcRequestBuf);
        if (ADM_IS_ERR(iRet))
        {
            log_e("create request data failed");
            break;
        }

        /** 发起编程请求 */
        iRet = adm_dim_vdi_programEcusTrigger(adm_soa_getVdiInstanceId(), ADM_IPM_PROGRAM_TIMEOUT, pcRequestBuf, &pcResponseBuf);
        log_i("[ABUP-Procedure]: Install End Post Program, [err:%d]", iRet);
    } while (FALSE);
    
    /** 内存释放 */
    ipmProgramParamDel(pstReqParam);
    adm_free(pcRequestBuf);
    adm_free(pcResponseBuf);
    ipmProgramEcuListDestory();

    ADM_DEBUG_RETURN_D32S(iRet);
}

static D32S ipmDoFlashEcus(ADM_IPM_DO_FLASH_T *f) {
    D32S ret = ADM_ECODE_FAILED;
    log_i("[ABUP-Procedure]: Install Start Main Program");
    D32S buffEcusLen = adm_ips_maxEcusCountGet();
    if(buffEcusLen == 0) {
        log_e("inval upgrade policy");
        ret = ADM_ECODE_INSTALL_INVAL_POLICY;
    } else {
        ADM_IPS_INSTALL_ECUINFO_T *buffEcus = adm_malloc((D32U)buffEcusLen *sizeof(*(buffEcus)));
        if(buffEcus == NULL) {
            log_e("malloc failed");
            ret = ADM_ECODE_NO_MEMORY;
        } else {
            ipsInstallEcuInfoArrayInit(buffEcus, buffEcusLen);
            ret = ipmDoFlashEcusReally(f, buffEcus, buffEcusLen);
            ipsInstallEcuInfoArrayDeInit(buffEcus, buffEcusLen);
            adm_free(buffEcus);
        }
    }
    log_i("[ABUP-Procedure]: Install End Main Program, [err:%d]", ret);
    // ADM_DEBUG_RETURN_D32S(ret);
    return ret;
}

//!< 进度消息通知时间间隔，单位ms
#define ADM_IPM_MSG_NOTIFY_TIME_WIDE    2000

static DBOOL ipmIsOverNotifyTimeWide(struct timespec *pstTs, D32U* pWide)
{
    DBOOL bRet = FALSE;
    struct timespec tsNow;
    D32U uiWide = 0;

    if (ADM_IS_NULL(pstTs))
    {
        log_e("Invalid input param");
        return bRet;
    }

    (void)clock_gettime(CLOCK_MONOTONIC, &tsNow);

    if (NULL != pWide)
    {
        *pWide = (tsNow.tv_sec - pstTs->tv_sec) + ((tsNow.tv_nsec - pstTs->tv_nsec) / (1000 * 1000 * 1000));
    }

    if(((tsNow.tv_sec - pstTs->tv_sec) * 1000 +
        (tsNow.tv_nsec - pstTs->tv_nsec) / (1000 * 1000)) > 
              ADM_IPM_MSG_NOTIFY_TIME_WIDE)
    {
        bRet = TRUE;
        pstTs->tv_sec = tsNow.tv_sec;
        pstTs->tv_nsec = tsNow.tv_nsec;
    }
    
    return bRet;
}


static void ipmDoFlashEcuSetProgress(ADM_IPM_DO_FLASH_T *f, const DCHAR *ecuReqId, const DCHAR *ecuSwDid, D32S progress){
    ADM_IPS_ECU_PROGRESS_T stProgress = { 0 };
    static struct timespec stTs = {0};
    D32S iProgress = 0;
	
    (void)pthread_mutex_lock(&f->mutex);

    /** 获取ecu升级进度信息 */
    (void)adm_ips_ecuProgressGet(&stProgress, ecuReqId, ecuSwDid);
    if (stProgress.state == ADM_IPS_ECU_PHASE_INSTALL)
    {
        iProgress = stProgress.progress[ADM_IPS_ECU_PHASE_INDEX_UPGRADE].progress;
    }
    else if (stProgress.state == ADM_IPS_ECU_PHASE_INSTALL_ROLLBACK)
    {
        iProgress = stProgress.progress[ADM_IPS_ECU_PHASE_INDEX_ROLLBACK].progress;
    }
    else 
    {
        // log_i("requestId:[%s] swDid[%s] upgrade progress, current state is [%d].", ecuReqId, ecuSwDid, stProgress.state);
        ; ///< do nothing
    }

    ADM_IPM_ECU_INFO_NODE_T *node = ipmDoFlashEcusFind(f, ecuReqId, ecuSwDid);
    if(node != NULL)
    {
        if (iProgress != progress)
        {
            if ((ipmIsOverNotifyTimeWide(&stTs, NULL)) || (100 == progress))
            {
                ipmReportEventDependencyEventId("II_015",  NULL, ADM_ECODE_SUCCESSED);
            }

            ipmDoFlashEcuProgress(f, node, progress);
        }
    }
	else{		
		DCHAR eventDescTmp[128] = {0};
	    if((ADM_IPS_ECU_PHASE_POST_INSTALL != stProgress.state)&&(ADM_IPS_ECU_PHASE_POST_INSTALL_ROLLBACK != stProgress.state)) //当零件安装状态为安装完成或回滚完成时，不上报安装进度设置错误
	    {
	        adm_memset(eventDescTmp, 0x00, sizeof(eventDescTmp), sizeof(eventDescTmp));	
            (void)adm_snprintf(eventDescTmp, sizeof(eventDescTmp), "flash ecu set progress failed . ECU: ecuReqId %s,ecuSwDid %s", ecuReqId,ecuSwDid);
	        ipmReportEventDependencyEventId("II_016",  eventDescTmp, ADM_ECODE_SUCCESSED);
	    }
		log_i("flash ecu set progress failed . ECU: ecuReqId %s,ecuSwDid %s", ecuReqId,ecuSwDid);
	}

    adm_ips_ecuProgressDeInit(&stProgress);
    (void)pthread_mutex_unlock(&f->mutex);
}
static void ipmDoFlashEcuSetResult(ADM_IPM_DO_FLASH_T *f, const DCHAR *ecuReqId, const DCHAR *ecuSwDid, D32S result, const DCHAR* pResultDescrip)
{
    (void)pthread_mutex_lock(&f->mutex);
    ADM_IPM_ECU_INFO_NODE_T *node = ipmDoFlashEcusFind(f, ecuReqId, ecuSwDid);
    if (node != NULL)
    {
        if (1 == node->ecsEcu->serviceType && -1 == g_uiLastInstallGroup)
        {
            log_d("last install group is -1, so ignore this ecu");
        }
        else
        {
            ipmDoFlashEcuEnd(f, node, result, pResultDescrip);
        }
    }
    (void)pthread_mutex_unlock(&f->mutex);
}
static void ipmDoFlashEcusVerifyEndTime(ADM_IPM_DO_FLASH_T *f){
    ADM_IPM_ECU_INFO_NODE_T *p, *n;
    list_for_each_entry_safe(p, n, &f->head, ADM_IPM_ECU_INFO_NODE_T, entry){
	if(ipmEcuInfoNodeIsTimeOut(p)){
	    ipmDoFlashEcuEnd(f, p, ADM_ECODE_OPERATION_TIMEDOUT, NULL);
	}
    }
}
static void ipmDoFlashEcusQueryResult(ADM_IPM_DO_FLASH_T *f, D64U *queryTime){
    D64U curTime = ipmMonotonicTimeMSecGet();
    DCHAR *pResultDescrip = NULL;

    if (0 == *queryTime)
    {
        *queryTime = curTime;
        return;
    }

    if((curTime - *queryTime) < ADM_IPM_FLASH_QUERY_ECU_RESULT_TIME_MSEC_WIDE){}else{
        ADM_IPM_ECU_INFO_NODE_T *p, *n;
        list_for_each_entry_safe(p, n, &f->head, ADM_IPM_ECU_INFO_NODE_T, entry){
            D32S progress = 0;
            DBOOL dRetProgress = ipmDimFlashEcuQueryProgress(f->dim, p, &progress);
            D32S result = ADM_ECODE_FAILED;
            if (ipmDimFlashEcuQueryResult(f->dim, p, &result,&pResultDescrip)) {
                if (dRetProgress) {
                    ipmDoFlashEcuProgress(f, p, progress);
                }

                /** VDI刷写件的结果处理 */
                if (1 == p->ecsEcu->serviceType)
                {
                    /** 0x01：正在刷写中， 0x1000：未进入主编程 */
                    if (0x01 == result || 0x1000 == result)
                    {
                        log_w("VDI result[%d]", result);
                    }
                    else
                    {
                        log_i("result[%d]", result);
                        if (0 == result)
                        {
                            ipmDoFlashEcuProgress(f, p, 100);
                        }
                        
                        ipmDoFlashEcuEnd(f, p, result, pResultDescrip);
                    }
                }
                else /** 其他智能件的结果处理 */
                {
                    /** 只处理成功和失败的结果 */
                    if (UPGRADE_SUCCEED == result || UPGRADE_FAILED == result)
                    {
                        log_i("result[%d]", result);
                        if (0 == result)
                        {
                            ipmDoFlashEcuProgress(f, p, 100);
                        }

                        ipmDoFlashEcuEnd(f, p, result, NULL);
                    }
                    else 
                    {
                        log_i("result[%d], do not handle.", result);
                    }
                }
            }
        }

	    *queryTime = curTime;
    }

    adm_free(pResultDescrip);
    return;
}

static DBOOL ipmIsNeedResumeInstall(ADM_IPM_ECU_INFO_NODE_T *pstNodeInfo)
{
    DBOOL bIsNeedResume = FALSE;
    D32S result = ADM_ECODE_FAILED;
    DCHAR *pResultDescrip = NULL;

    /** VDI刷写的非智能件需要恢复触发 */
    if (1 == pstNodeInfo->ecsEcu->serviceType)
    {
        if (TRUE == ipmDimFlashEcuQueryResult(m_ipmContext.flash.dim, pstNodeInfo, &result, &pResultDescrip))
        {
            log_d("VDI result[0x%x]", result);
            if (0x1 == result)
            {
                log_d("%s_%s is installing", pstNodeInfo->ecsEcu->ecuReqId, pstNodeInfo->ecsEcu->ecuSwDid);
            }
            else if (0x1000 == result)
            {
                log_d("%s_%s is not in main program", pstNodeInfo->ecsEcu->ecuReqId, pstNodeInfo->ecsEcu->ecuSwDid);
                bIsNeedResume = TRUE;
            }
            else
            {
                log_d("%s_%s is install finish", pstNodeInfo->ecsEcu->ecuReqId, pstNodeInfo->ecsEcu->ecuSwDid);
            }
        }
        else
        {
            bIsNeedResume = TRUE;
        }
    }
    adm_free(pResultDescrip);
    /** 车机/从控/apk自升级都不需要再次触发 */
    if (2 == pstNodeInfo->ecsEcu->serviceType || 3 == pstNodeInfo->ecsEcu->serviceType)
    {
        bIsNeedResume = FALSE;
    }

    return bIsNeedResume;
}

static DBOOL ipmCheckVdiEcuInstalling(ADM_IPS_INSTALL_ECUINFO_T *pstEcusInfo, D32S iEcusLen)
{
    D32S i = 0;
    D32S result = ADM_ECODE_FAILED;
    ADM_IPM_ECU_INFO_NODE_T *pstEcusNode = NULL;
    DBOOL bIsInstalling = FALSE;
    DCHAR *pResultDescrip = NULL;

    if (ADM_IS_NULL(pstEcusInfo))
    {
        log_e("invalid param");
        return FALSE;
    }

    for (i = 0; i < iEcusLen; i++)
    {
        pstEcusNode = ipmDoFlashEcusFind(&m_ipmContext.flash.doFlash, pstEcusInfo[i].ecuReqId, pstEcusInfo[i].ecuSwDid);
        if (NULL != pstEcusNode)
        {
            if (1 != pstEcusNode->ecsEcu->serviceType)
            {
                log_d("ecu %s_%s is not VDI", pstEcusInfo[i].ecuReqId, pstEcusInfo[i].ecuSwDid);
                continue;
            }

            if (TRUE == ipmDimFlashEcuQueryResult(m_ipmContext.flash.dim, pstEcusNode, &result,&pResultDescrip))
            {
                log_d("VDI result[0x%x]", result);
                if (0x1000 != result)
                {
                    log_d("%s_%s is installing", pstEcusInfo[i].ecuReqId, pstEcusInfo[i].ecuSwDid);
                    bIsInstalling = TRUE;
                    break;
                }
            }
        }
    }
    adm_free(pResultDescrip);
    return bIsInstalling;
}

static DVOID ipmGroupFlashSwitchAb(D32U uiGroupId)
{
    ADM_IPS_PROGRESS_T stProgressInfo = {0};
    ADM_IPS_ECU_PROGRESS_T stEcuprogress = {0};
    DCHAR* pRequestId = NULL;
    DCHAR* pSwDid = NULL;
    ADM_DIM_SOA_VDI_ECU_AREA_INFO_E eDstAreaInfo = ADM_SYSI4VDI_ECU_AREA_INFO_UNKNOW;
    D32S iRet = ADM_ECODE_FAILED;
    D32S iFlashProgressType = 0;
    D32U uiTimeOut = 0;

    adm_ips_progressInit(&stProgressInfo);
    adm_ips_progressGet(&stProgressInfo);
    
    for (D32S i = 0; i < stProgressInfo.ecus.ecusLen; i++)
    {
        ADM_IPS_ECU_PROGRESS_T* pstEcu = stProgressInfo.ecus.ecus + i;
        pRequestId = stProgressInfo.ecus.ecus[i].ecuReqId;
        pSwDid = stProgressInfo.ecus.ecus[i].ecuSwDid;

        /** 安装完成 && 安装结果成功 */
        if ((ADM_IPS_ECU_PHASE_POST_INSTALL == pstEcu->state) && 
            0 == pstEcu->progress[ADM_IPS_ECU_PHASE_INDEX_UPGRADE].result)
        {
            if (DTRUE == pstEcu->bIsAb  && DTRUE == pstEcu->bBlockSwitchAb 
                && DFALSE == pstEcu->bIsFLashHasSwitchAb)
            {
                /** 操作当前升级组的AB件切区 */
                if (uiGroupId == ipmGetCurInstallGroup(pRequestId, pSwDid))
                {
                    eDstAreaInfo = pstEcu->eDestAbInfo;
                    // (DVOID)adm_ecs_getEcuFlashProgressType(pRequestId, 
                    //             pSwDid, &iFlashProgressType);

                    // if (1 == iFlashProgressType)
                    // {
                    //     /** 先临时解决,后期做异步通知 */
                        uiTimeOut = 5 * 60; ///< 四个件doip server 如果有一个不在线，就等待90s
                    // }
                    // else 
                    // {
                    //     uiTimeOut = m_ipmContext.flash.dim->requestTimeout;
                    // }

                    iRet = adm_ipm_switchAb(pRequestId, pSwDid, uiTimeOut, eDstAreaInfo);
                    if (ADM_IS_SUCCESS(iRet))
                    {
                        log_i("requestId[%s] swDid[%s] flash switch AB succeed.", pRequestId, pSwDid);
                        adm_ipm_updateFlashAbSwitch(pRequestId, pSwDid, DTRUE);
                    }
                    else 
                    {
                        log_i("requestId[%s] swDid[%s] flash switch AB failed.", pRequestId, pSwDid);
                        pstEcu->progress[ADM_IPS_ECU_PHASE_INDEX_UPGRADE].result = iRet;
                        adm_ipm_updFLashAbSwitchFailed(pRequestId, pSwDid, iRet);
                    }
                }
            }
        }
    }

    adm_ips_progressDeInit(&stProgressInfo);
    adm_ips_ecuProgressDeInit(&stEcuprogress);

    return;
}

static DVOID ipmGroupRollbackSwitchAb(D32U uiGroupId)
{
    ADM_IPS_PROGRESS_T stProgressInfo = {0};
    ADM_IPS_ECU_PROGRESS_T stEcuprogress = {0};
    DCHAR* pRequestId = NULL;
    DCHAR* pSwDid = NULL;
    ADM_DIM_SOA_VDI_ECU_AREA_INFO_E eDstAreaInfo = ADM_SYSI4VDI_ECU_AREA_INFO_UNKNOW;
    D32S iRet = ADM_ECODE_FAILED;

    adm_ips_progressInit(&stProgressInfo);
    adm_ips_progressGet(&stProgressInfo);
    
    for (D32S i = 0; i < stProgressInfo.ecus.ecusLen; i++)
    {
        ADM_IPS_ECU_PROGRESS_T* pstEcu = stProgressInfo.ecus.ecus + i;
        pRequestId = stProgressInfo.ecus.ecus[i].ecuReqId;
        pSwDid = stProgressInfo.ecus.ecus[i].ecuSwDid;

        /** 回滚阶段  && 安装结果成功 （零件组内其他零件要求本零件回滚的情况）*/
        if (ADM_IPS_ECU_PHASE_POST_INSTALL_JUMP != pstEcu->state && 
            ADM_IPS_ECU_PHASE_POST_INSTALL_ROLLBACK != pstEcu->state &&
            ADM_IPS_ECU_PHASE_TIMELY_PRE_INSTALL_ROLLBACK <= pstEcu->state && 
            0 == pstEcu->progress[ADM_IPS_ECU_PHASE_INDEX_UPGRADE].result)
        {
            if (DTRUE == pstEcu->bIsAb  && DTRUE == pstEcu->bBlockSwitchAb 
                && DFALSE == pstEcu->bIsRollbackHasSwitchAB)
            {
                /** 操作当前升级组的AB件切区 */
                if (uiGroupId == ipmGetCurInstallGroup(pRequestId, pSwDid))
                {
                    adm_ipm_getDestAbInfo(pstEcu->eDestAbInfo, &eDstAreaInfo);
                    iRet = adm_ipm_switchAb(pRequestId, pSwDid, m_ipmContext.flash.dim->requestTimeout, eDstAreaInfo);
                    if (ADM_IS_SUCCESS(iRet))
                    {
                        log_i("requestId[%s] swDid[%s] rollback switch AB succeed.", pRequestId, pSwDid);
                        adm_ipm_updateRollbackAbSwitch(pRequestId, pSwDid, DTRUE);
                    }
                    else 
                    {
                        log_i("requestId[%s] swDid[%s] Rollback switch AB failed.", pRequestId, pSwDid);
                        pstEcu->progress[ADM_IPS_ECU_PHASE_INDEX_ROLLBACK].result = iRet;
                        pstEcu->progress[ADM_IPS_ECU_PHASE_INDEX_ROLLBACK].upgradeNumbers++;
                        pstEcu->progress[ADM_IPS_ECU_PHASE_INDEX_ROLLBACK].endTime = time(NULL);
                        adm_ipm_updRollbackAbSwitchFailed(pRequestId, pSwDid, iRet, 
                            pstEcu->progress[ADM_IPS_ECU_PHASE_INDEX_ROLLBACK].upgradeNumbers++, 
                            pstEcu->progress[ADM_IPS_ECU_PHASE_INDEX_ROLLBACK].endTime);
                    }
                }
            }
        }
    }

    adm_ips_progressDeInit(&stProgressInfo);
    adm_ips_ecuProgressDeInit(&stEcuprogress);

    return;
}

static D32S ipmEcuPreAndPostProgramm(ADM_IPS_INSTALL_ECUINFO_T *pstEcusInfo, D32S iEcusLen, ADM_IPM_PREGRAM_ECU_DYNARRAY_T* pstPregramEcuInfo)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    D32U uiCurInstallGroup = 0;
    ADM_IPM_ECU_INFO_NODE_T *pstNode = NULL;

    ADM_COM_FUNC_ENTRY();

    if (ADM_IS_NULL(pstEcusInfo))
    {
        log_e("invalid param");
        iRet = ADM_ECODE_INVALID_PARAM;
        return iRet;
    }

    do
    {
        uiCurInstallGroup = ipmGetCurInstallGroup(pstEcusInfo->ecuReqId, pstEcusInfo->ecuSwDid);
        log_d("cur install group:%d, last install group:%d", uiCurInstallGroup, g_uiLastInstallGroup);
        if (g_uiLastInstallGroup == uiCurInstallGroup)
        {
            log_i("no need to pre and post programm");
            break;
        }

        if (-1 != g_uiLastInstallGroup)
        {
            /** 执行上一组的后编程 */
            (void)ipmDoPostProgramEcus(g_uiLastInstallGroup);
            ipmGroupFlashSwitchAb(g_uiLastInstallGroup);
        }

        /** 判断是否存在正在刷写中的ECU */
        if (ipmCheckVdiEcuInstalling(pstEcusInfo, iEcusLen))
        {
            log_i("exist ecu installing, no need to pre programm");
            g_uiLastInstallGroup = uiCurInstallGroup;
            break;
        }

        pstNode = ipmEcuInfoNodeNew(pstEcusInfo);

        /** 执行高压切换 */
        iRet = ipmDimIgSwitchCtrl(pstNode);
        if (ADM_IS_ERR(iRet))
        {
            log_e("ipmDimIgSwitchCtrl fail, ret:%d", iRet);
            // iRet = ADM_ECODE_INSTALL_PRE_PROGRAM_FAILED;
            g_uiLastInstallGroup = -1;
        }
        else
        {
            /** 校验当前是否有因升级成功却因为零件组回滚要求进行回滚的切区阻塞的AB件，如果有，执行回滚包刷写前需要进行切区 */
            ipmGroupRollbackSwitchAb(uiCurInstallGroup);

            /** 执行当前组的预编程 */
            iRet = ipmDoPreProgramEcus(uiCurInstallGroup,pstPregramEcuInfo);
            if (ADM_IS_ERR(iRet))
            {
                iRet = ADM_ECODE_INSTALL_PRE_PROGRAM_FAILED;
                log_e("ipmDoPreProgramEcus fail, ret:%d", iRet);
                g_uiLastInstallGroup = -1;
            }
            else
            {
                log_i("pre programm success");
                g_uiLastInstallGroup = uiCurInstallGroup;
            }
        }

        ipmEcuInfoNodeDel(pstNode);
    } while (FALSE);

    ADM_COM_FUNC_EXIT();

    return iRet;
}

static void ipmCleanPreProgramEcus(ADM_IPM_PREGRAM_ECU_DYNARRAY_T* pstPregramEcuInfo)
{ 
    if (ADM_IS_NULL(pstPregramEcuInfo))
    {
        return;
    }

    for (size_t i = 0; i < pstPregramEcuInfo->iNum; i++)
    {
        adm_free(pstPregramEcuInfo->pPreEcuInfo[i].pResultDescrip);
    }
    
    adm_free(pstPregramEcuInfo->pPreEcuInfo);

    return;
}

static DCHAR* ipmGetPreProgramEcuspResultDescrip(const DCHAR *pEcuReqId, ADM_IPM_PREGRAM_ECU_DYNARRAY_T* pstPregramEcuInfo)
{ 
    if (ADM_IS_NULL(pEcuReqId) || ADM_IS_NULL(pstPregramEcuInfo))
    {
        return NULL;
    }

    for (size_t i = 0; i < pstPregramEcuInfo->iNum; i++)
    {
        if (pstPregramEcuInfo->pPreEcuInfo[i].uiRequestId == ipmHtoi(pEcuReqId))
        {
            if (ADM_IS_ERR(pstPregramEcuInfo->pPreEcuInfo[i].iResult))
            {
                return adm_strdup(pstPregramEcuInfo->pPreEcuInfo[i].pResultDescrip);
            }
            else
            {
                break;
            }
        }
    }

    return NULL;
}

static D32S ipmDoFlashEcusGetIps(ADM_IPM_DO_FLASH_T *f, ADM_IPS_INSTALL_ECUINFO_T *buffEcus, D32S buffEcusLen)
{
    D32S i = 0;
    D32S iRet = ADM_ECODE_SUCCESSED;
    ADM_IPM_ECU_INFO_NODE_T *pstTmpNodeInfo = NULL;
    ADM_IPM_ECU_INFO_NODE_T *pstNewNode = NULL;
    ADM_IPM_PREGRAM_ECU_DYNARRAY_T stPregramEcuInfo = {0};
    DCHAR* pResultDescrip = NULL;
    DCHAR* pTriggerInfo = NULL;
    D32S ecusLen = 0;
    f->isNeedGetEcus = FALSE;
    ipsInstallEcuInfoArrayDeInit(buffEcus, buffEcusLen);
    ipsInstallEcuInfoArrayInit(buffEcus, buffEcusLen);

    pTriggerInfo = ipmIccInstallTriggerInfoGet();
    if(0 == adm_strcmp(pTriggerInfo, ICC_MCU_INSTALL_START))
    {
        ADM_ECS_ECU_ID_T stEcsEcuId = {0};
        (void)adm_ecs_getEcuConfigEcuId(ICC_REQ_ID, ICC_SW_DID, &stEcsEcuId);

        /**执行MCU升级*/
        buffEcus[0].ecuReqId = adm_strdup(stEcsEcuId.pEcuReqId);
        buffEcus[0].ecuSwDid = adm_strdup(stEcsEcuId.pEcuSwDid);
        buffEcus[0].ecuTimeConsuming = 5 * 60; 
        buffEcus[0].ecuTimeout = 300;  /**设置默认超时时间300s*/
        buffEcus[0].isRollback = DFALSE;

        adm_free(stEcsEcuId.pEcuReqId);
        adm_free(stEcsEcuId.pEcuSwDid);

        ecusLen = 1;
    }
    else
    {
        ecusLen = adm_ips_installEcuInfoArrayGet(buffEcus, buffEcusLen);
        if (0 == ecusLen)
        {
            return ecusLen;
        }
    }

    /** 预编程和后编程处理 */
    iRet = ipmEcuPreAndPostProgramm(buffEcus, buffEcusLen, &stPregramEcuInfo);
    if (ADM_ECODE_INSTALL_PRE_PROGRAM_FAILED == iRet ||
        ADM_ECODE_INSTALL_ENTER_HIGH_VOLTAGE_FAILED == iRet ||
        ADM_ECODE_INSTALL_EXIT_HIGH_VOLTAGE_FAILED == iRet)
    {
        for (i = 0; i < ecusLen; ++i)
        {
            if (ADM_ECODE_INSTALL_PRE_PROGRAM_FAILED == iRet)
            {
                pResultDescrip = ipmGetPreProgramEcuspResultDescrip(buffEcus[i].ecuReqId, &stPregramEcuInfo);
            }

            pstTmpNodeInfo = ipmDoFlashEcusFind(f, buffEcus[i].ecuReqId, buffEcus[i].ecuSwDid);
            if (NULL == pstTmpNodeInfo)
            {
                pstNewNode = ipmEcuInfoNodeNew(buffEcus + i);
                if (pstNewNode != NULL)
                {
                    list_add_tail(&pstNewNode->entry, &f->head);
                    f->ecusCount++;
                    ipmDoFlashEcusListStoreKeep(f, TRUE);
                    ipmDoFlashEcuEnd(f, pstNewNode, iRet, pResultDescrip);
                }
            }
            else
            {
                ipmDoFlashEcuEnd(f, pstTmpNodeInfo, iRet, pResultDescrip);
            }
            adm_free(pResultDescrip);
        }

        ipmCleanPreProgramEcus(&stPregramEcuInfo);
        return ecusLen;
    }
    ipmCleanPreProgramEcus(&stPregramEcuInfo);

    ipmReportEventDependencyEventId("IP_013", NULL, ADM_ECODE_SUCCESSED);
    for (i = 0; i < ecusLen; ++i)
    {
        pstTmpNodeInfo = ipmDoFlashEcusFind(f, buffEcus[i].ecuReqId, buffEcus[i].ecuSwDid);
        if (NULL == pstTmpNodeInfo)
        {
            pstNewNode = ipmEcuInfoNodeNew(buffEcus + i);
            if (pstNewNode != NULL)
            {
                list_add_tail(&pstNewNode->entry, &f->head);
                f->ecusCount++;
                ipmDoFlashEcusListStoreKeep(f, TRUE);
                D32S result = ipmDimFlashEcuRequest(f->dim, pstNewNode);
                if (result != ADM_ECODE_SUCCESSED)
                {
                    ipmReportEventDependencyEventId("IP_016", NULL, result);
                    ipmDoFlashEcuEnd(f, pstNewNode, result, NULL);
                }
                else
                {
                    ipmReportEventDependencyEventId("IP_015", NULL, ADM_ECODE_SUCCESSED);
                    ipmEcuInfoNodeLastTimeReset(pstNewNode);
                    if (DNULL != pstNewNode->ecsEcu && 3 == pstNewNode->ecsEcu->serviceType)
                    {
                        ipmDoFlashEcuEnd(f, pstNewNode, result, NULL);
                    }
                }
            }
        }
        else
        {
            if (TRUE == ipmIsNeedResumeInstall(pstTmpNodeInfo) &&
                TRUE == pstTmpNodeInfo->bIsNeedResume)
            {
                log_d("resume trigger install");
                pstTmpNodeInfo->bIsNeedResume = FALSE;
                D32S result = ipmDimFlashEcuRequest(f->dim, pstTmpNodeInfo);
                if (result != ADM_ECODE_SUCCESSED)
                {
                    ipmReportEventDependencyEventId("IP_016", NULL, result);
                    ipmDoFlashEcuEnd(f, pstTmpNodeInfo, result, NULL);
                }
                else
                {
                    ipmReportEventDependencyEventId("IP_015", NULL, ADM_ECODE_SUCCESSED);
                    ipmEcuInfoNodeLastTimeReset(pstTmpNodeInfo);
                    if (DNULL != pstTmpNodeInfo->ecsEcu && 3 == pstTmpNodeInfo->ecsEcu->serviceType)
                    {
                        ipmDoFlashEcuEnd(f, pstTmpNodeInfo, result, NULL);
                    }
                }
            }
        }
    }

    log_i("request flash ecus end in this round, ecus count[%d]", ecusLen);

    return ecusLen;
}

static void ipmDoFlashEcuStatEventReport(ADM_IPM_STAT_EVENT_REPORT_T* pSumary, ADM_IPM_ECU_INFO_NODE_T* pNode)
{
    IPM_STATISTICS_EVENT_E eventId = STAT_EVENT_INVALID;
    ADM_ECS_ECUCONFIG_T* pEcuCfg = pNode->ecsEcu;

    ADM_IPS_ECU_PROGRESS_T progress;
    adm_ips_ecuProgressInit(&progress);
    (void)adm_ips_ecuProgressGet(&progress, pEcuCfg->ecuReqId, pEcuCfg->ecuSwDid);
    if (ADM_IPS_ECU_PHASE_POST_INSTALL == progress.state)
    {
        if (1 == pEcuCfg->serviceType)     // VDI刷写
        {
            eventId = STAT_EVENT_VDI_FLASH;
        }
        else if (2 == pEcuCfg->serviceType)   // UCM刷写
        {
            eventId = STAT_EVENT_UCM_FLASH;
        }
		else
		{
	        ;/*null*/
		}
    }
    else if (ADM_IPS_ECU_PHASE_POST_INSTALL_ROLLBACK == progress.state)
    {
        if (1 == pEcuCfg->serviceType)     // VDI刷写
        {
            eventId = STAT_EVENT_VDI_ROLLBACK;
        }
        else if (2 == pEcuCfg->serviceType)   // UCM刷写
        {
            eventId = STAT_EVENT_UCM_ROLLBACK;
        }
		else
		{
	        ;/*null*/
		}
    }
	else
	{
        ;/*null*/
	}
    adm_ips_ecuProgressDeInit(&progress);

    if (STAT_EVENT_INVALID == eventId)
    {
        return;
    }

    IPM_STAT_EVENT_T statEvent = { 0 };
    statEvent.eventId = (D32S)eventId;
    statEvent.ecuReqId = pEcuCfg->ecuReqId;
    statEvent.ecuSwDid = pEcuCfg->ecuSwDid;
    statEvent.serviceType = pEcuCfg->serviceType;
    statEvent.fileSize = pNode->pisData.imagePackage.size;
    statEvent.startTime = pNode->startTime;
    statEvent.progress = 100;
    statEvent.endTime = (D64U)time(NULL);
    ipmStatEventReportReport(pSumary, &statEvent);
}

static void ipmDoFlashEcuEnd(ADM_IPM_DO_FLASH_T *f, ADM_IPM_ECU_INFO_NODE_T *node, D32S result, const DCHAR* pResultDescrip){
    ADM_IPS_INSTALL_ECUINFO_T *ecu = &node->ipsEcu;
    ADM_IPS_ECU_CURRENT_PROGRESS_T curProgress = {
        .ecuReqId = ecu->ecuReqId,
        .ecuSwDid = ecu->ecuSwDid,
        .isEnd = TRUE,
        .result = result
    };

    adm_snprintf(curProgress.resultDescrip,RESULT_DESCRIP_LEN,"%s", ADM_STR_SAFE_OUTPUT(pResultDescrip));


    adm_ips_currentProgressSet(&curProgress);

    if (ADM_ECODE_SUCCESSED == result)
    {
        ipmDoFlashEcuStatEventReport(&f->report->reportStatEvent, node);
    }

    list_del(&node->entry);
    f->ecusCount--;
    f->isNeedGetEcus = TRUE;
    ipmDoFlashEcusListStoreKeep(f, TRUE);

#ifdef ADM_IPM_UNZIP_ECU_PACKAGE
// del the ecu install temp unzip package file
    DCHAR *ipmPkgUnzipDir = NULL;
    do{
    //!< 获取下载升级包的解压路径
        ipmPkgUnzipDir = ipmDimgetEcuPackageUnzipDir(node->ipsEcu.ecuReqId,node->ipsEcu.ecuSwDid);
        if (ADM_IS_NULL(ipmPkgUnzipDir)){
            log_d("Failed to get ecu package unzip dir");
			break;
        }
        log_d("ecu package unzip dir[%s]", ipmPkgUnzipDir);

        if (TRUE == adm_com_isDirExist(ipmPkgUnzipDir)){
            (void)adm_com_rmDirAndFiles(ipmPkgUnzipDir);
        }
		
	    adm_free(ipmPkgUnzipDir);
    }while(FALSE);
#endif

    DCHAR eventDescTmp[128] = {0};
    adm_memset(eventDescTmp, 0x00, sizeof(eventDescTmp), sizeof(eventDescTmp));	
	
    if(ADM_ECODE_SUCCESSED == result){
        // ECU升级结束后校验版本信息
// #ifdef ADM_IPM_VERSION_VERIFY_INSTALL_HURRY        
        // (void)ipmDoVerifyEcuVersionPostInstall(ecu->ecuReqId, ecu->ecuSwDid);
// #endif
        (void)adm_snprintf(eventDescTmp, sizeof(eventDescTmp), "flash ecu successful . ECU: ecuReqId %s,ecuSwDid %s", ecu->ecuReqId,ecu->ecuSwDid);
        ipmReportEventDependencyEventId("S_IE_018",  eventDescTmp, ADM_ECODE_SUCCESSED);
    }
    else{
        (void)adm_snprintf(eventDescTmp, sizeof(eventDescTmp), "flash ecu failed . ECU: ecuReqId %s,ecuSwDid %s .errorcode:%d", ecu->ecuReqId,ecu->ecuSwDid,result);
        ipmReportEventDependencyEventId("S_IE_019",  eventDescTmp, ADM_ECODE_SUCCESSED);
    }

    ipmEcuInfoNodeDel(node);
}
static void ipmDoFlashEcuProgress(ADM_IPM_DO_FLASH_T *f, ADM_IPM_ECU_INFO_NODE_T *node, D32S progress){
    ADM_IPS_INSTALL_ECUINFO_T *ecu = &node->ipsEcu;
    ADM_IPS_ECU_CURRENT_PROGRESS_T curProgress = {
        .ecuReqId = ecu->ecuReqId,
        .ecuSwDid = ecu->ecuSwDid,
        .isEnd = FALSE,
        .progress = progress
    };
    adm_ips_currentProgressSet(&curProgress);
}
static ADM_IPM_ECU_INFO_NODE_T *ipmDoFlashEcusFind(ADM_IPM_DO_FLASH_T *f, const DCHAR *ecuReqId, const DCHAR *ecuSwDid){
    ADM_IPM_ECU_INFO_NODE_T *node = NULL;
    if(ecuReqId != NULL && ecuSwDid != NULL){
        ADM_IPM_ECU_INFO_NODE_T *pos;
        list_for_each_entry(pos, &f->head, ADM_IPM_ECU_INFO_NODE_T, entry){
		    if(pos->ipsEcu.ecuReqId != NULL && pos->ipsEcu.ecuSwDid != NULL){
				D32S reqId = ipmHtoi(pos->ipsEcu.ecuReqId);
				if(reqId == ipmHtoi(ecuReqId)){
					    D32S swDid = ipmHtoi(pos->ipsEcu.ecuSwDid);
						if(swDid == ipmHtoi(ecuSwDid)){
					    node = pos;
					    break;
					}
				}
		    }
        }
    }
    return node;
}
static struct json_object* ipmDoFlashEcusListJsonCreate(const ADM_IPM_DO_FLASH_T *f){
    struct json_object* root = json_object_new_array();
    if(NULL != root){
	ADM_IPM_ECU_INFO_NODE_T *pos;
	list_for_each_entry(pos, &f->head, ADM_IPM_ECU_INFO_NODE_T, entry){
	    struct json_object* posVal = ipmEcuInfoNodeJsonCreate(pos);
	    if(NULL != posVal){
		(void)json_object_array_add(root, posVal);
	    }
	}
    }
    return root;
}
static void ipmDoFlashEcusListJsonParse(ADM_IPM_DO_FLASH_T *f, const struct json_object* root){
    if(json_type_array == json_object_get_type(root)){
	size_t length = json_object_array_length(root);
	for(size_t i=0; i<length; ++i){
	    struct json_object* posVal = json_object_array_get_idx(root, i);
	    if(NULL != posVal){
		ADM_IPM_ECU_INFO_NODE_T* node = ipmEcuInfoNodeJsonParse(posVal);
		if(NULL != node){
		    list_add_tail(&node->entry, &f->head);
		    f->ecusCount++;
		}
	    }
	}
    }
}
static void ipmDoFlashEcusListAutoFill(ADM_IPM_DO_FLASH_T *f){
    ADM_IPM_ECU_INFO_NODE_T *pos;
    list_for_each_entry(pos, &f->head, ADM_IPM_ECU_INFO_NODE_T, entry){
	ipmEcuInfoNodeAutoFill(pos);
    }
}

static void ipmDoFlashEcusListStateUpdate(ADM_IPM_DO_FLASH_T *f)
{
    ADM_IPM_ECU_INFO_NODE_T *pos = NULL;

    list_for_each_entry(pos, &f->head, ADM_IPM_ECU_INFO_NODE_T, entry)
    {
        if (ADM_IS_NULL(pos))
        {
            continue;
        }

        pos->bIsNeedResume = TRUE;
    }

    return;
}

static void ipmDoFlashEcusListLoad(ADM_IPM_DO_FLASH_T *f){
    ADM_KV_STORAGE_VALUE_TYPE_E eType = ADM_KV_STORAGE_VALUE_TYPE_INVALID;
    DCHAR *ecuListJsonStr = NULL;
    size_t ecuListJsonStrLen = 0;
    if(adm_com_kvstorage_get_string(adm_core_service_getBusinessDatabaseDb(), ADM_IPM_DO_FLASH_ECU_LIST_KV_KEY,
                &ecuListJsonStr, &ecuListJsonStrLen, &eType)){
	if(NULL != ecuListJsonStr){
	    struct json_object* ecuListVal = json_tokener_parse(ecuListJsonStr);
	    if(NULL != ecuListVal){
		ipmDoFlashEcusListJsonParse(f, ecuListVal);
		(void)json_object_put(ecuListVal);
	    }
	    adm_free(ecuListJsonStr);
	}
    }
}
static void ipmDoFlashEcusListStore(ADM_IPM_DO_FLASH_T *f, DBOOL debug){
    struct json_object* root = ipmDoFlashEcusListJsonCreate(f);
    if(NULL != root){
	const DCHAR *ecuListJsonStr = json_object_to_json_string(root);
	if(debug){log_d("current install ecu list:\n%s", ADM_STR_SAFE_OUTPUT(ecuListJsonStr));}
	if(NULL != ecuListJsonStr){
	    (void)adm_com_kvstorage_set_string(adm_core_service_getBusinessDatabaseDb(), ADM_IPM_DO_FLASH_ECU_LIST_KV_KEY,
                ecuListJsonStr, ADM_KV_STORAGE_VALUE_TYPE_STRING);
	}
	(void)json_object_put(root);
    }
}
static void ipmDoFlashEcusListStoreKeep(ADM_IPM_DO_FLASH_T *f, DBOOL force){
    D64U curTime = ipmMonotonicTimeMSecGet();
    if(force || !((curTime - f->ecuListStoreLastTime) < ADM_IPM_FLASH_ECU_LIST_STORE_TIME_MSEC_WIDE)){
	    ipmDoFlashEcusListStore(f, (!force) ? FALSE : TRUE);
	    f->ecuListStoreLastTime = curTime;
    }
}

/* ADM_IPM_ECU_INFO_NODE_T */
static ADM_IPM_ECU_INFO_NODE_T *ipmEcuInfoNodeNew(const ADM_IPS_INSTALL_ECUINFO_T *ecu){
    ADM_IPM_ECU_INFO_NODE_T *node = ipmEcuInfoNodeNewWithoutParam();
    if(node != NULL){
	adm_ips_installEcuInfoClone(ecu, &node->ipsEcu);
	ipmEcuInfoNodeAutoFill(node);
	/*node->endTime = time(NULL) + ecu->ecuTimeout;*/
    node->startTime = (D64U)time(NULL);
	node->ecuTimeout = (D64U)ecu->ecuTimeout * 1000UL;
	/*node->lastTime = ipmMonotonicTimeMSecGet();*/
    }
    return node;
}

static void ipmEcuInfoNodeDel(ADM_IPM_ECU_INFO_NODE_T *n){
    if(n != NULL){
        adm_ips_installEcuInfoDeInit(&n->ipsEcu);
        if(n->ecsEcu != NULL){adm_ecs_delEcuConfigInfo(&n->ecsEcu);}
        adm_pis_packageDataDeInit(&n->pisData);
	    adm_free(n);
    }
}

static void ipmEcuInfoNodeLastTimeReset(ADM_IPM_ECU_INFO_NODE_T *n){
    n->lastTime = ipmMonotonicTimeMSecGet();
}
static DBOOL ipmEcuInfoNodeIsTimeOut(ADM_IPM_ECU_INFO_NODE_T *n){
    DBOOL dRet = FALSE;
    D64U curTime = ipmMonotonicTimeMSecGet();
    D64U deltaTime = curTime - n->lastTime;
    if(deltaTime < n->ecuTimeout){
	n->ecuTimeout -= deltaTime;
    }else{
	n->ecuTimeout = 0;
	dRet = TRUE;
    }
    n->lastTime = curTime;
    return dRet;
}
static ADM_IPM_ECU_INFO_NODE_T *ipmEcuInfoNodeNewWithoutParam(void){
    ADM_IPM_ECU_INFO_NODE_T *node = adm_malloc(sizeof(*(node)));
    if(node != NULL){
        adm_ips_installEcuInfoInit(&node->ipsEcu);
        node->ecsEcu = NULL;
        adm_pis_packageDataInit(&node->pisData);
        node->ecuTimeout = 0;
        node->lastTime = 0;
        node->startTime = 0;
    }
    return node;
}
// 
// static void ipmEcuInfoNodeDestroy(ADM_IPM_ECU_INFO_NODE_T** node)
// {
//     if (NULL != node)
//     {
//         ADM_IPM_ECU_INFO_NODE_T* tmp = *node;
// 
//         adm_ips_installEcuInfoDeInit(&tmp->ipsEcu);
//         adm_pis_packageDataDeInit(&tmp->pisData);
//         adm_ecs_delEcuConfigInfo(&tmp->ecsEcu);
//         adm_free(tmp);
//         *node = NULL;
//     }
// }

static void ipmEcuInfoNodeAutoFill(ADM_IPM_ECU_INFO_NODE_T *node){
    if(NULL != node->ecsEcu){
        adm_ecs_delEcuConfigInfo(&node->ecsEcu);
        node->ecsEcu = NULL;
    }
    adm_pis_packageDataDeInit(&node->pisData);
    adm_pis_packageDataInit(&node->pisData);

    const ADM_IPS_INSTALL_ECUINFO_T *ecu = &node->ipsEcu;
    if(ecu->ecuReqId != NULL && ecu->ecuSwDid != NULL){
        (void)adm_ecs_getEcuConfig(ecu->ecuReqId, ecu->ecuSwDid,  &node->ecsEcu);
        ADM_PIS_PACKAGE_HEAD_T packageHead = {
            .ecuReqId = ecu->ecuReqId,
            .ecuSwDid = ecu->ecuSwDid,
            .type = ecu->isRollback ? ADM_PIS_PACKAGE_TYPE_ECU_SRC_PACKAGE : ADM_PIS_PACKAGE_TYPE_ECU_DST_PACKAGE,
            .state = ADM_PIS_PACKAGE_STATE_VERIFY_SIGN_FINISH
        };
        (void)adm_pis_packageGet(&packageHead, &node->pisData);
    }
    node->lastTime = ipmMonotonicTimeMSecGet();
}
static struct json_object* ipmEcuInfoNodeJsonCreate(const ADM_IPM_ECU_INFO_NODE_T *node){
    struct json_object* root = json_object_new_object();
    if(NULL != root){
	struct json_object* ipsEcuVal = adm_ips_installEcuInfoJsonCreate(&node->ipsEcu);
	if(NULL != ipsEcuVal){
	    (void)json_object_object_add(root, "ipsEcu", ipsEcuVal);
	}
	struct json_object* ecuTimeoutVal = json_object_new_int64((int64_t)node->ecuTimeout);
	if(NULL != ecuTimeoutVal){
	    (void)json_object_object_add(root, "ecuTimeout", ecuTimeoutVal);
	}
	struct json_object* lastTimeVal = json_object_new_int64((int64_t)node->lastTime);
	if(NULL != lastTimeVal){
	    (void)json_object_object_add(root, "lastTime", lastTimeVal);
	}
    }
    return root;
}
static ADM_IPM_ECU_INFO_NODE_T* ipmEcuInfoNodeJsonParse(const struct json_object* root){
    ADM_IPM_ECU_INFO_NODE_T* node = ipmEcuInfoNodeNewWithoutParam();
    if(NULL != node){
	struct json_object* ipsEcuVal = json_object_object_get(root, "ipsEcu");
	if(NULL != ipsEcuVal){
	    adm_ips_installEcuInfoJsonParse(&node->ipsEcu, ipsEcuVal);
	}
	struct json_object* ecuTimeoutVal = json_object_object_get(root, "ecuTimeout");
	if(NULL != ecuTimeoutVal){
	    node->ecuTimeout = (D64U)json_object_get_int64(ecuTimeoutVal);
	}
	struct json_object* lastTimeVal = json_object_object_get(root, "lastTime");
	if(NULL != lastTimeVal){
	    node->lastTime = (D64U)json_object_get_int64(lastTimeVal);
	}
    }
    return node;
}


/* ADM_IPM_REPORT_T */
static void ipmReportInit(ADM_IPM_REPORT_T *r){
    ipmReportHmiInit(&r->reportHmi);
    ipmReportOtaServerInit(&r->reportOtaServer);
    ipmStatEventReportInit(&r->reportStatEvent);
}
static void ipmReportDeInit(ADM_IPM_REPORT_T *r){
    ipmReportHmiDeInit(&r->reportHmi);
    ipmReportOtaServerDeInit(&r->reportOtaServer);
    ipmStatEventReportDeInit(&r->reportStatEvent);
}
//static void ipmReportCondition(ADM_IPM_REPORT_T *r){
//}

static D32S ipmReportOtaServerCancelInstall(ADM_IPM_REPORT_T *r){
    return ipmReportOtaServerTaskCancelScheduleInstall(&r->reportOtaServer);
}

static D32S ipmReportOtaServerTaskVerify(ADM_IPM_REPORT_T *r)
{
    return ipmSyncReportOtaServerTaskStatusInstallPrepare(&r->reportOtaServer);
}
static void ipmReportProgressBegin(ADM_IPM_REPORT_T *r){
    ipmReportHmiProgressBegin(&r->reportHmi);
    ipmReportOtaServerProgressBegin(&r->reportOtaServer);
}
static void ipmReportProgressEnd(ADM_IPM_REPORT_T *r){
    ipmReportHmiProgressEnd(&r->reportHmi);
    ipmReportOtaServerProgressEnd(&r->reportOtaServer);
}
static void ipmReportProgressKeep(ADM_IPM_REPORT_T *r){
    ipmReportHmiProgressKeep(&r->reportHmi);
    ipmReportOtaServerProgressKeep(&r->reportOtaServer);
}
static D32S ipmReportResult(ADM_IPM_REPORT_T *r, D32S result){
    D32S ret = ipmReportHmiResult(&r->reportHmi, result);
    ipmNeedReNotiPolicyResultStore(result);   ///< 存储上电需要重新通知policyInstallResult,用户点击确定后再删除该标志
    ipmReportOtaServerResult(&r->reportOtaServer);
    return ret;
}
/*
static void ipmReportEvent(ADM_IPM_REPORT_T *r, const ADM_OCS_EVENTS_INFO_T* event){
    ipmReportOtaServerEvent(&r->reportOtaServer, event);
}
*/
static D32S ipmReportScheduleInstall(ADM_IPM_REPORT_T *r, ADM_APPLICATION_COMMON_CMDID_TYPE_E eType, D64S timestamp,ADM_APPLICATION_SCHEDULE_INSTALL_SOURCE_E eSource){
    D32S nRet = ADM_ECODE_FAILED;

    //根据需要上报预约结果至hmi

    //上报预约结果至云端
    ADM_IPM_REPORT_SERVER_DATA_EXT_LIST_T *ext_list = NULL;
    json_object* root = NULL;
    const DCHAR* rootStr = NULL;
    DCHAR szTemp[64] = {0};

    do
    {
        ext_list = adm_malloc(sizeof(ADM_IPM_REPORT_SERVER_DATA_EXT_LIST_T));
        if(ext_list == NULL)
        {
            log_e("Failed to malloc, ext_list[ %p ]", ext_list);
            nRet = ADM_ECODE_NO_MEMORY;
            break;
        }
        
        ipmReportServerDataExtListInit(ext_list);
        ext_list->extsLen = 2;
        ext_list->exts = adm_malloc(sizeof(*(ext_list->exts)) * ext_list->extsLen);
        if(ext_list->exts == NULL)
        {
            log_e("Failed to malloc, ext_list exts[ %p ]", ext_list->exts);
            nRet = ADM_ECODE_NO_MEMORY;
            break;
        }
        adm_memzero(szTemp, 64);
        (void)adm_snprintf(szTemp, sizeof(szTemp) / sizeof(DCHAR), "%d", eType);
        ipmReportServerDataExtCreate(ext_list->exts, REPORT_VEHICLE_STATE_EXTS_CMDID, szTemp);//预约升级
        adm_memzero(szTemp, 64);
        (void)adm_snprintf(szTemp, sizeof(szTemp) / sizeof(DCHAR), "%lld", timestamp);
        ipmReportServerDataExtCreate(ext_list->exts+1, REPORT_VEHICLE_STATE_EXTS_SCHEDULETIME, szTemp);//预约升级时间戳

        root = ipmReportServerDataExtListPackage(ext_list);
        if(root == NULL)
        {
            log_e("Failed to create exts object");
            nRet = ADM_ECODE_NO_MEMORY;
            break;
        }

        rootStr = json_object_to_json_string(root);
        if (rootStr == NULL) {
            log_e("Failed to create rootStr object");
            nRet = ADM_ECODE_NO_MEMORY;
            break;
        }
        log_d("rootStr[ %s ]", rootStr);
        
        int reason = 0;
        if (ADM_APPLICATION_SCHEDULE_INSTALL_APP == eSource)
        {
            reason = APP_INSTALL_SCHEDULED;
        }
        else if (ADM_APPLICATION_SCHEDULE_INSTALL_HMI == eSource)
        {
            reason = HMI_INSTALL_SCHEDULED;
        }
        log_i("report reason[%d]", reason);
        
        nRet = ipmReportOtaServerTaskScheduleInstall(&r->reportOtaServer, rootStr,reason);
        if(nRet != ADM_ECODE_SUCCESSED)
        {
            log_e("Failed to report schedule timestamp");
            break;
        }
    } while (FALSE);
    
    ipmReportServerDataExtListDeInit(ext_list);
    adm_free(ext_list);
	(void)json_object_put(root);
//    adm_free(rootStr);

    return nRet;
}


static void ipmReportUpLogFilePostInstall(D32S upgradeRes)
{
    /* DBOOL isReport = TRUE;*/
    ADM_IRM_LOG_FILE_TG_REPORT_INFO_T reportInfo = {0};

    do
    {
        /* 查看升级成功是否需要日志上报 */
        if (ADM_ECODE_SUCCESSED == upgradeRes)
        {
            reportInfo.iKeyLogFlag = 1; /** 升级成功不上报KeyLog日志*/

            /**升级成功删除KeyLog日志*/
            adm_irm_rmAllKeyLogFile();

            D32S isReportUpSuccLogFile = 0;
            if (ADM_ECODE_SUCCESSED != adm_ecs_getIsReportUpSuccLogFile(&isReportUpSuccLogFile))
            {
                log_e("Failed to get config of isReportUpSuccLogFile");
                break;
            }

            if (2 == isReportUpSuccLogFile)
            {
                break;
            }
            else if (1 != isReportUpSuccLogFile)
            {
                log_e("Error value of isReportUpSuccLogFile, value[%d]", isReportUpSuccLogFile);
                break;
            }
			else
			{
		        ;/*null*/
			}
        }

        // 触发日志上报
        reportInfo.cmdId = ADM_OCS_CMDID_INSTALL_FAILED;
        reportInfo.extsReqJsonStr = NULL;
        reportInfo.pushId = NULL;
        adm_irm_triggerLogFileReport(&reportInfo);
    } while (FALSE);
}

/* ADM_IPM_REPORT_OTA_SERVER_T */
static void ipmReportOtaServerInit(ADM_IPM_REPORT_OTA_SERVER_T *s){
//    s->session = irsSession;
    s->lastTime = ipmMonotonicTimeMSecGet();
}
static void ipmReportOtaServerDeInit(ADM_IPM_REPORT_OTA_SERVER_T *s){
}
/*
static void ipmReportOtaServerCondition(ADM_IPM_REPORT_OTA_SERVER_T *s){
    ipmReportOtaServerTaskStatusInstallPrepare(s);
}
*/
static void ipmReportOtaServerProgressBegin(ADM_IPM_REPORT_OTA_SERVER_T *s){
    ipmReportOtaServerTaskStatusInstallStart(s);
    ipmReportOtaServerEcuStatus(s);
    ipmReportOtaServerProgress(s, TRUE);
}
static void ipmReportOtaServerProgressEnd(ADM_IPM_REPORT_OTA_SERVER_T *s){
    ipmReportOtaServerProgress(s, TRUE);
//    ipmReportOtaServerEcuStatus(s); //重启后再上报ecu状态
}
static void ipmReportOtaServerProgressKeep(ADM_IPM_REPORT_OTA_SERVER_T *s){
    ipmReportOtaServerProgress(s, FALSE);
}
static void ipmReportOtaServerProgress(ADM_IPM_REPORT_OTA_SERVER_T *s, DBOOL force){
    if(ipmReportOtaServerProgressTimeOut(s, force)){
	ipmReportOtaServerEcuProgress(s);
    }
}
static void ipmReportOtaServerResult(ADM_IPM_REPORT_OTA_SERVER_T *s){
    ipmReportOtaServerTaskStatusInstallEnd(s);
}
static DBOOL ipmReportOtaServerProgressTimeOut(ADM_IPM_REPORT_OTA_SERVER_T *s, DBOOL force){
    DBOOL dRet = FALSE;
    D64U tv = ipmMonotonicTimeMSecGet();
    if(!force){
        if((tv - s->lastTime) < ADM_IPM_REPORT_OTA_SERVER_ECU_PROGRESS_WIDE_TIME_MSEC){}else{
            s->lastTime = tv;
            dRet = TRUE;
        }
    }else{
	s->lastTime = tv;
	dRet = TRUE;
    }
    return dRet;
}
static void ipmReportOtaServerTaskStatus(ADM_IPM_REPORT_OTA_SERVER_T *s, const DCHAR *state, const DCHAR *reason, const DCHAR *exts){
    D32S ret = ADM_ECODE_FAILED;
    DCHAR* vehicleVersion = NULL;
    ADM_IPS_POLICY_TASK_T task;
    adm_ips_policyTaskInit(&task);
//    adm_ips_policyTaskGet(&task);
    if(ADM_IS_SUCCESS(adm_ips_policyTaskGet(&task)))
	{
        if (0 == adm_strcmp(state, ADM_IRM_REPORT_SERVER_STATUS_Task_Install_Successed))
        {
            adm_cvs_getHmiVehicleVersion(&vehicleVersion);
            if (NULL == vehicleVersion)
            {
#ifdef AIR_PROTOCOL_VEHICLE_VERSION
            vehicleVersion = adm_strdup(AIR_PROTOCOL_VEHICLE_VERSION);
#endif            
            }
            adm_eim_setVehicleVersion(vehicleVersion);
        }

	    ret = adm_irm_reportVehicleState(time(NULL), ADM_SDK_VERSION, task.sessionId, task.taskId, task.strategyId, state, reason, task.versionNumber, vehicleVersion, exts);
	}
    if(ADM_ECODE_SUCCESSED != ret){
	log_w("adm_irm_reportVehicleState failed %d", ret);
    }

    adm_free(vehicleVersion);
    adm_ips_policyTaskDeInit(&task);
}

static D32S ipmSyncReportOtaServerTaskStatus(ADM_IPM_REPORT_OTA_SERVER_T *s, const DCHAR *state, const DCHAR *reason, const DCHAR *exts)
{
    D32S nRet = ADM_ECODE_FAILED;
    ADM_IPS_POLICY_TASK_T task;
    adm_ips_policyTaskInit(&task);
    ADM_APPLICATION_COMMON_CMDID_TYPE_E eTriggerType = ADM_APPLICATION_COMMON_CMDID_TYPE_INVALID;

    eTriggerType = uimSoaCmdidTypeGet();
//    adm_ips_policyTaskGet(&task);
    if(ADM_IS_SUCCESS(adm_ips_policyTaskGet(&task)))
	{
	    nRet = adm_irm_syncReportVehicleState(time(NULL), ADM_SDK_VERSION, task.sessionId, task.taskId, task.strategyId, state, reason, task.versionNumber, exts);
	}
    if (ADM_ECODE_SUCCESSED != nRet)
    {
        if (ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_USB == eTriggerType)
        {
            log_i("usb install no check mode,need not veriry task.");
            nRet = ADM_ECODE_SUCCESSED;
        }
        else 
        {
            log_w("adm_irm_reportVehicleState failed, nRet[ %d ]", nRet);
        }
    }
    adm_ips_policyTaskDeInit(&task);

    return nRet;
}

static D32S ipmReportOtaServerTaskStatusEnterOtaModeFailed(ADM_IPM_REPORT_OTA_SERVER_T *s)
{
    DCHAR reason[128] = {0};
    (void)adm_snprintf(reason, sizeof(reason) / sizeof(DCHAR), "%d", ADM_ECODE_INSTALL_ENTER_OTAMODE_FAILED);
    ipmReportOtaServerTaskStatus(s, ADM_IRM_REPORT_SERVER_STATUS_INSTALL_Enter_OTA_Mode_Failed, reason, NULL);
    return ADM_ECODE_SUCCESSED;
}

static D32S ipmReportOtaServerTaskStatusTaskStop(ADM_IPM_REPORT_OTA_SERVER_T *s, D32S iResult)
{
    DCHAR reason[128] = {0};
    D32S iRet = ADM_ECODE_SUCCESSED;
    ADM_CVS_UPGRADE_MODE_E eUpgradeMode = ADM_CVS_UPGRADE_MODE_INVALID;

    iRet = adm_cvs_getUpgradeMode((D32S *)&eUpgradeMode);

    /** 静默升级任务（强制升级，静默下载静默安装，OTA自升级)，下载阶段、安装阶段出现任何失败，上报云端"任务失败" */
    if(ADM_IS_SUCCESS(iRet))
    {
        if (ADM_CVS_UPGRADE_MODE_FORCE == eUpgradeMode ||
            ADM_CVS_UPGRADE_MODE_SLIENCE_INSTALL == eUpgradeMode ||
            ADM_CVS_UPGRADE_MODE_APK_SELF == eUpgradeMode)
        {
            (void)adm_snprintf(reason, sizeof(reason) / sizeof(DCHAR), "%d", iResult);
            ipmReportOtaServerTaskStatus(s, ADM_IRM_REPORT_SERVER_STATUS_Task_Failed_UC_IDLE, reason, NULL);
        }
    }

    return ADM_ECODE_SUCCESSED;
}

static D32S ipmReportOtaServerTaskScheduleInstall(ADM_IPM_REPORT_OTA_SERVER_T *s, const DCHAR *exts, int reason)
{
    DCHAR reasonStr[128] = {0};
    (void)adm_snprintf(reasonStr, sizeof(reasonStr) / sizeof(DCHAR), "%d", reason);
    log_i("report reasonStr[ %s ]", reasonStr);
    ipmReportOtaServerTaskStatus(s, ADM_IRM_REPORT_SERVER_STATUS_Task_Install_Schduled, reasonStr, exts);
    return ADM_ECODE_SUCCESSED;
}

static void ipmReportOtaServerTaskScheduleInstallFailed(ADM_IPM_REPORT_OTA_SERVER_T *s,int reason)
{
    DCHAR reasonStr[128] = {0};
    (void)adm_snprintf(reasonStr, sizeof(reasonStr) / sizeof(DCHAR), "%d", reason);
    log_i("report reasonStr[ %s ]", reasonStr);
    ipmReportOtaServerTaskStatus(s, ADM_IRM_REPORT_SERVER_STATUS_Task_Install_Schduled_Failed, reasonStr, NULL);
    return ;
}

static void ipmReportOtaServerTaskScheduleInstallCancel(ADM_IPM_REPORT_OTA_SERVER_T *s,int reason)
{
    DCHAR reasonStr[128] = {0};
    (void)adm_snprintf(reasonStr, sizeof(reasonStr) / sizeof(DCHAR), "%d", reason);
    log_i("report reasonStr[ %s ]", reasonStr);
    ipmReportOtaServerTaskStatus(s, ADM_IRM_REPORT_SERVER_STATUS_Task_Install_Schduled_Canceled, reasonStr, NULL);
    return ;
}
static void ipmReportOtaServerTaskScheduleInstallCancelFailed(ADM_IPM_REPORT_OTA_SERVER_T *s,int reason)
{
    DCHAR reasonStr[128] = {0};
    (void)adm_snprintf(reasonStr, sizeof(reasonStr) / sizeof(DCHAR), "%d", reason);
    log_i("report reasonStr[ %s ]", reasonStr);
    ipmReportOtaServerTaskStatus(s, ADM_IRM_REPORT_SERVER_STATUS_Task_Install_Schduled_Canceled_Failed, reasonStr, NULL);
    return ;
}

static D32S ipmReportOtaServerTaskCancelScheduleInstall(ADM_IPM_REPORT_OTA_SERVER_T *s)
{
    ipmReportOtaServerTaskStatus(s, ADM_IRM_REPORT_SERVER_STATUS_Task_Install_Canceled_Can_ReInstall, "0", NULL);
    return ADM_ECODE_SUCCESSED;
}

static void ipmReportOtaServerTaskStatusInstallPrepare(ADM_IPM_REPORT_OTA_SERVER_T *s){
    ipmReportOtaServerTaskStatus(s, ADM_IRM_REPORT_SERVER_STATUS_Task_Install_Prepare, "0", NULL);
}

static D32S ipmSyncReportOtaServerTaskStatusInstallPrepare(ADM_IPM_REPORT_OTA_SERVER_T *s)
{
    return ipmSyncReportOtaServerTaskStatus(s, ADM_IRM_REPORT_SERVER_STATUS_Task_Install_Prepare, "0", NULL);
}

static void ipmReportOtaServerTaskStatusInstallStart(ADM_IPM_REPORT_OTA_SERVER_T *s){
    ipmReportOtaServerTaskStatus(s, ADM_IRM_REPORT_SERVER_STATUS_Task_Install_Ing, "0", NULL);
}
static void ipmReportOtaServerTaskStatusInstallEnd(ADM_IPM_REPORT_OTA_SERVER_T *s){
    ADM_IPS_PROGRESS_T progress;
    adm_ips_progressInit(&progress);
    adm_ips_progressGet(&progress);
    const DCHAR *state = NULL;
    DCHAR result[128] = {0};
    if(progress.state == ADM_IPS_PHASE_POST_INSTALL){

        // 当条件校验失败、进入OTA模式失败时，已经直接上报汽车状态，在此不再上报
        if (progress.result == ADM_ECODE_INSTALL_CONDITION_FAILED ||
            progress.result == ADM_ECODE_INSTALL_ENTER_OTAMODE_FAILED)
        { 
            adm_ips_progressDeInit(&progress);
            return;
        }

        (void)adm_snprintf(result, sizeof(result) / sizeof(DCHAR), "%d", progress.result);
        if (progress.result == 0) {
            state = ADM_IRM_REPORT_SERVER_STATUS_Task_Install_Successed;
        }
        else {
            state = ADM_IRM_REPORT_SERVER_STATUS_Task_Install_Failed;
        }
    }else{
        (void)adm_snprintf(result, sizeof(result) / sizeof(DCHAR), "%d", ADM_ECODE_FAILED);
        state = ADM_IRM_REPORT_SERVER_STATUS_Task_Install_Not_Finished;
    }

    ipmReportOtaServerTaskStatus(s, state, result, NULL);
    adm_ips_progressDeInit(&progress);
}

static void ipmReportOtaServerEcuStatus(ADM_IPM_REPORT_OTA_SERVER_T *s){
    ADM_IPS_POLICY_TASK_T task;
	D32S ret = ADM_ECODE_FAILED;
		
    adm_ips_policyTaskInit(&task);
//    adm_ips_policyTaskGet(&task);
	if(ADM_IS_SUCCESS(adm_ips_policyTaskGet(&task)))
	{
	    ADM_IPS_PROGRESS_T progress;
	    adm_ips_progressInit(&progress);
	    adm_ips_progressGet(&progress);
	    ADM_OCS_ECU_STATE_INFO_LIST_T ecuStateList;
	    ocsEcuStateInfoListInit(&ecuStateList, &progress);
        do{
    
            ret = ocsEcuStateInfoListGetecuDids(&ecuStateList);
		    if(ADM_ECODE_SUCCESSED != ret)
		    {
			    log_w("adm_irm_reportEcuState failed %d", ret);
			    break;
	        }

	        ret = adm_irm_reportEcuState(task.sessionId, task.taskId, task.strategyId, time(NULL), ADM_SDK_VERSION, NULL, &ecuStateList);
	        if(ADM_ECODE_SUCCESSED != ret)
		    {
			    log_w("adm_irm_reportEcuState failed %d", ret);
			    break;
	        }
	
        }while(FALSE);
	    ocsEcuStateInfoListDeInit(&ecuStateList);
	    adm_ips_progressDeInit(&progress);
	}
    adm_ips_policyTaskDeInit(&task);
}
static void ipmReportOtaServerEcuProgress(ADM_IPM_REPORT_OTA_SERVER_T *s){
    ADM_IPS_POLICY_TASK_T task;
    adm_ips_policyTaskInit(&task);
//    adm_ips_policyTaskGet(&task);
	if(ADM_IS_SUCCESS(adm_ips_policyTaskGet(&task)))
	{
	    ADM_IPS_PROGRESS_T progress;
	    adm_ips_progressInit(&progress);
	    adm_ips_progressGet(&progress);
	    ADM_OCS_ECU_PROGRESS_INFO_LIST_T ecuProgressList;
	    ocsEcuProgressInfoListInit(&ecuProgressList, &progress);
	    D32S ret = adm_irm_reportVehicleProgress(task.sessionId, task.taskId, task.strategyId, time(NULL), ADM_SDK_VERSION, NULL, &ecuProgressList);
	    if(ADM_ECODE_SUCCESSED != ret){
		log_w("adm_irm_reportVehicleProgress failed %d", ret);
	    }
	    ocsEcuProgressInfoListDeInit(&ecuProgressList);
	    adm_ips_progressDeInit(&progress);
	}
    adm_ips_policyTaskDeInit(&task);
}
/*
static void ipmReportOtaServerEvent(ADM_IPM_REPORT_OTA_SERVER_T *s, const ADM_OCS_EVENTS_INFO_T* event){
    ADM_IPS_POLICY_TASK_T task;
    adm_ips_policyTaskInit(&task);
    adm_ips_policyTaskGet(&task);
    ADM_OCS_EVENTS_INFO_LIST_T eventsList = {
	.reportEvents = (ADM_OCS_EVENTS_INFO_T*)event,
	.reportEventsLen = 1
    };
    D32S ret = adm_irm_reportEvent(task.sessionId, task.taskId, task.strategyId, time(NULL), ADM_SDK_VERSION, NULL, &eventsList);
    if(ADM_ECODE_SUCCESSED != ret){
	log_w("adm_irm_reportEvent failed %d", ret);
    }
    adm_ips_policyTaskDeInit(&task);
}
*/

/* ADM_IPM_REPORT_HMI_T */
static void ipmReportHmiInit(ADM_IPM_REPORT_HMI_T *h){
    h->lastTime = ipmMonotonicTimeMSecGet();
    h->sessionId = NULL;
    h->ecusIsReportEnd = NULL;
    h->pcConditionResult = NULL;
    h->ecusLen = 0;
}
static void ipmReportHmiDeInit(ADM_IPM_REPORT_HMI_T *h)
{
    if (h->sessionId != NULL)
    {
        adm_free(h->sessionId);
    }
    if (h->ecusIsReportEnd != NULL)
    {
        adm_free(h->ecusIsReportEnd);
    }
    if (h->pcConditionResult != NULL)
    {
        adm_free(h->pcConditionResult);
    }
}
static void ipmReportHmiProgressBegin(ADM_IPM_REPORT_HMI_T *h){
    ipmReportHmiProgress(h, TRUE);
}
static void ipmReportHmiProgressEnd(ADM_IPM_REPORT_HMI_T *h){
    ipmReportHmiProgress(h, TRUE);
}
static void ipmReportHmiProgressKeep(ADM_IPM_REPORT_HMI_T *h){
    ipmReportHmiProgress(h, FALSE);
}
static void ipmReportHmiProgress(ADM_IPM_REPORT_HMI_T *h, DBOOL force){
    if(ipmReportHmiProgressTimeOut(h, force)){
        ADM_IPS_PROGRESS_T progress;
        adm_ips_progressInit(&progress);
        adm_ips_progressGet(&progress);

        // 进度小于15%，不上报，这15%的进度让给环境准备中；
        if (HMI_PROGRESS_SIMULATE >= progress.progress && 
            g_iSimulateProgress >= progress.progress)
        {
            log_d("real progress[%u], simulate progress[%u] do not handle.", progress.progress, g_iSimulateProgress);
            return;
        }
        
        if (g_iProgress >= progress.progress)
        {
            log_d("curProgress[%u] newProgress[%u], do not handle.", 
            g_iProgress, progress.progress);
            return;
        }

        if (100 ==  progress.progress)
        {
            g_iProgress = 0;
        }
        else 
        {
            g_iProgress = progress.progress;
        }

	    ipmUimSoaPolicyInstallProgressNotify(h->uim, &progress);
        if(ipmReportHmiProgressUpdate(h, &progress)){
            if (progress.ecus.ecus != NULL && h->ecusIsReportEnd != NULL) {
                for (D32S i = 0; i < progress.ecus.ecusLen; ++i) {
                    const ADM_IPS_ECU_PROGRESS_T* ecuProgress = progress.ecus.ecus + i;
                    if (ecuProgress->state == ADM_IPS_ECU_PHASE_INSTALL
                        || ecuProgress->state == ADM_IPS_ECU_PHASE_INSTALL_ROLLBACK) {
                        h->ecusIsReportEnd[i] = FALSE;

                        /**雄狮项目不需要通知单个Ecu进度 */
                        //ipmUimSoaEcuInstallProgressNotify(h->uim, ecuProgress);
                    }
                    else if (ecuProgress->state == ADM_IPS_ECU_PHASE_POST_INSTALL
                        || ecuProgress->state == ADM_IPS_ECU_PHASE_POST_INSTALL_ROLLBACK) {
                        if (!h->ecusIsReportEnd[i]) {

                            /**雄狮项目不需要通知单个Ecu结果 */
                            //ipmUimSoaEcuInstallResultNotify(h->uim, ecuProgress);
                            h->ecusIsReportEnd[i] = TRUE;
                        }
                    }
					else
					{
				        ;/*null*/
					}	
                }
            }
        }
        adm_ips_progressDeInit(&progress);
    }
}
static D32S ipmHmiProgressSimulateInit(KvStorage_t* pstOtaKvHandle, D32U* curProgress)
{
    D32S iRet = ADM_ECODE_FAILED;
    DBOOL bIsOk = FALSE;
    size_t valLength = 0;
    DCHAR *curProgressStr = NULL;
    ADM_KV_STORAGE_VALUE_TYPE_E eValType;

    if (ADM_IS_NULL(pstOtaKvHandle) || ADM_IS_NULL(curProgress))
    {
        log_e( "Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }

    bIsOk = adm_com_kvstorage_get_string(pstOtaKvHandle, ENVIRONMENT_WAIT_PROGRESS, 
                            &curProgressStr, &valLength, &eValType);
    if (FALSE == bIsOk)
    {
        log_e("Failed to get adm_com_kvstorage_get_string, key[%s]", ENVIRONMENT_WAIT_PROGRESS);
        return ADM_ECODE_KV_GET_STRING_FAILED;
    }
    (void)adm_stringToUint32(curProgressStr, curProgress);
    adm_free(curProgressStr);
    log_i("get curProgress = %u", *curProgress);

    return ADM_ECODE_SUCCESSED;
}
static D32S ipmSetHmiProgressSimulate
(
    KvStorage_t         *pstOtaKvHandle, 
    D32S                 curProgress
)
{
    DBOOL bIsOk = FALSE;
    DCHAR curProgressStr[16] = {0};

    if (ADM_IS_NULL(pstOtaKvHandle))
    {
        log_e("Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }

    //!< 将状态值转为字符串，存储到kv中
    (void)adm_memset(curProgressStr, 0x00, sizeof(curProgressStr), sizeof(curProgressStr));
    (void)adm_snprintf(curProgressStr, sizeof(curProgressStr), "%d", curProgress);

    log_i("set curProgressStr = %s", curProgressStr);
 
    bIsOk = adm_com_kvstorage_set_string(pstOtaKvHandle, ENVIRONMENT_WAIT_PROGRESS, 
                                    curProgressStr, ADM_KV_STORAGE_VALUE_TYPE_UINT32);
    if (FALSE == bIsOk)
    {
        log_e("Failed to set adm_com_kvstorage_set_string, key[%s]", ENVIRONMENT_WAIT_PROGRESS);
        return ADM_ECODE_KV_SET_STRING_FAILED;
    }
    log_i("set success...");

    return ADM_ECODE_SUCCESSED;
}
static void ipmEnvironmentWaitProgressDBRemove()
{
    g_iSimulateProgress = 0;
    g_bIsSimulateFinished = DFALSE;

    if (DTRUE == adm_com_kvstorage_has_key(adm_core_service_getBusinessDatabaseDb(), ENVIRONMENT_WAIT_PROGRESS))
    {
        if (DTRUE == adm_com_kvstorage_remove_key(adm_core_service_getBusinessDatabaseDb(), ENVIRONMENT_WAIT_PROGRESS))
        {
            log_i("key:[%s] has removed success.", ENVIRONMENT_WAIT_PROGRESS);
        }
        else 
        {
            log_w("key:[%s] has removed failed.", ENVIRONMENT_WAIT_PROGRESS);
        }
    }

    return;
}

static void* ipmReportHmiProgressSimulate(void* arg)
{
    log_i("start ipmReportHmiProgressSimulate. Current simulate progress[%u].", g_iSimulateProgress);

    ADM_IPM_REPORT_HMI_T *h = (ADM_IPM_REPORT_HMI_T*)arg;
    D32S iRet = 0;
    D32U uiSleepSec = 1;

    while (g_iSimulateProgress < HMI_PROGRESS_SIMULATE)
    {
        if (DTRUE == g_bIsSimulateFinished)
        {
            log_i("Need finish simulate progress.");
            break;
        }

        if(g_iProgress > g_iSimulateProgress)
        {
            log_i("real progress[%u] simulate progress[%u], don't simulate progress.", g_iProgress, g_iSimulateProgress);
            break;
        }

        ADM_IPS_PROGRESS_T progress = {0};
        adm_ips_progressInit(&progress);
        adm_ips_progressGet(&progress);

        if (ADM_ECODE_SUCCESSED != progress.result)
        {
            log_w("Flash failed, don't simulate progress.");
            break;
        }
        
        progress.progress = g_iSimulateProgress;

        // log_i("simulate progress is %u ",g_iSimulateProgress);
        ipmUimSoaPolicyInstallProgressNotify(h->uim, &progress);
        adm_ips_progressDeInit(&progress);

        g_iSimulateProgress++;
        uiSleepSec += 2;

        iRet = ipmSetHmiProgressSimulate(adm_core_service_getBusinessDatabaseDb(),g_iSimulateProgress);
        if (iRet!= ADM_ECODE_SUCCESSED)
        {
            log_w("ipmSetHmiProgressSimulate failed, iRet = %d iCurProgress %u", iRet,g_iSimulateProgress);
        }

        sleep(uiSleepSec);
    }

    ipmEnvironmentWaitProgressDBRemove();

    log_i("end ipmReportHmiProgressSimulate");
}

static D32S ipmReportHmiResult(ADM_IPM_REPORT_HMI_T *h, D32S result){
    ADM_IPS_PROGRESS_T progress;
    adm_ips_progressInit(&progress);
    adm_ips_progressGet(&progress);
    D32S ret = ipmUimSoaPolicyInstallResultNotify(h->uim, &progress, result, h->pcConditionResult);
    adm_ips_progressDeInit(&progress);
    ipmReportHmiProgressClear(h);
    return ret;
}
static DBOOL ipmReportHmiProgressUpdate(ADM_IPM_REPORT_HMI_T *h, const ADM_IPS_PROGRESS_T *progress){
    DBOOL dRet = FALSE;
    if(progress->sessionId != NULL && progress->ecus.ecusLen != 0){
        if(h->sessionId != NULL){
            if(adm_strcmp(progress->sessionId, h->sessionId) == 0 && progress->ecus.ecusLen == h->ecusLen){
                dRet = TRUE;
            }
        }
        if(!dRet){
	    ipmReportHmiProgressClear(h);
	    h->sessionId = adm_strdup(progress->sessionId);
	    h->ecusIsReportEnd = adm_malloc((D32U)progress->ecus.ecusLen *sizeof(*(h->ecusIsReportEnd)));
	    if(h->ecusIsReportEnd != NULL){
		h->ecusLen = progress->ecus.ecusLen;
		for(D32S i=0; i<h->ecusLen; ++i){
		    h->ecusIsReportEnd[i] = FALSE;
		}
		dRet = TRUE;
	    }
        }
    }
    return dRet;
}
static DBOOL ipmReportHmiProgressTimeOut(ADM_IPM_REPORT_HMI_T *h, DBOOL force){
    DBOOL dRet = FALSE;
    D64U tv = ipmMonotonicTimeMSecGet();
    if(!force){
        if((tv - h->lastTime) < ADM_IPM_REPORT_HMI_ECU_PROGRESS_WIDE_TIME_MSEC){}else{
            h->lastTime = tv;
            dRet = TRUE;
        }
    }else{
	h->lastTime = tv;
	dRet = TRUE;
    }
    return dRet;
}
static void ipmReportHmiProgressClear(ADM_IPM_REPORT_HMI_T *h){
    adm_free(h->sessionId); 
    adm_free(h->ecusIsReportEnd);
    adm_free(h->pcConditionResult);
    h->ecusLen = 0;
}

static struct json_object* ipmReportServerDataExtListPackage(const ADM_IPM_REPORT_SERVER_DATA_EXT_LIST_T *exts){
    struct json_object *root = json_object_new_array();
    if(NULL != root){
	if(NULL != exts->exts){
	    for(D32U i=0; i<exts->extsLen; ++i){
                struct json_object *extVal = ipmReportServerDataExtPackage(exts->exts + i);
		if(NULL != extVal){
                    (void)json_object_array_add(root, extVal);
                }
            }
        }
    }
    return root;
}


/* ADM_IPM_REPORT_SERVER_DATA_EXT_LIST_T */
static void ipmReportServerDataExtListInit(ADM_IPM_REPORT_SERVER_DATA_EXT_LIST_T *exts){
    exts->exts = NULL;
    exts->extsLen = 0;
}
static void ipmReportServerDataExtListDeInit(ADM_IPM_REPORT_SERVER_DATA_EXT_LIST_T *exts){
    if(NULL != exts->exts){
	for(D32U i=0; i<exts->extsLen; ++i){
            ipmReportServerDataExtDeInit(exts->exts + i);
        }
        adm_free(exts->exts);
    }
}

static void ipmReportServerDataExtDeInit(ADM_IPM_REPORT_SERVER_DATA_EXT_T *ext){
    adm_free(ext->extKey);
    adm_free(ext->extValue);
}

static void ipmReportServerDataExtCreate(ADM_IPM_REPORT_SERVER_DATA_EXT_T *ext, const DCHAR *key, DCHAR *value)
{
    if (NULL != key)
    {
        ext->extKey = adm_strdup(key);
    }
    if (NULL != value)
    {
        ext->extValue = adm_strdup(value);
    }
}


static struct json_object* ipmReportServerDataExtPackage(const ADM_IPM_REPORT_SERVER_DATA_EXT_T *ext){
    struct json_object *root = json_object_new_object();
    if(NULL != root){
	if(NULL != ext->extKey){(void)json_object_object_add(root, "extKey", json_object_new_string(ext->extKey));}
	if(NULL != ext->extValue){(void)json_object_object_add(root, "extValue", json_object_new_string(ext->extValue));}
    }
    return root;
}



/* ADM_IPM_UIM_T callback */
//static void ipmUimSchduleInstallCallback(const ADM_APPLICATION_COMMON_CMDID_TYPE_E eCmdidType, const ADM_UIM_SOA_INSTALL_SCHDULE_TYPE_E eSchduleType, const D16U schduleTime, ADM_UIM_SOA_ACTION_AUTH_TYPE_E* eActionType, void* priv){}
//static void ipmUimCancelInstallCallback(const ADM_APPLICATION_COMMON_CMDID_TYPE_E eCmdidType, ADM_UIM_SOA_ACTION_AUTH_TYPE_E* eActionType, void* priv){}
//static void ipmUimRefuseInstallCallback(const ADM_APPLICATION_COMMON_CMDID_TYPE_E eCmdidType, ADM_UIM_SOA_ACTION_AUTH_TYPE_E* eActionType){}
/*
static void ipmUimInstallTriggerCallback(ADM_UIM_SOA_ACTION_AUTH_TYPE_E* eActionType, void* priv){
    log_d("%s:%p, %s:%p", 
	   ADM_STR_FROM_PARAM(eActionType), eActionType,
	   ADM_STR_FROM_PARAM(priv), priv);
    if(eActionType && priv){
        ADM_IPM_UIM_T *uim = (ADM_IPM_UIM_T *)priv;
#if 1
	if(uim->fnCmdDriverClick){uim->fnCmdDriverClick(uim->priv);};
#else
        ADM_IPM_EVENT_DATA_TRIGGER_T trigger;
        ipmEventDataTriggerInit(&trigger, NULL);
	ipmEventSendTrigger(uim->event, ADM_IPM_EVENT_ID_CMD_START, &trigger);
        ipmEventDataTriggerDeInit(&trigger);
#endif
	*eActionType = ADM_UIM_SOA_ACTION_AUTH_TYPE_ALLOW;
    }
}
*/
static void ipmUimConditionsVerifiedCallback(const ADM_UIM_SOA_CONDITIONS_VERIFIED_TYPE_E eVerifiedType, const D16U cycleTime, const D16U timeout, D32S* errorCode, void* priv){
    log_d("%s:%d, %s:%u, %s:%u, %s:%p, %s:%p", 
	   ADM_STR_FROM_PARAM(eVerifiedType), eVerifiedType, 
	   ADM_STR_FROM_PARAM(cycleTime), cycleTime, 
	   ADM_STR_FROM_PARAM(timeout), timeout, 
	   ADM_STR_FROM_PARAM(errorCode), errorCode, 
	   ADM_STR_FROM_PARAM(priv), priv);
    if(errorCode != NULL && priv != NULL){
        ADM_IPM_UIM_T *uim = (ADM_IPM_UIM_T *)priv;
	if(!ipmUimConditionsVerified(uim, eVerifiedType, cycleTime, timeout)){
	    *errorCode = ADM_ECODE_FAILED;
	}else{
	    *errorCode = ADM_ECODE_SUCCESSED;
	}
    }
}
static D32S ipmUimAllEcuInstallProgressCallback(ADM_APPLICATION_COMMON_CMDID_TYPE_E* eCmdidType, DCHAR** sessionId, DCHAR** state, ADM_UIM_SOA_ECU_INSTALL_PROGRESS_ARRAY_INFO_T** pstEcuInstallProgressArrayInfo,  ADM_UIM_SOA_INSTALL_PROGRESS_INFO_T** pstPolicyInstallProgress, D32S* errorCode, void* priv){
    log_d("%s:%p, %s:%p, %s:%p, %s:%p, %s:%p, %s:%p, %s:%p", 
	   ADM_STR_FROM_PARAM(eCmdidType), eCmdidType,
	   ADM_STR_FROM_PARAM(sessionId), sessionId,
	   ADM_STR_FROM_PARAM(state), state,
	   ADM_STR_FROM_PARAM(pstEcuInstallProgressArrayInfo), pstEcuInstallProgressArrayInfo,
	   ADM_STR_FROM_PARAM(pstPolicyInstallProgress), pstPolicyInstallProgress,
	   ADM_STR_FROM_PARAM(errorCode), errorCode,
	   ADM_STR_FROM_PARAM(priv), priv);
    if((NULL != eCmdidType) && (NULL != sessionId) && (NULL != state) && (NULL != pstEcuInstallProgressArrayInfo) && (NULL != pstPolicyInstallProgress) && (NULL != errorCode) && (NULL != priv)){
//        ADM_IPM_UIM_T *uim = (ADM_IPM_UIM_T *)priv;
	ADM_IPS_PROGRESS_T progress;
        adm_ips_progressInit(&progress);
	adm_ips_progressGet(&progress);
	*eCmdidType = uimSoaCmdidTypeGet();
	*sessionId = uimSoaVehicleSessionId();
	*state = NULL;
	const DCHAR *tempstate = uimSoaVehicleState(&progress);
	if(NULL != tempstate){*state = adm_strdup(tempstate);}
	*pstEcuInstallProgressArrayInfo = uimSoaEcuInstallProgressArrayInfoNew(&progress.ecus);
	*pstPolicyInstallProgress = uimSoaInstallProgressInfoNewVehicle(&progress);
        adm_ips_progressDeInit(&progress);
	*errorCode = ADM_ECODE_SUCCESSED;
	return ADM_ECODE_SUCCESSED;
    }
    return ADM_ECODE_INVALID_PARAM;
}
static D32S ipmUimAllEcuInstallResultCallback(ADM_APPLICATION_COMMON_CMDID_TYPE_E* eCmdidType, DCHAR** sessionId, DCHAR** state, ADM_UIM_SOA_ECU_INSTALL_RESULT_ARRAY_INFO_T** pstEcuInstallResultArrayInfo, ADM_UIM_SOA_INSTALL_RESULT_INFO_T** pstPolicyInstallResult, D32S* errorCode, void* priv){
    log_d("%s:%p, %s:%p, %s:%p, %s:%p, %s:%p, %s:%p, %s:%p",
	   ADM_STR_FROM_PARAM(eCmdidType), eCmdidType,
	   ADM_STR_FROM_PARAM(sessionId), sessionId,
	   ADM_STR_FROM_PARAM(state), state,
	   ADM_STR_FROM_PARAM(pstEcuInstallResultArrayInfo), pstEcuInstallResultArrayInfo,
	   ADM_STR_FROM_PARAM(pstPolicyInstallResult), pstPolicyInstallResult,
	   ADM_STR_FROM_PARAM(errorCode), errorCode,
	   ADM_STR_FROM_PARAM(priv), priv);
    if((NULL != eCmdidType) && (NULL != sessionId) && (NULL != state) && (NULL != pstEcuInstallResultArrayInfo) && (NULL != pstPolicyInstallResult) && (NULL != errorCode) && (NULL != priv)){
//        ADM_IPM_UIM_T *uim = (ADM_IPM_UIM_T *)priv;
        ADM_IPS_PROGRESS_T progress;
        adm_ips_progressInit(&progress);
        adm_ips_progressGet(&progress);
        *eCmdidType = uimSoaCmdidTypeGet();
        *sessionId = uimSoaVehicleSessionId();
        *state = NULL;
        const DCHAR* tempstate = uimSoaVehicleState(&progress);
        if (NULL != tempstate) { *state = adm_strdup(tempstate); }
        *pstEcuInstallResultArrayInfo = uimSoaEcuInstallResultArrayInfoNew(&progress.ecus);
        *pstPolicyInstallResult = uimSoaInstallResultInfoNewVehicle(m_ipmContext.flash.result);
        adm_ips_progressDeInit(&progress);
        *errorCode = ADM_ECODE_SUCCESSED;
        return ADM_ECODE_SUCCESSED;
    }
    return ADM_ECODE_INVALID_PARAM;
}

/* ADM_IPM_UIM_T */
static void ipmUimInit(ADM_IPM_UIM_T *uim){
    uim->fnCmdDriverClick = NULL;
    uim->priv = NULL;
#if 0
    static ADM_UIM_SOA_INSTALL_INIT_INFO_T initInfo = {
	.schduleInstallCallback = NULL/*TODO trigger ipmUimSchduleInstallCallback*/,
	.cancelInstallCallback = NULL/*TODO trigger ipmUimCancelInstallCallback*/,
	.refuseInstallCallback = NULL/*TODO trigger ipmUimRefuseInstallCallback*/,
	.installTriggerCallback = ipmUimInstallTriggerCallback,
	.conditionsVerifiedCallback = NULL/*TODO hmi help update condition struct. ipmUimConditionsVerifiedCallback*/,
	.allEcuInstallProgressCallback = ipmUimAllEcuInstallProgressCallback,
	.allEcuInstallResultCallback = ipmUimAllEcuInstallResultCallback
    };
    adm_uim_installInit(&initInfo, uim);
#endif
    uim->condStop = (DCHAR)0;
    (void)pthread_mutex_init(&uim->condMutex, NULL);
    uim->eVerifiedType = ADM_UIM_SOA_CONDITIONS_VERIFIED_TYPE_INVALID;
    uim->cycleTime = 0U;
}
static void ipmUimDeInit(ADM_IPM_UIM_T *uim){
    uim->condStop = (DCHAR)1;
    (void)pthread_mutex_lock(&uim->condMutex);
    (void)pthread_mutex_unlock(&uim->condMutex);

#if 0
    adm_uim_installDeinit();
#endif
    (void)pthread_mutex_destroy(&uim->condMutex);
}

static void* ipmUimConditionsVerifiedHandle(void* argv){
    ADM_IPM_UIM_T *uim = (ADM_IPM_UIM_T *)argv;
    ADM_COND_POLICY_T cond;
    adm_condPolicyInit(&cond);
    D32S iType = (D32S)ADM_IPM_CONDITION_TYPE_USER;/* TODO value get */
    // (void)adm_ips_policyConditionGet(iType, &cond);
    (void)adm_ips_getConditionFromPolicy(iType, &cond);
    DBOOL pbIsConform = FALSE;
    D32S result = ADM_ECODE_FAILED;
    ADM_UIM_SOA_CONDITION_DATA_ARRAY_T* pStCondResults = NULL;
    if(uim->eVerifiedType == ADM_UIM_SOA_CONDITIONS_VERIFIED_TYPE_ONCE){

	    result = adm_cdm_verify_Compare(&cond, &pStCondResults, &pbIsConform);

        if(ADM_ECODE_SUCCESSED != result){
            DCHAR* eventDesc = ipmInstallConditionsVerifyResultEventDesc(pStCondResults, result);
		    if(eventDesc == NULL){
                log_w("getting Verify Condition descripe fail");;
            }
		    else{
			    DCHAR eventDescTmp[2048] = {0};
	            (void)adm_memset(eventDescTmp, 0x00, sizeof(eventDescTmp), sizeof(eventDescTmp));	
                (void)adm_snprintf(eventDescTmp, sizeof(eventDescTmp), "Installation pre condition check failed.(%s)", eventDesc);
                log_d("eventDescTmp = %s", eventDescTmp);
                ipmReportEventDependencyEventId("II_008",  eventDescTmp, ADM_ECODE_SUCCESSED);
			    adm_free(eventDesc);
		    }

			result = ADM_ECODE_INSTALL_CONDITION_FAILED;
            // 第一次失败或最大重试次数达到，其他忽略，避免频繁触发后台上报
            // 复用cycleTime（最大次数） timeout（第几次）
            if (uim->timeout == 1 || uim->cycleTime == uim->timeout) {
			    DCHAR* condsStr = adm_ipm_reportConditionsNotMet(result, &cond, pStCondResults);
		        DCHAR reason[128] = {0};
			    (void)adm_snprintf(reason, sizeof(reason) / sizeof(DCHAR), "%d", result);
			    ADM_IPM_REPORT_OTA_SERVER_T *reportOtaServer = NULL;
		        ipmReportOtaServerTaskStatus(reportOtaServer, ADM_IRM_REPORT_SERVER_STATUS_INSTALL_CONDITIONS_NOT_MET, reason, condsStr);
			    adm_free(condsStr);
            }
		
        }
	    else{
            ipmReportEventDependencyEventId("II_007",  NULL, ADM_ECODE_SUCCESSED);
	    }

	    ipmUimSoaInstallConditionsVerifyResultNotify(uim, pStCondResults, result);
	    if(pStCondResults != NULL){
	        (void)adm_cdm_verify_free(pStCondResults);
	    }
    }else if(uim->eVerifiedType == ADM_UIM_SOA_CONDITIONS_VERIFIED_TYPE_CYCLE){
	    D64U endTv = uim->timeout + ipmMonotonicTimeMSecGet();
	    for(D16U i=0; i<uim->cycleTime; ++i){
//#ifdef ADM_IPM_TEMP
//            result = ADM_ECODE_SUCCESSED;
//#else
	        result = adm_cdm_verify_Compare(&cond, &pStCondResults, &pbIsConform);
//#endif//ADM_IPM_TEMP

            if(ADM_ECODE_SUCCESSED != result){
				result = ADM_ECODE_INSTALL_CONDITION_FAILED;
                // 第一次失败或最大重试次数达到，其他忽略，避免频繁触发后台上报
				if (i == 0 || i == uim->cycleTime - 1) {
                    DCHAR* condsStr = adm_ipm_reportConditionsNotMet(result, &cond, pStCondResults);
                    DCHAR reason[128] = {0};
                    (void)adm_snprintf(reason, sizeof(reason) / sizeof(DCHAR), "%d", result);
                    ADM_IPM_REPORT_OTA_SERVER_T *reportOtaServer = NULL;
                    ipmReportOtaServerTaskStatus(reportOtaServer, ADM_IRM_REPORT_SERVER_STATUS_INSTALL_CONDITIONS_NOT_MET, reason, condsStr);
                    adm_free(condsStr);
                }
                DCHAR* eventDesc = ipmInstallConditionsVerifyResultEventDesc(pStCondResults, result);
                if (eventDesc == NULL) {
                    log_w("getting Verify Condition descripe fail");;
                }
                else {
                    DCHAR eventDescTmp[2048] = { 0 };
                    (void)adm_memset(eventDescTmp, 0x00, sizeof(eventDescTmp), sizeof(eventDescTmp));
                    (void)adm_snprintf(eventDescTmp, sizeof(eventDescTmp), "Installation pre condition check failed.(%s)", eventDesc);
                    log_d("eventDescTmp = %s", eventDescTmp);
                    ipmReportEventDependencyEventId("II_008", eventDescTmp, ADM_ECODE_SUCCESSED);
                    adm_free(eventDesc);
                }

            }
            else {
                ipmReportEventDependencyEventId("II_007", NULL, ADM_ECODE_SUCCESSED);
            }

            ADM_ECODE_APPLICATION_CONVERSION(result, ADM_ECODE_INSTALL_CONDITION_FAILED);
            ipmUimSoaInstallConditionsVerifyResultNotify(uim, pStCondResults, result);
            if (pStCondResults != NULL) {
                (void)adm_cdm_verify_free(pStCondResults);
                pStCondResults = NULL;
            }
            if (ADM_ECODE_SUCCESSED == result
                || (ipmMonotonicTimeMSecGet() > endTv)) {
                break;
            }
            else {
                (void)sleep(ADM_IPM_UIM_COND_NOTIFY_CYCLE_TIME_SECOND_WIDE);
            }
        }
    }
	else
	{
        ;/*null*/
	}
    adm_condPolicyDeInit(&cond);
    (void)pthread_mutex_unlock(&uim->condMutex);
    return NULL;
}
static DBOOL ipmUimConditionsVerified(ADM_IPM_UIM_T *uim, const ADM_UIM_SOA_CONDITIONS_VERIFIED_TYPE_E eVerifiedType, const D16U cycleTime, const D16U timeout){
    DBOOL dRet = FALSE;
    if(uim->condStop == (DCHAR)0){
	if(pthread_mutex_trylock(&uim->condMutex) == 0){
	    D32S ret = ipmStepInstallPrepare();
	    if(ADM_ECODE_SUCCESSED != ret){
		log_w("ipmStepInstallPrepare failed %d", ret);
	    }else{
	    uim->eVerifiedType = eVerifiedType;
	    uim->cycleTime = cycleTime;
	    uim->timeout = timeout;
	    pthread_t id;
	    if(pthread_create(&id, NULL, ipmUimConditionsVerifiedHandle, uim) == 0){
		(void)pthread_detach(id);
		dRet = TRUE;
	    }
	    if(!dRet){
		(void)pthread_mutex_unlock(&uim->condMutex);
	    }
	    }
	}
    }
    return dRet;
}
static void ipmUimSetCallback(ADM_IPM_UIM_T *uim, ADM_UIM_CMD_DRIVER_CLICK_F *fnCmdDriverClick, void *priv){
    uim->fnCmdDriverClick = fnCmdDriverClick;
    uim->priv = priv;
}
static void ipmUimSoaInstallConditionsVerifyResultNotify(ADM_IPM_UIM_T *uim, const ADM_UIM_SOA_CONDITION_DATA_ARRAY_T* condsArrarList, const D32S verifyResult){
    ADM_IPS_PROGRESS_T progress;
    adm_ips_progressInit(&progress);
    adm_ips_progressGet(&progress);
    DCHAR* sessionId = uimSoaVehicleSessionId();
	D32S respCode = ADM_ECODE_SUCCESSED;
//    adm_uim_installConditionsVerifyResultNotify(uimSoaCmdidTypeGet(), sessionId, uimSoaVehicleState(&progress), condsArrarList, verifyResult);
    if(adm_vsm_getHmiOnline())
    {
	    adm_uim_hmi_conds_installResult_RR(uimSoaCmdidTypeGet(), sessionId, uimSoaVehicleState(&progress), condsArrarList, verifyResult, &respCode);
        if(ADM_ECODE_SUCCESSED != respCode)
	    {  
            ipmReportEventDependencyEventId("II_017", NULL, respCode);
    	}
	}
    adm_free(sessionId);
    adm_ips_progressDeInit(&progress);
}
static void ipmUimSoaPolicyInstallProgressNotify(ADM_IPM_UIM_T *uim, const ADM_IPS_PROGRESS_T *progress){
    ADM_UIM_SOA_INSTALL_PROGRESS_INFO_T pstInstallProgress;
    uimSoaInstallProgressInfoInitVehicle(&pstInstallProgress, progress);
    DCHAR* sessionId = uimSoaVehicleSessionId();
    adm_uim_ensureCheckSubedEventSuccess(ADM_UIM_INSTALL_SERVICE_ID, ADM_UIM_POLICY_INSTALL_PROGRESS_NOTIFY_EVENT_ID);
    (void)adm_uim_policyInstallProgressNotify(uimSoaCmdidTypeGet(), sessionId, uimSoaVehicleState(progress), &pstInstallProgress);
    adm_free(sessionId);
    uimSoaInstallProgressInfoDeInit(&pstInstallProgress);
}
static D32S ipmUimSoaPolicyInstallResultNotify(ADM_IPM_UIM_T *uim, const ADM_IPS_PROGRESS_T* progress, D32S result, DCHAR *pcConditionResult)
{
    ADM_UIM_SOA_INSTALL_RESULT_INFO_T pstEcuInstallResult;
    uimSoaInstallResultInfoInitVehicle(&pstEcuInstallResult, result);
    DCHAR* sessionId = uimSoaVehicleSessionId();
	D32S respCode = ADM_ECODE_SUCCESSED;
    D32S ret = ADM_ECODE_SUCCESSED;
    
    if (ADM_ECODE_INSTALL_CONDITION_FAILED == result &&
        NULL != pcConditionResult)
    {
        pstEcuInstallResult.expectVal = adm_strdup(pcConditionResult);
    }
    
    if(adm_vsm_getHmiOnline())
    {
        adm_uim_hmi_policy_installResult_RR(uimSoaCmdidTypeGet(), sessionId, uimSoaVehicleState(progress), &pstEcuInstallResult, &respCode);
        if(ADM_ECODE_SUCCESSED != respCode)
	    {
	        ret = ADM_ECODE_FAILED;
            ipmReportEventDependencyEventId("II_018", NULL, respCode);
    	}
	}
    adm_free(sessionId);
    uimSoaInstallResultInfoDeInit(&pstEcuInstallResult);
    return ret;
}
static void ipmUimSoaEcuInstallProgressNotify(ADM_IPM_UIM_T *uim, const ADM_IPS_ECU_PROGRESS_T *progress){
    ADM_UIM_SOA_ECU_IDEN_INFO_T pstEcuIdenInfo;
    uimSoaEcuIdenInfoInit(&pstEcuIdenInfo, progress);
    ADM_UIM_SOA_INSTALL_PROGRESS_INFO_T pstEcuInstallProgressInfo;
    uimSoaInstallProgressInfoInitEcu(&pstEcuInstallProgressInfo, progress);
    DCHAR* sessionId = uimSoaVehicleSessionId();
    (void)adm_uim_ecuInstallProgressNotify(uimSoaCmdidTypeGet(), sessionId, uimSoaEcuState(progress), &pstEcuIdenInfo, &pstEcuInstallProgressInfo);
    adm_free(sessionId);
    uimSoaInstallProgressInfoDeInit(&pstEcuInstallProgressInfo);
    uimSoaEcuIdenInfoDeInit(&pstEcuIdenInfo);
}
static void ipmUimSoaEcuInstallResultNotify(ADM_IPM_UIM_T *uim, const ADM_IPS_ECU_PROGRESS_T *progress){
    ADM_UIM_SOA_ECU_IDEN_INFO_T pstEcuIdenInfo;
    uimSoaEcuIdenInfoInit(&pstEcuIdenInfo, progress);
    ADM_UIM_SOA_INSTALL_RESULT_INFO_T pstEcuInstallResultInfo;
    uimSoaInstallResultInfoInitEcu(&pstEcuInstallResultInfo, progress);
    DCHAR* sessionId = uimSoaVehicleSessionId();
	D32S respCode = ADM_ECODE_SUCCESSED;
//    adm_uim_ecuInstallResultNotify(uimSoaCmdidTypeGet(), sessionId, uimSoaEcuState(progress), &pstEcuIdenInfo, &pstEcuInstallResultInfo);
    if(adm_vsm_getHmiOnline())
    {
		adm_uim_hmi_ecu_installResult_RR(uimSoaCmdidTypeGet(), sessionId, uimSoaEcuState(progress), &pstEcuIdenInfo, &pstEcuInstallResultInfo, &respCode);
	    if(ADM_ECODE_SUCCESSED != respCode)
		{        
            ipmReportEventDependencyEventId("II_019", NULL, respCode);
    	}
	}
	
    adm_free(sessionId);
    uimSoaInstallResultInfoDeInit(&pstEcuInstallResultInfo);
    uimSoaEcuIdenInfoDeInit(&pstEcuIdenInfo);
}
//static D32S ipmUimSoaAllEcuInstallProgressNotify(ADM_IPM_UIM_T *uim, const ADM_APPLICATION_COMMON_CMDID_TYPE_E eCmdidType, const DCHAR* sessionId, const DCHAR* state, const ADM_UIM_SOA_ECU_INSTALL_PROGRESS_ARRAY_INFO_T* pstEcuInstallProgressArrayInfo, const ADM_UIM_SOA_INSTALL_PROGRESS_INFO_T* pstPolicyInstallProgress){}
//static D32S ipmUimSoaAllEcuInstallResultNotify(ADM_IPM_UIM_T *uim, const ADM_APPLICATION_COMMON_CMDID_TYPE_E eCmdidType, const DCHAR* sessionId, const DCHAR* state, const ADM_UIM_SOA_ECU_INSTALL_RESULT_ARRAY_INFO_T* pstEcuInstallResultArrayInfo, const ADM_UIM_SOA_INSTALL_RESULT_INFO_T* pstPolicyInstallResult){}
static D32S ipmUimAllEcuInstallResultNotify(ADM_IPM_UIM_T *uim, const ADM_IPS_PROGRESS_T *progress, D32S result) {
    D32S respCode = ADM_ECODE_SUCCESSED;
    DCHAR* sessionId = uimSoaVehicleSessionId();
    ADM_UIM_SOA_ECU_INSTALL_RESULT_ARRAY_INFO_T ecuInstallResultArrayInfo;
    uimSoaEcuInstallResultArrayInfoInit(&ecuInstallResultArrayInfo, &progress->ecus);
    ADM_UIM_SOA_INSTALL_RESULT_INFO_T installResult;
    uimSoaInstallResultInfoInitVehicle(&installResult, result);
    D32S ret = ADM_ECODE_SUCCESSED;
//	adm_uim_allEcuInstallResultNotify(uimSoaCmdidTypeGet(), sessionId, uimSoaVehicleState(progress), &ecuInstallResultArrayInfo, &installResult);
	adm_uim_hmi_allecu_installResult_RR(uimSoaCmdidTypeGet(), sessionId, uimSoaVehicleState(progress), &ecuInstallResultArrayInfo, &installResult, &respCode);
    if(ADM_ECODE_SUCCESSED != respCode)
	{
	    ret = ADM_ECODE_FAILED;
        if(adm_vsm_getHmiOnline())
    	{
            ipmReportEventDependencyEventId("II_020", NULL, respCode);
    	}
	}
	
    uimSoaInstallResultInfoDeInit(&installResult);
    uimSoaEcuInstallResultArrayInfoDeInit(&ecuInstallResultArrayInfo);
 
    adm_free(sessionId);
   
    return ret;
}


/* ADM_UIM_SOA_ECU_INSTALL_PROGRESS_ARRAY_INFO_T */
static void uimSoaEcuInstallProgressArrayInfoInit(ADM_UIM_SOA_ECU_INSTALL_PROGRESS_ARRAY_INFO_T *uimEcusProgress, const ADM_IPS_ECU_PROGRESS_ARRAY_T *ipsEcusProgress){
    uimEcusProgress->ecuInstallProgressArray = NULL;
    uimEcusProgress->ecuInstallProgressArrayLength = 0;
    if((NULL != ipsEcusProgress->ecus) && (0 !=ipsEcusProgress->ecusLen)){
	uimEcusProgress->ecuInstallProgressArray = adm_malloc((D32U)ipsEcusProgress->ecusLen *sizeof(*(uimEcusProgress->ecuInstallProgressArray)));
	if(NULL != uimEcusProgress->ecuInstallProgressArray){
	    uimEcusProgress->ecuInstallProgressArrayLength = (D16U)ipsEcusProgress->ecusLen;
	    for(D16U i=0; i<uimEcusProgress->ecuInstallProgressArrayLength; ++i){
		uimEcusProgress->ecuInstallProgressArray[i] = adm_malloc(sizeof(*(uimEcusProgress->ecuInstallProgressArray[i])));
		if(NULL != uimEcusProgress->ecuInstallProgressArray[i]){
		    uimSoaEcuInstallProgressInfoInit(uimEcusProgress->ecuInstallProgressArray[i], ipsEcusProgress->ecus + i);
		}
	    }
	}
    }
}
/*
static void uimSoaEcuInstallProgressArrayInfoDeInit(ADM_UIM_SOA_ECU_INSTALL_PROGRESS_ARRAY_INFO_T *uimEcusProgress){
    if(NULL != uimEcusProgress->ecuInstallProgressArray){
	for(D16U i=0; i<uimEcusProgress->ecuInstallProgressArrayLength; ++i){
	    if(NULL != uimEcusProgress->ecuInstallProgressArray[i]){
		uimSoaEcuInstallProgressInfoDeInit(uimEcusProgress->ecuInstallProgressArray[i]);
		adm_free(uimEcusProgress->ecuInstallProgressArray[i]);
	    }
	}
	adm_free(uimEcusProgress->ecuInstallProgressArray);
    }
}
*/
static ADM_UIM_SOA_ECU_INSTALL_PROGRESS_ARRAY_INFO_T* uimSoaEcuInstallProgressArrayInfoNew(const ADM_IPS_ECU_PROGRESS_ARRAY_T *ipsEcusProgress){
    ADM_UIM_SOA_ECU_INSTALL_PROGRESS_ARRAY_INFO_T *uimEcusProgress = adm_malloc(sizeof(*uimEcusProgress));
    if(NULL != uimEcusProgress){
        uimSoaEcuInstallProgressArrayInfoInit(uimEcusProgress, ipsEcusProgress);
    }
    return uimEcusProgress;
}
/*
static void uimSoaEcuInstallProgressArrayInfoDel(ADM_UIM_SOA_ECU_INSTALL_PROGRESS_ARRAY_INFO_T *uimEcusProgress){
    uimSoaEcuInstallProgressArrayInfoDeInit(uimEcusProgress);
    adm_free(uimEcusProgress);
}
*/

/* ADM_UIM_SOA_ECU_INSTALL_PROGRESS_INFO_T */
static void uimSoaEcuInstallProgressInfoInit(ADM_UIM_SOA_ECU_INSTALL_PROGRESS_INFO_T *uimEcuProgress, const ADM_IPS_ECU_PROGRESS_T *ipsEcuProgress){
    uimEcuProgress->pstEcuIdenInfo = adm_malloc(sizeof(*(uimEcuProgress->pstEcuIdenInfo)));
    if(NULL != uimEcuProgress->pstEcuIdenInfo){
        uimSoaEcuIdenInfoInit(uimEcuProgress->pstEcuIdenInfo, ipsEcuProgress);
    }
    uimEcuProgress->state = NULL;
    const DCHAR* state = uimSoaEcuState(ipsEcuProgress);
    if(NULL != state){uimEcuProgress->state = adm_strdup(state);}
    uimEcuProgress->pstEcuInstallProgress = adm_malloc(sizeof(*(uimEcuProgress->pstEcuInstallProgress)));
    if(NULL != uimEcuProgress->pstEcuInstallProgress){
        uimSoaInstallProgressInfoInitEcu(uimEcuProgress->pstEcuInstallProgress, ipsEcuProgress);
    }
}
/*
static void uimSoaEcuInstallProgressInfoDeInit(ADM_UIM_SOA_ECU_INSTALL_PROGRESS_INFO_T *uimEcuProgress){
    if(NULL != uimEcuProgress->pstEcuIdenInfo){
	uimSoaEcuIdenInfoDeInit(uimEcuProgress->pstEcuIdenInfo);
	adm_free(uimEcuProgress->pstEcuIdenInfo);
    }
    if(NULL != uimEcuProgress->state){adm_free(uimEcuProgress->state);}
    if(NULL != uimEcuProgress->pstEcuInstallProgress){
	uimSoaInstallProgressInfoDeInit(uimEcuProgress->pstEcuInstallProgress);
	adm_free(uimEcuProgress->pstEcuInstallProgress);
    }
}
*/

/* uim soa ecu state */
static const DCHAR* uimSoaEcuState(const ADM_IPS_ECU_PROGRESS_T *progress){
    const DCHAR *state = NULL;
    switch(progress->state){
	case ADM_IPS_ECU_PHASE_PRE_INSTALL:
	    state = ADM_STR_FROM_PARAM(ADM_IPS_ECU_PHASE_PRE_INSTALL);
	    break;
	case ADM_IPS_ECU_PHASE_INSTALL:
	    state = ADM_STR_FROM_PARAM(ADM_IPS_ECU_PHASE_INSTALL);
	    break;
	case ADM_IPS_ECU_PHASE_POST_INSTALL:
	    state = ADM_STR_FROM_PARAM(ADM_IPS_ECU_PHASE_POST_INSTALL);
	    break;
	case ADM_IPS_ECU_PHASE_INSTALL_ROLLBACK:
	    state = ADM_STR_FROM_PARAM(ADM_IPS_ECU_PHASE_INSTALL_ROLLBACK);
	    break;
	case ADM_IPS_ECU_PHASE_POST_INSTALL_ROLLBACK:
	    state = ADM_STR_FROM_PARAM(ADM_IPS_ECU_PHASE_POST_INSTALL_ROLLBACK);
	    break;
	case ADM_IPS_ECU_PHASE_POST_INSTALL_JUMP:
	    state = ADM_STR_FROM_PARAM(ADM_IPS_ECU_PHASE_POST_INSTALL_JUMP);
	    break;
	default:break;
    }
    return state;
}

/* ADM_UIM_SOA_ECU_IDEN_INFO_T */
static void uimSoaEcuIdenInfoInit(ADM_UIM_SOA_ECU_IDEN_INFO_T *pstEcuIdenInfo, const ADM_IPS_ECU_PROGRESS_T *ipsEcuProgress){
    pstEcuIdenInfo->partNum = NULL;
    pstEcuIdenInfo->requestId = ipmHtoi(ipsEcuProgress->ecuReqId);
    pstEcuIdenInfo->responseId = 0;
    pstEcuIdenInfo->swDid = ipmHtoi(ipsEcuProgress->ecuSwDid);
    pstEcuIdenInfo->name = NULL;
    pstEcuIdenInfo->softVer = NULL;
}
static void uimSoaEcuIdenInfoDeInit(ADM_UIM_SOA_ECU_IDEN_INFO_T *pstEcuIdenInfo){
    adm_free(pstEcuIdenInfo->partNum);
    adm_free(pstEcuIdenInfo->name);
    adm_free(pstEcuIdenInfo->softVer);
}

/* ADM_UIM_SOA_INSTALL_PROGRESS_INFO_T */
static void uimSoaInstallProgressInfoInitEcu(ADM_UIM_SOA_INSTALL_PROGRESS_INFO_T *pstEcuInstallProgress, const ADM_IPS_ECU_PROGRESS_T *ipsEcuProgress){
    pstEcuInstallProgress->progress = (D16U)ipsEcuProgress->progress[ipsEcuProgressIndexGet(ipsEcuProgress)].progress;
}
static void uimSoaInstallProgressInfoInitVehicle(ADM_UIM_SOA_INSTALL_PROGRESS_INFO_T *pstInstallProgress, const ADM_IPS_PROGRESS_T *ipsProgress){
    pstInstallProgress->progress = (D16U)ipsProgress->progress;
}
static void uimSoaInstallProgressInfoDeInit(ADM_UIM_SOA_INSTALL_PROGRESS_INFO_T *pstProgress){
}
static ADM_UIM_SOA_INSTALL_PROGRESS_INFO_T* uimSoaInstallProgressInfoNewVehicle(const ADM_IPS_PROGRESS_T *ipsProgress){
    ADM_UIM_SOA_INSTALL_PROGRESS_INFO_T *pstInstallProgress = adm_malloc(sizeof(*pstInstallProgress));
    if(NULL != pstInstallProgress){
	uimSoaInstallProgressInfoInitVehicle(pstInstallProgress, ipsProgress);
    }
    return pstInstallProgress;
}
/*
static void uimSoaInstallProgressInfoDel(ADM_UIM_SOA_INSTALL_PROGRESS_INFO_T *pstProgress){
    uimSoaInstallProgressInfoDeInit(pstProgress);
    adm_free(pstProgress);
}
*/

/* ADM_APPLICATION_COMMON_CMDID_TYPE_E */
ADM_APPLICATION_COMMON_CMDID_TYPE_E uimSoaCmdidTypeGet(void){
    ADM_APPLICATION_COMMON_CMDID_TYPE_E eCmdidType = ADM_APPLICATION_COMMON_CMDID_TYPE_INVALID;
    D32S cmdId = adm_ips_cmdIdGet();
    if(cmdId < (D32S)ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_USER
	    || cmdId > (D32S)ADM_APPLICATION_COMMON_CMDID_TYPE_INVALID){}else{
	eCmdidType = (ADM_APPLICATION_COMMON_CMDID_TYPE_E)cmdId;
    }
    return eCmdidType;
}

/* uim soa vehicle state */
static const DCHAR* uimSoaVehicleState(const ADM_IPS_PROGRESS_T *progress){
    const DCHAR *state = NULL;
    switch(progress->state){
	case ADM_IPS_PHASE_PRE_INSTALL:
	    state = ADM_STR_FROM_PARAM(ADM_IPS_PHASE_PRE_INSTALL);
	    break;
	case ADM_IPS_PHASE_INSTALL:
	    state = ADM_STR_FROM_PARAM(ADM_IPS_PHASE_INSTALL);
	    break;
	case ADM_IPS_PHASE_POST_INSTALL:
	    state = ADM_STR_FROM_PARAM(ADM_IPS_PHASE_POST_INSTALL);
	    break;
	default:
		state = ADM_STR_FROM_PARAM(ADM_IPS_PHASE_IDLE);
		break;
    }
    return state;
}

/* uim soa vehicle sessionId */
DCHAR* uimSoaVehicleSessionId(void){
    DCHAR *sessionId = NULL;
    ADM_IPS_POLICY_TASK_T task;
    adm_ips_policyTaskInit(&task);
//    adm_ips_policyTaskGet(&task);
	if(ADM_IS_SUCCESS(adm_ips_policyTaskGet(&task)))
	{
	    if(NULL != task.sessionId){sessionId = adm_strdup(task.sessionId);}
	}
    adm_ips_policyTaskDeInit(&task);
    return sessionId;
}

/* ADM_UIM_SOA_ECU_INSTALL_RESULT_ARRAY_INFO_T */
static void uimSoaEcuInstallResultArrayInfoInit(ADM_UIM_SOA_ECU_INSTALL_RESULT_ARRAY_INFO_T *pstEcuInstallResultArrayInfo, const ADM_IPS_ECU_PROGRESS_ARRAY_T *ipsEcusProgress){
    pstEcuInstallResultArrayInfo->ecuInstallResultArray = NULL;
    pstEcuInstallResultArrayInfo->ecuInstallResultArrayLength = 0;
    if((NULL != ipsEcusProgress->ecus) && (0 != ipsEcusProgress->ecusLen)){
	pstEcuInstallResultArrayInfo->ecuInstallResultArray = adm_malloc((D32U)ipsEcusProgress->ecusLen *sizeof(*(pstEcuInstallResultArrayInfo->ecuInstallResultArray)));
	if(NULL != pstEcuInstallResultArrayInfo->ecuInstallResultArray){
	    pstEcuInstallResultArrayInfo->ecuInstallResultArrayLength = (D16U)ipsEcusProgress->ecusLen;
	    for(D16U i=0; i<pstEcuInstallResultArrayInfo->ecuInstallResultArrayLength; ++i){
		pstEcuInstallResultArrayInfo->ecuInstallResultArray[i] = adm_malloc(sizeof(*(pstEcuInstallResultArrayInfo->ecuInstallResultArray[i])));
		if(NULL != pstEcuInstallResultArrayInfo->ecuInstallResultArray[i]){
		    uimSoaEcuInstallResultInfoInit(pstEcuInstallResultArrayInfo->ecuInstallResultArray[i], ipsEcusProgress->ecus + i);
		}
	    }
	}
    }
}
static void uimSoaEcuInstallResultArrayInfoDeInit(ADM_UIM_SOA_ECU_INSTALL_RESULT_ARRAY_INFO_T *pstEcuInstallResultArrayInfo){
    if(NULL != pstEcuInstallResultArrayInfo->ecuInstallResultArray){
	for(D16U i=0; i<pstEcuInstallResultArrayInfo->ecuInstallResultArrayLength; ++i){
	    if(NULL != pstEcuInstallResultArrayInfo->ecuInstallResultArray[i]){
		uimSoaEcuInstallResultInfoDeInit(pstEcuInstallResultArrayInfo->ecuInstallResultArray[i]);
		adm_free(pstEcuInstallResultArrayInfo->ecuInstallResultArray[i]);
	    }
	}
	adm_free(pstEcuInstallResultArrayInfo->ecuInstallResultArray);
    }
}
static ADM_UIM_SOA_ECU_INSTALL_RESULT_ARRAY_INFO_T* uimSoaEcuInstallResultArrayInfoNew(const ADM_IPS_ECU_PROGRESS_ARRAY_T *ipsEcusProgress){
    ADM_UIM_SOA_ECU_INSTALL_RESULT_ARRAY_INFO_T *pstEcuInstallResultArrayInfo = adm_malloc(sizeof(*pstEcuInstallResultArrayInfo));
    if(NULL != pstEcuInstallResultArrayInfo){
	uimSoaEcuInstallResultArrayInfoInit(pstEcuInstallResultArrayInfo, ipsEcusProgress);
    }
    return pstEcuInstallResultArrayInfo;
}
/*
static void uimSoaEcuInstallResultArrayInfoDel(ADM_UIM_SOA_ECU_INSTALL_RESULT_ARRAY_INFO_T *pstEcuInstallResultArrayInfo){
    uimSoaEcuInstallResultArrayInfoDeInit(pstEcuInstallResultArrayInfo);
    adm_free(pstEcuInstallResultArrayInfo);
}
*/

/* ADM_UIM_SOA_ECU_INSTALL_RESULT_INFO_T */
static void uimSoaEcuInstallResultInfoInit(ADM_UIM_SOA_ECU_INSTALL_RESULT_INFO_T *ecuInstallResult, const ADM_IPS_ECU_PROGRESS_T *ipsEcuProgress){
    ecuInstallResult->pstEcuIdenInfo = adm_malloc(sizeof(*(ecuInstallResult->pstEcuIdenInfo)));
    if(NULL != ecuInstallResult->pstEcuIdenInfo){
	uimSoaEcuIdenInfoInit(ecuInstallResult->pstEcuIdenInfo, ipsEcuProgress);
    }
    ecuInstallResult->pstEcuInstallResult = adm_malloc(sizeof(*(ecuInstallResult->pstEcuInstallResult)));
    if(NULL != ecuInstallResult->pstEcuInstallResult){
	uimSoaInstallResultInfoInitEcu(ecuInstallResult->pstEcuInstallResult, ipsEcuProgress);
    }
}
static void uimSoaEcuInstallResultInfoDeInit(ADM_UIM_SOA_ECU_INSTALL_RESULT_INFO_T *ecuInstallResult){
    if(NULL != ecuInstallResult->pstEcuIdenInfo){
	uimSoaEcuIdenInfoDeInit(ecuInstallResult->pstEcuIdenInfo);
	adm_free(ecuInstallResult->pstEcuIdenInfo);
    }
    if(NULL != ecuInstallResult->pstEcuInstallResult){
	uimSoaInstallResultInfoDeInit(ecuInstallResult->pstEcuInstallResult);
	adm_free(ecuInstallResult->pstEcuInstallResult);
    }
}

/* ADM_UIM_SOA_INSTALL_RESULT_INFO_T */
static void uimSoaInstallResultInfoInitEcu(ADM_UIM_SOA_INSTALL_RESULT_INFO_T *pstEcuInstallResult, const ADM_IPS_ECU_PROGRESS_T *ipsEcuProgress){
    pstEcuInstallResult->result = ipsEcuProgress->progress[ipsEcuProgressIndexGet(ipsEcuProgress)].result;
    pstEcuInstallResult->expectVal = NULL;
    pstEcuInstallResult->realVal = NULL;
}
void uimSoaInstallResultInfoInitVehicle(ADM_UIM_SOA_INSTALL_RESULT_INFO_T *pstInstallResult, D32S result){
    pstInstallResult->result = result;
    pstInstallResult->expectVal = NULL;
    pstInstallResult->realVal = NULL;
}

static void uimSoaInstallResultInfoDeInit(ADM_UIM_SOA_INSTALL_RESULT_INFO_T *pstResult){
    adm_free(pstResult->expectVal);
    adm_free(pstResult->realVal);

    return;
}
static ADM_UIM_SOA_INSTALL_RESULT_INFO_T* uimSoaInstallResultInfoNewVehicle(D32S result){
    ADM_UIM_SOA_INSTALL_RESULT_INFO_T *pstInstallResult = adm_malloc(sizeof(*pstInstallResult));
    if(NULL != pstInstallResult){
	    uimSoaInstallResultInfoInitVehicle(pstInstallResult, result);
    }
    return pstInstallResult;
}
/*
static void uimSoaInstallResultInfoDel(ADM_UIM_SOA_INSTALL_RESULT_INFO_T *pstResult){
    uimSoaInstallResultInfoDeInit(pstResult);
    adm_free(pstResult);
}
*/

/* ADM_IPS_ECU_PROGRESS_T ecu index */
static D32S ipsEcuProgressIndexGet(const ADM_IPS_ECU_PROGRESS_T *ecuProgress){
    return (ecuProgress->state == ADM_IPS_ECU_PHASE_INSTALL_ROLLBACK 
	    || ecuProgress->state == ADM_IPS_ECU_PHASE_POST_INSTALL_ROLLBACK
	    ) ? 1 : 0;
}


/* ADM_IPM_DIM_T vdi handle */
static D32S ipmDimReflashProgressNotify(const ADM_DIM_SOA_ECU_IDEN_INFO_T* pstIdenInfo, const D16U* reflashProgress, void* priv){
    log_d("%s:%p, %s:%p, %s:%p", 
	   ADM_STR_FROM_PARAM(pstIdenInfo), pstIdenInfo,
	   ADM_STR_FROM_PARAM(reflashProgress), reflashProgress,
	   ADM_STR_FROM_PARAM(priv), priv);
    if((NULL != pstIdenInfo) && (NULL != reflashProgress) && (NULL != priv))
	{
	    log_d("%s:0X%X, %s:0X%X, %s:%u", 
		    ADM_STR_FROM_PARAM(pstIdenInfo->requestId), pstIdenInfo->requestId,
		    ADM_STR_FROM_PARAM(pstIdenInfo->swDid), pstIdenInfo->swDid,
		    ADM_STR_FROM_PARAM(reflashProgress), *reflashProgress);
// 	    ADM_IPM_DIM_T *dim = (ADM_IPM_DIM_T *)priv;
// 	    DCHAR ecuReqId[ADM_IPM_DIM_ECU_MEMBER_SIZE];
// 	    DCHAR ecuSwDid[ADM_IPM_DIM_ECU_MEMBER_SIZE];
// 	    (void)adm_snprintf(ecuReqId, sizeof(ecuReqId)/sizeof(ecuReqId[0]), "0X%X", pstIdenInfo->requestId);
// 	    (void)adm_snprintf(ecuSwDid, sizeof(ecuSwDid)/sizeof(ecuSwDid[0]), "0X%X", pstIdenInfo->swDid);
// 	    ipmDoFlashEcuSetProgress(dim->doFlash, ecuReqId, ecuSwDid, (D32S)*reflashProgress);


        IPM_ECU_FLASH_PROGRESS_T flashProgress;
        flashProgress.dim = (ADM_IPM_DIM_T*)priv;
        (void)adm_snprintf(flashProgress.ecuReqId, sizeof(flashProgress.ecuReqId), "0X%X", pstIdenInfo->requestId);
        (void)adm_snprintf(flashProgress.ecuSwDid, sizeof(flashProgress.ecuSwDid), "0X%X", pstIdenInfo->swDid);
        flashProgress.progress = (D32S)*reflashProgress;

        (void)ipmMsgCenterSendMsg(IPM_MSG_ECU_FLASH_PROGRESS_NOTIFY, &flashProgress, (D32U)sizeof(flashProgress));
    }

    return ADM_ECODE_SUCCESSED;
}
static D32S ipmDimReflashResultSend(const ADM_DIM_SOA_ECU_IDEN_INFO_T* pstIdenInfo, const ADM_DIM_SOA_VDI_REFLASH_RESULT_INFO_T* pstReflashResultSendInfo, void* priv){
    log_d("%s:%p, %s:%p, %s:%p", 
	   ADM_STR_FROM_PARAM(pstIdenInfo), pstIdenInfo,
	   ADM_STR_FROM_PARAM(pstReflashResultSendInfo), pstReflashResultSendInfo,
	   ADM_STR_FROM_PARAM(priv), priv);
    if((NULL!= pstIdenInfo) && (NULL!= pstReflashResultSendInfo) && (NULL!= priv))
	{
        log_d("%s:0X%X, %s:0X%X, %s:%d, realVal:%s",
            ADM_STR_FROM_PARAM(pstIdenInfo->requestId), pstIdenInfo->requestId,
            ADM_STR_FROM_PARAM(pstIdenInfo->swDid), pstIdenInfo->swDid,
            ADM_STR_FROM_PARAM(pstReflashResultSendInfo->result), pstReflashResultSendInfo->result,
            ADM_STR_SAFE_OUTPUT(pstReflashResultSendInfo->realVal));

        IPM_ECU_FLASH_RESULT_T flashResult = {0};
        flashResult.dim = (ADM_IPM_DIM_T*)priv;
        (void)adm_snprintf(flashResult.ecuReqId, sizeof(flashResult.ecuReqId), "0X%X", pstIdenInfo->requestId);
        (void)adm_snprintf(flashResult.ecuSwDid, sizeof(flashResult.ecuSwDid), "0X%X", pstIdenInfo->swDid);
        flashResult.result = pstReflashResultSendInfo->result;
        adm_snprintf(flashResult.resultDescrip,RESULT_DESCRIP_LEN,"%s",ADM_STR_SAFE_OUTPUT(pstReflashResultSendInfo->realVal));

        DCHAR aDescription[128] = {0};
        adm_snprintf(aDescription, sizeof(aDescription), "ecuReqId[%s]ecuSwDid[%s],result[%d] ", flashResult.ecuReqId, flashResult.ecuSwDid,flashResult.result);
        if (ADM_IS_ERR(flashResult.result))
        {
            adm_pub_setKeylogString(ADM_PUB_KEYLOG_ID_034, 0, aDescription);
        }
        else
        {
            adm_pub_setKeylogString(ADM_PUB_KEYLOG_ID_033, 0, aDescription);
        }

        (void)ipmMsgCenterSendMsg(IPM_MSG_ECU_FLASH_RESULT_NOTIFY, &flashResult, (D32U)sizeof(flashResult));
    }
	
    return ADM_ECODE_SUCCESSED;
}

/* ADM_IPM_DIM_T ucm handle */
static D32S ipmDimSoaUcmUpgradeProgressNotify(const ADM_DIM_SOA_ECU_IDEN_INFO_T* pstEcuIdenInfo, const D16U progress, void* priv){
    log_d("%s:%p, %s:%u, %s:%p",
	   ADM_STR_FROM_PARAM(pstEcuIdenInfo), pstEcuIdenInfo,
	   ADM_STR_FROM_PARAM(progress), progress,
	   ADM_STR_FROM_PARAM(priv), priv);
    if((NULL != pstEcuIdenInfo) && (NULL != priv))
	{
	    log_d("%s:0X%X, %s:0X%X, %s:%u",
	        ADM_STR_FROM_PARAM(pstEcuIdenInfo->requestId), pstEcuIdenInfo->requestId,
	        ADM_STR_FROM_PARAM(pstEcuIdenInfo->swDid), pstEcuIdenInfo->swDid,
	        ADM_STR_FROM_PARAM(progress), progress);
	    //ADM_IPM_DIM_T *dim = (ADM_IPM_DIM_T *)priv;
	    //DCHAR ecuReqId[ADM_IPM_DIM_ECU_MEMBER_SIZE];
	    //DCHAR ecuSwDid[ADM_IPM_DIM_ECU_MEMBER_SIZE];
	    //(void)adm_snprintf(ecuReqId, sizeof(ecuReqId)/sizeof(ecuReqId[0]), "0X%X", pstEcuIdenInfo->requestId);
	    //(void)adm_snprintf(ecuSwDid, sizeof(ecuSwDid)/sizeof(ecuSwDid[0]), "0X%X", pstEcuIdenInfo->swDid);
	    //ipmDoFlashEcuSetProgress(dim->doFlash, ecuReqId, ecuSwDid, (D32S)progress);

        IPM_ECU_FLASH_PROGRESS_T flashProgress;
        flashProgress.dim = (ADM_IPM_DIM_T*)priv;
        (void)adm_snprintf(flashProgress.ecuReqId, sizeof(flashProgress.ecuReqId), "0X%X", pstEcuIdenInfo->requestId);
        (void)adm_snprintf(flashProgress.ecuSwDid, sizeof(flashProgress.ecuSwDid), "0X%X", pstEcuIdenInfo->swDid);
        flashProgress.progress = (D32S)progress;

        (void)ipmMsgCenterSendMsg(IPM_MSG_ECU_FLASH_PROGRESS_NOTIFY, &flashProgress, (D32U)sizeof(flashProgress));
    }
	
    return ADM_ECODE_SUCCESSED;
}
static D32S ipmDimSoaUcmUpgradeResultNotify(const ADM_DIM_SOA_ECU_IDEN_INFO_T* pstIdenInfo, const ADM_DIM_SOA_UCL_UPGRADE_RESULT_INFO_T* pstUpgradeResultInfo, void* priv){
    log_d("%s:%p, %s:%p, %s:%p", 
	   ADM_STR_FROM_PARAM(pstIdenInfo), pstIdenInfo,
	   ADM_STR_FROM_PARAM(pstUpgradeResultInfo), pstUpgradeResultInfo,
	   ADM_STR_FROM_PARAM(priv), priv);
    if((NULL != pstIdenInfo) && (NULL != pstUpgradeResultInfo) && (NULL != priv))
	{
        log_d("%s:0X%X, %s:0X%X, %s:%d",
            ADM_STR_FROM_PARAM(pstIdenInfo->requestId), pstIdenInfo->requestId,
            ADM_STR_FROM_PARAM(pstIdenInfo->swDid), pstIdenInfo->swDid,
            ADM_STR_FROM_PARAM(pstUpgradeResultInfo->result), pstUpgradeResultInfo->result);
        //ADM_IPM_DIM_T* dim = (ADM_IPM_DIM_T*)priv;
        //DCHAR ecuReqId[ADM_IPM_DIM_ECU_MEMBER_SIZE];
        //DCHAR ecuSwDid[ADM_IPM_DIM_ECU_MEMBER_SIZE];
        //(void)adm_snprintf(ecuReqId, sizeof(ecuReqId) / sizeof(ecuReqId[0]), "0X%X", pstIdenInfo->requestId);
        //(void)adm_snprintf(ecuSwDid, sizeof(ecuSwDid) / sizeof(ecuSwDid[0]), "0X%X", pstIdenInfo->swDid);
        //ipmDoFlashEcuSetResult(dim->doFlash, ecuReqId, ecuSwDid, pstUpgradeResultInfo->result);

        IPM_ECU_FLASH_RESULT_T flashResult = {0};
        ADM_ACTIVE_STATE_E eActiveState = ADM_IPS_ACTIVE_END;
        DBOOL bNeedSetKeyLog = DFALSE;
        flashResult.dim = (ADM_IPM_DIM_T*)priv;
        (void)adm_snprintf(flashResult.ecuReqId, sizeof(flashResult.ecuReqId), "0X%X", pstIdenInfo->requestId);
        (void)adm_snprintf(flashResult.ecuSwDid, sizeof(flashResult.ecuSwDid), "0X%X", pstIdenInfo->swDid);
        flashResult.result = pstUpgradeResultInfo->result;

        /** 判断是否智能件单独激活 */
        if (3 == flashResult.result)
        {
            if (adm_ips_getIsActiveSmartEcu(flashResult.ecuReqId, flashResult.ecuSwDid, &eActiveState))
            {
                if (ADM_IPS_ACTIVE_PRE == eActiveState)
                {
                    log_i("Handle active reboot result.");
                    bNeedSetKeyLog = DTRUE;
                    flashResult.result = ADM_ECODE_SUCCESSED;  ///< 单独激活智能件，激活前待重启置升级结果为0
                }
                else 
                {
                    log_e("Ecu requestId[%s] swDid[%s] state[%d] is invalid, ignore it.", flashResult.ecuReqId, flashResult.ecuSwDid, eActiveState);
                    return ADM_ECODE_SUCCESSED;
                }
            }
            else 
            {
                bNeedSetKeyLog = DTRUE;
            }
        }
        else 
        {
            if (adm_ips_getIsActiveSmartEcu(flashResult.ecuReqId, flashResult.ecuSwDid, &eActiveState))
            {
                log_i("Handle active other result.");
                /** 已经触发了激活,激活后结果需要记录 */
                adm_ips_updateEcuActiveState(ADM_IPS_ACTIVE_END, flashResult.ecuReqId, flashResult.ecuSwDid);
                adm_ips_updateEcuActiveResult(flashResult.result, flashResult.ecuReqId, flashResult.ecuSwDid);

                if (ADM_IPS_ACTIVE_ING == eActiveState)
                {
                    DCHAR aDescription[128] = {0};
                    adm_snprintf(aDescription, sizeof(aDescription), "ecuReqId[%s]ecuSwDid[%s],active result[%d] ", flashResult.ecuReqId, flashResult.ecuSwDid, flashResult.result);
                    if ((0 != flashResult.result))
                    {
                        adm_pub_setKeylogString(ADM_PUB_KEYLOG_ID_014, flashResult.result, aDescription);
                    }
                    else
                    {
                        adm_pub_setKeylogString(ADM_PUB_KEYLOG_ID_013, 0, aDescription);
                    }
                    return ADM_ECODE_SUCCESSED;
                }
                else 
                {
                    bNeedSetKeyLog = DTRUE;
                }
            }
            else 
            {
                bNeedSetKeyLog = DTRUE;
            }
        }

        if (DTRUE == bNeedSetKeyLog)
        {
            DCHAR aDescription[128] = {0};
            adm_snprintf(aDescription, sizeof(aDescription), "ecuReqId[%s]ecuSwDid[%s],result[%d] ", flashResult.ecuReqId, flashResult.ecuSwDid,flashResult.result);
            if ((0 != flashResult.result))
            {
                adm_pub_setKeylogString(ADM_PUB_KEYLOG_ID_014, flashResult.result, aDescription);
            }
            else
            {
                adm_pub_setKeylogString(ADM_PUB_KEYLOG_ID_013, 0, aDescription);
            }
        }

        (void)ipmMsgCenterSendMsg(IPM_MSG_ECU_FLASH_RESULT_NOTIFY, &flashResult, (D32U)sizeof(flashResult));
    }
	
    return ADM_ECODE_SUCCESSED;
}

static D32S ipmDimSoaUcmUpgradeIccProgressNotify(D32S installType, D32S progress, void* priv){
    log_d("installType:%d, progress:%d", installType, progress);

    IPM_ICC_FLASH_PROGRESS_T flashProgress;
    flashProgress.dim = (ADM_IPM_DIM_T*)priv;
    flashProgress.installType = installType;
    flashProgress.progress = progress;

    (void)ipmMsgCenterSendMsg(IPM_MSG_ICC_FLASH_PROGRESS_NOTIFY, &flashProgress, (D32U)sizeof(flashProgress));
	
    return ADM_ECODE_SUCCESSED;
}

static D32S ipmDimSoaUcmUpgradeIccResultNotify(D32S installType, D32S result, void* priv){
    log_d("installType:%d, result:%d", installType, result);

    IPM_ICC_FLASH_RESULT_T flashResult = {0};
    flashResult.dim = (ADM_IPM_DIM_T*)priv;
    flashResult.installType = installType;
    flashResult.result = result;
    ADM_ACTIVE_STATE_E eActiveState = ADM_IPS_ACTIVE_END;
    DBOOL bNeedSetKeyLog = DFALSE;
    const DCHAR *pRequestId = NULL;
    const DCHAR *pSwDId = NULL;

    switch (flashResult.installType)
    {
    case ICC_SOC:
        pRequestId = ICC_REQ_ID;
        pSwDId = ICC_SW_DID;
        break;
    case ICC_MCU:
        pRequestId = ICC_REQ_ID;
        pSwDId = ICC_SW_DID;
        break;
    case ICC_SOC_MCU:
        pRequestId = ICC_REQ_ID;
        pSwDId = ICC_SW_DID;
        break;
    case ICC_INSTRUMENT_PANEL:
        pRequestId = ICC_INSTRUMENT_PANEL_REQ_ID;
        pSwDId = ICC_INSTRUMENT_PANEL_SW_DID;
        break;
    case ICC_CENTRAL_CTRL_SCREEN:
        pRequestId = ICC_CENTRAL_CTRL_SCREEN_REQ_ID;
        pSwDId = ICC_CENTRAL_CTRL_SCREEN_SW_DID;
        break;
    case ICC_SECOND_ROW_AIR_CONDITION_SCREEN:
        pRequestId = ICC_AIR_CONDITION_SCREEN_REQ_ID;
        pSwDId = ICC_AIR_CONDITION_SCREEN_SW_DID;
        break;
    case ICC_CEILING_MOUNTED_SCREEN:
        pRequestId = ICC_AIR_CEILING_MOUNT_REQ_ID;
        pSwDId = ICC_AIR_CEILING_MOUNT_SW_DID;
        break;
    default:
        log_e("invalid installType = %d", flashResult.installType);
        return ADM_ECODE_SUCCESSED;
    }

    /** 判断是否智能件单独激活 */
    if (3 == flashResult.result)
    {
        if (adm_ips_getIsActiveSmartEcu(pRequestId, pSwDId, &eActiveState))
        {
            if (ADM_IPS_ACTIVE_PRE == eActiveState)
            {
                log_i("Handle active reboot result.");
                bNeedSetKeyLog = DTRUE;
                flashResult.result = ADM_ECODE_SUCCESSED; ///< 单独激活智能件，激活前待重启置升级结果为0
            }
            else
            {
                log_e("Ecu requestId[%s] swDid[%s] state[%d] is invalid, ignore it.", pRequestId, pSwDId, eActiveState);
                return ADM_ECODE_SUCCESSED;
            }
        }
        else
        {
            bNeedSetKeyLog = DTRUE;
        }
    }
    else
    {
        if (adm_ips_getIsActiveSmartEcu(pRequestId, pSwDId, &eActiveState))
        {
            log_i("Handle active other result.");
            /** 已经触发了激活,激活后结果需要记录 */
            adm_ips_updateEcuActiveState(ADM_IPS_ACTIVE_END, pRequestId, pSwDId);
            adm_ips_updateEcuActiveResult(flashResult.result, pRequestId, pSwDId);

            if (ADM_IPS_ACTIVE_ING == eActiveState)
            {
                DCHAR aDescription[128] = {0};
                adm_snprintf(aDescription, sizeof(aDescription), "ecuReqId[%s]ecuSwDid[%s],active result[%d] ", pRequestId, pSwDId, flashResult.result);
                if ((0 != flashResult.result))
                {
                    adm_pub_setKeylogString(ADM_PUB_KEYLOG_ID_018, flashResult.result, aDescription);
                }
                else
                {
                    adm_pub_setKeylogString(ADM_PUB_KEYLOG_ID_017, 0, aDescription);
                }
                return ADM_ECODE_SUCCESSED;
            }
            else 
            {
                bNeedSetKeyLog = DTRUE;
            }
        }
        else
        {
            bNeedSetKeyLog = DTRUE;
        }
    }

    if (DTRUE == bNeedSetKeyLog)
    {
        DCHAR aDescription[128] = {0};
        adm_snprintf(aDescription, sizeof(aDescription), "installType[%d],result[%d] ", installType, flashResult.result);
        if ((0 != flashResult.result) && (3 != flashResult.result))
        {
            adm_pub_setKeylogString(ADM_PUB_KEYLOG_ID_018, flashResult.result, aDescription);
        }
        else
        {
            adm_pub_setKeylogString(ADM_PUB_KEYLOG_ID_017, 0, aDescription);
        }
    }

    (void)ipmMsgCenterSendMsg(IPM_MSG_ICC_FLASH_RESULT_NOTIFY, &flashResult, (D32U)sizeof(flashResult));
	
    return ADM_ECODE_SUCCESSED;
}

/* ADM_IPM_DIM_T */
static void ipmDimInit(ADM_IPM_DIM_T *dim, ADM_IPM_DO_FLASH_T *doFlash){
    dim->requestTimeout = ADM_IPM_DIM_REQUEST_TIMEOUT;
    dim->doFlash = doFlash;
#if 0
    static ADM_DIM_SOA_VDI_INIT_INFO_T vdiInitInfo = {
        .readDiagInfoCallback = NULL,
        .reflashProgressCallback = ipmDimReflashProgressNotify,
        .reflashResultCallback = ipmDimReflashResultSend,
        .spyVehicleStatusCallback = NULL,
        .vehicleSignalEventCallback = NULL
    };
    log_d("ret:%d", adm_dim_soa_vdi_init(NULL, &vdiInitInfo, dim));
    static ADM_DIM_SOA_UCL_INIT_INFO_T ucmInitInfo = {
	.readPartInfoCallback = NULL,
	.upgradeProgressNotifyCallback = ipmDimSoaUcmUpgradeProgressNotify,
	.upgradeResultNotifyCallback = ipmDimSoaUcmUpgradeResultNotify,
	.decryptProgressNotifyCallback = NULL,
	.decryptResultNotifyCallback = NULL,
	.verifyPackageHashProgressNotifyCallback = NULL,
	.verifyPackageHashResultNotifyCallback = NULL,
    };
    log_d("ret:%d", adm_dim_soa_ucl_init(&ucmInitInfo, dim));
#endif
}
static void ipmDimDeInit(ADM_IPM_DIM_T *dim){
#if 0
    adm_dim_soa_vdi_deinit();
    adm_dim_soa_ucl_deinit();
#endif
}


static DBOOL ipmGetHvMode(ADM_DIM_SOA_VDI_CTRL_VEHICLE_STATUS_TYPE_E eType)
{
    ADM_DIM_SOA_MSGID_DATA_ARRAY_T stReqMsgData = {};
    ADM_DIM_SOA_MSGID_DATA_ARRAY_T* pstRespMsgData=NULL;
    D32S iType = -1;
    D32S iRet = ADM_ECODE_FAILED;
    D32S iErrCode = ADM_ECODE_FAILED;
    DBOOL bIsSwitch = DTRUE;

    stReqMsgData.msgIdArrayLength = 1;
    stReqMsgData.msgIdArray = (ADM_DIM_SOA_VDI_MSGID_T**)adm_malloc(sizeof(ADM_DIM_SOA_VDI_MSGID_T*) * (DSIZE)stReqMsgData.msgIdArrayLength);
    
    (void)adm_memset(stReqMsgData.msgIdArray, 0, sizeof(ADM_DIM_SOA_VDI_MSGID_T*) * stReqMsgData.msgIdArrayLength,  
               sizeof(ADM_DIM_SOA_VDI_MSGID_T*) * stReqMsgData.msgIdArrayLength);
    
    stReqMsgData.msgIdArray[0] = (ADM_DIM_SOA_VDI_MSGID_T*)adm_malloc((DSIZE)sizeof(ADM_DIM_SOA_VDI_MSGID_T));

    log_i("begin request readVehicleStatus.");
    stReqMsgData.msgIdArray[0]->msgId  = VEHICLE_HIGH_VOLTAGE_STATUS;
    stReqMsgData.msgIdArray[0]->msgIdData = NULL;

    /** 判断是否非下电状态； 否则，退出升级 */
    iRet = adm_dim_host_readVehicleStatus(adm_soa_getHostInstanceId(), 15, &stReqMsgData, &pstRespMsgData, &iErrCode);
    if (ADM_IS_ERR(iRet) || ADM_IS_ERR(iErrCode))
    {
        /** 默认获取不到则切换 */
        log_w("adm_dim_host_readVehicleStatus call error.iRet[%d], errCode[%d]", iRet, iErrCode);
        return bIsSwitch;
    }

    if (NULL != pstRespMsgData && NULL != pstRespMsgData->msgIdArray && 
        VEHICLE_HIGH_VOLTAGE_STATUS == pstRespMsgData->msgIdArray[0]->msgId && 
        NULL != pstRespMsgData->msgIdArray[0]->msgIdData)
    {
        do
        {
            if (VAR_S32 != pstRespMsgData->msgIdArray[0]->msgIdData->dataType)
            {
                /** 给触发源成功,不要进行重试 */
                log_e("Value type invalid.type[%u]", pstRespMsgData->msgIdArray[0]->msgIdData->dataType);
                break;
            }
            iType = pstRespMsgData->msgIdArray[0]->msgIdData->v.valueS32;

            log_i("Src value[%d], dest value[%d]", iType, eType);
            /** 切换目标和实际高低压一致,则不执行切换动作 */
            if (((HV_HIGH == iType) && (eType == VEHICLE_CTRL_ENTER_HV_MODE)) || 
                ((HV_NO_HIGH == iType) && (eType == VEHICLE_CTRL_EXIT_HV_MODE)))
            {
                bIsSwitch = DFALSE;
                break;
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

    adm_free(stReqMsgData.msgIdArray[0]);
    adm_free(stReqMsgData.msgIdArray);
    
    return bIsSwitch;
}

static VEHICLE_VOLTAGE_STATE_E ipmGetCurrentHvMode(void)
{
    ADM_DIM_SOA_MSGID_DATA_ARRAY_T stReqMsgData = {};
    ADM_DIM_SOA_MSGID_DATA_ARRAY_T* pstRespMsgData = NULL;
    VEHICLE_VOLTAGE_STATE_E eType = HV_ABNORMAL;
    D32S iRet = ADM_ECODE_FAILED;
    D32S iErrCode = ADM_ECODE_FAILED;
    DBOOL bIsSwitch = DTRUE;
    D32S i = 0;

    ADM_COM_FUNC_ENTRY();

    stReqMsgData.msgIdArrayLength = 1;
    stReqMsgData.msgIdArray = (ADM_DIM_SOA_VDI_MSGID_T**)adm_malloc(sizeof(ADM_DIM_SOA_VDI_MSGID_T*) * (DSIZE)stReqMsgData.msgIdArrayLength);
    
    (void)adm_memzero(stReqMsgData.msgIdArray, sizeof(ADM_DIM_SOA_VDI_MSGID_T*) * stReqMsgData.msgIdArrayLength);
    
    stReqMsgData.msgIdArray[0] = (ADM_DIM_SOA_VDI_MSGID_T*)adm_malloc((DSIZE)sizeof(ADM_DIM_SOA_VDI_MSGID_T));

    log_i("begin request readVehicleStatus.");
    stReqMsgData.msgIdArray[0]->msgId  = VEHICLE_HIGH_VOLTAGE_STATUS;
    stReqMsgData.msgIdArray[0]->msgIdData = NULL;

    do
    {
        /** 判断是否非下电状态； 否则，退出升级 */
        iRet = adm_dim_host_readVehicleStatus(adm_soa_getHostInstanceId(), 15, &stReqMsgData, &pstRespMsgData, &iErrCode);
        if (ADM_IS_ERR(iRet) || ADM_IS_ERR(iErrCode))
        {
            /** 默认获取不到则切换 */
            log_w("adm_dim_host_readVehicleStatus call error.iRet[%d], errCode[%d]", iRet, iErrCode);
            iRet = ADM_IS_ERR(iRet) ? iRet : iErrCode;
            break;
        }

        if (NULL == pstRespMsgData || 
            NULL == pstRespMsgData->msgIdArray ||
            VEHICLE_HIGH_VOLTAGE_STATUS != pstRespMsgData->msgIdArray[0]->msgId ||
            NULL == pstRespMsgData->msgIdArray[0]->msgIdData)
        {
            iRet = ADM_ECODE_INVALID_POINTER;
            log_e("Invalid response data.");
            break;
        }

        if (VAR_S32 != pstRespMsgData->msgIdArray[0]->msgIdData->dataType)
        {
            /** 给触发源成功,不要进行重试 */
            log_e("Value type invalid.type[%u]", pstRespMsgData->msgIdArray[0]->msgIdData->dataType);
            iRet = ADM_ECODE_INVALID_POINTER;
            break;
        }
        
        eType = (VEHICLE_VOLTAGE_STATE_E)pstRespMsgData->msgIdArray[0]->msgIdData->v.valueS32;

        for (i = 0; i < pstRespMsgData->msgIdArrayLength; i++)
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
    } while (FALSE);

    adm_free(stReqMsgData.msgIdArray[0]);
    adm_free(stReqMsgData.msgIdArray);

    ADM_COM_FUNC_EXIT();

    return eType;
}

static D32S ipmSwitchHvMode(const ADM_IPM_ECU_INFO_NODE_T *node,
                            ADM_DIM_SOA_VDI_CTRL_VEHICLE_STATUS_TYPE_E eControlType,
                            VEHICLE_VOLTAGE_STATE_E eExpectState)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    D32S errorCode = ADM_ECODE_FAILED;
    VEHICLE_VOLTAGE_STATE_E eCurrentState = HV_ABNORMAL;
    D32U uiRetryCount = 0;
    D32S modeSwitch = -1;
    ADM_APPLICATION_COMMON_CMDID_TYPE_E eTrigger = ADM_APPLICATION_COMMON_CMDID_TYPE_INVALID;

    DBOOL bIsFuel = DFALSE;  

    ADM_COM_FUNC_ENTRY();

    iRet = adm_vsm_getIsFuel(&bIsFuel);
    if (ADM_IS_ERR(iRet))
    {
        log_e("Get vehicle power type failed, iRet:%d", iRet);
        return ADM_ECODE_FAILED;  // 直接返回失败
    }

    if (DTRUE == bIsFuel)
    {
        log_i("ICE Vechile.");
        return ADM_ECODE_SUCCESSED;
    }

    do
    {
        /** U盘升级和救援升级需要根据策略判断是否需要切换高压 */
        eTrigger = uimSoaCmdidTypeGet();
        if (ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_USB == eTrigger ||
            ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_RESCUE == eTrigger)
        {
            iRet = adm_cvs_getModeSwitch(&modeSwitch);
            if (ADM_IS_SUCCESS(iRet) && 0 == modeSwitch)
            {
                log_i("mode[%d],need not switch HV.", eTrigger);
                break;
            }
        }

        /** 切换模式 */
        iRet = adm_dim_host_setVehicleStatus(adm_soa_getHostInstanceId(),(D16U)node->ipsEcu.ecuTimeout, (D32U)eControlType, &errorCode);
        log_i("Switch to %d. instanceId[%u], iRet[%d], errcode[%d]", eControlType, adm_soa_getHostInstanceId(), iRet, errorCode);
        if (ADM_IS_ERR(iRet) || ADM_IS_ERR(errorCode))
        {
            iRet = ADM_IS_ERR(iRet) ? iRet : errorCode;
            if (VEHICLE_CTRL_ENTER_HV_MODE == eControlType)
            {
                iRet = ADM_ECODE_INSTALL_ENTER_HIGH_VOLTAGE_FAILED;
                ipmReportEventDependencyEventId("II_012", NULL, iRet);
            }
            else
            {
                iRet = ADM_ECODE_INSTALL_EXIT_HIGH_VOLTAGE_FAILED;
                ipmReportEventDependencyEventId("II_014", NULL, iRet);
            }
            break;
        }

        /** 切换成功之后，查询是否切换成功 */
        eCurrentState = ipmGetCurrentHvMode();

        /** 切换失败，进行重试 */
        if (eExpectState != eCurrentState)
        {
            log_d("Switch failed. expect[%d], current[%d]", eExpectState, eCurrentState);
            sleep(2);
            uiRetryCount++;

            if (uiRetryCount >= 3)
            {
                log_w("more than Retry count");
                if (VEHICLE_CTRL_ENTER_HV_MODE == eControlType)
                {
                    iRet = ADM_ECODE_INSTALL_ENTER_HIGH_VOLTAGE_FAILED;
                    ipmReportEventDependencyEventId("II_012", NULL, iRet);
                }
                else
                {
                    iRet = ADM_ECODE_INSTALL_EXIT_HIGH_VOLTAGE_FAILED;
                    ipmReportEventDependencyEventId("II_014", NULL, iRet);
                }
                break;
            }

            log_d("Retry[%d]", uiRetryCount);
            continue;
        }
        else
        {   
            iRet = ADM_ECODE_SUCCESSED;
            log_d("Switch success. expect[%d], current[%d]", eExpectState, eCurrentState);
            if (VEHICLE_CTRL_ENTER_HV_MODE == eControlType)
            {
                ipmReportEventDependencyEventId("II_011", NULL, ADM_ECODE_SUCCESSED);
            }
            else
            {
                ipmReportEventDependencyEventId("II_013", NULL, ADM_ECODE_SUCCESSED);
            }
            break;
        }
    } while (TRUE);

    ADM_COM_FUNC_EXIT();

    return iRet;
}

static D32S ipmDimIgSwitchCtrl(const ADM_IPM_ECU_INFO_NODE_T *node)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    DBOOL bIsSwitch = DTRUE;
    VEHICLE_VOLTAGE_STATE_E eExpectState = HV_ABNORMAL;
    VEHICLE_VOLTAGE_STATE_E eCurrentState = HV_ABNORMAL;
    ADM_DIM_SOA_VDI_CTRL_VEHICLE_STATUS_TYPE_E eType;

    ADM_COM_FUNC_ENTRY();

    if(node == NULL)
    {
        log_e("invalid param node");
        return ADM_ECODE_INVALID_PARAM;
    }

    if(node->ecsEcu == NULL)
    {
        log_e("invalid param node->ecsEcu");
        return ADM_ECODE_INVALID_POINTER;
    }

    log_i("ecuReqId[ %x ] instCondition[ %d ] ecuTimeConsuming[ %d ] ecuTimeout[ %d ]", node->ecsEcu->ecuReqId, 
                                                                                        node->ecsEcu->instCondition, 
                                                                                        node->ipsEcu.ecuTimeConsuming, 
                                                                                        node->ipsEcu.ecuTimeout);

    do
    {
        /** 安装约束, 零件需要处于低压/高压条件下才可以刷写(1:high, 2:low) */
        if (node->ecsEcu->instCondition == 1)
        {
            eType = VEHICLE_CTRL_ENTER_HV_MODE;
            eExpectState = HV_HIGH;
        }
        else
        {
            eType = VEHICLE_CTRL_EXIT_HV_MODE;
            eExpectState = HV_NO_HIGH;
        }

        /** 获取当前高压状态 */
        // eCurrentState = ipmGetCurrentHvMode();

        // log_d("eCurrentState: %d, eExpectState: %d", eCurrentState, eExpectState);
        /** 当前高压状态与期望状态一致，不需要切换 */
        // if (eExpectState == eCurrentState)
        // {
        //     iRet = ADM_ECODE_SUCCESSED;
        //     break;
        // }   /** 股份要求，直接切换高低压，不需要查询 */ 

        log_i("[ABUP-Procedure]: Install Start High & Low mode switch");
        /** 执行切换高压 */
        iRet = ipmSwitchHvMode(node, eType, eExpectState);
        if (ADM_IS_ERR(iRet))
        {
            log_d("switch hv mode failed");
        }
        log_i("[ABUP-Procedure]: Install End High & Low mode switch, [err:%d]", iRet);
    } while (FALSE);

    log_d("switch hv mode result = %d", iRet);

    ADM_COM_FUNC_EXIT();

    return iRet;
}

static DVOID ipmDimIgSwitch2NoActive()
{
    D32S iRet = ADM_ECODE_FAILED;
    D32S iErrCode = ADM_ECODE_FAILED;
    D32S modeSwitch = -1;
    ADM_APPLICATION_COMMON_CMDID_TYPE_E eTrigger = ADM_APPLICATION_COMMON_CMDID_TYPE_INVALID;

    DBOOL bIsFuel = DFALSE;  

    /** U盘升级和救援升级需要根据策略判断是否需要切换高压 */
    eTrigger = uimSoaCmdidTypeGet();
    if (ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_USB == eTrigger ||
        ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_RESCUE == eTrigger)
    {
        iRet = adm_cvs_getModeSwitch(&modeSwitch);
        if (ADM_IS_SUCCESS(iRet) && 0 == modeSwitch)
        {
            log_i("mode[%d],need not switch not active state.", eTrigger);
            return;
        }
    }

    iRet = adm_vsm_getIsFuel(&bIsFuel);
    if (ADM_IS_ERR(iRet))
    {
        log_e("Get vehicle power type failed, iRet:%d", iRet);
        return;  // 直接返回失败
    }

    if (DTRUE == bIsFuel)
    {
        log_i("ICE Vechile.");
        return;
    }

    for (D32S i = 0; i < 3; i++)
    {
        iRet = adm_dim_host_setVehicleStatus(adm_soa_getHostInstanceId(),10, (D32U)VEHICLE_CTRL_NO_ACTIVE_HV_MODE, &iErrCode);
        if (ADM_IS_ERR(iRet) || ADM_IS_ERR(iErrCode))
        {
            log_i("Switch to hv no active failed.");
        }
        else 
        {
            log_i("Switch to hv no active succeed.");
            break;
        }
        sleep(1);
    }
    
    return;
}

static DVOID ipmDimWaitHmiOnline(void)
{
    D32S iRet = ADM_ECODE_FAILED;
    DCHAR* pSdkVersion = DNULL;
    D32S iErrCode = ADM_ECODE_FAILED;
    D32S iCount = 0;
    D32S i = 0;

    for (i = 0; i < 60; i++)
    {
        log_d("wait hmi online");
        if (TRUE == adm_vsm_getHmiOnline())
        {
            log_d("hmi online");
            break;
        }

        sleep(3);
    }

    do
    {
        iRet = adm_dim_ucl_getSdkVersion(0x2002, 2, &pSdkVersion, &iErrCode);
        if (ADM_ECODE_SUCCESSED == iRet && NULL != pSdkVersion)
        {
            log_d("get ucl sdk version[%s]", pSdkVersion);
            adm_free(pSdkVersion);
            log_i("[ABUP-Procedure]Hmi is online, break");
            break;
        }
        iCount ++;
        sleep(3);
        if (iCount > 20)
        {
            log_e("HMI is not online, break");
            break;
        }
    } while (TRUE);

    return;
}

/*! return value 0.request failed 1.request success */
static D32S ipmDimFlashEcuRequest(ADM_IPM_DIM_T *dim, const ADM_IPM_ECU_INFO_NODE_T *node){
    D32S ret = ADM_ECODE_FAILED;
    D32S errorCode = ADM_ECODE_FAILED;
    DCHAR apkPackPath[ADM_COM_LEN_256] = {0};
    DCHAR apkVersion[ADM_COM_LEN_256] = {0};
    DBOOL bIsAb = DFALSE;
    DBOOL bIsEnableAb = DFALSE;
    DBOOL bIsRollback = DFALSE;
    DBOOL bIsNeedSwtichAb = DFALSE;
    ADM_DIM_SOA_VDI_ECU_AREA_INFO_E eCurrentAbInfo = ADM_SYSI4VDI_ECU_AREA_INFO_UNKNOW;
    ADM_DIM_SOA_VDI_ECU_AREA_INFO_E eDestAbInfo = ADM_SYSI4VDI_ECU_AREA_INFO_UNKNOW;
    ADM_DIM_SOA_VDI_ECU_AREA_INFO_E eExitDestAbInfo = ADM_SYSI4VDI_ECU_AREA_INFO_UNKNOW;
    ADM_DIM_SOA_ECU_IDEN_INFO_T stDimEcuIden = {0};
    D32U uiTimeout = 0;
    D16U uclInstanceId = 0;
    D16U tboxInstanceId = 0;
    D16U adccInstanceId = 0;
    DBOOL bIsControlActive = DFALSE;

    uclInstanceId = adm_soa_getUclInstanceId();
    tboxInstanceId = adm_soa_getHostTboxInstanceId();
    adccInstanceId = adm_soa_getUclAdccInstanceId();

    log_d("%s:%p", ADM_STR_FROM_PARAM(node->ecsEcu), node->ecsEcu);
    if (node->ecsEcu != NULL)
    {
        log_d("%s:%s, %s:%s, %s:%d, %s:%s",
              ADM_STR_FROM_PARAM(node->ipsEcu.ecuReqId), ADM_STR_SAFE_OUTPUT(node->ipsEcu.ecuReqId),
              ADM_STR_FROM_PARAM(node->ipsEcu.ecuSwDid), ADM_STR_SAFE_OUTPUT(node->ipsEcu.ecuSwDid),
              ADM_STR_FROM_PARAM(node->ecsEcu->serviceType), node->ecsEcu->serviceType,
              ADM_STR_FROM_PARAM(node->ecsEcu->installInstanceId), ADM_STR_SAFE_OUTPUT(node->ecsEcu->installInstanceId));
        D32S serviceType = node->ecsEcu->serviceType;
        const DCHAR *installInstanceId = node->ecsEcu->installInstanceId;
        if (serviceType != 0 && installInstanceId != NULL)
        {
            if (serviceType == 1 /*VDI*/)
            {
                ADM_DIM_SOA_VDI_REFLASH_INFO_ARRAY_T reflashInfoList = {0};
                ADM_IPS_ECU_PROGRESS_T stProgress = {0};
                log_i("[ABUP-Procedure]: Install Start Vdi Flash trigger");
                if (ADM_ECODE_SUCCESSED == dimSoaVdiReflashInfoArrayInit(&reflashInfoList, node))
                {
                    adm_ipm_getIsRollback(node->ipsEcu.ecuReqId, node->ipsEcu.ecuSwDid, &bIsRollback);
                    /** 获取AB分区 */
                    ret = adm_ecs_getEcuSelfBackUp(node->ipsEcu.ecuReqId, node->ipsEcu.ecuSwDid, &bIsAb);
                    if (ADM_IS_SUCCESS(ret) && DTRUE == bIsAb)
                    {
                        /** 增加超时时间,兼容德赛qnx从控莫名进程挂起20多秒的场景 */
                        ret = adm_ipm_getAbInfo((D16U)ipmHtoi(installInstanceId), dim->requestTimeout + 20, 
                                    &bIsEnableAb, &eCurrentAbInfo, &reflashInfoList);
                        log_i("Get ab info enableAb[%d], abInfo[%d]", bIsEnableAb, eCurrentAbInfo);
                        if (ADM_IS_SUCCESS(ret) && DTRUE == bIsEnableAb)
                        {
                            adm_ipm_getDestAbInfo(eCurrentAbInfo, &eDestAbInfo);

                            adm_ipm_getExitDestAbInfo(node->ecsEcu->ecuReqId, node->ecsEcu->ecuSwDid, &eExitDestAbInfo);

                            if (DFALSE == bIsRollback && eCurrentAbInfo == eExitDestAbInfo)
                            {
                                log_e("AB ecu find current ab info[%d] eques to dest ab info [%d].", eCurrentAbInfo, eExitDestAbInfo);
                                ret = ADM_ECODE_INSTALL_ECU_ITSELF_IS_ABNORMAL;
                                dimSoaVdiReflashInfoArrayDeInit(&reflashInfoList);
                                ADM_DEBUG_RETURN_D32S(ret);

                            }

                            /* fcm包比较大，先临时解决,后续优化逻辑 */
                            if (1 == reflashInfoList.reflashInfoArray[0]->flashProgressType)
                            {
                                uiTimeout = 5 * 60;
                            }
                            else
                            {
                                /** 增加超时时间,兼容德赛qnx从控莫名进程挂起20多秒的场景 */
                                uiTimeout = dim->requestTimeout + 20;
                            }

                            ret = adm_dim_vdi_AbReflashEcusTrigger((D16U)ipmHtoi(installInstanceId), uiTimeout, 
                                bIsEnableAb, eDestAbInfo, &reflashInfoList, &errorCode);
                        }
                        else 
                        {
                            log_e("Read ab info failed.");
                            ret = ADM_ECODE_INSTALL_ECU_ITSELF_IS_ABNORMAL;
                        }
                    } 
                    else 
                    {
                        log_i("Do not belong to AB ecu.");
                        ret = adm_dim_vdi_reflashEcusTrigger((D16U)ipmHtoi(installInstanceId), dim->requestTimeout + 20, &reflashInfoList, &errorCode);
                    }
                }

                /** 更新AB分区信息 */
                if (DFALSE == bIsRollback)
                {
                    adm_ipm_updateFlashAbInfo(bIsAb, node->ecsEcu->ecuReqId, node->ecsEcu->ecuSwDid, 
                                        reflashInfoList.reflashInfoArray[0]->isBlockPartion, eDestAbInfo);
                }

                log_i("[ABUP-Procedure]: Install End Vdi Flash trigger, [err:%d]", ret);
                dimSoaVdiReflashInfoArrayDeInit(&reflashInfoList);
            }
            else if (serviceType == 2 /*UCM*/)
            {
                ADM_DIM_SOA_UCL_UPGRADE_INFO_ARRAY_T upgradeInfoList;
                log_i("[ABUP-Procedure]: Install Start UCL Upgrade");
                if (ADM_ECODE_SUCCESSED == dimSoaUcmUpgradeInfoArrayInit(&upgradeInfoList, node))
                {
                    if ((D16U)ipmHtoi(installInstanceId) == uclInstanceId || (D16U)ipmHtoi(installInstanceId) == tboxInstanceId ||
                        (D16U)ipmHtoi(installInstanceId) == adccInstanceId)
                    {
                        /** 智能件从控激活是否受OTA控制 */
                        if (-1 != adm_ips_ecuActiveOrderGet(node->ecsEcu->ecuReqId, node->ecsEcu->ecuSwDid))
                        {
                            log_i("Control instance[%s] active.", installInstanceId);
                            bIsControlActive = DTRUE;
                        }
                    }

                    if ((D16U)ipmHtoi(installInstanceId) == uclInstanceId)
                    {
#ifndef ADM_ACTIVATION_SUPPORT
                            ret = adm_dim_ucl_requestDoUpgrade((D16U)ipmHtoi(installInstanceId), dim->requestTimeout, &upgradeInfoList, bIsControlActive, &errorCode);
#else
                            DCHAR *pTriggerInfo = ipmIccInstallTriggerInfoGet();
                            if(ADM_IS_NULL(pTriggerInfo))
                            {
                                /**2-SOC 升级*/
                                ret = adm_dim_ucl_requestDoUpgrade_icc((D16U)ipmHtoi(installInstanceId), dim->requestTimeout, &upgradeInfoList, ICC_SOC , 0, &errorCode);
                            }
                            else
                            {
                                /**3-MCU 升级*/
                                ret = adm_dim_ucl_requestDoUpgrade_icc((D16U)ipmHtoi(installInstanceId), dim->requestTimeout, &upgradeInfoList, ICC_MCU , 0, &errorCode);
                            }
                            adm_free(pTriggerInfo);
#endif
                    }
                    else
                    {
                        ret = adm_dim_ucl_requestDoUpgrade((D16U)ipmHtoi(installInstanceId), dim->requestTimeout, &upgradeInfoList, bIsControlActive, &errorCode);
                    }
                }
                log_i("[ABUP-Procedure]: Install End UCL Upgrade, [err:%d]", ret);
                dimSoaUcmUpgradeInfoArrayDeInit(&upgradeInfoList);
            }
            else if (serviceType == 3 /* APK */)
            {
                //!< APK 自升级的执行逻辑
                dimSoaUcmEcuIdenInfoInit(&stDimEcuIden, &(node->ipsEcu));
                adm_memset(apkPackPath, 0x00, sizeof(apkPackPath), sizeof(apkPackPath));
                adm_memset(apkVersion, 0x00, sizeof(apkVersion), sizeof(apkVersion));
                adm_snprintf(apkPackPath, sizeof(apkPackPath), "%s", node->pisData.imagePackage.path);
                adm_snprintf(apkVersion, sizeof(apkVersion), "%s", "ABUP_DEFAULT_VERSION");

                ipmDimWaitHmiOnline();
                log_i("[ABUP-Procedure]: Install Start Apk Self Upgrade");
                if (ADM_EIM_APK_UCM_REQUEST_ID == ipmHtoi(node->ipsEcu.ecuReqId))
                {
                    log_i("UCM APK self upgrade, adm_dim_ucl_doApkUpgrade instanceId:%d apkPackPath:%s, apkVersion:%s begin", (D16U)ipmHtoi(installInstanceId), apkPackPath, apkVersion);
                    ret = adm_dim_ucl_doApkUpgrade((D16U)ipmHtoi(installInstanceId), dim->requestTimeout, &stDimEcuIden, apkPackPath, apkVersion, &errorCode);
                    errorCode = ret;
                }
                else if (ADM_EIM_APK_HMI_REQUEST_ID == ipmHtoi(node->ipsEcu.ecuReqId))
                {
                    log_i("HMI APK self upgrade, adm_ucl_host_doApkUpgrade instanceId:%d apkPackPath:%s, apkVersion:%s begin", (D16U)ipmHtoi(installInstanceId), apkPackPath, apkVersion);
                    ret = adm_dim_ucl_doApkUpgrade((D16U)ipmHtoi(installInstanceId), dim->requestTimeout, &stDimEcuIden, apkPackPath, apkVersion, &errorCode);
                }
                log_i("[ABUP-Procedure]: Install End Apk Self Upgrade, [err:%d]", ret);
                dimSoaUcmEcuIdenInfoDeInit(&stDimEcuIden);
            }
            else
            {
                ret = ADM_ECODE_FAILED;
                log_e("invalid serviceType:%d", serviceType);
            }
        }
    }

    log_d("ret:%d, errorCode:%d", ret, errorCode);
    if (ret == ADM_ECODE_SUCCESSED && errorCode != ADM_ECODE_SUCCESSED) {
        ret = errorCode;
    }
    ADM_DEBUG_RETURN_D32S(ret);
}
static DBOOL ipmDimFlashEcuQueryProgress(ADM_IPM_DIM_T *dim, const ADM_IPM_ECU_INFO_NODE_T *node, D32S *progress){
    DBOOL dRet = FALSE;
    D32S ret = ADM_ECODE_FAILED;
    D32S errorCode = ADM_ECODE_FAILED;
    D16U reflashProgress = 0;
    log_d("%s:%p", ADM_STR_FROM_PARAM(node->ecsEcu), node->ecsEcu);
    if(node->ecsEcu != NULL){
        log_d("%s:%s, %s:%s, %s:%d, %s:%s",
            ADM_STR_FROM_PARAM(node->ipsEcu.ecuReqId), ADM_STR_SAFE_OUTPUT(node->ipsEcu.ecuReqId),
            ADM_STR_FROM_PARAM(node->ipsEcu.ecuSwDid), ADM_STR_SAFE_OUTPUT(node->ipsEcu.ecuSwDid),
            ADM_STR_FROM_PARAM(node->ecsEcu->serviceType), node->ecsEcu->serviceType,
            ADM_STR_FROM_PARAM(node->ecsEcu->installInstanceId), ADM_STR_SAFE_OUTPUT(node->ecsEcu->installInstanceId));
        D32S serviceType = node->ecsEcu->serviceType;
        const DCHAR* installInstanceId = node->ecsEcu->installInstanceId;
        if(serviceType != 0 && installInstanceId != NULL){
            if(serviceType == 1/*VDI*/){
                ADM_DIM_SOA_ECU_IDEN_INFO_T ecuIdenInfo;
                dimSoaVdiEcuIdenInfoInit(&ecuIdenInfo, node->ecsEcu);
                ret = adm_dim_vdi_queryReflashProgress((D16U)ipmHtoi(installInstanceId), dim->requestTimeout, &ecuIdenInfo, &reflashProgress, &errorCode);
                dimSoaVdiEcuIdenInfoDeInit(&ecuIdenInfo);
            }else if(serviceType == 2/*UCM*/){
                ADM_DIM_SOA_ECU_IDEN_INFO_T ecuIdenInfo;
                dimSoaUcmEcuIdenInfoInit(&ecuIdenInfo, &node->ipsEcu);
				/**针对主机升级0X2002不去查询进度*/
				if(0 != adm_strcmp(installInstanceId, "0x2002"))
				{
	                ret = adm_dim_ucl_queryUpgradeProgress((D16U)ipmHtoi(installInstanceId), dim->requestTimeout, &ecuIdenInfo, &reflashProgress, &errorCode);
				}
                dimSoaUcmEcuIdenInfoDeInit(&ecuIdenInfo);
            }else if(serviceType == 3/*APK自升级*/){
                ADM_DIM_SOA_ECU_IDEN_INFO_T ecuIdenInfo;
                dimSoaUcmEcuIdenInfoInit(&ecuIdenInfo, &node->ipsEcu);
                ret = adm_dim_ucl_queryApkSelfUpgradeProgress((D16U)ipmHtoi(installInstanceId), dim->requestTimeout, &ecuIdenInfo, &reflashProgress, &errorCode);
                dimSoaUcmEcuIdenInfoDeInit(&ecuIdenInfo);
            }
            else {
                log_e("invalid serviceType :%d", serviceType);
            }
        }
    }
    
    log_d("ret:%d, errorCode:%d, reflashProgress:%u", ret, errorCode, reflashProgress);
    if(ADM_ECODE_SUCCESSED == ret && ADM_ECODE_SUCCESSED == errorCode){
        *progress = (D32S)reflashProgress;
        dRet = TRUE;
    }

    return dRet;
}
static DBOOL ipmDimFlashEcuQueryResult(ADM_IPM_DIM_T *dim, const ADM_IPM_ECU_INFO_NODE_T *node, D32S *result, DCHAR **pResultDesc){
    DBOOL dRet = FALSE;
    D32S ret = ADM_ECODE_FAILED;
    D32S errorCode = ADM_ECODE_FAILED;
    log_d("%s:%p", ADM_STR_FROM_PARAM(node->ecsEcu), node->ecsEcu);
    if(node->ecsEcu != NULL){
	    log_d("%s:%s, %s:%s, %s:%d, %s:%s", 
		    ADM_STR_FROM_PARAM(node->ipsEcu.ecuReqId), ADM_STR_SAFE_OUTPUT(node->ipsEcu.ecuReqId),
		    ADM_STR_FROM_PARAM(node->ipsEcu.ecuSwDid), ADM_STR_SAFE_OUTPUT(node->ipsEcu.ecuSwDid),
		    ADM_STR_FROM_PARAM(node->ecsEcu->serviceType), node->ecsEcu->serviceType,
		    ADM_STR_FROM_PARAM(node->ecsEcu->installInstanceId), ADM_STR_SAFE_OUTPUT(node->ecsEcu->installInstanceId));
	    D32S serviceType = node->ecsEcu->serviceType;
	    const DCHAR *installInstanceId = node->ecsEcu->installInstanceId;
        if(serviceType != 0 && installInstanceId != NULL){
            if(serviceType == 1/*VDI*/){
		        ADM_DIM_SOA_ECU_IDEN_INFO_T ecuIdenInfo;
		        dimSoaVdiEcuIdenInfoInit(&ecuIdenInfo, node->ecsEcu);
		        ADM_DIM_SOA_VDI_REFLASH_RESULT_INFO_T *pstReflashResultSendInfo = NULL;
		        ret = adm_dim_vdi_queryReflashResult((D16U)ipmHtoi(installInstanceId), dim->requestTimeout, &ecuIdenInfo,  &pstReflashResultSendInfo, &errorCode);
                if (NULL != pstReflashResultSendInfo) {
                    if (ADM_ECODE_SUCCESSED == ret && ADM_ECODE_SUCCESSED == errorCode) {
                        *result = pstReflashResultSendInfo->result;
                        *pResultDesc = adm_strdup(ADM_STR_SAFE_OUTPUT(pstReflashResultSendInfo->realVal));
                        dRet = TRUE;
                    }
                    dimSoaVdiReflashResultInfoDel(pstReflashResultSendInfo);
                }
		        dimSoaVdiEcuIdenInfoDeInit(&ecuIdenInfo);
            }else if(serviceType == 2/*UCM*/){
                ADM_DIM_SOA_ECU_IDEN_INFO_T ecuIdenInfo;
                dimSoaUcmEcuIdenInfoInit(&ecuIdenInfo, &node->ipsEcu);
                ADM_DIM_SOA_UCL_UPGRADE_RESULT_INFO_T* pstUpgradeResultInfo = NULL;
                ret = adm_dim_ucl_queryUpgradeResult((D16U)ipmHtoi(installInstanceId), dim->requestTimeout, &ecuIdenInfo, &pstUpgradeResultInfo, &errorCode);
                if (NULL != pstUpgradeResultInfo) {
                    if (ADM_ECODE_SUCCESSED == ret && ADM_ECODE_SUCCESSED == errorCode) {
                        *result = pstUpgradeResultInfo->result;
                        dRet = TRUE;
                    }
                    dimSoaUclUpgradeResultInfoDel(pstUpgradeResultInfo);
                }
		        dimSoaUcmEcuIdenInfoDeInit(&ecuIdenInfo);
            }else if(serviceType == 3/*APK*/){
                ADM_DIM_SOA_ECU_IDEN_INFO_T ecuIdenInfo;
                dimSoaUcmEcuIdenInfoInit(&ecuIdenInfo, &node->ipsEcu);
                ADM_DIM_SOA_UCL_UPGRADE_RESULT_INFO_T* pstUpgradeResultInfo = NULL;
                ret = adm_dim_ucl_queryApkSelfUpgradeResult((D16U)ipmHtoi(installInstanceId), dim->requestTimeout, &ecuIdenInfo, &pstUpgradeResultInfo, &errorCode);
                if (NULL != pstUpgradeResultInfo) {
                    if (ADM_ECODE_SUCCESSED == ret && ADM_ECODE_SUCCESSED == errorCode) {
                        *result = pstUpgradeResultInfo->result;
                        dRet = TRUE;
                    }
                    dimSoaUclUpgradeResultInfoDel(pstUpgradeResultInfo);
                }
                dimSoaUcmEcuIdenInfoDeInit(&ecuIdenInfo);
            }
			else 
			{
                log_e("invalid serviceType:%d", serviceType);
			}
        }
    }
    log_d("ret:%d, errorCode:%d, dRet:%d, result:%d", ret, errorCode, dRet, *result);
    return dRet;
}
/*
static DBOOL ipmDimEcusPackageDecrypt(ADM_IPM_DIM_T *dim){
#if 1
    return 1;
#else
    DBOOL dRet = 0;
#if 1
    //TODO params
    DCHAR* fileID = "x";
    DCHAR* filePath = "x";
    DCHAR* decryptKey = "x";
    D64S decryptSegLength = 1;
    DCHAR* hash = "x";
#endif
    log_d("fileID:%s, filePath:%s, decryptKey:%s, decryptSegLength:%lld, hash:%s", 
	    ADM_STR_SAFE_OUTPUT(fileID), 
	    ADM_STR_SAFE_OUTPUT(filePath), 
	    ADM_STR_SAFE_OUTPUT(decryptKey), 
	    decryptSegLength, 
	    ADM_STR_SAFE_OUTPUT(hash));
    if(fileID == NULL 
	    || filePath == NULL 
	    || decryptKey == NULL 
	    || decryptSegLength == 0 
	    || hash == NULL){}else{
        D32S errorCode = ADM_ECODE_FAILED;
        D32S ret = adm_dim_soa_ucl_requestDecryptPackage(fileID, filePath, decryptKey, decryptSegLength, hash, &errorCode);
	if(ret == 0){
	    if(errorCode == ADM_ECODE_SUCCESSED){
		dRet = 1;
	    }
	}
        log_d("ret:%d, errorCode:%d, dRet:%d", ret, errorCode, dRet);
    }
    return dRet;
#endif
}
static DBOOL ipmDimEcusPackageVerifySign(ADM_IPM_DIM_T *dim){
#if 1
    return 1;
#else
    DBOOL dRet = 0;
#if 1
    //TODO params
    DCHAR* signedData = "x";
    D32S signedDataLength = 1;
    DCHAR* orignalData = "x";
    D32S orignalDataLength = 1;
#endif
    log_d("signedData:%s, signedDataLength:%d, orignalData:%s, orignalDataLength:%d", 
	    ADM_STR_SAFE_OUTPUT(signedData), 
	    signedDataLength, 
	    ADM_STR_SAFE_OUTPUT(orignalData), 
	    orignalDataLength);
    if(signedData == NULL
	   || signedDataLength == 0 
	   || orignalData == NULL 
	   || orignalDataLength == 0){}else{
        D32S errorCode = ADM_ECODE_FAILED;
        D32S ret = adm_dim_soa_host_verifySignature(signedData, signedDataLength, orignalData, orignalDataLength, &errorCode);
	if(ret == 0){
	    if(errorCode == ADM_ECODE_SUCCESSED){
		dRet = 1;
	    }
	}
        log_d("ret:%d, errorCode:%d, dRet:%d", ret, errorCode, dRet);
    }
    return dRet;
#endif
}
*/
static D32S ipmDimEnvEntry(ADM_IPM_DIM_T *dim){
    //TODO DIM PRD03-158
    D32S ret = ADM_ECODE_FAILED;
    ADM_CVS_UPGRADE_MODE_E eUpgradeMode = ADM_CVS_UPGRADE_MODE_INVALID;

    adm_cvs_getUpgradeMode((D32S *)&eUpgradeMode);

    ///!< APK 自升级的环境进入，只需要维持唤醒状态，不需要唤醒整车
    if (ADM_CVS_UPGRADE_MODE_APK_SELF == eUpgradeMode)
    {
        log_i("[ABUP-Procedure]: Install Start wakeup");
        if((ret = ipmDimSoaHostExecOtaTaskStartWakeUp(dim)) == ADM_ECODE_SUCCESSED){
			ipmReportEventDependencyEventId("II_003",  NULL, ADM_ECODE_SUCCESSED);
        }
        else{
            ipmReportEventDependencyEventId("II_002", NULL, ret);
        }

        log_i("[ABUP-Procedure]: Install Start wakeup end, [err:%d]", ret);
        ADM_ECODE_APPLICATION_CONVERSION(ret, ADM_ECODE_INSTALL_ENTER_OTAMODE_FAILED);
        return ret;
    }

    if((ret = ipmDimSoaVdiPerformSpecDiagnosticEnterDiagMode(dim)) == ADM_ECODE_SUCCESSED){
		ipmReportEventDependencyEventId("II_001",  NULL, ADM_ECODE_SUCCESSED);
	    if((ret = ipmDimSoaHostExecOtaTaskStartWakeUp(dim)) == ADM_ECODE_SUCCESSED){
			ipmReportEventDependencyEventId("II_003",  NULL, ADM_ECODE_SUCCESSED);
            log_i("[ABUP-Procedure]: Install Start Entry OTA Mode");
	        ret = ipmDimSoaVdiControlVehicleStatusOTAModeEntry(dim);
	        if(ret != ADM_ECODE_SUCCESSED){
				ipmReportEventDependencyEventId("II_006",  NULL, ret);
		        (void)ipmDimSoaHostExecOtaTaskStopWakeUp(dim);
	        }
			else{
                ipmReportEventDependencyEventId("II_005",  NULL, ADM_ECODE_SUCCESSED);
			}
            log_i("[ABUP-Procedure]: Install End Entry OTA Mode, [err:%d], ", ret);
	    }
		else{
            ipmReportEventDependencyEventId("II_004",  NULL, ret);
	    }
	    if(ret != ADM_ECODE_SUCCESSED){
	        (void)ipmDimSoaVdiPerformSpecDiagnosticExitDiagMode(dim);
	    }
    }
	else{
        ipmReportEventDependencyEventId("II_002", NULL, ret);
	}

    ADM_ECODE_APPLICATION_CONVERSION(ret, ADM_ECODE_INSTALL_ENTER_OTAMODE_FAILED);
    return ret;
}
static D32S ipmDimEnvExit(ADM_IPM_DIM_T *dim){
    //TODO DIM PRD03-158
    D32S ret = ADM_ECODE_FAILED;
    D32S r = ADM_ECODE_FAILED;
    D32S iErrorCode = ADM_ECODE_FAILED;
    D32S iSetDiagResult = ADM_ECODE_FAILED;
    ADM_CVS_UPGRADE_MODE_E eUpgradeMode = ADM_CVS_UPGRADE_MODE_INVALID;
    ///< 告知车机OTA 高压未激活状态
    ipmDimIgSwitch2NoActive();

    adm_cvs_getUpgradeMode((D32S *)&eUpgradeMode);

    /** modi maxy 20215/12/30 股份车型APK自升级也需要退出ota模式，因为apk自升级是由tsp下发的，tsp下发检测阶段就已进入ota模式了 */
    ///!< APK 自升级的环境退出，只需要退出维持唤醒
    // if (ADM_CVS_UPGRADE_MODE_APK_SELF == eUpgradeMode)
    // {
    //     log_i("[ABUP-Procedure]: Install Start Stop Wake up");
    //     if((r = ipmDimSoaHostExecOtaTaskStopWakeUp(dim)) != ADM_ECODE_SUCCESSED){
    //         ret = r;
    //     }
    //     else
    //     {
    //         ret = ADM_ECODE_SUCCESSED;
    //     }
    //     log_i("[ABUP-Procedure]: Install End Stop Wake up, [err:%d]", ret);
    //     ADM_ECODE_APPLICATION_CONVERSION(ret, ADM_ECODE_INSTALL_EXIT_OTAMODE_FAILED);
    //     return ret;
    // }


    log_i("[ABUP-Procedure]: Install Start Exit OTA Mode");
    ret = ipmDimSoaVdiControlVehicleStatusOTAModeExit(dim);
	if(ADM_ECODE_SUCCESSED == ret){
        ipmReportEventDependencyEventId("S_IE_003",  NULL, ADM_ECODE_SUCCESSED);
    }
    else{
        ipmReportEventDependencyEventId("S_IE_004",  NULL, ret);
    }
    log_i("[ABUP-Procedure]: Install Start Exit OTA Mode, [err:%d]", ret);
	
    if((r = ipmDimSoaVdiPerformSpecDiagnosticExitDiagMode(dim)) != ADM_ECODE_SUCCESSED){
		ret = r;
		ipmReportEventDependencyEventId("S_IE_006",  NULL, ret);
	}
	else{
        ipmReportEventDependencyEventId("S_IE_005",  NULL, ADM_ECODE_SUCCESSED);
	}

    log_i("[ABUP-Procedure]: Install Start Stop Wake up");
    if((r = ipmDimSoaHostExecOtaTaskStopWakeUp(dim)) != ADM_ECODE_SUCCESSED){
        ret = r;
    }
    log_i("[ABUP-Procedure]: Install End Stop Wake up, [err:%d]", ret);

    log_i("[ABUP-Procedure]: Install Start Set Diag Tp");
    iSetDiagResult = adm_dim_vdi_setDiagTp(adm_soa_getVdiInstanceId(), dim->requestTimeout, &iErrorCode);
    if (ADM_ECODE_SUCCESSED == iSetDiagResult)
    {
        log_d("iErrorCode:%d", iErrorCode);
        iSetDiagResult = iErrorCode;
    }
    log_i("[ABUP-Procedure]: Install End Set Diag Tp, [ret:%d]", iSetDiagResult);

    ADM_ECODE_APPLICATION_CONVERSION(ret, ADM_ECODE_INSTALL_EXIT_OTAMODE_FAILED);
    return ret;
}
static D32S ipmDimSoaVdiControlVehicleStatus(ADM_IPM_DIM_T *dim, ADM_DIM_SOA_VDI_CTRL_VEHICLE_STATUS_TYPE_E eVechielStatusType){
#ifdef ADM_IPM_TEMP
    return ADM_ECODE_SUCCESSED;
#else
    D32S errorCode = ADM_ECODE_FAILED;
    D32S modeSwitch = -1;
    ADM_APPLICATION_COMMON_CMDID_TYPE_E eTrigger = ADM_APPLICATION_COMMON_CMDID_TYPE_INVALID;
    // D32S nKeeptime = 0;//add by wj 20211209 for gac

    if(VEHICLE_CTRL_ENTER_NORMAL_OTA_MODE == eVechielStatusType || 
        VEHICLE_CTRL_EXIT_NORMAL_OTA_MODE == eVechielStatusType)
    {
        ///< usb和救援模式判断是否需要进入和退出ota模式
        eTrigger = uimSoaCmdidTypeGet();
        if (ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_USB == eTrigger || 
            ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_RESCUE == eTrigger)
        {
            errorCode = adm_cvs_getModeSwitch(&modeSwitch);
            if (ADM_IS_SUCCESS(errorCode) && 0 == modeSwitch)
            {
                log_i("tiggerType[%d] install, need not OTA mode.", eTrigger);
                ADM_DEBUG_RETURN_D32S(errorCode); 
            }
        }
        //TO-DO call ips api 获取预计安装总时长，然后加上3*60
        // nKeeptime = nKeeptime + ADM_IPM_INSTALL_KEEP_NETWORK_TIMES;
    }
//    D32S ret = adm_dim_soa_vdi_controlVehicleStatus(0/*TODO instanceId*/, dim->requestTimeout, eVechielStatusType, nKeeptime, &errorCode);
	// D32S ret = adm_dim_vdi_controlVehicleStatus(adm_soa_getVdiInstanceId(),dim->requestTimeout,eVechielStatusType,0,nKeeptime,&errorCode);
    D32S ret = adm_dim_host_setVehicleStatus(adm_soa_getHostInstanceId(),dim->requestTimeout,(D32U)eVechielStatusType,&errorCode);
    log_d("instanceId:%u, eVechielStatusType:%u, ret:%d, errorCode:%d", adm_soa_getHostInstanceId(),
        (D32U)eVechielStatusType, ret, errorCode);
	ret = (ret == ADM_ECODE_SUCCESSED) ? errorCode : ret;
    ADM_DEBUG_RETURN_D32S(ret);
#endif//ADM_IPM_TEMP
}
static D32S ipmDimSoaVdiControlVehicleStatusOTAModeEntry(ADM_IPM_DIM_T *dim){
    //TODO ?? ADM_DIM_SOA_VDI_CTRL_VEHICLE_STATUS_TYPE_ENTER_NORMAL_OTA_MODE or ADM_DIM_SOA_VDI_CTRL_VEHICLE_STATUS_TYPE_ENTER_SLIENT_OTA_MODE
    return ipmDimSoaVdiControlVehicleStatus(dim, VEHICLE_CTRL_ENTER_NORMAL_OTA_MODE);
}
static D32S ipmDimSoaVdiControlVehicleStatusOTAModeExit(ADM_IPM_DIM_T *dim){
    //TODO ?? ADM_DIM_SOA_VDI_CTRL_VEHICLE_STATUS_TYPE_EXIT_NORMAL_OTA_MODE or ADM_DIM_SOA_VDI_CTRL_VEHICLE_STATUS_TYPE_EXIT_SLIENT_OTA_MODE
    return ipmDimSoaVdiControlVehicleStatus(dim, VEHICLE_CTRL_EXIT_NORMAL_OTA_MODE);
}
//note by wj for gac, 需要新增type确定是预约还是取消预约
static D32S ipmDimSchduleWakeUpTimeSet(ADM_IPM_DIM_T *dim, const ADM_UIM_SOA_INSTALL_SCHDULE_TYPE_E eSchduleType, D64S schduleTime){
//#ifdef ADM_IPM_TEMP
//    return ADM_ECODE_SUCCESSED;
//#else
    D32S errorCode = ADM_ECODE_FAILED;
    D32S ret = adm_dim_host_setSchduleWakeUpTime(adm_soa_getHostTboxInstanceId(), dim->requestTimeout, (ADM_DIM_HOST_SCHDULE_TYPE_E)eSchduleType, schduleTime, &errorCode);
    log_d("schduleTime:%lld, errorCode:%d, ret:%d", schduleTime, errorCode, ret);
	ret = (ret == ADM_ECODE_SUCCESSED) ? errorCode : ret;
    ADM_DEBUG_RETURN_D32S(ret);
//#endif//ADM_IPM_TEMP
}

//add by wj 20211208
static D32S ipmDimSchduleWakeUpTimeGet(ADM_IPM_DIM_T *dim, D64S *schduleTime)
{
//#ifdef ADM_IPM_TEMP
//    return ADM_ECODE_SUCCESSED;
//#else
    D32S errorCode = ADM_ECODE_FAILED;
    if(dim == NULL || schduleTime == NULL)
    {
        errorCode = ADM_ECODE_INVALID_PARAM;
        //log_e("dim or schduleTime is invalid param");
        //return errorCode;
        ADM_DEBUG_RETURN_D32S(errorCode);
    }
    
    D32S ret = adm_dim_host_getSchduleWakeUpTime(adm_soa_getHostTboxInstanceId(), dim->requestTimeout, schduleTime, &errorCode);
    log_d("schduleTime:%lld, errorCode:%d, ret:%d", schduleTime, errorCode, ret);
	ret = (ret == ADM_ECODE_SUCCESSED) ? errorCode : ret;
    ADM_DEBUG_RETURN_D32S(ret);
//#endif//ADM_IPM_TEMP
}

//end

static D32S ipmDimSoaVdiPerformSpecDiagnosticTask(ADM_IPM_DIM_T *dim, ADM_DIM_SOA_VDI_SPEC_DIAGNOSTIC_TASK_TYPE_E eSpecDiagTaskType){
// #ifdef ADM_IPM_TEMP
#if 1    
    return ADM_ECODE_SUCCESSED;
#else
    D32S errorCode = ADM_ECODE_FAILED;
    D32S ret = adm_dim_vdi_performSpecDiagnosticTask(0/*TODO instanceId*/, dim->requestTimeout, eSpecDiagTaskType, NULL, &errorCode);
    log_d("eSpecDiagTaskType:%d, ret:%d, errorCode:%d", eSpecDiagTaskType, ret, errorCode);
    ADM_DEBUG_RETURN_D32S((ret == ADM_ECODE_SUCCESSED) ? errorCode : ret);
#endif//ADM_IPM_TEMP
}
static D32S ipmDimSoaVdiPerformSpecDiagnosticEnterDiagMode(ADM_IPM_DIM_T *dim){
    return ipmDimSoaVdiPerformSpecDiagnosticTask(dim, ADM_DIM_SOA_VDI_SPEC_DIAGNOSTIC_TASK_TYPE_ENTER_DIAG_MODE);
}
static D32S ipmDimSoaVdiPerformSpecDiagnosticExitDiagMode(ADM_IPM_DIM_T *dim){
    return ipmDimSoaVdiPerformSpecDiagnosticTask(dim, ADM_DIM_SOA_VDI_SPEC_DIAGNOSTIC_TASK_TYPE_EXIT_DIAG_MODE);
}
static D32S ipmDimSoaHostExecOtaTask(ADM_IPM_DIM_T *dim, ADM_DIM_SOA_HOST_OTA_TASK_TYPE_E eOtaTaskType, const D32U keeptime){
#if 1
    return ADM_ECODE_SUCCESSED;
#else
    D32S errorCode = ADM_ECODE_FAILED;
    D32S ret = adm_dim_host_execOtaTask(adm_soa_getHostInstanceId(), dim->requestTimeout, eOtaTaskType, keeptime, &errorCode);
    log_d("eOtaTaskType:%d, ret:%d, errorCode:%d", eOtaTaskType, ret, errorCode);
	ret = (ret == ADM_ECODE_SUCCESSED) ? errorCode : ret;
    ADM_DEBUG_RETURN_D32S(ret);
#endif//ADM_IPM_TEMP
}
static D32S ipmDimSoaHostExecOtaTaskStartWakeUp(ADM_IPM_DIM_T *dim){
    D32U nKeeptime = 17 * 60;//add by wj 20211209 for gac
    //TO-DO call ips api 获取预计安装总时长，然后加上3*60
    nKeeptime = nKeeptime + ADM_IPM_INSTALL_KEEP_NETWORK_TIMES;
    return ipmDimSoaHostExecOtaTask(dim, ADM_DIM_SOA_HOST_OTA_TASK_TYPE_START_WAKE_UP, nKeeptime);
}
static D32S ipmDimSoaHostExecOtaTaskStopWakeUp(ADM_IPM_DIM_T *dim){
    return ipmDimSoaHostExecOtaTask(dim, ADM_DIM_SOA_HOST_OTA_TASK_TYPE_STOP_WAKE_UP, 0);
}
static D32S ipmDimSoaHostVerifyConditionStartWakeUp(ADM_IPM_DIM_T *dim){
    D32U nKeeptime = ADM_IPM_CONDITION_KEEP_NETWORK_TIMES;//add by wj 20211209 for gac
    return ipmDimSoaHostExecOtaTask(dim, ADM_DIM_SOA_HOST_OTA_TASK_TYPE_START_WAKE_UP, nKeeptime);
}
//static D32S ipmDimSoaHostVerifyConditionStopWakeUp(ADM_IPM_DIM_T *dim){
//    return ipmDimSoaHostExecOtaTask(dim, ADM_DIM_SOA_HOST_OTA_TASK_TYPE_STOP_WAKE_UP, 0);
//}

#ifdef ADM_IPM_UNZIP_ECU_PACKAGE

/*!
 *******************************************************************************
 * 释放升级包信息列表结构内存.
 *
 * \note    释放的为连续内存
 *
 * \param[in]  num            升级包信息个数
 * \param[in]  pstPackageList 升级包信息
 * \retval     none
 *******************************************************************************
 */
static void ipmDimDeInitPackageInfoList
(
    D32S                   num, 
    ADM_DPS_PACKAGEINFO_T *pstPackageList
)
{
    D32S i = 0;

    if ((0 >= num) || (ADM_IS_NULL(pstPackageList)))
    {
        return;
    }

    for (i = 0; i < num; i++)
    {
        adm_free(pstPackageList[i].updateObj);
        adm_free(pstPackageList[i].srcVer);
        adm_free(pstPackageList[i].dstVer);
        adm_free(pstPackageList[i].fileName);
        adm_free(pstPackageList[i].fileHash);
    }
    adm_free(pstPackageList);

    return;
}

/*!
 *******************************************************************************
 * 释放升级包扩展参数列表结构内存.
 *
 * \note    释放的为连续内存
 *
 * \param[in]  num            升级包信息个数
 * \param[in]  pstPackageList 升级包信息
 * \retval     none
 *******************************************************************************
 */
/*
static void ipmDimDeInitPackageExtParamList
(
    D32S                   num, 
    ADM_COM_EXTENDPARAM_T *pstExtParamList
)
{
    D32S i = 0;

    if ((0 >= num) || (ADM_IS_NULL(pstExtParamList)))
    {
        return;
    }

    for (i = 0; i < num; i++)
    {
        adm_free(pstExtParamList[i].extKey);
        adm_free(pstExtParamList[i].extValue);
    }
    adm_free(pstExtParamList);

    return;
}
*/

/*!
 *******************************************************************************
 * 清空升级包配置信息内存.
 *
 * \note
 *
 * \param[in]  pstPkgCfgInfo 升级包配置信息
 * \retval     none
 *******************************************************************************
 */
static void ipmDimDeInitPackageCfgInfo(ADM_DPS_PACKAGECFG_T *pstPkgCfgInfo)
{
    if (ADM_IS_NULL(pstPkgCfgInfo))
    {
        return;
    }

    adm_free(pstPkgCfgInfo->packageDesc);
    adm_free(pstPkgCfgInfo->srcVer);
    adm_free(pstPkgCfgInfo->dstVer);
    ipmDimDeInitPackageInfoList(pstPkgCfgInfo->stPackageList.num, 
             pstPkgCfgInfo->stPackageList.pstPkgInfo);
    pstPkgCfgInfo->stPackageList.num = 0;
	
/* 扩展字段暂时不解析,所以也不用释放
	ipmDimDeInitPackageExtParamList(pstPkgCfgInfo->stExtParamList.num, 
             pstPkgCfgInfo->stExtParamList.pstExtParamInfo);
    pstPkgCfgInfo->stExtParamList.num = 0;
*/
    return;
}

/*!
 *******************************************************************************
 * 初始化升级包配置信息内存.
 *
 * \note
 *
 * \param[in]  pstPkgCfgInfo 升级包配置信息
 * \retval     none
 *******************************************************************************
 */
static void ipmDimInitPackageCfgInfo(ADM_DPS_PACKAGECFG_T *pstPkgCfgInfo)
{
    if (ADM_IS_NULL(pstPkgCfgInfo))
    {
        return;
    }

    pstPkgCfgInfo->packageDesc = NULL;
    pstPkgCfgInfo->srcVer = NULL;
    pstPkgCfgInfo->dstVer = NULL;
	pstPkgCfgInfo->isRollback = 0;
	pstPkgCfgInfo->stPackageList.num = 0;
	pstPkgCfgInfo->stPackageList.pstPkgInfo = NULL;
    pstPkgCfgInfo->stExtParamList.num = 0;
	pstPkgCfgInfo->stExtParamList.pstExtParamInfo = NULL;
	
    return;
}


/*!
 *******************************************************************************
 * 获取下载的升级包解压目录.
 *
 * \note 集合包的情况下会需要解压升级包
 *
 * \retval  DCHAR *
 *******************************************************************************
 */
DCHAR *ipmDimgetEcuPackageUnzipDir(const DCHAR *ecuReqId,const DCHAR *ecuSwDid)
{
    DCHAR szPkgRunDIr[ADM_COM_FILEPATH_LEN] = {0};
    DCHAR *pcCoreServiceRuningDir = NULL;

    //!< 获取系统配置的存储升级包目录
    pcCoreServiceRuningDir = adm_core_service_getRuntimeDir(ADM_CORE_SERVICE_RUNTIME_DIR_TYPE_UNZIP);
    if (ADM_IS_NULL(pcCoreServiceRuningDir))
    {
        log_e("Failed to get coreService package runingDir, err[%d]", ADM_ECODE_CORESERVICE_GETRUNDIR_FAILED);
        return NULL;    
    }
    //!< 系统配置的存储包目录继续细化下载包目录
    (void)adm_snprintf(szPkgRunDIr, sizeof(szPkgRunDIr), "%s%s_%s_%s/",
                 pcCoreServiceRuningDir, ADM_IPM_INSTALL_UNZIPDIR_SUFFIX,ecuReqId,ecuSwDid);
    log_d("Install package unzip dir[%s].", szPkgRunDIr);
    adm_free(pcCoreServiceRuningDir);

    return adm_strdup(szPkgRunDIr);
}

/*!
 *******************************************************************************
 * ipm解析ecu升级包配置文件.
 *
 * \note
 * 
 * \param[in]  filePath      升级包配置文件路径
 * \param[in]  pstPkgCfgInfo 升级包配置文件信息
 * \retval     D32S
 *******************************************************************************
 */
static D32S ipmDimParsePackageConfigFile
(
    DCHAR                *filePath, 
    ADM_DPS_PACKAGECFG_T *pstPkgCfgInfo
)
{
    D32S   iRet = ADM_ECODE_SUCCESSED;
    DCHAR *pcFileData = NULL;
    json_object *jsonRoot = NULL;
    
    if (ADM_IS_NULL(filePath) || ADM_IS_NULL(pstPkgCfgInfo)){
        log_e("Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }

    //!< 读 downloadPolicy 文件
    iRet = adm_com_readFileData(filePath, &pcFileData);
    if (ADM_IS_ERR(iRet)){
        log_e("Failed to read ecu package config file, filePath[%s], err[%d]", filePath, iRet);
        return iRet;
    }

    jsonRoot = json_tokener_parse(pcFileData);
    if (ADM_IS_NULL(jsonRoot)) {
        log_e("Failed string to json, string[%s]", pcFileData);       
        adm_free(pcFileData);
        return ADM_ECODE_STRING_TO_JSON_FAILED;
    }

    //!< 解析文件数据
    iRet = adm_dps_parsePackageConfigInfo(jsonRoot, pstPkgCfgInfo);
    if (ADM_IS_ERR(iRet)){
        log_e("Failed to parse down package config file, err[%d]", iRet);
    }
    (void)json_object_put(jsonRoot);

    adm_free(pcFileData);

    return iRet;
}

/*!
 *******************************************************************************
 * ipm校验ecu升级包解压后的各个文件.
 *
 * \note
 * 
 * \param[in]  unzipDir      ecu升级包解压后的目录
 * \param[in]  pstPkgCfgInfo 升级包配置文件信息
 * \retval     D32S
 *******************************************************************************
 */
static D32S ipmDimVerifyPackageConfigFile
(
    DCHAR                *unzipDir, 
    ADM_DPS_PACKAGECFG_T *pstPkgCfgInfo
)
{
    D32S i = 0;
	DCHAR packageFilePath[ADM_COM_LEN_256] = {0};
	DCHAR fileHash[256] = {0};
	DBOOL isExist = FALSE;
    D32S  iRet = ADM_ECODE_SUCCESSED;
    struct stat fileStat = {0};
    
    if (ADM_IS_NULL(unzipDir) || ADM_IS_NULL(pstPkgCfgInfo)){
        log_e("Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }

	for (i = 0; i < pstPkgCfgInfo->stPackageList.num; i++){
		//!< 校验配置文件列表中的升级文件是否存在
        (void)adm_snprintf(packageFilePath, sizeof(packageFilePath), "%s%s", unzipDir, pstPkgCfgInfo->stPackageList.pstPkgInfo[i].fileName);
        isExist = adm_com_isFileExist(packageFilePath);
        if (FALSE == isExist)
        {
            iRet = ADM_ECODE_FILE_NOT_EXIST;
            log_e("Failed to find file[%s]", packageFilePath);         
            break;
        }

        iRet = stat(packageFilePath, &fileStat);
	    if (ADM_ECODE_SUCCESSED != iRet){			
		    iRet = ADM_ECODE_GET_FILESIZE_FILED;
            log_e( "filePath:%s, get file size failed", iRet); 
            break;
        } 
        if (fileStat.st_size != pstPkgCfgInfo->stPackageList.pstPkgInfo[i].fileSize){
			log_e("File really size[%lld] , assume size[%lld], filePath[%s], err[%d]", 
							(D64S)fileStat.st_size,pstPkgCfgInfo->stPackageList.pstPkgInfo[i].fileSize, 
							packageFilePath, ADM_ECODE_DOWN_VERIFY_SIZE_FAIL);
			iRet =  ADM_ECODE_DOWN_VERIFY_SIZE_FAIL;
			break;
		}

        adm_com_sha256File(packageFilePath, fileHash, 256/*sizeof(fileHash)/sizeof(fileHash[0])*/);
        if(adm_strcmp(pstPkgCfgInfo->stPackageList.pstPkgInfo[i].fileHash, fileHash) != 0){
            log_e("Checkhash failed, expect[%s], calc[%s], err[%d]", 
                pstPkgCfgInfo->stPackageList.pstPkgInfo[i].fileHash, fileHash, ADM_ECODE_DOWN_VERIFY_HASH_FAILED);
            iRet =  ADM_ECODE_DOWN_VERIFY_HASH_FAILED;
			break;
        }

		
    }
		
    return iRet;
}

/*!
 *******************************************************************************
 * ipm校验ecu升级包解压后的各个文件.
 *
 * \note 如果程序执行出错，在函数返回前释放所有申请的内存
 * 
 * \param[in]  node ecu升级节点信息
 * \param[in]  unzipDir      ecu升级包解压后的目录
 * \param[in]  pstPkgCfgInfo 升级包配置文件信息
 * \retval     D32S
 *******************************************************************************
 */
static D32S ipmDimGetPackageConfigUnzipPath
(
    const ADM_IPM_ECU_INFO_NODE_T *node, 
    ADM_DPS_PACKAGECFG_T *pstPkgCfgInfo,
    DCHAR                **unzipDir 
)
{
//    D32S i = 0;
//	DCHAR packageFilePath[ADM_COM_LEN_256] = {0};
	DCHAR ecuCfgFilePath[ADM_COM_LEN_256] = {0};
	DCHAR fileHash[256] = {0};
	DBOOL isExist = FALSE;
    D32S  iRet = ADM_ECODE_SUCCESSED;
    struct stat fileStat = {0};
	DCHAR *ipmPkgUnzipDir = NULL;
   
    if (ADM_IS_NULL(unzipDir) || ADM_IS_NULL(pstPkgCfgInfo) || ADM_IS_NULL(node)){
        log_e("Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }

    D64U uiStartTime = (D64U)time(NULL);
    
	//!< 校验pis中的升级包是否正确
	isExist = adm_com_isFileExist(node->pisData.imagePackage.path);
    if (FALSE == isExist){
        iRet = ADM_ECODE_FILE_NOT_EXIST;
        log_e("Failed to find file[%s]", node->pisData.imagePackage.path);         
        return iRet;
    }	
	iRet = stat(node->pisData.imagePackage.path, &fileStat);
	if (ADM_ECODE_SUCCESSED != iRet){			
		iRet = ADM_ECODE_GET_FILESIZE_FILED;
		log_e( "filePath:%s, get file size failed", iRet); 
		return iRet;
	} 
	if (fileStat.st_size != node->pisData.imagePackage.size){
		log_e("File really size[%lld] , assume size[%lld], filePath[%s], err[%d]", 
			(D64S)fileStat.st_size,node->pisData.imagePackage.size, 
			node->pisData.imagePackage.path, ADM_ECODE_DOWN_VERIFY_SIZE_FAIL);
		iRet =	ADM_ECODE_DOWN_VERIFY_SIZE_FAIL;
		return iRet;
	}	
	adm_com_sha256File(node->pisData.imagePackage.path, fileHash, 256/*sizeof(fileHash)/sizeof(fileHash[0])*/);
	if(adm_strcmp(node->pisData.imagePackage.hash, fileHash) != 0){
		log_e("Checkhash failed, expect[%s], calc[%s], err[%d]", 
		node->pisData.imagePackage.hash, fileHash, ADM_ECODE_DOWN_VERIFY_HASH_FAILED);
		iRet =	ADM_ECODE_DOWN_VERIFY_HASH_FAILED;
		return iRet;
	}
 
    //!< 获取下载升级包的解压路径
    ipmPkgUnzipDir = ipmDimgetEcuPackageUnzipDir(node->ipsEcu.ecuReqId,node->ipsEcu.ecuSwDid);
    if (ADM_IS_NULL(ipmPkgUnzipDir)){
        iRet = ADM_ECODE_CORESERVICE_GETRUNDIR_FAILED;
        log_e("Failed to get ecu package unzip dir, err[%d]", iRet);
        return iRet;
    }
    log_d("ecu package unzip dir[%s]", ipmPkgUnzipDir);

    isExist = adm_com_isDirExist(ipmPkgUnzipDir);
    if (TRUE == isExist){
        (void)adm_fremove(ipmPkgUnzipDir);
    }
    
	iRet = adm_com_createDir(ipmPkgUnzipDir);
	if (ADM_IS_ERR(iRet)){
		log_e("Failed to create dir[%s], err[%d]", ipmPkgUnzipDir, iRet);
		adm_free(ipmPkgUnzipDir);
		return iRet;
	}
	log_d("create dir[%s] ok.", ipmPkgUnzipDir);

	// TODO: 1. 解压 --- 需替换系统解压接口
    iRet = adm_com_unZipFile(node->pisData.imagePackage.path, ipmPkgUnzipDir);
    if (ADM_IS_ERR(iRet)){
            
        log_e("Failed to unZipFile[%s], unZipDir[%s], err[%d]", node->pisData.imagePackage.path, ipmPkgUnzipDir, iRet);            
        adm_free(ipmPkgUnzipDir);
		return iRet;
    }
    log_d("Unzip file[%s] to[%s] ok.", node->pisData.imagePackage.path, ipmPkgUnzipDir);


    IPM_STAT_EVENT_T statEvent = { 0 };
    statEvent.ecuReqId = node->ipsEcu.ecuReqId;
    statEvent.ecuSwDid = node->ipsEcu.ecuSwDid;
    statEvent.serviceType = node->ecsEcu->serviceType;
    statEvent.fileSize = node->pisData.imagePackage.size;
    statEvent.eventId = (D32S)STAT_EVENT_UNZIP_PACKAGE;
    statEvent.startTime = uiStartTime;
    statEvent.endTime = (D64U)time(NULL);
    statEvent.progress = 100;
    ipmStatEventReport(&statEvent);     // 安装包解压时间已统计在刷写时间内

    uiStartTime = (D64U)time(NULL);

	//!< 校验通用配置文件是否存在
    (void)adm_snprintf(ecuCfgFilePath, sizeof(ecuCfgFilePath), "%s%s", ipmPkgUnzipDir, ADM_IPM_ECU_PACKAGE_CFGFILE_NAME);
    isExist = adm_com_isFileExist(ecuCfgFilePath);
    if (FALSE == isExist){
        iRet = ADM_ECODE_INSTALL_PACKAGE_ABNORMAL;
        log_e("Failed to find file[%s]", ecuCfgFilePath);
        adm_free(ipmPkgUnzipDir);
		return iRet;
    }

	//!< 2. 解析配置文件    
    iRet = ipmDimParsePackageConfigFile(ecuCfgFilePath, pstPkgCfgInfo);
    if (ADM_IS_ERR(iRet)){
        log_e("Failed to parse down package config file[%s], err[%d]", ecuCfgFilePath, iRet);
		iRet = ADM_ECODE_INSTALL_PACKAGE_ABNORMAL;
        adm_free(ipmPkgUnzipDir);
		return iRet; 
    }
    log_i("ecu combine package config file parse ok.");
	
	//!< 3. 检查配置文件中ecu文件信息是否与解压目录下的ecu文件一致
	iRet = ipmDimVerifyPackageConfigFile(ipmPkgUnzipDir, pstPkgCfgInfo);
	if (ADM_IS_ERR(iRet)){
	    log_e("Failed to check ecu combine package config.son ecu file info, err[%d]", iRet);
		iRet = ADM_ECODE_INSTALL_PACKAGE_ABNORMAL;
        adm_free(ipmPkgUnzipDir);
		ipmDimDeInitPackageCfgInfo(pstPkgCfgInfo);
		return iRet; 
	}
	log_i("ecu combine package config file check ok.");

    (void)stat(ecuCfgFilePath, &fileStat);
    statEvent.fileSize = fileStat.st_size;
    statEvent.eventId = (D32S)STAT_EVENT_PARSE_PAKAGE_CFG;
    statEvent.startTime = uiStartTime;
    statEvent.endTime = (D64U)time(NULL);
    statEvent.progress = 100;
    ipmStatEventReport(&statEvent);     // 配置文件解析时间已统计在刷写时间内

    *unzipDir = ipmPkgUnzipDir;
    return iRet;
}
#endif

/* ADM_DIM_SOA_VDI_REFLASH_INFO_ARRAY_T */
static D32S dimSoaVdiReflashInfoArrayInit(ADM_DIM_SOA_VDI_REFLASH_INFO_ARRAY_T *reflashInfoList, const ADM_IPM_ECU_INFO_NODE_T *node){
    D32S ret = ADM_ECODE_FAILED;
    reflashInfoList->reflashInfoArray = NULL;
    reflashInfoList->reflashInfoArrayLength = 0;

    reflashInfoList->reflashInfoArray = adm_malloc(1UL *sizeof(*(reflashInfoList->reflashInfoArray)));
    if(reflashInfoList->reflashInfoArray != NULL){
        reflashInfoList->reflashInfoArrayLength = 1;
    	reflashInfoList->reflashInfoArray[0] = adm_malloc(sizeof(*(reflashInfoList->reflashInfoArray[0])));
    	if(reflashInfoList->reflashInfoArray[0] != NULL){
    	    return (dimSoaVdiReflashInfoInit(reflashInfoList->reflashInfoArray[0], node));
    	}
		else{
		    ret = ADM_ECODE_NO_MEMORY;
            log_e("malloc failed :ret:%d", ret);
	    }
    }
	else{
		ret = ADM_ECODE_NO_MEMORY;
        log_e("malloc failed :ret:%d", ret);
	}
	return ret;
}
static void dimSoaVdiReflashInfoArrayDeInit(ADM_DIM_SOA_VDI_REFLASH_INFO_ARRAY_T *reflashInfoList){
    if(NULL != reflashInfoList->reflashInfoArray){
	for(D16U i=0; i<reflashInfoList->reflashInfoArrayLength; ++i){
	    if(NULL != reflashInfoList->reflashInfoArray[i]){
		dimSoaVdiReflashInfoDeInit(reflashInfoList->reflashInfoArray[i]);
		adm_free(reflashInfoList->reflashInfoArray[i]);
	    }
	}
	adm_free(reflashInfoList->reflashInfoArray);
    }
}
#ifndef ADM_IPM_UNZIP_ECU_PACKAGE
/* ADM_DIM_SOA_VDI_IMAGE_INFO_T */
static void dimSoaVdiImageInfoInit(ADM_DIM_SOA_VDI_IMAGE_INFO_T *imageInfo, const ADM_PIS_PACKAGE_DATA_T *pisData){
    imageInfo->imagePath = NULL;
    imageInfo->imageHash = NULL;
    imageInfo->imageSize = 0;
    if(pisData->imagePackage.path != NULL){imageInfo->imagePath = adm_strdup(pisData->imagePackage.path);}
    if(pisData->imagePackage.hash != NULL){imageInfo->imageHash = adm_strdup(pisData->imagePackage.hash);}
    imageInfo->imageSize = pisData->imagePackage.size;
}
#endif
static void dimSoaVdiImageInfoDeInit(ADM_DIM_SOA_VDI_IMAGE_INFO_T *imageInfo){
    adm_free(imageInfo->imagePath);
    adm_free(imageInfo->imageHash);
}

/* ADM_DIM_SOA_VDI_IMAGE_INFO_ARRAY_T */
#ifndef ADM_IPM_UNZIP_ECU_PACKAGE
static D32S dimSoaVdiImageInfoArrayInit(ADM_DIM_SOA_VDI_IMAGE_INFO_ARRAY_T *imageInfoList, const ADM_PIS_PACKAGE_DATA_T *pisData){
    D32S ret = ADM_ECODE_FAILED;
	
    imageInfoList->imageInfoArray = NULL;
    imageInfoList->imageInfoArrayLength = 0;
    imageInfoList->imageInfoArray = adm_malloc(sizeof(*(imageInfoList->imageInfoArray)));
    if(imageInfoList->imageInfoArray){
	    imageInfoList->imageInfoArrayLength = 1;
	    imageInfoList->imageInfoArray[0] = adm_malloc(sizeof(*(imageInfoList->imageInfoArray[0])));
	    if(imageInfoList->imageInfoArray[0]){
	         dimSoaVdiImageInfoInit(imageInfoList->imageInfoArray[0], pisData);
			 ret = ADM_ECODE_SUCCESSED;
	    }
		else{
		     ret = ADM_ECODE_NO_MEMORY;
             log_e("malloc failed :ret:%d", ret);
	    }
    }
	else{
		ret = ADM_ECODE_NO_MEMORY;
        log_e("malloc failed :ret:%d", ret);
	}
	return ret;
}
#else
static D32S dimSoaVdiImageInfoArrayInit(ADM_DIM_SOA_VDI_IMAGE_INFO_ARRAY_T *imageInfoList, const ADM_IPM_ECU_INFO_NODE_T *node){
    D32S ret = ADM_ECODE_FAILED;
	ADM_DPS_PACKAGECFG_T  stPkgCfgInfo;  //!< 集合包配置文件信息
	DCHAR packageFilePath[ADM_COM_LEN_256] = {0};
	DCHAR                *unzipDirPath = NULL; 
	D16U i = 0;

	ipmDimInitPackageCfgInfo(&stPkgCfgInfo);
	ret = ipmDimGetPackageConfigUnzipPath(node, &stPkgCfgInfo,&unzipDirPath);

	if(ADM_ECODE_SUCCESSED == ret){
        imageInfoList->imageInfoArray = NULL;
        imageInfoList->imageInfoArrayLength = 0;
        imageInfoList->imageInfoArray = adm_malloc((D32U)stPkgCfgInfo.stPackageList.num * sizeof(*(imageInfoList->imageInfoArray)));
        if(NULL != imageInfoList->imageInfoArray){
	        imageInfoList->imageInfoArrayLength = (D16U)stPkgCfgInfo.stPackageList.num;
		    for(i=0; i<imageInfoList->imageInfoArrayLength; i++){
                imageInfoList->imageInfoArray[i] = NULL;
		    }

		    for(i=0; i<imageInfoList->imageInfoArrayLength; i++){
	            imageInfoList->imageInfoArray[i] = adm_malloc(sizeof(*(imageInfoList->imageInfoArray[i])));
	            if(NULL != imageInfoList->imageInfoArray[i]){			
		            adm_memzero(packageFilePath, ADM_COM_LEN_256);
                    (void)adm_snprintf(packageFilePath, sizeof(packageFilePath), "%s%s", unzipDirPath, stPkgCfgInfo.stPackageList.pstPkgInfo[i].fileName);
//	                dimSoaVdiImageInfoInit(imageInfoList->imageInfoArray[0], pisData);
                    ADM_DIM_SOA_VDI_IMAGE_INFO_T *imageInfo = imageInfoList->imageInfoArray[i];
					imageInfo->imagePath = adm_strdup(packageFilePath);
                    imageInfo->imageHash = adm_strdup(stPkgCfgInfo.stPackageList.pstPkgInfo[i].fileHash);
                    imageInfo->imageSize = stPkgCfgInfo.stPackageList.pstPkgInfo[i].fileSize;
			 
	            }
		        else{
		            ret = ADM_ECODE_NO_MEMORY;
                    log_e("malloc failed :ret:%d", ret);
				    break;
	            }
			}
        }
	    else{
		    ret = ADM_ECODE_NO_MEMORY;
            log_e("malloc failed :ret:%d", ret);
	    }

		//release memory
		adm_free(unzipDirPath);
		ipmDimDeInitPackageCfgInfo(&stPkgCfgInfo);
    }
	return ret;
}

#endif


static void dimSoaVdiImageInfoArrayDeInit(ADM_DIM_SOA_VDI_IMAGE_INFO_ARRAY_T *imageInfoList){
    if(NULL != imageInfoList->imageInfoArray){
	for(D16U i=0; i<imageInfoList->imageInfoArrayLength; ++i){
	    if(NULL != imageInfoList->imageInfoArray[i]){
		dimSoaVdiImageInfoDeInit(imageInfoList->imageInfoArray[i]);
		adm_free(imageInfoList->imageInfoArray[i]);
	    }
	}
	adm_free(imageInfoList->imageInfoArray);
    }
}

static ADM_SYSI4VDI_SA_LIB_INFO_ARRAY_T* ipmGetEcuFlashSaLibs(ADM_ECS_GENERALCONFIG_T* pEcuGeneralCfg, ADM_ECS_SALIBNUMBERLIST_T* pSrcSaLibList)
{
    ADM_SYSI4VDI_SA_LIB_INFO_ARRAY_T* pVdiSaLibList = NULL;
    ADM_SYSI4VDI_SA_LIB_INFO_T* pVdiSaLib = NULL;
    ADM_ECS_SALIBINFO_T* pSaLibInfo = NULL;
    ADM_ECS_SALIBLIST_T* pSaLibList = &pEcuGeneralCfg->stSaLibList;

    pVdiSaLibList = adm_malloc(sizeof(ADM_SYSI4VDI_SA_LIB_INFO_ARRAY_T));
    if (NULL == pVdiSaLibList)
    {
        return NULL;
    }

    pVdiSaLibList->saLibArray = adm_malloc(sizeof(ADM_SYSI4VDI_SA_LIB_INFO_T*) * (DSIZE)pSrcSaLibList->num);
    if (NULL == pVdiSaLibList->saLibArray)
    {
        adm_free(pVdiSaLibList);
        return NULL;
    }

    pVdiSaLibList->saLibArrayNum = 0;
    for (D32S i = 0; i < pSrcSaLibList->num; i++)
    {
        D32S iSaLibNumber = pSrcSaLibList[i].psaLibNumber[i];
        for (D32S i = 0; i < pSaLibList->num; i++)
        {
            pSaLibInfo = &pSaLibList->pstSaLibInfo[i];
            if (iSaLibNumber == pSaLibInfo->saLibNumber)
            {
                pVdiSaLib = adm_malloc(sizeof(ADM_SYSI4VDI_SA_LIB_INFO_T));
                if (NULL != pVdiSaLib)
                {
                    pVdiSaLib->saLibPath = adm_strdup(pSaLibInfo->stSaLibFile.filePath);
                    pVdiSaLibList->saLibArray[pVdiSaLibList->saLibArrayNum++] = pVdiSaLib;
                }
                break;
            }
        }
    }

    return pVdiSaLibList;
}

static DCHAR* ipmGetEcuFlashCrcLibByNumber(ADM_ECS_GENERALCONFIG_T* pEcuGeneralCfg, D32S iCrcLibNumber)
{
    DCHAR* pCrcLib = NULL;

    ADM_ECS_CRCLIBINFO_T* pCrcLibInfo = NULL;
    ADM_ECS_CRCLIBLIST_T* pCrcLibList = &pEcuGeneralCfg->stCrcLibList;
    for (D32S i = 0; i < pCrcLibList->num; i++)
    {
        pCrcLibInfo = &pCrcLibList->pstCrcLibInfo[i];
        if (iCrcLibNumber == pCrcLibInfo->crcLibNumber)
        {
            pCrcLib = adm_strdup(pCrcLibInfo->stCrcLibFile.filePath);
            break;
        }
    }
 
    return pCrcLib;
}

/* ADM_DIM_SOA_VDI_REFLASH_INFO_T */
static D32S dimSoaVdiReflashInfoInit(ADM_DIM_SOA_VDI_REFLASH_INFO_T *reflashInfo, const ADM_IPM_ECU_INFO_NODE_T *node){
    D32S ret = ADM_ECODE_FAILED;
    D32S iActiveOrder = -1;
    ADM_ECS_GENERALCONFIG_T* pEcuGeneralCfg = NULL;
    ADM_ECS_RSCONFIGINFO_T stRsCfgInfo = { 0 };

    reflashInfo->pstEcuIdenInfo = NULL;
	/* temp data for debug */
    reflashInfo->srcSoftVersion = adm_strdup("/opt/fota");
	/* temp data for debug */
    reflashInfo->dstSoftVersion = adm_strdup("/opt/fota");
    reflashInfo->eBusType = (ADM_DIM_SOA_VDI_BUS_TYPE_E)node->ecsEcu->busType;
    reflashInfo->isCGWRouted = (D16U)node->ecsEcu->isCGWRouted;
    reflashInfo->eChannelType = (ADM_DIM_SOA_PIN_CHANNEL_TYPE_E)node->ecsEcu->channelType;
    reflashInfo->saMask = adm_strdup(node->ecsEcu->saMask);

    reflashInfo->flashProgressType = 0; ///< 默认传统部件
    (DVOID)adm_ecs_getEcuFlashProgressType(node->ecsEcu->ecuReqId, 
                                node->ecsEcu->ecuSwDid, &reflashInfo->flashProgressType);
    reflashInfo->crcMask = (D32U)node->ecsEcu->crcMask;
    reflashInfo->flashScriptPath = NULL;
    reflashInfo->saLibList = NULL;
    reflashInfo->crcLibPath = NULL;
    reflashInfo->pstSaMaskList = NULL;
    reflashInfo->isBlockPartion = DFALSE;
    if (ADM_ECODE_SUCCESSED == adm_ecs_getConfigInfo(&stRsCfgInfo))
    {
        pEcuGeneralCfg = &stRsCfgInfo.stGeneralCfg;
        reflashInfo->flashScriptPath = ipmGetEcuFlashScriptByNumber(pEcuGeneralCfg, node->ecsEcu->flashScriptNumber);
        // reflashInfo->saLibList = ipmGetEcuFlashSaLibs(pEcuGeneralCfg, &node->ecsEcu->saLibNumberList);
        // reflashInfo->crcLibPath = ipmGetEcuFlashCrcLibByNumber(pEcuGeneralCfg, node->ecsEcu->crcLibNumber);
        adm_ecs_cleanRSConfigInfo(&stRsCfgInfo);
    }

    reflashInfo->imageInfoList = NULL;

    reflashInfo->pstEcuIdenInfo = adm_malloc(sizeof(*(reflashInfo->pstEcuIdenInfo)));
    if(reflashInfo->pstEcuIdenInfo != NULL){
	    dimSoaVdiEcuIdenInfoInit(reflashInfo->pstEcuIdenInfo, node->ecsEcu);
    }
	else{
		ret = ADM_ECODE_NO_MEMORY;
        log_e("malloc failed :ret:%d", ret);
	}
	
    reflashInfo->imageInfoList = adm_malloc(sizeof(*(reflashInfo->imageInfoList)));
    if(reflashInfo->imageInfoList != NULL){
#ifndef ADM_IPM_UNZIP_ECU_PACKAGE				
	    ret = dimSoaVdiImageInfoArrayInit(reflashInfo->imageInfoList, &node->pisData);
#else
        ret = dimSoaVdiImageInfoArrayInit(reflashInfo->imageInfoList, node);
#endif
    }
	else{
		ret = ADM_ECODE_NO_MEMORY;
        log_e("malloc failed :ret:%d", ret);
	}

    reflashInfo->pstSaMaskList = (ADM_DIM_SOA_SA_MASK_INFO_T*)adm_malloc(sizeof(ADM_DIM_SOA_SA_MASK_INFO_T));
    adm_ipm_initSaMask(node->ecsEcu->ecuReqId, node->ecsEcu->ecuSwDid, &reflashInfo->pstSaMaskList);

    iActiveOrder = adm_ips_ecuActiveOrderGet(node->ecsEcu->ecuReqId, node->ecsEcu->ecuSwDid);
    if (-1 != iActiveOrder)
    {
        reflashInfo->isBlockPartion = DTRUE;        ///< 零件有激活顺序,则代表阻止切区
    }
    else 
    {
        reflashInfo->isBlockPartion = DFALSE;
    }

	return ret;
}
static void dimSoaVdiReflashInfoDeInit(ADM_DIM_SOA_VDI_REFLASH_INFO_T *reflashInfo){
    if(NULL != reflashInfo->pstEcuIdenInfo){
	dimSoaVdiEcuIdenInfoDeInit(reflashInfo->pstEcuIdenInfo);
	adm_free(reflashInfo->pstEcuIdenInfo);
    }
    adm_free(reflashInfo->srcSoftVersion);
    adm_free(reflashInfo->dstSoftVersion);
    adm_free(reflashInfo->saMask);
    adm_free(reflashInfo->flashScriptPath);
    if(NULL != reflashInfo->saLibList){
	sysi4vdiSaLibInfoArrayDeInit(reflashInfo->saLibList);
	adm_free(reflashInfo->saLibList);
    }
    if(NULL != reflashInfo->imageInfoList){
	dimSoaVdiImageInfoArrayDeInit(reflashInfo->imageInfoList);
	adm_free(reflashInfo->imageInfoList);
    }
    if(NULL != reflashInfo->pstSaMaskList){
	adm_ipm_deInitSaMask(&reflashInfo->pstSaMaskList);
	adm_free(reflashInfo->pstSaMaskList);
    }

    return;
}

/* ADM_SYSI4VDI_SA_LIB_INFO_ARRAY_T */
/*
static void sysi4vdiSaLibInfoArrayInit(ADM_SYSI4VDI_SA_LIB_INFO_ARRAY_T *saLibList){
    saLibList->saLibArray = NULL;
    saLibList->saLibArrayNum = 0;
}
*/
static void sysi4vdiSaLibInfoArrayDeInit(ADM_SYSI4VDI_SA_LIB_INFO_ARRAY_T *saLibList){
    if(NULL != saLibList->saLibArray){
	for(D16U i=0; i<saLibList->saLibArrayNum; ++i){
	    if(NULL != saLibList->saLibArray[i]){
		sysi4vdiSaLibInfoDeInit(saLibList->saLibArray[i]);
		adm_free(saLibList->saLibArray[i]);
	    }
	}
	adm_free(saLibList->saLibArray);
    }
}

/* ADM_SYSI4VDI_SA_LIB_INFO_T */
/*
static void sysi4vdiSaLibInfoInit(ADM_SYSI4VDI_SA_LIB_INFO_T *saLib){
    saLib->saLibPath = NULL;
}
*/
static void sysi4vdiSaLibInfoDeInit(ADM_SYSI4VDI_SA_LIB_INFO_T *saLib){
    adm_free(saLib->saLibPath);
}

/* ADM_DIM_SOA_ECU_IDEN_INFO_T */
static void dimSoaVdiEcuIdenInfoInit(ADM_DIM_SOA_ECU_IDEN_INFO_T* pstEcuIdenInfo, const ADM_ECS_ECUCONFIG_T *pstEcuCfg){	
	pstEcuIdenInfo->requestId = ipmHtoi(pstEcuCfg->ecuReqId);
    pstEcuIdenInfo->responseId = ipmHtoi(pstEcuCfg->ecuResponseId);
    pstEcuIdenInfo->functionId = ipmHtoi(pstEcuCfg->ecuFunctionId);
    pstEcuIdenInfo->swDid = ipmHtoi(pstEcuCfg->ecuSwDid);
}
static void dimSoaVdiEcuIdenInfoDeInit(ADM_DIM_SOA_ECU_IDEN_INFO_T* pstEcuIdenInfo){
}

/* ADM_DIM_SOA_UCL_UPGRADE_INFO_ARRAY_T */
static D32S dimSoaUcmUpgradeInfoArrayInit(ADM_DIM_SOA_UCL_UPGRADE_INFO_ARRAY_T *upgradeInfoList, const ADM_IPM_ECU_INFO_NODE_T *node){
    D32S ret = ADM_ECODE_FAILED;
	
	upgradeInfoList->upgradeInfoArray = NULL;
    upgradeInfoList->upgradeInfoArrayLength = 0;

	upgradeInfoList->upgradeInfoArray = adm_malloc(1UL *sizeof(*(upgradeInfoList->upgradeInfoArray)));
	if(upgradeInfoList->upgradeInfoArray != NULL){
	    upgradeInfoList->upgradeInfoArrayLength = 1;
		upgradeInfoList->upgradeInfoArray[0] = adm_malloc(sizeof(*(upgradeInfoList->upgradeInfoArray[0])));
		if(upgradeInfoList->upgradeInfoArray[0] != NULL){
		    return (dimSoaUcmUpgradeInfoInit(upgradeInfoList->upgradeInfoArray[0], node));
		}
		else{
		    ret = ADM_ECODE_NO_MEMORY;
            log_e("malloc failed :ret:%d", ret);
	    }
	}
	else{
		ret = ADM_ECODE_NO_MEMORY;
        log_e("malloc failed :ret:%d", ret);
	}
	return ret;
}
static void dimSoaUcmUpgradeInfoArrayDeInit(ADM_DIM_SOA_UCL_UPGRADE_INFO_ARRAY_T *upgradeInfoList){
    if(NULL != upgradeInfoList->upgradeInfoArray){
	for(D16U i=0; i<upgradeInfoList->upgradeInfoArrayLength; ++i){
	    if(NULL != upgradeInfoList->upgradeInfoArray[i]){
		dimSoaUcmUpgradeInfoDeInit(upgradeInfoList->upgradeInfoArray[i]);
		adm_free(upgradeInfoList->upgradeInfoArray[i]);
	    }
	}
	adm_free(upgradeInfoList->upgradeInfoArray);
    }
}

/* ADM_DIM_SOA_UCL_UPGRADE_INFO_T */
static D32S dimSoaUcmUpgradeInfoInit(ADM_DIM_SOA_UCL_UPGRADE_INFO_T *upgradeInfo, const ADM_IPM_ECU_INFO_NODE_T *node){
    D32S ret = ADM_ECODE_FAILED;
	
    upgradeInfo->pstEcuIdenInfo = NULL;
    upgradeInfo->srcSoftVersion = NULL;
//#ifdef ADM_IPM_TEMP
//    upgradeInfo->dstSoftVersion = adm_strdup("123");
//#else
    upgradeInfo->dstSoftVersion = NULL;
//#endif
    upgradeInfo->imageInfoList = NULL;
    upgradeInfo->pstEcuIdenInfo = adm_malloc(sizeof(*(upgradeInfo->pstEcuIdenInfo)));
    if(upgradeInfo->pstEcuIdenInfo != NULL){
	    dimSoaUcmEcuIdenInfoInit(upgradeInfo->pstEcuIdenInfo, &node->ipsEcu);
    }
	else{
		ret = ADM_ECODE_NO_MEMORY;
        log_e("malloc failed :ret:%d", ret);
	}
	
    upgradeInfo->imageInfoList = adm_malloc(sizeof(*(upgradeInfo->imageInfoList)));
    if(upgradeInfo->imageInfoList != NULL){
#if 1//#ifndef ADM_IPM_UNZIP_ECU_PACKAGE		
	    return (dimSoaUcmImageInfoArrayInit(upgradeInfo->imageInfoList, &node->pisData));
#else
        return (dimSoaUcmImageInfoArrayInit(upgradeInfo->imageInfoList, node));
#endif
    }
	else{
		ret = ADM_ECODE_NO_MEMORY;
        log_e("malloc failed :ret:%d", ret);
	}
	return ret;
}
static void dimSoaUcmUpgradeInfoDeInit(ADM_DIM_SOA_UCL_UPGRADE_INFO_T *upgradeInfo){
    if(NULL != upgradeInfo->pstEcuIdenInfo){
	dimSoaUcmEcuIdenInfoDeInit(upgradeInfo->pstEcuIdenInfo);
	adm_free(upgradeInfo->pstEcuIdenInfo);
    }
    adm_free(upgradeInfo->srcSoftVersion);
    adm_free(upgradeInfo->dstSoftVersion);
    if(NULL != upgradeInfo->imageInfoList){
	dimSoaUcmImageInfoArrayDeInit(upgradeInfo->imageInfoList);
	adm_free(upgradeInfo->imageInfoList);
    }
}

/* ADM_DIM_SOA_ECU_IDEN_INFO_T */
static void dimSoaUcmEcuIdenInfoInit(ADM_DIM_SOA_ECU_IDEN_INFO_T *pstEcuIdenInfo, const ADM_IPS_INSTALL_ECUINFO_T *ecu){
    pstEcuIdenInfo->requestId = 0;
    pstEcuIdenInfo->responseId = 0;
    pstEcuIdenInfo->swDid = 0;
    pstEcuIdenInfo->requestId = ipmHtoi(ecu->ecuReqId);
    pstEcuIdenInfo->swDid = ipmHtoi(ecu->ecuSwDid);
}
static void dimSoaUcmEcuIdenInfoDeInit(ADM_DIM_SOA_ECU_IDEN_INFO_T *pstEcuIdenInfo){
}

/* ADM_DIM_SOA_UCL_IMAGE_INFO_ARRAY_T */
#if 1//#ifndef ADM_IPM_UNZIP_ECU_PACKAGE
static D32S dimSoaUcmImageInfoArrayInit(ADM_DIM_SOA_UCL_IMAGE_INFO_ARRAY_T *imageInfoList, const ADM_PIS_PACKAGE_DATA_T *pisData){
    D32S ret = ADM_ECODE_FAILED;
	
    imageInfoList->imageInfoArray = NULL;
    imageInfoList->imageInfoArrayLength = 0;
    imageInfoList->imageInfoArray = adm_malloc(sizeof(*(imageInfoList->imageInfoArray)));
    if(NULL != imageInfoList->imageInfoArray){
	    imageInfoList->imageInfoArrayLength = 1;
	    imageInfoList->imageInfoArray[0] = adm_malloc(sizeof(*(imageInfoList->imageInfoArray[0])));
	    if(NULL != imageInfoList->imageInfoArray[0]){
            imageInfoList->imageInfoArray[0]->makePackageType = pisData->makePackageType;
            dimSoaUcmImageInfoInit(imageInfoList->imageInfoArray[0], pisData);
			ret = ADM_ECODE_SUCCESSED;
	    }
	    else{
		     ret = ADM_ECODE_NO_MEMORY;
             log_e("malloc failed :ret:%d", ret);
	    }
    }
	else{
		ret = ADM_ECODE_NO_MEMORY;
        log_e("malloc failed :ret:%d", ret);
	}
	return ret;
}
#else
static D32S dimSoaUcmImageInfoArrayInit(ADM_DIM_SOA_UCL_IMAGE_INFO_ARRAY_T *imageInfoList, const ADM_IPM_ECU_INFO_NODE_T *node){
  D32S ret = ADM_ECODE_FAILED;
	ADM_DPS_PACKAGECFG_T  stPkgCfgInfo;  //!< 集合包配置文件信息
	DCHAR packageFilePath[ADM_COM_LEN_256] = {0};
	DCHAR                *unzipDirPath = NULL; 
	D32S i = 0;

	ipmDimInitPackageCfgInfo(&stPkgCfgInfo);
	ret = ipmDimGetPackageConfigUnzipPath(node, &stPkgCfgInfo,&unzipDirPath);
	
	if(ADM_ECODE_SUCCESSED == ret){
        imageInfoList->imageInfoArray = NULL;
        imageInfoList->imageInfoArrayLength = 0;
        imageInfoList->imageInfoArray = adm_malloc(stPkgCfgInfo.stPackageList.num * sizeof(*(imageInfoList->imageInfoArray)));
        if(imageInfoList->imageInfoArray){
	        imageInfoList->imageInfoArrayLength = stPkgCfgInfo.stPackageList.num;
			
			for(i=0; i<imageInfoList->imageInfoArrayLength; i++){
                imageInfoList->imageInfoArray[i] = NULL;
			}
			
			for(i=0; i<imageInfoList->imageInfoArrayLength; i++){
                imageInfoList->imageInfoArray[i] = adm_malloc(sizeof(*(imageInfoList->imageInfoArray[i])));
				
	            if(imageInfoList->imageInfoArray[i]){
					adm_memzero(packageFilePath, ADM_COM_LEN_256);
                    (void)adm_snprintf(packageFilePath, sizeof(packageFilePath), "%s%s", unzipDirPath, stPkgCfgInfo.stPackageList.pstPkgInfo[i].fileName);
//	                dimSoaUcmImageInfoInit(imageInfoList->imageInfoArray[0], pisData);
					ADM_DIM_SOA_UCL_IMAGE_INFO_T *imageInfo = imageInfoList->imageInfoArray[i];
					imageInfo->imagePath = adm_strdup(packageFilePath);
                    imageInfo->imageHash = adm_strdup(stPkgCfgInfo.stPackageList.pstPkgInfo[i].fileHash);
                    imageInfo->imageSize = stPkgCfgInfo.stPackageList.pstPkgInfo[i].fileSize;
    
	            }
	            else{
		           ret = ADM_ECODE_NO_MEMORY;
                   log_e("malloc failed :ret:%d", ret);
				   break;
	            }
		    }
        }
	    else{
		    ret = ADM_ECODE_NO_MEMORY;
            log_e("malloc failed :ret:%d", ret);
	    }
        //release memory
		adm_free(unzipDirPath);
		ipmDimDeInitPackageCfgInfo(&stPkgCfgInfo);
	}
	return ret;
}

#endif
static void dimSoaUcmImageInfoArrayDeInit(ADM_DIM_SOA_UCL_IMAGE_INFO_ARRAY_T *imageInfoList){
    if(imageInfoList->imageInfoArray != NULL){
	for(D16U i=0; i<imageInfoList->imageInfoArrayLength; ++i){
	    if(NULL != imageInfoList->imageInfoArray[i]){
		dimSoaUcmImageInfoDeInit(imageInfoList->imageInfoArray[i]);
		adm_free(imageInfoList->imageInfoArray[i]);
	    }
	}
	adm_free(imageInfoList->imageInfoArray);
    }
}

/* ADM_DIM_SOA_UCL_IMAGE_INFO_T */
static void dimSoaUcmImageInfoInit(ADM_DIM_SOA_UCL_IMAGE_INFO_T *imageInfo, const ADM_PIS_PACKAGE_DATA_T *pisData){
    imageInfo->imagePath = NULL;
    imageInfo->imageHash = NULL;
    imageInfo->imageSize = 0;
    if(pisData->imagePackage.path != NULL){imageInfo->imagePath = adm_strdup(pisData->imagePackage.path);}
    if(pisData->imagePackage.hash != NULL){imageInfo->imageHash = adm_strdup(pisData->imagePackage.hash);}
    imageInfo->imageSize = pisData->imagePackage.size;
            //流式拼接增加字段转换
        ADM_COM_FILESCYINFO_T* fileSecurityInfo=pisData->fileSecurityInfo;
        if(fileSecurityInfo==NULL){
            return;
        }

        imageInfo->fileMetaNum=fileSecurityInfo->fileMetaNum;
        if (0 < fileSecurityInfo->fileMetaNum)
        {
            imageInfo->pstFileMetaList = (ADM_COM_FILEMETA_T *)adm_malloc((D32U)fileSecurityInfo->fileMetaNum * sizeof(ADM_COM_FILEMETA_T));
            if (!ADM_IS_NULL(imageInfo->pstFileMetaList))
            {
                for (D32S i = 0; i < imageInfo->fileMetaNum; i++)
               {
                imageInfo->pstFileMetaList[i].type = fileSecurityInfo->pstFileMetaList[i].type;
                imageInfo->pstFileMetaList[i].size = fileSecurityInfo->pstFileMetaList[i].size;
                imageInfo->pstFileMetaList[i].name = fileSecurityInfo->pstFileMetaList[i].name;
                imageInfo->pstFileMetaList[i].concatOrder = fileSecurityInfo->pstFileMetaList[i].concatOrder;       
               }
            }else{
                adm_free(imageInfo->pstFileMetaList);
            }
        }

}
static void dimSoaUcmImageInfoDeInit(ADM_DIM_SOA_UCL_IMAGE_INFO_T *imageInfo){
    adm_free(imageInfo->imagePath);
    adm_free(imageInfo->imageHash);
    adm_free(imageInfo->pstFileMetaList);
}

/* ADM_IPS_INSTALL_ECUINFO_T */
static void ipsInstallEcuInfoArrayInit(ADM_IPS_INSTALL_ECUINFO_T *ecus, D32S ecusLen){
    for(D32S i=0; i<ecusLen; ++i){
        adm_ips_installEcuInfoInit(ecus + i);
    }
}
static void ipsInstallEcuInfoArrayDeInit(ADM_IPS_INSTALL_ECUINFO_T *ecus, D32S ecusLen){
    for(D32S i=0; i<ecusLen; ++i){
        adm_ips_installEcuInfoDeInit(ecus + i);
    }
}

/* ADM_DIM_SOA_UCL_UPGRADE_RESULT_INFO_T */
static void dimSoaUclUpgradeResultInfoDel(ADM_DIM_SOA_UCL_UPGRADE_RESULT_INFO_T *pstUpgradeResultInfo){
    adm_free(pstUpgradeResultInfo->expectVal);
    adm_free(pstUpgradeResultInfo->realVal);
    adm_free(pstUpgradeResultInfo);
}

/* ADM_DIM_SOA_VDI_REFLASH_RESULT_INFO_T */
static void dimSoaVdiReflashResultInfoDel(ADM_DIM_SOA_VDI_REFLASH_RESULT_INFO_T *pstReflashResultSendInfo){
    adm_free(pstReflashResultSendInfo->expectVal);
    adm_free(pstReflashResultSendInfo->realVal);
    adm_free(pstReflashResultSendInfo);
}
#if 0
static void ocsEcuStateInfoListGetecuDids(ADM_OCS_ECU_STATE_INFO_LIST_T *ecuStateList){
    D32S iRet = ADM_ECODE_SUCCESSED;
	D32S i = 0;
	D32S j = 0;
	ADM_ECS_ECUDIDLIST_T stEcuDidList = {0};
	
    if(ecuStateList->reportEcusLen > 0){		

		for(j=0; j<ecuStateList->reportEcusLen; ++j){

		// TODO:   待完善 ecuDid didvalue 信息
        adm_memzero(&stEcuDidList, sizeof(stEcuDidList));
		iRet = adm_ecs_getOneEcuDidListInfo(ecuStateList->reportEcus[j].ecuReqId,
                                            ecuStateList->reportEcus[j].ecuSwDid,
                                            &stEcuDidList);
		if (ADM_IS_ERR(iRet))
        {          
            log_e("Failed to get general ecu dids, err[%d]", iRet);
            break ;
        }
		log_i("ecus did list num[%d]", stEcuDidList.num);
			
        ADM_OCS_ECU_DID_LIST_T* ecuDidLists = (ADM_OCS_ECU_DID_LIST_T*)adm_malloc((D32U)sizeof(ADM_OCS_ECU_DID_LIST_T));
		if (ADM_IS_NULL(ecuDidLists)){
			log_e("Failed to malloc, err[%d]", ADM_ECODE_NO_MEMORY);
			iRet = ADM_ECODE_NO_MEMORY;
		    adm_ecs_delEcuDidList(stEcuDidList.num, &stEcuDidList.pstEcuDidInfo);
			break ;
		}
		adm_memzero(ecuDidLists, sizeof(*ecuDidLists));

        ecuDidLists->ecuDidsLen = stEcuDidList.num;
        
		ecuDidLists->ecuDids = 
            (ADM_OCS_ECU_DID_T *)adm_malloc((D32U)(sizeof(ADM_OCS_ECU_DID_T)) * (D32U)ecuDidLists->ecuDidsLen);
        if (ADM_IS_NULL(ecuDidLists->ecuDids))
        {   
            adm_free(ecuDidLists);
            log_e("Failed to malloc, err[%d]", ADM_ECODE_NO_MEMORY);
            iRet = ADM_ECODE_NO_MEMORY;
			adm_ecs_delEcuDidList(stEcuDidList.num, &stEcuDidList.pstEcuDidInfo);
			break ;
        }
        adm_memzero(ecuDidLists->ecuDids, sizeof(ADM_OCS_ECU_DID_T) * ecuDidLists->ecuDidsLen);
    
        for (i = 0; i < ecuDidLists->ecuDidsLen; i++)
        {
            ecuDidLists->ecuDids[i].did = adm_strdup(stEcuDidList.pstEcuDidInfo[i].did);
        // TODO: didValue 值如何获取 ??? 
            ecuDidLists->ecuDids[i].didValue = adm_strdup(stEcuDidList.pstEcuDidInfo[i].did);
    
            ecuDidLists->ecuDids[i].end   = stEcuDidList.pstEcuDidInfo[i].end;
            ecuDidLists->ecuDids[i].start = stEcuDidList.pstEcuDidInfo[i].start;
        }

		adm_ecs_delEcuDidList(stEcuDidList.num, &stEcuDidList.pstEcuDidInfo);

//        return ecuDidLists;

		/*
		ecuDidLists->ecuDidsLen = 2;
        ecuDidLists->ecuDids = (ADM_OCS_ECU_DID_T*)adm_malloc(
            sizeof(ADM_OCS_ECU_DID_T) * ecuDidLists->ecuDidsLen);
		if(NULL == ecuDidLists->ecuDids){
			adm_free(ecuDidLists);
			return ;
		}
        log_i("adm_irm_reportEcuState set default ecudid info ");
        ecuDidLists->ecuDids[0].did = adm_strdup("0XF187");
        ecuDidLists->ecuDids[0].didValue = adm_strdup("0X01");
        ecuDidLists->ecuDids[0].start = 0;
        ecuDidLists->ecuDids[0].end = 13;

        ecuDidLists->ecuDids[1].did = adm_strdup("0XF188");
        ecuDidLists->ecuDids[1].didValue = adm_strdup("V1.1.3");
        ecuDidLists->ecuDids[1].start = 0;
        ecuDidLists->ecuDids[1].end = 13;
        */
//		for(i=0; i<ecuStateList->reportEcusLen; ++i){
		    ecuStateList->reportEcus[j].ecuDidLists = ecuDidLists;
		
	    }
    }
    
	if(iRet != ADM_ECODE_SUCCESSED)	{
        for(i=0; i<ecuStateList->reportEcusLen; ++i){
           if(NULL !=	ecuStateList->reportEcus[i].ecuDidLists){
		       ocsEcuDidListDeInit(ecuStateList->reportEcus[i].ecuDidLists);
		       adm_free(ecuStateList->reportEcus[i].ecuDidLists);
		   }
        }
	}
    
/*	
	for (i = 0; i < stEcuDidList.num; i++){
		adm_free(stEcuDidList.pstEcuDidInfo[i].did);					
	}
    adm_free(stEcuDidList.pstEcuDidInfo);
*/
}
#else
static D32S ocsEcuStateInfoListGetecuDids(ADM_OCS_ECU_STATE_INFO_LIST_T *ecuStateList){
    D32S iRet = ADM_ECODE_SUCCESSED;
	D32S i = 0;
	D32S j = 0;
//	ADM_ECS_ECUDIDLIST_T stEcuDidList = {0};
	ADM_DIM_SOA_DID_DATA_ARRAY_T *singleEcuDidList = NULL;
	ADM_EIM_GETECUINFO_T* allEcuInfoList = NULL;

	iRet = adm_eim_getEcuInfoManagerKvGet(&allEcuInfoList);
	if (ADM_IS_ERR(iRet))
    {          
        log_e("Failed to get general ecu dids, err[%d]", iRet);
        return iRet ;
    }
	
    if(ecuStateList->reportEcusLen > 0)
	{		

		for(j=0; j<ecuStateList->reportEcusLen; ++j)
		{
		    singleEcuDidList = NULL;
            for(i=0; i<allEcuInfoList->num; ++i)
        	{
                if ((0 == adm_strcasecmp(ecuStateList->reportEcus[j].ecuReqId, allEcuInfoList->ecuInfo[i]->ecuReqId)) &&
		            (0 == adm_strcasecmp(ecuStateList->reportEcus[j].ecuSwDid, allEcuInfoList->ecuInfo[i]->ecuSwDid)))
		        {
		            singleEcuDidList = allEcuInfoList->ecuInfo[i]->vdiDidListResponse;

                    ecuStateList->reportEcus[j].partNumber = adm_strdup(allEcuInfoList->ecuInfo[i]->partNumber);
                    ecuStateList->reportEcus[j].partSupplier = adm_strdup(allEcuInfoList->ecuInfo[i]->partSupplier);
                    ecuStateList->reportEcus[j].softwarePartNumber = adm_strdup(allEcuInfoList->ecuInfo[i]->softwarePartNumber);
                    ecuStateList->reportEcus[j].partHardwareNum = adm_strdup(allEcuInfoList->ecuInfo[i]->partHardwareNum);
		            break;
		        }
        	}

			if (ADM_IS_NULL(singleEcuDidList))
	        {   
	            log_e("Failed to get ecu config, err[%d]", ADM_ECODE_EIM_GETECUINFO_FAILED);
	            //iRet = ADM_ECODE_EIM_GETECUINFO_FAILED;
				continue;
	        }
            log_i("ecus did list num[%d]", singleEcuDidList->didArrayLength);

			ADM_OCS_ECU_DID_LIST_T* ecuDidLists = (ADM_OCS_ECU_DID_LIST_T*)adm_malloc(sizeof(ADM_OCS_ECU_DID_LIST_T));
			if (ADM_IS_NULL(ecuDidLists)){
				log_e("Failed to malloc, err[%d]", ADM_ECODE_NO_MEMORY);
				iRet = ADM_ECODE_NO_MEMORY;
				break ;
			}
			adm_memzero(ecuDidLists, sizeof(*ecuDidLists));

	        ecuDidLists->ecuDidsLen = (D32S)singleEcuDidList->didArrayLength;
	        
			ecuDidLists->ecuDids = 
	            (ADM_OCS_ECU_DID_T *)adm_malloc(sizeof(ADM_OCS_ECU_DID_T) * (D32U)ecuDidLists->ecuDidsLen);
	        if (ADM_IS_NULL(ecuDidLists->ecuDids))
	        {   
	            adm_free(ecuDidLists);
	            log_e("Failed to malloc, err[%d]", ADM_ECODE_NO_MEMORY);
	            iRet = ADM_ECODE_NO_MEMORY;
				break ;
	        }
	        adm_memzero(ecuDidLists->ecuDids, sizeof(ADM_OCS_ECU_DID_T) * (D32U)ecuDidLists->ecuDidsLen);

			for (i = 0; i < ecuDidLists->ecuDidsLen; i++)
	        {
	            DCHAR did[128] = {0};
                (void)adm_snprintf(did, sizeof(did)/sizeof(DCHAR), "0X%X", singleEcuDidList->didArray[i]->did);
	            ecuDidLists->ecuDids[i].did = adm_strdup(did);
				
				adm_memzero(did, sizeof(did));
				switch ((VARIABLE_TYPE_EN)singleEcuDidList->didArray[i]->didData->dataType)
                {
                case VAR_U8:
                    (void)adm_snprintf(did, sizeof(did)/sizeof(DCHAR), "%d", singleEcuDidList->didArray[i]->didData->v.value[0]);
					ecuDidLists->ecuDids[i].didValue = adm_strdup(did);
                    break;
                case VAR_U32:
                    (void)adm_snprintf(did, sizeof(did)/sizeof(DCHAR), "%d", singleEcuDidList->didArray[i]->didData->v.valueU32);
					ecuDidLists->ecuDids[i].didValue = adm_strdup(did);
                    break;
                case VAR_S32:
                    (void)adm_snprintf(did, sizeof(did)/sizeof(DCHAR), "%d", singleEcuDidList->didArray[i]->didData->v.valueS32);
					ecuDidLists->ecuDids[i].didValue = adm_strdup(did);
                    break;
                case VAR_U64:
                    (void)adm_snprintf(did, sizeof(did)/sizeof(DCHAR), "%ld", singleEcuDidList->didArray[i]->didData->v.valueU64);
					ecuDidLists->ecuDids[i].didValue = adm_strdup(did);
                    break;
				case VAR_S64:
                    (void)adm_snprintf(did, sizeof(did)/sizeof(DCHAR), "%ld", singleEcuDidList->didArray[i]->didData->v.valueS64);
					ecuDidLists->ecuDids[i].didValue = adm_strdup(did);
                    break;
                case VAR_FLOAT:
                    (void)adm_snprintf(did, sizeof(did)/sizeof(DCHAR), "%f", singleEcuDidList->didArray[i]->didData->v.valueF);
					ecuDidLists->ecuDids[i].didValue = adm_strdup(did);
                    break;
                case VAR_STRING:
                    ecuDidLists->ecuDids[i].didValue = adm_strdup((DCHAR *)singleEcuDidList->didArray[i]->didData->v.valueS);
                    break;

                default:
                    break;
                }
	    
	            ecuDidLists->ecuDids[i].start = (D32S)singleEcuDidList->didArray[i]->startByte;
	            ecuDidLists->ecuDids[i].end = (D32S)singleEcuDidList->didArray[i]->endByte;
	        }


		    ecuStateList->reportEcus[j].ecuDidLists = ecuDidLists;
		
	    }
    }
    
	if(iRet != ADM_ECODE_SUCCESSED)	{
        for(i=0; i<ecuStateList->reportEcusLen; ++i){
           if(NULL !=	ecuStateList->reportEcus[i].ecuDidLists){
		       ocsEcuDidListDeInit(ecuStateList->reportEcus[i].ecuDidLists);
		       adm_free(ecuStateList->reportEcus[i].ecuDidLists);
		   }
        }
	}
    adm_eim_deleteGetEcuInfoBody(allEcuInfoList);
	return iRet ;
    
}


#endif

/* ADM_OCS_ECU_STATE_INFO_LIST_T */
static void ocsEcuStateInfoListInit(ADM_OCS_ECU_STATE_INFO_LIST_T *ecuStateList, const ADM_IPS_PROGRESS_T *progress){
    ecuStateList->reportEcus = NULL;
    ecuStateList->reportEcusLen = 0;

    if(NULL != progress->ecus.ecus && 0 != progress->ecus.ecusLen){
	ecuStateList->reportEcus = adm_malloc((D32U)progress->ecus.ecusLen * sizeof(*(ecuStateList->reportEcus)));
	if(NULL != ecuStateList->reportEcus){
	    ecuStateList->reportEcusLen =  progress->ecus.ecusLen;
	    for(D32S i=0; i<ecuStateList->reportEcusLen; ++i){
		ocsEcuStateInfoInit(&ecuStateList->reportEcus[i], &progress->ecus.ecus[i]);
	    }
	}
    }
}
static void ocsEcuStateInfoListDeInit(ADM_OCS_ECU_STATE_INFO_LIST_T *ecuStateList){
    if(NULL != ecuStateList->reportEcus){
	for(D32S i=0; i<ecuStateList->reportEcusLen; ++i){
	    ocsEcuStateInfoDeInit(&ecuStateList->reportEcus[i]);
	}
	adm_free(ecuStateList->reportEcus);
    }
}

/* ADM_OCS_ECU_STATE_INFO_T */
static void ocsEcuStateInfoInit(ADM_OCS_ECU_STATE_INFO_T *ecuState, const ADM_IPS_ECU_PROGRESS_T *progress){
    ecuState->ecuReqId = NULL;
    ecuState->ecuSwDid = NULL;
    ecuState->ecuDidLists = NULL;
    ecuState->ecuState = NULL;
    ecuState->ecuReason = NULL;
    ecuState->ecuResultDescrip = NULL;
    ecuState->beginTime = 0;
    ecuState->endTime = 0;

    if(progress->ecuReqId != NULL){ecuState->ecuReqId = adm_strdup(progress->ecuReqId);}
    if(progress->ecuSwDid != NULL){ecuState->ecuSwDid = adm_strdup(progress->ecuSwDid);}
    const DCHAR *state = NULL;
    D32S result = ADM_ECODE_FAILED;
    D64S beginTime = 0;
    D64S endTime = 0;
    if(progress->state == ADM_IPS_ECU_PHASE_PRE_INSTALL)
    {
    	result = 0;
    	state = ADM_IRM_REPORT_SERVER_STATUS_Ecu_Install_To_Be;
    }
    else if(progress->state == ADM_IPS_ECU_PHASE_INSTALL || progress->state == ADM_IPS_ECU_PHASE_INSTALL_ROLLBACK)
    {
	    result = 0;
	    state = ADM_IRM_REPORT_SERVER_STATUS_Ecu_Install_Ing;
    }
    else if(progress->state == ADM_IPS_ECU_PHASE_POST_INSTALL)
    {
        beginTime = progress->progress[0].beginTime;
        endTime = progress->progress[0].endTime;
        result = progress->progress[0].result;
        if (adm_strlen(progress->progress[0].resultDescrip) > 0)
        {
            ecuState->ecuResultDescrip = adm_strdup(progress->progress[0].resultDescrip);
        }

        if(result == ADM_ECODE_SUCCESSED)
        {
            state = ADM_IRM_REPORT_SERVER_STATUS_Ecu_Install_Successed;
        }
        else
        {
            state = ADM_IRM_REPORT_SERVER_STATUS_Ecu_Install_Failed;
        }
    }
    else if(progress->state == ADM_IPS_ECU_PHASE_POST_INSTALL_ROLLBACK)
    {
        beginTime = progress->progress[1].beginTime;
        endTime = progress->progress[1].endTime;
        result = progress->progress[1].result;
        if (adm_strlen(progress->progress[1].resultDescrip) > 0)
        {
            ecuState->ecuResultDescrip = adm_strdup(progress->progress[0].resultDescrip);
        }

        if(progress->progress[0].result == ADM_ECODE_SUCCESSED)
        {
            if(result == ADM_ECODE_SUCCESSED)
            {
                state = ADM_IRM_REPORT_SERVER_STATUS_Ecu_Install_Successed_Rollback_Successed;
            }
            else
            {
                state = ADM_IRM_REPORT_SERVER_STATUS_Ecu_Install_Successed_Rollback_Failed;
            }
        }
        else
        {
            if(result == ADM_ECODE_SUCCESSED)
            {
                state = ADM_IRM_REPORT_SERVER_STATUS_Ecu_Install_Failed_Rollback_Successed;
            }
            else
            {
                state = ADM_IRM_REPORT_SERVER_STATUS_Ecu_Install_Failed_Rollback_Failed;
            }
        }
    }
	else
	{
        ;/*null*/
	}
    if(state != NULL){ecuState->ecuState = adm_strdup(state);}
    DCHAR ecuReason[128] = {0};
    (void)adm_snprintf(ecuReason, sizeof(ecuReason)/sizeof(DCHAR), "%d", result);
    ecuState->ecuReason = adm_strdup(ecuReason);
    ecuState->beginTime = beginTime;
    ecuState->endTime = endTime;

    return;
}
static void ocsEcuStateInfoDeInit(ADM_OCS_ECU_STATE_INFO_T *ecuState){
    adm_free(ecuState->ecuReqId);
    adm_free(ecuState->ecuSwDid);
    if(NULL !=  ecuState->ecuDidLists){
	ocsEcuDidListDeInit(ecuState->ecuDidLists);
	adm_free(ecuState->ecuDidLists);
    }
    adm_free(ecuState->ecuState);
    adm_free(ecuState->ecuReason);
    adm_free(ecuState->ecuResultDescrip);

    return;
}

/* ADM_OCS_ECU_PROGRESS_INFO_LIST_T */
static void ocsEcuProgressInfoListInit(ADM_OCS_ECU_PROGRESS_INFO_LIST_T *ecuProgressList, const ADM_IPS_PROGRESS_T *progress){
    ecuProgressList->reportEcus = NULL;
    ecuProgressList->reportEcusLen = 0;

    ecuProgressList->progressType = adm_strdup(ADM_IPM_REPORT_SERVER_DATA_ECU_PROGRESS_TYPE_INSTALL);
    ecuProgressList->iTotalProgress = progress->progress;

    if(NULL != progress->ecus.ecus && 0 != progress->ecus.ecusLen){
	ecuProgressList->reportEcus = adm_malloc((D32U)progress->ecus.ecusLen *sizeof(*(ecuProgressList->reportEcus)));
	if(NULL != ecuProgressList->reportEcus){
	    ecuProgressList->reportEcusLen = progress->ecus.ecusLen;
	    for(D32S i=0; i<ecuProgressList->reportEcusLen; ++i){
		ocsEcuProgressInfoInit(&ecuProgressList->reportEcus[i], &progress->ecus.ecus[i]);
	    }
	}
    }
}
static void ocsEcuProgressInfoListDeInit(ADM_OCS_ECU_PROGRESS_INFO_LIST_T *ecuProgressList){
    if(NULL != ecuProgressList->reportEcus){
	for(D32S i=0; i<ecuProgressList->reportEcusLen; ++i){
	    ocsEcuProgressInfoDeInit(&ecuProgressList->reportEcus[i]);
	}
	adm_free(ecuProgressList->reportEcus);
    }
}

/* ADM_OCS_ECU_PROGRESS_INFO_T */
static void ocsEcuProgressInfoInit(ADM_OCS_ECU_PROGRESS_INFO_T *ecuProgress, const ADM_IPS_ECU_PROGRESS_T *progress)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    ADM_ECS_EXTERN_ECU_INFO_T* pstExternEcuInfo = NULL;
    ADM_ECS_ECUDIDLIST_T stEcuDidList = {0};

    ecuProgress->progressType = NULL;
    ecuProgress->ecuReqId = NULL;
    ecuProgress->ecuSwDid = NULL;
    ecuProgress->progress = 0;

    ecuProgress->progressType = adm_strdup(ADM_IPM_REPORT_SERVER_DATA_ECU_PROGRESS_TYPE_INSTALL);
    ecuProgress->ecuReqId = adm_strdup(progress->ecuReqId);
    ecuProgress->ecuSwDid = adm_strdup(progress->ecuSwDid);
    D32S index = (progress->state == ADM_IPS_ECU_PHASE_INSTALL_ROLLBACK || progress->state == ADM_IPS_ECU_PHASE_POST_INSTALL_ROLLBACK) ? 1 : 0;
    ecuProgress->progress = (D32S)progress->progress[index].progress;

    /** 获取硬件版本号等信息 */
    (void)adm_memzero(&stEcuDidList, sizeof(stEcuDidList));
    pstExternEcuInfo = (ADM_ECS_EXTERN_ECU_INFO_T*)adm_malloc(sizeof(ADM_ECS_EXTERN_ECU_INFO_T));
    if (ADM_IS_NULL(pstExternEcuInfo))
    {
        return;
    }

    iRet = adm_ecs_getOneEcuDidListInfo(ecuProgress->ecuReqId,
                                        ecuProgress->ecuSwDid,
                                        &stEcuDidList,
                                        pstExternEcuInfo);
    if (ADM_IS_ERR(iRet))
    {
        log_e("Failed to get ecu dids, ecuReqid[%s], ecuSwDid[%s], err[%d]", 
                ecuProgress->ecuReqId, 
                ecuProgress->ecuSwDid, iRet);
        return;
    }
    log_i("ecus did list num[%d]", stEcuDidList.num);
    adm_ecs_delEcuDidList(stEcuDidList.num, &stEcuDidList.pstEcuDidInfo);

    ecuProgress->partNumber = adm_strdup
    (pstExternEcuInfo->partNumber);
    ecuProgress->partSupplier = adm_strdup
    (pstExternEcuInfo->partSupplier);
    ecuProgress->softwarePartNumber = adm_strdup
    (pstExternEcuInfo->softwarePartNumber);
    ecuProgress->partHardwareNum = adm_strdup
    (pstExternEcuInfo->partHardwareNum);

    adm_ecs_delExternEcuInfo(&pstExternEcuInfo);

    return;
}
static void ocsEcuProgressInfoDeInit(ADM_OCS_ECU_PROGRESS_INFO_T *ecuProgress){
    adm_free(ecuProgress->progressType);
    adm_free(ecuProgress->ecuReqId);
    adm_free(ecuProgress->ecuSwDid);
/*    if(NULL != ecuProgress->ecuDidLists){
	ocsEcuDidListDeInit(ecuProgress->ecuDidLists);
	adm_free(ecuProgress->ecuDidLists);
    }*/
}

/* ADM_OCS_ECU_DID_LIST_T */
/*
static void ocsEcuDidListInit(ADM_OCS_ECU_DID_LIST_T *ecuDidList){
    ecuDidList->ecuDids = NULL;
    ecuDidList->ecuDidsLen = 0;
}
*/
static void ocsEcuDidListDeInit(ADM_OCS_ECU_DID_LIST_T *ecuDidList){
    if(NULL != ecuDidList->ecuDids){
	for(D32S i=0; i<ecuDidList->ecuDidsLen; ++i){
	    ocsEcuDidDeInit(&ecuDidList->ecuDids[i]);
	}
	adm_free(ecuDidList->ecuDids);
    }
}

/* ADM_OCS_ECU_DID_T */
/*
static void ocsEcuDidInit(ADM_OCS_ECU_DID_T *ecuDid){
    ecuDid->did = NULL;
    ecuDid->didValue = NULL;
    ecuDid->start = 0;
    ecuDid->end = 0;
}
*/
static void ocsEcuDidDeInit(ADM_OCS_ECU_DID_T *ecuDid){
    adm_free(ecuDid->did);
    adm_free(ecuDid->didValue);
}

/* others */
static D64U ipmMonotonicTimeMSecGet(void) {
    struct timespec ts;
    (void)clock_gettime(CLOCK_MONOTONIC, &ts);
    D64S msec = ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
    return (D64U)msec;
}
static D32S ipmHtoi(const DCHAR *hex){
    D32S a = 0;
    if(NULL != hex){a = htoi(hex);}
    return a;
}
/*
static void ipmSemTimedwaitMsecs(sem_t *sem, D64S msecs){
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    D64S secs = msecs/1000;
    msecs = msecs%1000;

    D64S add = 0;
    msecs = msecs*1000*1000 + ts.tv_nsec;
    add = msecs / (1000*1000*1000);
    ts.tv_sec += (add + secs);
    ts.tv_nsec = msecs%(1000*1000*1000);

    sem_timedwait(sem, &ts);
}
static D64S ipmSequenceIdGet(){
    static D64S sequenceId = 0;
    return sequenceId++;
}
*/

D32S adm_ipm_getUpgradePolicy(ADM_IPM_POLICY_T *policy){    
    D32S ret = ADM_ECODE_FAILED;
    ADM_CVS_VERSION_POLICY_T* versionPolicy = adm_cvs_newPolicyBody();
    if(versionPolicy == NULL){
	    ret = ADM_ECODE_NO_MEMORY;
		log_e("versionPolicy ADM_ECODE_NO_MEMORY");
    }
	else{
        ret = adm_cvs_getVersionPolicy(versionPolicy);
        log_d("adm_cvs_getVersionPolicy ret:%d, sessionId:%s, taskId:%lld, strategyId:%lld, installPolicyObjectStr:%s",
            ret, ADM_STR_SAFE_OUTPUT(versionPolicy->sessionId), versionPolicy->taskId, versionPolicy->strategyId,
            ADM_STR_SAFE_OUTPUT(versionPolicy->installMember.installPolicyObjectStr));
        if (ret == ADM_ECODE_SUCCESSED) {
			if(versionPolicy->installMember.installPolicyObjectStr == NULL){
				log_e("versionPolicy installMember installPolicyObjectStr:nullptr");
				ret = ADM_ECODE_FAILED;
			}
			else{
				struct json_object *root = json_tokener_parse(versionPolicy->installMember.installPolicyObjectStr);
				if(root == NULL){
					log_w("path:%s to json failed", versionPolicy->installMember.installPolicyObjectStr);
					ret = ADM_ECODE_FAILED;
				}
				else{
					struct json_object* verifyVersionVal = json_object_object_get(root, "verifyVersion");
				    if(NULL !=  verifyVersionVal){
					    policy->verifyVersion = json_object_get_int(verifyVersionVal);
				    }
					else{
                         log_e("versionPolicy installMember installPolicyObjectStr verifyVersion:");
//						 ret = ADM_ECODE_FAILED;
					}
	
					struct json_object* upgradeAutoStartVal = json_object_object_get(root, "upgradeAutoStart");
				    if(NULL != upgradeAutoStartVal){
						policy->upgradeAutoStart = json_object_get_int64(upgradeAutoStartVal);
						log_d("upgradeAutoStart :%lld ", policy->upgradeAutoStart);
						
				    }
					else{
                         log_e("versionPolicy installMember installPolicyObjectStr upgradeAutoStartVal:nullptr");
//						 ret = ADM_ECODE_FAILED;
					}
					policy->upgradeRetryCount = json_object_get_int(json_object_object_get(root, "upgradeRetryCount"));
                    policy->rollbackRetryCount = json_object_get_int(json_object_object_get(root, "rollbackRetryCount"));
	                policy->totalTimeConsuming = json_object_get_int(json_object_object_get(root, "totalTimeConsuming"));
					ret = ADM_ECODE_SUCCESSED;
	
				}
				(void)json_object_put(root);
			}
        }
        adm_cvs_deletePolicyBody(versionPolicy);
    }
    return ret;
}


D32S adm_ipm_getUpgradeAutoStartTimstamp(D64S *nTimestamp)
{
    if(nTimestamp == NULL)
    {
        log_e("invalid param nTimestamp");
        return ADM_ECODE_INVALID_PARAM;
    }

//    ADM_IPS_POLICY_TASK_T task;
//    adm_ips_policyTaskInit(&task);
//    adm_ips_policyTaskGet(&task);
	ADM_IPM_POLICY_T policy;
    //*nTimestamp = task.upgradeAutoStart;
//    D64S upgradeAutoStart = 0;
	
	if(adm_ipm_getUpgradePolicy(&policy) != ADM_ECODE_SUCCESSED){
		log_e("get upgradeAutoStart failed ");
		return ADM_ECODE_FAILED;
	}
	

    log_i("Before change Timestamp =%lld",policy.upgradeAutoStart);
    time_t  startTime = policy.upgradeAutoStart;
//	adm_ips_policyDeInit(&policy);
    struct tm *pStart = NULL;
    struct tm *pCurrent = NULL;
    struct tm newTimeStamp;
    struct tm curTimeStamp; 
    D32S startHour,startMin,startSec;
    D64S startTimeStamp,currectTimeStamp;

    pStart = localtime(&startTime);
    startHour = pStart->tm_hour;
    startMin  = pStart->tm_min;
    startSec  = pStart->tm_sec;

    pCurrent = localtime(NULL);

    newTimeStamp.tm_year = pCurrent->tm_year + 1900;
    newTimeStamp.tm_mon  = pCurrent->tm_mon + 1;
    newTimeStamp.tm_mday = pCurrent->tm_mday;
    newTimeStamp.tm_hour = startHour;
    newTimeStamp.tm_min  = startMin;
    newTimeStamp.tm_sec  = startSec;
    newTimeStamp.tm_isdst=0;

    curTimeStamp.tm_year = pCurrent->tm_year + 1900;
    curTimeStamp.tm_mon  = pCurrent->tm_mon + 1;
    curTimeStamp.tm_mday = pCurrent->tm_mday;
    curTimeStamp.tm_hour = pCurrent->tm_hour;
    curTimeStamp.tm_min  = pCurrent->tm_min;
    curTimeStamp.tm_sec  = pCurrent->tm_sec;
    curTimeStamp.tm_isdst=0;

    startTimeStamp   = mktime(&newTimeStamp);
    currectTimeStamp = mktime(&curTimeStamp);

    if(currectTimeStamp >= startTimeStamp)
    {
        *nTimestamp = startTimeStamp + (24*3600);
    }
    else
    {
        *nTimestamp = startTimeStamp;
    }
    log_i("After change Timestamp =%d",*nTimestamp);

    return ADM_ECODE_SUCCESSED;
}


/**
 * 队列
 */
static D32S ipmQueueInit(ADM_QUEUE_T* pQueue, D32S iCapacity)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    void** pElemets = adm_malloc(sizeof(void*) * (D32U)iCapacity);
    if (NULL != pElemets)
    {
        pQueue->capacity = iCapacity;
    }
    else
    {
        pQueue->capacity = 0;
        iRet = ADM_ECODE_FAILED;
    }
    pQueue->elements = pElemets;
    pQueue->count = 0;
    pQueue->first = 0;
    pQueue->last = 0;

    return iRet;
}

static void ipmQueueDestroy(ADM_QUEUE_T* pQueue)
{
    adm_free(pQueue->elements);
}

static D32S ipmQueuePush(ADM_QUEUE_T* pQueue, void* pEle)
{
    if (pQueue->count < pQueue->capacity)
    {
        D32S iPos = pQueue->last;
        pQueue->elements[iPos++] = pEle;
        pQueue->last = iPos % pQueue->capacity;
        pQueue->count++;
        return ADM_ECODE_SUCCESSED;
    }

    return ADM_ECODE_FAILED;
}

static void* ipmQueuePop(ADM_QUEUE_T* pQueue)
{
    void* pEle = NULL;
    if (pQueue->count > 0)
    {
        D32S iPos = pQueue->first;
        pEle = pQueue->elements[iPos++];
        pQueue->first = iPos % pQueue->capacity;
        pQueue->count--;
    }

    return pEle;
}
#if 0
static D32S ipmQueueGetCount(ADM_QUEUE_T* pQueue)
{
    return pQueue->count;
}

static D32S ipmQueueIsEmpty(ADM_QUEUE_T* pQueue)
{
    return 0 == pQueue->count ? 1 : 0;
}
#endif

/**
 * 消息队列
 */
static D32S ipmMsgQueueInit(ADM_MSG_QUEUE_T* pQueue, D32S iCapacity)
{
    (void)pthread_mutex_init(&pQueue->mutex, NULL);
    return ipmQueueInit(&pQueue->queue, iCapacity);
}

static void ipmMsgQueueDestroy(ADM_MSG_QUEUE_T* pQueue)
{
    ipmQueueDestroy(&pQueue->queue);
    (void)pthread_mutex_destroy(&pQueue->mutex);
}

static D32S ipmMsgQueuePush(ADM_MSG_QUEUE_T* pQueue, ADM_MQ_MSG_T* pMsg)
{
    (void)pthread_mutex_lock(&pQueue->mutex);
    D32S iRet= ipmQueuePush(&pQueue->queue, pMsg);
    (void)pthread_mutex_unlock(&pQueue->mutex);

    return iRet;
}

static ADM_MQ_MSG_T* ipmMsgQueuePop(ADM_MSG_QUEUE_T* pQueue)
{
    (void)pthread_mutex_lock(&pQueue->mutex);
    ADM_MQ_MSG_T* pMsg = ipmQueuePop(&pQueue->queue);
    (void)pthread_mutex_unlock(&pQueue->mutex);

    return pMsg;
}


/**
 * 消息分发处理器
 */
static void ipmMsgDispatcherInit(ADM_MSG_DISPATCHER_T* pDispatcher)
{
    pDispatcher->handlerCount = 0;
}

static D32S ipmMsgDispatcherAddHandler(ADM_MSG_DISPATCHER_T* pDispatcher, D32S iMsgType, ADM_MSG_HANDLER handler)
{
    if (pDispatcher->handlerCount < (D32S)(sizeof(pDispatcher->handlers) / sizeof(pDispatcher->handlers[0]))) 
    {
        ADM_MSG_HANDLER_T* pHandler = &pDispatcher->handlers[pDispatcher->handlerCount++];
        pHandler->msgType = iMsgType;
        pHandler->handler = handler;

        return ADM_ECODE_SUCCESSED;
    }

    return ADM_ECODE_FAILED;
}

static D32S ipmMsgDispatcherHandleMsg(ADM_MSG_DISPATCHER_T* pDispatcher, D32S iMsgType, void* pMsgData, D32U uiMsgDataLen)
{
    ADM_MSG_HANDLER     pHandler = NULL;
    ADM_MSG_HANDLER_T*  pHandlers = pDispatcher->handlers;
    for (D32S i = 0; i < pDispatcher->handlerCount; i++)
    {
        if (iMsgType == pHandlers[i].msgType)
        {
            pHandler = pHandlers[i].handler;
            break;
        }
    }

    if (NULL != pHandler)
    {
        (void)pHandler(pMsgData, uiMsgDataLen);
    }
    else
    {
        log_e("not find handler of msg[%d]", iMsgType);
    }

    return ADM_ECODE_SUCCESSED;
}


//static void ipmContextMsgDispatcherInit(ADM_MSG_DISPATCHER_T* pDispatcher)
//{
//    ipmMsgDispatcherInit(pDispatcher);
//    ipmMsgDispatcherRegister(pDispatcher, IPM_MSG_ECU_FLASH_PROGRESS_NOTIFY, ipmHandleEcuFlashProgressNotify);
//    ipmMsgDispatcherRegister(pDispatcher, IPM_MSG_ECU_FLASH_RESULT_NOTIFY, ipmHandleEcuFlashResultNotify);
//}
//
//static void ipmContextMsgDispatcherDestroy(ADM_MSG_DISPATCHER_T* pDispatcher)
//{
//    ipmMsgDispatcherInit(pDispatcher);
//}


static void* ipmMsgCenterProcessMsgSync(void* argv) {
    ADM_IPM_MSG_CENTER_T* pMsgCenter = (ADM_IPM_MSG_CENTER_T*)argv;
    while (DFALSE == pMsgCenter->bIsOver)
    {
        ipmMsgCenterProcessMsg(pMsgCenter);
        (void)usleep(20 * 1000);
    }

    (void)sem_post(&pMsgCenter->sem);

    return NULL;
}

/**
  消息中心 
 */
static void ipmMsgCenterInit(ADM_IPM_MSG_CENTER_T* pMsgCenter)
{
    ADM_MSG_DISPATCHER_T* pDispatcher = &pMsgCenter->msgDispatcher;

    ipmMsgDispatcherInit(pDispatcher);
    (void)ipmMsgDispatcherAddHandler(pDispatcher, IPM_MSG_ECU_FLASH_PROGRESS_NOTIFY, ipmHandleEcuFlashProgressNotify);
    (void)ipmMsgDispatcherAddHandler(pDispatcher, IPM_MSG_ECU_FLASH_RESULT_NOTIFY, ipmHandleEcuFlashResultNotify);
    (void)ipmMsgDispatcherAddHandler(pDispatcher, IPM_MSG_ICC_FLASH_PROGRESS_NOTIFY, ipmHandleIccFlashProgressNotify);
    (void)ipmMsgDispatcherAddHandler(pDispatcher, IPM_MSG_ICC_FLASH_RESULT_NOTIFY, ipmHandleIccFlashResultNotify);

    if (ADM_ECODE_SUCCESSED != ipmMsgQueueInit(&pMsgCenter->msgQueue, 1024))
    {
        log_e("init msg center failed");
    }

    (void)sem_init(&pMsgCenter->sem, 0, 1);
    pMsgCenter->bIsOver = DFALSE;
    pthread_t pth;
    if (pthread_create(&pth, NULL, ipmMsgCenterProcessMsgSync, (void*)pMsgCenter) == 0) {
        (void)pthread_detach(pth);
    }
}

static void ipmMsgCenterDestroy(ADM_IPM_MSG_CENTER_T* pMsgCenter)
{
    pMsgCenter->bIsOver = DTRUE;
    (void)sem_wait(&pMsgCenter->sem);
    (void)sem_destroy(&pMsgCenter->sem);
    ipmMsgQueueDestroy(&pMsgCenter->msgQueue);
}

static D32S ipmMsgCenterSendMsg(D32S iMsgType, void* pMsgData, D32U uiMsgDataLen)
{
    ADM_MQ_MSG_T* pMsg = adm_malloc((DSIZE)(sizeof(ADM_MQ_MSG_T) - 1UL) + (DSIZE)uiMsgDataLen);
    if (NULL == pMsg)
    {
        log_e("set ecu flash progress failed because of alloc memory failed");
        return ADM_ECODE_FAILED;
    }

    pMsg->msgType = iMsgType;
    pMsg->msgDataLen = uiMsgDataLen;
    (void)adm_memcpy(pMsg->msgData, pMsgData, uiMsgDataLen, (DSIZE)uiMsgDataLen);

    D32S iRet = ipmMsgQueuePush(&m_ipmContext.msgCenter.msgQueue, pMsg);
    if (ADM_ECODE_SUCCESSED != iRet)
    {
        log_e("add msg into msg queue failed, queue capacity[%d] count[%d]", 
               m_ipmContext.msgCenter.msgQueue.queue.capacity, m_ipmContext.msgCenter.msgQueue.queue.count);
    }

    return iRet;
}
 
//  static D32S ipmMsgCenterPostMsg(D32S iMsgType, void* pMsgData, D32U uiMsgDataLen)
//  {
//      return ipmMsgDispatcherHandleMsg(&m_ipmContext.msgCenter.msgDispatcher, iMsgType, pMsgData, uiMsgDataLen);
//  }


/**
 * 消息中心处理消息，用于解决向以下问题：
      DIM请求ECU刷写时，DIM同时通知application中IPM ECU刷写进度或刷写结果，会因为ipmDoFlashEcusReally占有f->mutex，
   导致ipmDoFlashEcuSetResult/ipmDoFlashEcuSetProgress中无法获取f->mutex而立即处理返回DIM，arpc也无法立即结束DIM
   对application通知消息，最终arpc中DIM ECU刷写请求超时，导致ECU刷写失败，也导致后续所有待升级ECU刷写请求都失败
 */
 
static void ipmMsgCenterProcessMsg(ADM_IPM_MSG_CENTER_T* pMsgCenter)
{
    // ADM_IPM_MSG_CENTER_T* pMsgCenter = &m_ipmContext.msgCenter;
    ADM_MQ_MSG_T* pMsg = ipmMsgQueuePop(&pMsgCenter->msgQueue);
    while (NULL != pMsg)
    {
        (void)ipmMsgDispatcherHandleMsg(&pMsgCenter->msgDispatcher, pMsg->msgType, pMsg->msgData, pMsg->msgDataLen);
        adm_free(pMsg);

        pMsg = ipmMsgQueuePop(&pMsgCenter->msgQueue);
    }
}

static D32S ipmHandleEcuFlashProgressNotify(void* pData, D32U uiDataLen)
{
    IPM_ECU_FLASH_PROGRESS_T* pProgress = (IPM_ECU_FLASH_PROGRESS_T*)pData;

    log_i("handle ecu flash process notify, ecuReqId[%s] ecuSwDid[%s] progress[%d]", pProgress->ecuReqId, pProgress->ecuSwDid, pProgress->progress);
    ipmDoFlashEcuSetProgress(pProgress->dim->doFlash, pProgress->ecuReqId, pProgress->ecuSwDid, pProgress->progress);
    return ADM_ECODE_SUCCESSED;
}

static D32S ipmHandleIccFlashProgressNotify(void* pData, D32U uiDataLen)
{
    D32S iTmpProgress = 0;
    IPM_ICC_FLASH_PROGRESS_T* pProgress = (IPM_ICC_FLASH_PROGRESS_T*)pData;
    const DCHAR* pRequestId = NULL;
    const DCHAR* pSwDId = NULL;

    log_i("handle icc flash process notify, installType[%d] progress[%d]", pProgress->installType, pProgress->progress);

    /* android 和 qnx 占比80%，MCU 占比20%. */
    switch (pProgress->installType)
    {
        case ICC_SOC:
            iTmpProgress = (pProgress->progress * ADM_IPM_ANDROID_QNX_PROGRESS_RATIO)/100;
            pRequestId = ICC_REQ_ID;
            pSwDId = ICC_SW_DID;
            break;
        case ICC_MCU:
            iTmpProgress = ADM_IPM_ANDROID_QNX_PROGRESS_RATIO + (pProgress->progress * ADM_IPM_MCU_PROGRESS_RATIO)/100;
            pRequestId = ICC_REQ_ID;
            pSwDId = ICC_SW_DID;
            break;
        case ICC_SOC_MCU:
            iTmpProgress = pProgress->progress;
            pRequestId = ICC_REQ_ID;
            pSwDId = ICC_SW_DID;
            break;
        case ICC_INSTRUMENT_PANEL:
            iTmpProgress = pProgress->progress;
            pRequestId = ICC_INSTRUMENT_PANEL_REQ_ID;
            pSwDId = ICC_INSTRUMENT_PANEL_SW_DID;
            break;
        case ICC_CENTRAL_CTRL_SCREEN:
            iTmpProgress = pProgress->progress;
            pRequestId = ICC_CENTRAL_CTRL_SCREEN_REQ_ID;
            pSwDId = ICC_CENTRAL_CTRL_SCREEN_SW_DID;
            break;
        case ICC_SECOND_ROW_AIR_CONDITION_SCREEN:
            iTmpProgress = pProgress->progress;
            pRequestId = ICC_AIR_CONDITION_SCREEN_REQ_ID;
            pSwDId = ICC_AIR_CONDITION_SCREEN_SW_DID;
            break;
        case ICC_CEILING_MOUNTED_SCREEN:
            iTmpProgress = pProgress->progress;
            pRequestId = ICC_AIR_CEILING_MOUNT_REQ_ID;
            pSwDId = ICC_AIR_CEILING_MOUNT_SW_DID;
            break;
        default:
            log_e("invalid installType = %d", pProgress->installType);
            return ADM_ECODE_SUCCESSED;
    }

    ipmDoFlashEcuSetProgress(pProgress->dim->doFlash, pRequestId, pSwDId, iTmpProgress);
    return ADM_ECODE_SUCCESSED;
}

/*!
 * @brief 异步信号标签
 */
typedef struct stDownAsyncTag
{
    pthread_cond_t       stCond;    //!< 信号量
    pthread_mutex_t      stMutex;   //!< 信号量锁
    DBOOL                bUpdate;   //!< 信息是否更新
    DBOOL                bIsInit;
}ADM_IPM_ASYNCTAG_S;

typedef struct {
    D16U installInstanceId;
    D16U requestTimeout;
} ADM_IPM_MY_PARAM_T;

static ADM_IPM_ASYNCTAG_S g_stEcuResultTag = {0};
#define ADM_IPM_WAIT_ECU_RESULT_CYCLE		5*60*1000
/** 毫秒 */
#define ADM_IPM_TIME_MSEC       1000
/** 微秒 */
#define ADM_IPM_TIME_USEC       1000000
/** 纳秒 */
#define ADM_IPM_TIME_NSEC       1000000000

/** 判断icc下挂件安装状态线程标志位 */
static DBOOL g_bIsThreadRunning = FALSE;

static void __iccInstallTriggerInfoDel()
{
	if(TRUE == adm_com_kvstorage_has_key(adm_core_service_getBusinessDatabaseDb(), ADM_IPM_ICC_TRIGGER_INFO_KEY))
	{
		adm_com_kvstorage_remove_key(adm_core_service_getBusinessDatabaseDb(), ADM_IPM_ICC_TRIGGER_INFO_KEY);	
	}	

	return;
}

static D32S __iccInstallTriggerInfoFromKV()
{
    size_t iBufLen = 0;
	D32S iRet = ADM_ECODE_FAILED;
	DCHAR *pcTriggerInfo = NULL;
    ADM_KV_STORAGE_VALUE_TYPE_E eValueType = ADM_KV_STORAGE_VALUE_TYPE_INVALID;

    pthread_mutex_lock(&g_stTriggerIccInfoMutex);

	if(TRUE == adm_com_kvstorage_has_key(adm_core_service_getBusinessDatabaseDb(),
										  ADM_IPM_ICC_TRIGGER_INFO_KEY))
	{
        log_i("kv has the key %s", ADM_IPM_ICC_TRIGGER_INFO_KEY);
        if(TRUE == adm_com_kvstorage_get_string(adm_core_service_getBusinessDatabaseDb(),
												ADM_IPM_ICC_TRIGGER_INFO_KEY, 
												&pcTriggerInfo, 
												&iBufLen, 
												&eValueType))
        {
			iRet = ADM_ECODE_SUCCESSED;
        }
		else
		{
			log_e("get install trigger info err from kv");
			iRet = ADM_ECODE_KV_GET_STRING_FAILED;
		}
	}

    pthread_mutex_unlock(&g_stTriggerIccInfoMutex);

	adm_free(pcTriggerInfo);
	
	return iRet;										  
}


static D32S __iccInstallTriggerInfoToKV(DCHAR *reqId)
{
    ADM_KV_STORAGE_VALUE_TYPE_E eType = ADM_KV_STORAGE_VALUE_TYPE_INVALID;
	DBOOL bIsSetOk = FALSE;
	D32S iRet = ADM_ECODE_SUCCESSED;
	
	if (ADM_IS_NULL(reqId))	
	{
		log_e("invalid param");
		return ADM_ECODE_INVALID_PARAM;
	}
		
    pthread_mutex_lock(&g_stTriggerIccInfoMutex);	

	eType = ADM_KV_STORAGE_VALUE_TYPE_STRING;
	bIsSetOk = adm_com_kvstorage_set_string(adm_core_service_getBusinessDatabaseDb(), 
										  	ADM_IPM_ICC_TRIGGER_INFO_KEY, 
										  	reqId, 
										  	eType);
    if (TRUE != bIsSetOk) 
	{
		log_e("set install trigger info failed");
		iRet = ADM_ECODE_KV_SET_STRING_FAILED;
    } 

    pthread_mutex_unlock(&g_stTriggerIccInfoMutex);
	log_i("set install trigger info success");

	return iRet;
}


/// @brief 检查icc下挂件刷写结果，均刷写成功后，调用激活和切面重启
/// @param argv 
/// @return 
static D32S __iccEcusResultHandle(D16U installInstanceId, D16U requestTimeout)
{
	log_i("__waitIccEcuResulteHandle start");
    
    ADM_IPS_PROGRESS_T stProgress = {0};
	ADM_IPS_ECU_PROGRESS_T *pstEcuProgress = NULL;
	D32S iRet = ADM_ECODE_FAILED;
    D32S errorCode = ADM_ECODE_FAILED;
    D32S i = 0;
	DBOOL bIsFinished = TRUE;
	
	adm_ips_progressInit(&stProgress);
	adm_ips_progressGet(&stProgress);

	if(0 == stProgress.ecus.ecusLen || 
	   NULL == stProgress.ecus.ecus)
	{
        log_i("ecusLen is 0, or ecus is null");
		return ADM_ECODE_SUCCESSED;
	}
	
	for(i = 0; i < stProgress.ecus.ecusLen; i++)
	{
		pstEcuProgress = &stProgress.ecus.ecus[i];

        /* 与ICC并行升级关联的零件是否安装成功 */
        /* TBD：ICC关联件待提供 */
        if(!strcmp(pstEcuProgress->ecuReqId, "0xicc1") || 
            !strcmp(pstEcuProgress->ecuReqId, "0xicc2") ||
            !strcmp(pstEcuProgress->ecuReqId, "0xicc3"))
        {
            /* 与ICC并行升级关联的零件不是安装完成，或者安装失败 */
            if (ADM_IPS_ECU_PHASE_POST_INSTALL != pstEcuProgress->state || 
                ADM_ECODE_SUCCESSED != pstEcuProgress->progress[0].result)
            {
                bIsFinished = FALSE;
                break;
            }
        }
	}

	adm_ips_progressDeInit(&stProgress);
    
    if (bIsFinished){
        log_i("the other ecus of icc already install ok, prepare change slot");

        log_i("start get slot");

        log_i("start set slot");
        iRet = adm_dim_ucl_requestSetChangeSlot_icc(installInstanceId,requestTimeout, &errorCode);
        log_d("adm_dim_ucl_requestSetChangeSlot_icc ret:%d, errorCode:%d", iRet, errorCode);

        if (!iRet){
            log_i("start reboot");
			/* kv保存已设置切换分区和重启切分区标志（收到车机结果后，删除此标记） */
			__iccInstallTriggerInfoToKV(ICC_REQ_ID);
		
            iRet = adm_dim_ucl_requestUpdateAndReboot(installInstanceId,requestTimeout, &errorCode);
            log_d("adm_dim_ucl_requestUpdateAndReboot ret:%d, errorCode:%d", iRet, errorCode);
            iRet = ADM_ECODE_SUCCESSED;
        }
    }

	log_i("__waitIccEcuResulteHandle end");
	
	return iRet;
}

static D32S adm_ipm_ThrdCondWait(pthread_cond_t *pstCond, pthread_mutex_t *pstMutex, DBOOL *pIsSignal, D32U ulAbsTime)
{
    if (NULL == pstCond || NULL == pstMutex || NULL == pIsSignal)
    {
        log_e("Invalid param.");
        return ADM_ECODE_INVALID_PARAM;
    }

    D32S iRet;
    // pthread_mutex_lock(pstMutex);

    log_d("start wait time = %d", ulAbsTime);

    if(0 != ulAbsTime)
    {
        iRet = adm_pub_pthread_cond_timedwait(pstCond, pstMutex, pIsSignal, NULL, (D32S)ulAbsTime);

        log_d("end wait ");
    }
    else
    {
        iRet = pthread_cond_wait(pstCond, pstMutex);
        log_d("end wait cond");
    }

    // pthread_mutex_unlock(pstMutex);

    return iRet;
}

static void* __waitIccEcusResultHandle(void* arg)
{
    D32S iRet;
    ADM_IPM_MY_PARAM_T* param = NULL;
    pthread_condattr_t stCondattr;

    log_i("__waitIccEcusResultHandle begin");

    if (ADM_IS_NULL(arg))
	{
		log_e("invalid param");
		return NULL;
	}
	
	param = (ADM_IPM_MY_PARAM_T*)arg;

    (void)pthread_mutex_init(&g_stEcuResultTag.stMutex, NULL);
    (void)pthread_condattr_init(&stCondattr);
    (void)pthread_condattr_setclock(&stCondattr, CLOCK_MONOTONIC);
    (void)pthread_cond_init(&g_stEcuResultTag.stCond, &stCondattr);
    
    g_stEcuResultTag.bIsInit = TRUE;

    do
    {
        if (TRUE == g_stEcuResultTag.bUpdate)
        {
            break ;
        }

        iRet = __iccEcusResultHandle(param->installInstanceId, param->requestTimeout);
		if (!iRet){
			log_i("All icc ecus already finished upgrade, request UpdateAndReboot icc");
			break;
		}

        /** 等待周期时间（5分钟） */
        adm_ipm_ThrdCondWait(&g_stEcuResultTag.stCond, &g_stEcuResultTag.stMutex,       
                            &g_stEcuResultTag.bUpdate, ADM_IPM_WAIT_ECU_RESULT_CYCLE);
    }while(TRUE);

    (void)pthread_mutex_destroy(&g_stEcuResultTag.stMutex);
    (void)pthread_cond_destroy(&g_stEcuResultTag.stCond);
	
    g_stEcuResultTag.bIsInit = FALSE;
    g_stEcuResultTag.bUpdate = FALSE;

    g_bIsThreadRunning = FALSE;
    adm_free(param);
    log_i("__waitIccEcusResultHandle end");

    return NULL;
}

static D32S __getIccInstallStatus(D32S *iccInstallProgress, ADM_IPS_ECU_PHASE_E *iccInstallState)
{
    D32S iRet = ADM_ECODE_FAILED;
    ADM_IPS_PROGRESS_T progress;
    adm_ips_progressInit(&progress);
    adm_ips_progressGet(&progress);

    ADM_IPS_ECU_PROGRESS_T* ecuProgresses = progress.ecus.ecus;

    for (D32S i = 0; i < progress.ecus.ecusLen; ++i) {
		if (!strcmp(ecuProgresses[i].ecuReqId, ICC_REQ_ID) && !strcmp(ecuProgresses[i].ecuSwDid, ICC_SW_DID)){
			*iccInstallProgress = ecuProgresses[i].progress[ADM_IPS_ECU_PHASE_INDEX_UPGRADE].progress;
            *iccInstallState = ecuProgresses[i].state;
            iRet = ADM_ECODE_SUCCESSED;
			log_i("icc install progress is %d, state %d", *iccInstallProgress, *iccInstallState);
			break;
		}
    }

    return iRet;
}

/// @brief 判断是否需要设置icc切换分区标志，以及设置重启切换分区
/// @param  
static void __waitIccEcusResultThreadCreate(D16U installInstanceId, D16U requestTimeout)
{
	D32S iRet = ADM_ECODE_SUCCESSED;
	pthread_t stwaitIccEcuResultThreadId = 0;
    ADM_IPM_MY_PARAM_T *pstParam = NULL;
	D32S iccInstallProgress = 0;
    ADM_IPS_ECU_PHASE_E iccInstallResult = ADM_IPS_ECU_PHASE_INVALID;

    if (TRUE == g_bIsThreadRunning)
    {
        log_w("wait icc ecus result thread is running");
        return ;
    }

    /* 判断数据库中是否存在已设置切换分区和重启切分区标志，如果已设置，则此处返回 */
	iRet = __iccInstallTriggerInfoFromKV();
	if(!iRet){
		log_w("already set slot, just return");
		return ;
	}

    /* 判断icc升级进度是否为100%，如果不是100%，则此处返回 */
	iRet = __getIccInstallStatus(&iccInstallProgress, &iccInstallResult);
	if (iRet || iccInstallProgress != 100 || 
        (ADM_IPS_ECU_PHASE_INSTALL == iccInstallResult || ADM_IPS_ECU_PHASE_POST_INSTALL_ROLLBACK == iccInstallResult)){
		log_w("The task does not include icc, or the icc upgrade progress has not yet reached 100, or the icc upgrade has been completed, just return");
		return ;
	}

	log_i("__waitIccEcusResultThreadCreate start");
    g_bIsThreadRunning = TRUE;

    pstParam = (ADM_IPM_MY_PARAM_T *)adm_malloc(sizeof(ADM_IPM_MY_PARAM_T));
    if (ADM_IS_NULL(pstParam))
    {
        log_e("malloc err");
        return ;
    }

    pstParam->installInstanceId = installInstanceId;
    pstParam->requestTimeout = requestTimeout;

	iRet = pthread_create(&stwaitIccEcuResultThreadId, NULL, __waitIccEcusResultHandle, (void*)pstParam);
    if (ADM_IS_SUCCESS(iRet)) 
	{
		log_i("pthread_create successed");
		pthread_detach(stwaitIccEcuResultThreadId);
    } 
	else 
	{
		log_e("pthread_create failed");
        adm_free(pstParam);
        g_bIsThreadRunning = FALSE;
    }	

	log_i("__waitIccEcusResultThreadCreate end");	

	return;
}

static D32S ipmHandleEcuFlashResultNotify(void* pData, D32U uiDataLen)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    IPM_ECU_FLASH_RESULT_T* pResult = (IPM_ECU_FLASH_RESULT_T*)pData;

    adm_ipm_init();

    log_i("handle ecu flash result notify, ecuReqId[%s] ecuSwDid[%s] result[%d]", pResult->ecuReqId, pResult->ecuSwDid, pResult->result);

    ipmDoFlashEcuSetResult(pResult->dim->doFlash, pResult->ecuReqId, pResult->ecuSwDid, pResult->result, pResult->resultDescrip);

    // ADM_IPM_ECU_INFO_NODE_T *node = ipmDoFlashEcusFind(pResult->dim->doFlash, ICC_REQ_ID, ICC_SW_DID);
	// if(node != NULL){
	//     /* 收到ecu结果时，判断是否需要设置icc切换分区标志以及重启切换分区 */
    //     const DCHAR *installInstanceId = node->ecsEcu->installInstanceId;
	//     __waitIccEcusResultThreadCreate((D16U)ipmHtoi(installInstanceId), pResult->dim->requestTimeout);

    //     sleep(1);

	//     if (g_stEcuResultTag.bIsInit){
	//         (void)pthread_mutex_lock(&g_stEcuResultTag.stMutex);
	//         (void)pthread_cond_signal(&g_stEcuResultTag.stCond);
	//         (void)pthread_mutex_unlock(&g_stEcuResultTag.stMutex);
	//     }
	// }

    return ADM_ECODE_SUCCESSED;
}

/// @brief 处理 icc 升级结果
/// @param pData 
/// @param uiDataLen 
/// @return 
static D32S ipmHandleIccFlashResultNotify(void* pData, D32U uiDataLen)
{
    IPM_ICC_FLASH_RESULT_T* pResult = (IPM_ICC_FLASH_RESULT_T*)pData;
    const DCHAR* pRequestId = NULL;
    const DCHAR* pSwDId = NULL;

    log_i("handle icc flash result notify,  installType[%d] result[%d]", pResult->installType, pResult->result);

    switch (pResult->installType)
    {
        case ICC_SOC:
            pRequestId = ICC_REQ_ID;
            pSwDId = ICC_SW_DID;
            break;
        case ICC_MCU:
            pRequestId = ICC_REQ_ID;
            pSwDId = ICC_SW_DID;
            break;
        case ICC_SOC_MCU:
            pRequestId = ICC_REQ_ID;
            pSwDId = ICC_SW_DID;
            break;
        case ICC_INSTRUMENT_PANEL:
            pRequestId = ICC_INSTRUMENT_PANEL_REQ_ID;
            pSwDId = ICC_INSTRUMENT_PANEL_SW_DID;
            break;
        case ICC_CENTRAL_CTRL_SCREEN:
            pRequestId = ICC_CENTRAL_CTRL_SCREEN_REQ_ID;
            pSwDId = ICC_CENTRAL_CTRL_SCREEN_SW_DID;
            break;
        case ICC_SECOND_ROW_AIR_CONDITION_SCREEN:
            pRequestId = ICC_AIR_CONDITION_SCREEN_REQ_ID;
            pSwDId = ICC_AIR_CONDITION_SCREEN_SW_DID;
            break;
        case ICC_CEILING_MOUNTED_SCREEN:
            pRequestId = ICC_AIR_CEILING_MOUNT_REQ_ID;
            pSwDId = ICC_AIR_CEILING_MOUNT_SW_DID;
            break;
        default:
            log_e("invalid installType = %d", pResult->installType);
            return ADM_ECODE_SUCCESSED;
    }

    ADM_IPM_ECU_INFO_NODE_T *node = ipmDoFlashEcusFind(pResult->dim->doFlash, pRequestId, pSwDId);
	if(n