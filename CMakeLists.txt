cmake_minimum_required(VERSION 3.18)

project(3D_viewer 
    VERSION 2.0
    DESCRIPTION "main project"
    HOMEPAGE_URL "https://github.com/1995zyl/3D-viewer"
    LANGUAGES CXX
)

set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/bin)
link_directories(${LIBRARY_OUTPUT_PATH})

message(STATUS "CMAKE_BUILD_TYPE = " ${CMAKE_BUILD_TYPE})
message(STATUS "CMAKE_INSTALL_PREFIX = " ${CMAKE_INSTALL_PREFIX})

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/3d_viewer.cmake)
find_qt_package()
find_assimp_library()
find_stb_library()
find_spdlog_library()

add_subdirectory(src)
