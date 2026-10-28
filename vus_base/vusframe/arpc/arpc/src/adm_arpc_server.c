#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <sys/time.h>
#include <errno.h>
#include <sys/uio.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "adm_queue.h"
#include "adm_arpc_buffer.h"
#include "adm_list.h"
#include "adm_arpc_eventLoop.h"
#include "adm_arpc_net.h"


#include "adm_arpc_server.h"
#include "adm_arpc_eventLoop.h"
#include "adm_arpc_net.h"
#include "adm_com_errorCode.h"
#include "adm_arpc_define.h"
#include "adm_typedefs.h"
#include "adm_memory.h"
#include "adm_string.h"
#include "adm_arpc_log.h"
#include "adm_arpc_protocol.h"
#include "adm_arpc_buffer.h"
#include "adm_cfg_api.h"
#include "adm_arpc_internal.h"
#include "adm_arpc_service.h"
#include "adm_arpc_select.h"
#if defined(ARPC_SSL)
#include "adm_arpc_ssl.h"
#endif
#include "adm_arpc_threadCond.h"

//static ADM_ARPC_HOST_T *host = NULL;
static ADM_ARPC_HOST_T *host = NULL;

static D32S tryConnTime = 0;		
static D32S tryConnInterval = 0;
static D32S heartCount = 0;		
static D32S heartInterval = 0;


ADM_ARPC_PEER_T* adm_arpc_allocPeer(ADM_ARPC_PEER_T* copyPeer)
{
    ADM_ARPC_PEER_T* peer = NULL;
    do
    {
		peer =  adm_malloc(sizeof(ADM_ARPC_PEER_T));
		if (peer == NULL) 
		{
		    break;
		}

		if(copyPeer != NULL)
		{
			peer->loop = copyPeer->loop;
			peer->fd = copyPeer->fd;

			(void)adm_memcpy(peer->ip, copyPeer->ip, 20, 20);
			peer->port = copyPeer->port;

			peer->instanceId = copyPeer->instanceId;
			peer->hostMode = copyPeer->hostMode;
		}
		else
		{
			peer->loop = NULL;
			peer->fd = -1;

			(void)adm_memset(peer->ip, 0x0, 20, 20);
			peer->port = 0;
			
			peer->instanceId = 0;
		}
		peer->read_buffer = adm_arpc_allocBuffer();
		peer->write_buffer = adm_arpc_allocBuffer();
		if (peer->read_buffer == NULL || peer->write_buffer == NULL) 
		{
		    break;
		}

		peer->heartCount = 0;
		peer->tryConnectCount = 0;
		peer->free = 0;
		peer->refCount = 1;

		peer->childInstancelist = (struct list_head*)adm_malloc(sizeof(struct list_head));
		(void)adm_memset(peer->childInstancelist, 0x0, sizeof(struct list_head), sizeof(struct list_head));
		INIT_LIST_HEAD(peer->childInstancelist);

		pthread_mutex_init(&peer->mutex, NULL);
		
    }while(FALSE);
    
    if (peer == NULL || peer->read_buffer == NULL || peer->write_buffer == NULL ) 
    {
        adm_free(peer->read_buffer);
        adm_free(peer->write_buffer);
        adm_free(peer);
		peer = NULL;
    }

    return peer;
}

void adm_arpc_freePeer(ADM_ARPC_PEER_T *peer)
{	
	// (void)pthread_mutex_lock(&host->peerMutex);

	// peer->free = 1;
	
	// trans
    // if (peer->fd > 0) 
    // {
    //     (void)adm_arpc_deleteFileEvent(peer->loop, peer->fd, AE_READABLE);
    //     //adm_arpc_deleteFileEvent(peer->loop, peer->fd, AE_WRITABLE); //写事件, 直接socket发送
    //     //close(peer->fd);
	// }

	// (void)close(peer->fd);
	log_i("adm_arpc_freePeer");
	// buffer
    adm_arpc_freeBuffer(peer->read_buffer);
    adm_arpc_freeBuffer(peer->write_buffer);

#if defined(ARPC_SSL)
	if(host->use_ssl == TRUE)
	{
		adm_arpc_sslFree(peer);
	}
#endif
	peer->write_buffer = NULL;
	peer->read_buffer = NULL;
    
    (void)adm_memset(peer->ip, 0, 20, 20);
    peer->port = 0;

	// ///< call offline callback
	// adm_arpc_callInstanceBehaviorCb(peer, INSTANCE_BEHAVIOR_OFFLINE);
	
    // // soa app, clear res
	// ///< remove client subed event, reconnect, sub again 
	// adm_arpc_delSubEvent(peer);

	// ///< 待重新订购事件, 历史订购事件加入待订购事件.			
	// adm_arpc_addWaitSubEventFromSubedEvent(peer);

	// ///< 清除在线回调
	// adm_arpc_clearInstanceCb(peer, INSTANCE_BEHAVIOR_ONLINE);

	// ///< remove instanceId, socket info bind relation
    // adm_arpc_delInstancePeerInfo(peer);

	// (void)pthread_mutex_unlock(&host->peerMutex);

	///< 此时free, 后面待发送消息, 内核缓存中接收消息,将访问野指针peer资源, 定时器延时删除
	(void)clock_gettime(CLOCK_MONOTONIC, &(peer->closeTv));

	///< 对端是服务端掉线，清楚自身实例Id, 等server重启后重新分配
	if(HOSTMODE_SERVER == (peer->hostMode & HOSTMODE_SERVER) )
	{
		// adm_arpc_setHostInstnceId(0);
	}

	pthread_mutex_destroy(&peer->mutex);
	
    adm_free(peer);

    return;
}

static void adm_arpc_writeEventHandler(ADM_ARPC_EVENT_LOOP_T *loop, D32S fd, void *data, D32S mask)
{	
	ADM_ARPC_PEER_T *peer = (ADM_ARPC_PEER_T *)data;
    ADM_ARPC_BUFFER_T *wbuffer = peer->write_buffer;

	/* peer->write_buffer相关数据的操作在同一线程中处理的，不需要加锁也能保证数据的正确性 */
    int data_size = (int)adm_arpc_readableBufferSize(wbuffer);
    if (data_size == 0) 
    {
        adm_arpc_DelEvent(peer->loop, peer->fd, AE_WRITABLE);
        return;
    }

    int writen = anetWrite(peer->fd, (char *)wbuffer->data + wbuffer->readIndex, data_size);
    if (writen > 0) 
    {
        wbuffer->readIndex += (D32U)writen;
    }
    if (adm_arpc_readableBufferSize(wbuffer) == 0U) 
    {
        adm_arpc_DelEvent(peer->loop, peer->fd, AE_WRITABLE);
    }

    return;
}

static D32S adm_arpc_socketBufferRead(ADM_ARPC_PEER_T *peer, D32S fd)
{
	DCHAR additional_buf[INIT_BUFF_SIZE] = {0};
    D32S result = -1;

    //log_d("recv before, read buffer: readidx: %d, writeidx: %d, len: %d", buffer->readIndex, 
	//						buffer->writeIndex,buffer->size);

    // 在锁内获取 buffer 并检查有效性
    (void)pthread_mutex_lock(&peer->mutex);
    if (1 == peer->free || NULL == peer->read_buffer)
    {
        (void)pthread_mutex_unlock(&peer->mutex);
        log_w("peer freed or read_buffer invalid in socketBufferRead");
        return -1;
    }

    ADM_ARPC_BUFFER_T *buffer = peer->read_buffer;
    D32U writeable_len = adm_arpc_writeableBufferSize(buffer);
	
	struct iovec vec[2];
	vec[0].iov_base = buffer->data + buffer->writeIndex;
	vec[0].iov_len = writeable_len;
	vec[1].iov_base = additional_buf;
	vec[1].iov_len = INIT_BUFF_SIZE;

    // 保存 writeIndex 用于后续更新
    D32U saved_writeIndex = buffer->writeIndex;
    // 解锁后再调用系统调用（避免在锁内阻塞）
    (void)pthread_mutex_unlock(&peer->mutex);

    errno = 0;
    ssize_t read_result = readv(fd, vec, 2);
    if(-1 == (D32S)read_result  && 0 != errno)
	{
		log_e(" errno=%d, reason=%s",errno, strerror(errno));
	}

    // 重新加锁更新 buffer
    (void)pthread_mutex_lock(&peer->mutex);

    // 检查 buffer 是否仍然有效
    if (1 == peer->free || NULL == peer->read_buffer || peer->read_buffer != buffer)
    {
        (void)pthread_mutex_unlock(&peer->mutex);
        log_w("peer freed or read_buffer changed during socket read, instanceId:0X%04X",
              peer->instanceId);
        // 丢弃这次读取的数据
        return -1;
    }

    if(read_result  > 0)
	{
		if((D32U)read_result  <= writeable_len)
		{
            buffer->writeIndex = saved_writeIndex + (D32U)read_result;
        }
		else
		{
			buffer->writeIndex = buffer->size;
            adm_arpc_bufferAppend(buffer, additional_buf, (D32U)read_result - writeable_len);
        }
        result = (D32S)read_result;
    }
	else if(read_result  == 0)
	{
        result = 0;
	}
	else
	{		
		log_i("read fd %d,	errno %d, reason=%s", fd, errno, strerror(errno));
        result = (D32S)read_result;
    }

    (void)pthread_mutex_unlock(&peer->mutex);
    return (D32S)result;
}


static void adm_arpc_readEventHandler(ADM_ARPC_EVENT_LOOP_T *loop, int fd, void *data, int mask)
{
	ADM_ARPC_PEER_T *peer = (ADM_ARPC_PEER_T *)data;
	
	// (void)pthread_mutex_lock(&host->peerMutex);
	// if(1 == peer->free)
	// {
	// 	log_w("when read, peer closed");
	// 	(void)pthread_mutex_unlock(&host->peerMutex);
	// 	return;
	// }
	// (void)pthread_mutex_unlock(&host->peerMutex);

    (void)pthread_mutex_lock(&peer->mutex);
    if (1 == peer->free)
    {
        log_w("when read, peer closed");
        (void)pthread_mutex_unlock(&peer->mutex);
        return;
    }
    (void)pthread_mutex_unlock(&peer->mutex);

    // 保存 fd，因为系统调用期间 peer 可能被销毁
    int saved_fd = fd;

    // 函数内部会处理锁和 buffer 保护
    D32S iret = adm_arpc_socketBufferRead(peer, saved_fd);

    if(iret >0)
	{
		ADM_ARPC_PACKAGE_T* reqPackage = NULL;
		ADM_ARPC_PACKAGE_T* respPackage = NULL;

		while(true)
		{
            // Check peer status in decode loop to prevent accessing freed peer
            (void)pthread_mutex_lock(&peer->mutex);
            if (1 == peer->free || NULL == peer->read_buffer)
            {
                (void)pthread_mutex_unlock(&peer->mutex);
                log_w("peer freed or read_buffer invalid during decode, stop decoding");
                break;
            }

            // 在锁内获取当前的 rbuffer（可能在循环中被替换）
            ADM_ARPC_BUFFER_T *rbuffer = peer->read_buffer;
            // 在锁内调用 decodePackage，保护 buffer 的访问和修改
            // 注意：adm_malloc 可能阻塞，但保护 buffer 不被释放更重要
            if( 0 != adm_arpc_decodePackage(rbuffer, &reqPackage) )
			{
                (void)pthread_mutex_unlock(&peer->mutex);
                log_w("decode package error");
                break;
			}
            (void)pthread_mutex_unlock(&peer->mutex);

            (void)pthread_mutex_lock(&host->recvQueueMutex);	
			// send in queue.
			void* priv = NULL;
			if(ADM_ECODE_SUCCESSED != adm_arpc_add2RecvQueue(reqPackage, peer, priv))
			{
				log_e("push data to queue error");
			}

            host->bRecvQueueSignal = DTRUE;
			(void)pthread_cond_signal(&host->recvQueueCond);
			(void)pthread_mutex_unlock(&host->recvQueueMutex);
		}
	}
	else
	{			
		log_i("peer close,instaceId:0X%04X,  %s:%u, fd:%d", peer->instanceId, peer->ip, peer->port, peer->fd);
        // 在锁内保存 peer 信息，避免在锁外访问
        (void)pthread_mutex_lock(&peer->mutex);
        D16U saved_instanceId = peer->instanceId;
        D16U saved_port = peer->port;
        D8S saved_hostMode = peer->hostMode;
        char saved_ip[20] = {0};
        (void)adm_memcpy(saved_ip, peer->ip, 20, 20);
        (void)pthread_mutex_unlock(&peer->mutex);

        log_i("peer close,instaceId:0X%04X,  %s:%u, fd:%d", saved_instanceId, saved_ip, saved_port, fd);
        if (0 == (D32S)saved_port)
        {
			return;
		}
		
		ADM_ARPC_PEER_T* retryPeer = DNULL;
		///< 客户端掉线不加入掉线列表
        if (HOSTMODE_SERVER == (saved_hostMode & HOSTMODE_SERVER))
        {
			///< copy to down peer list, try re connect
			retryPeer = adm_arpc_allocPeer(peer);
		}
		// (void)close(peer->fd);

		///< 留在链表中, 后面删除
		//list_del(&peer->entry); 
		// adm_arpc_freePeer(peer);	
		// log_i("adm_arpc_releasePeer");
		// adm_arpc_releasePeer(peer);
		adm_arpc_removePeer(peer);
		log_i("adm_arpc_destroyPeer");
		
		adm_arpc_destroyPeer(peer);


		if (DNULL != retryPeer)
		{
			// pthread_mutex_lock(&host->downPeerMutex);
			// list_add_tail(&retryPeer->entry, &host->downPeerHead);
			// pthread_mutex_unlock(&host->downPeerMutex);
			adm_arpc_addDownPeer(retryPeer);
		}

	/*		
		for(int i =0; i< host->max_peer_count; i++)
		{
			if(host->loop->events[i].mask != AE_NONE)
			{
				log_w("left fd: %d, mask:%d ", i, host->loop->events[i].mask );
			}
		}
	*/	

	}
	
	return;
}

static void adm_arpc_acceptTcpHandler(ADM_ARPC_EVENT_LOOP_T *loop, int fd, void *data, int mask)
{
    DCHAR cip[64] = {0};
    D16U cport;

    ADM_ARPC_HOST_T *tmpHost = (ADM_ARPC_HOST_T *)data;

    int cfd = anetTcpAccept(NULL, fd, cip, (socklen_t)(sizeof(cip)), &cport);
    if (cfd != -1) 
    {
        log_i("accepted ip %s:%d, fd:%d", cip, cport, cfd);
        (void)anetNonBlock(NULL, cfd);
        (void)anetEnableTcpNoDelay(NULL, cfd);
        ADM_ARPC_PEER_T *peer = adm_arpc_allocPeer(NULL);
        if (NULL == peer) 
        {
            log_i("alloc peer error...close socket\n");
            (void)close(fd);
            return;
        }

        peer->loop = loop;
        peer->fd = cfd;
        peer->hostMode = (D8S)HOSTMODE_CLIENT;
        (void)adm_memcpy(peer->ip, cip, adm_strlen(cip), adm_strlen(cip));
	    peer->port = cport;
        
		// pthread_mutex_lock(&tmpHost->peerMutex);
        // list_add_tail(&peer->entry, &tmpHost->peerHead);
		// pthread_mutex_unlock(&tmpHost->peerMutex);

		adm_arpc_addPeer(peer);
		log_i("adm_arpc_retainPeer");
		adm_arpc_retainPeer(peer);
        if (ADM_ECODE_SUCCESSED != adm_arpc_addFileEvent(peer, AE_READABLE, adm_arpc_readEventHandler)) 
        {
            log_i("create socket readable event error, close it.");
			adm_arpc_removePeer(peer);
			log_i("adm_arpc_releasePeer");
			adm_arpc_releasePeer(peer);
			log_i("adm_arpc_destroyPeer");
            adm_arpc_destroyPeer(peer);
        }
    }

    return;
}

// TODO: 根据peer 设置下线的动作.
static ADM_ARPC_PEER_T* adm_arpc_connectServer(DCHAR* ip, D16U port, D16U instanceId)
{	
	ADM_ARPC_PEER_T* peer = NULL;
	do
	{
		DCHAR err[ANET_ERR_LEN];
        int connfd = ANET_ERR;

#if 0
        /** 非阻塞式connect,需要正确处理EINPROGRESS信号,个人建议是：结合poll控制非阻塞的超时时间,EINPROGRESS错误
            超过规定时间按照connect失败处理,并close掉套接字,上层进行重试 */
        #define ADM_ARPC_CONNECT_COUNT 3
        #define ADM_ARPC_CONNECT_INTERVAL 5 * 1000 * 1000
        const DCHAR *pClientIp = NULL;

        D32S iClientPort = adm_cfg_iniParserGetInt(ADM_CFG_VUS_DIS_TCP_HOST_PORT, ADM_CFG_VUS_DIS_TCP_HOST_PORT_DEFAULT_VAL);
        if (iClientPort != ADM_CFG_VUS_DIS_TCP_HOST_PORT_DEFAULT_VAL)
        {
            pClientIp = adm_cfg_iniParserGetString(ADM_CFG_VUS_DIS_TCP_HOST_IP, ADM_CFG_VUS_DIS_TCP_HOST_IP_DEFAULT_VAL);
        }

        /** 非阻塞connect, 失败则connect ADM_ARPC_CONNECT_COUNT次 */
        for (D32U i = 0; i < ADM_ARPC_CONNECT_COUNT; i++)
        {
            /** 客户端绑定固定端口 */
            if (iClientPort != ADM_CFG_VUS_DIS_TCP_HOST_PORT_DEFAULT_VAL)
            {
                connfd = anetTcpBindNonBlockConnect(err, ip, (D16U)port, (DCHAR *)pClientIp, (D16U)iClientPort);
            }
            else 
            {
                connfd = anetTcpNonBlockConnect(err, ip, (D16U)port);
            }

            if (connfd == ANET_ERR)
            {
                log_e("connect error: %s", err);
                usleep(ADM_ARPC_CONNECT_INTERVAL);
            }
            else 
            {
                break;
            }
        }
#endif

        /** 判断客户端是否固定端口 */
        D32S iClientPort = adm_cfg_iniParserGetInt(ADM_CFG_VUS_DIS_TCP_HOST_PORT, ADM_CFG_VUS_DIS_TCP_HOST_PORT_DEFAULT_VAL);
        if (iClientPort != ADM_CFG_VUS_DIS_TCP_HOST_PORT_DEFAULT_VAL)
        {
            const DCHAR *pClientIp = adm_cfg_iniParserGetString(ADM_CFG_VUS_DIS_TCP_HOST_IP, ADM_CFG_VUS_DIS_TCP_HOST_IP_DEFAULT_VAL);
            connfd = anetTcpBindConnect(err, ip, (D16U)port, (DCHAR*)pClientIp, (D16U)iClientPort);
        }
        else 
        {
            connfd = anetTcpConnect(err, ip, (D16U)port);
        }

        if (connfd == ANET_ERR) 
		{
		    log_e("connect error: %s", err);
		    break;
		}
		else
		{
			(void)anetNonBlock(host->err_info, connfd);
		}

		peer = adm_arpc_allocPeer(NULL);
	    if (NULL == peer) 
	    {
	        log_e("alloc peer error...close socket\n");
	        (void)close(connfd);
	        break;
	    }

	    peer->loop = host->loop;
	    peer->fd = connfd;
	    (void)adm_memcpy(peer->ip, ip, adm_strlen(ip), adm_strlen(ip));
	    peer->port = port;
	    peer->instanceId = instanceId; 
	    peer->hostMode = (D8S)HOSTMODE_SERVER;

		adm_arpc_addPeer(peer);
		log_i("adm_arpc_retainPeer");
		adm_arpc_retainPeer(peer);
	    if(ADM_ECODE_SUCCESSED != adm_arpc_addFileEvent(peer, AE_READABLE, adm_arpc_readEventHandler)) 
	    {
	        log_e("create socket readable event error, close it.");
	        
	        // adm_arpc_freePeer(peer);
			adm_arpc_removePeer(peer);
			log_i("adm_arpc_releasePeer");
			adm_arpc_releasePeer(peer);
			log_i("adm_arpc_destroyPeer");
			adm_arpc_destroyPeer(peer);
			peer = DNULL;
	        break;
	    }

	}while(FALSE);

	return peer;
	
}


static D32S adm_arpc_initEventLoop(void)
{
	D32S retCode = ADM_ECODE_FAILED;
	errno = 0;
	if(SIG_ERR == signal(SIGPIPE, SIG_IGN) && 0 != errno)
	{
		return retCode;
	}

	do
	{
		host->hostMode = (D8S)(adm_cfg_iniParserGetInt(ADM_CFG_VUS_DIS_TCP_MODE, ADM_CFG_VUS_DIS_TCP_MODE_DEFAULT_VAL));
		host->hostIp = adm_strdup( adm_cfg_iniParserGetString(ADM_CFG_VUS_DIS_TCP_HOST_IP, ADM_CFG_VUS_DIS_TCP_HOST_IP_DEFAULT_VAL));
		host->port = adm_cfg_iniParserGetInt(ADM_CFG_VUS_DIS_TCP_SERVER_PORT, ADM_CFG_VUS_DIS_TCP_SERVER_PORT_DEFAULT_VAL);
		
		host->loop = adm_arpc_createEventLoop(host->max_peer_count);
		if(NULL == host->loop)
		{
			log_i("create eventloop fail");
			break;
		}

		(void)pthread_mutex_init(&host->peerMutex, NULL);
		(void)pthread_mutex_init(&host->downPeerMutex, NULL);

		//!< server
		if(HOSTMODE_SERVER == (host->hostMode & HOSTMODE_SERVER) )
		{			
			host->listen_fd = anetTcpServer(host->err_info, host->port, host->hostIp, host->backlog);
		    if (host->listen_fd != ADM_ECODE_FAILED) 
		    {
		        (void)anetNonBlock(host->err_info, host->listen_fd);
		    }
		    else
		    {
		    	log_i("create listen fd fail");
		    	break;
		    }
#if defined(ARPC_SSL)
			host->use_ssl = adm_cfg_iniParserGetBoolean(ADM_CFG_VUS_DIS_SOC_WITH_SSL, (DBOOL)ADM_CFG_VUS_DIS_SOC_WITH_SSL_DEFAULT_VAL);
			if(host->use_ssl == TRUE)
			{
				if (0 == adm_arpc_sslCtxInit(host))
				{
					log_e("create server ssl context failed\n");
					break;
				}

				retCode = adm_arpc_createFileEvent(host->loop, host->listen_fd, AE_READABLE, adm_arpc_acceptSSLHandler, host);
			}
			else
			{
				retCode = adm_arpc_createFileEvent(host->loop, host->listen_fd, AE_READABLE, adm_arpc_acceptTcpHandler, host);
			}
#else
			retCode = adm_arpc_createFileEvent(host->loop, host->listen_fd, AE_READABLE, adm_arpc_acceptTcpHandler, host);
#endif
			if(retCode == ADM_ECODE_SUCCESSED)
			{
				char conn_info[64];
				(void)anetFormatSock(host->listen_fd, conn_info, sizeof(conn_info));
				log_i("listen on: %s", conn_info);
			}
			
			//hostInstanceId = adm_cfg_iniParserGetInt(ADM_CFG_VUS_DIS_SOC_HOST_INSTANCEID, ADM_CFG_VUS_DIS_SOC_HOST_INSTANCEID_DEFAULT_VAL);
	    }

		//!< client
	    if(HOSTMODE_CLIENT == (host->hostMode & HOSTMODE_CLIENT) )
		{
			//!< read config, connct muti server
			const DCHAR* serverIp = adm_cfg_iniParserGetString(ADM_CFG_VUS_DIS_TCP_SERVER_IP,ADM_CFG_VUS_DIS_TCP_SERVER_IP_DEFAULT_VAL);
			D32S   serverPort = adm_cfg_iniParserGetInt(ADM_CFG_VUS_DIS_TCP_SERVER_PORT, ADM_CFG_VUS_DIS_TCP_SERVER_PORT_DEFAULT_VAL);
            D16U serverInstanceId = adm_cfg_iniParserGetInt(ADM_CFG_VUS_UIM_INSTANCEID, ADM_CFG_VUS_UIM_INSTANCEID_DEFAULT_VAL);
            // D16U   serverInstanceId = 0;
			ADM_ARPC_PEER_T *peer = NULL;
#if defined(ARPC_SSL)
			host->use_ssl = adm_cfg_iniParserGetBoolean(ADM_CFG_VUS_DIS_SOC_WITH_SSL, (DBOOL)ADM_CFG_VUS_DIS_SOC_WITH_SSL_DEFAULT_VAL);
			if (host->use_ssl == TRUE)
			{
				if(0 == adm_arpc_sslCtxInit(host))
				{
					log_e("create server ssl context failed\n");
					break;
				}
				peer = adm_arpc_connectSSLServer(serverIp, (D16U)serverPort, host->hostIp, serverInstanceId);
				if (peer == NULL)
				{
					log_e("connect instance 0X%04X, %s:%d fail ", serverInstanceId, serverIp, serverPort);
	
					ADM_ARPC_PEER_T* retryPeer = adm_arpc_allocPeer(NULL);
					if(NULL == retryPeer)
					{
						log_e("alloc peer fail");
						break;
					}
					
					retryPeer->hostMode = HOSTMODE_SERVER;
					(void)adm_memcpy(retryPeer->ip, serverIp, adm_strlen(serverIp), adm_strlen(serverIp));
					retryPeer->port = (D16U)serverPort;
					retryPeer->instanceId = serverInstanceId;
					
					// list_add_tail(&retryPeer->entry, &host->downPeerHead);
					adm_arpc_addDownPeer(retryPeer);
				}
			}
			else
			{
				peer = adm_arpc_connectServer((DCHAR*)serverIp, (D16U)serverPort, host->hostIp, serverInstanceId); 
				if(NULL != peer)
				{
					log_i("connect instance 0X%04X, %s:%d ok ", serverInstanceId, serverIp, serverPort);

					/*
					if(ADM_ECODE_SUCCESSED != adm_arpc_addInstancePeerInfo(serverInstanceId, peer))
					{
						break;
					}
					*/
					// tell peer, my ip,port
					adm_arpc_register(peer); 
				}
				else
				{
					log_e("connect instance 0X%04X, %s:%d fail ", serverInstanceId, serverIp, serverPort);
	
					ADM_ARPC_PEER_T* retryPeer = adm_arpc_allocPeer(NULL);
					if(NULL == retryPeer)
					{
						log_e("alloc peer fail");
						break;
					}
					
					retryPeer->hostMode = HOSTMODE_SERVER;
					(void)adm_memcpy(retryPeer->ip, serverIp, adm_strlen(serverIp), adm_strlen(serverIp));
					retryPeer->port = (D16U)serverPort;
					retryPeer->instanceId = serverInstanceId;
					
					// list_add_tail(&retryPeer->entry, &host->downPeerHead);
					adm_arpc_addDownPeer(retryPeer);
				}
			}
#else
			peer = adm_arpc_connectServer((DCHAR*)serverIp, serverPort, serverInstanceId); 
			if(NULL != peer)
			{
				log_i("connect instance 0X%04X, %s:%d ok ", serverInstanceId, serverIp, serverPort);

				/*
				if(ADM_ECODE_SUCCESSED != adm_arpc_addInstancePeerInfo(serverInstanceId, peer))
				{
					break;
				}
				*/
				// tell peer, my ip,port
				adm_arpc_register(peer); 
			}
			else
			{
				log_e("connect instance 0X%04X, %s:%d fail ", serverInstanceId, serverIp, serverPort);

				ADM_ARPC_PEER_T* retryPeer = adm_arpc_allocPeer(NULL);
				if(NULL == retryPeer)
				{
					log_e("alloc peer fail");
					break;
				}
				
				retryPeer->hostMode = HOSTMODE_SERVER;
				(void)adm_memcpy(retryPeer->ip, serverIp, adm_strlen(serverIp), adm_strlen(serverIp));
				retryPeer->port = (D16U)serverPort;
				retryPeer->instanceId = serverInstanceId;
				
				// list_add_tail(&retryPeer->entry, &host->downPeerHead);
				adm_arpc_addDownPeer(retryPeer);
			}
#endif
		}

	    retCode = ADM_ECODE_SUCCESSED;
	}while(FALSE);

	if(retCode == ADM_ECODE_FAILED)
	{
		// adm_free(host->loop);
		adm_arpc_deleteEventLoop(host->loop);
		host->loop = DNULL;
	}

	return retCode;
}

// 根据心跳判断是否掉线
static void adm_arpc_heartTimer(ADM_ARPC_EVENT_LOOP_T *eventLoop, void *privData)
{
	log_i("---------------heart Timer--------------------");
	ADM_ARPC_PEER_T ** peers = DNULL;
	D32S peerCount = 0;
	ADM_ARPC_PEER_T *peer = NULL;

	(void)pthread_mutex_lock(&host->peerMutex);
	if (0 == host->peerCount)
	{
		(void)pthread_mutex_unlock(&host->peerMutex);
		return;
	}
	
	peers = (ADM_ARPC_PEER_T**)adm_malloc(sizeof(ADM_ARPC_PEER_T*) * host->peerCount);
	if (DNULL == peers)
	{
		(void)pthread_mutex_unlock(&host->peerMutex);
		return;
	}

	list_for_each_entry(peer,&host->peerHead, ADM_ARPC_PEER_T, entry)
	{
		log_i("adm_arpc_retainPeer");
		if (DTRUE == adm_arpc_retainPeer(peer))
		{
			peers[peerCount++] = peer;
		}
	}
	(void)pthread_mutex_unlock(&host->peerMutex);

	for(D32S i = 0; i < peerCount; i++)
	{
		///< 已关闭, 待清理资源
		// (void)pthread_mutex_lock(&host->peerMutex);
		// if (1 == peer->free)
		// {
		// 	(void)pthread_mutex_unlock(&host->peerMutex);
		// 	continue;
		// }
		// (void)pthread_mutex_unlock(&host->peerMutex);
		peer = peers[i];

		peer->heartCount++;

		log_i("peer mode:%d,  instaceId:%04x, ip:%s, port:%d, hearCount: %d", peer->hostMode ,peer->instanceId, peer->ip, peer->port, peer->heartCount);

		if(peer->heartCount <= (D16U)heartCount)
		{
			if((D8S)HOSTMODE_SERVER == peer->hostMode )
			{
				// 对端是服务端				
				adm_arpc_sendHeart(peer);
			}
		}
		else
		{
			// 是否需要判读在掉线重试列表中, 同时心跳到期, 网络断开; 不判断
			log_e("peer is down, peer info : %s:%u, hearCount: %d", peer->ip, peer->port, peer->heartCount);
			// (void)close(peer->fd);
			
			///< 客户端掉线不加入掉线列表
			ADM_ARPC_PEER_T* retryPeer = DNULL;
			if(HOSTMODE_SERVER == (peer->hostMode & HOSTMODE_SERVER) )
			{
				retryPeer = adm_arpc_allocPeer(peer);
			}

			/// <list_del(&peer->entry);
			// adm_arpc_freePeer(peer); // 释放资源.
			adm_arpc_removePeer(peer);
			log_i("adm_arpc_destroyPeer");
			adm_arpc_destroyPeer(peer);

			if (NULL != retryPeer)
			{
				// pthread_mutex_lock(&host->downPeerMutex);
				// list_add_tail(&retryPeer->entry, &host->downPeerHead);
				// pthread_mutex_unlock(&host->downPeerMutex);
				adm_arpc_addDownPeer(retryPeer);
			}
		}

		log_i("adm_arpc_releasePeer");
		adm_arpc_releasePeer(peer);
	}

	adm_free(peers);
}

// 掉线重连
static void adm_arpc_ReconnTimer(ADM_ARPC_EVENT_LOOP_T *eventLoop, void *privData)
{
	log_i("---------------reconn Timer --------------------");

	// 判断是否在段线重连list中, 在, 清除.
	D32S iFind = 0;
	ADM_ARPC_PEER_T* reConnPeer = NULL;
	ADM_ARPC_PEER_T* downPeer, *n = NULL;
	D32S reConnFlag = 0;	
	list_for_each_entry_safe(downPeer,n, &host->downPeerHead, ADM_ARPC_PEER_T, entry)
	{
		reConnFlag = 0;
		if((tryConnTime>0 && (D32S)(downPeer->tryConnectCount)<tryConnTime) || (tryConnTime == -1))
		{
			downPeer->tryConnectCount++;
			log_i("down peer mode:%d,  instaceId:0X%04x, ip:%s, port:%d, tryCount:%d", 
				downPeer->hostMode ,downPeer->instanceId, downPeer->ip, downPeer->port, downPeer->tryConnectCount);
			
			// try connect peer, try max count
			if((D8S)HOSTMODE_SERVER == downPeer->hostMode)
			{
#if defined(ARPC_SSL)
				if(host->use_ssl == TRUE)
				{
					reConnPeer = adm_arpc_reConnectServer(downPeer->ip, downPeer->port, downPeer->instanceId);
					//此处有可能第一次连接没有成功时，第二次重试连接也来了。这样就会重新连接两次！
					//需要异步->同步
					if ( NULL != reConnPeer && (reConnPeer->sslState == ADM_ARPC_SSL_STATE_IN_SUCCESS))
					{
						reConnFlag = 1;
					}
					else
					{
						log_e("connct instance 0X%02X, %s:%d fail ",downPeer->instanceId, downPeer->ip, downPeer->port);
					}
				}
				else
				{
					reConnPeer = adm_arpc_connectServer(downPeer->ip, downPeer->port, downPeer->instanceId);
					if(NULL != reConnPeer)
					{	
						//添加PeerInfo放到register和registerACK里面做
						//adm_arpc_addInstancePeerInfo(downPeer->instanceId, reConnPeer);

						// tell peer, my ip,port
						adm_arpc_register(reConnPeer); 
						reConnFlag = 1;
					}
					else
					{
						log_i("connct instance 0X%02X, %s:%d fail ",downPeer->instanceId, downPeer->ip, downPeer->port);
					}
				}
#else
				reConnPeer = adm_arpc_connectServer(downPeer->ip, downPeer->port, downPeer->instanceId);

				if(NULL != reConnPeer)
				{
					//添加PeerInfo放到register和registerACK里面做
				    //(void)adm_arpc_addInstancePeerInfo(downPeer->instanceId, reConnPeer);

					// tell peer, my ip,port
				    adm_arpc_register(reConnPeer); 

					reConnFlag = 1;
				}
				else
				{
					log_i("connct instaceId:0X%04x, ip:%s, port:%d fail ",downPeer->instanceId, downPeer->ip, downPeer->port );
				}
#endif
			}

			else if((D8S)HOSTMODE_CLIENT == downPeer->hostMode)
			{
				///< 客户端判断不了, 实例id 动态分配. 
				///< 掉线时, 订阅事件已加入待重试订阅缓存

				/**
				list_for_each_entry(reConnPeer, &host->peerHead, ADM_ARPC_PEER_T, entry)
				{
					//log_i("reConnPeer instaceId:0X%04x, port:%d", reConnPeer->instanceId, reConnPeer->port);
					if((D8S)HOSTMODE_CLIENT == reConnPeer->hostMode && reConnPeer->free == 0)
					{
						if(downPeer->instanceId == reConnPeer->instanceId)
						{
							reConnFlag = 1;
							//添加PeerInfo放到register和registerACK里面做
							//(void)adm_arpc_addInstancePeerInfo(downPeer->instanceId, reConnPeer);
						}
					}
				}
				*/
			}
			else
			{
				log_e("invaild hostmode");
				return;
			}

			if(1 == reConnFlag)
			{
				log_i("reconnect ok, peer instaceId:0X%04X", downPeer->instanceId);
				
				// re connet success
				// pthread_mutex_lock(&host->downPeerMutex);
				// list_del(&downPeer->entry);
				// pthread_mutex_unlock(&host->downPeerMutex);
				adm_arpc_removeDownPeer(downPeer);
			}
		}
		else
		{
			// clear res, trans, rpc.
			log_i("del down peer,  mode:%d,  instaceId:0X%04X, ip:%s, port:%d", downPeer->hostMode ,downPeer->instanceId, downPeer->ip, downPeer->port);
			// pthread_mutex_lock(&host->downPeerMutex);
			// list_del(&downPeer->entry);
			// pthread_mutex_unlock(&host->downPeerMutex);
			adm_arpc_removeDownPeer(downPeer);
			log_i("adm_arpc_destroyPeer");
			adm_arpc_destroyPeer(downPeer);
		}		
	}

	// 有待重新订阅的事件, 重新订阅. 1 对端未上线时请求, 重试请求. 2 对端下线, 重连后, 重新请求
	if(1 == reConnFlag)
	{
		adm_arpc_reSubEvent();
	}

    return;
}

static void adm_arpc_clearResTimer(ADM_ARPC_EVENT_LOOP_T *eventLoop, void *privData)
{
	adm_arpc_cleanTimeOutSendDataSession();
	
	// 重连状态 -> 掉线状态
	
	return ;
}

static void adm_arpc_cleanDownPeer(ADM_ARPC_EVENT_LOOP_T *eventLoop, void *privData)
{
	ADM_ARPC_PEER_T *peer, *n = NULL;
	list_for_each_entry_safe(peer, n,&host->peerHead, ADM_ARPC_PEER_T, entry)
	{
		if(1 == peer->free )
		{
			struct timespec tv;
			(void)clock_gettime(CLOCK_MONOTONIC, &tv);
			if( tv.tv_sec - 10 > peer->closeTv.tv_sec )
			{
				
				log_i("free peer,instaceId:0X%04X, old fd:%d", peer->instanceId, peer->fd); 
		
				list_del(&peer->entry);
				adm_free(peer); // 释放资源.
			}
		}
	}

    return;
}

static void adm_arpc_PressuresTimer(ADM_ARPC_EVENT_LOOP_T *eventLoop, void *privData)
{
	log_i("---------------pressure print--------------------");

	ADM_ARPC_PEER_T** peers = NULL;
	ADM_ARPC_PEER_T* peer = NULL;
	D32S 		peerCount = 0;
	
	(void)pthread_mutex_lock(&host->peerMutex);
	if (0 == host->peerCount)
	{
		(void)pthread_mutex_unlock(&host->peerMutex);
		return;
	}
	
	peers = (ADM_ARPC_PEER_T**)adm_malloc(sizeof(ADM_ARPC_PEER_T*) * host->peerCount);
	if (DNULL == peers)
	{
		(void)pthread_mutex_unlock(&host->peerMutex);
		return;
	}

	list_for_each_entry(peer,&host->peerHead, ADM_ARPC_PEER_T, entry)
	{
		log_i("adm_arpc_retainPeer");
		if (DTRUE == adm_arpc_retainPeer(peer))
		{
			peers[peerCount++] = peer;
		}
	}
	(void)pthread_mutex_unlock(&host->peerMutex);

	for(D32S i = 0; i < peerCount; i++)
	{
		peer = peers[i];

		log_i("read buffer: %d, %d, %d", peer->read_buffer->readIndex, peer->read_buffer->writeIndex,peer->read_buffer->size);
		log_i("write buffer: %d, %d, %d", peer->write_buffer->readIndex, peer->write_buffer->writeIndex,peer->write_buffer->size);
		log_i("adm_arpc_releasePeer");
		adm_arpc_releasePeer(peer);
	}

	adm_arpc_print_buffPressures();
	
	return ;
}

static void adm_arpc_initCfg(void)
{
	tryConnTime = adm_cfg_iniParserGetInt(ADM_CFG_VUS_DIS_SOC_TRY_CONNECT_TIME, ADM_CFG_VUS_DIS_SOC_TRY_CONNECT_TIME_DEFAULT_VAL);		
	tryConnInterval = adm_cfg_iniParserGetInt(ADM_CFG_VUS_DIS_SOC_TRY_CONNECT_INTERVAL, ADM_CFG_VUS_DIS_SOC_TRY_CONNECT_INTERVAL_DEFAULT_VAL);
	heartCount = adm_cfg_iniParserGetInt(ADM_CFG_VUS_DIS_SOC_HEART_COUNT, ADM_CFG_VUS_DIS_SOC_HEART_COUNT_DEFAULT_VAL);	
	heartInterval = adm_cfg_iniParserGetInt(ADM_CFG_VUS_DIS_SOC_HEART_INTERVAL, ADM_CFG_VUS_DIS_SOC_HEART_INTERVAL_DEFAULT_VAL);	

    return;
}

static void adm_arpc_initTimer(void)
{
	// heart
	(void)adm_arpc_createTimeEvent(host->loop, (D64S)heartInterval*1000, adm_arpc_heartTimer, NULL);

	// reconn
	(void)adm_arpc_createTimeEvent(host->loop, (D64S)tryConnInterval*1000, adm_arpc_ReconnTimer, NULL);

	// clear timeout resource
	(void)adm_arpc_createTimeEvent(host->loop, 60000, adm_arpc_clearResTimer, NULL);

	// clear close peer res
	// (void)adm_arpc_createTimeEvent(host->loop, 10000, adm_arpc_cleanDownPeer, NULL);

	// print buff pressure
	(void)adm_arpc_createTimeEvent(host->loop, 60000, adm_arpc_PressuresTimer, NULL);

    return;
}

static void* adm_arpc_processRecvMessage(void *argv)
{
	// 获取队列消息

	D8S* data = NULL;
	D32S dataSize = 0;

	while(1)
	{	
		(void)pthread_mutex_lock(&host->hostStatusMutex);
		if(host->stop == 1)
		{
			(void)pthread_mutex_unlock(&host->hostStatusMutex);
			return NULL;
		}
		(void)pthread_mutex_unlock(&host->hostStatusMutex);
	
		if( 0 == adm_arpc_recvQueueSize() )
		{
			(void)pthread_mutex_lock(&host->recvQueueMutex);
            host->bRecvQueueSignal = DFALSE;
			
			struct timespec tv;
            (void)clock_gettime(CLOCK_MONOTONIC, &tv);
			
			tv.tv_sec += ((tv.tv_nsec + 100 * 1000 * 1000) / 1000000000);
			tv.tv_nsec = (tv.tv_nsec + 100 * 1000 * 1000) % 1000000000 ;
            (void)adm_arpc_pthread_cond_timedwait(&host->recvQueueCond, &host->recvQueueMutex, 
                                                    &host->bRecvQueueSignal, &tv); // 100 ms

            (void)pthread_mutex_unlock(&host->recvQueueMutex);
		}
		else
		{
			

			///< 给任务
			ADM_ARPC_QUEUE_DATA_T* queueData = NULL;
			if(1 == adm_arpc_popRecvQueue(&queueData))
			{
				///< 取线程
				ADM_ARPC_WORKER_THREAD_T* worker = adm_arpc_getWorkerThread(host->workPool);
				if(NULL == worker)
				{
					log_e("get workerthread fail");
					break;
				}
				
				adm_arpc_assignWorkerTask(worker, queueData);

				///< 10ms, 规避消息队列重复吐数据问题
				(void)usleep(10 * 1000); 
			}

			
			/*
			//log_i(" get recv data ");
			ADM_ARPC_QUEUE_DATA_T* queueData = NULL;
			if(1 == adm_arpc_popRecvQueue(&queueData))
			{
				// 线程池处理
				ADM_ARPC_WORKER_THREAD_T* worker = adm_arpc_getWorkerThread(host->workPool);
				if(NULL == worker)
				{
					log_e("get workerthread fail");
					break;
				}

				adm_arpc_assignWorkerTask(worker, queueData);

			}
			*/
		}

	}
	return NULL;
}

static void adm_arpc_sendQueueData(ADM_ARPC_QUEUE_DATA_T* queueData)
{
	D32S iRet = ADM_ECODE_FAILED;
	if(NULL== queueData){
		log_e("Invalid param.");
		return;
	}

	ADM_ARPC_PACKAGE_T* resPackage = queueData->package;

    if (NULL == queueData->peer || NULL == resPackage)
    {
        log_w("when send message, peer is NULL");
        return;
    }

	log_i("trans send: messagetype:%d, instanceId:0X%04x, peerInstanceId:0X%04x, serviceId:0X%04x, messageId:0X%04x, sessionId:%d, dataSize:%d", 
				resPackage->packageHead.messageType, resPackage->packageHead.instanceId,resPackage->packageHead.peerInstanceId,
				resPackage->packageHead.serviceId, resPackage->packageHead.messageId, resPackage->packageHead.sessionId,
				(D32S)(resPackage->packageHead.length) - ADM_ARPC_PROTOCOL_HEAD_IN_LEN);

    // Check peer status with mutex protection
	(void)pthread_mutex_lock(&queueData->peer->mutex);
	if (1 == queueData->peer->free)
	{
		log_w("when send message, peer closed, instanceId:0X%04X, fd:%d", 
			queueData->peer->instanceId, queueData->peer->fd);
		(void)pthread_mutex_unlock(&queueData->peer->mutex);
		return;
	}

	// Check write_buffer validity
	if (NULL == queueData->peer->write_buffer)
	{
		log_w("when send message, peer write_buffer is NULL, instanceId:0X%04X, fd:%d", 
			queueData->peer->instanceId, queueData->peer->fd);
		(void)pthread_mutex_unlock(&queueData->peer->mutex);
		return;
	}

	// Check fd validity
	if (queueData->peer->fd <= 0)
	{
		log_w("when send message, peer fd is invalid, instanceId:0X%04X, fd:%d", 
			queueData->peer->instanceId, queueData->peer->fd);
		(void)pthread_mutex_unlock(&queueData->peer->mutex);
		return;
	}

	/* peer->write_buffer相关数据的操作在同一线程中处理的，不需要加锁也能保证数据的正确性 */
	// Unlock before encoding to avoid holding lock during encoding
	// (void)pthread_mutex_unlock(&queueData->peer->mutex);


    /* peer->write_buffer相关数据的操作在同一线程中处理的，不需要加锁也能保证数据的正确性 */
	if(0 != adm_arpc_encodePackage(queueData->peer->write_buffer, resPackage))
	{
		// (void)pthread_mutex_unlock(&host->peerMutex);
        (void)pthread_mutex_unlock(&queueData->peer->mutex);
        return;
	}

    // 在锁内保存需要的数据，避免在锁外访问
    ADM_ARPC_BUFFER_T *write_buffer = queueData->peer->write_buffer;
    int saved_fd = queueData->peer->fd;
    D32U saved_readIndex = write_buffer->readIndex;
    D32U readable_size = adm_arpc_readableBufferSize(write_buffer);
    char *write_data = (char *)write_buffer->data + saved_readIndex;

#if defined(ARPC_SSL)
    // 在锁内保存 ssl 指针，避免在锁外访问 peer->ssl
    ADM_COM_SSL_T *saved_ssl = NULL;
    if (host->use_ssl == TRUE)
    {
        if (NULL == queueData->peer->ssl)
        {
            (void)pthread_mutex_unlock(&queueData->peer->mutex);
            log_w("peer ssl is NULL, instanceId:0X%04X", queueData->peer->instanceId);
            return;
        }
        saved_ssl = queueData->peer->ssl;
    }
#endif

    (void)pthread_mutex_unlock(&queueData->peer->mutex);

#if defined(ARPC_SSL)
	int writen = 0;
	if(host->use_ssl == TRUE)
	{
		// 使用保存的 ssl 指针，避免访问可能被销毁的 peer
		// 注意：如果 peer 在写入期间被销毁，ssl 也会被释放，但 adm_com_sslWrite 通常很快返回
		writen = adm_arpc_sslWriteWithSSL(saved_ssl, write_data, (int)readable_size);	
	}
	else
	{
		writen = anetWrite(saved_fd, write_data, (int)readable_size); 
	}
#else
	int writen = anetWrite(saved_fd, write_data, (int)readable_size);
#endif
	if (writen > 0) 
	{
        (void)pthread_mutex_lock(&queueData->peer->mutex);
        if (1 == queueData->peer->free || NULL == queueData->peer->write_buffer || queueData->peer->fd <= 0)
        {
            log_w("When update readIndex, peer close.");
        }
        else 
        {
		    queueData->peer->write_buffer->readIndex += (D32U)writen;
        }
        (void)pthread_mutex_unlock(&queueData->peer->mutex);
	}
	// (void)pthread_mutex_unlock(&host->peerMutex);

	if (writen == -1) 
	{		   		
		log_i("peer close");
		// adm_arpc_removePeer(peer);
		// log_i("adm_arpc_destroyPeer");
		// adm_arpc_destroyPeer(queueData->peer);
		return;
	}

    // Check peer status again before adding write event
	(void)pthread_mutex_lock(&queueData->peer->mutex);
	if (1 == queueData->peer->free || NULL == queueData->peer->write_buffer || queueData->peer->write_buffer != write_buffer)
	{
		(void)pthread_mutex_unlock(&queueData->peer->mutex);
		return;
	}
	
	if (adm_arpc_readableBufferSize(queueData->peer->write_buffer) != 0U) 
	{
        (void)pthread_mutex_unlock(&queueData->peer->mutex);
#if defined(ARPC_SSL)
		if(host->use_ssl == TRUE)
		{
			iRet = adm_arpc_addFileEvent(queueData->peer, AE_WRITABLE, adm_arpc_writeSSLEventHandler);
		}
		else
		{
			iRet = adm_arpc_addFileEvent(queueData->peer, AE_WRITABLE, adm_arpc_writeEventHandler);
		}
#else
		iRet = adm_arpc_addFileEvent(queueData->peer, AE_WRITABLE, adm_arpc_writeEventHandler);
#endif
		if (iRet == AE_ERR)
		{
			log_i("create socket writeable event error, close it.");
			// adm_arpc_freePeer(queueData->peer);
			// adm_arpc_releasePeer(queueData->peer);
		}
	}
    else
	{
		(void)pthread_mutex_unlock(&queueData->peer->mutex);
	}

    return;
}

static void adm_arpc_sendMessage(void)
{
	while(adm_arpc_sendQueueSize() > 0)
	{
		(void)pthread_mutex_lock(&host->hostStatusMutex);
		if(host->stop == 1)
		{
			(void)pthread_mutex_unlock(&host->hostStatusMutex);
			return;
		}
		(void)pthread_mutex_unlock(&host->hostStatusMutex);
		
		ADM_ARPC_QUEUE_DATA_T* queueData = NULL;
		if(1 == adm_arpc_popSendQueue(&queueData))
		{
			adm_arpc_sendQueueData(queueData);
			// log_i("adm_arpc_releasePeer");
			adm_arpc_releasePeer(queueData->peer);
			adm_free(queueData->package);
			adm_free(queueData);
		}
	}

    return;
}

ADM_ARPC_HOST_T* amd_arpc_getHostInfo(void)
{
	return host;
}

#define PING_MAX_TRY_COUNT 3
#define PING_INTERVAL 1 * 1000 * 1000 ///< 5s

/**
 * @brief 侦测主控网络状态
 *
 * @return DVOID
 */
static DVOID adm_arpc_detectNetwork()
{
    D8S iHostMode = HOSTMODE_CLIENT;
    iHostMode = (D8S)(adm_cfg_iniParserGetInt(ADM_CFG_VUS_DIS_TCP_MODE, ADM_CFG_VUS_DIS_TCP_MODE_DEFAULT_VAL));

    if (HOSTMODE_CLIENT == (iHostMode & HOSTMODE_CLIENT))
    {
        D32U uiTryCount = 0;
        const DCHAR *pServerIp = adm_cfg_iniParserGetString(ADM_CFG_VUS_DIS_DETECT_IP, ADM_CFG_VUS_DIS_DETECT_IP_DEFAULT_VAL);

        if (0 == adm_strcmp(pServerIp, ADM_CFG_VUS_DIS_DETECT_IP_DEFAULT_VAL))
        {
            return;
        }

        while (uiTryCount < PING_MAX_TRY_COUNT)
        {
            pid_t ping_pid = fork();
            if (-1 == ping_pid)
            {
                log_e("ping progress fork failed.");
            }
            else if (0 == ping_pid)
            {
                /** 子进程 */
                execlp("ping", "ping", "-c", "1", "-W", "5", pServerIp, NULL);
                log_e("exec ping failed.");
                _exit(EXIT_FAILURE);
            }
            else
            {
                /** 父进程 */
                D32S iStatus;
                waitpid(ping_pid, &iStatus, 0);

                if (WIFEXITED(iStatus))
                {
                    if (WEXITSTATUS(iStatus) == 0)
                    {
                        log_i("Ping [%s] succeed.", pServerIp);
                        return; ///< ping 成功退出监控
                    }
                    /** ping 失败继续循环 */
                    log_i("Ping [%s] failed.", pServerIp);
                }
                usleep(PING_INTERVAL);
            }

            uiTryCount++;
        }
    }

    return;
}

D32S adm_arpc_initserver(void)
{
	D32S retCode = ADM_ECODE_FAILED;
    D32S iMinThreadNum = 0;
    D32S iMaxThreadNum = 0;

	do
	{
		adm_arpc_initCfg();
		
		// init buffer
		adm_arpc_initBuff();
		
		host = adm_malloc(sizeof(ADM_ARPC_HOST_T));
		(void)adm_memset(host, 0x0, sizeof(ADM_ARPC_HOST_T), sizeof(ADM_ARPC_HOST_T));

		host ->stop = 0;
		host->backlog = DEFAULT_LISTEN_BACKLOG;
    	host->max_peer_count = DEFAULT_MAX_CLIENT_COUNT;

		host->peerCount = 0;
		host->downPeerCount = 0;
    	INIT_LIST_HEAD(&host->peerHead);
    	INIT_LIST_HEAD(&host->downPeerHead);

		(void)pthread_mutex_init(&host->recvQueueMutex, NULL);
	    (void)pthread_condattr_init(&host->recvQueueCattr);
	    (void)pthread_condattr_setclock(&host->recvQueueCattr, CLOCK_MONOTONIC);
	    (void)pthread_cond_init(&host->recvQueueCond, &host->recvQueueCattr);

        /** connect之前探测网络状态 */
        adm_arpc_detectNetwork();

        // init event loop
		if(ADM_ECODE_SUCCESSED != adm_arpc_initEventLoop())
		{
			break;
		}
		
		// init timer;
		adm_arpc_initTimer();	

        iMinThreadNum = adm_cfg_iniParserGetInt(ADM_CFG_VUS_DIS_MIN_THREAD_NUM, ADM_CFG_VUS_DIS_MIN_THREAD_NUM_DEFAULT_VAL);
        iMaxThreadNum = adm_cfg_iniParserGetInt(ADM_CFG_VUS_DIS_MAX_THREAD_NUM, ADM_CFG_VUS_DIS_MAX_THREAD_NUM_DEFAULT_VAL);
        if (iMinThreadNum < ADM_ARPC_MIN_PTHREAD_NUM)
        {
            log_w("Min thread set to default value [%d].", ADM_ARPC_MIN_PTHREAD_NUM);
            iMinThreadNum = ADM_ARPC_MIN_PTHREAD_NUM;
        }
        if (iMaxThreadNum > ADM_ARPC_MAX_PTHREAD_NUM)
        {
            log_w("Max thread set to default value [%d].", ADM_ARPC_MAX_PTHREAD_NUM);
            iMaxThreadNum = ADM_ARPC_MAX_PTHREAD_NUM;
        }

		ADM_ARPC_THREAD_POOL_T* pool = adm_arpc_initThreadPool(iMinThreadNum, iMaxThreadNum, adm_arpc_ProcessPackage);
		if(NULL == pool)
		{
			break;
		}

		host->workPool = pool;
		
		retCode = ADM_ECODE_SUCCESSED;

	}while(FALSE);

	return retCode;
}

static void* adm_arpc_eventLoopThread(void *argv)
{
	while (1) 
	{    
		(void)pthread_mutex_lock(&host->hostStatusMutex);
		if(host->stop == 1)
		{
			(void)pthread_mutex_unlock(&host->hostStatusMutex);
			return NULL;
		}
		(void)pthread_mutex_unlock(&host->hostStatusMutex);
	
        adm_arpc_eventLoop(host->loop);		

		adm_arpc_sendMessage();
    }
	return NULL;
}

static void* adm_arpc_timeEventLoopThread(void *argv)
{
	while (1) 
	{
		(void)pthread_mutex_lock(&host->hostStatusMutex);
		if(host->stop == 1)
		{
			(void)pthread_mutex_unlock(&host->hostStatusMutex);
			return NULL; 
		}
		(void)pthread_mutex_unlock(&host->hostStatusMutex);
		
		adm_arpc_processTimeEvent(host->loop);

		(void)usleep(50 * 1000); // 50 ms
	}
	return NULL;
}

D32S adm_arpc_startServer(void)
{

	// init network

	pthread_t recvTid;	
	(void)pthread_attr_init(&host->procMessageAttr); 
	(void)pthread_attr_setdetachstate(&host->procMessageAttr, PTHREAD_CREATE_DETACHED);	

	if (0 != pthread_create(&recvTid, &host->procMessageAttr, adm_arpc_processRecvMessage, NULL))
	{
		log_e("pthread_create tid:%d failed", recvTid);
		return ADM_ECODE_PTHREAD_CREATE_FAILED;
	}

	pthread_t eventloopTid;
	(void)pthread_attr_init(&host->eventloopAttr); 
	(void)pthread_attr_setdetachstate(&host->eventloopAttr, PTHREAD_CREATE_DETACHED);   

	if (0 != pthread_create(&eventloopTid, &host->eventloopAttr, adm_arpc_eventLoopThread, NULL))
    {
        log_e("pthread_create tid:%d failed", eventloopTid);
        return ADM_ECODE_PTHREAD_CREATE_FAILED;
    }

	// 心跳涉及发送消息, 有加锁过程, 不能跟发送线程 同一线程
    pthread_t eventloopTimeTid;
	(void)pthread_attr_init(&host->eventloopTimeAttr); 
	(void)pthread_attr_setdetachstate(&host->eventloopTimeAttr, PTHREAD_CREATE_DETACHED);   

	if (0 != pthread_create(&eventloopTimeTid, &host->eventloopTimeAttr, adm_arpc_timeEventLoopThread, NULL))
    {
        log_e("pthread_create tid:%d failed", eventloopTimeTid);
        return ADM_ECODE_PTHREAD_CREATE_FAILED;
    }
	
    return ADM_ECODE_SUCCESSED;
}

void adm_arpc_stopServer(void)
{
	log_i("adm_arpc_stop Server begin");

	(void)pthread_mutex_lock(&host->hostStatusMutex);
	host->stop = 1;
	(void)pthread_mutex_unlock(&host->hostStatusMutex);

	adm_arpc_deleteEventLoop(host->loop);

	adm_arpc_deInitBuff();
	
	//adm_arpc_clearTimeEvent(host->loop);

	(void)pthread_attr_destroy(&host->procMessageAttr);
	(void)pthread_attr_destroy(&host->eventloopAttr);
	
    (void)pthread_condattr_destroy(&host->recvQueueCattr);
    (void)pthread_cond_destroy(&host->recvQueueCond);
    (void)pthread_mutex_destroy(&host->recvQueueMutex);

	(void)pthread_mutex_destroy(&host->peerMutex);
	(void)pthread_mutex_destroy(&host->downPeerMutex);
	
	adm_arpc_destroyThreadPool(host->workPool);
#if defined(ARPC_SSL)
	//清理SSL资源
	if(host->use_ssl == TRUE)
	{
		adm_arpc_sslCtxFree(host);
	}
#endif
	log_i("adm_arpc_stop Server end");
}

void adm_arpc_addPeer(ADM_ARPC_PEER_T* peer)
{
	pthread_mutex_lock(&host->peerMutex);
	pthread_mutex_lock(&peer->mutex);

	list_add_tail(&peer->entry, &host->peerHead);
	host->peerCount++;

	pthread_mutex_unlock(&peer->mutex);
	pthread_mutex_unlock(&host->peerMutex);

}

void adm_arpc_removePeer(ADM_ARPC_PEER_T* peer)
{
	pthread_mutex_lock(&host->peerMutex);
	pthread_mutex_lock(&peer->mutex);
	if (NULL != peer->entry.next && NULL != peer->entry.prev)
	{
		
		list_del(&peer->entry);
		host->peerCount--;
	}
	pthread_mutex_unlock(&peer->mutex);
	pthread_mutex_unlock(&host->peerMutex);
}

void adm_arpc_addDownPeer(ADM_ARPC_PEER_T* peer)
{
	host->downPeerCount++;
	list_add_tail(&peer->entry, &host->downPeerHead);
}

void adm_arpc_removeDownPeer(ADM_ARPC_PEER_T* peer)
{
	if (NULL != peer->entry.next && NULL != peer->entry.prev)
	{
		host->downPeerCount--;
		list_del(&peer->entry);
	}
}

DBOOL adm_arpc_retainPeer(ADM_ARPC_PEER_T* peer)
{
	DBOOL isOk = DFALSE;
	pthread_mutex_lock(&peer->mutex);
	if (0 == peer->free)
	{
		peer->refCount++;
		isOk = DTRUE;
		// log_i("retain peer[%d] ref[%d]", peer->fd, peer->refCount);
	}
	pthread_mutex_unlock(&peer->mutex);

	return isOk;
}

void adm_arpc_releasePeer(ADM_ARPC_PEER_T* peer)
{
	pthread_mutex_lock(&peer->mutex);
	peer->refCount--;
    if (0 == peer->refCount)
    {
	    log_i("release peer[%d] ref[%d]", peer->fd, peer->refCount);
    }
	if (peer->refCount <= 0)
	{
		pthread_mutex_unlock(&peer->mutex);
		adm_arpc_freePeer(peer);
	}
	else
	{
		pthread_mutex_unlock(&peer->mutex);
	}

    return;
}

void adm_arpc_destroyPeer(ADM_ARPC_PEER_T* peer)
{
	pthread_mutex_lock(&peer->mutex);
	log_i("destroy peer[%d] ref[%d] free[%d]", peer->fd, peer->refCount, peer->free);
	if (1 == peer->free)
	{
		pthread_mutex_unlock(&peer->mutex);
		return;
	}
	
	peer->free = 1;

	D32S mask = adm_arpc_deleteAllFileEvent(peer->loop, peer->fd);
	(void)close(peer->fd);
	if (mask > 0)
	{
		log_i("adm_arpc_releasePeer");
		// adm_arpc_releasePeer(peer);
		peer->refCount--;
	}
	pthread_mutex_unlock(&peer->mutex);
	
	///< call offline callback
	adm_arpc_callInstanceBehaviorCb(peer, INSTANCE_BEHAVIOR_OFFLINE);
	
    // soa app, clear res
	///< remove client subed event, reconnect, sub again 
	adm_arpc_delSubEvent(peer);

	///< 待重新订购事件, 历史订购事件加入待订购事件.			
	adm_arpc_addWaitSubEventFromSubedEvent(peer);

	///< 清除在线回调
	adm_arpc_clearInstanceCb(peer, INSTANCE_BEHAVIOR_ONLINE);

	///< remove instanceId, socket info bind relation
    adm_arpc_delInstancePeerInfo(peer);

	pthread_mutex_lock(&peer->mutex);
	peer->refCount--;
	log_i("destroy peer[%d] ref[%d]", peer->fd, peer->refCount);
	if (peer->refCount <= 0)
	{
		pthread_mutex_unlock(&peer->mutex);
		adm_arpc_freePeer(peer);
	}
	else
	{
		pthread_mutex_unlock(&peer->mutex);
	}
}
