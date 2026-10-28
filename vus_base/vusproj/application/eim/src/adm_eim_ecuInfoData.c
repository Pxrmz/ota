#include "adm_eim_log.h"
#include "adm_eim_getEcuInfo.h"
#include "adm_com_errorCode.h"
#include "json.h"
#include "adm_memory.h"
#include "adm_string.h"

static const D32S g_dataTypeMaxLength = 40;

ECU_INFO_T* adm_eim_newEcuInfoBody(void)
{
    ECU_INFO_T* ecuInfo = NULL;

    ecuInfo = (ECU_INFO_T*)adm_malloc(sizeof(ECU_INFO_T));
    if (NULL != ecuInfo) {
        (void)adm_memset(ecuInfo, 0x00, sizeof(ECU_INFO_T), sizeof(ECU_INFO_T));
        ecuInfo->ecuReqId = NULL;
        ecuInfo->ecuSwDid = NULL;
        ecuInfo->vdiDidListResponse = NULL;
    }
    return ecuInfo;
}

void adm_eim_deleteEcuInfoBody(ECU_INFO_T* ecuInfo)
{
    if (NULL == ecuInfo) {
        return;
    }
    adm_free(ecuInfo->ecuReqId);
    adm_free(ecuInfo->ecuSwDid);
    adm_free(ecuInfo->partNumber);
    adm_free(ecuInfo->partSupplier);
    adm_free(ecuInfo->softwarePartNumber);
    adm_free(ecuInfo->partHardwareNum);

    if (NULL == ecuInfo->vdiDidListResponse) {
        adm_free(ecuInfo);
        return;
    }

    if (NULL != ecuInfo->vdiDidListResponse->didArray)
    {
        for (D16U i = 0; i < ecuInfo->vdiDidListResponse->didArrayLength; i++)
        {
            ADM_DIM_SOA_DID_DATA_T* pDidArray = ecuInfo->vdiDidListResponse->didArray[i];
            if (NULL != pDidArray->didData)
            {
                if (VAR_STRING == pDidArray->didData->dataType)
                {
                    adm_free(pDidArray->didData->v.valueS);
                }
                adm_free(pDidArray->didData);
            }

            adm_free(pDidArray);
        }
    }
    adm_free(ecuInfo->vdiDidListResponse->didArray);
    adm_free(ecuInfo->vdiDidListResponse);
    adm_free(ecuInfo);
}

ADM_EIM_GETECUINFO_T* adm_eim_newGetEcuInfoBody(void)
{
    ADM_EIM_GETECUINFO_T* getEcuInfo = NULL;
    getEcuInfo = (ADM_EIM_GETECUINFO_T*)adm_malloc(sizeof(ADM_EIM_GETECUINFO_T));
    if (NULL != getEcuInfo) {
        (void)adm_memset(getEcuInfo, 0x00, sizeof(ADM_EIM_GETECUINFO_T), sizeof(ADM_EIM_GETECUINFO_T));
        getEcuInfo->num = 0;
        getEcuInfo->ecuInfo = NULL;
    }
    return getEcuInfo;
}

void adm_eim_deleteGetEcuInfoBody(ADM_EIM_GETECUINFO_T* getEcuInfo)
{
    if (NULL == getEcuInfo) {
        return;
    }

    for (D32S i = 0; i < getEcuInfo->num; ++i) {
        if (NULL != getEcuInfo->ecuInfo[i]) {
            adm_eim_deleteEcuInfoBody(getEcuInfo->ecuInfo[i]);
        }
    }
    adm_free(getEcuInfo->ecuInfo);
    adm_free(getEcuInfo);
}

static DCHAR* adm_eim_tlvTostring(ADM_DIM_SOA_DATA_T* data)
{
    if (NULL == data) {
        return NULL;
    }

    DCHAR* dataStr = NULL;
    dataStr = (DCHAR*)adm_malloc(g_dataTypeMaxLength);

    if ((VARIABLE_TYPE_EN)data->dataType == VAR_U8) {
        (void)adm_snprintf(dataStr, g_dataTypeMaxLength, "%d", data->v.value[0]);
    } else if ((VARIABLE_TYPE_EN)data->dataType == VAR_U32) {
        (void)adm_snprintf(dataStr, g_dataTypeMaxLength, "%d", data->v.valueU32);
    } else if ((VARIABLE_TYPE_EN)data->dataType == VAR_S32) {
        (void)adm_snprintf(dataStr, g_dataTypeMaxLength, "%d", data->v.valueS32);
    } else if ((VARIABLE_TYPE_EN)data->dataType == VAR_U64) {
        (void)adm_snprintf(dataStr, g_dataTypeMaxLength, "%llu", data->v.valueU64);
    } else if ((VARIABLE_TYPE_EN)data->dataType == VAR_FLOAT) {
        (void)adm_snprintf(dataStr, g_dataTypeMaxLength, "%f", data->v.valueF);
    } else if ((VARIABLE_TYPE_EN)data->dataType == VAR_STRING) {
        adm_free(dataStr);
        dataStr = adm_strdup((DCHAR*)data->v.valueS);
    } else if ((VARIABLE_TYPE_EN)data->dataType == VAR_S64) {
        (void)adm_snprintf(dataStr, g_dataTypeMaxLength, "%lld", data->v.valueS64);
    } else {
        log_e("invalid data type = %d", (VARIABLE_TYPE_EN)data->dataType);
        adm_free(dataStr);
        return NULL;
    }
    return dataStr;
}

/***************************************ecuInfo to str**********************************************/
static DBOOL adm_eim_addDidToEcuObject(json_object* ecuInfoObject,
    const ADM_DIM_SOA_DID_DATA_ARRAY_T* vdiDidListResponse)
{
    if (NULL == ecuInfoObject || NULL == vdiDidListResponse || 0U == vdiDidListResponse->didArrayLength) {
        return FALSE;
    }
    json_object* didObjectArray = NULL;
    json_object* didObject = NULL;
    json_object* stringObject = NULL;
    json_object* intObject = NULL;
    DCHAR* dataStr = NULL;

    didObjectArray = json_object_new_array();
    if (NULL == didObjectArray)
    {
        return FALSE;
    }

    for (D16U i = 0; i < vdiDidListResponse->didArrayLength; ++i) {
        if (NULL != vdiDidListResponse->didArray[i] && 0 != vdiDidListResponse->didArray[i]->did
            && NULL != vdiDidListResponse->didArray[i]->didData) {
            didObject = json_object_new_object();
            if (NULL == didObject)
            {
                (void)json_object_put(didObjectArray);
                return FALSE;
            }

            /*set did*/
            dataStr = (DCHAR*)adm_malloc(g_dataTypeMaxLength);
            if (NULL == dataStr)
            {
                (void)json_object_put(didObjectArray);
                return FALSE;
            }
            (void)adm_snprintf(dataStr, g_dataTypeMaxLength, "0X%04X", vdiDidListResponse->didArray[i]->did);
            stringObject = json_object_new_string(dataStr);
            if (NULL == stringObject)
            {
                (void)json_object_put(didObjectArray);
                return FALSE;
            }

            (void)json_object_object_add(didObject, ADM_COM_AIR_PROTOCOL_FIELD_DID, stringObject);
            adm_free(dataStr);

            /*set didValue. if null also set null*/
            dataStr = adm_eim_tlvTostring(vdiDidListResponse->didArray[i]->didData);
            if (NULL == dataStr) 
            {
                log_w("did[%d] adm_eim_tlvTostring get null string.", vdiDidListResponse->didArray[i]->did);
                
                /** TODO:maxiangyang 等待云端协议支持null,再给null */
                // stringObject = json_object_new_null();
                stringObject = json_object_new_string("dafault");
            }
            else 
            {
                stringObject = json_object_new_string(dataStr);
                if (NULL == stringObject)
                {
                    (void)json_object_put(didObjectArray);
                    return FALSE;
                }
            }
            
            (void)json_object_object_add(didObject, ADM_COM_AIR_PROTOCOL_FIELD_DIDVALUE, stringObject);
            adm_free(dataStr);

            /*start*/
            intObject = json_object_new_int((int32_t)vdiDidListResponse->didArray[i]->startByte);
            if (NULL == intObject)
            {
                (void)json_object_put(didObjectArray);
                return FALSE;
            }
            (void)json_object_object_add(didObject, ADM_COM_AIR_PROTOCOL_FIELD_START, intObject);

            /*end*/
            intObject = json_object_new_int((int32_t)vdiDidListResponse->didArray[i]->endByte);
            if (NULL == intObject)
            {
                (void)json_object_put(didObjectArray);
                return FALSE;
            }
            (void)json_object_object_add(didObject, ADM_COM_AIR_PROTOCOL_FIELD_END, intObject);

            (void)json_object_array_add(didObjectArray, didObject);
        } else {
            log_w("number %d is NULL", i);
        }
    }

    if (0U == json_object_array_length(didObjectArray)) {
        log_e("ecuDids is empty");
        (void)json_object_put(didObjectArray);
        return FALSE;
    } else {
        (void)json_object_object_add(ecuInfoObject, ADM_COM_AIR_PROTOCOL_FIELD_ECUDIDS, didObjectArray);
    }
    return TRUE;
}

static DBOOL adm_eim_addReqIdSwDidToEcuObject(json_object* ecuInfoObject, const DCHAR* ecuReqId, const DCHAR* ecuSwDid)
{
    if (NULL == ecuInfoObject || NULL == ecuReqId || NULL == ecuSwDid) {
        return FALSE;
    }
    json_object* stringObject = NULL;
    /*set ecuReqId*/
    stringObject = json_object_new_string(ecuReqId);
    (void)json_object_object_add(ecuInfoObject, ADM_COM_AIR_PROTOCOL_FIELD_ECUREQID, stringObject);
    /*set ecuSwDid*/
    stringObject = json_object_new_string(ecuSwDid);
    (void)json_object_object_add(ecuInfoObject, ADM_COM_AIR_PROTOCOL_FIELD_ECUSWDID, stringObject);
    return TRUE;
}

static DBOOL adm_eim_addPartInfoToEcuObject(json_object* ecuInfoObject, const DCHAR* partNum, const DCHAR* partSupplier, 
                                            const DCHAR* softwarePartNumber,const DCHAR* partHardwareNum)
{
    if (NULL == ecuInfoObject || NULL == partNum || NULL == partSupplier || NULL == partHardwareNum) 
    {
        return FALSE;
    }
    json_object* stringObject = NULL;
    /*set ecuReqId*/
    stringObject = json_object_new_string(partNum);
    (void)json_object_object_add(ecuInfoObject, ADM_COM_AIR_PROTOCOL_FIELD_PARTNUM, stringObject);
    /*set ecuSwDid*/
    stringObject = json_object_new_string(partSupplier);
    (void)json_object_object_add(ecuInfoObject, ADM_COM_AIR_PROTOCOL_FIELD_PARTSUPPLIER, stringObject);
    /*set ecuSwDid*/

    stringObject = json_object_new_string(partHardwareNum);
    (void)json_object_object_add(ecuInfoObject, ADM_COM_AIR_PROTOCOL_FIELD_PARTHARDWARENUM, stringObject);

    if (NULL != softwarePartNumber)
    {
        stringObject = json_object_new_string(softwarePartNumber);
        (void)json_object_object_add(ecuInfoObject, ADM_COM_AIR_PROTOCOL_FIELD_SOFTWAREPARTNUMBER, stringObject);
    }
    return TRUE;
}

DCHAR* adm_eim_ecuInfoJsonStr(const ADM_EIM_GETECUINFO_T* getEcuInfo)
{
    if (NULL == getEcuInfo || 0 == getEcuInfo->num) {
        log_e("getEcuInfo is NULL");
        return NULL;
    }
    json_object* ecuInfoArrayObject = NULL;
    json_object* ecuInfoObject = NULL;
    DCHAR* ecuInfoArrayObjectStr = NULL;

    ecuInfoArrayObject = json_object_new_array();
    if (NULL == ecuInfoArrayObject) {
        log_e("json_object_new_object.failed");
        return NULL;
    }

    for (D32S i = 0; i < getEcuInfo->num; ++i) {
        if (NULL != getEcuInfo->ecuInfo[i] && NULL != getEcuInfo->ecuInfo[i]->ecuReqId
            && NULL != getEcuInfo->ecuInfo[i]->ecuSwDid && NULL != getEcuInfo->ecuInfo[i]->vdiDidListResponse
            && 0U != getEcuInfo->ecuInfo[i]->vdiDidListResponse->didArrayLength) {
            ecuInfoObject = json_object_new_object();
            /*add ecuReqId ecuSwDid*/
            if (FALSE
                == adm_eim_addReqIdSwDidToEcuObject(
                    ecuInfoObject, getEcuInfo->ecuInfo[i]->ecuReqId, getEcuInfo->ecuInfo[i]->ecuSwDid)) {
                log_e("adm_eim_addReqIdSwDidToEcuObject.failed");
                (void)json_object_put(ecuInfoObject);
                continue;
            }
            /*add ecu did*/
            if (FALSE == adm_eim_addDidToEcuObject(ecuInfoObject, getEcuInfo->ecuInfo[i]->vdiDidListResponse)) {
                log_e("adm_eim_addDidToEcuObject.failed");
                (void)json_object_put(ecuInfoObject);
                continue;
            }
            /*add part info*/
            if (FALSE
                == adm_eim_addPartInfoToEcuObject(
                    ecuInfoObject, getEcuInfo->ecuInfo[i]->partNumber, getEcuInfo->ecuInfo[i]->partSupplier, 
                    getEcuInfo->ecuInfo[i]->softwarePartNumber,getEcuInfo->ecuInfo[i]->partHardwareNum)) 
            {
                log_e("adm_eim_addPartInfoToEcuObject.failed");
                (void)json_object_put(ecuInfoObject);
                continue;
            }
            (void)json_object_array_add(ecuInfoArrayObject, ecuInfoObject);
        } else {
            log_w("getEcuInfo->ecuInfo[%d] is invalid", i);
        }
    }
    if (0U != json_object_array_length(ecuInfoArrayObject)){
        ecuInfoArrayObjectStr = adm_strdup(json_object_to_json_string(ecuInfoArrayObject));
    }
    (void)json_object_put(ecuInfoArrayObject);

    return ecuInfoArrayObjectStr;
}

DCHAR *adm_eim_singleEcuInfoJsonStr(const ECU_INFO_T *singleEcuInfo)
{
    if (NULL == singleEcuInfo || NULL == singleEcuInfo->ecuReqId || NULL == singleEcuInfo->ecuSwDid || NULL == singleEcuInfo->vdiDidListResponse || 0U == singleEcuInfo->vdiDidListResponse->didArrayLength)
    {
        log_e("singleEcuInfo is NULL");
        return NULL;
    }
    json_object *ecuInfoObject = NULL;
    DCHAR *singleEcuInfotStr = NULL;

    ecuInfoObject = json_object_new_object();
    /*add ecuReqId ecuSwDid*/
    if (TRUE == adm_eim_addReqIdSwDidToEcuObject(ecuInfoObject, singleEcuInfo->ecuReqId, singleEcuInfo->ecuSwDid))
    {
        /*add ecu did*/
        if (TRUE == adm_eim_addDidToEcuObject(ecuInfoObject, singleEcuInfo->vdiDidListResponse))
        {
            /*add part info*/
            if (TRUE == adm_eim_addPartInfoToEcuObject(ecuInfoObject, 
                                                        singleEcuInfo->partNumber, 
                                                        singleEcuInfo->partSupplier,
                                                        singleEcuInfo->softwarePartNumber, 
                                                        singleEcuInfo->partHardwareNum))
            {
                singleEcuInfotStr = adm_strdup(json_object_to_json_string(ecuInfoObject));
            }
            else
            {
                log_e("adm_eim_addPartInfoToEcuObject.failed");
            }
        }
        else
        {
            log_e("adm_eim_addDidToEcuObject.failed");
        }
    }
    else
    {
        log_e("adm_eim_addReqIdSwDidToEcuObject.failed");
    }
    (void)json_object_put(ecuInfoObject);
    return singleEcuInfotStr;
}
/***************************************ecuInfo to str**********************************************/

/***************************************str to ecuInfo**********************************************/
static DBOOL adm_eim_getJsonStrVal(const json_object* obj, const DCHAR* key, DCHAR** str)
{
    if ((NULL == obj) || (NULL == key)) {
        return FALSE;
    }

    json_object* jsonStr = NULL;

    if (!json_object_object_get_ex(obj, key, &jsonStr)) {
        return FALSE;
    }
    *str = adm_strdup(json_object_get_string(jsonStr));
    if (NULL != *str) {
        return TRUE;
    }
    return FALSE;
}

static DBOOL adm_eim_getJsonIntVal(const json_object* obj, const DCHAR* key, D32S* val)
{
    if ((NULL == obj) || (NULL == key)) {
        return FALSE;
    }

    json_object* jsonStr = NULL;
    if (!json_object_object_get_ex(obj, key, &jsonStr)) {
        return FALSE;
    }
    *val = json_object_get_int(jsonStr);
    return TRUE;
}

static DBOOL adm_eim_getJsonInt64Val(const json_object* obj, const DCHAR* key, D64S* val)
{
    if ((NULL == obj) || (NULL == key)) {
        return FALSE;
    }

    json_object* jsonStr = NULL;
    if (!json_object_object_get_ex(obj, key, &jsonStr)) {
        return FALSE;
    }
    *val = json_object_get_int64(jsonStr);
    return TRUE;
}

static D32S eim_parseEcuInfoObject(json_object* singleEcuInfoObject, ECU_INFO_T* singleEcuInfo)
{
    if (NULL == singleEcuInfoObject && NULL == singleEcuInfo) {
        log_e("input param is NULL");
        return ADM_ECODE_INVALID_PARAM;
    }

    D32S retVal = ADM_ECODE_FAILED;
    D32S getEcuDidRetVal = ADM_ECODE_FAILED;
    json_object* ecuDidObject = NULL;
    json_object* didObject = NULL;
    D32S ecuInfoLen = 0;

    DCHAR* didStr = NULL; /**!< Data ID */
    D32S did = 0;
    D32S startByte = 0;  ///< Starting byte position
    D32S endByte = 0;  ///< Ending byte positoin
    DCHAR* didValueStr = NULL;
    D32S didValue = 0;

    do {
        if (FALSE
            == adm_eim_getJsonStrVal(
                singleEcuInfoObject, ADM_COM_AIR_PROTOCOL_FIELD_ECUREQID, &(singleEcuInfo->ecuReqId))) {
            log_e("get ecuReqId failed");
            break;
        }

        if (FALSE
            == adm_eim_getJsonStrVal(
                singleEcuInfoObject, ADM_COM_AIR_PROTOCOL_FIELD_ECUSWDID, &(singleEcuInfo->ecuSwDid))) {
            log_e("get ecuSwId failed");
            break;
        }

        if (FALSE
            == adm_eim_getJsonStrVal(
                singleEcuInfoObject, ADM_COM_AIR_PROTOCOL_FIELD_PARTNUM, &(singleEcuInfo->partNumber))) {
            log_e("get partNumber failed");
            break;
        }

        if (FALSE
            == adm_eim_getJsonStrVal(
                singleEcuInfoObject, ADM_COM_AIR_PROTOCOL_FIELD_PARTSUPPLIER, &(singleEcuInfo->partSupplier))) {
            log_e("get partSupplier failed");
            break;
        }

        if (FALSE
            == adm_eim_getJsonStrVal(
                singleEcuInfoObject, ADM_COM_AIR_PROTOCOL_FIELD_SOFTWAREPARTNUMBER, &(singleEcuInfo->softwarePartNumber))) {
            log_d("get softwarePartNumber failed");
        }

        if (FALSE
            == adm_eim_getJsonStrVal(
                singleEcuInfoObject, ADM_COM_AIR_PROTOCOL_FIELD_PARTHARDWARENUM, &(singleEcuInfo->partHardwareNum))) {
            log_e("get partHardwareNum failed");
            break;
        }

        if (FALSE == json_object_object_get_ex(singleEcuInfoObject, ADM_COM_AIR_PROTOCOL_FIELD_ECUDIDS, &ecuDidObject)) {
            retVal = ADM_ECODE_JSON_GET_OBJECT_FAILED;
            log_e("get ecuDid from json_object. failed");
            break;
        }
        ecuInfoLen = (D32S)json_object_array_length(ecuDidObject);
        if (0 == ecuInfoLen) {
            log_e("json array is empty");
            break;
        }

        singleEcuInfo->vdiDidListResponse
            = (ADM_DIM_SOA_DID_DATA_ARRAY_T*)adm_malloc(sizeof(ADM_DIM_SOA_DID_DATA_ARRAY_T));
        singleEcuInfo->vdiDidListResponse->didArray
            = (ADM_DIM_SOA_DID_DATA_T**)adm_malloc(sizeof(ADM_DIM_SOA_DID_DATA_T*) * (D64U)ecuInfoLen);

        singleEcuInfo->vdiDidListResponse->didArrayLength = (D16U)ecuInfoLen;

        for (D32S i = 0; i < ecuInfoLen; ++i) {
            didObject = json_object_array_get_idx(ecuDidObject, (size_t)i);

            if (NULL == didObject) {
                log_e("didObject is NULL");
                break;
            }
            if (FALSE == adm_eim_getJsonStrVal(didObject, ADM_COM_AIR_PROTOCOL_FIELD_DID, &didStr)) {
                log_e("get did failed");
                break;
            }
            if (FALSE == adm_eim_getJsonStrVal(didObject, ADM_COM_AIR_PROTOCOL_FIELD_DIDVALUE, &didValueStr)) {
                log_e("get didVale failed");
                adm_free(didStr);
                break;
            }
            if (FALSE == adm_eim_getJsonIntVal(didObject, ADM_COM_AIR_PROTOCOL_FIELD_START, &startByte)) {
                log_e("get did failed");
                adm_free(didStr);
                adm_free(didValueStr);
                break;
            }
            if (FALSE == adm_eim_getJsonIntVal(didObject, ADM_COM_AIR_PROTOCOL_FIELD_END, &endByte)) {
                log_e("get didVale failed");
                adm_free(didStr);
                adm_free(didValueStr);
                break;
            }
            log_i("didStr = %s", didStr);
            (void)adm_hexStringToInt32(didStr, &did);
            
            singleEcuInfo->vdiDidListResponse->didArray[i]
                = (ADM_DIM_SOA_DID_DATA_T*)adm_malloc(sizeof(ADM_DIM_SOA_DID_DATA_T));

            singleEcuInfo->vdiDidListResponse->didArray[i]->did = did;
            singleEcuInfo->vdiDidListResponse->didArray[i]->startByte = (D16U)startByte;
            singleEcuInfo->vdiDidListResponse->didArray[i]->endByte = (D16U)endByte;
            singleEcuInfo->vdiDidListResponse->didArray[i]->didData
                = (ADM_DIM_SOA_DATA_T*)adm_malloc(sizeof(ADM_DIM_SOA_DATA_T));
            singleEcuInfo->vdiDidListResponse->didArray[i]->didData->dataLength = (D16U)adm_strlen(didValueStr);
            singleEcuInfo->vdiDidListResponse->didArray[i]->didData->dataType = (D16U)VAR_STRING;
            singleEcuInfo->vdiDidListResponse->didArray[i]->didData->v.valueS = (D8U*)adm_strdup(didValueStr);

            adm_free(didStr);
            adm_free(didValueStr);

            getEcuDidRetVal = ADM_ECODE_SUCCESSED;
        }
        if (ADM_ECODE_FAILED == getEcuDidRetVal) {
            log_e("get ecuDid failed");
            break;
        }
        log_i("get single ecuInfo success");
        retVal = ADM_ECODE_SUCCESSED;
    } while (FALSE);
    return retVal;
}

ADM_EIM_GETECUINFO_T* adm_eim_ecuInfoJsonStrParse(const DCHAR* ecuInfoStr)
{
    if (NULL == ecuInfoStr) {
        log_e("ecuInfoStr is NULL");
        return NULL;
    }

    json_object* ecuInfoObject = NULL;
    json_object* singleEcuInfoObject = NULL;
    D32S ecuInfoLen;
    ECU_INFO_T* singleEcuInfo = NULL;
    ADM_EIM_GETECUINFO_T* ecuInfo = NULL;
    json_type jsonType;
    D32S retVal = ADM_ECODE_FAILED;

    log_i("ecuInfoStr = %s", ecuInfoStr);
    ecuInfoObject = json_tokener_parse(ecuInfoStr);
    if (NULL == ecuInfoObject) {
        log_e("ecuInfoStr to json failed");
        return NULL;
    }

    do {
        jsonType = json_object_get_type(ecuInfoObject);
        if (json_type_array != json_object_get_type(ecuInfoObject)) {
            log_e("json object is not array");
            break;
        }

        ecuInfoLen = (D32S)json_object_array_length(ecuInfoObject);
        if (0 == ecuInfoLen) {
            log_e("json object len is 0");
            break;
        }
        log_i("ecuInfo array len = %d", ecuInfoLen);

        ecuInfo = adm_eim_newGetEcuInfoBody();
        ecuInfo->ecuInfo = (ECU_INFO_T**)adm_malloc((size_t)(sizeof(ECU_INFO_T*) * (size_t)ecuInfoLen));
        ecuInfo->num = 0;

        for (D32S i = 0; i < ecuInfoLen; ++i) {
            singleEcuInfoObject = json_object_array_get_idx(ecuInfoObject, (size_t)i);
            if (NULL == singleEcuInfoObject) {
                log_e("ecuInfoObject[%d] is NULL", i);
                continue;
            }

            singleEcuInfo = adm_eim_newEcuInfoBody();
            if (ADM_ECODE_SUCCESSED != eim_parseEcuInfoObject(singleEcuInfoObject, singleEcuInfo)) {
                log_e("eim_parseEcuInfoObject return failed,ecuInfoObject[%d]", i);
                adm_eim_deleteEcuInfoBody(singleEcuInfo);
                singleEcuInfo = NULL;
                continue;
            }
            ecuInfo->ecuInfo[ecuInfo->num] = singleEcuInfo;
            ecuInfo->num++;
        }

        if (0 == ecuInfo->num) {
            log_e("ecuInfo->num = 0");
            break;
        }
        retVal = ADM_ECODE_SUCCESSED;
        log_i("get ecu info success from KV ecuInfoStr num = %d", ecuInfo->num);
    } while (FALSE);

    if (retVal != ADM_ECODE_SUCCESSED) {
        adm_eim_deleteGetEcuInfoBody(ecuInfo);
        ecuInfo = NULL;
    }

    (void)json_object_put(ecuInfoObject);
    return ecuInfo;
}
/***************************************str to ecuInfo**********************************************/