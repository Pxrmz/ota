
#ifndef __ADM_SYSI4ICC_HOST_H__
#define __ADM_SYSI4ICC_HOST_H__

//ICC零件标识 reauestID
typedef enum
{
    ADM_CUSTOM_ECU_TYPE_SOCMCU = 1,     ///< ICC SOC + MCU
    ADM_CUSTOM_ECU_TYPE_SOC = 2,               ///< SOC
    ADM_CUSTOM_ECU_TYPE_MCU = 3,               ///< MCU
    ADM_CUSTOM_ECU_TYPE_CARDASHBOARD = 4,      ///< 仪表屏
    ADM_CUSTOM_ECU_TYPE_CARPAD = 5,            ///< 中控屏
    ADM_CUSTOM_ECU_TYPE_AC = 6,                ///< 空调屏
    ADM_CUSTOM_ECU_TYPE_CEILING = 7,           ///< 吸顶屏
    ADM_CUSTOM_ECU_TYPE_INVALID = 999          ///< 无效值
} ADM_CUSTOM_ECU_E;


//ICC零件属性
//零件号 正常返回ASCII码异常为NULL
#define ICC_ECU_DID_PARTNUM    0xF201
//零件软件版本号 正常返回ASCII码异常为NULL
#define ICC_ECU_DID_VERSION    0xF202
//零件硬件版本号 正常返回ASCII码异常为NULL
#define ICC_ECU_DID_HARDWARE    0xF203
//零件硬件编号 正常返回ASCII码异常为NULL
#define ICC_ECU_DID_SN    0xF204


#ifdef __cplusplus
extern "C" {
#endif

typedef D32S(*ADM_SYSI4HOST_CANRESPONSENOTIFY)(D32S canId, D8U *canMsg);


#ifdef __cplusplus
}
#endif





#endif  //__ADM_SYSI4ICC_HOST_H__

