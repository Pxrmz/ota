
#include "adm_string.h"
#include "adm_arpc_log.h"
#include "adm_arpc_protocol.h"
#include "adm_arpc_buffer.h"
#include "adm_arpc_internal.h"
#include "adm_memory.h"

static D16U sessionId = 1;
static D16U allocInstanceId = 2;	// server 分配给其他client
static D16U hostInstanceId = 0;		// 自身instanceId, 可自动生成,也可以从外部传入

void adm_arpc_setHostInstnceId(D16U instanceId)
{
	hostInstanceId = instanceId;
}

D16U adm_arpc_getHostInstanceId(void)
{
	return hostInstanceId;
}

D16U adm_arpc_allocInstanceId(void)
{
	D16U peerInstanceId = ++allocInstanceId;
	if (hostInstanceId == peerInstanceId)
	{
		peerInstanceId = ++allocInstanceId;
	}

	return peerInstanceId;
}

ADM_ARPC_PACKAGE_T* adm_arpc_makePackageDef(D16U peerInstanceId, D16U serviceId, 
												D16U messageId, D16U reqSessionId, D8U messageType, const DCHAR* data, 
												D32U dataSize)
{
	return adm_arpc_makePackage(hostInstanceId, peerInstanceId,serviceId,messageId, 
								reqSessionId, messageType, data, dataSize);
}

ADM_ARPC_PACKAGE_T* adm_arpc_makePackage(D16U instanceId, D16U peerInstanceId, D16U serviceId, 
												D16U messageId, D16U reqSessionId, D8U messageType, const DCHAR* data, 
												D32U dataSize)
{
	if(NULL == data && dataSize > (D32U)0)
	{
		log_e("package data error");
		return NULL;
	}

	ADM_ARPC_PACKAGE_T* package = (ADM_ARPC_PACKAGE_T*)adm_malloc(sizeof(ADM_ARPC_PACKAGE_T) + dataSize);
	if(NULL == package)
	{
		log_e("malloc package error");
		return NULL;
	}

	package->packageHead.serviceId = serviceId;
	package->packageHead.messageId = messageId;
	package->packageHead.length = (D32U)((D32U)ADM_ARPC_PROTOCOL_HEAD_IN_LEN + dataSize);
	package->packageHead.instanceId = instanceId;
	if(0 != (D32S)reqSessionId)
	{
		// resp
		package->packageHead.sessionId = reqSessionId;
	}
	else
	{
		// req
		D16U TmpSessionId = sessionId++;
		if(0 == TmpSessionId)
		{
			TmpSessionId = sessionId++;
		}
		package->packageHead.sessionId = TmpSessionId;
	}
	package->packageHead.protocolVersion = (D8U)ADM_ARPC_PROTOCOL_VERSION;
	package->packageHead.messageType = messageType;
	package->packageHead.peerInstanceId = peerInstanceId;
	package->packageHead.crc = 0x0;

	if(dataSize > (D32U)0)
	{
		(void)adm_memcpy(package->data, data, dataSize, dataSize);
	}
	
	return package;
}

// recv
D32S adm_arpc_decodePackage(ADM_ARPC_BUFFER_T* buffer, ADM_ARPC_PACKAGE_T** package)
{
	if(NULL == buffer)
	{
		log_i("buffer is null");
		return -1;
	}
	
	D32U buffLen = adm_arpc_readableBufferSize(buffer);
	if(buffLen == 0U)
	{
		// no data read
		return -1;
	}

	if(buffLen < (D32U)sizeof(ADM_ARPC_PACKAGE_HEAD_T))
	{
		log_e("decode package error, readable size: %d", buffLen);
		return -1;
	}

	ADM_ARPC_PACKAGE_HEAD_T* packageHead = (ADM_ARPC_PACKAGE_HEAD_T*)(buffer->data + buffer->readIndex);

	//log_d("recv datalen:%d ", packageHead->length );
	
	D32U packageLen = (D32U)sizeof(ADM_ARPC_PACKAGE_HEAD_T) + packageHead->length - (D32U)ADM_ARPC_PROTOCOL_HEAD_IN_LEN;
	// log_i("decode package, buf size[%u], package length[%u], data length[%u]", buffLen, packageLen, packageHead->length);
	if(buffLen < packageLen)
	{
		log_e("decode package error, buf size[%u] package length[%u] data length[%u]", buffLen, packageLen, packageHead->length);
		return -1;
	}

	ADM_ARPC_PACKAGE_T *new_package = adm_malloc(packageLen);
    (void)adm_memcpy(new_package, buffer->data + buffer->readIndex, packageLen, packageLen);
    buffer->readIndex += packageLen;

    *package = new_package;

    return 0;
}

D32S adm_arpc_encodePackage(ADM_ARPC_BUFFER_T* buffer, ADM_ARPC_PACKAGE_T* package)
{
	if(NULL == buffer)
	{	
		return -1;
	}
	
	D32U packageLen = (D32U)sizeof(ADM_ARPC_PACKAGE_T) + package->packageHead.length - (D32U)ADM_ARPC_PROTOCOL_HEAD_IN_LEN;	
	
	D32U wirteAbleSize = adm_arpc_writeableBufferSize(buffer);

	if(wirteAbleSize < packageLen )
	{
		adm_arpc_makeRoom(buffer, packageLen);
	}

	(void)adm_memcpy(buffer->data + buffer->writeIndex, package, packageLen, packageLen);
	buffer->writeIndex += packageLen;

	return 0;
}

ADM_ARPC_PACKAGE_T* adm_arpc_parsePackage(void* data, D32U dataSize)
{
	if(dataSize < (D32U)sizeof(ADM_ARPC_PACKAGE_HEAD_T))
	{
		log_e("dataSize less than head len");
		return NULL;
	}

	ADM_ARPC_PACKAGE_HEAD_T* packageHead = (ADM_ARPC_PACKAGE_HEAD_T*)data;

	D32U packageLen = (D32U)sizeof(ADM_ARPC_PACKAGE_HEAD_T) + packageHead->length - (D32U)ADM_ARPC_PROTOCOL_HEAD_IN_LEN;

	if(dataSize != packageLen)
	{
		log_e("dataSize not equal to package len");
		return NULL;
	}

	ADM_ARPC_PACKAGE_T* new_package = (ADM_ARPC_PACKAGE_T*)adm_malloc(packageLen);

	(void)adm_memcpy(new_package, data, packageLen, packageLen);

	return new_package;
}

//负责处理接收到的各种类型的 ARPC 数据包。该函数根据数据包的消息类型，调用相应的处理函数进行处理，是 ARPC 协议处理的中枢。
void* adm_arpc_ProcessPackage(void* data)
{
	ADM_ARPC_QUEUE_DATA_T* queueData = (ADM_ARPC_QUEUE_DATA_T*)data;
	if(NULL == queueData || NULL == queueData->package)
	{	
		return NULL;
	}

	ADM_ARPC_PACKAGE_T* recvPackage = queueData->package;

	log_i("trans recv: messagetype:%d, instanceId:0X%04x, serviceId:0X%04x, messageId:0X%04x, sessionId:%d, dataSize:%d", 
				recvPackage->packageHead.messageType, recvPackage->packageHead.instanceId,
				recvPackage->packageHead.serviceId, recvPackage->packageHead.messageId, recvPackage->packageHead.sessionId,
				recvPackage->packageHead.length - (D32U)ADM_ARPC_PROTOCOL_HEAD_IN_LEN);

	// 是不是本服务id
	
	switch(queueData->package->packageHead.messageType)
	{

		case ADM_ARPC_MESSAGE_TYPE_REGISTER:
		{
			// for server
			// instanceid check
			
			//log_i("req data:%s, size:%d", recvPackage->data, recvPackage->packageHead.length - ADM_ARPC_PROTOCOL_HEAD_IN_LEN);		

			adm_arpc_handlerRegister(queueData);
			
			break;		
		}
		case ADM_ARPC_MESSAGE_TYPE_REGISTER_ACK:
		{		
			// for client					
			
			//log_i("resp data:%s, size:%d", recvPackage->data, recvPackage->packageHead.length - ADM_ARPC_PROTOCOL_HEAD_IN_LEN);		
								
			adm_arpc_handlerRegisterAck(queueData);

			break;
		}
		case ADM_ARPC_MESSAGE_TYPE_METHOD_REQ:
		{
			// for server
			// instanceid check
			
			//log_i("req data:%s, size:%d", recvPackage->data, recvPackage->packageHead.length - ADM_ARPC_PROTOCOL_HEAD_IN_LEN);		

			adm_arpc_callServiceHandler(queueData);
			
			break;		
		}
		case ADM_ARPC_MESSAGE_TYPE_METHOD_RESP:
		{		
			// for client					
			
			//log_i("resp data:%s, size:%d", recvPackage->data, recvPackage->packageHead.length - ADM_ARPC_PROTOCOL_HEAD_IN_LEN);		
								
			adm_arpc_recvServerResp(queueData);

			break;
		}
		case ADM_ARPC_MESSAGE_TYPE_SUBEVENT:
		{
			// for server
			// single

			// multi
			if(recvPackage->packageHead.serviceId == (D16U)ADM_ARPC_SUB_MULTIEVENT_SERVICEID && recvPackage->packageHead.messageId == (D16U)ADM_ARPC_SUB_MULTIEVENT_MESSAGEID)
			{
				adm_arpc_addSubEventMultiClient(queueData);
			}
			else
			{
				adm_arpc_addSubEventClient(recvPackage->packageHead.peerInstanceId ,recvPackage->packageHead.serviceId, recvPackage->packageHead.messageId, 
										queueData->package->packageHead.sessionId, queueData->peer);
			}
			break;	
		}
		case ADM_ARPC_MESSAGE_TYPE_SUBEVENT_ACK:
		{
			// for client
			
			adm_arpc_recvServerResp(queueData);

			break;
		}
		case ADM_ARPC_MESSAGE_TYPE_EVENT_NOTIFY:
		{
			// for server

			// notify event 
			adm_arpc_callEventHandler(recvPackage->packageHead.instanceId, recvPackage->packageHead.serviceId, recvPackage->packageHead.messageId, 
										(D8S*)recvPackage->data, recvPackage->packageHead.length - (D32U)ADM_ARPC_PROTOCOL_HEAD_IN_LEN); 
			
			break;
		}
		case ADM_ARPC_MESSAGE_TYPE_HEART:
		{	
			adm_arpc_handlerHeart(queueData);
			break;
		}
		case ADM_ARPC_MESSAGE_TYPE_HEART_ACK:
		{
			adm_arpc_handlerHeartAck(queueData);
			break;
		}
		
		default:
			break;
	}

	log_i("adm_arpc_releasePeer");
	adm_arpc_releasePeer(queueData->peer);
	adm_free(queueData->package);
	adm_free(queueData);

	return NULL;
}

