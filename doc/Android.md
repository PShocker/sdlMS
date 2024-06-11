**Android编译(Android Studio Iguana | 2023.2.1 Patch 1)**

把根目录SDL文件夹的```android-project```提取出来,然后把项目放到```android-project\app\jni```

```
└── android-project
    ├── app
    │   ├── jni
    │   │   ├── freetype
    │   │   ├── SDL
    │   │   ├── sdlms 
    │   │   ├── wzlibcpp
    │   │   └── CMakeLists.txt(替换)
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

AndroidManifest.xml配置读取sdcard权限
```
<uses-permission android:name="android.permission.READ_EXTERNAL_STORAGE"/>
<uses-permission android:name="android.permission.MANAGE_EXTERNAL_STORAGE"/>
```

修改```android-project\app\src\main\java\org\libsdl\app\SDLActivity.java```申请读取文件权限(安卓11以上)
```java
private void requestManageExternalStoragePermission() {
    Intent intent = new Intent(Settings.ACTION_MANAGE_APP_ALL_FILES_ACCESS_PERMISSION);
    intent.setData(Uri.parse("package:" + getPackageName()));
    startActivityForResult(intent, 0);
}
// Setup
@Override
protected void onCreate(Bundle savedInstanceState) {
    if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R) {
        if (!Environment.isExternalStorageManager()) {
            requestManageExternalStoragePermission();
        }
    }
    ......
}
```

**资源文件放进/sdcard/Data/**
