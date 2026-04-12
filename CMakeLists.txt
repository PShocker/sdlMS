cmake_minimum_required(VERSION 3.28)
project(main)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories("${CMAKE_CURRENT_SOURCE_DIR}")

include_directories("C:/SDL3-3.4.0/include")
link_directories("C:/SDL3-3.4.0/lib/x64")

include_directories("C:/libuv-1.x/include")
link_directories("C:/libuv-1.x/build")

include_directories("C:/flatbuffers/include")

# ffmpeg
include_directories("C:/ffmpeg-master-latest-win64-gpl-shared/include")
link_directories("C:/ffmpeg-master-latest-win64-gpl-shared/lib")

# include_directories("C:/FFMPEG-WEB/include")
# link_directories("C:/FFMPEG-WEB/lib")

file(GLOB_RECURSE ALL_CPP_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
list(FILTER ALL_CPP_FILES EXCLUDE REGEX ".*CMakeFiles.*")
add_executable(main ${CMAKE_CURRENT_SOURCE_DIR}/src/client/main.cpp ${ALL_CPP_FILES})

target_link_libraries(main PRIVATE SDL3)

target_link_libraries(main PRIVATE avcodec avformat avutil swscale swresample)

target_link_libraries(main PRIVATE uv)

if (MSVC)
    add_compile_options(/WX)  # MSVC 的 "警告视为错误"
else()
    add_compile_options(-Werror)  # GCC/Clang 的选项
endif()

if(WIN32)
        if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
                set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /utf-8")
                set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /bigobj")
                add_definitions(-DNOMINMAX)
        endif()

        set_target_properties(main PROPERTIES
                RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
        target_link_libraries(main PRIVATE ws2_32)
endif()

if(APPLE)
        set_target_properties(main PROPERTIES
                RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
        )
endif()