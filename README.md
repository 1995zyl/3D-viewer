# 3D-viewer
基于Qt的简易3D模型查看器  
依赖Qt assimp、stb及spdlog    
支持*.glb;*.obj;*.fbx;等格式，实际上只测试过glb格式，其他格式理论支持，实际没有调试。

### 编译过程
1、git clone --recursive https://github.com/1995zyl/3D-viewer.git  
2、cd 3rdparty/assimpSceneImportExPlugin  
3、mkdir build & cd build  
编译AssimpSceneImportExPlugin及assimp，不要指定CMAKE_INSTALL_PREFIX  
4、cmake .. -DCMAKE_BUILD_TYPE=Release -DQT_SDK_DIR=D:/Qt/6.5.2/msvc2019_64  
将插件安装到qt目录中  
5、make -j4 & make install  
编译3D viewer    
6、cd ../../..  
7、mkdir build & cd build  
8、cmake .. -DCMAKE_BUILD_TYPE=Release -DQT_SDK_DIR=D:/Qt/6.5.2/msvc2019_64 -DCMAKE_INSTALL_PREFIX=D:/GitHub/3D-viewer/install  
9、make -j4 & make install  
