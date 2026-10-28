/**
 * @file adm_cfg_default.h
 * @author mengwei@abupdate.com
 * @brief 配置文件中获取失败，或无法获取的情况，会返回默认值，默认值在此文件中定义
 * @version 0.1
 * @date 2021-12-13
 * 
 * @brief 配置文件的解析，设置或获取字段等，包括：VUS、SYSI、eweb、rpc等的所有配置信息
 * 
 */
#ifndef __ADM_CFG_DEFAULT_H__
#define __ADM_CFG_DEFAULT_H__

#ifdef __cplusplus
extern "C" {
#endif

#define EXPORT_FUN __attribute__((visibility("default")))

/*************************************************************配置文件版本信息***********************************************************************/
#define ADM_CFG_OWNER_VER       "VER"       //!< <OWNER>    VER
#define ADM_CFG_OTA_CONFIG_VER              ADM_CFG_OWNER_VER":ota_config_ver"                  //!< <KEY>  配置文件版本号
#define ADM_CFG_OTA_CONFIG_VER_TIME         ADM_CFG_OWNER_VER":ota_config_ver_time"             //!< <KEY>  配置文件最新修改的时间
#define ADM_CFG_OTA_CONFIG_MODIFY_AUTHOR    ADM_CFG_OWNER_VER":ota_config_ver_modify_author"    //!< <KEY>  配置文件最新修改的作者
#define ADM_CFG_OTA_CONFIG_MODIFY_CONTENT   ADM_CFG_OWNER_VER":ota_config_ver_modify_content"   //!< <KEY>  配置文件更新履历
#define ADM_CFG_OTA_CONFIG_OEM_LOGO         ADM_CFG_OWNER_VER":ota_config_oem_logo"             //!< <KEY>  配置文件对应的项目标识

#define ADM_CFG_OTA_CONFIG_VER_DEFAULT_VAL              "0.0.6"                                 //!< <VALUE>    配置文件版本号的默认值
#define ADM_CFG_OTA_CONFIG_VER_TIME_DEFAULT_VAL         "2021-12-13 14:38"                      //!< <VALUE>    配置文件最新修改的时间的默认值
#define ADM_CFG_OTA_CONFIG_MODIFY_AUTHOR_DEFAULT_VAL    "abup"                                  //!< <VALUE>    配置文件最新修改的作者的默认值
#define ADM_CFG_OTA_CONFIG_MODIFY_CONTENT_DEFAULT_VAL   "增加日志大小和存储文件个数配置"           //!< <VALUE>    配置文件更新履历的默认值
#define ADM_CFG_OTA_CONFIG_OEM_LOGO_DEFAULT_VAL         "EVG HENGCHI_1"                         //!< <VALUE>    配置文件对应的项目标识的默认值
/*************************************************************配置文件版本信息***********************************************************************/

/*************************************************************VUS主控程序配置信息********************************************************************/
#define ADM_CFG_OWNER_VUS       "VUS"   //!< <OWNER>    VUS
#define ADM_CFG_VUS_REAL_VEHICLE_VER    ADM_CFG_OWNER_VUS":is_real_vehicle_ver"                 //!< <KEY>  当前版本是否为实车版本
#define ADM_CFG_VUS_DEF_VEHICLE_IDEN    ADM_CFG_OWNER_VUS":default_vehicle_iden"                //!< <KEY>  非实车版本，使用的车辆VIN码
#define ADM_CFG_VUS_IS_KVSTORAGE_ENCRYPT    ADM_CFG_OWNER_VUS":is_kvstorage_encrypt"            //!< <KEY>  kvstorage数据库是否加密
#define ADM_CFG_VUS_IS_COREDUMP_UPLOAD    ADM_CFG_OWNER_VUS":is_coredump_upload"                //!< <KEY>  系统崩溃后是否生成并上报coredump信息
#define ADM_CFG_VUS_IS_DETACH_RETRY         ADM_CFG_OWNER_VUS":is_detach_retry"                //!< <KEY>  是否分离刷写重试
#define ADM_CFG_VUS_IS_CFG_PRESSURE         ADM_CFG_OWNER_VUS":is_cfg_pressure"                //!< <KEY>  是否进行配置压测，TRUE(1):Y, FALSE(0):N
#define ADM_CFG_VUS_MODULE_NAME         ADM_CFG_OWNER_VUS":module_name"                            //!< <KEY>  集成模块名
#define ADM_CFG_VUS_KEYLOG_PATH        ADM_CFG_OWNER_VUS":keylog_path"       //!< <KEY>  keylog日志文件路径
#define ADM_CFG_VUS_KEY_LOG_MAX_SIZE        ADM_CFG_OWNER_VUS":keylog_log_max_size"         //!< <KEY>      单个日志文件存储的大小，单位：Byte
#define ADM_CFG_VUS_IS_REPORT_KEYLOG        ADM_CFG_OWNER_VUS":is_report_keylog"         //!< <KEY>      是否上报KEYLOG


#define ADM_CFG_VUS_IS_START_HTTP_PROXY        ADM_CFG_OWNER_VUS":is_start_http_proxy"            //!< <KEY>  是否启动代理网络服务
#define ADM_CFG_VUS_IS_HTTP_PROXY_CFG_PATH     ADM_CFG_OWNER_VUS":http_proxy_cfg_path"            //!< <KEY>  网络代理配置文件路径
#define ADM_CFG_VUS_IS_HTTP_PROXY              ADM_CFG_OWNER_VUS":is_http_proxy"                  //!< <KEY>  是否开启网络代理
#define ADM_CFG_VUS_IS_HTTP_PROXY_IP           ADM_CFG_OWNER_VUS":http_proxy_ip"                  //!< <KEY>  代理网络ip
#define ADM_CFG_VUS_CURL_PORT                  ADM_CFG_OWNER_VUS":curl_port"                      //!< <KEY>  curl端口

#define ADM_CFG_VUS_REAL_VEHICLE_VER_DEFAULT_VAL    0                                           //!< <VALUE>    当前版本是否为实车版本，TRUE(1):Y，FALSE(0):N
#define ADM_CFG_VUS_DEF_VEHICLE_IDEN_DEFAULT_VAL    "EVG20210625111000"                         //!< <VALUE>    非实车版本，使用的车辆VIN码
#define ADM_CFG_VUS_IS_KVSTORAGE_ENCRYPT_DEFAULT_VAL    0                                       //!< <VALUE>    kvstorage数据库是否加密，TRUE(1):Y, FALSE(0):N
#define ADM_CFG_VUS_IS_COREDUMP_UPLOAD_DEFAULT_VAL    0                                         //!< <VALUE>    系统崩溃后是否生成并上报coredump信息，TRUE(1):Y, FALSE(0):N
#define ADM_CFG_VUS_IS_DETACH_RETRY_DEFAULT_VAL       1                                         //!< <VALUE>  是否分离刷写重试，TRUE(1):Y, FALSE(0):N
#define ADM_CFG_VUS_IS_CFG_PRESSURE_DEFAULT_VAL       0                                         //!< <VALUE>  是否进行配置压测，TRUE(1):Y, FALSE(0):N
#define ADM_CFG_VUS_CURL_PORT_DEFAULT_VAL             12345                                     //!< <VALUE>  curl端口
#define ADM_CFG_VUS_MODULE_NAME_DEFAULT_VAL           "LITE_IHU"                                  //!< <VALUE>  模块名
#define ADM_CFG_VUS_KEYLOG_PATH_DEFAULT_VAL        "/tmp/admota/ucm/keylog/keylog.txt"       //!< <KEY>  keylog日志文件路径
#define ADM_CFG_VUS_KEY_LOG_MAX_SIZE_DEFAULT_VAL      1045504
#define ADM_CFG_VUS_IS_REPORT_KEYLOG_DEFAULT_VAL       0                                         //!< <VALUE>  是否上报KEYLOG，TRUE(1):Y, FALSE(0):N


#define ADM_CFG_VUS_RUNTIME_ROOT_DIR    ADM_CFG_OWNER_VUS":runtime_root_dir"                    //!< <KEY>  运行时目录
#define ADM_CFG_VUS_KV_STORAGE_DIR      ADM_CFG_OWNER_VUS":kv_storage_dir"                      //!< <KEY>  kv使用的目录
#define ADM_CFG_VUS_PKG_STORAGE_DIR     ADM_CFG_OWNER_VUS":pkg_storage_dir"                     //!< <KEY>  升级包存储目录
#define ADM_CFG_VUS_LOG_STORAGE_DIR     ADM_CFG_OWNER_VUS":log_storage_dir"                     //!< <KEY>  日志使用的目录
#define ADM_CFG_VUS_LOG_COLLECT_DIR     ADM_CFG_OWNER_VUS":log_collect_dir"                     //!< <KEY>  日志收集存储目录
#define ADM_CFG_VUS_LOG_PACK_DIR        ADM_CFG_OWNER_VUS":log_pack_dir"                        //!< <KEY>  被收集日志打包后的存储目录
#define ADM_CFG_VUS_CERT_STORAGE_DIR    ADM_CFG_OWNER_VUS":cert_storage_dir"                    //!< <KEY>  证书存储目录
#define ADM_CFG_VUS_UNZIP_STORAGE_DIR   ADM_CFG_OWNER_VUS":unzip_storage_dir"                   //!< <KEY>  文件解压目录
#define ADM_CFG_VUS_APK_LOG_STORAGE_DIR ADM_CFG_OWNER_VUS":apk_log_storage_dir"                 //!< <KEY>  APK日志存储目录

#define ADM_CFG_VUS_PKI_DIR ADM_CFG_OWNER_VUS":pki_storage_dir"                 //!< <KEY> pki产生数据存储目录,要求持久化存储目录，可读写

#define ADM_CFG_VUS_CURL_VERBOSE_DIR    ADM_CFG_OWNER_VUS":curl_verbose_dir"                    //!< <KEY>  curl详细信息存储目录
#define ADM_CFG_VUS_UPLOAD_CURL_VERBOSE_DIR    ADM_CFG_OWNER_VUS":upload_curl_verbose_dir"                    //!< <KEY>  日志上传curl详细信息存储目录

#define ADM_CFG_VUS_RUNTIME_ROOT_DIR_DEFAULT_VAL    "/tmp/admota/ucm/"                                 //!< <VALUE>    运行时目录的默认值
#define ADM_CFG_VUS_KV_STORAGE_DIR_DEFAULT_VAL      "/tmp/admota/ucm/kvstorage"                        //!< <VALUE>    kv使用目录的默认值
#define ADM_CFG_VUS_PKG_STORAGE_DIR_DEFAULT_VAL     "/tmp/admota/ucm/package/"                         //!< <VALUE>    升级包存储目录的默认值
#define ADM_CFG_VUS_LOG_STORAGE_DIR_DEFAULT_VAL     "/tmp/admota/ucm/log/storage/"                     //!< <VALUE>    日志使用目录的默认值
#define ADM_CFG_VUS_LOG_COLLECT_DIR_DEFAULT_VAL     "/tmp/admota/ucm/log/collect/"                     //!< <VALUE>    日志收集存储目录
#define ADM_CFG_VUS_LOG_PACK_DIR_DEFAULT_VAL        "/tmp/admota/ucm/log/pack/"                        //!< <VALUE>    被收集日志打包后的存储目录
#define ADM_CFG_VUS_CERT_STORAGE_DIR_DEFAULT_VAL    "/tmp/admota/ucm/cert/"                            //!< <VALUE>    证书存储目录的默认值
#define ADM_CFG_VUS_UNZIP_STORAGE_DIR_DEFAULT_VAL   "/tmp/admota/ucm/unzip"                            //!< <VALUE>    文件解压目录的默认值
#define ADM_CFG_VUS_APK_LOG_STORAGE_DIR_DEFAULT_VAL "/tmp/admota/cache/logger/"                        //!< <VALUE>    APK日志存储目录默认值

#define ADM_CFG_VUS_PKI_DIR_DEFAULT_VAL "/tmp/admota/pki/data/"  //!< <VALUE>    PKI数据存储目录

#define ADM_CFG_VUS_WAIT_NETWORK_OK_SEC 	ADM_CFG_OWNER_VUS":wait_network_ok_sec"                 //!< <KEY>  等待网络恢复时间
#define ADM_CFG_VUS_WAIT_NETWORK_OK_COUNT 	ADM_CFG_OWNER_VUS":wait_network_ok_count"               //!< <KEY>  等待网络恢复重试次数
#define ADM_CFG_VUS_HMIPOLICYFILE_IS_DOWNLOAD ADM_CFG_OWNER_VUS":hmipolicyfile_is_download"     //!< <KEY>  是否需要下载HMI策略文件

#define ADM_CFG_VUS_WAIT_NETWORK_OK_SEC_DEFAULT_VAL  3                                          //!< <VALUE>    等待网络恢复时间的默认值
#define ADM_CFG_VUS_WAIT_NETWORK_OK_COUNT_DEFAULT_VAL  3                                        //!< <VALUE>    等待网络恢复重试次数的默认值
#define ADM_CFG_VUS_HMIPOLICYFILE_IS_DOWNLOAD_DEFAULT_VAL  1                                    //!< <VALUE>    是否需要下载HMI策略文件

#define ADM_CFG_VUS_IS_SET_KVDATA_ABNORMAL_DEAL  ADM_CFG_OWNER_VUS":is_set_kvdata_abnormal_deal"      //!< <VALUE>    Y：异常处理 N：不异常处理
#define ADM_CFG_VUS_PROCESS_RESTART_TIMES        ADM_CFG_OWNER_VUS":process_restart_times"            //!< <VALUE>    进程重启次数
#define ADM_CFG_VUS_PROCESS_RESTART_TOTALTIME    ADM_CFG_OWNER_VUS":process_restart_totaltime"        //!< <VALUE>    该时间内达到重启次数，需要清空数据库数据，单位:Second

#define ADM_CFG_VUS_IS_SET_KVDATA_ABNORMAL_DEAL_VAL 0                                           //!< <VALUE>    是否异常处理，TRUE(1):Y, FALSE(0):N 
#define ADM_CFG_VUS_PROCESS_RESTART_TIMES_VAL       10                                          //!< <VALUE>    进程重启次数的默认值
#define ADM_CFG_VUS_PROCESS_RESTART_TOTALTIME_VAL   600                                         //!< <VALUE>    该时间内达到重启次数，需要清空数据库数据的默认值

#define ADM_CFG_VUS_LOG_MAX_SIZE        ADM_CFG_OWNER_VUS":log_max_size_of_single_file"         //!< <KEY>      单个日志文件存储的大小，单位：Byte
#define ADM_CFG_VUS_LOG_MAX_ROTATE      ADM_CFG_OWNER_VUS":log_max_rotate_file"                 //!< <KEY>      最多存储的文件个数
#define ADM_CFG_VUS_LOG_OUTPUT_TERMINAL ADM_CFG_OWNER_VUS":log_output_terminal"                 //!< <KEY>      是否输出log至串口终端

#define ADM_CFG_VUS_LOG_MAX_SIZE_DEFAULT_VAL        1045504                                     //!< <VALUE>   单个日志文件存储大小的默认值，单位：Byte 
#define ADM_CFG_VUS_LOG_MAX_ROTATE_DEFAULT_VAL      5                                           //!< <VALUE>   最多存储的文件个数的默认值
#define ADM_CFG_VUS_LOG_OUTPUT_TERMINAL_DEFAULT_VAL 1                                           //!< <VALUE>   默认输出log至串口终端

#define ADM_CFG_VUS_IS_START_HTTP_PROXY_DEFAULT_VAL       0                            //!< <VALUE>  是否启动代理网络
#define ADM_CFG_VUS_IS_HTTP_PROXY_DEFAULT_VAL      0                                   //!< <VALUE>  是否开启网络代理
#define ADM_CFG_VUS_IS_HTTP_PROXY_CFG_PATH_DEFAULT_VAL ":../etc/httpproxyCfg.ini"      //!< <VALUE>  网络代理配置文件路径
#define ADM_CFG_VUS_IS_HTTP_PROXY_IP_DEFAULT_VAL ":127.0.0.1:8888"                     //!< <VALUE>  代理网络ip

#define ADM_CFG_VUS_COMM_CERT_PATH      ADM_CFG_OWNER_VUS":comm_cert_path"                      //!< <KEY>  VUS主控程序与OTA平台通信的证书路径(注册、检测、上报等)
#define ADM_CFG_VUS_CDN_CERT_PATH       ADM_CFG_OWNER_VUS":cdn_cert_path"                       //!< <KEY>  VUS主控程序与CDN平台通信的证书路径(升级包下载)
#define ADM_CFG_VUS_CDN_CERT_DIR       ADM_CFG_OWNER_VUS":cdn_cert_dir"                       //!< <KEY>  VUS主控程序与CDN平台通信的证书路径(升级包下载)
#define ADM_CFG_VSU_RPC_COMM_CERT_PATH  ADM_CFG_OWNER_VUS":rpc_comm_cert_path"                  //!< <KEY>  VUS主控程序与从控程序通信使用的证书路径

#define ADM_CFG_VUS_COMM_CERT_PATH_DEFAULT_VAL          "/tmp/admota/ucm/cert/comm.cert"                   //!< <VALUE>    VUS主控程序与OTA平台通信的证书路径(注册、检测、上报等)
#define ADM_CFG_VUS_CDN_CERT_PATH_DEFAULT_VAL           "/tmp/admota/ucm/cert/cdn.cert"                    //!< <VALUE>    VUS主控程序与CDN平台通信的证书路径(升级包下载)
#define ADM_CFG_VUS_CDN_CERT_DIR_DEFAULT_VAL           "/tmp/admota/ucm/cert/"                    //!< <VALUE>    VUS主控程序与CDN平台通信的证书路径(升级包下载)
#define ADM_CFG_VSU_RPC_COMM_CERT_PATH_DEFAULT_VAL      "/tmp/admota/ucm/cert/rpccomm.cert"                //!< <VALUE>    VUS主控程序与从控程序通信使用的证书路径
#define ADM_CFG_VUS_CLI_CERT_PATH_DEFAULT_VAL           "/tmp/admota/ucm/cert/arpcClient.crt"
#define ADM_CFG_VUS_CLI_PRIVATE_KEY_PATH_DEFAULT_VAL    "/tmp/admota/ucm/cert/arpcClient.key"

#define ADM_CFG_VUS_COMM_CERT_PATH_2      ADM_CFG_OWNER_VUS":comm_cert_path_2"                      //!< <KEY>  VUS主控程序与OTA平台通信的证书路径(注册、检测、上报等)
#define ADM_CFG_VUS_CDN_CERT_PATH_2       ADM_CFG_OWNER_VUS":cdn_cert_path_2"                       //!< <KEY>  VUS主控程序与CDN平台通信的证书路径(升级包下载)
#define ADM_CFG_VUS_CDN_CERT_DIR_2       ADM_CFG_OWNER_VUS":cdn_cert_dir_2"                       //!< <KEY>  VUS主控程序与CDN平台通信的证书路径(升级包下载)

#define ADM_CFG_VUS_COMM_CERT_PATH_DEFAULT_VAL_2          "/tmp/admota/ucm/cert/comm.cert"                   //!< <VALUE>    VUS主控程序与OTA平台通信的证书路径(注册、检测、上报等)
#define ADM_CFG_VUS_CDN_CERT_PATH_DEFAULT_VAL_2           "/tmp/admota/ucm/cert/cdn.cert"                    //!< <VALUE>    VUS主控程序与CDN平台通信的证书路径(升级包下载)
#define ADM_CFG_VUS_CDN_CERT_DIR_DEFAULT_VAL_2           "/tmp/admota/ucm/cert/"                    //!< <VALUE>    VUS主控程序与CDN平台通信的证书路径(升级包下载)
#define ADM_CFG_VUS_CLI_CERT_PATH_DEFAULT_VAL_2           "/tmp/admota/ucm/cert/arpcClient.crt"
#define ADM_CFG_VUS_CLI_PRIVATE_KEY_PATH_DEFAULT_VAL_2    "/tmp/admota/ucm/cert/arpcClient.key"

/** 第一套云端环境 */
#define ADM_CFG_VUS_OTA_SRV_BUSINESS_PROTOCOL_TYPE   ADM_CFG_OWNER_VUS":ota_server_business_protocol_type"        //!< <KEY>  1:http; 2:https; 3:mqtt; 4. mqtts 业务通信接口的协议类型 1:http; 2:https; 3:mqtt; 4. mqtts
#define ADM_CFG_VUS_OTA_SRV_DOWNLOAD_PROTOCOL_TYPE  ADM_CFG_OWNER_VUS":ota_server_file_download_protocol_type"      //!< <KEY>  文件下载的协议类型 1:http; 2:https
#define ADM_CFG_VUS_OTA_SRV_UPLOAD_PROTOCOL_TYPE  ADM_CFG_OWNER_VUS":ota_server_file_upload_protocol_type"      //!< <KEY>  文件上传的协议类型 1:http; 2:https
#define ADM_CFG_VUS_OTA_SRV_DEFAULT_SECRET  ADM_CFG_OWNER_VUS":ota_server_default_secret"   //!< <KEY> 车云通信使用的默认Secret
#define ADM_CFG_VUS_OTA_SRV_REGISTER_URL    ADM_CFG_OWNER_VUS":ota_server_register_url"         //!< <KEY>  向OTA平台请求注册的地址
#define ADM_CFG_VUS_OTA_SRV_CHECK_URL       ADM_CFG_OWNER_VUS":ota_server_check_url"            //!< <KEY>  向OTA平台请求检测的地址
#define ADM_CFG_VUS_OTA_SRV_GET_CONFIG_URL  ADM_CFG_OWNER_VUS":ota_server_get_config_url"       //!< <KEY>  向OTA平台请求获取配置的地址
#define ADM_CFG_VUS_OTA_SRV_GET_ICP_URL     ADM_CFG_OWNER_VUS":ota_server_get_icp_url"       //!< <KEY>  向OTA平台请求获取ICP的地址
#define ADM_CFG_VUS_OTA_SRV_REPORT_VEHICLE_INFO_URL     ADM_CFG_OWNER_VUS":ota_server_report_vehicle_info_url"  //!< 向OTA平台请求上报车辆信息的地址
#define ADM_CFG_VUS_OTA_SRV_VEHICLE_STATE_REPORT_URL    ADM_CFG_OWNER_VUS":ota_server_vehicle_state_report_url" //!< <KEY>  向OTA平台请求上报车辆状态的地址
#define ADM_CFG_VUS_OTA_SRV_ECU_STATE_REPORT_URL        ADM_CFG_OWNER_VUS":ota_server_ecu_state_report_url"     //!< <KEY>  向OTA平台请求上报ECU状态的地址
#define ADM_CFG_VUS_OTA_SRV_PROGRESS_REPORT_URL         ADM_CFG_OWNER_VUS":ota_server_progress_report_url"      //!< <KEY>  向OTA平台请求上报进度信息的地址
#define ADM_CFG_VUS_OTA_SRV_EVENT_REPORT_URL            ADM_CFG_OWNER_VUS":ota_server_event_report_url"         //!< <KEY>  向OTA平台请求上报事件的地址
#define ADM_CFG_VUS_OTA_SRV_FILE_UPLOAD_URL             ADM_CFG_OWNER_VUS":ota_server_file_upload_url"          //!< <KEY>  向OTA平台请求上传日志的地址
#define ADM_CFG_VUS_OTA_SRV_PRE_CHECK_URL               ADM_CFG_OWNER_VUS":ota_server_pre_check_url"            //!< <KEY>  向OTA平台请求预检测的地址
#define ADM_CFG_VUS_OTA_SRV_GET_HISTORY_URL             ADM_CFG_OWNER_VUS":ota_server_get_history_url"          //!< <KEY>  向OTA平台请求获取升级历史记录的地址

#define ADM_CFG_VUS_OTA_SRV_SYNC_SCHDULE_URL       ADM_CFG_OWNER_VUS":ota_server_sync_schdule_url"            //!< <KEY>  向OTA平台同步预约信息的地址
#define ADM_CFG_VUS_OTA_SRV_TASK_VERIFY_URL       ADM_CFG_OWNER_VUS":ota_server_task_verify_url"            //!< <KEY>  向OTA平台进行任务有效性校验的地址
#define ADM_CFG_VUS_OTA_SRV_REQUEST_UPLOAD_URL    ADM_CFG_OWNER_VUS":ota_server_request_upload_url"            //!< <KEY>  向OTA平台请求文件上传url的地址

#define ADM_CFG_VUS_OTA_SRV_SSL_VERIFY_PEER             ADM_CFG_OWNER_VUS":ota_server_soa_ssl_verify_client"              //车云交互 是否进行双向认证 N:单向认证 Y:双向认证 
#define ADM_CFG_VUS_CLIENT_CERT_PATH                    ADM_CFG_OWNER_VUS":ota_server_soa_ssl_client_certfile"  //车云交互 车端(客户端)证书路径
#define ADM_CFG_VUS_CLIENT_PRIVATE_KEY_PATH             ADM_CFG_OWNER_VUS":ota_server_soa_ssl_client_keyfile"   //车云交互 车端(客户端)私钥路径

#define ADM_CFG_VUS_CDN_SRV_SSL_VERIFY_PEER             ADM_CFG_OWNER_VUS":ota_server_soa_ssl_download_verify_client"           //车cdn交互 是否进行双向认证 N:单向认证 Y:双向认证 
#define ADM_CFG_VUS_CDN_CLIENT_CERT_PATH                    ADM_CFG_OWNER_VUS":ota_server_soa_cdn_ssl_client_certfile"  //车cdn交互 车端(客户端)证书路径
#define ADM_CFG_VUS_CDN_CLIENT_PRIVATE_KEY_PATH             ADM_CFG_OWNER_VUS":ota_server_soa_cdn_ssl_client_keyfile"   //车cdn交互 车端(客户端)私钥路径

#define ADM_CFG_VUS_PKI_URL  ADM_CFG_OWNER_VUS":pki_server_url"    //PKI请求地址

#define ADM_CFG_VUS_OTA_SRV_ALLOW_MUL_AUTHEN_DEFAULT_VAL        0
#define ADM_CFG_VUS_CDN_SRV_ALLOW_MUL_AUTHEN_DEFAULT_VAL        0
#define ADM_CFG_VUS_OTA_SRV_BUSINESS_PROTOCOL_TYPE_DEFAULT_VAL   1      //!< <VALUE>    1:http; 2:https; 3:mqtt; 4. mqtts
#define ADM_CFG_VUS_OTA_SRV_DOWNLOAD_PROTOCOL_TYPE_DEFAULT_VAL  1      //!< <VALUE>  文件下载的协议类型 1:http; 2:https
#define ADM_CFG_VUS_OTA_SRV_UPLOAD_PROTOCOL_TYPE_DEFAULT_VAL  1      //!< <VALUE>  文件上传的协议类型 1:http; 2:https
#define ADM_CFG_VUS_OTA_SRV_DEFAULT_SECRET_DEFAULT_VAL  "39cabdfaab8c4da09bd6e9823c527836"                                                  //!< <VALUE> 车云通信使用的默认Secret 
#define ADM_CFG_VUS_OTA_SRV_REGISTER_URL_DEFAULT_VAL    "http://xiongshi.abupdate.com/v50/vehicle/api/register"                     //!< <VALUE>    向OTA平台请求注册的地址
#define ADM_CFG_VUS_OTA_SRV_CHECK_URL_DEFAULT_VAL       "http://xiongshi.abupdate.com/v50/vehicle/api/%s/check"                     //!< <VALUE>    向OTA平台请求检测的地址
#define ADM_CFG_VUS_OTA_SRV_GET_CONFIG_URL_DEFAULT_VAL  "http://xiongshi.abupdate.com/v50/vehicle/api/%s/client/config"             //!< <VALUE>    向OTA平台请求获取配置的地址
#define ADM_CFG_VUS_OTA_SRV_GET_ICP_URL_DEFAULT_VAL     "http://xiongshi.abupdate.com/v50/vehicle/api/%s/queryIcpRecord"     //!< <VALUE>    向OTA平台请求车辆ICP信息的地址

#define ADM_CFG_VUS_OTA_SRV_REPORT_VEHICLE_INFO_URL_DEFAULT_VAL     "http://xiongshi.abupdate.com/v50/vehicle/api/%s/report/vehicle/config"     //!< <VALUE>    向OTA平台请求上报车辆信息的地址
#define ADM_CFG_VUS_OTA_SRV_VEHICLE_STATE_REPORT_URL_DEFAULT_VAL    "http://xiongshi.abupdate.com/v50/vehicle/api/%s/state/report"  //!< <VALUE>    向OTA平台请求上报车辆状态的地址
#define ADM_CFG_VUS_OTA_SRV_ECU_STATE_REPORT_URL_DEFAULT_VAL        "http://xiongshi.abupdate.com/v50/vehicle/api/%s/ecu/state/report"     //!< <VALUE> 向OTA平台请求上报ECU状态的地址
#define ADM_CFG_VUS_OTA_SRV_PROGRESS_REPORT_URL_DEFAULT_VAL         "http://xiongshi.abupdate.com/v50/vehicle/api/%s/progress/report"      //!< <VALUE> 向OTA平台请求上报进度信息的地址
#define ADM_CFG_VUS_OTA_SRV_EVENT_REPORT_URL_DEFAULT_VAL            "http://xiongshi.abupdate.com/v50/vehicle/api/%s/event/report"         //!< <VALUE> 向OTA平台请求上报事件的地址
#define ADM_CFG_VUS_OTA_SRV_FILE_UPLOAD_URL_DEFAULT_VAL             "http://xiongshi.abupdate.com/v50/vehicle/api/%s/file/upload"          //!< <VALUE> 向OTA平台请求上传日志的地址
#define ADM_CFG_VUS_OTA_SRV_PRE_CHECK_URL_DEFAULT_VAL               "http://xiongshi.abupdate.com/v50/vehicle/api/%s/pre/check"            //!< <VALUE> 向OTA平台请求预检测的地址
#define ADM_CFG_VUS_OTA_SRV_GET_HISTORY_DEFAULT_VAL                 "http://xiongshi.abupdate.com/v50/vehicle/api/%s/upgrade/history"      //!< <VALUE> 向OTA平台请求查询升级历史记录的地址
#define ADM_CFG_VUS_OTA_SRV_SYNC_SCHDULE_URL_DEFAULT_VAL       "http://xiongshi.abupdate.com/v50/vehicle/api/%s/prepare/upgrade"                     //!< <VALUE>    向OTA平台请求同步预约信息的地址
#define ADM_CFG_VUS_OTA_SRV_TASK_VERIFY_URL_DEFAULT_VAL       "http://xiongshi.abupdate.com/v50/vehicle/api/%s/task/verify"                     //!< <VALUE>    向OTA平台请求同步预约信息的地址
#define ADM_CFG_VUS_OTA_SRV_REQUEST_UPLOAD_URL_DEFAULT_VAL    "http://xiongshi.abupdate.com/v50/vehicle/api/%s/file/tempUploadUrl"

#define ADM_CFG_VUS_CDN_SRV_SSL_VERIFY_PEER_DEFAULT_VAL             0
#define ADM_CFG_VUS_CDN_CLIENT_CERT_PATH_DEFAULT_VAL                "/mnt/uhab_fs/update/fota/etc/cdnClient.crt"
#define ADM_CFG_VUS_CDN_CLIENT_PRIVATE_KEY_PATH_DEFAULT_VAL         "/mnt/uhab_fs/update/fota/etc/cdnClient.key"

#define ADM_CFG_VUS_PKI_URL_DEFAULT_VAL  "https://pkipgw-uat.cheryinternational.com:6443/cn"    //PKI请求默认地址

/*** 第二套云端环境 **/
#define ADM_CFG_VUS_OTA_SRV_BUSINESS_PROTOCOL_TYPE_2    ADM_CFG_OWNER_VUS":ota_server_business_protocol_type_2"        //!< <KEY>  1:http; 2:https; 3:mqtt; 4. mqtts 业务通信接口的协议类型 1:http; 2:https; 3:mqtt; 4. mqtts
#define ADM_CFG_VUS_OTA_SRV_DOWNLOAD_PROTOCOL_TYPE_2    ADM_CFG_OWNER_VUS":ota_server_file_download_protocol_type_2"      //!< <KEY>  文件下载的协议类型 1:http; 2:https
#define ADM_CFG_VUS_OTA_SRV_UPLOAD_PROTOCOL_TYPE_2      ADM_CFG_OWNER_VUS":ota_server_file_upload_protocol_type_2"      //!< <KEY>  文件上传的协议类型 1:http; 2:https
#define ADM_CFG_VUS_OTA_SRV_DEFAULT_SECRET_2            ADM_CFG_OWNER_VUS":ota_server_default_secret_2"   //!< <KEY> 车云通信使用的默认Secret
#define ADM_CFG_VUS_OTA_SRV_REGISTER_URL_2              ADM_CFG_OWNER_VUS":ota_server_register_url_2"         //!< <KEY>  向OTA平台请求注册的地址
#define ADM_CFG_VUS_OTA_SRV_CHECK_URL_2                 ADM_CFG_OWNER_VUS":ota_server_check_url_2"            //!< <KEY>  向OTA平台请求检测的地址
#define ADM_CFG_VUS_OTA_SRV_GET_CONFIG_URL_2            ADM_CFG_OWNER_VUS":ota_server_get_config_url_2"       //!< <KEY>  向OTA平台请求获取配置的地址
#define ADM_CFG_VUS_OTA_SRV_GET_ICP_URL_2            ADM_CFG_OWNER_VUS":ota_server_get_icp_url_2"       //!< <KEY>  向OTA平台请求获取配置的地址

#define ADM_CFG_VUS_OTA_SRV_REPORT_VEHICLE_INFO_URL_2   ADM_CFG_OWNER_VUS":ota_server_report_vehicle_info_url_2"  //!< 向OTA平台请求上报车辆信息的地址
#define ADM_CFG_VUS_OTA_SRV_VEHICLE_STATE_REPORT_URL_2  ADM_CFG_OWNER_VUS":ota_server_vehicle_state_report_url_2" //!< <KEY>  向OTA平台请求上报车辆状态的地址
#define ADM_CFG_VUS_OTA_SRV_ECU_STATE_REPORT_URL_2      ADM_CFG_OWNER_VUS":ota_server_ecu_state_report_url_2"     //!< <KEY>  向OTA平台请求上报ECU状态的地址
#define ADM_CFG_VUS_OTA_SRV_PROGRESS_REPORT_URL_2       ADM_CFG_OWNER_VUS":ota_server_progress_report_url_2"      //!< <KEY>  向OTA平台请求上报进度信息的地址
#define ADM_CFG_VUS_OTA_SRV_EVENT_REPORT_URL_2          ADM_CFG_OWNER_VUS":ota_server_event_report_url_2"         //!< <KEY>  向OTA平台请求上报事件的地址
#define ADM_CFG_VUS_OTA_SRV_FILE_UPLOAD_URL_2           ADM_CFG_OWNER_VUS":ota_server_file_upload_url_2"          //!< <KEY>  向OTA平台请求上传日志的地址
#define ADM_CFG_VUS_OTA_SRV_PRE_CHECK_URL_2             ADM_CFG_OWNER_VUS":ota_server_pre_check_url_2"            //!< <KEY>  向OTA平台请求预检测的地址
#define ADM_CFG_VUS_OTA_SRV_GET_HISTORY_URL_2           ADM_CFG_OWNER_VUS":ota_server_get_history_url_2"          //!< <KEY>  向OTA平台请求获取升级历史记录的地址
#define ADM_CFG_VUS_OTA_SRV_SYNC_SCHDULE_URL_2          ADM_CFG_OWNER_VUS":ota_server_sync_schdule_url_2"            //!< <KEY>  向OTA平台请求同步预约信息的地址
#define ADM_CFG_VUS_OTA_SRV_TASK_VERIFY_URL_2           ADM_CFG_OWNER_VUS":ota_server_task_verify_url_2"            //!< <KEY>  向OTA平台进行任务有效性校验的地址
#define ADM_CFG_VUS_OTA_SRV_REQUEST_UPLOAD_URL_2    ADM_CFG_OWNER_VUS":ota_server_request_upload_url_2"            //!< <KEY>  向OTA平台请求文件上传url的地址

#define ADM_CFG_VUS_OTA_SRV_SSL_VERIFY_PEER_2           ADM_CFG_OWNER_VUS":ota_server_soa_ssl_verify_client_2"              //车云交互 是否进行双向认证 N:单向认证 Y:双向认证 
#define ADM_CFG_VUS_CLIENT_CERT_PATH_2                  ADM_CFG_OWNER_VUS":ota_server_soa_ssl_client_certfile_2"  //车云交互 车端(客户端)证书路径
#define ADM_CFG_VUS_CLIENT_PRIVATE_KEY_PATH_2           ADM_CFG_OWNER_VUS":ota_server_soa_ssl_client_keyfile_2"   //车云交互 车端(客户端)私钥路径

#define ADM_CFG_VUS_CDN_SRV_SSL_VERIFY_PEER_2             ADM_CFG_OWNER_VUS":ota_server_soa_ssl_download_verify_client_2"           //车cdn交互 是否进行双向认证 N:单向认证 Y:双向认证 
#define ADM_CFG_VUS_CDN_CLIENT_CERT_PATH_2                    ADM_CFG_OWNER_VUS":ota_server_soa_cdn_ssl_client_certfile_2"  //车cdn交互 车端(客户端)证书路径
#define ADM_CFG_VUS_CDN_CLIENT_PRIVATE_KEY_PATH_2             ADM_CFG_OWNER_VUS":ota_server_soa_cdn_ssl_client_keyfile_2"   //车cdn交互 车端(客户端)私钥路径

#define ADM_CFG_VUS_PKI_URL_2  ADM_CFG_OWNER_VUS":pki_server_url_2"    //PKI请求地址

#define ADM_CFG_VUS_OTA_SRV_ALLOW_MUL_AUTHEN_DEFAULT_VAL_2              0
#define ADM_CFG_VUS_CDN_SRV_ALLOW_MUL_AUTHEN_DEFAULT_VAL_2              0
#define ADM_CFG_VUS_OTA_SRV_BUSINESS_PROTOCOL_TYPE_DEFAULT_VAL_2        1      //!< <VALUE>    1:http; 2:https; 3:mqtt; 4. mqtts
#define ADM_CFG_VUS_OTA_SRV_DOWNLOAD_PROTOCOL_TYPE_DEFAULT_VAL_2        1      //!< <VALUE>  文件下载的协议类型 1:http; 2:https
#define ADM_CFG_VUS_OTA_SRV_UPLOAD_PROTOCOL_TYPE_DEFAULT_VAL_2          1      //!< <VALUE>  文件上传的协议类型 1:http; 2:https
#define ADM_CFG_VUS_OTA_SRV_DEFAULT_SECRET_DEFAULT_VAL_2                "39cabdfaab8c4da09bd6e9823c527836"                                                  //!< <VALUE> 车云通信使用的默认Secret 
#define ADM_CFG_VUS_OTA_SRV_REGISTER_URL_DEFAULT_VAL_2                  "http://votav5-api.abupdate.com/v50/vehicle/api/register"                     //!< <VALUE>    向OTA平台请求注册的地址
#define ADM_CFG_VUS_OTA_SRV_CHECK_URL_DEFAULT_VAL_2                     "http://votav5-api.abupdate.com/v50/vehicle/api/%s/check"                     //!< <VALUE>    向OTA平台请求检测的地址
#define ADM_CFG_VUS_OTA_SRV_GET_CONFIG_URL_DEFAULT_VAL_2                "http://votav5-api.abupdate.com/v50/vehicle/api/%s/client/config"             //!< <VALUE>    向OTA平台请求获取配置的地址
#define ADM_CFG_VUS_OTA_SRV_GET_ICP_URL_DEFAULT_VAL_2                  "http://votav5-api.abupdate.com/v50/vehicle/api/%s/queryIcpRecord"     //!< <VALUE>    向OTA平台请求车辆ICP信息的地址

#define ADM_CFG_VUS_OTA_SRV_REPORT_VEHICLE_INFO_URL_DEFAULT_VAL_2       "http://votav5-api.abupdate.com/v50/vehicle/api/%s/report/vehicle/config"     //!< <VALUE>    向OTA平台请求上报车辆信息的地址
#define ADM_CFG_VUS_OTA_SRV_VEHICLE_STATE_REPORT_URL_DEFAULT_VAL_2      "http://votav5-api.abupdate.com/v50/vehicle/api/%s/state/report"  //!< <VALUE>    向OTA平台请求上报车辆状态的地址
#define ADM_CFG_VUS_OTA_SRV_ECU_STATE_REPORT_URL_DEFAULT_VAL_2          "http://votav5-api.abupdate.com/v50/vehicle/api/%s/ecu/state/report"     //!< <VALUE> 向OTA平台请求上报ECU状态的地址
#define ADM_CFG_VUS_OTA_SRV_PROGRESS_REPORT_URL_DEFAULT_VAL_2           "http://votav5-api.abupdate.com/v50/vehicle/api/%s/progress/report"      //!< <VALUE> 向OTA平台请求上报进度信息的地址
#define ADM_CFG_VUS_OTA_SRV_EVENT_REPORT_URL_DEFAULT_VAL_2              "http://votav5-api.abupdate.com/v50/vehicle/api/%s/event/report"         //!< <VALUE> 向OTA平台请求上报事件的地址
#define ADM_CFG_VUS_OTA_SRV_FILE_UPLOAD_URL_DEFAULT_VAL_2               "http://votav5-api.abupdate.com/v50/vehicle/api/%s/file/upload"          //!< <VALUE> 向OTA平台请求上传日志的地址
#define ADM_CFG_VUS_OTA_SRV_PRE_CHECK_URL_DEFAULT_VAL_2                 "http://votav5-api.abupdate.com/v50/vehicle/api/%s/pre/check"            //!< <VALUE> 向OTA平台请求预检测的地址
#define ADM_CFG_VUS_OTA_SRV_GET_HISTORY_DEFAULT_VAL_2                   "http://votav5-api.abupdate.com/v50/vehicle/api/%s/upgrade/history"      //!< <VALUE> 向OTA平台请求查询升级历史记录的地址
#define ADM_CFG_VUS_OTA_SRV_SYNC_SCHDULE_URL_DEFAULT_VAL_2              "http://votav5-api.abupdate.com/v50/vehicle/api/%s/prepare/upgrade"                     //!< <VALUE>    向OTA平台请求同步预约信息的地址
#define ADM_CFG_VUS_OTA_SRV_TASK_VERIFY_URL_DEFAULT_VAL_2               "http://votav5-api.abupdate.com/v50/vehicle/api/%s/task/verify"                     //!< <VALUE>    向OTA平台请求同步预约信息的地址
#define ADM_CFG_VUS_OTA_SRV_REQUEST_UPLOAD_URL_DEFAULT_VAL_2            "http://votav5-api.abupdate.com/v50/vehicle/api/%s/file/tempUploadUrl"

#define ADM_CFG_VUS_CDN_SRV_SSL_VERIFY_PEER_DEFAULT_VAL_2             0
#define ADM_CFG_VUS_CDN_CLIENT_CERT_PATH_DEFAULT_VAL_2                "/mnt/uhab_fs/update/fota/etc/cdnClient.crt"
#define ADM_CFG_VUS_CDN_CLIENT_PRIVATE_KEY_PATH_DEFAULT_VAL_2         "/mnt/uhab_fs/update/fota/etc/cdnClient.key"

#define ADM_CFG_VUS_PKI_URL_DEFAULT_VAL_2  "https://pkipgw-uat.cheryinternational.com:6443/cn"    //PKI请求默认地址

#define ADM_CFG_VUS_OTA_SRV_ALLOW_MUL_AUTHEN_DEFAULT_VAL_2              0
#define ADM_CFG_VUS_OTA_SRV_BUSINESS_PROTOCOL_TYPE_DEFAULT_VAL_2        1      //!< <VALUE>    1:http; 2:https; 3:mqtt; 4. mqtts
#define ADM_CFG_VUS_OTA_SRV_DOWNLOAD_PROTOCOL_TYPE_DEFAULT_VAL_2        1      //!< <VALUE>  文件下载的协议类型 1:http; 2:https
#define ADM_CFG_VUS_OTA_SRV_UPLOAD_PROTOCOL_TYPE_DEFAULT_VAL_2          1      //!< <VALUE>  文件上传的协议类型 1:http; 2:https
#define ADM_CFG_VUS_OTA_SRV_DEFAULT_SECRET_DEFAULT_VAL_2                "39cabdfaab8c4da09bd6e9823c527836"                                                  //!< <VALUE> 车云通信使用的默认Secret 
#define ADM_CFG_VUS_OTA_SRV_REGISTER_URL_DEFAULT_VAL_2                  "http://votav5-api.abupdate.com/v50/vehicle/api/register"                     //!< <VALUE>    向OTA平台请求注册的地址
#define ADM_CFG_VUS_OTA_SRV_CHECK_URL_DEFAULT_VAL_2                     "http://votav5-api.abupdate.com/v50/vehicle/api/%s/check"                     //!< <VALUE>    向OTA平台请求检测的地址
#define ADM_CFG_VUS_OTA_SRV_GET_CONFIG_URL_DEFAULT_VAL_2                "http://votav5-api.abupdate.com/v50/vehicle/api/%s/client/config"             //!< <VALUE>    向OTA平台请求获取配置的地址
#define ADM_CFG_VUS_OTA_SRV_GET_ICP_URL_DEFAULT_VAL_2                  "http://votav5-api.abupdate.com/v50/vehicle/api/%s/queryIcpRecord"     //!< <VALUE>    向OTA平台请求车辆ICP信息的地址

#define ADM_CFG_VUS_OTA_SRV_REPORT_VEHICLE_INFO_URL_DEFAULT_VAL_2       "http://votav5-api.abupdate.com/v50/vehicle/api/%s/report/vehicle/config"     //!< <VALUE>    向OTA平台请求上报车辆信息的地址
#define ADM_CFG_VUS_OTA_SRV_VEHICLE_STATE_REPORT_URL_DEFAULT_VAL_2      "http://votav5-api.abupdate.com/v50/vehicle/api/%s/state/report"  //!< <VALUE>    向OTA平台请求上报车辆状态的地址
#define ADM_CFG_VUS_OTA_SRV_ECU_STATE_REPORT_URL_DEFAULT_VAL_2          "http://votav5-api.abupdate.com/v50/vehicle/api/%s/ecu/state/report"     //!< <VALUE> 向OTA平台请求上报ECU状态的地址
#define ADM_CFG_VUS_OTA_SRV_PROGRESS_REPORT_URL_DEFAULT_VAL_2           "http://votav5-api.abupdate.com/v50/vehicle/api/%s/progress/report"      //!< <VALUE> 向OTA平台请求上报进度信息的地址
#define ADM_CFG_VUS_OTA_SRV_EVENT_REPORT_URL_DEFAULT_VAL_2              "http://votav5-api.abupdate.com/v50/vehicle/api/%s/event/report"         //!< <VALUE> 向OTA平台请求上报事件的地址
#define ADM_CFG_VUS_OTA_SRV_FILE_UPLOAD_URL_DEFAULT_VAL_2               "http://votav5-api.abupdate.com/v50/vehicle/api/%s/file/upload"          //!< <VALUE> 向OTA平台请求上传日志的地址
#define ADM_CFG_VUS_OTA_SRV_PRE_CHECK_URL_DEFAULT_VAL_2                 "http://votav5-api.abupdate.com/v50/vehicle/api/%s/pre/check"            //!< <VALUE> 向OTA平台请求预检测的地址
#define ADM_CFG_VUS_OTA_SRV_GET_HISTORY_DEFAULT_VAL_2                   "http://votav5-api.abupdate.com/v50/vehicle/api/%s/upgrade/history"      //!< <VALUE> 向OTA平台请求查询升级历史记录的地址

#define ADM_CFG_VUS_OTA_SRV_MQTT_IP_OR_DOMAIN                   ADM_CFG_OWNER_VUS":ota_server_mqtt_ip_or_domain"            //!< <KEY>  与平台使用MQTT协议连接的地址/域名
#define ADM_CFG_VUS_OTA_SRV_MQTT_PORT                           ADM_CFG_OWNER_VUS":ota_server_mqtt_port"                    //!< <KEY>  与平台使用MQTT协议连接的端口
#define ADM_CFG_VUS_OTA_SRV_MQTT_USER                           ADM_CFG_OWNER_VUS":ota_server_mqtt_user"                    //!< <KEY>  与平台使用MQTT协议认证的用户名
#define ADM_CFG_VUS_OTA_SRV_MQTT_PASSWD                         ADM_CFG_OWNER_VUS":ota_server_mqtt_passwd"                  //!< <KEY>  与平台使用MQTT协议认证的密码
#define ADM_CFG_VUS_OTA_SRV_MQTT_API_NAME_REGISTER              ADM_CFG_OWNER_VUS":ota_server_mqtt_api_name_register"       //!< <KEY>  向OTA平台请求注册，使用的API Name
#define ADM_CFG_VUS_OTA_SRV_MQTT_API_NAME_GET_CONFIG            ADM_CFG_OWNER_VUS":ota_server_mqtt_api_name_get_config"     //!< <KEY>  向OTA平台请求获取车端配置，使用的API Name
#define ADM_CFG_VUS_OTA_SRV_MQTT_API_NAME_REPORT_VEHICLE_INFO   ADM_CFG_OWNER_VUS":ota_server_mqtt_api_name_report_vehicle_info"    //!< <KEY>  向OTA平台上报汽车端信息，使用的API Name
#define ADM_CFG_VUS_OTA_SRV_MQTT_API_NAME_CHECK                 ADM_CFG_OWNER_VUS":ota_server_mqtt_api_name_check"                  //!< <KEY>  向OTA平台请求版本检测，使用的API Name
#define ADM_CFG_VUS_OTA_SRV_MQTT_API_NAME_REPORT_VEHICLE_STATE  ADM_CFG_OWNER_VUS":ota_server_mqtt_api_name_vehicle_state_report"   //!< <KEY>  向OTA平台上报车辆状态，使用的API Name
#define ADM_CFG_VUS_OTA_SRV_MQTT_API_NAME_REPORT_ECU_STATE      ADM_CFG_OWNER_VUS":ota_server_mqtt_api_name_ecu_state_report"       //!< <KEY>  向OTA平台上报ECU状态，使用的API Name
#define ADM_CFG_VUS_OTA_SRV_MQTT_API_NAME_REPORT_PROGRESS       ADM_CFG_OWNER_VUS":ota_server_mqtt_api_name_progress_report"        //!< <KEY>  向OTA平台上报进度信息，使用的API Name
#define ADM_CFG_VUS_OTA_SRV_MQTT_API_NAME_REPORT_EVENT          ADM_CFG_OWNER_VUS":ota_server_mqtt_api_name_event_report"           //!< <KEY>  向OTA平台上报事件信息，使用的API Name
#define ADM_CFG_VUS_OTA_SRV_MQTT_API_NAME_PRE_CHECK             ADM_CFG_OWNER_VUS":ota_server_mqtt_api_name_pre_check"              //!< <KEY>  向OTA平台请求预检测的， 使用的API Name
#define ADM_CFG_VUS_OTA_SRV_MQTT_API_NAME_GET_HISTORY           ADM_CFG_OWNER_VUS":ota_server_mqtt_api_name_get_history"            //!< <KEY>  向OTA平台请求获取升级历史记录，使用的API Name

#define ADM_CFG_VUS_OTA_SRV_MQTT_IP_OR_DOMAIN_DEFAULT_VAL                   "113.40.5.62"                   //!< <VALUE>    与平台使用MQTT协议连接的地址/域名
#define ADM_CFG_VUS_OTA_SRV_MQTT_PORT_DEFAULT_VAL                           60123                           //!< <VALUE>    与平台使用MQTT协议连接的端口
#define ADM_CFG_VUS_OTA_SRV_MQTT_USER_DEFAULT_VAL                           "ABUP_EVG"                      //!< <VALUE>    与平台使用MQTT协议认证的用户名
#define ADM_CFG_VUS_OTA_SRV_MQTT_PASSWD_DEFAULT_VAL                         "Abup@123456"                   //!< <VALUE>    与平台使用MQTT协议认证的密码
#define ADM_CFG_VUS_OTA_SRV_MQTT_API_NAME_REGISTER_DEFAULT_VAL              "/v50/vehicle/api/register"                         //!< <VALUE>  向OTA平台请求注册，使用的API Name
#define ADM_CFG_VUS_OTA_SRV_MQTT_API_NAME_GET_CONFIG_DEFAULT_VAL            "/v50/vehicle/api/%s/client/config"         //!< <VALUE>  向OTA平台请求获取车端配置，使用的API Name
#define ADM_CFG_VUS_OTA_SRV_MQTT_API_NAME_REPORT_VEHICLE_INFO_DEFAULT_VAL   "/v50/vehicle/api/%s/report/vehicle/config" //!< <VALUE>  向OTA平台上报汽车端信息，使用的API Name
#define ADM_CFG_VUS_OTA_SRV_MQTT_API_NAME_CHECK_DEFAULT_VAL                 "/v50/vehicle/api/%s/check"                 //!< <VALUE>  向OTA平台请求版本检测，使用的API Name
#define ADM_CFG_VUS_OTA_SRV_MQTT_API_NAME_REPORT_VEHICLE_STATE_DEFAULT_VAL  "/v50/vehicle/api/%s/state/report"          //!< <VALUE>  向OTA平台上报车辆状态，使用的API Name
#define ADM_CFG_VUS_OTA_SRV_MQTT_API_NAME_REPORT_ECU_STATE_DEFAULT_VAL      "/v50/vehicle/api/%s/ecu/state/report"      //!< <VALUE>  向OTA平台上报ECU状态，使用的API Name
#define ADM_CFG_VUS_OTA_SRV_MQTT_API_NAME_REPORT_PROGRESS_DEFAULT_VAL       "/v50/vehicle/api/%s/progress/report"       //!< <VALUE>  向OTA平台上报进度信息，使用的API Name
#define ADM_CFG_VUS_OTA_SRV_MQTT_API_NAME_REPORT_EVENT_DEFAULT_VAL          "/v50/vehicle/api/%s/event/report"          //!< <VALUE>  向OTA平台上报事件信息，使用的API Name
#define ADM_CFG_VUS_OTA_SRV_MQTT_API_NAME_PRE_CHECK_DEFAULT_VAL             "/v50/vehicle/api/%s/pre/check"             //!< <VALUE> 向OTA平台请求预检测，使用的API Name
#define ADM_CFG_VUS_OTA_SRV_MQTT_API_NAME_GET_HISTORY_DEFAULT_VAL           "/v50/vehicle/api/%s/upgrade/history"       //!< <VALUE> 向OTA平台请求查询升级历史记录，使用的API Name

#define ADM_CFG_VUS_EVENT_REPORT_LEVEL                          ADM_CFG_OWNER_VUS":event_report_level"                  //!< <KEY>  事件上报的LEVEL级别定义
#define ADM_CFG_VUS_EVENT_REPORT_LEVEL_DEFAULT_VAL              3                                                       //!< <VALUE>    事件上报的LEVEL级别，默认定义

#define ADM_CFG_VUS_DIM_IMPL_TYPE								ADM_CFG_OWNER_VUS":dim_impl_type"
#define ADM_CFG_VUS_SOA_IMPL_TYPE							ADM_CFG_OWNER_VUS":soa_impl_type"

#define ADM_CFG_VUS_SOA_IMPL_LIB_PATH							ADM_CFG_OWNER_VUS":soa_impl_lib_path"
#define ADM_CFG_VUS_DIM_IMPL_LIB_PATH							ADM_CFG_OWNER_VUS":dim_impl_lib_path"
#define ADM_CFG_VUS_UIM_IMPL_LIB_PATH							ADM_CFG_OWNER_VUS":uim_impl_lib_path"

/* 并行数量限制M和N配置名 */
#define ADM_CFG_VUS_UPGRADE_PARALLEL_LIMIT_M                    ADM_CFG_OWNER_VUS ":upgrade_parallel_num_limit_m"
#define ADM_CFG_VUS_UPGRADE_PARALLEL_LIMIT_N                    ADM_CFG_OWNER_VUS ":upgrade_parallel_num_limit_n"

#define ADM_CFG_VUS_UPGRADE_PARALLEL_LIMIT_M_DEFAULT_VAL        3
#define ADM_CFG_VUS_UPGRADE_PARALLEL_LIMIT_N_DEFAULT_VAL        1

#define ADM_CFG_VUS_EIM_KEY_FIELD_COLLECT_RETRY_MAX              ADM_CFG_OWNER_VUS":eim_collect_retry_max"
#define ADM_CFG_VUS_EIM_KEY_FIELD_COLLECT_RETRY_MAX_DEFAULT_VAL  0

#define ADM_CFG_VUS_HMI_INSTANCEID								ADM_CFG_OWNER_VUS":hmi_instanceid"
#define ADM_CFG_VUS_UIM_INSTANCEID								ADM_CFG_OWNER_VUS":uim_instanceid"
#define ADM_CFG_VUS_VDI_INSTANCEID								ADM_CFG_OWNER_VUS":vdi_instanceid"
#define ADM_CFG_VUS_UCL_INSTANCEID								ADM_CFG_OWNER_VUS":ucl_instanceid"
#define ADM_CFG_VUS_HOST_INSTANCEID								ADM_CFG_OWNER_VUS":host_instanceid"
#define ADM_CFG_VUS_TRANS_INSTANCEID							ADM_CFG_OWNER_VUS":trans_instanceid"
#define ADM_CFG_VUS_DPO_INSTANCEID								ADM_CFG_OWNER_VUS":dpo_instanceid"

#define ADM_CFG_VUS_IS_ALLOW_IDEL_TRANS                         ADM_CFG_OWNER_VUS":is_allow_idel_trans"
#define ADM_CFG_VUS_IDEL_TRNAS_INSTANCEID                       ADM_CFG_OWNER_VUS":idel_trans_instanceid"

#define ADM_CFG_VUS_APC_APPID                                   ADM_CFG_OWNER_VUS":ucm_appid"           ///<tracer-ucm
#define ADM_CFG_VUS_DO_IP_SERVER                                ADM_CFG_OWNER_VUS":do_ip_server"
#define ADM_CFG_VUS_DO_IP_SERVER_E01                            ADM_CFG_OWNER_VUS":do_ip_server_e01"
#define ADM_CFG_VUS_DO_IP_SERVER_E02                            ADM_CFG_OWNER_VUS":do_ip_server_e02"
#define ADM_CFG_VUS_VEHICLE_TYPE                                ADM_CFG_OWNER_VUS":vehicle_type"

#define ADM_CFG_VUS_HMI_INSTANCEID_DEFAULT_VAL					2
#define ADM_CFG_VUS_UIM_INSTANCEID_DEFAULT_VAL					1		// 0x01, dim client
#define ADM_CFG_VUS_VDI_INSTANCEID_DEFAULT_VAL					4097	// 0x1001
#define ADM_CFG_VUS_UCL_INSTANCEID_DEFAULT_VAL					8193	// 0x2001
#define ADM_CFG_VUS_HOST_INSTANCEID_DEFAULT_VAL					12289	// 0x3001
#define ADM_CFG_VUS_TRANS_INSTANCEID_DEFAULT_VAL				16385	// 0x4001
#define ADM_CFG_VUS_DPO_INSTANCEID_DEFAULT_VAL					20481	// 0x5001

#define ADM_CFG_VUS_IS_ALLOW_IDEL_TRANS_DEFAULT_VAL             0       //不允许闲时传输
#define ADM_CFG_VUS_IDEL_TRNAS_INSTANCEID_DEFAULT_VAL           0       //默认闲时传输的实例ID为0

#define ADM_CFG_VUS_DIM_IMPL_TYPE_DEFAULT_VAL					"3"										//!< <VALUE>    dim的实现方式, 参考ADM_DIM_IMPL_TYPE
#define ADM_CFG_VUS_SOA_IMPL_TYPE_DEFAULT_VAL					2										//!< <VALUE>    dim的实现方式, 参考ADM_DIM_IMPL_TYPE
#define ADM_CFG_VUS_DIM_IMPL_LIB_PATH_DEFAULT_VAL 			"/tmp/vus/lib/libdisprotosoc.so"			//!< <VALUE>    dim的实现方式
#define ADM_CFG_VUS_SOA_IMPL_LIB_PATH_DEFAULT_VAL 			"/tmp/vus/lib/libsoasoc.so"			//!< <VALUE>    dim的实现方式
#define ADM_CFG_VUS_UIM_IMPL_LIB_PATH_DEFAULT_VAL 			"/tmp/vus/lib/libuisjsonsoc.so"			//!< <VALUE>    dim的实现方式

#define ADM_CFG_VUS_APC_APPID_DEFAULT_VAL  "UCM"                ///<tarcer-ucm default value
#define ADM_CFG_VUS_DO_IP_SERVER_DEFAULT_VAL "1890,1868,1984"


#define ADM_CFG_OWNER_VRPC      "VRPC"      //!< <OWNER>    VRPC
#define ADM_CFG_VRPC_SOMEIP_UNICAST_IP               ADM_CFG_OWNER_VRPC":rpc_someip_unicast_ip"           //!< <KEY>  配置someip使用的路由
#define ADM_CFG_VRPC_SOMEIP_ROUTE_ADD                ADM_CFG_OWNER_VRPC":rpc_someip_route_add"            //!< <KEY>  SOA服务单播地址
#define ADM_CFG_VRPC_SOMEIP_LOG_PATH_NAME            ADM_CFG_OWNER_VRPC":rpc_someip_log_path_name"        //!< <KEY>    RPC的日志路径
#define ADM_CFG_VRPC_SOMEIP_DATA_PATH_NAME           ADM_CFG_OWNER_VRPC":rpc_someip_data_path_name"       //!< <KEY>    RPC data的路径
#define ADM_CFG_VRPC_SOMEIP_RUNTIME_DIR              ADM_CFG_OWNER_VRPC":rpc_someip_runtime_dir"          //!< <KEY>    RPC runtime 路径

#define ADM_CFG_VRPC_SOMEIP_UNICAST_IP_DEFAULT_VAL               "192.168.0.6"                                               //!< <VALUE>    配置someip使用的路由
#define ADM_CFG_VRPC_SOMEIP_ROUTE_ADD_DEFAULT_VAL                "route add -n 255.255.255.255 gw 192.168.0.6 dev bridge0" //!< <VALUE>    SOA服务单播地址
#define ADM_CFG_VRPC_SOMEIP_LOG_PATH_NAME_DEFAULT_VAL            "/tmp/admota/xpc/log/vrpc.log"                                    //!< <VALUE>    RPC的日志路径
#define ADM_CFG_VRPC_SOMEIP_DATA_PATH_NAME_DEFAULT_VAL           "/tmp/admota/xpc/log/vrpc.data"                                   //!< <VALUE>    RPC data的路径
#define ADM_CFG_VRPC_SOMEIP_RUNTIME_DIR_DEFAULT_VAL              "/tmp/admota/xpc/admnv/tmp/"                                                    //!< <VALUE>    RPC runtime 路径


#define ADM_CFG_OWNER_DIS_SOC	"DIS_SOC"	//!< <OWNER>	DIS_SOC
#define ADM_CFG_VUS_DIS_TCP_MODE							ADM_CFG_OWNER_DIS_SOC":soa_soc_tcp_mode"           //!< <KEY>  soa_socket 启动模式, client or server or client&server
#define ADM_CFG_VUS_DIS_TCP_SERVER_IP						ADM_CFG_OWNER_DIS_SOC":soa_tcp_server_ip"           //!< <KEY>  soa_socket 服务地址
#define ADM_CFG_VUS_DIS_DETECT_IP                           ADM_CFG_OWNER_DIS_SOC ":soa_detect_ip"          //!< <KEY>  网络诊断地址
#define ADM_CFG_VUS_DIS_TCP_SERVER_PORT						ADM_CFG_OWNER_DIS_SOC":soa_tcp_server_port"         //!< <KEY>  soa_socket 服务端口
#define ADM_CFG_VUS_DIS_TCP_HOST_IP							ADM_CFG_OWNER_DIS_SOC":soa_tcp_host_ip"			//!< <KEY>	 自身IP
#define ADM_CFG_VUS_DIS_TCP_HOST_PORT                       ADM_CFG_OWNER_DIS_SOC":soa_tcp_host_port"       //!< <KEY>	 自身端口
#define ADM_CFG_VUS_DIS_SOC_HOST_INSTANCEID					ADM_CFG_OWNER_DIS_SOC":soa_soc_host_instance_id"			//!< <KEY>	SOC自身实例ID
#define ADM_CFG_VUS_DIS_SOC_APP_INSTANCEID					ADM_CFG_OWNER_DIS_SOC":soa_soc_app_instance_id"			//!< <KEY>	 主控实例ID
#define ADM_CFG_VUS_DIS_SOC_TRY_CONNECT_TIME				ADM_CFG_OWNER_DIS_SOC":soa_soc_try_connect_time"		//!< <KEY> 掉线重连,重试次数大于0 或 -1一直重试
#define ADM_CFG_VUS_DIS_SOC_TRY_CONNECT_INTERVAL			ADM_CFG_OWNER_DIS_SOC":soa_soc_try_connect_interval"	//!< <KEY> 重连时间间隔
#define ADM_CFG_VUS_DIS_SOC_HEART_COUNT						ADM_CFG_OWNER_DIS_SOC":soa_soc_heart_count"		//!< <KEY> 最大次数内无心跳, 进入掉线重连阶段
#define ADM_CFG_VUS_DIS_SOC_HEART_INTERVAL					ADM_CFG_OWNER_DIS_SOC":soa_soc_heart_interval"	//!< <KEY> 心跳时间间隔
#define ADM_CFG_VUS_DIS_SOC_WITH_SSL                        ADM_CFG_OWNER_DIS_SOC":soa_with_ssl"	//!< <KEY> soa_socket是否开启SSL功能,TRUE(1):Y，FALSE(0):Y
#define ADM_CFG_VUS_DIS_SOC_SSL_CAFILE                      ADM_CFG_OWNER_DIS_SOC":soa_ssl_cafile"	//!< <KEY> soa_socket服务端根证书路径
#define ADM_CFG_VUS_DIS_SOC_SSL_CERTFILE                    ADM_CFG_OWNER_DIS_SOC":soa_ssl_certfile"	//!< <KEY> soa_socket服务端证书路径
#define ADM_CFG_VUS_DIS_SOC_SSL_KEYFILE                     ADM_CFG_OWNER_DIS_SOC":soa_ssl_keyfile"	//!< <KEY> soa_socket服务端证书私钥路径
#define ADM_CFG_VUS_DIS_SOC_SSL_CIPHERS                     ADM_CFG_OWNER_DIS_SOC":soa_ssl_ciphers"	//!< <KEY> soa_socket SSL算法套件选择
#define ADM_CFG_VUS_DIS_SOC_SSL_PROTOCOLS                   ADM_CFG_OWNER_DIS_SOC":soa_ssl_protocols"	//!< <KEY> soa_socket SSL协议版本选择
#define ADM_CFG_VUS_DIS_SOC_SSL_VERIFY                      ADM_CFG_OWNER_DIS_SOC":soa_ssl_verify_client"	//!< <KEY> soa_socket 是否打开双向认证功能,TRUE(1):Y，FALSE(0):N
#define ADM_CFG_VUS_DIS_SOC_SSL_CLIENT_CAFILE               ADM_CFG_OWNER_DIS_SOC":soa_ssl_client_cafile"	//!< <KEY> soa_socket 客户端跟证书路径
#define ADM_CFG_VUS_DIS_SOC_SSL_CLIENT_CERTFILE             ADM_CFG_OWNER_DIS_SOC":soa_ssl_client_certfile"	//!< <KEY> soa_socket 客户端证书路径
#define ADM_CFG_VUS_DIS_SOC_SSL_CLIENT_KEYFILE              ADM_CFG_OWNER_DIS_SOC":soa_ssl_client_keyfile"	//!< <KEY> soa_socket 客户端私钥路径
#define ADM_CFG_VUS_DIS_MIN_THREAD_NUM						ADM_CFG_OWNER_DIS_SOC":soa_soc_min_thread_num"         //!< <KEY>  soa_socket 开启最小线程数
#define ADM_CFG_VUS_DIS_MAX_THREAD_NUM						ADM_CFG_OWNER_DIS_SOC":soa_soc_max_thread_num"         //!< <KEY>  soa_socket 开启最大线程数

#define ADM_CFG_VUS_DIS_TCP_MODE_DEFAULT_VAL					1           					//!< <VALUE>  soa_socket 启动模式, client or server or client&server
#define ADM_CFG_VUS_DIS_TCP_SERVER_IP_DEFAULT_VAL				"192.168.0.6"           		//!< <VALUE>  soa_socket 服务地址
#define ADM_CFG_VUS_DIS_TCP_SERVER_PORT_DEFAULT_VAL				12345           				//!< <VALUE>   soa_socket 服务端口
#define ADM_CFG_VUS_DIS_DETECT_IP_DEFAULT_VAL                   "127.0.0.1"                  //!< <VALUE>  网络诊断地址
#define ADM_CFG_VUS_DIS_TCP_HOST_IP_DEFAULT_VAL					"192.168.0.6"					//!< <VALUE>	 自身IP
#define ADM_CFG_VUS_DIS_TCP_HOST_PORT_DEFAULT_VAL               12346                           //!< <VALUE>	 自身端口
#define ADM_CFG_VUS_DIS_SOC_HOST_INSTANCEID_DEFAULT_VAL			2								//!< <VALUE>	 SOC自身实例ID
#define ADM_CFG_VUS_DIS_SOC_APP_INSTANCEID_DEFAULT_VAL			1								//!< <VALUE>	 主控实例ID
#define ADM_CFG_VUS_DIS_SOC_TRY_CONNECT_TIME_DEFAULT_VAL		3								//!< <VALUE> 掉线重连,重试次数大于0 或 -1一直重试
#define ADM_CFG_VUS_DIS_SOC_TRY_CONNECT_INTERVAL_DEFAULT_VAL	60								//!< <VALUE> 重连时间间隔
#define ADM_CFG_VUS_DIS_SOC_HEART_COUNT_DEFAULT_VAL		3								//!< <VALUE> 最大次数内无心跳, 进入掉线重连阶段
#define ADM_CFG_VUS_DIS_SOC_HEART_INTERVAL_DEFAULT_VAL	60								//!< <VALUE> 心跳时间间隔
#define ADM_CFG_VUS_DIS_SOC_WITH_SSL_DEFAULT_VAL             0                                       //!< <	VALUE> soa_socket是否开启SSL功能,TRUE(1):Y，FALSE(0):Y
#define ADM_CFG_VUS_DIS_SOC_SSL_CAFILE_DEFAULT_VAL           "/tmp/admota/ucm/cert/ca.crt"           //!< <	VALUE> soa_socket服务端根证书路径
#define ADM_CFG_VUS_DIS_SOC_SSL_CERTFILE_DEFAULT_VAL         "/tmp/admota/ucm/cert/server_cert.crt"         //!< <	VALUE> soa_socket服务端证书路径
#define ADM_CFG_VUS_DIS_SOC_SSL_KEYFILE_DEFAULT_VAL          "/tmp/admota/ucm/cert/server_key.pem"          //!< <	VALUE> soa_socket服务端证书私钥路径
#define ADM_CFG_VUS_DIS_SOC_SSL_CIPHERS_DEFAULT_VAL           "ECDHE-RSA-AES128-GCM-SHA256"                 //!< <	VALUE> soa_socket SSL算法套件选择
#define ADM_CFG_VUS_DIS_SOC_SSL_PROTOCOLS_DEFAULT_VAL         "TLSv1.2"                                     //!< <	VALUE> soa_socket SSL协议版本选择
#define ADM_CFG_VUS_DIS_SOC_SSL_VERIFY_DEFAULT_VAL            0                                             //!< <	VALUE> soa_socket 是否打开双向认证功能,TRUE(1):Y，FALSE(0):N
#define ADM_CFG_VUS_DIS_SOC_SSL_CLIENT_CAFILE_DEFAULT_VAL     "/tmp/admota/ucm/cert/client_ca.crt"          //!< <	VALUE> soa_socket 客户端跟证书路径
#define ADM_CFG_VUS_DIS_SOC_SSL_CLIENT_CERTFILE_DEFAULT_VAL   "/tmp/admota/ucm/cert/client_cert.crt"        //!< <	VALUE> soa_socket 客户端证书路径
#define ADM_CFG_VUS_DIS_SOC_SSL_CLIENT_KEYFILE_DEFAULT_VAL    "/tmp/admota/ucm/cert/client_key.pem"         //!< <	VALUE> soa_socket 客户端私钥路径
#define ADM_CFG_VUS_DIS_MIN_THREAD_NUM_DEFAULT_VAL		1        //!< <KEY>  soa_socket 开启最小线程数默认值
#define ADM_CFG_VUS_DIS_MAX_THREAD_NUM_DEFAULT_VAL		6		 //!< <KEY>  soa_socket 开启最大线程数默认值		ADM_CFG_OWNER_DIS_SOC":soa_soc_max_thread_num"         //!< <KEY>  soa_socket 开启最大线程数默认值
/*************************************************************VUS主控程序配置信息********************************************************************/

/*************************************************************EWEB配置信息**************************************************************************/
#define ADM_CFG_OWNER_EWEB      "EWEB"      //!< <OWNER>    EWEB
#define ADM_CFG_EWEB_DOC_ROOT_DIR       ADM_CFG_OWNER_EWEB":document_root_dir"          //!< <KEY>  Ewebserver使用的主目录
#define ADM_CFG_EWEB_DOC_UPLOAD_DIR     ADM_CFG_OWNER_EWEB":document_upload_dir"        //!< <KEY>  Ewebserver使用的文件上传目录
#define ADM_CFG_EWEB_DOC_DOWNLOAD_DIR   ADM_CFG_OWNER_EWEB":document_download_dir"      //!< <KEY>  Ewebserver使用文件的下载目录
#define ADM_CFG_EWEB_LOG_STORAGE_PATH   ADM_CFG_OWNER_EWEB":log_storage_path"           //!< <KEY>  Ewebserver存储日志的路径
#define ADM_CFG_EWEB_IS_HTTPS           ADM_CFG_OWNER_EWEB":is_https"                   //!< <KEY>  Ewebserver是否为HTTPS协议
#define ADM_CFG_EWEB_LISTENING_IP       ADM_CFG_OWNER_EWEB":listening_ip"                //!< <KEY>  Ewebserver监听的IP
#define ADM_CFG_EWEB_LISTENING_PORTS    ADM_CFG_OWNER_EWEB":listening_ports"            //!< <KEY>  Ewebserver监听的端口
#define ADM_CFG_EWEB_ENABLE_KEEP_ALIVE  ADM_CFG_OWNER_EWEB":enable_keep_alive"          //!< <KEY>  保持长链接状态
#define ADM_CFG_EWEB_SSL_CERT_PATH      ADM_CFG_OWNER_EWEB":eweb_ssl_certificate"       //!< <KEY>  Ewebserver使用的证书路径,该文件包含私钥
#define ADM_CFG_EWEB_SSL_CA_PATH        ADM_CFG_OWNER_EWEB":eweb_ca_certificate"        //!< <KEY>  Ewebserver使用的CA证书路径
#define ADM_CFG_EWEB_SSL_VERIFY_PEER    ADM_CFG_OWNER_EWEB":eweb_verify_peer"           //!< <KEY>  Ewebserver是否使用双向认证, FALSE(0) 单向认证 TRUE(1) 双向认证
#define ADM_CFG_EWEB_MUN_THREADES       ADM_CFG_OWNER_EWEB":num_threads"                //!< <KEY>  启动线程的个数
#define ADM_CFG_EWEB_REQ_TIMEOUT_MS     ADM_CFG_OWNER_EWEB":request_timeout_ms"         //!< <KEY>  请求的超时时间，单位：ms
#define ADM_CFG_EWEB_MAX_CONNECTIONS    ADM_CFG_OWNER_EWEB":max_connections"            //!< <KEY>  最大的连接数
#define ADM_CFG_EWEB_INSECURE           ADM_CFG_OWNER_EWEB":insecure"                   //!< <KEY>  是否安全
#define ADM_CFG_EWEB_PREFIX_URL         ADM_CFG_OWNER_EWEB":eweb_prefix_url"            //!< <KEY>  Ewebserver链接的前缀
#define ADM_CFG_EWEB_ADDRESS_HEADER     ADM_CFG_OWNER_EWEB":eweb_address_header"        //!< <KEY>  Ewebserver地址头
#define ADM_CFG_EWEB_FILE_DOWN_PROTOCOL_TYPE    ADM_CFG_OWNER_EWEB":eweb_file_download_protocol_type"   //!< <KEY>  Ewebserver文件下载的协议类型
#define ADM_CFG_EWEB_FILE_UPLOAD_PROTOCOL_TYPE  ADM_CFG_OWNER_EWEB":eweb_file_upload_protocol_type"     //!< <KEY>  Ewebserver文件上传的协议类型

#define ADM_CFG_EWEB_SSL_SERVER_CA_PATH                  ADM_CFG_OWNER_EWEB":eweb_ssl_ca"               //Ewebserver服务端证书路径
#define ADM_CFG_EWEB_CLIENT_CERT_PATH                    ADM_CFG_OWNER_EWEB":eweb_ssl_client_certfile"  //车云交互 车端(客户端)证书路径
#define ADM_CFG_EWEB_CLIENT_PRIVATE_KEY_PATH             ADM_CFG_OWNER_EWEB":eweb_ssl_client_keyfile"   //车云交互 车端(客户端)私钥路径

#define ADM_CFG_EWEB_USB_ROOT_DIR       ADM_CFG_OWNER_EWEB":usb_root_dir"          //!< <KEY>  USB存储的根目录
#define ADM_CFG_EWEB_USB_DOWNLOAD_DIR   ADM_CFG_OWNER_EWEB":usb_download_dir"      //!< <KEY>  USB存储的下载目录

#define ADM_CFG_EWEB_USB_ROOT_DIR_DEFAULT_VAL       "/storage/udisk2-p0/"               //!< <VALUE>  USB存储的根目录
#define ADM_CFG_EWEB_USB_DOWNLOAD_DIR_DEFAULT_VAL   "/storage/udisk2-p0/download"       //!< <VALUE>  USB存储的下载目录

#define ADM_CFG_EWEB_DOC_ROOT_DIR_DEFAULT_VAL       "/tmp/admota/"                   //!< <VALUE>    Ewebserver使用的主目录
#define ADM_CFG_EWEB_DOC_UPLOAD_DIR_DEFAULT_VAL     "/tmp/admota/ucm/trans/upload/"            //!< <VALUE>    Ewebserver使用的文件上传目录
#define ADM_CFG_EWEB_DOC_DOWNLOAD_DIR_DEFAULT_VAL   "/tmp/admota/ucm/trans/download/"          //!< <VALUE>    Ewebserver使用文件的下载目录
#define ADM_CFG_EWEB_LOG_STORAGE_PATH_DEFAULT_VAL   "/tmp/admota/ucm/log/ewebsrvlog.txt"       //!< <VALUE>    Ewebserver存储日志的路径
#define ADM_CFG_EWEB_IS_HTTPS_DEFAULT_VAL           0                                   //!< <VALUE>    Ewebserver是否为HTTPS协议
#define ADM_CFG_EWEB_LISTENING_IP_DEFAULT_VAL       "192.168.0.6"                       //!< <VALUE>    Ewebserver监听的IP
#define ADM_CFG_EWEB_LISTENING_PORTS_DEFAULT_VAL    8082                                //!< <VALUE>    Ewebserver监听的端口
#define ADM_CFG_EWEB_ENABLE_KEEP_ALIVE_DEFAULT_VAL  0                                  //!< <VALUE>    保持长链接状态
#define ADM_CFG_EWEB_SSL_CERT_PATH_DEFAULT_VAL      "/tmp/admota/ucm/cert/trans.crt"    //!< <VALUE>    Ewebserver使用的证书路径
#define ADM_CFG_EWEB_SSL_CA_PATH_DEFAULT_VAL        "/tmp/admota/ucm/cert/ca.crt"       //!< <VALUE>    Ewebserver使用的CA证书路径
#define ADM_CFG_EWEB_SSL_CLI_CERT_PATH_DEFAULT_VAL        "/tmp/admota/ucm/cert/client_cert.crt"       //!< <VALUE>    Ewebserver使用客户端证书路径
#define ADM_CFG_EWEB_SSL_CLI_PRI_KEY_PATH_DEFAULT_VAL        "/tmp/admota/ucm/cert/client_key.pem"       //!< <VALUE>    Ewebserver使用客户端私钥路径

#define ADM_CFG_VUS_CLI_CDN_CERT_PATH_DEFAULT_VAL           "/tmp/admota/ucm/cert/cdnClient.crt"
#define ADM_CFG_VUS_CLI_CDN_PRIVATE_KEY_PATH_DEFAULT_VAL    "/tmp/admota/ucm/cert/cdnClient.key"

#define ADM_CFG_EWEB_SSL_VERIFY_PEER_DEFAULT_VAL    0                                   //!< <VALUE>    Ewebserver默认使用单向认证, FALSE(0) 单向认证 TRUE(1) 双向认证
#define ADM_CFG_EWEB_MUN_THREADES_DEFAULT_VAL       50                                  //!< <VALUE>    启动线程的个数
#define ADM_CFG_EWEB_REQ_TIMEOUT_MS_DEFAULT_VAL     30000                               //!< <VALUE>    请求的超时时间，单位：ms
#define ADM_CFG_EWEB_MAX_CONNECTIONS_DEFAULT_VAL    100                                 //!< <VALUE>    最大的连接数
#define ADM_CFG_EWEB_INSECURE_DEFAULT_VAL           100                                 //!< <VALUE>    是否安全
#define ADM_CFG_EWEB_PREFIX_URL_DEFAULT_VAL         "http://192.168.0.6:8082/download/" //!< <VALUE>    Ewebserver链接的前缀
#define ADM_CFG_EWEB_ADDRESS_HEADER_DEFAULT_VAL     "http://192.168.0.6:8082/"          //!< <VALUE>    Ewebserver地址头
#define ADM_CFG_EWEB_FILE_DOWN_PROTOCOL_TYPE_DEFAULT_VAL    1                           //!< <VALUE>  Ewebserver文件下载的协议类型
#define ADM_CFG_EWEB_FILE_UPLOAD_PROTOCOL_TYPE_DEFAULT_VAL  1                           //!< <VALUE>  Ewebserver文件上传的协议类型
/*************************************************************EWEB配置信息**************************************************************************/

/*************************************************************SYSI配置信息**************************************************************************/
#define ADM_CFG_OWNER_SYSI      "SYSI"          //!< <OWNER>    SYSI
#define ADM_CFG_SYSI_RUNTIME_DEPENDENCY_DIR     ADM_CFG_OWNER_SYSI":runtime_dependency_dir"     //!< <KEY>  SYSI程序运行依赖目录
#define ADM_CFG_SYSI_RUNTIME_ROOT_DIR           ADM_CFG_OWNER_SYSI":runtime_root_dir"           //!< <KEY>  SYSI程序运行主目录
#define ADM_CFG_SYSI_KV_STORAGE_DIR             ADM_CFG_OWNER_SYSI":kv_storage_dir"             //!< <KEY>  SYSI程序，kv持久存储目录
#define ADM_CFG_SYSI_PKG_STORAGE_DIR            ADM_CFG_OWNER_SYSI":pkg_storage_dir"            //!< <KEY>  SYSI程序升级包下载目录
#define ADM_CFG_SYSI_LOG_STORAGE_DIR            ADM_CFG_OWNER_SYSI":log_storage_dir"                     //!< <KEY>  日志使用的目录
#define ADM_CFG_SYSI_LOG_COLLECT_DIR            ADM_CFG_OWNER_SYSI":log_collect_dir"                     //!< <KEY>  日志收集存储目录
#define ADM_CFG_SYSI_LOG_PACK_DIR               ADM_CFG_OWNER_SYSI":log_pack_dir"                        //!< <KEY>  被收集日志打包后的存储目录
#define ADM_CFG_SYSI_CERT_STORAGE_DIR           ADM_CFG_OWNER_SYSI":cert_storage_dir"           //!< <KEY>  SYSI程序证书存储目录
#define ADM_CFG_SYSI_UNZIP_STORAGE_DIR          ADM_CFG_OWNER_SYSI":unzip_storage_dir"         //!< <KEY>  SYSI程序解压信息存储目录
#define ADM_CFG_SYSI_PARALLEL_COLLECT_STYLE_DIR   ADM_CFG_OWNER_SYSI":parallel_collect_style_dir"                   //!< <KEY>  零件并行采集方式记录
#define ADM_CFG_SYSI_UPGRADE_RESULT_RECORD_DIR   ADM_CFG_OWNER_SYSI":upgrade_result_record"                   //!< <KEY>  升级结果信息存储目录
#define ADM_CFG_SYSI_IS_CONTROL_ACTIVE_DIR      ADM_CFG_OWNER_SYSI ":is_control_active"                    //!< <KEY>  是否控制激活存储目录
#define ADM_CFG_SYSI_HMI_LOG_STORAGE_DIR        ADM_CFG_OWNER_SYSI":hmi_log_storage_dir"       //!< <KEY>  hmi日志存储目录
#define ADM_CFG_SYSI_VDI_LOG_STORAGE_DIR        ADM_CFG_OWNER_SYSI":vdi_log_storage_dir"       //!< <KEY>  vdi日志存储目录

#define ADM_CFG_SYSI_LOG_MAX_SIZE               ADM_CFG_OWNER_SYSI":log_max_size_of_single_file"    //!< <KEY>  单个日志文件存储的大小，单位：Byte
#define ADM_CFG_SYSI_LOG_MAX_ROTATE             ADM_CFG_OWNER_SYSI":log_max_rotate_file"            //!< <KEY>  最多存储的文件个数
#define ADM_CFG_SYSI_LOG_OUTPUT_TERMINAL        ADM_CFG_OWNER_SYSI":log_output_terminal"            //!< <KEY>  是否输出log至串口终端


#define ADM_CFG_SYSI_COMM_CERT_PATH             ADM_CFG_OWNER_SYSI":comm_cert_path"             //!< <KEY>  SYSI程序与OTA平台通信的证书路径
#define ADM_CFG_SYSI_CDN_CERT_PATH              ADM_CFG_OWNER_SYSI":cdn_cert_path"              //!< <KEY>  SYSI程序与CDN平台通信的证书路径
#define ADM_CFG_SYSI_TRANS_CERT_PATH            ADM_CFG_OWNER_SYSI":trans_cert_path"            //!< <KEY>  SYSI程序与传输服务通信的证书路径
#define ADM_CFG_SYSI_RPC_COMM_CERT_PATH         ADM_CFG_OWNER_SYSI":rpc_comm_cert_path"         //!< <KEY>  SYSI从控程序与VUS主控程序通信使用的证书路径

#define ADM_CFG_UCM_SHARED_DIR         ADM_CFG_OWNER_SYSI":ucm_shared_dir"         //!<  vdi和主控共享目录,主控目录
#define ADM_CFG_VDI_SHARED_DIR         ADM_CFG_OWNER_SYSI":vdi_shared_dir"         //!<  vdi和主控共享目录,vdi目录


#define ADM_CFG_VUS_SYSI_APPID                  ADM_CFG_OWNER_VUS":sysi_appid"                  ///<tracer-sysi


#define ADM_CFG_SYSI_RUNTIME_DEPENDENCY_DIR_DEFAULT_VAL "/data_fota/update/fota/abupota/"   //!< <VALUE>    SYSI程序运行依赖目录
#define ADM_CFG_SYSI_RUNTIME_ROOT_DIR_DEFAULT_VAL   "/tmp/admota/lite/"                                //!< <VALUE>    SYSI程序运行主目录
#define ADM_CFG_SYSI_KV_STORAGE_DIR_DEFAULT_VAL     "/tmp/admota/lite/kvstorage"                       //!< <VALUE>    SYSI程序，kv持久存储目录
#define ADM_CFG_SYSI_PKG_STORAGE_DIR_DEFAULT_VAL    "/tmp/admota/lite/package/"                        //!< <VALUE>    SYSI程序升级包下载目录
#define ADM_CFG_SYSI_LOG_STORAGE_DIR_DEFAULT_VAL    "/tmp/admota/lite/log/storage/"                     //!< <VALUE>    日志使用目录的默认值
#define ADM_CFG_SYSI_LOG_COLLECT_DIR_DEFAULT_VAL    "/tmp/admota/lite/log/collect/"                     //!< <VALUE>    日志收集存储目录
#define ADM_CFG_SYSI_LOG_PACK_DIR_DEFAULT_VAL       "/tmp/admota/lite/log/pack/"                        //!< <VALUE>    被收集日志打包后的存储目录
#define ADM_CFG_SYSI_CERT_STORAGE_DIR_DEFAULT_VAL   "/tmp/admota/lite/cert/"                           //!< <VALUE>    SYSI程序证书存储目录
#define ADM_CFG_SYSI_UNZIP_STORAGE_DIR_DEFAULT_VAL  "/tmp/admota/lite/unzip"                           //!< <VALUE>    SYSI程序解压信息存储目录
#define ADM_CFG_SYSI_PARALLEL_COLLECT_STYLE_DIR_DEFAULT_VAL   "/tmp/admota/lite/parallelCollect"                            //!< <VALUE>    并行采集方式记录目录的默认值
#define ADM_CFG_SYSI_UPGRADE_RESULT_RECORD_DIR_DEFAULT_VAL   "/tmp/admota/lite/resultRecord"                            //!< <VALUE>    升级结果信息存储目录默认值
#define ADM_CFG_SYSI_IS_CONTROL_ACTIVE_DIR_DEFAULT_VAL "/tmp/admota/lite/controlActive"                              //!< <VALUE>    升级结果信息存储目录默认值
#define ADM_CFG_SYSI_LOG_MAX_SIZE_DEFAULT_VAL       1045504                                     //!< <VALUE>    单个日志文件存储的大小，单位：Byte
#define ADM_CFG_SYSI_LOG_MAX_ROTATE_DEFAULT_VAL     5                                           //!< <VALUE>    最多存储的文件个数
#define ADM_CFG_SYSI_LOG_OUTPUT_TERMINAL_DEFAULT_VAL 1                                           //!< <VALUE>   默认输出log至串口终端


#define ADM_CFG_SYSI_COMM_CERT_PATH_DEFAULT_VAL     "/tmp/admota/lite/cert/comm.crt"                        //!< <VALUE>    SYSI程序与OTA平台通信的证书路径
#define ADM_CFG_SYSI_CDN_CERT_PATH_DEFAULT_VAL      "/tmp/admota/lite/cert/cdn.crt"                         //!< <VALUE>    SYSI程序与CDN平台通信的证书路径
#define ADM_CFG_SYSI_TRANS_CERT_PATH_DEFAULT_VAL    "/tmp/admota/lite/cert/trans.crt"                       //!< <VALUE>    SYSI程序与传输服务通信的证书路径
#define ADM_CFG_SYSI_RPC_COMM_CERT_PATH_DEFAULT_VAL "/tmp/admota/lite/cert/rpccomm.cert"                //!< <VALUE>    SYSI从控程序与VUS主控程序通信使用的证书路径

#define ADM_CFG_VUS_SYSI_APPID_DEFAULT_VAL  "SYSI"                ///<tarcer-sysi default value
/*************************************************************SYSI配置信息**************************************************************************/


#define ADM_CFG_OWNER_EMS      "EMS"          //!< <OWNER>    EMS

#define ADM_CFG_EMS_LOG_FILE_FULL_DIR                  ADM_CFG_OWNER_EMS":log_get_file_full_dir"
#define ADM_CFG_EMS_SERVER_FLAG_FILE_DIR               ADM_CFG_OWNER_EMS":ota_server_flag_file_dir"
#define ADM_CFG_EMS_SERVER_FLAG_FILE_NAME              ADM_CFG_OWNER_EMS":ota_server_flag_file_name"
#define ADM_CFG_EMS_PKI_CERT_STORAGE_DIR               ADM_CFG_OWNER_EMS":ota_pki_cert_storage_dir"


#define ADM_CFG_EMS_LOG_FILE_FULL_DIR_DEFAULT_VAL      "/mnt/uhab_fs/update/fota/abupota/ucm/log/pack/"
#define ADM_CFG_EMS_SERVER_FLAG_FILE_DIR_DEFAULT_VAL   "/mnt/uhab_fs/update/fota/abupota/ucm/flag/"
#define ADM_CFG_EMS_SERVER_FLAG_FILE_NAME_DEFAULT_VAL  "otaServerType"
#define ADM_CFG_EMS_PKI_CERT_STORAGE_DIR_DEFAULT_VAL   "/mnt/uhab_fs/update/fota/abupota/ucm/cert/"



#ifdef __cplusplus  // close out "C" linkage in case of c++ compiling
}
#endif

#endif//__ADM_CFG_DEFAULT_H__
