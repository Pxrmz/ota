
#include "adm_cds_unittest.h"

void CCdsCompareTest::cds_greater_test()
{
    cds_compare_st U8Test;
    U8Test.type = COMPARE_SYMBOL_GREATE;
    cds_variable_st lValue;
    lValue.data_type = VAR_U8;
    lValue.v.value[0]='2';
    lValue.data_length = 1;
    cds_variable_st rValue;
    rValue.data_type = VAR_U8;
    rValue.v.value[0]='1';
    rValue.data_length = 1;
    U8Test.l_variable = &lValue;
    U8Test.r_variable = &rValue;

    D8U result = adm_cds_compare(&U8Test, 1);
    EXPECT_EQ(result,0x01);
    
    cds_compare_st intTest;
    intTest.type = COMPARE_SYMBOL_GREATE;
    lValue.data_type = VAR_S32;
    lValue.v.value_s32=2;
    lValue.data_length = 1;
    rValue.data_type = VAR_S32;
    rValue.v.value_s32=1;
    rValue.data_length = 1;
    intTest.l_variable = &lValue;
    intTest.r_variable = &rValue;

    result = adm_cds_compare(&intTest, 1);
    EXPECT_EQ(result,0x01);
    
    cds_compare_st uintTest;
    uintTest.type = COMPARE_SYMBOL_GREATE;
    lValue.data_type = VAR_U32;
    lValue.v.value_u32=2;
    lValue.data_length = 1;
    rValue.data_type = VAR_U32;
    rValue.v.value_u32=1;
    rValue.data_length = 1;
    uintTest.l_variable = &lValue;
    uintTest.r_variable = &rValue;

    result = adm_cds_compare(&uintTest, 1);
    EXPECT_EQ(result,0x01);    

    cds_compare_st u64Test;
    u64Test.type = COMPARE_SYMBOL_GREATE;
    lValue.data_type = VAR_U64;
    lValue.v.value_u64=2;
    lValue.data_length = 1;
    rValue.data_type = VAR_U64;
    rValue.v.value_u64=1;
    rValue.data_length = 1;
    u64Test.l_variable = &lValue;
    u64Test.r_variable = &rValue;

    result = adm_cds_compare(&u64Test, 1);
    EXPECT_EQ(result,0x01);
    
    cds_compare_st floatTest;
    floatTest.type = COMPARE_SYMBOL_GREATE;
    lValue.data_type = VAR_FLOAT;
    lValue.v.value_f=2.0;
    lValue.data_length = 1;
    rValue.data_type = VAR_FLOAT;
    rValue.v.value_f=1.0;
    rValue.data_length = 1;
    floatTest.l_variable = &lValue;
    floatTest.r_variable = &rValue;

    result = adm_cds_compare(&floatTest, 1);
    EXPECT_EQ(result,0x01);
    
    cds_compare_st stringTest;
    stringTest.type = COMPARE_SYMBOL_GREATE;
    lValue.data_type = VAR_STRING;
    lValue.v.value_s=(unsigned char*)"123";
    lValue.data_length = 3;
    rValue.data_type = VAR_STRING;
    rValue.v.value_s=(unsigned char*)"12";
    rValue.data_length = 2;
    stringTest.l_variable = &lValue;
    stringTest.r_variable = &rValue;

    result = adm_cds_compare(&stringTest, 1);
    EXPECT_EQ(result,0x01);
             
    return;
}

TEST_F(CCdsGreaterTest, Normal)
{
    CCdsCompareTest sameTypegreateTest;
    sameTypegreateTest.cds_greater_test();
}

void CCdsCompareTest::cds_greaterThan_test()
{
    cds_compare_st U8Test;
    U8Test.type = COMPARE_SYMBOL_GREATEEQ;
    cds_variable_st lValue;
    lValue.data_type = VAR_U8;
    lValue.v.value[0]='2';
    lValue.data_length = 1;
    cds_variable_st rValue;
    rValue.data_type = VAR_U8;
    rValue.v.value[0]='1';
    rValue.data_length = 1;
    U8Test.l_variable = &lValue;
    U8Test.r_variable = &rValue;

    D8U result = adm_cds_compare(&U8Test, 1);
    EXPECT_EQ(result,0x01);
    
    cds_compare_st intTest;
    intTest.type = COMPARE_SYMBOL_GREATEEQ;
    lValue.data_type = VAR_S32;
    lValue.v.value_s32=2;
    lValue.data_length = 1;
    rValue.data_type = VAR_S32;
    rValue.v.value_s32=1;
    rValue.data_length = 1;
    intTest.l_variable = &lValue;
    intTest.r_variable = &rValue;

    result = adm_cds_compare(&intTest, 1);
    EXPECT_EQ(result,0x01);
    
    cds_compare_st uintTest;
    uintTest.type = COMPARE_SYMBOL_GREATEEQ;
    lValue.data_type = VAR_U32;
    lValue.v.value_u32=2;
    lValue.data_length = 1;
    rValue.data_type = VAR_U32;
    rValue.v.value_u32=1;
    rValue.data_length = 1;
    uintTest.l_variable = &lValue;
    uintTest.r_variable = &rValue;

    result = adm_cds_compare(&uintTest, 1);
    EXPECT_EQ(result,0x01);    

    cds_compare_st u64Test;
    u64Test.type = COMPARE_SYMBOL_GREATEEQ;
    lValue.data_type = VAR_U64;
    lValue.v.value_u64=2;
    lValue.data_length = 1;
    rValue.data_type = VAR_U64;
    rValue.v.value_u64=1;
    rValue.data_length = 1;
    u64Test.l_variable = &lValue;
    u64Test.r_variable = &rValue;

    result = adm_cds_compare(&u64Test, 1);
    EXPECT_EQ(result,0x01);
    
    cds_compare_st floatTest;
    floatTest.type = COMPARE_SYMBOL_GREATEEQ;
    lValue.data_type = VAR_FLOAT;
    lValue.v.value_f=2.0;
    lValue.data_length = 1;
    rValue.data_type = VAR_FLOAT;
    rValue.v.value_f=1.0;
    rValue.data_length = 1;
    floatTest.l_variable = &lValue;
    floatTest.r_variable = &rValue;

    result = adm_cds_compare(&floatTest, 1);
    EXPECT_EQ(result,0x01);
    
    cds_compare_st stringTest;
    stringTest.type = COMPARE_SYMBOL_GREATEEQ;
    lValue.data_type = VAR_STRING;
    lValue.v.value_s=(unsigned char*)"123";
    lValue.data_length = 3;
    rValue.data_type = VAR_STRING;
    rValue.v.value_s=(unsigned char*)"12";
    rValue.data_length = 2;
    stringTest.l_variable = &lValue;
    stringTest.r_variable = &rValue;

    result = adm_cds_compare(&stringTest, 1);
    EXPECT_EQ(result,0x01);
             
    return;    
}

TEST_F(CCdsGreaterThanTest, Normal)
{
    CCdsCompareTest sameTypegreateThanTest;
    sameTypegreateThanTest.cds_greaterThan_test();
}

void CCdsCompareTest::cds_equal_test()
{
    cds_compare_st U8Test;
    U8Test.type = COMPARE_SYMBOL_EQ;
    cds_variable_st lValue;
    lValue.data_type = VAR_U8;
    lValue.v.value[0]='1';
    lValue.data_length = 1;
    cds_variable_st rValue;
    rValue.data_type = VAR_U8;
    rValue.v.value[0]='1';
    rValue.data_length = 1;
    U8Test.l_variable = &lValue;
    U8Test.r_variable = &rValue;

    D8U result = adm_cds_compare(&U8Test, 1);
    EXPECT_EQ(result,0x01);
    
    cds_compare_st intTest;
    intTest.type = COMPARE_SYMBOL_EQ;
    lValue.data_type = VAR_S32;
    lValue.v.value_s32=1;
    lValue.data_length = 1;
    rValue.data_type = VAR_S32;
    rValue.v.value_s32=1;
    rValue.data_length = 1;
    intTest.l_variable = &lValue;
    intTest.r_variable = &rValue;

    result = adm_cds_compare(&intTest, 1);
    EXPECT_EQ(result,0x01);
    
    cds_compare_st uintTest;
    uintTest.type = COMPARE_SYMBOL_EQ;
    lValue.data_type = VAR_U32;
    lValue.v.value_u32=1;
    lValue.data_length = 1;
    rValue.data_type = VAR_U32;
    rValue.v.value_u32=1;
    rValue.data_length = 1;
    uintTest.l_variable = &lValue;
    uintTest.r_variable = &rValue;

    result = adm_cds_compare(&uintTest, 1);
    EXPECT_EQ(result,0x01);    

    cds_compare_st u64Test;
    u64Test.type = COMPARE_SYMBOL_EQ;
    lValue.data_type = VAR_U64;
    lValue.v.value_u64=1;
    lValue.data_length = 1;
    rValue.data_type = VAR_U64;
    rValue.v.value_u64=1;
    rValue.data_length = 1;
    u64Test.l_variable = &lValue;
    u64Test.r_variable = &rValue;

    result = adm_cds_compare(&u64Test, 1);
    EXPECT_EQ(result,0x01);
    
    cds_compare_st floatTest;
    floatTest.type = COMPARE_SYMBOL_EQ;
    lValue.data_type = VAR_FLOAT;
    lValue.v.value_f=1.0;
    lValue.data_length = 1;
    rValue.data_type = VAR_FLOAT;
    rValue.v.value_f=1.0;
    rValue.data_length = 1;
    floatTest.l_variable = &lValue;
    floatTest.r_variable = &rValue;

    result = adm_cds_compare(&floatTest, 1);
    EXPECT_EQ(result,0x01);
    
    cds_compare_st stringTest;
    stringTest.type = COMPARE_SYMBOL_EQ;
    lValue.data_type = VAR_STRING;
    lValue.v.value_s=(unsigned char*)"123";
    lValue.data_length = 3;
    rValue.data_type = VAR_STRING;
    rValue.v.value_s=(unsigned char*)"123";
    rValue.data_length = 3;
    stringTest.l_variable = &lValue;
    stringTest.r_variable = &rValue;

    result = adm_cds_compare(&stringTest, 1);
    EXPECT_EQ(result,0x01);
             
    return;     
}

TEST_F(CCdsEqualTest, Normal)
{
    CCdsCompareTest sameTypeEqualTest;
    sameTypeEqualTest.cds_equal_test();
}

void CCdsCompareTest::cds_NotEq_test()
{
    cds_compare_st U8Test;
    U8Test.type = COMPARE_SYMBOL_NOTEQ;
    cds_variable_st lValue;
    lValue.data_type = VAR_U8;
    lValue.v.value[0]='2';
    lValue.data_length = 1;
    cds_variable_st rValue;
    rValue.data_type = VAR_U8;
    rValue.v.value[0]='1';
    rValue.data_length = 1;
    U8Test.l_variable = &lValue;
    U8Test.r_variable = &rValue;

    D8U result = adm_cds_compare(&U8Test, 1);
    EXPECT_EQ(result,0x01);
    
    cds_compare_st intTest;
    intTest.type = COMPARE_SYMBOL_NOTEQ;
    lValue.data_type = VAR_S32;
    lValue.v.value_s32=2;
    lValue.data_length = 1;
    rValue.data_type = VAR_S32;
    rValue.v.value_s32=1;
    rValue.data_length = 1;
    intTest.l_variable = &lValue;
    intTest.r_variable = &rValue;

    result = adm_cds_compare(&intTest, 1);
    EXPECT_EQ(result,0x01);
    
    cds_compare_st uintTest;
    uintTest.type = COMPARE_SYMBOL_NOTEQ;
    lValue.data_type = VAR_U32;
    lValue.v.value_u32=2;
    lValue.data_length = 1;
    rValue.data_type = VAR_U32;
    rValue.v.value_u32=1;
    rValue.data_length = 1;
    uintTest.l_variable = &lValue;
    uintTest.r_variable = &rValue;

    result = adm_cds_compare(&uintTest, 1);
    EXPECT_EQ(result,0x01);    

    cds_compare_st u64Test;
    u64Test.type = COMPARE_SYMBOL_NOTEQ;
    lValue.data_type = VAR_U64;
    lValue.v.value_u64=2;
    lValue.data_length = 1;
    rValue.data_type = VAR_U64;
    rValue.v.value_u64=1;
    rValue.data_length = 1;
    u64Test.l_variable = &lValue;
    u64Test.r_variable = &rValue;

    result = adm_cds_compare(&u64Test, 1);
    EXPECT_EQ(result,0x01);
    
    cds_compare_st floatTest;
    floatTest.type = COMPARE_SYMBOL_NOTEQ;
    lValue.data_type = VAR_FLOAT;
    lValue.v.value_f=2.0;
    lValue.data_length = 1;
    rValue.data_type = VAR_FLOAT;
    rValue.v.value_f=1.0;
    rValue.data_length = 1;
    floatTest.l_variable = &lValue;
    floatTest.r_variable = &rValue;

    result = adm_cds_compare(&floatTest, 1);
    EXPECT_EQ(result,0x01);
    
    cds_compare_st stringTest;
    stringTest.type = COMPARE_SYMBOL_NOTEQ;
    lValue.data_type = VAR_STRING;
    lValue.v.value_s=(unsigned char*)"1234";
    lValue.data_length = 4;
    rValue.data_type = VAR_STRING;
    rValue.v.value_s=(unsigned char*)"123";
    rValue.data_length = 3;
    stringTest.l_variable = &lValue;
    stringTest.r_variable = &rValue;

    result = adm_cds_compare(&stringTest, 1);
    EXPECT_EQ(result,0x01);
             
    return;     
}

TEST_F(CCdsNotEqualTest, Normal)
{
    CCdsCompareTest sameTypeNotEqualTest;
    sameTypeNotEqualTest.cds_NotEq_test();
}

void CCdsCompareTest::cds_Less_test()
{
    cds_compare_st U8Test;
    U8Test.type = COMPARE_SYMBOL_LESS;
    cds_variable_st lValue;
    lValue.data_type = VAR_U8;
    lValue.v.value[0]='1';
    lValue.data_length = 1;
    cds_variable_st rValue;
    rValue.data_type = VAR_U8;
    rValue.v.value[0]='2';
    rValue.data_length = 1;
    U8Test.l_variable = &lValue;
    U8Test.r_variable = &rValue;

    D8U result = adm_cds_compare(&U8Test, 1);
    EXPECT_EQ(result,0x01);
    
    cds_compare_st intTest;
    intTest.type = COMPARE_SYMBOL_LESS;
    lValue.data_type = VAR_S32;
    lValue.v.value_s32=1;
    lValue.data_length = 1;
    rValue.data_type = VAR_S32;
    rValue.v.value_s32=2;
    rValue.data_length = 1;
    intTest.l_variable = &lValue;
    intTest.r_variable = &rValue;

    result = adm_cds_compare(&intTest, 1);
    EXPECT_EQ(result,0x01);
    
    cds_compare_st uintTest;
    uintTest.type = COMPARE_SYMBOL_LESS;
    lValue.data_type = VAR_U32;
    lValue.v.value_u32=1;
    lValue.data_length = 1;
    rValue.data_type = VAR_U32;
    rValue.v.value_u32=2;
    rValue.data_length = 1;
    uintTest.l_variable = &lValue;
    uintTest.r_variable = &rValue;

    result = adm_cds_compare(&uintTest, 1);
    EXPECT_EQ(result,0x01);    

    cds_compare_st u64Test;
    u64Test.type = COMPARE_SYMBOL_LESS;
    lValue.data_type = VAR_U64;
    lValue.v.value_u64=1;
    lValue.data_length = 1;
    rValue.data_type = VAR_U64;
    rValue.v.value_u64=2;
    rValue.data_length = 1;
    u64Test.l_variable = &lValue;
    u64Test.r_variable = &rValue;

    result = adm_cds_compare(&u64Test, 1);
    EXPECT_EQ(result,0x01);
    
    cds_compare_st floatTest;
    floatTest.type = COMPARE_SYMBOL_LESS;
    lValue.data_type = VAR_FLOAT;
    lValue.v.value_f=1.0;
    lValue.data_length = 1;
    rValue.data_type = VAR_FLOAT;
    rValue.v.value_f=2.0;
    rValue.data_length = 1;
    floatTest.l_variable = &lValue;
    floatTest.r_variable = &rValue;

    result = adm_cds_compare(&floatTest, 1);
    EXPECT_EQ(result,0x01);
             
    return;     
}

TEST_F(CCdsLessTest, Normal)
{
    CCdsCompareTest sameTypeLessTest;
    sameTypeLessTest.cds_Less_test();
}

void CCdsCompareTest::cds_LessThan_test()
{
    cds_compare_st U8Test;
    U8Test.type = COMPARE_SYMBOL_LESSEQ;
    cds_variable_st lValue;
    lValue.data_type = VAR_U8;
    lValue.v.value[0]='2';
    lValue.data_length = 1;
    cds_variable_st rValue;
    rValue.data_type = VAR_U8;
    rValue.v.value[0]='2';
    rValue.data_length = 1;
    U8Test.l_variable = &lValue;
    U8Test.r_variable = &rValue;

    D8U result = adm_cds_compare(&U8Test, 1);
    EXPECT_EQ(result,0x01);
    
    cds_compare_st intTest;
    intTest.type = COMPARE_SYMBOL_LESSEQ;
    lValue.data_type = VAR_S32;
    lValue.v.value_s32=2;
    lValue.data_length = 1;
    rValue.data_type = VAR_S32;
    rValue.v.value_s32=2;
    rValue.data_length = 1;
    intTest.l_variable = &lValue;
    intTest.r_variable = &rValue;

    result = adm_cds_compare(&intTest, 1);
    EXPECT_EQ(result,0x01);
    
    cds_compare_st uintTest;
    uintTest.type = COMPARE_SYMBOL_LESSEQ;
    lValue.data_type = VAR_U32;
    lValue.v.value_u32=2;
    lValue.data_length = 1;
    rValue.data_type = VAR_U32;
    rValue.v.value_u32=2;
    rValue.data_length = 1;
    uintTest.l_variable = &lValue;
    uintTest.r_variable = &rValue;

    result = adm_cds_compare(&uintTest, 1);
    EXPECT_EQ(result,0x01);    

    cds_compare_st u64Test;
    u64Test.type = COMPARE_SYMBOL_LESSEQ;
    lValue.data_type = VAR_U64;
    lValue.v.value_u64=2;
    lValue.data_length = 1;
    rValue.data_type = VAR_U64;
    rValue.v.value_u64=2;
    rValue.data_length = 1;
    u64Test.l_variable = &lValue;
    u64Test.r_variable = &rValue;

    result = adm_cds_compare(&u64Test, 1);
    EXPECT_EQ(result,0x01);
    
    cds_compare_st floatTest;
    floatTest.type = COMPARE_SYMBOL_LESSEQ;
    lValue.data_type = VAR_FLOAT;
    lValue.v.value_f=2.0;
    lValue.data_length = 1;
    rValue.data_type = VAR_FLOAT;
    rValue.v.value_f=2.0;
    rValue.data_length = 1;
    floatTest.l_variable = &lValue;
    floatTest.r_variable = &rValue;

    result = adm_cds_compare(&floatTest, 1);
    EXPECT_EQ(result,0x01);    
}

TEST_F(CCdsLessThanTest, Normal)
{
    CCdsCompareTest sameTypeLessTest;
    sameTypeLessTest.cds_Less_test();
}

void CCdsCompareTest::cds_and_test()
{
    cds_compare_st U8Test[2];
    U8Test[0].type = COMPARE_SYMBOL_LESSEQ;
    cds_variable_st lValue;
    lValue.data_type = VAR_U8;
    lValue.v.value[0]='2';
    lValue.data_length = 1;
    cds_variable_st rValue;
    rValue.data_type = VAR_U8;
    rValue.v.value[0]='2';
    rValue.data_length = 1;
    U8Test[0].l_variable = &lValue;
    U8Test[0].r_variable = &rValue;
    U8Test[0].connect_symbol = COMPARE_AND;
    
    U8Test[1].type = COMPARE_SYMBOL_LESSEQ;

    lValue.data_type = VAR_U8;
    lValue.v.value[0]='2';
    lValue.data_length = 1;

    rValue.data_type = VAR_U8;
    rValue.v.value[0]='2';
    rValue.data_length = 1;
    U8Test[1].l_variable = &lValue;
    U8Test[1].r_variable = &rValue;
    U8Test[1].connect_symbol = COMPARE_AND;    

    D8U result = adm_cds_compare(U8Test, 2);
    EXPECT_EQ(result,0x01);
    
    cds_compare_st intTest[2];
    intTest[0].type = COMPARE_SYMBOL_LESSEQ;
    lValue.data_type = VAR_S32;
    lValue.v.value_s32=2;
    lValue.data_length = 1;
    rValue.data_type = VAR_S32;
    rValue.v.value_s32=2;
    rValue.data_length = 1;
    intTest[0].l_variable = &lValue;
    intTest[0].r_variable = &rValue;
    intTest[0].connect_symbol = COMPARE_AND; 
    
    intTest[1].type = COMPARE_SYMBOL_LESSEQ;
    cds_variable_st lValue1;
    cds_variable_st rValue1;
    lValue1.data_type = VAR_S32;
    lValue1.v.value_s32=2;
    lValue1.data_length = 1;
    rValue1.data_type = VAR_S32;
    rValue1.v.value_s32=2;
    rValue1.data_length = 1;
    intTest[1].l_variable = &lValue1;
    intTest[1].r_variable = &rValue1;
    intTest[1].connect_symbol = COMPARE_AND;    
    result = adm_cds_compare(intTest, 2);
    EXPECT_EQ(result,0x01);    
}

TEST_F(CCdsAndTest, Normal)
{
    CCdsCompareTest sameTypeAndTest;
    sameTypeAndTest.cds_and_test();
}

void CCdsCompareTest::cds_or_test()
{
    cds_compare_st U8Test[2];
    U8Test[0].type = COMPARE_SYMBOL_LESSEQ;
    cds_variable_st lValue;
    lValue.data_type = VAR_U8;
    lValue.v.value[0]='2';
    lValue.data_length = 1;
    cds_variable_st rValue;
    rValue.data_type = VAR_U8;
    rValue.v.value[0]='2';
    rValue.data_length = 1;
    U8Test[0].l_variable = &lValue;
    U8Test[0].r_variable = &rValue;
    U8Test[0].connect_symbol = COMPARE_OR;
    
    U8Test[1].type = COMPARE_SYMBOL_LESSEQ;

    lValue.data_type = VAR_U8;
    lValue.v.value[0]='2';
    lValue.data_length = 1;

    rValue.data_type = VAR_U8;
    rValue.v.value[0]='2';
    rValue.data_length = 1;
    U8Test[1].l_variable = &lValue;
    U8Test[1].r_variable = &rValue;
    U8Test[1].connect_symbol = COMPARE_OR;    

    D8U result = adm_cds_compare(U8Test, 2);
    EXPECT_EQ(result,0x01);
    
    cds_compare_st intTest[2];
    intTest[0].type = COMPARE_SYMBOL_LESSEQ;
    lValue.data_type = VAR_S32;
    lValue.v.value_s32=2;
    lValue.data_length = 1;
    rValue.data_type = VAR_S32;
    rValue.v.value_s32=2;
    rValue.data_length = 1;
    intTest[0].l_variable = &lValue;
    intTest[0].r_variable = &rValue;
    intTest[0].connect_symbol = COMPARE_OR; 
    
    intTest[1].type = COMPARE_SYMBOL_LESSEQ;
    cds_variable_st lValue1;
    cds_variable_st rValue1;
    lValue1.data_type = VAR_S32;
    lValue1.v.value_s32=2;
    lValue1.data_length = 1;
    rValue1.data_type = VAR_S32;
    rValue1.v.value_s32=2;
    rValue1.data_length = 1;
    intTest[1].l_variable = &lValue1;
    intTest[1].r_variable = &rValue1;
    intTest[1].connect_symbol = COMPARE_OR;    
    result = adm_cds_compare(intTest, 2);
    EXPECT_EQ(result,0x01);     
}

TEST_F(CCdsOrTest, Normal)
{
    CCdsCompareTest sameTypeOrTest;
    sameTypeOrTest.cds_or_test();
}

void CCdsCompareTest::cds_cross_int_uint_GeneralOrtest()
{
    cds_compare_st intTest[2];
    intTest[0].type = COMPARE_SYMBOL_GREATE;
    cds_variable_st lValue;
    lValue.data_type = VAR_S32;
    lValue.v.value_s32=2;
    lValue.data_length = 1;
    cds_variable_st rValue;
    rValue.data_type = VAR_U32;
    rValue.v.value_s32=2;
    rValue.data_length = 1;
    intTest[0].l_variable = &lValue;
    intTest[0].r_variable = &rValue;
    intTest[0].connect_symbol = COMPARE_OR; 
    
    intTest[1].type = COMPARE_SYMBOL_GREATE;
    cds_variable_st lValue1;
    cds_variable_st rValue1;
    lValue1.data_type = VAR_S32;
    lValue1.v.value_s32=2;
    lValue1.data_length = 1;
    rValue1.data_type = VAR_U32;
    rValue1.v.value_s32=2;
    rValue1.data_length = 1;
    intTest[1].l_variable = &lValue1;
    intTest[1].r_variable = &rValue1;
    intTest[1].connect_symbol = COMPARE_OR;    
    D8U result = adm_cds_compare(intTest, 2);
    EXPECT_EQ(result,0x00);    
}

TEST_F(CCdsCrossInit_Uint_General_Or_Test, Normal)
{
    CCdsCompareTest intUintGernalOrTest;
    intUintGernalOrTest.cds_cross_int_uint_GeneralOrtest();
}

void CCdsCompareTest::cds_cross_int_uint_GeneralAndtest()
{
    cds_compare_st intTest[2];
    intTest[0].type = COMPARE_SYMBOL_GREATE;
    cds_variable_st lValue;
    lValue.data_type = VAR_S32;
    lValue.v.value_s32=2;
    lValue.data_length = 1;
    cds_variable_st rValue;
    rValue.data_type = VAR_U32;
    rValue.v.value_s32=2;
    rValue.data_length = 1;
    intTest[0].l_variable = &lValue;
    intTest[0].r_variable = &rValue;
    intTest[0].connect_symbol = COMPARE_AND; 
    
    intTest[1].type = COMPARE_SYMBOL_GREATE;
    cds_variable_st lValue1;
    cds_variable_st rValue1;
    lValue1.data_type = VAR_S32;
    lValue1.v.value_s32=2;
    lValue1.data_length = 1;
    rValue1.data_type = VAR_U32;
    rValue1.v.value_s32=2;
    rValue1.data_length = 1;
    intTest[1].l_variable = &lValue1;
    intTest[1].r_variable = &rValue1;
    intTest[1].connect_symbol = COMPARE_AND;    
    D8U result = adm_cds_compare(intTest, 2);
    EXPECT_EQ(result,0x00);    
}

TEST_F(CCdsCrossInit_Uint_General_And_Test, Normal)
{
    CCdsCompareTest intUintGernalAndTest;
    intUintGernalAndTest.cds_cross_int_uint_GeneralAndtest();
}

void CCdsCompareTest::cds_cross_uint_u64_GeneralOrtest()
{
    cds_compare_st intTest[2];
    intTest[0].type = COMPARE_SYMBOL_GREATE;
    cds_variable_st lValue;
    lValue.data_type = VAR_U32;
    lValue.v.value_s32=2;
    lValue.data_length = 1;
    cds_variable_st rValue;
    rValue.data_type = VAR_U64;
    rValue.v.value_s32=2;
    rValue.data_length = 1;
    intTest[0].l_variable = &lValue;
    intTest[0].r_variable = &rValue;
    intTest[0].connect_symbol = COMPARE_OR; 

    intTest[1].type = COMPARE_SYMBOL_GREATE;
    cds_variable_st lValue1;
    cds_variable_st rValue1;
    lValue1.data_type = VAR_U32;
    lValue1.v.value_s32=2;
    lValue1.data_length = 1;
    rValue1.data_type = VAR_U64;
    rValue1.v.value_s32=2;
    rValue1.data_length = 1;
    intTest[1].l_variable = &lValue1;
    intTest[1].r_variable = &rValue1;
    intTest[1].connect_symbol = COMPARE_OR;    
    D8U result = adm_cds_compare(intTest, 2);
    EXPECT_EQ(result,0x00);    
}

TEST_F(CCdsCross_Uint_U64_General_Or_Test, Normal)
{
    CCdsCompareTest intUintGernalorTest;
    intUintGernalorTest.cds_cross_uint_u64_GeneralOrtest();
}

void CCdsCompareTest::cds_cross_uint_u64_GeneralAndtest()
{
    cds_compare_st intTest[2];
    intTest[0].type = COMPARE_SYMBOL_GREATE;
    cds_variable_st lValue;
    lValue.data_type = VAR_U32;
    lValue.v.value_u32=2;
    lValue.data_length = 1;
    cds_variable_st rValue;
    rValue.data_type = VAR_U64;
    rValue.v.value_u64=1;
    rValue.data_length = 1;
    intTest[0].l_variable = &lValue;
    intTest[0].r_variable = &rValue;
    intTest[0].connect_symbol = COMPARE_AND; 

    intTest[1].type = COMPARE_SYMBOL_GREATE;
    cds_variable_st lValue1;
    cds_variable_st rValue1;
    lValue1.data_type = VAR_U32;
    lValue1.v.value_u32=2;
    lValue1.data_length = 1;
    rValue1.data_type = VAR_U64;
    rValue1.v.value_u64=1;
    rValue1.data_length = 1;
    intTest[1].l_variable = &lValue1;
    intTest[1].r_variable = &rValue1;
    intTest[1].connect_symbol = COMPARE_AND;    
    D8U result = adm_cds_compare(intTest, 2);
    EXPECT_EQ(result,0x01);     
}

TEST_F(CCdsCross_Uint_U64_General_And_Test, Normal)
{
    CCdsCompareTest intUintGernalAndTest;
    intUintGernalAndTest.cds_cross_uint_u64_GeneralAndtest();
}

void CCdsCompareTest::cds_cross_int_float_Generalortest()
{
    cds_compare_st intTest[2];
    intTest[0].type = COMPARE_SYMBOL_GREATE;
    cds_variable_st lValue;
    lValue.data_type = VAR_U32;
    lValue.v.value_s32=2;
    lValue.data_length = 1;
    cds_variable_st rValue;
    rValue.data_type = VAR_FLOAT;
    rValue.v.value_f=1.0;
    rValue.data_length = 1;
    intTest[0].l_variable = &lValue;
    intTest[0].r_variable = &rValue;
    intTest[0].connect_symbol = COMPARE_OR; 

    intTest[1].type = COMPARE_SYMBOL_GREATE;
    cds_variable_st lValue1;
    cds_variable_st rValue1;
    lValue1.data_type = VAR_U32;
    lValue1.v.value_u32=2;
    lValue1.data_length = 1;
    rValue1.data_type = VAR_FLOAT;
    rValue1.v.value_f=1.0;
    rValue1.data_length = 1;
    intTest[1].l_variable = &lValue1;
    intTest[1].r_variable = &rValue1;
    intTest[1].connect_symbol = COMPARE_OR;    
    D8U result = adm_cds_compare(intTest, 2);
    EXPECT_EQ(result,0x01);    
}

TEST_F(CCdsCross_int_float_General_Or_Test, Normal)
{
    CCdsCompareTest aa;
    aa.cds_cross_int_float_Generalortest();
}

void CCdsCompareTest::cds_cross_int_float_GeneralAndtest()
{
    cds_compare_st intTest[2];
    intTest[0].type = COMPARE_SYMBOL_GREATE;
    cds_variable_st lValue;
    lValue.data_type = VAR_U32;
    lValue.v.value_s32=2;
    lValue.data_length = 1;
    cds_variable_st rValue;
    rValue.data_type = VAR_FLOAT;
    rValue.v.value_f=1.0;
    rValue.data_length = 1;
    intTest[0].l_variable = &lValue;
    intTest[0].r_variable = &rValue;
    intTest[0].connect_symbol = COMPARE_AND; 

    intTest[1].type = COMPARE_SYMBOL_GREATE;
    cds_variable_st lValue1;
    cds_variable_st rValue1;
    lValue1.data_type = VAR_U32;
    lValue1.v.value_u32=2;
    lValue1.data_length = 1;
    rValue1.data_type = VAR_FLOAT;
    rValue1.v.value_f=1.0;
    rValue1.data_length = 1;
    intTest[1].l_variable = &lValue1;
    intTest[1].r_variable = &rValue1;
    intTest[1].connect_symbol = COMPARE_AND;    
    D8U result = adm_cds_compare(intTest, 2);
    EXPECT_EQ(result,0x01);    
}

TEST_F(CCdsCross_int_float_General_And_Test, Normal)
{
    CCdsCompareTest aa;
    aa.cds_cross_int_float_GeneralAndtest();
}

void CCdsCompareTest::cds_cross_int_uint_GeneralEQ_Andtest()
{
    cds_compare_st intTest[2];
    intTest[0].type = COMPARE_SYMBOL_GREATEEQ;
    cds_variable_st lValue;
    lValue.data_type = VAR_U32;
    lValue.v.value_s32=2;
    lValue.data_length = 1;
    cds_variable_st rValue;
    rValue.data_type = VAR_U32;
    rValue.v.value_u32=1;
    rValue.data_length = 1;
    intTest[0].l_variable = &lValue;
    intTest[0].r_variable = &rValue;
    intTest[0].connect_symbol = COMPARE_AND; 

    intTest[1].type = COMPARE_SYMBOL_GREATEEQ;
    cds_variable_st lValue1;
    cds_variable_st rValue1;
    lValue1.data_type = VAR_U32;
    lValue1.v.value_s32=2;
    lValue1.data_length = 1;
    rValue1.data_type = VAR_U32;
    rValue1.v.value_u32=1;
    rValue1.data_length = 1;
    intTest[1].l_variable = &lValue1;
    intTest[1].r_variable = &rValue1;
    intTest[1].connect_symbol = COMPARE_AND;    
    D8U result = adm_cds_compare(intTest, 2);
    EXPECT_EQ(result,0x01);    
}

TEST_F(CCdsCross_int_uint_GeneralEQ_And_Test, Normal)
{
    CCdsCompareTest aa;
    aa.cds_cross_int_uint_GeneralEQ_Andtest();
}

void CCdsCompareTest::cds_cross_int_uint_GeneralEQ_Ortest()
{
    cds_compare_st intTest[2];
    intTest[0].type = COMPARE_SYMBOL_GREATEEQ;
    cds_variable_st lValue;
    lValue.data_type = VAR_U32;
    lValue.v.value_s32=2;
    lValue.data_length = 1;
    cds_variable_st rValue;
    rValue.data_type = VAR_U32;
    rValue.v.value_u32=1;
    rValue.data_length = 1;
    intTest[0].l_variable = &lValue;
    intTest[0].r_variable = &rValue;
    intTest[0].connect_symbol = COMPARE_OR; 

    intTest[1].type = COMPARE_SYMBOL_GREATEEQ;
    cds_variable_st lValue1;
    lValue1.data_type = VAR_U32;
    lValue1.v.value_s32=2;
    lValue1.data_length = 1;
    cds_variable_st rValue1;
    rValue1.data_type = VAR_U32;
    rValue1.v.value_u32=1;
    rValue1.data_length = 1;
    intTest[1].l_variable = &lValue1;
    intTest[1].r_variable = &rValue1;
    intTest[1].connect_symbol = COMPARE_OR;    
    D8U result = adm_cds_compare(intTest, 2);
    EXPECT_EQ(result,0x01);     
}

TEST_F(CCdsCross_int_uint_GeneralEQ_Or_Test, Normal)
{
    CCdsCompareTest aa;
    aa.cds_cross_int_uint_GeneralEQ_Ortest();
}

void CCdsCompareTest::cds_cross_int_u64_GeneralEQ_Andtest()
{
    cds_compare_st intTest[2];
    intTest[0].type = COMPARE_SYMBOL_GREATEEQ;
    cds_variable_st lValue;
    lValue.data_type = VAR_S32;
    lValue.v.value_s32=2;
    lValue.data_length = 1;
    cds_variable_st rValue;
    rValue.data_type = VAR_U64;
    rValue.v.value_u64=1;
    rValue.data_length = 1;
    intTest[0].l_variable = &lValue;
    intTest[0].r_variable = &rValue;
    intTest[0].connect_symbol = COMPARE_AND; 

    intTest[1].type = COMPARE_SYMBOL_GREATEEQ;
    cds_variable_st lValue1;
    cds_variable_st rValue1;
    lValue1.data_type = VAR_S32;
    lValue1.v.value_s32=2;
    lValue1.data_length = 1;
    
    rValue1.data_type = VAR_U64;
    rValue1.v.value_u64=1;
    rValue1.data_length = 1;
    intTest[1].l_variable = &lValue1;
    intTest[1].r_variable = &rValue1;
    intTest[1].connect_symbol = COMPARE_AND;    
    D8U result = adm_cds_compare(intTest, 2);
    EXPECT_EQ(result,0x01);      
}

TEST_F(CCdsCross_int_u64_GeneralEQ_And_Test, Normal)
{
    CCdsCompareTest aa;
    aa.cds_cross_int_u64_GeneralEQ_Andtest();
}

void CCdsCompareTest::cds_cross_int_u64_GeneralEQ_Ortest()
{
    cds_compare_st intTest[2];
    intTest[0].type = COMPARE_SYMBOL_GREATEEQ;
    cds_variable_st lValue;
    lValue.data_type = VAR_S32;
    lValue.v.value_s32=2;
    lValue.data_length = 1;
    cds_variable_st rValue;
    rValue.data_type = VAR_U64;
    rValue.v.value_u64=1;
    rValue.data_length = 1;
    intTest[0].l_variable = &lValue;
    intTest[0].r_variable = &rValue;
    intTest[0].connect_symbol = COMPARE_OR; 

    intTest[1].type = COMPARE_SYMBOL_GREATEEQ;
    cds_variable_st lValue1;
    cds_variable_st rValue1;
    lValue1.data_type = VAR_S32;
    lValue1.v.value_s32=2;
    lValue1.data_length = 1;
    
    rValue1.data_type = VAR_U64;
    rValue1.v.value_u64=1;
    rValue1.data_length = 1;
    intTest[1].l_variable = &lValue1;
    intTest[1].r_variable = &rValue1;
    intTest[1].connect_symbol = COMPARE_OR;    
    D8U result = adm_cds_compare(intTest, 2);
    EXPECT_EQ(result,0x01);     
}

TEST_F(CCdsCross_int_u64_GeneralEQ_Or_Test, Normal)
{
    CCdsCompareTest aa;
    aa.cds_cross_int_u64_GeneralEQ_Ortest();
}

void CCdsCompareTest::cds_cross_uint_u64_GeneralEQ_Andtest()
{
    cds_compare_st intTest[2];
    intTest[0].type = COMPARE_SYMBOL_GREATEEQ;
    cds_variable_st lValue;
    lValue.data_type = VAR_U32;
    lValue.v.value_u32=2;
    lValue.data_length = 1;
    cds_variable_st rValue;
    rValue.data_type = VAR_U64;
    rValue.v.value_u64=1;
    rValue.data_length = 1;
    intTest[0].l_variable = &lValue;
    intTest[0].r_variable = &rValue;
    intTest[0].connect_symbol = COMPARE_AND; 

    intTest[1].type = COMPARE_SYMBOL_GREATEEQ;
    cds_variable_st lValue1;
    cds_variable_st rValue1;
    lValue1.data_type = VAR_U32;
    lValue1.v.value_u32=2;
    lValue1.data_length = 1;
    
    rValue1.data_type = VAR_U64;
    rValue1.v.value_u64=1;
    rValue1.data_length = 1;
    intTest[1].l_variable = &lValue1;
    intTest[1].r_variable = &rValue1;
    intTest[1].connect_symbol = COMPARE_AND;    
    D8U result = adm_cds_compare(intTest, 2);
    EXPECT_EQ(result,0x01);    
}

TEST_F(CCdsCross_uint_u64_GeneralEQ_And_Test, Normal)
{
    CCdsCompareTest aa;
    aa.cds_cross_uint_u64_GeneralEQ_Andtest();
}

void CCdsCompareTest::cds_cross_uint_u64_GeneralEQ_Ortest()
{
    cds_compare_st intTest[2];
    intTest[0].type = COMPARE_SYMBOL_GREATEEQ;
    cds_variable_st lValue;
    lValue.data_type = VAR_U32;
    lValue.v.value_u32=2;
    lValue.data_length = 1;
    cds_variable_st rValue;
    rValue.data_type = VAR_U64;
    rValue.v.value_u64=1;
    rValue.data_length = 1;
    intTest[0].l_variable = &lValue;
    intTest[0].r_variable = &rValue;
    intTest[0].connect_symbol = COMPARE_OR; 

    intTest[1].type = COMPARE_SYMBOL_GREATEEQ;
    cds_variable_st lValue1;
    cds_variable_st rValue1;
    lValue1.data_type = VAR_U32;
    lValue1.v.value_u32=2;
    lValue1.data_length = 1;
    
    rValue1.data_type = VAR_U64;
    rValue1.v.value_u64=1;
    rValue1.data_length = 1;
    intTest[1].l_variable = &lValue1;
    intTest[1].r_variable = &rValue1;
    intTest[1].connect_symbol = COMPARE_OR;    
    D8U result = adm_cds_compare(intTest, 2);
    EXPECT_EQ(result,0x01);    
}

TEST_F(CCdsCross_uint_u64_GeneralEQ_Or_Test, Normal)
{
    CCdsCompareTest aa;
    aa.cds_cross_uint_u64_GeneralEQ_Ortest();
}

void CCdsCompareTest::cds_cross_char_string_EQ_Test()
{
    cds_compare_st intTest;
    intTest.type = COMPARE_SYMBOL_EQ;
    cds_variable_st lValue;
    lValue.data_type = VAR_U8;
    lValue.v.value[0]='2';
    lValue.data_length = 1;
    cds_variable_st rValue;
    rValue.data_type = VAR_STRING;
    DCHAR* pp = const_cast<DCHAR*>("2");
    
    rValue.v.value_s=reinterpret_cast<D8U*>(pp);
    rValue.data_length = 1;
    intTest.l_variable = &lValue;
    intTest.r_variable = &rValue;

    D8U result = adm_cds_compare(&intTest, 1);
    EXPECT_EQ(result,0x00);    
}

TEST_F(CCdsCross_char_string_EQ_Test, Normal)
{
    CCdsCompareTest aa;
    aa.cds_cross_char_string_EQ_Test();
}

void CCdsCompareTest::cds_cross_char_string_EQ_And_Test()
{
    cds_compare_st intTest[2];
    intTest[0].type = COMPARE_SYMBOL_EQ;
    cds_variable_st lValue;
    lValue.data_type = VAR_U8;
    lValue.v.value[0]='2';
    lValue.data_length = 1;
    cds_variable_st rValue;
    rValue.data_type = VAR_STRING;
    DCHAR* pp = const_cast<DCHAR*>("2");
    
    rValue.v.value_s=reinterpret_cast<D8U*>(pp);
    rValue.data_length = 1;
    intTest[0].l_variable = &lValue;
    intTest[0].r_variable = &rValue;
    intTest[0].connect_symbol = COMPARE_AND;
    
    cds_variable_st lValue1;
    cds_variable_st rValue1;
    intTest[1].type = COMPARE_SYMBOL_EQ;
    lValue1.data_type = VAR_U8;
    lValue1.v.value[0]='2';
    lValue1.data_length = 1;
    rValue1.data_type = VAR_STRING;
    DCHAR* pp1 = const_cast<DCHAR*>("2");
    
    rValue1.v.value_s=reinterpret_cast<D8U*>(pp1);
    rValue1.data_length = 1;
    intTest[0].l_variable = &lValue1;
    intTest[0].r_variable = &rValue1;    
    intTest[1].connect_symbol = COMPARE_AND;

    D8U result = adm_cds_compare(intTest, 2);
    EXPECT_EQ(result,0x00);
}

TEST_F(CCdsCross_char_string_EQ_And_Test, Normal)
{
    CCdsCompareTest aa;
    aa.cds_cross_char_string_EQ_And_Test();
}

void CCdsCompareTest::cds_cross_char_string_EQ_Or_Test()
{
    cds_compare_st intTest[2];
    intTest[0].type = COMPARE_SYMBOL_EQ;
    cds_variable_st lValue;
    lValue.data_type = VAR_U8;
    lValue.v.value[0]='2';
    lValue.data_length = 1;
    cds_variable_st rValue;
    rValue.data_type = VAR_STRING;
    DCHAR* pp = const_cast<DCHAR*>("2");
    
    rValue.v.value_s=reinterpret_cast<D8U*>(pp);
    rValue.data_length = 1;
    intTest[0].l_variable = &lValue;
    intTest[0].r_variable = &rValue;
    intTest[0].connect_symbol = COMPARE_OR;
    
    cds_variable_st lValue1;
    cds_variable_st rValue1;
    intTest[1].type = COMPARE_SYMBOL_EQ;
    lValue1.data_type = VAR_U8;
    lValue1.v.value[0]='2';
    lValue1.data_length = 1;
    rValue1.data_type = VAR_STRING;
    DCHAR* pp1 = const_cast<DCHAR*>("2");
    
    rValue1.v.value_s=reinterpret_cast<D8U*>(pp1);
    rValue1.data_length = 1;
    intTest[0].l_variable = &lValue1;
    intTest[0].r_variable = &rValue1;    
    intTest[1].connect_symbol = COMPARE_OR;

    D8U result = adm_cds_compare(intTest, 2);
    EXPECT_EQ(result,0x00);
}

TEST_F(CCdsCross_char_string_EQ_Or_Test, Normal)
{
    CCdsCompareTest aa;
    aa.cds_cross_char_string_EQ_Or_Test();
}

void CCdsCompareTest::cds_cross_int_uint_EQ_Test()
{
    cds_compare_st intTest;
    intTest.type = COMPARE_SYMBOL_EQ;
    cds_variable_st lValue;
    lValue.data_type = VAR_S32;
    lValue.v.value_s32=2;
    lValue.data_length = 1;
    cds_variable_st rValue;
    rValue.data_type = VAR_U32;
    
    rValue.v.value_u32=2;
    rValue.data_length = 1;
    intTest.l_variable = &lValue;
    intTest.r_variable = &rValue;

    D8U result = adm_cds_compare(&intTest, 1);
    EXPECT_EQ(result,0x01);     
}

TEST_F(CCdsCross_int_uint_EQ_Test, Normal)
{
    CCdsCompareTest aa;
    aa.cds_cross_int_uint_EQ_Test();
}

void CCdsCompareTest::cds_cross_int_uint_EQ_AND_Test()
{
    cds_compare_st intTest[2];
    intTest[0].type = COMPARE_SYMBOL_EQ;
    cds_variable_st lValue;
    lValue.data_type = VAR_S32;
    lValue.v.value_s32=2;
    lValue.data_length = 1;
    cds_variable_st rValue;
    rValue.data_type = VAR_U32;
    
    rValue.v.value_u32=2;
    rValue.data_length = 1;
    intTest[0].l_variable = &lValue;
    intTest[0].r_variable = &rValue;
    intTest[0].connect_symbol = COMPARE_AND;

    intTest[1].type = COMPARE_SYMBOL_EQ;
    cds_variable_st lValue1;
    lValue1.data_type = VAR_S32;
    lValue1.v.value_s32=2;
    lValue1.data_length = 1;
    cds_variable_st rValue1;
    rValue1.data_type = VAR_U32;
    rValue1.v.value_u32=2;
    rValue.data_length = 1;
    intTest[1].l_variable = &lValue1;
    intTest[1].r_variable = &rValue1;
    intTest[1].connect_symbol = COMPARE_AND;

    D8U result = adm_cds_compare(intTest, 2);
    EXPECT_EQ(result,0x01);     
}

TEST_F(CCdsCross_int_uint_EQ_AND_Test, Normal)
{
    CCdsCompareTest aa;
    aa.cds_cross_int_uint_EQ_AND_Test();
}

void CCdsCompareTest::cds_cross_int_uint_EQ_OR_Test()
{
    cds_compare_st intTest[2];
    intTest[0].type = COMPARE_SYMBOL_EQ;
    cds_variable_st lValue;
    lValue.data_type = VAR_S32;
    lValue.v.value_s32=2;
    lValue.data_length = 1;
    cds_variable_st rValue;
    rValue.data_type = VAR_U32;
    
    rValue.v.value_u32=2;
    rValue.data_length = 1;
    intTest[0].l_variable = &lValue;
    intTest[0].r_variable = &rValue;
    intTest[0].connect_symbol = COMPARE_OR;

    intTest[1].type = COMPARE_SYMBOL_EQ;
    cds_variable_st lValue1;
    lValue1.data_type = VAR_S32;
    lValue1.v.value_s32=2;
    lValue1.data_length = 1;
    cds_variable_st rValue1;
    rValue1.data_type = VAR_U32;
    rValue1.v.value_u32=2;
    rValue.data_length = 1;
    intTest[1].l_variable = &lValue1;
    intTest[1].r_variable = &rValue1;
    intTest[1].connect_symbol = COMPARE_OR;

    D8U result = adm_cds_compare(intTest, 2);
    EXPECT_EQ(result,0x01);     
}

TEST_F(CCdsCross_int_uint_EQ_OR_Test, Normal)
{
    CCdsCompareTest aa;
    aa.cds_cross_int_uint_EQ_OR_Test();
}

void CCdsCompareTest::cds_cross_int_u64_EQ_Test()
{
    cds_compare_st intTest;
    intTest.type = COMPARE_SYMBOL_EQ;
    cds_variable_st lValue;
    lValue.data_type = VAR_S32;
    lValue.v.value_s32=2;
    lValue.data_length = 1;
    cds_variable_st rValue;
    rValue.data_type = VAR_U64;
    
    rValue.v.value_u64=2;
    rValue.data_length = 1;
    intTest.l_variable = &lValue;
    intTest.r_variable = &rValue;

    D8U result = adm_cds_compare(&intTest, 1);
    EXPECT_EQ(result,0x01);     
}

TEST_F(CCdsCross_int_u64_EQ_Test, Normal)
{
    CCdsCompareTest aa;
    aa.cds_cross_int_u64_EQ_Test();
}

void CCdsCompareTest::cds_cross_int_u64_EQ_And_Test()
{
    cds_compare_st intTest[2];
    intTest[0].type = COMPARE_SYMBOL_EQ;
    cds_variable_st lValue;
    lValue.data_type = VAR_S32;
    lValue.v.value_s32=2;
    lValue.data_length = 1;
    cds_variable_st rValue;
    rValue.data_type = VAR_U64;
    
    rValue.v.value_u64=2;
    rValue.data_length = 1;
    intTest[0].l_variable = &lValue;
    intTest[0].r_variable = &rValue;
    intTest[0].connect_symbol = COMPARE_AND;

    intTest[1].type = COMPARE_SYMBOL_EQ;
    cds_variable_st lValue1;
    lValue1.data_type = VAR_S32;
    lValue1.v.value_s32=2;
    lValue1.data_length = 1;
    cds_variable_st rValue1;
    rValue1.data_type = VAR_U64;
    rValue1.v.value_u64=2;
    rValue.data_length = 1;
    intTest[1].l_variable = &lValue1;
    intTest[1].r_variable = &rValue1;
    intTest[1].connect_symbol = COMPARE_AND;

    D8U result = adm_cds_compare(intTest, 2);
    EXPECT_EQ(result,0x01);    
}

TEST_F(CCdsCross_int_u64_EQ_AND_Test, Normal)
{
    CCdsCompareTest aa;
    aa.cds_cross_int_u64_EQ_And_Test();
}

void CCdsCompareTest::cds_cross_int_u64_EQ_OR_Test()
{
    cds_compare_st intTest[2];
    intTest[0].type = COMPARE_SYMBOL_EQ;
    cds_variable_st lValue;
    lValue.data_type = VAR_S32;
    lValue.v.value_s32=2;
    lValue.data_length = 1;
    cds_variable_st rValue;
    rValue.data_type = VAR_U64;
    
    rValue.v.value_u64=2;
    rValue.data_length = 1;
    intTest[0].l_variable = &lValue;
    intTest[0].r_variable = &rValue;
    intTest[0].connect_symbol = COMPARE_OR;

    intTest[1].type = COMPARE_SYMBOL_EQ;
    cds_variable_st lValue1;
    lValue1.data_type = VAR_S32;
    lValue1.v.value_s32=2;
    lValue1.data_length = 1;
    cds_variable_st rValue1;
    rValue1.data_type = VAR_U64;
    rValue1.v.value_u64=2;
    rValue.data_length = 1;
    intTest[1].l_variable = &lValue1;
    intTest[1].r_variable = &rValue1;
    intTest[1].connect_symbol = COMPARE_OR;

    D8U result = adm_cds_compare(intTest, 2);
    EXPECT_EQ(result,0x01);        
}

TEST_F(CCdsCross_int_u64_EQ_OR_Test, Normal)
{
    CCdsCompareTest aa;
    aa.cds_cross_int_u64_EQ_OR_Test();
}

void CCdsCompareTest::cds_cross_uint_u64_EQ_Test()
{
    cds_compare_st intTest;
    intTest.type = COMPARE_SYMBOL_EQ;
    cds_variable_st lValue;
    lValue.data_type = VAR_U32;
    lValue.v.value_u32=2;
    lValue.data_length = 1;
    cds_variable_st rValue;
    rValue.data_type = VAR_U64;
    
    rValue.v.value_u64=2;
    rValue.data_length = 1;
    intTest.l_variable = &lValue;
    intTest.r_variable = &rValue;

    D8U result = adm_cds_compare(&intTest, 1);
    EXPECT_EQ(result,0x01);    
}

TEST_F(CCdsCross_uint_u64_EQ_Test, Normal)
{
    CCdsCompareTest aa;
    aa.cds_cross_uint_u64_EQ_Test();
}

void CCdsCompareTest::cds_cross_uint_u64_EQ_And_Test()
{
    cds_compare_st intTest[2];
    intTest[0].type = COMPARE_SYMBOL_EQ;
    cds_variable_st lValue;
    lValue.data_type = VAR_U32;
    lValue.v.value_u32=2;
    lValue.data_length = 1;
    cds_variable_st rValue;
    rValue.data_type = VAR_U64;
    
    rValue.v.value_u64=2;
    rValue.data_length = 1;
    intTest[0].l_variable = &lValue;
    intTest[0].r_variable = &rValue;
    intTest[0].connect_symbol = COMPARE_AND;

    intTest[1].type = COMPARE_SYMBOL_EQ;
    cds_variable_st lValue1;
    lValue1.data_type = VAR_U32;
    lValue1.v.value_u32=2;
    lValue1.data_length = 1;
    cds_variable_st rValue1;
    rValue1.data_type = VAR_U64;
    rValue1.v.value_u64=2;
    rValue.data_length = 1;
    intTest[1].l_variable = &lValue1;
    intTest[1].r_variable = &rValue1;
    intTest[1].connect_symbol = COMPARE_AND;

    D8U result = adm_cds_compare(intTest, 2);
    EXPECT_EQ(result,0x01);     
}

TEST_F(CCdsCross_uint_u64_EQ_And_Test, Normal)
{
    CCdsCompareTest aa;
    aa.cds_cross_uint_u64_EQ_And_Test();
}

void CCdsCompareTest::cds_cross_uint_u64_EQ_Or_Test()
{
    cds_compare_st intTest[2];
    intTest[0].type = COMPARE_SYMBOL_EQ;
    cds_variable_st lValue;
    lValue.data_type = VAR_U32;
    lValue.v.value_u32=2;
    lValue.data_length = 1;
    cds_variable_st rValue;
    rValue.data_type = VAR_U64;
    
    rValue.v.value_u64=2;
    rValue.data_length = 1;
    intTest[0].l_variable = &lValue;
    intTest[0].r_variable = &rValue;
    intTest[0].connect_symbol = COMPARE_OR;

    intTest[1].type = COMPARE_SYMBOL_EQ;
    cds_variable_st lValue1;
    lValue1.data_type = VAR_U32;
    lValue1.v.value_u32=2;
    lValue1.data_length = 1;
    cds_variable_st rValue1;
    rValue1.data_type = VAR_U64;
    rValue1.v.value_u64=2;
    rValue.data_length = 1;
    intTest[1].l_variable = &lValue1;
    intTest[1].r_variable = &rValue1;
    intTest[1].connect_symbol = COMPARE_OR;

    D8U result = adm_cds_compare(intTest, 2);
    EXPECT_EQ(result,0x01);    
}

TEST_F(CCdsCross_uint_u64_EQ_Or_Test, Normal)
{
    CCdsCompareTest aa;
    aa.cds_cross_uint_u64_EQ_Or_Test();
}

void CCdsCompareTest::cds_cross_int_uint_NotEQ_Test()
{
    cds_compare_st intTest;
    intTest.type = COMPARE_SYMBOL_NOTEQ;
    cds_variable_st lValue;
    lValue.data_type = VAR_S32;
    lValue.v.value_s32=2;
    lValue.data_length = 1;
    cds_variable_st rValue;
    rValue.data_type = VAR_U32;
    
    rValue.v.value_u32=1;
    rValue.data_length = 1;
    intTest.l_variable = &lValue;
    intTest.r_variable = &rValue;

    D8U result = adm_cds_compare(&intTest, 1);
    EXPECT_EQ(result,0x01);
}

TEST_F(CCdsCross_int_uint_NotEQ_Test, Normal)
{
    CCdsCompareTest aa;
    aa.cds_cross_int_uint_NotEQ_Test();
}

void CCdsCompareTest::cds_cross_int_uint_NotEQ_And_Test()
{
    cds_compare_st intTest[2];
    intTest[0].type = COMPARE_SYMBOL_NOTEQ;
    cds_variable_st lValue;
    lValue.data_type = VAR_S32;
    lValue.v.value_s32=2;
    lValue.data_length = 1;
    cds_variable_st rValue;
    rValue.data_type = VAR_U32;
    
    rValue.v.value_u32=1;
    rValue.data_length = 1;
    intTest[0].l_variable = &lValue;
    intTest[0].r_variable = &rValue;
    intTest[0].connect_symbol = COMPARE_AND;

    intTest[1].type = COMPARE_SYMBOL_NOTEQ;
    cds_variable_st lValue1;
    lValue1.data_type = VAR_S32;
    lValue1.v.value_s32=2;
    lValue1.data_length = 1;
    cds_variable_st rValue1;
    rValue1.data_type = VAR_U32;
    
    rValue1.v.value_u32=1;
    rValue1.data_length = 1;
    intTest[1].l_variable = &lValue;
    intTest[1].r_variable = &rValue;
    intTest[1].connect_symbol = COMPARE_AND;

    D8U result = adm_cds_compare(intTest, 2);
    EXPECT_EQ(result,0x01);    
}

TEST_F(CCdsCross_int_uint_NotEQ_And_Test, Normal)
{
    CCdsCompareTest aa;
    aa.cds_cross_int_uint_NotEQ_And_Test();
}

void CCdsCompareTest::cds_cross_int_uint_NotEQ_Or_Test()
{
    cds_compare_st intTest[2];
    intTest[0].type = COMPARE_SYMBOL_NOTEQ;
    cds_variable_st lValue;
    lValue.data_type = VAR_S32;
    lValue.v.value_s32=2;
    lValue.data_length = 1;
    cds_variable_st rValue;
    rValue.data_type = VAR_U32;
    
    rValue.v.value_u32=1;
    rValue.data_length = 1;
    intTest[0].l_variable = &lValue;
    intTest[0].r_variable = &rValue;
    intTest[0].connect_symbol = COMPARE_OR;

    intTest[1].type = COMPARE_SYMBOL_NOTEQ;
    cds_variable_st lValue1;
    lValue1.data_type = VAR_S32;
    lValue1.v.value_s32=2;
    lValue1.data_length = 1;
    cds_variable_st rValue1;
    rValue1.data_type = VAR_U32;
    
    rValue1.v.value_u32=1;
    rValue1.data_length = 1;
    intTest[1].l_variable = &lValue;
    intTest[1].r_variable = &rValue;
    intTest[0].connect_symbol = COMPARE_OR;

    D8U result = adm_cds_compare(intTest, 2);
    EXPECT_EQ(result,0x01);     
}

TEST_F(CCdsCross_int_uint_NotEQ_Or_Test, Normal)
{
    CCdsCompareTest aa;
    aa.cds_cross_int_uint_NotEQ_Or_Test();
}

void CCdsCompareTest::cds_cross_int_u64_NotEQ_Test()
{
    cds_compare_st intTest;
    intTest.type = COMPARE_SYMBOL_NOTEQ;
    cds_variable_st lValue;
    lValue.data_type = VAR_S32;
    lValue.v.value_s32=2;
    lValue.data_length = 1;
    cds_variable_st rValue;
    rValue.data_type = VAR_U64;
    
    rValue.v.value_u64=1;
    rValue.data_length = 1;
    intTest.l_variable = &lValue;
    intTest.r_variable = &rValue;

    D8U result = adm_cds_compare(&intTest, 1);
    EXPECT_EQ(result,0x01);    
}

TEST_F(CCdsCross_int_u64_NotEQ_Test, Normal)
{
    CCdsCompareTest aa;
    aa.cds_cross_int_u64_NotEQ_Test();
}

void CCdsCompareTest::cds_cross_int_u64_NotEQ_And_Test()
{
    cds_compare_st intTest[2];
    intTest[0].type = COMPARE_SYMBOL_NOTEQ;
    cds_variable_st lValue;
    lValue.data_type = VAR_S32;
    lValue.v.value_s32=2;
    lValue.data_length = 1;
    cds_variable_st rValue;
    rValue.data_type = VAR_U64;
    
    rValue.v.value_u64=1;
    rValue.data_length = 1;
    intTest[0].l_variable = &lValue;
    intTest[0].r_variable = &rValue;
    intTest[0].connect_symbol = COMPARE_AND;

    intTest[1].type = COMPARE_SYMBOL_NOTEQ;
    cds_variable_st lValue1;
    lValue1.data_type = VAR_S32;
    lValue1.v.value_s32=2;
    lValue1.data_length = 1;
    cds_variable_st rValue1;
    rValue1.data_type = VAR_U64;
    
    rValue1.v.value_u64=1;
    rValue1.data_length = 1;
    intTest[1].l_variable = &lValue;
    intTest[1].r_variable = &rValue;
    intTest[1].connect_symbol = COMPARE_AND;

    D8U result = adm_cds_compare(intTest, 2);
    EXPECT_EQ(result,0x01);    
}

TEST_F(CCdsCross_int_u64_NotEQ_And_Test, Normal)
{
    CCdsCompareTest aa;
    aa.cds_cross_int_u64_NotEQ_And_Test();
}

void CCdsCompareTest::cds_cross_int_u64_NotEQ_Or_Test()
{
    cds_compare_st intTest[2];
    intTest[0].type = COMPARE_SYMBOL_NOTEQ;
    cds_variable_st lValue;
    lValue.data_type = VAR_S32;
    lValue.v.value_s32=2;
    lValue.data_length = 1;
    cds_variable_st rValue;
    rValue.data_type = VAR_U64;
    
    rValue.v.value_u64=1;
    rValue.data_length = 1;
    intTest[0].l_variable = &lValue;
    intTest[0].r_variable = &rValue;
    intTest[0].connect_symbol = COMPARE_OR;

    intTest[1].type = COMPARE_SYMBOL_NOTEQ;
    cds_variable_st lValue1;
    lValue1.data_type = VAR_S32;
    lValue1.v.value_s32=2;
    lValue1.data_length = 1;
    cds_variable_st rValue1;
    rValue1.data_type = VAR_U64;
    
    rValue1.v.value_u64=1;
    rValue1.data_length = 1;
    intTest[1].l_variable = &lValue;
    intTest[1].r_variable = &rValue;
    intTest[1].connect_symbol = COMPARE_OR;

    D8U result = adm_cds_compare(intTest, 2);
    EXPECT_EQ(result,0x01);     
}

TEST_F(CCdsCross_int_u64_NotEQ_Or_Test, Normal)
{
    CCdsCompareTest aa;
    aa.cds_cross_int_u64_NotEQ_Or_Test();
}

void CCdsCompareTest::cds_cross_uint_u64_NotEQ_Test()
{
    cds_compare_st intTest;
    intTest.type = COMPARE_SYMBOL_NOTEQ;
    cds_variable_st lValue;
    lValue.data_type = VAR_U32;
    lValue.v.value_u32=2;
    lValue.data_length = 1;
    cds_variable_st rValue;
    rValue.data_type = VAR_U64;
    
    rValue.v.value_u64=1;
    rValue.data_length = 1;
    intTest.l_variable = &lValue;
    intTest.r_variable = &rValue;

    D8U result = adm_cds_compare(&intTest, 1);
    EXPECT_EQ(result,0x01);     
}

TEST_F(CCdsCross_uint_u64_NotEQ_Test, Normal)
{
    CCdsCompareTest aa;
    aa.cds_cross_uint_u64_NotEQ_Test();
}

void CCdsCompareTest::cds_cross_uint_u64_NotEQ_And_Test()
{
    cds_compare_st intTest[2];
    intTest[0].type = COMPARE_SYMBOL_NOTEQ;
    cds_variable_st lValue;
    lValue.data_type = VAR_U32;
    lValue.v.value_u32=2;
    lValue.data_length = 1;
    cds_variable_st rValue;
    rValue.data_type = VAR_U64;
    
    rValue.v.value_u64=1;
    rValue.data_length = 1;
    intTest[0].l_variable = &lValue;
    intTest[0].r_variable = &rValue;
    intTest[0].connect_symbol = COMPARE_AND;

    intTest[1].type = COMPARE_SYMBOL_NOTEQ;
    cds_variable_st lValue1;
    lValue1.data_type = VAR_U32;
    lValue1.v.value_u32=2;
    lValue1.data_length = 1;
    cds_variable_st rValue1;
    rValue1.data_type = VAR_U64;
    
    rValue1.v.value_u64=1;
    rValue1.data_length = 1;
    intTest[1].l_variable = &lValue;
    intTest[1].r_variable = &rValue;
    intTest[1].connect_symbol = COMPARE_AND;

    D8U result = adm_cds_compare(intTest, 2);
    EXPECT_EQ(result,0x01);     
}

TEST_F(CCdsCross_uint_u64_NotEQ_And_Test, Normal)
{
    CCdsCompareTest aa;
    aa.cds_cross_uint_u64_NotEQ_And_Test();
}

void CCdsCompareTest::cds_cross_uint_u64_NotEQ_Or_Test()
{
    cds_compare_st intTest[2];
    intTest[0].type = COMPARE_SYMBOL_NOTEQ;
    cds_variable_st lValue;
    lValue.data_type = VAR_U32;
    lValue.v.value_u32=2;
    lValue.data_length = 1;
    cds_variable_st rValue;
    rValue.data_type = VAR_U64;
    
    rValue.v.value_u64=1;
    rValue.data_length = 1;
    intTest[0].l_variable = &lValue;
    intTest[0].r_variable = &rValue;
    intTest[0].connect_symbol = COMPARE_OR;

    intTest[1].type = COMPARE_SYMBOL_NOTEQ;
    cds_variable_st lValue1;
    lValue1.data_type = VAR_U32;
    lValue1.v.value_u32=2;
    lValue1.data_length = 1;
    cds_variable_st rValue1;
    rValue1.data_type = VAR_U64;
    
    rValue1.v.value_u64=1;
    rValue1.data_length = 1;
    intTest[1].l_variable = &lValue;
    intTest[1].r_variable = &rValue;
    intTest[1].connect_symbol = COMPARE_OR;

    D8U result = adm_cds_compare(intTest, 2);
    EXPECT_EQ(result,0x01);     
}

TEST_F(CCdsCross_uint_u64_NotEQ_Or_Test, Normal)
{
    CCdsCompareTest aa;
    aa.cds_cross_uint_u64_NotEQ_Or_Test();
}

void CCdsCompareTest::cds_cross_char_string_NotEQ_Test()
{
    cds_compare_st intTest;
    intTest.type = COMPARE_SYMBOL_NOTEQ;
    cds_variable_st lValue;
    lValue.data_type = VAR_U8;
    lValue.v.value[0]='2';
    lValue.data_length = 1;
    cds_variable_st rValue;
    rValue.data_type = VAR_STRING;
    DCHAR* pp = const_cast<DCHAR*>("1");
    
    rValue.v.value_s=reinterpret_cast<D8U*>(pp);
    rValue.data_length = 1;
    intTest.l_variable = &lValue;
    intTest.r_variable = &rValue;

    D8U result = adm_cds_compare(&intTest, 1);
    EXPECT_EQ(result,0x01);
}

TEST_F(CCdsCross_char_string_NotEQ_Test, Normal)
{
    CCdsCompareTest aa;
    aa.cds_cross_char_string_NotEQ_Test();
}

void CCdsCompareTest::cds_cross_char_string_NotEQ_And_Test()
{
    cds_compare_st intTest[2];
    intTest[0].type = COMPARE_SYMBOL_NOTEQ;
    cds_variable_st lValue;
    lValue.data_type = VAR_U8;
    lValue.v.value[0]='2';
    lValue.data_length = 1;
    cds_variable_st rValue;
    rValue.data_type = VAR_STRING;
    DCHAR* pp = const_cast<DCHAR*>("1");
    
    rValue.v.value_s=reinterpret_cast<D8U*>(pp);
    rValue.data_length = 1;
    intTest[0].l_variable = &lValue;
    intTest[0].r_variable = &rValue;
    intTest[0].connect_symbol = COMPARE_AND;

    intTest[1].type = COMPARE_SYMBOL_NOTEQ;
    cds_variable_st lValue1;
    lValue1.data_type = VAR_U8;
    lValue1.v.value[0]='2';
    lValue1.data_length = 1;
    cds_variable_st rValue1;
    rValue1.data_type = VAR_STRING;
    DCHAR* pp1 = const_cast<DCHAR*>("1");
    
    rValue1.v.value_s=reinterpret_cast<D8U*>(pp1);
    rValue1.data_length = 1;
    intTest[1].l_variable = &lValue1;
    intTest[1].r_variable = &rValue1;
    intTest[1].connect_symbol = COMPARE_AND;

    D8U result = adm_cds_compare(intTest, 2);
    EXPECT_EQ(result,0x01);
}

TEST_F(CCdsCross_char_string_NotEQ_And_Test, Normal)
{
    CCdsCompareTest aa;
    aa.cds_cross_char_string_NotEQ_And_Test();
}

void CCdsCompareTest::cds_cross_char_string_NotEQ_Or_Test()
{
    cds_compare_st intTest[2];
    intTest[0].type = COMPARE_SYMBOL_NOTEQ;
    cds_variable_st lValue;
    lValue.data_type = VAR_U8;
    lValue.v.value[0]='2';
    lValue.data_length = 1;
    cds_variable_st rValue;
    rValue.data_type = VAR_STRING;
    DCHAR* pp = const_cast<DCHAR*>("1");
    
    rValue.v.value_s=reinterpret_cast<D8U*>(pp);
    rValue.data_length = 1;
    intTest[0].l_variable = &lValue;
    intTest[0].r_variable = &rValue;
    intTest[0].connect_symbol = COMPARE_OR;

    intTest[1].type = COMPARE_SYMBOL_NOTEQ;
    cds_variable_st lValue1;
    lValue1.data_type = VAR_U8;
    lValue1.v.value[0]='2';
    lValue1.data_length = 1;
    cds_variable_st rValue1;
    rValue1.data_type = VAR_STRING;
    DCHAR* pp1 = const_cast<DCHAR*>("1");
    
    rValue1.v.value_s=reinterpret_cast<D8U*>(pp1);
    rValue1.data_length = 1;
    intTest[1].l_variable = &lValue1;
    intTest[1].r_variable = &rValue1;
    intTest[1].connect_symbol = COMPARE_OR;

    D8U result = adm_cds_compare(intTest, 2);
    EXPECT_EQ(result,0x01);    
}

TEST_F(CCdsCross_char_string_NotEQ_Or_Test, Normal)
{
    CCdsCompareTest aa;
    aa.cds_cross_char_string_NotEQ_Or_Test();
}

void CCdsCompareTest::cds_cross_int_uint_Less_Test()
{
    cds_compare_st intTest;
    intTest.type = COMPARE_SYMBOL_LESS;
    cds_variable_st lValue;
    lValue.data_type = VAR_S32;
    lValue.v.value_s32=1;
    lValue.data_length = 1;
    cds_variable_st rValue;
    rValue.data_type = VAR_U32;
    
    rValue.v.value_u32=2;
    rValue.data_length = 1;
    intTest.l_variable = &lValue;
    intTest.r_variable = &rValue;

    D8U result = adm_cds_compare(&intTest, 1);
    EXPECT_EQ(result,0x01);    
}

TEST_F(CCdsCross_int_uint_Less_Test, Normal)
{
    CCdsCompareTest aa;
    aa.cds_cross_int_uint_Less_Test();
}

void CCdsCompareTest::cds_cross_int_uint_Less_And_Test()
{
    cds_compare_st intTest[2];
    intTest[0].type = COMPARE_SYMBOL_LESS;
    cds_variable_st lValue;
    lValue.data_type = VAR_S32;
    lValue.v.value_s32=1;
    lValue.data_length = 1;
    cds_variable_st rValue;
    rValue.data_type = VAR_U32;
    
    rValue.v.value_u32=2;
    rValue.data_length = 1;
    intTest[0].l_variable = &lValue;
    intTest[0].r_variable = &rValue;
    intTest[0].connect_symbol = COMPARE_AND;

    intTest[1].type = COMPARE_SYMBOL_LESS;
    cds_variable_st lValue1;
    lValue1.data_type = VAR_S32;
    lValue1.v.value_s32=1;
    lValue1.data_length = 1;
    cds_variable_st rValue1;
    rValue1.data_type = VAR_U32;
    
    rValue1.v.value_u32=2;
    rValue1.data_length = 1;
    intTest[1].l_variable = &lValue1;
    intTest[1].r_variable = &rValue1;
    intTest[1].connect_symbol = COMPARE_AND;

    D8U result = adm_cds_compare(intTest, 2);
    EXPECT_EQ(result,0x01);    
}

TEST_F(CCdsCross_int_uint_Less_And_Test, Normal)
{
    CCdsCompareTest aa;
    aa.cds_cross_int_uint_Less_And_Test();
}

void CCdsCompareTest::cds_cross_int_uint_Less_Or_Test()
{
    cds_compare_st intTest[2];
    intTest[0].type = COMPARE_SYMBOL_LESS;
    cds_variable_st lValue;
    lValue.data_type = VAR_S32;
    lValue.v.value_s32=1;
    lValue.data_length = 1;
    cds_variable_st rValue;
    rValue.data_type = VAR_U32;
    
    rValue.v.value_u32=2;
    rValue.data_length = 1;
    intTest[0].l_variable = &lValue;
    intTest[0].r_variable = &rValue;
    intTest[0].connect_symbol = COMPARE_OR;

    intTest[1].type = COMPARE_SYMBOL_LESS;
    cds_variable_st lValue1;
    lValue1.data_type = VAR_S32;
    lValue1.v.value_s32=1;
    lValue1.data_length = 1;
    cds_variable_st rValue1;
    rValue1.data_type = VAR_U32;
    
    rValue1.v.value_u32=2;
    rValue1.data_length = 1;
    intTest[1].l_variable = &lValue1;
    intTest[1].r_variable = &rValue1;
    intTest[0].connect_symbol = COMPARE_OR;

    D8U result = adm_cds_compare(intTest, 2);
    EXPECT_EQ(result,0x01); 
}

TEST_F(CCdsCross_int_uint_Less_Or_Test, Normal)
{
    CCdsCompareTest aa;
    aa.cds_cross_int_uint_Less_Or_Test();
}

void CCdsCompareTest::cds_cross_int_u64_Less_Test()
{
    cds_compare_st intTest;
    intTest.type = COMPARE_SYMBOL_LESS;
    cds_variable_st lValue;
    lValue.data_type = VAR_S32;
    lValue.v.value_s32=1;
    lValue.data_length = 1;
    cds_variable_st rValue;
    rValue.data_type = VAR_U64;
    
    rValue.v.value_u64=2;
    rValue.data_length = 1;
    intTest.l_variable = &lValue;
    intTest.r_variable = &rValue;

    D8U result = adm_cds_compare(&intTest, 1);
    EXPECT_EQ(result,0x01);     
}

TEST_F(CCdsCross_int_u64_Less_Test, Normal)
{
    CCdsCompareTest aa;
    aa.cds_cross_int_u64_Less_Test();
}

void CCdsCompareTest::cds_cross_int_u64_Less_And_Test()
{
    cds_compare_st intTest[2];
    intTest[0].type = COMPARE_SYMBOL_LESS;
    cds_variable_st lValue;
    lValue.data_type = VAR_S32;
    lValue.v.value_s32=1;
    lValue.data_length = 1;
    cds_variable_st rValue;
    rValue.data_type = VAR_U64;
    
    rValue.v.value_u64=2;
    rValue.data_length = 1;
    intTest[0].l_variable = &lValue;
    intTest[0].r_variable = &rValue;
    intTest[0].connect_symbol = COMPARE_AND;

    intTest[1].type = COMPARE_SYMBOL_LESS;
    cds_variable_st lValue1;
    lValue1.data_type = VAR_S32;
    lValue1.v.value_s32=1;
    lValue1.data_length = 1;
    cds_variable_st rValue1;
    rValue1.data_type = VAR_U64;
    
    rValue1.v.value_u64=2;
    rValue1.data_length = 1;
    intTest[1].l_variable = &lValue1;
    intTest[1].r_variable = &rValue1;
    intTest[1].connect_symbol = COMPARE_AND;

    D8U result = adm_cds_compare(intTest, 2);
    EXPECT_EQ(result,0x01);     
}

TEST_F(CCdsCross_int_u64_Less_And_Test, Normal)
{
    CCdsCompareTest aa;
    aa.cds_cross_int_u64_Less_And_Test();
}

void CCdsCompareTest::cds_cross_int_u64_Less_Or_Test()
{
    cds_compare_st intTest[2];
    intTest[0].type = COMPARE_SYMBOL_LESS;
    cds_variable_st lValue;
    lValue.data_type = VAR_S32;
    lValue.v.value_s32=1;
    lValue.data_length = 1;
    cds_variable_st rValue;
    rValue.data_type = VAR_U64;

    rValue.v.value_u64=2;
    rValue.data_length = 1;
    intTest[0].l_variable = &lValue;
    intTest[0].r_variable = &rValue;
    intTest[0].connect_symbol = COMPARE_OR;

    intTest[1].type = COMPARE_SYMBOL_LESS;
    cds_variable_st lValue1;
    lValue1.data_type = VAR_S32;
    lValue1.v.value_s32=1;
    lValue1.data_length = 1;
    cds_variable_st rValue1;
    rValue1.data_type = VAR_U64;
    
    rValue1.v.value_u64=1;
    rValue1.data_length = 1;
    intTest[1].l_variable = &lValue1;
    intTest[1].r_variable = &rValue1;
    intTest[1].connect_symbol = COMPARE_OR;

    D8U result = adm_cds_compare(intTest, 2);
    EXPECT_EQ(result,0x01);    
}

TEST_F(CCdsCross_int_u64_Less_Or_Test, Normal)
{
    CCdsCompareTest aa;
    aa.cds_cross_int_u64_Less_Or_Test();
}

void CCdsCompareTest::cds_cross_uint_u64_Less_Test()
{
    cds_compare_st intTest;
    intTest.type = COMPARE_SYMBOL_LESS;
    cds_variable_st lValue;
    lValue.data_type = VAR_U32;
    lValue.v.value_u32=1;
    lValue.data_length = 1;
    cds_variable_st rValue;
    rValue.data_type = VAR_U64;
    
    rValue.v.value_u64=2;
    rValue.data_length = 1;
    intTest.l_variable = &lValue;
    intTest.r_variable = &rValue;

    D8U result = adm_cds_compare(&intTest, 1);
    EXPECT_EQ(result,0x01);    
}

TEST_F(CCdsCross_uint_u64_Less_Test, Normal)
{
    CCdsCompareTest aa;
    aa.cds_cross_uint_u64_Less_Test();
}

void CCdsCompareTest::cds_cross_uint_u64_Less_And_Test()
{
    cds_compare_st intTest[2];
    intTest[0].type = COMPARE_SYMBOL_LESS;
    cds_variable_st lValue;
    lValue.data_type = VAR_U32;
    lValue.v.value_u32=1;
    lValue.data_length = 1;
    cds_variable_st rValue;
    rValue.data_type = VAR_U64;
    
    rValue.v.value_u64=2;
    rValue.data_length = 1;
    intTest[0].l_variable = &lValue;
    intTest[0].r_variable = &rValue;
    intTest[0].connect_symbol = COMPARE_AND;

    intTest[1].type = COMPARE_SYMBOL_LESS;
    cds_variable_st lValue1;
    lValue1.data_type = VAR_U32;
    lValue1.v.value_u32=1;
    lValue1.data_length = 1;
    cds_variable_st rValue1;
    rValue1.data_type = VAR_U64;
    
    rValue1.v.value_u64=2;
    rValue1.data_length = 1;
    intTest[1].l_variable = &lValue1;
    intTest[1].r_variable = &rValue1;
    intTest[1].connect_symbol = COMPARE_AND;

    D8U result = adm_cds_compare(intTest, 2);
    EXPECT_EQ(result,0x01);     
}

TEST_F(CCdsCross_uint_u64_Less_And_Test, Normal)
{
    CCdsCompareTest aa;
    aa.cds_cross_uint_u64_Less_And_Test();
}

void CCdsCompareTest::cds_cross_uint_u64_Less_Or_Test()
{
    cds_compare_st intTest[2];
    intTest[0].type = COMPARE_SYMBOL_LESS;
    cds_variable_st lValue;
    lValue.data_type = VAR_U32;
    lValue.v.value_u32=1;
    lValue.data_length = 1;
    cds_variable_st rValue;
    rValue.data_type = VAR_U64;
    
    rValue.v.value_u64=2;
    rValue.data_length = 1;
    intTest[0].l_variable = &lValue;
    intTest[0].r_variable = &rValue;
    intTest[0].connect_symbol = COMPARE_OR;

    intTest[1].type = COMPARE_SYMBOL_LESS;
    cds_variable_st lValue1;
    lValue1.data_type = VAR_U32;
    lValue1.v.value_u32=1;
    lValue1.data_length = 1;
    cds_variable_st rValue1;
    rValue1.data_type = VAR_U64;
    
    rValue1.v.value_u64=2;
    rValue1.data_length = 1;
    intTest[1].l_variable = &lValue1;
    intTest[1].r_variable = &rValue1;
    intTest[1].connect_symbol = COMPARE_OR;

    D8U result = adm_cds_compare(intTest, 2);
    EXPECT_EQ(result,0x01);    
}

TEST_F(CCdsCross_uint_u64_Less_Or_Test, Normal)
{
    CCdsCompareTest aa;
    aa.cds_cross_uint_u64_Less_Or_Test();
}

void CCdsCompareTest::cds_cross_int_float_Less_Test()
{
    cds_compare_st intTest;
    intTest.type = COMPARE_SYMBOL_LESS;
    cds_variable_st lValue;
    lValue.data_type = VAR_S32;
    lValue.v.value_s32=1;
    lValue.data_length = 1;
    cds_variable_st rValue;
    rValue.data_type = VAR_FLOAT;
    
    rValue.v.value_f=2.0;
    rValue.data_length = 1;
    intTest.l_variable = &lValue;
    intTest.r_variable = &rValue;

    D8U result = adm_cds_compare(&intTest, 1);
    EXPECT_EQ(result,0x01);     
}

TEST_F(CCdsCross_int_float_Less_Test, Normal)
{
    CCdsCompareTest aa;
    aa.cds_cross_int_float_Less_Test();
}

void CCdsCompareTest::cds_cross_int_float_Less_And_Test()
{
    cds_compare_st intTest[2];
    intTest[0].type = COMPARE_SYMBOL_LESS;
    cds_variable_st lValue;
    lValue.data_type = VAR_S32;
    lValue.v.value_s32=1;
    lValue.data_length = 1;
    cds_variable_st rValue;
    rValue.data_type = VAR_FLOAT;
    
    rValue.v.value_f=2.0;
    rValue.data_length = 1;
    intTest[0].l_variable = &lValue;
    intTest[0].r_variable = &rValue;
    intTest[0].connect_symbol = COMPARE_AND;

    intTest[1].type = COMPARE_SYMBOL_LESS;
    cds_variable_st lValue1;
    lValue1.data_type = VAR_S32;
    lValue1.v.value_s32=1;
    lValue1.data_length = 1;
    cds_variable_st rValue1;
    rValue1.data_type = VAR_FLOAT;
    
    rValue1.v.value_f=2.0;
    rValue1.data_length = 1;
    intTest[1].l_variable = &lValue1;
    intTest[1].r_variable = &rValue1;
    intTest[1].connect_symbol = COMPARE_AND;
    D8U result = adm_cds_compare(intTest, 2);
    EXPECT_EQ(result,0x01);    
}

TEST_F(CCdsCross_int_float_Less_And_Test, Normal)
{
    CCdsCompareTest aa;
    aa.cds_cross_int_float_Less_And_Test();
}

void CCdsCompareTest::cds_cross_int_float_Less_Or_Test()
{
    cds_compare_st intTest[2];
    intTest[0].type = COMPARE_SYMBOL_LESS;
    cds_variable_st lValue;
    lValue.data_type = VAR_S32;
    lValue.v.value_s32=1;
    lValue.data_length = 1;
    cds_variable_st rValue;
    rValue.data_type = VAR_FLOAT;
    
    rValue.v.value_f=2.0;
    rValue.data_length = 1;
    intTest[0].l_variable = &lValue;
    intTest[0].r_variable = &rValue;
    intTest[0].connect_symbol = COMPARE_OR;
    intTest[1].type = COMPARE_SYMBOL_LESS;
    cds_variable_st lValue1;
    lValue1.data_type = VAR_S32;
    lValue1.v.value_s32=1;
    lValue1.data_length = 1;
    cds_variable_st rValue1;
    rValue1.data_type = VAR_FLOAT;
    
    rValue1.v.value_f=2.0;
    rValue1.data_length = 1;
    intTest[1].l_variable = &lValue1;
    intTest[1].r_variable = &rValue1;
    intTest[1].connect_symbol = COMPARE_OR;
    D8U result = adm_cds_compare(intTest, 2);
    EXPECT_EQ(result,0x01);     
}

TEST_F(CCdsCross_int_float_Less_Or_Test, Normal)
{
    CCdsCompareTest aa;
    aa.cds_cross_int_float_Less_Or_Test();
}

void CCdsCompareTest::cds_cross_uint_float_Less_Test()
{
    cds_compare_st intTest;
    intTest.type = COMPARE_SYMBOL_LESS;
    cds_variable_st lValue;
    lValue.data_type = VAR_U32;
    lValue.v.value_u32=1;
    lValue.data_length = 1;
    cds_variable_st rValue;
    rValue.data_type = VAR_FLOAT;
    
    rValue.v.value_f=2.0;
    rValue.data_length = 1;
    intTest.l_variable = &lValue;
    intTest.r_variable = &rValue;

    D8U result = adm_cds_compare(&intTest, 1);
    EXPECT_EQ(result,0x01);     
}

TEST_F(CCdsCross_uint_float_Less_Test, Normal)
{
    CCdsCompareTest aa;
    aa.cds_cross_uint_float_Less_Test();
}

void CCdsCompareTest::cds_cross_uint_float_Less_And_Test()
{
    cds_compare_st intTest[2];
    intTest[0].type = COMPARE_SYMBOL_LESS;
    cds_variable_st lValue;
    lValue.data_type = VAR_U32;
    lValue.v.value_u32=1;
    lValue.data_length = 1;
    cds_variable_st rValue;
    rValue.data_type = VAR_FLOAT;
    
    rValue.v.value_f=2.0;
    rValue.data_length = 1;
    intTest[0].l_variable = &lValue;
    intTest[0].r_variable = &rValue;
    intTest[0].connect_symbol = COMPARE_AND;

    intTest[1].type = COMPARE_SYMBOL_LESS;
    cds_variable_st lValue1;
    lValue1.data_type = VAR_U32;
    lValue1.v.value_u32=1;
    lValue1.data_length = 1;
    cds_variable_st rValue1;
    rValue1.data_type = VAR_FLOAT;
    
    rValue1.v.value_f=2.0;
    rValue1.data_length = 1;
    intTest[1].l_variable = &lValue1;
    intTest[1].r_variable = &rValue1;
    intTest[1].connect_symbol = COMPARE_AND;
    D8U result = adm_cds_compare(intTest, 2);
    EXPECT_EQ(result,0x01);    
}

TEST_F(CCdsCross_uint_float_Less_And_Test, Normal)
{
    CCdsCompareTest aa;
    aa.cds_cross_uint_float_Less_And_Test();
}

void CCdsCompareTest::cds_cross_uint_float_Less_Or_Test()
{
    cds_compare_st intTest[2];
    intTest[0].type = COMPARE_SYMBOL_LESS;
    cds_variable_st lValue;
    lValue.data_type = VAR_U32;
    lValue.v.value_u32=1;
    lValue.data_length = 1;
    cds_variable_st rValue;
    rValue.data_type = VAR_FLOAT;
    
    rValue.v.value_f=2.0;
    rValue.data_length = 1;
    intTest[0].l_variable = &lValue;
    intTest[0].r_variable = &rValue;
    intTest[0].connect_symbol = COMPARE_OR;
    intTest[1].type = COMPARE_SYMBOL_LESS;
    cds_variable_st lValue1;
    lValue1.data_type = VAR_U32;
    lValue1.v.value_u32=1;
    lValue1.data_length = 1;
    cds_variable_st rValue1;
    rValue1.data_type = VAR_FLOAT;
    
    rValue1.v.value_f=2.0;
    rValue1.data_length = 1;
    intTest[1].l_variable = &lValue1;
    intTest[1].r_variable = &rValue1;
    intTest[1].connect_symbol = COMPARE_OR;
    D8U result = adm_cds_compare(intTest, 2);
    EXPECT_EQ(result,0x01);    
}

TEST_F(CCdsCross_uint_float_Less_Or_Test, Normal)
{
    CCdsCompareTest aa;
    aa.cds_cross_uint_float_Less_Or_Test();
}

void CCdsCompareTest::cds_cross_u64_float_Less_Test()
{
    cds_compare_st intTest;
    intTest.type = COMPARE_SYMBOL_LESS;
    cds_variable_st lValue;
    lValue.data_type = VAR_U64;
    lValue.v.value_u64=1;
    lValue.data_length = 1;
    cds_variable_st rValue;
    rValue.data_type = VAR_FLOAT;
    
    rValue.v.value_f=2.0;
    rValue.data_length = 1;
    intTest.l_variable = &lValue;
    intTest.r_variable = &rValue;

    D8U result = adm_cds_compare(&intTest, 1);
    EXPECT_EQ(result,0x01);     
}

TEST_F(CCdsCross_u64_float_Less_Test, Normal)
{
    CCdsCompareTest aa;
    aa.cds_cross_u64_float_Less_Test();
}

void CCdsCompareTest::cds_cross_u64_float_Less_And_Test()
{
    cds_compare_st intTest[2];
    intTest[0].type = COMPARE_SYMBOL_LESS;
    cds_variable_st lValue;
    lValue.data_type = VAR_U64;
    lValue.v.value_u64=1;
    lValue.data_length = 1;
    cds_variable_st rValue;
    rValue.data_type = VAR_FLOAT;
    
    rValue.v.value_f=2.0;
    rValue.data_length = 1;
    intTest[0].l_variable = &lValue;
    intTest[0].r_variable = &rValue;
    intTest[0].connect_symbol = COMPARE_AND;

    intTest[1].type = COMPARE_SYMBOL_LESS;
    cds_variable_st lValue1;
    lValue1.data_type = VAR_U64;
    lValue1.v.value_u64=1;
    lValue1.data_length = 1;
    cds_variable_st rValue1;
    rValue1.data_type = VAR_FLOAT;
    
    rValue1.v.value_f=2.0;
    rValue1.data_length = 1;
    intTest[1].l_variable = &lValue1;
    intTest[1].r_variable = &rValue1;
    intTest[1].connect_symbol = COMPARE_AND;
    D8U result = adm_cds_compare(intTest, 2);
    EXPECT_EQ(result,0x01);     
}

TEST_F(CCdsCross_u64_float_Less_And_Test, Normal)
{
    CCdsCompareTest aa;
    aa.cds_cross_u64_float_Less_And_Test();
}

void CCdsCompareTest::cds_cross_u64_float_Less_Or_Test()
{
    cds_compare_st intTest[2];
    intTest[0].type = COMPARE_SYMBOL_LESS;
    cds_variable_st lValue;
    lValue.data_type = VAR_U64;
    lValue.v.value_u64=1;
    lValue.data_length = 1;
    cds_variable_st rValue;
    rValue.data_type = VAR_FLOAT;
    
    rValue.v.value_f=2.0;
    rValue.data_length = 1;
    intTest[0].l_variable = &lValue;
    intTest[0].r_variable = &rValue;
    intTest[0].connect_symbol = COMPARE_OR;
    intTest[1].type = COMPARE_SYMBOL_LESS;
    cds_variable_st lValue1;
    lValue1.data_type = VAR_U64;
    lValue1.v.value_u64=1;
    lValue1.data_length = 1;
    cds_variable_st rValue1;
    rValue1.data_type = VAR_FLOAT;
    
    rValue1.v.value_f=2.0;
    rValue1.data_length = 1;
    intTest[1].l_variable = &lValue1;
    intTest[1].r_variable = &rValue1;
    intTest[1].connect_symbol = COMPARE_OR;
    D8U result = adm_cds_compare(intTest, 2);
    EXPECT_EQ(result,0x01);     
}

TEST_F(CCdsCross_u64_float_Less_Or_Test, Normal)
{
    CCdsCompareTest aa;
    aa.cds_cross_u64_float_Less_Or_Test();
}

void CCdsCompareTest::cds_cross_int_uint_LessEQ_Test()
{
    cds_compare_st intTest;
    intTest.type = COMPARE_SYMBOL_LESSEQ;
    cds_variable_st lValue;
    lValue.data_type = VAR_S32;
    lValue.v.value_s32=2;
    lValue.data_length = 1;
    cds_variable_st rValue;
    rValue.data_type = VAR_U32;
    
    rValue.v.value_u32=2;
    rValue.data_length = 1;
    intTest.l_variable = &lValue;
    intTest.r_variable = &rValue;

    D8U result = adm_cds_compare(&intTest, 1);
    EXPECT_EQ(result,0x01);    
}

TEST_F(CCdsCross_int_uint_LessEQ_Test, Normal)
{
    CCdsCompareTest aa;
    aa.cds_cross_int_uint_LessEQ_Test();
}

void CCdsCompareTest::cds_cross_int_uint_LessEQ_And_Test()
{
    cds_compare_st intTest[2];
    intTest[0].type = COMPARE_SYMBOL_LESSEQ;
    cds_variable_st lValue;
    lValue.data_type = VAR_S32;
    lValue.v.value_s32=1;
    lValue.data_length = 1;
    cds_variable_st rValue;
    rValue.data_type = VAR_U32;
    
    rValue.v.value_u32=2;
    rValue.data_length = 1;
    intTest[0].l_variable = &lValue;
    intTest[0].r_variable = &rValue;
    intTest[0].connect_symbol = COMPARE_AND;

    intTest[1].type = COMPARE_SYMBOL_LESSEQ;
    cds_variable_st lValue1;
    lValue1.data_type = VAR_S32;
    lValue1.v.value_s32=1;
    lValue1.data_length = 1;
    cds_variable_st rValue1;
    rValue1.data_type = VAR_U32;
    
    rValue1.v.value_u32=2;
    rValue1.data_length = 1;
    intTest[1].l_variable = &lValue1;
    intTest[1].r_variable = &rValue1;
    intTest[1].connect_symbol = COMPARE_AND;

    D8U result = adm_cds_compare(intTest, 2);
    EXPECT_EQ(result,0x01);      
}

TEST_F(CCdsCross_int_uint_LessEQ_And_Test, Normal)
{
    CCdsCompareTest aa;
    aa.cds_cross_int_uint_LessEQ_And_Test();
}

void CCdsCompareTest::cds_cross_int_uint_LessEQ_Or_Test()
{
    cds_compare_st intTest[2];
    intTest[0].type = COMPARE_SYMBOL_LESSEQ;
    cds_variable_st lValue;
    lValue.data_type = VAR_S32;
    lValue.v.value_s32=1;
    lValue.data_length = 1;
    cds_variable_st rValue;
    rValue.data_type = VAR_U32;
    
    rValue.v.value_u32=2;
    rValue.data_length = 1;
    intTest[0].l_variable = &lValue;
    intTest[0].r_variable = &rValue;
    intTest[0].connect_symbol = COMPARE_OR;

    intTest[1].type = COMPARE_SYMBOL_LESSEQ;
    cds_variable_st lValue1;
    lValue1.data_type = VAR_S32;
    lValue1.v.value_s32=1;
    lValue1.data_length = 1;
    cds_variable_st rValue1;
    rValue1.data_type = VAR_U32;
    
    rValue1.v.value_u32=2;
    rValue1.data_length = 1;
    intTest[1].l_variable = &lValue1;
    intTest[1].r_variable = &rValue1;
    intTest[0].connect_symbol = COMPARE_OR;

    D8U result = adm_cds_compare(intTest, 2);
    EXPECT_EQ(result,0x01);     
}

TEST_F(CCdsCross_int_uint_LessEQ_Or_Test, Normal)
{
    CCdsCompareTest aa;
    aa.cds_cross_int_uint_LessEQ_Or_Test();
}

void CCdsCompareTest::cds_cross_int_u64_LessEQ_Test()
{
    cds_compare_st intTest;
    intTest.type = COMPARE_SYMBOL_LESSEQ;
    cds_variable_st lValue;
    lValue.data_type = VAR_S32;
    lValue.v.value_s32=1;
    lValue.data_length = 1;
    cds_variable_st rValue;
    rValue.data_type = VAR_U64;
    
    rValue.v.value_u64=2;
    rValue.data_length = 1;
    intTest.l_variable = &lValue;
    intTest.r_variable = &rValue;

    D8U result = adm_cds_compare(&intTest, 1);
    EXPECT_EQ(result,0x01);     
}

TEST_F(CCdsCross_int_u64_LessEQ_Test, Normal)
{
    CCdsCompareTest aa;
    aa.cds_cross_int_u64_LessEQ_Test();
}

void CCdsCompareTest::cds_cross_int_u64_LessEQ_And_Test()
{
    cds_compare_st intTest[2];
    intTest[0].type = COMPARE_SYMBOL_LESSEQ;
    cds_variable_st lValue;
    lValue.data_type = VAR_S32;
    lValue.v.value_s32=1;
    lValue.data_length = 1;
    cds_variable_st rValue;
    rValue.data_type = VAR_U64;
    
    rValue.v.value_u64=1;
    rValue.data_length = 1;
    intTest[0].l_variable = &lValue;
    intTest[0].r_variable = &rValue;
    intTest[0].connect_symbol = COMPARE_AND;

    intTest[1].type = COMPARE_SYMBOL_LESSEQ;
    cds_variable_st lValue1;
    lValue1.data_type = VAR_S32;
    lValue1.v.value_s32=0;
    lValue1.data_length = 1;
    cds_variable_st rValue1;
    rValue1.data_type = VAR_U64;
    
    rValue1.v.value_u64=1;
    rValue1.data_length = 1;
    intTest[1].l_variable = &lValue1;
    intTest[1].r_variable = &rValue1;
    intTest[1].connect_symbol = COMPARE_AND;

    D8U result = adm_cds_compare(intTest, 2);
    EXPECT_EQ(result,0x01);    
}

TEST_F(CCdsCross_int_u64_LessEQ_And_Test, Normal)
{
    CCdsCompareTest aa;
    aa.cds_cross_int_u64_LessEQ_And_Test();
}

void CCdsCompareTest::cds_cross_int_u64_LessEQ_Or_Test()
{
    cds_compare_st intTest[2];
    intTest[0].type = COMPARE_SYMBOL_LESSEQ;
    cds_variable_st lValue;
    lValue.data_type = VAR_S32;
    lValue.v.value_s32=1;
    lValue.data_length = 1;
    cds_variable_st rValue;
    rValue.data_type = VAR_U64;

    rValue.v.value_u64=1;
    rValue.data_length = 1;
    intTest[0].l_variable = &lValue;
    intTest[0].r_variable = &rValue;
    intTest[0].connect_symbol = COMPARE_OR;

    intTest[1].type = COMPARE_SYMBOL_LESSEQ;
    cds_variable_st lValue1;
    lValue1.data_type = VAR_S32;
    lValue1.v.value_s32=2;
    lValue1.data_length = 1;
    cds_variable_st rValue1;
    rValue1.data_type = VAR_U64;

    rValue1.v.value_u64=1;
    rValue1.data_length = 1;
    intTest[1].l_variable = &lValue1;
    intTest[1].r_variable = &rValue1;
    intTest[1].connect_symbol = COMPARE_OR;

    D8U result = adm_cds_compare(intTest, 2);
    EXPECT_EQ(result,0x01);     
}

TEST_F(CCdsCross_int_u64_LessEQ_Or_Test, Normal)
{
    CCdsCompareTest aa;
    aa.cds_cross_int_u64_LessEQ_Or_Test();
}

void CCdsCompareTest::cds_cross_uint_u64_LessEQ_Test()
{
    cds_compare_st intTest;
    intTest.type = COMPARE_SYMBOL_LESSEQ;
    cds_variable_st lValue;
    lValue.data_type = VAR_U32;
    lValue.v.value_u32=1;
    lValue.data_length = 1;
    cds_variable_st rValue;
    rValue.data_type = VAR_U64;
    
    rValue.v.value_u64=2;
    rValue.data_length = 1;
    intTest.l_variable = &lValue;
    intTest.r_variable = &rValue;

    D8U result = adm_cds_compare(&intTest, 1);
    EXPECT_EQ(result,0x01);
}

TEST_F(CCdsCross_uint_u64_LessEQ_Test, Normal)
{
    CCdsCompareTest aa;
    aa.cds_cross_uint_u64_LessEQ_Test();
}

void CCdsCompareTest::cds_cross_uint_u64_LessEQ_And_Test()
{
    cds_compare_st intTest[2];
    intTest[0].type = COMPARE_SYMBOL_LESSEQ;
    cds_variable_st lValue;
    lValue.data_type = VAR_U32;
    lValue.v.value_u32=1;
    lValue.data_length = 1;
    cds_variable_st rValue;
    rValue.data_type = VAR_U64;
    
    rValue.v.value_u64=2;
    rValue.data_length = 1;
    intTest[0].l_variable = &lValue;
    intTest[0].r_variable = &rValue;
    intTest[0].connect_symbol = COMPARE_AND;

    intTest[1].type = COMPARE_SYMBOL_LESSEQ;
    cds_variable_st lValue1;
    lValue1.data_type = VAR_U32;
    lValue1.v.value_u32=1;
    lValue1.data_length = 1;
    cds_variable_st rValue1;
    rValue1.data_type = VAR_U64;
    
    rValue1.v.value_u64=2;
    rValue1.data_length = 1;
    intTest[1].l_variable = &lValue1;
    intTest[1].r_variable = &rValue1;
    intTest[1].connect_symbol = COMPARE_AND;

    D8U result = adm_cds_compare(intTest, 2);
    EXPECT_EQ(result,0x01);     
}

TEST_F(CCdsCross_uint_u64_LessEQ_And_Test, Normal)
{
    CCdsCompareTest aa;
    aa.cds_cross_uint_u64_LessEQ_And_Test();
}

void CCdsCompareTest::cds_cross_uint_u64_LessEQ_Or_Test()
{
    cds_compare_st intTest[2];
    intTest[0].type = COMPARE_SYMBOL_LESSEQ;
    cds_variable_st lValue;
    lValue.data_type = VAR_U32;
    lValue.v.value_u32=1;
    lValue.data_length = 1;
    cds_variable_st rValue;
    rValue.data_type = VAR_U64;
    
    rValue.v.value_u64=2;
    rValue.data_length = 1;
    intTest[0].l_variable = &lValue;
    intTest[0].r_variable = &rValue;
    intTest[0].connect_symbol = COMPARE_OR;

    intTest[1].type = COMPARE_SYMBOL_LESSEQ;
    cds_variable_st lValue1;
    lValue1.data_type = VAR_U32;
    lValue1.v.value_u32=1;
    lValue1.data_length = 1;
    cds_variable_st rValue1;
    rValue1.data_type = VAR_U64;
    
    rValue1.v.value_u64=2;
    rValue1.data_length = 1;
    intTest[1].l_variable = &lValue1;
    intTest[1].r_variable = &rValue1;
    intTest[1].connect_symbol = COMPARE_OR;

    D8U result = adm_cds_compare(intTest, 2);
    EXPECT_EQ(result,0x01);    
}

TEST_F(CCdsCross_uint_u64_LessEQ_Or_Test, Normal)
{
    CCdsCompareTest aa;
    aa.cds_cross_uint_u64_LessEQ_Or_Test();
}

void CCdsCompareTest::cds_cross_int_float_LessEQ_Test()
{
    cds_compare_st intTest;
    intTest.type = COMPARE_SYMBOL_LESSEQ;
    cds_variable_st lValue;
    lValue.data_type = VAR_S32;
    lValue.v.value_s32=1;
    lValue.data_length = 1;
    cds_variable_st rValue;
    rValue.data_type = VAR_FLOAT;
    
    rValue.v.value_f=2.0;
    rValue.data_length = 1;
    intTest.l_variable = &lValue;
    intTest.r_variable = &rValue;

    D8U result = adm_cds_compare(&intTest, 1);
    EXPECT_EQ(result,0x01);     
}

TEST_F(CCdsCross_int_float_LessEQ_Test, Normal)
{
    CCdsCompareTest aa;
    aa.cds_cross_int_float_LessEQ_Test();
}

void CCdsCompareTest::cds_cross_int_float_LessEQ_And_Test()
{
    cds_compare_st intTest[2];
    intTest[0].type = COMPARE_SYMBOL_LESSEQ;
    cds_variable_st lValue;
    lValue.data_type = VAR_S32;
    lValue.v.value_s32=1;
    lValue.data_length = 1;
    cds_variable_st rValue;
    rValue.data_type = VAR_FLOAT;
    
    rValue.v.value_f=2.0;
    rValue.data_length = 1;
    intTest[0].l_variable = &lValue;
    intTest[0].r_variable = &rValue;
    intTest[0].connect_symbol = COMPARE_AND;

    intTest[1].type = COMPARE_SYMBOL_LESSEQ;
    cds_variable_st lValue1;
    lValue1.data_type = VAR_S32;
    lValue1.v.value_s32=1;
    lValue1.data_length = 1;
    cds_variable_st rValue1;
    rValue1.data_type = VAR_FLOAT;
    
    rValue1.v.value_f=2.0;
    rValue1.data_length = 1;
    intTest[1].l_variable = &lValue1;
    intTest[1].r_variable = &rValue1;
    intTest[1].connect_symbol = COMPARE_AND;
    D8U result = adm_cds_compare(intTest, 2);
    EXPECT_EQ(result,0x01);     
}

TEST_F(CCdsCross_int_float_LessEQ_And_Test, Normal)
{
    CCdsCompareTest aa;
    aa.cds_cross_int_float_LessEQ_And_Test();
}

void CCdsCompareTest::cds_cross_int_float_LessEQ_Or_Test()
{
    cds_compare_st intTest[2];
    intTest[0].type = COMPARE_SYMBOL_LESSEQ;
    cds_variable_st lValue;
    lValue.data_type = VAR_S32;
    lValue.v.value_s32=1;
    lValue.data_length = 1;
    cds_variable_st rValue;
    rValue.data_type = VAR_FLOAT;
    
    rValue.v.value_f=2.0;
    rValue.data_length = 1;
    intTest[0].l_variable = &lValue;
    intTest[0].r_variable = &rValue;
    intTest[0].connect_symbol = COMPARE_OR;
    intTest[1].type = COMPARE_SYMBOL_LESSEQ;
    cds_variable_st lValue1;
    lValue1.data_type = VAR_S32;
    lValue1.v.value_s32=1;
    lValue1.data_length = 1;
    cds_variable_st rValue1;
    rValue1.data_type = VAR_FLOAT;
    
    rValue1.v.value_f=2.0;
    rValue1.data_length = 1;
    intTest[1].l_variable = &lValue1;
    intTest[1].r_variable = &rValue1;
    intTest[1].connect_symbol = COMPARE_OR;
    D8U result = adm_cds_compare(intTest, 2);
    EXPECT_EQ(result,0x01);    
}

TEST_F(CCdsCross_int_float_LessEQ_Or_Test, Normal)
{
    CCdsCompareTest aa;
    aa.cds_cross_int_float_LessEQ_Or_Test();
}

void CCdsCompareTest::cds_cross_uint_float_LessEQ_Test()
{
    cds_compare_st intTest;
    intTest.type = COMPARE_SYMBOL_LESSEQ;
    cds_variable_st lValue;
    lValue.data_type = VAR_U32;
    lValue.v.value_u32=1;
    lValue.data_length = 1;
    cds_variable_st rValue;
    rValue.data_type = VAR_FLOAT;
    
    rValue.v.value_f=2.0;
    rValue.data_length = 1;
    intTest.l_variable = &lValue;
    intTest.r_variable = &rValue;

    D8U result = adm_cds_compare(&intTest, 1);
    EXPECT_EQ(result,0x01);      
}

TEST_F(CCdsCross_uint_float_LessEQ_Test, Normal)
{
    CCdsCompareTest aa;
    aa.cds_cross_uint_float_LessEQ_Test();
}

void CCdsCompareTest::cds_cross_uint_float_LessEQ_And_Test()
{
    cds_compare_st intTest[2];
    intTest[0].type = COMPARE_SYMBOL_LESSEQ;
    cds_variable_st lValue;
    lValue.data_type = VAR_U32;
    lValue.v.value_u32=1;
    lValue.data_length = 1;
    cds_variable_st rValue;
    rValue.data_type = VAR_FLOAT;
    
    rValue.v.value_f=2.0;
    rValue.data_length = 1;
    intTest[0].l_variable = &lValue;
    intTest[0].r_variable = &rValue;
    intTest[0].connect_symbol = COMPARE_AND;

    intTest[1].type = COMPARE_SYMBOL_LESSEQ;
    cds_variable_st lValue1;
    lValue1.data_type = VAR_U32;
    lValue1.v.value_u32=1;
    lValue1.data_length = 1;
    cds_variable_st rValue1;
    rValue1.data_type = VAR_FLOAT;
    
    rValue1.v.value_f=2.0;
    rValue1.data_length = 1;
    intTest[1].l_variable = &lValue1;
    intTest[1].r_variable = &rValue1;
    intTest[1].connect_symbol = COMPARE_AND;
    D8U result = adm_cds_compare(intTest, 2);
    EXPECT_EQ(result,0x01);     
}

TEST_F(CCdsCross_uint_float_LessEQ_And_Test, Normal)
{
    CCdsCompareTest aa;
    aa.cds_cross_uint_float_LessEQ_And_Test();
}

void CCdsCompareTest::cds_cross_uint_float_LessEQ_Or_Test()
{
    cds_compare_st intTest[2];
    intTest[0].type = COMPARE_SYMBOL_LESSEQ;
    cds_variable_st lValue;
    lValue.data_type = VAR_U32;
    lValue.v.value_u32=1;
    lValue.data_length = 1;
    cds_variable_st rValue;
    rValue.data_type = VAR_FLOAT;
    
    rValue.v.value_f=2.0;
    rValue.data_length = 1;
    intTest[0].l_variable = &lValue;
    intTest[0].r_variable = &rValue;
    intTest[0].connect_symbol = COMPARE_OR;
    intTest[1].type = COMPARE_SYMBOL_LESSEQ;
    cds_variable_st lValue1;
    lValue1.data_type = VAR_U32;
    lValue1.v.value_u32=1;
    lValue1.data_length = 1;
    cds_variable_st rValue1;
    rValue1.data_type = VAR_FLOAT;
    
    rValue1.v.value_f=2.0;
    rValue1.data_length = 1;
    intTest[1].l_variable = &lValue1;
    intTest[1].r_variable = &rValue1;
    intTest[1].connect_symbol = COMPARE_OR;
    D8U result = adm_cds_compare(intTest, 2);
    EXPECT_EQ(result,0x01);    
}

TEST_F(CCdsCross_uint_float_LessEQ_Or_Test, Normal)
{
    CCdsCompareTest aa;
    aa.cds_cross_uint_float_LessEQ_Or_Test();
}

void CCdsCompareTest::cds_cross_u64_float_LessEQ_Test()
{
    cds_compare_st intTest;
    intTest.type = COMPARE_SYMBOL_LESSEQ;
    cds_variable_st lValue;
    lValue.data_type = VAR_U64;
    lValue.v.value_u64=1;
    lValue.data_length = 1;
    cds_variable_st rValue;
    rValue.data_type = VAR_FLOAT;
    
    rValue.v.value_f=2.0;
    rValue.data_length = 1;
    intTest.l_variable = &lValue;
    intTest.r_variable = &rValue;

    D8U result = adm_cds_compare(&intTest, 1);
    EXPECT_EQ(result,0x01);    
}

TEST_F(CCdsCross_u64_float_LessEQ_Test, Normal)
{
    CCdsCompareTest aa;
    aa.cds_cross_u64_float_LessEQ_Test();
}

void CCdsCompareTest::cds_cross_u64_float_LessEQ_And_Test()
{
    cds_compare_st intTest[2];
    intTest[0].type = COMPARE_SYMBOL_LESSEQ;
    cds_variable_st lValue;
    lValue.data_type = VAR_U64;
    lValue.v.value_u64=1;
    lValue.data_length = 1;
    cds_variable_st rValue;
    rValue.data_type = VAR_FLOAT;
    
    rValue.v.value_f=2.0;
    rValue.data_length = 1;
    intTest[0].l_variable = &lValue;
    intTest[0].r_variable = &rValue;
    intTest[0].connect_symbol = COMPARE_AND;

    intTest[1].type = COMPARE_SYMBOL_LESSEQ;
    cds_variable_st lValue1;
    lValue1.data_type = VAR_U64;
    lValue1.v.value_u64=1;
    lValue1.data_length = 1;
    cds_variable_st rValue1;
    rValue1.data_type = VAR_FLOAT;
    
    rValue1.v.value_f=2.0;
    rValue1.data_length = 1;
    intTest[1].l_variable = &lValue1;
    intTest[1].r_variable = &rValue1;
    intTest[1].connect_symbol = COMPARE_AND;
    D8U result = adm_cds_compare(intTest, 2);
    EXPECT_EQ(result,0x01);    
}

TEST_F(CCdsCross_u64_float_LessEQ_And_Test, Normal)
{
    CCdsCompareTest aa;
    aa.cds_cross_u64_float_LessEQ_And_Test();
}

void CCdsCompareTest::cds_cross_u64_float_LessEQ_Or_Test()
{
    cds_compare_st intTest[2];
    intTest[0].type = COMPARE_SYMBOL_LESSEQ;
    cds_variable_st lValue;
    lValue.data_type = VAR_U64;
    lValue.v.value_u64=1;
    lValue.data_length = 1;
    cds_variable_st rValue;
    rValue.data_type = VAR_FLOAT;
    
    rValue.v.value_f=2.0;
    rValue.data_length = 1;
    intTest[0].l_variable = &lValue;
    intTest[0].r_variable = &rValue;
    intTest[0].connect_symbol = COMPARE_OR;
    intTest[1].type = COMPARE_SYMBOL_LESSEQ;
    cds_variable_st lValue1;
    lValue1.data_type = VAR_U64;
    lValue1.v.value_u64=1;
    lValue1.data_length = 1;
    cds_variable_st rValue1;
    rValue1.data_type = VAR_FLOAT;
    
    rValue1.v.value_f=2.0;
    rValue1.data_length = 1;
    intTest[1].l_variable = &lValue1;
    intTest[1].r_variable = &rValue1;
    intTest[1].connect_symbol = COMPARE_OR;
    D8U result = adm_cds_compare(intTest, 2);
    EXPECT_EQ(result,0x01);     
}

TEST_F(CCdsCross_u64_float_LessEQ_Or_Test, Normal)
{
    CCdsCompareTest aa;
    aa.cds_cross_u64_float_LessEQ_Or_Test();
}