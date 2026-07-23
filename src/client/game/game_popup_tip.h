#pragma once

#include "src/client/window/window.h"
#include "src/common/flatbuffers/common.h"
#include <cstdint>

using namespace fbs;

enum class popup_tip_enums {
  trade,
  party,
};

struct game_popup_tip {
  popup_tip_enums type;
  PlayerT player_t;
  int32_t fade = 255;
  uint64_t destory = window::dt_now + 120 * 1000;
};