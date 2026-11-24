#pragma once

#include <SDL3/SDL_events.h>

namespace sdlms {

class CashShopUiSystem {
 private:
  static void RenderBackgrnd();
  static void RenderAvatar();

 public:
  // 试穿角色的逻辑：在试穿窗口的草地上左右走动。
  static bool Run();
  static bool Render();
  static bool Event(SDL_Event* event);

  // 作为覆盖层挂到当前地图的系统上，ESC 关闭并恢复地图现场。
  static void Open();
  static void Close();
  static void Toggle();
};

}  // namespace sdlms
