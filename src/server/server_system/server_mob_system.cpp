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
server_mob_system::load_mob_drops(server_mob &mob) {
  using cache_type = std::flat_map<std::u16string, std::vector<mob_drop>>;
  static cache_type cache;

  // 使用 try_emplace 一次完成查找和插入，避免两次哈希查找
  auto [it, inserted] = cache.try_emplace(mob.id);
  if (!inserted) {
    return it->second; // 已缓存，直接返回
  }

  // 查找节点
  auto node = wz_resource::ms->get_root()->find(u"MobDrop/" + mob.id);
  if (node == nullptr) {
    return {}; // 节点不存在，返回空（缓存中保留空条目）
  }

  auto &drops = it->second;
  drops.reserve(node->get_children()->size()); // 预分配内存，避免多次扩容

  // 遍历所有子节点
  for (auto [k, v] : *node->get_children()) {
    // 缓存指针，避免重复索引
    auto *child = v[0];
    if (!child)
      continue; // 安全检查

    // 获取三个属性，使用指针避免重复类型转换
    auto *min_quantity_prop =
        static_cast<wz::Property<int> *>(child->get_child(u"min_quantity"));
    auto *max_quantity_prop =
        static_cast<wz::Property<int> *>(child->get_child(u"max_quantity"));
    auto *chance_prop =
        static_cast<wz::Property<int> *>(child->get_child(u"chance"));

    // 验证所有属性都存在
    if (!min_quantity_prop || !max_quantity_prop || !chance_prop) {
      continue; // 跳过无效条目
    }

    mob_drop md;
    md.id = std::move(k); // 移动字符串，避免拷贝
    md.min_quantity = min_quantity_prop->get();
    md.max_quantity = max_quantity_prop->get();
    md.rate = chance_prop->get() / 1000000.0f;
    drops.push_back(std::move(md));
  }

  return drops;
}

void server_mob_system::run_network_sync(server_mob &mob, server_mob &o_mob) {
  // 1. 提前判断，减少无效调用
  bool pos_changed = (o_mob.pos.x != mob.pos.x || o_mob.pos.y != mob.pos.y);
  bool flip_changed = (o_mob.flip != mob.flip);
  bool action_changed = (o_mob.action != mob.action);
  bool hp_changed = (o_mob.hp != mob.hp);

  // 2. 没有任何变化，直接返回
  if (!pos_changed && !flip_changed && !action_changed && !hp_changed) {
    return;
  }
  if (pos_changed) {
    // 5. 直接构造 Movement，避免中间的 MovementT 临时对象
    MovementT mv;
    mv.x1 = o_mob.pos.x;
    mv.y1 = o_mob.pos.y;
    mv.x2 = mob.pos.x;
    mv.y2 = mob.pos.y;
    mv.page = mob.page;
    mv.time = std::min(window::delta_time, 33);

    MobEventUnionUnion muu;

    ServerMobMvT mvt;
    mvt.mob_index = mob.index;
    mvt.payload = std::make_unique<MovementT>(mv);
    muu.Set(mvt);

    events.payload.push_back(muu);
  }

  if (flip_changed) {
    FlipT ft;
    ft.flip = mob.flip;
    // 7. 翻转数据量小，直接赋值
    MobEventUnionUnion muu;

    ServerMobFlipT smf;
    smf.mob_index = mob.index;
    smf.payload = std::make_unique<FlipT>(ft);
    muu.Set(smf);
    events.payload.push_back(muu);
  }

  if (action_changed) {
    ActionT a;
    a.action = std::string{mob.action.begin(), mob.action.end()};
    a.action_animate = true;
    a.action_index = 0;
    MobEventUnionUnion muu;

    ServerMobActionT sma;
    sma.mob_index = mob.index;
    sma.payload = std::make_unique<ActionT>(a);
    muu.Set(sma);
    events.payload.push_back(muu);
  }

  if (hp_changed) {
    StateT st;
    st.state = StateEnum_HP;
    st.val = mob.hp;
    MobEventUnionUnion muu;

    ServerMobStateT smb;
    smb.mob_index = mob.index;
    smb.payload.push_back(std::make_unique<StateT>(st));
    muu.Set(smb);
    events.payload.push_back(muu);
  }
}

void server_mob_system::run_walk(server_mob &mob) {
  // 移动
  const auto &s_fhs = server_scene_instance::scenes.at(map_id).fhs;
  std::flat_map<int32_t, game_foothold> g_fhs;
  for (const auto &[key, value] : s_fhs) {
    g_fhs.emplace(key, value.fh);
  }

  SDL_FRect border;
  border.x = mob.rx0;
  border.w = mob.rx1;

  auto r =
      physic::walk(mob.pos, delta_time / 1000.0f, mob.hspeed, mob.vspeed,
                   mob.hforce, -80.0f, 80.0f, 0, false, mob.fh, border, g_fhs);
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
      std::bernoulli_distribution dist(0.5); // 50% 概率为 true
      left = dist(gen);
    }
    mob.hforce = left ? -1400 : 1400;
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

void server_mob_system::run_mob_drop(server_mob &mob) {
  // ServerMobDropT smd;
  // auto mob_drops = load_mob_drops(mob);
  // for (const auto &drop : mob_drops) {
  //   auto &gen = random_game_instance::gen;
  //   std::uniform_real_distribution<float> dis(0.0f, 1.0f);
  //   bool success = dis(gen) <= drop.rate;
  //   if (success) {
  //     auto min_quantity = drop.min_quantity;
  //     auto max_quantity = drop.max_quantity;
  //     std::uniform_int_distribution<int> dis(min_quantity, max_quantity);
  //     int random_num = dis(gen);
  //     DropT dt;
  //     dt.x1 = mob.pos.x;
  //     dt.y1 = mob.pos.y;
  //     dt.page = mob.page;
  //     std::uniform_int_distribution<uint64_t> dist;
  //     dt.random_id = dist(gen);
  //     if (item_game_instance::check_item(drop.id)) {
  //       ItemT it;
  //       it.item_id = std::stoi(std::string{drop.id.begin(), drop.id.end()});
  //       it.item_num = random_num;
  //       dt.drop.Set(it);
  //     } else {
  //       EquipT et;
  //       et.equip_id = std::stoi(std::string{drop.id.begin(), drop.id.end()});
  //       dt.drop.Set(et);
  //     }
  //     smd.payload.push_back(std::make_unique<DropT>(dt));
  //   }
  // }
  // auto drop_size = smd.payload.size();
  // if (drop_size == 0) {
  //   return;
  // }
  // auto border = map_info_game_instance::load_mr_border(map_id);
  // const auto w = 32;
  // int mid = drop_size / 2;
  // float offset = (drop_size % 2 == 0) ? 0.5f : 0.0f;
  // const auto &s_fhs = server_scene_instance::scenes.at(map_id).fhs;
  // std::flat_map<int32_t, game_foothold> g_fhs;
  // for (const auto &[key, value] : s_fhs) {
  //   g_fhs.emplace(key, value.fh);
  // }
  // for (size_t n = 0; n < smd.payload.size(); n++) {
  //   float centerX = (n - mid + offset) * (w);
  //   float x = centerX - w / 2.0f;
  //   auto &d = smd.payload[n];
  //   d->x2 = d->x1 + x;
  //   d->y2 = d->y1 - 100;

  //   int32_t tmp_fh;
  //   uint8_t tmp_page;
  //   float tmp_hsp = 0;
  //   float tmp_vsp = 10000;
  //   SDL_FPoint tmp_fp{d->x2, d->y2};
  //   physic::fall(tmp_fp, 100000, tmp_hsp, tmp_vsp, tmp_vsp, tmp_vsp, border,
  //                true, true, tmp_fh, tmp_page, g_fhs);
  //   d->x2 = tmp_fp.x;
  //   d->y2 = tmp_fp.y;
  // }
  // auto &clients = server_scene_instance::scenes[map_id].clients;
  // for (auto client_id : clients) {
  //   server_response::send_to_client(client_id, smd);
  // }
}

void server_mob_system::run_die(server_mob &mob) {
  mob.hate_id = 0;
  run_die_action(mob);
  run_mob_drop(mob);
}

bool server_mob_system::run_beat(server_mob &mob) {
  if (mob.beats.empty()) {
    return false;
  }

  // 累积所有beats的伤害
  for (auto &b : mob.beats | std::views::values) {
    mob.hp -= b.beat_num;
    b.beat_num = 0;
  }

  auto &l_beat = mob.beats.begin()->second;
  auto &r_beat = mob.beats.rbegin()->second;
  auto current_time = std::chrono::duration_cast<std::chrono::milliseconds>(
                          std::chrono::system_clock::now().time_since_epoch())
                          .count();

  if (l_beat.beat_start_time > current_time) {
    return false;
  }

  if (l_beat.beat_time < 0) {
    mob.beats.erase(mob.beats.begin());
    return false;
  }

  // 执行beat动作
  l_beat.beat_time -= delta_time;
  mob.hforce = l_beat.left ? 1400 : -1400;
  mob.flip = !l_beat.left;

  if (mob.hp > 0) {
    mob.hate_id = l_beat.beat_id;
    run_hit_action(mob);
  } else if (r_beat.beat_start_time < current_time) {
    run_die(mob);
    mob.beats.clear();
  }
  return true;
}

void server_mob_system::run_hit(server_mob &mob) {
  auto r = run_beat(mob);
  switch (mob.type) {
  case server_mob::mob_type::stand: {
    run_walk(mob);
    if (!r) {
      mob.duration = window::dt_now + 500;
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
  auto o_mob = mob;
  auto m_action = mob_logic_system::load_action_type(mob.action);
  switch (m_action) {
  case mob_logic_system::action_enum::revive: {
    break;
  }
  case mob_logic_system::action_enum::die: {
    break;
  }
  case mob_logic_system::action_enum::stand: {
    run_beat(mob);
    break;
  }
  case mob_logic_system::action_enum::move: {
    run_beat(mob);
    run_walk(mob);
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
  run_duration(mob);
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