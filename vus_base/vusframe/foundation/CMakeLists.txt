cmake_minimum_required(VERSION 3.10)
project(foundation C CXX)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(CUSTOM_CMAKE_SYSTEM_NAME)
else()
set(CUSTOM_CMAKE_SYSTEM_NAME Linux)
set(CUSTOM_CMAKE_XML_NAME MXML)
endif()

if(${CUSTOM_CMAKE_SYSTEM_NAME} MATCHES "Android")
	option(SUPPORT_LION_PKI    "Support lion pki"     ON)
else()
	option(SUPPORT_LION_PKI    "Support lion pki"     OFF)
endif()


#if(${CUSTOM_CMAKE_SYSTEM_NAME} MATCHES "QNX7")
#    set(SYSTEM_LIBRARY c c++ m socket)
#    set(CMAKE_CXX_STANDARD 11)
#endif()

message(STATUS "CMAKE_SYSTEM_NAME: ${CMAKE_SYSTEM_NAME}")
message(STATUS "CMAKE_CROSSCOMPILING: ${CMAKE_CROSSCOMPILING}")
message(STATUS "CMAKE_TOOLCHAIN_FILE: ${CMAKE_TOOLCHAIN_FILE}")
message(STATUS "CUSTOM_CMAKE_SYSTEM_NAME: ${CUSTOM_CMAKE_SYSTEM_NAME}")
message(STATUS "CUSTOM_CMAKE_XML_NAME: ${CUSTOM_CMAKE_XML_NAME}")

set(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR}/build/deploy)
set(CMAKE_INSTALL_BINDIR ${CMAKE_INSTALL_PREFIX}/bin)
set(CMAKE_INSTALL_LIBDIR ${CMAKE_INSTALL_PREFIX}/lib)
set(CMAKE_INSTALL_INCLUDEDIR ${CMAKE_INSTALL_PREFIX}/include)
set(CMAKE_BUILD_TYPE Debug)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/package/zlib/zlib)
include_directories(${CMAKE_CURRENT_BINARY_DIR}/deploy/include/zlib)

###########################################################SDK VERSION########################################################################

# SET(MAJOR_VERSION 1)
# SET(MINOR_VERSION 1)
# SET(HOTFIX_VERSION 1)
SET(PROJECT_NAME "foundation")
message("SDK_VERSION = ${SDK_VERSION}")
SET(ADM_SDK_VERSION ${SDK_VERSION})

EXECUTE_PROCESS(COMMAND date +%Y%m%d OUTPUT_VARIABLE COMPILE_DATE OUTPUT_STRIP_TRAILING_WHITESPACE)
EXECUTE_PROCESS(COMMAND git rev-parse --short HEAD OUTPUT_VARIABLE GIT_COMMITID OUTPUT_STRIP_TRAILING_WHITESPACE)

SET(PROJECT_VERSION ${PROJECT_NAME}_${ENV_PLATFORM}_${ADM_SDK_VERSION}_${COMPILE_DATE}_${GIT_COMMITID})
ADD_DEFINITIONS(-DADM_FOUNDATION_SDK_VERSION="${PROJECT_VERSION}")
MESSAGE("+++++++++++PROJECT_VERSION = ${PROJECT_VERSION}")
###########################################################SDK VERSION########################################################################


option(LOG "Enable build information or not" ON)

if (LOG)
    set(LOG_LEVEL 0)
    message(STATUS "LOG:${LOG}" ";LOG_LEVEL:${LOG_LEVEL}")
else ()
    set(LOG_LEVEL 1)
    message(STATUS "LOG:${LOG}" ";LOG_LEVEL:${LOG_LEVEL}")
endif ()

if(${CUSTOM_CMAKE_SYSTEM_NAME} MATCHES "QNX7")
else()
	#list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/package/gtest")
	list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/package/polipo")
endif()
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/package/json")
#list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/package/jsoncpp")
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/package/iniparser")
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/package/openssl")

if(NOT SUPPORT_LION_PKI)
	list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/package/curl")
endif()
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/package/zlib")
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/package/log")
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/package/leveldb")
#list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/package/paho.mqtt")
#list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/package/lwm2m")
#list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/package/protobuf")

if (CUSTOM_CMAKE_XML_NAME)
	#list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/package/xml")
endif()

if(${CUSTOM_CMAKE_SYSTEM_NAME} MATCHES "QNX7")

elseif(${CUSTOM_CMAKE_SYSTEM_NAME} MATCHES "Linux")
	find_package(polipo)
else()
	#find_package(gtest)
	find_package(polipo)
endif()
find_package(json)
#find_package(jsoncpp)
find_package(iniparser)
#find_package(openssl)
if(NOT SUPPORT_LION_PKI)
	find_package(curl)
endif()
find_package(zlib)
find_package(log)
find_package(leveldb)

#find_package(paho.mqtt)
#find_package(lwm2m)
#find_package(protobuf)
if (CUSTOM_CMAKE_XML_NAME)
#    find_package(xml)
endif()


# add_subdirectory(package/polipo)
add_subdirectory(platform)
add_subdirectory(libs)
