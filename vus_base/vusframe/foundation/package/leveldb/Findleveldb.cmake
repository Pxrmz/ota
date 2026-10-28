include (../cmake/ExternalProjectHelper)
find_external_project_add(NAME leveldb)
find_external_project_add(NAME leveldb
    EXPORT_LIBRARIES ${CMAKE_INSTALL_PREFIX}/lib/libleveldb.a)
