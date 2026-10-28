#ifndef ADM_CVM_EVENT_H
#define ADM_CVM_EVENT_H

#include "adm_cvm_checkVersion.h"

#ifdef __cplusplus
extern "C" {
#endif

extern void adm_cvm_triggerEventReport(ADM_APPLICATION_COMMON_CMDID_TYPE_E cmdId, ADM_CVS_NOTIFY_T* notifyF);
extern void adm_cvm_prepareEventReport(D32S retVal, ADM_CVS_NOTIFY_T* notifyF);
extern void adm_cvm_endEventReport(D32S retVal, ADM_CVS_NOTIFY_T* notifyF);

#ifdef __cplusplus
}
#endif

#endif  //