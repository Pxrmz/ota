#ifndef __ADM_SDK_SELF_UPGRADE_H__
#define __ADM_SDK_SELF_UPGRADE_H__

#ifdef __cplusplus
extern "C" {
#endif

#define EXPORT_FUN __attribute__((visibility("default")))

#include "adm_typedefs.h"

/*****************************************APK Self Upgrade*******************************************/
typedef D32S(*getApkVersionCallback)(DCHAR** ppApkVersion);
typedef D32S(*doApkSelfUpgradeCallback)(DCHAR* pApkPackPath, DCHAR* pApkVersion);
/*****************************************APK Self Upgrade*******************************************/

/**
 * @brief APK SDK 自升级相关的接口
 * 
 */
typedef struct 
{
    /*****************************************APK Self Upgrade*******************************************/
    getApkVersionCallback              getApkVersionCb;         ///< 获取 APK 的版本号
    doApkSelfUpgradeCallback           doApkSelfUpgradeCb;      ///< 触发 APK 执行升级
    /*****************************************APK Self Upgrade*******************************************/
} ADM_SDK_APK_UPGRADE_INFO_T;


/**
 * @brief 获取APK对应的版本号
 * 
 * @param[out] ppApkVersion     描述:获取到的APK的版本号 \n
 * 范围值: \n
 * 单位: \n
 * 
 * @return D32S 
 */
extern D32S adm_sdk_getApkVersion(DCHAR** ppApkVersion);


/**
 * @brief 执行 APK 自升级
 * 
 * @param[in] pApkPackPath     描述:APK升级包的存储路径 \n
 * 范围值: \n
 * 单位: \n
 * 
 * @param[in] pApkVersion     描述:当前升级包对应的版本 \n
 * 范围值: \n
 * 单位: \n
 * 
 * @return D32S 
 */
extern D32S adm_sdk_doApkSelfUpgrade(DCHAR* pApkPackPath, DCHAR* pApkVersion);



/**
 * @brief APK 自升级相关的接口初始化
 * 
 * @param[in] pstSdkApkInfo     描述: \n
 * 范围值: \n
 * 单位: \n
 * 
 * @return D32S 
 */
extern D32S adm_sdk_selfUpgradeInit(const ADM_SDK_APK_UPGRADE_INFO_T* pstSdkApkInfo);


/**
 * @brief APK 自升级模块反初始化
 * 
 * @return DVOID 
 */
extern DVOID adm_sdk_selfUpgradeDeinit();


#ifdef __cplusplus  // close out "C" linkage in case of c++ compiling
}
#endif

#endif//__ADM_SDK_SELF_UPGRADE_H__