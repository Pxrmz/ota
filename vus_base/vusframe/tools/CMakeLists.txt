cmake_minimum_required(VERSION 3.10)

###########################################################SDK VERSION########################################################################

# SET(MAJOR_VERSION 1)
# SET(MINOR_VERSION 1)
# SET(HOTFIX_VERSION 1)
SET(PROJECT_NAME "tools")
message("SDK_VERSION = ${SDK_VERSION}")
SET(ADM_SDK_VERSION ${SDK_VERSION})

EXECUTE_PROCESS(COMMAND date +%Y%m%d OUTPUT_VARIABLE COMPILE_DATE OUTPUT_STRIP_TRAILING_WHITESPACE)
EXECUTE_PROCESS(COMMAND git rev-parse --short HEAD OUTPUT_VARIABLE GIT_COMMITID OUTPUT_STRIP_TRAILING_WHITESPACE)

SET(PROJECT_VERSION ${PROJECT_NAME}_${ENV_PLATFORM}_${ADM_SDK_VERSION}_${COMPILE_DATE}_${GIT_COMMITID})
ADD_DEFINITIONS(-DADM_TOOLS_SDK_VERSION="${PROJECT_VERSION}")
MESSAGE("+++++++++++PROJECT_VERSION = ${PROJECT_VERSION}")

set(FOUNDATION_LIB_PATH ${CMAKE_SOURCE_DIR}/../foundation_lib/${ENV_PLATFORM}/packages/libs)
message("-- Foundation lib path ++++++++++" ${FOUNDATION_LIB_PATH})

message("PLATFORM_NAME = ${PLATFORM_NAME}")

if(${CUSTOM_CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set(SYSTEM_LIBRARY pthread rt dl stdc++ m)
elseif(${CUSTOM_CMAKE_SYSTEM_NAME} MATCHES "Android")
    set(SYSTEM_LIBRARY c stdc++ m)
    add_definitions(-D_DLT_SYSTEM_NAME_ANDROID_)
elseif(${CUSTOM_CMAKE_SYSTEM_NAME} MATCHES "Alios")
    set(SYSTEM_LIBRARY c)
elseif(${CUSTOM_CMAKE_SYSTEM_NAME} MATCHES "QNX7")
    set(SYSTEM_LIBRARY c c++ m socket)
else()
    set(SYSTEM_LIBRARY pthread rt dl stdc++)
endif()

###########################################################SDK VERSION########################################################################


add_subdirectory(tracer)
