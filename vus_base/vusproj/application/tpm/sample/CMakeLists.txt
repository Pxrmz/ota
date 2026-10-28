cmake_minimum_required(VERSION 3.10)
project(TPM CXX C)

set(project_name "admtpmsample")

add_executable(${project_name}
    adm_tpm_triggerTransSample.c
)

target_include_directories(
    ${project_name}
    PRIVATE
    ${CMAKE_SOURCE_DIR}/application/tpm/src/
    ${CMAKE_SOURCE_DIR}/application/tpm/inc/public
    ${CMAKE_SOURCE_DIR}/dim/inc/public
    ${CMAKE_SOURCE_DIR}/engine/config/inc/
    ${CMAKE_SOURCE_DIR}/engine
    ${CMAKE_SOURCE_DIR}/engine/common/lib/inc/
    ${CMAKE_SOURCE_DIR}/engine/common/config/inc/
    ${CMAKE_SOURCE_DIR}/engine/cds/inc/public
    ${CMAKE_SOURCE_DIR}/engine/tps/inc/public
    # ${CMAKE_SOURCE_DIR}/engine/dpo/inc/public
    ${CMAKE_SOURCE_DIR}/engine/common/errorCode/inc
    ${CMAKE_SOURCE_DIR}/soa/inc/public
    ${CMAKE_SOURCE_DIR}/soa/src
    ${CMAKE_SOURCE_DIR}/foundation/platform/libs/inc
    ${CMAKE_SOURCE_DIR}/foundation/platform/os
    ${CMAKE_SOURCE_DIR}/foundation/packages/log
    ${CMAKE_SOURCE_DIR}/foundation/packages
    ${CMAKE_SOURCE_DIR}/vrpc/vsomeip/interface 
    ${CMAKE_SOURCE_DIR}/vrpc/sdk/include
    ${CMAKE_SOURCE_DIR}/vrpc
    ${CMAKE_SOURCE_DIR}/tools/tracer/inc
)

target_link_libraries(
    ${project_name}
    PRIVATE
    ${CMAKE_SOURCE_DIR}/build/application/tpm/libdpm.a
    ${CMAKE_SOURCE_DIR}/build/application/tpm/libtpm.a
    ${CMAKE_SOURCE_DIR}/build/dim/libDim.a
    ${CMAKE_SOURCE_DIR}/build/application/cdm/libcdm.a
    ${CMAKE_SOURCE_DIR}/build/application/vrm/libvrm.a
    ${CMAKE_SOURCE_DIR}/build/externalLib/lib/libcore_service.a
    ${CMAKE_SOURCE_DIR}/build/soa/libsoa.a
	${CMAKE_SOURCE_DIR}/build/externalLib/lib/libconfig.a
    ${CMAKE_SOURCE_DIR}/build/oca/libadmoca.so
    ${CMAKE_SOURCE_DIR}/foundation/ewebserver/libs/libewebsrvapi-static.a
    ${FOUNDATION_LIB_PATH}/libfoundation.so
    ${DTS_TOOL_LIB}
    ${SSL_LIBRARY}
    ${SYSTEM_LIBRARY}
)