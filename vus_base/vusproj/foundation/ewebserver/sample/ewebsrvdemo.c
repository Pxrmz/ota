#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>

#include "ab_ewebsrv_api.h"

static volatile int g_exit_flag = 0; /* 0 = continue running main loop */

// console app to deal SIGTERM/SIGINT
static void signal_handler(int sig_num) {
    abEWebSrv_LogOut(AB_EWEBSRV_LOG_INFO, "Ctl-C : %d ", sig_num);
    abEWebSrv_Stop();
    g_exit_flag = 1;
}

int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;
    float ver = (float) abEWebSrv_GetVersionCode();
    abEWebSrv_SetLog(AB_EWEBSRV_LOG_DEBUG, "./log.txt",
                     true,
                     true,
                     false);
    abEWebSrv_LogOut(AB_EWEBSRV_LOG_INFO, "Run webserver, sdk ver:%2.2f, logfile:log.txt", ver/100);

    /* Setup signal handler: quit on Ctrl-C */
    signal(SIGTERM, signal_handler);
    signal(SIGINT, signal_handler);

    abEWebSrv_CfgSetDocumentDir(".", "./upload");
    abEWebSrv_CfgSetHttpPort(true, 8081);
    abEWebSrv_CfgSetSSLCertificateFile("./server.pem");
    abEWebSrv_CfgSetFile("./config.txt");

    abEWebSrv_Start();
    while (g_exit_flag == 0) {
        sleep(1);
        if (abEWebSrv_GetState() != AB_EWEBSRV_STATE_RUNNING)
            break;
        // signal_handler(1);
    }
    abEWebSrv_LogOut(AB_EWEBSRV_LOG_INFO, "Exit webserver: %d", abEWebSrv_GetState());
    while (abEWebSrv_GetState() != AB_EWEBSRV_STATE_CLOSED){
        sleep(1);
        abEWebSrv_LogOut(AB_EWEBSRV_LOG_INFO, ".");
    }
    return 0;
}

