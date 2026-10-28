#ifndef __KREF_H
#define __KREF_H

#include <pthread.h>
#include "atomic.h"

struct kref {
    atomic_t refcount;
};

void kref_init(struct kref *kref);
void kref_get(struct kref *kref);
int kref_sub(struct kref *kref, unsigned int count,
             void (*release)(struct kref *kref));
int kref_put(struct kref *kref, void (*release)(struct kref *kref));
int kref_put_mutex(struct kref *kref,
                   void (*release)(struct kref *kref),
                   pthread_mutex_t *mutex);
int kref_get_unless_zero(struct kref *kref);


#endif /* _KREF_H_ */
