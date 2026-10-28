
#include "adm_ocs_airProtocolAdapter.h"
#include "gtest/gtest.h"
#include "adm_memory.h"

class COCSAirProtocolAdapterTest
{
public:
    COCSAirProtocolAdapterTest();
    ~COCSAirProtocolAdapterTest();
public:
    static D32S adm_ocs_registerProtocolAdapter_Test_CallBack(const ADM_OCS_BUSINESS_TYPE_E eBusinessType, 
                                                              const DCHAR* reqMessageBody, 
                                                              DCHAR** rspMessageBody, 
                                                              DSIZE* rspStatusCode);
    void adm_ocs_registerProtocolAdapter_Test();
    
    void adm_ocs_registerProtocolParse_Test();
    
    void adm_ocs_checkProtocolAdapter_Test();
    
    void adm_ocs_checkProtocolParse_Test();
    
    void adm_ocs_otaStatusCodeTransfer_Test();
};

class COCSAirProtocolAdapterUnitTest:public testing::Test
{
protected:
  void SetUp() override 
  {
  }
  void TearDown() override 
  {
  }
};

class COCSAirProtocolAdapterParserUnitTest:public testing::Test
{
protected:
  void SetUp() override 
  {
  }
  void TearDown() override 
  {
  }
};

class COCSCheckProtocolAdapterUnitTest:public testing::Test
{
protected:
  void SetUp() override 
  {
  }
  void TearDown() override 
  {
  }
};

class COCSCheckProtocolAdapterParserUnitTest:public testing::Test
{
protected:
  void SetUp() override 
  {
  }
  void TearDown() override 
  {
  }
};

class COSCOTAStatusCodeTransferUnitTest:public testing::Test
{
protected:
  void SetUp() override 
  {
  }
  void TearDown() override 
  {
  }
};

