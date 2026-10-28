cmake_minimum_required(VERSION 3.5.0)
PROJECT(admecmsample)

set(project_name "admecmsample")

add_executable(${project_name}
    adm_ecm_getOtaConfigureSample.c
)

target_include_directories(
    ${project_name}
    PRIVATE
    ${CMAKE_CURRENT_DIR}/
    ${CMAKE_SOURCE_DIR}/engine/config/inc/
    ${CMAKE_SOURCE_DIR}/oca/inc/
    ${CMAKE_SOURCE_DIR}/oca/vendor/include
    ${CMAKE_SOURCE_DIR}/dim/inc/public
    ${CMAKE_SOURCE_DIR}/application
    ${CMAKE_SOURCE_DIR}/application/ocm/inc/
    ${CMAKE_SOURCE_DIR}/application/ecm/src/
    ${CMAKE_SOURCE_DIR}/application/ecm/inc/
    ${CMAKE_SOURCE_DIR}/application/pub/inc
    ${CMAKE_SOURCE_DIR}/application/irm/inc/include
    ${CMAKE_SOURCE_DIR}/engine/
    ${CMAKE_SOURCE_DIR}/engine/ecs/inc/public
    ${CMAKE_SOURCE_DIR}/engine/common/config/inc/
    ${CMAKE_SOURCE_DIR}/engine/common/lib/inc
    ${CMAKE_SOURCE_DIR}/engine/common/config/inc
    ${CMAKE_SOURCE_DIR}/engine/common/errorCode/inc
    ${CMAKE_SOURCE_DIR}/engine/ocs/inc/public

    ${CMAKE_SOURCE_DIR}/foundation/packages
    ${CMAKE_SOURCE_DIR}/foundation/packagescrypt
    ${CMAKE_SOURCE_DIR}/foundation/packages/openssl
    ${CMAKE_SOURCE_DIR}/foundation/packages/curl
    ${CMAKE_SOURCE_DIR}/foundation/packages/json-c
    ${CMAKE_SOURCE_DIR}/foundation/packages/log
    ${CMAKE_SOURCE_DIR}/foundation/packages/lib
    ${CMAKE_SOURCE_DIR}/foundation/packages/lib/hmac
    ${CMAKE_SOURCE_DIR}/foundation/platform/libs/inc/kvstorage
    ${CMAKE_SOURCE_DIR}/foundation/platform/libs/inc
    ${CMAKE_SOURCE_DIR}/foundation/platform/os
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

target_link_libraries(
    ${project_name}
    PRIVATE
    ${CMAKE_SOURCE_DIR}/build/application/ecm/libecm.a
    ${CMAKE_SOURCE_DIR}/build/application/pub/libpub.a
    ${CMAKE_SOURCE_DIR}/build/application/ocm/libocm.a
    ${CMAKE_SOURCE_DIR}/build/application/irm/libirm.a
    ${FOUNDATION_LIB_PATH}/libfoundation.so
    ${CMAKE_SOURCE_DIR}/build/externalLib/lib/libcore_service.a
	${CMAKE_SOURCE_DIR}/build/externalLib/lib/libconfig.a
    ${CMAKE_SOURCE_DIR}/build/oca/libadmoca.so
    ${FOUNDATION_LIB_PATH}/libfoundation.so
    ${LION_LIBSO}
    ${SSL_LIBRARY}
    ${SYSTEM_LIBRARY}
    ${DIM_LIBRARY}
    ${SOA_LIBRARY}
)