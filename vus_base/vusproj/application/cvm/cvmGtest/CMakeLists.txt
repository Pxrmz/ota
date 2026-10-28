cmake_minimum_required(VERSION 3.10)
project(CVS CXX C)

ADD_DEFINITIONS(-DADM_SDK_VERSION="V1.0.1")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC -g -std=gnu11 ")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -g  -fpermissive -std=c++11 -Wno-write-strings")

set(project_name "cvmGtest")

set(ROOT_SOURCE_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/../../..)
message(${ROOT_SOURCE_DIRECTORY})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wl,--wrap,adm_ecs_getRsConfigVersion")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wl,--wrap,adm_eim_getVehicleBigVersion")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wl,--wrap,adm_ocs_newCheckRespAdapterBody")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wl,--wrap,adm_ocs_newCheckReqAdapterBody")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wl,--wrap,adm_ocs_checkProtocolAdapter")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wl,--wrap,adm_ocs_checkProtocolParse")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wl,--wrap,adm_ocm_businessCommInterface")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wl,--wrap,adm_dpo_doDownloadFile")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wl,--wrap,adm_ecm_decryptAndVerificationSignature")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wl,--wrap,adm_uim_checkResultNotify")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wl,--wrap,adm_uim_hmi_checkResult_RR")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wl,--wrap,adm_sdk_getAllModulesVersionFormatJsonStr")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wl,--wrap,adm_ocm_fileDownloadInterface")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wl,--wrap,adm_irm_getEventSequenceId")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wl,--wrap,adm_eim_getEcuInfoManagerKvGetStr")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wl,--wrap,adm_eim_getEcuInfoManagerDimGetStr")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wl,--wrap,adm_uim_checkProgressNotify")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wl,--wrap,adm_irm_reportVehicleState")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wl,--wrap,adm_irm_reportEvent")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wl,--wrap,adm_vsm_triggerCheck")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wl,--wrap,adm_ecs_getEventReportLevel")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wl,--wrap,adm_ocs_checkProtocolHmiMember")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wl,--wrap,adm_ocs_otaStatusCodeTransfer")


include_directories(
    ${ROOT_SOURCE_DIRECTORY}/engine/config/inc/
    ${ROOT_SOURCE_DIRECTORY}/engine/common/config/inc/
    ${ROOT_SOURCE_DIRECTORY}/application/cvm/inc/public/
    ${ROOT_SOURCE_DIRECTORY}/application/ocm/inc/
    ${ROOT_SOURCE_DIRECTORY}/application/ecm/inc/
    ${ROOT_SOURCE_DIRECTORY}/foundation/platform/os
    ${ROOT_SOURCE_DIRECTORY}/foundation/platform/libs/inc/kvstorage/
    ${ROOT_SOURCE_DIRECTORY}/foundation/packages/log/
    ${ROOT_SOURCE_DIRECTORY}/foundation/packages/
    ${ROOT_SOURCE_DIRECTORY}/engine/common/errorCode/inc/
    ${ROOT_SOURCE_DIRECTORY}/engine/
    ${ROOT_SOURCE_DIRECTORY}/engine/common/lib/inc/
    ${ROOT_SOURCE_DIRECTORY}/engine/common/kvstorage/inc
    ${ROOT_SOURCE_DIRECTORY}/application/irm/inc/public/
    # ${ROOT_SOURCE_DIRECTORY}/application/irm/inc/
    # ${ROOT_SOURCE_DIRECTORY}/application/irm/src/
    # ${ROOT_SOURCE_DIRECTORY}/application
    ${ROOT_SOURCE_DIRECTORY}/engine/cvs/inc/public
    ${ROOT_SOURCE_DIRECTORY}/engine/ocs/inc/public/
    ${ROOT_SOURCE_DIRECTORY}/engine/dpo/inc/public/
    ${ROOT_SOURCE_DIRECTORY}/engine/ecs/inc/public/
    ${ROOT_SOURCE_DIRECTORY}/engine/irs/inc/public
    # ${ROOT_SOURCE_DIRECTORY}/engine/iss/inc/public
    ${ROOT_SOURCE_DIRECTORY}/oca/inc/
    ${ROOT_SOURCE_DIRECTORY}/application/
    ${ROOT_SOURCE_DIRECTORY}/application/cvm/cvmGtest/
    ${ROOT_SOURCE_DIRECTORY}/application/vsm/
    ${ROOT_SOURCE_DIRECTORY}/application/vsm/inc/
    ${ROOT_SOURCE_DIRECTORY}/application/eim/inc/public/
    ${ROOT_SOURCE_DIRECTORY}/application/sdk/inc/
    ${ROOT_SOURCE_DIRECTORY}/uim/inc/public/
    ${ROOT_SOURCE_DIRECTORY}/dim/inc/public/
)

add_executable(
	${project_name}
	${ROOT_SOURCE_DIRECTORY}/application/cvm/cvmGtest/adm_cvm_gtest.cc
	${ROOT_SOURCE_DIRECTORY}/application/cvm/cvmGtest/adm_cvm_parseJson.cc
	${ROOT_SOURCE_DIRECTORY}/application/cvm/src/adm_cvm_checkVersion.c
	${ROOT_SOURCE_DIRECTORY}/application/cvm/src/adm_cvm_uim.c
	${ROOT_SOURCE_DIRECTORY}/application/cvm/src/adm_cvm_event.c
)

set(SYSTEM_LIBRARY pthread rt dl stdc++)

if(${CUSTOM_CMAKE_SYSTEM_NAME} MATCHES "QNX7")
    set(GTEST_LIB "")
else()
    set(GTEST_LIB 
        ${ROOT_SOURCE_DIRECTORY}/foundation/packages/libs/libgtest_maind.a
        ${ROOT_SOURCE_DIRECTORY}/foundation/packages/libs/libgtestd.a
    )
endif()

target_link_libraries( ${project_name}
    ${ROOT_SOURCE_DIRECTORY}/foundation/packages/libs/libfoundation.so
    ${ROOT_SOURCE_DIRECTORY}/engine/lib/libcore_service.a  
	${ROOT_SOURCE_DIRECTORY}/engine/lib/libconfig.a
    ${GTEST_LIB}
    ${SYSTEM_LIBRARY}
)

