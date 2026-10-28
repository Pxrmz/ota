/**
 * @file adm_cvs_checkVersionImpl.h
 * @brief 车辆检测版本模块
 * @details 检测云端是否有新ECU版本
 * @author jibijun
 * @version v0.1.0
 * @date 2020-11-05
 * @copyright www.abupdate.com
 */
#ifndef _ADM_CVS_CHECKVERSIONIMPL_H_
#define _ADM_CVS_CHECKVERSIONIMPL_H_

#include "adm_typedefs.h"
#include "adm_ocs_airProtocolAdapter.h"
#include "adm_cvs_common.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ADM_OCS_CHECK_REQ_ADAPTER_ ADM_OCS_CHECK_REQ_ADAPTER_T;
typedef struct ADM_OCS_CHECK_RESP_ADAPTER_ ADM_OCS_CHECK_RESP_ADAPTER_T;
//typedef struct ADM_CVS_NOTIFY_ ADM_CVS_NOTIFY_T;

typedef enum ADM_CVS_POST_TYPE_
{
    ADM_CVS_CHECK = 1,  //!< 数据文件
    ADM_CVS_VEHICLEINFO,  //!< Log文件
    ADM_CVS_INVALID  //!< 非法类型
} ADM_CVS_POST_TYPE_T;

/**
 * @biref 检测版本实现的回调函数结构体
 *
 */
typedef struct ADM_CVS_IMPL_F_
{
    D32S (*createMessageBody_F)(const ADM_OCS_CHECK_REQ_ADAPTER_T* checkReq, DCHAR** adapterCheckReq); //!< 创建请求消息体的回调函数 打包、签名、加密
    D32S (*parseMessageBody_F)(DCHAR* respBodyFromCloud, ADM_OCS_CHECK_RESP_ADAPTER_T* checkResp);  //!< 解析检测消息体的回调函数 解析、解密、验签
    D32S (*parseVehicleInfoMessageBody_F)(DCHAR* respBodyFromCloud, D32S* status, DCHAR** versionNumber); //!< 解析车端信息消息体的回调函数 解析、解密、验签
    D32S (*submitPost_F)(ADM_OCS_BUSINESS_TYPE_E eBusinessCommType, const DCHAR* messageReq, DCHAR** messageResp, DSIZE* rspStatusCode);  // !< http(s)post方法回调函数
} ADM_CVS_IMPL_F_T;

ADM_CVS_IMPL_F_T* adm_cvs_newImplFBody(void);
void adm_cvs_deleteImplFBody(ADM_CVS_IMPL_F_T* implF);

/**
 * @biref 检测版本实现管理的结构体
 *
 */
typedef struct ADM_CVS_IMPL_
{
    ADM_OCS_CHECK_REQ_ADAPTER_T* checkReq;
    ADM_CVS_IMPL_F_T* implF;
    ADM_OCS_BUSINESS_TYPE_E eBusinessCommType;  //!< 车云交互业务接口标识
    /*out check*/
    ADM_OCS_CHECK_RESP_ADAPTER_T* checkResp;  //!< check返回
    /*out vehicle info*/
    D32S status;//!< vehicle info返回
    DCHAR* versionNumber;
    ADM_CVS_NOTIFY_T* notifyF;
} ADM_CVS_IMPL_T;

ADM_CVS_IMPL_T* adm_cvs_newImplBody(void);
void adm_cvs_deleteImplBody(ADM_CVS_IMPL_T* impl);

/*!
 *******************************************************************************
 * 向云端请求是否有新的ECU版本信息
 *
 * \note    此接口用于向云端请求是否有新的ECU版本信息，用于在application调用检查版本时，
 *          1. verMgr里面的成员必须声明初值
 *
 * \param   verMgr         检查版本管理结构体
 *
 * \retval  D32S            int_32 检查版本是否OK的结果，0：成功，non-0：失败
 *
 *******************************************************************************
 */
extern D32S adm_cvs_checkVersionImpl(ADM_CVS_POST_TYPE_T postType, ADM_CVS_IMPL_T* impl);

#ifdef __cplusplus
}
#endif

#endif  //_ADM_VRM_REGISTER_MANAGER_H_