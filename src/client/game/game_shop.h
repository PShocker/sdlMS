#pragma once

#include <string>
#include <vector>
struct game_shop_item {
  std::u16string itemId;
  int price;
};

class game_shop {
public:
  std::vector<game_shop_item> items;
};