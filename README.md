# 3D-viewer
基于Qt的简易3D模型查看器
依赖assimp、stb及spdlog
采用Qt3D模块渲染3d模型，但qt原生的assimpsceneimport库不支持glb格式，改进了一下支持glb格式，依赖assimpsceneimportex插件
支持*.glb;*.obj;*.fbx;等格式，实际上只测试过glb格式，其他格式理论支持，实际没有调试。

### 编译过程
编译AssimpSceneImportExPlugin及assimp
1、git clone --recursive https://github.com/1995zyl/3D-viewer.git
2、cd 3rdparty/assimpSceneImportExPlugin
3、mkdir build & cd build
4、cmake .. -DCMAKE_BUILD_TYPE=Release -DQT_SDK_DIR=D:/Qt/6.5.2/msvc2019_64
5、make -j4 & make install # 将插件安装到qt目录中
6、cd ../../..
7、mkdir build & cd build
8、cmake .. -DCMAKE_BUILD_TYPE=Release -DQT_SDK_DIR=D:/Qt/6.5.2/msvc2019_64 -DCMAKE_INSTALL_PREFIX=D:/GitHub/3D-viewer/install
9、make -j4 & make install