/**
 * @author mengwei@abupdate.com
 * @brief 定义车云之间协议交互的接口，包括车端请求云端和云端响应车端的接口。
 * @version 0.1
 * @date 2021-05-13
 * 
 * @copyright Copyright © 2018 All rights 上海艾拉比智能科技有限公司
 * 
 */
#ifndef __ADM_OCA_CONNECT_H__
#define __ADM_OCA_CONNECT_H__

#ifdef __cplusplus
extern "C" {
#endif

#define EXPORT_FUN __attribute__((visibility("default")))

#include "adm_typedefs.h"

typedef enum
{
    COUNTRY_CN,         ///<中国
    COUNTRY_CL,         ///<智利
    COUNTRY_ID,         ///<印尼
    COUNTRY_MY,         ///<马来西亚
    COUNTRY_BR,         ///<巴西
    COUNTRY_CA,         ///<中亚
    COUNTRY_GCC,        ///< 海湾
    COUNTRY_KM,         ///<大俄
    COUNTRY_EG,         ///<埃及
    COUNTRY_IT,         ///<意大利
    COUNTRY_UA,         ///<乌克兰
    COUNTRY_EC,         ///<厄瓜多尔
    COUNTRY_MX,         ///<墨西哥
    COUNTRY_PH,         ///<菲律宾
    COUNTRY_SA,         ///<沙特
    COUNTRY_TH,         ///<泰国
    COUNTRY_KZ,         ///<哈萨克斯坦
    COUNTRY_AU,         ///<澳大利亚
    COUNTRY_TR,         ///<土耳其
    COUNTRY_IL,         ///<以色列
    COUNTRY_AE = 21,    ///<阿联酋
    COUNTRY_UK,         ///<英国
    COUNTRY_IQ,         ///<伊拉克
    COUNTRY_AR,         ///<阿根廷
    COUNTRY_ZA,         ///<南非
    COUNTRY_DZ,         ///<阿尔及利亚
    COUNTRY_UY,         ///<乌拉圭
    COUNTRY_CO,         ///<哥伦比亚
    COUNTRY_PE,         ///<秘鲁
    COUNTRY_AZ,         ///<阿塞拜疆
    COUNTRY_SN,         ///<塞内加尔
    COUNTRY_LY,         ///<利比亚
    COUNTRY_NG,         ///<尼日利亚
    COUNTRY_SY,         ///<叙利亚
    COUNTRY_BO,         ///<玻利维亚
    COUNTRY_NO,         ///<挪威
    COUNTRY_US,         ///<美国
    COUNTRY_PK,         ///<巴基斯坦
    COUNTRY_SD,         ///<苏丹
    COUNTRY_IN,         ///<印度
    COUNTRY_NZ,         ///<新西兰
    COUNTRY_MM,         ///<缅甸
    COUNTRY_RS,         ///<塞尔维亚
    COUNTRY_ET,         ///<埃塞俄比亚
    COUNTRY_VN,         ///<越南
    COUNTRY_NA,         ///<北美
    COUNTRY_TN,         ///<突尼斯
    COUNTRY_UZ,         ///<乌兹别克斯坦
    COUNTRY_NP,         ///<尼泊尔
    COUNTRY_BD,         ///<孟加拉国
    COUNTRY_ES,         ///<西班牙
    COUNTRY_CR,         ///<哥斯达黎加
    COUNTRY_VE,         ///<委内瑞拉
    COUNTRY_FR,         ///<法国
    COUNTRY_DE,         ///<德国
    COUNTRY_NL,         ///<荷兰
    COUNTRY_PT,         ///<葡萄牙
    COUNTRY_SE,         ///<瑞典
    COUNTRY_PL,         ///<波兰
    COUNTRY_AT,         ///<奥地利
    COUNTRY_BE,         ///<比利时
    COUNTRY_CZ,         ///<捷克
    COUNTRY_HU,         ///<匈牙利
    COUNTRY_DK,         ///<丹麦
    COUNTRY_LU,         ///<卢森堡
    COUNTRY_RO,         ///<罗马尼亚
    COUNTRY_SK,         ///<斯洛伐克
    COUNTRY_LV,         ///<拉脱维亚
    COUNTRY_MT,         ///<马耳他
    COUNTRY_CY,         ///<塞浦路斯
    COUNTRY_SI,         ///<斯洛文尼亚
    COUNTRY_HR,         ///<克罗地亚
    COUNTRY_FI,         ///<芬兰
    COUNTRY_LT,         ///<立陶宛
    COUNTRY_IE,         ///<爱尔兰
    COUNTRY_BG,         ///<保加利亚
    COUNTRY_GR,         ///<希腊
    COUNTRY_EE,         ///<爱沙尼亚
    COUNTRY_CH,         ///<瑞士
    COUNTRY_SG,          ///<新加坡
    COUNTRY_DEFAULT
} ADM_COUNTRY_E;

typedef enum
{
    CHERY_BRAND = -1,
    EBRO_BRAND = 0,
    EXEED_BRAND = 1,
    EXLANTIX_BRAND = 2,
    JAECOO_BRAND = 3,
    OMODA_BRAND = 4,
    ESTEO_BRAND = 5,
    JELAND_BRAND = 6,
    ONIVES_BRAND = 7,
    TENET_BRAND = 8,
    FOWNIX_BRAND = 9,
    LUCANO_BRAND = 10,
    XTRIM_BRAND = 11,
    XCITE_BRAND = 12,
    DEFAULT_BRAND
} ADM_BRAND_E;

typedef struct ADM_OCA_COMMON_INIT_HANDLER_ ADM_OCA_COMMON_INIT_HANDLER_T;  //!< 通信句柄前向声明
/**
 * @brief 定义与平台通信的协议类型枚举值
 * 
 */
typedef enum ADM_OCA_CONNECT_PROTOCOL_TYPE_ {
    ADM_OCA_CONNECT_PROTOCOL_TYPE_HTTP = 1,         /**!< HTTP协议 */
    ADM_OCA_CONNECT_PROTOCOL_TYPE_HTTPS_ONE_AUTH,   /**!< HTTPS协议，单向认证 */
    ADM_OCA_CONNECT_PROTOCOL_TYPE_HTTPS_MUL_AUTH,   /**!< HTTPS协议，双向认证 */
    ADM_OCA_CONNECT_PROTOCOL_TYPE_MQTT,             /**!< MQTT协议 */
    ADM_OCA_CONNECT_PROTOCOL_TYPE_MQTTS_ONE_AUTH,   /**!< MQTTS协议，单向认证 */
    ADM_OCA_CONNECT_PROTOCOL_TYPE_MQTTS_MUL_AUTH,   /**!< MQTTS协议，双向认证 */
    ADM_OCA_CONNECT_PROTOCOL_TYPE_TCP,              /**!< TCP socket */
    ADM_OCA_CONNECT_PROTOCOL_TYPE_INVALID           /**!< Invalid */
} ADM_OCA_CONNECT_PROTOCOL_TYPE_E;

/**
 * @brief 设置与平台通信的协议类型
 * 
 * @param connectHandler    [IN]    ADM_OCA_COMMON_INIT_HANDLER_T*   此次与平台通信的句柄
 * @param eProtocolType     [IN]    const ADM_OCA_CONNECT_PROTOCOL_TYPE_E   与平台通信的协议类型的枚举值
 * @return D32S             0:successed, Non-0:failed
 */
extern D32S adm_oca_setConnectProtocolType(ADM_OCA_COMMON_INIT_HANDLER_T* connectHandler, const ADM_OCA_CONNECT_PROTOCOL_TYPE_E eProtocolType);

/**
 * @brief 设置与平台通信的证书路径（绝对路径），单向认证或双向认证时调用
 * 
 * @param connectHandler    [IN]    ADM_OCA_COMMON_INIT_HANDLER_T*   此次与平台通信的句柄
 * @param serverRootCert    [IN]    const DCHAR*    服务端根证书路径，必填
 * @param clientCert        [IN]    const DCHAR*    客户端证书路径，双向认证时必填
 * @param clientPrivateKey  [IN]    const DCHAR*    客户端私钥路径，双向认证时必填
 * @return D32S             0:successed, Non-0:failed
 */
extern D32S adm_oca_setConnectCertPath(ADM_OCA_COMMON_INIT_HANDLER_T* connectHandler, const DCHAR* serverRootCert, const DCHAR* clientCert, const DCHAR* clientPrivateKey);

/**
 * @brief 与平台通信的地址类型枚举值定义
 * 
 */
typedef enum ADM_OCA_CONNECT_ADDRESS_TYPE_ {
    ADM_OCA_CONNECT_ADDRESS_TYPE_URL = 1,           /**!< URL, eg: http(s)://www.baidu.com */
    ADM_OCA_CONNECT_ADDRESS_TYPE_IP,                /**!< IP, eg: 172.16.30.56:8090 */
    ADM_OCA_CONNECT_ADDRESS_TYPE_DOMAIN,            /**!< Domain, eg: www.baidu.com:8888 */
    ADM_OCA_CONNECT_ADDRESS_TYPE_INVALID            /**!< Invalid */
} ADM_OCA_CONNECT_ADDRESS_TYPE_E;

/**
 * @brief 设置与平台通信的地址
 * @note   1. http(s)://www.baidu.com; 2. ip:port; 3. domain:port
 * 
 * @param connectHandler    [IN]    ADM_OCA_COMMON_INIT_HANDLER_T*  此次与平台通信的句柄
 * @param address           [IN]    const DCHAR*    与平台通信的地址定义
 * @return D32S             0:successed, Non-0:failed
 */
extern D32S adm_oca_setConnectAddress(ADM_OCA_COMMON_INIT_HANDLER_T* connectHandler, const ADM_OCA_CONNECT_ADDRESS_TYPE_E eAddressType, const DCHAR* address);

/**
 * @brief 设置报文的出口网卡或地址
 * @note 1. 可以配置网卡名，例如：eth0;
 *       2. 可以配置出口IP地址，例如："192.168.1.1"
 * @warning 如果不需要设置，可以传入NULL
 * 
 * @param connectHandler[IN]    ADM_OCA_COMMON_INIT_HANDLER_T*  此次与平台通信的出口网卡或地址
 * @param outputApn[IN]         const DCHAR*    网卡名或地址
 * @return D32S     0:Successed, None-0:Failed
 */
extern D32S adm_oca_setConnectOutputApn(ADM_OCA_COMMON_INIT_HANDLER_T* connectHandler, const DCHAR* outputApn);

/**
 * @brief 触发请求平台，并接收平台的响应数据
 * 
 * @param connectHandler    [IN]    ADM_OCA_COMMON_INIT_HANDLER_T*   此次与平台通信的句柄
 * @param requestBody       [IN]    const DCHAR*    向平台发送的请求数据
 * @param responseBody      [OUT]   DCHAR**     平台响应的数据
 * @param statusCode        [OUT]   DSIZE*       平台响应的状态码
 * @return D32S             0:successed, Non-0:failed
 */
extern D32S adm_oca_requestConnectServer(ADM_OCA_COMMON_INIT_HANDLER_T* connectHandler, const DCHAR* requestBody, DCHAR** responseBody, DSIZE* statusCode);


/**
 * @brief 与平台通信的句柄初始化接口
 * 
 * @return connectHandler    [OUT]   ADM_OCA_COMMON_INIT_HANDLER_T*    此次通信的句柄
 */
extern ADM_OCA_COMMON_INIT_HANDLER_T* adm_oca_connectInit(void);


/**
 * @brief 释放与平台通信的句柄
 * 
 * @param connectHandler    [IN]    ADM_OCA_COMMON_INIT_HANDLER_T*   此次通信的句柄
 */
extern void adm_oca_connectDeinit(ADM_OCA_COMMON_INIT_HANDLER_T* connectHandler);

#ifdef __cplusplus  // close out "C" linkage in case of c++ compiling
}
#endif

#endif//__ADM_OCA_CONNECT_H__
