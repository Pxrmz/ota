/**
 * @file    adm_ecm_management.h
 * @brief   ecm get config moudle 
 * @details ecm 模块
 * @author  wangyanpeng
 * @version v0.1.0
 * @date    2020-11-06
 * @copyright www.abupdate.com
 */ 
#ifndef ADM_ECM_MANAGEMENT_H
#define ADM_ECM_MANAGEMENT_H


#include "adm_typedefs.h"
#include "adm_dim_ucl.h"
#include "adm_com_parseFileObject.h"


#ifdef __cplusplus
    extern "C" {
#endif



/*!
 *******************************************************************************
 * ecm模块 配置文件的解密和验签回调函数(ecm模块实现).
 *
 * \note 解密和验签后的文件路径会发生变化，外部使用时需要注意更新解密验签后的路径
 *           pstFileObject->filePath 会发生变化 
 *
 * \param[in]   pstFileObject      文件对象信息
 * \retval      D32S
 *******************************************************************************
 */
extern D32S adm_ecm_decryptAndVerificationSignature(ADM_COM_FILEOBJECT_T *pstFileObject);


#if 0
#endif


/*!
 *******************************************************************************
 * ecm模块初始化.
 *
 * \note
 *
 * \param   none
 * \retval  D32S    0: 成功 non-0：失败
 *******************************************************************************
 */
extern D32S adm_ecm_moduleInit(void);

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
extern void adm_ecm_moduleDeInit(void);


#ifdef __cplusplus
    }
#endif  /* __cplusplus */

#endif  /* ADM_ECM_MANAGEMENT_H */
