#include "adm_worker.h"
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
    usleep(100);
}
static void workerEngine_d(void *param){
}

int main(){
    adm_com_createDir("/tmp/debug");
    adm_debug_logInit("/tmp/debug/", (1 * 1024 * 1024), 5);

    ADM_WORKER_T* w;
    w = adm_workerNew(1, 1000, workerEngine_a);
    adm_workerAdd(w, (void*)"hello world");
    adm_workerAdd(w, (void*)"hello world");
    sleep(1);
    adm_workerAdd(w, (void*)"hello world");
    adm_workerDel(w);

#if 1
    w = adm_workerNew(3, 1000, workerEngine_b);
    adm_workerAdd(w, (void*)"hello world");
    adm_workerAdd(w, (void*)"hello world");
    adm_workerAdd(w, (void*)"hello world");
    adm_workerAdd(w, (void*)"hello world");
    adm_workerAdd(w, (void*)"hello world");
    adm_workerAdd(w, (void*)"hello world");
    adm_workerDel(w);

    w = adm_workerNew(3, 1000, workerEngine_c);
    for(int i=0; i<1002; ++i){
    adm_workerAdd(w, (void*)"hello world");
    }
    adm_workerDel(w);

    w = adm_workerNew(3, 1000, workerEngine_d);
    for(int i=0; i<10020; ++i){
    usleep(100);
    adm_workerAdd(w, (void*)"hello world");
    adm_workerAdd(w, (void*)"hello world");
    adm_workerAdd(w, (void*)"hello world");
    adm_workerAdd(w, (void*)"hello world");
    }
    adm_workerDel(w);
#endif

    return 0;
}
