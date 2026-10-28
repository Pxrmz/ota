#include "adm_pis_log.h"
#include "adm_pis_package.h"
#include "adm_typedefs.h"
#include "adm_list.h"
#include "adm_memory.h"
#include "adm_string.h"
//#include "adm_queue.h"//TODO compile warning
#include <pthread.h>


typedef struct {
    struct list_head entry;
    DCHAR *packageIndex;
    DCHAR *packagePath[ADM_PIS_PackageState_Max];
} PACKAGE_NODE_T;
static PACKAGE_NODE_T *packageNodeNew(const DCHAR *packageIndex, ADM_PIS_PackageState_E state, const DCHAR *path) {
    PACKAGE_NODE_T *node = adm_malloc(sizeof(PACKAGE_NODE_T));
    if(node == NULL) {} else {
        node->packageIndex = NULL;
        for(D32U i=0; i<ADM_PIS_PackageState_Max; ++i) {
            node->packagePath[i] = NULL;
        }

	node->packageIndex = adm_strdup(packageIndex);
	node->packagePath[state] = adm_strdup(path);
    }
    return node;
}
static void packageNodeDel(PACKAGE_NODE_T *node) {
    if(node->packageIndex) {
        adm_free(node->packageIndex);
    }
    for(D32U i=0; i<ADM_PIS_PackageState_Max; ++i) {
        if(node->packagePath[i]) {
            adm_free(node->packagePath[i]);
        }
    }
    adm_free(node);
}

typedef struct {
    struct list_head head;
    //adm_handle_t mutex;
    pthread_mutex_t mutex;
} PACKAGE_LIST_T;
static void packageListInit(PACKAGE_LIST_T *list) {
    INIT_LIST_HEAD(&list->head);
    //list->mutex = adm_mutex_create();
    pthread_mutex_init(&list->mutex, NULL);
}
static void packageListDeInit(PACKAGE_LIST_T *list) {
    PACKAGE_NODE_T *pos, *next;
    list_for_each_entry_safe(pos, next, &list->head, PACKAGE_NODE_T, entry) {
        list_del(&pos->entry);
        packageNodeDel(pos);
    }
    //adm_mutex_close(list->mutex);
    pthread_mutex_destroy(&list->mutex);
}
static void packageListSet(PACKAGE_LIST_T *list, const DCHAR *packageIndex, ADM_PIS_PackageState_E state, const DCHAR *path) {
    //adm_mutex_lock(list->mutex);
    pthread_mutex_lock(&list->mutex);
    DBOOL isExist = 0;
    PACKAGE_NODE_T *pos;
    list_for_each_entry(pos, &list->head, PACKAGE_NODE_T, entry) {
        if(pos->packageIndex) {
            if(adm_strcmp(pos->packageIndex,packageIndex) == 0) {
                if(pos->packagePath[state]) {
                    adm_free(pos->packagePath[state]);
                }
                pos->packagePath[state] = adm_strdup(path);
                isExist = 1;
            }
        }
    }

    if(isExist) {} else {
        PACKAGE_NODE_T *node = packageNodeNew(packageIndex, state, path);
        if(node == NULL) {} else {
            list_add_tail(&list->head, &node->entry);
        }
    }
    //adm_mutex_unlock(list->mutex);
    pthread_mutex_unlock(&list->mutex);
}
static DBOOL packageListGet(PACKAGE_LIST_T *list, const DCHAR *packageIndex, ADM_PIS_PackageState_E state, DCHAR *path, D32U pathLen) {
    DBOOL dRet = 0;
    //adm_mutex_lock(list->mutex);
    pthread_mutex_lock(&list->mutex);
    PACKAGE_NODE_T *pos;
    list_for_each_entry(pos, &list->head, PACKAGE_NODE_T, entry) {
        if(pos->packageIndex) {
            if(adm_strcmp(pos->packageIndex,packageIndex) == 0) {
                if(pos->packagePath[state]) {
                    if(adm_strcpy(path, pos->packagePath[state], pathLen)) {
                        dRet = 1;
                    }
                }
                break;
            }
        }
    }
    //adm_mutex_unlock(list->mutex);
    pthread_mutex_unlock(&list->mutex);
    return dRet;
}
static DBOOL packageListClean(PACKAGE_LIST_T *list) {
    /* TODO clean nvram */
    PACKAGE_NODE_T *pos, *next;
    list_for_each_entry_safe(pos, next, &list->head, PACKAGE_NODE_T, entry) {
        list_del(&pos->entry);
        packageNodeDel(pos);
    }
}

static PACKAGE_LIST_T s_list;
void adm_pis_packageInit() {
    packageListInit(&s_list);
}
void adm_pis_packageDeInit() {
    packageListDeInit(&s_list);
}
void adm_pis_packagePathSet(const DCHAR *packageIndex, ADM_PIS_PackageState_E state, const DCHAR *path) {
    if((packageIndex != NULL)
            && (state < ADM_PIS_PackageState_Max)
            && (state >= 0)
            && (path != NULL)) {
        packageListSet(&s_list, packageIndex, state, path);
    }
}
DBOOL adm_pis_packagePathGet(const DCHAR *packageIndex, ADM_PIS_PackageState_E state, DCHAR *path, D32U pathLen) {
    DBOOL dRet = 0;
    if((packageIndex != NULL)
            && (state < ADM_PIS_PackageState_Max)
            && (state >= 0)
            && (path != NULL)) {
        dRet = packageListGet(&s_list, packageIndex, state, path, pathLen);
    }
    return dRet;
}
void adm_pis_packageClean() {
    packageListClean(&s_list);
}

static void packageIndexCreateByEcu(const DCHAR *ecuReqId, const DCHAR *ecuDataId, DCHAR *packageIndex, D32U len) {
    adm_snprintf(packageIndex, len, "ECU.PACKAGE.%s.%s", ecuReqId, ecuDataId ? ecuDataId :"null");
}
void adm_pis_packageEcuPathSet(const DCHAR *ecuReqId, const DCHAR *ecuDataId, ADM_PIS_PackageState_E state, const DCHAR *path) {
    DCHAR packageIndex[256];
    packageIndexCreateByEcu(ecuReqId, ecuDataId, packageIndex, sizeof(packageIndex)/sizeof(DCHAR));
    adm_pis_packagePathSet(packageIndex, state, path);
}
DBOOL adm_pis_packageEcuPathGet(const DCHAR *ecuReqId, const DCHAR *ecuDataId, ADM_PIS_PackageState_E state, DCHAR *path, D32U pathLen) {
    DCHAR packageIndex[256];
    packageIndexCreateByEcu(ecuReqId, ecuDataId, packageIndex, sizeof(packageIndex)/sizeof(DCHAR));
    return adm_pis_packagePathGet(packageIndex, state, path, pathLen);
}
