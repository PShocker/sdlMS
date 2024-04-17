cmake_minimum_required(VERSION 3.19.2)
project(sdlMS)

set(CMAKE_CXX_STANDARD 20)

set(FFMPEG_INCLUDE "C:/Users/Shocker/Desktop/FFMPEG/include")
set(FFMPEG_LIB "C:/Users/Shocker/Desktop/FFMPEG/lib")

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
)

include_directories(
    ${FFMPEG_INCLUDE}
)

include_directories(
    ${wzlibcpp}
    ${wzlibcpp}/include
    ${wzlibcpp}/mio/include
)

link_directories(
    ${FFMPEG_LIB}
)

file(GLOB SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/**/*.cpp)

# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated-declarations")

# executable is ahead of `target_link...`
add_executable(sdlMS ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp ${SOURCE_FILES})
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

set_target_properties(sdlMS PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)