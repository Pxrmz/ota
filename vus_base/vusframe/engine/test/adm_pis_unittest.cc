#include "adm_pis_package.h"
#include "gtest/gtest.h"
#include "adm_memory.h"

#if 1

namespace {

const ADM_PIS_PACKAGE_HEAD_T g_head = {
    .ecuReqId = "ecuReqId",
    .ecuSwDid = "ecuSwDid",
    .type = ADM_PIS_PACKAGE_TYPE_ECU_DST_PACKAGE,
    .state = ADM_PIS_PACKAGE_STATE_DECRYPT_FINISH
};

const ADM_PIS_FILE_T g_imagePackage = {
    .path = "imagePackagePath",
    .hash = "imagePackageHash",
    .size = 100
};
const ADM_PIS_FILE_T g_signPackage = {
    .path = "signPackagePath",
    .hash = "signPackageHash",
    .size = 101
};

const ADM_COM_CHECKSUM_T g_CheckSums[] = {
    {
	.value = "value_1",
	.algorithm = 2,
    },
    {
	.value = "value_2",
	.algorithm = 3,
    }
};

const ADM_COM_FILEMETA_T g_fileMetas[] = {
    {
	.type = 1,
	.size = 528335,
	.stCheckSum = g_CheckSums[0]
    },
    {
	.type = 2,
	.size = 528336,
	.stCheckSum = g_CheckSums[1]
    }
};

const ADM_COM_FILESCYINFO_T g_fileSecurityInfo = {
    .oriHash = "oriHash",
    .pkgKey = "pkgKey",
    .isEncrypt = 1,
    .segmentDecryptLen = 1024,
    .oriSize = 528340,
    .fileMetaNum = 2,
    .pstFileMetaList = (ADM_COM_FILEMETA_T*)g_fileMetas
};

const ADM_PIS_PACKAGE_DATA_T g_data = {
    .imagePackage = g_imagePackage,
    .signPackage = (ADM_PIS_FILE_T*)(&g_signPackage),
    .fileSecurityInfo = (ADM_COM_FILESCYINFO_T*)(&g_fileSecurityInfo)
};

TEST(PisTest, normal){
    adm_pis_packageInit();
    adm_pis_packageImageSet(&g_head, &g_imagePackage);
    adm_pis_packageSignSet(&g_head, &g_signPackage);
    adm_pis_packageSecuritySet(&g_head, &g_fileSecurityInfo);
    ADM_PIS_PACKAGE_DATA_T data;
    adm_pis_packageDataInit(&data);
    ASSERT_NE(nullptr, adm_pis_packageGet(&g_head, &data));
    struct json_object* srcDataVal = adm_pis_packageDataJsonCreate(&g_data);
    struct json_object* dstDataVal = adm_pis_packageDataJsonCreate(&data);
    ASSERT_NE(nullptr, srcDataVal);
    ASSERT_NE(nullptr, dstDataVal);
    const DCHAR *srcDataString = json_object_to_json_string(srcDataVal);
    const DCHAR *dstDataString = json_object_to_json_string(dstDataVal);
    ASSERT_NE(nullptr, srcDataString);
    ASSERT_NE(nullptr, dstDataString);
    EXPECT_STREQ(srcDataString, dstDataString);
    json_object_put(srcDataVal);
    json_object_put(dstDataVal);
    adm_pis_packageDataDeInit(&data);
    adm_pis_packageDeInit();
}

}//namespace

#else

namespace {

class PisTest : public testing::Test {
protected:
  void SetUp() override {
      adm_pis_packageInit();

      head_.ecuReqId = (DCHAR*)ecuReqId_;
      head_.ecuSwDid = (DCHAR*)ecuSwDid_;
      head_.type = ADM_PIS_PACKAGE_TYPE_BEGIN;
      head_.state = ADM_PIS_PACKAGE_STATE_BEGIN;

      indata_.path = NULL;
      indata_.hash = NULL;
      indata_.size = 0;

  }
  void TearDown() override {
      adm_pis_packageDeInit();
  }

  void package_null(){
    adm_pis_packageDataInit(&outdata_);
    adm_pis_packageGet(&head_, &outdata_);
    EXPECT_EQ(nullptr, outdata_.path);
    EXPECT_EQ(nullptr, outdata_.hash);
    EXPECT_EQ(0, outdata_.size);
    adm_pis_packageDataDeInit(&outdata_);
  }

  void package_verify(const DCHAR* path, const DCHAR *hash, D64S size){
    indata_.path = (DCHAR*)path;
    indata_.hash = (DCHAR*)hash;
    indata_.size = size;
    adm_pis_packageSet(&head_, &indata_);
    adm_pis_packageDataInit(&outdata_);
    adm_pis_packageGet(&head_, &outdata_);
    ASSERT_NE(nullptr, outdata_.path);
    EXPECT_STREQ(path, outdata_.path);
    ASSERT_NE(nullptr, outdata_.hash);
    EXPECT_STREQ(hash, outdata_.hash);
    EXPECT_EQ(size, outdata_.size);
    adm_pis_packageDataDeInit(&outdata_);
  }

  ADM_PIS_PACKAGE_HEAD_T head_;
  ADM_PIS_PACKAGE_DATA_T indata_;
  ADM_PIS_PACKAGE_DATA_T outdata_;
  const DCHAR *ecuReqId_ = "0X700";
  const DCHAR *ecuSwDid_ = "0XF187";
  const DCHAR *path_download_ = "/tmp/download.zip";
  const DCHAR *hash_download_ = "/tmp/download_hash";
  const D64S size_download_ = 1001;
  const DCHAR *path_decrypt_ = "/tmp/decrypt.zip";
  const DCHAR *hash_decrypt_ = "/tmp/decrypt_hash";
  const D64S size_decrypt_ = 1002;
  const DCHAR *path_verify_ = "/tmp/verify.zip";
  const DCHAR *hash_verify_ = "/tmp/verify_hash";
  const D64S size_verify_ = 1003;
  const DCHAR *path_decrypt_c_ = "/tmp/decrypt_c.zip";
  const DCHAR *hash_decrypt_c_ = "/tmp/decrypt_c_hash";
  const D64S size_decrypt_c_ = 1006;
};


TEST_F(PisTest, src_package_null){
    head_.type = ADM_PIS_PACKAGE_TYPE_ECU_SRC_PACKAGE;

    head_.state = ADM_PIS_PACKAGE_STATE_DOWNLOAD_FINISH;
    package_null();

    head_.state = ADM_PIS_PACKAGE_STATE_DECRYPT_FINISH;
    package_null();

    head_.state = ADM_PIS_PACKAGE_STATE_VERIFY_SIGN_FINISH;
    package_null();
}
TEST_F(PisTest, dst_package_null){
    head_.type = ADM_PIS_PACKAGE_TYPE_ECU_DST_PACKAGE;

    head_.state = ADM_PIS_PACKAGE_STATE_DOWNLOAD_FINISH;
    package_null();

    head_.state = ADM_PIS_PACKAGE_STATE_DECRYPT_FINISH;
    package_null();

    head_.state = ADM_PIS_PACKAGE_STATE_VERIFY_SIGN_FINISH;
    package_null();
}

TEST_F(PisTest, src_package_download){
    head_.type = ADM_PIS_PACKAGE_TYPE_ECU_SRC_PACKAGE;
    head_.state = ADM_PIS_PACKAGE_STATE_DOWNLOAD_FINISH;
    package_verify(path_download_, hash_download_, size_download_);
}

TEST_F(PisTest, src_package_decrypt){
    head_.type = ADM_PIS_PACKAGE_TYPE_ECU_SRC_PACKAGE;
    head_.state = ADM_PIS_PACKAGE_STATE_DECRYPT_FINISH;
    package_verify(path_decrypt_, hash_decrypt_, size_decrypt_);
}

TEST_F(PisTest, src_package_verify){
    head_.type = ADM_PIS_PACKAGE_TYPE_ECU_SRC_PACKAGE;
    head_.state = ADM_PIS_PACKAGE_STATE_VERIFY_SIGN_FINISH;
    package_verify(path_verify_, hash_verify_, size_verify_);
}

TEST_F(PisTest, dst_package_download){
    head_.type = ADM_PIS_PACKAGE_TYPE_ECU_DST_PACKAGE;
    head_.state = ADM_PIS_PACKAGE_STATE_DOWNLOAD_FINISH;
    package_verify(path_download_, hash_download_, size_download_);
}

TEST_F(PisTest, dst_package_decrypt){
    head_.type = ADM_PIS_PACKAGE_TYPE_ECU_DST_PACKAGE;
    head_.state = ADM_PIS_PACKAGE_STATE_DECRYPT_FINISH;
    package_verify(path_decrypt_, hash_decrypt_, size_decrypt_);
}

TEST_F(PisTest, dst_package_verify){
    head_.type = ADM_PIS_PACKAGE_TYPE_ECU_DST_PACKAGE;
    head_.state = ADM_PIS_PACKAGE_STATE_VERIFY_SIGN_FINISH;
    package_verify(path_verify_, hash_verify_, size_verify_);
}

TEST_F(PisTest, dst_package_all){
    head_.type = ADM_PIS_PACKAGE_TYPE_ECU_DST_PACKAGE;
    head_.state = ADM_PIS_PACKAGE_STATE_DOWNLOAD_FINISH;
    package_verify(path_download_, hash_download_, size_download_);
    head_.state = ADM_PIS_PACKAGE_STATE_DECRYPT_FINISH;
    package_verify(path_decrypt_, hash_decrypt_, size_decrypt_);
    head_.state = ADM_PIS_PACKAGE_STATE_VERIFY_SIGN_FINISH;
    package_verify(path_verify_, hash_verify_, size_verify_);
}

TEST_F(PisTest, dst_cover_package){
    head_.type = ADM_PIS_PACKAGE_TYPE_ECU_DST_PACKAGE;
    head_.state = ADM_PIS_PACKAGE_STATE_DECRYPT_FINISH;
    package_verify(path_decrypt_, hash_decrypt_, size_decrypt_);
    package_verify(path_decrypt_c_, hash_decrypt_c_, size_decrypt_c_);
}

}//namespace

#endif
