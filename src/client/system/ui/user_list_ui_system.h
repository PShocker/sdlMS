#pragma once

#include <SDL3/SDL_events.h>

namespace sdlms {

// 好友列表窗口（UserList.img/Buddy）。好友标识使用人物唯一 id。
class UserListUiSystem {
 private:
  static void RenderBackgrnd();

 public:
  static bool Render();
  static bool Event(SDL_Event* event);
  static void Open();
  static void Close();
  static void Toggle();
};

}  // namespace sdlms