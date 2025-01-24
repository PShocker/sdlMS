重构版本,使用[LLVM18](https://github.com/mstorsjo/llvm-mingw/releases/tag/20240619),[Ninja](https://github.com/ninja-build/ninja/releases),[Cmake3.28](https://cmake.org/download/)编译

编译前先配置好环境变量

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

~~3.使用c++20特性,全部模块化~~
(目前编辑器不支持，又改成头文件形式)
