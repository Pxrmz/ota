#ifndef __ADM_DIM_SOA_CFG_H__
#define __ADM_DIM_SOA_CFG_H__


//#define TBox_INSTANCEID         0x01

#define UIM2TBOX_INSTANCE_ID        0X0001
#define UIM2HUT_INSTANCE_ID         0X0002
#define UIM2CGW_INSTANCE_ID         0X0003
#define UIM2ADAS_INSTANCE_ID        0X0004
#define UIM2ARHUD_INSTANCE_ID       0X0005

#define SYSI4VDI2TBOX_INSTANCE_ID   0X1001
#define SYSI4VDI2HUT_INSTANCE_ID    0X1002
#define SYSI4VDI2CGW_INSTANCE_ID    0X1003
#define SYSI4VDI2ADAS_INSTANCE_ID   0X1004
#define SYSI4VDI2ARHUD_INSTANCE_ID  0X1005

// ucl include trans dpo
#define SYSI4UCL2TBOX_INSTANCE_ID   0X2001
#define SYSI4UCL2HUT_INSTANCE_ID    0X2002
#define SYSI4UCL2CGW_INSTANCE_ID    0X2003
#define SYSI4UCL2ADAS_INSTANCE_ID   0X2004
#define SYSI4UCL2ARHUD_INSTANCE_ID  0X2005

#define SYSI4HOST2TBOX_INSTANCE_ID  0X3001
#define SYSI4HOST2HUT_INSTANCE_ID   0X3002
#define SYSI4HOST2CGW_INSTANCE_ID   0X3003
#define SYSI4HOST2ADAS_INSTANCE_ID  0X3004
#define SYSI4HOST2ARHUD_INSTANCE_ID 0X3005

#define SYSI4HMI2HUT_INTSTANCE_ID   0x0103

/********************UIM begin******************/

// service id
#define ADM_UIM_CHECK_SERVICE_ID            0x0200
#define ADM_UIM_DOWNLOAD_SERVICE_ID         0x0201
#define ADM_UIM_TRANSFER_LISTEN_SERVICE_ID  0x0202
#define ADM_UIM_INSTALL_SERVICE_ID          0x0203
#define ADM_UIM_EXT1_SERVICE_ID             0x0204
#define ADM_UIM_EXT2_SERVICE_ID             0x0205
#define ADM_UIM_CHECK_ICP_SERVICE_ID            0x0209

#define ADM_UIM_HMISERVICE_ID               0x0206    //uim request hmi as service
#define ADM_UIM_GENERAL_SERVICE_ID          0x0207    //UIM general affairs service id

#define ADM_UIM_ENGINEER_MODE_SERVICE_ID    0x0208      // OTA engineer mode

// method id
#define ADM_UIM_HMI_TRIGGER_CHECK_METHOD_ID                     0x0002  // adm_sysi4hmi_triggerCheck
#define ADM_UIM_HMI_QUERY_CHECK_RESULT_METHOD_ID                0x0003  // adm_sysi4hmi_queryCheckResult
#define ADM_UIM_HMI_USB_TRIGGER_CHECK_METHOD_ID                 0x0005  // adm_sysi4hmi_usbTriggerCheck
#define ADM_UIM_HMI_QUERY_ECU_INFO_METHOD_ID                    0x0006  // adm_sysi4hmi_usbTriggerCheck
#define ADM_UIM_HMI_TRIGGER_DOWNLOAD_METHOD_ID                  0x0015  // adm_sysi4hmi_triggerDownload
#define ADM_UIM_HMI_QUERY_ALL_ECU_DOWNLOAD_RESULT_METHOD_ID     0x0016  // adm_sysi4hmi_queryAllEcuDownloadResult
#define ADM_UIM_HMI_QUERY_ALL_ECU_DOWNLOAD_PROGRESS_METHOD_ID   0x0017  // adm_sysi4hmi_queryAllEcuDownloadProgress
#define ADM_UIM_HMI_DOWNLOAD_CONDITIONS_VERIFY_METHOD_ID        0x0020  // adm_sysi4hmi_downloadContitionsVerified
#define ADM_UIM_HMI_CANCEL_DOWNLOAD_METHOD_ID                   0x0022  // adm_sysi4hmi_cancelDownload
#define ADM_UIM_HMI_PAUSE_DOWNLOAD_METHOD_ID                    0x0023  // adm_sysi4hmi_pauseDownload
#define ADM_UIM_HMI_AUTO_DOWNLOAD_METHOD_ID                     0x0024  // adm_sysi4hmi_autoDownload
#define ADM_UIM_HMI_TRIGGER_INSTALL_METHOD_ID                   0x0105  // adm_sysi4hmi_triggerInstall
#define ADM_UIM_HMI_QUERY_ALL_ECU_INSTALL_RESULT_METHOD_ID      0x0106  // adm_sysi4hmi_queryAllEcuIntallResult
#define ADM_UIM_HMI_QUERY_ALL_ECU_INSTALL_PROGRESS_METHOD_ID    0x0107  // adm_sysi4hmi_queryAllEcuInstallProgress
#define ADM_UIM_HMI_SCHDULE_INSTALL_METHOD_ID                   0x0110  // adm_sysi4hmi_schduleInstall
#define ADM_UIM_HMI_CANCEL_INSTALL_METHOD_ID                    0x0111  // adm_sysi4hmi_cancelInstall
#define ADM_UIM_HMI_REFUSE_INSTALL_METHOD_ID                    0x0115  // adm_sysi4hmi_refuseInstall
#define ADM_UIM_HMI_INSTALL_CONDITIONS_VERIFY_METHOD_ID         0x0112  // adm_sysi4hmi_installConditionsVerified
#define ADM_UIM_HMI_REQUEST_READY_METHOD_ID                     0x0116  // adm_sysi4hmi_ready_notcie
#define ADM_UIM_HMI_GET_INSTALL_SCHEDULE_METHOD_ID              0x0156

#define ADM_UIM_HMI_QUERY_SMART_SCENARIO_METHOD_ID                   0x0117 // 查询是否智能场景
#define ADM_UIM_HMI_NOTIFY_CONFIRM_METHOD_ID                    0x0118 //告知用户已点击确定按钮,点击后重启后无需再次通知policyInstallResult

#define ADM_UIM_HMISERVICE_SET_LOG_LEVEL                                0x0120  //hmi as service
#define ADM_UIM_HMISERVICE_GETSDKVERSION_METHOD_ID                      0x0121  //hmi as service
#define ADM_UIM_HMISERVICE_CHECK_RESULT_METHOD_ID                       0x0122  //hmi as service 检测结果通知 cvm->uim->hmi->hut,响应原路返回
#define ADM_UIM_HMISERVICE_CONDS_DOWNLOAD_RESULT_METHOD_ID              0x0123  //hmi as service 带条件下载结果通知 dpm->uim->hmi->hut,响应原路返回
#define ADM_UIM_HMISERVICE_POLICY_DOWNLOAD_RESULT_METHOD_ID             0x0124  //hmi as service 下载总结果通知 dpm->uim->hmi->hut,响应原路返回
#define ADM_UIM_HMISERVICE_ECU_DOWNLOAD_RESULT_METHOD_ID                0x0125  //hmi as service 下载单ECU结果通知 dpm->uim->hmi->hut,响应原路返回
#define ADM_UIM_HMISERVICE_ALLECU_DOWNLOAD_RESULT_METHOD_ID             0x0126  //hmi as service 下载all ECU结果通知 dpm->uim->hmi->hut,响应原路返回
#define ADM_UIM_HMISERVICE_CONDS_INSTALL_RESULT_METHOD_ID               0x0127  //hmi as service 待条件安装总结果通知 ipm->uim->hmi->hut,响应原路返回
#define ADM_UIM_HMISERVICE_POLICY_INSTALL_RESULT_METHOD_ID              0x0128  //hmi as service 安装总结果通知 ipm->uim->hmi->hut,响应原路返回
#define ADM_UIM_HMISERVICE_ECU_INSTALL_RESULT_METHOD_ID                 0x0129  //hmi as service 安装单ECU结果通知 ipm->uim->hmi->hut,响应原路返回
#define ADM_UIM_HMISERVICE_ALLECU_INSTALL_RESULT_METHOD_ID              0x0130  //hmi as service 安装ALL ECU结果通知 ipm->uim->hmi->hut,响应原路返回
#define ADM_UIM_HMISERVICE_STAGE_INSTALL_RESULT_METHOD_ID               0x0131  //hmi as service 安装阶段性结果通知 ipm->uim->hmi->hut,响应原路返回
#define ADM_UIM_HMISERVICE_REPEAT_POLICY_INSTALL_RESULT_METHOD_ID       0x0132  //hmi as service 客户未点击确定，安装总结果重新通知 ipm->uim->hmi->hut,响应原路返回
#define ADM_UIM_HMISERVICE_DOWNLOAD_TASK_STOP_METHOD_ID                 0x0133  //hmi as service 下载完成后任务已失效通知

#define ADM_UIM_HMI_ENGINEER_MODE_VEHICLE_STATE_CTRL_METHOD_ID          0x0140  //< adm_sysi4hmi_emcVehicleStateCtrl
#define ADM_UIM_HMI_ENGINEER_MODE_OTA_SERVER_SWITCH_METHOD_ID           0x0141  //< adm_sysi4hmi_emcOTAServerSwitch
#define ADM_UIM_HMI_ENGINEER_MODE_OTA_SERVER_CONNECT_OK_METHOD_ID       0x0142  //< adm_sysi4hmi_emcOTAServerConnectOk
#define ADM_UIM_HMI_ENGINEER_MODE_VEHICLE_APP_CTRL_METHOD_ID            0x0143  //< adm_sysi4hmi_emcOTAVehicleAppCtrl

#define ADM_UIM_HMI_QUERY_ICP_INFO_METHOD_ID            0x0145  //< adm_sysi4hmi_queryIcpInfo 获取车辆ICP信息接口
// event group id
#define ADM_UIM_EVENT_GROUP_ID                              0x0001

// event id
#define ADM_UIM_CHECK_RESULT_NOTIFY_EVENT_ID                        0x0001  // adm_uim_soaCheckResultNotify
#define ADM_UIM_CHECK_PROGRESS_NOTIFY_EVENT_ID                      0x0004  // adm_uim_soaCheckProgressNotify
#define ADM_UIM_POLICY_DOWNLOAD_RESULT_NOTIFY_EVENT_ID              0x0011  // adm_uim_soaPolicyDownloadResultNotify
#define ADM_UIM_POLICY_DOWNLOAD_PROGRESS_NOTIFY_EVENT_ID            0x0012  // adm_uim_soaPolicyDownloadProgressNotify
#define ADM_UIM_ECU_DOWNLOAD_RESULT_NOTIFY_EVENT_ID                 0x0013  // adm_uim_soaEcuDownloadResultNotify
#define ADM_UIM_ECU_DOWNLOAD_PROGRESS_NOTIFY_EVENT_ID               0x0014  // adm_uim_soaEcuDownloadProgressNotify
#define ADM_UIM_ALL_ECU_DOWNLOAD_PROGRESS_NOTIFY_EVENT_ID           0x0018  // adm_uim_soaAllEcuDownloadProgressNotify
#define ADM_UIM_ALL_ECU_DOWNLOAD_RESULT_NOTIFY_EVENT_ID             0x0019  // adm_uim_soaAllEcuDownloadResultNotify
#define ADM_UIM_DOWNLOAD_CONDITIONS_VERIFY_RESULT_NOTIFY_EVENT_ID   0x0021  // adm_uim_soaDownloadConditionsVerifyResultNotify
#define ADM_UIM_POLICY_INSTALL_RESULT_NOTIFY_EVENT_ID               0x0101  // adm_uim_soaPolicyInstallResultNotify
#define ADM_UIM_POLICY_INSTALL_PROGRESS_NOTIFY_EVENT_ID             0x0102  // adm_uim_soaPolicyInstallProgressNotify
#define ADM_UIM_ECU_INSTALL_RESULT_NOTIFY_EVENT_ID                  0x0103  // adm_uim_soaEcuInstallResultNotify
#define ADM_UIM_ECU_INSTALL_PROGRESS_NOTIFY_EVENT_ID                0x0104  // adm_uim_soaEcuInstallProgressNotify
#define ADM_UIM_ALL_ECU_INSTALL_PROGRESS_NOTIFY_EVENT_ID            0x0108  // adm_uim_soaAllEcuInstallProgressNotify
#define ADM_UIM_ALL_ECU_INSTALL_RESULT_NOTIFY_EVENT_ID              0x0109  // adm_uim_soaAllEcuInstallResultNotify
#define ADM_UIM_INSTALL_CONDITIONS_VERIFY_RESULT_NOTIFY_EVENT_ID    0x0113  // adm_uim_soaInstallConditionsVerifyResultNotify
#define ADM_UIM_SCHDULE_TIME_ARRIVED_EVENT_ID                       0x0114  // adm_uim_soa_schduleTimeArrived
#define ADM_UIM_TSP_SCHDULE_TIME_EVENT_ID                           0x0115  // adm_uim_soa_tspSchduleTime

#define ADM_UIM_VSM_STATE_NOTIFY_EVENT_ID 0x0130  //uim一般性事务处理的methodID,130开始

/********************UIM end******************/

/********************VDI begin******************/

// instance id

// service id
#define ADM_VDI_SERVICE_ID                                  0x0300  //vdi service
// method id
#define ADM_VDI_READ_DIAGNOSTICINFO_METHOD_ID               0x0340
#define ADM_VDI_REFLASH_ECUS_TRIGGER_METHOD_ID              0x0341
#define ADM_VDI_REFLASH_ECU_PROGRESS_QUERY_METHOD_ID        0x0342
#define ADM_VDI_REFLASH_ECU_RESULT_QUERY_METHOD_ID          0x0343
#define ADM_VDI_PERFORM_SPECIFIC_DIAGNOSTIC_TASK_METHOD_ID  0x0344
#define ADM_VDI_READ_VEHICLES_STATUS_METHOD_ID              0x0345
#define ADM_VDI_SET_SPY_VEHICLE_STATUS_METHOD_ID            0x0346
#define ADM_VDI_CLEAR_SPY_VEHICLE_STATUS_METHOD_ID          0x0347
#define ADM_VDI_CONTROL_VEHICLE_STATUS_METHOD_ID            0x0348
#define ADM_VDI_SOFT_VERSION_CHECK_PROCESS_METHOD_ID        0x0349
#define ADM_VDI_GET_SDK_VERSION_METHOD_ID                   0x034A
#define ADM_VDI_SET_LOG_LEVEL_METHOD_ID                     0x034B
#define ADM_VDI_GET_SDKVERSION_METHOD_ID                    0x034C
#define ADM_VDI_PARALLEL_COLLECT_ECU_METHOD_ID              0x034D
#define ADM_VDI_PROGRAM_TRIGGER_METHOD_ID                   0x034E
#define ADM_VDI_READ_AB_INFO_METHOD_ID                      0x034F
#define ADM_VDI_SWITCH_AB_METHOD_ID                         0x0350
#define ADM_VDI_AB_REFLASH_ECUS_TRIGGER_METHOD_ID           0x0351
#define ADM_VDI_PROGRAM_REBOOT_METHOD_ID                    0x0352
#define ADM_VDI_SET_DIAG_TP_METHOD_ID                       0x0353
#define ADM_VDI_GET_VEHICLE_TYPE_METHOD_ID                  0x0354
#define ADM_VDI_NOTIFY_FLASH_STATE_METHOD_ID                0x0355
#define ADM_VDI_CLEAR_DTC_METHOD_ID                         0x0356

// event id
#define ADM_VDI_REFLASH_ECU_PROGRESS_NOTIFICATION_EVENT_ID  0x0370  // adm_sysi4vdi_postProgress
#define ADM_VDI_RELFASH_ECU_RESULT_NOTIFICATION_EVENT_ID    0x0371  // adm_sysi4vdi_postResult
#define ADM_VDI_SYNC_VEHICLE_STATUS_EVENT_ID                0x0372  // adm_sysi4vdi_postSpyVehicleStatus
#define ADM_VDI_SPY_VEHICLE_SIGNAL_TRIGGER_EVENT_ID         0x0373  // adm_sysi4vdi_postVehicleSignalTrigger
#define ADM_VDI_POST_DIAGNOSTIC_INFO_EVENT_ID               0x0374  // adm_sysi4vdi_readDiagnosticInfoNotify     
#define ADM_VDI_POST_PARALLEL_COLLECT_ECU_EVENT_ID          0x0375
/********************VDI end******************/


/********************HOST begin******************/
// instance id
// service id
#define ADM_HOST_SERVICE_ID                                 0x0500
// method id
#define ADM_HOST_GET_VIN_METHOD_ID                          0x0540  // adm_dim_soa_getVin
#define ADM_HOST_EXEC_OTATASK_METHOD_ID                     0x0541  // adm_dim_soa_host_execOtaTask
#define ADM_HOST_SET_SCHDULE_WAKEUPTIME_METHOD_ID           0x0542  // adm_dim_soa_host_setSchduleWakeUpTime
#define ADM_HOST_GET_SCHDULE_WAKEUPTIME_METHOD_ID           0x0543  // adm_dim_soa_host_setSchduleWakeUpTime
#define ADM_HOST_SET_VEHICLE_BIGVERSION_METHOD_ID           0x0544  // adm_dim_soa_host_setVehicleBigVersion
#define ADM_HOST_GET_VEHICLE_BIGVERSION_METHOD_ID           0x0545  // adm_dim_soa_host_getVehicleBigVersion
#define ADM_HOST_VERIFY_SPACEISENOUGH_METHOD_ID             0x0546  // adm_dim_soa_host_verifySpaceIsEnough
#define ADM_HOST_VERIFY_SIGNATURE_METHOD_ID                 0x0547  // adm_dim_soa_host_verifySignature
#define ADM_HOST_GET_CERTABSOLUTEPATH_METHOD_ID             0x0548  // adm_dim_soa_host_getCertAbsolutePath
#define ADM_HOST_SET_DTC_METHOD_ID                          0x0549  // adm_dim_soa_host_setDiagTroubleCode
#define ADM_HOST_CLEAR_DTC_METHOD_ID                        0x054A  // adm_dim_soa_host_clearDiagTroubleCode
#define ADM_HOST_READ_VEHICLES_STATUS_METHOD_ID             0x054B  // adm_dim_host_readVehicleStatus
#define ADM_HOST_SET_LOG_LEVEL_METHOD_ID                    0x054C
#define ADM_HOST_GET_SDKVERSION_METHOD_ID                   0x054D
#define ADM_HOST_GET_NETWORKTYPE_METHOD_ID                  0x054E
#define ADM_HOST_SET_VEHICLES_STATUS_METHOD_ID              0x054F  // adm_dim_host_setVehicleStatus
#define ADM_HOST_SET_WIFI_LINK_METHOD_ID                    0x0550  // adm_dim_host_setWifiLink
#define ADM_HOST_GET_ECU_INFO_METHOD_ID                     0x0551
#define ADM_HOST_SEND_AUTH_POWER_METHOD_ID                0x0552
#define ADM_HOST_GET_VEHICLE_VER_METHOD_ID                  0x0553
#define ADM_HOST_SET_VEHICLE_VER_METHOD_ID                  0x0554
#define ADM_HOST_SET_UPGRADE_MODE_METHOD_ID                 0x0555  // adm_dim_host_setUpgradeMode
#define ADM_HOST_REBOOT_METHOD_ID                           0x0556  // reboot
#define ADM_HOST_DOWNEXCEPTION_METHOD_ID                    0x0559  // 
#define ADM_HOST_GET_OTA_MODE_STATUS_METHOD_ID              0x055A  // 
#define ADM_HOST_SET_ICU_POWER_OFF_METHOD_ID                0x055B
#define ADM_HOST_NOTIFY_HMI_START_METHOD_ID                 0x055C
#define ADM_HOST_CONTROL_WAKEUP_MODE_METHOD_ID              0x055D  // adm_dim_host_setUpgradeMode
#define ADM_HOST_GET_TBOX_REMOTE_DIA_STATUS_METHOD_ID       0x055E  // adm_dim_host_getTboxRemoteDiaStatus

// #define ADM_HOST_GET_APK_VERSION_METHOD_ID                  0x0556   // adm_dim_host_getApkVersion
// #define ADM_HOST_DO_APK_SELF_UPGRADE_METHOD_ID              0x0557   // adm_dim_host_doApkSelfUpgrade

// event id
#define ADM_HOST_SIGNALS_EVENT_NOTIFY_EVENT_ID              0x0570  // adm_sysi4host_signalEventNotify
#define ADM_HOST_TSP_MESSAGE_PUSH_NOTIFY_EVENT_ID           0x0571  // adm_sysi4host_triggerEventNotify 
#define ADM_HOST_TSP_TASK_PUSH_METHOD_ID                    0x0572  // adm_sysi4host_tspTaskRequest
#define ADM_HOST_HMI_EVENT_REPORT_METHOD_ID                 0x0573  // hmi event report
#define ADM_HOST_GET_PRIVACYPATH_METHOD_ID             		0x0588  // adm_dim_soa_host_getPrivacyPathPath

/********************HOST end******************/

/********************UCL begin******************/
// instance id
// service id
#define ADM_UCL_ACTIVE_SERVICE_ID                               0x0400
// method id
#define ADM_UCL_READ_PART_INFORMATION_METHOD_ID             0x0441  // adm_dim_soa_ucm_readPartInformation
#define ADM_UCL_REQUEST_DO_UPGRADE_METHOD_ID                0x0442  // adm_dim_soa_ucm_requestDoUpgrade
#define ADM_UCL_QUERY_UPGRADE_PROGRESS_METHOD_ID            0x0443  // adm_dim_soa_ucm_queryUpgradeProgress
#define ADM_UCL_QUERY_UPGRADE_RESULT_METHOD_ID              0x0444  // adm_dim_soa_ucm_queryUpgradeResult
#define ADM_UCL_REQUEST_DECRYPT_PACKAGE_METHOD_ID           0x0445  // adm_dim_soa_ucm_requestDecryptPackage
#define ADM_UCL_GET_DECRYPT_PACKAGE_PROGRESS_METHOD_ID      0x0446  // adm_dim_soa_ucm_getDecryptPackageProgress
#define ADM_UCL_GET_DECRYPT_PACKAGE_RESULT_METHOD_ID        0x0447  // adm_dim_soa_ucm_getDecryptPackageResult
#define ADM_UCL_REQUEST_VERIFY_HASH_METHOD_ID               0x0448  // adm_dim_soa_ucm_requestVerifyHash
#define ADM_UCL_VERIFY_SPACE_IS_ENOUGH_METHOD_ID            0x0449  // adm_dim_soa_ucm_verifySpaceIsEnouth
#define ADM_UCL_GET_SDK_VERSION_METHOD_ID                   0x044A  // ucl_getSdkVersion
#define ADM_UCL_SET_LOG_LEVEL_METHOD_ID                     0x044B
#define ADM_UCL_REQUEST_VERIFY_SIGN_METHOD_ID               0x044C
#define ADM_UCL_GET_VEHICLES_STATUS_METHOD_ID               0x044D
#define ADM_UCL_SET_VEHICLES_STATUS_METHOD_ID               0x044E
#define ADM_UCL_REQUEST_DO_UPGRADE_ICC_METHOD_ID            0x044F  // adm_dim_soa_ucm_requestDoUpgrade_icc
#define ADM_UCL_REQUEST_SETCHANGESLOT_ICC_METHOD_ID         0x0450  // adm_dis_ucl_requestSetChangeSlot_icc_impl
#define ADM_UCL_REQUEST_UPDATEANDREBOOT_METHOD_ID       0x0451  // adm_dis_ucl_requestSetChangeSlot_icc_impl
#define ADM_UCL_PARALLEL_COLLECT_ECU_METHOD_ID              0x0452
#define ADM_UCL_READ_SN_INFORMATION_METHOD_ID               0x0453  // adm_dis_ucl_readSnInfo_impl
#define ADM_UCL_REQUEST_SYNC_DECRYPT_PACKAGE_METHOD_ID      0x0454
#define ADM_UCL_REQUEST_RM_FILE_METHOD_ID                   0x0455 

#define ADM_UCL_GET_APK_VERSION_METHOD_ID                   0x0460  // adm_dis_ucl_getApkVersion_impl
#define ADM_UCL_DO_APK_SELF_UPGRADE_METHOD_ID               0x0461  // adm_dis_ucl_doApkSelfUpgrade_impl
#define ADM_UCL_QUERY_APK_SELF_UPGRADE_PROGRESS_METHOD_ID   0x0462  // adm_dis_ucl_queryApkSelfUpgradeProgress_impl
#define ADM_UCL_QUERY_APK_SELF_UPGRADE_RESULT_METHOD_ID     0x0463  // adm_dis_ucl_queryApkSelfUpgradeResult_impl


//ucl event id
#define ADM_UCL_READ_PART_INFORMATION_NOTIFY_EVENT_ID   0x0470  // adm_sysi4ucm_readPartInfoNotify
#define ADM_UCL_UPGRADE_PROGRESS_NOTIFY_EVENT_ID        0x0471  // adm_sysi4ucm_upgradeProgressNotify
#define ADM_UCL_UPGRADE_RESULT_NOTIFY_EVENT_ID          0x0472  // adm_sysi4ucm_upgradeResultNotify
#define ADM_UCL_DECRYPT_PROGRESS_NOTIFY_EVENT_ID        0x0473  // adm_sysi4ucm_decryptProgressNotify
#define ADM_UCL_DECRYPT_RESULT_NOTIFY_EVENT_ID          0x0474  // adm_sysi4ucm_decryptResultNotify
#define ADM_UCL_VERIFY_HASH_PROGRESS_NOTIFY_EVENT_ID    0x0475  // adm_sysi4ucm_verifyHashProgressNotify
#define ADM_UCL_VERIFY_HASH_RESULT_NOTIFY_EVENT_ID      0x0476  // adm_sysi4ucm_verifyHashResultNotify
#define ADM_UCL_VERIFY_SIGN_PROGRESS_EVENT_ID           0x0479
#define ADM_UCL_VERIFY_SIGN_RESULT_EVENT_ID             0x047A
#define ADM_UCL_UPGRADE_ICC_PROGRESS_NOTIFY_EVENT_ID    0x047B  // adm_sysi4ucl_upgradeIccProgressNotify
#define ADM_UCL_UPGRADE_ICC_RESULT_NOTIFY_EVENT_ID      0x047C  // adm_sysi4ucl_upgradeIccResultNotify
#define ADM_UCL_POST_PARALLEL_COLLECT_ECU_EVENT_ID      0x047D

#define ADM_UCL_APK_SELF_UPGRADE_PROGRESS_NOTIFY_EVENT_ID   0x0480  // adm_sysi4ucl_apkSelfUpgradeProgressNotify
#define ADM_UCL_APK_SELF_UPGRADE_RESULT_NOTIFY_EVENT_ID     0x0490  // adm_sysi4ucl_apkSelfUpgradeResultNotify


// dpo service id
#define ADM_UCL_DPO_SERVICE_ID                                  0x0600
// method id
#define ADM_DPO_REQUEST_REMOTE_DOWNLOAD_PACKAGE_METHOD_ID   0x0640  // adm_dim_soa_dpo_requestRemoteDownloadPackage
#define ADM_DPO_GET_REMOTE_DOWNLOAD_PROGRESS_METHOD_ID      0x0641  // adm_dim_soa_dpo_getRemoteDownloadProgress
#define ADM_DPO_GET_REMOTE_DOWNLOAD_RESULT_METHOD_ID        0x0642  // adm_dim_soa_dpo_getRemoteDownloadResult
#define ADM_DPO_REQUEST_CLEAN_DOWNLOAD_DIR                  0x0643
#define ADM_DPO_SET_LOG_LEVEL_METHOD_ID                     0x0644
#define ADM_DPO_GET_SDKVERSION_METHOD_ID                    0x0645
#define ADM_DPO_REQUEST_INTERUPT_DOWNLOAD                   0x0646  // adm_dis_dpo_interuptDownload_impl
// event 
#define ADM_UCL_DPO_DOWNLOAD_PROGRESS_NOTIFY_EVENT_ID   0x0477  // adm_sysi4dpo_notifyDownloadProgress
#define ADM_UCL_DPO_DOWNLOAD_RESULT_NOTIFY_EVENT_ID     0x0478  // adm_sysi5dpo_NotifyDownloadResult

// trans service id
#define ADM_UCL_TRANSFER_SERVICE_ID    0x0700
// method id
#define ADM_UCL_TRANS_REQUEST_TRANSFER_DOWNLOAD_FROM_HOST_METHOD_ID         0x0740  // adm_dim_soa_trans_requestTransferDownloadFromHost
#define ADM_UCL_TRANS_QUERY_TRANSFER_DOWNLOAD_FROM_HOST_PROGRESS_METHOD_ID  0x0741  // adm_dim_soa_trans_queryTransferDownloadFromHostProgress
#define ADM_UCL_TRANS_QUERY_TRANSFER_DOWNLOAD_FROM_HOST_RESULT_METHOD_ID    0x0742  // adm_dim_soa_trans_queryTransferDownloadFromHostResult
#define ADM_UCL_TRANS_REQUEST_TRANSFER_UPLOAD_TO_HOST_METHOD_ID             0x0743  // adm_dim_soa_trans_requestTransferUploadToHost
#define ADM_UCL_TRANS_QUERY_TRANSFER_UPLOAD_TO_HOST_PROGRESS_METHOD_ID      0x0744  // adm_dim_soa_trans_queryTransferUploadToHostProgress
#define ADM_UCL_TRANS_QUERY_TRANSFER_UPLOAD_TO_HOST_RESULT_METHOD_ID        0x0745  // adm_dim_soa_trans_queryTransferUploadToHostResult
#define ADM_UCL_TRANS_SET_LOG_LEVEL_METHOD_ID                               0x0746
#define ADM_UCL_TRANS_GET_SDKVERSION_METHOD_ID                              0x0747
#define ADM_UCL_TRANS_REQUEST_TRANSFER_SYNC_DOWNLOAD_FROM_HOST_METHOD_ID    0x0748
// event id
#define ADM_UCL_TRANS_DOWNLOAD_FROM_HOST_PROGRESS_NOTIFY_EVENT_ID   0x0770  // adm_sysi4trans_downloadFromHostProgressNotify
#define ADM_UCL_TRANS_DOWNLOAD_FROM_HOST_RESULT_NOTIFY_EVENT_ID     0x0771  // adm_sysi4trans_downloadFromHostResultNotify
#define ADM_UCL_TRANS_UPLOAD_TO_HOST_PROGRESS_NOTIFY_EVENT_ID       0x0772  // adm_sysi4trans_uploadToHostProgressNotify
#define ADM_UCL_TRANS_UPLOAD_TO_HOST_RESULT_NOTIFY_EVENT_ID         0x0773  // adm_sysi4trans_uploadToHostResultNotify

#define ADM_UCL_TRANS_SWITCH_TRANS_MODE_METHOD_ID 0x0780 //

#endif // __ADM_DIM_SOA_CFG_H__
