include (../cmake/ExternalProjectHelper)

#find_external_project_add(NAME polipo)
find_external_project_add(NAME polipo
	EXPORT_LIBRARIES ${CMAKE_INSTALL_PREFIX}/lib/polipo.a)

