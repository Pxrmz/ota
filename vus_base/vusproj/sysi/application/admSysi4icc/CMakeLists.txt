cmake_minimum_required(VERSION 3.10)
project(SYSI4ICCADA CXX C)

set(project_name "sysi4iccada")


add_library(${project_name}

    SHARED

    ./adm_sysi4host_handler.cc
    ./adm_sysi4ucl_handler_up.cc
    ./adm_sysi4host_handler_auth.cc
    ./adm_sysi4emc_handler.c
    ${CMAKE_SOURCE_DIR}/sysi/application/admSysi4UclAda/src/adm_sysi4ucl_handler_sm.c
    
    ${CMAKE_SOURCE_DIR}/sysi/application/admSysi4HostAda/sysi4host_lvm/sysi4host_lvm/src/adm_sysi4host_handler_lvm.c
    ${CMAKE_SOURCE_DIR}/sysi/application/admSysi4UclAda/sysi4ucl_lvm/sysi4ucl_lvm/src/adm_sysi4ucl_handler_lvm.c
    )

target_include_directories(
    ${project_name}
    PRIVATE
    ${CMAKE_SOURCE_DIR}/sysi/jni/engine/ucl/inc
    ${CMAKE_SOURCE_DIR}/sysi/jni/engine/host/inc
    ${CMAKE_SOURCE_DIR}/sysi/jni/inc
    
    ${CMAKE_SOURCE_DIR}/sysi/application/admSysi4DeployApp/vendor/pub/inc
    ${CMAKE_SOURCE_DIR}/sysi/application/admSysi4HostAda/inc
    ${CMAKE_SOURCE_DIR}/sysi/application/admSysi4UclAda/inc
    ${CMAKE_SOURCE_DIR}/sysi/application/admSysi4cbf/inc
    ${CMAKE_SOURCE_DIR}/sysi/application/admSysi4cbf/cbf-parse/cbf_parse_lib/inc
    
    #这个不应该在适配层
    ${CMAKE_SOURCE_DIR}/sysi/application/admSysi4HostAda/sysi4host_lvm/sysi4host_lvm/inc
    ${CMAKE_SOURCE_DIR}/sysi/application/admSysi4UclAda/sysi4ucl_lvm/sysi4ucl_lvm/inc
    
    ${CMAKE_SOURCE_DIR}/sysi/application/lua_3th/lua_timer/inc
    ${CMAKE_SOURCE_DIR}/dim/dimtool/dim_lua_3th/lvm/inc/public
    ${CMAKE_SOURCE_DIR}/dim/dimtool/dim_lua_3th/lua_cjson/inc
    
    ${CMAKE_SOURCE_DIR}/sysi/engine/sysi4all/inc
    ${CMAKE_SOURCE_DIR}/sysi/engine/sysi4hmi/inc
    ${CMAKE_SOURCE_DIR}/engine/common/config/inc
    ${CMAKE_SOURCE_DIR}/engine/common/errorCode/inc
    ${CMAKE_SOURCE_DIR}/engine/common/lib/inc
    ${CMAKE_SOURCE_DIR}/engine/iss/inc/public
    
    ${CMAKE_SOURCE_DIR}/sysi/engine/sysi4loader/inc
    ${CMAKE_SOURCE_DIR}/sysi/engine/sysi4loader/inc/sysi4vdi
    ${CMAKE_SOURCE_DIR}/sysi/engine/sysi4loader/inc/sysi4host
    ${CMAKE_SOURCE_DIR}/sysi/engine/sysi4loader/inc/sysi4ucl
    ${CMAKE_SOURCE_DIR}/sysi/engine/sysi4loader/inc/sysi4trans
    ${CMAKE_SOURCE_DIR}/sysi/engine/sysi4loader/inc/sysi4dpo
    ${CMAKE_SOURCE_DIR}/sysi/engine/sysi4loader/proto_soc/inc

    ${CMAKE_SOURCE_DIR}/sysi/engine/sysiInit/inc

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
    ${CMAKE_SOURCE_DIR}/engine/config/inc
    ${CMAKE_SOURCE_DIR}/foundation/arpc/inc
    ${CMAKE_SOURCE_DIR}/soa/inc/public

    )

target_link_libraries(
    ${project_name}
    PRIVATE
    ${CMAKE_SOURCE_DIR}/build/sysi/application/lua_3th/lvm/liblua-static.a
    ${CMAKE_SOURCE_DIR}/build/sysi/application/lua_3th/lua_cjson/libcjson-static.a
    ${CMAKE_SOURCE_DIR}/build/sysi/application/lua_3th/lua_timer/liblua_timer.a
    ${CMAKE_SOURCE_DIR}/foundation/packages/libs/libfoundation.so
    ${SYSTEM_LIBRARY}
)

