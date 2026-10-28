
cmake_minimum_required(VERSION 3.0)
project(cdm_sample CXX C)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC -g -std=gnu11")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -g -std=gnu11")

set(project_name "cdm_sample")


add_executable(${project_name}
    ${CMAKE_SOURCE_DIR}/application/cdm/sample/cdm_test.cpp
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
	${CMAKE_SOURCE_DIR}/engine
    ${CMAKE_SOURCE_DIR}/engine/common/config/inc
    ${CMAKE_SOURCE_DIR}/engine/common/lib/inc
    ${CMAKE_SOURCE_DIR}/foundation/packages/log
    ${CMAKE_SOURCE_DIR}/foundation/arpc/inc
    ${CMAKE_SOURCE_DIR}/soa/inc/public

    ${CMAKE_SOURCE_DIR}/engine/common/errorCode/inc
    ${CMAKE_SOURCE_DIR}/engine/config/inc

)

target_link_libraries(
    ${project_name}
    PRIVATE
	${CMAKE_SOURCE_DIR}/build/application/cdm/libcdm.a
    ${CMAKE_SOURCE_DIR}/engine/lib/libcore_service.a
    ${CMAKE_SOURCE_DIR}/engine/lib/libconfig.a
    ${FOUNDATION_LIB_PATH}/libfoundation.so
    ${DTS_TOOL_LIB}
    ${SYSTEM_LIBRARY}

    ${SOA_LIBRARY}
    ${DIM_LIBRARY}
)
