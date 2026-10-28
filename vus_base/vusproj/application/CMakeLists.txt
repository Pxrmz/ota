cmake_minimum_required(VERSION 3.5.0)

if (SUPPORT_LION_PKI)
    ADD_DEFINITIONS(-D SUPPORT_LION_PKI)
endif()

#远程上下电
if (SUPPORT_REMOTE_AUTH)
    ADD_DEFINITIONS(-D SUPPORT_REMOTE_AUTH)
endif()

if (AIR_PROTOCOL_VEHICLE_VERSION)
    ADD_DEFINITIONS(-DAIR_PROTOCOL_VEHICLE_VERSION="hmi_V1.0")
endif()

add_subdirectory(pub)
# compilation clipping
message("=========== add_sub CUSTOM_EWEBSERVER_SUPPORT = ${CUSTOM_EWEBSERVER_SUPPORT}")
if(CUSTOM_EWEBSERVER_SUPPORT)
    add_subdirectory(tpm)
endif()
add_subdirectory(vsm)
add_subdirectory(sdk)
add_subdirectory(ocm)
add_subdirectory(cdm)
add_subdirectory(vrm)
add_subdirectory(ipm)
add_subdirectory(irm)
add_subdirectory(ecm)
add_subdirectory(ems)
add_subdirectory(eim)
add_subdirectory(cvm)
add_subdirectory(dpm)

#主控是否编译成库或者进程
if(COMPILE_UCM_APK_SUPPORT)
	add_subdirectory(ucmCore)
	add_subdirectory(ucmJni)
else()
	add_subdirectory(appcore)
endif()
# add_subdirectory(combineSample)
