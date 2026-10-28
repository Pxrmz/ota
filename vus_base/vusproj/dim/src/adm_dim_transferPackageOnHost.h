/**
 * @file adm_dim_soaTransferPackageOnHost.h
 * @brief 为升级包传输定义的需要UCM提供的服务化接口，注：ewebserver在宿主设备上。
 * @author Mengwei
 * @version v0.1.0
 * @date 2021-05-06
 * @copyright Copyright © 2018 All rights 上海艾拉比智能科技有限公司
 */
#ifndef __ADM_DIM_SOA_TRANSFER_PACKAGE_ON_HOST_H__
#define __ADM_DIM_SOA_TRANSFER_PACKAGE_ON_HOST_H__

#ifdef __cplusplus
extern "C" {
#endif

#define EXPORT_FUN __attribute__((visibility("default")))

#include "adm_typedefs.h"

// TODO: fileID???? 还有需要上传的文件
/**
 * @brief Structure definition used to uniquely identify an ECU.
 * 
 */
typedef struct 
{
    D32S requestId;             /**!< Request ID */
    D32S responseId;            /**!< Response ID */
    D32S swDid;                 /**!< The did of the software version, used to distinguish different upgrade parts of the same requestId ECU */
} ADM_DIM_SOA_TRANSFER_PACKAGE_ON_HOST_ECU_IDEN_INFO_T;


/***************************************************************************Transfer package Slave download from host**************************************************************************/
/**
 * @brief Request to download the upgrade package from the host device.
 * 
 * @param pstEcuIdenInfo    [IN]    const ADM_DIM_SOA_TRANSFER_PACKAGE_ON_HOST_ECU_IDEN_INFO_T*     Structure definition used to uniquely identify an ECU.
 * @param isRollbackflag    [IN]    const DBOOL     True: Rollback, False: Not Rollback 
 * @param url               [IN]    const url       The download address of the upgrade package
 * @param size              [IN]    const D64S      The size of the upgrade package, unit: bytes
 * @param hash              [IN]    const DCHAR*    HASH value of the upgrade package (SHA256 sum)
 * @return D32S             0:successed, Non-0:failed
 */
extern D32S adm_dim_requestTransferPackgeDownFromServerOnHost(const ADM_DIM_SOA_TRANSFER_PACKAGE_ON_HOST_ECU_IDEN_INFO_T* pstEcuIdenInfo, const DBOOL isRollbackflag, const DCHAR* url, const D64S size, const DCHAR* hash);

/**
 * @brief Upgrade package transmission, download the progress information structure definition of the upgrade package from the host device.
 * 
 */
typedef struct {
    D64S    savedSize;                                          /**!< The size of the downloaded upgrade package, unit:byte. */
    D64S    totalSize;                                          /**!< The total size of the upgrade package being downloaded, unit:byte */
    D32S    speed;                                              /**!< Download speed of upgrade package, unit:byte/s */
    D32S    remainTime;                                         /**!< Estimated remaining time to download, unit:second */
} ADM_DIM_SOA_TRANSFER_PACKAGE_ON_HOST_PROGRESS_INFO_T;

/**
 * @brief   Upgrade package transmission, the progress processing callback function of downloading the upgrade package from the host device. 
 * 
 * @param pstEcuIdenInfo        [IN]    const ADM_DIM_SOA_TRANSFER_PACKAGE_ON_HOST_ECU_IDEN_INFO_T*     Structure definition used to uniquely identify an ECU.
 * @param isRollbackflag        [IN]    const DBOOL     True: Rollback, False: Not Rollback 
 * @param state                 [IN]    const DCHAR*    The state value of the ECU, the default is Ecu_Transfer_Ing
 * @param transferProgressInfo  [IN]    const ADM_DIM_SOA_TRANSFER_PACKAGE_ON_HOST_PROGRESS_INFO_T*     Upgrade package transmission, download the progress information structure definition of the upgrade package from the host device.
 * @param priv                              [IN/OUT]    void*                               Reserved parameters
 * @return D32S                 0:successed, Non-0:failed
 */
typedef D32S (*ADM_DIM_SOA_TRANSFER_PACKAGE_PROGRESS_NOTIFY_DOWN_FROM_SERVER_ON_HOST_CALLBACK_F)(const ADM_DIM_SOA_TRANSFER_PACKAGE_ON_HOST_ECU_IDEN_INFO_T* pstEcuIdenInfo, const DBOOL isRollbackflag, const DCHAR* state, const ADM_DIM_SOA_TRANSFER_PACKAGE_ON_HOST_PROGRESS_INFO_T* transferProgressInfo, void* priv);

/**
 * @brief Upgrade package transmission, download the result information structure definition of the upgrade package from the host device.
 * 
 */
typedef struct {
    D32S result;                                                /**!< The result of the transfering the upgrade package */
    DCHAR*  expectVal;                                          /**!< Description of the reason for the failure, expected value */
    DCHAR*  realVal;                                            /**!< Description of the reason for the failure, expected value */
} ADM_DIM_SOA_TRANSFER_PACKAGE_ON_HOST_RESULT_INFO_T;

/**
 * @brief   Upgrade package transmission, the result processing callback function of downloading the upgrade package from the host device. 
 * 
 * @param pstEcuIdenInfo        [IN]    const ADM_DIM_SOA_TRANSFER_PACKAGE_ON_HOST_ECU_IDEN_INFO_T*     Structure definition used to uniquely identify an ECU.
 * @param isRollbackflag        [IN]    const DBOOL     True: Rollback, False: Not Rollback 
 * @param state                 [IN]    const DCHAR*    The state value of the ECU, the default is Ecu_Transfer_Ing
 * @param pstTransferResultInfo [IN]    const ADM_DIM_SOA_TRANSFER_PACKAGE_ON_HOST_RESULT_INFO_T*     Upgrade package transmission, download the result information structure definition of the upgrade package from the host device.
 * @param priv                              [IN/OUT]    void*                               Reserved parameters
 * @return D32S                 0:successed, Non-0:failed
 */
typedef D32S (*ADM_DIM_SOA_TRANSFER_PACKAGE_RESULT_SEND_DOWN_FROM_SERVER_ON_HOST_CALLBACK_F)(const ADM_DIM_SOA_TRANSFER_PACKAGE_ON_HOST_ECU_IDEN_INFO_T* pstEcuIdenInfo, const DBOOL isRollbackflag, const DCHAR* state, const ADM_DIM_SOA_TRANSFER_PACKAGE_ON_HOST_RESULT_INFO_T* pstTransferResultInfo, void* priv);

/**
 * @brief   Upgrade package transmission, query the progress of the upgrade package download from the host device. 
 * @note  The output parameters need to free up memory
 * 
 * @param pstEcuIdenInfo        [IN]    const ADM_DIM_SOA_TRANSFER_PACKAGE_ON_HOST_ECU_IDEN_INFO_T*     Structure definition used to uniquely identify an ECU.
 * @param isRollbackflag        [IN]    const DBOOL     True: Rollback, False: Not Rollback 
 * @param state                 [OUT]   DCHAR**    The state value of the ECU, the default is Ecu_Transfer_Ing
 * @param ppstTransferProgressInfo  [IN] ADM_DIM_SOA_TRANSFER_PACKAGE_ON_HOST_PROGRESS_INFO_T**     Upgrade package transmission, download the progress information structure definition of the upgrade package from the host device.
 * @return D32S                 0:successed, Non-0:failed
 */
extern D32S adm_dim_getTransferPackageProgressDownFromServerOnHost(const ADM_DIM_SOA_TRANSFER_PACKAGE_ON_HOST_ECU_IDEN_INFO_T* pstEcuIdenInfo, const DBOOL isRollbackflag, DCHAR** state, ADM_DIM_SOA_TRANSFER_PACKAGE_ON_HOST_PROGRESS_INFO_T** ppstTransferProgressInfo);

/**
 * @brief   Upgrade package transmission, the result processing callback function of downloading the upgrade package from the host device. 
 * @note  The output parameters need to free up memory
 * 
 * @param pstEcuIdenInfo        [IN]    const ADM_DIM_SOA_TRANSFER_PACKAGE_ON_HOST_ECU_IDEN_INFO_T*     Structure definition used to uniquely identify an ECU.
 * @param isRollbackflag        [IN]    const DBOOL     True: Rollback, False: Not Rollback 
 * @param state                 [OUT]    DCHAR**    The state value of the ECU, the default is Ecu_Transfer_Finished
 * @param ppstTransferResultInfo [OUT]   ADM_DIM_SOA_TRANSFER_PACKAGE_ON_HOST_RESULT_INFO_T**     Upgrade package transmission, download the result information structure definition of the upgrade package from the host device.
 * @return D32S                 0:successed, Non-0:failed
 */
extern D32S adm_dim_getTransferPackageResultDownFromServerOnHost(const ADM_DIM_SOA_TRANSFER_PACKAGE_ON_HOST_ECU_IDEN_INFO_T* pstEcuIdenInfo, const DBOOL isRollbackflag, DCHAR** state, ADM_DIM_SOA_TRANSFER_PACKAGE_ON_HOST_RESULT_INFO_T** pstTransferResultInfo);
/***************************************************************************Transfer package Slave download from host**************************************************************************/

/***************************************************************************Transfer package Slave upload to host******************************************************************************/

/**
 * @brief Upgrade package transmission, request the slave to upload the upgrade package to the host device.
 * 
 * @param pstEcuIdenInfo        [IN]    const ADM_DIM_SOA_TRANSFER_PACKAGE_ON_HOST_ECU_IDEN_INFO_T*     Structure definition used to uniquely identify an ECU.
 * @param isRollbackflag        [IN]    const DBOOL     True: Rollback, False: Not Rollback 
 * @param hash                  [OUT]   DCHAR**    HASH value of the upgrade package (SHA256 sum)
 * @param size                  [OUT]   D64S*      The size of the upgrade package, unit: bytes
 * @param filename              [OUT]   DCHAR**    Uploaded file name
 * @return D32S                 0:successed, Non-0:failed
 */
extern D32S adm_dim_requestTransferPackageUploadToServerOnHost(const ADM_DIM_SOA_TRANSFER_PACKAGE_ON_HOST_ECU_IDEN_INFO_T* pstEcuIdenInfo, const DBOOL isRollbackflag, DCHAR** hash, D64S* size, DCHAR** filename);

/**
 * @brief Upgrade package transmission, a callback function used to process the progress of the upgrade package uploaded by the Slave to the Host device.
 * 
 * @param pstEcuIdenInfo        [IN]    const ADM_DIM_SOA_TRANSFER_PACKAGE_ON_HOST_ECU_IDEN_INFO_T*     Structure definition used to uniquely identify an ECU.
 * @param state                 [IN]    const DCHAR*    The state value of the ECU, the default is Ecu_Transfer_Ing
 * @param pstTransferProgressInfo [IN]  const ADM_DIM_SOA_TRANSFER_PACKAGE_ON_HOST_PROGRESS_INFO_T*     Upgrade package transmission, download the progress information structure definition of the upgrade package from the host device.
 * @param priv                              [IN/OUT]    void*                               Reserved parameters
 * @return D32S                 0:successed, Non-0:failed
 */
typedef D32S (*ADM_DIM_SOA_TRANSFER_PACKAGE_PROGRESS_NOTIFY_UPLOAD_TO_SERVER_ON_HOST_CALLBACK_F)(const ADM_DIM_SOA_TRANSFER_PACKAGE_ON_HOST_ECU_IDEN_INFO_T* pstEcuIdenInfo, const DCHAR* state, const ADM_DIM_SOA_TRANSFER_PACKAGE_ON_HOST_PROGRESS_INFO_T* pstTransferProgressInfo, void* priv);


/**
 * @brief Upgrade package transmission, a callback function used to process the result of the upgrade package uploaded by the Slave to the Host device.
 * 
 * @param pstEcuIdenInfo        [IN]    const ADM_DIM_SOA_TRANSFER_PACKAGE_ON_HOST_ECU_IDEN_INFO_T*     Structure definition used to uniquely identify an ECU.
 * @param state                 [IN]    const DCHAR*    The state value of the ECU, the default is Ecu_Transfer_Finished
 * @param pstTransferResultInfo [IN]    const ADM_DIM_SOA_TRANSFER_PACKAGE_ON_HOST_RESULT_INFO_T*     Upgrade package transmission, download the result information structure definition of the upgrade package from the host device.
 * @param priv                              [IN/OUT]    void*                               Reserved parameters
 * @return D32S                 0:successed, Non-0:failed
 */
typedef D32S (*ADM_DIM_SOA_TRANSFER_PACKAGE_RESULT_SEND_UPLOAD_TO_SERVER_ON_HOST_CALLBACK_F)(const ADM_DIM_SOA_TRANSFER_PACKAGE_ON_HOST_ECU_IDEN_INFO_T* pstEcuIdenInfo, const DCHAR* state, const ADM_DIM_SOA_TRANSFER_PACKAGE_ON_HOST_RESULT_INFO_T* pstTransferResultInfo, void* priv);


/**
 * @brief Upgrade package transmission, used to query the progress of the slave uploading the upgrade package to the host device.
 * 
 * @param pstEcuIdenInfo        [IN]    const ADM_DIM_SOA_TRANSFER_PACKAGE_ON_HOST_ECU_IDEN_INFO_T*     Structure definition used to uniquely identify an ECU.
 * @param isRollbackflag        [IN]    const DBOOL     True: Rollback, False: Not Rollback 
 * @param filename              [OUT]   DCHAR**    Uploaded file name
 * @param state                 [OUT]   DCHAR**    The state value of the ECU, the default is Ecu_Transfer_Ing
 * @param ppstTransferProgressInfo  [IN] ADM_DIM_SOA_TRANSFER_PACKAGE_ON_HOST_PROGRESS_INFO_T**     Upgrade package transmission, download the progress information structure definition of the upgrade package from the host device.
 * @return D32S                 0:successed, Non-0:failed
 */
extern D32S adm_dim_getTransferPackageProgressUploadToServerOnHost(const ADM_DIM_SOA_TRANSFER_PACKAGE_ON_HOST_ECU_IDEN_INFO_T* pstEcuIdenInfo, const DBOOL isRollbackflag, DCHAR** state, ADM_DIM_SOA_TRANSFER_PACKAGE_ON_HOST_PROGRESS_INFO_T** ppstTransferProgressInfo);


/**
 * @brief Upgrade package transmission, used to query the result of the slave uploading the upgrade package to the host device.
 * 
 * @param pstEcuIdenInfo        [IN]    const ADM_DIM_SOA_TRANSFER_PACKAGE_ON_HOST_ECU_IDEN_INFO_T*     Structure definition used to uniquely identify an ECU.
 * @param isRollbackflag        [IN]    const DBOOL     True: Rollback, False: Not Rollback 
 * @param filename              [OUT]   DCHAR**    Uploaded file name
 * @param state                 [OUT]   DCHAR**    The state value of the ECU, the default is Ecu_Transfer_Finished
 * @param ppstTransferResultInfo [OUT]  ADM_DIM_SOA_TRANSFER_PACKAGE_ON_HOST_RESULT_INFO_T**     Upgrade package transmission, download the result information structure definition of the upgrade package from the host device.
 * @return D32S                 0:successed, Non-0:failed
 */
extern D32S adm_dim_getTransferPackageResultUploadToServerOnHost(const ADM_DIM_SOA_TRANSFER_PACKAGE_ON_HOST_ECU_IDEN_INFO_T* pstEcuIdenInfo, const DBOOL isRollbackflag, DCHAR** filename, DCHAR** state, ADM_DIM_SOA_TRANSFER_PACKAGE_ON_HOST_RESULT_INFO_T** ppstTransferResultInfo);

/***************************************************************************Transfer package Slave upload to host******************************************************************************/

/***************************************************************************SOA-DIM-TRANSFER-PACKAGE server on host init***********************************************************************/
/**
 * @brief DIM-SOA-TRANSFER-PACKAGE-ON-HOST initialization structure definition
 * 
 */
typedef struct {
    ADM_DIM_SOA_TRANSFER_PACKAGE_PROGRESS_NOTIFY_DOWN_FROM_SERVER_ON_HOST_CALLBACK_F downloadProgressNotifyCallback;    /**!< Upgrade package transmission, the progress processing callback function of downloading the upgrade package from the host device.  */
    ADM_DIM_SOA_TRANSFER_PACKAGE_RESULT_SEND_DOWN_FROM_SERVER_ON_HOST_CALLBACK_F    downloadResultNotifyCallback;       /**!< Upgrade package transmission, the result processing callback function of downloading the upgrade package from the host device.  */
    ADM_DIM_SOA_TRANSFER_PACKAGE_PROGRESS_NOTIFY_UPLOAD_TO_SERVER_ON_HOST_CALLBACK_F    uploadProgressNotifyCallback;   /**!< Upgrade package transmission, a callback function used to process the progress of the upgrade package uploaded by the Slave to the Host device. */
    ADM_DIM_SOA_TRANSFER_PACKAGE_RESULT_SEND_UPLOAD_TO_SERVER_ON_HOST_CALLBACK_F    uploadResultNotifyCallback;         /**!< Upgrade package transmission, a callback function used to process the result of the upgrade package uploaded by the Slave to the Host device. */
} ADM_DIM_SOA_TRANSFER_PACKAGE_ON_HOST_INIT_INFO_T;

/**
 * @brief The DIM-SOA-TRANSFER-PACKAGE-ON-HOST module initializes the interface.
 * @note Use adm_dim_soa_transferPackageOnHost_deinit to release the initialization information
 * 
 * @param initInfo  [IN]    const ADM_DIM_SOA_TRANSFER_PACKAGE_ON_HOST_INIT_INFO_T* DIM-SOA-TRANSFER_PACKAGE-ON-HOST initialization structure
 * @param priv      [IN/OUT]    void*           Reserved parameters
 * @return D32S     0:successed, Non-0:failed
 */
extern D32S adm_dim_soa_transferPackageOnHost_init(D16U instanceId, const ADM_DIM_SOA_TRANSFER_PACKAGE_ON_HOST_INIT_INFO_T* initInfo, void* priv);

/**
 * @brief Release initialization information
 * @note Used to release the information initialized by the adm_dim_soa_transferPackageOnHost_init function
 */
extern void adm_dim_soa_transferPackageOnHost_deinit(D16U instanceId);
/***************************************************************************SOA-DIM-TRANSFER-PACKAGE server on host init***********************************************************************/

#ifdef __cplusplus  // close out "C" linkage in case of c++ compiling
}
#endif

#endif//__ADM_DIM_SOA_TRANSFER_PACKAGE_ON_HOST_H__