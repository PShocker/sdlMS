#include "server_drop_instance.h"
#include "server_scene_instance.h"
#include "src/client/game/game_drop.h"
#include "src/client/game/game_gain_log.h"
#include "src/client/game_instance/character_game_instance.h"
#include "src/client/game_instance/cursor_game_instance.h"
#include "src/client/game_instance/drop_game_instance.h"
#include "src/client/game_instance/gain_log_game_instance.h"
#include "src/client/game_instance/item_game_instance.h"
#include "src/client/game_instance/map_info_game_instance.h"
#include "src/client/game_instance/package_game_instance.h"
#include "src/client/game_instance/random_game_instance.h"
#include "src/client/system/logic/character_logic_system.h"
#include "src/client/system/ui/equip_ui_system.h"
#include "src/client/system/ui/package_ui_system.h"
#include "src/client/window/window.h"
#include "src/common/flatbuffers/server.h"
#include "src/common/physic/physic.h"
#include "src/common/response/server_response.h"
#include "src/server/server/server_drop.h"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <format>
#include <optional>
#include <utility>

float server_drop_instance::cal_available_time(float y1, float y2) {
  const float VY = 400.0f;                     // 初速度（向上）
  const float G = 800.0f;                      // 重力加速度
  const float MAX_RISE = VY * VY / (2.0f * G); // 最大上升高度 = 100.0f

  float max_height = y1 - MAX_RISE;

  // 总下落距离 = 从最高点到y2
  float total_fall_distance = y2 - max_height;

  if (total_fall_distance < 0) {
    // y2在最高点上方，无法到达
    return -1.0f;
  }

  // 上升到最高点的时间
  float rise_time = VY / G; // 0.5s

  // 从最高点下落到y2的时间
  float fall_time = sqrtf(2.0f * total_fall_distance / G);

  return rise_time + fall_time;
}

void server_drop_instance::save_drop(uint64_t map_id, const DropT &drop) {
  auto &scene = server_scene_instance::scenes[map_id];
  server_drop sd;
  sd.dt = drop;
  sd.destroy = window::dt_now + 60 * 5 * 1000;
  sd.available = window::dt_now + cal_available_time(drop.y1, drop.y2) * 1000;
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
    if (scene.drops[r.random_id].available <= window::dt_now) {
      ServerCharacterPickT t;
      t.random_id = r.random_id;
      t.client_id = client_id;
      auto clients = server_scene_instance::scenes[map_id].clients;
      clients.erase(client_id);
      for (const auto c : clients) {
        server_response::send_to_client(c, t);
      }
      t.client_id = 0;
      server_response::send_to_client(client_id, t);
      scene.drops.erase(r.random_id);
    } else {
      ServerCharacterPickT t;
      t.random_id = 0;
      server_response::send_to_client(client_id, t);
    }
  }
}

void server_drop_instance::handle_server_pick(uint64_t client_id,
                                              ServerCharacterPickT &r) {
  if (r.client_id == 0 && r.random_id == 0) {
    // 捡取物品失败
    character_logic_system::ccp = {};
    return;
  }
  if (drop_game_instance::data.contains(r.random_id)) {
    auto &dt = drop_game_instance::data.at(r.random_id);
    game_drop_pick gdp;
    gdp.client_id = r.client_id;
    if (r.pet) {
      gdp.pet_index = r.pet_index;
    }
    dt.picker = gdp;
    dt.pick_time = window::dt_now;
    dt.type = game_drop::drop_enum::pick;

    if (r.client_id == 0) {
      auto itm = dt.data;
      package_game_instance::add_new_item(itm);

      character_logic_system::ccp = {};

      auto itm_num = item_game_instance::load_item_num(dt.data);
      itm_num = std::max(1, itm_num);
      game_gain_log g_log{
          .id = dt.data->id,
          .num = (uint64_t)itm_num,
          .destroy = window::dt_now + 5000,
          .type = gain_enum::item,
      };
      gain_log_game_instance::data.push_back(g_log);
    }
  }
}

void server_drop_instance::handle_server_dt(const DropT &dt) {
  game_drop drop;
  drop.random_id = dt.random_id;
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
  drop.type = game_drop::drop_enum::land;
  return;
}

void server_drop_instance::handle_server_drop(uint64_t client_id,
                                              ServerCharacterDropT &r) {
  if (cursor_game_instance::cursor_hand_net.has_value()) {
    if (r.client_id == 0) {
      cursor_game_instance::cursor_hand_net = std::nullopt;
      if (cursor_game_instance::cursor_hand.has_value()) {
        auto &hand = cursor_game_instance::cursor_hand.value();
        switch (hand.type) {
        case cursor_game_instance::equipment: {
          break;
        }
        case cursor_game_instance::package: {
          int num = 1;
          switch (r.payload->drop.type) {
          case fbs::ItemUnion_Item: {
            auto item = r.payload->drop.AsItem();
            num = item->item_num;
            break;
          }
          default: {
            break;
          }
          }
          auto &itm = package_game_instance::data[hand.val][hand.sub_val];
          switch (static_cast<item_enum>(hand.val)) {
          case item_enum::equip: {
            itm = std::polymorphic<game_item>(game_equip_item{});
            break;
          }
          case item_enum::deco: {
            itm = std::polymorphic<game_item>(game_deco_item{});
            break;
          }
          default: {
            if (!itm->id.starts_with(u"0207")) {
              itm = std::polymorphic<game_item>(game_consume_item{});
            } else {
              item_game_instance::dec_item_num(itm, num);
            }
            break;
          }
          }
          break;
        }
        default: {
          break;
        }
        }
      } else {
        // meso
        auto item = r.payload->drop.AsItem();
        auto num = item->item_num;
        package_game_instance::meso -= num;
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
  std::uniform_int_distribution<uint64_t> dist(1, UINT64_MAX);

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
    uint64_t random_id = 0;
    do {
      random_id = dist(gen);
    } while (server_scene_instance::scenes[map_id].drops.contains(random_id));
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

    // 提取判断墙面
    auto ins =
        physic::fall_intersect_pos({dt.x1, dt.y1}, {dt.x2, dt.y2}, g_fhs);
    auto hspeed = (dt.x1 > dt.x2) ? -1 : 1;

    for (const auto &[k, v] : ins) {
      if (!v.fh.k.has_value()) {
        auto check = physic::fall_collide_wall(hspeed, v.fh, g_fhs);
        if (check) {
          dt.x2 = v.pos.x;
          break;
        }
      }
    }

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

void server_drop_instance::handle_server_drop_fade(ServerDropFadeT &r) {
  auto &data = drop_game_instance::data;
  for (const auto id : r.drop_ids) {
    if (data.contains(id)) {
      data.at(id).type = game_drop::drop_enum::fade;
    }
  }
  return;
}