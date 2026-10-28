cmake_minimum_required(VERSION 3.10)
project(soa CXX C)

add_subdirectory(soa_impl)

set(project_name "soa")

set(
		SOA_SRC
    	src/adm_soa.cc
)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} \
    -fPIC\
    -fpermissive\
    ")

add_library(${project_name}
	STATIC
	#SHARED
	${SOA_SRC}
)

target_include_directories(
    ${project_name}
    PRIVATE
    ${CMAKE_SOURCE_DIR}/foundation/arpc/inc
    ${CMAKE_SOURCE_DIR}/foundation/packages/vsomeip
    ${CMAKE_SOURCE_DIR}/foundation/platform/os
    ${CMAKE_SOURCE_DIR}/foundation/platform/libs/inc
    ${CMAKE_SOURCE_DIR}/engine/common/errorCode/inc
    ${CMAKE_SOURCE_DIR}/engine/common/config/inc
    ${CMAKE_SOURCE_DIR}/engine/common/lib/inc
	${CMAKE_SOURCE_DIR}/engine/config/inc/
    ${CMAKE_SOURCE_DIR}/foundation/packages/log
    
    ${CMAKE_CURRENT_LIST_DIR}/inc/public   
    ${CMAKE_CURRENT_LIST_DIR}/soa_impl

    ${CMAKE_SOURCE_DIR}/application
    ${CMAKE_SOURCE_DIR}/foundation/packages
    ${CMAKE_SOURCE_DIR}/foundation/platform/os   
    ${CMAKE_SOURCE_DIR}/foundation/platform/libs/inc
    ${CMAKE_SOURCE_DIR}/engine/common/errorCode/inc
    ${CMAKE_SOURCE_DIR}/engine/common/config/inc
    ${CMAKE_SOURCE_DIR}/engine/config/inc
    ${CMAKE_SOURCE_DIR}/foundation/packages/log
    ${CMAKE_SOURCE_DIR}/foundation/tools/tracer/inc
    ${CMAKE_SOURCE_DIR}/foundation/packages/dlt
    ${CMAKE_SOURCE_DIR}/engine/common/lib/inc
    ${CMAKE_SOURCE_DIR}/application/dim/src
    ${CMAKE_SOURCE_DIR}/application/dim/inc/public
    ${CMAKE_SOURCE_DIR}/application/dis/dis_public/inc
    ${CMAKE_SOURCE_DIR}/application/dis/dis_public/inc/proto
    ${CMAKE_SOURCE_DIR}/soa/inc/public
    ${CMAKE_SOURCE_DIR}/soa/src
    ${CMAKE_SOURCE_DIR}/soa/soa_impl
    ${CMAKE_SOURCE_DIR}/engine/config/inc
)

target_link_libraries(
    ${project_name}
    PRIVATE
	${CMAKE_SOURCE_DIR}/build/externalLib/lib/libconfig.a
	${FOUNDATION_LIBRARY}
	${SYSTEM_LIBRARY}
)

#add_subdirectory(soa_impl)
