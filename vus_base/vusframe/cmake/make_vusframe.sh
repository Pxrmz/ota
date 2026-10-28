#!/bin/bash -e
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

PLATFORM=$1
VERSION=$2

#是否由APK拉起来进程 0-非apk拉起 非0-apk拉起,硬件环境好了后直接删除
if [[ "$1" == android* ]]; then
	TEST_APK_IF_SUPPORT=1
else
	TEST_APK_IF_SUPPORT=0
fi

PROJECT_NAME=vusframe
TOPDIR=$(pwd)
SRC_CODE_DIR=$TOPDIR/../
PROJ_CODE_DIR=$TOPDIR/../../vusproj
FOUNDATION_OUT_PATH=$TOPDIR/../foundation/build/output
EWEBSERVER_OUT_PATH=$TOPDIR/../ewebserver/build/output
TOOLS_OUT_PATH=$TOPDIR/../tools/build/output
ENGINE_OUT_PATH=$TOPDIR/../engine/build/output/engine
ARPC_OUT_PATH=$TOPDIR/../arpc/build/output
VRPC_OUT_PATH=$TOPDIR/../vrpc/build/output

# 创建库文件输出目录
OUTPUT=$SRC_CODE_DIR/build/output
rm -rf  $SRC_CODE_DIR/build/
mkdir -p $OUTPUT

#正式环境好了后去掉TEST_APK_IF_SUPPORT条件,统一直接放到$TOPDIR/../foundation_lib/$PLATFORM/即可
if [[ "$1" == android* ]]; then
 	if [[ TEST_APK_IF_SUPPORT -eq 0 ]]; then
 		FOUNDATION_LIB_DIR=$TOPDIR/../foundation_lib/$PLATFORM/GENERAL_SUPPORT/
 	else
 		FOUNDATION_LIB_DIR=$TOPDIR/../foundation_lib/$PLATFORM/APK_SUPPORT/
 	fi
else
	FOUNDATION_LIB_DIR=$TOPDIR/../foundation_lib/$PLATFORM/
 fi

# foundation库的压缩包生成后，需要解压到foundation_lib目录下
function foundation_unzip() {
    	echo "foundation lib dir:[$FOUNDATION_LIB_DIR]"
	mkdir -p $FOUNDATION_LIB_DIR
    	tar -zxvf $OUTPUT/foundation*.tar.gz -C $FOUNDATION_LIB_DIR
	cp -rf $FOUNDATION_LIB_DIR/* $FOUNDATION_LIB_DIR/../
	if [[ "$1" == android* ]]; then
    		tar -zxvf $OUTPUT/foundation*.tar.gz -C $FOUNDATION_LIB_DIR/../
	fi
    	tar -zxvf $OUTPUT/foundation*.tar.gz -C $PROJ_CODE_DIR/foundation/
}

# ewebserver库的压缩包生成后，需要解压到ewebserver_lib目录下
function ewebserver_unzip() {
	mkdir -p $TOPDIR/../ewebserver_lib/$PLATFORM/
	tar -zxvf $OUTPUT/ewebserver*.tar.gz -C $TOPDIR/../ewebserver_lib/$PLATFORM/
	tar -zxvf $OUTPUT/ewebserver*.tar.gz -C $PROJ_CODE_DIR/foundation/ewebserver/
}

# tools库的压缩包生成后，需要解压到tools_lib目录下
function tools_unzip() {
	mkdir -p $TOPDIR/../tools_lib/$PLATFORM/
	tar -zxvf $OUTPUT/tools*.tar.gz -C $TOPDIR/../tools_lib/$PLATFORM/
	tar -zxvf $OUTPUT/tools*.tar.gz -C $PROJ_CODE_DIR/foundation/tools/
}

# engine库的压缩包生成后，需要解压到engine_lib目录下
function engine_unzip() {
	mkdir -p $TOPDIR/../engine_lib/
	tar -zxvf $OUTPUT/engine*.tar.gz -C $TOPDIR/../engine_lib/ 
	tar -zxvf $OUTPUT/engine*.tar.gz -C $PROJ_CODE_DIR/engine/ 
}

# arpc库的压缩包生成后，需要解压到arpc_lib目录下
function arpc_unzip() {
	mkdir -p $TOPDIR/../arpc_lib/$PLATFORM/
	tar -zxvf $OUTPUT/arpc*.tar.gz  -C $TOPDIR/../arpc_lib/$PLATFORM/
	tar -zxvf $OUTPUT/arpc*.tar.gz  -C $PROJ_CODE_DIR/foundation/arpc/
}


# 编译foundation & copy 编译输出的文件到vusframe的build/output/路径下
function make_foundation() {
    echo "1=[$1] 2=[$2] 3=[$3]"    
    $TOPDIR/make_foundation.sh $1 $2 $3
    cp $FOUNDATION_OUT_PATH/*.tar.gz $OUTPUT/
}

# 编译ewebserver & copy 编译输出的文件到vusframe的build/output/路径下
function make_ewebserver() {
    $TOPDIR/make_ewebserver.sh $1 $2
    cp $EWEBSERVER_OUT_PATH/*.tar.gz $OUTPUT/
}

# 编译tools & copy 编译输出的文件到vusframe的build/output/路径下
function make_tools() {
    $TOPDIR/make_tools.sh $1 $2
    cp $TOOLS_OUT_PATH/*.tar.gz $OUTPUT/
}

# 编译engine & copy 编译输出的文件到vusframe的build/output/路径下
function make_engine() {
    $TOPDIR/make_engine.sh $1 $2
    cp $ENGINE_OUT_PATH/*.tar.gz $OUTPUT/
}

# 编译ARPC & copy 编译输出的文件到vusframe的build/output/路径下
function make_arpc() {
    $TOPDIR/make_arpc.sh $1 $2
    cp $ARPC_OUT_PATH/*.tar.gz $OUTPUT/
}

#控制foundation和arpc是否编译
COMPILE_FOUNDATION_CODE=OFF
COMPILE_ARPC_CODE=OFF
COMPILE_TOOLS_CODE=OFF
COMPILE_EWEBSERVER_CODE=OFF

if [ "$COMPILE_FOUNDATION_CODE" == "ON" ]; then 
	make_foundation $PLATFORM $VERSION $TEST_APK_IF_SUPPORT
	foundation_unzip
else
       cp -r -f $FOUNDATION_LIB_DIR/* $PROJ_CODE_DIR/foundation/ 
fi


if [ "$COMPILE_EWEBSERVER_CODE" == "ON" ]; then 
	make_ewebserver $PLATFORM $VERSION
	ewebserver_unzip
else 
	cp -r -f $TOPDIR/../ewebserver_lib/$PLATFORM/* $PROJ_CODE_DIR/foundation/ewebserver/
fi

if [[ $PLATFORM == android* ]] || [[ $PLATFORM == x86 ]]; then
	if [ "$COMPILE_TOOLS_CODE" == "ON" ]; then 
		make_tools $PLATFORM $VERSION
		tools_unzip
	else 
		cp -r -f $TOPDIR/../tools_lib/$PLATFORM/* $PROJ_CODE_DIR/foundation/tools/
	fi
fi

make_engine $PLATFORM $VERSION
engine_unzip

if [ "$COMPILE_ARPC_CODE" == "ON" ]; then 
	make_arpc $PLATFORM $VERSION
	arpc_unzip
else 
	cp -r -f $TOPDIR/../arpc_lib/$PLATFORM/* $PROJ_CODE_DIR/foundation/arpc/
fi


if [ "$?" == "0" ]; then
    echo $?
    echo "build vusframe successful"    
#    exit 1
else
    echo $?
    echo "build vusframe error"    
    exit 1
fi
