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
set(SDLMS_COMMON_SOURCES
  src/common/ffmpeg/ffmpeg.cpp
  src/common/freetype/freetype.cpp
  src/common/physic/physic.cpp
  src/common/request/client_request.cpp
  src/common/response/server_response.cpp
  src/common/script/consume/consume_2430000.cpp
  src/common/script/consume/consume_2430001.cpp
  src/common/script/npc/cab_ant_104.cpp
  src/common/script/npc/goldstein_111.cpp
  src/common/script/npc/jane_103.cpp
  src/common/script/npc/npc_101.cpp
  src/common/script/npc/npc_14.cpp
  src/common/script/npc/npc_15.cpp
  src/common/script/npc/npc_18.cpp
  src/common/script/npc/npc_19.cpp
  src/common/script/npc/npc_8.cpp
  src/common/script/npc/q1002e.cpp
  src/common/script/npc/q1002s.cpp
  src/common/script/npc/quest_1005.cpp
  src/common/script/reactor/q90008.cpp
  src/common/wz/wz_resource.cpp
)

set(SDLMS_CLIENT_SOURCES
  src/client/game_instance/afterimage_game_instance.cpp
  src/client/system/ui/component/button.cpp
  src/client/game_instance/audio_game_instance.cpp
  src/client/game_instance/backgrnd_game_instance.cpp
  src/client/game_instance/ball_game_instance.cpp
  src/client/game_instance/camera_game_instance.cpp
  src/client/game_instance/character_game_instance.cpp
  src/client/game_instance/character_stat_game_instance.cpp
  src/client/game_instance/clock_game_instance.cpp
  src/client/game_instance/cursor_game_instance.cpp
  src/client/game_instance/drop_game_instance.cpp
  src/client/game_instance/effect_game_instance.cpp
  src/client/game_instance/equip_game_instance.cpp
  src/client/game_instance/foothold_game_instance.cpp
  src/client/game_instance/gain_log_game_instance.cpp
  src/client/game_instance/item_game_instance.cpp
  src/client/game_instance/job_skill_game_instance.cpp
  src/client/game_instance/keyboard_game_instance.cpp
  src/client/game_instance/ladderrope_game_instance.cpp
  src/client/game_instance/map_info_game_instance.cpp
  src/client/game_instance/mob_game_instance.cpp
  src/client/game_instance/npc_game_instance.cpp
  src/client/game_instance/obj_game_instance.cpp
  src/client/game_instance/package_game_instance.cpp
  src/client/game_instance/party_game_instance.cpp
  src/client/game_instance/popup_tip_game_instance.cpp
  src/client/game_instance/portal_game_instance.cpp
  src/client/game_instance/quest_game_instance.cpp
  src/client/game_instance/reactor_game_instance.cpp
  src/client/game_instance/seat_game_instance.cpp
  src/client/game_instance/shader_game_instance.cpp
  src/client/game_instance/shop_game_instance.cpp
  src/client/game_instance/skill_game_instance.cpp
  src/client/game_instance/storage_game_instance.cpp
  src/client/game_instance/text_game_instance.cpp
  src/client/game_instance/tile_game_instance.cpp
  src/client/game_instance/tooltip_game_instance.cpp
  src/client/game_instance/trap_game_instance.cpp
  src/client/game_instance/triangle_game_instance.cpp
  src/client/game_skill/bandit.cpp
  src/client/game_skill/beginner.cpp
  src/client/game_skill/bowman.cpp
  src/client/game_skill/chief_bandit.cpp
  src/client/game_skill/cleric.cpp
  src/client/game_skill/crossbowman.cpp
  src/client/game_skill/crusader.cpp
  src/client/game_skill/fp_wizard.cpp
  src/client/game_skill/hermit.cpp
  src/client/game_skill/hunter.cpp
  src/client/game_skill/il_wizard.cpp
  src/client/game_skill/magician.cpp
  src/client/game_skill/ranger.cpp
  src/client/game_skill/sniper.cpp
  src/client/game_skill/thief.cpp
  src/client/game_skill/warrior.cpp
  src/client/system/input/input_system.cpp
  src/client/system/input/keyboard_input_system.cpp
  src/client/system/logic/audio_logic_system.cpp
  src/client/system/logic/backgrnd_logic_system.cpp
  src/client/system/logic/ball_logic_system.cpp
  src/client/system/logic/camera_logic_system.cpp
  src/client/system/logic/character_logic_system.cpp
  src/client/system/logic/chatballoon_logic_system.cpp
  src/client/system/logic/cursor_logic_system.cpp
  src/client/system/logic/drop_logic_system.cpp
  src/client/system/logic/effect_logic_system.cpp
  src/client/system/logic/item_logic_system.cpp
  src/client/system/logic/mob_logic_system.cpp
  src/client/system/logic/npc_logic_system.cpp
  src/client/system/logic/obj_logic_system.cpp
  src/client/system/logic/pet_logic_system.cpp
  src/client/system/logic/portal_logic_system.cpp
  src/client/system/logic/reactor_logic_system.cpp
  src/client/system/logic/skill_logic_system.cpp
  src/client/system/logic/trap_logic_system.cpp
  src/client/system/render/backgrnd_render_system.cpp
  src/client/system/render/ball_render_system.cpp
  src/client/system/render/character_render_system.cpp
  src/client/system/render/chatballoon_render_system.cpp
  src/client/system/render/cursor_render_system.cpp
  src/client/system/render/debuff_render_system.cpp
  src/client/system/render/drop_render_system.cpp
  src/client/system/render/effect_render_system.cpp
  src/client/system/render/gauge_render_system.cpp
  src/client/system/render/mob_render_system.cpp
  src/client/system/render/npc_render_system.cpp
  src/client/system/render/obj_render_system.cpp
  src/client/system/render/pet_render_system.cpp
  src/client/system/render/portal_render_system.cpp
  src/client/system/render/reactor_render_system.cpp
  src/client/system/render/tile_render_system.cpp
  src/client/system/ui/beauty_ui_system.cpp
  src/client/system/ui/buff_ui_system.cpp
  src/client/system/ui/cash_shop_ui_system.cpp
  src/client/system/ui/character_choose_ui_system.cpp
  src/client/system/ui/character_create_ui_system.cpp
  src/client/system/ui/character_info_ui_system.cpp
  src/client/system/ui/character_stat_ui_system.cpp
  src/client/system/ui/chat_log_ui_system.cpp
  src/client/system/ui/clock_ui_system.cpp
  src/client/system/ui/combo_box_ui_system.cpp
  src/client/system/ui/context_menu_ui_system.cpp
  src/client/system/ui/craft_ui_system.cpp
  src/client/system/ui/equip_ui_system.cpp
  src/client/system/ui/gain_log_ui_system.cpp
  src/client/system/ui/keybinding_ui_system.cpp
  src/client/system/ui/login_notice_ui_system.cpp
  src/client/system/ui/login_ui_system.cpp
  src/client/system/ui/minigame_card_ui_system.cpp
  src/client/system/ui/minigame_omok_ui_system.cpp
  src/client/system/ui/minimap_ui_system.cpp
  src/client/system/ui/notice_ui_system.cpp
  src/client/system/ui/npc_dlg_ui_system.cpp
  src/client/system/ui/option_ui_system.cpp
  src/client/system/ui/package_ui_system.cpp
  src/client/system/ui/party_ui_system.cpp
  src/client/system/ui/player_shop_ui_system.cpp
  src/client/system/ui/popup_tip_ui_system.cpp
  src/client/system/ui/quest_alarm_ui_system.cpp
  src/client/system/ui/quest_ui_system.cpp
  src/client/system/ui/quick_slot_ui_system.cpp
  src/client/system/ui/revive_ui_system.cpp
  src/client/system/ui/screen_notice_ui_system.cpp
  src/client/system/ui/scroll_ui_system.cpp
  src/client/system/ui/shop_ui_system.cpp
  src/client/system/ui/skill_ui_system.cpp
  src/client/system/ui/slider_ui_system.cpp
  src/client/system/ui/statusbar_menu_ui_system.cpp
  src/client/system/ui/statusbar_ui_system.cpp
  src/client/system/ui/storage_ui_system.cpp
  src/client/system/ui/text_input_ui_system.cpp
  src/client/system/ui/tooltip_ui_system.cpp
  src/client/system/ui/trade_ui_system.cpp
  src/client/system/ui/video_ui_system.cpp
  src/client/system/ui/worldmap_ui_system.cpp
  src/client/system_instance/cash_shop_system_instance.cpp
  src/client/system_instance/character_choose_system_instance.cpp
  src/client/system_instance/character_create_system_instance.cpp
  src/client/system_instance/fade_system_instance.cpp
  src/client/system_instance/game_save_system_instance.cpp
  src/client/system_instance/login_notice_system_instance.cpp
  src/client/system_instance/login_system_instance.cpp
  src/client/system_instance/scene_system_instance.cpp
  src/client/window/window.cpp
)

set(SDLMS_SERVER_SOURCES
  src/server/request_handler.cpp
  src/server/server_instance/server_ball_instance.cpp
  src/server/server_instance/server_character_instance.cpp
  src/server/server_instance/server_client_instance.cpp
  src/server/server_instance/server_drop_instance.cpp
  src/server/server_instance/server_foothold_instance.cpp
  src/server/server_instance/server_mob_instance.cpp
  src/server/server_instance/server_party_instance.cpp
  src/server/server_instance/server_reactor_instance.cpp
  src/server/server_instance/server_region_skill_instance.cpp
  src/server/server_instance/server_scene_instance.cpp
  src/server/server_instance/server_trade_instance.cpp
  src/server/server_instance/server_trap_instance.cpp
  src/server/server_main.cpp
  src/server/server_system/server_drop_system.cpp
  src/server/server_system/server_heartbeat_system.cpp
  src/server/server_system/server_mob_system.cpp
  src/server/server_system/server_reactor_system.cpp
  src/server/server_system/server_region_skill_system.cpp
  src/server/server_system_instance/server_system_instance.cpp
)

# ------------------------------------------------------------------------------
# Target
# ------------------------------------------------------------------------------
add_executable(main
  src/client/main.cpp
  ${SDLMS_COMMON_SOURCES}
  ${SDLMS_CLIENT_SOURCES}
  ${SDLMS_SERVER_SOURCES})

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
