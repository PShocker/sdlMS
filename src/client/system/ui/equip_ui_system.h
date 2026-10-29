#pragma once

#include "src/client/system/ui/component/ui_component.h"
#include "src/client/game/game_item.h"

#include <SDL3/SDL_events.h>
#include <SDL3/SDL_rect.h>

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

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
  static void RenderEquip();
  static void RenderDeco();
  static void RenderDisableTexture(SDL_FPoint slot);
  static void RenderEquipTexture(std::optional<GameEquipItem>& equip,
                                 SDL_FPoint slot, EquipMouseIndex i);
  static void RenderDecoTexture(std::optional<GameDecoItem>& deco,
                                SDL_FPoint slot, EquipMouseIndex i);
  static bool RenderInfo();
  static void RenderUI();
  static UIComponents& GetComponent();

  static SDL_FPoint LoadWh();

  static void EventClose();

  static bool EventClickEquip(SDL_Event* event);

  static void EventMotion(SDL_Event* event);
  static bool EventUI(SDL_Event* event);
  static void EventTop();

  static void EventMotionBtAudio(SDL_Event* event);

  static void EventDragStart(SDL_Event* event);
  static void EventDragEnd();
  static void EventDragMove(SDL_Event* event);

  static inline std::optional<SDL_FPoint> drag_;
  static inline uint8_t active_tab_;

  static inline UIComponents components_;

 public:
  static inline std::optional<EquipMouseIndex> mouse_index_;

  static inline SDL_FPoint pos_;

  static void Open();
  static void Close();

  static bool Render();
  static bool Event(SDL_Event* event);

  static bool CursorIn();
  static void Toggle();
};

}  // namespace sdlms
