#ifndef ADM_IRS_MESSAGEHANDLE_H
#define ADM_IRS_MESSAGEHANDLE_H

#include "adm_irs_upload.h"

#ifdef __cplusplus
extern "C" {
#endif

extern void adm_irs_initReportMessageHandle(ADM_IRS_SESSION_T* session);
extern void adm_irs_saveReportCallBack(void* argv);

#ifdef __cplusplus
}
#endif

#endif