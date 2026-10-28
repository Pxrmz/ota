#ifndef ADM_ARPC_SERVER_H
#define ADM_ARPC_SERVER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <sys/time.h>
#include "adm_typedefs.h"
#include "adm_arpc_net.h"
#include "adm_arpc_eventLoop.h"
#include "adm_arpc_buffer.h"
#include "adm_arpc_threadPool.h"
#include "adm_map.h"
#if defined(ARPC_SSL)
#include "adm_com_sslAdapter.h"
#include "adm_arpc_ssl.h"
#include "adm_arpc_internal.h"
#endif

typedef struct ADM_ARPC_HOST
{
    ADM_ARPC_EVENT_LOOP_T* loop;
    D32S listen_fd;
    D32S port;
    D32S backlog;
    D32U max_peer_count;
    DCHAR* hostIp;
    DCHAR err_info[ANET_ERR_LEN];

	ADM_ARPC_THREAD_POOL_T* workPool;

   	D32S stop;

   	D8S hostMode;
   	D16U hostInstanceId;

   	root_t* offserServInfos; // offered serv info, key: instanceid, value: D32S list, servid&method / servid&eventid
   	
   	struct list_head peerHead;		// online peer
    D32S             peerCount;
   	struct list_head downPeerHead;	// down peer, try connect
    D32S            downPeerCount;

	pthread_attr_t procMessageAttr;
	pthread_condattr_t recvQueueCattr;
    pthread_cond_t  recvQueueCond;
    pthread_mutex_t recvQueueMutex;
    DBOOL bRecvQueueSignal;     ///< recvQueue是否已发signal

	pthread_mutex_t peerMutex; ///< 清理peer的时候, 发送消息等一下
    pthread_mutex_t downPeerMutex; ///< 清理peer的时候, 发送消息等一下

	pthread_mutex_t hostStatusMutex; ///< 程序退出

 	pthread_attr_t eventloopAttr;
 	pthread_attr_t eventloopTimeAttr;
#if defined(ARPC_SSL)
    DBOOL use_ssl;
    ADM_COM_SSL_CTX_T *ssl_ctx;
    ADM_COM_SSL_CTX_OPT_T ssl_ctx_opt;
#endif
} ADM_ARPC_HOST_T;

typedef struct ADM_ARPC_PEER
{
	struct list_head entry; //peer list
	struct list_head* childInstancelist; // map, put_value, 内部用的malloc, 操作list用的list*; 用值, 取出来地址不对.
	
    ADM_ARPC_EVENT_LOOP_T* loop;

    DCHAR ip[20];
    D16U port;
    D16U instanceId;
    D32S fd;
    D8S hostMode;		///< tcp client or server
    D8U	free;			///< 通道是否关闭
    struct timespec closeTv; ///< 通道关闭时间

    D16U heartCount;
    D16U tryConnectCount;
#if defined(ARPC_SSL)
    ADM_COM_SSL_T *ssl;
    ADM_ARPC_SSL_STATE_E sslState;
    pthread_condattr_t sslCattr;
    pthread_cond_t  sslCond;
    pthread_mutex_t sslMutex;
    DBOOL bSslIsSignal;
#endif
    pthread_mutex_t mutex;

    ADM_ARPC_BUFFER_T *read_buffer;
    ADM_ARPC_BUFFER_T *write_buffer;

    D32U    refCount;
} ADM_ARPC_PEER_T;

ADM_ARPC_HOST_T* amd_arpc_getHostInfo(void);

ADM_ARPC_PEER_T* adm_arpc_allocPeer(ADM_ARPC_PEER_T* copyPeer);

void adm_arpc_freePeer(ADM_ARPC_PEER_T *peer);

D32S adm_arpc_initserver(void);

D32S adm_arpc_startServer(void);

void adm_arpc_stopServer(void);


#ifdef __cplusplus
}
#endif
#endif // ADM_ARPC_TRANS_H
