cmake_minimum_required (VERSION 3.0)
project(UIM C CXX)

set(project_name "uim")
SET(CMAKE_BUILD_TYPE "Debug")

add_definitions(-fPIC)

set(
	UIM_SRC
    ${CMAKE_SOURCE_DIR}/uim/src/adm_uim_apiImpl.cpp
    ${CMAKE_SOURCE_DIR}/uim/src/adm_uim_check.cpp
    ${CMAKE_SOURCE_DIR}/uim/src/adm_uim_download.cpp
    ${CMAKE_SOURCE_DIR}/uim/src/adm_uim_install.cpp
    ${CMAKE_SOURCE_DIR}/uim/src/adm_uim_services.cpp
    ${CMAKE_SOURCE_DIR}/uim/src/adm_uim_common.c
    ${CMAKE_SOURCE_DIR}/uim/src/adm_uim_request.cpp
    ${CMAKE_SOURCE_DIR}/uim/src/adm_uim_general.cpp
    ${CMAKE_SOURCE_DIR}/uim/src/adm_uim_ems.cpp
    ${CMAKE_SOURCE_DIR}/uim/src/adm_uim_icp.cpp
)


add_library(
    ${project_name}
    ${UIM_SRC}
)

target_include_directories(
    ${project_name}
    PRIVATE
    ${CMAKE_SOURCE_DIR}/application 
    ${CMAKE_SOURCE_DIR}/foundation/packages
    ${CMAKE_SOURCE_DIR}/foundation/platform/os 
    ${CMAKE_SOURCE_DIR}/foundation/platform/libs/inc
	${CMAKE_SOURCE_DIR}/foundation/packages/log
	${CMAKE_SOURCE_DIR}/foundation/packages/dlt
	
    ${CMAKE_SOURCE_DIR}/engine/common/errorCode/inc
    ${CMAKE_SOURCE_DIR}/engine/common/config/inc
    ${CMAKE_SOURCE_DIR}/engine/common/lib/inc
    ${CMAKE_SOURCE_DIR}/soa/inc/public
    ${CMAKE_CURRENT_LIST_DIR}/inc/public
    ${CMAKE_SOURCE_DIR}/uim/uis/uis_public/inc    

	${CMAKE_SOURCE_DIR}/engine/config/inc 
	${CMAKE_SOURCE_DIR}/foundation/tools/tracer/inc
	${CMAKE_SOURCE_DIR}/foundation/packages/json-c
    ${CMAKE_SOURCE_DIR}/foundation/arpc/inc
)

target_link_libraries( 
	${project_name} 
		PRIVATE
	    ${FOUNDATION_LIB_PATH}/libfoundation.so 
	    ${CMAKE_SOURCE_DIR}/build/externalLib/lib/${TARGET_PLATFORM}/libcore_service.a 
	    ${DTS_TOOL_LIB}
		${SYSTEM_LIBRARY}
		dl 
)

add_subdirectory(uis)
#add_subdirectory(sample) 


# 设置Doxygen变量用于生成System interface service user manual
set(DOXYGEN_GENERATE_HTML YES)
set(DOXYGEN_GENERATE_MAN NO)

set(DOXYGEN_PROJECT_NAME "VUS UIM API user manual")
set(DOXYGEN_PROJECT_NUMBER "v0.0.1")
set(DOXYGEN_PROJECT_BRIEF "UIM应用接口用户手册")
set(DOXYGEN_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build/application/uim/UIM_API_user_manual)
set(DOXYGEN_INDEX_FILE ${DOXYGEN_OUTPUT_DIRECTORY}/index.html)

# Interface User Manual
doxygen_add_docs(
	UIM_API_user_manual
	${CMAKE_SOURCE_DIR}/foundation/platform/os/adm_typedefs.h
	${CMAKE_SOURCE_DIR}/uim/inc/public/adm_uim_check.h
	${CMAKE_SOURCE_DIR}/uim/inc/public/adm_uim_common.h
	${CMAKE_SOURCE_DIR}/uim/inc/public/adm_uim_download.h
	${CMAKE_SOURCE_DIR}/uim/inc/public/adm_uim_install.h
	${CMAKE_SOURCE_DIR}/uim/inc/public/adm_uim_general.h
	COMMENT "VUS user interaction manager api user manual generate"
	)

