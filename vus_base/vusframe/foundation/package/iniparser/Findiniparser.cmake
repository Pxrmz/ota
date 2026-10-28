include (../cmake/ExternalProjectHelper)
find_external_project_add(NAME iniparser)
find_external_project_add(NAME iniparser
    EXPORT_LIBRARIES ${CMAKE_INSTALL_PREFIX}/lib/libiniparser.a)
