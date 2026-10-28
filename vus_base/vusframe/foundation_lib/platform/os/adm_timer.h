#ifndef _ADM_TIMER_H_
#define _ADM_TIMER_H_

#include "time.h"
#include "signal.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*ADM_TIMER_HANDLER_T)( union sigval v );

typedef struct
{
    struct sigevent timer_event;
    timer_t         timer_id;
    pid_t           pid;
    ADM_TIMER_HANDLER_T callback_func;
    D32U interval_ms;
    void*           arg;
}ADM_TIMER_T;


D32S adm_timer_init( ADM_TIMER_T* timer, D32U time_ms, ADM_TIMER_HANDLER_T callback_func, void* arg );
D32S adm_timer_start( ADM_TIMER_T* timer );
D32S adm_timer_stop( ADM_TIMER_T* timer );
D32S adm_timer_destroy( ADM_TIMER_T* timer );

#ifdef __cplusplus
}
#endif


#endif /* _ADM_TIMER_H_ */
