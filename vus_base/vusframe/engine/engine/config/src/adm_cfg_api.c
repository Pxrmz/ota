#include "adm_cfg_log.h"
#include "adm_cfg_api.h"
#include "adm_string.h"
#include "adm_memory.h"
#include "adm_com_lib.h"
#include "adm_com_errorCode.h"
#include "adm_iniparser.h"

static Dictionary_t* g_dict = NULL;

// static D32S adm_cfg_generateDefaultConfig(ADM_CFG_API_WRITE_FILE_DATA_CALLBACK_F writeFileDataCb)
// {
//     // 将默认的配置信息，写入默认的配置文件路径
//     D32S iRet = ADM_ECODE_FAILED;

//     do {
//         iRet = writeFileDataCb(ADM_CFG_DEFAULT_INI_FILE_PATH, ADM_CFG_DEFAULT_INIT_FILE_STR);
//         if (ADM_ECODE_SUCCESSED != iRet) {
//             log_e("adm_com_writeFileData failed, iRet:%d", iRet);
//             break;
//         }
//     } while(FALSE);

//     return iRet;
// }

D32S adm_cfg_iniParserInit(const DCHAR* filename, const ADM_CFG_API_INFO_T* pstData)
{
    DBOOL isFilenameOK = FALSE;
    const DCHAR* iniPath = NULL;
    D32S iRet = ADM_ECODE_FAILED;

    do {
        if (NULL == filename) {
            iRet = ADM_ECODE_FILE_NOT_EXIST;
            log_e("Input parameter [filename] is nil, iRet:%d", iRet);
            break;
        }

        if (NULL == pstData) {
            iRet = ADM_ECODE_INVALID_PARAM;
            log_e("Input parameter [pstData] is nil, iRet:%d", iRet);
            break;
        }

        if (NULL == pstData->isFileExistCb || NULL == pstData->writeFileDataCb) {
            iRet = ADM_ECODE_INVALID_PARAM;
           log_e("Inpur parameter [isFileExistCb | writeFileDataCb] is nil, iRet:%d", iRet);
           break;
        }

        // /** 如果传入的文件路径参数不为空，且文件路径存在并可读，则配置文件OK */
        // if (NULL != filename) {
        //     if (TRUE == pstData->isFileExistCb(filename)) {
        //         isFilenameOK = TRUE;
        //         log_i("adm_com_isFileExist is TRUE, %s", filename);
        //     } else {
        //         log_w("Input parameter [%s] is not exist, we will use default config in adm_cfg_defulat.h", filename);
        //     }
        // } else {
        //     log_w("Input parameter [filename] is nil, we will use default config in adm_cfg_default.h");
        // }

        // /* 如果上述判断OK，则加载配置文件 */
        // if (TRUE == isFilenameOK) {
        //     iniPath = filename;
        // } else {
        //     /** 如果上述判断为False，则加载默认配置文件 */
        //     iRet = adm_cfg_generateDefaultConfig(pstData->writeFileDataCb);
        //     if (ADM_ECODE_SUCCESSED != iRet) {
        //         iRet = ADM_ECODE_INI_PARSER_GENERATE_FAILED;
        //         log_e("adm_cfg_generateDefaultConfig failed, iRet:%d", iRet);
        //         break;
        //     }  else {
        //         log_i("adm_cfg_generateDefaultConfig successed, ini config path:%s", ADM_CFG_DEFAULT_INI_FILE_PATH);
        //     }
        //     iniPath = ADM_CFG_DEFAULT_INI_FILE_PATH;
        // }

        /** 加载 ini 配置文件 */
        g_dict = adm_iniparser_load(filename);
        if (NULL == g_dict) {
            iRet = ADM_ECODE_INI_PARSER_FILE_LOAD_FAILED;
            log_e("adm_iniparser_load failed, ini path:[%s], iRet:%d", filename, iRet);
            break;
        } else {
            iRet = ADM_ECODE_SUCCESSED;
            log_i("adm_iniparser_load successed");
            break;
        }
    } while(FALSE);

    return iRet;
}

void adm_cfg_iniParserDeinit(void)
{
    if (NULL != g_dict) {
        adm_iniparser_freedict(g_dict);
    }
}


const DCHAR* adm_cfg_iniParserGetString(const DCHAR* key, const DCHAR* notfound)
{
    const DCHAR* str = NULL;

    do {
        str = notfound;
        if (NULL == g_dict) {
            log_e("g_dict is nil, please call adm_cfg_iniParserInit to init ini config");
            break;
        }

        str = adm_iniparser_getstring(g_dict, key, notfound);
        log_i("[%s = %s]", key, str);
    } while(FALSE);

    return str;
}

D32S adm_cfg_iniParserGetInt(const DCHAR* key, D32S notfound)
{
    D32S intVal = 0;

    do {
        intVal = notfound;
        if (NULL == g_dict) {
           log_e("g_dict is nil, please call adm_cfg_iniParserInit to init ini config");
            break; 
        }

        intVal = adm_iniparser_getint(g_dict, key, notfound);
        log_i("[%s = %d]", key, intVal);
    } while(FALSE);

    return intVal;
}

long int adm_cfg_iniParserGetLongInt(const DCHAR* key, long int notfound)
{
    long int val = 0;

    do {
        val = notfound;
        if (NULL == g_dict) {
           log_e("g_dict is nil, please call adm_cfg_iniParserInit to init ini config");
            break; 
        }

        val = adm_iniparser_getlongint(g_dict, key, notfound);
        log_i("[%s = %ld]", key, val);

    } while(FALSE);

    return val;
}

double adm_cfg_iniParserGetDouble(const DCHAR* key, double notfound)
{
    double val = 0.0;

    do {
        val = notfound;
        if (NULL == g_dict) {
           log_e("g_dict is nil, please call adm_cfg_iniParserInit to init ini config");
            break; 
        }

        val = adm_iniparser_getdouble(g_dict, key, notfound);
        log_i("[%s = %lf]", key, val);
    } while(FALSE);

    return val;
}

DBOOL adm_cfg_iniParserGetBoolean(const DCHAR* key, DBOOL notfound)
{
    DBOOL val = FALSE;

    do {
        val = notfound;
        if (NULL == g_dict) {
           log_e("g_dict is nil, please call adm_cfg_iniParserInit to init ini config");
            break; 
        }

        val = (DBOOL)adm_iniparser_getboolean(g_dict, key, (int)notfound);
        log_i("[%s = %d]", key, val);
    } while(FALSE);

    return val;
}
