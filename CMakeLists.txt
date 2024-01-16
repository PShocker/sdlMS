cmake_minimum_required(VERSION 3.19.2)
project(sdlMS)

set(CMAKE_CXX_STANDARD 20)

# sdl2 is install here
set(SDL2_DIR "C:/Users/Shocker/Desktop/SDL2-2.28.5/cmake")

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



file(GLOB SOURCE_FILES src/**/*.cpp)


# set(CMAKE_EXE_LINKER_FLAGS "-static")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror")

# executable is ahead of `target_link...`
add_executable(sdlMS src/main.cpp ${SOURCE_FILES})
# 包含 SDL2 头文件目录
target_include_directories(sdlMS PRIVATE ${SDL2_INCLUDE_DIRS})
# 链接 SDL2 库
target_link_libraries(sdlMS PRIVATE ${SDL2_LIBRARIES} wzlib)