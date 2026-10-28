
#include "adm_irs_upload.h"
#include "gtest/gtest.h"
#include "adm_memory.h"

class CIRSTest
{
public:
    CIRSTest();
    ~CIRSTest();
public:
    static D32S Post(const ADM_OCS_BUSINESS_TYPE_E eBusinessType,
                     const DCHAR* reqMessageBody,
                     DCHAR** rspMessageBody,
                     DSIZE* rspStatusCode);
    static D32S PostFile(ADM_OCA_FILE_UPLOAD_INFO_T* uploadInfo, 
                         DCHAR** rspMessageBody, 
                         ADM_OCS_UPLOADFILE_T* priv);
    static D32S adm_apc_logInit(void);

    void adm_irs_addUploadFile_Test();
    
    void adm_irs_vehicle_State_Test();
    
    void adm_irs_vehicle_Progress_Test();
    
    void adm_irs_vehicle_Event_Test();
};

class CIRS_ADD_UPLoadFileUnitTest:public testing::Test
{
protected:
  void SetUp() override 
  {
  }
  void TearDown() override 
  {
  }
};

class CIRS_VEHICLE_State_UnitTest:public testing::Test
{
protected:
  void SetUp() override 
  {
  }
  void TearDown() override 
  {
  }
};

class CIRS_VEHICLE_Progress_UnitTest:public testing::Test
{
protected:
  void SetUp() override 
  {
  }
  void TearDown() override 
  {
  }
};

class CIRS_VEHICLE_Event_UnitTest:public testing::Test
{
protected:
  void SetUp() override 
  {
  }
  void TearDown() override 
  {
  }
};
