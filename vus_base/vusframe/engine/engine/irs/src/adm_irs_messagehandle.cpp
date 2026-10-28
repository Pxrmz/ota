#include "adm_irs_message.h"
#include "adm_irs_file.h"
#include "adm_irs_log.h"
#include "adm_memory.h"

#include <thread>
#include <memory>
#include <pthread.h>

namespace abup
{
namespace vus
{
namespace irs
{

class Event
{
public:
    Event()
        : _state(false)
        , bIsStart(true)
    {
        (void)pthread_mutex_init(&_mutex, NULL);
        (void)pthread_cond_init(&_cond, NULL);
    }

    ~Event()
    {
        bIsStart = false;

        (void)pthread_mutex_lock(&_mutex);
        (void)pthread_cond_broadcast(&_cond);
        (void)pthread_cond_destroy(&_cond);
        (void)pthread_mutex_unlock(&_mutex);
        (void)pthread_mutex_destroy(&_mutex);
    }

    void set()
    {
        (void)pthread_mutex_lock(&_mutex);
        _state = true;
        (void)pthread_cond_signal(&_cond);
        (void)pthread_mutex_unlock(&_mutex);
    }
    void wait()
    {

        if (false == bIsStart)
        {
            return;
        }

        (void)pthread_mutex_lock(&_mutex);
        log_i("state = %d", _state);
        while (false == _state) {
            (void)pthread_cond_wait(&_cond, &_mutex);
            log_i("receive notify");
        }

        _state = false;

        (void)pthread_mutex_unlock(&_mutex);
        return ;
    }

private:
    bool _state;
    pthread_mutex_t _mutex;
    pthread_cond_t _cond;
    bool bIsStart;
};

class IrsCondition
{
public:
    static IrsCondition& instance()
    {
        static IrsCondition app;
        return app;
    }

    void wait()
    {
        do {

            if (TRUE == adm_irs_hasVehicleStateMessage()) {
                isHaveVehicleState = TRUE;
                return;
            }
            if (TRUE == adm_irs_hasEcuStateMessage()) {
                isHaveEcuState = TRUE;
                return;
            }
            if (TRUE == adm_irs_hasProgressMessage()) {
                isHaveProgress = TRUE;
                return;
            }
            if (TRUE == adm_irs_hasEventMessage()) {
                isHaveEvent = TRUE;
                return;
            }
            if (TRUE == adm_irs_hasFile()) {
                isHaveFile = TRUE;
                return;
            }
            {
                _event.wait();
                return;
            }

        } while (1);
    }

    void set()
    {
        _event.set();
    }


    DBOOL isHaveVehicleState;
    DBOOL isHaveEcuState;
    DBOOL isHaveProgress;
    DBOOL isHaveEvent;
    DBOOL isHaveFile;

private:
    IrsCondition()
        : isHaveVehicleState(FALSE)
        , isHaveEcuState(FALSE)
        , isHaveProgress(FALSE)
        , isHaveEvent(FALSE)
        , isHaveFile(FALSE){

          };
    ~IrsCondition() = default;
    Event _event;
};

}  // namespace irs
}  // namespace vus
}  // namespace abup

extern "C" {

static DBOOL g_threadIsStart = FALSE;

void adm_irs_initReportMessageHandle(ADM_IRS_SESSION_T* session)
{
    if (FALSE == g_threadIsStart) {
        g_threadIsStart = TRUE;
        std::thread thread([session]() {
            D32S iRetVal = ADM_ECODE_SUCCESSED;

            while (NULL != session) {

                /*收到存储数据线程的通知或者等待到指定时间,查询KV中是否有数据,有数据结束等待*/
                abup::vus::irs::IrsCondition::instance().wait();

                do {
                    if (TRUE == abup::vus::irs::IrsCondition::instance().isHaveVehicleState) {
                        abup::vus::irs::IrsCondition::instance().isHaveVehicleState = FALSE;
                        log_i("report vehicle state");
                        iRetVal = adm_irs_reportVehicleState();
                        if (ADM_ECODE_SUCCESSED != iRetVal) {
                            log_e("adm_irs_reportVehicleState return error[%d]", iRetVal);
                            break;
                        }
                    }

                    if (TRUE == abup::vus::irs::IrsCondition::instance().isHaveEcuState) {
                        abup::vus::irs::IrsCondition::instance().isHaveEcuState = FALSE;
                        log_i("report ecu state");
                        iRetVal = adm_irs_reportEcuState();
                        if (ADM_ECODE_SUCCESSED != iRetVal) {
                            log_e("adm_irs_reportEcuState return error[%d]", iRetVal);
                            break;
                        }
                    }

                    if (TRUE == abup::vus::irs::IrsCondition::instance().isHaveProgress) {
                        abup::vus::irs::IrsCondition::instance().isHaveProgress = FALSE;
                        log_i("report progress");
                        iRetVal = adm_irs_reportProgress();
                        if (ADM_ECODE_SUCCESSED != iRetVal) {
                            log_e("adm_irs_reportProgress return error[%d]", iRetVal);
                            break;
                        }
                    }

                    if (TRUE == abup::vus::irs::IrsCondition::instance().isHaveEvent) {
                        abup::vus::irs::IrsCondition::instance().isHaveEvent = FALSE;
                        log_i("report event");
                        iRetVal = adm_irs_reportEvent();
                        if (ADM_ECODE_SUCCESSED != iRetVal) {
                            log_e("adm_irs_reportEvent return error[%d]", iRetVal);
                            break;
                        }
                    }
                    if (TRUE == abup::vus::irs::IrsCondition::instance().isHaveFile) {
                        abup::vus::irs::IrsCondition::instance().isHaveFile = FALSE;
                        log_i("report file");
                        iRetVal = adm_irs_reportFile();
                        if (ADM_ECODE_SUCCESSED != iRetVal) {
                            log_e("adm_irs_reportFile return error[%d]", iRetVal);
                            break;
                        }
                    }
                } while (0);
                if (ADM_ECODE_SUCCESSED != iRetVal ) {
                    // 非success，与产品沟通后，等待60s重试上报
                    std::this_thread::sleep_for(std::chrono::seconds(60));
                    iRetVal = ADM_ECODE_SUCCESSED;
                }
            }
            g_threadIsStart = FALSE;
            log_w("session is NULL, thread exit");
        });
        thread.detach();
    } else {
        log_e("report message thread has started");
    }
}

void adm_irs_saveReportCallBack(void* argv)
{
    ADM_IRS_UPLOAD_TYPE_E enUploadType;
    if (ADM_IRS_SAVE_SUCCESS == adm_irs_UploadCallBack(argv, &enUploadType)) {
        log_i("have [%d] upload data into KV, notify_one", enUploadType);

        if (ADM_IRS_UPLOAD_VEHICLE_STATE == enUploadType) {
            abup::vus::irs::IrsCondition::instance().isHaveVehicleState = TRUE;
        } else if (ADM_IRS_UPLOAD_ECU_STATE == enUploadType) {
            abup::vus::irs::IrsCondition::instance().isHaveEcuState = TRUE;
        } else if (ADM_IRS_UPLOAD_PROGRESS == enUploadType) {
            abup::vus::irs::IrsCondition::instance().isHaveProgress = TRUE;
        } else if (ADM_IRS_UPLOAD_EVENT == enUploadType) {
            abup::vus::irs::IrsCondition::instance().isHaveEvent = TRUE;
        } else if (ADM_IRS_UPLOAD_FILE == enUploadType) {
            abup::vus::irs::IrsCondition::instance().isHaveFile = TRUE;
        }
        abup::vus::irs::IrsCondition::instance().set();
    }
    return;
}
}