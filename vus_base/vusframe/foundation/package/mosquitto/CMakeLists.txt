cmake_minimum_required(VERSION 3.10)
project(mosquitto)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -fPIC")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -fPIC")

option(DOCUMENTATION "Build documentation?" ON)
option(WITH_TLS
	"Include SSL/TLS support?" ON)
option(WITH_TLS_PSK
	"Include TLS-PSK support (requires WITH_TLS)?" ON)
option(WITH_EC
	"Include Elliptic Curve support (requires WITH_TLS)?" ON)
option(WITH_DLT "Include DLT support?" ON)
option(WITH_STATIC_LIBRARIES "Build static versions of the libmosquitto/pp libraries?" OFF)

set(DOCUMENTATION OFF)
set(WITH_TLS OFF)
set(WITH_TLS_PSK OFF)
set(WITH_EC OFF)
set(WITH_DLT OFF)
set(WITH_STATIC_LIBRARIES ON)
set(WITH_THREADING OFF)
set(LIBPTHREAD ON)

add_subdirectory(mosquitto)
