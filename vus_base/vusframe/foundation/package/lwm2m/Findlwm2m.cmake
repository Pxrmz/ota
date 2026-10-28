include (../cmake/ExternalProjectHelper)
find_external_project_add(NAME lwm2m)
find_external_project_add(NAME lwm2m
    EXPORT_LIBRARIES ${CMAKE_INSTALL_PREFIX}/lib/liblwm2m)
