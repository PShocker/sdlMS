用SDL2复刻冒险岛,使用cmake(MinGW)构建.

如果使用Windows+MinGW,则wzlibcpp的zlib不需要更改,否则需要重新编译zlib静态库

https://github.com/madler/zlib

放到zlib目录

资源文件进QQ群760717877下载

资源路径修改 wz_util.hpp
```
const std::string filename_prefix = "C:/Users/Shocker/Desktop/sdlMS/data/"; //修改成你的路径
```

持续更新中...
