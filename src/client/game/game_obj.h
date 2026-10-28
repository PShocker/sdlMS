#ifndef SDLMS_SRC_CLIENT_GAME_GAME_OBJ_H_
#define SDLMS_SRC_CLIENT_GAME_GAME_OBJ_H_

#include <SDL3/SDL_rect.h>

#include <cstdint>
#include <optional>
#include <string>

namespace sdlms {

struct GameObj {
  std::u16string path;
  int64_t z;

  SDL_FPoint pos;
  int8_t flip;

  uint8_t alpha = 255;

  uint32_t ani_index = 0;
  uint32_t ani_time;
  uint8_t ani_count;
};

}  // namespace sdlms

#endif  // SDLMS_SRC_CLIENT_GAME_GAME_OBJ_H_
