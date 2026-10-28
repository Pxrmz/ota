cmake_minimum_required(VERSION 3.10)
project(CORE C CXX)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -rdynamic -funwind-tables -ffunction-sections -std=gnu11 -Wl,-Map,${CMAKE_BINARY_DIR}/output.map")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -rdynamic -funwind-tables -ffunction-sections -std=c++11 -Wl,-Map,${CMAKE_BINARY_DIR}/output.map")

###########################################################SDK VERSION########################################################################

# SET(MAJOR_VERSION 1)
# SET(MINOR_VERSION 1)
# SET(HOTFIX_VERSION 1)
SET(PROJECT_NAME "engine")
message("SDK_VERSION = ${SDK_VERSION}")
SET(ADM_SDK_VERSION ${SDK_VERSION})

EXECUTE_PROCESS(COMMAND date +%Y%m%d OUTPUT_VARIABLE COMPILE_DATE OUTPUT_STRIP_TRAILING_WHITESPACE)
EXECUTE_PROCESS(COMMAND git rev-parse --short HEAD OUTPUT_VARIABLE GIT_COMMITID OUTPUT_STRIP_TRAILING_WHITESPACE)

SET(PROJECT_VERSION ${PROJECT_NAME}_${ENV_PLATFORM}_${ADM_SDK_VERSION}_${COMPILE_DATE}_${GIT_COMMITID})
ADD_DEFINITIONS(-DADM_ENGINE_SDK_VERSION="${PROJECT_VERSION}")
MESSAGE("+++++++++++PROJECT_VERSION = ${PROJECT_VERSION}")
###########################################################SDK VERSION########################################################################

option(AIR_PROTOCOL_ENCRYPT    "Air protocol encrypt"     OFF)
option(AIR_PROTOCOL_SIGN       "Air protocol sign"  ON)
# ADD_DEFINITIONS(-D ADM_IPS_POLICY_UPGRADE_ORDER_OLD)
ADD_DEFINITIONS(-D UPGRADE_GROUP_POLICY_VERSION=3)
#支持事件优化
set(REPORT_EVENT_OPTIMIZE ON)
if (REPORT_EVENT_OPTIMIZE)
    ADD_DEFINITIONS(-D ADM_REPORT_EVENT_OPTIMIZE)
endif()

#ADD_DEFINITIONS(-D UPGRADE_GROUP_POLICY_VERSION=1)

if (LOG)
    set(LOG_LEVEL 0)
    message(STATUS "LOG:${LOG}" ";LOG_LEVEL:${LOG_LEVEL}")
else ()
    set(LOG_LEVEL 1)
    message(STATUS "LOG:${LOG}" ";LOG_LEVEL:${LOG_LEVEL}")
endif ()

set(FOUNDATION_LIB_PATH ${CMAKE_SOURCE_DIR}/../foundation_lib/${ENV_PLATFORM}/packages/libs)
message("-- Foundation lib path ++++++++++" ${FOUNDATION_LIB_PATH})

message("PLATFORM_NAME = ${PLATFORM_NAME}")

if(${PLATFORM_NAME} MATCHES "Linux")
    set(SYSTEM_LIBRARY pthread rt dl stdc++)
elseif(${PLATFORM_NAME} MATCHES "Android")
    set(SYSTEM_LIBRARY c stdc++ m)
elseif(${PLATFORM_NAME} MATCHES "Alios")
    set(SYSTEM_LIBRARY c)
elseif(${PLATFORM_NAME} MATCHES "QNX7")
    set(SYSTEM_LIBRARY c c++ m socket)
    set(CMAKE_CXX_STANDARD 11)
else()
    set(SYSTEM_LIBRARY pthread rt dl stdc++)
endif()

set(USE_SYSTEM_SSL OFF)
set(USE_MYSELF_SSL_SHARED_LIB OFF)

if(USE_SYSTEM_SSL)
    message("+++++++++++use system ssl USE_SYSTEM_SSL = ${USE_SYSTEM_SSL}")
    set(SSL_LIBRARY crypto ssl)
else()
    message("+++++++++++use project compile ssl USE_SYSTEM_SSL = ${USE_SYSTEM_SSL}")
    
    if(USE_MYSELF_SSL_SHARED_LIB)
        message("+++++++++++use project compile [shared] ssl USE_MYSELF_SSL_SHARED_LIB = ${USE_MYSELF_SSL_SHARED_LIB}")
        set(SSL_LIBRARY ${FOUNDATION_LIB_PATH}/libcrypto.so  ${FOUNDATION_LIB_PATH}/libssl.so)
    else()
       message("+++++++++++use project compile [static] ssl USE_MYSELF_SSL_SHARED_LIB = ${USE_MYSELF_SSL_SHARED_LIB}")
        set(SSL_LIBRARY ${FOUNDATION_LIB_PATH}/libcrypto-static.a ${FOUNDATION_LIB_PATH}/libssl-static.a)
    endif()
endif()

option(SSL_SUPPORT "SSL support" ON)
option(USE_OPENSSL "Force select OpenSSL" ON)
option(USE_MBEDTLS "Force select MbedTLS(PolarSSL)" OFF)

if(USE_OPENSSL OR USE_MBEDTLS)
    set(SSL_SUPPORT ON)

    if(USE_OPENSSL)
        set(SSL_NAME "OpenSSL")
        set(SSL_SRC ${CMAKE_CURRENT_SOURCE_DIR}/engine/common/ssladapter/src/adm_com_sslAdapter.c)
        #set(SSL_INC ${CMAKE_CURRENT_SOURCE_DIR}/engine/common/ssladapter/inc)
        set(SSL_LIBRARY ${FOUNDATION_LIB_PATH}/libcrypto-static.a ${FOUNDATION_LIB_PATH}/libssl-static.a)
        add_definitions(-DUSE_OPENSSL)
        #message("Force select OpenSSL ${SSL_LIBRARY}")
    elseif(USE_MBEDTLS)
        set(SSL_NAME "MbedTLS(PolarSSL)")
        set(SSL_SRC ${CMAKE_CURRENT_SOURCE_DIR}/engine/common/ssladapter/src/adm_com_mbedtls.c)
        #set(SSL_INC ${CMAKE_CURRENT_SOURCE_DIR}/engine/common/ssladapter/inc)
        set(SSL_LIBRARY ${FOUNDATION_LIB_PATH}/libmbedcrypto.a ${FOUNDATION_LIB_PATH}/libmbedtls.a ${FOUNDATION_LIB_PATH}/libmbedx509.a)
        add_definitions(-DUSE_MBEDTLS)
        #message("Force select MbedTLS(PolarSSL) ${SSL_LIBRARY}")
    endif()
endif()

# find Doxygen package
find_package(Doxygen
    REQUIRED dot
    OPTIONAL_COMPONENTS mscgen dia)

#add_subdirectory(foundation)
add_subdirectory(engine)

if(${PLATFORM_NAME} MATCHES "QNX7")
else()
	#add_subdirectory(test)
endif()

##---------------- 以下为用CPack打包成tar.gz包的内容 ----------------##
###########################################################OUT DIR########################################################################
set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/output/engine)
# 下面注释的几行，一定不要放开，会出现版本打包失败的情况
# set(CMAKE_INSTALL_BINDIR ${CMAKE_INSTALL_PREFIX}/bin)
# set(CMAKE_INSTALL_LIBDIR ${CMAKE_INSTALL_PREFIX}/lib)
# set(CMAKE_INSTALL_INCLUDEDIR ${CMAKE_INSTALL_PREFIX})
set(CMAKE_BUILD_TYPE Debug)
###########################################################OUT DIR########################################################################

# set(CPACK_OUTPUT_FILE_PREFIX ${CMAKE_BINARY_DIR}/output/)
# set(CPACK_PACKAGE_DIRECTORY ${CMAKE_BINARY_DIR}/output/engine/)
# message(${CPACK_OUTPUT_FILE_PREFIX}"xxxxxxxxxxxxxxxxxx")
# message(${CPACK_PACKAGE_DIRECTORY}"xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx")

# string(TIMESTAMP COMPILE_TIME %Y%m%d)
# set(CPACK_GENERATOR "TGZ")
# set(PACKAGE_FILE_NAME "${PROJECT_NAME}_${ENV_PLATFORM}_${ADM_SDK_VERSION}_${COMPILE_TIME}_${GIT_COMMITID}")
# set(CPACK_ARCHIVE_COMPONENT_INSTALL ON)
# set(CPACK_PACKAGE_FILE_NAME "${PACKAGE_FILE_NAME}")

# include(CPack)
# ##---------------- 以下为用CPack打包成tat.gz包的内容 ----------------##
