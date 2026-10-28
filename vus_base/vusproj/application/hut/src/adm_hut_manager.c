
#include "adm_hut_manager.h"
#include "adm_hut_comm.h"


char* m_SessionID[10];
void* m_Session[10];
D32S  m_CurrentIndex = 0;
D32S  m_Init = 0;

/**
 * @brief 管理模块初始化
 * 
 * @return D32S     0:successed, non-0:failed 
 */
D32S InitManager()
{
    if(1 == m_Init)
    {
        return 0;
    }
    D32S i = 0;
    for(i = 0; i < 10;i++)
    {
        m_SessionID[i] = 0;
        m_Session[i] = 0;
    }

    D32S iret = 0;
    iret = mqtt_lib_init_hut();
    if(iret != 0)
    {
        return -1;
    }
    m_Init = 1;
    return 0;
}

/**
 * @brief 启动通信
 * 
 * @return D32S     0:successed, non-0:failed 
 */
D32S StartComm(iNodeInfo_HUT* pNodeInfo)
{
    if(NULL == pNodeInfo)
    {
        return -1;
    }

    MQTTAsync pClientSession = mqtt_create_hut(pNodeInfo);
    if(NULL == pClientSession)
    {
        return -1;
    }

    m_SessionID[m_CurrentIndex] = pNodeInfo->inSessionID;
    m_Session[m_CurrentIndex]   = pClientSession;
    m_CurrentIndex++;    
    return 0;
}

/**
 * @brief 发送消息
 * 
 * @return D32S     0:successed, non-0:failed 
 */
D32S SendMgr(DCHAR* pSessionID,DCHAR* pMessage,DCHAR* pTopic)
{
    if(NULL == pSessionID)
    {
        return -1;
    }
    D32S iFind  = 0;
    D32S iIndex = 0;
    for(iIndex = 0; iIndex < 10;iIndex++)
    {
        if(0 == adm_strcmp(pSessionID,m_SessionID[iIndex]))
        {
            iFind = 1;
            break;
        }
    }
    if(0 == iFind)
    {
        printf("Not Find Correct Session\r\n");
        return -1;
    }
    void* clientSession = m_Session[iIndex];
    
    D32S iret = mqtt_send_hut(clientSession,pMessage,adm_strlen(pMessage),pTopic);
    return iret;
}


/**
 * @brief 停止通信
 * 
 * @return D32S     0:successed, non-0:failed 
 */
D32S StopComm()
{
    mqtt_lib_destory_hut();
    return 0;
}

