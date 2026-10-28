#ifndef ADM_ARPC_BUFFER_H
#define ADM_ARPC_BUFFER_H

#include <sys/time.h>
#include <pthread.h>
#include "adm_typedefs.h"
#include "adm_list.h"
#include "adm_arpc_protocol.h"


typedef struct ADM_ARPC_PEER ADM_ARPC_PEER_T;
typedef struct ADM_ARPC_EVENT_LOOP ADM_ARPC_EVENT_LOOP_T;

void adm_arpc_initBuff(void);
void adm_arpc_deInitBuff(void);

void adm_arpc_print_buffPressures(void);


// buffer
typedef struct ADM_ARPC_BUFFER
{
    DCHAR *data;
    D32U size;
    D32U readIndex;
    D32U writeIndex;
} ADM_ARPC_BUFFER_T;

D32U adm_arpc_readableBufferSize(ADM_ARPC_BUFFER_T* buffer);

D32U adm_arpc_writeableBufferSize(ADM_ARPC_BUFFER_T* buffer);

void adm_arpc_bufferAppend(ADM_ARPC_BUFFER_T *buffer, void *data, D32U size);

void adm_arpc_makeRoom(ADM_ARPC_BUFFER_T* buffer, D32U size);

ADM_ARPC_BUFFER_T* adm_arpc_allocBuffer(void);

void adm_arpc_freeBuffer(ADM_ARPC_BUFFER_T *buffer);

// queue handler
typedef struct ADM_ARPC_CLIENT_PRIVDATA
{
	pthread_cond_t  syncCond;
    pthread_mutex_t syncMutex;
    DBOOL bSyncIsSignal;
    DBOOL bCallerTimeout;
	DCHAR** respData;
	D32U* dataSize;
}ADM_ARPC_CLIENT_PRIVDATA_T;

typedef struct ADM_ARPC_QUEUE_DATA
{
	ADM_ARPC_PACKAGE_T* package;
	ADM_ARPC_PEER_T*  peer;
	struct timespec* sendTv;
	void* priv; // for client, sendqueue : server fd, mutex, cond ; recvqueue : server fd, mutex, cond 
}ADM_ARPC_QUEUE_DATA_T;

void adm_arpc_initSendQueue(void);
void adm_arpc_initRecvQueue(void);

// syncFlag 1 sync, 0 async
#define ADM_ARPC_REQ_SYNC  1
#define ADM_ARPC_REQ_ASYNC 0

D32S adm_arpc_pushToSendQueue(ADM_ARPC_PACKAGE_T* package, ADM_ARPC_PEER_T*  peer, D8U syncFlag, void* priv);
D32S adm_arpc_pushToRecvQueue(ADM_ARPC_PACKAGE_T* package, ADM_ARPC_PEER_T*  peer,void* priv);

D32S adm_arpc_popSendQueue(ADM_ARPC_QUEUE_DATA_T** queueData);
D32S adm_arpc_popRecvQueue(ADM_ARPC_QUEUE_DATA_T** queueData);

D32S adm_arpc_sendQueueSize(void);
D32S adm_arpc_recvQueueSize(void);

void adm_arpc_deinitSendQueue(void);
void adm_arpc_deinitRecvQueue(void);

/*--------------list begin-----------------*/
// list handler, for async request

typedef struct ADM_ARPC_SENDDATA_NODE{
    struct list_head entry;
    ADM_ARPC_QUEUE_DATA_T* sendData;
}ADM_ARPC_SENDDATA_NODE_T;

typedef struct ADM_ARPC_SENDDATA_SYNC_BUFFER{
	pthread_mutex_t sendDataMutex;
    struct list_head head;
    D32S count;
}ADM_ARPC_SENDDATA_SYNC_BUFFER_T;

void adm_arpc_freeSyncSendData(ADM_ARPC_QUEUE_DATA_T* sendData);

void adm_arpc_initSyncSendDataBuff(void);

void adm_arpc_addSyncSendDataSesssion(ADM_ARPC_SENDDATA_NODE_T* sendDataNode);

ADM_ARPC_QUEUE_DATA_T* adm_arpc_getSyncSendDataSesssion(D16U instanceId, D16U serviceId, D16U messageId, D16U sessionId);

void adm_arpc_cleanTimeOutSendDataSession(void);

void adm_arpc_deinitSyncSendDataBuff(void);

/*--------------list end-----------------*/

#endif // ADM_ARPC_BUFFER_H


