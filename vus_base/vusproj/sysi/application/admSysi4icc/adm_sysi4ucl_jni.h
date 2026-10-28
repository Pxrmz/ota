#ifndef __ADM_SYSI4UCL_JNI_H__
#define __ADM_SYSI4UCL_JNI_H__


#include "lion_ota_oemUpgrade_if.h"


#ifdef __cplusplus
extern "C" {
#endif


//jni函数类型
typedef D32S (*doUpgradeTriggerCallBack)(D32S iInstallType, DBOOL bIsRollback, DCHAR* pstImageInfo);
typedef D32S (*getUpgradeProgressCallback)(D32S iInstallType, D32S* pErrorCode);
typedef D32S (*getUpgradeResultCallback)(D32S iInstallType, D32S* pErrorCode);
typedef D32S (*updateAndRebootCallback)(DVOID);
typedef D32S (*upgradeWillStartCallback)(DVOID);
/*****************************************APK Self Upgrade*******************************************/
typedef D32S(*getApkVersionCallback)(DCHAR** ppApkVersion);
typedef D32S(*doApkSelfUpgradeCallback)(DCHAR* pApkPackPath, DCHAR* pApkVersion, D32S iRequestId);
/*****************************************APK Self Upgrade*******************************************/
typedef D32S (*prereductionCallback)(DCHAR* imagePath, D32S* pErrorCode);
typedef D32S (*notifyMcuAndScreenUpgradeCallback)(D32S upgradeProgress,D32S requestID, D32S upgradeProportion, D32S currentEcuUpgradeTime, D32S remainingUpgradeTime, D32S* pErrorCode);


typedef struct
{
    doUpgradeTriggerCallBack          doUpgradeTriggerCb;
    getUpgradeProgressCallback        getUpgradeProgressCb;
    getUpgradeResultCallback          getUpgradeResultCb;
    updateAndRebootCallback           updateAndRebootCb;
    upgradeWillStartCallback          upgradeWillStartCb;
    /*****************************************APK Self Upgrade*******************************************/
    getApkVersionCallback              getApkVersionCb;         ///< 获取 APK 的版本号
    doApkSelfUpgradeCallback           doApkSelfUpgradeCb;      ///< 触发 APK 执行升级
    /*****************************************APK Self Upgrade*******************************************/
    prereductionCallback    prereductionCb;
    notifyMcuAndScreenUpgradeCallback    notifyMcuAndScreenUpgradeCb;
}ADM_SYSI4UCL_JNI_CALLBACK_T;


//进度通知函数和结果通知函数
D32S sysi4ucl_upgrade_progress_ntf(const ADM_SYSI4UCL_ICC_INSTALL_TYPE_E installType, const D32U progress);
D32S sysi4ucl_upgrade_result_ntf(const ADM_SYSI4UCL_ICC_INSTALL_TYPE_E installType, const D32S result);



D32S adm_sysi4ucl_jni_Init(ADM_SYSI4UCL_JNI_CALLBACK_T* pstJniCb);
D32S adm_sysi4ucl_jni_Deinit(DVOID);




#ifdef __cplusplus
}
#endif


#endif  //__ADM_SYSI4UCL_JNI_H__


