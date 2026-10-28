#ifndef __ADM_DIM_API_H_
#define __ADM_DIM_API_H_

#include "adm_typedefs.h"

#include "adm_dim_host.h"
#include "adm_dim_ucl.h"
#include "adm_dim_vdi.h"
#include "adm_dim_transHost.h"
#include "adm_dim_dpo.h"

#ifdef __cplusplus
extern "C" {
#endif

#define EXPORT_FUN __attribute__((visibility("default")))

typedef enum ADM_DIM_IMPL_TYPE
{
    ADM_DIM_IMPL_BY_IDLE = 0,
    ADM_DIM_IMPL_BY_DISCPP_SIMU,	// c++ 模拟器
    ADM_DIM_IMPL_BY_DISLVM_SIMU,	// 脚本模拟器
    ADM_DIM_IMPL_BY_DISSOA_SOIP,	// ABUP someip
    ADM_DIM_IMPL_BY_DISSOA_SOC,		// ABUP	socket
    ADM_DIM_IMPL_BY_DISV_EVG,		// 恒大
    ADM_DIM_IMPL_BY_DISV_GAC,		// 广汽
    ADM_DIM_IMPL_BY_INVALID
}ADM_DIM_IMPL_TYPE_E;

typedef struct ADM_DIM_IMPL_INFO
{
    ADM_DIM_IMPL_TYPE_E dim_impl_type;
    DCHAR *dim_impl_library_path;
}ADM_DIM_IMPL_INFO_T;


extern D32S adm_dim_init( void* priv);

extern D32S adm_dim_isInit(void);

extern D32S adm_dim_deInit(void);

#ifdef __cplusplus
}
#endif

#endif//__ADM_DIM_API_H_
