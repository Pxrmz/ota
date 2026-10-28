#!/bin/bash
###
### my-script — does one thing well
###
### Usage:
###   my-script <input_1> <input_2>
###
### Options:
###   <input_1> 
###     @描述: 代表编译环境
###     @取值: x86/x86_64/ag35/ag566/android14/qnx7/qnx7-1/arma7
###     @说明: 后续若有新增，可提交申请增加选项
### 
###   <input_2>
###     @描述: 代表此次编译版本号
###     @取值: v1.0.1 ...
###     @说明: 版本号的发布，需要跟Roadmap相关联，临时版本，版本号可为随意字符串
###   -h        Show this message.

function help() {
    awk -F'### ' '/^###/ { print $2 }' "$0"
}

if [[ $# == 0 ]] || [[ "$1" == "-h" ]]; then
	help
	exit 1
fi

# 编译完成输出的包名
if [ $# -ne 2 ]; then
    echo "$0 add package name "
    echo "$0 add platform version x86 imx8"
    echo "$0 if compile arm version,please source env"
    exit 1
fi

PROJECT_NAME=arpc
TOPDIR=$(pwd)
SRC_CODE_DIR=$TOPDIR/../arpc/

# 创建目录和编译
rm -rf $SRC_CODE_DIR/build 
mkdir $SRC_CODE_DIR/build
cd $SRC_CODE_DIR/build

echo "platform specified $1"

if [ $1 == "x86" ]; then
    cmake -DENV_PLATFORM=$1 -DSDK_VERSION=$2 .. 
elif [ $1 == "x86_64" ]; then
    cmake -DENV_PLATFORM=$1 -DSDK_VERSION=$2 .. 
elif [ $1 == "ag35" ]; then
    cmake -DCMAKE_TOOLCHAIN_FILE=../cmake/toolchain-agxx.cmake -DENV_PLATFORM=$1 -DSDK_VERSION=$2 ..  
elif [ $1 == "arma7" ]; then
    cmake -DCMAKE_TOOLCHAIN_FILE=../cmake/toolchain-arma7.cmake -DENV_PLATFORM=$1 -DSDK_VERSION=$2 ..  
elif [ $1 == "android11" ]; then
    cmake -DCMAKE_TOOLCHAIN_FILE=../cmake/toolchain-android.cmake -DENV_PLATFORM=$1 -DSDK_VERSION=$2 .. 
elif [ $1 == "qnx7" ]; then
    cmake -DCMAKE_TOOLCHAIN_FILE=../cmake/toolchain-qnx7.cmake -DENV_PLATFORM=$1 -DSDK_VERSION=$2 .. 
elif [ $1 == "qnx7-1" ]; then
    cmake -DCMAKE_TOOLCHAIN_FILE=../cmake/toolchain-qnx7-1.cmake -DENV_PLATFORM=$1 -DSDK_VERSION=$2 ..
else
    help
    exit 1
fi

make

if [ "$?" == "0" ]; then
    echo $?
    echo "build arpc successful"    
#    exit 1
else
    echo $?
    echo "build arpc error"    
    exit 1
fi

#回到顶层目录
cd ..

#创建文件目录
#ROM_FOLDER=arpc
#OUTPUT=$SRC_CODE_DIR/build/output/${ROM_FOLDER}
OUTPUT=$SRC_CODE_DIR/build/output
OUTPUT_INCLUDE=$OUTPUT/inc
OUTPUT_ETC=$OUTPUT/etc
OUTPUT_BIN=$OUTPUT/bin
OUTPUT_LIB=$OUTPUT/lib


# 发布打包 Begin
mkdir -p $OUTPUT
mkdir -p $OUTPUT_INCLUDE
mkdir -p $OUTPUT_ETC
mkdir -p $OUTPUT_BIN
mkdir -p $OUTPUT_LIB

# copy inc
cp $SRC_CODE_DIR/arpc/inc/public/* $OUTPUT_INCLUDE

# copy lib
cp $SRC_CODE_DIR/build/arpc/libarpc.so $OUTPUT_LIB
cp $SRC_CODE_DIR/build/arpc/libarpc-static.a $OUTPUT_LIB

# copy bin
cp $SRC_CODE_DIR/build/arpc/sample/dim/dim $OUTPUT_BIN
cp $SRC_CODE_DIR/build/arpc/sample/sysi/sysi1 $OUTPUT_BIN
cp $SRC_CODE_DIR/build/arpc/sample/sysi/sysi2 $OUTPUT_BIN

# copy etc
cp $SRC_CODE_DIR/arpc/sample/dim/otaconfig.ini $OUTPUT_ETC/dim_config.ini
cp $SRC_CODE_DIR/arpc/sample/sysi/otaconfig.ini $OUTPUT_ETC/sysi_config.ini
cp $SRC_CODE_DIR/arpc/sample/cert/* $OUTPUT_ETC

# compress output
TIMESTAMP=$(date +'%Y%m%d')
GIT_COMMITID=$(git rev-parse --short HEAD)
cd $OUTPUT
tar -czvf ${PROJECT_NAME}_$1_$2_${TIMESTAMP}_${GIT_COMMITID}.tar.gz *
cd ../..

echo "export file path = $OUTPUT/${PROJECT_NAME}_$1_$2_${TIMESTAMP}_${GIT_COMMITID}.tar.gz"

