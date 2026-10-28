#ifndef __ADM_PIS_CORE_H_
#define __ADM_PIS_CORE_H_

#include "adm_pis_package.h"
#include "adm_typedefs.h"
#include <pthread.h>
#include "adm_list.h"
#include "json-c/json.h"
#include "adm_ips_debug.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    struct list_head entry;
    ADM_PIS_PACKAGE_HEAD_T head;
    ADM_PIS_PACKAGE_DATA_T data;
}ADM_PIS_PACKAGE_NODE_T;

typedef struct {
    pthread_mutex_t mutex;
    struct list_head head;
}ADM_PIS_PACKAGE_LIST_T;

#define	ADM_PIS_PACKAGE_LIST_KV_KEY	"pis_package_list"

/* ADM_PIS_PACKAGE_LIST_T */
static void pisPackageListInit(ADM_PIS_PACKAGE_LIST_T *package);
static void pisPackageListDeInit(ADM_PIS_PACKAGE_LIST_T *package);
static void pisPackageListSet(ADM_PIS_PACKAGE_LIST_T *package, const ADM_PIS_PACKAGE_HEAD_T *head, const ADM_PIS_PACKAGE_DATA_T *data);
static void pisPackageListSetImage(ADM_PIS_PACKAGE_LIST_T *package, const ADM_PIS_PACKAGE_HEAD_T *head, const ADM_PIS_FILE_T *imagePackage);
static void pisPackageListSetSign(ADM_PIS_PACKAGE_LIST_T *package, const ADM_PIS_PACKAGE_HEAD_T *head, const ADM_PIS_FILE_T *signPackage);
static void pisPackageListSetSecurity(ADM_PIS_PACKAGE_LIST_T *package, const ADM_PIS_PACKAGE_HEAD_T *head, const ADM_COM_FILESCYINFO_T *fileSecurityInfo);
static void pisPackageListSetMakeType(ADM_PIS_PACKAGE_LIST_T *package, const ADM_PIS_PACKAGE_HEAD_T *head, const D32S makePackageType);
static ADM_PIS_PACKAGE_DATA_T* pisPackageListGet(ADM_PIS_PACKAGE_LIST_T *package, const ADM_PIS_PACKAGE_HEAD_T *head, ADM_PIS_PACKAGE_DATA_T *data);
static ADM_PIS_PACKAGE_NODE_T* pisPackageListFind(ADM_PIS_PACKAGE_LIST_T *package, const ADM_PIS_PACKAGE_HEAD_T *head);
static struct json_object* pisPackageListJsonCreate(const ADM_PIS_PACKAGE_LIST_T *package);
static void pisPackageListJsonParse(ADM_PIS_PACKAGE_LIST_T *package, const struct json_object* root);
static void pisPackageListJsonLoad(ADM_PIS_PACKAGE_LIST_T *package);
static void pisPackageListJsonStore(const ADM_PIS_PACKAGE_LIST_T *package);


/* ADM_PIS_PACKAGE_DATA_T */
static void pisPackageDataInit(ADM_PIS_PACKAGE_DATA_T *data);
static void pisPackageDataDeInit(ADM_PIS_PACKAGE_DATA_T *data);
static void pisPackageDataClone(const ADM_PIS_PACKAGE_DATA_T *src, ADM_PIS_PACKAGE_DATA_T *dst);
static struct json_object* pisPackageDataJsonCreate(const ADM_PIS_PACKAGE_DATA_T *data);
static void pisPackageDataJsonParse(ADM_PIS_PACKAGE_DATA_T *data, const struct json_object* root);
static void pisPackageDataSetImage(ADM_PIS_PACKAGE_DATA_T *data, const ADM_PIS_FILE_T *imagePackage);
static void pisPackageDataSetSign(ADM_PIS_PACKAGE_DATA_T *data, const ADM_PIS_FILE_T *signPackage);
static void pisPackageDataSetSecurity(ADM_PIS_PACKAGE_DATA_T *data, const ADM_COM_FILESCYINFO_T *fileSecurityInfo);

/* ADM_PIS_PACKAGE_HEAD_T */
static void pisPackageHeadInit(ADM_PIS_PACKAGE_HEAD_T *head);
static void pisPackageHeadDeInit(ADM_PIS_PACKAGE_HEAD_T *head);
static void pisPackageHeadClone(const ADM_PIS_PACKAGE_HEAD_T *src, ADM_PIS_PACKAGE_HEAD_T *dst);
static DBOOL pisPackageHeadCompare(const ADM_PIS_PACKAGE_HEAD_T *src, const ADM_PIS_PACKAGE_HEAD_T *dst);
static struct json_object* pisPackageHeadJsonCreate(const ADM_PIS_PACKAGE_HEAD_T *head);
static void pisPackageHeadJsonParse(ADM_PIS_PACKAGE_HEAD_T *head, const struct json_object* root);
static struct json_object* pisPackageHeadJsonCreateDebug(const ADM_PIS_PACKAGE_HEAD_T *head);

/* ADM_PIS_PACKAGE_STATE_E */
static const DCHAR *pisPackageStateString(ADM_PIS_PACKAGE_STATE_E state);

/* ADM_PIS_PACKAGE_TYPE_E */
static const DCHAR *pisPackageTypeString(ADM_PIS_PACKAGE_TYPE_E type);

#define	PIS_PACKAGE_NODE_SHOW(node) \
do{\
    const ADM_PIS_PACKAGE_NODE_T *node_ = (node);\
    if(NULL != node_){\
        struct json_object* root = pisPackageNodeJsonCreate(node_);\
        if(NULL != root){\
            const DCHAR *str = json_object_to_json_string(root);\
            log_d("n%s", ADM_STR_SAFE_OUTPUT(str));\
            (void)json_object_put(root);\
        }\
    }\
}while(FALSE)


#ifdef __cplusplus
}
#endif

#endif//__ADM_PIS_CORE_H_
