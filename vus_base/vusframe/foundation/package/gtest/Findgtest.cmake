include (../cmake/ExternalProjectHelper)

if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
	find_external_project_add(NAME gtest
	    DEPENDS ""
	    CONFIGURE_COMMANDS -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX} -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
	    EXPORT_LIBRARIES ${CMAKE_INSTALL_PREFIX}/lib/libgtestd.a ${CMAKE_INSTALL_PREFIX}/lib/libgtest_maind.a)
else()
	find_external_project_add(NAME gtest
	    DEPENDS ""
	    CONFIGURE_COMMANDS -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX} -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
	    EXPORT_LIBRARIES ${CMAKE_INSTALL_PREFIX}/lib/libgtest.a ${CMAKE_INSTALL_PREFIX}/lib/libgtest_main.a)
endif()
