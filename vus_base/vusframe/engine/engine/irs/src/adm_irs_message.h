#ifndef ADM_IRS_MESSAGE_H
#define ADM_IRS_MESSAGE_H

#include "adm_irs_upload.h"
#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif


typedef enum enSaveUPLOADToKV_
{
    ADM_IRS_SAVE_SUCCESS = 1,
    ADM_IRS_SAVE_FAIL_UPFAIL = 2,
    ADM_IRS_SAVE_FAIL_UPSUCCESS = 3,
    ADM_IRS_SAVE_INVAILD = 4,
} ADM_IRS_SAVE_UPLOADTOKV_E;

extern KvStorage_t* g_messageDB;
extern ADM_OCS_BUSINESS_COMM_INTERFACE_CALLBACK_F g_post;

extern void adm_irs_deleteMessageBody(DCHAR* message);

extern ADM_IRS_SAVE_UPLOADTOKV_E adm_irs_UploadCallBack(void* pArgv, ADM_IRS_UPLOAD_TYPE_E *pUploadType);

typedef void (*ADM_IRS_GETPOSITIONINDEX_CALLBACK_F)(D32S* positionHeadVal, D32S* positionTailVal);
typedef void (*ADM_IRS_SETTAILINDEX_CALLBACK_F)(D32S iPositionTailVal);
typedef void (*ADM_IRS_SETPOSITIONINDEX_CALLBACK_F)(D32S positionHeadVal, D32S positionTailVal);

extern DBOOL adm_irs_hasVehicleStateMessage(void);
extern DBOOL adm_irs_hasEcuStateMessage(void);
extern DBOOL adm_irs_hasProgressMessage(void);
extern DBOOL adm_irs_hasEventMessage(void);

extern D32S adm_irs_reportVehicleState(void);
extern D32S adm_irs_reportEcuState(void);
extern D32S adm_irs_reportProgress(void);
extern D32S adm_irs_reportEvent(void);

#ifdef __cplusplus
}
#endif

#endif
