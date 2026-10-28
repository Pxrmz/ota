cmake_minimum_required(VERSION 3.10)
project(PUB CXX C)
set(project_name "pub")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} \
    -fPIC\
    ")

add_library(${project_name}
    src/adm_pub_keylog.c
    src/adm_pub_func.c
)

target_include_directories(
    ${project_name}
    PRIVATE
    ${CMAKE_CURRENT_DIR}/src
    ${CMAKE_SOURCE_DIR}/engine
    ${CMAKE_SOURCE_DIR}/engine/config/inc
    ${CMAKE_SOURCE_DIR}/engine/common/config/inc
    ${CMAKE_SOURCE_DIR}/engine/common/errorCode/inc
    ${CMAKE_SOURCE_DIR}/engine/common/lib/inc
    ${CMAKE_SOURCE_DIR}/dim/inc/public
    ${CMAKE_SOURCE_DIR}/application
    ${CMAKE_SOURCE_DIR}/application/pub/inc
    ${CMAKE_SOURCE_DIR}/engine/ecs/inc/public
    ${CMAKE_SOURCE_DIR}/engine/ocs/inc/public
    ${CMAKE_SOURCE_DIR}/engine/vrs/inc/public
    ${CMAKE_SOURCE_DIR}/engine/vrs/inc/private

    ${CMAKE_SOURCE_DIR}/oca/inc/
    ${CMAKE_SOURCE_DIR}/dim/inc/public
    ${CMAKE_SOURCE_DIR}/foundation/packages
    ${CMAKE_SOURCE_DIR}/foundation/packagescrypt
    ${CMAKE_SOURCE_DIR}/foundation/packages/openssl
    ${CMAKE_SOURCE_DIR}/foundation/packages/curl
    ${CMAKE_SOURCE_DIR}/foundation/packages/json-c

    ${CMAKE_SOURCE_DIR}/foundation/packages/log
    ${CMAKE_SOURCE_DIR}/foundation/packages/lib
    ${CMAKE_SOURCE_DIR}/foundation/packages/lib/hmac

    ${CMAKE_SOURCE_DIR}/foundation/platform/libs/inc/kvstorage
    ${CMAKE_SOURCE_DIR}/foundation/platform/libs/inc
    ${CMAKE_SOURCE_DIR}/foundation/platform/os
)

target_link_libraries(
    ${project_name}
    PRIVATE
    ${CMAKE_SOURCE_DIR}/build/engine/libcore_service.so
    ${FOUNDATION_LIB_PATH}/libfoundation.so
    ${SYSTEM_LIBRARY}
)
