#include "adm_hut_comm.h"
#include "adm_hut_check.h"
#include "adm_hut_download.h"
#include "adm_hut_install.h"
#include <unistd.h>


D32S      QOS              = 1;
D32S      m_Connect        = 0;
static    MQTTAsync mqtt_clientHandle = NULL;

/*
 * @brief           连接回调触发函数
 * @param inParam   void* context
 * @param inParam   MQTTAsync_successData* response    
 * @return void 
 */
void on_connect_hut(void* context, MQTTAsync_successData* response)
{
    MQTTAsync client = (MQTTAsync)context;
    MQTTAsync_responseOptions opts = MQTTAsync_responseOptions_initializer;
    D32S rc;
    printf("Successful connection\n");
    DCHAR* szCheckTopic = "/vus/dn/check";

    if ((rc = MQTTAsync_subscribe(client, szCheckTopic, QOS, &opts)) != MQTTASYNC_SUCCESS)
    {
        printf("Failed to  subscribe topic=%s, return code %d\r\n",szCheckTopic,rc);
        m_Connect = 0;
        return;
    }
    DCHAR* szDownLoadTopic = "/vus/dn/download";
    if((rc = MQTTAsync_subscribe(client, szDownLoadTopic, QOS, &opts)) != MQTTASYNC_SUCCESS)
    {
        printf("Failed to  subscribe topic=%s, return code %d\r\n", szDownLoadTopic,rc);
        m_Connect = 0;
        return;
    }
    DCHAR* szInstallTopic = "/vus/dn/install";
    if((rc = MQTTAsync_subscribe(client, szInstallTopic, QOS, &opts)) != MQTTASYNC_SUCCESS)
    {
        printf("Failed to  subscribe topic=%s, return code %d\r\n", szDownLoadTopic,rc);
        m_Connect = 0;
        return;
    }    
    m_Connect = 1;
    printf("success subscribe all Topic\r\n");
    return;    
}

/*
 * @brief           连接断开回调触发函数
 * @param inParam   void* context
 * @param inParam   MQTTAsync_failureData* response    
 * @return void 
 */
void on_disconnect_hut(void* context, MQTTAsync_failureData* response)
{
    printf("disconnect,Connect failed,errorcode =%s\r\n",MQTTAsync_strerror(response->code));
    return;    
}

/*
 * @brief           TOPIC订阅成功回调触发函数
 * @param inParam   void* context
 * @param inParam   MQTTAsync_successData* response    
 * @return void 
 */
void on_subscribe_hut(void* context, MQTTAsync_successData* response)
{
    printf("Subscribe succeeded\r\n");
    return;    
}

/*
 * @brief           TOPIC订阅失败回调触发函数
 * @param inParam   void* context
 * @param inParam   MQTTAsync_failureData* response    
 * @return void 
 */
void on_SubscribeFailure_hut(void* context, MQTTAsync_failureData* response)
{
    printf("Subscribe failed, rc %d\n", response ? response->code : 0);
    return;    
}

/*
 * @brief           接收到payload消息回调触发函数
 * @param inParam   void* context
 * @param inParam   char *topicName
 * @param inParam   int topicLen
 * @param inParam   MQTTAsync_message *message
 * @return int 
 */
D32S  on_message_hut(void *context, DCHAR *topicName, D32S topicLen, MQTTAsync_message *message)
{
    void* pClientSession = context;
    D32S i;
    DCHAR* payloadptr;
    printf("Message arrived\n");

    payloadptr = message->payload;
    if(0 == adm_strcmp(topicName,"/vus/dn/check"))
    {
        //执行检测
        printf("topic =%s\r\n",topicName);
        adm_hut_checkwork(pClientSession);
    }
    else if(0 == adm_strcmp(topicName,"/vus/dn/download"))
    {
        //执行下载
        printf("topic =%s\r\n",topicName);
        adm_hut_download(pClientSession);
    }
    else if(0 == adm_strcmp(topicName,"/vus/dn/install"))
    {
        //执行安装
        printf("topic =%s\r\n",topicName);
        adm_hut_install(pClientSession);
    }
    else
    {
        printf("can't distinguish topic = %s\n", topicName);
    }
    MQTTAsync_freeMessage(&message);
    MQTTAsync_free(topicName);
    return 1;
}

/*
 * @brief           Public消息成功回调触发函数
 * @param inParam   void* context
 * @param inParam   MQTTAsync_successData* response    
 * @return void 
 */
void on_Send_hut(void* context, MQTTAsync_successData* response)
{
    return;
}

/*
 * @brief           mqtt 库初始化函数
 * @param inParam   void   
 * @return int      0:successed, non-0:failed 
 */
D32S mqtt_lib_init_hut()
{
    return 0;
}

/*
 * @brief           mqtt 库销毁函数
 * @param inParam   void   
 * @return int      0:successed, non-0:failed 
 */
D32S mqtt_lib_destory_hut()
{
    MQTTAsync_destroy(&mqtt_clientHandle);
    return 0;
}

/*
 * @brief             创建一个MQTT连接通道
 * @param inParam     const char* host
 * @param inParam     int iPort
 * @return MQTTAsync  连接通道句柄 
 */
MQTTAsync mqtt_create_hut(iNodeInfo_HUT* inNodeInfo)
{
    if(NULL == inNodeInfo)
    {
        return NULL;
    }
    QOS = inNodeInfo->inQOS;

    MQTTAsync_connectOptions conn_opts = MQTTAsync_connectOptions_initializer;
    MQTTAsync_message pubmsg = MQTTAsync_message_initializer;
    MQTTAsync_token token;
    D32S rc;
    DCHAR szAddress[32]={0};
    DCHAR* pDst = szAddress;
    DCHAR* tcp = "tcp://";
    adm_strncpy(pDst, tcp,adm_strlen(tcp),sizeof(szAddress));
    adm_strncpy(pDst+adm_strlen(pDst),inNodeInfo->inHostName,adm_strlen(inNodeInfo->inHostName),sizeof(szAddress));

    DCHAR szPort[6] ={0};
    adm_snprintf(szPort,6,":%d",inNodeInfo->iPort);
    adm_strncpy(pDst+adm_strlen(pDst),szPort,adm_strlen(szPort),sizeof(szAddress));
    pDst[adm_strlen(pDst)+1]='\0';

    rc = MQTTAsync_create(&mqtt_clientHandle, pDst,inNodeInfo->inSessionID, MQTTCLIENT_PERSISTENCE_NONE, NULL);
    if(rc != MQTTASYNC_SUCCESS)
    {
        printf("Failed to create client object, return code %d\n", rc);

        return NULL;
    }
    rc = MQTTAsync_setCallbacks(mqtt_clientHandle, mqtt_clientHandle, NULL, on_message_hut, NULL);
    if(rc != MQTTASYNC_SUCCESS)
    {
        printf("Failed to create client object, return code %d\n", rc);
        return NULL;
    }

    conn_opts.automaticReconnect = 1;
    conn_opts.minRetryInterval = 1; //seconds
    conn_opts.maxRetryInterval = 365*24*60*60;
    conn_opts.keepAliveInterval = 20;
    conn_opts.cleansession = 1;
    conn_opts.onSuccess = on_connect_hut;
    conn_opts.onFailure = on_disconnect_hut;
    conn_opts.context = mqtt_clientHandle;
    conn_opts.username  = inNodeInfo->inUserName;
    conn_opts.password  = inNodeInfo->inPasswd;
    if ((rc = MQTTAsync_connect(mqtt_clientHandle, &conn_opts)) != MQTTASYNC_SUCCESS)
    {
        printf("Failed to start connect, return code %d\n", rc);
        return NULL;
    }
    return mqtt_clientHandle;
}

/*
 * @brief             数据发送函数
 * @param inParam     uint8_t * buffer
 * @param inParam     size_t length
 * @return int        0:successed, non-0:failed
 */
D32S mqtt_send_hut(void* pSession,DCHAR* buffer, size_t length,DCHAR* pTopic)
{
    if(NULL == mqtt_clientHandle || NULL == pTopic || NULL == buffer || length <= 0)
    {
        return -1;        
    }

    while(!m_Connect)
    {
        usleep(1);
    }

    MQTTAsync client = (MQTTAsync)(pSession);
    MQTTAsync_responseOptions opts = MQTTAsync_responseOptions_initializer;
    MQTTAsync_message pubmsg = MQTTAsync_message_initializer;
    D32S rc;

    pubmsg.payload = (void*)buffer;
    pubmsg.payloadlen = (D32S)length;
    pubmsg.qos = QOS;
    pubmsg.retained = 0;

    if ((rc = MQTTAsync_sendMessage(client, pTopic, &pubmsg, &opts)) != MQTTASYNC_SUCCESS)
    {
        printf("Failed to start sendMessage, return code %d\n", rc);
        return -1;
    }

    DCHAR* pp = (DCHAR*)pubmsg.payload;
    printf("sendmessage =%s,iLen = %d,topic=%s\r\n",pp,pubmsg.payloadlen,pTopic);
    return 0;
}

