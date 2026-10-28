#include <sys/stat.h>
#include <sys/un.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <netdb.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "adm_arpc_log.h"
#include "adm_arpc_net.h"


static void anetSetError(char *err, const char *fmt, ...) {
    va_list ap;

    if ( NULL == err) {
        return;
    }
    va_start(ap, fmt);
    (void)vsnprintf(err, ANET_ERR_LEN, fmt, ap);
    va_end(ap);
}

static int anetSetBlock(char *err, int fd, int non_block) {
    int flags;

    /* Set the socket blocking (if non_block is zero) or non-blocking.
     * Note that fcntl(2) for F_GETFL and F_SETFL can't be
     * interrupted by a signal. */
    if ((flags = fcntl(fd, F_GETFL)) == -1) {
        anetSetError(err, "fcntl(F_GETFL): %s", strerror(errno));
        return ANET_ERR;
    }

    if (0 != non_block) {
        flags |= O_NONBLOCK;
    } else {
        flags &= ~O_NONBLOCK;
    }

    if (fcntl(fd, F_SETFL, flags) == -1) {
        anetSetError(err, "fcntl(F_SETFL, O_NONBLOCK): %s", strerror(errno));
        return ANET_ERR;
    }
    return ANET_OK;
}

int anetNonBlock(char *err, int fd) {
    return anetSetBlock(err, fd, 1);
}

int anetBlock(char *err, int fd) {
    return anetSetBlock(err, fd, 0);
}

/* Set TCP keep alive option to detect dead peers. The interval option
 * is only used for Linux as we are using Linux-specific APIs to set
 * the probe send time, interval, and count. */
int anetKeepAlive(char *err, int fd, int interval) {
    int val = 1;

    if (setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, &val, (socklen_t)(sizeof(val))) == -1) {
        anetSetError(err, "setsockopt SO_KEEPALIVE: %s", strerror(errno));
        return ANET_ERR;
    }

#ifdef __linux__
    /* Default settings are more or less garbage, with the keepalive time
     * set to 7200 by default on Linux. Modify settings to make the feature
     * actually useful. */

    /* Send first probe after interval. */
    val = interval;
    if (setsockopt(fd, IPPROTO_TCP, TCP_KEEPIDLE, &val, (socklen_t)(sizeof(val))) < 0) {
        anetSetError(err, "setsockopt TCP_KEEPIDLE: %s\n", strerror(errno));
        return ANET_ERR;
    }

    /* Send next probes after the specified interval. Note that we set the
     * delay as interval / 3, as we send three probes before detecting
     * an error (see the next setsockopt call). */
    val = interval/3;
    if (val == 0){
		val = 1;
    }
	
    if (setsockopt(fd, IPPROTO_TCP, TCP_KEEPINTVL, &val, (socklen_t)(sizeof(val))) < 0) {
        anetSetError(err, "setsockopt TCP_KEEPINTVL: %s\n", strerror(errno));
        return ANET_ERR;
    }

    /* Consider the socket in error state after three we send three ACK
     * probes without getting a reply. */
    val = 3;
    if (setsockopt(fd, IPPROTO_TCP, TCP_KEEPCNT, &val, (socklen_t)(sizeof(val))) < 0) {
        anetSetError(err, "setsockopt TCP_KEEPCNT: %s\n", strerror(errno));
        return ANET_ERR;
    }
#else
    ((void) interval); /* Avoid unused var warning for non Linux systems. */
#endif

    return ANET_OK;
}

int anetTcpKeepAlive(char *err, int fd) {
    int yes = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, &yes, (socklen_t)(sizeof(yes))) == -1) {
        anetSetError(err, "setsockopt SO_KEEPALIVE: %s", strerror(errno));
        return ANET_ERR;
    }
    return ANET_OK;
}

static int anetSetTcpNoDelay(char *err, int fd, int val) {
    if (setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &val, (socklen_t)(sizeof(val))) == -1) {
        anetSetError(err, "setsockopt TCP_NODELAY: %s", strerror(errno));
        return ANET_ERR;
    }
    return ANET_OK;
}

int anetEnableTcpNoDelay(char *err, int fd) {
    return anetSetTcpNoDelay(err, fd, 1);
}

int anetDisableTcpNoDelay(char *err, int fd) {
    return anetSetTcpNoDelay(err, fd, 0);
}

int anetSetSendBuffer(char *err, int fd, int buffsize) {
    if (setsockopt(fd, SOL_SOCKET, SO_SNDBUF, &buffsize, (socklen_t)(sizeof(buffsize))) == -1) {
        anetSetError(err, "setsockopt SO_SNDBUF: %s", strerror(errno));
        return ANET_ERR;
    }
    return ANET_OK;
}

/* Set the socket send timeout (SO_SNDTIMEO socket option) to the specified
 * number of milliseconds, or disable it if the 'ms' argument is zero. */
int anetSendTimeout(char *err, int fd, long long ms) {
    struct timeval tv;

    tv.tv_sec = ms / 1000;
    tv.tv_usec = (ms % 1000) * 1000;
    if (setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &tv, (socklen_t)(sizeof(tv))) == -1) {
        anetSetError(err, "setsockopt SO_SNDTIMEO: %s", strerror(errno));
        return ANET_ERR;
    }
    return ANET_OK;
}

/* anetGenericResolve() is called by anetResolve() and anetResolveIP() to
 * do the actual work. It resolves the hostname "host" and set the string
 * representation of the IP address into the buffer pointed by "ipbuf".
 *
 * If flags is set to ANET_IP_ONLY the function only resolves hostnames
 * that are actually already IPv4 or IPv6 addresses. This turns the function
 * into a validating / normalizing function. */
static int anetGenericResolve(char *err, char *host, char *ipbuf, socklen_t ipbuf_len, int flags) {
    struct addrinfo hints, *info;
    int rv;

    memset(&hints, '\0', sizeof(hints));
    if ((flags & ANET_IP_ONLY) != 0) {
        hints.ai_flags = AI_NUMERICHOST;
    }
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = (int)SOCK_STREAM;  /* specify socktype to avoid dups */

    if ((rv = getaddrinfo(host, NULL, &hints, &info)) != 0) {
        anetSetError(err, "%s", gai_strerror(rv));
        return ANET_ERR;
    }
    if (info->ai_family == AF_INET) {
        struct sockaddr_in *sa = (struct sockaddr_in *) info->ai_addr;
        (void)inet_ntop(AF_INET, &(sa->sin_addr), ipbuf, ipbuf_len);
    } else {
        struct sockaddr_in6 *sa = (struct sockaddr_in6 *) info->ai_addr;
        (void)inet_ntop(AF_INET6, &(sa->sin6_addr), ipbuf, ipbuf_len);
    }

    freeaddrinfo(info);

    return ANET_OK;
}

int anetResolve(char *err, char *host, char *ipbuf, socklen_t ipbuf_len) {
    return anetGenericResolve(err, host, ipbuf, ipbuf_len, ANET_NONE);
}

int anetResolveIP(char *err, char *host, char *ipbuf, socklen_t ipbuf_len) {
    return anetGenericResolve(err, host, ipbuf, ipbuf_len, ANET_IP_ONLY);
}

static int anetSetReuseAddr(char *err, int fd) {
    int yes = 1;
    /* Make sure connection-intensive things like the redis benckmark
     * will be able to close/open sockets a zillion of times */
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, (socklen_t)(sizeof(yes))) == -1) {
        anetSetError(err, "setsockopt SO_REUSEADDR: %s", strerror(errno));
        return ANET_ERR;
    }
    return ANET_OK;
}

static int anetCreateSocket(char *err, int domain) {
    int s;
    if ((s = socket(domain, (int)SOCK_STREAM, 0)) == -1) {
        anetSetError(err, "creating socket: %s", strerror(errno));
        return ANET_ERR;
    }

    /* Make sure connection-intensive things like the redis benchmark
     * will be able to close/open sockets a zillion of times */
    if (anetSetReuseAddr(err, s) == ANET_ERR) {
        (void)close(s);
        return ANET_ERR;
    }
    return s;
}

#define ANET_CONNECT_NONE 0
#define ANET_CONNECT_NONBLOCK 1
#define ANET_CONNECT_BE_BINDING 2 /* Best effort binding. */
static int anetTcpGenericConnect(char *err, const char *addr, D16U port, char *source_addr, int flags) {
    int s = ANET_ERR, rv;
    char portstr[6];  /* strlen("65535") + 1; */
    struct addrinfo hints, *servinfo, *bservinfo, *p, *b;

    (void)snprintf(portstr, sizeof(portstr), "%d", port);
    (void)memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = (int)SOCK_STREAM;

    if ((rv = getaddrinfo(addr, portstr, &hints, &servinfo)) != 0) {
        anetSetError(err, "%s", gai_strerror(rv));
        return ANET_ERR;
    }
    for (p = servinfo; p != NULL; p = p->ai_next) {
        /* Try to create the socket and to connect it.
         * If we fail in the socket() call, or on connect(), we retry with
         * the next entry in servinfo. */
        if ((s = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            continue;
        }
        if (anetSetReuseAddr(err, s) == ANET_ERR) {
            goto error;
        }

        struct linger linger_opt = {1, 0}; // 立即关闭模式
        setsockopt(s, SOL_SOCKET, SO_LINGER, &linger_opt, sizeof(linger_opt));

        if (flags & ANET_CONNECT_NONBLOCK && anetNonBlock(err, s) != ANET_OK) {
            goto error;
        }
        if (source_addr != NULL) {
            int bound = 0;
            /* Using getaddrinfo saves us from self-determining IPv4 vs IPv6 */
            if ((rv = getaddrinfo(source_addr, NULL, &hints, &bservinfo)) != 0) {
                anetSetError(err, "%s", gai_strerror(rv));
                goto error;
            }
            for (b = bservinfo; b != NULL; b = b->ai_next) {
                if (bind(s, b->ai_addr, b->ai_addrlen) != -1) {
                    bound = 1;
                    break;
                }
            }
            freeaddrinfo(bservinfo);
            if (0 == bound) {
                anetSetError(err, "bind: %s", strerror(errno));
                goto error;
            }
        }
        if (connect(s, p->ai_addr, p->ai_addrlen) == -1) {
            anetSetError(err, "connect: %s", strerror(errno));
            /* If the socket is non-blocking, it is ok for connect() to
             * return an EINPROGRESS error here. */
            if (errno == EINPROGRESS && flags & ANET_CONNECT_NONBLOCK) {
                goto end;
            }
            (void)close(s);
            s = ANET_ERR;
            continue;
        }

        /* If we ended an iteration of the for loop without errors, we
         * have a connected socket. Let's return to the caller. */
        goto end;
    }
    if (p == NULL) {
        anetSetError(err, "creating socket: %s", strerror(errno));
    }

error:
    if (s != ANET_ERR) {
        (void)close(s);
        s = ANET_ERR;
    }

end:
    freeaddrinfo(servinfo);

    /* Handle best effort binding: if a binding address was used, but it is
     * not possible to create a socket, try again without a binding address. */
    if (s == ANET_ERR && NULL != source_addr && (flags & ANET_CONNECT_BE_BINDING)) {
        return anetTcpGenericConnect(err, addr, port, NULL, flags);
    } else {
        return s;
    }
}

static int anetTcpGenericBindConnect(char *err, const char *addr, D16U port, char *source_addr, D16U source_port, int flags)
{
    int s = ANET_ERR, rv;
    char portstr[6]; /* strlen("65535") + 1; */
    char clientPortStr[6];
    struct addrinfo hints, *servinfo, *bservinfo, *p, *b;

    (void)snprintf(portstr, sizeof(portstr), "%d", port);
    (void)snprintf(clientPortStr, sizeof(clientPortStr), "%d", source_port);
    (void)memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = (int)SOCK_STREAM;

    if ((rv = getaddrinfo(addr, portstr, &hints, &servinfo)) != 0)
    {
        anetSetError(err, "%s", gai_strerror(rv));
        return ANET_ERR;
    }
    for (p = servinfo; p != NULL; p = p->ai_next)
    {
        /* Try to create the socket and to connect it.
         * If we fail in the socket() call, or on connect(), we retry with
         * the next entry in servinfo. */
        if ((s = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
        {
            continue;
        }

        if (anetSetReuseAddr(err, s) == ANET_ERR)
        {
            goto error;
        }

        struct linger linger_opt = {1, 0}; // 立即关闭模式
        setsockopt(s, SOL_SOCKET, SO_LINGER, &linger_opt, sizeof(linger_opt));

        if (flags & ANET_CONNECT_NONBLOCK && anetNonBlock(err, s) != ANET_OK)
        {
            goto error;
        }
        if (source_addr != NULL)
        {
            int bound = 0;
            /* Using getaddrinfo saves us from self-determining IPv4 vs IPv6 */
            if ((rv = getaddrinfo(source_addr, clientPortStr, &hints, &bservinfo)) != 0)
            {
                anetSetError(err, "%s", gai_strerror(rv));
                goto error;
            }
            for (b = bservinfo; b != NULL; b = b->ai_next)
            {
                if (bind(s, b->ai_addr, b->ai_addrlen) != -1)
                {
                    bound = 1;
                    break;
                }
            }
            freeaddrinfo(bservinfo);
            if (0 == bound)
            {
                anetSetError(err, "bind: %s", strerror(errno));
                goto error;
            }
        }
        if (connect(s, p->ai_addr, p->ai_addrlen) == -1)
        {
            /* If the socket is non-blocking, it is ok for connect() to
             * return an EINPROGRESS error here. */
            if (errno == EINPROGRESS && flags & ANET_CONNECT_NONBLOCK)
            {
                goto end;
            }
            (void)close(s);
            s = ANET_ERR;
            continue;
        }

        /* If we ended an iteration of the for loop without errors, we
         * have a connected socket. Let's return to the caller. */
        goto end;
    }
    if (p == NULL)
    {
        anetSetError(err, "creating socket: %s", strerror(errno));
    }

error:
    if (s != ANET_ERR)
    {
        (void)close(s);
        s = ANET_ERR;
    }

end:
    freeaddrinfo(servinfo);

    /* Handle best effort binding: if a binding address was used, but it is
     * not possible to create a socket, try again without a binding address. */
    if (s == ANET_ERR && NULL != source_addr && (flags & ANET_CONNECT_BE_BINDING))
    {
        return anetTcpGenericBindConnect(err, addr, port, source_addr, source_port, flags);
    }
    else
    {
        return s;
    }
}

int anetTcpConnect(char *err, const char *addr, D16U port) {
    return anetTcpGenericConnect(err, addr, port, NULL, ANET_CONNECT_NONE);
}

int anetTcpNonBlockConnect(char *err, char *addr, D16U port) {
    return anetTcpGenericConnect(err, addr, port, NULL, ANET_CONNECT_NONBLOCK);
}

int anetTcpNonBlockBindConnect(char *err, char *addr, D16U port, char *source_addr) {
    return anetTcpGenericConnect(err, addr, port, source_addr, ANET_CONNECT_NONBLOCK);
}

int anetTcpNonBlockBestEffortBindConnect(char *err, char *addr, D16U port, char *source_addr) {
    return anetTcpGenericConnect(err, addr, port, source_addr, ANET_CONNECT_NONBLOCK | ANET_CONNECT_BE_BINDING);
}

int anetTcpBindConnect(char *err, char *addr, D16U port, char *source_addr, D16U source_port)
{
    return anetTcpGenericBindConnect(err, addr, port, source_addr, source_port, ANET_CONNECT_NONE);
}

int anetTcpBindNonBlockConnect(char *err, char *addr, D16U port, char *source_addr, D16U source_port)
{
    return anetTcpGenericBindConnect(err, addr, port, source_addr, source_port, ANET_CONNECT_NONBLOCK);
}

static int anetUnixGenericConnect(char *err, char *path, int flags) {
    int s;
    struct sockaddr_un sa;

    if ((s = anetCreateSocket(err, AF_LOCAL)) == ANET_ERR){
        return ANET_ERR;
    }
	
    sa.sun_family = AF_LOCAL;
    (void)strncpy(sa.sun_path, path, sizeof(sa.sun_path) - (D32U)1);
    if ((flags & ANET_CONNECT_NONBLOCK) != 0) {
        if (anetNonBlock(err, s) != ANET_OK) {
			(void)close(s);
            return ANET_ERR;
        }
    }
    if (connect(s, (struct sockaddr *) &sa, (socklen_t)(sizeof(sa))) == -1) {
        if (errno == EINPROGRESS && flags & ANET_CONNECT_NONBLOCK) {
            return s;
        }
        anetSetError(err, "connect: %s", strerror(errno));
        (void)close(s);
        return ANET_ERR;
    }

    return s;
}

int anetUnixConnect(char *err, char *path) {
    return anetUnixGenericConnect(err, path, ANET_CONNECT_NONE);
}

int anetUnixNonBlockConnect(char *err, char *path) {
    return anetUnixGenericConnect(err, path, ANET_CONNECT_NONBLOCK);
}

/* Like read(2) but make sure 'count' is read before to return
 * (unless error or EOF condition is encountered) */
int anetRead(int fd, char *buf, int count) {
    int nread, totlen = 0;
    while (totlen != count) {
        nread = (int)read(fd, buf, (size_t)count - (size_t)totlen);
        if (nread == 0) {
            return totlen;
        }
        if (nread == -1) {
            return -1;
        }
        totlen += nread;
        buf += nread;
    }

    return totlen;
}

/* Like write(2) but make sure 'count' is written before to return
 * (unless error is encountered) */
int anetWrite(int fd, char *buf, int count) {
    int nwritten, totlen = 0;

    while (totlen != count) {
		errno = 0;
        nwritten = (int)write(fd, buf, (size_t)count - (size_t)totlen);
        if (nwritten == 0) {
            return totlen;
        }
        if (nwritten == -1) {
			if (errno != EAGAIN) 
			{
				log_e("Failed to write errNum=%d, reason=%s", errno, strerror(errno));
				return -1;
			}
        }

        totlen += nwritten;
        buf += nwritten;
    }

    return totlen;
}

static int anetListen(char *err, int s, struct sockaddr *sa, socklen_t len, int backlog) {
    if (bind(s, sa, len) == -1) {
        anetSetError(err, "bind: %s", strerror(errno));
        (void)close(s);
        return ANET_ERR;
    }

    if (listen(s, backlog) == -1) {
        anetSetError(err, "listen: %s", strerror(errno));
        (void)close(s);
        return ANET_ERR;
    }

    return ANET_OK;
}

static int anetV6Only(char *err, int s) {
    int yes = 1;
    if (setsockopt(s, IPPROTO_IPV6, IPV6_V6ONLY, &yes, (socklen_t)(sizeof(yes))) == -1) {
        anetSetError(err, "setsockopt: %s", strerror(errno));
        (void)close(s);
        return ANET_ERR;
    }

    return ANET_OK;
}

static int _anetTcpServer(char *err, int port, char *bindaddr, int af, int backlog) {
    int s, rv;
    char _port[6];  /* strlen("65535") */
    struct addrinfo hints;
	struct addrinfo *servinfo =NULL;
	struct addrinfo *p = NULL;

    (void)snprintf(_port, 6, "%d", port);
    (void)memset(&hints, 0, sizeof(hints));
    hints.ai_family = af;
    hints.ai_socktype = (int)SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;    /* No effect if bindaddr != NULL */

    if ((rv = getaddrinfo(bindaddr, _port, &hints, &servinfo)) != 0) {
        anetSetError(err, "%s", gai_strerror(rv));
        return ANET_ERR;
    }

    for (p = servinfo; p != NULL; p = p->ai_next) {
        if ((s = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            continue;
        }
        if (af == AF_INET6 && anetV6Only(err, s) == ANET_ERR) {
            goto error;
        }
        if (anetSetReuseAddr(err, s) == ANET_ERR) {
            goto error;
        }

        struct linger so_linger;
		so_linger.l_onoff = 1;
		so_linger.l_linger = 0;
		if(0 != setsockopt(s,SOL_SOCKET,SO_LINGER, &so_linger,(socklen_t)(sizeof(so_linger)))){
			goto error;
		}

        if (anetListen(err, s, p->ai_addr, p->ai_addrlen, backlog) == ANET_ERR) {
            goto error;
        }
        goto end;
    }
    if (p == NULL) {
        anetSetError(err, "unable to bind socket");
        goto error;
    }

error:
    s = ANET_ERR;

end:
    freeaddrinfo(servinfo);

    return s;
}

int anetTcpServer(char *err, int port, char *bindaddr, int backlog) {
    return _anetTcpServer(err, port, bindaddr, AF_INET, backlog);
}

int anetTcp6Server(char *err, int port, char *bindaddr, int backlog) {
    return _anetTcpServer(err, port, bindaddr, AF_INET6, backlog);
}

int anetUnixServer(char *err, char *path, mode_t perm, int backlog) {
    int s;
    struct sockaddr_un sa;

    if ((s = anetCreateSocket(err, AF_LOCAL)) == ANET_ERR) {
        return ANET_ERR;
    }

    (void)memset(&sa, 0, sizeof(sa));
    sa.sun_family = AF_LOCAL;
    (void)strncpy(sa.sun_path, path, sizeof(sa.sun_path) - (D32U)1);

    if (anetListen(err, s, (struct sockaddr *) &sa, (socklen_t)(sizeof(sa)), backlog) == ANET_ERR) {
        return ANET_ERR;
    }
    if ((D32U)0 != perm) {
        int ret = chmod(sa.sun_path, perm);
		if(0 != ret)
		{
			return ret;
		}
    }

    return s;
}

static int anetGenericAccept(char *err, int s, struct sockaddr *sa, socklen_t *len) {
    int fd;
    while (true) {
        fd = accept(s, sa, len);
        if (fd == -1) {
            if (errno == EINTR) {
                continue;
            } else {
                anetSetError(err, "accept: %s", strerror(errno));
                return ANET_ERR;
            }
        }
        break;
    }
    return fd;
}

int anetTcpAccept(char *err, int serversock, char *ip, socklen_t ip_len, D16U *port) {
    int fd;
    struct sockaddr_storage sa;
    socklen_t salen = (D32S)sizeof(sa);

    if ((fd = anetGenericAccept(err, serversock, (struct sockaddr *) &sa, &salen)) == -1) {
        return ANET_ERR;
    }

    if (sa.ss_family == (D16U)AF_INET) {
        struct sockaddr_in *serversock = (struct sockaddr_in *) &sa;
        if (ip != NULL ) {
            (void)inet_ntop(AF_INET, (void *) &(serversock->sin_addr), ip, ip_len);
        }
        if (port != NULL) {
            *port = ntohs(serversock->sin_port);
        }
    } else {
        struct sockaddr_in6 *serversock6 = (struct sockaddr_in6 *) &sa;
        if (ip != NULL ) {
            (void)inet_ntop(AF_INET6, (void *) &(serversock6->sin6_addr), ip, ip_len);
        }
        if (port != NULL) {
            *port = ntohs(serversock6->sin6_port);
        }
    }

    return fd;
}

int anetUnixAccept(char *err, int serversock) {
    int fd;
    struct sockaddr_un sa;
    socklen_t salen = (socklen_t)sizeof(sa);
    if ((fd = anetGenericAccept(err, serversock, (struct sockaddr *) &sa, &salen)) == -1) {
        return ANET_ERR;
    }

    return fd;
}

/* Format an IP,port pair into something easy to parse. If IP is IPv6
 * (matches for ":"), the ip is surrounded by []. IP and port are just
 * separated by colons. This the standard to display addresses within Redis. */
static int anetFormatAddr(char *buf, size_t buf_len, char *ip, D16U port) {
    return snprintf(buf, buf_len, strchr(ip, (int)':') != NULL ? "[%s]:%d" : "%s:%d", ip, port);
}

static int anetPeerToString(int fd, char *ip, socklen_t ip_len, D16U *port) {
    struct sockaddr_storage sa;
    socklen_t salen = (socklen_t)sizeof(sa);

    if (getpeername(fd, (struct sockaddr *) &sa, &salen) == -1) {
        goto error;
    }
    if (ip_len == (socklen_t)0) {
        goto error;
    }

    if (sa.ss_family == (D16U)AF_INET) {
        struct sockaddr_in *s = (struct sockaddr_in *) &sa;
        if (ip != NULL ) {
            (void)inet_ntop(AF_INET, (void *) &(s->sin_addr), ip, ip_len);
        }
        if (NULL != port) {
            *port = ntohs(s->sin_port);
        }
    } else if (sa.ss_family == (D16U)AF_INET6) {
        struct sockaddr_in6 *s = (struct sockaddr_in6 *) &sa;
        if (ip != NULL ) {
            (void)inet_ntop(AF_INET6, (void *) &(s->sin6_addr), ip, ip_len);
        }
        if (NULL != port) {
            *port = ntohs(s->sin6_port);
        }
    } else if (sa.ss_family == (D16U)AF_UNIX) {
        if (ip != NULL ) {
            (void)strncpy(ip, "/unixsocket", ip_len);
        }
        if (port != NULL) {
            *port = 0;
        }
    } else {
        goto error;
    }

    return 0;

error:
    if (ip != NULL ) {
        if (ip_len >= (socklen_t)2) {
            ip[0] = '?';
            ip[1] = '\0';
        } else if (ip_len == (socklen_t)1) {
            ip[0] = '\0';
        } else{
			*port = 0;
		}
    }
    if (NULL != port) {
        *port = 0;
    }

    return -1;
}

static int anetSockName(int fd, char *ip, socklen_t ip_len, D16U *port) {
    struct sockaddr_storage sa;
    socklen_t salen = (socklen_t)(sizeof(sa));

    if (getsockname(fd, (struct sockaddr *) &sa, &salen) == -1) {
        if (NULL == port) {
			*port = 0;
        }
        ip[0] = '?';
        ip[1] = '\0';
        return -1;
    }
    if (sa.ss_family == (D16U)AF_INET) {
        struct sockaddr_in *s = (struct sockaddr_in *) &sa;
        if (NULL != ip) {
            (void)inet_ntop(AF_INET, (void *) &(s->sin_addr), ip, ip_len);
        }
        if (NULL != port) {
            *port = ntohs(s->sin_port);
        }
    } else {
        struct sockaddr_in6 *s = (struct sockaddr_in6 *) &sa;
        if (NULL != ip) {
            (void)inet_ntop(AF_INET6, (void *) &(s->sin6_addr), ip, ip_len);
        }
        if (NULL != port) {
            *port = ntohs(s->sin6_port);
        }
    }

    return 0;
}

/* Like anetFormatAddr() but extract ip and port from the socket's peer. */
int anetFormatPeer(int fd, char *buf, size_t buf_len) {
    char ip[INET6_ADDRSTRLEN];
    D16U port;

    (void)anetPeerToString(fd, ip, (socklen_t)(sizeof(ip)), &port);

    return anetFormatAddr(buf, buf_len, ip, port);
}

int anetFormatSock(int fd, char *buf, size_t buf_len) {
    char ip[INET6_ADDRSTRLEN];
    D16U port;

    (void)anetSockName(fd, ip, (socklen_t)sizeof(ip), &port);

    return anetFormatAddr(buf, buf_len, ip, port);
}
