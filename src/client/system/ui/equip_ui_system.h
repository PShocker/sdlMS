#ifndef SDLMS_SRC_CLIENT_SYSTEM_UI_EQUIP_UI_SYSTEM_H_
#define SDLMS_SRC_CLIENT_SYSTEM_UI_EQUIP_UI_SYSTEM_H_

#include "src/client/game/game_item.h"

#include <SDL3/SDL_events.h>
#include <SDL3/SDL_rect.h>

#include <cstdint>
#include <optional>
#include <string>
namespace sdlms {


enum class EquipMouseIndex {
  kCap,
  kEarcc,
  kClothes,
  kPants,
  kShoes,
  kGloves,
  kCape,
  kShield,
  kWeapon,
  kRing0,
  kRing1,
  kRing2,
  kRing3,
};

class EquipUiSystem {
 private:
  static void RenderBackgrnd();
  static void RenderBackgrnd2();
  static void RenderTab();
  static void RenderEquip();
  static void RenderDeco();
  static void RenderDisableTexture(SDL_FPoint slot);
  static void RenderEquipTexture(std::optional<GameEquipItem>& equip,
                                 SDL_FPoint slot, EquipMouseIndex i);
  static void RenderDecoTexture(std::optional<GameDecoItem>& deco,
                                SDL_FPoint slot, EquipMouseIndex i);
  static bool RenderInfo();
  static void RenderButton();

  static SDL_FPoint LoadWh();

  static void EventClose();

  static bool EventClickEquip(SDL_Event* Event);

  static void EventMotion(SDL_Event* Event);
  static void EventTab(SDL_Event* Event);
  static bool EventButton(SDL_Event* Event);
  static void EventTop();

  static void EventMotionBtAudio(SDL_Event* Event);

  static void EventDragStart(SDL_Event* Event);
  static void EventDragEnd();
  static void EventDragMove(SDL_Event* Event);

  static inline std::optional<SDL_FPoint> drag_;
  static inline uint8_t active_tab_;

 public:
  static inline std::optional<EquipMouseIndex> mouse_index_;

  static inline SDL_FPoint pos_;

  static void Open();
  static void Close();

  static bool Render();
  static bool Event(SDL_Event* Event);

  static bool CursorIn();
  static void Toggle();
};

}  // namespace sdlms

#endif  // SDLMS_SRC_CLIENT_SYSTEM_UI_EQUIP_UI_SYSTEM_H_
