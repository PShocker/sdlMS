cmake_minimum_required(VERSION 3.19.2)
project(SimpleWindow)

# sdl2 is install here
set(SDL2_DIR C:/Users/Shocker/Desktop/SDL2-2.28.5/cmake)

find_package(SDL2 REQUIRED)

# executable is ahead of `target_link...`
add_executable(SimpleWindow main.cpp)
# 包含 SDL2 头文件目录
target_include_directories(SimpleWindow PRIVATE ${SDL2_INCLUDE_DIRS})
# 链接 SDL2 库
target_link_libraries(SimpleWindow PRIVATE ${SDL2_LIBRARIES})