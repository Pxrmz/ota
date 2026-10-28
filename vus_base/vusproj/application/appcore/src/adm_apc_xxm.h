#ifndef __ADM_APC_XXM_H_
#define __ADM_APC_XXM_H_

#include "adm_typedefs.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    void *data;
}ADM_APC_CVM_T;
void adm_apc_cvmInit(ADM_APC_CVM_T *cvm);
void adm_apc_cvmDeInit(ADM_APC_CVM_T *cvm);
/*! return DBOOL 0.no new version, 1.find new version */
DBOOL adm_apc_cvmCheck(ADM_APC_CVM_T *cvm/* TODO cmdId */);
DBOOL adm_apc_cvmCheckResume(ADM_APC_CVM_T *cvm);

typedef struct {
    void *data;
}ADM_APC_DPM_T;
void adm_apc_dpmInit(ADM_APC_DPM_T *dpm);
void adm_apc_dpmDeInit(ADM_APC_DPM_T *dpm);
typedef enum{
    ADM_APC_DPM_RESULT_SUCCESS = 0,
    ADM_APC_DPM_RESULT_FATAL_ERROR,
    ADM_APC_DPM_RESULT_SHORT_ERROR,
}ADM_APC_DPM_RESULT_E;
ADM_APC_DPM_RESULT_E adm_apc_dpmDownload(ADM_APC_DPM_T *dpm/* TODO cmdId */);
ADM_APC_DPM_RESULT_E adm_apc_dpmDownloadResume(ADM_APC_DPM_T *dpm);

typedef struct {
    void *data;
}ADM_APC_IPM_T;
void adm_apc_ipmInit(ADM_APC_IPM_T *ipm);
void adm_apc_ipmDeInit(ADM_APC_IPM_T *ipm);
typedef enum {
    ADM_APC_IPM_RESULT_FINISH = 0,
    ADM_APC_IPM_RESULT_SHORT_ERROR,
    ADM_APC_IPM_RESULT_BROKEN,
}ADM_APC_IPM_RESULT_E;
ADM_APC_IPM_RESULT_E adm_apc_ipmInstall(ADM_APC_IPM_T *ipm/* TODO cmdId */);
ADM_APC_IPM_RESULT_E adm_apc_ipmInstallResume(ADM_APC_IPM_T *ipm);

typedef struct {
    void (*fnTriggerCheck)(void *priv);
    void (*fnTriggerDownload)(void *priv);
    void (*fnTriggerInstall)(void *priv);
    void *priv;
}ADM_APC_UIM_T;
void adm_apc_uimInit(ADM_APC_UIM_T *uim);
void adm_apc_uimDeInit(ADM_APC_UIM_T *uim);


#ifdef __cplusplus
}
#endif

#endif//__ADM_APC_XXM_H_
