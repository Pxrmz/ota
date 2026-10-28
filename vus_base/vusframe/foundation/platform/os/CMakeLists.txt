cmake_minimum_required(VERSION 3.10)

#file(COPY 
#inc/adm_memory.h
#inc/adm_queue.h
#inc/adm_string.h
#inc/adm_thread.h
#inc/adm_typedefs.h
#DESTINATION ${CMAKE_OUTPUT_PREFIX}/include)


set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC -D_GNU_SOURCE")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DADB_DBG_MEM")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")

include_directories(${CMAKE_SOURCE_DIR}/platform/os/inc)
include_directories(${CMAKE_SOURCE_DIR}/package/log/log)
include_directories(${CMAKE_SOURCE_DIR}/package/log/log/EasyLogger/easylogger/inc/)
include_directories(${CMAKE_SOURCE_DIR}/package/log/log/EasyLogger/easylogger/plugins/file/)

include_directories(${CMAKE_OUTPUT_PREFIX}/include)



MESSAGE(STATUS "operation system is ${CMAKE_SYSTEM}")

IF (CMAKE_SYSTEM_NAME MATCHES "Linux")
	MESSAGE(STATUS "current platform: Linux ")
ELSEIF (CMAKE_SYSTEM_NAME MATCHES "Windows")
	MESSAGE(STATUS "current platform: Windows")
ELSEIF (CMAKE_SYSTEM_NAME MATCHES "FreeBSD")
	MESSAGE(STATUS "current platform: FreeBSD")
ELSE ()
	MESSAGE(STATUS "other platform: ${CMAKE_SYSTEM_NAME}")
ENDIF (CMAKE_SYSTEM_NAME MATCHES "Linux")


set(BUILD_OS_DIR  ${CMAKE_CURRENT_SOURCE_DIR}/linux)

set(OS_HEADERS
    ./inc/adm_error.h
    ./inc/adm_memory.h
    ./inc/adm_string.h
    ./inc/adm_typedefs.h
)
set(OS_SOURCES
    ${BUILD_OS_DIR}/adm_os_memory.c
    ${BUILD_OS_DIR}/adm_os_string.c
    ${BUILD_OS_DIR}/adm_os_thread.c
    ${BUILD_OS_DIR}/adm_os_queue.c
    ${BUILD_OS_DIR}/adm_os_queue_test.c
    ${BUILD_OS_DIR}/adm_os_time.c
    ${BUILD_OS_DIR}/adm_os_timer.c
    ${BUILD_OS_DIR}/adm_os_file.c
    ${BUILD_OS_DIR}/adm_os_stringstream.cpp    
)

add_library(
    admos
    STATIC
    ${OS_SOURCES}
    ${OS_HEADERS}
)
add_dependencies(admos log)

install(FILES 
    inc/adm_memory.h
    inc/adm_queue.h
    inc/adm_string.h
    inc/adm_thread.h
    inc/adm_typedefs.h
    DESTINATION ${INSTALL_DIR}/include)

install(TARGETS admos
    ARCHIVE DESTINATION ${INSTALL_DIR}/lib
    LIBRARY DESTINATION ${INSTALL_DIR}/lib)
