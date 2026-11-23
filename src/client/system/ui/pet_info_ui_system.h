#pragma once

#include "src/client/system/ui/component/ui_component.h"

#include <SDL3/SDL_events.h>
#include <SDL3/SDL_rect.h>

#include <optional>

namespace sdlms {

// 宠物信息窗口（095 CUI_PetInfo）：背景用 UI.wz/CharacterInfo.img/PetInfo/
// backgrnd_detail，按 PetInfo 里的 vector:* 摆宠物图标、名字、类型/等级/
// 亲密度/饱食度，以及宠物身上穿戴的宠物装备图标。
// 由角色信息窗口的 "showPet" 按钮开关（与 095 相同）。
class PetInfoUiSystem : public UiComponentHost<PetInfoUiSystem> {
 private:
  static SDL_FPoint LoadWh();

  static void RenderBackgrnd();
  static void RenderPet();
  static void RenderEquip();
  static void RenderText();

  static void EventClose();
  static void EventDragStart(SDL_Event* event);
  static void EventDragMove(SDL_Event* event);
  static inline std::optional<SDL_FPoint> drag_;

 public:
  static inline SDL_FPoint pos_;

  static UIComponents& GetComponent();
  static void Open();
  static void Close();
  static void Toggle();
  static bool CursorIn();
  static bool Render();
  static bool Event(SDL_Event* event);
};

}  // namespace sdlms
