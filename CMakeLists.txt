cmake_minimum_required(VERSION 3.10.0)

project(3D_viewer VERSION 1.0)

MESSAGE(STATUS "operation system is ${CMAKE_SYSTEM}")

set(CMAKE_BUILD_TYPE RelWithDebInfo)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
    message(STATUS "Release is on!!!")
else()
    message(STATUS "CMAKE_BUILD_TYPE = " ${CMAKE_BUILD_TYPE})
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_INCLUDE_PATH ${PROJECT_SOURCE_DIR}/cmake)
set(CMAKE_CXX_FLAGS "/permissive- /Zc:__cplusplus")

include(${CMAKE_INCLUDE_PATH}/qt_library.cmake)
include(${CMAKE_INCLUDE_PATH}/third_library.cmake)
include_qt_library()
include_third_library_path()

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
set(CMAKE_INSTALL_PREFIX ${PROJECT_SOURCE_DIR}/install_3D_viewer)
set(THIRD_LIBRARY_PATH ${PROJECT_SOURCE_DIR}/third_library)

link_directories(${LIBRARY_OUTPUT_PATH})

add_subdirectory(3D_viewer)

# copy to bin
file(GLOB QT_DLL "${QT_PATH}/bin/*.dll")
file(COPY ${QT_DLL} DESTINATION ${PROJECT_SOURCE_DIR}/bin/${CMAKE_BUILD_TYPE})

file(GLOB QT_PLUGINS_DLL "${QT_PATH}/plugins")
file(COPY ${QT_PLUGINS_DLL} DESTINATION ${PROJECT_SOURCE_DIR}/bin/${CMAKE_BUILD_TYPE})

copy_third_library_dll()

# install
set(QT_NEED_INSTALL 
    ${QT_PATH}/bin/Qt6Core.dll 
    ${QT_PATH}/bin/Qt6Gui.dll 
    ${QT_PATH}/bin/Qt6Widgets.dll
    ${QT_PATH}/bin/Qt6OpenGL.dll
    ${QT_PATH}/bin/Qt6OpenGLWidgets.dll
    ${QT_PATH}/bin/Qt6Svg.dll
)
INSTALL(FILES ${QT_NEED_INSTALL}
       DESTINATION ${CMAKE_INSTALL_PREFIX}
)

install_third_library()

set(QT_PLUGINS 
    ${QT_PATH}/plugins/platforms
    ${QT_PATH}/plugins/styles
    ${QT_PATH}/plugins/iconengines
)
INSTALL(DIRECTORY ${QT_PLUGINS}
       DESTINATION ${CMAKE_INSTALL_PREFIX}/plugins
)

file(GLOB RESOURCE_PATH ${PROJECT_SOURCE_DIR}/3D_viewer_resource)
INSTALL(DIRECTORY ${RESOURCE_PATH}
    DESTINATION ${CMAKE_INSTALL_PREFIX}
)