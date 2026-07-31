#include "server_drop_instance.h"
#include "server_scene_instance.h"
#include "src/client/game_instance/cursor_game_instance.h"
#include "src/client/game_instance/drop_game_instance.h"
#include "src/client/game_instance/package_game_instance.h"
#include "src/common/flatbuffers/server.h"
#include "src/common/response/server_response.h"
#include "src/server/server/server_drop.h"
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
  drop.vspeed = -550;
  drop.pos = SDL_FPoint{dt.x1, dt.y1};
  drop.goal = SDL_FPoint{dt.x2, dt.y2};

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