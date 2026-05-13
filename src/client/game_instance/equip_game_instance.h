#pragma once

#include "wz/Node.h"
#include <string>
class equip_game_instance {
private:
  static std::u16string load_equip_type(const std::u16string &id);
public:
  static wz::Node *load_equip_info(const std::u16string &id);
};