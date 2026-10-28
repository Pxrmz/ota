#include "adm_queue.h"
#include <stdio.h>
#include <stdlib.h>

int main()
{
    ADM_QUEUE_T* queue = adm_queue_new();
    if (!queue) {
        printf("queue is NULL\n");
        return 0;
    }
    if (!adm_queue_push(queue, "test")) {
        printf("adm_queue_push is failed\n");
    }

    if (!adm_queue_push(queue, "avbcd")) {
        printf("adm_queue_push is failed\n");
    }

    if (!adm_queue_push(queue, "asfd")) {
        printf("adm_queue_push is failed\n");
    }

    printf("queue size is = %d\n", adm_queue_size(queue));
    printf("queue isEmpty is = %d\n", adm_queue_isEmpty(queue));

    char* data = NULL;
    int dataLength = 0;

    if (!adm_queue_pop(queue, &data, &dataLength)) {
        printf("adm_queue_pop is failed\n");
    } else {
        printf("data = %s\n", data);
        printf("dataLength = %d\n", dataLength);
        free(data);
        data = NULL;
    }

    if (!adm_queue_pop(queue, &data, &dataLength)) {
        printf("adm_queue_pop is failed\n");
    } else {
        printf("data = %s\n", data);
        printf("dataLength = %d\n", dataLength);
        free(data);
        data = NULL;
    }

    if (!adm_queue_pop(queue, &data, &dataLength)) {
        printf("adm_queue_pop is failed\n");
    } else {
        printf("data = %s\n", data);
        printf("dataLength = %d\n", dataLength);
        free(data);
        data = NULL;
    }

    if (!adm_queue_pop(queue, &data, &dataLength)) {
        printf("adm_queue_pop is failed\n");
    } else {
        printf("data = %s\n", data);
        printf("dataLength = %d\n", dataLength);
        free(data);
        data = NULL;
    }

    if(!adm_queue_clear(queue)){
      printf("adm_queue_clear is failed\n");  
    }

    printf("queue size is = %d\n", adm_queue_size(queue));
    printf("queue isEmpty is = %d\n", adm_queue_isEmpty(queue));

    adm_queue_free(queue);
    return 0;
}