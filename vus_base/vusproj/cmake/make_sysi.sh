#!/bin/bash

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

#工程名
PROJECT_NAME=sysi

if [ $# -lt 5 ]
then
    echo $#
    echo "Usage: param is invaild
        ./make_ucm.sh toolchain version_number version_type clound_env vehicle_type
    example:
    toolchain:
        android11
        qnx7

    version_number:
        0.8.2

    version_type:
        cn

    clound_env:
        test  //客户测试环境
        prod  //客户生产环境

    vehicle_type:
        V23
        D01
        T1J
        E01
        "

    exit 1;
fi


TOOLCHAIN=$1
VERSION_NUMBER=$2
VERSION_TYPE=$3
CLOUD_ENV=$4
VEHICLE_TYPE=$5


if [ -f "CMakeLists_sysi.txt" ]; then
	cp CMakeLists_sysi.txt ../CMakeLists.txt
fi

# 创建目录和编译
cd ..
rm -rf ./build 
mkdir build

#配置文件选择,其他车型自行在此处添加配置文件
if [[ $1 == "android11" ]]; then
	CONFIGFILE=./rawdata/etc/ICC-liteconfig.ini
elif [ $1 == "ag35" ]; then
	CONFIGFILE=./rawdata/etc/TBOX-yundong-liteconfig.ini
elif [ $1 == "arma7" ]; then
	CONFIGFILE=./rawdata/etc/TBOX-4G-liteconfig.ini
elif [ $1 == "qnx7" ]; then
	CONFIGFILE=./rawdata/etc/ICC-vdiconfig.ini
elif [ $1 == "qnx7-1" ]; then
	CONFIGFILE=./rawdata/etc/ADCC-liteconfig.ini
else 
	CONFIGFILE=./rawdata/etc/default-liteconfig.ini
fi
	

#兼容不同环境的库,减少底层Cmake针对不同环境的判断逻辑begin
if [[ "$1" == android* ]]; then
    ENVPLATFORM=android
else 
    ENVPLATFORM=$1
fi
echo "platform specified $ENVPLATFORM"

cp ./oca/vendor/lib/$ENVPLATFORM/*.so* ./oca/vendor/lib/ > /dev/null 2>&1
#兼容不同环境的库,减少底层Cmake针对不同环境的判断逻辑end

cd build

echo "platform specified $1"

# 生成OEM供应商需要提供的接口用户手册（Doxygen格式）
#function docs {
#    make  Oem_User_Manual 2>&1 | tee build_doxygen.log
#    make  System_Interface_Service_User_Manual 2>&1 | tee build_doxygen.log
#    make  System_Interface_Client_User_Manual 2>&1 | tee build_doxygen.log
#}

if [ $1 == "x86" ]; then
    cmake -DCMAKE_TOOLCHAIN_FILE=../cmake/toolchain-x86.cmake -DENV_PLATFORM=$1 -DSDK_VERSION=$2 -DVERSION_TYPE=${VERSION_TYPE} -DCLOUD_ENV=${CLOUD_ENV} -DVEHICLE_TYPE=${VEHICLE_TYPE} ..  2>&1 | tee build-cmake.log
    if [ -f "Makefile" ]; then
    	#docs
    	make 
    else
    	echo "build fail"
    	exit 1
    fi
elif [ $1 == "x86_64" ]; then
    cmake -DCMAKE_TOOLCHAIN_FILE=../cmake/toolchain-x86.cmake -DENV_PLATFORM=$1 -DSDK_VERSION=$2 -DVERSION_TYPE=${VERSION_TYPE} -DCLOUD_ENV=${CLOUD_ENV} -DVEHICLE_TYPE=${VEHICLE_TYPE} ..  2>&1 | tee build-cmake.log
    if [ -f "Makefile" ]; then
    	#docs
    	make 
    else
    	echo "build fail"
    	exit 1
    fi
elif [ $1 == "ag35" ]; then
    cmake -DCMAKE_TOOLCHAIN_FILE=../cmake/toolchain-ag35.cmake -DENV_PLATFORM=$1 -DSDK_VERSION=$2 -DVERSION_TYPE=${VERSION_TYPE} -DCLOUD_ENV=${CLOUD_ENV} -DVEHICLE_TYPE=${VEHICLE_TYPE} ..  2>&1 | tee build-cmake.log
   if [ -f "Makefile" ]; then
   		#docs
    	make 
    else
    	echo "build fail"
    	exit 1
    fi
elif [ $1 == "arma7" ]; then
    cmake -DCMAKE_TOOLCHAIN_FILE=../cmake/toolchain-arma7.cmake -DENV_PLATFORM=$1 -DSDK_VERSION=$2 -DVERSION_TYPE=${VERSION_TYPE} -DCLOUD_ENV=${CLOUD_ENV} -DVEHICLE_TYPE=${VEHICLE_TYPE} ..  2>&1 | tee build-cmake.log
   if [ -f "Makefile" ]; then
   		#docs
    	make 
    else
    	echo "build fail"
    	exit 1
    fi
elif [ $1 == "qnx7-1" ]; then
    cmake -DCMAKE_TOOLCHAIN_FILE=../cmake/toolchain-qnx7-1.cmake -DENV_PLATFORM=$1 -DSDK_VERSION=$2 -DVERSION_TYPE=${VERSION_TYPE} -DCLOUD_ENV=${CLOUD_ENV} -DVEHICLE_TYPE=${VEHICLE_TYPE} ..  2>&1 | tee build-cmake.log
   if [ -f "Makefile" ]; then
   		#docs
    	make 
    else
    	echo "build fail"
    	exit 1
    fi
elif [ $1 == "android11" ]; then
   cmake -DCMAKE_TOOLCHAIN_FILE=../cmake/toolchain-android.cmake -DENV_PLATFORM=$1 -DSDK_VERSION=$2 -DVERSION_TYPE=${VERSION_TYPE} -DCLOUD_ENV=${CLOUD_ENV} -DVEHICLE_TYPE=${VEHICLE_TYPE} ..  2>&1 | tee build-cmake.log
   if [ -f "Makefile" ]; then
        #docs
        make 
    else
        echo "build fail"
        exit 1
    fi
elif [ $1 == "qnx7" ]; then
   cmake -DCMAKE_TOOLCHAIN_FILE=../cmake/toolchain-qnx7.cmake -DENV_PLATFORM=$1 -DSDK_VERSION=$2 -DVERSION_TYPE=${VERSION_TYPE} -DCLOUD_ENV=${CLOUD_ENV} -DVEHICLE_TYPE=${VEHICLE_TYPE} ..  2>&1 | tee build-cmake.log
   if [ -f "Makefile" ]; then
   		#docs
    	make 
    else
    	echo "build fail"
    	exit 1
    fi
else
    echo "platform specified error"
    exit 1
fi


if [ "$?" == "0" ]; then
    echo $?
    echo "build sysi successful"    
#    exit 1
else
    echo $?
    echo "build sysi error"    
    exit 1
fi


#回到顶层目录
cd ..

#创建文件目录
TOPDIR=$(pwd)
SYSIDIR=$TOPDIR/build/output/sysi
if [[ "$1" == qnx7 ]]; then
    OUTPUT_ETC=$SYSIDIR/m1e/etc
    OUTPUT_BIN=$SYSIDIR/m1e/bin
    OUTPUT_LIB=$SYSIDIR/m1e/lib
    #add sysi_data 目录
    OUTPUT_SYSI_DATA=${TOPDIR}/build/output/sysi/m1e/etc/sysi_data
    #add sysi_lua 目录
    OUTPUT_SYSI_LUA=${TOPDIR}/build/output/sysi/m1e/etc/sysi_lua
else
    OUTPUT_ETC=$SYSIDIR/etc
    OUTPUT_BIN=$SYSIDIR/bin
    OUTPUT_LIB=$SYSIDIR/lib
    #add sysi_data 目录
    OUTPUT_SYSI_DATA=${TOPDIR}/build/output/sysi/etc/sysi_data
    #add sysi_lua 目录
    OUTPUT_SYSI_LUA=${TOPDIR}/build/output/sysi/etc/sysi_lua
fi
#OUTPUT_TRACER_RUNTIME=$TOPDIR/build/output/admnv/dlt

#OUTPUT_DOCS_DIR=${TOPDIR}/build/output/Docs

#创建目录
#mkdir -p $OUTPUT_DOCS_DIR

#移动用户手册到Docs目录并为用户手册目录添加指导链接文件
#cp -r ./build/sysi/application/Oem_User_Manual/ $OUTPUT_DOCS_DIR
#cp -r ./tools/html/API.html $OUTPUT_DOCS_DIR/Oem_User_Manual/

#cp -r ./build/sysi/engine/System_Interface_Service_User_Manual/ $OUTPUT_DOCS_DIR
#cp -r ./tools/html/API.html $OUTPUT_DOCS_DIR/System_Interface_Service_User_Manual/

#cp -r ./build/sysi/engine/System_Interface_Client_User_Manual/ $OUTPUT_DOCS_DIR
#cp -r ./tools/html/API.html $OUTPUT_DOCS_DIR/System_Interface_Client_User_Manual/

# 发布打包 Begin
# mkdir -p $OUTPUT_INCLUDE
mkdir -p $OUTPUT_ETC
mkdir -p $OUTPUT_BIN
mkdir -p $OUTPUT_LIB
#mkdir -p $OUTPUT_TRACER_RUNTIME
mkdir -p $OUTPUT_SYSI_DATA
mkdir -p $OUTPUT_SYSI_LUA

# copy lib
cp ./build/sysi/application/admSysi4DeployApp/libsysi4deploy.so $OUTPUT_LIB
cp ./foundation/packages/libs/libcrypto.so $OUTPUT_LIB
cp ./foundation/packages/libs/libssl.so $OUTPUT_LIB

cp ./build/oca/libadmoca.so $OUTPUT_LIB

cp -rf ./build/sysi/engine/sysi4loader/proto_soc/libsysi4protosoc.so $OUTPUT_LIB 2>/dev/null
cp -rf ./build/sysi/engine/sysi4loader/json_soc/libsysi4jsonsoc.so $OUTPUT_LIB 2>/dev/null


cp ./foundation/arpc/lib/libarpc.so $OUTPUT_LIB
cp ./foundation/packages/libs/libfoundation.so $OUTPUT_LIB
cp ./build/soa/soa_impl/soa_soc_impl/libsoasoc.so $OUTPUT_LIB 2>/dev/null


# copy bin
if [ -f build/sysi/application/admSysi4DeployApp/sample/admSysi4X/TBOX/*App ]; then
	cp ./build/sysi/application/admSysi4DeployApp/sample/admSysi4X/TBOX/*App $OUTPUT_BIN/admsysiApp
elif [ -f build/sysi/application/admSysi4DeployApp/sample/admSysi4X/QNX/*App ]; then
	cp ./build/sysi/application/admSysi4DeployApp/sample/admSysi4X/QNX/*App $OUTPUT_BIN/admsysiApp
elif [ -f build/sysi/application/admSysi4DeployApp/sample/admSysi4X/ADCC/*App ]; then
	cp ./build/sysi/application/admSysi4DeployApp/sample/admSysi4X/ADCC/*App $OUTPUT_BIN/admsysiApp
elif [ -f build/sysi/application/admSysi4DeployApp/sample/admSysi4X/X86/*App ]; then
	cp ./build/sysi/application/admSysi4DeployApp/sample/admSysi4X/X86/*App $OUTPUT_BIN/admsysiApp
fi

if [ -f ./build/sysi/application/admSysi4HmiApp/sample/hmi_simulator ]; then
	cp ./build/sysi/application/admSysi4HmiApp/sample/hmi_simulator $OUTPUT_BIN
fi
#cp ./build/sysi/application/admSysi4HmiApp/sample/hmi_ctrl $OUTPUT_BIN

# copy etc
#cp ./tools/tracer/TracerNode/etc/passive.conf $OUTPUT_ETC
#cp ./tools/tracer/TracerNode/etc/gw.conf $OUTPUT_ETC
#cp ./tools/tracer/TracerNode/etc/gwNodes.conf $OUTPUT_ETC
#cp ./tools/tracer/TracerNode/etc/dlt_logstorage.conf $OUTPUT_ETC
#cp ./tools/tracer/TracerNode/etc/dlt_logstorage.conf $OUTPUT_TRACER_RUNTIME
cp $CONFIGFILE   $OUTPUT_ETC/liteconfig.ini
cp ./rawdata/cert/cdn.cert   $OUTPUT_ETC
cp ./rawdata/cert/arpcCA.crt   $OUTPUT_ETC
cp ./rawdata/cert/arpcClient.crt   $OUTPUT_ETC
cp ./rawdata/cert/arpcClient.key   $OUTPUT_ETC
cp ./rawdata/cert/ewebClient.crt   $OUTPUT_ETC
cp ./rawdata/cert/ewebClient.key   $OUTPUT_ETC
cp ./rawdata/cert/ewebCA.pem   $OUTPUT_ETC

if [[ "$1" == qnx7 ]]; then
	cp ./rawdata/etc/qnx-start.sh $OUTPUT_ETC/start.sh
elif [ "$1" == arma7 ]; then
    cp ./rawdata/etc/Huihan_start.sh $OUTPUT_ETC/start.sh
elif [ "$1" == ag35 ]; then
    cp ./rawdata/etc/Yundong_start.sh $OUTPUT_ETC/start.sh
fi

#copy lua
cp  ./sysi/application/lua_3th/lua_timer/lua/*.lua $OUTPUT_SYSI_LUA
cp  ./sysi/application/admSysi4HostAda/sysi4host_lvm/sysi4host_lua/*.lua $OUTPUT_SYSI_LUA
cp  ./sysi/application/admSysi4UclAda/sysi4ucl_lvm/sysi4ucl_lua/*.lua $OUTPUT_SYSI_LUA
#copy json
cp  ./sysi/application/admSysi4HostAda/sysi4host_lvm/sysi4host_data/*.json $OUTPUT_SYSI_DATA
cp  ./sysi/application/admSysi4UclAda/sysi4ucl_lvm/sysi4ucl_data/*.json $OUTPUT_SYSI_DATA

#copy lib
cp  ./build/sysi/application/lua_3th/lua_cjson/libcjson.so $OUTPUT_LIB
if [ -f ./build/sysi/jni/libcom_abupdate_vus_uclService.so ]; then
	cp ./build/sysi/jni/libcom_abupdate_vus_uclService.so $OUTPUT_LIB
fi


#icc content copy begin
if [[ "$1" == qnx7 ]]; then
	cp -rf ./sysi/application/admSysi4VdiAda/vdilib/*.so $OUTPUT_LIB
fi

if [[ $ENVPLATFORM == "android" ]]; then 
	cp ./build/sysi/application/admSysi4icc/libsysi4iccada.so $OUTPUT_LIB
fi
#icc content copy end 

#copy third library
# if [ $1 == "qnx7-1" ]; then
# 	cp  ./sysi/application/admSysi4DeployApp/vendor/ADCC/lib/*.so* $OUTPUT_LIB
# elif [ $1 == "ag566" ]; then
# 	cp  ./sysi/application/admSysi4DeployApp/vendor/TBOX-5G/lib/*.so* $OUTPUT_LIB
# elif [ $1 == "ag35" ]; then
# 	cp  ./sysi/application/admSysi4DeployApp/vendor/TBOX-4G/lib/*.so* $OUTPUT_LIB
# fi

cp ./oca/vendor/lib/libscm.so $OUTPUT_LIB

#if [ $1 == "x86" ]; then
#	strip $OUTPUT_BIN/* > /dev/null 2>&1
#	strip $OUTPUT_LIB/*
#else
#	${STRIP} $OUTPUT_BIN/* > /dev/null 2>&1
#	${STRIP} $OUTPUT_LIB/*
#fi
#if [[ $4 == "prod" ]]; then
#	${STRIP} $OUTPUT_BIN/* > /dev/null 2>&1
#	${STRIP} $OUTPUT_LIB/*
#else 
	if [[ $1 == "qnx7" ]] || [[ $1 == "arma7" ]] || [ $1 == "ag35" ]; then 
		${STRIP} $OUTPUT_BIN/* > /dev/null 2>&1
		${STRIP} $OUTPUT_LIB/*
	fi
#fi

# compress output
cd $TOPDIR/sysi/
TIMESTAMP=$(date +'%Y%m%d')
GIT_COMMITID=$(git rev-parse --short HEAD)

cd $TOPDIR/build/output/
#tar -czvf ${PROJECT_NAME}_$1_$2_${TIMESTAMP}_${GIT_COMMITID}.tar.gz *
tar -czvf ${PROJECT_NAME}_${TOOLCHAIN}_${VEHICLE_TYPE}_${VERSION_TYPE}_${CLOUD_ENV}_${VERSION_NUMBER}_${TIMESTAMP}_${GIT_COMMITID}.tar.gz *

cd ../..

#删除因兼容不同环境的库,减少底层Cmake针对不同环境的判断逻辑产生的缓存文件
rm -f ./sysi/application/admSysi4VdiAda/vdilib/*.so > /dev/null 2>&1

rm -f ./oca/vendor/lib/libscm.so > /dev/null 2>&1

rm -rf CMakeLists.txt

echo "export file path = $TOPDIR/build/output/${PROJECT_NAME}_${TOOLCHAIN}_${VEHICLE_TYPE}_${VERSION_TYPE}_${CLOUD_ENV}_${VERSION_NUMBER}_${TIMESTAMP}_${GIT_COMMITID}.tar.gz"

