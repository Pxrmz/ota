#include "adm_vsm_log.h"
#include "adm_vsm_impl.h"
#include "adm_vsm_uim.h"
#include "adm_ipm_install.h"
#include "adm_ips_install.h"
#include "adm_com_errorCode.h"
#include "adm_memory.h"
#include "adm_cvm_uim.h"
#include "adm_cvm_checkVersion.h"
#include "adm_cvm_powerOncheck.h"
#include "adm_cvm_syncVehicleInfo.h"
#include "adm_dpm_uim.h"
#include "adm_dpm_downloadManagement.h"
#include "adm_com_kvstorage.h"
#include "adm_cvs_versionPolicy.h"
#include "adm_ips_debug.h"
#include "adm_application_errorCode.h"
#include "adm_vsm_tspImpl.h"
#include "adm_soa.h"
#include "adm_soa_cfg.h"
#include "adm_irm_report.h"
#include "adm_dpm_reportCloud.h"
#include "adm_vsm_signal_impl.h"
#include "adm_vrs_registerInfo.h"

#include "adm_irs_upload.h"

#include <unistd.h>

static ADM_VSM_MANAGER_T m_manager;
static DBOOL m_isInitManager = FALSE;
static DBOOL m_isHmiOnline = FALSE;
static DBOOL s_isHmiStatSync = DFALSE;
static DBOOL g_bIsTspDealing = DFALSE;
static DBOOL g_bTspChecking = DFALSE;
static ADM_APPLICATION_COMMON_CMDID_TYPE_E g_eCmdIdType = ADM_APPLICATION_COMMON_CMDID_TYPE_INVALID;

void adm_vsm_init(void)
{
    if (!m_isInitManager) {
        vsmManagerInit(&m_manager);
        m_isInitManager = TRUE;
    }
}
void adm_vsm_deinit(void)
{
    if (m_isInitManager) {
        vsmManagerDeInit(&m_manager);
        m_isInitManager = FALSE;
    }
}
ADM_VSM_STATE_E adm_vsm_stateGet(void)
{
    adm_vsm_init();
    return vsmStateGet(&m_manager.state);
}

void adm_vsm_stateSet(ADM_VSM_STATE_E eState)
{
    adm_vsm_init();
    vsmStateSet(&m_manager.state, eState);
    
    return;
}

void adm_vsm_setHmiOnline(DBOOL onLine)
{
    adm_vsm_init();
    m_isHmiOnline = onLine;
}

DBOOL adm_vsm_getHmiOnline(void)
{
    adm_vsm_init();
    return m_isHmiOnline;
}

void adm_vsm_setHmiSyncStat(DBOOL bIsSync)
{
    adm_vsm_init();
    s_isHmiStatSync = bIsSync;

    return;
}

DBOOL adm_vsm_getHmiSyncStat(void)
{
    adm_vsm_init();
    return s_isHmiStatSync;
}

static void vsmTriggerTypeKVStore(ADM_APPLICATION_COMMON_CMDID_TYPE_E eTriggerType)
{
    DCHAR aTriggerTypeStr[4] = {0};

    /** 将状态值转为字符串，存储到kv中 */
    (void)adm_memset(aTriggerTypeStr, 0x00, sizeof(aTriggerTypeStr), sizeof(aTriggerTypeStr));
    (void)adm_snprintf(aTriggerTypeStr, sizeof(aTriggerTypeStr), "%u", (D32U)eTriggerType);

    (void)adm_com_kvstorage_set_string(adm_core_service_getBusinessDatabaseDb(),
                                       ADM_VSM_TRIGGER_TYPE_KV_KEY,
                                       aTriggerTypeStr,
                                       ADM_KV_STORAGE_VALUE_TYPE_UINT32);
    return;
}

static ADM_APPLICATION_COMMON_CMDID_TYPE_E vsmTriggerTypeKVLoad()
{
    DBOOL bIsOk = FALSE;
    size_t valLength = 0;
    DCHAR *pTriggerTypeState = NULL;
    ADM_KV_STORAGE_VALUE_TYPE_E eValType;
    ADM_APPLICATION_COMMON_CMDID_TYPE_E eTriggerType = ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_IGN;   ///< 默认上电

    bIsOk = adm_com_kvstorage_get_string(adm_core_service_getBusinessDatabaseDb(), ADM_VSM_TRIGGER_TYPE_KV_KEY,
                                         &pTriggerTypeState, &valLength, &eValType);
    if (FALSE == bIsOk)
    {
        log_w("Failed to get adm_com_kvstorage_get_string, key[%s]", ADM_VSM_TRIGGER_TYPE_KV_KEY);
        return eTriggerType;
    }
    (void)adm_stringToUint32(pTriggerTypeState, (D32U *)(&eTriggerType));
    adm_free(pTriggerTypeState);

    log_d("triggerType:%u", (D32U)eTriggerType);

    return eTriggerType;
}

//!< 设置触发类型
void adm_vsm_setTriggerType(ADM_APPLICATION_COMMON_CMDID_TYPE_E eTriggerType)
{
    adm_vsm_init();
    m_manager.ucm.eTriggerType = eTriggerType;
    vsmTriggerTypeKVStore(eTriggerType);
    log_d("VSM set trigger type[%d]", m_manager.ucm.eTriggerType);
}

ADM_APPLICATION_COMMON_CMDID_TYPE_E adm_vsm_getTriggerType()
{
    adm_vsm_init();
    log_d("VSM get trigger type[%d]", m_manager.ucm.eTriggerType);
    return m_manager.ucm.eTriggerType;
}

D32S adm_vsm_getIsFuel(DBOOL *pbIsFuel)
{
    D32U iCount = 0;
    const D32U MAX_RETRY_COUNT = 10;
    ADM_VRS_VEHICLE_POWER_TYPE_E ePowerType = VEHICLE_INVALID;

    if (NULL == pbIsFuel)
    {
        log_e("Invalid param, pbIsFuel is NULL");
        return ADM_ECODE_INVALID_PARAM;
    }

    //重试获取，防止apc还没初始化完成，拿到的是默认值
    //如果重试失败都失败了，就直接失败
    do
    {
        ePowerType = adm_vrs_getVehicle();

        if (VEHICLE_INVALID == ePowerType)
        {
            //return ADM_ECODE_FAILED;
            iCount++;
            log_e("retry[%d/%d], power type invalid, waiting...",iCount,MAX_RETRY_COUNT);
            sleep(3);
        }
        else
        {
            log_d("power type[%d]",ePowerType);
            break;
        }

        if (iCount > MAX_RETRY_COUNT)
        {
            log_e("failed after 10 retries, power type not confirmed");
            return ADM_ECODE_FAILED;
        }

    }while (TRUE);
    
    *pbIsFuel = (VEHICLE_ICE == ePowerType) ? DTRUE : DFALSE;
    return ADM_ECODE_SUCCESSED;
}

/************* tsp任务类型相关begin **********/
static void vsmTspStateKVLoad()
{
    DBOOL bIsOk = FALSE;
    size_t valLength = 0;
    DCHAR *pTspState = NULL;
    ADM_KV_STORAGE_VALUE_TYPE_E eValType;

    bIsOk = adm_com_kvstorage_get_string(adm_core_service_getBusinessDatabaseDb(), ADM_VSM_STATE_TSP_KV_KEY,
                                         &pTspState, &valLength, &eValType);
    if (FALSE == bIsOk)
    {
        log_w("Failed to get adm_com_kvstorage_get_string, key[%s]", ADM_VSM_STATE_TSP_KV_KEY);
        return;
    }
    (void)adm_stringToUint32(pTspState, (D32U*)(&g_eCmdIdType));
    adm_free(pTspState);

    log_d("tspState:%u", (D32U)g_eCmdIdType);

    return;
}

static void vsmTspStateKVStore()
{
    DCHAR aStateStr[4] = {0};

    /** 将状态值转为字符串，存储到kv中 */
    (void)adm_memset(aStateStr, 0x00, sizeof(aStateStr), sizeof(aStateStr));
    (void)adm_snprintf(aStateStr, sizeof(aStateStr), "%u", (D32U)g_eCmdIdType);

    (void)adm_com_kvstorage_set_string(adm_core_service_getBusinessDatabaseDb(),
                                       ADM_VSM_STATE_TSP_KV_KEY,
                                       aStateStr,
                                       ADM_KV_STORAGE_VALUE_TYPE_UINT32);

    return;
}
static void vsmSetTspTriggerType(ADM_APPLICATION_COMMON_CMDID_TYPE_E eTriggerType)
{
    adm_vsm_init();
    g_eCmdIdType = eTriggerType;

    vsmTspStateKVStore();
    return;
}

ADM_APPLICATION_COMMON_CMDID_TYPE_E adm_vsm_getTspTriggerType()
{
    adm_vsm_init();

    return g_eCmdIdType;
}
/************* tsp任务类型相关end ************/

static DBOOL adm_vsm_getTspTriggerChecking() {
    return g_bTspChecking;
}

static void adm_vsm_setTspTriggerChecking(DBOOL isChecking) {
    g_bTspChecking = isChecking;
}


static DBOOL g_iResumeFlag = FALSE;
DBOOL adm_vsm_triggerCheck(ADM_APPLICATION_COMMON_CMDID_TYPE_E eTriggerType)
{
    //等待60秒的时间，如果vsm状态机恢复完毕；
    for (int i = 0; i < 12; i++)
    {
        if (g_iResumeFlag)
        {
            break;
        }
        sleep(5);
    }
    
    adm_vsm_init();
	ADM_VSM_STATE_E state = adm_vsm_stateGet();
    if (ADM_VSM_STATE_IDLE == state) {

        if (adm_vsm_getTspTriggerChecking()) {
            log_d("tsp checking,  eTriggerType=%d", eTriggerType);
            return FALSE;
        }

        D32S iRet = ADM_ECODE_FAILED;

        if (ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_TSP == eTriggerType)
        {
            adm_vsm_setTspTriggerChecking(TRUE);

            iRet = adm_vsm_entryTspEnv();
            if (ADM_IS_ERR(iRet))
            {
                adm_vsm_setTspTriggerChecking(FALSE); 
                return FALSE;
            }
        }

        g_eCmdIdType = eTriggerType;
        adm_vsm_setTriggerType(eTriggerType);
        vsmStateTriggerCheck(&m_manager.state);
        adm_vsm_setTspTriggerChecking(FALSE); 
        return TRUE;
    } else {
        log_w("the state do not allow check,state = %d", state);
        return FALSE;
    }
}

DBOOL adm_vsm_triggerDownload(ADM_APPLICATION_COMMON_CMDID_TYPE_E eTriggerType)
{    
    adm_vsm_init();
	ADM_VSM_STATE_E state = adm_vsm_stateGet();
    if (ADM_VSM_STATE_DOWNLOAD_READY == state || ADM_VSM_STATE_DOWNLOAD_PAUSE == state) {
        adm_vsm_setTriggerType(eTriggerType);
        vsmStateTriggerDownload(&m_manager.state);
        return TRUE;
    } else if (ADM_VSM_STATE_DOWNLOAD_DOING == state) {
        return TRUE;
    } else {
        log_w("the state do not allow download,state = %d", state);
        return FALSE;
    }
}

DBOOL adm_vsm_triggerInstall(void)
{
    ADM_CVS_UPGRADE_MODE_E mode = ADM_CVS_UPGRADE_MODE_INVALID;

    adm_vsm_init();

    (void)adm_cvs_getUpgradeMode((D32S *)&mode);

    switch (mode)
    {
        /** 静默升级 */
        case ADM_CVS_UPGRADE_MODE_SLIENCE_INSTALL:
            adm_vsm_setTriggerType(ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_SILENCE);
            break;

        /** USB升级  */
        case ADM_CVS_UPGRADE_MODE_USB_CHECK:
            adm_vsm_setTriggerType(ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_USB);
            break;

        /** 救援升级 */
        case ADM_CVS_UPGRADE_MODE_RESCUE:
            adm_vsm_setTriggerType(ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_RESCUE);
            break;

        /** 产线升级 */
        case ADM_CVS_UPGRADE_MODE_PROD_LINE:
            adm_vsm_setTriggerType(ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_PROD_LINE);
            break;

        /** APK自升级  */
        case ADM_CVS_UPGRADE_MODE_APK_SELF:
            adm_vsm_setTriggerType(ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_APK_SELF_UPGRADE);
            break;

        /** 强制升级 */
        case ADM_CVS_UPGRADE_MODE_FORCE:
            adm_vsm_setTriggerType(ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_FORCE);
            break;

        /** 常规任务/静默下载常规安装 */
        default:
            adm_vsm_setTriggerType(ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_USER);
            break;
    }

    ADM_VSM_STATE_E state = adm_vsm_stateGet();
    if (ADM_VSM_STATE_UPGRADE_READY == state || ADM_VSM_STATE_UPGRADE_PAUSE == state)
    {
        vsmStateTriggerInstall(&m_manager.state);
        return TRUE;
    }
    else if (ADM_VSM_STATE_UPGRADE_DOING == state)
    {
        return TRUE;
    }
    else
    {
        log_w("the state do not allow install,state = %d", state);
        return FALSE;
    }
}

void adm_vsm_resume(void)
{
    adm_vsm_init();
    vsmStateResume(&m_manager.state);
    g_iResumeFlag = TRUE;
}

/* ADM_VSM_MANAGER_T */
static void vsmManagerInit(ADM_VSM_MANAGER_T* m)
{
    vsmUcmInit(&m->ucm);
    vsmStateInit(&m->state, &m->worker);
    vsmWorkerInit(&m->worker, &m->ucm, &m->state);
}
static void vsmManagerDeInit(ADM_VSM_MANAGER_T* m)
{
    vsmWorkerDeInit(&m->worker);
    vsmStateDeInit(&m->state);
    vsmUcmDeInit(&m->ucm);
}

typedef enum
{
    ADM_VSM_WORKER_CMD_ID_UNKOWN = 0,
    ADM_VSM_WORKER_CMD_ID_CHECK,
    ADM_VSM_WORKER_CMD_ID_DOWNLOAD,
    ADM_VSM_WORKER_CMD_ID_DOWNLOAD_RESUME,
    ADM_VSM_WORKER_CMD_ID_INSTALL,
    ADM_VSM_WORKER_CMD_ID_INSTALL_RESUME,
} ADM_VSM_WORKER_CMD_ID_E;

typedef struct
{
    ADM_VSM_WORKER_T* worker;
    ADM_VSM_WORKER_CMD_ID_E cmdId;
    /* TODO union{}u; */
} ADM_VSM_WORKER_DATA_T;
static ADM_VSM_WORKER_DATA_T* vsmWorkerDataNew(ADM_VSM_WORKER_T* worker, ADM_VSM_WORKER_CMD_ID_E cmdId)
{
    ADM_VSM_WORKER_DATA_T* data = adm_malloc(sizeof(*data));
    if (data != NULL) {
        data->worker = worker;
        data->cmdId = cmdId;
    }
    return data;
}
static void vsmWorkerDataDel(ADM_VSM_WORKER_DATA_T* data)
{
    adm_free(data);
}

/* ADM_VSM_WORKER_T */
static void vsmWorkerInit(ADM_VSM_WORKER_T* w, ADM_VSM_XXM_T* ucm, ADM_VSM_STATE_T* state)
{
    w->ucm = ucm;
    w->state = state;
    (void)pthread_mutex_init(&w->mutex, NULL);
    w->stop = (DCHAR)0;
    w->count = 0U;
}
static void vsmWorkerDeInit(ADM_VSM_WORKER_T* w)
{
    (void)pthread_mutex_lock(&w->mutex);
    w->stop = (DCHAR)1;
    (void)pthread_mutex_unlock(&w->mutex);

    while (TRUE) {
        (void)pthread_mutex_lock(&w->mutex);
        D32U count = w->count;
        (void)pthread_mutex_unlock(&w->mutex);
        if (count == 0U) {
            break;
        } else {
            (void)usleep(200 * 1000);
        }
    }

    (void)pthread_mutex_destroy(&w->mutex);
}
static void* vsmWorkerRunHandle(void* argv)
{
    ADM_VSM_WORKER_DATA_T* data = (ADM_VSM_WORKER_DATA_T*)argv;
    ADM_VSM_WORKER_T* w = data->worker;

    switch (data->cmdId) {
    case ADM_VSM_WORKER_CMD_ID_CHECK:
        vsmStateResultCheck(w->state, vsmUcmCheck(w->ucm));
        break;
    case ADM_VSM_WORKER_CMD_ID_DOWNLOAD:
        vsmStateResultDownload(w->state, vsmUcmDownload(w->ucm));
        break;
    case ADM_VSM_WORKER_CMD_ID_DOWNLOAD_RESUME:
        vsmStateResultDownload(w->state, vsmUcmDownloadResume(w->ucm));
        break;
    case ADM_VSM_WORKER_CMD_ID_INSTALL:
        vsmStateResultInstall(w->state, vsmUcmInstall(w->ucm));
        break;
    case ADM_VSM_WORKER_CMD_ID_INSTALL_RESUME:
        vsmStateResultInstall(w->state, vsmUcmInstallResume(w->ucm));
        break;
    default:
        break;
    }

    (void)pthread_mutex_lock(&w->mutex);
    w->count--;
    (void)pthread_mutex_unlock(&w->mutex);
    vsmWorkerDataDel(data);
    return NULL;
}
static DBOOL vsmWorkerRun(ADM_VSM_WORKER_T* w, ADM_VSM_WORKER_DATA_T* data)
{
    DBOOL dRet = FALSE;
    (void)pthread_mutex_lock(&w->mutex);
    if (w->stop == (DCHAR)0) {
        pthread_t id;
        if (pthread_create(&id, NULL, vsmWorkerRunHandle, data) == 0) {
            w->count++;
            (void)pthread_detach(id);
            dRet = TRUE;
        }
    }
    (void)pthread_mutex_unlock(&w->mutex);
    return dRet;
}
static DBOOL vsmWorkerCheck(ADM_VSM_WORKER_T* w)
{
    DBOOL dRet = FALSE;
    ADM_VSM_WORKER_DATA_T* data = vsmWorkerDataNew(w, ADM_VSM_WORKER_CMD_ID_CHECK);
    if (data != NULL) {
        dRet = vsmWorkerRun(w, data);
        if (!dRet) {
            vsmWorkerDataDel(data);
        }
    }
    return dRet;
}
static DBOOL vsmWorkerDownload(ADM_VSM_WORKER_T* w)
{
    DBOOL dRet = FALSE;
    ADM_VSM_WORKER_DATA_T* data = vsmWorkerDataNew(w, ADM_VSM_WORKER_CMD_ID_DOWNLOAD);
    if (data != NULL) {
        dRet = vsmWorkerRun(w, data);
        if (!dRet) {
            vsmWorkerDataDel(data);
        }
    }
    return dRet;
}
static DBOOL vsmWorkerDownloadResume(ADM_VSM_WORKER_T* w)
{
    DBOOL dRet = FALSE;
    ADM_VSM_WORKER_DATA_T* data = vsmWorkerDataNew(w, ADM_VSM_WORKER_CMD_ID_DOWNLOAD_RESUME);
    if (data != NULL) {
        dRet = vsmWorkerRun(w, data);
        if (!dRet) {
            vsmWorkerDataDel(data);
        }
    }
    return dRet;
}
static DBOOL vsmWorkerInstall(ADM_VSM_WORKER_T* w)
{
    DBOOL dRet = FALSE;
    ADM_VSM_WORKER_DATA_T* data = vsmWorkerDataNew(w, ADM_VSM_WORKER_CMD_ID_INSTALL);
    if (data != NULL) {
        dRet = vsmWorkerRun(w, data);
        if (!dRet) {
            vsmWorkerDataDel(data);
        }
    }
    return dRet;
}
static DBOOL vsmWorkerInstallResume(ADM_VSM_WORKER_T* w)
{
    DBOOL dRet = FALSE;
    ADM_VSM_WORKER_DATA_T* data = vsmWorkerDataNew(w, ADM_VSM_WORKER_CMD_ID_INSTALL_RESUME);
    if (data != NULL) {
        dRet = vsmWorkerRun(w, data);
        if (!dRet) {
            vsmWorkerDataDel(data);
        }
    }
    return dRet;
}

/* ADM_VSM_STATE_T */
static void vsmStateInit(ADM_VSM_STATE_T* s, ADM_VSM_WORKER_T* worker)
{
    (void)pthread_mutex_init(&s->mutex, NULL);
    s->state = ADM_VSM_STATE_IDLE;
    s->worker = worker;
    vsmStateKVLoad(s);
    log_d("state:%s", vsmStateString(s->state));
}
static void vsmStateDeInit(ADM_VSM_STATE_T* s)
{
    (void)pthread_mutex_destroy(&s->mutex);
}
static ADM_VSM_STATE_E vsmStateGet(ADM_VSM_STATE_T* s)
{
    (void)pthread_mutex_lock(&s->mutex);
    ADM_VSM_STATE_E state = s->state;
    (void)pthread_mutex_unlock(&s->mutex);
    return state;
}
static void vsmStateTriggerCheck(ADM_VSM_STATE_T* s)
{
    (void)pthread_mutex_lock(&s->mutex);
    if (ADM_VSM_STATE_IDLE == s->state || ADM_VSM_STATE_UPGRADE_READY == s->state) {
        if (vsmWorkerCheck(s->worker)) {
            vsmStateSet(s, ADM_VSM_STATE_CHECK_DOING);
        }
    }
    (void)pthread_mutex_unlock(&s->mutex);
}

static void vsmStateTriggerDownload(ADM_VSM_STATE_T* s)
{
    (void)pthread_mutex_lock(&s->mutex);
    if (ADM_VSM_STATE_DOWNLOAD_READY == s->state || ADM_VSM_STATE_DOWNLOAD_PAUSE == s->state) {
        if (vsmWorkerDownload(s->worker)) {
            vsmStateSet(s, ADM_VSM_STATE_DOWNLOAD_DOING);
        }
    }
    (void)pthread_mutex_unlock(&s->mutex);
}

//static void vsmStateTriggerDownloadCancel(ADM_VSM_STATE_T* s)
//{}
//static void vsmStateTriggerDownloadPause(ADM_VSM_STATE_T* s)
//{}

static void vsmStateTriggerInstall(ADM_VSM_STATE_T* s)
{
    (void)pthread_mutex_lock(&s->mutex);
    if (ADM_VSM_STATE_UPGRADE_READY == s->state || ADM_VSM_STATE_UPGRADE_PAUSE == s->state) {
        if (vsmWorkerInstall(s->worker)) {
            vsmStateSet(s, ADM_VSM_STATE_UPGRADE_DOING);
        }
    }
    (void)pthread_mutex_unlock(&s->mutex);
}

#if 0
static D32S vsmUpgradeModeSwitch(D32S iUpgradeMode)
{
    D32S iUpgradeModeTemp = ADM_CVS_UPGRADE_MODE_INVALID;

    /** 1:立即升级
        2:预约升级
        3: 静默升级
        4: 产线升级
        5: 救援升级
        6: USB 升级
        7: 正常退出
    */
    switch (iUpgradeMode)
    {
    case ADM_CVS_UPGRADE_MODE_USER_CLICK:
    case ADM_CVS_UPGRADE_MODE_SLIENCE_DOWNLOAD:
        iUpgradeModeTemp = 1;
        break;
    case ADM_CVS_UPGRADE_MODE_FORCE:    
    case ADM_CVS_UPGRADE_MODE_APK_SELF:
    case ADM_CVS_UPGRADE_MODE_SLIENCE_INSTALL:
        iUpgradeModeTemp = 3;           //!< 静默升级
        break;
    case ADM_CVS_UPGRADE_MODE_PROD_LINE:
        iUpgradeModeTemp = 4;
        break;
    case ADM_CVS_UPGRADE_MODE_RESCUE:
        iUpgradeModeTemp = 5;
        break;
    case ADM_CVS_UPGRADE_MODE_USB_CHECK:
        iUpgradeModeTemp = 6;
        break;
    default:
        log_w("iUpgradeMode = %d is invalid", iUpgradeMode);
        break;
    }

    return iUpgradeModeTemp;
}
#endif

#if 0
static D32S vsmDimSoaHostSetUpgradeMode(D32S iUpgradeMode)
{
    D32S iRet = ADM_ECODE_FAILED;
    D32S iErrCode = ADM_ECODE_FAILED;

    log_i("upgradeMode[%d]", iUpgradeMode);
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

    return iRet;
}
#endif 


//static void vsmStateTriggerInstallCancel(ADM_VSM_STATE_T* s)
//{}
static void vsmStateResultCheck(ADM_VSM_STATE_T* s, ADM_VSM_RESULT_E result)
{
    ADM_APPLICATION_COMMON_CMDID_TYPE_E eTriggerType = ADM_APPLICATION_COMMON_CMDID_TYPE_INVALID;

    (void)pthread_mutex_lock(&s->mutex);
    if (ADM_VSM_STATE_CHECK_DOING == s->state) {
        if (ADM_VSM_RESULT_SUCCESS == result) {
            vsmStateSet(s, ADM_VSM_STATE_DOWNLOAD_READY);
            (void)pthread_mutex_unlock(&s->mutex);

            ADM_CVS_UPGRADE_MODE_E mode = ADM_CVS_UPGRADE_MODE_INVALID;
            D32S nResult = adm_cvs_getUpgradeMode((D32S*)&mode);

            /* 矫正真正的触发类型 */
            if ((ADM_ECODE_SUCCESSED == nResult))
            {
                switch (mode)
                {
                    case ADM_CVS_UPGRADE_MODE_USB_CHECK:
                        eTriggerType = ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_USB;
                        adm_vsm_setTriggerType(eTriggerType);
                        break;
                    case ADM_CVS_UPGRADE_MODE_SLIENCE_DOWNLOAD:
                        eTriggerType = ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_SILENCE;
                        break;
                    case ADM_CVS_UPGRADE_MODE_SLIENCE_INSTALL:
                        eTriggerType = ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_SILENCE;
                        break;
                    case ADM_CVS_UPGRADE_MODE_PROD_LINE:
                        eTriggerType = ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_PROD_LINE;
                        break;
                    case ADM_CVS_UPGRADE_MODE_APK_SELF:
                        eTriggerType = ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_APK_SELF_UPGRADE;
                        break;
                    case ADM_CVS_UPGRADE_MODE_FORCE:
                        eTriggerType = ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_FORCE;
                        break;
                    case ADM_CVS_UPGRADE_MODE_RESCUE:
                        eTriggerType = ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_RESCUE;
                        break;
                    default:
                        log_w("self download mode are not met, mode[ %d ]",mode);
                        break;
                }

                adm_ips_cmdIdSet(eTriggerType);
                log_i("cmdid is [%d]", adm_ips_cmdIdGet());

                if (eTriggerType != ADM_APPLICATION_COMMON_CMDID_TYPE_INVALID && mode != ADM_CVS_UPGRADE_MODE_SLIENCE_DOWNLOAD)
                {
                    /** 静默下载仍然是初始触发类型，除非类型是静默下载静默安装 */
                    adm_vsm_setTriggerType(eTriggerType);
                }
                
                if (TRUE == vsmGetIsAutoDownload()) 
                {
                    log_i("begin user click download");
                    eTriggerType = ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_USER;
                } 

                // vsmDimSoaHostSetUpgradeMode(vsmUpgradeModeSwitch(mode));

                if (ADM_APPLICATION_COMMON_CMDID_TYPE_INVALID == eTriggerType)
                {
                    eTriggerType = ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_USER;
                }

                /** 除了救援和U盘,其他下载不需要客户授权 */
                if (ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_USB != eTriggerType && 
                ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_RESCUE != eTriggerType)
                {
                    (void)adm_vsm_triggerDownload(eTriggerType);
                }
            }
            else 
            {
                log_w("nResult[%d]", nResult);
            }
            return;
        } 
        else if (ADM_VSM_RESULT_SHORT_FAILED == result) 
        {
            if (ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_TSP == g_eCmdIdType)
            {
                (DVOID)adm_vsm_exitTspEnv();
                vsmSetTspTriggerType(ADM_APPLICATION_COMMON_CMDID_TYPE_INVALID);
            }

            vsmStateSet(s, ADM_VSM_STATE_IDLE);

            if (ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_TSP == g_eCmdIdType)
            {
                (DVOID)adm_vsm_stopWakeUp();
            }
        } 
        else if (ADM_VSM_RESULT_FATAL_FAILED == result) 
        {
            if (ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_TSP == g_eCmdIdType)
            {
                (DVOID)adm_vsm_exitTspEnv();
                vsmSetTspTriggerType(ADM_APPLICATION_COMMON_CMDID_TYPE_INVALID);
            }

            vsmStateSet(s, ADM_VSM_STATE_IDLE);

            if (ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_TSP == g_eCmdIdType)
            {
                (DVOID)adm_vsm_stopWakeUp();
            }
        }
		else
		{
	        ;/*null*/
		}
    }
    (void)pthread_mutex_unlock(&s->mutex);
}
static void vsmStateResultDownload(ADM_VSM_STATE_T* s, ADM_VSM_RESULT_E result)
{
    ADM_APPLICATION_COMMON_CMDID_TYPE_E eTriggerType = ADM_APPLICATION_COMMON_CMDID_TYPE_INVALID;
    DBOOL bIsHasInstallTime = DFALSE;

    (void)pthread_mutex_lock(&s->mutex);
    if (ADM_VSM_STATE_DOWNLOAD_READY == s->state || ADM_VSM_STATE_DOWNLOAD_DOING == s->state) {
        if (ADM_VSM_RESULT_SUCCESS == result) {
            vsmStateSet(s, ADM_VSM_STATE_UPGRADE_READY);
            (void)pthread_mutex_unlock(&s->mutex);

            ADM_CVS_UPGRADE_MODE_E mode = ADM_CVS_UPGRADE_MODE_INVALID;
            D32S nResult = adm_cvs_getUpgradeMode((D32S*)&mode);
            if (nResult == ADM_ECODE_SUCCESSED) 
            {
                /** 静默或者强制是否设置了时间点 */
                bIsHasInstallTime = adm_dpm_getIsAnyInstallTime(DNULL);

                eTriggerType = adm_vsm_getTriggerType();
                switch (mode)
                {
                    case ADM_CVS_UPGRADE_MODE_USB_CHECK:
                        (DVOID)adm_vsm_triggerInstall();
                        break;
                    case ADM_CVS_UPGRADE_MODE_SLIENCE_INSTALL:
                    case ADM_CVS_UPGRADE_MODE_FORCE:
                        if (DFALSE == bIsHasInstallTime)
                        {
                            log_i("no time set.tigger install.");
                            (DVOID)adm_vsm_triggerInstall();
                        }
                        break;
                    case ADM_CVS_UPGRADE_MODE_RESCUE:
                        (DVOID)adm_vsm_triggerInstall();
                        break;
                    case ADM_CVS_UPGRADE_MODE_PROD_LINE:
                        (DVOID)adm_vsm_triggerInstall();
                        break;
                    case ADM_CVS_UPGRADE_MODE_APK_SELF:
                        (DVOID)adm_vsm_triggerInstall();
                        break;
                    default:
                        log_w("normal install mode are not met, mode[ %d ]",mode);
                        break;
                }

                // vsmDimSoaHostSetUpgradeMode(vsmUpgradeModeSwitch(mode));
            } else {
                log_w("self install mode not met, nResult[ %d ] mode[ %d ]", nResult, mode);
            }
            return;
        } else if (ADM_VSM_RESULT_SHORT_FAILED == result) {
            vsmStateSet(s, ADM_VSM_STATE_DOWNLOAD_PAUSE);
            // vsmDimSoaHostSetUpgradeMode(7);
        } else if (ADM_VSM_RESULT_FATAL_FAILED == result) {

            /** 下载失败，并且tsp推送任务,操作下电 */
            if (ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_TSP == g_eCmdIdType)
            {
                (DVOID)adm_vsm_exitTspEnv();
                vsmSetTspTriggerType(ADM_APPLICATION_COMMON_CMDID_TYPE_INVALID);
            }

            // vsmDimSoaHostSetUpgradeMode(7);
            vsmStateSet(s, ADM_VSM_STATE_IDLE);

            if (ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_TSP == g_eCmdIdType)
            {
                (DVOID)adm_vsm_stopWakeUp();
            }
        }
		else
		{
	        ;/*null*/
		}
    }
    (void)pthread_mutex_unlock(&s->mutex);
}
static void vsmStateResultInstall(ADM_VSM_STATE_T* s, ADM_VSM_RESULT_E result)
{
    D32S iRet = ADM_ECODE_FAILED;
    D32S errorCode = ADM_ECODE_FAILED;

    (void)pthread_mutex_lock(&s->mutex);
    if (ADM_VSM_STATE_UPGRADE_DOING == s->state) {
        if (ADM_VSM_RESULT_SUCCESS == result) {
            (DVOID)adm_vsm_stopWakeUp();

            vsmStateSet(s, ADM_VSM_STATE_IDLE);
            // vsmDimSoaHostSetUpgradeMode(7);
        } else if (ADM_VSM_RESULT_SHORT_FAILED == result) {
            /** 退出唤醒*/
            (DVOID)adm_vsm_stopWakeUp();

            vsmStateSet(s, ADM_VSM_STATE_UPGRADE_PAUSE);
            // vsmDimSoaHostSetUpgradeMode(7);
        } else if (ADM_VSM_RESULT_FATAL_FAILED == result) {
            (DVOID)adm_vsm_stopWakeUp();
            vsmStateSet(s, ADM_VSM_STATE_IDLE);
            // vsmDimSoaHostSetUpgradeMode(7);
        }
		else
		{
	        ;/*null*/
		}
    }

    if (ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_TSP == g_eCmdIdType)
    {
        vsmSetTspTriggerType(ADM_APPLICATION_COMMON_CMDID_TYPE_INVALID);
    }

    (void)pthread_mutex_unlock(&s->mutex);
}
static void vsmResumeTriggerTypeByUpgradeMode(ADM_CVS_UPGRADE_MODE_E mode)
{
    ADM_VSM_STATE_E eState = adm_vsm_stateGet();
    switch (mode)
    {
        case ADM_CVS_UPGRADE_MODE_SLIENCE_DOWNLOAD:
            if ((ADM_VSM_STATE_UPGRADE_DOING == eState) || (ADM_VSM_STATE_UPGRADE_READY == eState) || (ADM_VSM_STATE_UPGRADE_PAUSE == eState))
            {
                adm_vsm_setTriggerType(ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_USER);
            }
            else{
                adm_vsm_setTriggerType(ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_SILENCE);
            }
            break;

        case ADM_CVS_UPGRADE_MODE_SLIENCE_INSTALL:
            adm_vsm_setTriggerType(ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_SILENCE);
            break;

        case ADM_CVS_UPGRADE_MODE_USB_CHECK:
            adm_vsm_setTriggerType(ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_USB);
            break;

        case ADM_CVS_UPGRADE_MODE_RESCUE:
            adm_vsm_setTriggerType(ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_RESCUE);
            break;

        case ADM_CVS_UPGRADE_MODE_PROD_LINE:
            adm_vsm_setTriggerType(ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_PROD_LINE);
            break;

        case ADM_CVS_UPGRADE_MODE_APK_SELF:
            adm_vsm_setTriggerType(ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_APK_SELF_UPGRADE);
            break;

        case ADM_CVS_UPGRADE_MODE_FORCE:
            adm_vsm_setTriggerType(ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_FORCE);
            break;

        default:
            adm_vsm_setTriggerType(ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_USER);
            break;
    }

    return;
}

static void vsmStateResume(ADM_VSM_STATE_T *s)
{
    D64U ulAutoStartTime = 0;
    ADM_DPS_DOWNRESULT_T *pstResult = NULL;
    D32S iRet = ADM_ECODE_FAILED;

    if (0U != s->worker->count)
    {
        log_i("vsm worker working now ,no need resume");
        return;
    }

    ADM_CVS_UPGRADE_MODE_E mode = ADM_CVS_UPGRADE_MODE_INVALID;
    D32S nResult = adm_cvs_getUpgradeMode((D32S *)&mode);
    if (ADM_IS_SUCCESS(nResult))
    {
        vsmResumeTriggerTypeByUpgradeMode(mode);
    }

    if ((ADM_VSM_STATE_CHECK_DOING != s->state ||
        ADM_VSM_STATE_IDLE != s->state) && 
        ADM_CVS_UPGRADE_MODE_USB_CHECK != mode)
    {
        adm_vsm_waitCheckSubedEventSuccessed(ADM_UIM_INSTALL_SERVICE_ID, ADM_UIM_TSP_SCHDULE_TIME_EVENT_ID);
    }

    /** 强制恢复 */
    adm_vsm_forceResume(s->state, DTRUE);

    (void)pthread_mutex_lock(&s->mutex);
    if (ADM_VSM_STATE_CHECK_DOING == s->state)
    {
        vsmStateSet(s, ADM_VSM_STATE_IDLE);
    }
    else if (ADM_VSM_STATE_DOWNLOAD_READY == s->state)
    {
        /** 恢复下载的触发类型 */
        if (ADM_APPLICATION_COMMON_CMDID_TYPE_INVALID == adm_dpm_getDownInfo()->eTriggerType)
        {
            adm_dpm_getDownInfo()->eTriggerType = adm_vsm_getTriggerType();
        }

        if ((ADM_ECODE_SUCCESSED == nResult)
            //!< APK 自升级等同于静默升级，所以在 DownloadReady的状态需要恢复下载 modi maxy 雄狮车型除了V23,用户点击的任务也是自动下载的
            && ((ADM_CVS_UPGRADE_MODE_SLIENCE_DOWNLOAD == mode) || (ADM_CVS_UPGRADE_MODE_SLIENCE_INSTALL == mode) || (ADM_CVS_UPGRADE_MODE_APK_SELF == mode) || (ADM_CVS_UPGRADE_MODE_FORCE == mode) ||
                (ADM_CVS_UPGRADE_MODE_PROD_LINE == mode) || ADM_CVS_UPGRADE_MODE_USER_CLICK == mode))
        {
            if (!vsmWorkerDownloadResume(s->worker))
            {
                log_w("resume download failed");
            }
        }

        /** U盘升级，不需要断电恢复，状态机回到IDLE */
        if ((ADM_ECODE_SUCCESSED == nResult) && (ADM_CVS_UPGRADE_MODE_USB_CHECK == mode))
        {
            pstResult = adm_dpm_createDownResultInfo(ADM_ECODE_DOWNLOAD_FAILED, NULL);
            nResult = adm_dpm_policyDownResultNotify2Uim(pstResult, "downloading");
            if (ADM_IS_ERR(nResult))
            {
                log_e("Failed to notify download policy result, err[%d]", nResult);
            }
            adm_dpm_freeDownResultInfo(pstResult);
            vsmStateSet(s, ADM_VSM_STATE_IDLE);
        }

        /** 常规模式，需要进行升级任务校验 */
        // if ((ADM_ECODE_SUCCESSED == nResult) && (ADM_CVS_UPGRADE_MODE_USER_CLICK == mode))
        // {
        //     iRet = adm_dpm_taskVerify();
        //     if (ADM_ECODE_UPGRADE_TASK_STOPED == iRet)
        //     {
        //         log_e("Task has been stopped, err[%d]", iRet);
        //         pstResult = adm_dpm_createDownResultInfo(iRet, NULL);
        //         nResult = adm_dpm_policyDownResultNotify2Uim(pstResult, "downloading");
        //         if (ADM_IS_ERR(nResult))
        //         {
        //             log_e("Failed to notify download policy result, err[%d]", nResult);
        //         }
        //         adm_dpm_freeDownResultInfo(pstResult);
        //         adm_vsm_stateSet(ADM_VSM_STATE_IDLE);
        //     }
        // }
    }
    else if (ADM_VSM_STATE_DOWNLOAD_DOING == s->state || ADM_VSM_STATE_DOWNLOAD_PAUSE == s->state)
    {
        /** 恢复下载的触发类型 */
        if (ADM_APPLICATION_COMMON_CMDID_TYPE_INVALID == adm_dpm_getDownInfo()->eTriggerType)
        {
            adm_dpm_getDownInfo()->eTriggerType = adm_vsm_getTriggerType();
        }

        /** U盘升级，不需要断电恢复，状态机回到IDLE */
        if ((ADM_ECODE_SUCCESSED == nResult) && (ADM_CVS_UPGRADE_MODE_USB_CHECK == mode))
        {
            pstResult = adm_dpm_createDownResultInfo(ADM_ECODE_DOWNLOAD_FAILED, NULL);
            nResult = adm_dpm_policyDownResultNotify2Uim(pstResult, "downloading");
            if (ADM_IS_ERR(nResult))
            {
                log_e("Failed to notify download policy result, err[%d]", nResult);
            }
            adm_dpm_freeDownResultInfo(pstResult);
            vsmStateSet(s, ADM_VSM_STATE_IDLE);
        }
        else
        {
            if (!vsmWorkerDownloadResume(s->worker))
            {
                log_w("resume download failed");
            }
        }
    }
    else if (ADM_VSM_STATE_UPGRADE_READY == s->state)
    {
        //!< APK 自升级等同于静默升级，所以在UpgradeReady 的状态需要恢复升级
        if ((ADM_ECODE_SUCCESSED == nResult) && 
        ((ADM_CVS_UPGRADE_MODE_SLIENCE_INSTALL == mode) || 
         (ADM_CVS_UPGRADE_MODE_APK_SELF == mode) || 
         (ADM_CVS_UPGRADE_MODE_FORCE == mode) ||
         (ADM_CVS_UPGRADE_MODE_RESCUE == mode) || 
         (ADM_CVS_UPGRADE_MODE_PROD_LINE == mode)))
        {
            /** 没有设置预约时间的情况下，才恢复升级 */
            if (FALSE == adm_dpm_getIsAnyInstallTime(&ulAutoStartTime))
            {
                if (!vsmWorkerInstallResume(s->worker))
                {
                    log_w("resume install failed");
                }
            }
        }
    }
    else if (ADM_VSM_STATE_UPGRADE_DOING == s->state /* || ADM_VSM_STATE_UPGRADE_PAUSE == s->state*/)
    {
        if (!vsmWorkerInstallResume(s->worker))
        {
            log_w("resume install failed");
        }
    }
    else
    {
        ; /*null*/
    }
    (void)pthread_mutex_unlock(&s->mutex);
}
static struct json_object* vsmStateJsonCreate(const ADM_VSM_STATE_T* s)
{
    struct json_object* root = json_object_new_object();
    if (NULL != root) {
        struct json_object* stateVal = json_object_new_int((int)s->state);
        if (NULL != stateVal) {
            (void)json_object_object_add(root, "state", stateVal);
        }
    }
    return root;
}
static void vsmStateJsonParse(ADM_VSM_STATE_T* s, const struct json_object* root)
{
    struct json_object* stateVal = json_object_object_get(root, "state");
    if (NULL != stateVal) {
        s->state = (ADM_VSM_STATE_E)json_object_get_int(stateVal);
    }
}
static void vsmStateKVLoad(ADM_VSM_STATE_T* s)
{
    ADM_KV_STORAGE_VALUE_TYPE_E eType = ADM_KV_STORAGE_VALUE_TYPE_INVALID;
    DCHAR* stateJsonStr = NULL;
    size_t stateJsonStrLen = 0;
    if (TRUE
        == adm_com_kvstorage_get_string(
            adm_core_service_getBusinessDatabaseDb(), ADM_VSM_STATE_KV_KEY, &stateJsonStr, &stateJsonStrLen, &eType)) {
        if (NULL != stateJsonStr) {
            struct json_object* stateVal = json_tokener_parse(stateJsonStr);
            if (NULL != stateVal) {
                vsmStateJsonParse(s, stateVal);
                (void)json_object_put(stateVal);
            }
            adm_free(stateJsonStr);
        }
    }
}
static void vsmStateKVStore(const ADM_VSM_STATE_T* s)
{
    struct json_object* root = vsmStateJsonCreate(s);
    if (NULL != root) {
        const DCHAR* stateJsonStr = json_object_to_json_string(root);
        if (NULL != stateJsonStr) {
            (void)adm_com_kvstorage_set_string(adm_core_service_getBusinessDatabaseDb(),
                ADM_VSM_STATE_KV_KEY,
                stateJsonStr,
                ADM_KV_STORAGE_VALUE_TYPE_STRING);
        }
        (void)json_object_put(root);
    }
}
static void vsmStateSet(ADM_VSM_STATE_T* s, ADM_VSM_STATE_E state)
{
    s->state = state;
    vsmStateKVStore(s);
    log_d("state:%s", vsmStateString(s->state));
}

/* ADM_VSM_XXM_T */
static void vsmUcmInit(ADM_VSM_XXM_T* x)
{
    // x->eTriggerType = ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_IGN;
    x->eTriggerType = vsmTriggerTypeKVLoad();
    return;
}
static void vsmUcmDeInit(ADM_VSM_XXM_T* x)
{}
static ADM_VSM_RESULT_E vsmUcmCheck(ADM_VSM_XXM_T* x)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    log_i("Entry vsmUcmCheck.");

    if (ADM_IS_NULL(x)) {
        log_e("Invalid input param");
        return ADM_VSM_RESULT_FATAL_FAILED;
    }

    if (ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_USB == x->eTriggerType)
    {
        iRet = adm_cvm_usbCheck(x->eTriggerType);
    }
    else 
    {
        iRet = adm_cvm_check(x->eTriggerType);
    }

    if (ADM_ECODE_SUCCESSED == iRet) {
        return ADM_VSM_RESULT_SUCCESS;
    } else {
        return ADM_VSM_RESULT_SHORT_FAILED;
    }
}
static ADM_VSM_RESULT_E vsmUcmDownload(ADM_VSM_XXM_T* x)
{
    D32S iRet = ADM_ECODE_SUCCESSED;

    log_i("Entry vsmUcmDownload.");

    if (ADM_IS_NULL(x)) {
        log_e("Invalid input param");
        return ADM_VSM_RESULT_FATAL_FAILED;
    }

    iRet = adm_dpm_triggerDownload(x->eTriggerType);
    if (ADM_IS_ERR(iRet)) {
        log_e("Failed to trigger download, err[%d]", iRet);
        return ADM_VSM_RESULT_FATAL_FAILED;
    }

    return ADM_VSM_RESULT_SUCCESS;
}
static ADM_VSM_RESULT_E vsmUcmDownloadResume(ADM_VSM_XXM_T* x)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    ADM_CVS_UPGRADE_MODE_E mode = ADM_CVS_UPGRADE_MODE_INVALID;
    ADM_APPLICATION_COMMON_CMDID_TYPE_E eTriggerType = ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_USER;

    if (ADM_IS_NULL(x)) {
        log_e("Invalid input param");
        return ADM_VSM_RESULT_FATAL_FAILED;
    }

    iRet = adm_cvs_getUpgradeMode((D32S*)&mode);
    if ((ADM_ECODE_SUCCESSED == iRet) && ((ADM_CVS_UPGRADE_MODE_SLIENCE_DOWNLOAD == mode)||(ADM_CVS_UPGRADE_MODE_SLIENCE_INSTALL == mode))) 
    {
        eTriggerType = ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_IGN;
        log_i("begin slience download, triggerType = [%d]", eTriggerType);
    }
    else if ((ADM_ECODE_SUCCESSED == iRet) && (ADM_CVS_UPGRADE_MODE_APK_SELF == mode))
    {
        eTriggerType = ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_APK_SELF_UPGRADE;
        log_i("begin slience download, triggerType = [%d]", eTriggerType);
    }
    else
    {
        eTriggerType = x->eTriggerType;
        log_i("triggerType = [%d]", eTriggerType);
    }

    // TODO: 恢复下载时触发类型应该从kv中恢复，目前仅支持车机和静默下载
    // TODO: 所以此处先简单恢复一下
    iRet = adm_dpm_triggerDownload(eTriggerType);
    if (ADM_IS_ERR(iRet)) {
        log_e("Failed to trigger download, err[%d]", iRet);
        return ADM_VSM_RESULT_FATAL_FAILED;
    }

    return ADM_VSM_RESULT_SUCCESS;
}

/*!
 *******************************************************************************
 * 安装结果处理：对部分错误码设置为ADM_VSM_RESULT_SHORT_FAILED，以便设置状态机状态为安装中止
 *
 * \note 
 *
 * \param[in]   iError      安装结果
 * 
 * \retval      ADM_VSM_RESULT_E  封装后的状态机结果
 *******************************************************************************
 */
static ADM_VSM_RESULT_E vsmInstallResultHandle(D32S iError)
{
	ADM_VSM_RESULT_E eRet = ADM_VSM_RESULT_FATAL_FAILED;
    ADM_APPLICATION_COMMON_CMDID_TYPE_E eCmdidType = ADM_APPLICATION_COMMON_CMDID_TYPE_INVALID;

    eCmdidType = adm_vsm_getTriggerType();
	
	switch (iError)
	{
		case ADM_ECODE_SUCCESSED:
			eRet = ADM_VSM_RESULT_SUCCESS;
			break;
        case ADM_ECODE_INSTALL_CONDITION_FAILED:			
		case ADM_ECODE_INSTALL_ENTER_OTAMODE_FAILED:
		case ADM_ECODE_INSTALL_TASK_VERIFY_NET_FAILED:
		case ADM_ECODE_INSTALL_ENTER_BOOTMODE_FAILED:
		case ADM_ECODE_INSTALL_GET_ECU_VERSION_FAILED:	
        case ADM_ECODE_INSTALL_FAILED_MODE1:
        case ADM_ECODE_INSTALL_FAILED_MODE2:
        case ADM_ECODE_INSTALL_FAILED_MODE3:
        case ADM_ECODE_INSTALL_FAILED_MODE4:
			eRet = ADM_VSM_RESULT_SHORT_FAILED;
			break;
		default:
			eRet = ADM_VSM_RESULT_FATAL_FAILED;
			break;
	}

    if (ADM_VSM_RESULT_SHORT_FAILED == eRet)
    {
        /** 部分升级模式需要置状态机为idel，重新走检测流程 */
        if (ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_SILENCE == eCmdidType || 
            ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_RESCUE == eCmdidType || 
            ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_PROD_LINE == eCmdidType || 
            ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_APK_SELF_UPGRADE == eCmdidType || 
            ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_FORCE == eCmdidType ||
            ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_USB == eCmdidType)
        {
            log_i("triggerType[%d] Need turn to idel state.", eCmdidType);
            eRet = ADM_VSM_RESULT_FATAL_FAILED;
        }
    }

	return eRet;
}


static ADM_VSM_RESULT_E vsmUcmInstallInternal(ADM_VSM_XXM_T* x)
{
    if (ADM_IS_NULL(x)) {
        log_e("Invalid input VSM param");
        return ADM_VSM_RESULT_FATAL_FAILED;
    }

    D32S ret = adm_ipm_install(x->eTriggerType);
    
    return vsmInstallResultHandle(ret);
}
static ADM_VSM_RESULT_E vsmUcmInstall(ADM_VSM_XXM_T* x)
{
    return vsmUcmInstallInternal(x);
}
static ADM_VSM_RESULT_E vsmUcmInstallResume(ADM_VSM_XXM_T* x)
{
    return vsmUcmInstallInternal(x);
}

/* ADM_VSM_STATE_E */
static const DCHAR* vsmStateString(ADM_VSM_STATE_E state)
{
    const DCHAR* strstate = "ADM_VSM_STATE_unknown";
    switch (state) {
    case ADM_VSM_STATE_IDLE:
        strstate = ADM_STR_FROM_PARAM(ADM_VSM_STATE_IDLE);
        break;
    case ADM_VSM_STATE_CHECK_DOING:
        strstate = ADM_STR_FROM_PARAM(ADM_VSM_STATE_CHECK_DOING);
        break;
    case ADM_VSM_STATE_DOWNLOAD_READY:
        strstate = ADM_STR_FROM_PARAM(ADM_VSM_STATE_DOWNLOAD_READY);
        break;
    case ADM_VSM_STATE_DOWNLOAD_DOING:
        strstate = ADM_STR_FROM_PARAM(ADM_VSM_STATE_DOWNLOAD_DOING);
        break;
    case ADM_VSM_STATE_DOWNLOAD_PAUSE:
        strstate = ADM_STR_FROM_PARAM(ADM_VSM_STATE_DOWNLOAD_PAUSE);
        break;
    case ADM_VSM_STATE_UPGRADE_READY:
        strstate = ADM_STR_FROM_PARAM(ADM_VSM_STATE_UPGRADE_READY);
        break;
    case ADM_VSM_STATE_UPGRADE_DOING:
        strstate = ADM_STR_FROM_PARAM(ADM_VSM_STATE_UPGRADE_DOING);
        break;
    case ADM_VSM_STATE_UPGRADE_PAUSE:
        strstate = ADM_STR_FROM_PARAM(ADM_VSM_STATE_UPGRADE_PAUSE);
        break;
    case ADM_VSM_STATE_UPGRADE_BROKEN:
        strstate = ADM_STR_FROM_PARAM(ADM_VSM_STATE_UPGRADE_BROKEN);
        break;
    default:
        break;
    }
    return strstate;
}

static void adm_vsm_CheckVersion(ADM_APPLICATION_COMMON_CMDID_TYPE_E cmdId)
{
    log_i("adm_vsm_CheckVersion enter");
    D32S retVal = ADM_ECODE_FAILED;
    ADM_OCS_CHECK_RESP_ADAPTER_T* resp = NULL;

    retVal = adm_cvm_checkVersionAdaptorNotify(cmdId, NULL, NULL, &resp);
    if (ADM_ECODE_SUCCESSED == retVal || ADM_ECODE_NO_NEW_VERSION == retVal
        || ADM_ECODE_CHECK_LATEST_VERSION == retVal) {
        log_i("check success");
    } else {
        log_e("check error[%d]", retVal);
    }
    (void)adm_cvm_asyncVersionNumber(cmdId, NULL);
    adm_ocs_deleteCheckRespAdapterBody(resp);
    log_i("adm_vsm_CheckVersion exit");
}

D32S adm_vsm_dimSoaVdiSignalEventNotify(const ADM_DIM_SOA_VDI_VEHICLE_SIGNAL_TRIGGER_TYPE_E eSignalEventType)
{
    if (VEHICLE_EVENT_POWER_ON == eSignalEventType || VEHICLE_EVENT_IGN_ON == eSignalEventType) {
        if (adm_vsm_stateGet() != ADM_VSM_STATE_UPGRADE_DOING) {
            log_i("Not in the installing state, resume task into checking");
            adm_vsm_CheckVersion(ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_IGN);
        }
    }
    return 0;
}

static D32S vsmGetPowerState(D32S* pIsPowerOn)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    D32S iPowerType = -1;
    D32S iErrCode = -1;
    ADM_DIM_SOA_MSGID_DATA_ARRAY_T stReqMsgData = {};
    ADM_DIM_SOA_MSGID_DATA_ARRAY_T* pstRespMsgData=NULL;

    stReqMsgData.msgIdArrayLength = 1;
    stReqMsgData.msgIdArray = (ADM_DIM_SOA_VDI_MSGID_T**)adm_malloc(sizeof(ADM_DIM_SOA_VDI_MSGID_T*) * (DSIZE)stReqMsgData.msgIdArrayLength);
    
    (void)adm_memset(stReqMsgData.msgIdArray, 0, sizeof(ADM_DIM_SOA_VDI_MSGID_T*) * stReqMsgData.msgIdArrayLength,  
            sizeof(ADM_DIM_SOA_VDI_MSGID_T*) * stReqMsgData.msgIdArrayLength);
    
    stReqMsgData.msgIdArray[0] = (ADM_DIM_SOA_VDI_MSGID_T*)adm_malloc((DSIZE)sizeof(ADM_DIM_SOA_VDI_MSGID_T));

    log_i("begin request readVehicleStatus.");
    stReqMsgData.msgIdArray[0]->msgId  = VEHICLE_STATUS_POWER_SIGNAL;
    stReqMsgData.msgIdArray[0]->msgIdData = NULL;

    /** 判断是否非下电状态； 否则，退出升级 */
    iRet = adm_dim_host_readVehicleStatus(adm_soa_getHostInstanceId(), 15, &stReqMsgData, &pstRespMsgData, &iErrCode);
    if (ADM_IS_ERR(iRet) || ADM_IS_ERR(iErrCode))
    {
        /** 由触发源控制是否重试 */
        log_e("adm_dim_host_readVehicleStatus call error.iRet[%d], errCode[%d]", iRet, iErrCode);
        adm_free(stReqMsgData.msgIdArray[0]);
        adm_free(stReqMsgData.msgIdArray);
        iRet = ADM_ECODE_FAILED;
        return iRet;
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

            /** 当前如果是PowerOn 状态则不需要执行升级 */
            if (POWER_COMFORTABLE == iPowerType || OPWER_ON == iPowerType)
            {
                log_w("iPowerType = %d is not PowerOff state, do not need to trigger install", iPowerType);
                *pIsPowerOn = 1;  // 1- 上电
                break;
            }
            else
            {
                log_i("iPowerType = %d is PowerOn state, do exec trigger install now", iPowerType);
                *pIsPowerOn = 0;   // 0- 下电
                break;
            }
        }while (FALSE);

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

    return iRet;
}

D32S adm_vsm_dimSoaHostSignalEventNotify(const ADM_DIM_SOA_HOST_SIGNAL_EVENT_TYPE_E eSignalEventType, void* priv)
{
    static DBOOL s_bIsTriggerCheck = DFALSE;
    static DBOOL s_bIsHandleSchdule = DFALSE;

    log_i("Signal event[%d] has arrived.", eSignalEventType);

    adm_vsm_waitCheckSubedEventSuccessed(ADM_UIM_INSTALL_SERVICE_ID, ADM_UIM_TSP_SCHDULE_TIME_EVENT_ID);

    switch (eSignalEventType)
    {
        case ADM_DIM_SIGNAL_EVENT_TYPE_SCHDULE_TIME_ARRIVED:
            if (s_bIsHandleSchdule)
            {
                log_i("schdule arrived time running, ignore this event");
                return ADM_ECODE_SUCCESSED;
            }
            s_bIsHandleSchdule = DTRUE;
            adm_vsm_schduleTimeArrived(priv);
            s_bIsHandleSchdule = DFALSE;
            break;
        case ADM_DIM_SIGNAL_EVENT_TYPE_POWER_ON:
            if (s_bIsTriggerCheck)
            {
                log_i("current check is running, ignore this event");
                return ADM_ECODE_SUCCESSED;
            }
            s_bIsTriggerCheck = TRUE;
            adm_vsm_powerOnArrived();
            s_bIsTriggerCheck = FALSE;
            break;
        case ADM_DIM_SIGNAL_EVENT_GEAR_P:
            adm_vsm_gearPArrived();
            break;
        case ADM_DIM_SIGNAL_EVENT_GEAR_NONE_P:
            adm_vsm_gearNonePArrived();
            break;
        case ADM_DIM_SIGNAL_EVENT_ANTI_THEFT_MODE_ON:
            break;
        case ADM_DIM_SIGNAL_EVENT_ANTI_THEFT_MODE_OFF:
            break;
        case ADM_DIM_SIGNAL_EVENT_LFH_DOOR_OPEN:
            break;
        case ADM_DIM_SIGNAL_EVENT_LFH_DOOR_CLOSE:
            break;
        default:
            break;
    }

    return ADM_ECODE_SUCCESSED;
}

D32S adm_vsm_dimSoaHostTspRRMessageNotify(const ADM_DIM_SOA_TSP_OPERATION_TYPE_E eOperationType, D64S iAppointmentTimestamp, void* priv)
{
    ADM_UIM_SOA_TSP_OPERATION_TYPE_E eUimTspOperationType = (ADM_UIM_SOA_TSP_OPERATION_TYPE_E)eOperationType;

    log_i("adm_vsm_dimSoaHostTspRRMessageNotify begin");

    if (adm_vsm_getHmiOnline()) {
        log_i("Hmi is online, notify HMI APP operationType:%d, appointmentTimestamp:%d", eOperationType, iAppointmentTimestamp);
        adm_uim_tspSchduleTime(eUimTspOperationType, iAppointmentTimestamp);
    }
    
    log_i("adm_vsm_dimSoaHostTspRRMessageNotify end");

    return 0;
}

D32S adm_vsm_dimSoaHostTriggerEventNotify(const ADM_DIM_SOA_HOST_TRIGGER_EVENT_TYPE_E eTriggerEventType, void* priv)
{
    ADM_VSM_STATE_E state = adm_vsm_stateGet();
    if (ADM_DIM_SOA_HOST_TRIGGER_EVENT_TYPE_CHECK == eTriggerEventType
        && state != ADM_VSM_STATE_UPGRADE_DOING) {
        adm_vsm_CheckVersion(ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_TSP);
    }
    return 0;
}
void* adm_vsm_dimSoaHostPrivGet(void)
{
    /* TODO */
    return NULL;
}

static void* vsmTspDealAsync(DVOID* argv)
{
    D32S iRet = ADM_ECODE_FAILED;
    ADM_DIM_SOA_HOST_TSP_TASK_T* pstTspTask = (ADM_DIM_SOA_HOST_TSP_TASK_T*)argv;
    DCHAR *pcVerifyResult = NULL;
    D32S iErrorCode = ADM_ECODE_FAILED;
    DBOOL bIsVehicleStateReport = DFALSE;
    D32U uiRetryCnt = 0;
    DBOOL bIsFuel = DFALSE;  // 【新增】用于获取动力类型

    if (DNULL == pstTspTask)
    {
        iRet = ADM_ECODE_INVALID_PARAM;
        log_e("adm_vsm_dimSoaHostTspTaskDeal tsp input param is nil, iRet:%d", iRet);
        g_bIsTspDealing = DFALSE;
        return DNULL;
    }

    /** 车端控制：tsp推送的立即、 USB、救援都不允许执行*/
    if ((
            1 != pstTspTask->iSource && 
            (ADM_DIM_SOA_TSP_ACTIVE_UPGRADE == pstTspTask->eTaskType ||
            ADM_DIM_SOA_TSP_SLIENT_DOWNLOAD == pstTspTask->eTaskType)
        ) ||
        ADM_DIM_SOA_TSP_USB_UPGRADE == pstTspTask->eTaskType ||
        ADM_DIM_SOA_TSP_RESCUE_UPGRADE == pstTspTask->eTaskType)
    {
        log_i("source[%d] eTaskType:%d do not execute.", pstTspTask->iSource, pstTspTask->eTaskType);
        adm_vsm_reportState(pstTspTask->ulTaskId, ADM_IRM_REPORT_SERVER_STATUS_TSP_Task_Not_Match, ADM_ECODE_SUCCESSED, NULL);
        adm_free(pstTspTask);

        sleep(5);

        g_bIsTspDealing = DFALSE;

        iRet = adm_dim_host_execOtaTask(adm_soa_getHostTboxInstanceId(), 5, ADM_DIM_SOA_HOST_OTA_TASK_TYPE_STOP_WAKE_UP, 0, &iErrorCode);
        
        return DNULL;
    }

    log_i("[ABUP-Procedure] trigger tsp eTaskType = %d", pstTspTask->eTaskType);
    adm_vsm_waitCheckSubedEventSuccessed(ADM_UIM_INSTALL_SERVICE_ID, ADM_UIM_TSP_SCHDULE_TIME_EVENT_ID);

    /** tsp任务 静默下载静默安装，强制升级，OTA自升级，产线升级 前置条件校验 */
    if (ADM_DIM_SOA_TSP_SLIENT_UPGRADE == pstTspTask->eTaskType ||
        ADM_DIM_SOA_TSP_PRODUCT_LINE_UPGRADE == pstTspTask->eTaskType ||
        ADM_DIM_SOA_TSP_APK_SELF_UPGRADE == pstTspTask->eTaskType ||
        ADM_DIM_SOA_TSP_FORCE_UPGRADE == pstTspTask->eTaskType)
    {

        /** tsp推送检测需等待车辆状态上报，超时时间2min */
        while (uiRetryCnt < 24)
        {
            bIsVehicleStateReport = adm_irs_getHasVehicleStateMessage();
            if (DFALSE == bIsVehicleStateReport)
            {
                log_i("All Vehicle state has reported.");
                break;
            }
            uiRetryCnt++;
            sleep(5);
        }

        // 【修改】获取动力类型并检查
        iRet = adm_vsm_getIsFuel(&bIsFuel);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Get vehicle power type failed, iRet:%d", iRet);
            adm_vsm_reportState(pstTspTask->ulTaskId, ADM_IRM_REPORT_SERVER_STATUS_TSP_Conditions_Not_Met, ADM_ECODE_FAILED, "Invalid power type");
            adm_free(pcVerifyResult);
            adm_free(pstTspTask);
            g_bIsTspDealing = DFALSE;
            return DNULL;
        }

        if (DTRUE == bIsFuel)
        {
            log_i("ICE Vechile TSP condition Verify start");
            if (0 != adm_ipm_remotePowerOnConditionVerify(ADM_IPM_CONDITION_FILE_PATH_ICE, &pcVerifyResult))
            {
                adm_vsm_reportState(pstTspTask->ulTaskId, ADM_IRM_REPORT_SERVER_STATUS_TSP_Conditions_Not_Met, ADM_ECODE_SUCCESSED, pcVerifyResult);
                adm_free(pcVerifyResult);
                adm_free(pstTspTask);

                sleep(5);

                g_bIsTspDealing = DFALSE;

                iRet = adm_dim_host_execOtaTask(adm_soa_getHostTboxInstanceId(), 5, ADM_DIM_SOA_HOST_OTA_TASK_TYPE_STOP_WAKE_UP, 0, &iErrorCode);
                iRet = (iRet == ADM_ECODE_SUCCESSED) ? iErrorCode : iRet;
                log_d("[ABUP-Procedure] tsp task condition do not met, iRet:%d", iRet);
                return DNULL;
            }
            log_i("ICE Vechile TSP condition Verify end");
        }
        else
        {
            if (0 != adm_ipm_remotePowerOnConditionVerify(ADM_IPM_CONDITION_FILE_PATH, &pcVerifyResult))
            {
                adm_vsm_reportState(pstTspTask->ulTaskId, ADM_IRM_REPORT_SERVER_STATUS_TSP_Conditions_Not_Met, ADM_ECODE_SUCCESSED, pcVerifyResult);
                adm_free(pcVerifyResult);
                adm_free(pstTspTask);

                sleep(5);

                g_bIsTspDealing = DFALSE;

                iRet = adm_dim_host_execOtaTask(adm_soa_getHostTboxInstanceId(), 5, ADM_DIM_SOA_HOST_OTA_TASK_TYPE_STOP_WAKE_UP, 0, &iErrorCode);
                iRet = (iRet == ADM_ECODE_SUCCESSED) ? iErrorCode : iRet;
                log_d("[ABUP-Procedure] tsp task condition do not met, iRet:%d", iRet);
                return DNULL;
            }
        }
    }

    adm_cvm_setTspTaskId(pstTspTask->ulTaskId);
    switch (pstTspTask->eTaskType)
    {
    case ADM_DIM_SOA_TSP_ACTIVE_UPGRADE:        ///!< 主动升级
        iRet = adm_vsm_tspActionUpgrade(pstTspTask->ulTaskId, pstTspTask->eOperationType);
        break;
    case ADM_DIM_SOA_TSP_SLIENT_DOWNLOAD:       ///!< 静默下载常规安装
        iRet = adm_vsm_tspActionUpgrade(pstTspTask->ulTaskId, pstTspTask->eOperationType);
        break;
    case ADM_DIM_SOA_TSP_SLIENT_UPGRADE:        ///!< 静默下载静默安装
        // vsmDimSoaHostSetUpgradeMode(vsmUpgradeModeSwitch(ADM_CVM_UPGRADE_MODE_SLIENCE_INSTALL));
        iRet = adm_vsm_tspSlientUpgrade(pstTspTask->ulTaskId);
        break;
    // case ADM_DIM_SOA_TSP_USB_UPGRADE:           ///!< USB 升级
    //     iRet = adm_vsm_tspUsbUpgrade();
    //     break;
    // case ADM_DIM_SOA_TSP_RESCUE_UPGRADE:        ///!< 救援升级
    //     iRet = adm_vsm_tspRescueUpgrade(pstTspTask->ulTaskId);
    //     break;
    case ADM_DIM_SOA_TSP_PRODUCT_LINE_UPGRADE:  ///!< 产线升级
        iRet = adm_vsm_tspProductLineUpgrade(pstTspTask->ulTaskId);
        break;
    case ADM_DIM_SOA_TSP_APK_SELF_UPGRADE:      ///!< APK 自升级
        // vsmDimSoaHostSetUpgradeMode(vsmUpgradeModeSwitch(ADM_CVM_UPGRADE_MODE_APK_SELF)); 
        iRet = adm_vsm_tspApkSelfUpgrade(pstTspTask->ulTaskId);
        break;
    case ADM_DIM_SOA_TSP_FORCE_UPGRADE:         ///!< 强制升级
        // vsmDimSoaHostSetUpgradeMode(vsmUpgradeModeSwitch(ADM_CVM_UPGRADE_MODE_FORCE));  
        iRet = adm_vsm_tspForceUpgrade(pstTspTask->ulTaskId);
        break;
    case ADM_DIM_SOA_TSP_OTA_ONE_CLICK_RESCUE:  ///!< OTA 软件一键救援
        adm_vsm_tspOtaOneClickRescue();
        break;
    case ADM_DIM_SOA_TSP_OTA_LOG_REPORT:        ///!< OTA 日志上报
        iRet = adm_vsm_tspOtaLogReport();
        break;    
    case ADM_DIM_SOA_TSP_APPOINT_UPGRADE:       ///!< 预约升级
        iRet = adm_vsm_tspAppointUpgrade(pstTspTask->eOperationType, pstTspTask->appointmentTimestamp);
        break;
    default:
        /**防止消息处理过快*/
        sleep(1);
        log_e("eTaskType:%d is invalid", pstTspTask->eTaskType);
        break;
    }

    g_bIsTspDealing = DFALSE;
    adm_free(pstTspTask);
    return DNULL;
}

D32S adm_vsm_dimSoaHostTspTaskDeal(const ADM_DIM_SOA_HOST_TSP_TASK_T* pstTspTask)
{
    ADM_DIM_SOA_HOST_TSP_TASK_T* pstTspTemp = DNULL;
    D32S iRet = ADM_ECODE_FAILED;
    pthread_t id = 0;

    if (DTRUE == g_bIsTspDealing)
    {
        log_i("adm_vsm_dimSoaHostTspTaskDeal now, do not need to try again");
        return ADM_ECODE_SUCCESSED;
    }

    if (DNULL == pstTspTask)
    {
        log_e("Input param is nil");
        return ADM_ECODE_INVALID_PARAM;
    }

    pstTspTemp = (ADM_DIM_SOA_HOST_TSP_TASK_T*)adm_malloc(sizeof(ADM_DIM_SOA_HOST_TSP_TASK_T));
    pstTspTemp->appointmentTimestamp = pstTspTask->appointmentTimestamp;
    pstTspTemp->eOperationType = pstTspTask->eOperationType;
    pstTspTemp->eTaskType = pstTspTask->eTaskType;
    pstTspTemp->iSource = pstTspTask->iSource;
    pstTspTemp->ulTaskId = pstTspTask->ulTaskId;


    iRet = pthread_create(&id, DNULL, vsmTspDealAsync, (DVOID*)pstTspTemp);
    if (ADM_IS_SUCCESS(iRet))
    {
        g_bIsTspDealing = DTRUE;
        (DVOID)pthread_detach(id);
    }
    else
    {
        iRet = ADM_ECODE_PTHREAD_CREATE_FAILED;
        log_e("pthread_create failed, [err:%d]", iRet);
        adm_free(pstTspTemp);
    }

    return iRet;
}

const static DCHAR* g_isAutoDownloadKey = "[VSM]_[isAutoDownload]";
const static DCHAR* g_isAutoDownloadOnVal = "ON";
const static DCHAR* g_isAutoDownloadOffVal = "OFF";

void adm_vsm_hmiIsAutoDownload(DBOOL isAutoDownload, D32S *errorCode)
{
    if (TRUE == isAutoDownload) 
    {
        log_i("open auto download");
        if (FALSE
            == adm_com_kvstorage_set_string(adm_core_service_getBusinessDatabaseDb(),
                g_isAutoDownloadKey,
                g_isAutoDownloadOnVal,
                ADM_KV_STORAGE_VALUE_TYPE_STRING)) {
            *errorCode = ADM_ECODE_FAILED;
            log_e("set %d to kv,failed", g_isAutoDownloadKey);
        }else{
            *errorCode = ADM_ECODE_SUCCESSED;
        }
    } 
    else 
    {
        log_i("close auto download");
        if (FALSE
            == adm_com_kvstorage_set_string(adm_core_service_getBusinessDatabaseDb(),
                g_isAutoDownloadKey,
                g_isAutoDownloadOffVal,
                ADM_KV_STORAGE_VALUE_TYPE_STRING)) {
            *errorCode = ADM_ECODE_FAILED;
            log_e("set %d to kv,failed", g_isAutoDownloadKey);
        }else{
            *errorCode = ADM_ECODE_SUCCESSED;
        }
    }
    return;
}

static DBOOL vsmGetIsAutoDownload(void)
{
    ADM_KV_STORAGE_VALUE_TYPE_E eType = ADM_KV_STORAGE_VALUE_TYPE_INVALID;
    DCHAR* isAutoDownloadStr = NULL;
    size_t strLen = 0;
    DBOOL isAutoDownload = FALSE;

    if (TRUE
        == adm_com_kvstorage_get_string(
            adm_core_service_getBusinessDatabaseDb(), g_isAutoDownloadKey, &isAutoDownloadStr, &strLen, &eType)) {
        if (NULL != isAutoDownloadStr) {
            if (0 == adm_strcmp(isAutoDownloadStr, g_isAutoDownloadOnVal)) {
                log_i("get isAutoDownload TRUE");
                isAutoDownload = TRUE;
            } else {
                log_i("get isAutoDownload FALSE");
                isAutoDownload = FALSE;
            }
            adm_free(isAutoDownloadStr);
        }
    } else {
        log_i("get isAutoDownload failed,default FALSE");
        isAutoDownload = FALSE;
    }
    return isAutoDownload;
}

static void vsmStateDel(ADM_VSM_STATE_T* s)
{
    (void)pthread_mutex_lock(&s->mutex);
    vsmStateSet(s, ADM_VSM_STATE_IDLE);
    (void)pthread_mutex_unlock(&s->mutex);
}

/**
 * @brief 状态机清除，重归IDLE
 *
 */
void adm_vsm_stateDel(void)
{
    ADM_VSM_STATE_E state = adm_vsm_stateGet();
    if (ADM_VSM_STATE_UPGRADE_DOING != state && ADM_VSM_STATE_UPGRADE_PAUSE != state
        && ADM_VSM_STATE_UPGRADE_BROKEN != state) {
        log_i("[vsmState:%d]", state);
        adm_vsm_init();
        vsmStateDel(&m_manager.state);
    }

    return;
}

/**
 * @brief 强制恢复
 * @note 强制恢复（下电、退OTA模式、退网络维持、退3E80）
 * @param in ADM_VSM_STATE_E eState 状态机状态
 * @return void
 */
void adm_vsm_forceResume(ADM_VSM_STATE_E eState, DBOOL bIsExit3E80)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    D32S iErrorCode = ADM_ECODE_SUCCESSED;
    D32S iOtaModeStatus = 0;
    static DBOOL bIsHandleResume = DFALSE;

    ADM_COM_FUNC_ENTRY();

    if (DTRUE == bIsHandleResume)
    {
        log_w("Is in handle.");
        return;
    }

    bIsHandleResume = DTRUE;

    /** TSP任务，检测阶段就已经进入ota模式了 */
    if (DTRUE == adm_vsm_getTspTriggerChecking() || 
        ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_TSP == adm_vsm_getTspTriggerType())
    {
        log_i("Tsp task is running. [vsmState:%d].", eState);
        if (ADM_VSM_STATE_IDLE != eState && ADM_VSM_STATE_MAX != eState)
        {
            /**TSP推送任务 非IDEL/检测中，重新进入OTA模式*/
            (void)adm_vsm_entryOtaMode();

            bIsHandleResume = DFALSE;
            return;
        }
    }

    if (ADM_VSM_STATE_UPGRADE_DOING == eState)
    {
        log_w("[vsmState:%d]", eState);
        /**常规任务 安装中，重新进入OTA模式*/
        if (DTRUE == adm_ipm_QueryIsEntryOtaMode())
        {
            (void)adm_vsm_entryOtaMode();
        }

        bIsHandleResume = DFALSE;
        return ;
    }

    adm_vsm_waitCheckSubedEventSuccessed(ADM_UIM_INSTALL_SERVICE_ID, ADM_UIM_TSP_SCHDULE_TIME_EVENT_ID);

    /** 查询OTA模式状态 */
    iRet = adm_dim_host_getOtaModeStatus(adm_soa_getHostInstanceId(), 30, &iOtaModeStatus, &iErrorCode);
    
    /** 升级状态不在升级中，且处于OTA模式中 */
    if (ADM_IS_SUCCESS(iRet) && ADM_IS_SUCCESS(iErrorCode) && (1 == iOtaModeStatus))
    {
        /** 下高压 */
        iRet = adm_dim_host_setVehicleStatus(adm_soa_getHostInstanceId(),30, VEHICLE_CTRL_EXIT_HV_MODE, &iErrorCode);
        log_i("Switch to %d. instanceId[%u], iRet[%d], errcode[%d]", VEHICLE_CTRL_EXIT_HV_MODE, adm_soa_getHostInstanceId(), iRet, iErrorCode);

        /** 远程下电 */
        iRet = adm_dim_host_setPower(adm_soa_getHostInstanceId(), ADM_POWER_CONTROL_TIMEOUT, (D32U)ADM_DIM_SOA_HOST_OTA_TASK_TYPE_REMOTE_POWER_OFF, &iErrorCode);
        log_i("dim host set power off ret1[%d], errCode[%d]", iRet, iErrorCode);

        /** not active */
        iRet = adm_dim_host_setVehicleStatus(adm_soa_getHostInstanceId(), 30, (D32U)VEHICLE_CTRL_NO_ACTIVE_HV_MODE, &iErrorCode);

        /** 退出OTA模式 */
        iRet = adm_dim_host_setVehicleStatus(adm_soa_getHostInstanceId(), 30, VEHICLE_CTRL_EXIT_NORMAL_OTA_MODE, &iErrorCode);
        log_d("instanceId:%u, eVechielStatusType:%u, ret:%d, errorCode:%d", adm_soa_getHostInstanceId(), VEHICLE_CTRL_EXIT_NORMAL_OTA_MODE, iRet, iErrorCode);

        if (DTRUE == bIsExit3E80)
        {
            /** 退3E80 */
            iRet = adm_dim_vdi_setDiagTp(adm_soa_getVdiInstanceId(), 30, &iErrorCode);
            log_d("set Diag tp, instanceId:%u, ret:%d, errorCode:%d", adm_soa_getVdiInstanceId(), iRet, iErrorCode);
        }

        /** 退出唤醒 */
        iRet = adm_dim_host_execOtaTask(adm_soa_getHostTboxInstanceId(), 30, ADM_DIM_SOA_HOST_OTA_TASK_TYPE_STOP_WAKE_UP, 0, &iErrorCode);
        log_i("out of wake up ret[%d], errCode[%d]", iRet, iErrorCode);
    }

    bIsHandleResume = DFALSE;

    ADM_COM_FUNC_EXIT();

    return ;
}
