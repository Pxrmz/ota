#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <unistd.h>
#include <time.h>

#include "adm_cdm_log.h"
#include "adm_soa_cfg.h"
#include "adm_string.h"
#include "adm_cdm_verify_interface.h"
#include "adm_dim_vdi.h"
#include "adm_dim_host.h"
#include "adm_cds_compare.h"
#include "adm_cds_parse.h"
#include "adm_memory.h"
#include "adm_thread.h"
#include "adm_time.h"
#include "adm_soa.h"

//线程传递参数结构体声明
typedef struct ADM_CDM_LOOP_PARAM
{
    ADM_COND_POLICY_T* pCondition;
    D32S               iCycle;
    ADM_CDM_NOTIFY_Fn  callback;
    D32S               iThreadState;
    adm_handle_t       threadHandle;
}ADM_CDM_LOOP_PARAM_T;


//允许的最大定时器
#define ADM_CDM_MAXTIMER     100
static  ADM_CDM_LOOP_PARAM_T* m_pTimerInfoList[ADM_CDM_MAXTIMER]={0};

//cdm定时器控制量
static D32S          m_iRunTimer = 0;

#if 0
static D32S adm_test_dim_soa_vdi_readVehicleStatus(const D16U instanceId, const D16U timeout, const ADM_DIM_SOA_VDI_MSGID_DATA_ARRAY_T* msgIdListRequest, ADM_DIM_SOA_VDI_MSGID_DATA_ARRAY_T** msgIdListResponse, D32S* errorCode)
{
    *errorCode = 0;
    
    if(NULL == msgIdListRequest)
    {
        *errorCode = -1;
        return -1;
    }
    D32S msgIdArrayLength = msgIdListRequest->msgIdArrayLength;
    
    *msgIdListResponse = adm_malloc(sizeof(ADM_DIM_SOA_VDI_MSGID_DATA_ARRAY_T*));
    memset(*msgIdListResponse,0,sizeof(ADM_DIM_SOA_VDI_MSGID_DATA_ARRAY_T*));
    (*msgIdListResponse)->msgIdArrayLength = msgIdArrayLength;
    (*msgIdListResponse)->msgIdArray = (ADM_DIM_SOA_VDI_MSGID_T**)adm_malloc(sizeof(ADM_DIM_SOA_VDI_MSGID_T*)*(DSIZE)msgIdArrayLength);
    memset((*msgIdListResponse)->msgIdArray, 0x0, sizeof(ADM_DIM_SOA_VDI_MSGID_T*)*(DSIZE)msgIdArrayLength);

    for(D32S i = 0;i < msgIdArrayLength;i++)
    {
        (*msgIdListResponse)->msgIdArray[i] = (ADM_DIM_SOA_VDI_MSGID_T*)adm_malloc((DSIZE)sizeof(ADM_DIM_SOA_VDI_MSGID_T));
        memset((*msgIdListResponse)->msgIdArray[i],0x00,sizeof(ADM_DIM_SOA_VDI_MSGID_T));
        (*msgIdListResponse)->msgIdArray[i]->msgIdData = (ADM_DIM_SOA_DATA_T*)adm_malloc((DSIZE)sizeof(ADM_DIM_SOA_DATA_T));
        memset((*msgIdListResponse)->msgIdArray[i]->msgIdData,0x00,sizeof(ADM_DIM_SOA_DATA_T));
        (*msgIdListResponse)->msgIdArray[i]->msgId= 2;
    	(*msgIdListResponse)->msgIdArray[i]->msgIdData->dataType = 7;
    	(*msgIdListResponse)->msgIdArray[i]->msgIdData->dataLength = 1;
    	switch((*msgIdListResponse)->msgIdArray[i]->msgIdData->dataType)
    	{
    	case 1:
            (*msgIdListResponse)->msgIdArray[i]->msgIdData->v.value[0] = '0';
    	    break;
    	case 2:
            (*msgIdListResponse)->msgIdArray[i]->msgIdData->v.valueU32 = 4;
    	    break;
    	case 3:
            (*msgIdListResponse)->msgIdArray[i]->msgIdData->v.valueS32 = 6;
    	    break;
        case 4:
            (*msgIdListResponse)->msgIdArray[i]->msgIdData->v.valueU64 = 123456;
            break;
        case 5:
            (*msgIdListResponse)->msgIdArray[i]->msgIdData->v.valueF = 123.00;
            break;
        case 6:
            (*msgIdListResponse)->msgIdArray[i]->msgIdData->v.valueS = "1";
            break;
        case 7:
            (*msgIdListResponse)->msgIdArray[i]->msgIdData->v.valueS64 = 654321;
        default:
            break;
    	}	       
    }
    return 0;
}
#endif // 0

static int adm_cdm_ChangeHexString2DecNum(DCHAR* str,D32S length)     
{
    if (NULL == str || length <= 0)
    {
        return -1;
    }
    char  revstr[256]={0};
    int   num[256]={0};     
    int   count=1;     
    int   result=0;     
    (void)adm_strcpy(revstr, str, sizeof(revstr));
    for(int i=length-1;i>=0;i--)     
    {     
        if ((revstr[i] >= '0') && (revstr[i] <= '9'))
        { 
            num[i] = (int)revstr[i] - 0x30;//字符0的ASCII值为48
        }
        else if ((revstr[i] >= 'a') && (revstr[i] <= 'f'))
        {
            num[i] = (int)(revstr[i] - 'a') + 10;
        }
        else if ((revstr[i] >= 'A') && (revstr[i] <= 'F'))
        {
            num[i] = (int)(revstr[i] - 'A') + 10;
        }
        else
        {
            num[i] = 0;
        }
        result = result + num[i] * count;     
        count = count * 16;   
    }
    return result;     
}

static void cdmDimSoaData(ADM_DIM_SOA_DATA_T* pData)
{
    if (NULL != pData)
    {
        if (VAR_STRING == (VARIABLE_TYPE_EN)pData->dataType)
        {
            adm_free(pData->v.valueS);
        }

        adm_free(pData);
    }
}

static void cdmDimSoaVdiMsgIdDeinit(ADM_DIM_SOA_VDI_MSGID_T* pMsgId)
{
    if (NULL != pMsgId)
    {
        if (NULL != pMsgId->msgIdData)
        {
            cdmDimSoaData(pMsgId->msgIdData);
        }

        adm_free(pMsgId);
    }
}

static void cdmDimSoaVdiMsgDataArrayDeinit(ADM_DIM_SOA_VDI_MSGID_DATA_ARRAY_T* pDataArray)
{
    if (NULL != pDataArray)
    {
        for (D32U i = 0; i < (D32U)pDataArray->msgIdArrayLength; i++)
        {
            cdmDimSoaVdiMsgIdDeinit(pDataArray->msgIdArray[i]);
        }

        adm_free(pDataArray->msgIdArray);
        adm_free(pDataArray);
    }
}

static void cdsVarDeinit(cds_variable_pt pVar)
{
    if (NULL != pVar)
    {
        if (VAR_STRING == (VARIABLE_TYPE_EN)pVar->data_type)
        {
            adm_free(pVar->v.value_s);
        }

        adm_free(pVar);
    }
}

static void cdmCompareStDeinit(cds_compare_st* pSt)
{
    cdsVarDeinit(pSt->l_variable);
    cdsVarDeinit(pSt->r_variable);
}

static void cdmCompareStsDeinit(cds_compare_st* pSts, D32S iStCount)
{
    if (NULL != pSts)
    {
        for (D32S i = 0; i < iStCount; i++)
        {
            cdmCompareStDeinit(&pSts[i]);
        }

        adm_free(pSts);
    }
}


/**
 * @brief  获取比较左右值结构体
 * @param  源左值
 * @param  源右值
 * @param  目的左右值结合结构体
 * @param  结构体个数
 * @retval int o success
 */
static D32S adm_cdm_Handle(const ADM_COND_POLICY_T* pCondReq,ADM_UIM_SOA_CONDITION_DATA_ARRAY_T** pStCondResults)
{
    if(NULL == pCondReq)
    {
        log_i("adm_cdm_Handle pCondReq=NULL\r\n");
        return -1;
    }
    if(pCondReq->iCondNum < 0)
    {
        log_i("adm_cdm_Handle iCondNum <= 0,return\r\n");
        return -1;
    }
    //from here----->line 182 just logging the conditionsReq
    // log_i("pCondReq->iType=%d\r\n",pCondReq->iType);
    // log_i("pCondReq->iCondNum=%d\r\n",pCondReq->iCondNum);

    if (0 == pCondReq->iCondNum)
    {
        return 0;
    }

#if 0
    for(D32S i = 0; i < pCondReq->iCondNum;i++)
    {
        log_i("pCondReq->pCondtionS[%d].serviceType=%s\r\n", i, pCondReq->pCondtionS[i].serviceType);
        log_i("pCondReq->pCondtionS[%d].messageId=%s\r\n",i,pCondReq->pCondtionS[i].messageId);
        log_i("pCondReq->pCondtionS[%d].messageName=%s\r\n",i,pCondReq->pCondtionS[i].messageName);
        log_i("pCondReq->pCondtionS[%d].messageDesc=%s\r\n",i,pCondReq->pCondtionS[i].messageDesc);
        log_i("pCondReq->pCondtionS[%d].iExpressNum=%d\r\n",i,pCondReq->pCondtionS[i].iExpressNum);
        if(pCondReq->pCondtionS[i].expressions != NULL)
        {
            for(D32S j = 0; j < pCondReq->pCondtionS[i].iExpressNum;j++)
            {
                log_i("pCondReq->pCondtionS[%d].expressions[%d].iLogic=%d\r\n",i,j,pCondReq->pCondtionS[i].expressions[j].iLogic);
                log_i("pCondReq->pCondtionS[%d].expressions[%d].sData.data_type=%d\r\n",i,j,pCondReq->pCondtionS[i].expressions[j].sData.data_type);
                log_i("pCondReq->pCondtionS[%d].expressions[%d].sData.data_length=%d\r\n",i,j,pCondReq->pCondtionS[i].expressions[j].sData.data_length);
                switch((VARIABLE_TYPE_EN)pCondReq->pCondtionS[i].expressions[j].sData.data_type)
                {
                case VAR_U8:
                    log_i("pCondReq->pCondtionS[%d].expressions[%d].sData.v.value[0]=%d\r\n",i,j,pCondReq->pCondtionS[i].expressions[j].sData.v.value[0]);
                    break;
                case VAR_U32:
                    log_i("pCondReq->pCondtionS[%d].expressions[%d].sData.v.value_u32=%d\r\n",i,j,pCondReq->pCondtionS[i].expressions[j].sData.v.value_u32);
                    break;
                case VAR_S32:
                    log_i("pCondReq->pCondtionS[%d].expressions[%d].sData.v.value_s32=%d\r\n",i,j,pCondReq->pCondtionS[i].expressions[j].sData.v.value_s32);
                    break;
                case VAR_U64:
                    log_i("pCondReq->pCondtionS[%d].expressions[%d].sData.v.value_u64=%lld\r\n",i,j,pCondReq->pCondtionS[i].expressions[j].sData.v.value_u64);
                    break;
                case VAR_FLOAT:
                    log_i("pCondReq->pCondtionS[%d].expressions[%d].sData.v.value_f=%f\r\n",i,j,pCondReq->pCondtionS[i].expressions[j].sData.v.value_f);
                    break;
                case VAR_STRING:
                    log_i("pCondReq->pCondtionS[%d].expressions[%d].sData.v.value_s=%s\r\n",i,j,pCondReq->pCondtionS[i].expressions[j].sData.v.value_s);
                    break;
                case VAR_S64:
                    log_i("pCondReq->pCondtionS[%d].expressions[%d].sData.v.value_s64=%lld\r\n", i, j, pCondReq->pCondtionS[i].expressions[j].sData.v.value_s64);
                    break;
                default:
                    log_i("pCondReq->pCondtionS[%d].expressions[%d].sData.v,ca't distinguish type\r\n",i,j);
                    break;
                }
                log_i("pCondReq->pCondtionS[%d].expressions[%d].connectRelation=%d\r\n",i,j,pCondReq->pCondtionS[i].expressions[j].connectRelation);
            }
        }
        log_i("pCondReq->pCondtionS[%d].connectRelation=%d\r\n",i,pCondReq->pCondtionS[i].connectRelation);
        if(pCondReq->pCondtionS[i].onFailStrategy != NULL)
        {
            log_i("pCondReq->pCondtionS[%d].onFailStrategy.type=%d\r\n",i,pCondReq->pCondtionS[i].onFailStrategy->type);
            log_i("pCondReq->pCondtionS[%d].onFailStrategy.timeout=%d\r\n",i,pCondReq->pCondtionS[i].onFailStrategy->timeout);
            log_i("pCondReq->pCondtionS[%d].onFailStrategy.action=%d\r\n",i,pCondReq->pCondtionS[i].onFailStrategy->action);
            log_i("pCondReq->pCondtionS[%d].onFailStrategy.iExpressNum=%d\r\n",i,pCondReq->pCondtionS[i].onFailStrategy->iExpressNum);
            for(D32S j = 0; j < pCondReq->pCondtionS[i].onFailStrategy->iExpressNum;j++)
            {
                log_i("pCondReq->pCondtionS[%d].onFailStrategy.expressions[%d].iLogic=%d\r\n",i,j,pCondReq->pCondtionS[i].onFailStrategy->expressions[j].iLogic);
                log_i("pCondReq->pCondtionS[%d].onFailStrategy.expressions[%d].sData.data_type=%d\r\n",i,j,pCondReq->pCondtionS[i].onFailStrategy->expressions[j].sData.data_type);
                log_i("pCondReq->pCondtionS[%d].onFailStrategy.expressions[%d].sData.data_length=%d\r\n",i,j,pCondReq->pCondtionS[i].onFailStrategy->expressions[j].sData.data_length);
                switch((VARIABLE_TYPE_EN)pCondReq->pCondtionS[i].onFailStrategy->expressions[j].sData.data_type)
                {
                case VAR_U8:
                    log_i("pCondReq->pCondtionS[%d].onFailStrategy.expressions[%d].sData.v.value=%d\r\n",i,j,pCondReq->pCondtionS[i].onFailStrategy->expressions[j].sData.v.value[0]);
                    break;
                case VAR_U32:
                    log_i("pCondReq->pCondtionS[%d].onFailStrategy.expressions[%d].sData.v.value_u32=%d\r\n",i,j,pCondReq->pCondtionS[i].onFailStrategy->expressions[j].sData.v.value_u32);
                    break;
                case VAR_S32:
                    log_i("pCondReq->pCondtionS[%d].onFailStrategy.expressions[%d].sData.v.value_s32=%d\r\n",i,j,pCondReq->pCondtionS[i].onFailStrategy->expressions[j].sData.v.value_s32);
                    break;
                case VAR_U64:
                    log_i("pCondReq->pCondtionS[%d].onFailStrategy.expressions[%d].sData.v.value_u64=%lld\r\n",i,j,pCondReq->pCondtionS[i].onFailStrategy->expressions[j].sData.v.value_u64);
                    break;
                case VAR_FLOAT:
                    log_i("pCondReq->pCondtionS[%d].onFailStrategy.expressions[%d].sData.v.value_f=%f\r\n",i,j,pCondReq->pCondtionS[i].onFailStrategy->expressions[j].sData.v.value_f);
                    break;
                case VAR_STRING:
                    log_i("pCondReq->pCondtionS[%d].onFailStrategy.expressions[%d].sData.v.value_s=%s\r\n",i,j,pCondReq->pCondtionS[i].onFailStrategy->expressions[j].sData.v.value_s);
                    break;
                case VAR_S64:
                    log_i("pCondReq->pCondtionS[%d].onFailStrategy.expressions[%d].sData.v.value_s64=%lld\r\n", i, j, pCondReq->pCondtionS[i].onFailStrategy->expressions[j].sData.v.value_s64);
                    break;
                default:
                    log_i("pCondReq->pCondtionS[%d].onFailStrategy.expressions[%d].sData.v ,can't distinguish type\r\n",i,j);
                    break;
                }
            }
        }
    }
#endif ///< 删除请求信息的日志打印
    
    D32S iCondNum = pCondReq->iCondNum;
    
    //set call adm_dim_soa_vdi_readVehicleStatus function argv
    ADM_DIM_SOA_VDI_MSGID_DATA_ARRAY_T* requestInfo = NULL;
    requestInfo = (ADM_DIM_SOA_VDI_MSGID_DATA_ARRAY_T*)adm_malloc((DSIZE)sizeof(ADM_DIM_SOA_VDI_MSGID_DATA_ARRAY_T));
    if (NULL == requestInfo)
    {
        return -1;
    }
    (void)adm_memset(requestInfo,0,sizeof(ADM_DIM_SOA_VDI_MSGID_DATA_ARRAY_T),sizeof(ADM_DIM_SOA_VDI_MSGID_DATA_ARRAY_T));

    ADM_DIM_SOA_VDI_MSGID_T** pMsgIdArray = (ADM_DIM_SOA_VDI_MSGID_T**)adm_malloc(sizeof(ADM_DIM_SOA_VDI_MSGID_T*)* (DSIZE)iCondNum);
    (void)adm_memset(pMsgIdArray,0,sizeof(ADM_DIM_SOA_VDI_MSGID_T*)*(DSIZE)iCondNum,sizeof(ADM_DIM_SOA_VDI_MSGID_T*)* (DSIZE)iCondNum);

    requestInfo->msgIdArray = pMsgIdArray;
    requestInfo->msgIdArrayLength = (D16U)iCondNum;
    
    for(D32U i=0;i < (D32U)requestInfo->msgIdArrayLength;i++)
    {
        ADM_DIM_SOA_DATA_T* pSoaData = NULL;
        EXPRESSION_T* pExpression = NULL;
        ADM_DIM_SOA_VDI_MSGID_T* pMsgArray = (ADM_DIM_SOA_VDI_MSGID_T*)adm_malloc((DSIZE)sizeof(ADM_DIM_SOA_VDI_MSGID_T));
        if (NULL == pMsgArray)
        {
            cdmDimSoaVdiMsgDataArrayDeinit(requestInfo);
            return -1;
        }
        (void)adm_memset(pMsgArray,0,sizeof(ADM_DIM_SOA_VDI_MSGID_T),sizeof(ADM_DIM_SOA_VDI_MSGID_T));
        pMsgIdArray[i] = pMsgArray;

        D32S iMsgID = adm_cdm_ChangeHexString2DecNum(pCondReq->pCondtionS[i].messageId, (D32S)adm_strlen(pCondReq->pCondtionS[i].messageId));
        pMsgIdArray[i]->msgId = iMsgID;
        pSoaData = adm_malloc((DSIZE)sizeof(*pSoaData));
        if (NULL == pSoaData)
        {
            cdmDimSoaVdiMsgDataArrayDeinit(requestInfo);
            return -1;
        }
        pMsgIdArray[i]->msgIdData = pSoaData;

        pExpression = pCondReq->pCondtionS[i].expressions;
        pSoaData->dataType = pExpression->sData.data_type;
        pSoaData->dataLength = pExpression->sData.data_length;
        if (VAR_STRING != (VARIABLE_TYPE_EN)pSoaData->dataType)
        {
            (void)(void)adm_memcpy(&pSoaData->v, &pExpression->sData.v, sizeof(pSoaData->v), sizeof(pExpression->sData.v));
        }
        else
        {
            pSoaData->v.valueS = (D8U*)adm_strdup((DCHAR*)pExpression->sData.v.value_s);
        }

        // log_i("[adm_dim_soa_vdi_readVehicleStatus] requestInfo->pMsgIdArray[%u]->msgId=%d\r\n",i,iMsgID);
    }
    ADM_DIM_SOA_VDI_MSGID_DATA_ARRAY_T* responseInfo=NULL;
    // D32S iVehicleStatus=0;
    //call dim interface adm_dim_soa_vdi_readVehicleStatus to get realVal
    D32S iret = 0;
    log_i("before call [adm_dim_soa_vdi_readVehicleStatus]\r\n");
	D32S errorCode = -1;
	if (pCondReq->pCondtionS != NULL)
	{
		D32S iServiceType = adm_cdm_ChangeHexString2DecNum(pCondReq->pCondtionS->serviceType, (D32S)adm_strlen(pCondReq->pCondtionS->serviceType));
		log_i("serviceType=%d", iServiceType);
        // log_i("adm_soa_getVdiInstanceId()=%d", adm_soa_getVdiInstanceId());
        // log_i("adm_soa_getHostInstanceId()=%d", adm_soa_getHostInstanceId());
        //adm_test_dim_soa_vdi_readVehicleStatus(iServiceType, 5, requestInfo, &responseInfo, &errorCode);
        
		if (adm_soa_getVdiInstanceId() == (D16U)iServiceType)
		{
			iret = adm_dim_vdi_readVehicleStatus(adm_soa_getVdiInstanceId(), 5, requestInfo, &responseInfo, &errorCode);
		}
		else if (adm_soa_getHostInstanceId() == (D16U)iServiceType)
		{
            iret = adm_dim_host_readVehicleStatus(adm_soa_getHostInstanceId(), 5, (ADM_DIM_SOA_MSGID_DATA_ARRAY_T*)((void*)requestInfo), (ADM_DIM_SOA_MSGID_DATA_ARRAY_T**)((void*)&responseInfo), &errorCode);
		}
		else
		{
            cdmDimSoaVdiMsgDataArrayDeinit(requestInfo);
			return -1;
		}
	}
    cdmDimSoaVdiMsgDataArrayDeinit(requestInfo);
    log_i("after call [adm_dim_soa_vdi_readVehicleStatus]\r\n");

    if(iret != 0)
    {
        log_i("adm_dim_soa_vdi_readVehicleStatus::failed return=%d,errorcode=%d\r\n",iret, errorCode);
        return -1;
    }
    if(NULL == responseInfo)
    {
        log_i("adm_dim_soa_vdi_readVehicleStatus::responseInfo=NULL,iret = %d,errorcode=%d\r\n",iret, errorCode);
        return -1;
    }
    log_i("adm_dim_soa_vdi_readVehicleStatus::responseInfo->msgIdArrayLength=%d\r\n",responseInfo->msgIdArrayLength);
    if((D32U)responseInfo->msgIdArrayLength <= 0U)
    {
        log_i("adm_dim_soa_vdi_readVehicleStatus::responseInfo=NULL,iret = %d,msgIdArrayLength=%d\r\n",\
               iret,responseInfo->msgIdArrayLength);
        return -1;
    }
    ADM_DIM_SOA_VDI_MSGID_DATA_ARRAY_T* responseMsgArray = NULL;
    responseMsgArray = (ADM_DIM_SOA_VDI_MSGID_DATA_ARRAY_T*)adm_malloc((DSIZE)sizeof(ADM_DIM_SOA_VDI_MSGID_DATA_ARRAY_T));
    (void)adm_memset(responseMsgArray,0, sizeof(ADM_DIM_SOA_VDI_MSGID_DATA_ARRAY_T), sizeof(ADM_DIM_SOA_VDI_MSGID_DATA_ARRAY_T));
    responseMsgArray->msgIdArrayLength = (D16U)pCondReq->iCondNum;
    responseMsgArray->msgIdArray = (ADM_DIM_SOA_VDI_MSGID_T**)adm_malloc(sizeof(ADM_DIM_SOA_VDI_MSGID_T*) * (DSIZE)responseMsgArray->msgIdArrayLength);
    (void)adm_memset(responseMsgArray->msgIdArray, 0, sizeof(ADM_DIM_SOA_VDI_MSGID_T*) * responseMsgArray->msgIdArrayLength, 
               sizeof(ADM_DIM_SOA_VDI_MSGID_T*) * responseMsgArray->msgIdArrayLength);
    //will use responseMsgArray to save responseinfo
    for(D32S i = 0; i < pCondReq->iCondNum;i++)
    {
        responseMsgArray->msgIdArray[i] = (ADM_DIM_SOA_VDI_MSGID_T*)adm_malloc((DSIZE)sizeof(ADM_DIM_SOA_VDI_MSGID_T));
        (void)adm_memset(responseMsgArray->msgIdArray[i], 0, sizeof(ADM_DIM_SOA_VDI_MSGID_T), sizeof(ADM_DIM_SOA_VDI_MSGID_T));

        D32S iMessageID = adm_cdm_ChangeHexString2DecNum(pCondReq->pCondtionS[i].messageId, (D32S)adm_strlen(pCondReq->pCondtionS[i].messageId));
        responseMsgArray->msgIdArray[i]->msgId = iMessageID;
        responseMsgArray->msgIdArray[i]->msgIdData = (ADM_DIM_SOA_DATA_T*)adm_malloc((DSIZE)sizeof(ADM_DIM_SOA_DATA_T));
        (void)adm_memset(responseMsgArray->msgIdArray[i]->msgIdData,0,sizeof(ADM_DIM_SOA_DATA_T),sizeof(ADM_DIM_SOA_DATA_T));
    }
    for(D32S iRespMsgIndex = 0;iRespMsgIndex<pCondReq->iCondNum;iRespMsgIndex++)
    {
        for(D32U i = 0; i < (D32U)responseInfo->msgIdArrayLength;i++)
        {
            if(responseInfo->msgIdArray[i]->msgId == responseMsgArray->msgIdArray[iRespMsgIndex]->msgId)
            {
                if(responseInfo->msgIdArray[i]->msgIdData != NULL)
                {
                    responseMsgArray->msgIdArray[iRespMsgIndex]->msgIdData->dataType = responseInfo->msgIdArray[i]->msgIdData->dataType;
                    responseMsgArray->msgIdArray[iRespMsgIndex]->msgIdData->dataLength = responseInfo->msgIdArray[i]->msgIdData->dataLength;
                    switch((VARIABLE_TYPE_EN)responseInfo->msgIdArray[i]->msgIdData->dataType)
                    {
                    case VAR_U8:
                        responseMsgArray->msgIdArray[iRespMsgIndex]->msgIdData->v.value[0] = responseInfo->msgIdArray[i]->msgIdData->v.value[0];
                        break;
                    case VAR_U32:
                        responseMsgArray->msgIdArray[iRespMsgIndex]->msgIdData->v.valueU32 = responseInfo->msgIdArray[i]->msgIdData->v.valueU32;
                        break;
                    case VAR_S32:
                        responseMsgArray->msgIdArray[iRespMsgIndex]->msgIdData->v.valueS32 = responseInfo->msgIdArray[i]->msgIdData->v.valueS32;
                        break;
                    case VAR_U64:
                        responseMsgArray->msgIdArray[iRespMsgIndex]->msgIdData->v.valueU64 = responseInfo->msgIdArray[i]->msgIdData->v.valueU64;
                        break;
                    case VAR_FLOAT:
                        responseMsgArray->msgIdArray[iRespMsgIndex]->msgIdData->v.valueF = responseInfo->msgIdArray[i]->msgIdData->v.valueF;
                        break;
                    case VAR_STRING:
                        responseMsgArray->msgIdArray[iRespMsgIndex]->msgIdData->v.valueS = NULL;
                        if(NULL == responseInfo->msgIdArray[i]->msgIdData->v.valueS)
                        {
                            break;
                        }
                        responseMsgArray->msgIdArray[iRespMsgIndex]->msgIdData->v.valueS = 
                            (D8U*)adm_strdup((DCHAR*)responseInfo->msgIdArray[i]->msgIdData->v.valueS);
                        break;
                    case VAR_S64:
                        responseMsgArray->msgIdArray[iRespMsgIndex]->msgIdData->v.valueS64 = responseInfo->msgIdArray[i]->msgIdData->v.valueS64;
                        break;
                    default:
                        break;
                    }
                }
            }
        }
    }
#if 0
    //from here logging responseMsg
    for(D32S i = 0; i < (D32S)responseInfo->msgIdArrayLength;i++)
    {
        log_i("adm_dim_soa_vdi_readVehicleStatus::responseInfo->msgIdArray[%d].msgId=%d\r\n",i,responseInfo->msgIdArray[i]->msgId);
        if(responseInfo->msgIdArray[i]->msgIdData != NULL)
        {
            log_i("adm_dim_soa_vdi_readVehicleStatus::responseInfo->msgIdArray[%d].msgIdData->dataType=%d\r\n",i,responseInfo->msgIdArray[i]->msgIdData->dataType);
            log_i("adm_dim_soa_vdi_readVehicleStatus::responseInfo->msgIdArray[%d].msgIdData->dataLength=%d\r\n",i,responseInfo->msgIdArray[i]->msgIdData->dataLength);
            switch((VARIABLE_TYPE_EN)responseInfo->msgIdArray[i]->msgIdData->dataType)
            {
            case VAR_U8:
                log_i("adm_dim_soa_vdi_readVehicleStatus::responseInfo->msgIdArray[%d].msgIdData->v.value[0]=%d\r\n",i,responseInfo->msgIdArray[i]->msgIdData->v.value[0]);
                break;
            case VAR_U32:
                log_i("adm_dim_soa_vdi_readVehicleStatus::responseInfo->msgIdArray[%d].msgIdData->v.valueU32=%d\r\n",i,responseInfo->msgIdArray[i]->msgIdData->v.valueU32);
                break;
            case VAR_S32:
                log_i("adm_dim_soa_vdi_readVehicleStatus::responseInfo->msgIdArray[%d].msgIdData->v.valueS32=%d\r\n",i,responseInfo->msgIdArray[i]->msgIdData->v.valueS32);
                break;
            case VAR_U64:
                log_i("adm_dim_soa_vdi_readVehicleStatus::responseInfo->msgIdArray[%d].msgIdData->v.valueU64=%lld\r\n",i,responseInfo->msgIdArray[i]->msgIdData->v.valueU64);
                break;
            case VAR_FLOAT:
                log_i("adm_dim_soa_vdi_readVehicleStatus::responseInfo->msgIdArray[%d].msgIdData->v.valueF=%f\r\n",i,responseInfo->msgIdArray[i]->msgIdData->v.valueF);
                break;
            case VAR_STRING:
                log_i("adm_dim_soa_vdi_readVehicleStatus::responseInfo->msgIdArray[%d].msgIdData->v.valueS=%s\r\n",i,responseInfo->msgIdArray[i]->msgIdData->v.valueS);
                break;
            case VAR_S64:
                log_i("adm_dim_soa_vdi_readVehicleStatus::responseInfo->msgIdArray[%d].msgIdData->v.valueS64=%lld\r\n", i, responseInfo->msgIdArray[i]->msgIdData->v.valueS64);
                break;
            default:
                log_i("adm_dim_soa_vdi_readVehicleStatus::responseInfo->msgIdArray[%d].msgIdData,can't distinguish type\r\n",i);
                break;
            }
        }
        else
        {
            log_i("adm_dim_soa_vdi_readVehicleStatus::responseInfo->msgIdArray[%d].msgIdData=NULL\r\n",i);
        }
    }
#endif      ///< 屏蔽响应日志打印
    cdmDimSoaVdiMsgDataArrayDeinit(responseInfo);

    D8U compare_result = 0x00U;
    // D32S iFindMsgId = 0;

    //构造返回至UIM模块使用的结构体
    *pStCondResults = (ADM_UIM_SOA_CONDITION_DATA_ARRAY_T*)adm_malloc((DSIZE)sizeof(ADM_UIM_SOA_CONDITION_DATA_ARRAY_T));
    if (NULL == *pStCondResults)
    {
        cdmDimSoaVdiMsgDataArrayDeinit(responseMsgArray);
        return -1;
    }
    (void)adm_memset(*pStCondResults,0,sizeof(ADM_UIM_SOA_CONDITION_DATA_ARRAY_T),sizeof(ADM_UIM_SOA_CONDITION_DATA_ARRAY_T));
    ADM_UIM_SOA_CONDITION_DATA_ARRAY_T* pResult2UIM = *pStCondResults;
    pResult2UIM->condsArrayLength = (D16U)iCondNum;

    pResult2UIM->condsArray = (ADM_UIM_SOA_CONDITION_DATA_T**)adm_malloc((DSIZE)sizeof(ADM_UIM_SOA_CONDITION_DATA_T*)* (DSIZE)iCondNum);
    (void)adm_memset(pResult2UIM->condsArray,0,sizeof(ADM_UIM_SOA_CONDITION_DATA_T*)* (DSIZE)iCondNum,sizeof(ADM_UIM_SOA_CONDITION_DATA_T*)* (DSIZE)iCondNum);

    cds_variable_pt l_variable = NULL;
    cds_variable_pt r_variable = NULL;
    for(D32S i = 0; i < iCondNum;i++)
    {
        if(NULL == pCondReq->pCondtionS[i].expressions)
        {
            continue;
        }
        D32S icompNum = pCondReq->pCondtionS[i].iExpressNum;

        cds_compare_st* pCompareSt = NULL;
        pCompareSt = (cds_compare_st*)adm_malloc((DSIZE)sizeof(cds_compare_st) * (DSIZE)icompNum);
        if (NULL == pCompareSt)
        {
            cdmDimSoaVdiMsgDataArrayDeinit(responseMsgArray);
            return -1;
        }
        (void)adm_memset(pCompareSt,0,sizeof(cds_compare_st) * (DSIZE)icompNum, sizeof(cds_compare_st) * (DSIZE)icompNum);

        for(D32S iExpressIndex = 0; iExpressIndex < icompNum;iExpressIndex++)
        {
            pCompareSt[iExpressIndex].type = (enum COMPARE_SYMBOL)pCondReq->pCondtionS[i].expressions[iExpressIndex].iLogic;
            pCompareSt[iExpressIndex].connect_symbol = (COMPARE_CONNECT_SYMBOL_EN)pCondReq->pCondtionS[i].expressions[iExpressIndex].connectRelation;
            r_variable = adm_malloc((DSIZE)sizeof(cds_variable_st));
            if (NULL == r_variable)
            {
                cdmCompareStsDeinit(pCompareSt, icompNum);
                cdmDimSoaVdiMsgDataArrayDeinit(responseMsgArray);
                return -1;
            }
            (void)adm_memset(r_variable,0,sizeof(cds_variable_st),sizeof(cds_variable_st));
            r_variable->data_type = pCondReq->pCondtionS[i].expressions[iExpressIndex].sData.data_type;
            r_variable->data_length = pCondReq->pCondtionS[i].expressions[iExpressIndex].sData.data_length;
            
            switch((VARIABLE_TYPE_EN)r_variable->data_type)
            {
            case VAR_U8:
                r_variable->v.value[0] = pCondReq->pCondtionS[i].expressions[iExpressIndex].sData.v.value[0];
                break;
            case VAR_U32:
                r_variable->v.value_u32 = pCondReq->pCondtionS[i].expressions[iExpressIndex].sData.v.value_u32;
                break;
            case VAR_S32:
                r_variable->v.value_s32 = pCondReq->pCondtionS[i].expressions[iExpressIndex].sData.v.value_s32;
                break;
            case VAR_U64:
                r_variable->v.value_u64 = pCondReq->pCondtionS[i].expressions[iExpressIndex].sData.v.value_u64;
                break;
            case VAR_FLOAT:
                r_variable->v.value_f = pCondReq->pCondtionS[i].expressions[iExpressIndex].sData.v.value_f;
                break;
            case VAR_STRING:
                r_variable->v.value_s =
					(D8U*)adm_strdup((DCHAR*)pCondReq->pCondtionS[i].expressions[iExpressIndex].sData.v.value_s);
                break;
            case VAR_S64:
                r_variable->v.value_s64 = pCondReq->pCondtionS[i].expressions[iExpressIndex].sData.v.value_s64;
                break;
            default:
                break;
            }

            // 左值输入vdi返回的真实值
            l_variable = adm_malloc((DSIZE)sizeof(cds_variable_st));
            if (NULL == l_variable)
            {
                cdmCompareStsDeinit(pCompareSt, icompNum);
                cdmDimSoaVdiMsgDataArrayDeinit(responseMsgArray);
                return -1;
            }
            (void)adm_memset(l_variable,0,sizeof(cds_variable_st),sizeof(cds_variable_st));
            l_variable->data_type =responseMsgArray->msgIdArray[i]->msgIdData->dataType;
            l_variable->data_length = responseMsgArray->msgIdArray[i]->msgIdData->dataLength;
            switch((VARIABLE_TYPE_EN)l_variable->data_type)
            {
            case VAR_U8:
                l_variable->v.value[0] = responseMsgArray->msgIdArray[i]->msgIdData->v.value[0];
                break;
            case VAR_U32:
                l_variable->v.value_u32 = responseMsgArray->msgIdArray[i]->msgIdData->v.valueU32;
                break;
            case VAR_S32:
                l_variable->v.value_s32 = responseMsgArray->msgIdArray[i]->msgIdData->v.valueS32;
                break;
            case VAR_U64:
                l_variable->v.value_u64 = responseMsgArray->msgIdArray[i]->msgIdData->v.valueU64;
                break;
            case VAR_FLOAT:
                l_variable->v.value_f = responseMsgArray->msgIdArray[i]->msgIdData->v.valueF;
                break;
            case VAR_STRING:
                if(responseMsgArray->msgIdArray[i]->msgIdData->v.valueS != NULL)
                {
                    l_variable->v.value_s =
                        (D8U*)adm_strdup((DCHAR*)responseMsgArray->msgIdArray[i]->msgIdData->v.valueS);                    
                }
                break;
            case VAR_S64:
                l_variable->v.value_s64 = responseMsgArray->msgIdArray[i]->msgIdData->v.valueS64;
                break;
            default:
                break; 
            }

            pCompareSt[iExpressIndex].r_variable = r_variable;
            pCompareSt[iExpressIndex].l_variable = l_variable;
        }

        D8U  uResult = (D8U)adm_cds_compare(pCompareSt,icompNum);
        cdmCompareStsDeinit(pCompareSt, icompNum);
        if(0x00U == uResult)
        {
            // verify failed ,use failureExpress Val execute verify
            log_i("pCondReq->pCondtionS[i] [adm_cds_compare] return 0x01,check onFailStrategy\r\n",i);
            if(pCondReq->pCondtionS[i].onFailStrategy != NULL)
            {
                D32S ierror = 0;
                switch(pCondReq->pCondtionS[i].onFailStrategy->action)
                {
                case 1:
                    iret = adm_dim_vdi_controlVehicleStatus(adm_soa_getVdiInstanceId(),5,VEHICLE_CTRL_START_SMART_CHARGING, NULL,5,&ierror);
                    break;
                case 2:
                    iret = adm_dim_vdi_controlVehicleStatus(adm_soa_getVdiInstanceId(),5,VEHICLE_CTRL_ENTER_HV_MODE,NULL,5,&ierror);
                    break;
                case 3:
                    iret = adm_dim_vdi_controlVehicleStatus(adm_soa_getVdiInstanceId(),5,VEHICLE_CTRL_EXIT_HV_MODE, NULL,5,&ierror);
                    break;
                default:
                    log_i("can't distinguish pCondReq->pCondtionS[i].onFailStrategy->action\r\n",i);
                    break;
                }
                log_i("[adm_dim_soa_vdi_controlVehicleStatus] return=%d,ierror=&d\r\n",iret,ierror);
                cds_compare_st* pCompareStFail = NULL;
                D32S iFailExpressNum = pCondReq->pCondtionS[i].onFailStrategy->iExpressNum;
                pCompareStFail = adm_malloc((DSIZE)sizeof(cds_compare_st)* (DSIZE)iFailExpressNum);
                if (NULL == pCompareStFail)
                {
                    cdmDimSoaVdiMsgDataArrayDeinit(responseMsgArray);
                    return -1;
                }
                (void)adm_memset(pCompareStFail,0,sizeof(cds_compare_st)* (DSIZE)iFailExpressNum,sizeof(cds_compare_st)* (DSIZE)iFailExpressNum);
                
                for(D32S iFailExpressN=0;iFailExpressN<iFailExpressNum;iFailExpressN++)
                {
                    pCompareStFail[iFailExpressN].type = (enum COMPARE_SYMBOL)pCondReq->pCondtionS[i].onFailStrategy->expressions[iFailExpressN].iLogic;
                    pCompareStFail[iFailExpressN].connect_symbol = (COMPARE_CONNECT_SYMBOL_EN)pCondReq->pCondtionS[i].onFailStrategy->expressions[iFailExpressN].connectRelation;
                    r_variable = adm_malloc((DSIZE)sizeof(cds_variable_st));
                    if (NULL == r_variable)
                    {
                        cdmCompareStsDeinit(pCompareStFail, iFailExpressNum);
                        cdmDimSoaVdiMsgDataArrayDeinit(responseMsgArray);
                        return -1;
                    }
                    (void)adm_memset(r_variable,0,sizeof(cds_variable_st),sizeof(cds_variable_st));
                    
                    r_variable->data_type = pCondReq->pCondtionS[i].onFailStrategy->expressions[iFailExpressN].sData.data_type;
                    r_variable->data_length = pCondReq->pCondtionS[i].onFailStrategy->expressions[iFailExpressN].sData.data_length;
                    switch((VARIABLE_TYPE_EN)r_variable->data_type)
                    {
                    case VAR_U8:
                        r_variable->v.value[0] = pCondReq->pCondtionS[i].onFailStrategy->expressions[iFailExpressN].sData.v.value[0];
                        break;
                    case VAR_U32:
                        r_variable->v.value_u32 = pCondReq->pCondtionS[i].onFailStrategy->expressions[iFailExpressN].sData.v.value_u32;
                        break;
                    case VAR_S32:
                        r_variable->v.value_s32 = pCondReq->pCondtionS[i].onFailStrategy->expressions[iFailExpressN].sData.v.value_s32;
                        break;
                    case VAR_U64:
                        r_variable->v.value_u64 = pCondReq->pCondtionS[i].onFailStrategy->expressions[iFailExpressN].sData.v.value_u64;
                        break;
                    case VAR_FLOAT:
                        r_variable->v.value_f = pCondReq->pCondtionS[i].onFailStrategy->expressions[iFailExpressN].sData.v.value_f;
                        break;
                    case VAR_STRING:
                        if(pCondReq->pCondtionS[i].onFailStrategy->expressions[iFailExpressN].sData.v.value_s != NULL)
                        {
                            r_variable->v.value_s =
                                (D8U*)adm_strdup((DCHAR*)pCondReq->pCondtionS[i].onFailStrategy->expressions[iFailExpressN].sData.v.value_s);
                        }
                        break;
                    case VAR_S64:
                        r_variable->v.value_s64 = pCondReq->pCondtionS[i].onFailStrategy->expressions[iFailExpressN].sData.v.value_s64;
                        break;
                    default:
                        break;
                    }
                    l_variable = (cds_variable_st*)adm_malloc((DSIZE)sizeof(cds_variable_st));
                    if (NULL == l_variable)
                    {
                        cdmCompareStsDeinit(pCompareStFail, iFailExpressNum);
                        cdmDimSoaVdiMsgDataArrayDeinit(responseMsgArray);
                        return -1;
                    }
                    (void)adm_memset(l_variable,0,sizeof(cds_variable_st),sizeof(cds_variable_st));
                    l_variable->data_type = responseMsgArray->msgIdArray[i]->msgIdData->dataType;
                    l_variable->data_length = responseMsgArray->msgIdArray[i]->msgIdData->dataLength;
                    switch((VARIABLE_TYPE_EN)l_variable->data_type)
                    {
                    case VAR_U8:
                        l_variable->v.value[0] = responseMsgArray->msgIdArray[i]->msgIdData->v.value[0];
                        break;
                    case VAR_U32:
                        l_variable->v.value_u32 = responseMsgArray->msgIdArray[i]->msgIdData->v.valueU32;
                        break;
                    case VAR_S32:
                        l_variable->v.value_s32 = responseMsgArray->msgIdArray[i]->msgIdData->v.valueS32;
                        break;
                    case VAR_U64:
                        l_variable->v.value_u64 = responseMsgArray->msgIdArray[i]->msgIdData->v.valueU64;
                        break;
                    case VAR_FLOAT:
                        l_variable->v.value_f = responseMsgArray->msgIdArray[i]->msgIdData->v.valueF;
                        break;
                    case VAR_STRING:
                        if(responseMsgArray->msgIdArray[i]->msgIdData->v.valueS != NULL)
                        {
                            l_variable->v.value_s =
                                (D8U*)adm_strdup((DCHAR*)responseMsgArray->msgIdArray[i]->msgIdData->v.valueS);
                        }
                        break;
                    case VAR_S64:
                        l_variable->v.value_s64 = responseMsgArray->msgIdArray[i]->msgIdData->v.valueS64;
                        break;
                    default:
                        break;
                    }

                    pCompareStFail[iFailExpressN].r_variable = r_variable;
                    pCompareStFail[iFailExpressN].l_variable = l_variable;
                }

                //重新按失败策略的中的表达式比较
                uResult = (D8U)adm_cds_compare(pCompareStFail, iFailExpressNum);
                cdmCompareStsDeinit(pCompareStFail, iFailExpressNum);
            }
            else
            {
                log_i("pCondReq->pCondtionS[i] [adm_cds_compare] return 0x01,onFailStrategy==NULL\r\n");
            }
        }

        if(0 == i)
        {
            compare_result = uResult;
        }
        else
        {
            if((D32U)COMPARE_AND == pCondReq->pCondtionS[i].connectRelation)
            {
                if (compare_result != 0x00U)
                {
                    compare_result = uResult;
                }
            }
            else if((D32U)COMPARE_OR == pCondReq->pCondtionS[i].connectRelation)
            {
                if (compare_result == 0x00U)
                {
                    compare_result = uResult;
                }
            }
            else
            {
                compare_result = 0x00U;
            }
        }

        //保存本次比较结果数组
        ADM_UIM_SOA_CONDITION_DATA_T* condsArray = (ADM_UIM_SOA_CONDITION_DATA_T*)adm_malloc((DSIZE)sizeof(ADM_UIM_SOA_CONDITION_DATA_T));
        if (NULL == condsArray)
        {
            cdmDimSoaVdiMsgDataArrayDeinit(responseMsgArray);
            return -1;
        }
        (void)adm_memset(condsArray,0,sizeof(ADM_UIM_SOA_CONDITION_DATA_T),sizeof(ADM_UIM_SOA_CONDITION_DATA_T));
        pResult2UIM->condsArray[i] = condsArray;
        condsArray->msgId = responseMsgArray->msgIdArray[i]->msgId;

        // uResult = uResult ^ 0x01;
        condsArray->result = uResult == 0U ? 1 : 0;
        //add condition verify expressVal realVal 2022/2/23
        condsArray->expectVariable = NULL;
        
        condsArray->expectVariable = (variable_st*)adm_malloc((DSIZE)sizeof(variable_st));
        if (NULL == condsArray->expectVariable)
        {
            cdmDimSoaVdiMsgDataArrayDeinit(responseMsgArray);
            return -1;
        }
        (void)adm_memset(condsArray->expectVariable,0,sizeof(variable_st),sizeof(variable_st));
        
        //set expect Val
        condsArray->expectVariable->data_type = pCondReq->pCondtionS[i].expressions[0].sData.data_type;
        condsArray->expectVariable->data_length = pCondReq->pCondtionS[i].expressions[0].sData.data_length;
        switch((VARIABLE_TYPE_EN)condsArray->expectVariable->data_type)
        {
        case VAR_U8:
            condsArray->expectVariable->v.value[0]=pCondReq->pCondtionS[i].expressions[0].sData.v.value[0];
            break;
        case VAR_U32:
            condsArray->expectVariable->v.value_u32 = pCondReq->pCondtionS[i].expressions[0].sData.v.value_u32;
            break;
        case VAR_S32:
            condsArray->expectVariable->v.value_s32 = pCondReq->pCondtionS[i].expressions[0].sData.v.value_s32;
            break;
        case VAR_U64:
            condsArray->expectVariable->v.value_u64 = pCondReq->pCondtionS[i].expressions[0].sData.v.value_u64;
            break;
        case VAR_FLOAT:
            condsArray->expectVariable->v.value_f = pCondReq->pCondtionS[i].expressions[0].sData.v.value_f;
            break;
        case VAR_STRING:
            condsArray->expectVariable->v.value_s =
                (D8U*)adm_strdup((DCHAR*)pCondReq->pCondtionS[i].expressions[0].sData.v.value_s);
            break;
        case VAR_S64:
            //uim 未定义
            //condsArray->expectVariable->v.value_s64 = pCondReq->pCondtionS[i].expressions[0].sData.v.value_s64;
            break;
        default:
            break;
        }

        condsArray->realVariable = NULL;
        
        //set realVal
        condsArray->realVariable = (variable_st*)adm_malloc((DSIZE)sizeof(variable_st));
        if (NULL == condsArray->realVariable)
        {
            cdmDimSoaVdiMsgDataArrayDeinit(responseMsgArray);
            return -1;
        }
        (void)adm_memset(condsArray->realVariable,0,sizeof(variable_st),sizeof(variable_st));
        
        condsArray->realVariable->data_type = responseMsgArray->msgIdArray[i]->msgIdData->dataType;
        condsArray->realVariable->data_length = responseMsgArray->msgIdArray[i]->msgIdData->dataLength;
        switch((VARIABLE_TYPE_EN)condsArray->realVariable->data_type)
        {
        case VAR_U8:
            condsArray->realVariable->v.value[0] = responseMsgArray->msgIdArray[i]->msgIdData->v.value[0];
            break;
        case VAR_U32:
            condsArray->realVariable->v.value_u32 = responseMsgArray->msgIdArray[i]->msgIdData->v.valueU32;
            break;
        case VAR_S32:
            condsArray->realVariable->v.value_s32 = responseMsgArray->msgIdArray[i]->msgIdData->v.valueS32;
            break;
        case VAR_U64:
            condsArray->realVariable->v.value_u64 = responseMsgArray->msgIdArray[i]->msgIdData->v.valueU64;
            break;
        case VAR_FLOAT:
            condsArray->realVariable->v.value_f = responseMsgArray->msgIdArray[i]->msgIdData->v.valueF;
            break;
        case VAR_STRING:
            if(responseMsgArray->msgIdArray[i]->msgIdData->v.valueS != NULL)
            {
                condsArray->realVariable->v.value_s =
                    (D8U*)adm_strdup((DCHAR*)responseMsgArray->msgIdArray[i]->msgIdData->v.valueS);
            }
            break;
        case VAR_S64:
            //uim 未定义
            //condsArray->realVariable->v.value_s64 = responseMsgArray->msgIdArray[i]->msgIdData->v.value_s64;
            break;
        default:
            break;
        }
    }

    cdmDimSoaVdiMsgDataArrayDeinit(responseMsgArray);

    return compare_result == 0U ? -1 : 0;
}

/**
 * @brief  条件校验入口函数
 * @param  条件模块入参期望值
 * @param  条件模块出参用于UIM显示
 * @param  条件判断结果值
 * @retval int o success
 */
D32S adm_cdm_verify_Compare(const ADM_COND_POLICY_T* pCondReq,ADM_UIM_SOA_CONDITION_DATA_ARRAY_T** pStCondResults,DBOOL* pbIsConform)
{
    //执行条件比对
    D32S iresult  = adm_cdm_Handle(pCondReq,pStCondResults);

    *pbIsConform = (DBOOL)iresult;
    return iresult;
}

static void cdmVariableDeinit(variable_pt pVar)
{
    if (NULL != pVar)
    {
        if (VAR_STRING == (VARIABLE_TYPE_EN)pVar->data_type)
        {
            adm_free(pVar->v.value_s);
        }
        adm_free(pVar);
    }
}

static void cdmUimConditionDataDeinit(ADM_UIM_SOA_CONDITION_DATA_T* conditionData)
{
    if (NULL != conditionData)
    {
        cdmVariableDeinit(conditionData->expectVariable);
        cdmVariableDeinit(conditionData->realVariable);
        adm_free(conditionData);
    }
}

/**
 * @brief  释放内部空间
 * @param  条件结果返回指针
 * @retval result
 */
D32S adm_cdm_verify_free(ADM_UIM_SOA_CONDITION_DATA_ARRAY_T* pStCondResults)
{
    if(pStCondResults != NULL)
    {
        if(pStCondResults->condsArray != NULL)
        {
            for(D32U i = 0;i < (D32U)pStCondResults->condsArrayLength;i++)
            {
                if(pStCondResults->condsArray[i] != NULL)
                {
                    cdmUimConditionDataDeinit(pStCondResults->condsArray[i]);
                }
            }

            adm_free(pStCondResults->condsArray);
        }

        adm_free(pStCondResults);
    }
    
    return 0;
}

/**
 * @brief  定时校验循环函数
 * @param  条件模块入参
 * @retval void
 */
static DBOOL adm_cdm_TimerLoop(void* pParam)
{
    ADM_CDM_LOOP_PARAM_T* pInfo = (ADM_CDM_LOOP_PARAM_T *)pParam;
    ADM_COND_POLICY_T* pCondPolicy = pInfo->pCondition;
    // D32S iCondNum = pCondPolicy->iCondNum;
    ADM_CDM_NOTIFY_Fn funcCallback = pInfo->callback;

    time_t isecond    = time(NULL);
    time_t icurSecond = 0;
    ADM_UIM_SOA_CONDITION_DATA_ARRAY_T* pResultMsg = NULL;
    D32S iresult = 0;

    while(pInfo->iThreadState != 0)
    {
        //时间判断
        icurSecond = time(NULL);
        if(icurSecond > isecond+pInfo->iCycle)
        {
            iresult = adm_cdm_Handle(pCondPolicy,&pResultMsg);
            (void)funcCallback(pResultMsg,(DBOOL*)&iresult);
            (void)adm_cdm_verify_free(pResultMsg);
            pResultMsg = NULL;
            isecond =  icurSecond;
            iresult = 0;
        }
        (void)usleep(10);
    }

    return DFALSE;  // 返回false线程执行的任务才会正确结束
}

/**
 * @brief  get variable compare result
 * @param  compares condtions variable struct
 * @param  loop separatetimes
 * @param  回调函数指针
 * @retval result
 */
adm_handle_t adm_cdm_LoopVerify(const ADM_COND_POLICY_T* pCondtionList,D32S iCycle,ADM_CDM_NOTIFY_Fn callback)
{
    log_i("start loopVerify");
    if(ADM_CDM_MAXTIMER == m_iRunTimer)
    {
        //定时器启动大于最大值,直接返回错误
        return NULL;
    }
    ADM_CDM_LOOP_PARAM_T* pLoopVerifyInfo = NULL;
    pLoopVerifyInfo = (ADM_CDM_LOOP_PARAM_T*)adm_malloc((DSIZE)sizeof(ADM_CDM_LOOP_PARAM_T));
    if (NULL == pLoopVerifyInfo)
    {
        return NULL;
    }

    (void)adm_memset(pLoopVerifyInfo,0,sizeof(ADM_CDM_LOOP_PARAM_T),sizeof(ADM_CDM_LOOP_PARAM_T));
    
    pLoopVerifyInfo->pCondition = (ADM_COND_POLICY_T*)adm_malloc((DSIZE)sizeof(ADM_COND_POLICY_T));
    if (NULL == pLoopVerifyInfo->pCondition)
    {
        adm_free(pLoopVerifyInfo);
        return NULL;
    }

    adm_condPolicyClone(pCondtionList, pLoopVerifyInfo->pCondition);

    pLoopVerifyInfo->callback = callback;
    pLoopVerifyInfo->iCycle = iCycle;
    
    adm_handle_t  threadHandle;
    threadHandle = adm_thread_create_noname();
    pLoopVerifyInfo->iThreadState = 1;
    pLoopVerifyInfo->threadHandle = threadHandle;

    adm_thread_set_func(threadHandle, adm_cdm_TimerLoop,pLoopVerifyInfo);
    (void)adm_thread_start(threadHandle);
    
    m_pTimerInfoList[m_iRunTimer] = pLoopVerifyInfo;
    m_iRunTimer = m_iRunTimer+1;

    return threadHandle;
}

/**
 * @brief  finish Loop
 * @retval 0 success,else failed
 */
D32S adm_cdm_LoopFinish(adm_handle_t timerHandle)
{
    if(m_iRunTimer <= 0)
    {
        return -1;
    }
    if(NULL == timerHandle)
    {
        return -1;
    }
    
    D32S i = 0;
    // D32S k = 0;
    for(i = 0; i < ADM_CDM_MAXTIMER;i++)
    {
        if(NULL == m_pTimerInfoList[i])
        {
            continue;
        }
        if(m_pTimerInfoList[i]->threadHandle == timerHandle)
        {
            m_pTimerInfoList[i]->iThreadState = 0;
            (void)adm_thread_stop(timerHandle);
            adm_thread_destroy(timerHandle);
            if(m_pTimerInfoList[i]->pCondition != NULL)
            {
                adm_condPolicyDeInit(m_pTimerInfoList[i]->pCondition);
            }

            adm_free(m_pTimerInfoList[i]);
            log_i("ok,stop cdm loop verify condition thread\r\n");
            return 0;
        }
    }
    log_i("not find runing timer thread\r\n");
    return -1;
}


