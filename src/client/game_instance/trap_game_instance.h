#pragma once

#include "SDL3/SDL_rect.h"
#include "src/client/game/game_obj.h"
#include "wz/Node.h"
#include <array>
#include <cstdint>
#include <flat_map>
class TrapGameInstance {
public:
  static void load(wz::Node *image);
  static void load(uint32_t map_id);
  static inline std::array<std::flat_multimap<int64_t, GameObj>, 8> data;

  struct AnimateData {
    int index;
    int time;
  };
  static AnimateData load_animate_index(std::vector<int> delays);
  static std::vector<int> load_animate_delays(wz::Node *node);
};