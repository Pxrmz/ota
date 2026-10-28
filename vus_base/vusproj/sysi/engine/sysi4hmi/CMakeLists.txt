cmake_minimum_required(VERSION 3.0)
project(sysi4hmi CXX C)

SET(project_name "sysi4hmi")
SET(CMAKE_BUILD_TYPE "Debug")

add_definitions(-fPIC)

add_library(
    ${project_name}
    STATIC
    ${CMAKE_SOURCE_DIR}/sysi/engine/sysi4hmi/src/adm_sysi4hmi_check.c
    ${CMAKE_SOURCE_DIR}/sysi/engine/sysi4hmi/src/adm_sysi4hmi_download.c
    ${CMAKE_SOURCE_DIR}/sysi/engine/sysi4hmi/src/adm_sysi4hmi_install.c
    ${CMAKE_SOURCE_DIR}/sysi/engine/sysi4hmi/src/adm_sysi4hmi_general.c
    ${CMAKE_SOURCE_DIR}/sysi/engine/sysi4hmi/src/adm_sysi4hmi_public.cpp
    ${CMAKE_SOURCE_DIR}/sysi/engine/sysi4hmi/src/adm_sysi4hmi_servhandle.cc
    ${CMAKE_SOURCE_DIR}/sysi/engine/sysi4hmi/src/adm_sysi4hmi_icp.c
    # ${CMAKE_SOURCE_DIR}/sysi/engine/sysi4hmi/src/adm_sysi4hmi_emc.c
)

target_include_directories(
    ${project_name}
    PRIVATE
    ${CMAKE_SOURCE_DIR}/sysi/engine/sysi4all/inc
    ${CMAKE_SOURCE_DIR}/sysi/engine/sysi4hmi/inc
    ${CMAKE_SOURCE_DIR}/foundation/packages/log
    ${CMAKE_SOURCE_DIR}/foundation/platform/os
    ${CMAKE_SOURCE_DIR}/foundation/packages/
    ${CMAKE_SOURCE_DIR}/engine/common/config/inc
    ${CMAKE_SOURCE_DIR}/foundation/platform/libs/inc
    ${CMAKE_SOURCE_DIR}/foundation/packages/json-c
    ${CMAKE_SOURCE_DIR}/engine/common/errorCode/inc
    ${CMAKE_SOURCE_DIR}/engine/config/inc
    ${CMAKE_SOURCE_DIR}/foundation/arpc/inc
    ${CMAKE_SOURCE_DIR}/soa/inc/public
    )


target_link_libraries(
    ${project_name}
    PRIVATE
    #-Wl,--whole-archive

    #-Wl,--no-whole-archive
    ${SYSTEM_LIBRARY}
)

#add_subdirectory(sample)
