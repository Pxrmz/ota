#include "adm_com_parseFileObject.h"
#include "gtest/gtest.h"
#include "json-c/json.h"
#include "adm_memory.h"

namespace{

constexpr DCHAR *m_fileValueStr = R"(
{
  "fileName":"fileName",
  "url":"url",
  "hash":"hash",
  "filePath":"filePath",
  "fileSecurityInfo":{
    "oriHash":"oriHash",
    "pkgKey":"pkgKey",
    "isEncrypt":1,
    "segmentDecryptLen":64,
    "oriSize":1023,
    "fileMetas":[
      {
        "checkSum":{
            "algorithm":1,
            "value":"value_1",
          },
        "type":1,
        "size":102
      },
      {
        "checkSum":{
            "algorithm":2,
            "value":"value_2",
          },
        "type":2,
        "size":103
      }
    ]
  },
  "size":101
}
)";

void verifyFileMeta(const ADM_COM_FILEMETA_T *pstFileMetaList, const DCHAR *value, D32S algorithm, D32S type, D64S size){
    ASSERT_NE(nullptr, pstFileMetaList->stCheckSum.value);
    EXPECT_STREQ(value, pstFileMetaList->stCheckSum.value);
    EXPECT_EQ(algorithm, pstFileMetaList->stCheckSum.algorithm);
    EXPECT_EQ(type, pstFileMetaList->type);
    EXPECT_EQ(size, pstFileMetaList->size);
}

void verifyFileScyInfo(const ADM_COM_FILESCYINFO_T *pstFileSecurityInfo){
    ASSERT_NE(nullptr, pstFileSecurityInfo->oriHash);
    EXPECT_STREQ("oriHash", pstFileSecurityInfo->oriHash);
    ASSERT_NE(nullptr, pstFileSecurityInfo->pkgKey);
    EXPECT_STREQ("pkgKey", pstFileSecurityInfo->pkgKey);
    EXPECT_EQ(1, pstFileSecurityInfo->isEncrypt);
    EXPECT_EQ(64, pstFileSecurityInfo->segmentDecryptLen);
    EXPECT_EQ(1023, pstFileSecurityInfo->oriSize);
    ASSERT_EQ(2, pstFileSecurityInfo->fileMetaNum);
    ASSERT_NE(nullptr, pstFileSecurityInfo->pstFileMetaList);
    verifyFileMeta(&pstFileSecurityInfo->pstFileMetaList[0], "value_1", 1, 1, 102);
    verifyFileMeta(&pstFileSecurityInfo->pstFileMetaList[1], "value_2", 2, 2, 103);
}

void verifyFileObject(const ADM_COM_FILEOBJECT_T* pstFileObject){
    ASSERT_NE(nullptr, pstFileObject->fileName);
    EXPECT_STREQ("fileName", pstFileObject->fileName);
    ASSERT_NE(nullptr, pstFileObject->url);
    EXPECT_STREQ("url", pstFileObject->url);
    ASSERT_NE(nullptr, pstFileObject->hash);
    EXPECT_STREQ("hash", pstFileObject->hash);
    /* TODO wait src open
    ASSERT_NE(nullptr, pstFileObject->filePath);
    EXPECT_STREQ("filePath", pstFileObject->filePath);
    ASSERT_NE(nullptr, pstFileObject->pstFileSecurityInfo);
    verifyFileScyInfo(pstFileObject->pstFileSecurityInfo);
    */
    EXPECT_EQ(101, pstFileObject->size);
}

TEST(common_parse, common_file_object){
    struct json_object *root = json_tokener_parse(m_fileValueStr);
    ASSERT_NE(nullptr, root);
    ADM_COM_FILEOBJECT_T* pstFileObject = adm_com_newFileObject();
    ASSERT_NE(nullptr, pstFileObject);
    ASSERT_EQ(0, adm_com_parseJsonFileObject(root, pstFileObject));
    verifyFileObject(pstFileObject);
    adm_com_delFileObject(&pstFileObject);
    json_object_put(root);
}

TEST(common_parse, common_file_copy){
    struct json_object *root = json_tokener_parse(m_fileValueStr);
    ASSERT_NE(nullptr, root);
    ADM_COM_FILEOBJECT_T* pstFileObject = adm_com_newFileObject();
    ASSERT_NE(nullptr, pstFileObject);
    ASSERT_EQ(0, adm_com_parseJsonFileObject(root, pstFileObject));
    
    ADM_COM_FILEOBJECT_T* pstFileObjectCopy = adm_com_newFileObject();
    ASSERT_EQ(0, adm_com_copyFileObject(pstFileObjectCopy, pstFileObject));
    verifyFileObject(pstFileObjectCopy);
    adm_com_delFileObject(&pstFileObjectCopy);
    
    adm_com_delFileObject(&pstFileObject);
    json_object_put(root);
}

constexpr DCHAR *m_jsonValueStr = R"(
{
  "intval": 21,
  "int64val":1152921504606846975,
  "strval":"hello world",
  "floatval":3.1415
}
)";

TEST(common_parse, common_json_parse_D32S){
    struct json_object *root = json_tokener_parse(m_jsonValueStr);
    ASSERT_NE(nullptr, root);
    D32S intval = 0;
    EXPECT_EQ(0, adm_com_getJsonIntVal(root, "intval", &intval));
    EXPECT_EQ(21, intval);
    json_object_put(root);
}

TEST(common_parse, common_json_parse_D64S){
    struct json_object *root = json_tokener_parse(m_jsonValueStr);
    ASSERT_NE(nullptr, root);
    D64S int64val = 0;
    EXPECT_EQ(0, adm_com_getJsonInt64Val(root, "int64val", &int64val));
    EXPECT_EQ(1152921504606846975, int64val);
    json_object_put(root);
}

TEST(common_parse, common_json_parse_str){
    struct json_object *root = json_tokener_parse(m_jsonValueStr);
    ASSERT_NE(nullptr, root);
    DCHAR* strval = adm_com_getJsonStrVal(root, "strval");
    EXPECT_STRNE(nullptr, strval);
    EXPECT_STREQ("hello world", strval);
    adm_free(strval);
    json_object_put(root);
}

TEST(common_parse, common_json_parse_float){
    struct json_object *root = json_tokener_parse(m_jsonValueStr);
    ASSERT_NE(nullptr, root);
    float floatval = 0;
    EXPECT_EQ(0, adm_com_getJsonFloatVal(root, "floatval", &floatval));
    EXPECT_FLOAT_EQ(3.1415, floatval);
    json_object_put(root);
}




}
