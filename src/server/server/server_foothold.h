#ifndef SDLMS_SRC_SERVER_SERVER_SERVER_FOOTHOLD_H_
#define SDLMS_SRC_SERVER_SERVER_SERVER_FOOTHOLD_H_

#include "src/client/game/game_foothold.h"

#include <SDL3/SDL_rect.h>

#include <cstdint>
#include <flat_map>
#include <optional>
#include <string>

namespace sdlms {

class ServerFoothold {
 public:
  GameFoothold fh;
};

}  // namespace sdlms

#endif  // SDLMS_SRC_SERVER_SERVER_SERVER_FOOTHOLD_H_
