#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdlib.h>

#include "adm_cfg_log.h"
#include "adm_cfg_api.h"
#include "adm_com_errorCode.h"
#include "adm_string.h"
#include "adm_memory.h"

#define ADM_CONFIG_FILE_PATH    "./sample.ini"

static DBOOL adm_com_isFileExist(const DCHAR* pFilePath)
{
    DBOOL isFileExistOk = FALSE;

    do {
        if (NULL == pFilePath) {
            log_e("Parameter verification failed");
            break;
        }

        if (access(pFilePath, F_OK) != 0) {
            break;
        } else {
            isFileExistOk = TRUE;
        }
    } while(FALSE);

    return isFileExistOk;
}

/*!
 *******************************************************************************
 * 创建文件，若文件对应的目录不存在则会先创建对应的目录.
 *
 * \note eg. [in]/tmp/otaConfig/config.json ---> [out]mkdir("/tmp/otaConfig")
 *
 * \param[in]   pFilePath    文件路径
 * \param[in]   isCleanFile  是否需要清空文件内容
 * \retval      D32S       0: 成功 non-0：失败
 *******************************************************************************
 */
static D32S adm_com_createFileDirAndFile
(
    const DCHAR *pFilePath,
    DBOOL        needCleanFile
)
{
    D32S ret = 0;
    if (NULL == pFilePath)
    {
        log_e( "Parameter verification failed");
        return ADM_ECODE_INVALID_PARAM;
    }

    DCHAR dirName[256] = {0};
    (void)adm_memset(dirName, 0x00, sizeof(dirName), sizeof(dirName));
    (void)adm_strncpy(dirName, pFilePath, adm_strlen(pFilePath), sizeof(dirName));
    D32U i;
    D32U len = adm_strlen(dirName);

    if (0u >= len)
    {
        log_e( "Parameter verification failed");
        return ADM_ECODE_INVALID_PARAM;
    }

    for(i = 1; i < len; i++)
    {
        if(dirName[i] == '/')
        {
            dirName[i] = '\0';
            if(access(dirName, F_OK) != 0)
            {
                if(mkdir(dirName, S_IRWXU|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH) == -1)
                {
                    log_e( "dirName:%s mkdir failed", dirName);
                    return ADM_ECODE_CREATE_DIR_FAILED;
                }
            }
            dirName[i] = '/';
        }
    }

    //!< 若文件不存在，则创建文件
    if (0 != access(dirName, F_OK))
    {
        (void)adm_memset(dirName, 0x00, sizeof(dirName), sizeof(dirName));
        (void)adm_snprintf(dirName, sizeof(dirName), "touch %s", pFilePath);
        (void)system(dirName);
    }
    else
    {
        //!< 存在且需要清空，则先删除文件后创建
        if (needCleanFile)
        {
            ret = remove(dirName);
            if(ret == 0)
            {
                log_i("remove %s success",dirName);
            }
            else 
            {
                log_e("remove %s failed",dirName);
            }
            (void)adm_memset(dirName, 0x00, sizeof(dirName), sizeof(dirName));
            (void)adm_snprintf(dirName, sizeof(dirName), "touch %s", pFilePath);
            (void)system(dirName);
        }
    }

    return ADM_ECODE_SUCCESSED;
}


static D32S adm_cfg_apiWriteFileData(const DCHAR* filePath, const DCHAR* fileData)
{
    D32U size = 0;
    FILE *fp  = NULL;
    D32U writeLen = 0;
    D32S iRet = ADM_ECODE_FAILED;

    if ((NULL == filePath) || (NULL == fileData))
    {
        log_e( "Invalid input param");
        return ADM_ECODE_INVALID_PARAM;
    }

    do {
        /** 判断文件是否存在且可写入，如果存在 */
        if (FALSE == adm_com_isFileExist(filePath)) {
            iRet = adm_com_createFileDirAndFile(filePath, TRUE);
            if (ADM_ECODE_SUCCESSED != iRet) {
                log_e("adm_com_createFileDirAndFile failed, iRet:%d", iRet);
                break;
            }
        }

        fp = fopen(filePath, "wb+");
        if (NULL == fp) {
            iRet = ADM_ECODE_OPEN_FILE_FAILED;
            log_e("fopen %s failed, iRet:%d", filePath, iRet);
            break;
        }
        size = adm_strlen(fileData);

        writeLen = fwrite(fileData, 1, size, fp);
        if (writeLen == size) {
            iRet = ADM_ECODE_SUCCESSED;
            break;
        } else {
            ADM_ECODE_WRITE_FILE_FAILED;
        }
    } while(FALSE);

    if (NULL != fp) {
        (void)fclose(fp);
    }

    return iRet;
}

static DBOOL adm_cfg_apiIsFileExist(const DCHAR* pFilePath)
{
    DBOOL isFileExistOk = FALSE;

    do {
        if (NULL == pFilePath) {
            log_e("Parameter verification failed");
            break;
        }

        if (access(pFilePath, F_OK) != 0) {
            break;
        } else {
            isFileExistOk = TRUE;
        }
    } while(FALSE);

    return isFileExistOk;
}

int main(void)
{
    D32S iRet = ADM_ECODE_FAILED;
    const DCHAR* str = NULL;
    D32S intVal = 0;
    long int longVal = 0;
    DBOOL boolVal = FALSE;
    ADM_CFG_API_INFO_T pstApiData = {0};


    do {
        adm_debug_logInit("/tmp/", (1 * 1024 * 1024), 5);

        pstApiData.isFileExistCb = adm_cfg_apiIsFileExist;
        pstApiData.writeFileDataCb = adm_cfg_apiWriteFileData;
        iRet = adm_cfg_iniParserInit(ADM_CONFIG_FILE_PATH, &pstApiData);
        if (ADM_ECODE_SUCCESSED != iRet) {
            log_w("adm_cfg_iniParserInit failed, iRet :%d", iRet);
            // break;
        } 
        str = adm_cfg_iniParserGetString(ADM_CFG_OTA_CONFIG_VER, ADM_CFG_OTA_CONFIG_VER);
        log_i("[%s--->%s]", ADM_CFG_OTA_CONFIG_VER, str);
        str = adm_cfg_iniParserGetString(ADM_CFG_OTA_CONFIG_VER_TIME, ADM_CFG_OTA_CONFIG_VER_TIME);
        log_i("[%s--->%s]", ADM_CFG_OTA_CONFIG_VER_TIME, str);
        str = adm_cfg_iniParserGetString(ADM_CFG_OTA_CONFIG_MODIFY_AUTHOR, ADM_CFG_OTA_CONFIG_MODIFY_AUTHOR);
        log_i("[%s--->%s]", ADM_CFG_OTA_CONFIG_MODIFY_AUTHOR, str);
        str = adm_cfg_iniParserGetString(ADM_CFG_OTA_CONFIG_MODIFY_CONTENT, ADM_CFG_OTA_CONFIG_MODIFY_CONTENT);
        log_i("[%s--->%s]", ADM_CFG_OTA_CONFIG_MODIFY_CONTENT, str);
        str = adm_cfg_iniParserGetString(ADM_CFG_OTA_CONFIG_OEM_LOGO, ADM_CFG_OTA_CONFIG_OEM_LOGO);
        log_i("[%s--->%s]", ADM_CFG_OTA_CONFIG_OEM_LOGO, str);


        boolVal = adm_cfg_iniParserGetBoolean(ADM_CFG_VUS_REAL_VEHICLE_VER, FALSE);
        log_i("[%s--->%d]", ADM_CFG_VUS_REAL_VEHICLE_VER, boolVal);
        str = adm_cfg_iniParserGetString(ADM_CFG_VUS_DEF_VEHICLE_IDEN, ADM_CFG_VUS_DEF_VEHICLE_IDEN);
        log_i("[%s--->%s]",ADM_CFG_VUS_DEF_VEHICLE_IDEN, str);
        str = adm_cfg_iniParserGetString(ADM_CFG_VUS_RUNTIME_ROOT_DIR, ADM_CFG_VUS_RUNTIME_ROOT_DIR);
        log_i("[%s--->%s]", ADM_CFG_VUS_RUNTIME_ROOT_DIR, str);
        str = adm_cfg_iniParserGetString(ADM_CFG_VUS_KV_STORAGE_DIR, ADM_CFG_VUS_KV_STORAGE_DIR);
        log_i("[%s--->%s]", ADM_CFG_VUS_KV_STORAGE_DIR, str);
        str = adm_cfg_iniParserGetString(ADM_CFG_VUS_PKG_STORAGE_DIR, ADM_CFG_VUS_PKG_STORAGE_DIR);
        log_i("[%s--->%s]", ADM_CFG_VUS_PKG_STORAGE_DIR, str);
        str = adm_cfg_iniParserGetString(ADM_CFG_VUS_LOG_STORAGE_DIR, ADM_CFG_VUS_LOG_STORAGE_DIR);
        log_i("[%s--->%s]", ADM_CFG_VUS_LOG_STORAGE_DIR, str);
        str = adm_cfg_iniParserGetString(ADM_CFG_VUS_CERT_STORAGE_DIR, ADM_CFG_VUS_CERT_STORAGE_DIR);
        log_i("[%s--->%s]", ADM_CFG_VUS_CERT_STORAGE_DIR, str);

        intVal = adm_cfg_iniParserGetInt(ADM_CFG_VUS_WAIT_NETWORK_OK_SEC, 3);
        log_i("[%s--->%d]", ADM_CFG_VUS_WAIT_NETWORK_OK_SEC, intVal);

        longVal = adm_cfg_iniParserGetLongInt(ADM_CFG_VUS_LOG_MAX_SIZE, (1 * 1024 * 1024));
        log_i("[%s--->%ld]", ADM_CFG_VUS_LOG_MAX_SIZE, longVal);
        intVal = adm_cfg_iniParserGetInt(ADM_CFG_VUS_LOG_MAX_ROTATE, 5);
        log_i("[%s--->%d]", ADM_CFG_VUS_LOG_MAX_ROTATE, intVal);

        str = adm_cfg_iniParserGetString(ADM_CFG_VUS_COMM_CERT_PATH, ADM_CFG_VUS_COMM_CERT_PATH);
        log_i("[%s--->%s]", ADM_CFG_VUS_COMM_CERT_PATH, str);
        str = adm_cfg_iniParserGetString(ADM_CFG_VUS_CDN_CERT_PATH, ADM_CFG_VUS_CDN_CERT_PATH);
        log_i("[%s--->%s]", ADM_CFG_VUS_CDN_CERT_PATH, str);
        str = adm_cfg_iniParserGetString(ADM_CFG_VSU_RPC_COMM_CERT_PATH, ADM_CFG_VSU_RPC_COMM_CERT_PATH);
        log_i("[%s--->%s]", ADM_CFG_VSU_RPC_COMM_CERT_PATH, str);
        intVal = adm_cfg_iniParserGetInt(ADM_CFG_VUS_OTA_SRV_BUSINESS_PROTOCOL_TYPE, 100);
        log_i("[%s--->%s]", ADM_CFG_VUS_OTA_SRV_BUSINESS_PROTOCOL_TYPE, str);
        str = adm_cfg_iniParserGetString(ADM_CFG_VUS_OTA_SRV_REGISTER_URL, ADM_CFG_VUS_OTA_SRV_REGISTER_URL);
        log_i("[%s--->%s]", ADM_CFG_VUS_OTA_SRV_REGISTER_URL, str);
        str = adm_cfg_iniParserGetString(ADM_CFG_VUS_OTA_SRV_CHECK_URL, ADM_CFG_VUS_OTA_SRV_CHECK_URL);
        log_i("[%s--->%s]", ADM_CFG_VUS_OTA_SRV_CHECK_URL, str);
        str = adm_cfg_iniParserGetString(ADM_CFG_VUS_OTA_SRV_GET_CONFIG_URL, ADM_CFG_VUS_OTA_SRV_GET_CONFIG_URL);
        log_i("[%s--->%s]", ADM_CFG_VUS_OTA_SRV_GET_CONFIG_URL, str);
        str = adm_cfg_iniParserGetString(ADM_CFG_VUS_OTA_SRV_VEHICLE_STATE_REPORT_URL, ADM_CFG_VUS_OTA_SRV_VEHICLE_STATE_REPORT_URL);
        log_i("[%s--->%s]", ADM_CFG_VUS_OTA_SRV_VEHICLE_STATE_REPORT_URL, str);
        str = adm_cfg_iniParserGetString(ADM_CFG_VUS_OTA_SRV_ECU_STATE_REPORT_URL, ADM_CFG_VUS_OTA_SRV_ECU_STATE_REPORT_URL);
        log_i("[%s--->%s]", ADM_CFG_VUS_OTA_SRV_ECU_STATE_REPORT_URL, str);
        str = adm_cfg_iniParserGetString(ADM_CFG_VUS_OTA_SRV_PROGRESS_REPORT_URL, ADM_CFG_VUS_OTA_SRV_PROGRESS_REPORT_URL);
        log_i("[%s--->%s]", ADM_CFG_VUS_OTA_SRV_PROGRESS_REPORT_URL, str);
        str = adm_cfg_iniParserGetString(ADM_CFG_VUS_OTA_SRV_EVENT_REPORT_URL, ADM_CFG_VUS_OTA_SRV_EVENT_REPORT_URL);
        log_i("[%s--->%s]", ADM_CFG_VUS_OTA_SRV_EVENT_REPORT_URL, str);
        str = adm_cfg_iniParserGetString(ADM_CFG_VUS_OTA_SRV_FILE_UPLOAD_URL, ADM_CFG_VUS_OTA_SRV_FILE_UPLOAD_URL);
        log_i("[%s--->%s]", ADM_CFG_VUS_OTA_SRV_FILE_UPLOAD_URL, str);
        str = adm_cfg_iniParserGetString(ADM_CFG_VUS_OTA_SRV_MQTT_IP_OR_DOMAIN, ADM_CFG_VUS_OTA_SRV_MQTT_IP_OR_DOMAIN);
        log_i("[%s--->%s]", ADM_CFG_VUS_OTA_SRV_MQTT_IP_OR_DOMAIN, str);
        intVal = adm_cfg_iniParserGetInt(ADM_CFG_VUS_OTA_SRV_MQTT_PORT, 100);
        log_i("[%s--->%d]", ADM_CFG_VUS_OTA_SRV_MQTT_PORT, intVal);
        str = adm_cfg_iniParserGetString(ADM_CFG_VUS_OTA_SRV_MQTT_USER, ADM_CFG_VUS_OTA_SRV_MQTT_USER);
        log_i("[%s--->%s]", ADM_CFG_VUS_OTA_SRV_MQTT_USER, str);
        str = adm_cfg_iniParserGetString(ADM_CFG_VUS_OTA_SRV_MQTT_PASSWD, ADM_CFG_VUS_OTA_SRV_MQTT_PASSWD);
        log_i("[%s--->%s]", ADM_CFG_VUS_OTA_SRV_MQTT_PASSWD, str);
        str = adm_cfg_iniParserGetString(ADM_CFG_VRPC_SOMEIP_UNICAST_IP, ADM_CFG_VRPC_SOMEIP_UNICAST_IP_DEFAULT_VAL);
        log_i("[%s--->%s]", ADM_CFG_VRPC_SOMEIP_UNICAST_IP, str);
        str = adm_cfg_iniParserGetString(ADM_CFG_VRPC_SOMEIP_ROUTE_ADD, ADM_CFG_VRPC_SOMEIP_ROUTE_ADD_DEFAULT_VAL);
        log_i("[%s--->%s]", ADM_CFG_VRPC_SOMEIP_ROUTE_ADD, str);

        str = adm_cfg_iniParserGetString(ADM_CFG_EWEB_DOC_ROOT_DIR, ADM_CFG_EWEB_DOC_ROOT_DIR);
        log_i("[%s--->%s]", ADM_CFG_EWEB_DOC_ROOT_DIR, str);
        str = adm_cfg_iniParserGetString(ADM_CFG_EWEB_DOC_UPLOAD_DIR, ADM_CFG_EWEB_DOC_UPLOAD_DIR);
        log_i("[%s--->%s]", ADM_CFG_EWEB_DOC_UPLOAD_DIR, str);
        str = adm_cfg_iniParserGetString(ADM_CFG_EWEB_LISTENING_PORTS, ADM_CFG_EWEB_LISTENING_PORTS);
        log_i("[%s--->%s]", ADM_CFG_EWEB_LISTENING_PORTS, str);
        str = adm_cfg_iniParserGetString(ADM_CFG_EWEB_ENABLE_KEEP_ALIVE, ADM_CFG_EWEB_ENABLE_KEEP_ALIVE);
        log_i("[%s--->%s]", ADM_CFG_EWEB_ENABLE_KEEP_ALIVE, str);
        str = adm_cfg_iniParserGetString(ADM_CFG_EWEB_SSL_CERT_PATH, ADM_CFG_EWEB_SSL_CERT_PATH);
        log_i("[%s--->%s]", ADM_CFG_EWEB_SSL_CERT_PATH, str);
        intVal = adm_cfg_iniParserGetInt(ADM_CFG_EWEB_MUN_THREADES, 100);
        log_i("[%s--->%d]", ADM_CFG_EWEB_MUN_THREADES, intVal);
        intVal = adm_cfg_iniParserGetInt(ADM_CFG_EWEB_REQ_TIMEOUT_MS, 30000);
        log_i("[%s--->%d]", ADM_CFG_EWEB_REQ_TIMEOUT_MS, intVal);
        intVal = adm_cfg_iniParserGetInt(ADM_CFG_EWEB_MAX_CONNECTIONS, 50);
        log_i("[%s--->%d]", ADM_CFG_EWEB_MAX_CONNECTIONS, intVal);
        str = adm_cfg_iniParserGetString(ADM_CFG_EWEB_INSECURE, ADM_CFG_EWEB_INSECURE);
        log_i("[%s--->%s]", ADM_CFG_EWEB_INSECURE, str);

        str = adm_cfg_iniParserGetString(ADM_CFG_SYSI_RUNTIME_ROOT_DIR, ADM_CFG_SYSI_RUNTIME_ROOT_DIR);
        log_i("[%s--->%s]", ADM_CFG_SYSI_RUNTIME_ROOT_DIR, str);
        str = adm_cfg_iniParserGetString(ADM_CFG_SYSI_KV_STORAGE_DIR, ADM_CFG_SYSI_KV_STORAGE_DIR);
        log_i("[%s--->%s]", ADM_CFG_SYSI_KV_STORAGE_DIR, str);
        str = adm_cfg_iniParserGetString(ADM_CFG_SYSI_PKG_STORAGE_DIR, ADM_CFG_SYSI_PKG_STORAGE_DIR);
        log_i("[%s--->%s]", ADM_CFG_SYSI_PKG_STORAGE_DIR, str);
        str = adm_cfg_iniParserGetString(ADM_CFG_SYSI_LOG_STORAGE_DIR, ADM_CFG_SYSI_LOG_STORAGE_DIR);
        log_i("[%s--->%s]", ADM_CFG_SYSI_LOG_STORAGE_DIR, str);
        str = adm_cfg_iniParserGetString(ADM_CFG_SYSI_CERT_STORAGE_DIR, ADM_CFG_SYSI_CERT_STORAGE_DIR);
        log_i("[%s--->%s]", ADM_CFG_SYSI_CERT_STORAGE_DIR, str);

        longVal = adm_cfg_iniParserGetLongInt(ADM_CFG_SYSI_LOG_MAX_SIZE, (1 * 1024 * 1024));
        log_i("[%s--->%ld]", ADM_CFG_SYSI_LOG_MAX_SIZE, longVal);
        intVal = adm_cfg_iniParserGetInt(ADM_CFG_SYSI_LOG_MAX_ROTATE, 5);
        log_i("[%s--->%d]", ADM_CFG_SYSI_LOG_MAX_ROTATE, intVal);

        str = adm_cfg_iniParserGetString(ADM_CFG_SYSI_COMM_CERT_PATH, ADM_CFG_SYSI_COMM_CERT_PATH);
        log_i("[%s--->%s]", ADM_CFG_SYSI_COMM_CERT_PATH, str);
        str = adm_cfg_iniParserGetString(ADM_CFG_SYSI_CDN_CERT_PATH, ADM_CFG_SYSI_CDN_CERT_PATH);
        log_i("[%s--->%s]", ADM_CFG_SYSI_CDN_CERT_PATH, str);
        str = adm_cfg_iniParserGetString(ADM_CFG_SYSI_TRANS_CERT_PATH, ADM_CFG_SYSI_TRANS_CERT_PATH);
        log_i("[%s--->%s]", ADM_CFG_SYSI_TRANS_CERT_PATH, str);
        str = adm_cfg_iniParserGetString(ADM_CFG_SYSI_RPC_COMM_CERT_PATH, ADM_CFG_SYSI_RPC_COMM_CERT_PATH);
        log_i("[%s--->%s]", ADM_CFG_SYSI_RPC_COMM_CERT_PATH, str);

    } while(FALSE);

    adm_cfg_iniParserDeinit();
    adm_debug_logDeinit();

    return 0;   
}
