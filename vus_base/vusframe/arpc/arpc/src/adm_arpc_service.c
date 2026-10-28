#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include "adm_map.h"
#include "adm_string.h"
#include "adm_list.h"
#include "adm_typedefs.h"
#include "adm_memory.h"

#include "json_types.h"
#include "json_object.h"
#include "json.h"
#include "adm_cfg_api.h"
#include "adm_arpc_log.h"
#include "adm_com_errorCode.h"

#include "adm_arpc_service.h"
#include "adm_arpc_buffer.h"
#include "adm_arpc_server.h"
#include "adm_arpc_internal.h"
#include "adm_arpc_protocol.h"
#if defined(ARPC_SSL)
#include "adm_arpc_ssl.h"
#endif
#include "adm_arpc_threadCond.h"
// server
//服务端回调函数map, key: instaceid,serviceId,methodId, value: func,method callback
static root_t* mehtodHandlers = NULL;

//客户端信息map, key: instaceid,serviceId,methodId, value: instanceId
static root_t* subEventClients = NULL;
pthread_mutex_t subEventClientsMutex = PTHREAD_MUTEX_INITIALIZER;

//客户端回调函数map key: instaceid,serviceId,methodId, value: func,event callback
static root_t* eventHandlers = NULL;

pthread_mutex_t peerInfosMutex = PTHREAD_MUTEX_INITIALIZER;
//ip端口等通信信息map
static root_t* peerInfos = NULL;

static D32S trySubEventCount = 0;

// ADM_ARPC_SUBEVENT_NODE, 记录已订阅的事件, 段线后加入waitForSubEvent, 等待重连成功自动请求订阅
static struct list_head subedEvent;
pthread_mutex_t subedEventMutex = PTHREAD_MUTEX_INITIALIZER;

// 等待订阅的事件, map<instanceId, serviceId & eventId>. 1. 未等对端发布事件, 订阅失败. 2. 对端掉线, 待重连, 重新请求; 超时后删除
static struct list_head waitSubEvent; 
// 多个客户端同时心跳, 同时触发重复订阅
pthread_mutex_t waitSubEventMutex = PTHREAD_MUTEX_INITIALIZER;


// 实例在线回调
static struct list_head instanceBehaviorHandler;

typedef struct ADM_ARPC_EVENT_INFO
{
    D16U instanceId;
    D16U serviceId;
    D16U eventId;
}ADM_ARPC_EVENT_INFO_T;

typedef struct ADM_ARPC_SUBEVENT_NODE
{
    ADM_ARPC_EVENT_INFO_T* pstEventInfo;
    struct list_head entry;
	D32S retryCount;
}ADM_ARPC_SUBEVENT_NODE_T;

typedef struct ADM_ARPC_MSGLIST
{
	//ADM_ARPC_PEER_T *pstMsg;
	D16U instanceId;
	struct list_head entry; /** 指向下一个节点 */
} ADM_ARPC_MSGLIST_T;

typedef struct ADM_ARPC_CHILD_INSTANCE_LIST
{
	D16U instanceId;
	struct list_head entry; /** 指向下一个节点 */
} ADM_ARPC_CHILD_INSTANCE_LIST_T;

// 实例在线回调

typedef struct ADM_INSTANCE_BEHAVIOR_CB
{
    D16U instanceId;
    D16S callCount;
	INSTANCE_BEHAVIOR_ENUM_E behavior;
    ADM_ARPC_INSTANCE_BEHAVIOR_CALLBACK_F handler;
    void* priv;
    struct list_head entry;
}ADM_INSTANCE_BEHAVIOR_CB_T;

static ADM_ARPC_MSGLIST_T *adm_arpc_msgListNew(void)
{
    ADM_ARPC_MSGLIST_T *pstMsgList = NULL;	
    
    pstMsgList = (ADM_ARPC_MSGLIST_T *)adm_malloc(sizeof(ADM_ARPC_MSGLIST_T));
    if (NULL == pstMsgList) 
	{
        log_e("List malloc failed");
        return NULL;
    }

	(void)adm_memset(pstMsgList, 0x00, sizeof(ADM_ARPC_MSGLIST_T),sizeof(ADM_ARPC_MSGLIST_T));
	pstMsgList->instanceId = 0;
    pstMsgList->entry.next = NULL;
    pstMsgList->entry.prev = NULL;
    
    return pstMsgList;
}

D32S adm_arpc_initService(void* priv)
{
	D32S iRet = ADM_ECODE_FAILED;

	do
	{
		mehtodHandlers = map_new();
		subEventClients = map_new();
		eventHandlers = map_new();
		peerInfos = map_new();

		INIT_LIST_HEAD(&subedEvent);

		INIT_LIST_HEAD(&waitSubEvent);
		
		INIT_LIST_HEAD(&instanceBehaviorHandler);

		if(priv != NULL)
		{
			log_i("host InstanceId:%d", *(D16U*)priv);
			adm_arpc_setHostInstnceId(*(D16U*)priv);
		}
		else
		{
			D32S hostMode = adm_cfg_iniParserGetInt(ADM_CFG_VUS_DIS_TCP_MODE, ADM_CFG_VUS_DIS_TCP_MODE_DEFAULT_VAL);
			if(HOSTMODE_SERVER == (hostMode & HOSTMODE_SERVER) )
			{
				D16U instanceId = adm_arpc_getHostInstanceId();
				adm_arpc_setHostInstnceId(instanceId+1);
			}
		}

		trySubEventCount = adm_cfg_iniParserGetInt(ADM_CFG_VUS_DIS_SOC_TRY_CONNECT_TIME, (D32S)ADM_CFG_VUS_DIS_SOC_TRY_CONNECT_TIME_DEFAULT_VAL);

		iRet = adm_arpc_initserver();
		if( ADM_ECODE_SUCCESSED != iRet )
	    {
	    	log_e("arpc init failed, fail code: %d", iRet);
	    	break;
	    }

		pthread_mutex_init(&subEventClientsMutex, DNULL);
		pthread_mutex_init(&peerInfosMutex, DNULL);
		pthread_mutex_init(&subedEventMutex, DNULL);
		pthread_mutex_init(&waitSubEventMutex, DNULL);

		iRet = ADM_ECODE_SUCCESSED;

		log_i("arpc version:%s", adm_arpc_getSdkVersion());
		log_i("arpc init successed");
	}while(false);	
	
	return iRet;
}

D32S adm_arpc_startService(void* priv)
{
	D32S iRet = ADM_ECODE_FAILED;

	do
	{
		iRet = adm_arpc_startServer();
		if(ADM_ECODE_SUCCESSED != iRet)
		{
			log_e("arpc start failed, fail code: %d", iRet);
			break;
		}

		iRet = ADM_ECODE_SUCCESSED;
		log_i("arpc start successed");
	}while(false);
	
	return iRet;
}
D32S adm_arpc_stopService(void)
{
	log_i("adm_arpc_stopService begin");
	map_free(mehtodHandlers);
	map_free(subEventClients);
	map_free(eventHandlers);
	map_free(peerInfos);

	ADM_ARPC_SUBEVENT_NODE_T *p, *n;
    list_for_each_entry_safe(p, n, &subedEvent, ADM_ARPC_SUBEVENT_NODE_T, entry)
	{
	    list_del(&p->entry);
		adm_free(p);
	}

    list_for_each_entry_safe(p, n, &waitSubEvent, ADM_ARPC_SUBEVENT_NODE_T, entry)
	{
	    list_del(&p->entry);
		adm_free(p);
	}

	adm_arpc_stopServer();

	pthread_mutex_destroy(&subEventClientsMutex);
	pthread_mutex_destroy(&peerInfosMutex);
	pthread_mutex_destroy(&subedEventMutex);
	pthread_mutex_destroy(&waitSubEventMutex);

	log_i("adm_arpc_stopService end");

	return ADM_ECODE_SUCCESSED;
}

static void adm_arpc_getTwiceId(D16U instanceId, D16U serviceId, D16U methodId, DCHAR* strId)
{	
	if(strId == NULL)
	{
		log_i("strId error");
		return ;
	}

	(void)sprintf(strId, "%04X%04X%04X",instanceId, serviceId, methodId);
}

static void adm_arpc_add_child_instance(ADM_ARPC_PEER_T* peer, D16U instanceId)
{
	if(NULL == peer->childInstancelist)
	{
		log_e("peer->childInstancelist is null");
		return;
	}
	
	ADM_ARPC_CHILD_INSTANCE_LIST_T *p;
	list_for_each_entry(p, peer->childInstancelist, ADM_ARPC_CHILD_INSTANCE_LIST_T, entry)
	{
		if(p->instanceId == instanceId)
		{
			log_i("peer[%04X] already has instance[%04X]", peer->instanceId, instanceId);
			return;
		} 
	}
	
	ADM_ARPC_CHILD_INSTANCE_LIST_T* childInstance = (ADM_ARPC_CHILD_INSTANCE_LIST_T*)adm_malloc(sizeof(ADM_ARPC_CHILD_INSTANCE_LIST_T));
	if(childInstance == NULL)
	{
		log_e("malloc eventNode fail");
		return;
	}
	(void)adm_memset(childInstance, 0x0, sizeof(ADM_ARPC_CHILD_INSTANCE_LIST_T), sizeof(ADM_ARPC_CHILD_INSTANCE_LIST_T));

	childInstance->instanceId = instanceId;	
	list_add_tail(&childInstance->entry, peer->childInstancelist);

	log_i("peer:%p,childlist:%p, add child instance: 0X%04X", peer, peer->childInstancelist,instanceId);
	
}

static void adm_arpc_retrySubEvent_handler(DCHAR* respData, D32U dataSize)
{
	if(respData == NULL)
    {
    	log_w("subMultiEcent resp is null");
    	return;
    }

    json_object* respObj = json_tokener_parse(respData);
	struct json_object* subEventInfo = json_object_object_get(respObj,"subeventInfo");
	
	if (json_object_get_type(subEventInfo) != json_type_array)
	{
		(void)json_object_put(respObj);
		log_i("this is is an error array json!, subinfo:%s", respData);
		// resp error;

		return;
	}
	else
	{
		size_t eventlen = json_object_array_length(subEventInfo);
		D16U instanceId = 0;
		D16U serviceId = 0;
		D16U eventId = 0;
		D16U flag = 0;
		json_object *eventObj = NULL;
		for (size_t i = 0; i < eventlen; i++)
		{
			eventObj = json_object_array_get_idx(subEventInfo, i);
			instanceId = (D16U)(json_object_get_int(json_object_object_get(eventObj,"instanceId")));
			serviceId = (D16U)(json_object_get_int(json_object_object_get(eventObj,"serviceId")));
			eventId = (D16U)(json_object_get_int(json_object_object_get(eventObj,"eventId")));
			flag = (D16U)(json_object_get_int(json_object_object_get(eventObj,"flag")));

			if( (D16U)1 == flag )
			{
				ADM_ARPC_SUBEVENT_NODE_T *p, *n;

				pthread_mutex_lock(&waitSubEventMutex);
			    list_for_each_entry_safe(p, n, &waitSubEvent, ADM_ARPC_SUBEVENT_NODE_T, entry)
				{
				    if(p->pstEventInfo != NULL && p->pstEventInfo->instanceId == instanceId && p->pstEventInfo->serviceId == serviceId && p->pstEventInfo->eventId == eventId)
				    {
				    	log_i("retry sub instanceId = 0X%04X, serviceId = 0X%04X,eventId = 0X%04X successed",instanceId,serviceId, eventId);
				    	list_del(&p->entry);
						adm_free(p);
				    }
				}
				pthread_mutex_unlock(&waitSubEventMutex);

				// 加入已订阅事件列表
				adm_arpc_addSubedEvent(instanceId, serviceId, eventId);
			}
		}

		(void)json_object_put(respObj);
	}

    return;
}


// extend
void adm_arpc_callInstanceBehaviorCb(ADM_ARPC_PEER_T* peer, INSTANCE_BEHAVIOR_ENUM_E behavior)
{
	if(peer == NULL)
	{
		return;
	}
	
	ADM_INSTANCE_BEHAVIOR_CB_T *p;
	list_for_each_entry(p, &instanceBehaviorHandler, ADM_INSTANCE_BEHAVIOR_CB_T, entry)
	{
	    if(p->instanceId == peer->instanceId && p->behavior == behavior && p->handler != NULL )
	    {
			if(behavior == INSTANCE_BEHAVIOR_ONLINE && p->callCount != 0)
			{
		    	continue;
			}
			
			p->handler(p->priv);
			log_i("call instatnceId 0X%04X handler, behavior:%d,  priv:%p ",peer->instanceId, behavior, p->priv ); 
	    	p->callCount += 1;
	    }
	}

	if(NULL == peer->childInstancelist)
	{
		log_e("peer->childInstancelist is null");
		return;
	}
	
	// child instance
    ADM_ARPC_CHILD_INSTANCE_LIST_T *m;
    list_for_each_entry(m, peer->childInstancelist, ADM_ARPC_CHILD_INSTANCE_LIST_T, entry)
	{
		ADM_INSTANCE_BEHAVIOR_CB_T *child_p;
		list_for_each_entry(child_p, &instanceBehaviorHandler, ADM_INSTANCE_BEHAVIOR_CB_T, entry)
		{
		    if(child_p->instanceId == m->instanceId && child_p->behavior == behavior 
				&& child_p->handler != NULL )
		    {
				if(behavior == INSTANCE_BEHAVIOR_ONLINE && child_p->callCount != 0)
				{
			    	continue;
				}
				
				child_p->handler(child_p->priv);
				log_w("call instatnceId 0X%04X handler, behavior:%d,  priv:%p ",m->instanceId, behavior, child_p->priv ); 
		    	child_p->callCount += 1;
		    }
		}
    }

    return;

}

void adm_arpc_clearInstanceCb(ADM_ARPC_PEER_T* peer, INSTANCE_BEHAVIOR_ENUM_E behavior)
{
	ADM_INSTANCE_BEHAVIOR_CB_T *p;
	list_for_each_entry(p, &instanceBehaviorHandler, ADM_INSTANCE_BEHAVIOR_CB_T, entry)
	{
	    if(p->instanceId == peer->instanceId && p->behavior == behavior &&p->handler != NULL)
	    {
	    	log_i("instatnceId 0X%04X , clear online handler, behavior:%d ",peer->instanceId, behavior);
	    	p->callCount = 0;
	    }
	}

	if(NULL == peer->childInstancelist)
	{
		log_e("peer->childInstancelist is null");
		return;
	}

	// child instance
    ADM_ARPC_CHILD_INSTANCE_LIST_T *m;
    list_for_each_entry(m, peer->childInstancelist, ADM_ARPC_CHILD_INSTANCE_LIST_T, entry)
	{
		ADM_INSTANCE_BEHAVIOR_CB_T *child_p;
		list_for_each_entry(child_p, &instanceBehaviorHandler, ADM_INSTANCE_BEHAVIOR_CB_T, entry)
		{
		    if(child_p->instanceId == m->instanceId && child_p->behavior == behavior && child_p->handler != NULL)
		    {
		    	log_i("instatnceId 0X%04X , clear online handler, behavior:%d ", m->instanceId, behavior);
		    	child_p->callCount = 0;
		    }
		}
    }

    return;

}


// server
void adm_arpc_registerServMethodHandler(D16U instanceId, D16U serviceId, D16U methodId, ADM_ARPC_METHOD_CALLBACK_F methodHandler)
{
	DCHAR strKey[13] = {0};
	adm_arpc_getTwiceId(instanceId, serviceId, methodId,strKey);

	log_i("strkey:%s", strKey);	
	bool iet = map_put(mehtodHandlers, strKey, &methodHandler,(D32U)sizeof(methodHandler));
	if(iet == false)
	{
		log_i("Map mehtodHandlers put error!");
		return ;
	}
	
	log_i("register servMethodHandler, instanceId:0X%04X, serviceId:0X%04X, methodId:0X%04X success",instanceId, serviceId, methodId );

    return;
}

void adm_arpc_offerEvent(D16U instanceId, D16U serviceId, D16U eventId)
{
	DCHAR strKey[13] = {0};
	adm_arpc_getTwiceId(instanceId,serviceId,eventId, strKey);
	
	struct list_head* subedClientList = NULL;
	subedClientList = adm_malloc(sizeof(struct list_head));
	(void)adm_memset(subedClientList, 0x00, sizeof(struct list_head),sizeof(struct list_head));
	INIT_LIST_HEAD(subedClientList);

	pthread_mutex_lock(&subEventClientsMutex);
	bool iet = map_put(subEventClients, strKey, &subedClientList,(D32U)sizeof(char*));

	if(iet == false)
	{
		pthread_mutex_unlock(&subEventClientsMutex);
		log_e("Map subEventClients put error!\n");
		return;
	}
	pthread_mutex_unlock(&subEventClientsMutex);

	log_i("offerevent: instanceId: 0X%04X, serviceid: 0X%04X, eventId: 0X%04X",instanceId, serviceId, eventId);

    return;
}

void adm_arpc_notifyEvent(D16U instanceId,D16U serviceId, D16U eventId, const DCHAR* data, D32U dataSize)
{
	DCHAR strKey[13] = {0};
	adm_arpc_getTwiceId(instanceId,serviceId,eventId, strKey);
	D32U valueSize = 8;
	map_t* node = NULL;
	int ishavekey = 0;

	DCHAR strinstanceId[5] = {0};
	DCHAR strserviceId[5] = {0};
	DCHAR strEventId [5] = {0};

	D16U servInstanceId = 0;
	D16U servServiceId = 0;
	D16U servEventId = 0;

	pthread_mutex_lock(&subEventClientsMutex);
    for (node = map_first(subEventClients); node != NULL; node = map_next(node)) 
	{
		if(0 == adm_strcmp(strKey,map_get_key(node)))
		{
			(void)adm_memcpy(strinstanceId, strKey, 4, 4);
			(void)adm_memcpy(strserviceId, strKey+4, 4, 4);
			(void)adm_memcpy(strEventId, strKey+8, 4, 4);

			errno = 0;
			servInstanceId = (D16U)(strtol(strinstanceId, NULL, 16));
			if(0 != errno)
			{
				log_e("strtol, errno=%d, reason=%s",errno, strerror(errno));
			}

			errno = 0;
			servServiceId = (D16U)(strtol(strserviceId, NULL, 16));
			if(0 != errno)
			{
				log_e("strtol, errno=%d, reason=%s",errno, strerror(errno));
			}

			errno = 0;
			servEventId = (D16U)(strtol(strEventId, NULL, 16));
			if(0 != errno)
			{
				log_e("strtol, errno=%d, reason=%s",errno, strerror(errno));
			}
				
			ishavekey = 1;

			log_i("adm_arpc_notifyEvent, %s, 0X%04X, 0X%04X, 0X%04X", strKey, servInstanceId, servServiceId, servEventId);
		}
    }
	if(ishavekey == 1)
	{
		struct list_head** EventClients = (struct list_head**)map_get_val(subEventClients,strKey,&valueSize);
		if(*EventClients != NULL)
		{
			if(1 == list_empty(*EventClients))
			{	
				pthread_mutex_unlock(&subEventClientsMutex);			
				log_i("no client subscriber this instanceId: 0X%04X, serviceid: 0X%04X, eventId: 0X%04X",instanceId, serviceId, eventId);
				return;
			}
			
			ADM_ARPC_MSGLIST_T *pstPos  = NULL;
		    ADM_ARPC_MSGLIST_T *pstNext = NULL;
			list_for_each_entry_safe (pstPos,pstNext, *EventClients, ADM_ARPC_MSGLIST_T, entry) 
		    {
				// get instanceId

				ADM_ARPC_PEER_T* peer = adm_arpc_getPeerInfoByInstanceId(pstPos->instanceId);			
				// (void)adm_arpc_pushToSendQueue(reqPackage,peer,0,NULL);
				if (DNULL != peer)
				{
					ADM_ARPC_PACKAGE_T* reqPackage = adm_arpc_makePackage(servInstanceId,pstPos->instanceId,serviceId,eventId,
																		  0,ADM_ARPC_MESSAGE_TYPE_EVENT_NOTIFY,data,dataSize);
					adm_arpc_add2SendQueue(reqPackage,peer,0,NULL);
					log_i("adm_arpc_releasePeer");
					adm_arpc_releasePeer(peer);
				}
				/*
		    	if(NULL != pstPos->pstMsg && (D8U)0 == pstPos->pstMsg->free)
		    	{
		    		ADM_ARPC_PACKAGE_T* reqPackage = adm_arpc_makePackage(servInstanceId,pstPos->pstMsg->instanceId,serviceId,eventId,
																0,ADM_ARPC_MESSAGE_TYPE_EVENT_NOTIFY,data,dataSize);
					
					(void)adm_arpc_pushToSendQueue(reqPackage,pstPos->pstMsg,0,NULL);
		    	}
		    	*/
			}
		
		}
	}
	else
	{
		log_i("not offer this instanceId: 0X%04X,serviceid: 0X%04X, eventId: 0X%04X", instanceId, serviceId, eventId);
	}
	pthread_mutex_unlock(&subEventClientsMutex);
}

// client method
D32S adm_arpc_subscriberEvent(D16U instanceId, D16U serviceId, D16U eventId)
{
	D32S iRet = ADM_ECODE_FAILED;
	ADM_ARPC_PEER_T* peerinfo = adm_arpc_getPeerInfoByInstanceId(instanceId);
	if(peerinfo == NULL)
	{
		log_e("service is offline, subscriberEvent fail,  instanceId:0X%04X,serviceId:0X%04X,event:0X%04X ", instanceId, serviceId, eventId);
		adm_arpc_addWaitSubEvent(instanceId, serviceId, eventId);
		return iRet;
	}

	// 查找是否已订阅
	ADM_ARPC_SUBEVENT_NODE_T *p;
	pthread_mutex_lock(&subedEventMutex);
	list_for_each_entry(p, &subedEvent, ADM_ARPC_SUBEVENT_NODE_T, entry)
	{
	    if(p->pstEventInfo != NULL)
	    {
	    	if(p->pstEventInfo->instanceId == instanceId && 
	    		p->pstEventInfo->serviceId == serviceId && 
	    		p->pstEventInfo->eventId == eventId)
	    	{
				pthread_mutex_unlock(&subedEventMutex);
				log_i("adm_arpc_releasePeer");
				adm_arpc_releasePeer(peerinfo);
				iRet =	ADM_ECODE_SUCCESSED;
				log_i("subscriberEvent instanceId:0X%04X,serviceId:0X%04X,event:0X%04X ok", instanceId, serviceId, eventId);
	    		return iRet;
	    	}
	    }
	}
    pthread_mutex_unlock(&subedEventMutex);

    ADM_ARPC_PACKAGE_T* reqPackage = adm_arpc_makePackageDef(instanceId,serviceId,eventId,0,ADM_ARPC_MESSAGE_TYPE_SUBEVENT,NULL,0);

	pthread_mutex_t syncMutex;
	pthread_cond_t syncCond;
	pthread_condattr_t condAttr;
	(void)pthread_mutex_init(&syncMutex, NULL);
	(void)pthread_condattr_setclock(&condAttr, CLOCK_MONOTONIC);
	(void)pthread_cond_init(&syncCond, &condAttr);
	
	DCHAR* respData = NULL;
	D32U dataSize = 0;

	ADM_ARPC_CLIENT_PRIVDATA_T* priv = (ADM_ARPC_CLIENT_PRIVDATA_T*)adm_malloc(sizeof(ADM_ARPC_CLIENT_PRIVDATA_T));
	(void)adm_memset(priv, 0x0, sizeof(ADM_ARPC_CLIENT_PRIVDATA_T), sizeof(ADM_ARPC_CLIENT_PRIVDATA_T));

	priv->syncMutex = syncMutex;
	priv->syncCond = syncCond;
	priv->respData = &respData;
	priv->dataSize = &dataSize;
    priv->bSyncIsSignal = DFALSE;
	priv->bCallerTimeout = DFALSE; 

    // (void)adm_arpc_pushToSendQueue(reqPackage,peerinfo,1,priv);
	adm_arpc_add2SendQueue(reqPackage,peerinfo,1,priv);
	log_i("adm_arpc_releasePeer");
	adm_arpc_releasePeer(peerinfo);

    struct timespec timeout;
	(void)clock_gettime(CLOCK_MONOTONIC, &timeout);
	timeout.tv_sec += 3;
    
	(void)pthread_mutex_lock(&priv->syncMutex);
	D32S iStatus = adm_arpc_pthread_cond_timedwait(&priv->syncCond, &priv->syncMutex, &priv->bSyncIsSignal, &timeout);

	log_i("sub event, wait cond status %d", iStatus);

	if(0 != iStatus)
	{
		//设置超时标志
		priv->bCallerTimeout = DTRUE;
		log_i("sub event timeout, instanceId:0X%04X,serviceId:0X%04X,event:0X%04X", instanceId, serviceId, eventId);

		(void)pthread_mutex_unlock(&priv->syncMutex);
		
		ADM_ARPC_QUEUE_DATA_T* sendData = adm_arpc_getSyncSendDataSesssion(instanceId, serviceId, eventId, reqPackage->packageHead.sessionId);
		if (sendData != NULL)
		{
			(void)pthread_mutex_destroy(&priv->syncMutex);
			(void)pthread_cond_destroy(&priv->syncCond);
			
			adm_arpc_freeSyncSendData(sendData);

			/**如果超时且发送线程提前拿到队列中的sendData，交由发送线程处理*/
			adm_free(priv);
		}

		/**如果对端先回复，响应线程会处理释放，这里直接返回*/
		adm_arpc_addWaitSubEvent(instanceId, serviceId, eventId);

		return ADM_ECODE_RPC_REQSERV_TIMEOUT;
	}
	else
	{
		(void)pthread_mutex_unlock(&priv->syncMutex);
		(void)pthread_mutex_destroy(&priv->syncMutex);
		(void)pthread_cond_destroy(&priv->syncCond);

		if(respData == NULL)
		{
			log_e("resp data is empty");
			adm_free(priv);
			return iRet;
		}
		
		struct json_object* subAckObj = json_tokener_parse(respData);
		adm_free(respData);
		
		if(NULL == subAckObj)
		{
			log_e("invalid json format reqData");
			adm_free(priv);
			return iRet;
		}

		json_object* resultObj = json_object_object_get(subAckObj,"result");
		D32S result = json_object_get_int(resultObj);

		if(1 == result)
		{	
			log_i("subscriberEvent instanceId:0X%04X,serviceId:0X%04X,event:0X%04X ok", instanceId, serviceId, eventId);
			adm_arpc_addSubedEvent(instanceId, serviceId, eventId);		
			iRet = ADM_ECODE_SUCCESSED;
		}
		else
		{
			log_e("sub event fail, instanceId:0X%04X,serviceId:0X%04X,event:0X%04X", instanceId, serviceId, eventId);
			adm_arpc_addWaitSubEvent(instanceId, serviceId, eventId);
		}

		(void)json_object_put(subAckObj);
		
		/**对端正常回复，释放priv */
		adm_free(priv);
	}
	
	return iRet;
}

static D32S adm_arpc_subscriberMultiEvent_for_retry(D16U instanceId,DCHAR* reqData, D32U reqDataSize)
{
	D32S iRet = ADM_ECODE_FAILED;

	ADM_ARPC_PACKAGE_T* reqPackage = adm_arpc_makePackageDef(instanceId,(D16U)ADM_ARPC_SUB_MULTIEVENT_SERVICEID,
															(D16U)ADM_ARPC_SUB_MULTIEVENT_MESSAGEID,0,
															(D8U)ADM_ARPC_MESSAGE_TYPE_SUBEVENT,reqData,reqDataSize);
	ADM_ARPC_PEER_T* peerinfo = adm_arpc_getPeerInfoByInstanceId(instanceId);
	if(peerinfo == NULL)
	{
		log_e("service is offline");
		return iRet;
	}

	// (void)adm_arpc_pushToSendQueue(reqPackage,peerinfo,0,NULL);
	adm_arpc_add2SendQueue(reqPackage,peerinfo,0,NULL);
	log_i("adm_arpc_releasePeer");
	adm_arpc_releasePeer(peerinfo);

	iRet = ADM_ECODE_SUCCESSED;
	return iRet;
}


void adm_arpc_registerEventHandler(D16U instanceId,  D16U serviceId, D16U eventId, ADM_ARPC_EVENT_CALLBACK_F eventHandler)
{
	DCHAR strKey[13] = {0};
	adm_arpc_getTwiceId(instanceId, serviceId,eventId, strKey);
	bool iet = map_put(eventHandlers, strKey, &eventHandler,(D32U)sizeof(eventHandler));
	if(iet == false)
	{
		log_i("eventHandlers put error!");
		return ;
	}
	log_i("register eventHandler instanceId:0X%04X, serviceId:0X%04X, eventId:0X%04X success",instanceId,serviceId, eventId );

    return;
}

D32S adm_arpc_request_service(D16U instanceId, D16U serviceId, D16U methodId, D16U timeOut, 
										const DCHAR* reqData, D32U reqDataSize, DCHAR** respData, D32U* respDataSize)
{
	D32S iRet = ADM_ECODE_FAILED;

	ADM_ARPC_PEER_T* peerinfo = adm_arpc_getPeerInfoByInstanceId(instanceId);
	if(peerinfo == NULL)
	{
		log_e("service instanceId 0X%04X is offline, return %d", instanceId, ADM_ECODE_RPC_INSTANCE_OFFLINE);
		iRet = ADM_ECODE_RPC_INSTANCE_OFFLINE;
		return iRet;
	}

	ADM_ARPC_PACKAGE_T* reqPackage = adm_arpc_makePackageDef(instanceId,serviceId,methodId,
										0,ADM_ARPC_MESSAGE_TYPE_METHOD_REQ,reqData,reqDataSize);

	if(reqPackage == NULL)
	{
		log_i("adm_arpc_releasePeer");
		adm_arpc_releasePeer(peerinfo);
		log_w("check req data");
		return ADM_ECODE_FAILED;
	}
	//log_i("req: data: %s", reqPackage->data);

	D16U tempSessionId = reqPackage->packageHead.sessionId; // after push reqPackage will be freed
	
	pthread_mutex_t syncMutex;
	pthread_cond_t syncCond;
	pthread_condattr_t condAttr;
	(void)pthread_mutex_init(&syncMutex, NULL);
	(void)pthread_condattr_setclock(&condAttr, CLOCK_MONOTONIC);
	(void)pthread_cond_init(&syncCond, &condAttr);

	ADM_ARPC_CLIENT_PRIVDATA_T* priv = (ADM_ARPC_CLIENT_PRIVDATA_T*)adm_malloc(sizeof(ADM_ARPC_CLIENT_PRIVDATA_T));
	(void)adm_memset(priv, 0x0, sizeof(ADM_ARPC_CLIENT_PRIVDATA_T), sizeof(ADM_ARPC_CLIENT_PRIVDATA_T));

	priv->syncMutex = syncMutex;
	priv->syncCond = syncCond;
	priv->respData = respData;
	priv->dataSize = respDataSize;
    priv->bSyncIsSignal = DFALSE;
	priv->bCallerTimeout = DFALSE;

    // (void)adm_arpc_pushToSendQueue(reqPackage,peerinfo,1,priv);
	adm_arpc_add2SendQueue(reqPackage,peerinfo,1,priv);
	log_i("adm_arpc_releasePeer");
	adm_arpc_releasePeer(peerinfo);

    struct timespec tv;
	(void)clock_gettime(CLOCK_MONOTONIC, &tv);
	tv.tv_sec += (D32S)timeOut;

	(void)pthread_mutex_lock(&priv->syncMutex);
    D32S status = adm_arpc_pthread_cond_timedwait(&priv->syncCond, &priv->syncMutex, &priv->bSyncIsSignal, &tv);

    log_i("cond status %d", status);

	if(0 != status)
	{
		priv->bCallerTimeout = DTRUE;
		log_i("timeout:%d, instanceId:0X%04X, serviceId:0X%04X, methodId:0X%04X, return %d", timeOut,instanceId,serviceId, methodId, ADM_ECODE_RPC_REQSERV_TIMEOUT);

		(void)pthread_mutex_unlock(&priv->syncMutex);
		ADM_ARPC_QUEUE_DATA_T* sendData = adm_arpc_getSyncSendDataSesssion(instanceId, serviceId, methodId, tempSessionId);
		if (sendData != NULL)
		{
			(void)pthread_mutex_destroy(&priv->syncMutex);
			(void)pthread_cond_destroy(&priv->syncCond);
			
			adm_arpc_freeSyncSendData(sendData);

			/**如果超时且发送线程提前拿到队列中的sendData，交由发送线程处理*/
			adm_free(priv);
		}

		iRet = ADM_ECODE_RPC_REQSERV_TIMEOUT;
	}
	else
	{
		if (0 != *respDataSize)
		{			
			(void)pthread_mutex_unlock(&priv->syncMutex);
			(void)pthread_mutex_destroy(&priv->syncMutex);
			(void)pthread_cond_destroy(&priv->syncCond);

			json_object* respObj = json_tokener_parse(*respData);
			json_object* errObj = json_object_object_get(respObj,"arpcErrorCode");
			if (NULL != errObj)
			{
				iRet = json_object_get_int(errObj);
				log_e("errorCode:%d, instanceId:0X%04X, serviceId:0X%04X, methodId:0X%04X", iRet,instanceId,serviceId, methodId);
			}
			else
			{
				iRet = ADM_ECODE_SUCCESSED;
			}

			(void)json_object_put(respObj);
		}

		/**如果对端正常回复不管恢复的数据是否有效都要释放 */
		adm_free(priv);
	}

	//adm_free(priv);
	
	return iRet;
}

D32S adm_arpc_checkIfClientSubedEvent(D16U instanceId, D16U serviceId, D16U eventId, D16U clientId)
{
	DCHAR strKey[13] = {0};
	adm_arpc_getTwiceId(instanceId,serviceId,eventId, strKey);
	D32S valueSize = 8;
	map_t* node = NULL;
	D32S isSubed = 0;

	DCHAR strinstanceId[5] = {0};
	DCHAR strserviceId[5] = {0};
	DCHAR strEventId [5] = {0};

	D16U servInstanceId = 0;
	D16U servServiceId = 0;
	D16U servEventId = 0;
	
    for (node = map_first(subEventClients); node != NULL; node = map_next(node)) 
	{
		if(0 == adm_strcmp(strKey,map_get_key(node)))
		{
			(void)adm_memcpy(strinstanceId, strKey, 4, 4);
			(void)adm_memcpy(strserviceId, strKey+4, 4, 4);
			(void)adm_memcpy(strEventId, strKey+8, 4, 4);

			errno = 0;
			servInstanceId = (D16U)(strtol(strinstanceId, NULL, 16));
			if(0 != errno)
			{
				log_e("strtol, errno=%d, reason=%s",errno, strerror(errno));
			}

			errno = 0;
			servServiceId = (D16U)(strtol(strserviceId, NULL, 16));
			if(0 != errno)
			{
				log_e("strtol, errno=%d, reason=%s",errno, strerror(errno));
			}

			errno = 0;
			servEventId = (D16U)(strtol(strEventId, NULL, 16));
			if(0 != errno)
			{
				log_e("strtol, errno=%d, reason=%s",errno, strerror(errno));
			}
			
			struct list_head** EventClients = (struct list_head**)map_get_value(node);
			if(*EventClients != NULL)
			{
				ADM_ARPC_MSGLIST_T *pstPos  = NULL;
				list_for_each_entry(pstPos,*EventClients, ADM_ARPC_MSGLIST_T, entry) 
			    {
					if(pstPos->instanceId == clientId)
		    		{
		    			isSubed = 1;
		    			break;
		    		}
					/*
			    	if(NULL != pstPos->pstMsg && (D8U)0 == pstPos->pstMsg->free)
			    	{
			    		if(pstPos->pstMsg->instanceId == clientId)
			    		{
			    			isSubed = 1;
			    			break;
			    		}
			    	}
			    	*/
				}
			}

			if(isSubed == 1)
			{
				break;
			}
		}
    }

	log_i("client subscriber instanceId: 0X%04X, serviceid: 0X%04X, eventId: 0X%04X result:%d",instanceId, serviceId, eventId, isSubed);
	return isSubed;

}

//adm_arpc_internal.h
void adm_arpc_callServiceHandler(ADM_ARPC_QUEUE_DATA_T* recvQueueData)
{
	ADM_ARPC_PACKAGE_T* package = recvQueueData->package;
	DCHAR strID[13] = {0};
	adm_arpc_getTwiceId(package->packageHead.peerInstanceId, package->packageHead.serviceId,package->packageHead.messageId, strID);

	//log_i("key: %s", strID);

	// call handler
	D32U dataSize = 0;
	map_t* node = NULL;
	int ishavekey = 0;
	
    for (node = map_first(mehtodHandlers); node != NULL; node = map_next(node)) 
	{
		if(0 == adm_strcmp(strID,map_get_key(node)))
		{
			//log_i("mehtodHandlers has the key!");
			ishavekey = 1;
		}
    }
    DCHAR* respData = NULL;
	D32U respDataSize = 0;

	if(ishavekey == 1)
	{	
		//ADM_ARPC_METHOD_CALLBACK_F  handler = (ADM_ARPC_METHOD_CALLBACK_F)map_get_val(mehtodHandlers,strID,&dataSize);
		ADM_ARPC_METHOD_CALLBACK_F* ptr = map_get_val(mehtodHandlers,strID,&dataSize);
		ADM_ARPC_METHOD_CALLBACK_F	handler  = *ptr;
		if(handler != NULL)
		{
			
			D32S retCode = handler(package->data,package->packageHead.length - (D32U)ADM_ARPC_PROTOCOL_HEAD_IN_LEN, &respData, &respDataSize);
			log_i("call servHandler instanceId:0X%04X, serviceId:0X%04X, methodId:0X%04X, retCode:%d", package->packageHead.peerInstanceId, package->packageHead.serviceId, package->packageHead.messageId , retCode);
		}
		else
		{
			log_e("serviceId:%d, instanceId:0X%04X, instanceId:0X%04X, methodId:%d, have no handler", package->packageHead.peerInstanceId, package->packageHead.serviceId,package->packageHead.messageId);
			
		}	
		
	}
	else
	{
		///< no handler, but send ack
		///< 服务不在线  {"errorCode": xxx}
		log_i("mehtodHandlers does not have Handler!"); 
		json_object* respObj = json_object_new_object();
		(void)json_object_object_add(respObj, "arpcErrorCode", json_object_new_int(ADM_ECODE_RPC_SERVICE_OFFLINE));
		respData = (DCHAR*)json_object_to_json_string(respObj);
		respDataSize = (D32U)( adm_strlen(respData)+(D32U)1 );
	}
	
	// make packeage
	ADM_ARPC_PACKAGE_T* respPackage = adm_arpc_makePackage(package->packageHead.peerInstanceId,package->packageHead.instanceId, package->packageHead.serviceId, package->packageHead.messageId,
													package->packageHead.sessionId, ADM_ARPC_MESSAGE_TYPE_METHOD_RESP, respData, respDataSize);

	// send resp package
	void* priv = NULL;
	// (void)adm_arpc_pushToSendQueue(respPackage, recvQueueData->peer, 0, priv);
	adm_arpc_add2SendQueue(respPackage, recvQueueData->peer, 0, priv);

	adm_free(respData);

    return;
}

D32S adm_arpc_addInstancePeerInfo(D16U instanceId, ADM_ARPC_PEER_T* peer)
{
	DCHAR strID[5] = {0};
	(void)sprintf(strID,"%04X",instanceId);

	D32U valueSize = 0;
	pthread_mutex_lock(&peerInfosMutex);
	ADM_ARPC_PEER_T** oldpPeerInfo = (ADM_ARPC_PEER_T**)map_get_val(peerInfos,strID,&valueSize);
	if(oldpPeerInfo != NULL)
	{
		ADM_ARPC_PEER_T* oldPeerInfo = *oldpPeerInfo;
		if(adm_strcmp(oldPeerInfo->ip,peer->ip) == 0 && oldPeerInfo->port == peer->port)
		{
			pthread_mutex_unlock(&peerInfosMutex);
			return ADM_ECODE_SUCCESSED;
		}
	}

	if (DTRUE != adm_arpc_retainPeer(peer))
	{
		pthread_mutex_unlock(&peerInfosMutex);
		return ADM_ECODE_FAILED;
	}

	peer->instanceId = instanceId;
	log_i("adm_arpc_addInstancePeerInfo list_empty= %d, peer:%p, list:%p", list_empty(peer->childInstancelist), peer, peer->childInstancelist);
	
	if(false == map_put(peerInfos, strID, &peer,(D32U)sizeof(ADM_ARPC_PEER_T**)) )
	{
		pthread_mutex_unlock(&peerInfosMutex);
		log_e("Map peerInfo put error!");
		return ADM_ECODE_FAILED;
	}

	pthread_mutex_unlock(&peerInfosMutex);

	// map put data, malloc
	//ADM_ARPC_PEER_T* mapPeer = *(ADM_ARPC_PEER_T**)(map_get_val(peerInfos, strID, &valueSize));
	//INIT_LIST_HEAD(mapPeer->childInstancelist);
	//log_i("adm_arpc_addInstancePeerInfo strID:%s, list_empty= %d,peerInfos:%p, peer:%p, list:%p",strID, list_empty(mapPeer->childInstancelist),peerInfos, mapPeer, mapPeer->childInstancelist);	
	
	log_i("addInstancePeerInfo, instanceId:0X%04X, ip:%s, port:%d, fd:%d ", instanceId, peer->ip, peer->port, peer->fd);
	return ADM_ECODE_SUCCESSED;
}

D32S adm_arpc_replaceInstancePeerInfo(D16U instanceId, ADM_ARPC_PEER_T* peer)
{
	//!< 内部map_put 替换
	//return adm_arpc_addInstancePeerInfo(instanceId, peer);

	DCHAR strID[5] = {0};

	(void)sprintf(strID,"%04X",instanceId);

	// old peer
	D32U valueSize = 0;
	pthread_mutex_lock(&peerInfosMutex);
	ADM_ARPC_PEER_T** oldpPeerInfo = (ADM_ARPC_PEER_T**)(map_get_val(peerInfos,strID,&valueSize));
	if(NULL != oldpPeerInfo)
	{
		ADM_ARPC_PEER_T* oldPeerInfo = *oldpPeerInfo;
		if(adm_strcmp(oldPeerInfo->ip,peer->ip) == 0 && oldPeerInfo->port == peer->port)
		{
			pthread_mutex_unlock(&peerInfosMutex);
			return ADM_ECODE_SUCCESSED;
		}

		// 多实例, 相同的老ip, port 替换.
		map_t* node = NULL;
	    for (node = map_first(peerInfos); node != NULL; node = map_next(node)) 
		{
			ADM_ARPC_PEER_T* tmpPeer = *(ADM_ARPC_PEER_T**)map_get_value(node);

			DCHAR* tmpKey = map_get_key(node);

			if(adm_strcmp(tmpPeer->ip,oldPeerInfo->ip) == 0 && tmpPeer->port == oldPeerInfo->port)
			{
				if(false == map_put(peerInfos, tmpKey, &peer,(D32U)sizeof(ADM_ARPC_PEER_T*)) )
				{
					pthread_mutex_unlock(&peerInfosMutex);
					log_e("Map peerInfo put error!, key:%s", tmpKey);
					return ADM_ECODE_FAILED;
				}

				log_i("update peer info, instanceId:0X%04X, oldip:%s, oldport:%d, newip:%s, newport:%d", tmpKey, tmpPeer->ip, tmpPeer->port, peer->ip, peer->port);
			}
	    }
	}

	pthread_mutex_unlock(&peerInfosMutex);

	return ADM_ECODE_SUCCESSED;
}

void adm_arpc_delInstancePeerInfo(ADM_ARPC_PEER_T* peer)
{	
	DCHAR strID[5] = {0};

	(void)sprintf(strID,"%04X",peer->instanceId);

	D32U valSize = 0;
	pthread_mutex_lock(&peerInfosMutex);
	ADM_ARPC_PEER_T** ppeerInfo = (ADM_ARPC_PEER_T**)(map_get_val(peerInfos,strID,&valSize));	
	if(ppeerInfo == NULL)
	{
		log_w("%s can not find peerInfo value.", strID);
		pthread_mutex_unlock(&peerInfosMutex);
		return;
	}

    // if (peer->fd == (*ppeerInfo)->fd)  //新老的fd有可能也是一样的,地址直接比较即可
    if (peer == (*ppeerInfo))
    {
        log_i("Erase instanceId[0X%04X] fd[%d].", peer->instanceId, peer->fd);
        /** map_put时，如果对端是新的连接，会释放老的连接，存放新的连接 */
        map_erase(peerInfos, strID);
    }
    pthread_mutex_unlock(&peerInfosMutex);
    
    if(NULL == peer->childInstancelist)
	{
		log_e("peer[0X%04X] childInstancelist is null", peer->instanceId);
		// pthread_mutex_unlock(&peerInfosMutex);
		return;
	}

	// ADM_ARPC_PEER_T* peerInfo = *ppeerInfo;
	ADM_ARPC_CHILD_INSTANCE_LIST_T *p, *n;
    list_for_each_entry_safe(p, n, peer->childInstancelist, ADM_ARPC_CHILD_INSTANCE_LIST_T, entry)
	{
		log_i("del child instanceId, 0X%04X", p->instanceId);
	    list_del(&p->entry);
		adm_free(p);
	}

    log_i("free childInstancelist, %s %d", strID, peer->fd);
    adm_free(peer->childInstancelist);

    // pthread_mutex_unlock(&peerInfosMutex);

    adm_arpc_releasePeer(peer);

	//peerInfo = (ADM_ARPC_PEER_T*)map_get_val(peerInfos,strID,&valSize);	

    return;
}

ADM_ARPC_PEER_T* adm_arpc_getPeerInfoByInstanceId(D16U instanceId)
{	
	DCHAR strID[5] = {0};
	(void)sprintf(strID,"%04X",instanceId);
	int valueSize = 0;
	map_t* node = NULL;

	D32S find = 0;

	ADM_ARPC_PEER_T* peerInfo = NULL;
	pthread_mutex_lock(&peerInfosMutex);
    for (node = map_first(peerInfos); node != NULL; node = map_next(node)) 
	{
		peerInfo = *(ADM_ARPC_PEER_T**)map_get_value(node);		
		if(peerInfo != NULL)
		{
			if (DFALSE == adm_arpc_retainPeer(peerInfo))
			{
				peerInfo = DNULL;
				break;
			}
			// log_i("adm_arpc_retainPeer");
			// log_i("key[%s] node key[%s]", strID, map_get_key(node));
			if(0 == adm_strcmp(strID,map_get_key(node)))
			{
				find = 1;
				break;
			}

			// log_i("adm_arpc_getPeerInfoByInstanceId list_empty= %d, peer:%p, list:%p", list_empty(peerInfo->childInstancelist), peerInfo, peerInfo->childInstancelist);
			
			if(1 == list_empty(peerInfo->childInstancelist))
			{
				// log_i("adm_arpc_releasePeer");
				adm_arpc_releasePeer(peerInfo);
				peerInfo = DNULL;
				continue;
			}
			
			// find if is chlid instance
			ADM_ARPC_CHILD_INSTANCE_LIST_T *p;
		    list_for_each_entry(p, peerInfo->childInstancelist, ADM_ARPC_CHILD_INSTANCE_LIST_T, entry)
			{
				log_i("adm_arpc_getPeerInfoByInstanceId child list instanceId: 0X%04X", p->instanceId);
			    if(p->instanceId == instanceId)
			    {
			    	find = 1;
					break;
			    }
			}
		}

		if(1 == find)
		{
			break;
		}

		if (DNULL != peerInfo)
		{
			log_i("adm_arpc_releasePeer");
			adm_arpc_releasePeer(peerInfo);
			peerInfo = DNULL;
		}
    }
	pthread_mutex_unlock(&peerInfosMutex);

	return peerInfo;
}

void adm_arpc_recvServerResp(ADM_ARPC_QUEUE_DATA_T* recvQueueData)
{
	// send queue find session. copy resp data. trigger condition. 
	ADM_ARPC_PACKAGE_T* recvPackage = recvQueueData->package;
	ADM_ARPC_QUEUE_DATA_T* sendQueueData = adm_arpc_getSyncSendDataSesssion(recvPackage->packageHead.instanceId, recvPackage->packageHead.serviceId, 
																			recvPackage->packageHead.messageId, recvPackage->packageHead.sessionId);
	
	if(sendQueueData == NULL)
	{
		// multi sub, async, no session data
		if( recvQueueData->package->packageHead.messageType == (D8U)ADM_ARPC_MESSAGE_TYPE_SUBEVENT_ACK && 
			recvQueueData->package->packageHead.serviceId == (D16U)ADM_ARPC_SUB_MULTIEVENT_SERVICEID &&
			recvQueueData->package->packageHead.messageId == (D16U)ADM_ARPC_SUB_MULTIEVENT_SERVICEID)
		{
			//aync resub, for retry
			adm_arpc_retrySubEvent_handler(recvQueueData->package->data, recvPackage->packageHead.length - (D32U)ADM_ARPC_PROTOCOL_HEAD_IN_LEN);
			return;
		}
		else
		{
			log_i("no sync req, instanceid: 0X%04X, serviceId:0X%04X, messageId:0X%04X, sessionId:%d", 
				recvPackage->packageHead.instanceId, recvPackage->packageHead.serviceId,
				recvPackage->packageHead.messageId, recvPackage->packageHead.sessionId);
			return;	
		}
	}
	
	ADM_ARPC_CLIENT_PRIVDATA_T* clilentPriv = (ADM_ARPC_CLIENT_PRIVDATA_T*)sendQueueData->priv;
	if(clilentPriv == NULL)
	{
		log_i("no sync req priv, instanceid: 0X%04X, serviceId:0X%04X, messageId:0X%04X, sessionId:%d", 
			recvPackage->packageHead.instanceId, recvPackage->packageHead.serviceId,
			recvPackage->packageHead.messageId, recvPackage->packageHead.sessionId);
			
		return;
	}

	DCHAR** respData = clilentPriv->respData;
	D32U* dataSize = clilentPriv->dataSize;

	if(respData == NULL || dataSize == NULL)
	{
		log_e("invalid priv data pointers (NULL), instanceid: 0X%04X, serviceId:0X%04X, messageId:0X%04X, sessionId:%d",
			recvPackage->packageHead.instanceId, recvPackage->packageHead.serviceId,
			recvPackage->packageHead.messageId, recvPackage->packageHead.sessionId);
		adm_arpc_freeSyncSendData(sendQueueData);
		return;
	}
	
	(void)pthread_mutex_lock(&clilentPriv->syncMutex);

	//< 判断是否超时，如果超时直接返回
	if(clilentPriv->bCallerTimeout == DTRUE)
	{
		log_i("caller timeout, skip allocating resp data, instanceid: 0X%04X, serviceId:0X%04X, messageId:0X%04X, sessionId:%d",
			recvPackage->packageHead.instanceId, recvPackage->packageHead.serviceId,
			recvPackage->packageHead.messageId, recvPackage->packageHead.sessionId);
		
		(void)pthread_mutex_unlock(&clilentPriv->syncMutex);
		(void)pthread_mutex_destroy(&clilentPriv->syncMutex);
		(void)pthread_cond_destroy(&clilentPriv->syncCond);
		adm_arpc_freeSyncSendData(sendQueueData);

		/**超时且响应线程先拿到锁，有响应线程释放 */
		adm_free(clilentPriv);
		return;  
	}



	if(recvPackage->packageHead.length - (D32U)ADM_ARPC_PROTOCOL_HEAD_IN_LEN > 0U)
	{
		*respData = (DCHAR*)adm_malloc((D32U)(recvPackage->packageHead.length - (D32U)ADM_ARPC_PROTOCOL_HEAD_IN_LEN));
		if(NULL == *respData)
		{
			log_e("malloc respData fail");
			(void)pthread_mutex_unlock(&clilentPriv->syncMutex);
			return;
		}

		// copy resp data
		(void)adm_memcpy(*respData, recvPackage->data, (D32U)(recvPackage->packageHead.length - (D32U)ADM_ARPC_PROTOCOL_HEAD_IN_LEN), 
					(D32U)(recvPackage->packageHead.length - (D32U)ADM_ARPC_PROTOCOL_HEAD_IN_LEN));

		*(clilentPriv->dataSize) = recvPackage->packageHead.length - (D32U)ADM_ARPC_PROTOCOL_HEAD_IN_LEN;
	
	}

    clilentPriv->bSyncIsSignal = DTRUE;
    (void)pthread_cond_signal(&clilentPriv->syncCond);

    (void)pthread_mutex_unlock(&clilentPriv->syncMutex);

	adm_arpc_freeSyncSendData(sendQueueData);
	
    return;
}

void adm_arpc_addSubEventClient( D16U instanceId, D16U serviceId, D16U eventId, D16U sessionId, ADM_ARPC_PEER_T* client)
{
	DCHAR strKey[13] = {0};
	adm_arpc_getTwiceId(instanceId, serviceId,eventId, strKey);

	log_i("sub key:%s", strKey);

	D32U valueSize = 8;
	map_t* node = NULL;
	D32S ishavekey = 0;

	pthread_mutex_lock(&subEventClientsMutex);
    for (node = map_first(subEventClients); node != NULL; node = map_next(node)) 
	{
		if(0 == adm_strcmp(strKey,map_get_key(node)))
		{
			ishavekey = 1;
			break;
		}
    }
    
	json_object* subAckObj = json_object_new_object();
	json_object* resultObj = json_object_new_int(0);	
	
	do
	{
		if(ishavekey == 0)
		{		
			log_i("not offer serviceid:0X%04X, eventid:0X%04X for instanceid:0X%04X", serviceId, eventId, client->instanceId);
			break;
		}
	
		struct list_head** ListHead = (struct list_head**)map_get_val(subEventClients,strKey,&valueSize);

        if (ListHead == NULL || *ListHead == NULL)
        {
            log_i("not offer serviceid:0X%04X, eventid:0X%04X for instanceid:0X%04X, key:%s", serviceId, eventId, instanceId, strKey);
			break;
		}
		

		ADM_ARPC_MSGLIST_T *pstPos1  = NULL;
	    ADM_ARPC_MSGLIST_T *pstNext1 = NULL;

	    int ishaveSubed = 0;
	    list_for_each_entry_safe(pstPos1,pstNext1, *ListHead, ADM_ARPC_MSGLIST_T, entry) 
	    {
	    	if(pstPos1->instanceId == client->instanceId)
	    	{
	    		log_w("0X%04X is already subcribered servid 0X%04X, eventid 0X%04X ", client->instanceId,serviceId, eventId);
	    		ishaveSubed = 1;
	    		break;
	    	}
	    }

	    if(ishaveSubed == 1)
	    {
	    	break;
	    }

	    ADM_ARPC_MSGLIST_T *EventClients = adm_arpc_msgListNew();
		EventClients->instanceId = client->instanceId;
		list_add_tail(&EventClients->entry,*ListHead);		
		(void)json_object_set_int(resultObj, 1);

		log_i("0X%04X is subcriber instanceid 0X%04X,  servid 0X%04X, eventid 0X%04X success", client->instanceId, instanceId, serviceId, eventId);
    
	}while(false);
	pthread_mutex_unlock(&subEventClientsMutex);
	
	(void)json_object_object_add(subAckObj,"result",resultObj);
	size_t msgLength = 0;
	const DCHAR* strSubAck = json_object_to_json_string_length(subAckObj, JSON_C_TO_STRING_PLAIN, &msgLength);

	//log_i("event ack: %s", strSubAck, msgLength);
	
	
	ADM_ARPC_PACKAGE_T* respPackage = adm_arpc_makePackage(instanceId, client->instanceId, serviceId, eventId, sessionId, 
					ADM_ARPC_MESSAGE_TYPE_SUBEVENT_ACK, strSubAck, (D32U)(msgLength+(size_t)1));
	(void)json_object_put(subAckObj);	
	
	// send resp package
	// (void)adm_arpc_pushToSendQueue(respPackage, client, 0, NULL);
	adm_arpc_add2SendQueue(respPackage, client, 0, NULL);
	
	return;
}

void adm_arpc_addSubEventMultiClient(ADM_ARPC_QUEUE_DATA_T* queueData)
{
	log_i("Enter adm_arpc_addSubEventMultiClient");
	size_t eventlen = 0;
	ADM_ARPC_PACKAGE_T* package = queueData->package;
    D16U peerInstanceId = package->packageHead.instanceId;
    struct json_object* reqObj= NULL;
    reqObj = json_tokener_parse(queueData->package->data);
	//log_i("subeventInfo = %s",json_object_to_json_string(reqObj));

    // 最小修复：解析失败直接返回，避免后续对 NULL 解引用
	if (reqObj == NULL)
	{
		log_e("adm_arpc_addSubEventMultiClient: json parse fail");
		return;
	}

	struct json_object* subEventInfo = json_object_object_get(reqObj,"subeventInfo");
	
	if (json_object_get_type(subEventInfo) != json_type_array)
	{
		(void)json_object_put(reqObj);
		log_i("this is is an error array json!");
		// resp error;
        // 最小修复：直接返回，避免后续使用已释放的 reqObj
		return;
	}
	 else
	 {
		pthread_mutex_lock(&subEventClientsMutex);

		eventlen = json_object_array_length(subEventInfo);
		json_object *eventObj = NULL;
		for (size_t i = 0; i < eventlen; i++)
		{
			eventObj = json_object_array_get_idx(subEventInfo, i);
            if (eventObj == NULL)
            {
                continue;
            }

            D16U instanceId = (D16U)(json_object_get_int(json_object_object_get(eventObj,"instanceId")));
			D16U serviceId = (D16U)(json_object_get_int(json_object_object_get(eventObj,"serviceId")));
			D16U eventId = (D16U)(json_object_get_int(json_object_object_get(eventObj,"eventId")));
			
			//begin to put client to subEventClients
			DCHAR strKey[13] = {0};
			adm_arpc_getTwiceId(instanceId, serviceId,eventId, strKey);

			D32U valueSize = 8;
			map_t* node = NULL;
			D32S ishavekey = 0;
			for (node = map_first(subEventClients); node != NULL; node = map_next(node)) 
			{
				if(0 == adm_strcmp(strKey,map_get_key(node)))
				{
                    struct list_head **ListHead = (struct list_head **)map_get_val(subEventClients, strKey, &valueSize);
                    // 最小修复：仅判空，未 offer 时略过该项，不改协议
                    if (ListHead == NULL)
                    {
                        log_i("multi sub not offer, key:%s, peer:0X%04X, inst:0X%04X, serv:0X%04X, event:0X%04X", strKey, peerInstanceId, instanceId, serviceId, eventId);
                        continue;
                    }
                    if (*ListHead != NULL)
                    {
						ADM_ARPC_MSGLIST_T *pstPos1  = NULL;
					    ADM_ARPC_MSGLIST_T *pstNext1 = NULL;

					    D32S ishaveSubed = 0;
					    list_for_each_entry_safe(pstPos1,pstNext1, *ListHead, ADM_ARPC_MSGLIST_T, entry) 
					    {
					    	if(pstPos1->instanceId == peerInstanceId)
					    	{
					    		log_w("0X%04X is already subcribered servid 0X%04X, eventid 0X%04X ", pstPos1->instanceId,serviceId, eventId);
					    		ishaveSubed = 1;
					    		break;
					    	}
					    }

					    if(ishaveSubed == 1)
					    {
					    	break;
					    }

					    log_i("client sub instanceId = 0X%04X, serviceId = 0X%04X,eventId = 0X%04X ",instanceId,serviceId, eventId);
					
						ADM_ARPC_MSGLIST_T *EventClients = adm_arpc_msgListNew();
						//EventClients->pstMsg = queueData->peer;
                        EventClients->instanceId = peerInstanceId;

                        list_add_tail(&EventClients->entry,*ListHead);

						(void)json_object_object_add(eventObj,"flag",json_object_new_int(1));
					}
				}
			}
			
		}

		pthread_mutex_unlock(&subEventClientsMutex);
	 }

	size_t msgLength = 0;
	const DCHAR* strSubAck = json_object_to_json_string_length(reqObj, JSON_C_TO_STRING_PLAIN, &msgLength);

	//log_i("multi subevent, resp:%s", strSubAck);

	ADM_ARPC_PACKAGE_T* respPackage = adm_arpc_makePackage(package->packageHead.peerInstanceId,package->packageHead.instanceId, 
															(D16U)ADM_ARPC_SUB_MULTIEVENT_SERVICEID,(D16U)ADM_ARPC_SUB_MULTIEVENT_MESSAGEID,package->packageHead.sessionId,
															(D8U)ADM_ARPC_MESSAGE_TYPE_SUBEVENT_ACK, strSubAck, (D32U)msgLength);
	// send resp package
	// (void)adm_arpc_pushToSendQueue(respPackage, queueData->peer, 0, NULL);
	adm_arpc_add2SendQueue(respPackage, queueData->peer, 0, NULL);

	(void)json_object_put(reqObj);
	
	log_i("Leave adm_arpc_addSubEventMultiClient");

    return;
}

void adm_arpc_delSubEvent(ADM_ARPC_PEER_T* peer)
{
	int valueSize = 8;
	map_t* node = NULL;

	pthread_mutex_lock(&subEventClientsMutex);
    for (node = map_first(subEventClients); node != NULL; node = map_next(node)) 
	{
		struct list_head** ListHead = (struct list_head**)map_get_value(node);
		if(*ListHead == NULL)
		{
			break;
		}		

		ADM_ARPC_MSGLIST_T *pstPos1  = NULL;
	    ADM_ARPC_MSGLIST_T *pstNext1 = NULL;

	    int ishaveSubed = 0;
	    list_for_each_entry_safe(pstPos1,pstNext1, *ListHead, ADM_ARPC_MSGLIST_T, entry) 
	    {
	    	if(pstPos1->instanceId == peer->instanceId)
	    	{
	    		log_i("del instanceId 0X%04X subed event: %s", peer->instanceId, map_get_key(node));
	    		list_del(&pstPos1->entry);
	    		break;
	    	}
	    }

		if(NULL == peer->childInstancelist)
		{
			pthread_mutex_unlock(&subEventClientsMutex);
			log_e("peer->childInstancelist is null");
			return;
		}

	    list_for_each_entry_safe(pstPos1,pstNext1, *ListHead, ADM_ARPC_MSGLIST_T, entry) 
	    {
	    	ADM_ARPC_CHILD_INSTANCE_LIST_T *n;
		    list_for_each_entry(n, peer->childInstancelist, ADM_ARPC_CHILD_INSTANCE_LIST_T, entry)
			{
			    if(pstPos1->instanceId == n->instanceId)
			    {
			    	log_i("del instanceId 0X%04X subed event: %s", peer->instanceId, map_get_key(node));
		    		list_del(&pstPos1->entry);
		    		break;
			    }
			}
	    }
    }

	pthread_mutex_unlock(&subEventClientsMutex);
	
	return;  
}

void adm_arpc_callEventHandler(D16U instanceId, D16U servcieid, D16U eventId, D8S* data, D32U dataSize)
{
	DCHAR strKey[13] = {0};
	adm_arpc_getTwiceId(instanceId, servcieid,eventId, strKey);
	D32U valueSize = 8;
	map_t* node = NULL;
	D32S ishavekey = 0;
    for (node = map_first(eventHandlers); node != NULL; node = map_next(node)) 
	{
		if(0 == adm_strcmp(strKey,map_get_key(node)))
		{
			ishavekey = 1;
		}
    }
	if(ishavekey == 1)
	{
		ADM_ARPC_EVENT_CALLBACK_F*  handler = (ADM_ARPC_EVENT_CALLBACK_F*)map_get_val(eventHandlers,strKey,&valueSize);
		if(*handler != NULL)
		{
			D32S retCode = (*handler)(instanceId,(DCHAR*)data,dataSize);
			log_i("call eventHandlers instanceId:0X%04X, serviceId:0X%04X, eventId:0X%04X, retCode:%d",instanceId, servcieid, eventId , retCode);
		}
		
	}

    return;
}

static void adm_arpc_getInstnceInfo(DCHAR** strInstanceInfo, D32U* infoLength)
{
	json_object* instanceInfo  = json_object_new_object(); 
	json_object* serviceInfo = json_object_new_array();
	json_object* eventInfo = json_object_new_array();

	DCHAR strinstanceId[5] = {0};
	DCHAR strserviceId[5] = {0};
	DCHAR strmsgId[5] = {0};

	D16U instanceId = 0;
	D16U serviceId = 0;
	D16U msgId = 0;
	
	// instanceid, serviceid, methodid
	map_t* node = NULL;
    for (node = map_first(mehtodHandlers); node != NULL; node = map_next(node)) 
	{		
		(void)adm_memzero(strinstanceId, 5);
		(void)adm_memzero(strserviceId, 5);
		(void)adm_memzero(strmsgId, 5);
		
		char* servKey =  map_get_key(node);
		//log_i("servkey:%s", servKey);

		(void)adm_memcpy(strinstanceId, servKey, 4, 4);
		(void)adm_memcpy(strserviceId, servKey+4, 4, 4);
		(void)adm_memcpy(strmsgId, servKey+8, 4, 4);

		errno = 0;
		instanceId = (D16U)(strtol(strinstanceId, NULL, 16));
		if(0 != errno)
		{
			log_e("strtol, errno=%d, reason=%s",errno, strerror(errno));
		}

		errno = 0;
		serviceId = (D16U)(strtol(strserviceId, NULL, 16));
		if(0 != errno)
		{
			log_e("strtol, errNum=%d, reason=%s",errno, strerror(errno));
		}

		errno = 0;
		msgId = (D16U)(strtol(strmsgId, NULL, 16));
		if(0 != errno)
		{
			log_e("strtol, errNum=%d, reason=%s",errno, strerror(errno));
		}
		
		//sscanf( servKey, "%04X%04X%04X", &instanceId, &serviceId, &msgId );
			
		json_object* method = json_object_new_object();
		(void)json_object_object_add(method, "instanceId", json_object_new_int((D32S)instanceId));
        (void)json_object_object_add(method, "serviceId", json_object_new_int((D32S)serviceId));
        (void)json_object_object_add(method, "methodId", json_object_new_int((D32S)msgId));
 
        (void)json_object_array_add(serviceInfo,method);
       
    }
    (void)json_object_object_add(instanceInfo,"methodInfo",serviceInfo);

	node = NULL;
	
	// instanceid, serviceid, eventid
	pthread_mutex_lock(&subEventClientsMutex);
    for (node = map_first(subEventClients); node != NULL; node = map_next(node)) 
	{		
		(void)adm_memzero(strinstanceId, 5);
		(void)adm_memzero(strserviceId, 5);
		(void)adm_memzero(strmsgId, 5);
		
		DCHAR* servKey =  map_get_key(node);
		//log_i("event servkey:%s", servKey);

		(void)adm_memcpy(strinstanceId, servKey, 4, 4);
		(void)adm_memcpy(strserviceId, servKey+4, 4, 4);
		(void)adm_memcpy(strmsgId, servKey+8, 4, 4);

		errno = 0;
		instanceId = (D16U)(strtol(strinstanceId, NULL, 16));
		if(0 != errno)
		{
			log_e("strtol, errno=%d, reason=%s",errno, strerror(errno));
		}

		errno = 0;
		serviceId = (D16U)(strtol(strserviceId, NULL, 16));
		if(0 != errno)
		{
			log_e("strtol, errno=%d, reason=%s",errno, strerror(errno));
		}

		errno = 0;
		msgId = (D16U)(strtol(strmsgId, NULL, 16));
		if(0 != errno)
		{
			log_e("strtol, errno=%d, reason=%s",errno, strerror(errno));
		}
		
		//sscanf( servKey, "%04X%04X%04X", &instanceId, &serviceId, &msgId );
	 	
		json_object* event = json_object_new_object();
		(void)json_object_object_add(event, "instanceId", json_object_new_int((D32S)instanceId));
        (void)json_object_object_add(event, "serviceId", json_object_new_int((D32S)serviceId));
        (void)json_object_object_add(event, "eventId", json_object_new_int((D32S)msgId));
 
        (void)json_object_array_add(eventInfo,event);

       
    }

	pthread_mutex_unlock(&subEventClientsMutex);

    (void)json_object_object_add(instanceInfo,"eventInfo",eventInfo);

    //*strInstanceInfo = adm_strdup( json_object_to_json_string_length(instanceInfo, JSON_C_TO_STRING_PLAIN, (size_t *)infoLength) );

    *strInstanceInfo = adm_strdup(json_object_to_json_string(instanceInfo) );

	*infoLength = adm_strlen(*strInstanceInfo)+(D32U)1;
    
    (void)json_object_put(instanceInfo);
    
    return;
}

void adm_arpc_register(ADM_ARPC_PEER_T* peer)
{
	DCHAR* strRegInfo = NULL;
	
	ADM_ARPC_PACKAGE_T* heartPackage = adm_arpc_makePackageDef(peer->instanceId, 0,0,0, ADM_ARPC_MESSAGE_TYPE_REGISTER, NULL, 0);
	
	// (void)adm_arpc_pushToSendQueue(heartPackage, peer, 0, NULL);
	adm_arpc_add2SendQueue(heartPackage, peer, 0, NULL);
	
	return;
}

void adm_arpc_handlerRegister(ADM_ARPC_QUEUE_DATA_T* queueData)
{
	// 分配实例Id.
	D16U instanceId = queueData->package->packageHead.instanceId;
	if((D16U)0 == instanceId)
	{
		pthread_mutex_lock(&peerInfosMutex);
		instanceId = adm_arpc_allocInstanceId();
		pthread_mutex_unlock(&peerInfosMutex);
	}

	(void)adm_arpc_addInstancePeerInfo(instanceId, queueData->peer);
	log_i("reg instanceId : 0X%04X, ip:%s, port:%d", instanceId, queueData->peer->ip, queueData->peer->port);

	json_object* regAckInfo  = json_object_new_object();
	(void)json_object_object_add(regAckInfo,"instanceId",json_object_new_int((D32S)instanceId));

	const DCHAR* strRegAckInfo = json_object_to_json_string(regAckInfo);

	D32U msgLength = (D32U)( adm_strlen(strRegAckInfo)+(D32U)1 );
		
	ADM_ARPC_PACKAGE_T* respPackage = adm_arpc_makePackageDef(instanceId, 0,0,queueData->package->packageHead.sessionId, 
															ADM_ARPC_MESSAGE_TYPE_REGISTER_ACK, strRegAckInfo, msgLength+(D32U)1);															

	// (void)adm_arpc_pushToSendQueue(respPackage, queueData->peer,  0, NULL);
	adm_arpc_add2SendQueue(respPackage, queueData->peer,  0, NULL);

	(void)json_object_put(regAckInfo);

	///< 立马会收到心跳, 心跳会触发一次 历史事件订阅
	//adm_arpc_reSubEvent();

    return;
	
}

void adm_arpc_handlerRegisterAck(ADM_ARPC_QUEUE_DATA_T* queueData)
{
	// 获取自身instanceId, 与server instanceId
	json_object* regObj = json_tokener_parse(queueData->package->data);

	D16U instanceId = (D16U)(json_object_get_int(json_object_object_get(regObj,"instanceId")));

	if(1 > (D32S)instanceId)
	{
		log_e("instanceId get error");
		return;
	}

	adm_arpc_setHostInstnceId(instanceId);

	if(ADM_ECODE_SUCCESSED != adm_arpc_addInstancePeerInfo(queueData->package->packageHead.instanceId, queueData->peer))
    {
		log_e("add instanceId fail");	
    	return;
    }
	
	adm_arpc_sendHeart(queueData->peer);

	(void)json_object_put(regObj);

    return;
}


/*
{
	"methodInfo": [
		{
			"instanceId":1,
			"serviceId":2,
			"methodId":3
		}
	],
	"eventInfo": [
		{
			"instanceId":1,
			"serviceId":2,
			"eventId":3
		}
	]	
}	
*/
// send self's sevice, event
void adm_arpc_sendHeart(ADM_ARPC_PEER_T* peer)
{
    D32U msgLength = 0;
	DCHAR* strHeartInfo = NULL;
	adm_arpc_getInstnceInfo(&strHeartInfo, &msgLength);

	// log_i("heart info: %s, %d", strHeartInfo, msgLength);
	
	ADM_ARPC_PACKAGE_T* heartPackage = adm_arpc_makePackageDef(peer->instanceId, 0,0,0, ADM_ARPC_MESSAGE_TYPE_HEART, strHeartInfo, msgLength);
	adm_free(strHeartInfo);
	

	// (void)adm_arpc_pushToSendQueue(heartPackage, peer, 0, NULL);
	adm_arpc_add2SendQueue(heartPackage, peer, 0, NULL);
	
	return;
}

// server
void adm_arpc_handlerHeart(ADM_ARPC_QUEUE_DATA_T* queueData)
{
	// client count--
	// if(1 == queueData->peer->free)
	// {
	// 	log_w("when recv heart, peer closed");
	// 	return ;
	// }
	
	queueData->peer->heartCount = 0;	
	// log_i("%s", queueData->package->data);
	// 对端提供服务解析, instanceid, peerinfo bind.
	DCHAR strID[5] = {0};
	json_object* heartObj = json_tokener_parse(queueData->package->data);
	json_object* methodInfoObj = json_object_object_get(heartObj,"methodInfo");

	if(methodInfoObj != NULL)
	{
		for(D32U i = 0;i<json_object_array_length(methodInfoObj);i++)
	    {
	        json_object* methodObj = json_object_array_get_idx(methodInfoObj, i);
	        D16U methodInstanceId = (D16U)(json_object_get_int(json_object_object_get(methodObj,"instanceId")));
			
	        if(0 != (D32S)methodInstanceId)
	        {
				(void)sprintf(strID,"%04X",methodInstanceId);
	        	D32U valueSize = 0;
				// log_i("add method instance[%04X]", methodInstanceId);
				pthread_mutex_lock(&peerInfosMutex);
				ADM_ARPC_PEER_T** pPeerInfo = (ADM_ARPC_PEER_T**)map_get_val(peerInfos,strID,&valueSize);
				pthread_mutex_unlock(&peerInfosMutex);
				if(pPeerInfo == NULL)
				{
					log_i("add method instance[%04X] not found peer", methodInstanceId);
					adm_arpc_add_child_instance(queueData->peer, methodInstanceId);
				}
	        }
	    }
    }

	json_object* eventInfoObj = json_object_object_get(heartObj,"eventInfo");
	if(NULL != eventInfoObj)
	{
		for(D32U i = 0;i<json_object_array_length(eventInfoObj);i++)
	    {
	        json_object* eventObj = json_object_array_get_idx(eventInfoObj, i);
	        D16U eventInstanceId = (D16U)(json_object_get_int(json_object_object_get(eventObj,"instanceId")));
			
	        if(0 != (D32S)eventInstanceId)
	        {
				(void)sprintf(strID,"%04X",eventInstanceId);
	        	D32U valueSize = 0;
				// log_i("add event instance[%04X]", eventInstanceId);
				pthread_mutex_lock(&peerInfosMutex);
				ADM_ARPC_PEER_T** pPeerInfo = (ADM_ARPC_PEER_T**)map_get_val(peerInfos,strID,&valueSize);
				pthread_mutex_unlock(&peerInfosMutex);
				if(pPeerInfo == NULL)
				{
					log_i("add event instance[%04X] not found peer", eventInstanceId);
					adm_arpc_add_child_instance(queueData->peer, eventInstanceId);
				}
	        }
	    }
    }

	// adm_arpc_callInstanceBehaviorCb(queueData->peer, INSTANCE_BEHAVIOR_ONLINE);

	// send ack, with self's instance info
    D32U msgLength = 0;
	char* strHeartInfo = NULL;
	adm_arpc_getInstnceInfo(&strHeartInfo, &msgLength);

	//log_i("heart info: %s", strHeartInfo);
	
	ADM_ARPC_PACKAGE_T* respPackage = adm_arpc_makePackageDef(queueData->package->packageHead.instanceId, 0,0,queueData->package->packageHead.sessionId, 
															ADM_ARPC_MESSAGE_TYPE_HEART_ACK, strHeartInfo, (D32U)(msgLength+(D32U)1));	

	adm_free(strHeartInfo);	
	(void)json_object_put(heartObj);
	
	// (void)adm_arpc_pushToSendQueue(respPackage, queueData->peer,  0, NULL);
	adm_arpc_add2SendQueue(respPackage, queueData->peer,  0, NULL);

	// 重试订阅, 加次数限制
	// TODO: 
	adm_arpc_reSubInstanceIdEvent(queueData->package->packageHead.instanceId);
	//adm_arpc_reSubEvent();
	
	adm_arpc_callInstanceBehaviorCb(queueData->peer, INSTANCE_BEHAVIOR_ONLINE);
	
	return;
}

// client
void adm_arpc_handlerHeartAck(ADM_ARPC_QUEUE_DATA_T* queueData)
{
	queueData->peer->heartCount = 0;

	DCHAR strID[5] = {0};
	json_object* heartObj = json_tokener_parse(queueData->package->data);
	json_object* methodInfoObj = json_object_object_get(heartObj,"methodInfo");

	if(methodInfoObj != NULL)
	{
		for(D32U i = 0;i<json_object_array_length(methodInfoObj);i++)
	    {
	        json_object* methodObj = json_object_array_get_idx(methodInfoObj, i);
	        D16U methodInstanceId = (D16U)(json_object_get_int(json_object_object_get(methodObj,"instanceId")));
			
	        if((D32S)methodInstanceId != 0)
	        {
				(void)adm_snprintf(strID, sizeof(strID)/sizeof(strID[0]), "%04X",methodInstanceId);
	        	D32U valueSize = 0;
				// log_i("add method instance[%04X]", methodInstanceId);
				pthread_mutex_lock(&peerInfosMutex);
				ADM_ARPC_PEER_T** pPeerInfo = (ADM_ARPC_PEER_T**)map_get_val(peerInfos,strID,&valueSize);
				pthread_mutex_unlock(&peerInfosMutex);
				if(pPeerInfo == NULL)
				{
					log_i("add method instance[%04X] not foun peer", methodInstanceId);
					adm_arpc_add_child_instance(queueData->peer, methodInstanceId);
				}
	        }
	    }
    }

	json_object* eventInfoObj = json_object_object_get(heartObj,"eventInfo");
	if(eventInfoObj != NULL)
	{
		for(D32U i = 0;i<json_object_array_length(eventInfoObj);i++)
	    {
	        json_object* eventObj = json_object_array_get_idx(eventInfoObj, i);
	        D16U eventInstanceId = (D16U)(json_object_get_int(json_object_object_get(eventObj,"instanceId")));
			
	        if( (D16U)0 != eventInstanceId)
	        {
				(void)sprintf(strID,"%04X",eventInstanceId);
	        	D32U valueSize = 0;
				// log_i("add event instance[%04X]", eventInstanceId);
				pthread_mutex_lock(&peerInfosMutex);
				ADM_ARPC_PEER_T** pPeerInfo = (ADM_ARPC_PEER_T**)map_get_val(peerInfos,strID,&valueSize);
				pthread_mutex_unlock(&peerInfosMutex);
				if(pPeerInfo == NULL)
				{
					log_i("add event instance[%04X] not found peer", eventInstanceId);
					adm_arpc_add_child_instance(queueData->peer, eventInstanceId);
				}
	        }
	    }
    }

	(void)json_object_put(heartObj);

		// 重试订阅
	//adm_arpc_reSubEvent();
	adm_arpc_reSubInstanceIdEvent(queueData->package->packageHead.instanceId);

	adm_arpc_callInstanceBehaviorCb(queueData->peer, INSTANCE_BEHAVIOR_ONLINE);
	
	return;
}

void adm_arpc_addSubedEvent(D16U instanceId, D16U serviceId, D16U eventId)
{
	// 已经存在
	ADM_ARPC_SUBEVENT_NODE_T *p;
	pthread_mutex_lock(&subedEventMutex);
	list_for_each_entry(p, &subedEvent, ADM_ARPC_SUBEVENT_NODE_T, entry)
	{
	    if(p->pstEventInfo != NULL)
	    {
	    	if(p->pstEventInfo->instanceId == instanceId && 
	    		p->pstEventInfo->serviceId == serviceId && 
	    		p->pstEventInfo->eventId == eventId)
	    	{
				pthread_mutex_unlock(&subedEventMutex);
	    		return;
	    	}
	    }
	}
	pthread_mutex_unlock(&subedEventMutex);

	ADM_ARPC_SUBEVENT_NODE_T* eventNode = (ADM_ARPC_SUBEVENT_NODE_T*)adm_malloc(sizeof(ADM_ARPC_SUBEVENT_NODE_T));
	if(eventNode == NULL)
	{
		log_e("malloc eventNode fail");
		return;
	}

	eventNode->pstEventInfo = (ADM_ARPC_EVENT_INFO_T*)adm_malloc(sizeof(ADM_ARPC_EVENT_INFO_T));
	if(eventNode->pstEventInfo == NULL)
	{
		log_e("malloc pstEventInfo fail");
		return;
	}

	eventNode->pstEventInfo->instanceId = instanceId;
	eventNode->pstEventInfo->serviceId = serviceId;
	eventNode->pstEventInfo->eventId = eventId;
	
	pthread_mutex_lock(&subedEventMutex);
	list_add_tail(&eventNode->entry, &subedEvent);
	pthread_mutex_unlock(&subedEventMutex);
	
	return;
}


void adm_arpc_addWaitSubEventFromSubedEvent(ADM_ARPC_PEER_T* peer)
{
	// subsedEvent -> waitSubEvent
	ADM_ARPC_SUBEVENT_NODE_T* p;
	
	pthread_mutex_lock(&subedEventMutex);
	list_for_each_entry(p, &subedEvent, ADM_ARPC_SUBEVENT_NODE_T, entry)
	{	
	    if(p->pstEventInfo->instanceId == peer->instanceId)
	    {
	    	log_i("add to wait sub event: instanceId:0X%04X, serviceId:0X%04X, eventId:0X%04X", 
	    			p->pstEventInfo->instanceId, p->pstEventInfo->serviceId, p->pstEventInfo->eventId);

	    	ADM_ARPC_SUBEVENT_NODE_T* eventNode = (ADM_ARPC_SUBEVENT_NODE_T*)adm_malloc(sizeof(ADM_ARPC_SUBEVENT_NODE_T));

			(void)adm_memset(eventNode, 0x0, sizeof(ADM_ARPC_SUBEVENT_NODE_T), sizeof(ADM_ARPC_SUBEVENT_NODE_T));
			if(eventNode == NULL)
			{
				pthread_mutex_unlock(&subedEventMutex);
				log_e("malloc eventNode fail");
				return;
			}

			eventNode->pstEventInfo = (ADM_ARPC_EVENT_INFO_T*)adm_malloc(sizeof(ADM_ARPC_EVENT_INFO_T));
			(void)adm_memset(eventNode->pstEventInfo, 0x0, sizeof(ADM_ARPC_EVENT_INFO_T), sizeof(ADM_ARPC_EVENT_INFO_T));

			if(eventNode->pstEventInfo == NULL)
			{
				pthread_mutex_unlock(&subedEventMutex);
				log_e("malloc pstEventInfo fail");
				return;
			}

			eventNode->pstEventInfo->instanceId = p->pstEventInfo->instanceId;
			eventNode->pstEventInfo->serviceId = p->pstEventInfo->serviceId;
			eventNode->pstEventInfo->eventId = p->pstEventInfo->eventId;		
	    	
			pthread_mutex_lock(&waitSubEventMutex);
	    	list_add_tail(&eventNode->entry, &waitSubEvent);
			pthread_mutex_unlock(&waitSubEventMutex);
	    }

		// child instance

		if(NULL == peer->childInstancelist)
		{
			pthread_mutex_unlock(&subedEventMutex);
			log_e("peer->childInstancelist is null");
			return;
		}
		
	    ADM_ARPC_CHILD_INSTANCE_LIST_T *n;
	    list_for_each_entry(n, peer->childInstancelist, ADM_ARPC_CHILD_INSTANCE_LIST_T, entry)
		{
		    if(p->pstEventInfo->instanceId == n->instanceId)
		    {
		    	log_i("add to wait sub event: instanceId:0X%04X, serviceId:0X%04X, eventId:0X%04X", 
	    			p->pstEventInfo->instanceId, p->pstEventInfo->serviceId, p->pstEventInfo->eventId);
	    			
		    	ADM_ARPC_SUBEVENT_NODE_T* eventNode = (ADM_ARPC_SUBEVENT_NODE_T*)adm_malloc(sizeof(ADM_ARPC_SUBEVENT_NODE_T));
				(void)adm_memset(eventNode, 0x0, sizeof(ADM_ARPC_SUBEVENT_NODE_T), sizeof(ADM_ARPC_SUBEVENT_NODE_T));
				if(eventNode == NULL)
				{
					pthread_mutex_unlock(&subedEventMutex);
					log_e("malloc eventNode fail");
					return;
				}

				eventNode->pstEventInfo = (ADM_ARPC_EVENT_INFO_T*)adm_malloc(sizeof(ADM_ARPC_EVENT_INFO_T));
				(void)adm_memset(eventNode->pstEventInfo, 0x0, sizeof(ADM_ARPC_EVENT_INFO_T), sizeof(ADM_ARPC_EVENT_INFO_T));
				if(eventNode->pstEventInfo == NULL)
				{
					pthread_mutex_unlock(&subedEventMutex);
					log_e("malloc pstEventInfo fail");
					return;
				}

				eventNode->pstEventInfo->instanceId = p->pstEventInfo->instanceId;
				eventNode->pstEventInfo->serviceId = p->pstEventInfo->serviceId;
				eventNode->pstEventInfo->eventId = p->pstEventInfo->eventId;		
		    	
				pthread_mutex_lock(&waitSubEventMutex);
		    	list_add_tail(&eventNode->entry, &waitSubEvent);
				pthread_mutex_unlock(&waitSubEventMutex);
		    }
		}
	}

	pthread_mutex_unlock(&subedEventMutex);

	log_i("adm_arpc_addWaitSubEventFromSubedEvent end");

    return;
}

void adm_arpc_addWaitSubEvent(D16U instanceId, D16U serviceId, D16U eventId)
{
	// 已经存在
	ADM_ARPC_SUBEVENT_NODE_T *p;
	pthread_mutex_lock(&waitSubEventMutex);
	list_for_each_entry(p, &waitSubEvent, ADM_ARPC_SUBEVENT_NODE_T, entry)
	{
	    if(p->pstEventInfo != NULL)
	    {
	    	if(p->pstEventInfo->instanceId == instanceId && 
	    		p->pstEventInfo->serviceId == serviceId && 
	    		p->pstEventInfo->eventId == eventId)
	    	{
	    		//log_i("add wait sub event is exist, instanceId:0X%04X, serviceId:0X%04X, serviceId:0X%04X, eventId:0X%04X",
	    		//		instanceId, serviceId, eventId);
	    		pthread_mutex_unlock(&waitSubEventMutex);
				return;
	    	}
	    }
	}
	pthread_mutex_unlock(&waitSubEventMutex);

	ADM_ARPC_SUBEVENT_NODE_T* eventNode = (ADM_ARPC_SUBEVENT_NODE_T*)adm_malloc(sizeof(ADM_ARPC_SUBEVENT_NODE_T));
	if(eventNode == NULL)
	{
		log_e("malloc eventNode fail");
		return;
	}
	(void)adm_memset(eventNode, 0x0, sizeof(ADM_ARPC_SUBEVENT_NODE_T), sizeof(ADM_ARPC_SUBEVENT_NODE_T));
	

	eventNode->pstEventInfo = (ADM_ARPC_EVENT_INFO_T*)adm_malloc(sizeof(ADM_ARPC_EVENT_INFO_T));
	if(eventNode->pstEventInfo == NULL)
	{
		log_e("malloc pstEventInfo fail");
		return;
	}
	(void)adm_memset(eventNode->pstEventInfo, 0x0, sizeof(ADM_ARPC_EVENT_INFO_T), sizeof(ADM_ARPC_EVENT_INFO_T));

	eventNode->pstEventInfo->instanceId = instanceId;
	eventNode->pstEventInfo->serviceId = serviceId;
	eventNode->pstEventInfo->eventId = eventId;
	eventNode->retryCount = 0;
	
	pthread_mutex_lock(&waitSubEventMutex);
	list_add_tail(&eventNode->entry, &waitSubEvent);
	pthread_mutex_unlock(&waitSubEventMutex);
	
	return;
}

/*
{
	"servInfo": [
		{
			"instanceId": 1,
			"instanceInfo": [
				{
					"servId": 2,
					"servInfo": {
						
						"eventId": [4,
							5,
							6
						]
					}
				},
				{}
			]
		},
		{}
	]
}
*/

void adm_arpc_reSubInstanceIdEvent(D16U reSubInstanceId)
{

	// mult event subscriber
	if(1 == list_empty(&waitSubEvent))
	{
		return;
	}
	
	json_object* instanceInfo  = json_object_new_object(); 
	json_object* eventInfo = json_object_new_array();

	// map key peer fd, value instanceId
	root_t* multiPeer = map_new();

	pthread_mutex_lock(&waitSubEventMutex);
	ADM_ARPC_SUBEVENT_NODE_T *p;
	list_for_each_entry(p,&waitSubEvent, ADM_ARPC_SUBEVENT_NODE_T, entry)
	{
		// 暂时没有添加提供事件, 如没有提供, 不浪费资源 再请求
		if(p->pstEventInfo != NULL && (-1 == trySubEventCount || (trySubEventCount > 0 && p->retryCount <= trySubEventCount)))
		{
			p->retryCount += 1;
			
			// 不在线的instanceid, 不添加
			ADM_ARPC_PEER_T* peerinfo = adm_arpc_getPeerInfoByInstanceId(p->pstEventInfo->instanceId);
			if(peerinfo == NULL)
			{
				log_i("retry sub event peer is not online, instanceId:0X%04X, serviceId:0X%04X, eventId:0X%04X", 
					p->pstEventInfo->instanceId, p->pstEventInfo->serviceId, p->pstEventInfo->eventId);
				continue;
			}
	
			json_object* event = json_object_new_object();
			(void)json_object_object_add(event, "instanceId", json_object_new_int((D32S)(p->pstEventInfo->instanceId)));
			(void)json_object_object_add(event, "serviceId", json_object_new_int((D32S)(p->pstEventInfo->serviceId)));
			(void)json_object_object_add(event, "eventId", json_object_new_int((D32S)(p->pstEventInfo->eventId)));
	 
			(void)json_object_array_add(eventInfo,event);


			DCHAR strID[6] = {0};
			(void)sprintf(strID,"%d",peerinfo->fd);
			log_i("adm_arpc_releasePeer");
			adm_arpc_releasePeer(peerinfo);
			(void)map_put(multiPeer, strID, &(p->pstEventInfo->instanceId), (D32U)sizeof(p->pstEventInfo->instanceId) ); 
		}
	}
	pthread_mutex_unlock(&waitSubEventMutex);
	
	if((D32U)0 == json_object_array_length(eventInfo))
	{
		(void)json_object_put(eventInfo);
		(void)json_object_put(instanceInfo);
		map_free(multiPeer);
		return;
	}

	(void)json_object_object_add(instanceInfo,"subeventInfo",eventInfo);

	size_t infoLength = 0;
	DCHAR* strInstanceInfo = adm_strdup( json_object_to_json_string_length(instanceInfo, JSON_C_TO_STRING_PLAIN, &infoLength) );

	map_t* node = NULL;
	for (node = map_first(multiPeer); node != NULL; node = map_next(node)) 
	{
		D16U instanceId = *(D16U*)map_get_value(node);

		log_i("retry multievent sub, fd:%s, instanceid:0X%04X, sub instanceid:0X%04X", map_get_key(node), instanceId, reSubInstanceId);

		if((D32S)instanceId > 0 )
		{
			ADM_ARPC_PEER_T* reSubPeer = adm_arpc_getPeerInfoByInstanceId(reSubInstanceId);
			ADM_ARPC_PEER_T* peer = adm_arpc_getPeerInfoByInstanceId(instanceId);
			log_i("peer[%x] reSubPeer[%x]", peer, reSubPeer);
			if(DNULL != peer && DNULL != reSubPeer)
			{
				if (peer == reSubPeer)
				{
					if(ADM_ECODE_SUCCESSED != adm_arpc_subscriberMultiEvent_for_retry(instanceId, strInstanceInfo, (D32U)infoLength))
					{
						log_e("subMultiEcent fail");
						// continue;
					}
				}
			}

			if (DNULL != reSubPeer)
			{
				log_i("adm_arpc_releasePeer");
				adm_arpc_releasePeer(reSubPeer);
			}

			if (DNULL != peer)
			{
				log_i("adm_arpc_releasePeer");
				adm_arpc_releasePeer(peer);
			}
		}
	}
	
	(void)json_object_put(instanceInfo);

	map_free(multiPeer);

    return;
	
}


void adm_arpc_reSubEvent(void)
{

	// mult event subscriber
	if(1 == list_empty(&waitSubEvent))
	{
		return;
	}
	
	json_object* instanceInfo  = json_object_new_object(); 
	json_object* eventInfo = json_object_new_array();

	// map key peer fd, value instanceId
	root_t* multiPeer = map_new();

	pthread_mutex_lock(&waitSubEventMutex);
	ADM_ARPC_SUBEVENT_NODE_T *p;
    list_for_each_entry(p,&waitSubEvent, ADM_ARPC_SUBEVENT_NODE_T, entry)
	{
		// 暂时没有添加提供事件, 如没有提供, 不浪费资源 再请求
	    if(p->pstEventInfo != NULL && (-1 == trySubEventCount || (trySubEventCount > 0 && p->retryCount <= trySubEventCount)))
	    {
			p->retryCount += 1;
			
	    	// 不在线的instanceid, 不添加
	    	ADM_ARPC_PEER_T* peerinfo = adm_arpc_getPeerInfoByInstanceId(p->pstEventInfo->instanceId);
			if(peerinfo == NULL)
			{
				log_i("retry sub event peer is not online, instanceId:0X%04X, serviceId:0X%04X, eventId:0X%04X", 
	    			p->pstEventInfo->instanceId, p->pstEventInfo->serviceId, p->pstEventInfo->eventId);
				continue;
			}
	
	    	json_object* event = json_object_new_object();
			(void)json_object_object_add(event, "instanceId", json_object_new_int((D32S)(p->pstEventInfo->instanceId)));
	        (void)json_object_object_add(event, "serviceId", json_object_new_int((D32S)(p->pstEventInfo->serviceId)));
	        (void)json_object_object_add(event, "eventId", json_object_new_int((D32S)(p->pstEventInfo->eventId)));
	 
	        (void)json_object_array_add(eventInfo,event);


	        DCHAR strID[6] = {0};
			(void)sprintf(strID,"%d",peerinfo->fd);
			(void)map_put(multiPeer, strID, &(p->pstEventInfo->instanceId), (D32U)sizeof(p->pstEventInfo->instanceId) ); 
	    }
	}
	pthread_mutex_unlock(&waitSubEventMutex);
	
	if((D32U)0 == json_object_array_length(eventInfo))
	{
		(void)json_object_put(eventInfo);
		(void)json_object_put(instanceInfo);
		map_free(multiPeer);
		return;
	}

	(void)json_object_object_add(instanceInfo,"subeventInfo",eventInfo);

	size_t infoLength = 0;
    DCHAR* strInstanceInfo = adm_strdup( json_object_to_json_string_length(instanceInfo, JSON_C_TO_STRING_PLAIN, &infoLength) );

    map_t* node = NULL;
    for (node = map_first(multiPeer); node != NULL; node = map_next(node)) 
	{
		D16U instanceId = *(D16U*)map_get_value(node);

		log_i("retry multievent sub, fd:%s, instanceid:0X%04X", map_get_key(node), instanceId);

		if((D32S)instanceId > 0 )
	    {
			if(ADM_ECODE_SUCCESSED != adm_arpc_subscriberMultiEvent_for_retry(instanceId, strInstanceInfo, (D32U)infoLength))
			{
		    	log_e("subMultiEcent fail");
		    	continue;
			}
	    }
	}
    
    (void)json_object_put(instanceInfo);

	map_free(multiPeer);

    return;
    
}

void adm_arpc_sendHeartByInstanceId(D16U instanceId)
{
	
	ADM_ARPC_PEER_T* peerinfo = adm_arpc_getPeerInfoByInstanceId(instanceId);
	if(peerinfo != NULL)
	{
		adm_arpc_sendHeart(peerinfo);
		log_i("adm_arpc_releasePeer");
		adm_arpc_releasePeer(peerinfo);
	}
	
    return;
}

// extend
D32S adm_arpc_registerInstanceBehaviorCb(D16U instanceId, INSTANCE_BEHAVIOR_ENUM_E behavior,ADM_ARPC_INSTANCE_BEHAVIOR_CALLBACK_F findHandler, void* priv)
{
	D32S retCode = ADM_ECODE_FAILED;
	ADM_INSTANCE_BEHAVIOR_CB_T* onlineCb = (ADM_INSTANCE_BEHAVIOR_CB_T*)adm_malloc(sizeof(ADM_INSTANCE_BEHAVIOR_CB_T));
	if(onlineCb == NULL)
	{
		log_e("malloc eventNode fail");
		return retCode;
	}

	onlineCb->instanceId = instanceId;
	onlineCb->callCount = 0;
	onlineCb->behavior = behavior;
	onlineCb->handler = findHandler;
	onlineCb->priv = priv;
	
	list_add_tail(&onlineCb->entry, &instanceBehaviorHandler);

	log_w("add instance 0X%04X , behavior:%d, callback, priv:%p", instanceId, behavior, priv);

	return ADM_ECODE_SUCCESSED;
	
}

void adm_arpc_setSSLKeyPasswd(DCHAR *password)
{
#if defined(ARPC_SSL)
	adm_arpc_setKeyPasswd(password);
#else
	(void)password;/* avoid warnings */
#endif

    return;
}

/*!
 *******************************************************************************
 * arpc get software version number
 * \return	version format: ${PROJECT_NAME}_${ENV_PLATFORM}_${ADM_SDK_VERSION}_${COMPILE_DATE}_${GIT_COMMITID})
 *******************************************************************************
 */
DCHAR* adm_arpc_getSdkVersion(void)
{
	static char sdkVersion[128] = {0};
#ifdef ADM_ARPC_SDK_VERSION
    (void)snprintf(sdkVersion, sizeof(sdkVersion), "%s", ADM_ARPC_SDK_VERSION);
#else
    (void)snprintf(sdkVersion, sizeof(sdkVersion), "%s", ADM_ARPC_DEFAULT_SDK_VERSION);
#endif

    return sdkVersion;
}

// D32S adm_arpc_add2SendQueue(ADM_ARPC_PACKAGE_T* package, ADM_ARPC_PEER_T* peer, D8U syncFlag, void* priv)
// {
// 	D32S iRet = ADM_ECODE_FAILED;

// 	if (DTRUE == adm_arpc_retainPeer(peer))
// 	{
// 		iRet = adm_arpc_pushToSendQueue(package, peer, syncFlag, priv);
// 		if (ADM_ECODE_SUCCESSED != iRet)
// 		{
// 			adm_arpc_releasePeer(peer);
// 		}
// 	}

// 	return iRet;
// }

D32S adm_arpc_add2SendQueue(ADM_ARPC_PACKAGE_T* package, ADM_ARPC_PEER_T* peer, D8U syncFlag, void* priv)
{
	D32S iRet = ADM_ECODE_FAILED;
	log_i("adm_arpc_retainPeer");
	if (DTRUE == adm_arpc_retainPeer(peer))
	{
		iRet = adm_arpc_pushToSendQueue(package, peer, syncFlag, priv);
		if (ADM_ECODE_SUCCESSED != iRet)
		{
			log_i("adm_arpc_releasePeer");
			adm_arpc_releasePeer(peer);
		}
	}

	return iRet;
}


D32S adm_arpc_add2RecvQueue(ADM_ARPC_PACKAGE_T* package, ADM_ARPC_PEER_T* peer, void* priv)
{
	D32S iRet = ADM_ECODE_FAILED;
	log_i("adm_arpc_retainPeer");
	if (DTRUE == adm_arpc_retainPeer(peer))
	{
		if (1 == adm_arpc_pushToRecvQueue(package, peer, priv))
		{
			iRet = ADM_ECODE_SUCCESSED;
		}
		else
		{
			log_i("adm_arpc_releasePeer");
			adm_arpc_releasePeer(peer);
		}
	}

	return iRet;
}

D32S adm_arpc_addFileEvent(ADM_ARPC_PEER_T* peer, D32S mask, ADM_ARPC_FILE_PROC proc)
{
	D32S iRet = ADM_ECODE_FAILED;
	// log_i("adm_arpc_retainPeer");
	// if (DTRUE == adm_arpc_retainPeer(peer))
	// {
		iRet = adm_arpc_createFileEvent(peer->loop, peer->fd, mask, proc, peer);
	// 	if(ADM_ECODE_SUCCESSED != iRet) 
	// 	{
	// 		// adm_arpc_freePeer(peer);
	// 		log_i("adm_arpc_releasePeer");
	// 		adm_arpc_releasePeer(peer);
	// 	}
	// }

	return iRet;
}

// void adm_arpc_cleanFileEvent(ADM_ARPC_PEER_T* peer, D32S mask)
// {
// 	if(ADM_ECODE_SUCCESSED == adm_arpc_deleteFileEvent(peer->loop, peer->fd, mask)) 
// 	{
// 		// adm_arpc_freePeer(peer);
// 		log_i("adm_arpc_releasePeer");
// 		adm_arpc_releasePeer(peer);
// 	}
// }


// void adm_arpc_cleanAllFileEvent(ADM_ARPC_PEER_T* peer)
// {
// 	adm_arpc_cleanFileEvent(peer, AE_READABLE);
// 	adm_arpc_cleanFileEvent(peer, AE_WRITABLE);
// }
