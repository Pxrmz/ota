cmake_minimum_required(VERSION 3.10)
project(config CXX C)

set(project_name "config")

# -Wno-write-strings 忽略c++字符串相关转换过程的编译警告 
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC -g -std=gnu11")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -g -std=c++11 -Wno-write-strings")

add_library(${project_name}
	./src/adm_cfg_api.c
)

target_include_directories(
    ${project_name}
    PRIVATE
    ${CMAKE_SOURCE_DIR}/engine/config/inc
    ${CMAKE_SOURCE_DIR}/engine
    ${CMAKE_SOURCE_DIR}/engine/common/config/inc
    ${CMAKE_SOURCE_DIR}/engine/common/errorCode/inc
    ${CMAKE_SOURCE_DIR}/engine/common/lib/inc
    ${CMAKE_SOURCE_DIR}/engine/cvs/inc/private
    ${CMAKE_SOURCE_DIR}/engine/cvs/inc/public
    ${CMAKE_SOURCE_DIR}/application/dim/inc/public
    ${CMAKE_SOURCE_DIR}/application/cvm/inc/public
    ${CMAKE_SOURCE_DIR}/engine/cvs/src
    ${CMAKE_SOURCE_DIR}/engine/dps/inc/public
    ${CMAKE_SOURCE_DIR}/engine/dpo/inc/public
    ${CMAKE_SOURCE_DIR}/engine/ecs/inc/public
    ${CMAKE_SOURCE_DIR}/engine/ips/inc/public
    ${CMAKE_SOURCE_DIR}/engine/ips/src
    ${CMAKE_SOURCE_DIR}/engine/irs/inc/public
    ${CMAKE_SOURCE_DIR}/engine/iss/inc/public
    ${CMAKE_SOURCE_DIR}/engine/ocs/inc/public
    ${CMAKE_SOURCE_DIR}/engine/ocs/src
    ${CMAKE_SOURCE_DIR}/engine/pis/inc/public
    ${CMAKE_SOURCE_DIR}/engine/tps/inc/public
    ${CMAKE_SOURCE_DIR}/engine/vrs/inc/public
    ${CMAKE_SOURCE_DIR}/engine/vrs/inc/private
    ${CMAKE_SOURCE_DIR}/engine/vrs/src
    ${CMAKE_SOURCE_DIR}/engine/vss/inc/public
    ${CMAKE_SOURCE_DIR}/engine/vss/inc/private

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
    ${foundation_lib/${ENV_PLATFORM}_path}/libfoundation.so
    ${SYSTEM_LIBRARY}
)

install(TARGETS ${project_name} LIBRARY DESTINATION "lib")

#add_subdirectory(sample)
