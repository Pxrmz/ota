cmake_minimum_required(VERSION 3.5.0)


add_subdirectory(lua_3th/lvm)
add_subdirectory(lua_3th/lua_cjson)
add_subdirectory(lua_3th/lua_timer)

add_subdirectory(admSysi4cbf)
if(CUSTOM_VDI_SERVICE_SUPPORT)
	add_subdirectory(admSysi4VdiAda)
endif()
add_subdirectory(admSysi4UclAda)
add_subdirectory(admSysi4DpoAda)
add_subdirectory(admSysi4TransAda)
add_subdirectory(admSysi4HostAda)
if(${CUSTOM_CMAKE_SYSTEM_NAME} MATCHES "Android")
	add_subdirectory(admSysi4icc)
endif()
add_subdirectory(admSysi4DeployApp)

add_subdirectory(admSysi4HmiApp)


set(DOXYGEN_GENERATE_HTML YES)
set(DOXYGEN_GENERATE_MAN NO)

set(DOXYGEN_PROJECT_NAME "Oem Interface User Manual")
set(DOXYGEN_PROJECT_NUMBER "V2.0.0")
set(DOXYGEN_PROJECT_BRIEF "OTA program's dependency interface description document")
set(DOXYGEN_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build/sysi/application/Oem_User_Manual)
set(DOXYGEN_INDEX_FILE ${DOXYGEN_OUTPUT_DIRECTORY}/index.html)


# Interface User Manual
doxygen_add_docs(
    Oem_User_Manual
    ${CMAKE_SOURCE_DIR}/foundation/platform/os/adm_typedefs.h
    ${CMAKE_SOURCE_DIR}/sysi/application/admSysi4HostAda/inc/adm_sysi4host_oemInterface.h
    ${CMAKE_SOURCE_DIR}/sysi/application/admSysi4VdiAda/inc/adm_sysi4vdi_oemInterface.h
    COMMENT "Oem Interface User Manual"
    )



