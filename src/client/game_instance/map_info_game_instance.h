#pragma once

#include "wz/Node.h"
#include <cstdint>
class map_info_game_instance {
public:
  static inline wz::Node *map_info;
  static void load(uint32_t map_id);
};