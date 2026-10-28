/*
 * Copyright (C) kong
 */

#ifndef SCM_SC_PKI_H
#define SCM_SC_PKI_H

#include "SC_Define.h"
#include "SC_Enum.h"
#include "SC_Error.h"

#ifdef _WIN32
#define SC_EXPORT __declspec(dllexport)
#else
#define SC_EXPORT __attribute__ ((visibility ("default")))
#endif


typedef void *SC_HANDLE;

#ifdef __cplusplus
extern  "C" {
#endif

/**
* 证书请求绑定固定 IP
* @param dnsIPs             [IN] 指定DNS解析域名，host[:port][,host[:port]]...
* @param bindIP             [IN] 绑定的IP地址或网卡名
* @param bindPort           [IN] 绑定端口
* @param ttl                [IN] TTL
* @return O success, other error.
*/
SC_EXPORT SC_RESULT
IW_InitServiceSetNetConfig(
        const char *dnsIPs,
        const char *bindIP,
        unsigned short bindPort,
        int ttl);

/**
* 自定义日志路径，若不设置则默认与证书存储在相同位置
* @param logPath        [IN] 日志存储路径
* @return O success, other error.
*/
SC_EXPORT SC_RESULT
IW_LogInit(const char *logPath);

/**
 * 获取 SDK 版本号
 * @return 版本号
 */

SC_EXPORT char *
IW_GetVersion(void);

/**
 * 初始化 PKI 服务
 *
 * @param hHandle            [IN]  句柄，首次传 NULL 即可
 * @param path               [IN]  数据存储路径
 * @param baseUrl            [IN]  网络请求路径
 * @param clientType         [IN]  客户端类型 1-车端（Tbox, IVI, ADCU ,）
 *                                 1-车端(如：TBox, ivi, adcu, cdcu 等)
 *                                 2-手机端
 *                                 3-诊断设备（产线电检仪，售后诊断设备）
 *                                 4-平台（如数字钥匙平台，App 后台，OTA 平台等）
 * @param clientId           [IN]  设备硬件标识
 *                                 1-终端为车辆时，使用 VIN 信息。
 *                                 2-终端为手机时，使用手机唯一标识。
 *                                 3-终端为诊断设备时，使用诊断设备ID。
 *                                 4-终端为平台时，CA 业务平台分配的平台 ID
 * @param certChainFlag      [IN] 证书链标记，用于下载不同证书链
 * @return 0 success, other error.
 */

SC_EXPORT SC_RESULT
IW_InitService(
        SC_HANDLE *hHandle,
        const char *path,
        const char *baseUrl,
        SC_ClientType clientType,
        const char *clientId,
        long certChainFlag);

/**
 * 初始化 PKI 服务 (透传模式,给诊断仪设备,手机设备定制的接口,允许自定义http请求头)
 *
 * @param hHandle            [IN]  句柄，首次传 NULL 即可
 * @param path               [IN]  数据存储路径
 * @param baseUrl            [IN]  网络请求路径
 * @param clientType         [IN]  客户端类型 1-车端（Tbox, IVI, ADCU ,）
 *                                 1-车端(如：TBox, ivi, adcu, cdcu 等)
 *                                 2-手机端
 *                                 3-诊断设备（产线电检仪，售后诊断设备）
 *                                 4-平台（如数字钥匙平台，App 后台，OTA 平台等）
 * @param clientId           [IN]  设备硬件标识
 *                                 1-终端为车辆时，使用 VIN 信息。
 *                                 2-终端为手机时，使用手机唯一标识。
 *                                 3-终端为诊断设备时，使用诊断设备ID。
 *                                 4-终端为平台时，CA 业务平台分配的平台 ID
 * @param certChainFlag      [IN] 证书链标记，用于下载不同证书链
 * @param headers            [IN] 请求头字符串数组
 * @param header_size        [IN] 请求头数组长度
 * @return 0 success, other error.
 */
SC_EXPORT SC_RESULT
IW_InitService_Ex(
        SC_HANDLE *hHandle,
        const char *path,
        const char *baseUrl,
        SC_ClientType clientType,
        const char *clientId,
        long certChainFlag,
        const char **headers,
        int header_size);

/**
 * 初始化 PKI 服务 (只读模式,可以用于多进程的场景下,非证书管理进程初始化使用)
 * @param hHandle   [IN]  句柄
 * @param path      [IN]  数据存储路径
 * @return 0 success, other error.
 */
SC_EXPORT SC_RESULT
IW_InitServiceReadOnly(
        SC_HANDLE *hHandle,
        const char *path);

/**
 * 检查证书是否存在,（通过IW_GetCertificateByUserID下载的证书，通常是OTA验签证书）
 *
 * @param hHandle            [IN] 句柄
 * @param userID             [IN] 标识信息
 * @return 1 exist, 0 non exist
 */

SC_EXPORT SC_RESULT
IW_CheckCertificateExist(
        SC_HANDLE hHandle,
        const char *userID);

/**
 * 检查证书公钥与私钥是否匹配,（通过IW_ApplyCertificate申请的证书，通常是SSL客户端证书）
 *
 * @param hHandle            [IN] 句柄
 * @param userID             [IN] 标识信息
 * @param pin                [IN] PIN 信息 6 ~ 16 (字母、数字)
 * @return 1 available, 0 not available
 */
SC_EXPORT SC_RESULT
IW_CheckCertificateAvailable(
        SC_HANDLE hHandle,
        const char *userID,
        const char *pin);

/**
 * 申请证书
 * @param hHandle              [IN] 句柄
 * @param userID               [IN] 标识信息
 * @param pin                  [IN] PIN 信息 6 ~ 16 (字母、数字)
 * @param algorithm            [IN] 算法类型
 * @param csrInfo              [IN] 证书请求信息
 * @return O success, other error.
 */

SC_EXPORT SC_RESULT
IW_ApplyCertificate(
        SC_HANDLE hHandle,
        const char *userID,
        const char *pin,
        SC_KeyAlgorithm algorithm,
        SC_CSRInformation csrInfo);

/**
 * 生成密钥对
 * @param hHandle              [IN] 句柄
 * @param userID               [IN] 标识信息
 * @param pin                  [IN] PIN 信息 6 ~ 16 (字母、数字)
 * @param algorithm            [IN] 算法类型
 * @param publicKey            [OUT] 公钥
 * @param publicKeyLen         [OUT] 公钥长度
 * @return O success, other error.
 */

SC_EXPORT SC_RESULT
IW_GenerateKeyPair(SC_HANDLE hHandle,
                   const char *userID,
                   const char *pin,
                   SC_KeyAlgorithm algorithm,
                   unsigned char *publicKey, unsigned int *publicKeyLen);

/**
 * 导出公钥
 * @param hHandle              [IN] 句柄
 * @param userID               [IN] 标识信息
 * @param pin                  [IN] PIN 信息 6 ~ 16 (字母、数字)
 * @param algorithm            [IN] 算法类型
 * @param publicKey            [OUT] 公钥
 * @param publicKeyLen         [OUT] 公钥长度
 * @return O success, other error.
 */

SC_EXPORT SC_RESULT
IW_ExportPublicKey(SC_HANDLE hHandle,
                   const char *userID,
                   unsigned char *publicKey, unsigned int *publicKeyLen);

/**
 * 通过用户标识从平台获取证书
 * @param hHandle              [IN] 句柄
 * @param userID               [IN] 标识信息
 * @return 0 success, other error.
 */

SC_EXPORT SC_RESULT
IW_GetCertificateByUserID(
        SC_HANDLE hHandle,
        const char *userID);

/**
 * 删除证书
 * @param hHandle              [IN] 句柄
 * @param userID               [IN] 标识信息，删除的证书时标识同 ApplyCert 中的标识信息。
 */

SC_EXPORT SC_RESULT
IW_DeleteCertificate(
        SC_HANDLE handle,
        const char *userID);

/**
 * 导入证书链
 * @param hHandle              [IN] 句柄
 * @param userID               [IN] 标识信息
 * @param certificateChain     [IN] 证书链
 * @param certificateChainLen  [IN] 证书链长度
 * @return O success, other error.
 */

SC_EXPORT SC_RESULT
IW_ImportCertificateChain(
        SC_HANDLE hHandle,
        unsigned char *certificateChain,
        unsigned int certificateChainLen);

/**
 * 导出证书链
 * @param hHandle              [IN] 句柄
 * @param userID               [IN] 标识信息
 * @param certificateChain     [OUT] 证书链
 * @param certificateChainLen  [OUT] 证书链长度
 * @return O success, other error.
 */

SC_EXPORT SC_RESULT
IW_ExportCertificateChain(
        SC_HANDLE hHandle,
        unsigned char *certificateChain,
        unsigned int *certificateChainLen);

/**
 * 删除证书链
 * @param hHandle              [IN] 句柄
 */

SC_EXPORT SC_RESULT
IW_DeleteCertificateChain(SC_HANDLE hHandle);

/**
 * 导入证书
 * @param hHandle              [IN] 句柄
 * @param userID               [IN] 标识信息
 * @param certificate          [IN] 证书
 * @param certificateLen       [IN] 证书长度
 * @return O success, other error.
 */

SC_EXPORT SC_RESULT
IW_ImportCertificate(
        SC_HANDLE hHandle,
        const char *userID,
        unsigned char *certificate,
        unsigned int certificateLen);

/**
 * 导出证书
 * @param hHandle              [IN] 句柄
 * @param userID               [IN] 标识信息
 * @param certificate          [OUT] 证书
 * @param certificateLen       [OUT] 证书长度 需为预分配接收 certificate 长度
 * @return O success, other error.
 */

SC_EXPORT SC_RESULT
IW_ExportCertificate(
        SC_HANDLE hHandle,
        const char *userID,
        unsigned char *certificate,
        unsigned int *certificateLen);

/**
 * 哈希算法
 * @param hashAlgorithm        [IN]  哈希 Algorithm
 * @param inData               [IN]  待加密数据
 * @param inDataLen            [IN]  待加密数据长度
 * @param outData              [OUT] 密文数据
 * @param outDataLen           [OUT] 密文数据长度
 * @return O success, other error.
 */

SC_EXPORT SC_RESULT
IW_HashMessage(
        SC_HashAlgorithm hashAlgorithm,
        unsigned char *inData,
        unsigned int   inDataLen,
        unsigned char *outData,
        unsigned int  *outDataLen);

/**
 * 生成对称密钥
 * @param pucKey               [IN/OUT]  对称密钥
 * @param uiKeyLen             [IN/OUT]  对称密钥长度
 */
SC_EXPORT SC_RESULT
IW_GenerateSymmKey(
        unsigned char *pucKey,
        unsigned int   uiKeyLen);

/**
 * 创建对称算法密钥句柄
 * @param hKeyHandle           [IN]  对称算法密钥句柄
 * @param pucKey               [IN]  对称密钥
 * @param uiKeyLen             [IN]  对称密钥长度
 * @param pucIV                [IN]  初始向量，ECB 模式此参数忽略
 * @param uiIVLen              [IN]  初始向量长度
 * @param uiEncOrDec           [IN]  0 解密，1 加密
 * @param uiCryptoAlgID        [IN]  密钥算法 ID,
 * @return O success, other error.
 */
#define NO_ADCC_AAD    1
SC_EXPORT SC_RESULT
IW_CreateSymmKeyHandle(
        void **hKeyHandle,
        unsigned char *pucKey,
        unsigned int uiKeyLen,
        unsigned char *pucIV,
        unsigned int uiIVLen,
#if NO_ADCC_AAD
        unsigned char *pucAdd,
        unsigned int uiAddLen,
#endif
        unsigned int uiEncOrDec,
        unsigned int uiCryptoAlgID);

/**
 * 销毁对称算法句柄
 * @param hKeyHandle           [IN]  对称 key 句柄
 * @return O success, other error.
 */
SC_EXPORT SC_RESULT
IW_DestroySymmKeyHandle(
        void *hKeyHandle);

/**
 * 单块加密运算
 * @param hKeyHandle           [IN]  对称算法密钥句柄
 * @param pucInData            [IN]  输入数据
 * @param uiInDataLen          [IN]  输入数据长度
 * @param pucOutData           [OUT] 输出数据
 * @param puiOutDataLen        [OUT] 输出数据长度
 * @return O success, other error.
 */
SC_EXPORT SC_RESULT
IW_SymmEncrypt(
        void *hKeyHandle,
        unsigned char *pucInData,
        unsigned int   uiInDataLen,
        unsigned char *pucOutData,
        unsigned int  *puiOutDataLen);

/**
 * 多块加密运算
 * @param hKeyHandle           [IN]  对称算法密钥句柄
 * @param pucInData            [IN]  输入数据
 * @param uiInDataLen          [IN]  输入数据长度
 * @param pucOutData           [OUT] 输出数据
 * @param puiOutDataLen        [OUT] 输出数据长度
 * @return O success, other error.
 */
SC_EXPORT SC_RESULT
IW_SymmEncryptUpdate(
        void *hKeyHandle,
        unsigned char * pucInData,
        unsigned int uiInDataLen,
        unsigned char *pucOutData,
        unsigned int *puiOutDataLen);

/**
 * 结束多块加密运算
 * @param hKeyHandle           [IN]  对称算法密钥句柄
 * @param pucOutData           [OUT] 输出数据
 * @param puiOutDataLen        [OUT] 输出数据长度
 * @return O success, other error.
 */
SC_EXPORT SC_RESULT
IW_SymmEncryptFinal(
        void * hKeyHandle,
        unsigned char * pucOutData,
        unsigned int * puiOutDataLen);

/**
 *  获取 TAG
 * @param pucOutTag           [OUT] 输出 tag
 * @param uiInTagLen          [OUT] 输出 tag 长度
 * @return O success, other error.
 */
SC_EXPORT SC_RESULT
IW_SymmEncryptGetTag(
        void *hKeyHandle,
        unsigned char *pucOutTag,
        unsigned int   uiInTagLen);


/**
 * 单块解密运算
 * @param hKeyHandle           [IN]  对称算法密钥句柄
 * @param pucInData            [IN]  输入数据
 * @param uiInDataLen          [IN]  输入数据长度
 * @param pucOutData           [OUT] 输出数据
 * @param puiOutDataLen        [OUT] 输出数据长度
 * @return O success, other error.
 */
SC_EXPORT SC_RESULT
IW_SymmDecrypt(
        void *hKeyHandle,
        unsigned char *pucInData,
        unsigned int   uiInDataLen,
        unsigned char *pucOutData,
        unsigned int  *puiOutDataLen);

/**
 * 多块解密运算
 * @param hKeyHandle           [IN]  对称算法密钥句柄
 * @param pucInData            [IN]  输入数据
 * @param uiInDataLen          [IN]  输入数据长度
 * @param pucOutData           [OUT] 输出数据
 * @param puiOutDataLen        [OUT] 输出数据长度
 * @return O success, other error.
 */
SC_EXPORT SC_RESULT
IW_SymmDecryptUpdate(
        void *hKeyHandle,
        unsigned char * pucInData,
        unsigned int uiInDataLen,
        unsigned char *pucOutData,
        unsigned int *puiOutDataLen);

/**
 * 设置 TAG
 * @param pucOutTag           [IN] 设置 TAG
 * @param uiInTagLen          [IN] 设置 tag 长度
 * @return O success, other error.
 */
SC_EXPORT SC_RESULT
IW_SymmDecryptSetTag(
        void *hKeyHandle,
        unsigned char *pucInTag,
        unsigned int   uiInTagLen);

/**
 * 结束解密运算
 * @param hKeyHandle           [IN]  对称算法密钥句柄
 * @param pucOutData           [OUT] 输出数据
 * @param puiOutDataLen        [OUT] 输出数据长度
 * @return O success, other error.
 */
SC_EXPORT SC_RESULT
IW_SymmDecryptFinal(
        void * hKeyHandle,
        unsigned char * pucOutData,
        unsigned int * puiOutDataLen);

/**
 * HMac 算法
 * @param hashAlgorithm        [IN]  哈希 Algorithm
 * @param keyData              [IN]  密钥
 * @param keyDataLen           [IN]  密钥长度
 * @param inData               [IN]  待加密数据
 * @param inDataLen            [IN]  待加密数据长度
 * @param outData              [OUT] 密文数据
 * @param outDataLen           [OUT] 密文数据长度
 * @return 0 success, other error.
 */

SC_EXPORT SC_RESULT
IW_HmacMessage(
        SC_HashAlgorithm hashAlgorithm,
        unsigned char  *keyData,
        unsigned int    keyDataLen,
        unsigned char  *inData,
        unsigned int    inDataLen,
        unsigned char  *outData,
        unsigned int   *outDataLen);

/**
 * HKDF 算法
 * @param hashAlgorithm        [IN]  哈希 Algorithm
 * @param inData               [IN]  待计算数据
 * @param inDataLen            [IN]  待计算数据长度
 * @param ikmData              [IN]  密钥
 * @param ikmDataLen           [IN]  密钥长度
 * @param saltData             [IN]  盐值
 * @param saltDataLen          [IN]  盐值长度
 * @param outData              [OUT] 密文数据
 * @param outDataLen           [OUT] 密文数据长度
 * @return 0 success, other error.
 */
SC_EXPORT SC_RESULT
IW_HkdfMessage(
        SC_HashAlgorithm hashAlgorithm,
        unsigned char  *inData,
        unsigned int    inDataLen,
        unsigned char  *ikmData,
        unsigned int    ikmDataLen,
        unsigned char  *saltData,
        unsigned int    saltDataLen,
        unsigned char  *outData,
        unsigned int    outDataLen);


/**
 * 生成临时密钥
 * @param phAgreementHandle            [IN/OUT]  临时密钥句柄
 * @param inData                       [IN]  待计算数据
 * @param pucSponsorTmpPublicKey       [OUT] 临时密钥
 * @param puiSponsorTmpPublicKeyLen    [OUT] 临时密钥长度
 * @return 0 success, other error.
 */
SC_EXPORT SC_RESULT
IW_GenerateAgreementDataWithEC(
        void ** phAgreementHandle,
        unsigned char * pucSponsorTmpPublicKey,
        unsigned int  * puiSponsorTmpPublicKeyLen);

/**
 * 计算会话密钥
 * @param phAgreementHandle            [IN]  临时密钥句柄
 * @param pucResponseTmpPublicKey      [IN]  对端临时公钥
 * @param puiResponseTmpPublicKeyLen   [IN]  对端临时公钥长度
 * @param pucAgreementKey              [OUT] 临时密钥
 * @param puiAgreementKeyLen           [OUT] 临时密钥长度
 * @return 0 success, other error.
 */
SC_EXPORT SC_RESULT
IW_GenerateAgreementKeyWithEC(
        void * phAgreementHandle,
        unsigned char * pucResponseTmpPublicKey,
        unsigned int    puiResponseTmpPublicKeyLen,
        unsigned char * pucAgreementKey,
        unsigned int  * puiAgreementKeyLen);

/**
 * 释放销毁密钥
 * @param phAgreementHandle            [IN/OUT]  临时密钥句柄
 * @return 0 success, other error.
 */
SC_EXPORT SC_RESULT
IW_DestoryAgreementHandle(
        void * phAgreementHandle);

/**
 * 生成随机数方法
 * @param random               [IN/OUT]  输入 Buffer
 * @param len                  [IN]      待加密数据长度
 * @return 0 success, other error.
 */

SC_EXPORT SC_RESULT
IW_GenerateRandom(
        unsigned char  *random,
        unsigned int    len);


/**
 * 计算摘要
 * @param hashAlgorithm        [IN] 哈希算法类型
 * @param in                   [IN] 待哈希数据
 * @param inLen                [IN] 待哈希数据长度
 * @param digest               [OUT] 哈希数据
 * @param digestLen            [OUT] 哈希数据长度
 * @return 0 success, other error.
 */
SC_EXPORT SC_RESULT
IW_Digest(SC_HashAlgorithm hashAlgorithm,
          unsigned char  *in,
          unsigned int    inLen,
          unsigned char  *digest,
          unsigned int   *digestLen);

/**
 * 使用公钥加密数据
 * @param hHandle              [IN]  句柄
 * @param userID               [IN]  标识信息
 * @param inData               [IN]  待加密数据
 * @param inDataLen            [IN]  待加密数据长度
 * @param outData              [OUT] 密文数据
 * @param outDataLen           [OUT] 密文数据长度
 * @return 0 success, other error.
 */

SC_EXPORT SC_RESULT
IW_PublicKeyEncByID(
        SC_HANDLE      hHandle,
        const char    *userID,
        unsigned char *inData,
        unsigned int   inDataLen,
        unsigned char *outData,
        unsigned int  *outDataLen);

/**
 * 使用公钥加密数据
 * @param certificate          [IN]  证书
 * @param certificateLen       [IN]  证书长度
 * @param inData               [IN]  待加密数据
 * @param inDataLen            [IN]  待加密数据长度
 * @param outData              [OUT] 密文数据
 * @param outDataLen           [OUT] 密文数据长度
 * @return 0 success, other error.
 */

SC_EXPORT SC_RESULT
IW_PublicKeyEncByCert(
        unsigned char *certificate,
        unsigned int   certificateLen,
        unsigned char *inData,
        unsigned int   inDataLen,
        unsigned char *outData,
        unsigned int  *outDataLen);

/**
 * 使用公钥加密数据
 * @param publicKey            [IN]  公钥 ( P8 格式公钥 )
 * @param publicKeyLen         [IN]  公钥长度
 * @param inData               [IN]  待加密数据
 * @param inDataLen            [IN]  待加密数据长度
 * @param outData              [OUT] 密文数据
 * @param outDataLen           [OUT] 密文数据长度
 * @return 0 success, other error.
 */
SC_EXPORT SC_RESULT
IW_PublicKeyEnc(
        unsigned char *publicKey,
        unsigned int   publicKeyLen,
        unsigned char *inData,
        unsigned int   inDataLen,
        unsigned char *outData,
        unsigned int  *outDataLen);

/**
 * 使用默认公钥加密数据
 * @param inData               [IN]  待加密数据
 * @param inDataLen            [IN]  待加密数据长度
 * @param outData              [OUT] 密文数据
 * @param outDataLen           [OUT] 密文数据长度
 * @param isPrd                [IN]  1生产环境，0测试环境
 * @return 0 success, other error.
 */
SC_EXPORT SC_RESULT
IW_DefaultPublicKeyEnc(
        unsigned char *inData,
        unsigned int   inDataLen,
        unsigned char *outData,
        unsigned int  *outDataLen,
        int isPrd);

/**
 * 使用私钥解密数据
 * @param hHandle              [IN]  句柄
 * @param userID               [IN]  标识信息
 * @param inData               [IN]  待解密数据
 * @param inDataLen            [IN]  待解密数据长度
 * @param outData              [OUT] 明文数据
 * @param outDataLen           [OUT] 明文数据长度
 * @return 0 success, other error.
 */

SC_EXPORT SC_RESULT
IW_PrivateKeyDecByID(
        SC_HANDLE      hHandle,
        const char    *userID,
        const char    *pin,
        unsigned char *inData,
        unsigned int   inDataLen,
        unsigned char *outData,
        unsigned int  *outDataLen);

/**
 * 签名
 * @param hHandle              [IN]  句柄
 * @param userID               [IN]  标识信息
 * @param hashAlgorithm        [IN]  哈希算法标识 见: SC_HashAlgorithm , ED25519 曲线，哈希参数不生效，算法默认采用 SHA_512.
 * @param inData               [IN]  待解密数据
 * @param inDataLen            [IN]  待解密数据长度
 * @param signData             [OUT] 明文数据
 * @param signDataLen          [OUT] 明文数据长度
 * @return 0 success, other error.
 */

SC_EXPORT SC_RESULT
IW_SignByID(
        SC_HANDLE        hHandle,
        const char      *userID,
        const char      *pin,
        SC_HashAlgorithm hashAlgorithm,
        unsigned char   *inData,
        unsigned int     inDataLen,
        unsigned char   *signData,
        unsigned int    *signDataLen);

/**
 * 验证签名
 * @param hashAlgorithm        [IN]  哈希算法标识 见: SC_HashAlgorithm
 * @param certificate          [IN]  证书
 * @param certificateLen       [IN]  证书长度
 * @param inData               [IN]  待加密数据
 * @param inDataLen            [IN]  待加密数据长度
 * @param signData             [OUT] 密文数据
 * @param signDataLen          [OUT] 密文数据长度
 * @return 0 success, other error.
 */

SC_EXPORT SC_RESULT
IW_VerifySignByCert(
        SC_HashAlgorithm hashAlgorithm,
        unsigned char   *certificate,
        unsigned int     certificateLen,
        unsigned char   *inData,
        unsigned int     inDataLen,
        unsigned char   *signData,
        unsigned int     signDataLen);

/**
 * 验证签名
 * @param hHandle              [IN]  句柄
 * @param userID               [IN]  标识信息
 * @param hashAlgorithm        [IN]  哈希算法标识 见: SC_HashAlgorithm
 * @param inData               [IN]  待加密数据
 * @param inDataLen            [IN]  待加密数据长度
 * @param signData             [IN] 密文数据
 * @param signDataLen          [IN] 密文数据长度
 * @return 0 success, other error.
 */

SC_EXPORT SC_RESULT
IW_VerifySignByID(
        SC_HANDLE        hHandle,
        const char      *userID,
        SC_HashAlgorithm hashAlgorithm,
        unsigned char   *inData,
        unsigned int     inDataLen,
        unsigned char   *signData,
        unsigned int     signDataLen);

/**
 * 验证签名
 * @param hHandle              [IN]  句柄
 * @param userID               [IN]  标识信息
 * @param hashAlgorithm        [IN]  哈希算法标识 见: SC_HashAlgorithm
 * @param filePath             [IN]  文件路径
 * @param signData             [IN]  签名值
 * @param signDataLen          [IN]  签名长度
 * @return 0 success, other error.
 */

SC_EXPORT SC_RESULT
IW_VerifySignByFile(
        SC_HANDLE         hHandle,
        const char       *userID,
        SC_HashAlgorithm  hashAlgorithm,
        const char       *filePath,
        unsigned char    *signData,
        unsigned int      signDataLen);

/**
 * 文件验证签名
 * @param hHandle              [IN] 句柄
 * @param userID               [IN] 标识信息
 * @param filePath             [IN] 文件路径
 * @param hashAlgorithm        [IN] Algorithm
 * @param signature            [IN] 签名
 * @param signatureLen         [IN] 签名长度
 * @return O success, other error.
 */

SC_EXPORT SC_RESULT
IW_VerifyFileSignature(
        SC_HANDLE hHandle, const char *userID,
        const char *filePath, SC_HashAlgorithm hashAlgorithm,
        unsigned char *signature, unsigned int signatureLen);


/* 释放 SDK */

SC_EXPORT SC_RESULT
IW_FreeService(SC_HANDLE hHandle);

#ifdef __cplusplus
}
#endif

#endif //SCM_SC_PKI_H
