#pragma once

#include "src/client/game/game_mob.h"
#include "wz/Node.h"
#include <cstdint>
#include <flat_set>
#include <vector>
class mob_game_instance {
public:
  static wz::Node *load_link_mob_node(const std::u16string &id);

  static std::array<std::vector<game_mob>, 8> load(uint32_t map_id);

  static inline std::array<std::vector<game_mob>, 8> data;
};