
#include "adm_ocs_air_protocol_unittest.h"

COCS_Air_Protocol::COCS_Air_Protocol()
{
    
}

COCS_Air_Protocol::~COCS_Air_Protocol()
{
    
}

void COCS_Air_Protocol::adm_ocs_ariProtocolSign_Test()
{
    const DCHAR* protocolStr = 
    "{\
            \"deviceId\": \"e0984588053b2873c6c5aebdac6840bd8b66a83ac83db0911846dfe13d9e39c9\",\
            \"secret\": \"0d5fdcf89147040a5cbc5257c2b89529\"\
    }";
    const DCHAR* secret = "0d5fdcf89147040a5cbc5257c2b89529";
    DCHAR deviceId[256]={0};
    DCHAR* pSigInfo = adm_ocs_ariProtocolSign(protocolStr,secret, deviceId);
    
    ASSERT_NE(nullptr,pSigInfo);
    EXPECT_STREQ("1ee3ab1eac537a4721e1326e05f8c5c53ded87d56c85a8f24f3409e54476e125",pSigInfo);
    return;
}

TEST_F(COCS_Air_Protocol_UnitTest, Normal)
{
    COCS_Air_Protocol aa;
    aa.adm_ocs_ariProtocolSign_Test();
}
