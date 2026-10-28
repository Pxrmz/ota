function(find_external_project_add)
    set(options BUILD_SHARED_LIBRARY)
    set(oneValueArgs NAME)
    set(multiValueArgs DEPENDS EXPORT_LIBRARIES CONFIGURE_COMMANDS EXTRA_LINKS)
    cmake_parse_arguments(Argument "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    message(STATUS "==> Preparing add external project: ${CMAKE_SOURCE_DIR}/package/${Argument_NAME}")
    message(STATUS "    depends:   ${Argument_DEPENDS}")
    message(STATUS "    libraries: ${Argument_EXPORT_LIBRARIES}")
    message(STATUS "    configs:   ${Argument_CONFIGURE_COMMANDS}")

    get_property(${Argument_NAME}_FOUND GLOBAL PROPERTY ${Argument_NAME}_FOUND_PROPERTY)
    get_property(${Argument_NAME}_INCLUDE_DIRS GLOBAL PROPERTY ${Argument_NAME}_INCLUDE_DIRS_PROPERTY)
    get_property(${Argument_NAME}_LIBRARIES GLOBAL PROPERTY ${Argument_NAME}_LIBRARIES_PROPERTY)
    get_property(${Argument_NAME}_EXTRA_LINKS GLOBAL PROPERTY ${Argument_NAME}_EXTRA_LINKS)

    if (${Argument_NAME}_FOUND)
        message(STATUS "==> External project ${CMAKE_SOURCE_DIR}/package/${Argument_NAME} has existed!, exported libraries: ${${Argument_NAME}_LIBRARIES}")
        set(${Argument_NAME}_FOUND ${${Argument_NAME}_FOUND} PARENT_SCOPE)
        set(${Argument_NAME}_INCLUDE_DIRS ${${Argument_NAME}_INCLUDE_DIRS} PARENT_SCOPE)
        set(${Argument_NAME}_LIBRARIES ${${Argument_NAME}_LIBRARIES} ${${Argument_NAME}_EXTRA_LINKS} PARENT_SCOPE)
    else ()
        if (Argument_EXPORT_LIBRARIES)
            if (Argument_BUILD_SHARED_LIBRARY)
                if(${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
                    set(LIBRARY_SUFFIX "dylib")
                else()
                    set(LIBRARY_SUFFIX "so")
                endif()
            else ()
                set(LIBRARY_SUFFIX "a")
            endif()

            message(STATUS "==> Handling exported libraries for \"${CMAKE_SYSTEM_NAME}\" platform")
            set(new_library_paths "")
            foreach (library_path IN LISTS Argument_EXPORT_LIBRARIES)
                message(STATUS "==> Handling library: ${library_path}")
                string(REGEX REPLACE "(.*)[.]so" "\\1" temp_library_path ${library_path})
                string(REGEX REPLACE "(.*)[.]dylib" "\\1" temp_library_path ${library_path})
                string(REGEX REPLACE "(.*)[.]a" "\\1" temp_library_path ${library_path})
                string(JOIN "." new_library_path ${temp_library_path} ${LIBRARY_SUFFIX})
                message(STATUS "    >> ${new_library_path}")
                list(APPEND new_library_paths ${new_library_path})
            endforeach()
            set(Argument_EXPORT_LIBRARIES ${new_library_paths})
            message(STATUS "==> Exported libraries: ${Argument_EXPORT_LIBRARIES}")
        endif()

        message(STATUS "==> Added external project ${CMAKE_SOURCE_DIR}/package/${Argument_NAME}")
        include(ExternalProject)

        set(CMAKE_CONFIGURE_COMMAND
            cmake
            -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
            -DCMAKE_INSTALL_INCLUDEDIR=${CMAKE_INSTALL_INCLUDEDIR}
            -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
            -DCUSTOM_CMAKE_SYSTEM_NAME=${CUSTOM_CMAKE_SYSTEM_NAME}
            -DCUSTOM_CMAKE_XML_NAME=${CUSTOM_CMAKE_XML_NAME}
            -DUSE_SYSTEM_SSL=${USE_SYSTEM_SSL}
            -DOPENSSL_ROOT_DIR=${OPENSSL_ROOT_DIR}
            ${CMAKE_SOURCE_DIR}/package/${Argument_NAME}
            ${Argument_CONFIGURE_COMMANDS}
        )

        foreach (depend IN LISTS Argument_DEPENDS)
            find_package(${depend} REQUIRED)
        endforeach()

        ExternalProject_Add(
            ${Argument_NAME}
            PREFIX ${CMAKE_INSTALL_PREFIX}/../package/${Argument_NAME}
            TMP_DIR ${CMAKE_INSTALL_PREFIX}/../package/${Argument_NAME}
            SOURCE_DIR ${CMAKE_SOURCE_DIR}/package/${Argument_NAME}
            DEPENDS ${Argument_DEPENDS}
            CONFIGURE_COMMAND ${CMAKE_CONFIGURE_COMMAND}
            BUILD_COMMAND make
            BUILD_ALWAYS 1
            INSTALL_COMMAND make install
        )

        set_property(GLOBAL PROPERTY ${Argument_NAME}_FOUND_PROPERTY ON)
        set_property(GLOBAL PROPERTY ${Argument_NAME}_INCLUDE_DIRS_PROPERTY "${CMAKE_INSTALL_PREFIX}/include")
        set_property(GLOBAL PROPERTY ${Argument_NAME}_LIBRARIES_PROPERTY "${Argument_EXPORT_LIBRARIES}")
        set_property(GLOBAL PROPERTY ${Argument_NAME}_EXTRA_LINKS_PROPERTY "${Argument_EXTRA_LINKS}")

        set(${Argument_NAME}_FOUND TRUE PARENT_SCOPE )
        set(${Argument_NAME}_INCLUDE_DIRS ${CMAKE_INSTALL_PREFIX}/include PARENT_SCOPE )
        if (Argument_EXPORT_LIBRARIES)
            set(${Argument_NAME}_LIBRARIES ${Argument_EXPORT_LIBRARIES} ${Argument_EXTRA_LINKS} PARENT_SCOPE )
        endif ()
    endif ()

endfunction()
