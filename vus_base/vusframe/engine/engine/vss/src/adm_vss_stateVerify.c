#include "adm_vss_log.h"
#include "adm_vss_stateVerify.h"
#include "adm_vss_stateManager.h"
#include "adm_vss_stateOperation.h"

/**
 * @brief 预检测状态的校验，校验是否满足进入预检测状态
 * @details 可以进入预检测的状态，IDLE、POST_CHECK、PRE_DOWNLOAD、DOWNLOAD_PAUSE、PRE_TRANSFER、TRANSFER_PAUSE、POST_DOWNLOAD
 * @return D32S         0:successed, non-0: failed
 */
D32S adm_vss_stateVerifyWhenPreCheck(void)
{
    D32S iRet = ADM_ECODE_FAILED;
    ADM_VSS_STATE_TYPE_E eState = ADM_VSS_STATE_TYPE_INVALID;

    iRet = adm_vss_getStateMachine(&eState);
    if (ADM_ECODE_SUCCESSED != iRet) {
        log_e( "adm_vss_getStateMachine failed, reason of error code is %d", iRet);
    } else {
        if (ADM_VSS_STATE_TYPE_IDLE == eState || ADM_VSS_STATE_TYPE_PRE_CHECK == eState || 
            ADM_VSS_STATE_TYPE_POST_CHECK == eState || ADM_VSS_STATE_TYPE_PRE_DOWNLOAD == eState ||
            ADM_VSS_STATE_TYPE_POST_DOWNLOAD == eState || ADM_VSS_STATE_TYPE_DOWNLOAD_PAUSE == eState ||
            ADM_VSS_STATE_TYPE_PRE_TRANSFER == eState || ADM_VSS_STATE_TYPE_TRANSER_PAUSE == eState) {
                log_i( "State machine verification satisfies, current state is %d, you can enter PRE_CHECK", eState);
                iRet = ADM_ECODE_SUCCESSED;
            } else {
                log_w( "State machine verification is not satisfied current state is %d, you can not enter PRE_CHECK", eState);
                iRet = ADM_ECODE_STATE_NOT_SATISFIED;
            }
    }

    return iRet;
}


/**
 * @brief 检测中状态的校验，校验是否满足进入检测中状态
 * @details 可以进入检测中的状态，IDLE、PRE_CHECK、POST_CHECK、PRE_DOWNLOAD、DOWNLOAD_PAUSE、PRE_TRANSFER、TRANSFER_PAUSE、POST_DOWNLOAD
 * @return D32S         0:successed, non-0: failed
 */
D32S adm_vss_stateVerifyWhenChecking(void)
{
    D32S iRet = ADM_ECODE_FAILED;
    ADM_VSS_STATE_TYPE_E eState = ADM_VSS_STATE_TYPE_INVALID;

    iRet = adm_vss_getStateMachine(&eState);
    if (ADM_ECODE_SUCCESSED != iRet) {
        log_e( "adm_vss_getStateMachine failed, reason of error code is %d", iRet);
    } else {
        if (ADM_VSS_STATE_TYPE_IDLE == eState || ADM_VSS_STATE_TYPE_PRE_CHECK == eState || 
            ADM_VSS_STATE_TYPE_POST_CHECK == eState || ADM_VSS_STATE_TYPE_PRE_DOWNLOAD == eState ||
            ADM_VSS_STATE_TYPE_POST_DOWNLOAD == eState || ADM_VSS_STATE_TYPE_DOWNLOAD_PAUSE == eState ||
            ADM_VSS_STATE_TYPE_PRE_TRANSFER == eState || ADM_VSS_STATE_TYPE_TRANSER_PAUSE == eState) {
                log_i( "State machine verification satisfies, current state is %d, you can enter CHECKING", eState);
                iRet = ADM_ECODE_SUCCESSED;
            } else {
                log_w( "State machine verification is not satisfied current state is %d, you can not enter CHECKING", eState);
                iRet = ADM_ECODE_STATE_NOT_SATISFIED;
            }
    }

    return iRet;
}

/**
 * @brief 检测后状态的校验，校验是否满足进入检测到新版本状态
 * @details 可以进入到检测到新版本的状态，CHECKING
 * @return D32S         0:successed, non-0: failed
 */
D32S adm_vss_stateVerifyWhenPostCheck(void)
{
    D32S iRet = ADM_ECODE_FAILED;
    ADM_VSS_STATE_TYPE_E eState = ADM_VSS_STATE_TYPE_INVALID;

    iRet = adm_vss_getStateMachine(&eState);
    if (ADM_ECODE_SUCCESSED != iRet) {
        log_e( "adm_vss_getStateMachine failed, reason of error code is %d", iRet);
    } else {
        if (ADM_VSS_STATE_TYPE_CHECKING == eState) {
            log_i( "State machine verification satisfies, current state is %d, you can enter POST_CHECK", eState);
                iRet = ADM_ECODE_SUCCESSED;
            } else {
                log_w( "State machine verification is not satisfied current state is %d, you can not enter POST_CHECK", eState);
                iRet = ADM_ECODE_STATE_NOT_SATISFIED;
            }
    }

    return iRet;
}

/**
 * @brief 预下载状态的校验，校验是否满足进入预下载状态
 * @details 可以进入到预下载的状态，POST_CHECK
 * @return D32S         0:successed, non-0: failed
 */
D32S adm_vss_stateVerifyWhenPreDownload(void)
{
    // TODO:
    D32S iRet = ADM_ECODE_FAILED;
    ADM_VSS_STATE_TYPE_E eState = ADM_VSS_STATE_TYPE_INVALID;

    iRet = adm_vss_getStateMachine(&eState);
    if (ADM_ECODE_SUCCESSED != iRet) {
        log_e( "adm_vss_getStateMachine failed, reason of error code is %d", iRet);
    } else {
        if (ADM_VSS_STATE_TYPE_POST_CHECK == eState) {
            log_i( "State machine verification satisfies, current state is %d, you can enter PRE_DOWNLOAD", eState);
                iRet = ADM_ECODE_SUCCESSED;
            } else {
                log_w( "State machine verification is not satisfied current state is %d, you can not enter PRE_DOWNLOAD", eState);
                iRet = ADM_ECODE_STATE_NOT_SATISFIED;
            }
    }

    return iRet;
}

/**
 * @brief 下载中状态的校验，校验是否满足进入下载中状态
 * @details 可以进入到下载中的状态，PRE_DOWNLOAD、DOWNLOADING、DOWNLOAD_PAUSE
 * @return D32S         0:successed, non-0: failed
 */
D32S adm_vss_stateVerifyWhenDownloading(void)
{
    D32S iRet = ADM_ECODE_FAILED;
    ADM_VSS_STATE_TYPE_E eState = ADM_VSS_STATE_TYPE_INVALID;

    iRet = adm_vss_getStateMachine(&eState);
    if (ADM_ECODE_SUCCESSED != iRet) {
        log_e( "adm_vss_getStateMachine failed, reason of error code is %d", iRet);
    } else {
        if (ADM_VSS_STATE_TYPE_POST_CHECK == eState || ADM_VSS_STATE_TYPE_PRE_DOWNLOAD == eState) {
            log_i( "State machine verification satisfies, current state is %d, you can enter PRE_DOWNLOAD", eState);
                iRet = ADM_ECODE_SUCCESSED;
            } else {
                log_w( "State machine verification is not satisfied current state is %d, you can not enter PRE_DOWNLOAD", eState);
                iRet = ADM_ECODE_STATE_NOT_SATISFIED;
            }
    }

    return iRet;
}

/**
 * @brief 下载后状态的校验，校验是否满足进入下载完成状态
 * @details 可以进入到下载完成的状态，DOWNLOADING
 * @return D32S         0:successed, non-0: failed
 */
D32S adm_vss_stateVerifyWhenPostDownload(void)
{
    D32S iRet = ADM_ECODE_FAILED;
    ADM_VSS_STATE_TYPE_E eState = ADM_VSS_STATE_TYPE_INVALID;

    iRet = adm_vss_getStateMachine(&eState);
    if (ADM_ECODE_SUCCESSED != iRet) {
        log_e( "adm_vss_getStateMachine failed, reason of error code is %d", eState);
    } else {
        if (ADM_VSS_STATE_TYPE_DOWNLOADING == eState) {
            log_i( "State machine verification satisfies, current state is %d, you can enter POST_DOWNLOAD", eState);
                iRet = ADM_ECODE_SUCCESSED;
            } else {
                log_w( "State machine verification is not satisfied current state is %d, you can not enter POST_DOWNLOAD", eState);
                iRet = ADM_ECODE_STATE_NOT_SATISFIED;
            }
    }

    return iRet;
}

/**
 * @brief 预传输状态的校验，校验是否满足进入到预传输状态
 * @details 可以进入到预传输的状态，DOWNLOADING
 * @return D32S         0:successed, non-0: failed
 */
D32S adm_vss_stateVerifyWhenPreTransfer(void)
{
    D32S iRet = ADM_ECODE_FAILED;
    ADM_VSS_STATE_TYPE_E eState = ADM_VSS_STATE_TYPE_INVALID;

    iRet = adm_vss_getStateMachine(&eState);
    if (ADM_ECODE_SUCCESSED != iRet) {
        log_e( "adm_vss_getStateMachine failed, reason of error code is %d", eState);
    } else {
        if (ADM_VSS_STATE_TYPE_DOWNLOADING == eState) {
            log_i( "State machine verification satisfies, current state is %d, you can enter PRE_TRANSFER", eState);
                iRet = ADM_ECODE_SUCCESSED;
            } else {
                log_w( "State machine verification is not satisfied current state is %d, you can not enter PRE_TRANSFER", eState);
                iRet = ADM_ECODE_STATE_NOT_SATISFIED;
            }
    }

    return iRet;
}

/**
 * @brief 传输中状态的校验，校验是否满足进入传输中状态
 * @details 可以进入到传输中的状态, PRE_TRANSFER、TRANSFER_PAUSE
 * @return D32S         0:successed, non-0: failed
 */
D32S adm_vss_stateVerifyWhenTransfering(void)
{
    D32S iRet = ADM_ECODE_FAILED;
    ADM_VSS_STATE_TYPE_E eState = ADM_VSS_STATE_TYPE_INVALID;

    iRet = adm_vss_getStateMachine(&eState);
    if (ADM_ECODE_SUCCESSED != iRet) {
        log_e( "adm_vss_getStateMachine failed, reason of error code is %d", eState);
    } else {
        if (ADM_VSS_STATE_TYPE_PRE_TRANSFER == eState || ADM_VSS_STATE_TYPE_TRANSER_PAUSE == eState) {
            log_i( "State machine verification satisfies, current state is %d, you can enter TRANSFERING", eState);
                iRet = ADM_ECODE_SUCCESSED;
            } else {
                log_w( "State machine verification is not satisfied current state is %d, you can not enter TRANSFERING", eState);
                iRet = ADM_ECODE_STATE_NOT_SATISFIED;
            }
    }

    return iRet;
}

/**
 * @brief 预安装状态的校验，校验是否满足进入预安装状态
 * @details 可以进入到预安装的状态，POST_DOWNLOAD
 * @return D32S         0:successed, non-0: failed
 */
D32S adm_vss_stateVerifyWhenPreInstall(void)
{
    D32S iRet = ADM_ECODE_FAILED;
    ADM_VSS_STATE_TYPE_E eState = ADM_VSS_STATE_TYPE_INVALID;

    iRet = adm_vss_getStateMachine(&eState);
    if (ADM_ECODE_SUCCESSED != iRet) {
        log_e( "adm_vss_getStateMachine failed, reason of error code is %d", eState);
    } else {
        if (ADM_VSS_STATE_TYPE_POST_DOWNLOAD == eState) {
            log_i( "State machine verification satisfies, current state is %d, you can enter PRE_INSTALL", eState);
                iRet = ADM_ECODE_SUCCESSED;
            } else {
                log_w( "State machine verification is not satisfied current state is %d, you can not enter PRE_INSTALL", eState);
                iRet = ADM_ECODE_STATE_NOT_SATISFIED;
            }
    }

    return iRet;
}

/**
 * @brief 安装中状态的校验，校验是否满足进入安装中状态
 * @details 可以进入到安装中的状态，PRE_INSTALL、INSTALLING
 * @return D32S         0:successed, non-0: failed
 */
D32S adm_vss_stateVerifyWhenInstalling(void)
{
    D32S iRet = ADM_ECODE_FAILED;
    ADM_VSS_STATE_TYPE_E eState = ADM_VSS_STATE_TYPE_INVALID;

    iRet = adm_vss_getStateMachine(&eState);
    if (ADM_ECODE_SUCCESSED != iRet) {
        log_e( "adm_vss_getStateMachine failed, reason of error code is %d", eState);
    } else {
        if (ADM_VSS_STATE_TYPE_PRE_INSTALL == eState || ADM_VSS_STATE_TYPE_INSTALLING == eState) {
            log_i( "State machine verification satisfies, current state is %d, you can enter INSTALLING", eState);
                iRet = ADM_ECODE_SUCCESSED;
            } else {
                log_w( "State machine verification is not satisfied current state is %d, you can not enter INSTALLING", eState);
                iRet = ADM_ECODE_STATE_NOT_SATISFIED;
            }
    }

    return iRet;
}

/**
 * @brief 安装后状态的校验，校验是否满足进入安装完成状态
 * @details 可以进入到安装后的状态，INSTALLING
 * @return D32S         0:successed, non-0: failed
 */
D32S adm_vss_stateVerifyWhenPostInstall(void)
{
    // TODO:
    D32S iRet = ADM_ECODE_FAILED;
    ADM_VSS_STATE_TYPE_E eState = ADM_VSS_STATE_TYPE_INVALID;

    iRet = adm_vss_getStateMachine(&eState);
    if (ADM_ECODE_SUCCESSED != iRet) {
        log_e( "adm_vss_getStateMachine failed, reason of error code is %d", eState);
    } else {
        if (ADM_VSS_STATE_TYPE_PRE_INSTALL == eState || ADM_VSS_STATE_TYPE_INSTALLING == eState) {
            log_i( "State machine verification satisfies, current state is %d, you can enter INSTALLING", eState);
                iRet = ADM_ECODE_SUCCESSED;
            } else {
                log_w( "State machine verification is not satisfied current state is %d, you can not enter INSTALLING", eState);
                iRet = ADM_ECODE_STATE_NOT_SATISFIED;
            }
    }

    return iRet;
}

/**
 * @brief 紧急状态的校验，校验是否满足进入到紧急状态
 * @details 可以进入到Broken的状态，POST_INSTALL
 * @return D32S         0:successed, non-0: failed
 */
D32S adm_vss_vrifyStateWhenBroken(void)
{
    // TODO:
    D32S iRet = ADM_ECODE_FAILED;
    ADM_VSS_STATE_TYPE_E eState = ADM_VSS_STATE_TYPE_INVALID;

    iRet = adm_vss_getStateMachine(&eState);
    if (ADM_ECODE_SUCCESSED != iRet) {
        log_e( "adm_vss_getStateMachine failed, reason of error code is %d", eState);
    } else {
        if (ADM_VSS_STATE_TYPE_POST_INSTALL == eState) {
            log_i( "State machine verification satisfies, current state is %d, you can enter BROKEN", eState);
                iRet = ADM_ECODE_SUCCESSED;
            } else {
                log_w( "State machine verification is not satisfied current state is %d, you can not enter BROKEN", eState);
                iRet = ADM_ECODE_STATE_NOT_SATISFIED;
            }
    }

    return iRet;
}
