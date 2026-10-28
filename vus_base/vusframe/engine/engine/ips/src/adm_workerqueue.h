#ifndef __ADM_WORKER_QUEUE_H_
#define __ADM_WORKER_QUEUE_H_

#include "adm_typedefs.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void (ADM_WORKER_QUEUE_ENGINE_F)(void *param);
typedef struct ADM_WORKER_QUEUE_ ADM_WORKER_QUEUE_T;
ADM_WORKER_QUEUE_T *adm_workerQueueNew(ADM_WORKER_QUEUE_ENGINE_F *fnEngine);
void adm_workerQueueDel(ADM_WORKER_QUEUE_T *wq);
void adm_workerQueueQuit(ADM_WORKER_QUEUE_T *wq);
void adm_workerQueueAdd(ADM_WORKER_QUEUE_T *wq, void *param);

#ifdef __cplusplus
}
#endif

#endif//__ADM_WORKER_QUEUE_H_
