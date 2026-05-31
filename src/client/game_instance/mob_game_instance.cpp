#include "mob_game_instance.h"
#include "SDL3/SDL_rect.h"
#include "effect_game_instance.h"
#include "foothold_game_instance.h"
#include "src/client/game/game_effect.h"
#include "src/client/game/game_mob.h"
#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/system/logic/mob_logic_system.h"
#include "src/client/system_instance/scene_system_instance.h"
#include "src/common/flatbuffers/server.h"
#include "src/common/wz/wz_resource.h"
#include "wz/Property.h"
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

wz::Node *mob_game_instance::load_link_mob_node(const std::u16string &id) {
  auto mob_node = wz_resource::mob->find(id + u".img");
  while (mob_node->find(u"info/link")) {
    auto link_node = mob_node->find(u"info/link");
    auto link = static_cast<wz::Property<std::u16string> *>(link_node)->get();
    mob_node = wz_resource::mob->find(link + u".img");
  }
  return mob_node;
}

void mob_game_instance::load(uint32_t map_id) {
  data = {};

  auto map_node = wz_resource::load_map_node(map_id);
  auto map_life_node = map_node->get_child(u"life");
  auto fhs = foothold_game_instance::load(map_id);

  for (auto [key, val] : *map_life_node->get_children()) {
    auto type =
        static_cast<wz::Property<std::u16string> *>(val[0]->get_child(u"type"))
            ->get();
    if (type == u"n") {
      continue;
    }
    game_mob g_mob;
    auto mob_node = val[0];
    g_mob.index = std::stoi(std::string{key.begin(), key.end()});
    g_mob.id =
        static_cast<wz::Property<std::u16string> *>(mob_node->get_child(u"id"))
            ->get();
    g_mob.ani_index = 0;
    g_mob.ani_time = 0;
    g_mob.ani_animate = true;

    data[g_mob.index] = {g_mob, {}};
  }
}

void mob_game_instance::load_server_mob(
    const std::vector<std::unique_ptr<fbs::MobT>> &v) {
  for (const auto &m : v) {
    if (data.contains(m->mob_index)) {
      auto &mob = data.at(m->mob_index).mob;
      const auto &state = m->state;
      mob.action = std::u16string{state->action.begin(), state->action.end()};
      mob.page = state->page;
      mob.flip = state->flip;
      mob.pos.x = state->x;
      mob.pos.y = state->y;

      auto action_type = mob_logic_system::load_action_type(mob.action);
      if (action_type == mob_logic_system::action_enum::die) {
        mob_logic_system::run_revice(mob);
      }
    } else {
      // summon mob
    }
  }
}

void mob_game_instance::server_mob_logic(const ServerMobLogicT &v) {
  if (v.map_id != scene_system_instance::map_id) {
    return;
  }
  for (const auto &m : v.payload) {
    if (data.contains(m->mob_index)) {
      auto &logics = data.at(m->mob_index).logics;
      logics[m->payload.type].push_back(m->payload);
    }
  }
}

void mob_game_instance::load_mob_attack(uint64_t client_id,
                                        const MobAttackT *m) {
  // 伤害数字
  damage_data data = {
      .num = m->attack->num,
      .type = damage_data::red,
  };
  if (client_id == 0) {
    data.type = damage_data::viole;
  }
  game_effect d = {
      .id = u"",
      .index = 0,
      .time = 0,
      .delay = m->attack->delay,
      .type = game_effect::effect_type::damage,
      .pos = SDL_FPoint{m->attack->x, m->attack->y - 10},
      .z = true,
      .flip = false,
      .data = data,
  };
  effect_game_instance::data[7].emplace_back(d);
}