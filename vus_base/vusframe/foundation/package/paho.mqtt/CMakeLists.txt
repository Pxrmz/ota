cmake_minimum_required(VERSION 3.10)
project(paho.mqtt.c)

option(PAHO_BUILD_STATIC "Build static library" ON)
option(PAHO_ENABLE_TESTING "Build tests and run" ON)

set(PAHO_BUILD_STATIC ON)

set(CMAKE_INSTALL_INCLUDEDIR ${CMAKE_INSTALL_PREFIX}/include/paho)

if(CUSTOM_CMAKE_SYSTEM_NAME MATCHES "Android")
    set(PAHO_ENABLE_TESTING OFF)
    set(CMAKE_SYSTEM_NAME "Android")
endif()

add_definitions(-fPIC)
add_subdirectory(paho.mqtt.c)
