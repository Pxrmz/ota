#!/bin/bash
set -e
set -x

# Set directory
SCRIPTPATH=`realpath .`
export ANDROID_NDK_HOME=/opt/android-ndk-r21d
OPENSSL_DIR=$SCRIPTPATH/civetweb/openssl

# Find the toolchain for your build machine
toolchains_path=$(python toolchains_path.py --ndk ${ANDROID_NDK_HOME})

# Configure the OpenSSL environment, refer to NOTES.ANDROID in OPENSSL_DIR
# Set compiler clang, instead of gcc by default
CC=clang

# Add toolchains bin directory to PATH
PATH=$toolchains_path/bin:$PATH

# Set the Android API levels
ANDROID_API=21

# Set the target architecture
# Can be android-arm, android-arm64, android-x86, android-x86 etc
#architecture=android-arm

# Create the make file
cd ${OPENSSL_DIR}
#./Configure ${architecture} -D__ANDROID_API__=$ANDROID_API

# Build
#make clean
#make

 
echo "$toolchains_path"
for ARCH in armeabi-v7a arm64-v8a x86_64 
do
	echo $ARCH
	if [ "$ARCH" = "armeabi-v7a" ]; then
		ARCH_NAME=android-arm
		TOOL_CHAIN=arm-linux-androideabi-4.9
	fi
	if [ "$ARCH" = "arm64-v8a" ]; then
		ARCH_NAME=android-arm64
		TOOL_CHAIN=aarch64-linux-android-4.9
	fi
	if [ "$ARCH" = "x86_64" ]; then
		ARCH_NAME=android-x86_64
		TOOL_CHAIN=x86_64-4.9
	fi
	echo $TOOL_CHAIN
        ./Configure $ARCH_NAME -D__ANDROID_API__=$ANDROID_API no-shared no-zlib no-asm  no-sse2 no-comp no-dso -DOPENSSL_THREADS -pthread -D_REENTRANT -D_THREAD_SAFE -D_THREADSAFE
        make  clean
        make

# Copy the outputs
	OUTPUT_INCLUDE=$SCRIPTPATH/output/include
	OUTPUT_LIB=$SCRIPTPATH/output/lib/android/$ARCH
	mkdir -p $OUTPUT_INCLUDE
	mkdir -p $OUTPUT_LIB
	cp -RL include/openssl $OUTPUT_INCLUDE
#	cp libcrypto.so $OUTPUT_LIB
	cp libcrypto.a $OUTPUT_LIB
#	cp libssl.so $OUTPUT_LIB
	cp libssl.a $OUTPUT_LIB
done





