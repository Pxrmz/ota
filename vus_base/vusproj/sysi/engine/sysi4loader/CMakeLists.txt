cmake_minimum_required(VERSION 3.10)
project(SYSI4ALLLOADER CXX C)

set(project_name "sysi4loader")

add_library(${project_name}
    STATIC
    ./src/adm_sysi4all_loader.cc
)

target_include_directories(
    ${project_name}
    PRIVATE
    ${CMAKE_SOURCE_DIR}/foundation/arpc/inc
    ${CMAKE_SOURCE_DIR}/soa/inc/public
    ${CMAKE_SOURCE_DIR}/engine/common/lib/inc
    ${CMAKE_SOURCE_DIR}/engine/common/config/inc
    ${CMAKE_SOURCE_DIR}/engine/config/inc/
    ${CMAKE_SOURCE_DIR}/foundation/packages/log
    ${CMAKE_SOURCE_DIR}/foundation/packages
    ${CMAKE_SOURCE_DIR}/foundation/packages/vsomeip
    ${CMAKE_SOURCE_DIR}/foundation/platform/os
    ${CMAKE_SOURCE_DIR}/engine/common/errorCode/inc
    ${CMAKE_SOURCE_DIR}/sysi/engine/sysi4all/inc
    ${CMAKE_SOURCE_DIR}/sysi/engine/sysiInit/inc
    ${CMAKE_CURRENT_LIST_DIR}/inc/
    ${CMAKE_CURRENT_LIST_DIR}/inc/proto
    ${CMAKE_SOURCE_DIR}/foundation/tools/tracer/inc
    ${CMAKE_SOURCE_DIR}/foundation/packages/dlt
    ${CMAKE_SOURCE_DIR}/foundation/platform/libs/inc
    ${CMAKE_SOURCE_DIR}/sysi/engine/sysi4loader/inc
    ${CMAKE_SOURCE_DIR}/sysi/engine/sysi4loader/inc/sysi4vdi
    ${CMAKE_SOURCE_DIR}/sysi/engine/sysi4loader/inc/sysi4host
    ${CMAKE_SOURCE_DIR}/sysi/engine/sysi4loader/inc/sysi4ucl
    ${CMAKE_SOURCE_DIR}/sysi/engine/sysi4loader/inc/sysi4trans
    ${CMAKE_SOURCE_DIR}/sysi/engine/sysi4loader/inc/sysi4dpo
    ${CMAKE_SOURCE_DIR}/sysi/engine/sysi4loader/src
    ${CMAKE_SOURCE_DIR}/sysi/engine/sysi4loader/json_soc/inc
    #${CMAKE_SOURCE_DIR}/sysi/engine/sysi4loader/proto_soc/inc
    #${CMAKE_SOURCE_DIR}/sysi/engine/sysi4loader/proto_soc/src
)


target_link_libraries(
    ${project_name}
    PRIVATE
    ${SYSTEM_LIBRARY}
)

#add_subdirectory(proto_soc)
add_subdirectory(json_soc)
