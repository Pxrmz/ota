cmake_minimum_required(VERSION 3.5.0)

# add_subdirectory(iss)
# add_subdirectory(ocs)
# add_subdirectory(vrs)
# add_subdirectory(vss)
# add_subdirectory(irs)
# add_subdirectory(cvs)
# add_subdirectory(ips)
# add_subdirectory(ecs)

cmake_minimum_required(VERSION 3.10)
project(CVS CXX C)

# -Wno-write-strings 忽略c++字符串相关转换过程的编译警告 
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC -g -std=gnu11")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -g -std=c++11 -Wno-write-strings")

if(AIR_PROTOCOL_ENCRYPT)
    ADD_DEFINITIONS(-D AIR_PROTOCOL_ENCRYPT)
endif()

if(AIR_PROTOCOL_SIGN)
    ADD_DEFINITIONS(-D AIR_PROTOCOL_SIGN)
endif()

set(project_name "core_service")

add_library(${project_name} 
    # SHARED 
    adm_engine_common.c
    common/config/src/adm_com_modDefine.c
    common/lib/src/adm_com_hash.c
    common/lib/src/adm_com_lib.c
    common/lib/src/adm_com_parseFileObject.c
    common/kvstorage/src/adm_com_kvstorage.c
    ${SSL_SRC}
    cvs/src/adm_cvs_checkVersion.c  
    cvs/src/adm_cvs_checkVersionImpl.c  
    cvs/src/adm_cvs_versionPolicy.c
    cvs/src/adm_cvs_reportVehicleInfo.c
    cvs/src/adm_cvs_baseInteraction.c
    cvs/src/adm_cvs_license.c
    dpo/src/adm_dpo_download.c
    dps/src/adm_dps_downloadService.c  
    dps/src/adm_dps_kvStorage.c
    dps/src/adm_dps_parsePolicy.c
    dps/src/adm_dps_parsePackageConfig.c
    tps/src/adm_tps_kvStorage.c
    tps/src/adm_tps_transferService.c
    ecs/src/adm_ecs_rsConfig.c
    ecs/src/adm_ecs_kvStorage.c
    ecs/src/adm_ecs_parsePolicy.c
    tps/src/adm_tps_transferService.c  
    ips/src/adm_ips_impl.c
    ips/src/adm_worker.c
    ips/src/adm_workerqueue.c
    irs/src/adm_irs_upload.c
    irs/src/adm_irs_message.c
    irs/src/adm_irs_file.c
    irs/src/adm_irs_common.c
    irs/src/adm_irs_messagehandle.cpp
    iss/src/adm_iss_base64.c  
    iss/src/adm_iss_hmacSha2.c  
    iss/src/adm_iss_textAes.c  
    iss/src/adm_iss_textRsa2048.c
    iss/src/adm_iss_security.c
    ocs/src/adm_ocs_airProtocolSecurity.c
    ocs/src/adm_ocs_airProtocolAdapter.c   
    pis/src/adm_pis_core.c
    vrs/src/adm_vrs_registerInfo.c
    vrs/src/adm_vrs_registerManager.c
    cds/src/adm_cds_compare.c
    cds/src/adm_cds_parse.c
    
    iss/src/adm_se_aes.c
    iss/src/adm_se_aesEnc.c
    iss/src/adm_se_base64.c
    iss/src/adm_se_crypt.c
    iss/src/adm_se_md5.c
)

target_include_directories(
    ${project_name}
    PRIVATE
    ${CMAKE_SOURCE_DIR}/config/inc/
    ${CMAKE_SOURCE_DIR}/engine
    ${CMAKE_SOURCE_DIR}/engine/config/inc/
    ${CMAKE_SOURCE_DIR}/engine/common/config/inc
    ${CMAKE_SOURCE_DIR}/engine/common/errorCode/inc
    ${CMAKE_SOURCE_DIR}/engine/common/lib/inc
    ${CMAKE_SOURCE_DIR}/engine/common/kvstorage/inc
    ${CMAKE_SOURCE_DIR}/engine/common/ssladapter/inc
    ${CMAKE_SOURCE_DIR}/engine/cvs/inc/private
    ${CMAKE_SOURCE_DIR}/engine/cvs/inc/public
    ${CMAKE_SOURCE_DIR}/engine/cvs/src
    ${CMAKE_SOURCE_DIR}/engine/dps/inc/public
    ${CMAKE_SOURCE_DIR}/engine/dpo/inc/public
    ${CMAKE_SOURCE_DIR}/engine/tps/inc/public
    ${CMAKE_SOURCE_DIR}/engine/ecs/inc/public
    ${CMAKE_SOURCE_DIR}/engine/tps/inc/public
    ${CMAKE_SOURCE_DIR}/engine/ips/inc/public
    ${CMAKE_SOURCE_DIR}/engine/ips/src
    ${CMAKE_SOURCE_DIR}/engine/irs/inc/public
    ${CMAKE_SOURCE_DIR}/engine/iss/inc/public
    ${CMAKE_SOURCE_DIR}/engine/ocs/inc/public
    ${CMAKE_SOURCE_DIR}/engine/ocs/src
    ${CMAKE_SOURCE_DIR}/engine/pis/inc/public
    ${CMAKE_SOURCE_DIR}/engine/pis/src
    ${CMAKE_SOURCE_DIR}/engine/vrs/inc/public
    ${CMAKE_SOURCE_DIR}/engine/vrs/inc/private
    ${CMAKE_SOURCE_DIR}/engine/vrs/src
    ${CMAKE_SOURCE_DIR}/engine/vss/inc/public
    ${CMAKE_SOURCE_DIR}/engine/vss/inc/private
    ${CMAKE_SOURCE_DIR}/engine/cds/inc/public
    

    ${CMAKE_SOURCE_DIR}/../foundation_lib/${ENV_PLATFORM}/packages
    ${CMAKE_SOURCE_DIR}/../foundation_lib/${ENV_PLATFORM}/packages/curl
    ${CMAKE_SOURCE_DIR}/../foundation_lib/${ENV_PLATFORM}/packages/json-c
    ${CMAKE_SOURCE_DIR}/../foundation_lib/${ENV_PLATFORM}/packages/log
    ${CMAKE_SOURCE_DIR}/../foundation_lib/${ENV_PLATFORM}/packages/lib
    ${CMAKE_SOURCE_DIR}/../foundation_lib/${ENV_PLATFORM}/packages/lib/hmac

    ${CMAKE_SOURCE_DIR}/../foundation_lib/${ENV_PLATFORM}/platform/libs/inc/kvstorage
    ${CMAKE_SOURCE_DIR}/../foundation_lib/${ENV_PLATFORM}/platform/libs/inc/configToolchain
    ${CMAKE_SOURCE_DIR}/../foundation_lib/${ENV_PLATFORM}/platform/libs/inc
    ${CMAKE_SOURCE_DIR}/../foundation_lib/${ENV_PLATFORM}/platform/os
)

target_link_libraries(
    ${project_name}
    PRIVATE
    ${FOUNDATION_LIB_PATH}/libfoundation.so
    ${SYSTEM_LIBRARY}
)

install(TARGETS ${project_name} RUNTIME LIBRARY DESTINATION "lib")

# 设置库文件和头文件的生成路径
install(FILES "${CMAKE_SOURCE_DIR}/engine/adm_engine_common.h" 
    DESTINATION .)
install(DIRECTORY "${CMAKE_SOURCE_DIR}/engine/cds/inc/public/" 
    DESTINATION cds/inc/public
    PATTERN ".h" EXCLUDE)
install(DIRECTORY "${CMAKE_SOURCE_DIR}/engine/common/config/inc/" 
    DESTINATION common/config/inc
    PATTERN ".h" EXCLUDE)
#install(DIRECTORY "${CMAKE_SOURCE_DIR}/engine/common/config/src/" 
#    DESTINATION common/config/src
#    PATTERN ".c" EXCLUDE)
install(DIRECTORY "${CMAKE_SOURCE_DIR}/engine/common/errorCode/inc/" 
    DESTINATION common/errorCode/inc
    PATTERN ".h" EXCLUDE)
install(DIRECTORY "${CMAKE_SOURCE_DIR}/engine/common/lib/inc/" 
    DESTINATION common/lib/inc
    PATTERN ".h" EXCLUDE)
install(DIRECTORY "${CMAKE_SOURCE_DIR}/engine/common/kvstorage/inc/" 
    DESTINATION common/kvstorage/inc
    PATTERN ".h" EXCLUDE)
install(DIRECTORY "${CMAKE_SOURCE_DIR}/engine/common/ssladapter/inc/" 
    DESTINATION common/ssladapter/inc
    PATTERN ".h" EXCLUDE)
install(DIRECTORY "${CMAKE_SOURCE_DIR}/engine/cvs/inc/public/" 
    DESTINATION cvs/inc/public
    PATTERN ".h" EXCLUDE)    
install(DIRECTORY "${CMAKE_SOURCE_DIR}/engine/dpo/inc/public/" 
    DESTINATION dpo/inc/public
    PATTERN ".h" EXCLUDE)    
install(DIRECTORY "${CMAKE_SOURCE_DIR}/engine/dps/inc/public/" 
    DESTINATION dps/inc/public
    PATTERN ".h" EXCLUDE)    
install(DIRECTORY "${CMAKE_SOURCE_DIR}/engine/ecs/inc/public/" 
    DESTINATION ecs/inc/public
    PATTERN ".h" EXCLUDE)    
install(DIRECTORY "${CMAKE_SOURCE_DIR}/engine/ips/inc/public/" 
    DESTINATION ips/inc/public
    PATTERN ".h" EXCLUDE)    
install(DIRECTORY "${CMAKE_SOURCE_DIR}/engine/irs/inc/public/" 
    DESTINATION irs/inc/public
    PATTERN ".h" EXCLUDE)    
install(DIRECTORY "${CMAKE_SOURCE_DIR}/engine/iss/inc/public/" 
    DESTINATION iss/inc/public
    PATTERN ".h" EXCLUDE)    
install(DIRECTORY "${CMAKE_SOURCE_DIR}/engine/ocs/inc/public/" 
    DESTINATION ocs/inc/public
    PATTERN ".h" EXCLUDE)   
install(DIRECTORY "${CMAKE_SOURCE_DIR}/engine/pis/inc/public/" 
    DESTINATION pis/inc/public
    PATTERN ".h" EXCLUDE) 
install(DIRECTORY "${CMAKE_SOURCE_DIR}/engine/tps/inc/public/" 
    DESTINATION tps/inc/public
    PATTERN ".h" EXCLUDE) 
install(DIRECTORY "${CMAKE_SOURCE_DIR}/engine/vrs/inc/public/" 
    DESTINATION vrs/inc/public
    PATTERN ".h" EXCLUDE) 
install(DIRECTORY "${CMAKE_SOURCE_DIR}/engine/vss/inc/public/" 
    DESTINATION vss/inc/public
    PATTERN ".h" EXCLUDE) 
install(DIRECTORY "${CMAKE_SOURCE_DIR}/engine/config/inc/" 
    DESTINATION config/inc
    PATTERN ".h" EXCLUDE) 


# 设置Doxygen变量用于生成System interface service user manual
set(DOXYGEN_GENERATE_HTML YES)
set(DOXYGEN_GENERATE_MAN NO)

set(DOXYGEN_PROJECT_NAME "VUS core service API user manual")
set(DOXYGEN_PROJECT_NUMBER "V0.0.1")
set(DOXYGEN_PROJECT_BRIEF "VUS核心服务的应用接口用户手册")
set(DOXYGEN_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build/engine/core_service_API_user_manual)
set(DOXYGEN_INDEX_FILE ${DOXYGEN_OUTPUT_DIRECTORY}/index.html)


# Interface User Manual
doxygen_add_docs(
    core_service_API_user_manual
    ${CMAKE_SOURCE_DIR}/../foundation_lib/${ENV_PLATFORM}/platform/os/adm_typedefs.h
    ${CMAKE_SOURCE_DIR}/engine/adm_engine_common.h
    ${CMAKE_SOURCE_DIR}/engine/common/config/inc/adm_com_modDefine.h
    ${CMAKE_SOURCE_DIR}/engine/common/errorCode/inc/adm_com_errorCode.h
    ${CMAKE_SOURCE_DIR}/engine/common/lib/inc/adm_com_hash.h
    ${CMAKE_SOURCE_DIR}/engine/common/lib/inc/adm_com_lib.h
    ${CMAKE_SOURCE_DIR}/engine/common/lib/inc/adm_com_parseFileObject.h
    ${CMAKE_SOURCE_DIR}/engine/common/kvstorage/inc/adm_com_kvstorage.h
    ${CMAKE_SOURCE_DIR}/engine/common/ssladapter/inc/adm_com_sslAdapter.h
    ${CMAKE_SOURCE_DIR}/engine/cds/inc/public/adm_cds_compare.h
    ${CMAKE_SOURCE_DIR}/engine/cvs/inc/public/adm_cvs_checkVersion.h
    ${CMAKE_SOURCE_DIR}/engine/cvs/inc/public/adm_cvs_versionPolicy.h
    ${CMAKE_SOURCE_DIR}/engine/dpo/inc/public/adm_dpo_download.h
    ${CMAKE_SOURCE_DIR}/engine/dps/inc/public/adm_dps_downloadService.h
    ${CMAKE_SOURCE_DIR}/engine/dps/inc/public/adm_dps_parsePackageConfig.h
    ${CMAKE_SOURCE_DIR}/engine/dps/inc/public/adm_dps_parsePolicy.h
    ${CMAKE_SOURCE_DIR}/engine/ecs/inc/public/adm_ecs_kvStorage.h
    ${CMAKE_SOURCE_DIR}/engine/ecs/inc/public/adm_ecs_parsePolicy.h
    ${CMAKE_SOURCE_DIR}/engine/ecs/inc/public/adm_ecs_rsConfig.h
    ${CMAKE_SOURCE_DIR}/engine/ips/inc/public/adm_ips_install.h
    ${CMAKE_SOURCE_DIR}/engine/irs/inc/public/adm_irs_upload.h
    ${CMAKE_SOURCE_DIR}/engine/iss/inc/public/adm_iss_base64.h
    ${CMAKE_SOURCE_DIR}/engine/iss/inc/public/adm_iss_hmacSha2.h
    ${CMAKE_SOURCE_DIR}/engine/iss/inc/public/adm_iss_textAes.h
    ${CMAKE_SOURCE_DIR}/engine/iss/inc/public/adm_iss_textRsa2048.h
    ${CMAKE_SOURCE_DIR}/engine/iss/inc/public/adm_iss_security.h
    ${CMAKE_SOURCE_DIR}/engine/pis/inc/public/adm_pis_package.h
    ${CMAKE_SOURCE_DIR}/engine/tps/inc/public/adm_tps_transferService.h
    ${CMAKE_SOURCE_DIR}/engine/vrs/inc/public/adm_vrs_registerManager.h
    ${CMAKE_SOURCE_DIR}/engine/vss/inc/public/adm_vss_stateManager.h
    ${CMAKE_SOURCE_DIR}/engine/vss/inc/public/adm_vss_stateVerify.h
    COMMENT "VUS core service API user manual generate"
    )

add_subdirectory(config)
add_subdirectory(common)

