cmake_minimum_required(VERSION 3.0)
project(admoca CXX)

# set(CMAKE_C_FLAGS "-fPIC")
# set(CMAKE_CXX_FLAGS "-fPIC")
SET(CMAKE_BUILD_TYPE "Debug")

if (SUPPORT_LION_PKI)
    ADD_DEFINITIONS(-D SUPPORT_LION_PKI)

    if(${CUSTOM_CMAKE_SYSTEM_NAME} MATCHES "Linux")
        set(LION_LIBSO 
        ${CMAKE_SOURCE_DIR}/oca/vendor/lib/libscm.so
        ${CMAKE_SOURCE_DIR}/oca/vendor/lib/libmosquitto.so.1)
    else()
        set(LION_LIBSO ${CMAKE_SOURCE_DIR}/oca/vendor/lib/libscm.so)
    endif()

    set(OCA_VENDOR_INC_DIR ${CMAKE_SOURCE_DIR}/oca/vendor/include/)
else()
    set(LION_LIBSO "")
endif()

add_executable(
	adm_oca_sample
	./adm_oca_sample.cpp
)

target_include_directories(
	adm_oca_sample
    PUBLIC
    ${CMAKE_SOURCE_DIR}/engine/common/config/inc
    ${CMAKE_SOURCE_DIR}/engine/ocs/inc/public/
    ${CMAKE_SOURCE_DIR}/engine
    ${CMAKE_SOURCE_DIR}/engine/common/lib/inc
    ${CMAKE_SOURCE_DIR}/engine/common/errorCode/inc
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
    ${OCA_VENDOR_INC_DIR}
    ../inc/
)

target_link_libraries(
	adm_oca_sample	
    ${CMAKE_SOURCE_DIR}/build/externalLib/lib/libcore_service.a
    ${CMAKE_SOURCE_DIR}/build/oca/libadmoca.so
    ${LION_LIBSO}
    ${FOUNDATION_LIB_PATH}/libfoundation.so
    ${SYSTEM_LIBRARY}
)
