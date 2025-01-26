include_directories("${CMAKE_CURRENT_SOURCE_DIR}")
include_directories("${CMAKE_SOURCE_DIR}/3rdparty/entt/src")
include_directories("${CMAKE_SOURCE_DIR}/3rdparty/SDL/include")
include_directories("${CMAKE_SOURCE_DIR}/3rdparty/wzlibcpp")
include_directories("${CMAKE_SOURCE_DIR}/3rdparty/wzlibcpp/include")
include_directories("${CMAKE_SOURCE_DIR}/3rdparty/wzlibcpp/mio/include")

# ffmpeg
include_directories("C:/ffmpeg-master-latest-win64-gpl-shared/include")
link_directories("C:/ffmpeg-master-latest-win64-gpl-shared/lib")

include_directories("C:/FFMPEG-WEB/include")
link_directories("C:/FFMPEG-WEB/lib")

file(GLOB SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/**/*.cpp)
add_executable(sdlMS ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp ${SOURCE_FILES})

target_link_libraries(sdlMS PRIVATE SDL3::SDL3)
target_link_libraries(sdlMS PRIVATE wzlib)
target_link_libraries(sdlMS PRIVATE freetype)
target_link_libraries(sdlMS PRIVATE EnTT)

target_link_libraries(sdlMS PRIVATE avformat avdevice avcodec avutil swscale avfilter swresample)

if(WIN32)
        if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
                add_definitions(-DNOMINMAX)
        endif()

        set_target_properties(sdlMS PROPERTIES
                RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
endif()

if(APPLE)
        set_target_properties(sdlMS PROPERTIES
                RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
        )
endif()

# 设置 Emscripten 编译选项
if(EMSCRIPTEN)
        set(CMAKE_EXECUTABLE_SUFFIX ".html")

        # set_target_properties(sdlMS PROPERTIES LINK_FLAGS
        # "--preload-file ${CMAKE_SOURCE_DIR}/build/Data/@Data/"

        # # 可以添加其他Emscripten链接器选项
        # )
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -sALLOW_MEMORY_GROWTH -sUSE_ZLIB -sMAXIMUM_MEMORY=4294967296 -sSTACK_SIZE=256MB  -sINITIAL_MEMORY=512MB -sFETCH=1 -sASYNCIFY=1")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -sALLOW_MEMORY_GROWTH -sUSE_ZLIB -sMAXIMUM_MEMORY=4294967296 -sSTACK_SIZE=256MB -sINITIAL_MEMORY=512MB -sFETCH=1 -sASYNCIFY=1")

        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O2 -flto")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -flto")
        # set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g4")
        # set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g4")
        set_target_properties(sdlMS PROPERTIES
                RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
        )
endif()
