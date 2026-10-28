#ifndef __ADM_EMS_SERVER_TYPE_H__
#define __ADM_EMS_SERVER_TYPE_H__

#include "adm_typedefs.h"

#ifdef __cplusplus
extern "C" {
#endif


#define ADM_EMS_SERVER_TYPE_DEFAULT     1       ///< 默认云端环境标识
#define ADM_EMS_SERVER_TYPE_DEFAULT_2   2       ///< 第二套云端环境标识

/**
 * @brief 更新OTA平台环境标识
 * 
 * @param[in] uiServerType     描述: \n
 * 范围值: \n
 * 单位: \n
 * 
 * @return D32S 
 */
extern D32S adm_ems_switchServerType(void);


/**
 * @brief 获取OTA平台环境标识
 * 
 * @param[out] uiServerType     描述: \n
 * 范围值: \n
 * 单位: \n
 * 
 * @return D32S 
 */
extern D32S adm_ems_getServerType(D32U* uiServerType);


/**
 * @brief OTA平台标识初始化
 * 
 * @return D32S 
 */
extern D32S adm_ems_serverTypeInit(void);

#ifdef __cplusplus
}
#endif

#endif//__ADM_EMS_SERVER_TYPE_H__