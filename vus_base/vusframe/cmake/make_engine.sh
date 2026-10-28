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
###     @取值: x86/x86_64/ag35/ag566/android14/qnx7/qnx7-1
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

PROJECT_NAME=engine
TOPDIR=$(pwd)
SRC_CODE_DIR=$TOPDIR/../engine/

# 编译完成输出的包名
if [ $# -ne 2 ]; then
    echo "$0 add package name "
    echo "$0 add platform version x86 imx8"
    echo "$0 if compile arm version,please source env"
    exit 1
fi

# 创建目录和编译
rm -rf $SRC_CODE_DIR/build 
mkdir $SRC_CODE_DIR/build
cd $SRC_CODE_DIR/build

echo "platform specified $1"

# 生成OEM供应商需要提供的接口用户手册（Doxygen格式）
function make_docs {
    make  core_service_API_user_manual 2>&1 | tee build_doxygen.log
}

if [ $1 == "x86" ]; then
    cmake -DPLATFORM_NAME=Linux -DENV_PLATFORM=$1 -DSDK_VERSION=$2 .. 
elif [ $1 == "x86_64" ]; then
    cmake -DPLATFORM_NAME=Linux -DENV_PLATFORM=$1 -DSDK_VERSION=$2 .. 
elif [ $1 == "ag35" ]; then
    cmake -DPLATFORM_NAME=Linux -DENV_PLATFORM=$1 -DSDK_VERSION=$2 ..
elif [ $1 == "arma7" ]; then
    cmake -DCMAKE_TOOLCHAIN_FILE=../cmake/toolchain-arma7.cmake -DPLATFORM_NAME=Linux -DENV_PLATFORM=$1 -DSDK_VERSION=$2 ..
elif [ $1 == "android11" ]; then
   cmake -DPLATFORM_NAME=Android -DENV_PLATFORM=$1 -DSDK_VERSION=$2 ..
elif [ $1 == "qnx7" ]; then
   cmake -DPLATFORM_NAME=QNX7 -DENV_PLATFORM=$1 -DSDK_VERSION=$2 ..
elif [ $1 == "qnx7-1" ]; then
    cmake -DPLATFORM_NAME=QNX7 -DENV_PLATFORM=$1 -DSDK_VERSION=$2 ..
else
    echo "platform specified error"
    exit 1
fi

make_docs
make
make install
# make package

if [ "$?" == "0" ]; then
    echo $?
    echo "build vuscore successful"    
#    exit 1
else
    echo $?
    echo "build vuscore error"    
    exit 1
fi

#回到顶层目录
cd ..

#提供给vusapp使用的vuscore库

#创建DOCS目录
OUTPUT_DOCS_DIR=$SRC_CODE_DIR/build/output/Docs
mkdir -p $OUTPUT_DOCS_DIR

#移动用户手册到Docs目录并为用户手册目录添加指导链接文件
cp -r $SRC_CODE_DIR/build/engine/core_service_API_user_manual/ $OUTPUT_DOCS_DIR

#创建单元测试的目录
VUSCORE_GTEST=$SRC_CODE_DIR/build/output/gtest
mkdir -p $VUSCORE_GTEST
cp $SRC_CODE_DIR/build/test/adm_* $VUSCORE_GTEST
cp $SRC_CODE_DIR/build/test/check_resp.txt $VUSCORE_GTEST
cp -r $SRC_CODE_DIR/build/test/data $VUSCORE_GTEST

# compress output
TIMESTAMP=$(date +'%Y%m%d')
GIT_COMMITID=$(git rev-parse --short HEAD)
cd $SRC_CODE_DIR/build/output/engine/
tar -czvf ${PROJECT_NAME}_$1_$2_${TIMESTAMP}_${GIT_COMMITID}.tar.gz *
cd ../..

echo "export file path = $SRC_CODE_DIR/build/output/engine/${PROJECT_NAME}_$1_$2_${TIMESTAMP}_${GIT_COMMITID}.tar.gz"
