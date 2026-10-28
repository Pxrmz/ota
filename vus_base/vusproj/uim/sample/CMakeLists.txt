cmake_minimum_required (VERSION 3.0)
project(UIMServiceTest C CXX)

set(project_name "UIMServiceTest")
SET(CMAKE_BUILD_TYPE "Debug")

add_definitions(-fPIC)

add_executable(
    ${project_name}
    ${CMAKE_SOURCE_DIR}/uim/sample/src/UimSoaCheckConstruction.cpp
    ${CMAKE_SOURCE_DIR}/uim/sample/src/UIMSoaDownLoadConstruction.cpp
    ${CMAKE_SOURCE_DIR}/uim/sample/src/UimSoaInstallConstruction.cpp
    ${CMAKE_SOURCE_DIR}/uim/sample/src/UimSoaRequestHMIConstruction.cpp
    ${CMAKE_SOURCE_DIR}/uim/sample/src/UimServiceSample.cpp
)

target_include_directories(
    ${project_name}
    PRIVATE
    ${CMAKE_SOURCE_DIR}/engine/common/config/inc
    ${CMAKE_SOURCE_DIR}/foundation/platform/os
    ${CMAKE_SOURCE_DIR}/foundation/packages/log
    ${CMAKE_SOURCE_DIR}/uim/inc/public
    ${CMAKE_SOURCE_DIR}/uim/src
    ${CMAKE_SOURCE_DIR}/uim/sample/inc
    ${CMAKE_SOURCE_DIR}/application
    ${CMAKE_SOURCE_DIR}/foundation/packages
	${CMAKE_SOURCE_DIR}/engine/common/errorCode/inc

	${CMAKE_SOURCE_DIR}/foundation/platform/libs/inc
    ${CMAKE_SOURCE_DIR}/soa/inc/public
    ${CMAKE_SOURCE_DIR}/soa/src
    ${CMAKE_SOURCE_DIR}/foundation/tools/tracer/inc
    ${CMAKE_SOURCE_DIR}/foundation/arpc/inc
	${CMAKE_SOURCE_DIR}/engine/common/lib/inc
    ${CMAKE_SOURCE_DIR}/engine/config/inc
)

if(CUSTOME_USE_DLSYM)
    set(ARPC_LIBSO ${CMAKE_SOURCE_DIR}/foundation/arpc/lib/libarpc.so)
else()
    set(ARPC_LIBSO "")
endif()

target_link_libraries(
	${project_name}
	#${CMAKE_SOURCE_DIR}/build/uim/libuim.a
	#${CMAKE_SOURCE_DIR}/build/application/cvm/libcvm.a
	
	${CMAKE_SOURCE_DIR}/build/soa/libsoa.a
    ${FOUNDATION_LIB_PATH}/libfoundation.so
    ${SSL_LIBRARY}
    ${SYSTEM_LIBRARY}
    ${UIM_LIBRARY}
    ${SOA_LIBRARY}
    ${ARPC_LIBSO}
    ${CMAKE_SOURCE_DIR}/engine/lib/libcore_service.a
    ${CMAKE_SOURCE_DIR}/engine/lib/libconfig.a
    ${CMAKE_SOURCE_DIR}/engine/lib/libcommon.a
)


