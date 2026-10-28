#ifndef SDLMS_SRC_CLIENT_GAME_GAME_TILE_H_
#define SDLMS_SRC_CLIENT_GAME_GAME_TILE_H_

#include <SDL3/SDL_rect.h>

#include <cstdint>
#include <string>
namespace sdlms {

struct GameTile {
  SDL_FPoint pos;
  int64_t z;

  std::u16string path;
  uint8_t alpha = 255;
};

}  // namespace sdlms

#endif  // SDLMS_SRC_CLIENT_GAME_GAME_TILE_H_
