cmake_minimum_required(VERSION 3.10)
project(CVS CXX C)

ADD_DEFINITIONS(-DADM_SDK_VERSION="V1.0.1")

ADD_DEFINITIONS(-D CMAKE_CUSTOM_EWEBSERVER_SUPPORT)


set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC -g -std=gnu11 ")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -g  -fpermissive -std=c++11 -Wno-write-strings")

set(project_name "irmGtest")

set(ROOT_SOURCE_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/../../..)
message(${ROOT_SOURCE_DIRECTORY})

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wl,--wrap,adm_ecs_getEventReportCycle")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wl,--wrap,adm_ocs_otaStatusCodeTransfer")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wl,--wrap,adm_ocs_uploadProtocolParse")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wl,--wrap,adm_ecs_getLogReportCycle")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wl,--wrap,adm_ecs_getIsLogFileEncrypted")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wl,--wrap,adm_ocs_eventProtocolAdapter")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wl,--wrap,adm_ocs_vehicleProgressProtocolAdapter")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wl,--wrap,adm_ocs_ecuStateProtocolAdapter")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wl,--wrap,adm_ocs_vehicleStateProtocolAdapter")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wl,--wrap,adm_ocs_strSign")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wl,--wrap,adm_tpm_CollectLogFromSysiToHost")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wl,--wrap,FILE_AES_Encrypt")

add_subdirectory(./config/parsejson parsejson)

include_directories(
    ${ROOT_SOURCE_DIRECTORY}/engine/config/inc/
    ${ROOT_SOURCE_DIRECTORY}/engine/common/config/inc/
    ${ROOT_SOURCE_DIRECTORY}/application/cvm/inc/public
    ${ROOT_SOURCE_DIRECTORY}/application/ocm/inc/
    ${ROOT_SOURCE_DIRECTORY}/foundation/platform/os
    ${ROOT_SOURCE_DIRECTORY}/foundation/platform/libs/inc/kvstorage/
    ${ROOT_SOURCE_DIRECTORY}/foundation/packages/log
    ${ROOT_SOURCE_DIRECTORY}/foundation/packages
    ${ROOT_SOURCE_DIRECTORY}/engine/common/errorCode/inc
    ${ROOT_SOURCE_DIRECTORY}/engine/
    ${ROOT_SOURCE_DIRECTORY}/engine/common/lib/inc
    ${ROOT_SOURCE_DIRECTORY}/engine/common/kvstorage/inc
    ${ROOT_SOURCE_DIRECTORY}/application/irm/inc/public/
    ${ROOT_SOURCE_DIRECTORY}/application/irm/gtest/inc/
    ${ROOT_SOURCE_DIRECTORY}/application/irm/src/
    ${ROOT_SOURCE_DIRECTORY}/application
    ${ROOT_SOURCE_DIRECTORY}/engine/cvs/inc/public
    ${ROOT_SOURCE_DIRECTORY}/engine/ocs/inc/public
    ${ROOT_SOURCE_DIRECTORY}/engine/ecs/inc/public
    ${ROOT_SOURCE_DIRECTORY}/engine/irs/inc/public
    ${ROOT_SOURCE_DIRECTORY}/engine/iss/inc/public
    ${ROOT_SOURCE_DIRECTORY}/application/tpm/inc/public/
    ${ROOT_SOURCE_DIRECTORY}/dim/inc/public/
    ${ROOT_SOURCE_DIRECTORY}/oca/inc/
)

add_executable(
	${project_name}
	${ROOT_SOURCE_DIRECTORY}/application/irm/gtest/adm_irm_gtest.cc
	${ROOT_SOURCE_DIRECTORY}/application/irm/src/adm_irm_logFile.c
	${ROOT_SOURCE_DIRECTORY}/application/irm/src/adm_irm_report.c
)

set(SYSTEM_LIBRARY pthread rt dl stdc++)
set(SSL_LIBRARY crypto ssl)

if(${CUSTOM_CMAKE_SYSTEM_NAME} MATCHES "QNX7")
    set(GTEST_LIB "")
else()
    set(GTEST_LIB 
        ${ROOT_SOURCE_DIRECTORY}/foundation/packages/libs/libgtestd.a
        ${ROOT_SOURCE_DIRECTORY}/foundation/packages/libs/libgtest_maind.a
    )
endif()

target_link_libraries( ${project_name}
	gtestparsejson
    ${ROOT_SOURCE_DIRECTORY}/foundation/packages/libs/libfoundation.so
    ${ROOT_SOURCE_DIRECTORY}/engine/lib/libcore_service.a  
	${ROOT_SOURCE_DIRECTORY}/engine/lib/libconfig.a
    ${GTEST_LIB}
    
    ${SYSTEM_LIBRARY}
)

