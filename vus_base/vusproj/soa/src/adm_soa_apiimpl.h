#ifndef __ADM_SOA_APIIMPL_H__
#define __ADM_SOA_APIIMPL_H__

#include "adm_soa.h"

#ifdef __cplusplus
extern "C" {
#endif

#define EXPORT_FUN __attribute__((visibility("default")))

typedef struct {
	adm_soa_init_func		adm_dis_soa_init_impl	;
    adm_soa_start_func		adm_dis_soa_start_impl	;
	adm_soa_deinit_func		adm_dis_soa_deinit_impl	;
}soa_func_list_t;


#ifdef __cplusplus
}
#endif

#endif // __ADM_SOA_APIIMPL_H__
