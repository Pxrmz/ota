#include "adm_com_modDefine.h"

#define MOD_TAG_ID          ADM_COM_MOD_VRM
#define LOG_TAG             MOD_NAME(ADM_COM_MOD_VRM)         /*!< 宏函数，用于通过模块名的枚举值类型获取模块名称     */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "adm_cdm_verify_interface.h"
#include "adm_dim_vdi.h"
#include "adm_dim_host.h"
#include "adm_debug.h"

#if 0
void* MyTestCallBack(ADM_UIM_SOA_CONDITION_DATA_ARRAY_T* condsArrarList,DBOOL* pbIsConform)
{
    DownLoadCondVerifyCallBack msg;
    msg.set_icmdtype(1);

    msg.set_session("0x1001");

    msg.set_state("0x1002");
    
    Protoc_CondVerifyArrayCallback* pCondArray = msg.mutable_condsarrarlist();
    pCondArray->set_condsarraylength(condsArrarList->condsArrayLength);
    for(D32S i = 0; i < condsArrarList->condsArrayLength;i++)
    {
        ADM_UIM_SOA_CONDITION_DATA_T data = (*condsArrarList->condsArray)[i];
        Protoc_CondVerifyData* pCondData = pCondArray->add_condsarray();
        pCondData->set_msgid(data.msgId);
        
        Protoc_CondVerifyValue* pCondExpVal = pCondData->mutable_expectvalue();
        if(NULL == data.expectVariable)
        {
            continue;
        }
        pCondExpVal->set_data_type(data.expectVariable->data_type);
        pCondExpVal->set_data_length(data.expectVariable->data_length);
        switch(data.expectVariable->data_type)
        {
        case 1:
            {
                D8U* pValue = data.expectVariable->v.value;
                pCondExpVal->set_datavalue((DCHAR*)pValue);
                break;
            }
        case 2:
            pCondExpVal->set_datauint32(data.expectVariable->v.value_u32);
            break;
        case 3:
            pCondExpVal->set_dataint32(data.expectVariable->v.value_s32);
            break;
        case 4:
            pCondExpVal->set_datauint64(data.expectVariable->v.value_u64);
            break;
        case 5:
            pCondExpVal->set_datafloat(data.expectVariable->v.value_f);
            break;
        case 6:
            pCondExpVal->set_datastring((DCHAR*)(data.expectVariable->v.value_s));
            break;
        default:
            break;
        }
        
        Protoc_CondVerifyValue* pCondRealVal = pCondData->mutable_realvariable();
        if(NULL == data.realVariable)
        {
            continue;
        }
        pCondRealVal->set_data_type(data.realVariable->data_type);
        pCondRealVal->set_data_length(data.realVariable->data_length);
        switch(data.realVariable->data_type)
        {
        case 1:
            {
                D8U* pValue = data.realVariable->v.value;
                pCondRealVal->set_datavalue((DCHAR*)pValue);
                break;
            }
        case 2:
            pCondRealVal->set_datauint32(data.realVariable->v.value_u32);
            break;
        case 3:
            pCondRealVal->set_dataint32(data.realVariable->v.value_s32);
            break;
        case 4:
            pCondRealVal->set_datauint64(data.realVariable->v.value_u64);
            break;
        case 5:
            pCondRealVal->set_datafloat(data.realVariable->v.value_f);
            break;
        case 6:
            pCondRealVal->set_datastring((DCHAR*)(data.realVariable->v.value_s));
            break;
        default:
            break;
        }
    }
/*    
    msg.set_iresult(verifyResult);
    
    D32S iLen = msg.ByteSizeLong();
    DCHAR* pBuf=(DCHAR*)adm_malloc(iLen);
    msg.SerializeToArray(pBuf,iLen);
    std::vector<vsomeip::byte_t> resp;
    for(D32S i = 0; i < iLen;i++)
    {
        resp.push_back(pBuf[i]);
    }
    adm_free(pBuf);
*/    
    return NULL;
}

#endif

int main(int argc,char* argv[])
{
    //adm_initLog();
    //adm_tracer_entry("VUS", modInfo);

    ADM_COND_POLICY_T* pCondtions = (ADM_COND_POLICY_T*)malloc(sizeof(ADM_COND_POLICY_T));
    memset(pCondtions,0,sizeof(ADM_COND_POLICY_T));
    pCondtions->iType = 1;
    pCondtions->iCondNum = 1;
    pCondtions->pCondtionS = (CONDTION_T*)malloc(sizeof(CONDTION_T)*1);
    memset(pCondtions->pCondtionS,0,sizeof(CONDTION_T)*1);

    pCondtions->pCondtionS[0].serviceType = (DCHAR*)("0x001");
    pCondtions->pCondtionS[0].messageId = (DCHAR*)("0x002");
    pCondtions->pCondtionS[0].messageName = (DCHAR*)("netType");
    pCondtions->pCondtionS[0].messageDesc = (DCHAR*)("selectDownloadNetType");
    pCondtions->pCondtionS[0].iExpressNum = 1;
    pCondtions->pCondtionS[0].expressions = (EXPRESSION_T*)malloc(sizeof(EXPRESSION_T)*1);
    memset(pCondtions->pCondtionS[0].expressions,0,sizeof(EXPRESSION_T));
    
    
    pCondtions->pCondtionS[0].expressions[0].iLogic = 1;
    pCondtions->pCondtionS[0].expressions[0].sData.data_type = 7;
    pCondtions->pCondtionS[0].expressions[0].sData.data_length = 1;
    //pCondtions->pCondtionS[0].expressions[0].sData.v.value[0] = '0';
    //pCondtions->pCondtionS[0].expressions[0].sData.v.value_s32 = 4;
    //pCondtions->pCondtionS[0].expressions[0].sData.v.value_u32 = 6;
    //pCondtions->pCondtionS[0].expressions[0].sData.v.value_u64 = 123456;
    //pCondtions->pCondtionS[0].expressions[0].sData.v.value_f = 123.00;
    pCondtions->pCondtionS[0].expressions[0].sData.v.value_s64 = 654321;
    //pCondtions->pCondtionS[0].expressions[0].sData.v.value_s=(D8U*)("1");
    pCondtions->pCondtionS[0].expressions[0].connectRelation = 1;
    pCondtions->pCondtionS[0].connectRelation = 1;

/*
    pCondtions->pCondtionS[1].messageId = "2";
    pCondtions->pCondtionS[1].messageName = "allowCarEMode";
    pCondtions->pCondtionS[1].messageDesc = "isAllowCarEMode";
    pCondtions->pCondtionS[1].iLogic = 2;
    pCondtions->pCondtionS[1].sData.iDataType = 5;
    pCondtions->pCondtionS[1].sData.iDataLength = 1;
    pCondtions->pCondtionS[1].sData.v.valueF=100.0;
    pCondtions->pCondtionS[1].iConnectRelation = 1;

    pCondtions->pCondtionS[2].messageId = "3";
    pCondtions->pCondtionS[2].messageName = "allowCarCrash";
    pCondtions->pCondtionS[2].messageDesc = "isAllowCarCrash";
    pCondtions->pCondtionS[2].iLogic = 1;
    pCondtions->pCondtionS[2].sData.iDataType = 4;
    pCondtions->pCondtionS[2].sData.iDataLength = 1;
    pCondtions->pCondtionS[2].sData.v.valueU64 = 128;
    pCondtions->pCondtionS[2].iConnectRelation = 1;

    pCondtions->pCondtionS[3].messageId = "4";
    pCondtions->pCondtionS[3].messageName = "allowConnectOBD";
    pCondtions->pCondtionS[3].messageDesc = "isAllowConnectOBD";
    pCondtions->pCondtionS[3].iLogic = 2;
    pCondtions->pCondtionS[3].sData.iDataType = 3;
    pCondtions->pCondtionS[3].sData.iDataLength = 1;
    pCondtions->pCondtionS[3].sData.v.valueS32 = 32;
    pCondtions->pCondtionS[3].iConnectRelation = 1;

    pCondtions->pCondtionS[4].messageId = "5";
    pCondtions->pCondtionS[4].messageName = "batteryPowerMin";
    pCondtions->pCondtionS[4].messageDesc = "batteryPowerMinInDownload";
    pCondtions->pCondtionS[4].iLogic = 1;
    pCondtions->pCondtionS[4].sData.iDataType = 2;
    pCondtions->pCondtionS[4].sData.iDataLength = 1;
    pCondtions->pCondtionS[4].sData.v.valueU32= 90;
    pCondtions->pCondtionS[4].iConnectRelation = 1;
*/
    ADM_UIM_SOA_CONDITION_DATA_ARRAY_T* pStCondResults=NULL;
    printf("addr =%p\r\n",pStCondResults);
    DBOOL bIsConform=1;
    
    D32S iret = adm_cdm_verify_Compare(pCondtions,&pStCondResults,&bIsConform);
    printf("after addr =%p\r\n",pStCondResults);
    printf("verify result=%d\r\n",bIsConform);
    adm_cdm_verify_free(pStCondResults);

    pStCondResults=NULL;
/*
    adm_handle_t hHandle1 = adm_cdm_LoopVerify(pCondtions,5,MyTestCallBack);
    
    //adm_handle_t hHandle2 = adm_cdm_LoopVerify(pCondtions,5,MyTestCallBack);
    int i = 0;
    while(i< 10)
    {
        sleep(1);
        i++;
    }

    adm_cdm_LoopFinish(hHandle1);
*/
    //adm_cdm_LoopFinish(hHandle2);
   
    while(1)
    {
        sleep(1);
    }
    return 0;
}



