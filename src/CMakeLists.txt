set(TARGET 3D_viewer)

file(GLOB_RECURSE all_headers *.h)
file(GLOB_RECURSE all_cpps *.cpp)
set(all_files ${all_headers} ${all_cpps})
source_group_by_dir(all_files)

set(translate_path 
       "${CMAKE_CURRENT_SOURCE_DIR}/main_window.cpp"
       "${CMAKE_CURRENT_SOURCE_DIR}/render_container.cpp"
       "${CMAKE_CURRENT_SOURCE_DIR}/opengl/opengl_window.cpp"
)
execute_qt_translate("${CMAKE_CURRENT_SOURCE_DIR}/resource/ZH_CN.ts" ${translate_path})

set(rcc_path "${CMAKE_CURRENT_SOURCE_DIR}/resource/res.qrc")
execute_qt_rcc("${CMAKE_CURRENT_SOURCE_DIR}/resource" "${CMAKE_CURRENT_BINARY_DIR}" ${rcc_path})

file(GLOB extral_comple_file "${CMAKE_CURRENT_BINARY_DIR}/*.cpp")
add_executable(${TARGET} ${all_files} ${extral_comple_file})

find_package(Qt${QT_VERSION_MAJOR} COMPONENTS OpenGL OpenGLWidgets 3DCore 3DRender 3DExtras 3DAnimation REQUIRED)

# 添加 stb图像库 预定义
add_definitions(-DSTB_IMAGE_IMPLEMENTATION)

target_include_directories(${TARGET} PRIVATE 
       "${QT_SDK_DIR}/include/QtCore/6.5.2"
       "${QT_SDK_DIR}/include/Qt3DRender/6.5.2"
       "${QT_SDK_DIR}/include/QtCore/6.5.2/QtCore"
       "${QT_SDK_DIR}/include/Qt3DRender/6.5.2/Qt3DRender"
)

target_link_libraries(${TARGET}
       PUBLIC
       Qt${QT_VERSION_MAJOR}::Gui
       Qt${QT_VERSION_MAJOR}::Core
       Qt${QT_VERSION_MAJOR}::Widgets
       Qt${QT_VERSION_MAJOR}::OpenGL
       Qt${QT_VERSION_MAJOR}::OpenGLWidgets
       Qt${QT_VERSION_MAJOR}::3DCore
       Qt${QT_VERSION_MAJOR}::3DRender
       Qt${QT_VERSION_MAJOR}::3DExtras
       Qt${QT_VERSION_MAJOR}::3DAnimation
)

target_link_libraries(${TARGET} PRIVATE 
       opengl32
       ${ASSIMP_PATH}/lib/*.lib
)

if("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
       file(GLOB QT_OPENGL_DLL_LIST 
              "${QT_SDK_DIR}/bin/Qt6OpenGLWidgetsd.dll"
              "${QT_SDK_DIR}/bin/Qt6OpenGLd.dll"
       )
else ()
       file(GLOB QT_OPENGL_DLL_LIST 
              "${QT_SDK_DIR}/bin/Qt6OpenGLWidgets.dll"
              "${QT_SDK_DIR}/bin/Qt6OpenGL.dll"
       )
endif ()
foreach(QT_OPENGL_DLL ${QT_OPENGL_DLL_LIST})
       add_custom_command(TARGET ${TARGET} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy
              "${QT_OPENGL_DLL}"
              "${PROJECT_SOURCE_DIR}/bin/${CMAKE_BUILD_TYPE}"
       )
endforeach()
