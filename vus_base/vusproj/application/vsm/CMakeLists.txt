cmake_minimum_required(VERSION 3.10)

set(vsm_model_name "vsm")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} \
    -fPIC\
    ")
    
add_library(${vsm_model_name}
    src/adm_vsm_impl.c
    src/adm_vsm_tspImpl.c
    src/adm_vsm_signal_impl.c
)

target_include_directories(
    ${vsm_model_name}
    PRIVATE
    ${CMAKE_CURRENT_DIR}/src
    ${CMAKE_SOURCE_DIR}/application
    ${CMAKE_SOURCE_DIR}/application/vsm/inc
    ${CMAKE_SOURCE_DIR}/application/vsm/src
    ${CMAKE_SOURCE_DIR}/application/cvm/inc/public
    ${CMAKE_SOURCE_DIR}/application/dpm/inc/public
    ${CMAKE_SOURCE_DIR}/application/tpm/inc/public
    ${CMAKE_SOURCE_DIR}/application/ipm/inc/public
    ${CMAKE_SOURCE_DIR}/application/irm/inc/public
    ${CMAKE_SOURCE_DIR}/uim/inc/public
    ${CMAKE_SOURCE_DIR}/dim/inc/public

    ${CMAKE_SOURCE_DIR}/engine
    ${CMAKE_SOURCE_DIR}/engine/common/lib/inc
    ${CMAKE_SOURCE_DIR}/engine/common/config/inc
    ${CMAKE_SOURCE_DIR}/engine/common/errorCode/inc
    ${CMAKE_SOURCE_DIR}/engine/common/kvstorage/inc
    ${CMAKE_SOURCE_DIR}/engine/irs/inc/public
    ${CMAKE_SOURCE_DIR}/engine/dps/inc/public
    ${CMAKE_SOURCE_DIR}/engine/cds/inc/public
    ${CMAKE_SOURCE_DIR}/engine/cvs/inc/public
    ${CMAKE_SOURCE_DIR}/engine/ocs/inc/public
    ${CMAKE_SOURCE_DIR}/engine/ips/inc/public
    ${CMAKE_SOURCE_DIR}/engine/vrs/inc/public

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

    ${CMAKE_SOURCE_DIR}/foundation/arpc/inc
    ${CMAKE_SOURCE_DIR}/soa/inc/public
    ${CMAKE_SOURCE_DIR}/dim/dis/dissoa_json_soc/inc/
)

target_link_libraries(
    ${vsm_model_name}
    PRIVATE
    ${FOUNDATION_LIB_PATH}/libfoundation.so
    ${SSL_LIBRARY}
    ${SYSTEM_LIBRARY}
)