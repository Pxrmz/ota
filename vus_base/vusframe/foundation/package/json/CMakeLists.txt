cmake_minimum_required(VERSION 3.10)
project(json-c)

add_compile_options(-fPIC)


set(CMAKE_INSTALL_PREFIX ${INSTALL_DIR})

option(BUILD_STATIC_LIBS "Build static libs?" ON)
set(BUILD_STATIC_LIBS ON)

add_subdirectory(json-c)
