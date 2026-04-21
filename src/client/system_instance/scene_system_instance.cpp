#include "scene_system_instance.h"
#include "src/client/game/game_character.h"
#include "src/client/game_instance/backgrnd_game_instance.h"
#include "src/client/game_instance/character_game_instance.h"
#include "src/client/game_instance/drop_game_instance.h"
#include "src/client/game_instance/foothold_game_instance.h"
#include "src/client/game_instance/ladderrope_game_instance.h"
#include "src/client/game_instance/map_info_game_instance.h"
#include "src/client/game_instance/mob_game_instance.h"
#include "src/client/game_instance/npc_game_instance.h"
#include "src/client/game_instance/obj_game_instance.h"
#include "src/client/game_instance/portal_game_instance.h"
#include "src/client/game_instance/reactor_game_instance.h"
#include "src/client/game_instance/tile_game_instance.h"
#include "src/client/game_instance/tooltip_game_instance.h"
#include "src/client/system/logic/backgrnd_logic_system.h"
#include "src/client/system/logic/ball_logic_system.h"
#include "src/client/system/logic/camera_logic_system.h"
#include "src/client/system/logic/character_logic_system.h"
#include "src/client/system/logic/chatballoon_logic_system.h"
#include "src/client/system/logic/drop_logic_system.h"
#include "src/client/system/logic/mob_logic_system.h"
#include "src/client/system/logic/obj_logic_system.h"
#include "src/client/system/logic/portal_logic_system.h"
#include "src/client/system/logic/sound_logic_system.h"
#include "src/client/system/render/backgrnd_render_system.h"
#include "src/client/system/render/character_render_system.h"
#include "src/client/system/render/drop_render_system.h"
#include "src/client/system/render/mob_render_system.h"
#include "src/client/system/render/npc_render_system.h"
#include "src/client/system/render/obj_render_system.h"
#include "src/client/system/render/portal_render_system.h"
#include "src/client/system/render/reactor_render_system.h"
#include "src/client/system/render/tile_render_system.h"
#include "src/client/system/system.h"
#include "src/client/system/ui/minimap_ui_system.h"
#include "src/client/system/ui/statusbar_ui_system.h"
#include <array>
#include <cstdint>
#include <cstdlib>
#include <ranges>
#include <vector>

uint8_t
scene_system_instance::load_character_layer(game_character &g_character) {
  auto fh = g_character.fh;
  return foothold_game_instance::data.contains(fh)
             ? foothold_game_instance::data.at(fh).page
             : 7;
}

bool scene_system_instance::render_game() {
  std::array<std::vector<game_character *>, 8> character_array;
  for (auto &character : character_game_instance::others | std::views::values) {
    character_array[load_character_layer(character)].push_back(&character);
  }
  auto &self = character_game_instance::self;
  auto self_layer = load_character_layer(self);
  character_array[self_layer].push_back(&self);
  //   后景
  for (auto &b_backgrnd : backgrnd_game_instance::back | std::views::values) {
    backgrnd_render_system::render(b_backgrnd);
  }
  for (uint8_t i = 0; i < 8; i++) {
    for (auto &obj : obj_game_instance::data[i] | std::views::values) {
      obj_render_system::render(obj);
    }
    for (auto &tile : tile_game_instance::data[i] | std::views::values) {
      tile_render_system::render(tile);
    }
    for (auto &reactor : reactor_game_instance::data[i]) {
      reactor_render_system::render(reactor);
    }
    for (auto &npc : npc_game_instance::data[i]) {
      npc_render_system::render(npc);
    }
    for (auto &mob : mob_game_instance::data[i]) {
      mob_render_system::render(mob);
    }
    for (auto &character : character_array[i]) {
      character_render_system::render(*character);
    }
    for (auto &drop : drop_game_instance::data[i]) {
      drop_render_system::render(drop);
    }
  }
  // 传送门
  for (auto &portal : portal_game_instance::data) {
    portal_render_system::render(portal);
  }
  // 前景
  for (auto &f_backgrnd : backgrnd_game_instance::front | std::views::values) {
    backgrnd_render_system::render(f_backgrnd);
  }

  return true;
}

void scene_system_instance::enter(uint32_t map_id) {
  scene_system_instance::map_id = map_id;

  map_info_game_instance::load(map_id);
  foothold_game_instance::data = foothold_game_instance::load(map_id);
  backgrnd_game_instance::load(map_id);
  ladderrope_game_instance::load(map_id);
  mob_game_instance::data = mob_game_instance::load(map_id);
  npc_game_instance::load(map_id);
  obj_game_instance::load(map_id);
  portal_game_instance::load(map_id);
  reactor_game_instance::load(map_id);
  tile_game_instance::load(map_id);
  tooltip_game_instance::load(map_id);

  system::event_systems = {

  };
  system::logic_systems = {
      backgrnd_logic_system::run,    ball_logic_system::run,
      camera_logic_system::run,      character_logic_system::run,
      chatballoon_logic_system::run, drop_logic_system::run,
      mob_logic_system::run,         portal_logic_system::run,
      sound_logic_system::run,       obj_logic_system::run};
  system::render_systems = {
      render_game,
      minimap_ui_system::render,
      statusbar_ui_system::render,
  };
}