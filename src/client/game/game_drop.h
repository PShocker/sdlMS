#pragma once

#include "src/client/game/game_item.h"

#include <SDL3/SDL_rect.h>

#include <cstdint>
#include <memory>
#include <optional>

namespace sdlms {

struct GameDropPick {
  uint64_t client_id = 0;
  std::optional<uint8_t> pet_index;
};

struct GameDrop {
  uint64_t random_id = 0;

  std::polymorphic<GameItem> data;

  SDL_FPoint pos = {0.0f, 0.0f};
  SDL_FPoint goal = {0.0f, 0.0f};

  int32_t rotate = 0;
  uint8_t alpha = 255;

  float hspeed = 0;
  float vspeed = 0;

  // 图标尺寸由 DropRenderSystem 写入，用于 095 的“底部中心”定位。
  float icon_w = 0.0f;
  float icon_h = 0.0f;

  uint8_t page = 0;

  enum class DropEnum {
    kFly,
    kLand,
    kPick,
    kFade,
  };

  DropEnum type = DropEnum::kFly;

  uint64_t pick_time = 0;
  std::optional<GameDropPick> picker;

  // 服务器通知掉落物消失的时刻；alpha 在 500ms 内从 255 渐隐到 0。
  uint64_t fade_time = 0;
};

}  // namespace sdlms
