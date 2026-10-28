/**
 * @file adm_vrs_registerInfoIn.h
 * @author mengwei@abupdate.com
 * @brief 车辆注册数据拼接，注册响应信息的解析等
 * @version 0.1
 * @date 2020-11-04
 * 
 * @copyright Copyright © 2018 All rights 上海艾拉比智能科技有限公司
 * 
 */
#ifndef _ADM_VRS_REGISTER_INFO_IN_H_
#define _ADM_VRS_REGISTER_INFO_IN_H_

#include "adm_typedefs.h"
#include "adm_vrs_registerInfo.h"
#include "adm_vrs_registerManager.h"

#ifdef __cplusplus
extern "C" {
#endif

#define EXPORT_FUN __attribute__((visibility("default")))

/*!
 *******************************************************************************
 * 拼接注册信息.
 *
 * \note    此接口用于生成json格式的注册信息，执行注册前，需要按照json格式生成请求平台的数据。
 * 
 * \param   registerInfo     In  注册请求信息
 * \param   val              Out 注册请求信息拼接成json格式数据
 * 
 * \retval  D32S             int_32 拼接注册信息结果，0：成功，non-0：失败
 * 
 *******************************************************************************
 */
// extern D32S adm_vrs_createRegisterInfoJsonVal(const ADM_VRS_REGISTER_REQ_INFO_T* reqRegisterInfo, json_val** val);


/*!
 *******************************************************************************
 * 解析注册信息.
 *
 * \note    此接口用于平台反馈的json数据信息，解析成响应注册信息。
 * 
 * \param   val              In  注册接口返回的注册响应信息，json格式数据
 * \param   registerInfo     Out 注册响应信息
 * 
 * \retval  D32S             int_32 解析注册信息结果，0：成功，non-0：失败
 * 
 *******************************************************************************
 */
// extern D32S adm_vrs_parseRegisterInfoJsonVal(const json_val* val, ADM_VRS_REGISTER_INFO_T** registerInfo);

/**
 * @brief 更新注册信息
 * @note 此接口用于更新注册信息，注册成功后，调用此接口更新注册信息。
 * 
 * @param pRegInfo          [In] const ADM_VRS_REGISTER_INFO_T*     注册信息
 * @return D32S             0:successed, non-0: failed 
 */
extern D32S adm_vrs_setRegisterInfo(const ADM_VRS_REGISTER_INFO_T* pRegInfo);


/**
 * @brief 设置平台响应时间戳
 * @note 
 * 
 * @param pRegInfo          [In] const ADM_OCS_REGISTER_RESPONSE_INFO_T*     响应信息
 * @return void
 */
extern void adm_vrs_setTimestampInfo(ADM_OCS_REGISTER_RESPONSE_INFO_T* pstRegisterResponseInfo);

#ifdef __cplusplus  // close out "C" linkage in case of c++ compiling
}
#endif
#endif//_ADM_VRS_REGISTER_INFO_IN_H_
