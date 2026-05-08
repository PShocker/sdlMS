#pragma once

#include "SDL3/SDL_rect.h"
#include "src/client/game/game_foothold.h"
#include <cstdint>
#include <flat_map>
#include <optional>
class physic {
private:
  static bool walk_fh(SDL_FPoint &pos, bool fall, int32_t next_fh,
                      int32_t &current_fh, float &hspeed, float &vspeed,
                      const std::flat_map<int32_t, game_foothold> &fhs);

  static std::optional<SDL_FPoint> fall_intersect(const SDL_FPoint &p1,
                                                  const SDL_FPoint &p2,
                                                  const SDL_FPoint &p3,
                                                  const SDL_FPoint &p4);

public:
  static bool walk(SDL_FPoint &pos, float delta_time, float &hspeed,
                   float &vspeed, float &hforce, float hspeed_min,
                   float hspeed_max, float friction, bool fall,
                   int32_t &current_fh, const SDL_FRect &border,
                   const std::flat_map<int32_t, game_foothold> &fhs);

  static bool fall(SDL_FPoint &pos, float delta_time, float &hspeed,
                   float &vspeed, float vspeed_min, float vspeed_max,
                   const SDL_FRect &border, bool fall_collide,
                   bool wall_collide,
                   const std::flat_map<int32_t, game_foothold> &fhs);
};