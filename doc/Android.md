**Android编译(Android Studio Iguana | 2023.2.1 Patch 1)**

把根目录SDL文件夹的```android-project```提取出来,然后把项目放到```android-project\app\jni```

还要把资源文件放进assets目录里

```
└── android-project
    ├── app
    │   ├── jni
    │   │   ├── freetype
    │   │   ├── SDL
    │   │   ├── sdlms 
    │   │   ├── wzlibcpp
    │   │   └── CMakeLists.txt
    │   │   
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
    │   │            └── String.wz
```
修改```android-project\app\build.gradle```去掉mk编译,使用cmake编译,并且使用```c++_shared``` 
```
externalNativeBuild {
//  ndkBuild {
//      arguments "APP_PLATFORM=android-21"
//      abiFilters 'arm64-v8a'
//  }
    cmake {
        arguments "-DANDROID_APP_PLATFORM=android-21", "-DANDROID_STL=c++_shared"
        // abiFilters 'armeabi-v7a', 'arm64-v8a', 'x86', 'x86_64'
        abiFilters 'arm64-v8a'
    }
}
```
```
externalNativeBuild {
//  ndkBuild {
//      path 'jni/Android.mk'
//  }
    cmake {
        path 'jni/CMakeLists.txt'
    }
}
```
修改```android-project\app\build.gradle```指定ndk版本
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
调整jvm内存大小```gradle.properties```
```
org.gradle.jvmargs=-Xmx4096m
```