#ifndef SDLMS_SRC_CLIENT_GAME_GAME_STORAGE_H_
#define SDLMS_SRC_CLIENT_GAME_GAME_STORAGE_H_

#include "src/client/game/game_item.h"

#include <cstdint>
#include <memory>
#include <optional>
#include <vector>
namespace sdlms {


struct GameStorage {
  std::vector<std::polymorphic<GameItem>> data;
  uint64_t meso;
};

}  // namespace sdlms

#endif  // SDLMS_SRC_CLIENT_GAME_GAME_STORAGE_H_
