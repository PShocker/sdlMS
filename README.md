# sdlms

sdlms 是一个用 **C++26** 实现的 2.5D 横版游戏（MapleStory风格），包含**服务端**和
基于 SDL3 的客户端。资源直接读取游戏的 **WZ** 文件，物理、动作、UI 全部由 WZ 数据
驱动；客户端自带一个本地服务端，单人模式不额外起进程。

技术栈（版本列是当前实际验证过的组合，不是“最低要求”）：

| 依赖 | 版本 | 用途 |
| --- | --- | --- |
| [SDL3](https://github.com/libsdl-org/SDL) | **3.4.10** | 窗口、渲染、音频、输入 |
| [libuv](https://github.com/libuv/libuv) | **1.52.1-dev**（1.x） | UDP 网络事件循环 |
| [wzlib](https://github.com/PShocker/wzlib) | **git `5b7d465`**，内置 zlib 1.3.2.1 | WZ 资源解析 |
| [FlatBuffers](https://github.com/google/flatbuffers) | **25.12.19**（HEAD `10c99415`） | 网络协议序列化 |
| [FFmpeg](https://ffmpeg.org/) | **N-115845-g7c95c7de4a**（2024-06-16，gpl-shared 动态库） | 音频/视频解码 |
| [FreeType](https://github.com/freetype/freetype) | **2.13.2** | 文字渲染 |
| CMake | **3.30.0**（`CMakeLists.txt` 要求 ≥ 3.28） | 构建 |
| Ninja | **1.12.1** | 构建 |
| MinGW-w64 GCC | **16.1.0**（`x86_64-posix-seh-rev1`） | 编译器 |

版本约定：

- **FFmpeg 的 ABI 是成组绑定的**：`libavcodec 61 / libavformat 61 / libavutil 59 /
  libswscale 8 / libswresample 5`（对应 `bin/` 里的 `avcodec-61.dll` 这一套）。
  升级时必须把 `include/`、`lib/*.dll.a`、`bin/*.dll` 整包一起换，只换 dll
  或只换头文件都会在运行期崩。
- **SDL3** 是 3.4.x 系列；代码用了 `SDL_IOStream`/`SDL_ReadIO`/`SDL_ReadU8` 等
  3.x 接口，升级到 3.5+ 前要重新核对签名。`C:/SDL3-3.4.10` 这个目录名也带版本，
  换版本时记得同步 `SDLMS_SDL3_ROOT`。
- **wzlib** 没有语义化版本，按 git commit 固定；它会把自带的 zlib
  （`ZLIB_VERSION "1.3.2.1-motley"`）以静态对象方式并进 `wzlib` 共享库。
  跨平台（非 Windows/PE）构建时它需要 `CMAKE_POSITION_INDEPENDENT_CODE=ON`，
  否则 ELF 平台链接期会报 `recompile with -fPIC`。
- **FlatBuffers**：`https://github.com/google/flatbuffers.git`，本地 `C:/flatbuffers`
  （HEAD `10c99415`）。只有头文件参与编译，`flatc` 须与头文件同版本（25.12.19）。
- **libuv 与 FreeType 是本地源码树自建的**（分别产出 `libuv.dll.a`、
  `libfreetype.dll.a`），不来自包管理器；其余依赖是预编译二进制包。

### 项目架构

```
src/
  client/              客户端
    main.cpp           SDL 入口，驱动逻辑/渲染/事件系统列表
    game/              纯数据类型（GameCharacter、GameMob、GameItem …）
    game_instance/     各系统的全局状态（MobGameInstance、PackageGameInstance …）
    game_skill/        职业技能
    system/
      logic/           每帧推进的逻辑系统（角色、怪物、掉落、光标 …）
      render/          渲染系统（背景、角色、怪物、特效、UI …）
      ui/              界面系统（背包、技能、商店、选项窗口 …）
      ui/component/    可复用控件（Button、Tab、Slider、CheckBox、ComboBox、TextInput、Scroll）
      input/           键盘与光标输入
    system_instance/   高层状态机（登录、选角色、建角色、场景切换 …）
    window/            SDL 窗口与帧计时
  common/              客户端与服务端共用
    wz/                WZ 加载、纹理与字形资源
    physic/            地形、行走、跳跃、坠落物理
    flatbuffers/       flatc 生成的协议头
    request/ response/ 客户端→服务端、服务端→客户端的消息封装
    freetype/ ffmpeg/  文字与音频
    script/            NPC、消耗品、反应堆脚本
  server/              服务端（默认与客户端同进程）
    server/            数据结构（ServerMob、ServerDrop …）
    server_instance/   请求处理与实体实例（登录、场景、怪物、掉落、交易 …）
    server_system/     每帧推进的系统（怪物 AI、掉落、心跳 …）
    request_handler.cpp 网络包分发
flatbuffers/           协议定义（client.fbs、server.fbs、common.fbs、protocol.fbs）
shaders/              HLSL 着色器与编译产物（shaders/bin）
scripts/              flatbuffers / 着色器代码生成、clang-tidy 脚本
```

要点：

- **客户端与服务端同进程**：默认启动时客户端同时充当服务端（`ServerMain::ServerInit()`），
  所有服务端系统挂在同一个帧循环里；带参数启动则连接远端服务端。
- **网络**：UDP + FlatBuffers，顶层是 `NetPacket`，载荷是 `NetPayload` 联合体
  （见 `flatbuffers/protocol.fbs`）。
- **UI 风格**：每个界面系统实现 `GetComponent()`（每帧重建控件列表）、
  `RenderUI()`（按列表绘制）、`EventUI()`（处理点击），复用 `component/` 下的控件。
- **数据驱动**：界面位置、怪物动作、伤害范围等全部从 WZ 读取，代码里只保留规则。

### 环境要求

- Windows 10/11 x64（当前只在 Windows + MinGW-w64 上验证过，脚本是 `.bat`）
- **MinGW-w64 GCC 16.x**（实测 `16.1.0`，`x86_64-posix-seh-rev1`）
- CMake ≥ 3.28（实测 3.30.0）+ Ninja（实测 1.12.1）
- 需要 C++26 标准库设施：`std::polymorphic`（GCC 16 的 libstdc++ 才提供）和
  C++23 的 `std::flat_map`。已实测 GCC 16.1.0 + `-std=gnu++26` 可用；
  GCC 15 及更早会因缺 `std::polymorphic` 直接编译失败。
  Clang / MSVC 未验证，若要支持需自行确认这两个设施的实现。
- 依赖库：SDL3 3.4.10、libuv 1.52.1、wzlib（git `5b7d465`）、
  FlatBuffers 25.12.19、FFmpeg N-115845、FreeType 2.13.2（见上方版本表）

### 编译

依赖路径默认指向作者机器，可用 CMake 变量覆盖：

| 变量 | 默认值 | 该路径实际提供的版本 / 链接产物 |
| --- | --- | --- |
| `SDLMS_SDL3_ROOT` | `C:/SDL3-3.4.10` | SDL3 **3.4.10**，链接 `lib/x64/SDL3.dll.a` |
| `SDLMS_LIBUV_ROOT` | `C:/libuv-1.x` | libuv **1.52.1-dev**，源码树自建，链接 `build/libuv.dll.a` |
| `SDLMS_WZLIB_ROOT` | `C:/Users/Shocker/Desktop/wzlib` | wzlib @ `5b7d465`，链接 `build/libwzlib.dll.a`（内置 zlib 1.3.2.1） |
| `SDLMS_FLATBUFFERS_ROOT` | `C:/flatbuffers` | FlatBuffers **25.12.19**，只用头文件，不链接库 |
| `SDLMS_FFMPEG_ROOT` | `C:/ffmpeg-master-latest-win64-gpl-shared` | FFmpeg **N-115845-g7c95c7de4a**，链接 `lib/av*-*.dll.a` |
| `SDLMS_FREETYPE_ROOT` | `C:/freetype` | FreeType **2.13.2**，源码树自建，链接 `build/libfreetype.dll.a` |

```sh
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

常用选项：

- `-DSDLMS_WARNINGS_AS_ERRORS=OFF`：警告不当作错误，方便改警告。

> `CMakeLists.txt` 用 `file(GLOB_RECURSE)` 收集源文件：新增或删除 `.cpp` 后
> 需要重新执行一次 `cmake -S . -B build`。

### 运行

程序按工作目录找资源，运行前把数据放到 `build/` 下：

```
build/
  main.exe
  Data/      WZ 数据（必须，缺了会加载失败）
  Font/      simsun.ttc 等字体（必须）
  Save/      存档，首次运行时自动创建
```

```sh
cd build
./main.exe                      # 单人，本地
./main.exe <server-ip> <port>   # 连接远端服务端
```

`main.exe` 实际导入的 DLL（版本必须与编译期一致，可从上面的 `*_ROOT` 目录拷到
`build/` 或放进 `PATH`）：

| DLL | 来源 | 版本 |
| --- | --- | --- |
| `SDL3.dll` | SDL3 3.4.10 | 3.4.10 |
| `libuv.dll` | libuv 源码树 | 1.52.1-dev |
| `libwzlib.dll` | wzlib 构建产物 | wzlib `5b7d465`（内置 zlib 1.3.2.1） |
| `libfreetype.dll` | FreeType 源码树 | 2.13.2 |
| `avcodec-61.dll`、`avformat-61.dll`、`avutil-59.dll`、`swscale-8.dll` | FFmpeg 预编译包 | N-115845-g7c95c7de4a |
| `libstdc++-6.dll`、`libgcc_s_seh-1.dll` | MinGW-w64 GCC | 16.1.0 |

> `CMakeLists.txt` 里链接了 `swresample`，但当前代码没有引用它的符号，所以
> `main.exe` 并不导入 `swresample-5.dll`；真正会用到时再一并拷贝。

### 工具

工具版本（这些不是链接依赖，但生成产物必须与库版本对齐）：

| 工具 | 版本 | 说明 |
| --- | --- | --- |
| `clang-format` | 23.1.3 | `format` 目标使用 |
| `clang-tidy` | 23.1.4（LLVM 23） | 需配合 `C:/mingw64` 的 GCC 头文件 |
| `flatc` | 25.12.19 | **必须与 `flatbuffers/include` 头文件同版本** |
| `shadercross` | 3.0.0 | 与 SDL3 3.4.x 配套，输出 SPIR-V / MSL / DXIL |

```sh
cmake --build build --target format   # clang-format 格式化 src/
scripts\compile_flatbuffers.bat       # 由 flatbuffers/*.fbs 重新生成 src/common/flatbuffers/*.h
scripts\compile_shader.bat            # HLSL -> shaders/bin 下的 SPIR-V / MSL / DXIL
scripts\clang-tidy.bat                # 全量静态检查；也可只传一个文件路径
```

改动协议时先改 `flatbuffers/*.fbs`，再跑 `compile_flatbuffers.bat`。