/***************************************************************************
 *
 * Copyright (c) 2016 zkdnfcf, Inc. All Rights Reserved
 * $Id$
 *
 **************************************************************************/

#ifndef _QUEUE_H
#define _QUEUE_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct adm_queue_ ADM_QUEUE_T;

/*new and delete*/
ADM_QUEUE_T* adm_queue_new();
/*仅仅清空所有元素*/
bool adm_queue_clear(ADM_QUEUE_T* queue);
void adm_queue_free(ADM_QUEUE_T* queue);

/*get size*/
int adm_queue_size(const ADM_QUEUE_T* queue);
int adm_queue_isEmpty(const ADM_QUEUE_T* queue);

/*push and pop*/
bool adm_queue_push(ADM_QUEUE_T* queue, void* data, unsigned int dataSize);
bool adm_queue_pop(ADM_QUEUE_T* queue, void** data, unsigned int* dataSize);

#ifdef __cplusplus
}
#endif
#endif  //_MAP_H
