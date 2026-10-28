#include "adm_pis_core.h"
#include "adm_pis_log.h"
#include "adm_typedefs.h"
#include "adm_memory.h"
#include "adm_string.h"
//#include "adm_queue.h"//TODO compile warning
#include "adm_engine_common.h"
#include "adm_com_kvstorage.h"

static ADM_PIS_PACKAGE_LIST_T m_package;
static DBOOL m_isInitPackage = DFALSE;
void adm_pis_packageInit(void){
    if(DFALSE == m_isInitPackage){
        pisPackageListInit(&m_package);
        m_isInitPackage = DTRUE;
    }
}
void adm_pis_packageDeInit(void){
    if(DFALSE != m_isInitPackage){
        pisPackageListDeInit(&m_package);
		(void)adm_com_kvstorage_remove_key(adm_core_service_getBusinessDatabaseDb(), ADM_PIS_PACKAGE_LIST_KV_KEY);
	    m_isInitPackage = DFALSE;
    }
}
void adm_pis_packageSet(const ADM_PIS_PACKAGE_HEAD_T *head, const ADM_PIS_PACKAGE_DATA_T *data){
    adm_pis_packageInit();
    if(NULL != head && NULL != data){
        pisPackageListSet(&m_package, head, data);
    }
}
ADM_PIS_PACKAGE_DATA_T* adm_pis_packageGet(const ADM_PIS_PACKAGE_HEAD_T *head, ADM_PIS_PACKAGE_DATA_T *data){
    ADM_PIS_PACKAGE_DATA_T* resdata = NULL;
    adm_pis_packageInit();
    if(NULL != head && NULL != data){
        resdata = pisPackageListGet(&m_package, head, data);
    }
    return resdata;
}

void adm_pis_packageImageSet(const ADM_PIS_PACKAGE_HEAD_T *head, const ADM_PIS_FILE_T *imagePackage){
    adm_pis_packageInit();
    if(NULL != head && NULL != imagePackage){
	    pisPackageListSetImage(&m_package, head, imagePackage);
    }
}
void adm_pis_packageSignSet(const ADM_PIS_PACKAGE_HEAD_T *head, const ADM_PIS_FILE_T *signPackage){
    adm_pis_packageInit();
    if(NULL != head && NULL != signPackage){
	    pisPackageListSetSign(&m_package, head, signPackage);
    }
}
void adm_pis_packageSecuritySet(const ADM_PIS_PACKAGE_HEAD_T *head, const ADM_COM_FILESCYINFO_T *fileSecurityInfo){
    adm_pis_packageInit();
    if(NULL != head && NULL != fileSecurityInfo){
	    pisPackageListSetSecurity(&m_package, head, fileSecurityInfo);
    }
}
void adm_pis_packageMakeTypeSet(const ADM_PIS_PACKAGE_HEAD_T *head, const D32S makePackageType){
    adm_pis_packageInit();
    if(NULL != head){
	    pisPackageListSetMakeType(&m_package, head, makePackageType);
    }
}
void adm_pis_packageDataInit(ADM_PIS_PACKAGE_DATA_T *data){
    if(NULL != data){pisPackageDataInit(data);}
}
void adm_pis_packageDataDeInit(ADM_PIS_PACKAGE_DATA_T *data){
    if(NULL != data){pisPackageDataDeInit(data);}
}
struct json_object* adm_pis_packageDataJsonCreate(const ADM_PIS_PACKAGE_DATA_T *data){
    struct json_object* root = NULL;
    if(NULL != data){
	    root = pisPackageDataJsonCreate(data);
    }
    return root;
}
void adm_pis_packageDataJsonParse(ADM_PIS_PACKAGE_DATA_T *data, const struct json_object* root){
    if(NULL != data && NULL != root){
	    pisPackageDataJsonParse(data, root);
    }
}

/* ADM_PIS_PACKAGE_HEAD_T */
static void pisPackageHeadInit(ADM_PIS_PACKAGE_HEAD_T *head){
    head->ecuReqId = NULL;
    head->ecuSwDid = NULL;
    head->type = ADM_PIS_PACKAGE_TYPE_BEGIN;
    head->state = ADM_PIS_PACKAGE_STATE_BEGIN;
}
static void pisPackageHeadDeInit(ADM_PIS_PACKAGE_HEAD_T *head){
    if(NULL != head->ecuReqId){adm_free(head->ecuReqId);}
    if(NULL != head->ecuSwDid){adm_free(head->ecuSwDid);}
}
static void pisPackageHeadClone(const ADM_PIS_PACKAGE_HEAD_T *src, ADM_PIS_PACKAGE_HEAD_T *dst){
    if(NULL != src->ecuReqId){dst->ecuReqId = adm_strdup(src->ecuReqId);}
    if(NULL != src->ecuSwDid){dst->ecuSwDid = adm_strdup(src->ecuSwDid);}
    dst->type = src->type;
    dst->state = src->state;
}
static DBOOL pisPackageHeadCompare(const ADM_PIS_PACKAGE_HEAD_T *src, const ADM_PIS_PACKAGE_HEAD_T *dst){
    DBOOL dRet = DFALSE;
    if(NULL != src->ecuReqId && NULL != src->ecuSwDid && NULL != dst->ecuReqId && NULL != dst->ecuSwDid){
	if(adm_strcmp(src->ecuReqId, dst->ecuReqId) == 0
		&& adm_strcmp(src->ecuSwDid, dst->ecuSwDid) == 0
		&& src->type == dst->type
		&& src->state == dst->state
		){
	    dRet = DTRUE;
	}
    }
    return dRet;
}
static struct json_object* pisPackageHeadJsonCreate(const ADM_PIS_PACKAGE_HEAD_T *head){
    struct json_object* root = json_object_new_object();
    if(NULL != root){
	if(NULL != head->ecuReqId){
	    struct json_object* ecuReqIdVal = json_object_new_string(head->ecuReqId);
	    if(NULL != ecuReqIdVal){
		    (void)json_object_object_add(root, "ecuReqId", ecuReqIdVal);
	    }
	}

	if(NULL != head->ecuSwDid){
	    struct json_object* ecuSwDidVal = json_object_new_string(head->ecuSwDid);
	    if(NULL != ecuSwDidVal){
		    (void)json_object_object_add(root, "ecuSwDid", ecuSwDidVal);
	    }
	}

	struct json_object* typeVal = json_object_new_int((D32S)head->type);
	if(NULL != typeVal){
	    (void)json_object_object_add(root, "type", typeVal);
	}

	struct json_object* stateVal = json_object_new_int((D32S)head->state);
	if(NULL != stateVal){
	    (void)json_object_object_add(root, "state", stateVal);
	}
    }
    return root;
}
static void pisPackageHeadJsonParse(ADM_PIS_PACKAGE_HEAD_T *head, const struct json_object* root){
    struct json_object* ecuReqIdVal = json_object_object_get(root, "ecuReqId");
    if(NULL != ecuReqIdVal){
        const DCHAR* ecuReqId = json_object_get_string(ecuReqIdVal);
        if (NULL != ecuReqId) {
            head->ecuReqId = adm_strdup(ecuReqId);
        }
    }
    struct json_object* ecuSwDidVal = json_object_object_get(root, "ecuSwDid");
    if(NULL != ecuSwDidVal){
        const DCHAR* ecuSwDid = json_object_get_string(ecuSwDidVal);
        if (NULL != ecuSwDid) {
            head->ecuSwDid = adm_strdup(ecuSwDid);
        }
    }
    struct json_object* typeVal = json_object_object_get(root, "type");
    if(NULL != typeVal){
	    head->type = (ADM_PIS_PACKAGE_TYPE_E)json_object_get_int(typeVal);
    }
    struct json_object* stateVal = json_object_object_get(root, "state");
    if(NULL != stateVal){
	    head->state = (ADM_PIS_PACKAGE_STATE_E)json_object_get_int(stateVal);
    }
}

static struct json_object* pisPackageHeadJsonCreateDebug(const ADM_PIS_PACKAGE_HEAD_T *head){
    struct json_object* root = pisPackageHeadJsonCreate(head);
    if(NULL != root){
        struct json_object* typestringVal = json_object_new_string(pisPackageTypeString(head->type));
        if (NULL != typestringVal) {
            (void)json_object_object_add(root, "typestring", typestringVal);
        }

        struct json_object* statestringVal = json_object_new_string(pisPackageStateString(head->state));
        if (NULL != statestringVal) {
            (void)json_object_object_add(root, "statestring", statestringVal);
        }
    }
    return root;
}

/* ADM_PIS_FILE_T */
static void pisFileInit(ADM_PIS_FILE_T *file){
    file->path = NULL;
    file->hash = NULL;
    file->size = 0;
}
static void pisFileDeInit(ADM_PIS_FILE_T *file){
    if(NULL != file->path){adm_free(file->path);}
    if(NULL != file->hash){adm_free(file->hash);}
}
static void pisFileClone(const ADM_PIS_FILE_T *src, ADM_PIS_FILE_T *dst){
    if(NULL != src->path){dst->path = adm_strdup(src->path);}
    if(NULL != src->hash){dst->hash = adm_strdup(src->hash);}
    dst->size = src->size;
}
static struct json_object* pisFileJsonCreate(const ADM_PIS_FILE_T *file){
    struct json_object* root = json_object_new_object();
    if(NULL != root){
	if(NULL != file->path){
	    struct json_object* pathVal = json_object_new_string(file->path);
	    if(NULL != pathVal){
		    (void)json_object_object_add(root, "path", pathVal);
	    }
	}
	if(NULL != file->hash){
	    struct json_object* hashVal = json_object_new_string(file->hash);
	    if(NULL != hashVal){
		    (void)json_object_object_add(root, "hash", hashVal);
	    }
	}
	struct json_object* sizeVal = json_object_new_int64(file->size);
	if(NULL != sizeVal){
	    (void)json_object_object_add(root, "size", sizeVal);
	}
    }
    return root;
}
static void pisFileJsonParse(ADM_PIS_FILE_T *file, const struct json_object* root){
    struct json_object* pathVal = json_object_object_get(root, "path");
    if(NULL != pathVal){
        const DCHAR* path = json_object_get_string(pathVal);
        if (NULL != path) {
            file->path = adm_strdup(path);
        }
    }
    struct json_object* hashVal = json_object_object_get(root, "hash");
    if(NULL != hashVal){
	    const DCHAR *hash = json_object_get_string(hashVal);
        if (NULL != hash) {
            file->hash = adm_strdup(hash);
        }
    }
    struct json_object* sizeVal = json_object_object_get(root, "size");
    if(NULL != sizeVal){
	    file->size = json_object_get_int64(sizeVal);
    }
}

#if 0

/* ADM_COM_FILEMETA_T */
static struct json_object* comFileMetaJsonCreate(ADM_COM_FILEMETA_T *fm){
    struct json_object* root = json_object_new_object();
    if(NULL != root){
        struct json_object* checkSumVal = json_object_new_object();
        if (NULL != checkSumVal) {
            if (NULL != fm->hash) {
                struct json_object* hashVal = json_object_new_string(fm->hash);
                if (NULL != hashVal) {
                    (void)json_object_object_add(checkSumVal, "hash", hashVal);
                }
            }
            struct json_object* algorithmVal = json_object_new_int(fm->algorithm);
            if (NULL != algorithmVal) {
                (void)json_object_object_add(checkSumVal, "algorithm", algorithmVal);
            }

            (void)json_object_object_add(root, "checkSum", checkSumVal);
        }
        struct json_object* typeVal = json_object_new_int(fm->type);
        if (NULL != typeVal) {
            (void)json_object_object_add(root, "type", typeVal);
        }
        struct json_object* sizeVal = json_object_new_int64(fm->size);
        if (NULL != sizeVal) {
            (void)json_object_object_add(root, "size", sizeVal);
        }
    }
    return root;
}

/* ADM_COM_FILESCYINFO_T */
static struct json_object* comFileScyInfoJsonCreate(ADM_COM_FILESCYINFO_T *fsi){
    struct json_object* root = json_object_new_object();
    if(NULL != root){
        if (NULL != fsi->oriHash) {
            struct json_object* oriHashVal = json_object_new_string(fsi->oriHash);
            if (NULL != oriHashVal) {
                (void)json_object_object_add(root, "oriHash", oriHashVal);
            }
        }
        if (NULL != fsi->pkgKey) {
            struct json_object* pkgKeyVal = json_object_new_string(fsi->pkgKey);
            if (NULL != pkgKeyVal) {
                (void)json_object_object_add(root, "pkgKey", pkgKeyVal);
            }
        }
        struct json_object* isEncryptVal = json_object_new_int(fsi->isEncrypt);
        if (NULL != isEncryptVal) {
            (void)json_object_object_add(root, "isEncrypt", isEncryptVal);
        }
        struct json_object* segmentDecryptLenVal = json_object_new_int(fsi->segmentDecryptLen);
        if (NULL != segmentDecryptLenVal) {
            (void)json_object_object_add(root, "segmentDecryptLen", segmentDecryptLenVal);
        }
        struct json_object* oriSizeVal = json_object_new_int64(fsi->oriSize);
        if (NULL != oriSizeVal) {
            (void)json_object_object_add(root, "oriSize", oriSizeVal);
        }
        if (0 != fsi->fileMetaNum && NULL != fsi->pstFileMetaList) {
            struct json_object* fileMetasVal = json_object_new_array();
            if (NULL != fileMetasVal) {
                for (D32S i = 0; i < fsi->fileMetaNum; ++i) {
                    struct json_object* fileMetaVal = comFileMetaJsonCreate(&fsi->pstFileMetaList[i]);
                    if (NULL != fileMetaVal) {
                        (void)json_object_array_add(fileMetasVal, fileMetaVal);
                    }
                }
                (void)json_object_object_add(root, "fileMetas", fileMetasVal);
            }
        }
    }
    return root;
}

#endif

/* ADM_PIS_PACKAGE_DATA_T */
static void pisPackageDataInit(ADM_PIS_PACKAGE_DATA_T *data){
    pisFileInit(&data->imagePackage);
    data->signPackage = NULL;
    data->fileSecurityInfo = NULL;
}
static void pisPackageDataDeInit(ADM_PIS_PACKAGE_DATA_T *data){
    pisFileDeInit(&data->imagePackage);
    if(NULL != data->signPackage){
        pisFileDeInit(data->signPackage);
        adm_free(data->signPackage);
    }
    if(NULL != data->fileSecurityInfo){
	    adm_com_delFileSecurityInfo(&data->fileSecurityInfo);
    }
}
static void pisPackageDataClone(const ADM_PIS_PACKAGE_DATA_T *src, ADM_PIS_PACKAGE_DATA_T *dst){
    pisFileClone(&src->imagePackage, &dst->imagePackage);
    if(NULL != src->signPackage){
        dst->signPackage = adm_malloc((D32U)sizeof(*(dst->signPackage)));
        if (NULL != dst->signPackage) {
            pisFileInit(dst->signPackage);
            pisFileClone(src->signPackage, dst->signPackage);
        }
    }
    dst->makePackageType=src->makePackageType;
    if(NULL != src->fileSecurityInfo){
	    dst->fileSecurityInfo = adm_com_cloneFileSecurityInfo(src->fileSecurityInfo);
    }
}
static struct json_object* pisPackageDataJsonCreate(const ADM_PIS_PACKAGE_DATA_T *data){
    struct json_object* root = json_object_new_object();
    if(NULL != root){
        struct json_object* imagePackageVal = pisFileJsonCreate(&data->imagePackage);
        if (NULL != imagePackageVal) {
            (void)json_object_object_add(root, "imagePackage", imagePackageVal);
        }
        if (NULL != data->signPackage) {
            struct json_object* signPackageVal = pisFileJsonCreate(data->signPackage);
            if (NULL != signPackageVal) {
                (void)json_object_object_add(root, "signPackage", signPackageVal);
            }
        }
        struct json_object* typeVal = json_object_new_int(data->makePackageType);
        if (NULL != typeVal) {
                (void)json_object_object_add(root, "makePackageType", typeVal);
        }
        if (NULL != data->fileSecurityInfo) {
            struct json_object* fileSecurityInfoVal = json_object_new_object();
            if (NULL != fileSecurityInfoVal) {
                (void)adm_com_addFileSecurityInfo2JsonNode(data->fileSecurityInfo, fileSecurityInfoVal);
                (void)json_object_object_add(root, "fileSecurityInfo", fileSecurityInfoVal);
            }
        }
    }
    return root;
}
static void pisPackageDataJsonParse(ADM_PIS_PACKAGE_DATA_T *data, const struct json_object* root){
    struct json_object* imagePackageVal = json_object_object_get(root, "imagePackage");
    if(NULL != imagePackageVal){
	    pisFileJsonParse(&data->imagePackage, imagePackageVal);
    }
    struct json_object* signPackageVal = json_object_object_get(root, "signPackage");
    if(NULL != signPackageVal){
        data->signPackage = adm_malloc((D32U)sizeof(*(data->signPackage)));
        if (NULL != data->signPackage) {
            pisFileInit(data->signPackage);
            pisFileJsonParse(data->signPackage, signPackageVal);
        }
    }
    struct json_object* makePackageType = json_object_object_get(root, "makePackageType");
    if(NULL != makePackageType){
	    data->makePackageType = json_object_get_int64(makePackageType);
    }
    struct json_object* fileSecurityInfoVal = json_object_object_get(root, "fileSecurityInfo");
    if(NULL != fileSecurityInfoVal){
	    (void)adm_com_parseJsonFileSecurityInfo(fileSecurityInfoVal, &data->fileSecurityInfo);
    }
}
static void pisPackageDataSetImage(ADM_PIS_PACKAGE_DATA_T *data, const ADM_PIS_FILE_T *imagePackage){
    pisFileDeInit(&data->imagePackage);
    pisFileInit(&data->imagePackage);
    pisFileClone(imagePackage, &data->imagePackage);
}
static void pisPackageDataSetSign(ADM_PIS_PACKAGE_DATA_T *data, const ADM_PIS_FILE_T *signPackage){
    if(NULL == data->signPackage){
        data->signPackage = adm_malloc((D32U)sizeof(*(data->signPackage)));
        if (NULL != data->signPackage) {
            pisFileInit(data->signPackage);
            pisFileClone(signPackage, data->signPackage);
        }
    }
    else {
        pisFileDeInit(data->signPackage);
        pisFileInit(data->signPackage);
        pisFileClone(signPackage, data->signPackage);
    }
}
static void pisPackageDataSetSecurity(ADM_PIS_PACKAGE_DATA_T *data, const ADM_COM_FILESCYINFO_T *fileSecurityInfo){
    if(NULL != data->fileSecurityInfo){
	    adm_com_delFileSecurityInfo(&data->fileSecurityInfo);
    }
    data->fileSecurityInfo = adm_com_cloneFileSecurityInfo(fileSecurityInfo);
}

/* ADM_PIS_PACKAGE_NODE_T */
static ADM_PIS_PACKAGE_NODE_T *pisPackageNodeNewWithNull(void){
    ADM_PIS_PACKAGE_NODE_T *node = adm_malloc((D32U)sizeof(*node));
    if(NULL != node){
        pisPackageHeadInit(&node->head);
        pisPackageDataInit(&node->data);
    }
    return node;
}
static ADM_PIS_PACKAGE_NODE_T *pisPackageNodeNewWithHead(const ADM_PIS_PACKAGE_HEAD_T *head){
    ADM_PIS_PACKAGE_NODE_T *node = pisPackageNodeNewWithNull();
    if(NULL != node){
	    pisPackageHeadClone(head, &node->head);
    }
    return node;
}
static ADM_PIS_PACKAGE_NODE_T *pisPackageNodeNew(const ADM_PIS_PACKAGE_HEAD_T *head, const ADM_PIS_PACKAGE_DATA_T *data){
    ADM_PIS_PACKAGE_NODE_T *node = pisPackageNodeNewWithNull();
    if(NULL != node){
        pisPackageHeadClone(head, &node->head);
        pisPackageDataClone(data, &node->data);
    }
    return node;
}
static void pisPackageNodeDel(ADM_PIS_PACKAGE_NODE_T *node){
    pisPackageHeadDeInit(&node->head);
    pisPackageDataDeInit(&node->data);
    adm_free(node);
}
static void pisPackageNodeSetData(ADM_PIS_PACKAGE_NODE_T *node, const ADM_PIS_PACKAGE_DATA_T *data){
    pisPackageDataDeInit(&node->data);
    pisPackageDataInit(&node->data);
    pisPackageDataClone(data, &node->data);
}
static struct json_object* pisPackageNodeJsonCreate(const ADM_PIS_PACKAGE_NODE_T *node){
    struct json_object* root = json_object_new_object();
    if(NULL != root){
        struct json_object* headVal = pisPackageHeadJsonCreate(&node->head);
        if (NULL != headVal) {
            (void)json_object_object_add(root, "head", headVal);
        }
        struct json_object* dataVal = pisPackageDataJsonCreate(&node->data);
        if (NULL != dataVal) {
            (void)json_object_object_add(root, "data", dataVal);
        }
    }
    return root;
}
static ADM_PIS_PACKAGE_NODE_T* pisPackageNodeJsonParse(const struct json_object* root){
    ADM_PIS_PACKAGE_NODE_T* node = pisPackageNodeNewWithNull();
    if(NULL != node){
        struct json_object* headVal = json_object_object_get(root, "head");
        if (NULL != headVal) {
            pisPackageHeadJsonParse(&node->head, headVal);
        }
        struct json_object* dataVal = json_object_object_get(root, "data");
        if (NULL != dataVal) {
            pisPackageDataJsonParse(&node->data, dataVal);
        }
    }
    return node;
}

/* ADM_PIS_PACKAGE_LIST_T */
static void pisPackageListInit(ADM_PIS_PACKAGE_LIST_T *package){
    (void)pthread_mutex_init(&package->mutex, NULL);
    INIT_LIST_HEAD(&package->head);
    pisPackageListJsonLoad(package);
}
static void pisPackageListDeInit(ADM_PIS_PACKAGE_LIST_T *package){
    ADM_PIS_PACKAGE_NODE_T *p, *n;
    list_for_each_entry_safe(p, n, &package->head, ADM_PIS_PACKAGE_NODE_T, entry){
        list_del(&p->entry);
        pisPackageNodeDel(p);
    }
    (void)pthread_mutex_destroy(&package->mutex);
}
static void pisPackageListSet(ADM_PIS_PACKAGE_LIST_T *package, const ADM_PIS_PACKAGE_HEAD_T *head, const ADM_PIS_PACKAGE_DATA_T *data){
    (void)pthread_mutex_lock(&package->mutex);
    ADM_PIS_PACKAGE_NODE_T* node = pisPackageListFind(package, head);
    if(NULL != node){
	    pisPackageNodeSetData(node, data);
    }else{
        node = pisPackageNodeNew(head, data);
        if(NULL != node){
            list_add_tail(&node->entry, &package->head);
        }
    }
    pisPackageListJsonStore(package);
    PIS_PACKAGE_NODE_SHOW(node);
    (void)pthread_mutex_unlock(&package->mutex);
}
static void pisPackageListSetImage(ADM_PIS_PACKAGE_LIST_T *package, const ADM_PIS_PACKAGE_HEAD_T *head, const ADM_PIS_FILE_T *imagePackage){
    (void)pthread_mutex_lock(&package->mutex);
    ADM_PIS_PACKAGE_NODE_T* node = pisPackageListFind(package, head);
    if(NULL != node){
	    pisPackageDataSetImage(&node->data, imagePackage);
    }else{
        node = pisPackageNodeNewWithHead(head);
        if(NULL != node){
	        pisPackageDataSetImage(&node->data, imagePackage);
            list_add_tail(&node->entry, &package->head);
        }
    }
    pisPackageListJsonStore(package);
    PIS_PACKAGE_NODE_SHOW(node);
    (void)pthread_mutex_unlock(&package->mutex);
}
static void pisPackageListSetSign(ADM_PIS_PACKAGE_LIST_T *package, const ADM_PIS_PACKAGE_HEAD_T *head, const ADM_PIS_FILE_T *signPackage){
    (void)pthread_mutex_lock(&package->mutex);
    ADM_PIS_PACKAGE_NODE_T* node = pisPackageListFind(package, head);
    if(NULL != node){
	    pisPackageDataSetSign(&node->data, signPackage);
    }else{
        node = pisPackageNodeNewWithHead(head);
        if(NULL != node){
	        pisPackageDataSetSign(&node->data, signPackage);
            list_add_tail(&node->entry, &package->head);
        }
    }
    pisPackageListJsonStore(package);
    PIS_PACKAGE_NODE_SHOW(node);
    (void)pthread_mutex_unlock(&package->mutex);
}
static void pisPackageListSetMakeType(ADM_PIS_PACKAGE_LIST_T *package, const ADM_PIS_PACKAGE_HEAD_T *head, const D32S makePackageType){
    (void)pthread_mutex_lock(&package->mutex);
    ADM_PIS_PACKAGE_NODE_T* node = pisPackageListFind(package, head);
    if(NULL != node){
	    node->data.makePackageType=makePackageType;
    }
    pisPackageListJsonStore(package);
    PIS_PACKAGE_NODE_SHOW(node);
    (void)pthread_mutex_unlock(&package->mutex);
}
static void pisPackageListSetSecurity(ADM_PIS_PACKAGE_LIST_T *package, const ADM_PIS_PACKAGE_HEAD_T *head, const ADM_COM_FILESCYINFO_T *fileSecurityInfo){
    (void)pthread_mutex_lock(&package->mutex);
    ADM_PIS_PACKAGE_NODE_T* node = pisPackageListFind(package, head);
    if(NULL != node){
	    pisPackageDataSetSecurity(&node->data, fileSecurityInfo);
    }else{
        node = pisPackageNodeNewWithHead(head);
        if(NULL != node){
            pisPackageDataSetSecurity(&node->data, fileSecurityInfo);
            list_add_tail(&node->entry, &package->head);
        }
    }
    pisPackageListJsonStore(package);
    PIS_PACKAGE_NODE_SHOW(node);
    (void)pthread_mutex_unlock(&package->mutex);
}
static ADM_PIS_PACKAGE_DATA_T* pisPackageListGet(ADM_PIS_PACKAGE_LIST_T *package, const ADM_PIS_PACKAGE_HEAD_T *head, ADM_PIS_PACKAGE_DATA_T *data){
    ADM_PIS_PACKAGE_DATA_T *resdata = NULL;
    (void)pthread_mutex_lock(&package->mutex);
    ADM_PIS_PACKAGE_NODE_T* node = pisPackageListFind(package, head);
    PIS_PACKAGE_NODE_SHOW(node);
    if(NULL != node){
	    if(NULL == node->data.imagePackage.path || NULL == node->data.imagePackage.hash){}else{
	        pisPackageDataClone(&node->data, data);
	        resdata = data;
	    }
    }
    (void)pthread_mutex_unlock(&package->mutex);
    return resdata;
}
static ADM_PIS_PACKAGE_NODE_T* pisPackageListFind(ADM_PIS_PACKAGE_LIST_T *package, const ADM_PIS_PACKAGE_HEAD_T *head){
    ADM_PIS_PACKAGE_NODE_T* node = NULL;
    ADM_PIS_PACKAGE_NODE_T *p;
    list_for_each_entry(p, &package->head, ADM_PIS_PACKAGE_NODE_T, entry){
        if (DTRUE == pisPackageHeadCompare(head, &p->head)) {
            node = p;
            break;
        }
    }
    return node;
}
static struct json_object* pisPackageListJsonCreate(const ADM_PIS_PACKAGE_LIST_T *package){
    struct json_object* root = json_object_new_array();
    if(NULL != root){
        ADM_PIS_PACKAGE_NODE_T* pos;
        list_for_each_entry(pos, &package->head, ADM_PIS_PACKAGE_NODE_T, entry) {
            struct json_object* posVal = pisPackageNodeJsonCreate(pos);
            if (NULL != posVal) {
                (void)json_object_array_add(root, posVal);
            }
        }
    }
    return root;
}
static void pisPackageListJsonParse(ADM_PIS_PACKAGE_LIST_T *package, const struct json_object* root){
    if(json_type_array == json_object_get_type(root)){
        size_t length = json_object_array_length(root);
        for (size_t i = 0; i < length; ++i) {
            struct json_object* posVal = json_object_array_get_idx(root, i);
            if (NULL != posVal) {
                ADM_PIS_PACKAGE_NODE_T* node = pisPackageNodeJsonParse(posVal);
                list_add_tail(&node->entry, &package->head);
            }
        }
    }
}
static void pisPackageListJsonLoad(ADM_PIS_PACKAGE_LIST_T *package){
    ADM_KV_STORAGE_VALUE_TYPE_E eType = ADM_KV_STORAGE_VALUE_TYPE_INVALID;
    DCHAR *packageJsonStr = NULL;
    size_t packageJsonStrLen = 0;
    if(adm_com_kvstorage_get_string(adm_core_service_getBusinessDatabaseDb(), ADM_PIS_PACKAGE_LIST_KV_KEY,
                &packageJsonStr, &packageJsonStrLen, &eType)){
        if (NULL != packageJsonStr) {
            struct json_object* packageVal = json_tokener_parse(packageJsonStr);
            if (NULL != packageVal) {
                pisPackageListJsonParse(package, packageVal);
                (void)json_object_put(packageVal);
            }
            adm_free(packageJsonStr);
        }
    }
}
static void pisPackageListJsonStore(const ADM_PIS_PACKAGE_LIST_T *package){
    struct json_object* root = pisPackageListJsonCreate(package);
    if(NULL != root){
        const DCHAR* packageJsonStr = json_object_to_json_string(root);
        if (NULL != packageJsonStr) {
            (void)adm_com_kvstorage_set_string(adm_core_service_getBusinessDatabaseDb(), ADM_PIS_PACKAGE_LIST_KV_KEY,
                packageJsonStr, ADM_KV_STORAGE_VALUE_TYPE_STRING);
        }
        (void)json_object_put(root);
    }
}

/* ADM_PIS_PACKAGE_STATE_E */
static const DCHAR *pisPackageStateString(ADM_PIS_PACKAGE_STATE_E state){
    const DCHAR *strstate = "ADM_PIS_PACKAGE_STATE_BEGIN";
    switch(state){
	case ADM_PIS_PACKAGE_STATE_DOWNLOAD_FINISH:strstate = ADM_STR_FROM_PARAM(ADM_PIS_PACKAGE_STATE_DOWNLOAD_FINISH); break;
	case ADM_PIS_PACKAGE_STATE_DECRYPT_FINISH:strstate = ADM_STR_FROM_PARAM(ADM_PIS_PACKAGE_STATE_DECRYPT_FINISH); break;
	case ADM_PIS_PACKAGE_STATE_VERIFY_SIGN_FINISH:strstate = ADM_STR_FROM_PARAM(ADM_PIS_PACKAGE_STATE_VERIFY_SIGN_FINISH); break;
	default: break;
    }
    return strstate;
}

/* ADM_PIS_PACKAGE_TYPE_E */
static const DCHAR *pisPackageTypeString(ADM_PIS_PACKAGE_TYPE_E type){
    const DCHAR *strtype = "ADM_PIS_PACKAGE_TYPE_BEGIN";
    switch(type){
	case ADM_PIS_PACKAGE_TYPE_ECU_SRC_PACKAGE:strtype = ADM_STR_FROM_PARAM(ADM_PIS_PACKAGE_TYPE_ECU_SRC_PACKAGE);break;
	case ADM_PIS_PACKAGE_TYPE_ECU_DST_PACKAGE:strtype = ADM_STR_FROM_PARAM(ADM_PIS_PACKAGE_TYPE_ECU_DST_PACKAGE);break;
	default:break;
    }
    return strtype;
}
