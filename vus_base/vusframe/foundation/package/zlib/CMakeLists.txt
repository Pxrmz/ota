cmake_minimum_required(VERSION 3.10)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -fPIC")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -fPIC")

include_directories(./zlib)

add_subdirectory(zlib)
