cmake_minimum_required(VERSION 3.10)
project(CVS CXX C)

set(project_name "irmsample")

add_executable(${project_name}
    adm_irm_sample.c
)

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

target_include_directories(
    ${project_name}
    PRIVATE
    ${CMAKE_SOURCE_DIR}/engine/config/inc/
    ${CMAKE_SOURCE_DIR}/engine/common/config/inc/
    ${CMAKE_SOURCE_DIR}/application/cvm/inc/public
    ${CMAKE_SOURCE_DIR}/application/ocm/inc/
    ${CMAKE_SOURCE_DIR}/foundation/platform/os
    ${CMAKE_SOURCE_DIR}/foundation/packages/log
    ${CMAKE_SOURCE_DIR}/foundation/packages
    ${CMAKE_SOURCE_DIR}/engine/common/errorCode/inc
    ${CMAKE_SOURCE_DIR}/engine/
    ${CMAKE_SOURCE_DIR}/engine/ecs/inc/public
    ${CMAKE_SOURCE_DIR}/engine/irs/inc/public
    ${CMAKE_SOURCE_DIR}/engine/common/lib/inc
    ${CMAKE_SOURCE_DIR}/soa/src
    ${CMAKE_SOURCE_DIR}/soa/inc/public
    ${CMAKE_SOURCE_DIR}/application/irm/inc/public
    ${CMAKE_SOURCE_DIR}/application
    ${CMAKE_SOURCE_DIR}/dim/inc/public
    ${CMAKE_SOURCE_DIR}/uim/inc/public
    ${CMAKE_SOURCE_DIR}/engine/cvs/inc/public
    ${CMAKE_SOURCE_DIR}/engine/ocs/inc/public
    ${CMAKE_SOURCE_DIR}/engine
    ${CMAKE_SOURCE_DIR}/oca/inc/
    ${CMAKE_SOURCE_DIR}/oca/vendor/include/
)

target_link_libraries(
    ${project_name}
    PRIVATE
    ${CMAKE_SOURCE_DIR}/build/application/ocm/libocm.a
    ${CMAKE_SOURCE_DIR}/build/application/irm/libirm.a
    ${CMAKE_SOURCE_DIR}/engine/lib/libcore_service.a
	${CMAKE_SOURCE_DIR}/engine/lib/libconfig.a
    ${FOUNDATION_LIB_PATH}/libfoundation.so
    ${CMAKE_SOURCE_DIR}/build/oca/libadmoca.so
    ${LION_LIBSO}
    ${SYSTEM_LIBRARY}
)

