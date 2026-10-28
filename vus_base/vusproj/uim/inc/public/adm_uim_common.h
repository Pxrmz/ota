/**
 * @file adm_uim_common.h
 * @author mengwei@abupdate.com
 * @brief SOA-UIM模块使用的通用结构头文件，包含：ECU的标识信息、前置条件的配置信息等。
 * @version 0.1
 * @date 2021-05-08
 * 
 * @copyright Copyright © 2018 All rights 上海艾拉比智能科技有限公司
 * 
 */
#ifndef __ADM_UIM_COMMON_H__
#define __ADM_UIM_COMMON_H__

#ifdef __cplusplus
extern "C" {
#endif

#define EXPORT_FUN __attribute__((visibility("default")))

#include "adm_typedefs.h"
#include "adm_application_common.h"

/**
 * @brief Whether the requested trigger action is allowed
 * 
 */
typedef enum {
    ADM_UIM_SOA_ACTION_AUTH_TYPE_ALLOW = 1,         /**!< Allowed */
    ADM_UIM_SOA_ACTION_AUTH_TYPE_NOT_ALLOW,         /**!< Not allowed */
    ADM_UIM_SOA_ACTION_AUTH_TYPE_INVALID            /**!< Invalid */
} ADM_UIM_SOA_ACTION_AUTH_TYPE_E;
typedef enum{
        ADM_UIM_SOA_AUTO_DOWNLOAD_OFF=0,
        ADM_UIM_SOA_AUTO_DOWNLOAD_ON =1,
        ADM_UIM_SOA_AUTO_DOWNLOAD_INVALID
}ADM_UIM_SOA_AUTO_DOWNLOAD_TYPE_E;


typedef enum
{
	ADM_UIM_SOA_TSP_OPERATION_INSTALL_NOW = 1,			///!< 立即升级
    ADM_UIM_SOA_TSP_OPERATION_INSTALL_SCHDULE = 2,      ///!< 预约安装
    ADM_UIM_SOA_TSP_OPERATION_SCHDULE_UPDATE = 3,       ///!< 更新预约
    ADM_UIM_SOA_TSP_OPERATION_SCHDULE_CANCEL = 4,       ///!< 取消预约
} ADM_UIM_SOA_TSP_OPERATION_TYPE_E;

/**
 * @brief Types of condition detection.
 * 
 */
typedef enum {
    ADM_UIM_SOA_CONDITIONS_VERIFIED_TYPE_ONCE = 1,     /**!< Only once */
    ADM_UIM_SOA_CONDITIONS_VERIFIED_TYPE_CYCLE,        /**!< Period */
    ADM_UIM_SOA_CONDITIONS_VERIFIED_TYPE_INVALID       /**!< Invalid */
} ADM_UIM_SOA_CONDITIONS_VERIFIED_TYPE_E;

/**
 * @brief Structure definition used to uniquely identify an ECU.
 * 
 */
typedef struct {
    DCHAR* partNum;             /**!< Part number*/
    D32S requestId;             /**!< Request ID */
    D32S responseId;            /**!< Response ID */
    D32S swDid;                 /**!< The did of the software version, used to distinguish different upgrade parts of the same requestId ECU */
    DCHAR* name;                /**!< Part name */
    DCHAR*  softVer;            /**!< Software version number */
} ADM_UIM_SOA_ECU_IDEN_INFO_T;

/**
 * @brief Type definition of detection trigger.
 * 
 */
// typedef enum {
//     ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_USER = 1,      /**!< User click by HUT */
//     ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_IGN,           /**!< IGN */
//     ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_TSP,           /**!< TSP push */
//     ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_PHONE,         /**!< User click by phone */
//     ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_CYCLE,         /**!< Cycle */
//     ADM_APPLICATION_COMMON_CMDID_TYPE_INVALID                /**!< Invalid */
// } ADM_APPLICATION_COMMON_CMDID_TYPE_E;

// #ifndef __ADM_DIM_SOA_UCL_H__
/* variable define */
typedef struct variable_struct
{
	D16U  data_type;           /*<< data type */
	D16U  data_length;         /*<< data length */
	union V
	{
		D8U   value[1];
		D32U  value_u32;
		D32S  value_s32;
		D64U  value_u64;
		float value_f;
		D8U*  value_s;
	}v;
}variable_st, *variable_pt;
// #endif//__ADM_DIM_SOA_UCL_H__

/**
 * @brief Data definition of condition value
 * 
 */
typedef struct {
    D32S msgId;                     /**!< MsgId of vehicle state */
    variable_pt expectVariable;     /**!< Expect vehicle state */
    variable_pt realVariable;       /**!< Real vehicle state */
    D32S result;
} ADM_UIM_SOA_CONDITION_DATA_T;

/**
 * @brief Condition value data array definition
 * 
 */
typedef struct {
    ADM_UIM_SOA_CONDITION_DATA_T**  condsArray;     /**!< Condition value data array */
    D16U condsArrayLength;                          /**!< Length of condition valud data array */
} ADM_UIM_SOA_CONDITION_DATA_ARRAY_T;

typedef struct
{
	const DCHAR *state;
	const DCHAR *sessionId;
	const ADM_UIM_SOA_CONDITION_DATA_ARRAY_T *condsArrarList;
	ADM_APPLICATION_COMMON_CMDID_TYPE_E eCmdidType;
	D32S verifyResult;
} ADM_UIM_CHECK_CONDITION_RESULT_T;

/**
 * @brief 构建条件检查结果响应报文
 * @note 扩展接口
 * @param[in] pInfo 条件检查结果信息
 * @return DCHAR* 条件检查结果响应报文
 */
extern DCHAR* adm_uim_buildConditionCheckResponseEx(const ADM_UIM_CHECK_CONDITION_RESULT_T* pInfo);

extern DCHAR* adm_uim_buildConditionCheckResponse(const ADM_UIM_CHECK_CONDITION_RESULT_T* pInfo);


#define set_json_err_str(str)                   \
				do{ 								\
					if(NULL == (str)){				\
						(str) = adm_strdup("{\"errorCode\":301}");		 \
						buildRet = ADM_ECODE_FAILED;\
					}								\
				}while(FALSE == TRUE)
				
#define json_add_string(obj, key, value)                    \
	do {														\
		if(NULL == (value)){												\
			break;												\
		}														\
		json_object* pValObj = json_object_new_string(ADM_PRINT_STR(value));	\
		if(pValObj == NULL) {									\
			log_e("new key %s object failed", (key)); 			\
			buildRet = -1;										\
			goto end_build_json;								\
		}														\
		if(json_object_object_add(obj, key, pValObj) != 0) {	\
			log_e("add key %s failed", (key));					\
			buildRet = -1;										\
			goto end_build_json;								\
		}														\
	}while(FALSE == TRUE)
	
#define json_add_func(obj, key, value, func)                \
	do {														\
		json_object* pValObj = (func)(value); 					\
		if(pValObj == NULL) {									\
			log_e("new key %s object failed", (key)); 			\
			buildRet = -1;										\
			goto end_build_json;								\
		}														\
		if(json_object_object_add(obj, key, pValObj) != 0) {	\
			log_e("add key %s failed", (key));					\
			buildRet = -1;										\
			goto end_build_json;								\
		}														\
	}while(FALSE == TRUE)
		
#define json_add_int(obj, key, value) json_add_func(obj, key, value, json_object_new_int)
#define json_add_int64(obj, key, value) json_add_func(obj, key, value, json_object_new_int64)
#define json_add_uint64(obj, key, value) json_add_func(obj, key, value, json_object_new_uint64)
#define json_add_double(obj, key, value) json_add_func(obj, key, value, json_object_new_double)
	//#define json_add_string(obj, key, value) json_add_func(obj, key, value, json_object_new_string)
#define json_add_object(obj, key, value)                                \
				do {														\
					if(json_object_object_add(obj, key, value) != 0) {		\
						log_e("add key %s failed", (key));					\
						buildRet = -1;										\
						goto end_build_json;								\
					}														\
					(value) = NULL;											\
				}while(FALSE == TRUE)
	
#define json_add_array_item(obj, value)                 \
				do {										\
					(void)json_object_array_add(obj, value);		\
					(value) = NULL;							\
				}while(FALSE == TRUE)
	
#define json_new_object(pObj)                                       \
				do{ 													\
					if(((pObj) = json_object_new_object()) == NULL) { 	\
						log_e("json_object_new_object failed"); 		\
						buildRet = -1;									\
						goto end_build_json;							\
					}													\
				}while(FALSE == TRUE)
	
#define json_new_array(pObj, size)                                          \
				do{ 															\
					if(((pObj) = json_object_new_array_ext(size)) == NULL) {		\
						log_e("json_object_new_array_ext failed");				\
						buildRet = -1;											\
						goto end_build_json;									\
					}															\
				}while(FALSE == TRUE)
	
#define json_free_object(pObj)                  \
				do {								\
					if((pObj) != NULL) {				\
						(void)json_object_put(pObj);		\
						(pObj) = NULL;				\
					}								\
				}while(FALSE == TRUE)							\
	
	
#define buildProgressInfo(pObj, pProgress)                                  \
				do{ 															\
					json_add_int64(pObj, "savedBytes", pProgress->savedSize);	\
					json_add_int64(pObj, "totalBytes", pProgress->totalSize);	\
					json_add_int(pObj, "remainTime", pProgress->remainTime);	\
					json_add_int(pObj, "speed", pProgress->speed);				\
                    json_add_int(pObj, "progress", pProgress->iTotalProgress);				\
				}while(FALSE == TRUE)
			
#define buildEcuInfo(pObj, pEcuInfo)                                                \
				do {																	\
					json_add_string(pObj, "ecuPartNum", pEcuInfo->partNum); 			\
					json_add_int(pObj, "ecuReqId",	pEcuInfo->requestId);				\
					json_add_int(pObj, "ecuRespId", pEcuInfo->responseId);				\
					json_add_int(pObj, "ecuSwDid", pEcuInfo->swDid);					\
					json_add_string(pObj, "ecuName", pEcuInfo->name);					 \
					json_add_string(pObj, "ecuSoftVer", pEcuInfo->softVer); 				   \
				}while(FALSE == TRUE)
	
#define buildResult(pObj, pResult)                                           \
				do{ 															 \
					json_add_int(pObj, "result", pResult->result);				 \
					json_add_string(pObj, "expectValue", pResult->expectVal);	 \
					json_add_string(pObj, "realValue", pResult->realVal);		 \
				}while(FALSE == TRUE)
				
#define buildPolicyResult(pObj, pResult)                                           \
				do{ 															 \
					json_add_int(pObj, "result", pResult->result);				 \
					json_add_int(pObj, "existPrivatePolicy", pResult->existPrivatePolicy);				 \
					json_add_string(pObj, "expectValue", pResult->expectVal);	 \
					json_add_string(pObj, "realValue", pResult->realVal);		 \
				}while(FALSE == TRUE)
				
#define buildCommon(pObj, pInfo)                                            \
				do{ 															\
					json_add_int(pObj, "eCmdidType", (int32_t)pInfo->eCmdidType);		\
					json_add_string(pObj, "sessionId", pInfo->sessionId);		\
					json_add_string(pObj, "state", pInfo->state);				\
				}while(FALSE == TRUE)

/**
 * @brief 确保事件订阅成功
 * @note
 * @param in D16U usServiceId 服务ID
 * @param in D16U usEventId 事件ID
 * @return void
 */
void adm_uim_ensureCheckSubedEventSuccess(D16U usServiceId, D16U usEventId);

#ifdef __cplusplus  // close out "C" linkage in case of c++ compiling
}
#endif

#endif//__ADM_UIM_COMMON_H__
