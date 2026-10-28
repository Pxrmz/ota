#include "adm_com_modDefine.h"
#include "gtest/gtest.h"

namespace{

TEST(common_mod_test, common_mod){
    EXPECT_STREQ("CVM", adm_com_getModName(ADM_COM_MOD_CVM));
    EXPECT_STREQ("VRM", adm_com_getModName(ADM_COM_MOD_VRM));
    EXPECT_STREQ("DIM", adm_com_getModName(ADM_COM_MOD_DIM));
    EXPECT_STREQ("UIM", adm_com_getModName(ADM_COM_MOD_UIM));
    EXPECT_STREQ("CDM", adm_com_getModName(ADM_COM_MOD_CDM));
    EXPECT_STREQ("IRM", adm_com_getModName(ADM_COM_MOD_IRM));
    EXPECT_STREQ("DPM", adm_com_getModName(ADM_COM_MOD_DPM));
    EXPECT_STREQ("IPM", adm_com_getModName(ADM_COM_MOD_IPM));
    EXPECT_STREQ("IPO", adm_com_getModName(ADM_COM_MOD_IPO));
    EXPECT_STREQ("ETM", adm_com_getModName(ADM_COM_MOD_ETM));
    EXPECT_STREQ("VSS", adm_com_getModName(ADM_COM_MOD_VSS));
    EXPECT_STREQ("VSM", adm_com_getModName(ADM_COM_MOD_VSM));
    EXPECT_STREQ("VRS", adm_com_getModName(ADM_COM_MOD_VRS));
    EXPECT_STREQ("IRS", adm_com_getModName(ADM_COM_MOD_IRS));
    EXPECT_STREQ("OCS", adm_com_getModName(ADM_COM_MOD_OCS));
    EXPECT_STREQ("OCA", adm_com_getModName(ADM_COM_MOD_OCA));
    EXPECT_STREQ("ECS", adm_com_getModName(ADM_COM_MOD_ECS));
    EXPECT_STREQ("ECM", adm_com_getModName(ADM_COM_MOD_ECM));
    EXPECT_STREQ("CVS", adm_com_getModName(ADM_COM_MOD_CVS));
    EXPECT_STREQ("PIS", adm_com_getModName(ADM_COM_MOD_PIS));
    EXPECT_STREQ("DPO", adm_com_getModName(ADM_COM_MOD_DPO));
    EXPECT_STREQ("DPS", adm_com_getModName(ADM_COM_MOD_DPS));
    EXPECT_STREQ("IPS", adm_com_getModName(ADM_COM_MOD_IPS));
    EXPECT_STREQ("TPM", adm_com_getModName(ADM_COM_MOD_TPM));
    EXPECT_STREQ("TPS", adm_com_getModName(ADM_COM_MOD_TPS));
    EXPECT_STREQ("ISS", adm_com_getModName(ADM_COM_MOD_ISS));
    EXPECT_STREQ("COM", adm_com_getModName(ADM_COM_MOD_COM));
    EXPECT_STREQ("APC", adm_com_getModName(ADM_COM_MOD_APC));
    EXPECT_STREQ("4UCL", adm_com_getModName(ADM_COM_MOD_SYSI4UCL));
    EXPECT_STREQ("4VDI", adm_com_getModName(ADM_COM_MOD_SYSI4VDI));
    EXPECT_STREQ("4HOS", adm_com_getModName(ADM_COM_MOD_SYSI4HOST));
    EXPECT_STREQ("4HMI", adm_com_getModName(ADM_COM_MOD_SYSI4HMI));
    EXPECT_STREQ("SOIP", adm_com_getModName(ADM_COM_MOD_SOMEIP));
    EXPECT_STREQ("NSIP", adm_com_getModName(ADM_COM_MOD_NETPACK_VRPC_SOMEIP));
    EXPECT_STREQ("NSOC", adm_com_getModName(ADM_COM_MOD_NETPACK_VRPC_RAWSOC));
    EXPECT_STREQ("NHTP", adm_com_getModName(ADM_COM_MOD_NETPACK_AIR_HTTP));
    EXPECT_STREQ("NMQT", adm_com_getModName(ADM_COM_MOD_NETPACK_AIR_MQTT));
    EXPECT_STREQ("CFG", adm_com_getModName(ADM_COM_MOD_CFG));
    EXPECT_STREQ("EIM", adm_com_getModName(ADM_COM_MOD_EIM));
    EXPECT_STREQ("INVA", adm_com_getModName(ADM_COM_MOD_INVALID));
    EXPECT_STREQ("INVA", adm_com_getModName(static_cast<ADM_COM_MOD_E>(ADM_COM_MOD_INVALID + 1)));
}

}
