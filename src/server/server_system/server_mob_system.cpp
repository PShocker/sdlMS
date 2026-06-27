#include "server_mob_system.h"
#include "SDL3/SDL_rect.h"
#include "src/client/game_instance/item_game_instance.h"
#include "src/client/game_instance/map_info_game_instance.h"
#include "src/client/game_instance/random_game_instance.h"
#include "src/client/system/logic/mob_logic_system.h"
#include "src/client/window/window.h"
#include "src/common/flatbuffers/common.h"
#include "src/common/flatbuffers/server.h"
#include "src/common/physic/physic.h"
#include "src/common/response/server_response.h"
#include "src/common/wz/wz_resource.h"
#include "src/server/server_instance/server_client_instance.h"
#include "src/server/server_instance/server_mob_instance.h"
#include "src/server/server_instance/server_scene_instance.h"
#include "wz/Property.h"
#include <algorithm>
#include <chrono>
#include <cstdint>
#include <flat_map>
#include <flat_set>
#include <memory>
#include <ranges>
#include <string>
#include <utility>
#include <vector>

std::vector<server_mob_system::mob_drop>
server_mob_system::load_mob_drops(server_mob &s_mob) {
  static std::flat_map<std::u16string, std::vector<mob_drop>> cache;
  if (!cache.contains(s_mob.id)) {
    auto node = wz_resource::ms->get_root()->find(u"MobDrop/" + s_mob.id);
    for (auto [k, v] : *node->get_children()) {
      auto min_quantity =
          static_cast<wz::Property<int> *>(v[0]->get_child(u"min_quantity"))
              ->get();
      auto max_quantity =
          static_cast<wz::Property<int> *>(v[0]->get_child(u"max_quantity"))
              ->get();
      auto chance =
          static_cast<wz::Property<int> *>(v[0]->get_child(u"chance"))->get();
      mob_drop md;
      md.id = k;
      md.min_quantity = min_quantity;
      md.max_quantity = max_quantity;
      md.rate = chance / 1000000.0f;
      cache[s_mob.id].push_back(md);
    }
  }
  return cache.at(s_mob.id);
}

void server_mob_system::run_network_movement_sync(server_mob &s_mob,
                                                  server_mob &o_mob) {
  bool position_changed =
      (o_mob.pos.x != s_mob.pos.x || o_mob.pos.y != s_mob.pos.y);
  if (!position_changed)
    return;

  // 构造当前 movement
  MovementT mv;
  mv.x1 = o_mob.pos.x;
  mv.y1 = o_mob.pos.y;
  mv.x2 = s_mob.pos.x;
  mv.y2 = s_mob.pos.y;
  mv.page = s_mob.page;
  // 30帧补60帧
  mv.time = std::min(window::delta_time, 33);
  MobLogicT req;
  req.mob_index = s_mob.index;
  req.payload.Set(mv);
  unique_logics.push_back(std::make_unique<MobLogicT>(req));
}

void server_mob_system::run_network_action_sync(server_mob &s_mob,
                                                server_mob &o_mob) {
  if (o_mob.action != s_mob.action) {
    MobLogicT req;
    ActionT a;
    a.action = std::string{s_mob.action.begin(), s_mob.action.end()};
    a.action_animate = true;
    a.action_index = 0;
    req.mob_index = s_mob.index;
    req.payload.Set(a);
    unique_logics.push_back(std::make_unique<MobLogicT>(req));
  }
}

void server_mob_system::run_network_flip_sync(server_mob &s_mob,
                                              server_mob &o_mob) {
  if (o_mob.flip != s_mob.flip) {
    MobLogicT req;
    FlipT f;
    f.flip = s_mob.flip;
    req.mob_index = s_mob.index;
    req.payload.Set(f);
    unique_logics.push_back(std::make_unique<MobLogicT>(req));
  }
}

void server_mob_system::run_network_sync(server_mob &s_mob, server_mob &o_mob) {
  run_network_movement_sync(s_mob, o_mob);
  run_network_flip_sync(s_mob, o_mob);
  run_network_action_sync(s_mob, o_mob);
}

void server_mob_system::run_walk(server_mob &s_mob) {
  // 移动
  const auto &s_fhs = server_scene_instance::scenes.at(map_id).fhs;
  std::flat_map<int32_t, game_foothold> g_fhs;
  for (const auto &[key, value] : s_fhs) {
    g_fhs.emplace(key, value.fh);
  }

  SDL_FRect border;
  border.x = s_mob.rx0;
  border.w = s_mob.rx1;

  auto r = physic::walk(s_mob.pos, delta_time / 1000.0f, s_mob.hspeed,
                        s_mob.vspeed, s_mob.hforce, -80.0f, 80.0f, 0, false,
                        s_mob.fh, border, g_fhs);
}

void server_mob_system::run_duration(server_mob &s_mob) {

  if (s_mob.duration > window::dt_now) {
    return;
  }
  auto action_type = mob_logic_system::load_action_type(s_mob.action);
  if (action_type == mob_logic_system::action_enum::hit ||
      action_type == mob_logic_system::action_enum::die) {
    return;
  }
  static const std::flat_map<mob_logic_system::action_enum, std::u16string>
      actions = {
          {mob_logic_system::action_enum::stand, u"stand"},
          {mob_logic_system::action_enum::move, u"move"},
          {mob_logic_system::action_enum::fly, u"fly"},
      };
  std::flat_set<mob_logic_system::action_enum> actions2;
  switch (s_mob.type) {
  case server_mob::mob_type::stand: {
    if (s_mob.hate_id != 0) {
      actions2 = {
          mob_logic_system::action_enum::move,
      };

    } else {
      actions2 = {
          mob_logic_system::action_enum::stand,
          mob_logic_system::action_enum::move,
      };
    }
    break;
  }
  case server_mob::mob_type::swim:
  case server_mob::mob_type::fly: {
    break;
  }
  }
  auto &gen = random_game_instance::gen;
  std::uniform_int_distribution<size_t> dist(0, actions2.size() - 1);
  auto it = std::next(actions2.begin(), dist(gen));
  auto selected = *it;
  s_mob.action = actions.at(selected);
  switch (selected) {
  case mob_logic_system::action_enum::stand: {
    s_mob.duration = window::dt_now + 500;
    break;
  }
  case mob_logic_system::action_enum::jump: {
    break;
  }
  case mob_logic_system::action_enum::move: {
    bool left = false;
    const auto &clients = server_client_instance::clients;
    if (s_mob.hate_id != 0 && clients.contains(s_mob.hate_id)) {
      auto &g_character = clients.at(s_mob.hate_id).player_t.character;
      auto &g_x = g_character->state->x;
      left = g_x <= s_mob.pos.x;
    } else {
      std::bernoulli_distribution dist(0.5); // 50% 概率为 true
      left = dist(gen);
    }
    s_mob.hforce = left ? -1400 : 1400;
    s_mob.flip = left ? false : true;
    s_mob.hspeed = 0;
    s_mob.duration = window::dt_now + 1000;
    break;
  }
  default: {
    break;
  }
  }
}

void server_mob_system::run_hit_action(server_mob &s_mob) {
  s_mob.action = u"hit1";
}

void server_mob_system::run_move_action(server_mob &s_mob) {
  s_mob.action = u"move";
}

void server_mob_system::run_stand_action(server_mob &s_mob) {
  s_mob.action = u"stand";
}

void server_mob_system::run_die_action(server_mob &s_mob) {
  s_mob.action = u"die1";
}

void server_mob_system::run_network_drop_sync(server_mob &s_mob) {
  ServerMobDropT smd;
  auto mob_drops = load_mob_drops(s_mob);
  for (const auto &drop : mob_drops) {
    auto &gen = random_game_instance::gen;
    std::uniform_real_distribution<float> dis(0.0f, 1.0f);
    bool success = dis(gen) <= drop.rate;
    if (success) {
      auto min_quantity = drop.min_quantity;
      auto max_quantity = drop.max_quantity;
      std::uniform_int_distribution<int> dis(min_quantity, max_quantity);
      int random_num = dis(gen);
      DropT dt;
      dt.x1 = s_mob.pos.x;
      dt.y1 = s_mob.pos.y;
      dt.page = s_mob.page;
      std::uniform_int_distribution<uint64_t> dist;
      dt.random_id = dist(gen);
      if (item_game_instance::check_item(drop.id)) {
        ItemT it;
        it.item_id = std::stoi(std::string{drop.id.begin(), drop.id.end()});
        it.item_num = random_num;
        dt.drop.Set(it);
      } else {
        EquipT et;
        et.equip_id = std::stoi(std::string{drop.id.begin(), drop.id.end()});
        dt.drop.Set(et);
      }
      smd.payload.push_back(std::make_unique<DropT>(dt));
    }
  }
  auto drop_size = smd.payload.size();
  if (drop_size == 0) {
    return;
  }
  auto border = map_info_game_instance::load_mr_border(map_id);
  const auto w = 32;
  int mid = drop_size / 2;
  float offset = (drop_size % 2 == 0) ? 0.5f : 0.0f;
  const auto &s_fhs = server_scene_instance::scenes.at(map_id).fhs;
  std::flat_map<int32_t, game_foothold> g_fhs;
  for (const auto &[key, value] : s_fhs) {
    g_fhs.emplace(key, value.fh);
  }
  for (size_t n = 0; n < smd.payload.size(); n++) {
    float centerX = (n - mid + offset) * (w);
    float x = centerX - w / 2.0f;
    auto &d = smd.payload[n];
    d->x2 = d->x1 + x;
    d->y2 = d->y1 - 100;

    int32_t tmp_fh;
    uint8_t tmp_page;
    float tmp_hsp = 0;
    float tmp_vsp = 10000;
    SDL_FPoint tmp_fp{d->x2, d->y2};
    physic::fall(tmp_fp, 100000, tmp_hsp, tmp_vsp, tmp_vsp, tmp_vsp, border,
                 true, true, tmp_fh, tmp_page, g_fhs);
    d->x2 = tmp_fp.x;
    d->y2 = tmp_fp.y;
  }
  auto &clients = server_scene_instance::scenes[map_id].clients;
  for (auto client_id : clients) {
    server_response::send_to_client(client_id, smd);
  }
}

void server_mob_system::run_die(server_mob &s_mob) {
  s_mob.hate_id = 0;
  run_die_action(s_mob);
  run_network_drop_sync(s_mob);
}

bool server_mob_system::run_beat(server_mob &s_mob) {
  if (s_mob.beats.empty()) {
    return false;
  }
  const auto &begin = s_mob.beats.begin();
  auto &beat = begin->second;
  auto time = beat.beat_start_time;
  auto current_time = std::chrono::duration_cast<std::chrono::milliseconds>(
                          std::chrono::system_clock::now().time_since_epoch())
                          .count();
  if (time <= current_time) {
    if (beat.beat_time >= 0) {
      beat.beat_time -= delta_time;
      s_mob.hforce = beat.left ? 1400 : -1400;
      s_mob.flip = beat.left ? false : true;
      if (s_mob.hp > 0) {
        s_mob.hate_id = beat.beat_id;
        run_hit_action(s_mob);
      } else {
        run_die(s_mob);
      }
      return true;
    } else {
      s_mob.beats.clear();
      return false;
    }
  }
  return false;
}

void server_mob_system::run_hit(server_mob &s_mob) {
  auto r = run_beat(s_mob);
  switch (s_mob.type) {
  case server_mob::mob_type::stand: {
    run_walk(s_mob);
    if (!r) {
      s_mob.duration = window::dt_now + 500;
      run_stand_action(s_mob);
    }
    break;
  }
  case server_mob::mob_type::swim:
  case server_mob::mob_type::fly: {
    break;
  }
  }
  return;
}

void server_mob_system::run_state_machine(server_mob &s_mob) {
  auto o_mob = s_mob;
  auto m_action = mob_logic_system::load_action_type(s_mob.action);
  switch (m_action) {
  case mob_logic_system::action_enum::revive: {
    break;
  }
  case mob_logic_system::action_enum::die: {
    break;
  }
  case mob_logic_system::action_enum::stand: {
    run_beat(s_mob);
    break;
  }
  case mob_logic_system::action_enum::move: {
    run_beat(s_mob);
    run_walk(s_mob);
    break;
  }
  case mob_logic_system::action_enum::hit: {
    run_hit(s_mob);
    break;
  }
  case mob_logic_system::action_enum::jump: {
    break;
  }
  case mob_logic_system::action_enum::swim:
  case mob_logic_system::action_enum::fly: {
    break;
  }
  }
  run_duration(s_mob);
  run_network_sync(s_mob, o_mob);
}

void server_mob_system::run_send() {
  if (unique_logics.empty()) {
    return;
  }
  ServerMobLogicT r;
  r.map_id = map_id;
  r.payload = std::move(unique_logics);
  auto &clients = server_scene_instance::scenes[map_id].clients;
  for (auto client_id : clients) {
    server_response::send_to_client(client_id, r);
  }
  unique_logics.clear();
}

bool server_mob_system::run() {
  // 怪物逻辑为30帧（约33ms/帧）
  const uint32_t MIN_FRAME_INTERVAL_MS = 33;
  static uint64_t last = SDL_GetTicks();

  delta_time = window::dt_now - last;
  if (delta_time < MIN_FRAME_INTERVAL_MS) {
    return true; // 跳过这一帧
  }
  last = window::dt_now;
  delta_time = std::min(delta_time, MIN_FRAME_INTERVAL_MS);

  auto &scenes = server_scene_instance::scenes;
  for (auto &sc : scenes | std::views::values) {
    if (sc.mobs.empty() || sc.clients.empty()) {
      continue;
    }
    map_id = sc.map_id;
    for (auto &mob : sc.mobs | std::views::values) {
      run_state_machine(mob);
    }
    run_send();
  }
  return true;
}