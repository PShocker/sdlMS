#ifndef SDLMS_SRC_CLIENT_SYSTEM_UI_SHOP_UI_SYSTEM_H_
#define SDLMS_SRC_CLIENT_SYSTEM_UI_SHOP_UI_SYSTEM_H_

#include "src/client/game/game_item.h"
#include "src/client/game/game_npc.h"
#include "src/client/game/game_shop.h"

#include <SDL3/SDL_events.h>
#include <SDL3/SDL_rect.h>

#include <array>
#include <optional>
#include <string>

namespace sdlms {

class ShopUiSystem {
 private:
  static void RenderBackgrnd();
  static void RenderButton();

  static void RenderItem(std::vector<GameShopItem>& items, int page,
                         SDL_FPoint p);

  static void RenderItems();
  static void RenderTab();
  static void RenderVscr();
  static void RenderMeso();

  static void RenderNpc();
  static void RenderAvatar();
  static void RenderPkgItems();

  static void RenderActiveItem();

  static void RenderItemInfo(std::polymorphic<GameItem>& item);

  static std::optional<int> EventItemClick(SDL_Event* Event, SDL_FPoint p);
  static bool EventItem(SDL_Event* Event);

  static bool EventVscr(SDL_Event* Event);
  static void EventClose();
  static bool EventButton(SDL_Event* Event);

  static void EventVscrStart(SDL_Event* Event);
  static void EventVscrEnd();
  static void EventVscrMove(SDL_Event* Event);

  static inline std::array<bool, 2> vscr_motion_;

  static bool EventTab(SDL_Event* Event);

  static SDL_FPoint LoadWh();

 public:
  static inline std::array<int, 2> pages_;

  static inline std::array<std::optional<int>, 2> active_item_;
  static inline std::array<int, 2> active_tab_;

  static inline std::optional<GameNpc> npc_;
  static inline const GameShop* shop_;

  static inline SDL_FPoint pos_;

  static void Open();
  static void Close();

  static bool Render();
  static bool Event(SDL_Event* Event);

  static bool CursorIn();

  static void Toggle();
};

}  // namespace sdlms

#endif  // SDLMS_SRC_CLIENT_SYSTEM_UI_SHOP_UI_SYSTEM_H_
