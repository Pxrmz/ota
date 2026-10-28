#ifndef KEYLOG_H
#define KEYLOG_H

#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif


struct adm_keyLog_header_
{
    char filePath[256];
    int fileSize;
    pthread_mutex_t keyLogMutex;
};

typedef struct adm_keyLog_header_ adm_keyLog_header;

adm_keyLog_header* adm_keyLog_init(const char* pFilePath, int maxLogSize);

int adm_keyLog_rmFile(adm_keyLog_header* pKeylogHeader);

int adm_keyLog_deInit(adm_keyLog_header* pKeylogHeader);

int adm_add_keyLogInfo(adm_keyLog_header* pKeylogHeader, const char *pData, int size);


#ifdef __cplusplus
}
#endif

#endif  // VECTOR_H