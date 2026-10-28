#include "adm_ocs_log.h"
#include "adm_ocs_airProtocolSecurity.h"

/****************************字典排序数据定义****************************/
typedef struct DataNode
{
    DCHAR *key;
    DCHAR *value;
    struct DataNode *next;
} DataNode;

typedef struct ExtraNode
{
    DCHAR *key;
    DCHAR *value;
} ExtraNode;
ExtraNode *extraNodeNew(void);
void extraNodeDel(ExtraNode *extraNode);

typedef struct ExtraNodes
{
    ExtraNode **extraNodes;
    int len;
} ExtraNodes;
ExtraNodes *ExtraNodesNew(void);
void extraNodesDel(ExtraNodes *extraNodes);
/****************************字典排序数据定义****************************/


ExtraNode *extraNodeNew(void)
{
    ExtraNode *node = NULL;

    node = adm_malloc(sizeof(ExtraNode));
    if(NULL == node)
    {
        log_e( "malloc size %ld error", sizeof(ExtraNode));
        return NULL;
    }
    (void)adm_memset(node, 0x00, sizeof(ExtraNode), sizeof(ExtraNode) * sizeof(DCHAR));

    return node;
}

void extraNodeDel(ExtraNode *extraNode)
{
    if(NULL != extraNode)
    {
        if(NULL != extraNode->key)
        {
            adm_free(extraNode->key);
        }
        if(NULL != extraNode->value)
        {
            adm_free(extraNode->value);
        }
        adm_free(extraNode);
    }
}

ExtraNodes *ExtraNodesNew(void)
{
    ExtraNodes *nodes = NULL;

    nodes = adm_malloc(sizeof(ExtraNodes));
    if(NULL == nodes)
    {
        log_e( "malloc size %ld error", sizeof(ExtraNodes));
        return NULL;
    }
    (void)adm_memset(nodes, 0x00, sizeof(ExtraNodes), sizeof(ExtraNodes) * sizeof(DCHAR));

    return nodes;
}
void extraNodesDel(ExtraNodes *extraNodes)
{
    D32S i = 0;

    if(NULL == extraNodes)
    {
        return;
    }

    if(NULL != extraNodes->extraNodes)
    {
        for(i = 0; i < extraNodes->len; i++)
        {
            extraNodeDel(extraNodes->extraNodes[i]);
        }

        adm_free(extraNodes->extraNodes);
    }
    adm_free(extraNodes);
}

static DataNode *dataNodeNew(DCHAR key[], DCHAR value[])
{
    DataNode *node = NULL;
    D32U keyLen = 0;
    D32U keyLenCpy = 0;
    D32U valueLen = 0;
    D32U valueLenCpy = 0;

    node = adm_malloc(sizeof(DataNode));
    if(NULL == node)
    {
        log_e( "adm_malloc %ld size error", sizeof(DataNode));
        return NULL;
    }

    (void)adm_memset(node, 0x00, sizeof(DataNode), sizeof(DataNode) * sizeof(DCHAR));
    keyLen = adm_strlen(key) + 1u;
    keyLenCpy = adm_strlen(key);
    valueLen = adm_strlen(value) + 1u;
    valueLenCpy = adm_strlen(value);

    if (NULL != key)
    {
        node->key = adm_malloc((DSIZE)keyLen);
        (void)adm_memset(node->key, 0x00, (DSIZE)keyLen, (DSIZE)keyLen);
        (void)adm_memcpy(node->key, key, (DSIZE)keyLenCpy, (DSIZE)keyLen);
        node->key[adm_strlen(key)] = '\0';
    }
    else 
    {
        node->key = NULL;
    }

    if (NULL != value)
    {
        node->value = adm_malloc((DSIZE)valueLen);
        (void)adm_memset(node->value, 0x00, (DSIZE)valueLen, (DSIZE)valueLen);
        (void)adm_memcpy(node->value, value, (DSIZE)valueLenCpy, (DSIZE)valueLen);
        node->value[adm_strlen(value)] = '\0';
    }
    else 
    {
        node->value = NULL;
    }

    node->next = NULL;

    return node;
}

static void dataNodeFree(DataNode *head)
{
    DataNode *tmpNode = NULL;
    DataNode *tmpNode2 = NULL;	

    tmpNode = head;
    while(NULL != tmpNode)
    {
		tmpNode2 = tmpNode;
		
        if(NULL != tmpNode->key)
        {
            adm_free(tmpNode->key);
        }
        if(NULL != tmpNode->value)
        {
            adm_free(tmpNode->value);
        }
        tmpNode = tmpNode->next;

        adm_free(tmpNode2);
    }
}

static D32S pushBack(DataNode **head, DCHAR *key, DCHAR *value)
{
    DataNode *tail = NULL;
    D32S ret = ADM_ECODE_SUCCESSED;

    if(NULL == key)
    {
        log_e( "param key is nil");
        return ADM_ECODE_FAILED;
    }

    if(NULL == value)
    {
        log_e( "param value is nil. key:%s", key);
        return ADM_ECODE_SUCCESSED;
    }

    if(value[0] == '\0')
    {
        log_e( "param value is inval acsii 0. key:%s", key);
        return ADM_ECODE_SUCCESSED;
    }


    if(NULL != *head)
    {
        tail = *head;
        while(NULL != tail->next)
        {
            tail = tail->next;
        }
        // log_i( "real pushBack. key:%s,value:%s", key, value);
        tail->next = dataNodeNew(key, value);
        if(NULL == tail->next)
        {
            ret = ADM_ECODE_FAILED;
        }
    }
    else
    {
        log_i( "create head");
        *head = dataNodeNew(key, value);
        if(NULL == head)
        {
            ret = ADM_ECODE_FAILED;
        }
    }

    return ret;
}

#if 1
static void printList(DataNode *list)
{
    DataNode *tmp = NULL;
    D32S index = 0;

    tmp = list;
    while(NULL != tmp)
    {
        log_i( "index:%d,%s,%s", index++, tmp->key, tmp->value);
        tmp = tmp->next;
    }
}
#endif

#if 0
static D32S asciiCompare(DCHAR *str1, DCHAR *str2)
{
    D32S i;
    D32S len;

    len = adm_strlen(str1) > adm_strlen(str2) ? adm_strlen(str2) : adm_strlen(str1);
    for(i = 0; i < len; i++)
    {
        if((char)str1[i] == (char)str2[i])
        {
            continue;
        }

        return (char)str1[i] - (char)str2[i];
    }

    return 0;
}
#endif
static D32S asciiCompare(DCHAR *str1, DCHAR *str2)
{
    D32S i;
    D32S len;

    D32S len1 = 0;
    D32S len2 = 0;

    len1 = (D32S)adm_strlen(str1);
    len2 = (D32S)adm_strlen(str2);
    len = len1 > len2 ? len2 : len1;
    for(i = 0; i < len; i++)
    {
        if((char)str1[i] == (char)str2[i])
        {
            if (len2-1 == i)
            {
                return 1;
            }
            else
            {
                continue;
            }
        }

        return (char)str1[i] - (char)str2[i];
    }

    return 0;
}


static D32S bubbleSort(DataNode *head)
{
    DataNode *tmp, *tmp2;
    DCHAR *tmpKey, *tmpValue;

    for (tmp = head->next; tmp->next != NULL; tmp = tmp->next)
    {
        for (tmp2 = head->next; tmp2->next != NULL; tmp2 = tmp2->next)
        {
            if(asciiCompare(tmp2->key, tmp2->next->key) > 0)
            {
                tmpKey = tmp2->key;
                tmpValue = tmp2->value;

                tmp2->key = tmp2->next->key;
                tmp2->value = tmp2->next->value;

                tmp2->next->key = tmpKey;
                tmp2->next->value = tmpValue;
            }
        }
    }

    return 0;
}

static D64S sizeOfSignData(DataNode *head)
{
    DataNode *tmp;
    D64S size = 0;

    tmp = head;
    while(NULL != tmp)
    {
        if(NULL == tmp->key)
        {
            tmp = tmp->next;
            continue;
        }

        size += (D64S)adm_strlen(tmp->key);
        size += (D64S)adm_strlen(tmp->value);
        size += 2;

        tmp = tmp->next;
    }
    return size;
}

static D32S createSignDataList(const json_object *jsonVal, DCHAR* parentKey, DataNode** linkedList)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    DCHAR keyName[512] = {0};
    DCHAR* keyValue = NULL;
    DCHAR key[512] = {0};
    DCHAR* newParentKey = NULL;
    struct lh_entry *entry = NULL;
    const json_object* jsonObj = NULL;
    json_object* subJsonObj = NULL;
    D64S intVal = 0;
    double dbVal = 0.0;
    DCHAR valStr[64] = {0};
    D32S i = 0;
    D32S arrLen = 0;
    
    if (NULL == jsonVal) {
        log_e( "Parameter (jsonVal) verification failed");
        return ADM_ECODE_INVALID_PARAM;
    }

    entry = json_object_get_object(jsonVal)->head;
    // jsonObj = json_object_get_object(jsonVal);
    while(NULL != entry) {
        (void)adm_strcpy(keyName,entry->k,sizeof(keyName));
        jsonObj = entry->v;

        // log_i( "json_key name is %s", keyName);
        // Ignore keyName == sign
        if (0 == adm_strcmp(keyName, "sign")) {
            log_i( "Ignore keyName==sign value");
            entry = entry->next;
            continue;
        }

        switch (json_object_get_type(jsonObj)) {
            case json_type_double:
                dbVal = json_object_get_double(jsonObj);
                (void)adm_memset(valStr, 0x00, sizeof(valStr), sizeof(valStr) * sizeof(DCHAR));
                (void)adm_snprintf(valStr, sizeof(valStr), "%.1f", dbVal);
                (void)adm_memset(key, 0x00, sizeof(key), sizeof(key) * sizeof(DCHAR));
                if (NULL != parentKey) {
                    (void)adm_snprintf(key, sizeof(key), "%s%s", parentKey, keyName);
                } else {
                    (void)adm_snprintf(key, sizeof(key), "%s", keyName);
                }
                iRet = pushBack(linkedList, key, valStr);
                if (ADM_ECODE_SUCCESSED != iRet) {
                    log_e( "puchBack failed");
                }
                break;

            case json_type_int:
                intVal = json_object_get_int64(jsonObj);
                (void)adm_memset(valStr, 0x00, sizeof(valStr), sizeof(valStr) * sizeof(DCHAR));
                (void)adm_snprintf(valStr, sizeof(valStr), "%lld", intVal);
                (void)adm_memset(key, 0x00, sizeof(key), sizeof(key) * sizeof(DCHAR));
                if (NULL != parentKey) {
                    (void)adm_snprintf(key, sizeof(key), "%s%s", parentKey, keyName);
                } else {
                    (void)adm_snprintf(key, sizeof(key), "%s", keyName);
                }
                iRet = pushBack(linkedList, key, valStr);
                if (ADM_ECODE_SUCCESSED != iRet) {
                    log_e( "puchBack failed");
                }
                break;

            case json_type_object:
                (void)adm_memset(key, 0x00, sizeof(key), sizeof(key) * sizeof(DCHAR));
                if (NULL != parentKey) {
                    (void)adm_snprintf(key, sizeof(key), "%s%s.", parentKey, keyName);
                } else {
                    (void)adm_snprintf(key, sizeof(key), "%s.", keyName);
                }
                iRet = createSignDataList(jsonObj, key, linkedList);
                if (ADM_ECODE_SUCCESSED != iRet) {
                    log_e( "createSignDataList failed, reason of error code is %d", iRet);
                }
                break;

            case json_type_array:
                arrLen = (D32S)json_object_array_length(jsonObj);
                for (i = 0; i < arrLen; i++) {
                    // json_object_get_array();
                    subJsonObj = json_object_array_get_idx(jsonObj, (size_t)i);
                    (void)adm_memset(key, 0x00, sizeof(key), sizeof(key) * sizeof(DCHAR));
                    if (NULL != parentKey) {
                        (void)adm_snprintf(key, sizeof(key), "%s%s[%d].", parentKey, keyName, i);
                    } else {
                        (void)adm_snprintf(key, sizeof(key), "%s[%d].", keyName, i);
                    }
                    iRet = createSignDataList(subJsonObj, key, linkedList);
                    if (ADM_ECODE_SUCCESSED != iRet) {
                        log_e( "createSignDataList failed, reason of error code is %d", iRet);
                    }
                }
                break;

            case json_type_string:
                // log_i( "keyName:%s", keyName);
                keyValue = adm_strdup(json_object_get_string((struct json_object *)jsonObj));
                if (NULL != keyValue) {
                    (void)adm_memset(key, 0x00, sizeof(key), sizeof(key) * sizeof(DCHAR));
                    if (NULL != parentKey) {
                        (void)adm_snprintf(key, sizeof(key), "%s%s", parentKey, keyName);
                    } else {
                        (void)adm_snprintf(key, sizeof(key), "%s", keyName);
                    }
                    // log_i( "*************** key:%s, keyName:%s, keyValue:%s", key, keyName, keyValue);
                    iRet = pushBack(linkedList, key, keyValue);
                    if (ADM_ECODE_SUCCESSED != iRet) {
                        log_e( "puchBack failed");
                    }
                } else {
                    log_e( "json_object_get_string %s failed", keyName);
                }
                if (NULL != keyValue) {
                     adm_free(keyValue);
                }
                break;

            case json_type_null:
                break;
            case json_type_boolean:
                break;
            default:
                log_w( "unsupport json type");
                break;
        }
        if (ADM_ECODE_SUCCESSED != iRet) {
            log_e( "createSignDataList failed, reason of error code is %d", iRet);
            break;
        }
        entry = entry->next;
        if (NULL == entry) {
            // log_i( "Parse json end");
        }
    }
    
    return iRet;
}

static D32S signDataListCat(DataNode *head, DCHAR **signData)
{
    DataNode *tmpNode;
    DCHAR *signDataStr;
    DCHAR tmpStr[1024*10];
    D64S signDataLen;

    signDataLen = sizeOfSignData(head) + (D64S)1;
    // log_d( "signDataLen = %lld", signDataLen);

    signDataStr = adm_malloc((DSIZE)signDataLen);
    if(NULL == signDataStr)
    {
        log_e( "adm_malloc %lld size failed", signDataLen);
        return ADM_ECODE_FAILED;
    }
    (void)adm_memset(signDataStr, 0x00, (DSIZE)signDataLen, (DSIZE)signDataLen);
    tmpNode = head;
    while(NULL != tmpNode)
    {
        if((NULL == tmpNode->key) || (NULL == tmpNode->value))
        {
            tmpNode = tmpNode->next;
            continue;
        }

        (void)adm_snprintf(tmpStr, sizeof(tmpStr), "%s=%s", tmpNode->key, tmpNode->value);
        (void)adm_strcat(signDataStr, tmpStr, (DSIZE)signDataLen);
        (void)adm_strcat(signDataStr, "&", (DSIZE)signDataLen);

        tmpNode = tmpNode->next;
    }
    signDataStr[adm_strlen(signDataStr) - 1u] = '\0';
    *signData = signDataStr;
    // log_d( "signData:%s\n", *signData);

    return ADM_ECODE_SUCCESSED;
}

static D32S adm_rs_createSignDataList_Extra(json_object *jsonVal, ExtraNodes *extraNodes, DCHAR **dataList)
{
    DataNode *node;
    ExtraNode *extraNode;
    D32S extraNodeLen;
    D32S i;
    D32S retVal = 0;

    node = dataNodeNew(NULL, NULL);
    if(0 != createSignDataList(jsonVal, NULL, &node))
    {
        log_e( "createSignDataList error");
        retVal = ADM_ECODE_FAILED;
        goto out;
    }

    if(NULL != extraNodes && extraNodes->len > 0)
    {
        extraNodeLen = extraNodes->len;
        log_d( "extraNode size is %d", extraNodeLen);
        for(i = 0; i < extraNodeLen; i++)
        {
            extraNode = extraNodes->extraNodes[i];
            if(0 != pushBack(&node, extraNode->key, extraNode->value))
            {
                log_e( "pushBack extraNode %s,%s error", extraNode->key, extraNode->value);
                retVal = ADM_ECODE_FAILED;
                goto out;
            }
        }
    }

    (void)bubbleSort(node);
    // log_d( "the dataNode after sort:");
    //printList(node);

    if(0 != signDataListCat(node, dataList))
    {
        log_e( "signDataListCat error");
        retVal = ADM_ECODE_FAILED;
        goto out;
    }

out:
    if(NULL != node)
    {
        dataNodeFree(node);
    }
    return retVal;
}


/**
 * @brief 按照平台签名生成规则，生成签名数据
 * 
 * @param protocolStr   const DCHAR*        需要被签名的数据，字符串格式
 * @param secret        const DCHAR*        签名密钥字符串，
 * @note secret 1. 请求平台的数据，
 *                  1). 除注册接口使用vin，其余使用注册响应信息中的secret；
 *              2. 平台响应的数据，
 *                  1). 注册接口使用vin验签；
 *                  2). 其余接口使用secret验签； 如果平台响应500错误码，使用约定的公钥验签；
 * @param deviceId      const DCHAR*         注册响应信息中，平台返回的车辆身份ID（vin的hexMD5值）
 * @note deviceId   除了注册接口，其他所有接口请求报文签名时都要增加deviceId字段
 * @return DCHAR*       NULL:failed, Non-NULL:生成的签名信息
 */
DCHAR* adm_ocs_ariProtocolSign(const DCHAR* protocolStr, const DCHAR* secret, const DCHAR* deviceId)
{
    D32S iRet = ADM_ECODE_FAILED;
    ExtraNode *extraNode = NULL;
    ExtraNodes *extraNodes = NULL;
    json_object* jsonVal = NULL;
    DCHAR* stitchingData = NULL;
    DCHAR* hmacSignInfo = NULL;

    if (NULL == protocolStr || NULL == secret) {
        log_e( "Parameter (protocolStr || secret) verification failed");
        return NULL;
    }

    // json_object_to_json_string(protocolStr);
    jsonVal = json_tokener_parse(protocolStr);
    if (NULL == jsonVal) {
        log_e( "json_tokener_parse protocolStr:%s failed", protocolStr);
        return NULL;
    }

    if (NULL != deviceId) {
        extraNodes = ExtraNodesNew();
        if(NULL == extraNodes)
        {
            extraNodesDel(extraNodes);
            return NULL;
        }
        extraNodes->extraNodes = adm_malloc(sizeof(ExtraNode *));
        if(NULL == extraNodes->extraNodes)
        {
            log_e( "malloc size %u error", sizeof(ExtraNode *));
            extraNodesDel(extraNodes);
        }

        extraNode = extraNodeNew();
        if(NULL == extraNode)
        {
            log_e( "extraNodeNew failed");
            extraNodesDel(extraNodes);
            return NULL;
        }
        extraNode->key = strdup("deviceId");
        extraNode->value = strdup(deviceId);
        extraNodes->extraNodes[0] = extraNode;
        extraNodes->len = 1;
    } else {
        log_i( "deviceId is NULL");
    }

    iRet = adm_rs_createSignDataList_Extra(jsonVal, extraNodes, &stitchingData);
    if (ADM_ECODE_SUCCESSED != iRet) {
        log_e( "adm_rs_createSignDataList_Extra failed");
        stitchingData = NULL;
    } else {
        // log_i( "adm_rs_createSignDataList_Extra successed, stitchingData:%s", stitchingData);
        
    }

    hmacSignInfo = adm_iss_hmacSha256(secret, adm_strlen(secret), stitchingData, adm_strlen(stitchingData));
    if (NULL == hmacSignInfo) {
        log_e( "adm_iss_hmacSha256 stitchingData:%s", stitchingData);
    } 
    // else {
    //     log_i( "adm_iss_hmacSha256 stitchingData:%s, hmacSignInfo:%s", stitchingData, hmacSignInfo);
    // }

    (void)json_object_put(jsonVal);

    extraNodesDel(extraNodes);
    adm_free(stitchingData);

    return hmacSignInfo;
}
