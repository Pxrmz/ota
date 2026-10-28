
#include "adm_eim_log.h"
#include "adm_eim_getEcuInfo.h"
#include "adm_dim_vdi.h"
#include "adm_com_errorCode.h"
#include "adm_ecs_rsConfig.h"
#include "adm_dim_ucl.h"
#include "adm_dim_host.h"
#include "adm_soa_cfg.h"
#include "json.h"
#include "adm_soa.h"
#include "adm_dim_host.h"
#include "adm_eim_event.h"
#include "adm_cfg_api.h"
#include "adm_sdk_selfUpgrade.h"

#include "adm_memory.h"
#include "adm_string.h"

#include <time.h>
#include <unistd.h>

#define READECUINFO_TIMEOUT 60 /*读零件信息超时时间*/
#define GETVERSION_TIMEOUT 30 /*读零件信息超时时间*/
#define EVENT_DES_ARRAY_LEN 256
#define SUM_GETECUINFO_TIMEOUT 60U /*单个ECU读零件信息总超时时间*/
#define GETECUINFO_SLEEPTIME (20*1000) /*ECU读零件信息休眠时间*/



static void eim_addSingleEcuEventList(ADM_OCS_EVENTS_INFO_LIST_T* eventsList,
    const DCHAR* eventId,
    const DCHAR* ecuReqId,
    const DCHAR* ecuSwDid,
    const DCHAR* eventDesc)
{
    DCHAR eventDescArray[EVENT_DES_ARRAY_LEN];
    (void)adm_memset(eventDescArray, 0x00, EVENT_DES_ARRAY_LEN, EVENT_DES_ARRAY_LEN);
    (void)adm_snprintf(eventDescArray,
        EVENT_DES_ARRAY_LEN,
        "[%s + %s][%s](%s)",
        ecuReqId,
        ecuSwDid,
        adm_eim_getDateAndTimeStr(),
        eventDesc);
    (void)adm_eim_addEventList(eventsList, eventId, (D32S)ADM_OCS_EVENT_LEVEL_INFO, NULL, eventDescArray);
}

static void eim_addAllEcuEventList(ADM_OCS_EVENTS_INFO_LIST_T* eventsList, const DCHAR* eventId, const DCHAR* eventDesc)
{
    DCHAR eventDescArray[EVENT_DES_ARRAY_LEN];
    (void)adm_memset(eventDescArray, 0x00, EVENT_DES_ARRAY_LEN, EVENT_DES_ARRAY_LEN);
    (void)adm_snprintf(eventDescArray, EVENT_DES_ARRAY_LEN, "[%s](%s)", adm_eim_getDateAndTimeStr(), eventDesc);
    (void)adm_eim_addEventList(eventsList, eventId, (D32S)ADM_OCS_EVENT_LEVEL_INFO, NULL, eventDescArray);
}

D32S adm_eim_getSingleEcuInfoForEcu(const ADM_ECS_ECUCONFIG_T* pstEcuCfg,
    const ADM_ECS_ECUDIDLIST_T* pstEcuDidList,
    const ADM_DIM_SOA_ECU_IDEN_INFO_T* pstEcuIdenInfo,
    const DCHAR* diagScriptPath, ECU_INFO_T** ppEcuInfo)
{
    ADM_DIM_SOA_DID_DATA_ARRAY_T didListRequest = {0};
    D32S readRetVal = ADM_ECODE_FAILED;
    D32S errorCod = ADM_ECODE_FAILED;
    ECU_INFO_T* ecuInfo = NULL;
    D32S instanceId = 0xFFFF;
    D64U mallocLen = 0U;
    ADM_DIM_SOA_DATA_T didData = {0};
    ADM_OCS_EVENTS_INFO_LIST_T* eventsList = NULL;
    D32S eventNum = 0;
    D64S startTime = 0;
    D64S endTime = 0;
    DCHAR eventDesc[EVENT_DES_ARRAY_LEN];
    DBOOL isCGWRouted = FALSE;
    (void)adm_memset(eventDesc, 0x00, EVENT_DES_ARRAY_LEN, EVENT_DES_ARRAY_LEN);

    if (NULL == pstEcuCfg || NULL == pstEcuDidList || NULL == pstEcuIdenInfo) {
        log_e("pstEcuCfg %p, pstEcuDidList %p, pstEcuIdenInfo %p", pstEcuCfg, pstEcuDidList, pstEcuIdenInfo);
        return ADM_ECODE_FAILED;
    }

    /* 为零件配置+通用配置大小分配内存 */
    mallocLen = sizeof(ADM_DIM_SOA_DID_DATA_T) * ((D64U)(pstEcuCfg->stEcuDidList.num) + (D64U)(pstEcuDidList->num)) + 1U;
    didListRequest.didArray = adm_malloc(mallocLen);
    if (NULL == didListRequest.didArray) {
        log_e("adm_malloc error");
        return ADM_ECODE_FAILED;
    }
    (void)adm_memzero(didListRequest.didArray, mallocLen);

    log_i("PartConfig ecu didi num = %d", pstEcuCfg->stEcuDidList.num);
    log_i("GeneralConfig ecu did num = %d", pstEcuDidList->num);

    /* didListRequest ECU零件信息 = 零件配置信息 + 通用配置信息 */
    /* 零件配置信息 */
    for (D32S i = 0; i < pstEcuCfg->stEcuDidList.num; ++i) {
        if (NULL == (pstEcuCfg->stEcuDidList.pstEcuDidInfo + i)) {
            log_w("pstEcuCfg->stEcuDidList.pstEcuDidInfo[%d] is NULL", i);
            continue;
        }

        mallocLen = sizeof(ADM_DIM_SOA_DID_DATA_T) + 1U;
        didListRequest.didArray[didListRequest.didArrayLength] = adm_malloc(mallocLen);
        if (NULL == didListRequest.didArray[didListRequest.didArrayLength]) {
            log_e("adm_malloc error");
            goto free_didListRequest;
        }
        (void)adm_memzero(didListRequest.didArray[didListRequest.didArrayLength], mallocLen);

        didListRequest.didArray[didListRequest.didArrayLength]->did
            = (D32S)htoi(pstEcuCfg->stEcuDidList.pstEcuDidInfo[i].did);
        didListRequest.didArray[didListRequest.didArrayLength]->endByte = (D16U)pstEcuCfg->stEcuDidList.pstEcuDidInfo[i].end;
        didListRequest.didArray[didListRequest.didArrayLength]->startByte
            = (D16U)pstEcuCfg->stEcuDidList.pstEcuDidInfo[i].start;

        didData.dataType = (D16U)pstEcuCfg->stEcuDidList.pstEcuDidInfo[i].didValType;
        didListRequest.didArray[didListRequest.didArrayLength]->didData = &didData;
        didListRequest.didArrayLength++;
    }

    /* 通用配置信息 */
    for (D32S i = 0; i < pstEcuDidList->num; ++i) {

        if (NULL == (pstEcuDidList->pstEcuDidInfo + i)) {
            log_w("pstEcuDidList->pstEcuDidInfo[%d] is NULL", i);
            continue;
        }

        mallocLen = sizeof(ADM_DIM_SOA_DID_DATA_T) + 1U;
        didListRequest.didArray[didListRequest.didArrayLength] = (ADM_DIM_SOA_DID_DATA_T*)adm_malloc(mallocLen);
        if (NULL == didListRequest.didArray[didListRequest.didArrayLength]) {
            log_e("adm_malloc error");
            goto free_didListRequest;
        }
        (void)adm_memzero(didListRequest.didArray[didListRequest.didArrayLength], mallocLen);

        didListRequest.didArray[didListRequest.didArrayLength]->did = (D32S)htoi(pstEcuDidList->pstEcuDidInfo[i].did);
        didListRequest.didArray[didListRequest.didArrayLength]->endByte = (D16U)pstEcuDidList->pstEcuDidInfo[i].end;
        didListRequest.didArray[didListRequest.didArrayLength]->startByte = (D16U)pstEcuDidList->pstEcuDidInfo[i].start;

        didData.dataType = (D16U)pstEcuDidList->pstEcuDidInfo[i].didValType;
        didListRequest.didArray[didListRequest.didArrayLength]->didData = &didData;

        didListRequest.didArrayLength++;
    }

    mallocLen = sizeof(ECU_INFO_T) + 1U;
    ecuInfo = adm_malloc(mallocLen);
    if (NULL == ecuInfo) {
        log_e("adm_malloc error");
        goto free_didListRequest;
    }
    (void)adm_memzero(ecuInfo, mallocLen);

    eventNum = 5;
    eventsList = adm_eim_initEventList(eventNum);

    instanceId = htoi(pstEcuCfg->readInfoInstanceId);
    if (2 == pstEcuCfg->serviceType) {  //!< UCM
        // TODO: UCM 和 VDI ECU INFO 有结构体兼容问题，结构体内容一致，名不同，暂时强转
        startTime = time(NULL);
        (void)adm_memset(eventDesc, 0x00, EVENT_DES_ARRAY_LEN, EVENT_DES_ARRAY_LEN);
        (void)adm_snprintf(eventDesc, EVENT_DES_ARRAY_LEN, "start get ecuInfo from UCL");
        eim_addSingleEcuEventList(
            eventsList, (const DCHAR*)"PS_GET_ECUINFO_001", pstEcuCfg->ecuReqId, pstEcuCfg->ecuSwDid, eventDesc);
        log_d("%s", eventDesc);

        readRetVal = adm_dim_ucl_readPartInformation((const D16U)instanceId,
            READECUINFO_TIMEOUT,
            (const ADM_DIM_SOA_ECU_IDEN_INFO_T*)pstEcuIdenInfo,
            (const ADM_DIM_SOA_DID_DATA_ARRAY_T*)&didListRequest,
            (ADM_DIM_SOA_DID_DATA_ARRAY_T**)&ecuInfo->vdiDidListResponse,
            &errorCod);

        endTime = time(NULL);

        if (ADM_ECODE_SUCCESSED == readRetVal && ADM_ECODE_SUCCESSED == errorCod) {
            (void)adm_memset(eventDesc, 0x00, EVENT_DES_ARRAY_LEN, EVENT_DES_ARRAY_LEN);
            (void)adm_snprintf(eventDesc,
                EVENT_DES_ARRAY_LEN,
                "end get ecuInfo from UCL:successful[%d]",
                endTime > startTime ? (endTime - startTime) : 0);
            eim_addSingleEcuEventList(
                eventsList, (const DCHAR*)"PS_GET_ECUINFO_002", pstEcuCfg->ecuReqId, pstEcuCfg->ecuSwDid, eventDesc);
            log_d("%s", eventDesc);
        } else {
            (void)adm_memset(eventDesc, 0x00, EVENT_DES_ARRAY_LEN, EVENT_DES_ARRAY_LEN);
            (void)adm_snprintf(eventDesc, EVENT_DES_ARRAY_LEN, "end get ecuInfo from UCL:failed");
            eim_addSingleEcuEventList(
                eventsList, (const DCHAR*)"PS_GET_ECUINFO_002", pstEcuCfg->ecuReqId, pstEcuCfg->ecuSwDid, eventDesc);
            log_d("%s", eventDesc);
        }
    } else if (1 == pstEcuCfg->serviceType) {  //!< VDI

        startTime = time(NULL);
        (void)adm_memset(eventDesc, 0x00, EVENT_DES_ARRAY_LEN, EVENT_DES_ARRAY_LEN);
        (void)adm_snprintf(eventDesc, EVENT_DES_ARRAY_LEN, "start get ecuInfo from VDI");
        eim_addSingleEcuEventList(
            eventsList, (const DCHAR*)"PS_GET_ECUINFO_001", pstEcuCfg->ecuReqId, pstEcuCfg->ecuSwDid, eventDesc);
        log_d("%s", eventDesc);

        if(1 == pstEcuCfg->isCGWRouted)
        {
            isCGWRouted = TRUE;
        }
        
        readRetVal = adm_dim_vdi_readDiagnosticInfo((const D16U)instanceId,
            READECUINFO_TIMEOUT,
            (const ADM_DIM_SOA_ECU_IDEN_INFO_T*)((const void*)pstEcuIdenInfo),
            diagScriptPath,
            (const ADM_DIM_SOA_VDI_BUS_TYPE_E)pstEcuCfg->busType,
            (const ADM_DIM_SOA_PIN_CHANNEL_TYPE_E)pstEcuCfg->channelType,
            (const DBOOL)isCGWRouted,
            didListRequest,
            &(ecuInfo->vdiDidListResponse),
            &errorCod);
        endTime = time(NULL);

        if (ADM_ECODE_SUCCESSED == readRetVal && ADM_ECODE_SUCCESSED == errorCod) {
            (void)adm_memset(eventDesc, 0x00, EVENT_DES_ARRAY_LEN, EVENT_DES_ARRAY_LEN);
            (void)adm_snprintf(eventDesc,
                EVENT_DES_ARRAY_LEN,
                "end get ecuInfo from VDI:successful[%d]",
                endTime > startTime ? (endTime - startTime) : 0);
            eim_addSingleEcuEventList(
                eventsList, (const DCHAR*)"PS_GET_ECUINFO_002", pstEcuCfg->ecuReqId, pstEcuCfg->ecuSwDid, eventDesc);
            log_d("%s", eventDesc);
        } else {
            (void)adm_memset(eventDesc, 0x00, EVENT_DES_ARRAY_LEN, EVENT_DES_ARRAY_LEN);
            (void)adm_snprintf(eventDesc, EVENT_DES_ARRAY_LEN, "end get ecuInfo from VDI:failed");
            eim_addSingleEcuEventList(
                eventsList, (const DCHAR*)"PS_GET_ECUINFO_002", pstEcuCfg->ecuReqId, pstEcuCfg->ecuSwDid, eventDesc);
            log_d("%s", eventDesc);
        }
    }
    else if (3 == pstEcuCfg->serviceType)   ///!< APK 自升级版本版本获取
    {
        startTime = time(NULL);
        (void)adm_memset(eventDesc, 0x00, EVENT_DES_ARRAY_LEN, EVENT_DES_ARRAY_LEN);
        (void)adm_snprintf(eventDesc, EVENT_DES_ARRAY_LEN, "start get ecuInfo from APK ");
        eim_addSingleEcuEventList(
            eventsList, (const DCHAR*)"PS_GET_ECUINFO_001", pstEcuCfg->ecuReqId, pstEcuCfg->ecuSwDid, eventDesc);
        log_d("%s", eventDesc);

        readRetVal = adm_eim_getApkVersion(instanceId,
                pstEcuCfg,
                (const ADM_DIM_SOA_ECU_IDEN_INFO_T*)pstEcuIdenInfo,
                (const ADM_DIM_SOA_DID_DATA_ARRAY_T*)&didListRequest,
                (ADM_DIM_SOA_DID_DATA_ARRAY_T**)&ecuInfo->vdiDidListResponse,
                &errorCod);

        endTime = time(NULL);

        if (ADM_ECODE_SUCCESSED == readRetVal && ADM_ECODE_SUCCESSED == errorCod) {
            (void)adm_memset(eventDesc, 0x00, EVENT_DES_ARRAY_LEN, EVENT_DES_ARRAY_LEN);
            (void)adm_snprintf(eventDesc,
                EVENT_DES_ARRAY_LEN,
                "end get ecuInfo from APK:successful[%d]",
                endTime > startTime ? (endTime - startTime) : 0);
            eim_addSingleEcuEventList(
                eventsList, (const DCHAR*)"PS_GET_ECUINFO_002", pstEcuCfg->ecuReqId, pstEcuCfg->ecuSwDid, eventDesc);
            log_d("%s", eventDesc);
        } else {
            (void)adm_memset(eventDesc, 0x00, EVENT_DES_ARRAY_LEN, EVENT_DES_ARRAY_LEN);
            (void)adm_snprintf(eventDesc, EVENT_DES_ARRAY_LEN, "end get ecuInfo from APK:failed");
            eim_addSingleEcuEventList(
                eventsList, (const DCHAR*)"PS_GET_ECUINFO_002", pstEcuCfg->ecuReqId, pstEcuCfg->ecuSwDid, eventDesc);
            log_d("%s", eventDesc);
        }
    } 
    else {
        readRetVal = ADM_ECODE_FAILED;
        // log_e("adm_soa_getUclInstanceId() 0x%X", adm_soa_getUclInstanceId());
        log_e("not found InfoInstanceId 0x%X", instanceId);
        log_e("serviceType:%d is invalid", pstEcuCfg->serviceType);
    }
    log_i("ecuReqId = %s, ecuSwDid = %s", pstEcuCfg->ecuReqId, pstEcuCfg->ecuSwDid);
    if (ADM_ECODE_SUCCESSED == readRetVal && ADM_ECODE_SUCCESSED == errorCod) {
        ecuInfo->ecuReqId = adm_strdup(pstEcuCfg->ecuReqId);
        ecuInfo->ecuSwDid = adm_strdup(pstEcuCfg->ecuSwDid);
        ecuInfo->partNumber = adm_strdup(pstEcuCfg->stExternEcuInfo.partNumber);
        ecuInfo->partSupplier = adm_strdup(pstEcuCfg->stExternEcuInfo.partSupplier);
        ecuInfo->softwarePartNumber = adm_strdup(pstEcuCfg->stExternEcuInfo.softwarePartNumber);
        ecuInfo->partHardwareNum = adm_strdup(pstEcuCfg->stExternEcuInfo.partHardwareNum);
        *ppEcuInfo = ecuInfo;
        log_i("get single ecu info successful");
    } else {
		
//该错误处理方式会导致：1. 当readRetVal为错误且errorCod为正确时函数返回正确的结果；2. 当readRetVal为正确且errorCod为错误时函数返回正确的结果		
/*		
        if (ADM_ECODE_SUCCESSED != readRetVal)
        {
            readRetVal = errorCod;
        }
*/
        log_e("Failed to get single ecu info, result[%d] error[%d]", readRetVal, errorCod);
		readRetVal = (readRetVal == ADM_ECODE_SUCCESSED) ? errorCod : readRetVal;
        adm_free(ecuInfo);
        ecuInfo = NULL;
    }
    // adm_eim_getEcuInfoReport(eventsList);
    adm_eim_deInitEventsList(eventsList);
free_didListRequest:
    /* 释放每次循环申请的内存 */
    for (D16U i = 0; i < didListRequest.didArrayLength; ++i) {
        if (NULL != didListRequest.didArray[i]) {
            adm_free(didListRequest.didArray[i]);
        }
    }
    adm_free(didListRequest.didArray);

    return readRetVal;
}

D32S adm_eim_getEcuInfoForEcu(const ADM_ECS_RSCONFIGINFO_T* stRsCfgInfo, ADM_EIM_GETECUINFO_T* getEcuInfo)
{
    if (NULL == stRsCfgInfo || NULL == getEcuInfo) {
        log_e("ppstOtaConfigData , getEcuInfo is NULL");
        return ADM_ECODE_INVALID_PARAM;
    }

    const ADM_ECS_PARTCONFIG_T* stPartCfg = NULL;
    const ADM_ECS_GENERALCONFIG_T* stGeneralCfg = NULL;

    ADM_DIM_SOA_ECU_IDEN_INFO_T* pstEcuIdenInfo = NULL;
    DCHAR* diagScriptPath = NULL;
    ECU_INFO_T* ecuInfo = NULL;
    D32S retVal = ADM_ECODE_FAILED;
    D32S ecuNum = 0;
    ADM_OCS_EVENTS_INFO_LIST_T* eventsList = NULL;
    D32S eventNum = 0;
    D64S startTime = 0;
    D64S endTime = 0;
    DCHAR eventDesc[EVENT_DES_ARRAY_LEN];

    D64U uiStartTime = 0U;  // ECU信息获取开始时间
    D64U uiEndTime = 0U;    // ECU信息获取结束时间
    D64U uiCostTime = 0U;   // ECU信息获取已花费时间
    D64U uiLastTryTime = 0U;// 上次获取时间
    DBOOL bEnd = FALSE;     // ECU信息获取结束标志
	
    (void)adm_memset(eventDesc, 0x00, EVENT_DES_ARRAY_LEN, EVENT_DES_ARRAY_LEN);

    /*initialize variables*/
    {
        stPartCfg = &(stRsCfgInfo->stPartCfg);
        stGeneralCfg = &(stRsCfgInfo->stGeneralCfg);
        diagScriptPath = adm_strdup(stGeneralCfg->stBusinessScript.filePath);

        pstEcuIdenInfo = (ADM_DIM_SOA_ECU_IDEN_INFO_T*)adm_malloc(sizeof(ADM_DIM_SOA_ECU_IDEN_INFO_T));
        (void)adm_memset(pstEcuIdenInfo, 0x00, sizeof(ADM_DIM_SOA_ECU_IDEN_INFO_T), sizeof(ADM_DIM_SOA_ECU_IDEN_INFO_T));
    }

    do {
        /*获取配置ECU的个数，如果是0，则无零件信息*/
        ecuNum = stPartCfg->stEcuCfgList.num;
        log_i("ecu num is %d", ecuNum);
        if (0 == ecuNum) {
            log_e("ecu num is zero,failed");
            retVal = ADM_ECODE_EIM_GET_ECU_CONFIG_NUM_ZERO;
            break;
        }

        /*为每个ECUINFO 分配内存*/
        getEcuInfo->ecuInfo = (ECU_INFO_T**)adm_malloc(sizeof(ECU_INFO_T*) * (D64U)ecuNum + 1U);
        (void)adm_memset(getEcuInfo->ecuInfo, 0, sizeof(ECU_INFO_T*) * (D64U)ecuNum + 1U, sizeof(ECU_INFO_T*) * (D64U)ecuNum + 1U);
        getEcuInfo->num = 0;

        eventNum = 5;
        eventsList = adm_eim_initEventList(eventNum);

        (void)adm_memset(eventDesc, 0x00, EVENT_DES_ARRAY_LEN, EVENT_DES_ARRAY_LEN);
        (void)adm_snprintf(eventDesc, EVENT_DES_ARRAY_LEN, "start get all ecuInfo");
        eim_addAllEcuEventList(eventsList, (const DCHAR*)"PS_GET_ECUINFO_003", eventDesc);
        log_d("%s", eventDesc);
        startTime = time(NULL);
        /*循环读每个ECU零件信息*/
        for (D32S i = 0; i < ecuNum; ++i) {
            if (NULL == (stPartCfg->stEcuCfgList.pstEcuCfg + i)) {
                log_w("stPartCfg->stEcuCfgList.pstEcuCfg[%d] is NULL", i);
                continue;
            }
            /*每个零件的请求ID和软件ID*/
            {
                pstEcuIdenInfo->requestId = (D32S)htoi(stPartCfg->stEcuCfgList.pstEcuCfg[i].ecuReqId);
                pstEcuIdenInfo->swDid = (D32S)htoi(stPartCfg->stEcuCfgList.pstEcuCfg[i].ecuSwDid);
                pstEcuIdenInfo->responseId = (D32S)htoi(stPartCfg->stEcuCfgList.pstEcuCfg[i].ecuResponseId);
                pstEcuIdenInfo->functionId = (D32S)htoi(stPartCfg->stEcuCfgList.pstEcuCfg[i].ecuFunctionId);
            }


            
            uiEndTime = 0U;    // ECU信息获取结束时间
            uiCostTime = 0U;   // ECU信息获取已花费时间
            bEnd = FALSE;     // ECU信息获取结束标志
            uiLastTryTime = 0U;                  // 上次获取时间

			while(!bEnd)
			{
			    uiStartTime = (D64U)time(NULL);  // ECU信息获取开始时间
			    if (uiStartTime - uiLastTryTime >= 5U)
                {
	                retVal = adm_eim_getSingleEcuInfoForEcu(
	                   stPartCfg->stEcuCfgList.pstEcuCfg + i, &(stGeneralCfg->stEcuDidList), pstEcuIdenInfo, diagScriptPath, &ecuInfo);
					
					uiEndTime = (D64U)time(NULL);
				    switch (retVal)
	                {
		                case ADM_ECODE_SUCCESSED:
		                    // 获取ecu版本成功
		                    log_i("pstEcuCfg[%d] read diagnostic info,success", i);
		                    getEcuInfo->ecuInfo[getEcuInfo->num] = ecuInfo;
		                    getEcuInfo->num += 1;
							bEnd = TRUE;
							
		                    break;
		                case ADM_ECODE_RPC_INSTANCE_OFFLINE:
		                    /*no break*/
		                case ADM_ECODE_RPC_SERVICE_OFFLINE:
		                    /*no break*/
		                case ADM_ECODE_RPC_REQSERV_TIMEOUT:
		                    log_e("get ecu info faild! ecuReqId[%s] ecuSwDid[%s] result[%d]", stPartCfg->stEcuCfgList.pstEcuCfg[i].ecuReqId, stPartCfg->stEcuCfgList.pstEcuCfg[i].ecuSwDid, retVal);

		                    uiCostTime += uiEndTime > uiStartTime ? uiEndTime - uiStartTime : 6U;  // 获取ECU信息未超过1秒按一秒计算
		                    uiLastTryTime = uiEndTime; // 以ECU获取结束时间作为本次获取尝试时间
		                    /*ECU校验获取ECU信息因RPC实例或RPC服务或请求超时原因，可以在30s内尝试重新获取ECU信息校验*/
		                    if (uiCostTime >= SUM_GETECUINFO_TIMEOUT) // 超时失败
		                    {
		                        bEnd = TRUE;
		                        log_e("pstEcuCfg[%d] read diagnostic info time out,failed", i);
		                    }
		                    break;
		                default:    // 其他原因，获取失败
		                    bEnd = TRUE;
		                    log_e("get ecu info faild! ecuReqId[%s] ecuSwDid[%s] result[%d]", stPartCfg->stEcuCfgList.pstEcuCfg[i].ecuReqId, stPartCfg->stEcuCfgList.pstEcuCfg[i].ecuSwDid, retVal);
		                    break;
	                }
		    	}

				(void)usleep(GETECUINFO_SLEEPTIME);  // 等待20ms，防止循环导致CPU占用率过高

			}



			
            if (ADM_ECODE_SUCCESSED != retVal) {
                log_e("pstEcuCfg[%d] read diagnostic info, error", i);
            } 
        }
    } while (FALSE);
    endTime = time(NULL);
    if (0 == getEcuInfo->num) {
        log_e("no part information was read");
        (void)adm_memset(eventDesc, 0x00, EVENT_DES_ARRAY_LEN, EVENT_DES_ARRAY_LEN);
        (void)adm_snprintf(eventDesc, EVENT_DES_ARRAY_LEN, "end get all ecuInfo:failed");
        eim_addAllEcuEventList(eventsList, (const DCHAR*)"PS_GET_ECUINFO_004", eventDesc);
        log_d("%s", eventDesc);
        retVal = ADM_ECODE_EIM_GET_ECU_INFO_FAILED;
    } else {
        (void)adm_memset(eventDesc, 0x00, EVENT_DES_ARRAY_LEN, EVENT_DES_ARRAY_LEN);
        (void)adm_snprintf(eventDesc,
            EVENT_DES_ARRAY_LEN,
            "end get all ecuInfo:successful[%d]",
            endTime > startTime ? (endTime - startTime) : 0);
        eim_addAllEcuEventList(eventsList, (const DCHAR*)"PS_GET_ECUINFO_004", eventDesc);
        log_d("%s", eventDesc);
        retVal = ADM_ECODE_SUCCESSED;
    }
    adm_eim_getEcuInfoReport(eventsList);
    adm_eim_deInitEventsList(eventsList);
    adm_free(diagScriptPath);
    adm_free(pstEcuIdenInfo);
    return retVal;
}

D32S adm_eim_getVehicleBigVersion(DCHAR** vehicleBigVersion)
{
    D32S errorCode = 0;
    return adm_dim_host_getVehicleBigVersion(
        adm_soa_getHostInstanceId(), GETVERSION_TIMEOUT, vehicleBigVersion, &errorCode);
}

D32S adm_eim_setVehicleBigVersion(const DCHAR* vehicleBigVersion)
{
    D32S errorCode = 0;
    return adm_dim_host_setVehicleBigVersion(
        adm_soa_getHostInstanceId(), GETVERSION_TIMEOUT, vehicleBigVersion, &errorCode);
}

D32S adm_eim_getVehicleVersion(DCHAR** pVehicleVersion)
{
    D32S errorCode = 0;
    return adm_dim_getVehicleVersion(
        adm_soa_getHostInstanceId(), GETVERSION_TIMEOUT, pVehicleVersion, &errorCode);
}
D32S adm_eim_setVehicleVersion(const DCHAR* pVehicleVersion)
{
    D32S errorCode = 0;
    return adm_dim_setVehicleVersion(
        adm_soa_getHostInstanceId(), GETVERSION_TIMEOUT, pVehicleVersion, &errorCode);
}

D32S adm_eim_getApkVersion(const D32S instanceId, const ADM_ECS_ECUCONFIG_T* pstEcuCfg, const ADM_DIM_SOA_ECU_IDEN_INFO_T* pstEcuIdenInfo,
    const ADM_DIM_SOA_DID_DATA_ARRAY_T* didArrayRequest,
    ADM_DIM_SOA_DID_DATA_ARRAY_T** didListResponse, D32S* errorCode)
{
    D32S iRet = ADM_ECODE_FAILED;
    DCHAR* pApkVersion = DNULL;
    D32S i = 0;

    log_i("adm_eim_getApkVersion begin");
    do
    {
        if (DNULL == pstEcuIdenInfo || DNULL == pstEcuCfg || DNULL == didArrayRequest || DNULL == didListResponse)
        {
            iRet = ADM_ECODE_INVALID_PARAM;
            log_e("Input param pstEcuIdenInfo || didArrayRequest || didListResponse is nil, [err:%d]", iRet);
            break;
        }

        if (ADM_EIM_APK_UCM_REQUEST_ID == pstEcuIdenInfo->requestId)
        {
            // 获取主控的APK Version == UCM
            iRet = adm_sdk_getApkVersion(&pApkVersion);
            *errorCode = ADM_ECODE_SUCCESSED;
            if (ADM_ECODE_SUCCESSED != iRet || ADM_ECODE_SUCCESSED != *errorCode)
            {
                log_i("UCM APK version get failed, [iRet:%d, err:%d]", iRet, *errorCode);
                break;
            }
            
        }
        else if (ADM_EIM_APK_HMI_REQUEST_ID == pstEcuIdenInfo->requestId)
        {
            iRet = adm_dim_ucl_getApkVersion(instanceId, 5, pstEcuIdenInfo, &pApkVersion, errorCode);
            if (ADM_ECODE_SUCCESSED != iRet || ADM_ECODE_SUCCESSED != *errorCode)
            {
                log_i("HMI APK version get failed, [iRet:%d, err:%d]", iRet, *errorCode);
                break;
            }
        }

        //!< 填充DID信息
        (*didListResponse) = (ADM_DIM_SOA_DID_DATA_ARRAY_T*)adm_malloc(sizeof(ADM_DIM_SOA_DID_DATA_ARRAY_T));
        if (DNULL == (*didListResponse))
        {
            iRet = ADM_ECODE_APPLY_MEMORY_FAILED;
            log_e("adm_malloc failed, [err:%d]", iRet);
            break;
        }

        (*didListResponse)->didArray = (ADM_DIM_SOA_DID_DATA_T**)adm_malloc(sizeof(ADM_DIM_SOA_DID_DATA_T*) * (didArrayRequest->didArrayLength));
        (*didListResponse)->didArrayLength = (D16U)(didArrayRequest->didArrayLength);

        for (i = 0; i < didArrayRequest->didArrayLength; i++)
        {
            log_d("did:%d, startByte:%d, endByte:%d, dataType:%d", didArrayRequest->didArray[i]->did, didArrayRequest->didArray[i]->startByte, didArrayRequest->didArray[i]->endByte, VAR_STRING);

            (*didListResponse)->didArray[i] = (ADM_DIM_SOA_DID_DATA_T*)adm_malloc(sizeof(ADM_DIM_SOA_DID_DATA_T));
            (*didListResponse)->didArray[i]->did = didArrayRequest->didArray[i]->did;
            (*didListResponse)->didArray[i]->errorCode = 0;
            (*didListResponse)->didArray[i]->didData = (ADM_DIM_SOA_DATA_T*)adm_malloc(sizeof(ADM_DIM_SOA_DATA_T));
            (*didListResponse)->didArray[i]->didData->dataType = VAR_STRING;
            (*didListResponse)->didArray[i]->startByte = (D16U)didArrayRequest->didArray[i]->startByte;
            (*didListResponse)->didArray[i]->endByte = (D16U)didArrayRequest->didArray[i]->endByte;

            log_d("did:%d, startByte:%d, endByte:%d, dataType:%d", (*didListResponse)->didArray[i]->did, (*didListResponse)->didArray[i]->startByte, (*didListResponse)->didArray[i]->endByte, (*didListResponse)->didArray[i]->didData->dataType);

            switch (didArrayRequest->didArray[i]->did)
            {
            case 0xF190:            ///!< 零件编号did信息
                (*didListResponse)->didArray[i]->didData->v.valueS = (D8U*)adm_strdup(pstEcuCfg->stExternEcuInfo.partNumber);
                (*didListResponse)->didArray[i]->didData->dataLength = adm_strlen(pstEcuCfg->stExternEcuInfo.partNumber);
                break;
            case 0xF191:            ///!< 供应商编码did信息
                (*didListResponse)->didArray[i]->didData->v.valueS = (D8U*)adm_strdup(ADM_EIM_APK_SUPP_CODE_DID_VAL);
                (*didListResponse)->didArray[i]->didData->dataLength = adm_strlen(ADM_EIM_APK_SUPP_CODE_DID_VAL);
                break;
            case 0xF192:            ///!< 软件版本号did信息
                (*didListResponse)->didArray[i]->didData->v.valueS = (D8U*)adm_strdup(pApkVersion);
                (*didListResponse)->didArray[i]->didData->dataLength = adm_strlen(pApkVersion);
                break;
            case 0xF194:            ///!< 硬件版本号did信息
                (*didListResponse)->didArray[i]->didData->v.valueS = (D8U*)adm_strdup(ADM_EIM_APK_HARD_WARE_DID_VAL);
                (*didListResponse)->didArray[i]->didData->dataLength = adm_strlen(ADM_EIM_APK_HARD_WARE_DID_VAL);
                break;
            case 0xF195:            ///!< 序列号did信息
                (*didListResponse)->didArray[i]->didData->v.valueS = (D8U*)adm_strdup(ADM_EIM_APK_SERIAL_NUM_DID_VAL);
                (*didListResponse)->didArray[i]->didData->dataLength = adm_strlen(ADM_EIM_APK_SERIAL_NUM_DID_VAL);
                break;
            case 0xF196:            ///!< 生产日期
                (*didListResponse)->didArray[i]->didData->v.valueS = (D8U*)adm_strdup(ADM_EIM_APK_PROD_DATE_DID_VAL);
                (*didListResponse)->didArray[i]->didData->dataLength = adm_strlen(ADM_EIM_APK_PROD_DATE_DID_VAL);
                break;   
            default:
                log_w("did = %02x", didArrayRequest->didArray[i]->did);
                break;
            }

            log_i("didKey:%02x, didValue:%s", didArrayRequest->didArray[i]->did, (*didListResponse)->didArray[i]->didData->v.valueS);
        }

    } while (FALSE);

    ///!< 释放内存
    if (DNULL != pApkVersion)
    {
        adm_free(pApkVersion);
    }
    log_i("adm_eim_getApkVersion end");
    
    return iRet;
}
