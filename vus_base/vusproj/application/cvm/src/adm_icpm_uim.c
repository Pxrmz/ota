#include "adm_icpm_uim.h"
#include "adm_com_modDefine.h"
#include "adm_com_errorCode.h"
#include "adm_application_common.h"
#include "adm_uim_request.h"
#include "adm_icp_kvStorage.h"
#include "adm_string.h"
#include "adm_memory.h"
#include <time.h>

/**
 * 从数据库获取icp数据
 * 
 */
void adm_icpm_queryResultCallback(
    DCHAR** sessionId,
    DCHAR** state,
    DCHAR** ppstIcpInfo,
    D32S* errorCode,
    void* priv)
{
    log_i("adm_icp_queryResultCallback start");
    priv = NULL;
    int ret=0;
    ADM_ICP_RSDATA_T * icpData=adm_icp_getOtaIcpDataFromKV();
    if(ADM_IS_NULL(icpData)){
          log_i("adm_icp_queryResultCallback NULL");
          return;
    }
    * ppstIcpInfo=icpData->icpInfo;
    log_i("Icp version data is:%s",icpData->icpVersion);
    log_i("Icp info data is:%s",* ppstIcpInfo);
    log_i("adm_icp_queryResultCallback end");
    return;
}