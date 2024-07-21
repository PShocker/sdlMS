重构版本,使用LLVM18,Ninja,Cmake3.28编译

[LLVM](https://github.com/mstorsjo/llvm-mingw/releases/tag/20240619)
[Ninja](https://github.com/ninja-build/ninja/releases)
[Cmake](https://cmake.org/download/)
先配置ninja环境变量

拉取项目
```
git clone --recurse-submodules https://github.com/PShocker/sdlMS.git --depth 1 -b new
```

新建build目录,进入
```
cmake .. -G Ninja
ninja
```

重构内容:
1.使用SDL3
2.使用entt,全面去除Class
3.使用c++20特性,全部模块化

