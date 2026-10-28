#ifndef __ADM_DIM_VDI_SIMULATOR_H__
#define __ADM_DIM_VDI_SIMULATOR_H__

#ifdef __cplusplus
extern "C" {
#endif

#define EXPORT_FUN __attribute__((visibility("default")))

#include "adm_typedefs.h"
#include "adm_com_errorCode.h"


#include "adm_string.h"
#include "adm_memory.h"


extern D32S adm_vdi_simulatorInit();
extern D32S adm_vdi_simulatorExec();
extern D32S adm_vdi_simulatorDeinit();


#ifdef __cplusplus  // close out "C" linkage in case of c++ compiling
}
#endif
	
#endif // __ADM_DIM_VDI_SIMULATOR_H__