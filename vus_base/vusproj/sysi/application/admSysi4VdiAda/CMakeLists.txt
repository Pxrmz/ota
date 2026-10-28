cmake_minimum_required(VERSION 3.10)
project(SYSI4VDIADA CXX C)

set(project_name "sysi4vdiada")

add_library(${project_name}
    STATIC
     ./src/adm_sysi4vdi_handler.cc
)

target_include_directories(
    ${project_name}
    PRIVATE
    ${CMAKE_SOURCE_DIR}/sysi/engine/sysi4all/inc
    ${CMAKE_SOURCE_DIR}/sysi/engine/sysi4loader/inc
    ${CMAKE_SOURCE_DIR}/sysi/engine/sysiInit/inc
    ${CMAKE_SOURCE_DIR}/sysi/engine/sysi4loader/inc/sysi4ucl
    ${CMAKE_SOURCE_DIR}/sysi/engine/sysi4loader/inc/sysi4trans
    ${CMAKE_SOURCE_DIR}/sysi/engine/sysi4loader/inc/sysi4vdi
    ${CMAKE_SOURCE_DIR}/sysi/engine/sysi4loader/inc/sysi4host
    ${CMAKE_SOURCE_DIR}/sysi/engine/sysi4loader/inc/sysi4dpo
    
    ${CMAKE_SOURCE_DIR}/sysi/application/admSysi4cbf/cbf-parse/cbf_parse_lib/inc
    ${CMAKE_SOURCE_DIR}/sysi/application/admSysi4cbf/inc
    
    ${CMAKE_SOURCE_DIR}/foundation/platform/os
    ${CMAKE_SOURCE_DIR}/engine/common/errorCode/inc
    ${CMAKE_SOURCE_DIR}/engine/common/config/inc
    ${CMAKE_SOURCE_DIR}/engine/config/inc
    ${CMAKE_SOURCE_DIR}/engine/common/lib/inc
    
    ${CMAKE_CURRENT_LIST_DIR}/inc
    ${CMAKE_SOURCE_DIR}/foundation/packages/log
    ${CMAKE_SOURCE_DIR}/foundation/tools/tracer/inc
    ${CMAKE_SOURCE_DIR}/foundation/packages/dlt
    
    ${CMAKE_SOURCE_DIR}/foundation/packages
)

if(${CUSTOM_CMAKE_SYSTEM_NAME} MATCHES "Android")
    file(REMOVE ${CMAKE_CURRENT_LIST_DIR}/vdilib/libabupvdi.so)
    file(COPY ${CMAKE_CURRENT_LIST_DIR}/vdilib/android/libabupvdi.so DESTINATION ${CMAKE_CURRENT_LIST_DIR}/vdilib)
elseif(${CUSTOM_CMAKE_SYSTEM_NAME} MATCHES "QNX7")
    file(REMOVE ${CMAKE_CURRENT_LIST_DIR}/vdilib/libabupvdi.so)
    # file(REMOVE ${CMAKE_CURRENT_LIST_DIR}/vdilib/libdoipclient_adapter.so)
    file(REMOVE ${CMAKE_CURRENT_LIST_DIR}/vdilib/libota_lion.so)
    file(REMOVE ${CMAKE_CURRENT_LIST_DIR}/vdilib/libsvp_basic.so)
    file(COPY ${CMAKE_CURRENT_LIST_DIR}/vdilib/qnx7/libabupvdi.so DESTINATION ${CMAKE_CURRENT_LIST_DIR}/vdilib)
    # file(COPY ${CMAKE_CURRENT_LIST_DIR}/vdilib/qnx7/libdoipclient_adapter.so DESTINATION ${CMAKE_CURRENT_LIST_DIR}/vdilib)
    file(COPY ${CMAKE_CURRENT_LIST_DIR}/vdilib/qnx7/libota_lion.so DESTINATION ${CMAKE_CURRENT_LIST_DIR}/vdilib)
    file(COPY ${CMAKE_CURRENT_LIST_DIR}/vdilib/qnx7/libsvp_basic.so DESTINATION ${CMAKE_CURRENT_LIST_DIR}/vdilib)
else()
    file(REMOVE ${CMAKE_CURRENT_LIST_DIR}/vdilib/libabupvdi.so)
    file(COPY ${CMAKE_CURRENT_LIST_DIR}/vdilib/x86/libabupvdi.so DESTINATION ${CMAKE_CURRENT_LIST_DIR}/vdilib)
endif()

target_link_libraries(
    ${project_name}
    PRIVATE
    ${FOUNDATION_LIB_PATH}/libfoundation.so
    ${SSL_LIBRARY}
    ${SYSTEM_LIBRARY}
)

