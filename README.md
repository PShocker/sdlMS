用SDL2复刻冒险岛,使用[cmake(MinGW)](https://github.com/niXman/mingw-builds-binaries/releases/tag/13.2.0-rt_v11-rev0)构建.

需要配置好SDL2库,修改CMakeList.txt
```
# sdl2 is install here 修改你自己的SDL路径
set(SDL2_DIR "C:/Users/Shocker/Desktop/SDL2-2.28.5/cmake")
````

资源文件进QQ群760717877下载

资源路径修改 

src\util\wz_util.hpp
```
const std::string filename_prefix = "C:/Users/Shocker/Desktop/sdlMS/data/"; //修改成你的路径
```
src\util\freetype.cpp

```
FT_New_Face(*_library, (filename_prefix + "NotoSansSC-Regular.ttf").c_str(), 0, _face);
```
持续更新中...
