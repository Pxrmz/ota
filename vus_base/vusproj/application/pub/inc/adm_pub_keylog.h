/**
 * @file    adm_pub_keylog.h
 * @brief   
 * @details 公共函数处理模块
 * @author  wanglin
 * @version 
 * @date    2022-05-20
 * @copyright www.abupdate.com
 */
#ifndef ADM_PUB_KEYLOG_H
#define ADM_PUB_KEYLOG_H

#include <pthread.h>
#include "adm_typedefs.h"
#include <unistd.h>
#include <string.h>
#include <errno.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "adm_typedefs.h"
#include "adm_keyLog.h"

#define ADM_PUB_KEYLOG_ID_001   "KL_UCM_001" 
#define ADM_PUB_KEYLOG_ID_002   "KL_UCM_002" 
#define ADM_PUB_KEYLOG_ID_003   "KL_UCM_003" 
#define ADM_PUB_KEYLOG_ID_004   "KL_UCM_004" 
#define ADM_PUB_KEYLOG_ID_005   "KL_UCM_005" 
#define ADM_PUB_KEYLOG_ID_006   "KL_UCM_006" 
#define ADM_PUB_KEYLOG_ID_007   "KL_UCM_007" 
#define ADM_PUB_KEYLOG_ID_008   "KL_UCM_008" 
#define ADM_PUB_KEYLOG_ID_009   "KL_UCM_009" 
#define ADM_PUB_KEYLOG_ID_010   "KL_UCM_010" 
#define ADM_PUB_KEYLOG_ID_011   "KL_UCM_011" 
#define ADM_PUB_KEYLOG_ID_012   "KL_UCM_012" 
#define ADM_PUB_KEYLOG_ID_013   "KL_UCM_013" 
#define ADM_PUB_KEYLOG_ID_014   "KL_UCM_014" 
#define ADM_PUB_KEYLOG_ID_015   "KL_UCM_015" 
#define ADM_PUB_KEYLOG_ID_016   "KL_UCM_016" 
#define ADM_PUB_KEYLOG_ID_017   "KL_UCM_017" 
#define ADM_PUB_KEYLOG_ID_018   "KL_UCM_018" 
#define ADM_PUB_KEYLOG_ID_019   "KL_UCM_019" 
#define ADM_PUB_KEYLOG_ID_020   "KL_UCM_020" 
#define ADM_PUB_KEYLOG_ID_021   "KL_UCM_021" 
#define ADM_PUB_KEYLOG_ID_022   "KL_UCM_022" 
#define ADM_PUB_KEYLOG_ID_023   "KL_UCM_023" 
#define ADM_PUB_KEYLOG_ID_024   "KL_UCM_024" 
#define ADM_PUB_KEYLOG_ID_025   "KL_UCM_025" 
#define ADM_PUB_KEYLOG_ID_026   "KL_UCM_026" 
#define ADM_PUB_KEYLOG_ID_027   "KL_UCM_027" 
#define ADM_PUB_KEYLOG_ID_028   "KL_UCM_028" 
#define ADM_PUB_KEYLOG_ID_029   "KL_UCM_029" 
#define ADM_PUB_KEYLOG_ID_030   "KL_UCM_030" 
#define ADM_PUB_KEYLOG_ID_031   "KL_UCM_031" 
#define ADM_PUB_KEYLOG_ID_032   "KL_UCM_032" 
#define ADM_PUB_KEYLOG_ID_033   "KL_UCM_033" 
#define ADM_PUB_KEYLOG_ID_034   "KL_UCM_034" 
#define ADM_PUB_KEYLOG_ID_035   "KL_UCM_035" 
#define ADM_PUB_KEYLOG_ID_036   "KL_UCM_036" 
#define ADM_PUB_KEYLOG_ID_037   "KL_UCM_037" 
#define ADM_PUB_KEYLOG_ID_038   "KL_UCM_038" 
#define ADM_PUB_KEYLOG_ID_039   "KL_UCM_039" 
#define ADM_PUB_KEYLOG_ID_040   "KL_UCM_040" 
#define ADM_PUB_KEYLOG_ID_041   "KL_UCM_041" 
#define ADM_PUB_KEYLOG_ID_042   "KL_UCM_042" 
#define ADM_PUB_KEYLOG_ID_043   "KL_UCM_043" 
#define ADM_PUB_KEYLOG_ID_044   "KL_UCM_044" 
#define ADM_PUB_KEYLOG_ID_045   "KL_UCM_045" 
#define ADM_PUB_KEYLOG_ID_046   "KL_UCM_046" 
#define ADM_PUB_KEYLOG_ID_047   "KL_UCM_047" 
#define ADM_PUB_KEYLOG_ID_048   "KL_UCM_048" 
#define ADM_PUB_KEYLOG_ID_049   "KL_UCM_049" 



extern void adm_pub_keylogInit(void);

extern void adm_pub_setKeylogString(DCHAR* keyLogId, D32S iEreCode, const DCHAR* pDesp);

extern void adm_pub_rmKeylogFile();

#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif 

