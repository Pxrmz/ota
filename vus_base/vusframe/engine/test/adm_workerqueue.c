#include "adm_workerqueue.h"
#include "adm_debug.h"
#include "adm_com_lib.h"

#include <stdio.h>
#include <unistd.h>

static void workerEngine_a(void *param){
    char *s = (char*)param;
    printf("%s\n", s);
    usleep(100);
}
static void workerEngine_b(void *param){
    char *s = (char*)param;
    printf("%s\n", s);
    sleep(1);
}
static void workerEngine_c(void *param){
    usleep(1000);
    char *s = (char*)param;
    printf("%s\n", s);
}
static void workerEngine_d(void *param){
}

int main(){
    adm_com_createDir("/tmp/debug");
    adm_debug_logInit("/tmp/debug/", (1 * 1024 * 1024), 5);

    ADM_WORKER_QUEUE_T* wq;

#if 1
    wq = adm_workerQueueNew(workerEngine_a);
    adm_workerQueueAdd(wq, (void*)"hello world");
    adm_workerQueueAdd(wq, (void*)"hello world");
    sleep(1);
    adm_workerQueueAdd(wq, (void*)"hello world");
    adm_workerQueueQuit(wq);
    adm_workerQueueAdd(wq, (void*)"hello world");
    adm_workerQueueDel(wq);
#endif

#if 1
    wq = adm_workerQueueNew(workerEngine_b);
    adm_workerQueueAdd(wq, (void*)"hello world");
    adm_workerQueueAdd(wq, (void*)"hello world");
    adm_workerQueueAdd(wq, (void*)"hello world");
    adm_workerQueueAdd(wq, (void*)"hello world");
    adm_workerQueueAdd(wq, (void*)"hello world");
    adm_workerQueueAdd(wq, (void*)"hello world");
    adm_workerQueueQuit(wq);
    adm_workerQueueDel(wq);
#endif

#if 1
    wq = adm_workerQueueNew(workerEngine_c);
    for(int i=0; i<1002; ++i){
	char s[10020][6];
	sprintf(s[i], "%d", i);
    adm_workerQueueAdd(wq, (void*)s[i]);
    }
    adm_workerQueueQuit(wq);
    adm_workerQueueDel(wq);
#endif

#if 1
    wq = adm_workerQueueNew(workerEngine_d);
    for(int i=0; i<10020; ++i){
    usleep(100);
    adm_workerQueueAdd(wq, (void*)"hello world");
    adm_workerQueueAdd(wq, (void*)"hello world");
    adm_workerQueueAdd(wq, (void*)"hello world");
    adm_workerQueueAdd(wq, (void*)"hello world");
    }
    adm_workerQueueQuit(wq);
    adm_workerQueueDel(wq);
#endif

    return 0;
}
