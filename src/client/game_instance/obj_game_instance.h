#pragma once

#include "SDL3/SDL_rect.h"
#include "src/client/game/game_obj.h"
#include "wz/Node.h"
#include <array>
#include <cstdint>
#include <flat_map>
#include <vector>
class obj_game_instance {
private:
  static void load_clock(wz::Node *image);

public:
  static void load(wz::Node *image);
  static void load(uint32_t map_id);
  static inline std::array<std::flat_multimap<int64_t, game_obj>, 8> data;
  static inline std::vector<SDL_FRect> clock;
};