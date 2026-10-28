#include "adm_iss_hmacSha2.h"
#include "gtest/gtest.h"
#include "json/json.h"
#include "adm_test_tool.h"
#include "adm_string.h"
#include "adm_memory.h"


namespace{

const DCHAR* m_configPath = "./adm_iss_hmac_config.json";

class IssHmac : public testing::Test {
protected:
  void SetUp() override {
      adm_test_tool_readConfig(&root_, m_configPath);
  }
  void TearDown() override {
  }

  void read(const std::string & caseName){
      in_ = root_[caseName]["in"].asString();
      out_ = root_[caseName]["out"].asString();
      key_ = root_[caseName]["key"].asString();
  }

  Json::Value root_;
  std::string in_;
  std::string out_;
  std::string key_;
};

TEST_F(IssHmac, iss_hmac_sha224){
    read("iss_hmac_sha224");
    DCHAR* resOut = adm_iss_hmacSha224((D8U*)key_.c_str(), key_.length(), (D8U*)in_.c_str(), in_.length());
    ASSERT_NE(nullptr, resOut);
    EXPECT_NE(0, adm_strlen(resOut));
    EXPECT_STREQ(out_.c_str(), resOut);
    adm_free(resOut);
}

TEST_F(IssHmac, iss_hmac_sha256){
    read("iss_hmac_sha256");
    DCHAR* resOut = adm_iss_hmacSha256((D8U*)key_.c_str(), key_.length(), (D8U*)in_.c_str(), in_.length());
    ASSERT_NE(nullptr, resOut);
    EXPECT_NE(0, adm_strlen(resOut));
    EXPECT_STREQ(out_.c_str(), resOut);
    adm_free(resOut);
}

TEST_F(IssHmac, iss_hmac_sha384){
    read("iss_hmac_sha384");
    DCHAR* resOut = adm_iss_hmacSha384((D8U*)key_.c_str(), key_.length(), (D8U*)in_.c_str(), in_.length());
    ASSERT_NE(nullptr, resOut);
    EXPECT_NE(0, adm_strlen(resOut));
    EXPECT_STREQ(out_.c_str(), resOut);
    adm_free(resOut);
}

TEST_F(IssHmac, iss_hmac_sha512){
    read("iss_hmac_sha512");
    DCHAR* resOut = adm_iss_hmacSha512((D8U*)key_.c_str(), key_.length(), (D8U*)in_.c_str(), in_.length());
    ASSERT_NE(nullptr, resOut);
    EXPECT_NE(0, adm_strlen(resOut));
    EXPECT_STREQ(out_.c_str(), resOut);
    adm_free(resOut);
}



}
