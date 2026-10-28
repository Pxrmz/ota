#ifndef ADM_ARPC_SERVICE_H
#define ADM_ARPC_SERVICE_H

#include "adm_typedefs.h"
#ifdef __cplusplus
extern "C" {
#endif

///< respdata 应用层申请,arpc释放
typedef D32S (*ADM_ARPC_METHOD_CALLBACK_F)(DCHAR* reqData, D32U reqDataSize, DCHAR** respData, D32U* respDataSize);

typedef D32S (*ADM_ARPC_EVENT_CALLBACK_F)(D16U instanceId, DCHAR* data, D32U dataSize);

typedef void (*ADM_ARPC_INSTANCE_BEHAVIOR_CALLBACK_F)(void* priv);

///< publilc -------

/**
 * @brief 初始化arpc, 初始化OTA配置文件
 * 
 * @param filename [in]     配置文件的绝对路径
 * @return D32S     0:Successed, None-0:Failed
 */
D32S adm_arpc_initService(void* priv);

/**
 * @brief 启动arpc服务
 * 
 * @param priv     [in] 	目前没用
 * @return D32S    0:Successed, None-0:Failed
 */
D32S adm_arpc_startService(void* priv); 

/**
 * @brief 停止arpc服务
 * 
 * @param priv     [in] 	目前没用
 * @return D32S    0:Successed, None-0:Failed
 */
D32S adm_arpc_stopService(void);

///< ------for server begin--------

/**
 * @brief   提供服务方法, 注册该方法对应的处理函数
 * 
 * @param instanceId        [IN]   	D16U  instanceId 
 * @param serviceId         [IN]   	D16U  serviceId
 * @param methodId          [IN]   	D16U  methodId 
 * @param methodHandler     [IN]   	ADM_ARPC_METHOD_CALLBACK_F 
*/
void adm_arpc_registerServMethodHandler(D16U instanceId, D16U serviceId, D16U methodId, ADM_ARPC_METHOD_CALLBACK_F methodHandler);

/**
 * @brief   提供事件, 提供后客户端可订阅该事件
 * 
 * @param instanceId        [IN]   	D16U  instanceId 
 * @param serviceId         [IN]   	D16U  serviceId
 * @param methodId          [IN]   	D16U  eventId 
*/
void adm_arpc_offerEvent(D16U instanceId, D16U serviceId, D16U eventId); 

/**
 * @brief   事件通知
 * 
 * @param instanceId        [IN]   	D16U  instanceId 
 * @param serviceId         [IN]   	D16U  serviceId
 * @param methodId          [IN]   	D16U  eventId 
 * @param data          	[IN]   	const DCHAR*  notify data 
 * @param dataSize          [IN]   	D32U  notify dataSize 
*/
void adm_arpc_notifyEvent(D16U instanceId,D16U serviceId, D16U eventId, const DCHAR* data, D32U dataSize); 

///< ---------for server end-------

///< ---------for client begin--------
/**
 * @brief   订阅服务端提供的事件
 * 
 * @param instanceId       [IN]   	D16U  instanceId 
 * @param serviceId        [IN]   	D16U  serviceId
 * @param eventId          [IN]   	D16U  eventId 
*/
D32S adm_arpc_subscriberEvent(D16U instanceId, D16U serviceId, D16U eventId);

/**
 * @brief   服务端通知的事件,注册该事件对应的处理函数
 * 
 * @param instanceId       [IN]   	D16U  instanceId 
 * @param serviceId        [IN]   	D16U  serviceId
 * @param eventId          [IN]   	D16U  eventId 
*/
void adm_arpc_registerEventHandler(D16U instanceId, D16U serviceId, D16U eventId, ADM_ARPC_EVENT_CALLBACK_F eventHandler);

/**
 * @brief   服务请求, 同步接口
 * @note    注意 !!!, respData arpc 申请内存, 应用层释放, 释放函数adm_free
 * 
 * @param instanceId        [IN]   	D16U  instanceId 
 * @param serviceId         [IN]   	D16U  serviceId
 * @param methodId          [IN]   	D16U  methodId 
 * @param timeOut      		[IN]   	D16U  timeOut, The unit is the second
 * @param reqData        	[IN]   	const DCHAR* reqData 
 * @param reqDataSize       [IN]   	D32U reqDataSize
 * @param respData         	[OUT]  	DCHAR** respData. attention !!!,  arpc malloc, app delete
 * @param respDataSize      [OUT]   D32U* respDataSize
 * @return D32S             [OUT]  	D32S returncode
 									0  			///< 成功;
 									2001		///< 实例不在线;
 									2002		///< 实例在线,服务不在线;
 									2003		///< 请求服务超时;
*/
D32S adm_arpc_request_service(D16U instanceId, D16U serviceId, D16U methodId, D16U timeOut, 
										const DCHAR* reqData, D32U reqDataSize, DCHAR** respData, D32U* respDataSize);

///< ----------for client end------------

///< 扩展接口
/**
 * @brief   关键事件, 检查期待的客户端是否已订阅
 * 
 * @param instanceId        [IN]    D16U  instanceId
 * @param serviceId         [IN]    D16U  serviceId
 * @param messageId         [IN]    D16U  eventid
 * @param instanceId      	[IN]    D16U  checked instanceId
 * @return D32S             [OUT]   1:subed, 0:not sub
 */
D32S adm_arpc_checkIfClientSubedEvent(D16U instanceId, D16U serviceId, D16U eventId, D16U clientId);

typedef enum INSTANCE_BEHAVIOR_ENUM
{
    INSTANCE_BEHAVIOR_ONLINE = 1, ///< online
    INSTANCE_BEHAVIOR_OFFLINE,   ///< offline
    INSTANCE_BEHAVIOR_INVALID  ///< INVALID
} INSTANCE_BEHAVIOR_ENUM_E;

/**
 * @brief   对于关注的实例上下线行为, 注册回调处理函数
 * 
 * @param instanceId        [IN]    D16U  instanceId
 * @param serviceId         [IN]    D16U  serviceId
 * @param messageId         [IN]    D16U  eventid
 * @param instanceId      	[IN]    D16U  checked instanceId
 * @return D32S             [OUT]   1:subed, 0:not sub
 */
D32S adm_arpc_registerInstanceBehaviorCb(D16U instanceId, INSTANCE_BEHAVIOR_ENUM_E behavior, ADM_ARPC_INSTANCE_BEHAVIOR_CALLBACK_F findHandler, void* priv);

/**
 * @brief 使用SSL连接时，配置PKCS-8证书私钥的加密口令
 * @note  当SSL使用PCKS-8加密私钥时，通过该接口配置加密口令
 *
 * @param password    [In] DHCAR *     PKCS-8 私钥的加密口令
 */
void adm_arpc_setSSLKeyPasswd(DCHAR *password);

#define ADM_ARPC_DEFAULT_SDK_VERSION      "tools_default_version_1.1.0"       //!< 定义默认的软件版本号

/*!
 *******************************************************************************
 * arpc get software version number
 * \return	version format: ${PROJECT_NAME}_${ENV_PLATFORM}_${ADM_SDK_VERSION}_${COMPILE_DATE}_${GIT_COMMITID})
 *******************************************************************************
 */
DCHAR* adm_arpc_getSdkVersion(void);

#ifdef __cplusplus
}
#endif


#endif // ADM_ARPC_SERVICE_H


