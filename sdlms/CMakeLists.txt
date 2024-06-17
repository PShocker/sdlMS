
set(FFMPEG_INCLUDE "E:\\mygame\\Maple\\sdlMS2\\FFMPEG\\include")
set(FFMPEG_LIB "E:\\mygame\\Maple\\sdlMS2\\FFMPEG\\lib")

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
)

include_directories(
    ${FFMPEG_INCLUDE}
)

set(wzlibcpp ${PROJECT_SOURCE_DIR}/3rdparty/wzlibcpp)
include_directories(
    ${wzlibcpp}
    ${wzlibcpp}/include
    ${wzlibcpp}/mio/include
)

link_directories(
    ${FFMPEG_LIB}
)

file(GLOB SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/**/*.cpp)

if(WIN32)
    add_executable(sdlMS ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp ${SOURCE_FILES})
    # 将 SDL2 库链接到可执行文件中
    target_link_libraries(sdlMS PRIVATE SDL2-static SDL2main)
    # 链接 wzlib 库
    target_link_libraries(sdlMS PRIVATE wzlib)
    # 链接 FFMPEG库
    target_link_libraries(sdlMS PRIVATE avformat avdevice avcodec avutil swscale avfilter swresample)
    # freetype
    target_link_libraries(sdlMS PRIVATE freetype)
    # 链接 Windows平台库(FFMPEG)
    target_link_libraries(sdlMS PRIVATE bcrypt ws2_32 Secur32 Strmiids)
    # 链接 FFMPEG依赖库,wzlib里面已经包含了zlib库
    if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        # 使用微软编译器
        add_definitions(-DNOMINMAX)
    else()
        # 使用 Mingw 编译器
        target_link_libraries(sdlMS PRIVATE bz2-1 iconv-2 lzma-5)
    endif()
    
    set_target_properties(sdlMS PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
    )
endif()

if(LINUX)
    add_executable(sdlMS ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp ${SOURCE_FILES})
    # 将 SDL2 库链接到可执行文件中
    target_link_libraries(sdlMS PRIVATE SDL2-static SDL2main)
    # 链接 wzlib 库
    target_link_libraries(sdlMS PRIVATE wzlib)
    # 链接 FFMPEG库
    target_link_libraries(sdlMS PRIVATE avformat avdevice avcodec avutil swscale avfilter swresample)
    # freetype
    target_link_libraries(sdlMS PRIVATE freetype)
    set_target_properties(sdlMS PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
    )
endif()

# 设置 Emscripten 编译选项
if(EMSCRIPTEN)
    add_executable(sdlMS ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp ${SOURCE_FILES})
    # 将 SDL2 库链接到可执行文件中
    target_link_libraries(sdlMS PRIVATE SDL2-static SDL2main)
    # 链接 wzlib 库
    target_link_libraries(sdlMS PRIVATE wzlib)
    # 链接 FFMPEG库
    target_link_libraries(sdlMS PRIVATE avformat avdevice avcodec avutil swscale avfilter swresample)
    # freetype
    target_link_libraries(sdlMS PRIVATE freetype)

    set(CMAKE_EXECUTABLE_SUFFIX ".html")
    set_target_properties(sdlMS PROPERTIES LINK_FLAGS
            "--preload-file ${CMAKE_SOURCE_DIR}/build/Data/@Data/"
            # 可以添加其他Emscripten链接器选项
    )
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -sALLOW_MEMORY_GROWTH -g -sUSE_ZLIB")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -sALLOW_MEMORY_GROWTH -g -sUSE_ZLIB")
    set_target_properties(sdlMS PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
    )
endif()

if(ANDROID)
    add_library(main SHARED)
    target_sources(main PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp ${SOURCE_FILES})
    find_library(SDL2 SDL2)
    target_link_libraries(main SDL2 android)
    # 链接 wzlib 库
    target_link_libraries(main wzlib)
    # 链接 FFMPEG 库
    target_link_libraries(main avformat avdevice avcodec avutil swscale avfilter swresample -Wl,-Bsymbolic)
    # freetype
    target_link_libraries(main freetype)
endif()

if (APPLE)
    # Find packages
    find_library(COREMEDIA_FRAMEWORK CoreMedia)
    find_library(VIDEOTOOLBOX_FRAMEWORK VideoToolbox)
    find_library(SECURITY_FRAMEWORK Security)

    add_executable(sdlMS ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp ${SOURCE_FILES})

    # 将 SDL2 库链接到可执行文件中
    target_link_libraries(sdlMS PRIVATE SDL2-static SDL2main)

    # 链接 wzlib 库
    target_link_libraries(sdlMS PRIVATE wzlib)
    # 链接 FFMPEG库
    target_link_libraries(sdlMS PRIVATE avformat avdevice avcodec avutil swscale avfilter swresample)
    # freetype
    target_link_libraries(sdlMS PRIVATE freetype)

    # Link frameworks
    target_link_libraries(
        sdlMS
        PRIVATE
        ${COREMEDIA_FRAMEWORK}
        ${VIDEOTOOLBOX_FRAMEWORK}
        ${SECURITY_FRAMEWORK}
    )

    set_target_properties(sdlMS PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    )
endif()
