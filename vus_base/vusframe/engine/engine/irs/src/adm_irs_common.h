#ifndef ADM_IRS_COMMON_H
#define ADM_IRS_COMMON_H

#include "adm_typedefs.h"
#include "adm_com_kvstorage.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const D32S g_kRingMaxSize;
#define MESSAGEKEYSIZE 100

extern DBOOL adm_irs_ringDatabaseIsFull(D32S head, D32S tail, D32S maxSize);
extern DBOOL adm_irs_ringDatabaseIsEmpty(D32S head, D32S tail);
extern void adm_irs_forwardHead(KvStorage_t* kvs, D32S* head, D32S* tail, D32S maxSize, const DCHAR* uploadType, DBOOL bIsFile);
extern void adm_irs_forwardTail(D32S* tail, D32S maxSize);

extern void adm_irs_generateMessageKey(DCHAR* messageKey,
    D32S messageKeySize,
    const DCHAR* uploadType,
    D32S positionHeadVal);

extern void adm_irs_getPositionIndex(KvStorage_t* kvs,
    const DCHAR* positionHeadKey,
    D32S* positionHeadVal,
    const DCHAR* positionTailKey,
    D32S* positionTailVal);

extern DBOOL adm_irs_setPositionIndex(KvStorage_t* kvs,
    const DCHAR* positionHeadKey,
    D32S positionHeadVal,
    const DCHAR* positionTailKey,
    D32S positionTailVal);

#ifdef __cplusplus
}
#endif

#endif