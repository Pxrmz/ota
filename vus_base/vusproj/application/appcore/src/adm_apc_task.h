#ifndef __ADM_APC_TASK_H_
#define __ADM_APC_TASK_H_

#include "adm_typedefs.h"
#include "adm_list.h"
#include "adm_workerqueue.h"
#include "adm_apc_xxm.h"

#include <pthread.h>//TODO use platform api

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    ADM_APC_EVENT_ID_POWER_ON = 0,/*!< nullptr */
    ADM_APC_EVENT_ID_POWER_OFF,/*!< nullptr */
    ADM_APC_EVENT_ID_TRIGGER_TASK_CHECK,/*!< nullptr */
    ADM_APC_EVENT_ID_TRIGGER_TASK_DOWNLOAD,/*!< nullptr */
    ADM_APC_EVENT_ID_TRIGGER_TASK_UPGRADE,/*!< nullptr */
    ADM_APC_EVENT_ID_RESULT_CHECK_NO_NEW_VERSION,/*!< nullptr */
    ADM_APC_EVENT_ID_RESULT_CHECK_FIND_NEW_VERSION,/*!< nullptr */
    ADM_APC_EVENT_ID_RESULT_DOWNLOAD_SUCCESS,/*!< nullptr */
    ADM_APC_EVENT_ID_RESULT_DOWNLOAD_FATAL_ERROR,/*!< nullptr */
    ADM_APC_EVENT_ID_RESULT_DOWNLOAD_SHORT_ERROR,/*!< nullptr */
    ADM_APC_EVENT_ID_RESULT_UPGRADE_FINISH,/*!< nullptr */
    ADM_APC_EVENT_ID_RESULT_UPGRADE_SHORT_ERROR,/*!< nullptr */
    ADM_APC_EVENT_ID_RESULT_UPGRADE_BROKEN,/*!< nullptr */
    ADM_APC_EVENT_ID_BROKEN_CLEAN_UP,/*!< nullptr */
    ADM_APC_EVENT_ID_STATUS_END,/*!< nullptr */
    ADM_APC_EVENT_ID_MAX,
}ADM_APC_EVENT_ID_E;

typedef struct {
    ADM_APC_EVENT_ID_E eventId;
    union{
    }u;
}ADM_APC_EVENT_DATA_T;

typedef void (ADM_APC_EVENT_F)(const ADM_APC_EVENT_DATA_T *eventData, void *userData);

typedef struct {
    struct list_head entry;
    D64S indexId;
    ADM_APC_EVENT_ID_E eventId;
    ADM_APC_EVENT_F *fnHandle;
    void *userData;
}ADM_APC_EVENT_NODE_T;

typedef struct {
    ADM_WORKER_QUEUE_T *wq;
    pthread_mutex_t mutex;
    struct list_head head;
    D64S indexId;
}ADM_APC_EVENT_T;

typedef enum {
    ADM_APC_STATE_IDLE = 0,
    ADM_APC_STATE_CHECK_DOING,
    ADM_APC_STATE_DOWNLOAD_READY,
    ADM_APC_STATE_DOWNLOAD_DOING,
    ADM_APC_STATE_DOWNLOAD_PAUSE,
    ADM_APC_STATE_UPGRADE_READY,
    ADM_APC_STATE_UPGRADE_DOING,
    ADM_APC_STATE_UPGRADE_PAUSE,
    ADM_APC_STATE_UPGRADE_BROKEN,
    ADM_APC_STATE_MAX,
}ADM_APC_STATE_E;

typedef enum{
    ADM_APC_STEP_DATA_CMDID_UNKNOWN = 0,
    ADM_APC_STEP_DATA_CMDID_CHECK,
    ADM_APC_STEP_DATA_CMDID_CHECK_RESUME,
    ADM_APC_STEP_DATA_CMDID_DOWNLOAD,
    ADM_APC_STEP_DATA_CMDID_DOWNLOAD_RESUME,
    ADM_APC_STEP_DATA_CMDID_INSTALL,
    ADM_APC_STEP_DATA_CMDID_INSTALL_RESUME,
}ADM_APC_STEP_DATA_CMDID_E;

typedef struct {
    pthread_mutex_t mutex;
    ADM_APC_CVM_T cvm;
    ADM_APC_DPM_T dpm;
    ADM_APC_IPM_T ipm;
    ADM_APC_EVENT_T *event;
}ADM_APC_STEP_DATA_T;

typedef ADM_APC_STATE_E (*ADM_APC_STEP_F)(ADM_APC_STATE_E curState, const ADM_APC_EVENT_DATA_T *eventData, ADM_APC_STEP_DATA_T *stepData);

typedef struct {
    DBOOL isReady;
    ADM_APC_STATE_E curState;
    ADM_APC_STEP_F *fnSteps;
    ADM_APC_STEP_DATA_T stepData;
    ADM_WORKER_QUEUE_T *wq;
}ADM_APC_MANAGER_T;

typedef struct {
    ADM_APC_EVENT_T event;
    ADM_APC_MANAGER_T manager;
    //ADM_APC_UIM_T uim;
}ADM_APC_CONTEXT_T;



/* ADM_APC_CONTEXT_T */
static void apcContextInit(ADM_APC_CONTEXT_T *c);
static void apcContextDeInit(ADM_APC_CONTEXT_T *c);
static void apcContextStart(ADM_APC_CONTEXT_T *c);
static void apcContextStop(ADM_APC_CONTEXT_T *c);

/* ADM_APC_MANAGER_T */
static void apcManagerInit(ADM_APC_MANAGER_T *m, ADM_APC_EVENT_T *event);
static void apcManagerDeInit(ADM_APC_MANAGER_T *m);
static void apcManagerWorkerAdd(ADM_APC_MANAGER_T *m, const ADM_APC_EVENT_DATA_T *eventData);

/* ADM_APC_STEP_DATA_T */
static void apcStepDataInit(ADM_APC_STEP_DATA_T *s, ADM_APC_EVENT_T *event);
static void apcStepDataDeInit(ADM_APC_STEP_DATA_T *s);
static void apcStepDataAsyncCmd(ADM_APC_STEP_DATA_T *s, ADM_APC_STEP_DATA_CMDID_E cmdId, const ADM_APC_EVENT_DATA_T *eventData);

/* ADM_APC_STATE_E */
static const DCHAR *apcStateString(ADM_APC_STATE_E state);

/* ADM_APC_EVENT_T */
static void apcEventInit(ADM_APC_EVENT_T *event);
static void apcEventDeInit(ADM_APC_EVENT_T *event);
static void apcEventStop(ADM_APC_EVENT_T *event);
static D64S apcEventSub(ADM_APC_EVENT_T *event, ADM_APC_EVENT_ID_E eventId, ADM_APC_EVENT_F *fnHandle, void *userData);
static void apcEventUnSub(ADM_APC_EVENT_T *event, D64S indexId);
static void apcEventSendData(ADM_APC_EVENT_T *event, const ADM_APC_EVENT_DATA_T* eventData);
static void apcEventSendDataAsync(ADM_APC_EVENT_T *event, const ADM_APC_EVENT_DATA_T* eventData);
static void apcEventSend(ADM_APC_EVENT_T *event, ADM_APC_EVENT_ID_E eventId);

/* ADM_APC_EVENT_NODE_T */
static ADM_APC_EVENT_NODE_T* apcEventNodeNew(D64S indexId, ADM_APC_EVENT_ID_E eventId, ADM_APC_EVENT_F *fnHandle, void *userData);
static void apcEventNodeDel(ADM_APC_EVENT_NODE_T* node);

/* ADM_APC_EVENT_DATA_T */
static DBOOL apcEventDataInit(ADM_APC_EVENT_DATA_T *data, ADM_APC_EVENT_ID_E eventId);
static void apcEventDataInitDup(ADM_APC_EVENT_DATA_T *data, const ADM_APC_EVENT_DATA_T *src);
static void apcEventDataDeInit(ADM_APC_EVENT_DATA_T *data);

/* ADM_APC_EVENT_ID_E */
static const DCHAR *apcEventIdString(ADM_APC_EVENT_ID_E eventId);

#ifdef __cplusplus
}
#endif

#endif//__ADM_APC_TASK_H_
