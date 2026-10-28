#ifndef _ADM_OCA_HTTP_ERROR_CODE_H_
#define _ADM_OCA_HTTP_ERROR_CODE_H_

#include "adm_typedefs.h"
#include "curl/curl.h"
#include "adm_oca_connect.h"
#include "adm_oca_httpConnect.h"
// #include "adm_oca_httpRequest.h"

#ifdef __cplusplus
extern "C" {
#endif

#define EXPORT_FUN __attribute__((visibility("default")))
/**
 * @brief OCM模块错误码定义
 * @note  暂定错误码，后续移植到错误码总表中
 * 
*/
#define ADM_OCA_HTTP_ECODE_FAILED                        -1          // 失败，默认初始化的返回值
#define ADM_OCA_HTTP_ECODE_SUCCESSED                     0           // 成功
#define ADM_OCA_HTTP_ECODE_INVALID_PARAM                 1           // 无效的参数
#define ADM_OCA_HTTP_ECODE_INVALID_POINTER               2           // 无效的指针
#define ADM_OCA_HTTP_ECODE_INVALID_STATE               	3           // 无效的指针
#define ADM_OCA_HTTP_ECODE_A_TO_D32U_FAILED              4           // 字符串装换为无符号整型失败
#define ADM_OCA_HTTP_ECODE_BASE64_ENCODE_FAILED          5           /*!< Base64格式转码失败 */
#define ADM_OCA_HTTP_ECODE_BASE64_DECODE_FAILED          6           /*!< Base64格式解码失败 */
#define ADM_OCA_HTTP_ECODE_AES_ECB_ENCRYPT_FAILED        7           /*!< AES_ECB加密失败    */
#define ADM_OCA_HTTP_ECODE_AES_ECB_DECRYPT_FAILED        8           /*!< AES_ECB解密失败    */
#define ADM_OCA_HTTP_ECODE_AES_CBC_ENCRYPT_FAILED        9           /*!< AES_CBC加密失败    */
#define ADM_OCA_HTTP_ECODE_AES_CBC_DECRYPT_FAILED        10          /*!< AES_CBC解密失败    */
#define ADM_OCA_HTTP_ECODE_HMAC_SHA224_FAILED            11          /*!< hmac_sha224计算失败 */
#define ADM_OCA_HTTP_ECODE_HMAC_SHA256_FAILED            12          /*!< hmac_sha256计算失败 */
#define ADM_OCA_HTTP_ECODE_HMAC_SHA384_FAILED            13          /*!< hmac_sha384计算失败 */
#define ADM_OCA_HTTP_ECODE_HMAC_SHA512_FAILED            14          /*!< hmac_sha512计算失败 */
#define ADM_OCA_HTTP_ECODE_RSA2048_PUB_KEY_ENC_FAILED    15          /*!< RSA2048公钥加密失败 */
#define ADM_OCA_HTTP_ECODE_RSA2048_PUB_KEY_DEC_FAILED    16          /*!< RSA2048公钥解密失败 */
#define ADM_OCA_HTTP_ECODE_RSA2048_PRI_KEY_ENC_FAILED    18          /*!< RSA2048私钥加密失败 */
#define ADM_OCA_HTTP_ECODE_RSA2048_PRI_KEY_DEC_FAILED    19          /*!< RSA2048私钥解密失败 */

#define ADM_OCA_HTTP_ECODE_CURL_INIT_FAILED              101        //!< CURL句柄初始化失败
#define ADM_OCA_HTTP_ECODE_CURL_SET_ERR                  102        //!< CURL句柄参数设置失败
#define ADM_OCA_HTTP_ECODE_NO_MEMORY                     103        //!< 内存不足
#define ADM_OCA_HTTP_ECODE_URL_MALFORMAT                 104        //!< URL的格式不正确
#define ADM_OCA_HTTP_ECODE_COULDNT_RESOLVE_HOST          105        //!< 无法解析主机
#define ADM_OCA_HTTP_ECODE_COULDNT_CONNECT               106        //!< 连接的主机或代理失败
#define ADM_OCA_HTTP_ECODE_HTTP_RETURNED_ERROR           107        //!< 如果CURLOPT_FAILONERROR设置为TRUE并返回错误代码> = 400，则返回此值。
#define ADM_OCA_HTTP_ECODE_WRITE_FILE_FAIL               108        //!< 将接收的数据写入本地文件时发生错误，或者从写入回调将错误返回给libcurl。
#define ADM_OCA_HTTP_ECODE_UPLOAD_FAILED                 109        //!< 无法启动上传。对于FTP，服务器通常会拒绝STOR命令。错误缓冲区通常包含服务器的解释。
#define ADM_OCA_HTTP_ECODE_READ_FILE_FAIL                110        //!< 读取本地文件或读回读返回的错误有问题。
#define ADM_OCA_HTTP_ECODE_OPERATION_TIMEDOUT            111        //!< 操作超时 根据条件达到指定的超时期限。
#define ADM_OCA_HTTP_ECODE_RANGE_ERROR                   112        //!< 服务器不支持或接受范围请求。
#define ADM_OCA_HTTP_ECODE_HTTP_POST_ERROR               113        //!< 这是一个奇怪的错误，主要是由于内部的混乱造成的。
#define ADM_OCA_HTTP_ECODE_SSL_CONNECT_ERROR             114        //!< SSL / TLS握手中的某个地方出现问题。你真的想要错误缓冲区，并在那里读取消息，因为它更明确地指出了问题。可以是证书（文件格式，路径，权限），密码等。
#define ADM_OCA_HTTP_ECODE_BAD_DOWNLOAD_RESUME           115        //!< 由于指定的偏移超出文件边界，所以无法恢复下载。
#define ADM_OCA_HTTP_ECODE_ABORTED_BY_CALLBACK           116        //!< 通过回调中止。回调返回到libcurl“abort”。
#define ADM_OCA_HTTP_ECODE_TOO_MANY_REDIRECTS            117        //!< 重定向太多 当以下重定向时，libcurl命中最大数量。用CURLOPT_MAXREDIRS设置你的限制。
#define ADM_OCA_HTTP_ECODE_PEER_FAILED_VERIFICATION      118        //!< 远程服务器的SSL证书或SSH md5指纹被认为不正常。
#define ADM_OCA_HTTP_ECODE_GOT_NOTHING                   119        //!< 没有从服务器返回任何东西，在这种情况下，没有任何东西被认为是错误。
#define ADM_OCA_HTTP_ECODE_SSL_ENGINE_NOTFOUND           120        //!< 未找到指定的加密引擎。
#define ADM_OCA_HTTP_ECODE_SSL_ENGINE_SETFAILED          121        //!< 默认设置所选的SSL加密引擎失败！
#define ADM_OCA_HTTP_ECODE_SEND_ERROR                    122        //!< 发送网络数据失败
#define ADM_OCA_HTTP_ECODE_RECV_ERROR                    123        //!< 接收网络数据失败。
#define ADM_OCA_HTTP_ECODE_SSL_CERTPROBLEM               124        //!< 本地客户端证书出现问题。
#define ADM_OCA_HTTP_ECODE_SSL_CIPHER                    125        //!< 无法使用指定的密码。
#define ADM_OCA_HTTP_ECODE_SSL_CACERT                    126        //!< 对等证书无法通过已知的CA证书进行身份验证。
#define ADM_OCA_HTTP_ECODE_BAD_CONTENT_ENCODING          127        //!< 无法识别的传输编码。
#define ADM_OCA_HTTP_ECODE_FILESIZE_EXCEEDED             128        //!< 超过最大文件大小。
#define ADM_OCA_HTTP_ECODE_SSL_ENGINE_INITFAILED         129        //!< 启动SSL引擎失败。
#define ADM_OCA_HTTP_ECODE_LOGIN_DENIED                  130        //!< 远程服务器拒绝卷曲登录（7.13.1中添加）
#define ADM_OCA_HTTP_ECODE_REMOTE_DISK_FULL              131        //!< 超出服务器上的磁盘空间。
#define ADM_OCA_HTTP_ECODE_SSL_CACERT_BADFILE            132        //!< 阅读SSL CA证书（路径？访问权限？）的问题？
#define ADM_OCA_HTTP_ECODE_REMOTE_FILE_NOT_FOUND         133        //!< URL中引用的资源不存在。

#define ADM_OCA_HTTP_ECODE_BAD_REQUEST                   140        //!< 400  错误请求 — 请求中有语法问题，或不能满足请求。  
#define ADM_OCA_HTTP_ECODE_UNAUTHORIZED                  141        //!< 401  未授权 — 未授权客户机访问数据。  
#define ADM_OCA_HTTP_ECODE_NEED_PAY                      142        //!< 402  需要付款 — 表示计费系统已有效。  
#define ADM_OCA_HTTP_ECODE_PROHIBIT                      143        //!< 403  禁止 — 即使有授权也不需要访问。  
#define ADM_OCA_HTTP_ECODE_NOT_FOUND                     144        //!< 404  找不到 — 服务器找不到给定的资源；文档不存在。  
#define ADM_OCA_HTTP_ECODE_METHOD_DISABLED               145        //!< 405 （方法禁用） 禁用请求中指定的方法。
#define ADM_OCA_HTTP_ECODE_NOT_ACCEPTED                  146        //!< 406 （不接受） 无法使用请求的内容特性响应请求的网页。
#define ADM_OCA_HTTP_ECODE_PROXY_AUTH_REQUEST            147        //!< 407  代理认证请求 — 客户机首先必须使用代理认证自身。  
#define ADM_OCA_HTTP_ECODE_REQUEST_TIMEOUT               148        //!< 408 （请求超时） 服务器等候请求时发生超时。
#define ADM_OCA_HTTP_ECODE_CONFLICT                      149        //!< 409 （冲突） 服务器在完成请求时发生冲突。服务器必须在响应中包含有关冲突的信息。
#define ADM_OCA_HTTP_ECODE_DELETED                       150        //!< 410 （已删除） 如果请求的资源已永久删除，服务器就会返回此响应。
#define ADM_OCA_HTTP_ECODE_NEED_EFFECTIVE_LENGTH         151        //!< 411 （需要有效长度） 服务器不接受不含有效内容长度标头字段的请求。
#define ADM_OCA_HTTP_ECODE_PREREQUISITES_NOT_MET         152        //!< 412 （未满足前提条件） 服务器未满足请求者在请求中设置的其中一个前提条件。
#define ADM_OCA_HTTP_ECODE_REQUEST_ENTITY_TOO_LARGE      153        //!< 413 （请求实体过大） 服务器无法处理请求，因为请求实体过大，超出服务器的处理能力。
#define ADM_OCA_HTTP_ECODE_REQUEST_URL_TOO_LONG          154        //!< 414 （请求的 URI 过长） 请求的 URI（通常为网址）过长，服务器无法处理。
#define ADM_OCA_HTTP_ECODE_UNSUPPORTED_MEDIA_TYPE        155        //!< 415  介质类型不受支持 — 服务器拒绝服务请求，因为不支持请求实体的格式。
#define ADM_OCA_HTTP_ECODE_REQUEST_SCOPE_NOT_MEET_REQ    156        //!< 416 （请求范围不符合要求） 如果页面无法提供请求的范围，则服务器会返回此状态代码。
#define ADM_OCA_HTTP_ECODE_UNMET_EXPECTATIONS            157        //!< 417 （未满足期望值） 服务器未满足"期望"请求标头字段的要求。

#define ADM_OCA_HTTP_ECODE_INTERNAL_ERROR                160        //!< 500  内部错误 — 因为意外情况，服务器不能完成请求。  
#define ADM_OCA_HTTP_ECODE_NOT_PERFORMED                 161        //!< 501  未执行 — 服务器不支持请求的工具。
#define ADM_OCA_HTTP_ECODE_FAULT_GATEWAY                 162        //!< 502  错误网关 — 服务器接收到来自上游服务器的无效响应。
#define ADM_OCA_HTTP_ECODE_UNABLE_TO_GET_SERVICE         163        //!< 503  无法获得服务 — 由于临时过载或维护，服务器无法处理请求。
#define ADM_OCA_HTTP_ECODE_GATEWAY_TIMEOUT               164        //!< 504 （网关超时） 服务器作为网关或代理，但是没有及时从上游服务器收到请求。
#define ADM_OCA_HTTP_ECODE_UNSUPPORTED_HTTP_VER          165        //!< 505 （HTTP 版本不受支持） 服务器不支持请求中所用的 HTTP 协议版本。
#define ADM_OCA_HTTP_ECODE_NET_UNKNOWN                   170        //!< 未知的网络错误

#define ADM_OCA_HTTP_ECODE_SSL_SERVER_ROOT_CERT          180        //!< 设置服务器证书失败
#define ADM_OCA_HTTP_ECODE_SSL_CLIENT_CERT               181        //!< 设置客户端证书失败
#define ADM_OCA_HTTP_ECODE_SSL_CLIENT_CERT_PASS          182        //!< 设置客户端证书密码失败
#define ADM_OCA_HTTP_ECODE_SSL_CLIENT_CERT_TYPE          183        //!< 设置客户端证书类型失败
#define ADM_OCA_HTTP_ECODE_SSL_CLIENT_PRIVATE_KEY        184        //!< 设置客户端私钥失败
#define ADM_OCA_HTTP_ECODE_SSL_CLIENT_PRIVATE_KEY_PASS   185        //!< 设置客户端私钥密码失败
#define ADM_OCA_HTTP_ECODE_SSL_CLIENT_PRIVATE_KEY_TYPE   186        //!< 设置客户端私钥类型失败


#define ADM_OCA_HTTP_ECODE_CURL_FORMADD_MEMORY           187         /*!< if the FormInfo allocation fails   */
#define ADM_OCA_HTTP_ECODE_CURL_FORMADD_OPTION_TWICE     188         /*!< if one option is given twice for one Form  */
#define ADM_OCA_HTTP_ECODE_CURL_FORMADD_NULL             189         /*!< if a null pointer was given for a char */
#define ADM_OCA_HTTP_ECODE_CURL_FORMADD_UNKNOWN_OPTION   190         /*!< if an unknown option was used  */
#define ADM_OCA_HTTP_ECODE_CURL_FORMADD_INCOMPLETE       191         /*!< if the some FormInfo is not complete (or error) */
#define ADM_OCA_HTTP_ECODE_CURL_FORMADD_ILLEGAL_ARRAY    192         /*!< if an illegal option is used in an array   */
#define ADM_OCA_HTTP_ECODE_CURL_FORMADD_DISABLED         193         /*!< libcurl was built with this disabled */
#define ADM_OCA_HTTP_ECODE_PROTOCOL_TYPE_INVALID         194         /*!< 不支持的协议类型 */

#define ADM_OCA_HTTP_ECODE_REGISTER_VIN_REPLACEMENT      201         // 车辆vin码切换
#define ADM_OCA_HTTP_ECODE_REGISTER_INFO_VIN_EMPTY       202         // 本地存储的注册信息中，vin为空


#define ADM_OCA_HTTP_ECODE_APPLY_MEMORY_FAILED           301         // 内存申请失败

#define ADM_OCA_HTTP_ECODE_NEW_JSON_OBJECT_FAILED        401         // json对象创建失败
#define ADM_OCA_HTTP_ECODE_JSON_TO_STRING_FAILED         402         // json_to_string失败
#define ADM_OCA_HTTP_ECODE_JSON_ADD_INT_FAILED           401         // json对象添加INT对象失败
#define ADM_OCA_HTTP_ECODE_JSON_ADD_INT64_FAILED         402         // json对象添加INT64对象失败
#define ADM_OCA_HTTP_ECODE_JSON_ADD_STRING_FAILED        403         // json对象添加字符串对象失败
#define ADM_OCA_HTTP_ECODE_JSON_ADD_DOUBLE_FAILED        404         // json对象添加double对象失败
#define ADM_OCA_HTTP_ECODE_JSON_ADD_BOOL_FAILED          405         // json对象添加bool对象失败
#define ADM_OCA_HTTP_ECODE_STRING_TO_JSON_FAILED         406         // string_to_json失败
#define ADM_OCA_HTTP_ECODE_JSON_GET_INT_FAILED           407         // json对象获取INT对象失败
#define ADM_OCA_HTTP_ECODE_JSON_GET_INT64_FAILED         408         // json对象获取INT64对象失败
#define ADM_OCA_HTTP_ECODE_JSON_GET_STRING_FAILED        409         // json对象获取字符串对象失败
#define ADM_OCA_HTTP_ECODE_JSON_GET_DOUBLE_FAILED        410         // json对象获取double对象失败
#define ADM_OCA_HTTP_ECODE_JSON_GET_BOOL_FAILED          411         // json对象获取bool对象失败
#define ADM_OCA_HTTP_ECODE_JSON_GET_OBJECT_FAILED        412         // json对象获取子对象失败
#define ADM_OCA_HTTP_ECODE_JSON_GET_ARRAYOBJECT_FAILED   413         // json对象获取子数组对象失败
#define ADM_OCA_HTTP_ECODE_DEL_JSON_OBJECT_FAILED        415         // json对象释放失败
#define ADM_OCA_HTTP_ECODE_JSON_GET_KEY_FAILED           416         /*! json获取key对应值失败 */
#define ADM_OCA_HTTP_ECODE_JSON_KEY_TYPE_ERROR           417         /*! json校验key对应值的类型错误 */
#define ADM_OCA_HTTP_ECODE_JSON_ADD_ARRAY_FAILED         418         // json对象添加数组对象失败
#define ADM_OCA_HTTP_ECODE_JSON_OBJECT_TYPE_NOT_ARRAY    419         // json对象不是数组类型
#define ADM_OCA_HTTP_ECODE_JSON_ADD_SUBOBJECT_FAILED     420         // json对象添加子对象失败

#define ADM_OCA_HTTP_ECODE_FILE_NOT_EXIST                501         /*!< 文件不存在     */
#define ADM_OCA_HTTP_ECODE_OPEN_FILE_FAILED              502         /*!< 打开文件失败   */
#define ADM_OCA_HTTP_ECODE_READ_FILE_FAILED              503         /*!< 读文件失败     */
#define ADM_OCA_HTTP_ECODE_WRITE_FILE_FAILED             504         /*!< 写文件失败     */
#define ADM_OCA_HTTP_ECODE_FP_SEEK_FAILED                505         /*!< 文件指针偏移失败*/
#define ADM_OCA_HTTP_ECODE_CREATE_DIR_FAILED             506         /*!< 目录创建失败   */
#define ADM_OCA_HTTP_ECODE_GET_FILESIZE_FILED            507         /*!< 文件大小获取失败 */

#define ADM_OCA_HTTP_ECODE_PTHREAD_CREATE_FAILED         550         /*!< 线程创建失败   */
#define ADM_OCA_HTTP_ECODE_PTHREAD_START_FAILED          551         /*!< 启动线程失败   */
#define ADM_OCA_HTTP_ECODE_PTHREAD_ALREADY_RUNNING       552         /*!< 线程已经运行 */

#define ADM_OCA_HTTP_ECODE_PAUSE_DOWNLOAD                601         /*!< 暂停下载       */
#define ADM_OCA_HTTP_ECODE_CANCEL_DOWNLOAD               602         /*!< 取消下载       */
#define ADM_OCA_HTTP_ECODE_DOWNLOAD_FAILED               603         /*!< 下载失败       */
#define ADM_OCA_HTTP_ECODE_STATE_NOT_SATISFIED           604         /*!< 状态机不满足    */
#define ADM_OCA_HTTP_ECODE_NET_CHG_PAUSE_DOWNLOAD        607         /** 网络切换导致下载暂停 */
#define ADM_OCA_HTTP_ECODE_COND_ABNORMAL_PAUSE_DOWNLOAD  608         /** 下载条件异常导致下载暂停 */
#define ADM_OCA_HTTP_ECODE_PAUSE_IDEL_TRANSFER           609         /** 暂停闲时传输 */

#define ADM_OCA_HTTP_ECODE_FILE_UPLOAD_HANDLE_INVALID    650         /*!< 文件上传句柄无效 */
#define ADM_OCA_HTTP_ECODE_FILE_DOWNLOAD_HANDLE_INVALID  651         /*!< 文件下载句柄无效 */

#define ADM_OCA_HTTP_ECODE_KV_OPEN_FAILED                701         /*!< KV文件打开失败 */
#define ADM_OCA_HTTP_ECODE_KV_DESTORY_FAILED             702         /*!< KV句柄销毁失败 */
#define ADM_OCA_HTTP_ECODE_KV_CLOSED_FAILED              703         /*!< KV句柄关闭失败 */
#define ADM_OCA_HTTP_ECODE_KV_REMOVE_KEY_FAILED          704         /*!< KV句柄中，key移除失败 */
#define ADM_OCA_HTTP_ECODE_KV_SET_STRING_FAILED          705         /*!< KV句柄，存储失败   */
#define ADM_OCA_HTTP_ECODE_KV_GET_STRING_FAILED          706         /*!< KV句柄，获取失败    */

#define ADM_OCA_HTTP_ECODE_REGISTER_GET_VIN_FAILED               750         /*!< register获取vin码失败 */
#define ADM_OCA_HTTP_ECODE_REGISTER_CREATE_MESSAGE_BODY_FAILED   751         /*!< register车端打包数据错误 */
#define ADM_OCA_HTTP_ECODE_REGISTER_PARSE_MESSAGE_BODY_FAILED    752         /*!< register车端解析云端数据错误 */
#define ADM_OCA_HTTP_ECODE_REGISTER_HTTP_POST_FAILED             753         /*!< register车端http post错误 */
#define ADM_OCA_HTTP_ECODE_REGISTER_MESSAGE_SIGN_FAILED          754         /*!< register车云请求数据签名失败 */
#define ADM_OCA_HTTP_ECODE_REGISTER_RESPMESSAGE_SIGN_FAILED      755         /*!< register车云数据返回数据签名不匹配 */
#define ADM_OCA_HTTP_ECODE_REGISTER_MESSAGE_ENCRYPT_FAILED       756         /*!< register车云数据加密失败 */
#define ADM_OCA_HTTP_ECODE_REGISTER_RESPMESSAGE_DECRYPT_FAILED   757         /*!< register车云数据返回数据解密失败 */

#define ADM_OCA_HTTP_ECODE_OTACONFIG_NEED_UPDATE             770     /*!< 本地存储的OTA配置信息需要更新 */

#define ADM_OCA_HTTP_ECODE_CHECK_STATE_MISMATCH              801         /*!< check状态机时机不对*/
#define ADM_OCA_HTTP_ECODE_CHECK_GET_ECU_CONFIG_FAILED       802         /*!< check获取ECU配置失败*/
#define ADM_OCA_HTTP_ECODE_CHECK_GET_ECU_VERINFO_FAILED      803         /*!< check从VDI获取ECU版本信息失败*/
#define ADM_OCA_HTTP_ECODE_CHECK_CREAT_MESSAGE_BODY_FAILED   804         /*!< check车端打包数据错误*/
#define ADM_OCA_HTTP_ECODE_CHECK_PARSE_MESSAGE_BODY_FAILED   805         /*!< check车端解析云端数据错误*/
#define ADM_OCA_HTTP_ECODE_CHECK_HTTP_POST_FAILED            806         /*!< check车端http post错误*/
#define ADM_OCA_HTTP_ECODE_CHECK_DOWNLOAD_POLICY_FAILED      807         /*!< check车端下载策略文件失败*/
#define ADM_OCA_HTTP_ECODE_CHECK_MESSAGE_SIGN_FAILED         808         /*!< check车云数据请求数据签名失败*/
#define ADM_OCA_HTTP_ECODE_CHECK_RESPMESSAGE_SIGN_FAILED     809         /*!< check车云数据返回数据签名不匹配*/
#define ADM_OCA_HTTP_ECODE_CHECK_ENCRYPT_FAILED              810         /*!< check车云数据签名加密失败*/
#define ADM_OCA_HTTP_ECODE_CHECK_DECRYPT_FAILED              811         /*!< check车云数据签名解密失败*/
#define ADM_OCA_HTTP_ECODE_CHECK_NOT_NEWVERSION              812         /*!< check云端无新版本*/
#define ADM_OCA_HTTP_ECODE_CHECK_PARSE_POLICY_NAME_FAILED    813         /*!< check从URL解析'/'后的字符作为名字出错*/
#define ADM_OCA_HTTP_ECODE_CHECK_MESSAGE_ADD_ECUINFO_FAILED  814         /*!< check车云数据请求数据增加ECUINFO数据出错*/

#define ADM_OCA_HTTP_ECODE_IRS_UPLOADFILEPATHFORMAT_FAILED   850         /*!< irs上报的文件路径格式错误*/
#define ADM_OCA_HTTP_ECODE_IRS_UPLOADFILEPATHRENAME_FAILED   851         /*!< irs上报的文件路径重命名失败*/
#define ADM_OCA_HTTP_ECODE_IRS_KVGETMESSAGEVAL_FAILED        851         /*!< irs通过KV获得messageKey，通过key获取VAL失败*/
#define ADM_OCA_HTTP_ECODE_IRS_UPLOADFILE_NOTEXIST           852         /*!< irs的文件不存在*/

// #define ADM_OCA_HTTP_ECODE_SUCCESSED                1000        /*!< 云端下发状态：成功 */
#define ADM_OCA_HTTP_ECODE_UNKONW_STATUS_CODE          901        /*!< 云端下发状态：状态码不存在 */
#define ADM_OCA_HTTP_ECODE_NO_NEW_VERSION              902        /*!< 云端下发状态：暂无新版本 */
#define ADM_E_VEHICLE_NOT_REGISTER            903        /*!< 云端下发状态：车辆未注册 */
#define ADM_OCA_HTTP_ECODE_VEHICLE_NOT_EXIST           904        /*!< 云端下发状态：车辆不存在 */
#define ADM_OCA_HTTP_ECODE_GENERAL_CONFIG_NEED_UPDATE  905        /*!< 云端下发状态：通用配置信息需要更新 */
#define ADM_OCA_HTTP_ECODE_ECU_CONFIG_NEED_UPDATE      906        /*!< 云端下发状态：零件配置信息需要更新 */
#define ADM_OCA_HTTP_ECODE_CONFIG_INFO_NOT_EXIST       907        /*!< 云端下发状态：配置信息不存在 */
#define ADM_OCA_HTTP_ECODE_PARAM_JSON_FORMAT_FAILED    908        /*!< 云端下发状态：参数Json格式转换异常 */
#define ADM_OCA_HTTP_ECODE_PARAM_VERIFY_FAILED         909        /*!< 云端下发状态：参数校验失败 */
#define ADM_OCA_HTTP_ECODE_SIGN_VERIFY_FAILED          910        /*!< 云端下发状态：签名验证失败 */
#define ADM_OCA_HTTP_ECODE_FILE_SAVED_FAILED           911        /*!< 云端下发状态：文件保存异常 */
#define ADM_OCA_HTTP_ECODE_DECRYPT_VERIFY_FAILED       912        /*!< 云端下发状态：请求报文解密失败 */
#define ADM_OCA_HTTP_ECODE_UPGRADE_TASK_STOPED         913        /*!< 云端下发状态：任务终止 */
#define ADM_OCA_HTTP_ECODE_NO_ROOT_CERT_UPDATE         914        /*!< 云端下发状态：暂无可下发根证书 */
#define ADM_OCA_HTTP_ECODE_SERVER_INTERNAL_EXCEPTION   915        /*!< 云端下发状态：服务器内部异常 */
#define ADM_OCA_HTTP_ECODE_NO_SHORT_TIME_ACCESS        916        /*!< 云端下发状态：禁止短时间内重复访问服务器 */


// TODO: 待归类
#define ADM_OCA_HTTP_ECODE_RWLOCK_INIT_FAILED            950      /*!< 读写锁初始化失败         */
#define ADM_OCA_HTTP_ECODE_PACKAGE_TYPE_FAILED           951      /*!< 升级包类型错误(ecu集合包、ecuPart)            */
#define ADM_OCA_HTTP_ECODE_PACKAGE_NOFIND                952      /*!< 升级包未找到       */
#define ADM_OCA_HTTP_ECODE_VERIFY_SIZE_FAIL              953      /*! 文件大小校验失败 */
#define ADM_OCA_HTTP_ECODE_STORAGE_SPACE_NOT_ENOUGH      954      /*! 存储空间不足 */


#ifdef __cplusplus  // close out "C" linkage in case of c++ compiling
}
#endif

#endif//_ADM_OCA_HTTP_ERROR_CODE_H_
