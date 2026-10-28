cmake_minimum_required(VERSION 3.10)
project(ARPC CXX C)

set(project_name "arpc")

add_definitions(-fPIC)

option(USE_OPENSSL "Force select OpenSSL" ON)
option(USE_MBEDTLS "Force select MbedTLS(PolarSSL)" OFF)


#foundation链接ssl arpc链接foundation
#if(USE_OPENSSL OR USE_MBEDTLS OR USE_SYSTEM_SSL)
#    add_definitions(-DARPC_SSL)
#    if(USE_OPENSSL)
#        set(SSL_NAME "OpenSSL")
#        set(FOUNDATION_LIBRARY ${FOUNDATION_LIBRARY} ${FOUNDATION_LIB_PATH}/libcrypto-static.a ${FOUNDATION_LIB_PATH}/libssl-static.a)
#        message("Force select OpenSSL")
#    elseif(USE_MBEDTLS)
#        set(SSL_NAME "MbedTLS(PolarSSL)")
#        set(FOUNDATION_LIBRARY ${FOUNDATION_LIBRARY} ${FOUNDATION_LIB_PATH}/libmbedcrypto.a ${FOUNDATION_LIB_PATH}/libmbedtls.a #${FOUNDATION_LIB_PATH}/libmbedx509.a)
#        message("Force select MbedTLS(PolarSSL)")
#    elseif(USE_SYSTEM_SSL)
#        set(FOUNDATION_LIBRARY ${FOUNDATION_LIBRARY} crypto ssl)
#        message("Force select System OpenSSL")
#    endif()
#endif()

add_library(${project_name}
    SHARED
    #STATIC
    src/adm_arpc_threadCond.c
    src/adm_arpc_server.c
    src/adm_arpc_eventLoop.c
    src/adm_arpc_net.c
    src/adm_arpc_buffer.c
    src/adm_arpc_select.c
    src/adm_arpc_protocol.c
    src/adm_arpc_service.c
    src/adm_arpc_threadPool.c
    src/adm_arpc_ssl.c
)

target_include_directories(
    ${project_name}
    PRIVATE

    ${CMAKE_SOURCE_DIR}/arpc/inc/public
    ${CMAKE_SOURCE_DIR}/arpc/src
    
    ${CMAKE_SOURCE_DIR}/../engine_lib/config/inc
    ${CMAKE_SOURCE_DIR}/../engine_lib/common/errorCode/inc
    ${CMAKE_SOURCE_DIR}/../engine_lib/common/config/inc
    ${CMAKE_SOURCE_DIR}/../engine_lib/common/lib/inc
    ${CMAKE_SOURCE_DIR}/../engine_lib/common/ssladapter/inc

    ${CMAKE_SOURCE_DIR}/../foundation_lib/${ENV_PLATFORM}/platform/libs/inc
    ${CMAKE_SOURCE_DIR}/../foundation_lib/${ENV_PLATFORM}/platform/os
    ${CMAKE_SOURCE_DIR}/../foundation_lib/${ENV_PLATFORM}/packages/log
    ${CMAKE_SOURCE_DIR}/../foundation_lib/${ENV_PLATFORM}/packages/lib
    ${CMAKE_SOURCE_DIR}/../foundation_lib/${ENV_PLATFORM}/packages/libs/inc
    ${CMAKE_SOURCE_DIR}/../foundation_lib/${ENV_PLATFORM}/packages/json-c
    
)

target_link_libraries(
    ${project_name}
    PRIVATE

    #-Wl,--whole-archive
    ${CMAKE_SOURCE_DIR}/../engine_lib/lib/libconfig.a
    ${CMAKE_SOURCE_DIR}/../engine_lib/lib/libcommon.a
    ${SYSTEM_LIBRARY}
    ${FOUNDATION_LIBRARY}
    #-Wl,--no-whole-archive
)

add_library(
    arpc-static
    STATIC
    src/adm_arpc_threadCond.c
    src/adm_arpc_server.c
    src/adm_arpc_eventLoop.c
    src/adm_arpc_net.c
    src/adm_arpc_buffer.c
    src/adm_arpc_select.c
    src/adm_arpc_protocol.c
    src/adm_arpc_service.c
    src/adm_arpc_threadPool.c
    src/adm_arpc_ssl.c
)

target_include_directories(
    arpc-static
    PRIVATE

    ${CMAKE_SOURCE_DIR}/arpc/inc/public
    ${CMAKE_SOURCE_DIR}/arpc/src
    
    ${CMAKE_SOURCE_DIR}/../engine_lib/config/inc
    ${CMAKE_SOURCE_DIR}/../engine_lib/common/errorCode/inc
    ${CMAKE_SOURCE_DIR}/../engine_lib/common/config/inc
    ${CMAKE_SOURCE_DIR}/../engine_lib/common/lib/inc
    ${CMAKE_SOURCE_DIR}/../engine_lib/common/ssladapter/inc

    ${CMAKE_SOURCE_DIR}/../foundation_lib/${ENV_PLATFORM}/platform/libs/inc
    ${CMAKE_SOURCE_DIR}/../foundation_lib/${ENV_PLATFORM}/platform/os
    ${CMAKE_SOURCE_DIR}/../foundation_lib/${ENV_PLATFORM}/packages/log
    ${CMAKE_SOURCE_DIR}/../foundation_lib/${ENV_PLATFORM}/packages/lib
    ${CMAKE_SOURCE_DIR}/../foundation_lib/${ENV_PLATFORM}/packages/libs/inc
    ${CMAKE_SOURCE_DIR}/../foundation_lib/${ENV_PLATFORM}/packages/json-c
)

target_link_libraries(
    arpc-static
    PRIVATE

    ${SYSTEM_LIBRARY}
    ${CMAKE_SOURCE_DIR}/../engine_lib/lib/libconfig.a
    ${CMAKE_SOURCE_DIR}/../engine_lib/lib/libcommon.a
    ${FOUNDATION_LIBRARY}
)

#add_subdirectory(sample)

