#pragma once

#include "src/client/game/game_chatballoon.h"

#include <SDL3/SDL_rect.h>

#include <cstdint>
#include <optional>
#include <string>

namespace sdlms {

enum class NpcState : uint8_t {
  kStand,
  kMove,
  kAction,
};

struct GameNpc {
  std::u16string id;
  int32_t rx0 = 0;
  int32_t rx1 = 0;
  int32_t fh = 0;

  SDL_FPoint pos = {0.0f, 0.0f};
  bool flip = false;

  std::u16string action;
  NpcState state = NpcState::kStand;

  uint8_t alpha = 255;

  uint32_t ani_index = 0;
  uint32_t ani_time = 0;

  // 当前状态（站立等待 / 移动 / 一次性动作）的结束时间
  uint64_t duration = 0;

  std::optional<GameChatballoon> chatballoon;
  std::optional<float> hspeed = 100.0f;
  std::optional<float> hforce;
};

}  // namespace sdlms
