/**
 * @file adm_dim_soaTransHost.h
 * @author mengwei@abupdate.com
 * @brief ewebserver在host设备上，此模块提供slave从host设备上下载文件（升级包、日志、证书等）和 slave上传文件到host设备上（升级包、日志、证书等）。
 * @version 0.1
 * @date 2021-05-10
 *
 * @copyright Copyright © 2018 All rights 上海艾拉比智能科技有限公司
 *
 */
#ifndef __ADM_DIM_TRANS_HOST_H__
#define __ADM_DIM_TRANS_HOST_H__

#ifdef __cplusplus
extern "C" {
#endif

#define EXPORT_FUN __attribute__((visibility("default")))

#include "adm_typedefs.h"

typedef struct ADM_SOA_SESSION_ ADM_SOA_SESSION_T;

/***************************************************************************Get SDK Version***************************************************************************************/
/**
 * @brief 获取trans的sdk版本号
 *
 * @param instanceId    [IN]    实例ID
 * @param timeout       [IN]    请求的超时时间
 * @param sdkVersion    [OUT]  SDK 版本号
 * @param errorCode     [OUT]  获取版本号的结果
 */
extern void adm_dim_trans_getSdkVersion(const D16U instanceId,
                                        const D16U timeout, DCHAR **sdkVersion, D32S *errorCode);
typedef void (*adm_dim_trans_getSdkVersion_func)(const D16U instanceId,
        const D16U timeout, DCHAR **sdkVersion, D32S *errorCode);
/***************************************************************************Get SDK Version***************************************************************************************/

/**
 * @brief Transmission file type definition
 *
 */
typedef enum
{
    ADM_DIM_SOA_TRANS_FILE_TYPE_PKG = 1,            /**!< Upgrade package */
    ADM_DIM_SOA_TRANS_FILE_TYPE_LOG,                /**!< log */
    ADM_DIM_SOA_TRANS_FILE_TYPE_SERVER_CERT,        /**!< Server root certificate */
    ADM_DIM_SOA_TRANS_FILE_TYPE_CND_SERVER_CERT,    /**!< CDN Server root certificate*/
    ADM_DIM_SOA_TRANS_FILE_TYPE_CLIENT_CERT,        /**!< Client certificate */
    ADM_DIM_SOA_TRANS_FILE_TYPE_KEYLOG,             /**!< key log */
    ADM_DIM_SOA_TRANS_FILE_TYPE_INVALID
} ADM_DIM_SOA_TRANS_FILE_TYPE_E;


/**
 * @brief Definition of the progress information structure of the file transfer
 *
 */
typedef struct
{
    D64S    savedSize;                                          /**!< The size of the downloaded upgrade package, unit:byte. */
    D64S    totalSize;                                          /**!< The total size of the upgrade package being downloaded, unit:byte */
    D32S    speed;                                              /**!< Download speed of upgrade package, unit:byte/s */
    D32S    remainTime;                                         /**!< Estimated remaining time to download, unit:second */
} ADM_DIM_SOA_TRANS_PROGRESS_INFO_T;


/**
 * @brief The result structure definition of the transfer file
 *
 */
typedef struct
{
    D32S   result;                                             /**!< The result of the remote module downloading the upgrade package */
    DCHAR *expectVal;                                          /**!< Description of the reason for the failure, expected value */
    DCHAR *realVal;                                            /**!< Description of the reason for the failure, expected value */
} ADM_DIM_SOA_TRANS_RESULT_INFO_T;

/***************************************************************************Transfer file Slave download from Host**************************************************************************/

/**
 * @brief Request the slave device to download the file from the host device.
 *
 * @param instanceId        [IN]        const D16U  instance identifier
 * @param timeout               [IN]        const D16U  Timeout of sync interface, unit: second
 * @param fileID            [IN]        const DCHAR*              ID of the file to be downloaded
 * @param eFileType         [IN]        const ADM_DIM_SOA_TRANS_FILE_TYPE_E     Transmission file type definition
 * @param url               [IN]        const DCHAR*              The address of the file to be downloaded
 * @param filename[IN]    const DCHAR*              The address of the file to be downloaded
 * @param size[IN]        const D64S                The size of the file to be downloaded, unit: Bytes
 * @param hash[IN]        const DCHAR*              The hash value of the file to be downloaded
 * @param errorCode[OUT]  D32S*                     0:successed(The message is received successfully and the download is triggered) ; Non-0: failed (Failure reason to be defined)
 * @return D32S           0:successed, Non-0:failed
 */
extern D32S adm_dim_trans_requestTransferDownloadFromHost(const D16U instanceId,
        const D16U timeout, const DCHAR *fileID,
        const ADM_DIM_SOA_TRANS_FILE_TYPE_E eFileType, const DCHAR *url,
        DCHAR *filename, const D64S size, const DCHAR *hash, D32S *errorCode);
typedef D32S(*adm_dim_trans_requestTransferDownloadFromHost_func)(
    const D16U instanceId, const D16U timeout, const DCHAR *fileID,
    const ADM_DIM_SOA_TRANS_FILE_TYPE_E eFileType, const DCHAR *url,
    DCHAR *filename, const D64S size, const DCHAR *hash, D32S *errorCode);

extern D32S adm_dim_trans_requestTransferSyncDownloadFromHost(const D16U instanceId,
        const D16U timeout, const DCHAR *fileID, const DCHAR *url,
        DCHAR *filename, const D64S size, const DCHAR *hash, D32S *errorCode);
typedef D32S(*adm_dim_trans_requestTransferSyncDownloadFromHost_func)(
    const D16U instanceId, const D16U timeout, const DCHAR *fileID, const DCHAR *url,
    DCHAR *filename, const D64S size, const DCHAR *hash, D32S *errorCode);

/**
 * @brief Notify the progress of the file transfer.
 *
 * @param fileID            [IN]        const DCHAR*              ID of the file to be downloaded
 * @param pstTransProgress  [IN]        const ADM_DIM_SOA_TRANS_PROGRESS_INFO_T*   the progress information structure of the file transfer
 * @param priv                              [IN/OUT]    void*                               Reserved parameters
 * @return D32S             0:successed, Non-0:failed
 */
typedef D32S(*ADM_DIM_SOA_TRANS_DOWNLOAD_FROM_HOST_PROGRESS_NOTIFY_CALLBACK_F)(
    const DCHAR *fileID, const ADM_DIM_SOA_TRANS_PROGRESS_INFO_T *pstTransProgress,
    void *priv);

/**
 * @brief Send the result of the file transfer.
 *
 * @param fileID            [IN]        const DCHAR*              ID of the file to be downloaded
 * @param storePath                         [IN]        const DCHAR*                The path where the downloaded file is stored (Absolute dir/filename)
 * @param pstTransResult    [IN]        const ADM_DIM_SOA_TRANS_RESULT_INFO_T*     The result structure definition of the transfer file
 * @param priv                              [IN/OUT]    void*                               Reserved parameters
 * @return D32S             0:successed, Non-0:failed
 */
typedef D32S(*ADM_DIM_SOA_TRANS_DOWNLOAD_FROM_HOST_RESULT_NOTIFY_CALLBACK_F)(
    const DCHAR *fileID, const DCHAR *storePath,
    const ADM_DIM_SOA_TRANS_RESULT_INFO_T *pstTransResult, void *priv);

/**
 * @brief Request query transfer progress of download from host device.
 *
 * @param instanceId        [IN]        const D16U  instance identifier
 * @param timeout               [IN]        const D16U  Timeout of sync interface, unit: second
 * @param fileID            [IN]        const DCHAR*              ID of the file to be downloaded
 * @param ppstTransProgress [OUT]       ADM_DIM_SOA_TRANS_PROGRESS_INFO_T**    the progress information structure of the file transfer
 * @param errorCode         [OUT]   D32S*   Error code
 * @return D32S             0:successed, Non-0:failed
 */
extern D32S adm_dim_trans_queryProgressDownloadFromHost(
    const D16U instanceId, const D16U timeout, const DCHAR *fileID,
    ADM_DIM_SOA_TRANS_PROGRESS_INFO_T **ppstTransProgress, D32S *errorCode);
typedef D32S(*adm_dim_trans_queryTransferDownloadFromHostProgress_func)(
    const D16U instanceId, const D16U timeout, const DCHAR *fileID,
    ADM_DIM_SOA_TRANS_PROGRESS_INFO_T **ppstTransProgress, D32S *errorCode);

/**
 * @brief Request query transfer result of download from host device.
 *
 * @param instanceId        [IN]        const D16U  instance identifier
 * @param timeout               [IN]        const D16U  Timeout of sync interface, unit: second
 * @param fileID            [IN]        const DCHAR*              ID of the file to be downloaded
 * @param storagePath       [OUT]       DCHAR**                   Transfer file storage absolute path
 * @param ppstTransResult   [OUT]       ADM_DIM_SOA_TRANS_RESULT_INFO_T**      The result structure definition of the transfer file
 * @param errorCode         [OUT]   D32S*   Error code
 * @return D32S             0:successed, Non-0:failed
 */
extern D32S adm_dim_trans_queryResultDownloadFromHost(
    const D16U instanceId, const D16U timeout, const DCHAR *fileID,
    DCHAR **storagePath, ADM_DIM_SOA_TRANS_RESULT_INFO_T **ppstTransResult,
    D32S *errorCode);
typedef D32S(*adm_dim_trans_queryTransferDownloadFromHostResult_func)(
    const D16U instanceId, const D16U timeout, const DCHAR *fileID,
    DCHAR **storagePath, ADM_DIM_SOA_TRANS_RESULT_INFO_T **ppstTransResult,
    D32S *errorCode);
/***************************************************************************Transfer file Slave download from Host**************************************************************************/


/***************************************************************************Transfer file Slave upload to Host*****************************************************************************/
/**
 * @brief Request the slave device to upload the file to the host device.
 *
 * @param instanceId        [IN]        const D16U  instance identifier
 * @param timeout               [IN]        const D16U  Timeout of sync interface, unit: second
 * @param fileID            [IN]        const DCHAR*              ID of the file to be uploaded
 * @param eFileType         [IN]        const ADM_DIM_SOA_TRANS_FILE_TYPE_E     Transmission file type definition
 * @param url               [IN]        const DCHAR*              The address of the file to be uploaded
 * @param filename          [OUT]       DCHAR**                   Uploaded file name
 * @param hash              [OUT]       DHCAR**                   Hash valud of uploaded file, (SHA256SUM)
 * @param size              [OUT]       D64S*                     Size of uploaded file, unit: Bytes
 * @param errorCode         [OUT]   D32S*                                           The execution result of this request
 * @return D32S             0:successed, Non-0:failed
 */
extern D32S adm_dim_trans_requestTransferUploadToHost(const D16U instanceId,
        const D16U timeout, const DCHAR *fileID,
        const ADM_DIM_SOA_TRANS_FILE_TYPE_E eFileType, const DCHAR *url,
        DCHAR **filename, DCHAR **hash, D64S *size, D32S *errorCode);
typedef D32S(*adm_dim_trans_requestTransferUploadToHost_func)(
    const D16U instanceId, const D16U timeout, const DCHAR *fileID,
    const ADM_DIM_SOA_TRANS_FILE_TYPE_E eFileType, const DCHAR *url,
    DCHAR **filename, DCHAR **hash, D64S *size, D32S *errorCode);

/**
 * @brief Notify the progress of the file transfer.
 *
 * @param fileID            [IN]        const DCHAR*              ID of the file to be uploaded
 * @param pstTransProgress  [IN]        const ADM_DIM_SOA_TRANS_PROGRESS_INFO_T*   the progress information structure of the file transfer
 * @param priv                              [IN/OUT]    void*                               Reserved parameters
 * @return D32S             0:successed, Non-0:failed
 */
typedef D32S(*ADM_DIM_SOA_TRANS_UPLOAD_TO_HOST_PROGRESS_NOTIFY_CALLBACK_F)(
    const DCHAR *fileID, const ADM_DIM_SOA_TRANS_PROGRESS_INFO_T *pstTransProgress,
    void *priv);


/**
 * @brief Notify the result of the file transfer.
 *
 * @param fileID            [IN]        const DCHAR*              ID of the file to be downloaded
 * @param pstTransResult    [IN]        const ADM_DIM_SOA_TRANS_RESULT_INFO_T*     The result structure definition of the transfer file
 * @param priv                              [IN/OUT]    void*                               Reserved parameters
 * @return D32S             0:successed, Non-0:failed
 */
typedef D32S(*ADM_DIM_SOA_TRANS_UPLOAD_TO_HOST_RESULT_NOTIFY_CALLBACK_F)(
    const DCHAR *fileID, const DCHAR* filename, const ADM_DIM_SOA_TRANS_RESULT_INFO_T *pstTransResult,
    void *priv);


/**
 * @brief Request query transfer progress of upload to host device.
 *
 * @param instanceId        [IN]        const D16U  instance identifier
 * @param timeout               [IN]        const D16U  Timeout of sync interface, unit: second
 * @param fileID            [IN]        const DCHAR*              ID of the file to be downloaded
 * @param ppstTransProgress [OUT]       ADM_DIM_SOA_TRANS_PROGRESS_INFO_T**    the progress information structure of the file transfer
 * @param errorCode         [OUT]       D32S*   Error code
 * @return D32S             0:successed, Non-0:failed
 */
extern D32S adm_dim_trans_queryProgressUploadToHost(
    const D16U instanceId, const D16U timeout, const DCHAR *fileID,
    ADM_DIM_SOA_TRANS_PROGRESS_INFO_T **ppstTransProgress, D32S *errorCode);
typedef D32S(*adm_dim_trans_queryTransferUploadToHostProgress_func)(
    const D16U instanceId, const D16U timeout, const DCHAR *fileID,
    ADM_DIM_SOA_TRANS_PROGRESS_INFO_T **ppstTransProgress, D32S *errorCode);

/**
 * @brief Request query transfer result of upload to host device.
 *
 * @param instanceId        [IN]        const D16U  instance identifier
 * @param timeout               [IN]        const D16U  Timeout of sync interface, unit: second
 * @param fileID            [IN]        const DCHAR*              ID of the file to be downloaded
 * @param storagePath       [OUT]       DCHAR**                   Transfer file storage absolute path
 * @param ppstTransResult   [OUT]       ADM_DIM_SOA_TRANS_RESULT_INFO_T**      The result structure definition of the transfer file
 * @param errorCode         [OUT]       D32S*   Error code
 * @return D32S             0:successed, Non-0:failed
 */
extern D32S adm_dim_trans_queryResultUploadToHost(const D16U instanceId,
        const D16U timeout, const DCHAR *fileID, DCHAR **filename,
        ADM_DIM_SOA_TRANS_RESULT_INFO_T **ppstTransResult, D32S *errorCode);
typedef D32S(*adm_dim_trans_queryTransferUploadToHostResult_func)(
    const D16U instanceId, const D16U timeout, const DCHAR *fileID,
    DCHAR **filename, ADM_DIM_SOA_TRANS_RESULT_INFO_T **ppstTransResult,
    D32S *errorCode);
/***************************************************************************Transfer file Slave upload to Host*****************************************************************************/

extern D32S adm_dim_trans_set_log_level(const D16U instanceId, const D16U timeout, 
        D32U logLevel, D32S* errorCode);
typedef D32S (*adm_dim_trans_set_log_level_func)(const D16U instanceId, const D16U timeout, 
        D32U logLevel, D32S* errorCode);

/***************************************************************************SOA-DIM-TRANS server on host init******************************************************************************/
/**
 * @brief DIM-SOA-TRANSFER-ON-HOST initialization structure definition
 *
 */
typedef struct
{
    ADM_DIM_SOA_TRANS_DOWNLOAD_FROM_HOST_PROGRESS_NOTIFY_CALLBACK_F
    transDownloadFromHostProgressNotifyCallback;        /**!< Notify the progress of the file transfer. */
    ADM_DIM_SOA_TRANS_DOWNLOAD_FROM_HOST_RESULT_NOTIFY_CALLBACK_F
    transDownloadFromHostResultNotifyCallback;                /**!< Notify the result of the file transfer. */
    ADM_DIM_SOA_TRANS_UPLOAD_TO_HOST_PROGRESS_NOTIFY_CALLBACK_F
    transUploadToHostProgressNotifyCallback;                /**!< Notify the progress of the file transfer. */
    ADM_DIM_SOA_TRANS_UPLOAD_TO_HOST_RESULT_NOTIFY_CALLBACK_F
    transUploadToHostResultNotifyCallback;                        /**!< Notify the result of the file transfer. */
} ADM_DIM_SOA_TRANS_HOST_INIT_INFO_T;

/**
 * @brief The SOA-DIM-TRANS-ON-HOST module initializes the interface.
 * @note Use adm_dim_transferOnHost_deinit to release the initialization information
 *
 * @param initInfo  [IN]    const ADM_DIM_SOA_TRANS_HOST_INIT_INFO_T* SOA-DIM-TRANS-ON-HOST initialization structure
 * @param priv      [IN/OUT]    void*           Reserved parameters
 * @return D32S     0:successed, Non-0:failed
 */
extern D32S adm_dim_transferOnHost_init( const ADM_DIM_SOA_TRANS_HOST_INIT_INFO_T *initInfo, void *priv);
typedef D32S(*adm_dim_transferOnHost_init_func)(D16U instanceId, const
        ADM_DIM_SOA_TRANS_HOST_INIT_INFO_T *initInfo, void *priv);

/**
 * @brief Release initialization information
 * @note Used to release the information initialized by the adm_dim_transferOnHost_init function
 */
extern void adm_dim_transferOnHost_deinit(void);
typedef void (*adm_dim_transferOnHost_deinit_func)(D16U instanceId);
/***************************************************************************SOA-DIM-TRANS server on host init******************************************************************************/

extern D32S adm_dim_trans_switchTransMode(const D32S iSwitchType, const D16U uiInstanceId,
                                          const D16U uiTimeout, D32S *pErrorCode);
typedef D32S (*adm_dim_trans_switchTransMode_func)(
    const D32S iSwitchType,
    const D16U uiInstanceId,
    const D16U uiTimeout, D32S *pErrorCode);

#ifdef __cplusplus  // close out "C" linkage in case of c++ compiling
}
#endif

#endif//__ADM_DIM_TRANS_HOST_H__