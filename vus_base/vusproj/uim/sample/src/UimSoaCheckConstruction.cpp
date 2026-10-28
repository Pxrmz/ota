
#include "UimSoaCheckConstruction.h"
#include <stdlib.h>
#include <adm_memory.h>
#include "adm_string.h"

void UIMTEST_ADM_UIM_SOA_CHECK_TRIGGER_CALLBACK_F(ADM_UIM_SOA_ACTION_AUTH_TYPE_E* eActionType, void* priv)
{
    *eActionType = ADM_UIM_SOA_ACTION_AUTH_TYPE_ALLOW;
    return;
}

void UIMTEST_ADM_UIM_SOA_CHECK_QUERY_RESULT_CALLBACK_F(ADM_APPLICATION_COMMON_CMDID_TYPE_E* eCmdidType, 
                                               DCHAR** sessionId, 
                                               DCHAR** state, 
                                               ADM_UIM_SOA_CHECK_HMI_POLICY_INFO_T** ppstPolicyInfo, 
                                               D32S* errorCode, void* priv)
{
    (*eCmdidType)=ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_USER;
    *sessionId = adm_strdup("0x1236789");
    *state = adm_strdup("success");
    
	(*ppstPolicyInfo) = (ADM_UIM_SOA_CHECK_HMI_POLICY_INFO_T*)adm_malloc(sizeof(ADM_UIM_SOA_CHECK_HMI_POLICY_INFO_T));
	(void)adm_memset(*ppstPolicyInfo, 0x00, sizeof(ADM_UIM_SOA_CHECK_HMI_POLICY_INFO_T), sizeof(ADM_UIM_SOA_CHECK_HMI_POLICY_INFO_T));
	(*ppstPolicyInfo)->hmiFile = (ADM_UIM_HMI_FILE_T*)adm_malloc(sizeof(ADM_UIM_HMI_FILE_T)); 
	(*ppstPolicyInfo)->hmiFile->policyFileUrl  = adm_strdup("http://192.168.110.136");
	(*ppstPolicyInfo)->hmiFile->policyFileSize = 1234;
	(*ppstPolicyInfo)->hmiFile->policyFileHash = adm_strdup("0xffedfa10"); 
	//(void)adm_memset((*ppstPolicyInfo)->hmiInfo, 0x00, sizeof(ADM_UIM_HMI_INFO), sizeof(ADM_UIM_HMI_INFO));
	(*ppstPolicyInfo)->hmiInfo = (ADM_UIM_HMI_INFO*)adm_malloc(sizeof(ADM_UIM_HMI_INFO)); 
	(*ppstPolicyInfo)->hmiInfo->vehicleVersion = adm_strdup("version20220107");
	(*ppstPolicyInfo)->hmiInfo->totalSize = 128;
	(*ppstPolicyInfo)->hmiInfo->ecuInfoList = (ADM_UIM_ECU_INFO_ARRAY_T*)adm_malloc(sizeof(ADM_UIM_ECU_INFO_ARRAY_T));
	(*ppstPolicyInfo)->hmiInfo->ecuInfoList->hmiEcuInfoArrayLength = 2;    
	(*ppstPolicyInfo)->hmiInfo->ecuInfoList->hmiEcuInfoArray = (ADM_UIM_ECU_INFO_T**)adm_malloc(sizeof(ADM_UIM_ECU_INFO_T) * (*ppstPolicyInfo)->hmiInfo->ecuInfoList->hmiEcuInfoArrayLength);
	for(D16U i=0u; i< (*ppstPolicyInfo)->hmiInfo->ecuInfoList->hmiEcuInfoArrayLength; i++)
	{	
			(*ppstPolicyInfo)->hmiInfo->ecuInfoList->hmiEcuInfoArray[i] = (ADM_UIM_ECU_INFO_T*)adm_malloc(sizeof(ADM_UIM_ECU_INFO_T) );
			//(*ppstPolicyInfo)->hmiInfo->ecuInfoList->hmiEcuInfoArray[i]->ecuPartNum = (DCHAR*)("ecuPartNumXXX");
			(*ppstPolicyInfo)->hmiInfo->ecuInfoList->hmiEcuInfoArray[i]->requestId = adm_strdup("abcdefgAbup");
			(*ppstPolicyInfo)->hmiInfo->ecuInfoList->hmiEcuInfoArray[i]->swDid = adm_strdup("0x8888");
			(*ppstPolicyInfo)->hmiInfo->ecuInfoList->hmiEcuInfoArray[i]->ecuVersion = adm_strdup("version20220108");
			(*ppstPolicyInfo)->hmiInfo->ecuInfoList->hmiEcuInfoArray[i]->ecuSize = 64; 
	}
	(*ppstPolicyInfo)->upgradeMode = (ADM_UIM_UPGRADE_MODE_E)1;
    (*errorCode)=0;
    
    adm_uim_checkResultNotify((*eCmdidType), *sessionId, *state, (*ppstPolicyInfo),(*errorCode));

	for(D16U i=0u; i< (*ppstPolicyInfo)->hmiInfo->ecuInfoList->hmiEcuInfoArrayLength; i++)
    {
        adm_free((*ppstPolicyInfo)->hmiInfo->ecuInfoList->hmiEcuInfoArray[i]->requestId);
        adm_free((*ppstPolicyInfo)->hmiInfo->ecuInfoList->hmiEcuInfoArray[i]->swDid);
        adm_free((*ppstPolicyInfo)->hmiInfo->ecuInfoList->hmiEcuInfoArray[i]->ecuVersion);
    }
    adm_free(*sessionId);
    adm_free(*state);
    adm_free((*ppstPolicyInfo)->hmiFile->policyFileUrl);
    adm_free((*ppstPolicyInfo)->hmiFile->policyFileHash);
    adm_free((*ppstPolicyInfo)->hmiFile);
    adm_free((*ppstPolicyInfo)->hmiInfo->vehicleVersion);
    adm_free((*ppstPolicyInfo)->hmiInfo->ecuInfoList->hmiEcuInfoArray);
    adm_free((*ppstPolicyInfo)->hmiInfo->ecuInfoList);
    adm_free((*ppstPolicyInfo)->hmiInfo);
    adm_free(*ppstPolicyInfo);

    return;
}

