#!/bin/sh
projectId=$(. ${SVP_ETC_PATH}/svp.project.id.sh)

APP_NAME=admsysiApp

if [[ "$projectId" == "T1G-13-2" || "$projectId" == "T1G-15-6" ]]; then
    LION_PATH=/emmc/svp/tpa/ota_lion/default
elif [[ "$projectId" == "M1EFL1" || "$projectId" == "M1E" ]]; then
    LION_PATH=/emmc/svp/tpa/ota_lion/m1e
else
    LION_PATH=/emmc/svp/tpa/ota_lion/t19c
fi

LION_BIN_PATH=${LION_PATH}/bin
LION_ETC_PATH=${LION_PATH}/etc
LION_LIB_PATH=${LION_PATH}/lib
SUBMASTER_CFG_PATH=${LION_ETC_PATH}/liteconfig.ini
SUBMASTER_PATH=${LION_BIN_PATH}/admsysiApp

# 设置动态链接库的搜索路径
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$LION_LIB_PATH

# 定义检查进程并重启的函数
check_and_restart_process() {
    # 检查进程是否存在
    if ! pidin | grep "$APP_NAME" > /dev/null; then
        # 如果进程不存在，则记录日志并重启进程
        echo "[$(date)]$SUBMASTER_PATH is not running, restarting..." > /storage/fota/progress.log
        $SUBMASTER_PATH -c $SUBMASTER_CFG_PATH >/dev/null 2>&1 &
    fi
}

# 无限循环，定期检查进程
while [ 1 ]; do
    check_and_restart_process
    sleep 3  # 每3秒检查一次
done
