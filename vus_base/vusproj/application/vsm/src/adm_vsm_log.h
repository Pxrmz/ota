#ifndef __ADM_VSM_LOG_H__
#define __ADM_VSM_LOG_H__

#include "adm_com_modDefine.h"

#if !defined(MOD_TAG_ID)
#define MOD_TAG_ID ADM_COM_MOD_VSM
#endif  // MOD_TAG_ID

#if !defined(LOG_TAG)
#define LOG_TAG MOD_NAME(ADM_COM_MOD_VSM) /*!< 宏函数，用于通过模块名的枚举值类型获取模块名称     */
#endif  // LOG_TAG

#include "adm_debug.h"

#endif  //__ADM_VSM_LOG_H__