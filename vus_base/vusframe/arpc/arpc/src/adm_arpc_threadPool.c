#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <sys/time.h>
#include <errno.h>
#include <sys/uio.h>

#include "adm_arpc_threadPool.h"
#include "adm_arpc_log.h"
#include "adm_memory.h"
#include "adm_arpc_threadCond.h"

// worker thread
static void* adm_arpc_workerThreadProc(void *argv)
{
	ADM_ARPC_WORKER_THREAD_T* workerThread = (ADM_ARPC_WORKER_THREAD_T*)argv;
	while(true)
	{
		if(0 == workerThread->iStop)
		{
			(void)pthread_mutex_lock(&workerThread->runMutex);
			if(0 == workerThread->iRunning)
			{
			
			    struct timespec timeout;
				(void)clock_gettime(CLOCK_MONOTONIC, &timeout);
				timeout.tv_sec += 1;
				
                workerThread->bRunIsSignal = DFALSE;
                D32S status = adm_arpc_pthread_cond_timedwait(&workerThread->runCond, &workerThread->runMutex,
                                                     &workerThread->bRunIsSignal, &timeout); // 等待100ms
                if(0 == status)
				{
					(void)workerThread->proc(workerThread->taskParm);

					(void)pthread_mutex_lock(&workerThread->pool->poolMutex);	
					workerThread->taskParm = NULL;
					workerThread->iRunning = 0;
					(void)pthread_mutex_unlock(&workerThread->pool->poolMutex);
					//(void)adm_queue_push(workerThread->pool->idleWorkers, &workerThread, (D32U)sizeof(ADM_ARPC_WORKER_THREAD_T*));
					log_i("workerThread :%d done, threadid:%lld ",workerThread->id,  workerThread->workerThreadId);
					
				}
				(void)pthread_mutex_unlock(&workerThread->runMutex);

				//log_i("push, front data:%p", workerThread->pool->idleWorkers->front->data);

			}
			else
			{
				(void)workerThread->proc(workerThread->taskParm);

				(void)pthread_mutex_lock(&workerThread->pool->poolMutex);
				workerThread->taskParm = NULL;
				workerThread->iRunning = 0;
				(void)pthread_mutex_unlock(&workerThread->pool->poolMutex);
				//(void)adm_queue_push(workerThread->pool->idleWorkers, &workerThread, (D32U)sizeof(ADM_ARPC_WORKER_THREAD_T*));
				(void)pthread_mutex_unlock(&workerThread->runMutex);
				log_i("workerThread :%d done, threadid:%lld ",workerThread->id, workerThread->workerThreadId);
				
			}
		}
		else
		{
			(void)pthread_mutex_destroy(&workerThread->runMutex);
			(void)pthread_cond_destroy(&workerThread->runCond);

			adm_free(workerThread);
			pthread_exit(NULL);
			
		}
	}
	
}

ADM_ARPC_WORKER_THREAD_T* adm_arpc_createWorkerThread(ADM_ARPC_THREAD_POOL_T* pool, worker_func func)
{
	ADM_ARPC_WORKER_THREAD_T* workerThread = NULL;
	workerThread = adm_malloc(sizeof(ADM_ARPC_WORKER_THREAD_T));
	if(NULL == workerThread)
	{
		log_e("creat workerThread fail");
		return NULL;
	}

	//(void)adm_memset(workerThread, 0x0, sizeof(ADM_ARPC_WORKER_THREAD_T), sizeof(ADM_ARPC_WORKER_THREAD_T));
	(void)pthread_mutex_init(&workerThread->runMutex, NULL);
	(void)pthread_condattr_setclock(&workerThread->runCondAttr, CLOCK_MONOTONIC);
	(void)pthread_cond_init(&workerThread->runCond, &workerThread->runCondAttr);

	(void)pthread_attr_init(&workerThread->workerAttr); 
	(void)pthread_attr_setdetachstate(&workerThread->workerAttr, PTHREAD_CREATE_DETACHED);   
	if ( 0 != pthread_create(&workerThread->workerThreadId, &workerThread->workerAttr, adm_arpc_workerThreadProc, workerThread) )
    {
        log_e("pthread_create tid:%d failed", workerThread->workerThreadId);

        adm_free(workerThread);
        return NULL;
    }

	workerThread->pool = pool;
    workerThread->iRunning = 0;
    workerThread->taskParm = NULL;
	workerThread->proc = func;
	workerThread->iStop = 0;
    return workerThread;
}

void adm_arpc_assignWorkerTask(ADM_ARPC_WORKER_THREAD_T* workerThread, void* task)
{
	(void)pthread_mutex_lock(&workerThread->runMutex);
	(void)pthread_mutex_lock(&workerThread->pool->poolMutex);	
	workerThread->iRunning = 1;
	workerThread->taskParm = task;
	(void)pthread_mutex_unlock(&workerThread->pool->poolMutex);
    workerThread->bRunIsSignal = DTRUE;
    (void)pthread_cond_signal(&workerThread->runCond);
	(void)pthread_mutex_unlock(&workerThread->runMutex);
}	

// thread pool
ADM_ARPC_THREAD_POOL_T* adm_arpc_initThreadPool(D32S minSize, D32S maxSize, worker_func func)
{
	if(minSize < 1)
	{
		minSize = 1; // default
	}

	ADM_ARPC_THREAD_POOL_T* pool = NULL;
	D32U poolSize = (D32U)(sizeof(ADM_ARPC_THREAD_POOL_T) + sizeof(ADM_ARPC_WORKER_THREAD_T) * (D32U)maxSize);
	pool = adm_malloc(poolSize);
	if(NULL == pool)
	{
		log_e("malloc threawdpool fail");
		return NULL;
	}
	(void)adm_memset(pool, 0x0, poolSize, poolSize);
	//pool->idleWorkers = adm_queue_new();
	pool->minCount = minSize;
	pool->maxCount = maxSize;
	pool->curCount = 0;
	pool->proc = func;

	(void)pthread_mutex_init(&pool->poolMutex, NULL);
	
	for(D32S i=0; i< minSize; i++)
	{
		ADM_ARPC_WORKER_THREAD_T* workerThread = adm_arpc_createWorkerThread(pool, func);
		if(NULL == workerThread)
		{
			//adm_queue_free(pool->idleWorkers);
			adm_free(pool);
			return NULL;
		}
		//(void)adm_queue_push(pool->idleWorkers,&workerThread,(D32U)sizeof(ADM_ARPC_WORKER_THREAD_T*)); // push 指针值

		workerThread->id = i;
		pool->workers[i] = workerThread;
		pool->curCount++;

		log_i("create worker thread:%d, %llu", i,  workerThread->workerThreadId);
	}

	return pool;

}

ADM_ARPC_WORKER_THREAD_T* adm_arpc_getWorkerThread(ADM_ARPC_THREAD_POOL_T* pool)
{
	D32S iFind = 0;
	ADM_ARPC_WORKER_THREAD_T* workerThread = NULL;
	
	do{
		///< 减少一次锁, 不判断非空
		///< 有空闲线程
		//if(0 == adm_queue_isEmpty(pool->idleWorkers))
		//{
/*
		//log_i(" pop, front data:%p", pool->idleWorkers->front->data);
		D32U size = 0;
		void* data = NULL;
		//(void)adm_queue_pop(pool->idleWorkers, &data, &size);

		if(NULL != data)
		{
			workerThread = *(ADM_ARPC_WORKER_THREAD_T**)data;
			adm_free(data);

			///< 消息队列出问题了, 或线程未执行结束提前入队列
			if(1 == workerThread->iRunning && NULL != workerThread->taskParm)
			{
				log_e("workerThread id:%d, threadid:%lld is busy",workerThread->id, workerThread->workerThreadId);
				continue;
			}
			else
			{
				break;
			}
		}
*/
		(void)pthread_mutex_lock(&pool->poolMutex);
		for(int i=0; i< pool->curCount; i++)
		{
			if(0 == pool->workers[i]->iRunning && NULL == pool->workers[i]->taskParm)
			{
				workerThread = pool->workers[i];
				(void)pthread_mutex_unlock(&pool->poolMutex);
				break;
			}
		}
		(void)pthread_mutex_unlock(&pool->poolMutex);

		if (workerThread)
		{
			break;
		}

		//log_i("pool is full, curCount:%d, maxCount:%d", pool->curCount, pool->maxCount);

		///< 没找到, 还有扩容空间
		if(NULL == workerThread && pool->curCount < pool->maxCount)
		{
			workerThread = adm_arpc_createWorkerThread(pool, pool->proc);
			if(NULL == workerThread)
			{
				return NULL;
			}

			(void)pthread_mutex_lock(&pool->poolMutex);
			workerThread->id = pool->curCount;
			//(void)adm_queue_push(pool->idleWorkers, &workerThread,(D32U)sizeof(ADM_ARPC_WORKER_THREAD_T*));
			pool->workers[pool->curCount] = workerThread;
			pool->curCount++;
			(void)pthread_mutex_unlock(&pool->poolMutex);
			
			break;
		}

		///< 没扩容空间, 且都在忙
		log_i("all worker is running, wait");

		(void)usleep(50 * 1000); // 50 ms
		
	}while(true);
		

	log_i("get workerThread id:%d, threadid:%lld",workerThread->id, workerThread->workerThreadId);

	return workerThread;
}

void adm_arpc_destroyThreadPool(ADM_ARPC_THREAD_POOL_T* pool)
{
	ADM_ARPC_WORKER_THREAD_T* workerThread = NULL;
	D32S size = 0;
	for(D32S i = 0; i < pool->curCount; i++)
	{
		workerThread = pool->workers[i];

		(void)pthread_join(workerThread->workerThreadId, NULL);
		workerThread->iStop = 1;

		adm_free(workerThread);
	}

	//adm_queue_free(pool->idleWorkers);
	(void)pthread_mutex_destroy(&pool->poolMutex);
	adm_free(pool);
}
