
#include "UimSoaInstallConstruction.h"
#include "adm_uim_log.h"
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include "adm_memory.h"
#include "adm_string.h"

static D32S m_iPolicyNotify = 0;
static D32S m_iECUInstallNotify = 0;
static D32S m_iALLECUInstallNotify = 0;
static D32S m_iPauseInstall = 0;
static D32S m_iCancelInstall=0;

static void* UIMTEST_ADM_SOA_INSTALL_POLICYPROGRESS_THREAD(void* p)
{
    D32S iprogress=0;
    ADM_APPLICATION_COMMON_CMDID_TYPE_E eCmdidType = ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_USER;
    DCHAR* sessionId;
    DCHAR* state;
    while(TRUE)
    {
        if(0 != m_iPolicyNotify)
        {
            if(0 != m_iCancelInstall)
            {
                //ȡ��
                return NULL;
            }
            if(0 != m_iPauseInstall)
            {
                //��ͣ
                (void)usleep(10);
                continue;
            }
            if(iprogress > 100)
            {
                //���Խ��֪ͨ
                ADM_UIM_SOA_INSTALL_RESULT_INFO_T* pstPolicyInstallResult;
                pstPolicyInstallResult = (ADM_UIM_SOA_INSTALL_RESULT_INFO_T*)adm_malloc(sizeof(ADM_UIM_SOA_INSTALL_RESULT_INFO_T));
                pstPolicyInstallResult->result = 1;
                pstPolicyInstallResult->expectVal = adm_strdup("0x1");
                pstPolicyInstallResult->realVal = adm_strdup("0x1");

                (void)adm_uim_policyInstallResultNotify(eCmdidType, sessionId, state, pstPolicyInstallResult);

                adm_free(pstPolicyInstallResult->expectVal);
                adm_free(pstPolicyInstallResult->realVal);
                adm_free(pstPolicyInstallResult);
                m_iPolicyNotify = 0;
                //��������ECU��װ�����ϱ�
                m_iECUInstallNotify = 1;
                return NULL;
            }
            sessionId = adm_strdup("0x9876");
            state = adm_strdup("success");
            ADM_UIM_SOA_INSTALL_PROGRESS_INFO_T* pstPolicyInstallProgress;
            pstPolicyInstallProgress = (ADM_UIM_SOA_INSTALL_PROGRESS_INFO_T*)adm_malloc(sizeof(ADM_UIM_SOA_INSTALL_PROGRESS_INFO_T));
            pstPolicyInstallProgress->progress = (D16U)iprogress++;

            (void)adm_uim_policyInstallProgressNotify(eCmdidType,sessionId,state,pstPolicyInstallProgress);

            adm_free(pstPolicyInstallProgress);
            adm_free(sessionId);
            adm_free(state);
        }
        else
        {
            iprogress = 0;
        }
        (void)usleep(10);
    }
//    return NULL;
}


static void* UIMTEST_ADM_SOA_ECUINSTALL_PROGRESS_THREAD(void* p)
{
    
    ADM_APPLICATION_COMMON_CMDID_TYPE_E eCmdidType1 = ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_USER;
    const DCHAR* sessionId1;
    const DCHAR* state1;

    D32S iprogress = 0;
    while(TRUE)
    {
        if(0 != m_iECUInstallNotify)
        {
            if(0 != m_iCancelInstall)
            {
                //ȡ��
                return NULL;
            }            
            if(0 != m_iPauseInstall)
            {
                //��ͣ
                (void)usleep(10);
                continue;
            }
            if(iprogress > 100)
            {
                sessionId1 = "0x9876";                                   
                state1 = "Success";                 
                ADM_UIM_SOA_ECU_IDEN_INFO_T* pstEcuIdenInfo1 = NULL;
                pstEcuIdenInfo1 = (ADM_UIM_SOA_ECU_IDEN_INFO_T*)adm_malloc(sizeof(ADM_UIM_SOA_ECU_IDEN_INFO_T));
                pstEcuIdenInfo1->partNum = adm_strdup("0x1234567890");
                pstEcuIdenInfo1->requestId = 0x12;
                pstEcuIdenInfo1->responseId = 0x34;
                pstEcuIdenInfo1->swDid = 0x56;
                pstEcuIdenInfo1->name = adm_strdup("install result notify");
                pstEcuIdenInfo1->softVer = adm_strdup("ver1.0");

                ADM_UIM_SOA_INSTALL_RESULT_INFO_T* pstEcuDownloadResult1 = NULL;
                pstEcuDownloadResult1 = (ADM_UIM_SOA_INSTALL_RESULT_INFO_T*)adm_malloc(sizeof(ADM_UIM_SOA_INSTALL_RESULT_INFO_T));
                pstEcuDownloadResult1->result =  0x01;
                pstEcuDownloadResult1->expectVal = adm_strdup("0x01");
                pstEcuDownloadResult1->realVal = adm_strdup("0x01");    
                (void)adm_uim_ecuInstallResultNotify(eCmdidType1,sessionId1,state1,pstEcuIdenInfo1,pstEcuDownloadResult1);
                adm_free(pstEcuDownloadResult1);
                adm_free(pstEcuIdenInfo1);

                m_iECUInstallNotify = 0;
                //����ALLECU INSTALL�����ϱ�
                m_iALLECUInstallNotify = 1;

                adm_free(pstEcuIdenInfo1->partNum);
                adm_free(pstEcuIdenInfo1->name);
                adm_free(pstEcuIdenInfo1->softVer);
                adm_free(pstEcuDownloadResult1->expectVal);
                adm_free(pstEcuDownloadResult1->realVal);
                return NULL;
            } 
            sessionId1 = "0x9876"; 
            state1 = "Success";
            ADM_UIM_SOA_ECU_IDEN_INFO_T* pstEcuIdenInfo1 = (ADM_UIM_SOA_ECU_IDEN_INFO_T*)adm_malloc(sizeof(ADM_UIM_SOA_ECU_IDEN_INFO_T));
            pstEcuIdenInfo1->partNum = adm_strdup("0x1234567890");
            pstEcuIdenInfo1->requestId = 0x12;
            pstEcuIdenInfo1->responseId = 0x34;
            pstEcuIdenInfo1->swDid = 0x56;
            pstEcuIdenInfo1->name=adm_strdup("signal ECU");
            pstEcuIdenInfo1->softVer=adm_strdup("ver1.0");
            
            ADM_UIM_SOA_INSTALL_PROGRESS_INFO_T* pstInstallProgress;
            pstInstallProgress = (ADM_UIM_SOA_INSTALL_PROGRESS_INFO_T*)adm_malloc(sizeof(ADM_UIM_SOA_INSTALL_PROGRESS_INFO_T));
            pstInstallProgress->progress = (D16U)iprogress++;

            (void)adm_uim_ecuInstallProgressNotify(eCmdidType1, sessionId1, state1, pstEcuIdenInfo1,pstInstallProgress);
            adm_free(pstEcuIdenInfo1->partNum);
            adm_free(pstEcuIdenInfo1->name);
            adm_free(pstEcuIdenInfo1->softVer);
            adm_free(pstEcuIdenInfo1);
            adm_free(pstInstallProgress);
        }
        else
        {
            iprogress = 0;
        }
        (void)usleep(10);
    }
 //   return NULL;
}

static void* UIMTEST_ADM_SOA_ALLINSTALL_PROGRESS_THREAD(void* p)
{
    ADM_APPLICATION_COMMON_CMDID_TYPE_E eCmdidType = ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_USER;
    const DCHAR* sessionId;
    const DCHAR* state;

    D32S iInstallprogress = 0;
    D32S iecuprogress = 0;
    while(TRUE)
    {
        if(0 != m_iALLECUInstallNotify)
        {
            if(0 != m_iCancelInstall)
            {
                //ȡ��
                return NULL;
            }            
            if(0 != m_iPauseInstall)
            {
                //��ͣ
                (void)usleep(10);
                continue;
            }
            if(iInstallprogress > 100)
            {
                eCmdidType = ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_USER;
                sessionId = "0x9876";
                state="success";
                ADM_UIM_SOA_ECU_INSTALL_RESULT_ARRAY_INFO_T* pstEcuInstallResultArrayInfo;
                (pstEcuInstallResultArrayInfo) = (ADM_UIM_SOA_ECU_INSTALL_RESULT_ARRAY_INFO_T*)adm_malloc(sizeof(ADM_UIM_SOA_ECU_INSTALL_RESULT_ARRAY_INFO_T));
                (pstEcuInstallResultArrayInfo)->ecuInstallResultArrayLength = 2;
                (pstEcuInstallResultArrayInfo)->ecuInstallResultArray = (ADM_UIM_SOA_ECU_INSTALL_RESULT_INFO_T**)adm_malloc(sizeof(ADM_UIM_SOA_ECU_INSTALL_RESULT_INFO_T*));
                //*((pstEcuInstallResultArrayInfo)->ecuInstallResultArray) = (ADM_UIM_SOA_ECU_INSTALL_RESULT_INFO_T*)malloc(sizeof(ADM_UIM_SOA_ECU_INSTALL_RESULT_INFO_T)*2);
                for(D32S i = 0; i < 2;i++)
                {
                    ((pstEcuInstallResultArrayInfo)->ecuInstallResultArray)[i] = (ADM_UIM_SOA_ECU_INSTALL_RESULT_INFO_T*)adm_malloc(sizeof(ADM_UIM_SOA_ECU_INSTALL_RESULT_INFO_T));
                    ((pstEcuInstallResultArrayInfo)->ecuInstallResultArray)[i]->pstEcuIdenInfo = (ADM_UIM_SOA_ECU_IDEN_INFO_T*)adm_malloc(sizeof(ADM_UIM_SOA_ECU_IDEN_INFO_T));

                    ((pstEcuInstallResultArrayInfo)->ecuInstallResultArray)[i]->pstEcuIdenInfo->partNum = adm_strdup("0x1234567890");
                    ((pstEcuInstallResultArrayInfo)->ecuInstallResultArray)[i]->pstEcuIdenInfo->requestId=0x12;
                    ((pstEcuInstallResultArrayInfo)->ecuInstallResultArray)[i]->pstEcuIdenInfo->responseId=0x34;
                    ((pstEcuInstallResultArrayInfo)->ecuInstallResultArray)[i]->pstEcuIdenInfo->swDid=0x56;
                    ((pstEcuInstallResultArrayInfo)->ecuInstallResultArray)[i]->pstEcuIdenInfo->name =adm_strdup("test");
                    ((pstEcuInstallResultArrayInfo)->ecuInstallResultArray)[i]->pstEcuIdenInfo->softVer = adm_strdup("ver1.0");

                    ((pstEcuInstallResultArrayInfo)->ecuInstallResultArray)[i]->pstEcuInstallResult = (ADM_UIM_SOA_INSTALL_RESULT_INFO_T*)adm_malloc(sizeof(ADM_UIM_SOA_INSTALL_RESULT_INFO_T));
                    ((pstEcuInstallResultArrayInfo)->ecuInstallResultArray)[i]->pstEcuInstallResult->result=1;
                    ((pstEcuInstallResultArrayInfo)->ecuInstallResultArray)[i]->pstEcuInstallResult->expectVal = adm_strdup("0x01");
                    ((pstEcuInstallResultArrayInfo)->ecuInstallResultArray)[i]->pstEcuInstallResult->realVal = adm_strdup("0x01");
                }
                ADM_UIM_SOA_INSTALL_RESULT_INFO_T* pstPolicyInstallResult;
                (pstPolicyInstallResult) = (ADM_UIM_SOA_INSTALL_RESULT_INFO_T*)adm_malloc(sizeof(ADM_UIM_SOA_INSTALL_RESULT_INFO_T));
                (pstPolicyInstallResult)->result = 1;
                (pstPolicyInstallResult)->expectVal = adm_strdup("0x01");
                (pstPolicyInstallResult)->realVal = adm_strdup("0x01");

                (void)adm_uim_allEcuInstallResultNotify(eCmdidType,sessionId,state,(pstEcuInstallResultArrayInfo),(pstPolicyInstallResult));
                for(D32S i = 0; i < 2;i++)
                {
                    adm_free(((pstEcuInstallResultArrayInfo)->ecuInstallResultArray)[i]->pstEcuIdenInfo->partNum);
                    adm_free(((pstEcuInstallResultArrayInfo)->ecuInstallResultArray)[i]->pstEcuIdenInfo->name);
                    adm_free(((pstEcuInstallResultArrayInfo)->ecuInstallResultArray)[i]->pstEcuIdenInfo->softVer);
                    adm_free(((pstEcuInstallResultArrayInfo)->ecuInstallResultArray)[i]->pstEcuInstallResult->expectVal);
                    adm_free(((pstEcuInstallResultArrayInfo)->ecuInstallResultArray)[i]->pstEcuInstallResult->realVal);
                    adm_free(((pstEcuInstallResultArrayInfo)->ecuInstallResultArray)[i]->pstEcuIdenInfo);
                    adm_free(((pstEcuInstallResultArrayInfo)->ecuInstallResultArray)[i]->pstEcuInstallResult);
                	adm_free(((pstEcuInstallResultArrayInfo)->ecuInstallResultArray)[i]);
                }
                adm_free((pstEcuInstallResultArrayInfo)->ecuInstallResultArray);
                adm_free(pstEcuInstallResultArrayInfo);
                adm_free((pstPolicyInstallResult)->expectVal);
                adm_free((pstPolicyInstallResult)->realVal);
                adm_free(pstPolicyInstallResult);

                m_iALLECUInstallNotify = 0;
                return NULL;
            }
            eCmdidType = ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_USER;
            sessionId = "0x9876";
            state = "success";
            ADM_UIM_SOA_ECU_INSTALL_PROGRESS_ARRAY_INFO_T* pstEcuInstallProgressArrayInfo;
            (pstEcuInstallProgressArrayInfo) = (ADM_UIM_SOA_ECU_INSTALL_PROGRESS_ARRAY_INFO_T*)adm_malloc(sizeof(ADM_UIM_SOA_ECU_INSTALL_PROGRESS_ARRAY_INFO_T));
            (void)memset((pstEcuInstallProgressArrayInfo),0,sizeof(ADM_UIM_SOA_ECU_INSTALL_PROGRESS_ARRAY_INFO_T));
            (pstEcuInstallProgressArrayInfo)->ecuInstallProgressArrayLength = 2;
            (pstEcuInstallProgressArrayInfo)->ecuInstallProgressArray = (ADM_UIM_SOA_ECU_INSTALL_PROGRESS_INFO_T**)adm_malloc(sizeof(ADM_UIM_SOA_ECU_INSTALL_PROGRESS_INFO_T*));
            //*((pstEcuInstallProgressArrayInfo)->ecuInstallProgressArray) = (ADM_UIM_SOA_ECU_INSTALL_PROGRESS_INFO_T*)malloc(sizeof(ADM_UIM_SOA_ECU_INSTALL_PROGRESS_INFO_T)*2);
            for(D32S i = 0; i < 2;i++)
            {
            	((pstEcuInstallProgressArrayInfo)->ecuInstallProgressArray)[i] = (ADM_UIM_SOA_ECU_INSTALL_PROGRESS_INFO_T*)adm_malloc(sizeof(ADM_UIM_SOA_ECU_INSTALL_PROGRESS_INFO_T));
                ((pstEcuInstallProgressArrayInfo)->ecuInstallProgressArray)[i]->pstEcuIdenInfo = (ADM_UIM_SOA_ECU_IDEN_INFO_T*)adm_malloc(sizeof(ADM_UIM_SOA_ECU_IDEN_INFO_T));

                ((pstEcuInstallProgressArrayInfo)->ecuInstallProgressArray)[i]->pstEcuIdenInfo->partNum = adm_strdup("0x1234");
                ((pstEcuInstallProgressArrayInfo)->ecuInstallProgressArray)[i]->pstEcuIdenInfo->requestId=0x12;
                ((pstEcuInstallProgressArrayInfo)->ecuInstallProgressArray)[i]->pstEcuIdenInfo->responseId=0x34;
                ((pstEcuInstallProgressArrayInfo)->ecuInstallProgressArray)[i]->pstEcuIdenInfo->swDid=0x56;
                ((pstEcuInstallProgressArrayInfo)->ecuInstallProgressArray)[i]->pstEcuIdenInfo->name = adm_strdup("test");
                ((pstEcuInstallProgressArrayInfo)->ecuInstallProgressArray)[i]->pstEcuIdenInfo->softVer = adm_strdup("ver1.0");

                ((pstEcuInstallProgressArrayInfo)->ecuInstallProgressArray)[i]->state = adm_strdup("success");
                ((pstEcuInstallProgressArrayInfo)->ecuInstallProgressArray)[i]->pstEcuInstallProgress = (ADM_UIM_SOA_INSTALL_PROGRESS_INFO_T*)adm_malloc(sizeof(ADM_UIM_SOA_INSTALL_PROGRESS_INFO_T));
                ((pstEcuInstallProgressArrayInfo)->ecuInstallProgressArray)[i]->pstEcuInstallProgress->progress=(D16U)iecuprogress;
            }
            iecuprogress++;
            ADM_UIM_SOA_INSTALL_PROGRESS_INFO_T* pstPolicyInstallProgress;
            (pstPolicyInstallProgress) = (ADM_UIM_SOA_INSTALL_PROGRESS_INFO_T*)adm_malloc(sizeof(ADM_UIM_SOA_INSTALL_PROGRESS_INFO_T));
            (pstPolicyInstallProgress)->progress = (D16U)iInstallprogress++;
            
            
            //log_i("install progress=%d",iInstallprogress);
            
            (void)adm_uim_allEcuInstallProgressNotify(eCmdidType,sessionId,state,(pstEcuInstallProgressArrayInfo),(pstPolicyInstallProgress));
            for(D32S i = 0;i< 2;i++)
            {
                adm_free(((pstEcuInstallProgressArrayInfo)->ecuInstallProgressArray)[i]->pstEcuIdenInfo->partNum);
                adm_free(((pstEcuInstallProgressArrayInfo)->ecuInstallProgressArray)[i]->pstEcuIdenInfo->name);
                adm_free(((pstEcuInstallProgressArrayInfo)->ecuInstallProgressArray)[i]->pstEcuIdenInfo->softVer);
                adm_free(((pstEcuInstallProgressArrayInfo)->ecuInstallProgressArray)[i]->state);
                adm_free(((pstEcuInstallProgressArrayInfo)->ecuInstallProgressArray)[i]->pstEcuIdenInfo);
                adm_free(((pstEcuInstallProgressArrayInfo)->ecuInstallProgressArray)[i]->pstEcuInstallProgress);
            	adm_free(((pstEcuInstallProgressArrayInfo)->ecuInstallProgressArray)[i]);
            }
            adm_free((pstEcuInstallProgressArrayInfo)->ecuInstallProgressArray);
            adm_free(pstEcuInstallProgressArrayInfo);
            adm_free(pstPolicyInstallProgress);
        }
        else
        {
            iInstallprogress = 0;
        }
        (void)usleep(10);
        
    }
  //  return NULL;
}

void UIMTEST_ADM_SOA_INSTALL_PROGRESS_SIMULATE(void)
{
    pthread_t thrIDPolicyProgress;
    (void)pthread_create(&thrIDPolicyProgress,NULL,&UIMTEST_ADM_SOA_INSTALL_POLICYPROGRESS_THREAD,NULL);

    pthread_t thrIDECUDownLoadP;
    (void)pthread_create(&thrIDECUDownLoadP,NULL,&UIMTEST_ADM_SOA_ECUINSTALL_PROGRESS_THREAD,NULL);

    pthread_t thrIDALLECUDownLoadP;
    (void)pthread_create(&thrIDALLECUDownLoadP,NULL,&UIMTEST_ADM_SOA_ALLINSTALL_PROGRESS_THREAD,NULL);    
    return;
}


D32S UIMTEST_ADM_UIM_SOA_INSTALL_SCHDULED_CALLBACK_F(const ADM_APPLICATION_COMMON_CMDID_TYPE_E eCmdidType, 
                                                     const ADM_UIM_SOA_INSTALL_SCHDULE_TYPE_E eSchduleType, 
                                                     const D64S schduleTime, 
                                                     ADM_UIM_SOA_ACTION_AUTH_TYPE_E* eActionType, 
                                                     void* priv)
{
    log_i("UIMTEST_ADM_UIM_SOA_INSTALL_SCHDULED_CALLBACK_F ");
    
    *eActionType = ADM_UIM_SOA_ACTION_AUTH_TYPE_ALLOW;
    adm_uim_schduleTimeArrived(DTRUE, DFALSE);

    return;
}


void UIMTEST_ADM_UIM_SOA_INSTALL_CANCELD_CALLBACK_F(const ADM_APPLICATION_COMMON_CMDID_TYPE_E eCmdidType, 
                                                    ADM_UIM_SOA_ACTION_AUTH_TYPE_E* eActionType, 
                                                    void* priv)
{
    
    log_i("canceld install");
    *eActionType = ADM_UIM_SOA_ACTION_AUTH_TYPE_ALLOW;
    m_iCancelInstall =1;
    return;
}


void UIMTEST_ADM_UIM_SOA_INSTALL_REFUSED_CALLBACK_F(const ADM_APPLICATION_COMMON_CMDID_TYPE_E eCmdidType, 
                                                    ADM_UIM_SOA_ACTION_AUTH_TYPE_E* eActionType)
{
    log_i("refused install");
    *eActionType = ADM_UIM_SOA_ACTION_AUTH_TYPE_ALLOW;
    m_iPauseInstall = 0;
    return;
}

void UIMTEST_ADM_UIM_SOA_INSTALL_TRIGGER_CALLBACK_F(ADM_UIM_SOA_ACTION_AUTH_TYPE_E* eActionType, 
                                                    void* priv)
{
    log_i("trigger install");
    *eActionType = ADM_UIM_SOA_ACTION_AUTH_TYPE_ALLOW;
    m_iPolicyNotify = 0;         
    m_iECUInstallNotify = 0;     
    m_iALLECUInstallNotify = 0;  
    m_iPauseInstall = 0;         
    m_iCancelInstall=0;

    UIMTEST_ADM_SOA_INSTALL_PROGRESS_SIMULATE();
    
    adm_uim_schduleTimeArrived(DTRUE, DFALSE);
    return;
}

void UIMTEST_ADM_UIM_SOA_INSTALL_CONDITIONS_VERIFIED_CALLBACK_F(const ADM_UIM_SOA_CONDITIONS_VERIFIED_TYPE_E eVerifiedType, 
                                                                const D16U cycleTime, 
                                                                const D16U timeout, 
                                                                D32S* errorCode, 
                                                                void* priv)
{
    *errorCode = 100;
    ADM_APPLICATION_COMMON_CMDID_TYPE_E eCmdidType = ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_USER; 
    const DCHAR* sessionId = "0x9876"; 
    const DCHAR* state = "condVerify Success"; 
    ADM_UIM_SOA_CONDITION_DATA_ARRAY_T* condsArrarList;
    condsArrarList = (ADM_UIM_SOA_CONDITION_DATA_ARRAY_T*)adm_malloc(sizeof(ADM_UIM_SOA_CONDITION_DATA_ARRAY_T));
    condsArrarList->condsArrayLength = 2;
    condsArrarList->condsArray = (ADM_UIM_SOA_CONDITION_DATA_T**)adm_malloc(sizeof(ADM_UIM_SOA_CONDITION_DATA_T*));
    //*(condsArrarList->condsArray) = (ADM_UIM_SOA_CONDITION_DATA_T*)malloc(sizeof(ADM_UIM_SOA_CONDITION_DATA_T)*2);
    for(D32S i = 0; i < 2;i++)
    {
    	(condsArrarList->condsArray)[i] = (ADM_UIM_SOA_CONDITION_DATA_T*)adm_malloc(sizeof(ADM_UIM_SOA_CONDITION_DATA_T));
        ((condsArrarList->condsArray))[i]->expectVariable = (variable_st*)adm_malloc(sizeof(variable_st));
        ((condsArrarList->condsArray))[i]->realVariable   = (variable_st*)adm_malloc(sizeof(variable_st));

        variable_st* pExpt = ((condsArrarList->condsArray))[i]->expectVariable;
        variable_st* pReal = ((condsArrarList->condsArray))[i]->realVariable;

        pExpt->data_type = 2;
        pExpt->data_length =4;
        pExpt->v.value_u32 = 12;
        
        pReal->data_type = 2;    
        pReal->data_length =4;   
        pReal->v.value_u32 = 12; 
        
        ((condsArrarList->condsArray))[i]->msgId=i+1;
    }

    D32S verifyResult = 0;   

    (void)adm_uim_installConditionsVerifyResultNotify(eCmdidType,sessionId,state,condsArrarList,verifyResult);    

    for(D32S i = 0; i < 2;i++)
    {
        adm_free(((condsArrarList->condsArray))[i]->expectVariable);
        adm_free(((condsArrarList->condsArray))[i]->realVariable);
    	adm_free((condsArrarList->condsArray)[i]);
    }
    adm_free(condsArrarList->condsArray);
    adm_free(condsArrarList);
    
    m_iPolicyNotify = 1;
/*
    sessionId = (DCHAR*)"0x67890";
    state = (DCHAR*)"install policy 100%";
    ADM_UIM_SOA_INSTALL_PROGRESS_INFO_T* pstPolicyInstallProgress;
    pstPolicyInstallProgress = (ADM_UIM_SOA_INSTALL_PROGRESS_INFO_T*)malloc(sizeof(ADM_UIM_SOA_INSTALL_PROGRESS_INFO_T));
    pstPolicyInstallProgress->progress = 100;

    adm_uim_soaPolicyInstallProgressNotify(eCmdidType,sessionId,state,pstPolicyInstallProgress);
    free(pstPolicyInstallProgress);
    
    ADM_UIM_SOA_INSTALL_RESULT_INFO_T* pstPolicyInstallResult;
    pstPolicyInstallResult = (ADM_UIM_SOA_INSTALL_RESULT_INFO_T*)malloc(sizeof(ADM_UIM_SOA_INSTALL_RESULT_INFO_T));
    pstPolicyInstallResult->result = 0;
    pstPolicyInstallResult->expectVal = (DCHAR*)"0x10";
    pstPolicyInstallResult->realVal = (DCHAR*)"0x20";
    
    adm_uim_soaPolicyInstallResultNotify(eCmdidType, sessionId, state, pstPolicyInstallResult);
    free(pstPolicyInstallResult);
*/    
    return;
}


D32S UIMTEST_ADM_UIM_SOA_INSTALL_QUERY_ALL_ECU_PROGRESS_CALLBACK_F(ADM_APPLICATION_COMMON_CMDID_TYPE_E* eCmdidType, 
                                                                   DCHAR** sessionId, 
                                                                   DCHAR** state, 
                                                                   ADM_UIM_SOA_ECU_INSTALL_PROGRESS_ARRAY_INFO_T** pstEcuInstallProgressArrayInfo,  
                                                                   ADM_UIM_SOA_INSTALL_PROGRESS_INFO_T** pstPolicyInstallProgress,
                                                                   D32S* errorCode, 
                                                                   void* priv)
{
    *eCmdidType = ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_USER;
//    *sessionId = (DCHAR*)("0x12");
//  *state=(DCHAR*)("success");
    (*pstEcuInstallProgressArrayInfo) = (ADM_UIM_SOA_ECU_INSTALL_PROGRESS_ARRAY_INFO_T*)adm_malloc(sizeof(ADM_UIM_SOA_ECU_INSTALL_PROGRESS_ARRAY_INFO_T));
    (void)memset((*pstEcuInstallProgressArrayInfo),0,sizeof(ADM_UIM_SOA_ECU_INSTALL_PROGRESS_ARRAY_INFO_T));
    (*pstEcuInstallProgressArrayInfo)->ecuInstallProgressArrayLength = 2;
    (*pstEcuInstallProgressArrayInfo)->ecuInstallProgressArray = (ADM_UIM_SOA_ECU_INSTALL_PROGRESS_INFO_T**)adm_malloc(sizeof(ADM_UIM_SOA_ECU_INSTALL_PROGRESS_INFO_T*));
    //*((*pstEcuInstallProgressArrayInfo)->ecuInstallProgressArray) = (ADM_UIM_SOA_ECU_INSTALL_PROGRESS_INFO_T*)malloc(sizeof(ADM_UIM_SOA_ECU_INSTALL_PROGRESS_INFO_T)*2);
    for(D32S i = 0; i < 2;i++)
    {
    	((*pstEcuInstallProgressArrayInfo)->ecuInstallProgressArray)[i] = (ADM_UIM_SOA_ECU_INSTALL_PROGRESS_INFO_T*)adm_malloc(sizeof(ADM_UIM_SOA_ECU_INSTALL_PROGRESS_INFO_T));
        ((*pstEcuInstallProgressArrayInfo)->ecuInstallProgressArray)[i]->pstEcuIdenInfo = (ADM_UIM_SOA_ECU_IDEN_INFO_T*)adm_malloc(sizeof(ADM_UIM_SOA_ECU_IDEN_INFO_T));
        ((*pstEcuInstallProgressArrayInfo)->ecuInstallProgressArray)[i]->pstEcuIdenInfo->partNum = adm_strdup("0x1234");
        ((*pstEcuInstallProgressArrayInfo)->ecuInstallProgressArray)[i]->pstEcuIdenInfo->requestId=0x12;
        ((*pstEcuInstallProgressArrayInfo)->ecuInstallProgressArray)[i]->pstEcuIdenInfo->responseId=0x34;
        ((*pstEcuInstallProgressArrayInfo)->ecuInstallProgressArray)[i]->pstEcuIdenInfo->swDid=0x56;
        ((*pstEcuInstallProgressArrayInfo)->ecuInstallProgressArray)[i]->pstEcuIdenInfo->name=adm_strdup("test");
        ((*pstEcuInstallProgressArrayInfo)->ecuInstallProgressArray)[i]->pstEcuIdenInfo->softVer=adm_strdup("ver1.0");

        ((*pstEcuInstallProgressArrayInfo)->ecuInstallProgressArray)[i]->state = adm_strdup("success");
        ((*pstEcuInstallProgressArrayInfo)->ecuInstallProgressArray)[i]->pstEcuInstallProgress = (ADM_UIM_SOA_INSTALL_PROGRESS_INFO_T*)adm_malloc(sizeof(ADM_UIM_SOA_INSTALL_PROGRESS_INFO_T));
        ((*pstEcuInstallProgressArrayInfo)->ecuInstallProgressArray)[i]->pstEcuInstallProgress->progress=(D16U)100;
    }

    (*pstPolicyInstallProgress) = (ADM_UIM_SOA_INSTALL_PROGRESS_INFO_T*)adm_malloc(sizeof(ADM_UIM_SOA_INSTALL_PROGRESS_INFO_T));
    (*pstPolicyInstallProgress)->progress = (D16U)100;
    (*errorCode) = 0;
#if 0
    //���쵥������
    ADM_APPLICATION_COMMON_CMDID_TYPE_E eCmdidType1 = ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_USER; 
    DCHAR* sessionId1 = (DCHAR*)("0x9876"); 
    DCHAR* state1 = (DCHAR*)("ECU Install Success");
    ADM_UIM_SOA_ECU_IDEN_INFO_T* pstEcuIdenInfo1 = (ADM_UIM_SOA_ECU_IDEN_INFO_T*)malloc(sizeof(ADM_UIM_SOA_ECU_IDEN_INFO_T));
    pstEcuIdenInfo1->partNum = (DCHAR*)"0x1234567890";
    pstEcuIdenInfo1->requestId = 0x12;
    pstEcuIdenInfo1->responseId = 0x34;
    pstEcuIdenInfo1->swDid = 0x56;
    pstEcuIdenInfo1->name=(DCHAR*)("signal ECU");
    pstEcuIdenInfo1->softVer=(DCHAR*)("ver1.0");
    
    ADM_UIM_SOA_INSTALL_PROGRESS_INFO_T* pstInstallProgress;
    pstInstallProgress = (ADM_UIM_SOA_INSTALL_PROGRESS_INFO_T*)malloc(sizeof(ADM_UIM_SOA_INSTALL_PROGRESS_INFO_T));
    pstInstallProgress->progress = 100;
       
    adm_uim_soaEcuInstallProgressNotify(eCmdidType1, sessionId1, state1, pstEcuIdenInfo1,pstInstallProgress);
    free(pstEcuIdenInfo1);
    free(pstInstallProgress);
#endif    
    (void)adm_uim_allEcuInstallProgressNotify(*eCmdidType,"0x12","success",(*pstEcuInstallProgressArrayInfo),(*pstPolicyInstallProgress));

    return 0;
}                                                                    




D32S UIMTEST_ADM_UIM_SOA_INSTALL_ALL_ECU_RESULT_CALLBACK_F(ADM_APPLICATION_COMMON_CMDID_TYPE_E* eCmdidType, 
                                                                 DCHAR** sessionId, 
                                                                 DCHAR** state, 
                                                                 ADM_UIM_SOA_ECU_INSTALL_RESULT_ARRAY_INFO_T** pstEcuInstallResultArrayInfo, 
                                                                 ADM_UIM_SOA_INSTALL_RESULT_INFO_T** pstPolicyInstallResult, 
                                                                 D32S* errorCode, 
                                                                 void* priv)
{
/*    
    ADM_APPLICATION_COMMON_CMDID_TYPE_E eCmdidType1 = ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_USER;                 
    DCHAR* sessionId1 = (DCHAR*)"0x9876";                                   
    DCHAR* state1 = (DCHAR*)"ECU Download Result Notify";                 
    ADM_UIM_SOA_ECU_IDEN_INFO_T* pstEcuIdenInfo1 = NULL;
    pstEcuIdenInfo1 = (ADM_UIM_SOA_ECU_IDEN_INFO_T*)malloc(sizeof(ADM_UIM_SOA_ECU_IDEN_INFO_T));
    pstEcuIdenInfo1->partNum = (DCHAR*)("0x1234567890");
    pstEcuIdenInfo1->requestId = 0x12;
    pstEcuIdenInfo1->responseId = 0x34;
    pstEcuIdenInfo1->swDid = 0x56;
    pstEcuIdenInfo1->name = (DCHAR*)"result notify";
    pstEcuIdenInfo1->softVer = (DCHAR*)"ver1.0";

    ADM_UIM_SOA_INSTALL_RESULT_INFO_T* pstEcuDownloadResult1 = NULL;
    pstEcuDownloadResult1 = (ADM_UIM_SOA_INSTALL_RESULT_INFO_T*)malloc(sizeof(ADM_UIM_SOA_INSTALL_RESULT_INFO_T));
    pstEcuDownloadResult1->result =  0x0;
    pstEcuDownloadResult1->expectVal = (DCHAR*)"failed";
    pstEcuDownloadResult1->realVal = (DCHAR*)"success";    
    adm_uim_soaEcuInstallResultNotify(eCmdidType1,sessionId1,state1,pstEcuIdenInfo1,pstEcuDownloadResult1);
*/    
    
    *eCmdidType = ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_USER;
    *sessionId = adm_strdup("0x9876");
    *state=adm_strdup("success");
    (*pstEcuInstallResultArrayInfo) = (ADM_UIM_SOA_ECU_INSTALL_RESULT_ARRAY_INFO_T*)adm_malloc(sizeof(ADM_UIM_SOA_ECU_INSTALL_RESULT_ARRAY_INFO_T));
    (*pstEcuInstallResultArrayInfo)->ecuInstallResultArrayLength = 2;
    (*pstEcuInstallResultArrayInfo)->ecuInstallResultArray = (ADM_UIM_SOA_ECU_INSTALL_RESULT_INFO_T**)adm_malloc(sizeof(ADM_UIM_SOA_ECU_INSTALL_RESULT_INFO_T*));
    //*((*pstEcuInstallResultArrayInfo)->ecuInstallResultArray) = (ADM_UIM_SOA_ECU_INSTALL_RESULT_INFO_T*)malloc(sizeof(ADM_UIM_SOA_ECU_INSTALL_RESULT_INFO_T)*2);
    for(D32S i = 0; i < 2;i++)
    {
    	((*pstEcuInstallResultArrayInfo)->ecuInstallResultArray)[i] = (ADM_UIM_SOA_ECU_INSTALL_RESULT_INFO_T*)adm_malloc(sizeof(ADM_UIM_SOA_ECU_INSTALL_RESULT_INFO_T));
        ((*pstEcuInstallResultArrayInfo)->ecuInstallResultArray)[i]->pstEcuIdenInfo = (ADM_UIM_SOA_ECU_IDEN_INFO_T*)adm_malloc(sizeof(ADM_UIM_SOA_ECU_IDEN_INFO_T));
        ((*pstEcuInstallResultArrayInfo)->ecuInstallResultArray)[i]->pstEcuIdenInfo->partNum = adm_strdup("0x1234");
        ((*pstEcuInstallResultArrayInfo)->ecuInstallResultArray)[i]->pstEcuIdenInfo->requestId=0x12;
        ((*pstEcuInstallResultArrayInfo)->ecuInstallResultArray)[i]->pstEcuIdenInfo->responseId=0x34;
        ((*pstEcuInstallResultArrayInfo)->ecuInstallResultArray)[i]->pstEcuIdenInfo->swDid=0x56;
        ((*pstEcuInstallResultArrayInfo)->ecuInstallResultArray)[i]->pstEcuIdenInfo->name=adm_strdup("test");
        ((*pstEcuInstallResultArrayInfo)->ecuInstallResultArray)[i]->pstEcuIdenInfo->softVer=adm_strdup("ver1.0");

        ((*pstEcuInstallResultArrayInfo)->ecuInstallResultArray)[i]->pstEcuInstallResult = (ADM_UIM_SOA_INSTALL_RESULT_INFO_T*)adm_malloc(sizeof(ADM_UIM_SOA_INSTALL_RESULT_INFO_T));
        ((*pstEcuInstallResultArrayInfo)->ecuInstallResultArray)[i]->pstEcuInstallResult->result=0;
        ((*pstEcuInstallResultArrayInfo)->ecuInstallResultArray)[i]->pstEcuInstallResult->expectVal = adm_strdup("1");
        ((*pstEcuInstallResultArrayInfo)->ecuInstallResultArray)[i]->pstEcuInstallResult->realVal = adm_strdup("0");
    }
    (*pstPolicyInstallResult) = (ADM_UIM_SOA_INSTALL_RESULT_INFO_T*)adm_malloc(sizeof(ADM_UIM_SOA_INSTALL_RESULT_INFO_T));
    (*pstPolicyInstallResult)->result = 0;
    (*pstPolicyInstallResult)->expectVal = adm_strdup("1");
    (*pstPolicyInstallResult)->realVal = adm_strdup("0");
    (*errorCode) = 0;
    
    (void)adm_uim_allEcuInstallResultNotify(*eCmdidType,*sessionId,*state,(*pstEcuInstallResultArrayInfo),(*pstPolicyInstallResult));
    for(D32S i = 0; i < 2;i++)
    {
        adm_free(((*pstEcuInstallResultArrayInfo)->ecuInstallResultArray)[i]->pstEcuIdenInfo->partNum);
        adm_free(((*pstEcuInstallResultArrayInfo)->ecuInstallResultArray)[i]->pstEcuIdenInfo->name);
        adm_free(((*pstEcuInstallResultArrayInfo)->ecuInstallResultArray)[i]->pstEcuIdenInfo->softVer);
        adm_free(((*pstEcuInstallResultArrayInfo)->ecuInstallResultArray)[i]->pstEcuInstallResult->expectVal);
        adm_free(((*pstEcuInstallResultArrayInfo)->ecuInstallResultArray)[i]->pstEcuInstallResult->realVal);
        adm_free(((*pstEcuInstallResultArrayInfo)->ecuInstallResultArray)[i]->pstEcuIdenInfo);
        adm_free(((*pstEcuInstallResultArrayInfo)->ecuInstallResultArray)[i]->pstEcuInstallResult);
        adm_free(((*pstEcuInstallResultArrayInfo)->ecuInstallResultArray)[i]);
    }
    adm_free(*sessionId);
    adm_free(*state);
    adm_free((*pstEcuInstallResultArrayInfo)->ecuInstallResultArray);
    adm_free((*pstPolicyInstallResult)->expectVal);
    adm_free((*pstPolicyInstallResult)->realVal);
    adm_free(*pstPolicyInstallResult);

    return 0;
}
