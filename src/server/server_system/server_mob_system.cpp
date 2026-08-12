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

std::vector<server_mob_system::mob_drop>
server_mob_system::load_mob_drops(server_mob &mob) {
  using cache_type = std::flat_map<std::u16string, std::vector<mob_drop>>;
  static cache_type cache;

  // 使用 try_emplace 一次完成查找和插入
  auto [it, inserted] = cache.try_emplace(mob.id);
  if (!inserted) {
    return it->second;
  }

  auto node = wz_resource::ms->get_root()->find(u"MobDrop.img/" + mob.id);
  if (node == nullptr) {
    return {};
  }

  auto &drops = it->second;
  auto *children = node->get_children();
  drops.reserve(children ? children->size() : 0);

  if (!children) {
    return drops;
  }

  for (auto [k, v] : *children) {
    if (v.empty())
      continue;

    auto *child = v[0];
    if (!child)
      continue;

    auto *min_quantity_prop =
        static_cast<wz::Property<int> *>(child->get_child(u"min_quantity"));
    auto *max_quantity_prop =
        static_cast<wz::Property<int> *>(child->get_child(u"max_quantity"));
    auto *chance_prop =
        static_cast<wz::Property<int> *>(child->get_child(u"chance"));

    if (!min_quantity_prop || !max_quantity_prop || !chance_prop) {
      continue;
    }

    mob_drop md;
    md.id = k; // 如果后续不需要原字符串，可改为 std::move(k)
    md.min_quantity = min_quantity_prop->get();
    md.max_quantity = max_quantity_prop->get();
    md.rate = chance_prop->get() / 1000000.0f;
    drops.push_back(std::move(md));
  }

  return drops;
}

void server_mob_system::run_network_sync(server_mob &mob, server_mob &o_mob) {
  // 提前判断，减少无效操作
  bool pos_changed = (o_mob.pos.x != mob.pos.x || o_mob.pos.y != mob.pos.y);
  bool flip_changed = (o_mob.flip != mob.flip);
  bool action_changed = (o_mob.action != mob.action);
  bool hp_changed = (o_mob.hp != mob.hp);

  if (!pos_changed && !flip_changed && !action_changed && !hp_changed) {
    return;
  }
  if (pos_changed) {
    MovementT mv;
    mv.x1 = o_mob.pos.x;
    mv.y1 = o_mob.pos.y;
    mv.x2 = mob.pos.x;
    mv.y2 = mob.pos.y;
    mv.page = mob.page;
    mv.time = std::min(window::delta_time, 33);

    ServerMobMvT mvt;
    mvt.mob_index = mob.index;
    mvt.payload = std::make_unique<MovementT>(std::move(mv));

    MobEventUnionUnion muu;
    muu.Set(std::move(mvt));
    events.payload.push_back(std::move(muu));
  }

  if (flip_changed) {
    FlipT ft;
    ft.flip = mob.flip;

    ServerMobFlipT smf;
    smf.mob_index = mob.index;
    smf.payload = std::make_unique<FlipT>(std::move(ft));

    MobEventUnionUnion muu;
    muu.Set(std::move(smf));
    events.payload.push_back(std::move(muu));
  }

  if (action_changed) {
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

  if (hp_changed) {
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

void server_mob_system::run_walk(server_mob &mob) {
  const auto &s_fhs = server_scene_instance::scenes.at(map_id).fhs;
  std::flat_map<int32_t, game_foothold> g_fhs;
  for (const auto &[key, value] : s_fhs) {
    g_fhs.emplace(key, value.fh);
  }

  SDL_FRect border;
  border.x = mob.rx0;
  border.w = mob.rx1;

  physic::walk(mob.pos, delta_time / 1000.0f, mob.hspeed, mob.vspeed,
               mob.hforce, -80, 80, 0, false, mob.fh, border, g_fhs);
}

void server_mob_system::run_duration(server_mob &mob) {
  if (mob.duration > window::dt_now) {
    return;
  }

  auto action_type = mob_logic_system::load_action_type(mob.action);
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
  switch (mob.type) {
  case server_mob::mob_type::stand: {
    if (mob.hate_id != 0) {
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

  // 如果 actions2 为空，直接返回避免 dist 构造失败
  if (actions2.empty()) {
    return;
  }

  auto &gen = random_game_instance::gen;
  std::uniform_int_distribution<size_t> dist(0, actions2.size() - 1);
  auto it = std::next(actions2.begin(), dist(gen));
  auto selected = *it;
  mob.action = actions.at(selected);

  switch (selected) {
  case mob_logic_system::action_enum::stand: {
    mob.duration = window::dt_now + 500;
    break;
  }
  case mob_logic_system::action_enum::jump: {
    break;
  }
  case mob_logic_system::action_enum::move: {
    bool left = false;
    const auto &clients = server_client_instance::clients;
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
    mob.hspeed = 0;
    mob.duration = window::dt_now + 1000;
    break;
  }
  default: {
    break;
  }
  }
}

void server_mob_system::run_hit_action(server_mob &mob) {
  mob.action = u"hit1";
}

void server_mob_system::run_move_action(server_mob &mob) {
  mob.action = u"move";
}

void server_mob_system::run_stand_action(server_mob &mob) {
  mob.action = u"stand";
}

void server_mob_system::run_die_action(server_mob &mob) {
  mob.action = u"die1";
}

void server_mob_system::run_die(server_mob &mob) {
  mob.hate_id = 0;
  run_die_action(mob);
  ServerMobDieT smd;
  smd.mob_index = mob.index;

  MobEventUnionUnion muu;
  muu.Set(std::move(smd));
  events.payload.push_back(std::move(muu));

  auto mob_drops = load_mob_drops(mob);
  std::vector<DropT> dts;
  for (const auto &drop : mob_drops) {
    auto &gen = random_game_instance::gen;
    std::uniform_real_distribution<float> dis(0.0f, 1.0f);
    bool success = dis(gen) <= drop.rate;
    if (!success) {
      continue;
    }
    DropT dt;
    dt.x1 = mob.pos.x;
    dt.y1 = mob.pos.y;
    dt.page = mob.page;
    if (item_game_instance::check_item(drop.id)) {
      auto min_quantity = drop.min_quantity;
      auto max_quantity = drop.max_quantity;
      std::uniform_int_distribution<int> dis(min_quantity, max_quantity);
      int random_num = dis(gen);
      ItemT it;
      it.item_id = std::stoi(std::string{drop.id.begin(), drop.id.end()});
      it.item_num = random_num;
      dt.drop.Set(it);
    } else {
      EquipT et;
      et.equip_id = std::stoi(std::string{drop.id.begin(), drop.id.end()});
      dt.drop.Set(et);
    }
    dts.push_back(dt);
  }

  ServerMobDropT smt;
  smt.mob_index = mob.index;
  smt.drop = server_drop_instance::create_dts(dts, map_id);
  for (const auto &dt : smt.drop) {
    server_drop_instance::save_drop(map_id, *dt);
  }

  MobEventUnionUnion mu;
  mu.Set(std::move(smt));
  events.payload.push_back(std::move(mu));

  return;
}

bool server_mob_system::run_hitting(server_mob &mob) {
  if (mob.hits.empty()) {
    return false;
  }

  // 累积所有beats的伤害
  int32_t total_damage = 0;
  for (auto &b : mob.hits | std::views::values) {
    total_damage += b.hit_num;
    b.hit_num = 0; // 清零已处理的伤害
  }
  mob.hp -= total_damage;

  auto &l = mob.hits.begin()->second;
  auto &r = mob.hits.rbegin()->second;
  auto current_time = window::dt_time;

  if (l.hit_time > current_time) {
    return false;
  }

  if (l.hit_duration <= 0) {
    mob.hits.erase(mob.hits.begin());
    return false;
  }

  // 执行beat动作
  l.hit_duration -= delta_time;
  mob.hforce = l.left ? MOVE_FORCE : -MOVE_FORCE;
  mob.flip = !l.left;

  if (mob.hp > 0) {
    mob.hate_id = l.hit_id;
    mob.duration = window::dt_now + 220;
    run_hit_action(mob);
  } else if (r.hit_time < current_time) {
    run_die(mob);
    mob.hits.clear();
  }
  return true;
}

void server_mob_system::run_hit(server_mob &mob) {
  auto r = run_hitting(mob);
  switch (mob.type) {
  case server_mob::mob_type::stand: {
    run_walk(mob);
    if (window::dt_now > mob.duration) {
      run_stand_action(mob);
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

void server_mob_system::run_state_machine(server_mob &mob) {
  auto o_mob = mob; // 拷贝用于对比
  auto m_action = mob_logic_system::load_action_type(mob.action);

  switch (m_action) {
  case mob_logic_system::action_enum::revive: {
    break;
  }
  case mob_logic_system::action_enum::die: {
    break;
  }
  case mob_logic_system::action_enum::stand: {
    run_hitting(mob);
    run_duration(mob);
    break;
  }
  case mob_logic_system::action_enum::move: {
    run_hitting(mob);
    run_walk(mob);
    run_duration(mob);
    break;
  }
  case mob_logic_system::action_enum::hit: {
    run_hit(mob);
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
  run_network_sync(mob, o_mob);
}

void server_mob_system::run_send() {
  if (events.payload.empty()) {
    return;
  }
  events.map_id = map_id;
  auto &clients = server_scene_instance::scenes[map_id].clients;
  for (auto client_id : clients) {
    server_response::send_to_client(client_id, events);
  }
  events.payload.clear();
}

bool server_mob_system::run() {
  static uint64_t last = SDL_GetTicks();

  delta_time = window::dt_now - last;
  if (delta_time < MIN_FRAME_INTERVAL_MS) {
    return true;
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