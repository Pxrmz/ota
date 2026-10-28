#ifndef _ADM_EIM_EVENT_H
#define _ADM_EIM_EVENT_H

#include "adm_typedefs.h"
#include "adm_ocs_airProtocolAdapter.h"

#ifdef __cplusplus
extern "C" {
#endif

extern ADM_OCS_EVENTS_INFO_LIST_T* adm_eim_initEventList(D32S eventNum);
extern D32S adm_eim_addEventList(ADM_OCS_EVENTS_INFO_LIST_T* eventsList,
    const DCHAR* eventId,
    D32S eventLevel,
    const DCHAR* eventContext,
    const DCHAR* eventDesc);

extern void adm_eim_clearEventList(ADM_OCS_EVENTS_INFO_LIST_T* eventsList);
extern void adm_eim_deInitEventsList(ADM_OCS_EVENTS_INFO_LIST_T* eventsList);

extern void adm_eim_getEcuInfoReport(ADM_OCS_EVENTS_INFO_LIST_T* eventsList);

extern DCHAR* adm_eim_getDateAndTimeStr(void);

#ifdef __cplusplus
}
#endif
#endif
