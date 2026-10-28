#if !defined(LOG_TAG)
#define LOG_TAG             "SDK"         /*!< 宏函数，用于通过模块名的枚举值类型获取模块名称     */
#endif
#include "adm_debug.h"
#include "adm_sdk_selfUpgrade.h"
#include "adm_application_errorCode.h"


static ADM_SDK_APK_UPGRADE_INFO_T g_stApkUpgradeInfo = {};

/**
 * @brief 获取APK对应的版本号
 * 
 * @param[out] ppApkVersion     描述:获取到的APK的版本号 \n
 * 范围值: \n
 * 单位: \n
 * 
 * @return D32S 
 */
D32S adm_sdk_getApkVersion(DCHAR** ppApkVersion)
{
    D32S iRet = ADM_ECODE_FAILED;

    do
    {
        if (DNULL == ppApkVersion || DNULL == g_stApkUpgradeInfo.getApkVersionCb)
        {
            iRet = ADM_ECODE_INVALID_PARAM;
            log_e("Input param is nil, [err:%d]", iRet);
            break;
        }

        iRet = g_stApkUpgradeInfo.getApkVersionCb(ppApkVersion);
        if (ADM_ECODE_SUCCESSED != iRet || (DNULL == *ppApkVersion))
        {
            log_e("getApkVersion is failed, [err:%d]", iRet);
            break;
        }

        log_i("getApkVersion successed, apkVersion:%s", *ppApkVersion);

    } while (FALSE);
    

    return iRet;
}


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
D32S adm_sdk_doApkSelfUpgrade(DCHAR* pApkPackPath, DCHAR* pApkVersion)
{
    D32S iRet = ADM_ECODE_FAILED;

    do
    {
        if (DNULL == pApkPackPath || DNULL == pApkVersion || DNULL == g_stApkUpgradeInfo.doApkSelfUpgradeCb)
        {
            iRet = ADM_ECODE_INVALID_PARAM;
            log_e("Input param is nil, [err:%d]", iRet);
            break;
        }

        iRet = g_stApkUpgradeInfo.doApkSelfUpgradeCb(pApkPackPath, pApkVersion);
        if (ADM_ECODE_SUCCESSED != iRet)
        {
            log_e("doApkSelfUpgrade is failed, [err:%d]", iRet);
            break;
        }

        log_i("doApkSelfUpgrade successed, pApkPackPath:%s, pApkVersion:%s", pApkPackPath, pApkVersion);
    } while (FALSE);
    

    return iRet;
}



/**
 * @brief APK 自升级相关的接口初始化
 * 
 * @param[in] pstSdkApkInfo     描述: \n
 * 范围值: \n
 * 单位: \n
 * 
 * @return D32S 
 */
D32S adm_sdk_selfUpgradeInit(const ADM_SDK_APK_UPGRADE_INFO_T* pstSdkApkInfo)
{
    D32S iRet = ADM_ECODE_FAILED;

    do
    {
        if (DNULL == pstSdkApkInfo)
        {
            iRet = ADM_ECODE_INVALID_PARAM;
            log_e("Input param is nil, [err:%d]", iRet);
            break;
        }

        g_stApkUpgradeInfo.getApkVersionCb = pstSdkApkInfo->getApkVersionCb;
        g_stApkUpgradeInfo.doApkSelfUpgradeCb = pstSdkApkInfo->doApkSelfUpgradeCb;

        iRet = ADM_ECODE_SUCCESSED;
    } while (FALSE);
    

    return iRet;
}


/**
 * @brief APK 自升级模块反初始化
 * 
 * @return DVOID 
 */
DVOID adm_sdk_selfUpgradeDeinit()
{
    g_stApkUpgradeInfo.getApkVersionCb = DNULL;
    g_stApkUpgradeInfo.doApkSelfUpgradeCb = DNULL;

    return;
}