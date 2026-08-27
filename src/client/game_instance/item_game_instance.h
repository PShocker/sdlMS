#pragma once

#include "src/client/game/game_character.h"
#include "src/client/game/game_item.h"
#include "src/client/game/game_item_buff.h"
#include "wz/Node.h"
#include <cstdint>
#include <memory>
#include <string>
class item_game_instance {
public:
  static inline std::vector<game_item_buff> buff;

  static bool check_item(const std::u16string &id);

  static std::u16string load_item_text(const std::u16string &id,
                                       const std::u16string &val);
  static std::u16string load_item_type(const std::u16string &id);
  static wz::Node *load_item_info(const std::u16string &id, uint32_t num);
  static int load_slot_max(const std::u16string &id);
  static std::polymorphic<game_item> load_item(const std::u16string &id,
                                               uint32_t num);
  static int load_item_num(std::polymorphic<game_item> &itm);
  static void add_item_num(std::polymorphic<game_item> &item, int num);
  static void dec_item_num(std::polymorphic<game_item> &item, int num);
  static void set_item_num(std::polymorphic<game_item> &itm, int num);

  static void use_morph_item(const std::u16string &id, game_character &g);
  static bool use_mob_item(std::polymorphic<game_item> &itm);
  static void use_buff_item(std::polymorphic<game_item> &itm);
  static void unuse_buff_item(const std::u16string &id);
  static bool use_consume_item(const std::u16string &id);
  static bool use_consume_item(std::polymorphic<game_item> &itm);
  static void use_item(std::polymorphic<game_item> &itm);
};