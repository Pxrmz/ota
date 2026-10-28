
#include "adm_cvs_checkVersion_unittest.h"
#include "adm_vrs_registerManager.h"
#include <stdio.h>


CCVSCheckVersionTest::CCVSCheckVersionTest()
{
    
}

CCVSCheckVersionTest::~CCVSCheckVersionTest()
{
    
}

D32S CCVSCheckVersionTest::adm_businessComm_callBack(const ADM_OCS_BUSINESS_TYPE_E eBusinessType, const DCHAR* reqMessageBody, DCHAR** rspMessageBody, DSIZE* rspStatusCode)
{
/*    
    DCHAR* pMessage =
    "{\
        \"msg\":\"�������°汾����ʽ����δ�����κ�����\",\
        \"sign\":\"42129777eb6a6b11a45a203981a734226804c205ceb3a856cf650ab1a51178c5\",\
        \"status\":2001,\
        \"timestamp\":1629186419\
    }";
*/
    //printf("CCVSCheckVersionTest::adm_businessComm_callBack\r\n");
    FILE* fp = fopen("./check_resp.txt","r+");
    if(NULL == fp)
    {
        return -1;
    }
    fseek(fp,0,SEEK_END);
    long lSize = ftell(fp);
    DCHAR* pMessage = (DCHAR*)adm_malloc(lSize*sizeof(char));
    fseek(fp,0,SEEK_SET);
    int iRead = fread(pMessage,sizeof(char),lSize,fp);
    fclose(fp);
    //printf("pMessage = %s\r\n",pMessage);
    if(rspMessageBody != NULL)
    {
        *rspMessageBody = (DCHAR*)adm_malloc(strlen(pMessage)+1);
        adm_memset(*rspMessageBody,0,strlen(pMessage)+1,strlen(pMessage)+1);
        adm_memcpy(*rspMessageBody,pMessage,strlen(pMessage),strlen(pMessage));
    }
    else
    {
        rspMessageBody = (DCHAR**)adm_malloc(sizeof(DCHAR*));
        adm_memset(rspMessageBody,0,sizeof(sizeof(DCHAR*)),sizeof(sizeof(DCHAR*)));
        *rspMessageBody = (DCHAR*)adm_malloc(strlen(pMessage)+1);
        adm_memset(*rspMessageBody,0,strlen(pMessage)+1,strlen(pMessage)+1);
        adm_memcpy(*rspMessageBody,pMessage,strlen(pMessage),strlen(pMessage));        
    }
    (*rspStatusCode) = 1000;
    return 0;
}

D32S CCVSCheckVersionTest::adm_register_callBack(const ADM_OCS_BUSINESS_TYPE_E eBusinessType, 
                           const DCHAR* reqMessageBody, 
                           DCHAR** rspMessageBody, 
                           DSIZE* rspStatusCode)
{
    DCHAR* pResponseBody = 
    "{\
        \"status\": 1000,\
        \"msg\": \"�ɹ�\",\
        \"sign\": \"89f70c65fccf5b083738b62d163201be9137ee41679ebebab9325c6e131fb233\",\
        \"timestamp\": 1626056371,\
        \"data\": {\
            \"deviceId\": \"e0984588053b2873c6c5aebdac6840bd8b66a83ac83db0911846dfe13d9e39c9\",\
            \"secret\": \"0d5fdcf89147040a5cbc5257c2b89529\"\
        }\
    }";
    
    *rspMessageBody = (DCHAR*)adm_malloc(strlen(pResponseBody)+1);
    adm_memset(*rspMessageBody,0,strlen(pResponseBody)+1,strlen(pResponseBody)+1);
    
    adm_memcpy(*rspMessageBody,pResponseBody,strlen(pResponseBody),strlen(pResponseBody));
    
    *rspStatusCode = 1000;
    return 0;
}

void CCVSCheckVersionTest::checkProgressNotifyHmi_F(const ADM_CVS_COMMON_CMDID_TYPE_E eCmdidTYpe,
                                                 const DCHAR* state,
                                                 const ADM_CVS_COMMON_CHECK_PROGRESS_TYPE_T eCheckProgressType)
{
    return;
}

void CCVSCheckVersionTest::checkProgressUpload_F(const DCHAR* state, const DCHAR* reason)
{
    return;
}

void CCVSCheckVersionTest::adm_cvs_checkVersion_Test()
{
    adm_core_service_initRuntimeDir(ADM_CORE_SERVICE_RUNTIME_DIR_TYPE_KV_STORAGE,"./");
    adm_core_service_init();
    ADM_OCS_REGISTER_REQUEST_INFO_T reqInfo;

    D32S cmdId = 1;
    D64S timestamp = time(NULL);
    DCHAR* netType = "WiFi";
    DCHAR* versionNumber="V1.1.1";
    DCHAR* sdkVersion = "v5.0";
    DCHAR* confVersion="2021-06-02 15:05:30";
    DCHAR* vim = "AX40JX12345678901";
    //!< ���ע��������Ϣ
    reqInfo.sdkVersion = sdkVersion;
    reqInfo.fnBusinessCommCb = &(CCVSCheckVersionTest::adm_register_callBack);
    reqInfo.timeStamp = time(NULL);
    reqInfo.vinCode = vim;
    reqInfo.ext = NULL;

    ADM_VRS_REGISTER_RSP_INFO_T pstVrsResponseInfo;
    D32S iRegister = adm_vrs_doRegister(&reqInfo,&pstVrsResponseInfo);
    

    DCHAR* allEcuInfoObjectStr =
    "{\
    \"ecus\":\
    [\
        {\
            \"ecuReqId\": \"001\",\
            \"ecuSwDid\": \"0xD001\",\
            \"ecuDids\":\
            [\
                {\
                    \"did\": \"0xF187\",\
                    \"didValue\": \"pn001\",\
                    \"start\": 0,\
                    \"end\": 13\
                },\
                {\
                    \"did\": \"0xF188\",\
                    \"didValue\": \"v1.0\",\
                    \"start\": 0,\
                    \"end\": 13\
                },\
                {\
                    \"did\": \"0xF18B\",\
                    \"didValue\": \"pro001\",\
                    \"start\": 0,\
                    \"end\": 13\
                }\
            ]\
        }\
    ]\
    }";

    ADM_OCS_CHECK_REQ_ADAPTER_T CheckReq;
    CheckReq.cmdId = cmdId;
    CheckReq.timestamp = timestamp;
    CheckReq.netType = netType;
    CheckReq.versionNumber = versionNumber;
    CheckReq.sdkVersion = sdkVersion;
    CheckReq.confVersion = confVersion;
    CheckReq.allEcuInfoObjectStr = allEcuInfoObjectStr;
    CheckReq.extsObjectStr = NULL;
    CheckReq.fnBusinessCommCb = &(CCVSCheckVersionTest::adm_businessComm_callBack);

    ADM_CVS_NOTIFY_T notifyF;
    notifyF.progressNotifyHmi = checkProgressNotifyHmi_F;
    notifyF.stateReportCloud = checkProgressUpload_F;
    ADM_OCS_CHECK_RESP_ADAPTER_T checkResp;
    D32S iret = adm_cvs_checkVersion(&CheckReq,&notifyF,&checkResp);
    EXPECT_EQ(0,iret);
    ASSERT_NE(nullptr,checkResp.sessionId);
    EXPECT_STREQ("16396420260384dxb4tyt",checkResp.sessionId);
    ASSERT_NE(nullptr,checkResp.respMessageBodyStr);
    ASSERT_NE(nullptr,checkResp.hmiMember->hmiShowFileUrl);
    EXPECT_STREQ("http://otadown-dev.evergrandeauto.com/download/image/dzrEDmR4.zip",checkResp.hmiMember->hmiShowFileUrl);
    ASSERT_NE(nullptr,checkResp.hmiMember->hmiShowFileHash);
    EXPECT_STREQ("55ff732c05cf6f294315c74a47721d2b991db83b685f9de42cc5f83ac6bd8fbc",checkResp.hmiMember->hmiShowFileHash);
    EXPECT_EQ(323,checkResp.hmiMember->hmiShowFileSize);
    ASSERT_NE(nullptr,checkResp.hmiMember->vehicleVersion);
    EXPECT_STREQ("V3.9",checkResp.hmiMember->vehicleVersion);
    EXPECT_EQ(105706192,checkResp.hmiMember->totalSize);


    //ADM_CVS_VERSION_POLICY_T versionPolicy;
    //iret = adm_cvs_getVersionPolicy(&versionPolicy);
    //EXPECT_EQ(0,iret);
}

TEST_F(CCVSCheckVersionUnitTest, Normal)
{
    CCVSCheckVersionTest aa;
    aa.adm_cvs_checkVersion_Test();
}