#include "server_mob_system.h"
#include "SDL3/SDL_rect.h"
#include "src/client/game/game_debuff.h"
#include "src/client/game_instance/item_game_instance.h"
#include "src/client/game_instance/map_info_game_instance.h"
#include "src/client/game_instance/mob_game_instance.h"
#include "src/client/game_instance/random_game_instance.h"
#include "src/client/system/logic/mob_logic_system.h"
#include "src/client/window/window.h"
#include "src/common/flatbuffers/common.h"
#include "src/common/flatbuffers/server.h"
#include "src/common/physic/physic.h"
#include "src/common/response/server_response.h"
#include "src/common/wz/wz_resource.h"
#include "src/server/server_instance/server_client_instance.h"
#include "src/server/server_instance/server_drop_instance.h"
#include "src/server/server_instance/server_mob_instance.h"
#include "src/server/server_instance/server_scene_instance.h"
#include "wz/Property.h"
#include <algorithm>
#include <cstdint>
#include <flat_map>
#include <flat_set>
#include <memory>
#include <ranges>
#include <string>
#include <utility>
#include <vector>

// 常量定义
namespace {
constexpr float MOVE_FORCE = 1400.0f;
constexpr uint32_t MIN_FRAME_INTERVAL_MS = 33;
} // namespace

bool ServerMobSystem::run_try_jump(ServerMob &mob) {
  auto mob_node = MobGameInstance::load_link_mob_node(mob.id);
  if (!mob_node->children.contains(u"jump")) {
    return false;
  }
  const auto &s_fhs = ServerSceneInstance::scenes.at(map_id).fhs;
  const auto &s_fh = s_fhs.at(mob.fh).fh;
  int32_t n_fh = -1;
  if (mob.flip) {
    n_fh = s_fh.prev;
  } else {
    n_fh = s_fh.next;
  }
  auto &gen = RandomGameInstance::gen;
  bool jump = false;
  std::bernoulli_distribution dist(0.005);
  jump = dist(gen);
  if (s_fhs.contains(n_fh)) {
    if (!s_fhs.at(n_fh).fh.k.has_value()) {
      // wall
      std::bernoulli_distribution dist(0.4);
      jump = dist(gen);
    }
  }
  if (jump) {
    mob.action = u"jump";
    mob.vspeed = -555;
    mob.fh = 0;
  }
  return jump;
}

int ServerMobSystem::load_mob_hit_cd(ServerMob &mob) {
  int cd = 0;
  auto mob_node = MobGameInstance::load_link_mob_node(mob.id);
  mob_node = mob_node->get_child(u"hit1");
  for (auto [k, v] : *mob_node->get_children()) {
    if (v[0]->get_child(u"delay")) {
      auto delay =
          static_cast<wz::Property<int> *>(v[0]->get_child(u"delay"))->get();
      cd += delay;
    }
  }
  return cd;
}

void ServerMobSystem::run_network_sync_pos(ServerMob &mob,
                                             ServerMob &o_mob) {
  if (o_mob.pos.x != mob.pos.x || o_mob.pos.y != mob.pos.y) {
    MovementT mv;
    mv.x1 = o_mob.pos.x;
    mv.y1 = o_mob.pos.y;
    mv.x2 = mob.pos.x;
    mv.y2 = mob.pos.y;
    mv.page = mob.page;
    mv.time = std::min((uint32_t)Window::delta_time, MIN_FRAME_INTERVAL_MS);

    ServerMobMvT mvt;
    mvt.mob_index = mob.index;
    mvt.payload = std::make_unique<MovementT>(std::move(mv));

    MobEventUnionUnion muu;
    muu.Set(std::move(mvt));
    events.payload.push_back(std::move(muu));
  }
}

void ServerMobSystem::run_network_sync_flip(ServerMob &mob,
                                              ServerMob &o_mob) {
  if (o_mob.flip != mob.flip) {
    FlipT ft;
    ft.flip = mob.flip;

    ServerMobFlipT smf;
    smf.mob_index = mob.index;
    smf.payload = std::make_unique<FlipT>(std::move(ft));

    MobEventUnionUnion muu;
    muu.Set(std::move(smf));
    events.payload.push_back(std::move(muu));
  }
}

void ServerMobSystem::run_network_sync_action(ServerMob &mob,
                                                ServerMob &o_mob) {
  if (o_mob.action != mob.action) {
    ActionT a;
    a.action = std::string{mob.action.begin(), mob.action.end()};
    a.action_animate = true;
    a.action_index = 0;

    ServerMobActionT sma;
    sma.mob_index = mob.index;
    sma.payload = std::make_unique<ActionT>(std::move(a));

    MobEventUnionUnion muu;
    muu.Set(std::move(sma));
    events.payload.push_back(std::move(muu));
  }
}

void ServerMobSystem::run_network_sync_hp(ServerMob &mob,
                                            ServerMob &o_mob) {
  if (o_mob.hp != mob.hp) {
    StateT st;
    st.state = StateEnum_HP;
    st.val = mob.hp;

    ServerMobStateT smb;
    smb.mob_index = mob.index;
    smb.payload.push_back(std::make_unique<StateT>(std::move(st)));

    MobEventUnionUnion muu;
    muu.Set(std::move(smb));
    events.payload.push_back(std::move(muu));
  }
}

void ServerMobSystem::run_network_sync(ServerMob &mob, ServerMob &o_mob) {
  run_network_sync_pos(mob, o_mob);
  run_network_sync_flip(mob, o_mob);
  run_network_sync_action(mob, o_mob);
  run_network_sync_hp(mob, o_mob);
}

void ServerMobSystem::run_walk(ServerMob &mob) {
  SDL_FRect border;
  border.x = mob.rx0;
  border.w = mob.rx1;

  Physic::walk(mob.pos, delta_time / 1000.0f, mob.hspeed, mob.vspeed,
               mob.hforce, -mob.hspeed_max, mob.hspeed_max, 0, false, mob.fh,
               border, g_fhs);
}

void ServerMobSystem::run_duration(ServerMob &mob) {
  if (mob.duration > Window::dt_now) {
    return;
  }

  auto action_type = MobLogicSystem::load_action_type(mob.action);
  if (action_type == MobLogicSystem::ActionEnum::jump ||
      action_type == MobLogicSystem::ActionEnum::hit ||
      action_type == MobLogicSystem::ActionEnum::die) {
    return;
  }

  static const std::flat_map<MobLogicSystem::ActionEnum, std::u16string>
      actions = {
          {MobLogicSystem::ActionEnum::stand, u"stand"},
          {MobLogicSystem::ActionEnum::move, u"move"},
          {MobLogicSystem::ActionEnum::fly, u"fly"},
      };

  std::flat_set<MobLogicSystem::ActionEnum> actions2;
  switch (mob.type) {
  case ServerMob::MobType::stand: {
    if (mob.hate_id != 0) {
      actions2 = {
          MobLogicSystem::ActionEnum::move,
      };
    } else {
      actions2 = {
          MobLogicSystem::ActionEnum::stand,
          MobLogicSystem::ActionEnum::move,
      };
    }
    break;
  }
  case ServerMob::MobType::swim:
  case ServerMob::MobType::fly: {
    break;
  }
  }

  // 如果 actions2 为空，直接返回避免 dist 构造失败
  if (actions2.empty()) {
    return;
  }

  auto &gen = RandomGameInstance::gen;
  std::uniform_int_distribution<size_t> dist(0, actions2.size() - 1);
  auto it = std::next(actions2.begin(), dist(gen));
  auto selected = *it;
  mob.action = actions.at(selected);

  switch (selected) {
  case MobLogicSystem::ActionEnum::stand: {
    mob.duration = Window::dt_now + 500;
    mob.hspeed = 0;
    break;
  }
  case MobLogicSystem::ActionEnum::jump: {
    break;
  }
  case MobLogicSystem::ActionEnum::move: {
    bool left = false;
    const auto &clients = ServerClientInstance::clients;
    if (mob.hate_id != 0 && clients.contains(mob.hate_id)) {
      auto &g_character = clients.at(mob.hate_id).player_t.character;
      auto &g_x = g_character->state->x;
      left = g_x <= mob.pos.x;
    } else {
      std::bernoulli_distribution dist(0.5);
      left = dist(gen);
    }
    mob.hforce = left ? -MOVE_FORCE : MOVE_FORCE;
    mob.flip = left ? false : true;
    mob.hspeed = left == true ? -mob.hspeed_max : mob.hspeed_max;
    mob.duration = Window::dt_now + 1000;
    break;
  }
  default: {
    break;
  }
  }
}

void ServerMobSystem::run_default_action(ServerMob &mob) {
  switch (mob.type) {
  case ServerMob::MobType::stand: {
    if (mob.fh != 0) {
      run_stand_action(mob);
    }
    break;
  }
  case ServerMob::MobType::swim: {
    run_swim_action(mob);
    break;
  }
  case ServerMob::MobType::fly: {
    run_fly_action(mob);
    break;
  }
  }
}

void ServerMobSystem::run_fly_action(ServerMob &mob) {}

void ServerMobSystem::run_swim_action(ServerMob &mob) {}

void ServerMobSystem::run_hit_action(ServerMob &mob) {
  mob.action = u"hit1";
}

void ServerMobSystem::run_move_action(ServerMob &mob) {
  mob.action = u"move";
}

void ServerMobSystem::run_stand_action(ServerMob &mob) {
  mob.action = u"stand";
}

void ServerMobSystem::run_die_action(ServerMob &mob) {
  mob.action = u"die1";
}

void ServerMobSystem::run_die(ServerMob &mob, uint64_t client_id) {
  mob.hate_id = 0;
  run_die_action(mob);
  ServerMobDieT smd;
  smd.client_id = client_id;
  smd.mob_index = mob.index;

  MobEventUnionUnion muu;
  muu.Set(std::move(smd));
  events.payload.push_back(std::move(muu));

  auto mob_drops = ServerMobInstance::load_mob_drops(mob.id);
  std::vector<DropT> dts = ServerMobInstance::create_mob_dts(
      mob.id, mob.pos.x, mob.pos.y, mob.page);

  ServerMobDropT smt;
  smt.mob_index = mob.index;
  smt.drop = ServerDropInstance::create_dts(dts, map_id);
  for (const auto &dt : smt.drop) {
    ServerDropInstance::save_drop(map_id, *dt);
  }

  MobEventUnionUnion mu;
  mu.Set(std::move(smt));
  events.payload.push_back(std::move(mu));

  mob.debuffs.clear();
  return;
}

bool ServerMobSystem::run_hitting(ServerMob &mob) {
  if (mob.hits.empty()) {
    return false;
  }
  const auto current_time = Window::dt_time;
  // 1) 结算本帧累积伤害（用 int64_t 防溢出）
  int64_t total_damage = 0;
  for (auto &b : mob.hits | std::views::values) {
    if (b.hit_num != 0) {
      total_damage += b.hit_num;
      b.hit_num = 0;
    }
  }
  mob.hp -= total_damage;
  // 2) 取首/尾（hits 需为有序容器，保证 rbegin 是"最后一次命中"）
  const auto &first = mob.hits.begin()->second;
  const auto &last = mob.hits.rbegin()->second;
  // 首帧未到，暂不推进动作（伤害已结算，保持原语义）
  if (first.hit_time > current_time) {
    return false;
  }
  // 4) 死亡判定：HP 归零且最后一击已过期
  if (mob.hp <= 0 && last.hit_time < current_time) {
    const auto killer_id = last.hit_id; // clear 前保存
    mob.hits.clear();
    run_die(mob, killer_id);
    return true;
  }
  mob.hate_id = last.hit_id;
  // 5) 推进动作：非击退帧执行 hit action；击退帧弹出队首
  if (first.hit_push) {
    // 3) 更新朝向/击退方向
    mob.hforce = first.left ? MOVE_FORCE : -MOVE_FORCE;
    mob.flip = !first.left;
    run_hit_action(mob);
  } else {
    mob.hits.erase(mob.hits.begin());
  }
  return true;
}

bool ServerMobSystem::run_hit_check(ServerMob &mob) {
  if (mob.hits.empty()) {
    return false;
  }

  auto it = mob.hits.begin();
  const auto time = it->first;

  // 只读取需要的字段，避免拷贝整个 hit 对象
  if (!it->second.hit_push) {
    mob.hits.erase(it);
    return false;
  }

  const auto delay = load_mob_hit_cd(mob);
  if (time + delay > Window::dt_time) {
    return false;
  }

  mob.hits.erase(it);
  return true;
}

void ServerMobSystem::run_hit(ServerMob &mob) {
  if (run_hit_check(mob)) {
    run_default_action(mob);
    run_duration(mob);
  }
  return;
}

bool ServerMobSystem::run_fall(ServerMob &mob) {
  if (mob.fh != 0) {
    return false;
  }

  auto border = MapInfoGameInstance::load_mr_border(map_id);
  border.x = mob.rx0;
  border.w = mob.rx1;
  auto vspeed = mob.vspeed + delta_time * 2;
  mob.vspeed = vspeed;

  const float vspeed_min = -5550.0f;
  const float vspeed_max = 670.0f;

  bool r = Physic::fall(mob.pos, delta_time / 1000.0f, mob.hspeed, mob.vspeed,
                        vspeed_min, vspeed_max, border, true, true, mob.fh,
                        mob.page, g_fhs, INT32_MAX);
  return r;
}

bool ServerMobSystem::run_debuff_action(ServerMob &mob) {
  if (mob.debuffs.contains(GameDebuff::dizz)) {
    run_default_action(mob);
    return true;
  }
  return false;
}

void ServerMobSystem::run_state_machine(ServerMob &mob) {
  auto o_mob = mob; // 拷贝用于对比
  auto m_action = MobLogicSystem::load_action_type(mob.action);
  switch (m_action) {
  case MobLogicSystem::ActionEnum::revive: {
    break;
  }
  case MobLogicSystem::ActionEnum::die: {
    break;
  }
  case MobLogicSystem::ActionEnum::stand: {
    if (!run_hitting(mob)) {
      run_duration(mob);
    }
    break;
  }
  case MobLogicSystem::ActionEnum::move: {
    if (run_debuff_action(mob)) {
      break;
    }
    if (!run_hitting(mob)) {
      run_walk(mob);
      if (!run_try_jump(mob)) {
        run_duration(mob);
      }
    }
    break;
  }
  case MobLogicSystem::ActionEnum::hit: {
    switch (mob.type) {
    case ServerMob::MobType::stand: {
      if (mob.fh == 0) {
        if (!run_fall(mob)) {
          run_stand_action(mob);
        }
      } else {
        run_walk(mob);
      }
      break;
    }
    case ServerMob::MobType::swim:
    case ServerMob::MobType::fly: {
      break;
    }
    }
    run_hit(mob);
    run_hitting(mob);
    break;
  }
  case MobLogicSystem::ActionEnum::jump: {
    bool hit = run_hitting(mob);
    if (!run_fall(mob)) {
      if (!hit) {
        run_default_action(mob);
      }
    }
    break;
  }
  case MobLogicSystem::ActionEnum::swim:
  case MobLogicSystem::ActionEnum::fly: {
    break;
  }
  }
  run_network_sync(mob, o_mob);
}

void ServerMobSystem::run_send() {
  if (events.payload.empty()) {
    return;
  }
  events.map_id = map_id;
  auto &clients = ServerSceneInstance::scenes[map_id].clients;
  for (auto client_id : clients) {
    ServerResponse::send_to_client(client_id, events);
  }
  events.payload.clear();
}

bool ServerMobSystem::run() {
  static uint64_t last = SDL_GetTicks();

  delta_time = Window::dt_now - last;
  if (delta_time < MIN_FRAME_INTERVAL_MS) {
    return true;
  }
  last = Window::dt_now;
  delta_time = std::min(delta_time, MIN_FRAME_INTERVAL_MS);

  auto &scenes = ServerSceneInstance::scenes;
  for (auto &sc : scenes | std::views::values) {
    if (sc.mobs.empty() || sc.clients.empty()) {
      continue;
    }
    map_id = sc.map_id;
    g_fhs.clear();
    const auto &s_fhs = ServerSceneInstance::scenes.at(map_id).fhs;
    for (const auto &[key, value] : s_fhs) {
      g_fhs.emplace(key, value.fh);
    }
    for (auto &mob : sc.mobs | std::views::values) {
      run_state_machine(mob);
    }
    run_send();
  }
  return true;
}