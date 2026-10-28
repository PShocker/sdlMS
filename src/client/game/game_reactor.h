#ifndef SDLMS_SRC_CLIENT_GAME_GAME_REACTOR_H_
#define SDLMS_SRC_CLIENT_GAME_GAME_REACTOR_H_

#include <SDL3/SDL_rect.h>

#include <cstdint>
#include <string>
namespace sdlms {

struct GameReactor {
  std::u16string id;
  uint8_t index;

  SDL_FPoint pos;

  uint8_t alpha = 255;
  bool f;

  uint8_t state;
  uint32_t ani_index = 0;
  uint32_t ani_time;
  bool hit = false;
};

}  // namespace sdlms

#endif  // SDLMS_SRC_CLIENT_GAME_GAME_REACTOR_H_
