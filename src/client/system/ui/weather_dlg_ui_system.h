#pragma once

#include "src/client/system/ui/component/ui_component.h"

#include <SDL3/SDL_events.h>
#include <SDL3/SDL_rect.h>

#include <string>

namespace sdlms {

// 095 SendConsumeCashItemUseRequest（现金道具 type=16）+ CField::BlowWeather：
// 使用天气道具时先输入要显示的文字（最长 35 字），确认后由服务器广播到同图，
// 所有玩家按道具 info/floatType 显示 FloatNotice。
class WeatherDlgUiSystem : public UiComponentHost<WeatherDlgUiSystem> {
 private:
  static void RenderBackgrnd();
  static void RenderText();
  static void RenderInput();

  static void RestoreKeyboard();

  static void EventButtonOk();
  static void EventButtonCancel();

  static bool EventUI(SDL_Event* event);

  static inline std::u16string item_id_;
  static inline TextInput input_{};
  static inline SDL_FPoint pos_;

 public:
  static UIComponents& GetComponent();
  static void Open(const std::u16string& item_id);
  static void Close();
  static bool IsOpen();

  static bool Render();
  static bool Event(SDL_Event* event);

  static bool CursorIn();
};

}  // namespace sdlms
