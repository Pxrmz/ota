/**
 * @file    adm_pub_func.h
 * @brief   
 * @details 公共函数处理模块
 * @author  wanglin
 * @version 
 * @date    2022-05-20
 * @copyright www.abupdate.com
 */
#ifndef ADM_PUB_FUNC_H
#define ADM_PUB_FUNC_H

#include <pthread.h>
#include "adm_typedefs.h"
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include "adm_com_parseFileObject.h"
#include "adm_dim_ucl.h"

#ifdef __cplusplus
extern "C" {
#endif

/** 默认分段解密长度 */
#define ADM_PUB_DECRYPT_SEG_LEN_DEFAULT     1*1024*1024

/*!
 * @brief PUB模块
 */
typedef struct stPubHandle
{
    DBOOL            isInited;                      //!< 是否初始化
    pthread_rwlock_t stRWlock;                      //!< 读写锁

} ADM_PUB_HANDLE_T;



/*!
 * @brief 解密异步信息
 */
typedef struct stDecryptAsync
{
    pthread_cond_t       stCond;    //!< 信号量
    pthread_mutex_t      stMutex;   //!< 信号量锁
    DBOOL                bHasResult;   //!< 信息是否更新
    DBOOL                bIsFlush;   //!< 信息是否更新
    DBOOL                bIsUsing;   //!< 是否正在使用
    DCHAR               *pcFileID;  //!< 待解密文件id
    DCHAR               *pcDecryptStoragePath;       //!< 解密后的文件路径
    ADM_DIM_SOA_UCL_DECRYPT_RESULT_INFO_T stResult;  //!< 异步响应结果
}ADM_DECRYPT_ASYNC_S;


/*!
 * @brief 解密队列
 */
typedef struct stPubDecryptList
{
    D32S             s32Cnt;    //!< list长度 
    ADM_DECRYPT_ASYNC_S *pstDecryptAsyncInfo;  //!< 解密异步响应消息
} ADM_PUB_DECRYPTLIST_T;

/*!
 * @brief 解密信息句柄
 */
typedef struct stDecryptHandle
{
    pthread_rwlock_t stRWlock;                   //!< 读写锁
    DBOOL            isInited;                   //!< 模块是否初始化
    ADM_PUB_DECRYPTLIST_T stDecryptList;         //!< 解密队列

} ADM_DECRYPT_HANDLE_T;



/*!
 * @brief 验签异步信息
 */
typedef struct stVerifySignAsync
{
    pthread_cond_t       stCond;    //!< 信号量
    pthread_mutex_t      stMutex;   //!< 信号量锁
    DBOOL                bHasResult;  //!< 信息是否更新
    DBOOL                bIsFlush;   //!< 信息是否更新
    DBOOL                bIsUsing;   //!< 是否正在使用
    DCHAR               *pcFileID;  //!< 验签文件id
    ADM_SYSI4UCL_VERIFY_SIGN_RESULT_INFO_T stResult;  //!< 异步响应结果
}ADM_VERIFYSIGN_ASYNC_S;

/*!
 * @brief 验签队列
 */
typedef struct stPubVerifySignList
{
    D32S             s32Cnt;    //!< list长度 
    ADM_VERIFYSIGN_ASYNC_S* pstVerifySignAsync;  //!< 验签异步响应消息
} ADM_PUB_VERIFYSIGNLIST_T;


/*!
 * @brief 验签信息句柄
 */
typedef struct stVerifySignHandle
{
    pthread_rwlock_t stRWlock;                   //!< 读写锁
    DBOOL            isInited;                   //!< 模块是否初始化
    ADM_PUB_VERIFYSIGNLIST_T stVerifySignList;   //!< 验签队列
} ADM_VERIFYSIGN_HANDLE_T;


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
extern D32S adm_pub_moduleInit(void);

/*!
 *******************************************************************************
 * ecm模块销毁.
 *
 * \note
 *
 * \param   none
 * \retval  none
 *******************************************************************************
 */
extern void adm_pub_moduleDeInit(void);


/*!
 *******************************************************************************
 * decrypt模块初始化.
 *
 * \note
 *
 * \param   none
 * \retval  D32S    0: 成功 non-0：失败
 *******************************************************************************
 */
extern D32S adm_pub_decryptModuleInit(void);

/*!
 *******************************************************************************
 * decrypt模块销毁.
 *
 * \note
 *
 * \param   none
 * \retval  void    0: 成功 non-0：失败
 *******************************************************************************
 */
extern void adm_pub_decryptModuleDeInit(void);

/*!
 *******************************************************************************
 * 解密下载的文件信息
 *
 * \note
 * 
 * \param[in]   pstPkgFileObj  文件对象信息 
 * \param[in]   ppcDecryptPath 解密后的文件存储路径 --- 外部需要释放 
 * \param[in]   pcDownInstId   下载实例id 
 * \retval      D32S
 *******************************************************************************
 */
extern D32S adm_pub_downPackageDecrypt
(
    ADM_COM_FILEOBJECT_T  *pstPkgFileObj,
    DCHAR                **ppcDecryptPath,
    DCHAR                 *pcDownInstId
);

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
extern D32S adm_pub_dimUclDecryptResultHandle
(
    const DCHAR *fileID, 
    const DCHAR *storePath, 
    const ADM_DIM_SOA_UCL_DECRYPT_RESULT_INFO_T *pstDecryptResultInfo, 
    void *priv
);

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
extern D32S adm_pub_dimUclDecryptProgressHandle
(
    const DCHAR *fileID, 
    const ADM_DIM_SOA_UCL_DECRYPT_PROGRESS_INFO_T *pstDecryptProgressInfo, 
    void *priv
);




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
extern D32S adm_pub_VerifySignModuleInit(void);


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
extern void adm_pub_verifySignModuleDeInit(void);



/*!
 *******************************************************************************
 * UCL 服务验签进度通知回调函数
 *
 * \note 主控的 ucl 验签
 * 
 * \param[in]   fileID                 验签文件标识id
 * \param[in]   progress               验签进度信息
 * \param[in]   priv                   私有信息
 * \retval      D32S
 *******************************************************************************
 */
extern D32S adm_pub_dimUclVerifySignProgressHandle
(
    const DCHAR* fileID,
    const D16U progress
);

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
extern D32S adm_pub_dimUclVerifySignResultHandle
(
    const DCHAR* fileID, 
    const ADM_SYSI4UCL_VERIFY_SIGN_RESULT_INFO_T *pstVerifySignResultInfo
);

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
extern D32S adm_pub_downPackageVerifySign
(
    ADM_COM_FILEOBJECT_T  *pstPkgFileObj,
    DCHAR *pcDownInstId,
    DCHAR *pfileid,
    DBOOL isUnpick,
    ADM_SYSI4UCL_VERIFY_SIGN_RESULT_INFO_T *pstVerifySignResultInfo
);

/**
 * @brief  ucl同步解密接口
 * @note
 * @param  in DCHAR *pcSrcFilePath  待解密的文件路径
 * @param  in DCHAR *pcDstFilePath  解密后的文件路径
 * @param  in DCHAR *pcDecryptKey   解密key
 * @param  in D16U usTimeout        超时时间
 * @retval D32S 0:成功  其他:失败
 */
extern D32S adm_pub_packageSyncDecrypt(DCHAR *pcSrcFilePath, DCHAR *pcDstFilePath, DCHAR *pcDecryptKey, D16U usTimeout);

/**
 * @brief 兼容的pthread_cond_timedwait实现
 *
 * @note 解决JNI的集成方式中,pthread_cond_timedwait函数会被信号3打断且函数返回0的问题
 *       1.增加超时时间自行校验
 *
 * @return D32S
 */
extern D32S adm_pub_pthread_cond_timedwait(pthread_cond_t *pstCond, pthread_mutex_t *pstMutex,
                                           DBOOL *pIsSignal, DBOOL *pIsFlush, D32S ulAbsTime);

#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif 

