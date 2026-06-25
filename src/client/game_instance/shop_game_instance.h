#pragma once
#include "src/client/game/game_shop.h"
#include <optional>
#include <string>

class shop_game_instance {
public:
  static std::optional<game_shop> load_npc_shop(const std::u16string &npc_id);
  static game_shop load_shop(const std::u16string &shop_id);
};