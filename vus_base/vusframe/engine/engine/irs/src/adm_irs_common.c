#include "adm_irs_common.h"
#include "adm_file.h"
#include "adm_memory.h"
#include "adm_string.h"
#include "adm_irs_log.h"
#include "adm_irs_file.h"

const D32S g_kRingMaxSize = 100;

DBOOL adm_irs_ringDatabaseIsFull(D32S head, D32S tail, D32S maxSize)
{
    if ((head + 1) % maxSize == tail) {
        return TRUE;
    } else {
        return FALSE;
    }
}

DBOOL adm_irs_ringDatabaseIsEmpty(D32S head, D32S tail)
{
    if (tail == head) {
        return TRUE;
    } else {
        return FALSE;
    }
}

void adm_irs_forwardTail(D32S* tail, D32S maxSize)
{
   *tail = (*tail + 1) % maxSize;

}

void adm_irs_forwardHead(KvStorage_t* kvs, D32S* head, D32S* tail, D32S maxSize, const DCHAR* uploadType, DBOOL bIsFile)
{
    DCHAR messageKey[MESSAGEKEYSIZE];
    (void)adm_memzero(messageKey, (DSIZE)MESSAGEKEYSIZE);
    DCHAR* pMessageVal = NULL;
    size_t iLength = 0;
    ADM_KV_STORAGE_VALUE_TYPE_E eValType = ADM_KV_STORAGE_VALUE_TYPE_INVALID;
    ADM_OCS_UPLOADFILE_T* pUploadInfoStruct = NULL;

    if (TRUE == adm_irs_ringDatabaseIsFull(*head, *tail, maxSize)) 
    {
        *tail = (*tail + 1) % maxSize;
        *head = (*head + 1) % maxSize;
        adm_irs_generateMessageKey(messageKey, MESSAGEKEYSIZE, uploadType, *head);
        if (TRUE == adm_com_kvstorage_has_key(kvs, messageKey))
        {
            
            if ( TRUE == bIsFile)
            {
                 if (TRUE == adm_com_kvstorage_get_string(g_messageDB, messageKey, &pMessageVal, &iLength, &eValType))
                 {
                    if (ADM_ECODE_SUCCESSED == adm_irs_uploadInfoJsonToStruct(pMessageVal, &pUploadInfoStruct)) 
                    {
                        log_i("remove file : %s", pUploadInfoStruct->fullFileNamePath);
                        (void)adm_fremove(pUploadInfoStruct->fullFileNamePath);
                        adm_free(pMessageVal);
                    }
                 }
            }
            adm_com_kvstorage_remove_key(kvs, messageKey);
        }
    } 
    else 
    {
        *head = (*head + 1) % maxSize;
    }
}

void adm_irs_generateMessageKey(DCHAR* messageKey, D32S messageKeySize, const DCHAR* uploadType, D32S positionHeadVal)
{
    (void)adm_snprintf(messageKey, (D32U)messageKeySize, "%s_%d", uploadType, positionHeadVal);
}

void adm_irs_getPositionIndex(KvStorage_t* kvs,
    const DCHAR* positionHeadKey,
    D32S* positionHeadVal,
    const DCHAR* positionTailKey,
    D32S* positionTailVal)
{
    size_t length = 0;
    DCHAR* headVal = NULL;
    DCHAR* tailVal = NULL;
    ADM_KV_STORAGE_VALUE_TYPE_E eValType = ADM_KV_STORAGE_VALUE_TYPE_INVALID;

    if (adm_com_kvstorage_get_string(kvs, positionHeadKey, &headVal, &length, &eValType)) {
        (void)adm_stringToInt32(headVal, positionHeadVal);
        if (*positionHeadVal >= g_kRingMaxSize) {
            /*Read out abnormal data.reset*/
            *positionHeadVal = 0;
        }
    } else {
        *positionHeadVal = 0;
        *positionTailVal = 0;
        return;
    }

    if (TRUE == adm_com_kvstorage_get_string(kvs, positionTailKey, &tailVal, &length, &eValType)) {
        (void)adm_stringToInt32(tailVal, positionTailVal);
        if (*positionTailVal >= g_kRingMaxSize) {
            /*Read out abnormal data.reset*/
            *positionTailVal = 0;
        }
    } else {
        *positionTailVal = 0;
    }
    adm_free(headVal);
    adm_free(tailVal);
    return;
}

DBOOL adm_irs_setPositionIndex(KvStorage_t* kvs,
    const DCHAR* positionHeadKey,
    D32S positionHeadVal,
    const DCHAR* positionTailKey,
    D32S positionTailVal)
{
    DCHAR headVal[40];
    DCHAR tailVal[40];
    (void)adm_memzero(headVal, 40);
    (void)adm_memzero(tailVal, 40);

    (void)adm_snprintf(headVal, 40, "%d", positionHeadVal);
    if (FALSE == adm_com_kvstorage_set_string(kvs, positionHeadKey, headVal, ADM_KV_STORAGE_VALUE_TYPE_INT32)) {
        log_e("save head.failed");
        return FALSE;
    }

    (void)adm_snprintf(tailVal, 40, "%d", positionTailVal);
    if (FALSE == adm_com_kvstorage_set_string(kvs, positionTailKey, tailVal, ADM_KV_STORAGE_VALUE_TYPE_INT32)) {
        log_e("save tail.failed");
        return FALSE;
    }
    return TRUE;
}
