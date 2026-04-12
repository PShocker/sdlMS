#pragma once

#include "src/client/ui/ui_base.h"
#include <cstdint>
#include <flat_map>

class worldmap_ui_system {
public:
  std::flat_multimap<uint8_t, ui_base> ui;
};