cmake_minimum_required(VERSION 3.10)

file(COPY adm_unittest_starter.sh DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

set(adm_core_unittest "adm_core_unittest")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC -g -std=gnu11")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -g -std=c++11 -Wno-write-strings")

set(copy_test_file "copy_test_file")
add_custom_target(${copy_test_file}
    COMMAND ${CMAKE_COMMAND} -E copy adm_gtest2html.xslt "${CMAKE_CURRENT_BINARY_DIR}"
    COMMAND ${CMAKE_COMMAND} -E copy adm_common_hash_config.json "${CMAKE_CURRENT_BINARY_DIR}"
    COMMAND ${CMAKE_COMMAND} -E copy adm_common_hash_in.data "${CMAKE_CURRENT_BINARY_DIR}"
    COMMAND ${CMAKE_COMMAND} -E copy adm_common_lib_zip.zip "${CMAKE_CURRENT_BINARY_DIR}"
    COMMAND ${CMAKE_COMMAND} -E copy adm_iss_aes_config.json "${CMAKE_CURRENT_BINARY_DIR}"
    COMMAND ${CMAKE_COMMAND} -E copy adm_iss_b64_config.json "${CMAKE_CURRENT_BINARY_DIR}"
    COMMAND ${CMAKE_COMMAND} -E copy adm_iss_hmac_config.json "${CMAKE_CURRENT_BINARY_DIR}"
    COMMAND ${CMAKE_COMMAND} -E copy adm_iss_rsa_config.json "${CMAKE_CURRENT_BINARY_DIR}"

    COMMAND ${CMAKE_COMMAND} -E copy check_resp.txt "${CMAKE_CURRENT_BINARY_DIR}"
    
    COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_CURRENT_BINARY_DIR}/data/"
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/data "${CMAKE_CURRENT_BINARY_DIR}/data"
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

add_executable(${adm_core_unittest}    
    ./adm_test_tool.cc 
    ./adm_ips_unittest.cc
    ./adm_iss_aes_unittest.cc
    ./adm_iss_b64_unittest.cc
    ./adm_iss_hmac_unittest.cc
    ./adm_iss_rsa_unittest.cc
    ./adm_pis_unittest.cc
    ./adm_common_hash_unittest.cc
    ./adm_common_lib_unittest.cc
    ./adm_common_mod_unittest.cc
    ./adm_common_parse_unittest.cc
    ./adm_cds_unittest.h
    ./adm_cds_unittest.cc
    ./adm_cds_parser_unittest.h
    ./adm_cds_parser_unittest.cc
    ./adm_ocs_air_protocol_unittest.h
    ./adm_ocs_air_protocol_unittest.cc
    ./adm_ocs_airProtocol_Adapter_unittest.h
    ./adm_ocs_airProtocol_Adapter_unittest.cc
    ./adm_dpo_unittest.cc
    ./adm_dpo_unittest.h
    ./adm_ecs_unittest.cc
    ./adm_ecs_unittest.h
    ./adm_vrs_unittest.cc
    ./adm_vrs_unittest.h
    ./adm_dps_unittest.cc
    ./adm_dps_unittest.h
    ./adm_tps_unittest.cc
    ./adm_tps_unittest.h
    ./adm_cvs_checkVersion_unittest.h
    ./adm_cvs_checkVersion_unittest.cc
    ./adm_irs_unittest.h
    ./adm_irs_unittest.cc
)
target_include_directories(
    ${adm_core_unittest}
    PRIVATE
    ${CMAKE_SOURCE_DIR}/engine
    ${CMAKE_SOURCE_DIR}/engine/common/config/inc
    ${CMAKE_SOURCE_DIR}/engine/common/lib/inc
    ${CMAKE_SOURCE_DIR}/engine/ips/inc/public
    ${CMAKE_SOURCE_DIR}/engine/ips/inc/private
    ${CMAKE_SOURCE_DIR}/engine/ips/src
    ${CMAKE_SOURCE_DIR}/engine/ocs/inc/private
    ${CMAKE_SOURCE_DIR}/engine/dpo/inc/public
    ${CMAKE_SOURCE_DIR}/engine/dps/inc/public
    ${CMAKE_SOURCE_DIR}/engine/tps/inc/public
    ${CMAKE_SOURCE_DIR}/engine/ecs/inc/public
    ${CMAKE_SOURCE_DIR}/engine/cds/inc/public
    ${CMAKE_SOURCE_DIR}/engine/pis/inc/public
    ${CMAKE_SOURCE_DIR}/engine/soa/inc/public
    ${CMAKE_SOURCE_DIR}/engine/cvs/inc/public
    ${CMAKE_SOURCE_DIR}/engine/cvs/inc/private
    ${CMAKE_SOURCE_DIR}/engine/vss/inc/public
    ${CMAKE_SOURCE_DIR}/engine/vss/inc/private
    ${CMAKE_SOURCE_DIR}/engine/vrs/inc/public
    ${CMAKE_SOURCE_DIR}/engine/vrs/inc/private
    ${CMAKE_SOURCE_DIR}/engine/ocs/inc/public
    ${CMAKE_SOURCE_DIR}/engine/iss/inc/public
    ${CMAKE_SOURCE_DIR}/engine/irs/inc/public
    ${CMAKE_SOURCE_DIR}/engine/common/errorCode/inc
    ${CMAKE_SOURCE_DIR}/common/errorCode/inc
    ${CMAKE_SOURCE_DIR}/common/ecuInfo/inc
    ${CMAKE_SOURCE_DIR}/common/config/inc
    ${CMAKE_SOURCE_DIR}/common/lib/inc
    ${CMAKE_SOURCE_DIR}/../foundation_lib/${ENV_PLATFORM}/packages
    ${CMAKE_SOURCE_DIR}/../foundation_lib/${ENV_PLATFORM}/packages/openssl
    ${CMAKE_SOURCE_DIR}/../foundation_lib/${ENV_PLATFORM}/packages/json-c
    ${CMAKE_SOURCE_DIR}/../foundation_lib/${ENV_PLATFORM}/packages/log
    ${CMAKE_SOURCE_DIR}/../foundation_lib/${ENV_PLATFORM}/packages/lib
    ${CMAKE_SOURCE_DIR}/../foundation_lib/${ENV_PLATFORM}/packages/lib/base64
    ${CMAKE_SOURCE_DIR}/../foundation_lib/${ENV_PLATFORM}/packages/lib/hmac
    ${CMAKE_SOURCE_DIR}/../foundation_lib/${ENV_PLATFORM}/platform/libs/inc/kvstorage
    ${CMAKE_SOURCE_DIR}/../foundation_lib/${ENV_PLATFORM}/platform/libs/inc
    ${CMAKE_SOURCE_DIR}/../foundation_lib/${ENV_PLATFORM}/platform/os
    ${CMAKE_SOURCE_DIR}/../foundation_lib/${ENV_PLATFORM}/platform/os
)

if(${PLATFORM_NAME} MATCHES "QNX7")
  target_link_libraries(
    ${adm_core_unittest}
    PRIVATE
    ${FOUNDATION_LIB_PATH}/libfoundation.so
    -Wl,--whole-archive
    ${FOUNDATION_LIB_PATH}/libjsoncpp.a
    core_service
    ${SSL_LIBRARY}
    -Wl,--no-whole-archive
    ${SYSTEM_LIBRARY}
    c++
    c
    m
    socket
  )
else()
  target_link_libraries(
    ${adm_core_unittest}
    PRIVATE
    ${FOUNDATION_LIB_PATH}/libfoundation.so
    -Wl,--whole-archive
    ${FOUNDATION_LIB_PATH}/libjsoncpp.a
    core_service
    ${FOUNDATION_LIB_PATH}/libgtest_maind.a
    ${FOUNDATION_LIB_PATH}/libgtestd.a
    ${SSL_LIBRARY}
    -Wl,--no-whole-archive
    ${SYSTEM_LIBRARY}
    -lstdc++
  )
endif()

add_dependencies(${adm_core_unittest} ${copy_test_file})
