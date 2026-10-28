#ifndef SDLMS_SRC_CLIENT_GAME_INSTANCE_SEAT_GAME_INSTANCE_H_
#define SDLMS_SRC_CLIENT_GAME_INSTANCE_SEAT_GAME_INSTANCE_H_
#include "src/client/game/game_seat.h"

#include <SDL3/SDL_rect.h>

#include <vector>
namespace sdlms {

class SeatGameInstance {
 public:
  static void Load(uint32_t map_id);
  static inline std::vector<GameSeat> data_;
};

}  // namespace sdlms

#endif  // SDLMS_SRC_CLIENT_GAME_INSTANCE_SEAT_GAME_INSTANCE_H_
