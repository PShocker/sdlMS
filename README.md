用SDL2复刻冒险岛,使用[cmake(MinGW)](https://github.com/niXman/mingw-builds-binaries/releases/tag/13.2.0-rt_v11-rev0)构建.

资源文件进QQ群760717877下载

新建build目录

资源Data放进build目录 

MinGW编译
```
cd build
cmake ..
make -j8
```

emscripten编译
```
cd build
emcmake cmake ..
emmake make -j8
python -m http.server 8500
```
