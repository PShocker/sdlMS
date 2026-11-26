#pragma once

#include "src/client/system/ui/component/ui_component.h"

#include <SDL3/SDL_events.h>
#include <SDL3/SDL_rect.h>

#include <optional>
#include <string>

namespace sdlms {

// 小游戏开房设置弹窗（素材 UI/Minigame.img/Popup，095 CMiniRoomBaseDlg
// 的建房设置流程）：双击小游戏桌先弹这里——按道具类型选背景
//（backgrnd_omok / backgrnd_match_cards），锁图标（icon_public/
// icon_private）切换公开/私房，私房在 textinput 里输密码，确认后
// 发 step0 进房（同图无房则自己成为房主 slot0；密码随 chat 下发）。
// 窗口是 modal 的：打开期间下层界面不接受输入、按钮不接受 hover。
class MinigameSetupUiSystem : public UiComponentHost<MinigameSetupUiSystem> {
 private:
  static SDL_FPoint LoadWh();

  static void RenderBackgrnd();
  static void RenderInputs();

  static void EventButtonToggle();
  static void EventButtonOk();
  static void EventButtonCancel();
  static void RestoreKeyboard();

  static bool EventUI(SDL_Event* event);

 public:
  static UIComponents& GetComponent();

  // game_type：0=五子棋（0408 桌），1=卡片配对（040801 桌）。
  static void Open(int game_type);
  static void Close();
  static bool IsOpen();
  static bool CursorIn();

  static bool Render();
  static bool Event(SDL_Event* event);

  static inline SDL_FPoint pos_;
  // 0=五子棋 1=配对卡（决定背景与按钮位置）。
  static inline int game_type_ = 0;
  static inline bool use_password_ = false;
  static inline TextInput password_input_{};
  static inline std::optional<SDL_FPoint> drag_;
};

}  // namespace sdlms
