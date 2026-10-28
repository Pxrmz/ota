#ifndef __ATOMIC_H
#define __ATOMIC_H

#include <pthread.h>

typedef struct {
    volatile int counter;
    pthread_mutex_t mutex;
} atomic_t;

void atomic_init(atomic_t *v);
void atomic_deinit(atomic_t *v);
void atomic_set(atomic_t *v, int i);
int atomic_read(atomic_t *v);
int atomic_inc_return(atomic_t *v);
int atomic_sub_and_test(int i, atomic_t *v);
int atomic_dec_and_test(atomic_t *v);
int atomic_add_unless(atomic_t *v, int a, int u);

#endif
