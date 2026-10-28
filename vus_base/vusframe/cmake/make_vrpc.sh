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
###     @取值: x86/x86_64/9x07/imx8/ag35/android10/android11/qnx7/alios
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

PROJECT_NAME=vrpc
TOPDIR=$(pwd)
SRC_CODE_DIR=$TOPDIR/../vrpc/

# 创建目录和编译
rm -rf $SRC_CODE_DIR/build 
mkdir $SRC_CODE_DIR/build
cd $SRC_CODE_DIR/build

echo "platform specified $1"

if [ $1 == "x86" ]; then
    cmake -DENV_PLATFORM=$1 -DSDK_VERSION=$2 .. 
elif [ $1 == "x86_64" ]; then
    cmake -DENV_PLATFORM=$1 -DSDK_VERSION=$2 .. 
elif [ $1 == "imx8" ]; then
    cmake -DCMAKE_TOOLCHAIN_FILE=../cmake/toolchain-imx8-a35.cmake -DENV_PLATFORM=$1 -DSDK_VERSION=$2 ..
elif [ $1 == "ag35" ]; then
    cmake -DCMAKE_TOOLCHAIN_FILE=../cmake/toolchain-ag35.cmake -DENV_PLATFORM=$1 -DSDK_VERSION=$2 ..
elif [ $1 == "android9" ]; then
    cmake -DCMAKE_TOOLCHAIN_FILE=../cmake/toolchain-android.cmake -DENV_PLATFORM=$1 -DSDK_VERSION=$2 ..
elif [ $1 == "android10" ]; then
    cmake -DCMAKE_TOOLCHAIN_FILE=../cmake/toolchain-android.cmake -DENV_PLATFORM=$1 -DSDK_VERSION=$2 ..
elif [ $1 == "android11" ]; then
    cmake -DCMAKE_TOOLCHAIN_FILE=../cmake/toolchain-android.cmake -DENV_PLATFORM=$1 -DSDK_VERSION=$2 ..
elif [ $1 == "qnx7" ]; then
    cmake -DCMAKE_TOOLCHAIN_FILE=../cmake/toolchain-qnx7.cmake -DENV_PLATFORM=$1 -DSDK_VERSION=$2 ..
elif [ $1 == "alios" ]; then
    cmake -DCMAKE_TOOLCHAIN_FILE=../cmake/toolchain-alios.cmake -DENV_PLATFORM=$1 -DSDK_VERSION=$2 ..
elif [ $1 == "9x07" ]; then
    cmake -DCMAKE_TOOLCHAIN_FILE=../cmake/toolchain-9x07.cmake -DENV_PLATFORM=$1 -DSDK_VERSION=$2 ..
elif [ $1 == "A58" ]; then
    cmake -DCMAKE_TOOLCHAIN_FILE=../cmake/toolchain-A58.cmake -DENV_PLATFORM=$1 -DSDK_VERSION=$2 ..
elif [ $1 == "ag55x" ]; then
    cmake -DCMAKE_TOOLCHAIN_FILE=../cmake/toolchain-agxx.cmake -DENV_PLATFORM=$1 -DSDK_VERSION=$2 ..
else
    
    exit 1
fi

make

if [ "$?" == "0" ]; then
    echo $?
    echo "build vrpc successful"    
#    exit 1
else
    echo $?
    echo "build vrpc error"    
    exit 1
fi


#回到顶层目录
cd ..

#创建文件目录
OUTPUT_DIR=$SRC_CODE_DIR/build/output/
OUTPUT_INC_DIR=$SRC_CODE_DIR/build/output/sdk/include/tbox
OUTPUT_LIBS_DIR=$SRC_CODE_DIR/build/output/libs

mkdir -p $OUTPUT_DIR
mkdir -p $OUTPUT_INC_DIR
mkdir -p $OUTPUT_LIBS_DIR

#拷贝文件
if [ $1 == "x86" ]; then
    strip $SRC_CODE_DIR/build/libmytech_absomeip.so
else
    ${STRIP} $SRC_CODE_DIR/build/libmytech_absomeip.so
fi

cp -r $SRC_CODE_DIR/vsomeip/interface/* $OUTPUT_DIR
cp -r $SRC_CODE_DIR/sdk/include/tbox/* $OUTPUT_INC_DIR
cp $SRC_CODE_DIR/build/libmytech_absomeip* $OUTPUT_LIBS_DIR
cp $SRC_CODE_DIR/build/vsomeip/libvsomeip* $OUTPUT_LIBS_DIR
cp $SRC_CODE_DIR/build/vsomeip/implementation/helper/jsoncpp/src/lib_json/libjsoncpp.a $OUTPUT_LIBS_DIR
cp -r $SRC_CODE_DIR/samples $OUTPUT_DIR

#打包发布
TIMESTAMP=$(date +'%Y%m%d')
GIT_COMMITID=$(git rev-parse --short HEAD)
cd $SRC_CODE_DIR/build/output/
tar -czvf ${PROJECT_NAME}_$1_$2_${TIMESTAMP}_${GIT_COMMITID}.tar.gz *
cd ../..

echo "export file path = $SRC_CODE_DIR/build/output/${PROJECT_NAME}_$1_$2_${TIMESTAMP}_${GIT_COMMITID}.tar.gz"

