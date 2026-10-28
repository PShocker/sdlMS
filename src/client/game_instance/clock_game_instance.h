#ifndef SDLMS_SRC_CLIENT_GAME_INSTANCE_CLOCK_GAME_INSTANCE_H_
#define SDLMS_SRC_CLIENT_GAME_INSTANCE_CLOCK_GAME_INSTANCE_H_

#include <SDL3/SDL_rect.h>
#include <wz/Node.h>

#include <vector>

namespace sdlms {

class ClockGameInstance {
 public:
  static void Load(wz::Node* image);
  static void Load(uint32_t map_id);
  static inline std::vector<SDL_FRect> data_;
};

}  // namespace sdlms

#endif  // SDLMS_SRC_CLIENT_GAME_INSTANCE_CLOCK_GAME_INSTANCE_H_
