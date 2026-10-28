cmake_minimum_required(VERSION 3.10)
project(iniparser)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -fPIC")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -fPIC")

set(CMAKE_INSTALL_PREFIX ${INSTALL_DIR})

option(BUILD_STATIC_LIBS "Build static libs?" ON)
set(BUILD_STATIC_LIBS ON)

add_subdirectory(iniparser-4.1)
