cmake_minimum_required(VERSION 3.10)
project(CVS CXX C)

set(project_name "admdpmsample")

add_executable(${project_name}
    adm_dpm_triggerDownSample.c
)

target_include_directories(
    ${project_name}
    PRIVATE
    ${CMAKE_SOURCE_DIR}/application/dpm/src/
    ${CMAKE_SOURCE_DIR}/application/dpm/inc/public
    ${CMAKE_SOURCE_DIR}/uim/inc/public
    ${CMAKE_SOURCE_DIR}/dim/inc/public
    ${CMAKE_SOURCE_DIR}/engine/config/inc/
    ${CMAKE_SOURCE_DIR}/engine
    ${CMAKE_SOURCE_DIR}/engine/common/lib/inc/
    ${CMAKE_SOURCE_DIR}/engine/common/config/inc/
    ${CMAKE_SOURCE_DIR}/engine/cds/inc/public
    ${CMAKE_SOURCE_DIR}/engine/dps/inc/public
    ${CMAKE_SOURCE_DIR}/engine/dpo/inc/public
    ${CMAKE_SOURCE_DIR}/engine/common/errorCode/inc
    ${CMAKE_SOURCE_DIR}/soa/inc/public
    ${CMAKE_SOURCE_DIR}/soa/src
    ${CMAKE_SOURCE_DIR}/vrpc/vsomeip/interface 
    ${CMAKE_SOURCE_DIR}/vrpc/sdk/include
    ${CMAKE_SOURCE_DIR}/vrpc
    ${CMAKE_SOURCE_DIR}/foundation/platform/libs/inc
    ${CMAKE_SOURCE_DIR}/foundation/platform/os
    ${CMAKE_SOURCE_DIR}/foundation/packages/log
    ${CMAKE_SOURCE_DIR}/foundation/packages
)

target_link_libraries(
    ${project_name}
    PRIVATE
    ${CMAKE_SOURCE_DIR}/build/application/dpm/libdpm.a
    ${CMAKE_SOURCE_DIR}/build/application/cdm/libcdm.a
    ${CMAKE_SOURCE_DIR}/build/application/vsm/libvsm.a
    ${CMAKE_SOURCE_DIR}/build/application/vrm/libvrm.a
    #${CMAKE_SOURCE_DIR}/build/dim/libdim.a
    #${CMAKE_SOURCE_DIR}/build/uim/libUIM.a
    ${CMAKE_SOURCE_DIR}/build/externalLib/lib/libcore_service.a
	${CMAKE_SOURCE_DIR}/build/externalLib/lib/libconfig.a
    ${CMAKE_SOURCE_DIR}/build/oca/libadmoca.so
    ${FOUNDATION_LIB_PATH}/libfoundation.so
    ${SSL_LIBRARY}
    ${SYSTEM_LIBRARY}

	${SOA_LIBRARY}
    ${DIM_LIBRARY}
    ${UIM_LIBRARY}
)