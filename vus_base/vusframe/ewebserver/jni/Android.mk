# Copyright (C) 2009 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
#
LOCAL_PATH := $(call my-dir)/../civetweb
ROOT_PATH := $(LOCAL_PATH)/..
JNI_PATH := $(call my-dir)

common_cflags := \
    -std=gnu11  \
    -DNO_CACHING  \
    -DNO_CGI \
    -DNO_SSL_DL  \
    -DOPENSSL_API_1_1 \
    -DUSE_SERVER_STATS  \
    -DUSE_STACK_SIZE=102400 \
    -Wall -Wextra -Wshadow \
    -Wconversion -Wmissing-prototypes \
    -Wparentheses -Wno-padded \
    -Wno-unused-macros \
    -Wno-format-nonliteral \
    -Wno-cast-qual \
    -pedantic-errors \
    -fvisibility=hidden  
common_cppflags := \
    -std=c++11 \
    -Wnon-virtual-dtor \
    -Wdeprecated-declarations  \
    -fno-strict-aliasing \
    -fexceptions \
    -frtti
common_ldflags := \
    -Wl,--gc-sections

# ewebsrvapi (type: static_library)
# ========================================================
include $(CLEAR_VARS)
LOCAL_MODULE := libewebsrvapi
LOCAL_MODULE_CLASS := STATIC_LIBRARIES
LOCAL_CPP_EXTENSION := .cpp
LOCAL_CFLAGS := $(common_cflags)
LOCAL_CPPFLAGS := $(common_cppflags)
LOCAL_C_INCLUDES := \
    $(LOCAL_PATH)/include \
    $(LOCAL_PATH)/openssl/include \
    $(ROOT_PATH)/sdk/include  

LOCAL_SRC_FILES := \
	$(LOCAL_PATH)/src/civetweb.c       \
	$(LOCAL_PATH)/src/ewebsrvapi.c    \
	$(LOCAL_PATH)/src/ewebsrvcommon.cpp     
include $(BUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libcrypto_static
LOCAL_SRC_FILES := libs/android/$(TARGET_ARCH_ABI)/libcrypto.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libssl_static
LOCAL_SRC_FILES := libs/android/$(TARGET_ARCH_ABI)/libssl.a
include $(PREBUILT_STATIC_LIBRARY)

########################################################
# ewebsrvdemo client (type: executable)
# =============================================
#
include $(CLEAR_VARS)
LOCAL_MODULE := ewebsrvdemo 
LOCAL_MODULE_CLASS := EXECUTABLES
LOCAL_CFLAGS := $(common_cflags)
LOCAL_LDFLAGS := $(common_ldflags)
LOCAL_C_INCLUDES := \
    $(ROOT_PATH)/sdk/include  

LOCAL_STATIC_LIBRARIES := \
    libewebsrvapi \
    libcrypto_static \
    libssl_static       

LOCAL_SRC_FILES := $(LOCAL_PATH)/src/ewebsrvdemo.c
include $(BUILD_EXECUTABLE)


