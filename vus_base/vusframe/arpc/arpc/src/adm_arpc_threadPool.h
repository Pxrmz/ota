#ifndef __ADM_ARPC_THREADPOOL_H__
#define __ADM_ARPC_THREADPOOL_H__

#include <pthread.h>
#include "adm_typedefs.h"
#include "adm_queue.h"

#define ADM_ARPC_MIN_PTHREAD_NUM 1
#define ADM_ARPC_MAX_PTHREAD_NUM 8
// worker thread
typedef void* (*worker_func)(void* param);

typedef struct ADM_ARPC_THREAD_POOL ADM_ARPC_THREAD_POOL_T;

typedef struct ADM_ARPC_WORKER_THREAD
{
	pthread_t workerThreadId;
	pthread_attr_t workerAttr;
	
	pthread_mutex_t runMutex;
	pthread_cond_t runCond;
	pthread_condattr_t runCondAttr;
    DBOOL bRunIsSignal;
	
	worker_func proc;
	void* taskParm; // 可以扩容队列
	ADM_ARPC_THREAD_POOL_T* pool;

	D32S id;
	D32S iRunning; 
	D32S iStop;
}ADM_ARPC_WORKER_THREAD_T;

ADM_ARPC_WORKER_THREAD_T* adm_arpc_createWorkerThread(ADM_ARPC_THREAD_POOL_T* pool, worker_func func);

void adm_arpc_assignWorkerTask(ADM_ARPC_WORKER_THREAD_T* workerThread, void* task);


// thread pool
typedef struct ADM_ARPC_THREAD_POOL
{	
	D32S minCount;
	D32S maxCount;
	D32S curCount;
	//ADM_QUEUE_T* idleWorkers;
	worker_func proc;

	pthread_mutex_t poolMutex;
	
	ADM_ARPC_WORKER_THREAD_T* workers[1];
}ADM_ARPC_THREAD_POOL_T;

ADM_ARPC_THREAD_POOL_T* adm_arpc_initThreadPool(D32S minSize, D32S maxSize, worker_func func);

ADM_ARPC_WORKER_THREAD_T* adm_arpc_getWorkerThread(ADM_ARPC_THREAD_POOL_T* pool);

void adm_arpc_destroyThreadPool(ADM_ARPC_THREAD_POOL_T* pool);

#endif // __ADM_ARPC_THREADPOOL_H__
