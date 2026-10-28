cmake_minimum_required(VERSION 3.10)
project(leveldb)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -fPIC")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -fPIC")

# Allow enabling and disabling components
option(LEVELDB_BUILD_TESTS "Build LevelDB's unit tests" ON)
option(LEVELDB_BUILD_BENCHMARKS "Build LevelDB's benchmarks" ON)
option(LEVELDB_INSTALL "Install LevelDB's header and library" ON)

set(LEVELDB_BUILD_TESTS OFF)
set(LEVELDB_BUILD_BENCHMARKS OFF)
set(LEVELDB_INSTALL ON)
set(CMAKE_CXX_STANDARD 11)

add_subdirectory(leveldb-1.22)
