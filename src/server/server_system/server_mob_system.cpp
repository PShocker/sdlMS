#include "server_mob_system.h"
#include "src/client/window/window.h"
#include "src/common/physic/physic.h"
#include "src/server/server_instance/server_mob_instance.h"
#include "src/server/server_instance/server_scene_instance.h"
#include <ranges>

static uint32_t map_id = 0;

server_mob_system::action_enum
server_mob_system::load_action_type(server_mob &s_mob) {
  const static std::flat_map<std::u16string, action_enum> map_name = {
      {u"stand1", action_enum::stand},
      {u"stand2", action_enum::stand},
  };
  return map_name.at(s_mob.mob.action);
}

bool server_mob_system::run_action(server_mob &s_mob,
                                   const std::u16string &action) {
  if (s_mob.mob.action == action) {
    return false;
  }
  s_mob.mob.action = action;
  s_mob.mob.ani_index = 0;
  s_mob.mob.ani_time = 0;
  return true;
}

void server_mob_system::run_walk(server_mob &s_mob) {
  run_action(s_mob, u"move");
  // 移动
  auto delta_time = window::delta_time / 1000.0f;
  auto s_fhs = server_scene_instance::scenes.at(map_id).fhs;
  std::flat_map<int32_t, game_foothold> g_fhs;
  for (auto [key, value] : s_fhs) {
    g_fhs.emplace(key, std::move(value.fh)); // 移动避免拷贝
  }
  auto r = physic::walk(s_mob.mob.pos, delta_time, s_mob.hspeed, s_mob.vspeed,
                        s_mob.hforce, -100.0f, 100.0f, 800, true, s_mob.mob.fh,
                        {0, 0, 0, 0}, g_fhs);
}

void server_mob_system::run_state_machine(server_mob &s_mob) {
  auto o_mob = s_mob;
  auto m_action = load_action_type(s_mob);
  switch (m_action) {
  case action_enum::remove: {
    break;
  }
  case action_enum::stand: {
    break;
  }
  case action_enum::hit: {
    break;
  }
  case action_enum::jump: {
    break;
  }
  case action_enum::swim:
  case action_enum::fly: {
    break;
  }
  }
}

bool server_mob_system::run() {
  auto &scenes = server_scene_instance::scenes;
  for (auto &sc : scenes | std::views::values) {
    if (sc.mobs.empty()) {
      continue;
    }
    map_id = sc.map_id;
    for (auto &mob : sc.mobs | std::views::values) {
      run_state_machine(mob);
    }
  }
  return true;
}
