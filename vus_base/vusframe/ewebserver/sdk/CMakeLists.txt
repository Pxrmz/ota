# Determines what CMake APIs we can rely on
cmake_minimum_required (VERSION 2.8.11)

# Set up the project
project (sample)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu11")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

message(STATUS "CMAKE_C_COMPILER:${CMAKE_C_COMPILER} CMAKE_SYSTEM_NAME:${CMAKE_SYSTEM_NAME} PROJECT_SOURCE_DIR:${PROJECT_SOURCE_DIR}")
message(STATUS "CMAKE_C_FLAGS:${CMAKE_C_FLAGS}")
message(STATUS "CMAKE_CXX_FLAGS:${CMAKE_CXX_FLAGS}")

if ($ENV{CC} MATCHES ".*arm.*" )
    set(PLATFORM_NAME arm)
else()
    set(PLATFORM_NAME linux)
endif ()
	
   	
# Build the targets
# The smaple server executable

add_executable(sample sample.c)
target_include_directories(
    sample PUBLIC
    ${PROJECT_SOURCE_DIR}/include)
    
    
set(EWEBSRV_STATIC_LIB ${PROJECT_SOURCE_DIR}/lib/${PLATFORM_NAME}/ewebsrv_static.a)
target_link_libraries(sample
        ${EWEBSRV_STATIC_LIB}
        pthread
        stdc++
        )    




