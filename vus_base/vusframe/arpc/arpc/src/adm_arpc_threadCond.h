#ifndef ADM_ARPC_THREAD_COND_H
#define ADM_ARPC_THREAD_COND_H

#include <pthread.h>
#include "adm_typedefs.h"

D32S adm_arpc_pthread_cond_timedwait(pthread_cond_t *pstCond, pthread_mutex_t *pstMutex,
                                     DBOOL *pIsSignal, const struct timespec *pstAbstime);

#endif // ADM_ARPC_THREAD_COND_H