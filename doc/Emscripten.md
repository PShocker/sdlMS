**emscripten编译**

配置emscripten环境
https://emscripten.org/docs/getting_started/downloads.html

```
cd build
emcmake cmake ..
emmake make -j8
python -m http.server 8500
```
emscripten编译zlib可能会有问题,如果出现打不开的情况需要重新编译一次
