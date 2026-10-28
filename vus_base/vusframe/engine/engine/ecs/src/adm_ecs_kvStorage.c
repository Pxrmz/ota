/**
 * @file    adm_ecs_kvStorage.c
 * @brief   ecs module key value storage  
 * @details ecs 模块 kv 存储
 * @author  wangyanpeng
 * @version v0.1.0
 * @date    2020-11-06
 * @copyright www.abupdate.com
 */
#include <stddef.h>
#include <time.h>
#include "adm_string.h"
#include "adm_memory.h"
#include "adm_ecs_log.h"
#include "adm_com_lib.h"
#include "adm_com_errorCode.h"
#include "adm_com_modDefine.h"
#include "adm_ecs_kvStorage.h"
#include "adm_engine_common.h"
#include "adm_com_kvstorage.h"
#include "adm_com_parseFileObject.h"
#include "adm_ecs_parsePolicy.h"
#include "adm_configParse.h"

/*!
 *******************************************************************************
 * 反序列化json字符串到FileObject结构信息.
 *
 * \note
 * 
 * \param[in]   pcFileObjJsonChar  文件对象的json字符串
 * \param[out]  pstFileObj         文件对象结构信息
 * \retval      D32S
 *******************************************************************************
 */
static D32S __adm_ecs_deJsonChar2FileObjectInfo
(
    DCHAR                *pcFileObjJsonChar,
    ADM_COM_FILEOBJECT_T *pstFileObj
)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    json_object *jsonRoot = NULL;

    if (ADM_IS_NULL(pcFileObjJsonChar) || ADM_IS_NULL(pstFileObj))
    {
        log_e("Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }
    
    jsonRoot = json_tokener_parse(pcFileObjJsonChar);
    if (ADM_IS_NULL(jsonRoot)) 
    {
        log_e("Failed string to json, string[%s]", pcFileObjJsonChar);
        return ADM_ECODE_STRING_TO_JSON_FAILED;
    }

    //!< 解析json中businessScript节点信息
    iRet = adm_com_parseJsonFileObject(jsonRoot, pstFileObj);
    if (ADM_IS_ERR(iRet))
    {
        (void)json_object_put(jsonRoot);
        log_e("Failed to parse file object", iRet);
        return iRet;
    }
    (void)json_object_put(jsonRoot);

    return iRet;
}

/*!
 *******************************************************************************
 * 从kv数据库中获取ota配置文件对象信息.
 *
 * \note
 * 
 * \param[in]   pstOtaCfgKvHandle 模块kv存储句柄
 * \param[out]  pstRsCfgFileObj   配置文件对象结构信息
 * \retval      D32S
 *******************************************************************************
 */
static D32S __adm_ecs_getOtaCfgFileObjFromKV
(
    KvStorage_t          *pstOtaCfgKvHandle,
    ADM_COM_FILEOBJECT_T *pstRsCfgFileObj
)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    DBOOL bIsOk = FALSE;
    size_t valLength = 0;
    DCHAR *pcRsCfgFileObj = NULL;
	ADM_KV_STORAGE_VALUE_TYPE_E eValType;

    if (ADM_IS_NULL(pstOtaCfgKvHandle) || ADM_IS_NULL(pstRsCfgFileObj))
    {
        log_e("Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }

    //!< stRsCfgFileObj 
    bIsOk = adm_com_kvstorage_get_string(pstOtaCfgKvHandle, ADM_COM_OTACFG_KVSTORAGE_CFGFILEOBJ_KEY, 
                            &pcRsCfgFileObj, &valLength, &eValType);
    if (!bIsOk)
    {
        log_e("Failed to get adm_com_kvstorage_get_string, key[%s]", ADM_COM_OTACFG_KVSTORAGE_CFGFILEOBJ_KEY);
        return ADM_ECODE_KV_GET_STRING_FAILED;
    }

    iRet = __adm_ecs_deJsonChar2FileObjectInfo(pcRsCfgFileObj, pstRsCfgFileObj);
    if (ADM_IS_ERR(iRet))
    {
        adm_free(pcRsCfgFileObj);
        log_e("Failed to de ota config file object info from kv, err[%d]", iRet);
        return iRet;
    }
    adm_free(pcRsCfgFileObj);

    return iRet;
}

/*!
 *******************************************************************************
 * 从kv数据库中获取ota配置信息.
 *
 * \note
 * 
 * \param[in]   pstOtaCfgKvHandle 模块kv存储句柄
 * \param[out]  pstOtaCfgInfo     ota配置信息
 * \retval      D32S
 *******************************************************************************
 */
static D32S __adm_ecs_getOtaCfgInfoFromKV
(
    KvStorage_t            *pstOtaCfgKvHandle,
    ADM_ECS_RSCONFIGINFO_T *pstOtaCfgInfo
)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    DBOOL bIsOk = FALSE;
    size_t valLength = 0;
    size_t uiSecretLength = 0;
    DCHAR* pcRsCfgFileInfo = NULL;
    ADM_KV_STORAGE_VALUE_TYPE_E eValType;
    DCHAR* pcConfFileDir = NULL;

    if (ADM_IS_NULL(pstOtaCfgKvHandle) || ADM_IS_NULL(pstOtaCfgInfo))
    {
        log_e("Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }

    //!< 设置配置文件解压目录
    pcConfFileDir = adm_ecs_getDownConfigFileUnzipDir();
    if (ADM_IS_NULL(pcConfFileDir))
    {
        iRet = ADM_ECODE_CORESERVICE_GETRUNDIR_FAILED;
        log_e("Failed to get coreService config package save dir, err[%d]", iRet);
        return iRet;
    }
    log_d("Config file unzip dir[%s].", pcConfFileDir);

    /** 获取对称密钥 */
    iRet = adm_ecs_getSecretFromKV(&(pstOtaCfgInfo->secret), &uiSecretLength);
    if (ADM_IS_ERR(iRet))
    {
        log_e("Call adm_ecs_getSecretFromKV error.iRet[%d]", iRet);
        return ADM_ECODE_KV_GET_STRING_FAILED;
    }

    //!< ADM_COM_OTACFG_KVSTORAGE_CFGPARTINFO_KEY 
    bIsOk = adm_com_kvstorage_get_string(pstOtaCfgKvHandle, ADM_COM_OTACFG_KVSTORAGE_CFGPARTINFO_KEY, 
                            &pcRsCfgFileInfo, &valLength, &eValType);
    if (!bIsOk)
    {
        adm_free(pcConfFileDir);
        log_e("Failed to get adm_com_kvstorage_get_string, key[%s]", ADM_COM_OTACFG_KVSTORAGE_CFGPARTINFO_KEY);
        return ADM_ECODE_KV_GET_STRING_FAILED;
    }


    iRet = adm_ecs_parsePartConfigInfo(pstOtaCfgInfo->secret, pcRsCfgFileInfo, &pstOtaCfgInfo->stPartCfg);
    if (ADM_IS_ERR(iRet))
    {
        adm_free(pcConfFileDir);
        adm_free(pcRsCfgFileInfo);
        log_e("Failed to parse general config file");
        return iRet;
    }
    adm_free(pcRsCfgFileInfo);
    log_d("parse part config file ok.");

    //!< ADM_COM_OTACFG_KVSTORAGE_CFGGENERALINFO_KEY 
    bIsOk = adm_com_kvstorage_get_string(pstOtaCfgKvHandle, ADM_COM_OTACFG_KVSTORAGE_CFGGENERALINFO_KEY, 
                            &pcRsCfgFileInfo, &valLength, &eValType);
    if (!bIsOk)
    {
        adm_free(pcConfFileDir);
        log_e("Failed to get adm_com_kvstorage_get_string, key[%s]", ADM_COM_OTACFG_KVSTORAGE_CFGGENERALINFO_KEY);
        return ADM_ECODE_KV_GET_STRING_FAILED;
    }

    iRet = adm_ecs_parseGeneralConfigInfo(pstOtaCfgInfo->secret, pcConfFileDir, pcRsCfgFileInfo, &pstOtaCfgInfo->stGeneralCfg);
    if (ADM_IS_ERR(iRet))
    {
        adm_free(pcConfFileDir);
        adm_free(pcRsCfgFileInfo);
        log_e("Failed to parse general config file");
        return iRet;
    }
    adm_free(pcConfFileDir);
    adm_free(pcRsCfgFileInfo);
    log_d("parse general config file ok.");

    return iRet;
}


#if 0
#endif

/*!
 *******************************************************************************
 * 序列化FileObject结构信息为json字符串.
 *
 * \note
 * 
 * \param[in]   pstFileObj         文件对象结构信息
 * \param[out]  pcFileObjJsonChar  文件对象的json字符串
 * \retval      D32S
 *******************************************************************************
 */
static D32S __adm_ecs_enFileObjectInfo2JsonChar
(
    ADM_COM_FILEOBJECT_T *pstFileObj,
    DCHAR                **ppcFileObjJsonChar
)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    json_object *pstFileObjJson = NULL;

    if (ADM_IS_NULL(pstFileObj) || ADM_IS_NULL(ppcFileObjJsonChar))
    {
        log_e("Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }

    do
    {        
        pstFileObjJson = json_object_new_object();
        if (ADM_IS_NULL(pstFileObjJson))
        {
            iRet = ADM_ECODE_NEW_JSON_OBJECT_FAILED;
            log_e("Faile to new json object, err[%d]", iRet);
            break;
        }

        //!< en fileObject 2 json
        iRet = adm_com_enFileObjectAdd2JsonObj(pstFileObj, pstFileObjJson);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to add file object to json object, err[%d]", iRet);
            break;
        }

        //!< json to string
        *ppcFileObjJsonChar = adm_strdup(json_object_to_json_string(pstFileObjJson));
        if (ADM_IS_NULL(*ppcFileObjJsonChar))
        {
            log_e("Failed to json to string");
            iRet = ADM_ECODE_JSON_TO_STRING_FAILED;
            break;
        }
    } while(FALSE);

    if (NULL != pstFileObjJson)
    {
        (void)json_object_put(pstFileObjJson);
    }

    return iRet;
}

/*!
 *******************************************************************************
 * 存储ota配置文件对象信息到kv数据库.
 *
 * \note
 * 
 * \param[in]  pstOtaCfgKvHandle 模块kv存储句柄
 * \param[in]  pstRsCfgFileObj   配置文件对象结构信息
 * \retval      D32S
 *******************************************************************************
 */
static D32S __adm_ecs_setOtaCfgFileObj2KV
(
    KvStorage_t          *pstOtaCfgKvHandle,
    ADM_COM_FILEOBJECT_T *pstRsCfgFileObj
)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    DBOOL bIsOk = FALSE;
    DCHAR *pcRsCfgFileObj = NULL;

    if (ADM_IS_NULL(pstOtaCfgKvHandle) || ADM_IS_NULL(pstRsCfgFileObj))
    {
        log_e("Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }

    iRet = __adm_ecs_enFileObjectInfo2JsonChar(pstRsCfgFileObj, &pcRsCfgFileObj);
    if (ADM_IS_ERR(iRet))
    {
        log_e("Failed to en ota config file object info to char, err[%d]", iRet);
        return iRet;
    }
    log_i("===> rsCfgFileObj[%s]", pcRsCfgFileObj);

    //!< stRsCfgFileObj 
    bIsOk = adm_com_kvstorage_set_string(pstOtaCfgKvHandle, ADM_COM_OTACFG_KVSTORAGE_CFGFILEOBJ_KEY, 
                                 pcRsCfgFileObj,ADM_KV_STORAGE_VALUE_TYPE_STRING);
    if (!bIsOk)
    {
        adm_free(pcRsCfgFileObj);
        log_e("Failed to set adm_com_kvstorage_set_string, key[%s]", ADM_COM_OTACFG_KVSTORAGE_CFGFILEOBJ_KEY);
        return ADM_ECODE_KV_SET_STRING_FAILED;
    }
    adm_free(pcRsCfgFileObj);

    return iRet;
}



/*!
 *******************************************************************************
 * 存储ota配置文件信息到kv数据库.
 *
 * \note
 * 
 * \param[in]  pstOtaCfgKvHandle 模块kv存储句柄
 * \param[in]  pstOtaCfgInfo     ota配置信息
 * \retval      D32S
 *******************************************************************************
 */
static D32S __adm_ecs_setOtaCfgFileInfo2KV
(
    KvStorage_t            *pstOtaCfgKvHandle,
    ADM_ECS_RSCONFIGINFO_T *pstOtaCfgInfo
)
{
    D32S   iRet = ADM_ECODE_SUCCESSED;
    DBOOL  bIsOk = FALSE;
    DBOOL  isExist = FALSE;
    DCHAR *pcFileData = NULL;
    DCHAR  cfgFilePath[ADM_COM_FILEPATH_LEN] = {0};
    DCHAR *pcDownConfFileDir = NULL;

    if (ADM_IS_NULL(pstOtaCfgKvHandle))
    {
        log_e("Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }

    //!< 这里应为有配置文件了，所以可以不存储到kv
    ADM_UNUSED_ARG(pstOtaCfgInfo);

    //!< 设置配置文件解压目录
    pcDownConfFileDir = adm_ecs_getDownConfigFileUnzipDir();
    if (ADM_IS_NULL(pcDownConfFileDir))
    {
        iRet = ADM_ECODE_CORESERVICE_GETRUNDIR_FAILED;
        log_e("Failed to get coreService config package save dir, err[%d]", iRet);
        return iRet;
    }
    log_d("Config file unzip dir[%s].", pcDownConfFileDir);


    ///< 校验 密钥 配置文件是否存在
    (void)adm_snprintf(cfgFilePath, sizeof(cfgFilePath), "%s%s", pcDownConfFileDir, ADM_ECS_SECRET_CFGFILENAME);
    isExist = adm_com_isDirExist(cfgFilePath);
    if (FALSE == isExist)
    {
        adm_free(pcDownConfFileDir);
        log_e("Failed to find file[%s]", cfgFilePath);
        return ADM_ECODE_OTACFG_FILE_NOEXIST;
    }
    log_d("check symCryptConfig file ok.");

    ///< 读 密钥 文件内容
    iRet = adm_com_readFileData(cfgFilePath, &pcFileData);
    if (ADM_IS_ERR(iRet))
    {
        adm_free(pcDownConfFileDir);
        log_e("Failed to read download config file[%s], err[%d]", cfgFilePath, iRet);
        return iRet;
    }

    ///< 设置密钥内容到kv
    iRet = adm_ecs_setSecret2KV(pcFileData);
    if (ADM_IS_ERR(iRet))
    {
        adm_free(pcFileData);
        adm_free(pcDownConfFileDir);
        iRet = ADM_ECODE_KV_SET_STRING_FAILED;
        log_e( "Failed to set adm_com_kvstorage_set_string, key[%s]", ADM_COM_OTACFG_KVSTORAGE_SYM_SECRET_KEY);
        return iRet;
    }

    adm_free(pcFileData);

    //!< 校验 ADM_ECS_GENERAL_CFGFILENAME 配置文件是否存在
    (void)adm_snprintf(cfgFilePath, sizeof(cfgFilePath), "%s%s", pcDownConfFileDir, ADM_ECS_GENERAL_CFGFILENAME);
    isExist = adm_com_isDirExist(cfgFilePath);
    if (FALSE == isExist)
    {
        adm_free(pcDownConfFileDir);
        log_e("Failed to find file[%s]", cfgFilePath);
        return ADM_ECODE_OTACFG_FILE_NOEXIST;
    }
    log_d("check GeneralConfig file ok.");

    //!< 读 ADM_ECS_GENERAL_CFGFILENAME 文件
    iRet = adm_com_readFileData(cfgFilePath, &pcFileData);
    if (ADM_IS_ERR(iRet))
    {
        adm_free(pcDownConfFileDir);
        log_e("Failed to read download config file[%s], err[%d]", cfgFilePath, iRet);
        return iRet;
    }

    //!< ADM_ECS_GENERAL_CFGFILENAME 存储到 kv 
    bIsOk = adm_com_kvstorage_set_string(pstOtaCfgKvHandle, ADM_COM_OTACFG_KVSTORAGE_CFGGENERALINFO_KEY, pcFileData, ADM_KV_STORAGE_VALUE_TYPE_STRING);
    if (!bIsOk)
    {
        adm_free(pcFileData);
        adm_free(pcDownConfFileDir);
        log_e("Failed to set adm_com_kvstorage_get_string, key[%s]", ADM_COM_OTACFG_KVSTORAGE_CFGGENERALINFO_KEY);
        return ADM_ECODE_KV_SET_STRING_FAILED;
    }
    adm_free(pcFileData);

    //!< 校验 ADM_ECS_PART_CFGFILENAME 配置文件是否存在
    (void)adm_snprintf(cfgFilePath, sizeof(cfgFilePath), "%s%s", pcDownConfFileDir, ADM_ECS_PART_CFGFILENAME);
    isExist = adm_com_isDirExist(cfgFilePath);
    if (FALSE == isExist)
    {
        adm_free(pcDownConfFileDir);
        log_e("Failed to find file[%s]", cfgFilePath);
        return ADM_ECODE_OTACFG_FILE_NOEXIST;
    }
    log_d("check PartConfig file ok.");

    //!< 读 ADM_ECS_PART_CFGFILENAME 文件
    iRet = adm_com_readFileData(cfgFilePath, &pcFileData);
    if (ADM_IS_ERR(iRet))
    {
        adm_free(pcDownConfFileDir);
        log_e("Failed to read download policy file[%s], err[%d]", 
                cfgFilePath, iRet);
        return iRet;
    }

    //!< ADM_ECS_GENERAL_CFGFILENAME 存储到 kv 
    bIsOk = adm_com_kvstorage_set_string(pstOtaCfgKvHandle, ADM_COM_OTACFG_KVSTORAGE_CFGPARTINFO_KEY, pcFileData, ADM_KV_STORAGE_VALUE_TYPE_STRING);
    if (!bIsOk)
    {
        adm_free(pcFileData);
        adm_free(pcDownConfFileDir);
        log_e("Failed to set adm_com_kvstorage_get_string, key[%s]", ADM_COM_OTACFG_KVSTORAGE_CFGPARTINFO_KEY);
        return ADM_ECODE_KV_SET_STRING_FAILED;
    }

    ///< 设置时间戳
    time_t iNow;
    time(&iNow);
    DCHAR aTimestamp[11] = {0};
    (void)adm_memset(aTimestamp, 0x00, sizeof(aTimestamp), sizeof(aTimestamp));
    (void)adm_snprintf(aTimestamp, sizeof(aTimestamp), "%lld", iNow);
    bIsOk = adm_com_kvstorage_set_string(pstOtaCfgKvHandle, ADM_COM_OTACFG_KVSTORAGE_TIMESTAMP, aTimestamp, ADM_KV_STORAGE_VALUE_TYPE_INT64);

    adm_free(pcFileData);
    adm_free(pcDownConfFileDir);

    return iRet;
}

#if 0
#endif

/*!
 *******************************************************************************
 * 从缓存中获取ecu配置数据信息.
 *
 * \note
 * 
 * \param[out]  pstRsCfgData 模块配置数据信息
 * \retval      D32S
 *******************************************************************************
 */
ADM_ECS_RSCONFIGDATA_T *adm_ecs_getOtaConfigDataFromKV(void)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    DBOOL bIsOk = FALSE;
    size_t valLength = 0;
    DCHAR szEventBuf[ADM_COM_LEN_256] = {0}; //mod by wl 20220318
    KvStorage_t *pstOtaCfgKvHandle;
    ADM_ECS_RSCONFIGDATA_T *pstRsCfgData = NULL;
	ADM_KV_STORAGE_VALUE_TYPE_E eValType;

    //mod by wl 20220318
    adm_ecs_eventReport(ADM_COM_MOD_ECS, FALSE, ADM_EVENT_ID_GI_001, ADM_OCS_EVENT_LEVEL_DEBUG, "get vehicle config info from KV.", NULL);

    do
    {
        (void)adm_memzero(szEventBuf, sizeof(szEventBuf));
        pstOtaCfgKvHandle = adm_core_service_getBusinessDatabaseDb();
        if (ADM_IS_NULL(pstOtaCfgKvHandle))
        {
            iRet = ADM_ECODE_KV_OPEN_FAILED;
            log_e("Failed to get ota config kv handle.");
            //mod by wl 20220318
            (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), "Failed to get ota config kv handle, err[%d]", iRet);
            break;
        }

        pstRsCfgData = adm_ecs_newRsCfgDataInfo();
        if (ADM_IS_NULL(pstRsCfgData))
        {
            iRet = ADM_ECODE_NO_MEMORY;
            log_e("Failed to new ota config data info, err[%d]", iRet);
            //mod by wl 20220318
            (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), "Failed to new ota config data info, err[%d]", iRet);
            break;
        }

        //!< confVersion 
        bIsOk = adm_com_kvstorage_get_string(pstOtaCfgKvHandle, ADM_COM_OTACFG_KVSTORAGE_CONFVER_KEY, 
                                &pstRsCfgData->confVersion, &valLength,&eValType);
        if (!bIsOk)
        {
            iRet = ADM_ECODE_KV_GET_STRING_FAILED;
            log_e("Failed to get adm_com_kvstorage_get_string, key[%s]", ADM_COM_OTACFG_KVSTORAGE_CONFVER_KEY);
            //mod by wl 20220318
            (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), 
                    "Failed to get adm_com_kvstorage_get_string, key[%s], err[%d]", ADM_COM_OTACFG_KVSTORAGE_CONFVER_KEY, iRet);
            break;
        }

        //!< stRsCfgFileObj 
        iRet = __adm_ecs_getOtaCfgFileObjFromKV(pstOtaCfgKvHandle, &pstRsCfgData->stRsCfgFileObj);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to get otaCfgFileObj From KV, err[%d]", iRet);
            //mod by wl 20220318
            (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), "Failed to get otaCfgFileObj From KV, err[%d]", iRet);
            break;
        }

        //!< stRsCfgInfo 
        iRet = __adm_ecs_getOtaCfgInfoFromKV(pstOtaCfgKvHandle, &pstRsCfgData->stRsCfgInfo);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to get otaCfgInfo from KV, err[%d]", iRet);
            //mod by wl 20220318
            (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), "Failed to get otaCfgInfo From KV, err[%d]", iRet);
            break;
        }
    } while (FALSE);
    
    if (ADM_IS_ERR(iRet))
    {
        adm_ecs_delRsCfgDataInfo(&pstRsCfgData);
        pstRsCfgData = NULL;

        //mod by wl 20220318
        adm_ecs_eventReport(ADM_COM_MOD_ECS, TRUE, ADM_EVENT_ID_GI_002, ADM_OCS_EVENT_LEVEL_ERROR, 
                "get vehicle config info from KV failed!.", szEventBuf);
    }
    else
    {
        //mod by wl 20220318
        adm_ecs_eventReport(ADM_COM_MOD_ECS, FALSE, ADM_EVENT_ID_GI_003, ADM_OCS_EVENT_LEVEL_DEBUG, 
                "get vehicle config info from KV successed.", ADM_ECS_MARKER_EV_SUB_DOING_CACHE_PASS);
    }

    return pstRsCfgData;
}


/*!
 *******************************************************************************
 * 从缓存中获取ecu配置数据信息.
 *
 * \note
 * 
 * \param[out]  pstRsCfgData 模块配置数据信息
 * \retval      D32S
 *******************************************************************************
 */
D32S adm_ecs_setOtaConfigData2KV(ADM_ECS_RSCONFIGDATA_T *pstRsCfgData)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    DBOOL bIsOk = FALSE;
    DCHAR szEventBuf[ADM_COM_LEN_256] = {0};  //mod by wl 20220318
    KvStorage_t *pstOtaCfgKvHandle;

    //mod by wl 20220318
    adm_ecs_eventReport(ADM_COM_MOD_ECS, FALSE, ADM_EVENT_ID_GI_067, ADM_OCS_EVENT_LEVEL_TRACE, "vehicle set config info to kv info.", NULL);

    if (ADM_IS_NULL(pstRsCfgData))
    {
        log_e("Invalid input param, err[%d]", ADM_ECODE_INVALID_PARAM);

        //mod by wl 20220318
        (void)adm_memzero(szEventBuf, sizeof(szEventBuf));
        (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), "Invalid input param, err[%d]", ADM_ECODE_INVALID_PARAM);
        adm_ecs_eventReport(ADM_COM_MOD_ECS, TRUE, ADM_EVENT_ID_GI_068, ADM_OCS_EVENT_LEVEL_ERROR, "vehicle set config info to kv info failed!", szEventBuf);

        return ADM_ECODE_INVALID_PARAM;
    }

    do
    {
        //mod by wl 20220318
        (void)adm_memzero(szEventBuf, sizeof(szEventBuf));
        pstOtaCfgKvHandle = adm_core_service_getBusinessDatabaseDb();
        if (ADM_IS_NULL(pstOtaCfgKvHandle))
        {
            iRet = ADM_ECODE_KV_OPEN_FAILED;
            log_e("Failed to get ota config kv handle.");
            //mod by wl 20220318
            (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), "Failed to get ota config kv handle, err[%d]", iRet);
            break;
        }

        //!< confVersion 
        bIsOk = adm_com_kvstorage_set_string(pstOtaCfgKvHandle, ADM_COM_OTACFG_KVSTORAGE_CONFVER_KEY, 
                                pstRsCfgData->confVersion, ADM_KV_STORAGE_VALUE_TYPE_STRING);
        if (!bIsOk)
        {
            iRet = ADM_ECODE_KV_SET_STRING_FAILED;
            log_e( "Failed to set adm_com_kvstorage_set_string, key[%s]", ADM_COM_OTACFG_KVSTORAGE_CONFVER_KEY);
            //mod by wl 20220318
            (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), 
                    "Failed to set adm_com_kvstorage_set_string, key[%s], err[%d]", ADM_COM_OTACFG_KVSTORAGE_CONFVER_KEY, iRet);
            break;
        }

        //!< stRsCfgFileObj 
        iRet = __adm_ecs_setOtaCfgFileObj2KV(pstOtaCfgKvHandle, &pstRsCfgData->stRsCfgFileObj);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to set otaCfgFileObj to KV, err[%d]", iRet);
            //mod by wl 20220318
            (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), "Failed to set otaCfgFileObj to KV, err[%d]", iRet);
            break;
        }

        //!< stRsCfgInfo 
        iRet = __adm_ecs_setOtaCfgFileInfo2KV(pstOtaCfgKvHandle, &pstRsCfgData->stRsCfgInfo);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to set otaCfgInfo to KV, err[%d]", iRet);
            //mod by wl 20220318
            (void)adm_snprintf(szEventBuf, sizeof(szEventBuf), "Failed to set otaCfgInfo to KV,err[%d]", iRet);
            break;
        }
    } while (FALSE);

    //mod by wl 20220318
    if (ADM_IS_ERR(iRet))
    {
        adm_ecs_eventReport(ADM_COM_MOD_ECS, TRUE, ADM_EVENT_ID_GI_068, ADM_OCS_EVENT_LEVEL_ERROR, "vehicle set config info to kv info failed!", szEventBuf);
    }
    else
    {
        adm_ecs_eventReport(ADM_COM_MOD_ECS, FALSE, ADM_EVENT_ID_GI_069, ADM_OCS_EVENT_LEVEL_DEBUG, "vehicle set config info to kv info successed.", NULL);
    }

    return iRet;
}

D32S adm_ecs_getSecretFromKV(DCHAR** ppSecret, DSIZE* pSecretLength)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    ADM_KV_STORAGE_VALUE_TYPE_E eValType = ADM_KV_STORAGE_VALUE_TYPE_INVALID;
    DSIZE uiSecretEncLength = 0;
    DBOOL bIsOk = FALSE;
	DCHAR* pSymSecretEnc = NULL;
    KvStorage_t *pstOtaCfgKvHandle = NULL;

    if (ADM_IS_NULL(ppSecret) || ADM_IS_NULL(pSecretLength))
    {
        log_e("Invalid input param, err[%d]", ADM_ECODE_INVALID_PARAM);
        return ADM_ECODE_INVALID_PARAM;
    }

    do
    {
        pstOtaCfgKvHandle = adm_core_service_getBusinessDatabaseDb();
        if (ADM_IS_NULL(pstOtaCfgKvHandle))
        {
            iRet = ADM_ECODE_KV_OPEN_FAILED;
            log_e("Failed to get ota config kv handle.");
            break;
        }

        ///< 获取对称密钥
        bIsOk = adm_com_kvstorage_get_string(pstOtaCfgKvHandle, ADM_COM_OTACFG_KVSTORAGE_SYM_SECRET_KEY, 
                                &pSymSecretEnc, &uiSecretEncLength, &eValType);
        if (!bIsOk)
        {
            log_e("Failed to get adm_com_kvstorage_get_string, key[%s]", ADM_COM_OTACFG_KVSTORAGE_SYM_SECRET_KEY);
            iRet =  ADM_ECODE_KV_GET_STRING_FAILED;
            break;
        }

        /** 解密出对称密钥 */
        iRet = adm_parseCryptoKey(pSymSecretEnc, uiSecretEncLength, (D8U**)ppSecret, pSecretLength);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to get adm_parseCryptoKey.");
            break;
        }
    } while (FALSE);

    adm_free(pSymSecretEnc);

    return iRet;
}

D32S adm_ecs_setSecret2KV(DCHAR* pSecret)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    DBOOL bIsOk = FALSE;
    KvStorage_t *pstOtaCfgKvHandle = NULL;

    if (ADM_IS_NULL(pSecret))
    {
        log_e("Invalid input param, err[%d]", ADM_ECODE_INVALID_PARAM);
        return ADM_ECODE_INVALID_PARAM;
    }

    do{
        pstOtaCfgKvHandle = adm_core_service_getBusinessDatabaseDb();
        if (ADM_IS_NULL(pstOtaCfgKvHandle))
        {
            iRet = ADM_ECODE_KV_OPEN_FAILED;
            log_e("Failed to get ota config kv handle.");
            break;
        }

        bIsOk = adm_com_kvstorage_set_string(pstOtaCfgKvHandle, ADM_COM_OTACFG_KVSTORAGE_SYM_SECRET_KEY, 
                                pSecret, ADM_KV_STORAGE_VALUE_TYPE_STRING);
        if (!bIsOk)
        {
            iRet = ADM_ECODE_KV_SET_STRING_FAILED;
            log_e( "Failed to set adm_com_kvstorage_set_string, key[%s]", ADM_COM_OTACFG_KVSTORAGE_SYM_SECRET_KEY);
            break;
        }
    } while (FALSE);

    return iRet;
}


#if 0
#endif



