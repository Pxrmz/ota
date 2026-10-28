#include "adm_apc_task.h"
#include "adm_apc_log.h"
#include "adm_memory.h"
#include "adm_ips_tool.h"

static ADM_APC_CONTEXT_T m_context;
void adm_apc_contextInit(){
    apcContextInit(&m_context);
}
void adm_apc_contextDeInit(){
    apcContextDeInit(&m_context);
}
void adm_apc_contextStart(){
    apcContextStart(&m_context);
}
void adm_apc_contextStop(){
    apcContextStop(&m_context);
}

#if 0
/* ADM_APC_UIM_T init handle */
static void uimTriggerCheck(void *priv){
    ADM_APC_EVENT_T *event = (ADM_APC_EVENT_T *)priv;
    apcEventSend(event, ADM_APC_EVENT_ID_TRIGGER_TASK_CHECK);
}
static void uimTriggerDownload(void *priv){
    ADM_APC_EVENT_T *event = (ADM_APC_EVENT_T *)priv;
    apcEventSend(event, ADM_APC_EVENT_ID_TRIGGER_TASK_DOWNLOAD);
}
static void uimTriggerInstall(void *priv){
    ADM_APC_EVENT_T *event = (ADM_APC_EVENT_T *)priv;
    apcEventSend(event, ADM_APC_EVENT_ID_TRIGGER_TASK_UPGRADE);
}
#endif

/* ADM_APC_CONTEXT_T */
static void apcContextInit(ADM_APC_CONTEXT_T *c){
    apcEventInit(&c->event);
    apcManagerInit(&c->manager, &c->event);
}
static void apcContextDeInit(ADM_APC_CONTEXT_T *c){
    apcManagerDeInit(&c->manager);
    apcEventDeInit(&c->event);
}
static void apcContextStart(ADM_APC_CONTEXT_T *c){
    apcEventSend(&c->event, ADM_APC_EVENT_ID_POWER_ON);
}
static void apcContextStop(ADM_APC_CONTEXT_T *c){
    /* TODO stop all struct member */
    apcEventStop(&c->event);
}

/* ADM_APC_STEP_F */
static ADM_APC_STATE_E apcStepIdle(ADM_APC_STATE_E curState, const ADM_APC_EVENT_DATA_T *eventData, ADM_APC_STEP_DATA_T *stepData){
    ADM_APC_STATE_E state = curState;
    switch(eventData->eventId){
	case ADM_APC_EVENT_ID_TRIGGER_TASK_CHECK:
	    apcStepDataAsyncCmd(stepData, ADM_APC_STEP_DATA_CMDID_CHECK, eventData);
	    state = ADM_APC_STATE_CHECK_DOING;
	    break;
	default:break;
    }
    return state;
}
static ADM_APC_STATE_E apcStepCheckDoing(ADM_APC_STATE_E curState, const ADM_APC_EVENT_DATA_T *eventData, ADM_APC_STEP_DATA_T *stepData){
    ADM_APC_STATE_E state = curState;
    switch(eventData->eventId){
	case ADM_APC_EVENT_ID_POWER_ON:
	    apcStepDataAsyncCmd(stepData, ADM_APC_STEP_DATA_CMDID_CHECK_RESUME, eventData);
	    break;
	case ADM_APC_EVENT_ID_RESULT_CHECK_NO_NEW_VERSION:
	    apcEventSend(stepData->event, ADM_APC_EVENT_ID_STATUS_END);
	    state = ADM_APC_STATE_IDLE;
	    break;
	case ADM_APC_EVENT_ID_RESULT_CHECK_FIND_NEW_VERSION:
	    state = ADM_APC_STATE_DOWNLOAD_READY;
	    break;
	default:break;
    }
    return state;
}
static ADM_APC_STATE_E apcStepDownloadReady(ADM_APC_STATE_E curState, const ADM_APC_EVENT_DATA_T *eventData, ADM_APC_STEP_DATA_T *stepData){
    ADM_APC_STATE_E state = curState;
    switch(eventData->eventId){
	case ADM_APC_EVENT_ID_TRIGGER_TASK_DOWNLOAD:
	    apcStepDataAsyncCmd(stepData, ADM_APC_STEP_DATA_CMDID_DOWNLOAD, eventData);
	    state = ADM_APC_STATE_DOWNLOAD_DOING;
	    break;
	default:break;
    }
    return state;
}
static ADM_APC_STATE_E apcStepDownloadDoing(ADM_APC_STATE_E curState, const ADM_APC_EVENT_DATA_T *eventData, ADM_APC_STEP_DATA_T *stepData){
    ADM_APC_STATE_E state = curState;
    switch(eventData->eventId){
	case ADM_APC_EVENT_ID_POWER_ON:
	    apcStepDataAsyncCmd(stepData, ADM_APC_STEP_DATA_CMDID_DOWNLOAD_RESUME, eventData);
	    break;
	case ADM_APC_EVENT_ID_RESULT_DOWNLOAD_SUCCESS:
	    state = ADM_APC_STATE_UPGRADE_READY;
	    break;
	case ADM_APC_EVENT_ID_RESULT_DOWNLOAD_FATAL_ERROR:
	    apcEventSend(stepData->event, ADM_APC_EVENT_ID_STATUS_END);
	    state = ADM_APC_STATE_IDLE;
	    break;
	case ADM_APC_EVENT_ID_RESULT_DOWNLOAD_SHORT_ERROR:
	    state = ADM_APC_STATE_DOWNLOAD_PAUSE;
	    break;
	default:break;
    }
    return state;
}
static ADM_APC_STATE_E apcStepDownloadPause(ADM_APC_STATE_E curState, const ADM_APC_EVENT_DATA_T *eventData, ADM_APC_STEP_DATA_T *stepData){
    ADM_APC_STATE_E state = curState;
    switch(eventData->eventId){
	case ADM_APC_EVENT_ID_TRIGGER_TASK_DOWNLOAD:
	    apcStepDataAsyncCmd(stepData, ADM_APC_STEP_DATA_CMDID_DOWNLOAD, eventData);
	    state = ADM_APC_STATE_DOWNLOAD_DOING;
	    break;
	default:break;
    }
    return state;
}
static ADM_APC_STATE_E apcStepUpgradeReady(ADM_APC_STATE_E curState, const ADM_APC_EVENT_DATA_T *eventData, ADM_APC_STEP_DATA_T *stepData){
    ADM_APC_STATE_E state = curState;
    switch(eventData->eventId){
	case ADM_APC_EVENT_ID_TRIGGER_TASK_UPGRADE:
	    apcStepDataAsyncCmd(stepData, ADM_APC_STEP_DATA_CMDID_INSTALL, eventData);
	    state = ADM_APC_STATE_UPGRADE_DOING;
	    break;
	default:break;
    }
    return state;
}
static ADM_APC_STATE_E apcStepUpgradeDoing(ADM_APC_STATE_E curState, const ADM_APC_EVENT_DATA_T *eventData, ADM_APC_STEP_DATA_T *stepData){
    ADM_APC_STATE_E state = curState;
    switch(eventData->eventId){
	case ADM_APC_EVENT_ID_POWER_ON:
	    apcStepDataAsyncCmd(stepData, ADM_APC_STEP_DATA_CMDID_INSTALL_RESUME, eventData);
	    break;
	case ADM_APC_EVENT_ID_RESULT_UPGRADE_FINISH:
	    apcEventSend(stepData->event, ADM_APC_EVENT_ID_STATUS_END);
	    state = ADM_APC_STATE_IDLE;
	    break;
	case ADM_APC_EVENT_ID_RESULT_UPGRADE_SHORT_ERROR:
	    state = ADM_APC_STATE_UPGRADE_PAUSE;
	    break;
	case ADM_APC_EVENT_ID_RESULT_UPGRADE_BROKEN:
	    state = ADM_APC_STATE_UPGRADE_BROKEN;
	    break;
	default:break;
    }
    return state;
}
static ADM_APC_STATE_E apcStepUpgradePause(ADM_APC_STATE_E curState, const ADM_APC_EVENT_DATA_T *eventData, ADM_APC_STEP_DATA_T *stepData){
    ADM_APC_STATE_E state = curState;
    switch(eventData->eventId){
	case ADM_APC_EVENT_ID_TRIGGER_TASK_UPGRADE:
	    apcStepDataAsyncCmd(stepData, ADM_APC_STEP_DATA_CMDID_INSTALL, eventData);
	    state = ADM_APC_STATE_UPGRADE_DOING;
	    break;
	default:break;
    }
    return state;
}
static ADM_APC_STATE_E apcStepUpgradeBroken(ADM_APC_STATE_E curState, const ADM_APC_EVENT_DATA_T *eventData, ADM_APC_STEP_DATA_T *stepData){
    ADM_APC_STATE_E state = curState;
    switch(eventData->eventId){
	case ADM_APC_EVENT_ID_BROKEN_CLEAN_UP:
	    apcEventSend(stepData->event, ADM_APC_EVENT_ID_STATUS_END);
	    state = ADM_APC_STATE_IDLE;
	    break;
	default:break;
    }
    return state;
}

typedef struct {
    ADM_APC_EVENT_DATA_T eventData;
    ADM_APC_MANAGER_T *m;
}ADM_APC_MANAGER_WORKER_DATA_T;
static ADM_APC_MANAGER_WORKER_DATA_T *apcManagerWorkerDataNew(const ADM_APC_EVENT_DATA_T *eventData, ADM_APC_MANAGER_T *m){
    ADM_APC_MANAGER_WORKER_DATA_T *data = adm_malloc(sizeof(*data));
    if(data){
	apcEventDataInitDup(&data->eventData, eventData);
	data->m = m;
    }
    return data;
}
static void apcManagerWorkerDataDel(ADM_APC_MANAGER_WORKER_DATA_T *data){
    apcEventDataDeInit(&data->eventData);
    adm_free(data);
}
static void apcManagerWorkerEngine(void *param){
    ADM_APC_MANAGER_WORKER_DATA_T *data = (ADM_APC_MANAGER_WORKER_DATA_T *)param;
    ADM_APC_EVENT_DATA_T *eventData = &data->eventData;
    ADM_APC_MANAGER_T *m = data->m;
    if(m->isReady == 0){
	if(eventData->eventId == ADM_APC_EVENT_ID_POWER_ON){
	    m->isReady = 1;
	}
    }
    if(m->isReady == 1){
        ADM_APC_STATE_E curState = m->curState;
        m->curState = m->fnSteps[curState](curState, eventData, &m->stepData);
        log_d("state:[%s->%s]", apcStateString(curState), apcStateString(m->curState));
    }
    apcManagerWorkerDataDel(data);
}

/* ADM_APC_MANAGER_T event handle */
static void apcManagerEventHandle(const ADM_APC_EVENT_DATA_T *eventData, void *userData){
    ADM_APC_MANAGER_T *m = (ADM_APC_MANAGER_T *)userData;
    apcManagerWorkerAdd(m, eventData);
}

/* ADM_APC_MANAGER_T */
static void apcManagerInit(ADM_APC_MANAGER_T *m, ADM_APC_EVENT_T *event){
    static ADM_APC_STEP_F fnSteps[] = {
	apcStepIdle,
	apcStepCheckDoing,
	apcStepDownloadReady,
	apcStepDownloadDoing,
	apcStepDownloadPause,
	apcStepUpgradeReady,
	apcStepUpgradeDoing,
	apcStepUpgradePause,
	apcStepUpgradeBroken,
    };
    m->isReady = 0;
    m->curState = ADM_APC_STATE_IDLE;
    m->fnSteps = fnSteps;
    apcStepDataInit(&m->stepData, event);
    m->wq = NULL;
    apcEventSub(event, ADM_APC_EVENT_ID_POWER_ON, apcManagerEventHandle, m);
    apcEventSub(event, ADM_APC_EVENT_ID_TRIGGER_TASK_CHECK, apcManagerEventHandle, m);
    apcEventSub(event, ADM_APC_EVENT_ID_TRIGGER_TASK_DOWNLOAD, apcManagerEventHandle, m);
    apcEventSub(event, ADM_APC_EVENT_ID_TRIGGER_TASK_UPGRADE, apcManagerEventHandle, m);
    apcEventSub(event, ADM_APC_EVENT_ID_RESULT_CHECK_NO_NEW_VERSION, apcManagerEventHandle, m);
    apcEventSub(event, ADM_APC_EVENT_ID_RESULT_CHECK_FIND_NEW_VERSION, apcManagerEventHandle, m);
    apcEventSub(event, ADM_APC_EVENT_ID_RESULT_DOWNLOAD_SUCCESS, apcManagerEventHandle, m);
    apcEventSub(event, ADM_APC_EVENT_ID_RESULT_DOWNLOAD_FATAL_ERROR, apcManagerEventHandle, m);
    apcEventSub(event, ADM_APC_EVENT_ID_RESULT_DOWNLOAD_SHORT_ERROR, apcManagerEventHandle, m);
    apcEventSub(event, ADM_APC_EVENT_ID_RESULT_UPGRADE_FINISH, apcManagerEventHandle, m);
    apcEventSub(event, ADM_APC_EVENT_ID_RESULT_UPGRADE_SHORT_ERROR, apcManagerEventHandle, m);
    apcEventSub(event, ADM_APC_EVENT_ID_RESULT_UPGRADE_BROKEN, apcManagerEventHandle, m);
    apcEventSub(event, ADM_APC_EVENT_ID_BROKEN_CLEAN_UP, apcManagerEventHandle, m);
    apcEventSub(event, ADM_APC_EVENT_ID_STATUS_END, apcManagerEventHandle, m);
}
static void apcManagerDeInit(ADM_APC_MANAGER_T *m){
    if(m->wq){adm_workerQueueQuit(m->wq);adm_workerQueueDel(m->wq);}
    apcStepDataDeInit(&m->stepData);
}
static void apcManagerWorkerAdd(ADM_APC_MANAGER_T *m, const ADM_APC_EVENT_DATA_T *eventData){
    if(eventData->eventId == ADM_APC_EVENT_ID_STATUS_END){
	if(m->wq){
	    adm_workerQueueQuit(m->wq);
	    adm_workerQueueDel(m->wq);
	    m->wq = NULL;
	}
    }else{
        if(m->wq == NULL){
            m->wq = adm_workerQueueNew(apcManagerWorkerEngine);
        }
        if(m->wq == NULL){}else{
            ADM_APC_MANAGER_WORKER_DATA_T *data = apcManagerWorkerDataNew(eventData, m);
            if(data){
                adm_workerQueueAdd(m->wq, data);
            }
        }
    }
}

typedef struct {
    ADM_APC_STEP_DATA_CMDID_E cmdId;
    ADM_APC_EVENT_DATA_T eventData;
    ADM_APC_STEP_DATA_T *s;
}ADM_APC_STEP_DATA_ASYNC_DATA_T;
static ADM_APC_STEP_DATA_ASYNC_DATA_T *apcStepDataAsyncDataNew(ADM_APC_STEP_DATA_CMDID_E cmdId, const ADM_APC_EVENT_DATA_T *eventData, ADM_APC_STEP_DATA_T *s){
    ADM_APC_STEP_DATA_ASYNC_DATA_T *data = adm_malloc(sizeof(*data));
    if(data){
	data->cmdId = cmdId;
	apcEventDataInitDup(&data->eventData, eventData);
	data->s = s;
    }
    return data;
}
static void apcStepDataAsyncDataDel(ADM_APC_STEP_DATA_ASYNC_DATA_T *data){
    apcEventDataDeInit(&data->eventData);
    adm_free(data);
}

/* ADM_APC_STEP_DATA_T */
static void apcStepDataInit(ADM_APC_STEP_DATA_T *s, ADM_APC_EVENT_T *event){
    pthread_mutex_init(&s->mutex, NULL);
    adm_apc_cvmInit(&s->cvm);
    adm_apc_dpmInit(&s->dpm);
    adm_apc_ipmInit(&s->ipm);
    s->event = event;
}
static void apcStepDataDeInit(ADM_APC_STEP_DATA_T *s){
    adm_apc_cvmDeInit(&s->cvm);
    adm_apc_dpmDeInit(&s->dpm);
    adm_apc_ipmDeInit(&s->ipm);
    pthread_mutex_destroy(&s->mutex);
}
static void apcStepDataCheck(ADM_APC_STEP_DATA_T *s){
    DBOOL dRet = adm_apc_cvmCheck(&s->cvm);
    if(dRet == 0){
	apcEventSend(s->event, ADM_APC_EVENT_ID_RESULT_CHECK_NO_NEW_VERSION);
    }else if(dRet == 1){
	apcEventSend(s->event, ADM_APC_EVENT_ID_RESULT_CHECK_FIND_NEW_VERSION);
    }
}
static void apcStepDataCheckResume(ADM_APC_STEP_DATA_T *s){
    apcStepDataCheck(s);
}
static void apcStepDataDownload(ADM_APC_STEP_DATA_T *s){
    ADM_APC_DPM_RESULT_E eret = adm_apc_dpmDownload(&s->dpm);
    if(eret == ADM_APC_DPM_RESULT_SUCCESS){
	apcEventSend(s->event, ADM_APC_EVENT_ID_RESULT_DOWNLOAD_SUCCESS);
    }else if(eret == ADM_APC_DPM_RESULT_FATAL_ERROR){
	apcEventSend(s->event, ADM_APC_EVENT_ID_RESULT_DOWNLOAD_FATAL_ERROR);
    }else if(eret == ADM_APC_DPM_RESULT_SHORT_ERROR){
	apcEventSend(s->event, ADM_APC_EVENT_ID_RESULT_DOWNLOAD_SHORT_ERROR);
    }
}
static void apcStepDataDownloadResume(ADM_APC_STEP_DATA_T *s){
    apcStepDataDownload(s);
}
static void apcStepDataInstall(ADM_APC_STEP_DATA_T *s){
    ADM_APC_IPM_RESULT_E eret = adm_apc_ipmInstall(&s->ipm);
    if(eret == ADM_APC_IPM_RESULT_FINISH){
	apcEventSend(s->event, ADM_APC_EVENT_ID_RESULT_UPGRADE_FINISH);
    }else if(eret == ADM_APC_IPM_RESULT_SHORT_ERROR){
	apcEventSend(s->event, ADM_APC_EVENT_ID_RESULT_UPGRADE_SHORT_ERROR);
    }else if(eret == ADM_APC_IPM_RESULT_BROKEN){
	apcEventSend(s->event, ADM_APC_EVENT_ID_RESULT_UPGRADE_BROKEN);
    }
}
static void apcStepDataInstallResume(ADM_APC_STEP_DATA_T *s){
    apcStepDataInstall(s);
}
static void* apcStepDataAsyncCmdHandle(void *argv){
    ADM_APC_STEP_DATA_ASYNC_DATA_T *data = (ADM_APC_STEP_DATA_ASYNC_DATA_T *)argv;
    ADM_APC_STEP_DATA_T *s = data->s;
    switch(data->cmdId){
	case ADM_APC_STEP_DATA_CMDID_CHECK:
	    apcStepDataCheck(s);
	    break;
	case ADM_APC_STEP_DATA_CMDID_CHECK_RESUME:
	    apcStepDataCheckResume(s);
	    break;
	case ADM_APC_STEP_DATA_CMDID_DOWNLOAD:
	    apcStepDataDownload(s);
	    break;
	case ADM_APC_STEP_DATA_CMDID_DOWNLOAD_RESUME:
	    apcStepDataDownloadResume(s);
	    break;
	case ADM_APC_STEP_DATA_CMDID_INSTALL:
	    apcStepDataInstall(s);
	    break;
	case ADM_APC_STEP_DATA_CMDID_INSTALL_RESUME:
	    apcStepDataInstallResume(s);
	    break;
	default:break;
    }
    apcStepDataAsyncDataDel(data);
    pthread_mutex_unlock(&s->mutex);
    return NULL;
}
static void apcStepDataAsyncCmd(ADM_APC_STEP_DATA_T *s, ADM_APC_STEP_DATA_CMDID_E cmdId, const ADM_APC_EVENT_DATA_T *eventData){
    log_d("cmdId %d", cmdId);
    ADM_APC_STEP_DATA_ASYNC_DATA_T *data = apcStepDataAsyncDataNew(cmdId, eventData, s);
    if(data){
	DBOOL isOk = 0;
        if(pthread_mutex_trylock(&s->mutex) == 0){
            pthread_t pth;
            if(pthread_create(&pth, NULL, apcStepDataAsyncCmdHandle, data) == 0){
		pthread_detach(pth);
		isOk = 1;
	    }else{
                log_w("step async thread create failed");
                pthread_mutex_unlock(&s->mutex);
            }
        }else{
            log_w("step async trylock failed");
        }
	if(isOk == 0){
	    apcStepDataAsyncDataDel(data);
	}
    }
}

/* ADM_APC_STATE_E */
static const DCHAR *apcStateString(ADM_APC_STATE_E state){
    const DCHAR *strState = "ADM_APC_STATE_UNKNOWN";
    switch(state){
	case ADM_APC_STATE_IDLE:strState = ADM_STR_FROM_PARAM(ADM_APC_STATE_IDLE);break;
	case ADM_APC_STATE_CHECK_DOING:strState = ADM_STR_FROM_PARAM(ADM_APC_STATE_CHECK_DOING);break;
	case ADM_APC_STATE_DOWNLOAD_READY:strState = ADM_STR_FROM_PARAM(ADM_APC_STATE_DOWNLOAD_READY);break;
	case ADM_APC_STATE_DOWNLOAD_DOING:strState = ADM_STR_FROM_PARAM(ADM_APC_STATE_DOWNLOAD_DOING);break;
	case ADM_APC_STATE_DOWNLOAD_PAUSE:strState = ADM_STR_FROM_PARAM(ADM_APC_STATE_DOWNLOAD_PAUSE);break;
	case ADM_APC_STATE_UPGRADE_READY:strState = ADM_STR_FROM_PARAM(ADM_APC_STATE_UPGRADE_READY);break;
	case ADM_APC_STATE_UPGRADE_DOING:strState = ADM_STR_FROM_PARAM(ADM_APC_STATE_UPGRADE_DOING);break;
	case ADM_APC_STATE_UPGRADE_PAUSE:strState = ADM_STR_FROM_PARAM(ADM_APC_STATE_UPGRADE_PAUSE);break;
	case ADM_APC_STATE_UPGRADE_BROKEN:strState = ADM_STR_FROM_PARAM(ADM_APC_STATE_UPGRADE_BROKEN);break;
	default:break;
    }
    return strState;
}

typedef struct {
    ADM_APC_EVENT_DATA_T eventData;
    ADM_APC_EVENT_T *event;
}ADM_APC_EVENT_ENGINE_DATA_T;
static ADM_APC_EVENT_ENGINE_DATA_T *apcEventEngineDataNew(const ADM_APC_EVENT_DATA_T *eventData, ADM_APC_EVENT_T *event){
    ADM_APC_EVENT_ENGINE_DATA_T *data = adm_malloc(sizeof(*data));
    if(data){
	apcEventDataInitDup(&data->eventData, eventData);
	data->event = event;
    }
    return data;
}
static void apcEventEngineDataDel(ADM_APC_EVENT_ENGINE_DATA_T *data){
    apcEventDataDeInit(&data->eventData);
    adm_free(data);
}

/* ADM_APC_EVENT_T engine */
static void apcEventEngine(void *param){
    ADM_APC_EVENT_ENGINE_DATA_T *data = (ADM_APC_EVENT_ENGINE_DATA_T *)param;
    apcEventSendData(data->event, &data->eventData);
    apcEventEngineDataDel(data);
}

/* ADM_APC_EVENT_T */
static void apcEventInit(ADM_APC_EVENT_T *event){
    event->wq = adm_workerQueueNew(apcEventEngine);
    pthread_mutex_init(&event->mutex, NULL);
    INIT_LIST_HEAD(&event->head);
    event->indexId = 0;
}
static void apcEventDeInit(ADM_APC_EVENT_T *event){
    if(event->wq){adm_workerQueueDel(event->wq);}
    pthread_mutex_destroy(&event->mutex);
}
static void apcEventStop(ADM_APC_EVENT_T *event){
    pthread_mutex_lock(&event->mutex);
    ADM_APC_EVENT_NODE_T *p, *n;
    list_for_each_entry_safe(p, n, &event->head, ADM_APC_EVENT_NODE_T, entry){
	list_del(&p->entry);
	apcEventNodeDel(p);
    }
    pthread_mutex_unlock(&event->mutex);
    if(event->wq){adm_workerQueueQuit(event->wq);}
}
static D64S apcEventSub(ADM_APC_EVENT_T *event, ADM_APC_EVENT_ID_E eventId, ADM_APC_EVENT_F *fnHandle, void *userData){
    pthread_mutex_lock(&event->mutex);
    D64S indexId = event->indexId++;
    ADM_APC_EVENT_NODE_T* node = apcEventNodeNew(indexId, eventId, fnHandle, userData);
    if(node){
	list_add_tail(&node->entry, &event->head);
    }
    pthread_mutex_unlock(&event->mutex);
    return indexId;
}
static void apcEventUnSub(ADM_APC_EVENT_T *event, D64S indexId){
    pthread_mutex_lock(&event->mutex);
    ADM_APC_EVENT_NODE_T *p, *n;
    list_for_each_entry_safe(p, n, &event->head, ADM_APC_EVENT_NODE_T, entry){
	if(p->indexId == indexId){
	    list_del(&p->entry);
	    apcEventNodeDel(p);
	    break;
	}
    }
    pthread_mutex_unlock(&event->mutex);
}
static void apcEventSendData(ADM_APC_EVENT_T *event, const ADM_APC_EVENT_DATA_T* eventData){
    log_d("eventId:%s", apcEventIdString(eventData->eventId));
    pthread_mutex_lock(&event->mutex);
    ADM_APC_EVENT_NODE_T *p;
    list_for_each_entry(p, &event->head, ADM_APC_EVENT_NODE_T, entry){
	if(p->eventId == eventData->eventId){
	    if(p->fnHandle){p->fnHandle(eventData, p->userData);}
	}
    }
    pthread_mutex_unlock(&event->mutex);
}
static void apcEventSendDataAsync(ADM_APC_EVENT_T *event, const ADM_APC_EVENT_DATA_T* eventData){
    if(event->wq){
	ADM_APC_EVENT_ENGINE_DATA_T *data = apcEventEngineDataNew(eventData, event);
	if(data){
	    adm_workerQueueAdd(event->wq, data);
	}
    }
}
static void apcEventSend(ADM_APC_EVENT_T *event, ADM_APC_EVENT_ID_E eventId){
    ADM_APC_EVENT_DATA_T eventData;
    if(apcEventDataInit(&eventData, eventId)){
        apcEventSendDataAsync(event, &eventData);
        apcEventDataDeInit(&eventData);
    }
}
#if 0
static void apcEventSendTrigger(ADM_APC_EVENT_T *event, ADM_APC_EVENT_ID_E eventId, const ADM_APC_EVENT_DATA_TRIGGER_T *trigger){
    ADM_APC_EVENT_DATA_T eventData;
    if(apcEventDataInitTrigger(&eventData, eventId, trigger)){
        apcEventSendDataAsync(event, &eventData);
        apcEventDataDeInit(&eventData);
    }
}
static void apcEventSendFlashEcus(ADM_APC_EVENT_T *event, ADM_APC_EVENT_ID_E eventId, const ADM_APC_EVENT_DATA_FLASH_ECUS_T *flashEcus){
    ADM_APC_EVENT_DATA_T eventData;
    if(apcEventDataInitFlashEcus(&eventData, eventId, flashEcus)){
        apcEventSendDataAsync(event, &eventData);
        apcEventDataDeInit(&eventData);
    }
}
#endif

/* ADM_APC_EVENT_NODE_T */
static ADM_APC_EVENT_NODE_T* apcEventNodeNew(D64S indexId, ADM_APC_EVENT_ID_E eventId, ADM_APC_EVENT_F *fnHandle, void *userData){
    ADM_APC_EVENT_NODE_T* node = adm_malloc(sizeof(*node));
    if(node){
	node->indexId = indexId;
	node->eventId = eventId;
	node->fnHandle = fnHandle;
	node->userData = userData;
    }
    return node;
}
static void apcEventNodeDel(ADM_APC_EVENT_NODE_T* node){
    adm_free(node);
}

/* ADM_APC_EVENT_DATA_T */
static DBOOL apcEventDataInit(ADM_APC_EVENT_DATA_T *data, ADM_APC_EVENT_ID_E eventId){
    DBOOL dRet = 0;
    if(/*ipmEventDataIsTrigger(eventId)
	    || ipmEventDataIsFlashEcus(eventId) */0
	    ){}else{
	data->eventId = eventId;
	dRet = 1;
    }
    return dRet;
}
static void apcEventDataInitDup(ADM_APC_EVENT_DATA_T *data, const ADM_APC_EVENT_DATA_T *src){
    data->eventId = src->eventId;
/*
    if(ipmEventDataIsTrigger(src->eventId)){
	ipmEventDataTriggerInitDup(&data->u.trigger, &src->u.trigger);
    }else if(ipmEventDataIsFlashEcus(src->eventId)){
	ipmEventDataFlashEcusInitDup(&data->u.flashEcus, &src->u.flashEcus);
    }
*/
}
static void apcEventDataDeInit(ADM_APC_EVENT_DATA_T *data){
/*
    if(ipmEventDataIsTrigger(data->eventId)){
	ipmEventDataTriggerDeInit(&data->u.trigger);
    }else if(ipmEventDataIsFlashEcus(data->eventId)){
	ipmEventDataFlashEcusDeInit(&data->u.flashEcus);
    }
*/
}

/* ADM_APC_EVENT_ID_E */
static const DCHAR *apcEventIdString(ADM_APC_EVENT_ID_E eventId){
    const DCHAR *strEventId = "ADM_APC_EVENT_ID_POWER_UNKNOWN";
    switch(eventId){
	case ADM_APC_EVENT_ID_POWER_ON:strEventId = ADM_STR_FROM_PARAM(ADM_APC_EVENT_ID_POWER_ON);break;
	case ADM_APC_EVENT_ID_POWER_OFF:strEventId = ADM_STR_FROM_PARAM(ADM_APC_EVENT_ID_POWER_OFF);break;
	case ADM_APC_EVENT_ID_TRIGGER_TASK_CHECK:strEventId = ADM_STR_FROM_PARAM(ADM_APC_EVENT_ID_TRIGGER_TASK_CHECK);break;
	case ADM_APC_EVENT_ID_TRIGGER_TASK_DOWNLOAD:strEventId = ADM_STR_FROM_PARAM(ADM_APC_EVENT_ID_TRIGGER_TASK_DOWNLOAD);break;
	case ADM_APC_EVENT_ID_TRIGGER_TASK_UPGRADE:strEventId = ADM_STR_FROM_PARAM(ADM_APC_EVENT_ID_TRIGGER_TASK_UPGRADE);break;
	case ADM_APC_EVENT_ID_RESULT_CHECK_NO_NEW_VERSION:strEventId = ADM_STR_FROM_PARAM(ADM_APC_EVENT_ID_RESULT_CHECK_NO_NEW_VERSION);break;
	case ADM_APC_EVENT_ID_RESULT_CHECK_FIND_NEW_VERSION:strEventId = ADM_STR_FROM_PARAM(ADM_APC_EVENT_ID_RESULT_CHECK_FIND_NEW_VERSION);break;
	case ADM_APC_EVENT_ID_RESULT_DOWNLOAD_SUCCESS:strEventId = ADM_STR_FROM_PARAM(ADM_APC_EVENT_ID_RESULT_DOWNLOAD_SUCCESS);break;
	case ADM_APC_EVENT_ID_RESULT_DOWNLOAD_FATAL_ERROR:strEventId = ADM_STR_FROM_PARAM(ADM_APC_EVENT_ID_RESULT_DOWNLOAD_FATAL_ERROR);break;
	case ADM_APC_EVENT_ID_RESULT_DOWNLOAD_SHORT_ERROR:strEventId = ADM_STR_FROM_PARAM(ADM_APC_EVENT_ID_RESULT_DOWNLOAD_SHORT_ERROR);break;
	case ADM_APC_EVENT_ID_RESULT_UPGRADE_FINISH:strEventId = ADM_STR_FROM_PARAM(ADM_APC_EVENT_ID_RESULT_UPGRADE_FINISH);break;
	case ADM_APC_EVENT_ID_RESULT_UPGRADE_SHORT_ERROR:strEventId = ADM_STR_FROM_PARAM(ADM_APC_EVENT_ID_RESULT_UPGRADE_SHORT_ERROR);break;
	case ADM_APC_EVENT_ID_RESULT_UPGRADE_BROKEN:strEventId = ADM_STR_FROM_PARAM(ADM_APC_EVENT_ID_RESULT_UPGRADE_BROKEN);break;
	case ADM_APC_EVENT_ID_BROKEN_CLEAN_UP:strEventId = ADM_STR_FROM_PARAM(ADM_APC_EVENT_ID_BROKEN_CLEAN_UP);break;
	case ADM_APC_EVENT_ID_STATUS_END:strEventId = ADM_STR_FROM_PARAM(ADM_APC_EVENT_ID_STATUS_END);break;
	default:break;
    }
    return strEventId;
}
