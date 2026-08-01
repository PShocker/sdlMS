#include "server_drop_instance.h"
#include "server_scene_instance.h"
#include "src/client/game_instance/cursor_game_instance.h"
#include "src/client/game_instance/drop_game_instance.h"
#include "src/client/game_instance/item_game_instance.h"
#include "src/client/game_instance/map_info_game_instance.h"
#include "src/client/game_instance/package_game_instance.h"
#include "src/client/game_instance/random_game_instance.h"
#include "src/common/flatbuffers/server.h"
#include "src/common/physic/physic.h"
#include "src/common/response/server_response.h"
#include "src/server/server/server_drop.h"
#include <cmath>
#include <format>
#include <utility>

void server_drop_instance::save_drop(uint64_t map_id, const DropT &drop) {
  auto &scene = server_scene_instance::scenes[map_id];
  scene.drops.emplace(drop.random_id, server_drop{drop});
}

void server_drop_instance::handle_drop(uint64_t client_id,
                                       ClientCharacterDropT &r) {
  auto map_id = r.map_id;
  save_drop(map_id, *r.payload);

  const auto &scene = server_scene_instance::scenes[map_id];
  ServerCharacterDropT t;
  t.client_id = client_id;
  t.payload = std::move(r.payload);
  for (const auto c : scene.clients) {
    server_response::send_to_client(c, t);
  }
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
  drop.vspeed = -555;
  drop.pos = SDL_FPoint{dt.x1, dt.y1};
  drop.goal = SDL_FPoint{dt.x2, dt.y2};
  drop.hspeed = (dt.x2 - dt.x1) / (-drop.vspeed / 2000);

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

void server_drop_instance::handle_server_drop(uint64_t client_id,
                                              ServerCharacterDropT &r) {
  if (cursor_game_instance::cursor_hand_net.has_value()) {
    if (r.payload->random_id == cursor_game_instance::cursor_hand_net->id) {
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
  float mid = dts_size / 2;
  float offset = (dts_size % 2 == 0) ? 0.5f : 0.0f;

  int n = 0;
  auto max_h = std::pow(555, 2) / (2 * 2000);

  for (auto dt : dts) {
    dt.random_id = dist(gen);
    int x = (n - mid + offset) * (dt_w);
    x = x - dt_w / 2.0f;
    dt.x2 = dt.x1 + x;
    dt.y2 = dt.y1 - max_h;

    int32_t tmp_fh;
    uint8_t tmp_page;
    float tmp_hsp = 0;
    float tmp_vsp = 10000;
    SDL_FPoint tmp_fp{dt.x2, dt.y2};
    physic::fall(tmp_fp, 100000, tmp_hsp, tmp_vsp, tmp_vsp, tmp_vsp, border,
                 true, true, tmp_fh, tmp_page, g_fhs);
    dt.x2 = tmp_fp.x;
    dt.y2 = tmp_fp.y;

    r.push_back(std::make_unique<fbs::DropT>(dt));
    n++;
  }
  return r;
}