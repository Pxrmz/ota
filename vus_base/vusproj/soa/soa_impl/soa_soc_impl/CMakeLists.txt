cmake_minimum_required (VERSION 3.10)

project(SOA_SOC_IMPL CXX C)

set(project_name "soasoc")

add_compile_options(-fPIC)

set(
	SOA_SRC
    src/adm_dis_soa_soc.cpp
)

if(CUSTOME_USE_DLSYM) 
	add_library(${project_name}
		SHARED
		${SOA_SRC}
	)
else()
	add_library(${project_name}
		STATIC
		${SOA_SRC}
	)
endif() 

message(${CMAKE_CURRENT_LIST_DIR})

target_include_directories(
    ${project_name}
    PRIVATE
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
    ${CMAKE_SOURCE_DIR}/foundation/arpc/inc
    ${CMAKE_SOURCE_DIR}/engine/config/inc
)

if(CUSTOME_USE_DLSYM) 
	set(RPC_LIBRARY  ${CMAKE_SOURCE_DIR}/foundation/arpc/lib/libarpc.so)
else()
	set(RPC_LIBRARY  ${CMAKE_SOURCE_DIR}/foundation/arpc/lib/libarpc-static.a)
endif() 

target_link_libraries(
	${project_name}
	    PRIVATE
	    ${FOUNDATION_LIBRARY}
		${CMAKE_SOURCE_DIR}/engine/lib/libcore_service.a
		${CMAKE_SOURCE_DIR}/engine/lib/libconfig.a
		${RPC_LIBRARY}
)
