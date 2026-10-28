#include "adm_dim_calHash.h"
#include "adm_dim_vdi.h"

#include "adm_com_hash.h"
#include "adm_memory.h"


static const DCHAR* gArrPkgFile[] =
{
    (DCHAR*)"/renzhendi/tmp/0x06e2_0/PDC_S0000001550_000100_FLD1_MCU_UDS_20220829.cbf",
    (DCHAR*)"/renzhendi/tmp/0x06e2_0/PDC_S0000001550_000100_FLD1_MCU_UDS_20220829.rsa",
    (DCHAR*)"/renzhendi/tmp/0x06e2_0/PDC_S0000001551_000500_ASW1_MCU_UDS_20220829.cbf",
    (DCHAR*)"/renzhendi/tmp/0x06e2_0/PDC_S0000001551_000500_ASW1_MCU_UDS_20220830.rsa",
    (DCHAR*)"/renzhendi/tmp/0x06e2_0/PDC_S0000010775_000001_CAL1_MCU_UDS_20240326.cbf",
    (DCHAR*)"/renzhendi/tmp/0x06e2_0/PDC_S0000010775_000001_CAL1_MCU_UDS_20240326.rsa"
};


void calHash()
{
    DCHAR oriFileHash[256] = {0};

    for(D32U i =0;i<sizeof(gArrPkgFile) / sizeof(gArrPkgFile[0]); ++i)
    {
        adm_com_sha256File(gArrPkgFile[i], oriFileHash, (D32U)(sizeof(oriFileHash) / sizeof(oriFileHash[0])));
        log_i("oriFileHash=%s", oriFileHash);
        adm_memzero(oriFileHash, 256);
    }
    
}





