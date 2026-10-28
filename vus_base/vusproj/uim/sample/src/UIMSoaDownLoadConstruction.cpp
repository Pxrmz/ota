
#include "UimSoaDownloadConstruction.h"
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include "adm_memory.h"
#include "adm_string.h"


static D32S m_iCondVerifyNotify = 0;
static D32S m_iECUDownLoadNotify = 0;
static D32S m_iALLECUDownLoadNotify = 0;
static D32S m_iCancelDownLoad=0;
static D32S m_iPauseDownLoad=0;

static void* UIMTEST_ADM_SOA_DOWNLOAD_POLICYPROGRESS_THREAD(void* p)
{
    D32S iPolicyCounter=0;
    while(TRUE)
    {
        if(0 != m_iCondVerifyNotify)
        {
            if(0 != m_iCancelDownLoad)
            {
                //ȡ��
                return NULL;
            }
            if(0 != m_iPauseDownLoad)
            {
                //��ͣ
                (void)usleep(10);
                continue;
            }
            ADM_APPLICATION_COMMON_CMDID_TYPE_E eCmdidType = ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_USER; 
            const DCHAR* sessionId = "0x9876"; 
            const DCHAR* state = "Success"; 
            if(iPolicyCounter > 100)
            {
                //�������Խ����߳�
                ADM_UIM_SOA_DOWNLOAD_RESULT_INFO_T* pstPolicyDownloadResult = NULL;
                pstPolicyDownloadResult = (ADM_UIM_SOA_DOWNLOAD_RESULT_INFO_T*)adm_malloc(sizeof(ADM_UIM_SOA_DOWNLOAD_RESULT_INFO_T));

                pstPolicyDownloadResult->result = 1;
                pstPolicyDownloadResult->expectVal = adm_strdup("0x01");
                pstPolicyDownloadResult->realVal   = adm_strdup("0x01");

                (void)adm_uim_policyDownloadResultNotify(eCmdidType,sessionId,state,pstPolicyDownloadResult);
                adm_free(pstPolicyDownloadResult);
                m_iCondVerifyNotify = 0;
                //�������ؽ����ϱ�
                m_iECUDownLoadNotify=1;

                adm_free(pstPolicyDownloadResult->expectVal);
                adm_free(pstPolicyDownloadResult->realVal);
                return NULL;
            }
            else
            {
                ADM_UIM_SOA_DOWNLOAD_PROGRESS_INFO_T* pstPolicyProgress = NULL;
                pstPolicyProgress = (ADM_UIM_SOA_DOWNLOAD_PROGRESS_INFO_T*)adm_malloc(sizeof(ADM_UIM_SOA_DOWNLOAD_PROGRESS_INFO_T));
                pstPolicyProgress->savedSize =iPolicyCounter++; 
                pstPolicyProgress->totalSize =100; 
                pstPolicyProgress->speed     =1;
                pstPolicyProgress->remainTime =1;

                adm_uim_policyDownloadProgressNotify(eCmdidType,sessionId,state,pstPolicyProgress);
                adm_free(pstPolicyProgress);
            }         
        }
        else
        {
            iPolicyCounter = 0;
        }
        (void)usleep(10);
    }

//    return NULL;
}

static void* UIMTEST_ADM_SOA_ECUDOWNLOAD_PROGRESS_THREAD(void* p)
{
    D32S iProgress = 0;
    while(TRUE)
    {
        if(0 != m_iECUDownLoadNotify)
        {
            if(0 != m_iCancelDownLoad)
            {
                //ȡ��
                return NULL;
            }
            if(0 != m_iPauseDownLoad)
            {
                //��ͣ
                (void)usleep(10);
                continue;
            }            
            if(iProgress > 100)
            {
                ADM_APPLICATION_COMMON_CMDID_TYPE_E eCmdidType1 = ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_USER;                 
                DCHAR* sessionId1 = adm_strdup("0x9876");                                   
                DCHAR* state1 = adm_strdup("ECU Download Result Notify");                 
                ADM_UIM_SOA_ECU_IDEN_INFO_T* pstEcuIdenInfo1 = NULL;
                pstEcuIdenInfo1 = (ADM_UIM_SOA_ECU_IDEN_INFO_T*)adm_malloc(sizeof(ADM_UIM_SOA_ECU_IDEN_INFO_T));
                pstEcuIdenInfo1->partNum = adm_strdup("0x1234567890");
                pstEcuIdenInfo1->requestId = 0x12;
                pstEcuIdenInfo1->responseId = 0x34;
                pstEcuIdenInfo1->swDid = 0x56;
                pstEcuIdenInfo1->name = adm_strdup("result notify");
                pstEcuIdenInfo1->softVer = adm_strdup("ver1.0");

                ADM_UIM_SOA_DOWNLOAD_RESULT_INFO_T* pstEcuDownloadResult1 = NULL;
                pstEcuDownloadResult1 = (ADM_UIM_SOA_DOWNLOAD_RESULT_INFO_T*)adm_malloc(sizeof(ADM_UIM_SOA_DOWNLOAD_RESULT_INFO_T));
                pstEcuDownloadResult1->result =  0x1;
                pstEcuDownloadResult1->expectVal = adm_strdup("0x01");
                pstEcuDownloadResult1->realVal = adm_strdup("0x01");

                //���쵥��ECU����¼�
                (void)adm_uim_ecuDownloadResultNotify(eCmdidType1,sessionId1,state1,pstEcuIdenInfo1,pstEcuDownloadResult1);

                adm_free(sessionId1);
                adm_free(state1);
                adm_free(pstEcuIdenInfo1->partNum);
                adm_free(pstEcuIdenInfo1->name);
                adm_free(pstEcuIdenInfo1->softVer);
                adm_free(pstEcuDownloadResult1->expectVal);
                adm_free(pstEcuDownloadResult1->realVal);
                adm_free(pstEcuDownloadResult1);
                adm_free(pstEcuIdenInfo1);
                //֪ͨ����,�����ϱ�����
                m_iECUDownLoadNotify = 0;
                //����ALL ECU DownLoadProgress Notify
                m_iALLECUDownLoadNotify = 1;
                return NULL;
            }
            else
            {
                //���쵥��ECU���ؽ����¼�
                ADM_APPLICATION_COMMON_CMDID_TYPE_E eCmdidType1 = ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_USER; 
                DCHAR* sessionId1 = adm_strdup("0x9876"); 
                DCHAR* state1 = adm_strdup("ECU Download Success");

                ADM_UIM_SOA_ECU_IDEN_INFO_T* pstEcuIdenInfo1 = (ADM_UIM_SOA_ECU_IDEN_INFO_T*)adm_malloc(sizeof(ADM_UIM_SOA_ECU_IDEN_INFO_T));
                pstEcuIdenInfo1->partNum = adm_strdup("0x1234567890");
                pstEcuIdenInfo1->requestId = 0x12;
                pstEcuIdenInfo1->responseId = 0x34;
                pstEcuIdenInfo1->swDid = 0x56;
                pstEcuIdenInfo1->name=adm_strdup("signal ECU");
                pstEcuIdenInfo1->softVer=adm_strdup("ver1.0");

                ADM_UIM_SOA_DOWNLOAD_PROGRESS_INFO_T* pstProgress1 = (ADM_UIM_SOA_DOWNLOAD_PROGRESS_INFO_T*)adm_malloc(sizeof(ADM_UIM_SOA_DOWNLOAD_PROGRESS_INFO_T));
                pstProgress1->savedSize = iProgress++;
                pstProgress1->totalSize = 100;
                pstProgress1->speed     =1;
                pstProgress1->remainTime = 1;

                (void)adm_uim_ecuDownloadProgressNotify(eCmdidType1,sessionId1,state1,pstEcuIdenInfo1,pstProgress1);
                adm_free(pstEcuIdenInfo1->partNum);
                adm_free(pstEcuIdenInfo1->name);
                adm_free(pstEcuIdenInfo1->softVer);
                adm_free(pstEcuIdenInfo1);
                adm_free(pstProgress1);
                adm_free(sessionId1);
                adm_free(state1);
            }
        }
        else
        {
            iProgress = 0;
        }
        (void)usleep(10);
    }
//    return NULL;
}

static void* UIMTEST_ADM_SOA_ALLDOWNLOAD_PROGRESS_THREAD(void* p)
{
    D32S iProgress = 0;
    D32S iTotalProgress = 0;
    ADM_APPLICATION_COMMON_CMDID_TYPE_E eCmdidType;
    const DCHAR* sessionId;
    const DCHAR* state;
    while(TRUE)
    {
        if(0 != m_iALLECUDownLoadNotify)
        {
            if(0 != m_iCancelDownLoad)
            {
                //ȡ��
                return NULL;
            }
            if(0 != m_iPauseDownLoad)
            {
                //��ͣ
                (void)usleep(10);
                continue;
            }
            if(iTotalProgress > 100)
            {
                eCmdidType = ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_USER;
                sessionId  = "0x9876";
                state      = "Success";
                ADM_UIM_SOA_ECU_DOWNLOAD_RESULT_ARRAY_INFO_T* pstEcuDownloadResultArrayInfo;
                ADM_UIM_SOA_DOWNLOAD_RESULT_INFO_T* pstPolicyDownloadResult;
                
                (pstEcuDownloadResultArrayInfo) = (ADM_UIM_SOA_ECU_DOWNLOAD_RESULT_ARRAY_INFO_T*)adm_malloc(sizeof(ADM_UIM_SOA_ECU_DOWNLOAD_RESULT_ARRAY_INFO_T));
                (pstEcuDownloadResultArrayInfo)->ecuDownloadResultArrayLength = 2;
                (pstEcuDownloadResultArrayInfo)->ecuDownloadResultArray = (ADM_UIM_SOA_ECU_DOWNLOAD_RESULT_INFO_T**)adm_malloc(sizeof(ADM_UIM_SOA_ECU_DOWNLOAD_RESULT_INFO_T*));
               // *((pstEcuDownloadResultArrayInfo)->ecuDownloadResultArray) = (ADM_UIM_SOA_ECU_DOWNLOAD_RESULT_INFO_T*)malloc(sizeof(ADM_UIM_SOA_ECU_DOWNLOAD_RESULT_INFO_T)*2);
                for(D32S i = 0; i < 2;i++)
                {
                ((pstEcuDownloadResultArrayInfo)->ecuDownloadResultArray)[i] = (ADM_UIM_SOA_ECU_DOWNLOAD_RESULT_INFO_T*)adm_malloc(sizeof(ADM_UIM_SOA_ECU_DOWNLOAD_RESULT_INFO_T));
                    (((pstEcuDownloadResultArrayInfo)->ecuDownloadResultArray))[i]->pstEcuIdenInfo = (ADM_UIM_SOA_ECU_IDEN_INFO_T*)adm_malloc(sizeof(ADM_UIM_SOA_ECU_IDEN_INFO_T));
                    (((pstEcuDownloadResultArrayInfo)->ecuDownloadResultArray))[i]->pstEcuDownloadResult = (ADM_UIM_SOA_DOWNLOAD_RESULT_INFO_T*)adm_malloc(sizeof(ADM_UIM_SOA_DOWNLOAD_RESULT_INFO_T));
                    ADM_UIM_SOA_ECU_IDEN_INFO_T* p = (((pstEcuDownloadResultArrayInfo)->ecuDownloadResultArray))[i]->pstEcuIdenInfo;
                    ADM_UIM_SOA_DOWNLOAD_RESULT_INFO_T* q = (((pstEcuDownloadResultArrayInfo)->ecuDownloadResultArray))[i]->pstEcuDownloadResult;
                    (((pstEcuDownloadResultArrayInfo)->ecuDownloadResultArray))[i]->state=adm_strdup("success");
                    p->partNum = adm_strdup("0x1234");
                    p->requestId = 0x12;
                    p->responseId = 0x34;
                    p->swDid      = 0x56;
                    p->name       = adm_strdup("test");
                    p->softVer    = adm_strdup("Ver1.0");
                    
//                    (((pstEcuDownloadResultArrayInfo)->ecuDownloadResultArray))[i]->state=adm_strdup("success");
                    
                    q->result     = 1;
                    q->expectVal  =adm_strdup("1");
                    q->realVal    =adm_strdup("1");
                }
                
                (pstPolicyDownloadResult)=(ADM_UIM_SOA_DOWNLOAD_RESULT_INFO_T*)adm_malloc(sizeof(ADM_UIM_SOA_DOWNLOAD_RESULT_INFO_T));
                (pstPolicyDownloadResult)->result = 1;
                (pstPolicyDownloadResult)->expectVal = adm_strdup("0x01");
                (pstPolicyDownloadResult)->realVal   = adm_strdup("0x01");               

                (void)adm_uim_allEcuDownloadResultNotify(eCmdidType, sessionId, state, (pstEcuDownloadResultArrayInfo),(pstPolicyDownloadResult));
                for(D32S i = 0; i < 2;i++)
                {
                    adm_free((((pstEcuDownloadResultArrayInfo)->ecuDownloadResultArray))[i]->state);
                    adm_free((((pstEcuDownloadResultArrayInfo)->ecuDownloadResultArray))[i]->pstEcuIdenInfo->partNum);
                    adm_free((((pstEcuDownloadResultArrayInfo)->ecuDownloadResultArray))[i]->pstEcuIdenInfo->name);
                    adm_free((((pstEcuDownloadResultArrayInfo)->ecuDownloadResultArray))[i]->pstEcuIdenInfo->softVer);
                    adm_free((((pstEcuDownloadResultArrayInfo)->ecuDownloadResultArray))[i]->pstEcuDownloadResult->expectVal);
                    adm_free((((pstEcuDownloadResultArrayInfo)->ecuDownloadResultArray))[i]->pstEcuDownloadResult->realVal);
                    adm_free((((pstEcuDownloadResultArrayInfo)->ecuDownloadResultArray))[i]->pstEcuIdenInfo);
                    adm_free((((pstEcuDownloadResultArrayInfo)->ecuDownloadResultArray))[i]->pstEcuDownloadResult);
                	adm_free(((pstEcuDownloadResultArrayInfo)->ecuDownloadResultArray)[i]);
                }
                adm_free((pstEcuDownloadResultArrayInfo)->ecuDownloadResultArray);
                adm_free(pstEcuDownloadResultArrayInfo);
                adm_free(pstPolicyDownloadResult);

                m_iALLECUDownLoadNotify = 0;
                return NULL;
            }
            else
            {
                ADM_UIM_SOA_ECU_DOWNLOAD_PROGRESS_ARRAY_INFO_T* pstEcuDownloadProgressArrayInfo;
                ADM_UIM_SOA_DOWNLOAD_PROGRESS_INFO_T* pstPolicyDownloadProgress;

                //����ALL ECU������Ϣ
                eCmdidType = ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_USER;
                sessionId = "0x9876";
                state= "success";
                (pstEcuDownloadProgressArrayInfo) = (ADM_UIM_SOA_ECU_DOWNLOAD_PROGRESS_ARRAY_INFO_T*)adm_malloc(sizeof(ADM_UIM_SOA_ECU_DOWNLOAD_PROGRESS_ARRAY_INFO_T));

                (pstEcuDownloadProgressArrayInfo)->ecuDownloadProgressArrayLength = 2;
                (pstEcuDownloadProgressArrayInfo)->ecuDownloadProgressArray = (ADM_UIM_SOA_ECU_DOWNLOAD_PROGRESS_INFO_T**)adm_malloc(sizeof(ADM_UIM_SOA_ECU_DOWNLOAD_PROGRESS_INFO_T*));
                //*((pstEcuDownloadProgressArrayInfo)->ecuDownloadProgressArray) = (ADM_UIM_SOA_ECU_DOWNLOAD_PROGRESS_INFO_T*)malloc(sizeof(ADM_UIM_SOA_ECU_DOWNLOAD_PROGRESS_INFO_T)*2);
                for(D32S i = 0; i < 2;i++)
                {
                ((pstEcuDownloadProgressArrayInfo)->ecuDownloadProgressArray)[i] = (ADM_UIM_SOA_ECU_DOWNLOAD_PROGRESS_INFO_T*)adm_malloc(sizeof(ADM_UIM_SOA_ECU_DOWNLOAD_PROGRESS_INFO_T));
                    ((pstEcuDownloadProgressArrayInfo)->ecuDownloadProgressArray)[i]->pstEcuIdenInfo = (ADM_UIM_SOA_ECU_IDEN_INFO_T*)adm_malloc(sizeof(ADM_UIM_SOA_ECU_IDEN_INFO_T));
                    ((pstEcuDownloadProgressArrayInfo)->ecuDownloadProgressArray)[i]->pstEcuIdenInfo->partNum = adm_strdup("0x123455");
                    ((pstEcuDownloadProgressArrayInfo)->ecuDownloadProgressArray)[i]->pstEcuIdenInfo->requestId=0x12;
                    ((pstEcuDownloadProgressArrayInfo)->ecuDownloadProgressArray)[i]->pstEcuIdenInfo->responseId=0x34;
                    ((pstEcuDownloadProgressArrayInfo)->ecuDownloadProgressArray)[i]->pstEcuIdenInfo->swDid=0x56;
                    ((pstEcuDownloadProgressArrayInfo)->ecuDownloadProgressArray)[i]->pstEcuIdenInfo->name=adm_strdup("test");
                    ((pstEcuDownloadProgressArrayInfo)->ecuDownloadProgressArray)[i]->pstEcuIdenInfo->softVer=adm_strdup("ver1.0");

                    ((pstEcuDownloadProgressArrayInfo)->ecuDownloadProgressArray)[i]->pstEcuDownloadProgress = (ADM_UIM_SOA_DOWNLOAD_PROGRESS_INFO_T*)adm_malloc(sizeof(ADM_UIM_SOA_DOWNLOAD_PROGRESS_INFO_T));
                    ((pstEcuDownloadProgressArrayInfo)->ecuDownloadProgressArray)[i]->pstEcuDownloadProgress->savedSize=iProgress;
                    ((pstEcuDownloadProgressArrayInfo)->ecuDownloadProgressArray)[i]->pstEcuDownloadProgress->totalSize=100;
                    ((pstEcuDownloadProgressArrayInfo)->ecuDownloadProgressArray)[i]->pstEcuDownloadProgress->speed=1;
                    ((pstEcuDownloadProgressArrayInfo)->ecuDownloadProgressArray)[i]->pstEcuDownloadProgress->remainTime=1;
                }
                iProgress++;
                (pstPolicyDownloadProgress) = (ADM_UIM_SOA_DOWNLOAD_PROGRESS_INFO_T*)adm_malloc(sizeof(ADM_UIM_SOA_DOWNLOAD_PROGRESS_INFO_T));
                (pstPolicyDownloadProgress)->savedSize = iTotalProgress++;
                (pstPolicyDownloadProgress)->totalSize = 100;
                (pstPolicyDownloadProgress)->speed     = 1;
                (pstPolicyDownloadProgress)->remainTime= 1;    

                //����ALL ECU Event ֪ͨ
                (void)adm_uim_allEcuDownloadProgressNotify(eCmdidType, sessionId, state, (pstEcuDownloadProgressArrayInfo),(pstPolicyDownloadProgress));
                for(D32S i = 0;i< 2;i++)
                {
                    adm_free(((pstEcuDownloadProgressArrayInfo)->ecuDownloadProgressArray)[i]->pstEcuIdenInfo->partNum);
                    adm_free(((pstEcuDownloadProgressArrayInfo)->ecuDownloadProgressArray)[i]->pstEcuIdenInfo->name);
                    adm_free(((pstEcuDownloadProgressArrayInfo)->ecuDownloadProgressArray)[i]->pstEcuIdenInfo->softVer);
                    adm_free(((pstEcuDownloadProgressArrayInfo)->ecuDownloadProgressArray)[i]->pstEcuIdenInfo);
                    adm_free(((pstEcuDownloadProgressArrayInfo)->ecuDownloadProgressArray)[i]->pstEcuDownloadProgress);
					adm_free(((pstEcuDownloadProgressArrayInfo)->ecuDownloadProgressArray)[i]);
                }
                adm_free((pstEcuDownloadProgressArrayInfo)->ecuDownloadProgressArray);
                adm_free(pstEcuDownloadProgressArrayInfo);
            }
        }
        else
        {
            iProgress = 0;
        }
    }
 //   return NULL;
}

void UIMTEST_ADM_SOA_DOWNLOAD_PROGRESS_SIMULATE(void)
{
    pthread_t thrIDPolicyProgress;
    (void)pthread_create(&thrIDPolicyProgress,NULL,&UIMTEST_ADM_SOA_DOWNLOAD_POLICYPROGRESS_THREAD,NULL);

    pthread_t thrIDECUDownLoadP;
    (void)pthread_create(&thrIDECUDownLoadP,NULL,&UIMTEST_ADM_SOA_ECUDOWNLOAD_PROGRESS_THREAD,NULL);

    pthread_t thrIDALLECUDownLoadP;
    (void)pthread_create(&thrIDALLECUDownLoadP,NULL,&UIMTEST_ADM_SOA_ALLDOWNLOAD_PROGRESS_THREAD,NULL);

    return;
}


void UIMTEST_ADM_UIM_SOA_DOWNLOAD_TRIGGER_CALLBACK_F(ADM_UIM_SOA_ACTION_AUTH_TYPE_E* eActionType, 
                                                     void* priv)
{
    *eActionType = ADM_UIM_SOA_ACTION_AUTH_TYPE_ALLOW;

    m_iCondVerifyNotify = 0;    
    m_iECUDownLoadNotify = 0;   
    m_iALLECUDownLoadNotify = 0;
    m_iCancelDownLoad=0;        
    m_iPauseDownLoad=0;
 
    //�����̴߳��������ϱ�
    UIMTEST_ADM_SOA_DOWNLOAD_PROGRESS_SIMULATE();
    return;
}

void UIMTEST_ADM_UIM_SOA_DOWNLOAD_AUTO_CALLBACK_F(ADM_UIM_SOA_AUTO_DOWNLOAD_TYPE_E eSwitchType, D32S *errCode)
{
	*errCode = 0;

    return;
}

void UIMTEST_ADM_UIM_SOA_DOWNLOAD_CALCEL_CALLBACK_F(const ADM_APPLICATION_COMMON_CMDID_TYPE_E eCmdidType, 
                                                    ADM_UIM_SOA_ACTION_AUTH_TYPE_E* eActionType, 
                                                    D32S* errorCode, 
                                                    void* priv)
{
    *eActionType = ADM_UIM_SOA_ACTION_AUTH_TYPE_ALLOW;
    *errorCode   = 0;
    m_iCancelDownLoad = 1; 
    return;
}

void UIMTEST_ADM_UIM_SOA_DOWNLOAD_PAUSED_CALLBACK_F(const ADM_APPLICATION_COMMON_CMDID_TYPE_E eCmdidType, 
                                                    ADM_UIM_SOA_ACTION_AUTH_TYPE_E* eActionType, 
                                                    D32S* errorCode, 
                                                    void* priv)
{
    *eActionType = ADM_UIM_SOA_ACTION_AUTH_TYPE_ALLOW;
    *errorCode = 0;
    m_iPauseDownLoad = 1;
    return;
}

void UIMTEST_ADM_UIM_SOA_DOWNLOAD_CONDITIONS_VERIFIED_CALLBACK_F(const ADM_UIM_SOA_CONDITIONS_VERIFIED_TYPE_E eVerifiedType, 
                                                                 const D16U cycleTime, 
                                                                 const D16U timeout, 
                                                                 D32S* errorCode)
{
    *errorCode = 0;
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
        (condsArrarList->condsArray)[i]->expectVariable = (variable_st*)adm_malloc(sizeof(variable_st));
        (condsArrarList->condsArray)[i]->realVariable   = (variable_st*)adm_malloc(sizeof(variable_st));

        variable_st* pExpt = (condsArrarList->condsArray)[i]->expectVariable;
        variable_st* pReal = (condsArrarList->condsArray)[i]->realVariable;

        pExpt->data_type = 2;
        pExpt->data_length =4;
        pExpt->v.value_u32 = 12;
        
        pReal->data_type = 2;    
        pReal->data_length =4;   
        pReal->v.value_u32 = 12; 
        
        (condsArrarList->condsArray)[i]->msgId=i+1;
        (condsArrarList->condsArray)[i]->result=0;
    }

    D32S verifyResult = 0;
    adm_uim_downloadConditionsVerifyResultNotify(eCmdidType,sessionId,state,condsArrarList,verifyResult);
    for(D32S i = 0;i<2;i++)
    {
        adm_free((condsArrarList->condsArray)[i]->expectVariable);
        adm_free((condsArrarList->condsArray)[i]->realVariable);
    	adm_free((condsArrarList->condsArray)[i]);
    }
    adm_free(condsArrarList->condsArray);
    adm_free(condsArrarList);

    m_iCondVerifyNotify = 1;
/*
    //������Խ����¼�
    ADM_UIM_SOA_DOWNLOAD_PROGRESS_INFO_T* pstPolicyProgress = NULL;
    pstPolicyProgress = (ADM_UIM_SOA_DOWNLOAD_PROGRESS_INFO_T*)malloc(sizeof(ADM_UIM_SOA_DOWNLOAD_PROGRESS_INFO_T));
    pstPolicyProgress->savedSize =1024; 
    pstPolicyProgress->totalSize =1024; 
    pstPolicyProgress->speed     =1;
    pstPolicyProgress->remainTime =60;

    adm_uim_soaPolicyDownloadProgressNotify(eCmdidType,sessionId,state,pstPolicyProgress);
    free(pstPolicyProgress);

    //������Խ���¼�
    ADM_UIM_SOA_DOWNLOAD_RESULT_INFO_T* pstPolicyDownloadResult = NULL;
    pstPolicyDownloadResult = (ADM_UIM_SOA_DOWNLOAD_RESULT_INFO_T*)malloc(sizeof(ADM_UIM_SOA_DOWNLOAD_RESULT_INFO_T));

    pstPolicyDownloadResult->result = 1;
    pstPolicyDownloadResult->expectVal = (DCHAR*)"0x123456";
    pstPolicyDownloadResult->realVal   = (DCHAR*)"0x987654";

    adm_uim_soaPolicyDownloadResultNotify(eCmdidType,sessionId,state,pstPolicyDownloadResult);
    free(pstPolicyDownloadResult);
*/    
    return;
}


D32S UIMTEST_ADM_UIM_SOA_DOWNLOAD_QUERY_ALL_ECU_PROGRESS_CALLBACK_F(ADM_APPLICATION_COMMON_CMDID_TYPE_E* eCmdidType, 
                                                                    DCHAR** sessionId, 
                                                                    DCHAR** state, 
                                                                    ADM_UIM_SOA_ECU_DOWNLOAD_PROGRESS_ARRAY_INFO_T** pstEcuDownloadProgressArrayInfo, 
                                                                    ADM_UIM_SOA_DOWNLOAD_PROGRESS_INFO_T** pstPolicyDownloadProgress, 
                                                                    D32S* errorCode, 
                                                                    void* priv)
{
    m_iECUDownLoadNotify = 1;
   
    //���쵥��ECU���ؽ����¼�
    ADM_APPLICATION_COMMON_CMDID_TYPE_E eCmdidType1 = ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_USER; 
    const DCHAR* sessionId1 = "0x9876"; 
    const DCHAR* state1 = "ECU Download Success";
/*    
    ADM_UIM_SOA_ECU_IDEN_INFO_T* pstEcuIdenInfo1 = (ADM_UIM_SOA_ECU_IDEN_INFO_T*)malloc(sizeof(ADM_UIM_SOA_ECU_IDEN_INFO_T));
    pstEcuIdenInfo1->partNum = (DCHAR*)"0x1234567890";
    pstEcuIdenInfo1->requestId = 0x12;
    pstEcuIdenInfo1->responseId = 0x34;
    pstEcuIdenInfo1->swDid = 0x56;
    pstEcuIdenInfo1->name=(DCHAR*)("signal ECU");
    pstEcuIdenInfo1->softVer=(DCHAR*)("ver1.0");
    
    ADM_UIM_SOA_DOWNLOAD_PROGRESS_INFO_T* pstProgress1 = (ADM_UIM_SOA_DOWNLOAD_PROGRESS_INFO_T*)malloc(sizeof(ADM_UIM_SOA_DOWNLOAD_PROGRESS_INFO_T));
    pstProgress1->savedSize = 100;
    pstProgress1->totalSize = 100;
    pstProgress1->speed     =2;
    pstProgress1->remainTime = 120;

    adm_uim_soaEcuDownloadProgressNotify(eCmdidType1,sessionId1,state1,pstEcuIdenInfo1,pstProgress1);
    free(pstEcuIdenInfo1);
    free(pstProgress1);
*/

    //����ALL ECU������Ϣ
    *eCmdidType = ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_USER;
    *sessionId = adm_strdup("0x12");
    *state=adm_strdup("success");
    *pstEcuDownloadProgressArrayInfo = (ADM_UIM_SOA_ECU_DOWNLOAD_PROGRESS_ARRAY_INFO_T*)adm_malloc(sizeof(ADM_UIM_SOA_ECU_DOWNLOAD_PROGRESS_ARRAY_INFO_T));
    (void)memset((*pstEcuDownloadProgressArrayInfo),0,sizeof(ADM_UIM_SOA_ECU_DOWNLOAD_PROGRESS_ARRAY_INFO_T));
    (*pstEcuDownloadProgressArrayInfo)->ecuDownloadProgressArrayLength = 2;
    (*pstEcuDownloadProgressArrayInfo)->ecuDownloadProgressArray = (ADM_UIM_SOA_ECU_DOWNLOAD_PROGRESS_INFO_T**)adm_malloc(sizeof(ADM_UIM_SOA_ECU_DOWNLOAD_PROGRESS_INFO_T*));
   // *((*pstEcuDownloadProgressArrayInfo)->ecuDownloadProgressArray) = (ADM_UIM_SOA_ECU_DOWNLOAD_PROGRESS_INFO_T*)malloc(sizeof(ADM_UIM_SOA_ECU_DOWNLOAD_PROGRESS_INFO_T)*2);
    for(D32S i = 0; i < 2;i++)
    {
        ((*pstEcuDownloadProgressArrayInfo)->ecuDownloadProgressArray)[i] = (ADM_UIM_SOA_ECU_DOWNLOAD_PROGRESS_INFO_T*)adm_malloc(sizeof(ADM_UIM_SOA_ECU_DOWNLOAD_PROGRESS_INFO_T));
        ((*pstEcuDownloadProgressArrayInfo)->ecuDownloadProgressArray)[i]->pstEcuIdenInfo = (ADM_UIM_SOA_ECU_IDEN_INFO_T*)adm_malloc(sizeof(ADM_UIM_SOA_ECU_IDEN_INFO_T));
        ((*pstEcuDownloadProgressArrayInfo)->ecuDownloadProgressArray)[i]->pstEcuIdenInfo->partNum = adm_strdup("0x1234");
        ((*pstEcuDownloadProgressArrayInfo)->ecuDownloadProgressArray)[i]->pstEcuIdenInfo->requestId=0x01;
        ((*pstEcuDownloadProgressArrayInfo)->ecuDownloadProgressArray)[i]->pstEcuIdenInfo->responseId=0x02;
        ((*pstEcuDownloadProgressArrayInfo)->ecuDownloadProgressArray)[i]->pstEcuIdenInfo->swDid=0x03;
        ((*pstEcuDownloadProgressArrayInfo)->ecuDownloadProgressArray)[i]->pstEcuIdenInfo->name=adm_strdup("test");
        ((*pstEcuDownloadProgressArrayInfo)->ecuDownloadProgressArray)[i]->pstEcuIdenInfo->softVer=adm_strdup("ver1.0");

        ((*pstEcuDownloadProgressArrayInfo)->ecuDownloadProgressArray)[i]->pstEcuDownloadProgress = (ADM_UIM_SOA_DOWNLOAD_PROGRESS_INFO_T*)adm_malloc(sizeof(ADM_UIM_SOA_DOWNLOAD_PROGRESS_INFO_T));
        ((*pstEcuDownloadProgressArrayInfo)->ecuDownloadProgressArray)[i]->pstEcuDownloadProgress->savedSize=100;
        ((*pstEcuDownloadProgressArrayInfo)->ecuDownloadProgressArray)[i]->pstEcuDownloadProgress->totalSize=100;
        ((*pstEcuDownloadProgressArrayInfo)->ecuDownloadProgressArray)[i]->pstEcuDownloadProgress->speed=1;
        ((*pstEcuDownloadProgressArrayInfo)->ecuDownloadProgressArray)[i]->pstEcuDownloadProgress->remainTime=1;
    }

    (*pstPolicyDownloadProgress) = (ADM_UIM_SOA_DOWNLOAD_PROGRESS_INFO_T*)adm_malloc(sizeof(ADM_UIM_SOA_DOWNLOAD_PROGRESS_INFO_T));
    (*pstPolicyDownloadProgress)->savedSize = 100;
    (*pstPolicyDownloadProgress)->totalSize = 100;
    (*pstPolicyDownloadProgress)->speed     = 1;
    (*pstPolicyDownloadProgress)->remainTime= 1;    
    (*errorCode) = 0;    

    //����ALL ECU Event ֪ͨ
    (void)adm_uim_allEcuDownloadProgressNotify(*eCmdidType, *sessionId, *state, (*pstEcuDownloadProgressArrayInfo),(*pstPolicyDownloadProgress));

    adm_free(*sessionId);
    adm_free(*state);
   
    for(D32S i = 0; i < 2;i++)
    {
        adm_free(((*pstEcuDownloadProgressArrayInfo)->ecuDownloadProgressArray)[i]->pstEcuIdenInfo->partNum);
        adm_free(((*pstEcuDownloadProgressArrayInfo)->ecuDownloadProgressArray)[i]->pstEcuIdenInfo->name);
        adm_free(((*pstEcuDownloadProgressArrayInfo)->ecuDownloadProgressArray)[i]->pstEcuIdenInfo->softVer);
    }
    adm_free((*pstEcuDownloadProgressArrayInfo)->ecuDownloadProgressArray);
    adm_free(*pstEcuDownloadProgressArrayInfo);

    return 0;
}



D32S UIMTEST_ADM_UIM_DOWNLOAD_QUERY_ALL_ECU_RESULT_CALLBACK_F(ADM_APPLICATION_COMMON_CMDID_TYPE_E* eCmdidType, 
                                                                  DCHAR** sessionId, 
                                                                  DCHAR** state, 
                                                                  ADM_UIM_SOA_ECU_DOWNLOAD_RESULT_ARRAY_INFO_T** pstEcuDownloadResultArrayInfo, 
                                                                  ADM_UIM_SOA_DOWNLOAD_RESULT_INFO_T** pstPolicyDownloadResult, 
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

    ADM_UIM_SOA_DOWNLOAD_RESULT_INFO_T* pstEcuDownloadResult1 = NULL;
    pstEcuDownloadResult1 = (ADM_UIM_SOA_DOWNLOAD_RESULT_INFO_T*)malloc(sizeof(ADM_UIM_SOA_DOWNLOAD_RESULT_INFO_T));
    pstEcuDownloadResult1->result =  0x0;
    pstEcuDownloadResult1->expectVal = (DCHAR*)"failed";
    pstEcuDownloadResult1->realVal = (DCHAR*)"success";

    //���쵥��ECU����¼�
    //adm_uim_soaEcuDownloadResultNotify(eCmdidType1,sessionId1,state1,pstEcuIdenInfo1,pstEcuDownloadResult1);
    free(pstEcuDownloadResult1);
    free(pstEcuIdenInfo1);
*/
    //����ALL ECU Result֪ͨ
    *eCmdidType = ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_USER;
    *sessionId  = adm_strdup("0x13");
    *state      = adm_strdup("queryResult Success");
    
    (*pstEcuDownloadResultArrayInfo) = (ADM_UIM_SOA_ECU_DOWNLOAD_RESULT_ARRAY_INFO_T*)adm_malloc(sizeof(ADM_UIM_SOA_ECU_DOWNLOAD_RESULT_ARRAY_INFO_T));
    (*pstEcuDownloadResultArrayInfo)->ecuDownloadResultArrayLength = 2;
    (*pstEcuDownloadResultArrayInfo)->ecuDownloadResultArray = (ADM_UIM_SOA_ECU_DOWNLOAD_RESULT_INFO_T**)adm_malloc(sizeof(ADM_UIM_SOA_ECU_DOWNLOAD_RESULT_INFO_T*));
 //   *((*pstEcuDownloadResultArrayInfo)->ecuDownloadResultArray) = (ADM_UIM_SOA_ECU_DOWNLOAD_RESULT_INFO_T*)malloc(sizeof(ADM_UIM_SOA_ECU_DOWNLOAD_RESULT_INFO_T)*2);
    for(D32S i = 0; i < 2;i++)
    {
    	((*pstEcuDownloadResultArrayInfo)->ecuDownloadResultArray)[i] = (ADM_UIM_SOA_ECU_DOWNLOAD_RESULT_INFO_T*)adm_malloc(sizeof(ADM_UIM_SOA_ECU_DOWNLOAD_RESULT_INFO_T));
        (((*pstEcuDownloadResultArrayInfo)->ecuDownloadResultArray))[i]->pstEcuIdenInfo = (ADM_UIM_SOA_ECU_IDEN_INFO_T*)adm_malloc(sizeof(ADM_UIM_SOA_ECU_IDEN_INFO_T));
        (((*pstEcuDownloadResultArrayInfo)->ecuDownloadResultArray))[i]->pstEcuDownloadResult = (ADM_UIM_SOA_DOWNLOAD_RESULT_INFO_T*)adm_malloc(sizeof(ADM_UIM_SOA_DOWNLOAD_RESULT_INFO_T));

        (((*pstEcuDownloadResultArrayInfo)->ecuDownloadResultArray))[i]->state=adm_strdup("success");
        (((*pstEcuDownloadResultArrayInfo)->ecuDownloadResultArray))[i]->pstEcuIdenInfo->partNum = adm_strdup("0x1234");
        (((*pstEcuDownloadResultArrayInfo)->ecuDownloadResultArray))[i]->pstEcuIdenInfo->requestId = 0x01;
        (((*pstEcuDownloadResultArrayInfo)->ecuDownloadResultArray))[i]->pstEcuIdenInfo->responseId = 0x02;
        (((*pstEcuDownloadResultArrayInfo)->ecuDownloadResultArray))[i]->pstEcuIdenInfo->swDid      = 0x03;
        (((*pstEcuDownloadResultArrayInfo)->ecuDownloadResultArray))[i]->pstEcuIdenInfo->name       = adm_strdup("test");
        (((*pstEcuDownloadResultArrayInfo)->ecuDownloadResultArray))[i]->pstEcuIdenInfo->softVer    = adm_strdup("Ver1.0");

        (((*pstEcuDownloadResultArrayInfo)->ecuDownloadResultArray))[i]->pstEcuDownloadResult->result     = 1;
        (((*pstEcuDownloadResultArrayInfo)->ecuDownloadResultArray))[i]->pstEcuDownloadResult->expectVal  = adm_strdup("0");
        (((*pstEcuDownloadResultArrayInfo)->ecuDownloadResultArray))[i]->pstEcuDownloadResult->realVal    = adm_strdup("1");
    }

    (*pstPolicyDownloadResult)=(ADM_UIM_SOA_DOWNLOAD_RESULT_INFO_T*)adm_malloc(sizeof(ADM_UIM_SOA_DOWNLOAD_RESULT_INFO_T));
    (*pstPolicyDownloadResult)->result = 1;
    (*pstPolicyDownloadResult)->expectVal = adm_strdup("0");
    (*pstPolicyDownloadResult)->realVal   = adm_strdup("1");
    (*errorCode) = 0;

    (void)adm_uim_allEcuDownloadResultNotify(*eCmdidType, *sessionId, *state, (*pstEcuDownloadResultArrayInfo),(*pstPolicyDownloadResult));

    for(D32S i = 0; i < 2;i++)
    {
        adm_free((((*pstEcuDownloadResultArrayInfo)->ecuDownloadResultArray))[i]->state);
        adm_free((((*pstEcuDownloadResultArrayInfo)->ecuDownloadResultArray))[i]->pstEcuIdenInfo->partNum);
        adm_free((((*pstEcuDownloadResultArrayInfo)->ecuDownloadResultArray))[i]->pstEcuIdenInfo->name);
        adm_free((((*pstEcuDownloadResultArrayInfo)->ecuDownloadResultArray))[i]->pstEcuIdenInfo->softVer);
        adm_free((((*pstEcuDownloadResultArrayInfo)->ecuDownloadResultArray))[i]->pstEcuDownloadResult->expectVal);
        adm_free((((*pstEcuDownloadResultArrayInfo)->ecuDownloadResultArray))[i]->pstEcuDownloadResult->realVal);
        adm_free((((*pstEcuDownloadResultArrayInfo)->ecuDownloadResultArray))[i]->pstEcuIdenInfo);
        adm_free((((*pstEcuDownloadResultArrayInfo)->ecuDownloadResultArray))[i]->pstEcuDownloadResult);
        adm_free(((*pstEcuDownloadResultArrayInfo)->ecuDownloadResultArray)[i]);
    }
    adm_free(*sessionId);
    adm_free(*state);
    adm_free((*pstPolicyDownloadResult)->expectVal);
    adm_free((*pstPolicyDownloadResult)->realVal);
    adm_free(*pstPolicyDownloadResult);

    return 0;
}



