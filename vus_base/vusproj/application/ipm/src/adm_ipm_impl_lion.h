/**
 * @file adm_ipm_impl_lion.h
 * @author maxiangyang (maxiangyang@abupdate.com)
 * @brief 雄狮项目安装模块-改造功能
 * @version 0.1
 * @date 2024-12-25
 * 
 * @copyright Copyright (c) 2024 上海艾拉比智能科技有限公司
 * 
 * @par 修改日志:
 * <table>
 * <tr><th>Date       <th>Version <th>Author  <th>Description
 * <tr><td>2024-12-25 <td>1.0     <td>maxiangyang    <td>内容
 * </table>
 */
#ifndef	__ADM_IPM_IMPL_LION_H_
#define __ADM_IPM_IMPL_LION_H_

#include "adm_ipm_impl.h"

#ifdef __cplusplus
extern "C" {
#endif

/** 业务脚本类型枚举 */
typedef enum
{
    ADM_IPM_SCRIPT_READ_ECU_INFO = 1,       ///< 读取零件信息脚本
    ADM_IPM_SCRIPT_SWITCH_AB = 2            ///< 切面脚本
} ADM_IPM_BUSSINESS_SCRIPT_TYPE_E;

/** 刷写脚本类型枚举 */
typedef enum
{
    ADM_IPM_SCRIPT_MAIN_PROGRAMMING = 1,    ///< 主编程脚本
    ADM_IPM_SCRIPT_PRE_PROGRAMMING = 2,     ///< 预编程脚本
    ADM_IPM_SCRIPT_POST_PROGRAMMING = 3     ///< 后编程脚本
} ADM_IPM_FLASH_SCRIPT_TYPE_E;

static D32S ipmGetSchduleNotifyPreCondition(DBOOL bNeedSmart, DBOOL* pCondSatisfy, DCHAR** ppVerifyResult);
extern D32S adm_ipm_getAbInfo(D16U uiInstanceId, D16U uiTimeOut, DBOOL* pIsEnableAb, 
            ADM_DIM_SOA_VDI_ECU_AREA_INFO_E* pAbInfo, 
            ADM_DIM_SOA_VDI_REFLASH_INFO_ARRAY_T* pReflashInfoList);
extern DVOID adm_ipm_getDestAbInfo(ADM_DIM_SOA_VDI_ECU_AREA_INFO_E pSrcAbInfo, ADM_DIM_SOA_VDI_ECU_AREA_INFO_E* pDstAbInfo);
extern DVOID adm_ipm_getExitDestAbInfo(const DCHAR* pRequestId, const DCHAR* pSwDid, ADM_DIM_SOA_VDI_ECU_AREA_INFO_E* pExitDstAbInfo);
extern DVOID adm_ipm_initSaMask(const DCHAR* pRequestId, const DCHAR* pSwDid, ADM_DIM_SOA_SA_MASK_INFO_T **ppSaMaskList);
extern DVOID adm_ipm_deInitSaMask(ADM_DIM_SOA_SA_MASK_INFO_T **ppSaMaskList);
extern DVOID adm_ipm_updateFlashAbInfo(DBOOL bIsAb, const DCHAR* pRequestId, const DCHAR* pSwDid,
                        DBOOL bIsBlockSwitch,
                        ADM_DIM_SOA_VDI_ECU_AREA_INFO_E eDestAbInfo);
extern DVOID adm_ipm_updateFlashAbSwitch(const DCHAR* pRequestId, const DCHAR* pSwDid, DBOOL bIsHashSwitch);
extern DVOID adm_ipm_updateRollbackAbSwitch(const DCHAR* pRequestId, const DCHAR* pSwDid, DBOOL bIsHashSwitch);
extern DVOID adm_ipm_updFLashAbSwitchFailed(const DCHAR* pRequestId, const DCHAR* pSwDid, D32S iResult);
extern DVOID adm_ipm_updRollbackAbSwitchFailed(const DCHAR* pRequestId, const DCHAR* pSwDid, D32S iResult, D32S iUpgradeNumbers, D64S iEndTime);

extern DVOID adm_ipm_getFlashAbInfo(const DCHAR* pRequestId, const DCHAR* pSwDid, 
            DBOOL* pIsAbInfo, DBOOL* pIsBlockSwitchAb,
            ADM_DIM_SOA_VDI_ECU_AREA_INFO_E* pDestAbInfo);

extern DVOID adm_ipm_getIsRollback(const DCHAR *pEcuReqId, const DCHAR *pEcuSwDid, 
        DBOOL *pIsRollback);

extern D32S adm_ipm_getBussinessScipt(ADM_IPM_BUSSINESS_SCRIPT_TYPE_E eBussType, DCHAR** pScriptPath);
extern D32S adm_ipm_getFlashScipt(ADM_IPM_FLASH_SCRIPT_TYPE_E eFlashType, DCHAR** pScriptPath);
extern D32S adm_ipm_switchAb(const DCHAR* pRequestId, const DCHAR* pSwDid,
        const D16U uiTimeOut,
        const ADM_DIM_SOA_VDI_ECU_AREA_INFO_E eDstAreaInfo);
extern DVOID adm_ipm_notifyFlashState(ADM_DIM_SOA_VDI_FLASH_STATE_E eFlashState, D32U uiTimeOut);


#ifdef __cplusplus
}
#endif

#endif//__ADM_IPM_IMPL_LION_H_