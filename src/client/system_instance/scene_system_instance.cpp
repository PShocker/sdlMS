#include "scene_system_instance.h"
#include "cash_shop_system_instance.h"
#include "src/client/game/game_character.h"
#include "src/client/game/game_drop.h"
#include "src/client/game/game_mob.h"
#include "src/client/game_instance/audio_game_instance.h"
#include "src/client/game_instance/backgrnd_game_instance.h"
#include "src/client/game_instance/ball_game_instance.h"
#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/game_instance/character_game_instance.h"
#include "src/client/game_instance/character_stat_game_instance.h"
#include "src/client/game_instance/drop_game_instance.h"
#include "src/client/game_instance/effect_game_instance.h"
#include "src/client/game_instance/foothold_game_instance.h"
#include "src/client/game_instance/gain_log_game_instance.h"
#include "src/client/game_instance/ladderrope_game_instance.h"
#include "src/client/game_instance/map_info_game_instance.h"
#include "src/client/game_instance/mob_game_instance.h"
#include "src/client/game_instance/npc_game_instance.h"
#include "src/client/game_instance/obj_game_instance.h"
#include "src/client/game_instance/portal_game_instance.h"
#include "src/client/game_instance/reactor_game_instance.h"
#include "src/client/game_instance/seat_game_instance.h"
#include "src/client/game_instance/tile_game_instance.h"
#include "src/client/game_instance/tooltip_game_instance.h"
#include "src/client/system/input/keyboard_input_system.h"
#include "src/client/system/logic/audio_logic_system.h"
#include "src/client/system/logic/backgrnd_logic_system.h"
#include "src/client/system/logic/ball_logic_system.h"
#include "src/client/system/logic/camera_logic_system.h"
#include "src/client/system/logic/character_logic_system.h"
#include "src/client/system/logic/chatballoon_logic_system.h"
#include "src/client/system/logic/cursor_logic_system.h"
#include "src/client/system/logic/drop_logic_system.h"
#include "src/client/system/logic/effect_logic_system.h"
#include "src/client/system/logic/gain_log_logic_system.h"
#include "src/client/system/logic/mob_logic_system.h"
#include "src/client/system/logic/npc_logic_system.h"
#include "src/client/system/logic/obj_logic_system.h"
#include "src/client/system/logic/portal_logic_system.h"
#include "src/client/system/logic/reactor_logic_system.h"
#include "src/client/system/logic/skill_logic_system.h"
#include "src/client/system/render/backgrnd_render_system.h"
#include "src/client/system/render/ball_render_system.h"
#include "src/client/system/render/character_render_system.h"
#include "src/client/system/render/cursor_render_system.h"
#include "src/client/system/render/drop_render_system.h"
#include "src/client/system/render/effect_render_system.h"
#include "src/client/system/render/gain_log_render_system.h"
#include "src/client/system/render/mob_render_system.h"
#include "src/client/system/render/npc_render_system.h"
#include "src/client/system/render/obj_render_system.h"
#include "src/client/system/render/portal_render_system.h"
#include "src/client/system/render/reactor_render_system.h"
#include "src/client/system/render/tile_render_system.h"
#include "src/client/system/system.h"
#include "src/client/system/ui/minimap_ui_system.h"
#include "src/client/system/ui/buff_ui_system.h"
#include "src/client/system/ui/statusbar_ui_system.h"
#include "src/client/system_instance/fade_system_instance.h"
#include "src/client/system_instance/login_system_instance.h"
#include "src/client/window/window.h"
#include "src/common/flatbuffers/common.h"
#include "src/common/request/client_request.h"
#include "src/server/server_instance/server_character_instance.h"
#include <array>
#include <cstdint>
#include <cstdlib>
#include <optional>
#include <ranges>
#include <utility>
#include <vector>

bool scene_system_instance::render_game() {
  std::array<std::vector<game_character *>, 8> character_array;
  for (auto &other_data :
       character_game_instance::others | std::views::values) {
    auto &g_character = other_data.g_character;
    character_array[g_character.page].push_back(&g_character);
  }
  auto &self = character_game_instance::self;
  character_array[self.page].push_back(&self);

  std::array<std::vector<game_mob *>, 8> mob_array;
  for (auto &mob_data : mob_game_instance::data | std::views::values) {
    auto &mob = mob_data.mob;
    mob_array[mob.page].push_back(&mob);
  }

  std::array<std::vector<game_drop *>, 8> drop_array;
  for (auto &drop : drop_game_instance::data | std::views::values) {
    drop_array[drop.page].push_back(&drop);
  }
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
    for (auto &clock : obj_game_instance::clock) {
      obj_render_system::render_clock(clock);
    }
    for (auto &reactor : reactor_game_instance::data[i]) {
      reactor_render_system::render(reactor);
    }
    for (auto &effect : effect_game_instance::data[i]) {
      effect_render_system::render(effect);
    }
    for (auto &npc : npc_game_instance::data[i]) {
      npc_render_system::render(npc);
    }
    for (auto &mob : mob_array[i]) {
      mob_render_system::render(*mob);
    }
    for (auto &ball : ball_game_instance::data[i]) {
      ball_render_system::render(ball);
    }
    for (auto &character : character_array[i]) {
      character_render_system::render(*character);
    }
    for (auto &drop : drop_array[i]) {
      drop_render_system::render(*drop);
    }
  }
  // 传送门
  for (auto &portal : portal_game_instance::data | std::views::values) {
    portal_render_system::render(portal);
  }
  // 前景
  for (auto &f_backgrnd : backgrnd_game_instance::front | std::views::values) {
    backgrnd_render_system::render(f_backgrnd);
  }
  for (auto &npcs : npc_game_instance::data) {
    for (auto &npc : npcs) {
      npc_render_system::render_chatballoon(npc);
    }
  }
  for (auto &cs : character_array) {
    for (auto &c : cs) {
      character_render_system::render_chatballoon(*c);
    }
  }
  for (uint32_t i = 0; i < gain_log_game_instance::data.size(); i++) {
    gain_log_render_system::render(gain_log_game_instance::data[i], i);
  }
  return true;
}

void scene_system_instance::enter_prepare() {
  const auto &pos = scene_system_instance::prepare_pos;
  auto &self = character_game_instance::self;
  if (pos.has_value()) {
    self.pos = pos.value();
    self.action = u"jump";
    self.action_index = 0;
    self.action_time = 0;
  }
  character_logic_system::self_fh = 0;
  character_logic_system::self_lr = 0;
  character_logic_system::self_hspeed = 0;
  character_logic_system::self_vspeed = 0;

  self.tomb = std::nullopt;
}

void scene_system_instance::enter(uint32_t map_id) {
  scene_system_instance::map_id = map_id;
  enter_prepare();

  foothold_game_instance::data = foothold_game_instance::load(map_id);
  backgrnd_game_instance::load(map_id);
  ladderrope_game_instance::load(map_id);
  mob_game_instance::load(map_id);
  npc_game_instance::load(map_id);
  obj_game_instance::load(map_id);
  portal_game_instance::data = portal_game_instance::load(map_id);
  reactor_game_instance::load(map_id);
  tile_game_instance::load(map_id);
  tooltip_game_instance::load(map_id);
  seat_game_instance::load(map_id);
  minimap_ui_system::load();
  character_game_instance::others.clear();
  effect_game_instance::reset();
  camera_game_instance::reset();
  statusbar_ui_system::reset();
  ball_game_instance::reset();
  drop_game_instance::reset();
  auto bgm = map_info_game_instance::load_bgm(map_id);
  audio_game_instance::load_backgrnd_audio(bgm);

  system::event_systems = {
      minimap_ui_system::event,     statusbar_ui_system::event,
      buff_ui_system::event,  cursor_logic_system::event,
      keyboard_input_system::event,
  };
  system::logic_systems = {
      camera_logic_system::run,    backgrnd_logic_system::run,
      ball_logic_system::run,      effect_logic_system::run,
      character_logic_system::run, chatballoon_logic_system::run,
      drop_logic_system::run,      mob_logic_system::run,
      npc_logic_system::run,       portal_logic_system::run,
      audio_logic_system::run,     obj_logic_system::run,
      cursor_logic_system::run,    gain_log_logic_system::run,
      skill_logic_system::run,     reactor_logic_system::run,
  };
  system::render_systems = {
      render_game,
      buff_ui_system::render,
      statusbar_ui_system::render,
      minimap_ui_system::render,
      cursor_render_system::render,
  };

  fade_system_instance::enter_out();
  window::delta_time = 0;
}

SDL_FPoint scene_system_instance::load_prepare_pos(const std::u16string &pn,
                                                   uint8_t index) {
  SDL_FPoint r;
  game_portal *por;
  auto portals =
      portal_game_instance::load(scene_system_instance::prepare_map_id);
  if (index == 0) {
    // 只需要第一个元素，使用 find 更高效
    auto it = portals.find(pn);
    if (it != portals.end()) {
      por = &it->second;
    }
  } else {
    // 需要第 N 个元素（N>0）
    auto [first, last] = portals.equal_range(pn);
    if (std::distance(first, last) > index) {
      auto it = std::ranges::next(first, index);
      por = &it->second;
    }
  }
  r = por->pos;
  r.y -= 5;
  return r;
}

void scene_system_instance::enter_prepare(uint32_t map_id,
                                          const std::u16string &pn, int i) {
  prepare_map_id = map_id;
  prepare_pos = load_prepare_pos(pn, i);

  fbs::ClientSceneT client_scene;
  client_scene.fade = false;
  client_scene.map_id = map_id;

  client_request::send_to_host(client_scene);
}

void scene_system_instance::enter_fade() {
  auto g = character_game_instance::self;
  g.pos = prepare_pos.value();
  g.action = u"jump";
  auto c = server_character_instance::load_charactert(g);

  fbs::ClientSceneT client_scene;
  client_scene.fade = true;
  client_scene.map_id = prepare_map_id;
  client_scene.character = std::make_unique<fbs::CharacterT>(std::move(c));

  client_request::send_to_host(client_scene);
}

void scene_system_instance::enter_quit() {
  login_system_instance::enter();
  fade_system_instance::enter_out();
  fade_system_instance::enter_out();
}

void scene_system_instance::quit_prepare() {
  fbs::ClientSceneT client_scene;
  client_scene.fade = false;
  client_scene.map_id = 0;
  client_request::send_to_host(client_scene);
  fade_system_instance::enter_in(scene_system_instance::enter_quit);
}

void scene_system_instance::enter_cash_shop() {
  cash_shop_system_instance::enter();
  fade_system_instance::enter_out();
  fade_system_instance::enter_out();
}

void scene_system_instance::cash_shop_prepare() {
  fbs::ClientSceneT client_scene;
  client_scene.fade = false;
  client_scene.map_id = 0;
  client_request::send_to_host(client_scene);
  fade_system_instance::enter_in(scene_system_instance::enter_quit);
}