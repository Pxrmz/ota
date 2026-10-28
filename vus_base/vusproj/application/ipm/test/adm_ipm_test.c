#include "adm_ipm_install.h"
#include "adm_ips_install.h"
#include "adm_pis_package.h"
#include "adm_debug.h"
#include "adm_com_lib.h"
//#include "adm_dim_soaHost.h"
//#include "adm_dim_soaUcl.h"
//#include "adm_dim_soaVdi.h"
#include "adm_soa.h"
#include "adm_cvs_versionPolicy.h"

#include "adm_uim_install.h"
#include "adm_vrs_registerInfo.h"
#include "adm_ocs_airProtocolSecurity.h"
#include "adm_ocs_airProtocolAdapter.h"
#include "adm_irs_upload.h"
#include "adm_ecs_rsConfig.h"
#include "adm_typedefs.h"
#include "adm_memory.h"
#include "adm_ipm_test_log.h"
//#include "adm_soa.h"
#include "adm_vsm_fsm.h"
#include "adm_cdm_verify_interface.h"
#include "adm_ocm_interface.h"
#include "adm_irm_report.h"
#include "adm_irm_logFile.h"
#include "adm_eim_ecuInfoManager.h"
#include "adm_file.h"
#include "adm_dim_common.h"
#include "adm_uim_download.h"
#include "adm_cfg_api.h"

#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include "json-c/json.h"


#define ADM_IPM_TEST_ENV_DIR    "." 
#define ADM_IPM_TEST_DATA_DIR   ADM_IPM_TEST_ENV_DIR"/data"


typedef struct
{
    DCHAR ecuReqId[32];
    DCHAR ecuSwDid[16];
} IPM_ECU_ID_T;

static D32S ipm_getInstallStage();

static json_object* s_ecusInfosOri = NULL;
static json_object* s_ecusInfosLastest = NULL;
//
//static D32S ecusDidsOriInit(const DCHAR* str)
//{
//    s_ecusInfosOri = json_tokener_parse(str);
//    return ADM_ECODE_SUCCESSED;
//}
//
//static void ecusDidsOriDestory(const DCHAR* str)
//{
//    json_object_put(s_ecusInfosOri);
//}

static json_object* ecusDidsInit(const DCHAR* str)
{
    return json_tokener_parse(str);
}

static void ecusDidsDestory(json_object* root)
{
    json_object_put(root);
}

static const ECU_INFO_T* ecusDidDataGetFromJson(const json_object* root, ADM_DIM_SOA_DID_DATA_T* didData)
{
    didData->did = htoi((char *)json_object_get_string(json_object_object_get(root, "did")));
    didData->didData = adm_malloc(sizeof(*didData->didData));
    didData->didData->dataType = 6;    // VAR_STRING
    didData->didData->v.valueS = adm_strdup(json_object_get_string(json_object_object_get(root, "didValue")));
    didData->startByte = 0;
    didData->endByte = 0;
    didData->errorCode = 0;
}

static const ECU_INFO_T* ecuInfoGetEcuInfoFromJson(const json_object* root, ECU_INFO_T* ecuInfo)
{
    const json_object* did = NULL;
    ADM_DIM_SOA_DID_DATA_ARRAY_T* didDataArr = NULL;

    D32S didCount = json_object_array_length(root);
    if (didCount <= 0)
    {
        return NULL;
    }

    didDataArr = adm_malloc(sizeof(*didDataArr));
    didDataArr->didArray = adm_malloc(sizeof(didDataArr->didArray[0]) * didCount);
    for (D32S i = 0; i < didCount; i++) 
    {
        did = json_object_array_get_idx(root, i);
        didDataArr->didArray[i] = adm_malloc(sizeof(*didDataArr->didArray[0]) * didCount);
        ecusDidDataGetFromJson(did, didDataArr->didArray[i]);
    }

    didDataArr->didArrayLength = didCount;
    ecuInfo->vdiDidListResponse = didDataArr;

    return ecuInfo;
}

static const ECU_INFO_T* ecuInfoGetByReqIdFromJson(const json_object* root, const DCHAR* ecuReqId)
{
    ECU_INFO_T* ecuInfo = NULL;
    json_object* ecuDids = NULL;
    json_object* obj = NULL;
    D32S ecuCount = 0;

    ecuCount = json_object_array_length(root);
    for (D32S i = 0; i < ecuCount; i++)
    {
        ecuDids = json_object_array_get_idx(root, i);
        obj = json_object_object_get(ecuDids, "ecuReqId");
        if (0 == adm_strcasecmp(ecuReqId, json_object_get_string(obj)))
        {
            ecuInfo = adm_malloc(sizeof(*ecuInfo));
            ecuInfoGetEcuInfoFromJson(json_object_object_get(ecuDids, "dids"), ecuInfo);
            ecuInfo->ecuReqId = adm_strdup(ecuReqId);
            ecuInfo->ecuSwDid = adm_strdup(json_object_get_string(json_object_object_get(ecuDids, "ecuSwDid")));
            break;
        }
    }

    return ecuInfo;
}

static const ECU_INFO_T* ecuInfoGetByReqId(const DCHAR * ecuReqId, const DCHAR* ecuSwDid)
{
    const ECU_INFO_T* ecuInfo = NULL;
    ADM_IPS_ECU_PROGRESS_T progress;
    adm_ips_ecuProgressInit(&progress);
    adm_ips_ecuProgressGet(&progress, ecuReqId, ecuSwDid);
    ADM_IPS_ECU_PHASE_E state = progress.state;
    adm_ips_ecuProgressDeInit(&progress);

    if (ADM_IPS_ECU_PHASE_POST_INSTALL == state || 
        ADM_IPS_ECU_PHASE_POST_INSTALL_ROLLBACK == state || 
        ADM_IPS_ECU_PHASE_POST_INSTALL_JUMP == state)
    {
        ecuInfo = ecuInfoGetByReqIdFromJson(s_ecusInfosLastest, ecuReqId);
    }
    else 
    {
        ecuInfo = ecuInfoGetByReqIdFromJson(s_ecusInfosOri, ecuReqId);
    }

    return ecuInfo;
}


typedef struct
{
    DCHAR* ecuReqId;
    DCHAR* ecuSwDid;
    DCHAR* failReason;
    D32S upgradeTryTimes;
    D32S upgradeRes;
    D32S rollbackTryTimes;
    D32S rollbackRes;
} IPM_ECU_INSTALL_CONTROL_T;

typedef struct
{
    D32S num;
    IPM_ECU_INSTALL_CONTROL_T* dues;
} IPM_ECU_INSTALL_CTR_LIST_T;

IPM_ECU_INSTALL_CTR_LIST_T s_ecuInstallCtrList;

void ipmEcuInstallCtrInitByJson(const json_object* root, IPM_ECU_INSTALL_CONTROL_T* due)
{
    due->ecuReqId = adm_strdup(json_object_get_string(json_object_object_get(root, "ecuReqId")));
    due->ecuSwDid = adm_strdup(json_object_get_string(json_object_object_get(root, "ecuSwDid")));
    due->failReason = adm_strdup(json_object_get_string(json_object_object_get(root, "failReason")));
    due->upgradeTryTimes = json_object_get_int(json_object_object_get(root, "upgradeTryTimes"));
    due->upgradeRes = json_object_get_int(json_object_object_get(root, "upgradeRes"));
    due->rollbackTryTimes = json_object_get_int(json_object_object_get(root, "rollbackTryTimes"));
    due->rollbackRes = json_object_get_int(json_object_object_get(root, "rollbackTryRes"));
}

D32S ipmEcuInstallCtrsInit(const DCHAR* str, IPM_ECU_INSTALL_CTR_LIST_T* dueList)
{
    IPM_ECU_INSTALL_CONTROL_T* dues = NULL;
    D32S count = 0;
    json_object* root = json_tokener_parse(str);

    count = json_object_array_length(root);
    dues = adm_malloc(sizeof(dues[0]) * count);
    for (D32S i = 0; i < count; i++)
    {
        ipmEcuInstallCtrInitByJson(json_object_array_get_idx(root, i), &dues[i]);
    }

    dueList->dues = dues;
    dueList->num = count;

    return ADM_ECODE_SUCCESSED;
}

void ipmEcuInstallCtrsDestroy(IPM_ECU_INSTALL_CTR_LIST_T* dueList)
{
    IPM_ECU_INSTALL_CONTROL_T* due = NULL;
    for (D32S i = 0; i < dueList->num; i++)
    {
        due = &dueList->dues[i];
        adm_free(due->ecuReqId);
        adm_free(due->ecuSwDid);
        adm_free(due->failReason);
    }
}

const IPM_ECU_INSTALL_CONTROL_T* ipmEcuInstallCtrGet(const DCHAR* ecuReqId)
{
    IPM_ECU_INSTALL_CONTROL_T* dues = s_ecuInstallCtrList.dues;

    for (D32S i = 0; i < s_ecuInstallCtrList.num; i++)
    {
        if (0 == adm_strcasecmp(ecuReqId, dues[i].ecuReqId))
        {
            return &dues[i];
        }
    }

    return NULL;
}


D32S analyseInstallActionResult(const DCHAR* ecuReqId, const DCHAR* ecuSwDid, const DCHAR* reason)
{
    D32S ret = ADM_ECODE_SUCCESSED;
    ADM_IPS_ECU_PROGRESS_T progress;
    const IPM_ECU_INSTALL_CONTROL_T * due = ipmEcuInstallCtrGet(ecuReqId);

    if (0 == adm_strcmp(reason, due->failReason))
    {
        adm_ips_ecuProgressInit(&progress);
        adm_ips_ecuProgressGet(&progress, ecuReqId, ecuSwDid);

        switch (progress.state)
        {
        case ADM_IPS_ECU_PHASE_INSTALL:
            if (0 == due->upgradeRes)
            {
                if (due->upgradeTryTimes <= progress.progress[ADM_IPS_ECU_PHASE_INDEX_UPGRADE].upgradeNumbers)
                {
                    break;
                }
            }

            ret = ADM_ECODE_FAILED;
            break;
        case ADM_IPS_ECU_PHASE_INSTALL_ROLLBACK:
            if (0 == due->rollbackRes)
            {
                if (due->rollbackTryTimes <= progress.progress[ADM_IPS_ECU_PHASE_INDEX_ROLLBACK].upgradeNumbers)
                {
                    break;
                }
            }

            ret = ADM_ECODE_FAILED;
            break;
        }

        adm_ips_ecuProgressDeInit(&progress);
    }

    return ret;
}

static D32S ipm_getInstallStage()
{
    ADM_IPS_PROGRESS_T progress;
    ADM_IPS_PHASE_E stage = ADM_IPS_PHASE_IDLE;
    adm_ips_progressInit(&progress);
    adm_ips_progressGet(&progress);
    stage = progress.state;
    adm_ips_progressDeInit(&progress);

    return stage;
}

/* ECS */
ADM_ECS_RSCONFIGDATA_T s_ecsRsConfigData;

void ecsConfigDataEcuDidInitByJson(const json_object* root, ADM_ECS_ECUDID_T* did)
{
    did->did = adm_strdup(json_object_get_string(json_object_object_get(root, "did")));
    did->start = json_object_get_int(json_object_object_get(root, "start"));
    did->end = json_object_get_int(json_object_object_get(root, "end"));
}

void ecsConfigDataEcuDidDestroy(ADM_ECS_ECUDID_T* did)
{
    adm_free(did->did);
}

void ecsConfigDataEcuDidsInitByJson(const json_object* root, ADM_ECS_ECUDIDLIST_T* ecuDids)
{
    D32S didCount = 0;
    ADM_ECS_ECUDID_T* did = NULL;
    ADM_ECS_ECUDID_T* dids = NULL;

    if (NULL != root)
    {
        didCount = json_object_array_length(root);
        dids = adm_malloc(sizeof(dids[0]) * didCount);

        for (D32S i = 0; i < didCount; i++)
        {
            ecsConfigDataEcuDidInitByJson(json_object_array_get_idx(root, i), &dids[i]);
        }
    }

    ecuDids->pstEcuDidInfo = dids;
    ecuDids->num = didCount;
}

void ecsConfigDataEcuDidsDestroy(ADM_ECS_ECUDIDLIST_T* ecuDids)
{
    for (D32S i = 0; i < ecuDids->num; i++)
    {
        ecsConfigDataEcuDidDestroy(&ecuDids->pstEcuDidInfo[i]);
    }

    adm_free(ecuDids->pstEcuDidInfo);
}

void ecsConfigDataEcuCgfInitByJson(const json_object* root, ADM_ECS_ECUCONFIG_T* ecuCfg)
{
    ecuCfg->ecuReqId = adm_strdup(json_object_get_string(json_object_object_get(root, "ecuReqId")));
    ecuCfg->ecuSwDid = adm_strdup(json_object_get_string(json_object_object_get(root, "ecuSwDid")));
    ecuCfg->ecuResponseId = adm_strdup(json_object_get_string(json_object_object_get(root, "responseId")));
    ecuCfg->ecuFunctionId = adm_strdup(json_object_get_string(json_object_object_get(root, "functionId")));
    ecuCfg->installInstanceId = adm_strdup(json_object_get_string(json_object_object_get(root, "installInstanceId")));
    ecuCfg->transferInstanceId = adm_strdup(json_object_get_string(json_object_object_get(root, "transferInstanceId")));
    ecuCfg->serviceType = json_object_get_int(json_object_object_get(root, "serviceType"));
    ecuCfg->securityLevel = json_object_get_int(json_object_object_get(root, "securityLevel"));

    ecsConfigDataEcuDidsInitByJson(json_object_object_get(root, "ecuDids"), &ecuCfg->stEcuDidList);
}

void ecsConfigDataEcuCgfDestroy(ADM_ECS_ECUCONFIG_T* ecuCfg)
{
    adm_free(ecuCfg->ecuReqId);
    adm_free(ecuCfg->ecuSwDid);
    adm_free(ecuCfg->ecuResponseId);
    adm_free(ecuCfg->ecuFunctionId);
    adm_free(ecuCfg->installInstanceId);
}

void ecsConfigDataEcuCgfsInitByJson(const json_object* root, ADM_ECS_ECUCFGLIST_T* ecuCfgs)
{
    json_object* obj = NULL;
    ADM_ECS_ECUCONFIG_T* ecuConfigs = NULL;
    D32S ecuCount = json_object_array_length(root);
    
    ecuConfigs = adm_malloc(sizeof(ecuCfgs->pstEcuCfg[0]) * ecuCount);
    for (D32S i = 0; i < ecuCount; i++)
    {
        obj = json_object_array_get_idx(root, i);
        ecsConfigDataEcuCgfInitByJson(obj, &ecuConfigs[i]);
    }

    ecuCfgs->pstEcuCfg = ecuConfigs;
    ecuCfgs->num = ecuCount;
}

void ecsConfigDataEcuCgfsDestroy(ADM_ECS_ECUCFGLIST_T* ecuCfgs)
{
    for (D32S i = 0; i < ecuCfgs->num; i++)
    {
        ecsConfigDataEcuCgfDestroy(&ecuCfgs->pstEcuCfg[i]);
    }

    adm_free(ecuCfgs->pstEcuCfg);
}

D32S ecsConfigDataInit(const DCHAR* str, ADM_ECS_RSCONFIGDATA_T* rsConfigData)
{
    ADM_ECS_RSCONFIGINFO_T* cfgInfo = NULL;
    json_object* root = json_tokener_parse(str);
    
    cfgInfo = &rsConfigData->stRsCfgInfo;
    cfgInfo->stGeneralCfg.isReportUpSuccLogFile =
        json_object_get_int(json_object_object_get(json_object_object_get(root, "general"), "isReportUpSuccLogFile"));
    ecsConfigDataEcuDidsInitByJson(json_object_object_get(json_object_object_get(root, "general"), "ecuDids"),
        &cfgInfo->stGeneralCfg.stEcuDidList);

    ecsConfigDataEcuCgfsInitByJson(json_object_object_get(root, "parts"),
        &cfgInfo->stPartCfg.stEcuCfgList);

    return ADM_ECODE_SUCCESSED;
}

void ecsConfigDataDestroy(ADM_ECS_RSCONFIGDATA_T* rsConfigData)
{
    ADM_ECS_RSCONFIGINFO_T* cfgInfo = NULL;

    cfgInfo = &rsConfigData->stRsCfgInfo;
    ecsConfigDataEcuDidsDestroy(&cfgInfo->stGeneralCfg.stEcuDidList);
    ecsConfigDataEcuCgfsDestroy(&cfgInfo->stPartCfg.stEcuCfgList);
}

static D32S ecuCfgListGetEcuReqIdByIId(const ADM_ECS_ECUCFGLIST_T* ecuCfgList, const DCHAR* installInstanceId, IPM_ECU_ID_T* ecuId)
{
    ADM_ECS_ECUCONFIG_T* ecuCfgs = ecuCfgList->pstEcuCfg;

    for (D32S i = 0; i < ecuCfgList->num; i++)
    {
        if (0 == adm_strcasecmp(installInstanceId, ecuCfgs[i].installInstanceId))
        {
            adm_strcpy(ecuId->ecuReqId, ecuCfgs[i].ecuReqId, sizeof(ecuId->ecuReqId));
            adm_strcpy(ecuId->ecuSwDid, ecuCfgs[i].ecuSwDid, sizeof(ecuId->ecuSwDid));
            return ADM_ECODE_SUCCESSED;
        }
    }

    return ADM_ECODE_FAILED;
}

D32S ecsGetEcuReqIdByIId(const DCHAR* installInstallInstanceId, IPM_ECU_ID_T* ecuId)
{
    return ecuCfgListGetEcuReqIdByIId(&s_ecsRsConfigData.stRsCfgInfo.stPartCfg.stEcuCfgList, installInstallInstanceId, ecuId);
}

void pisPackageDataFixPath(const DCHAR* pkgDir, ADM_PIS_FILE_T * file)
{
    DCHAR path[256] = { 0 };

    adm_snprintf(path, sizeof(path), "%s/%s", pkgDir, file->path);
    adm_free(file->path);
    file->path = adm_strdup(path);
}

void pisPackageDataInitByJson(const json_object* root, const ADM_PIS_PACKAGE_HEAD_T* head, const DCHAR* pkgDir)
{
    ADM_PIS_PACKAGE_DATA_T data = {0};
    
    if (NULL != root)
    {
        adm_pis_packageDataJsonParse(&data, root);
        pisPackageDataFixPath(pkgDir, &data.imagePackage);
        if (NULL != data.signPackage)
        {
            pisPackageDataFixPath(pkgDir, data.signPackage);
        }

        adm_pis_packageSet(head, &data);
        adm_pis_packageDataDeInit(&data);
    }
}

void pisInstallPkgSetByJson(const json_object* root, const DCHAR* pkgDir)
{
    ADM_PIS_PACKAGE_HEAD_T head;
    
    head.state = ADM_PIS_PACKAGE_STATE_DECRYPT_FINISH;
    head.ecuReqId = (DCHAR*)json_object_get_string(json_object_object_get(root, "ecuReqId"));
    head.ecuSwDid = (DCHAR*)json_object_get_string(json_object_object_get(root, "ecuSwDid"));

    head.type = ADM_PIS_PACKAGE_TYPE_ECU_SRC_PACKAGE;
    pisPackageDataInitByJson(json_object_object_get(root, "srcPkg"), &head, pkgDir);

    head.type = ADM_PIS_PACKAGE_TYPE_ECU_DST_PACKAGE;
    pisPackageDataInitByJson(json_object_object_get(root, "destPkg"), &head, pkgDir);
}

void pisInstallPkgInit(const DCHAR* pkgDir, const DCHAR* str)
{
    json_object* root = json_tokener_parse(str);

    adm_pis_packageInit();

    D32S count = json_object_array_length(root);
    for (D32S i = 0; i < count; i++)
    {
        pisInstallPkgSetByJson(json_object_array_get_idx(root, i), pkgDir);
    }
}

void pisInstallPkgDestroy()
{
    adm_pis_packageDeInit();
}

typedef struct
{
    DCHAR ecuReqId[32];
    DCHAR ecuSwDid[32];
    D32S installState;
    D32S upgradeRes;
    D32S upgradeTimes;
    D32S rollbackRes;
    D32S rollbackTimes;
} IPM_TEST_ECU_INSTALL_EXPECTION;

typedef struct
{
    D32S num;
    IPM_TEST_ECU_INSTALL_EXPECTION* expections;
} IPM_TEST_ECU_INSTALL_EXP_LIST;

typedef struct
{
    D32S    installState;
    D32S    installRes;
    IPM_TEST_ECU_INSTALL_EXP_LIST expectionList;
} IPM_TEST_INSTALL_EXPECTION;

IPM_TEST_INSTALL_EXPECTION s_installExpection;

void ipmEcuInstallExpectionInitByJson(json_object* root, IPM_TEST_ECU_INSTALL_EXPECTION* expection)
{
    adm_strcpy(expection->ecuReqId, json_object_get_string(json_object_object_get(root, "ecuReqId")), sizeof(expection->ecuReqId));
    adm_strcpy(expection->ecuSwDid, json_object_get_string(json_object_object_get(root, "ecuSwDid")), sizeof(expection->ecuSwDid));
    expection->installState = json_object_get_int(json_object_object_get(root, "installState"));
    expection->upgradeRes = json_object_get_int(json_object_object_get(root, "upgradeRes"));
    expection->upgradeTimes = json_object_get_int(json_object_object_get(root, "upgradeTimes"));
    expection->rollbackRes = json_object_get_int(json_object_object_get(root, "rollbackRes"));
    expection->rollbackTimes = json_object_get_int(json_object_object_get(root, "rollbackTimes"));
}

void ipmEcuInstallExpectionListInitByJson(json_object* root, IPM_TEST_ECU_INSTALL_EXP_LIST* list)
{
    IPM_TEST_ECU_INSTALL_EXPECTION* expections = NULL;
    D32S count = json_object_array_length(root);

    expections = adm_malloc(count * sizeof(expections[0]));
    for (D32S i = 0; i < count; i++)
    {
        ipmEcuInstallExpectionInitByJson(json_object_array_get_idx(root, i), &expections[i]);
    }

    list->expections = expections;
    list->num = count;
}

void ipmInstallExpectionInit(const DCHAR* str, IPM_TEST_INSTALL_EXPECTION* expection)
{
    json_object* root = json_tokener_parse(str);

    expection->installState = json_object_get_int(json_object_object_get(root, "installState"));
    expection->installRes = json_object_get_int(json_object_object_get(root, "installRes"));
    ipmEcuInstallExpectionListInitByJson(json_object_object_get(root, "ecus"), &expection->expectionList);
}


D32S ipm_readFile(const DCHAR* name, DCHAR* buf, D32S bufSize)
{
    D32S size = 0;
    void* file = NULL;

    adm_fopen(name, "a+", &file);
    if (NULL != file)
    {
        adm_fread(buf, bufSize - 1, &size, file);
        adm_fclose(&file);
        buf[size] = '\0';
    }

    return size;
}

static DCHAR m_installPolicyObjectStr[4096];
D32S ipm_test_env_init(const DCHAR* dir)
{
    DCHAR buf[4096] = { 0 };
    D32S size = 0;
    DCHAR path[256];
    
    adm_core_service_initRuntimeDir(ADM_CORE_SERVICE_RUNTIME_DIR_TYPE_UNZIP, "./unzip");

    adm_snprintf(path, sizeof(path), "%s/%s", dir, "ecu_config.json");
    size = ipm_readFile(path, buf, sizeof(buf));
    if (0 == size)
    {
        return ADM_ECODE_FAILED;
    }
    ecsConfigDataInit(buf, &s_ecsRsConfigData);

    adm_snprintf(path, sizeof(path), "%s/%s", dir, "ecu_install_policy.json");
    size = ipm_readFile(path, m_installPolicyObjectStr, sizeof(m_installPolicyObjectStr));
    if (0 == size)
    {
        return ADM_ECODE_FAILED;
    }

    adm_snprintf(path, sizeof(path), "%s/%s", dir, "ecu_before_install.json");
    size = ipm_readFile(path, buf, sizeof(buf));
    if (0 == size)
    {
        return ADM_ECODE_FAILED;
    }
    s_ecusInfosOri = ecusDidsInit(buf);

    adm_snprintf(path, sizeof(path), "%s/%s", dir, "ecu_after_install.json");
    size = ipm_readFile(path, buf, sizeof(buf));
    if (0 == size)
    {
        return ADM_ECODE_FAILED;
    }
    s_ecusInfosLastest = ecusDidsInit(buf);

    adm_snprintf(path, sizeof(path), "%s/%s", dir, "ecu_install_control.json");
    size = ipm_readFile(path, buf, sizeof(buf));
    if (0 == size)
    {
        return ADM_ECODE_FAILED;
    }
    ipmEcuInstallCtrsInit(buf, &s_ecuInstallCtrList);

    adm_snprintf(path, sizeof(path), "%s/%s", dir, "ecu_install_pkg_info.json");
    size = ipm_readFile(path, buf, sizeof(buf));
    if (0 == size)
    {
        return ADM_ECODE_FAILED;
    }
    pisInstallPkgInit(dir, buf);

    return ADM_ECODE_SUCCESSED;
}

D32S ipm_test_env_destroy()
{
    m_installPolicyObjectStr[0] = '\0';
    ecusDidsDestory(s_ecusInfosOri);
    ecusDidsDestory(s_ecusInfosLastest);
    ecsConfigDataDestroy(&s_ecsRsConfigData);
    ipmEcuInstallCtrsDestroy(&s_ecuInstallCtrList);
    pisInstallPkgDestroy();
    // adm_cfg_iniParserDeinit();
    adm_core_service_deinitRuntimeDir(ADM_CORE_SERVICE_RUNTIME_DIR_TYPE_UNZIP);
}

#include <sys/types.h>
#include<dirent.h>
typedef struct
{
    DIR* dir;
    DCHAR srcPath[256];
} IPM_TEST_DIR_READER_T;

D32S ipm_test_dir_reader_init(IPM_TEST_DIR_READER_T* reader, const DCHAR* path)
{
    D32S ret = ADM_ECODE_FAILED;
    DIR* dir = NULL;

    dir = opendir(path);
    if (NULL != dir)
    {
        adm_strcpy(reader->srcPath, path, sizeof(reader->srcPath));
        ret = ADM_ECODE_SUCCESSED;
    }

    reader->dir = dir;
    return ret;
}

D32S ipm_test_dir_reader_next_dir(IPM_TEST_DIR_READER_T * reader, DCHAR* dir, D32S dirMaxSize)
{
    D32S ret = ADM_ECODE_FAILED;
    struct dirent* entry = NULL;

    if (NULL != reader->dir)
    {
        while ((entry = readdir(reader->dir)) != NULL)
        {
            if (0 == strcmp(entry->d_name, ".") || 0 == strcmp(entry->d_name, ".."))
            {
                continue;
            }

            if (entry->d_type == DT_DIR)
            {
                adm_snprintf(dir, dirMaxSize, "%s/%s", reader->srcPath, entry->d_name);
                ret = ADM_ECODE_SUCCESSED;
                break;
            }
        }
    }

    return ret;
}

void ipm_test_dir_reader_destroy(IPM_TEST_DIR_READER_T* reader)
{
    closedir(reader->dir);
}

ADM_ECS_RSCONFIGDATA_T* __wrap_adm_ecs_getOtaConfigDataFromKV()
{
    return &s_ecsRsConfigData;
}

void __wrap_adm_ecs_delEcuConfigInfo(ADM_ECS_ECUCONFIG_T* pstEcuCfgInfo)
{
}

D32S __wrap_adm_ecs_getEcuConfig(DCHAR* ecuReqId, DCHAR* ecuSwDid, ADM_ECS_ECUCONFIG_T** ppstEcuConfig)
{
    D32S ret = ADM_ECODE_FAILED;
    ADM_ECS_ECUCONFIG_T* ecuCnf = NULL;
    ADM_ECS_ECUCFGLIST_T * cnfList = &s_ecsRsConfigData.stRsCfgInfo.stPartCfg.stEcuCfgList;


    for (D32S i = 0; i < cnfList->num; i++)
    {
        ecuCnf = &cnfList->pstEcuCfg[i];
        if (0 == adm_strcasecmp(ecuReqId, ecuCnf->ecuReqId) &&
            0 == adm_strcasecmp(ecuSwDid, ecuCnf->ecuSwDid))
        {
            *ppstEcuConfig = ecuCnf;
            ret = ADM_ECODE_SUCCESSED;
            break;
        }
    }

    return ret;
}

D32S __wrap_adm_ecs_updateAndGetConfigInfo(ADM_ECS_RSCONFIGDATA_T** ppstOtaConfigData)
{
    return ADM_ECODE_SUCCESSED;
}

D32S __wrap_adm_ecs_getIsReportUpSuccLogFile(D32S* ps32IsReportUpSuccLogFile)
{
    *ps32IsReportUpSuccLogFile = s_ecsRsConfigData.stRsCfgInfo.stGeneralCfg.isReportUpSuccLogFile;
    return ADM_ECODE_SUCCESSED;
}

/* EIM */
D32S adm_eim_getSingleEcuInfoForEcu(
    const ADM_ECS_ECUCONFIG_T* pstEcuCfg,
    const ADM_ECS_ECUDIDLIST_T* pstEcuDidList,
    const ADM_DIM_SOA_ECU_IDEN_INFO_T* pstEcuIdenInfo,
    const DCHAR* diagScriptPath,
    ECU_INFO_T** ppEcuInfo)
{
    *ppEcuInfo = ecuInfoGetByReqId(pstEcuCfg->ecuReqId, pstEcuCfg->ecuSwDid);
    return ADM_ECODE_SUCCESSED;
}

D32S adm_uim_soaAllEcuInstallResultNotify(const ADM_APPLICATION_COMMON_CMDID_TYPE_E eCmdidType,
    const DCHAR* sessionId,
    const DCHAR* state,
    const ADM_UIM_SOA_ECU_INSTALL_RESULT_ARRAY_INFO_T* pstEcuInstallResultArrayInfo,
    const ADM_UIM_SOA_INSTALL_RESULT_INFO_T* pstPolicyInstallResult)
{
    return ADM_ECODE_SUCCESSED;
}

void adm_eim_getEcuInfoManagerEcusStore(ADM_EIM_GETECUINFO_T* newEcusArray) 
{
}

void adm_eim_deleteGetEcuInfoBody(ADM_EIM_GETECUINFO_T* getEcuInfo)
{
}

void adm_eim_deleteEcuInfoBody(ECU_INFO_T* ecuInfo)
{
}


/* DIM */
typedef struct {
    struct {
        D32S requestId;
        D32S swDid;
    }*ecus;
    D32S ecusLen;
}ADM_DIM_VDI_FLASH_T;
static void dimVdiFlashInit(ADM_DIM_VDI_FLASH_T* flash) {
    flash->ecus = NULL;
    flash->ecusLen = 0;
}
static void dimVdiFlashDeInit(ADM_DIM_VDI_FLASH_T* flash) {
    if (flash->ecus) { adm_free(flash->ecus); }
}
static void dimVdiFlashClear(ADM_DIM_VDI_FLASH_T* flash) {
    dimVdiFlashDeInit(flash);
    dimVdiFlashInit(flash);
}
static void dimVdiFlashSet(ADM_DIM_VDI_FLASH_T* flash, const ADM_DIM_SOA_VDI_REFLASH_INFO_ARRAY_T* reflashInfoList) {
    if (reflashInfoList->reflashInfoArray && reflashInfoList->reflashInfoArrayLength) {
        flash->ecus = adm_malloc(reflashInfoList->reflashInfoArrayLength * sizeof(*(flash->ecus)));
        if (flash->ecus) {
            flash->ecusLen = reflashInfoList->reflashInfoArrayLength;
            for (D32S i = 0; i < flash->ecusLen; ++i) {
                flash->ecus[i].requestId = 0;
                flash->ecus[i].swDid = 0;
                if (reflashInfoList->reflashInfoArray[i]) {
                    if (reflashInfoList->reflashInfoArray[i]->pstEcuIdenInfo) {
                        flash->ecus[i].requestId = reflashInfoList->reflashInfoArray[i]->pstEcuIdenInfo->requestId;
                        flash->ecus[i].swDid = reflashInfoList->reflashInfoArray[i]->pstEcuIdenInfo->swDid;
                    }
                }
            }
        }
    }
}

typedef struct {
    struct {
        D32S requestId;
        D32S swDid;
    }*ecus;
    D32S ecusLen;
}ADM_DIM_UCL_FLASH_T;
static void dimUcmFlashInit(ADM_DIM_UCL_FLASH_T* flash) {
    flash->ecus = NULL;
    flash->ecusLen = 0;
}
static void dimUcmFlashDeInit(ADM_DIM_UCL_FLASH_T* flash) {
    if (flash->ecus) { adm_free(flash->ecus); }
}
static void dimUcmFlashClear(ADM_DIM_UCL_FLASH_T* flash) {
    dimUcmFlashDeInit(flash);
    dimUcmFlashInit(flash);
}
static void dimUcmFlashSet(ADM_DIM_UCL_FLASH_T* flash, const ADM_DIM_SOA_UCL_UPGRADE_INFO_ARRAY_T* upgradeInfoList) {
    if (upgradeInfoList->upgradeInfoArray && upgradeInfoList->upgradeInfoArrayLength) {
        flash->ecus = adm_malloc(upgradeInfoList->upgradeInfoArrayLength * sizeof(*(flash->ecus)));
        if (flash->ecus) {
            flash->ecusLen = upgradeInfoList->upgradeInfoArrayLength;
            for (D32S i = 0; i < flash->ecusLen; ++i) {
                flash->ecus[i].requestId = 0;
                flash->ecus[i].swDid = 0;
                if (upgradeInfoList->upgradeInfoArray[i]) {
                    if (upgradeInfoList->upgradeInfoArray[i]->pstEcuIdenInfo) {
                        flash->ecus[i].requestId = upgradeInfoList->upgradeInfoArray[i]->pstEcuIdenInfo->requestId;
                        flash->ecus[i].swDid = upgradeInfoList->upgradeInfoArray[i]->pstEcuIdenInfo->swDid;
                    }
                }
            }
        }
    }
}

typedef struct {
    ADM_DIM_SOA_UCL_INIT_INFO_T initInfo;
    void* initInfoPriv;
    pthread_mutex_t flashMutex;
    ADM_DIM_UCL_FLASH_T flash;
}ADM_DIM_UCL_T;
static void dimUcmInit(ADM_DIM_UCL_T* ucm, const ADM_DIM_SOA_UCL_INIT_INFO_T* initInfo, void* initInfoPriv) {
    ucm->initInfo.readPartInfoCallback = initInfo->readPartInfoCallback;
    ucm->initInfo.upgradeProgressNotifyCallback = initInfo->upgradeProgressNotifyCallback;
    ucm->initInfo.upgradeResultNotifyCallback = initInfo->upgradeResultNotifyCallback;
    ucm->initInfo.decryptProgressNotifyCallback = initInfo->decryptProgressNotifyCallback;
    ucm->initInfo.decryptResultNotifyCallback = initInfo->decryptResultNotifyCallback;
    ucm->initInfo.verifyPackageHashProgressNotifyCallback = initInfo->verifyPackageHashProgressNotifyCallback;
    ucm->initInfo.verifyPackageHashResultNotifyCallback = initInfo->verifyPackageHashResultNotifyCallback;
    ucm->initInfoPriv = initInfoPriv;
    pthread_mutex_init(&ucm->flashMutex, NULL);
    dimUcmFlashInit(&ucm->flash);
}
static void dimUcmDeInit(ADM_DIM_UCL_T* ucm) {
    pthread_mutex_destroy(&ucm->flashMutex);
    dimUcmFlashDeInit(&ucm->flash);
}
static void* dimUcmFlashRunHandle(void* argv) {
    ADM_DIM_UCL_T* ucm = (ADM_DIM_UCL_T*)argv;
    const ADM_DIM_UCL_FLASH_T* flash = &ucm->flash;
    if (flash->ecus && ucm->initInfo.upgradeProgressNotifyCallback && ucm->initInfo.upgradeResultNotifyCallback) {
        for (D32S i = 0; i < 101; ++i) {
            usleep(150 * 1000);
            for (D32S j = 0; j < flash->ecusLen; ++j) {
                ADM_DIM_SOA_ECU_IDEN_INFO_T pstIdenInfo = { .requestId = flash->ecus[j].requestId, .swDid = flash->ecus[j].swDid };
                D16U progress = i;
                ucm->initInfo.upgradeProgressNotifyCallback(&pstIdenInfo, progress, ucm->initInfoPriv);
            }
        }
        for (D32S k = 0; k < flash->ecusLen; ++k) {
            ADM_DIM_SOA_ECU_IDEN_INFO_T pstIdenInfo = { .requestId = flash->ecus[k].requestId, .swDid = flash->ecus[k].swDid };
            ADM_DIM_SOA_UCL_UPGRADE_RESULT_INFO_T pstUpgradeResultInfo = { .result = 0, .expectVal = NULL, .realVal = NULL };
            ucm->initInfo.upgradeResultNotifyCallback(&pstIdenInfo, &pstUpgradeResultInfo, ucm->initInfoPriv);
        }
    }
    pthread_mutex_unlock(&ucm->flashMutex);
    return NULL;
}
static DBOOL dimUcmFlashRun(ADM_DIM_UCL_T* ucm, const ADM_DIM_SOA_UCL_UPGRADE_INFO_ARRAY_T* upgradeInfoList) {
    DBOOL dRet = 0;
    if (pthread_mutex_trylock(&ucm->flashMutex) == 0) {
        dimUcmFlashSet(&ucm->flash, upgradeInfoList);
        pthread_t pth;
        if (pthread_create(&pth, NULL, dimUcmFlashRunHandle, ucm) == 0) {
            pthread_detach(pth);
            dRet = 1;
        }
        if (dRet == 0) {
            pthread_mutex_unlock(&ucm->flashMutex);
        }
    }
    return dRet;
}

typedef struct {
    ADM_DIM_SOA_VDI_INIT_INFO_T initInfo;
    void* initInfoPriv;
    pthread_mutex_t flashMutex;
    ADM_DIM_VDI_FLASH_T flash;
}ADM_DIM_VDI_T;
static void dimVdiInit(ADM_DIM_VDI_T* vdi, const ADM_DIM_SOA_VDI_INIT_INFO_T* initInfo, void* initInfoPriv) {
    vdi->initInfo.readDiagInfoCallback = initInfo->readDiagInfoCallback;
    vdi->initInfo.reflashProgressCallback = initInfo->reflashProgressCallback;
    vdi->initInfo.reflashResultCallback = initInfo->reflashResultCallback;
    vdi->initInfo.spyVehicleStatusCallback = initInfo->spyVehicleStatusCallback;
    vdi->initInfo.vehicleSignalEventCallback = initInfo->vehicleSignalEventCallback;
    vdi->initInfoPriv = initInfoPriv;
    pthread_mutex_init(&vdi->flashMutex, NULL);
    dimVdiFlashInit(&vdi->flash);
}
static void dimVdiDeInit(ADM_DIM_VDI_T* vdi) {
    pthread_mutex_destroy(&vdi->flashMutex);
    dimVdiFlashDeInit(&vdi->flash);
}
static void* dimVdiFlashRunHandle(void* argv) {
    ADM_DIM_VDI_T* vdi = (ADM_DIM_VDI_T*)argv;
    const ADM_DIM_VDI_FLASH_T* flash = &vdi->flash;
    if (flash->ecus && vdi->initInfo.reflashProgressCallback && vdi->initInfo.reflashResultCallback) {
        for (D32S i = 0; i < 101; ++i) {
            usleep(50 * 1000);
            for (D32S j = 0; j < flash->ecusLen; ++j) {
                ADM_DIM_SOA_ECU_IDEN_INFO_T pstIdenInfo = { .requestId = flash->ecus[j].requestId, .swDid = flash->ecus[j].swDid };
                D16U reflashProgress = i;
                vdi->initInfo.reflashProgressCallback((ADM_DIM_SOA_ECU_IDEN_INFO_T*)&pstIdenInfo, &reflashProgress, vdi->initInfoPriv);
            }
        }
        for (D32S k = 0; k < flash->ecusLen; ++k) {
            ADM_DIM_SOA_ECU_IDEN_INFO_T pstIdenInfo = { .requestId = flash->ecus[k].requestId, .swDid = flash->ecus[k].swDid };
            ADM_DIM_SOA_VDI_REFLASH_RESULT_INFO_T pstReflashResultSendInfo = { .result = 0, .expectVal = NULL, .realVal = NULL };
            vdi->initInfo.reflashResultCallback((ADM_DIM_SOA_ECU_IDEN_INFO_T*)&pstIdenInfo, &pstReflashResultSendInfo, vdi->initInfoPriv);
        }
    }
    pthread_mutex_unlock(&vdi->flashMutex);
    return NULL;
}
static DBOOL dimVdiFlashRun(ADM_DIM_VDI_T* vdi, const ADM_DIM_SOA_VDI_REFLASH_INFO_ARRAY_T* reflashInfoList) {
    DBOOL dRet = 0;
    if (pthread_mutex_trylock(&vdi->flashMutex) == 0) {
        dimVdiFlashSet(&vdi->flash, reflashInfoList);
        pthread_t pth;
        if (pthread_create(&pth, NULL, dimVdiFlashRunHandle, vdi) == 0) {
            pthread_detach(pth);
            dRet = 1;
        }
        if (dRet == 0) {
            pthread_mutex_unlock(&vdi->flashMutex);
        }
    }
    return dRet;
}

D16U adm_soa_getVdiInstanceId()
{
    return 0;
}

static ADM_DIM_VDI_T m_dimVdi;
D32S adm_dim_soa_vdi_init(ADM_SOA_SESSION_T* soaSession, ADM_DIM_SOA_VDI_INIT_INFO_T* initInfo, void* priv)
{
    dimVdiInit(&m_dimVdi, initInfo, priv);
    return 0;
}
void adm_dim_soa_vdi_deinit()
{
    dimVdiDeInit(&m_dimVdi);
}

static ADM_DIM_UCL_T m_dimucm;
D32S adm_dim_soa_ucl_init(ADM_SOA_SESSION_T* session, ADM_DIM_SOA_UCL_INIT_INFO_T* initInfo, void* priv) {
    dimUcmInit(&m_dimucm, initInfo, priv);
    return 0;
}
void adm_dim_soa_ucl_deinit() {
    dimUcmDeInit(&m_dimucm);
}

D32S adm_dim_host_setSchduleWakeUpTime(const D16U instanceId, const D16U timeout, 
    const ADM_UIM_SOA_INSTALL_SCHDULE_TYPE_E eSchduleType, const D64S schduleTime, D32S* errorCode) {
    //TODO
    *errorCode = 0;
    return 0;
}

D32S adm_dim_host_getSchduleWakeUpTime(const D16U instanceId, const D16U timeout, D64S* schduleTime, D32S* errorCode)
{
    //TO-DO
    *errorCode = 0;
    return ADM_ECODE_SUCCESSED;
}

D32S adm_dim_host_execOtaTask(const D16U instanceId, const D16U timeout, const ADM_DIM_SOA_HOST_OTA_TASK_TYPE_E eOtaTaskType, const D32U keeptime, D32S* errorCode)
{
    *errorCode = 0;
    return ADM_ECODE_SUCCESSED;
}

D32S adm_dim_host_verifySignature(const D16U instanceId, const D16U timeout,
    const DCHAR* signedData, const D32S signedDataLength,
    const DCHAR* orignalData, const D32S orignalDataLength,
    D32S* errorCode)
{
    *errorCode = 0;
    return ADM_ECODE_SUCCESSED;
}

D32S adm_dim_vdi_controlVehicleStatus(const D16U instanceId, const D16U timeout,
    ADM_DIM_SOA_VDI_CTRL_VEHICLE_STATUS_TYPE_E eVechielStatusType, ADM_DIM_SOA_DATA_T* variable,
    const D32U keeptime, D32S* errorCode)
{
    *errorCode = 0;
    return ADM_ECODE_SUCCESSED;
}

D32S adm_dim_vdi_performSpecDiagnosticTask(const D16U instanceId, const D16U timeout,
    const ADM_DIM_SOA_VDI_SPEC_DIAGNOSTIC_TASK_TYPE_E eSpecDiagTaskType,
    const DCHAR* absolutePath, D32S* errorCode)
{
    *errorCode = 0;
    return ADM_ECODE_SUCCESSED;
}

D32S adm_dim_vdi_queryReflashProgress(
    const D16U instanceId, const D16U timeout,
    const ADM_DIM_SOA_ECU_IDEN_INFO_T* pstEcuIdenInfo,
    D16U* reflashProgress, D32S* errorCode)
{
    DCHAR ecuReqId[64] = { 0 };
    DCHAR ecuSwDid[64] = { 0 };
    adm_snprintf(ecuReqId, sizeof(ecuReqId), "0x%x", pstEcuIdenInfo->requestId);
    adm_snprintf(ecuSwDid, sizeof(ecuSwDid), "0x%x", pstEcuIdenInfo->swDid);

    *errorCode = 0;
    return ADM_ECODE_SUCCESSED;
}

D32S adm_dim_vdi_queryReflashResult(const D16U instanceId, const D16U timeout,
    const ADM_DIM_SOA_ECU_IDEN_INFO_T* pstEcuIdenInfo,
    ADM_DIM_SOA_VDI_REFLASH_RESULT_INFO_T** pstReflashResultSendInfo,
    D32S* errorCode)
{
    ADM_DIM_SOA_VDI_REFLASH_RESULT_INFO_T* resultInfo = NULL;
    DCHAR ecuReqId[64] = {0};
    DCHAR ecuSwDid[64] = { 0 };
    adm_snprintf(ecuReqId, sizeof(ecuReqId), "0x%x", pstEcuIdenInfo->requestId);
    adm_snprintf(ecuSwDid, sizeof(ecuSwDid), "0x%x", pstEcuIdenInfo->swDid);

    resultInfo = adm_malloc(sizeof(ADM_DIM_SOA_VDI_REFLASH_RESULT_INFO_T));
    memset(resultInfo, 0, sizeof(ADM_DIM_SOA_VDI_REFLASH_RESULT_INFO_T));
    resultInfo->result = 0;

    if (ADM_ECODE_SUCCESSED == analyseInstallActionResult(ecuReqId, ecuSwDid, "QueryReflashResult"))
    {
        resultInfo->result = 0;
    }
    else
    {
        resultInfo->result = 1;
    }

    *pstReflashResultSendInfo = resultInfo;
    *errorCode = ADM_ECODE_SUCCESSED;

    return ADM_ECODE_SUCCESSED;
}
D32S adm_dim_vdi_reflashEcusTrigger(const D16U instanceId, const D16U timeout, 
    const ADM_DIM_SOA_VDI_REFLASH_INFO_ARRAY_T* reflashInfoList, D32S* errorCode)
{
    DCHAR ecuReqId[64] = { 0 };
    DCHAR ecuSwDid[64] = { 0 };

    for (D32S i = 0; i < reflashInfoList->reflashInfoArrayLength; i++)
    {
        adm_snprintf(ecuReqId, sizeof(ecuReqId), "0x%x", reflashInfoList->reflashInfoArray[i]->pstEcuIdenInfo->requestId);
        adm_snprintf(ecuSwDid, sizeof(ecuSwDid), "0x%x", reflashInfoList->reflashInfoArray[i]->pstEcuIdenInfo->swDid);

        if (ADM_ECODE_SUCCESSED == analyseInstallActionResult(ecuReqId, ecuSwDid, "ReflashEcusTrigger"))
        {
            *errorCode = 0;
        }
        else
        {
            *errorCode = 1;
        }
    }


    return ADM_ECODE_SUCCESSED;
}

D32S adm_dim_ucl_queryUpgradeProgress(
    const D16U instanceId, const D16U timeout, 
    const ADM_DIM_SOA_ECU_IDEN_INFO_T* pstEcuIdenInfo, 
    D16U* progress, D32S* errorCode)
{
    DCHAR ecuReqId[64] = { 0 };
    DCHAR ecuSwDid[64] = { 0 };
    adm_snprintf(ecuReqId, sizeof(ecuReqId), "0x%x", pstEcuIdenInfo->requestId);
    adm_snprintf(ecuSwDid, sizeof(ecuSwDid), "0x%x", pstEcuIdenInfo->swDid);

    *errorCode = 0;
    return ADM_ECODE_SUCCESSED;
}

D32S adm_dim_ucl_queryUpgradeResult(
    const D16U instanceId, const D16U timeout, 
    const ADM_DIM_SOA_ECU_IDEN_INFO_T* pstEcuIdenInfo, 
    ADM_DIM_SOA_UCL_UPGRADE_RESULT_INFO_T** pstUpgradeResultInfo, 
    D32S* errorCode)
{
    ADM_DIM_SOA_UCL_UPGRADE_RESULT_INFO_T* resultInfo = NULL;
    DCHAR ecuReqId[64] = { 0 };
    DCHAR ecuSwDid[64] = { 0 };
    adm_snprintf(ecuReqId, sizeof(ecuReqId), "0x%x", pstEcuIdenInfo->requestId);
    adm_snprintf(ecuSwDid, sizeof(ecuSwDid), "0x%x", pstEcuIdenInfo->swDid);

    resultInfo = adm_malloc(sizeof(ADM_DIM_SOA_UCL_UPGRADE_RESULT_INFO_T));
    memset(resultInfo, 0, sizeof(ADM_DIM_SOA_UCL_UPGRADE_RESULT_INFO_T));

    if (ADM_ECODE_SUCCESSED == analyseInstallActionResult(ecuReqId, ecuSwDid, "QueryReflashResult"))
    {
        resultInfo->result = 0;
    }
    else
    {
        resultInfo->result = 1;
    }

    *pstUpgradeResultInfo = resultInfo;
    *errorCode = ADM_ECODE_SUCCESSED;

    return ADM_ECODE_SUCCESSED;
}

D32S adm_dim_ucl_requestDoUpgrade(const D16U instanceId, const D16U timeout,
    const ADM_DIM_SOA_UCL_UPGRADE_INFO_ARRAY_T* upgradeInfoList, D32S* errorCode) 
{
    DCHAR ecuReqId[64] = { 0 };
    DCHAR ecuSwDid[64] = { 0 };

    for (D32S i = 0; i < upgradeInfoList->upgradeInfoArrayLength; i++)
    {
        adm_snprintf(ecuReqId, sizeof(ecuReqId), "0x%x", upgradeInfoList->upgradeInfoArray[i]->pstEcuIdenInfo->requestId);
        adm_snprintf(ecuSwDid, sizeof(ecuSwDid), "0x%x", upgradeInfoList->upgradeInfoArray[i]->pstEcuIdenInfo->swDid);

        if (ADM_ECODE_SUCCESSED == analyseInstallActionResult(ecuReqId, ecuSwDid, "ReflashEcusTrigger"))
        {
            *errorCode = 0;
        }
        else
        {
            *errorCode = 1;
        }
    }

    *errorCode = 0;
    return ADM_ECODE_SUCCESSED;
}

/* IRM */
D64S adm_irm_getEventSequenceId()
{
    D64S sequenceId = 0;
    ++sequenceId;
    return sequenceId;
}

D32S adm_irm_reportVehicleState(D64S timestamp,
    const DCHAR* sdkVersion,
    const DCHAR* sessionId,
    D64S taskId,
    D64S strategyId,
    const DCHAR* state,
    const DCHAR* reason,
    const DCHAR* versionNumber,
    const DCHAR* vehicleVersion,
    const DCHAR* extsReqJsonStr) 
{
    log_i("timestamp[%ld] sdkVersion[%s] sessionId[%s] taskId[%d] strategyId[%d] state[%s] reason[%s] versionNumber[%s] ext[%s]", 
        timestamp, sdkVersion, sessionId, taskId, strategyId, state, reason, versionNumber, extsReqJsonStr);
    return ADM_ECODE_SUCCESSED;
}

D32S adm_irm_reportEcuState(const DCHAR* sessionId,
    D64S taskId,
    D64S strategyId,
    D64S timestamp,
    const DCHAR* sdkVersion,
    const DCHAR* extsReqJsonStr,
    ADM_OCS_ECU_STATE_INFO_LIST_T* ecusList)
{
    log_i("sessionId[%s] taskId[%d] strategyId[%d] timestamp[%ld] sdkVersion[%s] ext[%s]",
        sessionId, taskId, strategyId, timestamp, sdkVersion, extsReqJsonStr);
    return ADM_ECODE_SUCCESSED;
}

D32S adm_irm_reportVehicleProgress(const DCHAR* sessionId,
    D64S taskId,
    D64S strategyId,
    D64S timestamp,
    const DCHAR* sdkVersion,
    const DCHAR* extsReqJsonStr,
    ADM_OCS_ECU_PROGRESS_INFO_LIST_T* ecusProgressList)
{
    log_i("sessionId[%s] taskId[%d] strategyId[%d] timestamp[%ld] sdkVersion[%s] ext[%s]",
        sessionId, taskId, strategyId, timestamp, sdkVersion, extsReqJsonStr);
    return ADM_ECODE_SUCCESSED;
}

D32S adm_irm_reportEvent(const DCHAR* sessionId,
    D64S taskId,
    D64S strategyId,
    D64S timestamp,
    const DCHAR* sdkVersion,
    const DCHAR* extsReqJsonStr,
    ADM_OCS_EVENTS_INFO_LIST_T* eventsList)
{
    log_i("sessionId[%s] taskId[%d] strategyId[%d] timestamp[%ld] sdkVersion[%s] ext[%s]",
        sessionId, taskId, strategyId, timestamp, sdkVersion, extsReqJsonStr);
    return ADM_ECODE_SUCCESSED;
}

D32S adm_irm_syncReportVehicleState(D64S timestamp,
    const DCHAR* sdkVersion,
    const DCHAR* sessionId,
    D64S taskId,
    D64S strategyId,
    const DCHAR* state,
    const DCHAR* reason,
    const DCHAR* versionNumber,
    const DCHAR* extsReqJsonStr)
{
    log_i("timestamp[%ld] sdkVersion[%s] sessionId[%s] taskId[%d] strategyId[%d] state[%s] reason[%s] versionNumber[%s] ext[%s]",
        timestamp, sdkVersion, sessionId, taskId, strategyId, state, reason, versionNumber, extsReqJsonStr);
    return 0;
}

void adm_irm_triggerLogFileReport(const ADM_IRM_LOG_FILE_TG_REPORT_INFO_T* tgReportInfo)
{
    log_i("cmdId[%d] extsReqJsonStr[%s] pushId[%s]", tgReportInfo->cmdId, tgReportInfo->extsReqJsonStr, tgReportInfo->pushId);
}

/* SOA */
ADM_SOA_SESSION_T* adm_soa_getSoaSession()
{
    ADM_SOA_SESSION_T* soaSession = adm_malloc(100);
    return soaSession;
}

D16U adm_soa_getHostInstanceId()
{
    return 0;
}

/* VSM */
DBOOL adm_vsm_triggerInstall(void)
{
    return 1;
}

ADM_VSM_STATE_E adm_vsm_stateGet(void)
{
    return 0;
}

void adm_vsm_stateDel(void)
{

}

/* CDM */
D32S adm_cdm_verify_Compare(const ADM_COND_POLICY_T* pCondtions, ADM_UIM_SOA_CONDITION_DATA_ARRAY_T** pStCondResults, DBOOL* pbIsConform)
{
    return ADM_ECODE_SUCCESSED;
}

D32S adm_cdm_verify_free(ADM_UIM_SOA_CONDITION_DATA_ARRAY_T* pStCondResults)
{
    return ADM_ECODE_SUCCESSED;
}

D32S adm_pub_downPackageVerifySign
(
    ADM_COM_FILEOBJECT_T* pstPkgFileObj,
    DCHAR* pcDownInstId,
    DCHAR* pfileid,
    DBOOL isUnpick,
    ADM_SYSI4UCL_VERIFY_SIGN_RESULT_INFO_T* pstVerifySignResultInfo
)
{
    return 0;
}

D32S adm_eim_getEcuInfoManagerDimGet(ADM_EIM_GETECUINFO_T** ecuInfoList)
{
    return 0;
}

DBOOL adm_vsm_getHmiOnline(void)
{
    return 1;
}

void adm_uim_hmi_conds_installResult_RR(const ADM_APPLICATION_COMMON_CMDID_TYPE_E eCmdidType, const DCHAR* sessionId, const DCHAR* state,
    const ADM_UIM_SOA_CONDITION_DATA_ARRAY_T* condsArrarList, const D32S verifyResult, D32S* respCode)
{

}

void adm_uim_hmi_ecu_installResult_RR(const ADM_APPLICATION_COMMON_CMDID_TYPE_E eCmdidType, const DCHAR* sessionId, const DCHAR* state,
    const ADM_UIM_SOA_ECU_IDEN_INFO_T* pstEcuIdenInfo, const ADM_UIM_SOA_INSTALL_RESULT_INFO_T* pstEcuInstallResultInfo, D32S* respCode)
{
}

void adm_uim_hmi_policy_installResult_RR(const ADM_APPLICATION_COMMON_CMDID_TYPE_E eCmdidType, const DCHAR* sessionId, const DCHAR* state,
    const ADM_UIM_SOA_INSTALL_RESULT_INFO_T* pstPolicyInstallResult, D32S* respCode)
{

}
void adm_uim_hmi_allecu_installResult_RR(const ADM_APPLICATION_COMMON_CMDID_TYPE_E eCmdidType, const DCHAR* sessionId, const DCHAR* state,
    const ADM_UIM_SOA_ECU_INSTALL_RESULT_ARRAY_INFO_T* pstEcuInstallResultArrayInfo, const ADM_UIM_SOA_INSTALL_RESULT_INFO_T* pstPolicyInstallResult, D32S* respCode)
{
}
/* UIM */
//typedef struct {
//    ADM_UIM_SOA_INSTALL_INIT_INFO_T initInfo;
//    void* initInfoPriv;
//}ADM_UIM_INSTALL_T;
//
//static void uimInstallInit(ADM_UIM_INSTALL_T* uimInstall, const ADM_UIM_SOA_INSTALL_INIT_INFO_T* initInfo, void* initInfoPriv) {
//    uimInstall->initInfo.schduleInstallCallback = initInfo->schduleInstallCallback;
//    uimInstall->initInfo.cancelInstallCallback = initInfo->cancelInstallCallback;
//    uimInstall->initInfo.refuseInstallCallback = initInfo->refuseInstallCallback;
//    uimInstall->initInfo.installTriggerCallback = initInfo->installTriggerCallback;
//    uimInstall->initInfo.conditionsVerifiedCallback = initInfo->conditionsVerifiedCallback;
//    uimInstall->initInfo.allEcuInstallProgressCallback = initInfo->allEcuInstallProgressCallback;
//    uimInstall->initInfo.allEcuInstallResultCallback = initInfo->allEcuInstallResultCallback;
//    uimInstall->initInfoPriv = initInfoPriv;
//}
//static void uimInstallDeInit(ADM_UIM_INSTALL_T* uimInstall) {
//}
//static void* uimInstallTriggerHandle(void* argv) {
//    ADM_UIM_INSTALL_T* uimInstall = (ADM_UIM_INSTALL_T*)argv;
//    sleep(2);
//    if (uimInstall->initInfo.installTriggerCallback) {
//        ADM_UIM_SOA_ACTION_AUTH_TYPE_E eActionType;
//        uimInstall->initInfo.installTriggerCallback(&eActionType, uimInstall->initInfoPriv);
//    }
//    return NULL;
//}
//static void uimInstallTrigger(ADM_UIM_INSTALL_T* uimInstall) {
//    pthread_t pth;
//    if (pthread_create(&pth, NULL, uimInstallTriggerHandle, uimInstall) == 0) {
//        pthread_detach(pth);
//    }
//}
//
//static ADM_UIM_INSTALL_T m_uimInstall;
//D32S adm_uim_soaInstallInit(const ADM_UIM_SOA_INSTALL_INIT_INFO_T* initInfo, void* priv)
//{
//    uimInstallInit(&m_uimInstall, initInfo, priv);
//    uimInstallTrigger(&m_uimInstall);
//    return 0;
//}
//
//D32S adm_uim_soaInstallDeinit() {
//    uimInstallDeInit(&m_uimInstall);
//    return 0;
//}

D16U adm_soa_getUclInstanceId()
{
    return 0;
}

D32S adm_dim_ucl_request_Verify_Sign(const D16U instanceId,
    const D16U timeout,
    const DCHAR* fileID,
    const DCHAR* filePath,
    const FILEMETAS_INFO_ARRAY_T* fileMetasList,
    DBOOL isUnpick,
    D32S* errorCode)
{
    *errorCode = 0;
    return 0;
}

D32S adm_uim_installConditionsVerifyResultNotify(const ADM_APPLICATION_COMMON_CMDID_TYPE_E eCmdidType,
    const DCHAR* sessionId,
    const DCHAR* state,
    const ADM_UIM_SOA_CONDITION_DATA_ARRAY_T* condsArrarList,
    const D32S verifyResult)
{
    return 0;
}

D32S adm_uim_policyInstallProgressNotify(const ADM_APPLICATION_COMMON_CMDID_TYPE_E eCmdidType,
    const DCHAR* sessionId,
    const DCHAR* state,
    const ADM_UIM_SOA_INSTALL_PROGRESS_INFO_T* pstPolicyInstallProgress)
{
    return 0;
}

D32S adm_uim_policyInstallResultNotify(const ADM_APPLICATION_COMMON_CMDID_TYPE_E eCmdidType,
    const DCHAR* sessionId,
    const DCHAR* state,
    const ADM_UIM_SOA_INSTALL_RESULT_INFO_T* pstPolicyInstallResult)
{
    return 0;
}

D32S adm_uim_ecuInstallProgressNotify(const ADM_APPLICATION_COMMON_CMDID_TYPE_E eCmdidType,
    const DCHAR* sessionId,
    const DCHAR* state,
    const ADM_UIM_SOA_ECU_IDEN_INFO_T* pstEcuIdenInfo,
    const ADM_UIM_SOA_INSTALL_PROGRESS_INFO_T* pstEcuInstallProgressInfo)
{
    return 0;
}

D32S adm_uim_ecuInstallResultNotify(const ADM_APPLICATION_COMMON_CMDID_TYPE_E eCmdidType,
    const DCHAR* sessionId,
    const DCHAR* state,
    const ADM_UIM_SOA_ECU_IDEN_INFO_T* pstEcuIdenInfo,
    const ADM_UIM_SOA_INSTALL_RESULT_INFO_T* pstEcuInstallResultInfo)
{
    return 0;
}

D32S adm_uim_allEcuInstallResultNotify(const ADM_APPLICATION_COMMON_CMDID_TYPE_E eCmdidType,
    const DCHAR* sessionId,
    const DCHAR* state,
    const ADM_UIM_SOA_ECU_INSTALL_RESULT_ARRAY_INFO_T* pstEcuInstallResultArrayInfo,
    const ADM_UIM_SOA_INSTALL_RESULT_INFO_T* pstPolicyInstallResult)
{
    return 0;
}

D32S adm_uim_soaPolicyInstallProgressNotify(
    const ADM_APPLICATION_COMMON_CMDID_TYPE_E eCmdidType, const DCHAR* sessionId, 
    const DCHAR* state, const ADM_UIM_SOA_INSTALL_PROGRESS_INFO_T* pstPolicyInstallProgress)
{
    // log_d("********************** progress:%d\n", pstPolicyInstallProgress->progress);
    return 0;
}

D32S adm_uim_soaPolicyInstallResultNotify(
    const ADM_APPLICATION_COMMON_CMDID_TYPE_E eCmdidType, const DCHAR* sessionId, 
    const DCHAR* state, const ADM_UIM_SOA_INSTALL_RESULT_INFO_T* pstPolicyInstallResult)
{
    // log_d("********************** result:%d\n", pstPolicyInstallResult->result);
    return 0;
}

D32S adm_uim_soaEcuInstallProgressNotify(
    const ADM_APPLICATION_COMMON_CMDID_TYPE_E eCmdidType, const DCHAR* sessionId, 
    const DCHAR* state, const ADM_UIM_SOA_ECU_IDEN_INFO_T* pstEcuIdenInfo, 
    const ADM_UIM_SOA_INSTALL_PROGRESS_INFO_T* pstEcuInstallProgressInfo)
{
    //log_d("********************** requestId:0X%X, swDid:0X%X, progress:%d\n", 
    //    pstEcuIdenInfo->requestId, pstEcuIdenInfo->swDid, pstEcuInstallProgressInfo->progress);
    return 0;
}

D32S adm_uim_soaEcuInstallResultNotify
(const ADM_APPLICATION_COMMON_CMDID_TYPE_E eCmdidType, const DCHAR* sessionId, 
    const DCHAR* state, const ADM_UIM_SOA_ECU_IDEN_INFO_T* pstEcuIdenInfo, 
    const ADM_UIM_SOA_INSTALL_RESULT_INFO_T* pstEcuInstallResultInfo)
{
    // log_d("********************** requestId:0X%X, swDid:0X%X, result:%d\n", pstEcuIdenInfo->requestId, pstEcuIdenInfo->swDid, pstEcuInstallResultInfo->result);
    return 0;
}

D32S adm_uim_soaInstallConditionsVerifyResultNotify(
    const ADM_APPLICATION_COMMON_CMDID_TYPE_E eCmdidType, const DCHAR* sessionId, 
    const DCHAR* state, const ADM_UIM_SOA_CONDITION_DATA_ARRAY_T* condsArrarList, const D32S verifyResult)
{
    return 0;
}

DCHAR* adm_uim_buildConditionCheckResponse(const ADM_UIM_CHECK_CONDITION_RESULT_T* pInfo)
{
    return "0";
}

///* CORE */
//DCHAR* adm_core_service_getRuntimeDir(const ADM_CORE_SERVICE_RUNTIME_DIR_TYPE_E eType)
//{
//    switch (eType)
//    {
//    case ADM_CORE_SERVICE_RUNTIME_DIR_TYPE_UNZIP:
//        return ADM_IPM_TEST_ENV_DIR"/unzip";
//    }
//
//    return ADM_IPM_TEST_ENV_DIR;
//}

/* CVS */
ADM_CVS_VERSION_POLICY_T* adm_cvs_newPolicyBody() {
    static ADM_CVS_VERSION_POLICY_T policy;
    policy.sessionId = "123";
    policy.taskId = 234;
    policy.strategyId = 345;
    policy.downloadMember.downloadPolicyObjectStr = NULL;
    policy.installMember.installPolicyObjectStr = (DCHAR*)m_installPolicyObjectStr;
    return &policy;
}
void adm_cvs_deletePolicyBody(ADM_CVS_VERSION_POLICY_T* versionPolicy) {
}
D32S adm_cvs_getVersionPolicy(ADM_CVS_VERSION_POLICY_T* versionPolicy) {
    return 0;
}
D32S adm_eim_getEcuInfoManagerKvGet(ADM_EIM_GETECUINFO_T** ecuInfoList)
{
    return 0;
}


int main() {
    //adm_initLog();
    adm_com_createDir("./debug");
    adm_debug_logInit("./debug/", (1 * 1024 * 1024), 5);
    // ADM_IRS_SESSION_T* irsSession = adm_irs_new(NULL, NULL, NULL);

    //adm_ipm_init(/*irsSession*/);
    //adm_pis_packageInit();

    static ADM_DIM_SOA_VDI_INIT_INFO_T vdiInitInfo = {
        .readDiagInfoCallback = NULL,
        .reflashProgressCallback = (ADM_DIM_REFLASH_PROGRESS_NOTIFY_CALLBACK_F)adm_ipm_dimReflashProgressNotify,
        .reflashResultCallback = (ADM_DIM_REFLASH_RESULT_SEND_CALLBACK_F)adm_ipm_dimReflashResultSend,
        .spyVehicleStatusCallback = NULL,
        .vehicleSignalEventCallback = NULL
    };
    adm_dim_soa_vdi_init(NULL, &vdiInitInfo, adm_ipm_dimSoaUcmPrivGet());

    static ADM_DIM_SOA_UCL_INIT_INFO_T ucmInitInfo = {
        .readPartInfoCallback = NULL,
        .upgradeProgressNotifyCallback = adm_ipm_dimSoaUcmUpgradeProgressNotify,
        .upgradeResultNotifyCallback = adm_ipm_dimSoaUcmUpgradeResultNotify,
        .decryptProgressNotifyCallback = NULL,
        .decryptResultNotifyCallback = NULL,
        .verifyPackageHashProgressNotifyCallback = NULL,
        .verifyPackageHashResultNotifyCallback = NULL,
    };
    adm_dim_soa_ucl_init(NULL, &ucmInitInfo, adm_ipm_dimSoaUcmPrivGet());

    
    DCHAR dir[256] = {0};
    IPM_TEST_DIR_READER_T dirReader;

    ADM_CFG_API_INFO_T pstApiData = { 0 };
    pstApiData.isFileExistCb = adm_com_isFileExist;
    pstApiData.writeFileDataCb = adm_com_writeFileData;
    if (ADM_ECODE_SUCCESSED != adm_cfg_iniParserInit(ADM_IPM_TEST_DATA_DIR"/ucmconfig.ini", &pstApiData))
    {
        log_i("read %s/ucmconfig.ini failed", ADM_IPM_TEST_DATA_DIR);
        return 0;
    }

    ipm_test_dir_reader_init(&dirReader, ADM_IPM_TEST_DATA_DIR);

    while (ADM_ECODE_SUCCESSED == ipm_test_dir_reader_next_dir(&dirReader, dir, sizeof(dir)))
    {
        log_i("#################begin install by config[%s]#################", dir);

        if (ADM_ECODE_SUCCESSED != ipm_test_env_init(dir))
        {
            log_i("read config failed from path[%s]", dir);
            continue;
        }
        
        adm_ipm_init(/*irsSession*/);
        adm_pis_packageInit();

        ADM_DIM_SOA_ECU_IDEN_INFO_T pstIdenInfo;
        D16U reflashProgress;
        pstIdenInfo.requestId = 10;
        pstIdenInfo.swDid = 11;
        reflashProgress = 50;
        adm_ipm_dimReflashProgressNotify(&pstIdenInfo, &reflashProgress, adm_ipm_dimSoaUcmPrivGet());

        // ADM_DIM_SOA_ECU_IDEN_INFO_T pstIdenInfo;
        ADM_DIM_SOA_VDI_REFLASH_RESULT_INFO_T pstReflashResultSendInfo;
        pstIdenInfo.requestId = 12;
        pstIdenInfo.swDid = 13;
        pstReflashResultSendInfo.result = 0;
        adm_ipm_dimReflashResultSend(&pstIdenInfo, &pstReflashResultSendInfo, adm_ipm_dimSoaUcmPrivGet());

        void ipmMsgCenterHandleMsgEx();
        ipmMsgCenterHandleMsgEx();

        adm_ipm_install(ADM_APPLICATION_COMMON_CMDID_TYPE_TRIGGER_USER);

        ipm_test_env_destroy();

        adm_ipm_deInit();
        adm_pis_packageDeInit();
        // adm_irs_delete(irsSession);

        log_i("#################end install by config[%s]#################", dir);
    }

    ipm_test_dir_reader_destroy(&dirReader);

    //while (1) {
    //    if ('q' == getchar()) {
    //        break;
    //    }
    //    else {
    //        log_d("enter q exit");
    //    }
    //}

    adm_ipm_deInit();
    adm_pis_packageDeInit();
    // adm_irs_delete(irsSession);

    return 0;
}
