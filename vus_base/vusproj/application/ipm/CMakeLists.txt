cmake_minimum_required(VERSION 3.10)

set(adm_ipm_name "admipm")

# ADD_DEFINITIONS(-D ADM_IPM_UNZIP_ECU_PACKAGE)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} \
    -fPIC\
    ")
    
add_library(${adm_ipm_name}
    src/adm_ipm_impl.c  
    src/adm_ipm_impl_lion.c
    src/adm_ipm_event.c  
#    src/adm_ipm_hmi.c  
#    src/adm_ipm_dim.c  
)

target_include_directories(
    ${adm_ipm_name}
    PRIVATE
    ./inc/public
    ./inc/private
    ./src
    ${CMAKE_SOURCE_DIR}/application
    ${CMAKE_SOURCE_DIR}/application/vsm/inc
    ${CMAKE_SOURCE_DIR}/application/pub/inc
    ${CMAKE_SOURCE_DIR}/application/sdk/inc
    ${CMAKE_SOURCE_DIR}/uim/inc/public
    ${CMAKE_SOURCE_DIR}/dim/inc/public
    ${CMAKE_SOURCE_DIR}/application/cdm/inc/public
    ${CMAKE_SOURCE_DIR}/application/irm/inc/public
    ${CMAKE_SOURCE_DIR}/application/eim/inc/public
    ${CMAKE_SOURCE_DIR}/application/cvm/inc/public
    ${CMAKE_SOURCE_DIR}/engine
    ${CMAKE_SOURCE_DIR}/engine/common/config/inc
    ${CMAKE_SOURCE_DIR}/engine/common/lib/inc
    ${CMAKE_SOURCE_DIR}/engine/common/kvstorage/inc
    ${CMAKE_SOURCE_DIR}/engine/config/inc
    ${CMAKE_SOURCE_DIR}/application/ocm/inc/
    ${CMAKE_SOURCE_DIR}/oca/inc
    ${CMAKE_SOURCE_DIR}/engine/ips/inc/public
    ${CMAKE_SOURCE_DIR}/engine/ips/inc/private
    ${CMAKE_SOURCE_DIR}/engine/cds/inc/public
    ${CMAKE_SOURCE_DIR}/engine/irs/inc/public
    ${CMAKE_SOURCE_DIR}/engine/pis/inc/public
    ${CMAKE_SOURCE_DIR}/soa/inc/public
    ${CMAKE_SOURCE_DIR}/engine/cvs/inc/public
    ${CMAKE_SOURCE_DIR}/engine/cvs/inc/private
    ${CMAKE_SOURCE_DIR}/engine/dpo/inc/public
    ${CMAKE_SOURCE_DIR}/engine/dps/inc/public
    ${CMAKE_SOURCE_DIR}/engine/ecs/inc/public
    ${CMAKE_SOURCE_DIR}/engine/vss/inc/public
    ${CMAKE_SOURCE_DIR}/engine/vss/inc/private
    ${CMAKE_SOURCE_DIR}/engine/vrs/inc/public
    ${CMAKE_SOURCE_DIR}/engine/vrs/inc/private
    ${CMAKE_SOURCE_DIR}/engine/ocs/inc/public
    ${CMAKE_SOURCE_DIR}/engine/iss/inc/public
    ${CMAKE_SOURCE_DIR}/engine/common/errorCode/inc
    ${CMAKE_SOURCE_DIR}/common/errorCode/inc
    ${CMAKE_SOURCE_DIR}/common/ecuInfo/inc
    ${CMAKE_SOURCE_DIR}/common/config/inc
    ${CMAKE_SOURCE_DIR}/common/lib/inc
    ${CMAKE_SOURCE_DIR}/foundation/packages
    ${CMAKE_SOURCE_DIR}/foundation/packages/openssl
    ${CMAKE_SOURCE_DIR}/foundation/packages/json-c
    ${CMAKE_SOURCE_DIR}/foundation/packages/log
    ${CMAKE_SOURCE_DIR}/foundation/packages/lib/base64
    ${CMAKE_SOURCE_DIR}/foundation/packages/lib/hmac
    ${CMAKE_SOURCE_DIR}/foundation/platform/libs/inc/kvstorage
    ${CMAKE_SOURCE_DIR}/foundation/platform/libs/inc
    ${CMAKE_SOURCE_DIR}/foundation/platform/os
    ${CMAKE_SOURCE_DIR}/foundation/platform/os
    ${CMAKE_SOURCE_DIR}/dim/dis/disv_gac/inc
    
)

target_link_libraries(
    ${adm_ipm_name}
    PRIVATE
    # ${CMAKE_SOURCE_DIR}/foundation/packages/libs/libleveldb.a
    # ${CMAKE_SOURCE_DIR}/foundation/platform/libs/lib/libkvstorage.a
    ${FOUNDATION_LIB_PATH}/libfoundation.so
    pthread
)
