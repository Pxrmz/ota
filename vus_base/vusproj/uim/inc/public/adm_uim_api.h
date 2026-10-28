#ifndef __ADM_DIM_API_H__
#define __ADM_DIM_API_H__

#include "adm_typedefs.h"
//#include "adm_uim_apiImpl.h" 

#define cp_log(l) #l



#ifdef __cplusplus
extern "C" {
#endif

struct _adm_uim_impls;

extern D32S adm_uim_init(void);
extern D32S adm_uim_deInit(void);
extern const struct _adm_uim_impls* adm_uim_getImpls(void);

#ifdef __cplusplus
}
#endif

#endif // __ADM_DIM_API_H__
