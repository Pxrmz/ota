/**
 * @file adm_application_errorCode.h
 * @brief VUS项目中定义的错误码列表
 * @details 此错误码列表，所有模块皆可使用
 * @author Mengwei
 * @version v0.1.0
 * @date 2020-12-16
 * @copyright www.abupdate.com
 */
 #ifndef _ADM_APPLICATION_ERRORCODE_H_
 #define _ADM_APPLICATION_ERRORCODE_H_

#include "adm_com_errorCode.h"

#ifdef __cplusplus
extern "C" {
#endif

#define EXPORT_FUN __attribute__((visibility("default")))


/**
 * @brief app模块错误码定义
 * @note  暂定错误码，后续移植到错误码总表中
 * 
*/
#define ADM_ECODE_CYCLE_COLLECT_END             50100       /*!< 本次巡检结束 */
#define ADM_ECODE_CYCLE_COLLECT_INTERRUPT       50101      /*!< 周期采集条件不满足，中断周期采集 */
#define ADM_ECODE_CYCLE_COLLECT_FAILED          50102      /*!< 获取采集周期失败 */
#define ADM_ECODE_CYCLE_COLLECT_INVALID         50103      /*!< 获取的采集周期不合法 */
#define ADM_ECODE_UNZIP_FAILED                  50104      /*!< 解压文件失败 */
#define ADM_ECODE_TRANS_FILE_FAILED             50105      /*!< 传输文件失败 */
#define ADM_ECODE_USB_DIR_OPEN_FAILED           50106      /*!< U盘目录打开失败 */
#define ADM_ECODE_USB_PACKAGE_GET_FAILED        50107      /*!< U盘任务包获取失败 */
#define ADM_ECODE_USB_PACKAGE_DECRYPT_FAILED    50108      /*!< U盘任务包解密失败 */
#define ADM_ECODE_USB_PACKAGE_UNZIP_FAILED      50109      /*!< U盘任务包解压失败 */
#define ADM_ECODE_USB_PACKAGE_TYPE_CHECK_FAILED 50110      /*!< U盘任务包车型校验失败 */

//download
#define ADM_ECODE_DOWNLOAD_ERROR				50200	   /*!< 下载失败 */
#define ADM_ECODE_DOWNLOAD_SUCCESSED			50201      /*!< 下载成功 */
#define ADM_ECODE_DOWNLOAD_TASK_FAILED          50202      /*!< 任务有效性校验失败 */
#define ADM_ECODE_DOWNlOAD_CONDITION_FAILED		50203      /*!< 条件不满足 */
#define ADM_ECODE_DOWNLOAD_STORAGE_FAILED       50204      /*!< 存储空间不足 */
#define ADM_ECODE_DOWNLOAD_SIZE_FAILED          50205      /*!< 大小检验失败 */
#define ADM_ECODE_DOWNLOAD_HASH_FAILED          50206      /*!< hash校验失败 */
#define ADM_ECODE_DOWNLOAD_DECRYPT_FAILED       50207      /*!< 解密失败 */ 
#define ADM_ECODE_DOWNLOAD_AGAIN_SIZE_FAILED    50208      /*!< 解密后大小检验失败 */ 
#define ADM_ECODE_DOWNLOAD_AGAIN_HASH_FAILED    50209      /*!< 解密后hash校验失败 */ 
#define ADM_ECODE_DOWNLOAD_NO_PACKAGE_FAILED      50210   /*!< 安装包缺失(U盘) */ 
#define ADM_ECODE_DOWNLOAD_OVER_DOWNLOAD_TIME   50211      /*!< 产线限制下载时长是否已到 */ 

//install
#define ADM_ECODE_INSTALL_PREPARE_FAILED                50301   /*!< 安装前准备失败 */
#define ADM_ECODE_INSTALL_TASK_VERIFY_FAILED            50302   /*!< 任务有效性校验任务无效失败 */
#define ADM_ECODE_INSTALL_CONDITION_FAILED		        50303   /*!< 安装前置条件不满足 */
#define ADM_ECODE_INSTALL_KEEPNET_CONDITION_FAILED      50304   /*!< 安装前置条件校验网络维持失败 */
#define ADM_ECODE_INSTALL_KEEPNET_FAILED                50305   /*!< 安装网络维持失败 */
#define ADM_ECODE_INSTALL_ENTER_OTAMODE_FAILED          50306   /*!< 进入OTA模式失败 */
#define ADM_ECODE_INSTALL_SIGN_VERIFY_FAILED            50307   /*!< 升级包验签失败 */
#define ADM_ECODE_INSTALL_VERSION_VERIFY_FAILED         50308   /*!< 升级前零件号和版本号校验失败 */
#define ADM_ECODE_INSTALL_FAILED                        50309   /*!< 安装失败 */
#define ADM_ECODE_INSTALL_SUCCESSED                     50310   /*!< 安装成功 */
#define ADM_ECODE_INSTALL_ROLLBACK_SUCCESSED            50311   /*!< 已升级零件回滚成功 */
#define ADM_ECODE_INSTALL_ECU_FLASH_FAILED              50312   /*!< 零件刷写失败 */
#define ADM_ECODE_INSTALL_CRITICAL_ECU_FLASH_FAILED     50313   /*!< 重要零件刷写失败 */
#define ADM_ECODE_INSTALL_EXIT_OTAMODE_FAILED           50314   /*!< 退出OTA模式失败 */
#define ADM_ECODE_INSTALL_ENTER_HIGH_VOLTAGE_FAILED     50315   /*!< 上高压失败 */
#define ADM_ECODE_INSTALL_EXIT_HIGH_VOLTAGE_FAILED      50316   /*!< 下高压失败 */
#define ADM_ECODE_INSTALL_AFTERINSTALL_VERSION_VERIFY_FAILED         50317   /*!< 升级后零件号和版本号校验失败 */
#define ADM_ECODE_INSTALL_TASK_VERIFY_NET_FAILED        50318   /*!< 任务有效性校验网络通讯失败 */
#define ADM_ECODE_INSTALL_ENTER_BOOTMODE_FAILED         50319   /*!< ECU进入boot模式失败 */
#define ADM_ECODE_INSTALL_GET_ECU_VERSION_FAILED        50320   /*!< 升级前DPC获取ECU版本失败 */
#define ADM_ECODE_INSTALL_PRE_PROGRAM_FAILED            50321   /*!< 升级预编程失败 */
#define ADM_ECODE_INSTALL_ROLLBACK_FAILED               50322   /*!< 零件回滚失败 */
#define ADM_ECODE_INSTALL_REMOTE_POWER_ON_FAILED        50323   /*!< 远程上电失败 */
#define ADM_ECODE_INSTALL_REMOTE_POWER_OFF_FAILED       50324   /*!< 远程下电失败 */
#define ADM_ECODE_INSTALL_VEHICLE_IS_POWER_ON           50325   /*!< 车辆上电中 */


#define HMI_INSTALL_SCHEDULED	                        50358   /*!< HMI设置预约升级成功 */
#define APP_INSTALL_SCHEDULED	                        50359   /*!< APP设置预约升级成功 */
#define HMI_INSTALL_SCHEDULED_FAILED	                50360   /*!< HMI设置预约升级失败 */
#define APP_INSTALL_SCHEDULED_FAILED	                50361   /*!< APP设置预约升级失败 */
#define HMI_INSTALL_CANCELED_SCHEDULED	                50362   /*!< HMI取消预约升级成功 */
#define APP_INSTALL_CANCELED_SCHEDULED	                50363   /*!< APP取消预约升级成功 */
#define HMI_INSTALL_CANCELED_SCHEDULED_FAILED	        50364   /*!< HMI取消预约升级失败 */
#define APP_INSTALL_CANCELED_SCHEDULED_FAILED	        50365   /*!< APP取消预约升级失败 */

#define ADM_ECODE_INSTALL_FAILED_MODE1                  50326   /** 前置条件校验失败，下电成功，退出OTA模式失败 */
#define ADM_ECODE_INSTALL_FAILED_MODE2                  50327   /** 前置条件校验失败，下电成功，退出OTA模式成功 */
#define ADM_ECODE_INSTALL_FAILED_MODE3                  50338   /** 前置条件校验失败，下电失败，退出OTA模式失败 */
#define ADM_ECODE_INSTALL_FAILED_MODE4                  50339   /** 前置条件校验失败，下电失败，退出OTA模式成功 */
#define ADM_ECODE_INSTALL_FAILED_MODE5                  50342   /** 安装失败回滚成功，下电失败，退出OTA模式失败 */
#define ADM_ECODE_INSTALL_FAILED_MODE6                  50343   /** 安装失败回滚成功，下电失败，退出OTA模式成功 */
#define ADM_ECODE_INSTALL_FAILED_MODE7                  50333   /** 安装失败回滚成功，下电成功，退出OTA模式失败 */
#define ADM_ECODE_INSTALL_FAILED_MODE8                  50334   /** 安装失败回滚成功，下电成功，退出OTA模式成功 */
#define ADM_ECODE_INSTALL_FAILED_MODE9                  50340   /** 安装成功，下电失败，退出OTA模式失败 */
#define ADM_ECODE_INSTALL_FAILED_MODE10                 50341   /** 安装成功，下电失败，退出OTA模式成功 */
#define ADM_ECODE_INSTALL_FAILED_MODE11                 50330   /** 安装成功，下电成功，退出OTA模式失败 */
#define ADM_ECODE_INSTALL_GENERAL_FAILED                50331   /** 一般性安装失败场景 */

#define ADM_ECODE_SCHEDULE_CHARGE_CONFLACT		 50335		/** 预约安装与预约充电冲突 */
#define ADM_ECODE_SCHEDULE_TRAVEL_CONFLACT		 50336		/** 预约安装与预约出行冲突 */
#define ADM_ECODE_SCHEDULE_FAILED		         50337	    /** 预约失败 */

#define ADM_ECODE_INSTALL_IN_SENTINEL_SCENARIO          50350   /** 车辆当前处于哨兵模式智能场景 */
#define ADM_ECODE_INSTALL_IN_CHILDPET_SCENARIO          50351   /** 车辆当前处于童锁模式智能场景 */
#define ADM_ECODE_INSTALL_VEHICLE_IS_POWER_OFF          50352   /*!< 车辆下电中 */
#define ADM_ECODE_INSTALL_RESUME_FAILED                 50353   /*!< 安装中断电恢复失败 */

//check
#define ADM_ECODE_CHECK_HMI_LATEST_VERSION                      50401   /*!< 检测已是最新版本 */
#define ADM_ECODE_CHECK_HMI_NEW_VERSION_IMMEDIATELY_UPGRADED    50402   /*!< 检测发现新版本，请立即升级 */

#define ADM_ECODE_APPLICATION_CONVERSION(iRet, ecode)\
    do{\
        if((iRet) != ADM_ECODE_SUCCESSED)\
        {\
            (iRet) = (ecode);\
        }\
    }while (FALSE);


#ifdef __cplusplus  // close out "C" linkage in case of c++ compiling
}
#endif
 #endif//_ADM_APPLICATION_ERRORCODE_H_
