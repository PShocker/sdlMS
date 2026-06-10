#include "drop_game_instance.h"
#include "SDL3/SDL_rect.h"
#include "src/client/game/game_drop.h"
#include "src/client/game/game_equip.h"
#include "src/client/game_instance/cursor_game_instance.h"
#include <format>
#include <optional>

void drop_game_instance::load_drop(const DropT &dt) {
  game_drop drop;
  drop.page = dt.page;
  drop.vspeed = -550;
  drop.pos = SDL_FPoint{dt.x1, dt.y1};
  drop.goal = SDL_FPoint{dt.x2, dt.y2};

  switch (dt.drop.type) {
  case fbs::DropUnion_NONE: {
    break;
  }
  case fbs::DropUnion_Equip: {
    auto equipT = dt.drop.AsEquip();
    game_equip equip;
    auto tmp = std::format("{:08d}", equipT->equip_id);
    equip.id = {tmp.begin(), tmp.end()};
    drop.data = equip;
    break;
  }
  case fbs::DropUnion_Item: {
    auto item = dt.drop.AsItem();
    break;
  }
  }
  data.emplace(dt.random_id, drop);
}

void drop_game_instance::pick_drop(uint64_t client_id,
                                   ServerCharacterDropT &r) {
  auto random_id = r.payload->random_id;
  if (drop_game_instance::data.contains(random_id)) {
    auto &drop = drop_game_instance::data.at(random_id);
    drop.type = game_drop::pick;
    drop.picker = game_drop::pick_data{
        .client_id = r.client_id,
        .pet_id = std::nullopt,
    };
  }
}