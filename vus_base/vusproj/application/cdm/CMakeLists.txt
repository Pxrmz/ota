cmake_minimum_required(VERSION 3.10)
project(cdm CXX C)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC -g -std=gnu11")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -g -std=gnu11")

set(project_name "cdm")
add_definitions(-fPIC)

add_library(${project_name}
	STATIC
	${CMAKE_SOURCE_DIR}/application/cdm/src/adm_cdm_verify_interface.c
	${CMAKE_SOURCE_DIR}/application/cdm/src/adm_cdm_log.h
)

target_include_directories(
    ${project_name}
    PRIVATE
	${CMAKE_SOURCE_DIR}/application
    ${CMAKE_SOURCE_DIR}/application/cdm/inc/public
    ${CMAKE_SOURCE_DIR}/dim/inc/public
    ${CMAKE_SOURCE_DIR}/uim/inc/public
    ${CMAKE_SOURCE_DIR}/engine/cds/inc/public
    ${CMAKE_SOURCE_DIR}/foundation/platform/os
    ${CMAKE_SOURCE_DIR}/foundation/packages
    ${CMAKE_SOURCE_DIR}/engine
    ${CMAKE_SOURCE_DIR}/engine/common/config/inc
    ${CMAKE_SOURCE_DIR}/engine/common/lib/inc
    ${CMAKE_SOURCE_DIR}/foundation/packages/log
    ${CMAKE_SOURCE_DIR}/foundation/arpc/inc
    ${CMAKE_SOURCE_DIR}/soa/inc/public
	${CMAKE_SOURCE_DIR}/engine/config/inc
)

target_link_libraries(
    ${project_name}
    PRIVATE
	${CMAKE_SOURCE_DIR}/build/dim/libdim.a
    ${CMAKE_SOURCE_DIR}/engine/lib/libcore_service.a
    ${CMAKE_SOURCE_DIR}/engine/lib/libconfig.a
    ${FOUNDATION_LIB_PATH}/libfoundation.so
    ${DTS_TOOL_LIB}
    ${SYSTEM_LIBRARY}
)

#add_subdirectory(sample)

