/*
 * Copyright (C) kong
 */

#ifndef SCM_SC_ERROR_H
#define SCM_SC_ERROR_H

typedef enum  {
    SC_SUCCESS                       =    0x00000000,
    SC_ERR                           =    0x00000001,

    SC_ERR_INVALID_PARAMETER         =    0x10000002,
    SC_ERR_MEMORY                    =    0x10000003,
    SC_ERR_BUFFER_TOO_SMALL          =    0x10000004,
    SC_ERR_NOT_INIT                  =    0x10000005,
    SC_ERR_CONFIG                    =    0x10000006,
    SC_ERR_PIN_VFY                   =    0x10000007,

    SC_ERR_FOLDER_CREATE             =    0x20000000,
    SC_ERR_FILE_MEMORY               =    0x20000001,
    SC_ERR_FILE_INVALID              =    0x20000002,
    SC_ERR_FILE_OPEN                 =    0x20000003,
    SC_ERR_FILE_SIZE                 =    0x20000004,
    SC_ERR_FILE_WRITE                =    0x20000005,
    SC_ERR_FILE_READ                 =    0x20000006,
    SC_ERR_FILE_NOT_EXIST            =    0x20000007,
    SC_ERR_DIR_OPEN                  =    0x20000008,
    SC_ERR_FILE_SYNC                 =    0x20000009,
    SC_ERR_FILE_HASH                 =    0x20000010,
    SC_ERR_FILE_LOCK                 =    0x20000011,

    SC_ERR_KEY_GEN                   =    0x30000000,
    SC_ERR_KEY_SET_PARAMETER         =    0x30000001,
    SC_ERR_KEY_COVER                 =    0x30000002,
    SC_ERR_UNSUPPORT_ALG             =    0x30000010,
    SC_ERR_UNSUPPORT_HASH_ALG        =    0x30000011,
    SC_ERR_UNSUPPORT_METHOD          =    0x30000012,
    SC_ERR_UNSUPPORT_KEY_SIZE        =    0x30000013,
    SC_ERR_DIGEST                    =    0x30000020,
    SC_ERR_HMAC                      =    0x30000021,
    SC_ERR_CMAC                      =    0x30000022,
    SC_ERR_ENCRYPT_INIT              =    0x30000030,
    SC_ERR_ENCRYPT_SET_PARAMETER     =    0x30000031,
    SC_ERR_ENCRYPT                   =    0x30000032,
    SC_ERR_DECRYPT_INIT              =    0x30000033,
    SC_ERR_DECRYPT_SET_PARAMETER     =    0x30000034,
    SC_ERR_DECRYPT                   =    0x30000035,
    SC_ERR_SIGN                      =    0x30000040,
    SC_ERR_VERIFY_SIGN               =    0x30000041,
    SC_ERR_PRI_PADDING               =    0x30000042,

    SC_ERR_CERT_NOT_EXIST            =    0x40000000,
    SC_ERR_CERT_CHAIN_NOT_EXIST      =    0x40000001,
    SC_ERR_CERT_VERIFY_ERR           =    0x40000002,
    SC_ERR_CERT_FORMAT               =    0x40000003,
    SC_ERR_CERT_PARSE                =    0x40000004,
    SC_ERR_CERT_CHAIN_PARSE          =    0x40000005,
    SC_ERR_CERT_GET_INFO             =    0x40000006,
    SC_ERR_CERT_GET_ISSUER           =    0x40000007,
    SC_ERR_CERT_EXPIRATION           =    0x40000008,
    SC_ERR_CERT_REVOKE               =    0x40000009,
    SC_ERR_CSR_SET_VERSION           =    0x40000010,
    SC_ERR_CSR_GET_SUBJECT           =    0x40000011,
    SC_ERR_CSR_SET_PKEY              =    0x40000012,
    SC_ERR_CSR_GET_REQ               =    0x40000013,
    SC_ERR_CERT_CRL_NOT_FOUND        =    0x40000020,
    SC_ERR_CERT_REQUEST              =    0x40000021,
    SC_ERR_CERT_RESPONSE             =    0x40000022,
    SC_ERR_CERT_VFY_SIGN             =    0x40000023,
    SC_ERR_CERT_RESP_STATUS_ERR      =    0x40000024,

    SC_ERR_ENGINE_LOAD_ERR           =    0x5D000000,
    SC_ERR_ENGINE_DEV_NAME_SMALL     =    0x5D000001,
    SC_ERR_ENGINE_DEV_NOT_FOUND      =    0x5D000002,

    SC_ERR_CONFIG_SE_LOAD            =    0x6D000000,
    SC_ERR_CONFIG_FMT                =    0x6D000001,
    SC_ERR_OVER_MAX_KEY              =    0x6D000002,
    SC_ERR_CONFIG_NOT_FOUND          =    0x6D000003,
    SC_ERR_SE_UNSUPPORT              =    0x6D000004,
    SC_ERR_SE_NOT_FOUND              =    0x6D000005,
    SC_ERR_SE_LOAD_ALG               =    0x6D000006,
    SC_ERR_SE_NOT_OPEN               =    0x6D000007,

    SC_ERR_PARSE_VALUE               =    0x7D000000,
    SC_ERR_CONNECT                   =    0x7D000001,
    SC_ERR_CONFIG_CERT               =    0x7D000002,
    SC_ERR_CERT_REQ                  =    0x7D000003,
    SC_ERR_RES_SIGN                  =    0x7D000004,
    SC_ERR_RES_GETCA                 =    0x7D000005,
    SC_ERR_SERVER_PKI_STATUS         =    0x7D000006,
    SC_ERR_SERVER_NET_ERR            =    0x7D000007,
    SC_ERR_RESPONSE                  =    0x7D000008,
    SC_ERR_SERVER_CERT_NOT_MATCH     =    0x7D000009,

} SC_RES, SC_RESULT;

#endif //SCM_SC_ERROR_H
