
#include "adm_cds_parser_unittest.h"
#include "json.h"

void CCdsParseTest::adm_condPolicyClone_Test()
{
    ADM_COND_POLICY_T src;
    src.iType = COMPARE_SYMBOL_GREATE;
    src.iCondNum = 1;
    src.pCondtionS = (CONDTION_T*)adm_malloc(sizeof(CONDTION_T));
    adm_memset(src.pCondtionS,0,sizeof(CONDTION_T),sizeof(CONDTION_T));
    src.pCondtionS->messageId = "test_case01";
    src.pCondtionS->messageName="test_case01";
    src.pCondtionS->messageDesc = "test_case01";
    src.pCondtionS->iExpressNum = 1;
    src.pCondtionS->expressions = (EXPRESSION_T*)adm_malloc(sizeof(EXPRESSION_T));
    adm_memset(src.pCondtionS->expressions,0,sizeof(EXPRESSION_T),sizeof(EXPRESSION_T));
    
    src.pCondtionS->expressions->iLogic = 1;
    src.pCondtionS->expressions->sData.data_type = VAR_U32;
    src.pCondtionS->expressions->sData.data_length = 1;
    src.pCondtionS->expressions->sData.v.value_u32 = 2;
    src.pCondtionS->expressions->connectRelation = COMPARE_AND;
    src.pCondtionS->connectRelation = COMPARE_AND;
    src.pCondtionS->onFailStrategy = (FAILSTRATEGY_T*)adm_malloc(sizeof(FAILSTRATEGY_T));
    adm_memset(src.pCondtionS->onFailStrategy,0,sizeof(FAILSTRATEGY_T),sizeof(FAILSTRATEGY_T));
    src.pCondtionS->onFailStrategy->type = 1;
    src.pCondtionS->onFailStrategy->timeout = 0;
    src.pCondtionS->onFailStrategy->action = 1;
    src.pCondtionS->onFailStrategy->iExpressNum = 1;
    
    src.pCondtionS->onFailStrategy->expressions = (EXPRESSION_T*)adm_malloc(sizeof(EXPRESSION_T));
    adm_memset(src.pCondtionS->onFailStrategy->expressions,0,sizeof(EXPRESSION_T),sizeof(EXPRESSION_T));
    src.pCondtionS->onFailStrategy->expressions->iLogic = 1;
    src.pCondtionS->onFailStrategy->expressions->sData.data_type = VAR_U32;
    src.pCondtionS->onFailStrategy->expressions->sData.data_length = 1;
    src.pCondtionS->onFailStrategy->expressions->sData.v.value_u32 = 3;
    src.pCondtionS->onFailStrategy->expressions->connectRelation = COMPARE_AND;
    
    ADM_COND_POLICY_T dst;
    adm_condPolicyClone(&src,&dst);
    
    EXPECT_EQ(dst.iType,COMPARE_SYMBOL_GREATE);

    ASSERT_NE(nullptr,dst.pCondtionS);
    ASSERT_NE(nullptr,dst.pCondtionS->messageId);
    EXPECT_STREQ("test_case01",dst.pCondtionS->messageId);

    ASSERT_NE(nullptr,dst.pCondtionS->messageName);
    EXPECT_STREQ("test_case01",dst.pCondtionS->messageName);
    ASSERT_NE(nullptr,dst.pCondtionS->messageDesc);
    EXPECT_STREQ("test_case01",dst.pCondtionS->messageDesc);
    
    EXPECT_EQ(1,dst.pCondtionS->iExpressNum);
    ASSERT_NE(nullptr,dst.pCondtionS->expressions);
    EXPECT_EQ(1,dst.pCondtionS->iExpressNum);
    EXPECT_EQ(1,dst.pCondtionS->expressions->iLogic);
    EXPECT_EQ(VAR_U32,dst.pCondtionS->expressions->sData.data_type);
    EXPECT_EQ(1,dst.pCondtionS->expressions->sData.data_length);
    EXPECT_EQ(2,dst.pCondtionS->expressions->sData.v.value_u32);
    EXPECT_EQ(COMPARE_AND,dst.pCondtionS->expressions->connectRelation);
    ASSERT_NE(nullptr,dst.pCondtionS->onFailStrategy);
    EXPECT_EQ(1,dst.pCondtionS->onFailStrategy->type);
    EXPECT_EQ(0,dst.pCondtionS->onFailStrategy->timeout);
    EXPECT_EQ(1,dst.pCondtionS->onFailStrategy->action);
    EXPECT_EQ(1,dst.pCondtionS->onFailStrategy->iExpressNum);
    
    ASSERT_NE(nullptr,dst.pCondtionS->onFailStrategy->expressions);
    EXPECT_EQ(1,dst.pCondtionS->onFailStrategy->expressions->iLogic);
    EXPECT_EQ(VAR_U32,dst.pCondtionS->onFailStrategy->expressions->sData.data_type);
    EXPECT_EQ(1,dst.pCondtionS->onFailStrategy->expressions->sData.data_length);
    EXPECT_EQ(3,dst.pCondtionS->onFailStrategy->expressions->sData.v.value_u32);
    EXPECT_EQ(COMPARE_AND,dst.pCondtionS->onFailStrategy->expressions->connectRelation);
    adm_free(src.pCondtionS->onFailStrategy->expressions);
    adm_free(src.pCondtionS->onFailStrategy);
    adm_free(src.pCondtionS->expressions);
    adm_free(src.pCondtionS);
}

TEST_F(CCdsParserCloneTest, Normal)
{
    CCdsParseTest aa;
    aa.adm_condPolicyClone_Test();
}

void CCdsParseTest::adm_condPolicyJsonParse_Test()
{  
const DCHAR *m_installPolicyObjectStr = 
"    {"
"      \"type\": 1,"
"      \"conditions\": ["
"        {"
"          \"messageId\": \"xxx\","
"          \"messageName\": \"xxx\","
"          \"messageDesc\": \"xxx\","
"          \"expressions\": ["
"            {"
"              \"logic\": 1,"
"              \"dataType\": 6,"
"              \"dataLength\": 3,"
"              \"data\": \"101\","
"              \"connectRelation\": 1"
"            }"
"          ],"
"          \"connectRelation\": 1,"
"          \"onFailStrategy\": {"
"            \"type\": 1,"
"            \"timeout\": 30,"
"            \"action\": 1,"
"            \"expressions\": ["
"              {"
"                \"logic\": 1,"
"                \"dataType\": 6,"
"                \"dataLength\": 4,"
"                \"data\": \"1001\","
"                \"connectRelation\": 1"
"              }"
"            ]"
"          }"
"        }"
"      ]"
"    }";

    struct json_object *root = json_tokener_parse(m_installPolicyObjectStr);

    ASSERT_NE(nullptr,root);
    
    ADM_COND_POLICY_T dst;
    adm_condPolicyJsonParse(&dst, root);
    EXPECT_EQ(dst.iType,1);

    ASSERT_NE(nullptr,dst.pCondtionS);
    ASSERT_NE(nullptr,dst.pCondtionS->messageId);
    EXPECT_STREQ("xxx",dst.pCondtionS->messageId);

    ASSERT_NE(nullptr,dst.pCondtionS->messageName);
    EXPECT_STREQ("xxx",dst.pCondtionS->messageName);
    ASSERT_NE(nullptr,dst.pCondtionS->messageDesc);
    EXPECT_STREQ("xxx",dst.pCondtionS->messageDesc);
    
    EXPECT_EQ(1,dst.pCondtionS->iExpressNum);
    ASSERT_NE(nullptr,dst.pCondtionS->expressions);
    EXPECT_EQ(1,dst.pCondtionS->iExpressNum);
    EXPECT_EQ(1,dst.pCondtionS->expressions->iLogic);
    EXPECT_EQ(VAR_STRING,dst.pCondtionS->expressions->sData.data_type);
    EXPECT_EQ(3,dst.pCondtionS->expressions->sData.data_length);
    
    ASSERT_NE(nullptr,dst.pCondtionS->expressions->sData.v.value_s);
    EXPECT_STREQ("101",(DCHAR*)dst.pCondtionS->expressions->sData.v.value_s);

    EXPECT_EQ(1,dst.pCondtionS->expressions->connectRelation);
    ASSERT_NE(nullptr,dst.pCondtionS->onFailStrategy);
    EXPECT_EQ(1,dst.pCondtionS->onFailStrategy->type);
    EXPECT_EQ(30,dst.pCondtionS->onFailStrategy->timeout);
    EXPECT_EQ(1,dst.pCondtionS->onFailStrategy->action);
    
    EXPECT_EQ(1,dst.pCondtionS->onFailStrategy->iExpressNum);
    
    ASSERT_NE(nullptr,dst.pCondtionS->onFailStrategy->expressions);
    EXPECT_EQ(1,dst.pCondtionS->onFailStrategy->expressions->iLogic);
    EXPECT_EQ(VAR_STRING,dst.pCondtionS->onFailStrategy->expressions->sData.data_type);
    EXPECT_EQ(4,dst.pCondtionS->onFailStrategy->expressions->sData.data_length);
    EXPECT_STREQ("1001",(DCHAR*)dst.pCondtionS->onFailStrategy->expressions->sData.v.value_s);
    EXPECT_EQ(COMPARE_AND,dst.pCondtionS->onFailStrategy->expressions->connectRelation);
    json_object_put(root);
}

TEST_F(CCdsParserFromJsonTest, Normal)
{
    CCdsParseTest aa;
    aa.adm_condPolicyJsonParse_Test();
}

void CCdsParseTest::adm_condPolicyJsonCreate_Test()
{
    ADM_COND_POLICY_T src;
    src.iType = COMPARE_SYMBOL_GREATE;
    src.iCondNum = 1;
    src.pCondtionS = (CONDTION_T*)adm_malloc(sizeof(CONDTION_T));
    adm_memset(src.pCondtionS,0,sizeof(CONDTION_T),sizeof(CONDTION_T));
    src.pCondtionS->messageId = "test_case01";
    src.pCondtionS->messageName="test_case01";
    src.pCondtionS->messageDesc = "test_case01";
    src.pCondtionS->iExpressNum = 1;
    src.pCondtionS->expressions = (EXPRESSION_T*)adm_malloc(sizeof(EXPRESSION_T));
    adm_memset(src.pCondtionS->expressions,0,sizeof(EXPRESSION_T),sizeof(EXPRESSION_T));
    
    src.pCondtionS->expressions->iLogic = 1;
    src.pCondtionS->expressions->sData.data_type = VAR_U32;
    src.pCondtionS->expressions->sData.data_length = 1;
    src.pCondtionS->expressions->sData.v.value_u32 = 2;
    src.pCondtionS->expressions->connectRelation = COMPARE_AND;
    src.pCondtionS->connectRelation = COMPARE_AND;
    src.pCondtionS->onFailStrategy = (FAILSTRATEGY_T*)adm_malloc(sizeof(FAILSTRATEGY_T));
    adm_memset(src.pCondtionS->onFailStrategy,0,sizeof(FAILSTRATEGY_T),sizeof(FAILSTRATEGY_T));
    src.pCondtionS->onFailStrategy->type = 1;
    src.pCondtionS->onFailStrategy->timeout = 0;
    src.pCondtionS->onFailStrategy->action = 1;
    src.pCondtionS->onFailStrategy->iExpressNum = 1;
    
    src.pCondtionS->onFailStrategy->expressions = (EXPRESSION_T*)adm_malloc(sizeof(EXPRESSION_T));
    adm_memset(src.pCondtionS->onFailStrategy->expressions,0,sizeof(EXPRESSION_T),sizeof(EXPRESSION_T));
    src.pCondtionS->onFailStrategy->expressions->iLogic = 1;
    src.pCondtionS->onFailStrategy->expressions->sData.data_type = VAR_U32;
    src.pCondtionS->onFailStrategy->expressions->sData.data_length = 1;
    src.pCondtionS->onFailStrategy->expressions->sData.v.value_u32 = 3;
    src.pCondtionS->onFailStrategy->expressions->connectRelation = COMPARE_AND;
    
    struct json_object* pJson = adm_condPolicyJsonCreate(&src);
    ASSERT_NE(nullptr,pJson);
}

TEST_F(CCdsParserCreateJsonTest, Normal)
{
    CCdsParseTest aa;
    aa.adm_condPolicyJsonCreate_Test();
}
