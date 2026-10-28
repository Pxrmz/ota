#include "adm_typedefs.h"
#include "adm_debug.h"
#include "adm_com_lib.h"

#include <unistd.h>
#include <stdio.h>

void adm_apc_contextInit();
void adm_apc_contextDeInit();
void adm_apc_contextStart();
void adm_apc_contextStop();

int main(){
    adm_com_createDir("/tmp/debug");
    adm_debug_logInit("/tmp/debug/", (1 * 1024 * 1024), 5);

    adm_apc_contextInit();
    adm_apc_contextStart();
    while(1){
	if('q' == getchar()){
	    break;
	}else{
	    log_d("enter q to exit");
	}
    }
    adm_apc_contextStop();
    adm_apc_contextDeInit();
    return 0;
}
