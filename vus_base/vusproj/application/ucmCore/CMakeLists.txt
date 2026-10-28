cmake_minimum_required(VERSION 3.10)
project(APC CXX C)

set(project_name "ucmCore")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} \
    -fPIC\
    ")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} \
    -fPIC\
    -fpermissive\
    ")

add_library(
    ${project_name}
    SHARED
    src/adm_apc_coreAchieve.c
    src/adm_apc_backtrace.c
    src/adm_apc_apkCore.c
    src/adm_apc_event.c
)

target_include_directories(
    ${project_name}
    PRIVATE

    inc/public
    ${CMAKE_SOURCE_DIR}/application
    ${CMAKE_SOURCE_DIR}/application/sdk/inc/
    ${CMAKE_SOURCE_DIR}/application/ecm/inc/
    ${CMAKE_SOURCE_DIR}/application/ems/inc/
    ${CMAKE_SOURCE_DIR}/application/dpm/inc/public
    ${CMAKE_SOURCE_DIR}/dim/inc/public
    ${CMAKE_SOURCE_DIR}/uim/inc/public
    ${CMAKE_SOURCE_DIR}/uim/src
    ${CMAKE_SOURCE_DIR}/application/cvm/inc/public
    ${CMAKE_SOURCE_DIR}/application/cvm/src
    ${CMAKE_SOURCE_DIR}/application/eim/inc/public
    ${CMAKE_SOURCE_DIR}/application/ipm/inc/public
    ${CMAKE_SOURCE_DIR}/application/irm/inc/public
    ${CMAKE_SOURCE_DIR}/application/pub/inc
    ${CMAKE_SOURCE_DIR}/application/ocm/inc/
    ${CMAKE_SOURCE_DIR}/application/vsm/src
    ${CMAKE_SOURCE_DIR}/application/vrm/inc
    ${CMAKE_SOURCE_DIR}/application/vsm/inc
    ${CMAKE_SOURCE_DIR}/engine
    ${CMAKE_SOURCE_DIR}/engine/cds/inc/public
    ${CMAKE_SOURCE_DIR}/engine/pis/inc/public
    ${CMAKE_SOURCE_DIR}/engine/dps/inc/public
    ${CMAKE_SOURCE_DIR}/engine/tps/inc/public
    ${CMAKE_SOURCE_DIR}/engine/dpo/inc/public
    ${CMAKE_SOURCE_DIR}/engine/common/config/inc
    ${CMAKE_SOURCE_DIR}/engine/common/errorCode/inc
    ${CMAKE_SOURCE_DIR}/engine/common/lib/inc
    ${CMAKE_SOURCE_DIR}/engine/common/kvstorage/inc
    ${CMAKE_SOURCE_DIR}/engine/cvs/inc/private
    ${CMAKE_SOURCE_DIR}/engine/cvs/inc/public

    # ${CMAKE_SOURCE_DIR}/engine/cvs/src
    ${CMAKE_SOURCE_DIR}/engine/ecs/inc/public
    ${CMAKE_SOURCE_DIR}/engine/ips/inc/public
    ${CMAKE_SOURCE_DIR}/engine/ips/src
    ${CMAKE_SOURCE_DIR}/engine/irs/inc/public
    ${CMAKE_SOURCE_DIR}/engine/iss/inc/public
    ${CMAKE_SOURCE_DIR}/engine/ocs/inc/public
    ${CMAKE_SOURCE_DIR}/engine/ocs/src
    ${CMAKE_SOURCE_DIR}/engine/pis/inc/public
    ${CMAKE_SOURCE_DIR}/engine/tps/inc/public
    ${CMAKE_SOURCE_DIR}/engine/vrs/inc/public
    ${CMAKE_SOURCE_DIR}/engine/vrs/inc/private
    ${CMAKE_SOURCE_DIR}/engine/vrs/src
    ${CMAKE_SOURCE_DIR}/engine/vss/inc/public
    ${CMAKE_SOURCE_DIR}/engine/vss/inc/private
    ${CMAKE_SOURCE_DIR}/engine/config/inc
    ${CMAKE_SOURCE_DIR}/soa/inc/public
    ${CMAKE_SOURCE_DIR}/soa/src
    ${CMAKE_SOURCE_DIR}/foundation/arpc/inc
    
    ${CMAKE_SOURCE_DIR}/oca/inc/
    ${CMAKE_SOURCE_DIR}/oca/src/http
    ${CMAKE_SOURCE_DIR}/oca/src
    ${CMAKE_SOURCE_DIR}/oca/vendor/include

    ${CMAKE_SOURCE_DIR}/foundation/tools/tracer/inc

    ${CMAKE_SOURCE_DIR}/uim/inc/public
    ${CMAKE_SOURCE_DIR}/foundation/packages
    ${CMAKE_SOURCE_DIR}/foundation/platform/libs/inc
    ${CMAKE_SOURCE_DIR}/foundation/platform/os
    ${CMAKE_SOURCE_DIR}/foundation/packagescrypt
    ${CMAKE_SOURCE_DIR}/foundation/packages/openssl
    ${CMAKE_SOURCE_DIR}/foundation/packages/curl
    ${CMAKE_SOURCE_DIR}/foundation/packages/json-c
    ${CMAKE_SOURCE_DIR}/foundation/packages/log
    ${CMAKE_SOURCE_DIR}/foundation/packages/lib
    ${CMAKE_SOURCE_DIR}/foundation/packages/lib/inc
    ${CMAKE_SOURCE_DIR}/foundation/packages/lib/hmac
    ${CMAKE_SOURCE_DIR}/foundation/platform/libs/inc/kvstorage
    ${CMAKE_SOURCE_DIR}/foundation/platform/libs/inc/httpproxy

    # compilation clipping
    ${CUSTOM_SUPPORT_HEADERS}
)

if(CUSTOME_USE_DLSYM)
    set(ARPC_LIBSO ${CMAKE_SOURCE_DIR}/foundation/arpc/lib/libarpc.so)
else()
    set(ARPC_LIBSO "")
endif()

target_link_libraries(
    ${project_name}
    PRIVATE

    -Wl,--whole-archive 
    ${CMAKE_SOURCE_DIR}/build/application/pub/libpub.a
    ${CMAKE_SOURCE_DIR}/build/application/sdk/libsdk.a
    ${CMAKE_SOURCE_DIR}/build/application/vrm/libvrm.a
    ${CMAKE_SOURCE_DIR}/build/application/vsm/libvsm.a
    ${CMAKE_SOURCE_DIR}/build/application/ecm/libecm.a
    ${CMAKE_SOURCE_DIR}/build/application/cvm/libcvm.a
    ${CMAKE_SOURCE_DIR}/build/application/dpm/libdpm.a
    ${CMAKE_SOURCE_DIR}/build/application/cdm/libcdm.a
    ${CMAKE_SOURCE_DIR}/build/application/ipm/libadmipm.a
    ${CMAKE_SOURCE_DIR}/build/application/irm/libirm.a
    ${CMAKE_SOURCE_DIR}/build/application/eim/libeim.a
    ${CMAKE_SOURCE_DIR}/build/application/ocm/libocm.a
    ${CMAKE_SOURCE_DIR}/build/oca/libadmoca.so
    ${CMAKE_SOURCE_DIR}/build/application/ems/libems.a
    ${DTS_TOOL_LIB}
    # compilation clipping
    ${CUSTOM_SUPPORT_LIBRARY}
    ${SSL_LIBRARY}
    ${SOA_LIBRARY}
    ${ARPC_LIBSO}
    ${DIM_LIBRARY}
    ${UIM_LIBRARY}
    ${CMAKE_SOURCE_DIR}/build/externalLib/lib/libcore_service.a
    ${CMAKE_SOURCE_DIR}/build/externalLib/lib/libconfig.a
    ${FOUNDATION_LIB_PATH}/libfoundation.so
    -Wl,--no-whole-archive
    ${SYSTEM_LIBRARY}
)
