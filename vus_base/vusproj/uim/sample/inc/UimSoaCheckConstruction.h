
#ifndef __UIMTEST_SOACHECK_CONSTRUCTION_H_
#define __UIMTEST_SOACHECK_CONSTRUCTION_H_


#include "adm_uim_check.h"

extern void UIMTEST_ADM_UIM_SOA_CHECK_TRIGGER_CALLBACK_F(ADM_UIM_SOA_ACTION_AUTH_TYPE_E* eActionType, void* priv);


extern void UIMTEST_ADM_UIM_SOA_CHECK_QUERY_RESULT_CALLBACK_F(ADM_APPLICATION_COMMON_CMDID_TYPE_E* eCmdidType, 
                                               DCHAR** sessionId, 
                                               DCHAR** state, 
                                               ADM_UIM_SOA_CHECK_HMI_POLICY_INFO_T** ppstPolicyInfo, 
                                               D32S* errorCode, void* priv);
extern void InitCHeckSoaService(void);

#endif

