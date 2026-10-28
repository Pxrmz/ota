#include "ab_ewebsrv_api.h"

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <pthread.h>
#include <semaphore.h>
#include <string.h>
#include <memory.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "civetweb.h"

static struct mg_context *g_ctx = NULL; /* Set by start_civetweb() */
static volatile AB_EWEBSRV_STATE g_state_flag = AB_EWEBSRV_STATE_UNINIT;
static pthread_mutex_t g_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t g_cond = PTHREAD_COND_INITIALIZER;
static char *g_document_rootdir = NULL;
static char *g_document_uploaddir = NULL;
static char *g_ports = NULL;
static char *g_capem = NULL;
//g_certpem include certificate and privatekey
static char *g_certpem = NULL;
static char *g_cfgfile = NULL;
static bool g_verifypeer = NULL;

#define MAX_OPTIONS (50)
#define MAX_CONF_FILE_LINE_SIZE (8 * 1024)

#define ab_free(ioPtr)	\
	do {								\
		if (NULL != (ioPtr))					\
		{								\
			free((ioPtr));\
			(ioPtr) = NULL;				\
		}								\
	} while ( false )
    
int abEWebSrv_GetVersionCode(void){
    return AB_EWEBSRV_VERCODE_MAJOR *100 + AB_EWEBSRV_VERCODE_MINOR;
}

/*
 **********************************************************************************
 * webserver get software version number
 *  @return version format: ${PROJECT_NAME}_${ENV_PLATFORM}_${ADM_SDK_VERSION}_${COMPILE_DATE}_${GIT_COMMITID})
 */
char* abEwebsrv_GetSdkVersion(void)
{
    static char sdkVersion[128] = {0};
#ifdef ADM_EWEBSERVER_SDK_VERSION
    snprintf(sdkVersion, sizeof(sdkVersion), "%s", ADM_EWEBSERVER_SDK_VERSION);
#else
    snprintf(sdkVersion, sizeof(sdkVersion), "%s", AB_EWEBSRV_DEFAULT_VERSION);
#endif

    return sdkVersion;
}

static int log_message(const struct mg_connection *conn, const char *message){
    (void)conn;
    abEWebSrv_LogOut(AB_EWEBSRV_LOG_ERROR, "Error:%s", message);
    return 0;
}

static char *sdup(const char *str) {
    size_t len;
    char *p;
    if (str == NULL)
        return NULL;

    len = strlen(str) + 1;
    p = (char *)malloc(len);

    if (p == NULL) {
        abEWebSrv_LogOut(AB_EWEBSRV_LOG_ERROR, "Cannot allocate %u bytes", (unsigned)len);
    }

    memcpy(p, str, len);
    return p;
}

static bool IsDirctory(const char* pathname) {
    struct stat st;
    bool ret = false;
#ifdef _WIN32
    if (stat(pathname, &st) == 0){
#else
    if (stat(pathname, &st) == 0){
#endif
        ret = S_ISDIR(st.st_mode);
    }
    return ret;
}

static int set_option(char **options, const char *name, const char *value)
{
    int i, type;
    const struct mg_option *default_options = mg_get_valid_options();
    const char *multi_sep = NULL;

    /* check if it is a CivetWeb server option */
    type = MG_CONFIG_TYPE_UNKNOWN; /* type "unknown" means "option not found" */
    for (i = 0; default_options[i].name != NULL; i++) {
        if (!strcmp(default_options[i].name, name)) {
            type = default_options[i].type;
            break; /* no need to search for another option */
        }
    }

    switch (type) {

    case MG_CONFIG_TYPE_UNKNOWN:
        /* unknown option */
        return -1; /* return error */

    case MG_CONFIG_TYPE_NUMBER:
        /* integer number >= 0, e.g. number of threads */
    {
        char *chk = 0;
        unsigned long num = strtoul(value, &chk, 10);
        (void)num; /* do not check value, only syntax */
        if ((chk == NULL) || (*chk != 0) || (chk == value)) {
            /* invalid number */
            return 0;
        }
    }
        break;

    case MG_CONFIG_TYPE_STRING:
        /* any text */
        break;

    case MG_CONFIG_TYPE_STRING_LIST:
        /* list of text items, separated by , */
        // disable for port configuration
        // multi_sep = ",";
        break;

    case MG_CONFIG_TYPE_STRING_MULTILINE:
        /* lines of text, separated by carriage return line feed */
        multi_sep = "\r\n";
        break;

    case MG_CONFIG_TYPE_BOOLEAN:
        /* boolean value, yes or no */
        if ((0 != strcmp(value, "yes")) && (0 != strcmp(value, "no"))) {
            /* invalid boolean */
            return 0;
        }
        break;

    case MG_CONFIG_TYPE_YES_NO_OPTIONAL:
        /* boolean value, yes or no */
        if ((0 != strcmp(value, "yes")) && (0 != strcmp(value, "no"))
                && (0 != strcmp(value, "optional"))) {
            /* invalid boolean */
            return 0;
        }
        break;

    case MG_CONFIG_TYPE_FILE:
        /* TODO (low): check this option when it is set, instead of calling
         * verify_existence later */
        break;
    case MG_CONFIG_TYPE_DIRECTORY:
        if (!IsDirctory(value)) {
            abEWebSrv_LogOut(AB_EWEBSRV_LOG_ERROR, "%s is not directory!", value);
            return 0;
        }
        break;

    case MG_CONFIG_TYPE_EXT_PATTERN:
        /* list of patterns, separated by | */
        multi_sep = "|";
        break;

    default:
        abEWebSrv_LogOut(AB_EWEBSRV_LOG_ERROR, "Unknown option type - option %s", name);
    }

    for (i = 0; i < MAX_OPTIONS; i++) {
        if (options[2 * i] == NULL) {
            /* Option not set yet. Add new option */
            options[2 * i] = sdup(name);
            options[2 * i + 1] = sdup(value);
            options[2 * i + 2] = NULL;
            break;
        } else if (!strcmp(options[2 * i], name)) {
            if (multi_sep) {
                /* Option already set. Append new value. */
                char *s =
                        (char *)malloc(strlen(options[2 * i + 1])
                        + strlen(multi_sep) + strlen(value) + 1);
                if (!s) {
                    abEWebSrv_LogOut(AB_EWEBSRV_LOG_ERROR, "Out of memory");
                }
                sprintf(s, "%s%s%s", options[2 * i + 1], multi_sep, value);
                ab_free(options[2 * i + 1]);
                options[2 * i + 1] = s;
            } else {
                /* Option already set. Overwrite */
                ab_free(options[2 * i + 1]);
                options[2 * i + 1] = sdup(value);
            }
            break;
        }
    }

    if (i == MAX_OPTIONS) {
        abEWebSrv_LogOut(AB_EWEBSRV_LOG_ERROR, "Too many options specified");
    }

    if (options[2 * i] == NULL) {
        abEWebSrv_LogOut(AB_EWEBSRV_LOG_ERROR, "Out of memory");
    }
    if (options[2 * i + 1] == NULL) {
        abEWebSrv_LogOut(AB_EWEBSRV_LOG_ERROR, "Illegal escape sequence, or out of memory");
    }

    /* option set correctly */
    return 1;
}

static int read_config_file(const char *config_file, char **options)
{
    char line[MAX_CONF_FILE_LINE_SIZE], *p;
    FILE *fp = NULL;
    size_t i, j, line_no = 0;

    /* Open the config file */
    fp = fopen(config_file, "r");
    if (fp == NULL) {
        abEWebSrv_LogOut(AB_EWEBSRV_LOG_ERROR, "Failed to open the file %s!", config_file);
        return 0;
    }

    /* Load config file settings first */
    abEWebSrv_LogOut(AB_EWEBSRV_LOG_INFO, "Loading config file %s", config_file);

    /* Loop over the lines in config file */
    while (fgets(line, sizeof(line), fp) != NULL) {

        if (!line_no && !memcmp(line, "\xEF\xBB\xBF", 3)) {
            /* strip UTF-8 BOM */
            p = line + 3;
        } else {
            p = line;
        }
        line_no++;

        /* Ignore empty lines and comments */
        for (i = 0; isspace((unsigned char)p[i]);)
            i++;
        if (p[i] == '#' || p[i] == '\0') {
            continue;
        }

        /* Skip spaces, \r and \n at the end of the line */
        for (j = strlen(p); (j > 0)
             && (isspace((unsigned char)p[j - 1])
                 || iscntrl((unsigned char)p[j - 1]));)
            p[--j] = 0;

        /* Find the space character between option name and value */
        for (j = i; !isspace((unsigned char)p[j]) && (p[j] != 0);)
            j++;

        /* Terminate the string - then the string at (p+i) contains the
         * option name */
        p[j] = 0;
        j++;

        /* Trim additional spaces between option name and value - then
         * (p+j) contains the option value */
        while (isspace((unsigned char)p[j])) {
            j++;
        }

        /* Set option */
        if (!set_option(options, p + i, p + j)) {
            abEWebSrv_LogOut(AB_EWEBSRV_LOG_ERROR,
                             "%s: line %d is invalid, ignoring it:\n %s",
                             config_file,
                             (int)line_no,
                             p);
        }
    }

    (void)fclose(fp);

    return 1;
}

static void *civetweb_mainloop(void *arg)
{
    struct mg_callbacks callbacks;
    char *options[2 * MAX_OPTIONS + 1];

    (void)arg;
    g_state_flag = AB_EWEBSRV_STATE_RUNNING;
    /* Initialize options structure */
    memset(options, 0, sizeof(options));
    if (g_document_rootdir) {
        set_option(options, "document_root", g_document_rootdir);
        ab_free(g_document_rootdir);
    }
    else {
        set_option(options, "document_root", ".");
    }
    if (g_document_uploaddir) {
        set_option(options, "document_uploaddir", g_document_uploaddir);
        ab_free(g_document_uploaddir);
    }
    else {
        set_option(options, "document_uploaddir", ".");
    }
    if (g_ports){
        set_option(options, "listening_ports", g_ports);
        ab_free(g_ports);
    }
    if (g_capem){
        set_option(options, "ssl_ca_file", g_capem);
        ab_free(g_capem);
    }
    if (g_certpem){
        set_option(options, "ssl_certificate", g_certpem);
        ab_free(g_certpem);
    }
    if (g_verifypeer == true){
        set_option(options, "ssl_verify_peer", "yes");
    }
    if (g_cfgfile){
        read_config_file(g_cfgfile, options);
        ab_free(g_cfgfile);
    }

    /* Start Civetweb */
    memset(&callbacks, 0, sizeof(callbacks));
    callbacks.log_message = &log_message;
    abEWebSrv_LogOut(AB_EWEBSRV_LOG_INFO, "Start webserver");
    g_ctx = mg_start(&callbacks, NULL, (const char **)options);

    /* mg_start copies all options to an internal buffer.
     * The options data field here is not required anymore. */
    for (int i = 0; options[i] != NULL; i++) {
        ab_free(options[i]);
    }

    /* If mg_start fails, it returns NULL */
    if (g_ctx == NULL) {
        abEWebSrv_LogOut(AB_EWEBSRV_LOG_ERROR, "WebSrv start failed");
    }
    else {
        abEWebSrv_LogOut(AB_EWEBSRV_LOG_INFO,
                         "Started on port(s) %s with web root [%s] upload [%s]",
                         mg_get_option(g_ctx, "listening_ports"),
                         mg_get_option(g_ctx, "document_root"),
                         mg_get_option(g_ctx, "document_uploaddir"));
        pthread_mutex_lock(&g_mutex);
        if (g_state_flag == AB_EWEBSRV_STATE_RUNNING){
            pthread_cond_wait(&g_cond,&g_mutex);
        }
        pthread_mutex_unlock(&g_mutex);

        abEWebSrv_LogOut(AB_EWEBSRV_LOG_INFO, "Exiting, waiting for threads to finish...");
        mg_stop(g_ctx);
        abEWebSrv_LogOut(AB_EWEBSRV_LOG_INFO, "Webserver stopped");
    }

    // pthread_mutex_destroy(&g_mutex);
    // pthread_cond_destroy(&g_cond);
    g_state_flag = AB_EWEBSRV_STATE_CLOSED;
    return ((void *)0);
}

int abEWebSrv_Start(void){
    pthread_t thread_id;
    pthread_attr_t attr;
    int result;
    if (g_state_flag == AB_EWEBSRV_STATE_RUNNING || g_state_flag == AB_EWEBSRV_STATE_CLOSING)
        return -1;

    (void)pthread_attr_init(&attr);
    (void)pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    result = pthread_create(&thread_id, &attr, civetweb_mainloop, NULL);
    pthread_attr_destroy(&attr);

    return result;
}

void abEWebSrv_Stop(void){
    if (g_state_flag != AB_EWEBSRV_STATE_RUNNING)
        return;
    pthread_mutex_lock(&g_mutex);
    g_state_flag = AB_EWEBSRV_STATE_CLOSING;
    pthread_cond_signal(&g_cond);
    pthread_mutex_unlock(&g_mutex);
}

AB_EWEBSRV_STATE abEWebSrv_GetState(void) {
    return g_state_flag;
}

void abEWebSrv_CfgSetDocumentDir(const char* root, const char* uploaddir){
    ab_free(g_document_rootdir);
    g_document_rootdir = sdup(root);
    ab_free(g_document_uploaddir);
    g_document_uploaddir = sdup(uploaddir);
}

void abEWebSrv_CfgSetHttpPort(bool secured, int port){
    ab_free(g_ports);
    char buf[32];
    snprintf(buf, 32, "%d%s", port, secured?"s":"");
    g_ports = sdup(buf);
}

void abEWebSrv_CfgSetHttpIpPort(bool secured, const char* ipPort){
    ab_free(g_ports);
    char buf[32];
    snprintf(buf, 32, "%s%s", ipPort, secured?"s":"");
    g_ports = sdup(ipPort);
}

void abEWebSrv_CfgSetSSLCertificateFile(const char* pem){
    ab_free(g_certpem);
    g_certpem = sdup(pem);
}

void abEWebSrv_CfgSetFile(const char* filename){
    ab_free(g_cfgfile);
    g_cfgfile = sdup(filename);
}

void abEWebSrv_CfgSetCaCertificateFile(const char* pem){
    ab_free(g_capem);
    g_capem = sdup(pem);
}

void abEWebSrv_CfgSetSSLVerifyMode(bool verifypeer){
    g_verifypeer = verifypeer;
}

