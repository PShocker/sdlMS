#ifndef SDLMS_SRC_SERVER_SERVER_SERVER_DROP_H_
#define SDLMS_SRC_SERVER_SERVER_SERVER_DROP_H_

#include <SDL3/SDL_rect.h>
#include "src/client/game/game_drop.h"
#include "src/common/flatbuffers/common.h"
#include <cstdint>
#include <optional>
namespace sdlms {



struct ServerDrop{
  fbs::DropT dt;
  uint64_t available;
  uint64_t destroy;
};

}  // namespace sdlms

#endif  // SDLMS_SRC_SERVER_SERVER_SERVER_DROP_H_
