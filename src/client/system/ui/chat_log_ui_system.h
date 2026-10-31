#pragma once

#include "src/client/system/ui/component/ui_component.h"

#include <SDL3/SDL_events.h>

#include <string>
#include <vector>

namespace sdlms {

class ChatLogUiSystem : public UiComponentHost<ChatLogUiSystem> {
 private:
  static void RenderBackgrnd();
  static void RenderChats();
  static std::vector<std::u16string> LoadChatLines();

  static bool EventUI(SDL_Event* event);


 public:
  static UIComponents& GetComponent();
  static inline int page_;
  // 鼠标点击选中的视觉行，Ctrl+C 复制该行。
  static inline int selected_line_ = -1;
  // 聊天字号，由选项窗口的 Chat Box Font Size 决定
  static inline uint8_t chat_font_size_ = 12;

  static void Open();
  static void Close();

  static bool Render();
  static bool Event(SDL_Event* event);

  static bool CursorIn();

  static void Toggle();
};

}  // namespace sdlms
