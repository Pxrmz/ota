//
// Created by kong
//

#ifndef SCM_SC_ENUM_H
#define SCM_SC_ENUM_H

/* Key Algorithm */
typedef enum  {
    SC_NONE           = 0x00,
    SC_SM2            = 0x10,
    SC_ED25519        = 0x11,
    SC_ECC_256V1      = 0x12,
    SC_RSA_2048       = 0x20,
    SC_RSA_3072       = 0x21,
    SC_RSA_4096       = 0x22,
} SC_KeyAlgorithm;


typedef enum  {
    SC_NOHASH         = 0x00,
    SC_SHA1           = 0x01,
    SC_SHA256         = 0x02,
    SC_SHA512         = 0x03,
    SC_SHA1_RSA_PSS   = 0x04,
    SC_SHA256_RSA_PSS = 0x05,
    SC_SHA512_RSA_PSS = 0x06,
    SC_SM3            = 0x13,
} SC_HashAlgorithm;

/* Symm Algorithm */
typedef enum {
    SC_AES128_ECB     = 0x00,
    SC_AES128_CBC     = 0x01,
    SC_AES128_CTR     = 0x02,
    SC_AES128_OFB     = 0x03,
    SC_AES128_CFB     = 0x04,
    SC_AES128_GCM     = 0x05,

    SC_AES256_ECB     = 0x10,
    SC_AES256_CBC     = 0x11,
    SC_AES256_CTR     = 0x12,
    SC_AES256_OFB     = 0x13,
    SC_AES256_CFB     = 0x14,
    SC_AES256_GCM     = 0x15,

    SC_SM4_ECB        = 0x20,
    SC_SM4_CBC        = 0x21,
    SC_SM4_CTR        = 0x22,
    SC_SM4_OFB        = 0x23,
    SC_SM4_CFB        = 0x24,
    SC_SM4_GCM        = 0x25,
} SC_SymmAlgorithm;

typedef enum {
    SC_Client_Vehicle    = 0x01,   /* 车端 (TBOX, IVI, ADCU, CDCU) */
    SC_Client_Phone      = 0x02,   /* 手机端 (Android, iOS) */
    SC_Client_Diagnostic = 0x03,   /* 诊断设备 (诊断设备, 产线电检仪, 售后诊断设备) */
    SC_Client_Platform   = 0x04,   /* 平台 (数字钥匙平台, APP 后台, OTA 平台 等) */
} SC_ClientType;

#endif //SCM_SC_ENUM_H