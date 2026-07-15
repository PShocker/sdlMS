#pragma once

#include "src/client/game/game_item.h"
#include <memory>
#include <string>
#include <vector>
struct game_shop_item {
  std::polymorphic<game_item> item;
  int price;
};

class game_shop {
public:
  std::vector<game_shop_item> items;
};