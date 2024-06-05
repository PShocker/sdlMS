**DeepinV23/Debian/Ubuntu编译**
```bash
sudo apt-get install libsdl2-dev libfreetype-dev
# 安装ffmpeg相关库
sudo apt-get install libavcodec-dev libavdevice-dev libavfilter-dev libavformat-dev libavutil-dev libswresample-dev libswscale-dev
cd build
cmake -DLINUX=1 ..
make -j8
```