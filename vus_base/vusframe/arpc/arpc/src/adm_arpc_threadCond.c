#include <errno.h>
#include <time.h>
#include "adm_arpc_threadCond.h"
#include "adm_com_errorCode.h"
#include "adm_arpc_log.h"

/**
 * @brief 兼容的pthread_cond_timedwait实现
 *
 * @note 解决JNI的集成方式中,pthread_cond_timedwait函数会被信号3打断且函数返回0的问题
 *       1.增加超时时间自行校验
 *
 * @return D32S
 */
D32S adm_arpc_pthread_cond_timedwait(pthread_cond_t *pstCond, pthread_mutex_t *pstMutex, 
                                    DBOOL* pIsSignal, const struct timespec *pstAbstime)
{
    D32S iRet = ADM_ECODE_FAILED;
    struct timespec stCurrTime;

    if (NULL == pstCond || NULL == pstMutex ||
        NULL == pIsSignal || NULL == pstAbstime)
    {
        log_e("Invalid param.");
        return -1;
    }

    while (DFALSE == *pIsSignal)
    {
        iRet = pthread_cond_timedwait(pstCond, pstMutex, pstAbstime);
        if (ETIMEDOUT == iRet)
        {
            break;
        }
        
        if (ADM_IS_ERR(iRet))
        {
            log_i("Abnormal interrupt. value[%d].", iRet);

            clock_gettime(CLOCK_MONOTONIC, &stCurrTime);

            /** 检查当前时间是否大于超时时间 */
            if (stCurrTime.tv_sec > pstAbstime->tv_sec ||
                (stCurrTime.tv_sec == pstAbstime->tv_sec &&
                 stCurrTime.tv_nsec >= pstAbstime->tv_nsec))
            {
                /** 返回超时 */
                iRet = ETIMEDOUT;
                break;
            }
        }
    }

    return *pIsSignal ? ADM_ECODE_SUCCESSED : iRet;
}