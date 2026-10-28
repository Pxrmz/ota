#include "adm_cvm_parseJson.h"
#include "adm_memory.h"
#include "adm_file.h"
#include "adm_string.h"

static DCHAR g_othertConfigFileDir[ADM_CVM_GTEST_FILE_MAX_LEN] = {0};             //集成测试配置文件目录

/*!
 *******************************************************************************
 * 设置集成测试上报配置文件目录
 *
 * \note 
 *
 * \param[in]   pcConfigFileDir      集成测试配置文件目录
 * 
 * \retval      无
 *******************************************************************************
 */
void adm_cvm_GtestSetOtherConfFileDir(DCHAR* pcConfigFileDir)
{
    adm_memcpy(g_othertConfigFileDir, pcConfigFileDir, ADM_CVM_GTEST_FILE_MAX_LEN, ADM_CVM_GTEST_FILE_MAX_LEN);
}

/*!
 *******************************************************************************
 * 获取集成测试测试配置文件（testConfig.json）目录
 *
 * \note 
 *
 * \retval      集成测试配置文件目录
 *******************************************************************************
 */
DCHAR* adm_cvm_GtestGetOtherConfFileDir(void)
{
    return g_othertConfigFileDir;
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
static D32S adm_cvm_GtestGetJsonIntVal(const json_object *jsonObj, const char *key, D32S *val)
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
static D32S adm_cvm_GtestGetJsonUInt64Val(const json_object *jsonObj, const char *key, DSIZE *val)
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

    *val = json_object_get_uint64(jsonStr);
    
    return ADM_ECODE_SUCCESSED;
}

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
static DCHAR* adm_cvm_GtestGetJsonStrVal(const json_object *jsonObj, const char *key)
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
 * 获取上报配置信息配置参数
 *
 * \note 
 *
 * \param[in]   pRoot      json数据
 * \param[in]   pstGeneralConfig      通用配置信息指针
 * 
 * \retval      D32S  执行结果
 *******************************************************************************
 */
static D32S cvmGtestGetTestConfigData(json_object* pjsonRoot, ADM_CVM_TEST_OTHER_INFO_CONF_T* pstOtherConfigData)
{
    D32S iRet = ADM_ECODE_FAILED;
    json_object *pjsonEventreport = NULL;
    json_object *pjsonBigVersion = NULL;
    json_object *pjsonconfVersion = NULL;
    json_object *pjsonBusiCommInter = NULL;

    if ((NULL == pjsonRoot)||(NULL == pstOtherConfigData))
    {
        return ADM_ECODE_FAILED;
    }

    do
    {
        if (!json_object_object_get_ex(pjsonRoot, ADM_CVM_TEST_GET_EVENT_REPORT_RET, &pjsonEventreport))
        {
            (void)json_object_put(pjsonRoot);
            log_e("Failed to get json value, key[%s]", ADM_CVM_TEST_GET_EVENT_REPORT_RET);
            return ADM_ECODE_JSON_GET_KEY_FAILED;
        }

        iRet = adm_cvm_GtestGetJsonIntVal(pjsonEventreport, ADM_CVM_TEST_FUNCTION_RET, &pstOtherConfigData->stEvent_Report.ret);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to get json value, key[%s], err[%d]", ADM_CVM_TEST_FUNCTION_RET, iRet);
            return ADM_ECODE_JSON_GET_INT_FAILED;
        }
        else
        {
            log_i("successful to get json value, key[%s],value: %d", ADM_CVM_TEST_FUNCTION_RET, pstOtherConfigData->stEvent_Report.ret);
        }

        iRet = adm_cvm_GtestGetJsonIntVal(pjsonEventreport, ADM_CVM_TEST_EVENT_REPORT_RET, &pstOtherConfigData->stEvent_Report.reportLevel);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to get json value, key[%s], err[%d]", ADM_CVM_TEST_EVENT_REPORT_RET, iRet);
            return ADM_ECODE_JSON_GET_INT64_FAILED;
        }
        else
        {
            log_i("successful to get json value, key[%s],value: %d", ADM_CVM_TEST_EVENT_REPORT_RET, pstOtherConfigData->stEvent_Report.reportLevel);
        }   


        if (!json_object_object_get_ex(pjsonRoot, ADM_CVM_TEST_GET_RSCONFVERSION, &pjsonconfVersion))
        {
            (void)json_object_put(pjsonRoot);
            log_e("Failed to get json value, key[%s]", ADM_CVM_TEST_GET_RSCONFVERSION);
            return ADM_ECODE_JSON_GET_KEY_FAILED;
        }

        iRet = adm_cvm_GtestGetJsonIntVal(pjsonconfVersion, ADM_CVM_TEST_FUNCTION_RET, &pstOtherConfigData->stConfVer.ret);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to get json value, key[%s], err[%d]", ADM_CVM_TEST_FUNCTION_RET, iRet);
            return ADM_ECODE_JSON_GET_INT_FAILED;
        }
        else
        {
            log_i("successful to get json value, key[%s],value: %d", ADM_CVM_TEST_FUNCTION_RET, pstOtherConfigData->stConfVer.ret);
        }

        pstOtherConfigData->stConfVer.configVersion = adm_cvm_GtestGetJsonStrVal(pjsonconfVersion, ADM_CVM_TEST_CONFVERSION);
        if (ADM_IS_NULL(pstOtherConfigData->stConfVer.configVersion))
        {
            log_e("Failed to get json value, key[%s]", ADM_CVM_TEST_CONFVERSION);
            return ADM_ECODE_JSON_GET_STRING_FAILED;
        }

        // 解析adm_irm_reportVehicleState的结构数据
        // pstOtherConfigData->ConfigVersion = adm_cvm_GtestGetJsonStrVal(pjsonRoot, ADM_CVM_TEST_GET_RSCONFVERSION);
        // if (ADM_IS_NULL(pstOtherConfigData->ConfigVersion))
        // {
        //     log_e("Failed to get json value, key[%s]", ADM_CVM_TEST_GET_RSCONFVERSION);
        //     return ADM_ECODE_JSON_GET_STRING_FAILED;
        // }

        if (!json_object_object_get_ex(pjsonRoot, ADM_CVM_TEST_GET_VEHICLEBIGVERSION, &pjsonBigVersion))
        {
            (void)json_object_put(pjsonRoot);
            log_e("Failed to get json value, key[%s]", ADM_CVM_TEST_GET_VEHICLEBIGVERSION);
            return ADM_ECODE_JSON_GET_KEY_FAILED;
        }

        iRet = adm_cvm_GtestGetJsonIntVal(pjsonBigVersion, ADM_CVM_TEST_FUNCTION_RET, &pstOtherConfigData->bigVersion.ret);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to get json value, key[%s], err[%d]", ADM_CVM_TEST_FUNCTION_RET, iRet);
            return ADM_ECODE_JSON_GET_INT_FAILED;
        }
        else
        {
            log_i("successful to get json value, key[%s],value: %d", ADM_CVM_TEST_FUNCTION_RET, pstOtherConfigData->bigVersion.ret);
        }

        pstOtherConfigData->bigVersion.vehicleBigVersion = adm_cvm_GtestGetJsonStrVal(pjsonBigVersion, ADM_CVM_TEST_VEHICLEBIGVERSION);
        if (ADM_IS_NULL(pstOtherConfigData->bigVersion.vehicleBigVersion))
        {
            log_e("Failed to get json value, key[%s]", ADM_CVM_TEST_VEHICLEBIGVERSION);
            return ADM_ECODE_JSON_GET_STRING_FAILED;
        }

        iRet = adm_cvm_GtestGetJsonIntVal(pjsonRoot, ADM_CVM_TEST_NEW_CHECK_RESP_ADAPBODY, &pstOtherConfigData->newCheckRespAdapterBody_ret);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to get json value, key[%s], err[%d]", ADM_CVM_TEST_NEW_CHECK_RESP_ADAPBODY, iRet);
            return ADM_ECODE_JSON_GET_INT_FAILED;
        }
        else
        {
            log_i("successful to get json value, key[%s],value: %d", ADM_CVM_TEST_NEW_CHECK_RESP_ADAPBODY, pstOtherConfigData->newCheckRespAdapterBody_ret);
        }

        iRet = adm_cvm_GtestGetJsonIntVal(pjsonRoot, ADM_CVM_TEST_NEW_CHECK_REQ_ADAPBODY, &pstOtherConfigData->newCheckReqAdapterBody_ret);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to get json value, key[%s], err[%d]", ADM_CVM_TEST_NEW_CHECK_REQ_ADAPBODY, iRet);
            return ADM_ECODE_JSON_GET_INT_FAILED;
        }
        else
        {
            log_i("successful to get json value, key[%s],value: %d", ADM_CVM_TEST_NEW_CHECK_REQ_ADAPBODY, pstOtherConfigData->newCheckReqAdapterBody_ret);
        }

        iRet = adm_cvm_GtestGetJsonIntVal(pjsonRoot, ADM_CVM_TEST_CHECK_PRO_ADP, &pstOtherConfigData->checkProtocolAdapter_ret);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to get json value, key[%s], err[%d]", ADM_CVM_TEST_CHECK_PRO_ADP, iRet);
            return ADM_ECODE_JSON_GET_INT_FAILED;
        }
        else
        {
            log_i("successful to get json value, key[%s],value: %d", ADM_CVM_TEST_CHECK_PRO_ADP, pstOtherConfigData->checkProtocolAdapter_ret);
        }    

        iRet = adm_cvm_GtestGetJsonIntVal(pjsonRoot, ADM_CVM_TEST_CHECK_PRO_PARSE, &pstOtherConfigData->checkProtocolParse_ret);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to get json value, key[%s], err[%d]", ADM_CVM_TEST_CHECK_PRO_PARSE, iRet);
            return ADM_ECODE_JSON_GET_INT_FAILED;
        }
        else
        {
            log_i("successful to get json value, key[%s],value: %d", ADM_CVM_TEST_CHECK_PRO_PARSE, pstOtherConfigData->checkProtocolParse_ret);
        }   

        if (!json_object_object_get_ex(pjsonRoot, ADM_CVM_TEST_BUSI_COMM_INTER, &pjsonBusiCommInter))
        {
            (void)json_object_put(pjsonRoot);
            log_e("Failed to get json value, key[%s]", ADM_CVM_TEST_BUSI_COMM_INTER);
            return ADM_ECODE_JSON_GET_KEY_FAILED;
        }

        iRet = adm_cvm_GtestGetJsonIntVal(pjsonBusiCommInter, ADM_CVM_TEST_FUNCTION_RET, &pstOtherConfigData->Busi_Comm_inter_ret.ret);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to get json value, key[%s], err[%d]", ADM_CVM_TEST_FUNCTION_RET, iRet);
            return ADM_ECODE_JSON_GET_INT_FAILED;
        }
        else
        {
            log_i("successful to get json value, key[%s],value: %d", ADM_CVM_TEST_FUNCTION_RET, pstOtherConfigData->Busi_Comm_inter_ret.ret);
        }

        iRet = adm_cvm_GtestGetJsonUInt64Val(pjsonBusiCommInter, ADM_CVM_TEST_RSPSTATUSCODE, &pstOtherConfigData->Busi_Comm_inter_ret.rspStatusCode);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to get json value, key[%s], err[%d]", ADM_CVM_TEST_RSPSTATUSCODE, iRet);
            return ADM_ECODE_JSON_GET_INT64_FAILED;
        }
        else
        {
            log_i("successful to get json value, key[%s],value: %d", ADM_CVM_TEST_RSPSTATUSCODE, pstOtherConfigData->Busi_Comm_inter_ret.rspStatusCode);
        }   

        iRet = adm_cvm_GtestGetJsonIntVal(pjsonRoot, ADM_CVM_TEST_DO_DOWNLOAD_FILE, &pstOtherConfigData->doDownloadFile_ret);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to get json value, key[%s], err[%d]", ADM_CVM_TEST_DO_DOWNLOAD_FILE, iRet);
            return ADM_ECODE_JSON_GET_INT_FAILED;
        }
        else
        {
            log_i("successful to get json value, key[%s],value: %d", ADM_CVM_TEST_DO_DOWNLOAD_FILE, pstOtherConfigData->doDownloadFile_ret);
        }   

        iRet = adm_cvm_GtestGetJsonIntVal(pjsonRoot, ADM_CVM_TEST_DECRYPT_AND_VERIF, &pstOtherConfigData->decryptAndVerificationSignature_ret);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to get json value, key[%s], err[%d]", ADM_CVM_TEST_DECRYPT_AND_VERIF, iRet);
            return ADM_ECODE_JSON_GET_INT_FAILED;
        }
        else
        {
            log_i("successful to get json value, key[%s],value: %d", ADM_CVM_TEST_DECRYPT_AND_VERIF, pstOtherConfigData->decryptAndVerificationSignature_ret);
        }   


        iRet = adm_cvm_GtestGetJsonIntVal(pjsonRoot, ADM_CVM_TEST_EIM_ECUINFO_KV, &pstOtherConfigData->EcuInfoManagerKv_ret);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to get json value, key[%s], err[%d]", ADM_CVM_TEST_EIM_ECUINFO_KV, iRet);
            return ADM_ECODE_JSON_GET_INT_FAILED;
        }
        else
        {
            log_i("successful to get json value, key[%s],value: %d", ADM_CVM_TEST_EIM_ECUINFO_KV, pstOtherConfigData->EcuInfoManagerKv_ret);
        }  

        iRet = adm_cvm_GtestGetJsonIntVal(pjsonRoot, ADM_CVM_TEST_EIM_ECUINFO_DIM, &pstOtherConfigData->EcuInfoManagerDim_ret);
        if (ADM_IS_ERR(iRet))
        {
            log_e("Failed to get json value, key[%s], err[%d]", ADM_CVM_TEST_EIM_ECUINFO_DIM, iRet);
            return ADM_ECODE_JSON_GET_INT_FAILED;
        }
        else
        {
            log_i("successful to get json value, key[%s],value: %d", ADM_CVM_TEST_EIM_ECUINFO_DIM, pstOtherConfigData->EcuInfoManagerDim_ret);
        }  
    }while(FALSE);

    return iRet;
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
DCHAR* adm_cvm_GtestReadConfFile(const DCHAR* pcFileName)
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

D32S adm_cvm_GtestGetOtherInfoConfig(ADM_CVM_TEST_OTHER_INFO_CONF_T* pstOtherConfigData)
{
    DCHAR acFilePath[ADM_CVM_GTEST_FILE_MAX_LEN] = { 0 };
    json_object* pRoot = NULL;
    D32S iRet = ADM_ECODE_FAILED;
    DCHAR* pcFileReadbuf = NULL;

    DCHAR* pcConfigFileDir = adm_cvm_GtestGetOtherConfFileDir();
    if (NULL == pcConfigFileDir)
    {
        log_e(" Test Json Config File Not Found \n");
        return ADM_ECODE_FAILED;
    }
    do
    {
        /* 解析test配置信息 */
        (void)adm_snprintf(acFilePath, sizeof(acFilePath), "%s/%s", pcConfigFileDir, ADM_CVM_GTEST_TESTCONFIG_FILE);
        pcFileReadbuf = adm_cvm_GtestReadConfFile(acFilePath);
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

        iRet = cvmGtestGetTestConfigData(pRoot, pstOtherConfigData);
        if(ADM_ECODE_SUCCESSED != iRet)
        {
            log_e("parse test config data failed");
            (void)json_object_put(pRoot);
            break;
        }

        (void)json_object_put(pRoot);
        adm_free(pcFileReadbuf);

    }while(FALSE);

    adm_free(pcFileReadbuf);
    return iRet;
}