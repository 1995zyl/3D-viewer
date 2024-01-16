macro(include_third_library_path)   
    set(ASSIMP_PATH ${PROJECT_SOURCE_DIR}/third_library/assimp-master)
    set(STB_PATH ${PROJECT_SOURCE_DIR}/third_library/stb-master)
    set(SPDLOG_PATH ${PROJECT_SOURCE_DIR}/third_library/spdlog)
    
    include_directories(${ASSIMP_PATH}/include)
    include_directories(${STB_PATH})
    include_directories(${SPDLOG_PATH}/include)
endmacro()

# macro没有独立作用域，需要先调用include_third_library_path
macro(copy_third_library_dll)
    file(GLOB ASSIMP_DLL "${ASSIMP_PATH}/bin/assimp-vc143-mt.dll")
    file(COPY ${ASSIMP_DLL} DESTINATION ${PROJECT_SOURCE_DIR}/bin/${CMAKE_BUILD_TYPE})
endmacro()

macro(install_third_library)
    file(GLOB THIRD_LIBRARY_DLL "${ASSIMP_PATH}/bin/assimp-vc143-mt.dll")
    INSTALL(FILES ${THIRD_LIBRARY_DLL} DESTINATION ${CMAKE_INSTALL_PREFIX})
endmacro()