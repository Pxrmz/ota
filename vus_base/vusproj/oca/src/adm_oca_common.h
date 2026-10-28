/**
 * @file adm_oca_common.h
 * @author mengwei@abupdate.com
 * @brief  OTA 网络连接适配层通用接口定义 
 * @version 0.1
 * @date 2021-05-13
 * 
 * @copyright Copyright © 2018 All rights 上海艾拉比智能科技有限公司
 * 
 */
#ifndef __ADM_OCA_COMMON_H__
#define __ADM_OCA_COMMON_H__

#ifdef __cplusplus
extern "C" {
#endif

#define EXPORT_FUN __attribute__((visibility("default")))

#include "adm_typedefs.h"
#include "adm_oca_log.h"
#include "adm_oca_connect.h"

typedef struct ADM_OCA_COMMON_INIT_HANDLER_ {
    ADM_OCA_CONNECT_PROTOCOL_TYPE_E eConnectProtocolType;   //!< 连接的协议类型
    ADM_OCA_CONNECT_ADDRESS_TYPE_E  eConnectAddressType;    //!< 连接的地址类型
    DCHAR* serverRootCert;                                  //!< 服务端根证书
    DCHAR* clientCert;                                      //!< 客户端证书
    DCHAR* clientPrivateKey;                                //!< 客户端私钥
    DCHAR* address;                                         //!< 请求的地址
    D16U port;                                              //!< 请求的端口
    DCHAR* requestBody;                                     //!< 请求的数据报文
    DCHAR* outputApn;                                       //!< 出口的APN网卡设置
    DCHAR* apiName;                                         //!< 请求的业务接口名，主要用于MQTT协议中
    DCHAR* pTopicName;                                      //!< 与平台使用MQTT协议交互的主题类型
    D32S   iSeqNo;
    D32S   iBusinessType;
} ADM_OCA_COMMON_INIT_HANDLER_T;

#ifdef __cplusplus  // close out "C" linkage in case of c++ compiling
}
#endif

#endif//__ADM_OCA_COMMON_H__