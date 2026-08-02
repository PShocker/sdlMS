#include "server_drop_instance.h"
#include "server_scene_instance.h"
#include "src/client/game/game_drop.h"
#include "src/client/game_instance/cursor_game_instance.h"
#include "src/client/game_instance/drop_game_instance.h"
#include "src/client/game_instance/item_game_instance.h"
#include "src/client/game_instance/map_info_game_instance.h"
#include "src/client/game_instance/package_game_instance.h"
#include "src/client/game_instance/random_game_instance.h"
#include "src/client/window/window.h"
#include "src/common/flatbuffers/server.h"
#include "src/common/physic/physic.h"
#include "src/common/response/server_response.h"
#include "src/server/server/server_drop.h"
#include <cmath>
#include <format>
#include <utility>

float server_drop_instance::cal_drop_time(float y1, float y2) {
  const float VY = 400.0f; // 初速度 = 最大速度
  const float G = 800.0f;  // 重力加速度

  float delta_y = y1 - y2; // 起点-终点（正数=下落）
  // 终点在起点下方（下落）
  if (delta_y > 0) {
    float fall_distance = delta_y;
    if (fall_distance <= 100.0f) {
      // 距离 <= 100px：全程加速，未达到最大速度
      // t = sqrt(2*fall_distance/G)
      float t = sqrtf(2.0f * fall_distance / G);
      return 0.5f + t; // 上升0.5秒 + 下落时间
    } else {
      // 距离 > 100px：加速100px后匀速
      float t_accel = 0.5f;                          // 加速时间
      float t_const = (fall_distance - 100.0f) / VY; // 匀速时间
      float t = 0.5f + t_accel + t_const; // 上升0.5s + 加速0.5s + 匀速
      return t;
    }
  }
  // 终点在起点上方（上升）
  else if (delta_y < 0) {
    float rise_distance = -delta_y;
    // 上升阶段：t = (VY - sqrt(VY² - 2*G*rise_distance)) / G
    float t = (VY - sqrtf(VY * VY - 2.0f * G * rise_distance)) / G;
    return t;
  }
  // 相同高度
  else {
    return 1.0f; // 完整抛物线：0.5s上升 + 0.5s下落
  }
}

void server_drop_instance::save_drop(uint64_t map_id, const DropT &drop) {
  auto &scene = server_scene_instance::scenes[map_id];
  server_drop sd;
  sd.dt = drop;
  sd.destroy‌ = window::dt_now + 60 * 5 * 1000;
  sd.available = window::dt_now + cal_drop_time(drop.y1, drop.y2) * 1000;
  scene.drops.emplace(drop.random_id, sd);
}

void server_drop_instance::handle_client_drop(uint64_t client_id,
                                              ClientCharacterDropT &r) {
  auto map_id = r.map_id;

  auto dts = create_dts({*r.payload}, map_id);
  for (const auto &dt : dts) {
    save_drop(map_id, *dt);
  }
  auto clients = server_scene_instance::scenes[map_id].clients;
  clients.erase(client_id);
  ServerCharacterDropT t;
  t.client_id = client_id;
  t.payload = std::move(dts[0]);
  for (const auto c : clients) {
    server_response::send_to_client(c, t);
  }
  t.client_id = 0;
  server_response::send_to_client(client_id, t);
}

void server_drop_instance::handle_pick(uint64_t client_id,
                                       ClientCharacterPickT &r) {
  auto map_id = r.map_id;
  auto &scene = server_scene_instance::scenes[map_id];
  if (scene.drops.contains(r.random_id)) {
    ServerCharacterPickT t;
    t.random_id = r.random_id;
    t.client_id = client_id;
    for (const auto c : scene.clients) {
      server_response::send_to_client(c, t);
    }
    scene.drops.erase(r.random_id);
  }
}

void server_drop_instance::handle_server_dt(const DropT &dt) {
  game_drop drop;
  drop.page = dt.page;
  drop.vspeed = -400;
  drop.pos = SDL_FPoint{dt.x1, dt.y1};
  drop.goal = SDL_FPoint{dt.x2, dt.y2};
  drop.hspeed = (dt.x2 - dt.x1) / (-drop.vspeed / 800);

  switch (dt.drop.type) {
  case fbs::ItemUnion_Equip: {
    auto equipT = dt.drop.AsEquip();
    game_equip_item equip;
    auto tmp = std::format("{:08d}", equipT->equip_id);
    equip.id = {tmp.begin(), tmp.end()};
    drop.data =
        std::polymorphic<game_item>(std::in_place_type<game_equip_item>, equip);
    break;
  }
  case fbs::ItemUnion_Item: {
    auto item = dt.drop.AsItem();
    auto tmp = std::format("{:08d}", item->item_id);
    drop.data =
        item_game_instance::load_item({tmp.begin(), tmp.end()}, item->item_num);
    break;
  }
  default: {
    break;
  }
  }
  drop_game_instance::data.emplace(dt.random_id, drop);
}

void server_drop_instance::handle_server_scene_dt(const DropT &dt) {
  handle_server_dt(dt);
  auto &drop = drop_game_instance::data.at(dt.random_id);
  drop.hspeed = 0;
  drop.pos.x = dt.x2;
  drop.pos.y = dt.y2;
  drop.type = game_drop::land;
  return;
}

void server_drop_instance::handle_server_drop(uint64_t client_id,
                                              ServerCharacterDropT &r) {
  if (cursor_game_instance::cursor_hand_net.has_value()) {
    if (r.client_id == 0) {
      cursor_game_instance::cursor_hand_net = std::nullopt;
      auto &hand = cursor_game_instance::cursor_hand.value();
      switch (hand.type) {
      case cursor_game_instance::equipment: {
        break;
      }
      case cursor_game_instance::package: {
        package_game_instance::data[hand.val][hand.sub_val] =
            std::polymorphic<game_item>{};
        break;
      }
      default: {
        break;
      }
      }
      cursor_game_instance::cursor_hand = std::nullopt;
    }
  }
  handle_server_dt(*r.payload);
}

std::vector<std::unique_ptr<fbs::DropT>>
server_drop_instance::create_dts(const std::vector<DropT> &dts,
                                 uint32_t map_id) {
  std::vector<std::unique_ptr<fbs::DropT>> r;

  auto &gen = random_game_instance::gen;
  std::uniform_int_distribution<uint64_t> dist;

  const auto &s_fhs = server_scene_instance::scenes.at(map_id).fhs;
  std::flat_map<int32_t, game_foothold> g_fhs;
  for (const auto &[key, value] : s_fhs) {
    g_fhs.emplace(key, value.fh);
  }
  auto border = map_info_game_instance::load_mr_border(map_id);
  auto dts_size = dts.size();
  const auto dt_w = 24;

  int n = 0;
  auto max_h = std::pow(400, 2) / (2 * 800);

  for (auto dt : dts) {
    dt.random_id = dist(gen);

    int x;
    if (dts_size == 1) {
      x = 0;
    } else {
      float center_offset = (dts_size - 1) / 2.0f;
      x = (int)((n - center_offset) * dt_w);
    }

    dt.x2 = dt.x1 + x;
    dt.y2 = dt.y1 - max_h;

    int32_t tmp_fh;
    uint8_t tmp_page;
    float tmp_hsp = 0;
    float tmp_vsp = 10000;
    SDL_FPoint tmp_fp{dt.x2, dt.y2};
    physic::fall(tmp_fp, 100000, tmp_hsp, tmp_vsp, tmp_vsp, tmp_vsp, border,
                 true, true, tmp_fh, tmp_page, g_fhs);
    if (tmp_fp.y == border.h) {
      // 如果这个掉落物会掉落在地图外，则修正x为原始x
      tmp_fp = {dt.x1, dt.y2};
      physic::fall(tmp_fp, 100000, tmp_hsp, tmp_vsp, tmp_vsp, tmp_vsp, border,
                   true, true, tmp_fh, tmp_page, g_fhs);
    }
    dt.page = tmp_page;

    dt.x2 = tmp_fp.x;
    dt.y2 = tmp_fp.y;

    r.push_back(std::make_unique<fbs::DropT>(dt));
    n++;
  }
  return r;
}