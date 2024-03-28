cmake_minimum_required(VERSION 3.19.2)
project(sdlMS)

set(CMAKE_CXX_STANDARD 20)


if(CMAKE_BUILD_TYPE STREQUAL "Release")
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    # 启用lto -flto=auto
    # 把 MINGW-根目录\libexec\gcc\x86_64-w64-mingw32\13.2.0 下的 liblto_plugin.dll,复制到 MINGW-根目录\lib\bfd-plugins
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s -flto=auto")
        if(WIN32)
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mavx2")
            message("CMAKE_CXX_FLAGS: ${CMAKE_CXX_FLAGS}")
        endif()
    endif()
endif()

# sdl2 is install here
set(SDL2_DIR "C:/Users/Shocker/Desktop/SDL2-2.28.5/cmake")

set(FREETYPE_INCLUDE_DIRS "C:/Users/Shocker/Desktop/freetype/include")
set(FREETYPE_LIBRARIES "C:/Users/Shocker/Desktop/freetype/lib")


set(FFMPEG_INCLUDE "C:/Users/Shocker/Desktop/FFMPEG/include")
set(FFMPEG_LIB "C:/Users/Shocker/Desktop/FFMPEG/lib")

find_package(SDL2 REQUIRED)

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

# set(CMAKE_EXE_LINKER_FLAGS "-static")

# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated-declarations")

# executable is ahead of `target_link...`
add_executable(sdlMS src/main.cpp ${SOURCE_FILES})
# 包含 SDL2 头文件目录
target_include_directories(sdlMS PRIVATE ${SDL2_INCLUDE_DIRS})
# 链接 SDL2 库
target_link_libraries(sdlMS PRIVATE ${SDL2_LIBRARIES} wzlib)
# 链接 FFMPEG库
target_link_libraries(sdlMS PRIVATE avformat avdevice avcodec avutil swscale avfilter swresample)
# 链接 FFMPEG依赖库,wzlib里面已经包含了zlib库
target_link_libraries(sdlMS PRIVATE bz2-1 iconv-2 lzma-5)
# freetype
target_link_libraries(sdlMS PRIVATE freetype)
# 链接 Windows平台库(FFMPEG)
target_link_libraries(sdlMS PRIVATE bcrypt ws2_32 Secur32 Strmiids)

