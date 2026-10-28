#include "adm_iss_textRsa2048.h"
#include "gtest/gtest.h"
#include "json/json.h"
#include "adm_test_tool.h"
#include "adm_string.h"
#include "adm_memory.h"

namespace{

const DCHAR* m_configPath = "./adm_iss_rsa_config.json";

class IssRsa : public testing::Test {
protected:
  void SetUp() override {
      adm_test_tool_readConfig(&root_, m_configPath);
  }
  void TearDown() override {
  }

  void read(const std::string & caseName){
      in_ = root_[caseName]["in"].asString();
      out_ = root_[caseName]["out"].asString();
      pubkey_ = root_["iss_rsa_pubkey"].asString();
      prikey_ = root_["iss_rsa_prikey"].asString();
  }

  void rsa_decrypt(const std::string & caseName, D32S pad_mode){
      read(caseName);
      D8U* resOut = adm_iss_rsaPriDecrypt((D8U*)prikey_.c_str(), (D8U*)in_.c_str(), pad_mode);
      ASSERT_NE(nullptr, resOut);
      EXPECT_NE(0, adm_strlen((DCHAR*)resOut));
      EXPECT_STREQ(out_.c_str(), (DCHAR*)resOut);
      adm_free(resOut);
  }

  void rsa_encrypt_decrypt(const std::string & caseName, D32S pad_mode){
      read(caseName);
      ASSERT_NE(0, in_.length());
      D8U* enc = adm_iss_rsaPubEncrypt((D8U*)pubkey_.c_str(), (D8U*)in_.c_str(), pad_mode);
      ASSERT_NE(nullptr, enc);
      D8U* replain = adm_iss_rsaPriDecrypt((D8U*)prikey_.c_str(), enc, pad_mode);
      ASSERT_NE(nullptr, replain);
      EXPECT_STREQ(in_.c_str(), (DCHAR*)replain);
      adm_free(replain);
      adm_free(enc);
  }

  void rsa_verify(const std::string & caseName, D32S pad_mode){
      read(caseName);
      D8U* resOut = adm_iss_rsaPubDecrypt((D8U*)pubkey_.c_str(), (D8U*)in_.c_str(), pad_mode);
      ASSERT_NE(nullptr, resOut);
      EXPECT_NE(0, adm_strlen((DCHAR*)resOut));
      EXPECT_STREQ(out_.c_str(), (DCHAR*)resOut);
      adm_free(resOut);
  }

  void rsa_sign_verify(const std::string & caseName, D32S pad_mode){
      read(caseName);
      ASSERT_NE(0, in_.length());
      D8U* sign = adm_iss_rsaPriEncrypt((D8U*)prikey_.c_str(), (D8U*)in_.c_str(), pad_mode);
      ASSERT_NE(nullptr, sign);
      D8U* replain = adm_iss_rsaPubDecrypt((D8U*)pubkey_.c_str(), sign, pad_mode);
      ASSERT_NE(nullptr, replain);
      EXPECT_STREQ(in_.c_str(), (DCHAR*)replain);
      adm_free(replain);
      adm_free(sign);
  }

  Json::Value root_;
  std::string in_;
  std::string out_;
  std::string pubkey_;
  std::string prikey_;
};


TEST_F(IssRsa, iss_Decrypt_PKCS1_PADDING){
    rsa_decrypt("iss_Decrypt_PKCS1_PADDING", RSA_PKCS1_PADDING);
}

TEST_F(IssRsa, iss_Decrypt_NO_PADDING){
    rsa_decrypt("iss_Decrypt_NO_PADDING", RSA_NO_PADDING);
}

TEST_F(IssRsa, iss_Decrypt_PKCS1_OAEP_PADDING){
    rsa_decrypt("iss_Decrypt_PKCS1_OAEP_PADDING", RSA_PKCS1_OAEP_PADDING);
}

TEST_F(IssRsa, iss_Encrypt_Decrypt_PKCS1_PADDING){
    rsa_encrypt_decrypt("iss_Encrypt_Decrypt_PKCS1_PADDING", RSA_PKCS1_PADDING);
}

/* TODO adm_iss_rsaPubEncrypt bug */
//TEST_F(IssRsa, iss_Encrypt_Decrypt_NO_PADDING){
//    rsa_encrypt_decrypt("iss_Encrypt_Decrypt_NO_PADDING", RSA_NO_PADDING);
//}

TEST_F(IssRsa, iss_Encrypt_Decrypt_PKCS1_OAEP_PADDING){
    rsa_encrypt_decrypt("iss_Encrypt_Decrypt_PKCS1_OAEP_PADDING", RSA_PKCS1_OAEP_PADDING);
}

TEST_F(IssRsa, iss_Verify_PKCS1_PADDING){
    rsa_verify("iss_Verify_PKCS1_PADDING", RSA_PKCS1_PADDING);
}

TEST_F(IssRsa, iss_Verify_NO_PADDING){
    rsa_verify("iss_Verify_NO_PADDING", RSA_NO_PADDING);
}

TEST_F(IssRsa, iss_Sign_Verify_PKCS1_PADDING){
    rsa_sign_verify("iss_Sign_Verify_PKCS1_PADDING", RSA_PKCS1_PADDING);
}

/* TODO adm_iss_rsaPriEncrypt bug */
//TEST_F(IssRsa, iss_Sign_Verify_NO_PADDING){
//    rsa_sign_verify("iss_Sign_Verify_NO_PADDING", RSA_NO_PADDING);
//}







}
