/**
 * @file    adm_tps_kvStorage.c
 * @brief   tps module key value storage  
 * @details tps 模块 kv 存储
 * @author  wangyanpeng
 * @version v0.1.0
 * @date    2020-11-06
 * @copyright www.abupdate.com
 */
#include <stddef.h>
#include "adm_string.h"
#include "adm_memory.h"
#include "adm_tps_log.h"
#include "adm_com_lib.h"
#include "adm_com_errorCode.h"
#include "adm_com_modDefine.h"
#include "adm_dps_kvStorage.h"
#include "adm_tps_kvStorage.h"
#include "adm_engine_common.h"
#include "adm_com_kvstorage.h"


#define ADM_TPS_TRANSECU_TRANSECUINFO   "transEcuInfo"
#define ADM_TPS_TRANSECU_DOWNINSTID     "downInstId"
#define ADM_TPS_TRANSECU_TRANSINSTID    "transInstId"
#define ADM_TPS_TRANSECU_INSTALLINSTID  "installInstId"
#define ADM_TPS_TRANSECU_SRCPKG         "srcPkg"
#define ADM_TPS_TRANSECU_DSTPKG         "dstPkg"
#define ADM_TPS_TRANSECU_HASH           "hash" 
#define ADM_TPS_TRANSECU_FILENAME       "fileName"
#define ADM_TPS_TRANSECU_FILEPATH       "filePath"
#define ADM_TPS_TRANSECU_FILESIZE       "fileSize"

//!< result
#define ADM_TPS_TRANSECU_RESULT         "result"
#define ADM_TPS_TRANSECU_TRANSSTATE     "transState"
#define ADM_TPS_TRANSECU_FAILREASON     "failReason"

//!< progress
#define ADM_TPS_TRANSECU_SPEED          "speed"
#define ADM_TPS_TRANSECU_TOTALTIME      "totalTime"
#define ADM_TPS_TRANSECU_REMAINTIME     "remainTime"
#define ADM_TPS_TRANSECU_SAVEBYTES      "saveBytes"
#define ADM_TPS_TRANSECU_TOTALBYTES     "totalBytes"


#if 0
#endif


/*!
 *******************************************************************************
 * 添加一个集合包信息到json数组中.
 *
 * \note 
 *  
 * \param[in]     pstEcuCombinePkg 集合包信息
 * \param[inout]  arrayObjJson     json数组节点
 * \retval        D32S
 *******************************************************************************
 */
static D32S __adm_tps_addTransEcu2JsonArrayObject
(
    ADM_TPS_TRANSFERPKG_T *pstEcuPkg, 
    json_object           *arrayObjJson
)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    json_object *indexObjJson  = NULL;
    json_object *stringObject  = NULL;
    json_object *ecuPkgObjJson = NULL;

    if (ADM_IS_NULL(pstEcuPkg) || ADM_IS_NULL(arrayObjJson))
    {
        log_e("Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }

    do
    {
        indexObjJson = json_object_new_object();
        if (ADM_IS_NULL(indexObjJson))
        {
            iRet = ADM_ECODE_NEW_JSON_OBJECT_FAILED;
            log_e("Failed to new json object, err[%s]", iRet);
            break;
        }

        //!< add ecuReqid
        if (NULL != pstEcuPkg->ecuReqid)
        {
            stringObject = json_object_new_string(pstEcuPkg->ecuReqid);
        }
        if (ADM_IS_NULL(stringObject)) 
        {
            iRet = ADM_ECODE_NEW_JSON_OBJECT_FAILED;
            log_e("Failed to new ecuReqid json object, err[%d]", iRet);
            break;
        }
        iRet = json_object_object_add(indexObjJson, ADM_COM_AIR_PROTOCOL_FIELD_ECUREQID, stringObject);
        if (ADM_IS_ERR(iRet))
        {
            iRet = ADM_ECODE_JSON_ADD_STRING_FAILED;
            log_e("Failed to add ecuReqid json object, err[%d]", iRet);
            (void)json_object_put(stringObject);
            break;
        }

        //!< add ecuSwDid
        stringObject = NULL;
        if (NULL != pstEcuPkg->ecuSwDid)
        {
            stringObject = json_object_new_string(pstEcuPkg->ecuSwDid);
        }
        if (ADM_IS_NULL(stringObject)) 
        {
            iRet = ADM_ECODE_NEW_JSON_OBJECT_FAILED;
            log_e("Failed to new ecuSwDid json object, err[%d]", iRet);
            break;
        }
        iRet = json_object_object_add(indexObjJson, ADM_COM_AIR_PROTOCOL_FIELD_ECUSWDID, stringObject);
        if (ADM_IS_ERR(iRet))
        {
            iRet = ADM_ECODE_JSON_ADD_STRING_FAILED;
            log_e("Failed to add ecuSwDid json object, err[%d]", iRet);
            (void)json_object_put(stringObject);
            break;
        }
    
        //!< add downloadInstanceId
        stringObject = NULL;
        if (NULL != pstEcuPkg->downloadInstanceId)
        {
            stringObject = json_object_new_string(pstEcuPkg->downloadInstanceId);
        }
        if (ADM_IS_NULL(stringObject)) 
        {
            iRet = ADM_ECODE_NEW_JSON_OBJECT_FAILED;
            log_e("Failed to new downloadInstanceId json object, err[%d]", iRet);
            break;
        }
        iRet = json_object_object_add(indexObjJson, ADM_TPS_TRANSECU_DOWNINSTID, stringObject);
        if (ADM_IS_ERR(iRet))
        {
            iRet = ADM_ECODE_JSON_ADD_STRING_FAILED;
            log_e("Failed to add downloadInstanceId json object, err[%d]", iRet);
            (void)json_object_put(stringObject);
            break;
        }

        //!< add transferInstanceId
        stringObject = NULL;
        if (NULL != pstEcuPkg->transferInstanceId)
        {
            stringObject = json_object_new_string(pstEcuPkg->transferInstanceId);
        }
        if (ADM_IS_NULL(stringObject)) 
        {
            iRet = ADM_ECODE_NEW_JSON_OBJECT_FAILED;
            log_e("Failed to new transferInstanceId json object, err[%d]", iRet);
            break;
        }
        iRet = json_object_object_add(indexObjJson, ADM_TPS_TRANSECU_TRANSINSTID, stringObject);
        if (ADM_IS_ERR(iRet))
        {
            iRet = ADM_ECODE_JSON_ADD_STRING_FAILED;
            log_e("Failed to add transferInstanceId json object, err[%d]", iRet);
            (void)json_object_put(stringObject);
            break;
        }
        //!< add installInstanceId
        stringObject = NULL;
        if (NULL != pstEcuPkg->installInstanceId)
        {
            stringObject = json_object_new_string(pstEcuPkg->installInstanceId);
        }
        if (ADM_IS_NULL(stringObject)) 
        {
            iRet = ADM_ECODE_NEW_JSON_OBJECT_FAILED;
            log_e("Failed to new installInstanceId json object, err[%d]", iRet);
            break;
        }
        iRet = json_object_object_add(indexObjJson, ADM_TPS_TRANSECU_INSTALLINSTID, stringObject);
        if (ADM_IS_ERR(iRet))
        {
            iRet = ADM_ECODE_JSON_ADD_STRING_FAILED;
            log_e("Failed to add installInstanceId json object, err[%d]", iRet);
            (void)json_object_put(stringObject);
            break;
        }

        //!< add srcPkg
        if (NULL != pstEcuPkg->pstSrcPkgInfo)
        {
            ecuPkgObjJson = json_object_new_object();
            if (ADM_IS_NULL(ecuPkgObjJson))
            {
                iRet = ADM_ECODE_NEW_JSON_OBJECT_FAILED;
                log_e("Failed to new json object, err[%s]", iRet);
                break;
            }
            
            iRet = adm_com_enFileObjectAdd2JsonObj(pstEcuPkg->pstSrcPkgInfo, ecuPkgObjJson);
            if (ADM_IS_ERR(iRet))
            {
                iRet = ADM_ECODE_NEW_JSON_OBJECT_FAILED;
                log_e("Failed to create ecu src pkg info to json object, err[%s]", iRet);
                break;
            } 

            iRet = json_object_object_add(indexObjJson, ADM_TPS_TRANSECU_SRCPKG, ecuPkgObjJson);
            if (ADM_IS_ERR(iRet))
            {
                iRet = ADM_ECODE_JSON_ADD_STRING_FAILED;
                log_e("Failed to add installInstanceId json object, err[%d]", iRet);
                (void)json_object_put(ecuPkgObjJson);
                break;
            }  
        }

        //!< add dstPkg
        ecuPkgObjJson = json_object_new_object();
        if (ADM_IS_NULL(ecuPkgObjJson))
        {
            iRet = ADM_ECODE_NEW_JSON_OBJECT_FAILED;
            log_e("Failed to new json object, err[%s]", iRet);
            break;
        }

        iRet = adm_com_enFileObjectAdd2JsonObj(&pstEcuPkg->stDstPkgInfo, ecuPkgObjJson);
        if (ADM_IS_ERR(iRet))
        {
            iRet = ADM_ECODE_NEW_JSON_OBJECT_FAILED;
            log_e("Failed to create ecu dst pkg info to json object, err[%s]", iRet);
            break;
        } 
        
        iRet = json_object_object_add(indexObjJson, ADM_TPS_TRANSECU_DSTPKG, ecuPkgObjJson);
        if (ADM_IS_ERR(iRet))
        {
            iRet = ADM_ECODE_JSON_ADD_STRING_FAILED;
            log_e("Failed to add installInstanceId json object, err[%d]", iRet);
            (void)json_object_put(ecuPkgObjJson);
            break;
        }  

        //!< add arrayIndex to array
        iRet = json_object_array_add(arrayObjJson, indexObjJson);
        if (ADM_IS_ERR(iRet))
        {
            iRet = ADM_ECODE_JSON_ADD_ARRAY_FAILED;
            log_e("Failed to add object to array, err[%d]", iRet);
            break;
        }
    } while (FALSE);
    
    if (ADM_IS_ERR(iRet))
    {        
        (void)json_object_put(indexObjJson);
    }

    return iRet;
}

/*!
 *******************************************************************************
 * 解析策略数据中的transEcuInfo信息.
 * 
 * \note 
 *
 * \param[in]   jsonTransEcusInfo   Json transEcuInfo 信息
 * \param[out]  pstTransEcuPkg      待传输ecu信息
 * \retval      D32S
 *******************************************************************************
 */
static D32S __adm_tps_parseTransEcuInfo
(
    json_object           *jsonTransEcusInfo,
    ADM_TPS_TRANSFERPKG_T *pstTransEcuPkg
)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    json_type jsonType = json_type_null;
    json_object *jsonSrcPkg = NULL;
    json_object *jsonDstPkg = NULL;

    if (ADM_IS_NULL(jsonTransEcusInfo) || ADM_IS_NULL(pstTransEcuPkg))
    {
        log_e( "Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }

    do
    {
        //!< get ecuReqid
        pstTransEcuPkg->ecuReqid = adm_com_getJsonStrVal(jsonTransEcusInfo, ADM_COM_AIR_PROTOCOL_FIELD_ECUREQID);
        if (ADM_IS_NULL(pstTransEcuPkg->ecuReqid))
        {
            iRet = ADM_ECODE_JSON_GET_KEY_FAILED;
            log_e( "Failed to json value, key[%s], err[%d]", ADM_COM_AIR_PROTOCOL_FIELD_ECUREQID, iRet);
            break;
        }
        
        //!< get ecuSwDid
        pstTransEcuPkg->ecuSwDid = adm_com_getJsonStrVal(jsonTransEcusInfo, ADM_COM_AIR_PROTOCOL_FIELD_ECUSWDID);
        if (ADM_IS_NULL(pstTransEcuPkg->ecuSwDid))
        {
            iRet = ADM_ECODE_JSON_GET_KEY_FAILED;
            log_e( "Failed to json value, key[%s], err[%d]", ADM_COM_AIR_PROTOCOL_FIELD_ECUSWDID, iRet);
            break;
        }
        log_i("reqId[%s], eswDid[%s]", pstTransEcuPkg->ecuReqid, pstTransEcuPkg->ecuSwDid);
        
        //!< get downloadInstanceId
        pstTransEcuPkg->downloadInstanceId = adm_com_getJsonStrVal(jsonTransEcusInfo, ADM_TPS_TRANSECU_DOWNINSTID);
        if (ADM_IS_NULL(pstTransEcuPkg->downloadInstanceId))
        {
            iRet = ADM_ECODE_JSON_GET_KEY_FAILED;
            log_e( "Failed to json value, key[%s], err[%d]", ADM_TPS_TRANSECU_DOWNINSTID, iRet);
            break;
        }
        
        //!< get transferInstanceId
        pstTransEcuPkg->transferInstanceId = adm_com_getJsonStrVal(jsonTransEcusInfo, ADM_TPS_TRANSECU_TRANSINSTID);
        if (ADM_IS_NULL(pstTransEcuPkg->transferInstanceId))
        {
            iRet = ADM_ECODE_JSON_GET_KEY_FAILED;
            log_e( "Failed to json value, key[%s], err[%d]", ADM_TPS_TRANSECU_TRANSINSTID, iRet);
            break;
        }
        
        //!< get installInstanceId
        pstTransEcuPkg->installInstanceId = adm_com_getJsonStrVal(jsonTransEcusInfo, ADM_TPS_TRANSECU_INSTALLINSTID);
        if (ADM_IS_NULL(pstTransEcuPkg->installInstanceId))
        {
            iRet = ADM_ECODE_JSON_GET_KEY_FAILED;
            log_e( "Failed to json value, key[%s], err[%d]", ADM_TPS_TRANSECU_INSTALLINSTID, iRet);
            break;
        }
        
        //!< 判断Json中dstPkg是否存在 --- 必填字段
        if (!json_object_object_get_ex(jsonTransEcusInfo, ADM_TPS_TRANSECU_DSTPKG, &jsonDstPkg))
        {
            iRet = ADM_ECODE_JSON_GET_KEY_FAILED;
            log_e( "Failed to get json value, key[%s]", ADM_TPS_TRANSECU_DSTPKG);
            break;
        }
        
        jsonType = json_object_get_type(jsonDstPkg);
        if (jsonType != json_type_object)
        {
            iRet = ADM_ECODE_JSON_KEY_TYPE_ERROR;
            log_e( "Json key[%s] type error", ADM_TPS_TRANSECU_DSTPKG);
            break;
        }
        
        iRet = adm_com_parseJsonFileObject(jsonDstPkg, &pstTransEcuPkg->stDstPkgInfo);
        if (ADM_IS_ERR(iRet))
        {
            log_e( "Failed to parse object[%s], err[%d]", ADM_TPS_TRANSECU_DSTPKG, iRet);
            break;
        }
        
        //!< 判断Json中srcPkg是否存在 --- 可选字段
        if (!json_object_object_get_ex(jsonTransEcusInfo, ADM_TPS_TRANSECU_SRCPKG, &jsonSrcPkg))
        {
            iRet = ADM_ECODE_SUCCESSED;
            log_i( "Failed to get json value, key[%s]", ADM_TPS_TRANSECU_SRCPKG);
            break;
        }
        
        jsonType = json_object_get_type(jsonSrcPkg);
        if (jsonType != json_type_object)
        {
            iRet = ADM_ECODE_JSON_KEY_TYPE_ERROR;
            log_e( "Json key[%s] type error", ADM_TPS_TRANSECU_SRCPKG);
            break;
        }
        
        //!< pstSrcEcuPkgInfo 内存申请 --- 解析
        pstTransEcuPkg->pstSrcPkgInfo = adm_com_newFileObject();
        if (ADM_IS_NULL(pstTransEcuPkg->pstSrcPkgInfo))
        {
            iRet = ADM_ECODE_NO_MEMORY;
            log_e( "Failed to new ecu src package info");
            break;
        }
        
        iRet = adm_com_parseJsonFileObject(jsonSrcPkg, pstTransEcuPkg->pstSrcPkgInfo);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to parse object[%s], err[%d]", ADM_TPS_TRANSECU_SRCPKG, iRet);
            break;
        }
    } while (FALSE);
    
    if (ADM_IS_ERR(iRet))
    {
        adm_tps_cleanTransferEcuPackage(pstTransEcuPkg);
    }

    return iRet;
}


/*!
 *******************************************************************************
 * 序列化tps待分包ecu信息为json字符串.
 *
 * \note
 *
 * \param[in]     pstTransferECUList         待分包ecu信息
 * \param[out]    ppcTransferEcuInfoJsonChar 待分包ecu信息的json字符串
 * \retval        D32S
 *******************************************************************************
 */
static D32S __adm_tps_enTransEcuInfo2JsonChar
(
    ADM_TPS_TRANSFER_ECULIST_T  *pstTransferECUList,
    DCHAR                      **ppcTransferEcuInfoJsonChar
)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    D32S i = 0;
    json_object *arrayObjJson = NULL;
    json_object *pstTransferEcuInfoJson = NULL;

    if (ADM_IS_NULL(pstTransferECUList) || ADM_IS_NULL(ppcTransferEcuInfoJsonChar))
    {
        log_e( "Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }

    do
    {        
        pstTransferEcuInfoJson = json_object_new_object();
        if (ADM_IS_NULL(pstTransferEcuInfoJson))
        {
            iRet = ADM_ECODE_NEW_JSON_OBJECT_FAILED;
            log_e("Faile to new json object, err[%d]", iRet);
            break;
        }

        arrayObjJson = json_object_new_array();
        if (ADM_IS_NULL(arrayObjJson)) 
        {
            iRet = ADM_ECODE_NEW_JSON_OBJECT_FAILED;
            log_e("Failed to new json array object, err[%d]", iRet);
            break;
        }

        for (i = 0; (D32U)i < pstTransferECUList->num; ++i) 
        {
            iRet = __adm_tps_addTransEcu2JsonArrayObject(&pstTransferECUList->pstEcuPkg[i], arrayObjJson);
            if (ADM_IS_ERR(iRet))
            {
                log_e("Failed to add transfer ecu info to json array object, err[%s]", iRet);
                break;
            }        
        }
        
        if (ADM_IS_ERR(iRet))
        {
            (void)json_object_put(arrayObjJson);
            break;
        }
        
        iRet = json_object_object_add(pstTransferEcuInfoJson, ADM_TPS_TRANSECU_TRANSECUINFO, arrayObjJson);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to add transEcuInfo object to json object, err[%d]", ADM_ECODE_JSON_ADD_STRING_FAILED);
            (void)json_object_put(arrayObjJson);
            return iRet;
        }

        //!< json to string
        *ppcTransferEcuInfoJsonChar = adm_strdup(json_object_to_json_string(pstTransferEcuInfoJson));
        if (ADM_IS_NULL(*ppcTransferEcuInfoJsonChar))
        {
            log_e("Failed to json to string");
            iRet = ADM_ECODE_JSON_TO_STRING_FAILED;
            break;
        }
    } while(FALSE);

    if (NULL != pstTransferEcuInfoJson)
    {
        (void)json_object_put(pstTransferEcuInfoJson);
    }

    return iRet;
}

/*!
 *******************************************************************************
 * 反序列化json字符串到待下载ecu信息.
 *
 * \note
 * 
 * \param[in]   pstDownEcuInfoJsonChar 待下载ecu信息的json字符串
 * \param[out]  pstDownECUInfo         待下载ecu信息
 * \retval      D32S
 *******************************************************************************
 */
static D32S __adm_tps_deJsonChar2TransEcuInfo
(
    DCHAR                      *pstTransEcuInfoJsonChar,
    ADM_TPS_TRANSFER_ECULIST_T *pstTransferECUList
)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    D32S i = 0;
    json_type jsonType = json_type_null;
    json_object *jsonTransEcus = NULL;
    json_object *jsonArrayIndexObj = NULL;
    json_object *jsonRoot = NULL;

    if (ADM_IS_NULL(pstTransEcuInfoJsonChar) || ADM_IS_NULL(pstTransferECUList))
    {
        log_e("Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }
    
    jsonRoot = json_tokener_parse(pstTransEcuInfoJsonChar);
    if (ADM_IS_NULL(jsonRoot)) 
    {
        log_e("Failed string to json, string[%s]", pstTransEcuInfoJsonChar);
        return ADM_ECODE_STRING_TO_JSON_FAILED;
    }
    
    //!< Json transEcuInfo
    if (!json_object_object_get_ex(jsonRoot, ADM_TPS_TRANSECU_TRANSECUINFO, &jsonTransEcus))
    {
        (void)json_object_put(jsonRoot);
        log_e( "Failed to get json value, key[%s]", ADM_TPS_TRANSECU_TRANSECUINFO);
        return ADM_ECODE_JSON_GET_KEY_FAILED;
    }
    
    jsonType = json_object_get_type(jsonTransEcus);
    if (jsonType != json_type_array)
    {
        (void)json_object_put(jsonRoot);
        log_e( "Json key[%s] type error", ADM_TPS_TRANSECU_TRANSECUINFO);
        return ADM_ECODE_JSON_KEY_TYPE_ERROR;
    }
    
    pstTransferECUList->num = (D32U)json_object_array_length(jsonTransEcus);
    log_i("trans ecus array len[%d]", pstTransferECUList->num);
    if ((D32U)0 == pstTransferECUList->num)
    {
        (void)json_object_put(jsonRoot);
        log_i( "%s array list is zero", ADM_TPS_TRANSECU_TRANSECUINFO);
        return ADM_ECODE_SUCCESSED;
    }
    
    pstTransferECUList->pstEcuPkg = adm_tps_newTransferEcuPackageList(pstTransferECUList->num);
    if (ADM_IS_NULL(pstTransferECUList->pstEcuPkg))
    {
        (void)json_object_put(jsonRoot);
        log_e( "Failed to new ecu part package list");
        return ADM_ECODE_NO_MEMORY;
    }
    
    for (i = 0; (D32U)i < pstTransferECUList->num; ++i) 
    {
        jsonArrayIndexObj = json_object_array_get_idx(jsonTransEcus, (D32U)i);
        if (ADM_IS_NULL(jsonArrayIndexObj)) 
        {
            iRet = ADM_ECODE_JSON_GET_ARRAYOBJECT_FAILED;
            log_e( "Failed to backtracking Json array, arrayIndex[%d]", i);
            break;
        }
    
        iRet = __adm_tps_parseTransEcuInfo(jsonArrayIndexObj, &pstTransferECUList->pstEcuPkg[i]);
        if (ADM_IS_ERR(iRet))
        {
            log_e( "Failed to parse combine Package info, err[%d]", iRet);
            break;
        }
    }
    (void)json_object_put(jsonRoot);
    
    if (ADM_IS_ERR(iRet))
    {
        adm_tps_delTransferEcuPackageList(pstTransferECUList->num, &pstTransferECUList->pstEcuPkg);
        pstTransferECUList->num = (D32U)0;
        return iRet;
    }    
    log_i("Parse json char to transfer ecu info ok.");

    return iRet;
}


#if 0 
#endif

/*!
 *******************************************************************************
 * 序列化分包结果信息为json字符串.
 *
 * \note
 * 
 * \param[in]   pstTransResult         分包结果信息
 * \param[out]  ppcTransResultJsonChar 分包结果信息的json字符串
 * \retval      D32S
 *******************************************************************************
 */
static D32S __adm_tps_enTransResult2JsonChar
(
    ADM_TPS_TRANSRESULT_T  *pstTransResult,
    DCHAR                 **ppcTransResultJsonChar
)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    json_object *pstrObjJson = NULL;
    json_object *pstTransResultJson = NULL;

    if (ADM_IS_NULL(pstTransResult) || ADM_IS_NULL(ppcTransResultJsonChar))
    {
        log_e("Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }

    do
    {        
        pstTransResultJson = json_object_new_object();
        if (ADM_IS_NULL(pstTransResultJson))
        {
            iRet = ADM_ECODE_NEW_JSON_OBJECT_FAILED;
            log_e("Faile to new json object, err[%d]", iRet);
            break;
        }

        //!< add result
        pstrObjJson = json_object_new_int(pstTransResult->result);
        if (ADM_IS_NULL(pstrObjJson)) 
        {
            iRet = ADM_ECODE_NEW_JSON_OBJECT_FAILED;
            log_e("Failed to new result json object");
            break;
        }
        iRet = json_object_object_add(pstTransResultJson, ADM_TPS_TRANSECU_RESULT, pstrObjJson);
        if (ADM_IS_ERR(iRet))
        {
            (void)json_object_put(pstrObjJson);
            iRet = ADM_ECODE_JSON_ADD_INT_FAILED;
            log_e("Failed to add result json object");
            break;
        }

        //!< add downState
        pstrObjJson = json_object_new_int((int32_t)pstTransResult->eTransState);
        if (ADM_IS_NULL(pstrObjJson)) 
        {
            iRet = ADM_ECODE_NEW_JSON_OBJECT_FAILED;
            log_e("Failed to new downState json object");
            break;
        }
        iRet = json_object_object_add(pstTransResultJson, ADM_TPS_TRANSECU_TRANSSTATE, pstrObjJson);
        if (ADM_IS_ERR(iRet))
        {
            (void)json_object_put(pstrObjJson);
            iRet = ADM_ECODE_JSON_ADD_INT_FAILED;
            log_e("Failed to add downState json object");
            break;
        }

        //!< add failReason
        pstrObjJson = json_object_new_string(ADM_PRINT_STR(pstTransResult->failReason));
        if (ADM_IS_NULL(pstrObjJson)) 
        {
            log_e("Failed to new failReason json object");
            iRet = ADM_ECODE_NEW_JSON_OBJECT_FAILED;
            break;
        }
        iRet = json_object_object_add(pstTransResultJson, ADM_TPS_TRANSECU_FAILREASON, pstrObjJson);
        if (ADM_IS_ERR(iRet))
        {
            (void)json_object_put(pstrObjJson);
            iRet = ADM_ECODE_JSON_ADD_STRING_FAILED;
            log_e("Failed to add failReason json object");
            break;
        }

        //!< json to string
        *ppcTransResultJsonChar = adm_strdup(json_object_to_json_string(pstTransResultJson));
        if (ADM_IS_NULL(*ppcTransResultJsonChar))
        {
            log_e("Failed to json to string");
            iRet = ADM_ECODE_JSON_TO_STRING_FAILED;
            break;
        }
    } while(FALSE);

    if (NULL != pstTransResultJson)
    {
        (void)json_object_put(pstTransResultJson);
    }

    return iRet;
}


/*!
 *******************************************************************************
 * 反序列化json字符串到分包结果结构信息.
 *
 * \note
 * 
 * \param[in]   pcTransResultJsonChar 分包结果信息的json字符串
 * \param[out]  pstTransResult        分包结果信息
 * \retval      D32S
 *******************************************************************************
 */
static D32S __adm_tps_deJsonChar2TransResultInfo
(
    DCHAR                 *pcTransResultJsonChar,
    ADM_TPS_TRANSRESULT_T *pstTransResult
)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    D32S iTransState = 0;
    json_object *jsonRoot = NULL;

    if (ADM_IS_NULL(pcTransResultJsonChar) || ADM_IS_NULL(pstTransResult))
    {
        log_e("Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }
    
    do
    {
        jsonRoot = json_tokener_parse(pcTransResultJsonChar);
        if (ADM_IS_NULL(jsonRoot)) 
        {
            iRet = ADM_ECODE_STRING_TO_JSON_FAILED;
            log_e("Failed string to json, string[%s]", pcTransResultJsonChar);
            break;
        }

        //!< get failReason
        pstTransResult->failReason = adm_com_getJsonStrVal(jsonRoot, ADM_TPS_TRANSECU_FAILREASON);
        if (ADM_IS_NULL(pstTransResult->failReason))
        {
            iRet = ADM_ECODE_JSON_GET_KEY_FAILED;
            log_e("Failed to get json value, key[%s]", ADM_TPS_TRANSECU_FAILREASON);
            break;
        }

        //!< get result
        iRet = adm_com_getJsonIntVal(jsonRoot, ADM_TPS_TRANSECU_RESULT, &pstTransResult->result);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to get json value, key[%s], err[%d]", ADM_TPS_TRANSECU_RESULT, iRet);
            break;
        }

        //!< get downState
        iRet = adm_com_getJsonIntVal(jsonRoot, ADM_TPS_TRANSECU_TRANSSTATE, &iTransState);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to get json value, key[%s], err[%d]", ADM_TPS_TRANSECU_TRANSSTATE, iRet);
            break;
        }
        pstTransResult->eTransState = (ADM_TPS_TRANSPKGSTATE_E)iTransState;
    } while (FALSE);

    if (ADM_IS_ERR(iRet))
    {
        adm_free(pstTransResult->failReason);
        pstTransResult->result = 0;
        pstTransResult->eTransState = ADM_TPS_TRANSPKG_STATE_NONE;
    }

    if (NULL != jsonRoot)
    {
        (void)json_object_put(jsonRoot);
    }

    return iRet;
}

/*!
 *******************************************************************************
 * 序列化分包进度信息为json字符串.
 *
 * \note
 * 
 * \param[in]   pstTransProgress         分包进度信息
 * \param[out]  ppcTransProgressJsonChar 分包进度信息的json字符串
 * \retval      D32S
 *******************************************************************************
 */
static D32S __adm_tps_enTransProgress2JsonChar
(
    ADM_TPS_TRANSPROGRESS_T *pstTransProgress,
    DCHAR                  **ppcTransProgressJsonChar
)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    json_object *pstrObjJson = NULL;
    json_object *pstTransProgressJson = NULL;

    if (ADM_IS_NULL(pstTransProgress) || ADM_IS_NULL(ppcTransProgressJsonChar))
    {
        log_e("Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }

    do
    {        
        pstTransProgressJson = json_object_new_object();
        if (ADM_IS_NULL(pstTransProgressJson))
        {
            iRet = ADM_ECODE_NEW_JSON_OBJECT_FAILED;
            log_e("Faile to new json object, err[%d]", iRet);
            break;
        }

        //!< add speed
        pstrObjJson = json_object_new_int(pstTransProgress->speed);
        if (ADM_IS_NULL(pstrObjJson)) 
        {
            iRet = ADM_ECODE_NEW_JSON_OBJECT_FAILED;
            log_e("Failed to new speed json object");
            break;
        }
        iRet = json_object_object_add(pstTransProgressJson, ADM_TPS_TRANSECU_SPEED, pstrObjJson);
        if (ADM_IS_ERR(iRet))
        {
            (void)json_object_put(pstrObjJson);
            iRet = ADM_ECODE_JSON_ADD_INT_FAILED;
            log_e("Failed to add speed json object");
            break;
        }

        //!< add totalTime
        pstrObjJson = json_object_new_int(pstTransProgress->totalTime);
        if (ADM_IS_NULL(pstrObjJson)) 
        {
            iRet = ADM_ECODE_NEW_JSON_OBJECT_FAILED;
            log_e("Failed to new totalTime json object");
            break;
        }
        iRet = json_object_object_add(pstTransProgressJson, ADM_TPS_TRANSECU_TOTALTIME, pstrObjJson);
        if (ADM_IS_ERR(iRet))
        {
            (void)json_object_put(pstrObjJson);
            iRet = ADM_ECODE_JSON_ADD_INT_FAILED;
            log_e("Failed to add totalTime json object");
            break;
        }
        
        //!< add remainTime
        pstrObjJson = json_object_new_int(pstTransProgress->remainTime);
        if (ADM_IS_NULL(pstrObjJson)) 
        {
            iRet = ADM_ECODE_NEW_JSON_OBJECT_FAILED;
            log_e("Failed to new remainTime json object");
            break;
        }
        iRet = json_object_object_add(pstTransProgressJson, ADM_TPS_TRANSECU_REMAINTIME, pstrObjJson);
        if (ADM_IS_ERR(iRet))
        {
            (void)json_object_put(pstrObjJson);
            iRet = ADM_ECODE_JSON_ADD_INT_FAILED;
            log_e("Failed to add remainTime json object");
            break;
        }

        //!< add saveBytes
        pstrObjJson = json_object_new_int64(pstTransProgress->saveBytes);
        if (ADM_IS_NULL(pstrObjJson)) 
        {
            iRet = ADM_ECODE_NEW_JSON_OBJECT_FAILED;
            log_e("Failed to new saveBytes json object");
            break;
        }
        iRet = json_object_object_add(pstTransProgressJson, ADM_TPS_TRANSECU_SAVEBYTES, pstrObjJson);
        if (ADM_IS_ERR(iRet))
        {
            (void)json_object_put(pstrObjJson);
            iRet = ADM_ECODE_JSON_ADD_INT_FAILED;
            log_e("Failed to add saveBytes json object");
            break;
        }

        //!< add totalBytes
        pstrObjJson = json_object_new_int64(pstTransProgress->totalBytes);
        if (ADM_IS_NULL(pstrObjJson)) 
        {
            iRet = ADM_ECODE_NEW_JSON_OBJECT_FAILED;
            log_e("Failed to new totalBytes json object");
            break;
        }
        iRet = json_object_object_add(pstTransProgressJson, ADM_TPS_TRANSECU_TOTALBYTES, pstrObjJson);
        if (ADM_IS_ERR(iRet))
        {
            (void)json_object_put(pstrObjJson);
            iRet = ADM_ECODE_JSON_ADD_INT_FAILED;
            log_e("Failed to add totalBytes json object");
            break;
        }

        //!< json to string
        *ppcTransProgressJsonChar = adm_strdup(json_object_to_json_string(pstTransProgressJson));
        if (ADM_IS_NULL(*ppcTransProgressJsonChar))
        {
            log_e("Failed to json to string");
            iRet = ADM_ECODE_JSON_TO_STRING_FAILED;
            break;
        }
    } while(FALSE);

    if (NULL != pstTransProgressJson)
    {
        (void)json_object_put(pstTransProgressJson);
    }

    return iRet;
}


/*!
 *******************************************************************************
 * 反序列化json字符串到分包进度结构信息.
 *
 * \note
 * 
 * \param[in]   pcTransProgressJsonChar  分包进度信息的json字符串
 * \param[out]  pstTransProgress         分包进度信息
 * \retval      D32S
 *******************************************************************************
 */
static D32S __adm_tps_deJsonChar2TransProgressInfo
(
    DCHAR                   *pcTransProgressJsonChar,
    ADM_TPS_TRANSPROGRESS_T *pstTransProgress
)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    json_object *jsonRoot = NULL;

    if (ADM_IS_NULL(pcTransProgressJsonChar) || ADM_IS_NULL(pstTransProgress))
    {
        log_e("Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }
    
    do
    {
        jsonRoot = json_tokener_parse(pcTransProgressJsonChar);
        if (ADM_IS_NULL(jsonRoot)) 
        {
            iRet = ADM_ECODE_STRING_TO_JSON_FAILED;
            log_e("Failed string to json, string[%s]", pcTransProgressJsonChar);
            break;
        }

        //!< get speed
        iRet = adm_com_getJsonIntVal(jsonRoot, ADM_TPS_TRANSECU_SPEED, &pstTransProgress->speed);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to get json value, key[%s], err[%d]", ADM_TPS_TRANSECU_SPEED, iRet);
            break;
        }

        //!< get totalTime
        iRet = adm_com_getJsonIntVal(jsonRoot, ADM_TPS_TRANSECU_TOTALTIME, &pstTransProgress->totalTime);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to get json value, key[%s], err[%d]", ADM_TPS_TRANSECU_TOTALTIME, iRet);
            break;
        }

        //!< get remainTime
        iRet = adm_com_getJsonIntVal(jsonRoot, ADM_TPS_TRANSECU_REMAINTIME, &pstTransProgress->remainTime);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to get json value, key[%s], err[%d]", ADM_TPS_TRANSECU_REMAINTIME, iRet);
            break;
        }

        //!< get saveBytes
        iRet = adm_com_getJsonInt64Val(jsonRoot, ADM_TPS_TRANSECU_SAVEBYTES, &pstTransProgress->saveBytes);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to get json value, key[%s], err[%d]", ADM_TPS_TRANSECU_SAVEBYTES, iRet);
            break;
        }

        //!< get totalBytes
        iRet = adm_com_getJsonInt64Val(jsonRoot, ADM_TPS_TRANSECU_TOTALBYTES, &pstTransProgress->totalBytes);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to get json value, key[%s], err[%d]", ADM_TPS_TRANSECU_TOTALBYTES, iRet);
            break;
        }
    } while (FALSE);

    if (ADM_IS_ERR(iRet))
    {
        pstTransProgress->speed      = 0;
        pstTransProgress->totalTime  = 0;
        pstTransProgress->remainTime = 0;
        pstTransProgress->saveBytes  = 0;
        pstTransProgress->totalBytes = 0;
    }

    if (NULL != jsonRoot)
    {
        (void)json_object_put(jsonRoot);
    }

    return iRet;
}


/*!
 *******************************************************************************
 * 更新一个传输的零件ecu存储位置信息到KV中待传输ecu信息.
 *
 * \note
 * 
 * \param[inout] jsonEcuArrayIObj   transEcuInfo[i] json 句柄
 * \param[in]    ecuReqId           零件诊断请求ID
 * \param[in]    ecuSwDid           零件升级对象软件版本did
 * \param[in]    bIsRollback        是否是回滚包
 * \param[in]    pkgPath            ecu传输完成受控端存储路径
 * \retval       D32S
 *******************************************************************************
 */
static D32S __adm_tps_updateEcuPkgPathInfo
(
    json_object *jsonEcuArrayIObj,
    DCHAR       *ecuReqId, 
    DCHAR       *ecuSwDid,
    DBOOL        bIsRollback,
    DCHAR       *pkgPath 
)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    json_object *jsonSrcDstPkg = NULL;
    json_object *stringObject = NULL;


    if (ADM_IS_NULL(jsonEcuArrayIObj) || ADM_IS_NULL(ecuReqId) 
        || ADM_IS_NULL(ecuSwDid) || ADM_IS_NULL(pkgPath))
    {
        log_e("Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }

    do
    {
        //!< 找对象 Json(transEcuInfo[i][srcPkg/dstPkg])
        if (TRUE == bIsRollback)
        {
            //!< 判断 Json(transEcuInfo[i][srcPkg]) srcPkg 是否存在
            if (!json_object_object_get_ex(jsonEcuArrayIObj, ADM_TPS_TRANSECU_SRCPKG, &jsonSrcDstPkg))
            {
                iRet = ADM_ECODE_DOWN_RESOURCE_NOFIND;
                log_i( "Failed to get json value, key[%s]", ADM_TPS_TRANSECU_SRCPKG);
                break;
            }
        }
        else
        {
            //!< 判断 Json(transEcuInfo[i][dstPkg]) dstPkg 是否存在
            if (!json_object_object_get_ex(jsonEcuArrayIObj, ADM_TPS_TRANSECU_DSTPKG, &jsonSrcDstPkg))
            {
                iRet = ADM_ECODE_DOWN_RESOURCE_NOFIND;
                log_i( "Failed to get json value, key[%s]", ADM_TPS_TRANSECU_DSTPKG);
                break;
            }
        }

        //!< 删除 Json(transEcuInfo[i][srcPkg/dstPkg][filePath]) filePath
        json_object_object_del(jsonSrcDstPkg, ADM_TPS_TRANSECU_FILEPATH);
        
        //!< 添加 Json(transEcuInfo[i][srcPkg/dstPkg][filePath]) filePath
        stringObject = json_object_new_string(pkgPath);
        if (ADM_IS_NULL(stringObject)) 
        {
            iRet = ADM_ECODE_NEW_JSON_OBJECT_FAILED;
            log_e("Failed to new filePath json object, err[%d]", iRet);
            break;
        }
        iRet = json_object_object_add(jsonSrcDstPkg, ADM_TPS_TRANSECU_FILEPATH, stringObject);
        if (ADM_IS_ERR(iRet))
        {
            iRet = ADM_ECODE_JSON_ADD_STRING_FAILED;
            log_e("Failed to add filePath json object, err[%d]", iRet);
            (void)json_object_put(stringObject);
            break;
        }
    } while (FALSE);

    return iRet;
}


/*!
 *******************************************************************************
 * 更新一个传输的ecu存储位置信息到KV中待传输ecu信息.
 *
 * \note
 * 
 * \param[in]  ecuReqId      零件诊断请求ID
 * \param[in]  ecuSwDid      零件升级对象软件版本did
 * \param[in]  bIsRollback   是否是回滚包
 * \param[in]  pkgPath       ecu传输完成受控端存储路径
 * \param[inout] jsonRoot    待下载ecu json 句柄
 * \retval       D32S
 *******************************************************************************
 */
static D32S __adm_tps_updateEcuPkgPath2TransEcuInfo
(
    DCHAR       *ecuReqId, 
    DCHAR       *ecuSwDid,
    DBOOL        bIsRollback,
    DCHAR       *pkgPath,
    json_object *jsonRoot
)
{
    D32S i = 0;
    D32S iRet = ADM_ECODE_SUCCESSED;
    D32S iEcuListNum = 0;
    json_type    jsonType = json_type_null;
    json_object *jsonTransEcus = NULL;
    json_object *jsonArrayIndexObj = NULL;
    DCHAR *kvReqId = NULL; 
    DCHAR *kvSwDid = NULL;

    if (ADM_IS_NULL(ecuReqId) || ADM_IS_NULL(ecuSwDid) 
        || ADM_IS_NULL(pkgPath) || ADM_IS_NULL(jsonRoot))
    {
        log_e("Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }

    do
    {
        //!< Json transEcuInfo
        if (!json_object_object_get_ex(jsonRoot, ADM_TPS_TRANSECU_TRANSECUINFO, &jsonTransEcus))
        {
            iRet = ADM_ECODE_JSON_GET_KEY_FAILED;
            log_e( "Failed to get json value, key[%s]", ADM_TPS_TRANSECU_TRANSECUINFO);
            break;
        }
        
        jsonType = json_object_get_type(jsonTransEcus);
        if (jsonType != json_type_array)
        {
            iRet = ADM_ECODE_JSON_KEY_TYPE_ERROR;
            log_e( "Json key[%s] type error", ADM_TPS_TRANSECU_TRANSECUINFO);
            break;
        }
        
        iEcuListNum = (D32S)json_object_array_length(jsonTransEcus);
        if (0 >= iEcuListNum)
        {
            iRet = ADM_ECODE_DOWN_RESOURCE_NOFIND;
            log_i("Failed to get transEcuInfo array length[%d], err[%d]", iEcuListNum, iRet);
            break;
        }

        //!< 3. 找对应的集合包 
        for (i = 0; i < iEcuListNum; i++)
        {
            jsonArrayIndexObj = json_object_array_get_idx(jsonTransEcus, (D32U)i);
            if (ADM_IS_NULL(jsonArrayIndexObj)) 
            {
                iRet = ADM_ECODE_JSON_GET_ARRAYOBJECT_FAILED;
                log_e( "Failed to backtracking Json array, arrayIndex[%d]", i);
                break;
            }

            //!< 解析Json(transEcuInfo)[i]数组中的一个成员
            kvReqId = adm_com_getJsonStrVal(jsonArrayIndexObj, ADM_COM_AIR_PROTOCOL_FIELD_ECUREQID);
            if (ADM_IS_NULL(kvReqId))
            {
                iRet = ADM_ECODE_JSON_GET_KEY_FAILED;
                log_e( "Failed to json value, key[%s], err[%d]", ADM_COM_AIR_PROTOCOL_FIELD_ECUREQID, iRet);
                break;
            }
            
            kvSwDid = adm_com_getJsonStrVal(jsonArrayIndexObj, ADM_COM_AIR_PROTOCOL_FIELD_ECUSWDID);
            if (ADM_IS_NULL(kvSwDid))
            {
                iRet = ADM_ECODE_JSON_GET_KEY_FAILED;
                log_e( "Failed to json value, key[%s], err[%d]", ADM_COM_AIR_PROTOCOL_FIELD_ECUSWDID, iRet);
                break;
            }
            log_i("reqId[%s], eswDid[%s]", kvReqId, kvSwDid);

            //!< 判断是否是对应的 ecu 
            if ((0 != adm_strcasecmp(ecuReqId, kvReqId)) || (0 != adm_strcasecmp(ecuSwDid, kvSwDid)))
            {
                adm_free(kvReqId);
                adm_free(kvSwDid);
                continue;
            }
            adm_free(kvReqId);
            adm_free(kvSwDid);
            log_d("Find wait to update trans ecu pkg info.");

            //!< 更新该对象的filePath (transEcuInfo[i][srcPkg/dstPkg][filePath])
            iRet = __adm_tps_updateEcuPkgPathInfo(jsonArrayIndexObj, ecuReqId, ecuSwDid, bIsRollback, pkgPath);
            if (ADM_IS_ERR(iRet))
            {
                iRet = ADM_ECODE_JSON_ADD_STRING_FAILED;
                log_e("Failed to update trans ecu pkg path json object, err[%d]", iRet);
                break;
            }
            
            log_d("Find and update trans ecu pkg path json object info ok.");
            break;
        }

        //!< 未找到则报错
        if (i == iEcuListNum)
        {
            iRet = ADM_ECODE_DOWN_RESOURCE_NOFIND;
            log_i("Failed to find trans ecu pkg path info err[%d]", iRet);
        }

        log_i("update trans ecu package path info ok.");
    } while (FALSE);

    return iRet;
}


#if 0
#endif

/*!
 *******************************************************************************
 * 从KV中获取tps分包任务信息.
 *
 * \note
 *
 * \param[in]     pstOtaKvHandle  模块kv存储句柄
 * \param[out]    pstDownTaskInfo 下载任务信息
 * \retval        D32S
 *******************************************************************************
 */
static D32S __adm_tps_getTransTaskInfoFromKV
(
    KvStorage_t            *pstOtaKvHandle, 
    ADM_DPS_DOWNTSAKINFO_T *pstDownTaskInfo
)
{
    D32S iRet = ADM_ECODE_FAILED;
    DBOOL bIsOk = FALSE;
    size_t valLength = 0;
    DCHAR *pcDownTaskInfo = NULL;
    ADM_KV_STORAGE_VALUE_TYPE_E eValType;

    if (ADM_IS_NULL(pstOtaKvHandle) || ADM_IS_NULL(pstDownTaskInfo))
    {
        log_e( "Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }

    //!< stDownTaskInfo 
    bIsOk = adm_com_kvstorage_get_string(pstOtaKvHandle, ADM_TPS_TRANSTASKINFO_KV_KEY, 
                            &pcDownTaskInfo, &valLength, &eValType);
    if (!bIsOk)
    {
        log_e("Failed to get adm_com_kvstorage_get_string, key[%s]", ADM_TPS_TRANSTASKINFO_KV_KEY);
        return ADM_ECODE_KV_GET_STRING_FAILED;
    }

    iRet = adm_dps_deJsonChar2DownTaskInfo(pcDownTaskInfo, pstDownTaskInfo);
    if (ADM_IS_ERR(iRet))
    {
        adm_free(pcDownTaskInfo);
        log_e("Failed to de transfer task info from kv, err[%d]", iRet);
        return iRet;
    }
    adm_free(pcDownTaskInfo);

    return ADM_ECODE_SUCCESSED;
}

/*!
 *******************************************************************************
 * 从KV中获取dps待下载ecu信息.
 *
 * \note
 *
 * \param[in]     pstOtaKvHandle      模块kv存储句柄
 * \param[out]    pstTransferECUList  待下载ecu信息
 * \retval        D32S
 *******************************************************************************
 */
static D32S __adm_tps_getTransEcuInfoFromKV
(
    KvStorage_t                *pstOtaKvHandle, 
    ADM_TPS_TRANSFER_ECULIST_T *pstTransferECUList
)
{
    D32S iRet = ADM_ECODE_FAILED;
    DBOOL bIsOk = FALSE;
    size_t valLength = 0;
    DCHAR *pcTransECUInfo = NULL;
    ADM_KV_STORAGE_VALUE_TYPE_E eValType;

    if (ADM_IS_NULL(pstOtaKvHandle) || ADM_IS_NULL(pstTransferECUList))
    {
        log_e( "Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }

    //!< stTransferECUList 
    bIsOk = adm_com_kvstorage_get_string(pstOtaKvHandle, ADM_TPS_TRANSECUINFO_KV_KEY, 
                            &pcTransECUInfo, &valLength, &eValType);
    if (!bIsOk)
    {
        log_e("Failed to get adm_com_kvstorage_get_string, key[%s]", ADM_TPS_TRANSECUINFO_KV_KEY);
        return ADM_ECODE_KV_GET_STRING_FAILED;
    }
    log_i("===> Get pcTransECUInfo[%s]", pcTransECUInfo);

    iRet = __adm_tps_deJsonChar2TransEcuInfo(pcTransECUInfo, pstTransferECUList);
    if (ADM_IS_ERR(iRet))
    {
        adm_free(pcTransECUInfo);
        log_e("Failed to de transfer ecu info from kv, err[%d]", iRet);
        return iRet;
    }
    adm_free(pcTransECUInfo);

    return ADM_ECODE_SUCCESSED;
}

#if 0
#endif


/*!
 *******************************************************************************
 * 设置tps分包任务信息到KV中.
 *
 * \note
 *
 * \param[in]     pstOtaKvHandle  模块kv存储句柄
 * \param[in]     pstDownTaskInfo 分包任务信息
 * \retval        D32S
 *******************************************************************************
 */
static D32S __adm_tps_setTransTaskInfo2KV
(
    KvStorage_t            *pstOtaKvHandle, 
    ADM_DPS_DOWNTSAKINFO_T *pstDownTaskInfo
)
{
    D32S iRet = ADM_ECODE_FAILED;
    DBOOL bIsOk = FALSE;
    DCHAR *pcDownTaskInfo = NULL;

    if (ADM_IS_NULL(pstOtaKvHandle) || ADM_IS_NULL(pstDownTaskInfo))
    {
        log_e("Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }

    iRet = adm_dps_enDownTaskInfo2JsonChar(pstDownTaskInfo, &pcDownTaskInfo);
    if (ADM_IS_ERR(iRet))
    {
        log_e("Failed to en transfer task info to char, err[%d]", iRet);
        return iRet;
    }
    log_i("===> transTaskInfo[%s]", pcDownTaskInfo);

    //!< stDownTaskInfo 
    bIsOk = adm_com_kvstorage_set_string(pstOtaKvHandle, ADM_TPS_TRANSTASKINFO_KV_KEY, 
                                 pcDownTaskInfo, ADM_KV_STORAGE_VALUE_TYPE_STRING);
    if (!bIsOk)
    {
        adm_free(pcDownTaskInfo);
        log_e("Failed to set adm_com_kvstorage_set_string, key[%s]", ADM_TPS_TRANSTASKINFO_KV_KEY);
        return ADM_ECODE_KV_SET_STRING_FAILED;
    }
    adm_free(pcDownTaskInfo);

    return ADM_ECODE_SUCCESSED;
}

/*!
 *******************************************************************************
 * 设置tps待分包ecu信息到KV中.
 *
 * \note
 *
 * \param[in]     pstOtaKvHandle      模块kv存储句柄
 * \param[in]     pstTransferECUList  待分包ecu信息
 * \retval        D32S
 *******************************************************************************
 */
static D32S __adm_tps_setTransEcuInfo2KV
(
    KvStorage_t                *pstOtaKvHandle, 
    ADM_TPS_TRANSFER_ECULIST_T *pstTransferECUList
)
{
    D32S iRet = ADM_ECODE_FAILED;
    DBOOL bIsOk = FALSE;
    DCHAR *pcTransEcuInfos = NULL;

    if (ADM_IS_NULL(pstOtaKvHandle) || ADM_IS_NULL(pstTransferECUList))
    {
        log_e( "Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }

    iRet = __adm_tps_enTransEcuInfo2JsonChar(pstTransferECUList, &pcTransEcuInfos);
    if (ADM_IS_ERR(iRet))
    {
        log_e("Failed to en transfer ecu info to char, err[%d]", iRet);
        return iRet;
    }
    log_i("===> pcTransEcuInfos[%s]", pcTransEcuInfos);

    //!< stTransferECUList 
    bIsOk = adm_com_kvstorage_set_string(pstOtaKvHandle, ADM_TPS_TRANSECUINFO_KV_KEY, 
                                 pcTransEcuInfos, ADM_KV_STORAGE_VALUE_TYPE_STRING);
    if (!bIsOk)
    {
        adm_free(pcTransEcuInfos);
        log_e("Failed to set adm_com_kvstorage_set_string, key[%s]", ADM_TPS_TRANSECUINFO_KV_KEY);
        return ADM_ECODE_KV_SET_STRING_FAILED;
    }
    adm_free(pcTransEcuInfos);

    return iRet;
}

#if 0
#endif

/*!
 *******************************************************************************
 * 将KV中存储的待分包ecu进度和结果信息清除.
 *
 * \note 清空位置为 0 - toIndex
 *
 * \param[in]     pstOtaKvHandle      模块kv存储句柄
 * \param[in]     pstTransferECUList  待分包ecu信息
 * \param[in]     toIndex             清空结束索引位置
 * \retval        D32S
 *******************************************************************************
 */
static D32S __adm_tps_cleanTransProgressAndResultFromKV
(
    KvStorage_t                *pstOtaKvHandle, 
    ADM_TPS_TRANSFER_ECULIST_T *pstTransferECUList,
    D32U                        toIndex
)
{
    D32S i = 0;
    DBOOL bIsOk = FALSE;
    DCHAR szTransKVKey[ADM_COM_FILEPATH_LEN] = {0};

    if (ADM_IS_NULL(pstOtaKvHandle) || ADM_IS_NULL(pstTransferECUList))
    {
        log_e( "Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }

    //!< 保护结束索引位置
    if (toIndex > pstTransferECUList->num)
    {
        toIndex = pstTransferECUList->num;
    }
    
    for (i = 0; (D32U)i < toIndex; i++)
    {
        (void)adm_memzero(&szTransKVKey, sizeof(szTransKVKey));
        (void)adm_snprintf(szTransKVKey, sizeof(szTransKVKey), "%s%s_%s", ADM_TPS_TRANSPROGRESS_KV_KEY_PREFIX,
                     pstTransferECUList->pstEcuPkg[i].ecuReqid, 
                     pstTransferECUList->pstEcuPkg[i].ecuSwDid);
        log_d("Trans ecu part package progress key[%s].", szTransKVKey);

        //!< remove down progress from kv
        bIsOk = adm_com_kvstorage_remove_key(pstOtaKvHandle, szTransKVKey);
        if (!bIsOk)
        {
            log_e("Failed to remove trans progress info from KV, key[%s], err[%d]", 
                    szTransKVKey, ADM_ECODE_KV_REMOVE_KEY_FAILED);
        }

        (void)adm_memzero(&szTransKVKey, sizeof(szTransKVKey));
        (void)adm_snprintf(szTransKVKey, sizeof(szTransKVKey), "%s%s_%s", ADM_TPS_TRANSRESULT_KV_KEY_PREFIX,
                     pstTransferECUList->pstEcuPkg[i].ecuReqid, 
                     pstTransferECUList->pstEcuPkg[i].ecuSwDid);
        log_d("Trans ecu part package result key[%s].", szTransKVKey);

        //!< remove down result from kv
        bIsOk = adm_com_kvstorage_remove_key(pstOtaKvHandle, szTransKVKey);
        if (!bIsOk)
        {
            log_e("Failed to remove trans result info from KV, key[%s], err[%d]", 
                    szTransKVKey, ADM_ECODE_KV_REMOVE_KEY_FAILED);
        }
    }

    return ADM_ECODE_SUCCESSED;
}


/*!
 *******************************************************************************
 * 清除历史存储的分包相关记录.
 *
 * \note
 *
 * \param[in]     pstOtaKvHandle  模块kv存储句柄
 * \retval        D32S
 *******************************************************************************
 */
static D32S __adm_tps_cleanTransHistoryKVFromKV
(
    KvStorage_t *pstOtaKvHandle
)
{
    D32S iRet = ADM_ECODE_FAILED;
    ADM_TPS_TRANSFER_ECULIST_T stTransferECUList = {0};

    if (ADM_IS_NULL(pstOtaKvHandle))
    {
        log_e( "Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }

    //!< 先读出历史 stTransferECUList, 删除分包进度/结果，再删除 stTransferECUList
    iRet = __adm_tps_getTransEcuInfoFromKV(pstOtaKvHandle, &stTransferECUList);
    if (ADM_IS_ERR(iRet))
    {
        log_e("Failed to get down ecu info from kv, err[%d]", iRet);
        return iRet;
    } 
    (void)__adm_tps_cleanTransProgressAndResultFromKV(pstOtaKvHandle, &stTransferECUList, stTransferECUList.num);
    (void)adm_com_kvstorage_remove_key(pstOtaKvHandle, ADM_TPS_TRANSTASKINFO_KV_KEY);
    (void)adm_com_kvstorage_remove_key(pstOtaKvHandle, ADM_TPS_TRANSECUINFO_KV_KEY);

    //!< 清空临时数据 stTransferECUList
    adm_tps_delTransferEcuPackageList(stTransferECUList.num, &stTransferECUList.pstEcuPkg);

    return ADM_ECODE_SUCCESSED;
}


#if 0
#endif

/*!
 *******************************************************************************
 * 从KV中获取一个ecu分包结果信息.
 *
 * \note 包的 pcTransRstkeyID = ecuReqid_ecuSwDid
 *
 * "transRst_pkgName": {
 *     "result": 0,
 *     "failReason": "ok",
 *     "transState": 10
 * }
 *
 * \param[in]     pstOtaKvHandle  模块kv存储句柄
 * \param[in]     pcTransRstkeyID 分包结果keyId
 * \param[inout]  pstResult       分包结果信息
 * \retval        D32S
 *******************************************************************************
 */
static D32S adm_tps_getTransResultFromKV
(
    KvStorage_t           *pstOtaKvHandle, 
    const DCHAR           *pcTransRstkeyID, 
    ADM_TPS_TRANSRESULT_T *pstResult
)
{
    D32S iRet = ADM_ECODE_FAILED;
    DBOOL bIsOk = FALSE;
    size_t valLength = 0;
    DCHAR *pcTransResultInfo = NULL;
	ADM_KV_STORAGE_VALUE_TYPE_E eValType;
    DCHAR szTransRstKey[ADM_COM_FILEPATH_LEN] = {0};

    if (ADM_IS_NULL(pstOtaKvHandle) || ADM_IS_NULL(pcTransRstkeyID) || ADM_IS_NULL(pstResult))
    {
        log_e("Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }

    (void)adm_snprintf(szTransRstKey, sizeof(szTransRstKey), "%s%s", ADM_TPS_TRANSRESULT_KV_KEY_PREFIX, pcTransRstkeyID);
    log_d("Get trans ecu result key[%s].", szTransRstKey);

    //!< pstResult 
    bIsOk = adm_com_kvstorage_get_string(pstOtaKvHandle, szTransRstKey, 
                            &pcTransResultInfo, &valLength, &eValType);
    if (!bIsOk)
    {
        log_e("Failed to get adm_com_kvstorage_get_string, key[%s]", szTransRstKey);
        return ADM_ECODE_KV_GET_STRING_FAILED;
    }

    iRet = __adm_tps_deJsonChar2TransResultInfo(pcTransResultInfo, pstResult);
    if (ADM_IS_ERR(iRet))
    {
        adm_free(pcTransResultInfo);
        log_e("Failed to de trans result info from kv[%s], err[%d]", pcTransResultInfo, iRet);
        return iRet;
    }
    adm_free(pcTransResultInfo);

    return ADM_ECODE_SUCCESSED;
}

/*!
 *******************************************************************************
 * 设置一个ecu分包结果信息到KV中.
 *
 * \note 包的 pcTransRstkeyID = ecuReqid_ecuSwDid
 *  
 * "transRst_pkgName": {
 *     "result": 0,
 *     "failReason": "ok",
 *     "transState": 10
 * }
 *
 * \param[in]     pcTransRstkeyID 分包结果keyId
 * \param[in]     pstResult       分包结果信息
 * \retval        D32S
 *******************************************************************************
 */
D32S adm_tps_setTransResult2KV
(
    const DCHAR           *pcTransRstkeyID, 
    ADM_TPS_TRANSRESULT_T *pstResult
)
{
    D32S iRet = ADM_ECODE_FAILED;
    DBOOL bIsOk = FALSE;
    DCHAR *pcTransResult = NULL;
    DCHAR szTransRstKey[ADM_COM_FILEPATH_LEN] = {0};
    KvStorage_t *pstOtaKvHandle = NULL;

    if (ADM_IS_NULL(pcTransRstkeyID) || ADM_IS_NULL(pstResult))
    {
        log_e("Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }
    
    pstOtaKvHandle = adm_core_service_getBusinessDatabaseDb();
    if (ADM_IS_NULL(pstOtaKvHandle))
    {
        log_e("Failed to get ota kv handle, err[%d]", ADM_ECODE_KV_OPEN_FAILED);
        return ADM_ECODE_KV_OPEN_FAILED;
    }

    iRet = __adm_tps_enTransResult2JsonChar(pstResult, &pcTransResult);
    if (ADM_IS_ERR(iRet))
    {
        log_e("Failed to en trans result info to char, RstKeyID[%s] err[%d]", pcTransRstkeyID, iRet);
        return iRet;
    }
    log_i("===> TransResult[%s]", pcTransResult);

    (void)adm_snprintf(szTransRstKey, sizeof(szTransRstKey), "%s%s", ADM_TPS_TRANSRESULT_KV_KEY_PREFIX, pcTransRstkeyID);
    log_d("Trans ecu result key[%s].", szTransRstKey);

    //!< downRst_pkgName 
    bIsOk = adm_com_kvstorage_set_string(pstOtaKvHandle, szTransRstKey, 
                                 pcTransResult, ADM_KV_STORAGE_VALUE_TYPE_STRING);
    if (!bIsOk)
    {
        adm_free(pcTransResult);
        log_e("Failed to set adm_com_kvstorage_set_string, key[%s]", szTransRstKey);
        return ADM_ECODE_KV_SET_STRING_FAILED;
    }
    adm_free(pcTransResult);

    return ADM_ECODE_SUCCESSED;
}


/*!
 *******************************************************************************
 * 从KV中获取一个ecu分包进度信息.
 *
 * \note 包的 pcTransProgkeyID = ecuReqid_ecuSwDid
 * 
 * "transProg_pkgName": {
 *     "speed": 60,
 *     "totalTime": 120,
 *     "remainTime": 100,
 *     "saveBytes": 10570,
 *     "totalBytes": 105706192
 * }
 *
 * \param[in]     pstOtaKvHandle   模块kv存储句柄
 * \param[in]     pcTransProgkeyID 分包进度keyId
 * \param[inout]  pstProgress      分包进度信息
 * \retval        D32S
 *******************************************************************************
 */
static D32S adm_tps_getTransProgressFromKV
(
    KvStorage_t             *pstOtaKvHandle, 
    const DCHAR             *pcTransProgkeyID, 
    ADM_TPS_TRANSPROGRESS_T *pstProgress
)
{
    D32S iRet = ADM_ECODE_FAILED;
    DBOOL bIsOk = FALSE;
    size_t valLength = 0;
    DCHAR *pcTransProgressInfo = NULL;
	ADM_KV_STORAGE_VALUE_TYPE_E eValType;
    DCHAR szTransProgKey[ADM_COM_FILEPATH_LEN] = {0};

    if (ADM_IS_NULL(pstOtaKvHandle) || ADM_IS_NULL(pcTransProgkeyID) || ADM_IS_NULL(pstProgress))
    {
        log_e("Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }

    (void)adm_snprintf(szTransProgKey, sizeof(szTransProgKey), "%s%s", ADM_TPS_TRANSPROGRESS_KV_KEY_PREFIX, pcTransProgkeyID);
    log_d("Get trans ecu progress key[%s].", szTransProgKey);

    //!< pstProgress 
    bIsOk = adm_com_kvstorage_get_string(pstOtaKvHandle, szTransProgKey, 
                            &pcTransProgressInfo, &valLength, &eValType);
    if (!bIsOk)
    {
        log_e("Failed to get adm_com_kvstorage_get_string, key[%s]", szTransProgKey);
        return ADM_ECODE_KV_GET_STRING_FAILED;
    }

    iRet = __adm_tps_deJsonChar2TransProgressInfo(pcTransProgressInfo, pstProgress);
    if (ADM_IS_ERR(iRet))
    {
        adm_free(pcTransProgressInfo);
        log_e("Failed to de trans progress info from kv[%s], err[%d]", szTransProgKey, iRet);
        return iRet;
    }
    adm_free(pcTransProgressInfo);

    return ADM_ECODE_SUCCESSED;
}

/*!
 *******************************************************************************
 * 设置一个ecu分包进度信息到KV中.
 *
 * \note 包的 pcDownRstkeyID = ecuReqid_ecuSwDid
 *
 * "transProg_pkgName": {
 *     "speed": 60,
 *     "totalTime": 120,
 *     "remainTime": 100,
 *     "saveBytes": 10570,
 *     "totalBytes": 105706192
 * }
 *
 * \param[in]     pcTransProgkeyID 分包进度keyId
 * \param[in]     pstProgress      分包进度信息
 * \retval        D32S
 *******************************************************************************
 */
D32S adm_tps_setTransProgress2KV
(
    const DCHAR             *pcTransProgkeyID, 
    ADM_TPS_TRANSPROGRESS_T *pstProgress
)
{
    D32S iRet = ADM_ECODE_FAILED;
    DBOOL bIsOk = FALSE;
    DCHAR *pcTransProgress = NULL;
    DCHAR szTransProgKey[ADM_COM_FILEPATH_LEN] = {0};
    KvStorage_t *pstOtaKvHandle = NULL;

    if (ADM_IS_NULL(pcTransProgkeyID) || ADM_IS_NULL(pstProgress))
    {
        log_e("Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }

    pstOtaKvHandle = adm_core_service_getBusinessDatabaseDb();
    if (ADM_IS_NULL(pstOtaKvHandle))
    {
        log_e("Failed to get ota kv handle, err[%d]", ADM_ECODE_KV_OPEN_FAILED);
        return ADM_ECODE_KV_OPEN_FAILED;
    }

    iRet = __adm_tps_enTransProgress2JsonChar(pstProgress, &pcTransProgress);
    if (ADM_IS_ERR(iRet))
    {
        log_e("Failed to en trans result info to char, ProgKeyID[%s] err[%d]", pcTransProgkeyID, iRet);
        return iRet;
    }
    log_i("===> TransProgress[%s]", pcTransProgress);

    (void)adm_snprintf(szTransProgKey, sizeof(szTransProgKey), "%s%s", ADM_TPS_TRANSPROGRESS_KV_KEY_PREFIX, pcTransProgkeyID);
    log_d("Trans ecu result key[%s].", szTransProgKey);

    //!< downProg_pkgName 
    bIsOk = adm_com_kvstorage_set_string(pstOtaKvHandle, szTransProgKey, 
                                 pcTransProgress, ADM_KV_STORAGE_VALUE_TYPE_STRING);
    if (!bIsOk)
    {
        adm_free(pcTransProgress);
        log_e("Failed to set adm_com_kvstorage_set_string, key[%s]", szTransProgKey);
        return ADM_ECODE_KV_SET_STRING_FAILED;
    }
    adm_free(pcTransProgress);

    return ADM_ECODE_SUCCESSED;
}

/*!
 *******************************************************************************
 * 从KV中获取tps待分包ecu进度和结果信息.
 *
 * \note
 *
 * \param[in]     pstOtaKvHandle      模块kv存储句柄
 * \param[in]     pstTransferECUList  待分包ecu信息
 * \retval        D32S
 *******************************************************************************
 */
static D32S adm_tps_getTransProgressAndResultFromKV
(
    KvStorage_t                *pstOtaKvHandle, 
    ADM_TPS_TRANSFER_ECULIST_T *pstTransferECUList
)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    D32S i = 0;
    DCHAR szTransPkgKeyId[ADM_COM_KEYID_LEN] = {0};

    if (ADM_IS_NULL(pstOtaKvHandle) || ADM_IS_NULL(pstTransferECUList))
    {
        log_e( "Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }

    for (i = 0; (D32U)i < pstTransferECUList->num; i++)
    {
        (void)adm_memzero(&szTransPkgKeyId, sizeof(szTransPkgKeyId));
        (void)adm_snprintf(szTransPkgKeyId, sizeof(szTransPkgKeyId), "%s_%s", 
                     pstTransferECUList->pstEcuPkg[i].ecuReqid, 
                     pstTransferECUList->pstEcuPkg[i].ecuSwDid);
        log_d("Trans ecu part package keyId[%s].", szTransPkgKeyId);
    
        //!< set trans progress from kv
        iRet = adm_tps_getTransProgressFromKV(pstOtaKvHandle, szTransPkgKeyId, 
                                            &pstTransferECUList->pstEcuPkg[i].stTransProcess.stProgress);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to get trans progress info from KV, keyId[%s], err[%d]", szTransPkgKeyId, iRet);
            break;
        }
    
        //!< set trans result from kv
        iRet = adm_tps_getTransResultFromKV(pstOtaKvHandle, szTransPkgKeyId, 
                                            &pstTransferECUList->pstEcuPkg[i].stTransProcess.stResult);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to get trans result info from KV, keyId[%s], err[%d]", szTransPkgKeyId, iRet);
            break;
        }
    }

    if (ADM_IS_ERR(iRet))
    {
        log_e("Failed to get trans progress or result info from kv, err[%d]", iRet);
    }

    return iRet;
}

/*!
 *******************************************************************************
 * 设置tps待分包ecu进度和结果信息到KV中.
 *
 * \note
 *
 * \param[in]     pstOtaKvHandle      模块kv存储句柄
 * \param[in]     pstTransferECUList  待分包ecu信息
 * \retval        D32S
 *******************************************************************************
 */
static D32S adm_tps_setTransProgressAndResult2KV
(
    KvStorage_t                *pstOtaKvHandle, 
    ADM_TPS_TRANSFER_ECULIST_T *pstTransferECUList
)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    D32S i = 0;
    DCHAR szTransPkgKeyId[ADM_COM_KEYID_LEN] = {0};

    if (ADM_IS_NULL(pstOtaKvHandle) || ADM_IS_NULL(pstTransferECUList))
    {
        log_e( "Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }
    for (i = 0; (D32U)i < pstTransferECUList->num; i++)
    {
        (void)adm_memzero(&szTransPkgKeyId, sizeof(szTransPkgKeyId));
        (void)adm_snprintf(szTransPkgKeyId, sizeof(szTransPkgKeyId), "%s_%s", 
                     pstTransferECUList->pstEcuPkg[i].ecuReqid, 
                     pstTransferECUList->pstEcuPkg[i].ecuSwDid);
        log_d("Trans ecu part package keyId[%s].", szTransPkgKeyId);
    
        //!< set down progress 2 kv
        iRet = adm_tps_setTransProgress2KV(szTransPkgKeyId, &pstTransferECUList->pstEcuPkg[i].stTransProcess.stProgress);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to set trans progress info to KV, keyId[%s], err[%d]", szTransPkgKeyId, iRet);
            break;
        }
    
        //!< set down result 2 kv
        iRet = adm_tps_setTransResult2KV(szTransPkgKeyId, &pstTransferECUList->pstEcuPkg[i].stTransProcess.stResult);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to set trans result info to KV, keyId[%s], err[%d]", szTransPkgKeyId, iRet);
            break;
        }
    }

    if (ADM_IS_ERR(iRet))
    {
        //!< clear err before set
        (void)__adm_tps_cleanTransProgressAndResultFromKV(pstOtaKvHandle, pstTransferECUList, (D32U)i);
        log_e("Failed to set trans progress or result info 2 kv, err[%d]", iRet);
    }

    return iRet;
}


/*!
 *******************************************************************************
 * 更新一个传输的ecu存储位置信息到KV中.
 *
 * \note 传输完成后需要把受控端的存储包的位置存储到kv中
 *
 * \param[in]  ecuReqId    零件诊断请求ID
 * \param[in]  ecuSwDid    零件升级对象软件版本did
 * \param[in]  bIsRollback 是否是回滚包
 * \param[in]  pkgPath     ecu传输完成受控端存储路径
 * \retval     D32S
 *******************************************************************************
 */
D32S adm_tps_updateTransPkgPath2TransEcuInfoKV
(
    DCHAR *ecuReqId, 
    DCHAR *ecuSwDid, 
    DBOOL  bIsRollback,
    DCHAR *pkgPath
)
{
    
    D32S iRet   = ADM_ECODE_FAILED;
    DBOOL bIsOk = FALSE;
    size_t valLength = 0;
    DCHAR *pcTransECUInfo = NULL;
    ADM_KV_STORAGE_VALUE_TYPE_E eValType;
    json_object *jsonRoot = NULL;
    KvStorage_t *pstOtaKvHandle = NULL;
    DCHAR *pcUpdateTransECUInfo = NULL;

    if (ADM_IS_NULL(ecuReqId) || ADM_IS_NULL(ecuSwDid) || ADM_IS_NULL(pkgPath))
    {
        log_e("Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }

    pstOtaKvHandle = adm_core_service_getBusinessDatabaseDb();
    if (ADM_IS_NULL(pstOtaKvHandle))
    {
        log_e("Failed to get ota kv handle.");
        return ADM_ECODE_KV_OPEN_FAILED;
    }

    //!< 1. get kv transEcuInfo 
    bIsOk = adm_com_kvstorage_get_string(pstOtaKvHandle, ADM_TPS_TRANSECUINFO_KV_KEY, 
                            &pcTransECUInfo, &valLength, &eValType);
    if (!bIsOk)
    {
        log_e("Failed to get adm_com_kvstorage_get_string, key[%s]", ADM_TPS_TRANSECUINFO_KV_KEY);
        return ADM_ECODE_KV_GET_STRING_FAILED;
    }
    log_i("===> update before trans ecu pkg[%d (isSrcPkg)][path(%s)] json[%s].", 
            bIsRollback, pkgPath, pcTransECUInfo);

    //!< 2. value 转为 json handle
    jsonRoot = json_tokener_parse(pcTransECUInfo);
    if (ADM_IS_NULL(jsonRoot)) 
    {
        adm_free(pcTransECUInfo);
        iRet = ADM_ECODE_STRING_TO_JSON_FAILED;
        log_e("Failed string to json, string[%s]", pcTransECUInfo);
        return iRet;
    }
    adm_free(pcTransECUInfo);

    iRet = __adm_tps_updateEcuPkgPath2TransEcuInfo(ecuReqId, ecuSwDid, bIsRollback, pkgPath, jsonRoot);
    if (ADM_IS_ERR(iRet))
    {
        (void)json_object_put(jsonRoot);
        log_e("Failed to en trans ecu pkg info to char, err[%d]", iRet);
        return iRet;
    }
    log_i("===> update trans ecu pkg path 2 json trans ecu info ok.");

    //!< json to string
    pcUpdateTransECUInfo = adm_strdup(json_object_to_json_string(jsonRoot));
    if (ADM_IS_NULL(pcUpdateTransECUInfo))
    {
        (void)json_object_put(jsonRoot);
        log_e("Failed to json to string");
        return ADM_ECODE_JSON_TO_STRING_FAILED;
    }
    log_i("===> updated after trans ecu Pkg path json[%s].", pcUpdateTransECUInfo);
    (void)json_object_put(jsonRoot);
    
    //!< stDownECUInfo 
    bIsOk = adm_com_kvstorage_set_string(pstOtaKvHandle, ADM_TPS_TRANSECUINFO_KV_KEY, 
                                 pcUpdateTransECUInfo, ADM_KV_STORAGE_VALUE_TYPE_STRING);
    if (!bIsOk)
    {
        adm_free(pcUpdateTransECUInfo);
        log_e("Failed to set adm_com_kvstorage_set_string, key[%s]", ADM_TPS_TRANSECUINFO_KV_KEY);
        return ADM_ECODE_KV_SET_STRING_FAILED;
    }
    adm_free(pcUpdateTransECUInfo);

    return ADM_ECODE_SUCCESSED;
}


#if 0
#endif

/*!
 *******************************************************************************
 * 从KV中获取tps分包策略信息.
 *
 * \note
 *
 * \param[inout]  pstTransferInfo 模块配置数据信息
 * \retval        D32S
 *******************************************************************************
 */
D32S adm_tps_getnTransferInfoFromKV(ADM_TPS_TRANSFERINFO_T *pstTransferInfo)
{
    D32S iRet = ADM_ECODE_FAILED;
    KvStorage_t *pstOtaKvHandle = NULL;

    if (ADM_IS_NULL(pstTransferInfo))
    {
        log_e( "Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }

    pstOtaKvHandle = adm_core_service_getBusinessDatabaseDb();
    if (ADM_IS_NULL(pstOtaKvHandle))
    {
        log_e("Failed to get ota kv handle.");
        return ADM_ECODE_KV_OPEN_FAILED;
    }

    do
    {
        //!< 分包任务信息
        iRet = __adm_tps_getTransTaskInfoFromKV(pstOtaKvHandle, &pstTransferInfo->stDownTaskInfo);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to get transfer task info from kv, err[%d]", iRet);
            break;
        }
        log_i("Get transfer task info from kv ok.");
        
        //!< 待分包ecu信息
        iRet = __adm_tps_getTransEcuInfoFromKV(pstOtaKvHandle, &pstTransferInfo->stTransferECUList);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to get transfer ecu info from kv, err[%d]", iRet);
            break;
        } 
        log_i("Get transfer ecu info from kv ok.");

        //!< 分包ecu进度/结果信息
        iRet = adm_tps_getTransProgressAndResultFromKV(pstOtaKvHandle, &pstTransferInfo->stTransferECUList);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to get transfer progress and result info from kv, err[%d]", iRet);
            break;
        } 
        log_i("Get transfer progress and result info from kv ok.");
    } while (FALSE);

    if (ADM_IS_ERR(iRet))
    {
        adm_tps_cleanTransferCacheInfo(pstTransferInfo); 
    }
    
    return iRet;
}

/*!
 *******************************************************************************
 * 设置tps传输策略信息到KV中.
 *
 * \note 
 * 
 * \param[in]  pstTransferInfo  模块配置数据信息
 * \retval     D32S
 *******************************************************************************
 */
D32S adm_tps_setTransferInfo2KV(ADM_TPS_TRANSFERINFO_T *pstTransferInfo)
{
    D32S iRet = ADM_ECODE_FAILED;
    KvStorage_t *pstOtaKvHandle = NULL;

    if (ADM_IS_NULL(pstTransferInfo))
    {
        log_e( "Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }

    do
    {
        pstOtaKvHandle = adm_core_service_getBusinessDatabaseDb();
        if (ADM_IS_NULL(pstOtaKvHandle))
        {
            iRet = ADM_ECODE_KV_OPEN_FAILED;
            log_e("Failed to get ota kv handle.");
            break;
        }

        //!< 清空之前的数据 
        iRet = __adm_tps_cleanTransHistoryKVFromKV(pstOtaKvHandle);
        if (ADM_IS_ERR(iRet))
        {
            //!< 非关键错误, 不用break, 仅记录日志
            log_i("Failed to clean transfer history kv info from kv, err[%d]", iRet);
        }
        
        //!< 分包任务信息
        iRet = __adm_tps_setTransTaskInfo2KV(pstOtaKvHandle, &pstTransferInfo->stDownTaskInfo);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to set transfer task info 2 kv, err[%d]", iRet);
            break;
        }
        log_i("Set transfer task info 2 kv ok.");
        
        //!< 待分包ecu信息
        iRet = __adm_tps_setTransEcuInfo2KV(pstOtaKvHandle, &pstTransferInfo->stTransferECUList);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to set transfer ecu info 2 kv, err[%d]", iRet);
            break;
        } 
        log_i("Set transfer ecu info 2 kv ok.");

        //!< 分包ecu进度及结果信息
        iRet = adm_tps_setTransProgressAndResult2KV(pstOtaKvHandle, &pstTransferInfo->stTransferECUList);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to set transfer ecu progress and result info 2 kv, err[%d]", iRet);
            break;
        } 
        log_i("Set transfer ecu progress and result info 2 kv ok.");
    } while (FALSE);
    
    return iRet;
}

D32S adm_tps_getIdelControlInfoFromKV(ADM_TPS_TRNASFER_IDEL_CONTROL_T* pstTransIdelControlInfo)
{
    D32S iRet = ADM_ECODE_FAILED;
    DBOOL bIsOk = FALSE;
    size_t valLength = 0;
    DCHAR* pInfo = NULL;
    ADM_KV_STORAGE_VALUE_TYPE_E eValType;
    KvStorage_t* pstOtaKvHandle = NULL;
    json_object* jsonRoot = NULL;

    if (ADM_IS_NULL(pstTransIdelControlInfo))
    {
        log_e("Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }

    pstOtaKvHandle = adm_core_service_getBusinessDatabaseDb();
    if (ADM_IS_NULL(pstOtaKvHandle))
    {
        log_e("Failed to get ota kv handle.");
        return ADM_ECODE_KV_OPEN_FAILED;
    }

    //!< stDownTaskInfo
    bIsOk = adm_com_kvstorage_get_string(pstOtaKvHandle, ADM_TPS_TRANSIDELCONTROL_KV_KEY,
                                         &pInfo, &valLength, &eValType);
    if (!bIsOk)
    {
        log_e("Failed to get adm_com_kvstorage_get_string, key[%s]", ADM_TPS_TRANSIDELCONTROL_KV_KEY);
        return ADM_ECODE_KV_GET_STRING_FAILED;
    }
    else 
    {
        (DVOID)adm_stringToInt32(pInfo, &pstTransIdelControlInfo->eIdelControl);
    }

    adm_free(pInfo);

    return iRet;
}

D32S adm_tps_setIdelControlInfo2KV(ADM_TPS_TRNASFER_IDEL_CONTROL_T *pstTransIdelControlInfo)
{
    D32S iRet = ADM_ECODE_FAILED;
    DBOOL bIsOk = FALSE;
    DCHAR aInfo[4];
    KvStorage_t *pstOtaKvHandle = NULL;

    if (ADM_IS_NULL(pstTransIdelControlInfo))
    {
        log_e("Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }

    pstOtaKvHandle = adm_core_service_getBusinessDatabaseDb();
    if (ADM_IS_NULL(pstOtaKvHandle))
    {
        log_e("Failed to get ota kv handle.");
        return ADM_ECODE_KV_OPEN_FAILED;
    }

    (DVOID)adm_memzero(aInfo, sizeof(aInfo));
    (DVOID)adm_snprintf(aInfo, sizeof(aInfo), "%d", pstTransIdelControlInfo->eIdelControl);

    //!< stDownTaskInfo
    bIsOk = adm_com_kvstorage_set_string(pstOtaKvHandle, ADM_TPS_TRANSIDELCONTROL_KV_KEY,
                                         aInfo, ADM_KV_STORAGE_VALUE_TYPE_INT32);
    if (!bIsOk)
    {
        log_e("Failed to set adm_com_kvstorage_set_string, key[%s]", ADM_TPS_TRANSTASKINFO_KV_KEY);
        return ADM_ECODE_KV_SET_STRING_FAILED;
    }

    return ADM_ECODE_SUCCESSED;
}