#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include "adm_queue.h"

typedef struct adm_queue_linkList_
{
    void* data;
    unsigned int   dataSize;
    struct adm_queue_linkList_* next;
} ADM_QUEUE_LINK_LIST_T;

typedef struct adm_queue_
{
    struct adm_queue_linkList_* front;
    struct adm_queue_linkList_* rear;
    unsigned int size;
    pthread_mutex_t queueMutex;
} ADM_QUEUE_T;

ADM_QUEUE_T* adm_queue_new()
{
    ADM_QUEUE_T* queue = NULL;
    int status;

    queue = (ADM_QUEUE_T*)malloc(sizeof(ADM_QUEUE_T));
    if (queue) {
        status = pthread_mutex_init(&queue->queueMutex, NULL);
        if (status != 0) {
            free(queue);
            queue = NULL;
            return NULL;
        }
        queue->size = 0;
        queue->front = NULL;
        queue->rear = NULL;
    }
    return queue;
}

void adm_queue_free(ADM_QUEUE_T* queue)
{
    if (!queue)
        return;
    struct adm_queue_linkList_* tmp = NULL;
    while (queue->front) {
        tmp = queue->front;
        queue->front = queue->front->next;
        if (tmp->data) {
            free(tmp->data);
            tmp->data = NULL;
        }
        free(tmp);
        tmp = NULL;
    }
    pthread_mutex_destroy(&queue->queueMutex);
    free(queue);
    queue = NULL;
}

bool adm_queue_clear(ADM_QUEUE_T* queue)
{
    if (!queue)
        return false;
    struct adm_queue_linkList_* tmp = NULL;

    int status;
    status = pthread_mutex_lock(&queue->queueMutex);
    if (status != 0) {
        return false;
    }

    while (queue->front) {
        tmp = queue->front;
        queue->front = queue->front->next;
        if (tmp->data) {
            free(tmp->data);
            tmp->data = NULL;
        }
        free(tmp);
        tmp = NULL;
    }
    queue->front = NULL;
    queue->rear = NULL;
    queue->size = 0;

    status = pthread_mutex_unlock(&queue->queueMutex);
    if (status != 0) {
        return false;
    }
    return true;
}

int adm_queue_size(const ADM_QUEUE_T* queue)
{
    if (!queue)
        return 0;

    int status = 0;
    status = pthread_mutex_lock(&queue->queueMutex);
    if (status != 0) {
        return -1;
    }

    int queueSize = queue->size;

    status = pthread_mutex_unlock(&queue->queueMutex);
    if (status != 0) {
        return -1;
    }
    
    return queueSize;
}

int adm_queue_isEmpty(const ADM_QUEUE_T* queue)
{
    if (!queue)
        return 0;

	int status = 0;
    status = pthread_mutex_lock(&queue->queueMutex);
    if (status != 0) {
        return -1;
    }

    int queueSize = queue->size;

    status = pthread_mutex_unlock(&queue->queueMutex);
    if (status != 0) {
        return -1;
    }
    
    if (queueSize> 0)
        return 0;
    else
        return 1;
}

bool adm_queue_push(ADM_QUEUE_T* queue, void* data, unsigned int dataSize)
{
    if (!queue || !data)
        return false;

    struct adm_queue_linkList_* node = NULL;
    int retVal = 0;
    int status;
    status = pthread_mutex_lock(&queue->queueMutex);
    if (status != 0) {
        return false;
    }

    node = (struct adm_queue_linkList_*)malloc(sizeof(struct adm_queue_linkList_));

    do {
        if (NULL == node) {
            retVal = -1;
            break;
        }

		node->data = malloc(dataSize);
		memset(node->data, 0x0, dataSize);
		memcpy(node->data, data, dataSize);

		node->dataSize = dataSize;
        //node->data = strdup(data);
        node->next = NULL;

        if (0 == queue->size) {
            queue->front = node;
            queue->rear = node;
        } else {
            queue->rear->next = node;
            queue->rear = node;
        }
        ++queue->size;
    } while (0);

    status = pthread_mutex_unlock(&queue->queueMutex);
    if (status != 0) {
        return false;
    }
    if (-1 == retVal) {
        return false;
    }
    return true;
}

bool adm_queue_pop(ADM_QUEUE_T* queue, void** data, unsigned int* dataSize)
{
    if (!queue)
        return false;
    if (adm_queue_isEmpty(queue)) {
        return false;
    }
    struct adm_queue_linkList_* tmp = NULL;
    int status;
    status = pthread_mutex_lock(&queue->queueMutex);
    if (status != 0) {
        return false;
    }

    tmp = queue->front;

    *data = malloc(tmp->dataSize);
    memset(*data, 0x0, tmp->dataSize);
    memcpy(*data, tmp->data, tmp->dataSize);

    *dataSize = tmp->dataSize;

	/*
    *data = strdup(queue->front->data);
    *dataLength = strlen(*data);
    */
    
    queue->front = queue->front->next;


    if (tmp->data) {
        free(tmp->data);
        tmp->data = NULL;
    }

    free(tmp);
    tmp = NULL;
    --queue->size;

    status = pthread_mutex_unlock(&queue->queueMutex);
    if (status != 0) {
        return false;
    }
    return true;
}