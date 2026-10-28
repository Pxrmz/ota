/**
 * @file adm_dim_common.h
 * @brief
 * @details
 * @author
 * @version
 * @date 2022-02-09
 * @copyright Copyright © 2018 All rights 上海艾拉比智能科技有限公司
 */
#ifndef __ADM_DIM_COMMON_H__
#define __ADM_DIM_COMMON_H__

//为区分c,c11
#ifdef __cplusplus
#define POINTER_NULL(ptr)    (nullptr == (ptr))
#define POINTER_NOT_NULL(ptr)    (nullptr != (ptr))
#else
#define POINTER_NULL(ptr)    (NULL == (ptr))
#define POINTER_NOT_NULL(ptr)    (NULL != (ptr))
#endif


/**
 * @enum VEHICLE_CTRL_ENUM
 * @brief 车辆的控制
 *
 */
typedef enum
{
    VEHICLE_CTRL_OTAUPGRADE_NOW = 0,       ///< 设置当前升级模式为立即升级
    VEHICLE_CTRL_OTAUPGRADE_SCHEDULED = 1, ///< 设置当前升级模式为预约升级
    VEHICLE_CTRL_OTAUPGRADE_SLIENT = 2,    ///< 设置当前升级模式为静默升级
    VEHICLE_CTRL_OTAUPGRADE_PRODUCT = 3,       ///< 设置当前升级模式为产线升级
    VEHICLE_CTRL_OTAUPGRADE_RESCUE = 4,       ///< 设置当前升级模式为救援升级
    VEHICLE_CTRL_OTAUPGRADE_USB = 5,       ///< 设置当前升级模式为USB升级
    VEHICLE_CTRL_OTAUPGRADE_INVALID
} VEHICLE_CTRL_OTAUPGRADE_MODE_E;

typedef enum VEHICLE_CTRL_ENUM
{
    VEHICLE_CTRL_BEGIN = 0x100,        ///< 车辆控制枚举开始
    VEHICLE_CTRL_ENTER_HV_MODE = 0x101,         ///< The vehicle enters high voltage mode
    VEHICLE_CTRL_EXIT_HV_MODE = 0x102,          ///< The vehicle exits high voltage mode
    VEHICLE_CTRL_ENTER_NORMAL_OTA_MODE = 0x103, ///< The vehicle enters the normal installation OTA mode
    VEHICLE_CTRL_EXIT_NORMAL_OTA_MODE = 0x104,  ///< The vehicle exits the normal installation OTA mode
    VEHICLE_CTRL_ENTER_SLIENT_OTA_MODE = 0x105, ///< The vehicle enters the slient installation OTA mode
    VEHICLE_CTRL_EXIT_SLIENT_OTA_MODE = 0x106,  ///< The vehicle exits the slient installation OTA mode
    VEHICLE_CTRL_START_SMART_CHARGING = 0x107,  ///< The vehicle turns on smart recharge
    VEHICLE_CTRL_STOP_SMART_CHARGING = 0x108,   ///< The vehicle turns off smart recharge
    VEHICLE_CTRL_ENTER_DIAG_MODE = 0x109,       ///< The vehicle enters diagnostic mode
    VEHICLE_CTRL_EXIT_DIAG_MODE = 0x10A,        ///< The vehicle exits diagnostic mode
    VEHICLE_CTRL_ENTER_STANDBY_MODE = 0x10B,    ///< The vehicle enters the standby state
    VEHICLE_CTRL_EXIT_STANDBY_MODE = 0x10C,     ///< The vehicle exits the standby state
    VEHICLE_CTRL_START_ENGINE = 0x10D,          ///< The vehicle starts engine
    VEHICLE_CTRL_STOP_ENGINE = 0x10E,           ///< The vehicle stops engine
    VEHICLE_CTRL_ENTER_AWAKE = 0x10F,           ///< The vehicle enters the awake state
    VEHICLE_CTRL_ENTER_SLEEP = 0x110,           ///< The vehicle enters the sleep state
    VEHICLE_CTRL_FUNC_SAFE_VEFIFY = 0x111,      ///< Functional safety verification
    VEHICLE_CTRL_HOST_PRE_PROGRAMMING = 0x112,  ///< The host device enters the pre-programming process
    /** 自定义枚举值从这里开始 */

    VEHICLE_CTRL_ON_CONTROL_SCREEN,         ///< 设置中控屏和仪表亮屏
    VEHICLE_CTRL_OFF_CONTROL_SCREEN,        ///< 设置中控屏和仪表息屏
    VEHICLE_CTRL_ENTER_OVERSCREEN,          ///< 进入霸屏模式
    VEHICLE_CTRL_EXIT_OVERSCREEN,           ///< 退出霸屏模式
    VEHICLE_CTRL_SET_WIFI_DISLINK,          ///< 控制车辆断开wifi
    VEHICLE_CTRL_OTAUPGRADE_MODE,           ///< 设置OTA升级模式,见 VEHICLE_CTRL_OTAUPGRADE_MODE_E
    VEHICLE_CTRL_CHANGE_SLOT,               ///< 切换分区
    VEHICLE_CTRL_UPGEADE_REBOOT,            ///< 升级后重启分区
    VEHICLE_CTRL_NO_ACTIVE_HV_MODE,         ///< OTA 高压未激活状态
    /** 自定义枚举底部 */
    VEHICLE_CTRL_END = 0x1FE,            ///< 车辆控制枚举结束
    VEHICLE_CTRL_INVALID = 0x1FF ///< Invalid
} VEHICLE_CTRL_E;

/**
 * @enum VEHICLE_EVENT_ENUM
 * @brief 车辆的触发事件
 *  *
 */
typedef enum VEHICLE_EVENT_ENUM
{
    VEHICLE_EVENT_BEGIN = 0x200,          ///< 车辆触发事件枚举开始
    VEHICLE_EVENT_SMART_CHARGING_FINISHED = 0x201, ///< Intelligent battery smart charging completed
    VEHICLE_EVENT_CRASH_OUT = 0x202,               ///< Vehicle is in crash out
    VEHICLE_EVENT_IN_E_MODE = 0x203,               ///< The vehicle is in emergency mode
    VEHICLE_EVENT_OBD_TEST_IN = 0x204,             ///< OBD port has diagnostic instrument intervention
    VEHICLE_EVENT_POWER_ON = 0x205,                ///< Power on the vehicle
    VEHICLE_EVENT_POWER_OFF = 0x206,               ///< Power off the vehicle
    VEHICLE_EVENT_IGN_ON = 0x207,                  ///< Vehicle ignition
    VEHICLE_EVENT_IGN_OFF = 0x208,                 ///< Vehicle flameout
    VEHICLE_EVENT_EXIT_BROKEN_MODE = 0x209,        ///< Was asked to exit Broken mode
    /** 自定义枚举值从这里开始 */

    /** 自定义枚举底部 */
    VEHICLE_EVENT_END = 0x2FE,             ///< 车辆触发事件枚举结束
    VEHICLE_EVENT_INVALID = 0x2FF, ///< Invalid

} VEHICLE_EVENT_E;

/**
 * @enum VEHICLE_STATUS_ENUM
 * @brief 车辆状态
 *
 */
typedef enum VEHICLE_STATUS_ENUM
{
    VEHICLE_STATUS_BEGIN = 0x300, ///< 车辆状态枚举开始
    //VEHICLE_STATUS_DRIVE_MODE = 0x301,     ///< 0内燃机车，1纯电动车，2燃料电池车，3混合动力车
    VEHICLE_STATUS_VIN = 0x301,     ///< VIN码
    VEHICLE_STATUS_VSPD = 0x302,           ///< 车速，单位km/h .
    VEHICLE_STATUS_ENGINE_SPD = 0x303,     ///< 发动机转速，单位rpm
    VEHICLE_STATUS_SOC = 0x304,            ///< 蓄电池剩余电量(小电池0-100)
    VEHICLE_STATUS_POWER_SOC = 0x305,      ///< 动力电池剩余电量
    VEHICLE_STATUS_IGN_STATUS = 0x306,     ///< 车辆钥匙状态，0为OFF档，1为ACC档，2为ON档，3为START档。
    VEHICLE_STATUS_GEAR = 0x307,           ///< 车辆挡位，0为P档，1为非P挡。
    VEHICLE_STATUS_EPBLOCK = 0x308,        ///< 车辆手刹状态，0为未拉起手刹，1为拉起手刹
    VEHICLE_STATUS_CHARGING = 0x309,       ///<  车辆充电状态，0正在充电，1未充电
    VEHICLE_STATUS_VOL_MV = 0x30A,         ///< 车辆蓄电池电压值，单位mv
    /** 自定义枚举值从这里开始 */

    VEHICLE_STATUS_ENGINE_STATE = 0x30B,    ///< 车辆发动机状态,1为引擎停止,2为引擎运行中
    VEHICLE_STATUS_SENTINEL_MODE = 0x30C,      ///< 车辆是否哨兵模式 0哨兵 1为非哨兵
    VEHICLE_STATUS_CHILDPET_MODE = 0x30D,              ///< 车辆是否童宠模式 0童宠 1为非童宠
    VEHICLE_STATUS_OBD_STATE = 0x30E,          ///< 车辆OBD接口状态1OBD接口已接入0为OBD接口未接口
    VEHICLE_STATUS_LOCK = 0x30F,               ///< 车辆锁车状态0为锁车状态1为非锁车状态
    VEHICLE_STATUS_DISCHARGE = 0x310,          ///< 车辆放电状态0为未放电1为放电状态
    VEHICLE_STATUS_READY = 0x311,             ///< 车辆ready状态/动力就绪状态 1为ready 0为未ready
    VEHICLE_STATUS_POWER_BATTERY_SUP = 0x312, ///< 动力电池供电状态0为已供电1为未供电
    VEHICLE_STATUS_ECALL = 0x313,            ///< E-CALL状态0为打开状态1为未打开状态
    VEHICLE_STATUS_FORTIFY = 0x314,                 ///< 车辆解防设防状态,0为解防状态,1为设防,-1为异常
    VEHICLE_STATUS_HIGH_VOLTAGE,       ///< 车辆是否高压状态1高压0非高压
    VEHICLE_STATUS_ETHERENT_TYPE,      ///< 车辆车内网络类型1以太网0非以太网
    VEHICLE_STATUS_TBOXNETWORK_TYPE,   ///< 车辆车外(TBOX)网络类型0为wifi1为移到网络
    VEHICLE_STATUS_COUNTRY_CODE,       ///< 车辆国家码
    VEHICLE_STATUS_MODE_TYPE,          ///< 整车模式类型0工厂模式1运输模式2整车模式
    VEHICLE_STATUS_APN_NAME,         ///< 获取APN网卡名
    VEHICLE_STATUS_SLOT,             ///< 获取分区标志位
    VEHICLE_STATUS_POWER_SIGNAL,    ///< 车辆电源状态,0为Comfortable状态,1为ON状态,2为OFF状态
    VEHICLE_STATUS_ESPPARK_STATE,      ///< 车辆ESP驻车状态1驻车状态0为非驻车状态
    VEHICLE_STATUS_THEME = 0x330,             ///< 座舱主题，取值1~99
    VEHICLE_STATUS_ANTI_THEFT_STATUS,             ///< 整车设防/解防状态。-1：获取整车设防/解防状态异常；0：解防状态；1：设防状态
    VEHICLE_STATUS_POWER_TYPE,             ///< 动力类型。0：无效；1：燃油；2：混动；3：增程；4：纯电
    VEHICLE_STATUS_SOC_OF_TRACTION_BAT,             ///< 动力电池电量（续航公里数）。和仪表显示保持一致，如100表示当前剩余续航100km
    VEHICLE_STATUS_SOC_OF_STORAGE_BAT,             ///< 蓄电池电量（毫安时）。如1500表示还剩余1500毫安时
    VEHICLE_STATUS_COUNT_OF_SCREEN,             ///< 屏幕数量。取值1~7（仪表，主驾娱乐，扶手，副驾娱乐，HUD，后座L，后座R）
    VEHICLE_STATUS_SCREEN_OF_OTA,             ///< OTA对应屏幕。取值1~7（仪表，主驾娱乐，扶手，副驾娱乐，HUD，后座L，后座R）
    VEHICLE_STATUS_NETWORK_TYPE,             ///< 网络类型。0：无效；1：4G/5G Only；2：Wi-Fi Only；3：Both 4G/5G and Wi-Fi
    VEHICLE_STATUS_BKLIGHT_MODE,             ///< 背光模式。0：无效；1：白天模式；2：黑夜模式
    VEHICLE_STATUS_TIMEZONE,             ///< 时区
    VEHICLE_STATUS_ARCHITECTURE_MODE,             ///< 整车架构类型。0：非以太网；1：以太网 
    VEHICLE_OPEN_DOOR_STATUS,               ///< 车辆开门状态。0:开门 1:关门 -1:异常
    VEHICLE_HIGH_VOLTAGE_STATUS,            ///< 高压就绪状态。1高压 0非高压 -1:异常
    VEHICLE_ENGINE_SPEED,                   ///< 发动机转速。 单位：rpm
    VEHICLE_PWR_OFF_SW_STS = 0x343,         ///< 车辆保电状态 0为关闭状态 1为开启状态

    VEHICLE_STATUS_TRAVEL_SCHEDULE_AVAIL = 0x401,      ///< 车辆是否处于出行预约-TravelScheduleAvail
    VEHICLE_STATUS_TRAVEL_SCHEDULE_START = 0x402,      ///< 车辆出行预约开始时间-TravelScheduleStart
    VEHICLE_STATUS_TRAVEL_SCHEDULE_STOP = 0x403,      ///< 车辆出行预约结束时间-TravelScheduleStop
    VEHICLE_STATUS_CHARGE_SCHEDULE_AVAIL = 0x404,      ///< 车辆是否处于充电预约-ChargeScheduleAvail
    VEHICLE_STATUS_CHARGE_SCHEDULE_START = 0x405,      ///< 车辆充电预约开始时间-ChargeScheduleStart
    VEHICLE_STATUS_CHARGE_SCHEDULE_STOP = 0x406,      ///< 车辆充电预约结束时间-ChargeScheduleStop
    VEHICLE_STATUS_PRESENTATION_MODE = 0x407,      ///< 车辆是否处于展厅模式-PresentationMode
    VEHICLE_STATUS_FACTORY_MODE = 0x408,      ///< 车辆是否处于工厂模式-FactoryMode
    VEHICLE_STATUS_TRANSPORT_MODE = 0x409,      ///< 车辆是否处于运输模式-TransportMode    
    VEHICLE_STATUS_NORMAL_MODE = 0x40A,      ///< 车辆是否处于用户模式-NormalMode

    VEHICLE_STATUS_BIG_VERSION = 0x601,      ///< 整车大版本-BigVersion
    VEHICLE_STATUS_DISPLAY_VERSION = 0x602,      ///< 车显示大版本-DisPlayVersion
    /** 自定义枚举底部 */
    VEHICLE_STATUS_END = 0x6FE, ///< 车辆状态枚举结束
    VEHICLE_STATUS_INVALID = 0x6FF
} VEHICLE_STATUS_E;

/**
 * @enum VEHICLE_POWER_ENUM
 * @brief 车辆状态
 *
 */
typedef enum VEHICLE_POWER_ENUM
{
    POWER_ABNORMAL = -1,
    POWER_COMFORTABLE = 0,
    OPWER_ON = 1,
    OPWER_OFF = 2
} VEHICLE_POWER_STATE_E;

/**
 * @enum VEHICLE_VOLTAGE_ENUM
 * @brief 车辆电压枚举
 *
 */
typedef enum VEHICLE_VOLTAGE_ENUM
{
    HV_ABNORMAL = -1,
    HV_HIGH = 1,
    HV_NO_HIGH = 0
} VEHICLE_VOLTAGE_STATE_E;


#ifdef __cplusplus
extern "C" {
#endif

#define EXPORT_FUN __attribute__((visibility("default")))

#include "adm_typedefs.h"
#include "adm_com_public_data.h"

#include "elog.h"
#define CHECK_POINTER_RETURN_VOID(pointer)    \
    do {    \
        if (POINTER_NULL(pointer))   \
        {     \
            log_e("%s is NULL", #pointer);    \
            return;    \
        }    \
    } while(TRUE == FALSE)

#define CHECK_POINTER_RETURN_RET(pointer, ret)    \
    do {    \
        if(POINTER_NULL(pointer))    \
        {    \
            log_e("%s is NULL", #pointer);    \
            return (ret);     \
        }    \
    } while(TRUE == FALSE)

#define CHECK_POINTER_BRK(pointer)    \
    if(POINTER_NULL(pointer))    \
    {    \
        log_e("%s is NULL", #pointer);    \
        break;     \
    }
#define CHECK_POINTER_CONTINUE(pointer)    \
    if(POINTER_NULL(pointer))    \
    {    \
        log_e("%s is NULL", #pointer);    \
        continue;     \
    }


#define SET_POINTER_TO_STRING(fun, pointer)    \
    do {    \
        if(POINTER_NOT_NULL(pointer))   \
        {   \
            (fun)((pointer));   \
        }   \
    } while(TRUE == FALSE)

#define CHECK_CONDITION_VOID(condition)    \
    do{    \
        if(condition)    \
        {    \
            log_e("%s", #condition);    \
            return;    \
        }    \
    }while(TRUE == FALSE)
#define CHECK_CONDITION_RET(condition, ret)    \
    do{    \
        if(condition)    \
        {    \
            log_e("%s", #condition);    \
            return (ret);    \
        }    \
    }while(TRUE == FALSE)


//JSON FORMAT
#define SET_POINTER_TO_JSON_OBJ_ADD(jsonObj, key, pointer)    \
    if(POINTER_NOT_NULL(pointer))    \
    {    \
        (void)json_object_object_add((jsonObj), (key), json_object_new_string((pointer)));    \
    }    \
    else    \
    {    \
        log_e("%s is NULL", #pointer);    \
    }


/**
 * @brief General data structure definition.
 */
typedef struct
{
    D16U dataType; /**!< data type */
    D16U dataLength; /**!< data length */
    union
    {
        /**!< Union of DID corresponding values */
        D8U value[1];
        D32U valueU32;
        D32S valueS32;
        D64U valueU64;
        D64S valueS64;
        float valueF;
        D8U* valueS;
    } v;
} ADM_DIM_SOA_DATA_T;

/**
 * @brief The obtained information structure corresponding to DID and DID.
 */
typedef struct
{
    D32S did; /**!< Data ID */
    D16U startByte;  ///< Starting byte position
    D16U endByte;  ///< Ending byte positoin
    D32U errorCode;  ///< errorCode : success => 0 else errorCode
    ADM_DIM_SOA_DATA_T* didData; /**!< Information corresponding to the DID obtained through the diagnostic command*/
} ADM_DIM_SOA_DID_DATA_T;

/**
 * @brief DID Array structure.
 */
typedef struct
{
    ADM_DIM_SOA_DID_DATA_T** didArray; /**!< Array of DID and DID values */
    D16U didArrayLength; /**!< Array length of DID and DID values */
} ADM_DIM_SOA_DID_DATA_ARRAY_T;

/**
 * @brief Structure definition used to uniquely identify an ECU.
 *
 */
typedef struct ADM_DIM_SOA_ECU_IDEN_INFO_ST
{
    D32S requestId; /**!< Request ID */
    D32S responseId; /**!< Response ID */
    D32S functionId; /**!< Function ID */
    D32S swDid; /**!< The did of the software version, used to distinguish different upgrade parts of the same requestId
                   ECU */
} ADM_DIM_SOA_ECU_IDEN_INFO_T;

/****************************** dim msgid list below ***************************/
typedef struct ADM_DIM_SOA_MSGID_ST
{
    D32S msgId;                                /**!< MsgId of vehicle state */
    ADM_DIM_SOA_DATA_T* msgIdData;             /**!< Vehicle status value obtained through msgId */
} ADM_DIM_SOA_MSGID_T;

/**
 * @brief dim MsgId Array structure.
*/
typedef struct
{
    ADM_DIM_SOA_MSGID_T** msgIdArray;             /**!< Array of masId and vehicle state */
    D16U msgIdArrayLength;                        /**!< Array length of masId and vehicle state */
} ADM_DIM_SOA_MSGID_DATA_ARRAY_T;
/******************************* dim msgid list above *************************/


/**
 * @brief the mask of Security Access for different level
 */
typedef struct
{
    D8U level; ///< the level of Security Access, eg: 27 01, level=0x01
    DCHAR *pMask; ///< the mask of Security Access, eg: 27 01, pMask="10082E87FA5F11731578EEE47D80C663"
} ADM_DIM_SOA_SA_DATA_T;


/**
 * @brief the mask info for Security Access
 */
typedef struct
{
    D8U num; ///< the number of ADM_DIM_SOA_SA_DATA_T
    ADM_DIM_SOA_SA_DATA_T *pSaData; ///< the mask info for Security Access
} ADM_DIM_SOA_SA_MASK_INFO_T;

typedef enum
{
    DIM_VEHICLE_E01,
    DIM_VEHICLE_E02,
    DIM_VEHICLE_E03,
    DIM_VEHICLE_T18,
    DIM_VEHICLE_T28,
    DIM_VEHICLE_T19C1,
    DIM_VEHICLE_T19C2,
    DIM_VEHICLE_T19C3,
    DIM_VEHICLE_M1E,
    DIM_VEHICLE_EXTERN1,
    DIM_VEHICLE_EXTERN2,
    DIM_VEHICLE_EXTERN3,
    DIM_VEHICLE_EXTERN4,
    DIM_VEHICLE_EXTERN5,
    DIM_VEHICLE_EXTERN6,
    DIM_VEHICLE_EXTERN7,
    DIM_VEHICLE_EXTERN8,
    DIM_VEHICLE_EXTERN9,
    DIM_VEHICLE_EXTERN10,
    DIM_VEHICLE_EXTERN11,
    DIM_VEHICLE_EXTERN12,
    DIM_VEHICLE_EXTERN13,
    DIM_VEHICLE_EXTERN14,
    DIM_VEHICLE_EXTERN15,
    DIM_VEHICLE_INVALID
} ADM_DIM_SOA_VEHICLE_TYPE_E;

#ifdef __cplusplus  // close out "C" linkage in case of c++ compiling
}
#endif

#endif  //__ADM_DIM_COMMON_H__
