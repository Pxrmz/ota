#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>

#include "ab_ewebsrv_api.h"

int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;
    float ver = (float) abEWebSrv_GetVersionCode();
    abEWebSrv_SetLog(AB_EWEBSRV_LOG_DEBUG, "./log.txt",
                     true,
                     true,
                     false);
    abEWebSrv_LogOut(AB_EWEBSRV_LOG_INFO, "Run webserver demo, sdk ver:%2.2f", ver/100);
    abEWebSrv_CfgSetFile("./config.txt");

    int ret = abEWebSrv_Start();

    for (int i =0; ret == 0 && i<10; i++) {
        printf(".");
        sleep(1);
    }
    abEWebSrv_LogOut(AB_EWEBSRV_LOG_INFO, "Exit webserver demo : %d", abEWebSrv_GetState());
    while (abEWebSrv_GetState() != AB_EWEBSRV_STATE_CLOSED){
        sleep(1);
        abEWebSrv_LogOut(AB_EWEBSRV_LOG_INFO, ".");
    }
    return 0;
}

