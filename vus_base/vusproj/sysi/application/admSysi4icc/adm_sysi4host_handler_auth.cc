#include "json_types.h"
#include "json_tokener.h"

#include "adm_sysi4host_common.h"
#include "adm_cfg_api.h"
#include "adm_sysi4all_loader.h"
#include "adm_sysi4icc.h"
//#include "com_abupdate_vus_host_HostService.h"
#include "adm_sysi4host_jni.h"
#include "adm_memory.h"
#include "adm_sysi_keylog.h"


//
extern ADM_SYSI4HOST_JNI_CALLBACK_T stSysi4hostJniCb;

static D32S retry_count = 0;
static D32S g_iPowerOnResult = ADM_ECODE_FAILED;
static D32S g_bGetPowerOnResult = DFALSE;
static D32S g_iPowerOffResult = ADM_ECODE_FAILED;
static D32S g_bGetPowerOffResult = DFALSE;
static D32S g_bPowerOptNow = DFALSE;
static D32S g_bPowerOffOptNow = DFALSE;
static D32S g_index = 0;
static D32S g_41AIndex = 0;
#define ADM_SYSI4HOST_POWER_ON_TIMEOUT  6 * 3      ///!! 远程上电的超时时间默认 6 * 3s（3次重试逻辑由IMMO自己处理）
#define ADM_SYSI4HOST_POWER_OFF_TIMEOUT 6          ///!! 远程下电的超时时间默认 6s

#define Nr 10 // number of rounds
#define ENTRY 0xA9
#define SFNUM 176   //11*16


#define CAN_PRINT_U8S(prefix, str, len)                                \
    do {                                                       \
        D8U log[256] = {0};                                     \
        D16U index = 0;                                         \
        for (; index < len; index++) {                         \
            sprintf((DCHAR*)&log[2 * index], "%02X", str[index]); \
        }                                                      \
        log_i("[CAN Data][%s]:%s\r\n", prefix, log);                   \
    } while (0)

unsigned char RoundKey[(Nr + 1) << 4];
//Value of S-box is as below
const unsigned char Sbox[256] =
{
    0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5, 0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76,
    0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0, 0xad, 0xd4, 0xa2, 0xaf, 0x9c, 0xa4, 0x72, 0xc0,
    0xb7, 0xfd, 0x93, 0x26, 0x36, 0x3f, 0xf7, 0xcc, 0x34, 0xa5, 0xe5, 0xf1, 0x71, 0xd8, 0x31, 0x15,
    0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05, 0x9a, 0x07, 0x12, 0x80, 0xe2, 0xeb, 0x27, 0xb2, 0x75,
    0x09, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0, 0x52, 0x3b, 0xd6, 0xb3, 0x29, 0xe3, 0x2f, 0x84,
    0x53, 0xd1, 0x00, 0xed, 0x20, 0xfc, 0xb1, 0x5b, 0x6a, 0xcb, 0xbe, 0x39, 0x4a, 0x4c, 0x58, 0xcf,
    0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85, 0x45, 0xf9, 0x02, 0x7f, 0x50, 0x3c, 0x9f, 0xa8,
    0x51, 0xa3, 0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5, 0xbc, 0xb6, 0xda, 0x21, 0x10, 0xff, 0xf3, 0xd2,
    0xcd, 0x0c, 0x13, 0xec, 0x5f, 0x97, 0x44, 0x17, 0xc4, 0xa7, 0x7e, 0x3d, 0x64, 0x5d, 0x19, 0x73,
    0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88, 0x46, 0xee, 0xb8, 0x14, 0xde, 0x5e, 0x0b, 0xdb,
    0xe0, 0x32, 0x3a, 0x0a, 0x49, 0x06, 0x24, 0x5c, 0xc2, 0xd3, 0xac, 0x62, 0x91, 0x95, 0xe4, 0x79,
    0xe7, 0xc8, 0x37, 0x6d, 0x8d, 0xd5, 0x4e, 0xa9, 0x6c, 0x56, 0xf4, 0xea, 0x65, 0x7a, 0xae, 0x08,
    0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6, 0xb4, 0xc6, 0xe8, 0xdd, 0x74, 0x1f, 0x4b, 0xbd, 0x8b, 0x8a,
    0x70, 0x3e, 0xb5, 0x66, 0x48, 0x03, 0xf6, 0x0e, 0x61, 0x35, 0x57, 0xb9, 0x86, 0xc1, 0x1d, 0x9e,
    0xe1, 0xf8, 0x98, 0x11, 0x69, 0xd9, 0x8e, 0x94, 0x9b, 0x1e, 0x87, 0xe9, 0xce, 0x55, 0x28, 0xdf,
    0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42, 0x68, 0x41, 0x99, 0x2d, 0x0f, 0xb0, 0x54, 0xbb, 0x16
};
const unsigned char Xtime2Sbox[256] =
{
    0xc6, 0xf8, 0xee, 0xf6, 0xff, 0xd6, 0xde, 0x91, 0x60, 0x02, 0xce, 0x56, 0xe7, 0xb5, 0x4d, 0xec,
    0x8f, 0x1f, 0x89, 0xfa, 0xef, 0xb2, 0x8e, 0xfb, 0x41, 0xb3, 0x5f, 0x45, 0x23, 0x53, 0xe4, 0x9b,
    0x75, 0xe1, 0x3d, 0x4c, 0x6c, 0x7e, 0xf5, 0x83, 0x68, 0x51, 0xd1, 0xf9, 0xe2, 0xab, 0x62, 0x2a,
    0x08, 0x95, 0x46, 0x9d, 0x30, 0x37, 0x0a, 0x2f, 0x0e, 0x24, 0x1b, 0xdf, 0xcd, 0x4e, 0x7f, 0xea,
    0x12, 0x1d, 0x58, 0x34, 0x36, 0xdc, 0xb4, 0x5b, 0xa4, 0x76, 0xb7, 0x7d, 0x52, 0xdd, 0x5e, 0x13,
    0xa6, 0xb9, 0x00, 0xc1, 0x40, 0xe3, 0x79, 0xb6, 0xd4, 0x8d, 0x67, 0x72, 0x94, 0x98, 0xb0, 0x85,
    0xbb, 0xc5, 0x4f, 0xed, 0x86, 0x9a, 0x66, 0x11, 0x8a, 0xe9, 0x04, 0xfe, 0xa0, 0x78, 0x25, 0x4b,
    0xa2, 0x5d, 0x80, 0x05, 0x3f, 0x21, 0x70, 0xf1, 0x63, 0x77, 0xaf, 0x42, 0x20, 0xe5, 0xfd, 0xbf,
    0x81, 0x18, 0x26, 0xc3, 0xbe, 0x35, 0x88, 0x2e, 0x93, 0x55, 0xfc, 0x7a, 0xc8, 0xba, 0x32, 0xe6,
    0xc0, 0x19, 0x9e, 0xa3, 0x44, 0x54, 0x3b, 0x0b, 0x8c, 0xc7, 0x6b, 0x28, 0xa7, 0xbc, 0x16, 0xad,
    0xdb, 0x64, 0x74, 0x14, 0x92, 0x0c, 0x48, 0xb8, 0x9f, 0xbd, 0x43, 0xc4, 0x39, 0x31, 0xd3, 0xf2,
    0xd5, 0x8b, 0x6e, 0xda, 0x01, 0xb1, 0x9c, 0x49, 0xd8, 0xac, 0xf3, 0xcf, 0xca, 0xf4, 0x47, 0x10,
    0x6f, 0xf0, 0x4a, 0x5c, 0x38, 0x57, 0x73, 0x97, 0xcb, 0xa1, 0xe8, 0x3e, 0x96, 0x61, 0x0d, 0x0f,
    0xe0, 0x7c, 0x71, 0xcc, 0x90, 0x06, 0xf7, 0x1c, 0xc2, 0x6a, 0xae, 0x69, 0x17, 0x99, 0x3a, 0x27,
    0xd9, 0xeb, 0x2b, 0x22, 0xd2, 0xa9, 0x07, 0x33, 0x2d, 0x3c, 0x15, 0xc9, 0x87, 0xaa, 0x50, 0xa5,
    0x03, 0x59, 0x09, 0x1a, 0x65, 0xd7, 0x84, 0xd0, 0x82, 0x29, 0x5a, 0x1e, 0x7b, 0xa8, 0x6d, 0x2c
};
const unsigned char Xtime3Sbox[256] =
{
    0xa5, 0x84, 0x99, 0x8d, 0x0d, 0xbd, 0xb1, 0x54, 0x50, 0x03, 0xa9, 0x7d, 0x19, 0x62, 0xe6, 0x9a,
    0x45, 0x9d, 0x40, 0x87, 0x15, 0xeb, 0xc9, 0x0b, 0xec, 0x67, 0xfd, 0xea, 0xbf, 0xf7, 0x96, 0x5b,
    0xc2, 0x1c, 0xae, 0x6a, 0x5a, 0x41, 0x02, 0x4f, 0x5c, 0xf4, 0x34, 0x08, 0x93, 0x73, 0x53, 0x3f,
    0x0c, 0x52, 0x65, 0x5e, 0x28, 0xa1, 0x0f, 0xb5, 0x09, 0x36, 0x9b, 0x3d, 0x26, 0x69, 0xcd, 0x9f,
    0x1b, 0x9e, 0x74, 0x2e, 0x2d, 0xb2, 0xee, 0xfb, 0xf6, 0x4d, 0x61, 0xce, 0x7b, 0x3e, 0x71, 0x97,
    0xf5, 0x68, 0x00, 0x2c, 0x60, 0x1f, 0xc8, 0xed, 0xbe, 0x46, 0xd9, 0x4b, 0xde, 0xd4, 0xe8, 0x4a,
    0x6b, 0x2a, 0xe5, 0x16, 0xc5, 0xd7, 0x55, 0x94, 0xcf, 0x10, 0x06, 0x81, 0xf0, 0x44, 0xba, 0xe3,
    0xf3, 0xfe, 0xc0, 0x8a, 0xad, 0xbc, 0x48, 0x04, 0xdf, 0xc1, 0x75, 0x63, 0x30, 0x1a, 0x0e, 0x6d,
    0x4c, 0x14, 0x35, 0x2f, 0xe1, 0xa2, 0xcc, 0x39, 0x57, 0xf2, 0x82, 0x47, 0xac, 0xe7, 0x2b, 0x95,
    0xa0, 0x98, 0xd1, 0x7f, 0x66, 0x7e, 0xab, 0x83, 0xca, 0x29, 0xd3, 0x3c, 0x79, 0xe2, 0x1d, 0x76,
    0x3b, 0x56, 0x4e, 0x1e, 0xdb, 0x0a, 0x6c, 0xe4, 0x5d, 0x6e, 0xef, 0xa6, 0xa8, 0xa4, 0x37, 0x8b,
    0x32, 0x43, 0x59, 0xb7, 0x8c, 0x64, 0xd2, 0xe0, 0xb4, 0xfa, 0x07, 0x25, 0xaf, 0x8e, 0xe9, 0x18,
    0xd5, 0x88, 0x6f, 0x72, 0x24, 0xf1, 0xc7, 0x51, 0x23, 0x7c, 0x9c, 0x21, 0xdd, 0xdc, 0x86, 0x85,
    0x90, 0x42, 0xc4, 0xaa, 0xd8, 0x05, 0x01, 0x12, 0xa3, 0x5f, 0xf9, 0xd0, 0x91, 0x58, 0x27, 0xb9,
    0x38, 0x13, 0xb3, 0x33, 0xbb, 0x70, 0x89, 0xa7, 0xb6, 0x22, 0x92, 0x20, 0x49, 0xff, 0x78, 0x7a,
    0x8f, 0xf8, 0x80, 0x17, 0xda, 0x31, 0xc6, 0xb8, 0xc3, 0xb0, 0x77, 0x11, 0xcb, 0xfc, 0xd6, 0x3a
};
//SubByte,ShiftRow
static void ShiftRows(unsigned char* state)
{
    unsigned char tmp;
    // just substitute row 0
    state[0] = Sbox[state[0]], state[4] = Sbox[state[4]];
    state[8] = Sbox[state[8]], state[12] = Sbox[state[12]];
    // rotate row 1
    tmp = Sbox[state[1]], state[1] = Sbox[state[5]];
    state[5] = Sbox[state[9]], state[9] = Sbox[state[13]], state[13] = tmp;
    // rotate row 2
    tmp = Sbox[state[2]], state[2] = Sbox[state[10]], state[10] = tmp;
    tmp = Sbox[state[6]], state[6] = Sbox[state[14]], state[14] = tmp;
    // rotate row 3
    tmp = Sbox[state[15]], state[15] = Sbox[state[11]];
    state[11] = Sbox[state[7]], state[7] = Sbox[state[3]], state[3] = tmp;
}
// SubByte,ShiftRow,MixColumn
static void MixColumns(unsigned char* state)
{
    unsigned char newstate[16];
    // mixing column 0
    newstate[0] = Xtime2Sbox[state[0]] ^ Xtime3Sbox[state[5]] ^ Sbox[state[10]] ^ Sbox[state[15]];
    newstate[1] = Sbox[state[0]] ^ Xtime2Sbox[state[5]] ^ Xtime3Sbox[state[10]] ^ Sbox[state[15]];
    newstate[2] = Sbox[state[0]] ^ Sbox[state[5]] ^ Xtime2Sbox[state[10]] ^ Xtime3Sbox[state[15]];
    newstate[3] = Xtime3Sbox[state[0]] ^ Sbox[state[5]] ^ Sbox[state[10]] ^ Xtime2Sbox[state[15]];
    // mixing column 1
    newstate[4] = Xtime2Sbox[state[4]] ^ Xtime3Sbox[state[9]] ^ Sbox[state[14]] ^ Sbox[state[3]];
    newstate[5] = Sbox[state[4]] ^ Xtime2Sbox[state[9]] ^ Xtime3Sbox[state[14]] ^ Sbox[state[3]];
    newstate[6] = Sbox[state[4]] ^ Sbox[state[9]] ^ Xtime2Sbox[state[14]] ^ Xtime3Sbox[state[3]];
    newstate[7] = Xtime3Sbox[state[4]] ^ Sbox[state[9]] ^ Sbox[state[14]] ^ Xtime2Sbox[state[3]];
    // mixing column 2
    newstate[8] = Xtime2Sbox[state[8]] ^ Xtime3Sbox[state[13]] ^ Sbox[state[2]] ^ Sbox[state[7]];
    newstate[9] = Sbox[state[8]] ^ Xtime2Sbox[state[13]] ^ Xtime3Sbox[state[2]] ^ Sbox[state[7]];
    newstate[10] = Sbox[state[8]] ^ Sbox[state[13]] ^ Xtime2Sbox[state[2]] ^ Xtime3Sbox[state[7]];
    newstate[11] = Xtime3Sbox[state[8]] ^ Sbox[state[13]] ^ Sbox[state[2]] ^ Xtime2Sbox[state[7]];
    // mixing column 3
    newstate[12] = Xtime2Sbox[state[12]] ^ Xtime3Sbox[state[1]] ^ Sbox[state[6]] ^ Sbox[state[11]];
    newstate[13] = Sbox[state[12]] ^ Xtime2Sbox[state[1]] ^ Xtime3Sbox[state[6]] ^ Sbox[state[11]];
    newstate[14] = Sbox[state[12]] ^ Sbox[state[1]] ^ Xtime2Sbox[state[6]] ^ Xtime3Sbox[state[11]];
    newstate[15] = Xtime3Sbox[state[12]] ^ Sbox[state[1]] ^ Sbox[state[6]] ^ Xtime2Sbox[state[11]];
    (void)memcpy(state, newstate, sizeof(newstate));
}

static void AddRoundKey(unsigned char* state, unsigned char* key)
{
    int idx;

    for(idx = 0; idx < 16; idx++)   //note: PC's "unsigned" is 4 bytes
    {
        state[idx] ^= key[idx];
    }
}

const unsigned char Rcon[11] = {0x00, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1b, 0x36};

//获取鉴权算法安全密钥接口
typedef struct
{
    D8U sc[4];          // 4 byte
    D8U sk[16];          // 16 byte
    D8U reserved[256];
} SKSCInfo;
#define ADM_SKSCINFO_SC         "sc"
#define ADM_SKSCINFO_SK         "sk"
#define ADM_SKSCINFO_RESERVED   "reserved"



// use 1.366ms by 24MHz
void AES_init(unsigned char* key)   //Substitute into a Secret Key (SK), 16 bytes SK array, Actaully only use 8 bytes.
{
    unsigned char tmp0, tmp1, tmp2, tmp3, tmp4;
    unsigned idx;

    for(idx = 0; idx < 16; idx++)
    {
        if(idx < 8)
        {
            RoundKey[idx] = key[idx];
        }
        else
        {
            RoundKey[idx] = key[idx - 8]; //Actually only use 8 bytes key (one CAN frame)
        }
    }

    for(idx = 4; idx < ((Nr + 1) << 2); idx++)
    {
        tmp0 = RoundKey[(idx << 2) - 4];
        tmp1 = RoundKey[(idx << 2) - 3];
        tmp2 = RoundKey[(idx << 2) - 2];
        tmp3 = RoundKey[(idx << 2) - 1];

        if(!(idx % 4))
        {
            tmp4 = tmp3;
            tmp3 = Sbox[tmp0];
            tmp0 = Sbox[tmp1] ^ Rcon[idx / 4];
            tmp1 = Sbox[tmp2];
            tmp2 = Sbox[tmp4];
        }

        // convert from longs to bytes
        RoundKey[(idx << 2) + 0] = RoundKey[((idx - 4) << 2) + 0] ^ tmp0;
        RoundKey[(idx << 2) + 1] = RoundKey[((idx - 4) << 2) + 1] ^ tmp1;
        RoundKey[(idx << 2) + 2] = RoundKey[((idx - 4) << 2) + 2] ^ tmp2;
        RoundKey[(idx << 2) + 3] = RoundKey[((idx - 4) << 2) + 3] ^ tmp3;
    }

    //change the AES RoundKey,specified by chery
    tmp3 = ENTRY;
    RoundKey[0] ^= tmp3;

    for(idx = 1, tmp0 = 0, tmp1 = 1; idx < SFNUM; idx++)
    {
        RoundKey[idx % ((Nr + 1) << 4)] = RoundKey[tmp0++] ^ RoundKey[tmp1++] ^ (tmp3);
        tmp0 %= ((Nr + 1) << 4);
        tmp0 %= ((Nr + 1) << 4);
        tmp3 += 41;
    }
}

// use 0.931ms by 24MHz
// encrypt one 128-bits data
void Encrypt(unsigned char* Plain, unsigned char* Cipher)
{
    unsigned round, idx;
    unsigned char state[16];

    for(idx = 0; idx < 16; idx++)
    {
        state[idx] = *Plain++;
    }

    AddRoundKey(&state[0], &RoundKey[0]);

    for(round = 1; round < Nr + 1; round++)
    {
        if(round < Nr)
        {
            MixColumns(state);
        }
        else
        {
            ShiftRows(state);
        }

        AddRoundKey(&state[0], &RoundKey[round << 4]);
    }

    for(idx = 0; idx < 16; idx++)
    {
        *Cipher++ = state[idx];
    }
}

static DBOOL adm_getJsonStrVal(const json_object* obj, const DCHAR* key, DCHAR** str)
{
    if(NULL == obj || NULL == key)
    {
        return FALSE;
    }

    json_object* jsonStr = NULL;

    if((json_bool)0 == json_object_object_get_ex(obj, key, &jsonStr))
    {
        return FALSE;
    }

    *str = adm_strdup(json_object_get_string(jsonStr));

    if(NULL != *str)
    {
        return TRUE;
    }

    return FALSE;
}

static D32S __lionSkScInfoSwitch(DCHAR* pInput, D8U* pOutput, D16U uiOutputLen)
{
    D32S index = 0;
    while (*pInput && index < uiOutputLen) {
        if (strncmp(pInput, "0x", 2) == 0) {
            pInput += 2; // 跳过 "0x"
        } else {
            // 读取一个十六进制数字字符
            sscanf(pInput, "%2hhx", &pOutput[index++]);

            // 移动到下一个数字字符或字符串结束
            while (*pInput && *pInput != ',' && *pInput != ' ')
            {
                pInput++;
            }

            //跳过逗号和空格
            while (*pInput && (*pInput == ',' || *pInput == ' '))
            {
                pInput++;
            } 
        }
    }

    CAN_PRINT_U8S("SkScInfo", pOutput, uiOutputLen);

    return 0;
}

static SKSCInfo* _lionOtaGetSKSCInfo()
{
    //17个字符
    D32S iRet = ADM_ECODE_SUCCESSED;
    json_object* sksInfoObject = NULL;
    DCHAR* pSKSCInfo = NULL;
    SKSCInfo* sksInfo = NULL;
    DCHAR* sk = DNULL;
    DCHAR* sc = DNULL;
    DCHAR* reserved = DNULL;
    D32S i = 0;

    //iRet = adm_sysi4host_getSkScInfoCallback(&pSKSCInfo);
    if(NULL != stSysi4hostJniCb.getSkScInfoCb)
    {
        iRet = stSysi4hostJniCb.getSkScInfoCb(&pSKSCInfo);
    }

    if(iRet || NULL == pSKSCInfo)
    {
        log_e("get SKSCInfo failed.iRet[%d]", iRet);
        return NULL;
    }

    sksInfoObject = json_tokener_parse(pSKSCInfo);

    if(!sksInfoObject)
    {
        log_e("str to son_object. failed");
        return NULL;
    }

    sksInfo = (SKSCInfo*)adm_malloc(sizeof(SKSCInfo));

    if(sksInfo)
    {
        adm_memset(sksInfo, 0x00, sizeof(SKSCInfo), sizeof(SKSCInfo));
    }

    if(!adm_getJsonStrVal(sksInfoObject, ADM_SKSCINFO_SC, &(sc)))
    {
        log_e("get sc value from json_object. failed");
    }
    else
    {
        log_i("get sc value from json_object. successed,  sc:%s", sc);
        __lionSkScInfoSwitch(sc, sksInfo->sc, 4);
        CAN_PRINT_U8S("SkSc", sksInfo->sc, 4);
    }

    if(!adm_getJsonStrVal(sksInfoObject, ADM_SKSCINFO_SK, &(sk)))
    {
        log_e("get sk value from json_object. failed");
    }
    else
    {
        log_i("get sk value from json_object. successed,  sk:%s", sk);
        __lionSkScInfoSwitch(sk, sksInfo->sk, 16);
        CAN_PRINT_U8S("SkSk", sksInfo->sk, 16);
    }

    if(!adm_getJsonStrVal(sksInfoObject, ADM_SKSCINFO_RESERVED, &(reserved)))
    {
        log_e("get reserved value from json_object. failed");
    }
    else
    {
        log_i("get sk value from json_object. successed,  reserved:%s", reserved);
        __lionSkScInfoSwitch(reserved, sksInfo->reserved, 16);
        CAN_PRINT_U8S("SkReserved", sksInfo->reserved, 16);
        // log_i("get reserved value from json_object. successed,  reserved:%s", sksInfo->reserved);
    }

    adm_free(pSKSCInfo);
    if (DNULL != sc)
    {
        adm_free(sc);
    }
    if (DNULL != sk)
    {
        adm_free(sk);
    }
    if (DNULL != reserved)
    {
        adm_free(reserved);
    }
    json_object_put(sksInfoObject);

    return sksInfo;
}

static void __sksInfoDel(SKSCInfo** sksInfo)
{
    if(NULL != sksInfo &&  NULL != *sksInfo)
    {
        // adm_free(*sksInfo->sc);
        // adm_free(*sksInfo->sk);
        // adm_free(*sksInfo->reserved);

        adm_free(*sksInfo);
    }

    return;
}


static D32S sysi4hostSendAuthenticateEventCb(D32S iEventId, D32S* pErrorCode)
{ 
    D32S iRet = ADM_ECODE_SUCCESSED;
    DCHAR aDescription[128] = {0};
    CHECK_POINTER_RETURN_RET(stSysi4hostJniCb.sendAuthenticateEventCb, ADM_ECODE_INVALID_POINTER);

    iRet = stSysi4hostJniCb.sendAuthenticateEventCb(iEventId, pErrorCode);

    adm_snprintf(aDescription, sizeof(aDescription), "eventId=%d,errorCode=%d, 0-powerOn 1-ContinuousON 2-NoRequest 3-powerOFF", iEventId,*pErrorCode);
    if (ADM_IS_ERR(iRet) || ADM_IS_ERR(*pErrorCode))
    {
        adm_sysi_setKeylogString(ADM_KEYLOG_ID_028, iRet, aDescription);
    }
    else
    {
        adm_sysi_setKeylogString(ADM_KEYLOG_ID_027, iRet, aDescription);
    }

    return iRet;
}


// 通知 UC 上电结果 -1:异常 1:成功 2:失败
D32S sysi4host_powerOnResultNotify(D32S result)
{
    log_i("sysi4host_powerOnResultNotify begin.result[%d]", result);
    D32S iResult = ADM_ECODE_FAILED;
    D32S iRet = ADM_ECODE_FAILED;
    D32S iErrorCode = ADM_ECODE_FAILED;
    DCHAR aDescription[128] = {0};
	
    adm_snprintf(aDescription, sizeof(aDescription), "result=%d, -1:异常 1:成功 2:失败", result);
    if (1 != result)
    {
        adm_sysi_setKeylogString(ADM_KEYLOG_ID_030, ADM_ECODE_FAILED, aDescription);
    }
    else
    {
        adm_sysi_setKeylogString(ADM_KEYLOG_ID_029, ADM_ECODE_SUCCESSED, aDescription);
    }
	
	if (DFALSE == g_bPowerOptNow)
	{
		log_e("sysi4host_powerOnResultNotify abnomal");
		return iResult;
	}

    if (1 == result)
    {
        iResult = ADM_ECODE_SUCCESSED;
    }

    /** 上电失败，发送No Request */
    if (ADM_ECODE_SUCCESSED != iResult)
    {
        //通知No Request
        if(stSysi4hostJniCb.sendAuthenticateEventCb)
        {
            for (D32S i = 0; i < 3; i++)
            {
                iRet = sysi4hostSendAuthenticateEventCb(ADM_AUTH_NO_REQUEST_EVENT, &iErrorCode);
                if(iRet || iErrorCode)
                {
                    log_e("sendAuthenticateEventCb eventId:%d failed, iRet %d ErrorCode:%d", ADM_AUTH_NO_REQUEST_EVENT, iRet, iErrorCode);
                }
                else
                {
                    log_i("sendAuthenticateEventCb eventId:%d successed", ADM_AUTH_NO_REQUEST_EVENT);
                    break;
                }
                sleep(1);
            }
        }
    }
    else 
    {
        //通知Continuous ON
        if(stSysi4hostJniCb.sendAuthenticateEventCb)
        {
            for (D32S i = 0; i < 3; i++)
            {
                iRet = sysi4hostSendAuthenticateEventCb(ADM_AUTH_CONTINUOS_ON_EVENT, &iErrorCode);
                if(iRet || iErrorCode)
                {
                    log_e("sendAuthenticateEventCb eventId:%d failed, iRet %d ErrorCode:%d", ADM_AUTH_CONTINUOS_ON_EVENT, iRet, iErrorCode);
                }
                else
                {
                    log_i("sendAuthenticateEventCb eventId:%d successed", ADM_AUTH_CONTINUOS_ON_EVENT);
                    break;
                }
                sleep(1);
            }
        }
    }
	
	if(g_bPowerOptNow == DTRUE)
	{  
		log_i("iResult :%d", iResult);
		g_iPowerOnResult = iResult;
    	g_bGetPowerOnResult = DTRUE;
	}
  
    retry_count = 0;

    log_i("sysi4host_powerOnResultNotify end");

    return 0;
}

// 通知 UC 下电结果 -1:异常 1:成功 2:失败
D32S sysi4host_powerOffResultNotify(D32S result)
{
    log_i("sysi4host_powerOffResultNotify begin.result[%d]", result);
    D32S iResult = ADM_ECODE_FAILED;
    D32S iRet = ADM_ECODE_FAILED;
    D32S iErrorCode = ADM_ECODE_FAILED;
    DCHAR aDescription[128] = {0};
    DCHAR aSolution[128] = {0};
	
    adm_snprintf(aDescription, sizeof(aDescription), "result=%d, -1:异常 1:成功 2:失败", result);
    if (1 != result)
    {
        adm_sysi_setKeylogString(ADM_KEYLOG_ID_032, ADM_ECODE_FAILED, aDescription);
    }
    else
    {
        adm_sysi_setKeylogString(ADM_KEYLOG_ID_031, ADM_ECODE_SUCCESSED, aDescription);
    }
	
	if (DFALSE == g_bPowerOffOptNow)
	{
		log_e("sysi4host_powerOffResultNotify abnormal");
		return iResult;
	}

    if (1 == result)
    {
        iResult = ADM_ECODE_SUCCESSED;
    }

    //通知No Request
    if(stSysi4hostJniCb.sendAuthenticateEventCb)
    {
        for (D32S i = 0; i < 3; i++)
        {
            iRet = sysi4hostSendAuthenticateEventCb(ADM_AUTH_NO_REQUEST_EVENT, &iErrorCode);
            if(iRet || iErrorCode)
            {
                log_e("sendAuthenticateEventCb eventId:%d failed, iRet %d ErrorCode:%d", ADM_AUTH_NO_REQUEST_EVENT, iRet, iErrorCode);
            }
            else
            {
                log_i("sendAuthenticateEventCb eventId:%d successed", ADM_AUTH_NO_REQUEST_EVENT);
                break;
            }
            sleep(1);
        }
    }

    g_iPowerOffResult = iResult;
    g_bGetPowerOffResult = DTRUE;

    log_i("sysi4host_powerOffResultNotify end");

    return 0;
}

#define ADM_ECODE_INSTALL_REMOTE_POWER_ON_FAILED       50325    /** 远程上电失败 */
//车辆鉴权随机值监听
D32S adm_sysi4host_canResponseNotify(D8U* canMsg)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    // All the IMMO and ICC have the same constant code (CC) that equal to “0x4368657279534844”
    D8U cc_str[8] = {0x43, 0x68, 0x65, 0x72, 0x79, 0x53, 0x48, 0x44};     // CherySHD
    D8U temp_str[8] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    D8U rn_str[4] = {'\0'};
    D8U A_str[4] = {'\0'};
    D8U assemble_str[17] =  {'\0'};
    D8U assemble_str_en[17] =  {'\0'};
    D8U en_str[9] = {'\0'};
    SKSCInfo* sksInfo = NULL;
    D32S i = 0;
    D32S iErrorCode = 0;
    D32S j = 0;

    adm_sysi_setKeylogString(ADM_KEYLOG_ID_033, iRet, NULL);

    if (DFALSE == g_bPowerOptNow)
    {
        g_index ++;
        if (g_index == (10 * 50 * 6))       ///!< 300s 打印一次，作为记录
        {
            log_d("Just get message.");
            g_index = 0;
        }
        return iRet;
    }
    log_d("Ota_authenticateResponse start.");

    CAN_PRINT_U8S("CanMsg", canMsg, 8);       //!< 8个字节

    // 1.解析出IMMO发送的随机数(RN-4byte)、32位加密结果A(4byte)
    for(i = 0; i < 4; i++)
    {
        rn_str[i] = canMsg[i];      // RN: 1-4 byte
    }
    CAN_PRINT_U8S("RandomStr", rn_str, 4);

    for(i = 0; i < 4; i++)
    {
        A_str[i] = canMsg[i + 4];   // 加密结果A: 5-8 byte
    }
    CAN_PRINT_U8S("EncryptResult", A_str, 4);

    // 2.获取安全秘钥(SK(16byte))、安全代码(SC(4byte))
    sksInfo = _lionOtaGetSKSCInfo();
    if(NULL == sksInfo)
    {
        log_e("get sksInfo failed");
        iErrorCode = ADM_ECODE_INSTALL_REMOTE_POWER_ON_FAILED;
        sysi4host_powerOnResultNotify(iErrorCode);
        return ADM_ECODE_FAILED;
    }

    // 3.按SC(4byte)+RN(4byte)+CC(8byte)顺序组合数据(16byte)
    CAN_PRINT_U8S("SkSc", sksInfo->sc, 4);
    CAN_PRINT_U8S("RandomStr", rn_str, 4);
    CAN_PRINT_U8S("CCStr", cc_str, 8);
    for (i = 0; i < 4; i++)
    {
        assemble_str[i] = sksInfo->sc[i];       //!< 拼接SC 4Byte
    }
    for (i = 4; i < 8; i++)
    {
        assemble_str[i] = rn_str[i - 4];        //!< 拼接rn 4Byte
    }
    for (i = 8; i < 16; i++)
    {
        assemble_str[i] = cc_str[i - 8];        //!< 拼接cc 8Byte
    }
    CAN_PRINT_U8S("AssembleStr", assemble_str, 16);

    // 4.将组合后的数据和SK传入算法中得到32位加密结果
    CAN_PRINT_U8S("SkSk", sksInfo->sk, 16);
    AES_init((D8U*)sksInfo->sk);
    Encrypt(assemble_str, assemble_str_en);
    CAN_PRINT_U8S("EncryptStr", assemble_str_en, 16);

    // 加密结果是16个byte，此处取后4个byte
    (void)adm_memcpy(en_str, assemble_str_en + 12, 4, 4);
    en_str[4] = '\0';
    CAN_PRINT_U8S("EncryptStrBehind4Byte", en_str, 4);

    if (0 != adm_memcmp(en_str, A_str, 4))
    {
        log_e("A_str != en_str do try 0xFFFFFFFFF");
        if(retry_count <= 3)
        {
            if(NULL != stSysi4hostJniCb.sendAuthenticateMsgCb)
            {
                CAN_PRINT_U8S("0xFFFFFFFF", temp_str, 8);
                iRet = stSysi4hostJniCb.sendAuthenticateMsgCb(temp_str, 8, &iErrorCode);

                retry_count ++;

                if(iRet || iErrorCode)
                {
                    log_e("sendAuthenticateMsgCb failed, iRet %d ErrorCode:%d; retry_count %d", iRet, iErrorCode, retry_count);
                }
                else
                {
                    log_i("sendAuthenticateMsgCb iRet %d ErrorCode:%d; retry_count %d successed", iRet, iErrorCode, retry_count);
                }
            }
        }
        else
        {
            retry_count = 0;
            
            //通知No Request
            if(stSysi4hostJniCb.sendAuthenticateEventCb)
            {
                for (D32S i = 0; i < 3; i++)
                {
                    iRet = sysi4hostSendAuthenticateEventCb(ADM_AUTH_NO_REQUEST_EVENT, &iErrorCode);
                    if(iRet || iErrorCode)
                    {
                        log_e("sendAuthenticateEventCb eventId:%d failed, iRet %d ErrorCode:%d", ADM_AUTH_NO_REQUEST_EVENT, iRet, iErrorCode);
                    }
                    else
                    {
                        log_i("sendAuthenticateEventCb eventId:%d successed", ADM_AUTH_NO_REQUEST_EVENT);
                        break;
                    }
                    sleep(1);
                }
            }

            // 通知UC上电失败
            log_e("power on failed!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
            iErrorCode = ADM_ECODE_INSTALL_REMOTE_POWER_ON_FAILED;
            sysi4host_powerOnResultNotify(iErrorCode);
        }
    }
    else
    {
        ///!< 重置为0，保证下一次上电的流程
        retry_count = 0;
        // 5.按CC+SC+RN顺序组合数据
        // sprintf((DCHAR*)assemble_str, "%s%s%s", cc_str, sksInfo->sc, rn_str);
        CAN_PRINT_U8S("CCStr", cc_str, 8);
        CAN_PRINT_U8S("SkSc", sksInfo->sc, 4);
        CAN_PRINT_U8S("RandomStr", rn_str, 4);
        for (i = 0; i < 8; i++)
        {
            assemble_str[i] = cc_str[i];       //!< 拼接CC 8Byte
        }
        for (i = 8; i < 12; i++)
        {
            assemble_str[i] = sksInfo->sc[i - 8];        //!< 拼接SC 4Byte
        }
        for (i = 12; i < 16; i++)
        {
            assemble_str[i] = rn_str[i - 12];        //!< 拼接RN 4Byte
        }
        assemble_str[16] = '\0';
        CAN_PRINT_U8S("AssembleStr", assemble_str, 16);

        // 6.将组合后数据和SK传入算法中得到64位加密结果C
        Encrypt(assemble_str, assemble_str_en);
        CAN_PRINT_U8S("AssembleStrEnc", assemble_str_en, 16);

        // 加密结果是16个byte，此处取后8个byte
        (void)adm_memcpy(en_str, assemble_str_en + 8, 8, 8);
        en_str[8] = '\0';
        CAN_PRINT_U8S("EncryptStrBehind8Byte", en_str, 8);

        // 7.发送加密结果C给IMMO
        if(NULL != stSysi4hostJniCb.sendAuthenticateMsgCb)
        {
            iRet = stSysi4hostJniCb.sendAuthenticateMsgCb(en_str, 8, &iErrorCode);

            if(!iRet && !iErrorCode)
            {
                log_i("sendAuthenticateMsgCb iRet %d ErrorCode:%d successed", iRet, iErrorCode);
            }
            else
            {
                log_e("sendAuthenticateMsgCb failed iRet %d ErrorCode:%d", iRet, iErrorCode);
            }
        }
    }

    __sksInfoDel(&sksInfo);

    log_d("Ota_authenticateResponse end.");
    return iRet;
}

/// @brief 发送车辆远程上下电事件
/// ** uc 请求上电（0:Power On），sysi先鉴权再上电；
/// ** uc 请求下电（3:Power Off），sysi先发送（3:Power Off），再发送（2:No Request）
/// @param eventId
/// @return
D32S sysi4host_powerStateEvent(D32S iEventId, D32S* pErrorCode)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    D32S iErrorCode = 0;
    D32S iCount = 0;

    CHECK_POINTER_RETURN_RET(stSysi4hostJniCb.sendAuthenticateEventCb, ADM_ECODE_INVALID_POINTER);

    log_i("iEventId[%d]", iEventId);
    if(ADM_AUTH_POWER_ON_EVENT == iEventId)
    {
        iRet = sysi4hostSendAuthenticateEventCb(iEventId, &iErrorCode);
        g_bPowerOptNow = DTRUE;
        if(!iRet && !iErrorCode)
        {
            log_i("sendAuthenticateEventCb successed eventId:%d, iRet %d ErrorCode:%d", iEventId, iRet, iErrorCode);
        }
        else
        {
            log_e("sendAuthenticateEventCb failed eventId:%d, iRet %d ErrorCode:%d", iEventId, iRet, iErrorCode);
        }
        
        while(iCount < ADM_SYSI4HOST_POWER_ON_TIMEOUT)
        {
            if (DTRUE == g_bGetPowerOnResult)
            {
                iErrorCode = g_iPowerOnResult;
                g_bGetPowerOnResult = DFALSE;
                break;
            }
            sleep(1);
            iCount++;
        }
        if (iCount >= ADM_SYSI4HOST_POWER_ON_TIMEOUT)
        {
            iErrorCode = -1;
            adm_sysi_setKeylogString(ADM_KEYLOG_ID_034, iErrorCode, NULL);
            log_e("Remote power on auth timeout [err:%d]", iErrorCode);
        }
        g_bPowerOptNow = DFALSE;
    }
    else if(ADM_AUTH_POWER_OFF_EVENT == iEventId)
    {
        iRet = sysi4hostSendAuthenticateEventCb(iEventId, &iErrorCode);
		g_bPowerOffOptNow = DTRUE;

        if(!iRet && !iErrorCode)
        {
            log_e("power off success");
            log_i("sendAuthenticateEventCb successed eventId:%d, iRet %d ErrorCode:%d", iEventId, iRet, iErrorCode);

        }
        else
        {
            log_e("sendAuthenticateEventCb failed eventId:%d, iRet %d ErrorCode:%d", iEventId, iRet, iErrorCode);
        }

        iCount = 0;
        while(iCount < ADM_SYSI4HOST_POWER_OFF_TIMEOUT)
        {
            if (DTRUE == g_bGetPowerOffResult)
            {
                iErrorCode = g_iPowerOffResult;
                g_bGetPowerOffResult = DFALSE;
                break;
            }
            sleep(1);
            iCount++;
        }
        if (iCount >= ADM_SYSI4HOST_POWER_OFF_TIMEOUT)
        {
            iErrorCode = -1;
            adm_sysi_setKeylogString(ADM_KEYLOG_ID_035, iErrorCode, NULL);
            log_e("Remote power off auth timeout [err:%d]", iErrorCode);
        }

		g_bPowerOffOptNow = DFALSE;
    }

    *pErrorCode = iErrorCode;

    return iRet;
}

