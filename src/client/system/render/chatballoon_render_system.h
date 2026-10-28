#pragma once

#include "SDL3/SDL_pixels.h"
#include "SDL3/SDL_rect.h"
#include "SDL3/SDL_render.h"
#include "src/client/game/game_chatballoon.h"
#include <flat_map>
#include <optional>
#include <string>
class ChatballoonRenderSystem {
private:
  struct BackgrndPad {
    float x;
    float y;
    float w;
    float h;
    std::optional<int32_t> color;
  };
  static std::optional<BackgrndPad>
  render_backgrnd(GameChatballoon &g_chatballoon, SDL_FPoint base);

public:
  static bool render(GameChatballoon &g_chatballoon, SDL_FPoint base);
};