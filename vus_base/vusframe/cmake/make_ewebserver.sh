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

if [ $# -ne 2 ]; then
    echo "$0 version platform "
    exit 1
fi

# if [ $2  == "android" ]; then
# 	echo "goto jni to use NDK !"
# 	exit 2
# fi

PROJECT_NAME=ewebserver
TOPDIR=$(pwd)
SRC_CODE_DIR=$TOPDIR/../ewebserver/
CMAKE_DIR=$SRC_CODE_DIR/civetweb-1.15

rm -rf $SRC_CODE_DIR/build 
mkdir $SRC_CODE_DIR/build

platform=$1
cd $SRC_CODE_DIR/build

echo "platform:${platform}"

# if [ $2 != "linux" -a  $2 != "arm" ]; then
# 	echo "use win32 platform!"
# 	platform="win32"
# fi

if [ $1 == "x86" ]; then
    cmake ${CMAKE_DIR} -DCMAKE_BUILD_TYPE=Release -DPLATFORM_NAME=$1 -DSDK_VERSION=$2 .. 
elif [ $1 == "x86_64" ]; then
    cmake ${CMAKE_DIR} -DCMAKE_BUILD_TYPE=Release -DPLATFORM_NAME=$1 -DSDK_VERSION=$2 .. 
elif [ $1 == "ag35" ]; then
    cmake ${CMAKE_DIR} -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=${SRC_CODE_DIR}/../cmake/toolchain-agxx.cmake -DPLATFORM_NAME=$1 -DSDK_VERSION=$2 ..   
elif [ $1 == "arma7" ]; then
    cmake ${CMAKE_DIR} -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=${SRC_CODE_DIR}/../cmake/toolchain-arma7.cmake -DPLATFORM_NAME=$1 -DSDK_VERSION=$2 ..   
elif [ $1 == "android11" ]; then
    cmake ${CMAKE_DIR} -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=${SRC_CODE_DIR}/../cmake/toolchain-android.cmake -DPLATFORM_NAME=$1 -DSDK_VERSION=$2 .. 
elif [ $1 == "qnx7" ]; then
    cmake ${CMAKE_DIR} -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=${SRC_CODE_DIR}/../cmake/toolchain-qnx7.cmake -DPLATFORM_NAME=$1 -DSDK_VERSION=$2 .. 
elif [ $1 == "qnx7-1" ]; then
    cmake ${CMAKE_DIR} -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=${SRC_CODE_DIR}/../cmake/toolchain-qnx7-1.cmake -DPLATFORM_NAME=$1 -DSDK_VERSION=$2 ..   
else
    help
    exit 1
fi

make

if [ "$?" == "0" ]; then
    echo $?
    echo "build ewebserver successful"    
#    exit 1
else
    echo $?
    echo "build ewebserver error"    
    exit 1
fi

git=$(git rev-parse --short HEAD)
version=ewebsrv-$2-${platform}-Release
if [ git  != "" ]; then
	version=${version}-${git}
fi
echo "version:${version}"
cd ..

# # generate static lib
# ./makeallstatic.sh build $platform
# cp ./build/ewebsrv_static.a ${TOPDIR}/sdk/lib/${platform}/
# cp ./build/ewebsrvdemo ${TOPDIR}/sdk/lib/${platform}/

OUTPUT=$SRC_CODE_DIR/build/output
OUTPUT_INC=$SRC_CODE_DIR/build/output/include
OUTPUT_LIBS=$SRC_CODE_DIR/build/output/libs
OUTPUT_CONF=$SRC_CODE_DIR/build/output/etc
OUTPUT_BIN=$SRC_CODE_DIR/build/output/bin
OUTPUT_DEMO=$SRC_CODE_DIR/build/output/sample

mkdir -p $OUTPUT
mkdir -p $OUTPUT_INC
mkdir -p $OUTPUT_LIBS
mkdir -p $OUTPUT_CONF
mkdir -p $OUTPUT_BIN
mkdir -p $OUTPUT_DEMO

cp $SRC_CODE_DIR/build/ewebsrvdemo $OUTPUT_BIN
cp $SRC_CODE_DIR/civetweb/src/ewebsrvdemo.c $OUTPUT_DEMO
cp $SRC_CODE_DIR/user-guided.txt $OUTPUT
cp $SRC_CODE_DIR/build/libewebsrvapi* $OUTPUT_LIBS
cp $SRC_CODE_DIR/sdk/include/ab_ewebsrv_api.h $OUTPUT_INC
cp $SRC_CODE_DIR/sdk/server.pem $OUTPUT_CONF
cp $SRC_CODE_DIR/sdk/config.txt $OUTPUT_CONF

# compress output
TIMESTAMP=$(date +'%Y%m%d')
GIT_COMMITID=$(git rev-parse --short HEAD)
cd $OUTPUT
tar -czvf ${PROJECT_NAME}_$1_$2_${TIMESTAMP}_${GIT_COMMITID}.tar.gz *
cd ../../
echo "export file path $OUTPUT/${PROJECT_NAME}_$1_$2_${TIMESTAMP}_${GIT_COMMITID}.tar.gz"

if [ -z $STRIP ]; then
	stripcmd=strip
else
    stripcmd=$STRIP
fi
#echo "strip:$stripcmd"
#$stripcmd sample

#tar czf ../$version.tar.gz *
# echo "$SRC_CODE_DIR/sdk/lib/${platform} done"
# ls -l $SRC_CODE_DIR/sdk/lib/${platform}/
# cd ..

