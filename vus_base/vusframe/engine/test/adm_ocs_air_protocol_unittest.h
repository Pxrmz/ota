
#include "adm_ocs_airProtocolSecurity.h"
#include "gtest/gtest.h"
#include "adm_memory.h"

class COCS_Air_Protocol
{
public:
    COCS_Air_Protocol();
    ~COCS_Air_Protocol();
public:
    void adm_ocs_ariProtocolSign_Test();
};


class COCS_Air_Protocol_UnitTest:public testing::Test
{
protected:
  void SetUp() override 
  {
  }
  void TearDown() override 
  {
  }
};
