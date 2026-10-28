
#include "adm_hut_comm.h"
#include "adm_hut_download.h"

static void* m_pClientSession = NULL;

/*!
 * @brief 下载过程通知回调函数(总进度/结果 + 各ecu进度/结果信息)
 */
void adm_hut_downnload_notify(ADM_DPS_DOWNPROCESS_T *pstDownProcessInfo)
{
    if(NULL == pstDownProcessInfo)
    {
        return;
    }
    struct json_object *pOutJson  = NULL;
    struct json_object *pArray    = NULL;
    struct json_object *pChild    = NULL;
    DCHAR *partNum  = NULL;     
    DCHAR *partName = NULL;    
    DCHAR *reqId    = NULL;
    DCHAR *swDid    = NULL;       
    DCHAR *ecuSVer  = NULL;     

    ADM_DPS_RESULT_T   stResult;    
    ADM_DPS_PROGRESS_T stProgress;  
    
    D32S result = 0;
    D32S eDownState = 0;
    D32S speed      = 0;         
    D32S totalTime  = 0;     
    D64S saveBytes  = 0;     
    D64S totalBytes = 0;
    D32S num        = 0;
    D32S i          = 0;
    result = pstDownProcessInfo->stDownResult.result;
    eDownState = pstDownProcessInfo->stDownResult.eDownState;
    speed      = pstDownProcessInfo->stDownProgress.speed;
    totalTime  = pstDownProcessInfo->stDownProgress.totalTime;
    saveBytes  = pstDownProcessInfo->stDownProgress.saveBytes;
    totalBytes = pstDownProcessInfo->stDownProgress.totalBytes;
    num        = pstDownProcessInfo->stProcess.num;
    
    pOutJson = json_object_new_object();
    json_object_object_add(pOutJson,"state",json_object_new_int(eDownState));
    json_object_object_add(pOutJson,"result",json_object_new_int(result));
    json_object_object_add(pOutJson,"downloadSize",json_object_new_int64(saveBytes));
    json_object_object_add(pOutJson,"totalSize",json_object_new_int64(totalBytes));
    json_object_object_add(pOutJson,"downloadSpeed",json_object_new_int(speed));

    pArray = json_object_new_array();
    for(i = 0; i < num;i++)
    {
        partNum  = pstDownProcessInfo->stProcess.pstProcessList[i].partNum;
        partName = pstDownProcessInfo->stProcess.pstProcessList[i].partName;
        reqId    = pstDownProcessInfo->stProcess.pstProcessList[i].reqId;
        swDid    = pstDownProcessInfo->stProcess.pstProcessList[i].swDid;
        ecuSVer  = pstDownProcessInfo->stProcess.pstProcessList[i].ecuSVer;
        result   = pstDownProcessInfo->stProcess.pstProcessList[i].stResult.result;
        eDownState = pstDownProcessInfo->stProcess.pstProcessList[i].stResult.eDownState;
        speed    = pstDownProcessInfo->stProcess.pstProcessList[i].stProgress.speed;
        totalTime= pstDownProcessInfo->stProcess.pstProcessList[i].stProgress.totalTime;
        saveBytes= pstDownProcessInfo->stProcess.pstProcessList[i].stProgress.saveBytes;
        totalBytes = pstDownProcessInfo->stProcess.pstProcessList[i].stProgress.totalBytes;
        
        pChild    = json_object_new_object();
        json_object_object_add(pChild,"partNum",
                            json_object_new_string((NULL == partNum) ? "" : partNum));
        json_object_object_add(pChild,"partName",
                            json_object_new_string((NULL == partName) ? "" : partName));
        json_object_object_add(pChild,"ecuReqId",
                            json_object_new_string((NULL == reqId) ? "" : reqId));
        json_object_object_add(pChild,"ecuSwDId",
                            json_object_new_string((NULL == swDid) ? "" : swDid));
        json_object_object_add(pChild,"ecuSVer",
                            json_object_new_string((NULL == ecuSVer) ? "" : ecuSVer));
        json_object_object_add(pChild,"state",json_object_new_int(eDownState));
        json_object_object_add(pChild,"result",json_object_new_int(result));
        json_object_object_add(pChild,"ecuDownloadSize",json_object_new_int64(saveBytes));
        json_object_object_add(pChild,"ecuTotalSize",json_object_new_int64(totalBytes));
        json_object_object_add(pChild,"ecuDownloadSpeed",json_object_new_int(speed));
        json_object_array_add(pArray, pChild);
    }
    json_object_object_add(pOutJson,"detail",pArray);
    DCHAR* pPrintStr = (DCHAR*)json_object_to_json_string(pOutJson);
    if(pPrintStr != NULL)
    {   
        DCHAR* pTopic = "/vus/up/download/progress";
        D32S iLen = adm_strlen(pPrintStr);
        mqtt_send_hut(m_pClientSession,pPrintStr, iLen,pTopic);
    }
    else
    {
        printf("pPrintStr == NULL");
    }
    json_object_put(pOutJson);  

    return;
}


/*
 * @brief  执行download
 * @return D32S  
 */
D32S adm_hut_download(void* pClientSession)
{
  
    if(pClientSession == NULL)
    {
        return -1;
    }

    m_pClientSession = pClientSession;

    //D32S iret = adm_dpm_triggerDownload(&adm_hut_downnload_notify);
    D32S iret = adm_dpm_triggerDownload(1);
    if(iret != 0)
    {
        printf("%s,%d trgger download failed,errorcode=%d\r\n",__FILE__,__LINE__,iret);
        return -1;
    }
    printf("%s,%d trigger download success\r\n",__FILE__,__LINE__);

    return 0;
}

