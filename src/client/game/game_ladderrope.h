#ifndef SDLMS_SRC_CLIENT_GAME_GAME_LADDERROPE_H_
#define SDLMS_SRC_CLIENT_GAME_GAME_LADDERROPE_H_

#include <SDL3/SDL_rect.h>
#include <cstdint>
namespace sdlms {

struct GameLadderrope{
  uint8_t l;
  uint8_t uf;
  int32_t x;
  int32_t y1;
  int32_t y2;

  int32_t t;
  int32_t b;

  uint8_t page;
  uint8_t piece;
};

}  // namespace sdlms

#endif  // SDLMS_SRC_CLIENT_GAME_GAME_LADDERROPE_H_
