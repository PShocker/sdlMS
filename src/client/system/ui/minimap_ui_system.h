#pragma once

#include "src/client/ui/ui_base.h"
#include <cstdint>
#include <flat_map>

class minimap_ui_system {
public:
  minimap_ui_system(uint32_t map_id);
  std::flat_multimap<uint8_t, ui_base> ui;

private:
  void load_minimap(uint32_t map_id);
  void load_button();
};