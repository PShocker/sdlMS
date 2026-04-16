#pragma once

#include "src/client/ui/ui_base.h"
#include <cstdint>
#include <flat_map>

class statusbar_ui_system {
public:
  statusbar_ui_system();
  std::flat_multimap<uint8_t, ui_base> ui;

  void load_button();
  void load_hp_bar();
  void load_mp_bar();
  void load_exp_bar();
};