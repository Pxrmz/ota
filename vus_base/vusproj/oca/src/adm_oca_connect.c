/**
 * @file adm_oca_connect.h
 * @author mengwei@abupdate.com
 * @brief 定义车云之间协议交互的接口，包括车端请求云端和云端响应车端的接口。
 * @version 0.1
 * @date 2021-05-13
 * 
 * @copyright Copyright © 2018 All rights 上海艾拉比智能科技有限公司
 * 
 */
#include "adm_oca_common.h"
#include "adm_oca_httpConnect.h"
#include "adm_oca_httpErrorCode.h"
#include "adm_memory.h"
#include "adm_string.h"

/**
 * @brief 设置与平台通信的协议类型
 * 
 * @param connectHandler    [IN]    ADM_OCA_COMMON_INIT_HANDLER_T*   此次与平台通信的句柄
 * @param eProtocolType     [IN]    const ADM_OCA_CONNECT_PROTOCOL_TYPE_E   与平台通信的协议类型的枚举值
 * @return D32S             0:successed, Non-0:failed
 */
D32S adm_oca_setConnectProtocolType(ADM_OCA_COMMON_INIT_HANDLER_T* connectHandler, const ADM_OCA_CONNECT_PROTOCOL_TYPE_E eProtocolType)
{
    if (NULL == connectHandler) {
        log_e("connectHandler is nil");
        return ADM_OCA_HTTP_ECODE_INVALID_PARAM;
    }

    if (eProtocolType < ADM_OCA_CONNECT_PROTOCOL_TYPE_HTTP || eProtocolType > ADM_OCA_CONNECT_PROTOCOL_TYPE_TCP) {
        log_e("eProtocolType:%d is invalid");
        return ADM_OCA_HTTP_ECODE_PROTOCOL_TYPE_INVALID;
    }
    connectHandler->eConnectProtocolType = eProtocolType;

    return 0;
}

/**
 * @brief 设置与平台通信的证书路径（绝对路径），单向认证或双向认证时调用
 * 
 * @param connectHandler    [IN]    ADM_OCA_COMMON_INIT_HANDLER_T*   此次与平台通信的句柄
 * @param serverRootCert    [IN]    const DCHAR*    服务端根证书路径，必填
 * @param clientCert        [IN]    const DCHAR*    客户端证书路径，双向认证时必填
 * @param clientPrivateKey  [IN]    const DCHAR*    客户端私钥路径，双向认证时必填
 * @return D32S             0:successed, Non-0:failed
 */
D32S adm_oca_setConnectCertPath(ADM_OCA_COMMON_INIT_HANDLER_T* connectHandler, const DCHAR* serverRootCert, const DCHAR* clientCert, const DCHAR* clientPrivateKey)
{
    if (NULL == connectHandler) {
        log_e("connectHandler is nil");
        return ADM_OCA_HTTP_ECODE_INVALID_PARAM;
    }

    if (NULL != serverRootCert) {
        connectHandler->serverRootCert = adm_strdup(serverRootCert);
    }

    if (NULL != clientCert) {
        connectHandler->clientCert = adm_strdup(clientCert);
    }

    if (NULL != clientPrivateKey) {
        connectHandler->clientPrivateKey = adm_strdup(clientPrivateKey);
    }

    return 0;
}

/**
 * @brief 设置与平台通信的地址
 * @note   1. http(s)://www.baidu.com; 2. ip:port; 3. domain:port
 * 
 * @param connectHandler    [IN]    ADM_OCA_COMMON_INIT_HANDLER_T*  此次与平台通信的句柄
 * @param address           [IN]    const DCHAR*    与平台通信的地址定义
 * @return D32S             0:successed, Non-0:failed
 */
D32S adm_oca_setConnectAddress(ADM_OCA_COMMON_INIT_HANDLER_T* connectHandler, const ADM_OCA_CONNECT_ADDRESS_TYPE_E eAddressType, const DCHAR* address)
{
    if (NULL == connectHandler || NULL == address) {
        log_e("connectHandler or address is nil");
        return ADM_OCA_HTTP_ECODE_INVALID_PARAM;
    }

    switch (eAddressType)
    {
    case ADM_OCA_CONNECT_ADDRESS_TYPE_URL:
        connectHandler->address = adm_strdup(address);
        break;
    case ADM_OCA_CONNECT_ADDRESS_TYPE_IP:
        // TODO: parse ip:port
        break;
    case ADM_OCA_CONNECT_ADDRESS_TYPE_DOMAIN:  
        // TODO: parse domain:port
        connectHandler->address = adm_strdup(address);
        break;
    case ADM_OCA_CONNECT_ADDRESS_TYPE_INVALID:
    default:
        break;
    }

    return ADM_OCA_HTTP_ECODE_SUCCESSED;
}

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
D32S adm_oca_setConnectOutputApn(ADM_OCA_COMMON_INIT_HANDLER_T* connectHandler, const DCHAR* outputApn)
{
    if (NULL == connectHandler || NULL == outputApn) {
        log_e("connectHandler or address is nil");
        return ADM_OCA_HTTP_ECODE_INVALID_PARAM;
    }

    connectHandler->outputApn = adm_strdup(outputApn);

    return ADM_OCA_HTTP_ECODE_SUCCESSED;
}

/**
 * @brief 触发请求平台，并接收平台的响应数据
 * 
 * @param connectHandler    [IN]    ADM_OCA_COMMON_INIT_HANDLER_T*   此次与平台通信的句柄
 * @param requestBody       [IN]    const DCHAR*    向平台发送的请求数据
 * @param responseBody      [OUT]   DCHAR**     平台响应的数据
 * @param statusCode        [OUT]   DSIZE*       平台响应的状态码
 * @return D32S             0:successed, Non-0:failed
 */
D32S adm_oca_requestConnectServer(ADM_OCA_COMMON_INIT_HANDLER_T* connectHandler, const DCHAR* requestBody, DCHAR** responseBody, DSIZE* statusCode)
{   
    D32S iRet = ADM_OCA_HTTP_ECODE_FAILED;

    if (NULL == connectHandler || NULL == requestBody) {
        log_e("connectHandler or requestBody is nil");
        return ADM_OCA_HTTP_ECODE_INVALID_PARAM;
    }

    do {
        connectHandler->requestBody = adm_strdup(requestBody);
        switch (connectHandler->eConnectProtocolType)
        {
            case ADM_OCA_CONNECT_PROTOCOL_TYPE_HTTP:
            case ADM_OCA_CONNECT_PROTOCOL_TYPE_HTTPS_ONE_AUTH:
            case ADM_OCA_CONNECT_PROTOCOL_TYPE_HTTPS_MUL_AUTH:
                iRet = adm_oca_httpConnect(connectHandler, responseBody, statusCode);
                break;
            case ADM_OCA_CONNECT_PROTOCOL_TYPE_MQTT:
            case ADM_OCA_CONNECT_PROTOCOL_TYPE_MQTTS_ONE_AUTH:
            case ADM_OCA_CONNECT_PROTOCOL_TYPE_MQTTS_MUL_AUTH:
                // adm_oca_mqttConnect();
                // iRet = adm_oca_mqttSendMessage(connectHandler,responseBody,statusCode);
                break;
            case ADM_OCA_CONNECT_PROTOCOL_TYPE_TCP:
                // adm_oca_tcpConnect();
                break;
            case ADM_OCA_CONNECT_PROTOCOL_TYPE_INVALID:
            default:
                log_e("eProtocolType %d is invalid", connectHandler->eConnectProtocolType);
                break;
        }
        adm_free(connectHandler->requestBody);
    } while(FALSE);

    return iRet;
}

/**
 * @brief 与平台通信的句柄初始化接口
 * 
 * @return connectHandler    [OUT]   ADM_OCA_COMMON_INIT_HANDLER_T*    此次通信的句柄
 */
ADM_OCA_COMMON_INIT_HANDLER_T* adm_oca_connectInit(void)
{
    ADM_OCA_COMMON_INIT_HANDLER_T* connectHandler = NULL;

    connectHandler = adm_malloc(sizeof(ADM_OCA_COMMON_INIT_HANDLER_T));
    if (NULL != connectHandler) {
        (void)adm_memset(connectHandler, 0x00, sizeof(ADM_OCA_COMMON_INIT_HANDLER_T), sizeof(ADM_OCA_COMMON_INIT_HANDLER_T) * sizeof(DCHAR));
    }

    return connectHandler;
}

/**
 * @brief 释放与平台通信的句柄
 * 
 * @param connectHandler    [IN]    ADM_OCA_COMMON_INIT_HANDLER_T*   此次通信的句柄
 */
void adm_oca_connectDeinit(ADM_OCA_COMMON_INIT_HANDLER_T* connectHandler)
{
    if (NULL != connectHandler) {
        if (NULL != connectHandler->serverRootCert) {
            adm_free(connectHandler->serverRootCert);
        }

        if (NULL != connectHandler->clientCert) {
            adm_free(connectHandler->clientCert);
        }

        if (NULL != connectHandler->clientPrivateKey) {
            adm_free(connectHandler->clientPrivateKey);
        }

        if(NULL != connectHandler->address) {
            adm_free(connectHandler->address);
        }

        if(NULL != connectHandler->requestBody) {
            adm_free(connectHandler->requestBody);
        }

        if (NULL != connectHandler->outputApn) {
            adm_free(connectHandler->outputApn);
        }

        adm_free(connectHandler);
    }
}