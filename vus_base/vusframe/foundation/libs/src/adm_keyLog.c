#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <pthread.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h> 
#include "adm_keyLog.h"

#define MAX_LOG_LINE 100

static int getFileSize(const char* filePath)
{
    struct stat st;
    if (stat(filePath, &st) == 0) 
    {
        return st.st_size;
    }
    return 0;
}

// 修改 trimLogFile 函数为保留最多100行
static int trimLogFile(const char* filePath, int maxLines)
{
    FILE* fp = fopen(filePath, "rb");
    if (!fp) {
        return -1;
    }

    // 读取文件内容
    fseek(fp, 0, SEEK_END);
    long fileSize = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    char* buffer = malloc(fileSize + 1);
    if (!buffer) {
        fclose(fp);
        return -1;
    }

    size_t readSize = fread(buffer, 1, fileSize, fp);
    buffer[readSize] = '\0';
    fclose(fp);

    // 计算总行数
    int totalLines = 0;
    for (int i = 0; i < readSize; i++) {
        if (buffer[i] == '\n') {
            totalLines++;
        }
    }

    // 如果不超过最大行数，则不需要修剪
    if (totalLines <= maxLines) {
        free(buffer);
        return 0;
    }

    // 找到需要保留内容的起始位置（跳过最早的几行）
    int linesToSkip = totalLines - maxLines;
    char* trimStart = buffer;
    int skippedLines = 0;
    
    while (skippedLines < linesToSkip && trimStart < buffer + readSize) {
        if (*trimStart == '\n') {
            skippedLines++;
        }
        trimStart++;
    }

    // 写入剩余内容
    fp = fopen(filePath, "wb");
    if (!fp) {
        free(buffer);
        return -1;
    }

    if (trimStart < buffer + readSize) {
        fwrite(trimStart, 1, buffer + readSize - trimStart, fp);
    }
    
    fclose(fp);
    free(buffer);
    return 0;
}

// 在 writeKeyLogFileData 中调整调用参数
static int writeKeyLogFileData(const char* filePath, const char* fileData, int maxLogSize)
{
    int size = 0;
    FILE *fp  = NULL;
    int writeLen = 0;
    int iRet = -1;

    if ((NULL == filePath) || (NULL == fileData))
    {
        return -1;
    }

    // 检查文件大小并在必要时进行瘦身
    //int currentSize = getFileSize(filePath);
    //int dataSize = strlen(fileData);
    
    //if (currentSize + dataSize > maxLogSize) 
    //{
    // 保持最多100行记录
    trimLogFile(filePath, MAX_LOG_LINE);
    //}

    do {
        fp = fopen(filePath, "ab");  // 使用追加模式
        if (NULL == fp) 
        {
            break;
        }

        size = strlen(fileData);

        writeLen = fwrite(fileData, 1, size, fp);
        if (writeLen == size) 
        {
            iRet = 0;
            break;
        } 
    } while(0);

    if (NULL != fp) 
    {
        (void)fclose(fp);
    }

    return iRet;
}

adm_keyLog_header* adm_keyLog_init(const char* pFilePath, int maxLogSize)
{
    int status;

    if (NULL == pFilePath)
    {
        return NULL;
    }

    adm_keyLog_header* keyLogHeader = malloc(sizeof(adm_keyLog_header));
    if (NULL == keyLogHeader)
    {
        return NULL;
    }
    
    /**填充header头信息*/
    snprintf(keyLogHeader->filePath, sizeof(keyLogHeader->filePath), "%s", pFilePath);
    keyLogHeader->fileSize = maxLogSize;
    status = pthread_mutex_init(&keyLogHeader->keyLogMutex, NULL);
    if (status != 0) 
    {
        free(keyLogHeader);
        keyLogHeader = NULL;
        return NULL;
    }

    return keyLogHeader;
}

int adm_keyLog_rmFile(adm_keyLog_header* pKeylogHeader)
{
    if (NULL == pKeylogHeader)
    {
        return -1;
    }

    pthread_mutex_lock(&pKeylogHeader->keyLogMutex);
    remove(pKeylogHeader->filePath);
    pthread_mutex_unlock(&pKeylogHeader->keyLogMutex);

    return 0;
}


int adm_keyLog_deInit(adm_keyLog_header* pKeylogHeader)
{
    if (NULL == pKeylogHeader)
    {
        return -1;
    }

    /**销毁读写锁*/
    pthread_mutex_destroy(&pKeylogHeader->keyLogMutex);
    free(pKeylogHeader);
    pKeylogHeader = NULL;

    return 0;
}

int adm_add_keyLogInfo(adm_keyLog_header* pKeylogHeader, const char *pData, int size)
{
    if (NULL == pKeylogHeader || NULL == pData || size <= 0)
    {
        return -1;
    }

    pthread_mutex_lock(&pKeylogHeader->keyLogMutex);
    writeKeyLogFileData(pKeylogHeader->filePath, pData, pKeylogHeader->fileSize);
    pthread_mutex_unlock(&pKeylogHeader->keyLogMutex);

    return  0;
}
