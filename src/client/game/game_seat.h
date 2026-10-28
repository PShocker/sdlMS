#ifndef SDLMS_SRC_CLIENT_GAME_GAME_SEAT_H_
#define SDLMS_SRC_CLIENT_GAME_GAME_SEAT_H_

#include <SDL3/SDL_rect.h>

#include <cstdint>
#include <string>
namespace sdlms {

class GameSeat {
 public:
  SDL_FPoint pos;
};

}  // namespace sdlms

#endif  // SDLMS_SRC_CLIENT_GAME_GAME_SEAT_H_
