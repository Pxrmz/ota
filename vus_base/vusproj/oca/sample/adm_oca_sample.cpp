#include "adm_oca_log.h"
#include "adm_oca_connect.h"
// #include "adm_typedefs.h"
// #include "adm_debug.h"
#include "adm_oca_file.h"
#include "pthread.h"
#include "adm_string.h"
#include "adm_memory.h"
#include <unistd.h>


void adm_oca_downloadProgressHandler(ADM_OCA_FILE_PROGRESS_T* pstProgress, void* priv)
{
    // log_i( "savedSize:%ld, downloadSpeed:%lf, remainTime:%lf", pstProgress->savedSize, pstProgress->speed, pstProgress->remainTime);
    return;
}

void* uploadHandler(void* priv)
{

    D32S iRet = 0;
    ADM_OCA_FILE_UPLOAD_INFO_T* pstUploadInfo = NULL;
    ADM_OCA_FILE_UPLOAD_HANDLE_T* pHandle = NULL;
    ADM_OCA_COMMON_INIT_HANDLER_T* connectHandler = NULL;
    DCHAR* rspOutData = NULL;

    adm_initLog();

    do {
        pHandle = adm_oca_fileUploadInit();
        if (NULL == pHandle) {
            iRet = -1;
            log_e("adm_oca_fileUploadInit failed %d", iRet);
            break;
        }

        pstUploadInfo = adm_oca_newUploadInfo();
        if (NULL == pstUploadInfo) {
            iRet = -1;
            log_e("adm_oca_newUploadInfo failed, %d", iRet);
            break;
        }

        connectHandler = adm_oca_connectInit();
        if (NULL == connectHandler) {
            iRet = -1;
            log_e("adm_oca_connectInit failed");
            break;
        }

        adm_oca_setConnectProtocolType(connectHandler, ADM_OCA_CONNECT_PROTOCOL_TYPE_HTTP);
        adm_oca_setConnectAddress(connectHandler, ADM_OCA_CONNECT_ADDRESS_TYPE_URL, "http://172.16.30.56:8082/upload/uploadtest.zip");

        pstUploadInfo->cbUploadProgress = adm_oca_downloadProgressHandler;

        adm_oca_addSectionToHttpPost(pHandle, "filename", "uploadtest123.zip", ADM_OCA_HTTP_POST_SECTION_TYPE_COPYCONTENTS);
        adm_oca_addSectionToHttpPost(pHandle, "timestamp", "169999999", ADM_OCA_HTTP_POST_SECTION_TYPE_COPYCONTENTS);
        adm_oca_addSectionToHttpPost(pHandle, "logFile", "./Makefile", ADM_OCA_HTTP_POST_SECTION_TYPE_FILE);

        iRet = adm_oca_startUpload(connectHandler, pHandle, pstUploadInfo, &rspOutData);
        if (iRet == 0) {
            log_i("adm_oca_startUpload successed");
        } else {
            log_i( "adm_oca_startUpload failed, reason code:%d", iRet);
        }

    } while(0);

    if (rspOutData) {
        adm_free(rspOutData);
    }

    adm_oca_fileUploadDeinit(pHandle);
    adm_oca_delUploadInfo(pstUploadInfo);
    adm_oca_connectDeinit(connectHandler);

    return NULL;
}


int file_size(const char* filename)
{
    FILE *fp=fopen(filename,"r");
    if(!fp) return -1;
    fseek(fp,0L,SEEK_END);
    int size=ftell(fp);
    fclose(fp);
    
    return size;
}

void* downloadHandler(void* priv)
{
    D32S iRet = 0;
    ADM_OCA_COMMON_INIT_HANDLER_T* connectHandler = NULL;
    ADM_OCA_FILE_DOWNLOAD_INFO_T1* downloadInfo = NULL;

    do {
        // adm_initLog();
        connectHandler = adm_oca_connectInit();
        if (NULL == connectHandler) {
            iRet = -1;
            log_e("adm_oca_connectInit failed");
            break;
        }

        adm_oca_setConnectProtocolType(connectHandler, ADM_OCA_CONNECT_PROTOCOL_TYPE_HTTP);
        adm_oca_setConnectAddress(connectHandler, ADM_OCA_CONNECT_ADDRESS_TYPE_URL, "http://otadown-dev.evergrandeauto.com/download/pkg/yf000000000000012/testPkg");
        
        downloadInfo = adm_oca_newDownloadInfo();
        if (NULL == downloadInfo) {
            iRet = -1;
            break;
        }

        downloadInfo->savePath = adm_strdup("./config.json");


        if((access("./config.json",F_OK))!=-1)   
        {   
            downloadInfo->startPosition = file_size("./config.json");
        } else {
            downloadInfo->startPosition = 0;
        }
        downloadInfo->endPosition = 0;
        // downloadInfo->startPosition = 1728512;
        downloadInfo->cbDownloadProgress = adm_oca_downloadProgressHandler;
        
        iRet = adm_oca_startDownload(connectHandler, downloadInfo);
        if (iRet == 0) {
            log_i("adm_oca_startDownload successed");
        } else {
            log_i( "adm_oca_startDownload failed, reason code:%d", iRet);
        }
    }while(0);

    adm_oca_delDownloadInfo(downloadInfo);
    adm_oca_connectDeinit(connectHandler);

    return NULL;
}

D32S main()
{
    D32S iRet = -1;
    DSIZE statusCode = -1;
    DCHAR* responseBody = NULL;
    ADM_OCA_COMMON_INIT_HANDLER_T* connectHandler = NULL;
    pthread_t tid1;
    pthread_t tid2;

    adm_debug_logInit("/tmp/vus/", (1 * 1024 * 1025), 5);

    DBOOL bRet = TRUE;

    log_i("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx");
    // do {
    //     if (FALSE == adm_oca_registerSetConnectProtocolTypeHandler(adm_oca_setConnectProtocolType)) {
    //         log_e("adm_oca_registerSetConnectProtocolTypeHandler failed");
    //         bRet = FALSE;
    //         break;
    //     }
    //     if (FALSE == adm_oca_registerSetConnectCertPathHandler(adm_oca_setConnectCertPath)) {
    //         log_e("adm_oca_registerSetConnectCertPathHandler failed");
    //         bRet = FALSE;
    //         break;
    //     }
    //     if (FALSE == adm_oca_registerSetConnectAddressHandler(adm_oca_setConnectAddress)) {
    //         log_e("adm_oca_registerSetConnectAddressHandler failed");
    //         bRet = FALSE;
    //         break;
    //     }
    //     if (FALSE == adm_oca_registerSetMqttConnectApiNameHandler(adm_oca_setMqttConnectApiName)) {
    //         log_e("adm_oca_registerSetMqttConnectApiNameHandler failed");
    //         bRet = FALSE;
    //         break;
    //     }
    //     if (FALSE == adm_oca_registerSetMqttConnectTopicNameHandler(adm_oca_setMqttConnectTopicName)) {
    //         log_e("adm_oca_registerSetMqttConnectTopicTypeHandler failed");
    //         bRet = FALSE;
    //         break;
    //     }
    //     if (FALSE == adm_oca_registerSetConnectOutputApnHandler(adm_oca_setConnectOutputApn)) {
    //         log_e("adm_oca_registerSetConnectOutputApnHandler failed");
    //         bRet = FALSE;
    //         break;
    //     }
    //     if (FALSE == adm_oca_registerRequestConnectServerHandler(adm_oca_requestConnectServer)) {
    //         log_e("adm_oca_registerRequestConnectServerHandler failed");
    //         bRet = FALSE;
    //         break;
    //     }
    //     if (FALSE == adm_oca_registerConnectInitHandler(adm_oca_connectInit)) {
    //         log_e("adm_oca_registerConnectInitHandler failed");
    //         bRet = FALSE;
    //         break;
    //     }
    //     if (FALSE == adm_oca_registerConnectDeinitHandler(adm_oca_connectDeinit)) {
    //         log_e("adm_oca_registerConnectDeinitHandler failed");
    //         bRet = FALSE;
    //         break;
    //     }

    //     if (FALSE == adm_oca_registerPauseDownloadHandler(adm_oca_pauseDownload)) {
    //         log_e("adm_oca_registerPauseDownloadHandler failed");
    //         bRet = FALSE;
    //         break;
    //     }
    //     if (FALSE == adm_oca_registerCancelDownloadHandler(adm_oca_cancelDownload)) {
    //         log_e("adm_oca_registerCancelDownloadHandler failed");
    //         bRet = FALSE;
    //         break;
    //     }
    //     if (FALSE == adm_oca_registerStartDownloadHandler(adm_oca_startDownload)) {
    //         log_e("adm_oca_registerStartDownloadHandler failed");
    //         bRet = FALSE;
    //         break;
    //     }
    //     if (FALSE == adm_oca_registerNewDownloadInfoHandler(adm_oca_newDownloadInfo)) {
    //         log_e("adm_oca_registerNewDownloadInfoHandler failed");
    //         bRet = FALSE;
    //         break;
    //     }
    //     if (FALSE == adm_oca_registerDelDownloadInfoHandler(adm_oca_delDownloadInfo)) {
    //         log_e("adm_oca_registerDelDownloadInfoHandler failed");
    //         bRet = FALSE;
    //         break;
    //     }
    //     if (FALSE == adm_oca_registerFileUploadInitHandler(adm_oca_fileUploadInit)) {
    //         log_e("adm_oca_registerFileUploadInitHandler failed");
    //         bRet = FALSE;
    //         break;
    //     }
    //     if (FALSE == adm_oca_registerFileUploadDeinitHandler(adm_oca_fileUploadDeinit)) {
    //         log_e("adm_oca_registerFileUploadDeinitHandler failed");
    //         bRet = FALSE;
    //         break;
    //     }
    //     if (FALSE == adm_oca_registerAddSectionToHttpPostHandler(adm_oca_addSectionToHttpPost)) {
    //         log_e("adm_oca_registerAddSectionToHttpPostHandler failed");
    //         bRet = FALSE;
    //         break;
    //     }
    //     if (FALSE == adm_oca_registerNewUploadInfoHandler(adm_oca_newUploadInfo)) {
    //         log_e("adm_oca_registerNewUploadInfoHandler failed");
    //         bRet = FALSE;
    //         break;
    //     }
    //     if (FALSE == adm_oca_registerDelUploadInfoHandler(adm_oca_delUploadInfo)) {
    //         log_e("adm_oca_registerDelUploadInfoHandler failed");
    //         bRet = FALSE;
    //         break;
    //     }
    //     if (FALSE == adm_oca_registerStartUploadHandler(adm_oca_startUpload)) {
    //         log_e("adm_oca_registerStartUploadHandler failed");
    //         bRet = FALSE;
    //         break;
    //     }
    // } while(0);

    // do {
    //     connectHandler = adm_oca_connectInit();
        
    //     if (NULL == connectHandler) {
    //         iRet = -1;
    //         log_e("adm_oca_connectInit failed");
    //         break;
    //     }

    //     adm_oca_setConnectProtocolType(connectHandler, ADM_OCA_CONNECT_PROTOCOL_TYPE_HTTP);
    //     adm_oca_setConnectAddress(connectHandler, ADM_OCA_CONNECT_ADDRESS_TYPE_URL, "http://172.16.30.56:9999/register");
    //     adm_oca_requestConnectServer(connectHandler, "xxx", &responseBody, &statusCode);
    //     log_i("responseBody: %s", responseBody);
    //     adm_oca_connectDeinit(connectHandler);
    //     if (NULL != responseBody) {
    //         adm_free(responseBody);
    //     }
    // } while(0);

    do {
        iRet = pthread_create(&tid1, NULL, downloadHandler, NULL);
        if (0 != iRet) {
            log_e( "pthread_create failed, reason of error code is %d", iRet);
            iRet = -1;
        } else {
            pthread_detach(tid1);
        }
    } while(0);


    // do {
    //     iRet = pthread_create(&tid2, NULL, uploadHandler, NULL);
    //     if (0 != iRet) {
    //         log_e( "pthread_create failed, reason of error code is %d", iRet);
    //         iRet = -1;
    //     } else {
    //         pthread_detach(tid2);
    //     }
    // } while(0);

    while(1) {
        sleep(3);
    }

    return iRet;
}
