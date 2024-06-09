**Mac编译**

确保你的clang支持c++20

一、编译FFMPEG静态库
访问[FFMPEG官方网站](https://ffmpeg.org/download.html),下载xz源码包,或者进群下载.

1.解压
```bash
tar xvf ffmpeg-7.0.tar.xz
cd ffmpeg-7.0 
```
2.配置FFMPEG编译选项
```
./configure --enable-static --disable-shared --disable-ffmpeg --disable-ffplay --disable-debug --disable-doc --disable-asm --prefix=/usr/local
```
3.编译和安装
```
make
make install
```

二、编译sdlMS
```
cd build
cmake ..
make -j8
```
编译后的文件在
```
build/bin
```
