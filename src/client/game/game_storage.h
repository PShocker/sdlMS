#pragma once

#include "game_item.h"
#include <cstdint>
#include <memory>
#include <optional>
#include <vector>

class game_storage {
public:
  std::vector<std::polymorphic<game_item>> data;
  uint64_t meso;
};