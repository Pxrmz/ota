#!/bin/bash

script_dir=$(cd $(dirname ${BASH_SOURCE[0]}) && pwd)
output_dir=$script_dir/../build/output

pkg_sdk_dir=$output_dir/package
ucm_sdk_dir=$pkg_sdk_dir/ucm
ucm_sdk_lib_dir=$ucm_sdk_dir/lib
ucm_sdk_etc_dir=$ucm_sdk_dir/etc
ucm_sdk_inc_dir=$ucm_sdk_dir/include

have_package=0
echo "[preDeploy]: Prepare take out sdk."

# 打包 ucm
if [ -d ${output_dir} ]; then
    echo "[preDeploy]: Take out ucm sdk ready."
    rm $ucm_sdk_dir -rf
    mkdir -p $ucm_sdk_dir
    mkdir -p $ucm_sdk_lib_dir
    #mkdir -p $ucm_sdk_etc_dir
    mkdir -p $ucm_sdk_inc_dir

    cp ${output_dir}/admvus/lib/* $ucm_sdk_lib_dir
    #cp -r ${output_dir}/admvus/etc/* $ucm_sdk_etc_dir
    cp $script_dir/../application/ucmJni/inc/jni.h $ucm_sdk_inc_dir
    cp $script_dir/../application/ucmJni/ucm/inc/com_abupdate_vus_ucm_service.h $ucm_sdk_inc_dir

    echo "[preDeploy]: Take out ucm sdk OK."

    have_package=1
fi


if [ 0 != "$have_package" ]; then
    # 打包启动脚本
    cd $output_dir 
    tar -czvf ucmApk.tar.gz $pkg_sdk_dir
    echo "[preDeploy]: Package success.sdk path ---> $pkg_sdk_dir"
else
    echo "[preDeploy]: Nothing package take out."
fi
