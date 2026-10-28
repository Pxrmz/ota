cmake_minimum_required(VERSION 3.10)
project(SYSI4CBF CXX C)

set(project_name "sysi4cbfada")

# begin: 添加cbf解析的头文件和源文件
#if (VEHICLE_TYPE)
#if (VEHICLE_TYPE AND NOT (VEHICLE_TYPE STREQUAL "T1J"))
#message(STATUS "${VEHICLE_TYPE} need cbf-parse")
include_directories(
    ${CMAKE_CURRENT_LIST_DIR}/cbf-parse/cbf_parse_lib/src/utility
    ${CMAKE_CURRENT_LIST_DIR}/cbf-parse/cbf_parse_lib/src/component
    ${CMAKE_CURRENT_LIST_DIR}/cbf-parse/cbf_parse_lib/inc
    ${CMAKE_CURRENT_LIST_DIR}/cbf-parse/cbf_parse_lib/lib/nlohmann/include
    
)

file(
    GLOB CBF_PARSE_FILE_LIST ${CMAKE_CURRENT_SOURCE_DIR}/cbf-parse/cbf_parse_lib/src/utility/*.cpp
	GLOB CBF_PARSE_FILE_LIST ${CMAKE_CURRENT_SOURCE_DIR}/cbf-parse/cbf_parse_lib/src/component/*.cpp
	GLOB CBF_PARSE_FILE_LIST ${CMAKE_CURRENT_SOURCE_DIR}/cbf-parse/cbf_parse_lib/src/*.cpp
)
#else()
#message(STATUS "${VEHICLE_TYPE} not need cbf-parse")
#endif()
# end: 添加cbf解析的头文件和源文件

add_library(${project_name}
    STATIC
     ./src/adm_sysi4cbf.cc
     ${CBF_PARSE_FILE_LIST}
)

target_include_directories(
    ${project_name}
    PRIVATE
    
    ${CMAKE_CURRENT_LIST_DIR}/inc
    ${CMAKE_SOURCE_DIR}/sysi/engine/sysi4all/inc
    ${CMAKE_SOURCE_DIR}/sysi/engine/sysi4loader/inc
    ${CMAKE_SOURCE_DIR}/sysi/engine/sysi4loader/inc/sysi4ucl
    ${CMAKE_SOURCE_DIR}/sysi/engine/sysi4loader/inc/sysi4trans
    ${CMAKE_SOURCE_DIR}/sysi/engine/sysi4loader/inc/sysi4vdi
    ${CMAKE_SOURCE_DIR}/sysi/engine/sysi4loader/inc/sysi4host
    ${CMAKE_SOURCE_DIR}/sysi/engine/sysi4loader/inc/sysi4dpo
    
    ${CMAKE_SOURCE_DIR}/sysi/application/libcbf/inc
    
    ${CMAKE_SOURCE_DIR}/foundation/platform/os
    ${CMAKE_SOURCE_DIR}/engine/common/errorCode/inc
    ${CMAKE_SOURCE_DIR}/engine/common/config/inc
    ${CMAKE_SOURCE_DIR}/engine/config/inc
    ${CMAKE_SOURCE_DIR}/engine/common/lib/inc
    
    
    ${CMAKE_SOURCE_DIR}/foundation/packages/log
    ${CMAKE_SOURCE_DIR}/tools/tracer/inc
    ${CMAKE_SOURCE_DIR}/foundation/packages/dlt
    
    ${CMAKE_SOURCE_DIR}/foundation/packages
)

target_link_libraries(
    ${project_name}
    PRIVATE
    #${FOUNDATION_LIB_PATH}/libfoundation.so
    #${SSL_LIBRARY}
    ${SYSTEM_LIBRARY}
)

