#ifndef __ADM_MINIUNZ_H_
#define __ADM_MINIUNZ_H_

#ifdef __cplusplus
extern "C" {
#endif

/* return 0 success */
int adm_extract_zip(const char *zipfile, const char *extractdir);

/* return 0 success */
int adm_extract_zip_onefile(const char *zipfile, const char *extractdir, const char *filename_to_extract);

#ifdef __cplusplus
}
#endif

#endif//__ADM_MINIUNZ_H_
