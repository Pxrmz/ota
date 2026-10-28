cmake_minimum_required(VERSION 3.10)
project(SYSI4HOSTADA CXX C)

set(project_name "sysi4hostada")

add_library(${project_name}
    ./src/adm_sysi4host_simulationInterface.c
    ./src/adm_sysi4host_handler.c
    ./sysi4host_lvm/sysi4host_lvm/src/adm_sysi4host_handler_lvm.c

    )

target_include_directories(
    ${project_name}
    PRIVATE
    
    #oem头文件
    ${CMAKE_SOURCE_DIR}/sysi/application/admSysi4DeployApp/vendor/pub/inc

    ${CMAKE_SOURCE_DIR}/sysi/application/admSysi4HostAda/inc
    ${CMAKE_SOURCE_DIR}/sysi/application/admSysi4HostAda/sysi4host_lvm/sysi4host_lvm/inc
    ${CMAKE_SOURCE_DIR}/sysi/application/admSysi4HostAda/src
    ${CMAKE_SOURCE_DIR}/sysi/engine
    ${CMAKE_SOURCE_DIR}/sysi/engine/sysi4all/inc
    ${CMAKE_SOURCE_DIR}/engine/common/config/inc
    ${CMAKE_SOURCE_DIR}/engine/common/errorCode/inc
    ${CMAKE_SOURCE_DIR}/engine/common/lib/inc
    ${CMAKE_SOURCE_DIR}/sysi/engine/sysi4loader/inc/sysi4host
    ${CMAKE_SOURCE_DIR}/sysi/engine/sysi4loader/inc
    ${CMAKE_SOURCE_DIR}/sysi/engine/sysi4loader/inc/sysi4vdi
    ${CMAKE_SOURCE_DIR}/sysi/engine/sysi4loader/inc/sysi4host
    ${CMAKE_SOURCE_DIR}/sysi/engine/sysi4loader/inc/sysi4ucl
    ${CMAKE_SOURCE_DIR}/sysi/engine/sysi4loader/inc/sysi4trans
    ${CMAKE_SOURCE_DIR}/sysi/engine/sysi4loader/inc/sysi4dpo
    ${CMAKE_SOURCE_DIR}/sysi/engine/sysi4loader/proto_soc/inc

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
    ${CMAKE_SOURCE_DIR}/engine/config/inc/

    ${oem_vendor_include}
    ${CMAKE_SOURCE_DIR}/sysi/application/admSysi4HostAda/vendor/

    ${CMAKE_SOURCE_DIR}/sysi/application/lua_3th/lua_timer/inc
    ${CMAKE_SOURCE_DIR}/dim/dimtool/dim_lua_3th/lvm/inc/public
    ${CMAKE_SOURCE_DIR}/dim/dimtool/dim_lua_3th/lua_cjson/inc

    ${CMAKE_SOURCE_DIR}/foundation/arpc/inc
    ${CMAKE_SOURCE_DIR}/soa/inc/public
    ${CMAKE_SOURCE_DIR}/dim/dis/dissoa_json_soc/inc/

    )

target_link_libraries(
    ${project_name}
	PRIVATE
    ${CMAKE_SOURCE_DIR}/build/sysi/application/lua_3th/lvm/liblua-static.a
    ${CMAKE_SOURCE_DIR}/build/sysi/application/lua_3th/lua_cjson/libcjson-static.a
)

if(USE_SIMULATE_DATA)
    add_subdirectory(simulate)
endif()
