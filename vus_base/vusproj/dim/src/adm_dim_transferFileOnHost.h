/**
 * @file adm_dim_soaTransferFileOnHost.h
 * @brief 为文件传输定义的需要UCM提供的服务化接口，注：ewebserver在宿主设备上。
 * @author Mengwei
 * @version v0.1.0
 * @date 2021-05-07
 * @copyright Copyright © 2018 All rights 上海艾拉比智能科技有限公司
 */
#ifndef __ADM_DIM_SOA_TRANSFER_FILE_ON_HOST_H__
#define __ADM_DIM_SOA_TRANSFER_FILE_ON_HOST_H__
#ifdef __cplusplus
extern "C" {
#endif

#define EXPORT_FUN __attribute__((visibility("default")))

#include "adm_typedefs.h"

/**
 * @brief Definition of file transfer type.
 * 
 */
typedef enum {
    ADM_DIM_SOA_TRANSFER_FILE_ON_HOST_TYPE_LOG = 1,                 /**!< LOG file */
    ADM_DIM_SOA_TRANSFER_FILE_ON_HOST_TYPE_SERVER_ROOT_CERT,        /**!< Server root cert */
    ADM_DIM_SOA_TRANSFER_FILE_ON_HOST_TYPE_CLIENT_CERT,             /**!< Client cert */
    ADM_DIM_SOA_TRANSFER_FILE_ON_HOST_TYPE_CLIENT_PRIV_KEY,         /**!< Client private key */
    ADM_DIM_SOA_TRANSFER_FILE_ON_HOST_TYPE_CDN_SERVER_ROOT_CERT,    /**!< CDN server root cert */
    ADM_DIM_SOA_TRANSFER_FILE_ON_HOST_TYPE_INVALID                  /**!< Invalid */
} ADM_DIM_SOA_TRANSFER_FILE_ON_HOST_TYPE_E;


/***************************************************************************Transfer file Slave download from host**************************************************************************/

/**
 * @brief File transfer, the slave device downloads the corresponding file from the host device.
 * 
 * @param eFileType         [IN]    const ADM_DIM_SOA_TRANSFER_FILE_ON_HOST_TYPE_E      File transfer type.
 * @param url               [IN]    const url       The download address of the file
 * @param size              [IN]    const D64S      The size of the file, unit: bytes
 * @param hash              [IN]    const DCHAR*    HASH value of the file (SHA256 sum)
 * @return D32S             0:successed, Non-0:failed
 */
extern D32S adm_dim_requestTransferFileDownFromServerOnHost(const ADM_DIM_SOA_TRANSFER_FILE_ON_HOST_TYPE_E eFileType, const DCHAR* url, const D64S size, const DCHAR* hash);


/**
 * @brief File transmission, download the progress information structure definition of the file from the host device.
 * 
 */
typedef struct {
    D64S    savedSize;                                          /**!< The size of the downloaded file, unit:byte. */
    D64S    totalSize;                                          /**!< The total size of the file being downloaded, unit:byte */
    D32S    speed;                                              /**!< Download speed of file, unit:byte/s */
    D32S    remainTime;                                         /**!< Estimated remaining time to download, unit:second */
} ADM_DIM_SOA_TRANSFER_FILE_ON_HOST_PROGRESS_INFO_T;

/**
 * @brief File transmission, the progress processing callback function of downloading the file from the host device. 
 * 
 * @param eFileType                 [IN]    const ADM_DIM_SOA_TRANSFER_FILE_ON_HOST_TYPE_E      File transfer type.
 * @param pstTransferProgressInfo   [IN]    const ADM_DIM_SOA_TRANSFER_FILE_ON_HOST_PROGRESS_INFO_T*    File transmission, download the progress information structure definition of the file from the host device.
 * @param priv                      [IN/OUT]    void*                               Reserved parameters
 * @return  D32S             0:successed, Non-0:failed
 * 
 */
typedef D32S (*ADM_DIM_SOA_TRANSFER_FILE_PROGRESS_NOTIFY_DOWN_FROM_SERVER_ON_HOST_CALLBACK_F)(const ADM_DIM_SOA_TRANSFER_FILE_ON_HOST_TYPE_E eFileType, const ADM_DIM_SOA_TRANSFER_FILE_ON_HOST_PROGRESS_INFO_T* pstTransferProgressInfo, void* priv);

/**
 * @brief Result structure definition of file transfer.
 * 
 */
typedef struct {
    D32S result;                                                /**!< The result of the transfering the upgrade package */
    DCHAR*  expectVal;                                          /**!< Description of the reason for the failure, expected value */
    DCHAR*  realVal;                                            /**!< Description of the reason for the failure, expected value */
} ADM_DIM_SOA_TRANSFER_FILE_ON_HOST_RESULT_INFO_T;

/**
 * @brief File transmission, the result processing callback function of downloading the file from the host device. 
 * 
 * @param eFileType                 [IN]    const ADM_DIM_SOA_TRANSFER_FILE_ON_HOST_TYPE_E      File transfer type.
 * @param pstTransferResultInfo     [IN]    const ADM_DIM_SOA_TRANSFER_FILE_ON_HOST_RESULT_INFO_T*    File transmission, download the result information structure definition of the file from the host device.
 * @param priv                      [IN/OUT]    void*                               Reserved parameters
 * @return  D32S                    0:successed, Non-0:failed
 * 
 */
typedef D32S (*ADM_DIM_SOA_TRANSFER_FILE_RESULT_SEND_DOWN_FROM_SERVER_ON_HOST_CALLBACK_F)(const ADM_DIM_SOA_TRANSFER_FILE_ON_HOST_TYPE_E eFileType, const ADM_DIM_SOA_TRANSFER_FILE_ON_HOST_RESULT_INFO_T* pstTransferResultInfo, void* priv);


/**
 * @brief File transfer, the user queries the transfer progress information of the current file.
 * 
 * @param eFileType                 [IN]    const ADM_DIM_SOA_TRANSFER_FILE_ON_HOST_TYPE_E      File transfer type.
 * @param ppstTransferProgressInfo  [OUT]   ADM_DIM_SOA_TRANSFER_FILE_ON_HOST_PROGRESS_INFO_T**    File transmission, download the progress information structure definition of the file from the host device.
 * @return D32S                     0:successed, Non-0:failed
 */
extern D32S adm_dim_getTransferFileProgressDownFromServerOnHost(const ADM_DIM_SOA_TRANSFER_FILE_ON_HOST_TYPE_E eFileType, ADM_DIM_SOA_TRANSFER_FILE_ON_HOST_PROGRESS_INFO_T** ppstTransferProgressInfo);

/**
 * @brief File transfer, the user queries the transfer result information of the current file.
 * 
 * @param eFileType                 [IN]    const ADM_DIM_SOA_TRANSFER_FILE_ON_HOST_TYPE_E      File transfer type.
 * @param ppstTransferResultInfo    [OUT]    ADM_DIM_SOA_TRANSFER_FILE_ON_HOST_RESULT_INFO_T**    File transmission, download the result information structure definition of the file from the host device.
 * @return D32S                     0:successed, Non-0:failed
 */
extern D32S adm_dim_getTransferFileResultDownFromServerOnHost(const ADM_DIM_SOA_TRANSFER_FILE_ON_HOST_TYPE_E eFileType, ADM_DIM_SOA_TRANSFER_FILE_ON_HOST_RESULT_INFO_T** ppstTransferResultInfo);

/***************************************************************************Transfer file Slave download from host**************************************************************************/

/***************************************************************************Transfer file Slave upload to host******************************************************************************/

/**
 * @brief File transfer, request the Slave device to upload the file to the Host device.
 * 
 * @param eFileType                 [IN]    const ADM_DIM_SOA_TRANSFER_FILE_ON_HOST_TYPE_E      File transfer type.
 * @param url                       [IN]    const DCHAR*    Uploaded address
 * @param size                      [OUT]   D64S*   Upload file size
 * @param hash                      [OUT]   DCHAR** HASH value of the uploaded file (SHA256 sum)
 * @param filename                  [OUT]   DCHAR** Filename of the uploaded file
 * @return D32S                     0:successed, Non-0:failed
 */
extern D32S adm_dim_requestTransferFileUploadToServerOnHost(const ADM_DIM_SOA_TRANSFER_FILE_ON_HOST_TYPE_E eFileType, const DCHAR* url, D64S* size, DCHAR** hash, DCHAR** filename);

/**
 * @brief File transmission, the progress processing callback function of uploading the file to the host device. 
 * 
 * @param eFileType                 [IN]    const ADM_DIM_SOA_TRANSFER_FILE_ON_HOST_TYPE_E      File transfer type.
 * @param pstTransferProgressInfo   [IN]    const ADM_DIM_SOA_TRANSFER_FILE_ON_HOST_PROGRESS_INFO_T*    File transmission, upload the progress information structure definition of the file from the host device.
 * @param priv                      [IN/OUT]    void*                               Reserved parameters
 * @return  D32S                    0:successed, Non-0:failed
 */
typedef D32S (*ADM_DIM_SOA_TRANSFER_FILE_PROGRESS_NOTIFY_UPLOAD_TO_SERVER_ON_HOST_CALLBACK_F)(const ADM_DIM_SOA_TRANSFER_FILE_ON_HOST_TYPE_E eFileType, const ADM_DIM_SOA_TRANSFER_FILE_ON_HOST_PROGRESS_INFO_T* pstTransferProgressInfo, void* priv);

/**
 * @brief File transmission, the result processing callback function of uploading the file to the host device. 
 * 
 * @param eFileType                 [IN]    const ADM_DIM_SOA_TRANSFER_FILE_ON_HOST_TYPE_E      File transfer type.
 * @param pstTransferResultInfo     [IN]    const ADM_DIM_SOA_TRANSFER_FILE_ON_HOST_RESULT_INFO_T*   File transmission, upload the result information structure definition of the file from the host device.
 * @param priv                      [IN/OUT]    void*                               Reserved parameters
 * @return  D32S                    0:successed, Non-0:failed
 */
typedef D32S (*ADM_DIM_SOA_TRANSFER_FILE_RESULT_SEND_UPLOAD_TO_SERVER_ON_HOST_CALLBACK_F)(const ADM_DIM_SOA_TRANSFER_FILE_ON_HOST_TYPE_E eFileType, const ADM_DIM_SOA_TRANSFER_FILE_ON_HOST_RESULT_INFO_T* pstTransferResultInfo, void* priv);

/**
 * @brief File transfer, the user queries the transfer progress information of the current file.
 * 
 * @param eFileType                 [IN]    const ADM_DIM_SOA_TRANSFER_FILE_ON_HOST_TYPE_E      File transfer type.
 * @param ppstTransferProgressInfo  [OUT]   ADM_DIM_SOA_TRANSFER_FILE_ON_HOST_PROGRESS_INFO_T**     File transmission, upload the progress information structure definition of the file from the host device.
 * @return D32S                     0:successed, Non-0:failed
 */
extern D32S adm_dim_getTransferFileProgressUploadToServerOnHost(const ADM_DIM_SOA_TRANSFER_FILE_ON_HOST_TYPE_E eFileType, ADM_DIM_SOA_TRANSFER_FILE_ON_HOST_PROGRESS_INFO_T** ppstTransferProgressInfo);

/**
 * @brief File transfer, the user queries the transfer result information of the current file.
 * 
 * @param eFileType                 [IN]    const ADM_DIM_SOA_TRANSFER_FILE_ON_HOST_TYPE_E      File transfer type.
 * @param ppstTransferResultInfo    [OUT]   ADM_DIM_SOA_TRANSFER_FILE_ON_HOST_RESULT_INFO_T**   File transmission, upload the result information structure definition of the file from the host device.
 * @return D32S                     0:successed, Non-0:failed
 */
extern D32S adm_dim_getTransferFileResultUploadToServerOnHost(const ADM_DIM_SOA_TRANSFER_FILE_ON_HOST_TYPE_E eFileType, ADM_DIM_SOA_TRANSFER_FILE_ON_HOST_RESULT_INFO_T** ppstTransferResultInfo);

/***************************************************************************Transfer file Slave upload to host******************************************************************************/


/***************************************************************************SOA-DIM-TRANSFER-FILE server on host init***********************************************************************/

/**
 * @brief DIM-SOA-TRANSFER-FILE-ON-HOST initialization structure definition
 * 
 */
typedef struct {
    ADM_DIM_SOA_TRANSFER_FILE_PROGRESS_NOTIFY_DOWN_FROM_SERVER_ON_HOST_CALLBACK_F   downloadProgressNotifyCallback;     /**!< File transmission, the progress processing callback function of downloading the file from the host device.  */
    ADM_DIM_SOA_TRANSFER_FILE_RESULT_SEND_DOWN_FROM_SERVER_ON_HOST_CALLBACK_F       downloadResultSendCallback;         /**!< File transmission, the result processing callback function of downloading the file from the host device.  */
    ADM_DIM_SOA_TRANSFER_FILE_PROGRESS_NOTIFY_UPLOAD_TO_SERVER_ON_HOST_CALLBACK_F   uploadProgressNotifyCallback;       /**!< File transmission, the progress processing callback function of uploading the file to the host device.  */
    ADM_DIM_SOA_TRANSFER_FILE_RESULT_SEND_UPLOAD_TO_SERVER_ON_HOST_CALLBACK_F   uploadResultSendCallback;               /**!< File transmission, the result processing callback function of uploading the file to the host device.  */

} ADM_DIM_SOA_TRANSFER_FILE_ON_HOST_INIT_INFO_T;


/**
 * @brief The DIM-SOA-TRANSFER-FILE-ON-HOST module initializes the interface.
 * @note Use adm_dim_soa_transferFileOnHost_deinit to release the initialization information
 * 
 * @param initInfo  [IN]    const ADM_DIM_SOA_TRANSFER_FILE_ON_HOST_INIT_INFO_T*    DIM-SOA-TRANSFER-FILE-ON-HOST initialization structure
 * @param priv      [IN/OUT]    void*           Reserved parameters
 * @return D32S     0:successed, Non-0:failed
 */
extern D32S adm_dim_soa_transferFileOnHost_init(D16U instanceId, const ADM_DIM_SOA_TRANSFER_FILE_ON_HOST_INIT_INFO_T* initInfo, void* priv);

/**
 * @brief Release initialization information
 * @note Used to release the information initialized by the adm_dim_soa_transferFileOnHost_init function
 */
extern void adm_dim_soa_transferFileOnHost_deinit(D16U instanceId);

/***************************************************************************SOA-DIM-TRANSFER-FILE server on host init***********************************************************************/


#ifdef __cplusplus  // close out "C" linkage in case of c++ compiling
}
#endif

#endif//__ADM_DIM_SOA_TRANSFER_FILE_ON_HOST_H__