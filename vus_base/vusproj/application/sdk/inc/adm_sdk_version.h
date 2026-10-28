/**
 * @file adm_sdk_version.h
 * @author mengwei@abupdate.com
 * @brief 定义当前程序或库文件的版本号管理接口，包含：读、写等
 * @version 0.1
 * @date 2021-10-18
 * 
 * @copyright Copyright © 2018 All rights 上海艾拉比智能科技有限公司
 * 
 */
#ifndef __ADM_SDK_VERSION_H__
#define __ADM_SDK_VERSION_H__

#ifdef __cplusplus
extern "C" {
#endif

#define EXPORT_FUN __attribute__((visibility("default")))

#include "adm_typedefs.h"

#define ADM_OTA_DEFAULT_VERSION     "0.0.0_default_version"

/**
 * @brief 获取所有模块的版本号
 * @note 
 * 
 * @return const DCHAR*     NULL:Failed, None-NULL: json格式字符串，用于上传所有模块版本号
 * @note 返回值示例：
 *  [
 *      {
 *          "versionName":"admotaapp",
 *          "versionValue":"v1.1.1"
 *      },
 *      {
 *          "versionName":"engine",
 *          "versionValue":"v1.1.1"
 *      }
 *  ]
 */
extern DCHAR* adm_sdk_getAllModulesVersionFormatJsonStr(void);

/**
 * @brief 获取OTA程序的主版本号，包括：APP、foundation、vrpc、eweb、sysi等模块的主版本号
 * 
 * @return DCHAR*   NULL:Failed，None-NULL:返回的主版本号
 */
extern const DCHAR* adm_sdk_getMajorVersion(void);

/**
 * @brief 获取当前sdk的版本号，例如：APP、foundation、vrpm、eweb、sysi等各sdk
 * 
 * @return DCHAR*   NULL:Failed，None-NULL:返回的sdk版本号
 */
extern const DCHAR* adm_sdk_getSDKVersion(void);

#ifdef __cplusplus  // close out "C" linkage in case of c++ compiling
}
#endif

#endif//__ADM_SDK_VERSION_H__