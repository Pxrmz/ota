cmake_minimum_required(VERSION 2.6)

project(com_abupdate_vus_Service)

set(project_name "com_abupdate_vus_uclService")

add_definitions(-fPIC)

if(CUSTOM_HOST_SERVICE_SUPPORT)
    set(SYSI4HOST_SERVICE_LIB 
        ${CMAKE_SOURCE_DIR}/build/sysi/application/admSysi4icc/libsysi4iccada.so)
else()
    set(SYSI4HOST_SERVICE_LIB "")
endif()

if(CUSTOM_TRANS_SERVICE_SUPPORT)
    set(SYSI4TRANS_SERVICE_LIB 
    ${CMAKE_SOURCE_DIR}/build/sysi/application/admSysi4TransAda/libsysi4transhostada.a
    ${CMAKE_SOURCE_DIR}/build/oca/libadmoca.so)
else()
    set(SYSI4TRANS_SERVICE_LIB "")
endif()

if(CUSTOM_UCL_SERVICE_SUPPORT)
    set(SYSI4UCL_SERVICE_LIB 
    ${CMAKE_SOURCE_DIR}/build/sysi/application/admSysi4icc/libsysi4iccada.so
    ${CMAKE_SOURCE_DIR}/build/sysi/application/admSysi4cbf/libsysi4cbfada.a)
else()
    set(SYSI4UCL_SERVICE_LIB "")
endif()

if(CUSTOM_DPO_SERVICE_SUPPORT)
    set(SYSI4DPO_SERVICE_LIB 
    ${CMAKE_SOURCE_DIR}/build/sysi/application/admSysi4DpoAda/libsysi4dpoada.a
    ${CMAKE_SOURCE_DIR}/build/oca/libadmoca.so)
else()
    set(SYSI4DPO_SERVICE_LIB "")
endif()
if(CUSTOM_VDI_SERVICE_SUPPORT)
    set(SYSI4VDI_SERVICE_LIB
            ${CMAKE_SOURCE_DIR}/build/sysi/application/admSysi4VdiAda/libsysi4vdiada.a
            ${CMAKE_SOURCE_DIR}/sysi/application/admSysi4VdiAda/vdilib/libabupvdi.so
            ${CMAKE_SOURCE_DIR}/build/sysi/application/admSysi4cbf/libsysi4cbfada.a
        )
else()
    set(SYSI4VDI_SERVICE_LIB "")
endif()

file(GLOB FILE_LIST 
	${CMAKE_SOURCE_DIR}/sysi/jni/engine/ucl/src/*.c
	${CMAKE_SOURCE_DIR}/sysi/jni/engine/host/src/*.c
)

add_library(
	${project_name}
	SHARED
	${FILE_LIST}
)

target_include_directories(
    ${project_name}
    PRIVATE
	${CMAKE_SOURCE_DIR}/sysi/jni/inc
	${CMAKE_SOURCE_DIR}/sysi/jni/engine/ucl/inc
	${CMAKE_SOURCE_DIR}/sysi/jni/engine/host/inc
    ${CMAKE_SOURCE_DIR}/sysi/application/admSysi4DeployApp/sample/inc
     ${CMAKE_SOURCE_DIR}/sysi/application/admSysi4icc
    ${CMAKE_SOURCE_DIR}/sysi/application/admSysi4HostAda/src
    ${CMAKE_SOURCE_DIR}/sysi/application/admSysi4HostAda/inc
    # ${CMAKE_SOURCE_DIR}/sysi/application/admSysi4HostAda/sysi4host_lvm/sysi4host_lvm/inc
    ${CMAKE_SOURCE_DIR}/sysi/application/admSysi4UclAda/src
    ${CMAKE_SOURCE_DIR}/sysi/application/admSysi4UclAda/inc
    # ${CMAKE_SOURCE_DIR}/sysi/application/admSysi4UclAda/sysi4ucl_lvm/sysi4ucl_lvm/inc
    ${CMAKE_SOURCE_DIR}/sysi/application/admSysi4DpoAda/inc
    ${CMAKE_SOURCE_DIR}/sysi/application/admSysi4TransAda/inc
    
    ${CMAKE_SOURCE_DIR}/sysi/application/sdk/inc/
    ${CMAKE_SOURCE_DIR}/engine/common/config/inc/
    ${CMAKE_SOURCE_DIR}/foundation/platform/os
    ${CMAKE_SOURCE_DIR}/foundation/packages/log
    ${CMAKE_SOURCE_DIR}/engine
    ${CMAKE_SOURCE_DIR}/engine/config/inc/
    ${CMAKE_SOURCE_DIR}/engine/common/errorCode/inc
    ${CMAKE_SOURCE_DIR}/engine/common/lib/inc
    ${CMAKE_SOURCE_DIR}/sysi/engine/
    ${CMAKE_SOURCE_DIR}/sysi/engine/sysiInit/inc
    ${CMAKE_SOURCE_DIR}/sysi/engine/sysi4all/inc
    ${CMAKE_SOURCE_DIR}/sysi/engine/sysi4loader/inc
    ${CMAKE_SOURCE_DIR}/sysi/engine/sysi4loader/inc/sysi4vdi
    ${CMAKE_SOURCE_DIR}/sysi/engine/sysi4loader/inc/sysi4host
    ${CMAKE_SOURCE_DIR}/sysi/engine/sysi4loader/inc/sysi4ucl
    ${CMAKE_SOURCE_DIR}/sysi/engine/sysi4loader/inc/sysi4trans
    ${CMAKE_SOURCE_DIR}/sysi/engine/sysi4loader/inc/sysi4dpo
    ${CMAKE_SOURCE_DIR}/sysi/engine/sysi4loader/proto_soc/inc
    ${CMAKE_SOURCE_DIR}/sysi/application/admSysi4VdiAda/inc
    ${CMAKE_SOURCE_DIR}/sysi/engine/sysi4dpo/inc

    ${CMAKE_SOURCE_DIR}/foundation/packages
    ${CMAKE_SOURCE_DIR}/foundation/packages/log
    ${CMAKE_SOURCE_DIR}/foundation/packages/dlt
    ${CMAKE_SOURCE_DIR}/foundation/platform/os
    ${CMAKE_SOURCE_DIR}/sysi/engine/sysi4hmi/inc
    ${CMAKE_SOURCE_DIR}/foundation/arpc/inc
    ${CMAKE_SOURCE_DIR}/foundation/packages
    ${CMAKE_SOURCE_DIR}/foundation/tools/tracer/inc 
    ${CMAKE_SOURCE_DIR}/sysi/engine/sysi4all/inc
    ${CMAKE_SOURCE_DIR}/foundation/platform/libs/inc
    ${CMAKE_SOURCE_DIR}/foundation/packages/json-c
    ${CMAKE_SOURCE_DIR}/soa/inc/public
    ${CMAKE_SOURCE_DIR}/soa/src
    
	#oemͷ�ļ�
    ${CMAKE_SOURCE_DIR}/sysi/application/admSysi4DeployApp/vendor/pub/inc
)

target_link_libraries(
    ${project_name}
    PRIVATE

    -Wl,--whole-archive
    ${CMAKE_SOURCE_DIR}/build/sysi/application/admSysi4DeployApp/libsysi4deploy.so
    ${SYSI4HOST_SERVICE_LIB}
    ${SYSI4TRANS_SERVICE_LIB}
    ${SYSI4UCL_SERVICE_LIB}
    ${SYSI4DPO_SERVICE_LIB}
    ${SYSI4VDI_SERVICE_LIB}
    
    ${CMAKE_SOURCE_DIR}/foundation/arpc/lib/libarpc.so
    ${CMAKE_SOURCE_DIR}/build/application/ocm/libocm.a
    #${CMAKE_SOURCE_DIR}/build/application/ems/libems.a
    ${SSL_LIBRARY}
    
    # ${CMAKE_SOURCE_DIR}/build/sysi/application/lua_3th/lvm/liblua-static.a
    # ${CMAKE_SOURCE_DIR}/build/sysi/application/lua_3th/lua_cjson/libcjson-static.a
    # ${CMAKE_SOURCE_DIR}/build/sysi/application/lua_3th/lua_timer/liblua_timer.a
    
    ${DTS_TOOL_LIB}
    ${CMAKE_SOURCE_DIR}/foundation/packages/libs/libfoundation.so
    ${SYSTEM_LIBRARY}
    -Wl,--no-whole-archive
    )

# set_target_properties(${project_name} PROPERTIES INSTALL_RPATH ${INSTALL_RPATH_DIR})
# INSTALL(TARGETS ${project_name} LIBRARY DESTINATION ${MY_INSTALL_DIR})
