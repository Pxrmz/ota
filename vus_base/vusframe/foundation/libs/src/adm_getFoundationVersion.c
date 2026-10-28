#include "adm_getFoundationVersion.h"
#include <string.h>
#include <stdio.h>

const char* adm_getFoundationVersion()
{
    static char version[128];
    memset(version, '\0', 128);
#ifdef ADM_FOUNDATION_SDK_VERSION
    snprintf(version, 128, "%s", ADM_FOUNDATION_SDK_VERSION);
#else
    snprintf(version, 128, "%s", "");
#endif
    return version;
}