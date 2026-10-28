
#include "adm_integration_test.h"
#include "adm_typedefs.h"
#include "adm_memory.h"
#include "adm_file.h"
#include "adm_string.h"
#include "adm_typedefs.h"
#include "adm_com_errorCode.h"
#include "adm_com_hash.h"
#include "adm_application_errorCode.h"
#include "adm_com_lib.h"
#include "adm_list.h"
#include "adm_ocs_airProtocolSecurity.h"
#include "adm_soa.h"
#include "adm_cdm_verify_interface.h"
#include "adm_application_common.h"
#include "adm_com_kvstorage.h"
#include "adm_eim_ecuInfoManager.h"
#include "adm_ecs_kvStorage.h"
#include "adm_ecs_parsePolicy.h"
#include "adm_cfg_api.h"
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <time.h>
#include <pthread.h>






#define RETURN_IF_NULL(p, ret)  if(NULL == (p))\
                                {\
                                    return ret;\
                                }
#define RETURN_IF_NEQ(a, b, ret)    if((a) != (b))\
                                    {\
                                        return ret;\
                                    }


static DCHAR *g_testConfigFileDir = NULL;             //集成测试配置文件目录

static D32S __integrationTestParseEcuDidListInfo(const json_object          *jsonRoot, ADM_ECS_ECUDIDLIST_T *pstEcuDidList);


/*!
 *******************************************************************************
 * json获取key对应的string型value.
 * 
 * \note
 *
 * \param[in]   jsonObj 
 * \param[in]   key
 * \retval      char*
 *******************************************************************************
 */
static DCHAR* __integrationTestGetJsonStrVal(const json_object *jsonObj, const char *key)
{
    DCHAR *str = NULL;
    json_object *jsonStr = NULL;

    if (ADM_IS_NULL(jsonObj) || ADM_IS_NULL(key))
    {
        return NULL;
    }

    if (!json_object_object_get_ex(jsonObj, key, &jsonStr))
    {
        return NULL;
    }

    str = adm_strdup(json_object_get_string(jsonStr));

    return str;
}

/*!
 *******************************************************************************
 * json获取key对应的D64S型value.
 * 
 * \note
 *
 * \param[in]   JsonObj 
 * \param[in]   key
 * \param[out]  val
 * \retval      D32S 0: 成功, other: 失败
 *******************************************************************************
 */
static D32S __integrationTestGetJsonInt64Val(const json_object *jsonObj, const char *key, D64S *val)
{
    json_object* jsonStr = NULL;

    if (ADM_IS_NULL(jsonObj) || ADM_IS_NULL(key) || ADM_IS_NULL(val))
    {
        return ADM_ECODE_INVALID_PARAM;
    }

    if (!json_object_object_get_ex(jsonObj, key, &jsonStr))
    {
        return ADM_ECODE_JSON_GET_KEY_FAILED;
    }

    *val = json_object_get_int64(jsonStr);

    return ADM_ECODE_SUCCESSED;
}

/*!
 *******************************************************************************
 * 生成fileSecurityInfo结构内存
 * 
 * \note   
 *
 * \param    none
 * \retval   ADM_COM_FILESCYINFO_T*
 *******************************************************************************
 */
static ADM_COM_FILESCYINFO_T* __integrationTestNewFileSecurityInfo(void)
{
    ADM_COM_FILESCYINFO_T *pstFileSecurityInfo = NULL;

    pstFileSecurityInfo = (ADM_COM_FILESCYINFO_T *)adm_malloc(sizeof(*pstFileSecurityInfo));
    if (NULL != pstFileSecurityInfo)
    {
        (void)adm_memzero(pstFileSecurityInfo, sizeof(*pstFileSecurityInfo));
    }

    return pstFileSecurityInfo;
}

/*!
 *******************************************************************************
 * json获取key对应的D32S型value.
 * 
 * \note
 *
 * \param[in]   jsonObj 
 * \param[in]   key
 * \param[out]  val
 * \retval      D32S 0: 成功, other: 失败
 *******************************************************************************
 */
static D32S __integrationTestGetJsonIntVal(const json_object *jsonObj, const char *key, D32S *val)
{
    json_object* jsonStr = NULL;

    if (ADM_IS_NULL(jsonObj) || ADM_IS_NULL(key) || ADM_IS_NULL(val))
    {
        return ADM_ECODE_INVALID_PARAM;
    }

    if (!json_object_object_get_ex(jsonObj, key, &jsonStr)) 
    {
        return ADM_ECODE_JSON_GET_KEY_FAILED;
    }

    *val = json_object_get_int(jsonStr);
    
    return ADM_ECODE_SUCCESSED;
}

/*!
 *******************************************************************************
 * 申请文件元数据列表结构内存.
 *
 * \note 申请的为连续内存
 *
 * \param[in]  num 文件元数据列表个数
 * \retval     ADM_COM_FILEMETA_T*
 *******************************************************************************
 */
static ADM_COM_FILEMETA_T* __integrationTestNewFileMetaList(D32S num)
{
    ADM_COM_FILEMETA_T *pstFileMetaList = NULL;

    if (0 >= num)
    {
        return NULL;
    }

    pstFileMetaList = (ADM_COM_FILEMETA_T *)adm_malloc((D32U)num * sizeof(*pstFileMetaList));
    if (NULL != pstFileMetaList)
    {
        (void)adm_memzero(pstFileMetaList, (D32U)num * sizeof(*pstFileMetaList));
    }

    return pstFileMetaList;
}


/*!
 *******************************************************************************
 * 解析json中fileMetas节点.
 *
 * \note
 * 
 * \param[in]  jsonFileMetas json fileMetas node
 * \param[in]  pstFileMeta   文件元数据信息
 * \retval      D32S
 *******************************************************************************
 */
static D32S __integrationTestParseJsonFileMetas
(
    json_object        *jsonFileMetas, 
    ADM_COM_FILEMETA_T *pstFileMeta
)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    json_object *checkSum = NULL;

    if (ADM_IS_NULL(jsonFileMetas) || ADM_IS_NULL(pstFileMeta))
    {
        log_e("Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }

    iRet = __integrationTestGetJsonIntVal(jsonFileMetas, ADM_COM_AIR_PROTOCOL_FIELD_TYPE, 
                &pstFileMeta->type);
    if (ADM_IS_ERR(iRet))
    {
        log_e("Failed to get json value, key[%s], err[%d]", ADM_COM_AIR_PROTOCOL_FIELD_TYPE, iRet);
        return iRet;
    }

    iRet = __integrationTestGetJsonInt64Val(jsonFileMetas, ADM_COM_AIR_PROTOCOL_FIELD_SIZE, 
                &pstFileMeta->size);
    if (ADM_IS_ERR(iRet))
    {
        log_e("Failed to get json value, key[%s], err[%d]", ADM_COM_AIR_PROTOCOL_FIELD_SIZE, iRet);
        return iRet;
    }

    if (json_object_object_get_ex(jsonFileMetas, (const DCHAR*)ADM_COM_AIR_PROTOCOL_FIELD_CHECKSUM, &checkSum)) 
    {
        pstFileMeta->stCheckSum.value = __integrationTestGetJsonStrVal(checkSum, ADM_COM_AIR_PROTOCOL_FIELD_VALUE);
        if (ADM_IS_NULL(pstFileMeta->stCheckSum.value))
        {
            log_e("Failed to get json value, key[%s]", ADM_COM_AIR_PROTOCOL_FIELD_VALUE);
            return ADM_ECODE_JSON_GET_KEY_FAILED;
        }

        iRet = __integrationTestGetJsonIntVal(checkSum, ADM_COM_AIR_PROTOCOL_FIELD_ALGORITHM, &pstFileMeta->stCheckSum.algorithm);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to get json value, key[%s], err[%d]", ADM_COM_AIR_PROTOCOL_FIELD_ALGORITHM, iRet);
            return iRet;
        }
    }else{
        log_e("Failed to get json value, key[%s]", ADM_COM_AIR_PROTOCOL_FIELD_CHECKSUM);
        return ADM_ECODE_JSON_GET_KEY_FAILED;
    }

    return iRet;
}

/*!
 *******************************************************************************
 * 删除文件元数据列表结构内存.
 *
 * \note    释放的为连续内存
 *
 * \param[in]  num             文件元数据列表个数
 * \param[in]  pstFileMetaList 文件元数据列表信息
 * \retval     none
 *******************************************************************************
 */
void __integrationTestDelFileMetaList
(
    D32S num, 
    ADM_COM_FILEMETA_T **pstFileMetaList
)
{
    D32S i = 0;

    if ((0 >= num) || (ADM_IS_NULL(pstFileMetaList)) || (ADM_IS_NULL(*pstFileMetaList)))
    {
        return;
    }

    for (i = 0; i < num; i++)
    {
        adm_free((*pstFileMetaList)[i].stCheckSum.value);
    }
    adm_free(*pstFileMetaList);

    return;
}


/*!
 *******************************************************************************
 * 清空fileSecurityInfo对象内存信息.
 *
 * \note  不释放传入的节点最外层的内存
 *
 * \param[in]   pstFileSecurityInfo 文件安全信息结构
 * \retval      none
 *******************************************************************************
 */
static void __integrationTestCleanFileSecurityInfo(ADM_COM_FILESCYINFO_T *pstFileSecurityInfo)
{
    if (ADM_IS_NULL(pstFileSecurityInfo))
    {
        return;
    }

    adm_free(pstFileSecurityInfo->oriHash);
    adm_free(pstFileSecurityInfo->pkgKey);
    __integrationTestDelFileMetaList(pstFileSecurityInfo->fileMetaNum, 
             &pstFileSecurityInfo->pstFileMetaList);

    return;
}


/*!
 *******************************************************************************
 * 删除fileSecurityInfo对象内存信息.
 *
 * \note
 *
 * \param[in]   pstFileSecurityInfo 文件安全信息结构
 * \retval      none
 *******************************************************************************
 */
static void __integrationTestDelFileSecurityInfo(ADM_COM_FILESCYINFO_T **pstFileSecurityInfo)
{
    if (ADM_IS_NULL(pstFileSecurityInfo) || ADM_IS_NULL(*pstFileSecurityInfo))
    {
        return;
    }

    adm_free((*pstFileSecurityInfo)->oriHash);
    adm_free((*pstFileSecurityInfo)->pkgKey);
    __integrationTestDelFileMetaList((*pstFileSecurityInfo)->fileMetaNum, 
             &(*pstFileSecurityInfo)->pstFileMetaList);

    adm_free(*pstFileSecurityInfo);

    return;
}


/*!
 *******************************************************************************
 * 清空文件对象结构内存.
 *
 * \note 不释放传入的节点最外层的内存
 *
 * \param[in]  pstFileObject 文件对象信息
 * \retval     none
 *******************************************************************************
 */
static void __integrationTestCleanFileObject(ADM_COM_FILEOBJECT_T *pstFileObject)
{
    if (ADM_IS_NULL(pstFileObject))
    {
        return;
    }

    adm_free(pstFileObject->url);
    adm_free(pstFileObject->hash);
    adm_free(pstFileObject->fileName);
    adm_free(pstFileObject->filePath);
    __integrationTestDelFileSecurityInfo(&pstFileObject->pstFileSecurityInfo);
    pstFileObject->pstFileSecurityInfo = NULL;
    pstFileObject->size = 0;

    return;
}


/*!
 *******************************************************************************
 * 解析message中fileSecurityInfo信息.
 *
 * \note
 * 
 * \param[in]  jsonFileObj         json file Object node
 * \param[in]  ppstFileSecurityInfo 文件安全结构信息
 * \retval     D32S
 *******************************************************************************
 */
static D32S __integrationTestParseJsonFileSecurityInfo
(
    json_object            *jsonFileObj, 
    ADM_COM_FILESCYINFO_T **ppstFileSecurityInfo
)
{
	D32S iRet = ADM_ECODE_SUCCESSED;
	D32S i = 0;
	json_type	 jsonType = json_type_null;
	json_object *jsonFileMetas = NULL;
	json_object *jsonArrayIndexObj = NULL;
	json_object *jsonFileSecurityInfo = NULL;
	ADM_COM_FILESCYINFO_T *pstFileSecurityInfo = NULL;

	if (ADM_IS_NULL(jsonFileObj))
	{
		log_e("Invalid input param");
		return ADM_ECODE_INVALID_PARAM;
	}

	//!< 解析fileObject  中fileSecurityInfo --- 可选字段
	if (!json_object_object_get_ex(jsonFileObj, ADM_COM_AIR_PROTOCOL_FIELD_FILESCYINFO, &jsonFileSecurityInfo))
	{
		iRet = ADM_ECODE_SUCCESSED;
		log_i("Json node[%s] no key[%s]", ADM_COM_AIR_PROTOCOL_FIELD_FILEOBJECT,
					ADM_COM_AIR_PROTOCOL_FIELD_FILESCYINFO);
		return iRet;
	}
	
	//!< fileSecurityInfo 填充的情况
	jsonType = json_object_get_type(jsonFileSecurityInfo);
	if (jsonType != json_type_object)
	{
		iRet = ADM_ECODE_JSON_KEY_TYPE_ERROR;
		log_e("Json key[%s] type error", ADM_COM_AIR_PROTOCOL_FIELD_FILESCYINFO);
		return iRet;
	}

	do
	{
		//!< 可选条件必选情况下的参数校验 
		if (ADM_IS_NULL(ppstFileSecurityInfo))
		{
			log_e("Invalid input param");
			return ADM_ECODE_INVALID_PARAM;
		}

		pstFileSecurityInfo = __integrationTestNewFileSecurityInfo();
		if (ADM_IS_NULL(pstFileSecurityInfo))
		{
			iRet = ADM_ECODE_NO_MEMORY;
			log_e("Failed to malloc, err[%s]", ADM_ECODE_NO_MEMORY);
			break;
		}


		//!< sub fileSecurityInfo
		iRet = __integrationTestGetJsonIntVal(jsonFileSecurityInfo, ADM_COM_AIR_PROTOCOL_FIELD_ISENCRYPT, 
					&pstFileSecurityInfo->isEncrypt);
		if (ADM_IS_ERR(iRet))
		{
			iRet = ADM_ECODE_JSON_GET_KEY_FAILED;
			log_e("Failed to get json value, key[%s], err[%d]", ADM_COM_AIR_PROTOCOL_FIELD_ISENCRYPT, iRet);
			break;
		}

		//!< isEncrypt 为 1 的情况
		if ( 1 == pstFileSecurityInfo->isEncrypt)
		{
			pstFileSecurityInfo->oriHash = __integrationTestGetJsonStrVal(jsonFileSecurityInfo, ADM_COM_AIR_PROTOCOL_FIELD_ORIHASH);
			if (ADM_IS_NULL(pstFileSecurityInfo->oriHash))
			{
				iRet = ADM_ECODE_JSON_GET_KEY_FAILED;
				log_e("Failed to get json value, key[%s]", ADM_COM_AIR_PROTOCOL_FIELD_ORIHASH);
				break;
			}
		
			pstFileSecurityInfo->pkgKey = __integrationTestGetJsonStrVal(jsonFileSecurityInfo, ADM_COM_AIR_PROTOCOL_FIELD_PKGKEY);
			if (ADM_IS_NULL(pstFileSecurityInfo->pkgKey))
			{
				iRet = ADM_ECODE_JSON_GET_KEY_FAILED;
				log_e("Failed to get json value, key[%s]", ADM_COM_AIR_PROTOCOL_FIELD_PKGKEY);
				break;
			}
		
			iRet = __integrationTestGetJsonInt64Val(jsonFileSecurityInfo, ADM_COM_AIR_PROTOCOL_FIELD_ORISIZE, 
						&pstFileSecurityInfo->oriSize);
			if (ADM_IS_ERR(iRet))
			{
				log_e("Failed to get json value, key[%s], err[%d]", ADM_COM_AIR_PROTOCOL_FIELD_ORISIZE, iRet);
				break;
			}
		
			iRet = __integrationTestGetJsonIntVal(jsonFileSecurityInfo, ADM_COM_AIR_PROTOCOL_FIELD_SEGDECRYPTLEN, 
						&pstFileSecurityInfo->segmentDecryptLen);
			if (ADM_IS_ERR(iRet))
			{
				log_e("Failed to get json value, key[%s], err[%d]", ADM_COM_AIR_PROTOCOL_FIELD_SEGDECRYPTLEN, iRet);
				break;
			}
		}
		
		//!< 解析 fileSecurityInfo   中 fileMetas
		if (!json_object_object_get_ex(jsonFileSecurityInfo, ADM_COM_AIR_PROTOCOL_FIELD_FILEMETAS, &jsonFileMetas))
		{
			iRet = ADM_ECODE_JSON_GET_KEY_FAILED;
			log_e("Failed to get json value, key[%s], err[%d]", ADM_COM_AIR_PROTOCOL_FIELD_FILEMETAS, iRet);
			break;
		}
		
		jsonType = json_object_get_type(jsonFileMetas);
		if (jsonType != json_type_array)
		{
			iRet = ADM_ECODE_JSON_KEY_TYPE_ERROR;
			log_e("Json key[%s] type error", ADM_COM_AIR_PROTOCOL_FIELD_FILEMETAS);
			break;
		}
		
		pstFileSecurityInfo->fileMetaNum = (D32S)json_object_array_length(jsonFileMetas);
		if (0 == pstFileSecurityInfo->fileMetaNum)
		{
			iRet = ADM_ECODE_SUCCESSED;
			log_e("Json key[%s] array is zero", ADM_COM_AIR_PROTOCOL_FIELD_FILEMETAS);
			break;
		}
		
		pstFileSecurityInfo->pstFileMetaList = __integrationTestNewFileMetaList(pstFileSecurityInfo->fileMetaNum);
		if (ADM_IS_NULL(pstFileSecurityInfo->pstFileMetaList))
		{
			iRet = ADM_ECODE_NO_MEMORY;
			log_e("Failed to new file metas list");
			break;
		}
		
		for (i = 0; i < pstFileSecurityInfo->fileMetaNum; ++i) 
		{
			jsonArrayIndexObj = json_object_array_get_idx(jsonFileMetas, (D32U)i);
			if (ADM_IS_NULL(jsonArrayIndexObj)) 
			{
				iRet = ADM_ECODE_JSON_GET_ARRAYOBJECT_FAILED;
				log_e("Failed to backtracking Json array, arrayIndex[%d]", i);
				break;
			}
		
			iRet = __integrationTestParseJsonFileMetas(jsonArrayIndexObj, &pstFileSecurityInfo->pstFileMetaList[i]);
			if (ADM_IS_ERR(iRet))
			{
				log_e("Failed to parse file meta info, err[%d]", iRet);
				break;
			}
		}
	} while (FALSE);

	if (ADM_IS_ERR(iRet))
	{
		__integrationTestCleanFileSecurityInfo(pstFileSecurityInfo);
		pstFileSecurityInfo = NULL;
	}
	*ppstFileSecurityInfo = pstFileSecurityInfo;

	return iRet;
}




/*!
 *******************************************************************************
 * 设置集成测试配置文件目录
 *
 * \note 
 *
 * \param[in]   pcConfigFileDir      集成测试配置文件目录
 * 
 * \retval      无
 *******************************************************************************
 */
void adm_integrationtest_setConfigFileDir(DCHAR* pcConfigFileDir)
{
    g_testConfigFileDir = pcConfigFileDir;
}

/*!
 *******************************************************************************
 * 获取集成测试配置文件目录
 *
 * \note 
 *
 * \retval      集成测试配置文件目录
 *******************************************************************************
 */
static DCHAR*  __integrationTestGetConfigFileDir(void)
{
    return g_testConfigFileDir;
}

static D64S test_readFile(const DCHAR* pcName, DCHAR* pcBuf, D64S iBufSize)
{
    D32U iSize = 0;
    void* pFile = NULL;

    adm_fopen(pcName, "r", &pFile);
    RETURN_IF_NULL(pFile, 0);

    adm_fread(pcBuf, (D32U)(iBufSize - 1), &iSize, pFile);
    adm_fclose(&pFile);
    pcBuf[iSize] = '\0';

    return iSize;
}


/*!
 *******************************************************************************
 * 读取指定文件的数据，若读取失败返回空，否则返回读取buf。数据使用完后需要释放内存
 *
 * \note 
 *
 * \param[in]   pcFileName      文件名
 * 
 * \retval      DCHAR*  读取数据buf
 *******************************************************************************
 */
DCHAR*  adm_integrationtest_readFile(const DCHAR* pcFileName)
{
    D32U uiFileSize = 0;

    RETURN_IF_NEQ(ADM_ERR_OK, adm_fgetfilesize(pcFileName, &uiFileSize), NULL);

    DCHAR* pcBuf = (DCHAR*)adm_malloc(uiFileSize + 1);
    RETURN_IF_NULL(pcBuf, NULL);

    do
    {
        if (0 == test_readFile(pcFileName, pcBuf, uiFileSize + 1))  //read fail
        {
            adm_free(pcBuf);
            return NULL;
        }

        
    } while (FALSE);


    return pcBuf;
}


/*!
 *******************************************************************************
 * 解析fileObject结构信息.
 *
 * \note 内部若出错其前面申请的内存会在外层释放
 * 
 * \param[in]  jsonFileObj   文件对象节点
 * \param[in]  pstFileObject 文件对象结构信息
 * \retval     D32S
 *******************************************************************************
 */
static D32S __integrationTestParseJsonFileObject
(
	json_object 		 *jsonFileObj, 
	ADM_COM_FILEOBJECT_T *pstFileObject
)
{
	D32S iRet = ADM_ECODE_SUCCESSED;

	if (ADM_IS_NULL(jsonFileObj) || ADM_IS_NULL(pstFileObject))
	{
		log_e("Invalid input param");
		return ADM_ECODE_INVALID_PARAM;
	}

	do
	{
		pstFileObject->fileName = __integrationTestGetJsonStrVal(jsonFileObj, ADM_COM_AIR_PROTOCOL_FIELD_FILENAME);
		if (ADM_IS_NULL(pstFileObject->fileName))
		{
			iRet = ADM_ECODE_JSON_GET_KEY_FAILED;
			log_e("Failed to get json value, key[%s]", ADM_COM_AIR_PROTOCOL_FIELD_FILENAME);
			break;
		}

		//!< 可选字段,	   仅增加日志记录即可, 无需校验
		pstFileObject->filePath = __integrationTestGetJsonStrVal(jsonFileObj, ADM_COM_AIR_PROTOCOL_FIELD_FILEPATH);
		if (ADM_IS_NULL(pstFileObject->filePath))
		{
			log_i("Failed to get json value, key[%s]", ADM_COM_AIR_PROTOCOL_FIELD_FILEPATH);
		}
				
		pstFileObject->url = __integrationTestGetJsonStrVal(jsonFileObj, ADM_COM_AIR_PROTOCOL_FIELD_URL);
		if (ADM_IS_NULL(pstFileObject->url))
		{
			log_i("Failed to get json value, key[%s]", ADM_COM_AIR_PROTOCOL_FIELD_URL);
		}
				
		pstFileObject->hash = __integrationTestGetJsonStrVal(jsonFileObj, ADM_COM_AIR_PROTOCOL_FIELD_HASH);
		if (ADM_IS_NULL(pstFileObject->hash))
		{
			iRet = ADM_ECODE_JSON_GET_KEY_FAILED;
			log_e("Failed to get json value, key[%s]", ADM_COM_AIR_PROTOCOL_FIELD_HASH);
			break;
		}
		
		iRet = __integrationTestGetJsonInt64Val(jsonFileObj, ADM_COM_AIR_PROTOCOL_FIELD_SIZE, &pstFileObject->size);
		if (ADM_IS_ERR(iRet))
		{
			log_e("Failed to get json value, key[%s], err[%d]", ADM_COM_AIR_PROTOCOL_FIELD_SIZE, iRet);
			break;
		}

		//!< 解析fileObject  中fileSecurityInfo --- 可选字段
		iRet = __integrationTestParseJsonFileSecurityInfo(jsonFileObj, &pstFileObject->pstFileSecurityInfo);
		if (ADM_IS_ERR(iRet))
		{
			log_e("Failed to parse msg hash node, err[%d]", iRet);
			break;
		}
	} while (FALSE);

	if (ADM_IS_ERR(iRet))
	{
		__integrationTestCleanFileObject(pstFileObject);
	}

	return iRet;
}


/*!
 *******************************************************************************
 * 解析业务脚本对象信息.
 *
 * \note
 * 
 * \param[in]   jsonRoot          Json 根节点句柄
 * \param[in]   pcConfFileDir       配置文件目录
 * \param[out]  pstBusinessScript 业务脚本对象信息
 * \retval      D32S
 *******************************************************************************
 */
static D32S __integrationTestParseBusinessScriptInfo
(
    const json_object          *pjsonRoot, 
    DCHAR                *pcConfFileDir, 
    ADM_COM_FILEOBJECT_T *pstBusinessScript
)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    DCHAR szBuf[ADM_COM_FILEPATH_LEN] = {0};
    json_type    jsonType = json_type_null;
    json_object *jsonBusinessScript = NULL;

    if (ADM_IS_NULL(pjsonRoot) || ADM_IS_NULL(pcConfFileDir) || ADM_IS_NULL(pstBusinessScript))
    {
        log_e("Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }

    //!< 取json中businessScript节点信息
    if (!json_object_object_get_ex(pjsonRoot, ADM_ECS_CFG_BUSINESSSCRIPT, &jsonBusinessScript))
    {
        log_e("Failed to get json value, key[%s]", ADM_ECS_CFG_BUSINESSSCRIPT);
        return ADM_ECODE_JSON_GET_KEY_FAILED;
    }

    jsonType = json_object_get_type(jsonBusinessScript);
    if (jsonType != json_type_object)
    {
        log_e("Json key[%s] type error", ADM_ECS_CFG_BUSINESSSCRIPT);
        return ADM_ECODE_JSON_KEY_TYPE_ERROR;
    }

    iRet = __integrationTestParseJsonFileObject(jsonBusinessScript, pstBusinessScript);
    if (ADM_IS_ERR(iRet))
    {
        log_e("Failed to parse business script file object", iRet);
        return iRet;
    }
    //!< 更新业务脚本在车端的存储路径 
    adm_free(pstBusinessScript->filePath);
    (void)adm_snprintf(szBuf, sizeof(szBuf), "%s%s", pcConfFileDir, pstBusinessScript->fileName);
    pstBusinessScript->filePath = adm_strdup(szBuf);
    log_d("BusinessScriptPath[%s]", szBuf);

    return iRet;
}

/*!
 *******************************************************************************
 * 申请ecs算法库列表结构内存.
 *
 * \note 申请的为连续内存
 *
 * \param[in]  num 算法库个数
 * \retval     ADM_ECS_SALIBINFO_T*
 *******************************************************************************
 */
static ADM_ECS_SALIBINFO_T* __integrationTestNewSaLibList(D32S num)
{
    ADM_ECS_SALIBINFO_T *pstSaLibInfo = NULL;

    if (0 >= num)
    {
        return NULL;
    }

    pstSaLibInfo = (ADM_ECS_SALIBINFO_T *)adm_malloc((D32U)num * sizeof(*pstSaLibInfo));
    if (NULL != pstSaLibInfo)
    {
        (void)adm_memzero(pstSaLibInfo, (D32U)num * sizeof(*pstSaLibInfo));
    }

    return pstSaLibInfo;
}




/*!
 *******************************************************************************
 * 释放ecs算法库列表结构内存.
 *
 * \note    释放的为连续内存
 *
 * \param[in]  num 算法库个数
 * \param[in]  pstSaLibInfo 算法库列表信息
 * \retval     none
 *******************************************************************************
 */
static void __integrationTestDelSaLibList
(
    D32S num, 
    ADM_ECS_SALIBINFO_T **pstSaLibInfo
)
{
    D32S i = 0;

    if ((0 >= num) || (ADM_IS_NULL(pstSaLibInfo)) || (ADM_IS_NULL(*pstSaLibInfo)))
    {
        return;
    }

    for (i = 0; i < num; i++)
    {
        __integrationTestCleanFileObject(&(*pstSaLibInfo)->stSaLibFile);
    }
    adm_free(*pstSaLibInfo);

    return;
}


/*!
 *******************************************************************************
 * 解析算法库信息.
 *
 * \note
 * 
 * \param[in]   jsonSaLib    Json SaLib节点句柄
 * \param[out]  pstSaLibInfo 算法库信息
 * \retval      D32S
 *******************************************************************************
 */
static D32S __integrationTestParseSaLibInfo
(
    json_object         *jsonSaLib, 
    DCHAR               *confFileDir, 
    ADM_ECS_SALIBINFO_T *pstSaLibInfo
)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    DCHAR szBuf[ADM_COM_FILEPATH_LEN] = {0};
    json_type    jsonType = json_type_null;
    json_object *jsonSaLibFile = NULL;

    if (ADM_IS_NULL(jsonSaLib) || ADM_IS_NULL(confFileDir) || ADM_IS_NULL(pstSaLibInfo))
    {
        log_e("Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }

    iRet = __integrationTestGetJsonIntVal(jsonSaLib, ADM_ECS_CFG_SALIBNUMBER, &pstSaLibInfo->saLibNumber);
    if (ADM_IS_ERR(iRet))
    {
        log_e("Failed to get json value, key[%s], err[%d]", 
                    ADM_ECS_CFG_SALIBNUMBER, iRet);
        return iRet;
    }

    //!< 取json中 saLibFile 节点信息
    if (!json_object_object_get_ex(jsonSaLib, ADM_ECS_CFG_SALIBFILE, &jsonSaLibFile))
    {
        log_e("Failed to get json value, key[%s]", ADM_ECS_CFG_SALIBFILE);
        return ADM_ECODE_JSON_GET_KEY_FAILED;
    }

    jsonType = json_object_get_type(jsonSaLibFile);
    if (jsonType != json_type_object)
    {
        log_e("Json key[%s] type error", ADM_ECS_CFG_BUSINESSSCRIPT);
        return ADM_ECODE_JSON_KEY_TYPE_ERROR;
    }

    iRet = __integrationTestParseJsonFileObject(jsonSaLibFile, &pstSaLibInfo->stSaLibFile);
    if (ADM_IS_ERR(iRet))
    {
        log_e("Failed to parse business script file object", iRet);
        return iRet;
    }
    //!< 更新SaLib在车端的存储路径 
    adm_free(pstSaLibInfo->stSaLibFile.filePath);
    (void)adm_snprintf(szBuf, sizeof(szBuf), "%s%s", confFileDir, pstSaLibInfo->stSaLibFile.fileName);
    pstSaLibInfo->stSaLibFile.filePath = adm_strdup(szBuf);
    log_d("SaLibPath[%s]", szBuf);

    return iRet;
}


/*!
 *******************************************************************************
 * 解析算法库列表信息.
 *
 * \note
 * 
 * \param[in]   jsonRoot     Json 根节点句柄
 * \param[in]   confFileDir  配置文件目录
 * \param[out]  pstSaLibList 算法库列表信息
 * \retval      D32S
 *******************************************************************************
 */
static D32S __integrationTestParseSaLibListInfo
(
    const  json_object         *jsonRoot, 
    DCHAR               *confFileDir, 
    ADM_ECS_SALIBLIST_T *pstSaLibList
)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    D32S i = 0;
    json_type    jsonType      = json_type_null;
    json_object *jsonSaLibList = NULL;
    json_object *jsonArrayIndexObj = NULL;

    if (ADM_IS_NULL(jsonRoot) || ADM_IS_NULL(confFileDir) || ADM_IS_NULL(pstSaLibList))
    {
        log_e("Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }

    //!< 取json中saLibList节点信息
    if (!json_object_object_get_ex(jsonRoot, ADM_ECS_CFG_SALIBLIST, &jsonSaLibList))
    {
        log_e("Failed to get json value, key[%s]", ADM_ECS_CFG_SALIBLIST);
        return ADM_ECODE_JSON_GET_KEY_FAILED;
    }

    jsonType = json_object_get_type(jsonSaLibList);
    if (jsonType != json_type_array)
    {
        log_e("Json key[%s] type error", ADM_ECS_CFG_SALIBLIST);
        return ADM_ECODE_JSON_KEY_TYPE_ERROR;
    }

    pstSaLibList->num = (D32S)json_object_array_length(jsonSaLibList);
    if (0 == pstSaLibList->num)
    {
        log_i("Json key[%s] array is zero", ADM_ECS_CFG_SALIBLIST);
        return ADM_ECODE_SUCCESSED;
    }

    pstSaLibList->pstSaLibInfo = __integrationTestNewSaLibList(pstSaLibList->num);
    if (ADM_IS_NULL(pstSaLibList->pstSaLibInfo))
    {
        log_e("Failed to new ecu saLiblist");
        return ADM_ECODE_NO_MEMORY;
    }

    for (i = 0; i < pstSaLibList->num; ++i) 
    {
        jsonArrayIndexObj = json_object_array_get_idx(jsonSaLibList, (D32U)i);
        if (ADM_IS_NULL(jsonArrayIndexObj)) 
        {
            iRet = ADM_ECODE_JSON_GET_ARRAYOBJECT_FAILED;
            log_e("Failed to backtracking Json array, arrayIndex[%d]", i);
            break;
        }

        iRet = __integrationTestParseSaLibInfo(jsonArrayIndexObj, confFileDir, &pstSaLibList->pstSaLibInfo[i]);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to parse saLib info, err[%d]", iRet);
            break;
        }
    }

    if (ADM_IS_ERR(iRet))
    {
        __integrationTestDelSaLibList(pstSaLibList->num, &pstSaLibList->pstSaLibInfo);
    }

    return iRet;
}



/*!
 *******************************************************************************
 * 申请crc算法库列表结构内存.
 *
 * \note 申请的为连续内存
 *
 * \param[in]  num 算法库个数
 * \retval     ADM_ECS_CRCLIBINFO_T*
 *******************************************************************************
 */
static ADM_ECS_CRCLIBINFO_T* __integrationTestNewCrcLibList(D32S num)
{
    ADM_ECS_CRCLIBINFO_T *pstCrcLibInfo = NULL;

    if (0 >= num)
    {
        return NULL;
    }

    pstCrcLibInfo = (ADM_ECS_CRCLIBINFO_T *)adm_malloc((D32U)num * sizeof(*pstCrcLibInfo));
    if (NULL != pstCrcLibInfo)
    {
        (void)adm_memzero(pstCrcLibInfo, (D32U)num * sizeof(*pstCrcLibInfo));
    }

    return pstCrcLibInfo;
}

/*!
 *******************************************************************************
 * 释放crc算法库列表结构内存.
 *
 * \note    释放的为连续内存
 *
 * \param[in]  num           算法库个数
 * \param[in]  pstCrcLibInfo 算法库列表信息
 * \retval     none
 *******************************************************************************
 */
static void __integrationTestDelCrcLibList
(
    D32S num, 
    ADM_ECS_CRCLIBINFO_T **pstCrcLibInfo
)
{
    D32S i = 0;

    if ((0 >= num) || (ADM_IS_NULL(pstCrcLibInfo)) || (ADM_IS_NULL(*pstCrcLibInfo)))
    {
        return;
    }

    for (i = 0; i < num; i++)
    {
        __integrationTestCleanFileObject(&(*pstCrcLibInfo)->stCrcLibFile);
    }
    adm_free(*pstCrcLibInfo);

    return;
}


/*!
 *******************************************************************************
 * 解析算法库信息.
 *
 * \note
 * 
 * \param[in]   jsonSaLib    Json SaLib节点句柄
 * \param[in]   confFileDir  配置文件目录
 * \param[out]  pstSaLibInfo 算法库信息
 * \retval      D32S
 *******************************************************************************
 */
static D32S __integrationTestParseCrcLibInfo
(
    json_object          *jsonCrcLib, 
    DCHAR                *confFileDir, 
    ADM_ECS_CRCLIBINFO_T *pstCrcLibInfo
)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    DCHAR szBuf[ADM_COM_FILEPATH_LEN] = {0};
    json_type    jsonType = json_type_null;
    json_object *jsonCrcLibFile = NULL;

    if (ADM_IS_NULL(jsonCrcLib) || ADM_IS_NULL(confFileDir) || ADM_IS_NULL(pstCrcLibInfo))
    {
        log_e("Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }

    iRet = __integrationTestGetJsonIntVal(jsonCrcLib, ADM_ECS_CFG_CRCLIBNUMBER, &pstCrcLibInfo->crcLibNumber);
    if (ADM_IS_ERR(iRet))
    {
        log_e("Failed to get json value, key[%s], err[%d]", 
                    ADM_ECS_CFG_CRCLIBNUMBER, iRet);
        return iRet;
    }

    //!< 取json中 saLibFile 节点信息
    if (!json_object_object_get_ex(jsonCrcLib, ADM_ECS_CFG_CRCLIBFILE, &jsonCrcLibFile))
    {
        log_e("Failed to get json value, key[%s]", ADM_ECS_CFG_CRCLIBFILE);
        return ADM_ECODE_JSON_GET_KEY_FAILED;
    }

    jsonType = json_object_get_type(jsonCrcLibFile);
    if (jsonType != json_type_object)
    {
        log_e("Json key[%s] type error", ADM_ECS_CFG_CRCLIBFILE);
        return ADM_ECODE_JSON_KEY_TYPE_ERROR;
    }

    iRet = __integrationTestParseJsonFileObject(jsonCrcLibFile, &pstCrcLibInfo->stCrcLibFile);
    if (ADM_IS_ERR(iRet))
    {
        log_e("Failed to parse business script file object", iRet);
        return iRet;
    }
    //!< 更新CrcLib在车端的存储路径 
    adm_free(pstCrcLibInfo->stCrcLibFile.filePath);
    (void)adm_snprintf(szBuf, sizeof(szBuf), "%s%s", confFileDir, pstCrcLibInfo->stCrcLibFile.fileName);
    pstCrcLibInfo->stCrcLibFile.filePath = adm_strdup(szBuf);
    log_d("CrcLibPath[%s]", szBuf);

    return iRet;
}


/*!
 *******************************************************************************
 * 解析crc算法库列表信息.
 *
 * \note
 * 
 * \param[in]   jsonRoot      Json 根节点句柄
 * \param[in]   confFileDir   配置文件目录
 * \param[out]  pstCrcLibList 算法库列表信息
 * \retval      D32S
 *******************************************************************************
 */
static D32S __integrationTestParseCrcLibListInfo
(
    const json_object          *jsonRoot, 
    DCHAR                *confFileDir, 
    ADM_ECS_CRCLIBLIST_T *pstCrcLibList
)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    D32S i = 0;
    json_type    jsonType      = json_type_null;
    json_object *jsonCrcLibList = NULL;
    json_object *jsonArrayIndexObj = NULL;

    if (ADM_IS_NULL(jsonRoot) || ADM_IS_NULL(confFileDir) || ADM_IS_NULL(pstCrcLibList))
    {
        log_e("Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }

    //!< 取json中saLibList节点信息
    if (!json_object_object_get_ex(jsonRoot, ADM_ECS_CFG_CRCLIBLIST, &jsonCrcLibList))
    {
        log_e("Failed to get json value, key[%s]", ADM_ECS_CFG_CRCLIBLIST);
        return ADM_ECODE_JSON_GET_KEY_FAILED;
    }

    jsonType = json_object_get_type(jsonCrcLibList);
    if (jsonType != json_type_array)
    {
        log_e("Json key[%s] type error", ADM_ECS_CFG_CRCLIBLIST);
        return ADM_ECODE_JSON_KEY_TYPE_ERROR;
    }

    pstCrcLibList->num = (D32S)json_object_array_length(jsonCrcLibList);
    if (0 == pstCrcLibList->num)
    {
        log_i("Json key[%s] array is zero", ADM_ECS_CFG_CRCLIBLIST);
        return ADM_ECODE_SUCCESSED;
    }

    pstCrcLibList->pstCrcLibInfo = __integrationTestNewCrcLibList(pstCrcLibList->num);
    if (ADM_IS_NULL(pstCrcLibList->pstCrcLibInfo))
    {
        log_e("Failed to new ecu saLiblist");
        return ADM_ECODE_NO_MEMORY;
    }

    for (i = 0; i < pstCrcLibList->num; ++i) 
    {
        jsonArrayIndexObj = json_object_array_get_idx(jsonCrcLibList, (D32U)i);
        if (ADM_IS_NULL(jsonArrayIndexObj)) 
        {
            iRet = ADM_ECODE_JSON_GET_ARRAYOBJECT_FAILED;
            log_e("Failed to backtracking Json array, arrayIndex[%d]", i);
            break;
        }

        iRet = __integrationTestParseCrcLibInfo(jsonArrayIndexObj, confFileDir, &pstCrcLibList->pstCrcLibInfo[i]);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to parse saLib info, err[%d]", iRet);
            break;
        }
    }

    if (ADM_IS_ERR(iRet))
    {
        __integrationTestDelCrcLibList(pstCrcLibList->num, &pstCrcLibList->pstCrcLibInfo);
    }

    return iRet;
}


/*!
 *******************************************************************************
 * 申请ecs刷写脚本列表结构内存.
 *
 * \note 申请的为连续内存
 *
 * \param[in]  num 刷写脚本个数
 * \retval     ADM_ECS_FLASHSCRIPTINFO_T*
 *******************************************************************************
 */
ADM_ECS_FLASHSCRIPTINFO_T* __integrationTestNewFlashScriptList(D32S num)
{
    ADM_ECS_FLASHSCRIPTINFO_T *pstFlashScriptInfo = NULL;

    if (0 >= num)
    {
        return NULL;
    }

    pstFlashScriptInfo = (ADM_ECS_FLASHSCRIPTINFO_T *)adm_malloc((D32U)num * sizeof(*pstFlashScriptInfo));
    if (NULL != pstFlashScriptInfo)
    {
        (void)adm_memzero(pstFlashScriptInfo, (D32U)num * sizeof(*pstFlashScriptInfo));
    }

    return pstFlashScriptInfo;
}

/*!
 *******************************************************************************
 * 释放ecs刷写脚本列表结构内存.
 *
 * \note    释放的为连续内存
 *
 * \param[in]  num 刷写脚本个数
 * \param[in]  pstFlashScriptInfo 刷写脚本列表信息
 * \retval     none
 *******************************************************************************
 */
void __integrationTestDelFlashScriptList
(
    D32S num, 
    ADM_ECS_FLASHSCRIPTINFO_T **pstFlashScriptInfo
)
{
    D32S i = 0;

    if ((0 >= num) || (ADM_IS_NULL(pstFlashScriptInfo)) || (ADM_IS_NULL(*pstFlashScriptInfo)))
    {
        return;
    }

    for (i = 0; i < num; i++)
    {
        __integrationTestCleanFileObject(&(*pstFlashScriptInfo)->stFlashScriptFile);
    }
    adm_free(*pstFlashScriptInfo);

    return;
}



/*!
 *******************************************************************************
 * 解析刷写脚本信息.
 *
 * \note
 * 
 * \param[in]   jsonFlashScript    Json flashScript节点句柄
 * \param[in]   confFileDir        配置文件目录
 * \param[out]  pstFlashScriptInfo 刷写脚本信息
 * \retval      D32S
 *******************************************************************************
 */
static D32S __integrationTestParseFlashScriptInfo
(
    json_object               *jsonFlashScript, 
    DCHAR                     *confFileDir, 
    ADM_ECS_FLASHSCRIPTINFO_T *pstFlashScriptInfo
)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    DCHAR szBuf[ADM_COM_FILEPATH_LEN] = {0};
    json_type    jsonType = json_type_null;
    json_object *jsonFlashScriptFile = NULL;

    if (ADM_IS_NULL(jsonFlashScript) || ADM_IS_NULL(confFileDir) || ADM_IS_NULL(pstFlashScriptInfo))
    {
        log_e("Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }

    iRet = __integrationTestGetJsonIntVal(jsonFlashScript, ADM_ECS_CFG_FLASHSCRIPTNUMBER, 
                &pstFlashScriptInfo->flashScriptNumber);
    if (ADM_IS_ERR(iRet))
    {
        log_e("Failed to get json value, key[%s], err[%d]", 
                    ADM_ECS_CFG_FLASHSCRIPTNUMBER, iRet);
        return iRet;
    }

    //!< 取json中 flashScriptFile 节点信息
    if (!json_object_object_get_ex(jsonFlashScript, ADM_ECS_CFG_FLASHSCRIPTFILE, &jsonFlashScriptFile))
    {
        log_e("Failed to get json value, key[%s]", ADM_ECS_CFG_FLASHSCRIPTFILE);
        return ADM_ECODE_JSON_GET_KEY_FAILED;
    }

    jsonType = json_object_get_type(jsonFlashScriptFile);
    if (jsonType != json_type_object)
    {
        log_e("Json key[%s] type error", ADM_ECS_CFG_FLASHSCRIPTFILE);
        return ADM_ECODE_JSON_KEY_TYPE_ERROR;
    }

    iRet = __integrationTestParseJsonFileObject(jsonFlashScriptFile, 
                &pstFlashScriptInfo->stFlashScriptFile);
    if (ADM_IS_ERR(iRet))
    {
        log_e("Failed to parse business script file object", iRet);
        return iRet;
    }
    //!< 更新FlashScript在车端的存储路径 
    adm_free(pstFlashScriptInfo->stFlashScriptFile.filePath);
    (void)adm_snprintf(szBuf, sizeof(szBuf), "%s%s", confFileDir, pstFlashScriptInfo->stFlashScriptFile.fileName);
    pstFlashScriptInfo->stFlashScriptFile.filePath = adm_strdup(szBuf);
    log_d("FlashScriptPath[%s]", szBuf);

    return iRet;
}


/*!
 *******************************************************************************
 * 解析刷写脚本列表信息.
 *
 * \note
 * 
 * \param[in]   jsonRoot           Json 根节点句柄
 * \param[in]   confFileDir        配置文件目录
 * \param[out]  pstFlashScriptList 刷写脚本列表信息
 * \retval      D32S
 *******************************************************************************
 */
static D32S __integrationTestParseFlashScriptListInfo
(
    const json_object               *jsonRoot, 
    DCHAR                     *confFileDir, 
    ADM_ECS_FLASHSCRIPTLIST_T *pstFlashScriptList
)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    D32S i = 0;
    json_type    jsonType = json_type_null;
    json_object *jsonFlashScriptList = NULL;
    json_object *jsonArrayIndexObj = NULL;

    if (ADM_IS_NULL(jsonRoot) || ADM_IS_NULL(confFileDir) || ADM_IS_NULL(pstFlashScriptList))
    {
        log_e("Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }

    //!< 取json中flashScriptList节点信息
    if (!json_object_object_get_ex(jsonRoot, ADM_ECS_CFG_FLASHSCRIPTLIST, &jsonFlashScriptList))
    {
        log_e("Failed to get json value, key[%s]", ADM_ECS_CFG_FLASHSCRIPTLIST);
        return ADM_ECODE_JSON_GET_KEY_FAILED;
    }

    jsonType = json_object_get_type(jsonFlashScriptList);
    if (jsonType != json_type_array)
    {
        log_e("Json key[%s] type error", ADM_ECS_CFG_FLASHSCRIPTLIST);
        return ADM_ECODE_JSON_KEY_TYPE_ERROR;
    }

    pstFlashScriptList->num = (D32S)json_object_array_length(jsonFlashScriptList);
    if (0 == pstFlashScriptList->num)
    {
        log_i("Json key[%s] array is zero", ADM_ECS_CFG_FLASHSCRIPTLIST);
        return ADM_ECODE_SUCCESSED;
    }

    pstFlashScriptList->pstFlashScriptInfo = __integrationTestNewFlashScriptList(pstFlashScriptList->num);
    if (ADM_IS_NULL(pstFlashScriptList->pstFlashScriptInfo))
    {
        log_e("Failed to new ecu saLiblist");
        return ADM_ECODE_NO_MEMORY;
    }

    for (i = 0; i < pstFlashScriptList->num; ++i) 
    {
        jsonArrayIndexObj = json_object_array_get_idx(jsonFlashScriptList, (D32U)i);
        if (ADM_IS_NULL(jsonArrayIndexObj)) 
        {
            iRet = ADM_ECODE_JSON_GET_ARRAYOBJECT_FAILED;
            log_e("Failed to backtracking Json array, arrayIndex[%d]", i);
            break;
        }

        iRet = __integrationTestParseFlashScriptInfo(jsonArrayIndexObj, confFileDir, &pstFlashScriptList->pstFlashScriptInfo[i]);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to parse saLib info, err[%d]", iRet);
            break;
        }
    }

    if (ADM_IS_ERR(iRet))
    {
        __integrationTestDelFlashScriptList(pstFlashScriptList->num, &pstFlashScriptList->pstFlashScriptInfo);
    }

    return iRet;
}


/*!
 *******************************************************************************
 * 申请exts结构内存.
 *
 * \note 申请的为连续内存
 *
 * \param[in]  num                    exts列表个数
 * \retval     ADM_COM_EXTENDPARAM_T*
 *******************************************************************************
 */
ADM_COM_EXTENDPARAM_T* __integrationTestNewExtsInfo(D32S num)
{
    ADM_COM_EXTENDPARAM_T *pstExtParamInfo = NULL;

    if (0 >= num)
    {
        return NULL;
    }

    pstExtParamInfo = (ADM_COM_EXTENDPARAM_T *)adm_malloc((D32U)num * sizeof(*pstExtParamInfo));
    if (NULL != pstExtParamInfo)
    {
        (void)adm_memzero(pstExtParamInfo, (D32U)num * sizeof(*pstExtParamInfo));
    }

    return pstExtParamInfo;
}

/*!
 *******************************************************************************
 * 清空exts列表结构内存.
 *
 * \note 不释放传入的节点最外层的内存
 *
 * \param[in]  pstExtList exts列表信息
 * \retval     none
 *******************************************************************************
 */
void __integrationTestCleanExtsList(ADM_COM_EXTPARAMLIST_T *pstExtList)
{
    D32S i = 0;

    if (ADM_IS_NULL(pstExtList) || (0 >= pstExtList->num) || (ADM_IS_NULL(pstExtList->pstExtParamInfo)))
    {
        return;
    }

    for (i = 0; i < pstExtList->num; i++)
    {
        adm_free(pstExtList->pstExtParamInfo[i].extKey);
        adm_free(pstExtList->pstExtParamInfo[i].extValue);
    }    
    pstExtList->num = 0;
    adm_free(pstExtList->pstExtParamInfo);
    
    return;
}


/*!
 *******************************************************************************
 * 解析Exts结构信息.
 *
 * \note 
 * 
 * \param[in]  jsonRoot   Json节点
 * \param[in]  pstExtList 扩展字段list信息
 * \retval     D32S
 *******************************************************************************
 */
static D32S __integrationTestParseJsonExtsInfo
(
    const json_object            *jsonRoot, 
    ADM_COM_EXTPARAMLIST_T *pstExtList
)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    D32S i = 0;
    json_type    jsonType = json_type_null;
    json_object *jsonExtsList = NULL;
    json_object *jsonArrayIndexObj = NULL;

    if (ADM_IS_NULL(jsonRoot) || ADM_IS_NULL(pstExtList))
    {
        log_e("Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }

    //!< 取json中 exts 节点信息 --- 可选字段
    if (!json_object_object_get_ex(jsonRoot, ADM_COM_AIR_PROTOCOL_FIELD_EXT, &jsonExtsList))
    {
        pstExtList->num = 0;
        pstExtList->pstExtParamInfo = NULL;
        log_i("Failed to get json value, key[%s]", ADM_COM_AIR_PROTOCOL_FIELD_EXT);
        return ADM_ECODE_SUCCESSED;
    }
    
    jsonType = json_object_get_type(jsonExtsList);
    if (jsonType != json_type_array)
    {
        log_e("Json key[%s] type error", ADM_COM_AIR_PROTOCOL_FIELD_EXT);
        return ADM_ECODE_JSON_KEY_TYPE_ERROR;
    }
    
    pstExtList->num = (D32S)json_object_array_length(jsonExtsList);
    if (0 == pstExtList->num)
    {
        log_i("Json key[%s] array is zero", ADM_COM_AIR_PROTOCOL_FIELD_EXT);
        return ADM_ECODE_SUCCESSED;
    }
    
    pstExtList->pstExtParamInfo = __integrationTestNewExtsInfo(pstExtList->num);
    if (ADM_IS_NULL(pstExtList->pstExtParamInfo))
    {
        pstExtList->num = 0;
        log_e("Failed to new exts list");
        return ADM_ECODE_NO_MEMORY;
    }
    
    for (i = 0; i < pstExtList->num; ++i) 
    {
        jsonArrayIndexObj = json_object_array_get_idx(jsonExtsList, (D32U)i);
        if (ADM_IS_NULL(jsonArrayIndexObj)) 
        {
            iRet = ADM_ECODE_JSON_GET_ARRAYOBJECT_FAILED;
            log_e("Failed to backtracking Json array, arrayIndex[%d]", i);
            break;
        }
    
        // get extKey 
        pstExtList->pstExtParamInfo[i].extKey = __integrationTestGetJsonStrVal(jsonArrayIndexObj, ADM_COM_AIR_PROTOCOL_FIELD_EXTKEY);
        if (ADM_IS_NULL(pstExtList->pstExtParamInfo[i].extKey))
        {
            iRet = ADM_ECODE_JSON_GET_KEY_FAILED;
            log_e("Failed to get json value, key[%s]", ADM_COM_AIR_PROTOCOL_FIELD_EXTKEY);
            break;
        }
    
        // get extValue 
        pstExtList->pstExtParamInfo[i].extValue = __integrationTestGetJsonStrVal(jsonArrayIndexObj, ADM_COM_AIR_PROTOCOL_FIELD_EXTVALUE);
        if (ADM_IS_NULL(pstExtList->pstExtParamInfo[i].extValue))
        {
            iRet = ADM_ECODE_JSON_GET_KEY_FAILED;
            log_e("Failed to get json value, key[%s]", ADM_COM_AIR_PROTOCOL_FIELD_EXTVALUE);
            break;
        }
    }
    
    if (ADM_IS_ERR(iRet))
    {
        __integrationTestCleanExtsList(pstExtList);
    }

    return iRet;
}

/*!
 *******************************************************************************
 * 申请ecs业务脚本列表结构内存.
 *
 * \note 申请的为连续内存
 *
 * \param[in]  num 业务脚本个数
 * \retval     ADM_ECS_BUSINESSSCRIPTINFO_T*
 *******************************************************************************
 */
ADM_ECS_BUSINESSSCRIPTINFO_T* __integrationTestNewBusinessScriptList(D32S num)
{
    ADM_ECS_BUSINESSSCRIPTINFO_T *pstBusinessScriptInfo = NULL;

    if (0 >= num)
    {
        return NULL;
    }

    pstBusinessScriptInfo = (ADM_ECS_BUSINESSSCRIPTINFO_T *)adm_malloc((D32U)num * sizeof(*pstBusinessScriptInfo));
    if (NULL != pstBusinessScriptInfo)
    {
        (void)adm_memzero(pstBusinessScriptInfo, (D32U)num * sizeof(*pstBusinessScriptInfo));
    }

    return pstBusinessScriptInfo;
}

/*!
 *******************************************************************************
 * 解析业务脚本对象列表信息.
 *
 * \note
 * 
 * \param[in]   jsonBusinessScript    Json businessScripts节点句柄
 * \param[in]   confFileDir           配置文件目录
 * \param[out]  pstBusinessScriptInfo 业务脚本信息
 * \retval      D32S
 *******************************************************************************
 */
static D32S __integrationTestParseBusinessScriptsInfo
(
    json_object                  *jsonBusinessScript, 
    DCHAR                        *confFileDir, 
    ADM_ECS_BUSINESSSCRIPTINFO_T *pstBusinessScriptInfo
)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    DCHAR szBuf[ADM_COM_FILEPATH_LEN] = {0};
    json_type    jsonType = json_type_null;
    json_object *jsonBusinessScriptFile = NULL;

    if (ADM_IS_NULL(jsonBusinessScript) || ADM_IS_NULL(confFileDir) || ADM_IS_NULL(pstBusinessScriptInfo))
    {
        log_e("Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }

    iRet = __integrationTestGetJsonIntVal(jsonBusinessScript, ADM_ECS_CFG_BUSINESSSCRIPTNUMBER, 
                &pstBusinessScriptInfo->businessScriptNumber);
    if (ADM_IS_ERR(iRet))
    {
        log_e("Failed to get json value, key[%s], err[%d]", 
                    ADM_ECS_CFG_BUSINESSSCRIPTNUMBER, iRet);
        return iRet;
    }

    //!< 取json中 businessScriptFile 节点信息
    if (!json_object_object_get_ex(jsonBusinessScript, ADM_ECS_CFG_BUSINESSSCRIPTFILE, &jsonBusinessScriptFile))
    {
        log_e("Failed to get json value, key[%s]", ADM_ECS_CFG_BUSINESSSCRIPTFILE);
        return ADM_ECODE_JSON_GET_KEY_FAILED;
    }

    jsonType = json_object_get_type(jsonBusinessScriptFile);
    if (jsonType != json_type_object)
    {
        log_e("Json key[%s] type error", ADM_ECS_CFG_BUSINESSSCRIPTFILE);
        return ADM_ECODE_JSON_KEY_TYPE_ERROR;
    }

    iRet = __integrationTestParseJsonFileObject(jsonBusinessScriptFile, 
                &pstBusinessScriptInfo->stBusinessScriptFile);
    if (ADM_IS_ERR(iRet))
    {
        log_e("Failed to parse business script file object", iRet);
        return iRet;
    }
    //!< 更新BusinessScript在车端的存储路径 
    adm_free(pstBusinessScriptInfo->stBusinessScriptFile.filePath);
    (void)adm_snprintf(szBuf, sizeof(szBuf), "%s%s", confFileDir, pstBusinessScriptInfo->stBusinessScriptFile.fileName);
    pstBusinessScriptInfo->stBusinessScriptFile.filePath = adm_strdup(szBuf);
    log_d("BusinessScriptPath[%s]", szBuf);

    return iRet;
}

/*!
 *******************************************************************************
 * 释放ecs业务脚本列表结构内存.
 *
 * \note    释放的为连续内存
 *
 * \param[in]  num 业务脚本个数
 * \param[in]  pstBusinessScriptInfo 业务脚本列表信息
 * \retval     none
 *******************************************************************************
 */
void __integrationTestDelBusinessScriptList
(
    D32S num, 
    ADM_ECS_BUSINESSSCRIPTINFO_T **pstBusinessScriptInfo
)
{
    D32S i = 0;

    if ((0 >= num) || (ADM_IS_NULL(pstBusinessScriptInfo)) || (ADM_IS_NULL(*pstBusinessScriptInfo)))
    {
        return;
    }

    for (i = 0; i < num; i++)
    {
       __integrationTestCleanFileObject(&(*pstBusinessScriptInfo)->stBusinessScriptFile);
    }
    adm_free(*pstBusinessScriptInfo);

    return;
}


/*!
 *******************************************************************************
 * 解析业务脚本对象列表信息.
 *
 * \note
 * 
 * \param[in]   jsonRoot           Json 根节点句柄
 * \param[in]   confFileDir        配置文件目录
 * \param[out]  pstBusinessScriptList 业务脚本对象列表
 * \retval      D32S
 *******************************************************************************
 */
static D32S __integrationTestParseBusinessScriptListInfo
(
    const  json_object                   *jsonRoot, 
    DCHAR                         *confFileDir, 
    ADM_ECS_BUSINESSSCRIPTLIST_T  *pstBusinessScriptList
)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    D32S i = 0;
    json_type    jsonType = json_type_null;
    json_object *jsonBusinessScriptList = NULL;
    json_object *jsonArrayIndexObj = NULL;

    if (ADM_IS_NULL(jsonRoot) || ADM_IS_NULL(confFileDir) || ADM_IS_NULL(pstBusinessScriptList))
    {
        log_e("Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }

    //!< 取json中businessScriptList节点信息
    if (!json_object_object_get_ex(jsonRoot, ADM_ECS_CFG_BUSINESSSCRIPTLIST, &jsonBusinessScriptList))
    {
        log_e("Failed to get json value, key[%s]", ADM_ECS_CFG_BUSINESSSCRIPTLIST);
        return ADM_ECODE_JSON_GET_KEY_FAILED;
    }

    jsonType = json_object_get_type(jsonBusinessScriptList);
    if (jsonType != json_type_array)
    {
        log_e("Json key[%s] type error", ADM_ECS_CFG_BUSINESSSCRIPTLIST);
        return ADM_ECODE_JSON_KEY_TYPE_ERROR;
    }

    pstBusinessScriptList->num = (D32S)json_object_array_length(jsonBusinessScriptList);
    if (0 == pstBusinessScriptList->num)
    {
        log_i("Json key[%s] array is zero", ADM_ECS_CFG_BUSINESSSCRIPTLIST);
        return ADM_ECODE_SUCCESSED;
    }

    pstBusinessScriptList->pstBusinessScriptInfo = __integrationTestNewBusinessScriptList(pstBusinessScriptList->num);
    if (ADM_IS_NULL(pstBusinessScriptList->pstBusinessScriptInfo))
    {
        log_e("Failed to new ecu businessLiblist");
        return ADM_ECODE_NO_MEMORY;
    }

    for (i = 0; i < pstBusinessScriptList->num; ++i) 
    {
        jsonArrayIndexObj = json_object_array_get_idx(jsonBusinessScriptList, (D32U)i);
        if (ADM_IS_NULL(jsonArrayIndexObj)) 
        {
            iRet = ADM_ECODE_JSON_GET_ARRAYOBJECT_FAILED;
            log_e("Failed to backtracking Json array, arrayIndex[%d]", i);
            break;
        }

        iRet = __integrationTestParseBusinessScriptsInfo(jsonArrayIndexObj, confFileDir, &pstBusinessScriptList->pstBusinessScriptInfo[i]);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to parse businessscript info, err[%d]", iRet);
            break;
        }
    }

    if (ADM_IS_ERR(iRet))
    {
        __integrationTestDelBusinessScriptList(pstBusinessScriptList->num, &pstBusinessScriptList->pstBusinessScriptInfo);
    }

    return iRet;
}



/*!
 *******************************************************************************
 * 获取通用平台配置信息配置参数
 *
 * \note 
 *
 * \param[in]   pRoot      json数据
 * \param[in]   pstGeneralConfig      通用配置信息指针
 * 
 * \retval      D32S  执行结果
 *******************************************************************************
 */
static D32S __integrationTestGetGeneralConfigData(const json_object* jsonRoot, ADM_ECS_GENERALCONFIG_T* pstGeneralConfig)
{
	D32S iRet = ADM_ECODE_FAILED;

	if ((NULL == jsonRoot)||(NULL == pstGeneralConfig))
	{
	    return ADM_ECODE_FAILED;
	}
    
    do
	{
	    pstGeneralConfig->protocolVersion = __integrationTestGetJsonStrVal(jsonRoot, ADM_ECS_CFG_PTLVERSION);
        if (ADM_IS_NULL(pstGeneralConfig->protocolVersion))
        {
            iRet = ADM_ECODE_JSON_GET_STRING_FAILED;
            log_e("Failed to get json value, key[%s]", ADM_ECS_CFG_PTLVERSION);
            break;
        }
		else
		{
            log_i("successful to get json value, key[%s],value :%s", ADM_ECS_CFG_PTLVERSION,pstGeneralConfig->protocolVersion);

        }

		//!< get logReportUrl --- 可选字段 
        pstGeneralConfig->logReportUrl = __integrationTestGetJsonStrVal(jsonRoot, ADM_ECS_CFG_LOGREPORTURL);
        if (ADM_IS_NULL(pstGeneralConfig->logReportUrl))
        {
            log_i("Failed to get json value, key[%s]", ADM_ECS_CFG_LOGREPORTURL);
        }
	
		//!< get pollingCycle
        iRet = __integrationTestGetJsonIntVal(jsonRoot, ADM_ECS_CFG_POLLINGCYCLE, &pstGeneralConfig->pollingCycle);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to get json value, key[%s], err[%d]", 
                        ADM_ECS_CFG_POLLINGCYCLE, iRet);
            break;
        }
        else
		{
            log_i("successful to get json value, key[%s],value: %d", ADM_ECS_CFG_POLLINGCYCLE,pstGeneralConfig->pollingCycle);
        }
		
		iRet = __integrationTestGetJsonIntVal(jsonRoot, ADM_ECS_CFG_LOGREPORTCYC, &pstGeneralConfig->logReportCycle);
		if (ADM_IS_ERR(iRet))
		{
			log_e("Failed to get json value, key[%s], err[%d]", 
						ADM_ECS_CFG_LOGREPORTCYC, iRet);
			break;
		}
		else
		{
            log_i("successful to get json value, key[%s],value: %d", ADM_ECS_CFG_LOGREPORTCYC,pstGeneralConfig->logReportCycle);
        }
			
		iRet = __integrationTestGetJsonIntVal(jsonRoot, ADM_ECS_CFG_LOGPRINTLEVEL, &pstGeneralConfig->logPrintLevel);
		if (ADM_IS_ERR(iRet))
		{
			log_e("Failed to get json value, key[%s], err[%d]", 
						ADM_ECS_CFG_LOGPRINTLEVEL, iRet);
			break;
		}
			
		iRet = __integrationTestGetJsonIntVal(jsonRoot, ADM_ECS_CFG_ISLOGFILEENCRY, &pstGeneralConfig->isLogFileEncrypted);
		if (ADM_IS_ERR(iRet))
		{
			log_e("Failed to get json value, key[%s], err[%d]", 
						ADM_ECS_CFG_ISLOGFILEENCRY, iRet);
			break;
		}
			
		iRet = __integrationTestGetJsonIntVal(jsonRoot, ADM_ECS_CFG_ISREPORTUPSUCCLOGFILE, 
										&pstGeneralConfig->isReportUpSuccLogFile);
		if (ADM_IS_ERR(iRet))
		{
			log_e("Failed to get json value, key[%s], err[%d]", 
						ADM_ECS_CFG_ISREPORTUPSUCCLOGFILE, iRet);
			break;
		}

		iRet = __integrationTestGetJsonIntVal(jsonRoot, ADM_ECS_CFG_EVENTREPORTLEVEL, &pstGeneralConfig->eventReportLevel);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to get json value, key[%s], err[%d]", 
                        ADM_ECS_CFG_EVENTREPORTLEVEL, iRet);
            break;
        }
    
        iRet = __integrationTestGetJsonIntVal(jsonRoot, ADM_ECS_CFG_EVENTREPORTCYC, &pstGeneralConfig->eventReportCycle);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to get json value, key[%s], err[%d]", 
                        ADM_ECS_CFG_EVENTREPORTCYC, iRet);
            break;
        }
    
        iRet = __integrationTestGetJsonIntVal(jsonRoot, ADM_ECS_CFG_STATEREPORTCYC, &pstGeneralConfig->stateReportCycle);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to get json value, key[%s], err[%d]", 
                        ADM_ECS_CFG_STATEREPORTCYC, iRet);
            break;
        }		
		
		iRet = __integrationTestGetJsonIntVal(jsonRoot, ADM_ECS_CFG_SUMMARYREPORTCYC, &pstGeneralConfig->summaryReportCycle);
		if (ADM_IS_ERR(iRet))
		{
			log_e("Failed to get json value, key[%s], err[%d]", 
						ADM_ECS_CFG_SUMMARYREPORTCYC, iRet);
			break;
		}
			
		iRet = __integrationTestGetJsonIntVal(jsonRoot, ADM_ECS_CFG_PROGRESSREPORTCYC, &pstGeneralConfig->progressReportCycle);
		if (ADM_IS_ERR(iRet))
		{
			log_e("Failed to get json value, key[%s], err[%d]", 
						ADM_ECS_CFG_PROGRESSREPORTCYC, iRet);
			break;
		}
		
		iRet = __integrationTestGetJsonIntVal(jsonRoot, ADM_ECS_CFG_CYCLEPERIOD, &pstGeneralConfig->cyclePeriod);
		if (ADM_IS_ERR(iRet))
		{
			log_e("Failed to get json value, key[%s], err[%d]", 
						ADM_ECS_CFG_CYCLEPERIOD, iRet);
			break;
		}
		
		iRet = __integrationTestGetJsonIntVal(jsonRoot, ADM_ECS_CFG_MAXFIRETRIGGER, &pstGeneralConfig->maxFireTrigger);
		if (ADM_IS_ERR(iRet))
		{
			log_e("Failed to get json value, key[%s], err[%d]", 
						ADM_ECS_CFG_MAXFIRETRIGGER, iRet);
			break;
		}

		//!< 解析业务脚本对象信息
	    iRet = __integrationTestParseBusinessScriptInfo(jsonRoot, __integrationTestGetConfigFileDir(), &pstGeneralConfig->stBusinessScript);
	    if (ADM_IS_ERR(iRet))
	    {
	        log_e("Failed to parse BusinessScript, key[%s], err[%d]", 
	                    ADM_ECS_CFG_BUSINESSSCRIPT, iRet);
	        break;
	    }

		//!< 解析业务脚本对象列表
	    iRet = __integrationTestParseBusinessScriptListInfo(jsonRoot, __integrationTestGetConfigFileDir(), &pstGeneralConfig->stBusinessScriptList);
	    if (ADM_IS_ERR(iRet))
	    {
	        log_e("Failed to parse businessScriptList, key[%s], err[%d]", 
	                    ADM_ECS_CFG_BUSINESSSCRIPTLIST, iRet);
	        break;
	    }


		//!< 解析资源id列表信息
        iRet = __integrationTestParseEcuDidListInfo(jsonRoot, &pstGeneralConfig->stEcuDidList);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to parse ecuDid list, err[%d]", iRet);
            break;
        }

		//!< 解析sa算法库列表信息
        iRet = __integrationTestParseSaLibListInfo(jsonRoot, __integrationTestGetConfigFileDir(), &pstGeneralConfig->stSaLibList);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to parse saLibList, key[%s], err[%d]", 
                        ADM_ECS_CFG_SALIBLIST, iRet);
            break;
        }

		//!< 解析crc算法库列表信息
        iRet = __integrationTestParseCrcLibListInfo(jsonRoot, __integrationTestGetConfigFileDir(), &pstGeneralConfig->stCrcLibList);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to parse saLibList, key[%s], err[%d]", 
                        ADM_ECS_CFG_SALIBLIST, iRet);
            break;
        }
    
        //!< 解析刷写脚本列表信息
        iRet = __integrationTestParseFlashScriptListInfo(jsonRoot, __integrationTestGetConfigFileDir(), &pstGeneralConfig->stflashScriptList);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to parse FlashScriptList, key[%s], err[%d]", 
                        ADM_ECS_CFG_FLASHSCRIPTLIST, iRet);
            break;
        }

        //!< 扩展字段需要使用时在解析
        iRet = __integrationTestParseJsonExtsInfo(jsonRoot, &pstGeneralConfig->stExtList);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to parse exts, key[%s], err[%d]", 
                        ADM_COM_AIR_PROTOCOL_FIELD_EXT, iRet);
            break;
        }
	}while(FALSE);
	

    return ADM_ECODE_SUCCESSED;
}









/*!
 *******************************************************************************
 * 申请ecs ecus配置列表结构内存.
 *
 * \note 申请的为连续内存
 *
 * \param[in]  num ecus配置个数
 * \retval     ADM_ECS_ECUCONFIG_T*
 *******************************************************************************
 */
static ADM_ECS_ECUCONFIG_T* __integrationTestNewEcusConfigList(D32S num)
{
    ADM_ECS_ECUCONFIG_T *pstEcuList = NULL;

    if (0 >= num)
    {
        log_e("Invalid input param");
        return NULL;
    }

    pstEcuList = (ADM_ECS_ECUCONFIG_T *)adm_malloc((D32U)num * sizeof(*pstEcuList));
    if (NULL != pstEcuList)
    {
        (void)adm_memzero(pstEcuList, (D32U)num * sizeof(*pstEcuList));
    }

    return pstEcuList;
}


/*!
 *******************************************************************************
 * 申请ecs安全算法库编号列表结构内存.
 *
 * \note 申请的为连续内存
 *
 * \param[in]  num 算法库编号个数
 * \retval     D32S*
 *******************************************************************************
 */
static D32S* __integrationTestNewSaLibNumberList(D32S num)
{
    D32S *pstSaLibNumberList = NULL;

    if (0 >= num)
    {
        return NULL;
    }

    pstSaLibNumberList = (D32S *)adm_malloc((D32U)num * sizeof(*pstSaLibNumberList));
    if (NULL != pstSaLibNumberList)
    {
        (void)adm_memzero(pstSaLibNumberList, (D32U)num * sizeof(*pstSaLibNumberList));
    }

    return pstSaLibNumberList;
}

/*!
 *******************************************************************************
 * 释放ecs算法库编号列表结构内存.
 *
 * \note    释放的为连续内存
 *
 * \param[in]  num 算法库编号个数
 * \param[in]  pstSaLibNumberList 算法库编号列表信息
 * \retval     none
 *******************************************************************************
 */
static void __integrationTestDelSaLibNumberList
(
    D32S  num, 
    D32S **pstSaLibNumberList
)
{

    if ((0 >= num) || (ADM_IS_NULL(pstSaLibNumberList)) || (ADM_IS_NULL(*pstSaLibNumberList)))
    {
        return;
    }

    adm_free(*pstSaLibNumberList);

    return;
}

/*!
 *******************************************************************************
 * 解析算法库编号列表信息.
 *
 * \note
 * 
 * \param[in]   jsonRoot           Json 节点句柄
 * \param[out]  pstSaLibNumberList 算法库编号列表信息
 * \retval      D32S
 *******************************************************************************
 */
static D32S __integrationTestParseSaLibNumberListInfo
(
    json_object               *jsonRoot, 
    ADM_ECS_SALIBNUMBERLIST_T *pstSaLibNumberList
)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    D32S i = 0;
    json_type    jsonType      = json_type_null;
    json_object *jsonArrayIndexObj = NULL;
    json_object *jsonSaLibNumberList = NULL;

    if (ADM_IS_NULL(jsonRoot) || ADM_IS_NULL(pstSaLibNumberList))
    {
        log_e("Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }

    //!< 取json中saLibList节点信息
    if (!json_object_object_get_ex(jsonRoot, ADM_ECS_CFG_SALIBNUMBERLIST, &jsonSaLibNumberList))
    {
        log_e("Failed to get json value, key[%s]", ADM_ECS_CFG_SALIBNUMBERLIST);
        return ADM_ECODE_JSON_GET_KEY_FAILED;
    }

    jsonType = json_object_get_type(jsonSaLibNumberList);
    if (jsonType != json_type_array)
    {
        log_e("Json key[%s] type error", ADM_ECS_CFG_SALIBNUMBERLIST);
        return ADM_ECODE_JSON_KEY_TYPE_ERROR;
    }

    pstSaLibNumberList->num = (D32S)json_object_array_length(jsonSaLibNumberList);
    if (0 == pstSaLibNumberList->num)
    {
        log_i("Json key[%s] array is zero", ADM_ECS_CFG_SALIBNUMBERLIST);
        return ADM_ECODE_SUCCESSED;
    }

    pstSaLibNumberList->psaLibNumber = __integrationTestNewSaLibNumberList(pstSaLibNumberList->num);
    if (ADM_IS_NULL(pstSaLibNumberList->psaLibNumber))
    {
        log_e("Failed to new ecu saLiblist");
        return ADM_ECODE_NO_MEMORY;
    }

    for (i = 0; i < pstSaLibNumberList->num; ++i) 
    {
        jsonArrayIndexObj = json_object_array_get_idx(jsonSaLibNumberList, (D32U)i);
        if (ADM_IS_NULL(jsonArrayIndexObj)) 
        {
            iRet = ADM_ECODE_JSON_GET_ARRAYOBJECT_FAILED;
            log_e("Failed to backtracking Json array, arrayIndex[%d]", i);
            break;
        }

        //!< jsonArrayIndexObj 里面就一个成员，这里直接解
        iRet = __integrationTestGetJsonIntVal(jsonArrayIndexObj, ADM_ECS_CFG_SALIBNUMBER, &pstSaLibNumberList->psaLibNumber[i]);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to get json value, key[%s], err[%d]", 
                        ADM_ECS_CFG_SALIBNUMBER, iRet);
            break;
        }
    }

    if (ADM_IS_ERR(iRet))
    {
        __integrationTestDelSaLibNumberList(pstSaLibNumberList->num, &pstSaLibNumberList->psaLibNumber);
        pstSaLibNumberList->num = 0;
    }

    return iRet;
}


/*!
 *******************************************************************************
 * 申请ecs CRC算法库编号列表结构内存.
 *
 * \note 申请的为连续内存
 *
 * \param[in]  num 算法库编号个数
 * \retval     D32S*
 *******************************************************************************
 */
static D32S* __integrationTestNewCrcLibNumberList(D32S num)
{
    D32S *pstCrcLibNumberList = NULL;

    if (0 >= num)
    {
        return NULL;
    }

    pstCrcLibNumberList = (D32S *)adm_malloc((D32U)num * sizeof(*pstCrcLibNumberList));
    if (NULL != pstCrcLibNumberList)
    {
        (void)adm_memzero(pstCrcLibNumberList, (D32U)num * sizeof(*pstCrcLibNumberList));
    }

    return pstCrcLibNumberList;
}

/*!
 *******************************************************************************
 * 释放ecs CRC算法库编号列表结构内存.
 *
 * \note    释放的为连续内存
 *
 * \param[in]  num 算法库编号个数
 * \param[in]  pstCrcLibNumberList CRC算法库编号列表信息
 * \retval     none
 *******************************************************************************
 */
static void __integrationTestDelCrcLibNumberList
(
    D32S  num, 
    D32S **pstCrcLibNumberList
)
{

    if ((0 >= num) || (ADM_IS_NULL(pstCrcLibNumberList)) || (ADM_IS_NULL(*pstCrcLibNumberList)))
    {
        return;
    }

    adm_free(*pstCrcLibNumberList);

    return;
}


/*!
 *******************************************************************************
 * 解析CRC算法库编号列表信息.
 *
 * \note
 * 
 * \param[in]   jsonRoot           Json 节点句柄
 * \param[out]  pstCrcLibNumberList CRC算法库编号列表信息
 * \retval      D32S
 *******************************************************************************
 */
static D32S __integrationTestParseCrcLibNumberListInfo
(
    json_object               *jsonRoot, 
    ADM_ECS_CRCLIBNUMBERLIST_T *pstCrcLibNumberList
)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    D32S i = 0;
    json_type    jsonType      = json_type_null;
    json_object *jsonArrayIndexObj = NULL;
    json_object *jsonCrcLibNumberList = NULL;

    if (ADM_IS_NULL(jsonRoot) || ADM_IS_NULL(pstCrcLibNumberList))
    {
        log_e("Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }

    //!< 取json中crcLibList节点信息
    if (!json_object_object_get_ex(jsonRoot, ADM_ECS_CFG_CRCLIBNUMBERLIST, &jsonCrcLibNumberList))
    {
        log_e("Failed to get json value, key[%s]", ADM_ECS_CFG_CRCLIBNUMBERLIST);
        return ADM_ECODE_JSON_GET_KEY_FAILED;
    }

    jsonType = json_object_get_type(jsonCrcLibNumberList);
    if (jsonType != json_type_array)
    {
        log_e("Json key[%s] type error", ADM_ECS_CFG_CRCLIBNUMBERLIST);
        return ADM_ECODE_JSON_KEY_TYPE_ERROR;
    }

    pstCrcLibNumberList->num = (D32S)json_object_array_length(jsonCrcLibNumberList);
    if (0 == pstCrcLibNumberList->num)
    {
        log_i("Json key[%s] array is zero", ADM_ECS_CFG_CRCLIBNUMBERLIST);
        return ADM_ECODE_SUCCESSED;
    }

    pstCrcLibNumberList->pcrcLibNumber = __integrationTestNewCrcLibNumberList(pstCrcLibNumberList->num);
    if (ADM_IS_NULL(pstCrcLibNumberList->pcrcLibNumber))
    {
        log_e("Failed to new ecu CrcLibNumberList");
        return ADM_ECODE_NO_MEMORY;
    }

    for (i = 0; i < pstCrcLibNumberList->num; ++i) 
    {
        jsonArrayIndexObj = json_object_array_get_idx(jsonCrcLibNumberList, (D32U)i);
        if (ADM_IS_NULL(jsonArrayIndexObj)) 
        {
            iRet = ADM_ECODE_JSON_GET_ARRAYOBJECT_FAILED;
            log_e("Failed to backtracking Json array, arrayIndex[%d]", i);
            break;
        }

        //!< jsonArrayIndexObj 里面就一个成员，这里直接解
        iRet = __integrationTestGetJsonIntVal(jsonArrayIndexObj, ADM_ECS_CFG_CRCLIBNUMBER, &pstCrcLibNumberList->pcrcLibNumber[i]);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to get json value, key[%s], err[%d]", 
                        ADM_ECS_CFG_CRCLIBNUMBER, iRet);
            break;
        }
    }

    if (ADM_IS_ERR(iRet))
    {
        __integrationTestDelCrcLibNumberList(pstCrcLibNumberList->num, &pstCrcLibNumberList->pcrcLibNumber);
        pstCrcLibNumberList->num = 0;
    }

    return iRet;
}


/*!
 *******************************************************************************
 * 申请ecu did列表结构内存.
 *
 * \note 申请的为连续内存
 *
 * \param[in]  num 资源id个数
 * \retval     ADM_ECS_ECUDID_T*
 *******************************************************************************
 */
static ADM_ECS_ECUDID_T* __integrationTestNewEcuDidList(D32S num)
{
    ADM_ECS_ECUDID_T *pstEcuDidInfo = NULL;

    if (0 >= num)
    {
        return NULL;
    }

    pstEcuDidInfo = (ADM_ECS_ECUDID_T *)adm_malloc((D32U)num * sizeof(*pstEcuDidInfo));
    if (NULL != pstEcuDidInfo)
    {
        (void)adm_memzero(pstEcuDidInfo, (D32U)num * sizeof(*pstEcuDidInfo));
    }

    return pstEcuDidInfo;
}


/*!
 *******************************************************************************
 * 解析ecu资源id信息.
 *
 * \note
 * 
 * \param[in]   jsonResDid    Json resourceDid节点句柄
 * \param[out]  pstEcuDidInfo ecu资源id信息
 * \retval      D32S
 *******************************************************************************
 */
static D32S __integrationTestParseEcuDidInfo
(
    json_object      *jsonResDid, 
    ADM_ECS_ECUDID_T *pstEcuDidInfo
)
{
    D32S iRet = ADM_ECODE_SUCCESSED;

    if (ADM_IS_NULL(jsonResDid) || ADM_IS_NULL(pstEcuDidInfo))
    {
        log_e("Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }

    pstEcuDidInfo->did = __integrationTestGetJsonStrVal(jsonResDid, ADM_ECS_CFG_DID);
    if (ADM_IS_NULL(pstEcuDidInfo->did))
    {
        log_e("Failed to get json value, key[%s]", ADM_ECS_CFG_DID);
        return ADM_ECODE_JSON_GET_STRING_FAILED;
    }

    iRet = __integrationTestGetJsonIntVal(jsonResDid, ADM_ECS_CFG_START, &pstEcuDidInfo->start);
    if (ADM_IS_ERR(iRet))
    {
        log_e("Failed to get json value, key[%s], err[%d]", ADM_ECS_CFG_START, iRet);
        return iRet;
    }
    
    iRet = __integrationTestGetJsonIntVal(jsonResDid, ADM_ECS_CFG_END, &pstEcuDidInfo->end);
    if (ADM_IS_ERR(iRet))
    {
        log_e("Failed to get json value, key[%s], err[%d]", ADM_ECS_CFG_END, iRet);
        return iRet;
    }

    iRet = __integrationTestGetJsonIntVal(jsonResDid, ADM_ECS_CFG_DIDVALTYPE, &pstEcuDidInfo->didValType);
    if (ADM_IS_ERR(iRet))
    {
        log_e("Failed to get json value, key[%s], err[%d]", ADM_ECS_CFG_DIDVALTYPE, iRet);
        return iRet;
    }

    return iRet;
}


/*!
 *******************************************************************************
 * 释放ecu资源id列表结构内存.
 *
 * \note    释放的为连续内存
 *
 * \param[in]  num           资源id个数
 * \param[in]  pstEcuDidInfo 资源id列表信息
 * \retval     none
 *******************************************************************************
 */
static void __integrationTestDelEcuDidList
(
    D32S              num, 
    ADM_ECS_ECUDID_T **pstEcuDidInfo
)
{
    D32S i = 0;

    if ((0 >= num) || (ADM_IS_NULL(pstEcuDidInfo)) || (ADM_IS_NULL(*pstEcuDidInfo)))
    {
        return;
    }

    for (i = 0; i < num; i++)
    {
        adm_free((*pstEcuDidInfo)[i].did);
    }
    adm_free(*pstEcuDidInfo);

    return;
}


/*!
 *******************************************************************************
 * 解析资源id列表信息.
 *
 * \note
 * 
 * \param[in]   jsonRoot      Json 根节点句柄
 * \param[out]  pstResDidList 资源id列表信息
 * \retval      D32S
 *******************************************************************************
 */
static D32S __integrationTestParseEcuDidListInfo
(
    const  json_object          *jsonRoot, 
    ADM_ECS_ECUDIDLIST_T *pstEcuDidList
)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    D32S i = 0;
    json_type    jsonType      = json_type_null;
    json_object *jsonResDidList = NULL;
    json_object *jsonArrayIndexObj = NULL;

    if (ADM_IS_NULL(jsonRoot) || ADM_IS_NULL(pstEcuDidList))
    {
        log_e("Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }

    //!< 取json中resourceDidList节点信息
    if (!json_object_object_get_ex(jsonRoot, ADM_ECS_CFG_ECUDIDLIST, &jsonResDidList))
    {
        log_e("Failed to get json value, key[%s]", ADM_ECS_CFG_ECUDIDLIST);
        return ADM_ECODE_JSON_GET_KEY_FAILED;
    }

    jsonType = json_object_get_type(jsonResDidList);
    if (jsonType != json_type_array)
    {
        log_e("Json key[%s] type error", ADM_ECS_CFG_ECUDIDLIST);
        return ADM_ECODE_JSON_KEY_TYPE_ERROR;
    }

    pstEcuDidList->num = (D32S)json_object_array_length(jsonResDidList);
    if (0 == pstEcuDidList->num)
    {
        log_e("Json key[%s] array is zero", ADM_ECS_CFG_ECUDIDLIST);
        return ADM_ECODE_SUCCESSED;
    }

    pstEcuDidList->pstEcuDidInfo = __integrationTestNewEcuDidList(pstEcuDidList->num);
    if (ADM_IS_NULL(pstEcuDidList->pstEcuDidInfo))
    {
        log_e("Failed to new ecu ecuDidlist");
        return ADM_ECODE_NO_MEMORY;
    }

    for (i = 0; i < pstEcuDidList->num; ++i) 
    {
        jsonArrayIndexObj = json_object_array_get_idx(jsonResDidList, (D32U)i);
        if (ADM_IS_NULL(jsonArrayIndexObj)) 
        {
            iRet = ADM_ECODE_JSON_GET_ARRAYOBJECT_FAILED;
            log_e("Failed to backtracking Json array, arrayIndex[%d]", i);
            break;
        }

        iRet = __integrationTestParseEcuDidInfo(jsonArrayIndexObj, &pstEcuDidList->pstEcuDidInfo[i]);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to parse ecuDid info, err[%d]", iRet);
            break;
        }
    }

    if (ADM_IS_ERR(iRet))
    {
        __integrationTestDelEcuDidList(pstEcuDidList->num, &pstEcuDidList->pstEcuDidInfo);
    }

    return iRet;
}


/*!
 *******************************************************************************
 * 解析ecu配置信息.
 *
 * \note
 * 
 * \param[in]   jsonEcus   Json ecus节点句柄
 * \param[out]  pstEcuInfo ecu配置信息
 * \retval      D32S
 *******************************************************************************
 */
static D32S __integrationTestParseEcuConfigInfo
(
    json_object         *jsonEcus, 
    ADM_ECS_ECUCONFIG_T *pstEcuInfo
)
{
    D32S iRet = ADM_ECODE_SUCCESSED;

    if (ADM_IS_NULL(jsonEcus) || ADM_IS_NULL(pstEcuInfo))
    {
        log_e("Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }

    pstEcuInfo->ecuReqId = __integrationTestGetJsonStrVal(jsonEcus, ADM_COM_AIR_PROTOCOL_FIELD_ECUREQID);
    if (ADM_IS_NULL(pstEcuInfo->ecuReqId))
    {
        log_e("Failed to get json value, key[%s]", 
                    ADM_COM_AIR_PROTOCOL_FIELD_ECUREQID);
        return ADM_ECODE_JSON_GET_STRING_FAILED;
    }
	else
	{
        log_i("successful to get json value, key[%s] ,value :%s", 
                    ADM_COM_AIR_PROTOCOL_FIELD_ECUREQID,pstEcuInfo->ecuReqId);
    }

    pstEcuInfo->ecuSwDid = __integrationTestGetJsonStrVal(jsonEcus, ADM_COM_AIR_PROTOCOL_FIELD_ECUSWDID);
    if (ADM_IS_NULL(pstEcuInfo->ecuSwDid))
    {
        log_e("Failed to get json value, key[%s]", 
                    ADM_COM_AIR_PROTOCOL_FIELD_ECUSWDID);
        return ADM_ECODE_JSON_GET_STRING_FAILED;
    }
    
    pstEcuInfo->ecuResponseId = __integrationTestGetJsonStrVal(jsonEcus, ADM_ECS_CFG_ECURSPID);
    if (ADM_IS_NULL(pstEcuInfo->ecuResponseId))
    {
        log_e("Failed to get json value, key[%s]", ADM_ECS_CFG_ECURSPID);
        return ADM_ECODE_JSON_GET_STRING_FAILED;
    }

    pstEcuInfo->ecuFunctionId = __integrationTestGetJsonStrVal(jsonEcus, ADM_ECS_CFG_ECUFUNCID);
    if (ADM_IS_NULL(pstEcuInfo->ecuFunctionId))
    {
        log_e("Failed to get json value, key[%s]", ADM_ECS_CFG_ECUFUNCID);
        return ADM_ECODE_JSON_GET_STRING_FAILED;
    }

    pstEcuInfo->readInfoInstanceId = __integrationTestGetJsonStrVal(jsonEcus, ADM_ECS_CFG_READINFOISTID);
    if (ADM_IS_NULL(pstEcuInfo->readInfoInstanceId))
    {
        log_e("Failed to get json value, key[%s]", ADM_ECS_CFG_READINFOISTID);
        return ADM_ECODE_JSON_GET_STRING_FAILED;
    }

    pstEcuInfo->downloadInstanceId = __integrationTestGetJsonStrVal(jsonEcus, ADM_ECS_CFG_DOWNLOADISTID);
    if (ADM_IS_NULL(pstEcuInfo->downloadInstanceId))
    {
        log_e("Failed to get json value, key[%s]", ADM_ECS_CFG_DOWNLOADISTID);
        return ADM_ECODE_JSON_GET_STRING_FAILED;
    }
	else
	{
        log_i("successful to get json value, key[%s] ,value :%s", 
                    ADM_ECS_CFG_DOWNLOADISTID,pstEcuInfo->downloadInstanceId);
    }

    pstEcuInfo->transferInstanceId = __integrationTestGetJsonStrVal(jsonEcus, ADM_ECS_CFG_TRANSFERISTID);
    if (ADM_IS_NULL(pstEcuInfo->transferInstanceId))
    {
        log_e("Failed to get json value, key[%s]", ADM_ECS_CFG_TRANSFERISTID);
        return ADM_ECODE_JSON_GET_STRING_FAILED;
    }
	else
	{
        log_i("successful to get json value, key[%s] ,value :%s", 
                    ADM_ECS_CFG_TRANSFERISTID,pstEcuInfo->transferInstanceId);
    }

    pstEcuInfo->installInstanceId = __integrationTestGetJsonStrVal(jsonEcus, ADM_ECS_CFG_INSTALLISTID);
    if (ADM_IS_NULL(pstEcuInfo->installInstanceId))
    {
        log_e("Failed to get json value, key[%s]", ADM_ECS_CFG_INSTALLISTID);
        return ADM_ECODE_JSON_GET_STRING_FAILED;
    }
	else
	{
        log_i("successful to get json value, key[%s] ,value :%s", 
                    ADM_ECS_CFG_INSTALLISTID,pstEcuInfo->installInstanceId);
    }

    //!< get selfBackup
    iRet = __integrationTestGetJsonIntVal(jsonEcus, ADM_ECS_CFG_SELFBACKUP, &pstEcuInfo->selfBackup);
    if (ADM_IS_ERR(iRet))
    {
        log_e("Failed to get json value, key[%s], err[%d]", 
                    ADM_ECS_CFG_SELFBACKUP, iRet);
        return iRet;
    }

    //!< get selfBackup
    iRet = __integrationTestGetJsonIntVal(jsonEcus, ADM_ECS_CFG_ISSELFUPDATE, &pstEcuInfo->isSelfUpdate);
    if (ADM_IS_ERR(iRet))
    {
        log_e("Failed to get json value, key[%s], err[%d]", 
                    ADM_ECS_CFG_ISSELFUPDATE, iRet);
        return iRet;
    }

    iRet = __integrationTestGetJsonIntVal(jsonEcus, ADM_ECS_CFG_BUSTYPE, &pstEcuInfo->busType);
    if (ADM_IS_ERR(iRet))
    {
        log_e("Failed to get json value, key[%s], err[%d]", 
                    ADM_ECS_CFG_BUSTYPE, iRet);
        return iRet;
    }

    iRet = __integrationTestGetJsonIntVal(jsonEcus, ADM_ECS_CFG_CHANNELTYPE, &pstEcuInfo->channelType);
    if (ADM_IS_ERR(iRet))
    {
        log_e("Failed to get json value, key[%s], err[%d]", 
                    ADM_ECS_CFG_CHANNELTYPE, iRet);
        return iRet;
    }

    iRet = __integrationTestGetJsonIntVal(jsonEcus, ADM_ECS_CFG_CRCMASK, &pstEcuInfo->crcMask);
    if (ADM_IS_ERR(iRet))
    {
        log_e("Failed to get json value, key[%s], err[%d]", 
                    ADM_ECS_CFG_CRCMASK, iRet);
        return iRet;
    }

    iRet = __integrationTestGetJsonIntVal(jsonEcus, ADM_ECS_CFG_CRCLIBNUMBER, &pstEcuInfo->crcLibNumber);
    if (ADM_IS_ERR(iRet))
    {
        log_e("Failed to get json value, key[%s], err[%d]", 
                    ADM_ECS_CFG_CRCLIBNUMBER, iRet);
        return iRet;
    }

    iRet = __integrationTestGetJsonIntVal(jsonEcus, ADM_ECS_CFG_SAMASK, &pstEcuInfo->saMask);
    if (ADM_IS_ERR(iRet))
    {
        log_e("Failed to get json value, key[%s], err[%d]", 
                    ADM_ECS_CFG_SAMASK, iRet);
        return iRet;
    }

    iRet = __integrationTestGetJsonIntVal(jsonEcus, ADM_ECS_CFG_FLASHSCRIPTNUMBER, &pstEcuInfo->flashScriptNumber);
    if (ADM_IS_ERR(iRet))
    {
        log_e("Failed to get json value, key[%s], err[%d]", 
                    ADM_ECS_CFG_FLASHSCRIPTNUMBER, iRet);
        return iRet;
    }

    iRet = __integrationTestGetJsonIntVal(jsonEcus, ADM_ECS_CFG_ISCGWROUTED, &pstEcuInfo->isCGWRouted);
    if (ADM_IS_ERR(iRet))
    {
        log_e("Failed to get json value, key[%s], err[%d]", 
                    ADM_ECS_CFG_ISCGWROUTED, iRet);
        return iRet;
    }

    iRet = __integrationTestGetJsonIntVal(jsonEcus, ADM_ECS_CFG_SERVICETYPE, &pstEcuInfo->serviceType);
    if (ADM_IS_ERR(iRet))
    {
        log_e("Failed to get json value, key[%s], err[%d]", 
                    ADM_ECS_CFG_SERVICETYPE, iRet);
        return iRet;
    }

    iRet = __integrationTestGetJsonIntVal(jsonEcus, ADM_ECS_CFG_INSTCONDITION, &pstEcuInfo->instCondition);
    if (ADM_IS_ERR(iRet))
    {
        log_e("Failed to get json value, key[%s], err[%d]", 
                    ADM_ECS_CFG_INSTCONDITION, iRet);
        return iRet;
    }

    iRet = __integrationTestGetJsonIntVal(jsonEcus, ADM_ECS_CFG_SECURITYLEVEL, &pstEcuInfo->securityLevel);
    if (ADM_IS_ERR(iRet))
    {
        log_e("Failed to get json value, key[%s], err[%d]", 
                    ADM_ECS_CFG_SECURITYLEVEL, iRet);
        return iRet;
    }

    //!< 解析 saLibNumbers 列表信息
    iRet = __integrationTestParseSaLibNumberListInfo(jsonEcus, &pstEcuInfo->saLibNumberList);
    if (ADM_IS_ERR(iRet))
    {
        log_e("Failed to get json value, key[%s], err[%d]", 
                    ADM_ECS_CFG_SALIBNUMBER, iRet);
        return iRet;
    }

    //!< 解析 crcLibNumbers 列表信息
    iRet = __integrationTestParseCrcLibNumberListInfo(jsonEcus, &pstEcuInfo->crcLibNumberList);
    if (ADM_IS_ERR(iRet))
    {
        log_e("Failed to get json value, key[%s], err[%d]", 
                    ADM_ECS_CFG_CRCLIBNUMBERLIST, iRet);
        return iRet;
    }


    //!< 解析ecu id列表信息
    iRet = __integrationTestParseEcuDidListInfo(jsonEcus, &pstEcuInfo->stEcuDidList);
    if (ADM_IS_ERR(iRet))
    {
        log_e("Failed to parse ResourceDid, key[%s], err[%d]", 
                    ADM_ECS_CFG_ECUDIDLIST, iRet);
        return iRet;
    }

    //!< 取扩展字段解析
    iRet = __integrationTestParseJsonExtsInfo(jsonEcus, &pstEcuInfo->stExtList);
    if (ADM_IS_ERR(iRet))
    {
        log_e("Failed to parse exts, key[%s], err[%d]", 
                    ADM_COM_AIR_PROTOCOL_FIELD_EXT, iRet);
        return iRet;
    }

    return iRet;
}


/*!
 *******************************************************************************
 * 释放ecs ecus配置列表结构内存.
 *
 * \note    释放的为连续内存
 *
 * \param[in]  num        ecus配置个数
 * \param[in]  pstEcuList ecus配置列表信息
 * \retval     none
 *******************************************************************************
 */
static void __integrationTestDelEcusConfigList
(
    D32S num, 
    ADM_ECS_ECUCONFIG_T **pstEcuList
)
{
    D32S i = 0;

    if ((0 >= num) || (ADM_IS_NULL(pstEcuList)) || (ADM_IS_NULL(*pstEcuList)))
    {
        return;
    }

    for (i = 0; i < num; i++)
    {
        adm_free((*pstEcuList)[i].ecuReqId);
        adm_free((*pstEcuList)[i].ecuSwDid);
        adm_free((*pstEcuList)[i].ecuResponseId);
        adm_free((*pstEcuList)[i].ecuFunctionId);
        adm_free((*pstEcuList)[i].readInfoInstanceId);
        adm_free((*pstEcuList)[i].downloadInstanceId);        
        adm_free((*pstEcuList)[i].transferInstanceId);
        adm_free((*pstEcuList)[i].installInstanceId);

        __integrationTestCleanExtsList(&(*pstEcuList)[i].stExtList);
        __integrationTestDelSaLibNumberList((*pstEcuList)[i].saLibNumberList.num, 
                &(*pstEcuList)[i].saLibNumberList.psaLibNumber);
        __integrationTestDelCrcLibNumberList((*pstEcuList)[i].crcLibNumberList.num, 
                &(*pstEcuList)[i].crcLibNumberList.pcrcLibNumber);
        //!< 该ecu特有的ResourceDid信息
        __integrationTestDelEcuDidList((*pstEcuList)[i].stEcuDidList.num, 
                &(*pstEcuList)[i].stEcuDidList.pstEcuDidInfo);
    }
    adm_free(*pstEcuList);

    return;
}


/*!
 *******************************************************************************
 * 解析ecus配置信息.
 *
 * \note
 * 
 * \param[in]   ecusCfgData  ecus配置数据
 * \param[out]  pstEcsConfig ecus配置结构信息
 * \retval      D32S
 *******************************************************************************
 */
static D32S __integrationTestParsePartConfigInfo
(
    DCHAR                *ecusCfgData, 
    ADM_ECS_PARTCONFIG_T *pstPartCfg
)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    D32S i = 0;
    json_type    jsonType = json_type_null;
    json_object *jsonRoot = NULL;
    json_object *jsonEcus = NULL;
    json_object *jsonArrayIndexObj = NULL;

    if (ADM_IS_NULL(ecusCfgData) || ADM_IS_NULL(pstPartCfg))
    {
        log_e("Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }

    jsonRoot = json_tokener_parse(ecusCfgData);
    if (ADM_IS_NULL(jsonRoot)) 
    {
        log_e("Failed string to json, string[%s]", ecusCfgData);
        return ADM_ECODE_STRING_TO_JSON_FAILED;
    }

    //!< 取json中ecus节点信息
    if (!json_object_object_get_ex(jsonRoot, ADM_ECS_CFG_ECULIST, &jsonEcus))
    {
        (void)json_object_put(jsonRoot);
        log_e("Failed to get json value, key[%s]", ADM_ECS_CFG_ECULIST);
        return ADM_ECODE_JSON_GET_KEY_FAILED;
    }

    jsonType = json_object_get_type(jsonEcus);
    if (jsonType != json_type_array)
    {
        (void)json_object_put(jsonRoot);
        log_e("Json key[%s] type error", ADM_ECS_CFG_ECULIST);
        return ADM_ECODE_JSON_KEY_TYPE_ERROR;
    }

    pstPartCfg->stEcuCfgList.num = (D32S)json_object_array_length(jsonEcus);
    if (0 == pstPartCfg->stEcuCfgList.num)
    {
        (void)json_object_put(jsonRoot);
        log_e("Json key[%s] array is zero", ADM_ECS_CFG_ECULIST);
        return ADM_ECODE_SUCCESSED;
    }

    pstPartCfg->stEcuCfgList.pstEcuCfg = __integrationTestNewEcusConfigList(pstPartCfg->stEcuCfgList.num);
    if (ADM_IS_NULL(pstPartCfg->stEcuCfgList.pstEcuCfg))
    {
        (void)json_object_put(jsonRoot);
        pstPartCfg->stEcuCfgList.num = 0;
        log_e("Failed to new ecu config list");
        return ADM_ECODE_NO_MEMORY;
    }

    for (i = 0; i < pstPartCfg->stEcuCfgList.num; ++i) 
    {
        jsonArrayIndexObj = json_object_array_get_idx(jsonEcus, (D32U)i);
        if (ADM_IS_NULL(jsonArrayIndexObj)) 
        {
            iRet = ADM_ECODE_JSON_GET_ARRAYOBJECT_FAILED;
            log_e("Failed to backtracking Json array, arrayIndex[%d]", i);
            break;
        }

        iRet = __integrationTestParseEcuConfigInfo(jsonArrayIndexObj, &pstPartCfg->stEcuCfgList.pstEcuCfg[i]);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to parse saLib info, err[%d]", iRet);
            break;
        }
    }

    if (ADM_IS_ERR(iRet))
    {
        (void)json_object_put(jsonRoot);
        __integrationTestDelEcusConfigList(pstPartCfg->stEcuCfgList.num, &pstPartCfg->stEcuCfgList.pstEcuCfg);
        pstPartCfg->stEcuCfgList.num = 0;
        return iRet;
    }

    //!< 取扩展字段解析
    iRet = __integrationTestParseJsonExtsInfo(jsonRoot, &pstPartCfg->stExtList);
    if (ADM_IS_ERR(iRet))
    {
        (void)json_object_put(jsonRoot);
        __integrationTestDelEcusConfigList(pstPartCfg->stEcuCfgList.num, &pstPartCfg->stEcuCfgList.pstEcuCfg);
        pstPartCfg->stEcuCfgList.num = 0;
        pstPartCfg->stEcuCfgList.pstEcuCfg = NULL;
        log_e("Failed to parse exts, key[%s], err[%d]", 
                    ADM_COM_AIR_PROTOCOL_FIELD_EXT, iRet);
        return iRet;
    }
    (void)json_object_put(jsonRoot);

    return iRet;
}


/*!
 *******************************************************************************
 * 获取平台配置信息
 *
 * \note 
 *
 * \param[in]   pstEcsRsConfigData      平台配置信息指针
 * 
 * \retval      D32S  获取结果
 *******************************************************************************
 */
D32S adm_integrationtest_getEcsRsConfigData(ADM_ECS_RSCONFIGDATA_T* pstEcsRsConfigData)
{
    //ADM_ECS_RSCONFIGINFO_T* pstConfigInfo = NULL;
	DCHAR acFilePath[ADM_IPM_TEST_FILE_PATH_LENGTH] = { 0 };
    json_object* pRoot = NULL;
	D32S iRet = ADM_ECODE_FAILED;
	DCHAR* pcFileReadbuf = NULL;

	if (NULL == pstEcsRsConfigData)
	{
		return ADM_ECODE_FAILED;
	}

	DCHAR* pcConfigFileDir = __integrationTestGetConfigFileDir();
	if (NULL == pcConfigFileDir)
	{
	    return ADM_ECODE_FAILED;
	}
	do
	{
        /* 解析通用配置信息 */
		(void)adm_snprintf(acFilePath, sizeof(acFilePath), "%s/%s", pcConfigFileDir, ADM_IPM_TEST_GENERALCONFIG_FILE);
		
		pcFileReadbuf = adm_integrationtest_readFile(acFilePath);
		if (NULL == pcFileReadbuf)
		{
		    return ADM_ECODE_FAILED;
		}

		pRoot = json_tokener_parse(pcFileReadbuf);
		if (NULL == pRoot) 
		{
	        log_e("str to son_object. failed");
	        iRet = ADM_ECODE_STRING_TO_JSON_FAILED;
			break;
	    }

		
		//!< 解析confVersion
        //pstEcsRsConfigData->confVersion = adm_com_getJsonStrVal(pRoot, ADM_COM_AIR_PROTOCOL_FIELD_CONFVERSION);
        //if (ADM_IS_NULL(pstEcsRsConfigData->confVersion))
        //{
        //    log_e("Failed to get json value, key[%s]", ADM_COM_AIR_PROTOCOL_FIELD_CONFVERSION);
        //    iRet = ADM_ECODE_JSON_GET_STRING_FAILED;
        //    break;
        //}
	    //pstConfigInfo = &pstEcsRsConfigData->stRsCfgInfo;

		iRet = __integrationTestGetGeneralConfigData(pRoot, &pstEcsRsConfigData->stRsCfgInfo.stGeneralCfg);
		if(ADM_ECODE_SUCCESSED != iRet)
		{
		     log_e("parse general config data failed");
             (void)json_object_put(pRoot);
			 break;
		}

        (void)json_object_put(pRoot);
		adm_free(pcFileReadbuf);

		/* 解析零件配置信息 */
		(void)adm_memset(acFilePath, 0x00, ADM_IPM_TEST_FILE_PATH_LENGTH,ADM_IPM_TEST_FILE_PATH_LENGTH);
		(void)adm_snprintf(acFilePath, sizeof(acFilePath), "%s/%s", pcConfigFileDir, ADM_IPM_TEST_PARTCONFIG_FILE);
				
		pcFileReadbuf = adm_integrationtest_readFile(acFilePath);
		if (NULL == pcFileReadbuf)
		{
		    log_e("read file %s failed",acFilePath);
			iRet = ADM_ECODE_READ_FILE_FAILED;
			break;
		}

		iRet = __integrationTestParsePartConfigInfo(pcFileReadbuf, &pstEcsRsConfigData->stRsCfgInfo.stPartCfg);
		if(ADM_ECODE_SUCCESSED != iRet) 
		{
		     log_e("parse general config data failed");
			 break;
		}
	
	
	}while(FALSE);

    adm_free(pcFileReadbuf);
	
    return iRet;
}

//test code
#if 0
static D32S __integrationTestCreateDir(const DCHAR* pDirPath)
{
    if (NULL == pDirPath) {
        log_e( "Parameter verification failed");
        return ADM_ECODE_INVALID_PARAM;
    }

    DCHAR dirName[256] = {0};
    (void)adm_memset(dirName, 0x00, sizeof(dirName), sizeof(dirName));
    (void)adm_strncpy(dirName, pDirPath, adm_strlen(pDirPath), sizeof(dirName));
    D32S i, len = (D32S)adm_strlen(dirName);

    for(i = 1; i < len; i++)
    {
        if(dirName[i] == '/')
        {
            dirName[i] = '\0';
            if(access(dirName, F_OK) != 0)
            {
                if(mkdir(dirName, MODE755) == -1)
                {
                    log_e( "dirName:%s mkdir failed", dirName);
                    return ADM_ECODE_CREATE_DIR_FAILED;
                }
            }
            dirName[i] = '/';
        }
    }

    if (len > 0 && access(dirName, F_OK) != 0) {
        (void)mkdir(dirName, MODE755);
    }

    return ADM_ECODE_SUCCESSED;
}

int main(void) 
{
    ADM_ECS_RSCONFIGDATA_T stEcsRsConfigData = {0};
	D32S iRet = ADM_ECODE_FAILED;
	
    //adm_initLog();
    __integrationTestCreateDir("../debug");
    adm_debug_logInit("../debug/", (1 * 1024 * 1024), 5);

	adm_integrationtest_setConfigFileDir("../configdir");

	iRet = adm_integrationtest_getEcsRsConfigData(&stEcsRsConfigData);
	if(ADM_ECODE_SUCCESSED != iRet)
	{
	     log_e("parse general config data failed");
		 
	}
	else
    {
	     log_i("parse general config data successful");
		 
	}
	return iRet;

}
#endif


