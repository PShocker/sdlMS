#pragma once

#include "game_equip.h"
#include "game_item.h"
#include <cstdint>
#include <optional>
#include <variant>
#include <vector>

class game_storage {
public:
  std::vector<std::optional<std::variant<game_equip, game_item>>> data;
  uint64_t meso;
};