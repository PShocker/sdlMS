#pragma once

#include "src/client/window/window.h"
#include "src/common/flatbuffers/common.h"

#include <any>
#include <cstdint>

namespace sdlms {

enum class PopupTipEnums {
  kTrade,
  kParty,
  kQuest,
};

struct GamePopupTip {
  PopupTipEnums type;
  int32_t alpha = 0;
  uint64_t destroy = Window::dt_now_ + 120 * 1000;
  std::any data;
};

}  // namespace sdlms
