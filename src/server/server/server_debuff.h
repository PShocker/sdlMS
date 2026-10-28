#ifndef SDLMS_SRC_SERVER_SERVER_SERVER_DEBUFF_H_
#define SDLMS_SRC_SERVER_SERVER_SERVER_DEBUFF_H_

#include "src/client/game/game_debuff.h"

#include <SDL3/SDL_rect.h>

#include <cstdint>
#include <flat_map>
#include <optional>
#include <string>
namespace sdlms {

struct ServerDebuff {
  uint64_t client_id;
  GameDebuff db;
};

}  // namespace sdlms

#endif  // SDLMS_SRC_SERVER_SERVER_SERVER_DEBUFF_H_
