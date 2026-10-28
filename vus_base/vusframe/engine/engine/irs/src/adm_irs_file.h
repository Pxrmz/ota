#ifndef ADM_IRS_FILE_H
#define ADM_IRS_FILE_H

#include "adm_typedefs.h"
#include "adm_com_kvstorage.h"
#include "adm_ocs_airProtocolAdapter.h"
#include "adm_irs_upload.h"
#include "adm_irs_message.h"
#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif


extern KvStorage_t* g_fileDB;
extern ADM_FILE_UPLOAD_INTERFACE_F g_postFile;
extern ADM_OCA_FILE_UPLOAD_INFO_T* g_uploadInfo;


extern ADM_OCS_UPLOADFILE_T* adm_irs_newFileBody(const DCHAR* sessionId,
    D64S taskId,
    D64S strategyId,
    const D64S timestamp,
    D64S startTime,
    D64S endTime,
    const DCHAR* sdkVersion,
    ADM_OCS_UPLOAD_FILE_CMDID_TYPE_T cmdId,
    const ADM_OCS_UPLOAD_FILE_TYPE_T fileType,
    const DCHAR* fullFileNamePath,
    ADM_OCS_UPLOAD_LOGFILE_ENCRYPTED_T isFileEncrypted,
    const DCHAR* fileHash,
    D64S fileSize,
    const DCHAR* pushId,
    const DCHAR* extsReqJsonStr);
extern D32S adm_irs_uploadInfoJsonToStruct(const DCHAR* uploadInfoStr, ADM_OCS_UPLOADFILE_T** uploadInfoStruct);
extern void adm_irs_deleteFileBody(ADM_OCS_UPLOADFILE_T* message);
extern ADM_IRS_SAVE_UPLOADTOKV_E adm_irs_handleFile(ADM_OCS_UPLOADFILE_T* pArgvData);
extern DBOOL adm_irs_hasFile(void);
extern D32S adm_irs_reportFile(void);

#ifdef __cplusplus
}
#endif

#endif