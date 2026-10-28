#ifndef __ADM_SOA_LOG_H__
#define __ADM_SOA_LOG_H__

#include "adm_com_modDefine.h"

#if !defined(LOG_TAG)
#define LOG_TAG             MOD_NAME(ADM_COM_MOD_COM)         /*!< 宏函数，用于通过模块名的枚举值类型获取模块名称     */
#endif

#if !defined(MOD_TAG_ID)
#define MOD_TAG_ID          ADM_COM_MOD_COM
#endif

#include "adm_debug.h"

#endif//__ADM_SOA_LOG_H__
