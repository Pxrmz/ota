cmake_minimum_required (VERSION 3.0)
project(HMI C CXX)

set(project_name "HMI")
SET(CMAKE_BUILD_TYPE "Debug")

add_definitions(-fPIC)

add_library(${project_name}
    STATIC
    src/adm_hut_interface.c
    src/adm_hut_manager.c
    src/adm_hut_comm.c
    src/adm_hut_check.c
    src/adm_hut_download.c
    src/adm_hut_install.c
)

MESSAGE(STATUS "===> CMAKE_CURRENT_LIST_DIR, ${CMAKE_CURRENT_LIST_DIR}")
MESSAGE(STATUS "===> CMAKE_SOURCE_DIR, ${CMAKE_SOURCE_DIR}")

target_include_directories(
    ${project_name}
    PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/../../foundation/packages/json-c
    ${CMAKE_CURRENT_LIST_DIR}/../../foundation/packages/log
    ${CMAKE_CURRENT_LIST_DIR}/../../foundation/packages/paho
    ${CMAKE_CURRENT_LIST_DIR}/../../foundation/platform/libs/inc
    ${CMAKE_CURRENT_LIST_DIR}/../../foundation/platform/libs
    ${CMAKE_CURRENT_LIST_DIR}/../../foundation/platform/os
    ${CMAKE_CURRENT_LIST_DIR}/../cvm/inc/public
    ${CMAKE_CURRENT_LIST_DIR}/../dpm/inc/public
    ${CMAKE_CURRENT_LIST_DIR}/../ipm/inc/public
    ${CMAKE_CURRENT_LIST_DIR}/../ipm/inc/private
    ${CMAKE_CURRENT_LIST_DIR}/../vrm/inc
    ${CMAKE_CURRENT_LIST_DIR}/../../common/config/inc/
    ${CMAKE_CURRENT_LIST_DIR}/../../engine/vrs/inc/public
    ${CMAKE_CURRENT_LIST_DIR}/../../engine/ocs/inc/public
    ${CMAKE_CURRENT_LIST_DIR}/../../engine/dpo/inc/public
    ${CMAKE_CURRENT_LIST_DIR}/../../engine/dps/inc/public
    ${CMAKE_CURRENT_LIST_DIR}/../../engine/ips/inc/public
    ${CMAKE_CURRENT_LIST_DIR}/../../engine/common/config/inc
    ${CMAKE_CURRENT_LIST_DIR}/../../engine
    ${CMAKE_CURRENT_LIST_DIR}/../../engine/common/errorCode/inc
    ./inc/private
    ./inc/public
)

target_link_libraries(
	${PROJECT_NAME}
	    ${CMAKE_SOURCE_DIR}/build/application/cvm/libcvm.a
	    ${CMAKE_SOURCE_DIR}/build/application/dpm/libdpm.a
	    ${CMAKE_SOURCE_DIR}/build/application/ipm/libadmipm.a
	    ${CMAKE_SOURCE_DIR}/build/application/vrm/libvrm.a
        ${FOUNDATION_LIB_PATH}/libfoundation.so
        ${SYSTEM_LIBRARY}
)

#add_subdirectory(sample/main)

