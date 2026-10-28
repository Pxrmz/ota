/**
 * @file adm_apc_event.h
 * @author mengwei@abupdate.com
 * @brief 定义主控初始化程序的事件，以及事件上报的接口
 * @version 0.1
 * @date 2022-03-15
 * 
 * @copyright Copyright © 2018 All rights 上海艾拉比智能科技有限公司
 * 
 */
#ifndef __ADM_APC_EVENT_H__
#define __ADM_APC_EVENT_H__

#include "adm_typedefs.h"
#include "adm_ocs_airProtocolAdapter.h"

#define ADM_APC_EVENT_REPORT_COUNT      10           //!< 事件上报计数，用于判断触发上报
#define ADM_APC_EV_INIT_ING_MAX         70           //!< 初始化中的最大事件数，用于判断触发上报

/**
 * @brief 初始化事件阶段定义
 * 
 */
typedef enum {
    ADM_APC_EV_STAGE_PRE = 0x01,    //!< 初始化准备阶段
    ADM_APC_EV_STAGE_ING = 0x02,    //!< 初始化中阶段
    ADM_APC_EV_STAGE_END = 0x03,    //!< 初始化结束阶段
    ADM_APC_EV_STAGE_INV = 0x04,    //!< 无效值
} ADM_APC_EV_STAGE_E;

/**
 * @brief 增加事件给到事件列表
 * 
 * @param eStage        [IN]    事件阶段
 * @param serNum        [IN]    事件序号
 * @param eventLevel    [IN]    事件级别
 * @param errCode       [IN]    事件错误码，成功：0，失败：错误码
 * @param eventContext  [IN]    事件详细上下文，用于记录事件产生时的详细上下文
 */
extern void adm_apc_eventAdd(const ADM_APC_EV_STAGE_E eStage, const D32U serNumber, const D32S eventLevel, const D32S errCode, const DCHAR* eventContext);

/**
 * @brief 触发上报接口，上报APP CORE模块产生的事件信息
 * 
 */
extern D32S adm_apc_triggerReportEvent(void);

//TODO: 需要统一定义到IRM模块中
typedef D64S (*ADM_APC_IRM_GET_EV_SEQID_CALLBACK_F)(void);       //!< 获取事件顺序ID

typedef D32S (*ADM_APC_IRM_REPORT_EVENT_CALLBACK_F)(const DCHAR* sessionId,
    D64S taskId,
    D64S strategyId,
    D64S timestamp,
    const DCHAR* sdkVersion,
    const DCHAR* extsReqJsonStr,
    ADM_OCS_EVENTS_INFO_LIST_T* eventsList);           //!< 执行事件上报的接口

/**
 * @brief 初始化程序启动阶段的事件上报模块
 * 
 */
extern void adm_apc_initReportEvent(ADM_APC_IRM_GET_EV_SEQID_CALLBACK_F getSeqIdCb, 
                                    ADM_APC_IRM_REPORT_EVENT_CALLBACK_F reportEventCb);

#endif//__ADM_APC_EVENT_H__
