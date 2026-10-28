#ifndef __ADM_WORKER_H_
#define	__ADM_WORKER_H_

#include "adm_typedefs.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void (ADM_WORKER_ENGINE_F)(void *param);
typedef struct ADM_WORKER_ ADM_WORKER_T;
ADM_WORKER_T* adm_workerNew(D32S maxThreads, D32S maxItems, ADM_WORKER_ENGINE_F *fnEngine);
void adm_workerDel(ADM_WORKER_T *w);
void adm_workerAdd(ADM_WORKER_T *w, void *param);

#ifdef __cplusplus
}
#endif

#endif//__ADM_WORKER_H_
