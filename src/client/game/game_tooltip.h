#ifndef SDLMS_SRC_CLIENT_GAME_GAME_TOOLTIP_H_
#define SDLMS_SRC_CLIENT_GAME_GAME_TOOLTIP_H_

#include <SDL3/SDL_rect.h>
#include <cstdint>
#include <string>
namespace sdlms {

struct GameTooltip{
  std::u16string path;
  SDL_FRect r;
};

}  // namespace sdlms

#endif  // SDLMS_SRC_CLIENT_GAME_GAME_TOOLTIP_H_
