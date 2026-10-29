# sdlMS

A MapleStory-style 2.5D game with an authoritative server, written in C++26 on
top of [SDL3](https://github.com/libsdl-org/SDL/tree/main),
[libuv](https://github.com/libuv/libuv), [wzlib](https://github.com/PShocker/wzlib),
[ffmpeg](https://ffmpeg.org/), [freetype](https://github.com/freetype/freetype)
and [flatbuffers](https://github.com/google/flatbuffers).

## Layout

```
src/
  client/          game client
    game/          plain data types (GameMob, GameItem, ...)
    game_instance/ per-system game state
    game_skill/    job skills
    system/        logic, render, ui and input systems
    system_instance/ high level client state machines
    window/        SDL window and frame timing
  common/          shared code: wz loading, fonts, physics, flatbuffers protocol
  server/          authoritative server: instances, systems, request handling
shaders/           HLSL shaders and compiled SPIR-V/MSL output
scripts/           helper scripts for shaders and flatbuffers codegen
```

## Requirements

* CMake 3.28 or newer, Ninja
* GCC 14+/Clang 18+/MSVC 19.4+ with C++26 support (`std::flat_map`)
* Prebuilt SDL3, libuv, wzlib, freetype and ffmpeg

## Build

Dependency locations default to the author's machine and can be overridden
through CMake cache variables:

| Variable | Default |
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

Useful options:

* `-DSDLMS_WARNINGS_AS_ERRORS=OFF` — keep building while fixing warnings.
* `-DSDLMS_NATIVE_OPTIMIZATIONS=OFF` — drop `-march=native`/LTO from Release
  builds so the binary runs on other CPUs.

## Run

```sh
cd build
./main.exe                 # single player, hosts locally
./main.exe <server-ip> <port>
```

## Style

The sources follow the
[Google C++ Style Guide](https://google.github.io/styleguide/cppguide.html):

* Types and functions are `PascalCase`; variables are `snake_case`; class data
  members carry a trailing underscore (`Window::renderer_`); constants and
  enumerators are `kPascalCase` (`JobType::kWarrior`).
* Layout is handled by `clang-format` with the Google base style: two space
  indentation, 80 columns, asterisk bound to the type (`.clang-format`).
* Headers use `#pragma once` and include their own dependencies first; the
  related header comes first in `.cpp` files.
* All project declarations live in `namespace sdlms`; flatbuffers generated
  types stay in `namespace fbs` and are always used fully qualified.
* Only passive data holders are `struct`s; anything with behaviour is a `class`.
  UI buttons go through the `Button` value type
  (`src/client/system/ui/button.h`) instead of hand-rolled texture switching.

Format the tree before sending a change:

```sh
cmake --build build --target format   # or: clang-format -i src/**/*.cpp
```

Static analysis follows the naming rules in `.clang-tidy` and uses the
compilation database in `build/compile_commands.json`:

```sh
scripts\clang-tidy.bat                                  # whole tree
scripts\clang-tidy.bat src\client\system\ui\button.cpp  # one file
```
