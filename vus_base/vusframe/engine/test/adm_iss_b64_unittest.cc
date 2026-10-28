#include "adm_iss_base64.h"
#include "gtest/gtest.h"
#include "json/json.h"
#include "adm_test_tool.h"
#include "adm_typedefs.h"
#include "adm_string.h"
#include "adm_memory.h"

#include <string>
#include <memory>

namespace{

const DCHAR* m_configPath = "./adm_iss_b64_config.json";

TEST(iss_base64, iss_base64_encode_ascii){
    Json::Value root;
    adm_test_tool_readConfig(&root, m_configPath);
    Json::Value inVal = root["iss_base64_encode_ascii"]["in"];
    D32S len = inVal.size();
    ASSERT_NE(0, len);
    D8U *buff = new D8U[len];
    ASSERT_NE(nullptr, buff);
    for(D32S i =0; i<len; ++i){
	buff[i] = inVal[i].asInt();
    }
    const std::string out = root["iss_base64_encode_ascii"]["out"].asString();
    DCHAR *resBuff = adm_iss_b64Encode(buff, len);
    ASSERT_NE(nullptr, resBuff);
    EXPECT_STREQ(out.c_str(), resBuff);
    adm_free(resBuff);
    delete []buff;
}

TEST(iss_base64, iss_base64_encode_string){
    Json::Value root;
    adm_test_tool_readConfig(&root, m_configPath);
    const std::string in = root["iss_base64_encode_string"]["in"].asString();
    const std::string out = root["iss_base64_encode_string"]["out"].asString();
    DCHAR *resBuff = adm_iss_b64Encode((D8U*)(in.c_str()), in.length());
    ASSERT_NE(nullptr, resBuff);
    EXPECT_STREQ(out.c_str(), resBuff);
    adm_free(resBuff);
}

TEST(iss_base64, iss_base64_decode_ascii){
    Json::Value root;
    adm_test_tool_readConfig(&root, m_configPath);
    const std::string in = root["iss_base64_decode_ascii"]["in"].asString();
    DSIZE resBuffLen = 0;
    D8U* resBuff = adm_iss_b64Decode(in.c_str(), in.length(), &resBuffLen);
    ASSERT_NE(0, resBuffLen);
    ASSERT_NE(nullptr, resBuff);
    Json::Value outVal = root["iss_base64_decode_ascii"]["out"];
    ASSERT_EQ(resBuffLen, outVal.size());
    for(D32S i=0; i<resBuffLen; ++i){
	EXPECT_EQ(resBuff[i], outVal[i].asInt());
    }
    adm_free(resBuff);
}

TEST(iss_base64, iss_base64_decode_string){
    Json::Value root;
    adm_test_tool_readConfig(&root, m_configPath);
    const std::string in = root["iss_base64_decode_string"]["in"].asString();
    DSIZE resBuffLen = 0;
    D8U* resBuff = adm_iss_b64Decode(in.c_str(), in.length(), &resBuffLen);
    ASSERT_NE(0, resBuffLen);
    ASSERT_NE(nullptr, resBuff);
    const std::string out = root["iss_base64_decode_string"]["out"].asString();
    ASSERT_EQ(resBuffLen, out.length());
    EXPECT_EQ(0, adm_memcmp(resBuff, out.c_str(), resBuffLen));
    adm_free(resBuff);
}

TEST(iss_base64, iss_base64_encode_decode_ascii){
    Json::Value root;
    adm_test_tool_readConfig(&root, m_configPath);
    Json::Value inVal = root["iss_base64_encode_decode_ascii"];
    D32S len = inVal.size();
    ASSERT_NE(0, len);
    D8U *buff = new D8U[len];
    ASSERT_NE(nullptr, buff);
    for(D32S i =0; i<len; ++i){
	buff[i] = inVal[i].asInt();
    }
    DCHAR* enbuf = adm_iss_b64Encode(buff, len);
    ASSERT_NE(nullptr, enbuf);
    ASSERT_NE(0, adm_strlen(enbuf));
    DSIZE debufLen = 0;
    D8U* debuf = adm_iss_b64Decode (enbuf, adm_strlen(enbuf), &debufLen);
    ASSERT_NE(nullptr, debuf);
    EXPECT_EQ(len, debufLen);
    EXPECT_EQ(0, adm_memcmp(buff, debuf, len));
    adm_free(debuf);
    adm_free(enbuf);
    delete []buff;
}

TEST(iss_base64, iss_base64_encode_decode_string){
    Json::Value root;
    adm_test_tool_readConfig(&root, m_configPath);
    const std::string in = root["iss_base64_encode_decode_string"].asString();
    ASSERT_NE(0, in.length());
    DCHAR* enbuf = adm_iss_b64Encode((D8U*)(in.c_str()), in.length());
    ASSERT_NE(nullptr, enbuf);
    ASSERT_NE(0, adm_strlen(enbuf));
    DSIZE debufLen = 0;
    D8U* debuf = adm_iss_b64Decode (enbuf, adm_strlen(enbuf), &debufLen);
    ASSERT_NE(nullptr, debuf);
    EXPECT_EQ(in.length(), debufLen);
    EXPECT_EQ(0, adm_memcmp(in.c_str(), debuf, in.length()));
    adm_free(debuf);
    adm_free(enbuf);
}


}
