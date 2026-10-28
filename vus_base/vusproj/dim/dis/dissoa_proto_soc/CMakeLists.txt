cmake_minimum_required (VERSION 3.10)
#project(Dim CXX C)

set(project_name "disprotosoc")

set(
	DIS_SRC
    src/adm_dis_vdi_proto_soc.cc
    src/adm_dis_vdi_event_proto_soc.cc

	src/adm_dis_ucl_proto_soc.cc
    src/adm_dis_ucl_event_proto_soc.cc

    src/adm_dis_host_proto_soc.cc
    src/adm_dis_host_event_proto_soc.cc
  
 	src/adm_dis_transHost_proto_soc.cc
    src/adm_dis_transHost_event_proto_soc.cc	
    
    src/adm_dis_dpo_proto_soc.cc
    src/adm_dis_dpo_event_proto_soc.cc

    ${CMAKE_SOURCE_DIR}/dim/dis/dis_public/src/proto/admSysi4vdiCallMsg.pb.cc
    ${CMAKE_SOURCE_DIR}/dim/dis/dis_public/src/proto/admSysi4ucmMsg.pb.cc
    ${CMAKE_SOURCE_DIR}/dim/dis/dis_public/src/proto/admSysi4hostMsg.pb.cc
    ${CMAKE_SOURCE_DIR}/dim/dis/dis_public/src/proto/admSysi4transhostMsg.pb.cc

    #${CMAKE_SOURCE_DIR}/dim/dis/dis_public/src/adm_dis_dpo.c
)

if(CUSTOME_USE_DLSYM)
add_library(${project_name}
	SHARED
	${DIS_SRC}
	)
else()
add_library(${project_name}
	STATIC
	${DIS_SRC}
	)
endif()

message(${CMAKE_CURRENT_LIST_DIR})

target_include_directories(
    ${project_name}
    PRIVATE
    
    ${CMAKE_SOURCE_DIR}/application
    ${CMAKE_SOURCE_DIR}/foundation/packages
    ${CMAKE_SOURCE_DIR}/foundation/platform/os   
    ${CMAKE_SOURCE_DIR}/engine/common/errorCode/inc
    ${CMAKE_SOURCE_DIR}/foundation/platform/libs/inc
    ${CMAKE_SOURCE_DIR}/foundation/platform/libs/inc/async
    ${CMAKE_SOURCE_DIR}/engine/common/config/inc
    ${CMAKE_SOURCE_DIR}/foundation/packages/log
    ${CMAKE_SOURCE_DIR}/tools/tracer/inc
    ${CMAKE_SOURCE_DIR}/foundation/packages/dlt
    ${CMAKE_SOURCE_DIR}/engine/common/lib/inc
    ${CMAKE_SOURCE_DIR}/dim/src
    ${CMAKE_SOURCE_DIR}/dim/inc/public
    ${CMAKE_SOURCE_DIR}/dim/dis/dis_public/inc
    ${CMAKE_SOURCE_DIR}/dim/dis/dis_public/inc/proto
    ${CMAKE_SOURCE_DIR}/arpc/inc
    ${CMAKE_SOURCE_DIR}/config/inc
    ${CMAKE_SOURCE_DIR}/soa/inc/public
)

if(CUSTOME_USE_DLSYM) 
	set(RPC_LIBRARY  ${CMAKE_SOURCE_DIR}/arpc/lib/libarpc.so) 
else() 
	set(RPC_LIBRARY  ${CMAKE_SOURCE_DIR}/arpc/lib/libarpc-static.a) 
endif()

target_link_libraries(
	${project_name}
	    PRIVATE
	    ${FOUNDATION_LIBRARY}
		${CMAKE_SOURCE_DIR}/engine/lib/libcore_service.a
		${RPC_LIBRARY}
)
