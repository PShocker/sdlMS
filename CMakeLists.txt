cmake_minimum_required(VERSION 3.28...4.0)

project(sdlms
  VERSION 0.1.0
  DESCRIPTION "MapleStory-style client and server built on SDL3"
  LANGUAGES CXX)

# ------------------------------------------------------------------------------
# Options
# ------------------------------------------------------------------------------
option(SDLMS_WARNINGS_AS_ERRORS "Treat compiler warnings as errors" ON)
option(SDLMS_NATIVE_OPTIMIZATIONS "Compile for the host CPU in Release builds" ON)

# Needed by clangd / clang-tidy (see .clang-tidy).
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ------------------------------------------------------------------------------
# Third-party dependencies (prebuilt, installed outside this repository)
# ------------------------------------------------------------------------------
set(SDLMS_SDL3_ROOT "C:/SDL3-3.4.10" CACHE PATH "SDL3 installation prefix")
set(SDLMS_LIBUV_ROOT "C:/libuv-1.x" CACHE PATH "libuv installation prefix")
set(SDLMS_WZLIB_ROOT "C:/Users/Shocker/Desktop/wzlib" CACHE PATH "wzlib prefix")
set(SDLMS_FLATBUFFERS_ROOT "C:/flatbuffers" CACHE PATH "flatbuffers prefix")
set(SDLMS_FFMPEG_ROOT "C:/ffmpeg-master-latest-win64-gpl-shared"
    CACHE PATH "ffmpeg installation prefix")
set(SDLMS_FREETYPE_ROOT "C:/freetype" CACHE PATH "freetype installation prefix")

# ------------------------------------------------------------------------------
# Sources
# ------------------------------------------------------------------------------
file(GLOB_RECURSE ALL_CPP_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
list(FILTER ALL_CPP_FILES EXCLUDE REGEX ".*CMakeFiles.*")

# ------------------------------------------------------------------------------
# Target
# ------------------------------------------------------------------------------
add_executable(main ${CMAKE_CURRENT_SOURCE_DIR}/src/client/main.cpp ${ALL_CPP_FILES})

target_compile_features(main PRIVATE cxx_std_26)
set_target_properties(main PROPERTIES
  CXX_STANDARD_REQUIRED ON
  CXX_SCAN_FOR_MODULES OFF
  RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

target_include_directories(main PRIVATE
  ${PROJECT_SOURCE_DIR}
  ${SDLMS_SDL3_ROOT}/include
  ${SDLMS_LIBUV_ROOT}/include
  ${SDLMS_WZLIB_ROOT}/include
  ${SDLMS_FLATBUFFERS_ROOT}/include
  ${SDLMS_FFMPEG_ROOT}/include
  ${SDLMS_FREETYPE_ROOT}/include)

target_link_directories(main PRIVATE
  ${SDLMS_SDL3_ROOT}/lib/x64
  ${SDLMS_LIBUV_ROOT}/build
  ${SDLMS_WZLIB_ROOT}/build
  ${SDLMS_FFMPEG_ROOT}/lib
  ${SDLMS_FREETYPE_ROOT}/build)

target_link_libraries(main PRIVATE
  SDL3
  wzlib
  uv
  freetype
  avcodec
  avformat
  avutil
  swscale
  swresample)

if(WIN32)
  target_link_libraries(main PRIVATE ws2_32)
endif()

# ------------------------------------------------------------------------------
# Warnings
# ------------------------------------------------------------------------------
if(MSVC)
  target_compile_options(main PRIVATE /W4 /utf-8 /bigobj)
  target_compile_definitions(main PRIVATE NOMINMAX)
else()
  # -Wall -Wextra, minus the checks that the SDL/ffmpeg callback signatures and
  # flatbuffers tables make unavoidable.
  target_compile_options(main PRIVATE
    -Wall
    -Wextra
    -Wno-unused-parameter
    -Wno-unused-variable
    -Wno-unused-but-set-variable
    -Wno-missing-field-initializers
    -Wno-sign-compare
    -Wno-unused-function)
endif()

if(SDLMS_WARNINGS_AS_ERRORS)
  if(MSVC)
    target_compile_options(main PRIVATE /WX)
  else()
    target_compile_options(main PRIVATE -Werror)
  endif()
endif()

# ------------------------------------------------------------------------------
# Release tuning
# ------------------------------------------------------------------------------
if(CMAKE_BUILD_TYPE STREQUAL "Release")
  if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_link_options(-s)
    if(SDLMS_NATIVE_OPTIMIZATIONS)
      target_compile_options(main PRIVATE -march=native -flto=auto)
      target_link_options(main PRIVATE -flto=auto)
    endif()
  endif()
endif()

# ------------------------------------------------------------------------------
# Formatting helper: `cmake --build build --target format`
# ------------------------------------------------------------------------------
find_program(CLANG_FORMAT_EXECUTABLE NAMES clang-format)
if(CLANG_FORMAT_EXECUTABLE)
  file(GLOB_RECURSE SDLMS_FORMAT_SOURCES
    ${PROJECT_SOURCE_DIR}/src/*.h
    ${PROJECT_SOURCE_DIR}/src/*.cpp)
  add_custom_target(format
    COMMAND ${CLANG_FORMAT_EXECUTABLE} -i ${SDLMS_FORMAT_SOURCES}
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
    COMMENT "Formatting sources with clang-format")
endif()
