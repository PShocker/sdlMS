#pragma once

#include "SDL3/SDL_rect.h"
#include "src/client/game/game_foothold.h"
#include <cstdint>
#include <flat_map>
class physic {
private:
  static bool walk_fh(SDL_FPoint &pos, bool fall, int32_t next_fh,
                      int32_t &current_fh, float &hspeed, float &vspeed,
                      const std::flat_map<int32_t, game_foothold> &fhs);

public:
  static bool walk(SDL_FPoint &pos,float delta_time, float &hspeed, float &vspeed, float &hforce,
                   float &hspeed_min, float &hspeed_max, float friction,
                   bool fall, int32_t &current_fh, const SDL_FRect &border,
                   const std::flat_map<int32_t, game_foothold> &fhs);
};