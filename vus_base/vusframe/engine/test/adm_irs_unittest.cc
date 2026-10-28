
#include "adm_irs_unittest.h"

CIRSTest::CIRSTest()
{
    
}

CIRSTest::~CIRSTest()
{
    
}

D32S CIRSTest::Post(const ADM_OCS_BUSINESS_TYPE_E eBusinessType,
    const DCHAR* reqMessageBody,
    DCHAR** rspMessageBody,
    DSIZE* rspStatusCode)
{
    printf("eBusinessType = %d, reqMessageBody = %s\n", eBusinessType, reqMessageBody);
    return 0;
}

D32S CIRSTest::PostFile(ADM_OCA_FILE_UPLOAD_INFO_T* uploadInfo, DCHAR** rspMessageBody, ADM_OCS_UPLOADFILE_T* priv)
{
    printf("cmdId = %d, fileHash = %s\n", priv->cmdId, priv->fileHash);
    printf("fileSize = %lld, fileType = %d\n", priv->fileSize, priv->fileType);
    printf("fullFileNamePath = %s, isFileEncrypted = %d\n", priv->fullFileNamePath, priv->isFileEncrypted);
    printf("sdkVersion = %s, timestamp = %lld\n", priv->sdkVersion, priv->timestamp);
    return 0;
}

D32S CIRSTest::adm_apc_logInit(void)
{

    return 0;
}


void CIRSTest::adm_irs_addUploadFile_Test()
{
    adm_core_service_initRuntimeDir(ADM_CORE_SERVICE_RUNTIME_DIR_TYPE_KV_STORAGE, "./");
    adm_core_service_init();
    adm_apc_logInit();
    
    FILE* fp = fopen("./123.zip","a+");
    if(fp != NULL)
    {
        fclose(fp);
    }

    ADM_IRS_SESSION_T* session = adm_irs_new(CIRSTest::Post, CIRSTest::PostFile, nullptr);
    
    D32S iret = adm_irs_addUploadFile(session,
                                    NULL,
                                        0,
                                        0,
                                        0,
                                        0,
                                        0,
                                      "v2.22",
                                      "./123.zip",
                                      ADM_OCS_FILE_ENCRYPTED,
                                      "4856544",
                                      4088,
                                      ADM_OCS_UCLOG_FILE,
                                      ADM_OCS_CMDID_TSP,
                                      NULL,
                                      NULL);
    
    EXPECT_EQ(0,iret);
    return;
}

TEST_F(CIRS_ADD_UPLoadFileUnitTest, Normal)
{
    CIRSTest aa;
    aa.adm_irs_addUploadFile_Test();
}

void CIRSTest::adm_irs_vehicle_State_Test()
{
    adm_core_service_initRuntimeDir(ADM_CORE_SERVICE_RUNTIME_DIR_TYPE_KV_STORAGE, "./");
    adm_core_service_init();
    adm_apc_logInit();

    ADM_IRS_SESSION_T* session = adm_irs_new(CIRSTest::Post, CIRSTest::PostFile, nullptr);
    
    /*vehicle state*/
    DCHAR* generateMessageBody = NULL;
    adm_ocs_vehicleStateProtocolAdapter(
        123456, "v1.2", "456", 0, 0, 0, "Vehicle_Register_Prepare", "NA", "v3.12", NULL, &generateMessageBody);

    //printf("generateMessageBody= %s\n", generateMessageBody);
    D32S iret = adm_irs_addUploadMessage(session, ADM_IRS_UPLOAD_VEHICLE_STATE, generateMessageBody);
    EXPECT_EQ(0,iret);
    adm_free(generateMessageBody);
}

TEST_F(CIRS_VEHICLE_State_UnitTest, Normal)
{
    CIRSTest aa;
    aa.adm_irs_vehicle_State_Test();
}

void CIRSTest::adm_irs_vehicle_Progress_Test()
{
    adm_core_service_initRuntimeDir(ADM_CORE_SERVICE_RUNTIME_DIR_TYPE_KV_STORAGE, "./");
    adm_core_service_init();
    adm_apc_logInit();

    DCHAR* generateMessageBody = NULL;
    ADM_IRS_SESSION_T* session = adm_irs_new(CIRSTest::Post, CIRSTest::PostFile, nullptr);    
    /*progress*/
    ADM_OCS_ECU_PROGRESS_INFO_LIST_T* ecusProgressList = NULL;
    ecusProgressList = (ADM_OCS_ECU_PROGRESS_INFO_LIST_T*)adm_malloc(sizeof(ADM_OCS_ECU_PROGRESS_INFO_LIST_T));
    adm_memset(ecusProgressList,0,sizeof(ADM_OCS_ECU_PROGRESS_INFO_LIST_T),sizeof(ADM_OCS_ECU_PROGRESS_INFO_LIST_T));
    ecusProgressList->reportEcusLen = 2;
    ecusProgressList->reportEcus = (ADM_OCS_ECU_PROGRESS_INFO_T*)adm_malloc(
        sizeof(ADM_OCS_ECU_PROGRESS_INFO_T) * ecusProgressList->reportEcusLen);
    adm_memset(ecusProgressList->reportEcus,0,\
               sizeof(ADM_OCS_ECU_PROGRESS_INFO_T) * ecusProgressList->reportEcusLen,\
               sizeof(ADM_OCS_ECU_PROGRESS_INFO_T) * ecusProgressList->reportEcusLen);

    ecusProgressList->reportEcus[0].ecuReqId = strdup("ecuReqId");
    ecusProgressList->reportEcus[0].ecuSwDid = strdup("ecuSwDid");
    ecusProgressList->reportEcus[0].progressType = strdup("1");
    ecusProgressList->reportEcus[0].progress = 80;

    ecusProgressList->reportEcus[1].ecuReqId = strdup("ecuReqId");
    ecusProgressList->reportEcus[1].ecuSwDid = strdup("ecuSwDid");
    ecusProgressList->reportEcus[1].progressType = strdup("1");
    ecusProgressList->reportEcus[1].progress = 90;

    adm_ocs_vehicleProgressProtocolAdapter("456", 0, 0, 123456, "v1.2", 3, NULL, ecusProgressList, &generateMessageBody);
    D32S iret = adm_irs_addUploadMessage(session, ADM_IRS_UPLOAD_PROGRESS, generateMessageBody);
    EXPECT_EQ(0,iret);
    
    adm_free(generateMessageBody);    
}

TEST_F(CIRS_VEHICLE_Progress_UnitTest, Normal)
{
    CIRSTest aa;
    aa.adm_irs_vehicle_Progress_Test();
}

void CIRSTest::adm_irs_vehicle_Event_Test()
{
    adm_core_service_initRuntimeDir(ADM_CORE_SERVICE_RUNTIME_DIR_TYPE_KV_STORAGE, "./");
    adm_core_service_init();
    adm_apc_logInit();

    DCHAR* generateMessageBody = NULL;
    ADM_IRS_SESSION_T* session = adm_irs_new(CIRSTest::Post, CIRSTest::PostFile, nullptr);    
    
    /*events*/
    ADM_OCS_EVENTS_INFO_LIST_T* eventsList = NULL;
    eventsList = (ADM_OCS_EVENTS_INFO_LIST_T*)adm_malloc(sizeof(ADM_OCS_EVENTS_INFO_LIST_T));
    adm_memset(eventsList,0,sizeof(ADM_OCS_EVENTS_INFO_LIST_T),\
               sizeof(ADM_OCS_EVENTS_INFO_LIST_T));
    eventsList->reportEventsLen = 2;
    eventsList->reportEvents
        = (ADM_OCS_EVENTS_INFO_T*)adm_malloc(sizeof(ADM_OCS_EVENTS_INFO_T) * eventsList->reportEventsLen);
    adm_memset(eventsList->reportEvents,0,sizeof(ADM_OCS_EVENTS_INFO_T) * eventsList->reportEventsLen,\
               sizeof(ADM_OCS_EVENTS_INFO_T) * eventsList->reportEventsLen);
    //ADM_OCS_ECU_ACCESS_INFO_T* ecuAccessInfo; /*!< �Ǳ��û����NULL Ecu ��Ϣ eventType Ϊ 2 ʱ���� */
    DCHAR* eventDesc; /*!< �¼����� */
    D64S eventTime; /*!< �¼�����ʱ��ʱ�������ȷ���� */

    //eventsList->reportEvents[0].eventType = 1;
    eventsList->reportEvents[0].eventSequenceId = 2;
    eventsList->reportEvents[0].eventId = strdup("eventId");
    eventsList->reportEvents[0].eventLevel = 1;
    eventsList->reportEvents[0].eventDesc = strdup("eventDesc");
    eventsList->reportEvents[0].eventTime = 6;

    // eventsList->reportEvents[0].ecuAccessInfo
    //     = (ADM_OCS_ECU_ACCESS_INFO_T*)adm_malloc(sizeof(ADM_OCS_ECU_ACCESS_INFO_T));
    // eventsList->reportEvents[0].ecuAccessInfo->ecuReqId = strdup("ecuReqId");
    // eventsList->reportEvents[0].ecuAccessInfo->ecuSwDid = strdup("ecuSwDid");

    // eventsList->reportEvents[0].ecuAccessInfo->ecuDidLists
    //     = (ADM_OCS_ECU_DID_LIST_T*)adm_malloc(sizeof(ADM_OCS_ECU_DID_LIST_T));
    // eventsList->reportEvents[0].ecuAccessInfo->ecuDidLists->ecuDidsLen = 1;

    // eventsList->reportEvents[0].ecuAccessInfo->ecuDidLists->ecuDids = (ADM_OCS_ECU_DID_T*)adm_malloc(
    //     sizeof(ADM_OCS_ECU_DID_T) * eventsList->reportEvents[0].ecuAccessInfo->ecuDidLists->ecuDidsLen);

    // eventsList->reportEvents[0].ecuAccessInfo->ecuDidLists->ecuDids[0].did = strdup("0xF188");
    // eventsList->reportEvents[0].ecuAccessInfo->ecuDidLists->ecuDids[0].didValue = strdup("didValue");

    // eventsList->reportEvents[0].ecuAccessInfo->ecuDidLists->ecuDids[0].start = 0;
    // eventsList->reportEvents[0].ecuAccessInfo->ecuDidLists->ecuDids[0].end = 13;

    //eventsList->reportEvents[1].eventType = 1;
    eventsList->reportEvents[1].eventSequenceId = 2;
    eventsList->reportEvents[1].eventId = strdup("eventId");
    eventsList->reportEvents[1].eventLevel = 1;
    eventsList->reportEvents[1].eventDesc = strdup("eventDesc");
    eventsList->reportEvents[1].eventTime = 6;

    // eventsList->reportEvents[1].ecuAccessInfo
    //     = (ADM_OCS_ECU_ACCESS_INFO_T*)adm_malloc(sizeof(ADM_OCS_ECU_ACCESS_INFO_T));
    // eventsList->reportEvents[1].ecuAccessInfo->ecuReqId = strdup("ecuReqId");
    // eventsList->reportEvents[1].ecuAccessInfo->ecuSwDid = strdup("ecuSwDid");

    // eventsList->reportEvents[1].ecuAccessInfo->ecuDidLists
    //     = (ADM_OCS_ECU_DID_LIST_T*)adm_malloc(sizeof(ADM_OCS_ECU_DID_LIST_T));
    // eventsList->reportEvents[1].ecuAccessInfo->ecuDidLists->ecuDidsLen = 1;

    // eventsList->reportEvents[1].ecuAccessInfo->ecuDidLists->ecuDids = (ADM_OCS_ECU_DID_T*)adm_malloc(
    //     sizeof(ADM_OCS_ECU_DID_T) * eventsList->reportEvents[0].ecuAccessInfo->ecuDidLists->ecuDidsLen);
    // eventsList->reportEvents[1].ecuAccessInfo->ecuDidLists->ecuDids[0].did = strdup("0xF188");
    // eventsList->reportEvents[1].ecuAccessInfo->ecuDidLists->ecuDids[0].didValue = strdup("didValue");
    // eventsList->reportEvents[1].ecuAccessInfo->ecuDidLists->ecuDids[0].start = 0;
    // eventsList->reportEvents[1].ecuAccessInfo->ecuDidLists->ecuDids[0].end = 13;

    adm_ocs_eventProtocolAdapter("456", 0, 0, 123456, "v1.2", NULL, eventsList, &generateMessageBody);
    D32S iret = adm_irs_addUploadMessage(session, ADM_IRS_UPLOAD_EVENT, generateMessageBody);
    EXPECT_EQ(0,iret);
    adm_free(generateMessageBody);      
}

TEST_F(CIRS_VEHICLE_Event_UnitTest, Normal)
{
    CIRSTest aa;
    aa.adm_irs_vehicle_Event_Test();
}
