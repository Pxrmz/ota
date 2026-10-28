cmake_minimum_required(VERSION 3.10)
project(openssl)

# Allow enabling and disabling components
option(CMAKE_DISABLE_SOURCE_CHANGES "Disable source changes" ON)
option(CMAKE_DISABLE_IN_SOURCE_BUILD "Disable in source build" ON)
option(CMAKE_DISABLE_SOURCE_CHANGES "Disable source changes" ON)
option(WITH_APPS "Build applications" OFF)

set(CMAKE_DISABLE_SOURCE_CHANGES ON)
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)
set(WITH_APPS OFF)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -fPIC")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -fPIC")

add_subdirectory(openssl-1.1.1w)
