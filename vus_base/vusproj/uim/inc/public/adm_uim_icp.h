/**
 * @file adm_uim_icp.h
 * @brief hmi获取icp信息接口
 * @author wangqi
 * @version v0.0.1
 * @date 2025-01-12
 * @copyright chery
 */
#ifndef __ADM_UIM_ICP_H__
#define __ADM_UIM_ICP_H__

#ifdef __cplusplus
extern "C" {
#endif

#define EXPORT_FUN __attribute__((visibility("default")))

#include "adm_typedefs.h"
#include "adm_uim_common.h"
#include "adm_application_common.h"

/***************************************************************************Query icp result****************************************************************************/
/**
 * @brief Used to request to query the currently icp data.
 * @note Output type parameters need to release memory.
 * 
 * @param sessionId         [OUT]    DCHAR**    The session id of the platform response
 * @param state             [OUT]    DCHAR**    Task_State: Successed/Failed
 * @param ppstIcpInfo       [OUT]    icp data.
 * @param errorCode         [OUT]    D32S*  Error code of the reason for failure, 0 for success
 * @param priv                              [IN/OUT]    void*                               Reserved parameters
 * @return  D32S                            0:successed, Non-0:failed
 */
typedef void (*ADM_UIM_ICP_QUERY_RESULT_CALLBACK_F)(DCHAR** sessionId, DCHAR** state, DCHAR **ppstIcpInfo, D32S* errorCode, void* priv);

/**
 * @brief The UIM-SOA-ICP initialization structure definition
 * 
 */
typedef struct ADM_UIM_ICP_INIT_INFO{
    ADM_UIM_ICP_QUERY_RESULT_CALLBACK_F   queryResultCallback;   /**!< Used to request to query the currently icp result. */
} ADM_UIM_ICP_INIT_INFO_T;


/**
 * @brief The SOA-UIM-ICP module initializes the interface 
 */
extern D32S adm_uim_icpInit(const ADM_UIM_ICP_INIT_INFO_T* initInfo, void* priv);
typedef D32S (*adm_uim_icpInit_func)(const ADM_UIM_ICP_INIT_INFO_T* initInfo, void* priv);

/**
 * @brief Release initialization information
 */
extern D32S adm_uim_icpDeinit(void);
typedef D32S (*adm_uim_icpDeinit_func)(void);


#ifdef __cplusplus  // close out "C" linkage in case of c++ compiling
}
#endif

#endif//__ADM_UIM_CHECK_H__
