/**
 * @file adm_dim_soaDpo.h
 * @author mengwei@abupdate.com
 * @brief 提供服务化接口，用于下载一个文件
 * @version 0.1
 * @date 2021-05-10
 *
 * @copyright Copyright © 2018 All rights 上海艾拉比智能科技有限公司
 *
 */
#ifndef __ADM_DIM_DPO_H__
#define __ADM_DIM_DPO_H__

#ifdef __cplusplus
extern "C" {
#endif

#define EXPORT_FUN __attribute__((visibility("default")))

#include "adm_dim_common.h"

/***************************************************************************Get SDK Version***************************************************************************************/
/**
 * @brief 获取DPO的sdk版本号
 *
 * @param instanceId    [IN]    实例ID
 * @param timeout       [IN]    请求的超时时间
 * @param sdkVersion    [OUT]  SDK 版本号
 * @param errorCode     [OUT]  获取版本号的结果
 */
extern void adm_dim_dpo_getSdkVersion(const D16U instanceId, const D16U timeout,
                                      DCHAR **sdkVersion, D32S *errorCode);
typedef void (*adm_dim_dpo_getSdkVersion_func)(const D16U instanceId,
        const D16U timeout, DCHAR **sdkVersion, D32S *errorCode);
/***************************************************************************Get SDK Version***************************************************************************************/


/***************************************************************************Request remote download**************************************************************************/
// TODO: ErrorCdoe to be defined.
/**
 * @brief Request the SYSI4DPO module to perform the remote download action.
 *
 * @param instanceId        [IN]        const D16U  instance identifier
 * @param timeout               [IN]        const D16U  Timeout of sync interface, unit: second
 * @param fileID            [IN]        const DCHAR*              ID of the file to be downloaded
 * @param url               [IN]        const DCHAR*              The address of the file to be downloaded
 * @param size              [IN]        const D64S                The size of the file to be downloaded, unit: Bytes
 * @param hash              [IN]        const DCHAR*              The hash value of the file to be downloaded, unit: Bytes
 * @param filename          [IN]        const DCHAR*              File name used for storage
 * @param errorCode         [OUT]       D32S*                     0:successed(The message is received successfully and the download is triggered) ; Non-0: failed (Failure reason to be defined)
 * @return D32S             0:successed, Non-0:failed
 */
extern D32S adm_dim_dpo_requestRemoteDownloadPackage(const D16U instanceId,
        const D16U timeout, const DCHAR *fileID, const DCHAR *url, const D64S size,
        const DCHAR *hash, const D32S networkRetriesCnt, D32S *errorCode);
typedef D32S(*adm_dim_dpo_requestRemoteDownloadPackage_func)(
    const D16U instanceId, const D16U timeout, const DCHAR *fileID,
    const DCHAR *url, const D64S size, const DCHAR *hash, const D32S networkRetriesCnt, D32S *errorCode);
/***************************************************************************Request remote download**************************************************************************/

/***************************************************************************Notify download progress**************************************************************************/
/**
 * @brief Definition of the progress information structure of the upgrade package download
 *
 */
typedef struct
{
    D64S    savedSize;                                          /**!< The size of the downloaded upgrade package, unit:byte. */
    D64S    totalSize;                                          /**!< The total size of the upgrade package being downloaded, unit:byte */
    D32S    speed;                                              /**!< Download speed of upgrade package, unit:byte/s */
    D32S    remainTime;                                         /**!< Estimated remaining time to download, unit:second */
} ADM_DIM_SOA_DPO_DOWNLOAD_PROGRESS_INFO_T;

/**
 * @brief The callback function is used to process the progress information thrown by the remote download module.
 *
 * @param fileID                            [IN]        const DCHAR*              ID of the file to be downloaded
 * @param pstDownloadProgressInfo           [IN]   const ADM_DIM_SOA_DPO_DOWNLOAD_PROGRESS_INFO_T*  The progress information structure of the upgrade package download.
 * @param priv                              [IN/OUT]    void*                               Reserved parameters
 * @return  D32S                            0:successed, Non-0:failed
 */
typedef D32S(*ADM_DIM_DOWNLOAD_PROGRESS_NOTIFY_CALLBACK_F)(const DCHAR *fileID,
        const ADM_DIM_SOA_DPO_DOWNLOAD_PROGRESS_INFO_T *pstDownloadProgressInfo,
        void *priv);
/***************************************************************************Notify download progress**************************************************************************/

/***************************************************************************Send download result******************************************************************************/
/**
 * @brief The result structure definition of the remote download upgrade package
 *
 */
typedef struct
{
    D32S result;                                                /**!< The result of the remote module downloading the upgrade package */
    DCHAR  *expectVal;                                          /**!< Description of the reason for the failure, expected value */
    DCHAR  *realVal;                                            /**!< Description of the reason for the failure, expected value */
} ADM_DIM_SOA_DPO_DOWNLOAD_RESULT_INFO_T;

/**
 * @brief The callback function is used to process the result information notified by the remote download module.
 *
 * @param fileID                            [IN]        const DCHAR*              ID of the file to be downloaded
 * @param storePath                         [IN]        const DCHAR*                The path where the downloaded file is stored (Absolute dir/filename)
 * @param pstDownloadResultInfo             [IN]   const ADM_DIM_SOA_DPO_DOWNLOAD_RESULT_INFO_T*  The result structure definition of the remote download upgrade package
 * @param priv                              [IN/OUT]    void*                               Reserved parameters
 * @return  D32S                            0:successed, Non-0:failed
 */
typedef D32S(*ADM_DIM_DOWNLOAD_RESULT_NOTIFY_CALLBACK_F)(const DCHAR *fileID,
        const DCHAR *storePath, const ADM_DIM_SOA_DPO_DOWNLOAD_RESULT_INFO_T *pstDownloadResultInfo, void *priv);
/***************************************************************************Send download result******************************************************************************/

/***************************************************************************Query download progress***************************************************************************/
/**
 * @brief Request to obtain the progress of the current ECU upgrade package download.
 *
 * @param instanceId        [IN]        const D16U  instance identifier
 * @param timeout               [IN]        const D16U  Timeout of sync interface, unit: second
 * @param fileID                            [IN]        const DCHAR*              ID of the file to be downloaded
 * @param pstDownloadProgressNotifyINfo     [OUT]   ADM_DIM_SOA_DPO_DOWNLOAD_PROGRESS_INFO_T** The progress value of the current ECU upgrade package being downloaded
 * @param errorCode                         [OUT]   D32S*   error code
 * @return D32S                             0:successed, Non-0:failed
 */
extern D32S adm_dim_dpo_getRemoteDownloadProgress(const D16U instanceId,
        const D16U timeout, const DCHAR *fileID,
        ADM_DIM_SOA_DPO_DOWNLOAD_PROGRESS_INFO_T **pstDownloadProgressInfo,
        D32S *errorCode);
typedef D32S(*adm_dim_dpo_getRemoteDownloadProgress_func)(const D16U instanceId,
        const D16U timeout, const DCHAR *fileID,
        ADM_DIM_SOA_DPO_DOWNLOAD_PROGRESS_INFO_T **pstDownloadProgressInfo,
        D32S *errorCode);
/***************************************************************************Query download progress***************************************************************************/

/***************************************************************************Query download result*****************************************************************************/
/**
 * @brief Request to obtain the result of the current ECU upgrade package download.
 *
 * @param instanceId        [IN]        const D16U  instance identifier
 * @param timeout               [IN]        const D16U  Timeout of sync interface, unit: second
 * @param fileID                            [IN]        const DCHAR*              ID of the file to be downloaded
 * @param storePath                         [IN]        const DCHAR**                The path where the downloaded file is stored (Absolute dir/filename)
 * @param pstDownloadResultInfo             [OUT]   ADM_DIM_SOA_DPO_DOWNLOAD_RESULT_INFO_T**   The result value of the ECU upgrade package downloaded
 * @param errorCode                         [OUT]   D32S*   error code
 * @return D32S                             0:successed, Non-0:failed
 */
extern D32S adm_dim_dpo_getRemoteDownloadResult(const D16U instanceId,
        const D16U timeout, const DCHAR *fileID, DCHAR **storePath,
        ADM_DIM_SOA_DPO_DOWNLOAD_RESULT_INFO_T **pstDownloadResultInfo,
        D32S *errorCode);
typedef D32S(*adm_dim_dpo_getRemoteDownloadResult_func)(const D16U instanceId,
        const D16U timeout, const DCHAR *fileID, DCHAR **storePath,
        ADM_DIM_SOA_DPO_DOWNLOAD_RESULT_INFO_T **pstDownloadResultInfo,
        D32S *errorCode);
/***************************************************************************Query download result*****************************************************************************/

/******** request clean download dir below ***********/
extern D32S adm_dim_dpo_cleanDownloadDir(const D16U instanceId, const D16U timeout, D32S *errorCode);
typedef D32S (*adm_dim_dpo_cleanDownloadDir_func)(const D16U instanceId, const D16U timeout, D32S *errorCode);
/******** request clean download dir above ***********/

/******** request interupt Download below ***********/
extern D32S adm_dim_dpo_interuptDownload(const D16U instanceId, const D16U timeout, D32S actionType, D32S *errorCode);
typedef D32S (*adm_dim_dpo_interuptDownload_func)(const D16U instanceId, const D16U timeout, D32S actionType, D32S *errorCode);


extern D32S adm_dim_dpo_set_log_level(const D16U instanceId, const D16U timeout, 
        D32U logLevel, D32S* errorCode);
typedef D32S (*adm_dim_dpo_set_log_level_func)(const D16U instanceId, const D16U timeout, 
        D32U logLevel, D32S* errorCode);

/***************************************************************************SOA-DIM-DPO init**********************************************************************************/
/**
 * @brief Dim soa dpo initialization structure definition
 *
 */
typedef struct
{
    ADM_DIM_DOWNLOAD_PROGRESS_NOTIFY_CALLBACK_F downProgressNotifyCallback;
    ADM_DIM_DOWNLOAD_RESULT_NOTIFY_CALLBACK_F downloadResultNotifyCallback;
} ADM_DIM_SOA_DPO_INIT_INFO_T;

/**
 * @brief The DIM-SOA-DPO module initializes the interface
 * @note Use adm_dim_soaDpoDeinit to release the initialization information
 *
 * @param pstInitInfo   [IN]        ADM_DIM_SOA_DPO_INIT_INFO_T*        Dim soa dpo initialization structure
 * @param priv          [IN/OUT]    void*                               Reserved parameters
 * @return D32S         0:successed, Non-0:failed
 */
extern D32S adm_dim_dpo_init(ADM_DIM_SOA_DPO_INIT_INFO_T *pstInitInfo, void *priv);
typedef D32S(*adm_dim_dpo_init_func)(D16U instanceId,ADM_DIM_SOA_DPO_INIT_INFO_T *pstInitInfo,
                                     void *priv);

/**
 * @brief Release initialization information
 * @note Used to release the information initialized by the adm_dim_soaDpoInit function
 */
extern void adm_dim_dpo_deinit(void);
typedef void (*adm_dim_dpo_deinit_func)(D16U instanceId);
/***************************************************************************SOA-DIM-DPO init**********************************************************************************/

#ifdef __cplusplus  // close out "C" linkage in case of c++ compiling
}
#endif

#endif//__ADM_DIM_DPO_H__