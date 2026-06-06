#pragma once

#include "src/client/game/game_item.h"
#include "wz/Node.h"
#include <string>
class item_game_instance {
public:
  static std::u16string load_item_name(const std::u16string &id);
  static std::u16string load_item_desc(const std::u16string &id);
  static std::u16string load_item_type(const std::u16string &id);
  static wz::Node *load_item_info(const std::u16string &id);
};