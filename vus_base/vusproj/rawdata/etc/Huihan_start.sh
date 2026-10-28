#!/bin/sh

ABUP_PATH=/custapps/lionota
ABUP_BIN_PATH=${ABUP_PATH}/bin
ABUP_ETC_PATH=${ABUP_PATH}/etc
ABUP_LIB_PATH=${ABUP_PATH}/lib
SUBDA_CFG_PATH=${ABUP_ETC_PATH}/liteconfig.ini
SUBDA_PATH=${ABUP_BIN_PATH}/admsysiApp

# 设置动态链接库的搜索路径
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$ABUP_LIB_PATH:/custapps/lib

# 定义检查进程并重启的函数
check_and_restart_process() {
    # 检查进程是否存在
    if ! pgrep -f $SUBDA_PATH > /dev/null; then
        # 如果进程不存在，则记录日志并重启进程
        echo "[$(date)]$SUBDA_PATH is not running, restarting..." > /home/userspace/progress.log
        cd $ABUP_BIN_PATH
        $SUBDA_PATH -c $SUBDA_CFG_PATH >/dev/null 2>&1 &
    fi
}

# 无限循环，定期检查进程
while [ 1 ]; do
    check_and_restart_process
    sleep 3  # 每3秒检查一次
done
