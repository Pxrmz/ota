#ifndef	__ADM_INTEGRATION_TEST_H_
#define __ADM_INTEGRATION_TEST_H_
#include "adm_com_modDefine.h"

#include "adm_typedefs.h"
#include "adm_list.h"
#include "adm_ecs_rsConfig.h"
#include "json-c/json.h"
#include "adm_ocs_airProtocolAdapter.h"
#include "adm_com_modDefine.h"



#include <pthread.h>//TODO use platform api
#include <semaphore.h>

#ifdef __cplusplus
extern "C" {
#endif


#if !defined(LOG_TAG)
#define LOG_TAG             "TEST"         /*!< 宏函数，用于通过模块名的枚举值类型获取模块名称     */
#endif

#if !defined(MOD_TAG_ID)
#define MOD_TAG_ID          ADM_COM_MOD_PUB
#endif

#include "adm_debug.h"


#define ADM_IPM_TEST_FILE_PATH_LENGTH              256             // 文件路径最大长度



#define ADM_IPM_TEST_GENERALCONFIG_FILE     "GeneralConfig.json"   //!< 平台通用配置信息文件

#define ADM_IPM_TEST_PARTCONFIG_FILE     "PartConfig.json"         //!< 平台零件信息文件

/*!
 *******************************************************************************
 * 设置集成测试配置文件目录
 *
 * \note 
 *
 * \param[in]   pcConfigFileDir      集成测试配置文件目录
 * 
 * \retval      无
 *******************************************************************************
 */
void adm_integrationtest_setConfigFileDir(DCHAR* pcConfigFileDir);

/*!
 *******************************************************************************
 * 读取指定文件的数据，若读取失败返回空，否则返回读取buf。数据使用完后需要释放内存
 *
 * \note 
 *
 * \param[in]   pcFileName      文件名
 * 
 * \retval      DCHAR*  读取数据buf
 *******************************************************************************
 */
DCHAR*  adm_integrationtest_readFile(const DCHAR* pcFileName);

/*!
 *******************************************************************************
 * 获取平台配置信息
 *
 * \note 
 *
 * \param[in]   pstEcsRsConfigData      平台配置信息指针
 * 
 * \retval      D32S  获取结果
 *******************************************************************************
 */
D32S adm_integrationtest_getEcsRsConfigData(ADM_ECS_RSCONFIGDATA_T* pstEcsRsConfigData);


#ifdef __cplusplus
}
#endif

#endif//__ADM_INTEGRATION_TEST_H_