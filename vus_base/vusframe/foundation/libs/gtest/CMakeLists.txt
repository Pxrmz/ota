cmake_minimum_required(VERSION 3.13)
PROJECT(GTEST)
find_package(mosquitto)
find_package(vsomeip)
find_package(json)
find_package(gtest)

set(CMAKE_INSTALL_BINDIR ${CMAKE_INSTALL_PREFIX}/bin)
set(CMAKE_INSTALL_LIBDIR ${CMAKE_INSTALL_PREFIX}/lib)
set(CMAKE_INSTALL_INCLUDEDIR ${CMAKE_INSTALL_PREFIX}/include)

add_executable(
    gtestrpcclient
    soip/adm_soip_rpcClient_gtest.cpp
)

target_include_directories(gtestrpcclient
    PRIVATE
    ${CMAKE_SOURCE_DIR}/libs/src/soip/inc/public
    ${CMAKE_INSTALL_INCLUDEDIR}
)

target_link_libraries (gtestrpcclient
    -Wl,--whole-archive
    ${CMAKE_INSTALL_LIBDIR}/libmosquitto_static.a
    ${CMAKE_INSTALL_LIBDIR}/libjson-c.a
    ${CMAKE_INSTALL_LIBDIR}/libvsomeip3.a  
    ${CMAKE_INSTALL_LIBDIR}/libgtestd.a
    ${CMAKE_INSTALL_LIBDIR}/libgtest_maind.a
    ${CMAKE_SOURCE_DIR}/build/libs/src/soip/libadmrpc.a
    -Wl,--no-whole-archive
    #/usr/lib/gcc/x86_64-linux-gnu/9/libstdc++.so
    stdc++
    pthread
)

add_dependencies(gtestrpcclient json gtest mosquitto vsomeip)

add_executable(
    admvectortest
    adm_vector_test.c
)

target_include_directories(admvectortest
    PRIVATE
    ${CMAKE_SOURCE_DIR}/libs/inc
    ${CMAKE_INSTALL_INCLUDEDIR}
)

target_link_libraries (admvectortest
    ${CMAKE_SOURCE_DIR}/build/libs/libadmvector.a
    pthread
)

add_executable(
    admmaptest
    adm_map_test.c
)

target_include_directories(admmaptest
    PRIVATE
    ${CMAKE_SOURCE_DIR}/libs/inc
)

target_link_libraries (admmaptest
    ${CMAKE_SOURCE_DIR}/build/libs/libadmmap.a
    pthread
)

add_executable(
    admqueuetest
    adm_queue_test.c
)

target_include_directories(admqueuetest
    PRIVATE
    ${CMAKE_SOURCE_DIR}/libs/inc
)

target_link_libraries (admqueuetest
    ${CMAKE_SOURCE_DIR}/build/libs/libadmqueue.a
    pthread
)

add_executable(
    admprototest
    protobuf/readDiagnosticInfo.pb.cc
    protobuf/adm_protobuf_test.cpp
)

target_include_directories(admprototest
    PRIVATE
    ./protobuf
    ${CMAKE_INSTALL_INCLUDEDIR}
)

target_link_libraries (admprototest
    ${CMAKE_SOURCE_DIR}/build/libs/libfoundation.so
)

