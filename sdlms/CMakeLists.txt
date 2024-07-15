
set(wzlibcpp ${PROJECT_SOURCE_DIR}/3rdparty/wzlibcpp)
include_directories(
    ${wzlibcpp}
    ${wzlibcpp}/include
    ${wzlibcpp}/mio/include
)


file(GLOB_RECURSE
        systems_cpp
        CONFIGURE_DEPENDS
        Systems/*.cpp
)

file(GLOB_RECURSE
        systems_cppm
        CONFIGURE_DEPENDS
        Systems/*.cppm
)

add_library(mod
        ${systems_cpp}
)

target_sources(mod
        PUBLIC
        FILE_SET core_cxx_modules TYPE CXX_MODULES FILES
        ${systems_cppm}
)


add_executable(sdlMS main.cpp)
target_link_libraries(sdlMS PRIVATE SDL3::SDL3)
target_link_libraries(sdlMS PRIVATE wzlib)
# # target_link_libraries(sdlMS PRIVATE avformat avdevice avcodec avutil swscale avfilter swresample)
target_link_libraries(sdlMS PRIVATE freetype)
target_link_libraries(sdlMS PRIVATE EnTT)
target_link_libraries(sdlMS PRIVATE mod)


if(WIN32)
    # 链接 Windows平台库(FFMPEG)
    # target_link_libraries(sdlMS PRIVATE bcrypt ws2_32 Secur32 Strmiids)
    # 链接 FFMPEG依赖库,wzlib里面已经包含了zlib库
    if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        # 使用微软编译器
        add_definitions(-DNOMINMAX)
    else()
        # 使用 Mingw 编译器
        # target_link_libraries(sdlMS PRIVATE bz2-1 iconv-2 lzma-5)
    endif()
    
    set_target_properties(sdlMS PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
    )
endif()

if(LINUX)
    set_target_properties(sdlMS PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
    )
endif()

# 设置 Emscripten 编译选项
if(EMSCRIPTEN)
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
