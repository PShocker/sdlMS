#pragma once

#include "SDL3/SDL_rect.h"
#include "src/client/window/window.h"
#include "src/common/flatbuffers/common.h"
#include <cstdint>
enum class popup_tip_enums {
  trade,
  party,
};
struct game_popup_tip {
  popup_tip_enums type;
  SDL_FPoint pos;
  fbs::PlayerT player;
  uint64_t destory = window::dt_now + 120 * 1000;
};