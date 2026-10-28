cmake_minimum_required(VERSION 3.10)
project(SYSI4DPOADA CXX C)

set(project_name "sysi4dpoada")
    
add_library(${project_name}

    ./src/adm_sysi4dpo_handler.c)

target_include_directories(
    ${project_name}
    PRIVATE
    ${CMAKE_SOURCE_DIR}/engine/
    ${CMAKE_SOURCE_DIR}/sysi/engine/sysi4all/inc
    ${CMAKE_SOURCE_DIR}/sysi/engine/sysi4loader/inc/sysi4trans
    ${CMAKE_SOURCE_DIR}/sysi/engine/sysi4loader/inc/
    ${CMAKE_SOURCE_DIR}/sysi/engine/sysi4loader/inc/sysi4host
    ${CMAKE_SOURCE_DIR}/sysi/engine/sysi4loader/inc/sysi4ucl
    ${CMAKE_SOURCE_DIR}/sysi/engine/sysi4loader/inc/sysi4vdi
    ${CMAKE_SOURCE_DIR}/sysi/engine/sysi4loader/inc/sysi4dpo

    ${CMAKE_SOURCE_DIR}/application/ocm/inc/
    ${CMAKE_SOURCE_DIR}/oca/inc/
    
    ${CMAKE_SOURCE_DIR}/engine/config/inc/
    ${CMAKE_SOURCE_DIR}/engine/ecs/inc/public
    ${CMAKE_SOURCE_DIR}/engine/dpo/inc/public
    ${CMAKE_SOURCE_DIR}/engine/ocs/inc/public
    ${CMAKE_SOURCE_DIR}/engine/common/lib/inc
    ${CMAKE_SOURCE_DIR}/engine/common/config/inc
    ${CMAKE_SOURCE_DIR}/engine/common/errorCode/inc
    ${VENDOR_INC_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/inc
    ${CMAKE_SOURCE_DIR}/foundation/packages
    ${CMAKE_SOURCE_DIR}/foundation/packagescrypt
    ${CMAKE_SOURCE_DIR}/foundation/packages/openssl
    ${CMAKE_SOURCE_DIR}/foundation/packages/curl
    ${CMAKE_SOURCE_DIR}/foundation/packages/json-c
    ${CMAKE_SOURCE_DIR}/foundation/platform/os
    ${CMAKE_SOURCE_DIR}/foundation/packages/log
    ${CMAKE_SOURCE_DIR}/foundation/packages/dlt
    ${CMAKE_SOURCE_DIR}/tools/tracer/inc

    ${CMAKE_SOURCE_DIR}/arpc/inc/public
    ${CMAKE_SOURCE_DIR}/arpc
    ${CMAKE_SOURCE_DIR}/soa/inc/public 

)


target_link_libraries(
    ${project_name}
    PRIVATE
    ${SYSTEM_LIBRARY}

)
##add_subdirectory(sample)
