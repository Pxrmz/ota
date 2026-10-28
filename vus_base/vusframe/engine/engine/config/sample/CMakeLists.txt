cmake_minimum_required(VERSION 3.10)
project(config CXX C)

set(project_name "admcfgsample")

add_executable(${project_name}
	adm_cfg_sample.c
)

target_include_directories(
    ${project_name}
    PRIVATE
    ${CMAKE_SOURCE_DIR}/engine/common/config/inc/
    ${CMAKE_SOURCE_DIR}/engine/config/inc/
    ${CMAKE_SOURCE_DIR}/../foundation_lib/${ENV_PLATFORM}/platform/os
    ${CMAKE_SOURCE_DIR}/../foundation_lib/${ENV_PLATFORM}/packages/log
    ${CMAKE_SOURCE_DIR}/../foundation_lib/${ENV_PLATFORM}/packages
    ${CMAKE_SOURCE_DIR}/../foundation_lib/${ENV_PLATFORM}/platform/libs/inc/iniparser
    ${CMAKE_SOURCE_DIR}/engine/common/errorCode/inc
    ${CMAKE_SOURCE_DIR}/engine/
    ${CMAKE_SOURCE_DIR}/application
)

target_link_libraries(
        ${project_name}
        PRIVATE
        ${CMAKE_SOURCE_DIR}/build/engine/config/libconfig.a
        ${CMAKE_SOURCE_DIR}/build/engine/libcore_service.a
        ${FOUNDATION_LIB_PATH}/libfoundation.so
        ${SSL_LIBRARY}
        ${SYSTEM_LIBRARY}
)

