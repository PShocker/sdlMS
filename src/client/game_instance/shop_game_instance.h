#pragma once
#include "src/client/game/game_shop.h"
#include <cstdint>
#include <optional>
#include <string>

class shop_game_instance {
public:
  static std::polymorphic<game_item> load_item(const std::u16string &id);
  static const game_shop *load_npc_shop(const std::u16string &npc_id);
  static game_shop load_shop(const std::u16string &shop_id);
  static game_shop_item load_shop_item(const std::u16string &id);
};