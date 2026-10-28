#ifndef AB_EWEBSRV_API_H_
#define AB_EWEBSRV_API_H_

#include <stddef.h>
#include <stdio.h>
#include <stdbool.h>

/* version-subversion */
#define AB_EWEBSRV_VERCODE_MAJOR      00
#define AB_EWEBSRV_VERCODE_MINOR      04

/*********************************************************************************/
/* CHANGES
201230               0.1  -- initialization
210316               0.2  --
                          1. change file cache from 8K to 256K
                          2. bugfix upload file for incorrect size
210330               0.3  -- bugfix for libcur:CURLOPT_RESUME_FROM_LARGE
                     0.4  -- support libcur curl_formadd
*/

typedef unsigned int        AB_EWEBSRV_STATE;
#define AB_EWEBSRV_STATE_UNINIT       0
#define AB_EWEBSRV_STATE_RUNNING      1
#define AB_EWEBSRV_STATE_CLOSING      2
#define AB_EWEBSRV_STATE_CLOSED       3

typedef unsigned int        AB_EWEBSRV_LOG_LEVEL;
#define AB_EWEBSRV_LOG_UNINIT       0
#define AB_EWEBSRV_LOG_DEBUG        1
#define AB_EWEBSRV_LOG_INFO         2
#define AB_EWEBSRV_LOG_WARNING      3
#define AB_EWEBSRV_LOG_ERROR        4
#define AB_EWEBSRV_LOG_NONE         5

#ifdef __cplusplus
extern "C" {
#endif

/*
 **********************************************************************************
 * webserver root document configuration
 *  @param [in] root : root file directory
 *  @param [in] uploaddir : upload file directory
 *  @return
 */
void abEWebSrv_CfgSetDocumentDir(const char* root, const char* uploaddir);

/*
 **********************************************************************************
 * webserver http port configuration
 *  @param [in] secured : https or http
 *  @param [in] port : socket port
 *  @return
 */
void abEWebSrv_CfgSetHttpPort(bool secured, int port);

/*
 **********************************************************************************
 * webserver http ip-port configuration
 *  @param [in] secured : https or http
 *  @param [in] ipPort : socket ip-port
 *  @details: eg:192.0.2.3:8080, 192.0.2.3:8080s . s for https
 *  @return
 */
void abEWebSrv_CfgSetHttpIpPort(bool secured, const char* ipPort);

/*
 **********************************************************************************
 * webserver https certification configuration
 *  @param [in] pem : certification file pathname
 *  @return
 */
void abEWebSrv_CfgSetSSLCertificateFile(const char* pem);

/*
 **********************************************************************************
 * webserver set configuration file
 *  @param [in] filename : configuration file pathname
 *  @return
 */
void abEWebSrv_CfgSetFile(const char* filename);

/*
 **********************************************************************************
 * webserver https CA certification configuration
 *  @param [in] pem : CA certification file pathname
 *  @return
 */
void abEWebSrv_CfgSetCaCertificateFile(const char* pem);

/*
 **********************************************************************************
 * webserver https verify mode configuration
 *  @param [bool] verifypeer : true if verify peer certification,false if not
 *  @return
 */
void abEWebSrv_CfgSetSSLVerifyMode(bool verifypeer);

/*
 **********************************************************************************
 * webserver start webservice
 *  @return 0 on success or failed
 */
int abEWebSrv_Start(void);

/*
 **********************************************************************************
 * webserver stop webservice
 */
void abEWebSrv_Stop(void);

/*
 **********************************************************************************
 * webserver get webservice state
 *  @return an code:AB_EWEBSRV_STATE
 */
AB_EWEBSRV_STATE abEWebSrv_GetState(void);

/*
 **********************************************************************************
 * webserver get software version number
 *  @return version format: 100*version+subversion
 */
int abEWebSrv_GetVersionCode(void);

#define AB_EWEBSRV_DEFAULT_VERSION  "ewebserver_default_version_1.0.0"
/*
 **********************************************************************************
 * webserver get software version number
 *  @return version format: ${PROJECT_NAME}_${ENV_PLATFORM}_${ADM_SDK_VERSION}_${COMPILE_DATE}_${GIT_COMMITID})
 */
char* abEwebsrv_GetSdkVersion(void);

/*
 **********************************************************************************
 * webserver log configuration
 *  @param [in] curLevel : off on AB_EWEBSRV_LOG_NONE
 *  @param [in] logfilename : save as file pathname
 *  @param [in] stdio : printf out or not
 *  @param [in] filesave : save as file or not
 *  @param [in] append : append content or not
 *  @return
 */
void abEWebSrv_SetLog(AB_EWEBSRV_LOG_LEVEL curLevel, const char* logfilename,
                      bool stdio,
                      bool filesave,
                      bool append);

/*
 **********************************************************************************
 * webserver logout
 *  @param [in] loglevel : message logout level
 *  @param [in] fmt : var argument supported
 *  @return
 */
void abEWebSrv_LogOut(AB_EWEBSRV_LOG_LEVEL loglevel, const char *fmt, ...) ;

#ifdef __cplusplus
}
#endif

#endif  /* AB_EWEBSRV_API_H_ */
