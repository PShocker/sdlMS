#ifndef SDLMS_SRC_CLIENT_GAME_GAME_TOMB_H_
#define SDLMS_SRC_CLIENT_GAME_GAME_TOMB_H_

#include <SDL3/SDL_rect.h>
#include <cstdint>
#include <string>
namespace sdlms {

struct GameTomb{
  std::u16string ani_type;
  uint8_t ani_index;
  uint32_t ani_time;

  SDL_FPoint pos;
  SDL_FPoint b;

  float rotation = 0;
};

}  // namespace sdlms

#endif  // SDLMS_SRC_CLIENT_GAME_GAME_TOMB_H_
