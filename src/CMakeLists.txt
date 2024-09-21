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

add_definitions(-DSTART_INFO_CONSOLE) # 控制是否开启debug控制台，用于观察是否有报错日志
add_definitions(-DSTB_IMAGE_IMPLEMENTATION) # 添加 stb图像库 预定义

target_include_directories(${TARGET} PRIVATE 
       "${QT_SDK_DIR}/include/QtCore/6.5.2"
       "${QT_SDK_DIR}/include/Qt3DRender/6.5.2"
       "${QT_SDK_DIR}/include/QtCore/6.5.2/QtCore"
       "${QT_SDK_DIR}/include/Qt3DRender/6.5.2/Qt3DRender"
       ${ASSIMP_PATH}/include
       ${SPDLOG_PATH}
       ${STB_PATH}
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
       Vulkan::Vulkan
       ${ASSIMP_PATH}/lib/*.lib
)

if("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
       list(APPEND QT_BASE_DLL_LIST 
              "${QT_SDK_DIR}/bin/Qt6Cored.dll"
              "${QT_SDK_DIR}/bin/Qt6Guid.dll"
              "${QT_SDK_DIR}/bin/Qt6Widgetsd.dll"
              "${QT_SDK_DIR}/bin/Qt6Networkd.dll"
       )
       list(APPEND QT_OPENGL_DLL_LIST 
              "${QT_SDK_DIR}/bin/Qt6OpenGLWidgetsd.dll"
              "${QT_SDK_DIR}/bin/Qt6OpenGLd.dll"
       )
       list(APPEND QT_3D_DLL_LIST
              "${QT_SDK_DIR}/bin/Qt63DCored.dll"
              "${QT_SDK_DIR}/bin/Qt63DExtrasd.dll"
              "${QT_SDK_DIR}/bin/Qt63DInputd.dll"
              "${QT_SDK_DIR}/bin/Qt63DRenderd.dll"
              "${QT_SDK_DIR}/bin/Qt63DLogicd.dll"
              "${QT_SDK_DIR}/bin/Qt6Concurrentd.dll"
              "${QT_SDK_DIR}/bin/Qt6ShaderToolsd.dll"
              "${QT_SDK_DIR}/bin/Qt63DAnimationd.dll"
       )
       list(APPEND ASSIMP_DLL "${ASSIMP_PATH}/bin/assimp-vc143-mtd.dll")
else ()
       list(APPEND QT_BASE_DLL_LIST 
              "${QT_SDK_DIR}/bin/Qt6Core.dll"
              "${QT_SDK_DIR}/bin/Qt6Gui.dll"
              "${QT_SDK_DIR}/bin/Qt6Widgets.dll"
               "${QT_SDK_DIR}/bin/Qt6Network.dll"
       )
       list(APPEND QT_OPENGL_DLL_LIST 
              "${QT_SDK_DIR}/bin/Qt6OpenGLWidgets.dll"
              "${QT_SDK_DIR}/bin/Qt6OpenGL.dll"
       )
       list(APPEND QT_3D_DLL_LIST
              "${QT_SDK_DIR}/bin/Qt63DCore.dll"
              "${QT_SDK_DIR}/bin/Qt63DExtras.dll"
              "${QT_SDK_DIR}/bin/Qt63DInput.dll"
              "${QT_SDK_DIR}/bin/Qt63DRender.dll"
              "${QT_SDK_DIR}/bin/Qt63DLogic.dll"
              "${QT_SDK_DIR}/bin/Qt6Concurrent.dll"
              "${QT_SDK_DIR}/bin/Qt6ShaderTools.dll"
              "${QT_SDK_DIR}/bin/Qt63DAnimation.dll"
       )
       list(APPEND ASSIMP_DLL "${ASSIMP_PATH}/bin/assimp-vc143-mt.dll")
endif ()

foreach(QT_BASE_DLL ${QT_BASE_DLL_LIST})
       add_custom_command(TARGET ${TARGET} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy
              "${QT_BASE_DLL}"
              "${PROJECT_SOURCE_DIR}/bin/${CMAKE_BUILD_TYPE}"
       )
       INSTALL(FILES ${QT_BASE_DLL} DESTINATION ${CMAKE_INSTALL_PREFIX})
endforeach()

foreach(QT_OPENGL_DLL ${QT_OPENGL_DLL_LIST})
       add_custom_command(TARGET ${TARGET} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy
              "${QT_OPENGL_DLL}"
              "${PROJECT_SOURCE_DIR}/bin/${CMAKE_BUILD_TYPE}"
       )
       INSTALL(FILES ${QT_OPENGL_DLL} DESTINATION ${CMAKE_INSTALL_PREFIX})
endforeach()

foreach(QT_3D_DLL ${QT_3D_DLL_LIST})
       add_custom_command(TARGET ${TARGET} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy
              "${QT_3D_DLL}"
              "${PROJECT_SOURCE_DIR}/bin/${CMAKE_BUILD_TYPE}"
       )
       INSTALL(FILES ${QT_3D_DLL} DESTINATION ${CMAKE_INSTALL_PREFIX})
endforeach()

add_custom_command(TARGET ${TARGET} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy
       "${ASSIMP_DLL}"
       "${PROJECT_SOURCE_DIR}/bin/${CMAKE_BUILD_TYPE}"
)
INSTALL(FILES ${ASSIMP_DLL} DESTINATION ${CMAKE_INSTALL_PREFIX})

list(APPEND QT_PLUGINS_NAME_LIST
       "platforms"
       "styles"
       "renderers"
       "sceneparsers"
)
foreach(QT_PLUGINS_NAME ${QT_PLUGINS_NAME_LIST})
       add_custom_command(TARGET ${TARGET} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory
              "${QT_SDK_DIR}/plugins/${QT_PLUGINS_NAME}"
              "${PROJECT_SOURCE_DIR}/bin/${CMAKE_BUILD_TYPE}/plugins/${QT_PLUGINS_NAME}"
       )
       INSTALL(DIRECTORY ${QT_SDK_DIR}/plugins/${QT_PLUGINS_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX}/plugins)
endforeach()

if (WIN32)
       INSTALL(TARGETS ${TARGET} RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX})
elseif (UNIX)
       INSTALL(TARGETS ${TARGET} DESTINATION ${CMAKE_INSTALL_PREFIX})
endif ()
