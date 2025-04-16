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

target_link_libraries(sdlMS PRIVATE avformat avcodec avutil swscale swresample)

if(WIN32)
        if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
                set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /utf-8")
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

        set_target_properties(sdlMS PROPERTIES LINK_FLAGS "--preload-file ${CMAKE_SOURCE_DIR}/build/Font/@Font/")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -sALLOW_MEMORY_GROWTH -sUSE_ZLIB -sSTACK_SIZE=32MB")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -sINITIAL_MEMORY=64MB")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -sFETCH=1")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -sASYNCIFY=1")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -sMAXIMUM_MEMORY=4GB")

        # set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -g4 -sNO_DISABLE_EXCEPTION_CATCHING=1")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -O3 -flto")

        set_target_properties(sdlMS PROPERTIES
                RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
        )
endif()
