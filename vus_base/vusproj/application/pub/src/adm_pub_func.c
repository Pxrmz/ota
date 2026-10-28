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
#include "adm_pub_func.h"
#include "adm_list.h"
#include "adm_string.h"
#include "adm_memory.h"
#include "adm_dim_host.h"
#include "adm_dim_ucl.h"
#include "adm_com_lib.h"
#include "adm_com_modDefine.h"
#include "adm_com_errorCode.h"
#include "adm_cfg_api.h"
#include "adm_pub_keylog.h"

#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

//!< 同步请求超时时间 
#define ADM_PUB_SYNC_REQ_TIMEOUT (60)
//!< 解密并发的最大数
#define ADM_PUB_DECRYPT_MAX_NUM (3)
//!< 验签并发的最大数
#define ADM_PUB_VERIFYSIGN_MAX_NUM (3)

static ADM_PUB_HANDLE_T gstPubHandle = {0};
static ADM_DECRYPT_HANDLE_T gstDecryptHandle = {0};
static ADM_VERIFYSIGN_HANDLE_T gstVerifySignptHandle = {0};

/*!
 *******************************************************************************
 * PUB模块初始化.
 *
 * \note
 *
 * \param   none
 * \retval  D32S    0: 成功 non-0：失败
 *******************************************************************************
 */
D32S adm_pub_moduleInit(void)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    D32S errNum = 0;

    if (TRUE == gstPubHandle.isInited)
    {
        log_i("PUB module already init.");
        return ADM_ECODE_SUCCESSED;
    }
    
    do
    {
        //解密模块初始化
        iRet = adm_pub_decryptModuleInit();
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to init dps module, err[%d]", iRet);
            break;
        }

        //验签模块初始化
        iRet = adm_pub_VerifySignModuleInit();
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to init dps module, err[%d]", iRet);
            break;
        }
    
        iRet = pthread_rwlock_init(&gstPubHandle.stRWlock, NULL);
        if (ADM_IS_ERR(iRet))
        {
            errNum = errno;
            log_e("Failed to init rwlock, err[%d](%s)", ADM_ECODE_RWLOCK_INIT_FAILED, strerror(errNum));
            return ADM_ECODE_RWLOCK_INIT_FAILED;
        }

        adm_pub_keylogInit();
        
        gstPubHandle.isInited = TRUE;
    } while (FALSE);
    
    log_d("===> PUB init module ok.");

    return iRet;
}

/*!
 *******************************************************************************
 * pub模块销毁.
 *
 * \note
 *
 * \param   none
 * \retval  none
 *******************************************************************************
 */
void adm_pub_moduleDeInit(void)
{
    if (TRUE == gstPubHandle.isInited)
    {
        gstPubHandle.isInited = FALSE;

        //<! 解密模块反初始化
        adm_pub_decryptModuleDeInit();
        //<! 验签模块反初始化
        adm_pub_verifySignModuleDeInit();
        
        (void)pthread_rwlock_destroy(&gstPubHandle.stRWlock);

    }
    
    return;
}

/*!
 *******************************************************************************
 * 创建解密列表内存.
 *
 * \note 
 *
 * \retval      none
 *******************************************************************************
 */
static D32S pubCreateDecryptList(void)
{
    D32S i = 0;
    gstDecryptHandle.stDecryptList.pstDecryptAsyncInfo = (ADM_DECRYPT_ASYNC_S *)adm_malloc(sizeof(ADM_DECRYPT_ASYNC_S) * (D32U)ADM_PUB_DECRYPT_MAX_NUM);
    if (ADM_IS_NULL(gstDecryptHandle.stDecryptList.pstDecryptAsyncInfo))
    {
        log_e("Failed to malloc pstDecryptAsyncInfo");
        return ADM_ECODE_NO_MEMORY;
    }
    else
    {
        pthread_condattr_t cattr;
        pthread_condattr_init(&cattr);
        pthread_condattr_setclock(&cattr, CLOCK_MONOTONIC);

        for(i = 0; i < ADM_PUB_DECRYPT_MAX_NUM; i++)
        {
            gstDecryptHandle.stDecryptList.pstDecryptAsyncInfo[i].pcFileID = NULL;
            gstDecryptHandle.stDecryptList.pstDecryptAsyncInfo[i].pcDecryptStoragePath = NULL;
            gstDecryptHandle.stDecryptList.pstDecryptAsyncInfo[i].bHasResult = FALSE;
            gstDecryptHandle.stDecryptList.pstDecryptAsyncInfo[i].bIsFlush = FALSE;
            gstDecryptHandle.stDecryptList.pstDecryptAsyncInfo[i].bIsUsing = FALSE;
            gstDecryptHandle.stDecryptList.pstDecryptAsyncInfo[i].stResult.result = ADM_ECODE_SUCCESSED;
            (void)pthread_mutex_init(&gstDecryptHandle.stDecryptList.pstDecryptAsyncInfo[i].stMutex, NULL);
            (void)pthread_cond_init(&gstDecryptHandle.stDecryptList.pstDecryptAsyncInfo[i].stCond, &cattr);
        }
    }

    return ADM_ECODE_SUCCESSED;
}

/*!
 *******************************************************************************
 * 解密模块初始化.
 *
 * \note
 *
 * \param   none
 * \retval  D32S    0: 成功 non-0：失败
 *******************************************************************************
 */
D32S adm_pub_decryptModuleInit(void)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    D32S errNum = 0;

    if (TRUE == gstDecryptHandle.isInited)
    {
        log_i("decrypt module already init.");
        return ADM_ECODE_SUCCESSED;
    }

    iRet = pthread_rwlock_init(&gstDecryptHandle.stRWlock, NULL);
    if (ADM_IS_ERR(iRet))
    {
        iRet = ADM_ECODE_RWLOCK_INIT_FAILED;
        errNum = errno;
        log_e("Failed to init rwlock, err[%d](%s)", ADM_ECODE_RWLOCK_INIT_FAILED, strerror(errNum));
        return iRet;
    }
    
    //!< 解密异步响应信息
    gstDecryptHandle.stDecryptList.pstDecryptAsyncInfo = NULL;
    gstDecryptHandle.stDecryptList.s32Cnt = 0;
    iRet = pubCreateDecryptList();
    if (ADM_IS_ERR(iRet))
    {
        log_e("init decrypt module failed, err[%d]", iRet);
        return iRet;
    }
    gstDecryptHandle.isInited = TRUE;
    
    log_d("===> decrypt init module ok.");

    return iRet;
}

/*!
 *******************************************************************************
 * 销毁解密异步信息
 *
 * \note 释放 pstDecryptAsyncInfo 内存
 * 
 * \param[in]  pstDecryptAsyncInfo   解密异步信息
 * \retval     none
 *******************************************************************************
 */
static void pubDestroyDecryptAsyncInfo(ADM_DECRYPT_ASYNC_S *pstDecryptAsyncInfo)
{
    if (ADM_IS_NULL(pstDecryptAsyncInfo))
    {
        return;
    }
    
    adm_free(pstDecryptAsyncInfo->pcFileID);
    adm_free(pstDecryptAsyncInfo->stResult.realVal);
    adm_free(pstDecryptAsyncInfo->stResult.expectVal);
    adm_free(pstDecryptAsyncInfo->pcDecryptStoragePath);
    (void)pthread_mutex_destroy(&pstDecryptAsyncInfo->stMutex);
    (void)pthread_cond_destroy(&pstDecryptAsyncInfo->stCond);

    return;
}

/*!
 *******************************************************************************
 * 清空解密列表内存.
 *
 * \note 
 *
 * \retval      none
 *******************************************************************************
 */
static void pubDestroyDecryptList(void)
{
    D32S i = 0;

    //!< 模块未初始化，则直接报错
    if (FALSE == gstDecryptHandle.isInited)
    {
        return;
    }
    (void)pthread_rwlock_wrlock(&gstDecryptHandle.stRWlock);
    for(i = 0; i < ADM_PUB_DECRYPT_MAX_NUM ; i++)
    {
        pubDestroyDecryptAsyncInfo(&gstDecryptHandle.stDecryptList.pstDecryptAsyncInfo[i]);
    }
    gstDecryptHandle.stDecryptList.pstDecryptAsyncInfo = NULL;

    (void)pthread_rwlock_unlock(&gstDecryptHandle.stRWlock);

    return;
}


/*!
 *******************************************************************************
 * 解密模块销毁.
 *
 * \note
 *
 * \param   none
 * \retval  none
 *******************************************************************************
 */
void adm_pub_decryptModuleDeInit(void)
{
    if (TRUE == gstDecryptHandle.isInited)
    {
        /** 销毁解密列表 */
        pubDestroyDecryptList();

        (void)pthread_rwlock_wrlock(&gstDecryptHandle.stRWlock);
        gstDecryptHandle.stDecryptList.s32Cnt = 0;
        gstDecryptHandle.isInited = FALSE;
        (void)pthread_rwlock_unlock(&gstDecryptHandle.stRWlock);
        (void)pthread_rwlock_destroy(&gstDecryptHandle.stRWlock);
        
    }

    return;
}

/*!
 *******************************************************************************
 * 进程等待信号函数
 *
 * \note
 *
 * \param[in]  pstCond   信号量
 * \param[in]  pstMutex  信号量锁
 * \param[in]  pIsSignal 信号是否被主动打断
 * \param[in]  pIsFlush 等待时间是否需要刷新
 * \param[in]  ulAbsTime 等待信号量超时时间(单位毫秒)
 * \retval     D32S
 *******************************************************************************
 */
static D32S adm_pub_ThrdCondWait
(
    pthread_cond_t  *pstCond,
    pthread_mutex_t *pstMutex,
    DBOOL   *pIsSignal,
    DBOOL   *pIsFlush,
    D32S    ulAbsTime
)
{
    if (NULL == pstCond || NULL == pstMutex || NULL == pIsSignal)
    {
        log_e("Invalid param.");
        return ADM_ECODE_INVALID_PARAM;
    }

    D32S iRet = ADM_ECODE_SUCCESSED;
    // (void)pthread_mutex_lock(pstMutex);

    if(0 < ulAbsTime)
    {
        iRet = adm_pub_pthread_cond_timedwait(pstCond, pstMutex, pIsSignal, pIsFlush, ulAbsTime);
    }
    else
    {
        iRet = pthread_cond_wait(pstCond, pstMutex);
    }

    // (void)pthread_mutex_unlock(pstMutex);

    return iRet;
}


/*!
 *******************************************************************************
 * 获取解密异步标签信息
 *
 * \note 对应销毁函数 pubDestroyDecryptAsyncInfo
 * 
 * \param   none
 * \retval  ADM_DOWN_ASYNCTAG_S
 *******************************************************************************
 */
static ADM_DECRYPT_ASYNC_S* pubGetDecryptAsyncTagInfo(void)
{
    D32S i = 0;
    ADM_DECRYPT_ASYNC_S *pstDecryptAsyncTag = NULL;

    (void)pthread_rwlock_rdlock(&gstDecryptHandle.stRWlock);
    for(i = 0; i < ADM_PUB_DECRYPT_MAX_NUM; i++)
    {
        if(gstDecryptHandle.stDecryptList.pstDecryptAsyncInfo != NULL)
        {
            //找空闲的异步标签
            if(FALSE == gstDecryptHandle.stDecryptList.pstDecryptAsyncInfo[i].bIsUsing)
            {
                pstDecryptAsyncTag = &gstDecryptHandle.stDecryptList.pstDecryptAsyncInfo[i];
                pstDecryptAsyncTag->bHasResult = FALSE;
                pstDecryptAsyncTag->bIsFlush = FALSE;
                pstDecryptAsyncTag->stResult.result = ADM_ECODE_SUCCESSED;
                pstDecryptAsyncTag->bIsUsing = TRUE;
                gstDecryptHandle.stDecryptList.s32Cnt++;
                break;
            }
        }
    }
    (void)pthread_rwlock_unlock(&gstDecryptHandle.stRWlock);
    
    return pstDecryptAsyncTag;
}

/*!
 *******************************************************************************
 * 清空解密异步信息
 *
 * \note 不清空锁及条件变量 
 * 
 * \param[in]  pstDecryptAsyncInfo   解密异步信息
 * \retval     none
 *******************************************************************************
 */
static void pubCleanDecryptAsyncInfo(ADM_DECRYPT_ASYNC_S *pstDecryptAsyncInfo)
{
    if (ADM_IS_NULL(pstDecryptAsyncInfo))
    {
        return;
    }

    pstDecryptAsyncInfo->bHasResult = FALSE;
    pstDecryptAsyncInfo->bIsFlush = FALSE;
    pstDecryptAsyncInfo->bIsUsing = FALSE;
    pstDecryptAsyncInfo->stResult.result = ADM_ECODE_SUCCESSED;
    adm_free(pstDecryptAsyncInfo->stResult.realVal);
    adm_free(pstDecryptAsyncInfo->stResult.expectVal);
    adm_free(pstDecryptAsyncInfo->pcDecryptStoragePath);
    adm_free(pstDecryptAsyncInfo->pcFileID);

    return;
}

/*!
 *******************************************************************************
 * 设置解密结束
 * 
 * \note  
 * 
 * \retval     none
 *******************************************************************************
 */
static void pubDecryptSetDecryptEnd(ADM_DECRYPT_ASYNC_S *pstDecryptAsyncInfo)
{
    if (ADM_IS_NULL(pstDecryptAsyncInfo))
    {
        return;
    }

    //!< 模块未初始化，则直接报错
    if (FALSE == gstDecryptHandle.isInited)
    {
        log_i("recv ucl decrypt not init, err[%d]", ADM_ECODE_MODULE_UNINIT);
        return;
    }

    (void)pthread_rwlock_wrlock(&gstDecryptHandle.stRWlock);
    pubCleanDecryptAsyncInfo(pstDecryptAsyncInfo);
    gstDecryptHandle.stDecryptList.s32Cnt--;
    (void)pthread_rwlock_unlock(&gstDecryptHandle.stRWlock);
    return;
}

/*!
 *******************************************************************************
 * 检查解密并发情况
 * 
 * \note  
 * 
 * \retval     none
 *******************************************************************************
 */
static D32S pubDecryptNumCheck(void)
{
    //!< 模块未初始化，则直接报错
    if (FALSE == gstDecryptHandle.isInited)
    {
        log_i("recv ucl decrypt, dpm module not init, err[%d]", ADM_ECODE_MODULE_UNINIT);
        return ADM_ECODE_MODULE_UNINIT;
    }

    //!< 解密并发数量
    (void)pthread_rwlock_rdlock(&gstDecryptHandle.stRWlock);
    if (gstDecryptHandle.stDecryptList.s32Cnt > ADM_PUB_DECRYPT_MAX_NUM)
    {
        (void)pthread_rwlock_unlock(&gstDecryptHandle.stRWlock);
        log_e("other module has decrypting, err[%d]", ADM_ECODE_HAVE_DECRYPT_ING);
        return ADM_ECODE_HAVE_DECRYPT_ING;
    }
    (void)pthread_rwlock_unlock(&gstDecryptHandle.stRWlock);

    return ADM_ECODE_SUCCESSED;
}

/*!
 *******************************************************************************
 * 异步转同步超时判断.
 *
 * \note 
 *
 * \param[in]   pstDecryptAsyncInfo 异步条件变量信息
 * \retval      none
 *******************************************************************************
 */
static D32S __adm_pub_waitDecryptAsyncResult(ADM_DECRYPT_ASYNC_S *pstDecryptAsyncInfo)
{
    D32S iRet = ADM_ECODE_SUCCESSED;

    log_i("Wait decrypt async result.");
    //!<  异步响应提前到达
    if (FALSE != pstDecryptAsyncInfo->bHasResult)
    {
        return iRet;
    }

    #if 0
    do
    {
        //!< 1、异步请求转同步等待 --- 等待超时时间 10s
        (void)adm_pub_ThrdCondWait(&pstDecryptAsyncInfo->stCond, &pstDecryptAsyncInfo->stMutex, 
                                    &pstDecryptAsyncInfo->bHasResult, ADM_COM_ASYNC_WAIT_TIME);

        //!< 2、等待10s后，有结果，不再等待
        if (TRUE == pstDecryptAsyncInfo->bHasResult)
        {
            break ;
        }
        //!< 3、等待10s后，没有结果，但是有进度，需要继续等待
        else if (TRUE == pstDecryptAsyncInfo->bIsFlush)
        {
            (void)pthread_mutex_lock(&pstDecryptAsyncInfo->stMutex);
            pstDecryptAsyncInfo->bIsFlush = FALSE;
            (void)pthread_mutex_unlock(&pstDecryptAsyncInfo->stMutex);
            continue; 
        }
        //!< 4、既没结果又没有进度刷新，返回异步响应超时
        else
        {
            iRet = ADM_ECODE_MSG_WAIT_TIMEOUT;
            break;
        }
    }while (TRUE);
#endif

    iRet = adm_pub_ThrdCondWait(&pstDecryptAsyncInfo->stCond, &pstDecryptAsyncInfo->stMutex,
                         &pstDecryptAsyncInfo->bHasResult, &pstDecryptAsyncInfo->bIsFlush, ADM_COM_ASYNC_WAIT_TIME);
    if (ADM_IS_ERR(iRet))
    {
        iRet = ADM_ECODE_MSG_WAIT_TIMEOUT;
    }

    log_i("Wait decrypt async result [%d]", iRet);
    return iRet;

}

/*!
 *******************************************************************************
 * ucl 服务解密结果通知回调函数
 *
 * \note 主控的 ucl 解密
 * 
 * \param[in]   fileID               解密文件标识id
 * \param[in]   storePath            解密成功时解密后文件的存储路径
 * \param[in]   pstDecryptResultInfo 解密结果信息
 * \param[in]   priv                 私有信息
 * \retval      D32S
 *******************************************************************************
 */
D32S adm_pub_dimUclDecryptResultHandle
(
    const DCHAR *fileID, 
    const DCHAR *storePath, 
    const ADM_DIM_SOA_UCL_DECRYPT_RESULT_INFO_T *pstDecryptResultInfo, 
    void *priv
)
{
    D32S i = 0;
    ADM_DECRYPT_ASYNC_S *pstDecryptAsyncTag = NULL;

    if (ADM_IS_NULL(fileID) || ADM_IS_NULL(pstDecryptResultInfo))
    {
        log_e("Recv ucl decrypt, Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }

    log_i("Recv ucl decrypt result notify, fileID[%s], result[%d].", fileID, pstDecryptResultInfo->result);

    //!< 模块未初始化，则直接报错
    if (FALSE == gstDecryptHandle.isInited)
    {
        log_i("Recv ucl decrypt, dpm module not init, err[%d]", ADM_ECODE_MODULE_UNINIT);
        return ADM_ECODE_MODULE_UNINIT;
    }

    //设置解密结果异步响应
    for(i = 0; i < ADM_PUB_DECRYPT_MAX_NUM; i++)
    {
        if(gstDecryptHandle.stDecryptList.pstDecryptAsyncInfo != NULL)
        {
            if(gstDecryptHandle.stDecryptList.pstDecryptAsyncInfo[i].pcFileID != NULL)
            {
                if (0 == adm_strcmp(fileID, gstDecryptHandle.stDecryptList.pstDecryptAsyncInfo[i].pcFileID))
                {
                    log_i("decrypt result notify ing");
                    pstDecryptAsyncTag = &gstDecryptHandle.stDecryptList.pstDecryptAsyncInfo[i];

                    // 异步转同步 --- launch async cond
                    (void)pthread_mutex_lock(&pstDecryptAsyncTag->stMutex);
                    pstDecryptAsyncTag->bHasResult = TRUE;
                    pstDecryptAsyncTag->stResult.result = pstDecryptResultInfo->result;
                    pstDecryptAsyncTag->stResult.realVal = 
                        ((NULL != pstDecryptResultInfo->realVal) ? (adm_strdup(pstDecryptResultInfo->realVal)) : (NULL));
                    pstDecryptAsyncTag->stResult.expectVal = 
                        ((NULL != pstDecryptResultInfo->expectVal) ? (adm_strdup(pstDecryptResultInfo->expectVal)) : (NULL));
                    pstDecryptAsyncTag->pcDecryptStoragePath = ((NULL != storePath) ? (adm_strdup(storePath)) : (NULL));
                    (void)pthread_cond_signal(&pstDecryptAsyncTag->stCond);
                    (void)pthread_mutex_unlock(&pstDecryptAsyncTag->stMutex);
                    break;
                }
            }
        }
    }
    log_i("Recv ucl decrypt result notify end");

    return ADM_ECODE_SUCCESSED;
}




/*!
 *******************************************************************************
 * UCL 服务解密进度通知回调函数
 *
 * \note 主控的 ucl 解密
 * 
 * \param[in]   fileID                 解密文件标识id
 * \param[in]   pstDecryptProgressInfo 解密进度信息
 * \param[in]   priv                   私有信息
 * \retval      D32S
 *******************************************************************************
 */
D32S adm_pub_dimUclDecryptProgressHandle
(
    const DCHAR *fileID, 
    const ADM_DIM_SOA_UCL_DECRYPT_PROGRESS_INFO_T *pstDecryptProgressInfo, 
    void *priv
)
{
    D32S i = 0;
    ADM_DECRYPT_ASYNC_S *pstDecryptAsyncTag = NULL;
    if (ADM_IS_NULL(fileID) || ADM_IS_NULL(pstDecryptProgressInfo))
    {
        log_e("Recv ucl decrypt, Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }

    log_i("Recv ucl decrypt progress notify, fileID[%s], progress[%lld - %lld].", 
                    fileID, pstDecryptProgressInfo->savedSize, pstDecryptProgressInfo->totalSize);

    //!< 模块未初始化，则直接报错
    if (FALSE == gstDecryptHandle.isInited)
    {
        log_i("Recv ucl decrypt, dpm module not init, err[%d]", ADM_ECODE_MODULE_UNINIT);
        return ADM_ECODE_MODULE_UNINIT;
    }

    //设置解密结果异步响应
    for(i = 0; i < ADM_PUB_DECRYPT_MAX_NUM; i++)
    {
        if(gstDecryptHandle.stDecryptList.pstDecryptAsyncInfo != NULL)
        {
            if(gstDecryptHandle.stDecryptList.pstDecryptAsyncInfo[i].pcFileID != NULL)
            {
                if (0 == adm_strcmp(fileID, gstDecryptHandle.stDecryptList.pstDecryptAsyncInfo[i].pcFileID))
                {
                    pstDecryptAsyncTag = &gstDecryptHandle.stDecryptList.pstDecryptAsyncInfo[i];

                    // 异步转同步 --- launch async cond
                    (void)pthread_mutex_lock(&pstDecryptAsyncTag->stMutex);
                    pstDecryptAsyncTag->bIsFlush = TRUE;
                    (void)pthread_cond_signal(&pstDecryptAsyncTag->stCond);
                    (void)pthread_mutex_unlock(&pstDecryptAsyncTag->stMutex);
                    break;
                }
            }
        }
    }

    return ADM_ECODE_SUCCESSED;
}


/*!
 *******************************************************************************
 * 解密下载的文件信息
 *
 * \note
 * 
 * \param[in]   pstPkgFileObj  文件对象信息 
 * \param[in]   ppcDecryptPath 解密后的文件存储路径 --- 外部需要释放 
* \param[in]    pcDownInstId   下载的instid 
 * \retval      D32S
 *******************************************************************************
 */
D32S adm_pub_downPackageDecrypt
(
    ADM_COM_FILEOBJECT_T  *pstPkgFileObj,
    DCHAR                **ppcDecryptPath,
    DCHAR                 *pcDownInstId
)
{
    log_i("ucl Decrypt begin! Decrypting num[%d]",gstDecryptHandle.stDecryptList.s32Cnt);

    D32S iRet = ADM_ECODE_SUCCESSED;
    D32S errorCode=0;
    D32S s32UclInstId = 0;
    D32S s32UCMDownInstId = 0;
    ADM_DECRYPT_ASYNC_S *pstDecryptAsyncTag = NULL;

    if (ADM_IS_NULL(pstPkgFileObj) || ADM_IS_NULL(ppcDecryptPath) || ADM_IS_NULL(pcDownInstId))
    {
        log_e("Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }

    //!< 解密的条件是否满足 
    if((NULL == pstPkgFileObj->pstFileSecurityInfo) || (pstPkgFileObj->pstFileSecurityInfo->isEncrypt == 0)) 
    {
        log_i("Filed dont need decrypt, file[%s]", pstPkgFileObj->filePath);
        return ADM_ECODE_SUCCESSED;
    }

    do
    {
        //!< 创建异步消息
        pstDecryptAsyncTag = pubGetDecryptAsyncTagInfo();
        if (ADM_IS_NULL(pstDecryptAsyncTag))
        {
            iRet = ADM_ECODE_HAVE_DECRYPT_ING;
            log_e("Failed to get Decrypt async tag info");
            break;
        }
        
        //!< 由于解密不能超过并发数量
        iRet = pubDecryptNumCheck();
        if (ADM_IS_ERR(iRet))
        {
            log_e("check decrypt status, err[%d]", iRet);
            break;
        }

        pstDecryptAsyncTag->pcFileID = adm_strdup(pstPkgFileObj->fileName);

        log_i("package decrypt, filePath[%s], pcDownInstId[%s]", pstPkgFileObj->filePath, pcDownInstId);
        //如果下载的instanceid = uim_instanceid，则需要获取ucl的instanceid进行解密
        s32UCMDownInstId = adm_cfg_iniParserGetInt(ADM_CFG_VUS_UIM_INSTANCEID,  ADM_CFG_VUS_UIM_INSTANCEID_DEFAULT_VAL);
        if(s32UCMDownInstId == htoi(pcDownInstId))
        {
            s32UclInstId = adm_cfg_iniParserGetInt(ADM_CFG_VUS_UCL_INSTANCEID,  ADM_CFG_VUS_UCL_INSTANCEID_DEFAULT_VAL);
        }
        else
        {
            s32UclInstId = htoi(pcDownInstId);
        }

        iRet = adm_dim_ucl_requestDecryptPackage((D16U)s32UclInstId, 30, pstPkgFileObj->fileName,
                        pstPkgFileObj->filePath, 
                        pstPkgFileObj->pstFileSecurityInfo->pkgKey,
                        pstPkgFileObj->pstFileSecurityInfo->segmentDecryptLen, 
                        pstPkgFileObj->pstFileSecurityInfo->oriHash, 
                        &errorCode);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to request ucl decrypt package, uclInstID[%d], filePath[%s], iRet[%d], errCode[%d], err[%d]", 
                    s32UclInstId, pstPkgFileObj->filePath, iRet, errorCode, ADM_ECODE_AES_ECB_DECRYPT_FAILED);
            iRet = ADM_ECODE_AES_ECB_DECRYPT_FAILED;
            break;
        }
        
        //!< 异步判断是否超时
        iRet = __adm_pub_waitDecryptAsyncResult(pstDecryptAsyncTag);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Wait decrypt async result time out. err[%d]", iRet);
            break;
        }

    } while (FALSE);
    
    //!< 若请求解密失败则更新解密结果 
    if (ADM_IS_ERR(iRet))
    {
        pubDecryptSetDecryptEnd(pstDecryptAsyncTag);
        return iRet;
    }

    //!< 若请求解密成功，需要判断异步解密结果是否成功 
    iRet = pstDecryptAsyncTag->stResult.result;
    if (ADM_IS_ERR(iRet))
    {
        pubDecryptSetDecryptEnd(pstDecryptAsyncTag);
        /** 回调函数对端响应解密失败 */
        log_e("UCL decrypt failed, err[%d]", iRet);
        return ADM_ECODE_AES_ECB_DECRYPT_FAILED;
    }
    else
    {
        /** 回调函数对端响应解密成功 */
        if (NULL == pstDecryptAsyncTag->pcDecryptStoragePath)
        {
            pubDecryptSetDecryptEnd(pstDecryptAsyncTag);
            log_e("UCL decrypt success, but not decryptStoragePath");
            return ADM_ECODE_AES_ECB_DECRYPT_FAILED;
        }
        //!< 解密成功需要更新文件存储路径 --- 文件名会不会变 ???
        log_d("UCL decrypt success, pcDecryptStoragePath[%s], oldFilePath[%s]", 
                pstDecryptAsyncTag->pcDecryptStoragePath, pstPkgFileObj->filePath);
        *ppcDecryptPath = adm_strdup(pstDecryptAsyncTag->pcDecryptStoragePath);    
    }

    pubDecryptSetDecryptEnd(pstDecryptAsyncTag);

    return ADM_ECODE_SUCCESSED;
}


#if 0
#endif


/*!
 *******************************************************************************
 * 创建解密列表内存.
 *
 * \note 
 *
 * \retval      none
 *******************************************************************************
 */
static D32S pubCreateVerifySignList(void)
{
    D32S i = 0;

    gstVerifySignptHandle.stVerifySignList.pstVerifySignAsync =(ADM_VERIFYSIGN_ASYNC_S*)adm_malloc(sizeof(ADM_VERIFYSIGN_ASYNC_S) * (D32U)ADM_PUB_VERIFYSIGN_MAX_NUM);
    if (ADM_IS_NULL(gstVerifySignptHandle.stVerifySignList.pstVerifySignAsync))
    {
        log_e("Failed to malloc pstVerifySignAsyncInfo");
        return ADM_ECODE_NO_MEMORY;
    }
    else
    {
        pthread_condattr_t cattr;
        pthread_condattr_init(&cattr);
        pthread_condattr_setclock(&cattr, CLOCK_MONOTONIC);

        for(i = 0; i < ADM_PUB_VERIFYSIGN_MAX_NUM; i++)
        {
            gstVerifySignptHandle.stVerifySignList.pstVerifySignAsync[i].pcFileID = NULL;
            gstVerifySignptHandle.stVerifySignList.pstVerifySignAsync[i].bHasResult = FALSE;
            gstVerifySignptHandle.stVerifySignList.pstVerifySignAsync[i].bIsFlush = FALSE;
            gstVerifySignptHandle.stVerifySignList.pstVerifySignAsync[i].stResult.result = ADM_ECODE_SUCCESSED;
            gstVerifySignptHandle.stVerifySignList.pstVerifySignAsync[i].bIsUsing = FALSE;
            (void)pthread_mutex_init(&gstVerifySignptHandle.stVerifySignList.pstVerifySignAsync[i].stMutex, NULL);
            (void)pthread_cond_init(&gstVerifySignptHandle.stVerifySignList.pstVerifySignAsync[i].stCond, &cattr);
        }
    }

    return ADM_ECODE_SUCCESSED;
}

/*!
 *******************************************************************************
 * 验签模块初始化.
 *
 * \note
 *
 * \param   none
 * \retval  D32S    0: 成功 non-0：失败
 *******************************************************************************
 */
D32S adm_pub_VerifySignModuleInit(void)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    D32S errNum = 0;

    if (TRUE == gstVerifySignptHandle.isInited)
    {
        log_i("Verify Sign module already init.");
        return ADM_ECODE_SUCCESSED;
    }

    iRet = pthread_rwlock_init(&gstVerifySignptHandle.stRWlock, NULL);
    if (ADM_IS_ERR(iRet))
    {
        errNum = errno;
        log_e("Failed to init rwlock, err[%d](%s)", ADM_ECODE_RWLOCK_INIT_FAILED, strerror(errNum));
        return ADM_ECODE_RWLOCK_INIT_FAILED;
    }
    
    //!< 验签异步响应信息
    gstVerifySignptHandle.stVerifySignList.pstVerifySignAsync = NULL;
    gstVerifySignptHandle.stVerifySignList.s32Cnt = 0;
    iRet = pubCreateVerifySignList();
    if (ADM_IS_ERR(iRet))
    {
        log_e("init decrypt module failed, err[%d]", iRet);
        return iRet;
    }
    gstVerifySignptHandle.isInited = TRUE;
    
    log_d("===> Verify Sign init module ok.");

    return iRet;
}

/*!
 *******************************************************************************
 * 销毁验签异步信息
 *
 * \note 不释放 pstVerifySignAsyncInfo 内存
 * 
 * \param[in]  pstVerifySignAsyncInfo   验签异步信息
 * \retval     none
 *******************************************************************************
 */
static void pubDestroyVerifySignAsyncInfo(ADM_VERIFYSIGN_ASYNC_S *pstVerifySignAsyncInfo)
{
    if (ADM_IS_NULL(pstVerifySignAsyncInfo))
    {
        return;
    }

    adm_free(pstVerifySignAsyncInfo->pcFileID);
    adm_free(pstVerifySignAsyncInfo->stResult.signature);
    adm_free(pstVerifySignAsyncInfo->stResult.filePath);
    (void)pthread_mutex_destroy(&pstVerifySignAsyncInfo->stMutex);
    (void)pthread_cond_destroy(&pstVerifySignAsyncInfo->stCond);

    return;
}

/*!
 *******************************************************************************
 * 清空验签异步信息
 *
 * \note 不清空锁及条件变量 
 * 
 * \param[in]  pstVerifySignAsyncInfo   验签异步信息
 * \retval     none
 *******************************************************************************
 */
static void pubCleanVerifySignAsyncInfo(ADM_VERIFYSIGN_ASYNC_S *pstVerifySignAsyncInfo)
{
    if (ADM_IS_NULL(pstVerifySignAsyncInfo))
    {
        return;
    }

    pstVerifySignAsyncInfo->bHasResult = FALSE;
    pstVerifySignAsyncInfo->bIsFlush = FALSE;
    pstVerifySignAsyncInfo->bIsUsing = FALSE;
    pstVerifySignAsyncInfo->stResult.result = ADM_ECODE_SUCCESSED;
    adm_free(pstVerifySignAsyncInfo->stResult.signature);
    adm_free(pstVerifySignAsyncInfo->stResult.filePath);
    adm_free(pstVerifySignAsyncInfo->pcFileID);

    return;
}

/*!
 *******************************************************************************
 * 设置验签结束
 * 
 * \note  
 * 
 * \retval     none
 *******************************************************************************
 */
static void pubSetVerifySignEnd(ADM_VERIFYSIGN_ASYNC_S *pstVerifySignAsyncInfo)
{
    if (ADM_IS_NULL(pstVerifySignAsyncInfo))
    {
        return;
    }

    //!< 模块未初始化，则直接报错
    if (FALSE == gstVerifySignptHandle.isInited)
    {
        log_i("recv ucl verify sign not init, err[%d]", ADM_ECODE_MODULE_UNINIT);
        return;
    }

    (void)pthread_rwlock_wrlock(&gstVerifySignptHandle.stRWlock);
    pubCleanVerifySignAsyncInfo(pstVerifySignAsyncInfo);
    gstVerifySignptHandle.stVerifySignList.s32Cnt--;
    (void)pthread_rwlock_unlock(&gstVerifySignptHandle.stRWlock);

    return;
}

/*!
 *******************************************************************************
 * 清空验签列表内存.
 *
 * \note 
 *
 * \retval      none
 *******************************************************************************
 */
static void pubDestroyVerifySignList(void)
{
    D32S i = 0;

    //!< 模块未初始化，则直接报错
    if (FALSE == gstVerifySignptHandle.isInited)
    {
        return;
    }

    (void)pthread_rwlock_wrlock(&gstVerifySignptHandle.stRWlock);
    for(i = 0; i < ADM_PUB_VERIFYSIGN_MAX_NUM ; i++)
    {
        pubDestroyVerifySignAsyncInfo(&gstVerifySignptHandle.stVerifySignList.pstVerifySignAsync[i]);
    }
    gstVerifySignptHandle.stVerifySignList.pstVerifySignAsync = NULL;
    (void)pthread_rwlock_wrlock(&gstVerifySignptHandle.stRWlock);

    return;
}

/*!
 *******************************************************************************
 * 验签模块销毁.
 *
 * \note
 *
 * \param   none
 * \retval  none
 *******************************************************************************
 */
void adm_pub_verifySignModuleDeInit(void)
{
    if (TRUE == gstVerifySignptHandle.isInited)
    {
        /** 销毁解密列表 */
        pubDestroyVerifySignList();

        (void)pthread_rwlock_wrlock(&gstVerifySignptHandle.stRWlock);
        gstVerifySignptHandle.stVerifySignList.s32Cnt = 0;
        gstVerifySignptHandle.isInited = FALSE;
        (void)pthread_rwlock_unlock(&gstVerifySignptHandle.stRWlock);
        (void)pthread_rwlock_destroy(&gstVerifySignptHandle.stRWlock);
    }

    return;
}

/*!
 *******************************************************************************
 * 检查验签并发情况
 * 
 * \note  
 * 
 * \retval     none
 *******************************************************************************
 */
static D32S pubVerifySignNumCheck(void)
{
    //!< 模块未初始化，则直接报错
    if (FALSE == gstVerifySignptHandle.isInited)
    {
        log_i("recv ucl Verify Sign, module not init, err[%d]", ADM_ECODE_MODULE_UNINIT);
        return ADM_ECODE_MODULE_UNINIT;
    }

    //!< 验签最大并发数量
    (void)pthread_rwlock_wrlock(&gstVerifySignptHandle.stRWlock);
    if (gstVerifySignptHandle.stVerifySignList.s32Cnt > ADM_PUB_VERIFYSIGN_MAX_NUM)
    {
        (void)pthread_rwlock_unlock(&gstVerifySignptHandle.stRWlock);
        log_e("other module has VerifySigning, err[%d]", ADM_ECODE_HAVE_VERIFYSIGN_ING);
        return ADM_ECODE_HAVE_VERIFYSIGN_ING;
    }
    (void)pthread_rwlock_unlock(&gstVerifySignptHandle.stRWlock);

    return ADM_ECODE_SUCCESSED;
}

/*!
 *******************************************************************************
 * 获取下载异步标签信息
 *
 * \note 
 * 
 * \param   none
 * \retval  ADM_VERIFYSIGN_ASYNC_S
 *******************************************************************************
 */
static ADM_VERIFYSIGN_ASYNC_S* pubGetVerifySignAsyncTagInfo(void)
{
    D32S i = 0;
    ADM_VERIFYSIGN_ASYNC_S *pstVerifySignAsync = NULL;

    (void)pthread_rwlock_rdlock(&gstVerifySignptHandle.stRWlock);
    for(i = 0; i < ADM_PUB_DECRYPT_MAX_NUM; i++)
    {
        if(gstVerifySignptHandle.stVerifySignList.pstVerifySignAsync != NULL)
        {
            if(FALSE == gstVerifySignptHandle.stVerifySignList.pstVerifySignAsync[i].bIsUsing)
            {
                pstVerifySignAsync = &gstVerifySignptHandle.stVerifySignList.pstVerifySignAsync[i];
                pstVerifySignAsync->bHasResult = FALSE;
                pstVerifySignAsync->bIsFlush = FALSE;
                pstVerifySignAsync->stResult.result = ADM_ECODE_SUCCESSED;
                pstVerifySignAsync->bIsUsing = TRUE;
                gstVerifySignptHandle.stVerifySignList.s32Cnt++;
                break;
            }
        }
    }
    (void)pthread_rwlock_unlock(&gstVerifySignptHandle.stRWlock);
    
    return pstVerifySignAsync;
}

/*!
 *******************************************************************************
 * 验签异步转同步超时判断.
 *
 * \note 
 *
 * \param[in]   pstVerifySignAsyncInfo 异步条件变量信息
 * \retval      none
 *******************************************************************************
 */
static D32S __adm_pub_waitVerifySignAsyncResult(ADM_VERIFYSIGN_ASYNC_S *pstVerifySignAsyncInfo)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    log_i("Wait verify sign async result.");
    //!<  异步响应提前到达
    if (FALSE != pstVerifySignAsyncInfo->bHasResult)
    {
        return iRet;
    }

#if 0
    do
    {
        //!< 1、异步请求转同步等待 --- 等待超时时间 10s
        (void)adm_pub_ThrdCondWait(&pstVerifySignAsyncInfo->stCond, &pstVerifySignAsyncInfo->stMutex,
                                   &pstVerifySignAsyncInfo->bHasResult, ADM_COM_ASYNC_WAIT_TIME * 5);

        //!< 2、等待10s后，有结果，不再等待
        if (TRUE == pstVerifySignAsyncInfo->bHasResult)
        {
            break ;
        }
        //!< 3、等待10s后，没有结果，但是有进度，需要继续等待
        else if (TRUE == pstVerifySignAsyncInfo->bIsFlush)
        {
            (void)pthread_mutex_lock(&pstVerifySignAsyncInfo->stMutex);
            pstVerifySignAsyncInfo->bIsFlush = FALSE;
            (void)pthread_mutex_unlock(&pstVerifySignAsyncInfo->stMutex);
            continue; 
        }
        //!< 4、既没结果又没有进度刷新，返回异步响应超时
        else
        {
            iRet = ADM_ECODE_MSG_WAIT_TIMEOUT;
            break;
        }
    }while (TRUE);
#endif

    iRet = adm_pub_ThrdCondWait(&pstVerifySignAsyncInfo->stCond, &pstVerifySignAsyncInfo->stMutex,
                                &pstVerifySignAsyncInfo->bHasResult, &pstVerifySignAsyncInfo->bIsFlush, ADM_COM_ASYNC_WAIT_TIME * 5);
    if (ADM_IS_ERR(iRet))
    {
        iRet = ADM_ECODE_MSG_WAIT_TIMEOUT;
    }

    return iRet;

}

/*!
 *******************************************************************************
 * ucl 服务验签结果通知回调函数
 *
 * \note 主控的 ucl 验签
 * 
 * \param[in]   fileID               验签文件标识id
 * \param[in]   pstVerifySignResultInfo 验签结果信息
 * \retval      D32S
 *******************************************************************************
 */
D32S adm_pub_dimUclVerifySignResultHandle
(
    const DCHAR* fileID, 
    const ADM_SYSI4UCL_VERIFY_SIGN_RESULT_INFO_T *pstVerifySignResultInfo
)
{
    D32S i = 0;
    ADM_VERIFYSIGN_ASYNC_S* pstVerifySignAsync = NULL;

    if (ADM_IS_NULL(fileID) || ADM_IS_NULL(pstVerifySignResultInfo))
    {
        log_e("Recv ucl verify sign, Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }

    log_i("Recv ucl verify sign result notify, fileID[%s], result[%d].", fileID, pstVerifySignResultInfo->result);

    //!< 模块未初始化，则直接报错
    if (FALSE == gstVerifySignptHandle.isInited)
    {
        log_i("Recv ucl verify sign, module not init, err[%d]", ADM_ECODE_MODULE_UNINIT);
        return ADM_ECODE_MODULE_UNINIT;
    }

    //设置解密结果异步响应
    for(i = 0; i < ADM_PUB_VERIFYSIGN_MAX_NUM; i++)
    {
        if(gstVerifySignptHandle.stVerifySignList.pstVerifySignAsync != NULL)
        {
            if(gstVerifySignptHandle.stVerifySignList.pstVerifySignAsync[i].pcFileID != NULL)
            {
                if (0 == adm_strcmp(fileID, gstVerifySignptHandle.stVerifySignList.pstVerifySignAsync[i].pcFileID))
                {
                    log_i("Verify Sign result notify ing");
                    pstVerifySignAsync = &gstVerifySignptHandle.stVerifySignList.pstVerifySignAsync[i];

                    // 异步转同步 --- launch async cond
                    (void)pthread_mutex_lock(&pstVerifySignAsync->stMutex);
                    pstVerifySignAsync->bHasResult = TRUE;
                    pstVerifySignAsync->stResult.result = pstVerifySignResultInfo->result;
                    pstVerifySignAsync->stResult.signatureLen = pstVerifySignResultInfo->signatureLen;
                    pstVerifySignAsync->stResult.signature = 
                        ((NULL != pstVerifySignResultInfo->signature) ? (adm_strdup(pstVerifySignResultInfo->signature)) : (NULL));
                    pstVerifySignAsync->stResult.filePath = 
                        ((NULL != pstVerifySignResultInfo->filePath) ? (adm_strdup(pstVerifySignResultInfo->filePath)) : (NULL));
                    (void)pthread_cond_signal(&pstVerifySignAsync->stCond);
                    (void)pthread_mutex_unlock(&pstVerifySignAsync->stMutex);
                    break;
                }
            }
        }
    }
    log_i("Recv ucl Verify Sign result notify end");
    return ADM_ECODE_SUCCESSED;
}


/*!
 *******************************************************************************
 * UCL 服务验签进度通知回调函数
 *
 * \note 主控的 ucl 验签
 * 
 * \param[in]   fileID                 验签文件标识id
 * \param[in]   progress               验签进度信息
 * \retval      D32S
 *******************************************************************************
 */
D32S adm_pub_dimUclVerifySignProgressHandle
(
    const DCHAR* fileID,
    const D16U progress
)
{
    D32S i = 0;
    ADM_VERIFYSIGN_ASYNC_S* pstVerifySignAsync = NULL;

    if (ADM_IS_NULL(fileID))
    {
        log_e("Recv ucl verify sign, Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }

    log_i("Recv ucl verify sign progress notify, fileID[%s], progress[%d].", fileID, progress);

    //!< 模块未初始化，则直接报错
    if (FALSE == gstVerifySignptHandle.isInited)
    {
        log_i("Recv ucl verify sign, module not init, err[%d]", ADM_ECODE_MODULE_UNINIT);
        return ADM_ECODE_MODULE_UNINIT;
    }

    //设置解密结果异步响应
    for(i = 0; i < ADM_PUB_VERIFYSIGN_MAX_NUM; i++)
    {
        if(gstVerifySignptHandle.stVerifySignList.pstVerifySignAsync != NULL)
        {
            if(gstVerifySignptHandle.stVerifySignList.pstVerifySignAsync[i].pcFileID != NULL)
            {
                if (0 == adm_strcmp(fileID, gstVerifySignptHandle.stVerifySignList.pstVerifySignAsync[i].pcFileID))
                {
                    pstVerifySignAsync = &gstVerifySignptHandle.stVerifySignList.pstVerifySignAsync[i];

                    // 异步转同步 --- launch async cond
                    (void)pthread_mutex_lock(&pstVerifySignAsync->stMutex);
                    pstVerifySignAsync->bIsFlush = TRUE;
                    (void)pthread_cond_signal(&pstVerifySignAsync->stCond);
                    (void)pthread_mutex_unlock(&pstVerifySignAsync->stMutex);
                    break;
                }
            }
        }
    }

    return ADM_ECODE_SUCCESSED;
}

/*!
*******************************************************************************
* 删除验签file meta 数据.
*
* \note
*
* \param[in]  pfileMetasList file list信息
* \param[in]  sum            已创建fileMetasArray的个数
* \retval	   none
*******************************************************************************
*/
static void __adm_pub_delfileMetasListInfo(FILEMETAS_INFO_ARRAY_T **pfileMetasList, D32U sum)
{
    if (ADM_IS_NULL(pfileMetasList) || ADM_IS_NULL(*pfileMetasList) || ((D32U)0 == sum))
    {
        return;
    }

    for(D32U i = (D32U)0; i < sum; i++)
    {
        adm_free((*pfileMetasList)->fileMetasArray[i]->tObject.pAbstracValue);
        adm_free((*pfileMetasList)->fileMetasArray[i]);
    }

    adm_free((*pfileMetasList)->fileMetasArray);
    adm_free(*pfileMetasList);

    return;
}

/*!
 *******************************************************************************
 * 获取filematas信息
 *
 * \note
 * 
 * \param[in]   pstFileSecurityInfo  文件安全信息 
 * \param[out]  pcfileMetasList  filematas信息
 * \retval      D32S
 *******************************************************************************
 */
static D32S adm_pub_getfilemetasinfo(ADM_COM_FILESCYINFO_T *pstFileSecurityInfo, FILEMETAS_INFO_ARRAY_T **pcfileMetasList)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    FILEMETAS_INFO_ARRAY_T* pfileMetasList = NULL;
    D32S i = 0;

    if (ADM_IS_NULL(pstFileSecurityInfo))
    {
        log_e("Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }

    pfileMetasList = (FILEMETAS_INFO_ARRAY_T*)adm_malloc((D32U)sizeof(FILEMETAS_INFO_ARRAY_T));
    if(ADM_IS_NULL(pfileMetasList))
    {
        log_e("Failed to malloc fileMetasList");
        return ADM_ECODE_NO_MEMORY;
    }
    (void)adm_memset(pfileMetasList, 0x0, sizeof(FILEMETAS_INFO_ARRAY_T), sizeof(FILEMETAS_INFO_ARRAY_T));

    pfileMetasList->fileMetaNum = (D32U)pstFileSecurityInfo->fileMetaNum;
    pfileMetasList->fileMetasArray = (FILEMETAS_INFO_T**)adm_malloc(sizeof(FILEMETAS_INFO_T*) * pfileMetasList->fileMetaNum);
    if(ADM_IS_NULL(pfileMetasList->fileMetasArray))
    {
        log_e("Failed to malloc fileMetasArray");
        adm_free(pfileMetasList);
        return ADM_ECODE_NO_MEMORY;
    }
    (void)adm_memset(pfileMetasList->fileMetasArray, 0x0, 
                            sizeof(FILEMETAS_INFO_T*) * pfileMetasList->fileMetaNum, 
                            sizeof(FILEMETAS_INFO_T*) * pfileMetasList->fileMetaNum);

    for(i = 0; (D32U)i < pfileMetasList->fileMetaNum; i++)
    {
        pfileMetasList->fileMetasArray[i] = adm_malloc((D32U)sizeof(FILEMETAS_INFO_T));
        if(ADM_IS_NULL(pfileMetasList->fileMetasArray[i]))
        {
            log_e("Failed to malloc fileMetasList");
            __adm_pub_delfileMetasListInfo(&pfileMetasList,(D32U)i);
            return ADM_ECODE_NO_MEMORY;
        }
        
        (void)adm_memset(pfileMetasList->fileMetasArray[i], 0x0, sizeof(FILEMETAS_INFO_T), sizeof(FILEMETAS_INFO_T));
        
        pfileMetasList->fileMetasArray[i]->size = pstFileSecurityInfo->pstFileMetaList[i].size;
        pfileMetasList->fileMetasArray[i]->tObject.eAlgorthm = (ALGORITHM_TYPE_E)pstFileSecurityInfo->pstFileMetaList[i].stCheckSum.algorithm;
        pfileMetasList->fileMetasArray[i]->tObject.pAbstracValue = adm_strdup(pstFileSecurityInfo->pstFileMetaList[i].stCheckSum.value);
        if(1 == pstFileSecurityInfo->pstFileMetaList[i].type)
        {
            pfileMetasList->fileMetasArray[i]->efiletype = ADM_SIM_SOA_UCL_FILE_TYPE_ORI;
        }
        else if(2 == pstFileSecurityInfo->pstFileMetaList[i].type)
        {
            pfileMetasList->fileMetasArray[i]->efiletype = ADM_SIM_SOA_UCL_FILE_TYPE_SIGN;
        }
        else if(5 == pstFileSecurityInfo->pstFileMetaList[i].type)
        {
            pfileMetasList->fileMetasArray[i]->efiletype = ADM_SIM_SOA_UCL_FILE_TYPE_ORI_512;
            /** 重新赋值,云端下发该字段值为0 */
            pfileMetasList->fileMetasArray[i]->size = 
                (D64S)adm_strlen(pfileMetasList->fileMetasArray[i]->tObject.pAbstracValue);
        }
        else
        {
            pfileMetasList->fileMetasArray[i]->efiletype = ADM_SIM_SOA_UCL_FILE_TYPE_IINVALID;
        }

        log_i("size:%lld, eAlgorthm:%d, pAbstracValue:%s, efiletype:%d", 
            pfileMetasList->fileMetasArray[i]->size,
            pfileMetasList->fileMetasArray[i]->tObject.eAlgorthm,
            pfileMetasList->fileMetasArray[i]->tObject.pAbstracValue,
            pfileMetasList->fileMetasArray[i]->efiletype);
    }

    *pcfileMetasList = pfileMetasList;

    return iRet;
}

/*!
 *******************************************************************************
 * 远程验签
 *
 * \note
 * 
 * \param[in]   pstPkgFileObj  文件对象信息 
 * \param[in]   pcDownInstId   下载实例id 
 * \param[in]   pfileid        文件id
 * \param[in]   isUnpick       是否拆包 
 * \param[out]  pstVerifySignResultInfo   验签结果
 * \retval      D32S
 *******************************************************************************
 */
D32S adm_pub_downPackageVerifySign
(
    ADM_COM_FILEOBJECT_T  *pstPkgFileObj,
    DCHAR *pcDownInstId,
    DCHAR *pfileid,
    DBOOL isUnpick,
    ADM_SYSI4UCL_VERIFY_SIGN_RESULT_INFO_T *pstVerifySignResultInfo
)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    D32S errorCode = 0;
    D32S s32UCMHostInstId = 0;
    D32U signatureLen = 0;
    D32S i = 0;
    D32S originPkgsize = 0;
    DCHAR* poriginPkghash = NULL;
    DCHAR* psignature = NULL;
    FILEMETAS_INFO_ARRAY_T* pfileMetasList = NULL;
    ADM_VERIFYSIGN_ASYNC_S *pstVerifySignAsync = NULL;

    if (ADM_IS_NULL(pstPkgFileObj) || ADM_IS_NULL(pcDownInstId))
    {
        log_e("Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }

    log_i("ucl verify sign begin! verifysigning num[%d]",gstVerifySignptHandle.stVerifySignList.s32Cnt);

    //!< 验签的条件是否满足 
    if (FALSE == adm_com_checkIsNeedToVerifySignature(pstPkgFileObj))
    {
        //不需要验签
        iRet = ADM_ECODE_SUCCESSED;
        log_d("conditions dissatisfy, dont need to Verify signature");
        return iRet;
    }

    do
    {
        //!< 获取异步消息
        pstVerifySignAsync = pubGetVerifySignAsyncTagInfo();
        if (ADM_IS_NULL(pstVerifySignAsync))
        {
            iRet = ADM_ECODE_HAVE_VERIFYSIGN_ING;
            log_e("Failed to get verify sign async tag info");
            break;
        }

        //!< 由于验签并发最大数量校验
        iRet = pubVerifySignNumCheck();
        if (ADM_IS_ERR(iRet))
        {
            log_e("check verify sign num, err[%d]", iRet);
            break;
        }

        //<! 获取验签需要的filemetaslist数据
        iRet = adm_pub_getfilemetasinfo(pstPkgFileObj->pstFileSecurityInfo, &pfileMetasList);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to get filemetas info err[%d]", iRet);
            break;
        }
        
        pstVerifySignAsync->pcFileID = adm_strdup(pfileid);
        
        log_i("package verify sign, pcFileID[%s], pcDownInstId[%s]", pfileid, pcDownInstId);
        iRet = adm_dim_ucl_request_Verify_Sign((D16U)htoi(pcDownInstId), 30, pfileid,
                        pstPkgFileObj->filePath, 
                        pfileMetasList,
                        isUnpick,
                        &errorCode);
        if (ADM_IS_ERR(iRet) || ADM_IS_ERR(errorCode))
        {
            log_e("Failed to request ucl verify sign package, uclInstID[%s], filePath[%s], iRet[%d], errCode[%d], err[%d]", 
                    pcDownInstId, pstPkgFileObj->filePath, iRet, errorCode, ADM_ECODE_VERIFY_SIGNATURE_FAILED);
            iRet = ADM_ECODE_VERIFY_SIGNATURE_FAILED;
            break;
        }
        
        //!< 异步判断是否超时
        iRet = __adm_pub_waitVerifySignAsyncResult(pstVerifySignAsync);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Wait verify sign async result time out. err[%d]", iRet);
            break;
        }

    } while (FALSE);
    
    if (ADM_IS_SUCCESS(iRet))
    {
        //!< 若请求验签成功，需要判断异步验签结果是否成功 
        if((D32U)0x01 == pstVerifySignAsync->stResult.result)
        {
            //下载时远程验签不需要信息返回，故增加此判断. 安装验签时需要数据需要返回数据
            if(NULL != pstVerifySignResultInfo)
            {
                pstVerifySignResultInfo->result = pstVerifySignAsync->stResult.result;
                pstVerifySignResultInfo->signatureLen = pstVerifySignAsync->stResult.signatureLen;

                if(NULL != pstVerifySignAsync->stResult.signature)
                {
                    pstVerifySignResultInfo->signature = adm_strdup(pstVerifySignAsync->stResult.signature);
                }
                
                if(NULL != pstVerifySignAsync->stResult.filePath)
                {
                    pstVerifySignResultInfo->filePath = adm_strdup(pstVerifySignAsync->stResult.filePath);
                }
            }
        }
        else if ((D8U)0xff == pstVerifySignAsync->stResult.result)
        {
            //当返回验签结果为0xff时，需要根据调用host接口进行验签处理
            for(i=0; (D32U)i< pfileMetasList->fileMetaNum; i++)
            {
#ifdef SUPPORT_LION_PKI
                if (ADM_SIM_SOA_UCL_FILE_TYPE_ORI_512 == pfileMetasList->fileMetasArray[i]->efiletype)
                {
                    //原文件hash512值
                    originPkgsize = (D32S)pfileMetasList->fileMetasArray[i]->size;
                    poriginPkghash = adm_strdup(pfileMetasList->fileMetasArray[i]->tObject.pAbstracValue);
                    break;
                }
#else
                //原文件hash值,为签名文件存储内容
                if (ADM_SIM_SOA_UCL_FILE_TYPE_ORI == pfileMetasList->fileMetasArray[i]->efiletype)
                {
                    poriginPkghash = adm_strdup(pfileMetasList->fileMetasArray[i]->tObject.pAbstracValue);
                    originPkgsize = adm_strlen(poriginPkghash);
                    break;
                }
#endif          
            }

            if(0 == originPkgsize || ADM_IS_NULL(poriginPkghash))
            {
                iRet = ADM_ECODE_VERIFY_SIGNATURE_FAILED;
                if(NULL != poriginPkghash)
                {
                    adm_free(poriginPkghash);
                }
                log_e("get Verify signature originPkgsize[%d],poriginPkghash[%s] failed", originPkgsize, poriginPkghash);
            }
            else
            {
                //hostinstid
                s32UCMHostInstId = adm_cfg_iniParserGetInt(ADM_CFG_VUS_HOST_INSTANCEID,  ADM_CFG_VUS_HOST_INSTANCEID_DEFAULT_VAL);
                log_d("s32UCMHostInstId[%d]", s32UCMHostInstId);
                //签名数据长度和签名内容
                signatureLen = pstVerifySignAsync->stResult.signatureLen;
                psignature = adm_strdup(pstVerifySignAsync->stResult.signature);
                log_d("signatureLen[%d],signature[%s]", signatureLen, psignature);
                log_d("originPkgsize[%d],poriginPkghash[%s]", originPkgsize, poriginPkghash);

                //host验签接口
                iRet = adm_dim_host_verifySignature((D16U)s32UCMHostInstId, ADM_PUB_SYNC_REQ_TIMEOUT, 
                                        psignature, (D32S)signatureLen, 
                                        poriginPkghash, originPkgsize, &errorCode);
                if (ADM_IS_ERR(iRet) || ADM_IS_ERR(errorCode))
                {
                    iRet = ADM_ECODE_VERIFY_SIGNATURE_FAILED;
                    log_e("host Verify signature failed, errorCode[%d], err[%d]", errorCode, iRet);
                }
                adm_free(psignature);
                adm_free(poriginPkghash);
            }

            //下载时远程验签不需要信息返回，故增加此判断. 安装验签时需要数据需要返回数据
            if(NULL != pstVerifySignResultInfo)
            {
                pstVerifySignResultInfo->result = pstVerifySignAsync->stResult.result;
                pstVerifySignResultInfo->signatureLen = pstVerifySignAsync->stResult.signatureLen;

                if(NULL != pstVerifySignAsync->stResult.signature)
                {
                    pstVerifySignResultInfo->signature = adm_strdup(pstVerifySignAsync->stResult.signature);
                }
                
                if(NULL != pstVerifySignAsync->stResult.filePath)
                {
                    pstVerifySignResultInfo->filePath = adm_strdup(pstVerifySignAsync->stResult.filePath);
                }
            }
        }
        else
        {
            iRet = ADM_ECODE_FAILED;
        }
    }

    pubSetVerifySignEnd(pstVerifySignAsync);
    __adm_pub_delfileMetasListInfo(&pfileMetasList,pfileMetasList->fileMetaNum);

    log_d("ucl verify sign end!");

    return iRet;
}

/**
 * @brief  ucl同步解密接口
 * @note
 * @param  in DCHAR *pcSrcFilePath  待解密的文件路径
 * @param  in DCHAR *pcDstFilePath  解密后的文件路径
 * @param  in DCHAR *pcDecryptKey   解密key
 * @param  in D16U usTimeout        超时时间
 * @retval D32S 0:成功  其他:失败
 */
D32S adm_pub_packageSyncDecrypt(DCHAR *pcSrcFilePath, DCHAR *pcDstFilePath, DCHAR *pcDecryptKey, D16U usTimeout)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    D32S iErrorCode = ADM_ECODE_SUCCESSED;
    D32S s32UclInstId = ADM_ECODE_SUCCESSED;

    /** 入参校验 */
    if (ADM_IS_NULL(pcSrcFilePath) || 
        ADM_IS_NULL(pcDstFilePath) || 
        ADM_IS_NULL(pcDecryptKey))
    {
        log_e("Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }

    log_d("package decrypt, filePath[%s]", pcSrcFilePath);

    /** 获取解密实例ID */
    s32UclInstId = adm_cfg_iniParserGetInt(ADM_CFG_VUS_UCL_INSTANCEID,  ADM_CFG_VUS_UCL_INSTANCEID_DEFAULT_VAL);

    /** 执行解密 */
    iRet = adm_dim_ucl_requestSyncDecryptPackage((D16U)s32UclInstId,
                                                 usTimeout,
                                                 pcSrcFilePath,
                                                 pcDstFilePath,
                                                 pcDecryptKey,
                                                 ADM_PUB_DECRYPT_SEG_LEN_DEFAULT,
                                                 &iErrorCode);
    if (ADM_IS_ERR(iRet) || ADM_IS_ERR(iErrorCode))
    {
        log_e("Failed to request ucl decrypt package, uclInstID[%d], filePath[%s], iRet[%d], errCode[%d], err[%d]", 
                s32UclInstId, pcSrcFilePath, iRet, iErrorCode, ADM_ECODE_AES_ECB_DECRYPT_FAILED);
        iRet = ADM_ECODE_AES_ECB_DECRYPT_FAILED;
    }
    else
    {
        log_d("Success to request ucl decrypt package, filePath[%s]", pcSrcFilePath);
    }

    return iRet;
}

static DVOID adm_pub_get_absTime(D32S iAbsTime, struct timespec *pstAbsTime)
{
    if (NULL == pstAbsTime || 0 >= iAbsTime)
    {
        log_e("Invalid param.");
        return;
    }

    (void)adm_memzero(pstAbsTime, sizeof(struct timespec));
    if (0 == clock_gettime(CLOCK_MONOTONIC, pstAbsTime))
    {
        pstAbsTime->tv_sec += (D64S)iAbsTime / 1000;
        pstAbsTime->tv_nsec += ((D64S)iAbsTime % 1000) * 1000000;
        if (pstAbsTime->tv_nsec >= 1000000000)
        {
            pstAbsTime->tv_sec += 1;
            pstAbsTime->tv_nsec -= 1000000000;
        }
    }

    return;
}

/**
 * @brief 兼容的pthread_cond_timedwait实现
 *
 * @note 解决JNI的集成方式中,pthread_cond_timedwait函数会被信号3打断且函数返回0的问题
 *       1.增加超时时间自行校验
 *
 * @return D32S
 */
D32S adm_pub_pthread_cond_timedwait(pthread_cond_t *pstCond, pthread_mutex_t *pstMutex,
                                    DBOOL *pIsSignal, DBOOL *pIsFlush,
                                    D32S iAbsTime)
{
    D32S iRet = ADM_ECODE_FAILED;
    struct timespec stCurrTime;
    struct timespec stAbsTime;
    D64U uiFlushCnt = 0;

    if (NULL == pstCond || NULL == pstMutex ||
        NULL == pIsSignal || 0 >= iAbsTime)
    {
        log_e("Invalid param.");
        return -1;
    }

    (void)pthread_mutex_lock(pstMutex);

    adm_pub_get_absTime(iAbsTime, &stAbsTime);

    log_i("pIsSignal[%d]", *pIsSignal);
    while (DFALSE == *pIsSignal)
    {
        iRet = pthread_cond_timedwait(pstCond, pstMutex, &stAbsTime);
        if (ETIMEDOUT == iRet)
        {
            break;
        }

        if (ADM_IS_ERR(iRet))
        {
            log_i("Abnormal interrupt. value[%d].", iRet);

            clock_gettime(CLOCK_MONOTONIC, &stCurrTime);

            /** 检查当前时间是否大于超时时间 */
            if (stCurrTime.tv_sec > stAbsTime.tv_sec ||
                (stCurrTime.tv_sec == stAbsTime.tv_sec &&
                 stCurrTime.tv_nsec >= stAbsTime.tv_nsec))
            {
                /** 返回超时 */
                iRet = ETIMEDOUT;
                break;
            }
        }

        // log_i("pthread_cond_timedwait return value[%d].", iRet);

        if (NULL != pIsFlush && DTRUE == *pIsFlush && DFALSE == *pIsSignal)
        {
            if (0 == uiFlushCnt % 10)
            {
                log_i("Flush wait time.");
            }
            *pIsFlush = DFALSE;
            /** 如果需要刷新，则刷新时间重新等待 */
            adm_pub_get_absTime(iAbsTime, &stAbsTime);
            uiFlushCnt++;
        }
    }
    log_i("ret[%d] [%d] [%d]", iRet, *pIsSignal, (*pIsSignal ? ADM_ECODE_SUCCESSED : iRet));

    (void)pthread_mutex_unlock(pstMutex);

    return *pIsSignal ? ADM_ECODE_SUCCESSED : iRet;
}