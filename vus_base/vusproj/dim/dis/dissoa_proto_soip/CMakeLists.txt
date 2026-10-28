cmake_minimum_required (VERSION 3.10)
#project(Dim CXX C)

set(project_name "disprotosoip")

#add_definitions(-D NO_SOMEIP_LOG)


set(
	DIM_SRC
    src/adm_dis_vdi_proto_soip.cc
    src/adm_dis_vdi_event_proto_soip.cc

	src/adm_dis_ucl_proto_soip.cc
    src/adm_dis_ucl_event_proto_soip.cc

    src/adm_dis_host_proto_soip.cc
    src/adm_dis_host_event_proto_soip.cc
  
 	src/adm_dis_transHost_proto_soip.cc
    src/adm_dis_transHost_event_proto_soip.cc	

    ${CMAKE_SOURCE_DIR}/dim/dis/dis_public/src/proto/admSysi4vdiCallMsg.pb.cc
    ${CMAKE_SOURCE_DIR}/dim/dis/dis_public/src/proto/admSysi4ucmMsg.pb.cc
    ${CMAKE_SOURCE_DIR}/dim/dis/dis_public/src/proto/admSysi4hostMsg.pb.cc
    ${CMAKE_SOURCE_DIR}/dim/dis/dis_public/src/proto/admSysi4transhostMsg.pb.cc
)


add_library(${project_name}
	#STATIC
	SHARED
	${DIM_SRC}
)

message(${CMAKE_CURRENT_LIST_DIR})

target_include_directories(
    ${project_name}
    PRIVATE
    ${CMAKE_SOURCE_DIR}/application
    ${CMAKE_SOURCE_DIR}/foundation/packages
    ${CMAKE_SOURCE_DIR}/foundation/platform/os   
    ${CMAKE_SOURCE_DIR}/engine/common/errorCode/inc
    ${CMAKE_SOURCE_DIR}/foundation/platform/libs/inc
    ${CMAKE_SOURCE_DIR}/com/vrpc/sdk/include 
    ${CMAKE_SOURCE_DIR}/com/vrpc/
    ${CMAKE_SOURCE_DIR}/engine/common/config/inc
    ${CMAKE_SOURCE_DIR}/foundation/packages/log
    ${CMAKE_SOURCE_DIR}/foundation/tools/tracer/inc
    ${CMAKE_SOURCE_DIR}/foundation/packages/dlt
    ${CMAKE_SOURCE_DIR}/engine/common/lib/inc
    ${CMAKE_SOURCE_DIR}/application/dim/src
    ${CMAKE_SOURCE_DIR}/application/dim/inc/public
    ${CMAKE_SOURCE_DIR}/application/dis/dis_public/inc
    ${CMAKE_SOURCE_DIR}/application/dis/dis_public/inc/proto
    ${CMAKE_SOURCE_DIR}/config/inc
)

target_link_libraries(
	${project_name}
	    PRIVATE
	    ${FOUNDATION_LIB_PATH}
		${CMAKE_SOURCE_DIR}/build/externalLib/lib/libcore_service.a
		#${FOUNDATION_LIB_PATH}/libs/libdlt.so
		${DTS_TOOL_LIB}
		${CMAKE_SOURCE_DIR}/build/config/libconfig.a
		${RPC_LIBRARY}
		
        pthread
)

#add_subdirectory(sample)
#add_subdirectory(gtest)
