cmake_minimum_required(VERSION 3.10)
project(packakes C CXX)
add_subdirectory(sysi4loader)
add_subdirectory(sysi4hmi)

add_subdirectory(sysiInit)


# 设置Doxygen变量用于生成System interface service user manual
set(DOXYGEN_GENERATE_HTML YES)
set(DOXYGEN_GENERATE_MAN NO)

set(DOXYGEN_PROJECT_NAME "System Interface Service User Manual")
set(DOXYGEN_PROJECT_NUMBER "V2.0.0")
set(DOXYGEN_PROJECT_BRIEF "为DIM客户端提供服务的用户接口手册")
set(DOXYGEN_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build/engine/System_Interface_Service_User_Manual)
set(DOXYGEN_INDEX_FILE ${DOXYGEN_OUTPUT_DIRECTORY}/index.html)


# Interface User Manual
doxygen_add_docs(
    System_Interface_Service_User_Manual
    ${CMAKE_SOURCE_DIR}/foundation/platform/os/adm_typedefs.h
    ${CMAKE_SOURCE_DIR}/sysi/engine/sysi4all/inc/adm_sysi4all_data.h
    ${CMAKE_SOURCE_DIR}/sysi/engine/sysi4loader/inc/sysi4host/adm_sysi4host_service.h
    ${CMAKE_SOURCE_DIR}/sysi/engine/sysi4loader/inc/sysi4ucl/adm_sysi4ucl_service.h
    ${CMAKE_SOURCE_DIR}/sysi/engine/sysi4loader/inc/sysi4vdi/adm_sysi4vdi_service.h
    ${CMAKE_SOURCE_DIR}/sysi/engine/sysi4loader/inc/sysi4trans/adm_sysi4trans_hostService.h
    ${CMAKE_SOURCE_DIR}/sysi/engine/sysi4loader/inc/sysi4dpo/adm_sysi4dpo_service.h
    COMMENT "System Interface Service User Manual"
    )


# 设置Doxygen变量用于生成System interface service user manual
set(DOXYGEN_GENERATE_HTML YES)
set(DOXYGEN_GENERATE_MAN NO)

set(DOXYGEN_PROJECT_NAME "System Interface Client User Manual")
set(DOXYGEN_PROJECT_NUMBER "V2.0.0")
set(DOXYGEN_PROJECT_BRIEF "访问UIM服务端的客户端用户接口手册")
set(DOXYGEN_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build/engine/System_Interface_Client_User_Manual)
set(DOXYGEN_INDEX_FILE ${DOXYGEN_OUTPUT_DIRECTORY}/index.html)


# Interface User Manual
doxygen_add_docs(
    System_Interface_Client_User_Manual
    ${CMAKE_SOURCE_DIR}/foundation/platform/os/adm_typedefs.h
    ${CMAKE_SOURCE_DIR}/sysi/engine/sysi4hmi/inc/adm_sysi4hmi_check.h
    ${CMAKE_SOURCE_DIR}/sysi/engine/sysi4hmi/inc/adm_sysi4hmi_download.h
    ${CMAKE_SOURCE_DIR}/sysi/engine/sysi4hmi/inc/adm_sysi4hmi_install.h
    ${CMAKE_SOURCE_DIR}/sysi/engine/sysi4hmi/inc/adm_sysi4hmi_common.h
    COMMENT "System Interface Client User Manual"
    )


