#pragma once

#include "src/client/game/game_chatballoon.h"

#include <SDL3/SDL_pixels.h>
#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_render.h>

#include <flat_map>
#include <optional>
#include <string>

namespace sdlms {

class ChatballoonRenderSystem {
 private:
  // 气泡背景的绘制结果：整块气泡的屏幕矩形 + 文字排版区域
  struct BackgrndPad {
    SDL_FRect rect;  // 含箭头的整块气泡（屏幕坐标）
    SDL_FRect text;  // 文字绘制区域（已按行高垂直居中）
    std::optional<int32_t> color;
  };
  static std::optional<BackgrndPad> RenderBackgrnd(
      GameChatballoon& g_chatballoon, SDL_FPoint base);

  static int CeilToMultiple(int value, int tile);

 public:
  // base 是气泡尾巴尖端要对准的世界坐标（角色头顶 / NPC 头顶）
  static bool Render(GameChatballoon& g_chatballoon, SDL_FPoint base);
};

}  // namespace sdlms
