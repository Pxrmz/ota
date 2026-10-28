#include "adm_ips_install.h"
#include "adm_pis_package.h"
#include "gtest/gtest.h"
#include "adm_memory.h"
#include "adm_file.h"
#include "adm_string.h"
//#include "adm_debug.h"
//#include "adm_com_lib.h"

#define ADM_IPM_UPGRADE_PARALLEL_NUM_LIMIT              99999999        // 并行数量限制
#define ADM_IPM_UPGRADE_PARALLEL_LIMIT_M_DEFAULT        ADM_IPM_UPGRADE_PARALLEL_NUM_LIMIT      // 并行数量M
#define ADM_IPM_UPGRADE_PARALLEL_LIMIT_N_DEFAULT        ADM_IPM_UPGRADE_PARALLEL_NUM_LIMIT      // 并行数量N

namespace {

const ADM_CVS_VERSION_POLICY_T* versionPolicyGet(const DCHAR* policyStr);

typedef struct {
    ADM_IPS_INSTALL_ECUINFO_T *ecus;
    D32S ecusLen;
}ADM_IPS_INSTALL_ECUINFO_ARRAY_T;
void ipsInstallEcuinfoArrayInit(ADM_IPS_INSTALL_ECUINFO_ARRAY_T *ecus){
    ecus->ecus = NULL;
    ecus->ecusLen = 0;
}
void ipsInstallEcuinfoArrayDeInit(ADM_IPS_INSTALL_ECUINFO_ARRAY_T *ecus){
    if(ecus->ecus != NULL){
	for(D32S i=0; i<ecus->ecusLen; ++i){
	    adm_ips_installEcuInfoDeInit(ecus->ecus + i);
	}
	adm_free(ecus->ecus);
    }
}
DBOOL ipsInstallEcuinfoArrayCreate(ADM_IPS_INSTALL_ECUINFO_ARRAY_T *ecus, D32S ecusLen){
    DBOOL dRet = 0;
    ecus->ecus = (ADM_IPS_INSTALL_ECUINFO_T *)adm_malloc(ecusLen *sizeof(*(ecus->ecus)));
    if(ecus->ecus != NULL){
	ecus->ecusLen = ecusLen;
	for(D32S i=0; i<ecus->ecusLen; ++i){
	    adm_ips_installEcuInfoInit(ecus->ecus + i);
	}
	dRet = 1;
    }
    return dRet;
}
void ipsInstallEcuinfoArrayClear(ADM_IPS_INSTALL_ECUINFO_ARRAY_T *ecus){
    if(ecus->ecus != NULL){
	for(D32S i=0; i<ecus->ecusLen; ++i){
	    adm_ips_installEcuInfoDeInit(ecus->ecus + i);
	    adm_ips_installEcuInfoInit(ecus->ecus + i);
	}
    }
}

typedef struct
{
    DCHAR* ecuReqId;
    DCHAR* ecuSwDid;
    D32S    progress;
    D32S    result;
    D32S    state;
} IPS_TEST_ECU_UPGRADE_PROCESS_T;

typedef struct
{
    IPS_TEST_ECU_UPGRADE_PROCESS_T* processes;
    D32S    count;
} IPS_TEST_ECU_UPGRADE_PROCESSES_T;

typedef struct
{
    IPS_TEST_ECU_UPGRADE_PROCESSES_T* ecuProcesses;
    D32S    count;
} IPS_TEST_UPGRADE_PROCESSES_T;

typedef struct
{
    DCHAR*  installPolicy;
    D32S    ecuCount;
    D32S    paralellLimitM;
    D32S    paralellLimitN;
    IPS_TEST_UPGRADE_PROCESSES_T    upgradeProcesses;
} IPS_TEST_ECU_UPGRADE_CONTROL_T;

#define RETURN_IF_NULL(p, ret)  if(NULL == (p))\
                                {\
                                    return ret;\
                                }
#define RETURN_IF_NEQ(a, b, ret)    if((a) != (b))\
                                    {\
                                        return ret;\
                                    }


D32S test_ips_ecuUpgradeProcessInitByJson(const json_object* root, IPS_TEST_ECU_UPGRADE_PROCESS_T* progress)
{
    progress->ecuReqId = adm_strdup(json_object_get_string(json_object_object_get(root, "ecuReqId")));
    RETURN_IF_NULL(progress->ecuReqId, ADM_ECODE_FAILED);
    progress->ecuSwDid = adm_strdup(json_object_get_string(json_object_object_get(root, "ecuSwDid")));
    RETURN_IF_NULL(progress->ecuSwDid, ADM_ECODE_FAILED);
    progress->progress = json_object_get_int(json_object_object_get(root, "progress"));
    progress->result = json_object_get_int(json_object_object_get(root, "result"));
    progress->state = json_object_get_int(json_object_object_get(root, "state"));

    return ADM_ECODE_SUCCESSED;
}

void test_ips_ecuUpgradeProcessDeinit(IPS_TEST_ECU_UPGRADE_PROCESS_T* progress)
{
    adm_free(progress->ecuReqId);
    adm_free(progress->ecuSwDid);
}

D32S test_ips_ecuUpgradeProcessesInitByJson(const json_object* root, IPS_TEST_ECU_UPGRADE_PROCESSES_T* upgradeProcesses)
{
    D32S ret = ADM_ECODE_FAILED;
    D32S count = json_object_array_length(root);
    if (count > 0)
    {
        IPS_TEST_ECU_UPGRADE_PROCESS_T* processes = (IPS_TEST_ECU_UPGRADE_PROCESS_T*)adm_malloc(sizeof(IPS_TEST_ECU_UPGRADE_PROCESS_T) * count);
        RETURN_IF_NULL(processes, ADM_ECODE_FAILED);

        for (D32S i = 0; i < count; i++)
        {
            RETURN_IF_NEQ(ADM_ECODE_SUCCESSED,
                test_ips_ecuUpgradeProcessInitByJson(json_object_array_get_idx(root, i), &processes[i]),
                ADM_ECODE_FAILED);
        }

        upgradeProcesses->processes = processes;
        upgradeProcesses->count = count;
        ret = ADM_ECODE_SUCCESSED;
    }

    return ret;
}

void test_ips_ecuUpgradeProcessesDeinit(IPS_TEST_ECU_UPGRADE_PROCESSES_T* upgradeProcesses)
{
    IPS_TEST_ECU_UPGRADE_PROCESS_T* processes = upgradeProcesses->processes;
    for (D32S i = 0; i < upgradeProcesses->count; i++)
    {
        test_ips_ecuUpgradeProcessDeinit(&processes[i]);
    }

    adm_free(processes);
}

D32S test_ips_upgradeProcessesInitByJson(const json_object* root, IPS_TEST_UPGRADE_PROCESSES_T* upgradeProcesses)
{
    D32S ret = ADM_ECODE_FAILED;
    D32S count = json_object_array_length(root);
    if (count > 0)
    {
        IPS_TEST_ECU_UPGRADE_PROCESSES_T* processes = (IPS_TEST_ECU_UPGRADE_PROCESSES_T*)adm_malloc(sizeof(IPS_TEST_ECU_UPGRADE_PROCESSES_T) * count);
        RETURN_IF_NULL(processes, ADM_ECODE_FAILED);

        for (D32S i = 0; i < count; i++)
        {
            RETURN_IF_NEQ(ADM_ECODE_SUCCESSED,
                test_ips_ecuUpgradeProcessesInitByJson(json_object_array_get_idx(root, i), &processes[i]),
                ADM_ECODE_FAILED);
        }

        upgradeProcesses->ecuProcesses = processes;
        upgradeProcesses->count = count;
        ret = ADM_ECODE_SUCCESSED;
    }

    return ret;
}

void test_ips_upgradeProcessesDeinit(IPS_TEST_UPGRADE_PROCESSES_T* upgradeProcesses)
{
    IPS_TEST_ECU_UPGRADE_PROCESSES_T* processes = upgradeProcesses->ecuProcesses;
    for (D32S i = 0; i < upgradeProcesses->count; i++)
    {
        test_ips_ecuUpgradeProcessesDeinit(&processes[i]);
    }

    adm_free(processes);
}

D32S test_ips_ecuUpgradeControlInitByJson(const json_object* root, IPS_TEST_ECU_UPGRADE_CONTROL_T* control)
{
    json_object* obj = json_object_object_get(root, "installPolicy");
    
    json_object* ecuGroupOrders = json_object_object_get(obj, "ecuGroupOrders");
    D32S ecuGroupOrderCount = json_object_array_length(ecuGroupOrders);
    D32S ecuCount = 0;
    for (D32S i = 0; i < ecuGroupOrderCount; i++)
    {
        ecuCount += json_object_array_length(json_object_object_get(json_object_array_get_idx(ecuGroupOrders, i), "groupEcus"));
    }
    
    control->ecuCount = ecuCount;
    control->installPolicy = adm_strdup(json_object_to_json_string(obj));
    RETURN_IF_NULL(control->installPolicy, ADM_ECODE_FAILED);

    obj = json_object_object_get(root, "paralellLimitM");
    control->paralellLimitM = NULL != obj ? json_object_get_int(obj) : ADM_IPM_UPGRADE_PARALLEL_LIMIT_M_DEFAULT;
    obj = json_object_object_get(root, "paralellLimitN");
    control->paralellLimitN = NULL != obj ? json_object_get_int(obj) : ADM_IPM_UPGRADE_PARALLEL_LIMIT_N_DEFAULT;

    return test_ips_upgradeProcessesInitByJson(json_object_object_get(root, "upgradeProcesses"), &control->upgradeProcesses);
}

void test_ips_ecuUpgradeControlDeinit(IPS_TEST_ECU_UPGRADE_CONTROL_T* control)
{
    adm_free(control->installPolicy);
    test_ips_upgradeProcessesDeinit(&control->upgradeProcesses);
}

D32S test_ips_ecuUpgradeProcessInitByStr(const DCHAR* str, IPS_TEST_ECU_UPGRADE_CONTROL_T* control)
{
    json_object* root = json_tokener_parse(str);
    RETURN_IF_NULL(root, ADM_ECODE_FAILED);

    D32S ret = test_ips_ecuUpgradeControlInitByJson(root, control);
    json_object_put(root);

    return ret;
}

D32U test_ips_readFile(const DCHAR* name, DCHAR* buf, D32S bufSize)
{
    D32U size = 0;
    void* file = NULL;

    adm_fopen(name, "a+", &file);
    RETURN_IF_NULL(file, ADM_ECODE_FAILED);

    adm_fread(buf, bufSize - 1, &size, file);
    adm_fclose(&file);
    buf[size] = '\0';

    return size;
}

D32S test_ips_ecuUpgradeControlInitByFile(const DCHAR* fileName, IPS_TEST_ECU_UPGRADE_CONTROL_T* control)
{
    D32S ret = ADM_ECODE_FAILED;
    D32U fileSize = 0;

    RETURN_IF_NEQ(ADM_ERR_OK, adm_fgetfilesize(fileName, &fileSize), ADM_ECODE_FAILED);

    DCHAR* buf = (DCHAR*)adm_malloc(fileSize + 1);
    RETURN_IF_NULL(buf, ADM_ECODE_FAILED);

    do
    {
        if (0 == test_ips_readFile(fileName, buf, fileSize + 1))
        {
            break;
        }

        ret = test_ips_ecuUpgradeProcessInitByStr(buf, control);
    } while (0);

    adm_free(buf);

    return ret;
}

D32S calculateTotalProgress()
{
    D32S totalProgress = 0;
    ADM_IPS_PROGRESS_T progress;
    adm_ips_progressInit(&progress);
    adm_ips_progressGet(&progress);

    ADM_IPS_ECU_PROGRESS_T* ecuProgresses = progress.ecus.ecus;

    for (D32S i = 0; i < progress.ecus.ecusLen; ++i) {
        D32U A = ecuProgresses[i].progress[ADM_IPS_ECU_PHASE_INDEX_UPGRADE].progress;
        D32U B = ecuProgresses[i].progress[ADM_IPS_ECU_PHASE_INDEX_ROLLBACK].progress;
        totalProgress += ((B != 0) ? B : A) * ecuProgresses[i].precent;
    }

    return totalProgress / 100;
}

void test_ips_testCase(const DCHAR* fileName)
{
    IPS_TEST_ECU_UPGRADE_CONTROL_T control;
    test_ips_ecuUpgradeControlInitByFile(fileName, &control);

    ADM_IPS_PROGRESS_T progress;
    ADM_IPS_INSTALL_ECUINFO_ARRAY_T ecusInfo;

    adm_ips_init();
    adm_ips_stepInstallPrepare(versionPolicyGet(control.installPolicy), control.paralellLimitM, control.paralellLimitN);

    D32S maxEcusLen = adm_ips_maxEcusCountGet();
    ASSERT_EQ(control.ecuCount, maxEcusLen);
    ipsInstallEcuinfoArrayInit(&ecusInfo);
    ASSERT_EQ(1, ipsInstallEcuinfoArrayCreate(&ecusInfo, maxEcusLen));

    adm_ips_stepInstallStart();

    adm_ips_progressInit(&progress);
    adm_ips_progressGet(&progress);
    EXPECT_EQ(ADM_IPS_PHASE_INSTALL, progress.state);
    EXPECT_EQ(0, progress.progress);
    EXPECT_EQ(control.ecuCount, progress.ecus.ecusLen);
    adm_ips_progressDeInit(&progress);

    IPS_TEST_UPGRADE_PROCESSES_T* upgradeProcesses = &control.upgradeProcesses;
    IPS_TEST_ECU_UPGRADE_PROCESSES_T* ecuProcesses = upgradeProcesses->ecuProcesses;
    IPS_TEST_ECU_UPGRADE_PROCESSES_T* ecuProcess = NULL;
    IPS_TEST_ECU_UPGRADE_PROCESS_T* processes = NULL;
    IPS_TEST_ECU_UPGRADE_PROCESS_T* process = NULL;
    for (D32S i = 0; i < upgradeProcesses->count; i++)
    {
        ecuProcess = &ecuProcesses[i];
        ASSERT_EQ(ecuProcess->count, adm_ips_installEcuInfoArrayGet(ecusInfo.ecus, maxEcusLen));

        for (D32S j = 0; j < ecuProcess->count; j++)
        {
            ADM_IPS_ECU_CURRENT_PROGRESS_T curEcuProgress;
            ADM_IPS_ECU_PROGRESS_T ecuProgress;

            process = &ecuProcess->processes[j];
            ADM_IPS_INSTALL_ECUINFO_T* ecu = &ecusInfo.ecus[j];

            ASSERT_NE(nullptr, ecu->ecuReqId);
            EXPECT_STREQ(process->ecuReqId, ecu->ecuReqId);
            ASSERT_NE(nullptr, ecu->ecuSwDid);
            EXPECT_STREQ(process->ecuSwDid, ecu->ecuSwDid);

            adm_ips_ecuProgressInit(&ecuProgress);
            adm_ips_ecuProgressGet(&ecuProgress, ecu->ecuReqId, ecu->ecuSwDid);
            ADM_IPS_ECU_PHASE_E state = ecuProgress.state;
            adm_ips_ecuProgressDeInit(&ecuProgress);

            curEcuProgress.ecuReqId = ecu->ecuReqId;
            curEcuProgress.ecuSwDid = ecu->ecuSwDid;
            curEcuProgress.progress = process->progress;
            curEcuProgress.isEnd = 0;
            curEcuProgress.result = process->result;
            adm_ips_currentProgressSet(&curEcuProgress);

            D32S isEnd = 0;
            if (0 != process->result || 100 == process->progress)
            {
                curEcuProgress.isEnd = 1;
                adm_ips_currentProgressSet(&curEcuProgress);
            }

            adm_ips_ecuProgressInit(&ecuProgress);
            adm_ips_ecuProgressGet(&ecuProgress, curEcuProgress.ecuReqId, curEcuProgress.ecuSwDid);
            if (ADM_IPS_ECU_PHASE_INSTALL == state)
            {
                EXPECT_EQ(process->progress, ecuProgress.progress[ADM_IPS_ECU_PHASE_INDEX_UPGRADE].progress);
                if (1 == curEcuProgress.isEnd)
                {
                    EXPECT_EQ(process->result, ecuProgress.progress[ADM_IPS_ECU_PHASE_INDEX_UPGRADE].result);
                }
            }
            else if(ADM_IPS_ECU_PHASE_INSTALL_ROLLBACK == state)
            {
                EXPECT_EQ(process->progress, ecuProgress.progress[ADM_IPS_ECU_PHASE_INDEX_ROLLBACK].progress);
                if (1 == curEcuProgress.isEnd)
                {
                    EXPECT_EQ(process->result, ecuProgress.progress[ADM_IPS_ECU_PHASE_INDEX_ROLLBACK].result);
                }
            }

            EXPECT_EQ(process->state, ecuProgress.state);
            adm_ips_ecuProgressDeInit(&ecuProgress);
        }

        ipsInstallEcuinfoArrayClear(&ecusInfo);
    }

    adm_ips_stepPostInstall(1000);

    adm_ips_progressInit(&progress);
    adm_ips_progressGet(&progress);
    EXPECT_EQ(ADM_IPS_PHASE_POST_INSTALL, progress.state);
    D32S totalProgress = calculateTotalProgress();
    EXPECT_EQ(totalProgress, progress.progress);
    EXPECT_EQ(1000, progress.result);
    adm_ips_progressDeInit(&progress);

    ipsInstallEcuinfoArrayDeInit(&ecusInfo);
    test_ips_ecuUpgradeControlDeinit(&control);
}


class IpsTest : public testing::Test {
protected:
  void SetUp() override {
      //adm_debug_logInit("/tmp/debug/", (1 * 1024 * 1024), 5);
      adm_ips_init();
      adm_pis_packageInit();
  }
  void TearDown() override {
      adm_ips_deInit();
      adm_pis_packageDeInit();
  }
};

#ifdef ADM_IPS_POLICY_UPGRADE_ORDER_OLD
TEST_F(IpsTest, Normal){
    const DCHAR* policyStr = R"({
  "upgradeAutoStart": "xxx",
  "upgradeRetryCount": 1,
  "rollbackRetryCount": 1,
  "upgradeConditions": [
    {
      "type": 1,
      "conditions": [
        {
          "messageId": "xxx",
          "messageName": "xxx",
          "messageDesc": "xxx",
          "expressions": [
            {
              "logic": 1,
              "dataType": 1,
              "dataLength": 3,
              "data": "101",
              "connectRelation": 1
            }
          ],
          "connectRelation": 1,
          "onFailStrategy": {
            "type": 1,
            "timeout": 30,
            "action": 1,
            "expressions": [
              {
                "logic": 1,
                "dataType": 2,
                "dataLength": 4,
                "data": "1001",
                "connectRelation": 1
              }
            ]
          }
        }
      ]
    }
  ],
  "totalTimeConsuming": 8,
  "dependencies": [
    {
      "type": 1,
      "ecuReqId": "0X700",
      "ecuSwDid": "0XF187",
      "ecuDids": [
        {
          "did": "0XF187",
          "didValue": "V1.0"
        }
      ]
    },
    {
      "type": 2,
      "ecuReqId": "0X700",
      "ecuSwDid": "0XF187",
      "ecuDids": [
        {
          "did": "0XF187",
          "didValue": "V2.0"
        }
      ]
    },
    {
      "type": 1,
      "ecuReqId": "0X7B4",
      "ecuSwDid": "0XF187",
      "ecuDids": [
        {
          "did": "0XF187",
          "didValue": "V1.0"
        }
      ]
    },
    {
      "type": 2,
      "ecuReqId": "0X7B4",
      "ecuSwDid": "0XF187",
      "ecuDids": [
        {
          "did": "0XF187",
          "didValue": "V2.0"
        }
      ]
    }
  ],
  "ecuGroupOrders": [
    {
      "groupFlag": 1,
      "actionForFailed": 1,
      "groupEcus": [
        {
          "ecuReqId": "0X700",
          "ecuSwDid": "0XF187",
          "upgradeOrder": 1,
          "ecuTimeConsuming": 2,
          "ecuTimeout": 4
        }
      ]
    },
    {
      "groupFlag": 2,
      "actionForFailed": 2,
      "groupEcus": [
        {
          "ecuReqId": "0X7B4",
          "ecuSwDid": "0XF187",
          "upgradeOrder": 2,
          "ecuTimeConsuming": 6,
          "ecuTimeout": 12
        }
      ]
    }
  ]
})";
    ADM_IPS_PROGRESS_T progress;
    ADM_IPS_INSTALL_ECUINFO_ARRAY_T ecusInfo;
    ADM_IPS_ECU_CURRENT_PROGRESS_T curEcuProgress;
    ADM_IPS_ECU_PROGRESS_T ecuProgress;

    adm_ips_stepInstallPrepare(versionPolicyGet(policyStr), 999999, 999999);

    D32S maxEcusLen = adm_ips_maxEcusCountGet();
    ASSERT_EQ(2, maxEcusLen);
    ipsInstallEcuinfoArrayInit(&ecusInfo);
    ASSERT_EQ(1, ipsInstallEcuinfoArrayCreate(&ecusInfo, maxEcusLen));

    adm_ips_stepInstallStart();

    adm_ips_progressInit(&progress);
    adm_ips_progressGet(&progress);
    EXPECT_EQ(ADM_IPS_PHASE_INSTALL, progress.state);
    EXPECT_EQ(0, progress.progress);
    EXPECT_EQ(2, progress.ecus.ecusLen);
    adm_ips_progressDeInit(&progress);

    ipsInstallEcuinfoArrayClear(&ecusInfo);
    ASSERT_EQ(1, adm_ips_installEcuInfoArrayGet(ecusInfo.ecus));
    ASSERT_NE(nullptr, ecusInfo.ecus[0].ecuReqId);
    EXPECT_STREQ("0X700", ecusInfo.ecus[0].ecuReqId);
    ASSERT_NE(nullptr, ecusInfo.ecus[0].ecuSwDid);
    EXPECT_STREQ("0XF187", ecusInfo.ecus[0].ecuSwDid);

    curEcuProgress.ecuReqId = ecusInfo.ecus[0].ecuReqId;
    curEcuProgress.ecuSwDid = ecusInfo.ecus[0].ecuSwDid;
    curEcuProgress.isEnd = 0;
    curEcuProgress.progress = 60;
    adm_ips_currentProgressSet(&curEcuProgress);
    adm_ips_ecuProgressInit(&ecuProgress);
    adm_ips_ecuProgressGet(&ecuProgress, curEcuProgress.ecuReqId, curEcuProgress.ecuSwDid);
    EXPECT_EQ(ADM_IPS_ECU_PHASE_INSTALL, ecuProgress.state);
    EXPECT_EQ(60, ecuProgress.progress[0].progress);
    adm_ips_ecuProgressDeInit(&ecuProgress);

    curEcuProgress.progress = 100;
    adm_ips_currentProgressSet(&curEcuProgress);
    adm_ips_ecuProgressInit(&ecuProgress);
    adm_ips_ecuProgressGet(&ecuProgress, curEcuProgress.ecuReqId, curEcuProgress.ecuSwDid);
    EXPECT_EQ(100, ecuProgress.progress[0].progress);
    adm_ips_ecuProgressDeInit(&ecuProgress);

    curEcuProgress.isEnd = 1;
    curEcuProgress.result = 0;
    adm_ips_currentProgressSet(&curEcuProgress);
    adm_ips_ecuProgressInit(&ecuProgress);
    adm_ips_ecuProgressGet(&ecuProgress, curEcuProgress.ecuReqId, curEcuProgress.ecuSwDid);
    EXPECT_EQ(ADM_IPS_ECU_PHASE_POST_INSTALL, ecuProgress.state);
    EXPECT_EQ(0, ecuProgress.progress[0].result);
    adm_ips_ecuProgressDeInit(&ecuProgress);

    ipsInstallEcuinfoArrayClear(&ecusInfo);
    ASSERT_EQ(1, adm_ips_installEcuInfoArrayGet(ecusInfo.ecus));
    ASSERT_NE(nullptr, ecusInfo.ecus[0].ecuReqId);
    EXPECT_STREQ("0X7B4", ecusInfo.ecus[0].ecuReqId);
    ASSERT_NE(nullptr, ecusInfo.ecus[0].ecuSwDid);
    EXPECT_STREQ("0XF187", ecusInfo.ecus[0].ecuSwDid);

    curEcuProgress.ecuReqId = ecusInfo.ecus[0].ecuReqId;
    curEcuProgress.ecuSwDid = ecusInfo.ecus[0].ecuSwDid;
    curEcuProgress.isEnd = 0;
    curEcuProgress.progress = 100;
    adm_ips_currentProgressSet(&curEcuProgress);
    adm_ips_ecuProgressInit(&ecuProgress);
    adm_ips_ecuProgressGet(&ecuProgress, curEcuProgress.ecuReqId, curEcuProgress.ecuSwDid);
    EXPECT_EQ(ADM_IPS_ECU_PHASE_INSTALL, ecuProgress.state);
    EXPECT_EQ(100, ecuProgress.progress[0].progress);
    adm_ips_ecuProgressDeInit(&ecuProgress);

    curEcuProgress.isEnd = 1;
    curEcuProgress.result = 0;
    adm_ips_currentProgressSet(&curEcuProgress);
    adm_ips_ecuProgressInit(&ecuProgress);
    adm_ips_ecuProgressGet(&ecuProgress, curEcuProgress.ecuReqId, curEcuProgress.ecuSwDid);
    EXPECT_EQ(ADM_IPS_ECU_PHASE_POST_INSTALL, ecuProgress.state);
    EXPECT_EQ(0, ecuProgress.progress[0].result);
    adm_ips_ecuProgressDeInit(&ecuProgress);

    adm_ips_stepPostInstall(1000);

    adm_ips_progressInit(&progress);
    adm_ips_progressGet(&progress);
    EXPECT_EQ(ADM_IPS_PHASE_POST_INSTALL, progress.state);
    EXPECT_EQ(100, progress.progress);
    EXPECT_EQ(1000, progress.result);
    adm_ips_progressDeInit(&progress);

    ipsInstallEcuinfoArrayDeInit(&ecusInfo);

    ADM_COND_POLICY_T cond;
    adm_condPolicyInit(&cond);
    adm_ips_policyConditionGet(1, &cond);
    ASSERT_EQ(1, cond.iCondNum);
    adm_condPolicyDeInit(&cond);
}
#else
#if (!defined(UPGRADE_GROUP_POLICY_VERSION) || 1 == UPGRADE_GROUP_POLICY_VERSION)
TEST_F(IpsTest, Normal){
    const DCHAR* policyStr = R"({
  "upgradeAutoStart": "xxx",
  "upgradeRetryCount" : 1,
  "rollbackRetryCount" : 1,
  "upgradeConditions" : [
    {
      "type": 1,
      "conditions" : [
        {
          "messageId": "xxx",
          "messageName" : "xxx",
          "messageDesc" : "xxx",
          "expressions" : [
            {
              "logic": 1,
              "dataType" : 1,
              "dataLength" : 3,
              "data" : "101",
              "connectRelation" : 1
            }
          ] ,
          "connectRelation": 1,
          "onFailStrategy" : {
            "type": 1,
            "timeout" : 30,
            "action" : 1,
            "expressions" : [
              {
                "logic": 1,
                "dataType" : 2,
                "dataLength" : 4,
                "data" : "1001",
                "connectRelation" : 1
              }
            ]
          }
        }
      ]
    }
  ],
  "totalTimeConsuming": 8,
  "dependencies" : [
    {
      "type": 1,
      "ecuReqId" : "0X700",
      "ecuSwDid" : "0XF187",
      "ecuDids" : [
        {
          "did": "0XF187",
          "didValue" : "V1.0"
        }
      ]
    },
    {
      "type": 2,
      "ecuReqId" : "0X700",
      "ecuSwDid" : "0XF187",
      "ecuDids" : [
        {
          "did": "0XF187",
          "didValue" : "V2.0"
        }
      ]
    },
    {
      "type": 1,
      "ecuReqId" : "0X701",
      "ecuSwDid" : "0XF187",
      "ecuDids" : [
        {
          "did": "0XF187",
          "didValue" : "V1.0"
        }
      ]
    },
    {
      "type": 2,
      "ecuReqId" : "0X701",
      "ecuSwDid" : "0XF187",
      "ecuDids" : [
        {
          "did": "0XF187",
          "didValue" : "V2.0"
        }
      ]
    },
    {
      "type": 1,
      "ecuReqId" : "0X702",
      "ecuSwDid" : "0XF187",
      "ecuDids" : [
        {
          "did": "0XF187",
          "didValue" : "V1.0"
        }
      ]
    },
    {
      "type": 2,
      "ecuReqId" : "0X702",
      "ecuSwDid" : "0XF187",
      "ecuDids" : [
        {
          "did": "0XF187",
          "didValue" : "V2.0"
        }
      ]
    },
    {
      "type": 1,
      "ecuReqId" : "0X7B4",
      "ecuSwDid" : "0XF187",
      "ecuDids" : [
        {
          "did": "0XF187",
          "didValue" : "V1.0"
        }
      ]
    },
    {
      "type": 2,
      "ecuReqId" : "0X7B4",
      "ecuSwDid" : "0XF187",
      "ecuDids" : [
        {
          "did": "0XF187",
          "didValue" : "V2.0"
        }
      ]
    }
  ],
  "ecuGroupOrders": [
    {
      "groupFlag": 1,
      "actionForFailed" : 1,
      "groupEcus" : [
        {
          "ecuReqId": "0X700",
          "ecuSwDid" : "0XF187",
          "upgradeOrder" : "0x112233",
          "ecuTimeConsuming" : 2,
          "ecuTimeout" : 4
        },
        {
          "ecuReqId": "0X701",
          "ecuSwDid" : "0XF187",
          "upgradeOrder" : "0x112234",
          "ecuTimeConsuming" : 2,
          "ecuTimeout" : 4
        },
        {
          "ecuReqId": "0X702",
          "ecuSwDid" : "0XF187",
          "upgradeOrder" : "0x122233",
          "ecuTimeConsuming" : 2,
          "ecuTimeout" : 4
        }
      ]
    },
    {
      "groupFlag": 2,
      "actionForFailed" : 2,
      "groupEcus" : [
        {
          "ecuReqId": "0X7B4",
          "ecuSwDid" : "0XF187",
          "upgradeOrder" : "0x112233",
          "ecuTimeConsuming" : 6,
          "ecuTimeout" : 12
        },
        {
          "ecuReqId": "0X7B5",
          "ecuSwDid" : "0XF187",
          "upgradeOrder" : "0x112501",
          "ecuTimeConsuming" : 6,
          "ecuTimeout" : 12
        }
      ]
    }
  ]
 })";
    ADM_IPS_PROGRESS_T progress;
    ADM_IPS_INSTALL_ECUINFO_ARRAY_T ecusInfo;
    ADM_IPS_ECU_CURRENT_PROGRESS_T curEcuProgress;
    ADM_IPS_ECU_PROGRESS_T ecuProgress;

    adm_ips_stepInstallPrepare(versionPolicyGet(policyStr), 999999, 999999);

    D32S maxEcusLen = adm_ips_maxEcusCountGet();
    ASSERT_EQ(5, maxEcusLen);
    ipsInstallEcuinfoArrayInit(&ecusInfo);
    ASSERT_EQ(1, ipsInstallEcuinfoArrayCreate(&ecusInfo, maxEcusLen));

    adm_ips_stepInstallStart();

    adm_ips_progressInit(&progress);
    adm_ips_progressGet(&progress);
    EXPECT_EQ(ADM_IPS_PHASE_INSTALL, progress.state);
    EXPECT_EQ(0, progress.progress);
    EXPECT_EQ(5, progress.ecus.ecusLen);
    adm_ips_progressDeInit(&progress);

    ipsInstallEcuinfoArrayClear(&ecusInfo);
    ASSERT_EQ(3, adm_ips_installEcuInfoArrayGet(ecusInfo.ecus, maxEcusLen));
    ASSERT_NE(nullptr, ecusInfo.ecus[0].ecuReqId);
    EXPECT_STREQ("0X700", ecusInfo.ecus[0].ecuReqId);
    ASSERT_NE(nullptr, ecusInfo.ecus[0].ecuSwDid);
    EXPECT_STREQ("0XF187", ecusInfo.ecus[0].ecuSwDid);
    ASSERT_NE(nullptr, ecusInfo.ecus[1].ecuReqId);
    EXPECT_STREQ("0X7B4", ecusInfo.ecus[1].ecuReqId);
    ASSERT_NE(nullptr, ecusInfo.ecus[2].ecuReqId);
    EXPECT_STREQ("0X7B5", ecusInfo.ecus[2].ecuReqId);

    curEcuProgress.ecuReqId = ecusInfo.ecus[0].ecuReqId;
    curEcuProgress.ecuSwDid = ecusInfo.ecus[0].ecuSwDid;
    curEcuProgress.isEnd = 0;
    curEcuProgress.progress = 60;
    adm_ips_currentProgressSet(&curEcuProgress);
    adm_ips_ecuProgressInit(&ecuProgress);
    adm_ips_ecuProgressGet(&ecuProgress, curEcuProgress.ecuReqId, curEcuProgress.ecuSwDid);
    EXPECT_EQ(ADM_IPS_ECU_PHASE_INSTALL, ecuProgress.state);
    EXPECT_EQ(60, ecuProgress.progress[0].progress);
    adm_ips_ecuProgressDeInit(&ecuProgress);

    curEcuProgress.progress = 100;
    adm_ips_currentProgressSet(&curEcuProgress);
    adm_ips_ecuProgressInit(&ecuProgress);
    adm_ips_ecuProgressGet(&ecuProgress, curEcuProgress.ecuReqId, curEcuProgress.ecuSwDid);
    EXPECT_EQ(100, ecuProgress.progress[0].progress);
    adm_ips_ecuProgressDeInit(&ecuProgress);

    curEcuProgress.isEnd = 1;
    curEcuProgress.result = 0;
    adm_ips_currentProgressSet(&curEcuProgress);
    adm_ips_ecuProgressInit(&ecuProgress);
    adm_ips_ecuProgressGet(&ecuProgress, curEcuProgress.ecuReqId, curEcuProgress.ecuSwDid);
    EXPECT_EQ(ADM_IPS_ECU_PHASE_POST_INSTALL, ecuProgress.state);
    EXPECT_EQ(0, ecuProgress.progress[0].result);
    adm_ips_ecuProgressDeInit(&ecuProgress);

    ipsInstallEcuinfoArrayClear(&ecusInfo);
    ASSERT_EQ(2, adm_ips_installEcuInfoArrayGet(ecusInfo.ecus, maxEcusLen));
    ASSERT_NE(nullptr, ecusInfo.ecus[0].ecuReqId);
    EXPECT_STREQ("0X7B4", ecusInfo.ecus[0].ecuReqId);
    ASSERT_NE(nullptr, ecusInfo.ecus[0].ecuSwDid);
    EXPECT_STREQ("0XF187", ecusInfo.ecus[0].ecuSwDid);
    ASSERT_NE(nullptr, ecusInfo.ecus[1].ecuReqId);
    EXPECT_STREQ("0X7B5", ecusInfo.ecus[1].ecuReqId);

    curEcuProgress.ecuReqId = ecusInfo.ecus[0].ecuReqId;
    curEcuProgress.ecuSwDid = ecusInfo.ecus[0].ecuSwDid;
    curEcuProgress.isEnd = 0;
    curEcuProgress.progress = 100;
    adm_ips_currentProgressSet(&curEcuProgress);
    adm_ips_ecuProgressInit(&ecuProgress);
    adm_ips_ecuProgressGet(&ecuProgress, curEcuProgress.ecuReqId, curEcuProgress.ecuSwDid);
    EXPECT_EQ(ADM_IPS_ECU_PHASE_INSTALL, ecuProgress.state);
    EXPECT_EQ(100, ecuProgress.progress[0].progress);
    adm_ips_ecuProgressDeInit(&ecuProgress);

    curEcuProgress.isEnd = 1;
    curEcuProgress.result = 0;
    adm_ips_currentProgressSet(&curEcuProgress);
    adm_ips_ecuProgressInit(&ecuProgress);
    adm_ips_ecuProgressGet(&ecuProgress, curEcuProgress.ecuReqId, curEcuProgress.ecuSwDid);
    EXPECT_EQ(ADM_IPS_ECU_PHASE_POST_INSTALL, ecuProgress.state);
    EXPECT_EQ(0, ecuProgress.progress[0].result);
    adm_ips_ecuProgressDeInit(&ecuProgress);

    ipsInstallEcuinfoArrayClear(&ecusInfo);
    ASSERT_EQ(2, adm_ips_installEcuInfoArrayGet(ecusInfo.ecus, maxEcusLen));
    ASSERT_NE(nullptr, ecusInfo.ecus[0].ecuReqId);
    EXPECT_STREQ("0X701", ecusInfo.ecus[0].ecuReqId);
    ASSERT_NE(nullptr, ecusInfo.ecus[0].ecuSwDid);
    EXPECT_STREQ("0XF187", ecusInfo.ecus[0].ecuSwDid);
    ASSERT_NE(nullptr, ecusInfo.ecus[1].ecuReqId);
    EXPECT_STREQ("0X7B5", ecusInfo.ecus[1].ecuReqId);

    curEcuProgress.ecuReqId = ecusInfo.ecus[0].ecuReqId;
    curEcuProgress.ecuSwDid = ecusInfo.ecus[0].ecuSwDid;
    curEcuProgress.isEnd = 0;
    curEcuProgress.progress = 100;
    adm_ips_currentProgressSet(&curEcuProgress);

    curEcuProgress.isEnd = 1;
    curEcuProgress.result = 0;
    adm_ips_currentProgressSet(&curEcuProgress);

    curEcuProgress.ecuReqId = ecusInfo.ecus[1].ecuReqId;
    curEcuProgress.ecuSwDid = ecusInfo.ecus[1].ecuSwDid;
    curEcuProgress.isEnd = 0;
    curEcuProgress.progress = 100;
    adm_ips_currentProgressSet(&curEcuProgress);

    curEcuProgress.isEnd = 1;
    curEcuProgress.result = 0;
    adm_ips_currentProgressSet(&curEcuProgress);

    ipsInstallEcuinfoArrayClear(&ecusInfo);
    ASSERT_EQ(1, adm_ips_installEcuInfoArrayGet(ecusInfo.ecus, maxEcusLen));
    ASSERT_NE(nullptr, ecusInfo.ecus[0].ecuReqId);
    EXPECT_STREQ("0X702", ecusInfo.ecus[0].ecuReqId);
    ASSERT_NE(nullptr, ecusInfo.ecus[0].ecuSwDid);
    EXPECT_STREQ("0XF187", ecusInfo.ecus[0].ecuSwDid);

    curEcuProgress.ecuReqId = ecusInfo.ecus[0].ecuReqId;
    curEcuProgress.ecuSwDid = ecusInfo.ecus[0].ecuSwDid;
    curEcuProgress.isEnd = 0;
    curEcuProgress.progress = 100;
    adm_ips_currentProgressSet(&curEcuProgress);

    curEcuProgress.isEnd = 1;
    curEcuProgress.result = 0;
    adm_ips_currentProgressSet(&curEcuProgress);

    ipsInstallEcuinfoArrayClear(&ecusInfo);
    ASSERT_EQ(0, adm_ips_installEcuInfoArrayGet(ecusInfo.ecus, maxEcusLen));

    adm_ips_stepPostInstall(1000);

    adm_ips_progressInit(&progress);
    adm_ips_progressGet(&progress);
    EXPECT_EQ(ADM_IPS_PHASE_POST_INSTALL, progress.state);
    EXPECT_EQ(100, progress.progress);
    EXPECT_EQ(1000, progress.result);
    adm_ips_progressDeInit(&progress);

    ipsInstallEcuinfoArrayDeInit(&ecusInfo);

    ADM_COND_POLICY_T cond;
    adm_condPolicyInit(&cond);
    adm_ips_policyConditionGet(1, &cond);
    ASSERT_EQ(1, cond.iCondNum);
    adm_condPolicyDeInit(&cond);
}
#elif(2 == UPGRADE_GROUP_POLICY_VERSION)

#elif(3 == UPGRADE_GROUP_POLICY_VERSION)
//TEST_F(IpsTest, Normal) {
//    const DCHAR* policyStr = R"({
//  "upgradeAutoStart": "xxx",
//  "upgradeRetryCount" : 1,
//  "rollbackRetryCount" : 1,
//  "upgradeConditions" : [
//    {
//      "type": 1,
//      "conditions" : [
//        {
//          "messageId": "xxx",
//          "messageName" : "xxx",
//          "messageDesc" : "xxx",
//          "expressions" : [
//            {
//              "logic": 1,
//              "dataType" : 1,
//              "dataLength" : 3,
//              "data" : "101",
//              "connectRelation" : 1
//            }
//          ] ,
//          "connectRelation": 1,
//          "onFailStrategy" : {
//            "type": 1,
//            "timeout" : 30,
//            "action" : 1,
//            "expressions" : [
//              {
//                "logic": 1,
//                "dataType" : 2,
//                "dataLength" : 4,
//                "data" : "1001",
//                "connectRelation" : 1
//              }
//            ]
//          }
//        }
//      ]
//    }
//  ],
//  "totalTimeConsuming": 8,
//  "dependencies" : [
//    {
//      "type": 1,
//      "ecuReqId" : "0X700",
//      "ecuSwDid" : "0XF187",
//      "ecuDids" : [
//        {
//          "did": "0XF187",
//          "didValue" : "V1.0"
//        }
//      ]
//    },
//    {
//      "type": 2,
//      "ecuReqId" : "0X700",
//      "ecuSwDid" : "0XF187",
//      "ecuDids" : [
//        {
//          "did": "0XF187",
//          "didValue" : "V2.0"
//        }
//      ]
//    },
//    {
//      "type": 1,
//      "ecuReqId" : "0X701",
//      "ecuSwDid" : "0XF187",
//      "ecuDids" : [
//        {
//          "did": "0XF187",
//          "didValue" : "V1.0"
//        }
//      ]
//    },
//    {
//      "type": 2,
//      "ecuReqId" : "0X701",
//      "ecuSwDid" : "0XF187",
//      "ecuDids" : [
//        {
//          "did": "0XF187",
//          "didValue" : "V2.0"
//        }
//      ]
//    },
//    {
//      "type": 1,
//      "ecuReqId" : "0X702",
//      "ecuSwDid" : "0XF187",
//      "ecuDids" : [
//        {
//          "did": "0XF187",
//          "didValue" : "V1.0"
//        }
//      ]
//    },
//    {
//      "type": 2,
//      "ecuReqId" : "0X702",
//      "ecuSwDid" : "0XF187",
//      "ecuDids" : [
//        {
//          "did": "0XF187",
//          "didValue" : "V2.0"
//        }
//      ]
//    },
//    {
//      "type": 1,
//      "ecuReqId" : "0X7B4",
//      "ecuSwDid" : "0XF187",
//      "ecuDids" : [
//        {
//          "did": "0XF187",
//          "didValue" : "V1.0"
//        }
//      ]
//    },
//    {
//      "type": 2,
//      "ecuReqId" : "0X7B4",
//      "ecuSwDid" : "0XF187",
//      "ecuDids" : [
//        {
//          "did": "0XF187",
//          "didValue" : "V2.0"
//        }
//      ]
//    }
//  ],
//  "ecuGroupOrders": [
//    {
//      "groupFlag": 1,
//      "actionForFailed" : 1,
//      "groupEcus" : [
//        {
//          "ecuReqId": "0X700",
//          "ecuSwDid" : "0XF187",
//          "upgradeOrder" : "0x112133",
//          "ecuTimeConsuming" : 2,
//          "ecuTimeout" : 4
//        },
//        {
//          "ecuReqId": "0X701",
//          "ecuSwDid" : "0XF187",
//          "upgradeOrder" : "0x112138",
//          "ecuTimeConsuming" : 2,
//          "ecuTimeout" : 4
//        },
//        {
//          "ecuReqId": "0X702",
//          "ecuSwDid" : "0XF187",
//          "upgradeOrder" : "0x112231",
//          "ecuTimeConsuming" : 2,
//          "ecuTimeout" : 4
//        }
//      ]
//    },
//    {
//      "groupFlag": 2,
//      "actionForFailed" : 2,
//      "groupEcus" : [
//        {
//          "ecuReqId": "0X7B4",
//          "ecuSwDid" : "0XF187",
//          "upgradeOrder" : "0x112232",
//          "ecuTimeConsuming" : 6,
//          "ecuTimeout" : 12
//        },
//        {
//          "ecuReqId": "0X7B5",
//          "ecuSwDid" : "0XF187",
//          "upgradeOrder" : "0x112234",
//          "ecuTimeConsuming" : 6,
//          "ecuTimeout" : 12
//        },
//        {
//          "ecuReqId": "0X7B6",
//          "ecuSwDid" : "0XF187",
//          "upgradeOrder" : "0x112335",
//          "ecuTimeConsuming" : 6,
//          "ecuTimeout" : 12
//        }
//      ]
//    },
//    {
//      "groupFlag": 3,
//      "actionForFailed" : 2,
//      "groupEcus" : [
//        {
//          "ecuReqId": "0X7C4",
//          "ecuSwDid" : "0XF187",
//          "upgradeOrder" : "0x112339",
//          "ecuTimeConsuming" : 6,
//          "ecuTimeout" : 12
//        },
//        {
//          "ecuReqId": "0X7C5",
//          "ecuSwDid" : "0XF187",
//          "upgradeOrder" : "0x112436",
//          "ecuTimeConsuming" : 6,
//          "ecuTimeout" : 12
//        },
//        {
//          "ecuReqId": "0X7C6",
//          "ecuSwDid" : "0XF187",
//          "upgradeOrder" : "0x112437",
//          "ecuTimeConsuming" : 6,
//          "ecuTimeout" : 12
//        }
//      ]
//    }
//  ]
// })";
//    ADM_IPS_PROGRESS_T progress;
//    ADM_IPS_INSTALL_ECUINFO_ARRAY_T ecusInfo;
//    ADM_IPS_ECU_CURRENT_PROGRESS_T curEcuProgress;
//    ADM_IPS_ECU_PROGRESS_T ecuProgress;
//
//    adm_ips_init();
//    // adm_ips_setPolicyFetchLimit(5, 2);
//    adm_ips_stepInstallPrepare(versionPolicyGet(policyStr), 5, 2);
//
//    D32S maxEcusLen = adm_ips_maxEcusCountGet();
//    ASSERT_EQ(9, maxEcusLen);
//    ipsInstallEcuinfoArrayInit(&ecusInfo);
//    ASSERT_EQ(1, ipsInstallEcuinfoArrayCreate(&ecusInfo, maxEcusLen));
//
//    adm_ips_stepInstallStart();
//
//    adm_ips_progressInit(&progress);
//    adm_ips_progressGet(&progress);
//    EXPECT_EQ(ADM_IPS_PHASE_INSTALL, progress.state);
//    EXPECT_EQ(0, progress.progress);
//    EXPECT_EQ(9, progress.ecus.ecusLen);
//    adm_ips_progressDeInit(&progress);
//
//    ipsInstallEcuinfoArrayClear(&ecusInfo);
//    ASSERT_EQ(5, adm_ips_installEcuInfoArrayGet(ecusInfo.ecus, maxEcusLen));
//    ASSERT_NE(nullptr, ecusInfo.ecus[0].ecuReqId);
//    EXPECT_STREQ("0X702", ecusInfo.ecus[0].ecuReqId);
//    ASSERT_NE(nullptr, ecusInfo.ecus[0].ecuSwDid);
//    EXPECT_STREQ("0XF187", ecusInfo.ecus[0].ecuSwDid);
//    ASSERT_NE(nullptr, ecusInfo.ecus[1].ecuReqId);
//    EXPECT_STREQ("0X7B4", ecusInfo.ecus[1].ecuReqId);
//    ASSERT_NE(nullptr, ecusInfo.ecus[2].ecuReqId);
//    EXPECT_STREQ("0X700", ecusInfo.ecus[2].ecuReqId);
//    ASSERT_NE(nullptr, ecusInfo.ecus[3].ecuReqId);
//    EXPECT_STREQ("0X7B6", ecusInfo.ecus[3].ecuReqId);
//    ASSERT_NE(nullptr, ecusInfo.ecus[4].ecuReqId);
//    EXPECT_STREQ("0X7C5", ecusInfo.ecus[4].ecuReqId);
//
//    curEcuProgress.ecuReqId = ecusInfo.ecus[2].ecuReqId;
//    curEcuProgress.ecuSwDid = ecusInfo.ecus[2].ecuSwDid;
//    curEcuProgress.isEnd = 0;
//    curEcuProgress.progress = 60;
//    adm_ips_currentProgressSet(&curEcuProgress);
//    adm_ips_ecuProgressInit(&ecuProgress);
//    adm_ips_ecuProgressGet(&ecuProgress, curEcuProgress.ecuReqId, curEcuProgress.ecuSwDid);
//    EXPECT_EQ(ADM_IPS_ECU_PHASE_INSTALL, ecuProgress.state);
//    EXPECT_EQ(60, ecuProgress.progress[0].progress);
//    adm_ips_ecuProgressDeInit(&ecuProgress);
//
//    curEcuProgress.progress = 100;
//    adm_ips_currentProgressSet(&curEcuProgress);
//    adm_ips_ecuProgressInit(&ecuProgress);
//    adm_ips_ecuProgressGet(&ecuProgress, curEcuProgress.ecuReqId, curEcuProgress.ecuSwDid);
//    EXPECT_EQ(100, ecuProgress.progress[0].progress);
//    adm_ips_ecuProgressDeInit(&ecuProgress);
//
//    curEcuProgress.isEnd = 1;
//    curEcuProgress.result = 0;
//    adm_ips_currentProgressSet(&curEcuProgress);
//    adm_ips_ecuProgressInit(&ecuProgress);
//    adm_ips_ecuProgressGet(&ecuProgress, curEcuProgress.ecuReqId, curEcuProgress.ecuSwDid);
//    EXPECT_EQ(ADM_IPS_ECU_PHASE_POST_INSTALL, ecuProgress.state);
//    EXPECT_EQ(0, ecuProgress.progress[0].result);
//    adm_ips_ecuProgressDeInit(&ecuProgress);
//
//    ipsInstallEcuinfoArrayClear(&ecusInfo);
//    ASSERT_EQ(5, adm_ips_installEcuInfoArrayGet(ecusInfo.ecus, maxEcusLen));
//    ASSERT_NE(nullptr, ecusInfo.ecus[0].ecuReqId);
//    EXPECT_STREQ("0X702", ecusInfo.ecus[0].ecuReqId);
//    ASSERT_NE(nullptr, ecusInfo.ecus[0].ecuSwDid);
//    EXPECT_STREQ("0XF187", ecusInfo.ecus[0].ecuSwDid);
//    ASSERT_NE(nullptr, ecusInfo.ecus[1].ecuReqId);
//    EXPECT_STREQ("0X7B4", ecusInfo.ecus[1].ecuReqId);
//    ASSERT_NE(nullptr, ecusInfo.ecus[2].ecuReqId);
//    EXPECT_STREQ("0X7B6", ecusInfo.ecus[2].ecuReqId);
//    ASSERT_NE(nullptr, ecusInfo.ecus[3].ecuReqId);
//    EXPECT_STREQ("0X7C5", ecusInfo.ecus[3].ecuReqId);
//    ASSERT_NE(nullptr, ecusInfo.ecus[4].ecuReqId);
//    EXPECT_STREQ("0X7C6", ecusInfo.ecus[4].ecuReqId);
//
//    curEcuProgress.ecuReqId = ecusInfo.ecus[0].ecuReqId;
//    curEcuProgress.ecuSwDid = ecusInfo.ecus[0].ecuSwDid;
//    curEcuProgress.isEnd = 0;
//    curEcuProgress.progress = 100;
//    adm_ips_currentProgressSet(&curEcuProgress);
//    adm_ips_ecuProgressInit(&ecuProgress);
//    adm_ips_ecuProgressGet(&ecuProgress, curEcuProgress.ecuReqId, curEcuProgress.ecuSwDid);
//    EXPECT_EQ(ADM_IPS_ECU_PHASE_INSTALL, ecuProgress.state);
//    EXPECT_EQ(100, ecuProgress.progress[0].progress);
//    adm_ips_ecuProgressDeInit(&ecuProgress);
//
//    curEcuProgress.isEnd = 1;
//    curEcuProgress.result = 0;
//    adm_ips_currentProgressSet(&curEcuProgress);
//    adm_ips_ecuProgressInit(&ecuProgress);
//    adm_ips_ecuProgressGet(&ecuProgress, curEcuProgress.ecuReqId, curEcuProgress.ecuSwDid);
//    EXPECT_EQ(ADM_IPS_ECU_PHASE_POST_INSTALL, ecuProgress.state);
//    EXPECT_EQ(0, ecuProgress.progress[0].result);
//    adm_ips_ecuProgressDeInit(&ecuProgress);
//
//    ipsInstallEcuinfoArrayClear(&ecusInfo);
//    ASSERT_EQ(5, adm_ips_installEcuInfoArrayGet(ecusInfo.ecus, maxEcusLen));
//    ASSERT_NE(nullptr, ecusInfo.ecus[0].ecuReqId);
//    EXPECT_STREQ("0X7B4", ecusInfo.ecus[0].ecuReqId);
//    ASSERT_NE(nullptr, ecusInfo.ecus[0].ecuSwDid);
//    EXPECT_STREQ("0XF187", ecusInfo.ecus[0].ecuSwDid);
//    ASSERT_NE(nullptr, ecusInfo.ecus[1].ecuReqId);
//    EXPECT_STREQ("0X7B6", ecusInfo.ecus[1].ecuReqId);
//    ASSERT_NE(nullptr, ecusInfo.ecus[2].ecuReqId);
//    EXPECT_STREQ("0X7C5", ecusInfo.ecus[2].ecuReqId);
//    ASSERT_NE(nullptr, ecusInfo.ecus[3].ecuReqId);
//    EXPECT_STREQ("0X7C6", ecusInfo.ecus[3].ecuReqId);
//    ASSERT_NE(nullptr, ecusInfo.ecus[4].ecuReqId);
//    EXPECT_STREQ("0X7B5", ecusInfo.ecus[4].ecuReqId);
//
//
//    curEcuProgress.ecuReqId = ecusInfo.ecus[0].ecuReqId;
//    curEcuProgress.ecuSwDid = ecusInfo.ecus[0].ecuSwDid;
//    curEcuProgress.isEnd = 0;
//    curEcuProgress.progress = 100;
//    adm_ips_currentProgressSet(&curEcuProgress);
//
//    curEcuProgress.isEnd = 1;
//    curEcuProgress.result = 0;
//    adm_ips_currentProgressSet(&curEcuProgress);
//
//    curEcuProgress.ecuReqId = ecusInfo.ecus[1].ecuReqId;
//    curEcuProgress.ecuSwDid = ecusInfo.ecus[1].ecuSwDid;
//    curEcuProgress.isEnd = 0;
//    curEcuProgress.progress = 100;
//    adm_ips_currentProgressSet(&curEcuProgress);
//
//    curEcuProgress.isEnd = 1;
//    curEcuProgress.result = 0;
//    adm_ips_currentProgressSet(&curEcuProgress);
//
//    ipsInstallEcuinfoArrayClear(&ecusInfo);
//    ASSERT_EQ(5, adm_ips_installEcuInfoArrayGet(ecusInfo.ecus, maxEcusLen));
//    ASSERT_NE(nullptr, ecusInfo.ecus[0].ecuReqId);
//    EXPECT_STREQ("0X7B5", ecusInfo.ecus[0].ecuReqId);
//    ASSERT_NE(nullptr, ecusInfo.ecus[0].ecuSwDid);
//    EXPECT_STREQ("0XF187", ecusInfo.ecus[0].ecuSwDid);
//    ASSERT_NE(nullptr, ecusInfo.ecus[1].ecuReqId);
//    EXPECT_STREQ("0X7C5", ecusInfo.ecus[1].ecuReqId);
//    ASSERT_NE(nullptr, ecusInfo.ecus[2].ecuReqId);
//    EXPECT_STREQ("0X7C6", ecusInfo.ecus[2].ecuReqId);
//    ASSERT_NE(nullptr, ecusInfo.ecus[3].ecuReqId);
//    EXPECT_STREQ("0X701", ecusInfo.ecus[3].ecuReqId);
//    ASSERT_NE(nullptr, ecusInfo.ecus[4].ecuReqId);
//    EXPECT_STREQ("0X7C4", ecusInfo.ecus[4].ecuReqId);
//
//
//    curEcuProgress.ecuReqId = ecusInfo.ecus[1].ecuReqId;
//    curEcuProgress.ecuSwDid = ecusInfo.ecus[1].ecuSwDid;
//    curEcuProgress.isEnd = 0;
//    curEcuProgress.progress = 100;
//    adm_ips_currentProgressSet(&curEcuProgress);
//
//    curEcuProgress.isEnd = 1;
//    curEcuProgress.result = 0;
//    adm_ips_currentProgressSet(&curEcuProgress);
//
//    curEcuProgress.ecuReqId = ecusInfo.ecus[2].ecuReqId;
//    curEcuProgress.ecuSwDid = ecusInfo.ecus[2].ecuSwDid;
//    curEcuProgress.isEnd = 0;
//    curEcuProgress.progress = 100;
//    adm_ips_currentProgressSet(&curEcuProgress);
//
//    curEcuProgress.isEnd = 1;
//    curEcuProgress.result = 0;
//    adm_ips_currentProgressSet(&curEcuProgress);
//
//
//    ipsInstallEcuinfoArrayClear(&ecusInfo);
//    ASSERT_EQ(3, adm_ips_installEcuInfoArrayGet(ecusInfo.ecus, maxEcusLen));
//    ASSERT_NE(nullptr, ecusInfo.ecus[0].ecuReqId);
//    EXPECT_STREQ("0X7B5", ecusInfo.ecus[0].ecuReqId);
//    ASSERT_NE(nullptr, ecusInfo.ecus[0].ecuSwDid);
//    EXPECT_STREQ("0XF187", ecusInfo.ecus[0].ecuSwDid);
//    ASSERT_NE(nullptr, ecusInfo.ecus[1].ecuReqId);
//    EXPECT_STREQ("0X701", ecusInfo.ecus[1].ecuReqId);
//    ASSERT_NE(nullptr, ecusInfo.ecus[2].ecuReqId);
//    EXPECT_STREQ("0X7C4", ecusInfo.ecus[2].ecuReqId);
//
//    curEcuProgress.ecuReqId = ecusInfo.ecus[0].ecuReqId;
//    curEcuProgress.ecuSwDid = ecusInfo.ecus[0].ecuSwDid;
//    curEcuProgress.isEnd = 0;
//    curEcuProgress.progress = 100;
//    adm_ips_currentProgressSet(&curEcuProgress);
//
//    curEcuProgress.isEnd = 1;
//    curEcuProgress.result = 0;
//    adm_ips_currentProgressSet(&curEcuProgress);
//
//    curEcuProgress.ecuReqId = ecusInfo.ecus[1].ecuReqId;
//    curEcuProgress.ecuSwDid = ecusInfo.ecus[1].ecuSwDid;
//    curEcuProgress.isEnd = 0;
//    curEcuProgress.progress = 100;
//    adm_ips_currentProgressSet(&curEcuProgress);
//
//    curEcuProgress.isEnd = 1;
//    curEcuProgress.result = 0;
//    adm_ips_currentProgressSet(&curEcuProgress);
//
//    curEcuProgress.ecuReqId = ecusInfo.ecus[2].ecuReqId;
//    curEcuProgress.ecuSwDid = ecusInfo.ecus[2].ecuSwDid;
//    curEcuProgress.isEnd = 0;
//    curEcuProgress.progress = 100;
//    adm_ips_currentProgressSet(&curEcuProgress);
//
//    curEcuProgress.isEnd = 1;
//    curEcuProgress.result = 0;
//    adm_ips_currentProgressSet(&curEcuProgress);
//
//
//    ipsInstallEcuinfoArrayClear(&ecusInfo);
//    ASSERT_EQ(0, adm_ips_installEcuInfoArrayGet(ecusInfo.ecus, maxEcusLen));
//
//    adm_ips_stepPostInstall(1000);
//
//    adm_ips_progressInit(&progress);
//    adm_ips_progressGet(&progress);
//    EXPECT_EQ(ADM_IPS_PHASE_POST_INSTALL, progress.state);
//    EXPECT_EQ(100, progress.progress);
//    EXPECT_EQ(1000, progress.result);
//    adm_ips_progressDeInit(&progress);
//
//    ipsInstallEcuinfoArrayDeInit(&ecusInfo);
//
//    ADM_COND_POLICY_T cond;
//    adm_condPolicyInit(&cond);
//    adm_ips_policyConditionGet(1, &cond);
//    ASSERT_EQ(1, cond.iCondNum);
//    adm_condPolicyDeInit(&cond);
//}

TEST_F(IpsTest, Normal00) {
    test_ips_testCase("data/ips/upgrade_ecus_success.json");
}

TEST_F(IpsTest, Normal01) {
    test_ips_testCase("data/ips/upgrade_ecus_success01.json");
}


TEST_F(IpsTest, Fail_00) {
    test_ips_testCase("data/ips/upgrade_ecus_fail_00.json");
}

TEST_F(IpsTest, Fail_01) {
    test_ips_testCase("data/ips/upgrade_ecus_fail_01.json");
}

#endif // UPGRADE_GROUP_POLICY_VERSION

#endif//ADM_IPS_POLICY_UPGRADE_ORDER_OLD
//
//const DCHAR *m_installPolicyObjectStr = 
//#ifdef ADM_IPS_POLICY_UPGRADE_ORDER_OLD
//R"(
//{
//  "upgradeAutoStart": "xxx",
//  "upgradeRetryCount": 1,
//  "rollbackRetryCount": 1,
//  "upgradeConditions": [
//    {
//      "type": 1,
//      "conditions": [
//        {
//          "messageId": "xxx",
//          "messageName": "xxx",
//          "messageDesc": "xxx",
//          "expressions": [
//            {
//              "logic": 1,
//              "dataType": 1,
//              "dataLength": 3,
//              "data": "101",
//              "connectRelation": 1
//            }
//          ],
//          "connectRelation": 1,
//          "onFailStrategy": {
//            "type": 1,
//            "timeout": 30,
//            "action": 1,
//            "expressions": [
//              {
//                "logic": 1,
//                "dataType": 2,
//                "dataLength": 4,
//                "data": "1001",
//                "connectRelation": 1
//              }
//            ]
//          }
//        }
//      ]
//    }
//  ],
//  "totalTimeConsuming": 8,
//  "dependencies": [
//    {
//      "type": 1,
//      "ecuReqId": "0X700",
//      "ecuSwDid": "0XF187",
//      "ecuDids": [
//        {
//          "did": "0XF187",
//          "didValue": "V1.0"
//        }
//      ]
//    },
//    {
//      "type": 2,
//      "ecuReqId": "0X700",
//      "ecuSwDid": "0XF187",
//      "ecuDids": [
//        {
//          "did": "0XF187",
//          "didValue": "V2.0"
//        }
//      ]
//    },
//    {
//      "type": 1,
//      "ecuReqId": "0X7B4",
//      "ecuSwDid": "0XF187",
//      "ecuDids": [
//        {
//          "did": "0XF187",
//          "didValue": "V1.0"
//        }
//      ]
//    },
//    {
//      "type": 2,
//      "ecuReqId": "0X7B4",
//      "ecuSwDid": "0XF187",
//      "ecuDids": [
//        {
//          "did": "0XF187",
//          "didValue": "V2.0"
//        }
//      ]
//    }
//  ],
//  "ecuGroupOrders": [
//    {
//      "groupFlag": 1,
//      "actionForFailed": 1,
//      "groupEcus": [
//        {
//          "ecuReqId": "0X700",
//          "ecuSwDid": "0XF187",
//          "upgradeOrder": 1,
//          "ecuTimeConsuming": 2,
//          "ecuTimeout": 4
//        }
//      ]
//    },
//    {
//      "groupFlag": 2,
//      "actionForFailed": 2,
//      "groupEcus": [
//        {
//          "ecuReqId": "0X7B4",
//          "ecuSwDid": "0XF187",
//          "upgradeOrder": 2,
//          "ecuTimeConsuming": 6,
//          "ecuTimeout": 12
//        }
//      ]
//    }
//  ]
//}
//)"
//#else
//R"(
//{
//  "upgradeAutoStart": "xxx",
//  "upgradeRetryCount": 1,
//  "rollbackRetryCount": 1,
//  "upgradeConditions": [
//    {
//      "type": 1,
//      "conditions": [
//        {
//          "messageId": "xxx",
//          "messageName": "xxx",
//          "messageDesc": "xxx",
//          "expressions": [
//            {
//              "logic": 1,
//              "dataType": 1,
//              "dataLength": 3,
//              "data": "101",
//              "connectRelation": 1
//            }
//          ],
//          "connectRelation": 1,
//          "onFailStrategy": {
//            "type": 1,
//            "timeout": 30,
//            "action": 1,
//            "expressions": [
//              {
//                "logic": 1,
//                "dataType": 2,
//                "dataLength": 4,
//                "data": "1001",
//                "connectRelation": 1
//              }
//            ]
//          }
//        }
//      ]
//    }
//  ],
//  "totalTimeConsuming": 8,
//  "dependencies": [
//    {
//      "type": 1,
//      "ecuReqId": "0X700",
//      "ecuSwDid": "0XF187",
//      "ecuDids": [
//        {
//          "did": "0XF187",
//          "didValue": "V1.0"
//        }
//      ]
//    },
//    {
//      "type": 2,
//      "ecuReqId": "0X700",
//      "ecuSwDid": "0XF187",
//      "ecuDids": [
//        {
//          "did": "0XF187",
//          "didValue": "V2.0"
//        }
//      ]
//    },
//    {
//      "type": 1,
//      "ecuReqId": "0X701",
//      "ecuSwDid": "0XF187",
//      "ecuDids": [
//        {
//          "did": "0XF187",
//          "didValue": "V1.0"
//        }
//      ]
//    },
//    {
//      "type": 2,
//      "ecuReqId": "0X701",
//      "ecuSwDid": "0XF187",
//      "ecuDids": [
//        {
//          "did": "0XF187",
//          "didValue": "V2.0"
//        }
//      ]
//    },
//    {
//      "type": 1,
//      "ecuReqId": "0X702",
//      "ecuSwDid": "0XF187",
//      "ecuDids": [
//        {
//          "did": "0XF187",
//          "didValue": "V1.0"
//        }
//      ]
//    },
//    {
//      "type": 2,
//      "ecuReqId": "0X702",
//      "ecuSwDid": "0XF187",
//      "ecuDids": [
//        {
//          "did": "0XF187",
//          "didValue": "V2.0"
//        }
//      ]
//    },
//    {
//      "type": 1,
//      "ecuReqId": "0X7B4",
//      "ecuSwDid": "0XF187",
//      "ecuDids": [
//        {
//          "did": "0XF187",
//          "didValue": "V1.0"
//        }
//      ]
//    },
//    {
//      "type": 2,
//      "ecuReqId": "0X7B4",
//      "ecuSwDid": "0XF187",
//      "ecuDids": [
//        {
//          "did": "0XF187",
//          "didValue": "V2.0"
//        }
//      ]
//    }
//  ],
//  "ecuGroupOrders": [
//    {
//      "groupFlag": 1,
//      "actionForFailed": 1,
//      "groupEcus": [
//        {
//          "ecuReqId": "0X700",
//          "ecuSwDid": "0XF187",
//          "upgradeOrder": "0x112233",
//          "ecuTimeConsuming": 2,
//          "ecuTimeout": 4
//        },
//        {
//          "ecuReqId": "0X701",
//          "ecuSwDid": "0XF187",
//          "upgradeOrder": "0x112234",
//          "ecuTimeConsuming": 2,
//          "ecuTimeout": 4
//        },
//        {
//          "ecuReqId": "0X702",
//          "ecuSwDid": "0XF187",
//          "upgradeOrder": "0x122233",
//          "ecuTimeConsuming": 2,
//          "ecuTimeout": 4
//        }
//      ]
//    },
//    {
//      "groupFlag": 2,
//      "actionForFailed": 2,
//      "groupEcus": [
//        {
//          "ecuReqId": "0X7B4",
//          "ecuSwDid": "0XF187",
//          "upgradeOrder": "0x112233",
//          "ecuTimeConsuming": 6,
//          "ecuTimeout": 12
//        },
//        {
//          "ecuReqId": "0X7B5",
//          "ecuSwDid": "0XF187",
//          "upgradeOrder": "0x112501",
//          "ecuTimeConsuming": 6,
//          "ecuTimeout": 12
//        }
//      ]
//    }
//  ]
//}
//)"
//#endif//ADM_IPS_POLICY_UPGRADE_ORDER_OLD
;

const ADM_CVS_VERSION_POLICY_T* versionPolicyGet(const DCHAR* policyStr) {
    static ADM_CVS_VERSION_POLICY_T policy;
    policy.sessionId = (DCHAR*)"123";
    policy.taskId = 234;
    policy.strategyId = 345;
    policy.downloadMember.downloadPolicyObjectStr = NULL;
    policy.installMember.installPolicyObjectStr = (DCHAR*)policyStr;
    return &policy;
}

}//namespace
