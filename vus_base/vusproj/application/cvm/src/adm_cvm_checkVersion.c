#include "adm_cvm_checkVersion.h"
#include "adm_cvm_log.h"
#include "adm_engine_common.h"
#include "adm_cvs_checkVersion.h"
#include "adm_cvs_versionPolicy.h"
#include "adm_ocs_airProtocolAdapter.h"
#include "adm_ocm_interface.h"
#include "adm_ecs_rsConfig.h"
#include "adm_com_errorCode.h"
#include "adm_memory.h"
#include "adm_string.h"
#include "adm_com_modDefine.h"
#include "adm_eim_ecuInfoManager.h"
#include "adm_com_kvstorage.h"
#include "adm_dpo_download.h"
#include "adm_com_lib.h"
#include "adm_file.h"
#include "adm_cfg_api.h"
#include "adm_irm_report.h"
#include "adm_sdk_version.h"
#include "adm_ecm_management.h"
#include "adm_cvm_event.h"
#include "adm_application_errorCode.h"
#include "adm_pub_func.h"
#include "adm_ecs_kvStorage.h"
#include "adm_cfg_default.h"
#include "adm_vrs_registerInfo.h"
#include "adm_vsm_fsm.h"

#include <time.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>

#define  gPrivacyPath   "/storage/emulated/0/Documents/privacyPolicy/"

static D64S g_ulTspTaskId = 0;
/**
 * @brief 获取U盘升级包路径
 * @note
 * @param out DCHAR **ppcFilePath 升级包路径
 * @retval D32S 0:成功 非0:失败
 */
static D32S cvmGetUsbPackagePath(DCHAR **ppcFilePath)
{
    D32S iRet = ADM_ECODE_FAILED;
    DIR *dir;
    struct dirent *entry;
    DCHAR aFilepath[1024] = {0};
    DCHAR *pcUsbPath = NULL;

    ADM_COM_FUNC_ENTRY();

    /** 入参校验 */
    if (ADM_IS_NULL(ppcFilePath))
    {
        log_e("invalid param");
        return ADM_ECODE_INVALID_PARAM;
    }

    /** 打开U盘目录 */
    pcUsbPath = adm_cvs_getUsbDir();
    if (ADM_IS_NULL(pcUsbPath))
    {
        log_e("adm_cvs_getUsbDir failed");
        return ADM_ECODE_KV_GET_STRING_FAILED;
    }

    // pcUsbPath = (DCHAR *)adm_cfg_iniParserGetString(ADM_CFG_EWEB_USB_ROOT_DIR, ADM_CFG_EWEB_USB_ROOT_DIR_DEFAULT_VAL);
    log_d("usb path is %s", pcUsbPath);
    if (!(dir = opendir(pcUsbPath)))
    {
        log_e("Error opening directory, errno = %s", strerror(errno));
        return ADM_ECODE_USB_DIR_OPEN_FAILED;
    }

    /** 循环遍历目录，找到带U盘任务包前缀的文件 */
    while ((entry = readdir(dir)) != NULL)
    {
        if (adm_strstr(entry->d_name, ADM_CVS_USB_UPDATE_PACKAGE_PREFIX))
        {
            snprintf(aFilepath, sizeof(aFilepath), "%s%s", pcUsbPath, entry->d_name);
            *ppcFilePath = adm_strdup(aFilepath);
            iRet = ADM_ECODE_SUCCESSED;
            log_d("find file %s", aFilepath);
            break;
        }
    }

    closedir(dir);

    adm_free(pcUsbPath);
    ADM_COM_FUNC_EXIT();

    return iRet;
}

/**
 * @brief 获取U盘策略文件路径
 * @note
 * @param out DCHAR **ppcFilePath 策略文件路径
 * @retval D32S 0:成功 非0:失败
 */
static D32S cvmGetUsbPolicyFilePath(DCHAR **ppcFilePath)
{
    D32S iRet = ADM_ECODE_FAILED;
    DIR *dir;
    struct dirent *entry;
    DCHAR aFilepath[1024] = {0};
    DCHAR *pcUsbDownloadPath = NULL;
    ADM_COM_FUNC_ENTRY();

    /** 入参校验 */
    if (ADM_IS_NULL(ppcFilePath))
    {
        log_e("invalid param");
        return ADM_ECODE_INVALID_PARAM;
    }

    /** 打开U盘下载目录 */
    pcUsbDownloadPath = adm_cvs_getUsbDownloadDir();
    if (ADM_IS_NULL(pcUsbDownloadPath))
    {
        log_e("adm_cvs_getUsbDownloadDir failed");
        return ADM_ECODE_KV_GET_STRING_FAILED;
    }

    log_d("usb download path is %s", pcUsbDownloadPath);
    if (!(dir = opendir(pcUsbDownloadPath)))
    {
        log_e("Error opening directory");
        return ADM_ECODE_USB_DIR_OPEN_FAILED;
    }

    /** 循环遍历目录，找到带U盘任务策略后缀的文件 */
    while ((entry = readdir(dir)) != NULL)
    {
        if (adm_strstr(entry->d_name, ADM_CVS_USB_UPDATE_POLICY_FILE_POSTFIX))
        {
            snprintf(aFilepath, sizeof(aFilepath), "%s%s", pcUsbDownloadPath, entry->d_name);
            *ppcFilePath = adm_strdup(aFilepath);
            iRet = ADM_ECODE_SUCCESSED;
            log_d("find file %s", aFilepath);
            break;
        }
    }

    closedir(dir);

    adm_free(pcUsbDownloadPath);
    ADM_COM_FUNC_EXIT();

    return iRet;
}

/**
 * @brief 检查升级包是否匹配当前车辆类型
 * @note
 * @param in DCHAR *pcPackagePath 升级包路径
 * @retval D32S 0:成功 非0:失败
 */
static D32S cvmCheckUsbPackage(DCHAR *pcPackagePath)
{
    D32S iRet = ADM_ECODE_FAILED;
    DCHAR aVehicleType[ADM_COM_LEN_256] = {0};
    ADM_VRS_REGISTER_INFO_T *pstRegisterInfo = NULL;
    
    ADM_COM_FUNC_ENTRY();

    /** 入参校验 */
    if (ADM_IS_NULL(pcPackagePath))
    {
        log_e("invalid param");
        return ADM_ECODE_INVALID_PARAM;
    }

    do
    {
        /** 通过注册信息获取车辆类型 */
        pstRegisterInfo = adm_vrs_newRegisterInfo();
        if (NULL == pstRegisterInfo)
        {
            log_e("adm_vrs_newRegisterInfo failed");
            break;
        }

        /** 获取注册信息 */
        iRet = adm_vrs_getRegisterInfo(pstRegisterInfo);
        if (ADM_ECODE_SUCCESSED != iRet)
        {
            log_e("adm_vrs_getRegisterInfo failed, err:%d", iRet);
            break;
        }

        log_d("task package path[%s], real vehicle type[%s]", pcPackagePath, pstRegisterInfo->pcVehlType);

        /** 判断包名中是否包含对应车型 */
        if (ADM_IS_NULL(adm_strstr(pcPackagePath, pstRegisterInfo->pcVehlType)))
        {
            iRet = ADM_ECODE_INVALID_POINTER;
            log_e("invalid pointer error[%d]", iRet);
            break;
        }
        else
        {
            iRet = ADM_ECODE_SUCCESSED;
        }
    } while (FALSE);

    adm_vrs_delRegisterInfo(pstRegisterInfo);

    ADM_COM_FUNC_EXIT();

    return iRet;
}

/**
 * @brief 获取解密升级包路径
 * @note
 * @param in DVOID
 * @retval DCHAR * 解密升级包路径
 */
static DCHAR *cvmGetUsbPackageDecryptFilePath(DVOID)
{
    DCHAR szDecryptFilePath[ADM_COM_LEN_256] = {0};
    DCHAR *pcFilePath = NULL;
    DCHAR *pcUsbDir = NULL;

    /** 获取U盘根目录 */
    pcUsbDir = adm_cvs_getUsbDir();
    if (ADM_IS_NULL(pcUsbDir))
    {
        log_e("adm_cvs_getUsbDir failed");
        return NULL;
    }
    log_d("usb path is %s", pcUsbDir);

    /** 拼接解密文件路径 */
    adm_snprintf(szDecryptFilePath, sizeof(szDecryptFilePath), "%s%s", pcUsbDir, ADM_CVS_USB_DECRYPT_PACKAGE_NAME);

    pcFilePath = adm_strdup(szDecryptFilePath);

    adm_free(pcUsbDir);
    return pcFilePath;
}

/**
 * @brief 获取升级包解密key
 * @note
 * @param in DCHAR *pcPackagePath 升级包路径
 * @param out DCHAR **ppcDecryptKey 解密key
 * @retval D32S 0:成功 非0:失败
 */
static D32S cvmUsbGetPackageDecryptKey(DCHAR *pcPackagePath, DCHAR **ppcDecryptKey)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    DCHAR *pcKeyStart = NULL;
    DCHAR *pcKeyEnd = NULL;
    DCHAR szTmpKey[ADM_COM_LEN_256] = {0};

    ADM_COM_FUNC_ENTRY();

    /** 入参校验 */
    if (ADM_IS_NULL(ppcDecryptKey) || ADM_IS_NULL(pcPackagePath))
    {
        log_e("invalid param");
        return ADM_ECODE_INVALID_PARAM;
    }

    do
    {
        /** 从后往前，找到第一个'_' */
        pcKeyStart = adm_strrchr(pcPackagePath, '_');
        if (ADM_IS_NULL(pcKeyStart))
        {
            iRet = ADM_ECODE_INVALID_POINTER;
            log_e("invalid pointer error[%d]", iRet);
            break;
        }

        /** 从后往前，找到第一个'.' */
        pcKeyEnd = adm_strrchr(pcPackagePath, '.');
        if (ADM_IS_NULL(pcKeyEnd))
        {
            iRet = ADM_ECODE_INVALID_POINTER;
            log_e("invalid pointer error[%d]", iRet);
            break;
        }

        adm_memcpy(szTmpKey, pcKeyStart + 1, pcKeyEnd - pcKeyStart - 1, ADM_COM_LEN_256);
        log_d("decrypt key is %s", szTmpKey);
        *ppcDecryptKey = adm_strdup(szTmpKey);
    } while (FALSE);
    
    ADM_COM_FUNC_EXIT();

    return iRet;
}

static void cvmfillFileObjToDecrypt(ADM_COM_FILEOBJECT_T *pstPkgFileObj, DCHAR *pcSrcFilePath,DCHAR *pcDecryptKey)
{
    if (ADM_IS_NULL(pstPkgFileObj) || ADM_IS_NULL(pcSrcFilePath)|| ADM_IS_NULL(pcDecryptKey))
    {
        log_e("Invalid input param");
        return ;
    }

    pstPkgFileObj->fileName = adm_strdup(ADM_CVS_USB_DECRYPT_PACKAGE_NAME);
    pstPkgFileObj->filePath = adm_strdup(pcSrcFilePath);
    pstPkgFileObj->hash = NULL;
    pstPkgFileObj->size = 0;
    pstPkgFileObj->url = NULL;
    pstPkgFileObj->pstFileSecurityInfo = (ADM_COM_FILESCYINFO_T*)adm_malloc(sizeof(ADM_COM_FILESCYINFO_T));
    if (NULL != pstPkgFileObj->pstFileSecurityInfo)
    {
        pstPkgFileObj->pstFileSecurityInfo->isEncrypt = 1;
        pstPkgFileObj->pstFileSecurityInfo->oriHash = adm_strdup(pcDecryptKey);
        pstPkgFileObj->pstFileSecurityInfo->oriSize = 0;
        pstPkgFileObj->pstFileSecurityInfo->pkgKey  = adm_strdup(pcDecryptKey);
        pstPkgFileObj->pstFileSecurityInfo->segmentDecryptLen = ADM_PUB_DECRYPT_SEG_LEN_DEFAULT;
        pstPkgFileObj->pstFileSecurityInfo->fileMetaNum = 0;
        pstPkgFileObj->pstFileSecurityInfo->pstFileMetaList = NULL;
    }

    return ;
}



/**
 * @brief U盘任务包解密
 * @note
 * @param in DCHAR *pcPackagePath 升级包路径
 * @param in DCHAR *pcDecryptFilePath 解密文件路径
 * @retval D32S 0:成功 非0:失败
 */
static D32S cvmUsbPackageDecrypt(DCHAR *pcPackagePath, DCHAR *pcDecryptFilePath)
{
    D32S iRet = ADM_ECODE_FAILED;
    DCHAR *pcDecryptKey = NULL;
    D32S s32UclInstId = 0;
    DCHAR szUclInstId[10] = {0};
    ADM_COM_FILEOBJECT_T stPkgFileObj = {0};
    DCHAR *pcDecryptPath = NULL;

    ADM_COM_FUNC_ENTRY();

    /** 入参校验 */
    if (ADM_IS_NULL(pcPackagePath) || ADM_IS_NULL(pcDecryptFilePath))
    {
        log_e("invalid param");
        return ADM_ECODE_INVALID_PARAM;
    }

    do
    {
        /** 如果文件存在，则先删除 */
        if (TRUE == adm_com_isFileExist(pcDecryptFilePath))
        {
            adm_com_rmFile(pcDecryptFilePath);
        }

        /** 获取解密key */
        iRet = cvmUsbGetPackageDecryptKey(pcPackagePath, &pcDecryptKey);
        if (ADM_IS_ERR(iRet)) 
        {
            log_e("cvmUsbGetPackageDecryptKey. return error[%d]", iRet);
            break;
        }

        /** 升级包解密 */
        s32UclInstId = adm_cfg_iniParserGetInt(ADM_CFG_VUS_UCL_INSTANCEID,  ADM_CFG_VUS_UCL_INSTANCEID_DEFAULT_VAL);
        (void)adm_snprintf(szUclInstId, (D32U)sizeof(szUclInstId), "%x", s32UclInstId);
        cvmfillFileObjToDecrypt(&stPkgFileObj, pcPackagePath, pcDecryptKey);
        iRet = adm_pub_downPackageDecrypt(&stPkgFileObj, &pcDecryptPath, szUclInstId);
        log_i("adm_pub_downPackageDecrypt. return [%d]",iRet);

        adm_free(pcDecryptPath);
        adm_com_cleanFileObject(&stPkgFileObj);
        
        //iRet = adm_pub_packageSyncDecrypt(pcPackagePath, 
        //                                  pcDecryptFilePath, 
        //                                  pcDecryptKey,
        //                                  ADM_CVM_USB_PACKAGE_DECRYPT_TIMEOUT);
    } while (FALSE);
    
    /** 内存释放 */
    adm_free(pcDecryptKey);

    ADM_COM_FUNC_EXIT();

    return iRet;
}


 /**
  * @brief 设置TSP任务ID
  * @note
  * @param in D64S ulTspTaskId TSP任务ID
  * @retval 无
  */
void adm_cvm_setTspTaskId(D64S ulTspTaskId)
{
    log_d("set tsp task id = %lld", ulTspTaskId);
    g_ulTspTaskId = ulTspTaskId;
    return;
}
 /**
  * @brief 获取TSP任务ID
  * @note
  * @retval 当前TSP任务的ID，类型为D64S
  */
D64S adm_cvm_getTspTaskId(void)
{
    return g_ulTspTaskId;
}
/**
 * @brief U盘任务包处理
 * @note 包括解密、解压
 * @param in void
 * @retval D32S  0 成功，其他失败
 */
static D32S usbTaskPackageHandle(void)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    DCHAR *pcPackagePath = NULL;
    DCHAR *pcUsbDownloadPath = NULL;
    DCHAR *pcDecryptPackagePath = NULL;

    ADM_COM_FUNC_ENTRY();

    do
    {
        /** 在U盘目录下找到任务包（根据zip后缀） */
        iRet = cvmGetUsbPackagePath(&pcPackagePath);
        if (ADM_ECODE_SUCCESSED != iRet) 
        {
            log_e("cvmGetUsbPackagePath. return error[%d]", iRet);
            iRet = ADM_ECODE_DOWNLOAD_NO_PACKAGE_FAILED;
            break;
        }

        /** 校验任务包对应的车型 */
        iRet = cvmCheckUsbPackage(pcPackagePath);
        if (ADM_ECODE_SUCCESSED != iRet)
        {
            log_e("cvmCheckUsbPackage. return error[%d]", iRet);
            break;
        }

        pcDecryptPackagePath = cvmGetUsbPackageDecryptFilePath();
        if (ADM_IS_NULL(pcDecryptPackagePath))
        {
            iRet = ADM_ECODE_INVALID_POINTER;
            log_e("cvmGetUsbPackageDecryptFilePath. return error[%d]", iRet);
            break;
        }

        /** 任务包解密 */
        iRet = cvmUsbPackageDecrypt(pcPackagePath, pcDecryptPackagePath);
        if (ADM_ECODE_SUCCESSED != iRet) 
        {
            log_e("cvmUsbPackageDecrypt. return error[%d]", iRet);
            break;
        }

        pcUsbDownloadPath = adm_cvs_getUsbDownloadDir();
        if (ADM_IS_NULL(pcUsbDownloadPath))
        {
            log_e("adm_cvs_getUsbDownloadDir failed");
            break;
        }
        log_d("usb download path is %s", pcUsbDownloadPath);
        
        /** 解压前判断解压目录是否存在，存在则需要删除 */
        if (TRUE == adm_com_isDirExist(pcUsbDownloadPath))
        {
            (void)adm_com_rmFile(pcUsbDownloadPath);
        }

        iRet = adm_com_createDir(pcUsbDownloadPath);
        if (ADM_ECODE_SUCCESSED != iRet) 
        {
            log_e("adm_com_createDir. return error[%d]", iRet);
            break;
        }

        /** 任务包解压 */
        iRet = adm_com_unZipFile(pcDecryptPackagePath, pcUsbDownloadPath);
        if (ADM_ECODE_SUCCESSED != iRet) 
        {
            log_e("adm_com_unZipFile. return error[%d]", iRet);
            break;
        }
        else
        {
            log_i("adm_com_unZipFile. successed");
        }
        
        /** 解压成功后，删除解密后的任务包文件 */
        iRet = adm_com_rmFile(pcDecryptPackagePath);
        if (ADM_ECODE_SUCCESSED != iRet) 
        {
            log_e("adm_com_rmFile. return error[%d]", iRet);
            break;
        }
    } while (FALSE);
    
    adm_free(pcPackagePath);
    adm_free(pcDecryptPackagePath);
    adm_free(pcUsbDownloadPath);

    ADM_COM_FUNC_EXIT();
    
    return iRet;
}

D32S adm_cvm_getReqFieldValue(D64S* timestamp,
    DCHAR** netType,
    DCHAR** versionNumber,
    DCHAR** sdkVersion,
    DCHAR** allModuleVerInfoStr)
{
    D32S retVal = ADM_ECODE_SUCCESSED;

/*TODO: get value*/
#ifdef ADM_SDK_VERSION
    *sdkVersion = adm_strdup(ADM_SDK_VERSION);
#else
    *sdkVersion = adm_strdup("v5.0");
#endif
    /*option*/
    if (ADM_ECODE_SUCCESSED != adm_eim_getVehicleBigVersion(versionNumber)) {
        log_e("get Vehicle Big Version,warning");
        *versionNumber = NULL;
    }
    *netType = adm_strdup("WiFi");
    *timestamp = time(NULL);
    *allModuleVerInfoStr = adm_sdk_getAllModulesVersionFormatJsonStr();
    return retVal;
}

static D32S adm_cvm_downloadFile(const DCHAR* url, const DCHAR* hash, const D64S fileSize, const DCHAR* savePath)
{
    if (NULL == url || NULL == savePath || 0 == fileSize) {
        log_e("adm_cvs_downloadFile input param invalid");
        return ADM_ECODE_INVALID_PARAM;
    }
    D32S retVal = ADM_ECODE_SUCCESSED;
    ADM_DPO_DOWNLOAD_REQPARAM_T* downloadInfo = NULL;
    downloadInfo = adm_dpo_newDownReqParamInfo();
    if(NULL == downloadInfo)
    {
        log_e("adm_cvs_downloadFile Out of memory");
        return ADM_ECODE_NO_MEMORY;
    }
    downloadInfo->url = adm_strdup(url);
    downloadInfo->filePath = adm_strdup(savePath);
    downloadInfo->hash = adm_strdup(hash);
    downloadInfo->fileSize = fileSize;
    downloadInfo->IsUnlimitretry = FALSE;
    downloadInfo->networkRetriesCnt = 0;
    downloadInfo->fnDownloadCb = adm_ocm_fileDownloadInterface;
    retVal = adm_dpo_doDownloadFile(downloadInfo);
    adm_dpo_delDownReqParamInfo(&downloadInfo);
    return retVal;
}

static D32S adm_cvm_hmiFileSecurityInfo(ADM_OCS_CHECK_HMI_MEMBER_T* hmiMember, const DCHAR* fullFilePath)
{
    if (NULL == hmiMember || NULL == hmiMember->fileSecurityInfo || NULL == fullFilePath) {
        return ADM_ECODE_INVALID_PARAM;
    }

    D32S retVal = ADM_ECODE_SUCCESSED;
    ADM_COM_FILEOBJECT_T* fileObject = NULL;
    ADM_COM_FILEINFOSET_T* fileInfoSet = NULL;

    do {
        if (1 == hmiMember->fileSecurityInfo->isEncrypt) {
            log_i("hmi file is encrypt");
            fileObject = adm_com_newFileObject();
            if (NULL == fileObject) {
                retVal = ADM_ECODE_NO_MEMORY;
                log_e("adm_com_newFileObject. return failed");
                break;
            }
            fileObject->fileName = adm_strdup(hmiMember->hmiShowFileName);
            fileObject->filePath = adm_strdup(fullFilePath);
            fileObject->hash = adm_strdup(hmiMember->hmiShowFileHash);
            fileObject->pstFileSecurityInfo = hmiMember->fileSecurityInfo;
            fileObject->size = hmiMember->hmiShowFileSize;
            fileObject->url = adm_strdup(hmiMember->hmiShowFileUrl);
            log_i("hmi policy file decrypt verification signature");
            retVal = adm_ecm_decryptAndVerificationSignature(fileObject);
            fileObject->pstFileSecurityInfo = NULL;
            if (ADM_ECODE_SUCCESSED != retVal) {
                log_e("adm_ecm_decryptAndVerificationSignature. return error[%d]", retVal);
                break;
            }
            /*解密验签后文件路径会发生变化，重命名到eweb下载路径下*/
            if (ADM_ERR_OK != adm_frename(fileObject->filePath, fullFilePath)) {
                log_e("rename file failed");
                retVal = ADM_ECODE_FAILED;
                break;
            }
        } else {
            log_i("hmi file is not encrypt");
        }

        fileInfoSet = adm_com_newFileInfoSet();
        if (NULL == fileInfoSet) {
            retVal = ADM_ECODE_NO_MEMORY;
            log_e("adm_com_newFileObject. return failed");
            break;
        }
        log_i("get hmi policy origin file size");
        retVal = adm_com_getOriginAndSignFlieLen(hmiMember->fileSecurityInfo, fileInfoSet);
        if (ADM_ECODE_SUCCESSED != retVal) {
            log_e("adm_com_getOriginAndSignFlieLen. return error[%d]", retVal);
            break;
        }
        log_i("get hmi policy origin file hash");
        retVal = adm_com_getOriginAndSignFliehash(hmiMember->fileSecurityInfo, fileInfoSet);
        if (ADM_ECODE_SUCCESSED != retVal) {
            log_e("adm_com_getOriginAndSignFlieLen. return error[%d]", retVal);
            break;
        }
        hmiMember->ucmHmiPolicyFileHash = adm_strdup(fileInfoSet->originPkgFileHash);
        hmiMember->ucmHmiPolicyFileSize = fileInfoSet->originPkgFileLen;
        log_i("hmi policy origin file hash = %s", hmiMember->ucmHmiPolicyFileHash);
        log_i("hmi policy origin file size = %d", hmiMember->ucmHmiPolicyFileSize);
        retVal = ADM_ECODE_SUCCESSED;
    } while (FALSE);
    adm_com_delFileObject(&fileObject);
    adm_com_delFileInfoSet(&fileInfoSet);
    return retVal;
}
static D32S adm_cvm_privacyFileSecurityInfo(ADM_OCS_CHECK_PRIVACY_MEMBER_T* privacyMember, const DCHAR* fullFilePath, const DCHAR* policySavePath)
{
    if (NULL == privacyMember || NULL == privacyMember->fileSecurityInfo || NULL == fullFilePath) {
        return ADM_ECODE_INVALID_PARAM;
    }
	
    DCHAR policyFilePath[256];
    D32S retVal = ADM_ECODE_SUCCESSED;
    ADM_COM_FILEOBJECT_T* fileObject = NULL;
    ADM_COM_FILEINFOSET_T* fileInfoSet = NULL;

    do {
        if (1 == privacyMember->fileSecurityInfo->isEncrypt) {
            log_i("privacy file is encrypt");
            fileObject = adm_com_newFileObject();
            if (NULL == fileObject) {
                retVal = ADM_ECODE_NO_MEMORY;
                log_e("adm_com_newFileObject. return failed");
                break;
            }
            fileObject->fileName = adm_strdup(privacyMember->privacyShowFileName);
            fileObject->filePath = adm_strdup(fullFilePath);
            fileObject->hash = adm_strdup(privacyMember->privacyShowFileHash);
            fileObject->pstFileSecurityInfo = privacyMember->fileSecurityInfo;
            fileObject->size = privacyMember->privacyShowFileSize;
            fileObject->url = adm_strdup(privacyMember->privacyShowFileUrl);
            log_i("privacy policy file decrypt verification signature");
            retVal = adm_ecm_decryptAndVerificationSignature(fileObject);
            fileObject->pstFileSecurityInfo = NULL;
            if (ADM_ECODE_SUCCESSED != retVal) {
                log_e("adm_ecm_decryptAndVerificationSignature. return error[%d]", retVal);
                break;
            }
			(void)adm_com_rmPrivacyFile(policySavePath);		

			/** 固定名称 policyFile.zip */
			(void)adm_snprintf(policyFilePath, 128, "%s%s", policySavePath, "policyFile.zip");
            log_i("inOldpath:[%s]inNewpath:[%s]", fileObject->filePath, policyFilePath);
			/** 重命名到对应目录下前，前清空对应目录文件内容 */
            /*解密验签后文件路径会发生变化，重命名到eweb下载路径下*/
            if (ADM_ERR_OK != adm_com_renameFile(fileObject->filePath, policyFilePath)) {
                log_e("rename file failed");
                retVal = ADM_ECODE_FAILED;
                break;
            }
        } else {
            log_i("privacy file is not encrypt");
        }

        fileInfoSet = adm_com_newFileInfoSet();
        if (NULL == fileInfoSet) {
            retVal = ADM_ECODE_NO_MEMORY;
            log_e("adm_com_newFileObject. return failed");
            break;
        }
        log_i("get privacy policy origin file size");
        retVal = adm_com_getOriginAndSignFlieLen(privacyMember->fileSecurityInfo, fileInfoSet);
        if (ADM_ECODE_SUCCESSED != retVal) {
            log_e("adm_com_getOriginAndSignFlieLen. return error[%d]", retVal);
            break;
        }
        log_i("get privacy policy origin file hash");
        retVal = adm_com_getOriginAndSignFliehash(privacyMember->fileSecurityInfo, fileInfoSet);
        if (ADM_ECODE_SUCCESSED != retVal) {
            log_e("adm_com_getOriginAndSignFlieLen. return error[%d]", retVal);
            break;
        }
        privacyMember->ucmPrivacyPolicyFileHash = adm_strdup(fileInfoSet->originPkgFileHash);
        privacyMember->ucmPrivacyPolicyFileSize = fileInfoSet->originPkgFileLen;
        log_i("privacy policy origin file hash = %s", privacyMember->ucmPrivacyPolicyFileHash);
        log_i("privacy policy origin file size = %d", privacyMember->ucmPrivacyPolicyFileSize);
        retVal = ADM_ECODE_SUCCESSED;
    } while (FALSE);
    adm_com_delFileObject(&fileObject);
    adm_com_delFileInfoSet(&fileInfoSet);
    return retVal;
}

static D32S cvmUcmHmiFileInfo(const DCHAR* url, const DCHAR* hash, D32S size, DCHAR** fileInfoStr)
{
    if (NULL == url || NULL == hash) {
        return ADM_ECODE_INVALID_PARAM;
    }
    json_object* root = NULL;
    json_object* subObject = NULL;
    D32S retVal = ADM_ECODE_SUCCESSED;

    root = json_object_new_object();
    if (NULL == root) {
        log_e("new object.failed");
        return ADM_ECODE_NO_MEMORY;
    }

    subObject = json_object_new_string(url);
    (void)json_object_object_add(root, ADM_CVS_UCM_HMI_POLICY_FILE_URL_FIELD, subObject);

    subObject = json_object_new_string(hash);
    (void)json_object_object_add(root, ADM_CVS_UCM_HMI_POLICY_FILE_HASH_FIELD, subObject);

    subObject = json_object_new_int(size);
    (void)json_object_object_add(root, ADM_CVS_UCM_HMI_POLICY_FILE_SIZE_FIELD, subObject);

    *fileInfoStr = adm_strdup(json_object_to_json_string(root));
    (void)json_object_put(root);
    return retVal;
}
static D32S cvmUcmPrivacyFileInfo(const DCHAR* url, const DCHAR* hash, D32S size, DCHAR** fileInfoStr)
{
    if (NULL == url || NULL == hash) {
        return ADM_ECODE_INVALID_PARAM;
    }
    json_object* root = NULL;
    json_object* subObject = NULL;
    D32S retVal = ADM_ECODE_SUCCESSED;

    root = json_object_new_object();
    if (NULL == root) {
        log_e("new object.failed");
        return ADM_ECODE_NO_MEMORY;
    }

    subObject = json_object_new_string(url);
    (void)json_object_object_add(root, ADM_CVS_UCM_PRIVACY_POLICY_FILE_URL_FIELD, subObject);

    subObject = json_object_new_string(hash);
    (void)json_object_object_add(root, ADM_CVS_UCM_PRIVACY_POLICY_FILE_HASH_FIELD, subObject);

    subObject = json_object_new_int(size);
    (void)json_object_object_add(root, ADM_CVS_UCM_PRIVACY_POLICY_FILE_SIZE_FIELD, subObject);

    *fileInfoStr = adm_strdup(json_object_to_json_string(root));
    (void)json_object_put(root);
    return retVal;
}

static D32S adm_cvm_implHandlerHmiPolicy(ADM_OCS_CHECK_HMI_MEMBER_T* hmiMember, const DCHAR* policySavePath)
{
    if (NULL == policySavePath || NULL == hmiMember) {
        return ADM_ECODE_INVALID_PARAM;
    }
    D32S retVal = ADM_ECODE_SUCCESSED;
    DCHAR* kvGetHmiPolicyPath = NULL;
    size_t kvGetHmiPolicyPathLen = 0;
    ADM_KV_STORAGE_VALUE_TYPE_E eType;
    DCHAR policyFileDownloadPath[256];
    DCHAR ucmPolicyFileUrl[256];
    // const DCHAR* eweb_prefix_url = NULL;
    const DCHAR* pUcmHmiDownUrl = NULL;

    (void)adm_memset(policyFileDownloadPath, 0x00, 256, 256);
    (void)adm_memset(ucmPolicyFileUrl, 0x00, 256, 256);

    do {
        /*生成hmi策略文件的下载路径*/
        (void)adm_snprintf(policyFileDownloadPath, 128, "%s%s", policySavePath, hmiMember->hmiShowFileName);
        log_i("save policy full path = %s", policyFileDownloadPath);
        log_i("download policy file");
        retVal = adm_cvm_downloadFile(
            hmiMember->hmiShowFileUrl, hmiMember->hmiShowFileHash, hmiMember->hmiShowFileSize, policyFileDownloadPath);
        if (ADM_ECODE_SUCCESSED != retVal) {
            retVal = ADM_ECODE_CHECK_DOWNLOAD_POLICYFILE_FAILED;
            log_e("download policy file failed");
            break;
        }

        /*hmi策略文件加密、验签、拆包*/
        if (NULL != hmiMember->fileSecurityInfo) {
            log_i("hmi file have fileSecurityInfo");
            retVal = adm_cvm_hmiFileSecurityInfo(hmiMember, policyFileDownloadPath);
            if (ADM_ECODE_SUCCESSED != retVal) {
                log_e("adm_cvm_hmiFileSecurityInfo return error[%d]", retVal);
                break;
            }
        } else {
            hmiMember->ucmHmiPolicyFileHash = adm_strdup(hmiMember->hmiShowFileHash);
            hmiMember->ucmHmiPolicyFileSize = hmiMember->hmiShowFileSize;
            log_i("hmi file whitout fileSecurityInfo");
        }

        /*生成wewb的下载链接;ucmPolicyFileUrl =  eweb_url前缀 + 文件名*/
        log_i("generate URLS for HMI files on UCM");
        // eweb_prefix_url = adm_cfg_iniParserGetString(ADM_CFG_EWEB_PREFIX_URL, ADM_CFG_EWEB_PREFIX_URL_DEFAULT_VAL);
        // (void)adm_snprintf(ucmPolicyFileUrl, 256, "%s%s", eweb_prefix_url, hmiMember->hmiShowFileName);
        pUcmHmiDownUrl = adm_cfg_iniParserGetString(ADM_CFG_EWEB_DOC_DOWNLOAD_DIR, ADM_CFG_EWEB_DOC_DOWNLOAD_DIR_DEFAULT_VAL);
        (void)adm_snprintf(ucmPolicyFileUrl, 256, "%s%s", pUcmHmiDownUrl, hmiMember->hmiShowFileName);
        hmiMember->ucmHmiPolicyFileUrl = adm_strdup(ucmPolicyFileUrl);
        log_i("ucmHmiPolicyFileUrl = %s", hmiMember->ucmHmiPolicyFileUrl);

        log_i("get old policy file path from KV and remove it");
        if (TRUE
            == adm_com_kvstorage_get_string(adm_core_service_getBusinessDatabaseDb(),
                (const DCHAR*)ADM_CVS_KV_UCMHMIPOLICYFILPATH_KEY,
                &kvGetHmiPolicyPath,
                &kvGetHmiPolicyPathLen,
                &eType)) {
            log_i("old policy file full path = %s", kvGetHmiPolicyPath);
            if (0 != adm_memcmp(kvGetHmiPolicyPath, policyFileDownloadPath, adm_strlen(kvGetHmiPolicyPath))) {
                log_i("delete old file");
                (void)adm_fremove(kvGetHmiPolicyPath);
            } else {
                log_i("files not updated");
            }
        } else {
            log_w("old policy file not exist");
        }

        eType = ADM_KV_STORAGE_VALUE_TYPE_STRING;
        log_i("set new policy file path to kv");
        if (FALSE
            == adm_com_kvstorage_set_string(adm_core_service_getBusinessDatabaseDb(),
                (const DCHAR*)ADM_CVS_KV_UCMHMIPOLICYFILPATH_KEY,
                policyFileDownloadPath,
                eType)) {
            log_e("kv storage download policy Path failed");
        }
    } while (FALSE);
    adm_free(kvGetHmiPolicyPath);
    return retVal;
}

static DBOOL adm_cvm_policyResultToKv(const DCHAR* policyResultKvKey)
{
    ADM_KV_STORAGE_VALUE_TYPE_E eType = ADM_KV_STORAGE_VALUE_TYPE_STRING;
    DCHAR policyResultStr[64];

    (void)adm_memset(policyResultStr, 0x00, 64, 64);
    (void)adm_snprintf(policyResultStr, 64, "%s", "success");

    if (TRUE
        == adm_com_kvstorage_set_string(
            adm_core_service_getBusinessDatabaseDb(), policyResultKvKey, policyResultStr, eType)) {
        log_i("set policyResult success, policyResultStr = %s ", policyResultStr);
        return TRUE;
    } else {
        log_e("set policyResult failed");
        return FALSE;
    }
}

static D32S adm_cvm_implHandlerPrivacyPolicy(ADM_OCS_CHECK_PRIVACY_MEMBER_T* privacyMember, const DCHAR* policySavePath)
{
    if (NULL == policySavePath || NULL == privacyMember) {
        return ADM_ECODE_INVALID_PARAM;
    }
    D32S retVal = ADM_ECODE_SUCCESSED;
    DCHAR* kvGetPolicyPolicyPath = NULL;
    size_t kvGetPolicyPolicyPathLen = 0;
    ADM_KV_STORAGE_VALUE_TYPE_E eType;
    DCHAR policyFileDownloadPath[256];
    DCHAR ucmPolicyFileUrl[256];
    // const DCHAR* eweb_prefix_url = NULL;
    const DCHAR* pUcmPolicyDownUrl = NULL;

    (void)adm_memset(policyFileDownloadPath, 0x00, 256, 256);
    (void)adm_memset(ucmPolicyFileUrl, 0x00, 256, 256);

    do {
        /*生成hmi策略文件的下载路径*/
        (void)adm_snprintf(policyFileDownloadPath, 128, "%s%s", policySavePath, privacyMember->privacyShowFileName);
        log_i("save policy full path = %s", policyFileDownloadPath);
        retVal = adm_cvm_downloadFile(
            privacyMember->privacyShowFileUrl, privacyMember->privacyShowFileHash, privacyMember->privacyShowFileSize, policyFileDownloadPath);
        if (ADM_ECODE_SUCCESSED != retVal) {
            retVal = ADM_ECODE_CHECK_DOWNLOAD_POLICYFILE_FAILED;
            log_e("download policy file failed");
            break;
        }

        /*hmi策略文件加密、验签、拆包*/
        if (NULL != privacyMember->fileSecurityInfo) {
            log_i("Privacy file have fileSecurityInfo");
            retVal = adm_cvm_privacyFileSecurityInfo(privacyMember, policyFileDownloadPath, policySavePath);
            if (ADM_ECODE_SUCCESSED != retVal) {
                log_e("adm_cvm_hmiFileSecurityInfo return error[%d]", retVal);
                break;
            }
        } else {
            privacyMember->ucmPrivacyPolicyFileHash = adm_strdup(privacyMember->privacyShowFileHash);
            privacyMember->ucmPrivacyPolicyFileSize = privacyMember->privacyShowFileSize;
            log_i("Privacy file whitout fileSecurityInfo");
        }
        /*生成wewb的下载链接;ucmPolicyFileUrl =  eweb_url前缀 + 文件名*/
        log_i("generate URLS for Privacy Policy files on UCM");
        // eweb_prefix_url = adm_cfg_iniParserGetString(ADM_CFG_EWEB_PREFIX_URL, ADM_CFG_EWEB_PREFIX_URL_DEFAULT_VAL);
        // (void)adm_snprintf(ucmPolicyFileUrl, 256, "%s%s", eweb_prefix_url, privacyMember->privacyShowFileName);
        pUcmPolicyDownUrl = adm_cfg_iniParserGetString(ADM_CFG_EWEB_DOC_DOWNLOAD_DIR, ADM_CFG_EWEB_DOC_DOWNLOAD_DIR_DEFAULT_VAL);
        (void)adm_snprintf(ucmPolicyFileUrl, 256, "%s%s", pUcmPolicyDownUrl, privacyMember->privacyShowFileName);
        privacyMember->ucmPrivacyPolicyFileUrl = adm_strdup(ucmPolicyFileUrl);
        log_i("ucmPrivacyPolicyFileUrl = %s", privacyMember->ucmPrivacyPolicyFileUrl);

        log_i("get old PRIVACY policy file path from KV and remove it");
        if (TRUE
            == adm_com_kvstorage_get_string(adm_core_service_getBusinessDatabaseDb(),
                (const DCHAR*)ADM_CVS_KV_UCMPRIVACYPOLICYFILPATH_KEY,
                &kvGetPolicyPolicyPath,
                &kvGetPolicyPolicyPathLen,
                &eType)) {
            log_i("old PRIVACY policy file full path = %s", kvGetPolicyPolicyPath);
            if (0 != adm_memcmp(kvGetPolicyPolicyPath, policyFileDownloadPath, adm_strlen(kvGetPolicyPolicyPath))) {
                log_i("delete old file");
                (void)adm_fremove(kvGetPolicyPolicyPath);
            } else {
                log_i("files not updated");
            }
        } else {
            log_w("old PRIVACY policy file not exist");
        }

        eType = ADM_KV_STORAGE_VALUE_TYPE_STRING;
        log_i("set new PRIVACY policy file path to kv");
        if (FALSE
            == adm_com_kvstorage_set_string(adm_core_service_getBusinessDatabaseDb(),
                (const DCHAR*)ADM_CVS_KV_UCMPRIVACYPOLICYFILPATH_KEY,
                policyFileDownloadPath,
                eType)) {
            log_e("kv storage download PRIVACY policy Path failed");
        }
    } while (FALSE);
    adm_free(kvGetPolicyPolicyPath);
	if(retVal == ADM_ECODE_SUCCESSED)
	{
		adm_cvm_policyResultToKv(ADM_CVS_KV_PRIVACYPOLICY);
	}
    return retVal;
}

static D32S adm_cvm_handleRespPolicy(ADM_OCS_CHECK_RESP_ADAPTER_T* respPolicy)
{
    D32S retVal = ADM_ECODE_SUCCESSED;
    const DCHAR* saveHmiPolicyDir = NULL;
    DCHAR* savePrivacyPolicyDir = NULL;
    DBOOL hmiPolicyFileIsDownload = TRUE;
    ADM_KV_STORAGE_VALUE_TYPE_E eType = ADM_KV_STORAGE_VALUE_TYPE_STRING;
    DCHAR* fileInfoStr = NULL;
    DCHAR* privacyFileInfoStr = NULL;
	D32S s32hostInstId = 0;
    D32S errorCode = 0;
	//host实例id
	
	s32hostInstId = adm_cfg_iniParserGetInt(ADM_CFG_VUS_HOST_INSTANCEID,  ADM_CFG_VUS_HOST_INSTANCEID_DEFAULT_VAL);

    hmiPolicyFileIsDownload = adm_cfg_iniParserGetBoolean(
        ADM_CFG_VUS_HMIPOLICYFILE_IS_DOWNLOAD, (DBOOL)ADM_CFG_VUS_HMIPOLICYFILE_IS_DOWNLOAD_DEFAULT_VAL);
    if (FALSE == hmiPolicyFileIsDownload) {
        /*HMI策略文件不需要下载，给HMI的文件url、hash、size和云端下发给主控UCM的一致 */
        log_i("hmi policy file not download");
        respPolicy->hmiMember->ucmHmiPolicyFileUrl = adm_strdup(respPolicy->hmiMember->hmiShowFileUrl);
        respPolicy->hmiMember->ucmHmiPolicyFileHash = adm_strdup(respPolicy->hmiMember->hmiShowFileHash);
        respPolicy->hmiMember->ucmHmiPolicyFileSize = respPolicy->hmiMember->hmiShowFileSize;
        return ADM_ECODE_SUCCESSED;
    }

    do {
        saveHmiPolicyDir
            = adm_cfg_iniParserGetString(ADM_CFG_EWEB_DOC_DOWNLOAD_DIR, ADM_CFG_EWEB_DOC_DOWNLOAD_DIR_DEFAULT_VAL);
        log_i("eweb download dir = %s", saveHmiPolicyDir);

        log_i("download hmi policy file");
        retVal = adm_cvm_implHandlerHmiPolicy(respPolicy->hmiMember, saveHmiPolicyDir);
        if (ADM_ECODE_SUCCESSED == retVal) {
            log_i(" hmi file info to str");
            retVal = cvmUcmHmiFileInfo(respPolicy->hmiMember->ucmHmiPolicyFileUrl,
                respPolicy->hmiMember->ucmHmiPolicyFileHash,
                (D32S)respPolicy->hmiMember->ucmHmiPolicyFileSize,
                &fileInfoStr);
            if (ADM_ECODE_SUCCESSED != retVal) {
                log_e("cvmUcmHmiFileInfo return failed");
                break;
            }
            log_i("set hmi file info on UCM to KV. fileInfoStr = %s", fileInfoStr);
            if (FALSE
                == adm_com_kvstorage_set_string(adm_core_service_getBusinessDatabaseDb(),
                    (const DCHAR*)ADM_CVS_KV_UCMHMIPOLICYFILEINFO_KEY,
                    fileInfoStr,
                    eType)) {
                retVal = ADM_ECODE_KV_SET_STRING_FAILED;
                log_e("kv storage respMessage failed");
                break;
            }
        } else {
            log_e("adm_cvm_implHandlerHmiPolicy. failed");
            retVal = ADM_ECODE_CHECK_DOWNLOAD_POLICYFILE_FAILED;
            break;
        }
#if 1
		/** savePrivacyPolicyDir = 从车机接口获取 */
		retVal = adm_dim_host_getPrivacyPath((D16U)s32hostInstId, 30, &savePrivacyPolicyDir, &errorCode);
	    if (ADM_IS_ERR(retVal) || ADM_IS_ERR(errorCode))
	    {
	        log_e("Failed dm_dim_host_getPrivacyPath,err[%d],errcode[%d]", retVal, errorCode);
			retVal = ADM_ECODE_SUCCESSED;//不能干扰正常检测
	        //break;
			savePrivacyPolicyDir = gPrivacyPath;
	    }
		if(NULL == savePrivacyPolicyDir)
		{
			savePrivacyPolicyDir = gPrivacyPath;
		}
		log_i("download PrivacyPolicyDir:%s", savePrivacyPolicyDir);

        log_i("download Privac policy file");
        retVal = adm_cvm_implHandlerPrivacyPolicy(respPolicy->privacyMember, savePrivacyPolicyDir);
        if (ADM_ECODE_SUCCESSED == retVal) 
		{
            log_i(" privacy file info to str");
            retVal = cvmUcmHmiFileInfo(respPolicy->privacyMember->ucmPrivacyPolicyFileUrl,
                respPolicy->privacyMember->ucmPrivacyPolicyFileHash,
                (D32S)respPolicy->privacyMember->ucmPrivacyPolicyFileSize,
                &privacyFileInfoStr);
            if (ADM_ECODE_SUCCESSED != retVal) {
                log_e("cvmUcmHmiFileInfo return failed");
				retVal = ADM_ECODE_SUCCESSED;//不能干扰正常检测
                break;
            }
            log_i("set Privacy file info on UCM to KV. privacyFileInfoStr = %s", privacyFileInfoStr);
            if (FALSE
                == adm_com_kvstorage_set_string(adm_core_service_getBusinessDatabaseDb(),
                    (const DCHAR*)ADM_CVS_KV_UCMPRIVACYPOLICYFILEINFO_KEY,
                    privacyFileInfoStr,
                    eType)) {
				retVal = ADM_ECODE_SUCCESSED;//不能干扰正常检测
                log_e("kv storage respMessage failed");
                break;
            }
        } else {
            log_e("adm_cvm_implHandlerPrivacPolicy. failed");
            retVal = ADM_ECODE_SUCCESSED;
            break;
        }
#endif		
        retVal = ADM_ECODE_SUCCESSED;
    } while (FALSE);
    adm_free(fileInfoStr);
	adm_free(privacyFileInfoStr);
    return retVal;
}

D32S adm_cvm_checkVersion(const ADM_APPLICATION_COMMON_CMDID_TYPE_E cmdId,
    ADM_CVS_NOTIFY_T* notifyF,
    DCHAR* ext,
    ADM_OCS_CHECK_RESP_ADAPTER_T** cvmResp)
{
    if (cmdId >= ADM_APPLICATION_COMMON_CMDID_TYPE_INVALID || NULL == notifyF || NULL == notifyF->stateReportCloud
        || NULL == notifyF->progressNotifyHmi || NULL == notifyF->eventReportCloud
        || NULL == notifyF->getEventSequenceId) {
        log_e("param notifyF is NULL");
        return ADM_ECODE_INVALID_PARAM;
    }
    adm_cvm_triggerEventReport(cmdId, notifyF);

    ADM_OCS_CHECK_REQ_ADAPTER_T* checkReq = NULL;
    D64S timestamp;
    DCHAR* netType = NULL;
    DCHAR* versionNumber = NULL;
    DCHAR* sdkVersion = NULL;
    DCHAR* allEcuInfoStr = NULL;
    DCHAR* allModuleVerInfoStr = NULL;
    DCHAR* pVehicleVersion = NULL;
    D32S retVal = ADM_ECODE_SUCCESSED;
    ADM_OCS_CHECK_RESP_ADAPTER_T* respPolicy = NULL;
    DCHAR* confVersion = NULL;
    DCHAR reason[32] = {0};
    D64S ulTaskId = 0; 

    respPolicy = adm_ocs_newCheckRespAdapterBody();
    if (NULL == respPolicy) {
        log_e("adm_ocs_newCheckRespAdapterBody failed");
        return ADM_ECODE_APPLY_MEMORY_FAILED;
    }

    log_i("[ABUP-Procedure] check Start.");

    /*notify check start*/
    log_i("---------------check start---------------");
    (void)sprintf(reason, "%d", ADM_ECODE_SUCCESSED);
    notifyF->progressNotifyHmi(cmdId, "CHECK_START", ADM_APPLICATION_COMMON_CHECK_PROGRESS_TYPE_START);
    // notifyF->stateReportCloud(ADM_IRM_REPORT_SERVER_STATUS_Task_Check_Ing, reason);
    
    log_i("[ABUP-Procedure] check Trigger type: %d.", cmdId);

    if (ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_TSP == cmdId) 
    {
        ulTaskId = adm_cvm_getTspTaskId();
        log_i("[ABUP-Procedure] check Trigger taskId: %llu.", ulTaskId);
    }


    do {
        log_i("get confVersion from ecs");
        confVersion = adm_ecs_getRsConfigVersion();
        if (NULL == confVersion) {
            retVal = ADM_ECODE_APPLY_MEMORY_FAILED;
            log_e("get confVersion ,error[%d]", retVal);
            break;
        }

        log_i("get all ecu info");
        notifyF->progressNotifyHmi(cmdId, "GET_ECUINFO", ADM_APPLICATION_COMMON_CHECK_PROGRESS_TYPE_GET_ECUINFO);

        if (ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_USER == cmdId)
        {
            /** 用户触发的检测，优先从缓存读取，缓存读取失败再实时读取 */
            retVal = adm_eim_getEcuInfoManagerKvGetStr(&allEcuInfoStr);
            if (ADM_ECODE_SUCCESSED != retVal)
            {
                retVal = adm_eim_getEcuInfoManagerDimGetStr(&allEcuInfoStr, COLLECT_TRIGGER_TYPE_CHECK);
                if (ADM_ECODE_SUCCESSED != retVal)
                {
                    retVal = ADM_ECODE_EIM_GET_ECU_INFO_FAILED;
                    adm_cvm_prepareEventReport(retVal, notifyF);
                    log_e("get ecu info str. error[%d]", retVal);
                    break;
                }
            }
        }
        else
        {
            /** 非用户触发（TSP推送版本检测、上电版本检测等），需要实时采集 */
            retVal = adm_eim_getEcuInfoManagerDimGetStr(&allEcuInfoStr, COLLECT_TRIGGER_TYPE_CHECK);
            if (ADM_ECODE_SUCCESSED != retVal) 
            {
                retVal = ADM_ECODE_EIM_GET_ECU_INFO_FAILED;
                adm_cvm_prepareEventReport(retVal, notifyF);
                log_e("get ecu info str. error[%d]", retVal);
                break;
            }
        }

        log_i("allEcuInfoStr = %s", allEcuInfoStr);

        adm_cvm_prepareEventReport(retVal, notifyF);
        log_i("get ecu sdkVersion versionNumber netType timestamp");
        notifyF->progressNotifyHmi(cmdId, "GET_NET_INFO", ADM_APPLICATION_COMMON_CHECK_PROGRESS_TYPE_GET_NET_INFO);
        retVal = adm_cvm_getReqFieldValue(&timestamp, &netType, &versionNumber, &sdkVersion, &allModuleVerInfoStr);
        if (ADM_ECODE_SUCCESSED != retVal) {
            log_e("get Req field value. failed");
            break;
        }

        //adm_cvs_getHmiVehicleVersion(&pVehicleVersion);
        adm_eim_getVehicleVersion(&pVehicleVersion);
        if (NULL == pVehicleVersion)
        {
#ifdef AIR_PROTOCOL_VEHICLE_VERSION
            pVehicleVersion = adm_strdup(AIR_PROTOCOL_VEHICLE_VERSION);
#endif            
        }

        log_i("new check req body");
        checkReq = adm_ocs_newCheckReqAdapterBody((D32S)cmdId,
            timestamp,
            netType,
            versionNumber,
            sdkVersion,
            confVersion,
            allEcuInfoStr,
            allModuleVerInfoStr,
            pVehicleVersion,
            ulTaskId,
            ext,
            adm_ocm_businessCommInterface);
        if (NULL == checkReq) {
            retVal = ADM_ECODE_APPLY_MEMORY_FAILED;
            log_e("call adm_cvs_newReqBody.failed");
            break;
        }

        log_i("call adm_cvs_checkVersion");
        retVal = adm_cvs_checkVersion(checkReq, notifyF, respPolicy);
        (void)memset(reason, 0, sizeof(reason));
        if (ADM_ECODE_SUCCESSED == retVal) {
            log_i("the cloud has new version information");
            retVal = adm_cvm_handleRespPolicy(respPolicy);
            if (ADM_ECODE_SUCCESSED == retVal) {
                *cvmResp = respPolicy;
                (void)sprintf(reason, "%d", ADM_ECODE_SUCCESSED);
                // notifyF->stateReportCloud(ADM_IRM_REPORT_SERVER_STATUS_Task_Check_Successed, (const DCHAR*)reason);
            } else {
                (void)sprintf(reason, "%d", ADM_ECODE_FAILED);
                // notifyF->stateReportCloud(ADM_IRM_REPORT_SERVER_STATUS_Task_Check_Failed, (const DCHAR*)reason);
            }
        } else if (ADM_ECODE_CHECK_LATEST_VERSION == retVal) {
            *cvmResp = respPolicy;
            log_i("the current version is the latest");
            (void)sprintf(reason, "%d", ADM_ECODE_SUCCESSED);
            // notifyF->stateReportCloud(ADM_IRM_REPORT_SERVER_STATUS_Task_Check_Successed, (const DCHAR*)reason);
        } else if (ADM_ECODE_NO_NEW_VERSION == retVal) {
            log_i("the cloud has no new version information");
            (void)sprintf(reason, "%d", ADM_ECODE_NO_NEW_VERSION);
            // notifyF->stateReportCloud(ADM_IRM_REPORT_SERVER_STATUS_Task_Check_Successed, (const DCHAR*)reason);
        } else {
            log_e("check return error[%d]", retVal);
            (void)sprintf(reason, "%d", ADM_ECODE_FAILED);
            // notifyF->stateReportCloud(ADM_IRM_REPORT_SERVER_STATUS_Task_Check_Failed, (const DCHAR*)reason);
        }
        adm_cvm_endEventReport(retVal, notifyF);
        log_i("exit adm_cvs_checkVersion");
    } while (FALSE);

    adm_free(netType);
    adm_free(versionNumber);
    adm_free(sdkVersion);
    adm_free(allEcuInfoStr);
    adm_free(allModuleVerInfoStr);
    adm_free(confVersion);
    adm_free(pVehicleVersion);
    adm_ocs_deleteCheckReqAdapterBody(checkReq);
    /*notify check end*/
    log_i("---------------check end---------------");
    notifyF->progressNotifyHmi(cmdId, "CHECK_END", ADM_APPLICATION_COMMON_CHECK_PROGRESS_TYPE_END);

    sleep(2);

    log_i("[ABUP-Procedure] check End. [iRet = %d]", retVal);

    return retVal;
}

static void adm_cvm_checkProgressNotifyHmi(const ADM_CVS_COMMON_CMDID_TYPE_E eCmdidTYpe,
    const DCHAR* state,
    const ADM_CVS_COMMON_CHECK_PROGRESS_TYPE_T eCheckProgressType)
{
    /*empty functions do not notify HMI*/
    return;
}

static void adm_cvm_checkStateReport(const DCHAR* state, const DCHAR* reason)
{
    if (NULL == state || NULL == reason) {
        log_e("state or reason is NULL");
        return;
    }
    D32S retVal = ADM_ECODE_FAILED;
    D64S timestamp = time(NULL);
    DCHAR* sdkVersion = NULL;
    DCHAR* sessionId = NULL;
    D64S taskId = 0;
    D64S strategyId = 0;
    DCHAR* versionNumber = NULL;

#ifdef ADM_SDK_VERSION
    sdkVersion = adm_strdup(ADM_SDK_VERSION);
#else
    sdkVersion = adm_strdup("v5.0");
#endif

    retVal = adm_eim_getVehicleBigVersion(&versionNumber);
    if (ADM_ECODE_SUCCESSED != retVal) {
        log_e("get Vehicle Big Version,error[%d]", retVal);
    }
    retVal = adm_irm_reportVehicleState(
        timestamp, sdkVersion, sessionId, taskId, strategyId, state, reason, versionNumber, NULL, NULL);
    if (ADM_ECODE_SUCCESSED != retVal) {
        log_e("adm_irm_reportVehicleState, return error[%d]", retVal);
    }
    adm_free(sdkVersion);
    adm_free(versionNumber);
    return;
}

static void adm_cvm_checkReportEvent(ADM_OCS_EVENTS_INFO_LIST_T* eventsList)
{
    if (NULL == eventsList || 0 == eventsList->reportEventsLen) {
        log_e("eventsList is NULL");
        return;
    }

    D32S retVal = ADM_ECODE_FAILED;
    DCHAR* sessionId = NULL;
    D64S taskId = 0;
    D64S strategyId = 0;
    D64S timestamp = time(NULL);
    DCHAR* sdkVersion = NULL;
    DCHAR* extsReqJsonStr = NULL;
    D32S i = 0;

#ifdef ADM_SDK_VERSION
    sdkVersion = adm_strdup(ADM_SDK_VERSION);
#else
    sdkVersion = adm_strdup("v5.0");
#endif

    for (i = 0; i < eventsList->reportEventsLen; i++) 
    {
        /** 检测成功，需要填充任务信息 */
        if (0 == adm_strcmp(eventsList->reportEvents[i].eventId, "S_CE_001"))
        {
            adm_cvs_getSessionId(&sessionId);
            adm_cvs_getTaskId(&taskId);
            adm_cvs_getStrategyId(&strategyId);
        }
    }

    retVal = adm_irm_reportEvent(sessionId, taskId, strategyId, timestamp, sdkVersion, extsReqJsonStr, eventsList);
    if (ADM_ECODE_SUCCESSED != retVal)
    {
        log_e("adm_irm_reportEvent, return error[%d]", retVal);
    }

    adm_free(sdkVersion);
    adm_free(sessionId);

    return;
}

static const D16U eventNum = 50;

D32S adm_cvm_checkVersionAdaptorNotify(const ADM_APPLICATION_COMMON_CMDID_TYPE_E cmdId,
    checkProgressNotifyHmi_F progressNotifyHmi,
    DCHAR* ext,
    ADM_OCS_CHECK_RESP_ADAPTER_T** cvmResp)
{
    if (cmdId >= ADM_APPLICATION_COMMON_CMDID_TYPE_INVALID) {
        log_e("param notifyF is NULL");
        return ADM_ECODE_INVALID_PARAM;
    }
    D32S retVal = ADM_ECODE_FAILED;
    ADM_CVS_NOTIFY_T* notifyF = NULL;
    ADM_OCS_CHECK_RESP_ADAPTER_T* resp = NULL;

    D32S configReportEventLevel = ADM_CFG_VUS_EVENT_REPORT_LEVEL_DEFAULT_VAL;
	if (ADM_ECODE_SUCCESSED != adm_ecs_getEventReportLevel(&configReportEventLevel))
	{
        configReportEventLevel = adm_cfg_iniParserGetInt(ADM_CFG_VUS_EVENT_REPORT_LEVEL, ADM_CFG_VUS_EVENT_REPORT_LEVEL_DEFAULT_VAL);
	}
        
    log_i("configReportEventLevel = %d", configReportEventLevel);
    if (NULL != progressNotifyHmi) {
        notifyF = adm_cvs_initNotifyCb(progressNotifyHmi,
            adm_cvm_checkStateReport,
            adm_cvm_checkReportEvent,
            adm_irm_getEventSequenceId,
            eventNum,
            configReportEventLevel);
    } else {
        notifyF = adm_cvs_initNotifyCb(adm_cvm_checkProgressNotifyHmi,
            adm_cvm_checkStateReport,
            adm_cvm_checkReportEvent,
            adm_irm_getEventSequenceId,
            eventNum,
            configReportEventLevel);
    }

    retVal = adm_cvm_checkVersion(cmdId, notifyF, NULL, cvmResp);
    log_i("retVal = %d", retVal);
    if (ADM_ECODE_SUCCESSED == retVal || ADM_ECODE_NO_NEW_VERSION == retVal
        || ADM_ECODE_CHECK_LATEST_VERSION == retVal) {
        log_i("check success");
    } else {
        log_e("check error[%d]", retVal);
    }
    adm_cvs_deInitNotifyCb(notifyF);
    return retVal;
}

#define BUFFER_SIZE 1024
/**
 * @brief 从usb拷贝hmi文件到本地传输路径下
 * 
 * @param[in] pSrcPath     描述: 源hmi文件(绝对路径) \n
 * 范围值: 指针 \n
 * 单位: N/A \n
 * 
 * @param[in] pDstPath     描述: 目的hmi文件(绝对路径) \n
 * 范围值: 指针 \n
 * 单位: N/A \n
 * 
 * @return D32S 
 */
static D32S cvmCopyUsbHmiFile(const DCHAR* pSrcPath, const DCHAR* pDstPath)
{
    struct stat fileStats = {0};
    D32S iRet = ADM_ECODE_SUCCESSED;
    DCHAR aFileData[BUFFER_SIZE +1] = {0}; ///< 文件按照1K分段拷贝
    D64U uiTotalSize = 0;
    DSIZE uiBytesRead = 0;
    DSIZE uiBytesWritten = 0;
    FILE* pFileIn = NULL;
    FILE* pFileOut = NULL;

    if ((NULL == pSrcPath) || (NULL == pDstPath))
    {
        log_e( "Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }

    if (0 != access(pSrcPath, F_OK))
    {
        log_e( "src file [%s] not exist.", pSrcPath);
        return ADM_ECODE_FILE_NOT_EXIST;
    }

    iRet = stat(pSrcPath, &fileStats);
    if (iRet != 0) 
    {
        log_e("state file [%s] failed.", pSrcPath);
        return ADM_ECODE_OPEN_FILE_FAILED;
    }
    uiTotalSize = (D64U)fileStats.st_size;
    log_i("file:[%s] total size:[%llu]", pSrcPath, uiTotalSize);

    pFileIn = fopen(pSrcPath, "rb");
    if (NULL == pFileIn)
    {
        log_e( "open file %s, error %s", pSrcPath, strerror(errno));
        return ADM_ECODE_OPEN_FILE_FAILED;
    }

    pFileOut = fopen(pDstPath, "wb+");
    if (NULL == pFileOut) 
    {
        fclose(pFileIn);
        log_e( "open file %s, error %s", pDstPath, strerror(errno));
        return ADM_ECODE_OPEN_FILE_FAILED;
    }

    (void)adm_memset(aFileData, '\0', sizeof(aFileData), sizeof(aFileData));

    while ((uiBytesRead = fread(aFileData, 1, BUFFER_SIZE, pFileIn)) > 0)
    {
        uiBytesWritten = fwrite(aFileData, 1, uiBytesRead, pFileOut);
        if (uiBytesWritten < uiBytesRead)
        {
            log_e("write byte lower than read byte");
            iRet = ADM_ECODE_WRITE_FILE_FAILED;
            break;
        }
    }

    fclose(pFileIn);
    fclose(pFileOut);

    return iRet;
}

#define USB_CHECK_POLICY_FILE_PATH "/tmp/share/policyResp.txt" ///< 根据项目变更
static D32S cvmUsbHandleRespPolicy(ADM_OCS_CHECK_RESP_ADAPTER_T* respPolicy)
{
    D32S retVal = ADM_ECODE_SUCCESSED;
    D32U uiSrcLen = 0;
    D32U uiDstLen = 0;
    const DCHAR* saveHmiPolicyDir = NULL;
    // const DCHAR* eweb_prefix_url = NULL;
    const DCHAR* pUcmHmiDownUrl = NULL;
    DCHAR* srcHmiPolicyPath = NULL;
    DCHAR* dstHmiPolicyPath = NULL;
    DCHAR* fileData = NULL;
    DCHAR* kvGetHmiPolicyPath = NULL;
    size_t kvGetHmiPolicyPathLen = 0;
    DBOOL hmiPolicyFileIsDownload = TRUE;
    DCHAR ucmPolicyFileUrl[256];
    ADM_KV_STORAGE_VALUE_TYPE_E eType = ADM_KV_STORAGE_VALUE_TYPE_STRING;
    DCHAR* fileInfoStr = NULL;
    DCHAR *pcSrcDir = NULL;

    do {
        /* 拷贝U盘hmi文件到本地,并拼接eweb路径 */
        saveHmiPolicyDir = (DCHAR *)adm_cfg_iniParserGetString(ADM_CFG_EWEB_DOC_DOWNLOAD_DIR, ADM_CFG_EWEB_DOC_DOWNLOAD_DIR_DEFAULT_VAL);
        log_i("copy hmi file dir = %s", saveHmiPolicyDir);

        /** 从配置中获取U盘对应的下载路径 */
        pcSrcDir = adm_cvs_getUsbDownloadDir();
        if (ADM_IS_NULL(pcSrcDir))
        {
            log_e("adm_cvs_getUsbDownloadDir failed");
            break;
        }

        uiSrcLen = adm_strlen(pcSrcDir) + adm_strlen(respPolicy->hmiMember->hmiShowFileName) + 1U;
        uiDstLen = adm_strlen(saveHmiPolicyDir) + adm_strlen(respPolicy->hmiMember->hmiShowFileName) + 1U;

        srcHmiPolicyPath = (DCHAR*)adm_malloc(uiSrcLen);
        dstHmiPolicyPath = (DCHAR*)adm_malloc(uiDstLen);
        (DVOID)adm_snprintf(srcHmiPolicyPath, uiSrcLen, "%s%s", pcSrcDir, respPolicy->hmiMember->hmiShowFileName);
        (DVOID)adm_snprintf(dstHmiPolicyPath, uiDstLen, "%s%s", saveHmiPolicyDir, 
                    respPolicy->hmiMember->hmiShowFileName);

        retVal = adm_com_createFileDirAndFile(saveHmiPolicyDir, DFALSE);
        if (ADM_IS_ERR(retVal))
        {
            log_e("Failed to check/create dir, path[%s], err[%d]", saveHmiPolicyDir, retVal);
            break;
        }

        ///< 拷贝U盘文件到本地
        retVal = cvmCopyUsbHmiFile(srcHmiPolicyPath, dstHmiPolicyPath);
        if (ADM_IS_ERR(retVal))
        {
            log_e("copy usb hmi file failed[%d].", retVal);
            break;
        }

        ///< 文件解密、验签、拆分原文件+签名文件
        if (NULL != respPolicy->hmiMember->fileSecurityInfo)
        {
            retVal = adm_cvm_hmiFileSecurityInfo(respPolicy->hmiMember, dstHmiPolicyPath);
            if (ADM_ECODE_SUCCESSED != retVal) 
            {
                log_e("usb check adm_cvm_hmiFileSecurityInfo return error[%d]", retVal);
                break;
            }
        }
        else 
        {
            respPolicy->hmiMember->ucmHmiPolicyFileHash = adm_strdup(respPolicy->hmiMember->hmiShowFileHash);
            respPolicy->hmiMember->ucmHmiPolicyFileSize = respPolicy->hmiMember->hmiShowFileSize;
        }

        ///< 移除老的hmi策略文件,设置本地存储的hmi路径到数据库
        if (TRUE == adm_com_kvstorage_get_string(
            adm_core_service_getBusinessDatabaseDb(),
            (const DCHAR*)ADM_CVS_KV_UCMHMIPOLICYFILPATH_KEY,
            &kvGetHmiPolicyPath,
            &kvGetHmiPolicyPathLen,
            &eType)
            ) 
        {
            log_i("usb check old policy file full path = %s", kvGetHmiPolicyPath);
            if (0 != adm_memcmp(kvGetHmiPolicyPath, dstHmiPolicyPath, adm_strlen(kvGetHmiPolicyPath))) 
            {
                log_i("usb check delete old file");
                (void)adm_fremove(kvGetHmiPolicyPath);
            } 
            else 
            {
                log_i("usb check files not updated");
            }
        } 
        else 
        {
            log_w("usb check old policy file not exist");
        }

        eType = ADM_KV_STORAGE_VALUE_TYPE_STRING;
        log_i("usb check set new policy file path to kv");
        if (FALSE
            == adm_com_kvstorage_set_string(adm_core_service_getBusinessDatabaseDb(),
                (const DCHAR*)ADM_CVS_KV_UCMHMIPOLICYFILPATH_KEY,
                dstHmiPolicyPath,
                eType)) 
        {
            log_e("kv storage download policy Path failed");
        }

        ///< 文件路径拼接eweb头,并存储policyFileInfo
        // eweb_prefix_url = adm_cfg_iniParserGetString(ADM_CFG_EWEB_PREFIX_URL, ADM_CFG_EWEB_PREFIX_URL_DEFAULT_VAL);
        // (DVOID)adm_snprintf(ucmPolicyFileUrl, sizeof(ucmPolicyFileUrl), "%s%s", eweb_prefix_url,respPolicy->hmiMember->hmiShowFileName);

        pUcmHmiDownUrl = adm_cfg_iniParserGetString(ADM_CFG_EWEB_DOC_DOWNLOAD_DIR, ADM_CFG_EWEB_DOC_DOWNLOAD_DIR_DEFAULT_VAL);
        (DVOID)adm_snprintf(ucmPolicyFileUrl, sizeof(ucmPolicyFileUrl), "%s%s", pUcmHmiDownUrl,respPolicy->hmiMember->hmiShowFileName);
        respPolicy->hmiMember->ucmHmiPolicyFileUrl = adm_strdup(ucmPolicyFileUrl);
        log_i("usb check hmi file info to str");
        retVal = cvmUcmHmiFileInfo(respPolicy->hmiMember->ucmHmiPolicyFileUrl,
            respPolicy->hmiMember->ucmHmiPolicyFileHash,
            (D32S)respPolicy->hmiMember->ucmHmiPolicyFileSize,
            &fileInfoStr);
        if (ADM_ECODE_SUCCESSED != retVal) 
        {
            log_e("cvmUcmHmiFileInfo return failed");
            break;
        }
        log_i("set hmi file info on UCM to KV. fileInfoStr = %s", fileInfoStr);
        if (FALSE
            == adm_com_kvstorage_set_string(adm_core_service_getBusinessDatabaseDb(),
                (const DCHAR*)ADM_CVS_KV_UCMHMIPOLICYFILEINFO_KEY,
                fileInfoStr,
                eType)) 
        {
            retVal = ADM_ECODE_KV_SET_STRING_FAILED;
            log_e("kv storage respMessage failed");
            break;
        }
    } while (FALSE);

    adm_free(fileInfoStr);
    adm_free(srcHmiPolicyPath);
    adm_free(fileData);
    adm_free(kvGetHmiPolicyPath);
    adm_free(pcSrcDir);

    return retVal;
}

static D32S cvmUsbCheckVersionHandle(const ADM_APPLICATION_COMMON_CMDID_TYPE_E cmdId,
                                     ADM_CVS_NOTIFY_T *notifyF,
                                     DCHAR *ext,
                                     ADM_OCS_CHECK_RESP_ADAPTER_T **cvmResp)
{
    D32S retVal = ADM_ECODE_SUCCESSED;
    ADM_OCS_CHECK_RESP_ADAPTER_T *respPolicy = NULL;
    DCHAR reason[32] = {0};
    DCHAR *pcPolicyFile = NULL;
    DCHAR *pcUsbDir = DNULL;

    if (cmdId >= ADM_APPLICATION_COMMON_CMDID_TYPE_INVALID || 
        NULL == notifyF || 
        NULL == notifyF->stateReportCloud || 
        NULL == notifyF->progressNotifyHmi || 
        NULL == notifyF->eventReportCloud || 
        NULL == notifyF->getEventSequenceId)
    {
        log_e("param notifyF is NULL");
        return ADM_ECODE_INVALID_PARAM;
    }

    adm_cvm_triggerEventReport(cmdId, notifyF);

    respPolicy = adm_ocs_newCheckRespAdapterBody();
    if (NULL == respPolicy)
    {
        log_e("adm_ocs_newCheckRespAdapterBody failed");
        return ADM_ECODE_APPLY_MEMORY_FAILED;
    }

    /*notify check start*/
    log_i("---------------check start---------------");
    (void)sprintf(reason, "%d", ADM_ECODE_SUCCESSED);
    notifyF->progressNotifyHmi(cmdId, "CHECK_START", ADM_APPLICATION_COMMON_CHECK_PROGRESS_TYPE_START);
    notifyF->stateReportCloud(ADM_IRM_REPORT_SERVER_STATUS_Task_Check_Ing, reason);

    do
    {
        log_i("call adm_cvs_usbCheckVersion");
        
        pcUsbDir = adm_cvs_getUsbDir();
        if (ADM_IS_NULL(pcUsbDir))
        {
            retVal = ADM_ECODE_KV_GET_STRING_FAILED;
            log_e("adm_cvs_getUsbdDir failed, [err:%d]", retVal);
            break;
        }

        /** 判断U盘路径是否存在 */
        if (FALSE == adm_com_isDirExist(pcUsbDir))
        {
            retVal = ADM_ECODE_USB_DIR_IS_NOT_EXIST;
            break;
        }

        /** 解密、解压升级任务包 */
        retVal = usbTaskPackageHandle();
        if (ADM_ECODE_SUCCESSED != retVal)
        {
            log_e("usb task decrypt or unzip failed.[%d]", retVal);
            break;
        }

        /** 查询升级策略文件路径 */
        retVal = cvmGetUsbPolicyFilePath(&pcPolicyFile);
        if (ADM_ECODE_SUCCESSED != retVal)
        {
            log_e("usb get policy file path failed.[%d]", retVal);
            break;
        }

        retVal = adm_cvs_usbCheckVersion(pcPolicyFile, notifyF, respPolicy);
        (void)memset(reason, 0, sizeof(reason));
        if (ADM_ECODE_SUCCESSED == retVal)
        {
            log_i("usb get new version information");
            retVal = cvmUsbHandleRespPolicy(respPolicy);
            if (ADM_ECODE_SUCCESSED == retVal)
            {
                *cvmResp = respPolicy;
                (void)sprintf(reason, "%d", ADM_ECODE_SUCCESSED);
                notifyF->stateReportCloud(ADM_IRM_REPORT_SERVER_STATUS_Task_Check_Successed, (const DCHAR *)reason);
            }
            else
            {
                (void)sprintf(reason, "%d", ADM_ECODE_FAILED);
                notifyF->stateReportCloud(ADM_IRM_REPORT_SERVER_STATUS_Task_Check_Failed, (const DCHAR *)reason);
            }
        }
        else
        {
            log_e("usb check return error[%d]", retVal);
            (void)sprintf(reason, "%d", ADM_ECODE_FAILED);
            notifyF->stateReportCloud(ADM_IRM_REPORT_SERVER_STATUS_Task_Check_Failed, (const DCHAR *)reason);
        }
        adm_cvm_endEventReport(retVal, notifyF);
    } while (FALSE);

    /*notify check end*/
    log_i("---------------check end---------------");
    notifyF->progressNotifyHmi(cmdId, "CHECK_END", ADM_APPLICATION_COMMON_CHECK_PROGRESS_TYPE_END);

    sleep(2);

    adm_free(pcUsbDir);
    adm_free(pcPolicyFile);

    return retVal;
}

D32S adm_cvm_usbCheckVersion(const ADM_APPLICATION_COMMON_CMDID_TYPE_E cmdId,
    checkProgressNotifyHmi_F progressNotifyHmi,
    DCHAR* ext,
    ADM_OCS_CHECK_RESP_ADAPTER_T** cvmResp)
{
    D32S retVal = ADM_ECODE_FAILED;
    ADM_CVS_NOTIFY_T* notifyF = NULL;
    ADM_OCS_CHECK_RESP_ADAPTER_T* resp = NULL;
    D32S configReportEventLevel = ADM_CFG_VUS_EVENT_REPORT_LEVEL_DEFAULT_VAL;

    if (cmdId >= ADM_APPLICATION_COMMON_CMDID_TYPE_INVALID) 
    {
        log_e("param notifyF is NULL");
        return ADM_ECODE_INVALID_PARAM;
    }
    
    log_i("[ABUP-Procedure] usb check Start.");

	if (ADM_ECODE_SUCCESSED != adm_ecs_getEventReportLevel(&configReportEventLevel))
	{
        configReportEventLevel = adm_cfg_iniParserGetInt(ADM_CFG_VUS_EVENT_REPORT_LEVEL, ADM_CFG_VUS_EVENT_REPORT_LEVEL_DEFAULT_VAL);
	}
        
    log_i("usb check, configReportEventLevel = %d", configReportEventLevel);
    if (NULL != progressNotifyHmi) {
        notifyF = adm_cvs_initNotifyCb(progressNotifyHmi,
            adm_cvm_checkStateReport,
            adm_cvm_checkReportEvent,
            adm_irm_getEventSequenceId,
            eventNum,
            configReportEventLevel);
    } else {
        notifyF = adm_cvs_initNotifyCb(adm_cvm_checkProgressNotifyHmi,
            adm_cvm_checkStateReport,
            adm_cvm_checkReportEvent,
            adm_irm_getEventSequenceId,
            eventNum,
            configReportEventLevel);
    }

    retVal = cvmUsbCheckVersionHandle(cmdId, notifyF, NULL, cvmResp);
    log_i("retVal = %d", retVal);
    if (ADM_ECODE_SUCCESSED == retVal || ADM_ECODE_NO_NEW_VERSION == retVal
        || ADM_ECODE_CHECK_LATEST_VERSION == retVal) {
        log_i("check success");
    } else {
        log_e("check error[%d]", retVal);
    }
    adm_cvs_deInitNotifyCb(notifyF);

    log_i("[ABUP-Procedure] usb check End. [iRet = %d]", retVal);

    return retVal;
}

/** 启动后台收集ecu零件信息 */
void* adm_cvm_triggerGetEcuInfo(void *pArgv)
{
    DCHAR* allEcuInfoStr = NULL;
    D32S iRetVal = ADM_ECODE_FAILED;
    ADM_VSM_STATE_E eStat = ADM_VSM_STATE_MAX;
    static DBOOL bIsBackCollect = DFALSE;

    if (DTRUE == bIsBackCollect)
    {
        log_w("Back collecting. do not handle.");
        return NULL;
    }

    bIsBackCollect = DTRUE;
    while (TRUE)
    {
        eStat = adm_vsm_stateGet();

        if (ADM_VSM_STATE_IDLE == eStat)
        {
            log_i("Idel,start back collect.");
            iRetVal = adm_eim_getEcuInfoManagerDimGetStr(&allEcuInfoStr, COLLECT_TRIGGER_TYPE_BACK);
            if (ADM_IS_SUCCESS(iRetVal)) 
            {
                log_i("Back collect ecu info[%s].", allEcuInfoStr);
                adm_free(allEcuInfoStr);
                break;
            }
            else 
            {
                if (ADM_ECODE_EIM_BACK_COLLECT_COND_FAILED == iRetVal)
                {
                    log_i("Over max cond verify time.");
                    break;
                }
                log_i("Back collect pause.retVal[%d].", iRetVal);
                sleep(3);
            }
        }
        else if (ADM_VSM_STATE_CHECK_DOING == eStat)
        {
            log_i("Checking,wait back collect.");
            sleep(3);
        }
        else 
        {
            log_i("Stat[%d] do not need collect.", eStat);
            break;
        }
    }

    bIsBackCollect = DFALSE;

    return NULL;
}
