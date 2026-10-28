#ifndef ADM_SOA_H
#define ADM_SOA_H

#ifdef __cplusplus
extern "C" {
#endif

#define EXPORT_FUN __attribute__((visibility("default")))

#include "adm_typedefs.h"

typedef enum ADM_SOA_IMPL_TYPE
{
    ADM_SOA_IMPL_BY_IDLE = 0,
    ADM_SOA_IMPL_BY_SOMEIP,
    ADM_SOA_IMPL_BY_ARPC,
    ADM_SOA_IMPL_BY_INVALID
}ADM_SOA_IMPL_TYPE_E;


typedef struct ADM_SOA_SESSION_{
	void* context;
	void* priv;
}ADM_SOA_SESSION_T;

ADM_SOA_SESSION_T* adm_soa_getSession(void);

// adm_soa_getxxxInstanceId(), for server
D16U adm_soa_getVdiInstanceId(void);
D16U adm_soa_getUclInstanceId(void);
D16U adm_soa_getHostInstanceId(void);
D16U adm_soa_getTransInstanceId(void);
D16U adm_soa_getDpoInstanceId(void);
D16U adm_soa_getHmiInstanceId(void);
D16U adm_soa_getUimInstanceId(void);
D16U adm_soa_getHostTboxInstanceId(void);
D16U adm_soa_getUclAdccInstanceId(void);
D16U adm_soa_getIdelTransInstanceId(void);


extern ADM_SOA_SESSION_T* adm_soa_init(void* priv);
typedef ADM_SOA_SESSION_T* (*adm_soa_init_func)(void* priv);

extern D32S adm_soa_start(void);
typedef D32S (*adm_soa_start_func)(ADM_SOA_SESSION_T* soaSession);

extern void adm_soa_deinit(void);
typedef void (*adm_soa_deinit_func)(ADM_SOA_SESSION_T* soaSession);


#ifdef __cplusplus  // close out "C" linkage in case of c++ compiling
}
#endif

#endif	// ADM_SOA_H

