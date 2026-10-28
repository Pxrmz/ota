#include <sys/uio.h>
#include "adm_typedefs.h"
#include "adm_memory.h"
#include "adm_queue.h"
#include "adm_com_errorCode.h"

#include "adm_arpc_buffer.h"
#include "adm_arpc_define.h"
#include "adm_arpc_log.h"

static ADM_QUEUE_T* sendQueue = NULL;
static ADM_QUEUE_T* recvQueue = NULL;
static ADM_ARPC_SENDDATA_SYNC_BUFFER_T* syncSendDataBuff = NULL;

void adm_arpc_initBuff(void)
{
	adm_arpc_initSendQueue();
	adm_arpc_initRecvQueue();
	adm_arpc_initSyncSendDataBuff();
}

void adm_arpc_deInitBuff(void)
{
	adm_arpc_deinitSendQueue();
	adm_arpc_deinitRecvQueue();
	adm_arpc_deinitSyncSendDataBuff();
}

void adm_arpc_print_buffPressures(void)
{
	// queue
	log_i("send queue size: %d",adm_queue_size(sendQueue));

	log_i("recv queue size: %d",adm_queue_size(recvQueue));
	
	// sync list
	log_i("sync lsit size: %d", syncSendDataBuff->count);

    return;
}

#ifndef SOCKET_BUFF
#define SOCKET_BUFF
D32U adm_arpc_readableBufferSize(ADM_ARPC_BUFFER_T* buffer)
{
	return buffer->writeIndex - buffer->readIndex;
}

D32U adm_arpc_writeableBufferSize(ADM_ARPC_BUFFER_T* buffer)
{
	return buffer->size - buffer->writeIndex;
}

static D32U adm_arpc_frontSpareSize(ADM_ARPC_BUFFER_T* buffer)
{
	return buffer->readIndex;
}

void adm_arpc_bufferAppend(ADM_ARPC_BUFFER_T *buffer, void *data, D32U size) 
{
    if (data != NULL) 
    {
        adm_arpc_makeRoom(buffer, size);

        (void)adm_memcpy(buffer->data + buffer->writeIndex, data, size, size);
        buffer->writeIndex += size;
    }
}

void adm_arpc_makeRoom(ADM_ARPC_BUFFER_T* buffer, D32U size)
{
	if(adm_arpc_frontSpareSize(buffer) + adm_arpc_writeableBufferSize(buffer) >= size)
	{
		D32U readable = adm_arpc_readableBufferSize(buffer);
		// move writeIndex - readIndex to  0 - readIndex, 

		//adm_memmove(buffer->data, buffer->data + buffer->readIndex, readable, readable);
        for (D32U i = 0; i < readable; i++) 
        {
            (void)adm_memcpy(buffer->data + i, buffer->data + buffer->readIndex + i, 1, 1);
        }

		buffer->readIndex = 0;
		buffer->writeIndex = readable;
	}
	else
	{
		D8S* tmpData = adm_malloc((size_t)(buffer->size) + (size_t)size);
		(void)adm_memset(tmpData, 0x0, (size_t)(buffer->size) + (size_t)size, (size_t)(buffer->size) + (size_t)size);
		(void)adm_memcpy(tmpData, buffer->data, buffer->writeIndex, buffer->writeIndex);
		adm_free(buffer->data);
		buffer->data = (DCHAR*)tmpData;
		buffer->size += size;
	}
}

ADM_ARPC_BUFFER_T* adm_arpc_allocBuffer(void)
{
	ADM_ARPC_BUFFER_T *buffer = NULL;
	do
	{
		buffer = adm_malloc(sizeof(ADM_ARPC_BUFFER_T));
	    if (buffer == NULL) 
	    {
	        break;
	    }
	    
	    buffer->data = adm_malloc((D32U)(DEFAULT_BUFF_SIZE));
	    if (buffer->data == NULL) 
	    {
	        break;
	    }
	    (void)adm_memset(buffer->data, 0x0, (D32U)(DEFAULT_BUFF_SIZE) , (D32U)(DEFAULT_BUFF_SIZE));
	    
	    buffer->size = (D32U)(DEFAULT_BUFF_SIZE);
	    buffer->readIndex = 0;
	    buffer->writeIndex = 0;
	}while(false);

	if( NULL == buffer || NULL == buffer->data )
	{
		adm_free(buffer->data);
		adm_free(buffer);
	}
    return buffer;
}

void adm_arpc_freeBuffer(ADM_ARPC_BUFFER_T *buffer)
{
	buffer->readIndex = 0;
	buffer->writeIndex = 0;
	buffer->size = 0;
	adm_free(buffer->data);

	adm_free(buffer);
}

#endif

#ifndef QUEUE_BUFF
#define QUEUE_BUFF

void adm_arpc_initSendQueue(void)
{
	sendQueue = adm_queue_new();
}

void adm_arpc_initRecvQueue(void)
{
	recvQueue = adm_queue_new();
}

D32S adm_arpc_pushToSendQueue(ADM_ARPC_PACKAGE_T* package, ADM_ARPC_PEER_T* peer, D8U syncFlag, void* priv)
{
	// ADM_ARPC_QUEUE_DATA_T* queueData = (ADM_ARPC_QUEUE_DATA_T*)adm_malloc(sizeof(ADM_ARPC_QUEUE_DATA_T));
	// if(NULL == queueData)
	// {
	// 	log_e("malloc queueData fail");
	// 	return ADM_ECODE_FAILED;
	// }
	ADM_ARPC_QUEUE_DATA_T queueData = {};
	queueData.package = package;
	queueData.peer = peer;
	queueData.priv = priv;

	if((D8U)ADM_ARPC_REQ_SYNC == syncFlag)
	{
		ADM_ARPC_SENDDATA_NODE_T* listNode = (ADM_ARPC_SENDDATA_NODE_T*)adm_malloc(sizeof(ADM_ARPC_SENDDATA_NODE_T));
		if(NULL == listNode)
		{
			log_e("malloc listNode fail");
			return ADM_ECODE_FAILED;
		}

		// queuedata will be free after socket send, keep one in sync list
		ADM_ARPC_QUEUE_DATA_T* syncQueueData = (ADM_ARPC_QUEUE_DATA_T*)adm_malloc(sizeof(ADM_ARPC_QUEUE_DATA_T));
		if(NULL == syncQueueData)
		{
			log_e("malloc queueData fail");
			return ADM_ECODE_FAILED;
		}	

		D32U packageSize = (D32U)sizeof(ADM_ARPC_PACKAGE_HEAD_T) + package->packageHead.length - (D32U)ADM_ARPC_PROTOCOL_HEAD_IN_LEN;
		syncQueueData->package = (ADM_ARPC_PACKAGE_T*)adm_malloc(packageSize);
		syncQueueData->sendTv = (struct timespec*)adm_malloc(sizeof(struct timespec));
		if(NULL == syncQueueData->package || NULL == syncQueueData->sendTv)
		{
			log_e("malloc queueData fail");
			adm_free(syncQueueData);
			return ADM_ECODE_FAILED;
		}		

		(void)adm_memset(syncQueueData->package, 0,packageSize, packageSize);
		(void)adm_memcpy(syncQueueData->package, package, packageSize, packageSize );
		syncQueueData->peer = peer;
		syncQueueData->priv = priv;

		(void)clock_gettime(CLOCK_MONOTONIC, syncQueueData->sendTv);
		
		listNode->sendData = syncQueueData;
		
		adm_arpc_addSyncSendDataSesssion(listNode);
	}


	// 内部memcpy data
	if (0 == adm_queue_push(sendQueue, &queueData, (D32U)sizeof(ADM_ARPC_QUEUE_DATA_T)))
	{
		return ADM_ECODE_FAILED;
	}
	
	// adm_free(queueData);
	
	return ADM_ECODE_SUCCESSED;
}

D32S adm_arpc_pushToRecvQueue(ADM_ARPC_PACKAGE_T* package, ADM_ARPC_PEER_T*  peer,void* priv)
{
	bool flag = 0;
	ADM_ARPC_QUEUE_DATA_T queueDdata;
	queueDdata.package = package;
	queueDdata.peer = peer;
	queueDdata.priv = priv;

	flag = adm_queue_push(recvQueue, &queueDdata, (D32U)sizeof(ADM_ARPC_QUEUE_DATA_T));
	if(flag)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

D32S adm_arpc_popSendQueue(ADM_ARPC_QUEUE_DATA_T** queueData)
{
	D32U dataSize = 0;
	bool flag = adm_queue_pop(sendQueue, (void**)queueData, &dataSize);

	if(flag)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

D32S adm_arpc_popRecvQueue(ADM_ARPC_QUEUE_DATA_T** queueData)
{
	
	D32U dataSize = 0;
	bool flag = adm_queue_pop(recvQueue, (void**)queueData, &dataSize);
	
	return (D32S)flag;
}


D32S adm_arpc_sendQueueSize(void)
{
	return adm_queue_size(sendQueue);
}

D32S adm_arpc_recvQueueSize(void)
{
	return adm_queue_size(recvQueue);
}

void adm_arpc_deinitSendQueue(void)
{
	adm_queue_free(sendQueue);
}

void adm_arpc_deinitRecvQueue(void)
{
	adm_queue_free(recvQueue);
}

#endif

#ifndef LIST_BUFF
#define LIST_BUFF

void adm_arpc_initSyncSendDataBuff(void)
{
	syncSendDataBuff = (ADM_ARPC_SENDDATA_SYNC_BUFFER_T*)adm_malloc(sizeof(ADM_ARPC_SENDDATA_SYNC_BUFFER_T));
	if(NULL == syncSendDataBuff)
	{
		log_e("syncSendDataBuff malloc error");
		return;
	}
	
	(void)pthread_mutex_init(&syncSendDataBuff->sendDataMutex, NULL);

	INIT_LIST_HEAD(&syncSendDataBuff->head);

	syncSendDataBuff->count = 0;
}

void adm_arpc_addSyncSendDataSesssion(ADM_ARPC_SENDDATA_NODE_T* sendDataNode)
{
	(void)pthread_mutex_lock(&syncSendDataBuff->sendDataMutex);
	
	list_add_tail(&sendDataNode->entry, &syncSendDataBuff->head);
	syncSendDataBuff->count++;

	(void)pthread_mutex_unlock(&syncSendDataBuff->sendDataMutex);
}

ADM_ARPC_QUEUE_DATA_T* adm_arpc_getSyncSendDataSesssion(D16U instanceId, D16U serviceId, D16U messageId, D16U sessionId)
{
	ADM_ARPC_QUEUE_DATA_T* data = NULL;
	ADM_ARPC_SENDDATA_NODE_T *p, *n;

	(void)pthread_mutex_lock(&syncSendDataBuff->sendDataMutex);
	
	list_for_each_entry_safe(p, n, &syncSendDataBuff->head, ADM_ARPC_SENDDATA_NODE_T, entry)
	{
	    if(p->sendData != NULL)
	    {
		
	    	if(p->sendData->package->packageHead.peerInstanceId == instanceId &&
	    		p->sendData->package->packageHead.serviceId == serviceId &&
	    		p->sendData->package->packageHead.messageId == messageId &&
	    		p->sendData->package->packageHead.sessionId == sessionId )
	    	{	
		    	data = p->sendData;
		    	list_del(&p->entry);
				adm_free(p);

				syncSendDataBuff->count--;
		    	break;
	    	}
	    }
	}

	(void)pthread_mutex_unlock(&syncSendDataBuff->sendDataMutex);

	return data;
}

void adm_arpc_freeSyncSendData(ADM_ARPC_QUEUE_DATA_T* sendData)
{
	if(sendData)
	{
		adm_free(sendData->package);
		adm_free(sendData->sendTv);
		adm_free(sendData);
	}
}

void adm_arpc_cleanTimeOutSendDataSession(void)
{
	struct timespec tv;
	(void)clock_gettime(CLOCK_MONOTONIC, &tv);
	
	// clear timeout aync date , list data.
	ADM_ARPC_QUEUE_DATA_T* data = NULL;
	ADM_ARPC_SENDDATA_NODE_T *p, *n;

	(void)pthread_mutex_lock(&syncSendDataBuff->sendDataMutex);
	list_for_each_entry_safe(p, n, &syncSendDataBuff->head, ADM_ARPC_SENDDATA_NODE_T, entry)
	{
		if(p->sendData != NULL)
		{
			if( tv.tv_sec - 300 > p->sendData->sendTv->tv_sec )
			{	
				log_i("clean timeout request buff, peer instanceId:0X%02X, serviceId:0X%04X, messageId:0X%04X, sessionId:%d", 
						p->sendData->package->packageHead.peerInstanceId,
						p->sendData->package->packageHead.serviceId, 
						p->sendData->package->packageHead.messageId,
						p->sendData->package->packageHead.sessionId);

				list_del(&p->entry);
				adm_arpc_freeSyncSendData(p->sendData);
				adm_free(p);

				syncSendDataBuff->count--;
			}
		}
	}
	(void)pthread_mutex_unlock(&syncSendDataBuff->sendDataMutex);

    return;
}

void adm_arpc_deinitSyncSendDataBuff(void)
{
	(void)pthread_mutex_destroy(&syncSendDataBuff->sendDataMutex);

    ADM_ARPC_SENDDATA_NODE_T *p, *n;
    list_for_each_entry_safe(p, n, &syncSendDataBuff->head, ADM_ARPC_SENDDATA_NODE_T, entry)
    {
		list_del(&p->entry);
		adm_arpc_freeSyncSendData(p->sendData);
		adm_free(p);
		
		syncSendDataBuff->count--;
	}
}

#endif // LIST_BUFF

