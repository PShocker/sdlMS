# sdlms

sdlms 是一个用 **C++26** 实现的 2.5D 横版游戏（冒险岛风格），包含**服务端**和
基于 SDL3 的客户端。资源直接读取游戏的 **WZ** 文件，物理、动作、UI 全部由 WZ 数据
驱动；客户端自带一个本地服务端，单人模式不额外起进程。

技术栈：

| 依赖 | 用途 |
| --- | --- |
| [SDL3](https://github.com/libsdl-org/SDL/tree/main) | 窗口、渲染、音频、输入 |
| [libuv](https://github.com/libuv/libuv) | UDP 网络事件循环 |
| [wzlib](https://github.com/PShocker/wzlib) | WZ 资源解析 |
| [FlatBuffers](https://github.com/google/flatbuffers) | 网络协议序列化 |
| [FFmpeg](https://ffmpeg.org/) | 音频解码 |
| [FreeType](https://github.com/freetype/freetype) | 文字渲染 |
| CMake + Ninja | 构建 |

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
- CMake ≥ 3.28，Ninja
- 支持 C++26 的编译器：GCC 14+ / Clang 18+ / MSVC 19.4+（用到 `std::flat_map`、`std::polymorphic`）
- 预编译好的 SDL3、libuv、wzlib、FlatBuffers、FFmpeg、FreeType

### 编译

依赖路径默认指向作者机器，可用 CMake 变量覆盖：

| 变量 | 默认值 |
| --- | --- |
| `SDLMS_SDL3_ROOT` | `C:/SDL3-3.4.10` |
| `SDLMS_LIBUV_ROOT` | `C:/libuv-1.x` |
| `SDLMS_WZLIB_ROOT` | `C:/Users/Shocker/Desktop/wzlib` |
| `SDLMS_FLATBUFFERS_ROOT` | `C:/flatbuffers` |
| `SDLMS_FFMPEG_ROOT` | `C:/ffmpeg-master-latest-win64-gpl-shared` |
| `SDLMS_FREETYPE_ROOT` | `C:/freetype` |

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

### 工具

```sh
cmake --build build --target format   # clang-format 格式化 src/
scripts\compile_flatbuffers.bat       # 由 flatbuffers/*.fbs 重新生成 src/common/flatbuffers/*.h
scripts\compile_shader.bat            # HLSL -> shaders/bin 下的 SPIR-V / MSL / DXIL
scripts\clang-tidy.bat                # 全量静态检查；也可只传一个文件路径
```

改动协议时先改 `flatbuffers/*.fbs`，再跑 `compile_flatbuffers.bat`。