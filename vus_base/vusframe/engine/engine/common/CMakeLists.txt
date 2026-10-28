cmake_minimum_required(VERSION 3.10)
project(config CXX C)

set(project_name "common")

# -Wno-write-strings 忽略c++字符串相关转换过程的编译警告 
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC -g -std=gnu11")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -g -std=c++11 -Wno-write-strings")

add_library(${project_name}
	./config/src/adm_com_config.c
	./config/src/adm_com_modDefine.c
	./lib/src/adm_com_hash.c
	./lib/src/adm_com_lib.c
	./lib/src/adm_com_parseFileObject.c
	${SSL_SRC}
)

target_include_directories(
    ${project_name}
    PRIVATE
    ${CMAKE_SOURCE_DIR}/engine/config/inc
    ${CMAKE_SOURCE_DIR}/engine/common/config/inc
    ${CMAKE_SOURCE_DIR}/engine/common/errorCode/inc
    ${CMAKE_SOURCE_DIR}/engine/common/lib/inc
    ${CMAKE_SOURCE_DIR}/engine/common/ssladapter/inc

    ${CMAKE_SOURCE_DIR}/../foundation_lib/${ENV_PLATFORM}/packages
    ${CMAKE_SOURCE_DIR}/../foundation_lib/${ENV_PLATFORM}/packagescrypt
    ${CMAKE_SOURCE_DIR}/../foundation_lib/${ENV_PLATFORM}/packages/openssl
    ${CMAKE_SOURCE_DIR}/../foundation_lib/${ENV_PLATFORM}/packages/curl
    ${CMAKE_SOURCE_DIR}/../foundation_lib/${ENV_PLATFORM}/packages/json-c

    ${CMAKE_SOURCE_DIR}/../foundation_lib/${ENV_PLATFORM}/packages/log
    ${CMAKE_SOURCE_DIR}/../foundation_lib/${ENV_PLATFORM}/packages/lib
    ${CMAKE_SOURCE_DIR}/../foundation_lib/${ENV_PLATFORM}/packages/lib/hmac

    ${CMAKE_SOURCE_DIR}/../foundation_lib/${ENV_PLATFORM}/platform/libs/inc/kvstorage
    ${CMAKE_SOURCE_DIR}/../foundation_lib/${ENV_PLATFORM}/platform/libs/inc/iniparser
    ${CMAKE_SOURCE_DIR}/../foundation_lib/${ENV_PLATFORM}/platform/libs/inc
    ${CMAKE_SOURCE_DIR}/../foundation_lib/${ENV_PLATFORM}/platform/os
)

target_link_libraries(
    ${project_name}
    PRIVATE
    ${SYSTEM_LIBRARY}

    -Wl,--whole-archive
	${FOUNDATION_LIB_PATH}/libadm_minizip.a
    ${FOUNDATION_LIB_PATH}/libz.a
	-Wl,--no whole-archive
)

install(TARGETS ${project_name} LIBRARY DESTINATION "lib")
