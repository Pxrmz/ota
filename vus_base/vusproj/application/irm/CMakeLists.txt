cmake_minimum_required(VERSION 3.10)
project(IRM CXX C)

set(project_name "irm")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} \
    -fPIC\
    ")
    
add_library(${project_name}
    src/adm_irm_report.c
    src/adm_irm_logFile.c
)

target_include_directories(
    ${project_name}
    PRIVATE
    inc/public/
    ${CMAKE_SOURCE_DIR}/engine
    ${CMAKE_SOURCE_DIR}/engine/ocs/inc/public
    ${CMAKE_SOURCE_DIR}/engine/irs/inc/public
    ${CMAKE_SOURCE_DIR}/engine/iss/inc/public
    ${CMAKE_SOURCE_DIR}/engine/ips/inc/public
    ${CMAKE_SOURCE_DIR}/engine/ecs/inc/public
    ${CMAKE_SOURCE_DIR}/engine/cvs/inc/public
    ${CMAKE_SOURCE_DIR}/engine/common/lib/inc
    ${CMAKE_SOURCE_DIR}/engine/common/config/inc
    ${CMAKE_SOURCE_DIR}/engine/common/errorCode/inc
    ${CMAKE_SOURCE_DIR}/engine/common/kvstorage/inc
    ${CMAKE_SOURCE_DIR}/engine/config/inc/
    ${CMAKE_SOURCE_DIR}/application/
    ${CMAKE_SOURCE_DIR}/application/pub/inc
    ${CMAKE_SOURCE_DIR}/application/irm/inc/public
    ${CMAKE_SOURCE_DIR}/application/irm/src
    ${CMAKE_SOURCE_DIR}/dim/inc/public/
    ${CMAKE_SOURCE_DIR}/soa/inc/public/
    ${CMAKE_SOURCE_DIR}/foundation/platform/os
    ${CMAKE_SOURCE_DIR}/foundation/packages/log
    ${CMAKE_SOURCE_DIR}/foundation/packages
    ${CMAKE_SOURCE_DIR}/foundation/platform/libs/inc/kvstorage
    ${CMAKE_SOURCE_DIR}/foundation/platform/libs/inc
    # compilation clipping
    ${CUSTOM_SUPPORT_HEADERS}
)

target_link_libraries(
    ${project_name}
    PRIVATE
    ${CMAKE_SOURCE_DIR}/build/externalLib/lib/libcore_service.a
)

#add_subdirectory(sample)
