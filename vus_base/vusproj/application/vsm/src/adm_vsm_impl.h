#ifndef __ADM_VSM_IMPL_H_
#define __ADM_VSM_IMPL_H_

#include "adm_vsm_fsm.h"
#include "adm_typedefs.h"
#include "json-c/json.h"

#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    ADM_VSM_RESULT_SUCCESS = 0,
    ADM_VSM_RESULT_SHORT_FAILED,
    ADM_VSM_RESULT_FATAL_FAILED,
} ADM_VSM_RESULT_E;

typedef struct
{
    ADM_APPLICATION_COMMON_CMDID_TYPE_E eTriggerType;
} ADM_VSM_XXM_T;

#define ADM_VSM_STATE_KV_KEY "vsm_state"
#define ADM_VSM_TRIGGER_TYPE_KV_KEY "vsm_triggerType"
#define ADM_VSM_STATE_TSP_KV_KEY "tsp_vsm_state"

typedef struct ADM_VSM_STATE_
{
    pthread_mutex_t mutex;
    ADM_VSM_STATE_E state;
    struct ADM_VSM_WORKER_* worker;
} ADM_VSM_STATE_T;

#define ADM_VSM_WORKER_WAIT_MSEC (200)

typedef struct ADM_VSM_WORKER_
{
    ADM_VSM_XXM_T* ucm;
    struct ADM_VSM_STATE_* state;
    pthread_mutex_t mutex;
    DCHAR stop;
    D32U count;
} ADM_VSM_WORKER_T;

typedef struct
{
    ADM_VSM_XXM_T ucm;
    ADM_VSM_STATE_T state;
    ADM_VSM_WORKER_T worker;
} ADM_VSM_MANAGER_T;

/* ADM_VSM_MANAGER_T */
static void vsmManagerInit(ADM_VSM_MANAGER_T* m);
static void vsmManagerDeInit(ADM_VSM_MANAGER_T* m);

/* ADM_VSM_WORKER_T */
static void vsmWorkerInit(ADM_VSM_WORKER_T* w, ADM_VSM_XXM_T* ucm, ADM_VSM_STATE_T* state);
static void vsmWorkerDeInit(ADM_VSM_WORKER_T* w);
static DBOOL vsmWorkerCheck(ADM_VSM_WORKER_T* w);
static DBOOL vsmWorkerDownload(ADM_VSM_WORKER_T* w);
static DBOOL vsmWorkerDownloadResume(ADM_VSM_WORKER_T* w);
static DBOOL vsmWorkerInstall(ADM_VSM_WORKER_T* w);
static DBOOL vsmWorkerInstallResume(ADM_VSM_WORKER_T* w);

/* ADM_VSM_STATE_T */
static void vsmStateInit(ADM_VSM_STATE_T* s, ADM_VSM_WORKER_T* worker);
static void vsmStateDeInit(ADM_VSM_STATE_T* s);
static ADM_VSM_STATE_E vsmStateGet(ADM_VSM_STATE_T* s);
static void vsmStateTriggerCheck(ADM_VSM_STATE_T* s);
static void vsmStateTriggerDownload(ADM_VSM_STATE_T* s);
//static void vsmStateTriggerDownloadCancel(ADM_VSM_STATE_T* s);
//static void vsmStateTriggerDownloadPause(ADM_VSM_STATE_T* s);
static void vsmStateTriggerInstall(ADM_VSM_STATE_T* s);
//static void vsmStateTriggerInstallCancel(ADM_VSM_STATE_T* s);
static void vsmStateResultCheck(ADM_VSM_STATE_T* s, ADM_VSM_RESULT_E result);
static void vsmStateResultDownload(ADM_VSM_STATE_T* s, ADM_VSM_RESULT_E result);
static void vsmStateResultInstall(ADM_VSM_STATE_T* s, ADM_VSM_RESULT_E result);
static void vsmStateResume(ADM_VSM_STATE_T* s);
static struct json_object* vsmStateJsonCreate(const ADM_VSM_STATE_T* s);
static void vsmStateJsonParse(ADM_VSM_STATE_T* s, const struct json_object* root);
static void vsmStateKVLoad(ADM_VSM_STATE_T* s);
static void vsmStateKVStore(const ADM_VSM_STATE_T* s);
static void vsmStateSet(ADM_VSM_STATE_T* s, ADM_VSM_STATE_E state);

/* ADM_VSM_XXM_T */
static void vsmUcmInit(ADM_VSM_XXM_T* x);
static void vsmUcmDeInit(ADM_VSM_XXM_T* x);
static ADM_VSM_RESULT_E vsmUcmCheck(ADM_VSM_XXM_T* x);
static ADM_VSM_RESULT_E vsmUcmDownload(ADM_VSM_XXM_T* x);
static ADM_VSM_RESULT_E vsmUcmDownloadResume(ADM_VSM_XXM_T* x);
static ADM_VSM_RESULT_E vsmUcmInstall(ADM_VSM_XXM_T* x);
static ADM_VSM_RESULT_E vsmUcmInstallResume(ADM_VSM_XXM_T* x);

/* ADM_VSM_STATE_E */
static const DCHAR* vsmStateString(ADM_VSM_STATE_E state);

/* UIM */
static DBOOL vsmGetIsAutoDownload(void);

ADM_APPLICATION_COMMON_CMDID_TYPE_E adm_vsm_getTriggerType();
void adm_vsm_setTriggerType(ADM_APPLICATION_COMMON_CMDID_TYPE_E eTriggerType);
void adm_vsm_forceResume(ADM_VSM_STATE_E eState, DBOOL bIsExit3E80);

#ifdef __cplusplus
}
#endif

#endif  //__ADM_VSM_IMPL_H_
