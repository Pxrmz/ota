cmake_minimum_required(VERSION 3.10)
project(DPM CXX C)

ADD_DEFINITIONS(-DADM_SDK_VERSION="V1.0.1")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC -g -std=gnu11 ")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -g  -fpermissive -std=c++11 -Wno-write-strings")

set(project_name "dpmgtest")

set(ROOT_SOURCE_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/../../..)
message(${ROOT_SOURCE_DIRECTORY})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wl,--wrap,adm_cvs_getVersionPolicy")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wl,--wrap,adm_dim_host_getNetworkType")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wl,--wrap,adm_dim_host_verifySpaceIsEnough")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wl,--wrap,adm_dim_ucl_verifySpaceIsEnouth")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wl,--wrap,adm_cdm_LoopVerify")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wl,--wrap,adm_cdm_LoopFinish")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wl,--wrap,adm_cdm_verify_Compare")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wl,--wrap,adm_ocm_fileDownloadInterface")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wl,--wrap,adm_irm_reportEvent")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wl,--wrap,adm_dim_dpo_cleanDownloadDir")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wl,--wrap,adm_eim_getVehicleBigVersion")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wl,--wrap,adm_irm_reportVehicleState")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wl,--wrap,adm_vsm_getHmiOnline")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wl,--wrap,adm_uim_hmi_conds_downloadResult_RR")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wl,--wrap,adm_uim_hmi_ecu_downloadResult_RR")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wl,--wrap,adm_uim_hmi_policy_downloadResult_RR")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wl,--wrap,adm_irm_syncReportVehicleState")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wl,--wrap,adm_dim_dpo_requestRemoteDownloadPackage")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wl,--wrap,adm_pub_downPackageDecrypt")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wl,--wrap,adm_dim_host_verifySignature")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wl,--wrap,adm_pub_downPackageVerifySign")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wl,--wrap,adm_ecs_getDownloadInstanceId")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wl,--wrap,adm_pis_packageImageSet")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wl,--wrap,adm_pis_packageSecuritySet")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wl,--wrap,adm_tpm_triggerTransferPkg")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wl,--wrap,adm_uim_policyDownloadProgressNotify")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wl,--wrap,adm_uim_ecuDownloadProgressNotify")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wl,--wrap,adm_uim_allEcuDownloadProgressNotify")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wl,--wrap,adm_uim_hmi_allecu_downloadResult_RR")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wl,--wrap,adm_cdm_verify_free")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wl,--wrap,adm_vsm_triggerDownload")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wl,--wrap,adm_vsm_hmiIsAutoDownload")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wl,--wrap,adm_oca_interuptDownload")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wl,--wrap,adm_irm_getEventSequenceId")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wl,--wrap,adm_irm_reportEcuState")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wl,--wrap,adm_irm_reportVehicleProgress")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wl,--wrap,adm_getSdkVersion")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wl,--wrap,adm_ecs_getEventReportLevel")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wl,--wrap,adm_ecs_getOneEcuDidListInfo")

#add_subdirectory(./parsejson parsejson)

include_directories(
    ${ROOT_SOURCE_DIRECTORY}/application/dpm/src/
    ${ROOT_SOURCE_DIRECTORY}/engine/config/inc/
    ${ROOT_SOURCE_DIRECTORY}/engine
    ${ROOT_SOURCE_DIRECTORY}/engine/common/config/inc
    ${ROOT_SOURCE_DIRECTORY}/engine/common/errorCode/inc
    ${ROOT_SOURCE_DIRECTORY}/engine/common/lib/inc
    ${ROOT_SOURCE_DIRECTORY}/engine/common/kvstorage/inc
    ${ROOT_SOURCE_DIRECTORY}/engine/cvs/inc/public
    ${ROOT_SOURCE_DIRECTORY}/application
    ${ROOT_SOURCE_DIRECTORY}/dim/inc/public
    ${ROOT_SOURCE_DIRECTORY}/application/cvm/inc/public
    ${ROOT_SOURCE_DIRECTORY}/application/ocm/inc/
    ${ROOT_SOURCE_DIRECTORY}/application/vrm/inc
    ${ROOT_SOURCE_DIRECTORY}/application/vsm/inc
    ${ROOT_SOURCE_DIRECTORY}/application/pub/inc
    ${ROOT_SOURCE_DIRECTORY}/application/eim/inc/public
    ${ROOT_SOURCE_DIRECTORY}/application/irm/inc/public
    ${ROOT_SOURCE_DIRECTORY}/application/dpm/inc/public
    ${ROOT_SOURCE_DIRECTORY}/application/dpm/dpmgtest
    ${ROOT_SOURCE_DIRECTORY}/application/ecm/inc/
    ${ROOT_SOURCE_DIRECTORY}/soa/inc/public
    ${ROOT_SOURCE_DIRECTORY}/uim/inc/public
    ${ROOT_SOURCE_DIRECTORY}/application/cdm/inc/public
    ${ROOT_SOURCE_DIRECTORY}/engine/cds/inc/public
    ${ROOT_SOURCE_DIRECTORY}/engine/dps/inc/public
    ${ROOT_SOURCE_DIRECTORY}/engine/dpo/inc/public
    ${ROOT_SOURCE_DIRECTORY}/engine/ecs/inc/public
    ${ROOT_SOURCE_DIRECTORY}/engine/irs/inc/public
    ${ROOT_SOURCE_DIRECTORY}/engine/iss/inc/public
    ${ROOT_SOURCE_DIRECTORY}/engine/ocs/inc/public
    ${ROOT_SOURCE_DIRECTORY}/engine/pis/inc/public
    ${ROOT_SOURCE_DIRECTORY}/engine/tps/inc/public
    ${ROOT_SOURCE_DIRECTORY}/engine/vrs/inc/public
    ${ROOT_SOURCE_DIRECTORY}/engine/vrs/inc/private
    ${ROOT_SOURCE_DIRECTORY}/engine/vss/inc/public

    ${ROOT_SOURCE_DIRECTORY}/oca/inc/

    ${ROOT_SOURCE_DIRECTORY}/foundation/packages
    ${ROOT_SOURCE_DIRECTORY}/foundation/packagescrypt
    ${ROOT_SOURCE_DIRECTORY}/foundation/packages/openssl
    ${ROOT_SOURCE_DIRECTORY}/foundation/packages/curl
    ${ROOT_SOURCE_DIRECTORY}/foundation/packages/json-c

    ${ROOT_SOURCE_DIRECTORY}/foundation/packages/log
    ${ROOT_SOURCE_DIRECTORY}/foundation/packages/lib
    ${ROOT_SOURCE_DIRECTORY}/foundation/packages/lib/hmac

    ${ROOT_SOURCE_DIRECTORY}/foundation/platform/libs/inc/kvstorage
    ${ROOT_SOURCE_DIRECTORY}/foundation/platform/libs/inc
    ${ROOT_SOURCE_DIRECTORY}/foundation/platform/os
    ${CUSTOM_SUPPORT_HEADERS}
)

add_executable(
    ${project_name}
    ${ROOT_SOURCE_DIRECTORY}/application/dpm/dpmgtest/adm_integration_test.c
    ${ROOT_SOURCE_DIRECTORY}/application/dpm/dpmgtest/adm_dpm_management_unittest.cc
    ${ROOT_SOURCE_DIRECTORY}/application/dpm/src/adm_dpm_uim.c
    ${ROOT_SOURCE_DIRECTORY}/application/dpm/src/adm_dpm_report.c
    ${ROOT_SOURCE_DIRECTORY}/application/dpm/src/adm_dpm_download.c
    ${ROOT_SOURCE_DIRECTORY}/application/dpm/src/adm_dpm_reportCloud.c
    ${ROOT_SOURCE_DIRECTORY}/application/dpm/src/adm_dpm_downloadManagement.c
    
)

set(SYSTEM_LIBRARY pthread rt dl stdc++)
#set(SSL_LIBRARY crypto ssl)

if(${CUSTOM_CMAKE_SYSTEM_NAME} MATCHES "QNX7")
    set(GTEST_LIB "")
else()
    set(GTEST_LIB 
        ${ROOT_SOURCE_DIRECTORY}/foundation/packages/libs/libgtestd.a
        ${ROOT_SOURCE_DIRECTORY}/foundation/packages/libs/libgtest_maind.a
    )
endif()

target_link_libraries( ${project_name}
    ${ROOT_SOURCE_DIRECTORY}/foundation/packages/libs/libfoundation.so
    ${ROOT_SOURCE_DIRECTORY}/engine/lib/libcore_service.a  
    ${ROOT_SOURCE_DIRECTORY}/engine/lib/libconfig.a
    ${ROOT_SOURCE_DIRECTORY}/foundation/packages/libs/libjsoncpp.a
    ${ROOT_SOURCE_DIRECTORY}/application/dpm/dpmgtest/libs/libadmoca.so
    ${ROOT_SOURCE_DIRECTORY}/application/dpm/dpmgtest/libs/libocm.a
    ${GTEST_LIB}
    ${SYSTEM_LIBRARY}
    #-lstdc++
)

