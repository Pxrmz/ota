#!/bin/bash

if [ $# -ne 2 ]; then
    echo "error $0 DIR PLATFORM"
    exit 1
fi
echo "generate ewebsrv_static.a $1 $2"

PLATFORM=$2

if [ ${OS} ]; then
    echo "OS: ${OS}}"
fi

echo "use PLATFORM:${PLATFORM}"
sleep 1

cd $1 
dirname="`date +%y%m%d%H%M%S`_atemp" 
    
if [ -x $dirname ]; then
    echo "$0 $dirname exists!"
    exit 1
fi

if [ -x ewebsrv_static.a ]; then
    rm ewebsrv_static.a
fi

mkdir $dirname
cd $dirname
ar x ../../civetweb/libs/${PLATFORM}/libcrypto.a
ar x ../../civetweb/libs/${PLATFORM}/libssl.a
ar x ../libewebsrvapi.a

if [[ $PLATFORM = Windows* || $PLATFORM = win32 ]]; then
    ar rcs ewebsrv_static.a *.obj	
else
    ar rcs ewebsrv_static.a *.o	
fi

mv ewebsrv_static.a ..
cd ..
rm -rf $dirname
cd ..

    
    
