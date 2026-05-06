#pragma once

#include "SDL3/SDL_rect.h"
#include "wz/Node.h"
#include <cstdint>
#include <optional>
#include <vector>
class map_info_game_instance {
public:
  static wz::Node *load(uint32_t map_id);
  static SDL_FRect load_vr_border(uint32_t map_id);
  static std::optional<SDL_FRect> load_swim_rect(uint32_t map_id);
  static std::vector<SDL_FPoint> load_seat(uint32_t map_id);
};