#ifndef _ADM_THREAD_H_
#define _ADM_THREAD_H_

#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif


//100 milliseconds
#define IDLE_TIME           1000000
#define MAX_THREAD_NAME     16

typedef DBOOL (* THREAD_EVENT_FUNC)(void* param);

#define DM_PL_INVALIDTHREADID	0xFFFFFFFF	//Invalid thread id

typedef enum {
    ThreadStateDown,               // thread is not running
    ThreadStateBusy,               // thread is currently handling a task
    ThreadStateWaiting,            // thread is waiting for something to do
    ThreadStateShuttingDown,       // thread is in the process of shutting down
    ThreadStateFault               // an error has occurred and the thread could not be launched
} adm_thread_state_t;

typedef struct {
    pthread_cond_t m_ready;
    pthread_mutex_t m_lock;
    DBOOL m_isSignaled;
} adm_thread_event_t;

typedef struct {
    adm_thread_event_t event;      // event controller
    pthread_t           hThread;    // thread handle
    pthread_t           id;         // id of this thread
    pthread_mutex_t     lock;       // mutex that protects threads internal data


    THREAD_EVENT_FUNC   handleEventFunc;
    void               *handleEventParam;
    DBOOL               isRunning;      // set to TRUE if thread is running
    adm_thread_state_t        state;          // current state of thread see thread state data

//#if !defined (PROD_MIN) && defined (PR_SET_NAME)
    char                name[MAX_THREAD_NAME];  //not necessarily null-terminated
//#endif
    void *pQueue;

} adm_thread_t;




extern D32U adm_thread_get_tid(void);
DBOOL adm_thread_signal(adm_handle_t inThreadHandle);



//thread create
adm_handle_t	adm_thread_create_noname(void);
adm_handle_t	adm_thread_create_byname(const char* inName);


adm_handle_t adm_thread_create(void *threadHandle, void *param, D16U queueNum, D16U queueSize);


void adm_thread_set_func(
		adm_handle_t		inThreadHandle, 
		THREAD_EVENT_FUNC	inHandleEventFunc, 
		void*				inHandleEventParam);

//start / stop / destroy
DBOOL adm_thread_start(adm_handle_t inThreadHandle);
DBOOL adm_thread_stop(adm_handle_t inThreadHandle);
void adm_thread_destroy(adm_handle_t inThreadHandle);

//
void adm_thread_set_name(DCHAR *inName);
void DM_PL_Thread_wait(adm_handle_t inThreadHandle);


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* _ADM_THREAD_H_ */

