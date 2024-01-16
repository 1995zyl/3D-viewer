set(TARGET 3D_viewer)

set(public_include ${CMAKE_CURRENT_SOURCE_DIR}/source/tool_source)
execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory ${public_include})

file(GLOB moc_path 
    "${CMAKE_CURRENT_SOURCE_DIR}/include/main_window.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/opengl_window.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/opengl_container.h"
)
execute_qt_moc("${TARGET}/include" "${TARGET}/source/tool_source" ${moc_path})
file(GLOB translate_path 
    "${CMAKE_CURRENT_SOURCE_DIR}/source/main_window.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/opengl_window.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/opengl_container.cpp"
)
execute_qt_translate("${CMAKE_CURRENT_SOURCE_DIR}/resource/translator/ZH_CN.ts" ${translate_path})
file(GLOB rcc_path "${CMAKE_CURRENT_SOURCE_DIR}/resource/*.qrc")
execute_qt_rcc("${TARGET}/resource" "${TARGET}/source/tool_source" ${rcc_path})

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include")

file(GLOB HEADER "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h")
file(GLOB SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp")
file(GLOB TOOL_SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/source/tool_source/*.cpp")
source_group("Source Files\\tool_source" FILES ${TOOL_SOURCE})

set(SRC_LIST ${SOURCE} ${HEADER} ${TOOL_SOURCE})

add_executable(${TARGET} ${SRC_LIST})

# 添加 stb图像库 预定义
add_definitions(-DSTB_IMAGE_IMPLEMENTATION)
# 控制是否开启debug控制台，用于观察是否有报错日志
add_definitions(-DSTART_INFO_CONSOLE)

target_link_libraries(${TARGET} PRIVATE opengl32)
target_link_libraries(${TARGET} PRIVATE ${QT_PATH}/lib/*.lib)
target_link_libraries(${TARGET} PRIVATE ${ASSIMP_PATH}/lib/*.lib)

INSTALL(TARGETS ${TARGET}
       DESTINATION ${CMAKE_INSTALL_PREFIX}
)

set(ASSIMP_NEED_INSTALL 
    ${ASSIMP_PATH}/bin/assimp-vc143-mt.dll
)
INSTALL(FILES ${ASSIMP_NEED_INSTALL}
       DESTINATION ${CMAKE_INSTALL_PREFIX}
)
