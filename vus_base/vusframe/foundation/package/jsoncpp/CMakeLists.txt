cmake_minimum_required(VERSION 3.10)
project(jsoncpp)

add_compile_options(-fPIC)


set(CMAKE_INSTALL_PREFIX ${INSTALL_DIR})

option(JSONCPP_WITH_TESTS "Compile and (for jsoncpp_check) run JsonCpp test executables" ON)
option(JSONCPP_WITH_POST_BUILD_UNITTEST "Automatically run unit-tests as a post build step" ON)
option(JSONCPP_WITH_PKGCONFIG_SUPPORT "Generate and install .pc files" ON)
option(JSONCPP_WITH_CMAKE_PACKAGE "Generate and install cmake package files" ON)

set(JSONCPP_WITH_TESTS OFF)
set(JSONCPP_WITH_POST_BUILD_UNITTEST OFF)
set(JSONCPP_WITH_PKGCONFIG_SUPPORT OFF)
set(JSONCPP_WITH_CMAKE_PACKAGE OFF)

add_subdirectory(jsoncpp)
