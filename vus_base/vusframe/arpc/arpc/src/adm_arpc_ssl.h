#ifndef ADM_ARPC_SSL_H
#define ADM_ARPC_SSL_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <sys/uio.h>

#include "adm_typedefs.h"
#include "adm_arpc_eventLoop.h"
#include "adm_arpc_buffer.h"
#include "adm_com_sslAdapter.h"

#define ADM_ARPC_SSL_RECORD_MAX_LEN 16384
typedef struct ADM_ARPC_PEER ADM_ARPC_PEER_T;
typedef struct ADM_ARPC_HOST ADM_ARPC_HOST_T;

typedef enum
{
    ADM_ARPC_SSL_STATE_IN_PROGRESS = 0,
    ADM_ARPC_SSL_STATE_IN_SUCCESS,
    ADM_ARPC_SSL_STATE_IN_RETRY,
} ADM_ARPC_SSL_STATE_E;

D32S adm_arpc_sslCtxInit(ADM_ARPC_HOST_T *host);
void adm_arpc_sslFree(const ADM_ARPC_PEER_T *peer);
void adm_arpc_sslCtxFree(const ADM_ARPC_HOST_T *host);

D32S adm_arpc_sslBufferRead(ADM_ARPC_PEER_T *peer);
void adm_arpc_writeSSLEventHandler(ADM_ARPC_EVENT_LOOP_T *loop, D32S fd, void *data, D32S mask);
void adm_arpc_readSSLEventHandler(ADM_ARPC_EVENT_LOOP_T *loop, D32S fd, void *data, D32S mask);
ADM_ARPC_PEER_T *adm_arpc_connectSSLServer(const DCHAR *ip, D16U port, D16U instanceId);
void adm_arpc_acceptSSLHandler(ADM_ARPC_EVENT_LOOP_T *loop, D32S fd, void *data, D32S mask);
ADM_ARPC_PEER_T *adm_arpc_reConnectServer(DCHAR *ip, D16U port, D16U instanceId);
D32S adm_arpc_sslWrite(const ADM_ARPC_PEER_T *peer, const void *buf, D32S len);
void adm_arpc_setKeyPasswd(DCHAR *password);

#ifdef __cplusplus
}
#endif
#endif ///< ADM_ARPC_SSL_H
