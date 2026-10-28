cmake_minimum_required (VERSION 3.10)
#project(Dim CXX C)

set(project_name "uisjsonsoc")

set(
	UIS_SRC
    src/adm_uis_services_json_soc.c
    src/adm_uis_check_json_soc.c
    src/adm_uis_icp_json_soc.c
    src/adm_uis_download_json_soc.c
    src/adm_uis_install_json_soc.c
    src/adm_uis_request_json_soc.c
    src/adm_uis_general_json_soc.c
    src/adm_uis_ems_json_soc.c
    ../uis_public/src/adm_uis_common.c
)

if(CUSTOME_USE_DLSYM) 
add_library(${project_name}
	SHARED
	${UIS_SRC}
)
else() 
add_library(${project_name}
	STATIC
	${UIS_SRC}
)
endif() 

message(${CMAKE_CURRENT_LIST_DIR})

target_include_directories(
    ${project_name}
    PRIVATE
    ${CMAKE_SOURCE_DIR}/application
    
    ${CMAKE_CURRENT_LIST_DIR}/src
    ${CMAKE_SOURCE_DIR}/engine/common/config/inc
    ${CMAKE_SOURCE_DIR}/foundation/packages
    ${CMAKE_SOURCE_DIR}/foundation/platform/os   
    ${CMAKE_SOURCE_DIR}/engine/common/errorCode/inc
    ${CMAKE_SOURCE_DIR}/foundation/platform/libs/inc
    
    ${CMAKE_SOURCE_DIR}/foundation/packages/log
    ${CMAKE_SOURCE_DIR}/tools/tracer/inc
    ${CMAKE_SOURCE_DIR}/foundation/packages/dlt
    ${CMAKE_SOURCE_DIR}/engine/common/lib/inc
    ${CMAKE_SOURCE_DIR}/uim/src
    ${CMAKE_SOURCE_DIR}/uim/inc/public
    ${CMAKE_SOURCE_DIR}/uim/uis/uis_public/inc

    ${CMAKE_SOURCE_DIR}/foundation/arpc/inc
    ${CMAKE_SOURCE_DIR}/config/inc
    ${CMAKE_SOURCE_DIR}/soa/inc/public
    ${CMAKE_SOURCE_DIR}/application
    ${CMAKE_SOURCE_DIR}/foundation/packages/json-c
)

if(CUSTOME_USE_DLSYM) 
	set(RPC_LIBRARY  ${CMAKE_SOURCE_DIR}/foundation/arpc/lib/libarpc.so) 
else() 
	set(RPC_LIBRARY  ${CMAKE_SOURCE_DIR}/foundation/arpc/lib/libarpc-static.a) 
endif() 

target_link_libraries(
	${project_name}
	    PRIVATE
	    ${FOUNDATION_LIB_PATH}
		${CMAKE_SOURCE_DIR}/engine/lib/libcore_service.a
		${RPC_LIBRARY}
		
)
