#pragma once

#include "src/client/game/game_npc.h"
#include "wz/Node.h"
#include <flat_set>
#include <string>
#include <vector>
class npc_game_instance {
public:
  static void load(uint32_t map_id);
  static wz::Node *load_link_npc_node(const std::u16string &id);
  static inline std::array<std::vector<game_npc>, 8> data;

  static inline const std::flat_set<std::u16string> default_action = {
      u"stand",
      u"eye",
  };
};