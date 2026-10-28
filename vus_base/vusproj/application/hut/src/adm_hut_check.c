
#include "adm_hut_check.h"
#include "adm_hut_comm.h"
#include <string.h>

/**
 * @brief 执行check操作
 * @brief
 *       Response to cloud Json
 *       {
 *          "result":1,
 *          "HMIShowPath":/hmi/show/path
 *          "HMIShowFileHash":"8078abdb927b9f85399b29ffed541b9957463078ad7f08ccc281a600d889a048",
 *          "HMIShowFileSize":578
 *       }
 *       Request from cloud Json
 *       {
 *            "check":1
 *       }
 * 
 * @return D32S     0:successed, non-0:failed 
 */
D32S adm_hut_checkwork(void* pClientSession)
{
    const DCHAR* pConstructArg = "Check Demo";
    ADM_CVM_RESP_T* pResp = adm_cvm_newRespBody();
    if(NULL == pResp)
    {
        printf("%s,%d adm_cvm_newRespBody error\r\n",__FILE__,__LINE__);
        return -1;
    }
    
    printf("adm_hut_checkwork\r\n");
    
    struct json_object *pOutJson  = NULL;
    
    pOutJson   = json_object_new_object();
 
    D32S iret = adm_cvm_checkVersion(123,NULL,pResp);
    if(iret != 0)
    {
        printf("%s,%d checkVersion falied\r\n",__FILE__,__LINE__);
        json_object_object_add(pOutJson,"result",json_object_new_int(1));
        json_object_object_add(pOutJson,"HMIShowPath",json_object_new_string(""));
        json_object_object_add(pOutJson,"HMIShowFileHash",json_object_new_string(""));
        json_object_object_add(pOutJson,"HMIShowFileSize",json_object_new_int(0));
    }
    else
    {
        printf("checkVersion success\r\n");
        json_object_object_add(pOutJson,"result",json_object_new_int(0));
        json_object_object_add(pOutJson,"HMIShowPath",
            json_object_new_string((NULL == pResp->hmiShowPath) ? "" : pResp->hmiShowPath));
        json_object_object_add(pOutJson,"HMIShowFileHash",
            json_object_new_string((NULL == pResp->hmiShowFileHash) ? "" : pResp->hmiShowFileHash));
        json_object_object_add(pOutJson,"HMIShowFileSize",json_object_new_int(pResp->hmiShowFileSize)); 
    }

    DCHAR* pPrintStr = NULL;
    pPrintStr = (DCHAR*)json_object_to_json_string(pOutJson);

    D32S   iLen = 0;
    iLen = strlen(pPrintStr);
    
    DCHAR* pTopic = "/vus/up/check";
    mqtt_send_hut(pClientSession,pPrintStr, iLen,pTopic);
    //printf("check response json message=%s\r\n",pPrintStr);
    json_object_put(pOutJson);
    return 0;
}

