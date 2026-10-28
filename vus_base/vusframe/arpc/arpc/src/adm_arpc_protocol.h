#ifndef ADM_ARPC_PROTOCOL_H
#define ADM_ARPC_PROTOCOL_H

/**
协议处理相关
*/

#include "adm_typedefs.h"

#define HOSTMODE_CLIENT  0x01
#define HOSTMODE_SERVER  0x02


#define ADM_ARPC_MESSAGE_TYPE_METHOD_REQ  	0x01	// client
#define ADM_ARPC_MESSAGE_TYPE_METHOD_RESP 	0x02	// server
#define ADM_ARPC_MESSAGE_TYPE_SUBEVENT		0x03	// client
#define ADM_ARPC_MESSAGE_TYPE_SUBEVENT_ACK  0x04	// server
#define ADM_ARPC_MESSAGE_TYPE_EVENT_NOTIFY	0x05	// server
#define ADM_ARPC_MESSAGE_TYPE_HEART			0x06	// client
#define ADM_ARPC_MESSAGE_TYPE_HEART_ACK		0x07	// server
#define ADM_ARPC_MESSAGE_TYPE_REGISTER		0x08	// client
#define ADM_ARPC_MESSAGE_TYPE_REGISTER_ACK	0x09	// server
#define ADM_ARPC_MESSAGE_CLOSE_PEER			0x10	// 

#define ADM_ARPC_SUB_MULTIEVENT_SERVICEID 	0XFFFF  
#define ADM_ARPC_SUB_MULTIEVENT_MESSAGEID	0XFFFF  


#define ADM_ARPC_PROTOCOL_VERSION			0x01
#define ADM_ARPC_INTERFACE_VERSION			0x01

#define ADM_ARPC_PROTOCOL_HEAD_IN_LEN		12

typedef struct ADM_ARPC_BUFFER ADM_ARPC_BUFFER_T;
typedef struct ADM_ARPC_QUEUE_DATA ADM_ARPC_QUEUE_DATA_T;

typedef struct ADM_ARPC_PACKAGE_HEAD
{
	D16U serviceId;
	D16U messageId;	// methoidId, eventId
	D32U length;	// include below field,  instanceId,sessionId... 
	D16U instanceId;
	D16U peerInstanceId; // reserveData
	D16U sessionId;
	D8U  protocolVersion;
	D8U  messageType;
	D32S crc;

}ADM_ARPC_PACKAGE_HEAD_T;

typedef struct ADM_ARPC_PACKAGE
{
	ADM_ARPC_PACKAGE_HEAD_T packageHead;
	DCHAR data[0];
}ADM_ARPC_PACKAGE_T;


D16U adm_arpc_getHostInstanceId(void);

void adm_arpc_setHostInstnceId(D16U instanceId);

D16U adm_arpc_allocInstanceId(void);

///< 报文头自身实例Id默认使用 aprc instanceId
ADM_ARPC_PACKAGE_T* adm_arpc_makePackageDef(D16U peerInstanceId, D16U serviceId, 
												D16U messageId, D16U reqSessionId, D8U messageType, const DCHAR* data, 
												D32U dataSize);

///< 报文头自身实例Id使用业务层 instanceId
ADM_ARPC_PACKAGE_T* adm_arpc_makePackage(D16U instanceId, D16U peerInstanceId, D16U serviceId, 
												D16U messageId, D16U reqSessionId, D8U messageType, const DCHAR* data, 
												D32U dataSize);


D32S adm_arpc_decodePackage(ADM_ARPC_BUFFER_T* buffer, ADM_ARPC_PACKAGE_T** package);

D32S adm_arpc_encodePackage(ADM_ARPC_BUFFER_T* buffer, ADM_ARPC_PACKAGE_T* package);

ADM_ARPC_PACKAGE_T* adm_arpc_parsePackage(void* data, D32U dataSize);

//void adm_arpc_ProcessPackage(ADM_ARPC_QUEUE_DATA_T* queueData);
void* adm_arpc_ProcessPackage(void* data);

#endif // ADM_ARPC_PROTOCOL_H
