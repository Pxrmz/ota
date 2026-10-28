cmake_minimum_required(VERSION 3.10)
project(ARPC CXX C)

###########################################################SDK VERSION########################################################################

# SET(MAJOR_VERSION 1)
# SET(MINOR_VERSION 1)
# SET(HOTFIX_VERSION 1)
SET(PROJECT_NAME "arpc")
message("SDK_VERSION = ${SDK_VERSION}")
SET(ADM_SDK_VERSION ${SDK_VERSION})

EXECUTE_PROCESS(COMMAND date +%Y%m%d OUTPUT_VARIABLE COMPILE_DATE OUTPUT_STRIP_TRAILING_WHITESPACE)
EXECUTE_PROCESS(COMMAND git rev-parse --short HEAD OUTPUT_VARIABLE GIT_COMMITID OUTPUT_STRIP_TRAILING_WHITESPACE)

SET(PROJECT_VERSION ${PROJECT_NAME}_${ENV_PLATFORM}_${ADM_SDK_VERSION}_${COMPILE_DATE}_${GIT_COMMITID})
ADD_DEFINITIONS(-DADM_ARPC_SDK_VERSION="${PROJECT_VERSION}")
MESSAGE("+++++++++++PROJECT_VERSION = ${PROJECT_VERSION}")
###########################################################SDK VERSION########################################################################

if(CUSTOM_CMAKE_SYSTEM_NAME)
else()
    set(CUSTOM_CMAKE_SYSTEM_NAME Linux)

    # base library(foundation and vrpc) type: ON: static; OFF: shared
    set(CUSTOM_IMPORT_BASE_FULL_STATIC_LIB OFF)

    set(CUSTOM_ADM_COM_ROOT_DIR "/data/admnv/")


endif()

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -std=gnu11 -Wl,-Map,${CMAKE_BINARY_DIR}/output.map")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -std=c++11 -Wl,-Map,${CMAKE_BINARY_DIR}/output.map")

if (LOG)
    set(LOG_LEVEL 0)
    message(STATUS "LOG:${LOG}" ";LOG_LEVEL:${LOG_LEVEL}")
else ()
    set(LOG_LEVEL 1)
    message(STATUS "LOG:${LOG}" ";LOG_LEVEL:${LOG_LEVEL}")
endif ()
set(FOUNDATION_LIB_PATH ${CMAKE_SOURCE_DIR}/../foundation_lib/${ENV_PLATFORM}/packages/libs)

message("-- Foundation lib path ++++++++++" ${FOUNDATION_LIB_PATH})

set(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR}/build/deploy)
set(CMAKE_INSTALL_LIBDIR ${CMAKE_INSTALL_PREFIX}/lib)
set(CMAKE_INSTALL_INCLUDEDIR ${CMAKE_INSTALL_PREFIX}/include)
set(CMAKE_BUILD_TYPE Debug)

set(EXTERNAL_LIB_BINARY_DIR ${CMAKE_BINARY_DIR}/externalLib/lib)
file(MAKE_DIRECTORY ${EXTERNAL_LIB_BINARY_DIR})

set(LIB_SHARED_SUPPORT ON)

if(${CUSTOM_CMAKE_SYSTEM_NAME} MATCHES "Linux")
    if(${ENV_PLATFORM} MATCHES "SVW_GP" OR  ${ENV_PLATFORM} MATCHES "arma7")
        set(SYSTEM_LIBRARY pthread rt dl stdc++)
    else()
        set(SYSTEM_LIBRARY pthread rt dl z c stdc++)
    endif()
elseif(${CUSTOM_CMAKE_SYSTEM_NAME} MATCHES "Android")
    set(SYSTEM_LIBRARY c stdc++ m)
elseif(${CUSTOM_CMAKE_SYSTEM_NAME} MATCHES "Alios")
    set(SYSTEM_LIBRARY c)
elseif(${CUSTOM_CMAKE_SYSTEM_NAME} MATCHES "QNX7")
    set(SYSTEM_LIBRARY c c++ m socket)
else()
    set(SYSTEM_LIBRARY pthread rt z stdc++)
endif()


message("-- custom_import_base_full_static_lib ++++++++++" ${CUSTOM_IMPORT_BASE_FULL_STATIC_LIB})
if(CUSTOM_IMPORT_BASE_FULL_STATIC_LIB)
    set(FOUNDATION_LIBRARY

    ${FOUNDATION_LIB_PATH}/libadm_minizip.a
    ${FOUNDATION_LIB_PATH}/libadmdebug.a
    ${FOUNDATION_LIB_PATH}/libz.a
    ${FOUNDATION_LIB_PATH}/libadmsbuf.a
    ${FOUNDATION_LIB_PATH}/libjson-c.a
    ${FOUNDATION_LIB_PATH}/libadmlog.a
    ${FOUNDATION_LIB_PATH}/libadmvector.a
    ${FOUNDATION_LIB_PATH}/libadmmap.a
    ${FOUNDATION_LIB_PATH}/libadmlist.a
    ${FOUNDATION_LIB_PATH}/libadmqueue.a
    ${FOUNDATION_LIB_PATH}/libadmworkqueue.a
	${FOUNDATION_LIB_PATH}/libadmos.a
	${FOUNDATION_LIB_PATH}/libadmlib.a
	${FOUNDATION_LIB_PATH}/libadminiparser.a
	${FOUNDATION_LIB_PATH}/libiniparser.a
	${FOUNDATION_LIB_PATH}/libadmkvstorage.a
	${FOUNDATION_LIB_PATH}/libleveldb.a
    )
else()
    #rpc依赖foundation动态库
    set(FOUNDATION_LIBRARY
        ${FOUNDATION_LIB_PATH}/libfoundation.so
    )
endif()

add_subdirectory(arpc)

