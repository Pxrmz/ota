

#ifndef __ADM_SYSI4HOST_JNI_H__
#define __ADM_SYSI4HOST_JNI_H__


#ifdef __cplusplus
extern "C" {
#endif


//jni函数类型
typedef D32S(*sendAuthenticateEventCallback)(D32S iEventId, D32S* pErrorCode);
typedef D32S(*sendAuthenticateMsgCallback)(D8U* pResult, DSIZE uiSize, D32S* pErrorCode);
typedef D32S(*controlOtaModeCallback)(D32S iControlType, D32S* pErrorCode);
typedef D32S(*controlHighVoltageCallback)(D32S iControlType, D32S* pErrorCode);
typedef D32S(*controlNetWorkWakeUpCallback)(D32S iControlType, D32S* pErrorCode);
typedef D32S(*getEthernetTypeCallback)(D32S* pErrorCode);
typedef D32S(*getTboxNetworkTypeCallback)(D32S* pErrorCode);
typedef D32S(*screenOverControlCallback)(D32S iControlType, D32S* pErrorCode);
typedef D32S(*setWifiDisLinkCallback)(D32S* pErrorCode);
typedef D32S(*getSkScInfoCallback)(DCHAR** ppSkScInfoStr);
typedef D32S(*setUpgradeModeCallback)(D32S iUpgradeMode, D32S* pErrorCode);
typedef D32S(*setWifiLinkCallback)(DCHAR* pWifiListStr, D32S* pErrorCode);
typedef D32S(*getSmartScenarioCallback)(DCHAR* pScenarioArray, DCHAR** ppResponse);
typedef D32S(*getVehicleSignalCallback)(DCHAR* pSignalArray, DCHAR** ppResponse);
typedef D32S(*getVehOSVersionCallback)(DCHAR** ppRespons);
typedef D32S(*setVehOSVersionCallBack)(DCHAR* pBigVersionNumber, D32S* pErrorCode);
typedef D32S(*getHostVersionCallback)(D32S requestId, const DCHAR* pVerTypeArray, DCHAR** ppRespons);
typedef D32S(*getPrivacyPathCallback)(DCHAR** ppRespons);
typedef D32S(*downExceptionNotifyCallback)(D32S iResult, D32S* pErrorCode);
typedef D32S(*getOtaModeStatusCallback)(D32S *pOtaModeStatus);
typedef D32S(*setIcuPowerOffCallback)(D32S* pErrorCode);
typedef D32S (*notifyOtaHmiStartCallback)(D32S *pErrorCode);
typedef D32S(*controlWakeUpSignCallback)(D32S iControlMode, D32S iTimeOut, D32S* pErrorCode);
typedef D32S (*getRemoteDiaStatusCallback)();

//成员
typedef struct
{
    sendAuthenticateEventCallback      sendAuthenticateEventCb;
    sendAuthenticateMsgCallback        sendAuthenticateMsgCb;
    controlOtaModeCallback             controlOtaModeCb;
    controlHighVoltageCallback         controlHighVoltageCb;
    screenOverControlCallback          screenOverControlCb;
    setWifiDisLinkCallback             setWifiDisLinkCb;
    getSkScInfoCallback                getSkScInfoCb;
    setUpgradeModeCallback             setUpgradeModeCb;
    setWifiLinkCallback                setWifiLinkCb;
    getSmartScenarioCallback           getSmartScenarioCb;
    getVehicleSignalCallback           getVehicleSignalCb;
    getVehOSVersionCallback           getVehOSVersionCb;
    setVehOSVersionCallBack           setVehOSVersionCb;
    getHostVersionCallback           getHostVersionCb;
    getPrivacyPathCallback           getPrivacyPathCb;
	downExceptionNotifyCallback        downExceptionNotifyCb;
    getOtaModeStatusCallback           getOtaModeStatusCb;
    setIcuPowerOffCallback             setIcuPowerOffCallback;
    notifyOtaHmiStartCallback          notifyOtaHmiStartCb;
    controlWakeUpSignCallback          controlWakeUpSignModeCb;
    getRemoteDiaStatusCallback         getRemoteDiaStatusCb;
} ADM_SYSI4HOST_JNI_CALLBACK_T;

extern D32S adm_sysi4host_jni_Init(ADM_SYSI4HOST_JNI_CALLBACK_T* pstJniCb);
extern D32S adm_sysi4host_jni_Deinit(DVOID);
extern D32S sysi4host_powerOnResultNotify(D32S result);
extern D32S sysi4host_powerOffResultNotify(D32S result);
extern D32S adm_sysi4host_canResponseNotify(D8U *canMsg);
extern void sysi4host_smartScenarioChanged(DCHAR *pMsg);
extern void sysi4host_vehicleSignalChanged(DCHAR *pMsg);
extern void sysi4host_hostJustOnNotify(D32S reason);
extern void sysi4host_hostWillOffNotify(D32S timeout_ms);
extern D32S adm_sysi4host_ipmGetInstallSchedule(DCHAR ** pMsg);

#ifdef __cplusplus
}
#endif


#endif  //__ADM_SYSI4HOST_JNI_H__

