#include "adm_arpc_eventLoop.h"
#include "adm_arpc_select.h"
#include "adm_arpc_log.h"
#include "adm_com_errorCode.h"
#include "adm_memory.h"
#include "adm_arpc_log.h"
#include <pthread.h>
#include <sys/time.h>



ADM_ARPC_EVENT_LOOP_T* adm_arpc_createEventLoop(D32U setSize)
{
	ADM_ARPC_EVENT_LOOP_T* eventLoop = NULL;

	D32S bFlag = 0;

	do
	{
		eventLoop = (ADM_ARPC_EVENT_LOOP_T*)adm_malloc(sizeof(ADM_ARPC_EVENT_LOOP_T));
		if(NULL == eventLoop)
		{
			break;
		}
		
		eventLoop->events = (ADM_ARPC_FILE_EVENT_T*)adm_malloc(sizeof(ADM_ARPC_FILE_EVENT_T) * setSize);
		eventLoop->fired = (ADM_ARPC_FIREDEVENT_T*)adm_malloc(sizeof(ADM_ARPC_FIREDEVENT_T) * setSize);

		if(NULL == eventLoop->events || NULL == eventLoop->fired)
		{
			break;
		}

		(void)adm_memset(eventLoop->events, 0x0, sizeof(ADM_ARPC_FILE_EVENT_T) * setSize, sizeof(ADM_ARPC_FILE_EVENT_T) * setSize);
		(void)adm_memset(eventLoop->fired, 0x0, sizeof(ADM_ARPC_FIREDEVENT_T) * setSize, sizeof(ADM_ARPC_FIREDEVENT_T) * setSize);

		eventLoop->setSize = (D32S)setSize;

		// 初始化时间事件
    	eventLoop->timeEventHead = (ADM_ARPC_TIMEEVENT_HEAD_T*)adm_malloc(sizeof(ADM_ARPC_TIMEEVENT_HEAD_T));
    	
    	INIT_LIST_HEAD(&eventLoop->timeEventHead->head);
    	eventLoop->timeEventHead->count = 0;
		
		eventLoop->maxfd = -1;

		(void)adm_arpc_createEventLoopApi(eventLoop);

		for (D32U i = 0; i < setSize; i++) 
		{
	        eventLoop->events[i].mask = AE_NONE;
			eventLoop->events[i].fd = 0;
	    }

		pthread_mutex_init(&eventLoop->mutex, NULL);
		bFlag = 1;
		
	}while(false);

	if(0 == bFlag)
	{
		adm_free(eventLoop->events);
		adm_free(eventLoop->fired);
		adm_free(eventLoop);	
		eventLoop = NULL;
	}

	return eventLoop;
}

D32S adm_arpc_createFileEvent(ADM_ARPC_EVENT_LOOP_T *eventLoop, D32S fd, D32S mask, ADM_ARPC_FILE_PROC proc, void *privData)
{
	/*
	if (fd > eventLoop->setSize) 
	{
        return ADM_ECODE_FAILED;
    }
	*/
	ADM_ARPC_FILE_EVENT_T *fe = NULL;
	for(D32S i=0; i<eventLoop->setSize; i++)
	{
		if(eventLoop->events[i].fd != 0 && eventLoop->events[i].fd != fd )
		{
			continue;
		}

		fe = &eventLoop->events[i];
		break;
	}
	
    //ADM_ARPC_FILE_EVENT_T *fe = &eventLoop->events[fd];

	fe->fd = fd;
	if (adm_arpc_AddEvent(eventLoop, fd, mask) == -1) 
	{
        return ADM_ECODE_FAILED;
    }

    fe->mask |= mask;
    if ( (mask & AE_READABLE) != 0) 
    {
        fe->rfileProc = proc;
    }

    if ( (mask & AE_WRITABLE) != 0) 
    {
        fe->wfileProc = proc;
    }

    fe->clientData = privData;
    if (fd > eventLoop->maxfd) 
    {
        eventLoop->maxfd = fd;
    }

    return ADM_ECODE_SUCCESSED;
}

D32S adm_arpc_setFileEvent(ADM_ARPC_EVENT_LOOP_T *eventLoop, D32S fd, D32S mask, ADM_ARPC_FILE_PROC proc, void *privData)
{
	/*
	if (fd > eventLoop->setSize) 
	{
        return ADM_ECODE_FAILED;
    }
	*/
	ADM_ARPC_FILE_EVENT_T *fe = NULL;
	for(D32S i=0; i<eventLoop->setSize; i++)
	{
		if(eventLoop->events[i].fd != 0 && eventLoop->events[i].fd != fd )
		{
			continue;
		}

		fe = &eventLoop->events[i];
		break;
	}
	
    //ADM_ARPC_FILE_EVENT_T *fe = &eventLoop->events[fd];

	fe->fd = fd;
	if (adm_arpc_AddEvent(eventLoop, fd, mask) == -1) 
	{
        return -1;
    }

	D32S lastMask = fe->mask;
    fe->mask |= mask;
    if ( (mask & AE_READABLE) != 0) 
    {
        fe->rfileProc = proc;
    }

    if ( (mask & AE_WRITABLE) != 0) 
    {
        fe->wfileProc = proc;
    }

    fe->clientData = privData;
    if (fd > eventLoop->maxfd) 
    {
        eventLoop->maxfd = fd;
    }

    return lastMask;
}

D32S adm_arpc_deleteFileEvent(ADM_ARPC_EVENT_LOOP_T *eventLoop, D32S fd, D32S mask) 
{
	/*
    if (fd >= eventLoop->setSize) 
    {
        return ADM_ECODE_FAILED;
    }
	*/
	
	int iNeedupdateMaxfd = 0;
	
    //ADM_ARPC_FILE_EVENT_T *fe = &eventLoop->events[fd];
	pthread_mutex_lock(&eventLoop->mutex);
	ADM_ARPC_FILE_EVENT_T *fe = adm_arpc_getFileEvent(eventLoop, fd);
	if(fe == NULL)
	{
		pthread_mutex_unlock(&eventLoop->mutex);
		return ADM_ECODE_FAILED;
	}

	if (AE_NONE == (mask & fe->mask))
    {
		pthread_mutex_unlock(&eventLoop->mutex);
        return ADM_ECODE_FAILED;
    }

    adm_arpc_DelEvent(eventLoop, fd, mask);

    fe->mask &= ~mask;

	if(fd == eventLoop->maxfd)
	{
		iNeedupdateMaxfd = 1;
	}
	
	if(fe->mask == AE_NONE)
	{
		fe->fd = 0;
		fe->clientData = NULL;
		fe->rfileProc = NULL;
		fe->wfileProc = NULL;
	}

	if(1 == iNeedupdateMaxfd)
	{
		D32S iMaxFd = 0;
		for(D32S i=0; i<eventLoop->setSize; i++)
		{
			if(eventLoop->events[i].fd != 0 && eventLoop->events[i].fd > iMaxFd)
			{
				iMaxFd = eventLoop->events[i].fd;
			}
		}

		eventLoop->maxfd = iMaxFd;
		log_i("maxfd = %d", eventLoop->maxfd);
	}

	pthread_mutex_unlock(&eventLoop->mutex);

    return ADM_ECODE_SUCCESSED;
}

D32S adm_arpc_deleteAllFileEvent(ADM_ARPC_EVENT_LOOP_T *eventLoop, D32S fd) 
{
	/*
    if (fd >= eventLoop->setSize) 
    {
        return ADM_ECODE_FAILED;
    }
	*/
	
	int iNeedupdateMaxfd = 0;
	
    //ADM_ARPC_FILE_EVENT_T *fe = &eventLoop->events[fd];
	pthread_mutex_lock(&eventLoop->mutex);
	ADM_ARPC_FILE_EVENT_T *fe = adm_arpc_getFileEvent(eventLoop, fd);
	if(fe == NULL)
	{
		pthread_mutex_unlock(&eventLoop->mutex);
		return AE_NONE;
	}

	if (AE_NONE == fe->mask)
    {
		pthread_mutex_unlock(&eventLoop->mutex);
        return AE_NONE;
    }

	D32S mask = fe->mask;
    adm_arpc_DelEvent(eventLoop, fd, fe->mask);
    fe->mask = AE_NONE;

	if(fd == eventLoop->maxfd)
	{
		iNeedupdateMaxfd = 1;
	}
	
	if(fe->mask == AE_NONE)
	{
		fe->fd = 0;
		fe->clientData = NULL;
		fe->rfileProc = NULL;
		fe->wfileProc = NULL;
	}

	if(1 == iNeedupdateMaxfd)
	{
		D32S iMaxFd = 0;
		for(D32S i=0; i<eventLoop->setSize; i++)
		{
			if(eventLoop->events[i].fd != 0 && eventLoop->events[i].fd > iMaxFd)
			{
				iMaxFd = eventLoop->events[i].fd;
			}
		}

		eventLoop->maxfd = iMaxFd;
		log_i("maxfd = %d", eventLoop->maxfd);
	}

	pthread_mutex_unlock(&eventLoop->mutex);

    return mask;
}

// 将milliseconds个毫秒转换为sec个秒和ms个毫秒，保存到参数中
static void adm_arpc_AddMillisecondsToNow(D64S milliseconds, D64S *sec, D64S *ms) 
{
   
    // 获取当前的时间
    struct timespec tv;
	(void)clock_gettime(CLOCK_MONOTONIC, &tv);
	D64S cur_sec = tv.tv_sec;
	D64S cur_ms = tv.tv_nsec/1000000; // 毫秒
    
    // 获取秒数
    D64S whenSec = cur_sec + milliseconds/1000;
    // 获取毫秒数
    D64S whenMs = cur_ms + milliseconds%1000;
    // 毫秒数大于1000需要进位
    if (whenMs >= 1000) 
    {
        whenSec ++;
        whenMs -= 1000;
    }
    // 保存秒数和毫秒数
    *sec = whenSec;
    *ms = whenMs;
}


D32S adm_arpc_createTimeEvent(ADM_ARPC_EVENT_LOOP_T *eventLoop, D64S milliseconds, ADM_ARPC_TIME_PROC proc, void *privData)
{
	// malloc node
	ADM_ARPC_TIMEEVENT_T* timeEventNode = adm_malloc(sizeof(ADM_ARPC_TIMEEVENT_T));
	if(NULL == timeEventNode)
	{
		return ADM_ECODE_FAILED;
	}

	timeEventNode->interval = milliseconds;
	timeEventNode->timeProc = proc;
	timeEventNode->privData = privData;

	adm_arpc_AddMillisecondsToNow(milliseconds,&timeEventNode->whenSec,&timeEventNode->whenMs);

	// list add 
	list_add_tail(&timeEventNode->entry, &eventLoop->timeEventHead->head);
	eventLoop->timeEventHead->count++;

	return ADM_ECODE_SUCCESSED;
}

D32S adm_arpc_clearTimeEvent(ADM_ARPC_EVENT_LOOP_T *eventLoop)
{
	ADM_ARPC_TIMEEVENT_T *p, *n;
    list_for_each_entry_safe(p, n, &eventLoop->timeEventHead->head, ADM_ARPC_TIMEEVENT_T, entry)
    {
		list_del(&p->entry);
		eventLoop->timeEventHead->count--;
	}
	return ADM_ECODE_SUCCESSED;
}

ADM_ARPC_FILE_EVENT_T* adm_arpc_getFileEvent(ADM_ARPC_EVENT_LOOP_T* eventLoop, D32S fd)
{
	ADM_ARPC_FILE_EVENT_T *fe = NULL;
	for(D32S i=0; i<eventLoop->setSize; i++)
	{
		if(eventLoop->events[i].fd == fd)
		{
			fe = &eventLoop->events[i];
			break;
		}
	}

	return fe;
}

void adm_arpc_eventLoop(ADM_ARPC_EVENT_LOOP_T* eventLoop)
{	
	struct timeval tvp;
    D32S iTimeOut = 100; // 100 ms

	D32S numevents = adm_arpc_eventApiPoll(eventLoop, iTimeOut);
    // log_d("numevents[%d]", numevents);
    // for (D32S i = 0; i < numevents; i++) 
    // {
    //     //ADM_ARPC_FILE_EVENT_T *fe = &eventLoop->events[eventLoop->fired[i].fd];
    //     log_i("numevents[%d] mask[%d] fd[%d]", numevents, eventLoop->fired[i].mask, eventLoop->fired[i].fd);
    //     ADM_ARPC_FILE_EVENT_T *fe = adm_arpc_getFileEvent(eventLoop, eventLoop->fired[i].fd);
	// 	if(fe == NULL)
	// 	{
    //         log_e("fe is null.");
	// 		return ;
	// 	}
		
    //     int mask = eventLoop->fired[i].mask;
    //     int fd = eventLoop->fired[i].fd;

    //     int rfired = 0;
        
    //     if ((fe->mask & mask & AE_READABLE) != 0 ) 
    //     {
    //         rfired = 1;
    //         fe->rfileProc(eventLoop, fd, fe->clientData, mask);
    //     }
        
    //     if ((fe->mask & mask & AE_WRITABLE) != 0 )
    //     {
    //         if (!rfired || fe->wfileProc != fe->rfileProc) 
    //         {
    //             fe->wfileProc(eventLoop, fd, fe->clientData, mask);
    //         }
    //     }

    //     eventLoop->fired[i].mask = 0;
    //     eventLoop->fired[i].fd = 0;
        
    // }

    return;
}

void adm_arpc_processTimeEvent(ADM_ARPC_EVENT_LOOP_T* eventLoop)
{
	// 循环list
	ADM_ARPC_TIMEEVENT_T* pTimeEvent = NULL;
	list_for_each_entry(pTimeEvent, &eventLoop->timeEventHead->head, ADM_ARPC_TIMEEVENT_T, entry)
	{

	    struct timespec tv;
		(void)clock_gettime(CLOCK_MONOTONIC, &tv);
		D64S nowSec = tv.tv_sec;
		D64S nowMs = (D32S)(tv.tv_nsec/1000000); // 毫秒

	    if (nowSec > pTimeEvent->whenSec ||
            (nowSec == pTimeEvent->whenSec && nowMs >= pTimeEvent->whenMs))
        {
            int retval;

            // 调用时间事件处理方法
            pTimeEvent->timeProc(eventLoop, pTimeEvent->privData);

              // 如果不是定时事件，则继续设置它的到时时间
            if (pTimeEvent->interval != AE_NOMORE) 
            {
                adm_arpc_AddMillisecondsToNow(pTimeEvent->interval,&pTimeEvent->whenSec,&pTimeEvent->whenMs);
            }
            else
            {
            	// delte timer
            	list_del(&pTimeEvent->entry);
            	adm_free(pTimeEvent);

            	eventLoop->timeEventHead->count--;
            }

        }
	}

    return;
}

void adm_arpc_deleteEventLoop(ADM_ARPC_EVENT_LOOP_T* eventLoop)
{
	adm_arpc_eventApiFree(eventLoop);
	
	adm_free(eventLoop->events);
	adm_free(eventLoop->fired);
	pthread_mutex_destroy(&eventLoop->mutex);
	adm_free(eventLoop);
}


