
#ifndef __ADM_UIM_SERVICES_H_
#define __ADM_UIM_SERVICES_H_


#include "adm_typedefs.h"

#ifdef __cplusplus
extern "C" {
#endif


//extern SomeIPContext* UIMContext;


//extern DCHAR*         m_pIpAddr;
typedef D32S (*adm_uim_serviceInit_func)(void* priv);
extern D32S adm_uim_serviceInit(void* priv);

typedef D32S (*adm_uim_serviceDeinit_func)(void);
extern D32S adm_uim_serviceDeinit(void);
#ifdef __cplusplus
}
#endif

#endif // __ADM_UIM_SERVICES_H_

