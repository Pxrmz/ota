
/*!
 *******************************************************************************
 * \file	aADM_error.h
 *
 * \brief	Error Codes
 *
 * The error codes returned by ADM APIs.
 *******************************************************************************
 */
#ifndef _ADM_ERROR_H_
#define _ADM_ERROR_H_			//!< Internal

#ifdef __cplusplus
extern "C" {
#endif

#include "adm_typedefs.h"

/*! ADM_ERROR */
typedef D32S ADM_ERROR;

/*! \addtogroup ADM_ERR_defs Errors
* @{
*/

/*! Success */
#define ADM_ERR_OK									0x0000	/*     0 */

/*! Invalid input */
#define ADM_ERR_INVALID_INPUT_PARAM					0x0002	/*     2 */

/*! Unspecified error */
#define ADM_ERR_UNSPECIFIC							0x0010	/*    16 */

/*! Memory error */
#define ADM_ERR_MEMORY								0x0011	/*    17 */

/*! Routine called when not allowed or bad parameters */
#define ADM_ERR_INVALID_CALL						0x0012	/*    18 */

/*! \ref ADM_run called with non-resume trigger when suspended */
#define ADM_ERR_IS_SUSPENDED						0x0013	/*    19 */

/*! SyncML message protocol or version error */
#define ADM_ERR_INVALID_PROTO_OR_VERSION			0x0020	/*	  32 */

/*! RTK workspace buffer too small */
#define ADM_ERR_RTK_BUFFER_OVERFLOW					0x2002	/*	8194 */

/*! Update Task is cancelled */
#define ADM_ERR_SERVER_DL_CANCELLED                  0x2100
/*! Download Task is cancelled */
#define ADM_ERR_SERVER_UPDATE_CANCELLED              0x2101
/*! Download max resume times is reached */
#define ADM_ERR_MAX_DL_RESUME_TIMES_REACHED          0x2102


/*
 * Synchronous Exec return codes range
 */

/*! Start range of synchronous Exec return code (not an error) */
#define ADM_ERR_EXEC_START_RANGE					0x3000 /* 12288 */

/*! End range of synchronous Exec return code (not an error) */
#define ADM_ERR_EXEC_END_RANGE						0x3FFF /* 16383 */


/*
 * Analytics error codes range
 */
/*! Start range of Analytics return code (not an error) */
#define ADM_ERR_ANALYTICS_RANGE_START				0x4000 	/* 16384 */

/*! Failed to create database */
#define ADM_ERR_DATABASE_CREATE_FAILED				0x4200 	/* 16896 */

/*! Failed to open database */
#define ADM_ERR_DATABASE_OPEN_FAILED				0x4201 	/* 16897 */

/*! Failed to insert data to database */
#define ADM_ERR_DATABASE_INSERT_DATA_FAILED			0x4202 	/* 16898 */

/*! Failed to update data to database */
#define ADM_ERR_DATABASE_UPDATE_DATA_FAILED			0x4203 	/* 16899 */

/*! Failed to select data from database */
#define ADM_ERR_DATABASE_SELECT_DATA_FAILED			0x4204 	/* 16900 */

/*! Failed to delete data from database */
#define ADM_ERR_DATABASE_DELETE_DATA_FAILED			0x4205 	/* 16901 */

/*! Failed to purge data from database */
#define ADM_ERR_DATABASE_PURGE_DATA_FAILED			0x4206 	/* 16902 */

/*! Failed to upgrade the database schema */
#define ADM_ERR_DATABASE_UPGRADE_FAILED				0x4207	/* 16903 */

/*! Bad  Message from server */
#define ADM_ERR_BAD_MSG								0x4230 	/* 16944 */

/*! End range of Analytics return code (not an error) */
#define ADM_ERR_ANALYTICS_RANGE_END					0x4FFF 	/* 20479 */



/*! Supplied buffer is too small */
#define ADM_ERR_BUFFER_OVERFLOW						0x6000	/* 24576 */

/*! Badly formatted input */
#define ADM_ERR_BAD_INPUT							0x6001	/* 24577 */

/*! Tree node already exists */
#define ADM_ERR_NODE_ALREADY_EXISTS					0x6002	/* 24578 */

/*! Tree node is missing */
#define ADM_ERR_NODE_MISSING						0x6003	/* 24579 */

/*! Parent node is missing */
#define ADM_ERR_PARENT_MISSING						0x6004	/* 24580 */

/*! Error in leaf node */
#define ADM_ERR_LEAF_NODE							0x6005	/* 24581 */

/*! Leaf node expected */
#define ADM_ERR_NOT_LEAF_NODE						0x6006	/* 24582 */

/*! Unknown property */
#define ADM_ERR_UNKNOWN_PROPERTY					0x6007	/* 24583 */

/*! Attempt made to delete a permanent node */
#define ADM_ERR_PERMANENT_NODE						0x6008	/* 24584 */

/*! Not allowed by AccessType */
#define ADM_ERR_NOT_ALLOWED							0x6009	/* 24585 */

/*! Client aborted */
#define ADM_ERR_ABORT								0x600a	/* 24586 */

/*! Client access denied */
#define ADM_ERR_TREE_ACCESS_DENIED					0x600b	/* 24587 */

/*! Partial write of external data not allowed */
#define ADM_ERR_TREE_EXT_NOT_PARTIAL				0x600c	/* 24588 */

/*! Write of external data not allowed at this time */
#define	ADM_ERR_TREE_EXT_NOT_ALLOWED				0x600d	/* 24589 */

/*! May not replace */
#define ADM_ERR_TREE_MAY_NOT_REPLACE				0x600e	/* 24590 */

/*! Persistent storage read error */
#define ADM_ERR_STORAGE_READ						0x600f	/* 24591 */

/*! Persistent storage write error */
#define ADM_ERR_STORAGE_WRITE						0x6010	/* 24592 */

/*! Authentication failure */
#define ADM_ERR_AUTHENTICATION						0x6011	/* 24593 */

/*! Access denied by ACL */
#define ADM_ERR_NODE_ACCESS_DENIED					0x6012	/* 24594 */

/*! External data value is not readable */	
#define ADM_ERR_NODE_VALUE_NOT_READABLE				0x6013	/* 24595 */

/*! External data value is not writable */
#define ADM_ERR_NODE_VALUE_NOT_WRITEABLE			0x6014	/* 24596 */

/*! Node not registered for execute */
#define ADM_ERR_NODE_NOT_EXECUTABLE					0x6015	/* 24597 */

/*! Tree open error */
#define ADM_ERR_STORAGE_OPEN						0x6016	/* 24598 */

/*! Tree commit error */
#define ADM_ERR_STORAGE_COMMIT						0x6017	/* 24599 */



/*! No more commands<br>Used internally by the Engine. This is not really an
 * error. */
#define ADM_ERR_NO_MORE_COMMANDS					0x6100 /* 24832 */

/*! Missing start message command */
#define ADM_ERR_MISSING_START_MESSAGE_CMD			0x6101 /* 24833 */

/*! Missing status command */
#define ADM_ERR_MISSING_STATUS_CMD					0x6102 /* 24834 */

/*! Optional feature not implemented */
#define ADM_ERR_NOT_IMPLEMENTED						0x6103 /* 24835 */

/*! Alert - options parsing error */
#define ADM_ERR_ALERT_PARSING_ERROR					0x6105 /* 24837 */

/*! Alert - not enough items */
#define ADM_ERR_ALERT_MISSING_ITEMS					0x6106 /* 24838 */

/*! Alert - not enough data */
#define ADM_ERR_ALERT_MISSING_DATA					0x6107 /* 24839 */

/*! No data */
#define ADM_ERR_NO_DATA								0x6108 /* 24840 */

/*! Alert - user cancelled or aborted */
#define ADM_ERR_ALERT_USER_ABORTED					0x6109 /* 24841 */

/*! Alert - too many choices passed to implementation */
#define ADM_ERR_ALERT_TOO_MANY_CHOICES				0x610a /* 24842 */

/*! Alert - server has sent a session-abort alert */
#define ADM_ERR_ALERT_SESSION_ABORTED				0x610b /* 24843 */

/*! Large object item has been handled<br>Used internally by the Engine. This
 * is not really an error. */
#define ADM_ERR_LO_HANDLED							0x610c /* 24844 */

/*! Data is too long to pass back as a large object */
#define ADM_ERR_TOO_BIG								0x610d /* 24845 */

/*! Command status code is failed */
#define ADM_ERR_COMMAND_FAILED						0x610e /* 24846 */

/*! Notification - message has invalid length */
#define ADM_ERR_NOTIF_BAD_LENGTH					0x6200	/* 25088 */

/*! Notification - message has invalid digest */
#define ADM_ERR_NOTIF_BAD_DIGEST					0x6201	/* 25089 */

/*! Boot message has invalid digest */
#define ADM_ERR_BOOT_DIGEST							0x6202	/* 25090 */

/*! Could not get NSS for bootstrap */
#define ADM_ERR_BOOT_NSS							0x6203	/* 25091 */

/*! Could not get PIN for bootstrap */
#define ADM_ERR_BOOT_PIN							0x6204	/* 25092 */

/*! Bad bootstrap PIN length */
#define ADM_ERR_BOOT_PINLENGTH						0x6205	/* 25093 */

/*! Bad bootstrap SEC value */
#define ADM_ERR_BOOT_BAD_SEC						0x6206	/* 25094 */

/*! Bad bootstrap MAC */
#define ADM_ERR_BOOT_BAD_MAC						0x6207	/* 25095 */

/*! Bad bootstrap message */
#define ADM_ERR_BOOT_BAD_MESSAGE					0x6208	/* 25096 */

/*! Bad bootstrap profile */
#define ADM_ERR_BOOT_BAD_PROF						0x6209	/* 25097 */

/*! Bad trigger reason */
#define ADM_ERR_TRG_BAD_REASON						0x6210	/* 25104 */

/*! Notification - message contains unsupported version info */
#define ADM_ERR_NOTIF_UNSUPPORTED_VERSION			0x6211	/* 25105 */

/*! Bootstrap not currently allowed */
#define ADM_ERR_BOOT_DISABLED						0x6212	/* 25106 */

/*! Non-ADM Bootstrap message  */
#define ADM_ERR_CP_BOOSTRAP_WITHOUT_ADM_PROFILE		0x6213	/* 25107 */

/*! Download object too large */
#define ADM_ERR_DL_OBJ_TOO_LARGE					0x6214  /* 25108 */

/*! Bad Nia Format */
#define ADM_ERR_BAD_NIA_FORMAT						0x6215  /* 25109 */

/*
 *	Communication errors
 */

/*! Unsupported protocol */
#define ADM_ERR_COMMS_BAD_PROTOCOL					0x6300 /* 25344 */

/*! Mismatched reply: XML received when WBXML sent or vice-versa */
#define ADM_ERR_COMMS_MIME_MISMATCH					0x6301 /* 25345 */

/*! General fatal transport error */
#define ADM_ERR_COMMS_FATAL							0x6302 /* 25346 */

/*! Start range of non-fatal communication errors */
#define ADM_ERR_COMMS_NON_FATAL_RANGE_START			0x6303

/*! General non-fatal transport error (can be retried) */
 #define ADM_ERR_COMMS_NON_FATAL					0x6303 /* 25347 */
/*! Socket timeout transport error */
 #define ADM_ERR_COMMS_SOCKET_TIMEOUT				0x6304 /* 25348 */
 /*! General socket non-fatal (retriable) socket error */
 #define ADM_ERR_COMMS_SOCKET_ERROR					0x6305 /* 25349 */
  /*! HTTP result wasn't found */
 #define ADM_ERR_HTTP_RESULT_NOT_FOUND				0x6306 /* 25350 */
  /*! HTTP result bad request*/
 #define ADM_ERR_HTTP_RESULT_BAD_REQUEST			0x6307 /* 25351 */
  /*! HTTP result internal server error*/
 #define ADM_ERR_HTTP_RESULT_INTERNAL_SERVER_ERROR    0x6308 /* 25352  */
/*! End range of non-fatal communication errors */
 #define ADM_ERR_COMMS_NON_FATAL_RANGE_END			0x633F
  
/*! Start range of fatal communication errors */
 #define ADM_ERR_COMMS_FATAL_RANGE_START				0x6340
/*! HTTP error */
 #define ADM_ERR_COMMS_HTTP_ERROR					0x6340 /* 25408 */
/*! SSL error */
 #define ADM_ERR_COMMS_SSL_ERROR					    0x6341 /* 25409 */
/*! SSL error */
 #define ADM_ERR_COMMS_SSL_BAD_CACERT                0x6342 /* 25410 */
/*! SSL error */
 #define ADM_ERR_COMMS_SSL_BAD_CIPHER                0x6343 /* 25411 */
/*! SSL error */
 #define ADM_ERR_COMMS_SSL_BAD_CERT                  0x6344 /* 25412 */
  /*! hostname can not be resolved. */
 #define ADM_ERR_HOSTNAME_RESOLVE_FAIL               0x6345 /* 25413  */
/*! End range of fatal communication errors */
#define ADM_ERR_COMMS_FATAL_RANGE_END				0x637F


/*! Get if fatal communication error */
#define IS_ADM_COMMS_ERROR_FATAL(E)	\
	(E == (ADM_ERR_COMMS_FATAL) || \
	(E >= (ADM_ERR_COMMS_FATAL_RANGE_START) && E <= (ADM_ERR_COMMS_FATAL_RANGE_END)))

/*! Get if non-fatal communication error */
#define IS_ADM_COMMS_ERROR_NON_FATAL(E)	\
	(E >= (ADM_ERR_COMMS_NON_FATAL_RANGE_START) && E <= (ADM_ERR_COMMS_FATAL_RANGE_END))

/*! Start range of vendor specified transport errors */
#define ADM_ERR_COMMS_VENDOR_SPECIFIC_RANGE_START	0x6380 /* 25472 */
/*! End range of vendor specified transport errors */
#define ADM_ERR_COMMS_VENDOR_SPECIFIC_RANGE_END		0x63FF /* 25599 */


/*! Internal error */
#define ADM_ERR_INTERNAL							0x6401 /* 25601 */

/*! Error accessing MO external storage */
#define ADM_ERR_MO_STORAGE							0x6402 /* 25602 */

/*! User cancelled update or download */
#define ADM_ERR_CANCEL								0x6404 /* 25604 */

/*! Could not initiate update client */
#define ADM_ERR_UPDATE_INIT							0x6405 /* 25605 */

/*! Download URL is malformed or bad */
#define ADM_ERR_BAD_URL								0x6406 /* 25606 */

/*! Error while parsing Download Descriptor */
#define ADM_ERR_BAD_DD								0x6407 /* 25607 */

/*! ETag changed, 412 returned because of If-match */
#define ADM_ERR_COMMS_OBJECT_CHANGED				0x6408 /* 25608 */

/*! Error while parsing Download Descriptor - descriptor size is 0*/
#define ADM_ERR_BAD_DD_INVALID_SIZE					0x6409 /* 25609 */

/*! A call to a ADM API is out of sync */
#define ADM_ERR_OUT_OF_SYNC							0x6500 /* 25856 */

/*
 * DS error codes (for future use).
 */

/*! For future use */
#define ADM_ERR_MISSING_DATASTORE					0x7001 /* 28673 */

/*! For future use */
#define ADM_ERR_NO_ANCHOR							0x7002 /* 28674 */

/*! For future use */
#define ADM_ERR_SUCCESS_ADD							0x7201 /* 29185 */

/*! For future use */
#define ADM_ERR_CLIENT_PREVAIL						0x7208 /* 29192 */

/*! For future use */
#define ADM_ERR_DELETED_WITHOUT_ARCHIVE 			0x7210 /* 29200 */

/*! For future use */
#define ADM_ERR_DATA_NOT_DELETED					0x7211 /* 29201 */

/*! For future use */
#define ADM_ERR_PASS_AUTHENTICATION					0x7212 /* 29202 */

/*! For future use */
#define ADM_ERR_DELIVER_LARGE_OBJ					0x7213 /* 29203 */

/*! For future use */
#define ADM_ERR_AUTH_ERR							0x7401 /* 29697 */

/*! For future use */
#define ADM_ERR_AUTH_ERR_ANOTHER_SYNCML				0x7403 /* 29699 */

/*! For future use */
#define ADM_ERR_DELETE_NO_DATE          			0x7404 /* 29700 */

/*! For future use */
#define ADM_ERR_AUTH_REQUESTED          			0x7407 /* 29703 */

/*! For future use */
#define ADM_ERR_REVISE_CONFLICT         			0x7409 /* 29705 */

/*! For future use */
#define ADM_ERR_UNINTEGRATED_CMD        			0x7412 /* 29714 */

/*! For future use */
#define ADM_ERR_TARGET_DATA_EXISTS      			0x7418 /* 29720 */

/*! For future use */
#define ADM_ERR_CONFLICT_SRV_PREVAIL    			0x7419 /* 29721 */

/*! For future use */
#define ADM_ERR_LIMIT_CAPACITY          			0x7420 /* 29728 */

/*! For future use */
#define ADM_ERR_DEFAULT_SERVER_ERR      			0x7500 /* 29952 */

/*! For future use */
#define ADM_ERR_BUSY_SERVER             			0x7503 /* 29955 */

/*! For future use */
#define ADM_ERR_MORE_SLOW_SYNC          			0x7508 /* 29960 */

/*! For future use */
#define ADM_ERR_ACCESS_DATABASE         			0x7510 /* 29968 */

/*! For future use */
#define ADM_ERR_SERVER_ERROR            			0x7511 /* 29969 */

/*! For future use */
#define ADM_ERR_FAILED_SYNCML          				0x7512 /* 29970 */

/*! For future use */
#define ADM_ERR_UNSUPPORTED_PROT_VER    			0x7513 /* 29971 */

/*
 *	ADM errors
 */

/*! Error accessing registry */
#define ADM_ERR_REGISTRY						 0x8001	/* 32769 */

/*! Shutting down - cannot enqueue triggers or timed events */
#define ADM_ERR_SHUTTING_DOWN					 0x8002	/* 32770 */

/*! Out of Bounds - an array was accessed out of bounds */
#define ADM_ERR_OUT_OF_BOUNDS					 0x8003	/* 32771 */

/*! Download package cannot be removed */
#define ADM_ERR_STORAGE_REMOVE					 0x8004	/* 32772 */

/*! Error accessing storage */
#define ADM_ERR_STORAGE_SEEK					 0x8005	/* 32773 */

/*! One of the storage errors */
#define IS_ADM_TREE_ERROR(E)					((E == ADM_ERR_STORAGE_WRITE) || \
												(E == ADM_ERR_STORAGE_READ) || \
												(E == ADM_ERR_STORAGE_COMMIT))

/*! Download package verification sha1 mismatch */
#define ADM_ERR_DIGEST_MISMATCH                      0x8101
/*! Download package verification dependency not satisfied*/
#define ADM_ERR_PACKAGE_NOT_COMPATIBLE               0x8102


/*
 *	TRUE errors
 */

/*! SED not provisioned */
#define ADM_ERR_MISSING_ENT						 0x9001	/* 36865 */

/*! SED is locked */
#define ADM_ERR_LOCKED_ENT						 0x9002	/* 36866 */
/* @} */


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif

