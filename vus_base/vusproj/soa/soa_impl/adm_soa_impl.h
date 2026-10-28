#ifndef __ADM_SOA_IMPL_H__
#define __ADM_SOA_IMPL_H__

#ifdef __cplusplus
extern "C" {
#endif

#define EXPORT_FUN __attribute__((visibility("default")))

#include "adm_typedefs.h"

extern ADM_SOA_SESSION_T* adm_dis_soa_init_impl(void* priv);

extern D32S adm_dis_soa_start_impl(ADM_SOA_SESSION_T* soaSession);

extern D32S adm_dis_soa_deinit_impl(ADM_SOA_SESSION_T* soaSession);


#ifdef __cplusplus  // close out "C" linkage in case of c++ compiling
}
#endif

#endif	// __ADM_SOA_IMPL_H__


