#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>

#include <signal.h>
#include <ctype.h>

#include "adm_com_errorCode.h"
#include "adm_typedefs.h"
#include "adm_memory.h"
#include "adm_string.h"

#include "adm_arpc_log.h"
#include "adm_arpc_service.h"
#include "adm_cfg_api.h"
#include "adm_com_lib.h"

#define COMMAND_END_LIST {NULL, NULL, NULL, NULL, NULL}
#define HELP_COMMAND "help"
#define HELP_DESC    "Type help for more details on a command."
#define UNKNOWN_CMD_MSG "Unknown command."
/*
	dim1 server info : instance 0x01
	method
	0x101, 0x01, 0x01
*/


/*
// sysi1 server info:  instance 0x02, 0x03
// method	
	0x102, 0x02, 0x03
	0x103, 0x02, 0x03
// eventid
	0x102, 0x02, 0x10
	0x102, 0x02, 0x20
	0x103, 0x02, 0x10
*/

/*
// sysi2 server info:  instance 0x04, 0x05
// method	
	0x104, 0x02, 0x03
	0x105, 0x02, 0x03
// eventid
	0x104, 0x02, 0x10
	0x104, 0x02, 0x20
	0x105, 0x02, 0x10
*/


static D32S s_break = 0;

static void handle_signal(int _signal)
{
	if (_signal == SIGINT || _signal == SIGTERM) {
        printf("\n CTL-C Break \n");
        s_break = 1;
	}
}

void sysiOnlinehandler(void* priv)
{
	log_w("apphandler,  0X%04X is online", *(D16U*)priv);
}


void sysiOfflinehandler(void* priv)
{
	log_w("apphandler,	0X%04X is offline", *(D16U*)priv);
}


D32S reqHandler(DCHAR* reqData, D32U reqDataSize, DCHAR** respData, D32U* respDataSize)
{
	log_e("app begin reqHandler, 0x02");

	log_e("app recv req data: %s, size: %d", reqData, reqDataSize);

	*respData = adm_strdup("12345");
	*respDataSize = 6;

	log_e("app end reqHandler");

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

D32S reqHandler1(DCHAR* reqData, D32U reqDataSize, DCHAR** respData, D32U* respDataSize)
{
	log_e("app begin reqHandler1");

	//sleep(7);

	log_e("app recv req data: %s, size: %d", reqData, reqDataSize);

	*respData = adm_strdup("serv:1,1,1");
	*respDataSize = 11;

	log_e("app end reqHandler1");

	return 0;
}

void requestService2(char * buffer,void * user_data)
{
	char* resp = NULL;
	int respSize = 0;
	adm_arpc_request_service(0x102, 0x02, 0x03, 5, "abcd2", 6, &resp, &respSize);
	log_e("app recv resp, data: %s, size: %d", resp, respSize);
}

void requestService3(char * buffer,void * user_data)
{
	char* resp = NULL;
	int respSize = 0;
	adm_arpc_request_service(0x103, 0x02, 0x03, 5, "abcd3", 6, &resp, &respSize);
	log_e("app recv resp, data: %s, size: %d", resp, respSize);
}

void requestService4(char * buffer,void * user_data)
{
	char* resp = NULL;
	int respSize = 0;
	adm_arpc_request_service(0x104, 0x02, 0x03, 5, "abcd4", 6, &resp, &respSize);
	log_e("app recv resp, data: %s, size: %d", resp, respSize);
}

void requestService5(char * buffer,void * user_data)
{
	char* resp = NULL;
	int respSize = 0;
	adm_arpc_request_service(0x105, 0x02, 0x03, 5, "abcd5", 6, &resp, &respSize);
	log_e("app recv resp, data: %s, size: %d", resp, respSize);
}

void subevent2(char * buffer,void * user_data)
{
	char* resp = NULL;
	int respSize = 0;
	D32S retcode = adm_arpc_subscriberEvent(0x102, 0x02, 0x10);
	log_i("sub 0x102, 0x02, 0x10, retcode:%d", retcode);

	retcode = adm_arpc_subscriberEvent(0x102, 0x02, 0x20);
	log_i("sub 0x102, 0x02, 0x10, retcode:%d", retcode);
	
}

int test()
{
    command_desc_t commands[] =
    {
        {"r2","requestService2", NULL, requestService2, NULL},

        {"r3","requestService3", NULL, requestService3, NULL},

        {"r4","requestService2", NULL, requestService4, NULL},

        {"r5","requestService3", NULL, requestService5, NULL},

		{"s2","subevent2", NULL, subevent2, NULL},
      
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


D32S eventHandler2(D16U instanceId, DCHAR* reqData, D32U reqDataSize)
{
	log_e("app begin eventHandler2");

	log_e("app recv notify, instanceId:%d, %s, size: %d", instanceId, reqData, reqDataSize);

	log_e("app end eventHandler2");

	return 0;
}

D32S eventHandler3(D16U instanceId, DCHAR* reqData, D32U reqDataSize)
{
	log_e("app begin eventHandler3");

	log_e("app recv notify, instanceId:%d, %s, size: %d", instanceId, reqData, reqDataSize);

	log_e("app end eventHandler3");

	return 0;
}

D32S eventHandler4(D16U instanceId, DCHAR* reqData, D32U reqDataSize)
{
	log_e("app begin eventHandler4");

	log_e("app recv notify, instanceId:%d size: %d", instanceId, reqDataSize);

    DCHAR *offset;
    DCHAR temp[10] = {0};
    offset = adm_strchr(reqData, 'a');
    adm_memset(temp, 0x00, sizeof(temp), sizeof(temp));
    strncpy(temp, reqData, (DSIZE)(offset - reqData));

    D32S length = atoi(temp);
    //检查传输长度是否正确
    if(length == reqDataSize)
    {
        log_e("data complete!");
    }
    else
    {
        log_e("trans length %d,recv length %d",length, adm_strlen(reqData));
        log_e("data:%s",reqData);
    }
	log_e("app end eventHandler4");

	return 0;
}

int main(int argc, const char* argv[])
{
	
	//adm_debug_logInit("/tmp/arpc/dim/", (1 * 1024 * 1024), 5);
	
	D32U loopCount = 0;

	signal(SIGINT, handle_signal);
	signal(SIGTERM, handle_signal);

	DCHAR* otaConfigPath = NULL;
	D32S c = 0;

	//!< 接收入参
    while ((c = getopt (argc, (char * const*)argv, "c:")) != -1)
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
		usleep(10000); // 10ms
    }

	// read cfg, app core do
	ADM_CFG_API_INFO_T pstApiData = {0};
    pstApiData.isFileExistCb = adm_com_isFileExist;
    pstApiData.writeFileDataCb = adm_com_writeFileData;
    D32S iRet = adm_cfg_iniParserInit(otaConfigPath, &pstApiData);
    if (ADM_ECODE_SUCCESSED != iRet) 
    {
        printf("adm_cfg_iniParserInit %s failed, iRet:%d\n", otaConfigPath, iRet);
        return 0;
    }

	DCHAR* strLogPath = adm_cfg_iniParserGetString(ADM_CFG_VUS_LOG_STORAGE_DIR, ADM_CFG_VUS_LOG_STORAGE_DIR_DEFAULT_VAL);
	printf("strLogPath %s \n", strLogPath);
	adm_debug_logInit(strLogPath, (1 * 1024 * 1024), 5);
    
#if defined(ARPC_SSL)
    //初始化证书密码
    adm_arpc_setSSLKeyPasswd("123456");
#endif
    //if(ADM_ECODE_SUCCESSED != adm_arpc_initService(otaConfigPath))
    D16U instanceId = 1;
	if(ADM_ECODE_SUCCESSED != adm_arpc_initService(&instanceId))
	{
		return 0;
	}

	if(ADM_ECODE_SUCCESSED != adm_arpc_startService(NULL))
	{
		return 0;
	}

#if 1
	sleep(5);

	log_i("*******************************");
	
	// event 
	// sub 2,3
	adm_arpc_subscriberEvent(0x102, 0x02, 0x10);
	adm_arpc_registerEventHandler(0x102, 0x02, 0x10, eventHandler2);

	adm_arpc_subscriberEvent(0x102, 0x02, 0x10);

	adm_arpc_subscriberEvent(0x102, 0x02, 0x20);

	adm_arpc_registerEventHandler( 0x102, 0x02, 0x20, eventHandler2);
	
	adm_arpc_subscriberEvent(0x103, 0x02, 0x10);
	adm_arpc_registerEventHandler(0x103, 0x02, 0x10, eventHandler3);

	// sub 4,5
	adm_arpc_subscriberEvent(0x104, 0x02, 0x10);
	adm_arpc_registerEventHandler(0x104, 0x02, 0x10, eventHandler2);

	adm_arpc_subscriberEvent(0x104, 0x02, 0x20);
	adm_arpc_registerEventHandler( 0x104, 0x02, 0x20, eventHandler2);
	
	adm_arpc_subscriberEvent(0x105, 0x02, 0x10);
	adm_arpc_registerEventHandler(0x105, 0x02, 0x10, eventHandler3);

	adm_arpc_subscriberEvent(0x106, 0x02, 0x10);
	adm_arpc_registerEventHandler(0x106, 0x02, 0x10, eventHandler4);

	D16U* priv = (D16U*)adm_malloc(sizeof(D16U));
	*priv = 0x102;
	adm_arpc_registerInstanceBehaviorCb(0x102,INSTANCE_BEHAVIOR_ONLINE, sysiOnlinehandler, (void*)priv);

	adm_arpc_registerInstanceBehaviorCb(0x102,INSTANCE_BEHAVIOR_OFFLINE, sysiOfflinehandler, (void*)priv);

	// serv
	adm_arpc_registerServMethodHandler(0x101, 0x01, 0x01, reqHandler1);
	
#endif

	test();
	
    adm_arpc_stopService();

    adm_debug_logDeinit();
	
	return 0;
}


