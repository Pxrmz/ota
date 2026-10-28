cmake_minimum_required(VERSION 3.10)
project(irs_sample CXX C)

set(project_name "irs_sample")

add_executable(${project_name}
    adm_irs_sample.c
)

target_include_directories(
    ${project_name}
    PRIVATE
    ${CMAKE_SOURCE_DIR}/application
    ${CMAKE_SOURCE_DIR}/config/inc/
    ${CMAKE_SOURCE_DIR}/engine
    ${CMAKE_SOURCE_DIR}/engine/common/config/inc
    ${CMAKE_SOURCE_DIR}/engine/common/errorCode/inc
    ${CMAKE_SOURCE_DIR}/engine/common/lib/inc
    ${CMAKE_SOURCE_DIR}/engine/cvs/inc/private
    ${CMAKE_SOURCE_DIR}/engine/cvs/inc/public
    ${CMAKE_SOURCE_DIR}/application/cvm/inc/public
    ${CMAKE_SOURCE_DIR}/engine/cvs/src
    ${CMAKE_SOURCE_DIR}/engine/dps/inc/public
    ${CMAKE_SOURCE_DIR}/engine/dpo/inc/public
    ${CMAKE_SOURCE_DIR}/engine/tps/inc/public
    ${CMAKE_SOURCE_DIR}/engine/ecs/inc/public
    ${CMAKE_SOURCE_DIR}/engine/tps/inc/public
    ${CMAKE_SOURCE_DIR}/engine/ips/inc/public
    ${CMAKE_SOURCE_DIR}/engine/ips/src
    ${CMAKE_SOURCE_DIR}/engine/irs/inc/public
    ${CMAKE_SOURCE_DIR}/engine/iss/inc/public
    ${CMAKE_SOURCE_DIR}/engine/ocs/inc/public
    ${CMAKE_SOURCE_DIR}/engine/ocs/src
    ${CMAKE_SOURCE_DIR}/engine/pis/inc/public
    ${CMAKE_SOURCE_DIR}/engine/pis/src
    ${CMAKE_SOURCE_DIR}/engine/vrs/inc/public
    ${CMAKE_SOURCE_DIR}/engine/vrs/inc/private
    ${CMAKE_SOURCE_DIR}/engine/vrs/src
    ${CMAKE_SOURCE_DIR}/engine/vss/inc/public
    ${CMAKE_SOURCE_DIR}/engine/vss/inc/private
    ${CMAKE_SOURCE_DIR}/engine/cds/inc/public
    

    ${CMAKE_SOURCE_DIR}/../foundation_lib/packages
    ${CMAKE_SOURCE_DIR}/../foundation_lib/packages/curl
    ${CMAKE_SOURCE_DIR}/../foundation_lib/packages/json-c
    ${CMAKE_SOURCE_DIR}/../foundation_lib/packages/log
    ${CMAKE_SOURCE_DIR}/../foundation_lib/packages/lib
    ${CMAKE_SOURCE_DIR}/../foundation_lib/packages/lib/hmac

    ${CMAKE_SOURCE_DIR}/../foundation_lib/platform/libs/inc/kvstorage
    ${CMAKE_SOURCE_DIR}/../foundation_lib/platform/libs/inc
    ${CMAKE_SOURCE_DIR}/../foundation_lib/platform/os

    ${CMAKE_SOURCE_DIR}/com/vrpc/sdk/include
    ${CMAKE_SOURCE_DIR}/com/vrpc/
    ${CMAKE_SOURCE_DIR}/com/oca/inc
    ${CMAKE_SOURCE_DIR}/engine/soa/inc/public
    ${CMAKE_SOURCE_DIR}/engine/soa/src
    ${CMAKE_SOURCE_DIR}/engine/com_interaction/inc/public
    ${CMAKE_SOURCE_DIR}/engine/com_interaction/src
    ../inc/public
)

target_link_libraries(
    ${project_name}
    PRIVATE
    ${CMAKE_SOURCE_DIR}/build/engine/libcore_service.a
    ${FOUNDATION_LIB_PATH}/libfoundation.so
    ${SYSTEM_LIBRARY}  
)
