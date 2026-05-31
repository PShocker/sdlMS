#pragma once

#include "src/client/game/game_item.h"
#include <string>
class item_game_instance {
public:
  static std::u16string load_item_name(game_item &item);
};