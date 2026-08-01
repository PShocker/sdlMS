#pragma once

#include "src/client/game/game_item.h"
#include "wz/Node.h"
#include <cstdint>
#include <memory>
#include <string>
class item_game_instance {
public:
  static bool check_item(const std::u16string &id);

  static std::u16string load_item_text(const std::u16string &id,
                                       const std::u16string &val);
  static std::u16string load_item_type(const std::u16string &id);
  static wz::Node *load_item_info(const std::u16string &id, uint32_t num);
  static int load_slot_max(const std::u16string &id);
  static std::polymorphic<game_item> load_item(const std::u16string &id,
                                               uint32_t num);
  static int load_item_num(std::polymorphic<game_item> &itm);
};