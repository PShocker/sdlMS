cmake_minimum_required(VERSION 3.19.2)
project(sdlMS)

set(CMAKE_CXX_STANDARD 20)


if(CMAKE_BUILD_TYPE STREQUAL "Release")
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        # 启用lto -flto=auto
        # 把 MINGW-根目录\libexec\gcc\x86_64-w64-mingw32\13.2.0 下的 liblto_plugin.dll,复制到 MINGW-根目录\lib\bfd-plugins
        # 去掉调试符号
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -s")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -march=native")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -flto=auto")

        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s")
        # 对本机cpu生成特定优化代码,可能存在兼容性问题
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -flto=auto")
        # add_compile_options(-s -flto -march=native)
    endif()
endif()

add_subdirectory(freetype)

set(FFMPEG_INCLUDE "C:/Users/Shocker/Desktop/FFMPEG/include")
set(FFMPEG_LIB "C:/Users/Shocker/Desktop/FFMPEG/lib")

add_subdirectory(SDL)


add_subdirectory(wzlibcpp)

include_directories(
    src/
)

include_directories(
    wzlibcpp/
    wzlibcpp/include
    wzlibcpp/mio/include
)

include_directories(
    ${FREETYPE_INCLUDE_DIRS}
    ${FFMPEG_INCLUDE}
)

link_directories(
    ${FREETYPE_LIBRARIES}
    ${FFMPEG_LIB}
)

file(GLOB SOURCE_FILES src/**/*.cpp)

# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated-declarations")

# executable is ahead of `target_link...`
add_executable(sdlMS src/main.cpp ${SOURCE_FILES})
# 将 SDL2 库链接到可执行文件中
target_link_libraries(sdlMS PRIVATE SDL2-static SDL2main)
# 包含 SDL2 头文件目录
target_include_directories(sdlMS PRIVATE ${SDL2_INCLUDE_DIRS})
# 链接 SDL2 库
target_link_libraries(sdlMS PRIVATE ${SDL2_STATIC_LIBRARIES} wzlib)
# 链接 FFMPEG库
target_link_libraries(sdlMS PRIVATE avformat avdevice avcodec avutil swscale avfilter swresample)
# 链接 FFMPEG依赖库,wzlib里面已经包含了zlib库
target_link_libraries(sdlMS PRIVATE bz2-1 iconv-2 lzma-5)
# freetype
target_link_libraries(sdlMS PRIVATE freetype)
# 链接 Windows平台库(FFMPEG)
target_link_libraries(sdlMS PRIVATE bcrypt ws2_32 Secur32 Strmiids)