cmake_minimum_required (VERSION 3.10)
project(dimTest C CXX)

set(project_name "dimTest")

set(CUSTOM_HOST_SUPPORT ON) 
set(CUSTOM_TRANS_SUPPORT ON)
set(CUSTOM_UCL_SUPPORT ON)
set(CUSTOM_VDI_SUPPORT ON)
set(CUSTOM_DPO_SUPPORT ON)

if(CUSTOM_HOST_SUPPORT) 
	ADD_DEFINITIONS(-D__CUSTOM_HOST__)  
endif() 


if(CUSTOM_TRANS_SUPPORT) 
	ADD_DEFINITIONS(-D__CUSTOM_TRANS__) 
endif() 


if(CUSTOM_UCL_SUPPORT) 
	ADD_DEFINITIONS(-D__CUSTOM_UCL__) 
endif() 


if(CUSTOM_VDI_SUPPORT) 
	ADD_DEFINITIONS(-D__CUSTOM_VDI__) 
endif() 

if(CUSTOM_DPO_SUPPORT) 
	ADD_DEFINITIONS(-D__CUSTOM_DPO__) 
endif() 

if (SUPPORT_LION_PKI)
    ADD_DEFINITIONS(-D SUPPORT_LION_PKI)

    if(${CUSTOM_CMAKE_SYSTEM_NAME} MATCHES "Linux")
        set(LION_LIBSO 
        ${CMAKE_SOURCE_DIR}/oca/vendor/lib/libscm.so
        ${CMAKE_SOURCE_DIR}/oca/vendor/lib/libmosquitto.so.1)
    else()
        set(LION_LIBSO 
        ${CMAKE_SOURCE_DIR}/oca/vendor/lib/libscm.so)
    endif() 
else()
    set(LION_LIBSO "")
endif()

add_executable(
    ${project_name}
    adm_dim_simulator_main.c
    adm_dim_dpo_simulator.c
    adm_dim_host_simulator.c
    adm_dim_transHost_simulator.c
    adm_dim_ucl_simulator.c
    adm_dim_vdi_simulator.c
    
    adm_dim_calHash.c
)

target_include_directories(
    ${project_name}
    PRIVATE
    
    ${CMAKE_CURRENT_LIST_DIR}/inc/public
    ${CMAKE_SOURCE_DIR}/application
    ${CMAKE_SOURCE_DIR}/foundation/packages
    ${CMAKE_SOURCE_DIR}/foundation/platform/os   
	${CMAKE_SOURCE_DIR}/foundation/platform/libs/inc
	${CMAKE_SOURCE_DIR}/foundation/packages/log
	${CMAKE_SOURCE_DIR}/foundation/packages/dlt

	${CMAKE_SOURCE_DIR}/dim/inc/public
	${CMAKE_SOURCE_DIR}/dim/src

    ${CMAKE_SOURCE_DIR}/engine/common/errorCode/inc
    ${CMAKE_SOURCE_DIR}/engine/common/config/inc
    ${CMAKE_SOURCE_DIR}/engine/common/lib/inc
    ${CMAKE_SOURCE_DIR}/engine/config/inc

    ${CMAKE_SOURCE_DIR}/soa/inc/public	
    ${CMAKE_SOURCE_DIR}/oca/inc
    ${CMAKE_SOURCE_DIR}/oca/src
    ${CMAKE_SOURCE_DIR}/oca/src/http
    ${CMAKE_SOURCE_DIR}/oca/vendor/include
    ${CMAKE_SOURCE_DIR}/foundation/tools/tracer/inc
)

target_link_libraries(
    ${PROJECT_NAME}
    ${SSL_LIBRARY}
    ${OEM_API_LIBS}

    ${DIM_LIBRARY}
    ${SOA_LIBRARY}
    ${CMAKE_SOURCE_DIR}/build/oca/libadmoca.so
    ${LION_LIBSO}
    
    ${DTS_TOOL_LIB}
    ${CMAKE_SOURCE_DIR}/engine/lib/libconfig.a
    ${CMAKE_SOURCE_DIR}/engine/lib/libcore_service.a
    ${FOUNDATION_LIB_PATH}/libfoundation.so
    ${SYSTEM_LIBRARY}
)
