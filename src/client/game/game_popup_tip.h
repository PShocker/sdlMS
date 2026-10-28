#pragma once

#include "src/client/window/window.h"
#include "src/common/flatbuffers/common.h"
#include <any>
#include <cstdint>

using namespace fbs;

enum class PopupTipEnums {
  trade,
  party,
  quest,
};

class GamePopupTip {
public:
  PopupTipEnums type;
  int32_t alpha = 0;
  uint64_t destroy = Window::dt_now + 120 * 1000;
  std::any data;
};