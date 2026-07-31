#pragma once

#include "src/client/game/game_foothold.h"
#include <cstdint>
#include <flat_map>
#include <vector>
class foothold_game_instance {
private:
  static inline std::vector<game_foothold> load_obj_fhs(uint32_t map_id);

public:
  static std::flat_map<int32_t, game_foothold> load(uint32_t map_id);
  static inline std::flat_map<int32_t, game_foothold> data;
};