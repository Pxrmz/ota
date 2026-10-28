#!/bin/sh
export QC9X07_PWD=/opt/gcc-arm
export PATH=$PATH:$QC9X07_PWD/bin/arm-oe-linux-gnueabi/
export SYSROOT=$QC9X07_PWD/9615-cdp
export CC="arm-oe-linux-gnueabi-gcc --sysroot=$SYSROOT "
export LD="arm-oe-linux-gnueabi-ld --sysroot=$SYSROOT "
export AR="arm-oe-linux-gnueabi-ar"
export STRIP="arm-oe-linux-gnueabi-strip "
export OBJCOPY="arm-oe-linux-gnueabi-objcopy "
export OBJDUMP="arm-oe-linux-gnueabi-objdump "
export RANLIB="arm-oe-linux-gnueabi-ranlib "
export CXX="arm-oe-linux-gnueabi-g++  --sysroot=$SYSROOT "
export CPP="arm-oe-linux-gnueabi-g++  --sysroot=$SYSROOT "
#export CROSS_COMPILE_POSTFIX=" --sysroot=$SYSROOT "


