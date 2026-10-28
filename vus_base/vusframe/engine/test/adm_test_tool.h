#ifndef __ADM_TEST_TOOL_H_
#define __ADM_TEST_TOOL_H_

#include "json/json.h"
#include "adm_typedefs.h"

#ifdef __cplusplus
extern "C" {
#endif

void adm_test_tool_readConfig(Json::Value* root, const DCHAR *path);




#ifdef __cplusplus
}
#endif

#endif//__ADM_TEST_TOOL_H_
