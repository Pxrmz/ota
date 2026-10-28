#include <functional>
#include <limits>
#include <memory>
#include <string>
#include <mutex>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <chrono>

#include "ab_ewebsrv_api.h"
#include <stdlib.h>
#include <stdarg.h>

// customized log header
#define LOG_PREFIX "[EWEB]"
#define LOG_DEFAULT_FILENAME "./ewebsrvlog.txt"
#define LOGOUT_FILE      1
#define LOGOUT_STDIO     1
#define LOGOUT_APPEND    0

namespace abewebsrv {

class Logout {
    std::string log_filename;
    bool bStdio_;
    bool bFilesave_;
    int counter_;

public:
    AB_EWEBSRV_LOG_LEVEL local_level_;
    std::mutex lock_;

    Logout(){
        Init();
        OpenLog(AB_EWEBSRV_LOG_DEBUG, LOG_DEFAULT_FILENAME, LOGOUT_FILE, LOGOUT_STDIO, LOGOUT_APPEND);
    }
    ~Logout() {
        CloseLog();
    }

    void Init() {
        local_level_ =  AB_EWEBSRV_LOG_UNINIT;
        bStdio_ = false;
        bFilesave_ = false;
        counter_ = 0;
    }

    void OpenLog(AB_EWEBSRV_LOG_LEVEL curLevel,
                 const char* logfilename,
                 bool stdio,
                 bool filesave,
                 bool append) {
        local_level_ = curLevel;
        bStdio_ = stdio;
        bFilesave_ = filesave;
        counter_ = 0;

        if (local_level_ == AB_EWEBSRV_LOG_NONE)
            return;

        if (logfilename)
            log_filename = logfilename;
        if (log_filename.length() == 0){
            log_filename = LOG_DEFAULT_FILENAME;
        }
        if (bFilesave_) {
            std::ofstream log_outstream(log_filename, append ? std::ios::app : std::ios::binary);
            if (log_outstream.is_open()) {
                log_outstream << "\r\n";
                log_outstream.close();
            }
        }
    }

    void CloseLog() {
        local_level_ = AB_EWEBSRV_LOG_UNINIT;
    }

    void LogOut(AB_EWEBSRV_LOG_LEVEL loglevel, const char* _buf) {
        char m[4] = {'D', 'I', 'W', 'E'};
        counter_++;

        std::chrono::time_point<std::chrono::system_clock> chrono_time;
        chrono_time = std::chrono::system_clock::now();
        std::time_t end_time = std::chrono::system_clock::to_time_t(chrono_time);
        struct tm now_time;
        struct tm* p = &now_time;
        localtime_r(&end_time, &now_time);
        char buf[64];
        snprintf(buf, 64, "%s[%c]%04d[%d/%d/%d-%d:%d:%d]",
                 LOG_PREFIX, m[loglevel-1],
                counter_, p->tm_year + 1900, p->tm_mon + 1, p->tm_mday,
                p->tm_hour, p->tm_min, p->tm_sec);
        std::string logss = buf;
        logss = logss + _buf + "\r\n";
        if (bFilesave_){
            std::ofstream log_outstream(log_filename, std::ios::app);
            if (log_outstream.is_open()) {
                log_outstream << logss;
                log_outstream.close();
            }
        }
        if (bStdio_){
            printf("%s", logss.c_str());
        }
    }
};
}

static abewebsrv::Logout s_log;

void abEWebSrv_LogOut(AB_EWEBSRV_LOG_LEVEL loglevel, const char *fmt, ...) {

    std::lock_guard<std::mutex> its_lock(s_log.lock_);
    if (loglevel != AB_EWEBSRV_LOG_ERROR && (loglevel < s_log.local_level_ || s_log.local_level_ == AB_EWEBSRV_LOG_NONE))
        return;

    if (loglevel <= AB_EWEBSRV_LOG_UNINIT || loglevel >= AB_EWEBSRV_LOG_NONE || s_log.local_level_ == AB_EWEBSRV_LOG_UNINIT) {
        printf("LogOut ignore!\n");
        return;
    }

    va_list ap;
    char buf[1024];

    va_start(ap, fmt);
    vsnprintf(buf, 1024, fmt, ap);
    buf[1023] = 0;
    va_end(ap);
    s_log.LogOut(loglevel, buf);
}

void abEWebSrv_SetLog(AB_EWEBSRV_LOG_LEVEL curLevel, const char* logfilename,
                      bool stdio,
                      bool filesave,
                      bool append) {
    std::lock_guard<std::mutex> its_lock(s_log.lock_);
    s_log.OpenLog(curLevel, logfilename, stdio, filesave, append);
}


