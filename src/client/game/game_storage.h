#pragma once

#include "game_item.h"
#include <cstdint>
#include <optional>
#include <variant>
#include <vector>

class game_storage {
public:
  std::vector<std::optional<game_item>> data;
  uint64_t meso;
};