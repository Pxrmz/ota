#if defined(ARPC_SSL)
#include <unistd.h>
#include "adm_arpc_ssl.h"
#include "adm_com_sslAdapter.h"
#include "adm_arpc_define.h"
#include "adm_arpc_eventLoop.h"
#include "adm_arpc_buffer.h"
#include "adm_arpc_server.h"
#include "adm_cfg_api.h"
#include "adm_com_errorCode.h"
#include "adm_arpc_log.h"
#include "adm_memory.h"
#include "adm_arpc_select.h"
#include "adm_string.h"
#include "adm_arpc_threadCond.h"

static DCHAR *g_arpcpassword = NULL;

void adm_arpc_setKeyPasswd(DCHAR *password)
{
	g_arpcpassword = password;
}

//PKCS-8的私钥密码需要及时在内存中清除
static void adm_arpc_cleanKeyPasswd(void)
{
	g_arpcpassword = NULL;
}

D32S adm_arpc_sslCtxInit(ADM_ARPC_HOST_T *host)
{
	if(g_arpcpassword != NULL)
	{
		host->ssl_ctx_opt.password = g_arpcpassword;
	}
	if ((D8S)HOSTMODE_SERVER == host->hostMode)
	{
		host->ssl_ctx_opt.ca_file = adm_cfg_iniParserGetString(ADM_CFG_VUS_DIS_SOC_SSL_CAFILE, ADM_CFG_VUS_DIS_SOC_SSL_CAFILE_DEFAULT_VAL);
		host->ssl_ctx_opt.key_file = adm_cfg_iniParserGetString(ADM_CFG_VUS_DIS_SOC_SSL_KEYFILE, ADM_CFG_VUS_DIS_SOC_SSL_KEYFILE_DEFAULT_VAL);
		host->ssl_ctx_opt.crt_file = adm_cfg_iniParserGetString(ADM_CFG_VUS_DIS_SOC_SSL_CERTFILE, ADM_CFG_VUS_DIS_SOC_SSL_CERTFILE_DEFAULT_VAL);
		host->ssl_ctx_opt.ciphers = adm_cfg_iniParserGetString(ADM_CFG_VUS_DIS_SOC_SSL_CIPHERS, ADM_CFG_VUS_DIS_SOC_SSL_CIPHERS_DEFAULT_VAL);
		host->ssl_ctx_opt.version = adm_cfg_iniParserGetString(ADM_CFG_VUS_DIS_SOC_SSL_PROTOCOLS, ADM_CFG_VUS_DIS_SOC_SSL_PROTOCOLS_DEFAULT_VAL);
		host->ssl_ctx_opt.verify_peer = adm_cfg_iniParserGetBoolean(ADM_CFG_VUS_DIS_SOC_SSL_VERIFY, (DBOOL)ADM_CFG_VUS_DIS_SOC_SSL_VERIFY_DEFAULT_VAL);
		host->ssl_ctx_opt.endpoint = 1;
		host->ssl_ctx = adm_com_sslCtxNew(&host->ssl_ctx_opt);
		if (host->ssl_ctx == NULL)
		{
			log_e("create server ssl context failed");
			return 0;
		}
	}
	else if ((D8S)HOSTMODE_CLIENT == host->hostMode)
	{
		host->ssl_ctx_opt.ca_file = adm_cfg_iniParserGetString(ADM_CFG_VUS_DIS_SOC_SSL_CLIENT_CAFILE, ADM_CFG_VUS_DIS_SOC_SSL_CLIENT_CAFILE_DEFAULT_VAL);
		host->ssl_ctx_opt.ciphers = adm_cfg_iniParserGetString(ADM_CFG_VUS_DIS_SOC_SSL_CIPHERS, ADM_CFG_VUS_DIS_SOC_SSL_CIPHERS_DEFAULT_VAL);
		host->ssl_ctx_opt.version = adm_cfg_iniParserGetString(ADM_CFG_VUS_DIS_SOC_SSL_PROTOCOLS, ADM_CFG_VUS_DIS_SOC_SSL_PROTOCOLS_DEFAULT_VAL);
		host->ssl_ctx_opt.verify_peer = adm_cfg_iniParserGetBoolean(ADM_CFG_VUS_DIS_SOC_SSL_VERIFY, (DBOOL)ADM_CFG_VUS_DIS_SOC_SSL_VERIFY_DEFAULT_VAL);
		host->ssl_ctx_opt.endpoint = 0;
		//客户端的双向认证需要配置客户端的证书和私钥，非双向模式则不需要
		if(TRUE == host->ssl_ctx_opt.verify_peer)
		{
			host->ssl_ctx_opt.key_file = adm_cfg_iniParserGetString(ADM_CFG_VUS_DIS_SOC_SSL_CLIENT_KEYFILE, ADM_CFG_VUS_DIS_SOC_SSL_CLIENT_KEYFILE_DEFAULT_VAL);
			host->ssl_ctx_opt.crt_file = adm_cfg_iniParserGetString(ADM_CFG_VUS_DIS_SOC_SSL_CLIENT_CERTFILE, ADM_CFG_VUS_DIS_SOC_SSL_CLIENT_CERTFILE_DEFAULT_VAL);
		}
		host->ssl_ctx = adm_com_sslCtxNew(&host->ssl_ctx_opt);
		if (host->ssl_ctx == NULL)
		{
			log_e("create client ssl context failed");
			return 0;
		}
	}
	else
	{
		log_e("invaild hostmode");
		return 0;
	}
	if (g_arpcpassword != NULL)
	{
		adm_arpc_cleanKeyPasswd();
	}

	return 1;
}

void adm_arpc_sslFree(const ADM_ARPC_PEER_T *peer)
{
	if ((peer != NULL) && (peer->ssl != NULL))
	{
		adm_com_sslFree(peer->ssl);
	}
}

void adm_arpc_sslCtxFree(const ADM_ARPC_HOST_T *host)
{
	if ((host != NULL) && (host->ssl_ctx != NULL))
	{
		adm_com_sslCtxFree(host->ssl_ctx);
	}
}

D32S adm_arpc_sslBufferRead(ADM_ARPC_PEER_T *peer)
{
	DCHAR additional_buf[INIT_BUFF_SIZE] = {0};
	D32S result = 0, total_length = 0, vec_num = 2, vec_used = 0;

	// 在锁内获取 buffer 并检查有效性
	(void)pthread_mutex_lock(&peer->mutex);
	
	if (1 == peer->free || NULL == peer->read_buffer || NULL == peer->ssl)
	{
		(void)pthread_mutex_unlock(&peer->mutex);
		log_w("peer freed or read_buffer/ssl invalid in sslBufferRead");
		return ADM_SSL_ERROR;
	}
	
	ADM_ARPC_BUFFER_T *buffer = peer->read_buffer;
	ADM_COM_SSL_T *ssl = peer->ssl;  // 在锁内保存 ssl 指针

	D32U writeable_len = adm_arpc_writeableBufferSize(buffer);

	struct iovec vec[2];
	vec[0].iov_base = buffer->data + buffer->writeIndex;
	vec[0].iov_len = (size_t)writeable_len;
	vec[1].iov_base = additional_buf;
	vec[1].iov_len = INIT_BUFF_SIZE;

    // 解锁后再调用系统调用（避免在锁内阻塞）
	(void)pthread_mutex_unlock(&peer->mutex);

	for (D32S i = 0; i < vec_num; ++i)
	{
		if (vec[i].iov_len == (size_t)0)
		{
			continue;
		}
		//这个while循环用来处理ssl读取到record后需要继续读取来获取下一个record的情况
		while(TRUE)
		{
			if (vec[i].iov_len == (size_t)0)
			{
				break;
			}
			result = adm_com_sslRead(ssl, (DCHAR *)vec[i].iov_base, (D32S)vec[i].iov_len);
			if (result > 0)
			{
				++vec_used;
				total_length += result;
				//当读取的长度为单个record长度时(16384)，需要继续读取下一个record
				if (total_length == ADM_ARPC_SSL_RECORD_MAX_LEN)
				{
					vec[i].iov_base += result;
					vec[i].iov_len -= (size_t)result;
					continue;
				}
			}
			break;
		}
		if (result > 0)
		{
			// buffer没读满时，不需要读取下一个buffer
			if ((size_t)result < vec[i].iov_len)
			{
				break;
			}
		}
		else
		{
			//目前只是打印信息将错误码返回
			switch (result) {
			case ADM_SSL_WANT_READ:
				log_i("ssl read result is ADM_SSL_WANT_READ");
				break;
			case ADM_SSL_WANT_WRITE:
				log_i("ssl read result is ADM_SSL_WANT_WRITE");
				break;
			case ADM_SSL_ERROR:
				log_i("ssl read result is ADM_SSL_ERROR");
				break;
			default:
				log_i("ssl read result is unknow");
				break;
			}
			break;
		}
	}

    // 重新加锁更新 buffer
	(void)pthread_mutex_lock(&peer->mutex);
	
	// 检查 buffer 和 ssl 是否仍然有效
	if (1 == peer->free || NULL == peer->read_buffer || peer->read_buffer != buffer || peer->ssl != ssl)
	{
		(void)pthread_mutex_unlock(&peer->mutex);
		log_w("peer freed or read_buffer/ssl changed during SSL read, instanceId:0X%04X", 
			peer->instanceId);
		// 丢弃这次读取的数据
		return ADM_SSL_ERROR;
	}

	if (vec_used > 0)
	{
		if ((D32U)total_length <= writeable_len)
		{
			buffer->writeIndex += (D32U)total_length;
		}
		else
		{
			buffer->writeIndex = buffer->size;
			adm_arpc_bufferAppend(buffer, additional_buf, (D32U)total_length - writeable_len);
		}
		result = total_length;
	}

    (void)pthread_mutex_unlock(&peer->mutex);
	return result;
}

void adm_arpc_writeSSLEventHandler(ADM_ARPC_EVENT_LOOP_T *loop, D32S fd, void *data, D32S mask)
{
	ADM_ARPC_PEER_T *peer = (ADM_ARPC_PEER_T *)data;
	ADM_ARPC_BUFFER_T *wbuffer = NULL;
	(void)fd;
	(void)loop;
	(void)mask;

	/* peer->write_buffer相关数据的操作在同一线程中处理的，不需要加锁也能保证数据的正确性 */
	wbuffer = peer->write_buffer;
	do
	{
		D32S data_size = (D32S)adm_arpc_readableBufferSize(wbuffer);
		if (data_size == 0)
		{
			adm_arpc_DelEvent(peer->loop, peer->fd, AE_WRITABLE);
			// log_i("adm_arpc_releasePeer");
			// adm_arpc_releasePeer(peer);
			break;
		}
	
		D32S writen = adm_arpc_sslWrite(peer, (DCHAR *)wbuffer->data + wbuffer->readIndex, data_size);
		if (writen > 0)
		{
			wbuffer->readIndex += (D32U)writen;
		}
		if ((D32S)adm_arpc_readableBufferSize(wbuffer) == 0)
		{
			adm_arpc_DelEvent(peer->loop, peer->fd, AE_WRITABLE);
			// log_i("adm_arpc_releasePeer");
			// adm_arpc_releasePeer(peer);
		}
	} while (FALSE);

	return;
}

void adm_arpc_readSSLEventHandler(ADM_ARPC_EVENT_LOOP_T *loop, D32S fd, void *data, D32S mask)
{
	ADM_ARPC_PEER_T *peer = (ADM_ARPC_PEER_T *)data;
	ADM_ARPC_HOST_T *host = amd_arpc_getHostInfo();
	(void)fd;
	(void)loop;
	(void)mask;

	// Check peer status before reading
	(void)pthread_mutex_lock(&peer->mutex);
    if (1 == peer->free)
	{
		(void)pthread_mutex_unlock(&peer->mutex);
		log_w("peer already freed when reading SSL");
		return;
	}
    (void)pthread_mutex_unlock(&peer->mutex);

	D32S iret = adm_arpc_sslBufferRead(peer);
	if (iret > 0)
	{
		ADM_ARPC_PACKAGE_T *reqPackage = NULL;
		while (TRUE)
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
			ADM_ARPC_BUFFER_T* rbuffer = peer->read_buffer;
			// 在锁内调用 decodePackage，保护 buffer 的访问和修改
			// 注意：adm_malloc 可能阻塞，但保护 buffer 不被释放更重要
			if (0 != adm_arpc_decodePackage(rbuffer, &reqPackage))
			{
                (void)pthread_mutex_unlock(&peer->mutex);
				break;
			}
            (void)pthread_mutex_unlock(&peer->mutex);

			(void)pthread_mutex_lock(&host->recvQueueMutex);
			// send in queue.
			void *priv = NULL;
			if (ADM_ECODE_SUCCESSED != adm_arpc_add2RecvQueue(reqPackage, peer, priv))
			{
				log_e("push data to queue error");
			}

			(void)pthread_cond_signal(&host->recvQueueCond);
			(void)pthread_mutex_unlock(&host->recvQueueMutex);
		}
	}
	else if (iret == ADM_SSL_ERROR)
	{
		// 在锁内保存 peer 信息，避免在锁外访问
		(void)pthread_mutex_lock(&peer->mutex);
		D16U saved_instanceId = peer->instanceId;
		D16U saved_port = peer->port;
		char saved_ip[20] = {0};
		(void)adm_memcpy(saved_ip, peer->ip, 20, 20);
		(void)pthread_mutex_unlock(&peer->mutex);
		
		log_i("peer close,instaceId:0X%04X,  %s:%u, fd:%d", saved_instanceId, saved_ip, saved_port, fd);

		if(0 == (D32S)(saved_port))
		{
			log_i("peer port is 0");
			return;
		}

		// copy to down peer list, try re connect
		ADM_ARPC_PEER_T *retryPeer = adm_arpc_allocPeer(peer);
		list_add_tail(&retryPeer->entry, &host->downPeerHead);

		// (void)close(peer->fd);

		// list_del(&peer->entry);
		// adm_arpc_freePeer(peer);

		adm_arpc_removePeer(peer);
		// log_i("adm_arpc_releasePeer");
		// adm_arpc_releasePeer(peer);
		log_i("adm_arpc_destroyPeer");
		adm_arpc_destroyPeer(peer);
	}
	else if (iret == ADM_SSL_WANT_WRITE)
	{
		//需要调用wirte接口避免阻塞
		if (ADM_ECODE_SUCCESSED != adm_arpc_addFileEvent(peer, AE_WRITABLE, adm_arpc_writeSSLEventHandler))
		{
			log_i("create socket writeable event error, close it.");
			adm_arpc_removePeer(peer);
			// log_i("adm_arpc_releasePeer");
			// adm_arpc_releasePeer(peer);
			log_i("adm_arpc_destroyPeer");
			adm_arpc_destroyPeer(peer);
		}
	}
	else
	{
		log_i("unknow ssl read error.");
	}

	return;
}

static void adm_arpc_serverHandshake(ADM_ARPC_EVENT_LOOP_T *loop, D32S fd, void *data, D32S mask)
{
	log_i("ssl server handshaking ...");
	D32S ret;
	(void)mask;
	ADM_ARPC_PEER_T *peer = (ADM_ARPC_PEER_T *)data;
	ADM_ARPC_HOST_T *host = amd_arpc_getHostInfo();
	ret = adm_com_sslAccept(peer->ssl);

	if (ret == 0)
	{

		// adm_arpc_deleteFileEvent(loop, fd, AE_WRITABLE);
		log_i("ssl handshake finished.");

		//握手成功时，将该peer添加到队列里，并配置flag
		// TCP建立成功时，添加队列
		// pthread_mutex_lock(&host->peerMutex);
		// list_add_tail(&peer->entry, &host->peerHead);
		// pthread_mutex_unlock(&host->peerMutex);
		adm_arpc_addPeer(peer);
		peer->sslState = ADM_ARPC_SSL_STATE_IN_SUCCESS;

		if (ADM_ECODE_SUCCESSED != adm_arpc_addFileEvent(peer, AE_READABLE, adm_arpc_readSSLEventHandler))
		{
			log_i("create socket readable event error, close it.");

			adm_arpc_removePeer(peer);
			adm_arpc_destroyPeer(peer);
		}
	}
	else if (ret == ADM_SSL_WANT_READ)
	{
		if (ADM_ECODE_SUCCESSED != adm_arpc_addFileEvent(peer, AE_READABLE, adm_arpc_serverHandshake))
		{
			log_e("create handshake readable event error, close it.");
			//创建event失败时，是否需要释放freePeer，多次容易释放导致段错误
			adm_arpc_destroyPeer(peer);
		};
	}
	else
	{
		log_e("ssl handshake failed,error code: %d", ret);
		//服务端不需要做，继续监听即可
		// (void)close(peer->fd);
		// if ((peer->entry.next != NULL) && (peer->entry.prev != NULL))
		// {
		// 	//删除peer时，会同时清楚host->peerHead.
		// 	list_del(&peer->entry);
		// }

		// adm_arpc_removePeer(peer);
		adm_arpc_destroyPeer(peer);

		//握手失败时，服务端需要重新监听TCP连接
		// if (ADM_ECODE_SUCCESSED != adm_arpc_createFileEvent(loop, host->listen_fd, AE_READABLE, adm_arpc_acceptSSLHandler, host))
		// {
		// 	log_e("SSL create handshake readable event error, close it.");
		// }
		// adm_arpc_freePeer(peer);
	}
}

static void adm_arpc_clientHandshake(ADM_ARPC_EVENT_LOOP_T *loop, D32S fd, void *data, D32S mask)
{
	log_i("ssl client handshaking ...");
	D32S ret;
	(void)mask;
	ADM_ARPC_PEER_T *peer = (ADM_ARPC_PEER_T *)data;
	ADM_ARPC_HOST_T *host = amd_arpc_getHostInfo();
	ret = adm_com_sslConnect(peer->ssl);

	if (ret == 0)
	{
		adm_arpc_register(peer);
		log_i("ssl handshake finished.");
		// pthread_mutex_lock(&host->peerMutex);
		// list_add_tail(&peer->entry, &host->peerHead);
		// pthread_mutex_unlock(&host->peerMutex);
		adm_arpc_addPeer(peer);

		if (ADM_ECODE_SUCCESSED == adm_arpc_addFileEvent(peer, AE_READABLE, adm_arpc_readSSLEventHandler))
		{
			if (peer->sslState == ADM_ARPC_SSL_STATE_IN_RETRY)
			{
				(void)pthread_mutex_lock(&peer->sslMutex);
				peer->sslState = ADM_ARPC_SSL_STATE_IN_SUCCESS;
                peer->bSslIsSignal = DTRUE;
				(void)pthread_cond_signal(&peer->sslCond);
				(void)pthread_mutex_unlock(&peer->sslMutex);
			}
			else if (peer->sslState == ADM_ARPC_SSL_STATE_IN_PROGRESS)
			{
				peer->sslState = ADM_ARPC_SSL_STATE_IN_SUCCESS;
			}
			else
			{
				log_i("ssl handshake already success in state %d",(D32S)peer->sslState);
			}
		}
		else
		{
			log_i("create socket readable event error, close it.");

			if (peer->sslState == ADM_ARPC_SSL_STATE_IN_PROGRESS)
			{
				ADM_ARPC_PEER_T *retryPeer = adm_arpc_allocPeer(peer);
				retryPeer->sslState = ADM_ARPC_SSL_STATE_IN_RETRY;
				if (NULL != retryPeer)
				{
					list_add_tail(&retryPeer->entry, &host->downPeerHead);
				}
				else
				{
					log_e("ssl handshake alloc retry peer fail");
				}
			}

			// adm_arpc_freePeer(peer);
			adm_arpc_removePeer(peer);
			log_i("adm_arpc_destroyPeer");
			adm_arpc_destroyPeer(peer);
			// adm_arpc_releasePeer(peer);
		}
	}
	else if (ret == ADM_SSL_WANT_READ)
	{
		if (ADM_ECODE_SUCCESSED != adm_arpc_addFileEvent(peer, AE_READABLE, adm_arpc_clientHandshake))
		{
			log_e("create handshake readable event error, close it.");

			if (peer->sslState == ADM_ARPC_SSL_STATE_IN_PROGRESS)
			{
				ADM_ARPC_PEER_T *retryPeer = adm_arpc_allocPeer(peer);
				retryPeer->sslState = ADM_ARPC_SSL_STATE_IN_RETRY;
				if (NULL != retryPeer)
				{
					list_add_tail(&retryPeer->entry, &host->downPeerHead);
				}
				else
				{
					log_e("ssl handshake alloc retry peer fail");
				}
			}

			// adm_arpc_freePeer(peer);
			adm_arpc_removePeer(peer);
			log_i("adm_arpc_destroyPeer");
			adm_arpc_destroyPeer(peer);
			// adm_arpc_releasePeer(peer);
		}
	}
	else
	{
		log_e("ssl handshake failed,error code: %d", ret);
		// (void)close(peer->fd);
		if (peer->sslState == ADM_ARPC_SSL_STATE_IN_RETRY)
		{
            peer->bSslIsSignal = DTRUE;
            (void)pthread_cond_signal(&peer->sslCond);
		}
		//重连流程中，downPeerHead已经存在，不需要再次添加。
		//同时也不需要清除，重连成功后会从downPeerHead里清除
		else if (peer->sslState == ADM_ARPC_SSL_STATE_IN_PROGRESS)
		{
			log_e("Add to retry list,fd is %d", peer->fd);
			//非重连握手时，需要将此次握手加入断线链表中
			ADM_ARPC_PEER_T *retryPeer = adm_arpc_allocPeer(peer);
			retryPeer->sslState = ADM_ARPC_SSL_STATE_IN_RETRY;
			if (NULL == retryPeer)
			{
				log_e("ssl handshake alloc retry peer fail");
				return;
			}
			list_add_tail(&retryPeer->entry, &host->downPeerHead);
			// if ((peer->entry.next != NULL) && (peer->entry.prev != NULL))
			// {
			// 	//删除peer时，会同时清除host->peerHead.
			// 	list_del(&peer->entry);
			// }
			// adm_arpc_freePeer(peer);

			// adm_arpc_removePeer(peer);
			log_i("adm_arpc_destroyPeer");
			adm_arpc_destroyPeer(peer);
		}
		else
		{
			log_i("ssl handshake already success in state %d",(D32S)peer->sslState);
		}
	}

	// log_i("adm_arpc_releasePeer");
	// adm_arpc_releasePeer(peer);
	return;
}

ADM_ARPC_PEER_T *adm_arpc_connectSSLServer(const DCHAR *ip, D16U port, D16U instanceId)
{
	ADM_ARPC_PEER_T *peer = NULL;
	ADM_ARPC_HOST_T *host = amd_arpc_getHostInfo();
	do
	{
		DCHAR err[ANET_ERR_LEN];
        D32S connfd = ANET_ERR;

        /** 判断客户端是否固定端口 */
        D32S iClientPort = adm_cfg_iniParserGetInt(ADM_CFG_VUS_DIS_TCP_HOST_PORT, ADM_CFG_VUS_DIS_TCP_HOST_PORT_DEFAULT_VAL);
        if (iClientPort != ADM_CFG_VUS_DIS_TCP_HOST_PORT_DEFAULT_VAL)
        {
            const DCHAR *pClientIp = adm_cfg_iniParserGetString(ADM_CFG_VUS_DIS_TCP_HOST_IP, ADM_CFG_VUS_DIS_TCP_HOST_IP_DEFAULT_VAL);
            connfd = anetTcpBindConnect(err, ip, (D16U)port, (DCHAR*)pClientIp, (D16U)iClientPort);
        }
        else
        {
            connfd = anetTcpConnect(err, ip, port);
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

		ADM_COM_SSL_T *conn_ssl = NULL;
		conn_ssl = adm_com_sslNew(host->ssl_ctx, connfd);
		if (conn_ssl == NULL)
		{
			log_e("create ssl failed...close socket");
			(void)close(connfd);
			break;
		}

		peer = adm_arpc_allocPeer(NULL);
		if (NULL == peer)
		{
			log_e("alloc peer error...close socket");
			(void)close(connfd);
			break;
		}

		peer->loop = host->loop;
		peer->fd = connfd;
		(void)adm_memcpy(peer->ip, ip, adm_strlen(ip), adm_strlen(ip));
		peer->port = port;
		peer->instanceId = instanceId;
		peer->hostMode = (D8S)HOSTMODE_SERVER;

		peer->ssl = conn_ssl;
		peer->sslState = ADM_ARPC_SSL_STATE_IN_PROGRESS;

		adm_arpc_retainPeer(peer);
		adm_arpc_clientHandshake(host->loop, peer->fd, peer, AE_READABLE);

	} while (FALSE);

	return peer;
}

void adm_arpc_acceptSSLHandler(ADM_ARPC_EVENT_LOOP_T *loop, D32S fd, void *data, D32S mask)
{
	DCHAR cip[64] = {0};
	D16U cport;

	ADM_ARPC_HOST_T *host = (ADM_ARPC_HOST_T *)data;

	D32S cfd = anetTcpAccept(NULL, fd, cip, (socklen_t)sizeof(cip), &cport);
	if (cfd != -1)
	{
		log_i("accepted ip %s:%d, fd:%d", cip, cport, cfd);
		(void)anetNonBlock(NULL, cfd);
		(void)anetEnableTcpNoDelay(NULL, cfd);

		ADM_COM_SSL_T *conn_ssl = NULL;
		conn_ssl = adm_com_sslNew(host->ssl_ctx, cfd);
		if (conn_ssl == NULL)
		{
			log_e("create ssl failed...close socket");
			(void)close(fd);
			return;
		}

		ADM_ARPC_PEER_T *peer = adm_arpc_allocPeer(NULL);
		if (NULL == peer)
		{
			log_i("alloc peer error...close socket");
			(void)close(fd);
			return;
		}

		peer->loop = loop;
		peer->fd = cfd;
		peer->hostMode = HOSTMODE_CLIENT;
		(void)adm_memcpy(peer->ip, cip, adm_strlen(cip), adm_strlen(cip));
		peer->port = cport;
		//异常处理需要添加
		peer->ssl = conn_ssl;
		peer->sslState = ADM_ARPC_SSL_STATE_IN_PROGRESS;

		adm_arpc_retainPeer(peer);
		adm_arpc_serverHandshake(loop, cfd, peer, AE_READABLE);
	}
	return;
}

//异步->同步接口
ADM_ARPC_PEER_T *adm_arpc_reConnectServer(DCHAR *ip, D16U port, D16U instanceId)
{
	ADM_ARPC_PEER_T *peer = NULL;
	ADM_ARPC_HOST_T *host = amd_arpc_getHostInfo();
	do
	{
		DCHAR err[ANET_ERR_LEN];
        D32S connfd = ANET_ERR;

        /** 判断客户端是否固定端口 */
        D32S iClientPort = adm_cfg_iniParserGetInt(ADM_CFG_VUS_DIS_TCP_HOST_PORT, ADM_CFG_VUS_DIS_TCP_HOST_PORT_DEFAULT_VAL);
        if (iClientPort != ADM_CFG_VUS_DIS_TCP_HOST_PORT_DEFAULT_VAL)
        {
            const DCHAR *pClientIp = adm_cfg_iniParserGetString(ADM_CFG_VUS_DIS_TCP_HOST_IP, ADM_CFG_VUS_DIS_TCP_HOST_IP_DEFAULT_VAL);
            connfd = anetTcpBindConnect(err, ip, (D16U)port, (DCHAR*)pClientIp, (D16U)iClientPort);
        }
        else 
        {
            connfd = anetTcpConnect(err, ip, port);
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

		ADM_COM_SSL_T *conn_ssl = NULL;
		conn_ssl = adm_com_sslNew(host->ssl_ctx, connfd);
		if (conn_ssl == NULL)
		{
			log_e("create ssl failed...close socket");
			(void)close(connfd);
			break;
		}
		peer = adm_arpc_allocPeer(NULL);
		if (peer == NULL)
		{
			log_e("alloc peer error...close socket");
			(void)close(connfd);
			break;
		}

		peer->loop = host->loop;
		peer->fd = connfd;
		(void)adm_memcpy(peer->ip, ip, adm_strlen(ip), adm_strlen(ip));
		peer->port = port;
		peer->instanceId = instanceId;
		peer->hostMode = (D8S)HOSTMODE_SERVER;
        peer->bSslIsSignal = DFALSE;

        peer->sslState = ADM_ARPC_SSL_STATE_IN_RETRY;
		(void)pthread_mutex_init(&peer->sslMutex, NULL);
		(void)pthread_condattr_setclock(&peer->sslCattr, CLOCK_MONOTONIC);
		(void)pthread_cond_init(&peer->sslCond, &peer->sslCattr);
		struct timespec timeout;
		peer->ssl = conn_ssl;
		
		adm_arpc_retainPeer(peer);
		adm_arpc_clientHandshake(host->loop, peer->fd, peer, AE_READABLE);

		(void)clock_gettime(CLOCK_MONOTONIC, &timeout);
		timeout.tv_sec += 8;

		(void)pthread_mutex_lock(&peer->sslMutex);
        D32S iStatus = adm_arpc_pthread_cond_timedwait(&peer->sslCond, &peer->sslMutex, &peer->bSslIsSignal, &timeout);

        //此次重连失败，重连流程不会将新建的peer放入downPeer中，此处需要释放资源
		if(peer->sslState == ADM_ARPC_SSL_STATE_IN_RETRY)
		{
			// adm_arpc_freePeer(peer);
			adm_arpc_destroyPeer(peer);
		}
		(void)pthread_mutex_unlock(&peer->sslMutex);

		(void)pthread_mutex_destroy(&peer->sslMutex);
		(void)pthread_cond_destroy(&peer->sslCond);
		
	} while (FALSE);

	return peer;
}

D32S adm_arpc_sslWrite(const ADM_ARPC_PEER_T *peer, const void *buf, D32S len)
{
	D32S ret = ADM_ECODE_FAILED;

    // 检查 peer 和 ssl 的有效性（注意：这个检查不是线程安全的，调用者应该确保在锁内调用）
	if (NULL == peer || NULL == peer->ssl)
	{
		log_e("adm_arpc_sslWrite() peer or ssl is NULL");
		return ADM_ECODE_FAILED;
	}

	D32S writen = adm_com_sslWrite(peer->ssl, (const DCHAR *)buf, len);
	if (writen > 0)
	{
		ret = writen;
	}
	else if (writen == ADM_SSL_WANT_WRITE)
	{
		//不用特殊处理,等待下次写调用即可
		ret = ADM_ECODE_SUCCESSED;
	}
	else
	{
		log_e("adm_arpc_sslWrite() failed,ret ADM_SSL_WANT_READ or ADM_SSL_ERROR");
	}
	return ret;
}

// 直接使用 ssl 指针的版本，避免访问 peer 结构
D32S adm_arpc_sslWriteWithSSL(ADM_COM_SSL_T *ssl, const void *buf, D32S len)
{
	D32S ret = ADM_ECODE_FAILED;
	
	if (NULL == ssl)
	{
		log_e("adm_arpc_sslWriteWithSSL() ssl is NULL");
		return ADM_ECODE_FAILED;
	}
	
	D32S writen = adm_com_sslWrite(ssl, (const DCHAR *)buf, len);
	if (writen > 0)
	{
		ret = writen;
	}
	else if (writen == ADM_SSL_WANT_WRITE)
	{
		//不用特殊处理,等待下次写调用即可
		ret = ADM_ECODE_SUCCESSED;
	}
	else
	{
		log_e("adm_arpc_sslWriteWithSSL() failed,ret ADM_SSL_WANT_READ or ADM_SSL_ERROR");
	}
	return ret;
}

#endif ///< ARPC_SSL
