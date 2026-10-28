#ifndef __ADM_DEBUG_H
#define __ADM_DEBUG_H

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

//#define  LOG_TAG        "[Easylogger Test]"

#include "elog.h"


/* macro definition for all formats */
#define ELOG_FMT_ALL_NO_DIR    (ELOG_FMT_LVL|ELOG_FMT_TAG|ELOG_FMT_TIME|ELOG_FMT_T_INFO|ELOG_FMT_DIR|ELOG_FMT_FUNC|ELOG_FMT_LINE)
#define ADM_DEBUG_FILE_NAME "admdebug.log"
#define ADM_DEBUG_DEFAULT_DIR   "/tmp/"

extern void adm_initLog(void);
extern void adm_deinitLog(void);

/**
 * @brief 日志文件存储目录初始化
 * @note 日志文件的存储目录必须存在
 * 
 * @param logDir        [IN]    const char*     日志文件的存储目录
 * @param max_size      [IN]    const size_t    单个日志文件存储的大小
 * @param max_rotate    [IN]    const int       最多存储的文件个数
 */
extern void adm_debug_logInit(const char* logDir, const size_t max_size, const int max_rotate);
/**
 * @brief 日志文件存储目录和日志标识初始化
 * @note 1. 日志文件的存储目录必须存在
 *       2. 日志文件格式： logFileName_timeStr.log/logFileName_timeStr.log.0/logFileName_timeStr.log.1
 *                        admfotaapp_20220425_141308.log
 * 
 * @param logDir        [IN]    const char*     日志文件的存储目录
 * @param logFileName   [IN]    const char*     日志文件的存储文件名
 * @param max_size      [IN]    const size_t    单个日志文件存储的大小
 * @param max_rotate    [IN]    const int       最多存储的文件个数
 * @param logLevel      [IN]    const D8U       根据设置的logLevel判断是否需要将日志内容写入到文件中
 * @param bOutputTerminal    [IN]    bool  串口终端输出Log使能标志
 * @note 
 *  1. if (当前日志级别 <= logLevel) 写入到日志文件中;
 *  2. log_evt(ELOG_LVL_VERBOSE)除外，此特殊Level用于事件信息的打印，默认排除在外；
 *      ELOG_LVL_ASSERT                      0
 *      ELOG_LVL_ERROR                       1
 *      ELOG_LVL_WARN                        2
 *      ELOG_LVL_INFO                        3
 *      ELOG_LVL_DEBUG                       4
 *      ELOG_LVL_VERBOSE                     5
 */
extern void adm_debug_logInitWithFileName(const char* logDir, const char* logFileName, const size_t max_size, const int max_rotate, 
const unsigned char logLevel, bool bOutputTerminal);

extern void adm_debug_logDeinit(void);

extern void adm_logTracerRegister(LOG_TRACER_OUTPUT_CALLBACK_F logTracerHandler);

extern int adm_processStart(char* pidName, char *systemCmd);


#ifdef __cplusplus
} /* extern "C" */
#endif


#endif //__ADM_DEBUG_H
