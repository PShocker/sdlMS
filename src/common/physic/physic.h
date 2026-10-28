#ifndef SDLMS_SRC_COMMON_PHYSIC_PHYSIC_H_
#define SDLMS_SRC_COMMON_PHYSIC_PHYSIC_H_

#include "src/client/game/game_foothold.h"

#include <SDL3/SDL_rect.h>

#include <cstdint>
#include <flat_map>
#include <optional>
namespace sdlms {

class Physic {
 private:
  static bool WalkFh(SDL_FPoint& pos, bool Fall, int32_t next_fh,
                     int32_t& current_fh, float& hspeed, float& vspeed,
                     const std::flat_map<int32_t, GameFoothold>& fhs);

  static std::optional<SDL_FPoint> FallIntersect(const SDL_FPoint& p1,
                                                 const SDL_FPoint& p2,
                                                 const SDL_FPoint& p3,
                                                 const SDL_FPoint& p4);

 public:
  static bool FallCollideWall(float hspeed, GameFoothold fh,
                              const std::flat_map<int32_t, GameFoothold>& fhs);

  struct IntersectPos {
    SDL_FPoint pos;
    GameFoothold fh;
  };
  static std::flat_multimap<float, IntersectPos> FallIntersectPos(
      const SDL_FPoint& p1, const SDL_FPoint& p2,
      const std::flat_map<int32_t, GameFoothold>& fhs);

  static bool Walk(SDL_FPoint& pos, float delta_time, float& hspeed,
                   float& vspeed, float& hforce, float hspeed_min,
                   float hspeed_max, float friction, bool Fall,
                   int32_t& current_fh, std::optional<SDL_FRect> border,
                   const std::flat_map<int32_t, GameFoothold>& fhs);

  static bool Fall(SDL_FPoint& pos, float delta_time, float& hspeed,
                   float& vspeed, float vspeed_min, float vspeed_max,
                   std::optional<SDL_FRect> border, bool fall_collide,
                   bool wall_collide, int32_t& current_fh, uint8_t& page,
                   const std::flat_map<int32_t, GameFoothold>& fhs,
                   int32_t fall_fh = INT32_MAX);
};

}  // namespace sdlms

#endif  // SDLMS_SRC_COMMON_PHYSIC_PHYSIC_H_
