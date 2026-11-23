#pragma once

#include "src/client/game/game_item.h"
#include "src/client/game/game_npc.h"
#include "src/client/system/ui/component/ui_component.h"

#include <SDL3/SDL_events.h>
#include <SDL3/SDL_rect.h>

#include <array>
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace sdlms {

// 仓库（Trunk）窗口：左侧是仓库里的道具，右侧是角色背包，
// 选中道具后按 get／put 在两个容器之间搬运，按 sort 整理仓库。
// 由仓库 NPC（Npc.wz 里带 info/trunkPut 的 NPC）触发打开。
class StorageUiSystem : public UiComponentHost<StorageUiSystem> {
 private:
  static SDL_FPoint LoadWh();

  static void RenderBackgrnd();
  static void RenderTab();
  static void RenderNpc();
  static void RenderAvatar();
  static void RenderItems(std::vector<std::polymorphic<GameItem>>& items,
                          SDL_FPoint lt, uint16_t page,
                          const std::optional<uint16_t>& active);
  static void RenderMeso();
  static void RenderItemInfo();

  static bool AddItem(std::polymorphic<GameItem>& item);
  static void ClearItem(std::polymorphic<GameItem>& item);

  static std::optional<uint32_t> LoadMouseIndex(SDL_FPoint lt, uint16_t page,
                                                size_t count);

  static void EventButtonGet();
  static void EventButtonPut();
  static void EventButtonSort();
  static void EventButtonExit();
  static bool EventItem(SDL_Event* event);
  static bool EventUI(SDL_Event* event);
  static void EventClose();

  static inline std::array<std::optional<uint16_t>, 2> active_item_;
  static inline int active_tab_ = 0;
  static inline std::array<uint16_t, 2> page_;

 public:
  static UIComponents& GetComponent();

  static inline std::u16string npc_id_;
  static inline std::optional<GameNpc> npc_;
  static inline SDL_FPoint pos_;

  static bool CursorIn();

  static void Toggle();

  static void Open();
  static void Close();
  static bool Render();
  static bool Event(SDL_Event* event);
};

}  // namespace sdlms
