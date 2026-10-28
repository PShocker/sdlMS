#pragma once

#include "src/client/game/game_portal.h"
#include <cstdint>
#include <flat_map>
#include <flat_set>
#include <string>
#include <vector>
namespace sdlms {

class PortalGameInstance {
public:
  static std::flat_multimap<std::u16string, GamePortal> Load(uint32_t map_id);
  static inline std::flat_multimap<std::u16string, GamePortal> data_;
};

}  // namespace sdlms
