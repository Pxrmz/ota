#include "adm_cvm_powerOncheck.h"
#include "adm_vsm_impl.h"
#include "adm_ecs_rsConfig.h"
#include "adm_com_kvstorage.h"

#include "adm_com_errorCode.h"
#include "adm_typedefs.h"
#include "adm_cvm_log.h"
#include "adm_memory.h"
#include "adm_string.h"
#include "adm_irs_upload.h"
#include "adm_dim_host.h"
#include "adm_soa.h"

#include <sys/time.h>
#include <pthread.h>

static DBOOL g_bIsPowerOnCheck = DFALSE;

/*#########PowerOnCheckNum########*/
//每天上电检测版本的次数储存的key
static const DCHAR* g_sPowerOnCheckNumKvKey = "[CVM]_[PowerOnCheckNum]";
//上次检测日期
static const DCHAR* g_sLastTimeDateStrKvKey = "[CVM]_[LastTimeDateStrKvKey]";


static D32S cvmPthreadCreate(
    pthread_t *pThreadHandle,
    void *(*start_routine)(void *param),
    void *arg)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    D32S errNum = 0;
    pthread_attr_t stThreadAttr = {0};

    /* 获取线程属性句柄 */
    iRet = pthread_attr_init(&stThreadAttr);
    if (ADM_IS_ERR(iRet))
    {
        errNum = errno;
        log_e("pthread_attr_init failed. err[%d], strerror[%s]", iRet, strerror(errNum));
        return ADM_ECODE_PTHREAD_CREATE_FAILED;
    }

    /* 设置线程分离属性 */
    iRet = pthread_attr_setdetachstate(&stThreadAttr, PTHREAD_CREATE_DETACHED);
    if (ADM_IS_ERR(iRet))
    {
        errNum = errno;
        log_e("pthread_attr_setdetachstate failed. iRet[%u], strerror[%s]", iRet, strerror(errNum));
        (void)pthread_attr_destroy(&stThreadAttr);
        return ADM_ECODE_PTHREAD_CREATE_FAILED;
    }

    //!< 启下载任务线程
    iRet = pthread_create(pThreadHandle,
                          &stThreadAttr,
                          start_routine,
                          arg);
    if (ADM_IS_ERR(iRet))
    {
        errNum = errno;
        log_e("Failed to create thread, err[%d], strerror[%s]", iRet, strerror(errNum));
        (void)pthread_attr_destroy(&stThreadAttr);
        return ADM_ECODE_PTHREAD_CREATE_FAILED;
    }

    /* 销毁线程属性句柄 */
    (void)pthread_attr_destroy(&stThreadAttr);

    return iRet;
}

/*!
 *******************************************************************************
 * cvm模块 从kv获取每天上电检测版本的次数
 *
 * \note 从kv获取每天上电检测版本的次数
 *
 * \param[in]   pPowerOnCheckNumKvKey      每天上电检测版本的次数储存的key
 * \retval      D64S                       每天上电检测版本的次数
 * 
 *******************************************************************************
 */
static D64S adm_cvm_getPowerOnCheckNumFromKv(const DCHAR* pPowerOnCheckNumKvKey)
{
    DCHAR* pPowerOnCheckNumStr = NULL;
    size_t pPowerOnCheckNumStrLen = 0;
    ADM_KV_STORAGE_VALUE_TYPE_E eType;
    D64S iPowerOnCheckNum = 0;

    if(NULL == pPowerOnCheckNumKvKey)
    {
        return -1;
    }

    if (TRUE
        == adm_com_kvstorage_get_string(
            adm_core_service_getBusinessDatabaseDb(), pPowerOnCheckNumKvKey, &pPowerOnCheckNumStr, &pPowerOnCheckNumStrLen, &eType)) {
        (void)adm_stringToInt64(pPowerOnCheckNumStr, &iPowerOnCheckNum);
        log_i("get ipowerOnCheckNum success, pPowerOnCheckNumStr = %s , iPowerOnCheckNum = %d , value_type = %d",
            pPowerOnCheckNumStr,
            iPowerOnCheckNum,
            eType);
        adm_free(pPowerOnCheckNumStr);
    } else {
        log_e("get failed,init iPowerOnCheckNum = 0");
        iPowerOnCheckNum = 0;
    }
    return iPowerOnCheckNum;
}

/*!
 *******************************************************************************
 * cvm模块 向kv设置每天上电检测版本的次数
 *
 * \note 向kv设置每天上电检测版本的次数
 *
 * \param[in]   apowerOnCheckNumKvKey      每天上电检测版本的次数储存的key
 * \param[in]  ipowerOnCheckNum            每天上电检测版本的次数储存的key
 * \retval      DBOOL                     DTRUE成功，DFALSE失败
 * 
 *******************************************************************************
 */
static DBOOL adm_cvm_setpowerOnCheckNumToKv(const DCHAR* pPowerOnCheckNumKvKey, D64S iPowerOnCheckNum)
{
    ADM_KV_STORAGE_VALUE_TYPE_E eType = ADM_KV_STORAGE_VALUE_TYPE_UINT64;
    DCHAR aPwerOnCheckNumStr[64];

    (void)adm_memset(aPwerOnCheckNumStr, 0x00, 64, 64);
    (void)adm_snprintf(aPwerOnCheckNumStr, 64, "%lld", iPowerOnCheckNum);

    if (TRUE
        == adm_com_kvstorage_set_string(
            adm_core_service_getBusinessDatabaseDb(), pPowerOnCheckNumKvKey, aPwerOnCheckNumStr, eType)) {
        log_i("set iPowerOnCheckNum success, aPwerOnCheckNumStr = %s , iPowerOnCheckNum = %lld", aPwerOnCheckNumStr, iPowerOnCheckNum);
        return TRUE;
    } else {
        log_e("set iPowerOnCheckNum failed");
        return FALSE;
    }
}

/*!
 *******************************************************************************
 * cvm模块 监控到上电事件后的回调函数的实现函数
 * 
 * \note 1.从持久存储中读取“当天已上电检测版本计数”的值A
 *       2.读取车辆配置信息中的“每天上电检测版本的次数”值B
 *       3.比较A和B大小,A<B或A=B；触发检测动作，并将A+1,存储到持久存储中
 * 
 *******************************************************************************
 */
static void __adm_cvm_GetDateStr(DCHAR * pdate, const D32S size)
{
    struct timeval    stTv;
    struct tm         *pTm;

    if((NULL == pdate) || (size < DATE_STR_NUM))
    {
        log_e("get date failed, ptime = null!!!");
        return ;
    }
    gettimeofday(&stTv, NULL);
    pTm = localtime(&stTv.tv_sec);
    sprintf(pdate,"%04d%02d%02d", 1900+pTm->tm_year, 1+pTm->tm_mon, pTm->tm_mday);
}

/*!
 *******************************************************************************
 * cvm模块 向kv设置每天上电检测版本的次数
 *
 * \note 向kv设置每天上电检测版本的次数
 *
 * \param[in]   apowerOnCheckNumKvKey      每天上电检测版本的次数储存的key
 * \param[in]  ipowerOnCheckNum            每天上电检测版本的次数储存的key
 * \retval      DBOOL                     DTRUE成功，DFALSE失败
 * 
 *******************************************************************************
 */
static DBOOL adm_cvm_setDateStrToKv(const DCHAR* pDateKvKey, DCHAR * pDate)
{
    ADM_KV_STORAGE_VALUE_TYPE_E eType = ADM_KV_STORAGE_VALUE_TYPE_STRING;

    if((NULL == pDateKvKey) || (NULL == pDate))
    {
        return DFALSE;
    }

    if (TRUE
        == adm_com_kvstorage_set_string(
            adm_core_service_getBusinessDatabaseDb(), pDateKvKey, pDate, eType)) {
        log_i("set pDate success, pDate = %s", pDate);
        return TRUE;
    } else {
        log_e("set pDate failed");
        return FALSE;
    }
}

/*!
 *******************************************************************************
 * cvm模块 从kv获取每天上电检测版本的次数
 *
 * \note 从kv获取每天上电检测版本的次数
 *
 * \param[in]   pPowerOnCheckNumKvKey      每天上电检测版本的次数储存的key
 * \retval      D64S                       每天上电检测版本的次数
 * 
 *******************************************************************************
 */
static DBOOL adm_cvm_getDateStrFromKv(const DCHAR* pDateKvKey, DCHAR * pDate, D32S size)
{
    DCHAR* pDateStr = NULL;
    size_t pDateStrLen = 0;
    ADM_KV_STORAGE_VALUE_TYPE_E eType;

    if((NULL == pDateKvKey) || (NULL == pDate) || (size < DATE_STR_NUM))
    {
        return DFALSE;
    }

    if (TRUE
        == adm_com_kvstorage_get_string(
            adm_core_service_getBusinessDatabaseDb(), pDateKvKey, &pDateStr, &pDateStrLen, &eType)) {
        (void)adm_memcpy(pDate, pDateStr, pDateStrLen, size);
        log_i("get pDate success, pDate = %s, pDateStrLen = %ld, value_type = %d",
            pDate,
            pDateStrLen,
            eType);
        adm_free(pDateStr);
    } else {
        log_e("get failed,init pDate");
        __adm_cvm_GetDateStr(pDate, size);
        (void)adm_cvm_setDateStrToKv(pDateKvKey, pDate);
        log_i("get pDate success, pDate = %s, size = %d", pDate, size);
    }
    return DTRUE;
}

static void* cvmThreadTriggerCheck(void* pArgv)
{
    D32S iTboxRemoteDiaStatus = -1;
    D32S iErrorCode = ADM_ECODE_SUCCESSED;

    /** 一直等待tbox远程诊断完成才能进行检测（零件收集） */
    do
    {
        sleep(5);
        (DVOID)adm_dim_host_getTboxRemoteDiaStatus(adm_soa_getHostInstanceId(), 10, &iTboxRemoteDiaStatus, &iErrorCode);
    } while (iTboxRemoteDiaStatus != REMOTE_DIA_END_READ);


    adm_vsm_triggerCheck(ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_IGN);

    g_bIsPowerOnCheck = DFALSE;

    return NULL;
}

/*!
 *******************************************************************************
 * cvm模块 监控到上电事件后的回调函数的实现函数
 * 
 * \note 1.从持久存储中读取“当天已上电检测版本计数”的值A
 *       2.读取车辆配置信息中的“每天上电检测版本的次数”值B
 *       3.比较A和B大小,A<B或A=B；触发检测动作，并将A+1,存储到持久存储中
 * 
 *******************************************************************************
 */
static DVOID* __adm_cvm_PoweOnCheckVersion(DVOID* pArgv)
{
    log_i("__adm_dim_vdi_SpyPoweOnCheckVersion enter");
    D32S iRet = ADM_ECODE_FAILED;
    D64S iPowerOnCheckNum = 0;
    ADM_ECS_RSCONFIGINFO_T stRsCfgInfo;
    DCHAR aNowDate[DATE_STR_NUM+1]      = {0};
    DCHAR aLastTimeDate[DATE_STR_NUM+1] = {0};
    D32S iTboxRemoteDiaStatus = -1;
    D32S iErrorCode = ADM_ECODE_SUCCESSED;

    if (DTRUE == g_bIsPowerOnCheck)
    {
        log_i("Execing power-on check task.");
        return NULL;
    }
    g_bIsPowerOnCheck = DTRUE;

    //1.获取上次上电检测的日期和今天的日期做比较
    (void)adm_cvm_getDateStrFromKv(g_sLastTimeDateStrKvKey, aLastTimeDate, DATE_STR_NUM);
    __adm_cvm_GetDateStr(aNowDate, DATE_STR_NUM);
    log_i("aLastTimeDate = %s, aNowDate = %s", aLastTimeDate, aNowDate);

    //若两次日期不相等，更新日期，也初始化今天的上电次数

    if(0 != adm_memcmp(aLastTimeDate, aNowDate, DATE_STR_NUM))
    {
        log_i("Init aLastTimeDate and iPowerOnCheckNum");
        (void)adm_cvm_setDateStrToKv(g_sLastTimeDateStrKvKey, aNowDate);
        (void)adm_cvm_setpowerOnCheckNumToKv(g_sPowerOnCheckNumKvKey, iPowerOnCheckNum);
    }
    //若是相等，读取当天已上电检测版本计数
    else
    {
        //2.从持久存储中读取“当天已上电检测版本计数”的值A
        iPowerOnCheckNum = adm_cvm_getPowerOnCheckNumFromKv(g_sPowerOnCheckNumKvKey);
    }

    //3.读取车辆配置信息中的“每天上电检测版本的次数”值B
    iRet = adm_ecs_getConfigInfo(&stRsCfgInfo);
    if(ADM_ECODE_SUCCESSED != iRet)
    {
        log_e("adm_ecs_getConfigInfo failed");
        g_bIsPowerOnCheck = DFALSE;
        return NULL;
    }
    log_i("iPowerOnCheckNum = %lld, maxFireTrigger = %d", iPowerOnCheckNum + 1, 
                                        stRsCfgInfo.stGeneralCfg.maxFireTrigger);

    //4.比较A和B大小,A<B；触发检测动作，并将A+1,存储到持久存储中
    if(iPowerOnCheckNum < stRsCfgInfo.stGeneralCfg.maxFireTrigger)
    {
        log_i("Power On trigger start to check");

<<<<<<< HEAD
        /** 上电检测需等待车辆状态上报，超时时间2min */
        while (uiRetryCnt < 24)
        {
            bIsVehicleStateReport = adm_irs_getHasVehicleStateMessage();
            if (DFALSE == bIsVehicleStateReport)
            {
                log_i("All Vehicle state has reported.");
                break;
            }
            sleep(5);
        }

        if(adm_vsm_triggerCheck(ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_IGN))
=======
        (DVOID)adm_dim_host_getTboxRemoteDiaStatus(adm_soa_getHostInstanceId(), 10, &iTboxRemoteDiaStatus, &iErrorCode);
        if (iTboxRemoteDiaStatus != REMOTE_DIA_END_READ)
>>>>>>> 3db494fe0 (feat:[QRV29-59] 上电检测和安装前版本校验收集零件信息增加tbox远程诊断状态的判断.)
        {
            ++iPowerOnCheckNum;
            (void)adm_cvm_setpowerOnCheckNumToKv(g_sPowerOnCheckNumKvKey, iPowerOnCheckNum);
            log_i("Start thread check task, update check number.");

            pthread_t stThreadHandle = {0};
            cvmPthreadCreate(&stThreadHandle, cvmThreadTriggerCheck, NULL);
        }
        else 
        {
            if(adm_vsm_triggerCheck(ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_IGN))
            {
                ++iPowerOnCheckNum;
                (void)adm_cvm_setpowerOnCheckNumToKv(g_sPowerOnCheckNumKvKey, iPowerOnCheckNum);
                log_i("Power On trigger check successed");
            }

            g_bIsPowerOnCheck = DFALSE;
        }
    }
    else
    {
        g_bIsPowerOnCheck = DFALSE;
        log_i("iPowerOnCheckNum > maxFireTrigger, not trigger check");
    }
    
    log_i("__adm_dim_vdi_SpyPoweOnCheckVersion exit");

    g_bIsPowerOnCheck = DFALSE;

    return NULL;
}

/*!
 *******************************************************************************
 * cvm模块 监控到上电事件后的回调函数
 * 
 * \note 
 *
 * \param[in]      u64TaskId      监控任务id
 * \param[inout]   priv           预留
 * \retval         D32S           D32S    0: 成功 non-0：失败
 *******************************************************************************
 */
static D32S adm_cvm_VehiclePowerOnCallback(D64U u64TaskId, void *priv)
{
    if (DTRUE == g_bIsPowerOnCheck)
    {
        log_i("Execing power-on check task.");
        return ADM_ECODE_SUCCESSED;
    }
    g_bIsPowerOnCheck = DTRUE;

    pthread_t stThreadHandle = {0};
    cvmPthreadCreate(&stThreadHandle, __adm_cvm_PoweOnCheckVersion, NULL);

    // __adm_cvm_PoweOnCheckVersion();
    return ADM_ECODE_SUCCESSED;
}

/*!
 *******************************************************************************
 * cvm模块 监控到上电事件后的回调函数
 * 
 * \note 
 *
 * \retval         D32S           D32S    0: 成功 non-0：失败
 *******************************************************************************
 */
D32S adm_cvm_PoweOnCheckVersion()
{
    if (DTRUE == g_bIsPowerOnCheck)
    {
        log_i("Execing power-on check task.");
        return ADM_ECODE_SUCCESSED;
    }
    g_bIsPowerOnCheck = DTRUE;

    pthread_t stThreadHandle = {0};
    cvmPthreadCreate(&stThreadHandle, __adm_cvm_PoweOnCheckVersion, NULL);

    // __adm_cvm_PoweOnCheckVersion();
    return ADM_ECODE_SUCCESSED;
}

//检测上电事件的任务id
static D64U s_iSpyVPOTaskId = 0;
/*!
 *******************************************************************************
 * cvm模块 监控车辆上电事件
 *
 * \note 监控车辆上电事件,监控到后调用回调进行检测 
 * 
 *******************************************************************************
 */
void adm_dmi_vdiSpyAddVehiclePowerOn(void)
{
    D32S iRet = ADM_ECODE_FAILED;
    ADM_SYSI4VDI_ENGINE_SPY_ARRAY_T stSpy;
    (void)adm_memset(&stSpy, 0, sizeof(ADM_SYSI4VDI_ENGINE_SPY_ARRAY_T), sizeof(ADM_SYSI4VDI_ENGINE_SPY_ARRAY_T));
    stSpy.msgIdArrayLength = 1;
    stSpy.spyArray[0].vehicle_id = VEHICLE_EVENT_POWER_ON;

    iRet = adm_dim_vdiSpyRegisterSpyTask(&stSpy, adm_cvm_VehiclePowerOnCallback, &s_iSpyVPOTaskId);
    if(ADM_ECODE_SUCCESSED != iRet)
    {
        log_e("Spy Vehicle Power On failed, iRet:%d", iRet);
    } 
    else 
    {
        log_i("Spy Vehicle Power On successed");   
    }
}

/*!
 *******************************************************************************
 * cvm模块 取消监控车辆上电事件
 *
 * \note 取消监控车辆上电事件
 * 
 *******************************************************************************
 */
void adm_dmi_vdiSpyClearVehiclePowerOn(void)
{
    D32S iRet = ADM_ECODE_FAILED;
    if(0 == s_iSpyVPOTaskId)
    {
        log_e("Not spy vehicle power on");
        return ;
    }
    iRet = adm_dim_vdiSpyCancelSpyTask(s_iSpyVPOTaskId);
    if(ADM_ECODE_SUCCESSED != iRet)
    {
        log_e("Spy Clear Vehicle Power On failed, err:%d", iRet);
    } 
    else 
    {
        log_i("Spy Clear Vehicle Power On successed");   
    }
}