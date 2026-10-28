cmake_minimum_required(VERSION 3.10)
project(protobuf)

# Allow enabling and disabling components
option(protobuf_BUILD_TESTS "Build tests" OFF)
option(protobuf_BUILD_CONFORMANCE "Build conformance tests" OFF)
option(protobuf_BUILD_EXAMPLES "Build examples" OFF)
option(protobuf_BUILD_PROTOC_BINARIES "Build libprotoc and protoc compiler" OFF)
option(BUILD_SHARED_LIBS "Enable for building shared library" OFF)
set(protobuf_WITH_ZLIB_DEFAULT OFF)
option(protobuf_VERBOSE "Enable for verbose output" ON)
# FindProtobuf module compatibel
option(protobuf_MODULE_COMPATIBLE "CMake build-in FindProtobuf.cmake module compatible" OFF)

set(protobuf_BUILD_TESTS OFF)
set(protobuf_BUILD_CONFORMANCE OFF)
set(protobuf_BUILD_EXAMPLES OFF)
set(protobuf_BUILD_PROTOC_BINARIES OFF)
set(BUILD_SHARED_LIBS OFF)
set(protobuf_WITH_ZLIB_DEFAULT OFF)
set(protobuf_VERBOSE ON)
# FindProtobuf module compatibel
set(protobuf_MODULE_COMPATIBLE OFF)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -fPIC")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -fPIC")

set(ZLIB_INCLUDE_DIRECTORIES ${CMAKE_INSTALL_PREFIX}/include)
set(ZLIB_INCLUDE_DIRS ${CMAKE_INSTALL_PREFIX}/include)
set(ZLIB_LIBRARIES ${CMAKE_INSTALL_PREFIX}/lib/libz.a)
add_subdirectory(protobuf)
