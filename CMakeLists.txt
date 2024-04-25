cmake_minimum_required(VERSION 3.19.2)
project(main)

set(CMAKE_CXX_STANDARD 20)


if(CMAKE_BUILD_TYPE STREQUAL "Release")
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        # 启用lto -flto=auto
        # 把 MINGW-根目录\libexec\gcc\x86_64-w64-mingw32\13.2.0 下的 liblto_plugin.dll,复制到 MINGW-根目录\lib\bfd-plugins
        # 去掉调试符号
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -s")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -march=native")
        # set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -flto=auto")

        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s")
        # 对本机cpu生成特定优化代码,可能存在兼容性问题
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native")
        # set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -flto=auto")
    endif()
endif()



add_subdirectory(SDL)

set(wzlibcpp ${CMAKE_CURRENT_SOURCE_DIR}/wzlibcpp)
add_subdirectory(wzlibcpp)

add_subdirectory(freetype)

add_subdirectory(sdlms)
