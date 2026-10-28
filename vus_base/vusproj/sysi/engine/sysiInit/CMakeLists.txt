cmake_minimum_required(VERSION 3.10)
project(SYSI4ALL CXX C)

set(project_name "sysiInit")

add_library(${project_name}
    # SHARED
	STATIC
    ./src/adm_sysiInit.c
    ./src/adm_sysi_backtrace.c
    ./src/adm_sysi_keylog.c
    ./src/adm_apk_soa_init.c
    )

target_include_directories(
    ${project_name}
    PRIVATE
    ${CMAKE_SOURCE_DIR}/engine/
    ${CMAKE_SOURCE_DIR}/engine/common/lib/inc
    ${CMAKE_SOURCE_DIR}/engine/common/config/inc
    ${CMAKE_SOURCE_DIR}/engine/common/kvstorage/inc
    ${CMAKE_SOURCE_DIR}/engine/config/inc/
    ${CMAKE_SOURCE_DIR}/engine/common/errorCode/inc
    ${CMAKE_SOURCE_DIR}/sysi/engine/sysi4loader/inc
    ${CMAKE_SOURCE_DIR}/sysi/engine/sysi4loader/inc/sysi4dpo
    ${CMAKE_SOURCE_DIR}/sysi/engine/sysi4loader/inc/sysi4host
    ${CMAKE_SOURCE_DIR}/sysi/engine/sysi4loader/inc/sysi4trans
    ${CMAKE_SOURCE_DIR}/sysi/engine/sysi4loader/inc/sysi4ucl
    ${CMAKE_SOURCE_DIR}/sysi/engine/sysi4loader/inc/sysi4vdi
    ${CMAKE_SOURCE_DIR}/sysi/engine/sysi4all/inc/

    ${CMAKE_SOURCE_DIR}/soa/inc/public

	${CMAKE_SOURCE_DIR}/foundation/packages/log
    ${CMAKE_SOURCE_DIR}/foundation/packages
    ${CMAKE_SOURCE_DIR}/foundation/platform/os
 
    ${CMAKE_CURRENT_LIST_DIR}/inc/
    ${CMAKE_SOURCE_DIR}/foundation/tools/tracer/inc
    ${CMAKE_SOURCE_DIR}/foundation/packages/dlt
    ${CMAKE_SOURCE_DIR}/foundation/platform/libs/inc
    ${CMAKE_SOURCE_DIR}/foundation/platform/libs/inc/kvstorage/
)


target_link_libraries(
    ${project_name}
	PRIVATE
	${FOUNDATION_LIB_PATH}/libfoundation.so
	#${CMAKE_SOURCE_DIR}/foundation/tools/lib/libtracer_client.so
	${SYSTEM_LIBRARY}
)

