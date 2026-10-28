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
###     @取值: x86:x86/ag35:tbox-4g/ag45:tbox-5g/android14:icc/qnx7:icc-vdi/qnx7-1:adcc
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



#导入debug cmake文件中的配置
if [ -f "CMakeLists_ucm.txt" ]; then
	cp CMakeLists_ucm.txt ../CMakeLists.txt

        line_num=`cat -n ../CMakeLists.txt |grep "#debug_control"| awk '{print $1}'`
        echo "line  $line_num"
        line_num=`expr $line_num + 1`
        cat CMakeLists_debug.txt | while read line
            do
                 sed -i "${line_num}i\ ${line}" ../CMakeLists.txt
                 line_num=`expr $line_num + 1`
            done

fi

# 创建目录和编译
cd ..
rm -rf ./build 
mkdir build

#配置文件选择,其他车型自行在此处添加配置文件 从控配置由主控统一布置
if [[ $1 == "android11" ]]; then
	if [[ $4 == "prod" ]]; then
		CONFIGFILE=./rawdata/etc/ICC-PROD-ucmconfig.ini
	else 
		CONFIGFILE=./rawdata/etc/ICC-ucmconfig.ini
	fi
	CONFIGFILE_UCL=./rawdata/etc/ICC-liteconfig.ini
else 
	CONFIGFILE=./rawdata/etc/default-ucmconfig.ini
	CONFIGFILE_UCL=./rawdata/etc/default-liteconfig.ini
fi

#兼容不同环境的库,减少底层Cmake针对不同环境的判断逻辑begin
if [[ "$1" == android* ]]; then
    ENVPLATFORM=android
else 
    ENVPLATFORM=$1
fi
echo "platform specified $ENVPLATFORM"

cp ./oca/vendor/lib/$ENVPLATFORM/*.so* ./oca/vendor/lib/
#兼容不同环境的库,减少底层Cmake针对不同环境的判断逻辑end

cd build

echo "platform specified $1"

# 生成OEM供应商需要提供的接口用户手册（Doxygen格式）
function docs {
    make  core_service_API_user_manual 2>&1 | tee build_doxygen.log
}

if [ $1 == "x86" ]; then
    cmake -DCMAKE_TOOLCHAIN_FILE=../cmake/toolchain-x86.cmake -DENV_PLATFORM=$1 -DSDK_VERSION=$2 -DVERSION_TYPE=${VERSION_TYPE} -DCLOUD_ENV=${CLOUD_ENV} -DVEHICLE_TYPE=${VEHICLE_TYPE} .. 
    #docs
    make
elif [ $1 == "ag35" ]; then
    cmake -DCMAKE_TOOLCHAIN_FILE=../cmake/toolchain-ag35.cmake -DENV_PLATFORM=$1 -DSDK_VERSION=$2 -DVERSION_TYPE=${VERSION_TYPE} -DCLOUD_ENV=${CLOUD_ENV} -DVEHICLE_TYPE=${VEHICLE_TYPE} .. 
    docs
    make 
elif [ $1 == "android11" ]; then
   cmake -DCMAKE_TOOLCHAIN_FILE=../cmake/toolchain-android.cmake -DENV_PLATFORM=$1 -DSDK_VERSION=$2 -DVERSION_TYPE=${VERSION_TYPE} -DCLOUD_ENV=${CLOUD_ENV} -DVEHICLE_TYPE=${VEHICLE_TYPE} .. 
   docs
   make 
elif [ $1 == "qnx7" ]; then
   cmake -DCMAKE_TOOLCHAIN_FILE=../cmake/toolchain-qnx7.cmake -DENV_PLATFORM=$1 -DSDK_VERSION=$2 -DVERSION_TYPE=${VERSION_TYPE} -DCLOUD_ENV=${CLOUD_ENV} -DVEHICLE_TYPE=${VEHICLE_TYPE} .. 
   docs
   make 
elif [ $1 == "qnx7-1" ]; then
    cmake -DCMAKE_TOOLCHAIN_FILE=../cmake/toolchain-qnx7-1.cmake -DENV_PLATFORM=$1 -DSDK_VERSION=$2 -DVERSION_TYPE=${VERSION_TYPE} -DCLOUD_ENV=${CLOUD_ENV} -DVEHICLE_TYPE=${VEHICLE_TYPE} .. 
    docs
    make 
else
    echo "platform specified error"
    exit 1
fi



if [ "$?" == "0" ]; then
    echo $?
    echo "build vus app successful"    
#    exit 1
else
    echo $?
    echo "build vus app error"    
    exit 1
fi

#回到顶层目录
cd ..

#创建文件目录
PROJECT_NAME=admfotaapp
ROM_FOLDER=admvus
TOPDIR=$(pwd)
OUTPUT_INCLUDE=$TOPDIR/build/output/${ROM_FOLDER}/inc
OUTPUT_ETC=$TOPDIR/build/output/${ROM_FOLDER}/etc
OUTPUT_BIN=$TOPDIR/build/output/${ROM_FOLDER}/bin
OUTPUT_LIB=$TOPDIR/build/output/${ROM_FOLDER}/lib
#OUTPUT_TRACER_RUNTIME=$TOPDIR/build/output/admnv/dlt

#OUTPUT_DOCS_DIR=${TOPDIR}/build/output/Docs

#创建目录
#mkdir -p $OUTPUT_DOCS_DIR

#移动用户手册到Docs目录并为用户手册目录添加指导链接文件
#cp -r ./build/engine/core_service_API_user_manual/ $OUTPUT_DOCS_DIR


# 发布打包 Begin
mkdir -p $OUTPUT_ETC
mkdir -p $OUTPUT_BIN
mkdir -p $OUTPUT_LIB
#mkdir -p $OUTPUT_TRACER_RUNTIME

# copy lib
cp ./build/oca/libadmoca.so $OUTPUT_LIB
cp ./oca/vendor/lib/*.so* $OUTPUT_LIB
cp ./foundation/packages/libs/libcrypto.so $OUTPUT_LIB
#cp ./foundation/packages/libs/libcrypto.so $OUTPUT_LIB/libcrypto.so.1.1
cp ./foundation/packages/libs/libssl.so $OUTPUT_LIB
#cp ./foundation/packages/libs/libssl.so $OUTPUT_LIB/libssl.so.1.1

cp ./foundation/arpc/lib/libarpc.so $OUTPUT_LIB
cp ./build/dim/dis/dissoa_proto_soc/libdisprotosoc.so $OUTPUT_LIB 2>/dev/null
cp ./build/dim/dis/dissoa_json_soc/libdisjsonsoc.so $OUTPUT_LIB 2>/dev/null

cp ./build/soa/soa_impl/soa_soc_impl/libsoasoc.so $OUTPUT_LIB 2>/dev/null
cp ./build/uim/uis/uissoa_json_soc/libuisjsonsoc.so $OUTPUT_LIB 2>/dev/null
cp ./foundation/packages/libs/libfoundation.so $OUTPUT_LIB

cp ./build/application/ucmCore/libucmCore.so $OUTPUT_LIB > /dev/null 2>&1
cp ./build/application/ucmJni/libcom_abupdate_vus_service.so $OUTPUT_LIB > /dev/null 2>&1

# copy bin
#cp ./tools/tracer/bin/admDtsDaemon $OUTPUT_BIN
#cp ./tools/tracer/bin/admDtsLoader $OUTPUT_BIN
#cp ./tools/tracer/bin/admDtsNodeTest $OUTPUT_BIN
#cp ./tools/tracer/lib/libadmdts.so $OUTPUT_LIB
#cp ./tools/tracer/etc/admdts.conf $OUTPUT_ETC
#cp ./tools/tracer/etc/gwNodes.conf $OUTPUT_ETC
#chmod +x $OUTPUT_BIN/admDtsDaemon
#chmod +x $OUTPUT_BIN/admDtsLoader
#chmod +x $OUTPUT_BIN/admDtsNodeTest
#chmod +x $OUTPUT_LIB/libadmdts.so

cp ./build/application/appcore/admfotaapp $OUTPUT_BIN > /dev/null 2>&1


#if [[ $4 == "prod" ]]; then
#	${STRIP} $OUTPUT_BIN/* > /dev/null 2>&1
#	${STRIP} $OUTPUT_LIB/*
#fi
#if [ $1 == "x86" ]; then
#	strip $OUTPUT_BIN/* > /dev/null 2>&1
#	strip $OUTPUT_LIB/*
#else
#	${STRIP} $OUTPUT_BIN/* > /dev/null 2>&1
#	${STRIP} $OUTPUT_LIB/*
#fi

# copy etc 主控代apk从控一起布置配置
cp $CONFIGFILE   $OUTPUT_ETC/ucmconfig.ini
cp $CONFIGFILE_UCL $OUTPUT_ETC/liteconfig.ini
cp ./rawdata/etc/Chery_Post_Program.ttb $OUTPUT_ETC
cp ./rawdata/etc/Chery_Pre_Program.ttb $OUTPUT_ETC
cp ./rawdata/etc/httpproxyCfg.ini $OUTPUT_ETC
cp ./rawdata/etc/replace.json $OUTPUT_ETC
cp ./rawdata/etc/schduleInstallConditions.json $OUTPUT_ETC
cp ./rawdata/etc/ICE-schduleInstallConditions.json $OUTPUT_ETC
cp ./rawdata/etc/smartScenConditions.json $OUTPUT_ETC
cp ./rawdata/etc/powerOnConditions.json $OUTPUT_ETC
cp ./rawdata/etc/ICE-powerOnConditions.json $OUTPUT_ETC
cp ./rawdata/etc/idelTransConditions.json $OUTPUT_ETC
cp ./rawdata/cert/*     $OUTPUT_ETC

#add sysi_data 目录
OUTPUT_SYSI_DATA=${TOPDIR}/build/output/admvus/etc/sysi_data
#add sysi_lua 目录
OUTPUT_SYSI_LUA=${TOPDIR}/build/output/admvus/etc/sysi_lua
#add lvm extern lib
OUTPUT_EXTERN_LIB=${TOPDIR}/rawdata/externLib
mkdir -p $OUTPUT_SYSI_DATA
mkdir -p $OUTPUT_SYSI_LUA
#copy lua
cp  ./sysi/application/lua_3th/lua_timer/lua/*.lua $OUTPUT_SYSI_LUA
cp  ./sysi/application/admSysi4HostAda/sysi4host_lvm/sysi4host_lua/*.lua $OUTPUT_SYSI_LUA
cp  ./sysi/application/admSysi4UclAda/sysi4ucl_lvm/sysi4ucl_lua/*.lua $OUTPUT_SYSI_LUA
#copy json
cp  ./sysi/application/admSysi4HostAda/sysi4host_lvm/sysi4host_data/*.json $OUTPUT_SYSI_DATA
cp  ./sysi/application/admSysi4UclAda/sysi4ucl_lvm/sysi4ucl_data/*.json $OUTPUT_SYSI_DATA
#copy extern lib
cp $OUTPUT_EXTERN_LIB/*.so $OUTPUT_ETC

#replace etc version
ETC_VERSION_FILE=${OUTPUT_ETC}/replace.json
source ${TOPDIR}/cmake/replaceVersion.sh ${ETC_VERSION_FILE} $2 $4

# compress output
cd $TOPDIR/application/
TIMESTAMP=$(date +'%Y%m%d')
GIT_COMMITID=$(git rev-parse --short HEAD)

cd $TOPDIR/build/output/
#tar -czvf ${PROJECT_NAME}_$1_$2_${TIMESTAMP}_${GIT_COMMITID}.tar.gz *
tar -czvf ${PROJECT_NAME}_${TOOLCHAIN}_${VEHICLE_TYPE}_${VERSION_TYPE}_${CLOUD_ENV}_${VERSION_NUMBER}_${TIMESTAMP}_${GIT_COMMITID}.tar.gz *

cd ../..

#删除因兼容不同环境的库,减少底层Cmake针对不同环境的判断逻辑产生的缓存文件
rm -f ./oca/vendor/lib/libscm.so > /dev/null 2>&1

rm -rf CMakeLists.txt

echo "export file path = $TOPDIR/build/output/${PROJECT_NAME}_${TOOLCHAIN}_${VEHICLE_TYPE}_${VERSION_TYPE}_${CLOUD_ENV}_${VERSION_NUMBER}_${TIMESTAMP}_${GIT_COMMITID}.tar.gz"


