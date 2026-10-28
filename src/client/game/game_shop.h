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

class GameShop {
public:
  std::vector<GameShopItem> items;
};

}  // namespace sdlms
