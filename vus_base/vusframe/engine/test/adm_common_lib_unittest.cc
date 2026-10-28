#include "adm_com_lib.h"
#include "gtest/gtest.h"
#include "adm_string.h"
#include "adm_memory.h"
#include "adm_com_hash.h"

#include <stdio.h>

namespace{

constexpr DCHAR *m_file = "./adm_common_lib_file";
constexpr DCHAR *m_dir = "./adm_common_lib_dir/";
constexpr DCHAR *m_multiDirFile = "./adm_common_lib_dir/temp/temp/test_file";
constexpr DCHAR *m_data = "adm_common_lib_file_data";

constexpr DCHAR *m_zip = "./adm_common_lib_zip.zip";
constexpr DCHAR *m_zipDstDir = "./adm_common_lib_zip_dst";
constexpr DCHAR *m_zipFile = "./adm_common_lib_zip_dst/adm_common_lib_zip/test_file";
constexpr DCHAR *m_zipMultiDirFile = "./adm_common_lib_zip_dst/adm_common_lib_zip/temp/temp/test_file";
constexpr DCHAR *m_zipFileMd5 = "99a6a915dd0e21bb9839289b83321a0e";

TEST(common_lib, common_file_create){
    adm_com_rmFile(m_file);
    EXPECT_FALSE(adm_com_isFileExist(m_file));
    EXPECT_EQ(0, adm_com_createFileDirAndFile(m_file, 0));
    EXPECT_TRUE(adm_com_isFileExist(m_file));
    adm_com_rmFile(m_file);
}

TEST(common_lib, common_dir_create){
    adm_com_rmFile(m_dir);
    EXPECT_FALSE(adm_com_isDirExist(m_dir));
    EXPECT_EQ(0, adm_com_createDir(m_dir));
    EXPECT_TRUE(adm_com_isDirExist(m_dir));
    adm_com_rmFile(m_dir);
}

TEST(common_lib, common_DirAndFile_create){
    DCHAR* data = NULL;

    adm_com_rmFile(m_dir);

    EXPECT_FALSE(adm_com_isFileExist(m_multiDirFile));
    EXPECT_EQ(0, adm_com_createFileDirAndFile(m_multiDirFile, 0));
    EXPECT_TRUE(adm_com_isFileExist(m_multiDirFile));

    EXPECT_EQ(0, adm_com_writeFileData(m_multiDirFile, m_data));

    EXPECT_EQ(0, adm_com_readFileData(m_multiDirFile, &data));
    ASSERT_NE(nullptr, data);
    EXPECT_NE(0, adm_strlen(data));
    adm_free(data);data = NULL;

    EXPECT_EQ(0, adm_com_createFileDirAndFile(m_multiDirFile, 0));

    EXPECT_EQ(0, adm_com_readFileData(m_multiDirFile, &data));
    ASSERT_NE(nullptr, data);
    EXPECT_NE(0, adm_strlen(data));
    adm_free(data);data = NULL;

    EXPECT_EQ(0, adm_com_createFileDirAndFile(m_multiDirFile, 1));

    EXPECT_EQ(0, adm_com_readFileData(m_multiDirFile, &data));
    ASSERT_NE(nullptr, data);
    EXPECT_EQ(0, adm_strlen(data));
    adm_free(data);data = NULL;

    adm_com_rmFile(m_dir);
}

TEST(common_lib, common_file_read_write){
    DCHAR *data = NULL;
    adm_com_rmFile(m_file);
    EXPECT_EQ(0, adm_com_createFileDirAndFile(m_file, 0));

    EXPECT_EQ(0, adm_com_readFileData(m_file, &data));
    ASSERT_NE(nullptr, data);
    EXPECT_EQ(0, adm_strlen(data));
    adm_free(data);data = NULL;

    EXPECT_EQ(0, adm_com_writeFileData(m_file, m_data));

    EXPECT_EQ(0, adm_com_readFileData(m_file, &data));
    ASSERT_NE(nullptr, data);
    EXPECT_EQ(adm_strlen(m_data), adm_strlen(data));
    EXPECT_EQ(0, adm_strcmp(m_data, data));
    adm_free(data);data = NULL;

    adm_com_rmFile(m_file);
}

TEST(common_lib, common_unzip_file){
    DCHAR md[256] = {0};
    EXPECT_FALSE(adm_com_isFileExist(m_zipFile));
    EXPECT_FALSE(adm_com_isFileExist(m_zipMultiDirFile));
    EXPECT_EQ(0, adm_com_unZipFile(m_zip, m_zipDstDir));

    EXPECT_TRUE(adm_com_isFileExist(m_zipFile));
    md[0] = 0;
    adm_com_md5File(m_zipFile, md, sizeof(md)/sizeof(md[0]));
    EXPECT_EQ(0, adm_strcmp(m_zipFileMd5, md));

    EXPECT_TRUE(adm_com_isFileExist(m_zipMultiDirFile));
    md[0] = 0;
    adm_com_md5File(m_zipMultiDirFile, md, sizeof(md)/sizeof(md[0]));
    EXPECT_EQ(0, adm_strcmp(m_zipFileMd5, md));

    adm_com_rmFile(m_zipDstDir);
}

}
