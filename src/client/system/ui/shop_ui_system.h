#pragma once

#include "src/client/system/ui/component/ui_component.h"
#include "src/client/game/game_item.h"
#include "src/client/game/game_npc.h"
#include "src/client/game/game_shop.h"

#include <SDL3/SDL_events.h>
#include <SDL3/SDL_rect.h>

#include <array>
#include <optional>
#include <string>
#include <vector>

namespace sdlms {

class ShopUiSystem : public UiComponentHost<ShopUiSystem> {
 private:
  static void RenderBackgrnd();

  static void RenderItem(std::vector<GameShopItem>& items, int page,
                         SDL_FPoint p);

  static void RenderItems();
  static void RenderMeso();

  static void RenderNpc();
  static void RenderAvatar();
  static void RenderPkgItems();

  static void RenderActiveItem();

  static void RenderItemInfo(std::polymorphic<GameItem>& item);

  static std::optional<int> EventItemClick(SDL_Event* event, SDL_FPoint p);
  static bool EventItem(SDL_Event* event);

  static void EventClose();
  static bool EventUI(SDL_Event* event);




  static SDL_FPoint LoadWh();


 public:
  static UIComponents& GetComponent();
  static inline std::array<int, 2> pages_;

  static inline std::array<std::optional<int>, 2> active_item_;
  static inline std::array<int, 2> active_tab_;

  static inline std::optional<GameNpc> npc_;
  static inline const GameShop* shop_;

  static inline SDL_FPoint pos_;

  static void Open();
  static void Close();

  static bool Render();
  static bool Event(SDL_Event* event);

  static bool CursorIn();

  static void Toggle();
};

}  // namespace sdlms
