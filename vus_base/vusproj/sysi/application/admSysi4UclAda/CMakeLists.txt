cmake_minimum_required(VERSION 3.10)
project(SYSI4UCLADA CXX C)

set(project_name "sysi4uclada")

add_library(${project_name}

    ./src/adm_sysi4ucl_handler_sm.c
    ./src/adm_sysi4ucl_handler_up.cc
    ./sysi4ucl_lvm/sysi4ucl_lvm/src/adm_sysi4ucl_handler_lvm.c

)


target_include_directories(
    ${project_name}
    PRIVATE

    #oem头文件
    ${CMAKE_SOURCE_DIR}/sysi/application/admSysi4DeployApp/vendor/pub/inc

    sysi4ucl_lvm/sysi4ucl_lvm/inc
    ${CMAKE_SOURCE_DIR}/sysi/application/admSysi4cbf/cbf-parse/cbf_parse_lib/inc
    ${CMAKE_SOURCE_DIR}/sysi/application/admSysi4cbf/inc
    
    ${CMAKE_SOURCE_DIR}/foundation/platform/os

    ${CMAKE_SOURCE_DIR}/engine/common/errorCode/inc

    ${CMAKE_SOURCE_DIR}/sysi/engine/sysi4all/inc
    ${CMAKE_SOURCE_DIR}/sysi/engine/sysi4loader/inc
    ${CMAKE_SOURCE_DIR}/sysi/engine/sysi4loader/inc/sysi4ucl
    ${CMAKE_SOURCE_DIR}/sysi/engine/sysi4loader/inc/sysi4trans
    ${CMAKE_SOURCE_DIR}/sysi/engine/sysi4loader/inc/sysi4vdi
    ${CMAKE_SOURCE_DIR}/sysi/engine/sysi4loader/inc/sysi4host
    ${CMAKE_SOURCE_DIR}/sysi/engine/sysi4loader/inc/sysi4dpo
    ${CMAKE_SOURCE_DIR}/sysi/engine/sysiInit/inc
    ${CMAKE_SOURCE_DIR}/foundation/packages/json-c

    ${VENDOR_INC_DIR}
    
    ${CMAKE_CURRENT_LIST_DIR}/inc
    ${CMAKE_SOURCE_DIR}/engine/common/config/inc
    ${CMAKE_SOURCE_DIR}/engine/iss/inc/public


    ${CMAKE_SOURCE_DIR}/foundation/packages/log
    ${CMAKE_SOURCE_DIR}/foundation/packages
    ${CMAKE_SOURCE_DIR}/foundation/packages/openssl
    ${CMAKE_SOURCE_DIR}/foundation/packages/json-c

    ${CMAKE_SOURCE_DIR}/foundation/tools/tracer/inc

    ${CMAKE_SOURCE_DIR}/foundation/packages/dlt	
    
    ${CMAKE_SOURCE_DIR}/sysi/application/admSysi4HostAda/vendor/
    ${CMAKE_SOURCE_DIR}/engine/config/inc/
    ${CMAKE_SOURCE_DIR}/engine/common/lib/inc
    ${oem_vendor_include}

    ${CMAKE_SOURCE_DIR}/sysi/application/lua_3th/lua_timer/inc
    ${CMAKE_SOURCE_DIR}/dim/dimtool/dim_lua_3th/lvm/inc/public
    ${CMAKE_SOURCE_DIR}/dim/dimtool/dim_lua_3th/lua_cjson/inc
    

    )


target_link_libraries(
    ${project_name}
	PRIVATE
    ${CMAKE_SOURCE_DIR}/build/sysi/application/lua_3th/lvm/liblua-static.a
    ${CMAKE_SOURCE_DIR}/build/sysi/application/lua_3th/lua_cjson/libcjson-static.a

	${FOUNDATION_LIB_PATH}/libfoundation.so
	${SSL_LIBRARY}
    ${SYSTEM_LIBRARY}
)

if(USE_SIMULATE_DATA)
    add_subdirectory(simulate)
endif()