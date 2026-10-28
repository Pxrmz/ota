#include "adm_cds_compare.h"
#include "adm_cds_parse.h"
#include "gtest/gtest.h"
#include "adm_memory.h"

class CCdsParseTest
{
public:
    CCdsParseTest(){};
    ~CCdsParseTest(){}; 
public:
    void adm_condPolicyClone_Test();
    void adm_condPolicyJsonParse_Test();
    void adm_condPolicyJsonCreate_Test();
};

class CCdsParserCloneTest:public testing::Test
{
protected:
  void SetUp() override 
  {
  }
  void TearDown() override 
  {
  }
};

class CCdsParserFromJsonTest:public testing::Test
{
protected:
  void SetUp() override 
  {
  }
  void TearDown() override 
  {
  }
};

class CCdsParserCreateJsonTest:public testing::Test
{
protected:
  void SetUp() override 
  {
  }
  void TearDown() override 
  {
  }
};