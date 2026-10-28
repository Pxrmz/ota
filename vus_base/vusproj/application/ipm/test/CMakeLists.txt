cmake_minimum_required(VERSION 3.10)

ADD_DEFINITIONS(-DADM_SDK_VERSION="V1.0.1")
#ADD_DEFINITIONS(-D ADM_IPM_TEMP)
#ADD_DEFINITIONS(-D ADM_IPM_UNZIP_ECU_PACKAGE)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -Wl,--wrap,adm_ecs_getOtaConfigDataFromKV")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wl,--wrap,adm_ecs_getEcuConfig")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wl,--wrap,adm_ecs_delEcuConfigInfo")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wl,--wrap,adm_ecs_updateAndGetConfigInfo")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wl,--wrap,adm_ecs_getIsReportUpSuccLogFile")

get_filename_component(tmp ${CMAKE_SOURCE_DIR} DIRECTORY)
get_filename_component(tmp ${tmp} DIRECTORY)
get_filename_component(PROJECT_DIR ${tmp} DIRECTORY)

set(FOUNDATION_LIB_PATH ${PROJECT_DIR}/foundation/packages/libs)

set(adm_ipm_test "adm_ipm_test")
add_executable(${adm_ipm_test} 
    ${PROJECT_DIR}/application/ipm/src/adm_ipm_impl.c
    ${CMAKE_SOURCE_DIR}/adm_ipm_test.c
)
target_include_directories(
    ${adm_ipm_test}
    PRIVATE
    ${PROJECT_DIR}/application
    ${PROJECT_DIR}/application/ipm/inc/public
    ${PROJECT_DIR}/dim/inc/public
    ${PROJECT_DIR}/dim/dis/disv_gac/inc
    ${PROJECT_DIR}/uim/inc/public
    ${PROJECT_DIR}/application/cdm/inc/public
    ${PROJECT_DIR}/application/irm/inc/public
    ${PROJECT_DIR}/application/vsm/inc
    ${PROJECT_DIR}/application/ocm/inc
    ${PROJECT_DIR}/application/pub/inc
    ${PROJECT_DIR}/application/eim/inc/public
    ${PROJECT_DIR}/oca/inc
    ${PROJECT_DIR}/soa/inc/public
    ${PROJECT_DIR}/engine
    ${PROJECT_DIR}/engine/common/config/inc
    ${PROJECT_DIR}/engine/common/lib/inc
    ${PROJECT_DIR}/engine/common/kvstorage/inc
    ${PROJECT_DIR}/engine/config/inc
    ${PROJECT_DIR}/engine/ips/inc/public
    ${PROJECT_DIR}/engine/ips/inc/private
    ${PROJECT_DIR}/engine/cds/inc/public
    ${PROJECT_DIR}/engine/irs/inc/public
    ${PROJECT_DIR}/engine/pis/inc/public
    ${PROJECT_DIR}/engine/cvs/inc/public
    ${PROJECT_DIR}/engine/cvs/inc/private
    ${PROJECT_DIR}/engine/ecs/inc/public
    ${PROJECT_DIR}/engine/vss/inc/public
    ${PROJECT_DIR}/engine/vss/inc/private
    ${PROJECT_DIR}/engine/vrs/inc/public
    ${PROJECT_DIR}/engine/vrs/inc/private
    ${PROJECT_DIR}/engine/dpo/inc/public
    ${PROJECT_DIR}/engine/dps/inc/public
    ${PROJECT_DIR}/engine//ocs/inc/public
    ${PROJECT_DIR}/engine/iss/inc/public
    ${PROJECT_DIR}/engine/common/errorCode/inc
    ${PROJECT_DIR}/common/errorCode/inc
    ${PROJECT_DIR}/common/ecuInfo/inc
    ${PROJECT_DIR}/common/config/inc
    ${PROJECT_DIR}/common/lib/inc
    ${PROJECT_DIR}/foundation/packages
    ${PROJECT_DIR}/foundation/packages/openssl
    ${PROJECT_DIR}/foundation/packages/json-c
    ${PROJECT_DIR}/foundation/packages/log
    ${PROJECT_DIR}/foundation/packages/lib/base64
    ${PROJECT_DIR}/foundation/packages/lib/hmac
    ${PROJECT_DIR}/foundation/platform/libs/inc/kvstorage
    ${PROJECT_DIR}/foundation/platform/libs/inc
    ${PROJECT_DIR}/foundation/platform/os
    ${PROJECT_DIR}/foundation/platform/os
)

set(SYSTEM_LIBRARY pthread rt dl stdc++)
set(SSL_LIBRARY crypto ssl)
target_link_libraries(
    ${adm_ipm_test}
    PRIVATE
    ${adm_ipm_name}
    ${PROJECT_DIR}/build/externalLib/lib/libcore_service.a
    ${PROJECT_DIR}/build/externalLib/lib/libconfig.a
    ${FOUNDATION_LIB_PATH}/libfoundation.so
	${SSL_LIBRARY}
    ${SYSTEM_LIBRARY}
)

#[[
set(FOUNDATION_LIB_PATH ${CMAKE_SOURCE_DIR}/foundation/packages/libs)

set(adm_ipm_test "adm_ipm_test")
add_executable(${adm_ipm_test} 
    ${CMAKE_SOURCE_DIR}/application/ipm/src/adm_ipm_impl.c
    ./adm_ipm_test.c
)
target_include_directories(
    ${adm_ipm_test}
    PRIVATE
    ${CMAKE_SOURCE_DIR}/application
    ${CMAKE_SOURCE_DIR}/application/ipm/inc/public
    ${CMAKE_SOURCE_DIR}/dim/inc/public
    ${CMAKE_SOURCE_DIR}/dim/dis/disv_gac/inc
    ${CMAKE_SOURCE_DIR}/uim/inc/public
    ${CMAKE_SOURCE_DIR}/application/cdm/inc/public
    ${CMAKE_SOURCE_DIR}/application/irm/inc/public
    ${CMAKE_SOURCE_DIR}/application/vsm/inc
    ${CMAKE_SOURCE_DIR}/application/ocm/inc
    ${CMAKE_SOURCE_DIR}/application/eim/inc/public
    ${CMAKE_SOURCE_DIR}/oca/inc
    ${CMAKE_SOURCE_DIR}/soa/inc/public
    ${CMAKE_SOURCE_DIR}/engine
    ${CMAKE_SOURCE_DIR}/engine/common/config/inc
    ${CMAKE_SOURCE_DIR}/engine/common/lib/inc
    ${CMAKE_SOURCE_DIR}/engine/common/kvstorage/inc
    ${CMAKE_SOURCE_DIR}/engine/config/inc
    ${CMAKE_SOURCE_DIR}/engine/ips/inc/public
    ${CMAKE_SOURCE_DIR}/engine/ips/inc/private
    ${CMAKE_SOURCE_DIR}/engine/cds/inc/public
    ${CMAKE_SOURCE_DIR}/engine/irs/inc/public
    ${CMAKE_SOURCE_DIR}/engine/pis/inc/public
    ${CMAKE_SOURCE_DIR}/engine/cvs/inc/public
    ${CMAKE_SOURCE_DIR}/engine/cvs/inc/private
    ${CMAKE_SOURCE_DIR}/engine/ecs/inc/public
    ${CMAKE_SOURCE_DIR}/engine/vss/inc/public
    ${CMAKE_SOURCE_DIR}/engine/vss/inc/private
    ${CMAKE_SOURCE_DIR}/engine/vrs/inc/public
    ${CMAKE_SOURCE_DIR}/engine/vrs/inc/private
    ${CMAKE_SOURCE_DIR}/engine/dpo/inc/public
    ${CMAKE_SOURCE_DIR}/engine/dps/inc/public
    ${CMAKE_SOURCE_DIR}/engine//ocs/inc/public
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
)

set(SYSTEM_LIBRARY pthread rt dl stdc++)
set(SSL_LIBRARY crypto ssl)
target_link_libraries(
    ${adm_ipm_test}
    PRIVATE
    ${adm_ipm_name}
    ${CMAKE_SOURCE_DIR}/build/externalLib/lib/libcore_service.a
    ${CMAKE_SOURCE_DIR}/build/externalLib/lib/libconfig.a
    ${FOUNDATION_LIB_PATH}/libfoundation.so
	${SSL_LIBRARY}
    ${SYSTEM_LIBRARY}
)
]]