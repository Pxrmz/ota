#include "adm_apc_xxm.h"

#if 1

#include "adm_ipm_install.h"

/* ADM_APC_CVM_T */
void adm_apc_cvmInit(ADM_APC_CVM_T *cvm){
}
void adm_apc_cvmDeInit(ADM_APC_CVM_T *cvm){
}
/*! return DBOOL 0.no new version, 1.find new version */
DBOOL adm_apc_cvmCheck(ADM_APC_CVM_T *cvm){
    return 1;
}
DBOOL adm_apc_cvmCheckResume(ADM_APC_CVM_T *cvm){
    return 0;
}

/* ADM_APC_DPM_T */
void adm_apc_dpmInit(ADM_APC_DPM_T *dpm){
}
void adm_apc_dpmDeInit(ADM_APC_DPM_T *dpm){
}
ADM_APC_DPM_RESULT_E adm_apc_dpmDownload(ADM_APC_DPM_T *dpm){
    return ADM_APC_DPM_RESULT_SUCCESS;
}
ADM_APC_DPM_RESULT_E adm_apc_dpmDownloadResume(ADM_APC_DPM_T *dpm){
    return ADM_APC_DPM_RESULT_FATAL_ERROR;
}

/* ADM_APC_IPM_T */
void adm_apc_ipmInit(ADM_APC_IPM_T *ipm){
    adm_ipm_init();//TODO move to appcore
}
void adm_apc_ipmDeInit(ADM_APC_IPM_T *ipm){
    adm_ipm_deInit();//TODO move to appcore
}
ADM_APC_IPM_RESULT_E adm_apc_ipmInstall(ADM_APC_IPM_T *ipm){
    D32S ret = adm_ipm_install();
    //TODO ret
    return ADM_APC_IPM_RESULT_FINISH;
}
ADM_APC_IPM_RESULT_E adm_apc_ipmInstallResume(ADM_APC_IPM_T *ipm){
    return ADM_APC_IPM_RESULT_BROKEN;
}

/* ADM_APC_UIM_T */
void adm_apc_uimInit(ADM_APC_UIM_T *uim){
    adm_ipm_uimSetCallback(uim->fnTriggerInstall, uim->priv);
}
void adm_apc_uimDeInit(ADM_APC_UIM_T *uim){
}

#else
/***************************************************************************************************************/
/* examples */

#include <pthread.h>
#include <unistd.h>

/* ADM_APC_CVM_T */
void adm_apc_cvmInit(ADM_APC_CVM_T *cvm){
}
void adm_apc_cvmDeInit(ADM_APC_CVM_T *cvm){
}
/*! return DBOOL 0.no new version, 1.find new version */
DBOOL adm_apc_cvmCheck(ADM_APC_CVM_T *cvm){
    return 1;
}
DBOOL adm_apc_cvmCheckResume(ADM_APC_CVM_T *cvm){
    return 0;
}

/* ADM_APC_DPM_T */
void adm_apc_dpmInit(ADM_APC_DPM_T *dpm){
}
void adm_apc_dpmDeInit(ADM_APC_DPM_T *dpm){
}
ADM_APC_DPM_RESULT_E adm_apc_dpmDownload(ADM_APC_DPM_T *dpm){
    return ADM_APC_DPM_RESULT_SUCCESS;
}
ADM_APC_DPM_RESULT_E adm_apc_dpmDownloadResume(ADM_APC_DPM_T *dpm){
    return ADM_APC_DPM_RESULT_FATAL_ERROR;
}

/* ADM_APC_IPM_T */
void adm_apc_ipmInit(ADM_APC_IPM_T *ipm){
}
void adm_apc_ipmDeInit(ADM_APC_IPM_T *ipm){
}
ADM_APC_IPM_RESULT_E adm_apc_ipmInstall(ADM_APC_IPM_T *ipm){
    return ADM_APC_IPM_RESULT_FINISH;
}
ADM_APC_IPM_RESULT_E adm_apc_ipmInstallResume(ADM_APC_IPM_T *ipm){
    return ADM_APC_IPM_RESULT_BROKEN;
}

/* uimTrigger handle */
static void* uimTriggerHandle(void* argv){
    ADM_APC_UIM_T *uim = (ADM_APC_UIM_T *)argv;
    uim->fnTriggerCheck(uim->priv);
    usleep(100 *1000);
    uim->fnTriggerDownload(uim->priv);
    usleep(100 *1000);
    uim->fnTriggerInstall(uim->priv);
    return NULL;
}

/* ADM_APC_UIM_T */
void adm_apc_uimInit(ADM_APC_UIM_T *uim){
    static ADM_APC_UIM_T _uim;
    _uim.fnTriggerCheck = uim->fnTriggerCheck;
    _uim.fnTriggerDownload = uim->fnTriggerDownload;
    _uim.fnTriggerInstall = uim->fnTriggerInstall;
    _uim.priv = uim->priv;
    pthread_t pth;
    pthread_create(&pth, NULL, uimTriggerHandle, &_uim);
    pthread_detach(pth);
}
void adm_apc_uimDeInit(ADM_APC_UIM_T *uim){
}
/***************************************************************************************************************/
#endif
