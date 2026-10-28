#ifndef __ADM_MINIZIP_H__
#define __ADM_MINIZIP_H__

#ifdef __cplusplus
extern "C" {
#endif

/* return 0 success */
/**
 * @brief 压缩目录下的所有文件到指定的输出路径
 * 
 * @param compressedDir     [IN]    被压缩的文件目录
 * @param zipDir            [IN]    待输出的压缩文件的目录
 * @param zipFileName       [IN]    待输出的压缩文件名 
 * @return int  0:Successed
 */
int adm_compress_zip(const char *compressedDir, const char *zipDir, const char* zipFileName);

#ifdef __cplusplus
}
#endif

#endif//__ADM_MINIZIP_H__