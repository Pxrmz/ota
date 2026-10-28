
#include "adm_cds_compare.h"
#include "gtest/gtest.h"
#include "adm_memory.h"

class CCdsCompareTest
{
public:
    CCdsCompareTest(){};
    ~CCdsCompareTest(){};
public:
    void cds_greater_test();
    void cds_greaterThan_test();
    void cds_equal_test();
    void cds_NotEq_test();
    void cds_Less_test();
    void cds_LessThan_test();
    
    void cds_and_test();
    void cds_or_test();
    
    void cds_cross_int_uint_GeneralOrtest();
    void cds_cross_int_uint_GeneralAndtest();
    
    void cds_cross_uint_u64_GeneralOrtest();
    void cds_cross_uint_u64_GeneralAndtest();
    
    void cds_cross_int_float_Generalortest();
    void cds_cross_int_float_GeneralAndtest();
    
    void cds_cross_int_uint_GeneralEQ_Andtest();
    void cds_cross_int_uint_GeneralEQ_Ortest();
    
    void cds_cross_int_u64_GeneralEQ_Andtest();
    void cds_cross_int_u64_GeneralEQ_Ortest();
    
    void cds_cross_uint_u64_GeneralEQ_Andtest();
    void cds_cross_uint_u64_GeneralEQ_Ortest();
    
    void cds_cross_char_string_EQ_Test();
    void cds_cross_char_string_EQ_And_Test();
    void cds_cross_char_string_EQ_Or_Test();
  
    void cds_cross_int_uint_EQ_Test();
    void cds_cross_int_uint_EQ_AND_Test();
    void cds_cross_int_uint_EQ_OR_Test();
    
    void cds_cross_int_u64_EQ_Test();
    void cds_cross_int_u64_EQ_And_Test();
    void cds_cross_int_u64_EQ_OR_Test();
    
    void cds_cross_uint_u64_EQ_Test();
    void cds_cross_uint_u64_EQ_And_Test();
    void cds_cross_uint_u64_EQ_Or_Test();
    
    void cds_cross_int_uint_NotEQ_Test();
    void cds_cross_int_uint_NotEQ_And_Test();
    void cds_cross_int_uint_NotEQ_Or_Test();
    
    void cds_cross_int_u64_NotEQ_Test();
    void cds_cross_int_u64_NotEQ_And_Test();
    void cds_cross_int_u64_NotEQ_Or_Test();
    
    void cds_cross_uint_u64_NotEQ_Test();
    void cds_cross_uint_u64_NotEQ_And_Test();
    void cds_cross_uint_u64_NotEQ_Or_Test();
    
    void cds_cross_char_string_NotEQ_Test();
    void cds_cross_char_string_NotEQ_And_Test();
    void cds_cross_char_string_NotEQ_Or_Test();
    
    void cds_cross_int_uint_Less_Test();
    void cds_cross_int_uint_Less_And_Test();
    void cds_cross_int_uint_Less_Or_Test();
    
    void cds_cross_int_u64_Less_Test();
    void cds_cross_int_u64_Less_And_Test();
    void cds_cross_int_u64_Less_Or_Test();
    
    void cds_cross_uint_u64_Less_Test();
    void cds_cross_uint_u64_Less_And_Test();
    void cds_cross_uint_u64_Less_Or_Test();
    
    void cds_cross_int_float_Less_Test();
    void cds_cross_int_float_Less_And_Test();
    void cds_cross_int_float_Less_Or_Test();
    
    void cds_cross_uint_float_Less_Test();
    void cds_cross_uint_float_Less_And_Test();
    void cds_cross_uint_float_Less_Or_Test();
    
    void cds_cross_u64_float_Less_Test();
    void cds_cross_u64_float_Less_And_Test();
    void cds_cross_u64_float_Less_Or_Test();
    
    void cds_cross_int_uint_LessEQ_Test();
    void cds_cross_int_uint_LessEQ_And_Test();
    void cds_cross_int_uint_LessEQ_Or_Test();
    
    void cds_cross_int_u64_LessEQ_Test();
    void cds_cross_int_u64_LessEQ_And_Test();
    void cds_cross_int_u64_LessEQ_Or_Test();
    
    void cds_cross_uint_u64_LessEQ_Test();
    void cds_cross_uint_u64_LessEQ_And_Test();
    void cds_cross_uint_u64_LessEQ_Or_Test();
    
    void cds_cross_int_float_LessEQ_Test();
    void cds_cross_int_float_LessEQ_And_Test();
    void cds_cross_int_float_LessEQ_Or_Test();
    
    void cds_cross_uint_float_LessEQ_Test();
    void cds_cross_uint_float_LessEQ_And_Test();
    void cds_cross_uint_float_LessEQ_Or_Test();
    
    void cds_cross_u64_float_LessEQ_Test();
    void cds_cross_u64_float_LessEQ_And_Test();
    void cds_cross_u64_float_LessEQ_Or_Test();
};

class CCdsGreaterTest:public testing::Test
{
protected:
  void SetUp() override 
  {
  }
  void TearDown() override 
  {
  }
};

class CCdsGreaterThanTest:public testing::Test
{
protected:
  void SetUp() override 
  {
  }
  void TearDown() override 
  {
  }
};

class CCdsEqualTest:public testing::Test
{
protected:
  void SetUp() override 
  {
  }
  void TearDown() override 
  {
  }
};

class CCdsNotEqualTest:public testing::Test
{
protected:
  void SetUp() override 
  {
  }
  void TearDown() override 
  {
  }
};

class CCdsLessTest:public testing::Test
{
protected:
  void SetUp() override 
  {
  }
  void TearDown() override 
  {
  }
};

class CCdsLessThanTest:public testing::Test
{
protected:
  void SetUp() override 
  {
  }
  void TearDown() override 
  {
  }
};

class CCdsAndTest:public testing::Test
{
protected:
  void SetUp() override 
  {
  }
  void TearDown() override 
  {
  }
};

class CCdsOrTest:public testing::Test
{
protected:
  void SetUp() override 
  {
  }
  void TearDown() override 
  {
  }
};

class CCdsCrossInit_Uint_General_Or_Test:public testing::Test
{
protected:
  void SetUp() override 
  {
  }
  void TearDown() override 
  {
  }
};

class CCdsCrossInit_Uint_General_And_Test:public testing::Test
{
protected:
  void SetUp() override 
  {
  }
  void TearDown() override 
  {
  }
};

class CCdsCross_Uint_U64_General_Or_Test:public testing::Test
{
protected:
  void SetUp() override 
  {
  }
  void TearDown() override 
  {
  }
};

class CCdsCross_Uint_U64_General_And_Test:public testing::Test
{
protected:
  void SetUp() override 
  {
  }
  void TearDown() override 
  {
  }
};

class CCdsCross_int_float_General_Or_Test:public testing::Test
{
protected:
  void SetUp() override 
  {
  }
  void TearDown() override 
  {
  }
};

class CCdsCross_int_float_General_And_Test:public testing::Test
{
protected:
  void SetUp() override 
  {
  }
  void TearDown() override 
  {
  }
};

class CCdsCross_int_uint_GeneralEQ_And_Test:public testing::Test
{
protected:
  void SetUp() override 
  {
  }
  void TearDown() override 
  {
  }
};

class CCdsCross_int_uint_GeneralEQ_Or_Test:public testing::Test
{
protected:
  void SetUp() override 
  {
  }
  void TearDown() override 
  {
  }
};

class CCdsCross_int_u64_GeneralEQ_And_Test:public testing::Test
{
protected:
  void SetUp() override 
  {
  }
  void TearDown() override 
  {
  }
};

class CCdsCross_int_u64_GeneralEQ_Or_Test:public testing::Test
{
protected:
  void SetUp() override 
  {
  }
  void TearDown() override 
  {
  }
};

class CCdsCross_uint_u64_GeneralEQ_And_Test:public testing::Test
{
protected:
  void SetUp() override 
  {
  }
  void TearDown() override 
  {
  }
};

class CCdsCross_uint_u64_GeneralEQ_Or_Test:public testing::Test
{
protected:
  void SetUp() override 
  {
  }
  void TearDown() override 
  {
  }
};

class CCdsCross_char_string_EQ_Test:public testing::Test
{
protected:
  void SetUp() override 
  {
  }
  void TearDown() override 
  {
  }
};

class CCdsCross_char_string_EQ_And_Test:public testing::Test
{
protected:
  void SetUp() override 
  {
  }
  void TearDown() override 
  {
  }
};

class CCdsCross_char_string_EQ_Or_Test:public testing::Test
{
protected:
  void SetUp() override 
  {
  }
  void TearDown() override 
  {
  }
};

class CCdsCross_int_uint_EQ_Test:public testing::Test
{
protected:
  void SetUp() override 
  {
  }
  void TearDown() override 
  {
  }
};

class CCdsCross_int_uint_EQ_AND_Test:public testing::Test
{
protected:
  void SetUp() override 
  {
  }
  void TearDown() override 
  {
  }
};

class CCdsCross_int_uint_EQ_OR_Test:public testing::Test
{
protected:
  void SetUp() override 
  {
  }
  void TearDown() override 
  {
  }
};

class CCdsCross_int_u64_EQ_Test:public testing::Test
{
protected:
  void SetUp() override 
  {
  }
  void TearDown() override 
  {
  }
};

class CCdsCross_int_u64_EQ_AND_Test:public testing::Test
{
protected:
  void SetUp() override 
  {
  }
  void TearDown() override 
  {
  }
};

class CCdsCross_int_u64_EQ_OR_Test:public testing::Test
{
protected:
  void SetUp() override 
  {
  }
  void TearDown() override 
  {
  }
};

class CCdsCross_uint_u64_EQ_Test:public testing::Test
{
protected:
  void SetUp() override 
  {
  }
  void TearDown() override 
  {
  }
};

class CCdsCross_uint_u64_EQ_And_Test:public testing::Test
{
protected:
  void SetUp() override 
  {
  }
  void TearDown() override 
  {
  }
};

class CCdsCross_uint_u64_EQ_Or_Test:public testing::Test
{
protected:
  void SetUp() override 
  {
  }
  void TearDown() override 
  {
  }
};

class CCdsCross_int_uint_NotEQ_Test:public testing::Test
{
protected:
  void SetUp() override 
  {
  }
  void TearDown() override 
  {
  }
};

class CCdsCross_int_uint_NotEQ_And_Test:public testing::Test
{
protected:
  void SetUp() override 
  {
  }
  void TearDown() override 
  {
  }
};

class CCdsCross_int_uint_NotEQ_Or_Test:public testing::Test
{
protected:
  void SetUp() override 
  {
  }
  void TearDown() override 
  {
  }
};

class CCdsCross_int_u64_NotEQ_Test:public testing::Test
{
protected:
  void SetUp() override 
  {
  }
  void TearDown() override 
  {
  }
};

class CCdsCross_int_u64_NotEQ_And_Test:public testing::Test
{
protected:
  void SetUp() override 
  {
  }
  void TearDown() override 
  {
  }
};

class CCdsCross_int_u64_NotEQ_Or_Test:public testing::Test
{
protected:
  void SetUp() override 
  {
  }
  void TearDown() override 
  {
  }
};

class CCdsCross_uint_u64_NotEQ_Test:public testing::Test
{
protected:
  void SetUp() override 
  {
  }
  void TearDown() override 
  {
  }
};

class CCdsCross_uint_u64_NotEQ_And_Test:public testing::Test
{
protected:
  void SetUp() override 
  {
  }
  void TearDown() override 
  {
  }
};

class CCdsCross_uint_u64_NotEQ_Or_Test:public testing::Test
{
protected:
  void SetUp() override 
  {
  }
  void TearDown() override 
  {
  }
};

class CCdsCross_char_string_NotEQ_Test:public testing::Test
{
protected:
  void SetUp() override 
  {
  }
  void TearDown() override 
  {
  }
};

class CCdsCross_char_string_NotEQ_And_Test:public testing::Test
{
protected:
  void SetUp() override 
  {
  }
  void TearDown() override 
  {
  }
};

class CCdsCross_char_string_NotEQ_Or_Test:public testing::Test
{
protected:
  void SetUp() override 
  {
  }
  void TearDown() override 
  {
  }
};

class CCdsCross_int_uint_Less_Test:public testing::Test
{
protected:
  void SetUp() override 
  {
  }
  void TearDown() override 
  {
  }
};

class CCdsCross_int_uint_Less_And_Test:public testing::Test
{
protected:
  void SetUp() override 
  {
  }
  void TearDown() override 
  {
  }
};

class CCdsCross_int_uint_Less_Or_Test:public testing::Test
{
protected:
  void SetUp() override 
  {
  }
  void TearDown() override 
  {
  }
};

class CCdsCross_int_u64_Less_Test:public testing::Test
{
protected:
  void SetUp() override 
  {
  }
  void TearDown() override 
  {
  }
};

class CCdsCross_int_u64_Less_And_Test:public testing::Test
{
protected:
  void SetUp() override 
  {
  }
  void TearDown() override 
  {
  }
};

class CCdsCross_int_u64_Less_Or_Test:public testing::Test
{
protected:
  void SetUp() override 
  {
  }
  void TearDown() override 
  {
  }
};

class CCdsCross_uint_u64_Less_Test:public testing::Test
{
protected:
  void SetUp() override 
  {
  }
  void TearDown() override 
  {
  }
};

class CCdsCross_uint_u64_Less_And_Test:public testing::Test
{
protected:
  void SetUp() override 
  {
  }
  void TearDown() override 
  {
  }
};

class CCdsCross_uint_u64_Less_Or_Test:public testing::Test
{
protected:
  void SetUp() override 
  {
  }
  void TearDown() override 
  {
  }
};

class CCdsCross_int_float_Less_Test:public testing::Test
{
protected:
  void SetUp() override 
  {
  }
  void TearDown() override 
  {
  }
};

class CCdsCross_int_float_Less_And_Test:public testing::Test
{
protected:
  void SetUp() override 
  {
  }
  void TearDown() override 
  {
  }
};

class CCdsCross_int_float_Less_Or_Test:public testing::Test
{
protected:
  void SetUp() override 
  {
  }
  void TearDown() override 
  {
  }
};

class CCdsCross_uint_float_Less_Test:public testing::Test
{
protected:
  void SetUp() override 
  {
  }
  void TearDown() override 
  {
  }
};

class CCdsCross_uint_float_Less_And_Test:public testing::Test
{
protected:
  void SetUp() override 
  {
  }
  void TearDown() override 
  {
  }
};

class CCdsCross_uint_float_Less_Or_Test:public testing::Test
{
protected:
  void SetUp() override 
  {
  }
  void TearDown() override 
  {
  }
};

class CCdsCross_u64_float_Less_Test:public testing::Test
{
protected:
  void SetUp() override 
  {
  }
  void TearDown() override 
  {
  }
};

class CCdsCross_u64_float_Less_And_Test:public testing::Test
{
protected:
  void SetUp() override 
  {
  }
  void TearDown() override 
  {
  }
};

class CCdsCross_u64_float_Less_Or_Test:public testing::Test
{
protected:
  void SetUp() override 
  {
  }
  void TearDown() override 
  {
  }
};

class CCdsCross_int_uint_LessEQ_Test:public testing::Test
{
protected:
  void SetUp() override 
  {
  }
  void TearDown() override 
  {
  }
};

class CCdsCross_int_uint_LessEQ_And_Test:public testing::Test
{
protected:
  void SetUp() override 
  {
  }
  void TearDown() override 
  {
  }
};

class CCdsCross_int_uint_LessEQ_Or_Test:public testing::Test
{
protected:
  void SetUp() override 
  {
  }
  void TearDown() override 
  {
  }
};

class CCdsCross_int_u64_LessEQ_Test:public testing::Test
{
protected:
  void SetUp() override 
  {
  }
  void TearDown() override 
  {
  }
};

class CCdsCross_int_u64_LessEQ_And_Test:public testing::Test
{
protected:
  void SetUp() override 
  {
  }
  void TearDown() override 
  {
  }
};

class CCdsCross_int_u64_LessEQ_Or_Test:public testing::Test
{
protected:
  void SetUp() override 
  {
  }
  void TearDown() override 
  {
  }
};

class CCdsCross_uint_u64_LessEQ_Test:public testing::Test
{
protected:
  void SetUp() override 
  {
  }
  void TearDown() override 
  {
  }
};

class CCdsCross_uint_u64_LessEQ_And_Test:public testing::Test
{
protected:
  void SetUp() override 
  {
  }
  void TearDown() override 
  {
  }
};

class CCdsCross_uint_u64_LessEQ_Or_Test:public testing::Test
{
protected:
  void SetUp() override 
  {
  }
  void TearDown() override 
  {
  }
};

class CCdsCross_int_float_LessEQ_Test:public testing::Test
{
protected:
  void SetUp() override 
  {
  }
  void TearDown() override 
  {
  }
};

class CCdsCross_int_float_LessEQ_And_Test:public testing::Test
{
protected:
  void SetUp() override 
  {
  }
  void TearDown() override 
  {
  }
};

class CCdsCross_int_float_LessEQ_Or_Test:public testing::Test
{
protected:
  void SetUp() override 
  {
  }
  void TearDown() override 
  {
  }
};

class CCdsCross_uint_float_LessEQ_Test:public testing::Test
{
protected:
  void SetUp() override 
  {
  }
  void TearDown() override 
  {
  }
};

class CCdsCross_uint_float_LessEQ_And_Test:public testing::Test
{
protected:
  void SetUp() override 
  {
  }
  void TearDown() override 
  {
  }
};

class CCdsCross_uint_float_LessEQ_Or_Test:public testing::Test
{
protected:
  void SetUp() override 
  {
  }
  void TearDown() override 
  {
  }
};

class CCdsCross_u64_float_LessEQ_Test:public testing::Test
{
protected:
  void SetUp() override 
  {
  }
  void TearDown() override 
  {
  }
};

class CCdsCross_u64_float_LessEQ_And_Test:public testing::Test
{
protected:
  void SetUp() override 
  {
  }
  void TearDown() override 
  {
  }
};

class CCdsCross_u64_float_LessEQ_Or_Test:public testing::Test
{
protected:
  void SetUp() override 
  {
  }
  void TearDown() override 
  {
  }
};