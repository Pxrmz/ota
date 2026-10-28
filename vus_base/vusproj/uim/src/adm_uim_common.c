#include "adm_uim_log.h"
#include "json_types.h"
#include "json_object.h"
#include "json_tokener.h"
#include "adm_typedefs.h"
#include "adm_com_errorCode.h"
#include "adm_memory.h"
#include "adm_string.h"
#include "adm_soa.h"
#include "adm_uim_common.h"
#include "adm_arpc_service.h"

DCHAR* adm_uim_buildConditionCheckResponse(const ADM_UIM_CHECK_CONDITION_RESULT_T* pInfo)
{
	if(NULL == pInfo)
	{
		return NULL;
	}
	
    DCHAR* pReqStr = NULL;
    json_object* pObj = NULL;
    json_object* pCondList = NULL;
    json_object* pItem = NULL;
    json_object* pVal = NULL;
    D16U condSize =0;
    D32S buildRet = ADM_ECODE_SUCCESSED;
    do
    {
        json_new_object(pObj);
        buildCommon(pObj, pInfo);
        json_add_int(pObj, "verifyResult", pInfo->verifyResult);

		if(NULL == pInfo->condsArrarList)
		{
			break;
		}

        //build condsArrarList
        condSize = pInfo->condsArrarList->condsArrayLength;
		char buf[24] = {0};
        json_new_array(pCondList, (int)condSize);
        ADM_UIM_SOA_CONDITION_DATA_T**  condsArray = pInfo->condsArrarList->condsArray;
        for(D16U i=0; i<condSize; i++)
        {
            json_new_object(pItem);
            json_add_int(pItem, "msgId", condsArray[i]->msgId);
            json_add_int(pItem, "result", condsArray[i]->result);
            //build expectVariable
            if(NULL == condsArray[i]->expectVariable){
					continue;
            }
            json_new_object(pVal); 
            json_add_int(pVal, "dataType", (int32_t)condsArray[i]->expectVariable->data_type);
            json_add_int(pVal, "dataLength", (int32_t)condsArray[i]->expectVariable->data_length);
	        switch(condsArray[i]->expectVariable->data_type)
	        {
		        case 1:
					json_add_int(pVal, "data", (int32_t)(condsArray[i]->expectVariable->v.value[0]));
	                break;
		        case 2:
					json_add_uint64(pVal, "data", (D64U)condsArray[i]->expectVariable->v.value_u32);
		            break;
		        case 3:
					json_add_int(pVal, "data", (int32_t)condsArray[i]->expectVariable->v.value_s32);
		            break;
		        case 4:
					json_add_uint64(pVal, "data", (D64U)condsArray[i]->expectVariable->v.value_u64);
		            break;
		        case 5:
					json_add_double(pVal, "data", condsArray[i]->expectVariable->v.value_f);
		            break;
		        case 6:
					json_add_string(pVal, "data", (DCHAR *)condsArray[i]->expectVariable->v.value_s);
		            break;
		        default:
		            break;
	        } 			
            
            json_add_object(pItem, "expectVariable", pVal);
            //build realVariable
            if(NULL == condsArray[i]->realVariable){
					continue;
            }
            json_new_object(pVal); 
            json_add_int(pVal, "dataType", (int32_t)condsArray[i]->realVariable->data_type);
            json_add_int(pVal, "dataLength", (int32_t)condsArray[i]->realVariable->data_length);
	        switch(condsArray[i]->realVariable->data_type)
	        {
		        case 1:
					json_add_int(pVal, "data", (int32_t)(condsArray[i]->realVariable->v.value[0]));
	                break;
		        case 2:
					json_add_uint64(pVal, "data", (D64U)condsArray[i]->realVariable->v.value_u32);
		            break;
		        case 3:
					json_add_int(pVal, "data", (int32_t)condsArray[i]->realVariable->v.value_s32);
		            break;
		        case 4:
					json_add_uint64(pVal, "data", (D64U)condsArray[i]->realVariable->v.value_u64);
		            break;
		        case 5:
					json_add_double(pVal, "data", condsArray[i]->realVariable->v.value_f);
		            break;
		        case 6:
					json_add_string(pVal, "data", (DCHAR *)condsArray[i]->realVariable->v.value_s);
		            break;
		        default:
		            break;
	        }
            json_add_object(pItem, "realVariable", pVal);
            //end build item
            json_add_array_item(pCondList, pItem);
        }
        json_add_object(pObj, "condsArrarList", pCondList);
        pReqStr = adm_strdup(json_object_get_string(pObj));
    }while(FALSE);
end_build_json:

    json_free_object(pObj);
    return pReqStr;
}

/**
 * @brief 构建条件检查结果响应报文
 * @note 扩展接口
 * @param[in] pInfo 条件检查结果信息
 * @return DCHAR* 条件检查结果响应报文
 */
DCHAR* adm_uim_buildConditionCheckResponseEx(const ADM_UIM_CHECK_CONDITION_RESULT_T* pInfo)
{
    DCHAR* pReqStr = NULL;
    json_object* pObj = NULL;
    json_object* pCondList = NULL;
    json_object* pItem = NULL;
    json_object* pVal = NULL;
    D16U condSize =0;
    D32S buildRet = ADM_ECODE_SUCCESSED;

    if (NULL == pInfo)
    {
        return NULL;
    }

    do
    {
        json_new_object(pObj);
        buildCommon(pObj, pInfo);
        json_add_int(pObj, "verifyResult", pInfo->verifyResult);

        if (NULL == pInfo->condsArrarList)
        {
            break;
        }

        // build condsArrarList
        condSize = pInfo->condsArrarList->condsArrayLength;
        DCHAR buf[24] = {0};
        json_new_array(pCondList, (int)condSize);
        ADM_UIM_SOA_CONDITION_DATA_T **condsArray = pInfo->condsArrarList->condsArray;
        for (D16U i = 0; i < condSize; i++)
        {
            json_new_object(pItem);
            json_add_int(pItem, "msgId", condsArray[i]->msgId);
            json_add_int(pItem, "result", condsArray[i]->result);

            // build realVariable
            if (NULL == condsArray[i]->realVariable)
            {
                continue;
            }

            /** 成功不需要返回realVariable */
            if (0 != condsArray[i]->result)
            {
                json_new_object(pVal);
                switch (condsArray[i]->realVariable->data_type)
                {
                case 1:
                    json_add_int(pVal, "data", (int32_t)(condsArray[i]->realVariable->v.value[0]));
                    break;
                case 2:
                    json_add_uint64(pVal, "data", (D64U)condsArray[i]->realVariable->v.value_u32);
                    break;
                case 3:
                    json_add_int(pVal, "data", (int32_t)condsArray[i]->realVariable->v.value_s32);
                    break;
                case 4:
                    json_add_uint64(pVal, "data", (D64U)condsArray[i]->realVariable->v.value_u64);
                    break;
                case 5:
                    json_add_double(pVal, "data", condsArray[i]->realVariable->v.value_f);
                    break;
                case 6:
                    json_add_string(pVal, "data", (DCHAR *)condsArray[i]->realVariable->v.value_s);
                    break;
                default:
                    break;
                }
                json_add_object(pItem, "realVariable", pVal);
            }

            // end build item
            json_add_array_item(pCondList, pItem);
        }
        json_add_object(pObj, "condsArrarList", pCondList);
        pReqStr = adm_strdup(json_object_get_string(pObj));
    } while (FALSE);

end_build_json:

    json_free_object(pObj);
    return pReqStr;
}

/**
 * @brief 确保事件订阅成功
 * @note
 * @param in D16U usServiceId 服务ID
 * @param in D16U usEventId 事件ID
 * @return void
 */
void adm_uim_ensureCheckSubedEventSuccess(D16U usServiceId, D16U usEventId)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    D32S iWaitCount = 0;

    do
    {
        /* 查询事件是否订阅成功，1：订阅成功；0：未订阅成功 */
        iRet = adm_arpc_checkIfClientSubedEvent(adm_soa_getUimInstanceId(),
                                                usServiceId,
                                                usEventId,
                                                adm_soa_getHmiInstanceId());
        log_w("adm_arpc_checkIfClientSubedEvent 0x%x, iRet = %d", usEventId,
              iRet);
        if (1 == iRet)
        {
            log_i("usEventId 0x%x Subed Success", usEventId);
            break;
        }

        /** 20分钟仍未订阅成功，等待超时退出 */
        if (iWaitCount >= 20 * 60)
        {
            log_e("usEventId 0x%x Subed Failed", usEventId);
            break;
        }
        sleep(1);

    } while (TRUE);

    return;
}