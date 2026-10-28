cmake_minimum_required(VERSION 3.5.0)
project(libs C CXX)

#find_package(gtest)
find_package(json)
find_package(iniparser)
#find_package(vsomeip)
find_package(openssl)
if(NOT SUPPORT_LION_PKI)
	find_package(curl)
endif()
find_package(zlib)
find_package(log)
find_package(leveldb)
#find_package(paho.mqtt)
#find_package(lwm2m)
#if (CUSTOM_CMAKE_XML_NAME)
#find_package(xml)
#endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/inc)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/inc/configToolchain)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/lib)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/lib/crypto/symCrypto)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/lib/crypto/asymCrypto)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/lib/crypto/base64)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/lib/crypto/public)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/base64)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/hmac)
include_directories(${CMAKE_INSTALL_INCLUDEDIR})
include_directories(${CMAKE_INSTALL_INCLUDEDIR}/log)
include_directories(${CMAKE_SOURCE_DIR}/platform/os/inc)

message("+++++++++++###########foundation compile TEST_APK_IF_SUPPORT = ${TEST_APK_IF_SUPPORT}")
if (NOT TEST_APK_IF_SUPPORT EQUAL 0)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} \
    -lm   \
    -Wall \
    -g \
    -O0 \
    -Wno-unused-parameter \
    -std=gnu11\
    -fPIC\
    ")

	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} \
    -lm   \
    -Wall \
    -g \
    -O0 \
    -Wno-unused-parameter \
    -std=c++11\
    -fPIC\
    -fpermissive\
    ")
else()
add_compile_options(
        -rdynamic
        -funwind-tables
        -ffunction-sections
        -Wall
        -g
        -O0
        -fPIC
)
endif()
    
add_library(
    admkeylog
    STATIC
    src/adm_keyLog.c
)

install(TARGETS admkeylog
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/libs)

add_library(
    admlist
    STATIC
    src/adm_list.c
)

install(TARGETS admlist
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/libs)

add_library(
    admvector
    STATIC
    src/adm_vector.c
)

install(TARGETS admvector
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

add_library(
    admsbuf
    STATIC
    src/adm_sbuf.c
)

install(TARGETS admsbuf
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

add_library(
    admmap
    STATIC
    src/map/adm_map.c
    src/map/adm_rbtree.c
)

install(TARGETS admmap
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

add_library(
    foundation_version
    STATIC
    src/version/version.c
)

install(TARGETS foundation_version
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

add_library(
    admqueue
    STATIC
    src/adm_queue.c
)
install(TARGETS admqueue
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

add_library(
    admworkqueue
    STATIC
    src/workqueue/adm_workqueue.c
    src/workqueue/workqueue.c
)
add_dependencies(admworkqueue log admos)

install(TARGETS admworkqueue
    ARCHIVE DESTINATION ${INSTALL_DIR}/lib
    LIBRARY DESTINATION ${INSTALL_DIR}/lib)

add_subdirectory(src/kvstorage)
add_subdirectory(src/iniparser)
add_subdirectory(src/adm_minizip)
#add_subdirectory(gtest)

if(${CUSTOM_CMAKE_SYSTEM_NAME} MATCHES "QNX7")
	set(ASYNC_LIBRARY "")
    set(POLIPO_LIBRARY "")
    set(HTTPPROXY_LIBRARY "")
elseif(${CUSTOM_CMAKE_SYSTEM_NAME} MATCHES "Linux")
    add_subdirectory(src/async)
    add_subdirectory(src/httpproxy)
    set(ASYNC_LIBRARY ${CMAKE_SOURCE_DIR}/build/libs/src/async/libadmasync.a)
    set(POLIPO_LIBRARY ${CMAKE_INSTALL_LIBDIR}/libpolipo.a)
    set(HTTPPROXY_LIBRARY ${CMAKE_SOURCE_DIR}/build/libs/src/httpproxy/libadmhttpproxy.a)
else()
	add_subdirectory(src/async)
    add_subdirectory(src/httpproxy)
	set(ASYNC_LIBRARY ${CMAKE_SOURCE_DIR}/build/libs/src/async/libadmasync.a)
    set(POLIPO_LIBRARY ${CMAKE_INSTALL_LIBDIR}/libpolipo.a)
    set(HTTPPROXY_LIBRARY ${CMAKE_SOURCE_DIR}/build/libs/src/httpproxy/libadmhttpproxy.a)
endif()

add_library(
    admlib
    STATIC
    src/lib/base64/base64.c
    src/lib/hmac/hmac_sha2.c
    src/lib/hmac/sha2.c
    src/lib/atomic.c
    src/lib/glob.c
    src/lib/kref.c
    src/lib/md5.c
    src/lib/simple_string.c
    src/lib/stringprintf.c
    src/lib/crypto/base64/adm_base64.c
    src/lib/crypto/symCrypto/adm_symCrypt.c
    src/lib/crypto/asymCrypto/adm_asymCrypt.c
)
install(TARGETS admlib
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})


#获取配置工具链编译
add_library(
    adm_config_tool
    STATIC
    src/configToolchain/adm_configParse.c
)
install(TARGETS adm_config_tool
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

add_library(
    foundation
    SHARED
    src/adm_getFoundationVersion.c
)

target_include_directories(foundation
    PRIVATE
    ${CMAKE_SOURCE_DIR}/libs/inc
)

message("+++++++++++###########foundation compile CUSTOM_CMAKE_SYSTEM_NAME = ${CUSTOM_CMAKE_SYSTEM_NAME}")

if (NOT TEST_APK_IF_SUPPORT EQUAL 0)
	if(${CUSTOM_CMAKE_SYSTEM_NAME} MATCHES "Linux")
    		set(SYSTEM_LIBRARY pthread rt dl stdc++)
	elseif(${CUSTOM_CMAKE_SYSTEM_NAME} MATCHES "Android")
    		set(SYSTEM_LIBRARY c stdc++ m)
	elseif(${CUSTOM_CMAKE_SYSTEM_NAME} MATCHES "Alios")
    		set(SYSTEM_LIBRARY c)
	elseif(${CUSTOM_CMAKE_SYSTEM_NAME} MATCHES "QNX7")
    		set(SYSTEM_LIBRARY c c++ m socket)
    		set(CMAKE_CXX_STANDARD 11)
	elseif(${CUSTOM_CMAKE_SYSTEM_NAME} MATCHES "ag55x")
    		set(SYSTEM_LIBRARY pthread rt dl stdc++)
	else()
    		set(SYSTEM_LIBRARY pthread rt dl stdc++)
	endif()
else()
	if(${CUSTOM_CMAKE_SYSTEM_NAME} MATCHES "Linux")
    		set(SYSTEM_LIBRARY pthread rt dl)
	elseif(${CUSTOM_CMAKE_SYSTEM_NAME} MATCHES "Android")
    		set(SYSTEM_LIBRARY c)
	elseif(${CUSTOM_CMAKE_SYSTEM_NAME} MATCHES "Alios")
    		set(SYSTEM_LIBRARY c)
	elseif(${CUSTOM_CMAKE_SYSTEM_NAME} MATCHES "QNX7")
    		set(SYSTEM_LIBRARY c)
	else()
    		set(SYSTEM_LIBRARY pthread rt dl)
	endif()
endif()

if(USE_SYSTEM_SSL)
    message("+++++++++++use system ssl USE_SYSTEM_SSL = ${USE_SYSTEM_SSL}")
    set(SSL_LIBRARY crypto ssl)
else()
    message("+++++++++++use project compile ssl USE_SYSTEM_SSL = ${USE_SYSTEM_SSL}")
    if(USE_DYNAMIC_SSL_LIB)
        message("+++++++++++use project compile dynamic ssl USE_DYNAMIC_SSL_LIB = ${USE_DYNAMIC_SSL_LIB}")
        set(SSL_LIBRARY ${CMAKE_INSTALL_LIBDIR}/libcrypto.so  ${CMAKE_INSTALL_LIBDIR}/libssl.so)
    else()
        message("+++++++++++use project compile static ssl USE_DYNAMIC_SSL_LIB = ${USE_DYNAMIC_SSL_LIB}")
        set(SSL_LIBRARY ${CMAKE_INSTALL_LIBDIR}/libcrypto-static.a  ${CMAKE_INSTALL_LIBDIR}/libssl-static.a)
    endif()
endif()

if(NOT SUPPORT_LION_PKI)
	set(CURL_LIB ${CMAKE_INSTALL_LIBDIR}/libcurl-d.a)
	set(CURL_NAME "curl")
else()
	set(CURL_LIB "")
	set(CURL_NAME "")
endif()

target_link_libraries (foundation
    PRIVATE
    -Wl,--whole-archive
    ${SSL_LIBRARY}
    ${CMAKE_INSTALL_LIBDIR}/libz.a
    ${CURL_LIB}
    #    ${CMAKE_INSTALL_LIBDIR}/libcurl-d.a
    ${CMAKE_INSTALL_LIBDIR}/libadmlog.a
    ${CMAKE_INSTALL_LIBDIR}/libadmdebug.a
    ${CMAKE_INSTALL_LIBDIR}/libjson-c.a
    ${CMAKE_INSTALL_LIBDIR}/libiniparser.a
    ${CMAKE_INSTALL_LIBDIR}/libleveldb.a
    ${POLIPO_LIBRARY}

    ${CMAKE_SOURCE_DIR}/build/platform/os/libadmos.a
    ${CMAKE_SOURCE_DIR}/build/libs/libadmkeylog.a  
    ${CMAKE_SOURCE_DIR}/build/libs/libadmlist.a  
    ${CMAKE_SOURCE_DIR}/build/libs/libadmmap.a  
    ${CMAKE_SOURCE_DIR}/build/libs/libfoundation_version.a  
    ${CMAKE_SOURCE_DIR}/build/libs/libadmqueue.a  
    ${CMAKE_SOURCE_DIR}/build/libs/libadmsbuf.a  
    ${CMAKE_SOURCE_DIR}/build/libs/libadmvector.a  
    ${CMAKE_SOURCE_DIR}/build/libs/libadmworkqueue.a
    ${CMAKE_SOURCE_DIR}/build/libs/libadmlib.a
    ${CMAKE_SOURCE_DIR}/build/libs/libadm_config_tool.a
    ${CMAKE_SOURCE_DIR}/build/libs/src/kvstorage/libadmkvstorage.a
    ${CMAKE_SOURCE_DIR}/build/libs/src/iniparser/libadminiparser.a
    ${ASYNC_LIBRARY}
    #    ${CMAKE_SOURCE_DIR}/build/libs/src/async/libadmasync.a
    ${CMAKE_SOURCE_DIR}/build/libs/src/adm_minizip/libadm_minizip.a
    ${HTTPPROXY_LIBRARY}

    ${SYSTEM_LIBRARY}
    -Wl,--no-whole-archive
    # -Wl,-Bsymbolic
)


if(${CUSTOM_CMAKE_SYSTEM_NAME} MATCHES "QNX7")
	add_dependencies(${foundation} json iniparser openssl ${CURL_NAME} log leveldb admkeylog admlist admvector admsbuf admmap foundation_version admqueue admworkqueue admlib adm_config_tool admos admkvstorage adminiparser adm_minizip)
elseif(${CUSTOM_CMAKE_SYSTEM_NAME} MATCHES "Linux")
    add_dependencies(${foundation} json iniparser openssl ${CURL_NAME} log leveldb admkeylog admlist admvector admsbuf admmap foundation_version admqueue admworkqueue admlib adm_config_tool admos admkvstorage adminiparser admasync adm_minizip admhttpproxy)
else()
	add_dependencies(${foundation} json iniparser openssl ${CURL_NAME} log leveldb admkeylog admlist admvector admsbuf admmap foundation_version admqueue admworkqueue admlib adm_config_tool admos admkvstorage adminiparser admasync adm_minizip admhttpproxy)
endif()

#add_dependencies(${foundation} json iniparser openssl curl log leveldb admlist admvector admsbuf admmap foundation_version admqueue admworkqueue admlib adm_config_tool admos admkvstorage adminiparser admasync adm_minizip)
