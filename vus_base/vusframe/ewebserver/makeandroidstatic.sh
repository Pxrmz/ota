#!/bin/bash


SCRIPTPATH=`realpath .`

if [ $# -ne 1 ]; then
    echo "error $0 DIR for android PLATFORM"
    exit 1
fi
echo "generate android ewebsrv_static.a $1 $2"

cd $1 
dirname="`date +%y%m%d%H%M%S`_atemp" 
    
if [ -x $dirname ]; then
    echo "$0 $dirname exists!"
    exit 1
fi

set -e
set -x

for ARCH in armeabi-v7a arm64-v8a x86_64 
do
	echo $ARCH
    mkdir $dirname
    cd $dirname

    ar x $SCRIPTPATH/civetweb/libs/android/$ARCH/libcrypto.a
    ar x $SCRIPTPATH/civetweb/libs/android/$ARCH/libssl.a
    ar x ../$ARCH/libewebsrvapi.a
    ar rcs ewebsrv_static.a *.o	

# Copy the outputs
	OUTPUT_LIB=$SCRIPTPATH/sdk/lib/android/$ARCH
	mkdir -p $OUTPUT_LIB
	mv ewebsrv_static.a $OUTPUT_LIB
	cp ../$ARCH/ewebsrvdemo $OUTPUT_LIB
	cd ..
	rm -rf $dirname
done

echo "Done!"
