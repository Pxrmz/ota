#include "adm_ocs_airProtocol_Adapter_unittest.h"
#include "adm_iss_textRsa2048.h"
#include "adm_ocs_airProtocolSecurity.h"

COCSAirProtocolAdapterTest::COCSAirProtocolAdapterTest()
{
    
}

COCSAirProtocolAdapterTest::~COCSAirProtocolAdapterTest()
{
    
}

D32S COCSAirProtocolAdapterTest::adm_ocs_registerProtocolAdapter_Test_CallBack(const ADM_OCS_BUSINESS_TYPE_E eBusinessType, 
                                                          const DCHAR* reqMessageBody, 
                                                          DCHAR** rspMessageBody, 
                                                          DSIZE* rspStatusCode)
{
    printf("ok here\r\n");
    return 0;
}

void COCSAirProtocolAdapterTest::adm_ocs_registerProtocolAdapter_Test()
{
    ADM_OCS_REGISTER_REQUEST_INFO_T* pstRegisterRequestInfo = (ADM_OCS_REGISTER_REQUEST_INFO_T*)adm_malloc(sizeof(ADM_OCS_REGISTER_REQUEST_INFO_T));
    adm_memset(pstRegisterRequestInfo,0,sizeof(ADM_OCS_REGISTER_REQUEST_INFO_T),sizeof(ADM_OCS_REGISTER_REQUEST_INFO_T));
    pstRegisterRequestInfo->vinCode = "Arknights00000004";
    pstRegisterRequestInfo->sdkVersion = "v1.0";
    pstRegisterRequestInfo->timeStamp=1618902541;
    pstRegisterRequestInfo->ext=NULL;
    pstRegisterRequestInfo->fnBusinessCommCb=&(COCSAirProtocolAdapterTest::adm_ocs_registerProtocolAdapter_Test_CallBack);

    DCHAR* pSignInfo=NULL;
    pSignInfo = adm_ocs_registerProtocolAdapter(pstRegisterRequestInfo,pstRegisterRequestInfo->vinCode);
    ASSERT_NE(nullptr,pSignInfo);
    
    return;
}

TEST_F(COCSAirProtocolAdapterUnitTest, Normal)
{
    COCSAirProtocolAdapterTest aa;
    aa.adm_ocs_registerProtocolAdapter_Test();
}

void COCSAirProtocolAdapterTest::adm_ocs_registerProtocolParse_Test()
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
    D32S statusCode = 1000;
    DCHAR* secret = "Arknights00000004";
    ADM_OCS_REGISTER_RESPONSE_INFO_T pstRegisterResponseInfo;
    D32S iret = adm_ocs_registerProtocolParse(pResponseBody,statusCode,secret,&pstRegisterResponseInfo);
    EXPECT_EQ(0,iret);
    ASSERT_NE(nullptr,pstRegisterResponseInfo.deviceId);
    EXPECT_STREQ("e0984588053b2873c6c5aebdac6840bd8b66a83ac83db0911846dfe13d9e39c9",pstRegisterResponseInfo.deviceId);
    ASSERT_NE(nullptr,pstRegisterResponseInfo.secret);
    EXPECT_STREQ("0d5fdcf89147040a5cbc5257c2b89529",pstRegisterResponseInfo.secret);
}

TEST_F(COCSAirProtocolAdapterParserUnitTest, Normal)
{
    COCSAirProtocolAdapterTest aa;
    aa.adm_ocs_registerProtocolParse_Test();
}

void COCSAirProtocolAdapterTest::adm_ocs_checkProtocolAdapter_Test()
{
    D32S cmdId = 1;
    D64S timestamp = time(NULL);
    DCHAR* netType = "WiFi";
    DCHAR* versionNumber="V1.1.1";
    DCHAR* sdkVersion = "v5.0";
    DCHAR* confVersion="2021-06-02 15:05:30";
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
    CheckReq.fnBusinessCommCb = NULL;
    
    // ADM_OCS_SECURITY_IMPL_F_T securityImplF;
    
    // securityImplF.getSign_F = adm_ocs_ariProtocolSign;
    // securityImplF.enCrypt_F = adm_iss_rsaPubEncrypt;
    // securityImplF.deCrypt_F = adm_iss_rsaPubDecrypt;
    // securityImplF.deviceId  ="e0984588053b2873c6c5aebdac6840bd8b66a83ac83db0911846dfe13d9e39c9";
    // securityImplF.secret    ="0d5fdcf89147040a5cbc5257c2b89529";
    //DCHAR* adapterCheckReq = NULL;
    //D32S iret = adm_ocs_checkProtocolAdapter(&CheckReq,&securityImplF,&adapterCheckReq);
    // D32S iret = adm_ocs_checkProtocolAdapter(&CheckReq,&adapterCheckReq);
    // EXPECT_EQ(0,iret);
    // ASSERT_NE(nullptr,adapterCheckReq);
    
}

TEST_F(COCSCheckProtocolAdapterUnitTest, Normal)
{
    COCSAirProtocolAdapterTest aa;
    aa.adm_ocs_checkProtocolAdapter_Test();
}

void COCSAirProtocolAdapterTest::adm_ocs_checkProtocolParse_Test()
{
    DCHAR* respBodyFromCloud=
    "{\
        \"msg\":\"�������°汾����ʽ����δ�����κ�����\",\
        \"sign\":\"42129777eb6a6b11a45a203981a734226804c205ceb3a856cf650ab1a51178c5\",\
        \"status\":2001,\
        \"timestamp\":1629186419\
     }";
    
    ADM_OCS_CHECK_RESP_ADAPTER_T checkResp;

    // ADM_OCS_SECURITY_IMPL_F_T securityImplF;
    
    // securityImplF.getSign_F = adm_ocs_ariProtocolSign;
    // securityImplF.enCrypt_F = adm_iss_rsaPubEncrypt;
    // securityImplF.deCrypt_F = adm_iss_rsaPubDecrypt;
    // securityImplF.deviceId  ="e0984588053b2873c6c5aebdac6840bd8b66a83ac83db0911846dfe13d9e39c9";
    // securityImplF.secret    ="0d5fdcf89147040a5cbc5257c2b89529";    
    //D32S iret = adm_ocs_checkProtocolParse(respBodyFromCloud,&checkResp,&securityImplF);
    // D32S iret = adm_ocs_checkProtocolParse(respBodyFromCloud,&checkResp);
    // EXPECT_EQ(iret,812);
}

TEST_F(COCSCheckProtocolAdapterParserUnitTest, Normal)
{
    COCSAirProtocolAdapterTest aa;
    aa.adm_ocs_checkProtocolParse_Test();
}

void COCSAirProtocolAdapterTest::adm_ocs_otaStatusCodeTransfer_Test()
{
    D32S statusCode = ADM_OCS_AIR_STATUS_CODE_SUCCESSED;
    D32S iret = adm_ocs_otaStatusCodeTransfer(statusCode);
    EXPECT_EQ(iret,ADM_ECODE_SUCCESSED);
}

TEST_F(COSCOTAStatusCodeTransferUnitTest, Normal)
{
    COCSAirProtocolAdapterTest aa;
    aa.adm_ocs_otaStatusCodeTransfer_Test();
}