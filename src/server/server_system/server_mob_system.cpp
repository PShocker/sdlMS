#include "server_mob_system.h"
#include "src/client/game_instance/random_game_instance.h"
#include "src/client/window/window.h"
#include "src/common/flatbuffers/server.h"
#include "src/common/physic/physic.h"
#include "src/common/response/server_response.h"
#include "src/server/server_instance/server_mob_instance.h"
#include "src/server/server_instance/server_scene_instance.h"
#include <flat_map>
#include <flat_set>
#include <ranges>
#include <string>

server_mob_system::action_enum
server_mob_system::load_action_type(server_mob &s_mob) {
  const static std::flat_map<std::u16string, action_enum> map_name = {
      {u"stand", action_enum::stand},
      {u"move", action_enum::move},
  };
  return map_name.at(s_mob.action);
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
  auto delta_time = window::delta_time / 1000.0f;
  auto s_fhs = server_scene_instance::scenes.at(map_id).fhs;
  std::flat_map<int32_t, game_foothold> g_fhs;
  for (auto [key, value] : s_fhs) {
    g_fhs.emplace(key, std::move(value.fh)); // 移动避免拷贝s
  }
  auto r = physic::walk(s_mob.pos, delta_time, s_mob.hspeed, s_mob.vspeed,
                        s_mob.hforce, -100.0f, 100.0f, 0, false, s_mob.fh,
                        {0, 0, 0, 0}, g_fhs);
}

void server_mob_system::run_duration(server_mob &s_mob) {
  if (s_mob.duration > window::dt_now) {
    return;
  }
  static const std::flat_map<action_enum, std::u16string> actions = {
      {action_enum::stand, u"stand"},
      {action_enum::move, u"move"},
      {action_enum::fly, u"fly"},
  };
  std::flat_set<action_enum> actions2;
  switch (s_mob.type) {
  case server_mob::mob_type::stand: {
    actions2 = {action_enum::stand, action_enum::move};
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
  action_enum selected = *it;
  s_mob.action = actions.at(selected);
  switch (selected) {
  case action_enum::stand: {
    s_mob.duration = window::dt_now + 500;
    break;
  }
  case action_enum::jump: {
    break;
  }
  case action_enum::move: {
    std::bernoulli_distribution dist(0.5); // 50% 概率为 true
    bool random_bool = dist(gen);
    s_mob.hforce = random_bool ? 1400 : -1400;
    s_mob.duration = window::dt_now + 1000;
    break;
  }
  default: {
    break;
  }
  }
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
  case action_enum::move: {
    run_walk(s_mob);
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
  run_duration(s_mob);
  run_network_sync(s_mob, o_mob);
}

bool server_mob_system::run() {
  // 怪物逻辑为30帧
  const int32_t MIN_FRAME_INTERVAL_MS = 33;
  static uint64_t last_frame_time = 0;
  // 节流：未达到帧间隔时直接返回
  if (window::dt_now - last_frame_time < MIN_FRAME_INTERVAL_MS) {
    return true; // 跳过这一帧
  }
  last_frame_time = window::dt_now;

  auto &scenes = server_scene_instance::scenes;
  for (auto &sc : scenes | std::views::values) {
    if (sc.mobs.empty()) {
      continue;
    }
    map_id = sc.map_id;
    unique_logics.clear();
    for (auto &mob : sc.mobs | std::views::values) {
      run_state_machine(mob);
    }
    if (unique_logics.empty()) {
      continue;
    }
    ServerMobLogicT r;
    r.payload = std::move(unique_logics);
    auto &clients = server_scene_instance::scenes[map_id].clients;
    for (auto client_id : clients) {
      server_response::mob_logic_response(client_id, r);
    }
  }
  return true;
}
