#pragma once
#include "src/client/game/game_tooltip.h"
#include <flat_map>
#include <vector>
namespace sdlms {

class TooltipGameInstance {
public:
  static void Load(uint32_t map_id);
  static inline std::vector<GameTooltip> data_;
};

}  // namespace sdlms
