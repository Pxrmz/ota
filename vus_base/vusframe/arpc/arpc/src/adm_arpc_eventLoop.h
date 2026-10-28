#ifndef ADM_ARPC_EVENT_H
#define ADM_ARPC_EVENT_H

#include "adm_typedefs.h"
#include "adm_list.h"
#include <pthread.h>
#include <time.h>


#define AE_OK        0
#define AE_ERR      -1

#define AE_NONE      0
#define AE_READABLE  1
#define AE_WRITABLE  2

#define AE_FILE_EVENTS  1
#define AE_TIME_EVENTS  2
#define AE_ALL_EVENTS    (AE_FILE_EVENTS|AE_TIME_EVENTS)
#define AE_DONT_WAIT    4

#define AE_NOMORE            -1
#define AE_DELETED_EVENT_ID  -1

typedef struct ADM_ARPC_EVENT_LOOP ADM_ARPC_EVENT_LOOP_T;

typedef void (*ADM_ARPC_FILE_PROC)(ADM_ARPC_EVENT_LOOP_T *eventLoop, D32S fd, void *clientData, D32S mask);

typedef void (*ADM_ARPC_TIME_PROC)(ADM_ARPC_EVENT_LOOP_T *eventLoop, void *privData);

typedef struct ADM_ARPC_FILEEVENT 
{
    D32S mask;       /* one of AE_(READABLE|WRITABLE) */
    ADM_ARPC_FILE_PROC rfileProc;
    ADM_ARPC_FILE_PROC wfileProc;
    void *clientData;
} ADM_ARPC_FILEEVENT_T;

/* Time event structure */
typedef struct ADM_ARPC_TIMEEVENT 
{
	struct list_head entry;

    D64S interval;		/* -1, just once */ 
    D64S whenSec;      /* seconds */
    D64S whenMs;       /* milliseconds */
    ADM_ARPC_TIME_PROC timeProc;

    void *privData;
    
} ADM_ARPC_TIMEEVENT_T;

typedef struct ADM_ARPC_TIMEEVENT_HEAD 
{
    struct list_head head;
    D32S count;
} ADM_ARPC_TIMEEVENT_HEAD_T;

/* A fired event */
typedef struct  ADM_ARPC_FIREDEVENT 
{
    D32S fd;
    D32S mask;
} ADM_ARPC_FIREDEVENT_T;


typedef struct ADM_ARPC_FILE_EVENT 
{
	D32S fd;	
    D32S mask;       /* one of AE_(READABLE|WRITABLE) */
    ADM_ARPC_FILE_PROC rfileProc;
    ADM_ARPC_FILE_PROC wfileProc;
    void *clientData;
} ADM_ARPC_FILE_EVENT_T;

typedef struct ADM_ARPC_EVENT_LOOP 
{
    D32S maxfd;                  /* highest file descriptor currently registered */
    D32S setSize;                /* max number of file descriptors tracked */
    ADM_ARPC_TIMEEVENT_HEAD_T *timeEventHead;
    
    ADM_ARPC_FILE_EVENT_T *events;        /* Registered events */
    ADM_ARPC_FIREDEVENT_T *fired;        /* Fired events */
    
    void *apidata;              /* This is used for polling API specific data */
    //aeBeforeSleepProc *beforesleep;
    pthread_mutex_t     mutex;
} ADM_ARPC_EVENT_LOOP_T;

ADM_ARPC_EVENT_LOOP_T* adm_arpc_createEventLoop(D32U setSize);

ADM_ARPC_FILE_EVENT_T* adm_arpc_getFileEvent(ADM_ARPC_EVENT_LOOP_T* eventLoop, D32S fd);

// file event
D32S adm_arpc_createFileEvent(ADM_ARPC_EVENT_LOOP_T *eventLoop, D32S fd, D32S mask, ADM_ARPC_FILE_PROC proc, void *privData);
D32S adm_arpc_setFileEvent(ADM_ARPC_EVENT_LOOP_T *eventLoop, D32S fd, D32S mask, ADM_ARPC_FILE_PROC proc, void *privData);
D32S adm_arpc_deleteFileEvent(ADM_ARPC_EVENT_LOOP_T *eventLoop, D32S fd, D32S mask); 
D32S adm_arpc_deleteAllFileEvent(ADM_ARPC_EVENT_LOOP_T *eventLoop, D32S fd);

// time event
D32S adm_arpc_createTimeEvent(ADM_ARPC_EVENT_LOOP_T *eventLoop, D64S milliseconds, ADM_ARPC_TIME_PROC proc, void *privData);

D32S adm_arpc_clearTimeEvent(ADM_ARPC_EVENT_LOOP_T *eventLoop);

void adm_arpc_eventLoop(ADM_ARPC_EVENT_LOOP_T* eventLoop);

void adm_arpc_processTimeEvent(ADM_ARPC_EVENT_LOOP_T* eventLoop);

void adm_arpc_deleteEventLoop(ADM_ARPC_EVENT_LOOP_T* eventLoop);

#endif // ADM_ARPC_EVENT_H
