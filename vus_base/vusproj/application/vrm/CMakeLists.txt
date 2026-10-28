cmake_minimum_required(VERSION 3.10)
project(VRM CXX C)

set(project_name "vrm")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} \
    -fPIC\
    ")
    
add_library(${project_name}
	./src/adm_vrm_register.c
    ./src/adm_vrm_event.c
)

target_include_directories(
    ${project_name}
    PRIVATE
    ./inc/
    ${CMAKE_SOURCE_DIR}/application
    ${CMAKE_SOURCE_DIR}/engine/config/inc/
    ${CMAKE_SOURCE_DIR}/engine
    ${CMAKE_SOURCE_DIR}/engine/common/config/inc
    ${CMAKE_SOURCE_DIR}/engine/common/errorCode/inc
    ${CMAKE_SOURCE_DIR}/engine/common/lib/inc
    ${CMAKE_SOURCE_DIR}/engine/cvs/inc/private
    ${CMAKE_SOURCE_DIR}/engine/cvs/inc/public
    ${CMAKE_SOURCE_DIR}/dim/inc/public
    ${CMAKE_SOURCE_DIR}/application/cvm/inc/public
    ${CMAKE_SOURCE_DIR}/application/ocm/inc/
    ${CMAKE_SOURCE_DIR}/application/sdk/inc/
    ${CMAKE_SOURCE_DIR}/engine/cvs/src
    ${CMAKE_SOURCE_DIR}/engine/dps/inc/public
    ${CMAKE_SOURCE_DIR}/engine/dpo/inc/public
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
    ${CMAKE_SOURCE_DIR}/arpc/inc
    ${CMAKE_SOURCE_DIR}/oca/inc/
    ${CMAKE_SOURCE_DIR}/soa/inc/public
)

target_link_libraries(
    ${project_name}
    PRIVATE
    ${CMAKE_SOURCE_DIR}/build/application/ocm/libocm.a
    ${CMAKE_SOURCE_DIR}/build/externalLib/lib/libcore_service.a
	${CMAKE_SOURCE_DIR}/build/externalLib/lib/libconfig.a
    ${FOUNDATION_LIB_PATH}/libfoundation.so
    ${CMAKE_SOURCE_DIR}/build/oca/libadmoca.so
    ${SYSTEM_LIBRARY}
)

#add_subdirectory(sample)
