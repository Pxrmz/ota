cmake_minimum_required (VERSION 3.10)
project(admdtsloaderwapper C CXX)

set(PROJECT_NAME "admdtsloaderwapper")
SET(CMAKE_BUILD_TYPE "Debug")

# include_directories(${CMAKE_CURRENT_LIST_DIR}/../../tracer/dts/inc)

# include_directories(${CMAKE_CURRENT_LIST_DIR}/inc)


add_definitions(-fPIC)

add_library(${PROJECT_NAME}
    STATIC
    ./dts/src/test-daemon-loader-wrapper.c
)

message("CMAKE_SOURCE_DIR = ${CMAKE_SOURCE_DIR}")

target_include_directories(
    ${PROJECT_NAME}
    PRIVATE
    ${CMAKE_SOURCE_DIR}/tracer/dts/inc
)

target_link_libraries(
	${PROJECT_NAME}
    PRIVATE
    ${SYSTEM_LIBRARY}
)



