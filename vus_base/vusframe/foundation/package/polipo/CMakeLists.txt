cmake_minimum_required(VERSION 3.10)
project(polipo)
set(project_name "polipo")

message("build polipo")

#add_custom_target(Build COMMAND make -f ${CMAKE_CURRENT_SOURCE_DIR}/polipo-master/Makefile WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/polipo-master)
set(LOCAL_ROOT /usr/share/polipo/www)
set(DISK_CACHE_ROOT /var/cache/polipo)

# -DLOCAL_ROOT="\"/sdcard/polipo/www\"" \
# -DDISK_CACHE_ROOT="\"/sdcard/polipo/cache/\"" \

ADD_DEFINITIONS(-DLOCAL_ROOT="$(LOCAL_ROOT)/")
ADD_DEFINITIONS(-DDISK_CACHE_ROOT="$(DISK_CACHE_ROOT)/")
#ADD_DEFINITIONS(-DHAS_STDINT_H)



#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Os -g -Wall -fno-strict-aliasing")
set(CMAKE_C_FLAGS "-Os -g -Wall -fPIC -fno-strict-aliasing")
#set(POLIPO_SRC polipo-master)
set(POLIPO_SRC polipo-1.1.0)

include_directories(${POLIPO_SRC})

add_library(${project_name}.1
    STATIC
    ${POLIPO_SRC}/util.c
    ${POLIPO_SRC}/event.c
    ${POLIPO_SRC}/io.c
    ${POLIPO_SRC}/chunk.c
    ${POLIPO_SRC}/atom.c
    ${POLIPO_SRC}/object.c
    ${POLIPO_SRC}/log.c
    ${POLIPO_SRC}/diskcache.c
    ${POLIPO_SRC}/config.c
    ${POLIPO_SRC}/local.c
    ${POLIPO_SRC}/http.c
    ${POLIPO_SRC}/client.c
    ${POLIPO_SRC}/server.c
    ${POLIPO_SRC}/auth.c
    ${POLIPO_SRC}/tunnel.c
    ${POLIPO_SRC}/http_parse.c
    ${POLIPO_SRC}/parse_time.c
    ${POLIPO_SRC}/dns.c
    ${POLIPO_SRC}/forbidden.c
    ${POLIPO_SRC}/md5import.c
    ${POLIPO_SRC}/ftsimport.c
    ${POLIPO_SRC}/socks.c
    ${POLIPO_SRC}/mingw.c)


add_executable(${project_name}
    ${POLIPO_SRC}/main.c)
target_link_libraries(${project_name} ${project_name}.1)



message("CMAKE_INSTALL_BINDIR: "${CMAKE_INSTALL_BINDIR})
install(TARGETS polipo
        RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin
        ARCHIVE DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
        LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)

install(CODE "FILE(RENAME libpolipo.1.a libpolipo.a)")
install(FILES
        ${CMAKE_BINARY_DIR}/libpolipo.a
        DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)

install(FILES 
        ${POLIPO_SRC}/mingw.h
        ${POLIPO_SRC}/ftsimport.h
        ${POLIPO_SRC}/atom.h
        ${POLIPO_SRC}/util.h
        ${POLIPO_SRC}/config.h
        ${POLIPO_SRC}/event.h
        ${POLIPO_SRC}/io.h
        ${POLIPO_SRC}/socks.h
        ${POLIPO_SRC}/chunk.h
        ${POLIPO_SRC}/object.h
        ${POLIPO_SRC}/dns.h
        ${POLIPO_SRC}/http.h
        ${POLIPO_SRC}/client.h
        ${POLIPO_SRC}/local.h
        ${POLIPO_SRC}/diskcache.h
        ${POLIPO_SRC}/server.h
        ${POLIPO_SRC}/http_parse.h
        ${POLIPO_SRC}/parse_time.h
        ${POLIPO_SRC}/forbidden.h
        ${POLIPO_SRC}/fts_compat.h
        ${POLIPO_SRC}/log.h
        ${POLIPO_SRC}/auth.h
        ${POLIPO_SRC}/tunnel.h
        ${POLIPO_SRC}/polipo.h
        DESTINATION ${CMAKE_INSTALL_PREFIX}/include/polipo)

install(FILES
        ${POLIPO_SRC}/config.sample 
        DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
install(CODE "FILE(RENAME ${CMAKE_INSTALL_PREFIX}/bin/config.sample ${CMAKE_INSTALL_PREFIX}/bin/polipo.ini)")

message("build polipo end")