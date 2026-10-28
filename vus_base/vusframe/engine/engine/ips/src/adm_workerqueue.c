#include "adm_workerqueue_internal.h"
#include "adm_memory.h"
#include "adm_ips_log.h"


ADM_WORKER_QUEUE_T *adm_workerQueueNew(ADM_WORKER_QUEUE_ENGINE_F *fnEngine){
    D32U maxItems = ADM_WORKER_QUEUE_MAX_ITEMS;
    ADM_WORKER_QUEUE_T *wq = adm_malloc((DSIZE)sizeof(*wq) + (((DSIZE)maxItems - 1UL) * sizeof(ADM_WORKER_QUEUE_ITEM_T)));
    if(NULL != wq){
        workerQueueInit(wq, fnEngine, maxItems);
    }
    return wq;
}
void adm_workerQueueDel(ADM_WORKER_QUEUE_T *wq){
    if(NULL != wq){
        workerQueueDeInit(wq);
	    adm_free(wq);
    }
}
void adm_workerQueueQuit(ADM_WORKER_QUEUE_T *wq){
    if(NULL != wq){
        workerQueueQuit(wq);
    }
}
void adm_workerQueueAdd(ADM_WORKER_QUEUE_T *wq, void *param){
    workerQueueAdd(wq, param);
}


/* ADM_WORKER_QUEUE_T */
static void workerQueueInit(ADM_WORKER_QUEUE_T *wq, ADM_WORKER_QUEUE_ENGINE_F *fnEngine, D32U maxItems){
    (void)pthread_cond_init(&wq->cond, NULL);
    (void)pthread_mutex_init(&wq->mutex, NULL);
    wq->quit = DFALSE;
    wq->fnEngine = fnEngine;
    wq->items = 0U;
    wq->maxItems = maxItems;
    wq->read = 0U;
    wq->write = 0U;

    if(pthread_create(&wq->id, NULL, workerQueueEngine, wq) != 0){
	    log_w("thread create failed");
    }
}
static void workerQueueDeInit(ADM_WORKER_QUEUE_T *wq){
    (void)pthread_cond_destroy(&wq->cond);
    (void)pthread_mutex_destroy(&wq->mutex);
}
static void workerQueueQuit(ADM_WORKER_QUEUE_T *wq){
    (void)pthread_mutex_lock(&wq->mutex);
    wq->quit = DTRUE;
    (void)pthread_cond_signal(&wq->cond);
    (void)pthread_mutex_unlock(&wq->mutex);
    (void)pthread_join(wq->id, NULL);
}
static void workerQueueAdd(ADM_WORKER_QUEUE_T *wq, void *param){
    (void)pthread_mutex_lock(&wq->mutex);
    if(wq->quit == DFALSE && wq->items < wq->maxItems){
        wq->head[wq->write].param = param;
        wq->items++;
        wq->write++;
        if (wq->write == wq->maxItems) {
            wq->write = 0U;
        }
        (void)pthread_cond_signal(&wq->cond);
    }else{
	    log_w("items:%u, quit:%d", wq->items, wq->quit);
    }
    (void)pthread_mutex_unlock(&wq->mutex);
}

/* ADM_WORKER_QUEUE_T engine */
static void* workerQueueEngine(void *param){
    ADM_WORKER_QUEUE_T *wq = (ADM_WORKER_QUEUE_T *)param;
    ADM_WORKER_QUEUE_TASK_T task;
    while(DTRUE){
        ADM_WORKER_QUEUE_TASK_T *t = NULL;

        (void)pthread_mutex_lock(&wq->mutex);
        if (wq->items == 0U) {
            if (wq->quit == DFALSE) {
                (void)pthread_cond_wait(&wq->cond, &wq->mutex);
            }
            else {
                (void)pthread_mutex_unlock(&wq->mutex);
                break;
            }
        }
        else {
            task.param = wq->head[wq->read].param;
            task.fnEngine = wq->fnEngine;
            t = &task;
            wq->items--;
            wq->read++;
            if (wq->read == wq->maxItems) {
                wq->read = 0U;
            }
        }
        (void)pthread_mutex_unlock(&wq->mutex);

        if (NULL != t) {
            if (NULL != t->fnEngine) {
                t->fnEngine(t->param);
            }
        }
    }
    return NULL;
}
