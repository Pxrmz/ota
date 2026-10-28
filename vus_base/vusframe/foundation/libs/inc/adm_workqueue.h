#ifndef __ADM_WORKQUEUE_H_
#define	__ADM_WORKQUEUE_H_

#include "adm_typedefs.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ADM_LIB_Workqueue_ ADM_LIB_Workqueue_T;
extern ADM_LIB_Workqueue_T *adm_WorkqueueNew(D32S maxThreadCount,
        void (*engine)(void *param));
extern D32S adm_WorkqueueAdd(ADM_LIB_Workqueue_T *wq, void *param);
extern D32S adm_WorkqueueDel(ADM_LIB_Workqueue_T *wq);

#ifdef __cplusplus
}
#endif

#endif//__ADM_WORKQUEUE_H_
