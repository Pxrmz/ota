#ifndef ADM_ARPC_INTERNAL_H
#define ADM_ARPC_INTERNAL_H


#ifdef __cplusplus
extern "C" {
#endif

#include "adm_typedefs.h"
#include "adm_arpc_server.h"
#include "adm_arpc_service.h"

#include "adm_list.h"

// public

// client call, server recv heart call 
D32S adm_arpc_addInstancePeerInfo(D16U instanceId, ADM_ARPC_PEER_T* peer);

ADM_ARPC_PEER_T* adm_arpc_getPeerInfoByInstanceId(D16U instanceId);

void adm_arpc_delInstancePeerInfo(ADM_ARPC_PEER_T* peer);


// soa server call, recv, recv req method
void adm_arpc_callServiceHandler(ADM_ARPC_QUEUE_DATA_T* recvQueueData);

// soa client call, 

// 对端掉线重连后, 讲历史订阅的该实例的事件 加入 待订购事件.
void adm_arpc_addSubedEvent(D16U instanceId, D16U serviceId, D16U eventId);

void adm_arpc_addWaitSubEventFromSubedEvent(ADM_ARPC_PEER_T* peer);
void adm_arpc_addWaitSubEvent(D16U instanceId, D16U serviceId, D16U eventId);
void adm_arpc_reSubEvent(void);

void adm_arpc_reSubInstanceIdEvent(D16U reSubInstanceId);


// tcp server call, 客户端段线重连, 替换peer信息
D32S adm_arpc_replaceInstancePeerInfo(D16U instanceId, ADM_ARPC_PEER_T* peer);

// client recv, ack ok, callback.
void adm_arpc_recvServerResp(ADM_ARPC_QUEUE_DATA_T* recvQueueData);

void adm_arpc_addSubEventClient( D16U instanceId, D16U serviceId, D16U eventId, D16U sessionId, ADM_ARPC_PEER_T* client);

void adm_arpc_addSubEventMultiClient(ADM_ARPC_QUEUE_DATA_T* queueData);

void adm_arpc_delSubEvent(ADM_ARPC_PEER_T* peer);

// client recv, notify event
void adm_arpc_callEventHandler(D16U instanceId, D16U servcieid, D16U eventId, D8S* data, D32U dataSize);

// heart
void adm_arpc_sendHeart(ADM_ARPC_PEER_T* peer);

void adm_arpc_handlerHeart(ADM_ARPC_QUEUE_DATA_T* queueData);

void adm_arpc_handlerHeartAck(ADM_ARPC_QUEUE_DATA_T* queueData);

// register
void adm_arpc_register(ADM_ARPC_PEER_T* peer);

void adm_arpc_handlerRegister(ADM_ARPC_QUEUE_DATA_T* queueData);

void adm_arpc_handlerRegisterAck(ADM_ARPC_QUEUE_DATA_T* queueData);

// test
void adm_arpc_sendHeartByInstanceId(D16U instanceId);

// extend
//typedef enum INSTANCE_BEHAVIOR_ENUM INSTANCE_BEHAVIOR_ENUM_E;

void adm_arpc_clearInstanceCb(ADM_ARPC_PEER_T* peer, INSTANCE_BEHAVIOR_ENUM_E behavior);

void adm_arpc_callInstanceBehaviorCb(ADM_ARPC_PEER_T* peer, INSTANCE_BEHAVIOR_ENUM_E behavior);


void adm_arpc_addPeer(ADM_ARPC_PEER_T* peer);
void adm_arpc_removePeer(ADM_ARPC_PEER_T* peer);

void adm_arpc_addDownPeer(ADM_ARPC_PEER_T* peer);
void adm_arpc_removeDownPeer(ADM_ARPC_PEER_T* peer);

DBOOL adm_arpc_retainPeer(ADM_ARPC_PEER_T* peer);
void adm_arpc_releasePeer(ADM_ARPC_PEER_T* peer);
void adm_arpc_destroyPeer(ADM_ARPC_PEER_T* peer);


D32S adm_arpc_addFileEvent(ADM_ARPC_PEER_T* peer, D32S mask, ADM_ARPC_FILE_PROC proc);
// void adm_arpc_cleanFileEvent(ADM_ARPC_PEER_T* peer);
// D32S adm_arpc_cleanAllFileEvent(ADM_ARPC_PEER_T* peer);

D32S adm_arpc_add2SendQueue(ADM_ARPC_PACKAGE_T* package, ADM_ARPC_PEER_T* peer, D8U syncFlag, void* priv);
D32S adm_arpc_add2RecvQueue(ADM_ARPC_PACKAGE_T* package, ADM_ARPC_PEER_T* peer, void* priv);

#ifdef __cplusplus
}
#endif


#endif // ADM_ARPC_INTERNAL_H

