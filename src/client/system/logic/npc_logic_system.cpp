#include "npc_logic_system.h"
#include "src/client/game_instance/foothold_game_instance.h"
#include "src/client/game_instance/npc_game_instance.h"
#include "src/client/game_instance/random_game_instance.h"
#include "src/client/window/window.h"
#include "src/common/physic/physic.h"
#include "wz/Property.h"
#include <optional>
#include <random>
#include <string>

bool npc_logic_system::run_animate(game_npc &g_npc) {
  bool r = false;
  g_npc.ani_time += window::delta_time;
  auto npc_node = npc_game_instance::load_link_npc_node(g_npc.id);
  auto action_node = npc_node->get_child(g_npc.action);

  auto texture_node = action_node->get_child(std::to_string(g_npc.ani_index));

  int32_t delay = 100;
  auto delay_node = texture_node->get_child(u"delay");
  if (delay_node != nullptr) {
    delay = static_cast<wz::Property<int> *>(delay_node)->get();
  }
  if (g_npc.ani_time >= delay) {
    g_npc.ani_time = 0;
    g_npc.ani_index++;
    r = g_npc.ani_index >= action_node->children_count();
    g_npc.ani_index = g_npc.ani_index % action_node->children_count();
  }
  return r;
}

bool npc_logic_system::run_duration(game_npc &g_npc) {
  if (g_npc.duration > window::dt_now) {
    return false;
  }
  auto npc_node = npc_game_instance::load_link_npc_node(g_npc.id);
  auto child = *npc_node->get_children();
  child.erase(u"info");
  std::uniform_int_distribution<> dis(0, child.size() - 1);

  // flat_map 支持随机访问
  auto gen = random_game_instance::gen;
  auto it = std::next(child.begin(), dis(gen));
  auto random_key = it->first;

  g_npc.action = random_key;
  auto action_type = load_action_type(g_npc);
  if (action_type == action_enum::move) {
    std::bernoulli_distribution dist(0.5); // 50% 概率为 true
    bool random_bool = dist(gen);
    g_npc.hforce = random_bool ? 1400 : -1400;
  } else {
    g_npc.hforce = std::nullopt;
    g_npc.hspeed = std::nullopt;
  }
  g_npc.duration = window::dt_now + 1000;
  return true;
}

bool npc_logic_system::run_move(game_npc &g_npc) {
  // 移动
  auto delta_time = window::delta_time / 1000.0f;
  float vspeed = 0;
  auto r = physic::walk(g_npc.pos, delta_time, g_npc.hspeed.value(), vspeed,
                        g_npc.hforce.value(), -100, 100, 800, false, g_npc.fh,
                        {0, 0, 0, 0}, foothold_game_instance::data);
  return r;
}

npc_logic_system::action_enum
npc_logic_system::load_action_type(game_npc &g_npc) {
  if (g_npc.action == u"move") {
    return action_enum::move;
  }
  return action_enum::stand;
}

bool npc_logic_system::run() {
  for (auto &m : npc_game_instance::data) {
    for (auto &g_npc : m) {
      auto action_type = load_action_type(g_npc);
      switch (action_type) {
      case action_enum::move: {
        run_move(g_npc);
        break;
      }
      default: {
        break;
      }
      }
      if (run_animate(g_npc)) {
        run_duration(g_npc);
      }
    }
  }
  return true;
}