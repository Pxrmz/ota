/*!
 * \file
 * \brief
 * \author
 * \date 2021-11-16
 * \copyright www.abupdate.com
 */

#ifndef __ADM_VSM_UIM_H_
#define __ADM_VSM_UIM_H_

#include "adm_typedefs.h"

#ifdef __cplusplus
extern "C" {
#endif

/*!
 *******************************************************************************
 * 用于给UIM调用开启或者关闭UCM在检测完成后自动下载的功能
 *
 * \param   isAutoDownload[in]      设置UCM开启TRUE或者关闭FALSE自动下载
 *
 *******************************************************************************
 */
extern void adm_vsm_hmiIsAutoDownload(DBOOL isAutoDownload, D32S *errorCode);

#ifdef __cplusplus
}
#endif

#endif  //__ADM_VSM_UIM_H_
