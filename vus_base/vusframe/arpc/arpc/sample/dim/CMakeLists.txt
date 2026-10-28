cmake_minimum_required(VERSION 3.10)
project(dim C)


set(project_name "dim")

add_executable(${project_name}
    dim.c
)

target_include_directories(
    ${project_name}
    PRIVATE

    ${CMAKE_SOURCE_DIR}/../engine_lib/config/inc
    ${CMAKE_SOURCE_DIR}/../engine_lib/common/errorCode/inc
    ${CMAKE_SOURCE_DIR}/../engine_lib/common/config/inc
    ${CMAKE_SOURCE_DIR}/../engine_lib/common/lib/inc

	${CMAKE_SOURCE_DIR}/../foundation_lib/${ENV_PLATFORM}/platform/libs/inc
	${CMAKE_SOURCE_DIR}/../foundation_lib/${ENV_PLATFORM}/platform/os
    ${CMAKE_SOURCE_DIR}/../foundation_lib/${ENV_PLATFORM}/packages/log
    ${CMAKE_SOURCE_DIR}/../foundation_lib/${ENV_PLATFORM}/packages/lib
    ${CMAKE_SOURCE_DIR}/../foundation_lib/${ENV_PLATFORM}/packages/libs/inc
    ${CMAKE_SOURCE_DIR}/../foundation_lib/${ENV_PLATFORM}/packages/json-c

    ${CMAKE_SOURCE_DIR}/../foundation/platform/libs/inc
    ${CMAKE_SOURCE_DIR}/../foundation/platform/os
    
    ${CMAKE_SOURCE_DIR}/arpc/inc/public
    ${CMAKE_SOURCE_DIR}/arpc/src
)

message(CMAKE_SOURCE_DIR: "${CMAKE_SOURCE_DIR}")

target_link_libraries(
    ${project_name}
    PRIVATE
    
    ${CMAKE_SOURCE_DIR}/build/arpc/libarpc.so
    stdc++
)

