#include "adm_iss_textAes.h"
#include "gtest/gtest.h"
#include "json/json.h"
#include "adm_test_tool.h"
#include "adm_string.h"
#include "adm_memory.h"

namespace{

const DCHAR* m_configPath = "./adm_iss_aes_config.json";

constexpr D32U m_aes128KeyLength = 16;
constexpr D32U m_aes192KeyLength = 24;
constexpr D32U m_aes256KeyLength = 32;
constexpr D32U m_aesIvLength = 16;

class IssAes : public testing::Test {
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
      iv_ = root_[caseName]["iv"].asString();
  }

  void aes_ecb_encrypt(const std::string & caseName, D32U keyLen){
      read(caseName);
      ASSERT_EQ(keyLen, key_.length());
      DCHAR* resOut = adm_iss_AES_ECB_PKCS7_Encrypt(in_.c_str(), (D8U*)key_.c_str());
      ASSERT_NE(nullptr, resOut);
      EXPECT_NE(0, adm_strlen(resOut));
      EXPECT_STREQ(out_.c_str(), resOut);
      adm_free(resOut);
  }

  void aes_ecb_decrypt(const std::string & caseName, D32U keyLen){
      read(caseName);
      ASSERT_EQ(keyLen, key_.length());
      DCHAR* resOut = adm_iss_AES_ECB_PKCS7_Decrypt(in_.c_str(), (D8U*)key_.c_str());
      ASSERT_NE(nullptr, resOut);
      EXPECT_NE(0, adm_strlen(resOut));
      EXPECT_STREQ(out_.c_str(), resOut);
      adm_free(resOut);
  }

  void aes_ecb_encrypt_decrypt(const std::string & caseName, D32U keyLen){
      read(caseName);
      ASSERT_EQ(keyLen, key_.length());
      DCHAR* encrypt = adm_iss_AES_ECB_PKCS7_Encrypt(in_.c_str(), (D8U*)key_.c_str());
      ASSERT_NE(nullptr, encrypt);
      EXPECT_NE(0, adm_strlen(encrypt));
      DCHAR* decrypt = adm_iss_AES_ECB_PKCS7_Decrypt(encrypt, (D8U*)key_.c_str());
      ASSERT_NE(nullptr, decrypt);
      EXPECT_NE(0, adm_strlen(decrypt));
      EXPECT_STREQ(in_.c_str(), decrypt);
      adm_free(decrypt);
      adm_free(encrypt);
  }

  void aes_cbc_encrypt(const std::string & caseName, D32U keyLen){
      read(caseName);
      ASSERT_EQ(keyLen, key_.length());
      ASSERT_EQ(m_aesIvLength, iv_.length());
      DCHAR* resOut = adm_iss_AES_CBC_PKCS7_Encrypt(in_.c_str(), (D8U*)key_.c_str(), (D8U*)iv_.c_str());
      ASSERT_NE(nullptr, resOut);
      EXPECT_NE(0, adm_strlen(resOut));
      EXPECT_STREQ(out_.c_str(), resOut);
      adm_free(resOut);
  }

  void aes_cbc_decrypt(const std::string & caseName, D32U keyLen){
      read(caseName);
      ASSERT_EQ(keyLen, key_.length());
      ASSERT_EQ(m_aesIvLength, iv_.length());
      DCHAR* resOut = adm_iss_AES_CBC_PKCS7_Decrypt(in_.c_str(), (D8U*)key_.c_str(), (D8U*)iv_.c_str());
      ASSERT_NE(nullptr, resOut);
      EXPECT_NE(0, adm_strlen(resOut));
      EXPECT_STREQ(out_.c_str(), resOut);
      adm_free(resOut);
  }

  void aes_cbc_encrypt_decrypt(const std::string & caseName, D32U keyLen){
      read(caseName);
      ASSERT_EQ(keyLen, key_.length());
      ASSERT_EQ(m_aesIvLength, iv_.length());
      DCHAR* encrypt = adm_iss_AES_CBC_PKCS7_Encrypt(in_.c_str(), (D8U*)key_.c_str(), (D8U*)iv_.c_str());
      ASSERT_NE(nullptr, encrypt);
      EXPECT_NE(0, adm_strlen(encrypt));
      DCHAR* decrypt = adm_iss_AES_CBC_PKCS7_Decrypt(encrypt, (D8U*)key_.c_str(), (D8U*)iv_.c_str());
      ASSERT_NE(nullptr, decrypt);
      EXPECT_NE(0, adm_strlen(decrypt));
      EXPECT_STREQ(in_.c_str(), decrypt);
      adm_free(decrypt);
      adm_free(encrypt);
  }


  Json::Value root_;
  std::string in_;
  std::string out_;
  std::string key_;
  std::string iv_;
};

TEST_F(IssAes, iss_AES_128_ECB_PKCS7_Encrypt){
    aes_ecb_encrypt("iss_AES_128_ECB_PKCS7_Encrypt", m_aes128KeyLength);
}

TEST_F(IssAes, iss_AES_128_ECB_PKCS7_Decrypt){
    aes_ecb_decrypt("iss_AES_128_ECB_PKCS7_Decrypt", m_aes128KeyLength);
}

TEST_F(IssAes, iss_AES_192_ECB_PKCS7_Encrypt){
    aes_ecb_encrypt("iss_AES_192_ECB_PKCS7_Encrypt", m_aes192KeyLength);
}

TEST_F(IssAes, iss_AES_192_ECB_PKCS7_Decrypt){
    aes_ecb_decrypt("iss_AES_192_ECB_PKCS7_Decrypt", m_aes192KeyLength);
}

TEST_F(IssAes, iss_AES_256_ECB_PKCS7_Encrypt){
    aes_ecb_encrypt("iss_AES_256_ECB_PKCS7_Encrypt", m_aes256KeyLength);
}

TEST_F(IssAes, iss_AES_256_ECB_PKCS7_Decrypt){
    aes_ecb_decrypt("iss_AES_256_ECB_PKCS7_Decrypt", m_aes256KeyLength);
}

TEST_F(IssAes, iss_AES_128_ECB_PKCS7_Encrypt_Decrypt){
    aes_ecb_encrypt_decrypt("iss_AES_128_ECB_PKCS7_Encrypt_Decrypt", m_aes128KeyLength);
}

TEST_F(IssAes, iss_AES_192_ECB_PKCS7_Encrypt_Decrypt){
    aes_ecb_encrypt_decrypt("iss_AES_192_ECB_PKCS7_Encrypt_Decrypt", m_aes192KeyLength);
}

TEST_F(IssAes, iss_AES_256_ECB_PKCS7_Encrypt_Decrypt){
    aes_ecb_encrypt_decrypt("iss_AES_256_ECB_PKCS7_Encrypt_Decrypt", m_aes256KeyLength);
}

TEST_F(IssAes, iss_AES_128_CBC_PKCS7_Encrypt){
    aes_cbc_encrypt("iss_AES_128_CBC_PKCS7_Encrypt", m_aes128KeyLength);
}

TEST_F(IssAes, iss_AES_128_CBC_PKCS7_Decrypt){
    aes_cbc_decrypt("iss_AES_128_CBC_PKCS7_Decrypt", m_aes128KeyLength);
}

TEST_F(IssAes, iss_AES_192_CBC_PKCS7_Encrypt){
    aes_cbc_encrypt("iss_AES_192_CBC_PKCS7_Encrypt", m_aes192KeyLength);
}

TEST_F(IssAes, iss_AES_192_CBC_PKCS7_Decrypt){
    aes_cbc_decrypt("iss_AES_192_CBC_PKCS7_Decrypt", m_aes192KeyLength);
}

TEST_F(IssAes, iss_AES_256_CBC_PKCS7_Encrypt){
    aes_cbc_encrypt("iss_AES_256_CBC_PKCS7_Encrypt", m_aes256KeyLength);
}

TEST_F(IssAes, iss_AES_256_CBC_PKCS7_Decrypt){
    aes_cbc_decrypt("iss_AES_256_CBC_PKCS7_Decrypt", m_aes256KeyLength);
}

TEST_F(IssAes, iss_AES_128_CBC_PKCS7_Encrypt_Decrypt){
    aes_cbc_encrypt_decrypt("iss_AES_128_CBC_PKCS7_Encrypt_Decrypt", m_aes128KeyLength);
}

TEST_F(IssAes, iss_AES_192_CBC_PKCS7_Encrypt_Decrypt){
    aes_cbc_encrypt_decrypt("iss_AES_192_CBC_PKCS7_Encrypt_Decrypt", m_aes192KeyLength);
}

TEST_F(IssAes, iss_AES_256_CBC_PKCS7_Encrypt_Decrypt){
    aes_cbc_encrypt_decrypt("iss_AES_256_CBC_PKCS7_Encrypt_Decrypt", m_aes256KeyLength);
}


}
