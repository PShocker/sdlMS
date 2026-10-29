#pragma once

#include "src/client/game/game_item.h"

#include <memory>
#include <string>
#include <vector>

namespace sdlms {

struct GameShopItem {
  std::polymorphic<GameItem> item;
  int price;
};

struct GameShop {
  std::vector<GameShopItem> items;
};

}  // namespace sdlms
