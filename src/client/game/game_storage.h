#pragma once

#include "game_item.h"
#include <cstdint>
#include <memory>
#include <optional>
#include <vector>
namespace sdlms {


struct GameStorage{
  std::vector<std::polymorphic<GameItem>> data;
  uint64_t meso;
};

}  // namespace sdlms
