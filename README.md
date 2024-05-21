用SDL2复刻冒险岛,使用[cmake(MinGW)](https://github.com/niXman/mingw-builds-binaries/releases/tag/13.2.0-rt_v11-rev0)构建.

资源文件进QQ群760717877下载

新建build目录

资源Data放进build目录 

**MinGW编译**
```
cd build
cmake ..
make -j8
```

**emscripten编译**
```
cd build
emcmake cmake ..
emmake make -j8
python -m http.server 8500
```
emscripten编译zlib可能会有问题,如果出现打不开的情况需要重新编译一次


**Android编译(Android Studio)**

把SDL文件夹的android-project提取出来,然后把项目放到里,**还要把资源文件放进assets目录里**

```
└── android-project
    ├── app
    │   ├── jni
    │   │   ├── freetype
    │   │   ├── SDL
    │   │   ├── sdlms 
    │   │   ├── wzlibcpp
    │   │   ├── CMakeLists.txt
    │   │   ├── CMakeLists.txt
    │   │   
    │   │──src   
    │   │   └── main
    │   │        └── assets
    │   │            ├── Character.wz
    │   │            ├── Map.wz
    │   │            ├── Mob.wz
    │   │            ├── Npc.wz
    │   │            ├── simsun.ttc
    │   │            ├── Sound.wz
    │   │            ├── String.wz
```
修改```android-project\app\build.gradle```去掉mk编译,使用cmake编译,并且使用```c++_shared``` 
```
        externalNativeBuild {
//            ndkBuild {
//                arguments "APP_PLATFORM=android-21"
//                abiFilters 'arm64-v8a'
//            }
             cmake {
                 arguments "-DANDROID_APP_PLATFORM=android-21", "-DANDROID_STL=c++_shared"
                 // abiFilters 'armeabi-v7a', 'arm64-v8a', 'x86', 'x86_64'
                 abiFilters 'arm64-v8a'
             }
        }
```
```
        externalNativeBuild {
//            ndkBuild {
//                path 'jni/Android.mk'
//            }
             cmake {
                 path 'jni/CMakeLists.txt'
             }
        }
```
配置ndk路径
build.gradle指定ndk版本
```
    android {
        ndkVersion "26.3.11579264" // e.g.,  ndkVersion "21.3.6528147"
    }
```
或者配置ndk路径
```android-project\local.properties```
```
ndk.dir=C\:\\Users\\Shocker\\AppData\\Local\\Android\\Sdk\\ndk\\android-ndk-r27-beta1
```