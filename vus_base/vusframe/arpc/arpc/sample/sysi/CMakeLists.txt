cmake_minimum_required(VERSION 3.10)
project(CVS CXX C)


set(project_name "sysi1")

add_executable(${project_name}
    sysi1.c
)

target_include_directories(
    ${project_name}
    PRIVATE

    ${CMAKE_SOURCE_DIR}/../engine_lib/config/inc

    ${CMAKE_SOURCE_DIR}/../engine_lib/common/errorCode/inc
    ${CMAKE_SOURCE_DIR}/../engine_lib/common/config/inc

	${CMAKE_SOURCE_DIR}/../foundation_lib/${ENV_PLATFORM}/platform/libs/inc
	${CMAKE_SOURCE_DIR}/../foundation_lib/${ENV_PLATFORM}/platform/os
    ${CMAKE_SOURCE_DIR}/../foundation_lib/${ENV_PLATFORM}/packages/log
    ${CMAKE_SOURCE_DIR}/../foundation_lib/${ENV_PLATFORM}/packages/lib
    ${CMAKE_SOURCE_DIR}/../foundation_lib/${ENV_PLATFORM}/packages/libs/inc
    ${CMAKE_SOURCE_DIR}/../foundation_lib/${ENV_PLATFORM}/packages/json-c

    ${CMAKE_SOURCE_DIR}/../foundation/platform/libs/inc
    ${CMAKE_SOURCE_DIR}/../foundation/platform/os
    ${CMAKE_SOURCE_DIR}/../engine_lib/common/errorCode/inc
    ${CMAKE_SOURCE_DIR}/../engine_lib/common/config/inc
    ${CMAKE_SOURCE_DIR}/../engine_lib/common/lib/inc
    ${CMAKE_SOURCE_DIR}/../engine_lib/common/ssladapter/inc
    ${CMAKE_SOURCE_DIR}/arpc/inc/public
    ${CMAKE_SOURCE_DIR}/arpc/src
    ${CMAKE_SOURCE_DIR}/config/inc

    ${CMAKE_SOURCE_DIR}/foundation/packages/log
)

message(CMAKE_SOURCE_DIR: "${CMAKE_SOURCE_DIR}")

target_link_libraries(
    ${project_name}
    PRIVATE
    
    ${SSL_LIBRARY}
    ${SYSTEM_LIBRARY}
    
    ${CMAKE_SOURCE_DIR}/build/arpc/libarpc.so
    #pthread
    stdc++
)


set(project_name "sysi2")

add_executable(${project_name}
    sysi2.c
)

target_include_directories(
    ${project_name}
    PRIVATE

    ${CMAKE_SOURCE_DIR}/../engine_lib/config/inc

    ${CMAKE_SOURCE_DIR}/../engine_lib/common/errorCode/inc
    ${CMAKE_SOURCE_DIR}/../engine_lib/common/config/inc

	${CMAKE_SOURCE_DIR}/../foundation_lib/${ENV_PLATFORM}/platform/libs/inc
	${CMAKE_SOURCE_DIR}/../foundation_lib/${ENV_PLATFORM}/platform/os
    ${CMAKE_SOURCE_DIR}/../foundation_lib/${ENV_PLATFORM}/packages/log
    ${CMAKE_SOURCE_DIR}/../foundation_lib/${ENV_PLATFORM}/packages/lib
    ${CMAKE_SOURCE_DIR}/../foundation_lib/${ENV_PLATFORM}/packages/libs/inc
    ${CMAKE_SOURCE_DIR}/../foundation_lib/${ENV_PLATFORM}/packages/json-c

    ${CMAKE_SOURCE_DIR}/../foundation/platform/libs/inc
    ${CMAKE_SOURCE_DIR}/../foundation/platform/os
    ${CMAKE_SOURCE_DIR}/../engine_lib/common/errorCode/inc
    ${CMAKE_SOURCE_DIR}/../engine_lib/common/config/inc
    ${CMAKE_SOURCE_DIR}/../engine_lib/common/lib/inc
    ${CMAKE_SOURCE_DIR}/../engine_lib/common/ssladapter/inc
    ${CMAKE_SOURCE_DIR}/arpc/inc/public
    ${CMAKE_SOURCE_DIR}/arpc/src
    ${CMAKE_SOURCE_DIR}/config/inc

    ${CMAKE_SOURCE_DIR}/foundation/packages/log
)

message(CMAKE_SOURCE_DIR: "${CMAKE_SOURCE_DIR}")

target_link_libraries(
    ${project_name}
    PRIVATE
    
    ${CMAKE_SOURCE_DIR}/build/arpc/libarpc.so
    #pthread
    stdc++
)
