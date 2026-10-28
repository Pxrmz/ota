cmake_minimum_required(VERSION 3.10)
project(googletest)

add_compile_options(-fPIC)

# Allow enabling and disabling components
option(BUILD_GMOCK "Builds the googlemock subproject" ON)
set(BUILD_GMOCK ON)
option(INSTALL_GTEST "Enable installation of googletest. (Projects embedding googletest may want to turn this OFF.)" ON)
set(INSTALL_GTEST ON)
add_subdirectory(googletest)
