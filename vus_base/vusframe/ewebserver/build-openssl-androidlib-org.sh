#!/bin/bash
set -e
set -x

#https://proandroiddev.com/tutorial-compile-openssl-to-1-1-1-for-android-application-87137968fee
#Tutorial: Compile OpenSSL 1.1.1 for Android application

# Set directory
SCRIPTPATH=`realpath .`
export ANDROID_NDK_HOME=$SCRIPTPATH/android-ndk-r20
OPENSSL_DIR=$SCRIPTPATH/openssl-1.1.1c

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
architecture=android-arm

# Create the make file
cd ${OPENSSL_DIR}
./Configure ${architecture} -D__ANDROID_API__=$ANDROID_API

# Build
make

# Copy the outputs
OUTPUT_INCLUDE=$SCRIPTPATH/output/include
OUTPUT_LIB=$SCRIPTPATH/output/lib/${architecture}
mkdir -p $OUTPUT_INCLUDE
mkdir -p $OUTPUT_LIB
cp -RL include/openssl $OUTPUT_INCLUDE
cp libcrypto.so $OUTPUT_LIB
cp libcrypto.a $OUTPUT_LIB
cp libssl.so $OUTPUT_LIB
cp libssl.a $OUTPUT_LIB
