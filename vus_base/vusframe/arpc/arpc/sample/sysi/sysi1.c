#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <ctype.h>

#include <signal.h>

#include "adm_com_errorCode.h"
#include "adm_typedefs.h"

#include "adm_memory.h"

#include "adm_arpc_log.h"
#include "adm_string.h"
#include "adm_cfg_api.h"
#include "adm_com_lib.h"

#include "adm_arpc_service.h"
#include "adm_arpc_internal.h"

#define COMMAND_END_LIST {NULL, NULL, NULL, NULL, NULL}
#define HELP_COMMAND "help"
#define HELP_DESC    "Type help for more details on a command."
#define UNKNOWN_CMD_MSG "Unknown command."

/*
// server info:  instance 0x02, 0x03
// method	
	0x02, 0x02, 0x03
	0x03, 0x02, 0x03
// eventid
	0x02, 0x02, 0x10
	0x02, 0x02, 0x20
	0x03, 0x02, 0x10
*/

static D32S s_break = 0;

static void handle_signal(int _signal)
{
	if (_signal == SIGINT || _signal == SIGTERM) {
        printf("\n CTL-C Break \n");
        s_break = 1;
	}
}

D32S reqHandler2(DCHAR* reqData, D32U reqDataSize, DCHAR** respData, D32U* respDataSize)
{
	log_e("app begin reqHandler2");

	//sleep(7);

	log_e("app recv req data: %s, size: %d", reqData, reqDataSize);

	// 嵌套 请求1
	char* resp = NULL;
	int respSize = 0;
	adm_arpc_request_service(0x101, 0x01, 0x01, 5, "req1,1,1", 9, &resp, &respSize);
	log_e("app recv resp, data: %s, size: %d", resp, respSize);
	

	*respData = adm_strdup("12345_0x2");
	*respDataSize = 10;

	log_e("app end reqHandler2");

	return 0;
}

D32S reqHandler3(DCHAR* reqData, D32U reqDataSize, DCHAR** respData, D32U* respDataSize)
{
	log_e("app begin reqHandler3");

	log_e("app recv req data: %s, size: %d", reqData, reqDataSize);

	*respData = adm_strdup("12345_0x3");
	*respDataSize = 10;

	log_e("app end reqHandler3");

	return 0;
}


void usage()
{
    printf("Usage: admfotaapp [options]...  [param]... \n");
    printf("Options:\n");
    printf("  -c  filename          fota config absolute\n");
}

typedef void (*command_handler_t) (char * args, void * user_data);

typedef struct
{
    char *            name;
    char *            shortDesc;
    char *            longDesc;
    command_handler_t callback;
    void *            userData;
} command_desc_t;


void notifyEvent2(char * buffer,void * user_data)
{
	adm_arpc_checkIfClientSubedEvent(0x102, 0x02, 0x10, 0x01);
	
	adm_arpc_notifyEvent(0x102, 0x02, 0x10, "10notify1234_0x2", 17);

	adm_arpc_notifyEvent(0x102, 0x02, 0x20, "20notify1234_0x2", 17);
}

void notifyEvent3(char * buffer,void * user_data)
{
	adm_arpc_notifyEvent(0x103, 0x02, 0x10, "10notify1234_0x3", 17);

}

void notifyEvent4(char * buffer,void * user_data)
{
    DCHAR *buff, *p;
    //数据中需要带上长度
    D32S offset = sizeof(D32S);
    D32S length = atoi(buffer) + offset;
    buff = (DCHAR *)adm_malloc(length);

    adm_snprintf(buff, length, "%d", length);
    p = buff + adm_strlen(buff);
    adm_memset(p, 'a', length - adm_strlen(buff), length - adm_strlen(buff));
	adm_arpc_notifyEvent(0x106, 0x02, 0x10, buff, length);
    adm_free(buff);
}

void sendheart(char * buffer,void * user_data)
{
	adm_arpc_sendHeartByInstanceId(0x01);
}

static command_desc_t * prv_find_command(command_desc_t * commandArray,
                                         char * buffer,
                                         size_t length)
{
    int i;

    if (length == 0) return NULL;

    i = 0;
    while (commandArray[i].name != NULL
        && (adm_strlen(commandArray[i].name) != length || adm_strncmp(buffer, commandArray[i].name, length)))
    {
        i++;
    }

    if (commandArray[i].name == NULL)
    {
        return NULL;
    }
    else
    {
        return &commandArray[i];
    }
}

static void prv_displayHelp(command_desc_t * commandArray,
                            char * buffer)
{
    command_desc_t * cmdP;
    int length;

    // find end of first argument
    length = 0;
    while (buffer[length] != 0 && !isspace((int)buffer[length]&0xff))
        length++;

    cmdP = prv_find_command(commandArray, buffer, length);

    if (cmdP == NULL)
    {
        int i;

        for (i = 0 ; commandArray[i].name != NULL ; i++)
        {
            fprintf(stdout, "%-50s%-50s\r\n", commandArray[i].name, commandArray[i].shortDesc);
        }
    }
    else
    {
        fprintf(stdout, "%s\r\n", cmdP->longDesc?cmdP->longDesc:cmdP->shortDesc);
    }
}

void handle_command(command_desc_t * commandArray,
                    char * buffer)
{
    command_desc_t * cmdP;
    int length;

    // find end of command name
    length = 0;
    while (buffer[length] != 0 && !isspace((int)buffer[length]&0xFF))
        length++;

    cmdP = prv_find_command(commandArray, buffer, length);
    if (cmdP != NULL)
    {
        while (buffer[length] != 0 && isspace((int)buffer[length]&0xFF))
            length++;
        cmdP->callback(buffer + length, cmdP->userData);
    }
    else
    {
        if (!adm_strncmp(buffer, HELP_COMMAND, length))
        {
            while (buffer[length] != 0 && isspace((int)buffer[length]&0xFF))
                length++;
            prv_displayHelp(commandArray, buffer + length);
        }
        else
        {
            fprintf(stdout, UNKNOWN_CMD_MSG"\r\n");
        }
        
    }
}


int test()
{
    command_desc_t commands[] =
    {
        {"s","sendheart", NULL, sendheart, NULL},
        {"n2","notify", NULL, notifyEvent2, NULL},
      	{"n3","notify", NULL, notifyEvent3, NULL},
        {"n4","notify specified length data", NULL, notifyEvent4, NULL},
      	
        COMMAND_END_LIST
    };

    fd_set readfds;
    struct timeval tv;
    int result;
    while(!s_break)
    {
        FD_ZERO(&readfds);
        FD_SET(STDIN_FILENO, &readfds);
        tv.tv_sec = 60;
        tv.tv_usec = 0;

        result = select(FD_SETSIZE, &readfds, 0, 0, &tv);
        if( result < 0 )
        {
            if(errno != EINTR)
            {
              fprintf(stderr, "Error in select(): %d\r\n", errno);
            }
        }
        else if(result > 0)
        {
            if(FD_ISSET(STDIN_FILENO, &readfds))
            {
                int buffer[1024]={0};
                int numBytes;
                numBytes = read(STDIN_FILENO, buffer, 1024 - 1);

                if (numBytes > 1)
                {
                    buffer[numBytes] = 0;
                    handle_command(commands, (char*)buffer);
                    fprintf(stdout, "\r\n");
                }
                if (s_break == 0)
                {
                    fprintf(stdout, "\r\n");
                    fflush(stdout);
                }
                else
                {
                    fprintf(stdout, "\r\n");
                }
            }
        }   
    }
    return 0;
}


int main(int argc,char* argv[])
{
	//signal(SIGINT, handle_signal);
    //signal(SIGTERM, handle_signal);	

	adm_debug_logInit("/tmp/arpc/sysi1/", 1*1024*1024, 5);

	D32U loopCount = 0;

	DCHAR* otaConfigPath = NULL;
	D32S c = 0;

	//!< 接收入参
    while ((c = getopt (argc, argv, "c:")) != -1)
    {
        switch (c)
        {
            case 'c' :
                otaConfigPath = optarg;
                printf("optarg:%s\n", optarg);
                break;
            case '?':
            {
                if (optopt == 'o')
                    fprintf (stderr, "Option -%c requires an argument.\n", optopt);
                else if (isprint ((int)optopt))
                    fprintf (stderr, "Unknown option `-%c'.\n", optopt);
                else
                    fprintf (stderr, "Unknown option character `\\x%x'.\n", optopt);

                /* unknown or wrong option used, show usage information and terminate */
                usage();
                return -1;
            }
            default:
            {
                abort ();
                return -1;    /*for parasoft */
            }
        }
    }

    ADM_CFG_API_INFO_T pstApiData = {0};
    pstApiData.isFileExistCb = adm_com_isFileExist;
    pstApiData.writeFileDataCb = adm_com_writeFileData;
    D32S iRet = adm_cfg_iniParserInit(otaConfigPath, &pstApiData);
    if (ADM_ECODE_SUCCESSED != iRet) 
    {
        log_e("adm_cfg_iniParserInit %s failed, iRet:%d\n", otaConfigPath, iRet);
        return 0;
    }


	D16U instanceId = 0;
	if(ADM_ECODE_SUCCESSED != adm_arpc_initService(&instanceId))
	{
		return 0;
	}

	if(ADM_ECODE_SUCCESSED != adm_arpc_startService(NULL))
	{
		return 0;
	}

	
    adm_arpc_registerServMethodHandler(0x102, 0x02, 0x03, reqHandler2);

    adm_arpc_registerServMethodHandler(0x103, 0x02, 0x03, reqHandler3);

    log_i("***************************");

    adm_arpc_offerEvent(0x102, 0x02, 0x10);

    adm_arpc_offerEvent(0x102, 0x02, 0x20);

    adm_arpc_offerEvent(0x103, 0x02, 0x10);

    adm_arpc_offerEvent(0x106, 0x02, 0x10);

    adm_arpc_checkIfClientSubedEvent(0x102, 0x02, 0x10, 0x01);

    test();

#if 0
    sleep(90);

	log_i("***********notify****************");
    adm_arpc_notifyEvent(0x02, 0x10, "10notify1234", 11);

	
	for(int i=0; i<3; i++)
	{
    	adm_arpc_notifyEvent(0x02, 0x10, "10notify1234", 11);
    	//sleep(1);
	}

	sleep(3);
	adm_arpc_notifyEvent(0x02, 0x20, "20notify1234", 11);

	// test client reconnect
	//sleep(90);

	//log_i("test client reconnect");
	
	// test big data, buffer 100
	DCHAR* buf[1000] = {0};
	adm_memset(buf, 'a', 999, 999);
	adm_arpc_notifyEvent(0x02, 0x10, buf, 1000);



    while(/*loopCount<100 && */ !s_break)
    {
       	sleep(1);
		loopCount++;
    }
#endif

    adm_arpc_stopService();

    adm_debug_logDeinit();

	return 0;
}

