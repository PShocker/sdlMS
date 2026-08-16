#pragma once
#include "src/client/game/game_item_buff.h"
#include <string>
#include <vector>
class item_buff_game_instance {
public:
  static inline std::vector<game_item_buff> data;

  static void use(const std::u16string &id);
  static void end(const std::u16string &id);
};