#ifndef __ADM_UIM_GENERAL_H_
#define __ADM_UIM_GENERAL_H_

#include "adm_typedefs.h"

#ifdef __cplusplus
extern "C" {
#endif

/*********************************************************************************
 * @brief 状态机所处状态通知
 * 
 * @param state         [In]    D32S    状态
*******************************************************************************/
typedef void (*adm_uim_vsmStateNotify_func)(const D32S state);
extern void adm_uim_vsmStateNotify(const D32S state);

#ifdef __cplusplus
}
#endif

#endif // __ADM_UIM_SERVICES_H_
