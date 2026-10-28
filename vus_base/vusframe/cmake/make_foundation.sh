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

PROJECT_NAME=foundation
TOPDIR=$(pwd)
SRC_CODE_DIR=$TOPDIR/../foundation/

build_version_h()
{

  
  if [ $1 == "x86" ]; then
    tmplink_tool=GCC7.5
    
elif [ $1 == "ag35" ]; then
    tmplink_tool=toolchain-agxx.cmake 

elif [ $1 == "arma7" ]; then
    tmplink_tool=toolchain-arma7.cmake
    
elif [ $1 == "android11" ]; then
   tmplink_tool=toolchain-android.cmake
   
elif [ $1 == "qnx7" ]; then
   tmplink_tool=toolchain-qnx7.cmake

elif [ $1 == "qnx7-1" ]; then
   tmplink_tool=toolchain-qnx7-1.cmake
   
else
    tmplink_tool=platform_specified_error

fi



  date2=$(date +%Y-%m-%d)'-'$(date +%H:%M:%S)
#  echo 'date2:'$date2
  
  git_version=$(git rev-parse HEAD)
#  targetZipPath=$5


  h_content="  #ifndef _BUILD_VERSION_H_\n
#define _BUILD_VERSION_H_\n
\n
\n
   #define V_BUILD_TIME \"$date2\"\n
   #define V_GIT_INFO \"$git_version\"\n
   #define V_BUILD_TOOLLINK \"$tmplink_tool\"\n
   #define V_PROJECT \"foundation\"\n
//   char *version_string = '{\"project_name\":\"foundation\",\"build_time\":\""$date2"\",\"git_id\":\""$git_version"\",\"tool_link\":\""$tmplink_tool"\"}';\n
\n
\n
#endif // _BUILD_VERSION_H_\n
"
  if [ -f ./libs/src/version/"build_version.h" ]; then
       rm -rf $SRC_CODE_DIR/libs/src/version/"build_version.h"
  fi

#  mkdir -p ./include

  echo -e $h_content >> $SRC_CODE_DIR/libs/src/version/"build_version.h"

  return 0

}


# 编译完成输出的包名
if [ $# -ne 3 ]; then
    echo "$0 add package name "
    echo "$0 add platform version x86 imx8"
    echo "$0 if compile arm version,please source env"
    exit 1
fi
   
   build_version_h $1
#   exit 0
# 创建目录和编译
rm -rf $SRC_CODE_DIR/build 
mkdir $SRC_CODE_DIR/build
cd $SRC_CODE_DIR/build

echo "platform specified $1"

if [ $1 == "x86" ]; then
    cmake -DENV_PLATFORM=$1 -DSDK_VERSION=$2 -DTEST_APK_IF_SUPPORT=$3 ..
elif [ $1 == "x86_64" ]; then
    cmake -DENV_PLATFORM=$1 -DSDK_VERSION=$2 -DTEST_APK_IF_SUPPORT=$3 ..
elif [ $1 == "ag35" ]; then
    cmake -DCMAKE_TOOLCHAIN_FILE=$TOPDIR/toolchain-agxx.cmake -DENV_PLATFORM=$1 -DSDK_VERSION=$2 -DTEST_APK_IF_SUPPORT=$3 ..
elif [ $1 == "arma7" ]; then
    cmake -DCMAKE_TOOLCHAIN_FILE=$TOPDIR/toolchain-arma7.cmake -DENV_PLATFORM=$1 -DSDK_VERSION=$2 -DTEST_APK_IF_SUPPORT=$3 ..
elif [ $1 == "android11" ]; then
    cmake -DCMAKE_TOOLCHAIN_FILE=$TOPDIR/toolchain-android.cmake -DENV_PLATFORM=$1 -DSDK_VERSION=$2 -DTEST_APK_IF_SUPPORT=$3 ..
elif [ $1 == "qnx7" ]; then
    cmake -DCMAKE_TOOLCHAIN_FILE=$TOPDIR/toolchain-qnx7.cmake -DENV_PLATFORM=$1 -DSDK_VERSION=$2 -DTEST_APK_IF_SUPPORT=$3 ..
elif [ $1 == "qnx7-1" ]; then
    cmake -DCMAKE_TOOLCHAIN_FILE=$TOPDIR/toolchain-qnx7-1.cmake -DENV_PLATFORM=$1 -DSDK_VERSION=$2 -DTEST_APK_IF_SUPPORT=$3 ..
else
    help
    exit 1
fi

make

if [ "$?" == "0" ]; then
    echo $?
    echo "build foundation successful"    
#    exit 1
else
    echo $?
    echo "build foundation error"    
    exit 1
fi


#回到顶层目录
cd ..

#创建文件目录
OUTPUT_DIR=$SRC_CODE_DIR/build/output/

PACAKGE=$OUTPUT_DIR/packages
PACAKGE_LIBS=$PACAKGE/libs
PACAKGE_SUB=$OUTPUT_DIR/packages/lib
PACAKGE_SUB_BASE64=$PACAKGE_SUB/base64
PACAKGE_SUB_CRYPT=$PACAKGE_SUB/crypto
PACAKGE_SUB_CRYPT_ASYM=$PACAKGE_SUB/crypto/asymCrypto
PACAKGE_SUB_CRYPT_SYM=$PACAKGE_SUB/crypto/symCrypto
PACAKGE_SUB_CRYPT_BASE64=$PACAKGE_SUB/crypto/base64
PACAKGE_SUB_CRYPT_PUB=$PACAKGE_SUB/crypto/public
PACAKGE_SUB_HMAC=$PACAKGE_SUB/hmac
PLATFORM_OS=$OUTPUT_DIR/platform/os
PLATFORM_LIBSINC=$OUTPUT_DIR/platform/libs/inc

mkdir -p $PACAKGE
mkdir -p $PACAKGE_LIBS
mkdir -p $PACAKGE_SUB
mkdir -p $PACAKGE_SUB_BASE64
mkdir -p $PACAKGE_SUB_CRYPT
mkdir -p $PACAKGE_SUB_CRYPT_ASYM
mkdir -p $PACAKGE_SUB_CRYPT_SYM
mkdir -p $PACAKGE_SUB_CRYPT_BASE64
mkdir -p $PACAKGE_SUB_CRYPT_PUB
mkdir -p $PACAKGE_SUB_HMAC
mkdir -p $PLATFORM_OS
mkdir -p $PLATFORM_LIBSINC

#拷贝文件
cp -r $SRC_CODE_DIR/build/deploy/include/* $PACAKGE
cp -f $SRC_CODE_DIR/libs/src/lib/*.h $PACAKGE_SUB
cp -f $SRC_CODE_DIR/libs/src/lib/base64/*.h $PACAKGE_SUB_BASE64
cp -f $SRC_CODE_DIR/libs/src/lib/hmac/*.h $PACAKGE_SUB_HMAC
cp -f $SRC_CODE_DIR/libs/src/lib/crypto/public/*.h $PACAKGE_SUB_CRYPT_PUB
cp -f $SRC_CODE_DIR/libs/src/lib/crypto/asymCrypto/*.h $PACAKGE_SUB_CRYPT_ASYM
cp -f $SRC_CODE_DIR/libs/src/lib/crypto/symCrypto/*.h $PACAKGE_SUB_CRYPT_SYM
cp -f $SRC_CODE_DIR/libs/src/lib/crypto/base64/*.h $PACAKGE_SUB_CRYPT_BASE64
cp -r $SRC_CODE_DIR/platform/os/inc/* $PLATFORM_OS
cp -r $SRC_CODE_DIR/libs/inc/* $PLATFORM_LIBSINC

#if [ $1 == "x86" ]; then
#    strip $SRC_CODE_DIR/build/libs/libfoundation.so
#else
#    ${STRIP} $SRC_CODE_DIR/build/libs/libfoundation.so
#fi

cp $SRC_CODE_DIR/build/libs/libfoundation.so $PACAKGE_LIBS
cp $SRC_CODE_DIR/build/libs/lib*.a $PACAKGE_LIBS

cp $SRC_CODE_DIR/build/libs/src/kvstorage/libadmkvstorage.a $PACAKGE_LIBS
cp $SRC_CODE_DIR/build/libs/src/iniparser/libadminiparser.a $PACAKGE_LIBS
cp $SRC_CODE_DIR/build/libs/src/observe/libadmasynchnotify.a $PACAKGE_LIBS
cp $SRC_CODE_DIR/build/libs/src/async/libadmasync.a $PACAKGE_LIBS
cp $SRC_CODE_DIR/build/libs/src/adm_minizip/libadm_minizip.a $PACAKGE_LIBS

cp $SRC_CODE_DIR/build/deploy/lib/lib*.a $PACAKGE_LIBS
cp $SRC_CODE_DIR/build/deploy/lib/lib*.so* $PACAKGE_LIBS

cp $SRC_CODE_DIR/build/platform/os/libadmos.a $PACAKGE_LIBS


#打包发布
TIMESTAMP=$(date +'%Y%m%d')
GIT_COMMITID=$(git rev-parse --short HEAD)
cd $SRC_CODE_DIR/build/output/
tar -czvf ${PROJECT_NAME}_$1_$2_${TIMESTAMP}_${GIT_COMMITID}.tar.gz *
cd ../..

echo "export file path = $SRC_CODE_DIR/build/output/${PROJECT_NAME}_$1_$2_${TIMESTAMP}_${GIT_COMMITID}.tar.gz"

