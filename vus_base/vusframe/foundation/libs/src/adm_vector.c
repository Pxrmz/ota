#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <pthread.h>
#include "adm_vector.h"

struct ADM_VECTOR_
{
    pthread_mutex_t vectorMutex;
    int size;
    char** data;
};

ADM_VECTOR_T* adm_vector_new()
{
    ADM_VECTOR_T* vec = NULL;
    int status;
    vec = (ADM_VECTOR_T*)malloc(sizeof(ADM_VECTOR_T));
    if (vec) {
        status = pthread_mutex_init(&vec->vectorMutex, NULL);
        if (status != 0) {
            free(vec);
            vec = NULL;
            return NULL;
        }
        vec->size = 0;
        vec->data = NULL;
    }
    return vec;
}

void adm_vector_delete(ADM_VECTOR_T* vec)
{
    if (!vec)
        return;
    for (int i = 0; i < vec->size; ++i) {
        if (vec->data[i]) {
            free(vec->data[i]);
            vec->data[i] = NULL;
        }
    }

    pthread_mutex_destroy(&vec->vectorMutex);

    if (vec->data) {
        free(vec->data);
        vec->data = NULL;
    }

    free(vec);
    vec = NULL;
}

int adm_vector_get_size(const ADM_VECTOR_T* vec)
{
    if (vec) {
        return vec->size;
    }
    return 0;
}

int adm_vector_pushback(ADM_VECTOR_T* vec, const char* str)
{
    if (!str || !vec)
        return -1;
    int status;
    status = pthread_mutex_lock(&vec->vectorMutex);
    if (status != 0) {
        return status;
    }
    char** tmp = NULL;
    // realloc add 1
    tmp = (char**)malloc(sizeof(char*) * (vec->size + 1));
    if (!tmp) {
        status = pthread_mutex_unlock(&vec->vectorMutex);
        if (status != 0) {
            return status;
        }
        return -1;
    }

    memset(tmp, 0x00, sizeof(char*) * (vec->size + 1));

    // copy data
    for (int i = 0; i < vec->size; ++i) {
        tmp[i] = strdup(vec->data[i]);
    }
    // add new data
    tmp[vec->size] = strdup(str);

    // release original data
    for (int i = 0; i < vec->size; ++i) {
        if (vec->data[i]) {
            free(vec->data[i]);
            vec->data[i] = NULL;
        }
    }
    if (vec->data) {
        free(vec->data);
        vec->data = NULL;
    }
    vec->data = tmp;
    vec->size += 1;

    status = pthread_mutex_unlock(&vec->vectorMutex);
    if (status != 0) {
        return status;
    }
    return 0;
}

char* adm_iterator_get_value(const ADM_VECTOR_T* vec, const int iterator)
{
    if (!vec || !vec->data || iterator >= vec->size)
        return NULL;

    return vec->data[iterator];
}

int adm_vector_popback(ADM_VECTOR_T* vec)
{
    if (!vec || vec->size <= 0)
        return -1;
    int status;
    status = pthread_mutex_lock(&vec->vectorMutex);
    if (status != 0) {
        return status;
    }
    if (vec->data[vec->size - 1]) {
        free(vec->data[vec->size - 1]);
        vec->data[vec->size - 1] = NULL;
    }
    vec->size -= 1;
    status = pthread_mutex_unlock(&vec->vectorMutex);
    if (status != 0) {
        return status;
    }
    return 0;
}

int adm_vector_clear(ADM_VECTOR_T* vec)
{
    if (!vec)
        return -1;
    int status;
    status = pthread_mutex_lock(&vec->vectorMutex);
    if (status != 0) {
        return status;
    }
    for (int i = 0; i < vec->size; ++i) {
        if (vec->data[i]) {
            free(vec->data[i]);
            vec->data[i] = NULL;
        }
    }

    if (vec->data) {
        free(vec->data);
        vec->data = NULL;
    }
    vec->size = 0;

    status = pthread_mutex_unlock(&vec->vectorMutex);
    if (status != 0) {
        return status;
    }
    return 0;
}

int adm_vector_erase(ADM_VECTOR_T* vec, const int index)
{
    if (!vec || index > vec->size - 1)
        return -1;
    int status;
    status = pthread_mutex_lock(&vec->vectorMutex);
    if (status != 0) {
        return status;
    }

    for (int i = index; i < vec->size; ++i) {
        if (vec->data[i]) {
            free(vec->data[i]);
            vec->data[i] = NULL;
        }
        if (i == vec->size - 1) {
            break;
        } else {
            vec->data[i] = strdup(vec->data[i + 1]);
        }
    }
    vec->size -= 1;

    status = pthread_mutex_unlock(&vec->vectorMutex);
    if (status != 0) {
        return status;
    }
    return 0;
}