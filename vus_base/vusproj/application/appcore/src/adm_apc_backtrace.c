#include "adm_apc_log.h"
#include "adm_typedefs.h"
#include "adm_apc_backtrace.h"
#include "adm_memory.h"

#ifdef __ADM_GNU_EXTERNAL_BACK_TRACE__
#include <stdio.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#ifdef __QNX__

//qnx下无execinfo.h
#include <backtrace.h>
//实现backtrace
static void backtrace_signal_handler(D32S signal)
{
    return;
}
void adm_backtrace(void)
{
    struct sigaction sa;
    (void)memset(&sa, 0, sizeof(sa));
    sa.sa_handler = backtrace_signal_handler;
    return;
}
#else

#include <execinfo.h>  /*backtrace*/
static void backtrace_signal_handler(D32S signal)
{
    log_w("signal:%d", signal);

    DCHAR buf[10 * 1024] = {0};
    D64U buflen = sizeof(buf) / sizeof(buf[0]);
    D32S len = 0;


    void* array[128] = {0};
    D32S size = backtrace(array, 128);
    DCHAR** strings = (DCHAR**) backtrace_symbols(array, size);

    if(NULL != strings)
    {
        for(D32S i = 0; i < size; i++)
        {
            if(NULL != strings[i])
            {
                len += sprintf(buf + len, "%s\n", strings[i]);
            }
        }

        log_w("backtrace:%s", buf);
        adm_free(strings);
    }

    DCHAR file[64] = {0};
    (void)snprintf(file, sizeof(file) / sizeof(file[0]), "/proc/%d/maps", getpid());
    FILE* fp = fopen(file, "r");

    if(fp != NULL)
    {
        D64U rlen = fread(buf, 1, buflen - 1u, fp);

        if(rlen > 0u)
        {
            buf[rlen] = '\0';
            log_w("%s\n", buf);
        }

        (void)fclose(fp);
    }

    exit(1);
}

void adm_backtrace(void)
{
    struct sigaction sa;
    (void)memset(&sa, 0, sizeof(sa));
    sa.sa_handler = backtrace_signal_handler;
    (void)sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;

    D32S errNum = 0;

    do
    {
        /* Trap SIGSEGV */
        if(sigaction(SIGSEGV, &sa, NULL) == -1)
        {
            errNum = errno;
            log_w("sigaction(): %s\n", strerror(errNum));
            break;
        }

        /* Trap SIGABRT */
        if(sigaction(SIGABRT, &sa, NULL) == -1)
        {
            errNum = errno;
            log_w("sigaction(): %s\n", strerror(errNum));
            break;
        }

        /* Trap SIGPIPE */
        if(sigaction(SIGPIPE, &sa, NULL) == -1)
        {
            errNum = errno;
            log_w("sigaction(): %s", strerror(errNum));
            break;
        }
    }
    while(FALSE);
}
#endif


#else//__ADM_GNU_EXTERNAL_BACK_TRACE__

void adm_backtrace(void)
{
}

#endif//__ADM_GNU_EXTERNAL_BACK_TRACE__
