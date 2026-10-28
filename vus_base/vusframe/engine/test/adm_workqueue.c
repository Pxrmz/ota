#include "adm_workqueue.h"
#include "adm_debug.h"
#include "adm_com_lib.h"

#include <stdio.h>
#include <unistd.h>

static void workerEngine_a(void *param){
    char *s = (char*)param;
    printf("%s\n", s);
    usleep(100);
}


int main(){
    adm_com_createDir("/tmp/debug");
    adm_debug_logInit("/tmp/debug/", (1 * 1024 * 1024), 5);

    ADM_LIB_Workqueue_T *w;
    w = adm_WorkqueueNew(1, workerEngine_a);
    adm_WorkqueueAdd(w,(void*)"hello world");
    adm_WorkqueueAdd(w,(void*)"hello world");
    sleep(1);
    adm_WorkqueueAdd(w,(void*)"hello world");
    adm_WorkqueueDel(w);

    return 0;
}
