cmake_minimum_required(VERSION 3.10)
project(sysi4deploy CXX C)

SET(project_name "sysi4deploy")
SET(CMAKE_BUILD_TYPE "Debug")

add_definitions(-fPIC)

if(CUSTOM_HMI_CLIENT_SUPPORT)
    if(CUSTOME_USE_DLSYM)
        set(SYSI4HMI_CLIENT_LIB
        ${CMAKE_SOURCE_DIR}/build/sysi/engine/sysi4hmi/libsysi4hmi.a
        ${CMAKE_SOURCE_DIR}/foundation/arpc/lib/libarpc.so
        )
    else()
        set(SYSI4HMI_CLIENT_LIB 
        ${CMAKE_SOURCE_DIR}/build/sysi/engine/sysi4hmi/libsysi4hmi.a
         )
    endif()
else()
    set(SYSI4HMI_CLIENT_LIB "")
endif()

add_library(
    ${project_name}
    SHARED
    ${CMAKE_SOURCE_DIR}/sysi/application/admSysi4DeployApp/src/adm_sysideploy_libarary.c
)

target_include_directories(
    ${project_name}
    PRIVATE
    ${CMAKE_SOURCE_DIR}/engine/sysiInit/inc
    #${CMAKE_SOURCE_DIR}/engine/sysi4all/inc
    ${CMAKE_SOURCE_DIR}/engine/sysi4hmi/inc
    ${CMAKE_SOURCE_DIR}/foundation/packages/log
    ${CMAKE_SOURCE_DIR}/foundation/platform/os
    ${CMAKE_SOURCE_DIR}/foundation/packages/
    ${CMAKE_SOURCE_DIR}/foundation/platform/libs/inc
    ${CMAKE_SOURCE_DIR}/engine/common/config/inc
    ${CMAKE_SOURCE_DIR}/engine/common/config/inc
    ${CMAKE_SOURCE_DIR}/engine/config/inc
    ${CMAKE_SOURCE_DIR}/foundation/platform/libs/include
    ${CMAKE_SOURCE_DIR}/soa/inc/public
    ${CMAKE_SOURCE_DIR}/engine/common/errorCode/inc
    ${CMAKE_SOURCE_DIR}/engine/common/lib/inc
    ${CMAKE_SOURCE_DIR}/foundation/arpc/inc
    ${CMAKE_SOURCE_DIR}/soa/inc/public
    ${CMAKE_CURRENT_LIST_DIR}/inc
    )

message("project_name:${project_name}, CUSTOM_IMPORT_BASE_FULL_STATIC_LIB:${CUSTOM_IMPORT_BASE_FULL_STATIC_LIB}")

if(CUSTOM_IMPORT_BASE_FULL_STATIC_LIB)
target_link_libraries(
    ${project_name}
    PRIVATE
    -Wl,--whole-archive
    ${SYSI4HMI_CLIENT_LIB}

    ${CMAKE_SOURCE_DIR}/build/sysi/engine/sysiInit/libsysiInit.a
    ${CMAKE_SOURCE_DIR}/build/externalLib/lib/libconfig.a
    ${CMAKE_SOURCE_DIR}/build/externalLib/lib/libcore_service.a

    ${SYSTEM_LIBRARY}
    ${FOUNDATION_LIBRARY}
    ${SSL_LIBRARY}

    ${SYSILOADER_LIBRARY}
    ${SOA_LIBRARY}
    -Wl,--no-whole-archive
    
    )
else()
target_link_libraries(
    ${project_name}
    PRIVATE
    -Wl,--whole-archive
    ${SYSI4HMI_CLIENT_LIB}

    ${CMAKE_SOURCE_DIR}/build/sysi/engine/sysiInit/libsysiInit.a
    ${CMAKE_SOURCE_DIR}/build/externalLib/lib/libconfig.a
    ${CMAKE_SOURCE_DIR}/build/externalLib/lib/libcore_service.a
    
    ${SYSILOADER_LIBRARY}
    ${SOA_LIBRARY}
    ${FOUNDATION_LIB_PATH}/libfoundation.so
    #${SSL_LIBRARY}
    #${SYSTEM_LIBRARY}
    -Wl,--no-whole-archive
    )
endif()

#因为jni原因，此处不能编译执行程序
#add_subdirectory(sample)
