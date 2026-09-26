#pragma once

#include "src/client/window/window.h"
#include "src/common/flatbuffers/common.h"
#include <any>
#include <cstdint>

using namespace fbs;

enum class popup_tip_enums {
  trade,
  party,
  quest,
};

class game_popup_tip {
public:
  popup_tip_enums type;
  int32_t alpha = 0;
  uint64_t destroy = window::dt_now + 120 * 1000;
  std::any data;
};