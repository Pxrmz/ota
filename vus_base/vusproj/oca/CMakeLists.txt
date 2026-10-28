cmake_minimum_required(VERSION 3.5.0)
PROJECT(admoca C CXX)

set(LIBRARY_NAME "admoca")
SET(CMAKE_BUILD_TYPE "Debug")

add_definitions(-fPIC)

add_library(
	${LIBRARY_NAME}
	SHARED
	#STATIC
	#${CMAKE_SOURCE_DIR}/engine/common/config/src/adm_com_modDefine.c
    src/http/adm_oca_httpCommon.c
    src/http/adm_oca_httpConnect.c
    src/http/adm_oca_httpFileDownload.c
    src/http/adm_oca_httpFileUpload.c
    src/adm_oca_connect.c
)

if (USE_PUBLIC_V50_PLATFORM)
    ADD_DEFINITIONS(-D USE_PUBLIC_V50_PLATFORM)
endif()

if (SUPPORT_LION_PKI)
    ADD_DEFINITIONS(-D SUPPORT_LION_PKI)
    set(OCA_VENDOR_INC_DIR ${CMAKE_SOURCE_DIR}/oca/vendor/include/)
endif()

if(${CUSTOM_CMAKE_SYSTEM_NAME} MATCHES "Android")
    set(LION_LIBSO 
        ${CMAKE_SOURCE_DIR}/oca/vendor/lib/libscm.so
        )
else()
    set(LION_LIBSO "")
endif()

target_include_directories(
    ${LIBRARY_NAME}
    PRIVATE
    ${CMAKE_SOURCE_DIR}/application/ems/inc/
    ${CMAKE_SOURCE_DIR}/application/pub/inc/
    ${CMAKE_SOURCE_DIR}/engine/common/config/inc
    ${CMAKE_SOURCE_DIR}/engine/ocs/inc/public/
    ${CMAKE_SOURCE_DIR}/engine/config/inc/
    ${CMAKE_SOURCE_DIR}/engine/common/lib/inc
    ${CMAKE_SOURCE_DIR}/foundation/platform/libs/inc/iniparser
    ${CMAKE_SOURCE_DIR}/engine/common/errorCode/inc
    ${CMAKE_SOURCE_DIR}/engine
        
    ${CMAKE_SOURCE_DIR}/foundation/packages
    ${CMAKE_SOURCE_DIR}/foundation/packagescrypt
    ${CMAKE_SOURCE_DIR}/foundation/packages/openssl
    ${CMAKE_SOURCE_DIR}/foundation/packages/curl
    ${CMAKE_SOURCE_DIR}/foundation/packages/json-c

    ${CMAKE_SOURCE_DIR}/foundation/packages/log
    ${CMAKE_SOURCE_DIR}/foundation/packages/lib
    ${CMAKE_SOURCE_DIR}/foundation/packages/lib/hmac

    ${CMAKE_SOURCE_DIR}/foundation/platform/libs/inc/kvstorage
    ${CMAKE_SOURCE_DIR}/foundation/platform/libs/inc/oca
    ${CMAKE_SOURCE_DIR}/foundation/platform/libs/inc
    ${CMAKE_SOURCE_DIR}/foundation/platform/os
    ${CMAKE_SOURCE_DIR}/foundation/packages/paho
    ./inc/
	./src/
    ./src/http
    ${OCA_VENDOR_INC_DIR}
    )

if(${CUSTOM_CMAKE_SYSTEM_NAME} MATCHES "Android")
target_link_libraries (
    ${LIBRARY_NAME}
    PRIVATE
    ${CMAKE_SOURCE_DIR}/build/externalLib/lib/libcore_service.a
	${CMAKE_SOURCE_DIR}/build/externalLib/lib/libconfig.a
    ${LION_LIBSO}
    ${FOUNDATION_LIB_PATH}/libfoundation.so
    )
else()
target_link_libraries (
    ${LIBRARY_NAME}
    PRIVATE
    ${CMAKE_SOURCE_DIR}/build/externalLib/lib/libcore_service.a
    ${CMAKE_SOURCE_DIR}/build/externalLib/lib/libconfig.a
    ${LION_LIBSO}
    ${SYSTEM_LIBRARY}
    )
endif()
#add_subdirectory(sample)
