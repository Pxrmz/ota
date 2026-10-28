#include "adm_hut_comm.h"
#include "adm_ipm_hmi.h"

static void* m_pInstallClientSession = NULL;

/*
 * @brief  安装回调函数
 * @return
 */
void adm_hut_installNotify(ADM_IPS_INSTALL_PROGRESS_T *pInstallNotify)
{
    if(NULL == pInstallNotify)
    {
        return;
    }

    struct json_object *pOutJson  = NULL;
    struct json_object *pArray    = NULL;
    struct json_object *pChild    = NULL;
    D32S strategyTimeConsuming = 0; 
    D32S state                 = 0;      
    D32U progress              = 0;              
    D32S result                = 0;
    D32U ecusLen               = 0;
    D32S i                     = 0;

    DCHAR* ecuReqId   = NULL;
    DCHAR *ecuSwDId   = NULL;
    DCHAR *ecuName    = NULL; 
    DCHAR *ecuPartNum = NULL; 
    DCHAR *dstver     = NULL; 
    DCHAR *srcver     = NULL; 

    strategyTimeConsuming = pInstallNotify->estimateTotaltime;
    state                 = pInstallNotify->state;
    progress              = pInstallNotify->progress;
    result                = pInstallNotify->result;
    ecusLen               = pInstallNotify->ecusLen;

    pOutJson = json_object_new_object();
    json_object_object_add(pOutJson,"estimateTotaltime",json_object_new_int(strategyTimeConsuming));
    json_object_object_add(pOutJson,"state",json_object_new_int(state));
    json_object_object_add(pOutJson,"progress",json_object_new_int(progress));
    json_object_object_add(pOutJson,"result",json_object_new_int(result));

    pArray = json_object_new_array();
    for(i = 0;i<ecusLen;i++)
    {
        ecuReqId   = pInstallNotify->ecus[i].ecuReqId;
        ecuSwDId   = pInstallNotify->ecus[i].ecuSwDid;
        ecuName    = pInstallNotify->ecus[i].ecuName;
        ecuPartNum = pInstallNotify->ecus[i].ecuPartNum;
        dstver     = pInstallNotify->ecus[i].dstVer;
        srcver     = pInstallNotify->ecus[i].srcVer;
        state      = pInstallNotify->ecus[i].state;
        progress   = pInstallNotify->ecus[i].progress;
        result     = pInstallNotify->ecus[i].result;

        pChild    = json_object_new_object();
        json_object_object_add(pChild,"ecuPartNum",
                        json_object_new_string((NULL == ecuPartNum) ? "" : ecuPartNum));
        json_object_object_add(pChild,"ecuPartName",
                        json_object_new_string((NULL == ecuName) ? "" : ecuName));
        json_object_object_add(pChild,"ecuReqId",
                        json_object_new_string((NULL == ecuReqId) ? "" : ecuReqId));
        json_object_object_add(pChild,"ecuSwDId",
                        json_object_new_string((NULL == ecuSwDId) ? "" : ecuSwDId));
        json_object_object_add(pChild,"srcVer",
                        json_object_new_string((NULL == srcver) ? "" : srcver));
        json_object_object_add(pChild,"dstVer",
                        json_object_new_string((NULL == dstver) ? "" : dstver));
        json_object_object_add(pChild,"state",json_object_new_int(state));
        json_object_object_add(pChild,"progress",json_object_new_int(progress));
        json_object_object_add(pChild,"result",json_object_new_int(result));
        json_object_array_add(pArray, pChild);
    }
    json_object_object_add(pOutJson,"detail",pArray);
    DCHAR* pPrintStr = (DCHAR*)json_object_to_json_string(pOutJson);
    if(pPrintStr != NULL)
    {
        DCHAR* pTopic = "/vus/up/install/progress";
        D32S iLen = adm_strlen(pPrintStr);
        mqtt_send_hut(m_pInstallClientSession,pPrintStr, iLen,pTopic);
    }
    else
    {
        printf("pPrintStr == NULL");
    }
    json_object_put(pOutJson);
    return;
}

/*
 * @brief  安装执行函数
 * @return D32S  
 */
D32S adm_hut_install(void* pClientSession)
{
    if(pClientSession == NULL)
    {
        return -1;
    }
    m_pInstallClientSession = pClientSession;
    D32S iret = adm_ipm_installHmiTrigger(&adm_hut_installNotify);
    if(iret != 0)
    {
        printf("%s,%d trigger install failed,errorcode=%d\r\n",__FILE__,__LINE__,iret);
        return -1;
    }
    printf("%s,%d trigger install success\r\n",__FILE__,__LINE__);

    return 0;
}





