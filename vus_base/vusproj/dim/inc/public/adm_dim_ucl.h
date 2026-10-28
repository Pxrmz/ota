/**
 * @file adm_dim_soaUcl.h
 * @brief 智能设备上集成的UCM模块，已服务化的形式为UC-Master提供的相关接口
 * @details 主要由UCM提供接口，用于OTA过程中对零件控制，包括：零件信息获取，升级包传输，零件升级等。
 * @author Mengwei
 * @version v0.1.0
 * @date 2021-05-06
 * @copyright Copyright © 2018 All rights 上海艾拉比智能科技有限公司
 */
#ifndef __ADM_DIM_SOA_UCL_H__
#define __ADM_DIM_SOA_UCL_H__

#ifdef __cplusplus
extern "C" {
#endif

#define EXPORT_FUN __attribute__((visibility("default")))

#include "adm_dim_common.h"
#include "adm_com_parseFileObject.h"
/***************************************************************************Get SDK Version***************************************************************************************/
/**
 * @brief 获取UCM的sdk版本号
 *
 * @param instanceId    [IN]    实例ID
 * @param timeout       [IN]    请求的超时时间
 * @param sdkVersion    [OUT]  SDK 版本号
 * @param errorCode     [OUT]  获取版本号的结果
 */
extern D32S adm_dim_ucl_getSdkVersion(const D16U instanceId, const D16U timeout, DCHAR** sdkVersion, D32S* errorCode);
typedef D32S(*adm_dim_ucl_getSdkVersion_func)(const D16U instanceId, const D16U timeout, DCHAR** sdkVersion, D32S* errorCode);

/***************************************************************************Get SDK Version***************************************************************************************/

/**
 * @brief Identification information of ECU upgrade package decrypt
 *
 */
typedef struct
{
    ADM_DIM_SOA_ECU_IDEN_INFO_T*    pstEcuIdenInfo;         /**!< Identification information of ECU */
    DBOOL   isRollbackFlag;                                     /**!< True: Rollback, False: Not Rollback */
} ADM_DIM_SOA_UCL_DECRYPT_IDEN_INFO_T;

/***************************************************************************Read diagnostic information**************************************************************************/

/**
 * @brief The obtained information structure corresponding to DID and DID.
*/

// TODO: 定义错误
/**
 * @brief Read part information.
 *
 * @param instanceId        [IN]        const D16U  instance identifier
 * @param timeout               [IN]        const D16U  Timeout of sync interface, unit: second
 * @param pstEcuIdenInfo    [IN]    const ADM_DIM_SOA_ECU_IDEN_INFO_T*          Identification information of ECU
 * @param didArrayRequest   [IN]    const ADM_DIM_SOA_DID_DATA_ARRAY_T*              DID and DID value array list request
 * @param errorCode         [OUT]   D32S*                                           The execution result of this request
 * @return D32S             0:successed, Non-0:failed
 */
extern D32S adm_dim_ucl_readPartInformation(const D16U instanceId, const D16U timeout,
        const ADM_DIM_SOA_ECU_IDEN_INFO_T* pstEcuIdenInfo, const ADM_DIM_SOA_DID_DATA_ARRAY_T* didArrayRequest,
        ADM_DIM_SOA_DID_DATA_ARRAY_T** didListResponse, D32S* errorCode);
typedef D32S(*adm_dim_ucl_readPartInformation_func)(const D16U instanceId, const D16U timeout,
        const ADM_DIM_SOA_ECU_IDEN_INFO_T* pstEcuIdenInfo, const ADM_DIM_SOA_DID_DATA_ARRAY_T* didArrayRequest,
        ADM_DIM_SOA_DID_DATA_ARRAY_T** didListResponse, D32S* errorCode);

/**
 * @brief 读取从控SN号
 * 
 * @param[in] instanceId     描述: 从控实例ID\n
 * 范围值: 无符号短整型 \n
 * 单位: N/A \n
 * 
 * @param[in] timeout     描述: 超时时间 \n
 * 范围值: 无符号短整型 \n
 * 单位: N/A \n
 * 
 * @param[in] pstEcuIdenInfo     描述: ecu信息 \n
 * 范围值: 指针\n
 * 单位: N/A \n
 * 
 * @param[out] ppSnInfo     描述: sn号 \n
 * 范围值: 二级指针 \n
 * 单位: N/A \n
 * 
 * @param[out] pErrorCode     描述: 响应码 \n
 * 范围值: 获取sn号的响应码 \n
 * 单位: N/A \n
 * 
 * @return D32S 
 */
D32S adm_dim_ucl_readSnInfo(const D16U instanceId, const D16U timeout,
                            DCHAR** ppSnInfo, D32S* pErrorCode);
typedef D32S(*adm_dim_ucl_readSnInfo_func)(const D16U instanceId, const D16U timeout, 
            DCHAR** ppSnInfo, D32S* pErrorCode);

//并行采集
D32S adm_dim_ucl_parallelCollectEcu(const D16U instanceId,
                                    const D16U timeout,
                                    const ADM_DIM_SOA_ECU_IDEN_INFO_T* pstEcuIdenInfo,
                                    const ADM_DIM_SOA_DID_DATA_ARRAY_T* didArrayRequest,
                                    D32S* errorCode);
typedef D32S(*adm_dim_ucl_parallelCollectEcu_func)(const D16U instanceId,
        const D16U timeout,
        const ADM_DIM_SOA_ECU_IDEN_INFO_T* pstEcuIdenInfo,
        const ADM_DIM_SOA_DID_DATA_ARRAY_T* didArrayRequest,
        D32S* errorCode);


/**
 * @brief Read part information notification.
 *
 * @param pstEcuIdenInfo        [IN]    const ADM_DIM_SOA_ECU_IDEN_INFO_T*  Identification information of ECU
 * @param didListResponse       [OUT]   const ADM_DIM_SOA_DID_DATA_ARRAY_T*          Response structure for reading diagnostic information.
 *
 * @return non
 */
typedef D32S(*ADM_DIM_SOA_UCL_READ_PART_INFO_CALLBACK_F)(const ADM_DIM_SOA_ECU_IDEN_INFO_T* pstEcuIdenInfo,
        const ADM_DIM_SOA_DID_DATA_ARRAY_T* didListResponse);
/***************************************************************************Read diagnostic information**************************************************************************/


/***************************************************************************Do Upgrage*******************************************************************************************/
/**
 * @brief Image information to be flashed.
 *
 */
typedef struct
{
    DCHAR*  imagePath;                              /**!< Absolute path of image */
    DCHAR*  imageHash;                              /**!< Hash(SHA256 sum) of image */
    D64S    imageSize;                              /**!< Size of image, unit:Byte */
    D32S  makePackageType;                    //是否支持拼接 2-支持，其他-不支持
    D32S                fileMetaNum;          //!< 文件元数据个数 ---支持流式拼接增加字段
    ADM_COM_FILEMETA_T *pstFileMetaList;      //!< 文件元数据list ---支持流式拼接增加字段
} ADM_DIM_SOA_UCL_IMAGE_INFO_T;

/**
 * @brief Array of image information to be flashed.
 * @note This information generally needs to be obtained from the decompressed upgrade package configuration file.
 *
 */
typedef struct
{
    ADM_DIM_SOA_UCL_IMAGE_INFO_T** imageInfoArray;     /**!< Array of image information to be flashed */
    D16U    imageInfoArrayLength;                   /**!< Array length of image information to be flashed */
} ADM_DIM_SOA_UCL_IMAGE_INFO_ARRAY_T;

/**
 * @brief All information of the ECU to be flashed.
 *
 */
typedef struct
{
    ADM_DIM_SOA_ECU_IDEN_INFO_T* pstEcuIdenInfo;   /**!< Identification information of ECU */
    DCHAR* srcSoftVersion;                             /**!< The source version of the upgraded part */
    DCHAR* dstSoftVersion;                             /**!< The target version of the upgraded part */
    ADM_DIM_SOA_UCL_IMAGE_INFO_ARRAY_T* imageInfoList; /**!< Array of image information to be flashed */
} ADM_DIM_SOA_UCL_UPGRADE_INFO_T;

typedef struct
{
    ADM_DIM_SOA_UCL_UPGRADE_INFO_T**    upgradeInfoArray;   /**!< Array of all ECU information to be flashed */
    D16U    upgradeInfoArrayLength;                     /**!< Array length of all ECU information to be flashed */
} ADM_DIM_SOA_UCL_UPGRADE_INFO_ARRAY_T;

extern D32S adm_dim_ucl_requestDoUpgrade_icc(const D16U instanceId, const D16U timeout,
        const ADM_DIM_SOA_UCL_UPGRADE_INFO_ARRAY_T* upgradeInfoList, D32S installType, D32S bIsRollback, D32S* errorCode);
typedef D32S(*adm_dim_ucl_requestDoUpgrade_icc_func)(const D16U instanceId, const D16U timeout,
        const ADM_DIM_SOA_UCL_UPGRADE_INFO_ARRAY_T* upgradeInfoList, D32S installType, D32S bIsRollback, D32S* errorCode);

extern D32S adm_dim_ucl_requestSetChangeSlot_icc(const D16U instanceId, const D16U timeout, D32S* errorCode);
typedef D32S(*adm_dim_ucl_requestSetChangeSlot_icc_func)(const D16U instanceId, const D16U timeout, D32S* errorCode);

extern D32S adm_dim_ucl_requestUpdateAndReboot(const D16U instanceId, const D16U timeout, D32S* errorCode);
typedef D32S(*adm_dim_ucl_requestUpdateAndReboot_func)(const D16U instanceId, const D16U timeout, D32S* errorCode);

/**
 * @brief Trigger flashing of multiple or one ECU.
 * @warning This interface needs to support multi-threaded or parallel calls.
 *
 * @param instanceId        [IN]        const D16U  instance identifier
 * @param timeout               [IN]        const D16U  Timeout of sync interface, unit: second
 * @param upgradeInfoList   [IN]    const ADM_DIM_SOA_UCL_UPGRADE_INFO_ARRAY_T*      All information of the ECU to be flashed
 * @param errorCode         [OUT]   D32S*   The execution result of this request
 * @return D32S             0:successed, Non-0:failed
 */
extern D32S adm_dim_ucl_requestDoUpgrade(const D16U instanceId, const D16U timeout,
        const ADM_DIM_SOA_UCL_UPGRADE_INFO_ARRAY_T* upgradeInfoList, DBOOL bIsControlActive, 
        D32S* errorCode);
typedef D32S(*adm_dim_ucl_requestDoUpgrade_func)(const D16U instanceId, const D16U timeout,
        const ADM_DIM_SOA_UCL_UPGRADE_INFO_ARRAY_T* upgradeInfoList, DBOOL bIsControlActive, 
        D32S* errorCode);

/**
 * @brief The callback function is used to process the progress information thrown by the UCM do upgrade module.
 *
 * @param pstEcuIdenInfo      [IN]   const ADM_DIM_SOA_ECU_IDEN_INFO_T*  Identification information of ECU.
 * @param progress            [IN]   const D16U     Progress of part do upgrade.
 * @param priv                              [IN/OUT]    void*                               Reserved parameters
 * @return  D32S                            0:successed, Non-0:failed
 */
typedef D32S(*ADM_DIM_SOA_UCL_UPGRADE_PROGRESS_NOTIFY_CALLBACK_F)(const ADM_DIM_SOA_ECU_IDEN_INFO_T* pstEcuIdenInfo,
        const D16U progress, void* priv);

/**
 * @brief Used to actively query the flashing progress of an ECU
 *
 * @param instanceId        [IN]        const D16U  instance identifier
 * @param timeout               [IN]        const D16U  Timeout of sync interface, unit: second
 * @param pstEcuIdenInfo      [IN]   const ADM_DIM_SOA_ECU_IDEN_INFO_T*  Identification information of ECU.
 * @param progress            [IN]   const D16U     Progress of part do upgrade.
 * @param errorCode             [OUT] D32S*     Error code
 * @return D32S                             0:successed, Non-0:failed
 */
extern D32S adm_dim_ucl_queryUpgradeProgress(const D16U instanceId, const D16U timeout,
        const ADM_DIM_SOA_ECU_IDEN_INFO_T* pstEcuIdenInfo, D16U* progress, D32S* errorCode);
typedef D32S(*adm_dim_ucl_queryUpgradeProgress_func)(const D16U instanceId, const D16U timeout,
        const ADM_DIM_SOA_ECU_IDEN_INFO_T* pstEcuIdenInfo, D16U* progress, D32S* errorCode);


/**
 * @brief The result structure definition of the remote download upgrade package
 *
 */
typedef struct
{
    D32S result;                                                /**!< The result of the remote module downloading the upgrade package */
    DCHAR*  expectVal;                                          /**!< Description of the reason for the failure, expected value */
    DCHAR*  realVal;                                            /**!< Description of the reason for the failure, expected value */
} ADM_DIM_SOA_UCL_UPGRADE_RESULT_INFO_T;

/**
 * @brief The callback function is used to process the result information thrown by the UCM do upgrade module.
 *
 * @param pstEcuIdenInfo      [IN]   const ADM_DIM_SOA_ECU_IDEN_INFO_T*  Identification information of ECU.
 * @param pstUpgradeResultInfo          [IN]   const ADM_DIM_SOA_UCL_UPGRADE_RESULT_INFO_T*  The result relfash information of the ECU upgrade thrown by the sysi4vdi module.
 * @param priv                              [IN/OUT]    void*                               Reserved parameters
 * @return  D32S                            0:successed, Non-0:failed
 */
typedef D32S(*ADM_DIM_SOA_UCL_UPGRADE_RESULT_NOTIFY_CALLBACK_F)(const ADM_DIM_SOA_ECU_IDEN_INFO_T* pstIdenInfo,
        const ADM_DIM_SOA_UCL_UPGRADE_RESULT_INFO_T* pstUpgradeResultInfo, void* priv);

typedef D32S(*ADM_DIM_SOA_UCL_UPGRADE_ICC_PROGRESS_NOTIFY_CALLBACK_F)(D32S installType, D32S progress, void* priv);
typedef D32S(*ADM_DIM_SOA_UCL_UPGRADE_ICC_RESULT_NOTIFY_CALLBACK_F)(D32S installType, D32S result, void* priv);
typedef D32S(*ADM_DIM_SOA_UCL_UPGRADE_ICC_REBOOT_RESULT_NOTIFY_CALLBACK_F)(D32S installType, D32S result, void* priv);

/**
 * @brief Used to actively query the flashing result of an ECU
 *
 * @param instanceId        [IN]        const D16U  instance identifier
 * @param timeout               [IN]        const D16U  Timeout of sync interface, unit: second
 * @param pstEcuIdenInfo                    [IN]    const ADM_DIM_SOA_ECU_IDEN_INFO_T*  Structure used to uniquely identify an ECU.
 * @param pstUpgradeResultInfo          [OUT]   ADM_DIM_SOA_UCL_UPGRADE_RESULT_INFO_T** The result relfash information of the ECU upgrade thrown by the sysi4vdi module.
 * @param errorCode             [OUT] D32S*     Error code
 * @return D32S                             0:successed, Non-0:failed
 */
extern D32S adm_dim_ucl_queryUpgradeResult(const D16U instanceId, const D16U timeout,
        const ADM_DIM_SOA_ECU_IDEN_INFO_T* pstEcuIdenInfo,  ADM_DIM_SOA_UCL_UPGRADE_RESULT_INFO_T** pstUpgradeResultInfo,
        D32S* errorCode);
typedef D32S(*adm_dim_ucl_queryUpgradeResult_func)(const D16U instanceId, const D16U timeout,
        const ADM_DIM_SOA_ECU_IDEN_INFO_T* pstEcuIdenInfo,  ADM_DIM_SOA_UCL_UPGRADE_RESULT_INFO_T** pstUpgradeResultInfo,
        D32S* errorCode);

/***************************************************************************Do Upgrage*******************************************************************************************/

/***************************************************************************Decrypt package**************************************************************************************/
/**
 * @brief Request the UCM module to trigger the decryption action of the upgrade package.
 *
 * @param instanceId        [IN]        const D16U  instance identifier
 * @param timeout               [IN]        const D16U  Timeout of sync interface, unit: second
 * @param fileID            [IN]        const DCHAR*              ID of the file to be decrypted
 * @param filePath          [IN]        const DCAHR*              Absolute path of decrypted file
 * @param decryptKey        [IN]        const DCHAR*              Key used for decryption
 * @param decryptSegLength  [IN]        const D64S                Segment length for file decryption
 * @param hash              [IN]        const DCHAR*              File HASH value after decryption
 * @param errorCode         [OUT]       D32S*                     The execution result of this request
 * @return D32S             0:successed, Non-0:failed
 */
extern D32S adm_dim_ucl_requestDecryptPackage(const D16U instanceId, const D16U timeout, const DCHAR* fileID,
        const DCHAR* filePath, const DCHAR* decryptKey, const D64S decryptSegLength, const DCHAR* hash, D32S* errorCode);
typedef D32S(*adm_dim_ucl_requestDecryptPackage_func)(const D16U instanceId, const D16U timeout, const DCHAR* fileID,
        const DCHAR* filePath, const DCHAR* decryptKey, const D64S decryptSegLength, const DCHAR* hash, D32S* errorCode);

/**
 * @brief Definition of upgrade package decryption progress structure.
 *
 */
typedef struct
{
    D64S    savedSize;                                          /**!< The size of the downloaded upgrade package, unit:byte. */
    D64S    totalSize;                                          /**!< The total size of the upgrade package being downloaded, unit:byte */
    D32S    speed;                                              /**!< Decrypt speed of upgrade package, unit:byte/s */
    D32S    remainTime;                                         /**!< Estimated remaining time to download, unit:second */
} ADM_DIM_SOA_UCL_DECRYPT_PROGRESS_INFO_T;

/**
 * @brief The callback function is used to process the progress information thrown by the UCM model.
 *
 * @param fileID                        [IN]        const DCHAR*              ID of the file to be decrypted
 * @param pstDecryptProgressInfo        [IN]   const ADM_DIM_SOA_UCL_DECRYPT_PROGRESS_INFO_T*  The progress information structure of the upgrade package decrypted.
 * @param priv                              [IN/OUT]    void*                               Reserved parameters
 * @return  D32S                            0:successed, Non-0:failed
 */
typedef D32S(*ADM_DIM_SOA_UCL_DECRYPT_PROGRESS_NOTIFY_CALLBACK_F)(const DCHAR* fileID,
        const ADM_DIM_SOA_UCL_DECRYPT_PROGRESS_INFO_T* pstDecryptProgressInfo, void* priv);


/**
 * @brief Request to query the decryption progress of the current upgrade package.
 *
 * @param instanceId        [IN]        const D16U  instance identifier
 * @param timeout               [IN]        const D16U  Timeout of sync interface, unit: second
 * @param fileID                        [IN]        const DCHAR*              ID of the file to be decrypted
 * @param pstDecryptProgressInfo        [OUT]        ADM_DIM_SOA_UCL_DECRYPT_PROGRESS_INFO_T**  The progress information structure of the upgrade package decrypted.
 * @param errorCode                     [OUT]       D32S*   error code
 * @return D32S                             0:successed, Non-0:failed
 */
extern D32S adm_dim_ucl_getDecryptPackageProgress(const D16U instanceId, const D16U timeout, const DCHAR* fileID,
        ADM_DIM_SOA_UCL_DECRYPT_PROGRESS_INFO_T** pstDecryptProgressInfo, D32S* errorCode);
typedef D32S(*adm_dim_ucl_getDecryptPackageProgress_func)(const D16U instanceId, const D16U timeout, const DCHAR* fileID,
        ADM_DIM_SOA_UCL_DECRYPT_PROGRESS_INFO_T** pstDecryptProgressInfo, D32S* errorCode);

/**
 * @brief The result structure definition of the decrypt upgrade package
 *
 */
typedef struct
{
    D32S result;                                                /**!< The result of the remote module decrypting the upgrade package */
    DCHAR*  expectVal;                                          /**!< Description of the reason for the failure, expected value */
    DCHAR*  realVal;                                            /**!< Description of the reason for the failure, expected value */
} ADM_DIM_SOA_UCL_DECRYPT_RESULT_INFO_T;

/**
 * @brief The callback function is used to process the decrypted result information sent by the UCM module.
 *
 * @param fileID                        [IN]        const DCHAR*              ID of the file to be decrypted
 * @param storagePath                   [IN]        const DCHAR*              The absolute path where the upgrade package downloaded by the remote module is stored
 * @param pstDecryptResultInfo          [IN]   const ADM_DIM_SOA_UCL_DECRYPT_RESULT_INFO_T*   The result structure definition of the decrypt upgrade package.
 * @param priv                              [IN/OUT]    void*                               Reserved parameters
 * @return  D32S                            0:successed, Non-0:failed
 */
typedef D32S(*ADM_DIM_SOA_UCL_DECRYPT_RESULT_NOTIFY_CALLBACK_F)(const DCHAR* fileID, const DCHAR* storagePath,
        const ADM_DIM_SOA_UCL_DECRYPT_RESULT_INFO_T* pstDecryptResultInfo, void* priv);

/**
 * @brief Request to query the decryption result of the current upgrade package.
 *
 * @param instanceId        [IN]        const D16U  instance identifier
 * @param timeout               [IN]        const D16U  Timeout of sync interface, unit: second
 * @param fileID                        [IN]        const DCHAR*              ID of the file to be decrypted
 * @param storagePath                   [OUT]        DCHAR**              The absolute path where the upgrade package downloaded by the remote module is stored
 * @param pstDecryptResultInfo          [OUT]        ADM_DIM_SOA_UCL_DECRYPT_RESULT_INFO_T**   The result structure definition of the decrypt upgrade package.
 * @param errorCode                     [OUT]       D32S*   error code
 * @return D32S                             0:successed, Non-0:failed
 */
extern D32S adm_dim_ucl_getDecryptPackageResult(const D16U instanceId, const D16U timeout, const DCHAR* fileID,
        DCHAR** storagePath, ADM_DIM_SOA_UCL_DECRYPT_RESULT_INFO_T** pstDecryptResultInfo, D32S* errorCode);
typedef D32S(*adm_dim_ucl_getDecryptPackageResult_func)(const D16U instanceId, const D16U timeout, const DCHAR* fileID,
        DCHAR** storagePath, ADM_DIM_SOA_UCL_DECRYPT_RESULT_INFO_T** pstDecryptResultInfo, D32S* errorCode);

/***************************************************************************Decrypt package**************************************************************************************/

/***************************************************************************Verify hash of package*******************************************************************************/
/**
 * @brief HASH algorithm type definition
 *
 */
typedef enum
{
    ADM_DIM_SOA_UCL_HASH_TYPE_MD5 = 1,      /**!< MD5 */
    ADM_DIM_SOA_UCL_HASH_TYPE_SHA1,         /**!< SHA1 */
    ADM_DIM_SOA_UCL_HASH_TYPE_SHA256,       /**!< SHA256 */
    ADM_DIM_SOA_UCL_HASH_TYPE_SHA512,       /**!< SHA512 */
    ADM_DIM_SOA_UCL_HASH_TYPE_INVALID       /**!< Invalid */
} ADM_DIM_SOA_UCL_HASH_TYPE_E;

/**
 * @brief Request to trigger the HAS value verification of the upgrade package.
 *
 * @param instanceId        [IN]        const D16U  instance identifier
 * @param timeout               [IN]        const D16U  Timeout of sync interface, unit: second
 * @param fileID                        [IN]        const DCHAR*              ID of the file to be verified
 * @param filePath                      [IN]        const DCAHR*              Absolute path of decrypted file
 * @param eHashType                     [IN]        const ADM_DIM_SOA_UCL_HASH_TYPE_E   HASH algorithm type definition
 * @param hash                          [IN]        const DCHAR*               HASH value of file to be verified
 * @param errorCode                     [OUT]       D32S*                     The execution result of this request
 * @return D32S                             0:successed, Non-0:failed
 */
extern D32S adm_dim_ucl_requestVerifyHash(const D16U instanceId, const D16U timeout, const DCHAR* fileID, const DCHAR* filePath,
        const ADM_DIM_SOA_UCL_HASH_TYPE_E eHashType, const DCHAR* hash, D32S* errorCode);
typedef D32S(*adm_dim_ucl_requestVerifyHash_func)(const D16U instanceId, const D16U timeout, const DCHAR* fileID,
        const DCHAR* filePath, const ADM_DIM_SOA_UCL_HASH_TYPE_E eHashType, const DCHAR* hash, D32S* errorCode);

/**
 * @brief Callback function for processing upgrade package HASH value verification
 *
 * @param fileID                        [IN]        const DCHAR*              ID of the file to be verified
 * @param progress                      [IN]        const D16U                Progress value of hash verification (0-100)
 * @param priv                                  [IN/OUT]    void*                               Reserved parameters
 * @return  D32S                                0:successed, Non-0:failed
 *
 */
typedef D32S(*ADM_DIM_SOA_UCL_VERIFY_HASH_PROGRESS_NOTIFY_CALLBACK_F)(const DCHAR* fileID, const D16U progress, void* priv);


/**
 * @brief The result structure definition of the verify upgrade package's hash.
 *
 */
typedef struct
{
    D32S result;                                                /**!< The result of the remote module verify hash of the upgrade package */
    DCHAR*  expectVal;                                          /**!< Description of the reason for the failure, expected value */
    DCHAR*  realVal;                                            /**!< Description of the reason for the failure, expected value */
} ADM_DIM_SOA_UCL_VERIFY_PACKAGE_HASH_RESULT_INFO_T;

/**
 * @brief Callback function for result upgrade package HASH value verification
 *
 * @param fileID                                [IN]        const DCHAR*              ID of the file to be verified
 * @param pstVerifyPackageHashResultInfo        [IN]   const ADM_DIM_SOA_UCL_VERIFY_PACKAGE_HASH_RESULT_INFO_T*      upgrade package HASH verification result.
 * @param priv                                  [IN/OUT]    void*                               Reserved parameters
 * @return  D32S                                0:successed, Non-0:failed
 *
 */
typedef D32S(*ADM_DIM_SOA_UCL_VERIFY_HASH_RESULT_NOTIFY_CALLBACK_F)(const DCHAR* fileID,
        const ADM_DIM_SOA_UCL_VERIFY_PACKAGE_HASH_RESULT_INFO_T* pstVerifyPackageHashResultInfo, void* priv);
/***************************************************************************Verify hash of package*******************************************************************************/

/***************************************************************************Verify the storage space*****************************************************************************/
/**
 * @brief Verify that the storage space is sufficient.
 *
 * @param instanceId        [IN]        const D16U  instance identifier
 * @param timeout               [IN]        const D16U  Timeout of sync interface, unit: second
 * @param needSize          [IN]    const D64S  The size of the required space, unit: byte
 * @param realSize          [OUT]   D64S*       The real space size, unit: byte
 * @param errorCode         [OUT]   D32S*   The execution result of this request
 * @return D32S             0:successed, Non-0:failed
 */
extern D32S adm_dim_ucl_verifySpaceIsEnouth(const D16U instanceId, const D16U timeout, const D64S needSize, D64S* realSize,
        D32S* errorCode);
typedef D32S(*adm_dim_ucl_verifySpaceIsEnouth_func)(const D16U instanceId, const D16U timeout, const D64S needSize,
        D64S* realSize, D32S* errorCode);

/***************************************************************************Verify the storage space*****************************************************************************/

/***************************************************************************APK Self Upgrade*****************************************************************************/
extern D32S adm_dim_ucl_getApkVersion(const D16U instanceId, const D16U timeout,
        const ADM_DIM_SOA_ECU_IDEN_INFO_T* pstEcuIdenInfo, DCHAR** apkVersion, D32S* errorCode);
typedef D32S (*adm_dim_ucl_getApkVersion_func)(const D16U instanceId, const D16U timeout,
        const ADM_DIM_SOA_ECU_IDEN_INFO_T* pstEcuIdenInfo, DCHAR** apkVersion, D32S* errorCode);
extern D32S adm_dim_ucl_doApkUpgrade(const D16U instanceId, const D16U timeout,
        const ADM_DIM_SOA_ECU_IDEN_INFO_T* pstEcuIdenInfo, DCHAR* pApkPackPath, DCHAR* pApkVersion, D32S* errorCode);
typedef D32S (*adm_dim_ucl_doApkUpgrade_func)(const D16U instanceId, const D16U timeout,
        const ADM_DIM_SOA_ECU_IDEN_INFO_T* pstEcuIdenInfo, DCHAR* pApkPackPath, DCHAR* pApkVersion, D32S* errorCode);
extern D32S adm_dim_ucl_queryApkSelfUpgradeProgress(const D16U instanceId, const D16U timeout,
        const ADM_DIM_SOA_ECU_IDEN_INFO_T* pstEcuIdenInfo, D16U* progress, D32S* errorCode);
typedef D32S(*adm_dim_ucl_queryApkSelfUpgradeProgress_func)(const D16U instanceId, const D16U timeout,
        const ADM_DIM_SOA_ECU_IDEN_INFO_T* pstEcuIdenInfo, D16U* progress, D32S* errorCode);
extern D32S adm_dim_ucl_queryApkSelfUpgradeResult(const D16U instanceId, const D16U timeout,
        const ADM_DIM_SOA_ECU_IDEN_INFO_T* pstEcuIdenInfo,  ADM_DIM_SOA_UCL_UPGRADE_RESULT_INFO_T** pstUpgradeResultInfo,
        D32S* errorCode);
typedef D32S(*adm_dim_ucl_queryApkSelfUpgradeResult_func)(const D16U instanceId, const D16U timeout,
        const ADM_DIM_SOA_ECU_IDEN_INFO_T* pstEcuIdenInfo,  ADM_DIM_SOA_UCL_UPGRADE_RESULT_INFO_T** pstUpgradeResultInfo,
        D32S* errorCode);

typedef D32S(*ADM_DIM_SOA_UCL_APK_SELF_UPGRADE_PROGRESS_NOTIFY_CALLBACK_F)(const ADM_DIM_SOA_ECU_IDEN_INFO_T* pstEcuIdenInfo,
        const D16U progress, void* priv);
typedef D32S(*ADM_DIM_SOA_UCL_APK_SELF_UPGRADE_RESULT_NOTIFY_CALLBACK_F)(const ADM_DIM_SOA_ECU_IDEN_INFO_T* pstIdenInfo,
        const ADM_DIM_SOA_UCL_UPGRADE_RESULT_INFO_T* pstUpgradeResultInfo, void* priv);
/***************************************************************************APK Self Upgrade*****************************************************************************/

/******************************** Verify Sign below **********************************/
//摘要算法类型
typedef enum
{
    ADM_DIM_SOA_UCL_ALGORITHM_TYPE_CRC32  = 1,
    ADM_DIM_SOA_UCL_ALGORITHM_TYPE_MD5    = 2,
    ADM_DIM_SOA_UCL_ALGORITHM_TYPE_SHA256 = 3,
    ADM_DIM_SOA_UCL_ALGORITHM_TYPE_SHA512 = 4,
    ADM_DIM_SOA_UCL_ALGORITHM_TYPE_INVALID
} ALGORITHM_TYPE_E;

//文件类型 原文件、签名文件
typedef enum
{
    ADM_SIM_SOA_UCL_FILE_TYPE_ORI  = 1,
    ADM_SIM_SOA_UCL_FILE_TYPE_SIGN = 2,
    ADM_SIM_SOA_UCL_FILE_TYPE_ORI_512 = 3,
    ADM_SIM_SOA_UCL_FILE_TYPE_IINVALID
} FILE_TYPE_E;

//摘要算法类型 摘要值
typedef struct
{
    ALGORITHM_TYPE_E eAlgorthm;
    DCHAR* pAbstracValue;
} checkSum;

typedef struct
{
    D64S size;
    checkSum tObject;
    FILE_TYPE_E efiletype;
} FILEMETAS_INFO_T;

typedef struct
{
    D32U fileMetaNum;
    FILEMETAS_INFO_T** fileMetasArray;
} FILEMETAS_INFO_ARRAY_T;

typedef struct
{
    D8U result;    //0x01=suc; 0x02-0xfe=fail; 0xff时不支持验签 签名的内容长度绝对路径必填
    DCHAR* signature;
    D32U signatureLen;
    DCHAR*  filePath;    //必填 拆包后升级包绝对路径
} ADM_SYSI4UCL_VERIFY_SIGN_RESULT_INFO_T;

extern D32S adm_dim_ucl_request_Verify_Sign(const D16U instanceId,
        const D16U timeout,
        const DCHAR* fileID,
        const DCHAR* filePath,
        const FILEMETAS_INFO_ARRAY_T* fileMetasList,
        DBOOL isUnpick,
        D32S* errorCode);
typedef D32S(*adm_dim_ucl_request_Verify_Sign_func)(const D16U instanceId,
        const D16U timeout,
        const DCHAR* fileID,
        const DCHAR* filePath,
        const FILEMETAS_INFO_ARRAY_T* fileMetasList,
        DBOOL isUnpick,
        D32S* errorCode);


typedef D32S(*ADM_DIM_SOA_UCL_VERIFY_SIGN_PROGRESS_NOTIFY_CALLBACK_F)(
    const DCHAR* fileID, const D16U progress);
typedef D32S(*ADM_DIM_SOA_UCL_VERIFY_SIGN_RESULT_NOTIFY_CALLBACK_F)(
    const DCHAR* fileID,
    const ADM_SYSI4UCL_VERIFY_SIGN_RESULT_INFO_T* pVerifySignResultInfo);

/******************************** Verify Sign above **********************************/

/******************************** SOA-SIM-UCL do download pkg below **********************************/

extern D32S adm_dim_ucl_set_log_level(const D16U instanceId, const D16U timeout,
                                      D32U logLevel, D32S* errorCode);
typedef D32S(*adm_dim_ucl_set_log_level_func)(const D16U instanceId, const D16U timeout,
        D32U logLevel, D32S* errorCode);


extern D32S adm_dim_ucl_readVehicleStatus(const D16U instanceId, const D16U timeout,
        const ADM_DIM_SOA_MSGID_DATA_ARRAY_T* msgIdListRequest,
        ADM_DIM_SOA_MSGID_DATA_ARRAY_T** msgIdListResponse, D32S* errorCode);
typedef D32S(*adm_dim_ucl_readVehicleStatus_func)(const D16U instanceId, const D16U timeout,
        const ADM_DIM_SOA_MSGID_DATA_ARRAY_T* msgIdListRequest,
        ADM_DIM_SOA_MSGID_DATA_ARRAY_T** msgIdListResponse, D32S* errorCode);
extern D32S adm_dim_ucl_setVehicleStatus(const D16U instanceId, const D16U timeout,
        const ADM_DIM_SOA_MSGID_DATA_ARRAY_T* msgIdListRequest, D32S* errorCode);
typedef D32S(*adm_dim_ucl_setVehicleStatus_func)(const D16U instanceId, const D16U timeout,
        const ADM_DIM_SOA_MSGID_DATA_ARRAY_T* msgIdListRequest, D32S* errorCode);

/**
 * @brief 同步解密接口
 * @note
 * @param in const D16U usInstanceId        实例ID
 * @param in const D16U usTimeout           超时时间
 * @param in const DCHAR *pcSrcFilePath     待解密的文件路径
 * @param in const DCHAR **ppcDstFilePath   解密后的文件路径
 * @param in const DCHAR *pcDecryptKey      解密密钥
 * @param in const D64S ulDecryptSegLength  每次解密的长度
 * @param out D32S *errorCode 错误码
 * @retval D32S 0 成功，其他失败
 */
typedef D32S (*adm_dim_ucl_requestSyncDecryptPackage_func)(const D16U usInstanceId,
                                                           const D16U usTimeout,
                                                           const DCHAR *pcSrcFilePath,
                                                           const DCHAR *pcDstFilePath,
                                                           const DCHAR *pcDecryptKey,
                                                           const D64S ulDecryptSegLength,
                                                           D32S *errorCode);

/**
 * @brief 同步解密接口
 * @note
 * @param in const D16U usInstanceId        实例ID
 * @param in const D16U usTimeout           超时时间
 * @param in const DCHAR *pcSrcFilePath     待解密的文件路径
 * @param in const DCHAR **ppcDstFilePath   解密后的文件路径
 * @param in const DCHAR *pcDecryptKey      解密密钥
 * @param in const D64S ulDecryptSegLength  每次解密的长度
 * @param out D32S *errorCode 错误码
 * @retval D32S 0 成功，其他失败
 */
extern D32S adm_dim_ucl_requestSyncDecryptPackage(const D16U usInstanceId,
                                                 const D16U usTimeout,
                                                 const DCHAR *pcSrcFilePath,
                                                 const DCHAR *pcDstFilePath,
                                                 const DCHAR *pcDecryptKey,
                                                 const D64S ulDecryptSegLength,
                                                 D32S*errorCode);
/***************************************************************************SOA-DIM-UCM initialization***************************************************************************/

typedef D32S (*adm_dim_ucl_requestRmFile_func)(const D16U usInstanceId, const D16U usTimeout, DCHAR *pFilePath, D32S *errorCode);

extern D32S adm_dim_ucl_requestRmFile(const D16U usInstanceId, const D16U usTimeout, DCHAR *pFilePath, D32S*errorCode);

/**
 * @brief DIM-SOA-UCM initialization structure definition
 *
 */
typedef struct
{
    ADM_DIM_SOA_UCL_READ_PART_INFO_CALLBACK_F
    readPartInfoCallback;                                   /**!< The callback function is used to process the read part information thrown by UCM */
    ADM_DIM_SOA_UCL_UPGRADE_PROGRESS_NOTIFY_CALLBACK_F
    upgradeProgressNotifyCallback;                   /**!< The callback function is used to process the progress information thrown by the UCM do upgrade module */
    ADM_DIM_SOA_UCL_UPGRADE_RESULT_NOTIFY_CALLBACK_F
    upgradeResultNotifyCallback;                          /**!< The callback function is used to process the result information thrown by the UCM do upgrade module  */
    ADM_DIM_SOA_UCL_DECRYPT_PROGRESS_NOTIFY_CALLBACK_F
    decryptProgressNotifyCallback;                   /**!< The callback function is used to process the progress information thrown by the UCM model. */
    ADM_DIM_SOA_UCL_DECRYPT_RESULT_NOTIFY_CALLBACK_F
    decryptResultNotifyCallback;                      /**!< The callback function is used to process the decrypted result information sent by the UCM module. */
    ADM_DIM_SOA_UCL_VERIFY_HASH_PROGRESS_NOTIFY_CALLBACK_F
    verifyPackageHashProgressNotifyCallback;     /**!< The callback function for processing upgrade package HASH value verification */
    ADM_DIM_SOA_UCL_VERIFY_HASH_RESULT_NOTIFY_CALLBACK_F
    verifyPackageHashResultNotifyCallback;         /**!< The callback function for result upgrade package HASH value verification */
    ADM_DIM_SOA_UCL_VERIFY_SIGN_PROGRESS_NOTIFY_CALLBACK_F verifySignProgressNotifyCallback;
    ADM_DIM_SOA_UCL_VERIFY_SIGN_RESULT_NOTIFY_CALLBACK_F verifySignResultNotifyCallback;
    ADM_DIM_SOA_UCL_UPGRADE_ICC_PROGRESS_NOTIFY_CALLBACK_F upgradeIccProgressNotifyCallback;
    ADM_DIM_SOA_UCL_UPGRADE_ICC_RESULT_NOTIFY_CALLBACK_F upgradeIccResultNotifyCallback;
    ADM_DIM_SOA_UCL_UPGRADE_ICC_REBOOT_RESULT_NOTIFY_CALLBACK_F upgradeIccRebootResultNotifyCallback;
    ADM_DIM_SOA_UCL_APK_SELF_UPGRADE_PROGRESS_NOTIFY_CALLBACK_F apkSelfUpgradeProgressNotifyCallback;       ///!< APK 自升级进度抛送
    ADM_DIM_SOA_UCL_APK_SELF_UPGRADE_RESULT_NOTIFY_CALLBACK_F apkSelfUpgradeResultNotifyCallback;           ///!< APK 自升级结果抛送
} ADM_DIM_SOA_UCL_INIT_INFO_T;

/**
 * @brief The DIM-SOA-UCM module initializes the interface
 * @note Use adm_dim_ucl_deinit to release the initialization information
 *
 * @param initInfo      [IN]        ADM_DIM_SOA_UCL_INIT_INFO_T*        The SOA-DIM-UCM module initializes information, passing in the callback function required by the module
 * @param priv          [IN/OUT]    void*                               Reserved parameters
 * @return D32S         0:successed, Non-0:failed
 */
extern D32S adm_dim_ucl_init(ADM_DIM_SOA_UCL_INIT_INFO_T* initInfo, void* priv);
typedef D32S(*adm_dim_ucl_init_func)(D16U instanceId, ADM_DIM_SOA_UCL_INIT_INFO_T* initInfo, void* priv);

/**
 * @brief Release initialization information
 * @note Used to release the information initialized by the adm_dim_ucl_init function
 */
extern void adm_dim_ucl_deinit(void);
typedef void (*adm_dim_ucl_deinit_func)(D16U instanceId);

/***************************************************************************SOA-DIM-UCM initialization***************************************************************************/

#ifdef __cplusplus  // close out "C" linkage in case of c++ compiling
}
#endif

#endif//__ADM_DIM_SOA_UCL_H__
