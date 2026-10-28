#ifndef ADM_ARPC_SELECT_H
#define ADM_ARPC_SELECT_H

#include "adm_typedefs.h"
#include "poll.h"
#include "adm_arpc_define.h"
//#include <sys/time.h>

typedef struct ADM_ARPC_EVENT_LOOP ADM_ARPC_EVENT_LOOP_T;

typedef struct ADM_ARPC_IO_STATE 
{
    struct pollfd fds[DEFAULT_MAX_CLIENT_COUNT];
} ADM_ARPC_IO_STATE_T;

D32S adm_arpc_createEventLoopApi(ADM_ARPC_EVENT_LOOP_T *eventLoop);


//int adm_arpc_ApiResize(ADM_ARPC_EVENT_LOOP_T *eventLoop, int setsize);

//void adm_arpc_ApiFree(ADM_ARPC_EVENT_LOOP_T *eventLoop);

D32S adm_arpc_AddEvent(ADM_ARPC_EVENT_LOOP_T *eventLoop, D32S fd, D32S mask);

void adm_arpc_DelEvent(ADM_ARPC_EVENT_LOOP_T *eventLoop, D32S fd, D32S mask);

D32S adm_arpc_eventApiPoll(ADM_ARPC_EVENT_LOOP_T *eventLoop, D32S iTimeOut);

void adm_arpc_eventApiFree(ADM_ARPC_EVENT_LOOP_T *eventLoop);


#endif // ADM_ARPC_SELECT_H
