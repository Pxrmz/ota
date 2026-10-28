

#include "adm_arpc_define.h"
#include "adm_com_errorCode.h"
#include "adm_arpc_eventLoop.h"
#include "adm_arpc_select.h"
#include "adm_arpc_log.h"

#include "adm_memory.h"


#include <sys/types.h>
#include <unistd.h>

static DVOID arpcClearPollFd(struct pollfd* pstfds, D32S iSize, D32S iFd)
{
    for (D32S i = 0; i < iSize; i++)
    {
        if (iFd == pstfds[i].fd)
        {
            pstfds[i].fd = -1;
            pstfds[i].events = 0;
            pstfds[i].revents = 0;
            break;
        }
    }

    return;
}

D32S adm_arpc_createEventLoopApi(ADM_ARPC_EVENT_LOOP_T *eventLoop) 
{
    log_d("POLLOUT:[%d] POLLIN[%d]", POLLOUT, POLLIN);
    ADM_ARPC_IO_STATE_T *state = adm_malloc(sizeof(ADM_ARPC_IO_STATE_T));

    if (NULL == state) 
    {
        return ADM_ECODE_FAILED;
    }

    for (D32S i = 0; i < DEFAULT_MAX_CLIENT_COUNT; i++)
    {
        state->fds[i].fd = -1;
        state->fds[i].events = 0;
        state->fds[i].revents = 0;
    }

    eventLoop->apidata = state;

    return ADM_ECODE_SUCCESSED;
}

D32S adm_arpc_AddEvent(ADM_ARPC_EVENT_LOOP_T *eventLoop, D32S fd, D32S mask) 
{
    ADM_ARPC_IO_STATE_T *state = eventLoop->apidata;
    D32S iEvent = -1;

    log_i("Add event fd:[%d]", fd);

    if (0 != (mask & AE_READABLE))
    {
        iEvent = POLLIN;
    }
    else if (0 != (mask & AE_WRITABLE))
    {
        iEvent = POLLOUT;
    }
    else 
    {
        log_w("Error mode.[%d]", mask);
        return ADM_ECODE_SUCCESSED;
    }

    for (D32S i = 0; i < DEFAULT_MAX_CLIENT_COUNT; i++)
    {
        if (-1 == state->fds[i].fd)
        {
            log_i("state->fds[%d].fd set to %d", i, fd);
            state->fds[i].fd = fd;
            state->fds[i].events = iEvent;
            break;
        }
    }

    return ADM_ECODE_SUCCESSED;
}

void adm_arpc_DelEvent(ADM_ARPC_EVENT_LOOP_T *eventLoop, D32S fd, D32S mask) 
{
    ADM_ARPC_IO_STATE_T *state = eventLoop->apidata;

    arpcClearPollFd(state->fds, DEFAULT_MAX_CLIENT_COUNT, fd);

    return;
}

D32S adm_arpc_eventApiPoll(ADM_ARPC_EVENT_LOOP_T *eventLoop, D32S iTimeOut) 
{
    if (eventLoop == NULL || eventLoop->apidata == NULL) 
    {
        log_e("Invalid eventLoop or apidata");
        return -1;
    }

    ADM_ARPC_IO_STATE_T *state = eventLoop->apidata;
    D32S retval = 0;
	D32S numevents = 0;
    ADM_ARPC_FILE_EVENT_T *fe = NULL;

    retval = poll(state->fds, DEFAULT_MAX_CLIENT_COUNT, iTimeOut);
    if (retval > 0) 
    {
        for (D32S j = 0; j < eventLoop->setSize; j++) 
        {
            if (-1 == state->fds[j].fd)
            {
                continue;
            }

            // int mask = 0;

            fe = &eventLoop->events[j];
            if (NULL == fe)
            {
                log_e("Find abnormal fd[%d] event.", state->fds[j].fd);
                continue;
            }

            if ((state->fds[j].events & POLLIN) && (state->fds[j].revents & POLLIN))
            {
                // mask |= AE_READABLE;
                log_d("poll in fd[%d] events[%d] revents[%d]", state->fds[j].fd, state->fds[j].events, state->fds[j].revents);
                if (fe->rfileProc != NULL)
                {
                    fe->rfileProc(eventLoop, fe->fd, fe->clientData, 0);
                    numevents++;
                }
                else 
                {
                    log_e("rfileProc is NULL for fd[%d]", state->fds[j].fd);
                }
            }

            if ((state->fds[j].events & POLLOUT) && (state->fds[j].revents & POLLOUT))
            {
                log_d("poll out fd[%d] events[%d] revents[%d]", state->fds[j].fd, state->fds[j].events, state->fds[j].revents);
                // mask |= AE_WRITABLE;
                if (fe->wfileProc != NULL)
                {
                    fe->wfileProc(eventLoop, fe->fd, fe->clientData, 0);
                    numevents++;
                }
                else 
                {
                    log_e("wfileProc is NULL for fd[%d]", state->fds[j].fd);
                }
            }

			// if(mask == 0)
			// {
			// 	continue;
			// }
            // else 
            // {
            //     log_i("fd[%d] events[%d] revents[%d]", state->fds[j].fd, state->fds[j].events, state->fds[j].revents);
            // }

            // eventLoop->fired[numevents].fd = state->fds[j].fd;
            // eventLoop->fired[numevents].mask = mask;
            // numevents++;
        }
    }

    return numevents;
}

void adm_arpc_eventApiFree(ADM_ARPC_EVENT_LOOP_T *eventLoop)
{
	adm_free(eventLoop->apidata);

    return;
}

