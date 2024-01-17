# 3D-viewer
基于Qt及OpenGL的简易3D模型查看器

### 使用环境
1、本地需要安装cmake工具与visual studio。cmake版本为3.23左右，vs版本为vs2022左右。

2、工程依赖的Qt(6.5)、assimp、stb及spdlog等以三方库的形式上传到该工程的third_libray目录了，本地不再需要这些依赖。

3、支持*.glb;*.obj;*.fbx;等格式，实际上只测试过glb格式，其他格式理论支持，实际没有调试。

### 编译过程
1、git clone https://github.com/1995zyl/3D-viewer.git

2、在工程中新建build目录，利用cmake工程生成vs工程，不指定CMAKE_INSTALL_PREFIX，会默认安装到当前工程的install_3D_viewer目录下，也可以手动指定安装目录。对生成的vs工程的INSTALL目标进行编译即可安装

3、宏START_INFO_CONSOLE控制是否开启debug控制台，用于观察是否有报错日志，打开后会产生两个窗口
