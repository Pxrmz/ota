#include <stdio.h>
#include "adm_cfg_api.h"
#include "adm_sysi4ucl_log.h"
#include "adm_sysi4all_data.h"
#include "adm_sysi4all_loader.h"

#include "json_types.h"
#include "json_tokener.h"
#include "adm_com_parseFileObject.h"
#include "adm_com_lib.h"
#include "adm_file.h"
#include "adm_memory.h"
#include "adm_string.h"
#include "adm_sysi4ucl_handler.h"
#include "adm_sysi4cbf.h"
#include "cbf_parse.h"
#include "adm_sysi4icc.h"
#include "adm_sysi4ucl_jni.h"
#include "adm_sysi4ucl_handler_lvm.h"
#include "adm_sysi4host_handler.h"
#include "adm_com_hash.h"
#include "lion_ota_oemUpgrade_if.h"
#include <unistd.h>
#include <fcntl.h>
#include "adm_sysi_keylog.h"

typedef struct
{
    D32S   installType;     /**ICC升级类型*/
    D32S   bIsRollback;
    ADM_SYSI4UCL_UPGRADE_INFO_ARRAY_T* upgradeInfoList;   //!< 升级包列表
} ADM_SYS4UCL_ICC_UPGPKGINFO_T;

static ADM_SYSI4UCL_JNI_CALLBACK_T stSysi4uclJniCb = {};
static DBOOL g_bIsApkUpgrading = DFALSE;        ///!< 标识APK正在升级中

#define ADM_UPGRADE_FILENAME "upgradeInfo.txt"
#define ADM_CONTROL_ACTIVE_FILENAME "controlActive.txt"

#define ICC_REQ_ID        0x0788                    /** ICC本身 */
#define ICC_INSTRUMENT_PANEL_REQ_ID 0xFFF4          /** 仪表屏 */
#define ICC_CENTRAL_CTRL_SCREEN_REQ_ID 0xFFF5       /** 中控屏 */
#define ICC_AIR_CONDITION_SCREEN_REQ_ID 0xFFF6      /** 二排空调屏 */
#define ICC_AIR_CEILING_MOUNT_REQ_ID 0xFFF7         /** 吸顶屏 */

typedef struct 
{
    D32U uiProgress;                        ///< 升级进度
    D32S iResult;                           ///< 升级结果
    ADM_SYSI4UCL_ICC_INSTALL_TYPE_E     eInstallType; ///< 升级类型
} ADM_UPGRADE_INFO_T;

static ADM_UPGRADE_INFO_T g_stUpgradeInfo = {
    .uiProgress = 0,
    .iResult = (D32S)UPGRADE_IDEL,
    .eInstallType = ICC_INSTALL_INVALID
};

static DBOOL g_bIsControlActive = DFALSE; ///< 默认不控制从控激活，由从控自行激活

static D32S sysi4uclGetInstallType(D32S iRequestId, ADM_SYSI4UCL_ICC_INSTALL_TYPE_E* pInstallType)
{
    D32S iRet = ADM_ECODE_SUCCESSED;

    if (ADM_IS_NULL(pInstallType))
    {
        iRet = ADM_ECODE_INVALID_PARAM;
        return iRet;
    }

    switch (iRequestId)
    {
        case ICC_REQ_ID:
            *pInstallType = ICC_SOC_MCU;
            break;
        case ICC_INSTRUMENT_PANEL_REQ_ID:
            *pInstallType = ICC_INSTRUMENT_PANEL;
            break;
        case ICC_CENTRAL_CTRL_SCREEN_REQ_ID:
            *pInstallType = ICC_CENTRAL_CTRL_SCREEN;
            break;
        case ICC_AIR_CONDITION_SCREEN_REQ_ID:
            *pInstallType = ICC_SECOND_ROW_AIR_CONDITION_SCREEN;
            break;
        case ICC_AIR_CEILING_MOUNT_REQ_ID:
            *pInstallType = ICC_CEILING_MOUNTED_SCREEN;
            break;
        default:
            log_e("Invalid requestId[%x].", iRequestId);
            iRet = ADM_ECODE_INVALID_PARAM;
            break;
    }

    return iRet;
}

static D32S sysi4DoUpgradeTrigger(D32S iInstallType, DBOOL bIsRollback, DCHAR* pstImageInfo)
{
    D32S iRet = ADM_ECODE_FAILED;
    DCHAR aDescription[128] = {0};
    DCHAR aSolution[128] = {0};

    CHECK_POINTER_RETURN_RET(stSysi4uclJniCb.doUpgradeTriggerCb, ADM_ECODE_INVALID_POINTER);

    iRet = stSysi4uclJniCb.doUpgradeTriggerCb(iInstallType, bIsRollback, pstImageInfo);

    adm_snprintf(aDescription, sizeof(aDescription), "installType=%d", iInstallType);
    if (ADM_IS_ERR(iRet))
    {
        adm_sysi_setKeylogString(ADM_KEYLOG_ID_016, iRet, aDescription);
    }
    else
    {
        adm_sysi_setKeylogString(ADM_KEYLOG_ID_015, iRet, aDescription);
    }

    return iRet;
}

static D32S sysi4uclGetUpgradeInfo()
{
    D32S iRet = ADM_ECODE_FAILED;
    DCHAR aFullPath[ADM_COM_LEN_256] = {0};
    DVOID* pFile = NULL;
    D32U uiSize = 0;
    DCHAR* pBuff = NULL;

    do
    {
        const DCHAR* pUpgradeDir = adm_cfg_iniParserGetString(ADM_CFG_SYSI_UPGRADE_RESULT_RECORD_DIR, ADM_CFG_SYSI_UPGRADE_RESULT_RECORD_DIR_DEFAULT_VAL);
        if (NULL == pUpgradeDir)
        {
            log_w("Can not find collect dir.");
            break;
        }
        
        (DVOID)adm_snprintf(aFullPath, sizeof(aFullPath), "%s/%s", pUpgradeDir, ADM_UPGRADE_FILENAME);

        iRet = adm_com_readFileData(aFullPath, &pBuff);
        if (ADM_IS_ERR(iRet) || ADM_IS_NULL(pBuff))
        {
            log_i("Read file data empty.file path [%s], retValue[%d]", aFullPath, iRet);
            break;
        }

        if (NULL == pBuff)
        {
            log_i("No result info find.");
            return ADM_ECODE_FAILED;
        }

        log_i("Upgrade Info [%s].", pBuff);
        
        json_object* pstResultObj = json_tokener_parse(pBuff);

        g_stUpgradeInfo.iResult = json_object_get_int(json_object_object_get(pstResultObj, "result"));
        g_stUpgradeInfo.uiProgress = (D32U)json_object_get_uint64(json_object_object_get(pstResultObj, "progress"));
        g_stUpgradeInfo.eInstallType = 
            (ADM_SYSI4UCL_ICC_INSTALL_TYPE_E)json_object_get_int(json_object_object_get(pstResultObj, "installType"));
    } while (FALSE);

    return iRet;
}

static D32S sysi4uclSetUpgradeInfo(ADM_SYSI4UCL_ICC_INSTALL_TYPE_E eInstallType, D32S iResult, D32U uiProgress)
{
    D32S iRet = ADM_ECODE_FAILED;
    DCHAR aFullPath[ADM_COM_LEN_256] = {0};
    DVOID* pFile = NULL;
    D32U uiSize = 0;


    do
    { 
        const DCHAR* pUpgradeDir = adm_cfg_iniParserGetString(ADM_CFG_SYSI_UPGRADE_RESULT_RECORD_DIR, ADM_CFG_SYSI_UPGRADE_RESULT_RECORD_DIR_DEFAULT_VAL);
        if (NULL == pUpgradeDir)
        {
            log_e("Can not find upgrade dir.");
            break;
        }
        
        (DVOID)adm_snprintf(aFullPath, sizeof(aFullPath), "%s/%s", pUpgradeDir, ADM_UPGRADE_FILENAME);

        json_object* pstResultObj = json_object_new_object();
        
        (void)json_object_object_add(pstResultObj, "installType",
                                     json_object_new_int(eInstallType));
        (void)json_object_object_add(pstResultObj, "result",
                                     json_object_new_int(iResult));
        (void)json_object_object_add(pstResultObj, "progress",
                                     json_object_new_uint64(uiProgress));

        const DCHAR* pBuff = json_object_to_json_string(pstResultObj);

        iRet = adm_com_writeFileData(aFullPath, pBuff);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Write file data error.file path [%s], retValue[%d]", aFullPath, iRet);
            break;
        }

        /** 更新内存 */
        log_i("set Info success: result[%d], progress[%u],installType[%d]", iResult, uiProgress, eInstallType);
        g_stUpgradeInfo.iResult = iResult;
        g_stUpgradeInfo.uiProgress = uiProgress;
        g_stUpgradeInfo.eInstallType = eInstallType;
    } while (FALSE);

    return iRet;
}

static DVOID sysi4uclClearUpgradeInfo()
{
    log_i("Clear result info.");
    DCHAR aFullPath[ADM_COM_LEN_256] = {0};

    g_stUpgradeInfo.iResult = (D32S)UPGRADE_IDEL;
    g_stUpgradeInfo.uiProgress = 0;
    g_stUpgradeInfo.eInstallType = ICC_INSTALL_INVALID;
    
    const DCHAR* pUpgradeDir = adm_cfg_iniParserGetString(ADM_CFG_SYSI_UPGRADE_RESULT_RECORD_DIR, ADM_CFG_SYSI_UPGRADE_RESULT_RECORD_DIR_DEFAULT_VAL);
    if (NULL == pUpgradeDir)
    {
        log_e("Can not find collect dir.");
        return;
    }
    
    (DVOID)adm_snprintf(aFullPath, sizeof(aFullPath), "%s/%s", pUpgradeDir, ADM_UPGRADE_FILENAME);

    FILE *fp = fopen(aFullPath, "wb+");
    if (NULL == fp) 
    {
        log_w("Fopen file[%s] failed.", aFullPath);
        return;
    }

    fclose(fp);
    return;
}

static D32S sysi4uclSetControlActiveInfo(DBOOL bIsControlActive)
{
    D32S iRet = ADM_ECODE_FAILED;
    DCHAR aFullPath[ADM_COM_LEN_256] = {0};
    DVOID *pFile = NULL;
    DCHAR *pBuff = NULL;
    D32U uiSize = 0;

    do
    {
        const DCHAR *pControlActiveDir = adm_cfg_iniParserGetString(ADM_CFG_SYSI_IS_CONTROL_ACTIVE_DIR, ADM_CFG_SYSI_IS_CONTROL_ACTIVE_DIR_DEFAULT_VAL);
        if (NULL == pControlActiveDir)
        {
            log_e("Can not find control active dir.");
            break;
        }

        (DVOID) adm_snprintf(aFullPath, sizeof(aFullPath), "%s/%s", pControlActiveDir, ADM_CONTROL_ACTIVE_FILENAME);

        json_object *pstObj = json_object_new_object();

        (void)json_object_object_add(pstObj, "isControlActive",
                                     json_object_new_int(bIsControlActive));

        const DCHAR *pBuff = json_object_to_json_string(pstObj);

        iRet = adm_com_writeFileData(aFullPath, pBuff);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Write file data error.file path [%s], retValue[%d]", aFullPath, iRet);
            break;
        }

        /** 更新内存 */
        log_i("set control active success.value[%s]", pBuff);
        g_bIsControlActive = bIsControlActive;
    } while (FALSE);

    return iRet;
}

D32S sysi4uclGetControlActiveInfo()
{
    D32S iRet = ADM_ECODE_FAILED;
    DCHAR aFullPath[ADM_COM_LEN_256] = {0};
    DVOID *pFile = NULL;
    D32U uiSize = 0;
    DCHAR *pBuff = NULL;

    do
    {
        const DCHAR *pControlActiveDir = adm_cfg_iniParserGetString(ADM_CFG_SYSI_IS_CONTROL_ACTIVE_DIR, ADM_CFG_SYSI_IS_CONTROL_ACTIVE_DIR_DEFAULT_VAL);
        if (NULL == pControlActiveDir)
        {
            log_w("Can not find control active dir.");
            break;
        }

        (DVOID) adm_snprintf(aFullPath, sizeof(aFullPath), "%s/%s", pControlActiveDir, ADM_CONTROL_ACTIVE_FILENAME);

        iRet = adm_com_readFileData(aFullPath, &pBuff);
        if (ADM_IS_ERR(iRet) || ADM_IS_NULL(pBuff))
        {
            log_i("Read file data empty.file path [%s], retValue[%d]", aFullPath, iRet);
            break;
        }

        if (NULL == pBuff)
        {
            log_i("No control active info find.");
            return ADM_ECODE_FAILED;
        }

        log_i("Control active Info [%s].", pBuff);

        json_object *pstObj = json_tokener_parse(pBuff);

        g_bIsControlActive = (DBOOL)json_object_get_int(json_object_object_get(pstObj, "isControlActive"));

        json_object_put(pstObj);
        adm_free(pBuff);
    } while (FALSE);

    return iRet;
}

static DVOID sysi4uclClearControlActiveInfo()
{
    log_i("Clear result info.");
    DCHAR aFullPath[ADM_COM_LEN_256] = {0};

    g_bIsControlActive = DFALSE;

    const DCHAR *pControlActiveDir = adm_cfg_iniParserGetString(ADM_CFG_SYSI_IS_CONTROL_ACTIVE_DIR, ADM_CFG_SYSI_IS_CONTROL_ACTIVE_DIR_DEFAULT_VAL);
    if (NULL == pControlActiveDir)
    {
        log_e("Can not find control active dir.");
        return;
    }

    (DVOID) adm_snprintf(aFullPath, sizeof(aFullPath), "%s/%s", pControlActiveDir, ADM_CONTROL_ACTIVE_FILENAME);

    FILE *fp = fopen(aFullPath, "wb+");
    if (NULL == fp)
    {
        log_w("Fopen file[%s] failed.", aFullPath);
        return;
    }

    fclose(fp);
    return;
}

/// @brief 宿主进度和时间通知
/// @param upgradeProgress 
/// @param requestID 
/// @param upgradeProportion 
/// @param currentEcuUpgradeTime 
/// @param remainingUpgradeTime 
/// @return 
D32S adm_sysi4ucl_notifyMcuAndScreenUpgrade(D32S upgradeProgress,D32S requestID, D32S upgradeProportion, D32S currentEcuUpgradeTime, D32S remainingUpgradeTime)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    D32S iErrorCode = DFALSE;

    CHECK_POINTER_RETURN_RET(stSysi4uclJniCb.notifyMcuAndScreenUpgradeCb, ADM_ECODE_INVALID_POINTER);

    iRet = stSysi4uclJniCb.notifyMcuAndScreenUpgradeCb(upgradeProgress, requestID, upgradeProportion, currentEcuUpgradeTime, remainingUpgradeTime, &iErrorCode);
    if (iRet){
        log_e("prereductionCb return error.");
        return iRet;
    }

    return iErrorCode;
}

/// @brief 差分升级预还原
/// @param imagePath 
/// @return 
D32S adm_sysi4ucl_prereduction(const DCHAR* imagePath)
{
    CHECK_POINTER_RETURN_RET(imagePath, ADM_ECODE_INVALID_PARAM);

    D32S iRet = ADM_ECODE_SUCCESSED;
    D32S iErrorCode = DFALSE;

    CHECK_POINTER_RETURN_RET(stSysi4uclJniCb.prereductionCb, ADM_ECODE_INVALID_POINTER);

    iRet = stSysi4uclJniCb.prereductionCb((DCHAR*)imagePath, &iErrorCode);
    if (iRet){
        log_e("prereductionCb return error.");
        return iRet;
    }

    if(0 == iErrorCode)
    {
        log_i("prereduction succeed.");
    }
    else if(1 == iErrorCode)
    {
        log_e("prereduction executing");
    }
    else
    {
        log_e("prereduction error");
    }

    return iErrorCode;
}

static DCHAR* sysi4ucl_handleImageInfo(ADM_SYSI4UCL_IMAGE_INFO_T* pstImageInfo, D16U uiLength)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    DCHAR* pImageInfo = NULL;

    if (ADM_IS_NULL(pstImageInfo) || 0 == uiLength)
    {
        log_e("sysi4ucl_handleImageInfo param is NULL");
        return NULL;
    }

    json_object* outObj = json_object_new_object();
    json_object* listObj = json_object_new_array();

    for (size_t j = 0u; j < uiLength; ++j)
    {
        json_object* outeacharray = json_object_new_object();
        (void)json_object_object_add(outeacharray, "imageType",
                                     json_object_new_int((D32S)pstImageInfo[j].eType));
        (void)json_object_object_add(outeacharray, "imagePath",
                                     json_object_new_string((const DCHAR*)pstImageInfo[j].imagePath));
        (void)json_object_array_add(listObj, outeacharray);
    }

    (void)json_object_object_add(outObj, "updateImageInfos", listObj);

    pImageInfo = adm_strdup(json_object_to_json_string(outObj));

    (void)json_object_put(outObj);

    return pImageInfo;
}

static D32S sysi4ucl_readPartInfoNotify(const ADM_SYSI4ALL_ECU_IDEN_T* pstEcuIdenInfo,
                                        const ADM_SYSI4ALL_DID_DATA_ARRAY_T* didListResponse)
{
    SYSI4LOADER_FUNC_LIST_T* func = adm_getSysi4LoaderFuncList();

    if(ADM_IS_NULL(func) || ADM_IS_NULL(func->sysi4ucl_readPartInfoNotify_mem))
    {
        log_e("readPartInfoNotify cb is NULL");
        return ADM_ECODE_INVALID_PARAM;
    }

    return func->sysi4ucl_readPartInfoNotify_mem(pstEcuIdenInfo, didListResponse);
}

static D32S sysi4ucl_handleCbfUpgrade(D32S iInstallType, const ECU_KEY_INFO_T* pstEcuInfoKey)
{
    CHECK_POINTER_RETURN_RET(pstEcuInfoKey, ADM_ECODE_INVALID_PARAM);

    D32S iRet = ADM_ECODE_SUCCESSED;

    do
    {
        //获取解压后的cbf&rsa file list
        ADM_UPGRADE_CFG_LIST_T* pstUpCfgList = admSysi4CbfGetCfgInfoList(pstEcuInfoKey);
        if(ADM_IS_NULL(pstUpCfgList))
        {
            //非 cbf&rsa格式的
            log_e("pstUpCfgList is NULL");
            iRet = ADM_ECODE_FAILED;
            break;
        }
        else 
        {
            D16U listLen = pstUpCfgList->listSize;
            log_i("listSize=%u", listLen);
            if (listLen > 1)
            {
                log_e("cbf file over one num.");
                iRet = ADM_ECODE_FAILED;
                break;
            }

            //循环拆cbf
            DCHAR cbfNoFormatPath[ADM_COM_FILEPATH_LEN] = {0};  //不带后缀的cbf文件路径
            DCHAR cbfBinPath[ADM_COM_FILEPATH_LEN] = {0};
            DCHAR cbfZipPath[ADM_COM_FILEPATH_LEN] = {0};

            ///< UCL如果是CBF + RSA，则只能是一个,不会多条 VDI可能多条
            for(auto iter = pstUpCfgList->cfgList.begin(); iter != pstUpCfgList->cfgList.end(); ++iter)
            {
                //cbf解析
                ADM_SYSI4CBF_IMAGE_CFG_T* pstImage = *iter;
                CHECK_POINTER_CONTINUE(pstImage);

                // 不传具体目录在当前cbf目录下生成bin和header json
                if (pstImage->cbfFileType == 1)
                {
                    cbf_file_output_data_bin(pstImage->pCbfPath, "");
                    admSysi4CbfGetStrrchr(pstImage->pCbfPath, '.', cbfNoFormatPath, ADM_COM_FILEPATH_LEN);
                    (void)adm_snprintf(cbfBinPath, sizeof(cbfBinPath), "%s%s", cbfNoFormatPath, ".bin");
                    (void)adm_snprintf(cbfZipPath, sizeof(cbfZipPath), "%s%s", cbfNoFormatPath, ".zip");
                    log_i("cbfBinPath=%s ,cbfZipPath=%s", cbfBinPath, cbfZipPath);

                    if (ADM_ERR_OK != adm_frename(cbfBinPath, cbfZipPath))
                    {
                        log_e("Rename bin to zip failed."); 
                        return ADM_ECODE_FAILED;  
                    }
                }

                ADM_SYSI4UCL_IMAGE_INFO_T* pstImageInfo = NULL;
                pstImageInfo = (ADM_SYSI4UCL_IMAGE_INFO_T*)adm_malloc(2* sizeof(ADM_SYSI4UCL_IMAGE_INFO_T));
                pstImageInfo[0].eType = ADM_SYSI4UCL_IMAGE_TYPE_RSA;
                pstImageInfo[0].imagePath = (int8_t*)pstImage->pRsaPath;
                // pstImageInfo[1].eType = ADM_SYSI4UCL_IMAGE_TYPE_ZIP;
                // pstImageInfo[1].imagePath = (int8_t*)cbfZipPath;
                if (pstImage->cbfFileType == 1)
                {
                    pstImageInfo[1].eType = ADM_SYSI4UCL_IMAGE_TYPE_ZIP;
                    pstImageInfo[1].imagePath = (int8_t*)cbfZipPath;
                }
                else
                {
                    pstImageInfo[1].eType = (ADM_SYSI4UCL_IMAGE_TYPE_E)pstImage->cbfFileType;
                    pstImageInfo[1].imagePath = (int8_t*)pstImage->pCbfPath;
                }
                log_i("rsa path : %s",pstImageInfo[0].imagePath);
                log_i("img path : %s",pstImageInfo[1].imagePath);


                DCHAR* pJsonStr = sysi4ucl_handleImageInfo(pstImageInfo, 2);
                if (NULL != pJsonStr)
                {
                    log_i("Trigger upgrade data:[%s]", pJsonStr);
                }
                stSysi4uclJniCb.upgradeWillStartCb();
                iRet = sysi4DoUpgradeTrigger(iInstallType, DFALSE, pJsonStr);
                log_i("doUpgradeTriggerCb call result[%d]", iRet);
                adm_free(pstImageInfo);
                adm_free(pJsonStr);
            }
        }
    } while (false);

    return iRet;
}

//解析非cbf格式的文件
static D32S sysi4ucl_parsePkgCfg(const DCHAR* pkgDir, const DCHAR* cfgJosn, ADM_SYSI4_IMAGE_CFG_T* pstImage, ADM_SYSI4UCL_IMAGE_TYPE_E* pFileType)
{
    CHECK_POINTER_RETURN_RET(pkgDir, ADM_ECODE_INVALID_PARAM);
    CHECK_POINTER_RETURN_RET(cfgJosn, ADM_ECODE_INVALID_PARAM);
    CHECK_POINTER_RETURN_RET(pstImage, ADM_ECODE_INVALID_PARAM);

    D32S iRet = ADM_ECODE_SUCCESSED;
    DBOOL isExist = FALSE;
    //解压后文件packageConfig.json是否存在
    DCHAR packageConfig[ADM_COM_FILEPATH_LEN] = {0};

    (void)adm_snprintf(packageConfig, sizeof(packageConfig), "%s/%s", pkgDir, cfgJosn);
    log_i("packageConfig=%s", packageConfig);

    isExist = adm_com_isDirExist(packageConfig);

    if(FALSE == isExist)
    {
        log_e("file[%s] not exist", packageConfig);
        return ADM_ECODE_OTACFG_FILE_NOEXIST;
    }

    //解析packageConfig.json
    DCHAR* ppackageConfigData = NULL;
    struct json_object* jsonRoot = NULL;

    do
    {
        iRet = adm_com_readFileData(packageConfig, &ppackageConfigData);

        if(ADM_IS_ERR(iRet) || (ADM_IS_NULL(ppackageConfigData)))
        {
            log_e("adm_com_readFileData failed, err[%d]", iRet);
            break;
        }

        jsonRoot = json_tokener_parse(ppackageConfigData);

        if(ADM_IS_NULL(jsonRoot) || (json_object_get_type(jsonRoot) != json_type_object))
        {
            log_e("json_tokener_parse faield");
            iRet = ADM_ECODE_STRING_TO_JSON_FAILED;
            break;
        }

        json_object* eachObj_PkgInfos = json_object_object_get(jsonRoot, "packageInfos");

        if(ADM_IS_NULL(eachObj_PkgInfos) || (json_object_get_type(eachObj_PkgInfos) != json_type_array))
        {
            log_e("json_object_object_get packageInfos faield");
            iRet = ADM_ECODE_JSON_GET_KEY_FAILED;
            break;
        }

        size_t arrayLength = json_object_array_length(eachObj_PkgInfos);

        log_i("json_tokener_parse imageInfoList arrayLength=%u", arrayLength);

        if (arrayLength > 1)
        {
            log_e("package num error.");
            iRet = ADM_ECODE_JSON_GET_KEY_FAILED;
            break;
        }

        json_object* eachObj_cbf = json_object_array_get_idx(eachObj_PkgInfos, 0);
        if (ADM_IS_NULL(eachObj_cbf))
        {
            log_e("eachObj_cbf get null.");
            iRet = ADM_ECODE_JSON_GET_KEY_FAILED;
            break;
        }
        
        //获取文件名
        const DCHAR* pFile = json_object_get_string(json_object_object_get(eachObj_cbf, "fileName"));
        if (ADM_IS_NULL(pFile))
        {
            log_e("fileName get error.object[%s]", json_object_to_json_string(eachObj_cbf));
            iRet = ADM_ECODE_JSON_GET_KEY_FAILED;
            break;
        }
        log_i("source fileName=%s", pFile);

        //文件路径
        DCHAR filePath[ADM_COM_FILEPATH_LEN] = {0};
        (void)adm_snprintf(filePath, sizeof(filePath), "%s/%s", pkgDir, pFile);
        log_i("source filePath=%s", filePath);

        pstImage->pImgPath = adm_strdup(filePath);
        pstImage->pImgHash = adm_strdup(json_object_get_string(json_object_object_get(eachObj_cbf, "fileHash")));
        pstImage->imgSize = json_object_get_int64(json_object_object_get(eachObj_cbf, "fileSize"));
        *pFileType = (ADM_SYSI4UCL_IMAGE_TYPE_E)json_object_get_int(json_object_object_get(eachObj_cbf, "fileType"));

    } while(false);

    adm_free(ppackageConfigData);
    (void)json_object_put(jsonRoot);

    return iRet;
}

static D32S sysi4ucl_verifyFile(ADM_SYSI4_IMAGE_CFG_T* pstImage)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    DCHAR oriFileHash[ADM_COM_FILEPATH_LEN] = {0};
    D64S fileSize = 0;

    CHECK_POINTER_RETURN_RET(pstImage, ADM_ECODE_INVALID_PARAM);
    CHECK_POINTER_RETURN_RET(pstImage->pImgPath, ADM_ECODE_INVALID_PARAM);
    CHECK_POINTER_RETURN_RET(pstImage->pImgHash, ADM_ECODE_INVALID_PARAM);

    fileSize = getOneFileSize(pstImage->pImgPath);
    if(fileSize != pstImage->imgSize)
    {
        log_e("cbf file[%s], req size[%lld], real size[%lld]", pstImage->pImgPath, pstImage->imgSize, fileSize);
        iRet = ADM_ECODE_FAILED;
        return iRet;
    }

    adm_com_sha256File(pstImage->pImgPath, oriFileHash, (D32U)(sizeof(oriFileHash) / sizeof(oriFileHash[0])));
    if(0 != adm_strcmp(oriFileHash, pstImage->pImgHash))
    {
        log_e("cbf file[%s],req hash[%s], real hash[%s]", pstImage->pImgPath, pstImage->pImgHash, oriFileHash);
        iRet = ADM_ECODE_FAILED;
        return iRet;
    }

    return iRet;
}

D32S sysi4ucl_upgrade_progress_ntf(const ADM_SYSI4UCL_ICC_INSTALL_TYPE_E installType, const D32U progress)
{
    D16U uiProgress = 0;

    SYSI4LOADER_FUNC_LIST_T* func = adm_getSysi4LoaderFuncList();

    if(ADM_IS_NULL(func) || ADM_IS_NULL(func->sysi4ucl_iccUpgradeProgressNotify_mem))
    {
        log_e("upgrade progress cb is NULL");
        return ADM_ECODE_FAILED;
    }

    log_w("installType[%d], progress[%u] ", installType, progress);


     //进度值校验0-100
    if(0 <= progress && 100 >= progress)
    {
        uiProgress = (D16U)progress;
    }
    else
    {
        log_e("progress[%u] is invalid", progress);
        return ADM_ECODE_FAILED;
    }

    if (uiProgress > g_stUpgradeInfo.uiProgress)
    {
        log_i("Notify progress[%u]", uiProgress);
        (DVOID)func->sysi4ucl_iccUpgradeProgressNotify_mem(installType, uiProgress);
        g_stUpgradeInfo.uiProgress = uiProgress;
    }
    else 
    {
        log_i("new progress[%u], old progress[%u], do not handle.", uiProgress, g_stUpgradeInfo.uiProgress);
    }

    if (100 == g_stUpgradeInfo.uiProgress)
    {
        g_stUpgradeInfo.uiProgress = 0;
    }

    return ADM_ECODE_SUCCESSED;
}

/** 异步通知升级结果 */
static void* sysi4uclNotifyResult(void *pArgv)
{
    D32S iRet = ADM_ECODE_FAILED;
    D16U i = 0;

    SYSI4LOADER_FUNC_LIST_T* func = adm_getSysi4LoaderFuncList();

    if (ADM_IS_NULL(func) || ADM_IS_NULL(func->sysi4ucl_iccUpgradeResultNotify_mem))
    {
        log_e("upgrade result cb is NULL");
        return NULL;
    }

    /**  5分钟超时时间,如果此时还是不能送达主控,认定系统异常,清除数据 */
    for (; i < 30; i++)
    {
        if (ADM_ECODE_SUCCESSED == g_stUpgradeInfo.iResult)
        {
            (DVOID)sysi4ucl_upgrade_progress_ntf(g_stUpgradeInfo.eInstallType, 100);
        }
        iRet = func->sysi4ucl_iccUpgradeResultNotify_mem(g_stUpgradeInfo.eInstallType , g_stUpgradeInfo.iResult);
        if (ADM_IS_SUCCESS(iRet))
        {
            break;
        }

        (void)sleep(10);
    }

    if (UPGRADE_WRITEDONE !=  g_stUpgradeInfo.iResult)
    {
        sysi4uclClearUpgradeInfo();
        sysi4uclClearControlActiveInfo();
    }

    return NULL;
}

D32S sysi4ucl_upgrade_result_ntf(const ADM_SYSI4UCL_ICC_INSTALL_TYPE_E installType, const D32S result)
{
    SYSI4LOADER_FUNC_LIST_T* func = adm_getSysi4LoaderFuncList();
    D32S iResult = UPGRADE_FAILED;
    D32S iRet = ADM_ECODE_FAILED;
    pthread_t threadId = 0;
    DCHAR aDescription[128] = {0};

    if(ADM_IS_NULL(func) || ADM_IS_NULL(func->sysi4ucl_iccUpgradeResultNotify_mem))
    {
        log_e("upgrade result cb is NULL");
        return ADM_ECODE_FAILED;
    }

    log_w("installType[%d], result[%d] ", installType, result);

    if ((D32S)UPGRADE_SUCCEED == result)
    {
        iResult = result;
    }
    else if (UPGRADE_WRITEDONE == result)
    {
        if (DTRUE == g_bIsControlActive)
        {
            iResult = result;
        }
        else
        {
            /** 设备受控,需要重启 */
            log_i("Prepare reboot.");
            adm_snprintf(aDescription, sizeof(aDescription), "installType=%d", installType);
            adm_sysi_setKeylogString(ADM_KEYLOG_ID_017, result, aDescription);
            iResult = UPGRADE_IDEL;
            CHECK_POINTER_RETURN_RET(stSysi4uclJniCb.updateAndRebootCb, ADM_ECODE_INVALID_POINTER);
            (DVOID)stSysi4uclJniCb.updateAndRebootCb();
        }
    }
    else 
    {
        iResult = result;
    }

    if (0 > iResult || (D32S)UPGRADE_SUCCEED == iResult || (D32S)UPGRADE_WRITEDONE == iResult)
    {
        if (UPGRADE_WRITEDONE != iResult)
        {
            adm_snprintf(aDescription, sizeof(aDescription), "installType=%d", installType);
            if (0 != iResult)
            {
                adm_sysi_setKeylogString(ADM_KEYLOG_ID_018, iResult, aDescription);
            }
            else
            {
                adm_sysi_setKeylogString(ADM_KEYLOG_ID_017, iResult, aDescription);
            }
        }
        else
        {
            (DVOID)sysi4ucl_upgrade_progress_ntf(installType, 100);
        }

        iRet = sysi4uclSetUpgradeInfo(installType, iResult, g_stUpgradeInfo.uiProgress);
        if (ADM_IS_ERR(iRet))
        {
            g_stUpgradeInfo.eInstallType = installType;
            g_stUpgradeInfo.iResult = iResult;
        }

        iRet = func->sysi4ucl_iccUpgradeResultNotify_mem(installType, iResult);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Notify result to ucm failed.iRet[%d]", iRet);

            if (0 == pthread_create(&threadId, NULL, sysi4uclNotifyResult, NULL)) 
            {
                log_i("pthread_create sysi4uclNotifyResult successed");
                (void)pthread_detach(threadId);
            } 
            else 
            {
                log_e("pthread_create failed");
            }
        }
        else 
        {
            if (UPGRADE_WRITEDONE != iResult)
            {
                sysi4uclClearUpgradeInfo();
                sysi4uclClearControlActiveInfo();
            }
        }
    }
    else 
    {
        log_i("upgrade result return value[%d], do not deal.", iResult);
    }

    return ADM_ECODE_SUCCESSED;
}

DCHAR* replaceSubstring_icc(const DCHAR *source, const DCHAR *pattern, const DCHAR *replacement) 
{
    DCHAR szDstPath[ADM_COM_LEN_256] = {0};
    DCHAR *pcInsertPoint = NULL;
    DCHAR *pcDstPath = NULL;

    if (ADM_IS_NULL(source) || ADM_IS_NULL(pattern) || ADM_IS_NULL(replacement))
    {
        log_e("invalid param");
        return NULL;
    }

    log_d("source: %s, pattern: %s, replacement: %s", source, pattern, replacement);

    pcInsertPoint = adm_strstr(source, pattern);
    if (NULL ==  pcInsertPoint)
    {
        log_w("not found pattern");
        pcDstPath = adm_strdup(source);
    }
    else
    {
        adm_snprintf(szDstPath, ADM_COM_LEN_256, "%s%s", replacement, source + adm_strlen(pattern));
        log_d("szDstPath: %s", szDstPath);
        pcDstPath = adm_strdup(szDstPath);
    }

    return pcDstPath;
}

/**目录转换,android共享目录转换成vdi共享目录*/
static DCHAR* sysi4QnxShareDirChange(const DCHAR* pSrcpath) 
{
    if (ADM_IS_NULL(pSrcpath))
    {
        return NULL;
    }

    return replaceSubstring_icc(pSrcpath,"/mnt/uhab_fs/update/","/update/");
}

#define ADM_PKG_CFG_NAME "packageConfig.json"
#define ADM_CMD_EXEC_LOG_NAME "cmdResult.txt"

static DBOOL b_isDoTrigger = DFALSE;
static D32S uclUnZipFile(DCHAR *srcFilePath, DCHAR *dstUnzipDir)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    DCHAR szCmd[512] = {0};
    DCHAR unzip_doneFile[128] = {0};
    D32S overTimes = 0;
    const DCHAR* pCmdLogFile = NULL;
    DCHAR aCmdLogFile[ADM_COM_LEN_256] = {0};
    
    if (ADM_IS_NULL(srcFilePath) || ADM_IS_NULL(dstUnzipDir))
    {
        log_e( "Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }

    pCmdLogFile = adm_cfg_iniParserGetString(ADM_CFG_SYSI_LOG_STORAGE_DIR, ADM_CFG_SYSI_LOG_STORAGE_DIR_DEFAULT_VAL);
    (void)adm_snprintf(aCmdLogFile, ADM_COM_LEN_256, "%s%s", pCmdLogFile, ADM_CMD_EXEC_LOG_NAME);

    (void)adm_snprintf(szCmd, sizeof(szCmd), "unzip %s -d %s 1>%s 2>&1 ", srcFilePath, dstUnzipDir, aCmdLogFile);
    log_i("===> going to run cmd[%s]", szCmd);
    (void)system(szCmd);


    (void)adm_snprintf(unzip_doneFile, sizeof(unzip_doneFile),"%s/%s",dstUnzipDir, ADM_PKG_CFG_NAME);
    while (overTimes < (8 * 60))
    {
        if (TRUE == adm_com_isFileExist(unzip_doneFile))
        {
            break;
        }
        (void)sleep(1);
        overTimes++;
        log_i("======> unzip ing :%s, wait ing: %s",srcFilePath, unzip_doneFile);
    }
    log_i("unzip %s end", dstUnzipDir);

    return ADM_ECODE_SUCCESSED;
}

static D32S sysi4ucl_deepCloneImageList(const ADM_SYSI4UCL_IMAGE_INFO_ARRAY_T* pSrc, ADM_SYSI4UCL_IMAGE_INFO_ARRAY_T* pDst)
{
    if(ADM_IS_NULL(pSrc) || ADM_IS_NULL(pDst))
    {
        log_e("input param is NULL");
        return ADM_ECODE_INVALID_PARAM;
    }

    D32S iRet = ADM_ECODE_SUCCESSED;

    if (0 < pSrc->imageInfoArrayLength)
    {
        pDst->imageInfoArrayLength = pSrc->imageInfoArrayLength;
        pDst->imageInfoArray = (ADM_SYSI4UCL_IMAGE_DETAIL_INFO_T  **)adm_malloc(sizeof(ADM_SYSI4UCL_IMAGE_DETAIL_INFO_T *) * pSrc->imageInfoArrayLength);
        if (ADM_IS_NULL(pDst->imageInfoArray))
        {
            log_e("no memory for imageInfoArray.");
            return ADM_ECODE_NO_MEMORY;
        }

        for (D16U index = 0; index < pSrc->imageInfoArrayLength; ++index)
        {
            if (ADM_IS_NULL(pSrc->imageInfoArray[index]))
            {
                pDst->imageInfoArrayLength = index;
                break;
            }

            pDst->imageInfoArray[index] = (ADM_SYSI4UCL_IMAGE_DETAIL_INFO_T *)adm_malloc(sizeof(ADM_SYSI4UCL_IMAGE_DETAIL_INFO_T ));
            if (ADM_IS_NULL(pDst->imageInfoArray[index]))
            {
                log_e("no memory for imageInfoArray[%u].", index);
                iRet = ADM_ECODE_NO_MEMORY;
                break;
            }

            pDst->imageInfoArray[index]->imageSize = pSrc->imageInfoArray[index]->imageSize;
            pDst->imageInfoArray[index]->imageHash = adm_strdup(pSrc->imageInfoArray[index]->imageHash);
            pDst->imageInfoArray[index]->imagePath = adm_strdup(pSrc->imageInfoArray[index]->imagePath);

            //增加流式拼接需求
            pDst->imageInfoArray[index]->fileMetaNum=pSrc->imageInfoArray[index]->fileMetaNum;
            pDst->imageInfoArray[index]->makePackageType=pSrc->imageInfoArray[index]->makePackageType;
            D32S fileMetaNum=pDst->imageInfoArray[index]->fileMetaNum;
            if(fileMetaNum>0 && pDst->imageInfoArray[index]->makePackageType==FILE_MAKE_PKG_TYPE){
                pDst->imageInfoArray[index]->pstFileMetaList=(ADM_COM_FILEMETA_T *)adm_malloc(sizeof(ADM_COM_FILEMETA_T)*fileMetaNum);
                for (size_t i = 0; i < fileMetaNum; i++)
                {
                    pDst->imageInfoArray[index]->pstFileMetaList[i].concatOrder=pSrc->imageInfoArray[index]->pstFileMetaList[i].concatOrder;  
                    pDst->imageInfoArray[index]->pstFileMetaList[i].type=pSrc->imageInfoArray[index]->pstFileMetaList[i].type;
                    pDst->imageInfoArray[index]->pstFileMetaList[i].size=pSrc->imageInfoArray[index]->pstFileMetaList[i].size;
                    pDst->imageInfoArray[index]->pstFileMetaList[i].name= adm_strdup(pSrc->imageInfoArray[index]->pstFileMetaList[i].name);
                }
                
            }
        }
    }
    else 
    {
        log_w("imageInfoArray num equal 0!");
    }

    return iRet;
}

static D32S sysi4ucl_deepCloneUpgradeInfo(const ADM_SYSI4UCL_UPGRADE_INFO_ARRAY_T* pSrc, ADM_SYSI4UCL_UPGRADE_INFO_ARRAY_T* pDst)
{
    if(ADM_IS_NULL(pSrc) || ADM_IS_NULL(pDst))
    {
        log_e("input param is NULL");
        return ADM_ECODE_INVALID_PARAM;
    }

    D32S iRet = ADM_ECODE_SUCCESSED;

    pDst->upgradeInfoArrayLength = pSrc->upgradeInfoArrayLength;
    pDst->upgradeInfoArray = (ADM_SYSI4UCL_UPGRADE_INFO_T**)adm_malloc(sizeof(ADM_SYSI4UCL_UPGRADE_INFO_T*) * pSrc->upgradeInfoArrayLength);
    if (ADM_IS_NULL(pDst->upgradeInfoArray))
    {
        log_e("no memory for upgradeInfoArray.");
        return ADM_ECODE_NO_MEMORY;
    }

    for (D16U index = 0; index < pSrc->upgradeInfoArrayLength; ++index)
    {
        if (ADM_IS_NULL(pSrc->upgradeInfoArray[index]))
        {
            pDst->upgradeInfoArrayLength = index;
            break;
        }

        pDst->upgradeInfoArray[index] = (ADM_SYSI4UCL_UPGRADE_INFO_T*)adm_malloc(sizeof(ADM_SYSI4UCL_UPGRADE_INFO_T));
        if (ADM_IS_NULL(pDst->upgradeInfoArray[index]))
        {
            log_e("no memory for upgradeInfoArray[%u].", index);
            iRet = ADM_ECODE_NO_MEMORY;
            break;
        }

        pDst->upgradeInfoArray[index]->srcSoftVersion = adm_strdup(pSrc->upgradeInfoArray[index]->srcSoftVersion);
        pDst->upgradeInfoArray[index]->dstSoftVersion = adm_strdup(pSrc->upgradeInfoArray[index]->dstSoftVersion);

        /** ecu信息深拷贝 */
        pDst->upgradeInfoArray[index]->pstEcuIdenInfo = (ADM_SYSI4ALL_ECU_IDEN_T*)adm_malloc(sizeof(ADM_SYSI4ALL_ECU_IDEN_T));
        if (ADM_IS_NULL(pDst->upgradeInfoArray[index]->pstEcuIdenInfo))
        {
            log_e("no memory for ecuIden.");
            iRet = ADM_ECODE_NO_MEMORY;
            break;
        }
        pDst->upgradeInfoArray[index]->pstEcuIdenInfo->functionId =  pSrc->upgradeInfoArray[index]->pstEcuIdenInfo->functionId;
        pDst->upgradeInfoArray[index]->pstEcuIdenInfo->responseId =  pSrc->upgradeInfoArray[index]->pstEcuIdenInfo->responseId;
        pDst->upgradeInfoArray[index]->pstEcuIdenInfo->requestId =  pSrc->upgradeInfoArray[index]->pstEcuIdenInfo->requestId;
        pDst->upgradeInfoArray[index]->pstEcuIdenInfo->swDid =  pSrc->upgradeInfoArray[index]->pstEcuIdenInfo->swDid;

        /** 镜像信息深拷贝 */
        pDst->upgradeInfoArray[index]->imageInfoList = (ADM_SYSI4UCL_IMAGE_INFO_ARRAY_T*)adm_malloc(sizeof(ADM_SYSI4UCL_IMAGE_INFO_ARRAY_T));
        if (ADM_IS_NULL(pDst->upgradeInfoArray[index]->imageInfoList))
        {
            log_e("no memory for imageInfoList.");
            iRet = ADM_ECODE_NO_MEMORY;
            break;
        }
        iRet = sysi4ucl_deepCloneImageList(pSrc->upgradeInfoArray[index]->imageInfoList, pDst->upgradeInfoArray[index]->imageInfoList);
        if (ADM_IS_ERR(iRet))
        {
            break;
        }
    }

    return iRet;
}
static D32S uclSplitFile(const DCHAR *srcFileName, DCHAR *dstFilePath,D32S fileMetaNum,ADM_COM_FILEMETA_T * metaInfo){
    D32S iRet = ADM_ECODE_SUCCESSED;
    if(NULL==srcFileName || NULL==dstFilePath || NULL==metaInfo){
        log_e("input param is NULL");
        return ADM_ECODE_INVALID_PARAM;
    }
    log_e("srcFileName：%s dstFilePath%s",srcFileName,dstFilePath);
    if(fileMetaNum<=0){
        log_e("get filemeta list size error");
        return ADM_ECODE_RANGE_ERROR;
    }
    if(adm_fisexist(srcFileName)!=TRUE){
        log_e("source file %s not exit",srcFileName);
        return ADM_ECODE_FILE_NOT_EXIST;
    }
    log_e("adm_fisexist");
    D64U  srcFileSize=0;
    
    struct stat fileStats = {0};
    iRet = stat(srcFileName, &fileStats);
    if (iRet != 0) 
    {
        log_e("state file [%s] failed.", srcFileName);
        return ADM_ECODE_OPEN_FILE_FAILED;
    }
    srcFileSize = (D64U)fileStats.st_size;
    log_e("adm_fgetfilesize%d:%lld",iRet,srcFileSize);
    D64S total_filesize=0;
    ADM_COM_FILEMETA_T * metaListSortArry=NULL;
    metaListSortArry=(ADM_COM_FILEMETA_T *)adm_malloc(sizeof(ADM_COM_FILEMETA_T)*fileMetaNum);
    D32S sortListNum=0;
    //先根据concatOrder进行下排序
    for (D32S i = 0; i < fileMetaNum; i++){
        if(metaInfo[i].type==FILE_METAS_TYPE_CONCAT && metaInfo[i].concatOrder<fileMetaNum){
            total_filesize+=metaInfo[i].size;
            
            metaListSortArry[metaInfo[i].concatOrder].concatOrder=metaInfo[i].concatOrder;
            metaListSortArry[metaInfo[i].concatOrder].size=metaInfo[i].size;
            metaListSortArry[metaInfo[i].concatOrder].type=metaInfo[i].type;
            metaListSortArry[metaInfo[i].concatOrder].name=adm_strdup(metaInfo[i].name);
            sortListNum++;
            log_i("concatOrder：%d", metaListSortArry[metaInfo[i].concatOrder].concatOrder);
            log_i("name：%s",  metaListSortArry[metaInfo[i].concatOrder].name);
        }
    }
    //校验下拼接数据中总文件长度是否与实际一致
    if(total_filesize!=srcFileSize){
        log_e("fiesize not match:source file size:%lld,size from metalist:%lld,",srcFileSize,total_filesize);
        return ADM_ECODE_FAILED;
    }
    for (D32S i = sortListNum-1; i >=0; i--)
    {

        if(i==0){
               DCHAR uclSplitFileName[ADM_COM_FILEPATH_LEN] = {0};
               (void)adm_snprintf(uclSplitFileName, sizeof(uclSplitFileName), "%s/%s",
                 dstFilePath, metaListSortArry[i].name);
                 log_i("start rename file i=%d",i);
                if(rename(srcFileName,uclSplitFileName)!=0){
                    log_e("rename file error");
                    iRet= ADM_ECODE_FAILED;
                }
        }else{
            
            DCHAR uclSplitFileName[ADM_COM_FILEPATH_LEN] = {0};
            (void)adm_snprintf(uclSplitFileName, sizeof(uclSplitFileName), "%s/%s",
            dstFilePath, metaListSortArry[i].name);
             //其他文件读取拷贝到目标
             iRet =adm_com_split_file(srcFileName,uclSplitFileName,metaListSortArry[i].size);
             log_i("split file return  %d", iRet);
             if(iRet!=ADM_ECODE_SUCCESSED){
                log_e("split file failed");
                iRet=  ADM_ECODE_FAILED;
                break;
             }
             int fd = open(srcFileName, O_RDWR);
             if(fd==-1){
                log_e("open file failed");
                break;
             }
             total_filesize-=metaListSortArry[i].size;
             if(ftruncate(fd,total_filesize)==-1){

                log_e("ftruncate failed");
                iRet=  ADM_ECODE_FAILED;
             }
             close(fd);
        }   
    }
    adm_free(metaListSortArry);  
    return iRet;       
}
static void* doUpgradeAsync(void* argv)
{
    ADM_SYSI4UCL_UPGRADE_INFO_ARRAY_T* upgradeInfoList = (ADM_SYSI4UCL_UPGRADE_INFO_ARRAY_T*)argv;
    D32S iRet = ADM_ECODE_FAILED;
    DBOOL bIsCbfFmt = DTRUE;        ///< 默认升级包为cbf格式
    ADM_SYSI4_IMAGE_CFG_T* pstImage = NULL;
    ADM_SYSI4ALL_ECU_IDEN_T stEcuId = {0};
    ADM_SYSI4UCL_IMAGE_TYPE_E eFileType = ADM_SYSI4UCL_IMAGE_TYPE_OTHER;
    ADM_SYSI4UCL_ICC_INSTALL_TYPE_E eInstallType = ICC_INSTALL_INVALID;

    if(ADM_IS_NULL(upgradeInfoList))
    {
        log_e("input param is NULL");
        b_isDoTrigger = DFALSE;
        return DNULL;
    }

    if (DNULL == stSysi4uclJniCb.doUpgradeTriggerCb)
    {
        log_e("stSysi4uclJniCb.doUpgradeTriggerCb is nil");
        b_isDoTrigger = DFALSE;
        return DNULL;
    }
    // CHECK_POINTER_RETURN_VOID(stSysi4uclJniCb.doUpgradeTriggerCb);
    
    for(D16U index = 0; index < upgradeInfoList->upgradeInfoArrayLength; ++index)
    {
        ADM_SYSI4UCL_UPGRADE_INFO_T* pstUpInfo = upgradeInfoList->upgradeInfoArray[index];
        CHECK_POINTER_CONTINUE(pstUpInfo);
        CHECK_POINTER_CONTINUE(pstUpInfo->imageInfoList);

        ECU_KEY_INFO_T stEcuKeyInfo;
        stEcuKeyInfo.uiRequestId = pstUpInfo->pstEcuIdenInfo->requestId;
        stEcuKeyInfo.uiSwDid = pstUpInfo->pstEcuIdenInfo->swDid;

        iRet = sysi4uclGetInstallType(pstUpInfo->pstEcuIdenInfo->requestId, &eInstallType);
        if (ADM_IS_ERR(iRet))
        {
            (DVOID)adm_sysi4ucl_upgradeIccResultNotify(eInstallType, iRet);
            break;
        }

        stEcuId.requestId = pstUpInfo->pstEcuIdenInfo->requestId;
        stEcuId.responseId = pstUpInfo->pstEcuIdenInfo->responseId;
        stEcuId.functionId = pstUpInfo->pstEcuIdenInfo->functionId;
        stEcuId.swDid = pstUpInfo->pstEcuIdenInfo->swDid;

        //只有 requestId生效 见 ADM_CUSTOM_ECU_E
        for(D16U imageIndex = 0; imageIndex < pstUpInfo->imageInfoList->imageInfoArrayLength; ++imageIndex)
        {
            ADM_SYSI4UCL_IMAGE_DETAIL_INFO_T * pstImageInfo = pstUpInfo->imageInfoList->imageInfoArray[imageIndex];
            CHECK_POINTER_CONTINUE(pstImageInfo);
            CHECK_POINTER_CONTINUE(pstImageInfo->imagePath);
            //获取升级包目录
            DCHAR currentImageDir[ADM_COM_FILEPATH_LEN] = {0};
            admSysi4CbfGetStrrchr(pstImageInfo->imagePath, '/', currentImageDir, ADM_COM_FILEPATH_LEN);

            log_i("currentImageDir=%s", currentImageDir);
            //拼接一个解压目录
            DCHAR unzipDir[ADM_COM_FILEPATH_LEN] = {0};
            (void)adm_snprintf(unzipDir, sizeof(unzipDir), "%s/0x%04x_%u",
                               currentImageDir, pstUpInfo->pstEcuIdenInfo->requestId, imageIndex);
            log_i("unzipDir=%s", unzipDir);

            if (DTRUE == adm_fisexist(pstImageInfo->imagePath))
            {
                DBOOL isExist = FALSE;
                isExist = adm_com_isDirExist(unzipDir);

                if(TRUE == isExist)
                {
                    log_i("remove dir[%s]", unzipDir);
                    (void)adm_com_rmDirAndFiles(unzipDir);
                }

                log_i("create dir[%s]", unzipDir);
                iRet = adm_com_createDir(unzipDir);
                if(ADM_IS_ERR(iRet))
                {
                    log_e("creat dir[%s] failed", unzipDir);
                    break;
                }

                /** 1.打印下待解压文件大小 2.判断待解压路径是否真的已创建成功 */
                log_i("File[%s] size[%lld]", pstImageInfo->imagePath, getOneFileSize(pstImageInfo->imagePath));

                if (DFALSE == adm_com_isDirExist(unzipDir))
                {
                    log_w("Dir[%s] does not exists.", unzipDir);
                    sleep(1);
                }

                if(pstImageInfo->makePackageType==FILE_MAKE_PKG_TYPE){
                    //TODO 在指定目录拆分文件---拼接文件拆分
                   iRet=uclSplitFile(pstImageInfo->imagePath, unzipDir,pstImageInfo->fileMetaNum,pstImageInfo->pstFileMetaList); 
                   if(ADM_IS_ERR(iRet))
                   {
                       log_e("uclSplitFile[%s] to dir[%s] failed,iRet[%d]", pstImageInfo->imagePath,
                           unzipDir, iRet);
                       break;
                   }
                }else{
                   //解压到指定目录
                   iRet = uclUnZipFile(pstImageInfo->imagePath, unzipDir);
                   if(ADM_IS_ERR(iRet))
                   {
                       log_e("unZipFile[%s] to dir[%s] failed,iRet[%d]", pstImageInfo->imagePath,
                           unzipDir, iRet);
                       break;
                   }
                }

            }

            //解析packageConfig.json
            iRet = admSysi4CbfParsePkgCfg(&stEcuKeyInfo, unzipDir, ADM_PKG_CFG_NAME);
            if (ADM_ECODE_SUCCESSED != iRet)
            {
                if (ADM_ECODE_PRASE_RSACBF_FAILED == iRet)
                {
                    /** 如果非rsa+cbf格式，直接就是原包进行升级 */
                    bIsCbfFmt = DFALSE;
                    pstImage = (ADM_SYSI4_IMAGE_CFG_T*)adm_malloc(sizeof(ADM_SYSI4_IMAGE_CFG_T));
                    iRet = sysi4ucl_parsePkgCfg(unzipDir, ADM_PKG_CFG_NAME, pstImage, &eFileType);
                    if (ADM_IS_SUCCESS(iRet))
                    {
                        iRet = sysi4ucl_verifyFile(pstImage);
                        if (ADM_ECODE_SUCCESSED != iRet)
                        {
                            log_e("verify file failed");
                            break;
                        }
                        else 
                        {
                            log_i("delete zip file[%s]", pstImageInfo->imagePath);
                            (DVOID)adm_fremove(pstImageInfo->imagePath);
                        }
                    }
                }
                else 
                {
                    log_e("parse packageConfig.json failed.retValue[%d]", iRet);
                    break;
                }
            }
            else 
            {
                //解压后验证原cbf和rsa hash
                if( pstImageInfo->makePackageType!=FILE_MAKE_PKG_TYPE){
                     iRet = admSysi4CbfVerifyFile(&stEcuKeyInfo);
                     if (ADM_ECODE_SUCCESSED != iRet)
                     {
                         log_e("cbf verify file failed");
                         break;
                      }
                     else 
                      {
                         log_i("delete zip file[%s]", pstImageInfo->imagePath);
                          (DVOID)adm_fremove(pstImageInfo->imagePath);
                       }
                   }
             }
        }
         
        if (ADM_IS_SUCCESS(iRet))
        {
            if (DTRUE == bIsCbfFmt)
            {
                /** cbf格式包处理 */
                iRet = sysi4ucl_handleCbfUpgrade(eInstallType, &stEcuKeyInfo);
                if (ADM_IS_ERR(iRet))
                {
                    (DVOID)adm_sysi4ucl_upgradeIccResultNotify(eInstallType, iRet);
                }
            }
            else 
            {
                ADM_SYSI4UCL_IMAGE_INFO_T stImageInfo = {};
                log_i("upgrade path[%s]", pstImage->pImgPath);
                log_i("requestId[%0x] swDid[%0x]", stEcuId.requestId, stEcuId.swDid);
                /** 非cbf格式包处理 */
                stImageInfo.eType = eFileType;
                stImageInfo.imagePath = (int8_t*)pstImage->pImgPath;

                DCHAR* pJsonStr = sysi4ucl_handleImageInfo(&stImageInfo, 1);
                b_isDoTrigger = DFALSE;
                // 1:整包升级; 2:MCU ;7:SOC TODO adaptor
                stSysi4uclJniCb.upgradeWillStartCb();
                iRet = sysi4DoUpgradeTrigger(eInstallType, DFALSE, pJsonStr);
                if (ADM_IS_ERR(iRet))
                {
                    (DVOID)adm_sysi4ucl_upgradeIccResultNotify(eInstallType, iRet);
                }
                log_i("lionOtaDoUpgrade call result[%d]", iRet);

                adm_free(pJsonStr);
            }
        }
        else
        {
            log_i("start notify ucm upgrade failed ...");
            D32S iResult = UPGRADE_FAILED;
            iRet = sysi4uclSetUpgradeInfo(eInstallType, iResult, g_stUpgradeInfo.uiProgress);
            if (ADM_IS_ERR(iRet))
            {
                g_stUpgradeInfo.eInstallType = eInstallType;
                g_stUpgradeInfo.iResult = iResult;
                log_i("set upgrade info failed, eInstallType %d , iResult %d", eInstallType, iResult);
            }
            SYSI4LOADER_FUNC_LIST_T* func = adm_getSysi4LoaderFuncList();
            pthread_t threadId = 0;
            if(ADM_IS_NULL(func) || ADM_IS_NULL(func->sysi4ucl_iccUpgradeResultNotify_mem))
            {
                log_e("upgrade result cb is NULL");
            }
            iRet = func->sysi4ucl_iccUpgradeResultNotify_mem(eInstallType, iResult);
            if (ADM_IS_ERR(iRet))
            {
                log_e("Notify result to ucm failed.iRet[%d]", iRet);

                if (0 == pthread_create(&threadId, NULL, sysi4uclNotifyResult, NULL)) 
                {
                    log_i("pthread_create sysi4uclNotifyResult successed");
                    (void)pthread_detach(threadId);
                } 
                else 
                {
                    log_e("pthread_create failed");
                }
            }
            else 
            {
                sysi4uclClearUpgradeInfo();
                sysi4uclClearControlActiveInfo();
            }
            log_i("end notify ucm upgrade failed ...");
        }
    }

    // *errorCode = iRet;

    //free
    for(D16U index = 0; index < upgradeInfoList->upgradeInfoArrayLength; ++index)
    {
        ADM_SYSI4UCL_UPGRADE_INFO_T* pstUpInfo = upgradeInfoList->upgradeInfoArray[index];
        CHECK_POINTER_CONTINUE(pstUpInfo);

        ECU_KEY_INFO_T stEcuKeyInfo;

        if(NULL != pstUpInfo->pstEcuIdenInfo)
        {
            stEcuKeyInfo.uiRequestId = pstUpInfo->pstEcuIdenInfo->requestId;
            stEcuKeyInfo.uiSwDid = pstUpInfo->pstEcuIdenInfo->swDid;
        }

        admSysi4CbfClear(&stEcuKeyInfo);
    }

    b_isDoTrigger = DFALSE;
    return DNULL;
}

D32S adm_sysi4ucl_doUpgradeTrigger(const ADM_SYSI4UCL_UPGRADE_INFO_ARRAY_T *upgradeInfoList,
                                   DBOOL bIsControlActive, D32S *errorCode, void *priv)
{
    D32S iRet = ADM_ECODE_FAILED;
    // DBOOL bIsCbfFmt = DTRUE;        ///< 默认升级包为cbf格式
    // ADM_SYSI4_IMAGE_CFG_T* pstImage = NULL;
    // ADM_SYSI4ALL_ECU_IDEN_T stEcuId = {0};
    // ADM_SYSI4UCL_IMAGE_TYPE_E eFileType = ADM_SYSI4UCL_IMAGE_TYPE_OTHER;
    ADM_SYSI4UCL_UPGRADE_INFO_ARRAY_T* pstDstUpgradeInfoList = DNULL;
    pthread_t tid = 0;

    if (DTRUE == b_isDoTrigger)
    {
        log_e("Async upgrading now, do not trigger again");
        return ADM_ECODE_SUCCESSED;
    }

    if(ADM_IS_NULL(upgradeInfoList) || ADM_IS_NULL(errorCode))
    {
        log_e("input param is NULL");
        return ADM_ECODE_INVALID_PARAM;
    }

    CHECK_POINTER_RETURN_RET(stSysi4uclJniCb.doUpgradeTriggerCb, ADM_ECODE_INVALID_POINTER);

    iRet = sysi4uclSetControlActiveInfo(bIsControlActive);
    if (ADM_IS_ERR(iRet))
    {
        g_bIsControlActive = bIsControlActive;
    }

    pstDstUpgradeInfoList = (ADM_SYSI4UCL_UPGRADE_INFO_ARRAY_T*)adm_malloc(sizeof(ADM_SYSI4UCL_UPGRADE_INFO_ARRAY_T));
    iRet = sysi4ucl_deepCloneUpgradeInfo(upgradeInfoList, pstDstUpgradeInfoList);
    if (ADM_IS_ERR(iRet))
    {
        log_e("sysi4ucl_deepCloneUpgradeInfo failed, [err:%d]", iRet);
    }
    else
    {
        if(0 != pthread_create(&tid, NULL, doUpgradeAsync, (void*)pstDstUpgradeInfoList))
        {
            log_e("pthread_create tid:%d failed", tid);
            iRet = ADM_ECODE_PTHREAD_CREATE_FAILED;
        }
        else
        {
            b_isDoTrigger = DTRUE;
            (void)usleep(100);
            pthread_detach(tid);
        }
    }

#if 0
    for(D16U index = 0; index < upgradeInfoList->upgradeInfoArrayLength; ++index)
    {
        ADM_SYSI4UCL_UPGRADE_INFO_T* pstUpInfo = upgradeInfoList->upgradeInfoArray[index];
        CHECK_POINTER_CONTINUE(pstUpInfo);
        CHECK_POINTER_CONTINUE(pstUpInfo->imageInfoList);

        ECU_KEY_INFO_T stEcuKeyInfo;
        stEcuKeyInfo.uiRequestId = pstUpInfo->pstEcuIdenInfo->requestId;
        stEcuKeyInfo.uiSwDid = pstUpInfo->pstEcuIdenInfo->swDid;

        stEcuId.requestId = pstUpInfo->pstEcuIdenInfo->requestId;
        stEcuId.responseId = pstUpInfo->pstEcuIdenInfo->responseId;
        stEcuId.functionId = pstUpInfo->pstEcuIdenInfo->functionId;
        stEcuId.swDid = pstUpInfo->pstEcuIdenInfo->swDid;

        //只有 requestId生效 见 ADM_CUSTOM_ECU_E
        for(D16U imageIndex = 0; imageIndex < pstUpInfo->imageInfoList->imageInfoArrayLength; ++imageIndex)
        {
            ADM_SYSI4UCL_IMAGE_DETAIL_INFO_T * pstImageInfo = pstUpInfo->imageInfoList->imageInfoArray[imageIndex];
            CHECK_POINTER_CONTINUE(pstImageInfo);
            CHECK_POINTER_CONTINUE(pstImageInfo->imagePath);
            //获取升级包目录
            DCHAR currentImageDir[ADM_COM_FILEPATH_LEN] = {0};
            admSysi4CbfGetStrrchr(pstImageInfo->imagePath, '/', currentImageDir, ADM_COM_FILEPATH_LEN);

            log_i("currentImageDir=%s", currentImageDir);
            //拼接一个解压目录
            DCHAR unzipDir[ADM_COM_FILEPATH_LEN] = {0};
            (void)adm_snprintf(unzipDir, sizeof(unzipDir), "%s/0x%04x_%u",
                               currentImageDir, pstUpInfo->pstEcuIdenInfo->requestId, imageIndex);
            log_i("unzipDir=%s", unzipDir);

            DBOOL isExist = FALSE;
            isExist = adm_com_isDirExist(unzipDir);

            if(TRUE == isExist)
            {
                log_i("remove dir[%s]", unzipDir);
                (void)adm_com_rmDirAndFiles(unzipDir);
            }

            log_i("create dir[%s]", unzipDir);
            iRet = adm_com_createDir(unzipDir);

            if(ADM_IS_ERR(iRet))
            {
                log_e("creat dir[%s] failed", unzipDir);
                break;
            }

            //解压到指定目录
            //iRet = adm_com_unZipFile(pstImageInfo->imagePath, unzipDir);
            iRet = uclUnZipFile(pstImageInfo->imagePath, unzipDir);
            if(ADM_IS_ERR(iRet))
            {
                log_e("unZipFile[%s] to dir[%s] failed,iRet[%d]", pstImageInfo->imagePath,
                      unzipDir, iRet);
                break;
            }

            //解析packageConfig.json
            iRet = admSysi4CbfParsePkgCfg(&stEcuKeyInfo, unzipDir, ADM_PKG_CFG_NAME);
            if (ADM_ECODE_SUCCESSED != iRet)
            {
                if (ADM_ECODE_PRASE_RSACBF_FAILED == iRet)
                {
                    /** 如果非rsa+cbf格式，直接就是原包进行升级 */
                    bIsCbfFmt = DFALSE;
                    pstImage = (ADM_SYSI4_IMAGE_CFG_T*)adm_malloc(sizeof(ADM_SYSI4_IMAGE_CFG_T));
                    iRet = sysi4ucl_parsePkgCfg(unzipDir, ADM_PKG_CFG_NAME, pstImage, &eFileType);
                    if (ADM_IS_SUCCESS(iRet))
                    {
                        iRet = sysi4ucl_verifyFile(pstImage);
                        if (ADM_ECODE_SUCCESSED != iRet)
                        {
                            log_e("verify file failed");
                            break;
                        }
                    }
                }
                else 
                {
                    log_e("parse packageConfig.json failed.retValue[%d]", iRet);
                    break;
                }
            }
            else 
            {
                //解压后验证原cbf和rsa hash
                iRet = admSysi4CbfVerifyFile(&stEcuKeyInfo);
                if (ADM_ECODE_SUCCESSED != iRet)
                {
                    log_e("cbf verify file failed");
                    break;
                }
            }
        }

        if (ADM_IS_SUCCESS(iRet))
        {
            if (DTRUE == bIsCbfFmt)
            {
                /** cbf格式包处理 */
                iRet = sysi4ucl_handleCbfUpgrade(ICC_SOC_MCU, &stEcuKeyInfo);
            }
            else 
            {
                ADM_SYSI4UCL_IMAGE_INFO_T stImageInfo = {};
                log_i("upgrade path[%s]", pstImage->pImgPath);
                log_i("requestId[%u] swDid[%u]", stEcuId.requestId, stEcuId.swDid);
                /** 非cbf格式包处理 */
                stImageInfo.eType = eFileType;
                stImageInfo.imagePath = (int8_t*)pstImage->pImgPath;

                DCHAR* pJsonStr = sysi4ucl_handleImageInfo(&stImageInfo, 1);
                // 1:整包升级; 2:MCU ;7:SOC TODO adaptor
                stSysi4uclJniCb.upgradeWillStartCb();
                iRet = sysi4DoUpgradeTrigger(ICC_SOC_MCU, DFALSE, pJsonStr);
                log_i("lionOtaDoUpgrade call result[%d]", iRet);

                adm_free(pJsonStr);
            }
        }
    }

#endif
    *errorCode = iRet;

    //free
    for(D16U index = 0; index < upgradeInfoList->upgradeInfoArrayLength; ++index)
    {
        ADM_SYSI4UCL_UPGRADE_INFO_T* pstUpInfo = upgradeInfoList->upgradeInfoArray[index];
        CHECK_POINTER_CONTINUE(pstUpInfo);

        ECU_KEY_INFO_T stEcuKeyInfo;

        if(NULL != pstUpInfo->pstEcuIdenInfo)
        {
            stEcuKeyInfo.uiRequestId = pstUpInfo->pstEcuIdenInfo->requestId;
            stEcuKeyInfo.uiSwDid = pstUpInfo->pstEcuIdenInfo->swDid;
        }

        // admSysi4CbfClear(&stEcuKeyInfo);
    }

    return iRet;
}

D32S adm_sysi4ucl_queryUpgradeProgress(const ADM_SYSI4ALL_ECU_IDEN_T* pstEcuIdenInfo,
                                       D16U* progress, D32S* errorCode, void* priv)
{
    D32S iRet = ADM_ECODE_FAILED;
    ADM_SYSI4UCL_ICC_INSTALL_TYPE_E eInstallType = ICC_INSTALL_INVALID;
    // ADM_SYSI4ALL_ECU_IDEN_T stOemEcuId = {};

    if(ADM_IS_NULL(pstEcuIdenInfo) || ADM_IS_NULL(progress) || ADM_IS_NULL(errorCode))
    {
        log_e("input param is NULL");
        return ADM_ECODE_INVALID_PARAM;
    }

    CHECK_POINTER_RETURN_RET(stSysi4uclJniCb.getUpgradeProgressCb, ADM_ECODE_INVALID_POINTER);

    // stOemEcuId.requestId = pstEcuIdenInfo->requestId;
    // stOemEcuId.responseId = pstEcuIdenInfo->responseId;
    // stOemEcuId.functionId = pstEcuIdenInfo->functionId;
    // stOemEcuId.swDid = pstEcuIdenInfo->swDid;

    iRet = sysi4uclGetInstallType(pstEcuIdenInfo->requestId, &eInstallType);
    if (ADM_IS_ERR(iRet))
    {
        return iRet;
    }

    D32S iPkgprogress = 0;

    iRet = stSysi4uclJniCb.getUpgradeProgressCb(eInstallType, &iPkgprogress);

    //进度值校验0-100
    if(100 < iPkgprogress)
    {
        log_e("iPkgprogress[%d] is invalid", iPkgprogress);
    }

    log_i("progress[%d]", iPkgprogress);

    *progress = (D16U)iPkgprogress;

    *errorCode = iRet;

    return iRet;
}

D32S adm_sysi4ucl_queryUpgraderesult(const ADM_SYSI4ALL_ECU_IDEN_T* pstEcuIdenInfo,
                                     ADM_SYSI4UCL_UPGRADE_RESULT_INFO_T** pstUpgradeResultInfo, D32S* errorCode,
                                     void* priv)
{
    D32S iRet = ADM_ECODE_FAILED;
    // ADM_SYSI4ALL_ECU_IDEN_T stOemEcuId = {};
    D32S iUpResult = 0;
    ADM_SYSI4UCL_ICC_INSTALL_TYPE_E eInstallType = ICC_INSTALL_INVALID;

    if(ADM_IS_NULL(pstEcuIdenInfo) || ADM_IS_NULL(errorCode))
    {
        log_e("input param is NULL");
        return ADM_ECODE_INVALID_PARAM;
    }

    CHECK_POINTER_RETURN_RET(stSysi4uclJniCb.getUpgradeResultCb, ADM_ECODE_INVALID_POINTER);

    // stOemEcuId.requestId = pstEcuIdenInfo->requestId;
    // stOemEcuId.responseId = pstEcuIdenInfo->responseId;
    // stOemEcuId.functionId = pstEcuIdenInfo->functionId;
    // stOemEcuId.swDid = pstEcuIdenInfo->swDid;

    ///!< 解压等处理中，查询结果返回默认值
    if (DTRUE == b_isDoTrigger)
    {
        iUpResult = 0xFF;
    }
    else
    {
        //(DVOID)sysi4uclGetInstallType(pstEcuIdenInfo->requestId, &eInstallType);
        //iRet = stSysi4uclJniCb.getUpgradeResultCb(eInstallType, &iUpResult);
    }

    *pstUpgradeResultInfo = (ADM_SYSI4UCL_UPGRADE_RESULT_INFO_T*)adm_malloc(sizeof(ADM_SYSI4UCL_UPGRADE_RESULT_INFO_T));

    if(ADM_IS_NULL(*pstUpgradeResultInfo))
    {
        log_e("adm_malloc failed");
        return ADM_ECODE_NO_MEMORY;
    }

    if (iUpResult < 0)
    {
        iUpResult = UPGRADE_FAILED;
    }
    (*pstUpgradeResultInfo)->result = 2;
    (*pstUpgradeResultInfo)->expectVal = DNULL;
    (*pstUpgradeResultInfo)->realVal = DNULL;

    *errorCode = iRet;

    return iRet;
}


// icc-host读取零件信息,ucl适配层直接调用host的功能
D32S adm_sysi4ucl_readPartInfo(const ADM_SYSI4ALL_ECU_IDEN_T* pstEcuIdenInfo,
                            ADM_SYSI4ALL_DID_DATA_ARRAY_T* didArrayRequest, D32S* errorCode, void* priv)
{
    D32S iRet = ADM_ECODE_FAILED;
    ADM_SYSI4UCL_ICC_INSTALL_TYPE_E eInstallType = ICC_INSTALL_INVALID;

    if(ADM_IS_NULL(pstEcuIdenInfo) || ADM_IS_NULL(didArrayRequest) || ADM_IS_NULL(errorCode))
    {
        log_e("input param is NULL");
        return ADM_ECODE_INVALID_PARAM;
    }

    iRet = sysi4uclGetInstallType(pstEcuIdenInfo->requestId, &eInstallType);
    if (ADM_IS_SUCCESS(iRet))
    {
        iRet = adm_sysi4host_readEcuInfo((D32S)eInstallType, didArrayRequest, errorCode, priv);
    }

    /** 类型不匹配,需要通知空的结果 */
    (DVOID)sysi4ucl_readPartInfoNotify(pstEcuIdenInfo, didArrayRequest);

    *errorCode = ADM_ECODE_SUCCESSED;

    return iRet;
}

D32S adm_sysi4ucl_readSnInfo(D32S* errorCode, DCHAR** ppSnInfo, void* priv)
{
    D32S iRet = ADM_ECODE_FAILED;
    if(ADM_IS_NULL(ppSnInfo) || ADM_IS_NULL(errorCode))
    {
        log_e("input param is NULL");
        return ADM_ECODE_INVALID_PARAM;
    }

    iRet = adm_sysi4host_readSnInfo(errorCode, ppSnInfo, priv);

    return iRet;
}

//获取切换分区标志位
// static D32S sysi4ucl_GetCurrentSlot(sysi4all_variable_pt pValue)
// {
//     CHECK_POINTER_RETURN_RET(pValue, ADM_ECODE_INVALID_POINTER);
//     D32S iErrCode = 0;
//     D32S iSlotNum = 0;

//     CHECK_POINTER_RETURN_RET(stSysi4uclJniCb.getChangeSectionMarkerCb, ADM_ECODE_INVALID_POINTER);

//     iErrCode = stSysi4uclJniCb.getChangeSectionMarkerCb(&iSlotNum);
//     log_i("iSlotNum=%d", iSlotNum);

//     pValue->data_type = VAR_S32;
//     pValue->data_length = sizeof(D32S);
//     pValue->v.value_s32 = iSlotNum;
//     log_i("GetCurrentSlot value=%d", pValue->v.value_s32);
//     return iErrCode;
// }
// static D32S sysi4ucl_SetChangeSectionSlot(void)
// {
//     D32S iRet = ADM_ECODE_SUCCESSED;
//     DBOOL bRet = DFALSE;

//     CHECK_POINTER_RETURN_RET(stSysi4uclJniCb.setChangeSectionMarkerCb, ADM_ECODE_INVALID_POINTER);

//     iRet = stSysi4uclJniCb.setChangeSectionMarkerCb(&bRet);

//     if(DTRUE == bRet)
//     {
//         log_i("succeed.");
//     }
//     else if(DFALSE == bRet)
//     {
//         log_e("failed");
//     }

//     return iRet;
// }

static D32S sysi4ucl_UpdateAndReboot(D32S* errorCode, void* priv)
{
    D32S iRet = ADM_ECODE_SUCCESSED;

    log_i("Prepare reboot.");
    CHECK_POINTER_RETURN_RET(stSysi4uclJniCb.updateAndRebootCb, ADM_ECODE_INVALID_POINTER);
    iRet = stSysi4uclJniCb.updateAndRebootCb();
    *errorCode = iRet;
    return iRet;
}

static D32S sysi4ucl_set_vehicleCtl(D32U msgId, sysi4all_variable_pt pValue)
{
    D32S iRet = 0;
    VEHICLE_CTRL_E eVehicleCtl = (VEHICLE_CTRL_E)msgId;

    switch(eVehicleCtl)
    {
        // case VEHICLE_CTRL_CHANGE_SLOT:
        // {
        //     iRet = sysi4ucl_SetChangeSectionSlot();
        //     break;
        // }

        case VEHICLE_CTRL_UPGEADE_REBOOT:
        {
            // iRet = sysi4ucl_UpdateAndReboot();
            // break;
        }

        default:
        {
            break;
        }
    }

    return iRet;
}

// static D32S sysi4ucl_get_vehicleState(D32U msgId, sysi4all_variable_pt pValue)
// {
//     D32S iRet = ADM_ECODE_SUCCESSED;
//     VEHICLE_STATUS_E eVehicleState = (VEHICLE_STATUS_E)msgId;

//     switch(eVehicleState)
//     {
//         case VEHICLE_STATUS_SLOT:
//         {
//             iRet = sysi4ucl_GetCurrentSlot(pValue);
//             break;
//         }

//         default:
//         {
//             break;
//         }
//     }

//     return iRet;
// }

D32S adm_sysi4ucl_read_VehicleStatue(D32U msgId, sysi4all_variable_pt value,
                                     void* priv, D32U priv_size)
{
    D32S iRet = ADM_ECODE_FAILED;
    CHECK_POINTER_RETURN_RET(value, ADM_ECODE_INVALID_POINTER);

    // if((VEHICLE_STATUS_BEGIN < (VEHICLE_STATUS_E)msgId) && (VEHICLE_STATUS_END > (VEHICLE_STATUS_E)msgId))
    // {
        // iRet = sysi4ucl_get_vehicleState(msgId, value);
        log_i("adm_sysi4ucl_read_VehicleStatue no handler.");
    // }

    return iRet;
}
D32S adm_sysi4ucl_set_VehicleStatue(D32U msgId, sysi4all_variable_pt value,
                                    void* priv, D32U priv_size)
{
    D32S iRet = 0;

    if((VEHICLE_CTRL_BEGIN < (VEHICLE_CTRL_E)msgId) && (VEHICLE_CTRL_END > (VEHICLE_CTRL_E)msgId))
    {
        iRet = sysi4ucl_set_vehicleCtl(msgId, value);
    }

    return iRet;
}

D32S adm_sysi4ucl_getApkVersion(const ADM_SYSI4ALL_ECU_IDEN_T* pstEcuIdenInfo, DCHAR** pApkVersion, D32S* errorCode, void* priv)
{
    D32S iRet = ADM_ECODE_FAILED;

    log_i("adm_sysi4ucl_getApkVersion begin");
    if (ADM_IS_NULL(pApkVersion) || ADM_IS_NULL(errorCode))
    {
        log_e("Input param is nil");
        return ADM_ECODE_INVALID_PARAM;
    }

    CHECK_POINTER_RETURN_RET(stSysi4uclJniCb.getApkVersionCb, ADM_ECODE_INVALID_POINTER);

    iRet = stSysi4uclJniCb.getApkVersionCb(pApkVersion);
    if (ADM_IS_SUCCESS(iRet) && (DNULL != *pApkVersion))
    {
        log_i("apkVersion:%s", *pApkVersion);
    }
    *errorCode = iRet;

    log_i("adm_sysi4ucl_getApkVersion end");
    return iRet;
}

D32S adm_sysi4ucl_doApkSelfUpgrade(const ADM_SYSI4ALL_ECU_IDEN_T* pstEcuIdenInfo, DCHAR* pApkPackPath, DCHAR* pApkVersion, D32S* errorCode, void* priv)
{
    D32S iRet = ADM_ECODE_FAILED;
    DBOOL bIsCbfFmt = FALSE;        ///< 默认升级包为cbf格式
    DCHAR currentImageDir[ADM_COM_FILEPATH_LEN] = {0};
    ADM_SYSI4_IMAGE_CFG_T* pstImage = NULL;
    ADM_SYSI4UCL_IMAGE_TYPE_E eFileType = ADM_SYSI4UCL_IMAGE_TYPE_OTHER;

    log_i("adm_sysi4ucl_doApkSelfUpgrade begin");
    if (ADM_IS_NULL(pstEcuIdenInfo) || ADM_IS_NULL(pApkPackPath) || ADM_IS_NULL(pApkVersion) || ADM_IS_NULL(errorCode))
    {
        log_e("Input param is nil");
        return ADM_ECODE_INVALID_PARAM;
    }

    log_i("upgrade path[%s], pApkVersion:%s", pApkPackPath, pApkVersion);
    log_i("requestId[%u] swDid[%u]", pstEcuIdenInfo->requestId, pstEcuIdenInfo->swDid);

    CHECK_POINTER_RETURN_RET(stSysi4uclJniCb.doApkSelfUpgradeCb, ADM_ECODE_INVALID_POINTER);
    
    do
    {
        ECU_KEY_INFO_T stEcuKeyInfo;
        stEcuKeyInfo.uiRequestId = pstEcuIdenInfo->requestId;
        stEcuKeyInfo.uiSwDid = pstEcuIdenInfo->swDid;

        // 解压、解析升级文件
        admSysi4CbfGetStrrchr(pApkPackPath, '/', currentImageDir, ADM_COM_FILEPATH_LEN);
        log_i("currentImageDir=%s", currentImageDir);
        //拼接一个解压目录
        DCHAR unzipDir[ADM_COM_FILEPATH_LEN] = {0};
        (void)adm_snprintf(unzipDir, sizeof(unzipDir), "%s/0x%04x",
                            currentImageDir, pstEcuIdenInfo->requestId);
        log_i("unzipDir=%s", unzipDir);

        DBOOL isExist = FALSE;
        isExist = adm_com_isDirExist(unzipDir);
        if(TRUE == isExist)
        {
            log_i("remove dir[%s]", unzipDir);
            (void)adm_com_rmDirAndFiles(unzipDir);
        }

        log_i("create dir[%s]", unzipDir);
        iRet = adm_com_createDir(unzipDir);
        if(ADM_IS_ERR(iRet))
        {
            log_e("creat dir[%s] failed", unzipDir);
            break;
        }

        //解压到指定目录
        iRet = adm_com_unZipFile(pApkPackPath, unzipDir);
        if(ADM_IS_ERR(iRet))
        {
            log_e("unZipFile[%s] to dir[%s] failed,iRet[%d]", pApkPackPath,
                    unzipDir, iRet);
            break;
        }

        //解析packageConfig.json
        iRet = admSysi4CbfParsePkgCfg(&stEcuKeyInfo, unzipDir, ADM_PKG_CFG_NAME);
        if (ADM_ECODE_SUCCESSED != iRet)
        {
            if (ADM_ECODE_PRASE_RSACBF_FAILED == iRet)
            {
                /** 如果非rsa+cbf格式，直接就是原包进行升级 */
                bIsCbfFmt = DFALSE;
                pstImage = (ADM_SYSI4_IMAGE_CFG_T*)adm_malloc(sizeof(ADM_SYSI4_IMAGE_CFG_T));
                iRet = sysi4ucl_parsePkgCfg(unzipDir, ADM_PKG_CFG_NAME, pstImage, &eFileType);
                if (ADM_IS_SUCCESS(iRet))
                {
                    iRet = sysi4ucl_verifyFile(pstImage);
                    if (ADM_ECODE_SUCCESSED != iRet)
                    {
                        log_e("verify file failed");
                        break;
                    }
                }
            }
            else 
            {
                log_e("parse packageConfig.json failed.retValue[%d]", iRet);
                break;
            }
        }
        else 
        {
            //解压后验证原cbf和rsa hash
            iRet = admSysi4CbfVerifyFile(&stEcuKeyInfo);
            if (ADM_ECODE_SUCCESSED != iRet)
            {
                log_e("cbf verify file failed");
                break;
            }
        }

        ///!< 升级中的标识置为 TRUE
        g_bIsApkUpgrading = DTRUE;
        log_i("upgrade path[%s]", pstImage->pImgPath);
        log_i("requestId[%u] swDid[%u]", pstEcuIdenInfo->requestId, pstEcuIdenInfo->swDid);
        ///!< HMI 和 UCM 的升级都统一由此接口负责升级
        iRet = stSysi4uclJniCb.doApkSelfUpgradeCb(pstImage->pImgPath, pApkVersion, pstEcuIdenInfo->requestId);
        admSysi4CbfClear(&stEcuKeyInfo);
    } while (FALSE);

    *errorCode = iRet;

    log_i("adm_sysi4ucl_doApkSelfUpgrade end");
    return iRet;
}

static D32S uclIccUnZipFile(DCHAR *srcFilePath, DCHAR *dstUnzipDir)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    DCHAR szCmd[256] = {0};
    
    if (ADM_IS_NULL(srcFilePath) || ADM_IS_NULL(dstUnzipDir))
    {
        log_e( "Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }

    (void)adm_snprintf(szCmd, sizeof(szCmd), "unzip %s -d %s 1>/dev/null 2>&1 ", srcFilePath, dstUnzipDir);
    log_i("===> going to run cmd[%s]", szCmd);
    (void)system(szCmd);

    return ADM_ECODE_SUCCESSED;
}

/*!
 *******************************************************************************
 * 解压文件.
 * 
 * \note
 *
 * \param[in]   srcFilePath  待解压文件路径
 * \param[in]   dstUnzipDir  解压到文件目录
 * \retval      D32S
 *******************************************************************************
 */

static void* doIccUpgradeAsync(void* argv)
{
    ADM_SYS4UCL_ICC_UPGPKGINFO_T* pstIccUpgradeInfo = (ADM_SYS4UCL_ICC_UPGPKGINFO_T*)argv;
    D32S iRet = ADM_ECODE_FAILED;
    DBOOL bIsCbfFmt = DTRUE;        ///< 默认升级包为cbf格式
    ADM_SYSI4_IMAGE_CFG_T* pstImage = NULL;
    ADM_SYSI4ALL_ECU_IDEN_T stEcuId = {0};
    ADM_SYSI4UCL_IMAGE_TYPE_E eFileType = ADM_SYSI4UCL_IMAGE_TYPE_OTHER;
    ADM_SYSI4UCL_UPGRADE_INFO_ARRAY_T* upgradeInfoList = NULL;
    D32S installType = 0;

    if(ADM_IS_NULL(pstIccUpgradeInfo) || ADM_IS_NULL(pstIccUpgradeInfo->upgradeInfoList))
    {
        log_e("input param is NULL");
        b_isDoTrigger = DFALSE;
        (DVOID)adm_sysi4ucl_upgradeIccResultNotify(2, iRet);
        return DNULL;
    }

    if (DNULL == stSysi4uclJniCb.doUpgradeTriggerCb)
    {
        log_e("stSysi4uclJniCb.doUpgradeTriggerCb is nil");
        b_isDoTrigger = DFALSE;
        (DVOID)adm_sysi4ucl_upgradeIccResultNotify(2, iRet);
        return DNULL;
    }

    upgradeInfoList = pstIccUpgradeInfo->upgradeInfoList;
    installType = pstIccUpgradeInfo->installType;

    for(D16U index = 0; index < upgradeInfoList->upgradeInfoArrayLength; ++index)
    {
        ADM_SYSI4UCL_UPGRADE_INFO_T* pstUpInfo = upgradeInfoList->upgradeInfoArray[index];
        CHECK_POINTER_CONTINUE(pstUpInfo);
        CHECK_POINTER_CONTINUE(pstUpInfo->imageInfoList);

        ECU_KEY_INFO_T stEcuKeyInfo;
        stEcuKeyInfo.uiRequestId = pstUpInfo->pstEcuIdenInfo->requestId;
        stEcuKeyInfo.uiSwDid = pstUpInfo->pstEcuIdenInfo->swDid;

        stEcuId.requestId = pstUpInfo->pstEcuIdenInfo->requestId;
        stEcuId.responseId = pstUpInfo->pstEcuIdenInfo->responseId;
        stEcuId.functionId = pstUpInfo->pstEcuIdenInfo->functionId;
        stEcuId.swDid = pstUpInfo->pstEcuIdenInfo->swDid;

        //只有 requestId生效 见 ADM_CUSTOM_ECU_E
        for(D16U imageIndex = 0; imageIndex < pstUpInfo->imageInfoList->imageInfoArrayLength; ++imageIndex)
        {
            ADM_SYSI4UCL_IMAGE_DETAIL_INFO_T * pstImageInfo = pstUpInfo->imageInfoList->imageInfoArray[imageIndex];
            CHECK_POINTER_CONTINUE(pstImageInfo);
            CHECK_POINTER_CONTINUE(pstImageInfo->imagePath);
            //获取升级包目录
            DCHAR currentImageDir[ADM_COM_FILEPATH_LEN] = {0};
            admSysi4CbfGetStrrchr(pstImageInfo->imagePath, '/', currentImageDir, ADM_COM_FILEPATH_LEN);

            log_i("currentImageDir=%s", currentImageDir);
            //拼接一个解压目录
            DCHAR unzipDir[ADM_COM_FILEPATH_LEN] = {0};
            (void)adm_snprintf(unzipDir, sizeof(unzipDir), "%s/0x%04x_%u",
                               currentImageDir, pstUpInfo->pstEcuIdenInfo->requestId, imageIndex);
            log_i("unzipDir=%s", unzipDir);

            DBOOL isExist = FALSE;
            isExist = adm_com_isDirExist(unzipDir);

            /**文件存在且是MCU升级时,不处理*/
            if(TRUE == isExist && ICC_MCU == installType)
            {
                log_i("unzip already success!");
            }
            else
            {
                (void)adm_com_rmFile(unzipDir);
                /**删除文件*/

                iRet = adm_com_createDir(unzipDir);
                if(ADM_IS_ERR(iRet))
                {
                    log_e("creat dir[%s] failed", unzipDir);
                    break;
                }

                //解压到指定目录
                log_d("unZipFile[%s] to dir[%s]", pstImageInfo->imagePath, unzipDir);
                log_d("[ABUP-Procedure] unZipFile[%s] Start.", pstImageInfo->imagePath);
                if(pstImageInfo->makePackageType==FILE_MAKE_PKG_TYPE)
                {
                    //TODO 在指定目录拆分文件---拼接文件拆分
                   iRet=uclSplitFile(pstImageInfo->imagePath, unzipDir,pstImageInfo->fileMetaNum,pstImageInfo->pstFileMetaList); 
                   if(ADM_IS_ERR(iRet))
                   {
                       log_e("uclSplitFile[%s] to dir[%s] failed,iRet[%d]", pstImageInfo->imagePath,
                           unzipDir, iRet);
                       break;
                   }
                }
                else
                {
                    iRet = uclIccUnZipFile(pstImageInfo->imagePath, unzipDir);
                    log_d("[ABUP-Procedure] unZipFile[%s] End. [iRet = %d]", pstImageInfo->imagePath, iRet);
                    if(ADM_IS_ERR(iRet))
                    {
                        log_e("unZipFile[%s] to dir[%s] failed,iRet[%d]", pstImageInfo->imagePath,
                            unzipDir, iRet);
                        break;
                    }
                }
                log_d("unZipFile success");
            }
            
            //解析packageConfig.json
            iRet = admSysi4CbfParsePkgCfg(&stEcuKeyInfo, unzipDir, ADM_PKG_CFG_NAME);
            if (ADM_ECODE_SUCCESSED != iRet)
            {
                if (ADM_ECODE_PRASE_RSACBF_FAILED == iRet)
                {
                    /** 如果非rsa+cbf格式，直接就是原包进行升级 */
                    bIsCbfFmt = DFALSE;
                    pstImage = (ADM_SYSI4_IMAGE_CFG_T*)adm_malloc(sizeof(ADM_SYSI4_IMAGE_CFG_T));
                    iRet = sysi4ucl_parsePkgCfg(unzipDir, ADM_PKG_CFG_NAME, pstImage, &eFileType);
                    if (ADM_IS_SUCCESS(iRet))
                    {
                        /** 升级类型为MCU时，不再进行文件校验，直接进行升级 */
                        if (ICC_MCU == installType)
                        {
                            log_d("MCU installType, skip verify file");
                        }
                        else
                        {
                            log_d("[ABUP-Procedure] verify file[%s] Start.", pstImage->pImgPath);
                            iRet = sysi4ucl_verifyFile(pstImage);
                            if (ADM_ECODE_SUCCESSED != iRet)
                            {
                                log_d("[ABUP-Procedure] verify file[%s] End. [iRet = %d]", pstImage->pImgPath, iRet);
                                break;
                            }

                            log_d("[ABUP-Procedure] verify file[%s] End. [iRet = %d]", pstImage->pImgPath, iRet);
                        }
                    }
                }
                else 
                {
                    log_e("parse packageConfig.json failed.retValue[%d]", iRet);
                    break;
                }
            }
            else 
            {
                if( pstImageInfo->makePackageType!=FILE_MAKE_PKG_TYPE)
                {
                    //解压后验证原cbf和rsa hash
                    iRet = admSysi4CbfVerifyFile(&stEcuKeyInfo);
                    if (ADM_ECODE_SUCCESSED != iRet)
                    {
                        log_e("cbf verify file failed");
                        break;
                    }
                }
            }
        }

        if (ADM_IS_SUCCESS(iRet))
        {
            if (DTRUE == bIsCbfFmt)
            {
                /** cbf格式包处理 */
                iRet = sysi4ucl_handleCbfUpgrade(installType, &stEcuKeyInfo);
            }
            else 
            {
                /** 非cbf格式包处理 */
                ADM_SYSI4UCL_IMAGE_INFO_T stImageInfo = {};
                log_i("upgrade path[%s]", pstImage->pImgPath);
                log_i("requestId[%u] swDid[%u]", stEcuId.requestId, stEcuId.swDid);
                /** 非cbf格式包处理 */
                stImageInfo.eType = eFileType;
                stImageInfo.imagePath = (int8_t*)pstImage->pImgPath;

                DCHAR* pJsonStr = sysi4ucl_handleImageInfo(&stImageInfo, 1);
                b_isDoTrigger = DFALSE;
                // 2:MCU ;7:SOC
                log_d("[ABUP-Procedure] upgrade trigger [installType = %d] Start.", installType);
                iRet = sysi4DoUpgradeTrigger(installType, DFALSE, pJsonStr);
                log_d("[ABUP-Procedure] upgrade trigger [installType = %d] End. [iRet = %d]", installType, iRet);

                adm_free(pJsonStr);
            }
        }
    }

    if (ADM_IS_ERR(iRet))
    {
        (DVOID)adm_sysi4ucl_upgradeIccResultNotify(installType, iRet);
    }

    //free
    for(D16U index = 0; index < upgradeInfoList->upgradeInfoArrayLength; ++index)
    {
        ADM_SYSI4UCL_UPGRADE_INFO_T* pstUpInfo = upgradeInfoList->upgradeInfoArray[index];
        CHECK_POINTER_CONTINUE(pstUpInfo);

        ECU_KEY_INFO_T stEcuKeyInfo;

        if(NULL != pstUpInfo->pstEcuIdenInfo)
        {
            stEcuKeyInfo.uiRequestId = pstUpInfo->pstEcuIdenInfo->requestId;
            stEcuKeyInfo.uiSwDid = pstUpInfo->pstEcuIdenInfo->swDid;
        }

        admSysi4CbfClear(&stEcuKeyInfo);
    }
    adm_free(pstIccUpgradeInfo->upgradeInfoList);
    adm_free(pstIccUpgradeInfo);

    b_isDoTrigger = DFALSE;
    return DNULL;
}

D32S adm_sysi4ucl_doIccUpgradeTrigger(D32S installType, D32S bIsRollback, 
                const ADM_SYSI4UCL_UPGRADE_INFO_ARRAY_T* upgradeInfoList, D32S* errorCode, void* priv)
{
    D32S iRet = ADM_ECODE_FAILED;
    ADM_SYS4UCL_ICC_UPGPKGINFO_T* pstIccUpgradeInfo = DNULL;
    pthread_t tid = 0;

    if (DTRUE == b_isDoTrigger)
    {
        log_e("Async upgrading now, do not trigger again");
        return ADM_ECODE_SUCCESSED;
    }

    if(ADM_IS_NULL(upgradeInfoList) || ADM_IS_NULL(errorCode))
    {
        log_e("input param is NULL");
        return ADM_ECODE_INVALID_PARAM;
    }

    CHECK_POINTER_RETURN_RET(stSysi4uclJniCb.doUpgradeTriggerCb, ADM_ECODE_INVALID_POINTER);

    pstIccUpgradeInfo = (ADM_SYS4UCL_ICC_UPGPKGINFO_T*)adm_malloc(sizeof(ADM_SYS4UCL_ICC_UPGPKGINFO_T));
    if (ADM_IS_NULL(pstIccUpgradeInfo))
    {
        log_e("pstIccUpgradeInfo is NULL");
        return ADM_ECODE_INVALID_PARAM;
    }

    pstIccUpgradeInfo->installType = installType;
    pstIccUpgradeInfo->upgradeInfoList = (ADM_SYSI4UCL_UPGRADE_INFO_ARRAY_T*)adm_malloc(sizeof(ADM_SYSI4UCL_UPGRADE_INFO_ARRAY_T));
    iRet = sysi4ucl_deepCloneUpgradeInfo(upgradeInfoList, pstIccUpgradeInfo->upgradeInfoList);
    if (ADM_IS_ERR(iRet))
    {
        log_e("sysi4ucl_deepCloneUpgradeInfo failed, [err:%d]", iRet);
    }
    else
    {
        if(0 != pthread_create(&tid, NULL, doIccUpgradeAsync, (void*)pstIccUpgradeInfo))
        {
            log_e("pthread_create tid:%d failed", tid);
            iRet = ADM_ECODE_PTHREAD_CREATE_FAILED;
        }
        else
        {
            b_isDoTrigger = DTRUE;
            (void)usleep(100);
            pthread_detach(tid);
        }
    }

    *errorCode = iRet;

    return iRet;
}

D32S adm_sysi4ucl_queryApkSelfUpgradeProgress(const ADM_SYSI4ALL_ECU_IDEN_T* pstEcuIdenInfo, D16U* progress,  D32S* errorCode, void* priv)
{
    D32S iRet = ADM_ECODE_FAILED;
    ADM_SYSI4ALL_ECU_IDEN_T stOemEcuId = {};
    static D32S s_apkUpgradeProgress_count = 0;     ///!< 默认APK的升级进度抛送5次


    if(ADM_IS_NULL(pstEcuIdenInfo) || ADM_IS_NULL(progress) || ADM_IS_NULL(errorCode))
    {
        log_e("input param is NULL");
        return ADM_ECODE_INVALID_PARAM;
    }

    // CHECK_POINTER_RETURN_RET(stSysi4uclJniCb.getUpgradeProgressCb, ADM_ECODE_INVALID_POINTER);

    stOemEcuId.requestId = pstEcuIdenInfo->requestId;
    stOemEcuId.responseId = pstEcuIdenInfo->responseId;
    stOemEcuId.functionId = pstEcuIdenInfo->functionId;
    stOemEcuId.swDid = pstEcuIdenInfo->swDid;

    D32S iPkgprogress = 0;
    
    ///!< APK 升级的进度抛送，按照最大抛送为5次，20 - 100
    // iRet = stSysi4uclJniCb.getUpgradeProgressCb(stOemEcuId.requestId, &iPkgprogress);
    s_apkUpgradeProgress_count++;
    if (5 < s_apkUpgradeProgress_count)
    {
        s_apkUpgradeProgress_count = 0;
    }
    iPkgprogress = s_apkUpgradeProgress_count * 20;
    iRet = ADM_ECODE_SUCCESSED;
    log_i("APK requestID:%d upgrade progress:%d", stOemEcuId.requestId, iPkgprogress);


    //进度值校验0-100
    if(100 < iPkgprogress)
    {
        log_e("iPkgprogress[%d] is invalid", iPkgprogress);
    }

    *progress = (D16U)iPkgprogress;

    *errorCode = iRet;

    return iRet;
}

D32S adm_sysi4ucl_queryApkSelfUpgraderesult(const ADM_SYSI4ALL_ECU_IDEN_T* pstEcuIdenInfo,  ADM_SYSI4UCL_UPGRADE_RESULT_INFO_T** pstUpgradeResultInfo, D32S* errorCode, void* priv)
{
    D32S iRet = ADM_ECODE_FAILED;
    ADM_SYSI4ALL_ECU_IDEN_T stOemEcuId = {};

    if (ADM_IS_NULL(pstEcuIdenInfo) || ADM_IS_NULL(pstUpgradeResultInfo) || ADM_IS_NULL(errorCode))
    {
        log_e("Input param is nil");
        return ADM_ECODE_INVALID_PARAM;
    }

    stOemEcuId.requestId = pstEcuIdenInfo->requestId;
    stOemEcuId.responseId = pstEcuIdenInfo->responseId;
    stOemEcuId.functionId = pstEcuIdenInfo->functionId;
    stOemEcuId.swDid = pstEcuIdenInfo->swDid;

    if (DTRUE == g_bIsApkUpgrading)
    {
        *pstUpgradeResultInfo = (ADM_SYSI4UCL_UPGRADE_RESULT_INFO_T*)adm_malloc(sizeof(ADM_SYSI4UCL_UPGRADE_RESULT_INFO_T));

        if(ADM_IS_NULL(*pstUpgradeResultInfo))
        {
            log_e("adm_malloc failed");
            iRet = ADM_ECODE_NO_MEMORY;
            return iRet;
        }

        (*pstUpgradeResultInfo)->result = 3;        ///!< 标识升级中
        (*pstUpgradeResultInfo)->expectVal = DNULL;
        (*pstUpgradeResultInfo)->realVal = DNULL;
        iRet = ADM_ECODE_SUCCESSED;
    }

    *errorCode = iRet;

    return iRet;
}

D32S adm_sysi4uclApp_handlerInit(void* priv)
{
    D32S iRet = ADM_ECODE_FAILED;
    ADM_SYSI4UCL_INIT_INFO_T stUclFn = {0};
    log_i("adm_sysi4uclApp_handlerInit begin");

    stUclFn.readPartInfoCallback = adm_sysi4ucl_readPartInfo;
    stUclFn.readSnCallback = adm_sysi4ucl_readSnInfo;
    stUclFn.doUpgradeCallback = adm_sysi4ucl_doUpgradeTrigger;
    stUclFn.doUpgradeIccCallback = adm_sysi4ucl_doIccUpgradeTrigger;
    stUclFn.queryUpgradeProgressCallback = adm_sysi4ucl_queryUpgradeProgress;
    stUclFn.queryUpgradeResultCallback = adm_sysi4ucl_queryUpgraderesult;

    stUclFn.requestDecryptCallback = adm_sysi4ucl_requestDecryptPackage;
    stUclFn.getDecryptProgressCallback = adm_sysi4ucl_getDecryptProgress;
    stUclFn.getDecryptResultCallback = adm_sysi4ucl_getDecryptResult;
    stUclFn.requestVerifyHashCallback = adm_sysi4ucl_requestVerifyHash;
    stUclFn.verifySpaceIsEnoughCallback = adm_sysi4ucl_verifySpaceIsEnough;
    stUclFn.requestVerifySignCallback = adm_sysi4ucl_request_Verify_Sign;
    stUclFn.requestSetLogLevelCallback = adm_sysi4ucl_set_log_level;
    stUclFn.getSdkVersionCallback = adm_sysi4ucl_get_sdkVersion;
    stUclFn.readVehicleStatue = adm_sysi4ucl_read_VehicleStatue;
    stUclFn.setVehicleStatue = adm_sysi4ucl_set_VehicleStatue;
    stUclFn.getApkVersionCallback = adm_sysi4ucl_getApkVersion;
    stUclFn.doApkSelfUpgradeCallback = adm_sysi4ucl_doApkSelfUpgrade;
    stUclFn.queryApkSelfUpgradeProgressCallback = adm_sysi4ucl_queryApkSelfUpgradeProgress;
    stUclFn.queryApkSelfUpgradeResultCallback = adm_sysi4ucl_queryApkSelfUpgraderesult;
	stUclFn.requestSyncDecryptCallback = adm_sysi4ucl_requestSyncDecryptPackage;
    stUclFn.updateAndRebootCallback = sysi4ucl_UpdateAndReboot;
	stUclFn.requestRmFileCallback = adm_sysi4ucl_requestRmFile;
	
    do
    {
        iRet = adm_sysi4ucl_init_handle(&stUclFn, priv);
        if(ADM_ECODE_SUCCESSED != iRet)
        {
            log_e("adm_sysi4ucl_init_handle failed");
            break;
        }

        //cbf parse
        iRet = admSysi4CbfInit();
        if(ADM_ECODE_SUCCESSED != iRet)
        {
            log_e("admSysi4CbfInit failed");
            break;
        }

        (DVOID)sysi4uclGetControlActiveInfo();

        iRet = sysi4uclGetUpgradeInfo();
        if (ADM_IS_SUCCESS(iRet))
        {
            /** 处理重启后未通知出去的升级结果 */
            if (0 > g_stUpgradeInfo.iResult || UPGRADE_SUCCEED == g_stUpgradeInfo.iResult )
            {
                pthread_t threadId = 0;
                if (0 == pthread_create(&threadId, NULL, sysi4uclNotifyResult, NULL)) 
                {
                    log_i("Init module ,pthread_create sysi4uclNotifyResult successed");
                    (void)pthread_detach(threadId);
                } 
                else 
                {
                    log_e("pthread_create failed");
                }
            }
        }
        else 
        {
            iRet = ADM_ECODE_SUCCESSED;
        }

    }
    while(TRUE == FALSE);
    log_i("adm_sysi4uclApp_handlerInit end");

    return iRet;
}

D32S adm_sysi4uclApp_handlerDeinit(void)
{
    adm_sysi4ucl_deinit_handle();

    (DVOID)admSysi4CbfDeinit();

    return 0;
}


D32S adm_sysi4ucl_jni_Init(ADM_SYSI4UCL_JNI_CALLBACK_T* pstJniCb)
{
    D32S iRet = ADM_ECODE_FAILED;

    CHECK_POINTER_RETURN_RET(pstJniCb, ADM_ECODE_INVALID_PARAM);
    log_i("adm_sysi4ucl_jni_Init begin");

    stSysi4uclJniCb.doUpgradeTriggerCb = pstJniCb->doUpgradeTriggerCb;
    stSysi4uclJniCb.getUpgradeProgressCb = pstJniCb->getUpgradeProgressCb;
    stSysi4uclJniCb.getUpgradeResultCb = pstJniCb->getUpgradeResultCb;
    stSysi4uclJniCb.updateAndRebootCb = pstJniCb->updateAndRebootCb;
    stSysi4uclJniCb.upgradeWillStartCb = pstJniCb->upgradeWillStartCb;
    stSysi4uclJniCb.getApkVersionCb = pstJniCb->getApkVersionCb;
    stSysi4uclJniCb.doApkSelfUpgradeCb = pstJniCb->doApkSelfUpgradeCb;
    stSysi4uclJniCb.prereductionCb = pstJniCb->prereductionCb;
    stSysi4uclJniCb.notifyMcuAndScreenUpgradeCb = pstJniCb->notifyMcuAndScreenUpgradeCb;

    const DCHAR* sysi_lvm_modules = adm_cfg_iniParserGetString("SYSI:sysi_lvm_modules", "");

    if(strstr(sysi_lvm_modules, "ucl") != DNULL)
    {
        log_d("adm_sysi4ucl_loadFunctions load lvm func");
        ADM_SYSI4UCL_INIT_INFO_T stUclFn = {0};

        stUclFn.readPartInfoCallback = adm_sysi4ucl_readPartInfo_lvm;
        stUclFn.readSnCallback = adm_sysi4ucl_readSnInfo_lvm;
        stUclFn.doUpgradeCallback = adm_sysi4ucl_doUpgradeTrigger_lvm;
        stUclFn.queryUpgradeProgressCallback = adm_sysi4ucl_queryUpgradeProgress_lvm;
        stUclFn.queryUpgradeResultCallback = adm_sysi4ucl_queryUpgraderesult_lvm;
        //Using the decryption operation
        stUclFn.requestDecryptCallback = adm_sysi4ucl_requestDecryptPackage;
        stUclFn.getDecryptProgressCallback = adm_sysi4ucl_getDecryptProgress;
        stUclFn.getDecryptResultCallback = adm_sysi4ucl_getDecryptResult;

        stUclFn.requestVerifyHashCallback = adm_sysi4ucl_requestVerifyHash_lvm;
        stUclFn.verifySpaceIsEnoughCallback = adm_sysi4ucl_verifySpaceIsEnough_lvm;
        // stUclFn.requestVerifySignCallback = adm_sysi4ucl_request_Verify_Sign_lvm;
        stUclFn.requestVerifySignCallback = adm_sysi4ucl_request_Verify_Sign;
        stUclFn.requestSetLogLevelCallback = adm_sysi4ucl_set_log_level_lvm;
        stUclFn.getSdkVersionCallback = adm_sysi4ucl_get_sdkVersion_lvm;
        stUclFn.getApkVersionCallback = adm_sysi4ucl_getApkVersion;
        stUclFn.doApkSelfUpgradeCallback = adm_sysi4ucl_doApkSelfUpgrade;
        stUclFn.queryApkSelfUpgradeProgressCallback = adm_sysi4ucl_queryApkSelfUpgradeProgress;
        stUclFn.queryApkSelfUpgradeResultCallback = adm_sysi4ucl_queryApkSelfUpgraderesult;
        
        stUclFn.requestRmFileCallback = adm_sysi4ucl_requestRmFile;
        adm_sysi4ucl_handlerInit_lvm(DNULL);

        void* priv = DNULL;

        if(ADM_ECODE_SUCCESSED !=  adm_sysi4ucl_init_handle(&stUclFn, priv))
        {
            log_e("adm_sysi4ucl_init_handle return failed");
            return ADM_ECODE_FAILED;
        }

        //cbf parse
        iRet = admSysi4CbfInit();
        if(ADM_ECODE_SUCCESSED != iRet)
        {
            log_e("admSysi4CbfInit failed");
            return iRet;
        }
    }
    else 
    {
        iRet = adm_sysi4uclApp_handlerInit(NULL);
    }

    if(ADM_IS_ERR(iRet))
    {
        log_e("adm_sysi4uclApp_handlerInit failed");
        return iRet;
    }

    return iRet;
}


