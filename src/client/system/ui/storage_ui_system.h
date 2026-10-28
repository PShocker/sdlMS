#ifndef SDLMS_SRC_CLIENT_SYSTEM_UI_STORAGE_UI_SYSTEM_H_
#define SDLMS_SRC_CLIENT_SYSTEM_UI_STORAGE_UI_SYSTEM_H_

#include "src/client/game/game_item.h"

#include <SDL3/SDL_events.h>
#include <SDL3/SDL_rect.h>

#include <array>
#include <cstdint>
#include <memory>
#include <optional>
#include <string>

namespace sdlms {

class StorageUiSystem {
 private:
  static bool AddItem(std::polymorphic<GameItem>& item);

  static std::optional<uint32_t> LoadMouseIndex();
  static SDL_FPoint LoadWh();

  static void RenderBackgrnd();
  static void RenderButton();
  static void RenderTab();

  static void RenderItems();
  static void RenderItemInfo();
  static void RenderVscr();

  static void EventTab();
  static void EventButtonGet();
  static void EventButtonPut();
  static void EventButtonSort();
  static void EventButtonExit();
  static bool EventButton(SDL_Event* Event);
  static void EventClose();

  static inline std::array<std::optional<uint16_t>, 2> active_item_;
  static inline int active_tab_;
  static inline std::array<uint16_t, 2> page_;

 public:
  static inline std::u16string npc_id_;
  static inline SDL_FPoint pos_;

  static bool CursorIn();

  static void Toggle();

  static void Open();
  static void Close();
  static bool Render();
  static bool Event(SDL_Event* Event);
};

}  // namespace sdlms

#endif  // SDLMS_SRC_CLIENT_SYSTEM_UI_STORAGE_UI_SYSTEM_H_
