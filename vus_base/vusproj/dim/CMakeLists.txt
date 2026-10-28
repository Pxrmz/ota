cmake_minimum_required (VERSION 3.10)
project(dim CXX C)

add_definitions(-fPIC)

#add_subdirectory(dimtool/dim_lua_3th/lvm)
#add_subdirectory(dimtool/dim_lua_3th/lua_cjson)
add_subdirectory(dis)
#add_subdirectory(dimtool/dis_lvm)
#add_subdirectory(dimtool/dim_lvm)


set(project_name "dim")

set(DIM_SRC 
    src/adm_dim_api.c
    src/adm_dim_vdi.c
    src/adm_dim_vdiSpy.c
    src/adm_dim_ucl.c
    src/adm_dim_host.c
    src/adm_dim_transHost.c
    src/adm_dim_dpo.c
)

if (SUPPORT_LION_PKI)
    ADD_DEFINITIONS(-D SUPPORT_LION_PKI)
endif()

add_library(${project_name}
	STATIC
	${DIM_SRC}
)

target_include_directories(
    ${project_name}
    PRIVATE
	 
    ${CMAKE_SOURCE_DIR}/application
    ${CMAKE_SOURCE_DIR}/foundation/packages
    ${CMAKE_SOURCE_DIR}/foundation/platform/os   
	${CMAKE_SOURCE_DIR}/foundation/platform/libs/inc
    ${CMAKE_SOURCE_DIR}/foundation/packages/lib/base64
	${CMAKE_SOURCE_DIR}/foundation/packages/log
	${CMAKE_SOURCE_DIR}/foundation/packages/dlt

    ${CMAKE_SOURCE_DIR}/engine/common/errorCode/inc
    ${CMAKE_SOURCE_DIR}/engine/common/config/inc
    ${CMAKE_SOURCE_DIR}/engine/common/lib/inc

    ${CMAKE_SOURCE_DIR}/soa/inc/public
    ${CMAKE_CURRENT_LIST_DIR}/inc/public

	${CMAKE_SOURCE_DIR}/engine/config/inc
    ${CMAKE_SOURCE_DIR}/foundation/tools/tracer/inc

    ${CMAKE_SOURCE_DIR}/dim/dis/dis_public/inc
    ${CMAKE_SOURCE_DIR}/oca/inc
    ${CMAKE_SOURCE_DIR}/oca/src
    ${CMAKE_SOURCE_DIR}/oca/src/http
    ${CMAKE_SOURCE_DIR}/oca/vendor/include
)

target_link_libraries(
	${project_name}
	    PRIVATE
	    ${FOUNDATION_LIB_PATH}/libfoundation.so
		${CMAKE_SOURCE_DIR}/build/externalLib/lib/${TARGET_PLATFORM}/libcore_service.a
		${DTS_TOOL_LIB}
		${SYSTEM_LIBRARY}
)

#add_subdirectory(sample)
