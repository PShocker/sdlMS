#pragma once
#include "src/client/game/game_tooltip.h"
#include <flat_map>
#include <vector>
class TooltipGameInstance {
public:
  static void load(uint32_t map_id);
  static inline std::vector<GameTooltip> data;
};