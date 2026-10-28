
#include "adm_cvs_checkVersion.h"
#include "adm_cvs_versionPolicy.h"
#include "gtest/gtest.h"
#include "adm_memory.h"


class CCVSCheckVersionTest
{
public:
    CCVSCheckVersionTest();
    ~CCVSCheckVersionTest();
public:
    static D32S adm_businessComm_callBack(const ADM_OCS_BUSINESS_TYPE_E eBusinessType, 
                                          const DCHAR* reqMessageBody, 
                                          DCHAR** rspMessageBody, 
                                          DSIZE* rspStatusCode);
                                          
    static D32S adm_register_callBack(const ADM_OCS_BUSINESS_TYPE_E eBusinessType, 
                                      const DCHAR* reqMessageBody, 
                                      DCHAR** rspMessageBody, 
                                      DSIZE* rspStatusCode);

    static void checkProgressNotifyHmi_F(const ADM_CVS_COMMON_CMDID_TYPE_E eCmdidTYpe,
                                      const DCHAR* state,
                                      const ADM_CVS_COMMON_CHECK_PROGRESS_TYPE_T eCheckProgressType);
    static void checkProgressUpload_F(const DCHAR* state, const DCHAR* reason);
    void adm_cvs_checkVersion_Test();

};

class CCVSCheckVersionUnitTest:public testing::Test
{
protected:
  void SetUp() override 
  {
  }
  void TearDown() override 
  {
  }
};


