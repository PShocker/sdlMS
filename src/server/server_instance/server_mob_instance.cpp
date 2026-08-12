#include "server_mob_instance.h"
#include "server_client_instance.h"
#include "server_scene_instance.h"
#include "src/client/game/game_gain_log.h"
#include "src/client/game_instance/effect_game_instance.h"
#include "src/client/game_instance/foothold_game_instance.h"
#include "src/client/game_instance/gain_log_game_instance.h"
#include "src/client/game_instance/mob_game_instance.h"
#include "src/client/system/logic/mob_logic_system.h"
#include "src/client/system_instance/scene_system_instance.h"
#include "src/client/window/window.h"
#include "src/common/flatbuffers/server.h"
#include "src/common/response/server_response.h"
#include "src/common/wz/wz_resource.h"
#include "src/server/server/server_mob.h"
#include "src/server/server_instance/server_drop_instance.h"
#include "wz/Property.h"
#include <cstdint>
#include <flat_map>
#include <memory>
#include <optional>

void server_mob_instance::load_mob(server_scene &scene) {
  std::flat_map<uint32_t, server_mob> data;
  auto map_id = scene.map_id;
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
    server_mob mob;
    auto mob_node = val[0];
    mob.index = std::stoi(std::string{key.begin(), key.end()});
    mob.id =
        static_cast<wz::Property<std::u16string> *>(mob_node->get_child(u"id"))
            ->get();
    mob.fh =
        static_cast<wz::Property<int> *>(mob_node->get_child(u"fh"))->get();
    mob.rx0 =
        static_cast<wz::Property<int> *>(mob_node->get_child(u"rx0"))->get();
    mob.rx1 =
        static_cast<wz::Property<int> *>(mob_node->get_child(u"rx1"))->get();

    auto x = static_cast<wz::Property<int> *>(mob_node->get_child(u"x"))->get();
    auto m_fh = fhs.at(mob.fh);
    auto y = m_fh.k.value() * x + m_fh.intercept.value();
    mob.pos = {static_cast<float>(x), static_cast<float>(y)};
    mob.page = m_fh.page;
    // default action
    mob_node = wz_resource::mob->find(mob.id + u".img");
    if (mob_node->get_child(u"info")->get_child("flySpeed")) {
      mob.action = u"fly";
      mob.type = server_mob::mob_type::fly;
    } else {
      mob.action = u"stand";
      mob.type = server_mob::mob_type::stand;
    }
    mob.duration = window::dt_now;

    auto info_node = mob_game_instance::load_mob_info(mob.id);
    mob.hp =
        static_cast<wz::Property<int> *>(info_node->get_child(u"maxHP"))->get();
    mob.mp =
        static_cast<wz::Property<int> *>(info_node->get_child(u"maxMP"))->get();
    if (info_node->get_child(u"speed") != nullptr) {
      mob.hspeed =
          static_cast<wz::Property<int> *>(info_node->get_child(u"speed"))
              ->get();
      mob.hspeed = -1 * (float)(mob.hspeed + 100) / 100 * 125;
      mob.hspeed = (float)(mob.hspeed + 100) / 100 * 125;
    }
    data[mob.index] = mob;
  }
  scene.mobs = data;
}

void server_mob_instance::handle_attack(uint64_t client_id,
                                        ClientMobAttackT &r) {
  if (!server_client_instance::clients.contains(client_id)) {
    return;
  }
  // 转发
  auto map_id = r.map_id;
  auto clients = server_scene_instance::scenes.at(map_id).clients;
  clients.erase(client_id);
  ServerMobAttackT t;
  t.client_id = client_id;
  t.payload = std::move(r.payload);
  for (auto c : clients) {
    server_response::send_to_client(c, t);
  }
}

void server_mob_instance::handle_server_mv(const ServerMobMvT &m) {
  auto &mvs = mob_game_instance::data.at(m.mob_index).mvs;
  mvs.push_back(*m.payload);
}

void server_mob_instance::handle_server_flip(const ServerMobFlipT &m) {
  auto &mob = mob_game_instance::data.at(m.mob_index).mob;
  mob.flip = m.payload->flip;
}

void server_mob_instance::handle_server_action(const ServerMobActionT &m) {
  auto &mob = mob_game_instance::data.at(m.mob_index).mob;
  const auto &a = m.payload;
  mob.action = std::u16string{a->action.begin(), a->action.end()};
  mob.ani_index = 0;
  mob.ani_time = 0;
  mob.ani_animate = a->action_animate;
}

void server_mob_instance::handle_server_state(const ServerMobStateT &m) {
  auto &mob = mob_game_instance::data.at(m.mob_index).mob;
  for (const auto &s : m.payload) {
    switch (s->state) {
    case fbs::StateEnum_HP: {
      mob.hp = s->val;
      break;
    }
    default: {
      break;
    }
    }
  }
}

void server_mob_instance::handle_server_die(const ServerMobDieT &m) {
  auto &mob = mob_game_instance::data.at(m.mob_index).mob;
  float percent = (float)mob.attack_val / mob.max_hp;
  if (percent) {
    auto info_node = mob_game_instance::load_link_mob_node(mob.id);
    auto max_exp =
        static_cast<wz::Property<int> *>(info_node->get_child(u"exp"))->get();
    auto exp = percent * max_exp;
    game_gain_log g_log{
        .id = u"",
        .num = static_cast<uint64_t>(exp),
        .destroy = window::dt_now + 5000,
        .type = gain_enum::experience,
    };
    gain_log_game_instance::data.push_back(g_log);
  }
  mob.attack_val = 0;
}

void server_mob_instance::handle_server_drop(const ServerMobDropT &m) {
  // Drop
  for (const auto &dt : m.drop) {
    server_drop_instance::handle_server_dt(*dt);
  }
  return;
}

void server_mob_instance::handle_server_event(const ServerMobEventT &m) {
  if (m.map_id != scene_system_instance::map_id) {
    return;
  }
  for (const auto &ev : m.payload) {
    switch (ev.type) {
    case fbs::MobEventUnion_ServerMobMv: {
      const auto mv = ev.AsServerMobMv();
      handle_server_mv(*mv);
      break;
    }
    case fbs::MobEventUnion_ServerMobFlip: {
      const auto fp = ev.AsServerMobFlip();
      handle_server_flip(*fp);
      break;
    }
    case fbs::MobEventUnion_ServerMobAction: {
      const auto action = ev.AsServerMobAction();
      handle_server_action(*action);
      break;
    }
    case fbs::MobEventUnion_ServerMobState: {
      const auto st = ev.AsServerMobState();
      handle_server_state(*st);
      break;
    }
    case fbs::MobEventUnion_ServerMobDie: {
      const auto d = ev.AsServerMobDie();
      handle_server_die(*d);
      break;
    }
    case fbs::MobEventUnion_ServerMobDrop: {
      const auto d = ev.AsServerMobDrop();
      handle_server_drop(*d);
      break;
    }
    default: {
      break;
    }
    }
  }
}

void server_mob_instance::handle_s_attack(uint64_t client_id, AttackT &at) {
  const auto &attack = at;
  // 伤害数字
  damage_data data = {
      .num = attack.num,
      .type = (damage_data::t)attack.type,
  };
  game_effect d = {
      .id = u"",
      .index = 0,
      .time = 0,
      .delay = attack.delay,
      .type = game_effect::effect_type::damage,
      .pos = SDL_FPoint{attack.x, attack.y - 10},
      .z = true,
      .flip = false,
      .data = data,
  };
  effect_game_instance::data[7].emplace_back(d);
}

void server_mob_instance::handle_server_attack(ServerMobAttackT &r) {
  handle_s_attack(r.client_id, *r.payload);
}

void server_mob_instance::hanle_server_mob(
    const std::unique_ptr<fbs::MobT> &m) {
  auto &data = mob_game_instance::data;
  if (data.contains(m->mob_index)) {
    auto &mob = data.at(m->mob_index).mob;
    const auto &state = m->state;
    mob.action = std::u16string{state->action.begin(), state->action.end()};
    mob.page = state->page;
    mob.flip = state->flip;
    mob.pos.x = state->x;
    mob.pos.y = state->y;
    mob.hp = m->mob_hp;

    auto action_type = mob_logic_system::load_action_type(mob.action);
    if (action_type == mob_logic_system::action_enum::die) {
      mob_logic_system::run_revive(mob);
    }
  } else {
    // summon mob
  }
}